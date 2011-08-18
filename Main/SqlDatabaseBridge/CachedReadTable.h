#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

class CachedReadTable : public ICachedReadTable
{
private:
    gcroot<BulkTableReadResult^> m_result;

public:
    // ICachedReadTable
    IMPLEMENT_INTERFACE(ICachedReadTable);

    CachedReadTable(BulkTableReadResult^ result);

    int GetNumCachedRows(unsigned int* piNumRows);

    int GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys);
    //int GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys);

    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);
    
    int ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                             unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadRow(unsigned int iKey, Variant** ppvData);

    int ReadData(const char* pszColumn, Variant* pvData);

    int ReadData(unsigned int iKey, const char* pszColumn, int* piData);
    int ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data);
    int ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData);
};