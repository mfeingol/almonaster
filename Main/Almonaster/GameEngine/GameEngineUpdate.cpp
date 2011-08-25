//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
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

// Input:
// iGameClass -> Gameclass
// iGameNumber - > Gamenumber
//
// Output:
// *pbUpdate -> true if an update occurred
//
// Executes an update if one has occurred.  Also updates game lastlogin data

int GameEngine::CheckGameForUpdates (int iGameClass, int iGameNumber, bool fUpdateCheckTime, bool* pbUpdate) {

    int iErrCode;

    *pbUpdate = false;

    bool bGameOver = false, bExist;
    int iSecondsSince, iSecondsUntil, iNumUpdates, iState;

    Variant vNumPrevUpdates;

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Get time
    UTCTime tNow;
    Time::GetTime (&tNow);

    // Make sure game still exists
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bExist);
    if (iErrCode != OK || !bExist) {
        iErrCode = ERROR_GAME_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Get game state
    int iGameState;
    iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // If game hasn't started, never mind
    if (!(iGameState & STARTED)) {
        goto Cleanup;
    }

    // Check for update time if game is not paused
    if (!(iGameState & PAUSED)) {

        Vector<UTCTime> vecUpdateTimes;

        // Check for updates
        iErrCode = GetGameUpdateData (
            iGameClass, 
            iGameNumber, 
            &iSecondsSince, 
            &iSecondsUntil, 
            &iNumUpdates, 
            &iState,
            &vecUpdateTimes
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Get num updates already executed
        iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vNumPrevUpdates);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Are we in time for an update?
        if (iNumUpdates - vNumPrevUpdates.GetInteger() != 0) {

            Assert (iNumUpdates > vNumPrevUpdates.GetInteger());
            unsigned int i, iNumNewUpdates = iNumUpdates - vNumPrevUpdates.GetInteger();
            Assert (iNumNewUpdates == vecUpdateTimes.GetNumElements());

            const UTCTime* ptUpdateTime = vecUpdateTimes.GetData();

            // Yes, we're going to update
            *pbUpdate = true;

            for (i = 0; i < iNumNewUpdates && !bGameOver; i ++) {

                // Execute an update
                iErrCode = RunUpdate (iGameClass, iGameNumber, ptUpdateTime[i], &bGameOver);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (!bGameOver) {

                    // Break out of the loop if the game was paused
                    iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    if (iGameState & PAUSED)
                        break;
                }
            }

            // Lock game for reading again
            if (!bGameOver)
            {
                // Refresh game state
                iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }
    }

    // Closed games can update even if they're paused, as long as all empires have ended turn
    if (!bGameOver && !(iGameState & STILL_OPEN)) {

        // Check for all empires hitting end turn when game has already closed
        Variant vNumUpdated, vIdle = 0, vTemp;

        GAME_EMPIRES(pszGameEmpires, iGameClass, iGameNumber);

        // Loop until no more updates
        while (!bGameOver) {
            
            iErrCode = t_pCache->ReadData(strGameData, GameData::NumEmpiresUpdated, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            unsigned int iNumUpdated = vTemp.GetInteger();
            
            unsigned int iNumNeeded;
            iErrCode = t_pCache->GetNumCachedRows(pszGameEmpires, &iNumNeeded);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // If not all empires are ready for an update, exit the loop
            if (iNumUpdated < iNumNeeded) {
                break;
            }
            Assert (iNumUpdated == iNumNeeded);

            // Only update if not all empires are idle
            bool bIdle;
            iErrCode = AreAllEmpiresIdle (iGameClass, iGameNumber, &bIdle);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (bIdle) {
                // Everyone is idle, so don't update
                goto Cleanup;
            }
            
            // Execute an update
            iErrCode = RunUpdate (iGameClass, iGameNumber, tNow, &bGameOver);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            *pbUpdate = true;
        }
    }
    
Cleanup:

    if (fUpdateCheckTime && !bGameOver && iErrCode == OK) {
        
        // Update last checked
        iErrCode = t_pCache->WriteData (strGameData, GameData::LastUpdateCheck, tNow);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    return iErrCode;
}

void GameEngine::GetLastUpdateTimeForPausedGame (const UTCTime& tNow, 
                                                 Seconds sSecondsUntilNextUpdate,
                                                 Seconds sUpdatePeriod,
                                                 int iNumUpdates, 
                                                 Seconds sFirstUpdateDelay,
                                                 UTCTime* ptLastUpdateTime) {

    Seconds sRealUpdatePeriod = sUpdatePeriod;
    if (iNumUpdates == 0 && sFirstUpdateDelay > 0) {
        sRealUpdatePeriod += sFirstUpdateDelay;
    }

    // Note - sSecondsUntilNextUpdate may be negative
    if (sSecondsUntilNextUpdate > sRealUpdatePeriod) {
        sSecondsUntilNextUpdate = sRealUpdatePeriod;
    }

    Time::SubtractSeconds (tNow, sRealUpdatePeriod - sSecondsUntilNextUpdate, ptLastUpdateTime);
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *piSecondsSince -> Number of seconds since last update
// *piSecondsUntil -> Number of seconds remaining for update
// *piNumUpdates -> Number of updates transpired
// *piGameState -> Game state (started, paused, not started)
//
// Returns the number of seconds remaining before game will update

int GameEngine::GetGameUpdateData (int iGameClass, int iGameNumber, int* piSecondsSince, int* piSecondsUntil, 
                                   int* piNumUpdates, int* piGameState, Vector<UTCTime>* pvecUpdateTimes) {

    int iErrCode = OK, iGameState;

    Variant vTemp;

    Seconds sUpdatePeriod;
    Seconds sFirstUpdateDelay = 0, sAfterWeekendDelay = 0;

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Get update period
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    sUpdatePeriod = vTemp.GetInteger();

    // Get game state
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iGameState = vTemp.GetInteger();

    // Easy short-circuit: game hasn't started
    if (!(iGameState & STARTED)) {

        // Get first update delay
        iErrCode = t_pCache->ReadData(strGameData, GameData::FirstUpdateDelay, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        sFirstUpdateDelay = vTemp.GetInteger();

        *piGameState = iGameState;
        *piNumUpdates = 0;
        *piSecondsSince = 0;
        *piSecondsUntil = sUpdatePeriod + sFirstUpdateDelay;

        return OK;
    }
    
    // Get num updates
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    int iNumUpdates = vTemp.GetInteger();
    
    // Get first update delay
    if (iNumUpdates == 0) {
        
        iErrCode = t_pCache->ReadData(strGameData, GameData::FirstUpdateDelay, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        sFirstUpdateDelay = vTemp.GetInteger();
    }

    // Get last update time
    UTCTime tLastUpdateTime;
    iErrCode = t_pCache->ReadData(strGameData, GameData::LastUpdateTime, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    tLastUpdateTime = vTemp.GetInteger64();

    UTCTime tRealLastUpdateTime;
    iErrCode = t_pCache->ReadData(strGameData, GameData::RealLastUpdateTime, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    tRealLastUpdateTime = vTemp.GetInteger64();

    // Get options
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    bool bWeekends = (vTemp.GetInteger() & WEEKEND_UPDATES) != 0;

    if (!bWeekends) {

        // Get after weekend delay
        iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::AfterWeekendDelay, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        sAfterWeekendDelay = vTemp.GetInteger();
    }

    // Get time
    UTCTime tNow, tNextUpdateTime;
    Time::GetTime (&tNow);

    if ((iGameState & ADMIN_PAUSED) || (iGameState & PAUSED)) {

        iErrCode = t_pCache->ReadData(strGameData, GameData::SecondsUntilNextUpdateWhilePaused, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        Seconds sSecondsUntilNext = vTemp.GetInteger();
        // Note - sSecondsUntil can be negative

        // Calculate the simulated last update time
        GetLastUpdateTimeForPausedGame (
            tNow,
            sSecondsUntilNext, 
            sUpdatePeriod,
            iNumUpdates,
            sFirstUpdateDelay,
            &tLastUpdateTime
            );

        // Calculate hypothetical next update time
        GetNextUpdateTime (
            tLastUpdateTime,
            sUpdatePeriod,
            iNumUpdates,
            sFirstUpdateDelay,
            sAfterWeekendDelay,
            bWeekends,
            &tNextUpdateTime
            );

        *piGameState = iGameState;
        *piSecondsSince = Time::GetSecondDifference (tNow, tRealLastUpdateTime);
        *piSecondsUntil = Time::GetSecondDifference (tNextUpdateTime, tNow);
        *piNumUpdates = iNumUpdates;

        return OK;
    }

    // Loop until done
    while (true) {

        GetNextUpdateTime (
            tLastUpdateTime,
            sUpdatePeriod,
            iNumUpdates,
            sFirstUpdateDelay,
            sAfterWeekendDelay,
            bWeekends,
            &tNextUpdateTime
            );
        
        // Exit the loop if we're not going to update
        if (Time::OlderThan (tNow, tNextUpdateTime)) {
            break;
        }
        
        //
        // An update
        //
        
        iNumUpdates ++;
        tLastUpdateTime = tNextUpdateTime;
        
        if (pvecUpdateTimes != NULL) {
            
            iErrCode = pvecUpdateTimes->Add (tNextUpdateTime);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
        }
    }
    
    // At this point, tNextUpdateTime will be the next unfulfilled update
    *piGameState = iGameState;
    *piSecondsSince = Time::GetSecondDifference (tNow, tRealLastUpdateTime);
    *piSecondsUntil = Time::GetSecondDifference (tNextUpdateTime, tNow);
    *piNumUpdates = iNumUpdates;

    return OK;
}

// Resets all games' update times to now
int GameEngine::ResetAllGamesUpdateTime() {

    unsigned int i, iNumKeys;
    int iGameClass, iGameNumber;
    Variant* pvGame;

    int iErrCode = t_pCache->ReadColumn (
        SYSTEM_ACTIVE_GAMES,
        SystemActiveGames::GameClassGameNumber,
        NULL,
        &pvGame,
        &iNumKeys
        );
    
    if (iErrCode == OK) {
        
        for (i = 0; i < iNumKeys; i ++) {
            
            GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

            // Best effort reset game update time
            ResetGameUpdateTime (iGameClass, iGameNumber);
        }
        
        t_pCache->FreeData(pvGame);
    
    } else {
        
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert (false);
    }

    return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Resets a game's update time to now

int GameEngine::ResetGameUpdateTime (int iGameClass, int iGameNumber) {

    int iErrCode;
    ICachedTable* pGameData = NULL;

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Flush remaining updates
    bool bExists;
    iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, true, &bExists);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Prevent the game from updating
    if (DoesGameExist (iGameClass, iGameNumber, &bExists) != OK || !bExists) {
        iErrCode = ERROR_GAME_DOES_NOT_EXIST;
        goto Cleanup;
    }

    iErrCode = t_pCache->GetTable(strGameData, &pGameData);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    int iState;
    iErrCode = pGameData->ReadData(GameData::State, &iState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iState & PAUSED) {
        iErrCode = ERROR_GAME_PAUSED;
        goto Cleanup;
    }

    UTCTime tTime;
    Time::GetTime (&tTime);

    iErrCode = pGameData->WriteData (GameData::LastUpdateTime, tTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameData->WriteData (GameData::LastUpdateCheck, tTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease(pGameData);

    return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Sets an empire to be ready for the next update.

int GameEngine::SetEmpireReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet) {

    int iErrCode;
    Variant vOptions;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    *pbSet = (vOptions.GetInteger() & UPDATED) == 0;
    if (*pbSet) {

        GAME_DATA (strGameData, iGameClass, iGameNumber);
        
        iErrCode = t_pCache->WriteOr(strGameEmpireData, GameEmpireData::Options, UPDATED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Increment empire updated count
        iErrCode = t_pCache->Increment(strGameData, GameData::NumEmpiresUpdated, 1);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

#ifdef _DEBUG
        iErrCode = VerifyUpdatedEmpireCount (iGameClass, iGameNumber);
        Assert (iErrCode == OK);
        iErrCode = OK;
#endif

    }

Cleanup:

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Sets an empire to not be ready for the next update.

int GameEngine::SetEmpireNotReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet) {

    int iErrCode;
    
    Variant vOptions;

    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    *pbSet = (vOptions.GetInteger() & UPDATED) != 0;
    if (*pbSet) {
        
        iErrCode = t_pCache->WriteAnd(strEmpireData, GameEmpireData::Options, ~UPDATED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = t_pCache->Increment(strGameData, GameData::NumEmpiresUpdated, -1);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

#ifdef _DEBUG
        iErrCode = VerifyUpdatedEmpireCount (iGameClass, iGameNumber);
        Assert (iErrCode == OK);
        iErrCode = OK;
#endif

    }
    
Cleanup:

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Force a game to update

int GameEngine::ForceUpdate (int iGameClass, int iGameNumber) {

    Variant vStarted;
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszGameData, GameData::State, &vStarted);

    if (iErrCode != OK || !(vStarted.GetInteger() & STARTED)) {
        return ERROR_FAILURE;
    }

    UTCTime tUpdateTime;
    Time::GetTime (&tUpdateTime);

    bool bGameOver;
    iErrCode = RunUpdate (iGameClass, iGameNumber, tUpdateTime, &bGameOver);
    Assert (iErrCode == OK);

    return iErrCode;
}


// Check all currently existing games for an update
// This is best effort

int GameEngine::CheckAllGamesForUpdates (bool fUpdateCheckTime) {

    unsigned int i, iNumGames;
    int iGameClass, iGameNumber;
    bool bUpdate;
    
    Variant* pvGame;

    int iErrCode = t_pCache->ReadColumn (
        SYSTEM_ACTIVE_GAMES, 
        SystemActiveGames::GameClassGameNumber,
        NULL,
        &pvGame, 
        &iNumGames
        );
    
    if (iErrCode == OK) {
        
        // Best effort
        for (i = 0; i < iNumGames; i ++) {
            
            GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);
            CheckGameForUpdates (iGameClass, iGameNumber, fUpdateCheckTime, &bUpdate);
        }
        
        // Clean up
        t_pCache->FreeData(pvGame);
    }

    return OK;
}

void GameEngine::GetNextUpdateTime (const UTCTime& tLastUpdateTime, Seconds sUpdatePeriod, int iNumUpdates,
                                    Seconds sFirstUpdateDelay, Seconds sAfterWeekendDelay, bool bWeekends,
                                    UTCTime* ptNextUpdateTime) {

    UTCTime tNextUpdateTime;

    // Calculate next update time
    Time::AddSeconds (tLastUpdateTime, sUpdatePeriod, &tNextUpdateTime);
    
    // Add first update delay
    if (iNumUpdates == 0) {
        Time::AddSeconds (tNextUpdateTime, sFirstUpdateDelay, &tNextUpdateTime);
    }
    
    // If no weekend updates and current time is in weekend, push it back to Monday
    if (!bWeekends) {
        AdvanceWeekendTime (tNextUpdateTime, sAfterWeekendDelay, &tNextUpdateTime);
    }

    *ptNextUpdateTime = tNextUpdateTime;
}

void GameEngine::AdvanceWeekendTime (const UTCTime& tNextUpdateTime, Seconds sAfterWeekendDelay, 
                                     UTCTime* ptNextUpdateTime) {

    Seconds sAdd = 0;
    
    if (Time::IsWeekendTime (tNextUpdateTime)) {
        
        // Add the rest of the weekend and the after weekend delay 
        sAdd = Time::GetRemainingWeekendSeconds (tNextUpdateTime) + sAfterWeekendDelay;
        Assert (sAdd > 0);

    } else if (sAfterWeekendDelay > 0) {

        UTCTime tTemp;
        Time::SubtractSeconds (tNextUpdateTime, sAfterWeekendDelay, &tTemp);
        
        // If the new update period falls between the weekend and the weekend delay,
        // push it to the end of the weekend delay
        if (Time::IsWeekendTime (tTemp)) {
            sAdd = Time::GetRemainingWeekendSeconds (tTemp);
            Assert (sAdd > 0);
        }
    }
    
    if (sAdd != 0) {
        
        Assert (sAdd > 0);
        Time::AddSeconds (tNextUpdateTime, sAdd, ptNextUpdateTime);
    
    } else {

        *ptNextUpdateTime = tNextUpdateTime;
    }
}


#ifdef _DEBUG

int GameEngine::VerifyUpdatedEmpireCount (int iGameClass, int iGameNumber) {

    int iErrCode = OK, iOptions = 0, iNumCountedEmpires = 0, iRealNumUpdated = 0;

    unsigned int iKey;
    Variant vEmpireKey;

    GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    iKey = NO_KEY;
    while (true) {

        iErrCode = t_pCache->GetNextKey (pszGameEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->ReadData(pszGameEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = GetEmpireOptions (iGameClass, iGameNumber, vEmpireKey.GetInteger(), &iOptions);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iOptions & UPDATED) {
            iRealNumUpdated ++;
        }
    }

    iErrCode = GetNumUpdatedEmpires (iGameClass, iGameNumber, &iNumCountedEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    Assert (iRealNumUpdated == iNumCountedEmpires && "Bad updated empire count!");

Cleanup:

    return iErrCode;
}

#endif