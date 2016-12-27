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

#include <stdio.h>

#define DATABASE_BUILD
#include "Template.h"
#include "CDatabase.h"
#include "DatabaseStrings.h"
#undef DATABASE_BUILD

#include "Osal/File.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Template::Template (const char* pszName, Database* pDatabase) {
    
    TemplateData.Name = String::StrDup (pszName);

    m_pDatabase = pDatabase;
    m_bDelete = false;

    m_iNumRefs = 1;
    
    TemplateData.NumColumns = 0;
    TemplateData.Type = NULL;
    TemplateData.Size = NULL;
    TemplateData.OneRow = false;
    TemplateData.NumIndexes = 0;
    TemplateData.IndexColumn = NULL;

    RowSize = 0;
    Offset = NULL;

    VariableLengthRows = false;
    NumInitialInsertionLengthRows = 0;
}

Template::Template (const TemplateDescription& ttTemplate, Database* pDatabase) {

    unsigned int i;

    m_pDatabase = pDatabase;
    m_bDelete = false;

    m_iNumRefs = 1;

    TemplateData.Name = String::StrDup (ttTemplate.Name);

    TemplateData.OneRow = ttTemplate.OneRow;
    TemplateData.NumColumns = ttTemplate.NumColumns;
    TemplateData.IndexColumn = ttTemplate.IndexColumn;

    // Copy sizes
    TemplateData.Size = new size_t [TemplateData.NumColumns];
    memcpy (TemplateData.Size, ttTemplate.Size, sizeof (size_t) * TemplateData.NumColumns);

    // Copy types
    TemplateData.Type = new VariantType [TemplateData.NumColumns];
    memcpy (TemplateData.Type, ttTemplate.Type, sizeof (VariantType) * TemplateData.NumColumns);

    Offset = new size_t [TemplateData.NumColumns];

    // Calculate row sizes, offsets and total row size
    NumInitialInsertionLengthRows = 0;

    RowSize = sizeof (ROW_HEADER);
    for (i = 0; i < TemplateData.NumColumns; i ++) {
        
        Offset[i] = RowSize;
        
        switch (TemplateData.Type[i]) {
        
        case V_STRING:

            if (TemplateData.Size[i] == INITIAL_INSERTION_LENGTH) {

                VariableLengthRows = true;
                NumInitialInsertionLengthRows ++;

            } else {

                // Add NULL char and align
                TemplateData.Size[i] = ALIGN (TemplateData.Size[i] + 1, MAX_ELEMENT_SIZE);
                RowSize += TemplateData.Size[i];
            }

            break;

        case V_INT64:

            TemplateData.Size[i] = 8;
            RowSize += 8;
            break;

        default:

            // Use 32 bits for all other values
            TemplateData.Size[i] = BASE_ELEMENT_SIZE;
            RowSize += BASE_ELEMENT_SIZE;
            break;
        }
    }

    VariableLengthRows = NumInitialInsertionLengthRows > 0;

    TemplateData.NumIndexes = ttTemplate.NumIndexes;
    
    if (TemplateData.NumIndexes > 0) {

        TemplateData.IndexColumn = new unsigned int [TemplateData.NumIndexes];

        for (i = 0; i < TemplateData.NumIndexes; i ++) {
            TemplateData.IndexColumn[i] = ttTemplate.IndexColumn[i];
        }

    } else {
        TemplateData.IndexColumn = NULL;
    }

    TemplateData.NumRowsGuess = ttTemplate.NumRowsGuess;
}

Template::~Template() {

    if (TemplateData.Type != NULL) {
        delete [] TemplateData.Type;
    }

    if (Offset != NULL) {
        delete [] Offset;
    }

    if (TemplateData.Size != NULL) {
        delete [] TemplateData.Size;
    }

    if (m_bDelete) {

        // Delete template directory with all its tables!
        char pszFileName [OS::MaxFileNameLength];

        sprintf (pszFileName, "%s/" DATA_DIRECTORY "/%s", m_pDatabase->GetDirectory(), TemplateData.Name);
        File::DeleteDirectory (pszFileName);
    }

    if (TemplateData.Name != NULL) {
        OS::HeapFree (TemplateData.Name);
    }

    if (TemplateData.IndexColumn != NULL) {
        delete [] TemplateData.IndexColumn;
    }
}

Template* Template::CreateInstance (const char* pszName, Database* pDatabase) {
    return new Template (pszName, pDatabase);
}

Template* Template::CreateInstance (const TemplateDescription& ttTemplate, Database* pDatabase) {

    return new Template (ttTemplate, pDatabase);
}


