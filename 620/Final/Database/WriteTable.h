// WriteTable.h: interface for the WriteTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WRITETABLE_H__3B5FD4A4_370A_11D3_A10B_0050047FE2E2__INCLUDED_)
#define AFX_WRITETABLE_H__3B5FD4A4_370A_11D3_A10B_0050047FE2E2__INCLUDED_

#include "ReadTable.h"

class WriteTable : public IWriteTable {
protected:

    Table* m_pTable;
    TableContext* m_pCtx;

    ReadTable m_rTable;

    int InsertDuplicateVariableLengthRows (Variant* pvColVal, unsigned int iNumRows);

public:

    WriteTable();
    virtual ~WriteTable();

    inline void SetTable (Table* pTable) {
        m_pTable = pTable;
        m_rTable.SetTable (pTable);
    }

    inline TableContext* GetContext() {
        return m_rTable.GetContext();
    }

    inline ReadTable* GetReadTable() {
        return &m_rTable;
    }

    // IObject
    DECLARE_IOBJECT;

    // IReadTable                                                       
    inline unsigned int GetNumRows (unsigned int* piNumRows) {                                                  
        return m_rTable.GetNumRows (piNumRows);                                                         
    }                                                                                                       
                                                                                                            
    inline int DoesRowExist (unsigned int iKey, bool* pbExists) {                                                       
        return m_rTable.DoesRowExist (iKey, pbExists);                                                              
    }                                                                                                       
                                                                                                            
    inline int GetFirstKey (unsigned int iColumn, int iData, unsigned int* piKey) {                 
        return m_rTable.GetFirstKey (iColumn, iData, piKey);                                                
    }                                                                                                       
                                                                                                            
    inline int GetFirstKey (unsigned int iColumn, float fData, unsigned int* piKey) {                   
        return m_rTable.GetFirstKey (iColumn, fData, piKey);                                                
    }                                                                                                       
                                                                                                            
    inline int GetFirstKey (unsigned int iColumn, const char* pszData, bool bCaseInsensitive,           
                                 unsigned int* piKey) {                                                     
        return m_rTable.GetFirstKey (iColumn, pszData, bCaseInsensitive, piKey);                            
    }                                                                                                       
                                                                                                            
    inline int GetFirstKey (unsigned int iColumn, const UTCTime& tData, unsigned int* piKey) {          
        return m_rTable.GetFirstKey (iColumn, tData, piKey);                                                
    }                                                                                                       
                                                                                                            
    inline int GetFirstKey (unsigned int iColumn, int64 i64Data, unsigned int* piKey) {             
        return m_rTable.GetFirstKey (iColumn, i64Data, piKey);                                          
    }                                                                                                       
                                                                                                            
    inline int GetFirstKey (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive,          
                                 unsigned int* piKey) {                                                     
        return m_rTable.GetFirstKey (iColumn, vData, bCaseInsensitive, piKey);                          
    }                                                                                                       
                                                                                                            
