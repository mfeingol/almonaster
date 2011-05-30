// TableTableContext::Indexing.cpp: implementation of the TableContext class.
//
//////////////////////////////////////////////////////////////////////

#include "TableContext.h"

int TableContext::IndexWriteData (unsigned int iKey, unsigned int iColumn, int iData) {
    
    int iErrCode, iOldData;
    unsigned int iNewHash, iOldHash, iNumBuckets, iIndex = NO_KEY;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_INT);

    iOldData = *(int*) GetData (iKey, iColumn);
    Assert (iOldData != iData);

    iErrCode = IndexCheckUniqueData<int> (iData, iIndex, iKey, iColumn);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iNumBuckets = GetNumIndexBuckets();

    // Hash data
    iNewHash = Algorithm::GetIntHashValue (iData, iNumBuckets);
    iOldHash = Algorithm::GetIntHashValue (iOldData, iNumBuckets);

    if (iOldHash == iNewHash) {
        return OK;
    }

    // Insert new hash
    iErrCode = CreateHash (iIndex, iNewHash, iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Delete old hash - this should always succeed
    int iIgnoreErrCode = DeleteHash (iIndex, iOldHash, iKey);
    Assert (iIgnoreErrCode == OK);

    return iErrCode;
}

int TableContext::IndexWriteData (unsigned int iKey, unsigned int iColumn, float fData) {
    
    int iErrCode;
    unsigned int iNewHash, iOldHash, iNumBuckets, iIndex;
    float fOldData;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_FLOAT);

    fOldData = *(float*) GetData (iKey, iColumn);
    Assert (fOldData != fData);

    iErrCode = IndexCheckUniqueData<float> (fData, iIndex, iKey, iColumn);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iNumBuckets = GetNumIndexBuckets();

    // Hash data
    iNewHash = Algorithm::GetFloatHashValue (fData, iNumBuckets);
    iOldHash = Algorithm::GetFloatHashValue (fOldData, iNumBuckets);

    if (iOldHash == iNewHash) {
        return OK;
    }

    // Insert new hash
    iErrCode = CreateHash (iIndex, iNewHash, iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Delete old hash - this should always succeed
    int iIgnoreErrCode = DeleteHash (iIndex, iOldHash, iKey);
    Assert (iIgnoreErrCode == OK);

    return iErrCode;
}

int TableContext::IndexWriteData (unsigned int iKey, unsigned int iColumn, const char* pszData) {
    
    int iErrCode;
    unsigned int iNewHash, iOldHash, iNumBuckets, iIndex;
    const char* pszOldData;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_STRING);

    bool bCaseInsensitive = !(GetIndexFlags (iIndex) & INDEX_CASE_SENSITIVE);

    pszOldData = GetStringData (iKey, iColumn);

    if (bCaseInsensitive) {

        if (String::StriCmp (pszOldData, pszData) == 0) {
            return OK;
        }

    } else {

        if (String::StrCmp (pszOldData, pszData) == 0) {
            return OK;
        }
    }

    iErrCode = IndexCheckUniqueData<const char*> (pszData, iIndex, iKey, iColumn);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iNumBuckets = GetNumIndexBuckets();

    // Hash data
    iNewHash = Algorithm::GetStringHashValue (pszData, iNumBuckets, bCaseInsensitive);
    iOldHash = Algorithm::GetStringHashValue (pszOldData, iNumBuckets, bCaseInsensitive);

    if (iOldHash == iNewHash) {
        return OK;
    }

    // Insert new hash
    iErrCode = CreateHash (iIndex, iNewHash, iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Delete old hash - this should always succeed
    int iIgnoreErrCode = DeleteHash (iIndex, iOldHash, iKey);
    Assert (iIgnoreErrCode == OK);

    return iErrCode;
}

int TableContext::IndexWriteData (unsigned int iKey, unsigned int iColumn, const UTCTime& tData) {
    
    int iErrCode;
    unsigned int iNewHash, iOldHash, iNumBuckets, iIndex;
    UTCTime tOldData;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_TIME);

    iErrCode = IndexCheckUniqueData<const UTCTime&> (tData, iIndex, iKey, iColumn);
    if (iErrCode != OK) {
        return iErrCode;
    }

    tOldData = *(UTCTime*) GetData (iKey, iColumn);
    Assert (tOldData != tData);

    iNumBuckets = GetNumIndexBuckets();

    // Hash data
    iNewHash = Time::GetHashValue (tData, iNumBuckets);
    iOldHash = Time::GetHashValue (tOldData, iNumBuckets);

    if (iOldHash == iNewHash) {
        return OK;
    }

    // Insert new hash
    iErrCode = CreateHash (iIndex, iNewHash, iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Delete old hash - this should always succeed
    int iIgnoreErrCode = DeleteHash (iIndex, iOldHash, iKey);
    Assert (iIgnoreErrCode == OK);

    return iErrCode;
}

int TableContext::IndexWriteData (unsigned int iKey, unsigned int iColumn, int64 i64Data) {
    
    int iErrCode;
    unsigned int iNewHash, iOldHash, iNumBuckets, iIndex;
    int64 i64OldData;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_INT64);

    iErrCode = IndexCheckUniqueData<int64> (i64Data, iIndex, iKey, iColumn);
    if (iErrCode != OK) {
        return iErrCode;
    }

    i64OldData = *(int64*) GetData (iKey, iColumn);
    Assert (i64OldData != i64Data);

    iNumBuckets = GetNumIndexBuckets();

    // Hash new data
    iNewHash = Algorithm::GetInt64HashValue (i64Data, iNumBuckets);

    // Hash old data
    iOldHash = Algorithm::GetInt64HashValue (*(int64*) GetData (iKey, iColumn), iNumBuckets);

    if (iOldHash == iNewHash) {
        return OK;
    }

    // Insert new hash
    iErrCode = CreateHash (iIndex, iNewHash, iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    // Delete old hash - this should always succeed
    int iIgnoreErrCode = DeleteHash (iIndex, iOldHash, iKey);
    Assert (iIgnoreErrCode == OK);

    return iErrCode;
}

