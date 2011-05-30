// WriteTable.cpp: implementation of the WriteTable class.
//
//////////////////////////////////////////////////////////////////////

#define DATABASE_BUILD
#include "Table.h"
#undef DATABASE_BUILD


WriteTable::WriteTable() {

    m_pTable = NULL;
    // Uses reference count from contained ReadTable

    m_pCtx = m_rTable.GetContext();
    Assert (m_pCtx != NULL);
}

WriteTable::~WriteTable() {}

unsigned int WriteTable::AddRef() {

    unsigned int iNumRefs = m_rTable.AddRefInternal();
    if (iNumRefs == 2) {
        m_pCtx->GetDatabase()->GetGlobalLock()->WaitReader();
        m_pTable->WaitWriter();
    }

    return iNumRefs;
}

unsigned int WriteTable::Release() {

    unsigned int iNumRefs = m_rTable.ReleaseInternal();
    if (iNumRefs == 1) {
        m_pTable->SignalWriter();
        m_pCtx->GetDatabase()->GetGlobalLock()->SignalReader();
    }

    return iNumRefs;
}

int WriteTable::QueryInterface (const Uuid& iidInterface, void** ppInterface) {

    if (iidInterface == IID_IObject) {
        *ppInterface = (void*) static_cast<IObject*> (this);
        AddRef();
        return OK;
    }

    if (iidInterface == IID_IReadTable) {
        *ppInterface = (void*) static_cast<IReadTable*> (static_cast<IWriteTable*> (this));
        AddRef();
        return OK;
    }

    if (iidInterface == IID_IWriteTable) {
        *ppInterface = (void*) static_cast<IWriteTable*> (this);
        AddRef();
        return OK;
    }

    return ERROR_NO_INTERFACE;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, int iData) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    int* piData = (int*) m_pCtx->GetData (iKey, iColumn);
    if (*piData == iData) {
        return OK;
    }

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, iData);
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    *piData = iData;

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, float fData) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_FLOAT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    float* pfData = (float*) m_pCtx->GetData (iKey, iColumn);
    if (*pfData == fData) {
        return OK;
    }

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, fData);
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    *pfData = fData;

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, const char* pszData) {

    int iErrCode;
    Offset oFreeOffset = NO_OFFSET, oAllocatedOffset = NO_OFFSET;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_STRING) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    // TODOTODO - deadlock check
#ifdef _DEBUG
    m_pCtx->Unlock();
    m_pCtx->Freeze();
    m_pCtx->Unfreeze();
    m_pCtx->Lock();
#endif

    // Get length of string
    size_t stNewLength = String::StrLen (pszData) + 1;

    // Get address of data
    void* pData = m_pCtx->GetData (iKey, iColumn);
    Offset* poOrigData = NULL;

    // Check length
    size_t stSize = m_pCtx->GetColumnSize (iColumn);
    if (stSize == VARIABLE_LENGTH_STRING) {

        // Get offset into varlen data heap
        Offset oOffset = *(Offset*) pData;

        if (oOffset == NO_OFFSET) {
            
            // Allocate some new space for us
            if (stNewLength > 1) {

                oOffset = m_pCtx->AllocateVarLen (stNewLength);
                if (oOffset == NO_OFFSET) {
                    return ERROR_OUT_OF_DISK_SPACE;
                }

                oAllocatedOffset = oOffset;
                poOrigData = (Offset*) pData;
            }

        } else {

            if (stNewLength > 1) {

                // Just reallocate and let the heap optimize
                oOffset = m_pCtx->ReallocateVarLen (oOffset, stNewLength);
                if (oOffset == NO_OFFSET) {
                    return ERROR_OUT_OF_DISK_SPACE;
                }

                *(Offset*) pData = oOffset;

            } else {

                // Free the offset
                oFreeOffset = oOffset;
                poOrigData = (Offset*) pData;

                oOffset = NO_OFFSET;
            }
        }

        if (oOffset == NO_OFFSET) {
            pData = NULL;
        } else {
            pData = m_pCtx->GetAddressVarLen (oOffset);
            Assert (pData != NULL);
        }

    } else {

        if (stNewLength > stSize) {
            return ERROR_STRING_IS_TOO_LONG;
        }
    }

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, pszData);
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {

        Assert (iErrCode == ERROR_DUPLICATE_DATA);

        if (stSize == VARIABLE_LENGTH_STRING) {

            // Restore the old offset if necessary
            if (oFreeOffset != NO_OFFSET) {
                *(Offset*) m_pCtx->GetData (iKey, iColumn) = oFreeOffset;
            }

            // Free the new empty offset if necessary
            if (oAllocatedOffset != NO_OFFSET) {
                m_pCtx->FreeVarLen (oAllocatedOffset);
                *(Offset*) m_pCtx->GetData (iKey, iColumn) = NO_OFFSET;
            }
        }

        return iErrCode;
    }

    // Finally, copy the string
    if (pData != NULL) {
        String::StrnCpy ((char*) pData, pszData, stNewLength);
    }

    // Set new offset if necessary
    if (poOrigData != NULL) {
        *poOrigData = oAllocatedOffset;
    }

    // Free old offset if necessary
    if (oFreeOffset != NO_OFFSET) {
        m_pCtx->FreeVarLen (oFreeOffset);
    }

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, const UTCTime& tData) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_TIME) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    UTCTime* ptData = (UTCTime*) m_pCtx->GetData (iKey, iColumn);
    if (*ptData == tData) {
        return OK;
    }

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, tData);
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    *ptData = tData;

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, int64 i64Data) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT64) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    int64* pi64Data = (int64*) m_pCtx->GetData (iKey, iColumn);
    if (*pi64Data == i64Data) {
        return OK;
    }

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, i64Data);
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    *pi64Data = i64Data;

    return OK;
}

