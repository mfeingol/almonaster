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

int GameEngine::CheckGameForUpdates(int iGameClass, int iGameNumber, bool* pbUpdate)
{
    int iErrCode;

    *pbUpdate = false;

    bool bGameOver = false, bExist;
    int iSecondsSince, iSecondsUntil, iNumUpdates, iState;

    Variant vNumPrevUpdates;

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Get time
    UTCTime tNow;
    Time::GetTime (&tNow);

    // Make sure game still exists
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bExist);
    RETURN_ON_ERROR(iErrCode);
    if (!bExist)
    {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    // Get game state
    int iGameState;
    iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
    RETURN_ON_ERROR(iErrCode);
    
    // If game hasn't started, never mind
    if (!(iGameState & STARTED)) {
        return OK;
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

        RETURN_ON_ERROR(iErrCode);

        // Get num updates already executed
        iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vNumPrevUpdates);
        RETURN_ON_ERROR(iErrCode);

        // Are we in time for an update?
        if (iNumUpdates - vNumPrevUpdates.GetInteger() != 0) {

            Assert(iNumUpdates > vNumPrevUpdates.GetInteger());
            unsigned int i, iNumNewUpdates = iNumUpdates - vNumPrevUpdates.GetInteger();
            Assert(iNumNewUpdates == vecUpdateTimes.GetNumElements());

            const UTCTime* ptUpdateTime = vecUpdateTimes.GetData();

            // Yes, we're going to update
            *pbUpdate = true;

            for (i = 0; i < iNumNewUpdates && !bGameOver; i ++)
            {
                // Execute an update
                iErrCode = RunUpdate (iGameClass, iGameNumber, ptUpdateTime[i], &bGameOver);
                RETURN_ON_ERROR(iErrCode);

                if (!bGameOver) {

                    // Break out of the loop if the game was paused
                    iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
                    RETURN_ON_ERROR(iErrCode);

                    if (iGameState & PAUSED)
                        break;
                }
            }

            // Lock game for reading again
            if (!bGameOver)
            {
                // Refresh game state
                iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    // Closed games can update even if they're paused, as long as all empires have ended turn
    if (!bGameOver && !(iGameState & STILL_OPEN)) {

        // Check for all empires hitting end turn when game has already closed
        Variant vNumUpdated, vIdle = 0, vTemp;

        GET_GAME_EMPIRES(pszGameEmpires, iGameClass, iGameNumber);

        // Loop until no more updates
        while (!bGameOver)
        {
            iErrCode = t_pCache->ReadData(strGameData, GameData::NumEmpiresUpdated, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            unsigned int iNumUpdated = vTemp.GetInteger();
            
            unsigned int iNumNeeded;
            iErrCode = t_pCache->GetNumCachedRows(pszGameEmpires, &iNumNeeded);
            RETURN_ON_ERROR(iErrCode);

            // If not all empires are ready for an update, exit the loop
            if (iNumUpdated < iNumNeeded)
            {
                break;
            }
            Assert(iNumUpdated == iNumNeeded);

            // Only update if not all empires are idle
            bool bIdle;
            iErrCode = AreAllEmpiresIdle (iGameClass, iGameNumber, &bIdle);
            RETURN_ON_ERROR(iErrCode);

            if (bIdle)
            {
                // Everyone is idle, so don't update
                break;
            }
            
            // Execute an update
            iErrCode = RunUpdate (iGameClass, iGameNumber, tNow, &bGameOver);
            RETURN_ON_ERROR(iErrCode);
            
            *pbUpdate = true;
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

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Get update period
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    sUpdatePeriod = vTemp.GetInteger();

    // Get game state
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameState = vTemp.GetInteger();

    // Easy short-circuit: game hasn't started
    if (!(iGameState & STARTED)) {

        // Get first update delay
        iErrCode = t_pCache->ReadData(strGameData, GameData::FirstUpdateDelay, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        sFirstUpdateDelay = vTemp.GetInteger();

        *piGameState = iGameState;
        *piNumUpdates = 0;
        *piSecondsSince = 0;
        *piSecondsUntil = sUpdatePeriod + sFirstUpdateDelay;

        return OK;
    }
    
    // Get num updates
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iNumUpdates = vTemp.GetInteger();
    
    // Get first update delay
    if (iNumUpdates == 0) {
        
        iErrCode = t_pCache->ReadData(strGameData, GameData::FirstUpdateDelay, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        sFirstUpdateDelay = vTemp.GetInteger();
    }

    // Get last update time
    UTCTime tLastUpdateTime;
    iErrCode = t_pCache->ReadData(strGameData, GameData::LastUpdateTime, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    tLastUpdateTime = vTemp.GetInteger64();

    UTCTime tRealLastUpdateTime;
    iErrCode = t_pCache->ReadData(strGameData, GameData::RealLastUpdateTime, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    tRealLastUpdateTime = vTemp.GetInteger64();

    // Get options
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    bool bWeekends = (vTemp.GetInteger() & WEEKEND_UPDATES) != 0;

    if (!bWeekends) {

        // Get after weekend delay
        iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::AfterWeekendDelay, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        sAfterWeekendDelay = vTemp.GetInteger();
    }

    // Get time
    UTCTime tNow, tNextUpdateTime;
    Time::GetTime (&tNow);

    if ((iGameState & ADMIN_PAUSED) || (iGameState & PAUSED)) {

        iErrCode = t_pCache->ReadData(strGameData, GameData::SecondsUntilNextUpdateWhilePaused, &vTemp);
        RETURN_ON_ERROR(iErrCode);
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
            RETURN_ON_ERROR(iErrCode);
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
int GameEngine::ResetAllGamesUpdateTime()
{
    int iErrCode;

    Variant** ppvGame;
    AutoFreeData free_ppvGame(ppvGame);
    unsigned int iNumGames;

    iErrCode = GetActiveGames(&ppvGame, &iNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        iErrCode = CacheGameData((const Variant**)ppvGame, NO_KEY, iNumGames);
        RETURN_ON_ERROR(iErrCode);

        for (unsigned int i = 0; i < iNumGames; i ++)
        {
            int iGameClass = ppvGame[i][0].GetInteger();
            int iGameNumber = ppvGame[i][1].GetInteger();

            iErrCode = ResetGameUpdateTime(iGameClass, iGameNumber);
            if (iErrCode == ERROR_GAME_PAUSED)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);
        }
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
    AutoRelease<ICachedTable> rel(pGameData);

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Flush remaining updates
    bool bExists;
    iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, &bExists);
    RETURN_ON_ERROR(iErrCode);

    // Prevent the game from updating
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bExists);
    RETURN_ON_ERROR(iErrCode);
    if (!bExists)
    {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    iErrCode = t_pCache->GetTable(strGameData, &pGameData);
    RETURN_ON_ERROR(iErrCode);

    int iState;
    iErrCode = pGameData->ReadData(GameData::State, &iState);
    RETURN_ON_ERROR(iErrCode);

    if (iState & PAUSED)
    {
        return ERROR_GAME_PAUSED;
    }

    UTCTime tTime;
    Time::GetTime (&tTime);

    iErrCode = pGameData->WriteData(GameData::LastUpdateTime, tTime);
    RETURN_ON_ERROR(iErrCode);

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

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);
    
    *pbSet = (vOptions.GetInteger() & UPDATED) == 0;
    if (*pbSet)
    {
        GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
        
        iErrCode = t_pCache->WriteOr(strGameEmpireData, GameEmpireData::Options, UPDATED);
        RETURN_ON_ERROR(iErrCode);

        // Increment empire updated count
        iErrCode = t_pCache->Increment(strGameData, GameData::NumEmpiresUpdated, 1);
        RETURN_ON_ERROR(iErrCode);
    }

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

    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);
    
    *pbSet = (vOptions.GetInteger() & UPDATED) != 0;
    if (*pbSet)
    {
        iErrCode = t_pCache->WriteAnd(strEmpireData, GameEmpireData::Options, ~UPDATED);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->Increment(strGameData, GameData::NumEmpiresUpdated, -1);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Force a game to update

int GameEngine::ForceUpdate (int iGameClass, int iGameNumber)
{
    Variant vStarted;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszGameData, GameData::State, &vStarted);
    RETURN_ON_ERROR(iErrCode);

    if (!(vStarted.GetInteger() & STARTED))
    {
        return ERROR_GAME_HAS_NOT_STARTED;
    }

    UTCTime tUpdateTime;
    Time::GetTime (&tUpdateTime);

    bool bGameOver;
    iErrCode = RunUpdate (iGameClass, iGameNumber, tUpdateTime, &bGameOver);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

//
// Check all currently existing games for an update
//
int GameEngine::CheckAllGamesForUpdates()
{
    int iErrCode;

    Variant** ppvGame;
    AutoFreeData free_ppvGame(ppvGame);
    unsigned int iNumGames;

    iErrCode = GetActiveGames(&ppvGame, &iNumGames);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CacheGameData((const Variant**)ppvGame, NO_KEY, iNumGames);
    RETURN_ON_ERROR(iErrCode);

    for (unsigned int i = 0; i < iNumGames; i ++)
    {
        int iGameClass = ppvGame[i][0].GetInteger();
        int iGameNumber = ppvGame[i][1].GetInteger();

        bool bUpdate;
        iErrCode = CheckGameForUpdates(iGameClass, iGameNumber, &bUpdate);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
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
        Assert(sAdd > 0);

    } else if (sAfterWeekendDelay > 0) {

        UTCTime tTemp;
        Time::SubtractSeconds (tNextUpdateTime, sAfterWeekendDelay, &tTemp);
        
        // If the new update period falls between the weekend and the weekend delay,
        // push it to the end of the weekend delay
        if (Time::IsWeekendTime (tTemp)) {
            sAdd = Time::GetRemainingWeekendSeconds (tTemp);
            Assert(sAdd > 0);
        }
    }
    
    if (sAdd != 0) {
        
        Assert(sAdd > 0);
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

    GET_GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    iKey = NO_KEY;
    while (true) {

        iErrCode = t_pCache->GetNextKey (pszGameEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(pszGameEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireOptions (iGameClass, iGameNumber, vEmpireKey.GetInteger(), &iOptions);
        RETURN_ON_ERROR(iErrCode);

        if (iOptions & UPDATED) {
            iRealNumUpdated ++;
        }
    }

    iErrCode = GetNumUpdatedEmpires (iGameClass, iGameNumber, &iNumCountedEmpires);
    RETURN_ON_ERROR(iErrCode);

    Assert(iRealNumUpdated == iNumCountedEmpires && "Bad updated empire count!");

    return iErrCode;
}

#endif