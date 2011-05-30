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

    return SendLongRunningQueryMessage (FlushDatabaseMsg, (void*) (size_t) iEmpireKey);
}

int GameEngine::FlushDatabaseMsg (LongRunningQueryMessage* pMessage) {

    return pMessage->pGameEngine->FlushDatabasePrivate ((int) (size_t) pMessage->pArguments);
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

    return SendLongRunningQueryMessage (BackupDatabaseMsg, (void*) (size_t) iEmpireKey);
}

int GameEngine::BackupDatabaseMsg (LongRunningQueryMessage* pMessage) {

    return pMessage->pGameEngine->BackupDatabasePrivate ((int) (size_t) pMessage->pArguments);
}

int GameEngine::BackupDatabasePrivate (int iEmpireKey) {

    int iErrCode = OK;
    bool bInternal = iEmpireKey == SYSTEM, bFlag, * pbUnpause = NULL;

    char pszText [128];

    int* piGameClass = NULL, * piGameNumber = NULL, iNumGames = 0, i, iErrCode2 = OK;

    // Set ourselves as high priority
    Thread tSelf;
    Thread::GetCurrentThread (&tSelf);

    Thread::ThreadPriority tpPriority = tSelf.GetPriority();
    tSelf.SetPriority (Thread::HigherPriority);

    // Update all games
    CheckAllGamesForUpdates (true);

    m_bActiveBackup = true;

    // When this function returns, we'll be the only thread active
    m_pPageSourceControl->LockWithNoThreads();
    bool bPageSrcLocked = true;

    // Start timer
    Timer tTimer;
    Time::StartTimer (&tTimer);

    // Talk to the unwashed masses
    iErrCode = SetSystemProperty (SystemData::AccessDisabledReason, "The server is being backed up");
    if (iErrCode != OK) {
        Assert (false);
        goto End;
    }

    iErrCode = SetSystemProperty (SystemData::NewEmpiresDisabledReason, "The server is being backed up");
    if (iErrCode != OK) {
        Assert (false);
        goto End;
    }

    iErrCode = SetSystemOption (ACCESS_ENABLED | NEW_EMPIRES_ENABLED, false);
    if (iErrCode != OK) {
        Assert (false);
        goto End;
    }

    iErrCode = GetActiveGames (&piGameClass, &piGameNumber, &iNumGames);
    if (iErrCode != OK) {
        Assert (false);
        goto End;
    }

    pbUnpause = (bool*) StackAlloc (iNumGames * sizeof (bool));
    memset (pbUnpause, 0, iNumGames * sizeof (bool));

    if (iNumGames > 0) {

        int iErrCode2;

        // Best effort
        for (i = 0; i < iNumGames; i ++) {

            iErrCode2 = WaitGameReader (piGameClass[i], piGameNumber[i], NO_KEY, NULL);
            if (iErrCode2 == OK) {

                if (IsGameAdminPaused (piGameClass[i], piGameNumber[i], &bFlag) == OK && 
                    !bFlag &&
                    PauseGame (piGameClass[i], piGameNumber[i], true, false) == OK) {

                    pbUnpause[i] = true;
                }

                SignalGameReader (piGameClass[i], piGameNumber[i], NO_KEY, NULL);
            }
        }
    }

    // Let the unwashed masses receive responses
    m_pPageSourceControl->ReleaseLock();
    bPageSrcLocked = false;

    // Tell the database to back itself up:  when this terminates it will have finished
    iErrCode = m_pGameData->Backup (this, m_scConfig.bCheckDatabase);
    Assert (iErrCode == OK);

    m_bActiveBackup = false;

    // Best effort let the unwashed masses play again
    iErrCode2 = SetSystemOption (ACCESS_ENABLED | NEW_EMPIRES_ENABLED, true);
    Assert (iErrCode2 == OK);

    iErrCode2 = iErrCode = SetSystemProperty (SystemData::AccessDisabledReason, (const char*) NULL);
    Assert (iErrCode2 == OK);

    iErrCode2 = iErrCode = SetSystemProperty (SystemData::NewEmpiresDisabledReason, (const char*) NULL);
    Assert (iErrCode2 == OK);

    if (iNumGames > 0) {

        for (i = 0; i < iNumGames; i ++) {

            if (pbUnpause[i] && WaitGameReader (piGameClass[i], piGameNumber[i], NO_KEY, NULL) == OK) {

                iErrCode2 = UnpauseGame (piGameClass[i], piGameNumber[i], true, false);
                Assert (iErrCode2 == OK);

                SignalGameReader (piGameClass[i], piGameNumber[i], NO_KEY, NULL);
            }
        }

        delete [] piGameClass;
        delete [] piGameNumber;
    }

    // Reset priority
    tSelf.SetPriority (tpPriority);

End:

    MilliSeconds ms = Time::GetTimerCount (tTimer);

    // Send message to originator
    if (!bInternal && iEmpireKey != NO_KEY && iEmpireKey != SYSTEM) {

        const char* pszMessage;

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

    Seconds sec = ms / 1000;

    // Inform report file
    sprintf (pszText, "The database was backed up in %i second%c", sec, sec == 1 ? '\0' : 's');
    m_pReport->WriteReport (pszText);

    if (bPageSrcLocked) {
        m_pPageSourceControl->ReleaseLock();
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

    DatabaseBackupInfo* pInfo = (DatabaseBackupInfo*) pMessage->pArguments;

    int iErrCode = pMessage->pGameEngine->RestoreDatabaseBackupPrivate (
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

    DatabaseBackupInfo* pInfo = (DatabaseBackupInfo*) pMessage->pArguments;

    int iErrCode = pMessage->pGameEngine->DeleteDatabaseBackupPrivate (
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

    DatabasePurge* pPurgeInfo = (DatabasePurge*) pMessage->pArguments;

    int iErrCode = pMessage->pGameEngine->PurgeDatabasePrivate (
        pPurgeInfo->EmpireKey,
        pPurgeInfo->Criteria
        );

    delete pPurgeInfo;
    return iErrCode;
}


int GameEngine::PurgeDatabasePrivate (int iEmpireKey, int iCriteria) {

    int iErrCode = OK;
    unsigned int iNumEmpiresDeleted = 0;

    char pszText [256];

    if ((iCriteria & (~TEST_PURGE_ONLY)) != 0) {

        const Seconds sThirtyDays = 30 * 24 * 60 * 60;
        unsigned int iEmpireKey = NO_KEY;

        IReadTable* pEmpires = NULL;

        int iValue;
        float fValue;
        UTCTime tValue, tNow;

        Time::GetTime (&tNow);

        while (true) {

            int64 i64SecretKey;

            SafeRelease (pEmpires);

            iErrCode = m_pGameData->GetTableForReading (SYSTEM_EMPIRE_DATA, &pEmpires);
            if (iErrCode != OK) {
                Assert (false);
                continue;
            }

            iErrCode = pEmpires->GetNextKey (iEmpireKey, &iEmpireKey);
            if (iErrCode != OK) {
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    iErrCode = OK;
                } else Assert (false);
                break;
            }

            // Never purge administrators
            iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::Privilege, &iValue);
            if (iErrCode != OK || iValue >= ADMINISTRATOR) {
                continue;
            }
            
            // Never played a game
            if (iCriteria & NEVER_PLAYED_A_GAME) {
                
                // If max econ = 0, then this empire never played a game
                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::MaxEcon, &iValue);
                if (iErrCode != OK || iValue > 0) {
                    continue;
                }
            }
            
            // Never won a game
            if (iCriteria & NEVER_WON_A_GAME) {

                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::Wins, &iValue);
                if (iErrCode != OK || iValue > 0) {
                    continue;
                }
            }
            
            // Only logged in once
            if (iCriteria & ONLY_ONE_LOGIN) {

                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::NumLogins, &iValue);
                if (iErrCode != OK || iValue > 1) {
                    continue;
                }
            }
            
            // Bad score
            if (iCriteria & CLASSIC_SCORE_IS_ZERO_OR_LESS) {
                
                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::ClassicScore, &fValue);
                if (iErrCode != OK || fValue <= 0) {
                    continue;
                }
            }
            
            // Last logged in a while ago
            if (iCriteria & LAST_LOGGED_IN_A_MONTH_AGO) {

                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::LastLoginTime, &tValue);
                if (iErrCode != OK || Time::GetSecondDifference (tNow, tValue) < sThirtyDays) {
                    continue;
                }
            }

            // Secret key
            iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::SecretKey, &i64SecretKey);
            if (iErrCode != OK) {
                continue;
            }

            SafeRelease (pEmpires);

            GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszText, iEmpireKey);

            if (m_pGameData->DoesTableExist (pszText)) {

                // Never purge an empire in a game
                unsigned int iNumGames;
                iErrCode = m_pGameData->GetNumRows (pszText, &iNumGames);
                if (iErrCode != OK || iNumGames > 0) {
                    continue;
                }
            }
            
            // Best effort delete the empire
            if (!(iCriteria & TEST_PURGE_ONLY)) {
                iErrCode = DeleteEmpire (iEmpireKey, &i64SecretKey, true, false);
            }
            
            if (iErrCode == OK) {
                iNumEmpiresDeleted ++;
            }
        }

        SafeRelease (pEmpires);
        iErrCode = OK;
    }

    // Send message to originator of purge       
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

            plrqMessage->pQueryCall (plrqMessage);
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

    pMessage->pArguments = pVoid;
    pMessage->pQueryCall = pfxnFunction;
    pMessage->pGameEngine = this;

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

    m_dbsStage = DATABASE_BACKUP_NONE;
    m_iMaxNumTemplates = 0;
    m_iMaxNumTables = 0;

    Time::GetTime (&m_tBackupStartTime);
}

