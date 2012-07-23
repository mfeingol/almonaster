//
// SqlDatabaseBridge.dll - A database library
// Copyright(c) 1998 Max Attar Feingold(maf6@cornell.edu)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or(at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA  02111-1307, USA.

#include "CachedTable.h"
#include "Utils.h"

#include "Osal/Vector.h"

using namespace System::Linq;

CachedTable::CachedTable(SqlCommandManager^ cmd, BulkTableReadResult^ result, bool bCompleteTable)
    :
    m_iNumRefs(1),
    m_cmd(cmd),
    m_result(result),
    m_bCompleteTable(bCompleteTable),
    m_keyToRows(gcnew SortedDictionary<int64, IDictionary<System::String^, System::Object^>^>()),
    m_writes(gcnew Dictionary<int64, IDictionary<System::String^, System::Object^>^>()),
    m_ID_COLUMN_NAME(gcnew System::String(ID_COLUMN_NAME))
{
    // Populate key lookup table
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        int64 id = (int64)row[m_ID_COLUMN_NAME];
        m_keyToRows->Add(id, row);
    }
}

BulkTableReadResult^ CachedTable::GetResult()
{
    return m_result;
}

IDictionary<int64, IDictionary<System::String^, System::Object^>^>^ CachedTable::ObtainWrites()
{
    IDictionary<int64, IDictionary<System::String^, System::Object^>^>^ ret = m_writes;
    m_writes = nullptr;
    return ret;
}

int CachedTable::GetNumCachedRows(unsigned int* piNumRows)
{
    Trace(TRACE_VERBOSE, "CachedTable :: GetNumCachedRows :: %s", m_result->TableName);

    *piNumRows = m_result->Rows->Count;
    return OK;
}

int CachedTable::GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace(TRACE_VERBOSE, "CachedTable :: GetFirstKey :: %s :: %s", m_result->TableName, columnName);

    *piKey = NO_KEY;

    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        System::Object^ value = row[columnName];
        Variant vValue;
        Convert(value, &vValue);
        if (vValue == vData)
        {
            System::Object^ id = row[m_ID_COLUMN_NAME];
            *piKey = (unsigned int)(int64)id;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}

int CachedTable::GetNextKey(unsigned int iKey, unsigned int* piNextKey)
{
    Trace(TRACE_VERBOSE, "CachedTable :: GetNextKey :: %s", m_result->TableName);

    *piNextKey = NO_KEY;

    if (m_result->Rows->Count == 0)
    {
        return (iKey == NO_KEY) ? ERROR_DATA_NOT_FOUND : ERROR_UNKNOWN_ROW_KEY;
    }

    if (iKey == NO_KEY)
    {
        *piNextKey = (unsigned int)Enumerable::First(m_keyToRows->Keys);
        return OK;
    }

    // TODO - can we do better than a linear search with a better data structure?
    bool bNext = false;
    for each (long id in m_keyToRows->Keys)
    {
        unsigned int iId = (unsigned int)id;
        if (bNext)
        {
            *piNextKey = iId;
            return OK;
        }

        if (iKey == iId)
        {
            bNext = true;
        }
    }

    return bNext ? ERROR_DATA_NOT_FOUND : ERROR_UNKNOWN_ROW_KEY;
}

int CachedTable::GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    return GetEqualKeys(&pszColumn, &vData, 1, ppiKey, piNumKeys);
}

int CachedTable::GetEqualKeys(const char** ppszColumn, const Variant* pvData, unsigned int iNumColumns, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    Trace(TRACE_VERBOSE, "CachedTable :: GetEqualKeys :: %s", m_result->TableName);

    if (ppiKey)
        *ppiKey = NULL;
    *piNumKeys = 0;

    Vector<unsigned int> vectKeys;

    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        bool bMatched = true;
        for (unsigned int i = 0; i < iNumColumns; i ++)
        {
            System::String^ columnName = gcnew System::String(ppszColumn[i]);
            System::Object^ value = row[columnName];
            Variant vValue;
            Convert(value, &vValue);
            if (vValue != pvData[i])
            {
                bMatched = false;
                break;
            }
        }

        if (bMatched)
        {
            System::Object^ id = row[m_ID_COLUMN_NAME];
            vectKeys.Add((unsigned int)(int64)id);
        }
    }

    *piNumKeys = vectKeys.GetNumElements();
    if (*piNumKeys == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    if (ppiKey)
    {
        *ppiKey = new unsigned int[*piNumKeys];
        Assert(*ppiKey);
        memcpy(*ppiKey, vectKeys.GetData(), *piNumKeys * sizeof(unsigned int));
    }
    return OK;
}