int TableContext::IndexWriteColumn (unsigned int iColumn, int iData) {

    int iErrCode = OK, iOldData;
    unsigned int iHash, iOldHash, iNumBuckets, iKey = NO_KEY, iIndex;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_INT);

    // If the column has a unique-data index, forget it
    if (GetIndexFlags (iIndex) & INDEX_UNIQUE_DATA) {
        return ERROR_DUPLICATE_DATA;
    }

    iNumBuckets = GetNumIndexBuckets();

    // Get new hash
    iHash = Algorithm::GetIntHashValue (iData, iNumBuckets);

    while (true) {

        iKey = FindNextValidRow (iKey);
        if (iKey == NO_KEY) {
            break;
        }

        iOldData = *(int*) GetData (iKey, iColumn);
        if (iOldData != iData) {

            // Hash old data
            iOldHash = Algorithm::GetIntHashValue (iOldData, iNumBuckets);
            if (iOldHash != iHash) {

                iErrCode = CreateHash (iIndex, iHash, iKey);
                if (iErrCode != OK) {
                    break;
                }
            }
        }
    }

    if (iErrCode != OK) {

        Assert (false);

        unsigned int iDelKey = NO_KEY;
        while (true) {

            iDelKey = FindNextValidRow (iKey);
            if (iDelKey == NO_KEY || iDelKey >= iKey) {
                break;
            }

            iOldData = *(int*) GetData (iDelKey, iColumn);
            if (iOldData != iData) {

                // Hash old data
                iOldHash = Algorithm::GetIntHashValue (iOldData, iNumBuckets);
                if (iOldHash != iHash) {

                    int iIgnoreErrCode = DeleteHash (iIndex, iHash, iDelKey);
                    Assert (iIgnoreErrCode == OK);
                }
            }
        }

        return iErrCode;
    }

    while (true) {
        
        iKey = FindNextValidRow (iKey);
        if (iKey == NO_KEY) {
            break;
        }
        
        iOldData = *(int*) GetData (iKey, iColumn);
        if (iOldData != iData) {

            // Hash old data
            iOldHash = Algorithm::GetIntHashValue (iOldData, iNumBuckets);
            if (iOldHash != iHash) {
            
                int iIgnoreErrCode = DeleteHash (iIndex, iHash, iKey);
                Assert (iIgnoreErrCode == OK);
            }
        }
    }
    
    return OK;
}

int TableContext::IndexWriteColumn (unsigned int iColumn, float fData) {
    
    int iErrCode = OK;
    unsigned int iHash, iOldHash, iNumBuckets, iKey = NO_KEY, iIndex;
    float fOldData;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_FLOAT);

    // If the column has a unique-data index, forget it
    if (GetIndexFlags (iIndex) & INDEX_UNIQUE_DATA) {
        return ERROR_DUPLICATE_DATA;
    }

    iNumBuckets = GetNumIndexBuckets();

    // Get new hash
    iHash = Algorithm::GetFloatHashValue (fData, iNumBuckets);

    while (true) {

        iKey = FindNextValidRow (iKey);
        if (iKey == NO_KEY) {
            break;
        }

        fOldData = *(float*) GetData (iKey, iColumn);
        if (fOldData != fData) {

            // Hash old data
            iOldHash = Algorithm::GetFloatHashValue (fOldData, iNumBuckets);
            if (iOldHash != iHash) {

                iErrCode = CreateHash (iIndex, iHash, iKey);
                if (iErrCode != OK) {
                    break;
                }
            }
        }
    }

    if (iErrCode != OK) {

        Assert (false);

        unsigned int iDelKey = NO_KEY;
        while (true) {

            iDelKey = FindNextValidRow (iKey);
            if (iDelKey == NO_KEY || iDelKey >= iKey) {
                break;
            }

            fOldData = *(float*) GetData (iDelKey, iColumn);
            if (fOldData != fData) {

                // Hash old data
                iOldHash = Algorithm::GetFloatHashValue (fOldData, iNumBuckets);
                if (iOldHash != iHash) {

                    int iIgnoreErrCode = DeleteHash (iIndex, iHash, iDelKey);
                    Assert (iIgnoreErrCode == OK);
                }
            }
        }

        return iErrCode;
    }

    while (true) {
        
        iKey = FindNextValidRow (iKey);
        if (iKey == NO_KEY) {
            break;
        }
        
        fOldData = *(float*) GetData (iKey, iColumn);
        if (fOldData != fData) {
            
            // Hash old data
            iOldHash = Algorithm::GetFloatHashValue (fOldData, iNumBuckets);
            if (iOldHash != iHash) {

                int iIgnoreErrCode = DeleteHash (iIndex, iHash, iKey);
                Assert (iIgnoreErrCode == OK);
            }
        }
    }
    
    return OK;
}

