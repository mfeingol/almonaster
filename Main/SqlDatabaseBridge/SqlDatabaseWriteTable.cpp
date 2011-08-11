#include "SqlDatabaseWriteTable.h"
#include "Utils.h"

SqlDatabaseWriteTable::SqlDatabaseWriteTable(SqlCommandManager^ cmd, System::String^ tableName)
    : m_readTable(cmd, tableName)
{
    m_iNumRefs = 1;

    m_cmd = cmd;
    m_tableName = tableName;
}

SqlDatabaseWriteTable::~SqlDatabaseWriteTable()
{
}

int SqlDatabaseWriteTable::WriteData(unsigned int iKey, const char* pszColumn, int iData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteData(unsigned int iKey, const char* pszColumn, float fData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteData(unsigned int iKey, const char* pszColumn, const char* pszData)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("WriteData {0} :: {1}", m_tableName, columnName);

    m_cmd->Write(m_tableName, gcnew System::String(IdColumnName), iKey, columnName, gcnew System::String(pszData));
    return OK;
}

int SqlDatabaseWriteTable::WriteData(unsigned int iKey, const char* pszColumn, int64 i64Data)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("WriteData {0} :: {1}", m_tableName, columnName);

    m_cmd->Write(m_tableName, gcnew System::String(IdColumnName), iKey, columnName, i64Data);
    return OK;
}

int SqlDatabaseWriteTable::WriteData(unsigned int iKey, const char* pszColumn, const Variant& vData)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("WriteData {0} :: {1}", m_tableName, columnName);

    m_cmd->Write(m_tableName, gcnew System::String(IdColumnName), iKey, columnName, Convert(vData));
    return OK;
}

int SqlDatabaseWriteTable::WriteData(const char* pszColumn, int iData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteData(const char* pszColumn, float fData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteData(const char* pszColumn, const char* pszData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteData(const char* pszColumn, int64 i64Data)
{
    Variant vData = i64Data;
    return WriteData(pszColumn, vData);
}

int SqlDatabaseWriteTable::WriteData(const char* pszColumn, const Variant& vData)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("WriteData {0} :: {1}", m_tableName, columnName);

    m_cmd->WriteSingle(m_tableName, columnName, Convert(vData));
    return OK;
}

int SqlDatabaseWriteTable::WriteAnd(unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("WriteAnd {0} :: {1}", m_tableName, columnName);

    m_cmd->WriteBitField(m_tableName, gcnew System::String(IdColumnName), iKey, columnName, BooleanOperation::And, iBitField);
    return OK;
}

int SqlDatabaseWriteTable::WriteAnd(const char* pszColumn, unsigned int iBitField)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("WriteAnd {0} :: {1}", m_tableName, columnName);

    m_cmd->WriteBitField(m_tableName, columnName, BooleanOperation::And, iBitField);
    return OK;
}

int SqlDatabaseWriteTable::WriteOr(unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("WriteOr {0} :: {1}", m_tableName, columnName);

    m_cmd->WriteBitField(m_tableName, gcnew System::String(IdColumnName), iKey, columnName, BooleanOperation::Or, iBitField);
    return OK;
}

int SqlDatabaseWriteTable::WriteOr(const char* pszColumn, unsigned int iBitField)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("WriteOr {0} :: {1}", m_tableName, columnName);

    m_cmd->WriteBitField(m_tableName, columnName, BooleanOperation::Or, iBitField);
    return OK;
}