int WriteTable::WriteData (unsigned int iKey, unsigned int iColumn, const Variant& vData) {

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    switch (vData.GetType()) {

    case V_INT:

        return WriteData (iKey, iColumn, vData.GetInteger());

    case V_INT64:

        return WriteData (iKey, iColumn, vData.GetInteger64());

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

    Assert (m_pCtx->IsOneRow());
    return WriteData (0, iColumn, iData);
}

int WriteTable::WriteData (unsigned int iColumn, float fData) {

    Assert (m_pCtx->IsOneRow());
    return WriteData (0, iColumn, fData);
}

int WriteTable::WriteData (unsigned int iColumn, const char* pszData) {

    Assert (m_pCtx->IsOneRow());
    return WriteData (0, iColumn, pszData);
}

int WriteTable::WriteData (unsigned int iColumn, const UTCTime& tData) {

    Assert (m_pCtx->IsOneRow());
    return WriteData (0, iColumn, tData);
}

int WriteTable::WriteData (unsigned int iColumn, int64 i64Data) {

    Assert (m_pCtx->IsOneRow());
    return WriteData (0, iColumn, i64Data);
}

int WriteTable::WriteData (unsigned int iColumn, const Variant& vData) {

    Assert (m_pCtx->IsOneRow());
    return WriteData (0, iColumn, vData);
}

int WriteTable::WriteAnd (unsigned int iColumn, unsigned int iBitField) {

    Assert (m_pCtx->IsOneRow());
    return WriteAnd (0, iColumn, iBitField);
}

int WriteTable::WriteOr (unsigned int iColumn, unsigned int iBitField) {

    Assert (m_pCtx->IsOneRow());
    return WriteOr (0, iColumn, iBitField);
}

int WriteTable::WriteXor (unsigned int iColumn, unsigned int iBitField) {

    Assert (m_pCtx->IsOneRow());
    return WriteXor (0, iColumn, iBitField);
}

int WriteTable::WriteNot (unsigned int iColumn) {

    return WriteNot (0, iColumn);
}

int WriteTable::WriteColumn (unsigned int iColumn, int iData) {

    int iErrCode;

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_pCtx->IndexWriteColumn (iColumn, iData);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_COLUMN_NOT_INDEXED) {
            iErrCode = OK;
        } else {
            Assert (false);
            return iErrCode;
        }
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {
        
        iRowKey = m_pCtx->FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }
        
        // Set data
        *(int*) m_pCtx->GetData (iRowKey, iColumn) = iData;
    }

    return iErrCode;
}


