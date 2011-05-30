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
#include "Template.h"
#include "Backup.h"

void Database::GetBackupDirectory (int iDay, int iMonth, int iYear, int iVersion, char* pszPath) {

    char pszMonth[20], pszDay[20];

    if (iVersion == 0) {
        
        sprintf (pszPath, "%s%i_%s_%s",
            m_pszBackupDirectory,
            iYear,
            String::ItoA (iMonth, pszMonth, 10, 2),
            String::ItoA (iDay, pszDay, 10, 2)
            );
    
    } else {

        sprintf (pszPath, "%s%i_%s_%s.%i",
            m_pszBackupDirectory,
            iYear,
            String::ItoA (iMonth, pszMonth, 10, 2),
            String::ItoA (iDay, pszDay, 10, 2),
            iVersion
            );
    }
}

int Database::Backup (IDatabaseBackupNotificationSink* pSink, bool bCheckFirst) {

    int iErrCode;

    MemoryMappedFile mmfTemplates, mmfVarLenData, mmfTables, mmfMetaData;
    Size stTemplateSize, stVarLenSize, stTableSize, stMetaDataSize;

    char pszBackupDir [OS::MaxFileNameLength];
    char pszFileName [OS::MaxFileNameLength];

    ::Backup* pNewBackup = NULL;

    // Take the global lock
    m_rwGlobalLock.WaitWriter();

    // Run a check if requested
    if (bCheckFirst) {
        iErrCode = Check();
        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    // Get date
    int iSec, iMin, iHour, iDay, iMonth, iYear, iVersion = 0;
    DayOfWeek day;

    Time::GetDate (&iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);

    // Prepare a backup directory
    GetBackupDirectory (iDay, iMonth, iYear, 0, pszBackupDir);

    m_rwBackupLock.WaitWriter();
    while (File::DoesDirectoryExist (pszBackupDir)) {
        GetBackupDirectory (iDay, iMonth, iYear, ++ iVersion, pszBackupDir);
    }
    m_rwBackupLock.SignalWriter();

    // Create backup directory
    iErrCode = File::CreateDirectory (pszBackupDir);
    if (iErrCode != OK) {
        goto Aborted;
    }

    // Create backup object
    pNewBackup = Backup::CreateInstance (iDay, iMonth, iYear, iVersion);
    if (pNewBackup == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Aborted;
    }

    // Sink event
    if (pSink != NULL) {
        pSink->BeginBackup (pszBackupDir);
    }

    // Get table sizes
    stTemplateSize = m_fhTemplateData.GetSize();
    stVarLenSize = m_fhVariableData.GetSize();
    stTableSize = m_fhTableData.GetSize();
    stMetaDataSize = m_fhMetaData.GetSize();

    // Backup templates
    sprintf (pszFileName, "%s/" TEMPLATE_DATA_FILE, pszBackupDir);
    iErrCode = mmfTemplates.OpenNew (pszFileName, (size_t)stTemplateSize, MEMMAP_WRITETHROUGH);
    if (iErrCode != OK) {
        goto Aborted;
    }

    // Sink event
    if (pSink != NULL) {
        pSink->BeginTemplateBackup (m_pTemplates->GetNumElements());
    }

    // Copy data
    memcpy (mmfTemplates.GetAddress(), m_fhTemplateData.GetBaseAddress(), (size_t)stTemplateSize);

    // Sink event
    if (pSink != NULL) {
        pSink->EndTemplateBackup();
    }

    // Backup table data
    sprintf (pszFileName, "%s/" TABLE_DATA_FILE, pszBackupDir);
    iErrCode = mmfTables.OpenNew (pszFileName, (size_t)stTableSize, MEMMAP_WRITETHROUGH);
    if (iErrCode != OK) {
        goto Aborted;
    }

    sprintf (pszFileName, "%s/" VARIABLE_DATA_FILE, pszBackupDir);
    iErrCode = mmfVarLenData.OpenNew (pszFileName, (size_t)stVarLenSize, MEMMAP_WRITETHROUGH);
    if (iErrCode != OK) {
        goto Aborted;
    }

    sprintf (pszFileName, "%s/" META_DATA_FILE, pszBackupDir);
    iErrCode = mmfMetaData.OpenNew (pszFileName, (size_t)stMetaDataSize, MEMMAP_WRITETHROUGH);
    if (iErrCode != OK) {
        goto Aborted;
    }

    // Sink event
    if (pSink != NULL) {
        pSink->BeginTableBackup (m_pTables->GetNumElements());
    }

    // Copy data
    memcpy (mmfTables.GetAddress(), m_fhTableData.GetBaseAddress(), (size_t)stTableSize);

    if (pSink != NULL) {
        pSink->EndTableBackup();
        pSink->BeginVariableLengthDataBackup();
    }

    memcpy (mmfVarLenData.GetAddress(), m_fhVariableData.GetBaseAddress(), (size_t)stVarLenSize);

    if (pSink != NULL) {
        pSink->EndVariableLengthDataBackup();
        pSink->BeginMetaDataBackup();
    }

    memcpy (mmfMetaData.GetAddress(), m_fhMetaData.GetBaseAddress(), (size_t)stMetaDataSize);

    if (pSink != NULL) {
        pSink->EndMetaDataBackup();
    }

    // Sink event
    if (pSink != NULL) {
        pSink->EndTableBackup();
    }

    // We succeeded, so add the backup to the list
    m_rwBackupLock.WaitWriter();

    if (m_iNumBackups == m_iNumBackupsSpace) {

        unsigned int iNumBackupsSpace = m_iNumBackupsSpace * 2;

        IDatabaseBackup** ppBackupArray = new IDatabaseBackup* [iNumBackupsSpace];
        if (ppBackupArray == NULL) {
            m_rwBackupLock.SignalWriter();
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Aborted;
        }

        memcpy (ppBackupArray, m_ppBackupArray, m_iNumBackups * sizeof (IDatabaseBackup*));
        
        delete [] m_ppBackupArray;
        m_ppBackupArray = ppBackupArray;
        m_iNumBackupsSpace = iNumBackupsSpace;
    }

    m_ppBackupArray[m_iNumBackups] = pNewBackup;
    m_iNumBackups ++;

    // Sink event
    if (pSink != NULL) {
        pSink->EndBackup (m_ppBackupArray[m_iNumBackups]);
    }

    m_rwBackupLock.SignalWriter();

    // Release the global lock
    m_rwGlobalLock.SignalWriter();

    // Flush the new files
    mmfTemplates.Flush();
    mmfTemplates.Close();

    mmfTables.Flush();
    mmfTables.Close();

    mmfVarLenData.Flush();
    mmfVarLenData.Close();

    return iErrCode;

Aborted:

    if (pSink != NULL) {
        pSink->AbortBackup (iErrCode);
    }

    m_rwGlobalLock.SignalWriter();

    if (pNewBackup != NULL) {
        pNewBackup->Release();
    }

    mmfTemplates.Close();
    mmfTables.Close();
    mmfVarLenData.Close();

    File::DeleteDirectory (pszBackupDir);

    return iErrCode;
}

unsigned int Database::DeleteOldBackups (Seconds iNumSecondsOld) {

    // Get the old time
    UTCTime tNow, tThen;
    Time::GetTime (&tNow);
    Time::SubtractSeconds (tNow, iNumSecondsOld, &tThen);

    // Get the old date
    int iSec, iMin, iHour, iDay, iMonth, iYear, iTestDay, iTestMonth, iTestYear, iTestVersion, iErrCode;
    DayOfWeek day;

    Time::GetDate (tThen, &iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);
    
    // Lock backup table
    m_rwBackupLock.WaitWriter();

    IDatabaseBackup** ppDelBackup = (IDatabaseBackup**) StackAlloc (m_iNumBackups * sizeof (IDatabaseBackup*));

    unsigned int i, iNumDel = 0;

    for (i = 0; i < m_iNumBackups; i ++) {

        m_ppBackupArray[i]->GetDate (&iTestDay, &iTestMonth, &iTestYear, &iTestVersion);

        if (iTestYear < iYear || 
            (iTestYear == iYear && iTestMonth < iMonth) ||
            (iTestYear == iYear && iTestMonth == iMonth && iTestDay < iDay)) {

            ppDelBackup[iNumDel] = m_ppBackupArray[i];
            ppDelBackup[iNumDel]->AddRef();

            iErrCode = RemoveBackup (i);
            Assert (iErrCode == OK || iErrCode == WARNING);

            iNumDel ++;
        }
    }

    m_rwBackupLock.SignalWriter();

    for (i = 0; i < iNumDel; i ++) {
        DeleteBackupFromDisk (ppDelBackup[i]);
        ppDelBackup[i]->Release();
    }

    return iNumDel;
}

IDatabaseBackupEnumerator* Database::GetBackupEnumerator() {

    m_rwBackupLock.WaitReader();
    IDatabaseBackupEnumerator* pBackupEnumerator = BackupEnumerator::CreateInstance (m_ppBackupArray, m_iNumBackups);
    m_rwBackupLock.SignalReader();

    return pBackupEnumerator;
}


int Database::RestoreBackup (IDatabaseBackup* pBackup) {

    int iErrCode = OK;

    char pszRestoreBackup [OS::MaxFileNameLength];

    int iDay, iMonth, iYear, iVersion;
    pBackup->GetDate (&iDay, &iMonth, &iYear, &iVersion);

    GetBackupDirectory (iDay, iMonth, iYear, iVersion, pszRestoreBackup);

    if (!File::DoesDirectoryExist (pszRestoreBackup)) {

        iErrCode = ERROR_FAILURE;

    } else {

        if (m_pRestoreBackup == pBackup) {
            m_pRestoreBackup->Release();
            iErrCode = WARNING;
        }

        m_pRestoreBackup = pBackup;
        m_pRestoreBackup->AddRef();
    }

    return iErrCode;
}

int Database::DeleteBackup (IDatabaseBackup* pBackup) {

    int iErrCode = OK;
    
    m_rwBackupLock.WaitWriter();
    
    unsigned int i;
    for (i = 0; i < m_iNumBackups; i ++) {
        
        // Pointer equality is sufficient here
        if (pBackup == m_ppBackupArray[i]) {
            iErrCode = RemoveBackup (i);
            break;
        }
    }

    m_rwBackupLock.SignalWriter();

    DeleteBackupFromDisk (pBackup);

    return iErrCode;
}

// Assume writer lock is held by calling thread
int Database::RemoveBackup (unsigned int iIndex) {

    int iErrCode = OK;

    // Are we killing our restore-ee?
    if (m_pRestoreBackup == m_ppBackupArray[iIndex]) {
        m_pRestoreBackup->Release();
        m_pRestoreBackup = NULL;
        iErrCode = WARNING;
    }

    m_ppBackupArray[iIndex]->Release();

    m_iNumBackups --;

    if (iIndex == m_iNumBackups) {
        m_ppBackupArray[iIndex] = NULL;
    } else {
        m_ppBackupArray[iIndex] = m_ppBackupArray[m_iNumBackups];
    }

    return iErrCode;
}

int Database::DeleteBackupFromDisk (IDatabaseBackup* pBackup) {

    // Nuke the directory
    char pszRestoreBackup [OS::MaxFileNameLength];

    int iDay, iMonth, iYear, iVersion;
    pBackup->GetDate (&iDay, &iMonth, &iYear, &iVersion);

    GetBackupDirectory (iDay, iMonth, iYear, iVersion, pszRestoreBackup);

    return File::DeleteDirectory (pszRestoreBackup);
}

int Database::ReplaceDatabaseWithBackup (IDatabaseBackup* pRestoreBackup) {

    int iSec, iMin, iHour, iDay, iMonth, iYear, iVersion = 0;
    DayOfWeek day;

    char pszBackupDir [OS::MaxFileNameLength];
    char pszRestoreDir [OS::MaxFileNameLength];

    // First verify that the backup has a data directory
    int iErrCode = pRestoreBackup->GetDate (&iDay, &iMonth, &iYear, &iVersion);
    if (iErrCode != OK) {
        return iErrCode;
    }

    GetBackupDirectory (iDay, iMonth, iYear, iVersion, pszRestoreDir);
    if (!File::DoesDirectoryExist (pszRestoreDir)) {
        return ERROR_FAILURE;
    }

    // Make a new backup directory with the current time
    Time::GetDate (&iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);

    GetBackupDirectory (iDay, iMonth, iYear, 0, pszBackupDir);

    while (File::DoesDirectoryExist (pszBackupDir)) {
        GetBackupDirectory (iDay, iMonth, iYear, ++ iVersion, pszBackupDir);
    }

    // Move the current database to the new backup directory
    iErrCode = File::MoveDirectory (m_pszDataDirectory, pszBackupDir);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Move the backup to the main database dir
    return File::MoveDirectory (pszRestoreDir, m_pszDataDirectory);
}