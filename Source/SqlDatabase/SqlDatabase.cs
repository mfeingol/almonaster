using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;

namespace Almonaster.Database.Sql
{
    public class SqlDatabase
    {
        string connString;

        public SqlDatabase(string connString)
        {
            this.connString = connString;
        }

        public void CreateIfNecessary()
        {
            SqlConnectionStringBuilder builder = new SqlConnectionStringBuilder(this.connString);
            string databaseName = builder.InitialCatalog;
            builder.InitialCatalog = String.Empty;

            using (var cmd = CreateCommandManager(builder.ToString(), IsolationLevel.Unspecified))
            {
                cmd.CreateDatabaseIfNecessary(databaseName);
            }
        }

        public SqlCommandManager CreateCommandManager()
        {
            return CreateCommandManager(this.connString, IsolationLevel.Unspecified);
        }

        public SqlCommandManager CreateCommandManager(IsolationLevel isoLevel)
        {
            return new SqlCommandManager(this.connString, isoLevel);
        }

        SqlCommandManager CreateCommandManager(string connString, IsolationLevel isoLevel)
        {
            return new SqlCommandManager(connString, isoLevel);
        }
    }
}
