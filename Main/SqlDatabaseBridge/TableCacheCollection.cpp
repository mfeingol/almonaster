#include "TableCacheCollection.h"
#include "CachedReadTable.h"
#include "Utils.h"

using namespace System::Collections::Generic;
using namespace System::Linq;

unsigned int TableCacheHashValue::GetHashValue (const char* pszData, unsigned int iNumBuckets, const void* pHashHint)
{
    return Algorithm::GetStringHashValue(pszData, iNumBuckets, true);
}

bool TableCacheEquals::Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint)
{
    return String::StriCmp(pszLeft, pszRight) == 0;
}

TableCacheCollection::TableCacheCollection(SqlCommandManager^ cmd)
    :
    m_iNumRefs(1),
    m_htTableViews(NULL, NULL)
{
    m_cmd = cmd;

    bool ret = m_htTableViews.Initialize(256);
    Assert(ret);
}

TableCacheCollection::~TableCacheCollection()
{
    HashTableIterator<char*, ICachedReadTable*> htiView;
    while (m_htTableViews.GetNextIterator(&htiView))
    {
        char* pszKey = htiView.GetKey();
        OS::HeapFree(pszKey);

        ICachedReadTable* pTable = htiView.GetData();
        SafeRelease(pTable);
    }
}

int TableCacheCollection::Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries)
{
    return Cache(pcCacheEntry, iNumEntries, NULL);
}

int TableCacheCollection::Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries, unsigned int* piKey)
{
    System::String^ strIdColumnName = gcnew System::String(IdColumnName);

    if (piKey)
        *piKey = NO_KEY;

    List<BulkTableReadRequest>^ requests = gcnew List<BulkTableReadRequest>(iNumEntries);
    char** ppszCacheEntryName = (char**)StackAlloc(iNumEntries * sizeof(char*));
    
    unsigned int iActual = 0;
    for (unsigned int i = 0; i < iNumEntries; i ++)
    {
        String s;
        const char* pszTableName = pcCacheEntry[i].pszTableName;
        unsigned int iNumColumns;

        // Might already be cached
        char* pszCacheEntryName;
        if (pcCacheEntry[i].iKey == NO_KEY)
        {
            iNumColumns = pcCacheEntry[i].iNumColumns;
            if (iNumColumns == 0)
            {
                pszCacheEntryName = (char*)pszTableName;
            }
            else
            {
                s = pszTableName;
                for (unsigned int j = 0; j < pcCacheEntry[i].iNumColumns; j ++)
                {
                    s += "_";
                    s += pcCacheEntry[i].pcColumns[j].pszColumn;
                    s += "_";
                    s += (String)pcCacheEntry[i].pcColumns[j].vData;
                }
                pszCacheEntryName = s.GetCharPtr();
            }
        }
        else
        {
            Assert(pcCacheEntry[i].iNumColumns == 0);
            iNumColumns = 1;

            pszCacheEntryName = (char*)StackAlloc((strlen(pszTableName) + 32) * sizeof(char));
            sprintf(pszCacheEntryName, "%s%i", pszTableName, pcCacheEntry[i].iKey);
        }

        if (m_htTableViews.Contains(pszCacheEntryName))
            continue;

        array<BulkTableReadRequestColumn>^ columns = gcnew array<BulkTableReadRequestColumn>(iNumColumns);

        BulkTableReadRequest request;
        request.TableName = gcnew System::String(pszTableName);
        request.Columns = columns;

        if (pcCacheEntry[i].iKey == NO_KEY)
        {
            for (unsigned int j = 0; j < pcCacheEntry[i].iNumColumns; j ++)
            {
                columns[j].ColumnName = gcnew System::String(pcCacheEntry[i].pcColumns[j].pszColumn);
                columns[j].ColumnValue = Convert(pcCacheEntry[i].pcColumns[j].vData);
            }
        }
        else
        {
            columns[0].ColumnName = strIdColumnName;
            columns[0].ColumnValue = (int64)pcCacheEntry[i].iKey;
        }

        // Add request to list
        ppszCacheEntryName[iActual] = String::StrDup(pszCacheEntryName);
        Assert(ppszCacheEntryName[iActual]);

        requests->Add(request);
        iActual ++;
    }

    if (iActual == 0)
        return OK;

    IEnumerable<BulkTableReadResult^>^ results = m_cmd->BulkRead(requests);

    // TODOTODOTODO - Handle no results appropriately

    unsigned int iKey = NO_KEY;
    int index = 0;
    for each (BulkTableReadResult^ result in results)
    {
        CachedReadTable* pView = new CachedReadTable(result);
        Assert(pView);

        bool ret = m_htTableViews.Insert(ppszCacheEntryName[index++], pView);
        Assert(ret);

        if (pcCacheEntry->iNumColumns > 0 && Enumerable::Count(result->Rows) == 1)
        {
            // One row found via column search, so allow the table to be looked up by key as well
            iKey = (unsigned int)(int64)Enumerable::First(result->Rows)[strIdColumnName];

            char* pszCacheEntryName = (char*)StackAlloc((result->TableName->Length + 32) * sizeof(char));
            sprintf(pszCacheEntryName, "%s%i", result->TableName, iKey);

            pView->AddRef();
            bool ret = m_htTableViews.Insert(String::StrDup(pszCacheEntryName), pView);
            Assert(ret);
        }
    }

    // If there happens to be one result, and you wanted it, here it is...
    if (piKey && Enumerable::Count(results) == 1)
    {
        if (piKey)
            *piKey = iKey;
    }

    return OK;
}