    inline int GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys) {                            
        return m_rTable.GetAllKeys (ppiKey, piNumKeys);                                                 
    }
    
    inline int GetNextKey (unsigned int iKey, unsigned int* piNextKey) {                            
        return m_rTable.GetNextKey (iKey, piNextKey);                                                   
    }
                                                                                                            
    inline int GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive,         
                                  unsigned int** ppiKey, unsigned int* piNumKeys) {                         
        return m_rTable.GetEqualKeys (iColumn, vData, bCaseInsensitive, ppiKey, piNumKeys);             
    }                                                                                                       
                                                                                                            
    inline int ReadData (unsigned int iKey, unsigned int iColumn, int* piData) {                        
        return m_rTable.ReadData (iKey, iColumn, piData);                                                   
    }                                                                                                       
                                                                                                            
    inline int ReadData (unsigned int iKey, unsigned int iColumn, float* pfData) {                      
        return m_rTable.ReadData (iKey, iColumn, pfData);                                                   
    }                                                                                                       
                                                                                                            
    inline int ReadData (unsigned int iKey, unsigned int iColumn, const char** ppszData) {              
        return m_rTable.ReadData (iKey, iColumn, ppszData);                                             
    }                                                                                                       
                                                                                                            
    inline int ReadData (unsigned int iKey, unsigned int iColumn, UTCTime* ptData) {                    
        return m_rTable.ReadData (iKey, iColumn, ptData);                                                   
    }                           
                                
    inline int ReadData (unsigned int iKey, unsigned int iColumn, int64* pi64Data) {                    
        return m_rTable.ReadData (iKey, iColumn, pi64Data);                                             
    }                           
                                
    inline int ReadData (unsigned int iKey, unsigned int iColumn, Variant* pvData) {                    
        return m_rTable.ReadData (iKey, iColumn, pvData);                                                   
    }                           
                                
    inline int ReadData (unsigned int iColumn, int* piData) {                                           
        return m_rTable.ReadData (0, iColumn, piData);                                                  
    }                           
                                
    inline int ReadData (unsigned int iColumn, float* pfData) {                                     
        return m_rTable.ReadData (0, iColumn, pfData);                                                  
    }                           
                                
    inline int ReadData (unsigned int iColumn, const char** ppszData) {                             
        return m_rTable.ReadData (0, iColumn, ppszData);                                                    
    }                           
                                
    inline int ReadData (unsigned int iColumn, UTCTime* ptData) {                                       
        return m_rTable.ReadData (0, iColumn, ptData);                                                  
    }                           
                                
    inline int ReadData (unsigned int iColumn, int64* pi64Data) {                                       
        return m_rTable.ReadData (0, iColumn, pi64Data);                                                    
    }                           
                                
    inline int ReadData (unsigned int iColumn, Variant* pvData) {                                       
        return m_rTable.ReadData (0, iColumn, pvData);                                                  
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int** ppiData,                  
                                unsigned int* piNumRows) {                                                  
        return m_rTable.ReadColumn (iColumn, ppiKey, ppiData, piNumRows);                                   
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, float** ppfData,                
                                unsigned int* piNumRows) {                                                  
        return m_rTable.ReadColumn (iColumn, ppiKey, ppfData, piNumRows);                                   
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, char*** ppszData,               
                                unsigned int* piNumRows) {                                                  
        return m_rTable.ReadColumn (iColumn, ppiKey, ppszData, piNumRows);                              
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, UTCTime** pptData,          
                                unsigned int* piNumRows) {                                                  
        return m_rTable.ReadColumn (iColumn, ppiKey, pptData, piNumRows);                                   
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int64** ppi64Data,          
                                unsigned int* piNumRows) {                                                  
        return m_rTable.ReadColumn (iColumn, ppiKey, ppi64Data, piNumRows);                             
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData,          
                                unsigned int* piNumRows) {                                                  
        return m_rTable.ReadColumn (iColumn, ppiKey, ppvData, piNumRows);                                   
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, int** ppiData, unsigned int* piNumRows) {              
        return m_rTable.ReadColumn (iColumn, NULL, ppiData, piNumRows);                                 
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, float** ppfData, unsigned int* piNumRows) {            
        return m_rTable.ReadColumn (iColumn, NULL, ppfData, piNumRows);                                 
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, char*** ppszData, unsigned int* piNumRows) {           
        return m_rTable.ReadColumn (iColumn, NULL, ppszData, piNumRows);                                    
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, UTCTime** pptData, unsigned int* piNumRows) {          
        return m_rTable.ReadColumn (iColumn, NULL, pptData, piNumRows);                                 
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, int64** ppi64Data, unsigned int* piNumRows) {          
        return m_rTable.ReadColumn (iColumn, NULL, ppi64Data, piNumRows);                                   
    }                           
                                
    inline int ReadColumn (unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows) {          
        return m_rTable.ReadColumn (iColumn, NULL, ppvData, piNumRows);                                 
    }                                                                                                       
                                                                                                            
    inline int ReadColumns (unsigned int iNumColumns, const unsigned int* piColumn, 
        unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows) {
        
        return m_rTable.ReadColumns (iNumColumns, piColumn, ppiKey, pppvData, piNumRows);                           
    }                                                                                                       
                                                                                                            
    inline int ReadRow (unsigned int iKey, void*** ppData) {                                            
        return m_rTable.ReadRow (iKey, ppData);                                                         
    }                                                                                                       
                                                                                                            
    inline int ReadRow (unsigned int iKey, Variant** ppvData) {                                     
        return m_rTable.ReadRow (iKey, ppvData);                                                            
    }                                                                                                       
                                                                                                            
    inline int GetSearchKeys (unsigned int iNumColumns, const unsigned int* piColumn, const unsigned int* piFlags,
        const Variant* pvData, const Variant* pvData2, unsigned int iStartKey, unsigned int iSkipHits, 
        unsigned int iMaxNumHits, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey) {

        return m_rTable.GetSearchKeys (iNumColumns, piColumn, piFlags, pvData, pvData2, iStartKey, iSkipHits,       
            iMaxNumHits, ppiKey, piNumHits, piStopKey);                                                     
    }

    inline int ReadColumnWhereEqual (unsigned int iEqualColumn, const Variant& vData, bool bCaseInsensitive, 
        unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys) {

        return m_rTable.ReadColumnWhereEqual (iEqualColumn, vData, bCaseInsensitive, iReadColumn, ppiKey, 
            ppvData, piNumKeys);
    }

    // IWriteTable
    int WriteData (unsigned int iKey, unsigned int iColumn, int iData);
    int WriteData (unsigned int iKey, unsigned int iColumn, float fData);
    int WriteData (unsigned int iKey, unsigned int iColumn, const char* pszData);
    int WriteData (unsigned int iKey, unsigned int iColumn, const UTCTime& tData);
    int WriteData (unsigned int iKey, unsigned int iColumn, int64 i64Data);
    int WriteData (unsigned int iKey, unsigned int iColumn, const Variant& vData);

    int WriteData (unsigned int iColumn, int iData);
    int WriteData (unsigned int iColumn, float fData);
    int WriteData (unsigned int iColumn, const char* pszData);
    int WriteData (unsigned int iColumn, const UTCTime& tData);
    int WriteData (unsigned int iColumn, int64 i64Data);
    int WriteData (unsigned int iColumn, const Variant& vData);

    int WriteAnd (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteAnd (unsigned int iColumn, unsigned int iBitField);

    int WriteOr (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteOr (unsigned int iColumn, unsigned int iBitField);

    int WriteXor (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteXor (unsigned int iColumn, unsigned int iBitField);

    int WriteNot (unsigned int iKey, unsigned int iColumn);
    int WriteNot (unsigned int iColumn);

    int WriteColumn (unsigned int iColumn, int iData);
    int WriteColumn (unsigned int iColumn, float fData);
    int WriteColumn (unsigned int iColumn, const char* pszData);
    int WriteColumn (unsigned int iColumn, const UTCTime& tData);
    int WriteColumn (unsigned int iColumn, int64 i64Data);
    int WriteColumn (unsigned int iColumn, const Variant& vData);

    int InsertRow (const Variant* pvColVal, unsigned int* piKey);
    int InsertRow (const Variant* pvColVal);

    int InsertRows (const Variant* pvColVal, unsigned int iNumRows);
    int InsertDuplicateRows (const Variant* pvColVal, unsigned int iNumRows);

    int Increment (unsigned int iColumn, const Variant& vIncrement);
    int Increment (unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue);

    int Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement);
    int Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue);

    int DeleteRow (unsigned int iKey);
    int DeleteAllRows();
};

