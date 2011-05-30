// ReadOnlyTable.cpp: implementation of the ReadOnlyTable class.
//
//////////////////////////////////////////////////////////////////////

#define DATABASE_BUILD
#include "TableContext.h"
#include "Table.h"
#undef DATABASE_BUILD

ReadTable::ReadTable() {

    m_iNumRefs = 1;
    m_pTable = NULL;
}

ReadTable::~ReadTable() {
}

unsigned int ReadTable::AddRef() {

    unsigned int iRefs = AddRefInternal();

    if (iRefs == 2) {
        m_tcContext.GetDatabase()->GetGlobalLock()->WaitReader();
        m_pTable->WaitReader();
    }

    return iRefs;
}

unsigned int ReadTable::Release() {

    unsigned int iRefs = ReleaseInternal();

    if (iRefs == 1) {
        m_pTable->SignalReader();
        m_tcContext.GetDatabase()->GetGlobalLock()->SignalReader();
    }

    return iRefs;
}

int ReadTable::QueryInterface (const Uuid& iidInterface, void** ppInterface) {

    if (iidInterface == IID_IObject) {
        *ppInterface = (void*) static_cast<IObject*> (this);
        AddRef();
        return OK;
    }

    if (iidInterface == IID_IReadTable) {
        *ppInterface = (void*) static_cast<IReadTable*> (this);
        AddRef();
        return OK;
    }

    return ERROR_NO_INTERFACE;
}


//
// IReadTable
//

unsigned int ReadTable::GetNumRows (unsigned int* piNumRows) {

    *piNumRows = m_tcContext.GetNumRows();
    return OK;
}

int ReadTable::DoesRowExist (unsigned int iKey, bool* pbExists) {

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());
    *pbExists = IsValidKey (iKey);
    return OK;
}

int ReadTable::ReadData (unsigned int iColumn, int* piData) {
    Assert (m_tcContext.IsOneRow());
    return ReadTable::ReadData (0, iColumn, piData);
}

int ReadTable::ReadData (unsigned int iColumn, float* pfData) {
    Assert (m_tcContext.IsOneRow());
    return ReadTable::ReadData (0, iColumn, pfData);
}

int ReadTable::ReadData (unsigned int iColumn, const char** ppszData) {
    Assert (m_tcContext.IsOneRow());
    return ReadTable::ReadData (0, iColumn, ppszData);
}

int ReadTable::ReadData (unsigned int iColumn, UTCTime* ptData) {
    Assert (m_tcContext.IsOneRow());
    return ReadTable::ReadData (0, iColumn, ptData);
}

int ReadTable::ReadData (unsigned int iColumn, int64* pi64Data) {
    Assert (m_tcContext.IsOneRow());
    return ReadTable::ReadData (0, iColumn, pi64Data);
}

int ReadTable::ReadData (unsigned int iColumn, Variant* pvData) {
    Assert (m_tcContext.IsOneRow());
    return ReadTable::ReadData (0, iColumn, pvData);
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, int* piData) {

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());

    if (!IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *piData = *((int*) m_tcContext.GetData (iKey, iColumn));

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, float* pfData) {

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());

    if (!IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_FLOAT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *pfData = *((float*) m_tcContext.GetData (iKey, iColumn));

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, UTCTime* ptData) {

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());

    if (!IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_TIME) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *ptData = *((UTCTime*) m_tcContext.GetData (iKey, iColumn));

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, int64* pi64Data) {

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());

    if (!IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_INT64) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *pi64Data = *((int64*) m_tcContext.GetData (iKey, iColumn));

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, const char** ppszData) {

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());

    if (!IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_STRING) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *ppszData = m_tcContext.GetStringData (iKey, iColumn);

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, Variant* pvData) {

    int iErrCode;

    int iData;
    int64 i64Data;
    float fData;
    UTCTime tData;
    const char* pszData;

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    switch (m_tcContext.GetColumnType (iColumn)) {

    case V_INT:

        iErrCode = ReadData (iKey, iColumn, &iData);
        if (iErrCode == OK) {
            *pvData = iData;
        }
        break;

    case V_TIME:
        
        iErrCode = ReadData (iKey, iColumn, &tData);
        if (iErrCode == OK) {
            *pvData = tData;
        }
        break;

    case V_FLOAT:
        
        iErrCode = ReadData (iKey, iColumn, &fData);
        if (iErrCode == OK) {
            *pvData = fData;
        }
        break;

    case V_STRING:
        
        iErrCode = ReadData (iKey, iColumn, &pszData);
        if (iErrCode == OK) {
            *pvData = pszData;
        }
        break;

    case V_INT64:

        iErrCode = ReadData (iKey, iColumn, &i64Data);
        if (iErrCode == OK) {
            *pvData = i64Data;
        }
        break;

    default:

        Assert (false);
        return ERROR_DATA_CORRUPTION;
    }

    return iErrCode;
}


