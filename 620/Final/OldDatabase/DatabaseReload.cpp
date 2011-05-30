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
#include "CDatabase.h"
#include "Table.h"
#include "Backup.h"
#undef DATABASE_BUILD

int Database::Reload() {

    int iErrCode = ReloadTemplates();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = ReloadTables();
    if (iErrCode != OK) {
        return iErrCode;
    }

    return ReloadBackups();
}

int Database::ReloadTemplates() {

    Template* pTemplate;
    char pszDataFile [OS::MaxFileNameLength];

    // Get all directory names
    unsigned int i;
    bool bAdded = true;

    FileEnumerator fEnum;
    int iErrCode = File::EnumerateFiles ((String) m_pszDataDirectory + "*", &fEnum);
    if (iErrCode != OK) {
        return iErrCode;
    }

    const char** ppszFileName = fEnum.GetFileNames();
    unsigned int iNumFiles = fEnum.GetNumFiles();

    for (i = 0; i < iNumFiles; i ++) {

        bAdded = false;

        if (File::GetFileType ((String) m_pszDataDirectory + ppszFileName[i]) == FILETYPE_DIRECTORY) {

            // Does template.dat exist?
            sprintf (pszDataFile, "%s%s/" TEMPLATE_DATA_FILE, m_pszDataDirectory, ppszFileName[i]);
            if (File::DoesFileExist (pszDataFile)) {

                // Create a new Template
                pTemplate = Template::CreateInstance (ppszFileName[i], this);
                
                // Try to reload Template.dat and add to template hash table
                if (pTemplate->Reload (pszDataFile) == OK) {

                    // This must succeed
                    bAdded = m_htTemplates.Insert (pTemplate->GetName(), pTemplate);
                    Assert (bAdded);

                    pTemplate->AddRef();

                    size_t stSize;
                    iErrCode = File::GetFileSize (pszDataFile, &stSize);
                    Assert (iErrCode == OK);
                        
                    IncrementSizeOnDisk (stSize);
                }

                pTemplate->Release();
            }
        }
    }

    return m_htTemplates.GetNumElements() > 0 ? OK : WARNING;
}

int Database::ReloadTables() {

    Table* pTable;
    Template* pTemplate;
    const char* pszTemplateName;

    char pszDataFile [OS::MaxFileNameLength];

    // Get all template names
    const char** ppszSubFileName;
    unsigned int i, iNumSubFiles;
    int iErrCode;
    bool bAdded;

    FileEnumerator fEnum;

    HashTableIterator<const char*, Template*> hitIterator;

    while (m_htTemplates.GetNextIterator (&hitIterator)) {

        pTemplate = hitIterator.GetData();
        pTemplate->AddRef();

        pszTemplateName = hitIterator.GetKey();

        sprintf (pszDataFile, "%s%s", m_pszDataDirectory, pszTemplateName);

        if (File::GetFileType (pszDataFile) != FILETYPE_DIRECTORY) {
            continue;
        }

        // Get all directories under here
        strcat (pszDataFile, "/*");

        iErrCode = File::EnumerateFiles (pszDataFile, &fEnum);
        if (iErrCode != OK) {
            return iErrCode;
        }

        ppszSubFileName = fEnum.GetFileNames();
        iNumSubFiles = fEnum.GetNumFiles();

        size_t stFileSize;
        for (i = 0; i < iNumSubFiles; i ++) {
            
            // Do table data files exist?
            sprintf (pszDataFile, "%s%s/%s/" TABLE_DATA_FILE, m_pszDataDirectory, pszTemplateName,
                ppszSubFileName[i]);

            if (File::DoesFileExist (pszDataFile)) {

                // Create a new Table object
                pTable = Table::CreateInstance (ppszSubFileName[i], pTemplate);

                // Try to reload the data
                if (pTable->Reload (pszDataFile) == OK) {

                    // This must succeed, or there is a bug somewhere
                    bAdded = m_htTables.Insert (pTable->GetName(), pTable);
                    Assert (bAdded);
                        
                    // Stablize the table
                    pTable->AddRef();
                        
                    // Add its size to the database
                    if (File::GetFileSize (pszDataFile, &stFileSize) == OK) {
                        IncrementSizeOnDisk (stFileSize);
                    }
                }
                
                pTable->Release();
            }
        }

        // Clean up
        pTemplate->Release();
    }

    return m_htTables.GetNumElements() > 0 ? OK : WARNING;
}

int Database::ReloadBackups() {

    unsigned int i, iNumFiles;
    int iYear, iMonth, iDay, iVersion;
    const char** ppszFileName;
    char pszPath [OS::MaxFileNameLength];
    
    // Look for backups
    FileEnumerator fEnum;
    int iErrCode = File::EnumerateFiles ((String) m_pszBackupDirectory + "*_*_*", &fEnum);

    if (iErrCode != OK || fEnum.GetNumFiles() == 0) {
        return OK;
    }
    
    ppszFileName = fEnum.GetFileNames();
    iNumFiles = fEnum.GetNumFiles();

    for (i = 0; i < iNumFiles; i ++) {

        strcpy (pszPath, m_pszBackupDirectory);
        strcat (pszPath, ppszFileName[i]);

        if (File::GetFileType (pszPath) == FILETYPE_DIRECTORY) {

            if (Database::IsBackup (ppszFileName[i], &iDay, &iMonth, &iYear, &iVersion)) {

                // Looks like a valid backup name
                if (m_iNumBackups == m_iNumBackupsSpace) {
                    
                    m_iNumBackupsSpace *= 2;
                    
                    IDatabaseBackup** ppBackupArray = new IDatabaseBackup* [m_iNumBackupsSpace];
                    
                    memcpy (ppBackupArray, m_ppBackupArray, m_iNumBackups * sizeof (IDatabaseBackup*));
                    
                    delete [] m_ppBackupArray;
                    
                    m_ppBackupArray = ppBackupArray;
                }

                m_ppBackupArray[m_iNumBackups] = Backup::CreateInstance (iDay, iMonth, iYear, iVersion);
                m_iNumBackups ++;
            }
        }
    }

    return OK;
}

bool Database::IsBackup (const char* pszBackupName, int* piDay, int* piMonth, int* piYear, int* piVersion) {

    char pszFileName [OS::MaxFileNameLength];
    strcpy (pszFileName, pszBackupName);

    char* pszStr = pszFileName, * pszTemp = strstr (pszFileName, "_");
    if (pszTemp == NULL) {
        return false;
    }

    // Read year
    *pszTemp = '\0';
    *piYear = atoi (pszStr);

    // Advance to next underscore
    pszStr = pszTemp + 1;
    pszTemp = strstr (pszStr, "_");
    if (pszTemp == NULL) {
        return false;
    }
    
    // Read month
    *pszTemp = '\0';
    *piMonth = atoi (pszStr);

    // Read day
    pszStr = pszTemp + 1;
    *pszTemp = '\0';
    *piDay = atoi (pszStr);
    pszStr = pszTemp + 1;

    // Scan for period to get version
    pszTemp = strstr (pszStr, ".");

    if (pszTemp == NULL) {
        *piVersion = 0;
    } else {
        *piVersion = atoi (pszTemp + 1);
    }

    return true;
}