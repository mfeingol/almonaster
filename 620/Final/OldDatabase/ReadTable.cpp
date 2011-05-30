// ReadOnlyTable.cpp: implementation of the ReadOnlyTable class.
//
//////////////////////////////////////////////////////////////////////

#define DATABASE_BUILD
#include "ReadTable.h"
#undef DATABASE_BUILD

ReadTable::ReadTable (Table* pTable) {

    m_pTable = pTable;
    m_pTable->AddRef();

    m_bWriteTable = false;
    m_iRefCount = 1;
}

ReadTable::~ReadTable() {

    if (m_bWriteTable) {
        m_pTable->SignalWriter();
    } else {
        m_pTable->SignalReader();
    }
    m_pTable->Release();
}

unsigned int ReadTable::AddRef() {

    return Algorithm::AtomicIncrement (&m_iRefCount);
}

unsigned int ReadTable::Release() {

    int iRefCount = Algorithm::AtomicDecrement (&m_iRefCount);
    if (iRefCount == 0) {
        delete this;
    }

    return iRefCount;
}

int ReadTable::QueryInterface (const Uuid& iidInterface, void** ppInterface) {

    if (iidInterface == IID_IReadTable) {
        *ppInterface = static_cast<IReadTable*> (this);
        AddRef();
        return OK;
    }

    *ppInterface = NULL;
    return ERROR_NO_INTERFACE;
}

ReadTable* ReadTable::CreateInstance (Table* pTable) {
    return new ReadTable (pTable);
}

int ReadTable::CheckLoad() {
    return m_pTable->CheckLoad();
}

int ReadTable::GetNumRows (unsigned int* piNumRows) {
    return m_pTable->GetNumRows (piNumRows);
}

int ReadTable::DoesRowExist (unsigned int iKey, bool* pbExists) {
    *pbExists = !(iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID);
    return OK;
}

int ReadTable::ReadData (unsigned int iColumn, int* piData) {
    return ReadData (0, iColumn, piData);
}

int ReadTable::ReadData (unsigned int iColumn, float* pfData) {
    return ReadData (0, iColumn, pfData);
}

int ReadTable::ReadData (unsigned int iColumn, const char** ppszData) {
    return ReadData (0, iColumn, ppszData);
}

int ReadTable::ReadData (unsigned int iColumn, UTCTime* ptData) {
    return ReadData (0, iColumn, ptData);
}

int ReadTable::ReadData (unsigned int iColumn, int64* pi64Data) {
    return ReadData (0, iColumn, pi64Data);
}

int ReadTable::ReadData (unsigned int iColumn, Variant* pvData) {
    return ReadData (0, iColumn, pvData);
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, int* piData) {

    CHECK_LOAD;

    if (iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *piData = *((int*) m_pTable->Data (iKey, iColumn));

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, float* pfData) {

    CHECK_LOAD;

    if (iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_FLOAT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *pfData = *((float*) m_pTable->Data (iKey, iColumn));

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, UTCTime* ptData) {

    CHECK_LOAD;

    if (iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_TIME) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *ptData = *((UTCTime*) m_pTable->Data (iKey, iColumn));

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, int64* pi64Data) {

    CHECK_LOAD;

    if (iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_INT64) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *pi64Data = *((int64*) m_pTable->Data (iKey, iColumn));

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, const char** ppszData) {

    CHECK_LOAD;

    if (iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_STRING) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    *ppszData = (char*) m_pTable->Data (iKey, iColumn);

    return OK;
}

int ReadTable::ReadData (unsigned int iKey, unsigned int iColumn, Variant* pvData) {

    CHECK_LOAD;

    if (iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    switch (m_pTable->m_pTemplate->TemplateData.Type [iColumn]) {

    case V_INT:

        *pvData = *((int*) m_pTable->Data (iKey, iColumn));
        break;

    case V_TIME:

        *pvData = *((UTCTime*) m_pTable->Data (iKey, iColumn));
        break;

    case V_FLOAT:

        *pvData = *((float*) m_pTable->Data (iKey, iColumn));
        break;

    case V_STRING:

        *pvData = (char*) m_pTable->Data (iKey, iColumn);
        break;

    case V_INT64:

        *pvData = *((int64*) m_pTable->Data (iKey, iColumn));
        break;

    default:

        Assert (false);
        return ERROR_DATA_CORRUPTION;
    }

    return OK;
}


int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows) {

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppiData = NULL;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *ppiData = new int [m_pTable->m_iNumRows];
    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_pTable->m_iNumRows];
    }

    unsigned int i, iTerminatorKey = m_pTable->m_iTerminatorRowKey;

    for (i = 0; i < iTerminatorKey; i ++) {
        if (m_pTable->RowHeader (i)->Tag == VALID) {
            (*ppiData)[*piNumRows] = *((int*) m_pTable->Data (i, iColumn));
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == m_pTable->m_iNumRows);

    return OK;
}


int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, float** ppfData, 
                           unsigned int* piNumRows) {

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppfData = NULL;

    CHECK_LOAD;
    
    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_FLOAT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *ppfData = new float [m_pTable->m_iNumRows];
    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_pTable->m_iNumRows];
    }

    unsigned int i, iTerminatorKey = m_pTable->m_iTerminatorRowKey;

    for (i = 0; i < iTerminatorKey; i ++) {
        if (m_pTable->RowHeader (i)->Tag == VALID) {
            (*ppfData)[*piNumRows] = *((float*) m_pTable->Data (i, iColumn));
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == m_pTable->m_iNumRows);

    return OK;
}


