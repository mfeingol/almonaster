using System;
using System.Data;

using Microsoft.SqlServer.Management.Smo;

namespace Almonaster.Database.Sql
{
    static class TypeMap
    {
        public static DataType Convert(SqlDbType sqlDbType, int size)
        {
            SqlDataType sqlDataType = Convert(sqlDbType);
            switch (sqlDataType)
            {
                case SqlDataType.NVarChar:
                    return size == Int32.MaxValue ? DataType.NVarCharMax : DataType.NVarChar(size);
                default:
                    return new DataType(sqlDataType);
            }
        }

        public static SqlDataType Convert(SqlDbType sqlDbType)
        {
            switch (sqlDbType)
            {
                case SqlDbType.BigInt:
                    return SqlDataType.BigInt;
                case SqlDbType.Binary:
                    return SqlDataType.Binary;
                case SqlDbType.Bit:
                    return SqlDataType.Bit;
                case SqlDbType.Char:
                    return SqlDataType.Char;
                case SqlDbType.DateTime:
                    return SqlDataType.DateTime;
                case SqlDbType.Decimal:
                    return SqlDataType.Decimal;
                case SqlDbType.Float:
                    return SqlDataType.Float;
                case SqlDbType.Image:
                    return SqlDataType.Image;
                case SqlDbType.Int:
                    return SqlDataType.Int;
                case SqlDbType.Money:
                    return SqlDataType.Money;
                case SqlDbType.NChar:
                    return SqlDataType.NChar;
                case SqlDbType.NText:
                    return SqlDataType.NText;
                case SqlDbType.NVarChar:
                    return SqlDataType.NVarChar;
                case SqlDbType.Real:
                    return SqlDataType.Real;
                case SqlDbType.UniqueIdentifier:
                    return SqlDataType.UniqueIdentifier;
                case SqlDbType.SmallDateTime:
                    return SqlDataType.SmallDateTime;
                case SqlDbType.SmallInt:
                    return SqlDataType.SmallInt;
                case SqlDbType.SmallMoney:
                    return SqlDataType.SmallMoney;
                case SqlDbType.Text:
                    return SqlDataType.Text;
                case SqlDbType.Timestamp:
                    return SqlDataType.Timestamp;
                case SqlDbType.TinyInt:
                    return SqlDataType.TinyInt;
                case SqlDbType.VarBinary:
                    return SqlDataType.VarBinary;
                case SqlDbType.VarChar:
                    return SqlDataType.VarChar;
                case SqlDbType.Variant:
                    return SqlDataType.Variant;
                case SqlDbType.Xml:
                    return SqlDataType.Xml;
                case SqlDbType.Udt:
                    return SqlDataType.UserDefinedDataType;
                case SqlDbType.Date:
                    return SqlDataType.Date;
                case SqlDbType.Time:
                    return SqlDataType.Time;
                case SqlDbType.DateTime2:
                    return SqlDataType.DateTime2;
                case SqlDbType.DateTimeOffset:
                    return SqlDataType.DateTimeOffset;
                case SqlDbType.Structured:
                    Invariant.Assert(false, "Unsupported SqlDbType");
                    break;
                default:
                    Invariant.Assert(false, "Unknown SqlDbType");
                    break;
            }

            return SqlDataType.Xml; // Keep the compiler happy
        }
    }
}