int CachedTable::GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys)
{
    Trace(TRACE_VERBOSE, "CachedTable :: GetAllKeys :: %s", m_result->TableName);

    unsigned int iNumRows = m_result->Rows->Count;

    *ppiKey = NULL;
    *piNumKeys = iNumRows;

    if (iNumRows == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }

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
    Trace(TRACE_VERBOSE, "CachedTable :: ReadColumn %s :: %s", m_result->TableName, columnName);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    *ppiData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    int* piData = new int[iNumRows];
    Assert(piData);

    if (ppiKey != NULL)
    {
        *ppiKey = new unsigned int[iNumRows];
        Assert(*ppiKey);
    }

    int index = 0;
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        if (ppiKey != NULL)
        {
            System::Object^ id = row[m_ID_COLUMN_NAME];
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
    Trace(TRACE_VERBOSE, "CachedTable :: ReadColumn %s :: %s", m_result->TableName, columnName);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    *ppvData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    Variant* pvData = new Variant[iNumRows];
    Assert(pvData);

    if (ppiKey != NULL)
    {
        *ppiKey = new unsigned int[iNumRows];
        Assert(*ppiKey);
    }

    int index = 0;
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        if (ppiKey != NULL)
        {
            System::Object^ id = row[m_ID_COLUMN_NAME];
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
    Trace(TRACE_VERBOSE, "CachedTable :: ReadColumns %s", m_result->TableName);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    *pppvData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    Variant** ppvData = new Variant*[iNumRows];
    Assert(ppvData);

    Variant* pvData = new Variant[iNumRows * iNumColumns];
    Assert(pvData);

    if (ppiKey != NULL)
    {
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
            System::Object^ id = row[m_ID_COLUMN_NAME];
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

    Trace(TRACE_VERBOSE, "CachedTable :: ReadColumnWhereEqual %s :: %s :: %s", m_result->TableName, pszEqualColumn, pszReadColumn);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    *ppvData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    Variant* pvData = new Variant[iNumRows];
    Assert(pvData);

    if (ppiKey != NULL)
    {
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
                System::Object^ id = row[m_ID_COLUMN_NAME];
                (*ppiKey)[index] = (unsigned int)(int64)id;
            }

            Convert(row[readColumnName], pvData + index);
            index ++;
        }
    }

    *piNumRows = index;
    if (index > 0)
    {
        *ppvData = pvData;
    }
    else
    {
        delete [] pvData;
        if (ppiKey)
        {
            delete [] (*ppiKey);
            *ppiKey = NULL;
        }
    }
    return OK;
}

int CachedTable::ReadColumnsWhereEqual(const char* pszEqualColumn, const Variant& vData, const char** ppszReadColumn, unsigned int iNumReadColumns,
                                       unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows)
{
    Trace(TRACE_VERBOSE, "CachedTable :: ReadColumnsWhereEqual %s :: %s", m_result->TableName, pszEqualColumn);

    if (ppiKey != NULL)
        *ppiKey = NULL;

    if (pppvData)
        *pppvData = NULL;

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);
    *piNumRows = iNumRows;
    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    Variant** ppvData = NULL, * pvData = NULL;
    if (pppvData)
    {
        ppvData = new Variant*[iNumRows];
        Assert(ppvData);

        pvData = new Variant[iNumRows * iNumReadColumns];
        Assert(pvData);
    }

    array<System::String^>^ columnNames = gcnew array<System::String^>(iNumReadColumns);
    for (unsigned int i = 0; i < iNumReadColumns; i ++)
    {
        columnNames[i] = gcnew System::String(ppszReadColumn[i]);
    }

    if (ppiKey)
    {
        *ppiKey = new unsigned int[iNumRows];
        Assert(*ppiKey);
    }

    System::String^ equalColumnName = gcnew System::String(pszEqualColumn);

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
                System::Object^ id = row[m_ID_COLUMN_NAME];
                (*ppiKey)[index] = (unsigned int)(int64)id;
            }

            if (pppvData)
            {
                ppvData[index] = pvData + index * iNumReadColumns;
                for (unsigned int column = 0; column < iNumReadColumns; column ++)
                {
                    System::Object^ value = row[columnNames[column]];
                    Convert(value, ppvData[index] + column);
                }
            }

            index ++;
        }
    }

    *piNumRows = index;
    if (index > 0)
    {
        if (pppvData)
            *pppvData = ppvData;
    }
    else
    {
        delete [] pvData;
        delete [] ppvData;
        if (ppiKey)
        {
            delete [] (*ppiKey);
            *ppiKey = NULL;
        }
    }

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
    Trace(TRACE_VERBOSE, "CachedTable :: ReadRow %s", m_result->TableName);

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

    int index = 0;
    for each (KeyValuePair<System::String^, System::Object^>^ pair in row)
    {
        // Skip id
        if (pair->Key != m_ID_COLUMN_NAME)
        {
            Convert(pair->Value, pvData + index ++);
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
    Trace(TRACE_VERBOSE, "CachedTable :: ReadData %s :: %s", m_result->TableName, columnName);

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
    Trace(TRACE_VERBOSE, "CachedTable :: InsertRow %s", m_result->TableName);
    return InsertDuplicateRows(ttTemplate, pvColVal, 1, piKey);
}

int CachedTable::InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows)
{
    Trace(TRACE_VERBOSE, "CachedTable :: InsertDuplicateRows %s", m_result->TableName);
    return InsertDuplicateRows(ttTemplate, pvColVal, iNumRows, NULL);
}

