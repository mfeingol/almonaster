#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

#include "Osal/HashTable.h"

class TableViewHashValue
{
public:
    static unsigned int GetHashValue(const char* pszData, unsigned int iNumBuckets, const void* pHashHint);
};

class TableViewEquals
{
public:
    static bool Equals(const char* pszLeft, const char* pszRight, const void* pEqualsHint);
};

class TableViewCollection : public ITableViewCollection
{
private:
    gcroot<SqlCommandManager^> m_cmd;

    HashTable<char*, IReadTableView*, TableViewHashValue, TableViewEquals> m_htTableViews;

public:

    // ITableViewCollection
    IMPLEMENT_INTERFACE(ITableViewCollection);

    TableViewCollection(SqlCommandManager^ cmd);
    ~TableViewCollection();

    int CreateViews(const char** ppszTableName, const char** ppszViewName, const unsigned int* ppiKey, unsigned int iNumTables);

    int GetTableForReading(const char* pszViewName, IReadTableView** ppTable);

    int GetAllKeys(const char* pszViewName, unsigned int** ppiKey, unsigned int* piNumKeys);

    int ReadRow(const char* pszViewName, unsigned int iKey, Variant** ppvData);

    int ReadData(const char* pszViewName, unsigned int iKey, const char* pszColumn, Variant* pvData);
    int ReadData(const char* pszViewName, const char* pszColumn, Variant* pvData);
};