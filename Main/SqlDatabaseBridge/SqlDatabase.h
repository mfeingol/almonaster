#pragma once

#include "Osal/IObject.h"
#include "Osal/Variant.h"
#include "Osal/FileHeap.h"

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
SQL_DATABASE_EXPORT extern const Uuid IID_IReadTable;
SQL_DATABASE_EXPORT extern const Uuid IID_IWriteTable;
SQL_DATABASE_EXPORT extern const Uuid IID_IDatabase;
SQL_DATABASE_EXPORT extern const Uuid IID_IDatabaseConnection;
SQL_DATABASE_EXPORT extern const Uuid IID_IDatabaseBackup;
SQL_DATABASE_EXPORT extern const Uuid IID_IDatabaseBackupEnumerator;
SQL_DATABASE_EXPORT extern const Uuid IID_ITableEnumerator;
SQL_DATABASE_EXPORT extern const Uuid IID_ITemplateEnumerator;
SQL_DATABASE_EXPORT extern const Uuid IID_IDatabaseBackupNotificationSink;
SQL_DATABASE_EXPORT extern const Uuid IID_ITemplate;

// Index flags
#define INDEX_CASE_SENSITIVE (0x00000001)
#define INDEX_UNIQUE_DATA    (0x00000002)

struct TemplateDescription {
    char* Name;
    unsigned int NumColumns;
    char** ColumnNames;
    VariantType* Type;
    unsigned int* Size;
    bool OneRow;
    unsigned int NumIndexes;
    char** IndexColumns;
    unsigned int NumRowsGuess;
    unsigned int* IndexFlags;
};

struct DatabaseStatistics {

    unsigned int iNumTables;
    unsigned int iNumTemplates;

    FileHeapStatistics fhsTableStats;
    FileHeapStatistics fhsMetaStats;
    FileHeapStatistics fhsVarlenStats;
    FileHeapStatistics fhsTemplateStats;
};

struct SearchColumn {

    const char* pszColumn;
    unsigned int iFlags;
    Variant vData;
    Variant vData2;
};

struct SearchDefinition {

    unsigned int iStartKey;
    unsigned int iSkipHits;
    unsigned int iMaxNumHits;
    unsigned int iNumColumns;
    SearchColumn* pscColumns;
};

//
// Search flags
//

// Strings
#define SEARCH_SUBSTRING       (0x00000001)
#define SEARCH_EXACT           (0x00000002)
#define SEARCH_BEGINS_WITH     (0x00000004)
#define SEARCH_ENDS_WITH       (0x00000008)

// Miscellaneous constants
const unsigned int VARIABLE_LENGTH_STRING = 0xffffffff;

#define MAX_NUM_INDEX_BUCKETS  (100000)

// Initialization options
#define DATABASE_CHECK         (0x00000001)
#define DATABASE_WRITETHROUGH  (0x00000002)

// Error codes
#define ERROR_TABLE_ALREADY_EXISTS (-1000002)
#define ERROR_UNKNOWN_TABLE_NAME (-1000003)
#define ERROR_DATA_NOT_FOUND (-1000004)
#define ERROR_UNKNOWN_ROW_KEY (-1000005)
#define ERROR_UNKNOWN_COLUMN_INDEX (-1000006)
#define ERROR_TYPE_MISMATCH (-1000007)
#define ERROR_TOO_MANY_ROWS (-1000008)
#define ERROR_REPEATED_INDEX_COLUMNS (-1000009)
#define ERROR_BLANK_TEMPLATE_NAME (-1000010)
#define ERROR_NO_COLUMNS (-1000011)
#define ERROR_ONE_ROW_CANNOT_BE_INDEXED (-100012)
#define ERROR_INVALID_TYPE (-1000013)
#define ERROR_TRANSACTION_IN_PROGRESS (-1000014)
#define ERROR_TOO_MANY_HITS (-1000015)
#define ERROR_INVALID_RANGE (-1000016)
#define ERROR_UNKNOWN_TEMPLATE_NAME (-1000017)
#define ERROR_COULD_NOT_CREATE_TEMPLATE (-1000018)
#define ERROR_COULD_NOT_CREATE_TABLE (-1000019)
#define ERROR_COULD_NOT_MAKE_BACKUP (-1000020)
#define ERROR_TEMPLATE_ALREADY_EXISTS (-1000021)
#define ERROR_COULD_NOT_CREATE_TEMPLATE_DIRECTORY (-1000022)
#define ERROR_COULD_NOT_CREATE_TABLE_DIRECTORY (-1000023)
#define ERROR_TABLE_HAS_MORE_THAN_ONE_ROW (-1000024)
#define ERROR_DATA_CORRUPTION (-1000025)
#define ERROR_STRING_IS_TOO_LONG (-1000026)
#define ERROR_COLUMN_NOT_INDEXED (-1000027)
#define ERROR_NOT_ENOUGH_ROWS (-1000028)
#define ERROR_UNSORTED_INDEX_COLUMNS (-1000029)
#define ERROR_DATABASE_IS_INCOMPLETE (-1000030)
#define ERROR_INDEX_COLUMN_TYPE_MISMATCH (-1000031)
#define ERROR_DATABASE_ALREADY_INITIALIZED (-1000032)
#define ERROR_DATABASE_HAS_NO_BACKUPS (-1000033)
#define ERROR_COULD_NOT_CREATE_TEMPLATE_TABLE_DIRECTORY (-1000034)
#define ERROR_DUPLICATE_DATA (-1000035)
#define ERROR_TABLE_COULD_NOT_BE_INITIALIZED (-1000036)
#define ERROR_TEMPLATE_COULD_NOT_BE_INITIALIZED (-1000037)


