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
#include "Template.h"
#include "Backup.h"
#undef DATABASE_BUILD

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

int Database::Backup (IDatabaseBackupNotificationSink* pSink) {

	int iErrCode;

	HashTableIterator <const char*, Table*> htiTableIterator;
	HashTableIterator <const char*, Template*> htiTemplateIterator;

	char pszBackupDir [OS::MaxFileNameLength];

	// Get date
	int iSec, iMin, iHour, iDay, iMonth, iYear, iVersion = 0;
	Time::GetDate (&iSec, &iMin, &iHour, &iDay, &iMonth, &iYear);

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
		return iErrCode;
	}

	// Sink event
	if (pSink != NULL) {
		pSink->BeginBackup (pszBackupDir);
	}

	// Backup each template into the directory
	m_rwTemplateLock.WaitReader();
	
	// Sink event
	if (pSink != NULL) {
		pSink->BeginTemplateBackup (m_htTemplates.GetNumElements());
	}

	Template* pTemplate;
	while (m_htTemplates.GetNextIterator (&htiTemplateIterator) && iErrCode == OK) {
		
		pTemplate = htiTemplateIterator.GetData();
		pTemplate->AddRef();

		if (iErrCode == OK) {
			iErrCode = pTemplate->Backup (pszBackupDir);

			// Sink event
			if (iErrCode == OK && pSink != NULL) {
				pSink->BackupTemplate (pTemplate->GetName());
			}

			Assert (iErrCode == OK);
		}

		pTemplate->Release();
	}

	m_rwTemplateLock.SignalReader();

	// Sink event
	if (pSink != NULL) {
		pSink->EndTemplateBackup();
	}

	if (iErrCode != OK) {
		Assert (false);
		
		// Sink event
		if (pSink != NULL) {
			pSink->AbortBackup (iErrCode);
		}

		File::DeleteDirectory (pszBackupDir);
		return iErrCode;
	}

	unsigned int i = 0, iNumTables;
	Table** ppTable;

	// Get a read lock on the table list
	m_rwTableLock.WaitReader();

	// Sink event
	if (pSink != NULL) {
		pSink->BeginTableBackup (m_htTables.GetNumElements());
	}

	// Get number of tables
	iNumTables = m_htTables.GetNumElements();

	ppTable = (Table**) StackAlloc (iNumTables * sizeof (Table*));

	while (m_htTables.GetNextIterator (&htiTableIterator)) {
		
		ppTable[i] = htiTableIterator.GetData();
		ppTable[i]->WaitReader();
		i ++;
	}

	m_rwTableLock.SignalReader();

	Assert (i == iNumTables);

	iErrCode = OK;
	for (i = 0; i < iNumTables; i ++) {

		if (iErrCode == OK) {
			iErrCode = ppTable[i]->Backup (pszBackupDir);

			// Sink event
			if (iErrCode == OK && pSink != NULL) {
				pSink->BackupTable (ppTable[i]->GetName());
			}

			Assert (iErrCode == OK);
		}
		ppTable[i]->SignalReader();
	}

	// Sink event
	if (pSink != NULL) {
		pSink->EndTableBackup();
	}

	if (iErrCode != OK) {
		Assert (false);

		// Sink event
		if (pSink != NULL) {
			pSink->AbortBackup (iErrCode);
		}

		File::DeleteDirectory (pszBackupDir);
		return iErrCode;
	}

	// We succeeded, so add the backup to the list
	m_rwBackupLock.WaitWriter();

	if (m_iNumBackups == m_iNumBackupsSpace) {
			
		m_iNumBackupsSpace *= 2;
		IDatabaseBackup** ppBackupArray = new IDatabaseBackup* [m_iNumBackupsSpace];
		memcpy (ppBackupArray, m_ppBackupArray, m_iNumBackups * sizeof (IDatabaseBackup*));
		
		delete [] m_ppBackupArray;
		m_ppBackupArray = ppBackupArray;
	}

	m_ppBackupArray[m_iNumBackups] = Backup::CreateInstance (iDay, iMonth, iYear, iVersion);
	m_iNumBackups ++;

	// Sink event
	if (pSink != NULL) {
		pSink->EndBackup (m_ppBackupArray[m_iNumBackups]);
	}

	m_rwBackupLock.SignalWriter();

	return iErrCode;
}

unsigned int Database::DeleteOldBackups (Seconds iNumSecondsOld) {

	unsigned int i, iNumDel = 0;
	IDatabaseBackup** ppDelBackup;
	unsigned int* piIndex;

	UTCTime tNow, tThen;
	int iTestSec, iTestMin, iTestHour, iTestDay, iTestMonth, iTestYear, iTestVersion, iErrCode;\

	// Get the current time
	Time::GetTime (&tNow);

	// Lock backup table
	m_rwBackupLock.WaitWriter();

	ppDelBackup = (IDatabaseBackup**) StackAlloc (m_iNumBackups * sizeof (IDatabaseBackup*));
	piIndex = (unsigned int*) StackAlloc (m_iNumBackups * sizeof (unsigned int));

	for (i = 0; i < m_iNumBackups; i ++) {

		m_ppBackupArray[i]->GetDate (&iTestDay, &iTestMonth, &iTestYear, &iTestVersion);

		// Pretend version is number of seconds
		iTestHour = iTestVersion % (60 * 60);
		if (iTestHour > 0) {
			iTestVersion -= iTestHour * 60 * 60;
		}

		iTestMin = iTestVersion % 60;
		if (iTestMin > 0) {
			iTestVersion -= iTestMin * 60;
		}

		iTestSec = iTestVersion;

		// Get test time
		Time::GetTime (iTestSec, iTestMin, iTestHour, iTestDay, iTestMonth, iTestYear, &tThen);

		if (Time::GetSecondDifference (tNow, tThen) > iNumSecondsOld) {

			ppDelBackup[iNumDel] = m_ppBackupArray[i];
			ppDelBackup[iNumDel]->AddRef();
			piIndex[iNumDel] = i;

			iNumDel ++;
		}
	}

	for (i = 0; i < iNumDel; i ++) {

		// This will succeed
		iErrCode = RemoveBackup (piIndex[i]);
		Assert (iErrCode == OK || iErrCode == WARNING);
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
	Time::GetDate (&iSec, &iMin, &iHour, &iDay, &iMonth, &iYear);

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