using System;
using System.Collections.Generic;
using System.Data;

namespace Almonaster.Database.Sql
{
    public struct ColumnDescription
    {
        public string Name;
        public SqlDbType Type;
        public int Size;
        public bool IsPrimaryKey;
    }

    public struct TableDescription
    {
        public string Name;
        public IEnumerable<ColumnDescription> Columns;
        public IEnumerable<string> IndexColumns;
        public IEnumerable<ForeignKeyDescription> ForeignKeys;
    }

    public struct ForeignKeyDescription
    {
        public string Name;
        public string ReferencedTableName;
        public string ColumnName;
        public string ReferencedColumnName;
    }

    public struct ColumnValue
    {
        public string Name;
        public SqlDbType Type;
        public object Value;
    }

    public struct ColumnSearchDescription
    {
        public string Name;
        public object GreaterThanOrEqual;
        public object LessThanOrEqual;
    }

    public struct RowValues
    {
        public IEnumerable<object> Values;
    }

    // ...

    public struct BulkTableReadRequest
    {
        public string TableName;
        public string ColumnName;
        public object ColumnValue;
    }

    public class BulkTableReadResult
    {
        public string TableName;
        public IEnumerable<IDictionary<string, object>> Rows;
    }
}
