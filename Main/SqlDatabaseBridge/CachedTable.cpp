#include "CachedTable.h"
#include "Utils.h"

#include "Osal/Vector.h"

using namespace System::Linq;

CachedTable::CachedTable(SqlCommandManager^ cmd, BulkTableReadResult^ result)
    :
    m_iNumRefs(1),
    m_cmd(cmd),
    m_result(result),
    m_keyToRows(gcnew SortedDictionary<int64, IDictionary<System::String^, System::Object^>^>())
{
    System::String^ idCol = gcnew System::String(IdColumnName);
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        int64 id = (int64)row[idCol];
        m_keyToRows->Add(id, row);
    }
}

int CachedTable::GetNumCachedRows(unsigned int* piNumRows)
{
    Trace("CachedTable :: GetNumCachedRows :: {0}", m_result->TableName);

    *piNumRows = m_result->Rows->Count;
    return OK;
}

int CachedTable::GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: GetFirstKey :: {0} :: {1}", m_result->TableName, columnName);

    *piKey = NO_KEY;

    System::String^ idCol = gcnew System::String(IdColumnName);
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        System::Object^ value = row[columnName];
        Variant vValue;
        Convert(value, &vValue);
        if (vValue == vData)
        {
            System::Object^ id = row[idCol];
            *piKey = (unsigned int)(int64)id;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}

int CachedTable::GetNextKey(unsigned int iKey, unsigned int* piNextKey)
{
    Trace("CachedTable :: GetNextKey :: {0}", m_result->TableName);

    *piNextKey = NO_KEY;

    if (m_result->Rows->Count == 0)
    {
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iKey == NO_KEY)
    {
        *piNextKey = (unsigned int)Enumerable::First(m_keyToRows->Keys);
        return OK;
    }

    // TODO - can we do better than this with a better data structure?
    bool bNext = false;
    for each (long id in m_keyToRows->Keys)
    {
        unsigned int iId = (unsigned int)id;
        if (iKey == iId)
        {
            bNext = true;
        }

        if (bNext)
        {
            *piNextKey = iId;
            return OK;
        }
    }

    return bNext ? ERROR_DATA_NOT_FOUND : ERROR_UNKNOWN_ROW_KEY;
}

int CachedTable::GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: GetEqualKeys :: {0} :: {1}", m_result->TableName, columnName);

    *ppiKey = NULL;
    *piNumKeys = 0;

    Vector<unsigned int> vectKeys;

    System::String^ idCol = gcnew System::String(IdColumnName);
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        System::Object^ value = row[columnName];
        Variant vValue;
        Convert(value, &vValue);
        if (vValue == vData)
        {
            System::Object^ id = row[idCol];
            vectKeys.Add((unsigned int)(int64)id);
        }
    }

    *piNumKeys = vectKeys.GetNumElements();
    if (*piNumKeys == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    *ppiKey = new unsigned int[*piNumKeys];
    Assert(*ppiKey);

    memcpy(*ppiKey, vectKeys.GetData(), *piNumKeys * sizeof(unsigned int));
    return OK;
}

int CachedTable::GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys)
{
    Trace("CachedTable :: GetAllKeys :: {0}", m_result->TableName);

    unsigned int iNumRows = m_result->Rows->Count;

    *ppiKey = NULL;
    *piNumKeys = iNumRows;

    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    *ppiKey = new unsigned int[iNumRows];
    Assert(*ppiKey);

    int index = 0;
    for each (long key in m_keyToRows->Keys)
    {
        (*ppiKey)[index ++] = (unsigned int)key;
    }

    return OK;
}

int CachedTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: ReadColumn {0} :: {1}", m_result->TableName, columnName);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    *ppiData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    int* piData = new int[iNumRows];
    Assert(piData);

    System::String^ idCol;
    if (ppiKey != NULL)
    {
        idCol = gcnew System::String(IdColumnName);
        *ppiKey = new unsigned int[iNumRows];
        Assert(*ppiKey);
    }

    int index = 0;
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        if (ppiKey != NULL)
        {
            System::Object^ id = row[idCol];
            (*ppiKey)[index] = (unsigned int)(int64)id;
        }

        piData[index] = (int)row[columnName];
        index ++;
    }

    *ppiData = piData;
    return OK;
}

