#include "SqlDatabaseConnection.h"
#include "SqlDatabaseReadTable.h"
#include "Utils.h"

using namespace System::Collections::Generic;
using namespace System::Data::SqlClient;

SqlDatabaseConnection::SqlDatabaseConnection(SqlDatabase^ sqlDatabase, TransactionIsolationLevel isoLevel)
    :
    m_iNumRefs(1),
    m_sqlDatabase(sqlDatabase),
    m_cmd(m_sqlDatabase->CreateCommandManager(Convert(isoLevel))),
    m_viewCollection(m_cmd)
{
}

SqlDatabaseConnection::~SqlDatabaseConnection()
{
    delete m_cmd;
}

int SqlDatabaseConnection::Commit()
{
    int iErrCode = m_viewCollection.Commit();
    if (iErrCode == OK)
    {
        try
        {
            m_cmd->SetComplete();
        }
        catch (SqlDatabaseException^)
        {
            iErrCode = ERROR_FAILURE;
        }
    }
    return iErrCode;
}

ICachedTableCollection* SqlDatabaseConnection::GetCache()
{
    return &m_viewCollection;
}

// Table operations
int SqlDatabaseConnection::DoesTableExist(const char* pszTableName, bool* pbExist)
{
    System::String^ tableName = gcnew System::String(pszTableName);
    Trace("DoesTableExist {0}", tableName);

    try
    {
        *pbExist = m_cmd->DoesTableExist(tableName);
    }
    catch (SqlDatabaseException^)
    {
        return ERROR_DATABASE_EXCEPTION;
    }

    return OK;
}

int SqlDatabaseConnection::CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate)
{
    Trace("CreateTable {0}", gcnew System::String(pszTableName));

    List<ColumnDescription>^ cols = gcnew List<ColumnDescription>();

    TableDescription tableDesc;
    tableDesc.Name = gcnew System::String(pszTableName);
    tableDesc.Columns = cols;

    ColumnDescription colDesc;
    colDesc.Name = gcnew System::String(ID_COLUMN_NAME);
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

    try
    {
        m_cmd->CreateTable(tableDesc);
    }
    catch (SqlDatabaseException^)
    {
        return ERROR_DATABASE_EXCEPTION;
    }

    // TODOTODO - Indexes

    // TODOTODO - Foreign keys

    return OK;
}

int SqlDatabaseConnection::DeleteTable(const char* pszTableName)
{
    System::String^ tableName = gcnew System::String(pszTableName);
    Trace("DeleteTable {0}", tableName);
    
    try
    {
        m_cmd->DeleteTable(tableName);
    }
    catch (SqlDatabaseException^)
    {
        return ERROR_DATABASE_EXCEPTION;
    }
    return OK;
}

// Row operations
int SqlDatabaseConnection::GetNumPhysicalRows(const char* pszTableName, unsigned int* piNumRows)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetNumRows(piNumRows);
}

int SqlDatabaseConnection::GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetSearchKeys(sdRange, ppiKey, piNumHits, piStopKey);
}

int SqlDatabaseConnection::GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, const OrderByDefinition& sdOrderBy, unsigned int** ppiKey, unsigned int* piNumHits)
{
    SqlDatabaseReadTable read(m_cmd, gcnew System::String(pszTableName));
    return read.GetSearchKeys(sdRange, sdOrderBy, ppiKey, piNumHits);
}