// No key
#define NO_KEY ((unsigned int) 0xffffffff)

//
// Interfaces
//

class IDatabaseBackup : virtual public IObject {
public:

    virtual int GetDate(int* piDay, int* piMonth, int* piYear, int* piVersion) = 0;
};

class IDatabaseBackupEnumerator : virtual public IObject {
public:

    virtual unsigned int GetNumBackups() = 0;
    virtual IDatabaseBackup** GetBackups() = 0;
};

class IDatabaseBackupNotificationSink : virtual public IObject {
public:

    virtual void BeginBackup(const char* pszBackupDirectory) = 0;

    virtual void BeginTemplateBackup(unsigned int iNumTemplates) = 0;
    virtual void EndTemplateBackup() = 0;

    virtual void BeginTableBackup(unsigned int iNumTables) = 0;
    virtual void EndTableBackup() = 0;

    virtual void BeginVariableLengthDataBackup() = 0;
    virtual void EndVariableLengthDataBackup() = 0;

    virtual void BeginMetaDataBackup() = 0;
    virtual void EndMetaDataBackup() = 0;

    virtual void EndBackup(IDatabaseBackup* pBackup) = 0;

    virtual void AbortBackup(int iErrCode) = 0;
};

class ITableEnumerator : virtual public IObject {
public:

    virtual unsigned int GetNumTables() = 0;
    virtual const char** GetTableNames() = 0;
};

class IReadTable : virtual public IObject {
public:

    virtual unsigned int GetNumRows(unsigned int* piNumRows) = 0;

    virtual int DoesRowExist(unsigned int iKey, bool* pbExists) = 0;

    virtual int GetFirstKey(const char* pszColumn, int iData, unsigned int* piKey) = 0;
    virtual int GetFirstKey(const char* pszColumn, float fData, unsigned int* piKey) = 0;
    virtual int GetFirstKey(const char* pszColumn, const char* pszData, unsigned int* piKey) = 0;
    virtual int GetFirstKey(const char* pszColumn, int64 i64Data, unsigned int* piKey) = 0;
    virtual int GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey) = 0;

    virtual int GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys) = 0;
    virtual int GetNextKey(unsigned int iKey, unsigned int* piNextKey) = 0;

    virtual int GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys) = 0;
    virtual int GetSearchKeys(const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey) = 0;

    virtual int ReadData(unsigned int iKey, const char* pszColumn, int* piData) = 0;
    virtual int ReadData(unsigned int iKey, const char* pszColumn, float* pfData) = 0;
    virtual int ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data) = 0;
    virtual int ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData) = 0;

    virtual int ReadData(const char* pszColumn, int* piData) = 0;
    virtual int ReadData(const char* pszColumn, float* pfData) = 0;
    virtual int ReadData(const char* pszColumn, int64* pi64Data) = 0;
    virtual int ReadData(const char* pszColumn, Variant* pvData) = 0;

    virtual int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, unsigned int** ppiKey, float** ppfData, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, unsigned int** ppiKey, char*** ppszData, unsigned int* piNumRows) = 0;

    virtual int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows) = 0;

    virtual int ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows) = 0;

    virtual int ReadColumn(const char* pszColumn, int** ppiData, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, float** ppfData, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, char*** ppszData, unsigned int* piNumRows) = 0;
    
    virtual int ReadColumn(const char* pszColumn, int64** ppi64Data, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, Variant** ppvData, unsigned int* piNumRows) = 0;

    virtual int ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, 
        Variant*** pppvData, unsigned int* piNumRows) = 0;

    virtual int ReadRow(unsigned int iKey, void*** ppData) = 0;
    virtual int ReadRow(unsigned int iKey, Variant** ppvData) = 0;

    virtual int ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                     unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys) = 0;
};