int SqlDatabaseWriteTable::WriteXor(unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteXor(const char* pszColumn, unsigned int iBitField)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteNot(unsigned int iKey, const char* pszColumn)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteNot(const char* pszColumn)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteColumn(const char* pszColumn, int iData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteColumn(const char* pszColumn, float fData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteColumn(const char* pszColumn, const char* pszData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteColumn(const char* pszColumn, int64 i64Data)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::WriteColumn(const char* pszColumn, const Variant& vData)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::InsertRow(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey)
{
    Trace("InsertRow {0}", m_tableName);

    array<ColumnValue>^ values = gcnew array<ColumnValue>(ttTemplate.NumColumns);

    for (unsigned int i = 0; i < ttTemplate.NumColumns; i ++)
    {
        values[i].Name = gcnew System::String(ttTemplate.ColumnNames[i]);
        values[i].Type = Convert(ttTemplate.Type[i]);
        values[i].Value = Convert(pvColVal[i]);
    }

    int64 id = m_cmd->Insert(m_tableName, values);
    if (piKey != NULL)
    {
        *piKey = (unsigned int)id;
    }

    return OK;
}

int SqlDatabaseWriteTable::InsertRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::Increment(const char* pszColumn, const Variant& vIncrement)
{
    Assert(false);
    return OK;
}

int SqlDatabaseWriteTable::Increment(const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("Increment {0} :: {1}", m_tableName, columnName);

    System::Object^ oldValue = m_cmd->IncrementSingle(m_tableName, columnName, Convert(vIncrement));
    Convert(oldValue, pvOldValue);
    return OK;
}

int SqlDatabaseWriteTable::Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("Increment {0} :: {1}", m_tableName, columnName);

    m_cmd->Increment(m_tableName, gcnew System::String(IdColumnName), (int64)iKey, columnName, Convert(vIncrement));
    return OK;
}

int SqlDatabaseWriteTable::Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("Increment {0} :: {1}", m_tableName, columnName);

    System::Object^ oldValue;
    m_cmd->Increment(m_tableName, gcnew System::String(IdColumnName), (int64)iKey, columnName, Convert(vIncrement), oldValue);
    Convert(oldValue, pvOldValue);

    return OK;
}

int SqlDatabaseWriteTable::DeleteRow(unsigned int iKey)
{
    Trace("DeleteRows {0}", m_tableName);

    m_cmd->DeleteRow(m_tableName, gcnew System::String(IdColumnName), (int64)iKey);
    return OK;
}

int SqlDatabaseWriteTable::DeleteAllRows()
{
    Trace("DeleteAllRows {0}", m_tableName);

    m_cmd->DeleteAllRows(m_tableName);
    return OK;
}

//
// Redirects
//

unsigned int SqlDatabaseWriteTable::GetNumRows(unsigned int* piNumRows)
{
    return m_readTable.GetNumRows(piNumRows);
}

int SqlDatabaseWriteTable::DoesRowExist(unsigned int iKey, bool* pbExists)
{
    return m_readTable.DoesRowExist(iKey, pbExists);
}

int SqlDatabaseWriteTable::GetFirstKey(const char* pszColumn, int iData, unsigned int* piKey)
{
    return m_readTable.GetFirstKey(pszColumn, iData, piKey);
}

int SqlDatabaseWriteTable::GetFirstKey(const char* pszColumn, float fData, unsigned int* piKey)
{
    return m_readTable.GetFirstKey(pszColumn, fData, piKey);
}

int SqlDatabaseWriteTable::GetFirstKey(const char* pszColumn, const char* pszData, unsigned int* piKey)
{
    return m_readTable.GetFirstKey(pszColumn, pszData, piKey);
}

int SqlDatabaseWriteTable::GetFirstKey(const char* pszColumn, int64 i64Data, unsigned int* piKey)
{
    return m_readTable.GetFirstKey(pszColumn, i64Data, piKey);
}

int SqlDatabaseWriteTable::GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey)
{
    return m_readTable.GetFirstKey(pszColumn, vData, piKey);
}

int SqlDatabaseWriteTable::GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys)
{
    return m_readTable.GetAllKeys(ppiKey, piNumKeys);
}

int SqlDatabaseWriteTable::GetNextKey(unsigned int iKey, unsigned int* piNextKey)
{
    return m_readTable.GetNextKey(iKey, piNextKey);
}