int CachedTable::ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: ReadColumn {0} :: {1}", m_result->TableName, columnName);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    *ppvData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    Variant* pvData = new Variant[iNumRows];
    Assert(pvData);

    System::String^ idCol;
    if (ppiKey != NULL)
    {
        idCol = gcnew System::String(IdColumnName);
        *ppiKey = new unsigned int[iNumRows];
        Assert(*ppiKey);
    }

    int index = 0;
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        if (ppiKey != NULL)
        {
            System::Object^ id = row[idCol];
            (*ppiKey)[index] = (unsigned int)(int64)id;
        }

        Convert(row[columnName], pvData + index);
        index ++;
    }

    *ppvData = pvData;
    return OK;
}

int CachedTable::ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows)
{
    Trace("CachedTable :: ReadColumns {0}", m_result->TableName);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    *pppvData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    Variant** ppvData = new Variant*[iNumRows];
    Assert(ppvData);

    Variant* pvData = new Variant[iNumRows * iNumColumns];
    Assert(ppvData);

    System::String^ idCol;
    if (ppiKey != NULL)
    {
        idCol = gcnew System::String(IdColumnName);
        *ppiKey = new unsigned int[iNumRows];
        Assert(*ppiKey);
    }

    array<System::String^>^ columnNames = gcnew array<System::String^>(iNumColumns);
    for (unsigned int i = 0; i < iNumColumns; i ++)
    {
        columnNames[i] = gcnew System::String(ppszColumn[i]);
    }

    unsigned int index = 0;
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        if (ppiKey != NULL)
        {
            System::Object^ id = row[idCol];
            (*ppiKey)[index] = (unsigned int)(int64)id;
        }

        ppvData[index] = pvData + index * iNumColumns;
        for (unsigned int column = 0; column < iNumColumns; column ++)
        {
            System::Object^ value = row[columnNames[column]];
            Convert(value, ppvData[index] + column);
        }

        index ++;
    }

    *pppvData = ppvData;
    return OK;
}

int CachedTable::ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                      unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    System::String^ equalColumnName = gcnew System::String(pszEqualColumn);
    System::String^ readColumnName = gcnew System::String(pszReadColumn);

    Trace("CachedTable :: ReadColumnWhereEqual {0} :: {1} :: {2}", m_result->TableName, equalColumnName, readColumnName);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    *ppvData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    Variant* pvData = new Variant[iNumRows];
    Assert(pvData);

    System::String^ idCol;
    if (ppiKey != NULL)
    {
        idCol = gcnew System::String(IdColumnName);
        *ppiKey = new unsigned int[iNumRows];
        Assert(*ppiKey);
    }

    unsigned int index = 0;
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        System::Object^ value = row[equalColumnName];
        Variant vValue;
        Convert(value, &vValue);

        if (vValue == vData)
        {
            if (ppiKey != NULL)
            {
                System::Object^ id = row[idCol];
                (*ppiKey)[index] = (unsigned int)(int64)id;
            }

            Convert(row[readColumnName], pvData + index);
            index ++;
        }
    }

    *piNumRows = index;
    *ppvData = pvData;

    return OK;
}

IDictionary<System::String^, System::Object^>^ CachedTable::GetRow(unsigned int iKey, int* piErrCode)
{
    *piErrCode = OK;
    IDictionary<System::String^, System::Object^>^ row = nullptr;

    if (m_result->Rows->Count == 0)
    {
        *piErrCode = ERROR_UNKNOWN_ROW_KEY;
    }
    else
    {
        if (iKey == NO_KEY)
        {
            int count = m_result->Rows->Count;
            if (count > 1)
            {
                *piErrCode = ERROR_TABLE_HAS_MORE_THAN_ONE_ROW;
            }
            else
            {
                row = m_result->Rows[0];
            }
        }
        else if (!m_keyToRows->TryGetValue(iKey, row))
        {
            *piErrCode = ERROR_UNKNOWN_ROW_KEY;
        }
    }

    return row;
}

int CachedTable::ReadRow(Variant** ppvData)
{
    return ReadRow(NO_KEY, ppvData);
}

