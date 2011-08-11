#pragma once
#pragma warning(disable:4511)

#include <vcclr.h>
#include "SqlDatabase.h"
#include "SqlDatabaseReadTable.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

class SqlDatabaseWriteTable : public IWriteTable
{
private:
    gcroot<SqlCommandManager^> m_cmd;
    gcroot<System::String^> m_tableName;
    SqlDatabaseReadTable m_readTable;

public:

    SqlDatabaseWriteTable(SqlCommandManager^ cmd, System::String^ tableName);
    ~SqlDatabaseWriteTable();

    IMPLEMENT_INTERFACE(IWriteTable);

    unsigned int GetNumRows(unsigned int* piNumRows);

    int DoesRowExist(unsigned int iKey, bool* pbExists);

    int GetFirstKey(const char* pszColumn, int iData, unsigned int* piKey);
    int GetFirstKey(const char* pszColumn, float fData, unsigned int* piKey);
    int GetFirstKey(const char* pszColumn, const char* pszData, unsigned int* piKey);
    int GetFirstKey(const char* pszColumn, int64 i64Data, unsigned int* piKey);
    int GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey);

    int GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey(unsigned int iKey, unsigned int* piNextKey);

    int GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetSearchKeys(const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);

    int ReadData(unsigned int iKey, const char* pszColumn, int* piData);
    int ReadData(unsigned int iKey, const char* pszColumn, float* pfData);
    int ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data);
    int ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData);

    int ReadData(const char* pszColumn, int* piData);
    int ReadData(const char* pszColumn, float* pfData);
    int ReadData(const char* pszColumn, int64* pi64Data);
    int ReadData(const char* pszColumn, Variant* pvData);

    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, float** ppfData, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumn(const char* pszColumn, int** ppiData, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, float** ppfData, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, int64** ppi64Data, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);

    int ReadRow(unsigned int iKey, void*** ppData);
    int ReadRow(unsigned int iKey, Variant** ppvData);

    int ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn,
                             unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys);

    // IWriteTable
    int WriteData(unsigned int iKey, const char* pszColumn, int iData);
    int WriteData(unsigned int iKey, const char* pszColumn, float fData);
    int WriteData(unsigned int iKey, const char* pszColumn, const char* pszData);

    int WriteData(unsigned int iKey, const char* pszColumn, int64 i64Data);
    int WriteData(unsigned int iKey, const char* pszColumn, const Variant& vData);

    int WriteData(const char* pszColumn, int iData);
    int WriteData(const char* pszColumn, float fData);
    int WriteData(const char* pszColumn, const char* pszData);

    int WriteData(const char* pszColumn, int64 i64Data);
    int WriteData(const char* pszColumn, const Variant& vData);

    int WriteAnd(unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteAnd(const char* pszColumn, unsigned int iBitField);

    int WriteOr(unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteOr(const char* pszColumn, unsigned int iBitField);

    int WriteXor(unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteXor(const char* pszColumn, unsigned int iBitField);

    int WriteNot(unsigned int iKey, const char* pszColumn);
    int WriteNot(const char* pszColumn);

    int WriteColumn(const char* pszColumn, int iData);
    int WriteColumn(const char* pszColumn, float fData);
    int WriteColumn(const char* pszColumn, const char* pszData);
    
    int WriteColumn(const char* pszColumn, int64 i64Data);
    int WriteColumn(const char* pszColumn, const Variant& vData);

    int InsertRow(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey);
    int InsertRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows);
    int InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows);

    int Increment(const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);

    int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement);
    int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);

    int DeleteRow(unsigned int iKey);
    int DeleteAllRows();
};