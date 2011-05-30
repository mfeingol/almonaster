//
// GameEngine.dll:  a component of Almonaster 2.0
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "GameEngine.h"

struct DatabasePurge {
	int Criteria; 
	int EmpireKey;
};

struct DatabaseBackupInfo {
	int EmpireKey;
	int Day;
	int Month;
	int Year;
	int Version;
};

int GameEngine::DeleteOldDatabaseBackups() {

	Assert (m_scConfig.iBackupLifeTimeInSeconds > 0);
	
	Thread tSelf;
	Thread::GetCurrentThread (&tSelf);
	Thread::ThreadPriority tpPriority = tSelf.GetPriority();
	tSelf.SetPriority (Thread::LowestPriority);
	
	m_pGameData->DeleteOldBackups (m_scConfig.iBackupLifeTimeInSeconds);
	
	tSelf.SetPriority (tpPriority);

	return OK;
}

int GameEngine::FlushDatabase (int iEmpireKey) {

	return SendLongRunningQueryMessage (FlushDatabaseMsg, (void*) iEmpireKey);
}

int GameEngine::FlushDatabaseMsg (LongRunningQueryMessage* pMessage) {

	return pMessage->GameEngine->FlushDatabasePrivate ((int) pMessage->Arguments);
}

int GameEngine::FlushDatabasePrivate (int iEmpireKey) {

	int iErrCode = m_pGameData->Flush();
	
	if (iEmpireKey != NO_KEY && iEmpireKey != SYSTEM) {

		const char* pszMessage;
		char pszText [256];

		if (iErrCode == OK) {
			pszMessage = "The database was successfully flushed";
		} else {
			sprintf (pszText, "The database could not be flushed: error %i occurred", iErrCode);
			pszMessage = pszText;
		}
		
		SendSystemMessage (
			iEmpireKey, 
			pszMessage,
			SYSTEM
			);
	}

	return iErrCode;
}

int GameEngine::BackupDatabase (int iEmpireKey) {

	return SendLongRunningQueryMessage (BackupDatabaseMsg, (void*) iEmpireKey);
}

int GameEngine::BackupDatabaseMsg (LongRunningQueryMessage* pMessage) {

	return pMessage->GameEngine->BackupDatabasePrivate ((int) pMessage->Arguments);
}

