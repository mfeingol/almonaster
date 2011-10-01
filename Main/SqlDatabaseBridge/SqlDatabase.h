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

#include "Osal/IObject.h"
#include "Osal/Variant.h"
#include "Osal/FileHeap.h"
#include "Osal/TraceLog.h"

#ifndef SQL_DATABASE_EXPORT
#ifdef SQL_DATABASE_BUILD
#define SQL_DATABASE_EXPORT EXPORT
#else
#define SQL_DATABASE_EXPORT IMPORT
#endif
#endif

////////////
// Export //
////////////

SQL_DATABASE_EXPORT extern const Uuid CLSID_SqlDatabase;
SQL_DATABASE_EXPORT extern const Uuid IID_ICachedTableCollection;
SQL_DATABASE_EXPORT extern const Uuid IID_ICachedTable;
SQL_DATABASE_EXPORT extern const Uuid IID_IDatabase;
SQL_DATABASE_EXPORT extern const Uuid IID_IDatabaseConnection;

// Index flags
#define INDEX_UNIQUE_DATA    (0x00000001)

struct TemplateDescription
{
    char* Name;
    unsigned int NumColumns;
    char** ColumnNames;
    VariantType* Type;
    unsigned int* Size;
    unsigned int NumIndexes;
    char** IndexColumns;
    unsigned int* IndexFlags;
};

struct RangeSearchColumnDefinition
{
    const char* pszColumn;
    unsigned int iFlags;
    Variant vData;
    Variant vData2;
};

struct RangeSearchDefinition
{
    unsigned int iStartKey;
    unsigned int iSkipHits;
    unsigned int iMaxNumHits;
    unsigned int iNumColumns;
    RangeSearchColumnDefinition* pscColumns;
};

struct OrderByColumnDefinition
{
    const char* pszColumn;
    bool bAscending;
};

struct OrderByDefinition
{
    unsigned int iNumColumns;
    const OrderByColumnDefinition* pscColumns;
};

struct ColumnEntry
{
    const char* Name;
    Variant Data;
};

struct TableEntry
{
    const char* Name;
    unsigned int Key;
    unsigned int NumColumns;
    const ColumnEntry* Columns;
};

struct CrossJoinEntry
{
    TableEntry Table;
    const char* LeftColumnName;
    const char* RightColumnName;
};

struct TableCacheEntry
{
    TableEntry Table;
    void* Reserved;
    const char* PartitionColumn;
    CrossJoinEntry* CrossJoin;
};

#define ID_COLUMN_NAME "Id"

//
// Search flags
//

// Strings
#define SEARCH_SUBSTRING       (0x00000001)
#define SEARCH_EXACT           (0x00000002)
#define SEARCH_BEGINS_WITH     (0x00000004)
#define SEARCH_ENDS_WITH       (0x00000008)

// Size
const unsigned int VARIABLE_LENGTH_STRING = 0xffffffff;

// Error codes
#define ERROR_TABLE_ALREADY_EXISTS (-1000002)
#define ERROR_UNKNOWN_TABLE_NAME (-1000003)
#define ERROR_DATA_NOT_FOUND (-1000004)
#define ERROR_UNKNOWN_ROW_KEY (-1000005)
//#define ERROR_UNKNOWN_COLUMN_INDEX (-1000006)
//#define ERROR_TYPE_MISMATCH (-1000007)
//#define ERROR_TOO_MANY_ROWS (-1000008)
//#define ERROR_REPEATED_INDEX_COLUMNS (-1000009)
//#define ERROR_BLANK_TEMPLATE_NAME (-1000010)
//#define ERROR_NO_COLUMNS (-1000011)
//#define ERROR_ONE_ROW_CANNOT_BE_INDEXED (-100012)
//#define ERROR_INVALID_TYPE (-1000013)
//#define ERROR_TRANSACTION_IN_PROGRESS (-1000014)
#define ERROR_TOO_MANY_HITS (-1000015)
//#define ERROR_INVALID_RANGE (-1000016)
//#define ERROR_UNKNOWN_TEMPLATE_NAME (-1000017)
//#define ERROR_COULD_NOT_CREATE_TEMPLATE (-1000018)
//#define ERROR_COULD_NOT_CREATE_TABLE (-1000019)
//#define ERROR_COULD_NOT_MAKE_BACKUP (-1000020)
//#define ERROR_TEMPLATE_ALREADY_EXISTS (-1000021)
//#define ERROR_COULD_NOT_CREATE_TEMPLATE_DIRECTORY (-1000022)
//#define ERROR_COULD_NOT_CREATE_TABLE_DIRECTORY (-1000023)
#define ERROR_TABLE_HAS_MORE_THAN_ONE_ROW (-1000024)
//#define ERROR_DATA_CORRUPTION (-1000025)
//#define ERROR_COLUMN_NOT_INDEXED (-1000027)
//#define ERROR_NOT_ENOUGH_ROWS (-1000028)
//#define ERROR_UNSORTED_INDEX_COLUMNS (-1000029)
//#define ERROR_DATABASE_IS_INCOMPLETE (-1000030)
//#define ERROR_INDEX_COLUMN_TYPE_MISMATCH (-1000031)
//#define ERROR_DATABASE_ALREADY_INITIALIZED (-1000032)
//#define ERROR_DATABASE_HAS_NO_BACKUPS (-1000033)
//#define ERROR_COULD_NOT_CREATE_TEMPLATE_TABLE_DIRECTORY (-1000034)
//#define ERROR_DUPLICATE_DATA (-1000035)
//#define ERROR_TABLE_COULD_NOT_BE_INITIALIZED (-1000036)
//#define ERROR_TEMPLATE_COULD_NOT_BE_INITIALIZED (-1000037)
#define ERROR_DATABASE_EXCEPTION (-1000038)


