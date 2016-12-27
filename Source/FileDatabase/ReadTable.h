// ReadOnlyTable.h: interface for the ReadOnlyTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_READONLYTABLE_H__3B5FD4A1_370A_11D3_A10B_0050047FE2E2__INCLUDED_)
#define AFX_READONLYTABLE_H__3B5FD4A1_370A_11D3_A10B_0050047FE2E2__INCLUDED_

#include "FileDatabase.h"
#include "TableContext.h"

class Table;

class ReadTable : public IReadTable {

protected:

    unsigned int m_iNumRefs;
    TableContext m_tcContext;

    Table* m_pTable;

    inline void Lock() { m_tcContext.Lock(); }
    inline void Unlock() { m_tcContext.Unlock(); }

public:

    ReadTable();
    ~ReadTable();

    inline unsigned int AddRefInternal() {
        return Algorithm::AtomicIncrement (&m_iNumRefs);
    }

    inline unsigned int ReleaseInternal() {
        return Algorithm::AtomicDecrement (&m_iNumRefs);
    }

    inline void SetTable (Table* pTable) {
        m_pTable = pTable;
    }
    
    inline TableContext* GetContext() {
        return &m_tcContext;
    }

    inline bool IsValidKey (unsigned int iKey) {
        return m_tcContext.IsValidRow (iKey);
    }

    inline bool IsValidColumn (unsigned int iColumn) {
        return iColumn < m_tcContext.GetNumColumns();
    }

    // IReadTable
    DECLARE_IOBJECT;

    unsigned int GetNumRows (unsigned int* piNumRows);

    int DoesRowExist (unsigned int iKey, bool* pbExists);

    int GetFirstKey (unsigned int iColumn, int iData, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, float fData, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, const char* pszData, bool bCaseInsensitive, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, int64 i64Data, unsigned int* piKey);
    int GetFirstKey (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey);

    int GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey (unsigned int iKey, unsigned int* piNextKey);

    int GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int** ppiKey, 
        unsigned int* piNumKeys);

    int GetSearchKeys (const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, 
        unsigned int* piStopKey);

    int ReadData (unsigned int iKey, unsigned int iColumn, int* piData);
    int ReadData (unsigned int iKey, unsigned int iColumn, float* pfData);
    int ReadData (unsigned int iKey, unsigned int iColumn, const char** ppszData);
    int ReadData (unsigned int iKey, unsigned int iColumn, int64* pi64Data);
    int ReadData (unsigned int iKey, unsigned int iColumn, Variant* pvData);

    int ReadData (unsigned int iColumn, int* piData);
    int ReadData (unsigned int iColumn, float* pfData);
    int ReadData (unsigned int iColumn, const char** ppszData);
    int ReadData (unsigned int iColumn, int64* pi64Data);
    int ReadData (unsigned int iColumn, Variant* pvData);

    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, float** ppfData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, char*** ppszData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumn (unsigned int iColumn, int** ppiData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, float** ppfData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, char*** ppszData, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, int64** ppi64Data, unsigned int* piNumRows);
    int ReadColumn (unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns (unsigned int iNumColumns, const unsigned int* piColumn, unsigned int** ppiKey, 
        Variant*** pppvData, unsigned int* piNumRows);

    int ReadRow (unsigned int iKey, void*** ppData);
    int ReadRow (unsigned int iKey, Variant** ppvData);

    int ReadColumnWhereEqual (unsigned int iEqualColumn, const Variant& vData, bool bCaseInsensitive, 
        unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys);
};

#endif // !defined(AFX_READONLYTABLE_H__3B5FD4A1_370A_11D3_A10B_0050047FE2E2__INCLUDED_)