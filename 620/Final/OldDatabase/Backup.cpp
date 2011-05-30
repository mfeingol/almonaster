// Backup.cpp: implementation of the Backup class.
//
//////////////////////////////////////////////////////////////////////

#define DATABASE_BUILD
#include "Backup.h"
#include "CDatabase.h"
#undef DATABASE_BUILD

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

BackupEnumerator::BackupEnumerator (IDatabaseBackup** ppBackupArray, unsigned int iNumBackups) {

    m_iNumRefs = 1;

    m_iNumBackups = iNumBackups;

    if (iNumBackups == 0) {
        m_ppBackupArray = NULL;
    } else {
        m_ppBackupArray = new IDatabaseBackup* [iNumBackups];
        memcpy (m_ppBackupArray, ppBackupArray, iNumBackups * sizeof (IDatabaseBackup*));

        unsigned int i;
        for (i = 0; i < iNumBackups; i ++) {
            m_ppBackupArray[i]->AddRef();
        }
    }
}

BackupEnumerator::~BackupEnumerator() {

    unsigned int i;
    for (i = 0; i < m_iNumBackups; i ++) {
        m_ppBackupArray[i]->Release();
    }

    if (m_ppBackupArray != NULL) {
        delete [] m_ppBackupArray;
    }
}

BackupEnumerator* BackupEnumerator::CreateInstance (IDatabaseBackup** ppBackupArray, unsigned int iNumBackups) {

    return new BackupEnumerator (ppBackupArray, iNumBackups);
}

unsigned int BackupEnumerator::GetNumBackups() {
    return m_iNumBackups;
}

IDatabaseBackup** BackupEnumerator::GetBackups() {

    return m_ppBackupArray;
}