class IWriteTable : virtual public IReadTable {
public:

    virtual int WriteData(unsigned int iKey, const char* pszColumn, int iData) = 0;
    virtual int WriteData(unsigned int iKey, const char* pszColumn, float fData) = 0;
    virtual int WriteData(unsigned int iKey, const char* pszColumn, const char* pszData) = 0;

    virtual int WriteData(unsigned int iKey, const char* pszColumn, int64 i64Data) = 0;
    virtual int WriteData(unsigned int iKey, const char* pszColumn, const Variant& vData) = 0;

    virtual int WriteData(const char* pszColumn, int iData) = 0;
    virtual int WriteData(const char* pszColumn, float fData) = 0;
    virtual int WriteData(const char* pszColumn, const char* pszData) = 0;

    virtual int WriteData(const char* pszColumn, int64 i64Data) = 0;
    virtual int WriteData(const char* pszColumn, const Variant& vData) = 0;

    virtual int WriteAnd(unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteAnd(const char* pszColumn, unsigned int iBitField) = 0;

    virtual int WriteOr(unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteOr(const char* pszColumn, unsigned int iBitField) = 0;

    virtual int WriteXor(unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteXor(const char* pszColumn, unsigned int iBitField) = 0;

    virtual int WriteNot(unsigned int iKey, const char* pszColumn) = 0;
    virtual int WriteNot(const char* pszColumn) = 0;

    virtual int WriteColumn(const char* pszColumn, int iData) = 0;
    virtual int WriteColumn(const char* pszColumn, float fData) = 0;
    virtual int WriteColumn(const char* pszColumn, const char* pszData) = 0;
    
    virtual int WriteColumn(const char* pszColumn, int64 i64Data) = 0;
    virtual int WriteColumn(const char* pszColumn, const Variant& vData) = 0;

    virtual int InsertRow(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey) = 0;
    virtual int InsertRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows) = 0;
    virtual int InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows) = 0;

    virtual int Increment(const char* pszColumn, const Variant& vIncrement) = 0;
    virtual int Increment(const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue) = 0;

    virtual int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement) = 0;
    virtual int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue) = 0;

    virtual int DeleteRow(unsigned int iKey) = 0;
    virtual int DeleteAllRows() = 0;
};

class IDatabase;

class IDatabaseConnection : virtual public IObject {
public:

    virtual bool DoesTableExist(const char* pszTableName) = 0;

