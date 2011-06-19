using System;
using System.Data.SqlClient;

using Microsoft.SqlServer.Management.Common;
using Microsoft.SqlServer.Management.Smo;

namespace Almonaster.Database.Sql
{
    public class SqlDatabaseException : Exception
    {
        public SqlDatabaseException(string message)
            : base(message)
        {
        }

        public SqlDatabaseException(SqlException e) 
            : base(e.Message, e)
        {
        }

        public SqlDatabaseException(ConnectionException e) 
            : base (e.Message, e)
        {
        }

        public SqlDatabaseException(SmoException e) 
            : base(e.Message, e)
        {
        }
    }
}