int CachedTable::InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows, unsigned int* piKey)
{
    if (piKey)
        *piKey = NO_KEY;

    // Insert rows into the database
    array<InsertValue>^ values = gcnew array<InsertValue>(ttTemplate.NumColumns);
    for (unsigned int i = 0; i < ttTemplate.NumColumns; i ++)
    {
        values[i].Type = Convert(ttTemplate.Type[i]);
        values[i].Value = Convert(pvColVal[i]);
    }
    
    List<IEnumerable<InsertValue>^>^ insert = gcnew List<IEnumerable<InsertValue>^>();
    for (unsigned int i = 0; i < iNumRows; i ++)
    {
        insert->Add(values);
    }

    IList<int64>^ ids;
    try
    {
        ids = m_cmd->Insert(m_result->TableName, insert);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }

    if (iNumRows == 1 && piKey)
    {
        *piKey = (unsigned int)Enumerable::First(ids);
    }

    // Make a new template row for insertion
    Dictionary<System::String^, System::Object^>^ templateRow = gcnew Dictionary<System::String^, System::Object^>();
    for (unsigned int i = 0; i < ttTemplate.NumColumns; i ++)
    {
        System::String^ columnName = gcnew System::String(ttTemplate.ColumnNames[i]);
        templateRow[columnName] = values[i].Value;
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
        insertRow[m_ID_COLUMN_NAME] = id;
        
        m_result->Rows->Add(insertRow);
        m_keyToRows->Add(id, insertRow);
    }

    return OK;
}

int CachedTable::DeleteRow(unsigned int iKey)
{
    Trace(TRACE_VERBOSE, "CachedTable :: DeleteRows %s", m_result->TableName);

    // First, try to delete the actual row
    // We can't defer this to commit time if we want unique constraints to work
    try
    {
        m_cmd->DeleteRow(m_result->TableName, m_ID_COLUMN_NAME, (int64)iKey);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }

    // Second, remove the row from cache; if this fails, it's because the row never existed
    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (iErrCode == OK)
    {
        m_keyToRows->Remove(iKey);
        m_result->Rows->Remove(row);
    }

    return iErrCode;
}

int CachedTable::DeleteAllRows()
{
    Trace(TRACE_VERBOSE, "CachedTable :: DeleteAllRows %s", m_result->TableName);
    try
    {
        if (m_bCompleteTable)
        {
            m_cmd->DeleteAllRows(m_result->TableName);
        }
        else
        {
            m_cmd->DeleteRows(m_result->TableName, m_ID_COLUMN_NAME, m_keyToRows->Keys);
        }
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }

    // Second, flush the cache
    m_keyToRows->Clear();
    m_result->Rows->Clear();

    return OK;
}

