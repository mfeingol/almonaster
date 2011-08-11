#include "ReadTableView.h"
#include "Utils.h"

using namespace System::Collections::Generic;
using namespace System::Linq;

ReadTableView::ReadTableView(BulkTableReadResult^ result)
    :
    m_iNumRefs(1),
    m_result(result)
{
}

int ReadTableView::GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys)
{
    Trace("View :: GetAllKeys :: {0}", m_result->TableName);

    unsigned int iNumRows = Enumerable::Count(m_result->Rows);

    *ppiKey = NULL;
    *piNumKeys = iNumRows;

    if (iNumRows == 0)
        return ERROR_DATA_NOT_FOUND;

    System::String^ idCol = gcnew System::String(IdColumnName);
    unsigned int* piKey = new unsigned int[iNumRows];
    Assert(piKey != NULL);

    int index = 0;
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        System::Object^ id = row[idCol];
        piKey[index ++] = (unsigned int)(int64)id;
    }

    *ppiKey = piKey;
    return OK;
}

int ReadTableView::ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows)
{
    System::String^ columnName = gcnew System::String(pszColumn);
    Trace("View :: ReadColumn {0} :: {1}", m_result->TableName, columnName);

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

int ReadTableView::ReadRow(unsigned int iKey, Variant** ppvData)
{
    Trace("View :: ReadRow {0}", m_result->TableName);

    IDictionary<System::String^, System::Object^>^ row;

    if (iKey == NO_KEY)
    {
        if (Enumerable::Count(m_result->Rows) > 1)
            return ERROR_TABLE_HAS_MORE_THAN_ONE_ROW;
        row = Enumerable::First(m_result->Rows);
    }
    else
    {
        // TODO - Index keys if tables get large...
        System::String^ idCol = gcnew System::String(IdColumnName);
        for each (IDictionary<System::String^, System::Object^>^ tryRow in m_result->Rows)
        {
            System::Object^ id = tryRow[idCol];
            if ((unsigned int)(int64)id == iKey)
            {
                row = tryRow;
                break;
            }
        }
    }

    if (row == nullptr)
        return ERROR_UNKNOWN_ROW_KEY;

    int cols = Enumerable::Count(row->Values) - 1;
    Variant* pvData = new Variant[cols];
    Assert(pvData);

    int index = 0;
    for each (System::Object^ obj in row->Values)
    {
        // Skip id - we assume it's the zeroth element
        if (index > 0)
            Convert(obj, pvData + index - 1);
        index ++;
    }

    *ppvData = pvData;
    return OK;
}

int ReadTableView::ReadData(unsigned int iKey, const char* pszColumn, int* piData)
{
    Variant vData;
    int iErrCode = ReadData(iKey, pszColumn, &vData);
    if (iErrCode != OK)
        return iErrCode;

    *piData = vData.GetInteger();
    return OK;
}

int ReadTableView::ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data)
{
    Variant vData;
    int iErrCode = ReadData(iKey, pszColumn, &vData);
    if (iErrCode != OK)
        return iErrCode;

    *pi64Data = vData.GetInteger64();
    return OK;
}

int ReadTableView::ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    Trace("View :: ReadData {0} :: {1}", m_result->TableName, gcnew System::String(pszColumn));

    if (iKey == NO_KEY)
    {
        if (Enumerable::Count(m_result->Rows) > 1)
            return ERROR_TABLE_HAS_MORE_THAN_ONE_ROW;

        Convert(Enumerable::First(m_result->Rows)[gcnew System::String(pszColumn)], pvData);
        return OK;
    }

    System::String^ idCol = gcnew System::String(IdColumnName);
    for each (IDictionary<System::String^, System::Object^>^ row in m_result->Rows)
    {
        System::Object^ id = row[idCol];
        if ((unsigned int)(int64)id == iKey)
        {
            Convert(row[gcnew System::String(pszColumn)], pvData);
            return OK;
        }
    }

    return ERROR_UNKNOWN_ROW_KEY;
}