int GameEngine::BackupDatabasePrivate (int iEmpireKey) {

	int iErrCode = OK;
	bool bInternal = iEmpireKey == SYSTEM;

	int* piGameClass = NULL, * piGameNumber = NULL, iNumGames = 0, i, iErrCode2 = OK;
	bool* pbAdminPaused = NULL;

	// Set ourselves as high priority
	Thread tSelf;
	Thread::GetCurrentThread (&tSelf);
	Thread::ThreadPriority tpPriority = tSelf.GetPriority();
	tSelf.SetPriority (Thread::HigherPriority);

	m_bActiveBackup = true;

	// When this function returns, we'll be the only thread active
	m_pPageSourceControl->LockWithNoThreads();

	// Talk to the unwashed masses
	iErrCode = SetAccessDisabledReason ("The server is being backed up");
	if (iErrCode != OK) {
		Assert (false);
		goto EndPause;
	}

	iErrCode = SetEmpireCreationDisabledReason ("The server is being backed up");
	if (iErrCode != OK) {
		Assert (false);
		goto EndPause;
	}

	iErrCode = SetSystemOption (ACCESS_ENABLED | NEW_EMPIRES_ENABLED, false);
	if (iErrCode != OK) {
		Assert (false);
		goto EndPause;
	}

	iErrCode = GetActiveGames (&piGameClass, &piGameNumber, &iNumGames);
	if (iErrCode != OK) {
		Assert (false);
		goto EndPause;
	}

	if (iNumGames > 0) {

		Variant vState, vUpdate;
		char pszGameData [256];

		pbAdminPaused = (bool*) StackAlloc (iNumGames * sizeof (bool));

		// Best effort
		for (i = 0; i < iNumGames; i ++) {

			GET_GAME_DATA (pszGameData, piGameClass[i], piGameNumber[i]);

			iErrCode2 = m_pGameData->ReadData (
				pszGameData,
				GameData::State,
				&vState
				);

			if (iErrCode2 != OK) {
				Assert (false);
				pbAdminPaused[i] = false;
				continue;
			}
			
			iErrCode2 = m_pGameData->ReadData (
				SYSTEM_GAMECLASS_DATA,
				piGameClass[i],
				SystemGameClassData::NumSecPerUpdate,
				&vUpdate
				);

			if (iErrCode2 != OK) {
				Assert (false);
				pbAdminPaused[i] = false;
				continue;
			}

			if ((vState.GetInteger() & ADMIN_PAUSED) || (vUpdate.GetInteger() >= (60 * 60 * 12))) {
				pbAdminPaused[i] = true;
			} else {
				pbAdminPaused[i] = false;
				iErrCode2 = AdminPauseGame (piGameClass[i], piGameNumber[i], false);
				Assert (iErrCode2 == OK);
			}
		}
	}

EndPause:

	// Let the unwashed masses receive responses
	m_pPageSourceControl->ReleaseLock();

	if (iErrCode != OK) {
		goto End;
	}

	// Tell the database to back itself up:  when this terminates it will have finished
	iErrCode = m_pGameData->Backup (this);
	Assert (iErrCode == OK);

	m_bActiveBackup = false;

	// Best effort let the unwashed masses play again
	iErrCode2 = SetSystemOption (ACCESS_ENABLED | NEW_EMPIRES_ENABLED, true);
	Assert (iErrCode2 == OK);

	iErrCode2 = SetAccessDisabledReason ("");
	Assert (iErrCode2 == OK);

	iErrCode2 = SetEmpireCreationDisabledReason ("");
	Assert (iErrCode2 == OK);

	if (iNumGames > 0) {

		for (i = 0; i < iNumGames; i ++) {

			// Best effort
			if (!pbAdminPaused[i]) {
				iErrCode2 = AdminUnpauseGame (piGameClass[i], piGameNumber[i], false);
				Assert (iErrCode2 == OK);
			}
		}

		delete [] piGameClass;
		delete [] piGameNumber;
	}

	// Reset priority
	tSelf.SetPriority (tpPriority);

End:

	// Send message to originator
	if (!bInternal && iEmpireKey != NO_KEY && iEmpireKey != SYSTEM) {

		const char* pszMessage;
		char pszText [256];

		if (iErrCode == OK) {
			pszMessage = "The database was successfully backed up";
		} else {
			sprintf (pszText, "The database could not be backed up: error %i occurred", iErrCode);
			pszMessage = pszText;
		}

		SendSystemMessage (
			iEmpireKey, 
			pszMessage,
			SYSTEM
			);
	}

	// We're set
	return iErrCode;
}