int TableContext::IndexWriteColumn (unsigned int iColumn, const UTCTime& tData) {
    
    int iErrCode = OK;
    unsigned int iHash, iOldHash, iNumBuckets, iKey = NO_KEY, iIndex;
    UTCTime tOldData;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_TIME);

    // If the column has a unique-data index, forget it
    if (GetIndexFlags (iIndex) & INDEX_UNIQUE_DATA) {
        return ERROR_DUPLICATE_DATA;
    }

    iNumBuckets = GetNumIndexBuckets();

    // Get new hash
    iHash = Time::GetHashValue (tData, iNumBuckets);

    while (true) {

        iKey = FindNextValidRow (iKey);
        if (iKey == NO_KEY) {
            break;
        }

        tOldData = *(UTCTime*) GetData (iKey, iColumn);
        if (tOldData != tData) {

            // Hash old data
            iOldHash = Time::GetHashValue (tOldData, iNumBuckets);
            if (iOldHash != iHash) {

                iErrCode = CreateHash (iIndex, iHash, iKey);
                if (iErrCode != OK) {
                    break;
                }
            }
        }
    }

    if (iErrCode != OK) {

        Assert (false);

        unsigned int iDelKey = NO_KEY;
        while (true) {

            iDelKey = FindNextValidRow (iKey);
            if (iDelKey == NO_KEY || iDelKey >= iKey) {
                break;
            }

            tOldData = *(UTCTime*) GetData (iDelKey, iColumn);
            if (tOldData != tData) {

                // Hash old data
                iOldHash = Time::GetHashValue (tOldData, iNumBuckets);
                if (iOldHash != iHash) {

                    int iIgnoreErrCode = DeleteHash (iIndex, iHash, iDelKey);
                    Assert (iIgnoreErrCode == OK);
                }
            }
        }

        return iErrCode;
    }

    while (true) {
        
        iKey = FindNextValidRow (iKey);
        if (iKey == NO_KEY) {
            break;
        }
        
        tOldData = *(UTCTime*) GetData (iKey, iColumn);
        if (tOldData != tData) {
            
            // Hash old data
            iOldHash = Time::GetHashValue (tOldData, iNumBuckets);
            if (iOldHash != iHash) {

                int iIgnoreErrCode = DeleteHash (iIndex, iHash, iKey);
                Assert (iIgnoreErrCode == OK);
            }
        }
    }
    
    return OK;
}

int TableContext::IndexWriteColumn (unsigned int iColumn, int64 i64Data) {
    
    int iErrCode = OK;
    unsigned int iHash, iOldHash, iNumBuckets, iKey = NO_KEY, iIndex;
    int64 i64OldData;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_INT64);

    // If the column has a unique-data index, forget it
    if (GetIndexFlags (iIndex) & INDEX_UNIQUE_DATA) {
        return ERROR_DUPLICATE_DATA;
    }

    iNumBuckets = GetNumIndexBuckets();

    // Get new hash
    iHash = Algorithm::GetInt64HashValue (i64Data, iNumBuckets);

    while (true) {

        iKey = FindNextValidRow (iKey);
        if (iKey == NO_KEY) {
            break;
        }

        i64OldData = *(int64*) GetData (iKey, iColumn);
        if (i64OldData != i64Data) {

            // Hash old data
            iOldHash = Algorithm::GetInt64HashValue (i64OldData, iNumBuckets);
            if (iOldHash != iHash) {

                iErrCode = CreateHash (iIndex, iHash, iKey);
                if (iErrCode != OK) {
                    break;
                }
            }
        }
    }

    if (iErrCode != OK) {

        Assert (false);

        unsigned int iDelKey = NO_KEY;
        while (true) {

            iDelKey = FindNextValidRow (iKey);
            if (iDelKey == NO_KEY || iDelKey >= iKey) {
                break;
            }

            i64OldData = *(int64*) GetData (iDelKey, iColumn);
            if (i64OldData != i64Data) {

                // Hash old data
                iOldHash = Algorithm::GetInt64HashValue (i64OldData, iNumBuckets);
                if (iOldHash != iHash) {

                    int iIgnoreErrCode = DeleteHash (iIndex, iHash, iDelKey);
                    Assert (iIgnoreErrCode == OK);
                }
            }
        }

        return iErrCode;
    }

    while (true) {
        
        iKey = FindNextValidRow (iKey);
        if (iKey == NO_KEY) {
            break;
        }
        
        i64OldData = *(int64*) GetData (iKey, iColumn);
        if (i64OldData != i64Data) {
            
            // Hash old data
            iOldHash = Algorithm::GetInt64HashValue (i64OldData, iNumBuckets);
            if (iOldHash != iHash) {

                int iIgnoreErrCode = DeleteHash (iIndex, iHash, iKey);
                Assert (iIgnoreErrCode == OK);
            }
        }
    }
    
    return OK;
}

int TableContext::IndexInsertRow (unsigned int iKey, const Variant* pvData) {
    
    int iErrCode = OK;

    unsigned int i, iHash, iNumIndexes = GetNumIndexColumns(), iColumn;
    unsigned int iNumBuckets = GetNumIndexBuckets();

    if (iNumIndexes == 0) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    // Uniqueness check
    for (i = 0; i < iNumIndexes; i ++) {

        unsigned int iFlags = GetIndexFlags (i);

        if (iFlags & INDEX_UNIQUE_DATA) {

            unsigned int iDupKey;

            iColumn = m_pTemplate->TemplateData.IndexColumn[i];
            Assert (iColumn != NO_KEY);
            Assert (m_pTemplate->TemplateData.Type [iColumn] == pvData[iColumn].GetType());

            switch (m_pTemplate->TemplateData.Type [iColumn]) {

            case V_INT:
                iErrCode = IndexGetFirstKey (iColumn, pvData[iColumn].GetInteger(), &iDupKey);
                break;

            case V_TIME:
                iErrCode = IndexGetFirstKey (iColumn, pvData[iColumn].GetUTCTime(), &iDupKey);
                break;

            case V_FLOAT:
                iErrCode = IndexGetFirstKey (iColumn, pvData[iColumn].GetFloat(), &iDupKey);
                break;

            case V_STRING:
                iErrCode = IndexGetFirstKey (iColumn, pvData[iColumn].GetCharPtr(), !(iFlags & INDEX_CASE_SENSITIVE), &iDupKey);
                break;

            case V_INT64:
                iErrCode = IndexGetFirstKey (iColumn, pvData[iColumn].GetInteger64(), &iDupKey);
                break;

            default:
                iErrCode = ERROR_DATA_CORRUPTION;
            }

            if (iErrCode == OK) {
                return ERROR_DUPLICATE_DATA;
            }

            if (iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                return iErrCode;
            }
            iErrCode = OK;
        }
    }

    // Insertion
    for (i = 0; i < iNumIndexes; i ++) {

        bool bCaseInsensitive = !(GetIndexFlags (i) & INDEX_CASE_SENSITIVE);

        iColumn = m_pTemplate->TemplateData.IndexColumn[i];
        iHash = Algorithm::GetVariantHashValue (pvData [iColumn], iNumBuckets, bCaseInsensitive);

        iErrCode = CreateHash (i, iHash, iKey);
        if (iErrCode != OK) {
            Assert (false);
            break;
        }
    }

    if (iErrCode != OK) {

        unsigned int iLastIndex = i;

        for (i = 0; i < iLastIndex; i ++) {

            bool bCaseInsensitive = !(GetIndexFlags (i) & INDEX_CASE_SENSITIVE);

            iColumn = m_pTemplate->TemplateData.IndexColumn[i];
            iHash = Algorithm::GetVariantHashValue (pvData [iColumn], iNumBuckets, bCaseInsensitive);

            int iIgnoreErrorCode = DeleteHash (i, iHash, iKey);
            Assert (iIgnoreErrorCode == OK);
        }
    }

    return iErrCode;
}

