//
// Database.dll - A database library
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include <stdio.h>

#define FILE_DATABASE_BUILD
#include "Template.h"
#include "TableContext.h"
#undef FILE_DATABASE_BUILD

#include "Osal/File.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Template::Template (Database* pDatabase) {
    
    m_iNumRefs = 1;

    m_pDatabase = pDatabase;
    m_bDelete = false;

    memset (&TemplateData, 0, sizeof (TemplateData));

    RowSize = 0;
    ColOffset = NULL;
    m_pDataBlock = NULL;

    HasVariableLengthData = false;
    HasUniqueDataIndex = false;

    m_oBaseOffset = NO_OFFSET;
}

Template::~Template() {

    if (m_bDelete && m_pDatabase != NULL && m_oBaseOffset != NO_OFFSET) {
        m_pDatabase->GetTemplateFileHeap()->Free (m_oBaseOffset);
    }

    if (m_pDataBlock != NULL) {
        OS::HeapFree (m_pDataBlock);
    }
}

bool Template::IsEqual (const FileTemplateDescription& ttTemplate) {

    unsigned int i;

    if (String::StriCmp (TemplateData.Name, ttTemplate.Name) != 0) {
        return false;
    }

    if (TemplateData.NumColumns != ttTemplate.NumColumns) {
        return false;
    }

    if (TemplateData.OneRow != ttTemplate.OneRow) {
        return false;
    }

    if (TemplateData.NumIndexes != ttTemplate.NumIndexes) {
        return false;
    }

    if (TemplateData.NumRowsGuess != ttTemplate.NumRowsGuess) {
        return false;
    }

    for (i = 0; i < TemplateData.NumIndexes; i ++) {
        
        if (TemplateData.IndexColumn[i] != ttTemplate.IndexColumn[i]) {
            return false;
        }
        
        if (TemplateData.IndexFlags[i] != ttTemplate.IndexFlags[i]) {
            return false;
        }
    }

    for (i = 0; i < TemplateData.NumColumns; i ++) {

        if (TemplateData.Type[i] != ttTemplate.Type[i]) {
            return false;
        }

        if (TemplateData.Type[i] == V_STRING) {
            
            if (TemplateData.Size[i] == VARIABLE_LENGTH_STRING) {
                
                if (ttTemplate.Size[i] != VARIABLE_LENGTH_STRING) {
                    return false;
                }
            
            } else {

                if ((size_t)TemplateData.Size[i] != ALIGN (ttTemplate.Size[i], 8)) {
                    return false;
                }
            }
        }
    }

    return true;
}

int Template::GetDescription (FileTemplateDescription* pttTemplate) {

    memcpy (pttTemplate, &TemplateData, sizeof (FileTemplateDescription));

    return OK;
}