bool Template::IsEqual (const TemplateDescription& ttTemplate) {

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

    unsigned int i;

    for (i = 0; i < TemplateData.NumIndexes; i ++) {
        if (TemplateData.IndexColumn[i] != ttTemplate.IndexColumn[i]) {
            return false;
        }
    }

    for (i = 0; i < TemplateData.NumColumns; i ++) {

        if (TemplateData.Type[i] != ttTemplate.Type[i]) {
            return false;
        }

        if (TemplateData.Type[i] == V_STRING) {
            
            if (TemplateData.Size[i] == INITIAL_INSERTION_LENGTH) {
                if (ttTemplate.Size[i] != INITIAL_INSERTION_LENGTH) {
                    return false;
                }
            } else {
                if (TemplateData.Size[i] != ALIGN (ttTemplate.Size[i] + 1, MAX_ELEMENT_SIZE)) {
                    return false;
                }
            }
        }
    }

    return true;
}

int Template::GetDescription (TemplateDescription* pttTemplate) {

    memcpy (pttTemplate, &TemplateData, sizeof (TemplateDescription));

    return OK;
}

int Template::Reload (const char* pszFileName) {

    File fTemplate;

    int iErrCode = fTemplate.OpenRead (pszFileName);
    if (iErrCode != OK) {
        return iErrCode;
    }

    size_t stNumBytes;

    // OneRow
    iErrCode = fTemplate.Read (&TemplateData.OneRow, sizeof (bool), &stNumBytes);
    if (iErrCode != OK || stNumBytes != sizeof (bool)) {
        fTemplate.Close();
        return iErrCode;
    }

    // IndexColumns
    iErrCode = fTemplate.Read (&TemplateData.NumIndexes, sizeof (unsigned int), &stNumBytes);
    if (iErrCode != OK || stNumBytes != sizeof (unsigned int)) {
        fTemplate.Close();
        return iErrCode;
    }
    
    if (TemplateData.NumIndexes > 0) {
        TemplateData.IndexColumn = new unsigned int [TemplateData.NumIndexes];
        iErrCode = fTemplate.Read (TemplateData.IndexColumn, TemplateData.NumIndexes * sizeof (unsigned int), &stNumBytes);
        if (iErrCode != OK || stNumBytes != TemplateData.NumIndexes * sizeof (unsigned int)) {
            fTemplate.Close();
            return iErrCode;
        }
    }

    // NumCols
    iErrCode = fTemplate.Read (&TemplateData.NumColumns, sizeof (int), &stNumBytes);
    if (iErrCode != OK || stNumBytes != sizeof (int)) {
        fTemplate.Close();
        return iErrCode;
    }

    // Types
    TemplateData.Type = new VariantType [TemplateData.NumColumns];
    iErrCode = fTemplate.Read (TemplateData.Type, TemplateData.NumColumns * sizeof (VariantType), &stNumBytes);
    if (iErrCode != OK || stNumBytes != TemplateData.NumColumns * sizeof (VariantType)) {
        fTemplate.Close();
        return iErrCode;
    }

    // Sizes
    TemplateData.Size = new size_t [TemplateData.NumColumns];
    iErrCode = fTemplate.Read (TemplateData.Size, TemplateData.NumColumns * sizeof (size_t), &stNumBytes);
    if (iErrCode != OK || stNumBytes != TemplateData.NumColumns * sizeof (size_t)) {
        fTemplate.Close();
        return iErrCode;
    }

    iErrCode = fTemplate.Read (&TemplateData.NumRowsGuess, sizeof (unsigned int), &stNumBytes);
    if (iErrCode != OK || stNumBytes != sizeof (unsigned int)) {
        fTemplate.Close();
        return iErrCode;
    }

    // Verify
    iErrCode = VerifyTemplate (TemplateData);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Calculate Offsets and RowSize
    NumInitialInsertionLengthRows = 0;

    Offset = new size_t [TemplateData.NumColumns];

    RowSize = sizeof (ROW_HEADER);
    for (unsigned int i = 0; i < TemplateData.NumColumns; i ++) {
        
        Offset[i] = RowSize;

        if (TemplateData.Size[i] == INITIAL_INSERTION_LENGTH) {
            NumInitialInsertionLengthRows ++;
        } else {
            
            // Assume size was written correctly
            RowSize += TemplateData.Size[i];
        }
    }

    VariableLengthRows = NumInitialInsertionLengthRows > 0;

    fTemplate.Close();
    return OK;
}