int GameEngine::RestoreDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion) {

	DatabaseBackupInfo* pInfo = new DatabaseBackupInfo;
	if (pInfo == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	pInfo->EmpireKey = iEmpireKey;
	pInfo->Day = iDay;
	pInfo->Month = iMonth;
	pInfo->Year = iYear;
	pInfo->Version = iVersion;

	return SendLongRunningQueryMessage (RestoreDatabaseBackupMsg, pInfo);
}

int GameEngine::RestoreDatabaseBackupMsg (LongRunningQueryMessage* pMessage) {

	DatabaseBackupInfo* pInfo = (DatabaseBackupInfo*) pMessage->Arguments;

	int iErrCode = pMessage->GameEngine->RestoreDatabaseBackupPrivate (
		pInfo->EmpireKey,
		pInfo->Day,
		pInfo->Month,
		pInfo->Year,
		pInfo->Version
		);

	delete pInfo;
	return iErrCode;
}


int GameEngine::RestoreDatabaseBackupPrivate (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion) {

	int iTestDay, iTestMonth, iTestYear, iTestVersion, iErrCode = ERROR_FAILURE;

	IDatabaseBackupEnumerator* pBackupEnumerator = m_pGameData->GetBackupEnumerator();
	if (pBackupEnumerator != NULL) {

		IDatabaseBackup** ppBackups = pBackupEnumerator->GetBackups();
		if (ppBackups != NULL) {

			unsigned int i, iNumBackups = pBackupEnumerator->GetNumBackups();

			for (i = 0; i < iNumBackups; i ++) {
		
				ppBackups[i]->GetDate (&iTestDay, &iTestMonth, &iTestYear, &iTestVersion);

				if (iTestDay == iDay && iTestMonth == iMonth && iTestYear == iYear && iTestVersion == iVersion) {

					iErrCode = m_pGameData->RestoreBackup (ppBackups[i]);
					break;
				}
			}
		}
		pBackupEnumerator->Release();
	}

	if (iEmpireKey != SYSTEM) {

		char pszText [1024];

		if (iErrCode == OK) {

			const char* pszBackedUp = 
				" backup was restored. You will need to restart Almonaster to make the restore effective";

			if (iVersion == 0) {
				sprintf (pszText, "The %i-%i-%i%s",  iYear, iMonth, iDay, pszBackedUp);
			} else {
				sprintf (pszText, "The %i-%i-%i (%i)%s",  iYear, iMonth, iDay, iVersion, pszBackedUp);
			}

			SendSystemMessage (
				iEmpireKey, 
				pszText, 
				SYSTEM
				);

		} else {

			const char* pszBackedUp = 
				" backup could not be restored; error ";

			if (iVersion == 0) {
				sprintf (pszText, "The %i-%i-%i%s%i occurred",  iYear, iMonth, iDay, pszBackedUp, iErrCode);
			} else {
				sprintf (pszText, "The %i-%i-%i (%i)%s%i occurred",  iYear, iMonth, iDay, iVersion, pszBackedUp, iErrCode);
			}
			
			SendSystemMessage (
				iEmpireKey, 
				pszText, 
				SYSTEM
				);
		}
	}

	return iErrCode;
}

int GameEngine::DeleteDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion) {

	DatabaseBackupInfo* pInfo = new DatabaseBackupInfo;
	if (pInfo == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	pInfo->EmpireKey = iEmpireKey;
	pInfo->Day = iDay;
	pInfo->Month = iMonth;
	pInfo->Year = iYear;
	pInfo->Version = iVersion;

	return SendLongRunningQueryMessage (DeleteDatabaseBackupMsg, pInfo);
}

int GameEngine::DeleteDatabaseBackupMsg (LongRunningQueryMessage* pMessage) {

	DatabaseBackupInfo* pInfo = (DatabaseBackupInfo*) pMessage->Arguments;

	int iErrCode = pMessage->GameEngine->DeleteDatabaseBackupPrivate (
		pInfo->EmpireKey,
		pInfo->Day,
		pInfo->Month,
		pInfo->Year,
		pInfo->Version
		);

	delete pInfo;
	return iErrCode;
}

int GameEngine::DeleteDatabaseBackupPrivate (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion) {

	int iTestDay, iTestMonth, iTestYear, iTestVersion, iErrCode = ERROR_FAILURE;

	IDatabaseBackupEnumerator* pBackupEnumerator = m_pGameData->GetBackupEnumerator();
	if (pBackupEnumerator != NULL) {

		IDatabaseBackup** ppBackups = pBackupEnumerator->GetBackups();
		if (ppBackups != NULL) {

			unsigned int i, iNumBackups = pBackupEnumerator->GetNumBackups();

			for (i = 0; i < iNumBackups; i ++) {
		
				ppBackups[i]->GetDate (&iTestDay, &iTestMonth, &iTestYear, &iTestVersion);

				if (iTestDay == iDay && iTestMonth == iMonth && iTestYear == iYear && iTestVersion == iVersion) {

					iErrCode = m_pGameData->DeleteBackup (ppBackups[i]);
					break;
				}
			}
		}
		pBackupEnumerator->Release();
	}

	if (iEmpireKey != SYSTEM) {

		char pszVersion[8];
		if (iVersion == 0) {
			pszVersion[0] = '\0';
		} else {
			sprintf (pszVersion, "(%i)", iVersion);
		}

		char pszMessage [1024];
		sprintf (pszMessage, "The %i_%i_%i%s backup ", iYear, iMonth, iDay, pszVersion);

		if (iErrCode == OK) {

			strcat (pszMessage, "was deleted");
			SendSystemMessage (iEmpireKey, pszMessage, SYSTEM);

		} else {

			char pszErrorCode [32];
			itoa (iErrCode, pszErrorCode, 10);

			strcat (pszMessage, "could not be deleted; the error code was ");
			strcat (pszMessage, pszErrorCode);
			
			SendSystemMessage (iEmpireKey, pszMessage, SYSTEM);
		}
	}

	return iErrCode;
}


// Output:
// *piNumEmpires -> Number of empires deleted
//
// Remove all non-Admin empires with one of the following two characteristics:
// 1) MaxEcon == 0 (means that they haven't even played a single update)
// 2) LastLogin is more than thirty days ago
//
// Run this with caution  :-)

int GameEngine::PurgeDatabase (int iEmpireKey, int iCriteria) {

	DatabasePurge* pPurgeInfo = new DatabasePurge;
	if (pPurgeInfo == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}
	
	pPurgeInfo->Criteria = iCriteria; 
	pPurgeInfo->EmpireKey = iEmpireKey;

	return SendLongRunningQueryMessage (PurgeDatabaseMsg, pPurgeInfo);
}


int GameEngine::PurgeDatabaseMsg (LongRunningQueryMessage* pMessage) {

	DatabasePurge* pPurgeInfo = (DatabasePurge*) pMessage->Arguments;

	int iErrCode = pMessage->GameEngine->PurgeDatabasePrivate (
		pPurgeInfo->EmpireKey,
		pPurgeInfo->Criteria
		);

	delete pPurgeInfo;
	return iErrCode;
}


int GameEngine::PurgeDatabasePrivate (int iEmpireKey, int iCriteria) {

	int iErrCode = OK;
	unsigned int iNumEmpiresDeleted = 0, iNumEmpiresNotDeleted = 0;

	if (iCriteria != 0 && iCriteria != TEST_PURGE_ONLY) {

		UTCTime tNow;
		Time::GetTime (&tNow);

		const Seconds sThirtyDays = 30 * 24 * 60 * 60;

		unsigned int* piEmpireKey, i, iNumKeys = 0;

		iErrCode = m_pGameData->GetAllKeys (
			SYSTEM_EMPIRE_DATA, 
			&piEmpireKey, 
			&iNumKeys
			);
		
		if (iErrCode == OK) {

			Variant vValue;
			unsigned int iValue;
			NamedMutex nmLockEmpire;

			char pszGames [256];

			for (i = 0; i < iNumKeys; i ++) {

				LockEmpire (piEmpireKey[i], &nmLockEmpire);
				
				// Never purge administrators
				iErrCode = m_pGameData->ReadData (
					SYSTEM_EMPIRE_DATA, 
					piEmpireKey[i], 
					SystemEmpireData::Privilege, 
					&vValue
					);
				
				if (iErrCode != OK || vValue.GetInteger() >= ADMINISTRATOR) {
					goto Unlock;
				}

				GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, piEmpireKey[i]);

				// Never purge an empire in a game
				iErrCode = m_pGameData->GetNumRows (pszGames, &iValue);
				if (iErrCode != OK || iValue > 0) {
					goto Unlock;
				}

				// Never played a game
				if (iCriteria & NEVER_PLAYED_A_GAME) {

					// If max econ = 0, then this empire never played a game
					iErrCode = m_pGameData->ReadData (
						SYSTEM_EMPIRE_DATA, 
						piEmpireKey[i],
						SystemEmpireData::MaxEcon, 
						&vValue
						);
					
					if (iErrCode != OK || vValue.GetInteger() > 0) {
						goto Unlock;
					}
				}
				
				// Never won a game
				if (iCriteria & NEVER_WON_A_GAME) {

					iErrCode = m_pGameData->ReadData (
						SYSTEM_EMPIRE_DATA, 
						piEmpireKey[i],
						SystemEmpireData::Wins, 
						&vValue
						);
					
					if (iErrCode != OK || vValue.GetInteger() > 0) {
						goto Unlock;
					}
				}

				// Only logged in once
				if (iCriteria & ONLY_ONE_LOGIN) {

					iErrCode = m_pGameData->ReadData (
						SYSTEM_EMPIRE_DATA, 
						piEmpireKey[i],
						SystemEmpireData::NumLogins, 
						&vValue
						);
					
					if (iErrCode != OK || vValue.GetInteger() > 1) {
						goto Unlock;
					}
				}

				// Bad score
				if (iCriteria & CLASSIC_SCORE_IS_ZERO_OR_LESS) {

					iErrCode = m_pGameData->ReadData (
						SYSTEM_EMPIRE_DATA, 
						piEmpireKey[i],
						SystemEmpireData::ClassicScore, 
						&vValue
						);
					
					if (iErrCode != OK || vValue.GetFloat() <= 0) {
						goto Unlock;
					}
				}

				// Last logged in a while ago
				if (iCriteria & LAST_LOGGED_IN_A_MONTH_AGO) {

					iErrCode = m_pGameData->ReadData (
						SYSTEM_EMPIRE_DATA, 
						piEmpireKey[i], 
						SystemEmpireData::LastLoginTime, 
						&vValue
						);

					if (iErrCode != OK || 
						Time::GetSecondDifference (tNow, vValue.GetUTCTime()) < sThirtyDays) {
						goto Unlock;
					}
				}

				// Best effort delete the empire
				if (!(iCriteria & TEST_PURGE_ONLY)) {
					LockEmpires();
					iErrCode = RemoveEmpire (piEmpireKey[i]);
					UnlockEmpires();
				}

				if (iErrCode == OK) {
					iNumEmpiresDeleted ++;
				} else {
					iErrCode = OK;
					iNumEmpiresNotDeleted ++;
				}
Unlock:
				UnlockEmpire (nmLockEmpire);
			}

			m_pGameData->FreeKeys (piEmpireKey);

		}	// End if get all keys returned OK
		else if (iErrCode == ERROR_DATA_NOT_FOUND) iErrCode = OK;
	}

	// Send message to originator of purge
	char pszText [256];

	if (iErrCode != OK) {

		if (iCriteria & TEST_PURGE_ONLY) {
			sprintf (pszText, "Error %i occurred testing a database purge", iErrCode);
		} else {
			sprintf (pszText, "Error %i occurred purging the database", iErrCode);
		}

	} else {
		
		if (iCriteria & TEST_PURGE_ONLY) {

			if (iNumEmpiresDeleted == 1) {
				strcpy (pszText, "1 empire would have been purged from the database");
			} else {
				sprintf (pszText, "%i empires would have been purged from the database", iNumEmpiresDeleted);
			}

		} else {
			
			if (iNumEmpiresDeleted == 1) {
				strcpy (pszText, "1 empire was purged from the database");
			} else {
				sprintf (pszText, "%i empires were purged from the database", iNumEmpiresDeleted);
			}
			
			if (iNumEmpiresNotDeleted > 0) {
				
				char pszNum [64];
				
				strcat (pszText, "\n");
				strcat (pszText, itoa (iNumEmpiresNotDeleted, pszNum, 10));
				strcat (pszText, " empires could not be purged");
			}
		}
	}

	// Best effort send message
	SendSystemMessage (iEmpireKey, pszText, SYSTEM);

	return iErrCode;
}