int WriteTable::WriteColumn (unsigned int iColumn, float fData) {

    int iErrCode;

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_pCtx->IndexWriteColumn (iColumn, fData);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_COLUMN_NOT_INDEXED) {
            iErrCode = OK;
        } else {
            Assert (false);
            return iErrCode;
        }
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {
        
        iRowKey = m_pCtx->FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }
        
        // Set data
        *(float*) m_pCtx->GetData (iRowKey, iColumn) = fData;
    }

    return iErrCode;
}

int WriteTable::WriteColumn (unsigned int iColumn, const char* pszData) {

    int iErrCode = OK;

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    if (m_pCtx->GetNumRows() == 0) {
        Assert (false);
        return ERROR_DATA_NOT_FOUND;
    }

    // TODOTODO - deadlock check
#ifdef _DEBUG
    m_pCtx->Unlock();
    m_pCtx->Freeze();
    m_pCtx->Unfreeze();
    m_pCtx->Lock();
#endif

    // Check length
    size_t stSize = m_pCtx->GetColumnSize (iColumn);
    size_t stNewLength = String::StrLen (pszData);
    
    if (stSize != VARIABLE_LENGTH_STRING && stNewLength >= stSize) {
        Assert (false);
        return ERROR_STRING_IS_TOO_LONG;
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {
        
        iRowKey = m_pCtx->FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }
        
        // Just re-use writedata logic - slightly slower, but less code
        iErrCode = WriteData (iRowKey, iColumn, pszData);
        if (iErrCode != OK) {
            Assert (false);
            break;
        }

        // Can't do much if we fail...
    }

    return iErrCode;
}

int WriteTable::WriteColumn (unsigned int iColumn, const UTCTime& tData) {

    int iErrCode;

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_pCtx->IndexWriteColumn (iColumn, tData);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_COLUMN_NOT_INDEXED) {
            iErrCode = OK;
        } else {
            Assert (false);
            return iErrCode;
        }
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {
        
        iRowKey = m_pCtx->FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }
        
        // Set data
        *(UTCTime*) m_pCtx->GetData (iRowKey, iColumn) = tData;
    }

    return iErrCode;
}

int WriteTable::WriteColumn (unsigned int iColumn, int64 i64Data) {

    int iErrCode;

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    iErrCode = m_pCtx->IndexWriteColumn (iColumn, i64Data);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_COLUMN_NOT_INDEXED) {
            iErrCode = OK;
        } else {
            Assert (false);
            return iErrCode;
        }
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {
        
        iRowKey = m_pCtx->FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }
        
        // Set data
        *(int64*) m_pCtx->GetData (iRowKey, iColumn) = i64Data;
    }

    return iErrCode;
}


int WriteTable::WriteColumn (unsigned int iColumn, const Variant& vData) {

    switch (vData.GetType()) {

    case V_INT:

        return WriteColumn (iColumn, vData.GetInteger());

    case V_INT64:

        return WriteColumn (iColumn, vData.GetInteger64());

    case V_TIME:

        return WriteColumn (iColumn, vData.GetUTCTime());

    case V_FLOAT:

        return WriteColumn (iColumn, vData.GetFloat());

    case V_STRING:

        return WriteData (iColumn, vData.GetCharPtr());

    default:

        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }
}

int WriteTable::Increment (unsigned int iColumn, const Variant& vIncrement) {

    Assert (m_pCtx->IsOneRow());
    return Increment (0, iColumn, vIncrement, NULL);
}

int WriteTable::Increment (unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue) {

    Assert (m_pCtx->IsOneRow());
    return Increment (0, iColumn, vIncrement, pvOldValue);
}

int WriteTable::Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement) {

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());
    return Increment (iKey, iColumn, vIncrement, NULL);
}