int Template::Reload (Offset oTemplate) {

    int iErrCode = OK;
    FileHeap* pTemplateHeap = NULL;

    char* pszName, * pAddress;
    size_t stNameLen, stLen, stBlockSize = 0;

    unsigned int* piIndices = NULL, * piIndexFlags = NULL;
    VariantType* pvtTypes;
    int64* pcbSizes;

    // Save input offset
    Assert (oTemplate != NO_OFFSET);

    m_oBaseOffset = oTemplate;

    pTemplateHeap = m_pDatabase->GetTemplateFileHeap();
    Assert (pTemplateHeap != NULL);

    m_pDatabase->GetHeapLock()->WaitReader();

    //
    // Calculate size of block needed, copy simple data members
    //

    // Name
    pAddress = (char*) pTemplateHeap->GetAddress (oTemplate);
    if (pAddress == NULL || *pAddress == '\0') {
        iErrCode = ERROR_UNKNOWN_TEMPLATE_NAME;
        goto Cleanup;
    }

    stNameLen = strlen (pAddress) + 1;
    
    pszName = pAddress;
    pAddress += stNameLen;
    
    stBlockSize += stNameLen;

    // OneRow
    pAddress = (char*) ALIGN (pAddress, sizeof (TemplateData.OneRow));
    TemplateData.OneRow = *(bool*) pAddress;
    pAddress += sizeof (TemplateData.OneRow);

    // IndexColumns
    pAddress = (char*) ALIGN (pAddress, sizeof (TemplateData.NumIndexes));
    TemplateData.NumIndexes = *(unsigned int*) pAddress;

    pAddress += sizeof (TemplateData.NumIndexes);

    if (TemplateData.NumIndexes > 0) {

        pAddress = (char*) ALIGN (pAddress, sizeof (unsigned int));
        stLen = TemplateData.NumIndexes * sizeof (unsigned int) * 2;

        piIndices = (unsigned int*) pAddress;
        piIndexFlags = piIndices + TemplateData.NumIndexes;
        pAddress += stLen;

        stBlockSize = ALIGN (stBlockSize, sizeof (unsigned int));
        stBlockSize += stLen;
    }

    // Column data
    pAddress = (char*) ALIGN (pAddress, sizeof (TemplateData.NumColumns));
    TemplateData.NumColumns = *(unsigned int*) pAddress;
    pAddress += sizeof (TemplateData.NumColumns);

    if (TemplateData.NumColumns == 0) {
        pvtTypes = NULL;
        pcbSizes = NULL;
    } else {

        // Types
        pAddress = (char*) ALIGN (pAddress, sizeof (VariantType));
        stLen = TemplateData.NumColumns * sizeof (VariantType);

        pvtTypes = (VariantType*) pAddress;
        pAddress += stLen;

        stBlockSize = ALIGN (stBlockSize, sizeof (VariantType));
        stBlockSize += stLen;

        // Sizes
        pAddress = (char*) ALIGN (pAddress, sizeof (int64));
        stLen = TemplateData.NumColumns * sizeof (int64);

        pcbSizes = (int64*) pAddress;
        pAddress += stLen;

        stBlockSize = ALIGN (stBlockSize, sizeof (int64));
        stBlockSize += stLen;
    }

    // NumRowsGuess
    pAddress = (char*) ALIGN (pAddress, sizeof (TemplateData.NumRowsGuess));
    TemplateData.NumRowsGuess = *(unsigned int*) pAddress;
    pAddress += sizeof (TemplateData.NumRowsGuess);

    iErrCode = FinalConstruct (
        stBlockSize,
        pszName,
        stNameLen,
        piIndices,
        piIndexFlags,
        pvtTypes,
        pcbSizes
        );

Cleanup:

    m_pDatabase->GetHeapLock()->SignalReader();

    return iErrCode;

}