int SqlDatabaseWriteTable::GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    return m_readTable.GetEqualKeys(pszColumn, vData, ppiKey, piNumKeys);
}

int SqlDatabaseWriteTable::GetSearchKeys(const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
    return m_readTable.GetSearchKeys(sdSearch, ppiKey, piNumHits, piStopKey);
}

int SqlDatabaseWriteTable::ReadData(unsigned int iKey, const char* pszColumn, int* piData)
{
    return m_readTable.ReadData(iKey, pszColumn, piData);
}

int SqlDatabaseWriteTable::ReadData(unsigned int iKey, const char* pszColumn, float* pfData)
{
    return m_readTable.ReadData(iKey, pszColumn, pfData);
}

int SqlDatabaseWriteTable::ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data)
{
    return m_readTable.ReadData(iKey, pszColumn, pi64Data);
}

int SqlDatabaseWriteTable::ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    return m_readTable.ReadData(iKey, pszColumn, pvData);
}

int SqlDatabaseWriteTable::ReadData(const char* pszColumn, int* piData)
{
    return m_readTable.ReadData(pszColumn, piData);
}

int SqlDatabaseWriteTable::ReadData(const char* pszColumn, float* pfData)
{
    return m_readTable.ReadData(pszColumn, pfData);
}

int SqlDatabaseWriteTable::ReadData(const char* pszColumn, int64* pi64Data)
{
    return m_readTable.ReadData(pszColumn, pi64Data);
}

int SqlDatabaseWriteTable::ReadData(const char* pszColumn, Variant* pvData)
{
    return m_readTable.ReadData(pszColumn, pvData);
}

int SqlDatabaseWriteTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows)
{
    return m_readTable.ReadColumn(pszColumn, ppiKey, ppiData, piNumRows);
}

int SqlDatabaseWriteTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, float** ppfData, unsigned int* piNumRows)
{
    return m_readTable.ReadColumn(pszColumn, ppiKey, ppfData, piNumRows);
}

int SqlDatabaseWriteTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows)
{
    return m_readTable.ReadColumn(pszColumn, ppiKey, ppi64Data, piNumRows);
}

int SqlDatabaseWriteTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    return m_readTable.ReadColumn( pszColumn, ppiKey, ppvData, piNumRows);
}

int SqlDatabaseWriteTable::ReadColumn(const char* pszColumn, int** ppiData, unsigned int* piNumRows)
{
   return m_readTable.ReadColumn(pszColumn, ppiData, piNumRows);
}

int SqlDatabaseWriteTable::ReadColumn(const char* pszColumn, float** ppfData, unsigned int* piNumRows)
{
    return m_readTable.ReadColumn(pszColumn, ppfData, piNumRows);
}

int SqlDatabaseWriteTable::ReadColumn(const char* pszColumn, int64** ppi64Data, unsigned int* piNumRows)
{
    return m_readTable.ReadColumn(pszColumn, ppi64Data, piNumRows);
}

int SqlDatabaseWriteTable::ReadColumn(const char* pszColumn, Variant** ppvData, unsigned int* piNumRows)
{
    return m_readTable.ReadColumn(pszColumn, ppvData, piNumRows);
}

int SqlDatabaseWriteTable::ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows)
{
    return m_readTable.ReadColumns(iNumColumns, ppszColumn, ppiKey, pppvData, piNumRows);
}

int SqlDatabaseWriteTable::ReadRow(unsigned int iKey, void*** ppData)
{
    return m_readTable.ReadRow(iKey, ppData);
}

int SqlDatabaseWriteTable::ReadRow(unsigned int iKey, Variant** ppvData)
{
    return m_readTable.ReadRow(iKey, ppvData);
}

int SqlDatabaseWriteTable::ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                                unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys)
{
    return m_readTable.ReadColumnWhereEqual(pszEqualColumn, vData, pszReadColumn, ppiKey, ppvData, piNumKeys);
}