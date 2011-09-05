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

        // ...

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
                        string param = "@p" + index ++;
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

        public bool DoesRowExist(string tableName, string idColumnName, long id)
        {
            string cmdText = String.Format("SELECT [{0}] FROM [{1}] WHERE [{0}] = @p0", idColumnName, tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));

                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    return reader.Read();
                }
            }
        }

        public long GetIdOfFirstMatch(string tableName, string idColumnName, string columnName, object data)
        {
            string cmdText = String.Format("SELECT TOP(1) [{0}] FROM [{1}] WHERE [{2}] = @p0 GROUP BY [{0}]", idColumnName, tableName, columnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", data));

                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    if (!reader.Read())
                    {
                        return long.MinValue;
                    }
                    return (long)reader[0];
                }
            }
        }

        public long GetNextId(string tableName, string idColumnName, long id)
        {
            string cmdText = String.Format("SELECT TOP(1) [{0}] FROM [{1}] WHERE [{2}] > @p0 GROUP BY [{0}]", idColumnName, tableName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));

                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    if (!reader.Read())
                    {
                        return long.MinValue;
                    }
                    return (long)reader[0];
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

        public long SearchCount(string tableName, IEnumerable<RangeSearchColumn> rangeSearchCols)
        {
            // TODOTODO - refactor this with Search()
            // TODOTODO - search flags

            string where = String.Empty;
            uint index = 0;

            using (SqlCommand cmd = new SqlCommand())
            {
                foreach (RangeSearchColumn col in rangeSearchCols)
                {
                    if (String.IsNullOrEmpty(where))
                    {
                        where = "WHERE ";
                    }
                    else
                    {
                        where += " AND ";
                    }

                    if (col.LessThanOrEqual == col.GreaterThanOrEqual)
                    {
                        string param = "@p" + index++;
                        where += String.Format("[{0}] = {1}", col.ColumnName, param);
                        cmd.Parameters.Add(new SqlParameter(param, col.LessThanOrEqual));
                    }
                    else
                    {
                        string lessThanParam = "@p" + index++;
                        string greaterThanParam = "@p" + index++;

                        where += String.Format("[{0}] >= {1} AND [{0}] <= {2}", col.ColumnName, greaterThanParam, lessThanParam);

                        cmd.Parameters.Add(new SqlParameter(lessThanParam, col.LessThanOrEqual));
                        cmd.Parameters.Add(new SqlParameter(greaterThanParam, col.GreaterThanOrEqual));
                    }
                }

                // TODOTODO - DIFFERENCES BEGIN HERE
                string cmdText = String.Format("SELECT COUNT_BIG(*) FROM [{0}] {1}", tableName, where);

                cmd.CommandText = cmdText;
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;

                try
                {
                    return (long)GetSingleResult(cmd);
                }
                catch (SqlException e)
                {
                    throw new SqlDatabaseException(e);
                }
            }
        }

        public object ReadSingle(string tableName, string columnName)
        {
            string cmdText = String.Format("SELECT [{0}] FROM [{1}]", columnName, tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                return GetSingleResult(cmd);
            }
        }

        public object Read(string tableName, string idColumnName, long id, string columnName)
        {
            string cmdText = String.Format("SELECT [{0}] FROM [{1}] WHERE [{2}] = @p0", columnName, tableName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));

                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    if (!reader.Read())
                    {
                        return null;
                    }
                    return reader[0];
                }
            }
        }

        public void WriteBitField(string tableName, string idColumnName, long id, string columnName, BooleanOperation op, int bitField)
        {
            string opString;
            switch (op)
            {
                case BooleanOperation.And:
                    opString = "&";
                    break;
                case BooleanOperation.Or:
                    opString = "|";
                    break;
                default:
                    throw new ArgumentException("Invalid op");
            }

            string cmdText = String.Format("UPDATE [{0}] SET [{1}] = [{1}] {2} @p1 WHERE [{3}] = @p0", tableName, columnName, opString, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));
                cmd.Parameters.Add(new SqlParameter("@p1", bitField));
                cmd.ExecuteNonQuery();
            }
        }

        public void WriteBitField(string tableName, string columnName, BooleanOperation op, int bitField)
        {
            string opString;
            switch (op)
            {
                case BooleanOperation.And:
                    opString = "&";
                    break;
                case BooleanOperation.Or:
                    opString = "|";
                    break;
                default:
                    throw new ArgumentException("Invalid op");
            }

            string cmdText = String.Format("UPDATE [{0}] SET [{1}] = [{1}] {2} @p0", tableName, columnName, opString);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", bitField));
                cmd.ExecuteNonQuery();
            }
        }

        public void Write(string tableName, string idColumnName, long id, string columnName, object value)
        {
            string cmdText = String.Format("UPDATE [{0}] SET [{1}] = @p1 WHERE [{2}] = @p0", tableName, columnName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));
                cmd.Parameters.Add(new SqlParameter("@p1", value));
                cmd.ExecuteNonQuery();
            }
        }

        public void WriteSingle(string tableName, string columnName, object value)
        {
            string cmdText = String.Format("UPDATE [{0}] SET [{1}] = @p0", tableName, columnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", value));
                cmd.ExecuteNonQuery();
            }
        }

        public void Increment(string tableName, string idColumnName, long id, string columnName, object inc)
        {
            string cmdText = String.Format("UPDATE [{0}] SET [{1}] = [{1}] + @p1 WHERE [{2}] = @p0", tableName, columnName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));
                cmd.Parameters.Add(new SqlParameter("@p1", inc));
                cmd.ExecuteNonQuery();
            }
        }

        public void Increment(string tableName, string idColumnName, long id, string columnName, object inc, out object oldValue)
        {
            string cmdText = String.Format("SELECT [{1}] FROM [{0}] WHERE [{2}] = @p0; UPDATE [{0}] SET [{1}] = [{1}] + @p1 WHERE [{2}] = @p0", tableName, columnName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));
                cmd.Parameters.Add(new SqlParameter("@p1", inc));

                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    if (reader.Read())
                    {
                        oldValue = reader.GetValue(0);
                    }
                    else
                    {
                        throw new SqlDatabaseException("No value returned from command execution");
                    }
                }
            }
        }

        public object IncrementSingle(string tableName, string columnName, object inc)
        {
            string cmdText = String.Format("SELECT [{1}] FROM [{0}]; UPDATE [{0}] SET [{1}] = [{1}] + @p0", tableName, columnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", inc));
                return GetSingleResult(cmd);
            }
        }

        public IEnumerable<RowValues> ReadColumns(string tableName, IEnumerable<string> columnNames)
        {
            int cols;
            string columnSet = MakeColumnSet(columnNames, out cols);
            List<RowValues> rows = new List<RowValues>();
            
            string cmdText = String.Format("SELECT {0} FROM [{1}]", columnSet, tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                try
                {
                    using (SqlDataReader reader = cmd.ExecuteReader())
                    {
                        while (reader.Read())
                        {
                            List<object> values = new List<object>(cols);
                            foreach (string columnName in columnNames)
                            {
                                values.Add(reader[columnName]);
                            }
                            RowValues row;
                            row.Values = values;
                            rows.Add(row);
                        }
                    }
                }
                catch (SqlException e)
                {
                    throw new SqlDatabaseException(e);
                }
            }

            return rows;
        }

        public RowValues ReadRow(string tableName, string idColumnName, long id)
        {
            string cmdText = String.Format("SELECT * FROM [{0}] WHERE [{1}] = @p0", tableName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));

                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    RowValues row = new RowValues();
                    if (reader.Read())
                    {
                        int cols = reader.VisibleFieldCount;
                        object[] values = new object[cols];
                        row.Values = values;

                        for (int i = 0; i < cols; i++)
                        {
                            values[i] = reader[i];
                        }
                    }
                    return row;
                }
            }
        }

        public RowValues ReadRow(string tableName)
        {
            string cmdText = String.Format("SELECT * FROM [{0}]", tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    RowValues row = new RowValues();
                    if (reader.Read())
                    {
                        int cols = reader.VisibleFieldCount;
                        object[] values = new object[cols];
                        reader.GetValues(values);
                        row.Values = values;

                        if (reader.Read())
                        {
                            throw new SqlDatabaseException("Table contains more rows than expected");
                        }
                    }
                    return row;
                }
            }
        }

        public IEnumerable<RowValues> ReadColumnsWhere(string tableName, IEnumerable<string> columnNames, string equalColumnName, object value)
        {
            int cols;
            string columnSet = MakeColumnSet(columnNames, out cols);

            List<RowValues> rows = new List<RowValues>();

            string cmdText = String.Format("SELECT {0} FROM [{1}] WHERE [{2}] = @p0", columnSet, tableName, equalColumnName);
            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", value));

                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    while (reader.Read())
                    {
                        List<object> values = new List<object>(cols);
                        foreach (string columnName in columnNames)
                        {
                            values.Add(reader[columnName]);
                        }
                        RowValues row;
                        row.Values = values;
                        rows.Add(row);
                    }
                }
            }

            return rows;
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

        //public SqlDataReader ReadColumn(string tableName, string columnName)
        //{
        //    string cmdText = String.Format("SELECT [{0}] FROM {1}", columnName, tableName);

        //    using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
        //    {
        //        SqlDataReader reader = cmd.ExecuteReader();
        //        reader.Read();
        //        return reader;
        //    }
        //}

        //public SqlDataReader ReadRowsFromJoinWhere(string tableName1, string joinColumnName1, string tableName2, string joinColumnName2, string columnName, object data)
        //{
        //    string cmdText = String.Format("SELECT * FROM {0} JOIN {2} ON {0}.{1} = {2}.{3} WHERE [{4}] = @p0",
        //                                   tableName1, joinColumnName1, tableName2, joinColumnName2, columnName);

        //    using (SqlCommand cmd = new SqlCommand(cmdText, this.conn, this.tx))
        //    {
        //        cmd.Parameters.Add(new SqlParameter("@p0", data));

        //        SqlDataReader reader = cmd.ExecuteReader();
        //        reader.Read();
        //        return reader;
        //    }
        //}

        //public SqlDataReader ReadRowWhere(string tableName, string colName, object equals)
        //{
        //    using (SqlCommand cmd = new SqlCommand())
        //    {
        //        cmd.Connection = this.conn;
        //        cmd.Transaction = this.tx;
        //        cmd.CommandText = "SELECT * FROM " + tableName + " WHERE [" + colName + "] = @p0";
        //        cmd.Parameters.Add(new SqlParameter("@p0", equals));

        //        SqlDataReader reader = cmd.ExecuteReader();
        //        reader.Read();
        //        return reader;
        //    }
        //}

        //
        // Helper methods
        //

        object GetSingleResult(SqlCommand cmd)
        {
            using (SqlDataReader reader = cmd.ExecuteReader())
            {
                if (!reader.Read())
                {
                    return null;
                }
                object result = reader[0];
                if (reader.Read())
                {
                    throw new SqlDatabaseException("Table contains more rows than expected");
                }
                return result;
            }
        }

        string MakeColumnSet(IEnumerable<string> columnNames, out int cols)
        {
            cols = 0;
            string columnSet = String.Empty;
            foreach (string columnName in columnNames)
            {
                if (!String.IsNullOrEmpty(columnSet))
                {
                    columnSet += ", ";
                }
                columnSet += "[" + columnName + "]";
                cols++;
            }

            return columnSet;
        }
    }
}
