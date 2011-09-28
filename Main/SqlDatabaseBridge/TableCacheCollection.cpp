#include "TableCacheCollection.h"
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
    m_strID_COLUMN_NAME = gcnew System::String(ID_COLUMN_NAME);

    bool ret = m_htTableViews.Initialize(256);
    Assert(ret);
}

TableCacheCollection::~TableCacheCollection()
{
    HashTableIterator<char*, CachedTable*> htiView;
    while (m_htTableViews.GetNextIterator(&htiView))
    {
        char* pszKey = htiView.GetKey();
        OS::HeapFree(pszKey);

        CachedTable* pTable = htiView.GetData();
        SafeRelease(pTable);
    }
}

int TableCacheCollection::Commit()
{
    List<BulkTableWriteRequest>^ requests = gcnew List<BulkTableWriteRequest>();

    HashTableIterator<char*, CachedTable*> htiView;
    while (m_htTableViews.GetNextIterator(&htiView))
    {
        CachedTable* pTable = htiView.GetData();
        IDictionary<int64, IDictionary<System::String^, System::Object^>^>^ writes = pTable->ObtainWrites();
        if (writes && writes->Count > 0)
        {
            BulkTableWriteRequest req = { pTable->GetResult()->TableName, writes };
            requests->Add(req);
        }
    }

    if (requests->Count > 0)
    {
        try
        {
            m_cmd->BulkWrite(requests, gcnew System::String(ID_COLUMN_NAME));
        }
        catch (SqlDatabaseException^)
        {
            return ERROR_FAILURE;
        }
    }
    return OK;

}

CachedTable* TableCacheCollection::CreateEmptyTable(const char* pszTableName, bool bCompleteTable)
{
    BulkTableReadResult^ result = gcnew BulkTableReadResult();
    result->TableName = gcnew System::String(pszTableName);
    result->Rows = gcnew List<IDictionary<System::String^, System::Object^>^>();

    CachedTable* pTable = new CachedTable(m_cmd, result, bCompleteTable);
    Assert(pTable);
    return pTable;
}

void TableCacheCollection::InsertTable(const char* pszCacheTableName, CachedTable* pTable)
{
    char* pszKey = String::StrDup(pszCacheTableName);
    Assert(pszKey);

    InsertTableNoDup(pszKey, pTable);
}

void TableCacheCollection::InsertTableNoDup(const char* pszCacheTableName, CachedTable* pTable)
{
    Trace("TableCacheCollection caching {0}", gcnew System::String(pszCacheTableName));

    pTable->AddRef();
    bool ret = m_htTableViews.Insert((char*)pszCacheTableName, pTable);
    Assert(ret);
}

// Table operations
int TableCacheCollection::CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate)
{
    Trace("TableCacheCollection::CreateTable {0}", gcnew System::String(pszTableName));

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

    // TODOTODO - Indexes
    // TODOTODO - Foreign keys

    try
    {
        m_cmd->CreateTable(tableDesc);
    }
    catch (SqlDatabaseException^)
    {
        // TODO - other errors
        return ERROR_TABLE_ALREADY_EXISTS;
    }

    // Add to cache
    CachedTable* pTable = CreateEmptyTable(pszTableName, true);
    InsertTable(pszTableName, pTable);
    SafeRelease(pTable);

    return OK;
}

int TableCacheCollection::CreateEmpty(const char* pszTableName, const char* pszCachedTableName)
{
    if (m_htTableViews.Contains((char*)pszCachedTableName))
        return ERROR_TABLE_ALREADY_EXISTS;

    CachedTable* pTable = CreateEmptyTable(pszTableName, false);
    InsertTable(pszCachedTableName, pTable);
    SafeRelease(pTable);

    return OK;
}

int TableCacheCollection::Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries)
{
    return Cache(pcCacheEntry, iNumEntries, NULL);
}

int TableCacheCollection::Cache(const TableCacheEntry& cCacheEntry, ICachedTable** ppTable)
{
    return Cache(&cCacheEntry, 1, ppTable);
}