int TableContext::IndexInsertDuplicateRows (unsigned int iNumRows, const unsigned int* piNewKey, 
                                            const Variant* pvColVal) {
    
    int iErrCode = OK;
    unsigned int i;

    Assert (iNumRows > 0);

    if (iNumRows > 1 && HasUniqueDataIndex()) {
        return ERROR_DUPLICATE_DATA;
    }

    for (i = 0; i < iNumRows; i ++) {

        iErrCode = IndexInsertRow (piNewKey[i], pvColVal);
        if (iErrCode != OK) {
            break;
        }
    }

    if (iErrCode != OK) {
        
        unsigned int iLastIndex = i;
        
        for (i = 0; i < iLastIndex; i ++) {
            int iIgnoreErrorCode = IndexDeleteRow (piNewKey[i], pvColVal);
            Assert (iIgnoreErrorCode == OK);
        }
    }
    
    return iErrCode;
}

int TableContext::IndexDeleteRow (int iKey, const Variant* pvData) {

    Offset oOffset;

    unsigned int i, iHash, iNumIndexes = GetNumIndexColumns(), iColumn, iNumBuckets = GetNumIndexBuckets();

    if (iNumIndexes == 0) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    for (i = 0; i < iNumIndexes; i ++) {

        bool bCaseInsensitive = !(GetIndexFlags (i) & INDEX_CASE_SENSITIVE);

        oOffset = GetIndexOffset (i);
        iColumn = m_pTemplate->TemplateData.IndexColumn[i];

        if (pvData != NULL) {

            iHash = Algorithm::GetVariantHashValue (pvData [iColumn], iNumBuckets, bCaseInsensitive);
        
        } else {

            VariantType vtType = GetColumnType (iColumn);

            switch (vtType) {
            
            case V_INT:
                iHash = Algorithm::GetIntHashValue (*(int*) GetData (iKey, iColumn), iNumBuckets);
                break;

            case V_FLOAT:
                iHash = Algorithm::GetFloatHashValue (*(float*) GetData (iKey, iColumn), iNumBuckets);
                break;

            case V_STRING:

                iHash = Algorithm::GetStringHashValue (GetStringData (iKey, iColumn), iNumBuckets, bCaseInsensitive);
                break;

            case V_TIME:
                iHash = Time::GetHashValue (*(UTCTime*) GetData (iKey, iColumn), iNumBuckets);
                break;

            case V_INT64:
                iHash = Algorithm::GetInt64HashValue (*(int64*) GetData (iKey, iColumn), iNumBuckets);
                break;

            default:
                iHash = 0;
                Assert (false);
                break;
            }
        }

        int iIgnoreErrorCode = DeleteHash (i, iHash, iKey);
        Assert (iIgnoreErrorCode == OK);
    }

    return OK;
}

int TableContext::IndexDeleteAllRows() {

    DeleteAllIndexes();

    return OK;
}


int TableContext::IndexGetFirstKey (unsigned int iColumn, int iData, unsigned int* piKey) {

    unsigned int i, iHash, iIndex;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_INT);

    Offset oOffset = GetIndexOffset (iIndex);

    iHash = Algorithm::GetIntHashValue (iData, GetNumIndexBuckets());

    FileHeap* pMetaHeap = GetMetaDataHeap();
    IndexHeader* pIndexHeader = GetIndexHeader (oOffset);
    Offset oChain = pIndexHeader->pBucket[iHash].oChain;

    while (oChain != NO_OFFSET) {

        IndexNode* pNode = (IndexNode*) pMetaHeap->GetAddress (oOffset + oChain);
        Assert (pNode->oNext != oChain);

        for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {

            unsigned int iKey = pNode->piKey[i];
            if (iKey != NO_KEY) {

                if (*(int*) GetData (iKey, iColumn) == iData) { // Test data equality
                    *piKey = iKey;
                    return OK;
                }
            }
        }

        oChain = pNode->oNext;  // Next node
    }

    *piKey = NO_KEY;
    return ERROR_DATA_NOT_FOUND;
}

int TableContext::IndexGetFirstKey (unsigned int iColumn, float fData, unsigned int* piKey) {

    unsigned int i, iHash, iIndex;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_FLOAT);

    Offset oOffset = GetIndexOffset (iIndex);
    
    iHash = Algorithm::GetFloatHashValue (fData, GetNumIndexBuckets());

    FileHeap* pMetaHeap = GetMetaDataHeap();
    IndexHeader* pIndexHeader = GetIndexHeader (oOffset);
    Offset oChain = pIndexHeader->pBucket[iHash].oChain;

    while (oChain != NO_OFFSET) {

        IndexNode* pNode = (IndexNode*) pMetaHeap->GetAddress (oOffset + oChain);
        Assert (pNode->oNext != oChain);

        for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {

            unsigned int iKey = pNode->piKey[i];
            if (iKey != NO_KEY) {

                if (*(float*) GetData (iKey, iColumn) == fData) {   // Test data equality
                    *piKey = iKey;
                    return OK;
                }
            }
        }

        oChain = pNode->oNext;  // Next node
    }

    *piKey = NO_KEY;
    return ERROR_DATA_NOT_FOUND;
}

