// TableContext.cpp: implementation of the TableContext class.
//
//////////////////////////////////////////////////////////////////////

#define DATABASE_BUILD
#include "TableContext.h"
#undef DATABASE_BUILD

#define INITIAL_INSERTION_LENGTH_ASSUMPTION_SIZE (64)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TableContext::TableContext() {

    m_pTemplate = NULL;
    m_pDatabase = NULL;
    m_pszName = NULL;

    m_oBaseOffset = NO_OFFSET;
    m_oTableHeader = NO_OFFSET;
    m_oFirstRow = NO_OFFSET;
}

TableContext::~TableContext() {

    if (m_pszName != NULL) {
        OS::HeapFree (m_pszName);
    }

    if (m_pTemplate != NULL) {
        m_pTemplate->Release();
        m_pTemplate = NULL;
    }
}

void TableContext::DeleteOnDisk() {

    if (m_oBaseOffset != NO_OFFSET) {
        
        unsigned int i, iNumIndexCols = GetNumIndexColumns();

        // Free all varlen and meta blocks
        FileHeap* pTableHeap = GetTableHeap();
        FileHeap* pMetaHeap = GetMetaDataHeap();
        FileHeap* pVarLenHeap = GetVarLenHeap();

        pTableHeap->Lock();
        pVarLenHeap->Lock();

        // Varlen data
        FreeAllVarLenData();

        pVarLenHeap->Unlock();

        // Bitmap offset
        TableHeader* pTableHeader = (TableHeader*) pTableHeap->GetAddress (m_oTableHeader);

        if (pTableHeader->oRowBitmapOffset != NO_OFFSET) {
            pMetaHeap->Free (pTableHeader->oRowBitmapOffset);
        }

        // Index offsets
        for (i = 0; i < iNumIndexCols; i ++) {

            if (pTableHeader->poIndexOffset[i] != NO_OFFSET) {
                pMetaHeap->Free (pTableHeader->poIndexOffset[i]);
            }
        }       

        pTableHeap->Unlock();

        // Free the table block
        pTableHeap->Free (m_oBaseOffset);
        m_oBaseOffset = NO_OFFSET;
    }
}


void TableContext::FreeAllVarLenData() {

    if (HasVariableLengthData() && GetNumRows() > 0) {

        Offset oOffset;
        
        unsigned int i, iNumCols = GetNumColumns();
        unsigned int* piVarLenCol = (unsigned int*) StackAlloc (iNumCols * sizeof (unsigned int));
        unsigned int iNumVarLenCols = 0;
        
        for (i = 0; i < iNumCols; i ++) {
            
            if (IsVariableLengthString (i)) {
                piVarLenCol [iNumVarLenCols ++] = i;
            }
        }
        
        unsigned int iKey = NO_KEY;
        while (true) {
            
            iKey = FindNextValidRow (iKey);
            if (iKey == NO_KEY) {
                break;
            }

            for (i = 0; i < iNumVarLenCols; i ++) {
                oOffset = *(Offset*) GetData (iKey, piVarLenCol[i]);
                if (oOffset != NO_OFFSET) {
                    FreeVarLen (oOffset);
                }
            }
        }
    }
}

const char* TableContext::GetStringData (unsigned int iKey, unsigned int iColumn) {

    Assert (GetColumnType (iColumn) == V_STRING);

    const char* pszData = (char*) GetData (iKey, iColumn);
    if (GetColumnSize (iColumn) == VARIABLE_LENGTH_STRING) {

        Offset oOffset = *(Offset*) pszData;
        if (oOffset == NO_OFFSET) {
            pszData = "";
        } else {        
            pszData = (char*) GetAddressVarLen (oOffset);
        }
    }

    return pszData;
}


