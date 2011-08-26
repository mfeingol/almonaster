#pragma once

#include "Osal/Variant.h"
#include "SqlDatabase.h"

using namespace System::Data;

static const char* IdColumnName = "Id";

System::Object^ Convert(const Variant& v);
void Convert(System::Object^ object, Variant* pv);
SqlDbType Convert(VariantType type);
IsolationLevel Convert(TransactionIsolationLevel);

System::Object^ Increment(System::Object^ original, const Variant& inc);

unsigned int* ConvertIdsToKeys(System::Collections::Generic::IEnumerable<int64>^ ids, unsigned int* piCount);

void Trace(System::String^ fmt, ... array<System::Object^>^ params);


//
// TODOTODO - Delete all these
//

#include "Osal/IObject.h"
#include "SqlDatabase.h"

class IReadTable : virtual public IObject {
public:

    virtual int GetNumRows(unsigned int* piNumRows) = 0;

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
    virtual int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows) = 0;

    virtual int ReadColumn(const char* pszColumn, int** ppiData, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, float** ppfData, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, int64** ppi64Data, unsigned int* piNumRows) = 0;
    virtual int ReadColumn(const char* pszColumn, Variant** ppvData, unsigned int* piNumRows) = 0;

    virtual int ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows) = 0;

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