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

void GameEngine::LockGame (int iGameClass, int iGameNumber, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "Game%i.%i", iGameClass, iGameNumber);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockGame (const NamedMutex& nmMutex) {
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

void GameEngine::LockEmpireShips (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "GameEmpireShips%i.%i.%i", iGameClass, iGameNumber, iEmpireKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockEmpireShips (const NamedMutex& nmMutex) {
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

void GameEngine::LockAutoUpdate (int iGameClass, int iGameNumber, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "AutoUpdate%i.%i", iGameClass, iGameNumber);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockAutoUpdate (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockPauseGame (int iGameClass, int iGameNumber, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "PauseGame%i.%i", iGameClass, iGameNumber);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockPauseGame (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockDrawGame (int iGameClass, int iGameNumber, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "DrawGame%i.%i", iGameClass, iGameNumber);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockDrawGame (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockEmpireUpdated (int iGameClass, int iGameNumber, int iEmpireKey, 
                                          NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "UpdateEmpire%i.%i.%i", iGameClass, iGameNumber, iEmpireKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockEmpireUpdated (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockEmpireTechs (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "EmpireTechs%i.%i.%i", iGameClass, iGameNumber, iEmpireKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockEmpireTechs (const NamedMutex& nmMutex) {
    Mutex::Signal (nmMutex);
}

void GameEngine::LockEmpireFleets (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex) {

    char pszLock [256];
    sprintf (pszLock, "EmpireFleets%i.%i.%i", iGameClass, iGameNumber, iEmpireKey);

    Mutex::Wait (pszLock, pnmMutex);
}

void GameEngine::UnlockEmpireFleets (const NamedMutex& nmMutex) {
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


int GameEngine::WaitGameReader (int iGameClass, int iGameNumber) {

    // Get the object
    GameObject* pGameObject = GetGameObject (iGameClass, iGameNumber);
    if (pGameObject == NULL) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    pGameObject->WaitReader();
    pGameObject->Release();

    return OK;
}


int GameEngine::SignalGameReader (int iGameClass, int iGameNumber) {

    // Get the object
    GameObject* pGameObject = GetGameObject (iGameClass, iGameNumber);
    if (pGameObject == NULL) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    pGameObject->SignalReader();
    pGameObject->Release();

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
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    // Set database state - this should succeed
    iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_WAITING_TO_UPDATE);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_UPDATING);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
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