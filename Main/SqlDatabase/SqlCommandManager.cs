using System;
using System.Linq;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.Text;

using Microsoft.SqlServer.Management.Common;
using Microsoft.SqlServer.Management.Smo;

namespace Almonaster.Database.Sql
{
    public class SqlCommandManager : IDisposable
    {
        SqlConnection conn;
        SqlTransaction tx;
        bool setComplete;

        internal SqlCommandManager(string connString, IsolationLevel isoLevel)
        {
            this.conn = new SqlConnection(connString);
            this.conn.Open();

            if (isoLevel != IsolationLevel.Unspecified)
            {
                this.tx = this.conn.BeginTransaction(isoLevel);
            }
        }

        public void Dispose()
        {
            try
            {
                if (this.tx != null)
                {
                    if (this.setComplete)
                    {
                        this.tx.Commit();
                    }
                    else
                    {
                        this.tx.Rollback();
                    }
                }
            }
            finally
            {
                this.conn.Dispose();
                this.conn = null;

                if (this.tx != null)
                {
                    this.tx.Dispose();
                    this.tx = null;
                }

                GC.SuppressFinalize(this);
            }
        }

        public void SetComplete()
        {
            this.setComplete = true;
        }

        internal bool CreateDatabaseIfNecessary(string databaseName)
        {
            ServerConnection serverConn = new ServerConnection(this.conn);
            Server server = new Server(serverConn);

            if (!server.Databases.Contains(databaseName))
            {
                var database = new Microsoft.SqlServer.Management.Smo.Database(server, databaseName);
                database.Create();
                return true;
            }

            return false;
        }

        public bool DoesTableExist(string tableName)
        {
            string cmdText = String.Format("SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE [TABLE_NAME] = '{0}'", tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    if (!reader.Read())
                    {
                        throw new SqlDatabaseException("No data returned from query");
                    }
                    return (int)reader[0] != 0;
                }
            }

            //ServerConnection serverConn = new ServerConnection(this.conn);
            //Server server = new Server(serverConn);

            //var connString = new SqlConnectionStringBuilder(this.conn.ConnectionString);
            //return server.Databases[connString.InitialCatalog].Tables.Contains(tableName);
        }

        public void CreateTable(TableDescription tableDesc)
        {
            // TODOTODO - redo this to be transactional, use TSQL
            SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder(this.conn.ConnectionString);
            ServerConnection serverConn = new ServerConnection(builder.DataSource);

            Server server = new Server(serverConn);
            var database = server.Databases[builder.InitialCatalog];

            Table table = new Table(database, tableDesc.Name);

            List<Index> indexes = new List<Index>();
            foreach (ColumnDescription colDesc in tableDesc.Columns)
            {
                DataType dt = TypeMap.Convert(colDesc.Type, colDesc.Size);
                Column col = new Column(table, colDesc.Name, dt)
                {
                    Identity = colDesc.IsPrimaryKey
                };

                table.Columns.Add(col);

                if (colDesc.IsPrimaryKey)
                {
                    Index index = new Index(table, tableDesc.Name + "_PK")
                    {
                        IndexKeyType = IndexKeyType.DriPrimaryKey,
                        IsClustered = true,
                        IsUnique = true,
                    };
                    index.IndexedColumns.Add(new IndexedColumn(index, colDesc.Name));
                    indexes.Add(index);
                }
            }

            List<ForeignKey> fks = new List<ForeignKey>();
            if (tableDesc.ForeignKeys != null)
            {
                foreach (ForeignKeyDescription fkDesc in tableDesc.ForeignKeys)
                {
                    ForeignKey fk = new ForeignKey(table, fkDesc.Name)
                    {
                        ReferencedTable = fkDesc.ReferencedTableName,
                    };

                    ForeignKeyColumn fkc = new ForeignKeyColumn(fk, fkDesc.ColumnName, fkDesc.ReferencedColumnName);
                    fk.Columns.Add(fkc);
                    fks.Add(fk);
                }
            }

            // TODOTODO - Index Flags

            try
            {
                table.Create();

                foreach (Index index in indexes)
                {
                    index.Create();
                }

                foreach (ForeignKey fk in fks)
                {
                    fk.Create();
                }
            }
            catch (FailedOperationException e)
            {
                throw new SqlDatabaseException(e);
            }
        }

