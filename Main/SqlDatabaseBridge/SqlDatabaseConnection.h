#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

#include "TableCacheCollection.h"
#include "Utils.h"

const gcroot<System::String^> TemplateTableName = gcnew System::String("_Templates");
const gcroot<System::String^> TemplateColumnsTableName = gcnew System::String("_TemplateColumns");
const gcroot<System::String^> TemplateIndexesTableName = gcnew System::String("_TemplateIndexes");

class SqlDatabaseConnection : public IDatabaseConnection
{
private:
    gcroot<SqlDatabase^> m_sqlDatabase;
    gcroot<SqlCommandManager^> m_cmd;

    TableCacheCollection m_viewCollection;

public:
    SqlDatabaseConnection(SqlDatabase^ sqlDatabase, TransactionIsolationLevel isoLevel);
    ~SqlDatabaseConnection();

    // IDatabaseConnection
    IMPLEMENT_INTERFACE(IDatabaseConnection);
    
    int Commit();
    ICachedTableCollection* GetCache();

    // Table operations
    bool DoesTableExist(const char* pszTableName);
    int CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate);
    int DeleteTable(const char* pszTableName);

    // Standard operations
    int ReadData(const char* pszTableName, unsigned int iKey, const char* pszColumn, Variant* pvData);
    int ReadData(const char* pszTableName, const char* pszColumn, Variant* pvData);
    
    int WriteData(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vData);
    int WriteData(const char* pszTableName, const char* pszColumn, const Variant& vData);
    
    int Increment(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszTableName, unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);
    int Increment(const char* pszTableName, const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszTableName, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);

    int WriteAnd(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteAnd(const char* pszTableName, const char* pszColumn, unsigned int iBitField);

    int WriteOr(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteOr(const char* pszTableName, const char* pszColumn, unsigned int iBitField);

    int WriteXor(const char* pszTableName, unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteXor(const char* pszTableName, const char* pszColumn, unsigned int iBitField);

    int WriteNot(const char* pszTableName, unsigned int iKey, const char* pszColumn);
    int WriteNot(const char* pszTableName, const char* pszColumn);

    int WriteColumn(const char* pszTableName, const char* pszColumn, const Variant& vData);

    // Row operations
    int GetNumPhysicalRows(const char* pszTableName, unsigned int* piNumRows);
    int DoesPhysicalRowExist(const char* pszTableName, unsigned int iKey, bool* pbExists);

    int InsertRow(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey);
    int InsertRows(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows);
    int InsertDuplicateRows(const char* pszTableName, const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows);
    
    int DeleteRow(const char* pszTableName, unsigned int iKey);
    int DeleteAllRows(const char* pszTableName);
    
    int ReadRow(const char* pszTableName, unsigned int iKey, Variant** ppvData);

    // Column operations
    int ReadColumn(const char* pszTableName, const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);
    int ReadColumn(const char* pszTableName, const char* pszColumn, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns(const char* pszTableName, unsigned int iNumColumns, const char* const* ppszColumn, 
                    unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);
    int ReadColumns(const char* pszTableName, unsigned int iNumColumns, const char* const* ppszColumn, 
                    Variant*** pppvData, unsigned int* piNumRows);

    // Searches
    int GetAllKeys(const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey(const char* pszTableName, unsigned int iKey, unsigned int* piNextKey);

    int GetFirstKey(const char* pszTableName, const char* pszColumn, const Variant& vData, unsigned int* piKey);
    int GetEqualKeys(const char* pszTableName, const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);
    int GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, const OrderByDefinition& sdOrderBy, unsigned int** ppiKey, unsigned int* piNumHits);

    int ReadColumnWhereEqual(const char* pszTableName, const char* pszEqualColumn, const Variant& vData, 
                             const char* pszReadColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys);

    // Direct API
    int GetTableForReading(const char* pszTableName, IReadTable** ppTable);
    int GetTableForWriting(const char* pszTableName, IWriteTable** ppTable);

    // Memory
    void FreeData(void** ppData);
    
    void FreeData(Variant* pvData);
    void FreeData(Variant** ppvData);

    void FreeData(int* piData);
    void FreeData(unsigned int* puiData);
    void FreeData(float* ppfData);
    void FreeData(char** ppszData);
    void FreeData(int64* pi64Data);

    void FreeKeys(unsigned int* piKeys);
};