int TableCacheCollection::Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries, ICachedTable** ppTable)
{
    if (ppTable)
        *ppTable = NULL;

    char** ppszCacheEntryName = (char**)StackAlloc(iNumEntries * sizeof(char*));
    const TableCacheEntry** ppcActualCacheEntry = (const TableCacheEntry**)StackAlloc(iNumEntries * sizeof(TableCacheEntry*));

    List<BulkTableReadRequest>^ requests = gcnew List<BulkTableReadRequest>(iNumEntries);

    unsigned int iActual = 0;
    for (unsigned int i = 0; i < iNumEntries; i ++)
    {
        char* pszEntryName = EnsureNewCacheEntry(pcCacheEntry[i]);
        if (pszEntryName == NULL)
            continue;

        ppszCacheEntryName[iActual] = pszEntryName;
        ppcActualCacheEntry[iActual] = pcCacheEntry + i;
        iActual ++;

        ConvertToRequest(pcCacheEntry[i], requests);
    }

    // Maybe we've already cached everything we wanted...
    if (iActual == 0)
        return OK;

    // Go to the database
    IEnumerable<BulkTableReadResult^>^ results;
    try
    {
        results = m_cmd->BulkRead(requests);
    }
    catch (SqlDatabaseException^)
    {
        // TODOTODO - error code?
        return ERROR_FAILURE;
    }

    // Process results
    CachedTable* pTable = NULL;
    iActual = 0;
    for each (BulkTableReadResult^ result in results)
    {
        const TableCacheEntry& entry = *(ppcActualCacheEntry[iActual]);

        pTable = new CachedTable(m_cmd, result, entry.Table.NumColumns == 0 && entry.CrossJoin == NULL);
        Assert(pTable);

        InsertTableNoDup(ppszCacheEntryName[iActual], pTable);
        pTable->Release();

        if (entry.PartitionColumn)
        {
            CreateTablePartitions(result, ppszCacheEntryName[iActual], entry.PartitionColumn);
        }

        if (entry.Table.NumColumns > 0 && Enumerable::Count(result->Rows) == 1)
        {
            // One row found via column search, so allow the table to be looked up by key as well
            char* pszCacheEntryName = (char*)StackAlloc((result->TableName->Length + 32) * sizeof(char));
            unsigned int iKey = (unsigned int)(int64)Enumerable::First(result->Rows)[m_strID_COLUMN_NAME];
            sprintf(pszCacheEntryName, "%s_%s_%i", result->TableName, ID_COLUMN_NAME, iKey);

            InsertTable(pszCacheEntryName, pTable);
        }

        iActual ++;
    }

    // If there happens to be one result, and you wanted it, here it is...
    if (ppTable && Enumerable::Count(results) <= 1)
    {
        pTable->AddRef();
        *ppTable = pTable;
    }

    return OK;
}

void TableCacheCollection::CreateTablePartitions(BulkTableReadResult^ result, const char* pszCacheEntryName, const char* pszPartitionColumn)
{
    Dictionary<System::Object^, BulkTableReadResult^>^ partitionedResults = gcnew Dictionary<System::Object^, BulkTableReadResult^>();

    System::String^ columnName = gcnew System::String(pszPartitionColumn);
    for each (IDictionary<System::String^, System::Object^>^ row in result->Rows)
    {
        System::Object^ value = row[columnName];

        BulkTableReadResult^ newResult;
        if (!partitionedResults->TryGetValue(value, newResult))
        {
            newResult = gcnew BulkTableReadResult();
            newResult->TableName = result->TableName;
            newResult->Rows = gcnew List<IDictionary<System::String^, System::Object^>^>();
            partitionedResults[value] = newResult;
        }

        newResult->Rows->Add(row);
    }

    for each (KeyValuePair<System::Object^, BulkTableReadResult^>^ pair in partitionedResults)
    {
        Variant vValue;
        Convert(pair->Key, &vValue);
        String strValueText = vValue;
        Assert(strValueText.GetCharPtr());

        size_t cChars = strlen(pszCacheEntryName) + strlen(strValueText.GetCharPtr()) + strlen(pszCacheEntryName) + 3;
        char* pszPartitionCacheName = (char*)OS::HeapAlloc(cChars * sizeof(char));
        Assert(pszPartitionCacheName);
        sprintf(pszPartitionCacheName, "%s_%s_%s", pszCacheEntryName, pszPartitionColumn, strValueText.GetCharPtr());

        // We might have already processed the cache entry
        if (m_htTableViews.Contains(pszPartitionCacheName))
        {
            OS::HeapFree(pszPartitionCacheName);
        }
        else
        {
            CachedTable* pTable = new CachedTable(m_cmd, pair->Value, false);
            Assert(pTable);

            InsertTableNoDup(pszPartitionCacheName, pTable);
            SafeRelease(pTable);
        }
    }
}

