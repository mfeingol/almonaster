#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"
#include "CachedTable.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

#include "Osal/HashTable.h"

class TableCacheHashValue
{
public:
    static unsigned int GetHashValue(const char* pszData, unsigned int iNumBuckets, const void* pHashHint);
};

class TableCacheEquals
{
public:
    static bool Equals(const char* pszLeft, const char* pszRight, const void* pEqualsHint);
};

class TableCacheCollection : public ICachedTableCollection
{
private:
    gcroot<SqlCommandManager^> m_cmd;

    HashTable<char*, ICachedTable*, TableCacheHashValue, TableCacheEquals> m_htTableViews;

    CachedTable* CreateEmptyTable(const char* pszCacheTableName);
    void InsertTable(const char* pszCacheTableName, ICachedTable* pTable);

public:

    // ICachedTableCollection
    IMPLEMENT_INTERFACE(ICachedTableCollection);

    TableCacheCollection(SqlCommandManager^ cmd);
    ~TableCacheCollection();

    int CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate);

    int Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries);
    int Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries, unsigned int* ppiKey);

    int GetTable(const char* pszCacheTableName, ICachedTable** ppTable);

    int GetNumCachedRows(const char* pszCacheTableName, unsigned int* piNumRows);

    int GetFirstKey(const char* pszCacheTableName, const char* pszColumn, const Variant& vData, unsigned int* piKey);
    int GetNextKey(const char* pszCacheTableName, unsigned int iKey, unsigned int* piNextKey);
    int GetAllKeys(const char* pszCacheTableName, unsigned int** ppiKey, unsigned int* piNumKeys);

    int ReadColumn(const char* pszCacheTableName, const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns(const char* pszCacheTableName, unsigned int iNumColumns, const char* const* ppszColumn,
                    unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);

    int ReadRow(const char* pszCacheTableName, unsigned int iKey, Variant** ppvData);

    int ReadData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, Variant* pvData);
    int ReadData(const char* pszCacheTableName, const char* pszColumn, Variant* pvData);

    int InsertRow(const char* pszCacheTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey);
    int InsertDuplicateRows(const char* pszCacheTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows);

    int Increment(const char* pszCacheTableName, const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszCacheTableName, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);
    int Increment(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);

    int WriteData(const char* pszCacheTableName, const char* pszColumn, const char* pszData);
    int WriteData(const char* pszCacheTableName, const char* pszColumn, const Variant& vData);
    int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const char* pszData);
    int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vData);

    int WriteAnd(const char* pszCacheTableName, const char* pszColumn, unsigned int iBitField);
    int WriteAnd(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteOr(const char* pszCacheTableName, const char* pszColumn, unsigned int iBitField);
    int WriteOr(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField);

    // Memory
    void FreeData(Variant* pvData);
    void FreeData(Variant** ppvData);

    void FreeData(int* piData);
    void FreeData(float* ppfData);
    void FreeData(int64* pi64Data);

    void FreeKeys(unsigned int* piKeys);
};