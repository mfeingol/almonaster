//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998-2004-2001 Max Attar Feingold (maf6@cornell.edu)
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

    bool bGameOver = false, bExist, bGameWriter = false, bGameReader = false;
    int iSecondsSince, iSecondsUntil, iNumUpdates, iState;

    Variant vState, vNumPrevUpdates;

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Lock game for reading
    iErrCode = WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
    if (iErrCode != OK) {
        return iErrCode;
    }
    bGameReader = true;

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
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // If game hasn't started, never mind
    if (!(vState.GetInteger() & STARTED)) {
        goto Cleanup;
    }

    // Check for update time if game is paused
    if (!(vState.GetInteger() & PAUSED) && !(vState.GetInteger() & ADMIN_PAUSED)) {

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
        iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumPrevUpdates);
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

            // Release the game lock
            iErrCode = SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            bGameReader = false;

            // Yes, we're going to update
            *pbUpdate = true;
            
            iErrCode = WaitForUpdate (iGameClass, iGameNumber);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            bGameWriter = true;
                    
            for (i = 0; i < iNumNewUpdates && !bGameOver; i ++) {
            
                // Execute an update
                iErrCode = RunUpdate (iGameClass, iGameNumber, ptUpdateTime[i], &bGameOver);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }

            SignalAfterUpdate (iGameClass, iGameNumber);
            bGameWriter = false;

            // Lock game for reading again
            if (!bGameOver) {
                iErrCode = WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
                if (iErrCode != OK) {
                    return iErrCode;
                }
                bGameReader = true;
            }
        }

        // Refresh game state if necessary
        if (!bGameOver && (*pbUpdate)) {

            iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    // Closed games can update even if they're paused, as long as all empires have ended turn
    if (!bGameOver && !(vState.GetInteger() & STILL_OPEN)) {

        // Check for all empires hitting end turn when game has already closed
        Variant vNumUpdated, vIdle = 0, vTemp;
        unsigned int iNumNeeded;

        GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

        // Loop until no more updates
        while (!bGameOver) {
            
            iErrCode = m_pGameData->ReadData (strGameData, GameData::NumEmpiresUpdated, &vNumUpdated);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->GetNumRows (pszGameEmpires, &iNumNeeded);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // If not all empires are ready for an update, exit the loop
            if ((unsigned int) vNumUpdated.GetInteger() < iNumNeeded) {
                break;
            }
            Assert ((unsigned int) vNumUpdated.GetInteger() == iNumNeeded);

            // Only update if not all empires are idle
            bool bIdle;
            iErrCode = AreAllEmpiresIdle (iGameClass, iGameNumber, &bIdle);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (bIdle) {
                // Everyone is idle, so don't update
                iErrCode = OK;
                goto Cleanup;
            }
            
            if (!bGameWriter) {

                // Release the game lock
                iErrCode = SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                bGameReader = false;

                if (WaitForUpdate (iGameClass, iGameNumber) != OK) {
                    bGameOver = true;
                    break;
                }
                bGameWriter = true;
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
        iErrCode = m_pGameData->WriteData (strGameData, GameData::LastUpdateCheck, tNow);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Release the lock
    if (bGameWriter) {
        SignalAfterUpdate (iGameClass, iGameNumber);
    }

    if (bGameReader) {
        SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
    }

    return iErrCode;
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

    Variant vSecPerUpdate, vLastUpdateTime, vTemp, vNumUpdates, vOptions, vAfterWeekendDelay = 0, vState,
        vFirstUpdateDelay = 0;

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Get update period
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::NumSecPerUpdate, 
        &vSecPerUpdate
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Get game state
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iGameState = vState.GetInteger();

    // Easy short-circuit: game hasn't started
    if (!(iGameState & STARTED)) {

        // Get first update delay
        iErrCode = m_pGameData->ReadData (strGameData, GameData::FirstUpdateDelay, &vFirstUpdateDelay);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        *piGameState = iGameState;
        *piNumUpdates = 0;
        *piSecondsSince = 0;
        *piSecondsUntil = vSecPerUpdate.GetInteger() + vFirstUpdateDelay.GetInteger();

        return OK;
    }
    
    // Get num updates
    iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Get first update delay
    if (vNumUpdates.GetInteger() == 0) {
        
        iErrCode = m_pGameData->ReadData (strGameData, GameData::FirstUpdateDelay, &vFirstUpdateDelay);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    // Get options
    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!(vOptions.GetInteger() & WEEKEND_UPDATES)) {

        // Get after weekend delay
        iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AfterWeekendDelay, &vAfterWeekendDelay);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    // Get time
    UTCTime tNow, tLastUpdateTime, tNextUpdateTime;
    Time::GetTime (&tNow);

    bool bWeekends = (vOptions.GetInteger() & WEEKEND_UPDATES) != 0;
    int iNumUpdates = vNumUpdates.GetInteger();
    
    Seconds sUpdatePeriod = vSecPerUpdate.GetInteger();
    Seconds sAfterWeekendDelay = vAfterWeekendDelay.GetInteger();
    Seconds sFirstUpdateDelay = vFirstUpdateDelay.GetInteger();

    if ((iGameState & ADMIN_PAUSED) || (iGameState & PAUSED)) {

        iErrCode = m_pGameData->ReadData (strGameData, GameData::SecondsSinceLastUpdateWhilePaused, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        Assert (vTemp.GetInteger() >= 0);

        // If the time is greater than an update period, it means that 
        // the game was paused on a weekend. In this case, we assume
        // that the game has a full update remaining
        Seconds sSecondsSinceLast = vTemp.GetInteger();
        if (sSecondsSinceLast > sUpdatePeriod + (iNumUpdates == 0 ? sFirstUpdateDelay : 0)) {
            Assert (!bWeekends);
            sSecondsSinceLast = 1;  // 1 second
        }

        // Calculate the simulated last update time
        Time::SubtractSeconds (tNow, sSecondsSinceLast, &tLastUpdateTime);

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

        Assert (Time::OlderThan (tNow, tNextUpdateTime));

        *piGameState = iGameState;
        *piSecondsSince = vTemp.GetInteger();
        *piSecondsUntil = Time::GetSecondDifference (tNextUpdateTime, tNow);
        *piNumUpdates = iNumUpdates;

        return OK;
    }

    // Get last update time
    iErrCode = m_pGameData->ReadData (strGameData, GameData::LastUpdateTime, &vLastUpdateTime);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Set last update time, loop until done
    tLastUpdateTime = vLastUpdateTime.GetUTCTime();

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
    *piSecondsSince = Time::GetSecondDifference (tNow, tLastUpdateTime);
    *piSecondsUntil = Time::GetSecondDifference (tNextUpdateTime, tNow);
    *piNumUpdates = iNumUpdates;

    return OK;
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

    iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    *pbSet = (vOptions.GetInteger() & UPDATED) == 0;
    if (*pbSet) {

        GAME_DATA (strGameData, iGameClass, iGameNumber);
        
        iErrCode = m_pGameData->WriteOr (strGameEmpireData, GameEmpireData::Options, UPDATED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Increment empire updated count
        iErrCode = m_pGameData->Increment (strGameData, GameData::NumEmpiresUpdated, 1);
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

    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    *pbSet = (vOptions.GetInteger() & UPDATED) != 0;
    if (*pbSet) {
        
        iErrCode = m_pGameData->WriteAnd (strEmpireData, GameEmpireData::Options, ~UPDATED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->Increment (strGameData, GameData::NumEmpiresUpdated, -1);
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

    int iErrCode = m_pGameData->ReadData (pszGameData, GameData::State, &vStarted);

    if (iErrCode != OK || !(vStarted.GetInteger() & STARTED)) {
        return ERROR_FAILURE;
    }

    // Lock
    iErrCode = WaitForUpdate (iGameClass, iGameNumber);
    if (iErrCode == OK) {

        UTCTime tUpdateTime;
        Time::GetTime (&tUpdateTime);

        bool bGameOver;
        iErrCode = RunUpdate (iGameClass, iGameNumber, tUpdateTime, &bGameOver);
        Assert (iErrCode == OK);

        SignalAfterUpdate (iGameClass, iGameNumber);
    }

    return iErrCode;
}


// Check all currently existing games for an update
// This is best effort

int GameEngine::CheckAllGamesForUpdates (bool fUpdateCheckTime) {

    unsigned int i, iNumGames;
    int iGameClass, iGameNumber;
    bool bUpdate;
    
    Variant* pvGame;

    int iErrCode = m_pGameData->ReadColumn (
        SYSTEM_ACTIVE_GAMES, 
        SystemActiveGames::GameClassGameNumber,
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
        m_pGameData->FreeData (pvGame);
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

        iErrCode = m_pGameData->GetNextKey (pszGameEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameData->ReadData (pszGameEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
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