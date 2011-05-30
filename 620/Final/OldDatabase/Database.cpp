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
#include "ReadTable.h"
#include "WriteTable.h"
#undef DATABASE_BUILD

#include "Osal/Time.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Database::Database() : m_htTemplates (NULL, NULL), m_htTables (NULL, NULL) {

    m_pszMainDirectory = NULL;
    m_pszDataDirectory = NULL;
    m_pszBackupDirectory = NULL;

    m_iNumLoadedRows = 0;
    m_stSizeOnDisk = 0;

    m_iNumBackups = 0;
    m_ppBackupArray = new IDatabaseBackup* [5];
    m_iNumBackupsSpace = 5;

    m_pRestoreBackup = NULL;

    m_iNumRefs = 1;

    m_iOptions = 0;
}


Database::~Database() {

    HashTableIterator <const char*, Template*> htiTemplateIterator;
    HashTableIterator <const char*, Table*> htiTableIterator;
    
    while (m_htTemplates.GetNextIterator (&htiTemplateIterator)) {
        htiTemplateIterator.GetData()->Release();   // Deletes key, data        
    }

    while (m_htTables.GetNextIterator (&htiTableIterator)) {
        htiTableIterator.GetData()->Release();  // Deletes key, data        
    }

    if (m_pRestoreBackup != NULL) {

        ReplaceDatabaseWithBackup (m_pRestoreBackup);
        m_pRestoreBackup->Release();
    }

    if (m_ppBackupArray != NULL) {

        unsigned i;

        for (i = 0; i < m_iNumBackups; i ++) {
            m_ppBackupArray[i]->Release();
        }
        delete [] m_ppBackupArray;
    }

    if (m_pszMainDirectory != NULL) {
        OS::HeapFree (m_pszMainDirectory);
    }

    if (m_pszDataDirectory != NULL) {
        delete [] m_pszDataDirectory;
    }

    if (m_pszBackupDirectory != NULL) {
        delete [] m_pszBackupDirectory;
    }
}

Database* Database::CreateInstance() {
    return new Database();
}

