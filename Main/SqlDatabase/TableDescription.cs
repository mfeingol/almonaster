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

    public struct InsertValue
    {
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

    public struct BulkTableReadRequestColumn
    {
        public string ColumnName;
        public object ColumnValue;
    }

    public struct BulkTableReadRequest
    {
        public string TableName;
        public IEnumerable<BulkTableReadRequestColumn> Columns;
    }

    public class BulkTableReadResult
    {
        public string TableName;
        public List<IDictionary<string, object>> Rows;
    }

    public struct BulkTableWriteRequest
    {
        public string TableName;
        public IDictionary<long, IDictionary<string, object>> Rows;
    }
}