int Template::FinalConstruct (size_t stBlockSize, const char* pszName, size_t stNameLen,
                              unsigned int* piIndices, unsigned int* piIndexFlags, VariantType* pvtTypes, 
                              int64* pcbSizes) {

    int iErrCode = OK;
    unsigned int i;

    char* pAddress;
    size_t stLen;

    // Add space for column offsets
    stBlockSize = ALIGN (stBlockSize, sizeof (int64));
    stBlockSize += TemplateData.NumColumns * sizeof (int64);

    // Allocate the size block
    // We do this because the on-disk block might change its address,
    // so we can't just save the pointers
    // We use this data a lot during runtime, so it's important to have it in memory

    m_pDataBlock = OS::HeapAlloc (stBlockSize);
    if (m_pDataBlock ==  NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    pAddress = (char*) m_pDataBlock;

    // Copy data
    TemplateData.Name = (char*) pAddress;
    memcpy (TemplateData.Name, pszName, stNameLen);

    pAddress += stNameLen;

    if (TemplateData.NumIndexes == 0) {
        TemplateData.IndexColumn = NULL;
    } else {

        pAddress = (char*) ALIGN (pAddress, sizeof (unsigned int));
        stLen = TemplateData.NumIndexes * sizeof (unsigned int);

        TemplateData.IndexColumn = (unsigned int*) pAddress;
        memcpy (TemplateData.IndexColumn, piIndices, stLen);

        pAddress += stLen;

        TemplateData.IndexFlags = (unsigned int*) pAddress;
        memcpy (TemplateData.IndexFlags, piIndexFlags, stLen);

        pAddress += stLen;
    }

    if (TemplateData.NumColumns == 0) {
        
        TemplateData.Type = NULL;
        TemplateData.Size = NULL;

    } else {

        pAddress = (char*) ALIGN (pAddress, sizeof (VariantType));
        stLen = TemplateData.NumColumns * sizeof (VariantType);

        TemplateData.Type = (VariantType*) pAddress;
        memcpy (TemplateData.Type, pvtTypes, stLen);

        pAddress += stLen;

        pAddress = (char*) ALIGN (pAddress, sizeof (int64));
        stLen = TemplateData.NumColumns * sizeof (int64);

        TemplateData.Size = (int64*) pAddress;
        for (i = 0; i < TemplateData.NumColumns; i ++) {

            if (TemplateData.Type[i] == V_STRING && pcbSizes[i] != VARIABLE_LENGTH_STRING) {
                TemplateData.Size[i] = ALIGN (pcbSizes[i], 8);
            } else {
                TemplateData.Size[i] = pcbSizes[i];
            }
        }
        pAddress += stLen;
    }

    // Verify the final template
    iErrCode = VerifyTemplate (TemplateData);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Calculate Offsets and RowSize
    ColOffset = (Offset*) ALIGN (pAddress, sizeof (Offset));

    RowSize = 0;
    for (i = 0; i < TemplateData.NumColumns; i ++) {
        
        ColOffset[i] = RowSize;

        if (TemplateData.Size[i] == VARIABLE_LENGTH_STRING) {

            // Align if necessary
            RowSize += RowSize % sizeof (Offset);
            RowSize += sizeof (Offset);

            HasVariableLengthData = true;

        } else {

            if (TemplateData.Type[i] != V_STRING) {

                // Align if necessary
                RowSize += RowSize % TemplateData.Size[i];
            }

            RowSize += TemplateData.Size[i];
        }
    }

    for (i = 0; i < TemplateData.NumIndexes; i ++) {

        if (TemplateData.IndexFlags[i] & INDEX_UNIQUE_DATA) {
            HasUniqueDataIndex = true;
            break;
        }
    }

Cleanup:

    return iErrCode;
}


int Template::Create (const FileTemplateDescription& ttTemplate) {

    unsigned int i;
    size_t stSize = 0, stNameLen, stLen;
    int64* pcbSizes;

    // Verify data
    int iErrCode = VerifyTemplate (ttTemplate);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Copy simple member data
    TemplateData.OneRow = ttTemplate.OneRow;
    TemplateData.NumRowsGuess = ttTemplate.NumRowsGuess;
    TemplateData.NumColumns = ttTemplate.NumColumns;
    TemplateData.NumIndexes = ttTemplate.NumIndexes;

    // Name
    stNameLen = strlen (ttTemplate.Name) + 1;
    stSize += stNameLen;

    // Index columns, flags
    stSize += ttTemplate.NumIndexes * sizeof (unsigned int) * 2;

    // Sizes
    stSize += ttTemplate.NumColumns * sizeof (int64);

    // Types
    stSize += ttTemplate.NumColumns * sizeof (VariantType);

    // Create size array
    pcbSizes = (int64*) StackAlloc (ttTemplate.NumColumns * sizeof(int64));

    for (i = 0; i < ttTemplate.NumColumns; i ++) {

        switch (ttTemplate.Type[i]) {
        
        case V_STRING:

            if (ttTemplate.Size[i] == VARIABLE_LENGTH_STRING) {
                pcbSizes[i] = VARIABLE_LENGTH_STRING;
            } else {
                pcbSizes[i] = ttTemplate.Size[i];
            }
            break;

        case V_INT64:
            pcbSizes[i] = sizeof (int64);
            break;

        case V_INT:
            pcbSizes[i] = sizeof (int);
            break;

        case V_FLOAT:
            pcbSizes[i] = sizeof (float);
            break;

        default:
            Assert (false);
            return ERROR_TYPE_MISMATCH;
        }
    }

    // Finish initializing our template data
    iErrCode = FinalConstruct (
        stSize,
        ttTemplate.Name,
        stNameLen,
        ttTemplate.IndexColumn,
        ttTemplate.IndexFlags,
        ttTemplate.Type,
        pcbSizes
        );

    if (iErrCode != OK) {
        return iErrCode;
    }

    // Create memory block in heap
    FileHeap* pTemplateHeap = m_pDatabase->GetTemplateFileHeap();
    Assert (pTemplateHeap != NULL);

    m_oBaseOffset = pTemplateHeap->Allocate (stSize + sizeof (FileTemplateDescription));
    if (m_oBaseOffset == NO_OFFSET) {
        return ERROR_OUT_OF_MEMORY;
    }

    char* pAddress = (char*) pTemplateHeap->GetAddress (m_oBaseOffset);
    Assert (pAddress != NULL);

    // Name
    memcpy (pAddress, TemplateData.Name, stNameLen);
    pAddress += stNameLen;

    // OneRow
    pAddress = (char*) ALIGN (pAddress, sizeof (bool));
    *(bool*) pAddress = TemplateData.OneRow;
    pAddress += sizeof (bool);

    // IndexColumns
    pAddress = (char*) ALIGN (pAddress, sizeof (unsigned int));
    *(unsigned int*) pAddress = TemplateData.NumIndexes;
    pAddress += sizeof (unsigned int);

    if (TemplateData.NumIndexes > 0) {

        pAddress = (char*) ALIGN (pAddress, sizeof (unsigned int));

        stLen = TemplateData.NumIndexes * sizeof (unsigned int);
        memcpy (pAddress, TemplateData.IndexColumn, stLen);
        pAddress += stLen;

        pAddress = (char*) ALIGN (pAddress, sizeof (unsigned int));
        memcpy (pAddress, TemplateData.IndexFlags, stLen);
        pAddress += stLen;
    }

    // NumCols, Types, Sizes
    pAddress = (char*) ALIGN (pAddress, sizeof (unsigned int));
    *(unsigned int*) pAddress = TemplateData.NumColumns;
    pAddress += sizeof (unsigned int);

    if (TemplateData.NumColumns > 0) {

        pAddress = (char*) ALIGN (pAddress, sizeof (VariantType));

        stLen = TemplateData.NumColumns * sizeof (VariantType);
        memcpy (pAddress, TemplateData.Type, stLen);
        pAddress += stLen;

        pAddress = (char*) ALIGN (pAddress, sizeof (int64));

        stLen = TemplateData.NumColumns * sizeof (int64);
        memcpy (pAddress, TemplateData.Size, stLen);
        pAddress += stLen;
    }

    // NumRowsGuess
    pAddress = (char*) ALIGN (pAddress, sizeof (unsigned int));
    *(unsigned int*) pAddress = TemplateData.NumRowsGuess;

    return OK;
}

int Template::VerifyTemplate (const FileTemplateDescription& ttTemplate) {

    int iErrCode;

    // Bad name
    if (ttTemplate.Name == NULL || *ttTemplate.Name == '\0') {
        return ERROR_BLANK_TEMPLATE_NAME;
    }

    // No columns
    if (ttTemplate.NumColumns == 0) {
        return ERROR_NO_COLUMNS;
    }

    // OneRow with indices
    if (ttTemplate.OneRow && ttTemplate.NumIndexes > 0) {
        return ERROR_ONE_ROW_CANNOT_BE_INDEXED;
    }

    // Verify indices
    iErrCode = VerifyIndices (ttTemplate);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Bad types
    if (HasBadTypes (ttTemplate.NumColumns, ttTemplate.Type)) {
        return ERROR_INVALID_TYPE;
    }

    if (ttTemplate.NumRowsGuess == 0) {
        return ERROR_NOT_ENOUGH_ROWS;
    }

    // Too many rows in hint
    if (ttTemplate.NumRowsGuess > MAX_NUM_INDEX_BUCKETS) {
        return ERROR_TOO_MANY_ROWS;
    }

    return OK;
}

int Template::VerifyIndices (const FileTemplateDescription& ttTemplate) {

    unsigned int i, iNumIndexes = ttTemplate.NumIndexes;

    if (iNumIndexes == 0) {
        return OK;
    }

    // Verify sorted and no duplicates
    for (i = 1; i < iNumIndexes; i ++) {

        if (ttTemplate.IndexColumn[i - 1] > ttTemplate.IndexColumn[i]) {
            return ERROR_UNSORTED_INDEX_COLUMNS;
        }

        if (ttTemplate.IndexColumn[i - 1] == ttTemplate.IndexColumn[i]) {
            return ERROR_REPEATED_INDEX_COLUMNS;
        }
    }

    // Verify flags match types
    for (i = 0; i < iNumIndexes; i ++) {

        if ((ttTemplate.IndexFlags[i] & INDEX_CASE_SENSITIVE) &&
            ttTemplate.Type [ttTemplate.IndexColumn[i]] != V_STRING) {

            return ERROR_INDEX_COLUMN_TYPE_MISMATCH;
        }
    }

    return OK;
}

bool Template::HasBadTypes (unsigned int iNumColumns, VariantType* pvtType) {

    unsigned int i;

    for (i = 0; i < iNumColumns; i ++) {
        if (pvtType[i] < V_STRING || pvtType[i] > V_INT64) {
            return true;
        }
    }

    return false;
}