#include "SqlDatabaseConnection.h"
#include "SqlDatabaseWriteTable.h"
#include "Utils.h"

using namespace System::Collections::Generic;
using namespace System::Data::SqlClient;

SqlDatabaseConnection::SqlDatabaseConnection(SqlDatabase^ sqlDatabase)
    :
    m_iNumRefs(1),
    m_sqlDatabase(sqlDatabase),
    m_cmd(m_sqlDatabase->CreateCommandManager()),
    m_viewCollection(m_cmd)
{
}

SqlDatabaseConnection::~SqlDatabaseConnection()
{
    delete m_cmd;
}

// View operations
ICachedTableCollection* SqlDatabaseConnection::GetCache()
{
    return &m_viewCollection;
}

// Table operations
int SqlDatabaseConnection::CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate)
{
    Trace("CreateTable {0}", gcnew System::String(pszTableName));

    List<ColumnDescription>^ cols = gcnew List<ColumnDescription>();

    TableDescription tableDesc;
    tableDesc.Name = gcnew System::String(pszTableName);
    tableDesc.Columns = cols;

    ColumnDescription colDesc;
    colDesc.Name = gcnew System::String(IdColumnName);
    colDesc.Type = SqlDbType::BigInt;
    colDesc.Size = 0;
    colDesc.IsPrimaryKey = true;
    cols->Add(colDesc);

    for (unsigned int i = 0; i < ttTemplate.NumColumns; i ++)
    {
        colDesc.Name = gcnew System::String(ttTemplate.ColumnNames[i]);
        colDesc.Type = Convert(ttTemplate.Type[i]);
        colDesc.Size = ttTemplate.Size[i] == VARIABLE_LENGTH_STRING ? System::Int32::MaxValue : ttTemplate.Size[i];
        colDesc.IsPrimaryKey = false;
        cols->Add(colDesc);
    }

    m_cmd->CreateTable(tableDesc);

    // TODOTODO - Indexes

    // TODOTODO - Foreign keys

    return OK;
}

int SqlDatabaseConnection::DeleteTable(const char* pszTableName)
{
    System::String^ tableName = gcnew System::String(pszTableName);
    Trace("DeleteTable {0}", tableName);

    m_cmd->DeleteTable(tableName);
    return OK;
}

bool SqlDatabaseConnection::DoesTableExist(const char* pszTableName)
{
    System::String^ tableName = gcnew System::String(pszTableName);
    Trace("DoesTableExist {0}", tableName);

    return m_cmd->DoesTableExist(tableName);
}

// Standard operations
int SqlDatabaseConnection::ReadData(const char* pszTableName, unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.ReadData(iKey, pszColumn, pvData);
}

int SqlDatabaseConnection::ReadData(const char* pszTableName, const char* pszColumn, Variant* pvData)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.ReadData(pszColumn, pvData);
}
    
int SqlDatabaseConnection::WriteData(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vData)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.WriteData(iKey, pszColumn, vData);
}

int SqlDatabaseConnection::WriteData(const char* pszTableName, const char* pszColumn, const Variant& vData)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.WriteData(pszColumn, vData);
}

int SqlDatabaseConnection::Increment(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.Increment(iKey, pszColumn, vIncrement);
}

int SqlDatabaseConnection::Increment(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.Increment(iKey, pszColumn, vIncrement, pvOldValue);
}

int SqlDatabaseConnection::Increment(const char* pszTableName, const char* pszColumn, const Variant& vIncrement)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.Increment(pszColumn, vIncrement);
}

int SqlDatabaseConnection::Increment(const char* pszTableName, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.Increment(pszColumn, vIncrement, pvOldValue);
}

int SqlDatabaseConnection::WriteAnd(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.WriteAnd(iKey, pszColumn, iBitField);
}

int SqlDatabaseConnection::WriteAnd(const char* pszTableName, const char* pszColumn, unsigned int iBitField)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.WriteAnd(pszColumn, iBitField);
}

int SqlDatabaseConnection::WriteOr(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.WriteOr(iKey, pszColumn, iBitField);
}

int SqlDatabaseConnection::WriteOr(const char* pszTableName, const char* pszColumn, unsigned int iBitField)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.WriteOr(pszColumn, iBitField);
}