void GameEngine::BeginTemplateBackup (unsigned int iNumTemplates) {

    m_dbsStage = DATABASE_BACKUP_TEMPLATES;
    m_iMaxNumTemplates = iNumTemplates;
}

void GameEngine::EndTemplateBackup() {
}

void GameEngine::BeginTableBackup (unsigned int iNumTables) {

    m_dbsStage = DATABASE_BACKUP_TABLES;
    m_iMaxNumTables = iNumTables;
}

void GameEngine::EndTableBackup() {
}

void GameEngine::BeginVariableLengthDataBackup() {

    m_dbsStage = DATABASE_BACKUP_VARIABLE_LENGTH_DATA;
}

void GameEngine::EndVariableLengthDataBackup() {
}

void GameEngine::BeginMetaDataBackup() {

    m_dbsStage = DATABASE_BACKUP_METADATA;
}

void GameEngine::EndMetaDataBackup() {
}

void GameEngine::EndBackup (IDatabaseBackup* pBackup) {

    m_dbsStage = DATABASE_BACKUP_NONE;
    m_iMaxNumTemplates = 0;
    m_iMaxNumTables = 0;
}

void GameEngine::AbortBackup (int iErrCode) {

    char pszText [96];
    sprintf (pszText, "A database backup failed with error code %i", iErrCode);

    m_pReport->WriteReport (pszText);

    m_iMaxNumTemplates = 0;
    m_iMaxNumTables = 0;
}

bool GameEngine::IsDatabaseBackingUp() {
    return m_bActiveBackup;
}

void GameEngine::GetDatabaseBackupProgress (DatabaseBackupStage* pdbsStage, Seconds* piElapsedTime, 
                                            unsigned int* piNumber) {

    UTCTime tNow;
    Time::GetTime (&tNow);

    *pdbsStage = m_dbsStage;
    *piElapsedTime = Time::GetSecondDifference (tNow, m_tBackupStartTime);

    switch (m_dbsStage) {

    case DATABASE_BACKUP_TABLES:
        *piNumber = m_iMaxNumTables;
        break;

    case DATABASE_BACKUP_TEMPLATES:
        *piNumber = m_iMaxNumTemplates;
        break;

    default:
        *piNumber = 0;
        break;
    }

    *piElapsedTime = Time::GetSecondDifference (tNow, m_tBackupStartTime);
}