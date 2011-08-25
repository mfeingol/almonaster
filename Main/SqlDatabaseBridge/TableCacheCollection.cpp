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

        ICachedTable* pTable = htiView.GetData();
        SafeRelease(pTable);
    }
}

CachedTable* TableCacheCollection::CreateEmptyTable(const char* pszCacheTableName)
{
    BulkTableReadResult^ result = gcnew BulkTableReadResult();
    result->TableName = gcnew System::String(pszCacheTableName);
    result->Rows = gcnew List<IDictionary<System::String^, System::Object^>^>();

    CachedTable* pTable = new CachedTable(m_cmd, result);
    Assert(pTable);
    return pTable;
}

void TableCacheCollection::InsertTable(const char* pszCacheTableName, CachedTable* pTable)
{
    char* pszKey = String::StrDup(pszCacheTableName);
    Assert(pszKey);

    pTable->AddRef();

    bool ret = m_htTableViews.Insert(pszKey, pTable);
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
    CachedTable* pTable = CreateEmptyTable(pszTableName);
    InsertTable(pszTableName, pTable);
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
    System::String^ strIdColumnName = gcnew System::String(IdColumnName);

    if (ppTable)
        *ppTable = NULL;

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
                // TODOTODO - really?
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

    // TODOTODOTODO - Handle zero results appropriately

    unsigned int iKey = NO_KEY;
    int index = 0;
    CachedTable* pTable = NULL;
    for each (BulkTableReadResult^ result in results)
    {
        pTable = new CachedTable(m_cmd, result);
        Assert(pTable);

        bool ret = m_htTableViews.Insert(ppszCacheEntryName[index++], pTable);
        Assert(ret);

        if (pcCacheEntry->iNumColumns > 0 && Enumerable::Count(result->Rows) == 1)
        {
            // One row found via column search, so allow the table to be looked up by key as well
            iKey = (unsigned int)(int64)Enumerable::First(result->Rows)[strIdColumnName];

            char* pszCacheEntryName = (char*)StackAlloc((result->TableName->Length + 32) * sizeof(char));
            sprintf(pszCacheEntryName, "%s%i", result->TableName, iKey);

            pTable->AddRef();
            bool ret = m_htTableViews.Insert(String::StrDup(pszCacheEntryName), pTable);
            Assert(ret);
        }
    }

    // If there happens to be one result, and you wanted it, here it is...
    if (ppTable && Enumerable::Count(results) == 1)
    {
        pTable->AddRef();
        *ppTable = pTable;
    }

    return OK;
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