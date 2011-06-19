﻿using System;
using System.Linq;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;

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

            this.conn.Dispose();
            this.conn = null;

            if (this.tx != null)
            {
                this.tx.Dispose();
                this.tx = null;
            }

            GC.SuppressFinalize(this);
        }

        public void SetComplete()
        {
            this.setComplete = true;
        }

        internal bool CreateDatabaseIfNecessary()
        {
            SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder(this.conn.ConnectionString);
            string databaseName = builder.InitialCatalog;

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
            SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder(this.conn.ConnectionString);
            ServerConnection serverConn = new ServerConnection(this.conn);
            Server server = new Server(serverConn);
            var database = server.Databases[builder.InitialCatalog];

            return database.Tables.Contains(tableName);
        }

        public void CreateTable(TableDescription tableDesc)
        {
            SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder(this.conn.ConnectionString);
            ServerConnection serverConn = new ServerConnection(this.conn);
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

        public long Insert(string tableName, IEnumerable<ColumnValue> columns)
        {
            //INSERT INTO @TableName (Col0, Col1, Col2, Col3)
            //VALUES (@p0, @p1, @p2, @p3);

            using (SqlCommand cmd = new SqlCommand())
            {
                cmd.Connection = this.conn;
                cmd.Transaction = this.tx;

                string insert = String.Format("INSERT INTO [{0}] ([{1}]", tableName, columns.First().Name);
                string values = "VALUES (@p0";

                int index = 0;
                foreach (ColumnValue col in columns)
                {
                    string param = "@p" + index;

                    if (index > 0)
                    {
                        insert += ", [" + col.Name + "]";
                        values += ", " + param;
                    }

                    SqlParameter value = new SqlParameter(param, col.Type)
                    {
                        Value = col.Value
                    };
                    cmd.Parameters.Add(value);

                    index++;
                }

                insert += ")";
                values += ");";

                cmd.CommandText = insert + " " + values + " SELECT SCOPE_IDENTITY();";

                object id = cmd.ExecuteScalar();
                return (long)(decimal)id;
            }
        }

        public long GetRowCount(string tableName)
        {
            string cmdText = String.Format("SELECT COUNT_BIG(*) FROM [{0}]", tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
            {
                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    reader.Read();
                    return (long)reader[0];
                }
            }
        }

        public bool DoesRowExist(string tableName, string idColumnName, long id)
        {
            string cmdText = String.Format("SELECT [{0}] FROM [{1}] WHERE [{0}] = @p0", idColumnName, tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
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

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
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

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
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

        public object ReadSingleData(string tableName, string columnName)
        {
            string cmdText = String.Format("SELECT [{0}] FROM [{1}]", columnName, tableName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
            {
                using (SqlDataReader reader = cmd.ExecuteReader())
                {
                    if (!reader.Read())
                    {
                        return null;
                    }
                    object ret = reader[0];
                    if (reader.Read())
                    {
                        throw new SqlDatabaseException("Table contains more rows than expected");
                    }
                    return ret;
                }
            }
        }

        public object ReadData(string tableName, string idColumnName, long id, string columnName)
        {
            string cmdText = String.Format("SELECT [{0}] FROM [{1}] WHERE [{2}] = @p0", columnName, tableName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
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

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));
                cmd.Parameters.Add(new SqlParameter("@p1", bitField));

                cmd.ExecuteNonQuery();
            }
        }

        public void WriteData(string tableName, string idColumnName, long id, string columnName, object value)
        {
            string cmdText = String.Format("UPDATE [{0}] SET [{1}] = @p1 WHERE [{2}] = @p0", tableName, columnName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));
                cmd.Parameters.Add(new SqlParameter("@p1", value));

                cmd.ExecuteNonQuery();
            }
        }

        public void WriteSingleData(string tableName, string columnName, object value)
        {
            string cmdText = String.Format("UPDATE [{0}] SET [{1}] = @p0", tableName, columnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", value));

                cmd.ExecuteNonQuery();
            }
        }

        public void Increment(string tableName, string idColumnName, long id, string columnName, object inc)
        {
            string cmdText = String.Format("UPDATE [{0}] SET [{1}] = [{1}] + @p1 WHERE [{2}] = @p0", tableName, columnName, idColumnName);

            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
            {
                cmd.Parameters.Add(new SqlParameter("@p0", id));
                cmd.Parameters.Add(new SqlParameter("@p1", inc));

                cmd.ExecuteNonQuery();
            }
        }

        public IEnumerable<RowValues> ReadColumns(string tableName, IEnumerable<string> columnNames)
        {
            int cols = 0;
            string columnSet = String.Empty;
            foreach (string columnName in columnNames)
            {
                if (!String.IsNullOrEmpty(columnSet))
                {
                    columnSet += ", ";
                }
                columnSet += "[" + columnName + "]";
                cols ++;
            }

            List<RowValues> rows = new List<RowValues>();
            
            string cmdText = String.Format("SELECT {0} FROM {1}", columnSet, tableName);
            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
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

            return rows;
        }

        public void DeleteAllRows(string tableName)
        {
            string cmdText = String.Format("DELETE FROM [{0}]", tableName);
            using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
            {
                cmd.ExecuteNonQuery();
            }
        }

        //public SqlDataReader ReadColumn(string tableName, string columnName)
        //{
        //    string cmdText = String.Format("SELECT [{0}] FROM {1}", columnName, tableName);

        //    using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
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

        //    using (SqlCommand cmd = new SqlCommand(cmdText, this.conn))
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
        //        cmd.CommandText = "SELECT * FROM " + tableName + " WHERE [" + colName + "] = @p0";
        //        cmd.Parameters.Add(new SqlParameter("@p0", equals));

        //        SqlDataReader reader = cmd.ExecuteReader();
        //        reader.Read();
        //        return reader;
        //    }
        //}
    }
}
