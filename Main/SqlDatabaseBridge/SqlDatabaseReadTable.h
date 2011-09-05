#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"
#include "Utils.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

class SqlDatabaseReadTable : public IReadTable
{
private:
    gcroot<SqlCommandManager^> m_cmd;
    gcroot<System::String^> m_tableName;

public:

    SqlDatabaseReadTable(SqlCommandManager^ cmd, System::String^ tableName);
    ~SqlDatabaseReadTable();

    // IReadTable
    IMPLEMENT_INTERFACE(IReadTable);

    int GetNumRows(unsigned int* piNumRows);

    int DoesRowExist(unsigned int iKey, bool* pbExists);

    int GetFirstKey(const char* pszColumn, int iData, unsigned int* piKey);
    int GetFirstKey(const char* pszColumn, float fData, unsigned int* piKey);
    int GetFirstKey(const char* pszColumn, const char* pszData, unsigned int* piKey);
    int GetFirstKey(const char* pszColumn, int64 i64Data, unsigned int* piKey);
    int GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey);

    int GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey(unsigned int iKey, unsigned int* piNextKey);

    int GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetSearchKeys(const RangeSearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);
    int GetSearchKeys(const RangeSearchDefinition& sdSearch, const OrderByDefinition& sdOrderBy, unsigned int** ppiKey, unsigned int* piNumHits);

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

    int ReadRow(unsigned int iKey, void*** pppData);
    int ReadRow(unsigned int iKey, Variant** ppvData);

    int ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                             unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys);
};