int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows) {

    const unsigned int iNumRows = m_tcContext.GetNumRows();

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppiData = NULL;

    if (iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *ppiData = new int [iNumRows];
    if (*ppiData == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [iNumRows];
        if (*ppiKey == NULL) {
            delete [] (*ppiData);
            *ppiData = NULL;
            return ERROR_OUT_OF_MEMORY;
        }
    }

    unsigned int i, iTerminatorKey = m_tcContext.GetTerminatorRowKey();

    for (i = 0; i < iTerminatorKey; i ++) {

        if (m_tcContext.IsValidRow (i)) {
            
            Assert (*piNumRows < iNumRows);
            (*ppiData)[*piNumRows] = *((int*) m_tcContext.GetData (i, iColumn));
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == iNumRows);

    return OK;
}


int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, float** ppfData, unsigned int* piNumRows) {

    const unsigned int iNumRows = m_tcContext.GetNumRows();

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppfData = NULL;
    
    if (iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_FLOAT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *ppfData = new float [iNumRows];
    if (*ppfData == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [iNumRows];
        if (*ppiKey == NULL) {
            delete [] (*ppfData);
            *ppfData = NULL;
            return ERROR_OUT_OF_MEMORY;
        }
    }

    unsigned int i, iTerminatorKey = m_tcContext.GetTerminatorRowKey();

    for (i = 0; i < iTerminatorKey; i ++) {

        if (m_tcContext.IsValidRow (i)) {

            Assert (*piNumRows < iNumRows);
            (*ppfData)[*piNumRows] = *((float*) m_tcContext.GetData (i, iColumn));
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == iNumRows);

    return OK;
}


int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, char*** pppszData, unsigned int* piNumRows) {

    const unsigned int iNumRows = m_tcContext.GetNumRows();

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *pppszData = NULL;

    if (iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_STRING) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *pppszData = new char* [iNumRows];
    if (*pppszData == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [iNumRows];
        if (*ppiKey == NULL) {
            delete [] (*pppszData);
            *pppszData = NULL;
            return ERROR_OUT_OF_MEMORY;
        }
    }

    unsigned int i, iTerminatorKey = m_tcContext.GetTerminatorRowKey();

    for (i = 0; i < iTerminatorKey; i ++) {

        if (m_tcContext.IsValidRow (i)) {

            Assert (*piNumRows < iNumRows);
            (*pppszData)[*piNumRows] = (char*) m_tcContext.GetStringData (i, iColumn);
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == iNumRows);

    return OK;
}

int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, UTCTime** pptData, unsigned int* piNumRows) {
    
    unsigned int iNumRows;

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *pptData = NULL;

    iNumRows = m_tcContext.GetNumRows();
    if (iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_TIME) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *pptData = new UTCTime [iNumRows];
    if (*pptData == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [iNumRows];
        if (*ppiKey == NULL) {
            delete [] (*pptData);
            *pptData = NULL;
            return ERROR_OUT_OF_MEMORY;
        }
    }

    unsigned int i, iTerminatorKey = m_tcContext.GetTerminatorRowKey();

    for (i = 0; i < iTerminatorKey; i ++) {

        if (m_tcContext.IsValidRow (i)) {

            Assert (*piNumRows < iNumRows);
            (*pptData)[*piNumRows] = *((UTCTime*) m_tcContext.GetData (i, iColumn));
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == iNumRows);

    return OK;
}

int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows) {

    unsigned int iNumRows;

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppi64Data = NULL;

    iNumRows = m_tcContext.GetNumRows();
    if (iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_INT64) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *ppi64Data = new int64 [iNumRows];
    if (*ppi64Data == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [iNumRows];
        if (*ppiKey == NULL) {
            delete [] (*ppi64Data);
            *ppi64Data = NULL;
            return ERROR_OUT_OF_MEMORY;
        }
    }

    unsigned int i, iTerminatorKey = m_tcContext.GetTerminatorRowKey();

    for (i = 0; i < iTerminatorKey; i ++) {

        if (m_tcContext.IsValidRow (i)) {

            Assert (*piNumRows < iNumRows);
            (*ppi64Data)[*piNumRows] = *((int64*) m_tcContext.GetData (i, iColumn));
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == iNumRows);

    return OK;
}

int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, 
                           unsigned int* piNumRows) {

    unsigned int iNumRows;

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppvData = NULL;

    iNumRows = m_tcContext.GetNumRows();
    if (iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (!IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    // Allocate space
    *ppvData = new Variant [iNumRows];
    if (*ppvData == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [iNumRows];
        if (*ppiKey == NULL) {
            delete [] (*ppvData);
            *ppvData = NULL;
            return ERROR_OUT_OF_MEMORY;
        }
    }

    unsigned int i, iTerminatorKey = m_tcContext.GetTerminatorRowKey();

    switch (m_tcContext.GetColumnType (iColumn)) {

    case V_INT:
        
        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_tcContext.IsValidRow (i)) {

                Assert (*piNumRows < iNumRows);
                (*ppvData)[*piNumRows] = *((int*) m_tcContext.GetData (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_TIME:

        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_tcContext.IsValidRow (i)) {

                Assert (*piNumRows < iNumRows);
                (*ppvData)[*piNumRows] = *((UTCTime*) m_tcContext.GetData (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_FLOAT:

        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_tcContext.IsValidRow (i)) {

                Assert (*piNumRows < iNumRows);
                (*ppvData)[*piNumRows] = *((float*) m_tcContext.GetData (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_STRING:

        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_tcContext.IsValidRow (i)) {

                Assert (*piNumRows < iNumRows);
                (*ppvData)[*piNumRows] = m_tcContext.GetStringData (i, iColumn);
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_INT64:
        
        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_tcContext.IsValidRow (i)) {

                Assert (*piNumRows < iNumRows);
                (*ppvData)[*piNumRows] = *((int64*) m_tcContext.GetData (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    default:

        Assert (false);
        delete [] (*ppvData);
        *ppvData = NULL;
        *piNumRows = 0;
        return ERROR_DATA_CORRUPTION;
    }

    Assert (*piNumRows == iNumRows);

    return OK;
}

int ReadTable::ReadColumn (unsigned int iColumn, int** ppiData, unsigned int* piNumRows) {
    return ReadTable::ReadColumn (iColumn, NULL, ppiData, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, float** ppfData, unsigned int* piNumRows) {
    return ReadTable::ReadColumn (iColumn, NULL, ppfData, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, char*** ppszData, unsigned int* piNumRows) {
    return ReadTable::ReadColumn (iColumn, NULL, ppszData, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, UTCTime** pptData, unsigned int* piNumRows) {
    return ReadTable::ReadColumn (iColumn, NULL, pptData, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, int64** ppi64Data, unsigned int* piNumRows) {
    return ReadTable::ReadColumn (iColumn, NULL, ppi64Data, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows) {
    return ReadTable::ReadColumn (iColumn, NULL, ppvData, piNumRows);
}

int ReadTable::ReadRow (unsigned int iKey, void*** pppData) {

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());

    *pppData = NULL;

    if (!IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    unsigned int i, iNumCols = m_tcContext.GetNumColumns();

    *pppData = new void* [iNumCols];
    if (*pppData == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    for (i = 0; i < iNumCols; i ++) {

        if (m_tcContext.GetColumnType (i) == V_STRING) {
            (*pppData)[i] = (void*) m_tcContext.GetStringData (iKey, i);
        } else {
            (*pppData)[i] = m_tcContext.GetData (iKey, i);
        }
    }

    return OK;
}


int ReadTable::ReadRow (unsigned int iKey, Variant** ppvData) {

    Assert ((m_tcContext.IsOneRow() && iKey == 0) || !m_tcContext.IsOneRow());

    *ppvData = NULL;

    if (!IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    unsigned int i, iNumCols = m_tcContext.GetNumColumns();
    
    *ppvData = new Variant [iNumCols];
    if (*ppvData == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    for (i = 0; i < iNumCols; i ++) {
        
        switch (m_tcContext.m_pTemplate->TemplateData.Type [i]) {
            
        case V_INT:
            
            (*ppvData)[i] = *((int*) m_tcContext.GetData (iKey, i));
            break;
            
        case V_TIME:
            
            (*ppvData)[i] = *((UTCTime*) m_tcContext.GetData (iKey, i));
            break;
            
        case V_FLOAT:
            
            (*ppvData)[i] = *((float*) m_tcContext.GetData (iKey, i));
            break;
            
        case V_STRING:
            
            (*ppvData)[i] = m_tcContext.GetStringData (iKey, i);
            break;

        case V_INT64:
            
            (*ppvData)[i] = *((int64*) m_tcContext.GetData (iKey, i));
            break;
            
        default:
            
            Assert (false);
            
            delete [] (*ppvData);
            *ppvData = NULL;

            return ERROR_DATA_CORRUPTION;
        }
    }

    return OK;
}


int ReadTable::GetFirstKey (unsigned int iColumn, int iData, unsigned int* piKey) {

    int iErrCode;

    *piKey = NO_KEY;

    if (m_tcContext.GetNumRows() == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_tcContext.IndexGetFirstKey (iColumn, iData, piKey);
    if (iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        return iErrCode;
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {

        iRowKey = m_tcContext.FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }

        if (*(int*) m_tcContext.GetData (iRowKey, iColumn) == iData) {
            *piKey = iRowKey;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}

int ReadTable::GetFirstKey (unsigned int iColumn, int64 i64Data, unsigned int* piKey) {

    int iErrCode;

    *piKey = NO_KEY;

    if (m_tcContext.GetNumRows() == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_INT64) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_tcContext.IndexGetFirstKey (iColumn, i64Data, piKey);
    if (iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        return iErrCode;
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {

        iRowKey = m_tcContext.FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }

        if (*(int64*) m_tcContext.GetData (iRowKey, iColumn) == i64Data) {
            *piKey = iRowKey;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}


int ReadTable::GetFirstKey (unsigned int iColumn, float fData, unsigned int* piKey) {

    int iErrCode;

    *piKey = NO_KEY;

    if (m_tcContext.GetNumRows() == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_FLOAT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_tcContext.IndexGetFirstKey (iColumn, fData, piKey);
    if (iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        return iErrCode;
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {

        iRowKey = m_tcContext.FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }

        if (*(float*) m_tcContext.GetData (iRowKey, iColumn) == fData) {
            *piKey = iRowKey;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}

int ReadTable::GetFirstKey (unsigned int iColumn, const UTCTime& tData, unsigned int* piKey) {

    int iErrCode;

    *piKey = NO_KEY;

    if (m_tcContext.GetNumRows() == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_TIME) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_tcContext.IndexGetFirstKey (iColumn, tData, piKey);
    if (iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        return iErrCode;
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {

        iRowKey = m_tcContext.FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }

        if (*(UTCTime*) m_tcContext.GetData (iRowKey, iColumn) == tData) {
            *piKey = iRowKey;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}


int ReadTable::GetFirstKey (unsigned int iColumn, const char* pszData, bool bCaseInsensitive, 
                            unsigned int* piKey) {

    int iErrCode;

    *piKey = NO_KEY;

    if (m_tcContext.GetNumRows() == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_tcContext.GetColumnType (iColumn) != V_STRING) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_tcContext.IndexGetFirstKey (iColumn, pszData, bCaseInsensitive, piKey);
    if (iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        return iErrCode;
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {

        iRowKey = m_tcContext.FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }

        const char* pszOnDisk = m_tcContext.GetStringData (iRowKey, iColumn);

        if (bCaseInsensitive) {
            if (String::StriCmp (pszData, pszOnDisk) == 0) {
                *piKey = iRowKey;
                return OK;
            }
        } else {
            if (String::StrCmp (pszData, pszOnDisk) == 0) {
                *piKey = iRowKey;
                return OK;
            }
        }
    }

    return ERROR_DATA_NOT_FOUND;
}

int ReadTable::GetFirstKey (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, 
                            unsigned int* piKey) {

    *piKey = NO_KEY;

    VariantType vtType = vData.GetType();

    if (m_tcContext.GetColumnType (iColumn) != vtType) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    switch (vtType) {

    case V_INT:

        return ReadTable::GetFirstKey (iColumn, vData.GetInteger(), piKey);
        break;

    case V_TIME:

        return ReadTable::GetFirstKey (iColumn, vData.GetUTCTime(), piKey);
        break;

    case V_FLOAT:

        return ReadTable::GetFirstKey (iColumn, vData.GetFloat(), piKey);
        break;

    case V_STRING:

        return ReadTable::GetFirstKey (iColumn, vData.GetCharPtr(), bCaseInsensitive, piKey);
        break;

    case V_INT64:

        return ReadTable::GetFirstKey (iColumn, vData.GetInteger64(), piKey);
        break;

    default:

        Assert (false);
        *piKey = NO_KEY;
        return ERROR_DATA_CORRUPTION;
    }
}

int ReadTable::GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys) {

    *piNumKeys = 0;
    *ppiKey = NULL; 

    unsigned int iNumRows = m_tcContext.GetNumRows();
    if (iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    unsigned int* piKey = new unsigned int [iNumRows];
    if (piKey == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    unsigned int iRowKey = NO_KEY, iNumKeys = 0;
    while (true) {

        iRowKey = m_tcContext.FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }

        piKey[iNumKeys ++] = iRowKey;
    }

    Assert (iNumKeys == iNumRows);

    *ppiKey = piKey;
    *piNumKeys = iNumKeys;

    return OK;
}

int ReadTable::GetNextKey (unsigned int iKey, unsigned int* piNextKey) {

    *piNextKey = NO_KEY;

    if (iKey != NO_KEY && iKey >= m_tcContext.GetTerminatorRowKey()) {
        return ERROR_DATA_NOT_FOUND;
    }

    *piNextKey = m_tcContext.FindNextValidRow (iKey);

    return (*piNextKey == NO_KEY) ? ERROR_DATA_NOT_FOUND : OK;
}

int ReadTable::GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, 
                             unsigned int** ppiKey, unsigned int* piNumKeys) {

    int iErrCode;

    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *piNumKeys = 0;

    VariantType vtType = vData.GetType();

    if (vtType != m_tcContext.GetColumnType (iColumn)) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    unsigned int iNumRows = m_tcContext.GetNumRows();
    if (iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    iErrCode = m_tcContext.IndexGetEqualKeys (iColumn, vData, bCaseInsensitive, ppiKey, piNumKeys);
    if (iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        return iErrCode;
    }

    unsigned int* piKey = NULL, iNumKeys = 0, iKeySpace = 0, iRowKey = NO_KEY;

    switch (vtType) {

    case V_INT:
        {

        int iData = vData.GetInteger();

        while (true) {
            
            iRowKey = m_tcContext.FindNextValidRow (iRowKey);
            if (iRowKey == NO_KEY) {
                break;
            }
            
            if (*(int*) m_tcContext.GetData (iRowKey, iColumn) == iData) {

                if (ppiKey != NULL) {
                    iErrCode = AppendKey (piKey, iNumKeys, iKeySpace, iRowKey);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }                   
                }
                iNumKeys ++;
            }
        }

        }
        break;

    case V_FLOAT:
        {

        float fData = vData.GetFloat();

        while (true) {
            
            iRowKey = m_tcContext.FindNextValidRow (iRowKey);
            if (iRowKey == NO_KEY) {
                break;
            }
            
            if (*(float*) m_tcContext.GetData (iRowKey, iColumn) == fData) {

                if (ppiKey != NULL) {
                    iErrCode = AppendKey (piKey, iNumKeys, iKeySpace, iRowKey);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }                   
                }
                iNumKeys ++;
            }
        }

        }
        break;
        
    case V_TIME:
        {

        UTCTime tData = vData.GetUTCTime();

        while (true) {
            
            iRowKey = m_tcContext.FindNextValidRow (iRowKey);
            if (iRowKey == NO_KEY) {
                break;
            }
            
            if (*(UTCTime*) m_tcContext.GetData (iRowKey, iColumn) == tData) {

                if (ppiKey != NULL) {
                    iErrCode = AppendKey (piKey, iNumKeys, iKeySpace, iRowKey);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }                   
                }
                iNumKeys ++;
            }
        }

        }
        break;

    case V_STRING:
        {
        const char* pszData = vData.GetCharPtr();

        while (true) {

            iRowKey = m_tcContext.FindNextValidRow (iRowKey);
            if (iRowKey == NO_KEY) {
                break;
            }
            
            const char* pszOnDisk = m_tcContext.GetStringData (iRowKey, iColumn);
            
            if (bCaseInsensitive) {
                if (String::StriCmp (pszData, pszOnDisk) == 0) {
                    
                    if (ppiKey != NULL) {
                        iErrCode = AppendKey (piKey, iNumKeys, iKeySpace, iRowKey);
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }                   
                    }
                    iNumKeys ++;
                }
            } else {
                if (String::StrCmp (pszData, pszOnDisk) == 0) {
                    
                    if (ppiKey != NULL) {
                        iErrCode = AppendKey (piKey, iNumKeys, iKeySpace, iRowKey);
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }                   
                    }
                    iNumKeys ++;
                }
            }
        }

        }
        break;

    case V_INT64:
        {

        int64 i64Data = vData.GetInteger64();

        while (true) {
            
            iRowKey = m_tcContext.FindNextValidRow (iRowKey);
            if (iRowKey == NO_KEY) {
                break;
            }
            
            if (*(int64*) m_tcContext.GetData (iRowKey, iColumn) == i64Data) {

                if (ppiKey != NULL) {
                    iErrCode = AppendKey (piKey, iNumKeys, iKeySpace, iRowKey);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }                   
                }
                iNumKeys ++;
            }
        }

        }
        break;

    default:

        if (piKey != NULL) {
            delete [] piKey;
        }

        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    if (iNumKeys == 0) {
        if (piKey != NULL) {
            delete [] piKey;
            piKey = NULL;
        }
    }

    if (ppiKey != NULL) {
        *ppiKey = piKey;
    }
    *piNumKeys = iNumKeys;

    return iNumKeys > 0 ? OK : ERROR_DATA_NOT_FOUND;
}

// Meaning:
// pvData[i].GetType() is not V_STRING:
// Range between pvData[i] and pvData2[i], inclusive
//
// pvData[i].GetType() is V_STRING:
// pvData2[i].GetInteger() indicates SUBSTRING_SEARCH, EXACT_SEARCH or BEGINS_WITH_SEARCH

int ReadTable::GetSearchKeys (unsigned int iNumColumns, const unsigned int* piColumn, const unsigned int* piFlags,
                              const Variant* pvData, const Variant* pvData2, unsigned int iStartKey, 
                              unsigned int iSkipHits, unsigned int iMaxNumHits, unsigned int** ppiKey, 
                              unsigned int* piNumHits, unsigned int* piStopKey) {

    int iErrCode;

    *piNumHits = 0;

    if (piStopKey != NULL) {
        *piStopKey = NO_KEY;
    }

    if (m_tcContext.GetNumRows() == 0) {
        *ppiKey = NULL;
        return ERROR_DATA_NOT_FOUND;
    }

    // Check for errors
    unsigned int i, j, iRealNumColumns = m_tcContext.GetNumColumns(), iNumHitsSkipped = 0;

    for (i = 0; i < iNumColumns; i ++) {

        if (piColumn[i] == NO_KEY) {

            if (pvData[i].GetType() != V_INT || pvData2[i].GetType() != V_INT) {
                *ppiKey = NULL;
                return ERROR_TYPE_MISMATCH;
            }
            continue;
        }

        if (piColumn[i] >= iRealNumColumns) {
            Assert (false);
            *ppiKey = NULL;
            return ERROR_UNKNOWN_COLUMN_INDEX;
        }

        if (pvData[i].GetType() != m_tcContext.GetColumnType (piColumn[i])) {           
            Assert (false);
            *ppiKey = NULL;
            return ERROR_TYPE_MISMATCH;
        }
    }

    // Allocate space for keys
    size_t stKeySpace = 16;
    *ppiKey = new unsigned int [stKeySpace];
    if (*ppiKey == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    String strData, strPvData;
    unsigned int* piTemp;

    iErrCode = ERROR_DATA_NOT_FOUND;

    // Loop through all keys and match all conditions
    bool bHit;
    unsigned int iTerminatorRowKey = m_tcContext.GetTerminatorRowKey();

    if (iStartKey == NO_KEY) {
        iStartKey = 0;
    }

    for (i = iStartKey; i < iTerminatorRowKey; i ++) {

        if (m_tcContext.IsValidRow (i)) {

            // Match search parameters
            bHit = true;

            for (j = 0; j < iNumColumns && bHit; j ++) {

                switch (pvData[j].GetType()) {
                    
                case V_INT:

                    int iData;
                    
                    switch (piFlags[j]) {
                        
                    case 0:                 

                        if (piColumn[j] == NO_KEY) {
                            iData = i;
                        } else {
                            iData = *((int*) m_tcContext.GetData (i, piColumn[j]));
                        }

                        if (iData < pvData[j].GetInteger() || iData > pvData2[j].GetInteger()) {
                            bHit = false;
                        }

                        break;

                    case SEARCH_AND:
                    case SEARCH_NOTAND:

                        iData = *((int*) m_tcContext.GetData (i, piColumn[j]));

                        if (piFlags[j] == SEARCH_AND) {
                            bHit = (iData & pvData[j].GetInteger()) != 0;
                        } else {
                            bHit = (iData & pvData[j].GetInteger()) == 0;
                        }

                        break;

                    default:

                        bHit = false;
                        break;
                    }
                    break;

                case V_FLOAT:

                    {

                    float fData = *((float*) m_tcContext.GetData (i, piColumn[j]));

                    if (fData < pvData[j].GetFloat() || fData > pvData2[j].GetFloat()) {
                        bHit = false;
                    }
                    
                    }
                    break;

                case V_TIME:

                    {

                    UTCTime tData = *((UTCTime*) m_tcContext.GetData (i, piColumn[j]));

                    if (tData < pvData[j].GetUTCTime() || tData > pvData2[j].GetUTCTime()) {
                        bHit = false;
                    }
                    
                    }
                    break;
                    
                case V_STRING:

                    switch (piFlags[j]) {

                    case SEARCH_SUBSTRING | SEARCH_CASE_SENSITIVE:

                        if (String::StrStr (
                            m_tcContext.GetStringData (i, piColumn[j]), pvData[j].GetCharPtr()
                            ) == NULL) {
                            bHit = false;
                        }
                        break;

                    case SEARCH_SUBSTRING:

                        const char* pszStr;
                        iErrCode = String::StriStr (
                            m_tcContext.GetStringData (i, piColumn[j]), pvData[j].GetCharPtr(), &pszStr
                            );

                        if (iErrCode != OK) {
                            delete [] (*ppiKey);
                            *ppiKey = NULL;
                            *piNumHits = 0;
                            return iErrCode;
                        }

                        if (pszStr == NULL) {
                            bHit = false;
                        }
                        break;

                    case SEARCH_EXACT | SEARCH_CASE_SENSITIVE:

                        if (String::StrCmp (m_tcContext.GetStringData (i, piColumn[j]), pvData[j].GetCharPtr()) != 0) {
                            bHit = false;
                        }
                        break;

                    case SEARCH_EXACT:
                        
                        if (String::StriCmp (m_tcContext.GetStringData (i, piColumn[j]), pvData[j].GetCharPtr()) != 0) {
                            bHit = false;
                        }
                        break;

                    case SEARCH_BEGINS_WITH | SEARCH_CASE_SENSITIVE:

                        if (String::StrnCmp (m_tcContext.GetStringData (i, piColumn[j]), pvData[j].GetCharPtr(), 
                            String::StrLen (pvData[j].GetCharPtr())) != 0) {
                            bHit = false;
                        }

                    case SEARCH_BEGINS_WITH:

                        if (String::StrniCmp (m_tcContext.GetStringData (i, piColumn[j]), pvData[j].GetCharPtr(), 
                            String::StrLen (pvData[j].GetCharPtr())) != 0) {
                            bHit = false;
                        }
                        break;

                    default:

                        bHit = false;
                        break;
                    }
                    break;

                case V_INT64:
                    {

                    int64 i64Data;

                    if (piColumn[j] == NO_KEY) {
                        i64Data = i;
                    } else {
                        i64Data = *((int64*) m_tcContext.GetData (i, piColumn[j]));
                    }

                    if (i64Data < pvData[j].GetInteger64() || i64Data > pvData2[j].GetInteger64()) {
                        bHit = false;
                    }
                    
                    }
                    break;

                default:

                    Assert (false);
                    bHit = false;
                    break;
                }
            }

            if (bHit) {

                if (iNumHitsSkipped < iSkipHits) {
                    iNumHitsSkipped ++;
                } else {

                    if (iMaxNumHits != 0 && *piNumHits == iMaxNumHits) {
                        
                        // Abort!
                        if (piStopKey != NULL) {
                            *piStopKey = i;
                        }
                        iErrCode = ERROR_TOO_MANY_HITS;
                        break;
                    }
                    
                    // Add to hits list
                    if (*piNumHits == stKeySpace) {
                        
                        stKeySpace *= 2;
                        piTemp = new unsigned int [stKeySpace];
                        if (piTemp == NULL) {

                            delete [] (*ppiKey);
                            *ppiKey = NULL;
                            *piNumHits = 0;

                            return ERROR_OUT_OF_MEMORY;
                        }

                        memcpy (piTemp, *ppiKey, *piNumHits * sizeof (unsigned int));

                        delete [] (*ppiKey);
                        *ppiKey = piTemp;
                    }
                    
                    (*ppiKey)[*piNumHits] = i;
                    (*piNumHits) ++;
                }
            }
        }
    }

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        
        if (*piNumHits > 0) {
            iErrCode = OK;
        } else {

            delete [] (*ppiKey);
            *ppiKey = NULL;
        }
    }

    return iErrCode;
}

int ReadTable::ReadColumns (unsigned int iNumColumns, const unsigned int* piColumn, unsigned int** ppiKey, 
                            Variant*** pppvData, unsigned int* piNumRows) {
    
    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *pppvData = NULL;

    unsigned int i, j, k, iRealNumColumns = m_tcContext.GetNumColumns();

    // Check column boundaries
    for (i = 0; i < iNumColumns; i ++) {
        if (piColumn[i] >= iRealNumColumns) {
            Assert (false);
            return ERROR_UNKNOWN_COLUMN_INDEX;
        }
    }

    if (m_tcContext.GetNumRows() == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    // Allocate space
    Variant* pvData = new Variant [iNumColumns * m_tcContext.GetNumRows()];
    if (pvData == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    *pppvData = new Variant* [m_tcContext.GetNumRows()];
    if (*pppvData == NULL) {
        delete [] pvData;
        return ERROR_OUT_OF_MEMORY;
    }

    for (i = 0; i < m_tcContext.GetNumRows(); i ++) {
        (*pppvData)[i] = &(pvData[i * iNumColumns]);
    }

    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_tcContext.GetNumRows()];
        if (*ppiKey == NULL) {
            delete [] pvData;
            delete [] (*pppvData);
            return ERROR_OUT_OF_MEMORY;
        }
    }

    unsigned int iTerminatorRowKey = m_tcContext.GetTerminatorRowKey();

    for (i = 0; i < iTerminatorRowKey; i ++) {
        
        if (m_tcContext.IsValidRow (i)) {
            
            for (j = 0; j < iNumColumns; j ++) {

                switch (m_tcContext.GetColumnType (piColumn[j])) {

                case V_INT:
                    (*pppvData)[*piNumRows][j] = *((int*) m_tcContext.GetData (i, piColumn[j]));
                    break;

                case V_FLOAT:
                    (*pppvData)[*piNumRows][j] = *((float*) m_tcContext.GetData (i, piColumn[j]));
                    break;

                case V_TIME:
                    (*pppvData)[*piNumRows][j] = *((UTCTime*) m_tcContext.GetData (i, piColumn[j]));
                    break;

                case V_STRING:
                    (*pppvData)[*piNumRows][j] = m_tcContext.GetStringData (i, piColumn[j]);
                    break;

                case V_INT64:
                    (*pppvData)[*piNumRows][j] = *((int64*) m_tcContext.GetData (i, piColumn[j]));
                    break;
                    
                default:
                    
                    Assert (false);
                    
                    for (k = 0; k < m_tcContext.GetNumRows(); k ++) {
                        delete [] (*pppvData)[k];
                    }
                    delete [] (*pppvData);
                    *pppvData = NULL;
                    
                    if (ppiKey != NULL) {
                        delete [] (*ppiKey);
                        *ppiKey = NULL;
                    }
                    
                    return ERROR_DATA_CORRUPTION;
                }
                
            }
            
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    return OK;
}

int ReadTable::ReadColumnWhereEqual (unsigned int iEqualColumn, const Variant& vData, bool bCaseInsensitive, 
                                     unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, 
                                     unsigned int* piNumKeys) {

    int iErrCode;

    *piNumKeys = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppvData = NULL;

    iErrCode = GetEqualKeys (iEqualColumn, vData, bCaseInsensitive, ppiKey, piNumKeys);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (*piNumKeys > 0) {

        unsigned int i;

        *ppvData = new Variant [*piNumKeys];
        if (*ppvData == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        for (i = 0; i < *piNumKeys; i ++) {

            iErrCode = ReadData ((*ppiKey)[i], iReadColumn, (*ppvData) + i);
            if (iErrCode != OK) {
                goto Cleanup;
            }
        }
    }

Cleanup:

    if (iErrCode != OK) {

        if (ppiKey != NULL && *ppiKey != NULL) {
            delete [] (*ppiKey);
            *ppiKey = NULL;
        }

        if (*ppvData != NULL) {
            delete [] (*ppvData);
            *ppvData = NULL;
        }
    }

    return iErrCode;
}