array<BulkTableReadRequestColumn>^ TableCacheCollection::ConvertToRequestColumns(const TableEntry& table)
{
    array<BulkTableReadRequestColumn>^ columns;
    
    if (table.Key == NO_KEY)
    {
        columns = gcnew array<BulkTableReadRequestColumn>(table.NumColumns);
        for (unsigned int i = 0; i < table.NumColumns; i ++)
        {
            columns[i].ColumnName = gcnew System::String(table.Columns[i].Name);
            columns[i].ColumnValue = Convert(table.Columns[i].Data);
        }
    }
    else if (table.Key != NO_KEY)
    {
        columns = gcnew array<BulkTableReadRequestColumn>(1);
        columns[0].ColumnName = m_strID_COLUMN_NAME;
        columns[0].ColumnValue = (int64)table.Key;
    }

    return columns;
}

void TableCacheCollection::ConvertToRequest(const TableCacheEntry& entry, List<BulkTableReadRequest>^ requests)
{
    BulkTableReadRequest request;
    request.TableName = gcnew System::String(entry.Table.Name);
    request.Columns = ConvertToRequestColumns(entry.Table);

    if (entry.CrossJoin)
    {
        request.CrossJoin = gcnew CrossJoinRequest();
        request.CrossJoin->TableName = gcnew System::String(entry.CrossJoin->Table.Name);
        request.CrossJoin->LeftColumnName = entry.CrossJoin->LeftColumnName ? gcnew System::String(entry.CrossJoin->LeftColumnName) : m_strID_COLUMN_NAME;
        request.CrossJoin->RightColumnName = gcnew System::String(entry.CrossJoin->RightColumnName);
        request.CrossJoin->Columns = ConvertToRequestColumns(entry.CrossJoin->Table);
    }

    requests->Add(request);
}

char* TableCacheCollection::EnsureNewCacheEntry(const TableCacheEntry& entry)
{
    String s;
    char* pszCacheEntryName;

    // Compute cache entry name
    if (entry.Table.Key == NO_KEY)
    {
        if (entry.Table.NumColumns == 0)
        {
            pszCacheEntryName = (char*)entry.Table.Name;
        }
        else
        {
            s = entry.Table.Name;
            for (unsigned int i = 0; i < entry.Table.NumColumns; i ++)
            {
                s += "_";
                s += entry.Table.Columns[i].Name;
                s += "_";
                s += (String)entry.Table.Columns[i].Data;
            }
            pszCacheEntryName = s.GetCharPtr();
        }
    }
    else
    {
        Assert(entry.Table.NumColumns == 0);

        pszCacheEntryName = (char*)StackAlloc((strlen(entry.Table.Name) + 32) * sizeof(char));
        sprintf(pszCacheEntryName, "%s_%s_%i", entry.Table.Name, ID_COLUMN_NAME, entry.Table.Key);
    }

    // We might have already processed the cache entry
    if (m_htTableViews.Contains(pszCacheEntryName))
        return NULL;

    char* pszRet = String::StrDup(pszCacheEntryName);
    Assert(pszRet);
    return pszRet;
}

bool TableCacheCollection::IsCached(const char* pszCacheTableName)
{
    return m_htTableViews.Contains((char* const)pszCacheTableName);
}

int TableCacheCollection::GetTable(const char* pszCacheTableName, ICachedTable** ppTable)
{
    *ppTable = NULL;

    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        *ppTable = pTable;
    }
    return iErrCode;
}