int Database::Initialize (const char* pszMainDirectory, unsigned int iOptions) {

    int iErrCode;

    m_iOptions = iOptions;

    iErrCode = m_rwTableLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_rwTemplateLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_rwBackupLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    m_pszMainDirectory = String::StrDup (pszMainDirectory);
    if (m_pszMainDirectory == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    size_t iMainLength = strlen (pszMainDirectory);

    m_pszDataDirectory = new char [iMainLength + sizeof (DATA_DIRECTORY) + 3];
    if (m_pszDataDirectory == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    sprintf (m_pszDataDirectory, "%s/" DATA_DIRECTORY "/", pszMainDirectory);

    m_pszBackupDirectory = new char [iMainLength + sizeof (BACKUP_DIRECTORY) + 3];
    if (m_pszBackupDirectory == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    sprintf (m_pszBackupDirectory, "%s/" BACKUP_DIRECTORY "/", pszMainDirectory);

    if (!File::DoesDirectoryExist (m_pszMainDirectory)) {
        int iErrCode = File::CreateDirectory (m_pszMainDirectory);
        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    int iNumFiles;
    bool bReload = true;

    if (!File::DoesDirectoryExist (m_pszDataDirectory)) {
        bReload = false;
        if (File::CreateDirectory (m_pszDataDirectory) != OK) {
            return ERROR_COULD_NOT_CREATE_TABLE;
        }
    }

    if (!File::DoesDirectoryExist (m_pszBackupDirectory)) {
        if (File::CreateDirectory (m_pszBackupDirectory) != OK) {
            return ERROR_COULD_NOT_MAKE_BACKUP;
        }
    }

    // Initialize hash tables
    FileEnumerator fEnum;
    iErrCode = File::EnumerateFiles ((String) m_pszDataDirectory + "*", &fEnum);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iNumFiles = max (fEnum.GetNumFiles(), 50);
    
    if (!m_htTemplates.Initialize (iNumFiles)) {
        return ERROR_OUT_OF_MEMORY;
    }

    iNumFiles = max (fEnum.GetNumFiles(), 5000);
    if (!m_htTables.Initialize (iNumFiles)) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (bReload) {
        return Reload();
    }
    
    return WARNING;
}

const char* Database::GetDirectory() {
    return m_pszMainDirectory;
}

const char* Database::GetDataDirectory() {
    return m_pszDataDirectory;
}

unsigned int Database::GetNumTables() {
    return (unsigned int) m_htTables.GetNumElements();
}

unsigned int Database::GetNumTemplates() {
    return (unsigned int) m_htTemplates.GetNumElements();
}

unsigned int Database::GetNumLoadedRows() {
    return m_iNumLoadedRows;
}

size_t Database::GetSizeOnDisk() {
    return m_stSizeOnDisk;
}

int Database::Flush() {

    size_t i = 0, iNumTables;
    Table** ppTable;

    HashTableIterator <const char*, Table*> htiTableIterator;

    // Get a read lock on the table list
    m_rwTableLock.WaitReader();

    // Get number of tables
    iNumTables = m_htTables.GetNumElements();

    ppTable = (Table**) StackAlloc (iNumTables * sizeof (Table*));

    while (m_htTables.GetNextIterator (&htiTableIterator)) {
        
        ppTable[i] = htiTableIterator.GetData();
        ppTable[i]->AddRef();
        i ++;
    }

    // Release the lock on the table list
    m_rwTableLock.SignalReader();

    Assert (i == iNumTables);

    for (i = 0; i < iNumTables; i ++) {

        ppTable[i]->Flush();
        ppTable[i]->Release();
    }

    return OK;
}


void Database::IncrementSizeOnDisk (size_t stChange) {
    Algorithm::AtomicIncrement (&m_stSizeOnDisk, (int) stChange);
}

void Database::DecrementSizeOnDisk (size_t stChange) {
    Algorithm::AtomicDecrement (&m_stSizeOnDisk, (int) stChange);
}

void Database::IncrementNumLoadedRows (unsigned int iNumRows) {
    Algorithm::AtomicIncrement (&m_iNumLoadedRows, iNumRows);
}

void Database::DecrementNumLoadedRows (unsigned int iNumRows) {
    Algorithm::AtomicDecrement (&m_iNumLoadedRows, iNumRows);
}

ITableEnumerator* Database::GetTableEnumerator() {
    
    m_rwTableLock.WaitReader();
    ITableEnumerator* pTableEnumerator = TableEnumerator::CreateInstance (&m_htTables);
    m_rwTableLock.SignalReader();

    return pTableEnumerator;
}

ITemplateEnumerator* Database::GetTemplateEnumerator() {

    m_rwTemplateLock.WaitReader();
    ITemplateEnumerator* pTemplateEnumerator = TemplateEnumerator::CreateInstance (&m_htTemplates);
    m_rwTemplateLock.SignalReader();

    return pTemplateEnumerator;
}

unsigned int Database::GetOptions() {
    return m_iOptions;
}

void Database::FreeData (void** ppData) {
    delete [] ppData;
}

void Database::FreeData (Variant* pvData) {
    delete [] pvData;
}

void Database::FreeData (Variant** ppvData) {
    delete [] (*ppvData);
    delete [] ppvData;
}

void Database::FreeKeys (unsigned int* piKeys) {
    delete [] piKeys;
}

void Database::FreeData (int* piData) {
    delete [] piData;
}

void Database::FreeData (unsigned int* puiData) {
    delete [] puiData;
}

void Database::FreeData (float* ppfData) {
    delete [] ppfData;
}

void Database::FreeData (char** ppszData) {
    delete [] ppszData;
}

void Database::FreeData (UTCTime* ptData) {
    delete [] ptData;
}