int CachedTable::ReadRow(unsigned int iKey, Variant** ppvData)
{
    Trace("CachedTable :: ReadRow {0}", m_result->TableName);

    *ppvData = NULL;

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    int cols = Enumerable::Count(row->Values) - 1;
    Variant* pvData = new Variant[cols];
    Assert(pvData);

    System::String^ idCol = gcnew System::String(IdColumnName);
    int index = 0;
    for each (KeyValuePair<System::String^, System::Object^>^ pair in row)
    {
        // Skip id
        if (pair->Key != idCol)
        {
            Convert(pair->Value, pvData + index - 1);
            index ++;
        }
    }

    *ppvData = pvData;
    return OK;
}

int CachedTable::ReadData(const char* pszColumn, int* piData)
{
    Variant vData;
    int iErrCode = ReadData(pszColumn, &vData);
    if (iErrCode == OK)
    {
        *piData = vData.GetInteger();
    }
    return iErrCode;
}

int CachedTable::ReadData(const char* pszColumn, int64* pi64Data)
{
    Variant vData;
    int iErrCode = ReadData(pszColumn, &vData);
    if (iErrCode == OK)
    {
        *pi64Data = vData.GetInteger64();
    }
    return iErrCode;
}

int CachedTable::ReadData(const char* pszColumn, float* pfData)
{
    Variant vData;
    int iErrCode = ReadData(pszColumn, &vData);
    if (iErrCode == OK)
    {
        *pfData = vData.GetFloat();
    }
    return iErrCode;
}

int CachedTable::ReadData(const char* pszColumn, Variant* pvData)
{
    return ReadData(NO_KEY, pszColumn, pvData);
}

int CachedTable::ReadData(unsigned int iKey, const char* pszColumn, int* piData)
{
    Variant vData;
    int iErrCode = ReadData(iKey, pszColumn, &vData);
    if (iErrCode == OK)
    {
        *piData = vData.GetInteger();
    }
    return iErrCode;
}

int CachedTable::ReadData(unsigned int iKey, const char* pszColumn, float* pfData)
{
    Variant vData;
    int iErrCode = ReadData(iKey, pszColumn, &vData);
    if (iErrCode == OK)
    {
        *pfData = vData.GetFloat();
    }
    return iErrCode;
}

int CachedTable::ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data)
{
    Variant vData;
    int iErrCode = ReadData(iKey, pszColumn, &vData);
    if (iErrCode == OK)
    {
        *pi64Data = vData.GetInteger64();
    }
    return iErrCode;
}

int CachedTable::ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: ReadData {0} :: {1}", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    Convert(row[columnName], pvData);
    return OK;
}

int CachedTable::InsertRow(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey)
{
    Trace("CachedTable :: InsertRow {0}", m_result->TableName);
    return InsertDuplicateRows(ttTemplate, pvColVal, 1, piKey);
}

int CachedTable::InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows)
{
    Trace("CachedTable :: InsertDuplicateRows {0}", m_result->TableName);
    return InsertDuplicateRows(ttTemplate, pvColVal, iNumRows, NULL);
}

int CachedTable::InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows, unsigned int* piKey)
{
    if (piKey)
        *piKey = NO_KEY;

    // Insert rows into the database
    array<System::String^>^ columnNames = gcnew array<System::String^>(ttTemplate.NumColumns);
    array<InsertValue>^ values = gcnew array<InsertValue>(ttTemplate.NumColumns);

    for (unsigned int i = 0; i < ttTemplate.NumColumns; i ++)
    {
        columnNames[i] = gcnew System::String(ttTemplate.ColumnNames[i]);
        values[i].Type = Convert(ttTemplate.Type[i]);
        values[i].Value = Convert(pvColVal[i]);
    }

    List<IEnumerable<InsertValue>^>^ insert = gcnew List<IEnumerable<InsertValue>^>();

    for (unsigned int i = 0; i < iNumRows; i ++)
    {
        insert->Add(values);
    }

    IEnumerable<int64>^ ids;
    try
    {
        ids = m_cmd->Insert(m_result->TableName, columnNames, insert);
    }
    catch (SqlDatabaseException^)
    {
        // TODO - other errors?
        return ERROR_DUPLICATE_DATA;
    }

    if (iNumRows == 1 && piKey)
    {
        *piKey = (unsigned int)Enumerable::First(ids);
    }

    // Make a new template row for insertion
    Dictionary<System::String^, System::Object^>^ templateRow = gcnew Dictionary<System::String^, System::Object^>();
    for (unsigned int i = 0; i < ttTemplate.NumColumns; i ++)
    {
        templateRow[columnNames[i]] = values[i].Value;
    }

    // Insert all the rows
    bool first = true;
    for each (int64 id in ids)
    {
        Dictionary<System::String^, System::Object^>^ insertRow;
        if (first)
        {
            insertRow = templateRow;
            first = false;
        }
        else
        {
            insertRow = gcnew Dictionary<System::String^, System::Object^>(templateRow);
        }

        m_result->Rows->Add(insertRow);
        m_keyToRows->Add(id, insertRow);
    }

    return OK;
}