int TableContext::IndexGetFirstKey (unsigned int iColumn, const char* pszData, unsigned int* piKey) {

    unsigned int iIndex;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    bool bCaseInsensitive = !(GetIndexFlags (iIndex) & INDEX_CASE_SENSITIVE);

    return IndexGetFirstKey (iColumn, pszData, bCaseInsensitive, piKey);
}

int TableContext::IndexGetFirstKey (unsigned int iColumn, const char* pszData, bool bCaseInsensitive, 
                                    unsigned int* piKey) {
    
    unsigned int i, iHash, iIndex;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_STRING);

    if (bCaseInsensitive == ((GetIndexFlags (iIndex) & INDEX_CASE_SENSITIVE) != 0)) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Offset oOffset = GetIndexOffset (iIndex);

    iHash = Algorithm::GetStringHashValue (pszData, GetNumIndexBuckets(), bCaseInsensitive);

    FileHeap* pMetaHeap = GetMetaDataHeap();
    IndexHeader* pIndexHeader = GetIndexHeader (oOffset);
    Offset oChain = pIndexHeader->pBucket[iHash].oChain;

    while (oChain != NO_OFFSET) {

        IndexNode* pNode = (IndexNode*) pMetaHeap->GetAddress (oOffset + oChain);
        Assert (pNode->oNext != oChain);

        for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {

            unsigned int iKey = pNode->piKey[i];
            if (iKey != NO_KEY) {

                if ((bCaseInsensitive && String::StriCmp (GetStringData (iKey, iColumn), pszData) == 0) ||
                    (!bCaseInsensitive && String::StrCmp (GetStringData (iKey, iColumn), pszData) == 0)) {
                    *piKey = iKey;
                    return OK;
                }
            }
        }

        oChain = pNode->oNext;  // Next node
    }

    *piKey = NO_KEY;
    return ERROR_DATA_NOT_FOUND;
}

int TableContext::IndexGetFirstKey (unsigned int iColumn, const UTCTime& tData, unsigned int* piKey) {
    
    unsigned int i, iHash, iIndex;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_TIME);
    
    Offset oOffset = GetIndexOffset (iIndex);

    iHash = Time::GetHashValue (tData, GetNumIndexBuckets());

    FileHeap* pMetaHeap = GetMetaDataHeap();
    IndexHeader* pIndexHeader = GetIndexHeader (oOffset);
    Offset oChain = pIndexHeader->pBucket[iHash].oChain;

    while (oChain != NO_OFFSET) {

        IndexNode* pNode = (IndexNode*) pMetaHeap->GetAddress (oOffset + oChain);
        Assert (pNode->oNext != oChain);

        for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {

            unsigned int iKey = pNode->piKey[i];
            if (iKey != NO_KEY) {

                if (*(UTCTime*) GetData (iKey, iColumn) == tData) { // Test data equality
                    *piKey = iKey;
                    return OK;
                }
            }
        }

        oChain = pNode->oNext;  // Next node
    }

    *piKey = NO_KEY;
    return ERROR_DATA_NOT_FOUND;
}

int TableContext::IndexGetFirstKey (unsigned int iColumn, int64 i64Data, unsigned int* piKey) {
    
    unsigned int i, iHash, iIndex;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == V_INT64);

    Offset oOffset = GetIndexOffset (iIndex);
    
    iHash = Algorithm::GetInt64HashValue (i64Data, GetNumIndexBuckets());

    FileHeap* pMetaHeap = GetMetaDataHeap();
    IndexHeader* pIndexHeader = GetIndexHeader (oOffset);
    Offset oChain = pIndexHeader->pBucket[iHash].oChain;

    while (oChain != NO_OFFSET) {

        IndexNode* pNode = (IndexNode*) pMetaHeap->GetAddress (oOffset + oChain);
        Assert (pNode->oNext != oChain);

        for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {

            unsigned int iKey = pNode->piKey[i];
            if (iKey != NO_KEY) {

                if (*(int64*) GetData (iKey, iColumn) == i64Data) { // Test data equality
                    *piKey = iKey;
                    return OK;
                }
            }
        }

        oChain = pNode->oNext;  // Next node
    }

    *piKey = NO_KEY;
    return ERROR_DATA_NOT_FOUND;
}

