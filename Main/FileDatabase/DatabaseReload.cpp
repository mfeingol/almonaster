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

#include "CDatabase.h"
#include "Table.h"
#include "Backup.h"

int Database::Reload() {

    int iErrCode;

    iErrCode = ReloadTemplates();
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

    int iErrCode = OK;
    bool bRetVal;

    Template* pTemplate, * pOldTemplate;

    // Get number of templates needed
    Count stNumTemplates = m_fhTemplateData.GetNumAllocatedBlocks();

    m_pTemplates = new TemplateHashTable (NULL, NULL);
    if (m_pTemplates == NULL || !m_pTemplates->Initialize (max ((unsigned int) stNumTemplates, 20))) {
        return ERROR_OUT_OF_MEMORY;
    }

    // Loop through blocks
    Offset oTemplate = m_fhTemplateData.GetFirstBlock();

    while (oTemplate != NO_OFFSET) {

        if (!m_fhTemplateData.IsBlockFree (oTemplate)) {

            pTemplate = new Template (this);
            if (pTemplate == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                break;
            }

            iErrCode = pTemplate->Reload (oTemplate);
            if (iErrCode != OK) {

                Assert (!"Template could not be reloaded");
                delete pTemplate;

                iErrCode = ERROR_TEMPLATE_COULD_NOT_BE_INITIALIZED;
                break;

            } else {

                // Make sure table name is unique
                pOldTemplate = FindTemplate (pTemplate->GetName());
                if (pOldTemplate != NULL) {

                    Assert (!"Duplicate template name");

                    delete pTemplate;
                    pOldTemplate->Release();

                    iErrCode = ERROR_TEMPLATE_ALREADY_EXISTS;
                    break;

                } else {

                    bRetVal = m_pTemplates->Insert (pTemplate->GetName(), pTemplate);
                    if (!bRetVal) {

                        delete pTemplate;

                        iErrCode = ERROR_OUT_OF_MEMORY;
                        break;
                    }
                }
            }
        }

        oTemplate = m_fhTemplateData.GetNextBlock (oTemplate);
    }

    if (iErrCode == OK && m_pTemplates->GetNumElements() != stNumTemplates) {
        iErrCode = ERROR_DATA_CORRUPTION;
    }

    return iErrCode;
}

int Database::ReloadTables() {

    int iErrCode = OK;
    bool bRetVal;

    Table* pTable, * pOldTable;

    // Get number of tables needed
    Count stNumTables = m_fhTableData.GetNumAllocatedBlocks();

    m_pTables = new TableHashTable (NULL, NULL);
    if (m_pTables == NULL || !m_pTables->Initialize (max ((unsigned int) stNumTables, 100))) {
        return ERROR_OUT_OF_MEMORY;
    }

    m_rwHeapLock.WaitReader();

    // Loop through all blocks
    Offset oTable = m_fhTableData.GetFirstBlock();
    while (oTable != NO_OFFSET) {

        if (!m_fhTableData.IsBlockFree (oTable)) {

            pTable = new Table (this);
            if (pTable == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                break;
            }

            iErrCode = pTable->Initialize();
            if (iErrCode != OK) {
                delete pTable;
                break;
            }

            iErrCode = pTable->Reload (oTable);
            if (iErrCode != OK) {

                Assert (!"Table could not be reloaded");
                delete pTable;

                Assert (false);
                iErrCode = ERROR_TABLE_COULD_NOT_BE_INITIALIZED;
                break;

            } else {

                // Make sure table name is unique
                pOldTable = FindTable (pTable->GetName());
                if (pOldTable != NULL) {

                    Assert (!"Duplicate table name");

                    delete pTable;
                    pOldTable->Release();

                    Assert (false);
                    iErrCode = ERROR_TABLE_ALREADY_EXISTS;
                    break;

                } else {

                    bRetVal = m_pTables->Insert (pTable->GetName(), pTable);
                    if (!bRetVal) {

                        delete pTable;

                        iErrCode = ERROR_OUT_OF_MEMORY;
                        break;
                    }
                }
            }
        }

        oTable = m_fhTableData.GetNextBlock (oTable);
    }

    m_rwHeapLock.SignalReader();

    if (iErrCode == OK && m_pTables->GetNumElements() != stNumTables) {
        Assert (false);
        iErrCode = ERROR_DATA_CORRUPTION;
    }

    return iErrCode;
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
                    
                    unsigned int iNumBackupsSpace = m_iNumBackupsSpace * 2;
                    
                    IDatabaseBackup** ppBackupArray = new IDatabaseBackup* [iNumBackupsSpace];
                    if (ppBackupArray == NULL) {
                        return ERROR_OUT_OF_MEMORY;
                    }
                    
                    memcpy (ppBackupArray, m_ppBackupArray, m_iNumBackups * sizeof (IDatabaseBackup*));
                    
                    delete [] m_ppBackupArray;
                    
                    m_ppBackupArray = ppBackupArray;
                    m_iNumBackupsSpace = iNumBackupsSpace;
                }

                m_ppBackupArray[m_iNumBackups] = Backup::CreateInstance (iDay, iMonth, iYear, iVersion);
                if (m_ppBackupArray[m_iNumBackups] == NULL) {
                    return ERROR_OUT_OF_MEMORY;
                }
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