int GameEngine::AutomaticBackup (void* pvGameEngine) {

	return ((GameEngine*) pvGameEngine)->AutomaticBackup();
}

int GameEngine::AutomaticBackup() {

	MilliSeconds iMilliSecondsPause = m_scConfig.iSecondsBetweenBackups * 1000;

	if (iMilliSecondsPause == 0) {
		Assert (false);
		return ERROR_FAILURE;
	}
	
	// This function will never exit.  It will be terminated by the destructor, since there's
	// no point in waiting hours for Sleep() to exit and this thread owns no storage anyway
	int iErrCode;

	if (m_scConfig.bBackupOnStartup) {

		// Backup
		iErrCode = BackupDatabasePrivate (SYSTEM);
		if (iErrCode == OK) {
			m_pReport->WriteReport ("Autobackup: Successfully backed up database");
		} else {

			char pszText [256];
			sprintf (pszText, "Autobackup: Error %i backing up database", iErrCode);

			m_pReport->WriteReport (pszText);
		}
	}

	while (true) {

		// Delete old backups
		DeleteOldDatabaseBackups();

		// Sleep for at least the specified time
		if (m_eAutoBackupEvent.Wait (iMilliSecondsPause) == OK) {

			// We were signalled - leave
			break;
		}

		// Backup
		iErrCode = BackupDatabasePrivate (SYSTEM);
		if (iErrCode == OK) {
			m_pReport->WriteReport ("Autobackup: Successfully backed up database");
		} else {

			char pszText [256];
			sprintf (pszText, "Autobackup: Error %i backing up database", iErrCode);

			m_pReport->WriteReport (pszText);
		}
	}

	return OK;
}