int TableContext::IndexGetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, 
                                     unsigned int** ppiKey, unsigned int* piNumKeys) {

    unsigned int i, iHash, iIndex;

    iIndex = GetIndexForColumn (iColumn);
    if (iIndex == NO_KEY) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Assert (m_pTemplate->TemplateData.Type[iColumn] == vData.GetType());

    if (vData.GetType() == V_STRING &&
        bCaseInsensitive == ((GetIndexFlags (iIndex) & INDEX_CASE_SENSITIVE) != 0)) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    Offset oOffset = GetIndexOffset (iIndex);

    iHash = Algorithm::GetVariantHashValue (vData, GetNumIndexBuckets(), bCaseInsensitive);

    FileHeap* pMetaHeap = GetMetaDataHeap();
    IndexHeader* pIndexHeader = GetIndexHeader (oOffset);
    Offset oChain = pIndexHeader->pBucket[iHash].oChain;

    unsigned int* piKey = NULL, iNumKeys = 0, iKeySpace = 0;

    while (oChain != NO_OFFSET) {

        IndexNode* pNode = (IndexNode*) pMetaHeap->GetAddress (oOffset + oChain);
        Assert (pNode->oNext != oChain);

        for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {

            unsigned int iKey = pNode->piKey[i];
            if (iKey != NO_KEY) {

                bool bHit;

                switch (vData.GetType()) {

                case V_INT:
                    bHit = *(int*) GetData (iKey, iColumn) == vData.GetInteger();
                    break;

                case V_STRING:
                    {
                    const char* pszData = vData.GetCharPtr();                   
                    bHit = 
                        (bCaseInsensitive && String::StriCmp (GetStringData (iKey, iColumn), pszData) == 0) 
                        ||
                        (!bCaseInsensitive && String::StrCmp (GetStringData (iKey, iColumn), pszData) == 0);
                    }
                    break;

                case V_FLOAT:
                    bHit = *(float*) GetData (iKey, iColumn) == vData.GetFloat();
                    break;

                case V_TIME:
                    bHit = *(UTCTime*) GetData (iKey, iColumn) == vData.GetUTCTime();
                    break;

                case V_INT64:
                    bHit = *(int64*) GetData (iKey, iColumn) == vData.GetInteger64();
                    break;

                default:
                    bHit = false;
                    Assert (false);
                    break;
                }

                if (bHit) {

                    if (ppiKey != NULL) {
                        int iErrCode = AppendKey (piKey, iNumKeys, iKeySpace, iKey);
                        if (iErrCode != OK) {
                            return iErrCode;
                        }
                    }

                    iNumKeys ++;
                }
            }
        }

        oChain = pNode->oNext;  // Next node
    }

    if (iNumKeys > 0) {

        *piNumKeys = iNumKeys;

        if (ppiKey != NULL) {

            // Sort the keys and return
            Algorithm::QSortAscending<unsigned int> (piKey, iNumKeys);
            *ppiKey = piKey;
        }

        return OK;
    }

    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *piNumKeys = 0;

    return ERROR_DATA_NOT_FOUND;
}


int TableContext::GetInitialSearchKeys (const SearchDefinition& sdSearch, 
                                        unsigned int** ppiKeys, unsigned int* piNumKeys) {

    int iErrCode;

    unsigned int iNumIndexes = GetNumIndexColumns();
    if (iNumIndexes == 0) {
        return ERROR_COLUMN_NOT_INDEXED;
    }

    *ppiKeys = NULL;
    *piNumKeys = 0;

    unsigned int i, j, iNumSearchCols = sdSearch.iNumColumns;

    for (i = 0; i < iNumSearchCols; i ++) {

        const SearchColumn& sc = sdSearch.pscColumns[i];
        const unsigned int iColumn = sc.iColumn;

        if (iColumn == NO_KEY) {
            continue;
        }

        const VariantType vt = sc.vData.GetType();
        Assert (m_pTemplate->TemplateData.Type[iColumn] == vt);

        bool bSearchCI = false;

        // Ensure exact search
        if (vt == V_STRING) {
            
            if (!(sc.iFlags & SEARCH_EXACT)) {
                continue;
            }

        } else {

            if (sc.vData != sc.vData2) {
                continue;
            }
        }

        // Ensure an index exists for the column
        const unsigned int iIndex = GetIndexForColumn (iColumn);
        if (iIndex == NO_KEY) {
            continue;
        }

        // Ensure string columns match the case sensitivity of the index
        if (vt == V_STRING) {

            const bool bIndexCI = !(GetIndexFlags (iIndex) & INDEX_CASE_SENSITIVE);

            bSearchCI = !(sc.iFlags & SEARCH_CASE_SENSITIVE);
            if (bSearchCI != bIndexCI) {
                continue;
            }
        }

        // We found a usable index for a search column. Do the query
        unsigned int* piKeys = NULL, iNumKeys;

        iErrCode = IndexGetEqualKeys (iColumn, sc.vData, bSearchCI, &piKeys, &iNumKeys);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }

        else if (iErrCode == OK) {

            // Blow away everything before the start key
            // TODO - use binary search
            unsigned int iStartKey = sdSearch.iStartKey;
            if (iStartKey != NO_KEY) {

                for (j = 0; j < iNumKeys; j ++) {

                    if (piKeys[j] >= iStartKey) {
                        break;
                    }
                }

                if (j == iNumKeys) {

                    iErrCode = ERROR_DATA_NOT_FOUND;
                    delete [] piKeys;
                    piKeys = NULL;
                    iNumKeys = 0;

                } else {

                    iNumKeys -= j;
                    memmove (piKeys, piKeys + j, iNumKeys * sizeof (unsigned int));
                }
            }

            *ppiKeys = piKeys;
            *piNumKeys = iNumKeys;
        }

        return iErrCode;
    }

    return ERROR_COLUMN_NOT_INDEXED;
}

template <class T> int TableContext::IndexCheckUniqueData (T tData, unsigned int iIndex, unsigned int iKey, 
                                                           unsigned int iColumn) {
    int iErrCode;
    unsigned int iDupKey;

    Assert (iIndex != NO_KEY);
    if (!(GetIndexFlags (iIndex) & INDEX_UNIQUE_DATA)) {
        return OK;
    }

    iErrCode = IndexGetFirstKey (iColumn, tData, &iDupKey);
    if (iErrCode == OK) {
        Assert (iDupKey != NO_KEY);
        return ERROR_DUPLICATE_DATA;
    }

    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }

    return OK;
}

//
// Utilities
//