int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, char*** pppszData, 
                           unsigned int* piNumRows) {
    
    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *pppszData = NULL;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_STRING) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *pppszData = new char* [m_pTable->m_iNumRows];
    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_pTable->m_iNumRows];
    }

    unsigned int i, iTerminatorKey = m_pTable->m_iTerminatorRowKey;

    for (i = 0; i < iTerminatorKey; i ++) {
        if (m_pTable->RowHeader (i)->Tag == VALID) {
            (*pppszData)[*piNumRows] = (char*) m_pTable->Data (i, iColumn);
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == m_pTable->m_iNumRows);

    return OK;
}

int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, UTCTime** pptData, 
                           unsigned int* piNumRows) {
    
    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *pptData = NULL;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_TIME) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *pptData = new UTCTime [m_pTable->m_iNumRows];
    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_pTable->m_iNumRows];
    }

    unsigned int i, iTerminatorKey = m_pTable->m_iTerminatorRowKey;

    for (i = 0; i < iTerminatorKey; i ++) {

        if (m_pTable->RowHeader (i)->Tag == VALID) {

            (*pptData)[*piNumRows] = *((UTCTime*) m_pTable->Data (i, iColumn));
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == m_pTable->m_iNumRows);

    return OK;
}

int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, int64** ppi64Data, unsigned int* piNumRows) {

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppi64Data = NULL;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_INT64) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Allocate space
    *ppi64Data = new int64 [m_pTable->m_iNumRows];
    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_pTable->m_iNumRows];
    }

    unsigned int i, iTerminatorKey = m_pTable->m_iTerminatorRowKey;

    for (i = 0; i < iTerminatorKey; i ++) {

        if (m_pTable->RowHeader (i)->Tag == VALID) {

            (*ppi64Data)[*piNumRows] = *((int64*) m_pTable->Data (i, iColumn));
            if (ppiKey != NULL) {
                (*ppiKey)[*piNumRows] = i;
            }
            (*piNumRows) ++;
        }
    }

    Assert (*piNumRows == m_pTable->m_iNumRows);

    return OK;
}

