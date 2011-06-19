using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;

using Microsoft.SqlServer.Management.Smo;

namespace Almonaster.Database.Sql
{
    public class SqlDatabase
    {
        string connString;

        public SqlDatabase(string connString)
        {
            this.connString = connString;
        }

        public bool CreateIfNecessary()
        {
            using (var cmd = CreateCommandManager())
            {
                return cmd.CreateDatabaseIfNecessary();
            }
        }

        public SqlCommandManager CreateCommandManager()
        {
            return CreateCommandManager(IsolationLevel.Unspecified);
        }

        public SqlCommandManager CreateCommandManager(IsolationLevel isoLevel)
        {
            return new SqlCommandManager(this.connString, isoLevel);
        }
    }
}
