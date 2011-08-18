#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"

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

    HashTable<char*, ICachedReadTable*, TableCacheHashValue, TableCacheEquals> m_htTableViews;

public:

    // ICachedTableCollection
    IMPLEMENT_INTERFACE(ICachedTableCollection);

    TableCacheCollection(SqlCommandManager^ cmd);
    ~TableCacheCollection();

    int Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries);
    int Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries, unsigned int* ppiKey);

    int GetTableForReading(const char* pszCacheTableName, ICachedReadTable** ppTable);

    int GetNumCachedRows(const char* pszCacheTableName, unsigned int* piNumRows);

    int GetAllKeys(const char* pszCacheTableName, unsigned int** ppiKey, unsigned int* piNumKeys);

    int ReadColumn(const char* pszCacheTableName, const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns(const char* pszCacheTableName, unsigned int iNumColumns, const char* const* ppszColumn,
                    unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);

    int ReadRow(const char* pszCacheTableName, unsigned int iKey, Variant** ppvData);

    int ReadData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, Variant* pvData);
    int ReadData(const char* pszCacheTableName, const char* pszColumn, Variant* pvData);
};