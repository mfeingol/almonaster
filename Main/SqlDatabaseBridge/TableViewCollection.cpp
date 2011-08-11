#include "TableViewCollection.h"
#include "ReadTableView.h"
#include "Utils.h"

using namespace System::Collections::Generic;

unsigned int TableViewHashValue::GetHashValue (const char* pszData, unsigned int iNumBuckets, const void* pHashHint)
{
    return Algorithm::GetStringHashValue(pszData, iNumBuckets, true);
}

bool TableViewEquals::Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint)
{
    return String::StriCmp(pszLeft, pszRight) == 0;
}

TableViewCollection::TableViewCollection(SqlCommandManager^ cmd)
    :
    m_iNumRefs(1),
    m_htTableViews(NULL, NULL)
{
    m_cmd = cmd;

    bool ret = m_htTableViews.Initialize(256);
    Assert(ret);
}

TableViewCollection::~TableViewCollection()
{
    HashTableIterator<char*, IReadTableView*> htiView;
    while (m_htTableViews.GetNextIterator(&htiView))
    {
        OS::HeapFree(htiView.GetKey());
        htiView.GetData()->Release();
    }
}

int TableViewCollection::CreateViews(const char** ppszTableName, const char** ppszViewName, const unsigned int* ppiKey, unsigned int iNumViews)
{
    array<BulkTableReadRequest>^ requests = gcnew array<BulkTableReadRequest>(iNumViews);
    
    for (unsigned int i = 0; i < iNumViews; i ++)
    {
        requests[i].TableName = gcnew System::String(ppszTableName[i]);
        if (ppiKey[i] == NO_KEY)
        {
            requests[i].ColumnName = nullptr;
            requests[i].ColumnValue = nullptr;
        }
        else
        {
            requests[i].ColumnName = gcnew System::String(IdColumnName);
            requests[i].ColumnValue = ppiKey[i];
        }
    }

    IEnumerable<BulkTableReadResult^>^ results = m_cmd->BulkRead(requests);

    int index = 0;
    for each (BulkTableReadResult^ result in results)
    {
        char* pszViewName = String::StrDup(ppszViewName[index++]);
        Assert(pszViewName);

        ReadTableView* pView = new ReadTableView(result);
        Assert(pView);

        bool ret = m_htTableViews.Insert(pszViewName, pView);
        Assert(ret);
    }

    return OK;
}

int TableViewCollection::GetTableForReading(const char* pszViewName, IReadTableView** ppTable)
{
    if (m_htTableViews.FindFirst((char* const)pszViewName, ppTable))
    {
        (*ppTable)->AddRef();
        return OK;
    }

    *ppTable = NULL;
    return ERROR_UNKNOWN_TABLE_NAME;
}

// ...

int TableViewCollection::ReadRow(const char* pszViewName, unsigned int iKey, Variant** ppvData)
{
    IReadTableView* pTable;
    int iErrCode = GetTableForReading(pszViewName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->ReadRow(iKey, ppvData);

    SafeRelease(pTable);
    return iErrCode;
}

int TableViewCollection::GetAllKeys(const char* pszViewName, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    IReadTableView* pTable;
    int iErrCode = GetTableForReading(pszViewName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->GetAllKeys(ppiKey, piNumKeys);

    SafeRelease(pTable);
    return iErrCode;
}

int TableViewCollection::ReadData(const char* pszViewName, unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    IReadTableView* pTable;
    int iErrCode = GetTableForReading(pszViewName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->ReadData(iKey, pszColumn, pvData);

    SafeRelease(pTable);
    return iErrCode;
}

int TableViewCollection::ReadData(const char* pszViewName, const char* pszColumn, Variant* pvData)
{
    return ReadData(pszViewName, NO_KEY, pszColumn, pvData);
}