int TableCacheCollection::GetTable(const char* pszCacheTableName, CachedTable** ppTable)
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

    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->GetNumCachedRows(piNumRows);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadColumn(const char* pszCacheTableName, const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    if (ppiKey)
        *ppiKey = NULL;

    *ppvData = NULL;
    *piNumRows = 0;

    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->ReadColumn(pszColumn, ppiKey, ppvData, piNumRows);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadColumnWhereEqual(const char* pszCacheTableName, const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                               unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    if (ppiKey)
        *ppiKey = NULL;

    *ppvData = NULL;
    *piNumRows = 0;

    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->ReadColumnWhereEqual(pszEqualColumn, vData, pszReadColumn, ppiKey, ppvData, piNumRows);
    }
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

    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->ReadColumns(iNumColumns, ppszColumn, ppiKey, pppvData, piNumRows);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadRow(const char* pszCacheTableName, unsigned int iKey, Variant** ppvData)
{
    *ppvData = NULL;

    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->ReadRow(iKey, ppvData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::GetFirstKey(const char* pszCacheTableName, const char* pszColumn, const Variant& vData, unsigned int* piKey)
{
    *piKey = NO_KEY;

    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->GetFirstKey(pszColumn, vData, piKey);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::GetNextKey(const char* pszCacheTableName, unsigned int iKey, unsigned int* piNextKey)
{
    *piNextKey = NO_KEY;

    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->GetNextKey(iKey, piNextKey);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::GetEqualKeys(const char* pszCacheTableName, const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    if (ppiKey)
        *ppiKey = NULL;
    *piNumKeys = 0;

    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->GetEqualKeys(pszColumn, vData, ppiKey, piNumKeys);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::GetEqualKeys(const char* pszCacheTableName, const char** ppszColumn, const Variant* pvData, unsigned int iNumColumns,
                                       unsigned int** ppiKey, unsigned int* piNumKeys)
{
    if (ppiKey)
        *ppiKey = NULL;
    *piNumKeys = 0;

    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->GetEqualKeys(ppszColumn, pvData, iNumColumns, ppiKey, piNumKeys);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::GetAllKeys(const char* pszCacheTableName, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    *ppiKey = NULL;
    *piNumKeys = 0;

    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->GetAllKeys(ppiKey, piNumKeys);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, Variant* pvData)
{
    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->ReadData(iKey, pszColumn, pvData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::ReadData(const char* pszCacheTableName, const char* pszColumn, Variant* pvData)
{
    return ReadData(pszCacheTableName, NO_KEY, pszColumn, pvData);
}

int TableCacheCollection::InsertRow(const char* pszCacheTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey)
{
    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->InsertRow(ttTemplate, pvColVal, piKey);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::InsertDuplicateRows(const char* pszCacheTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows)
{
    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->InsertDuplicateRows(ttTemplate, pvColVal, iNumRows);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::DeleteRow(const char* pszCacheTableName, unsigned int iKey)
{
    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->DeleteRow(iKey);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::DeleteAllRows(const char* pszCacheTableName)
{
    CachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->DeleteAllRows();
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::Increment(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->Increment(iKey, pszColumn, vIncrement);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::Increment(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->Increment(iKey, pszColumn, vIncrement, pvOldValue);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::Increment(const char* pszCacheTableName, const char* pszColumn, const Variant& vIncrement)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->Increment(pszColumn, vIncrement);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::Increment(const char* pszCacheTableName, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->Increment(pszColumn, vIncrement, pvOldValue);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, const char* pszColumn, int iData)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(pszColumn, iData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, const char* pszColumn, float fData)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(pszColumn, fData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, const char* pszColumn, int64 i64Data)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(pszColumn, i64Data);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, const char* pszColumn, const char* pszData)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(pszColumn, pszData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, const char* pszColumn, const Variant& vData)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(pszColumn, vData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, int iData)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(iKey, pszColumn, iData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, float fData)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(iKey, pszColumn, fData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, int64 i64Data)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(iKey, pszColumn, i64Data);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const char* pszData)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(iKey, pszColumn, pszData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vData)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteData(iKey, pszColumn, vData);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteAnd(const char* pszCacheTableName, const char* pszColumn, unsigned int iBitField)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteAnd(pszColumn, iBitField);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteAnd(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteAnd(iKey, pszColumn, iBitField);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteOr(const char* pszCacheTableName, const char* pszColumn, unsigned int iBitField)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteOr(pszColumn, iBitField);
    }
    SafeRelease(pTable);
    return iErrCode;
}

int TableCacheCollection::WriteOr(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField)
{
    ICachedTable* pTable;
    int iErrCode = GetTable(pszCacheTableName, &pTable);
    if (iErrCode == OK)
    {
        iErrCode = pTable->WriteOr(iKey, pszColumn, iBitField);
    }
    SafeRelease(pTable);
    return iErrCode;
}

void TableCacheCollection::FreeData(Variant* pvData)
{
    delete [] pvData;
}

void TableCacheCollection::FreeData(Variant** ppvData)
{
    if (ppvData)
    {
        delete [](*ppvData);
        delete [] ppvData;
    }
}

void TableCacheCollection::FreeKeys(unsigned int* piKeys)
{
    delete [] piKeys;
}

void TableCacheCollection::FreeData(int* piData)
{
    delete [] piData;
}

void TableCacheCollection::FreeData(float* ppfData)
{
    delete [] ppfData;
}

void TableCacheCollection::FreeData(int64* pi64Data)
{
    delete [] pi64Data;
}