int ReadTable::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, 
                           unsigned int* piNumRows) {

    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppvData = NULL;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (iColumn >= m_pTable->m_pTemplate->TemplateData.NumColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    // Allocate space
    *ppvData = new Variant [m_pTable->m_iNumRows];
    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_pTable->m_iNumRows];
    }

    unsigned int i, iTerminatorKey = m_pTable->m_iTerminatorRowKey;

    switch (m_pTable->m_pTemplate->TemplateData.Type [iColumn]) {

    case V_INT:
        
        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_pTable->RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = *((int*) m_pTable->Data (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_TIME:

        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_pTable->RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = *((UTCTime*) m_pTable->Data (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_FLOAT:

        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_pTable->RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = *((float*) m_pTable->Data (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_STRING:

        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_pTable->RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = (const char*) m_pTable->Data (i, iColumn);
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_INT64:
        
        for (i = 0; i < iTerminatorKey; i ++) {
            if (m_pTable->RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = *((int64*) m_pTable->Data (i, iColumn));
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

    Assert (*piNumRows == m_pTable->m_iNumRows);

    return OK;
}

int ReadTable::ReadColumn (unsigned int iColumn, int** ppiData, unsigned int* piNumRows) {
    return ReadColumn (iColumn, NULL, ppiData, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, float** ppfData, unsigned int* piNumRows) {
    return ReadColumn (iColumn, NULL, ppfData, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, char*** ppszData, unsigned int* piNumRows) {
    return ReadColumn (iColumn, NULL, ppszData, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, UTCTime** pptData, unsigned int* piNumRows) {
    return ReadColumn (iColumn, NULL, pptData, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, int64** ppi64Data, unsigned int* piNumRows) {
    return ReadColumn (iColumn, NULL, ppi64Data, piNumRows);
}

int ReadTable::ReadColumn (unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows) {
    return ReadColumn (iColumn, NULL, ppvData, piNumRows);
}

int ReadTable::ReadRow (unsigned int iKey, void*** pppData) {

    *pppData = NULL;

    CHECK_LOAD;

    if (iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    unsigned int i, iNumCols = m_pTable->m_pTemplate->TemplateData.NumColumns;

    *pppData = new void* [iNumCols];

    for (i = 0; i < iNumCols; i ++) {
        (*pppData)[i] = m_pTable->Data (iKey, i);
    }

    return OK;
}


int ReadTable::ReadRow (unsigned int iKey, Variant** ppvData) {

    *ppvData = NULL;

    CHECK_LOAD;

    if (iKey >= m_pTable->m_iTerminatorRowKey || m_pTable->RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    unsigned int i, iNumCols = m_pTable->m_pTemplate->TemplateData.NumColumns;
    
    *ppvData = new Variant [iNumCols];
    
    for (i = 0; i < iNumCols; i ++) {
        
        switch (m_pTable->m_pTemplate->TemplateData.Type [i]) {
            
        case V_INT:
            
            (*ppvData)[i] = *((int*) m_pTable->Data (iKey, i));
            break;
            
        case V_TIME:
            
            (*ppvData)[i] = *((UTCTime*) m_pTable->Data (iKey, i));
            break;
            
        case V_FLOAT:
            
            (*ppvData)[i] = *((float*) m_pTable->Data (iKey, i));
            break;
            
        case V_STRING:
            
            (*ppvData)[i] = (char*) m_pTable->Data (iKey, i);
            break;

        case V_INT64:
            
            (*ppvData)[i] = *((int64*) m_pTable->Data (iKey, i));
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

    *piKey = NO_KEY;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Indexing
    unsigned int i, iNumIndices = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndices; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            return m_pTable->m_pIndex[i].GetFirstKey (iData, piKey);
        }
    }

    unsigned int iTerminatorRowKey = m_pTable->m_iTerminatorRowKey;
    for (i = 0; i < iTerminatorRowKey; i ++) {
        if (m_pTable->RowHeader (i)->Tag == VALID && *((int*) m_pTable->Data (i, iColumn)) == iData) {
            *piKey = i;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}

int ReadTable::GetFirstKey (unsigned int iColumn, int64 i64Data, unsigned int* piKey) {

    *piKey = NO_KEY;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_INT64) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Indexing
    unsigned int i, iNumIndices = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndices; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            return m_pTable->m_pIndex[i].GetFirstKey (i64Data, piKey);
        }
    }

    unsigned int iTerminatorRowKey = m_pTable->m_iTerminatorRowKey;
    for (i = 0; i < iTerminatorRowKey; i ++) {
        if (m_pTable->RowHeader (i)->Tag == VALID && *((int64*) m_pTable->Data (i, iColumn)) == i64Data) {
            *piKey = i;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}


int ReadTable::GetFirstKey (unsigned int iColumn, float fData, unsigned int* piKey) {

    *piKey = NO_KEY;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_FLOAT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Indexing
    unsigned int i, iNumIndices = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndices; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            return m_pTable->m_pIndex[i].GetFirstKey (fData, piKey);
        }
    }

    unsigned int iTerminatorRowKey = m_pTable->m_iTerminatorRowKey;
    for (i = 0; i < iTerminatorRowKey; i ++) {
        if (m_pTable->RowHeader (i)->Tag == VALID && *((float*) m_pTable->Data (i, iColumn)) == fData) {
            *piKey = i;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}

int ReadTable::GetFirstKey (unsigned int iColumn, const UTCTime& tData, unsigned int* piKey) {

    *piKey = NO_KEY;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_TIME) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Indexing
    unsigned int i, iNumIndices = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndices; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            return m_pTable->m_pIndex[i].GetFirstKey (tData, piKey);
        }
    }

    unsigned int iTerminatorRowKey = m_pTable->m_iTerminatorRowKey;
    for (i = 0; i < iTerminatorRowKey; i ++) {
        if (m_pTable->RowHeader (i)->Tag == VALID && *((UTCTime*) m_pTable->Data (i, iColumn)) == tData) {
            *piKey = i;
            return OK;
        }
    }

    return ERROR_DATA_NOT_FOUND;
}


int ReadTable::GetFirstKey (unsigned int iColumn, const char* pszData, bool bCaseInsensitive, 
                            unsigned int* piKey) {

    *piKey = NO_KEY;

    CHECK_LOAD;

    if (m_pTable->m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != V_STRING) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // Indexing
    unsigned int i, iNumIndices = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndices; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            return m_pTable->m_pIndex[i].GetFirstKey (pszData, piKey);
        }
    }

    unsigned int iTerminatorRowKey = m_pTable->m_iTerminatorRowKey;

    if (bCaseInsensitive) {

        for (i = 0; i < iTerminatorRowKey; i ++) {
            if (m_pTable->RowHeader (i)->Tag == VALID &&
                String::StriCmp (pszData, (char*) m_pTable->Data (i, iColumn)) == 0) {
                *piKey = i;
                return OK;
            }
        }
    
    } else {

        for (i = 0; i < iTerminatorRowKey; i ++) {
            if (m_pTable->RowHeader (i)->Tag == VALID &&
                String::StrCmp (pszData, (char*) m_pTable->Data (i, iColumn)) == 0) {
                *piKey = i;
                return OK;
            }
        }
    }

    return ERROR_DATA_NOT_FOUND;
}

int ReadTable::GetFirstKey (unsigned int iColumn, const Variant& tData, bool bCaseInsensitive, 
                            unsigned int* piKey) {

    *piKey = NO_KEY;

    CHECK_LOAD;

    VariantType vtType = tData.GetType();

    if (m_pTable->m_pTemplate->TemplateData.Type [iColumn] != vtType) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    switch (vtType) {

    case V_INT:

        return GetFirstKey (iColumn, tData.GetInteger(), piKey);
        break;

    case V_TIME:

        return GetFirstKey (iColumn, tData.GetUTCTime(), piKey);
        break;

    case V_FLOAT:

        return GetFirstKey (iColumn, tData.GetFloat(), piKey);
        break;

    case V_STRING:

        return GetFirstKey (iColumn, tData.GetCharPtr(), bCaseInsensitive, piKey);
        break;

    case V_INT64:

        return GetFirstKey (iColumn, tData.GetInteger64(), piKey);
        break;

    default:

        Assert (false);
        *piKey = NO_KEY;
        return ERROR_DATA_CORRUPTION;
    }
}

int ReadTable::GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys) {

    return m_pTable->GetAllKeys (ppiKey, piNumKeys);
}

int ReadTable::GetNextKey (unsigned int iKey, unsigned int* piNextKey) {

    return m_pTable->GetNextKey (iKey, piNextKey);
}

int ReadTable::GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, 
                             unsigned int** ppiKey, unsigned int* piNumKeys) {

    return m_pTable->GetEqualKeys (iColumn, vData, bCaseInsensitive, ppiKey, piNumKeys);
}