// No key
#define NO_KEY ((unsigned int) 0xffffffff)

//
// Interfaces
//

class ICachedTable : public IObject
{
public:
    virtual int GetNumCachedRows(unsigned int* piNumRows) = 0;

    virtual int GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey) = 0;
    virtual int GetNextKey(unsigned int iKey, unsigned int* piNextKey) = 0;
    virtual int GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys) = 0;
    virtual int GetEqualKeys(const char** ppszColumn, const Variant* pvData, unsigned int iNumColumns, unsigned int** ppiKey, unsigned int* piNumKeys) = 0;
    virtual int GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys) = 0;

    virtual int ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows) = 0;
    virtual int ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows) = 0;
    
    virtual int ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                     unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows) = 0;

    virtual int ReadRow(Variant** ppvData) = 0;
    virtual int ReadRow(unsigned int iKey, Variant** ppvData) = 0;

    virtual int ReadData(const char* pszColumn, int* piData) = 0;
    virtual int ReadData(const char* pszColumn, float* pfData) = 0;
    virtual int ReadData(const char* pszColumn, int64* pi64Data) = 0;
    virtual int ReadData(const char* pszColumn, Variant* pvData) = 0;

    virtual int ReadData(unsigned int iKey, const char* pszColumn, int* piData) = 0;
    virtual int ReadData(unsigned int iKey, const char* pszColumn, float* pfData) = 0;
    virtual int ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data) = 0;
    virtual int ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData) = 0;
    
    virtual int InsertRow(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey) = 0;
    virtual int InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows) = 0;
    virtual int DeleteRow(unsigned int iKey) = 0;
    virtual int DeleteAllRows() = 0;

    virtual int Increment(const char* pszColumn, const Variant& vIncrement) = 0;
    virtual int Increment(const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue) = 0;
    virtual int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement) = 0;
    virtual int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue) = 0;

    virtual int WriteData(const char* pszColumn, const Variant& vData) = 0;
    virtual int WriteData(const char* pszColumn, int iData) = 0;
    virtual int WriteData(const char* pszColumn, float fData) = 0;
    virtual int WriteData(const char* pszColumn, int64 i64Data) = 0;
    virtual int WriteData(const char* pszColumn, const char* pszData) = 0;

    virtual int WriteData(unsigned int iKey, const char* pszColumn, const Variant& vData) = 0;
    virtual int WriteData(unsigned int iKey, const char* pszColumn, int iData) = 0;
    virtual int WriteData(unsigned int iKey, const char* pszColumn, float fData) = 0;
    virtual int WriteData(unsigned int iKey, const char* pszColumn, int64 i64Data) = 0;
    virtual int WriteData(unsigned int iKey, const char* pszColumn, const char* pszData) = 0;

    virtual int WriteAnd(const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteAnd(unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteOr(const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteOr(unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
};

class ICachedTableCollection : virtual public IObject
{
public:

    virtual int CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate) = 0;
    virtual int CreateEmpty(const char* pszTableName, const char* pszCachedTableName) = 0;

    virtual int Cache(const TableCacheEntry* pcCacheEntry, unsigned int iNumEntries) = 0;
    virtual int Cache(const TableCacheEntry& cCacheEntry, ICachedTable** ppTable) = 0;

    virtual bool IsCached(const char* pszCacheTableName) = 0;
    virtual int GetTable(const char* pszCacheTableName, ICachedTable** ppTable) = 0;

    virtual int GetNumCachedRows(const char* pszCacheTableName, unsigned int* piNumRows) = 0;

    virtual int GetFirstKey(const char* pszCacheTableName, const char* pszColumn, const Variant& vData, unsigned int* piKey) = 0;
    virtual int GetNextKey(const char* pszCacheTableName, unsigned int iKey, unsigned int* piNextKey) = 0;
    virtual int GetEqualKeys(const char* pszCacheTableName, const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys) = 0;
    virtual int GetEqualKeys(const char* pszCacheTableName, const char** ppszColumn, const Variant* pvData, unsigned int iNumColumns, unsigned int** ppiKey, unsigned int* piNumKeys) = 0;
    virtual int GetAllKeys(const char* pszCacheTableName, unsigned int** ppiKey, unsigned int* piNumRows) = 0;

    virtual int ReadColumn(const char* pszCacheTableName, const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows) = 0;

    virtual int ReadColumns(const char* pszCacheTableName, unsigned int iNumColumns, const char* const* ppszColumn,
                            unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows) = 0;
    
    virtual int ReadColumnWhereEqual(const char* pszCacheTableName, const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                     unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows) = 0;

    virtual int ReadRow(const char* pszCacheTableName, unsigned int iKey, Variant** ppvData) = 0;

    virtual int ReadData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, Variant* pvData) = 0;  
    virtual int ReadData(const char* pszCacheTableName, const char* pszColumn, Variant* pvData) = 0;

    virtual int InsertRow(const char* pszCacheTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey) = 0;
    virtual int InsertDuplicateRows(const char* pszCacheTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows) = 0;
    virtual int DeleteRow(const char* pszCacheTableName, unsigned int iKey) = 0;
    virtual int DeleteAllRows(const char* pszCacheTableName) = 0;

    virtual int Increment(const char* pszCacheTableName, const char* pszColumn, const Variant& vIncrement) = 0;
    virtual int Increment(const char* pszCacheTableName, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue) = 0;
    virtual int Increment(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement) = 0;
    virtual int Increment(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue) = 0;

    virtual int WriteData(const char* pszCacheTableName, const char* pszColumn, const Variant& vData) = 0;
    virtual int WriteData(const char* pszCacheTableName, const char* pszColumn, int iData) = 0;
    virtual int WriteData(const char* pszCacheTableName, const char* pszColumn, float fData) = 0;
    virtual int WriteData(const char* pszCacheTableName, const char* pszColumn, int64 i64Data) = 0;
    virtual int WriteData(const char* pszCacheTableName, const char* pszColumn, const char* pszData) = 0;

    virtual int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const Variant& vData) = 0;
    virtual int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, int iData) = 0;
    virtual int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, float fData) = 0;
    virtual int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, int64 i64Data) = 0;
    virtual int WriteData(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, const char* pszData) = 0;

    virtual int WriteAnd(const char* pszCacheTableName, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteAnd(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteOr(const char* pszCacheTableName, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteOr(const char* pszCacheTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;

    virtual void FreeData(Variant* pvData) = 0;
    virtual void FreeData(Variant** ppvData) = 0;

    virtual void FreeData(int* piData) = 0;
    virtual void FreeData(float* ppfData) = 0;
    virtual void FreeData(int64* pi64Data) = 0;

    virtual void FreeKeys(unsigned int* piKeys) = 0;

};

class IDatabaseConnection : virtual public IObject
{
public:

    virtual int Commit() = 0;
    virtual ICachedTableCollection* GetCache() = 0;

    virtual int DoesTableExist(const char* pszTableName, bool* pbExist) = 0;
    virtual int CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate) = 0;
    virtual int DeleteTable(const char* pszTableName) = 0;

    virtual int GetNumPhysicalRows(const char* pszTableName, unsigned int* piNumRows) = 0;

    virtual int GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey) = 0;
    virtual int GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, const OrderByDefinition& sdOrderBy, unsigned int** ppiKey, unsigned int* piNumHits) = 0;
};

enum TransactionIsolationLevel
{
    UNSPECIFIED,
    CHAOS,
    READ_UNCOMMITTED,
    READ_COMMITTED,
    REPEATABLE_READ,
    SERIALIZABLE,
    SNAPSHOT
};

class IDatabase : virtual public IObject
{
public:
    	// Return OK if database was reloaded, WARNING if new database was created, something else if an error occurred
    virtual int Initialize(const char* pszConnectionString, ITraceLog* pTrace) = 0;
    virtual IDatabaseConnection* CreateConnection(TransactionIsolationLevel isoLevel) = 0;
};


