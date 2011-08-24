#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"

using namespace System::Collections::Generic;

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

class CachedTable : public ICachedTable
{
private:
    gcroot<BulkTableReadResult^> m_result;
    gcroot<SqlCommandManager^> m_cmd;

    gcroot<List<IDictionary<System::String^, System::Object^>^>^> m_insertedRows;
    gcroot<SortedDictionary<int64, IDictionary<System::String^, System::Object^>^>^> m_keyToRows;

    int InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows, unsigned int* piKey);

public:
    CachedTable(SqlCommandManager^ cmd, BulkTableReadResult^ result);

    // ICachedTable
    IMPLEMENT_INTERFACE(ICachedTable);

    int GetNumCachedRows(unsigned int* piNumRows);

    int GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey);
    int GetNextKey(unsigned int iKey, unsigned int* piNextKey);
    int GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys);
    //int GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys);

    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);
    
    int ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                             unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadRow(Variant** ppvData);
    int ReadRow(unsigned int iKey, Variant** ppvData);

    int ReadData(const char* pszColumn, int* piData);
    int ReadData(const char* pszColumn, float* pfData);
    int ReadData(const char* pszColumn, int64* pi64Data);
    int ReadData(const char* pszColumn, Variant* pvData);
    int ReadData(unsigned int iKey, const char* pszColumn, int* piData);
    int ReadData(unsigned int iKey, const char* pszColumn, float* pfData);
    int ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data);
    int ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData);

    int InsertRow(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey);
    int InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows);

    int Increment(const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);
    int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement);
    int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);

    int WriteData(const char* pszColumn, const char* pszData);
    int WriteData(const char* pszColumn, const Variant& vData);
    int WriteData(unsigned int iKey, const char* pszColumn, const char* pszData);
    int WriteData(unsigned int iKey, const char* pszColumn, const Variant& vData);

    int WriteAnd(const char* pszColumn, unsigned int iBitField);
    int WriteAnd(unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteOr(const char* pszColumn, unsigned int iBitField);
    int WriteOr(unsigned int iKey, const char* pszColumn, unsigned int iBitField);
};