int WriteTable::Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    VariantType vtType = m_pCtx->GetColumnType (iColumn);

    switch (vtType) {

    case V_INT:

        if (vIncrement.GetType() != V_INT) {
            Assert (false);
            return ERROR_TYPE_MISMATCH;
        }

        if (vIncrement.GetInteger() != 0) {

            int iOldValue;

            iErrCode = ReadData (iKey, iColumn, &iOldValue);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            if (pvOldValue != NULL) {
                *pvOldValue = iOldValue;
            }

            return WriteData (iKey, iColumn, iOldValue + vIncrement.GetInteger());
        }
        break;

    case V_FLOAT:

        if (vIncrement.GetType() != V_FLOAT) {
            Assert (false);
            return ERROR_TYPE_MISMATCH;
        }

        if (vIncrement.GetFloat() != 0) {

            float fOldValue;

            iErrCode = ReadData (iKey, iColumn, &fOldValue);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            if (pvOldValue != NULL) {
                *pvOldValue = fOldValue;
            }

            return WriteData (iKey, iColumn, fOldValue + vIncrement.GetFloat());
        }
        break;

    case V_TIME:

        if (vIncrement.GetType() == V_INT) {

            UTCTime tFinal;
            Time::AddSeconds (pvOldValue->GetUTCTime(), vIncrement.GetInteger(), &tFinal);

            return WriteData (iKey, iColumn, tFinal);
        }

        if (vIncrement.GetType() == V_TIME) {
            return WriteData (iKey, iColumn, pvOldValue->GetUTCTime() + vIncrement.GetUTCTime());
        }

        Assert (false);
        return ERROR_TYPE_MISMATCH;

    case V_INT64:
        {
            int64 i64Increment;

            if (vIncrement.GetType() == V_INT) {
                i64Increment = vIncrement.GetInteger();
            }

            else if (vIncrement.GetType() == V_INT64) {
                i64Increment = vIncrement.GetInteger64();
            }

            else {
                Assert (false);
                return ERROR_TYPE_MISMATCH;
            }

            if (i64Increment != 0) {

                int64 i64OldValue;

                iErrCode = ReadData (iKey, iColumn, &i64OldValue);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }

                if (pvOldValue != NULL) {
                    *pvOldValue = i64OldValue;
                }

                return WriteData (iKey, iColumn, i64OldValue + i64Increment);
            }
        }
        break;

    case V_STRING:

        // TODOTODO - deadlock check
#ifdef _DEBUG
        m_pCtx->Unlock();
        m_pCtx->Freeze();
        m_pCtx->Unfreeze();
        m_pCtx->Lock();
#endif

        if (vIncrement.GetType() != V_STRING) {
            Assert (false);
            return ERROR_TYPE_MISMATCH;
        } else {

            size_t stNewLen = String::StrLen (vIncrement.GetCharPtr()) + 1;
            if (stNewLen > 1) {

                size_t stSize = m_pCtx->GetColumnSize (iColumn);
                char* pszData = (char*) m_pCtx->GetData (iKey, iColumn);

                if (stSize == VARIABLE_LENGTH_STRING) {

                    Offset oOffset = *(Offset*) pszData;
                    if (oOffset == NO_OFFSET) {
                        pszData = "";
                    } else {
                        pszData = (char*) m_pCtx->GetAddressVarLen (oOffset);
                    }
                }

                size_t stOldLen = String::StrLen (pszData);

                char* pszAppend = new char [stOldLen + stNewLen];
                if (pszAppend == NULL) {
                    return ERROR_OUT_OF_MEMORY;
                }
                
                memcpy (pszAppend, pszData, stOldLen + 1);
                memcpy (pszAppend + stOldLen, vIncrement.GetCharPtr(), stNewLen);

                iErrCode = WriteData (iKey, iColumn, pszAppend);

                delete [] pszAppend;
                return iErrCode;
            }
        }

    default:

        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    if (pvOldValue != NULL) {
        return ReadData (iKey, iColumn, pvOldValue);
    }

    return OK;
}


int WriteTable::WriteAnd (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    int* piData = (int*) m_pCtx->GetData (iKey, iColumn);

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, (int) ((*piData) & iBitField));
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    *piData &= iBitField;

    return OK;
}


int WriteTable::WriteOr (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    int* piData = (int*) m_pCtx->GetData (iKey, iColumn);

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, (int) ((*piData) | iBitField));
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    *piData |= iBitField;

    return OK;
}

int WriteTable::WriteXor (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    int* piData = (int*) m_pCtx->GetData (iKey, iColumn);

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, (int) ((*piData) ^ iBitField));
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    *piData ^= iBitField;

    return OK;
}