int TableContext::CreateAllIndexes (unsigned int iNumNewRows) {

    int iErrCode;

    unsigned int i, j, iNumIndexes = GetNumIndexColumns();
    if (iNumIndexes == 0) {
        return OK;
    }

    TableHeader* pTableHeader = GetTableHeader();
    FileHeap* pMetaHeap = GetMetaDataHeap();

    IndexHeader* pIndexHeader;

    //
    // Calculate initial size of block
    //
    unsigned int iNumBuckets = GetNumIndexBuckets();

    // Add space for buckets
    Size sHeaderSize = ALIGN ((sizeof (IndexHeader) + (iNumBuckets - 1) * sizeof (IndexBucket)), 8);
    size_t stNumNodesAlloc = iNumIndexes * (iNumNewRows / KEYS_PER_INDEX_NODE + 1);

    // Add space for the rows that have been allocated
    Size sSize = sHeaderSize + stNumNodesAlloc * sizeof (IndexNode);

    iErrCode = OK;

    for (i = 0; i < iNumIndexes; i ++) {

        // Allocate the new block
        pMetaHeap->Unlock();
        Offset oOffset = pMetaHeap->Allocate (sSize);
        pMetaHeap->Lock();
        
        if (oOffset == NO_OFFSET) {
            Assert (false);
            iErrCode = ERROR_OUT_OF_DISK_SPACE;
            break;
        }

        // Assign the new block
        Assert (pTableHeader->poIndexOffset[i] == NO_OFFSET);
        pTableHeader->poIndexOffset[i] = oOffset;
        
        // Zero the new block
        pMetaHeap->MemSet (oOffset, 0, sSize);

        pIndexHeader = GetIndexHeader (oOffset);
        pIndexHeader->oFreeList = (Offset) sHeaderSize;

        // Initialize the empty buckets
        for (j = 0; j < iNumBuckets; j ++) {
            pIndexHeader->pBucket[j].oChain = NO_OFFSET;
        }

        // Format the nodes in the free list
        FormatIndexNodes (oOffset, (Offset) sHeaderSize, stNumNodesAlloc);
    }

    if (iErrCode != OK) {

        unsigned int iLastIndex = i;
        
        for (i = 0; i < iLastIndex; i ++) {

            Offset oOffset = pTableHeader->poIndexOffset[i];
            
            Assert (oOffset != NO_OFFSET);
            
            pMetaHeap->Unlock();
            pMetaHeap->Free (oOffset);
            pMetaHeap->Lock();

            pTableHeader->poIndexOffset[i] = NO_OFFSET;
        }
    }

    return iErrCode;
}

void TableContext::FormatIndexNodes (Offset oBaseOffset, Offset oFirstNode, size_t stNumNodes) {

    Assert (oBaseOffset != NO_OFFSET);
    Assert (oFirstNode != NO_OFFSET && oFirstNode > 0);
    Assert (stNumNodes > 0);

    FileHeap* pMetaHeap = GetMetaDataHeap();

    Offset oNodeOffset = oFirstNode;
    IndexNode* pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oNodeOffset);
    Assert (pNode->oNext != oNodeOffset);

    size_t i = 0, j, stLoopGuard = stNumNodes - 1;
    while (i < stLoopGuard) {

        oNodeOffset += sizeof (IndexNode);
        pNode->oNext = oNodeOffset;

        for (j = 0; j < KEYS_PER_INDEX_NODE; j ++) {
            pNode->piKey[j] = NO_KEY;
        }

        pNode ++;
        i ++;
    }

    pNode->oNext = NO_OFFSET;
    for (j = 0; j < KEYS_PER_INDEX_NODE; j ++) {
        pNode->piKey[j] = NO_KEY;
    }

    Assert (oNodeOffset + sizeof (IndexNode) == oFirstNode + stNumNodes * sizeof (IndexNode));
}

void TableContext::DeleteAllIndexes() {

    unsigned int i, iNumIndexes = GetNumIndexColumns();

    TableHeader* pTableHeader = GetTableHeader();
    FileHeap* pMetaHeap = GetMetaDataHeap();

    pMetaHeap->Unlock();

    for (i = 0; i < iNumIndexes; i ++) {
        
        Offset oOffset = pTableHeader->poIndexOffset[i];
        
        if (oOffset != NO_OFFSET) {
            pMetaHeap->Free (oOffset);
            pTableHeader->poIndexOffset[i] = NO_OFFSET;
        }
    }

    pMetaHeap->Lock();
}

unsigned int TableContext::GetIndexForColumn (unsigned int iColumn) {

    const unsigned int iNumIndexes = GetNumIndexColumns();
    const unsigned int* piColumn = m_pTemplate->TemplateData.IndexColumn;

    unsigned int iIndex = NO_KEY;

    if (iNumIndexes > 0) {
        
        unsigned int iLeft = 0, iRight = iNumIndexes - 1, iCurrent;
        
        while (iLeft < iRight) {
            
            iCurrent = (iLeft + iRight) / 2;
            if (piColumn [iCurrent] < iColumn) {
                iLeft = iCurrent + 1;
            } else {        
                iRight = iCurrent;
            }
        }
        
        Assert (iLeft == iRight);
        
        if (piColumn [iLeft] == iColumn) {
            iIndex = iLeft;
        }
    }

#ifdef _DEBUG

    unsigned int i;

    for (i = 0; i < iNumIndexes; i ++) {
        
        if (piColumn[i] == iColumn) {
            Assert (i == iIndex);
            break;
        }
    }

    if (i == iNumIndexes) {
        Assert (iIndex == NO_KEY);
    }

#endif

    return iIndex;
}