int CachedTable::DeleteRow(unsigned int iKey)
{
    Trace("CachedTable :: DeleteRows {0}", m_result->TableName);

    // First delete the actual row
    try
    {
        m_cmd->DeleteRow(m_result->TableName, gcnew System::String(IdColumnName), (int64)iKey);
    }
    catch (SqlDatabaseException^)
    {
        // TODO - other errors?
        return ERROR_UNKNOWN_ROW_KEY;
    }

    // Second, remove the row from cache
    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (iErrCode == OK)
    {
        m_keyToRows->Remove(iKey);
        m_result->Rows->Remove(row);
    }

    return OK;
}

int CachedTable::DeleteAllRows()
{
    Trace("CachedTable :: DeleteAllRows {0}", m_result->TableName);

    // TODOTODO - if we're just a view, do the right thing
    // First delete the actual rows
    try
    {
        m_cmd->DeleteAllRows(m_result->TableName);
    }
    catch (SqlDatabaseException^)
    {
        // TODO - other errors?
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    // Second, flush the cache
    m_keyToRows->Clear();
    m_result->Rows->Clear();

    return OK;
}

int CachedTable::Increment(const char* pszColumn, const Variant& vIncrement)
{
    return Increment(NO_KEY, pszColumn, vIncrement, NULL);
}

int CachedTable::Increment(const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    return Increment(NO_KEY, pszColumn, vIncrement, pvOldValue);
}

int CachedTable::Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement)
{
    return Increment(iKey, pszColumn, vIncrement, NULL);
}

int CachedTable::Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: Increment {0} :: {1}", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    System::Object^ value = row[columnName];
    System::Object^ newValue = ::Increment(value, vIncrement);

    row[columnName] = newValue;
    if (pvOldValue)
        Convert(value, pvOldValue);

    // TODOTODO - persist

    return OK;
}

int CachedTable::WriteData(const char* pszColumn, const char* pszData)
{
    return WriteData(NO_KEY, pszColumn, pszData);
}

int CachedTable::WriteData(const char* pszColumn, const Variant& vData)
{
    return WriteData(NO_KEY, pszColumn, vData);
}

int CachedTable::WriteData(unsigned int iKey, const char* pszColumn, const char* pszData)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: WriteData {0} :: {1}", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    row[columnName] = gcnew System::String(pszData);
    // TODOTODO - persist

    return OK;
}

int CachedTable::WriteData(unsigned int iKey, const char* pszColumn, const Variant& vData)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: WriteData {0} :: {1}", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    row[columnName] = Convert(vData);
    // TODOTODO - persist

    return OK;
}

int CachedTable::WriteAnd(const char* pszColumn, unsigned int iBitField)
{
    return WriteAnd(NO_KEY, pszColumn, iBitField);
}

int CachedTable::WriteAnd(unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: WriteAnd {0} :: {1}", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    int iValue = (int)row[columnName];
    row[columnName] = iValue & iBitField;
    // TODOTODO - persist

    return OK;
}

int CachedTable::WriteOr(const char* pszColumn, unsigned int iBitField)
{
    return WriteOr(NO_KEY, pszColumn, iBitField);
}

int CachedTable::WriteOr(unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("CachedTable :: WriteOr {0} :: {1}", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    int iValue = (int)row[columnName];
    row[columnName] = iValue | iBitField;
    // TODOTODO - persist

    return OK;
}