    virtual int CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate) = 0;
    virtual int DeleteTable(const char* pszTableName) = 0;

    virtual int ReadData(const char* pszTableName, unsigned int iKey, const char* pszColumn, Variant* pvData) = 0;  
    virtual int ReadData(const char* pszTableName, const char* pszColumn, Variant* pvData) = 0;
    
    virtual int WriteData(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vData) = 0;
    virtual int WriteData(const char* pszTableName, const char* pszColumn, const Variant& vData) = 0;
    
    virtual int Increment(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement) = 0;
    virtual int Increment(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue) = 0;
    
    virtual int Increment(const char* pszTableName, const char* pszColumn, const Variant& vIncrement) = 0;
    virtual int Increment(const char* pszTableName, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue) = 0;

    virtual int WriteAnd(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteAnd(const char* pszTableName, const char* pszColumn, unsigned int iBitField) = 0;

    virtual int WriteOr(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteOr(const char* pszTableName, const char* pszColumn, unsigned int iBitField) = 0;

    virtual int WriteXor(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField) = 0;
    virtual int WriteXor(const char* pszTableName, const char* pszColumn, unsigned int iBitField) = 0;

    virtual int WriteNot(const char* pszTableName, unsigned int iKey, const char* pszColumn) = 0;
    virtual int WriteNot(const char* pszTableName, const char* pszColumn) = 0;

    virtual int WriteColumn(const char* pszTableName, const char* pszColumn, const Variant& vData) = 0;

    virtual unsigned int GetNumRows(const char* pszTableName, unsigned int* piNumRows) = 0;
    virtual int DoesRowExist(const char* pszTableName, unsigned int iKey, bool* pbExists) = 0;

    virtual int InsertRow(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey) = 0;
    virtual int InsertRows(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows) = 0;
    virtual int InsertDuplicateRows(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows) = 0;

    virtual int DeleteRow(const char* pszTableName, unsigned int iKey) = 0;
    virtual int DeleteAllRows(const char* pszTableName) = 0;
    
    virtual int ReadRow(const char* pszTableName, unsigned int iKey, Variant** ppvData) = 0;
    virtual int ReadRow(const char* pszTableName, Variant** ppvData) = 0;

    virtual int ReadColumn(const char* pszTableName, const char* pszColumn, unsigned int** ppiKey, 
        Variant** ppvData, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszTableName, const char* pszColumn, Variant** ppvData, 
        unsigned int* piNumRows) = 0;

    virtual int ReadColumns(const char* pszTableName, unsigned int iNumColumns, const char* const* ppszColumn, 
        unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows) = 0;
    virtual int ReadColumns(const char* pszTableName, unsigned int iNumColumns, const char* const* ppszColumn, 
        Variant*** pppvData, unsigned int* piNumRows) = 0;

    virtual int ReadColumnWhereEqual(const char* pszTableName, const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                                     unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys) = 0;

    virtual int GetAllKeys(const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys) = 0;
    virtual int GetNextKey(const char* pszTableName, unsigned int iKey, unsigned int* piNextKey) = 0;

    virtual int GetFirstKey(const char* pszTableName, const char* pszColumn, const Variant& vData, unsigned int* piKey) = 0;
    virtual int GetEqualKeys(const char* pszTableName, const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys) = 0;
    virtual int GetSearchKeys(const char* pszTableName, const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey) = 0;

    virtual int GetTableForReading(const char* pszTableName, IReadTable** ppTable) = 0;
    virtual int GetTableForWriting(const char* pszTableName, IWriteTable** ppTable) = 0;

    virtual void FreeData(void** ppData) = 0;

    virtual void FreeData(Variant* pvData) = 0;
    virtual void FreeData(Variant** ppvData) = 0;

    virtual void FreeData(int* piData) = 0;
    virtual void FreeData(unsigned int* puiData) = 0;
    virtual void FreeData(float* ppfData) = 0;
    virtual void FreeData(char** ppszData) = 0;
    virtual void FreeData(int64* pi64Data) = 0;

    virtual void FreeKeys(unsigned int* piKeys) = 0;
};

class IDatabase : virtual public IObject {
public:

    	// Return OK if database was reloaded, WARNING if new database was created, something else if an error occurred
    virtual int Initialize(const char* pszConnectionString, unsigned int iOptions) = 0;

    virtual const char* GetConnectionString() = 0;
    virtual unsigned int GetOptions() = 0;

    virtual IDatabaseConnection* CreateConnection() = 0;

    virtual int Backup(IDatabaseBackupNotificationSink* pSink, bool bCheckFirst) = 0;
    virtual unsigned int DeleteOldBackups(Seconds iNumSecondsOld) = 0;

    virtual IDatabaseBackupEnumerator* GetBackupEnumerator() = 0;

    virtual int RestoreBackup(IDatabaseBackup* pBackup) = 0;
    virtual int DeleteBackup(IDatabaseBackup* pBackup) = 0;

    virtual int GetStatistics(DatabaseStatistics* pdsStats) = 0;

    virtual unsigned int GetNumTables() = 0;
    virtual unsigned int GetNumTemplates() = 0;

    virtual ITableEnumerator* GetTableEnumerator() = 0;

    virtual int Flush() = 0;
    virtual int Check() = 0;
};
