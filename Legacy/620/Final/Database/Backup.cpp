// Backup.cpp: implementation of the Backup class.
//
//////////////////////////////////////////////////////////////////////

#include "Backup.h"
#include "CDatabase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Backup::Backup (int iDay, int iMonth, int iYear, int iVersion) {

    m_iNumRefs = 1;

    m_iDay = iDay;
    m_iMonth = iMonth;
    m_iYear = iYear;
    m_iVersion = iVersion;
}

Backup* Backup::CreateInstance (int iDay, int iMonth, int iYear, int iVersion) {

    return new Backup (iDay, iMonth, iYear, iVersion);
}

int Backup::GetDate (int* piDay, int* piMonth, int* piYear, int* piVersion) {

    *piDay = m_iDay;
    *piMonth = m_iMonth;
    *piYear = m_iYear;
    *piVersion = m_iVersion;

    return OK;
}

BackupEnumerator::BackupEnumerator() {

    m_iNumRefs = 1;
    m_iNumBackups = 0;
    m_ppBackupArray = NULL;
}

BackupEnumerator::~BackupEnumerator() {

    unsigned int i;
    
    if (m_ppBackupArray != NULL) {
        
        for (i = 0; i < m_iNumBackups; i ++) {
            m_ppBackupArray[i]->Release();
        }
        
        delete [] m_ppBackupArray;
    }
}

int BackupEnumerator::Initialize (IDatabaseBackup** ppBackupArray, unsigned int iNumBackups) {

    if (iNumBackups > 0) {
        
        unsigned int i;
        
        m_ppBackupArray = new IDatabaseBackup* [iNumBackups];
        if (m_ppBackupArray == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
        
        memcpy (m_ppBackupArray, ppBackupArray, iNumBackups * sizeof (IDatabaseBackup*));
        
        for (i = 0; i < iNumBackups; i ++) {
            m_ppBackupArray[i]->AddRef();
        }

        m_iNumBackups = iNumBackups;
    }

    return OK;
}

BackupEnumerator* BackupEnumerator::CreateInstance (IDatabaseBackup** ppBackupArray, unsigned int iNumBackups) {

    BackupEnumerator* pEnum = new BackupEnumerator();
    if (pEnum != NULL) {
        
        if (pEnum->Initialize (ppBackupArray, iNumBackups) != OK) {
            delete pEnum;
            pEnum = NULL;
        }
    }

    return pEnum;
}

unsigned int BackupEnumerator::GetNumBackups() {
    return m_iNumBackups;
}

IDatabaseBackup** BackupEnumerator::GetBackups() {

    return m_ppBackupArray;
}