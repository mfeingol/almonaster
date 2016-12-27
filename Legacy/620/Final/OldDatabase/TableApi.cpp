//
// Database.dll - A database library
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA  02111-1307, USA.

#define DATABASE_BUILD
#include "Table.h"
#undef DATABASE_BUILD

int Table::CheckLoad() {

    int iErrCode = OK;

    if (!m_bLoaded) {

        NamedMutex nmMutex;
        Mutex::Wait (m_pszFileName, &nmMutex);

        if (!m_bLoaded) {

            iErrCode = ReloadInternal (m_pszFileName, false);
            OS::HeapFree (m_pszFileName);
            m_pszFileName = NULL;
        }

        Mutex::Signal (nmMutex);
    }

    return iErrCode;
}

int Table::ReadData (unsigned int iKey, unsigned int iColumn, Variant* pvData) {

    CHECK_LOAD;

    if (iKey >= m_iTerminatorRowKey || RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    void* pData = Data (iKey, iColumn);

    switch (Type (iColumn)) {

    case V_INT:

        *pvData = *((int*) pData);
        break;

    case V_FLOAT:

        *pvData = *((float*) pData);
        break;

    case V_TIME:

        *pvData = *((UTCTime*) pData);
        break;

    case V_STRING:

        *pvData = (char*) pData;
        break;

    case V_INT64:

        *pvData = *((int64*) pData);
        break;

    default:

        Assert (false);
        iErrCode = ERROR_DATA_CORRUPTION;
    }

    return iErrCode;
}

int Table::WriteData (unsigned int iKey, unsigned int iColumn, const Variant& vData) {

    CHECK_LOAD;

    if (iKey >= m_iTerminatorRowKey || RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    VariantType vtType = Type (iColumn);

    if (vtType != vData.GetType()) {
        Assert (false);
        if (vtType < V_STRING || vtType > V_TIME) {
            return ERROR_DATA_CORRUPTION;
        }
        return ERROR_TYPE_MISMATCH;
    }

    // Get old data
    void* pData = Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY;
    for (i = 0; i < NumIndexCols; i ++) {

        if (m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    size_t stSize, stNewLength, stLength;

    switch (vtType) {

    case V_INT:

        *((int*) pData) = vData.GetInteger();
        break;

    case V_FLOAT:

        *((float*) pData) = vData.GetFloat();
        break;

    case V_TIME:

        *((UTCTime*) pData) = vData.GetUTCTime();
        break;

    case V_STRING:

        stSize = Size (iColumn);
        stLength = String::StrLen (vData.GetCharPtr());
        stNewLength = ALIGN (stLength + sizeof (char), MAX_ELEMENT_SIZE) + sizeof (size_t);

        if ((stSize == INITIAL_INSERTION_LENGTH && 
            stNewLength > *(size_t*) ((char*) pData - sizeof (size_t))) ||
            stLength >= stSize) {

            iErrCode = ERROR_STRING_IS_TOO_LONG;
    
        } else {

            strncpy ((char*) pData, vData.GetCharPtr(), stLength + 1);
        }

        break;

    case V_INT64:

        *((int64*) pData) = vData.GetInteger64();
        break;

    default:

        Assert (false);
        return ERROR_DATA_CORRUPTION;
    }
    
    if (iColumnIndex != NO_KEY) {

        iErrCode = m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return iErrCode;
}

int Table::Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue) {

    CHECK_LOAD;

    if (iKey >= m_iTerminatorRowKey || RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    VariantType vtType = Type (iColumn);

    if (vtType != vIncrement.GetType()) {
        Assert (false);
        if (vtType < V_STRING || vtType > V_TIME) {
            return ERROR_DATA_CORRUPTION;
        }
        return ERROR_TYPE_MISMATCH;
    }

    // Get old data
    void* pData = Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY;
    for (i = 0; i < NumIndexCols; i ++) {
        if (m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    switch (vtType) {

    case V_INT:

        if (pvOldValue != NULL) {
            *pvOldValue = *((int*) pData);
        }

        *((int*) pData) += vIncrement.GetInteger();
        break;

    case V_FLOAT:

        if (pvOldValue != NULL) {
            *pvOldValue = *((float*) pData);
        }

        *((float*) pData) += vIncrement.GetFloat();
        break;

    case V_TIME:

        if (pvOldValue != NULL) {
            *pvOldValue = *((UTCTime*) pData);
        }

        *((UTCTime*) pData) += vIncrement.GetUTCTime();
        break;

    case V_STRING:

        {
        size_t stSize = Size (iColumn);

        if (stSize == INITIAL_INSERTION_LENGTH) {
            
            if (ALIGN (String::StrLen ((char*) pData) + 
                String::StrLen (vIncrement.GetCharPtr()) + 
                sizeof (char), MAX_ELEMENT_SIZE) + sizeof (size_t) > 
                *(size_t*) ((char*) pData - sizeof (size_t))) {

                iErrCode = ERROR_STRING_IS_TOO_LONG;

            } else {
                String::StrCat ((char*) pData, vIncrement.GetCharPtr());
            }

        } else {
            
            if (String::StrLen ((char*) pData) + String::StrLen (vIncrement.GetCharPtr()) >= stSize) {

                iErrCode = ERROR_STRING_IS_TOO_LONG;

            } else {
                
                if (pvOldValue != NULL) {
                    *pvOldValue = (char*) pData;
                }
                
                String::StrCat ((char*) pData, vIncrement.GetCharPtr());
            }
        }

        }
        break;

    case V_INT64:

        if (pvOldValue != NULL) {
            *pvOldValue = *((int64*) pData);
        }

        *((int64*) pData) += vIncrement;
        break;

    default:

        Assert (false);
        return ERROR_DATA_CORRUPTION;
    }

    if (iColumnIndex != NO_KEY) {

        iErrCode = m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return iErrCode;
}


int Table::WriteAnd (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    CHECK_LOAD;

    if (iKey >= m_iTerminatorRowKey || RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (Type (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    void* pData = Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY;
    for (i = 0; i < NumIndexCols; i ++) {
        if (m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    *((int*) pData) &= iBitField;

    if (iColumnIndex != NO_KEY) {
        iErrCode = m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}


int Table::WriteOr (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    CHECK_LOAD;

    if (iKey >= m_iTerminatorRowKey || RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (Type (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    void* pData = Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY;
    for (i = 0; i < NumIndexCols; i ++) {
        if (m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    *((int*) pData) |= iBitField;

    if (iColumnIndex != NO_KEY) {
        iErrCode = m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}

int Table::WriteXor (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    CHECK_LOAD;

    if (iKey >= m_iTerminatorRowKey || RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (Type (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    void* pData = Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY;
    for (i = 0; i < NumIndexCols; i ++) {
        if (m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    *((int*) pData) ^= iBitField;

    if (iColumnIndex != NO_KEY) {
        iErrCode = m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}

int Table::WriteNot (unsigned int iKey, unsigned int iColumn) {

    CHECK_LOAD;

    if (iKey >= m_iTerminatorRowKey || RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (Type (iColumn) != V_INT) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    void* pData = Data (iKey, iColumn);

    // Update indexed columns
    unsigned int i, iColumnIndex = NO_KEY;
    for (i = 0; i < NumIndexCols; i ++) {
        if (m_pIndex[i].GetColumn() == iColumn) {
            
            iErrCode = m_pIndex[i].DeleteRow (iKey, pData);
            if (iErrCode != OK) {
                return iErrCode;
            }

            iColumnIndex = i;
            break;
        }
    }

    *((int*) pData) = ~(*((int*) pData));

    if (iColumnIndex != NO_KEY) {
        iErrCode = m_pIndex[iColumnIndex].InsertRow (iKey, pData);
        if (iErrCode != OK) {

            // Can't recover at this point
            Assert (false);
            return iErrCode;
        }
    }

    return OK;
}


int Table::WriteColumn (unsigned int iColumn, const Variant& vData) {

    CHECK_LOAD;

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    VariantType vtType = Type (iColumn);

    if (vtType != vData.GetType()) {
        Assert (false);
        if (vtType < V_STRING || vtType > V_TIME) {
            return ERROR_DATA_CORRUPTION;
        }
        return ERROR_TYPE_MISMATCH;
    }

    unsigned int i;

    // More code, but faster
    switch (vtType) {

    case V_INT:

        {

        int iData = vData.GetInteger();
    
        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                *((int*) Data (i, iColumn)) = iData;
            }
        }

        }
        break;

    case V_FLOAT:
                
        {

        float fData = vData.GetFloat();
    
        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                *((float*) Data (i, iColumn)) = fData;
            }
        }

        }
        break;
                
    case V_TIME:
        
        {

        UTCTime tData = vData.GetUTCTime();
    
        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                *((UTCTime*) Data (i, iColumn)) = tData;
            }
        }

        }
        break;

    case V_STRING:

        {

        void* pData;
        const char* pszData = vData.GetCharPtr();

        size_t stSize, stLength, stNewLength;

        stLength = String::StrLen (pszData);
        stNewLength = ALIGN (stLength + sizeof (char), MAX_ELEMENT_SIZE) + sizeof (size_t);
        stSize = Size (iColumn);

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            
            if (RowHeader (i)->Tag == VALID) {

                pData = Data (i, iColumn);
            
        
                if ((stSize == INITIAL_INSERTION_LENGTH && 
                    stNewLength > *(size_t*) ((char*) pData - sizeof (size_t))) ||
                    stLength >= stSize) {
                    
                    iErrCode = ERROR_STRING_IS_TOO_LONG;
                    break;
                    
                } else {
                    
                    strncpy ((char*) pData, vData.GetCharPtr(), stLength + 1);
                }
            }
        }

        }
        break;

    case V_INT64:
    
        {

        int64 i64Data = vData.GetInteger64();
    
        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                *((int64*) Data (i, iColumn)) = i64Data;
            }
        }

        }
        break;
        
    default:

        Assert (false);
        return ERROR_DATA_CORRUPTION;
    }

    return OK;
}

int Table::GetNumRows (unsigned int* piNumRows) {
    
    CHECK_LOAD;

    *piNumRows = m_iNumRows;

    return OK;
}

int Table::DoesRowExist (unsigned int iKey, bool* pbExists) {

    CHECK_LOAD;

    *pbExists = iKey < m_iTerminatorRowKey && RowHeader (iKey)->Tag == VALID;

    return OK;
}


int Table::InsertRow (Variant* pvColVal, unsigned int* piKey) {

    *piKey = NO_KEY;

    CHECK_LOAD;

    unsigned int i, iNumCols = NumberOfColumns, iKey = NO_KEY;
    size_t stSize;

    // Select key
    bool bFoundKey = false, bOverwriteSize = true;

    if (VariableLengthRows) {

        FifoQueue<unsigned int> fqFragQueue;

        size_t stOffset;
        while (m_fqFragQueue.Pop (&iKey)) {

            stOffset = sizeof (ROW_HEADER);

            // Need to make sure that all columns that have variable lengths are happy
            for (i = 0; i < iNumCols; i ++) {

                stSize = Size (i);
                if (stSize == INITIAL_INSERTION_LENGTH) {

                    stSize = *((size_t*) ((char*) m_ppAddress[iKey] + stOffset + (size_t) m_mmfFile.GetAddress()));

                    if (stSize < 
                        ALIGN (String::StrLen (pvColVal[i].GetCharPtr()) + sizeof (char), MAX_ELEMENT_SIZE) + 
                        sizeof (size_t)
                        ) {

                        // Too short
                        fqFragQueue.Push (iKey);
                        break;
                    }
                }

                stOffset += stSize;
            }

            if (i == iNumCols) {
                bFoundKey = true;
                bOverwriteSize = false;
                break;
            }
        }

        // Restore the frag queue
        unsigned int iTemp = 0;
        while (fqFragQueue.Pop (&iTemp)) {
            m_fqFragQueue.Push (iTemp);
        }

    } else {
        bFoundKey = m_fqFragQueue.Pop (&iKey);
    }

    size_t stAddSize = 0;
    if (!bFoundKey) {

        iKey = m_iTerminatorRowKey;
        m_iTerminatorRowKey ++;

        if (VariableLengthRows) {
            
            // Resize row pointers
            if (m_iTerminatorRowKey == m_stAddressSpace) {
                m_stAddressSpace *= 2;
                void** ppTemp = new void* [m_stAddressSpace];
                memcpy (ppTemp, m_ppAddress, m_iTerminatorRowKey * sizeof (void*));
                delete [] m_ppAddress;
                m_ppAddress = ppTemp;
            }

            // Calculate "extra" size of row
            for (i = 0; i < iNumCols; i ++) {
                if (Size (i) == INITIAL_INSERTION_LENGTH) {
                    stAddSize += 
                        ALIGN (String::StrLen (pvColVal[i].GetCharPtr()) + sizeof (char), MAX_ELEMENT_SIZE) + 
                        sizeof (size_t);
                }
            }
            
            m_ppAddress[m_iTerminatorRowKey] = (char*) m_ppAddress[iKey] + RowSize + stAddSize;
        }

        size_t stRowSize = RowSize;
        if ((char*) RowHeader (m_iTerminatorRowKey) + stRowSize + stAddSize > EndOfFile) {

            // Resize file dramatically
            size_t stOldSize = m_mmfFile.GetSize();
            size_t stNewSize = max (stOldSize * 2, FileSize + 10 * (stRowSize + stAddSize));

            iErrCode = m_mmfFile.Resize (stNewSize);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            // Fix up indices
            for (i = 0; i < NumIndexCols; i ++) {
                m_pIndex[i].SetBaseAddress (m_mmfFile.GetAddress());
            }

            m_pTemplate->IncrementTableSizeOnDisk (stNewSize - stOldSize);
        }

        RowHeader (m_iTerminatorRowKey)->Tag = TERMINATOR;
    }

    char* pData;
    for (i = 0; i < iNumCols; i ++) {

        switch (pvColVal[i].GetType()) {

        case V_INT:

            if (Type (i) != V_INT) {
                Assert (false);
                return ERROR_TYPE_MISMATCH;
            }

            *((int*) Data (iKey, i)) = pvColVal[i].GetInteger();
            break;

        case V_FLOAT:

            if (Type (i) != V_FLOAT) {
                Assert (false);
                return ERROR_TYPE_MISMATCH;
            }

            *((float*) Data (iKey, i)) = pvColVal[i].GetFloat();
            break;

        case V_TIME:

            if (Type (i) != V_TIME) {
                Assert (false);
                return ERROR_TYPE_MISMATCH;
            }

            *((UTCTime*) Data (iKey, i)) = pvColVal[i].GetUTCTime();
            break;

        case V_STRING:

            if (Type (i) != V_STRING) {
                Assert (false);
                return ERROR_TYPE_MISMATCH;
            }

            stAddSize = Size (i);
            stSize = String::StrLen (pvColVal[i].GetCharPtr());

            if (stAddSize == INITIAL_INSERTION_LENGTH) {

                if (!VariableLengthRows) {
                    Assert (false);
                    return ERROR_INITIAL_INSERTION_LENGTH_NOT_SPECIFIED;
                }
            
                pData = (char*) Data (iKey, i);

                if (bOverwriteSize) {
                    *((size_t*) ((char*) pData - sizeof (size_t))) = 
                        ALIGN (stSize + sizeof (char), MAX_ELEMENT_SIZE) + sizeof (size_t);
                }

                strncpy (pData, pvColVal[i].GetCharPtr(), stSize + 1);
                
            } else {
                
                if (stSize >= stAddSize) {  
                    return ERROR_STRING_IS_TOO_LONG;
                }

                strncpy ((char*) Data (iKey, i), pvColVal[i].GetCharPtr(), stSize + 1);
            }

            break;

        case V_INT64:

            if (Type (i) != V_INT64) {
                Assert (false);
                return ERROR_TYPE_MISMATCH;
            }

            *((int64*) Data (iKey, i)) = pvColVal[i].GetInteger64();
            break;
        
        default:

            Assert (false);
            return ERROR_DATA_CORRUPTION;
        }
    }

    // Maintain indexing
    for (i = 0; i < NumIndexCols; i ++) {
        
        iErrCode = m_pIndex[i].InsertRow (iKey, Data (iKey, m_pIndex[i].GetColumn()));
        if (iErrCode != OK) {

            Assert (false);

            int iErrCode2;

            for (unsigned int j = 0; j < i; j ++) {
                iErrCode2 = m_pIndex[j].DeleteRow (iKey, Data (iKey, m_pIndex[j].GetColumn()));
                Assert (iErrCode2 == OK);
            }

            *piKey = NO_KEY;
            return iErrCode;
        }
    }

    RowHeader (iKey)->Tag = VALID;
    *piKey = iKey;
    m_iNumRows ++;
    
    return OK;
}

int Table::InsertDuplicateRows (Variant* pvColVal, unsigned int iNumRows) {

    CHECK_LOAD;

    if (iNumRows == 0) {
        Assert (false);
        return ERROR_DATA_NOT_FOUND;
    }

    if (VariableLengthRows) {
        return InsertDuplicateVariableLengthRows (pvColVal, iNumRows);
    }

    unsigned int i, j, iNumCols = NumberOfColumns;
    size_t stSize;

    // Check types, lengths
    for (i = 0; i < iNumCols; i ++) {

        if (pvColVal[i].GetType() != Type (i)) {

            Assert (false);
            return ERROR_TYPE_MISMATCH;
        }

        if (Type (i) == V_STRING) {

            stSize = Size (i);
            if (stSize == INITIAL_INSERTION_LENGTH) {
                Assert (false);
                return ERROR_INITIAL_INSERTION_LENGTH_NOT_SPECIFIED;
            }
                
            if (String::StrLen (pvColVal[i].GetCharPtr()) >= stSize) {
                return ERROR_STRING_IS_TOO_LONG;
            }
        }
    }

    // Get keys for insertion
    unsigned int* piNewKey = (unsigned int*) StackAlloc (iNumRows * sizeof (unsigned int));

    size_t stRowSize = RowSize, stOldSize, stNewSize;
    for (i = 0; i < iNumRows; i ++) {

        if (!m_fqFragQueue.Pop (&(piNewKey[i]))) {
            
            piNewKey[i] = m_iTerminatorRowKey;
            m_iTerminatorRowKey ++;
            
            if ((char*) RowHeader (m_iTerminatorRowKey) + stRowSize > EndOfFile) {
                
                // Resize file dramatically
                // Need exclusive access of writer lock here, or every other thread will die horribly
                stOldSize = m_mmfFile.GetSize();
                stNewSize = max (stOldSize * 2, FileSize + 10 * stRowSize);

                iErrCode = m_mmfFile.Resize (stNewSize);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                // Fix up indices
                for (j = 0; j < NumIndexCols; j ++) {
                    m_pIndex[j].SetBaseAddress (m_mmfFile.GetAddress());
                }

                m_pTemplate->IncrementTableSizeOnDisk (stNewSize - stOldSize);
            }
        }
    }

    // Write new row data
    for (i = 0; i < iNumCols; i ++) {

        switch (pvColVal[i].GetType()) {

        case V_INT:

            for (j = 0; j < iNumRows; j ++) {
                *((int*) Data (piNewKey[j], i)) = pvColVal[i].GetInteger();
            }

            break;

        case V_FLOAT:

            for (j = 0; j < iNumRows; j ++) {
                *((float*) Data (piNewKey[j], i)) = pvColVal[i].GetFloat();
            }
            break;

        case V_TIME:

            for (j = 0; j < iNumRows; j ++) {
                *((UTCTime*) Data (piNewKey[j], i)) = pvColVal[i].GetUTCTime();
            }
            break;

        case V_STRING:

            for (j = 0; j < iNumRows; j ++) {
                String::StrCpy ((char*) Data (piNewKey[j], i), pvColVal[i].GetCharPtr());
            }
            break;

        case V_INT64:

            for (j = 0; j < iNumRows; j ++) {
                *((int64*) Data (piNewKey[j], i)) = pvColVal[i].GetInteger64();
            }

            break;
        }
    }
    
    // Maintain indexing
    unsigned int iColumn;
    for (i = 0; i < NumIndexCols; i ++) {
        
        iColumn = m_pIndex[i].GetColumn();
        
        for (j = 0; j < iNumRows; j ++) {

            iErrCode = m_pIndex[i].InsertRow (piNewKey[j], Data (piNewKey[j], iColumn));

            if (iErrCode != OK) {
                
                Assert (false);

                int iErrCode2;
                
                for (unsigned int k = 0; k <= i; k ++) {
                    
                    iColumn = m_pIndex[i].GetColumn();
                    
                    for (j = 0; j < iNumRows; j ++) {

                        iErrCode2 = m_pIndex[i].DeleteRow (piNewKey[j], Data (piNewKey[j], iColumn));
                        Assert (iErrCode2 == OK);

                        RowHeader (piNewKey[i])->Tag = INVALID;
                        m_fqFragQueue.Push (piNewKey[i]);
                    }
                }
                
                RowHeader (m_iTerminatorRowKey)->Tag = TERMINATOR;
                
                delete [] piNewKey;
                return iErrCode;
            }
        }
    }

    // Write valid
    for (i = 0; i < iNumRows; i ++) {
        RowHeader (piNewKey[i])->Tag = VALID;
        Assert (piNewKey[i] < m_iTerminatorRowKey);
    }
    RowHeader (m_iTerminatorRowKey)->Tag = TERMINATOR;

    m_iNumRows += iNumRows;

    return OK;
}


int Table::InsertDuplicateVariableLengthRows (Variant* pvColVal, unsigned int iNumRows) {

    CHECK_LOAD;

    unsigned int i, j, iNumCols = NumberOfColumns;
    size_t stSize, stNeededSize = 0;

    // Check types
    for (i = 0; i < iNumCols; i ++) {
        if (pvColVal[i].GetType() != Type (i)) {
            Assert (false);
            return ERROR_TYPE_MISMATCH;
        }
    }

    // Get keys for insertion
    unsigned int* piNewKey = (unsigned int*) StackAlloc ((iNumRows + iNumCols) * sizeof (unsigned int));
    unsigned int* piVarLenIndex = piNewKey + iNumRows;

    size_t* pstStrLen = (size_t*) StackAlloc (iNumCols * sizeof (size_t));
    size_t* pstNeededSize = (size_t*) StackAlloc (iNumCols * sizeof (size_t));
    bool* pbOverwriteSize = (bool*) StackAlloc (iNumRows * sizeof (bool));

    unsigned int iNumVarLenRows = 0, iKey = NO_KEY;
    
    for (i = 0; i < iNumCols; i ++) {

        stSize = Size (i);
        if (stSize == INITIAL_INSERTION_LENGTH) {

            pstStrLen[i] = String::StrLen (pvColVal[i].GetCharPtr());
            
            pstNeededSize[iNumVarLenRows] = 
                ALIGN (pstStrLen[i] + sizeof (char), MAX_ELEMENT_SIZE) + 
                sizeof (size_t);
            
            piVarLenIndex[iNumVarLenRows] = i;

            stNeededSize += pstNeededSize[iNumVarLenRows];
            iNumVarLenRows ++;
        
        }
        else if (Type(i) == V_STRING) {
            pstStrLen[i] = String::StrLen (pvColVal[i].GetCharPtr());
            pstNeededSize[i] = pstStrLen[i] + sizeof (char);
        }
    }
    
    Assert (iNumVarLenRows > 0);

    FifoQueue<unsigned int> fqDefragQueue;

    for (i = 0; i < iNumRows; i ++) {
        pbOverwriteSize[i] = true;
    }

    i = 0;
    while (i < iNumRows) {

        // Try to satisfy our needs with fragged keys
        if (!m_fqFragQueue.Pop (&iKey)) {           
            // No more keys
            break;
        }

        // See if key works
        for (j = 0; j < iNumVarLenRows; j ++) {
            
            stSize = *(size_t*) ((char*) Data (iKey, piVarLenIndex[j]));
            
            if (stSize < pstNeededSize[j]) {
                
                // Too short - key is no good
                fqDefragQueue.Push (iKey);
                break;
            }
        }
        
        if (j == iNumVarLenRows) {
            
            // The key worked, so we can reuse it
            piNewKey[i] = iKey;
            pbOverwriteSize[i] = false;
            i ++;
        }
    }

    // Put discarded keys back into frag queue
    while (fqDefragQueue.Pop (&iKey)) {
        m_fqFragQueue.Push (iKey);
    }

    // Use terminator keys for the rest of the keys
    unsigned int iFirstEndOfTableNewRow = i;
    for (; i < iNumRows; i ++) {

        piNewKey[i] = m_iTerminatorRowKey;
        m_iTerminatorRowKey ++;
    }
    
    // Check if we overstepped our bounds
    size_t stRowSize = RowSize;
    if ((char*) RowHeader (m_iTerminatorRowKey) + stRowSize + stNeededSize > EndOfFile) {

        // Resize file dramatically
        // Need exclusive access of writer lock here, or every other thread will die horribly
        size_t stOldSize = m_mmfFile.GetSize();
        size_t stNewSize = max (stOldSize * 2, FileSize + 10 * (stRowSize + stNeededSize));
        
        iErrCode = m_mmfFile.Resize (stNewSize);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        // Fix up indices
        for (i = 0; i < NumIndexCols; i ++) {
            m_pIndex[i].SetBaseAddress (m_mmfFile.GetAddress());
        }
        
        m_pTemplate->IncrementTableSizeOnDisk (stNewSize - stOldSize);
    }
    
    // Write new row data
    void* pData;

    for (i = 0; i < iNumCols; i ++) {

        switch (pvColVal[i].GetType()) {

        case V_INT:

            for (j = 0; j < iNumRows; j ++) {
                *((int*) Data (piNewKey[j], i)) = pvColVal[i].GetInteger();
            }

            break;

        case V_FLOAT:

            for (j = 0; j < iNumRows; j ++) {
                *((float*) Data (piNewKey[j], i)) = pvColVal[i].GetFloat();
            }
            break;

        case V_TIME:

            for (j = 0; j < iNumRows; j ++) {
                *((UTCTime*) Data (piNewKey[j], i)) = pvColVal[i].GetUTCTime();
            }
            break;

        case V_STRING:

            for (j = 0; j < iNumRows; j ++) {

                pData = Data (piNewKey[j], i);

                strncpy ((char*) pData, pvColVal[i].GetCharPtr(), pstStrLen[j] + 1);

                if (pbOverwriteSize[i]) {
                    *(size_t*) ((char*) pData - sizeof (size_t)) = 
                        ALIGN (pstStrLen[j] + sizeof (char), MAX_ELEMENT_SIZE) + sizeof (size_t);
                }
            }
            break;

        case V_INT64:

            for (j = 0; j < iNumRows; j ++) {
                *((int64*) Data (piNewKey[j], i)) = pvColVal[i].GetInteger64();
            }

            break;
        }
    }
    
    // Maintain indexing
    unsigned int iColumn;
    for (i = 0; i < NumIndexCols; i ++) {

        iColumn = m_pIndex[i].GetColumn();

        for (j = 0; j < iNumRows; j ++) {

            iErrCode = m_pIndex[i].InsertRow (piNewKey[j], Data (piNewKey[j], iColumn));

            if (iErrCode != OK) {
                
                int iErrCode2;

                Assert (false);
                
                for (unsigned int k = 0; k <= i; k ++) {
                    
                    iColumn = m_pIndex[i].GetColumn();
                    
                    for (j = 0; j < iNumRows; j ++) {
                        
                        iErrCode2 = m_pIndex[i].DeleteRow (piNewKey[j], Data (piNewKey[j], iColumn));
                        Assert (iErrCode2 == OK);

                        RowHeader (piNewKey[i])->Tag = INVALID;
                        m_fqFragQueue.Push (piNewKey[i]);
                    }
                }
                
                RowHeader (m_iTerminatorRowKey)->Tag = TERMINATOR;

                return iErrCode;;
            }
        }
    }

    // Set frag key rows as valid
    for (i = 0; i < iFirstEndOfTableNewRow; i ++) {
        RowHeader (piNewKey[i])->Tag = VALID;
    }

    // Write all new keys as valid except first one
    i ++;
    for (; i < iNumRows; i ++) {
        RowHeader (piNewKey[i])->Tag = VALID;
    }

    // Write new terminator, then first new key:  this ensures that we're always terminated
    if (iFirstEndOfTableNewRow != iNumRows) {
        RowHeader (m_iTerminatorRowKey)->Tag = TERMINATOR;
        RowHeader (piNewKey[iFirstEndOfTableNewRow])->Tag = VALID;
    }

    m_iNumRows += iNumRows;

    return iErrCode;
}


int Table::InsertRows (Variant* pvColVal, unsigned int iNumRows) {

    CHECK_LOAD;

    if (iNumRows == 0) {
        Assert (false);
        return ERROR_DATA_NOT_FOUND;
    }

    unsigned int i, 
        * piKey = (unsigned int*) StackAlloc (iNumRows * sizeof (unsigned int)), iNumCols = NumberOfColumns;

    for (i = 0; i < iNumRows; i ++) {

        iErrCode = InsertRow (pvColVal + i * iNumCols, piKey + i);

        if (iErrCode != OK) {

            Assert (false);

            unsigned int j;
            int iErrCode2;

            for (j = 0; j < i; j ++) {

                // Gotta be best effort
                iErrCode2 = DeleteRow (piKey[i]);
                Assert (iErrCode2 == OK);
            }

            return iErrCode;
        }
    }

    return OK;
}

int Table::DeleteRow (unsigned int iKey) {

    CHECK_LOAD;

    unsigned int i;

    if (iKey >= m_iTerminatorRowKey) {
        return ERROR_UNKNOWN_ROW_KEY;
    }

    ROW_HEADER* pHeader = RowHeader (iKey);
    if (pHeader->Tag != VALID) {
        return ERROR_UNKNOWN_ROW_KEY;
    }

    if (iKey == m_iTerminatorRowKey - 1) {

        // We're deleting the last row, so maybe we can jump way back
        if (m_iNumRows == 1) {
            
            m_iTerminatorRowKey = 0;
            ((ROW_HEADER*) RowHeader (0))->Tag = TERMINATOR;
            
            // Clear frag queue
            m_fqFragQueue.Clear();
        
        } else {

            m_iTerminatorRowKey --;
            pHeader->Tag = TERMINATOR;
        }

    } else {

        pHeader->Tag = INVALID;
        m_fqFragQueue.Push (iKey);
    }

    m_iNumRows --;

    // Maintain indexing
    for (i = 0; i < NumIndexCols; i ++) {

        iErrCode = m_pIndex[i].DeleteRow (iKey, Data (iKey, m_pIndex[i].GetColumn()));

        // This should _always_ succeed
        Assert (iErrCode == OK);
    }

    return iErrCode;
}

int Table::DeleteAllRows() {

    CHECK_LOAD;

    ((ROW_HEADER*) RowHeader (0))->Tag = TERMINATOR;

    m_fqFragQueue.Clear();

    m_iNumRows = 0;
    m_iTerminatorRowKey = 0;

    // Maintain indexing
    unsigned int i;

    for (i = 0; i < NumIndexCols; i ++) {
        iErrCode = m_pIndex[i].DeleteAllRows();

        // This should _always_ succeed
        Assert (iErrCode == OK);
    }

    return iErrCode;
}

int Table::ReadRow (unsigned int iKey, Variant** ppvData) {

    *ppvData = NULL;

    CHECK_LOAD;

    if (iKey >= m_iTerminatorRowKey || RowHeader (iKey)->Tag != VALID) {
        Assert (false);
        return ERROR_UNKNOWN_ROW_KEY;
    }

    unsigned int i, iNumCols = NumberOfColumns;

    *ppvData = new Variant [iNumCols];

    for (i = 0; i < iNumCols; i ++) {

        switch (Type(i)) {

        case V_INT:

            (*ppvData)[i] = *((int*) Data (iKey, i));
            break;

        case V_FLOAT:

            (*ppvData)[i] = *((float*) Data (iKey, i));
            break;

        case V_TIME:

            (*ppvData)[i] = *((UTCTime*) Data (iKey, i));
            break;

        case V_STRING:

            (*ppvData)[i] = (char*) Data (iKey, i);
            break;

        case V_INT64:

            (*ppvData)[i] = *((int64*) Data (iKey, i));
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

// Column operations

int Table::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, 
                       unsigned int* piNumRows) {
    
    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppvData = NULL;

    CHECK_LOAD;

    if (iColumn >= NumberOfColumns) {
        Assert (false);
        return ERROR_UNKNOWN_COLUMN_NAME;
    }

    if (m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    // Allocate space
    *ppvData = new Variant [m_iNumRows];
    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_iNumRows];
    }

    unsigned int i;

    // More code, but faster
    switch (Type (iColumn)) {

    case V_INT:
    
        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = *((int*) Data (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_FLOAT:
                
        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = *((float*) Data (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;
                
    case V_TIME:
        
        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = *((UTCTime*) Data (i, iColumn));
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_STRING:

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = (char*) Data (i, iColumn);
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumRows] = i;
                }
                (*piNumRows) ++;
            }
        }
        break;

    case V_INT64:
    
        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID) {
                (*ppvData)[*piNumRows] = *((int64*) Data (i, iColumn));
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

        if (ppiKey != NULL) {
            delete [] (*ppiKey);
            *ppiKey = NULL;
        }
        *ppvData = NULL;

        return ERROR_DATA_CORRUPTION;
    }

    return OK;
}

int Table::ReadColumns (unsigned int iNumColumns, const unsigned int* piColumn, unsigned int** ppiKey, 
                        Variant*** pppvData, unsigned int* piNumRows) {
    
    *piNumRows = 0;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *pppvData = NULL;

    CHECK_LOAD;

    unsigned int i, j, k, iRealNumColumns = NumberOfColumns;

    // Check column boundaries
    for (i = 0; i < iNumColumns; i ++) {
        if (piColumn[i] >= iRealNumColumns) {
            Assert (false);
            return ERROR_UNKNOWN_COLUMN_NAME;
        }
    }

    if (m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    // Allocate space
    Variant* pvData = new Variant [iNumColumns * m_iNumRows];
    *pppvData = new Variant* [m_iNumRows];
    for (i = 0; i < m_iNumRows; i ++) {
        (*pppvData)[i] = &(pvData[i * iNumColumns]);
    }

    if (ppiKey != NULL) {
        *ppiKey = new unsigned int [m_iNumRows];
    }

    for (i = 0; i < m_iTerminatorRowKey; i ++) {
        
        if (RowHeader (i)->Tag == VALID) {
            
            for (j = 0; j < iNumColumns; j ++) {

                switch (Type (piColumn[j])) {

                case V_INT:
                    (*pppvData)[*piNumRows][j] = *((int*) Data (i, piColumn[j]));
                    break;

                case V_FLOAT:
                    (*pppvData)[*piNumRows][j] = *((float*) Data (i, piColumn[j]));
                    break;

                case V_TIME:
                    (*pppvData)[*piNumRows][j] = *((UTCTime*) Data (i, piColumn[j]));
                    break;

                case V_STRING:
                    (*pppvData)[*piNumRows][j] = (char*) Data (i, piColumn[j]);
                    break;

                case V_INT64:
                    (*pppvData)[*piNumRows][j] = *((int64*) Data (i, piColumn[j]));
                    break;
                    
                default:
                    
                    Assert (false);
                    
                    for (k = 0; k < m_iNumRows; k ++) {
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


int Table::GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys) {

    *piNumKeys = 0;

    CHECK_LOAD;

    if (m_iNumRows == 0) {
        
        *ppiKey = NULL; 
        return ERROR_DATA_NOT_FOUND;
    }

    *ppiKey = new unsigned int [m_iNumRows];

    for (unsigned int i = 0; i < m_iTerminatorRowKey; i ++) {
        
        if (RowHeader (i)->Tag == VALID) {

            (*ppiKey)[*piNumKeys] = i;
            (*piNumKeys) ++;
        }
    }

    return OK;
}

int Table::GetNextKey (unsigned int iKey, unsigned int* piNextKey) {

    *piNextKey = NO_KEY;

    CHECK_LOAD;

    unsigned int iFirstKey = (iKey == NO_KEY) ? 0 : iKey + 1;

    for (unsigned int i = iFirstKey; i < m_iTerminatorRowKey; i ++) {
        
        if (RowHeader (i)->Tag == VALID) {

            *piNextKey = i;
            break;
        }
    }

    return (*piNextKey) == NO_KEY ? ERROR_DATA_NOT_FOUND : OK;
}

int Table::GetFirstKey (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey) {

    *piKey = NO_KEY;

    CHECK_LOAD;

    if (vData.GetType() != Type (iColumn)) {
        Assert (false);
        return ERROR_TYPE_MISMATCH;
    }

    if (m_iNumRows == 0) {
        return ERROR_DATA_NOT_FOUND;
    }

    // Indexing
    unsigned int i;
    for (i = 0; i < NumIndexCols; i ++) {
        if (m_pIndex[i].GetColumn() == iColumn) {
            return m_pIndex[i].GetFirstKey (vData, piKey);
        }
    }

    switch (vData.GetType()) {

    case V_INT:
        {
        int iData = vData.GetInteger();

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID && *((int*) Data (i, iColumn)) == iData) {
                *piKey = i;
                return OK;
            }
        }

        }
        return ERROR_DATA_NOT_FOUND;

    case V_FLOAT:
        {
        float fData = vData.GetFloat();

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID && *((float*) Data (i, iColumn)) == fData) {
                *piKey = i;
                return OK;
            }
        }

        }
        return ERROR_DATA_NOT_FOUND;

    case V_TIME:
        {
        UTCTime tData = vData.GetUTCTime();

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID && *((UTCTime*) Data (i, iColumn)) == tData) {
                *piKey = i;
                return OK;
            }
        }

        }
        return ERROR_DATA_NOT_FOUND;

    case V_STRING:
        {
        const char* pszString = vData.GetCharPtr();

        if (bCaseInsensitive) {
            
            for (i = 0; i < m_iTerminatorRowKey; i ++) {
                if (RowHeader (i)->Tag == VALID && String::StriCmp ((char*) Data (i, iColumn), pszString) == 0) {
                    *piKey = i;
                    return OK;
                }
            }

        } else {

            for (i = 0; i < m_iTerminatorRowKey; i ++) {
                if (RowHeader (i)->Tag == VALID && String::StrCmp ((char*) Data (i, iColumn), pszString) == 0) {
                    *piKey = i;
                    return OK;
                }
            }
        }

        }
        return ERROR_DATA_NOT_FOUND;

    case V_INT64:
        {
        int64 i64Data = vData.GetInteger64();

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID && *((int64*) Data (i, iColumn)) == i64Data) {
                *piKey = i;
                return OK;
            }
        }

        }
        return ERROR_DATA_NOT_FOUND;

    default:

        Assert (false);
        *piKey = NO_KEY;
        return ERROR_DATA_CORRUPTION;
    }
}

int Table::GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, 
                         unsigned int** ppiKey, unsigned int* piNumKeys) {

    *piNumKeys = 0;

    CHECK_LOAD;

    if (vData.GetType() != Type (iColumn)) {
        Assert (false);
        if (ppiKey != NULL) {
            *ppiKey = NULL;
        }
        return ERROR_TYPE_MISMATCH;
    }

    if (m_iNumRows == 0) {

        if (ppiKey != NULL) {
            *ppiKey = NULL;
        }
        return ERROR_DATA_NOT_FOUND;
    }

    // Indexing
    unsigned int i;
    for (i = 0; i < NumIndexCols; i ++) {
        if (m_pIndex[i].GetColumn() == iColumn) {
            return m_pIndex[i].GetEqualKeys (vData, ppiKey, piNumKeys);
        }
    }

    switch (vData.GetType()) {

    case V_INT:
        {
        int iData = vData.GetInteger();

        if (ppiKey != NULL) {
            *ppiKey = new unsigned int [m_iNumRows];
        }

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID && *((int*) Data (i, iColumn)) == iData) {
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumKeys] = i;
                }
                (*piNumKeys) ++;
            }
        }

        }
        break;

    case V_FLOAT:
        {
        float fData = vData.GetFloat();

        if (ppiKey != NULL) {
            *ppiKey = new unsigned int [m_iNumRows];
        }

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID && *((float*) Data (i, iColumn)) == fData) {
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumKeys] = i;
                }
                (*piNumKeys) ++;
            }
        }

        }
        break;
        
    case V_TIME:
        {
        UTCTime tData = vData.GetUTCTime();

        if (ppiKey != NULL) {
            *ppiKey = new unsigned int [m_iNumRows];
        }

        for (i = 0; i < m_iTerminatorRowKey; i ++) {

            if (RowHeader (i)->Tag == VALID && *((UTCTime*) Data (i, iColumn)) == tData) {
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumKeys] = i;
                }
                (*piNumKeys) ++;
            }
        }

        }
        break;

    case V_STRING:
        {
        const char* pszString = vData.GetCharPtr();

        if (ppiKey != NULL) {
            *ppiKey = new unsigned int [m_iNumRows];
        }

        if (bCaseInsensitive) {
            
            for (i = 0; i < m_iTerminatorRowKey; i ++) {
                if (RowHeader (i)->Tag == VALID && String::StriCmp ((char*) Data (i, iColumn), pszString) == 0) {
                    if (ppiKey != NULL) {
                        (*ppiKey)[*piNumKeys] = i;
                    }
                    (*piNumKeys) ++;
                }
            }

        } else {

            for (i = 0; i < m_iTerminatorRowKey; i ++) {
                if (RowHeader (i)->Tag == VALID && String::StrCmp ((char*) Data (i, iColumn), pszString) == 0) {
                    if (ppiKey != NULL) {
                        (*ppiKey)[*piNumKeys] = i;
                    }
                    (*piNumKeys) ++;
                }
            }
        }

        }
        break;

    case V_INT64:
        {
        int64 i64Data = vData.GetInteger64();

        if (ppiKey != NULL) {
            *ppiKey = new unsigned int [m_iNumRows];
        }

        for (i = 0; i < m_iTerminatorRowKey; i ++) {
            if (RowHeader (i)->Tag == VALID && *((int64*) Data (i, iColumn)) == i64Data) {
                if (ppiKey != NULL) {
                    (*ppiKey)[*piNumKeys] = i;
                }
                (*piNumKeys) ++;
            }
        }

        }
        break;

    default:

        Assert (false);

        if (ppiKey != NULL) {
            *ppiKey = NULL;
        }
        *piNumKeys = 0;

        return ERROR_DATA_CORRUPTION;
    }

    if (*piNumKeys > 0) {
        return OK;
    }

    if (ppiKey != NULL && *ppiKey != NULL) {
        delete [] *ppiKey;
        *ppiKey = NULL;
    }

    return ERROR_DATA_NOT_FOUND;
}


// Meaning:
// pvData[i].GetType() is not V_STRING:
// Range between pvData[i] and pvData2[i], inclusive
//
// pvData[i].GetType() is V_STRING:
// pvData2[i].GetInteger() indicates SUBSTRING_SEARCH, EXACT_SEARCH or BEGINS_WITH_SEARCH

int Table::GetSearchKeys (unsigned int iNumColumns, const unsigned int* piColumn, const Variant* pvData, 
                          const Variant* pvData2, unsigned int iStartKey, unsigned int iSkipHits, 
                          unsigned int iMaxNumHits, unsigned int** ppiKey, unsigned int* piNumHits, 
                          unsigned int* piStopKey) {

    *piNumHits = 0;

    if (piStopKey != NULL) {
        *piStopKey = NO_KEY;
    }

    CHECK_LOAD;

    if (m_iNumRows == 0) {

        *ppiKey = NULL;
        return ERROR_DATA_NOT_FOUND;
    }

    if (iStartKey == NO_KEY) {
        iStartKey = 0;
    }

    // Check for errors
    unsigned int i, j, iRealNumColumns = NumberOfColumns, iNumHitsSkipped = 0;

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
            return ERROR_UNKNOWN_COLUMN_NAME;
        }

        if (pvData[i].GetType() != Type (piColumn[i])) {            
            Assert (false);
            *ppiKey = NULL;
            return ERROR_TYPE_MISMATCH;
        }
    }

    // Allocate space for keys
    size_t stKeySpace = 16;
    *ppiKey = new unsigned int [stKeySpace];

    unsigned int* piTemp;

    iErrCode = ERROR_DATA_NOT_FOUND;

    // Loop through all keys and match all conditions
    bool bHit;

    for (i = iStartKey; i < m_iTerminatorRowKey; i ++) {

        if (RowHeader (i)->Tag == VALID) {

            // Match search parameters
            bHit = true;

            for (j = 0; j < iNumColumns && bHit; j ++) {

                switch (pvData[j].GetType()) {
                    
                case V_INT:
                    {

                    int iData;

                    if (piColumn[j] == NO_KEY) {
                        iData = i;
                    } else {
                        iData = *((int*) Data (i, piColumn[j]));
                    }

                    if (iData < pvData[j].GetInteger() || iData > pvData2[j].GetInteger()) {
                        bHit = false;
                    }
                    
                    }
                    break;

                case V_FLOAT:

                    {

                    float fData = *((float*) Data (i, piColumn[j]));

                    if (fData < pvData[j].GetFloat() || fData > pvData2[j].GetFloat()) {
                        bHit = false;
                    }
                    
                    }
                    break;

                case V_TIME:

                    {

                    UTCTime tData = *((UTCTime*) Data (i, piColumn[j]));

                    if (tData < pvData[j].GetUTCTime() || tData > pvData2[j].GetUTCTime()) {
                        bHit = false;
                    }
                    
                    }
                    break;
                    
                case V_STRING:

                    switch (pvData2[j].GetInteger()) {

                    case SUBSTRING_SEARCH_CASE_SENSITIVE:

                        if (String::StrStr (
                            (char*) Data (i, piColumn[j]), pvData[j].GetCharPtr()
                            ) == NULL) {
                            bHit = false;
                        }
                        break;

                    case SUBSTRING_SEARCH_CASE_INSENSITIVE:

                        const char* pszStr;
                        iErrCode = String::StriStr (
                            (char*) Data (i, piColumn[j]), pvData[j].GetCharPtr(), &pszStr
                            );

                        if (iErrCode != OK) {
                            delete [] (*ppiKey);
                            *ppiKey = NULL;
                            *piNumHits = 0;
                            return ERROR_OUT_OF_MEMORY;
                        }

                        if (pszStr == NULL) {
                            bHit = false;
                        }
                        break;

                    case EXACT_SEARCH_CASE_SENSITIVE:

                        if (strcmp ((char*) Data (i, piColumn[j]), pvData[j].GetCharPtr()) != 0) {
                            bHit = false;
                        }
                        break;

                    case EXACT_SEARCH_CASE_INSENSITIVE:
                        
                        if (stricmp ((char*) Data (i, piColumn[j]), pvData[j].GetCharPtr()) != 0) {
                            bHit = false;
                        }
                        break;

                    case BEGINS_WITH_SEARCH_CASE_SENSITIVE:

                        if (String::StrnCmp ((char*) Data (i, piColumn[j]), pvData[j].GetCharPtr(), 
                            strlen (pvData[j].GetCharPtr())) != 0) {
                            
                            bHit = false;
                        }

                    case BEGINS_WITH_SEARCH_CASE_INSENSITIVE:

                        if (String::StrniCmp ((char*) Data (i, piColumn[j]), pvData[j].GetCharPtr(), 
                            strlen (pvData[j].GetCharPtr())) != 0) {
                            
                            bHit = false;
                        }
                        break;

                    default:

                        Assert (false);
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
                        i64Data = *((int64*) Data (i, piColumn[j]));
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
                    
                    // Add to hits list
                    if (*piNumHits == stKeySpace) {
                        
                        stKeySpace *= 2;
                        piTemp = new unsigned int [stKeySpace];
                        
                        memcpy (piTemp, *ppiKey, *piNumHits * sizeof (unsigned int));
                        
                        delete [] (*ppiKey);
                        *ppiKey = piTemp;
                    }
                    
                    (*ppiKey)[*piNumHits] = i;
                    (*piNumHits) ++;

                    if (*piNumHits == iMaxNumHits) {
                        
                        // Abort!
                        if (piStopKey != NULL) {
                            *piStopKey = i;
                        }
                        iErrCode = ERROR_TOO_MANY_HITS;
                        break;
                    }
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