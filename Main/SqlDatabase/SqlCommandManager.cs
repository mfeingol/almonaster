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
            // TODO - 610 - Rewrite CreateTable to use TSQL, be transactional
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
                    Identity = colDesc.IndexType == IndexType.PrimaryKey
                };

                table.Columns.Add(col);

                Index index = null;
                switch (colDesc.IndexType)
                {
                    case IndexType.None:
                        break;

                    case IndexType.Index:
                        index = new Index(table, String.Format("{0}_{1}_idx", tableDesc.Name, colDesc.Name))
                        {
                            IndexKeyType = IndexKeyType.None,
                            IsClustered = false,
                            IsUnique = false,
                        };
                        break;

                    case IndexType.IndexUnique:
                        index = new Index(table, String.Format("{0}_{1}_idx_Unique", tableDesc.Name, colDesc.Name))
                        {
                            IndexKeyType = IndexKeyType.None,
                            IsClustered = false,
                            IsUnique = true,
                        };
                        break;

                    case IndexType.PrimaryKey:
                        index = new Index(table, String.Format("{0}_{1}_PK", tableDesc.Name, colDesc.Name))
                        {
                            IndexKeyType = IndexKeyType.DriPrimaryKey,
                            IsClustered = true,
                            IsUnique = true,
                        };
                        break;

                    default:
                        throw new InvalidArgumentException();
                }

                if (index != null)
                {
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

        public IEnumerable<object> Search(string tableName, string selectColumnName, uint maxResults, uint skipResults,
                                          IEnumerable<SearchColumnMetadata> searchCols, IEnumerable<OrderBySearchColumn> orderByCols)
        {
            int index = 0;
            using (SqlCommand cmd = new SqlCommand())
            {
                StringBuilder selectTop = new StringBuilder();
                if (maxResults == 0)
                {
                    selectTop.Append(selectColumnName);
                }
                else
                {
                    selectTop.AppendFormat("TOP({0}) [{1}]", maxResults, selectColumnName);
                }

                StringBuilder orderBy = new StringBuilder();
                if (orderByCols != null)
                {
                    bool first = true;
                    foreach (OrderBySearchColumn orderByCol in orderByCols)
                    {
                        if (first)
                        {
                            first = false;
                            orderBy.AppendFormat("ORDER BY [{0}]", orderByCol.ColumnName);
                        }
                        else
                        {
                            orderBy.AppendFormat(",[{0}]", orderByCol.ColumnName);
                        }
                        orderBy.Append(orderByCol.Ascending ? " ASC" : " DESC");
                    }
                }
                else if (searchCols.Count() > 0)
                {
                    orderBy.AppendFormat("ORDER BY [{0}]", searchCols.First().ColumnName);
                }
                else
                {
                    orderBy.AppendFormat("ORDER BY [{0}]", selectColumnName);
                }

                StringBuilder where = new StringBuilder();
                if (searchCols != null)
                {
                    bool first = true;
                    foreach (SearchColumnMetadata col in searchCols)
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

                        string param = "@p" + index++;
                        cmd.Parameters.Add(new SqlParameter(param, col.Field1));

                        switch (col.Type)
                        {
                            case SearchType.RangeInclusive:
                                string lessThanParam = "@p" + index++;
                                cmd.Parameters.Add(new SqlParameter(lessThanParam, col.Field1));
                                where.AppendFormat("[{0}] >= {1} AND [{0}] <= {2}", col.ColumnName, lessThanParam, param);
                                break;

                            case SearchType.Equality:
                                where.AppendFormat("[{0}] = {1}", col.ColumnName, param);
                                break;

                            case SearchType.ContainsString:
                                where.AppendFormat("[{0}] LIKE '%' + {1} + '%'", col.ColumnName, param);
                                break;

                            case SearchType.BeginsWithString:
                                where.AppendFormat("[{0}] LIKE {1} + '%'", col.ColumnName, param);
                                break;

                            case SearchType.EndsWithString:
                                where.AppendFormat("[{0}] LIKE '%' + {1}", col.ColumnName, param);
                                break;

                            default:
                                throw new InvalidArgumentException();
                        }
                    }
                }

                StringBuilder tableNameWithSkip = new StringBuilder();
                if (skipResults == 0)
                {
                    tableNameWithSkip.AppendFormat("[{0}]", tableName);
                }
                else
                {
                    tableNameWithSkip.AppendFormat("(SELECT row_number() OVER ({0}) AS row_number, * FROM [{1}] {2}) t1 WHERE row_number > {3}",
                                                    orderBy, tableName, where, skipResults);

                    orderBy = null;
                    where = null;
                }

                cmd.CommandText = String.Format("SELECT {0} FROM {1} {2} {3};", selectTop, tableNameWithSkip, where, orderBy);
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;
                try
                {
                    using (SqlDataReader reader = cmd.ExecuteReader())
                    {
                        List<object> results = new List<object>();
                        while (reader.Read())
                        {
                            results.Add(reader[0]);
                        }
                        return results;
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

        public void WriteRecord(string tableName, IEnumerable<BulkTableReadRequestColumn> matchColumns, string columnName, object value)
        {
            using (SqlCommand cmd = new SqlCommand())
            {
                StringBuilder cmdText = new StringBuilder();
                cmdText.AppendFormat("UPDATE [{0}] SET [{1}] = @p0", tableName, columnName);
                cmd.Parameters.Add(new SqlParameter("@p0", value));

                int index = 1;
                bool first = true;
                foreach (BulkTableReadRequestColumn col in matchColumns)
                {
                    string param = "@p" + index++;
                    if (first)
                    {
                        first = false;
                        cmdText.AppendFormat(" WHERE [{0}] = {1}", col.ColumnName, param);
                    }
                    else
                    {
                        cmdText.AppendFormat(" AND [{0}] = {1}", col.ColumnName, param);
                    }
                    cmd.Parameters.Add(new SqlParameter(param, col.ColumnValue));
                }

                cmd.CommandText = cmdText.ToString();
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;
                cmd.ExecuteNonQuery();
            }
        }
    }
}