void CachedTable::SaveWrite(int64 id, System::String^ columnName, System::Object^ value)
{
    IDictionary<System::String^, System::Object^>^ writeRow;
    if (!m_writes->TryGetValue(id, writeRow))
    {
        writeRow = gcnew Dictionary<System::String^, System::Object^>();
        m_writes->Add(id, writeRow);
    }
    writeRow[columnName] = value;
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
    Trace(TRACE_VERBOSE, "CachedTable :: Increment %s :: %s", m_result->TableName, columnName);

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
    {
        Convert(value, pvOldValue);
    }

    SaveWrite(iKey != NO_KEY ? iKey : (unsigned int)(int64)row[m_ID_COLUMN_NAME], columnName, newValue);
    return OK;
}

int CachedTable::WriteData(const char* pszColumn, int iData)
{
    return WriteData(NO_KEY, pszColumn, iData);
}

int CachedTable::WriteData(const char* pszColumn, float fData)
{
    return WriteData(NO_KEY, pszColumn, fData);
}

int CachedTable::WriteData(const char* pszColumn, int64 i64Data)
{
    return WriteData(NO_KEY, pszColumn, i64Data);
}

int CachedTable::WriteData(const char* pszColumn, const char* pszData)
{
    return WriteData(NO_KEY, pszColumn, pszData);
}

int CachedTable::WriteData(const char* pszColumn, const Variant& vData)
{
    return WriteData(NO_KEY, pszColumn, vData);
}

int CachedTable::WriteData(unsigned int iKey, const char* pszColumn, int iData)
{
    Variant vData = iData;
    return WriteData(iKey, pszColumn, vData);
}

int CachedTable::WriteData(unsigned int iKey, const char* pszColumn, float fData)
{
    Variant vData = fData;
    return WriteData(iKey, pszColumn, vData);
}

int CachedTable::WriteData(unsigned int iKey, const char* pszColumn, int64 i64Data)
{
    Variant vData = i64Data;
    return WriteData(iKey, pszColumn, vData);
}

int CachedTable::WriteData(unsigned int iKey, const char* pszColumn, const char* pszData)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace(TRACE_VERBOSE, "CachedTable :: WriteData %s :: %s", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    System::String^ newValue = gcnew System::String(pszData);
    row[columnName] = newValue;
    
    SaveWrite(iKey != NO_KEY ? iKey : (unsigned int)(int64)row[m_ID_COLUMN_NAME], columnName, newValue);
    return OK;
}

int CachedTable::WriteData(unsigned int iKey, const char* pszColumn, const Variant& vData)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace(TRACE_VERBOSE, "CachedTable :: WriteData %s :: %s", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    System::Object^ newValue = Convert(vData);
    row[columnName] = newValue;

    SaveWrite(iKey != NO_KEY ? iKey : (unsigned int)(int64)row[m_ID_COLUMN_NAME], columnName, newValue);
    return OK;
}

int CachedTable::WriteAnd(const char* pszColumn, unsigned int iBitField)
{
    return WriteAnd(NO_KEY, pszColumn, iBitField);
}

int CachedTable::WriteAnd(unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace(TRACE_VERBOSE, "CachedTable :: WriteAnd %s :: %s", m_result->TableName, columnName);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    int iValue = (int)row[columnName];
    System::Object^ newValue = int(iValue & iBitField);
    row[columnName] = newValue;

    SaveWrite(iKey != NO_KEY ? iKey : (unsigned int)(int64)row[m_ID_COLUMN_NAME], columnName, newValue);

    return OK;
}

int CachedTable::WriteOr(const char* pszColumn, unsigned int iBitField)
{
    return WriteOr(NO_KEY, pszColumn, iBitField);
}

int CachedTable::WriteOr(unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    Trace(TRACE_VERBOSE, "CachedTable :: WriteOr(%s) :: %s", m_result->TableName, pszColumn);

    int iErrCode;
    IDictionary<System::String^, System::Object^>^ row = GetRow(iKey, &iErrCode);
    if (row == nullptr)
    {    
        return iErrCode;
    }

    System::String^ columnName = gcnew System::String(pszColumn);
    int iValue = (int)row[columnName];
    System::Object^ newValue = int(iValue | iBitField);
    row[columnName] = newValue;

    SaveWrite(iKey != NO_KEY ? iKey : (unsigned int)(int64)row[m_ID_COLUMN_NAME], columnName, newValue);
    return OK;
}