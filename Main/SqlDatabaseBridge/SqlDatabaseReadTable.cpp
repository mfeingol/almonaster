#include "SqlDatabaseReadTable.h"
#include "Utils.h"

using namespace System::Collections::Generic;
using namespace System::Linq;

SqlDatabaseReadTable::SqlDatabaseReadTable(SqlCommandManager^ cmd, System::String^ tableName)
{
    m_iNumRefs = 1;

    m_cmd = cmd;
    m_tableName = tableName;
}

SqlDatabaseReadTable::~SqlDatabaseReadTable()
{
}

unsigned int SqlDatabaseReadTable::GetNumRows(unsigned int* piNumRows)
{
    Trace("GetNumRows {0}", m_tableName);

    *piNumRows = (unsigned int)m_cmd->GetRowCount(m_tableName);
    return OK;
}

int SqlDatabaseReadTable::DoesRowExist(unsigned int iKey, bool* pbExists)
{
    Trace("DoesRowExist {0}", m_tableName);

    *pbExists = m_cmd->DoesRowExist(m_tableName, gcnew System::String(IdColumnName), iKey);
    return OK;
}

int SqlDatabaseReadTable::GetFirstKey(const char* pszColumn, int iData, unsigned int* piKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::GetFirstKey(const char* pszColumn, float fData, unsigned int* piKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::GetFirstKey(const char* pszColumn, const char* pszData, unsigned int* piKey)
{
    Trace("GetFirstKey {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    int64 id = m_cmd->GetIdOfFirstMatch(m_tableName, gcnew System::String(IdColumnName), gcnew System::String(pszColumn), gcnew System::String(pszData));
    if (id == System::Int64::MinValue)
    {
        *piKey = NO_KEY;
        return ERROR_DATA_NOT_FOUND;
    }
    else
    {
        *piKey = (unsigned int)id;
        return OK;
    }
}

int SqlDatabaseReadTable::GetFirstKey(const char* pszColumn, int64 i64Data, unsigned int* piKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey)
{
    Trace("GetFirstKey {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    int64 id = m_cmd->GetIdOfFirstMatch(m_tableName, gcnew System::String(IdColumnName), gcnew System::String(pszColumn), Convert(vData));
    if (id == System::Int64::MinValue)
    {
        *piKey = NO_KEY;
        return ERROR_DATA_NOT_FOUND;
    }
    else
    {
        *piKey = (unsigned int)id;
        return OK;
    }
}

int SqlDatabaseReadTable::GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys)
{
    Trace("GetAllKeys {0}", m_tableName);

    array<System::String^>^ cols = gcnew array<System::String^>(1);
    cols[0] = gcnew System::String(IdColumnName);

    IEnumerable<RowValues>^ rows = m_cmd->ReadColumns(m_tableName, cols);

    unsigned int iNumRows = Enumerable::Count(rows);
    if (iNumRows == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    unsigned int* piKey = new unsigned int[iNumRows];
    Assert(piKey != NULL);

    unsigned int iRow = 0;
    for each (RowValues row in rows)
    {
        for each (System::Object^ value in row.Values)
        {
            piKey[iRow] = (unsigned int)(int64)value;
        }
        iRow ++;
    }

    *ppiKey = piKey;
    *piNumKeys = iNumRows;
    return OK;
}

int SqlDatabaseReadTable::GetNextKey(unsigned int iKey, unsigned int* piNextKey)
{
    int64 id = m_cmd->GetNextId(m_tableName, gcnew System::String(IdColumnName), iKey);
    if (id == System::Int64::MinValue)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    *piNextKey = (unsigned int)id;
    return OK;
}

int SqlDatabaseReadTable::GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::GetSearchKeys(const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadData(unsigned int iKey, const char* pszColumn, int* piData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadData(unsigned int iKey, const char* pszColumn, float* pfData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadData(unsigned int iKey, const char* pszColumn, const char** ppszData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    Trace("ReadData {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    System::Object^ read = m_cmd->ReadData(m_tableName, gcnew System::String(IdColumnName), iKey, gcnew System::String(pszColumn));
    if (read == nullptr)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    Convert(read, pvData);
    return OK;
}

int SqlDatabaseReadTable::ReadData(const char* pszColumn, int* piData)
{
    Trace("ReadData {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    System::Object^ read = m_cmd->ReadSingleData(m_tableName, gcnew System::String(pszColumn));
    if (read == nullptr)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    *piData = (int)read;
    return OK;
}

int SqlDatabaseReadTable::ReadData(const char* pszColumn, float* pfData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadData(const char* pszColumn, const char** ppszData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadData(const char* pszColumn, int64* pi64Data)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadData(const char* pszColumn, Variant* pvData)
{
    Trace("ReadData {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    // Must be OneRow table
    System::Object^ read = m_cmd->ReadSingleData(m_tableName, gcnew System::String(pszColumn));
    if (read == nullptr)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    Convert(read, pvData);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, float** ppfData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, char*** ppszData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    Trace("ReadColumn {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    int iNumCols = ppiKey != NULL ? 2 : 1;
    array<System::String^>^ cols = gcnew array<System::String^>(iNumCols);
    cols[0] = gcnew System::String(pszColumn);
    if (ppiKey != NULL)
    {
        cols[1] = gcnew System::String(IdColumnName);
    }

    IEnumerable<RowValues>^ rows = m_cmd->ReadColumns(m_tableName, cols);
    unsigned int iNumRows = Enumerable::Count(rows);
    if (iNumRows == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    Variant* pvData = new Variant[iNumRows];
    Assert(pvData);

    if (ppiKey != NULL)
    {
        *ppiKey = new unsigned int[iNumRows];
    }

    unsigned int iRow = 0;
    for each (RowValues row in rows)
    {
        unsigned int iCol = 0;
        ppvData[iRow] = pvData + iRow;
        for each (System::Object^ value in row.Values)
        {
            if (ppiKey != NULL && iCol == 1)
            {
                *ppiKey[iRow] = (unsigned int)(long)value;
            }
            else
            {
                Convert(value, &pvData[iRow]);
                iCol ++;
            }
        }
        iRow ++;
    }

    *piNumRows = iNumRows;
    *ppvData = pvData;
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, int** ppiData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, float** ppfData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, char*** ppszData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, int64** ppi64Data, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumn(const char* pszColumn, Variant** ppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows)
{
    Trace("ReadColumns {0}", m_tableName);

    int iBonus = ppiKey != NULL ? 1:0;
    array<System::String^>^ cols = gcnew array<System::String^>(iNumColumns + iBonus);
    for (unsigned int i = 0; i < iNumColumns; i ++)
    {
        cols[i] = gcnew System::String(ppszColumn[i]);
    }

    if (ppiKey != NULL)
    {
        cols[iNumColumns] = gcnew System::String(IdColumnName);
    }

    IEnumerable<RowValues>^ rows = m_cmd->ReadColumns(m_tableName, cols);
    unsigned int iNumRows = Enumerable::Count(rows);
    if (iNumRows == 0)
    {
        if (ppiKey != NULL)
        {
            *ppiKey = NULL;
        }
        *pppvData = NULL;
        *piNumRows = 0;

        return ERROR_DATA_NOT_FOUND;
    }

    Variant** ppvData = new Variant*[iNumRows];
    Assert(ppvData);

    Variant* pvData = new Variant[iNumRows * iNumColumns];
    Assert(pvData);

    if (ppiKey != NULL)
    {
        *ppiKey = new unsigned int[iNumRows];
    }

    unsigned int iRow = 0;
    for each (RowValues row in rows)
    {
        unsigned int iCol = 0;
        ppvData[iRow] = pvData + iRow * iNumColumns;
        for each (System::Object^ value in row.Values)
        {
            if (ppiKey != NULL && iCol == iNumColumns)
            {
                *ppiKey[iRow] = (unsigned int)(long)value;
            }
            else
            {
                Convert(value, &ppvData[iRow][iCol]);
                iCol ++;
            }
        }
        iRow ++;
    }

    *piNumRows = iNumRows;
    *pppvData = ppvData;
    return OK;
}

int SqlDatabaseReadTable::ReadRow(unsigned int iKey, void*** ppData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadRow(unsigned int iKey, Variant** ppvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                               unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}
