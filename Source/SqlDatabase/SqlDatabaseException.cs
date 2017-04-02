using System;
using System.Data.SqlClient;

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
    }
}