int TableContext::CreateHash (unsigned int iIndex, unsigned int iHash, unsigned int iKey) {

    unsigned int i;
    IndexNode* pNode;

    Offset oNewNode, oCurrentNode = NO_OFFSET, oBaseOffset = GetIndexOffset (iIndex);

    FileHeap* pMetaHeap = GetMetaDataHeap();
    IndexHeader* pIndexHeader = GetIndexHeader (oBaseOffset);

    Offset oChain = pIndexHeader->pBucket[iHash].oChain;
    if (oChain == NO_OFFSET) {

        // Grab some space off the free list
        oNewNode = AllocateIndexNode (iIndex);
        if (oNewNode == NO_OFFSET) {
            Assert (false);
            return ERROR_OUT_OF_DISK_SPACE;
        }

        // Refresh locals
        oBaseOffset = GetIndexOffset (iIndex);
        pIndexHeader = GetIndexHeader (oBaseOffset);

        // Set chain into bucket
        pIndexHeader->pBucket[iHash].oChain = oChain = oNewNode;
    }

    // Find some free space in the node or its friends
    pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oChain);
    Assert (pNode->oNext != oChain);

    while (true) {

        for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {

            // Can't already have this hash
            Assert (pNode->piKey[i] != iKey);

            if (pNode->piKey[i] == NO_KEY) {
                pNode->piKey[i] = iKey;
                return OK;
            }
        }

        // No free space left in the node - try the next one
        oCurrentNode = oChain;
        oChain = pNode->oNext;
        if (oChain == NO_OFFSET) {
            break;
        }

        pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oChain);
        Assert (pNode->oNext != oChain);
    }

    // Everything in the chain is taken - need more space
    oNewNode = AllocateIndexNode (iIndex);
    if (oNewNode == NO_OFFSET) {
        Assert (false);
        return ERROR_OUT_OF_DISK_SPACE;
    }

    Assert (oCurrentNode != NO_OFFSET);
    Assert (oNewNode != oCurrentNode);

    oBaseOffset = GetIndexOffset (iIndex);

    // Set next pointer
    pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oCurrentNode);

    Assert (pNode->oNext == NO_OFFSET);
    pNode->oNext = oNewNode;

    // Use the new node
    pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oNewNode);

    Assert (pNode->oNext == NO_OFFSET);
    for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {
        Assert (pNode->piKey[i] == NO_KEY);
    }

    pNode->piKey[0] = iKey;

    return OK;
}

int TableContext::DeleteHash (unsigned int iIndex, unsigned int iHash, unsigned int iKey) {

    unsigned int i, j;

    Offset oBaseOffset = GetIndexOffset (iIndex);

    IndexNode* pNode, * pPrevNode;
    IndexHeader* pIndexHeader = GetIndexHeader (oBaseOffset);
    FileHeap* pMetaHeap = GetMetaDataHeap();

    // Get offset
    Offset oChain = pIndexHeader->pBucket[iHash].oChain;
    if (oChain == NO_OFFSET) {
        Assert (false);
        return WARNING;
    }

    // Traverse nodes
    pPrevNode = NULL;
    pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oChain);
    Assert (pNode->oNext != oChain);

    while (true) {
        
        for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {
            
            if (pNode->piKey[i] == iKey) {
                
                // Destroy the hash
                pNode->piKey[i] = NO_KEY;

                // See if all keys are null
                for (j = 0; j < KEYS_PER_INDEX_NODE; j ++) {

                    if (pNode->piKey[j] != NO_KEY) {
                        return OK;
                    }
                }

                // If they are, free the node
                if (pPrevNode == NULL) {
                    pIndexHeader->pBucket[iHash].oChain = pNode->oNext;
                } else {
                    pPrevNode->oNext = pNode->oNext;
                }

                FreeIndexNode (oBaseOffset, oChain);
                return OK;
            }
        }

        // Next node
        oChain = pNode->oNext;
        if (oChain == NO_OFFSET) {
            break;  // End of the line
        }

        pPrevNode = pNode;
        pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oChain);
        Assert (pNode->oNext != oChain);
    }

    Assert (false);
    return WARNING;
}

Offset TableContext::AllocateIndexNode (unsigned int iIndex) {

    unsigned int i;

    FileHeap* pMetaHeap = GetMetaDataHeap();
    Offset oBaseOffset = GetIndexOffset (iIndex);
    IndexHeader* pIndexHeader = GetIndexHeader (oBaseOffset);
    IndexNode* pNode;

    if (pIndexHeader->oFreeList == NO_OFFSET) {

        // Need to resize the whole block and get some more space
        Size sBucketSize = ALIGN (GetNumIndexBuckets() * sizeof (IndexBucket), 8);
        Size sOldHeapSize = pMetaHeap->GetBlockSize (oBaseOffset) - sBucketSize;

        // Heuristic:
        // Add 1/3 to the heap size, or ten nodes, whichever is biggest
        Size sNewHeapSize = ALIGN (
            sOldHeapSize + max (sOldHeapSize / 3, 10 * sizeof (IndexNode)), sizeof (IndexNode)
            );

        pMetaHeap->Unlock();
        Offset oNewBaseOffset = pMetaHeap->Reallocate (oBaseOffset, sBucketSize + sNewHeapSize);
        pMetaHeap->Lock();

        if (oNewBaseOffset == NO_OFFSET) {
            return NO_OFFSET;
        }

        if (oBaseOffset != oNewBaseOffset) {

            // Reset table metadata
            GetTableHeader()->poIndexOffset[iIndex] = oNewBaseOffset;

            // Set local
            oBaseOffset = oNewBaseOffset;
        }

        // Refresh index header
        pIndexHeader = GetIndexHeader (oBaseOffset);

        // Format the new nodes
        size_t stNumNewNodes = (sNewHeapSize - sOldHeapSize) / sizeof (IndexNode);
        Offset oNewFreeList = (Offset) sBucketSize + sOldHeapSize;

        FormatIndexNodes (oBaseOffset, oNewFreeList, stNumNewNodes);

        // Set new chain
        pIndexHeader->oFreeList = oNewFreeList;
    }
    
    // Grab a node off the free list
    Offset oRetOffset = pIndexHeader->oFreeList;
    pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oRetOffset);

    // Set free list to next node
    pIndexHeader->oFreeList = pNode->oNext;

    // Initialize fresh node
    pNode->oNext = NO_OFFSET;
    for (i = 0; i < KEYS_PER_INDEX_NODE; i ++) {
        pNode->piKey[i] = NO_KEY;
    }

    return oRetOffset;
}


void TableContext::FreeIndexNode (Offset oBaseOffset, Offset oNode) {

    FileHeap* pMetaHeap = GetMetaDataHeap();

    IndexHeader* pIndexHeader = GetIndexHeader (oBaseOffset);
    IndexNode* pNode = (IndexNode*) pMetaHeap->GetAddress (oBaseOffset + oNode);
    Assert (pNode->oNext != oNode);

    // Add node to start of free list
    pNode->oNext = pIndexHeader->oFreeList;
    pIndexHeader->oFreeList = oNode;
}