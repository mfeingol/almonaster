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

    public enum SearchType
    {
        RangeInclusive,
        Equality,
        ContainsString,
        BeginsWithString,
        EndsWithString
    }

    public struct SearchColumnMetadata
    {
        public string ColumnName;
        public SearchType Type;
        public object Field1;
        public object Field2;
    }

    public struct OrderBySearchColumn
    {
        public string ColumnName;
        public bool Ascending;
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

    public class CrossJoinRequest
    {
        public string TableName;
        public string LeftColumnName;
        public string RightColumnName;
        public IEnumerable<BulkTableReadRequestColumn> Columns;
    }

    public struct BulkTableReadRequest
    {
        public string TableName;
        public IEnumerable<BulkTableReadRequestColumn> Columns;
        public CrossJoinRequest CrossJoin;
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