#define IMPLEMENT_IWRITETABLE(CClassName)                                                                   \
                                                                                                            \
int CClassName::WriteData (unsigned int iKey, unsigned int iColumn, int iData) {                            \
    return WriteTable::WriteData (iKey, iColumn, iData);                                                    \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iKey, unsigned int iColumn, float fData) {                          \
    return WriteTable::WriteData (iKey, iColumn, fData);                                                    \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iKey, unsigned int iColumn, const char* pszData) {                  \
    return WriteTable::WriteData (iKey, iColumn, pszData);                                                  \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iKey, unsigned int iColumn, const UTCTime& tData) {                 \
    return WriteTable::WriteData (iKey, iColumn, tData);                                                    \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iKey, unsigned int iColumn, int64 i64Data) {                        \
    return WriteTable::WriteData (iKey, iColumn, i64Data);                                                  \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iKey, unsigned int iColumn, const Variant& vData) {                 \
    return WriteTable::WriteData (0, iColumn, vData);                                                       \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iColumn, int iData) {                                               \
    return WriteTable::WriteData (0, iColumn, iData);                                                       \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iColumn, float fData) {                                             \
    return WriteTable::WriteData (0, iColumn, fData);                                                       \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iColumn, const char* pszData) {                                     \
    return WriteTable::WriteData (0, iColumn, pszData);                                                     \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iColumn, const UTCTime& tData) {                                    \
    return WriteTable::WriteData (0, iColumn, tData);                                                       \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iColumn, int64 i64Data) {                                           \
    return WriteTable::WriteData (0, iColumn, i64Data);                                                     \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteData (unsigned int iColumn, const Variant& vData) {                                    \
    return WriteTable::WriteData (0, iColumn, vData);                                                       \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteAnd (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {                \
    return WriteTable::WriteAnd (iKey, iColumn, iBitField);                                                 \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteAnd (unsigned int iColumn, unsigned int iBitField) {                                   \
    return WriteTable::WriteAnd (0, iColumn, iBitField);                                                    \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteOr (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {                 \
    return WriteTable::WriteOr (iKey, iColumn, iBitField);                                                  \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteOr (unsigned int iColumn, unsigned int iBitField) {                                    \
    return WriteTable::WriteOr (0, iColumn, iBitField);                                                     \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteXor (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {                \
    return WriteTable::WriteXor (iKey, iColumn, iBitField);                                                 \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteXor (unsigned int iColumn, unsigned int iBitField) {                                   \
    return WriteTable::WriteXor (0, iColumn, iBitField);                                                    \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteNot (unsigned int iKey, unsigned int iColumn) {                                        \
    return WriteTable::WriteNot (iKey, iColumn);                                                            \
}                                                                                                           \
                                                                                                            \
int CClassName::WriteNot (unsigned int iColumn) {                                                           \
    return WriteTable::WriteNot (0, iColumn);                                                               \
}                                                                                                           \
                                                                                                            \
int CClassName::InsertRow (const Variant* pvColVal, unsigned int* piKey) {                                  \
    return WriteTable::InsertRow (pvColVal, piKey);                                                         \
}                                                                                                           \
                                                                                                            \
int CClassName::InsertRow (const Variant* pvColVal) {                                                       \
    return WriteTable::InsertRow (pvColVal);                                                                \
}                                                                                                           \
                                                                                                            \
int CClassName::InsertRows (const Variant* pvColVal, unsigned int iNumRows) {                               \
    return WriteTable::InsertRows (pvColVal, iNumRows);                                                     \
}                                                                                                           \
                                                                                                            \
int CClassName::InsertDuplicateRows (const Variant* pvColVal, unsigned int iNumRows) {                      \
    return WriteTable::InsertDuplicateRows (pvColVal, iNumRows);                                            \
}                                                                                                           \
                                                                                                            \
int Increment (unsigned int iColumn, const Variant& vIncrement) {                                           \
    return WriteTable::Increment (0, iColumn, vIncrement, NULL);                                            \
}                                                                                                           \
                                                                                                            \
int Increment (unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue) {                      \
    return WriteTable::Increment (0, iColumn, vIncrement, pvOldValue);                                      \
}                                                                                                           \
                                                                                                            \
int Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement) {                        \
    return WriteTable::Increment (iKey, iColumn, vIncrement, NULL);                                         \
}                                                                                                           \
                                                                                                            \
int Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue) {   \
    return WriteTable::Increment (iKey, iColumn, vIncrement, pvOldValue);                                   \
}                                                                                                           \
                                                                                                            \
int CClassName::DeleteRow (unsigned int iKey) {                                                             \
    return WriteTable::DeleteRow (iKey);                                                                    \
}                                                                                                           \
                                                                                                            \
int CClassName::DeleteAllRows() {                                                                           \
    return WriteTable::DeleteAllRows();                                                                     \
}                                                                                                           \
                                                                                                            \

#endif // !defined(AFX_WRITETABLE_H__3B5FD4A4_370A_11D3_A10B_0050047FE2E2__INCLUDED_)