        public void DeleteTable(string tableName)
        {
            string cmdText = String.Format("DROP TABLE [{0}]", tableName);
            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.ExecuteNonQuery();
            }
        }

        public IEnumerable<BulkTableReadResult> BulkRead(IEnumerable<BulkTableReadRequest> requests)
        {
            const string t1Alias = "T1";
            const string t2Alias = "T2";

            int index = 0;
            StringBuilder sb = new StringBuilder(512);

            List<string> tableNames = new List<string>();

            using (SqlCommand cmd = new SqlCommand())
            {
                foreach (BulkTableReadRequest req in requests)
                {
                    if (req.CrossJoin == null)
                    {
                        sb.AppendFormat("SELECT * FROM [{0}] AS [{1}]", req.TableName, t1Alias);
                    }
                    else
                    {
                        sb.AppendFormat("SELECT [{1}].* FROM [{0}] AS [{1}] CROSS JOIN [{3}] AS [{4}] WHERE [{1}].[{2}] = [{4}].[{5}]",
                                        req.TableName, t1Alias, req.CrossJoin.LeftColumnName, req.CrossJoin.TableName, t2Alias, req.CrossJoin.RightColumnName);

                        foreach (BulkTableReadRequestColumn col in req.CrossJoin.Columns)
                        {
                            string param = "@p" + index++;
                            sb.Append(String.Format(" AND [{0}].[{1}] = {2}", t2Alias, col.ColumnName, param));
                            cmd.Parameters.Add(new SqlParameter(param, col.ColumnValue));
                        }
                    }

                    if (req.Columns != null && req.Columns.Count() > 0)
                    {
                        bool first = req.CrossJoin == null;
                        foreach (BulkTableReadRequestColumn col in req.Columns)
                        {
                            string param = "@p" + index++;
                            if (first)
                            {
                                sb.Append(" WHERE");
                                first = false;
                            }
                            else
                            {
                                sb.Append(" AND");
                            }
                            sb.AppendFormat(" [{0}].[{1}] = {2}", t1Alias, col.ColumnName, param);
                            cmd.Parameters.Add(new SqlParameter(param, col.ColumnValue));
                        }
                    }

                    sb.Append(";");
                    tableNames.Add(req.TableName);
                }

                cmd.CommandText = sb.ToString();
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;

                List<BulkTableReadResult> results = new List<BulkTableReadResult>();

                try
                {
                    using (SqlDataReader reader = cmd.ExecuteReader())
                    {
                        int tableIndex = 0;
                        do
                        {
                            List<IDictionary<string, object>> rows = new List<IDictionary<string, object>>();
                            while (reader.Read())
                            {
                                Dictionary<string, object> values = new Dictionary<string, object>();

                                for (int i = 0; i < reader.VisibleFieldCount; i++)
                                {
                                    string name = reader.GetName(i);
                                    object value = reader.GetValue(i);
                                    values.Add(name, value);
                                }

                                rows.Add(values);
                            }

                            BulkTableReadResult result = new BulkTableReadResult()
                            {
                                TableName = tableNames[tableIndex++],
                                Rows = rows
                            };

                            results.Add(result);
                        }
                        while (reader.NextResult());
                    }
                }
                catch (SqlException e)
                {
                    throw new SqlDatabaseException(e);
                }

                return results;
            }
        }

        public void BulkWrite(IEnumerable<BulkTableWriteRequest> tableWriteRequests, string idColumnName)
        {
            // UPDATE [Table]
            // SET [Column1] = @p0, [Column2] = @p1
            // WHERE [Id] = @p2

            StringBuilder cmdText = new StringBuilder();
            int index = 0;
            string param;

            using (SqlCommand cmd = new SqlCommand())
            {
                foreach (BulkTableWriteRequest writeReq in tableWriteRequests)
                {
                    foreach (KeyValuePair<long, IDictionary<string, object>> sparseRow in writeReq.Rows)
                    {
                        cmdText.AppendFormat("UPDATE [{0}] ", writeReq.TableName);

                        bool first = true;
                        foreach (KeyValuePair<string, object> value in sparseRow.Value)
                        {
                            param = "@p" + index++;
                            if (first)
                            {
                                cmdText.AppendFormat("SET [{0}] = {1}", value.Key, param);
                                first = false;
                            }
                            else
                            {
                                cmdText.AppendFormat(", [{0}] = {1}", value.Key, param);
                            }
                            cmd.Parameters.AddWithValue(param, value.Value);
                        }

                        param = "@p" + index++;
                        cmdText.AppendFormat(" WHERE [{0}] = {1}; ", idColumnName, param);
                        cmd.Parameters.AddWithValue(param, sparseRow.Key);
                    }
                }

                cmd.CommandText = cmdText.ToString();
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;

                try
                {
                    cmd.ExecuteNonQuery();
                }
                catch (SqlException e)
                {
                    throw new SqlDatabaseException(e);
                }
            }
        }

        public IList<long> Insert(string tableName, IEnumerable<IEnumerable<InsertValue>> rows)
        {
            // INSERT INTO [Table] VALUES (@p0, @p1, @p2, @p3), (@p4, @p5, @p6, @p7)

            using (SqlCommand cmd = new SqlCommand())
            {
                StringBuilder cmdText = new StringBuilder();

                uint cRows = 0;
                int index = 0;
                foreach (IEnumerable<InsertValue> row in rows)
                {
                    cmdText.AppendFormat("INSERT INTO [{0}] VALUES (", tableName);

                    bool first = true;
                    foreach (InsertValue value in row)
                    {
                        string param = "@p" + index++;
                        if (first)
                        {
                            cmdText.Append(param);
                            first = false;
                        }
                        else
                        {
                            cmdText.AppendFormat(", {0}", param);
                        }

                        cmd.Parameters.Add(new SqlParameter(param, value.Type)
                        {
                            Value = value.Value
                        });
                    }

                    cmdText.Append("); SELECT SCOPE_IDENTITY();");
                    cRows++;
                }

                cmd.CommandText = cmdText.ToString();
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;

                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    long[] ids = new long[cRows];
                    index = 0;
                    do
                    {
                        while (reader.Read())
                        {
                            ids[index++] = (long)(decimal)reader[0];
                        }
                    }
                    while (reader.NextResult());

                    return ids;
                }
            }
        }

        public long GetRowCount(string tableName)
        {
            string cmdText = String.Format("SELECT COUNT_BIG(*) FROM [{0}]", tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                try
                {
                    using (SqlDataReader reader = cmd.ExecuteReader())
                    {
                        reader.Read();
                        return (long)reader[0];
                    }
                }
                catch (SqlException e)
                {
                    throw new SqlDatabaseException(e);
                }
            }
        }

        public IEnumerable<long> Search(string tableName, string idColumnName, long maxResults, long skipResults,
                                        IEnumerable<RangeSearchColumn> rangeCols, IEnumerable<OrderBySearchColumn> orderByCols)
        {
            // TODOTODO - search flags
            // TODOTODO - skip hits

            int index = 0;
            using (SqlCommand cmd = new SqlCommand())
            {
                StringBuilder where = new StringBuilder();

                if (rangeCols != null)
                {
                    bool first = true;
                    foreach (RangeSearchColumn col in rangeCols)
                    {
                        if (first)
                        {
                            where.Append("WHERE ");
                            first = false;
                        }
                        else
                        {
                            where.Append(" AND ");
                        }

                        if (col.LessThanOrEqual == col.GreaterThanOrEqual)
                        {
                            string param = "@p" + index++;
                            where.AppendFormat("[{0}] = {1}", col.ColumnName, param);
                            cmd.Parameters.Add(new SqlParameter(param, col.LessThanOrEqual));
                        }
                        else
                        {
                            string lessThanParam = "@p" + index++;
                            string greaterThanParam = "@p" + index++;

                            where.AppendFormat("[{0}] >= {1} AND [{0}] <= {2}", col.ColumnName, greaterThanParam, lessThanParam);

                            cmd.Parameters.Add(new SqlParameter(lessThanParam, col.LessThanOrEqual));
                            cmd.Parameters.Add(new SqlParameter(greaterThanParam, col.GreaterThanOrEqual));
                        }
                    }
                }

                StringBuilder orderOrGroupBy = new StringBuilder();
                if (orderByCols != null)
                {
                    bool first = true;
                    foreach (OrderBySearchColumn orderByCol in orderByCols)
                    {
                        if (first)
                        {
                            first = false;
                            orderOrGroupBy.AppendFormat("ORDER BY [{0}]", orderByCol.ColumnName);
                        }
                        else
                        {
                            orderOrGroupBy.AppendFormat(",[{0}]", orderByCol.ColumnName);
                        }
                        orderOrGroupBy.Append(orderByCol.Ascending ? " ASC" : " DESC");
                    }
                }
                else
                {
                    orderOrGroupBy.AppendFormat("GROUP BY [{0}]", idColumnName);
                }

                string cmdText;
                if (maxResults == Int32.MaxValue)
                {
                    cmdText = String.Format("SELECT [{0}] FROM [{1}] {2} {3};", idColumnName, tableName, where.ToString(), orderOrGroupBy.ToString());
                }
                else
                {
                    cmdText = String.Format("SELECT TOP({0}) [{1}] FROM [{2}] {3} {4};", maxResults, idColumnName, tableName, where.ToString(), orderOrGroupBy.ToString());
                }

                cmd.CommandText = cmdText;
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;

                try
                {
                    using (SqlDataReader reader = cmd.ExecuteReader())
                    {
                        List<long> ids = new List<long>();
                        while (reader.Read())
                        {
                            ids.Add((long)reader[0]);
                        }
                        return ids;
                    }
                }
                catch (SqlException e)
                {
                    throw new SqlDatabaseException(e);
                }
            }
        }

        public void DeleteRow(string tableName, string columnName, object value)
        {
            string cmdText = String.Format("DELETE FROM [{0}] WHERE [{1}] = @p0", tableName, columnName);
            try
            {
                using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
                {
                    cmd.Parameters.Add(new SqlParameter("@p0", value));
                    cmd.ExecuteNonQuery();
                }
            }
            catch (SqlException e)
            {
                throw new SqlDatabaseException(e);
            }
        }

        public void DeleteRows(string tableName, string columnName, IEnumerable<long> values)
        {
            if (values.Count() == 0)
                return;

            StringBuilder cmdText = new StringBuilder();
            cmdText.AppendFormat("DELETE FROM [{0}]", tableName);

            using (SqlCommand cmd = new SqlCommand())
            {
                int index = 0;
                bool first = true;
                foreach (object value in values)
                {
                    string param = "@p" + index++;
                    if (first)
                    {
                        first = false;
                        cmdText.Append(" WHERE");
                    }
                    else
                    {
                        cmdText.Append(" OR");
                    }
                    cmdText.AppendFormat(" [{0}] = {1}", columnName, param);
                    cmd.Parameters.Add(new SqlParameter(param, value));
                }

                cmd.CommandText = cmdText.ToString();
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;
                cmd.ExecuteNonQuery();
            }
        }

        public void DeleteAllRows(string tableName)
        {
            string cmdText = String.Format("DELETE FROM [{0}]", tableName);
            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.ExecuteNonQuery();
            }
        }
    }
}