int WriteTable::WriteNot (unsigned int iKey, unsigned int iColumn) {

    int iErrCode;

    Assert ((m_pCtx->IsOneRow() && iKey == 0) || !m_pCtx->IsOneRow());

    if (!m_rTable.IsValidKey (iKey)) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (!m_rTable.IsValidColumn (iColumn)) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_INDEX;
    }

    if (m_pCtx->GetColumnType (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    int* piData = (int*) m_pCtx->GetData (iKey, iColumn);

    iErrCode = m_pCtx->IndexWriteData (iKey, iColumn, ~(*piData));
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    *piData = ~(*piData);

    return OK;
}

int WriteTable::InsertRow (const Variant* pvColVal, unsigned int* piKey) {
    return InsertRow (pvColVal, NO_KEY, piKey);
}

int WriteTable::InsertRow (const Variant* pvColVal, unsigned int iKey) {
    return InsertRow (pvColVal, iKey, NULL);
}

int WriteTable::InsertRow (const Variant* pvColVal, unsigned int iKey, unsigned int* piKey) {

    if (pvColVal == NULL) {
        Assert (false);
        return ERROR_INVALID_ARGUMENT;
    }

    Assert (!m_pCtx->IsOneRow() || m_pCtx->GetNumRows() == 0);

    if (piKey != NULL) {
        *piKey = NO_KEY;
    }

    int iErrCode;
    unsigned int i, iNumCols = m_pCtx->GetNumColumns(), iLastColWritten = NO_KEY;

    // TODOTODO - deadlock check
#ifdef _DEBUG
    m_pCtx->Unlock();
    m_pCtx->Freeze();
    m_pCtx->Unfreeze();
    m_pCtx->Lock();
#endif

    // Get row size
    size_t stRowSize = m_pCtx->GetRowSize();

    // Get terminator key
    unsigned int iTerminatorRowKey = m_pCtx->GetTerminatorRowKey();

    // Select key
    if (iKey == NO_KEY) {

        iKey = m_pCtx->FindFirstInvalidRow();
        if (iKey == NO_KEY) {
            iKey = iTerminatorRowKey;
        }
        else Assert (iKey < iTerminatorRowKey);
    }

    unsigned int iExpandRows = 1;
    if (iKey >= iTerminatorRowKey) {
        iExpandRows = iKey - iTerminatorRowKey + 1;
    }

    iErrCode = m_pCtx->ExpandMetaDataIfNecessary (max (iKey + 1, iTerminatorRowKey));
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (m_pCtx->GetRowOffset (iKey) + stRowSize > m_pCtx->GetEndOfTable()) {
     
        Assert (iKey >= iTerminatorRowKey);

        // Not enough room for new row - resize table space
        iErrCode = m_pCtx->Resize (iExpandRows);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    Assert (m_pCtx->GetRowOffset (iKey) + stRowSize <= m_pCtx->GetEndOfTable());

    iErrCode = m_pCtx->IndexInsertRow (iKey, pvColVal);
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (iErrCode == ERROR_DUPLICATE_DATA);
        return iErrCode;
    }

    for (i = 0; i < iNumCols; i ++) {

        switch (pvColVal[i].GetType()) {

        case V_INT:

            if (m_pCtx->GetColumnType (i) != V_INT) {
                Assert (false);
                iErrCode = ERROR_TYPE_MISMATCH;
                goto OnError;
            }

            *((int*) m_pCtx->GetData (iKey, i)) = pvColVal[i].GetInteger();
            break;

        case V_FLOAT:

            if (m_pCtx->GetColumnType (i) != V_FLOAT) {
                Assert (false);
                iErrCode = ERROR_TYPE_MISMATCH;
                goto OnError;
            }

            *((float*) m_pCtx->GetData (iKey, i)) = pvColVal[i].GetFloat();
            break;

        case V_TIME:

            if (m_pCtx->GetColumnType (i) != V_TIME) {
                Assert (false);
                iErrCode = ERROR_TYPE_MISMATCH;
                goto OnError;
            }

            *((UTCTime*) m_pCtx->GetData (iKey, i)) = pvColVal[i].GetUTCTime();
            break;

        case V_STRING:

            {

            void* pData;

            if (m_pCtx->GetColumnType (i) != V_STRING) {
                Assert (false);
                iErrCode = ERROR_TYPE_MISMATCH;
                goto OnError;
            }

            size_t stColSize = m_pCtx->GetColumnSize (i);
            size_t stNewSize = String::StrLen (pvColVal[i].GetCharPtr()) + 1;

            if (stColSize == VARIABLE_LENGTH_STRING) {

                Offset oOffset;

                if (stNewSize == 1) {
                    oOffset = NO_OFFSET;
                } else {

                    oOffset = m_pCtx->AllocateVarLen (stNewSize);
                    if (oOffset == NO_OFFSET) {
                        Assert (false);
                        iErrCode = ERROR_OUT_OF_DISK_SPACE;
                        goto OnError;
                    }
                }

                pData = m_pCtx->GetData (iKey, i);
                *(Offset*) pData = oOffset;

                if (oOffset == NO_OFFSET) {
                    pData = NULL;
                } else {
                    pData = m_pCtx->GetAddressVarLen (oOffset);
                    Assert (pData != NULL);
                }

            } else {

                if (stNewSize > stColSize) {
                    iErrCode = ERROR_STRING_IS_TOO_LONG;
                    goto OnError;
                }

                pData = m_pCtx->GetData (iKey, i);
                Assert (pData != NULL);
            }

            if (pData != NULL) {
                String::StrnCpy ((char*) pData, pvColVal[i].GetCharPtr(), stNewSize);
            }

            }
            break;

        case V_INT64:

            if (m_pCtx->GetColumnType (i) != V_INT64) {
                Assert (false);
                iErrCode = ERROR_TYPE_MISMATCH;
                goto OnError;
            }

            *((int64*) m_pCtx->GetData (iKey, i)) = pvColVal[i].GetInteger64();
            break;
        
        default:

            Assert (false);
            iErrCode = ERROR_DATA_CORRUPTION;
            goto OnError;
        }

        iLastColWritten = i;
    }

    m_pCtx->IncrementNumRows (1);

    if (iKey >= iTerminatorRowKey) {
        m_pCtx->IncrementTerminatorRowKey (iExpandRows);
    }
    Assert (iKey < m_pCtx->GetTerminatorRowKey());

    m_pCtx->SetValidRow (iKey);

#ifdef _DEBUG
    m_pTable->CheckIntegrity();
#endif

    if (piKey != NULL) {
        *piKey = iKey;
    }

    return OK;

OnError:

    int iIgnoreErrorCode = m_pCtx->IndexDeleteRow (iKey, pvColVal);
    Assert (iIgnoreErrorCode == OK || iIgnoreErrorCode == ERROR_COLUMN_NOT_INDEXED);

    if (m_pCtx->HasVariableLengthData() && iLastColWritten != NO_KEY) {

        // Free allocated blocks
        for (i = 0; i <= iLastColWritten; i ++) {
            
            if (m_pCtx->IsVariableLengthString (i)) {
                
                Offset oOffset = *(Offset*) m_pCtx->GetData (iKey, i);
                if (oOffset != NO_OFFSET) {
                    m_pCtx->FreeVarLen (oOffset);
                }
            }
        }
    }

#ifdef _DEBUG
    m_pTable->CheckIntegrity();
#endif

    return iErrCode;
}

int WriteTable::InsertRows (const Variant* pvColVal, unsigned int iNumRows) {
    
    int iErrCode;

    if (iNumRows == 0) {
        Assert (false);
        return ERROR_DATA_NOT_FOUND;
    }

    Assert ((m_pCtx->IsOneRow() && m_pCtx->GetNumRows() == 0 && iNumRows == 1) || !m_pCtx->IsOneRow());

    unsigned int i;
    unsigned int* piKey = (unsigned int*) StackAlloc (iNumRows * sizeof (unsigned int));
    unsigned int iNumCols = m_pCtx->GetNumColumns();

    for (i = 0; i < iNumRows; i ++) {

        iErrCode = InsertRow (pvColVal + i * iNumCols, NO_KEY, piKey + i);
        if (iErrCode != OK) {

            Assert (false);

            for (unsigned int j = 0; j < i; j ++) {
                int iErrCode2 = DeleteRow (piKey[j]);   // Best effort
                Assert (iErrCode2 == OK);
            }

            return iErrCode;
        }
    }

    return OK;
}

int WriteTable::InsertDuplicateRows (const Variant* pvColVal, unsigned int iNumRows) {

    int iErrCode = OK;

    if (iNumRows == 0 || pvColVal == NULL) {
        Assert (false);
        return ERROR_INVALID_ARGUMENT;
    }

    // TODOTODO - deadlock check
#ifdef _DEBUG
    m_pCtx->Unlock();
    m_pCtx->Freeze();
    m_pCtx->Unfreeze();
    m_pCtx->Lock();
#endif

    Assert ((m_pCtx->IsOneRow() && m_pCtx->GetNumRows() == 0 && iNumRows == 1) || !m_pCtx->IsOneRow());

    unsigned int i, j, k, iNumCols = m_pCtx->GetNumColumns(), iLastCol = 0;
    unsigned int iNewTerminatorKey = m_pCtx->GetTerminatorRowKey();

    // Check types, lengths
    for (i = 0; i < iNumCols; i ++) {

        if (pvColVal[i].GetType() != m_pCtx->GetColumnType (i)) {
            Assert (false);
            return ERROR_TYPE_MISMATCH;
        }

        if (m_pCtx->GetColumnType (i) == V_STRING) {

            size_t stSize = m_pCtx->GetColumnSize (i);
            if (stSize != VARIABLE_LENGTH_STRING && String::StrLen (pvColVal[i].GetCharPtr()) >= stSize) {
                Assert (false);
                return ERROR_STRING_IS_TOO_LONG;
            }
        }
    }

    iErrCode = m_pCtx->ExpandMetaDataIfNecessary (iNewTerminatorKey + iNumRows);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Get keys for insertions
    unsigned int* piNewKey = (unsigned int*) StackAlloc (iNumRows * sizeof (unsigned int));

    size_t stRowSize = m_pCtx->GetRowSize();
    for (i = 0; i < iNumRows; i ++) {

        piNewKey[i] = m_pCtx->FindNextInvalidRow (i == 0 ? NO_KEY : piNewKey[i - 1]);
        if (piNewKey[i] == NO_KEY) {

            for (j = i; j < iNumRows; j ++) {
                piNewKey[j] = iNewTerminatorKey ++;
            }
            break;
        }
    }

    // Check for resize
    if (m_pCtx->GetRowOffset (iNewTerminatorKey) + stRowSize > m_pCtx->GetEndOfTable()) {

        iErrCode = m_pCtx->Resize (iNumRows);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    // Write new row data
    for (i = 0; i < iNumCols; i ++) {

        switch (pvColVal[i].GetType()) {

        case V_INT:

            for (j = 0; j < iNumRows; j ++) {
                *((int*) m_pCtx->GetData (piNewKey[j], i)) = pvColVal[i].GetInteger();
            }

            break;

        case V_FLOAT:

            for (j = 0; j < iNumRows; j ++) {
                *((float*) m_pCtx->GetData (piNewKey[j], i)) = pvColVal[i].GetFloat();
            }
            break;

        case V_TIME:

            for (j = 0; j < iNumRows; j ++) {
                *((UTCTime*) m_pCtx->GetData (piNewKey[j], i)) = pvColVal[i].GetUTCTime();
            }
            break;

        case V_STRING:
            {

            size_t stColLen = m_pCtx->GetColumnSize (i);
            size_t stNewLen = String::StrLen (pvColVal[i].GetCharPtr()) + 1;

            if (stColLen == VARIABLE_LENGTH_STRING) {

                if (stNewLen == 1) {

                    for (j = 0; j < iNumRows; j ++) {
                        *(Offset*) m_pCtx->GetData (piNewKey[j], i) = NO_OFFSET;
                    }

                } else {

                    // Allocate space for each row in the column
                    for (j = 0; j < iNumRows; j ++) {
                        
                        Offset oOffset = m_pCtx->AllocateVarLen (stNewLen);
                        if (oOffset == NO_OFFSET) {

                            // Clean up the column
                            for (k = 0; k < i; k ++) {

                                m_pCtx->FreeVarLen (
                                    *(Offset*) m_pCtx->GetData (piNewKey[k], i)
                                    );
                            }

                            Assert (false);
                            iErrCode = ERROR_OUT_OF_DISK_SPACE;
                            break;

                        } else {

                            *(Offset*) m_pCtx->GetData (piNewKey[j], i) = oOffset;

                            String::StrnCpy (
                                (char*) m_pCtx->GetAddressVarLen (oOffset),
                                pvColVal[i].GetCharPtr(),
                                stNewLen
                                );
                        }
                    }
                }

            } else {
            
                for (j = 0; j < iNumRows; j ++) {

                    String::StrnCpy (
                        (char*) m_pCtx->GetData (piNewKey[j], i), 
                        pvColVal[i].GetCharPtr(), 
                        stNewLen
                        );
                }
            }

            }
            break;

        case V_INT64:

            for (j = 0; j < iNumRows; j ++) {
                *((int64*) m_pCtx->GetData (piNewKey[j], i)) = pvColVal[i].GetInteger64();
            }

            break;

        default:

            Assert (false);
            iErrCode = ERROR_TYPE_MISMATCH;
            break;
        }

        if (iErrCode == OK) {
            iLastCol = i;
        }
        
        else break;
    }
    
    // Maintain indexing
    if (iErrCode == OK) {
        iErrCode = m_pCtx->IndexInsertDuplicateRows (iNumRows, piNewKey, pvColVal);
        if (iErrCode == ERROR_COLUMN_NOT_INDEXED) {
            iErrCode = OK;
        }
    }

    if (iErrCode != OK) {

        // First, free each piece of varlen data we allocated
        if (m_pCtx->HasVariableLengthData()) {

            Offset oOffset;

            for (i = 0; i <= iLastCol; i ++) {

                if (pvColVal[i].GetType() == V_STRING &&
                    m_pCtx->GetColumnSize (i) == VARIABLE_LENGTH_STRING) {

                    for (j = 0; j < iNumRows; j ++) {

                        oOffset = *(Offset*) m_pCtx->GetData (piNewKey[j], i);
                        if (oOffset != NO_OFFSET) {
                            m_pCtx->FreeVarLen (oOffset);                           
                        }
                    }
                }
            }
        }

        Assert (false);
        return iErrCode;
    }

    m_pCtx->IncrementNumRows (iNumRows);
    m_pCtx->SetTerminatorRowKey (iNewTerminatorKey);

    // Make valid
    for (i = 0; i < iNumRows; i ++) {
        m_pCtx->SetValidRow (piNewKey[i]);
        Assert (piNewKey[i] < iNewTerminatorKey);
    }

    return OK;
}

int WriteTable::DeleteRow (unsigned int iKey) {
    
    int iErrCode, iNewTerminatorKey = NO_KEY;

    if (!m_pCtx->IsValidRow (iKey)) {
        return ERROR_UNKNOWN_ROW_KEY;
    }

    iErrCode = m_pCtx->IndexDeleteRow (iKey, NULL);
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    unsigned int i, iTerminatorKey = m_pCtx->GetTerminatorRowKey();

    if (iKey == iTerminatorKey - 1) {

        iNewTerminatorKey = iTerminatorKey - 1;

        // We're deleting the last row before the terminator,
        // so maybe we can jump way back
        while (iNewTerminatorKey > 0 && !m_pCtx->IsValidRow (iNewTerminatorKey - 1)) {
            iNewTerminatorKey --;
        }
    }

    // Free varlen data
    if (m_pCtx->HasVariableLengthData()) {

        Offset oOffset;
        unsigned int iNumCols = m_pCtx->GetNumColumns();

        for (i = 0; i < iNumCols; i ++) {

            if (m_pCtx->IsVariableLengthString (i)) {

                oOffset = *(Offset*) m_pCtx->GetData (iKey, i);
                if (oOffset != NO_OFFSET) {
                    m_pCtx->FreeVarLen (oOffset);
                }
            }
        }
    }

    m_pCtx->SetInvalidRow (iKey);
    m_pCtx->IncrementNumRows (-1);

    if (iNewTerminatorKey != NO_KEY) {
        m_pCtx->SetTerminatorRowKey (iNewTerminatorKey);
    }

    // Hack
    if (iNewTerminatorKey == 0) {
        m_pCtx->DeleteAllIndexes();
    }

#ifdef _DEBUG
    m_pTable->CheckIntegrity();
#endif

    return OK;
}

int WriteTable::DeleteAllRows() {
    
    int iErrCode;

    iErrCode = m_pCtx->IndexDeleteAllRows();
    if (iErrCode != OK && iErrCode != ERROR_COLUMN_NOT_INDEXED) {
        Assert (false);
        return iErrCode;
    }

    // Free varlen data
    m_pCtx->FreeAllVarLenData();

    m_pCtx->SetTerminatorRowKey (0);
    m_pCtx->SetNumRows (0);
    m_pCtx->SetAllRowsInvalid();

    return OK;
}