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

#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"
#include "CachedTable.h"

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
    gcroot<System::String^> m_strID_COLUMN_NAME;

    HashTable<char*, CachedTable*, TableCacheHashValue, TableCacheEquals> m_htTableViews;

    CachedTable* CreateEmptyTable(const char* pszTableName, bool bCompleteTable);
    void InsertTable(const char* pszCacheTableName, CachedTable* pTable);
    void InsertTableNoDup(const char* pszCacheTableName, CachedTable* pTable);

    int GetTable(const char* pszCacheTableName, CachedTable** ppTable);
    int Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries, ICachedTable** ppTable);

    bool EnsureNewCacheEntry(const TableCacheEntry& entry, CachedTable** ppCachedTable, String& strCacheEntry, String& strPartitionCachePrefix);
    array<BulkTableReadRequestColumn>^ ConvertToRequestColumns(const TableEntry& table);
    void ConvertToRequest(const TableCacheEntry& entry, List<BulkTableReadRequest>^ requests);
    void CreateTablePartitions(BulkTableReadResult^ result, const char* pszCacheEntryName, const char* pszPartitionColumn);

public:

    // ICachedTableCollection
    IMPLEMENT_INTERFACE(ICachedTableCollection);

    TableCacheCollection(SqlCommandManager^ cmd);
    ~TableCacheCollection();

    int Commit();

    int CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate);
    int CreateEmpty(const char* pszTableName, const char* pszCachedTableName);

    int Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries);
    int Cache(const TableCacheEntry& cCacheEntry, ICachedTable** ppTable);

    bool IsCached(const char* pszCacheTableName);
    int GetTable(const char* pszCacheTableName, ICachedTable** ppTable);

    int GetNumCachedRows(const char* pszCacheTableName, unsigned int* piNumRows);

    int GetFirstKey(const char* pszCacheTableName, const char* pszColumn, const Variant& vData, unsigned int* piKey);
    int GetNextKey(const char* pszCacheTableName, unsigned int iKey, unsigned int* piNextKey);
    int GetEqualKeys(const char* pszCacheTableName, const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumRows);
    int GetEqualKeys(const char* pszCacheTableName, const char** ppszColumn, const Variant* pvData, unsigned int iNumColumns, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetAllKeys(const char* pszCacheTableName, unsigned int** ppiKey, unsigned int* piNumRows);

    int ReadColumn(const char* pszCacheTableName, const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumnWhereEqual(const char* pszCacheTableName, const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                             unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys);
    
    int ReadColumnsWhereEqual(const char* pszCacheTableName, const char* pszEqualColumn, const Variant& vData, const char** ppszReadColumn, unsigned int iNumReadColumns,
                              unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);

    int ReadColumns(const char* pszCacheTableName, unsigned int iNumColumns, const char* const* ppszColumn,
                    unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);

    int ReadRow(const char* pszCacheTableName, Variant** ppvData);
    int ReadRow(const char* pszCacheTableName, unsigned int iKey, Variant** ppvData);

    int ReadData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, Variant* pvData);
    int ReadData(const char* pszCacheTableName, const char* pszColumn, Variant* pvData);

    int InsertRow(const char* pszCacheTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey);
    int InsertDuplicateRows(const char* pszCacheTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows);
    int DeleteRow(const char* pszCacheTableName, unsigned int iKey);
    int DeleteAllRows(const char* pszCacheTableName);

    int Increment(const char* pszCacheTableName, const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszCacheTableName, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);
    int Increment(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);

    int WriteData(const char* pszCacheTableName, const char* pszColumn, const Variant& vData);
    int WriteData(const char* pszCacheTableName, const char* pszColumn, int iData);
    int WriteData(const char* pszCacheTableName, const char* pszColumn, float fData);
    int WriteData(const char* pszCacheTableName, const char* pszColumn, int64 i64Data);
    int WriteData(const char* pszCacheTableName, const char* pszColumn, const char* pszData);

    int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vData);
    int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, int iData);
    int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, float fData);
    int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, int64 i64Data);
    int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const char* pszData);

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