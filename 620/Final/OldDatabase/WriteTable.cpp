// WriteTable.cpp: implementation of the WriteTable class.
//
//////////////////////////////////////////////////////////////////////

#define DATABASE_BUILD
#include "WriteTable.h"
#undef DATABASE_BUILD

WriteTable::WriteTable (Table* pTable) : ReadTable (pTable) {
    m_bWriteTable = true;
}

unsigned int WriteTable::AddRef() {

    return Algorithm::AtomicIncrement (&m_iRefCount);
}

unsigned int WriteTable::Release() {

    int iRefCount = Algorithm::AtomicDecrement (&m_iRefCount);
    if (iRefCount == 0) {
        delete this;
    }

    return iRefCount;
}

int WriteTable::QueryInterface (const Uuid& iidInterface, void** ppInterface) {

    if (iidInterface == IID_IWriteTable) {
        *ppInterface = static_cast<IWriteTable*> (this);
        AddRef();
        return OK;
    }

    if (iidInterface == IID_IReadTable) {
        *ppInterface = static_cast<IReadTable*> (this);
        AddRef();
        return OK;
    }

    *ppInterface = NULL;
    return ERROR_NO_INTERFACE;
}

WriteTable* WriteTable::CreateInstance (Table* pTable) {

    return new WriteTable (pTable);
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, int iData) {

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

    // Get data
    void* pData = m_pTable->Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY, iNumIndexCols = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndexCols; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pTable->m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    *((int*) pData) = iData;

    if (iColumnIndex != NO_KEY) {
        iErrCode = m_pTable->m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, float fData) {

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

    // Get data
    void* pData = m_pTable->Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY, iNumIndexCols = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndexCols; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pTable->m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    *((float*) pData) = fData;

    if (iColumnIndex != NO_KEY) {
        iErrCode = m_pTable->m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, const char* pszData) {

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

    // Get data
    void* pData = m_pTable->Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY, iNumIndexCols = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndexCols; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pTable->m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }
    
    size_t stSize = m_pTable->m_pTemplate->TemplateData.Size [iColumn];
    size_t stNewLength = String::StrLen (pszData);
    
    if (
        (
        stSize == INITIAL_INSERTION_LENGTH && 
        ALIGN (stNewLength + sizeof (char), MAX_ELEMENT_SIZE) + sizeof (size_t) > 
        *(size_t*) ((char*) pData - sizeof (size_t))
        )
        ||
        stNewLength >= stSize) {

        return ERROR_STRING_IS_TOO_LONG;
        
    }

    strncpy ((char*) pData, pszData, stNewLength + 1);

    if (iColumnIndex != NO_KEY) {
        iErrCode = m_pTable->m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, const UTCTime& tData) {

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

    // Get data
    void* pData = m_pTable->Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY, iNumIndexCols = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndexCols; i ++) {
        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pTable->m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    *((UTCTime*) pData) = tData;

    if (iColumnIndex != NO_KEY) {
        iErrCode = m_pTable->m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, int64 i64Data) {

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

    // Get data
    void* pData = m_pTable->Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY, iNumIndexCols = m_pTable->m_pTemplate->TemplateData.NumIndexes;
    for (i = 0; i < iNumIndexCols; i ++) {

        if (m_pTable->m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pTable->m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    *((int64*) pData) = i64Data;

    if (iColumnIndex != NO_KEY) {

        iErrCode = m_pTable->m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, const Variant& vData) {

    CHECK_LOAD;

    switch (vData.GetType()) {

    case V_INT:

        return WriteData (iKey, iColumn, vData.GetInteger());

    case V_TIME:

        return WriteData (iKey, iColumn, vData.GetUTCTime());

    case V_FLOAT:

        return WriteData (iKey, iColumn, vData.GetFloat());

    case V_STRING:

        return WriteData (iKey, iColumn, vData.GetCharPtr());

    default:

        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }
}

int WriteTable::WriteData (unsigned int iColumn, int iData) {
    return WriteData (0, iColumn, iData);
}

int WriteTable::WriteData (unsigned int iColumn, float fData) {
    return WriteData (0, iColumn, fData);
}

int WriteTable::WriteData (unsigned int iColumn, const char* pszData) {
    return WriteData (0, iColumn, pszData);
}

int WriteTable::WriteData (unsigned int iColumn, const UTCTime& tData) {
    return WriteData (0, iColumn, tData);
}

int WriteTable::WriteData (unsigned int iColumn, int64 i64Data) {
    return WriteData (0, iColumn, i64Data);
}

int WriteTable::WriteData (unsigned int iColumn, const Variant& vData) {
    return WriteData (0, iColumn, vData);
}

int WriteTable::WriteAnd (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {
    return m_pTable->WriteAnd (iKey, iColumn, iBitField);
}

int WriteTable::WriteAnd (unsigned int iColumn, unsigned int iBitField) {
    return m_pTable->WriteAnd (0, iColumn, iBitField);
}

int WriteTable::WriteOr (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {
    return m_pTable->WriteOr (iKey, iColumn, iBitField);
}

int WriteTable::WriteOr (unsigned int iColumn, unsigned int iBitField) {
    return m_pTable->WriteOr (0, iColumn, iBitField);
}

int WriteTable::WriteXor (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {
    return m_pTable->WriteXor (iKey, iColumn, iBitField);
}

int WriteTable::WriteXor (unsigned int iColumn, unsigned int iBitField) {
    return m_pTable->WriteXor (0, iColumn, iBitField);
}

int WriteTable::WriteNot (unsigned int iKey, unsigned int iColumn) {
    return m_pTable->WriteNot (iKey, iColumn);
}

int WriteTable::WriteNot (unsigned int iColumn) {
    return m_pTable->WriteNot (0, iColumn);
}

int WriteTable::WriteColumn (unsigned int iColumn, int iData) {
    return m_pTable->WriteColumn (iColumn, iData);
}

int WriteTable::WriteColumn (unsigned int iColumn, float fData) {
    return m_pTable->WriteColumn (iColumn, fData);
}

int WriteTable::WriteColumn (unsigned int iColumn, const char* pszData) {
    return m_pTable->WriteColumn (iColumn, pszData);
}

int WriteTable::WriteColumn (unsigned int iColumn, const UTCTime& tData) {
    return m_pTable->WriteColumn (iColumn, tData);
}

int WriteTable::WriteColumn (unsigned int iColumn, int64 i64Data) {
    return m_pTable->WriteColumn (iColumn, i64Data);
}

int WriteTable::WriteColumn (unsigned int iColumn, const Variant& vData) {
    return m_pTable->WriteColumn (iColumn, vData);
}

int WriteTable::InsertRow (Variant* pvColVal, unsigned int* piKey) {
    return m_pTable->InsertRow (pvColVal, piKey);
}

int WriteTable::InsertRow (Variant* pvColVal) {
    unsigned int iKey;
    return m_pTable->InsertRow (pvColVal, &iKey);
}

int WriteTable::InsertRows (Variant* pvColVal, unsigned int iNumRows) {
    return m_pTable->InsertRows (pvColVal, iNumRows);
}

int WriteTable::InsertDuplicateRows (Variant* pvColVal, unsigned int iNumRows) {
    return m_pTable->InsertDuplicateRows (pvColVal, iNumRows);
}

int WriteTable::DeleteRow (unsigned int iKey) {
    return m_pTable->DeleteRow (iKey);
}

int WriteTable::DeleteAllRows() {
    return m_pTable->DeleteAllRows();
}