int GameEngine::LongRunningQueryProcessor (void* pVoid) {

	return ((GameEngine*) pVoid)->LongRunningQueryProcessorLoop();
}

int GameEngine::LongRunningQueryProcessorLoop() {

	LongRunningQueryMessage* plrqMessage;
	bool bExit = false;

	while (!bExit) {

		// Wait for action
		m_eQueryEvent.Wait();

		// Process messages
		while (true) {

			// Lock queue and get message
			if (!m_tsfqQueryQueue.Pop (&plrqMessage)) {
				// Back to sleep
				break;
			}

			// Process message
			if (plrqMessage == NULL) {
				
				// Shutting down
				bExit = true;
				break;
			}

			plrqMessage->QueryCall (plrqMessage);
			delete plrqMessage;
		}
	}

	return OK;
}

int GameEngine::SendLongRunningQueryMessage (Fxn_QueryCallBack pfxnFunction, void* pVoid) {

	// Build the message
	LongRunningQueryMessage* pMessage = new LongRunningQueryMessage;
	if (pMessage == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	pMessage->Arguments = pVoid;
	pMessage->QueryCall = pfxnFunction;
	pMessage->GameEngine = this;

	// Push message into the queue
	if (!m_tsfqQueryQueue.Push (pMessage)) {
		delete pMessage;
		return ERROR_OUT_OF_MEMORY;
	}

	// Signal the event
	m_eQueryEvent.Signal();

	return OK;
}



void GameEngine::BeginBackup (const char* pszBackupDirectory) {

	m_iNumTemplates = 0;
	m_iMaxNumTemplates = 0;
	m_iNumTables = 0;
	m_iMaxNumTables = 0;

	Time::GetTime (&m_tBackupStartTime);
}

void GameEngine::BeginTemplateBackup (unsigned int iNumTemplates) {
	m_iMaxNumTemplates = iNumTemplates;
}

void GameEngine::BackupTemplate (const char* pszTemplateName) {
	m_iNumTemplates ++;
}

void GameEngine::EndTemplateBackup() {
}

void GameEngine::BeginTableBackup (unsigned int iNumTables) {
	m_iMaxNumTables = iNumTables;
}

void GameEngine::BackupTable (const char* pszTableName) {
	m_iNumTables ++;
}

void GameEngine::EndTableBackup() {
}

void GameEngine::EndBackup (IDatabaseBackup* pBackup) {

	m_iNumTemplates = 0;
	m_iMaxNumTemplates = 0;
	m_iNumTables = 0;
	m_iMaxNumTables = 0;
}

void GameEngine::AbortBackup (int iErrCode) {

	char pszText [256];
	sprintf (pszText, "A database backup failed with error code %i", iErrCode);

	m_pReport->WriteReport (pszText);

	m_iNumTemplates = 0;
	m_iMaxNumTemplates = 0;
	m_iNumTables = 0;
	m_iMaxNumTables = 0;
}

bool GameEngine::IsDatabaseBackingUp() {
	return m_bActiveBackup;
}

void GameEngine::GetDatabaseBackupProgress (unsigned int* piNumTemplates, unsigned int* piMaxNumTemplates, 
											unsigned int* piNumTables, unsigned int* piMaxNumTables,
											Seconds* piElapsedTime) {

	*piNumTemplates = m_iNumTemplates;
	*piMaxNumTemplates = m_iMaxNumTemplates;
	*piNumTables = m_iNumTables;
	*piMaxNumTables = m_iMaxNumTables;

	UTCTime tNow;
	Time::GetTime (&tNow);

	*piElapsedTime = Time::GetSecondDifference (tNow, m_tBackupStartTime);
}