int TableCacheCollection::GetTableForReading(const char* pszCacheTableName, ICachedReadTable** ppTable)
{
    if (m_htTableViews.FindFirst((char* const)pszCacheTableName, ppTable))
    {
        (*ppTable)->AddRef();
        return OK;
    }

    // TODO - we'll handle a cache miss more gracefully in the future. For now, let's not have any.
    Assert(false);

    *ppTable = NULL;
    return ERROR_UNKNOWN_TABLE_NAME;
}

// ...

int TableCacheCollection::GetNumCachedRows(const char* pszCacheTableName, unsigned int* piNumRows)
{
    *piNumRows = 0;

    ICachedReadTable* pTable;
    int iErrCode = GetTableForReading(pszCacheTableName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->GetNumCachedRows(piNumRows);

    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadColumn(const char* pszCacheTableName, const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    if (ppiKey)
        *ppiKey = NULL;

    *ppvData = NULL;
    *piNumRows = 0;

    ICachedReadTable* pTable;
    int iErrCode = GetTableForReading(pszCacheTableName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->ReadColumn(pszColumn, ppiKey, ppvData, piNumRows);

    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadColumns(const char* pszCacheTableName, unsigned int iNumColumns, const char* const* ppszColumn,
                                     unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows)
{
    if (ppiKey)
        *ppiKey = NULL;

    *pppvData = NULL;
    *piNumRows = 0;

    ICachedReadTable* pTable;
    int iErrCode = GetTableForReading(pszCacheTableName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->ReadColumns(iNumColumns, ppszColumn, ppiKey, pppvData, piNumRows);

    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadRow(const char* pszCacheTableName, unsigned int iKey, Variant** ppvData)
{
    *ppvData = NULL;

    ICachedReadTable* pTable;
    int iErrCode = GetTableForReading(pszCacheTableName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->ReadRow(iKey, ppvData);

    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::GetAllKeys(const char* pszCacheTableName, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    *ppiKey = NULL;
    *piNumKeys = 0;

    ICachedReadTable* pTable;
    int iErrCode = GetTableForReading(pszCacheTableName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->GetAllKeys(ppiKey, piNumKeys);

    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    ICachedReadTable* pTable;
    int iErrCode = GetTableForReading(pszCacheTableName, &pTable);
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = pTable->ReadData(iKey, pszColumn, pvData);

    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadData(const char* pszCacheTableName, const char* pszColumn, Variant* pvData)
{
    return ReadData(pszCacheTableName, NO_KEY, pszColumn, pvData);
}