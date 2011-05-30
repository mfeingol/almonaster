// ReadOnlyTable.h: interface for the ReadOnlyTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_READONLYTABLE_H__3B5FD4A1_370A_11D3_A10B_0050047FE2E2__INCLUDED_)
#define AFX_READONLYTABLE_H__3B5FD4A1_370A_11D3_A10B_0050047FE2E2__INCLUDED_

#include "Table.h"

class ReadTable : public IReadTable {

protected:

    ReadTable (Table* pTable);
    virtual ~ReadTable();

    bool m_bWriteTable;
    Table* m_pTable;
    unsigned int m_iRefCount;

    int CheckLoad();

public:

    static ReadTable* CreateInstance (Table* pTable);

    // IReadTable
    DECLARE_IOBJECT;

    int GetNumRows (unsigned int* piNumRows);

    int DoesRowExist (unsigned int iKey, bool* pbExists);

    int GetFirstKey (unsigned int iColumn, int iData, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, float fData, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, const char* pszData, bool bCaseInsensitive, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, const UTCTime& tData, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, int64 i64Data, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey);

    int GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey (unsigned int iKey, unsigned int* piNextKey);

    int GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int** ppiKey, 
        unsigned int* piNumKeys);

    int GetSearchKeys (unsigned int iNumColumns, const unsigned int* piColumn, const Variant* pvData, 
        const Variant* pvData2, unsigned int iStartKey, unsigned int iSkipHits, unsigned int iMaxNumHits, 
        unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey) { 
        return ERROR_NOT_IMPLEMENTED;
    }

    int ReadData (unsigned int iKey, unsigned int iColumn, int* piData);
    int ReadData (unsigned int iKey, unsigned int iColumn, float* pfData);
    int ReadData (unsigned int iKey, unsigned int iColumn, const char** ppszData);
    int ReadData (unsigned int iKey, unsigned int iColumn, UTCTime* ptData);
    int ReadData (unsigned int iKey, unsigned int iColumn, int64* pi64Data);
    int ReadData (unsigned int iKey, unsigned int iColumn, Variant* pvData);

    int ReadData (unsigned int iColumn, int* piData);
    int ReadData (unsigned int iColumn, float* pfData);
    int ReadData (unsigned int iColumn, const char** ppszData);
    int ReadData (unsigned int iColumn, UTCTime* ptData);
    int ReadData (unsigned int iColumn, int64* pi64Data);
    int ReadData (unsigned int iColumn, Variant* pvData);

    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, float** ppfData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, char*** ppszData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, UTCTime** pptData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumn (unsigned int iColumn, int** ppiData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, float** ppfData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, char*** ppszData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, UTCTime** pptData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, int64** ppi64Data, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns (unsigned int iNumColumns, const unsigned int* piColumn, unsigned int** ppiKey, 
        Variant*** pppvData, unsigned int* piNumRows) { return ERROR_NOT_IMPLEMENTED; }

    int ReadRow (unsigned int iKey, void*** ppData);
    int ReadRow (unsigned int iKey, Variant** ppvData);
};

#endif // !defined(AFX_READONLYTABLE_H__3B5FD4A1_370A_11D3_A10B_0050047FE2E2__INCLUDED_)