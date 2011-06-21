#include "SqlDatabaseReadTable.h"
#include "Utils.h"

using namespace System::Collections::Generic;
using namespace System::Data::SqlClient;
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
    array<ColumnSearchDescription>^ cols = gcnew array<ColumnSearchDescription>(1);
    cols[0].Name = gcnew System::String(pszColumn);
    cols[0].GreaterThanOrEqual = Convert(vData);
    cols[0].LessThanOrEqual = cols[0].GreaterThanOrEqual;

    try
    {
        if (ppiKey == NULL)
        {
            int64 count = m_cmd->SearchCount(m_tableName, cols);
            *piNumKeys = (unsigned int)count;
        }
        else
        {
            IEnumerable<int64>^ results = m_cmd->Search(m_tableName, gcnew System::String(IdColumnName), System::Int32::MaxValue, 0, cols);
            *ppiKey = ConvertIdsToKeys(results, piNumKeys);
        }
    }
    catch (SqlDatabaseException^ e)
    {
        SqlException^ sqe = safe_cast<SqlException^>(e->InnerException);
        if (sqe != nullptr)
        {
            if (sqe->ErrorCode == 0x80131904)
            {
                return ERROR_UNKNOWN_TABLE_NAME;
            }
        }
        return ERROR_FAILURE;
    }

    if (*piNumKeys == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }
    return OK;
}

int SqlDatabaseReadTable::GetSearchKeys(const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
    array<ColumnSearchDescription>^ cols = gcnew array<ColumnSearchDescription>(sdSearch.iNumColumns);

    for (unsigned int i = 0; i < sdSearch.iNumColumns; i ++)
    {
        cols[i].Name = gcnew System::String(sdSearch.pscColumns[i].pszColumn);
        cols[i].GreaterThanOrEqual = Convert(sdSearch.pscColumns[i].vData);
        if (sdSearch.pscColumns[i].vData.GetType() == V_STRING)
        {
            cols[i].LessThanOrEqual = cols[i].GreaterThanOrEqual;
        }
        else
        {
            cols[i].LessThanOrEqual = Convert(sdSearch.pscColumns[i].vData2);
        }

        // TODOTODO - Flags
    }

    int64 maxHits = sdSearch.iMaxNumHits == 0 ? System::Int32::MaxValue : sdSearch.iMaxNumHits + 1;
    IEnumerable<int64>^ results = m_cmd->Search(m_tableName, gcnew System::String(IdColumnName), maxHits, sdSearch.iSkipHits, cols);

    unsigned int iNumHits;
    *ppiKey = ConvertIdsToKeys(results, &iNumHits);

    if (sdSearch.iMaxNumHits > 0 && iNumHits > sdSearch.iMaxNumHits)
    {
        if (piStopKey != NULL)
        {
            *piStopKey = (*ppiKey)[iNumHits - 1];
        }

        *piNumHits = sdSearch.iMaxNumHits;
        return ERROR_TOO_MANY_HITS;
    }
    else
    {
        if (piStopKey != NULL)
        {
            *piStopKey = NO_KEY;
        }

        *piNumHits = iNumHits;
        return OK;
    }
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

int SqlDatabaseReadTable::ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data)
{
    Trace("ReadData {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    System::Object^ read = m_cmd->Read(m_tableName, gcnew System::String(IdColumnName), iKey, gcnew System::String(pszColumn));
    if (read == nullptr)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    *pi64Data = (int64)read;
    return OK;
}

int SqlDatabaseReadTable::ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    Trace("ReadData {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    System::Object^ read = m_cmd->Read(m_tableName, gcnew System::String(IdColumnName), iKey, gcnew System::String(pszColumn));
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

    System::Object^ read = m_cmd->ReadSingle(m_tableName, gcnew System::String(pszColumn));
    if (read == nullptr)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    *piData = (int)read;
    return OK;
}

int SqlDatabaseReadTable::ReadData(const char* pszColumn, float* pfData)
{
    Trace("ReadData {0} :: {1}", m_tableName, gcnew System::String(pszColumn));

    System::Object^ read = m_cmd->ReadSingle(m_tableName, gcnew System::String(pszColumn));
    if (read == nullptr)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    *pfData = (float)(double)read;
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
    System::Object^ read = m_cmd->ReadSingle(m_tableName, gcnew System::String(pszColumn));
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

int SqlDatabaseReadTable::ReadRow(unsigned int iKey, void*** pppData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseReadTable::ReadRow(unsigned int iKey, Variant** ppvData)
{
    RowValues row = m_cmd->ReadRow(m_tableName, gcnew System::String(IdColumnName), iKey);
    IEnumerable<System::Object^>^ values = row.Values;
    if (values == nullptr)
    {
        return ERROR_UNKNOWN_ROW_KEY;
    }

    unsigned int iNumCols = Enumerable::Count(values);
    Variant* pvData = new Variant[iNumCols];
    Assert(pvData != NULL);

    for (unsigned int i = 1; i < iNumCols; i ++)
    {
        Convert(Enumerable::ElementAt(values, i), pvData + i - 1);
    }

    *ppvData = pvData;
    return OK;
}

int SqlDatabaseReadTable::ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                               unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//
// Helper methods
//