int SqlDatabaseConnection::WriteXor(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseConnection::WriteXor(const char* pszTableName, const char* pszColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseConnection::WriteNot(const char* pszTableName, unsigned int iKey, const char* pszColumn)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseConnection::WriteNot(const char* pszTableName, const char* pszColumn)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseConnection::WriteColumn(const char* pszTableName, const char* pszColumn, const Variant& vData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

// Row operations
int SqlDatabaseConnection::GetNumPhysicalRows(const char* pszTableName, unsigned int* piNumRows)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetNumRows(piNumRows);
}

int SqlDatabaseConnection::DoesPhysicalRowExist(const char* pszTableName, unsigned int iKey, bool* pbExists)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.DoesRowExist(iKey, pbExists);
}

int SqlDatabaseConnection::InsertRow(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.InsertRow(ttTemplate, pvColVal, piKey);
}

int SqlDatabaseConnection::InsertRows(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.InsertRows(ttTemplate, pvColVal, iNumRows);
}

int SqlDatabaseConnection::InsertDuplicateRows(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.InsertDuplicateRows(ttTemplate, pvColVal, iNumRows);
}

int SqlDatabaseConnection::DeleteRow(const char* pszTableName, unsigned int iKey)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.DeleteRow(iKey);
}

int SqlDatabaseConnection::DeleteAllRows(const char* pszTableName)
{
    SqlDatabaseWriteTable write(m_cmd, gcnew System::String(pszTableName));
    return write.DeleteAllRows();
}

int SqlDatabaseConnection::ReadRow(const char* pszTableName, unsigned int iKey, Variant** ppvData)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.ReadRow(iKey, ppvData);
}

// Column operations
int SqlDatabaseConnection::ReadColumn(const char* pszTableName, const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.ReadColumn(pszColumn, ppiKey, ppvData, piNumRows);
}

int SqlDatabaseConnection::ReadColumn(const char* pszTableName, const char* pszColumn, Variant** ppvData, unsigned int* piNumRows)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.ReadColumn(pszColumn, NULL, ppvData, piNumRows);
}

int SqlDatabaseConnection::ReadColumns(const char* pszTableName, unsigned int iNumColumns, const char* const* ppszColumn, 
                                       unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.ReadColumns(iNumColumns, ppszColumn, ppiKey, pppvData, piNumRows);
}

int SqlDatabaseConnection::ReadColumns(const char* pszTableName, unsigned int iNumColumns, const char* const* ppszColumn, 
                                       Variant*** pppvData, unsigned int* piNumRows)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.ReadColumns(iNumColumns, ppszColumn, NULL, pppvData, piNumRows);
}

// Searches
int SqlDatabaseConnection::GetAllKeys(const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetAllKeys(ppiKey, piNumKeys);
}

int SqlDatabaseConnection::GetNextKey(const char* pszTableName, unsigned int iKey, unsigned int* piNextKey)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetNextKey(iKey, piNextKey);
}

int SqlDatabaseConnection::GetFirstKey(const char* pszTableName, const char* pszColumn, const Variant& vData, unsigned int* piKey)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetFirstKey(pszColumn, vData, piKey);
}

int SqlDatabaseConnection::GetEqualKeys(const char* pszTableName, const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetEqualKeys(pszColumn, vData, ppiKey, piNumKeys);
}

int SqlDatabaseConnection::GetSearchKeys(const char* pszTableName, const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetSearchKeys(sdSearch, ppiKey, piNumHits, piStopKey);
}

int SqlDatabaseConnection::ReadColumnWhereEqual(const char* pszTableName, const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                                unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.ReadColumnWhereEqual(pszEqualColumn, vData, pszReadColumn, ppiKey, ppvData, piNumKeys);
}

//
// Direct API
//

int SqlDatabaseConnection::GetTableForReading(const char* pszTableName, IReadTable** ppTable)
{
    System::String^ tableName = gcnew System::String(pszTableName);
    Trace("GetTableForReading {0}", tableName);

    if (!m_cmd->DoesTableExist(tableName))
    {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    *ppTable = new SqlDatabaseReadTable(m_cmd, tableName);
    if (*ppTable == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }
    return OK;
}

int SqlDatabaseConnection::GetTableForWriting(const char* pszTableName, IWriteTable** ppTable)
{
    System::String^ tableName = gcnew System::String(pszTableName);
    Trace("GetTableForWriting {0}", tableName);

    if (!m_cmd->DoesTableExist(tableName))
    {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    *ppTable = new SqlDatabaseWriteTable(m_cmd, tableName);
    if (*ppTable == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }
    return OK;
}