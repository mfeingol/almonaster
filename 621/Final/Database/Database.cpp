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
 
#include "CDatabase.h"
#include "Table.h"

#include "Osal/Time.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Database::Database() : m_fhTableData (m_rwHeapLock), 
                       m_fhVariableData (m_rwHeapLock),
                       m_fhMetaData (m_rwHeapLock),
                       m_fhTemplateData (m_rwHeapLock)
{

    m_pTemplates = NULL;
    m_pTables = NULL;

    m_pszMainDirectory = NULL;
    m_pszDataDirectory = NULL;
    m_pszBackupDirectory = NULL;

    m_iNumBackups = 0;
    m_ppBackupArray = NULL;
    m_iNumBackupsSpace = 0;

    m_pRestoreBackup = NULL;

    m_iNumRefs = 1;
    m_iOptions = 0;

    m_bInitialized = false;
}


Database::~Database() {

#ifdef _DEBUG

    // Check if requested
    if (m_iOptions & DATABASE_CHECK) {
        int iErrCode = Check();
        Assert (iErrCode == OK);
    }

#endif

    if (m_pTemplates != NULL) {

        HashTableIterator <const char*, Template*> htiIterator;

        while (m_pTemplates->GetNextIterator (&htiIterator)) {
            htiIterator.GetData()->Release();   // Deletes key, data        
        }
        delete m_pTemplates;
    }

    if (m_pTables != NULL) {

        HashTableIterator <const char*, Table*> htiIterator;

        while (m_pTables->GetNextIterator (&htiIterator)) {
            htiIterator.GetData()->Release();   // Deletes key, data        
        }
        delete m_pTables;
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

    if (m_pszBackupDirectory != NULL) {
        OS::HeapFree (m_pszBackupDirectory);
    }
}

Database* Database::CreateInstance() {
    return new Database();
}

int Database::Initialize (const char* pszMainDirectory, unsigned int iOptions) {

    int iErrCode;
    bool bTables = false, bTemplates = false, bVariable = false, bMeta = false;

    char pszFileName [OS::MaxFileNameLength] = "";

    if (m_bInitialized) {
        return ERROR_DATABASE_ALREADY_INITIALIZED;
    }

    // Save input
    m_iOptions = iOptions;

    iErrCode = m_rwTableLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_rwTemplateLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_rwGlobalLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_rwBackupLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_rwHeapLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_fhTableData.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_fhVariableData.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_fhMetaData.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_fhTemplateData.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Init backup array
    Assert (m_ppBackupArray == NULL);
    m_ppBackupArray = new IDatabaseBackup* [5];
    if (m_ppBackupArray == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    Assert (m_iNumBackupsSpace == 0);
    m_iNumBackupsSpace = 5;

    Assert (m_pszMainDirectory == NULL);
    m_pszMainDirectory = String::StrDup (pszMainDirectory);

    // Look at directories
    m_pszBackupDirectory = (char*) OS::HeapAlloc ((strlen (pszMainDirectory) + sizeof (BACKUP_DIRECTORY) + 3) * sizeof (char));
    sprintf (m_pszBackupDirectory, "%s/" BACKUP_DIRECTORY "/", pszMainDirectory);

    if (!File::DoesDirectoryExist (m_pszMainDirectory)) {
        iErrCode = File::CreateDirectory (m_pszMainDirectory);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    if (!File::DoesDirectoryExist (m_pszBackupDirectory)) {
        if (File::CreateDirectory (m_pszBackupDirectory) != OK) {
            return ERROR_COULD_NOT_MAKE_BACKUP;
        }
    }

    // Fixed data file
    sprintf (pszFileName, "%s/" TABLE_DATA_FILE, pszMainDirectory);

    iErrCode = OpenDataFile (pszFileName, &m_fhTableData, DEFAULT_TABLE_SIZE, &bTables);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Variable length file
    sprintf (pszFileName, "%s/" VARIABLE_DATA_FILE, pszMainDirectory);

    iErrCode = OpenDataFile (pszFileName, &m_fhVariableData, DEFAULT_VARIABLE_SIZE, &bVariable);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Meta data file
    sprintf (pszFileName, "%s/" META_DATA_FILE, pszMainDirectory);

    iErrCode = OpenDataFile (pszFileName, &m_fhMetaData, DEFAULT_META_SIZE, &bMeta);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Template file
    sprintf (pszFileName, "%s/" TEMPLATE_DATA_FILE, pszMainDirectory);

    iErrCode = OpenDataFile (pszFileName, &m_fhTemplateData, DEFAULT_TEMPLATE_SIZE, &bTemplates);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (bTables != bVariable || bTables != bMeta || bTables != bTemplates) {
        Assert (false);
        return ERROR_DATABASE_IS_INCOMPLETE;
    }

    // Try to reload the database, if possible
    iErrCode = Reload();
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (m_iOptions & DATABASE_CHECK) {

        iErrCode = Check();
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    m_bInitialized = true;

    return OK;
}

int Database::OpenDataFile (const char* pszFile, FileHeap* pfhHeap, size_t stDefaultSize, bool* pbExists) {

    int iErrCode;
    bool bExists = File::DoesFileExist (pszFile);

    unsigned int iFlags = 0;

    if (m_iOptions & DATABASE_WRITETHROUGH) {
        iFlags |= MEMMAP_WRITETHROUGH;
    }
    
    if (bExists) {
        iErrCode = pfhHeap->OpenExisting (pszFile, iFlags);
    } else {
        iErrCode = pfhHeap->OpenNew (pszFile, stDefaultSize, iFlags);
    }

    *pbExists = bExists;

    return iErrCode;
}


const char* Database::GetDirectory() {
    return m_pszMainDirectory;
}

const char* Database::GetDataDirectory() {
    return m_pszDataDirectory;
}

unsigned int Database::GetNumTables() {
    return m_pTables->GetNumElements();
}

unsigned int Database::GetNumTemplates() {
    return m_pTemplates->GetNumElements();
}

int Database::Flush() {

    int iErrCode;
    
    iErrCode = m_fhTableData.Flush();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_fhVariableData.Flush();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_fhMetaData.Flush();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_fhTemplateData.Flush();
    if (iErrCode != OK) {
        return iErrCode;
    }

    return OK;
}

int Database::Check() {

    if (!m_fhTableData.Check()) {
        return ERROR_DATA_CORRUPTION;
    }

    if (!m_fhVariableData.Check()) {
        return ERROR_DATA_CORRUPTION;
    }

    if (!m_fhMetaData.Check()) {
        return ERROR_DATA_CORRUPTION;
    }

    if (!m_fhTemplateData.Check()) {
        return ERROR_DATA_CORRUPTION;
    }

    return OK;
}


ITableEnumerator* Database::GetTableEnumerator() {
    
    m_rwTableLock.WaitReader();
    ITableEnumerator* pTableEnumerator = TableEnumerator::CreateInstance (m_pTables);
    m_rwTableLock.SignalReader();

    return pTableEnumerator;
}

ITemplateEnumerator* Database::GetTemplateEnumerator() {

    m_rwTemplateLock.WaitReader();
    ITemplateEnumerator* pTemplateEnumerator = TemplateEnumerator::CreateInstance (m_pTemplates);
    m_rwTemplateLock.SignalReader();

    return pTemplateEnumerator;
}

void Database::FreeData (void** ppData) {
    delete [] ppData;
}

void Database::FreeData (Variant* pvData) {
    delete [] pvData;
}

void Database::FreeData (Variant** ppvData) {
    if (ppvData != NULL) {
        delete [] (*ppvData);
    }
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

int Database::GetStatistics (DatabaseStatistics* pdsStats) {

    pdsStats->iNumTables = GetNumTables();
    pdsStats->iNumTemplates = GetNumTemplates();

    m_fhTableData.GetStatistics (&pdsStats->fhsTableStats);
    m_fhTemplateData.GetStatistics (&pdsStats->fhsTemplateStats);
    m_fhMetaData.GetStatistics (&pdsStats->fhsMetaStats);
    m_fhVariableData.GetStatistics (&pdsStats->fhsVarlenStats);

    return OK;
}

unsigned int Database::GetOptions() {
    return m_iOptions;
}