int TableContext::Reload (Offset oTable) {

    size_t stTableNameLen;
    char* pszTableName, * pszTemplateName;

    Assert (oTable != NO_OFFSET);
    Assert (m_oBaseOffset == NO_OFFSET);

    m_oBaseOffset = oTable;

    // Get table name
    pszTableName = (char*) GetTableHeap()->GetAddress (m_oBaseOffset);
    if (pszTableName == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    Assert (m_pszName == NULL);
    m_pszName = String::StrDup (pszTableName);
    if (m_pszName == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    // Get template name
    stTableNameLen = strlen (pszTableName) + 1;
    pszTemplateName = pszTableName + stTableNameLen;

    // Get template (comes back addref'd)
    Assert (m_pTemplate == NULL);

    m_pTemplate = m_pDatabase->FindTemplate (pszTemplateName);
    if (m_pTemplate == NULL) {
        return ERROR_UNKNOWN_TEMPLATE_NAME;
    }

    // Get table header offset
    m_oTableHeader = 
        ALIGN ((m_oBaseOffset + stTableNameLen + strlen (pszTemplateName) + 1), 8);

    // Get first row offset
    m_oFirstRow = 
        ALIGN ((m_oTableHeader + sizeof (TableHeader) + sizeof (Offset) * (GetNumIndexColumns() - 1)), 8);

    return OK;
}

int TableContext::Create (const char* pszTableName, Template* pTemplate) {

    TableHeader* pHeader;

    FileHeap* pTableHeap = GetTableHeap();

    Assert (pszTableName != NULL && pTemplate != NULL);
    Assert (m_pszName == NULL && m_pTemplate == NULL);

    m_pszName = String::StrDup (pszTableName);
    if (m_pszName == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    m_pTemplate = pTemplate;
    m_pTemplate->AddRef();

    //
    // Decide the size of the block we need
    //

    unsigned int i, iNumIndexCols = GetNumIndexColumns();
    const char* pszTemplateName = m_pTemplate->GetName();

    size_t stTableNameLen = strlen (pszTableName) + 1;
    size_t stTemplateNameLen = strlen (pszTemplateName) + 1;

    // Add space for names
    size_t stSize = ALIGN ((stTableNameLen + stTemplateNameLen), 8);
    
    // Add space for 
    stSize += ALIGN ((sizeof (TableHeader) + sizeof (Offset) * (GetNumIndexColumns() - 1)), 8);

    // One row is all we need for starters
    stSize += GetRowSize();

    // Request computed size
    m_oBaseOffset = pTableHeap->Allocate (stSize);
    if (m_oBaseOffset == NO_OFFSET) {
        Assert (false);
        return ERROR_OUT_OF_DISK_SPACE;
    }

    pTableHeap->Lock();

    // Compute other offsets
    m_oTableHeader = ALIGN ((m_oBaseOffset + stTableNameLen + stTemplateNameLen), 8);
    m_oFirstRow = ALIGN ((m_oTableHeader + sizeof (TableHeader) + sizeof (Offset) * (iNumIndexCols - 1)), 8);

    // Write table name
    strncpy (
        (char*) pTableHeap->GetAddress (m_oBaseOffset), 
        pszTableName, 
        stTableNameLen
        );

    // Write template name
    strncpy (
        (char*) pTableHeap->GetAddress (m_oBaseOffset + stTableNameLen), 
        pszTemplateName, 
        stTemplateNameLen
        );

    // Initialize header
    pHeader = (TableHeader*) pTableHeap->GetAddress (
        ALIGN ((m_oBaseOffset + stTableNameLen + stTemplateNameLen), 8)
        );

    pHeader->iNumRows = 0;
    pHeader->iTerminatorRowKey = 0;
    pHeader->oRowBitmapOffset = NO_OFFSET;
    pHeader->iNumBitmapElements = 0;

    for (i = 0; i < iNumIndexCols; i ++) {
        pHeader->poIndexOffset[i] = NO_OFFSET;
    }

    pTableHeap->Unlock();

    return OK;
}

int TableContext::Resize (unsigned int iNumNewRows) {

    size_t stNewSize;
    size_t stOldSize = GetTableHeap()->GetBlockSize (m_oBaseOffset);
    size_t stRowSize = GetRowSize();

    // Heuristic:
    // Add 1/3 to our size, or add iNumNewRows + ten rows, whichever is smallest
    stNewSize = max (stOldSize + stOldSize / 3, stOldSize + (iNumNewRows + 10) * stRowSize);

    // Can't perform memory operations while holding the lock
    GetTableHeap()->Unlock();
    Offset oOffset = GetTableHeap()->Reallocate (m_oBaseOffset, stNewSize);
    GetTableHeap()->Lock();

    if (oOffset == NO_OFFSET) {
        return ERROR_OUT_OF_DISK_SPACE;
    }

    m_oFirstRow = m_oFirstRow - m_oBaseOffset + oOffset;
    m_oTableHeader = m_oTableHeader - m_oBaseOffset + oOffset;

    m_oBaseOffset = oOffset;

    return OK;
}


#define BITS_PER_BITMAP_ELEMENT (sizeof (size_t) * 8)
#define INITIAL_BITMAP_ELEMENTS (5)

int TableContext::ExpandMetaDataIfNecessary (unsigned int iNumNewRows) {

    TableHeader* pTableHeader = GetTableHeader();
    Offset oBitmapOffset = pTableHeader->oRowBitmapOffset;

    //
    // Bitmap
    //

    if (oBitmapOffset == NO_OFFSET) {

        Size sNewSize = 
            max (iNumNewRows / BITS_PER_BITMAP_ELEMENT, INITIAL_BITMAP_ELEMENTS) * sizeof (size_t);
        
        FileHeap* pMetaHeap = GetMetaDataHeap();

        pMetaHeap->Unlock();
        Offset oNewOffset = pMetaHeap->Allocate (sNewSize);
        pMetaHeap->Lock();

        if (oNewOffset == NO_OFFSET) {
            Assert (false);
            return ERROR_OUT_OF_DISK_SPACE;
        }

        pTableHeader->oRowBitmapOffset = oNewOffset;
        pTableHeader->iNumBitmapElements = INITIAL_BITMAP_ELEMENTS;

        // Zero new space
        memset (pMetaHeap->GetAddress (oNewOffset), 0, sNewSize);
    
    } else {

        unsigned int iSize = pTableHeader->iNumBitmapElements;
        unsigned int iNumRows = GetNumRows();

        if (iSize * BITS_PER_BITMAP_ELEMENT <= iNumRows + iNumNewRows) {

            FileHeap* pMetaHeap = GetMetaDataHeap();

            unsigned int iNewSize = (iNumRows + iNumNewRows) / BITS_PER_BITMAP_ELEMENT + INITIAL_BITMAP_ELEMENTS;
            
            // Reallocate space
            pMetaHeap->Unlock();
            Offset oNewOffset = pMetaHeap->Reallocate (oBitmapOffset, iNewSize * sizeof (size_t));
            pMetaHeap->Lock();

            if (oNewOffset == NO_OFFSET) {
                Assert (false);
                return ERROR_OUT_OF_DISK_SPACE;
            }

            // Zero new space
            memset (
                (char*) pMetaHeap->GetAddress (oNewOffset) + iSize * sizeof (size_t), 
                0, 
                (iNewSize - iSize) * sizeof (size_t)
                );

            pTableHeader->oRowBitmapOffset = oNewOffset;
            pTableHeader->iNumBitmapElements = iNewSize;
        }
    }

    //
    // Indexes
    //

    if (GetNumIndexColumns() > 0 && pTableHeader->poIndexOffset[0] == NO_OFFSET) {
        Assert (GetNumRows() == 0 && iNumNewRows > 0);
        return CreateAllIndexes (iNumNewRows);
    }

    return OK;
}


// Will need to be reworked for 64 bit
const size_t SignificantBit[BITS_PER_BITMAP_ELEMENT] = {
    0x00000001,0x00000002,0x00000004,0x00000008,
    0x00000010,0x00000020,0x00000040,0x00000080,
    0x00000100,0x00000200,0x00000400,0x00000800,
    0x00001000,0x00002000,0x00004000,0x00008000,
    0x00010000,0x00020000,0x00040000,0x00080000,
    0x00100000,0x00200000,0x00400000,0x00800000,
    0x01000000,0x02000000,0x04000000,0x08000000,
    0x10000000,0x20000000,0x40000000,0x80000000,
};

size_t* TableContext::GetRowBitmap() {

    Offset oBitmap = GetTableHeader()->oRowBitmapOffset;
    if (oBitmap == NO_OFFSET) {
        return NULL;
    }

    return (size_t*) GetMetaDataHeap()->GetAddress (oBitmap);
}

bool TableContext::IsValidRow (unsigned int iKey) {

    if (iKey >= GetTerminatorRowKey()) {
        return false;
    }

    size_t* pstBits = GetRowBitmap();
    if (pstBits == NULL) {
        return false;
    }

    size_t stBit = iKey % BITS_PER_BITMAP_ELEMENT;
    size_t stIndex = iKey / BITS_PER_BITMAP_ELEMENT;

    return (pstBits[stIndex] & SignificantBit[stBit]) != 0;
}

void TableContext::SetRowValidity (unsigned int iKey, bool bValid) {

    Assert (iKey < GetTerminatorRowKey());

    size_t* pstBits = GetRowBitmap();
    Assert (pstBits != NULL);

    size_t stBit = iKey % BITS_PER_BITMAP_ELEMENT;
    size_t stIndex = iKey / BITS_PER_BITMAP_ELEMENT;

    if (bValid) {

        Assert (!(pstBits[stIndex] & SignificantBit[stBit]));
        pstBits[stIndex] |= SignificantBit[stBit];
    
    } else {
    
        Assert (pstBits[stIndex] & SignificantBit[stBit]);
        pstBits[stIndex] &= ~(SignificantBit[stBit]);
    }
}

void TableContext::SetAllRowsInvalid() {

    TableHeader* pTableHeader = (TableHeader*) GetTableHeap()->GetAddress (m_oTableHeader);

    Offset oBitmap = pTableHeader->oRowBitmapOffset;
    if (oBitmap != NO_OFFSET) {
        
        FileHeap* pMetaHeap = GetMetaDataHeap();

        pMetaHeap->Unlock();
        pMetaHeap->Free (oBitmap);
        pMetaHeap->Lock();

        pTableHeader->oRowBitmapOffset = NO_OFFSET;
        pTableHeader->iNumBitmapElements = 0;
    }
}

unsigned int TableContext::FindNextInvalidRow (unsigned int iKey) {

    size_t* pstBits = GetRowBitmap();
    if (pstBits == NULL) {
        return NO_KEY;
    }

    size_t i, j, stCompBits, stLastBit;

    unsigned int iTerminatorKey = GetTerminatorRowKey();

    if (iKey == NO_KEY) {
        iKey = 0;
    } else {
        iKey ++;
        if (iKey == iTerminatorKey) {
            return NO_KEY;
        }
        Assert (iKey < iTerminatorKey);
    }

    size_t stIndex = iKey / BITS_PER_BITMAP_ELEMENT;
    size_t stBit = iKey % BITS_PER_BITMAP_ELEMENT;
    size_t stLastIndex = iTerminatorKey / BITS_PER_BITMAP_ELEMENT;

    // Exhaust the current block if necessary
    if (stBit != 0) {

        stCompBits = pstBits[stIndex];
        if (stCompBits != 0xffffffff) {

            if (stIndex == stLastIndex) {
                stLastBit = iTerminatorKey % BITS_PER_BITMAP_ELEMENT;
            } else {
                stLastBit = BITS_PER_BITMAP_ELEMENT;
            }
            
            for (j = stBit; j < stLastBit; j ++) {
                
                if (!(stCompBits & SignificantBit[j])) {
                    return (unsigned int) (stIndex * BITS_PER_BITMAP_ELEMENT + j);
                }
            }
        }

        stIndex ++;
    }

    // Now, search the rest of the bits
    for (i = stIndex; i <= stLastIndex; i ++) {

        stCompBits = pstBits[i];
        if (stCompBits != 0xffffffff) {

            if (i == stLastIndex) {
                stLastBit = iTerminatorKey % BITS_PER_BITMAP_ELEMENT;
            } else {
                stLastBit = BITS_PER_BITMAP_ELEMENT;
            }

            for (j = 0; j < stLastBit; j ++) {

                if (!(stCompBits & SignificantBit[j])) {
                    return (unsigned int) (i * BITS_PER_BITMAP_ELEMENT + j);
                }
            }
        }
    }

    return NO_KEY;
}


unsigned int TableContext::FindNextValidRow (unsigned int iKey) {

    size_t* pstBits = GetRowBitmap();
    if (pstBits == NULL) {
        return NO_KEY;
    }

    size_t i, j, stCompBits, stLastBit;

    unsigned int iTerminatorKey = GetTerminatorRowKey();

    if (iKey == NO_KEY) {
        iKey = 0;
    } else {
        iKey ++;
        if (iKey == iTerminatorKey) {
            return NO_KEY;
        }
        Assert (iKey < iTerminatorKey);
    }

    size_t stIndex = iKey / BITS_PER_BITMAP_ELEMENT;
    size_t stBit = iKey % BITS_PER_BITMAP_ELEMENT;
    size_t stLastIndex = iTerminatorKey / BITS_PER_BITMAP_ELEMENT;

    // Exhaust the current block if necessary
    if (stBit != 0) {

        stCompBits = pstBits[stIndex];
        if (stCompBits != 0) {

            if (stIndex == stLastIndex) {
                stLastBit = iTerminatorKey % BITS_PER_BITMAP_ELEMENT;
            } else {
                stLastBit = BITS_PER_BITMAP_ELEMENT;
            }
            
            for (j = stBit; j < stLastBit; j ++) {
                
                if (stCompBits & SignificantBit[j]) {
                    return (unsigned int) (stIndex * BITS_PER_BITMAP_ELEMENT + j);
                }
            }
        }

        stIndex ++;
    }

    // Now, search the rest of the bits
    for (i = stIndex; i <= stLastIndex; i ++) {

        stCompBits = pstBits[i];
        if (stCompBits != 0) {

            if (i == stLastIndex) {
                stLastBit = iTerminatorKey % BITS_PER_BITMAP_ELEMENT;
            } else {
                stLastBit = BITS_PER_BITMAP_ELEMENT;
            }

            for (j = 0; j < stLastBit; j ++) {

                if (stCompBits & SignificantBit[j]) {
                    return (unsigned int) (i * BITS_PER_BITMAP_ELEMENT + j);
                }
            }
        }
    }

    return NO_KEY;
}

int AppendKey (unsigned int*& piKey, unsigned int& iNumKeys, unsigned int& iKeySpace, int iRowKey) {

    if (iNumKeys == iKeySpace) {

        iKeySpace = iNumKeys == 0 ? 10 : iNumKeys * 2;
        
        unsigned int* piTemp = new unsigned int [iKeySpace];
        if (piTemp == NULL) {
            
            if (piKey != NULL) {
                delete [] piKey;
                piKey = NULL;
            }

            return ERROR_OUT_OF_MEMORY;
        }
        
        if (iNumKeys > 0) {
            memcpy (piTemp, piKey, iNumKeys * sizeof (unsigned int));
        }
        
        if (piKey != NULL) delete [] piKey;
        piKey = piTemp;
    }

    piKey[iNumKeys] = iRowKey;

    return OK;
}