int Template::Create (const char* pszDirName) {

    char pszCreate [OS::MaxFileNameLength];
    strcpy (pszCreate, pszDirName);

    // Create directory 
    int iErrCode;
    if (!File::DoesDirectoryExist (pszCreate) && File::CreateDirectory (pszCreate) != OK) {
        return ERROR_COULD_NOT_CREATE_TEMPLATE_DIRECTORY;
    }

    strcat (pszCreate, TEMPLATE_DATA_FILE);
    if (File::DoesFileExist (pszCreate)) {
        return ERROR_TEMPLATE_ALREADY_EXISTS;
    }

    File fTemplate;

    iErrCode = fTemplate.OpenWrite (pszCreate);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // OneRow
    iErrCode = fTemplate.Write (&TemplateData.OneRow, sizeof (bool));
    if (iErrCode != OK) {
        fTemplate.Close();
        return iErrCode;
    }

    // IndexColumns
    iErrCode = fTemplate.Write (&TemplateData.NumIndexes, sizeof (unsigned int));
    if (iErrCode != OK) {
        fTemplate.Close();
        return iErrCode;
    }

    if (TemplateData.NumIndexes > 0) {
        iErrCode = fTemplate.Write (TemplateData.IndexColumn, TemplateData.NumIndexes * sizeof (unsigned int));
        if (iErrCode != OK) {
            fTemplate.Close();
            return iErrCode;
        }
    }

    // NumCols
    iErrCode = fTemplate.Write (&TemplateData.NumColumns, sizeof (int));
    if (iErrCode != OK) {
        fTemplate.Close();
        return iErrCode;
    }

    // Types
    iErrCode = fTemplate.Write (TemplateData.Type, TemplateData.NumColumns * sizeof (VariantType));
    if (iErrCode != OK) {
        fTemplate.Close();
        return iErrCode;
    }

    // Sizes
    iErrCode = fTemplate.Write (TemplateData.Size, TemplateData.NumColumns * sizeof (size_t));
    if (iErrCode != OK) {
        fTemplate.Close();
        return iErrCode;
    }

    iErrCode = fTemplate.Write (&TemplateData.NumRowsGuess, sizeof (unsigned int));
    if (iErrCode != OK) {
        fTemplate.Close();
        return iErrCode;
    }
    
    fTemplate.Close();

    // Create the table subdirectory
    const char* pszTableDir = m_pDatabase->GetDataDirectory();

    strcpy (pszCreate, pszTableDir);
    strcat (pszCreate, TemplateData.Name);

    if (!File::DoesDirectoryExist (pszCreate) && File::CreateDirectory (pszCreate) != OK) {
        return ERROR_COULD_NOT_CREATE_TEMPLATE_TABLE_DIRECTORY;
    }

    return iErrCode;
}

const char* Template::GetDirectory() {
    return m_pDatabase->GetDirectory();
}

const char* Template::GetDataDirectory() {
    return m_pDatabase->GetDataDirectory();
}

void Template::IncrementTableSizeOnDisk (size_t stChange) {
    m_pDatabase->IncrementSizeOnDisk (stChange);
}

void Template::DecrementTableSizeOnDisk (size_t stChange) {
    m_pDatabase->DecrementSizeOnDisk (stChange);
}

void Template::IncrementNumLoadedRows (unsigned int iNumRows) {
    m_pDatabase->IncrementNumLoadedRows (iNumRows);
}

void Template::DecrementNumLoadedRows (unsigned int iNumRows) {
    m_pDatabase->DecrementNumLoadedRows (iNumRows);
}

int Template::Backup (const char* pszBackupDir) {

    char pszBackup [OS::MaxFileNameLength];
    char pszBackupSrc  [OS::MaxFileNameLength];

    sprintf (pszBackupSrc, "%s%s/" TEMPLATE_DATA_FILE, m_pDatabase->GetDataDirectory(), TemplateData.Name);

    // Best effort createdir, since it might already exist
    sprintf (pszBackup, "%s/%s/", pszBackupDir, TemplateData.Name);
    File::CreateDirectory (pszBackup);

    strcat (pszBackup, TEMPLATE_DATA_FILE);
    return File::CopyFile (pszBackupSrc, pszBackup);
}

int Template::VerifyTemplate (const TemplateDescription& ttTemplate) {

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

    // Repeated index columns
    if (ttTemplate.NumIndexes != 0 && 
        HasRepeatedIndices (ttTemplate.NumIndexes, ttTemplate.IndexColumn)) {
        return ERROR_REPEATED_INDEX_COLUMNS;
    }

    // Bad types
    if (HasBadTypes (ttTemplate.NumColumns, ttTemplate.Type)) {
        return ERROR_INVALID_TYPE;
    }

    return OK;
}

bool Template::HasRepeatedIndices (unsigned int iNumIndexes, unsigned int* piColumns) {

    unsigned int* piCopyCols = (unsigned int*) StackAlloc (iNumIndexes * sizeof (unsigned int));
    memcpy (piCopyCols, piColumns, iNumIndexes * sizeof (unsigned int));

    Algorithm::QSortAscending<unsigned int> (piCopyCols, iNumIndexes);

    unsigned int i;
    for (i = 1; i < iNumIndexes; i ++) {

        if (piCopyCols[i] == piCopyCols[i - 1]) {
            return true;
        }
    }

    return false;
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

unsigned int Template::GetOptions() {
    return m_pDatabase->GetOptions();
}