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

void GameEngine::LockGameClass (int iGameClass, NamedMutex* pnmMutex) {
    
    char pszLock [256];
    sprintf (pszLock, "GameClass%i", iGameClass);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockGameClass (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockGameClasses() {
    m_mGameClasses.Wait();
}

void GameEngine::UnlockGameClasses() {
    m_mGameClasses.Signal();
}

void GameEngine::LockSuperClasses() {
    m_mSuperClasses.Wait();
}

void GameEngine::UnlockSuperClasses() {
    m_mSuperClasses.Signal();
}

void GameEngine::LockEmpireSystemMessages (int iEmpireKey, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "Game%i", iEmpireKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockEmpireSystemMessages (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockEmpireBridier (int iEmpireKey, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "Bridier%i", iEmpireKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockEmpireBridier (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockEmpireGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, 
                                               NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "GameMessages%i.%i.%i", iGameClass, iGameNumber, iEmpireKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockEmpireGameMessages (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockEmpires() {
    m_mEmpires.Wait();
}

void GameEngine::UnlockEmpires() {
    m_mEmpires.Signal();
}

void GameEngine::LockAlienIcons() {
    m_mAlienIcons.Wait();
}

void GameEngine::UnlockAlienIcons() {
    m_mAlienIcons.Signal();
}

void GameEngine::LockTournament (unsigned int iTournamentKey, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "Tournament%i", iTournamentKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockTournament (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockEmpire (int iEmpireKey, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "EmpireLock%i", iEmpireKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockEmpire (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}


int GameEngine::WaitGameReader (int iGameClass, int iGameNumber, int iEmpireKey, GameEmpireLock** ppgeLock) {

    // Lock the game
    GameObject* pGameObject = GetGameObject (iGameClass, iGameNumber);
    if (pGameObject == NULL) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }
    pGameObject->WaitReader();
    
    if (iEmpireKey != NO_KEY) {

        Assert (ppgeLock != NULL);

        // Lock the empire
        GameEmpireId geId;
        geId.iEmpireKey = iEmpireKey;

        GameEmpireLock* pgeLock = m_lockMgr.GetGameEmpireLock (geId);
        if (pgeLock == NULL) {
            pGameObject->SignalReader();
            return ERROR_OUT_OF_MEMORY;
        }
        pgeLock->Wait();

        Assert (*ppgeLock == NULL);
        *ppgeLock = pgeLock;
    }

    pGameObject->Release();

    return OK;
}


int GameEngine::SignalGameReader (int iGameClass, int iGameNumber, int iEmpireKey, GameEmpireLock* pgeLock) {

    // Get the object
    GameObject* pGameObject = GetGameObject (iGameClass, iGameNumber);
    if (pGameObject == NULL) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    pGameObject->SignalReader();
    pGameObject->Release();

    if (iEmpireKey != NO_KEY) {

        Assert (pgeLock != NULL);

        pgeLock->Signal();
        m_lockMgr.ReleaseGameEmpireLock (pgeLock);
    }

    return OK;
}


int GameEngine::WaitGameWriter (int iGameClass, int iGameNumber) {

    // Get a writer lock on the game
    GameObject* pGameObject = GetGameObject (iGameClass, iGameNumber);
    if (pGameObject == NULL) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    pGameObject->WaitWriter();
    pGameObject->Release();

    return OK;
}


int GameEngine::SignalGameWriter (int iGameClass, int iGameNumber) {

    // Get game object
    GameObject* pGameObject = GetGameObject (iGameClass, iGameNumber);
    if (pGameObject == NULL) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    // Release our writer lock
    pGameObject->SignalWriter();
    pGameObject->Release();

    return OK;  
}

int GameEngine::WaitForUpdate (int iGameClass, int iGameNumber) {

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Write state to database
    int iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_WAITING_TO_UPDATE);
    if (iErrCode != OK) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    iErrCode = WaitGameWriter (iGameClass, iGameNumber);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Now we have a lock on the game;  make sure it still exists
    bool bExists;
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bExists);
    if (iErrCode != OK || !bExists) {
        iErrCode = ERROR_GAME_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Set database state - this should succeed
    iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_WAITING_TO_UPDATE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_UPDATING);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    if (iErrCode != OK) {
        SignalAfterUpdate (iGameClass, iGameNumber);
    }

    return iErrCode;
}

int GameEngine::SignalAfterUpdate (int iGameClass, int iGameNumber) {

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Best effort - set game state while we hold the writer lock
    m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_UPDATING);
    
    return SignalGameWriter (iGameClass, iGameNumber);
}

void GameEngine::LockGameConfigurationForReading() {

    m_rwGameConfigLock.WaitReader();
}

void GameEngine::UnlockGameConfigurationForReading() {

    m_rwGameConfigLock.SignalReader();
}

void GameEngine::LockGameConfigurationForWriting() {

    m_rwGameConfigLock.WaitWriter();
}

void GameEngine::UnlockGameConfigurationForWriting() {

    m_rwGameConfigLock.SignalWriter();
}

void GameEngine::LockMapConfigurationForReading() {

    m_rwMapConfigLock.WaitReader();
}

void GameEngine::UnlockMapConfigurationForReading() {

    m_rwMapConfigLock.SignalReader();
}

void GameEngine::LockMapConfigurationForWriting() {

    m_rwMapConfigLock.WaitWriter();
}

void GameEngine::UnlockMapConfigurationForWriting() {

    m_rwMapConfigLock.SignalWriter();
}


//
// GameEmpireLock
//

GameEmpireLock::GameEmpireLock() {

    m_geId.iEmpireKey = NO_KEY;
    m_bInUse = false;
}

GameEmpireLock* GameEmpireLock::Create() {

    GameEmpireLock* pgeLock = new GameEmpireLock();
    if (pgeLock == NULL) {
        return NULL;
    }

    int iErrCode = pgeLock->Initialize();
    if (iErrCode != OK) {
        delete pgeLock;
        return NULL;
    }

    return pgeLock;
}

//
// GameEmpireLockManager
//

GameEmpireLockManager::GameEmpireLockManager() : m_htLocks (NULL, NULL) {
}

GameEmpireLockManager::~GameEmpireLockManager() {
}

int GameEmpireLockManager::Initialize (const LockManagerConfig& lmConf) {
    
    m_lmConf = lmConf;

    int iErrCode = m_rwHTLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_mOCLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    unsigned int iNumObjects = max (m_lmConf.iNumEmpiresHint, 200);

    if (!m_oCache.Initialize (iNumObjects / 4)) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (!m_htLocks.Initialize (iNumObjects / 2)) {
        return ERROR_OUT_OF_MEMORY;
    }

    // Start the scan thread
    iErrCode = m_tScanThread.Start (ScanThread, this);

    return iErrCode;
}

void GameEmpireLockManager::Shutdown() {

    m_eShutdown.Signal();
    m_tScanThread.WaitForTermination();

    // Clean up the hash table (take the lock for safety)
    m_rwHTLock.WaitWriter();

    LMHashTableIterator itor;
    while (m_htLocks.GetNextIterator (&itor)) {
        FreeLock (itor.GetData());
    }

    m_htLocks.Clear();

    m_rwHTLock.SignalWriter();
}

GameEmpireLock* GameEmpireLockManager::GetGameEmpireLock (const GameEmpireId& geId) {

    GameEmpireLock* pgeLock = NULL, * pgeLockRelease = NULL;
    LMHashTableIterator itor;

    // First, try to look it up
    m_rwHTLock.WaitReader();

    if (m_htLocks.FindFirst (&geId, &itor)) {
        pgeLock = itor.GetData();
        pgeLock->SetInUse (true);
        pgeLock->SetActive (true);
    }

    m_rwHTLock.SignalReader();

    if (pgeLock == NULL) {

        // Create a new object and try to insert it
        pgeLock = FindOrCreateLock();
        if (pgeLock == NULL) {
            return NULL;
        }
        pgeLock->SetId (geId);

        m_rwHTLock.WaitWriter();

        // Again, try to look it up
        if (m_htLocks.FindFirst (&geId, &itor)) {
            
            pgeLockRelease = pgeLock;
            pgeLock = itor.GetData();
            pgeLock->SetInUse (true);
            pgeLock->SetActive (true);
        
        } else {

            if (!m_htLocks.Insert (&pgeLock->GetId(), pgeLock)) {
                pgeLockRelease = pgeLock;
                pgeLock = NULL;
            } else {
                pgeLock->SetInUse (true);
                pgeLock->SetActive (true);
            }
        }

        m_rwHTLock.SignalWriter();

        if (pgeLockRelease != NULL) {
            FreeLock (pgeLockRelease);
        }
    }

    Assert (pgeLock == NULL || pgeLock->GetId() == geId);

    return pgeLock;
}

void GameEmpireLockManager::ReleaseGameEmpireLock (GameEmpireLock* pgeLock) {

    pgeLock->SetLastAccess();
    pgeLock->SetInUse (false);
}

GameEmpireLock* GameEmpireLockManager::FindOrCreateLock() {

    m_mOCLock.Wait();
    GameEmpireLock* pgeLock = m_oCache.GetObject();
    m_mOCLock.Signal();

    return pgeLock;
}

void GameEmpireLockManager::FreeLock (GameEmpireLock* pgeLock) {

    m_mOCLock.Wait();
    m_oCache.ReleaseObject (pgeLock);
    m_mOCLock.Signal();
}

int GameEmpireLockManager::ScanThread (void* pThis) {
    ((GameEmpireLockManager*) pThis)->Scan();
    return OK;
}

void GameEmpireLockManager::Scan() {

    UTCTime tNow;
    Timer timer;
    LMHashTableIterator itor;

    const unsigned int iMaxNumAgedOut = m_lmConf.iMaxAgedOutPerScan;
    unsigned int iNumAgedOut;

    GameEmpireLock* pgeLock = NULL;

    GameEmpireId* ageTimeOut = (GameEmpireId*) StackAlloc (iMaxNumAgedOut * sizeof (GameEmpireId));
    GameEmpireLock** apgeLock = (GameEmpireLock**) StackAlloc (iMaxNumAgedOut * sizeof (GameEmpireLock*));

    while (true) {

        // Sleep for the scan period, exit if the event is signalled
        if (m_eShutdown.Wait (m_lmConf.msScanPeriod) == OK) {
            break;
        }

        iNumAgedOut = 0;

        m_rwHTLock.WaitReader();

        Time::GetTime (&tNow);
        Time::StartTimer (&timer);

        while (true) {

            if (!m_htLocks.GetNextIterator (&itor)) {
                itor.Reset();
                break;
            }
            pgeLock = itor.GetData();

            if (!pgeLock->InUse() && 
                (!pgeLock->Active() ||
                Time::GetSecondDifference (tNow, pgeLock->GetLastAccess()) > m_lmConf.sAgeOutPeriod)) {
                
                // Add to aged out list
                ageTimeOut [iNumAgedOut] = pgeLock->GetId();
                iNumAgedOut ++;

                // See if we've added enough entries already
                if (iNumAgedOut == iMaxNumAgedOut) {
                    break;
                }
            }

            // See if we've overstayed our welcome holding the read lock
            if (Time::GetTimerCount (timer) > m_lmConf.msMaxScanTime) {
                itor.Freeze();
                break;
            }
        }

        m_rwHTLock.SignalReader();

        if (iNumAgedOut > 0) {

            unsigned int i, iLoopGuard = iNumAgedOut;
            iNumAgedOut = 0;

            m_rwHTLock.WaitWriter();
            Time::GetTime (&tNow);

            for (i = 0; i < iLoopGuard; i ++) {

                if (!m_htLocks.FindFirst (ageTimeOut + i, &itor)) {
                    continue;
                }
                pgeLock = itor.GetData();

                if (!pgeLock->InUse() &&
                    (!pgeLock->Active() ||
                    Time::GetSecondDifference (tNow, pgeLock->GetLastAccess()) > m_lmConf.sAgeOutPeriod)) {

                    // Remove from the hash table
                    bool bRemove = m_htLocks.Delete (&itor, NULL, NULL);
                    Assert (bRemove);

                    apgeLock [iNumAgedOut] = pgeLock;
                    iNumAgedOut ++;
                }
            }

            m_rwHTLock.SignalWriter();

            // Return the aged-out locks to the cache
            for (i = 0; i < iNumAgedOut; i ++) {
                FreeLock (apgeLock[i]);
            }
        }

    }   // End forever loop
}