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

#include <math.h>

#include "GameEngine.h"
#include "Global.h"

#include "BridierScore.h"

// Input:
// iGameClass -> GameClass key
// iGameNumber -> GameNumber
// iEmpireKey -> Key of empire who deleted the game.  SYSTEM if the system did it
// pszMessage -> Message from admin who deleted the game
// iReason -> Reason for deletion, only used if SYSTEM deleted the game
//
// Delete a game without warning or appeal

int GameEngine::DeleteGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, int iReason)
{
    unsigned int i, iNumEmpires;

    Variant* pvEmpireKey = NULL;
    AutoFreeData free(pvEmpireKey);

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    int iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
    {
        return iErrCode;
    }
    RETURN_ON_ERROR(iErrCode);

    GET_GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadColumn (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        NULL,
        &pvEmpireKey, 
        &iNumEmpires
        );

    RETURN_ON_ERROR(iErrCode);

    char pszTemp [512 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    if (iEmpireKey == SYSTEM)
    {
        switch (iReason)
        {
        case REASON_SYSTEM_SHUTDOWN:
            sprintf (
                pszTemp,
                "%s %i was deleted after a system restart because the server was down too long",
                pszGameClassName,
                iGameNumber
                );
            break;
        
        case REASON_PASSWORD_PROTECTED:
            sprintf (
                pszTemp,
                "%s %i was deleted because it was password protected and only one empire was playing",
                pszGameClassName,
                iGameNumber
                );
            break;

        default:
            Assert(false);
            break;
        }
    }
    else
    {

        Variant vEmpireName;
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        RETURN_ON_ERROR(iErrCode);

        sprintf (
            pszTemp,
            "%s %i was deleted by %s",
            pszGameClassName,
            iGameNumber,
            vEmpireName.GetCharPtr()
            );
    }

    // Cleanup the game
    iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
    RETURN_ON_ERROR(iErrCode);

    // Best effort send messages
    for (i = 0; i < iNumEmpires; i ++)
    {
        iErrCode = SendSystemMessage (pvEmpireKey[i], pszTemp, SYSTEM, MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
    }

    // Best effort send the message from the admin  
    if (!String::IsBlank (pszMessage))
    {
        for (i = 0; i < iNumEmpires; i ++)
        {
            iErrCode = SendSystemMessage (pvEmpireKey[i], pszMessage, iEmpireKey, MESSAGE_ADMINISTRATOR);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}


// Delete rows from all game tables
//
// GameEmpires(I.I)
// GameData(I.I)
// GameMap(I.I)
//
// Delete player tables
// Delete game from open/closed lists
//
// Delete game's data and tables

int GameEngine::CleanupGame(int iGameClass, int iGameNumber, GameResult grResult, const char* pszWinnerName)
{
    int iErrCode;

    Variant vEmpireKey, vName, vGameState, vNumGames, vGameOptions, vGameClassOptions, * pvEmpireKey = NULL, * pvLoserEmpireName = NULL;
    AutoFreeData free1(pvEmpireKey);
    AutoFreeData free2(pvLoserEmpireName);

    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GET_GAME_NUKED_EMPIRES (strGameDeadEmpires, iGameClass, iGameNumber);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Notification
    global.GetEventSink()->OnCleanupGame(iGameClass, iGameNumber);

    // Get game state
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vGameState);
    RETURN_ON_ERROR(iErrCode);

    // Get gameclass name
    char pszGameClass[MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];
    iErrCode = GetGameClassName(iGameClass, pszGameClass);
    RETURN_ON_ERROR(iErrCode);

    // Get gameclass tournament
    unsigned int iTournamentKey;
    iErrCode = GetGameClassTournament(iGameClass, &iTournamentKey);
    RETURN_ON_ERROR(iErrCode);

    // Update scores if necessary
    iErrCode = UpdateScoresOnGameEnd(iGameClass, iGameNumber);
    RETURN_ON_ERROR(iErrCode);

    // Delete all remaining empires from the game
    unsigned int iNumEmpires;
    iErrCode = t_pCache->ReadColumn(strGameEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
        iErrCode = OK;
    RETURN_ON_ERROR(iErrCode);

    String strWinnerList;
    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        GET_SYSTEM_EMPIRE_DATA(strEmpire, pvEmpireKey[i].GetInteger());
        iErrCode = t_pCache->ReadData(strEmpire, pvEmpireKey[i].GetInteger(), SystemEmpireData::Name, &vName);
        RETURN_ON_ERROR(iErrCode);

        if (!strWinnerList.IsBlank())
        {
            strWinnerList += ", ";
        }
        strWinnerList += vName.GetCharPtr();

        iErrCode = DeleteEmpireFromGame(iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), EMPIRE_GAME_ENDED, NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    // Add the additional winner's name, if provided
    if (pszWinnerName != NULL)
    {
        if (!strWinnerList.IsBlank())
        {
            strWinnerList += ", ";
        }
        strWinnerList += pszWinnerName;
    }
    
    // Report the game ending
    char* pszMessage = (char*)StackAlloc(strWinnerList.GetLength() + 80 + MAX_FULL_GAME_CLASS_NAME_LENGTH);
    sprintf (
        pszMessage,
        "%s %i ended with the following empires still alive: %s",
        pszGameClass,
        iGameNumber,
        strWinnerList.GetCharPtr() == NULL ? "" : strWinnerList.GetCharPtr()
        );
    global.WriteReport(TRACE_INFO, pszMessage);

    // Add to latest games
    if (vGameState.GetInteger() & STARTED) {

        Variant pvLatestGame[SystemLatestGames::NumColumns];

        UTCTime tNow;
        Time::GetTime (&tNow);

        // Name
        pvLatestGame[SystemLatestGames::iName] = pszGameClass;
        pvLatestGame[SystemLatestGames::iNumber] = iGameNumber;
        pvLatestGame[SystemLatestGames::iResult] = (int) grResult;
        pvLatestGame[SystemLatestGames::iEnded] = tNow;
        pvLatestGame[SystemLatestGames::iTournamentKey] = iTournamentKey;
        pvLatestGame[SystemLatestGames::iWinners] = strWinnerList.GetCharPtr() ? strWinnerList : "";

        // Created
        iErrCode = t_pCache->ReadData(strGameData, GameData::CreationTime, pvLatestGame + SystemLatestGames::iCreated);
        RETURN_ON_ERROR(iErrCode);

        // Updates
        iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, pvLatestGame + SystemLatestGames::iUpdates);
        RETURN_ON_ERROR(iErrCode);

        // Loser list
        iErrCode = t_pCache->ReadColumn(strGameDeadEmpires, GameNukedEmpires::Name, NULL, &pvLoserEmpireName, &iNumEmpires);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
            pvLatestGame[SystemLatestGames::iLosers] = "";
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);

            String strLoserList;
            for (unsigned int i = 0; i < iNumEmpires; i ++)
            {
                if (!strLoserList.IsBlank())
                {
                    strLoserList += ", ";
                }
                strLoserList += pvLoserEmpireName[i].GetCharPtr();
            }
            pvLatestGame[SystemLatestGames::iLosers] = strLoserList;
        }

        Assert(pvLatestGame[SystemLatestGames::iWinners].GetCharPtr());
        Assert(pvLatestGame[SystemLatestGames::iLosers].GetCharPtr());

        iErrCode = AddToLatestGames(pvLatestGame);
        RETURN_ON_ERROR(iErrCode);
    }

    //////////////////////////
    // Delete all game rows //
    //////////////////////////

    // GameEmpires(I.I)
    iErrCode = t_pCache->DeleteAllRows(strGameEmpires);
    RETURN_ON_ERROR(iErrCode);

    // GameMap(I.I)
    GET_GAME_MAP(strGameMap, iGameClass, iGameNumber);
    iErrCode = t_pCache->DeleteAllRows(strGameMap);
    RETURN_ON_ERROR(iErrCode);

    // GameIndependentShips(I.I)
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vGameClassOptions);
    RETURN_ON_ERROR(iErrCode);

    if (vGameClassOptions.GetInteger() & INDEPENDENCE)
    {
        GET_GAME_EMPIRE_SHIPS(strIndependentShips, iGameClass, iGameNumber, INDEPENDENT);
        iErrCode = t_pCache->DeleteAllRows(strIndependentShips);
        RETURN_ON_ERROR(iErrCode);
    }

    // GameSecurity(I.I)
    iErrCode = t_pCache->ReadData(strGameData, GameData::Options, &vGameOptions);
    RETURN_ON_ERROR(iErrCode);

    if (vGameOptions.GetInteger() & GAME_ENFORCE_SECURITY)
    {
        GET_GAME_SECURITY(strGameSecurity, iGameClass, iGameNumber);
        iErrCode = t_pCache->DeleteAllRows(strGameSecurity);
        RETURN_ON_ERROR(iErrCode);
    }

    // GameNukedEmpires(I.I)
    GET_GAME_NUKED_EMPIRES(strGameNukedEmpires, iGameClass, iGameNumber);
    iErrCode = t_pCache->DeleteAllRows(strGameNukedEmpires);
    RETURN_ON_ERROR(iErrCode);

    // GameData(I.I)
    iErrCode = t_pCache->DeleteAllRows(strGameData);
    RETURN_ON_ERROR(iErrCode);

    // Delete game from active game list
    const char* ppszColumns[] = { SystemActiveGames::GameClass, SystemActiveGames::GameNumber };
    const Variant pvGameData[] = { iGameClass, iGameNumber };

    unsigned int* piGameKey = NULL, iNumEqualKeys;
    AutoFreeKeys free_piGameKey(piGameKey);
    iErrCode = t_pCache->GetEqualKeys(SYSTEM_ACTIVE_GAMES, ppszColumns, pvGameData, countof(ppszColumns), &piGameKey, &iNumEqualKeys);
    RETURN_ON_ERROR(iErrCode);
    Assert(iNumEqualKeys == 1);

    iErrCode = t_pCache->DeleteRow(SYSTEM_ACTIVE_GAMES, piGameKey[0]);
    RETURN_ON_ERROR(iErrCode);

    if (iTournamentKey != NO_KEY)
    {
        // Check for last game in tournament
        unsigned int iOwner;
        iErrCode = GetTournamentOwner (iTournamentKey, &iOwner);
        RETURN_ON_ERROR(iErrCode);

        if (iOwner == DELETED_EMPIRE_KEY)
        {
            unsigned int iNumGames;
            iErrCode = GetTournamentGames(iTournamentKey, NULL, &iNumGames);
            RETURN_ON_ERROR(iErrCode);

            if (iNumGames == 0)
            {
                iErrCode = DeleteTournament(DELETED_EMPIRE_KEY, iTournamentKey, false);
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    // Attempt to delete gameclass if it's marked for deletion
    if (vGameClassOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION)
    {
        unsigned int iNumActiveGames;
        iErrCode = GetGameClassNumActiveGames(iGameClass, &iNumActiveGames);
        RETURN_ON_ERROR(iErrCode);

        if (iNumActiveGames == 0)
        {
            bool bDeleted;
            iErrCode = DeleteGameClass(iGameClass, &bDeleted);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

// Input:
// iGameClass -> GameClass key
// iGameNumber -> GameNumber
//
// Output
// *ptCreationTime -> Creation time of game
//
// Return the creation time of a game

int GameEngine::GetGameCreationTime (int iGameClass, int iGameNumber, UTCTime* ptCreationTime) {

    Variant vTime;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszGameData, GameData::CreationTime, &vTime);
    RETURN_ON_ERROR(iErrCode);

    *ptCreationTime = vTime.GetInteger64();
    
    return iErrCode;
}


int GameEngine::GetGameState (int iGameClass, int iGameNumber, int* piGameState) {

    Variant vValue;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszGameData, GameData::State, &vValue);
    RETURN_ON_ERROR(iErrCode);

    *piGameState = vValue.GetInteger();
    
    return iErrCode;
}

int GameEngine::GetGameClassNumActiveGames(int iGameClass, unsigned int* piNumGames)
{
    int iErrCode = t_pCache->GetEqualKeys(SYSTEM_ACTIVE_GAMES, SystemActiveGames::GameClass, iGameClass, NULL, piNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

// Output:
// *piNumGames -> Number of open games
//
// Return the current number of active (open + closed) games on the server

int GameEngine::GetNumActiveGames(unsigned int* piNumGames)
{
    return t_pCache->GetNumCachedRows(SYSTEM_ACTIVE_GAMES, piNumGames);
}


// Output:
// *piNumGames -> Number of games
//
// Return the current number of open games on the server

int GameEngine::GetNumOpenGames(unsigned int* piNumGames)
{
    int iErrCode = t_pCache->GetEqualKeys(SYSTEM_ACTIVE_GAMES, SystemActiveGames::Open, STILL_OPEN, NULL, piNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}


// Output:
// *piNumGames -> Number of games
//
// Return the current number of closed games on the server

int GameEngine::GetNumClosedGames(unsigned int* piNumGames)
{
    int iErrCode = t_pCache->GetEqualKeys(SYSTEM_ACTIVE_GAMES, SystemActiveGames::Open, CLOSED, false, piNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

// Output:
// **ppiGameClass -> GameClasses of active games
// **ppiGameNumber -> GameNumbers of active games
// *piNumGames -> Number of gamenumbers returned.
//
// Return the keys of the server's active games

int GameEngine::GetActiveGames(Variant*** pppvActiveGames, unsigned int* piNumGames)
{
    int iErrCode;

    if (pppvActiveGames)
        *pppvActiveGames = NULL;
    *piNumGames = 0;

    if (pppvActiveGames)
    {
        const char* ppszColumns[] = { SystemActiveGames::GameClass, SystemActiveGames::GameNumber };

        iErrCode = t_pCache->ReadColumns(SYSTEM_ACTIVE_GAMES, countof(ppszColumns), ppszColumns, NULL, pppvActiveGames, piNumGames);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = t_pCache->GetNumCachedRows(SYSTEM_ACTIVE_GAMES, piNumGames);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

// Output:
// **ppvGameData -> GameClass.GameNumber strings
// *piNumGames -> Number of gamenumbers returned.
//
// Return the keys of the server's open games

int GameEngine::GetOpenGames(Variant*** pppvActiveGames, unsigned int* piNumGames)
{
    return GetGames(true, pppvActiveGames, piNumGames);
}

int GameEngine::GetClosedGames(Variant*** pppvActiveGames, unsigned int* piNumGames)
{
    return GetGames(false, pppvActiveGames, piNumGames);
}

int GameEngine::GetGames(bool bOpen, Variant*** pppvGames, unsigned int* piNumGames)
{
    int iErrCode;

    *pppvGames = NULL;
    *piNumGames = 0;

    const char* ppszGameCols[] = { SystemActiveGames::GameClass, SystemActiveGames::GameNumber };

    iErrCode = t_pCache->ReadColumnsWhereEqual(SYSTEM_ACTIVE_GAMES, SystemActiveGames::Open, bOpen ? STILL_OPEN : CLOSED, 
                                               ppszGameCols, countof(ppszGameCols), NULL, pppvGames, piNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Input:
// iGameClass -> Integer key of a gameclass
// iGameNumber -> Integer key of a gamenumber
//
// Output:
// *pbOpen -> true if the game is open;  false if not
//
// Return true if a game is open

int GameEngine::IsGameOpen (int iGameClass, int iGameNumber, bool* pbOpen) {

    Variant vOpen;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszGameData, GameData::State, &vOpen);
    RETURN_ON_ERROR(iErrCode);

    *pbOpen = (vOpen.GetInteger() & STILL_OPEN) != 0;
    
    return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
// iGameNumber -> Integer key of a gamenumber
//
// Output:
// *pbOpen -> true if the game has started;  false if not
//
// Return true if a game has started

int GameEngine::HasGameStarted (int iGameClass, int iGameNumber, bool* pbStarted) {

    Variant vStarted;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszGameData, GameData::State, &vStarted);
    RETURN_ON_ERROR(iErrCode);
    
    *pbStarted = (vStarted.GetInteger() & STARTED) != 0;

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
//
// Output:
// *pbProtected -> true if a password is required, false otherwise
//
// Return if a game is password protected

int GameEngine::IsGamePasswordProtected (int iGameClass, int iGameNumber, bool* pbProtected)
{
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    Variant vPasswordHash;
    int iErrCode = t_pCache->ReadData(pszGameData, GameData::PasswordHash, &vPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    *pbProtected = !String::IsBlank(vPasswordHash.GetCharPtr());
    
    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// pszPassword -> The game's new password; can be blank
//
// Change a game's password

int GameEngine::SetGamePassword(int iGameClass, int iGameNumber, const char* pszNewPassword)
{
    Variant vNewPasswordHash;
    int iErrCode = ComputePasswordHash(pszNewPassword, &vNewPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);
    return t_pCache->WriteData(pszGameData, GameData::PasswordHash, vNewPasswordHash);
}

// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iProp -> GameData column name
//
// Output:
// *pvProp -> The game property
//
// Return a game property

int GameEngine::GetGameProperty(int iGameClass, int iGameNumber, const char* pszColumn, Variant* pvProp) {

    GET_GAME_DATA(pszGameData, iGameClass, iGameNumber);
    return t_pCache->ReadData(pszGameData, pszColumn, pvProp);
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iProp -> GameData column name
// vProp -> The game's new property
//
// Change a game property

int GameEngine::SetGameProperty(int iGameClass, int iGameNumber, const char* pszColumn, const Variant& vProp) {

    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);
    return t_pCache->WriteData(pszGameData, pszColumn, vProp);
}


// Input:
// iGameClass -> Game class
// iGameNumber -> Game number
// iEmpireKey -> Empire key of creator
// goGameOptions -> Game options provided by creator
//
// Output:
// *piGameNumber -> Gamenumber of new game
//
// Create a new game.

int GameEngine::CreateGame(int iGameClass, int iEmpireCreator, const GameOptions& goGameOptions, int* piGameNumber) {

    int iNumUpdates, iErrCode;
    unsigned int i, j, k;

    Variant vTemp, vOptions, vHalted, vPrivilege, vEmpireScore, vGameNumber, vMaxNumActiveGames;

    bool bFlag;

    UTCTime tTime;
    Time::GetTime (&tTime);

    const unsigned int* piEmpireKey = goGameOptions.piEmpireKey;
    const unsigned int iNumEmpires = goGameOptions.iNumEmpires;

    Assert(iEmpireCreator != TOURNAMENT || (piEmpireKey != NULL && iNumEmpires > 0));

    // Make sure new game creation is enabled
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    if (!(vTemp.GetInteger() & NEW_GAMES_ENABLED))
    {
        return ERROR_DISABLED;
    }

    int* piCopy = (int*)StackAlloc(iNumEmpires * sizeof(int));
    memcpy(piCopy, piEmpireKey, iNumEmpires * sizeof(int));
    Algorithm::QSortAscending<int>(piCopy, iNumEmpires);

    // Check empire existence
    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = DoesEmpireExist(piCopy[i], &bFlag, NULL);
        RETURN_ON_ERROR(iErrCode);
        if (!bFlag)
        {
            return ERROR_EMPIRE_DOES_NOT_EXIST;
        }
    }

    // Test for gameclass halt
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vOptions);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    if (vOptions.GetInteger() & GAMECLASS_HALTED)
    {
        return ERROR_GAMECLASS_HALTED;
    }

    if (vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION && !(vOptions.GetInteger() & DYNAMIC_GAMECLASS))
    {
        return ERROR_GAMECLASS_DELETED;
    }

    if (goGameOptions.iTournamentKey != NO_KEY)
    {
        // Make sure empires can be entered into tournament game
        for (i = 0; i < iNumEmpires; i ++)
        {
            int iOpt2;
            iErrCode = GetEmpireOptions2(piEmpireKey[i], &iOpt2);
            RETURN_ON_ERROR(iErrCode);
            
            if (iOpt2 & UNAVAILABLE_FOR_TOURNAMENTS)
            {
                return ERROR_EMPIRE_IS_UNAVAILABLE_FOR_TOURNAMENTS;
            }
        }
    }

    // Test for too many games
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxNumActiveGames, &vMaxNumActiveGames);
    RETURN_ON_ERROR(iErrCode);

    if (vMaxNumActiveGames.GetInteger() != INFINITE_ACTIVE_GAMES)
    {
        unsigned int iNumActiveGames;
        iErrCode = GetGameClassNumActiveGames(iGameClass, &iNumActiveGames);
        RETURN_ON_ERROR(iErrCode);
        
        if (iNumActiveGames >= (unsigned int)vMaxNumActiveGames.GetInteger())
        {
            return ERROR_TOO_MANY_GAMES;
        }
    }

    for (i = 0; i < iNumEmpires; i ++)
    {
        GET_SYSTEM_EMPIRE_DATA(strSystemEmpireData, piEmpireKey[i]);

        // Make sure empire isn't halted
        iErrCode = t_pCache->ReadData(strSystemEmpireData, piEmpireKey[i], SystemEmpireData::Options, &vHalted);
        RETURN_ON_ERROR(iErrCode);

        if (vHalted.GetInteger() & EMPIRE_MARKED_FOR_DELETION)
        {
            return ERROR_EMPIRE_IS_HALTED;
        }

        // Make sure empire is at least a novice
        iErrCode = t_pCache->ReadData(strSystemEmpireData, piEmpireKey[i], SystemEmpireData::Privilege, &vPrivilege);
        RETURN_ON_ERROR(iErrCode);

        if (vPrivilege.GetInteger() < NOVICE)
        {
            return ERROR_INSUFFICIENT_PRIVILEGE;
        }
    }

    // Get unique game number and increment it
    iErrCode = t_pCache->Increment(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::OpenGameNum, 1, &vGameNumber);
    RETURN_ON_ERROR(iErrCode);

    *piGameNumber = vGameNumber.GetInteger();

    unsigned int iTournamentKey;
    iErrCode = GetGameClassTournament(iGameClass, &iTournamentKey);
    RETURN_ON_ERROR(iErrCode);

    // Inset row into SystemActiveGames
    Variant pvActiveGameData[SystemActiveGames::NumColumns] = 
    {
        iGameClass,
        *piGameNumber,
        STILL_OPEN,
        iTournamentKey,
    };

    iErrCode = t_pCache->InsertRow(SYSTEM_ACTIVE_GAMES, SystemActiveGames::Template, pvActiveGameData, NULL);
    RETURN_ON_ERROR(iErrCode);

    ////////////////////////////
    // Populate system tables //
    ////////////////////////////

    GET_GAME_DATA(strGameData, iGameClass, *piGameNumber);
    iErrCode = t_pCache->CreateEmpty(GAME_DATA, strGameData);
    RETURN_ON_ERROR(iErrCode);

    GET_GAME_NUKED_EMPIRES(strGameNukedEmpires, iGameClass, *piGameNumber);
    iErrCode = t_pCache->CreateEmpty(GAME_NUKED_EMPIRES, strGameNukedEmpires);
    RETURN_ON_ERROR(iErrCode);

    Variant vEmpireName = "";
    if (iEmpireCreator != TOURNAMENT)
    {
        iErrCode = GetEmpireName(iEmpireCreator, &vEmpireName);
        RETURN_ON_ERROR(iErrCode);
    }
    Assert(vEmpireName.GetCharPtr());

    // Add row to GameData(I.I)
    Variant pvGameData[GameData::NumColumns] = 
    {
        iGameClass,
        *piGameNumber,
        0,      // Max num empires so far
        0,      // 0 updates
        tTime,  // Last updated now
        STILL_OPEN,     // State
        MAX_COORDINATE,     // MinX
        0,      // 0 empires updated
        (const char*)NULL,  // Password
        MIN_COORDINATE,     // MaxX
        MAX_COORDINATE,     // MinY
        0,                  // Zero paused
        MIN_COORDINATE,     // MaxY
        0,          // SecondsUntilNextUpdateWhilePaused
        tTime,      // CreationTime
        0,          // NumPlanetsPerEmpire
        0,          // HWAg
        0,          // AvgAg
        0,          // HWMin
        0,          // AvgMin
        0,          // HWFuel
        0,          // AvgFuel
        0,          // NumEmpiresResigned
        goGameOptions.iOptions,                     // Options
        goGameOptions.iNumUpdatesBeforeGameCloses,  // NumUpdatesBeforeGameCloses
        goGameOptions.sFirstUpdateDelay,            // FirstUpdateDelay
        goGameOptions.pszEnterGameMessage,          // EnterGameMessage
        vEmpireName.GetCharPtr(),                   // CreatorName
        goGameOptions.fMinAlmonasterScore,  // MinAlmonasterScore,
        goGameOptions.fMaxAlmonasterScore,  // MaxAlmonasterScore,
        goGameOptions.fMinClassicScore,     // MinClassicScore
        goGameOptions.fMaxClassicScore,     // MaxClassicScore
        goGameOptions.iMinBridierRank,      // MinBridierRank
        goGameOptions.iMaxBridierRank,      // MaxBridierRank
        goGameOptions.iMinBridierIndex,     // MinBridierIndex
        goGameOptions.iMaxBridierIndex,     // MaxBridierIndex
        goGameOptions.iMinBridierRankGain,  // MinBridierRankGain
        goGameOptions.iMaxBridierRankGain,  // MaxBridierRankGain
        goGameOptions.iMinWins,             // MinWins
        goGameOptions.iMaxWins,             // MaxWins
        0,                                  // NumRequestingDraw
        goGameOptions.iMinBridierRankLoss,  // MinBridierRankLoss
        goGameOptions.iMaxBridierRankLoss,  // MaxBridierRankLoss
        tTime,                              // RealLastUpdateTime
        goGameOptions.gfoFairness,          // MapFairness
        0                                   // MapFairnessStandardDeviationPercentageOfMean
    };

    if (goGameOptions.pszPassword)
    {
        iErrCode = ComputePasswordHash(goGameOptions.pszPassword, pvGameData + GameData::iPasswordHash);
        RETURN_ON_ERROR(iErrCode);
        Assert(pvGameData[GameData::iPasswordHash].GetCharPtr());
    }
    Assert(pvGameData[GameData::iCreatorName].GetCharPtr());
    if (goGameOptions.pszEnterGameMessage)
    {
        Assert(pvGameData[GameData::iEnterGameMessage].GetCharPtr());
    }

    iErrCode = t_pCache->InsertRow(strGameData, GameData::Template, pvGameData, NULL);
    RETURN_ON_ERROR(iErrCode);

    // Create GameSecurity(I.I) table if necessary
    if (goGameOptions.iOptions & GAME_ENFORCE_SECURITY)
    {
        unsigned int i;
        Assert(goGameOptions.iOptions > 0);

        // Bulk cache relevant empire rows
        if (goGameOptions.iNumSecurityEntries > 0)
        {
            unsigned int* piEmpireEntries = (unsigned int*)StackAlloc(goGameOptions.iNumSecurityEntries * sizeof(unsigned int));
            for (i = 0; i < goGameOptions.iNumSecurityEntries; i ++)
            {
                piEmpireEntries[i] = goGameOptions.pSecurity[i].iEmpireKey;
            }

            iErrCode = CacheEmpires(piEmpireEntries, goGameOptions.iNumSecurityEntries);
            RETURN_ON_ERROR(iErrCode);
        }

        // Populate rows
        Variant pvGameSec[GameSecurity::NumColumns];
        for (i = 0; i < goGameOptions.iNumSecurityEntries; i ++)
        {
            ICachedTable* pEmpires = NULL;
            AutoRelease<ICachedTable> release(pEmpires);

            unsigned int iRowEmpireKey = goGameOptions.pSecurity[i].iEmpireKey;
            GET_SYSTEM_EMPIRE_DATA(strEmpire, iRowEmpireKey);

            //
            // The idea is to ignore rows that don't resolve to the intended empire
            //

            iErrCode = t_pCache->GetTable(strEmpire, &pEmpires);
            RETURN_ON_ERROR(iErrCode);

            pvGameSec[GameSecurity::iGameClass] = iGameClass;
            pvGameSec[GameSecurity::iGameNumber] = *piGameNumber;
            pvGameSec[GameSecurity::iEmpireKey] = iRowEmpireKey;
            pvGameSec[GameSecurity::iOptions] = goGameOptions.pSecurity[i].iOptions;

            // Get empire name, secret key, ip address, and session id
            iErrCode = pEmpires->ReadData(iRowEmpireKey, SystemEmpireData::Name, pvGameSec + GameSecurity::iName);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = pEmpires->ReadData(iRowEmpireKey, SystemEmpireData::IPAddress, pvGameSec + GameSecurity::iIPAddress);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = pEmpires->ReadData(iRowEmpireKey, SystemEmpireData::SessionId, pvGameSec + GameSecurity::iSessionId);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = pEmpires->ReadData(iRowEmpireKey, SystemEmpireData::SecretKey, pvGameSec + GameSecurity::iSecretKey);
            RETURN_ON_ERROR(iErrCode);

            // Check name against string provided by creator
            if (String::StriCmp(pvGameSec[GameSecurity::iName].GetCharPtr(), goGameOptions.pSecurity[i].pszEmpireName) != 0)
            {
                return ERROR_INVALID_ARGUMENT;
            }

            // Insert row
            GET_GAME_SECURITY(strGameSecurity, iGameClass, *piGameNumber);
            iErrCode = t_pCache->CreateEmpty(GAME_SECURITY, strGameSecurity);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->InsertRow(strGameSecurity, GameSecurity::Template, pvGameSec, NULL);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Enter empires into game
    GET_GAME_EMPIRES(strGameEmpires, iGameClass, *piGameNumber);
    iErrCode = t_pCache->CreateEmpty(GAME_EMPIRES, strGameEmpires);
    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumEmpires; i ++)
    {
        iErrCode = EnterGame(
            iGameClass,
            *piGameNumber,
            piEmpireKey[i],
            NULL,
            &goGameOptions,
            &iNumUpdates, 
            iEmpireCreator != TOURNAMENT, // Send messages
            true, // Creating game
            iEmpireCreator != TOURNAMENT // Check security
            );

        RETURN_ON_ERROR(iErrCode);
    }

    // Handle team arrangements
    if (goGameOptions.iTeamOptions != 0)
    {
        // Insert diplomatic entries if necessary
        if (goGameOptions.iTeamOptions & TEAM_PREARRANGED_DIPLOMACY && !(vOptions.GetInteger() & EXPOSED_DIPLOMACY))
        {
            Variant pvInsert[GameEmpireDiplomacy::NumColumns] = 
            {
                iGameClass,
                *piGameNumber,
                NO_KEY,
                NO_KEY,
                WAR,
                WAR,
                WAR,
                0,
                0,
                0,
                0,
            };

            // Add each empire to its teammate's diplomacy table
            for (i = 0; i < goGameOptions.iNumPrearrangedTeams; i ++)
            {
                const PrearrangedTeam& aTeam = goGameOptions.paPrearrangedTeam[i];

                for (j = 0; j < aTeam.iNumEmpires; j ++)
                {
                    for (k = j + 1; k < aTeam.iNumEmpires; k ++)
                    {
                        GET_GAME_EMPIRE_DIPLOMACY(strEmpireDip1, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);
                        pvInsert[GameEmpireDiplomacy::iEmpireKey] = aTeam.piEmpireKey[j];
                        pvInsert[GameEmpireDiplomacy::iReferenceEmpireKey] = aTeam.piEmpireKey[k];

                        iErrCode = t_pCache->InsertRow(strEmpireDip1, GameEmpireDiplomacy::Template, pvInsert, NULL);
                        RETURN_ON_ERROR(iErrCode);

                        GET_GAME_EMPIRE_DIPLOMACY(strEmpireDip2, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);
                        pvInsert[GameEmpireDiplomacy::iEmpireKey] = aTeam.piEmpireKey[k];
                        pvInsert[GameEmpireDiplomacy::iReferenceEmpireKey] = aTeam.piEmpireKey[j];

                        iErrCode = t_pCache->InsertRow(strEmpireDip2, GameEmpireDiplomacy::Template, pvInsert, NULL);
                        RETURN_ON_ERROR(iErrCode);
                    }
                }
            }

        }   // End if insertions are needed

        if (goGameOptions.iTeamOptions & TEAM_PREARRANGED_ALLIANCES)
        {
            int iDipLevel;
            iErrCode = GetGameClassDiplomacyLevel(iGameClass, &iDipLevel);
            RETURN_ON_ERROR(iErrCode);

            Variant vDipLevel;
            iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MapsShared, &vDipLevel);
            RETURN_ON_ERROR(iErrCode);

            bool bShared = vDipLevel.GetInteger() == ALLIANCE;

            if (iDipLevel & ALLIANCE)
            {
                unsigned int iMaxAlliances = 0;

                // Set each empire to alliance in teammate's diplomacy table
                for (i = 0; i < goGameOptions.iNumPrearrangedTeams; i ++)
                {
                    const PrearrangedTeam& aTeam = goGameOptions.paPrearrangedTeam[i];
                    unsigned int iKey;

                    if (aTeam.iNumEmpires - 1 > (unsigned int) iMaxAlliances)
                    {
                        iMaxAlliances = aTeam.iNumEmpires - 1;
                    }

                    for (j = 0; j < aTeam.iNumEmpires; j ++)
                    {
                        GET_GAME_EMPIRE_DATA(strGameEmpireData, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);

                        for (k = j + 1; k < aTeam.iNumEmpires; k ++)
                        {
                            GET_GAME_EMPIRE_DIPLOMACY(strDip1, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);

                            iErrCode = t_pCache->GetFirstKey(strDip1, GameEmpireDiplomacy::ReferenceEmpireKey, aTeam.piEmpireKey[k], &iKey);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->WriteData(strDip1, iKey, GameEmpireDiplomacy::DipOffer, ALLIANCE);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->WriteData(strDip1, iKey, GameEmpireDiplomacy::CurrentStatus, ALLIANCE);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->WriteData(strDip1, iKey, GameEmpireDiplomacy::DipOfferLastUpdate, ALLIANCE);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->WriteOr(strDip1, iKey, GameEmpireDiplomacy::State, ONCE_ALLIED_WITH);
                            RETURN_ON_ERROR(iErrCode);

                            GET_GAME_EMPIRE_DIPLOMACY(strDip2, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);

                            iErrCode = t_pCache->GetFirstKey(strDip2, GameEmpireDiplomacy::ReferenceEmpireKey, aTeam.piEmpireKey[j], &iKey);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->WriteData(strDip2, iKey, GameEmpireDiplomacy::DipOffer, ALLIANCE);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->WriteData(strDip2, iKey, GameEmpireDiplomacy::CurrentStatus, ALLIANCE);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->WriteData(strDip2, iKey, GameEmpireDiplomacy::DipOfferLastUpdate, ALLIANCE);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->WriteOr(strDip2, iKey, GameEmpireDiplomacy::State, ONCE_ALLIED_WITH);
                            RETURN_ON_ERROR(iErrCode);

                            // Share maps
                            if (bShared)
                            {
                                GET_GAME_EMPIRE_MAP(pszMap1, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);
                                GET_GAME_EMPIRE_MAP(pszMap2, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);

                                GET_GAME_EMPIRE_DATA(pszData1, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);
                                GET_GAME_EMPIRE_DATA(pszData2, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);

                                GET_GAME_MAP(pszMap, iGameClass, *piGameNumber);

                                char* ppszDip[2] = { strDip1, strDip2 };
                                char* ppszMap[2] = { pszMap1, pszMap2 };
                                char* ppszData[2] = { pszData1, pszData2 };

                                unsigned int piEmpireKey[2] = { aTeam.piEmpireKey[j], aTeam.piEmpireKey[k] };

                                iErrCode = SharePlanetsBetweenFriends (
                                    iGameClass, 
                                    *piGameNumber, 
                                    0, 
                                    1,
                                    (const char**) ppszMap,
                                    (const char**) ppszDip,
                                    (const char**) ppszData, 
                                    pszMap,
                                    countof(piEmpireKey),
                                    piEmpireKey,
                                    ALLIANCE,
                                    false
                                    );

                                RETURN_ON_ERROR(iErrCode);
                            }
                        }
                    }
                }
                
                unsigned int iAllianceLimit;
                iErrCode = GetMaxNumDiplomacyPartners (iGameClass, *piGameNumber, ALLIANCE, &iAllianceLimit);
                RETURN_ON_ERROR(iErrCode);

                if (iAllianceLimit != UNRESTRICTED_DIPLOMACY && iMaxAlliances > iAllianceLimit)
                {
                    return ERROR_ALLIANCE_LIMIT_EXCEEDED;
                }
            }
        }
    }

    //
    // No errors from this point on
    //

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];
    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    // Send notification messages for tournament games
    if (iEmpireCreator == TOURNAMENT)
    {
        char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];
        sprintf (
            pszMessage,
            "The tournament game %s %i has been started",
            pszGameClassName,
            *piGameNumber
            );

        for (i = 0; i < iNumEmpires; i ++)
        {
            iErrCode = SendSystemMessage (piEmpireKey[i], pszMessage, SYSTEM, MESSAGE_SYSTEM);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // UI notification
    global.GetEventSink()->OnCreateGame(iGameClass, *piGameNumber);

    char pszUpdateReport [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    sprintf (
        pszUpdateReport,
        "%s %i was created",
        pszGameClassName,
        *piGameNumber
        );

    global.WriteReport(TRACE_INFO, pszUpdateReport);

    return iErrCode;
}


// Input:
// iGameClass -> Game class
// iGameNumber -> Game number
// iEmpireKey -> Empire key
// pszPassword -> Password provided by user
//
// Output:
// *piNumUpdates -> Number of updates transpired
//
// Make an empire enter an already created game.

int GameEngine::EnterGame(int iGameClass, int iGameNumber, int iEmpireKey, const char* pszPassword,
                          const GameOptions* pgoGameOptions, int* piNumUpdates, 
                          bool bSendMessages, bool bCreatingGame, bool bCheckSecurity)
{
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_NUKED_EMPIRES (strGameDeadEmpires, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_SHIPS (strGameEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    String strDuplicateIPList, strDuplicateIdList, strDuplicateList, strMessage;
    bool bWarn, bBlock, bFlag, bAddedToGame = false, bClosed = false, bStarted = false, bPaused = false, bUnPaused = false;

    int iErrCode, iNumTechs, iDefaultOptions, iGameState, iSystemOptions, iEmpireOptions;
    unsigned int i, iKey = NO_KEY;

    Variant vGameClassOptions, vHalted, vPrivilege, vMinScore, vMaxScore, vEmpireScore, vTemp, 
        vMaxNumEmpires, vStillOpen, vNumUpdates, vMaxTechDev, vDiplomacyLevel,
        vGameOptions, vSecretKey, * pvEmpireKey = NULL;
    AutoFreeData free(pvEmpireKey);

    bool bGenerateMapForAllEmpires = false;

    GameFairnessOption gfoFairness;

    Variant pvGameEmpireData[GameEmpireData::NumColumns];

    // Get time
    UTCTime tTime;
    Time::GetTime (&tTime);

    iErrCode = DoesGameExist(iGameClass, iGameNumber, &bFlag);
    RETURN_ON_ERROR(iErrCode);
    if (!bFlag)
    {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vGameClassOptions);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemOptions (&iSystemOptions);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::DiplomacyLevel, &vDiplomacyLevel);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strGameData, GameData::Options, &vGameOptions);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strGameData, GameData::MapFairness, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    gfoFairness = (GameFairnessOption)vTemp.GetInteger();

    // Make sure we still exist, kill game if not and we just created it and we're alone
    Variant vEmpireName;
    iErrCode = DoesEmpireExist (iEmpireKey, &bFlag, &vEmpireName);
    RETURN_ON_ERROR(iErrCode);
    if (!bFlag)
    {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    // Make sure empire isn't halted
    iErrCode = GetEmpireOptions (iEmpireKey, &iEmpireOptions);
    RETURN_ON_ERROR(iErrCode);
    
    if (iEmpireOptions & EMPIRE_MARKED_FOR_DELETION)
    {
        return ERROR_EMPIRE_IS_HALTED;
    }

    // Make sure empire hasn't entered already  
    iErrCode = t_pCache->GetFirstKey(strGameEmpires, GameEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == OK)
    {
        return ERROR_ALREADY_IN_GAME;
    }
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::SecretKey, &vSecretKey);
    RETURN_ON_ERROR(iErrCode);

    // Make sure empire wasn't nuked out
    iErrCode = t_pCache->GetFirstKey(strGameDeadEmpires, GameNukedEmpires::SecretKey, vSecretKey, &iKey);
    if (iErrCode == OK)
    {
        return ERROR_WAS_ALREADY_IN_GAME;
    }
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
    }

    // Make sure game is still open
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameState = vTemp.GetInteger();
    if (!(iGameState & STILL_OPEN))
    {
        return ERROR_GAME_CLOSED;
    }

    // Test for correct password
    if (pszPassword == NULL && pgoGameOptions != NULL)
    {
        pszPassword = pgoGameOptions->pszPassword;
    }

    iErrCode = IsGamePasswordCorrect(iGameClass, iGameNumber, pszPassword);
    if (iErrCode == ERROR_PASSWORD)
    {
        return iErrCode;
    }
    RETURN_ON_ERROR(iErrCode);

    // Read all empires in game
    unsigned int iCurrentNumEmpires;
    iErrCode = t_pCache->ReadColumn(strGameEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iCurrentNumEmpires);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        iErrCode = CacheEmpires(pvEmpireKey, iCurrentNumEmpires);
        RETURN_ON_ERROR(iErrCode);
    }

    if (bCheckSecurity)
    {
        GameAccessDeniedReason reason;
        iErrCode = GameAccessCheck(iGameClass, iGameNumber, iEmpireKey, NULL, ENTER_GAME, &bFlag, &reason);
        RETURN_ON_ERROR(iErrCode);

        if (!bFlag)
        {
            return ERROR_ACCESS_DENIED;
        }

        // Search for duplicates        
        bWarn  = (vGameOptions.GetInteger() & GAME_WARN_ON_DUPLICATE_IP_ADDRESS) != 0;
        bBlock = (vGameOptions.GetInteger() & GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS) != 0;
        
        if (bWarn || bBlock)
        {
            int* piDuplicateKeys = NULL;
            Algorithm::AutoDelete<int> del(piDuplicateKeys, true);
            unsigned int iNumDuplicates;
            
            iErrCode = DoesEmpireHaveDuplicates(
                iGameClass, 
                iGameNumber, 
                iEmpireKey,
                SystemEmpireData::IPAddress,
                &piDuplicateKeys, 
                &iNumDuplicates
                );
            RETURN_ON_ERROR(iErrCode);
            
            if (iNumDuplicates > 0)
            {
                iErrCode = BuildDuplicateList (piDuplicateKeys, iNumDuplicates, &strDuplicateList);
                RETURN_ON_ERROR(iErrCode);
                
                if (bWarn && !bBlock)
                {
                    // Post message
                    strDuplicateIPList = BEGIN_STRONG;
                    strDuplicateIPList += vEmpireName.GetCharPtr();
                    strDuplicateIPList += END_STRONG " has the same " BEGIN_STRONG "IP address" END_STRONG " as ";
                    strDuplicateIPList += strDuplicateList;
                    
                    Assert(strDuplicateIPList.GetCharPtr());
                }
                else if (!bWarn && bBlock)
                {
                    return ERROR_DUPLICATE_IP_ADDRESS;
                }
                else if (bWarn && bBlock)
                {
                    // Prepare message
                    char* pszMessage = (char*) StackAlloc (256 + MAX_EMPIRE_NAME_LENGTH + strDuplicateList.GetLength());

                    sprintf (
                        pszMessage, 
                        BEGIN_STRONG "%s attempted to enter the game with the same " BEGIN_STRONG "IP address" END_STRONG " as %s",
                        vEmpireName.GetCharPtr(),
                        strDuplicateList.GetCharPtr()
                        );

                    iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
                    RETURN_ON_ERROR(iErrCode);

                    // Exit with access denied
                    return ERROR_DUPLICATE_IP_ADDRESS;
                }
            }
        }
        
        bWarn  = (vGameOptions.GetInteger() & GAME_WARN_ON_DUPLICATE_SESSION_ID) != 0;
        bBlock = (vGameOptions.GetInteger() & GAME_BLOCK_ON_DUPLICATE_SESSION_ID) != 0;
        
        if (bWarn || bBlock)
        {
            int* piDuplicateKeys;
            Algorithm::AutoDelete<int> del(piDuplicateKeys, true);
            unsigned int iNumDuplicates;
            
            iErrCode = DoesEmpireHaveDuplicates (
                iGameClass, 
                iGameNumber, 
                iEmpireKey,
                SystemEmpireData::SessionId,
                &piDuplicateKeys, 
                &iNumDuplicates
                );

            RETURN_ON_ERROR(iErrCode);
            
            if (iNumDuplicates > 0)
            {
                iErrCode = BuildDuplicateList (piDuplicateKeys, iNumDuplicates, &strDuplicateList);
                RETURN_ON_ERROR(iErrCode);
                
                if (bWarn && !bBlock)
                {
                    // Post message
                    strDuplicateIdList = BEGIN_STRONG;
                    strDuplicateIdList += vEmpireName.GetCharPtr();
                    strDuplicateIdList += END_STRONG " has the same " BEGIN_STRONG "Session Id" END_STRONG " as ";
                    strDuplicateIdList += strDuplicateList;
                    
                    Assert(strDuplicateIdList.GetCharPtr());
                }
                else if (!bWarn && bBlock)
                {
                    // Exit with access denied
                    return ERROR_DUPLICATE_SESSION_ID;
                }
                else if (bWarn && bBlock)
                {
                    // Prepare message
                    char* pszMessage = (char*) StackAlloc (256 + MAX_EMPIRE_NAME_LENGTH + strDuplicateList.GetLength());

                    sprintf (
                        pszMessage, 
                        BEGIN_STRONG "%s attempted to enter the game with the same " BEGIN_STRONG "Session Id" END_STRONG " as %s",
                        vEmpireName.GetCharPtr(),
                        strDuplicateList.GetCharPtr()
                        );
                    
                    // Best effort
                    iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
                    RETURN_ON_ERROR(iErrCode);

                    return ERROR_DUPLICATE_SESSION_ID;
                }
            }
        }

        // Check for 'pause on start' option
        if (!(iSystemOptions & PAUSE_GAMES_BY_DEFAULT)) {

            // If not admin-paused, unpause the game - a new empire came in and he's not paused by default
            if ((iGameState & PAUSED) && !(iGameState & ADMIN_PAUSED))
            {
                bUnPaused = true;

                iErrCode = UnpauseGame (iGameClass, iGameNumber, false, false);
                RETURN_ON_ERROR(iErrCode);

                iGameState &= ~PAUSED;
            }
        }
        
        // Try to trigger all remaining updates
        if (!bCreatingGame)
        {
            iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, &bFlag);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (!bCreatingGame)
    {
        // Make sure the game didn't end
        iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
        RETURN_ON_ERROR(iErrCode);
        if (!bFlag)
        {
            return ERROR_GAME_DOES_NOT_EXIST;
        }

        // Make sure game is still open
        iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        iGameState = vTemp.GetInteger();

        if (!(iGameState & STILL_OPEN))
        {
            return ERROR_GAME_CLOSED;
        }
    }

    char pszGameClassName[MAX_FULL_GAME_CLASS_NAME_LENGTH];
    iErrCode = GetGameClassName(iGameClass, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);
    
    // Insert row into GameEmpires(I.I) table
    Variant pvGameEmpire[GameEmpires::NumColumns] =
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        vEmpireName
    };
    Assert(pvGameEmpire[GameEmpires::iEmpireName].GetCharPtr());

    iErrCode = t_pCache->InsertRow(strGameEmpires, GameEmpires::Template, pvGameEmpire, NULL);
    RETURN_ON_ERROR(iErrCode);

    iCurrentNumEmpires ++;
    bAddedToGame = true;
    
    // Increment MaxNumEmpires count if game has started
    if (iGameState & STARTED) {

        iErrCode = t_pCache->ReadData(strGameData, GameData::MaxNumEmpires, &vMaxNumEmpires);
        RETURN_ON_ERROR(iErrCode);
        
        if ((int) iCurrentNumEmpires > vMaxNumEmpires.GetInteger())
        {
            iErrCode = t_pCache->WriteData(strGameData, GameData::MaxNumEmpires, (int)iCurrentNumEmpires);
            RETURN_ON_ERROR(iErrCode);
        }
    }
    
    // Close game if we're the last to enter    
    if (iGameState & STILL_OPEN)
    {
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxNumEmpires, &vMaxNumEmpires);
        RETURN_ON_ERROR(iErrCode);
        
        if (vMaxNumEmpires.GetInteger() != UNLIMITED_EMPIRES &&
            vMaxNumEmpires.GetInteger() == (int) iCurrentNumEmpires)
        {
            bClosed = true;
            
            // Close game
            iErrCode = t_pCache->WriteAnd(strGameData, GameData::State, ~STILL_OPEN);
            RETURN_ON_ERROR(iErrCode);

            iGameState &= ~STILL_OPEN;
            
            // Delete from open list
            const char* ppszColumns[] = { SystemActiveGames::GameClass, SystemActiveGames::GameNumber };
            const Variant pvGameData[] = { iGameClass, iGameNumber };

            unsigned int* piGameKey = NULL, iNumEqualKeys;
            AutoFreeKeys free_piGameKey(piGameKey);
            iErrCode = t_pCache->GetEqualKeys(SYSTEM_ACTIVE_GAMES, ppszColumns, pvGameData, countof(ppszColumns), &piGameKey, &iNumEqualKeys);
            RETURN_ON_ERROR(iErrCode);
            Assert(iNumEqualKeys == 1);

            iErrCode = t_pCache->WriteData(SYSTEM_ACTIVE_GAMES, piGameKey[0], SystemActiveGames::Open, CLOSED);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Insert row into SystemEmpireActiveGames table
    Variant pvEmpireActiveGames[SystemEmpireActiveGames::NumColumns] =
    {
        iEmpireKey, 
        iGameClass,
        iGameNumber,
    };
        
    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(strEmpireActiveGames, iEmpireKey);
    iErrCode = t_pCache->InsertRow(strEmpireActiveGames, SystemEmpireActiveGames::Template, pvEmpireActiveGames, NULL);
    RETURN_ON_ERROR(iErrCode);

    /////////////////////////
    // Add rows for empire //
    /////////////////////////
    
    // Insert data into GameEmpireData(I.I.I) table
    pvGameEmpireData[GameEmpireData::iGameClass] = iGameClass;
    pvGameEmpireData[GameEmpireData::iGameNumber] = iGameNumber;
    pvGameEmpireData[GameEmpireData::iEmpireKey] = iEmpireKey;
    pvGameEmpireData[GameEmpireData::iNumPlanets] = 1;
    pvGameEmpireData[GameEmpireData::iTotalAg] = 0;
    pvGameEmpireData[GameEmpireData::iTotalFuel] = 0;
    pvGameEmpireData[GameEmpireData::iTotalMin] = 0;
    pvGameEmpireData[GameEmpireData::iTotalPop] = 0;
    
    // Initial tech level
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::InitialTechLevel, 
        pvGameEmpireData + GameEmpireData::iTechLevel
        );
    RETURN_ON_ERROR(iErrCode);
    
    // Add tech to initial tech if empire is a late-comer
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vNumUpdates);
    RETURN_ON_ERROR(iErrCode);
    
    if (vNumUpdates.GetInteger() > 0) {

        Variant vPercentTechIncreaseForLatecomers;

        iErrCode = t_pCache->ReadData(
            SYSTEM_DATA,
            SystemData::PercentTechIncreaseForLatecomers,
            &vPercentTechIncreaseForLatecomers
            );

        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxTechDev, 
            &vMaxTechDev
            );
        RETURN_ON_ERROR(iErrCode);

        pvGameEmpireData[GameEmpireData::iTechLevel] += GetLateComerTechIncrease (
            vPercentTechIncreaseForLatecomers.GetInteger(),
            vNumUpdates.GetInteger(), 
            vMaxTechDev.GetFloat()
            );
    }

    pvGameEmpireData[GameEmpireData::iTotalBuild] = 0;
    pvGameEmpireData[GameEmpireData::iTotalMaintenance] = 0;
    pvGameEmpireData[GameEmpireData::iTotalFuelUse] = 0;
    pvGameEmpireData[GameEmpireData::iLastLogin] = tTime;
    pvGameEmpireData[GameEmpireData::iEnterGameIPAddress] = "";
    pvGameEmpireData[GameEmpireData::iPartialMapCenter] = PARTIAL_MAP_NATURAL_CENTER;
    pvGameEmpireData[GameEmpireData::iPartialMapXRadius] = PARTIAL_MAP_UNLIMITED_RADIUS;
    pvGameEmpireData[GameEmpireData::iPartialMapYRadius] = PARTIAL_MAP_UNLIMITED_RADIUS;

    // Initial number of techs to be developed is the min of the initial BR and the number
    // of developable techs that a game class allows
    iNumTechs = (int)sqrt(pvGameEmpireData[GameEmpireData::iTechLevel].GetFloat());

    // TechDevs
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::InitialTechDevs, 
        &pvGameEmpireData[GameEmpireData::iTechDevs]
        );
    RETURN_ON_ERROR(iErrCode);
    
    // TechUndevs
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DevelopableTechDevs, 
        pvGameEmpireData + GameEmpireData::iTechUndevs
        );

    RETURN_ON_ERROR(iErrCode);

    // NumAvailableTechUndevs
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::NumInitialTechDevs, 
        pvGameEmpireData + GameEmpireData::iNumAvailableTechUndevs
        );

    RETURN_ON_ERROR(iErrCode);
    
    // Filter out already developed techs
    ENUMERATE_TECHS(i)
    {
        if (pvGameEmpireData[GameEmpireData::iTechDevs].GetInteger() & TECH_BITS[i])
        {
            Assert(pvGameEmpireData[GameEmpireData::iTechUndevs].GetInteger() & TECH_BITS[i]);
            pvGameEmpireData[GameEmpireData::iTechUndevs] = pvGameEmpireData[GameEmpireData::iTechUndevs].GetInteger() & ~(TECH_BITS[i]);
        }
    }

    // Fill in the rest of the data
    pvGameEmpireData[GameEmpireData::iEcon] = 1;                                                 // Econ
    pvGameEmpireData[GameEmpireData::iMil] = (float) 0.0;                                        // Mil
    pvGameEmpireData[GameEmpireData::iTargetPop] = 0;
    pvGameEmpireData[GameEmpireData::iHomeWorld] = NO_KEY;                                       // HWKey
    pvGameEmpireData[GameEmpireData::iNumUpdatesIdle] = 0;                                   // 0 updates idle
    pvGameEmpireData[GameEmpireData::iMaxBR] = GetBattleRank((float)pvGameEmpireData[GameEmpireData::iTechLevel]);  // Max BR
    pvGameEmpireData[GameEmpireData::iBonusAg] = 0;                  // 0 bonus ag
    pvGameEmpireData[GameEmpireData::iBonusMin] = 0;                 // 0 bonus min
    pvGameEmpireData[GameEmpireData::iBonusFuel] = 0;                // 0 bonus fuel
    pvGameEmpireData[GameEmpireData::iNumBuilds] = 0;                // No ships being built
    pvGameEmpireData[GameEmpireData::iMinX] = MAX_COORDINATE;        // MinX
    pvGameEmpireData[GameEmpireData::iMaxX] = MIN_COORDINATE;        // MaxX
    pvGameEmpireData[GameEmpireData::iMinY] = MAX_COORDINATE;        // MinY
    pvGameEmpireData[GameEmpireData::iMaxY] = MIN_COORDINATE;        // MaxY
    pvGameEmpireData[GameEmpireData::iNextMaintenance] = 0;          // NextMaintenance
    pvGameEmpireData[GameEmpireData::iNextFuelUse] = 0;              // NextFuelUse
    pvGameEmpireData[GameEmpireData::iNextTotalPop] = 0;             // iNextTotalPop
    pvGameEmpireData[GameEmpireData::iNextMin] = 0;                  // NextMin
    pvGameEmpireData[GameEmpireData::iNextFuel] = 0;                 // NextFuel
    pvGameEmpireData[GameEmpireData::iLastMessageTargetMask] = 0;

    // Select default message target
    iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::DefaultMessageTarget, pvGameEmpireData + GameEmpireData::iDefaultMessageTarget);
    RETURN_ON_ERROR(iErrCode);

    switch (pvGameEmpireData[GameEmpireData::iDefaultMessageTarget].GetInteger()) {

    case MESSAGE_TARGET_TRUCE:

        if (!(vDiplomacyLevel.GetInteger() & TRUCE))
        {
            pvGameEmpireData[GameEmpireData::iDefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;

    case MESSAGE_TARGET_TRADE:

        if (!(vDiplomacyLevel.GetInteger() & TRADE))
        {
            pvGameEmpireData[GameEmpireData::iDefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;

    case MESSAGE_TARGET_ALLIANCE:

        if (!(vDiplomacyLevel.GetInteger() & ALLIANCE))
        {
            pvGameEmpireData[GameEmpireData::iDefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;
    }

    // Get default number of saved game messages
    iErrCode = t_pCache->ReadData(
        SYSTEM_DATA, 
        SystemData::DefaultMaxNumGameMessages, 
        &pvGameEmpireData[GameEmpireData::iMaxNumGameMessages]
        );

    RETURN_ON_ERROR(iErrCode);

    // Grab default game options from empire's options
    iDefaultOptions = iEmpireOptions & (
        AUTO_REFRESH | COUNTDOWN | GAME_REPEATED_BUTTONS | MAP_COLORING | SHIP_MAP_COLORING |
        SHIP_MAP_HIGHLIGHTING | SENSITIVE_MAPS | PARTIAL_MAPS | SHIPS_ON_MAP_SCREEN | 
        SHIPS_ON_PLANETS_SCREEN | LOCAL_MAPS_IN_UPCLOSE_VIEWS | GAME_DISPLAY_TIME | 
        REJECT_INDEPENDENT_SHIP_GIFTS | DISPLACE_ENDTURN_BUTTON | BUILD_ON_MAP_SCREEN |
        BUILD_ON_PLANETS_SCREEN
        );

    // Request pause if necessary
    if (iSystemOptions & PAUSE_GAMES_BY_DEFAULT) {
        iDefaultOptions |= REQUEST_PAUSE;
    }

    pvGameEmpireData[GameEmpireData::iOptions] = iDefaultOptions;

    // Blank notepad
    pvGameEmpireData[GameEmpireData::iNotepad] = "";

    // Default builder planet is empire's default
    iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::DefaultBuilderPlanet, pvGameEmpireData + GameEmpireData::iDefaultBuilderPlanet);
    RETURN_ON_ERROR(iErrCode);

    pvGameEmpireData[GameEmpireData::iLastBuilderPlanet] = NO_KEY;
    pvGameEmpireData[GameEmpireData::iMaxEcon] = 0;
    pvGameEmpireData[GameEmpireData::iMaxMil] = 0;
    pvGameEmpireData[GameEmpireData::iNumNukedAllies] = 0;

    if (vGameOptions.GetInteger() & GAME_COUNT_FOR_BRIDIER) {

        int iRank, iIndex;

        iErrCode = GetBridierScore (iEmpireKey, &iRank, &iIndex);
        RETURN_ON_ERROR(iErrCode);

        pvGameEmpireData [GameEmpireData::iInitialBridierRank] = iRank;
        pvGameEmpireData [GameEmpireData::iInitialBridierIndex] = iIndex;

    } else {

        pvGameEmpireData [GameEmpireData::iInitialBridierRank] = 0;
        pvGameEmpireData [GameEmpireData::iInitialBridierIndex] = 0;
    }

    iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::GameRatios, pvGameEmpireData + GameEmpireData::iGameRatios);
    RETURN_ON_ERROR(iErrCode);

    pvGameEmpireData[GameEmpireData::iMiniMaps] = MINIMAPS_NEVER;
    pvGameEmpireData[GameEmpireData::iMapFairnessResourcesClaimed] = 0;

    ///////////////////////////////
    // Insert GameEmpireData row //
    ///////////////////////////////

    iErrCode = t_pCache->CreateEmpty(GAME_EMPIRE_DATA, strGameEmpireData);
    if (iErrCode != OK && iErrCode != ERROR_TABLE_ALREADY_EXISTS)
    {
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->InsertRow(strGameEmpireData, GameEmpireData::Template, pvGameEmpireData, NULL);
    RETURN_ON_ERROR(iErrCode);

    //////////////////////////////////////
    // Ensure certain tables are cached //
    //////////////////////////////////////

    iErrCode = t_pCache->CreateEmpty(GAME_EMPIRE_DATA, strGameEmpireData);
    if (iErrCode != OK && iErrCode != ERROR_TABLE_ALREADY_EXISTS)
    {
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->CreateEmpty(GAME_EMPIRE_MAP, strGameEmpireMap);
    if (iErrCode != OK && iErrCode != ERROR_TABLE_ALREADY_EXISTS)
    {
        RETURN_ON_ERROR(iErrCode);
    }

    // Send empire welcome message
    iErrCode = t_pCache->ReadData(strGameData, GameData::EnterGameMessage, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    if (!String::IsBlank (vTemp.GetCharPtr()))
    {
        Variant vCreatorName;
        const char* pszMessage;

        iErrCode = t_pCache->ReadData(strGameData, GameData::CreatorName, &vCreatorName);
        RETURN_ON_ERROR(iErrCode);

        if (String::IsBlank (vCreatorName.GetCharPtr()))
        {
            // System game
            pszMessage = vTemp.GetCharPtr();
        }
        else
        {
            // Someone's game
            pszMessage = (char*)StackAlloc(sizeof(char) * (MAX_EMPIRE_NAME_LENGTH + strlen (vTemp.GetCharPtr()) + 128));;

            sprintf (
                (char*) pszMessage,
                "%s, the creator of this game, says the following:" NEW_PARAGRAPH "%s",
                vCreatorName.GetCharPtr(),
                vTemp.GetCharPtr()
                );
        }

        iErrCode = SendGameMessage (iGameClass, iGameNumber, iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM, NULL_TIME);
        RETURN_ON_ERROR(iErrCode);
    }

    /////////////////////////
    // Game starting stuff //
    /////////////////////////

    // Has game started?    
    if (iGameState & STARTED)
    {
        // Add new empire to map if the map has been generated already
        if (iGameState & GAME_MAP_GENERATED)
        {
            iErrCode = AddEmpiresToMap(iGameClass, iGameNumber, &iEmpireKey, 1, gfoFairness);
            RETURN_ON_ERROR(iErrCode);
        }
    }
    else
    {
        // Are we the trigger for the game to begin?
        unsigned int iNumActiveEmpires;
        Variant vMinNumEmpires;

        iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumActiveEmpires);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MinNumEmpires, 
            &vMinNumEmpires
            );
        RETURN_ON_ERROR(iErrCode);
        
        if ((int) iNumActiveEmpires == vMinNumEmpires.GetInteger())
        {
            bStarted = true;
            
            // Start game
            iErrCode = t_pCache->WriteOr(strGameData, GameData::State, STARTED);
            RETURN_ON_ERROR(iErrCode);

            iGameState |= STARTED;
            
            // Set last update time
            iErrCode = t_pCache->WriteData(strGameData, GameData::LastUpdateTime, tTime);
            RETURN_ON_ERROR(iErrCode);
            
            // Set number of updates to zero
            iErrCode = t_pCache->WriteData(strGameData, GameData::NumUpdates, 0);
            RETURN_ON_ERROR(iErrCode);

            // Set max num empires
            iErrCode = t_pCache->WriteData(strGameData, GameData::MaxNumEmpires, (int)iCurrentNumEmpires);
            RETURN_ON_ERROR(iErrCode);

            // Set paused if necessary
            if (iSystemOptions & PAUSE_GAMES_BY_DEFAULT)
            {

                bPaused = true;

                iErrCode = t_pCache->WriteData(strGameData, GameData::NumRequestingPause, (int)iCurrentNumEmpires);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = PauseGame (iGameClass, iGameNumber, false, false);
                RETURN_ON_ERROR(iErrCode);
            }
            
            // Add all players to map if the game isn't configured to do so on the first update
            if (!(vGameClassOptions.GetInteger() & GENERATE_MAP_FIRST_UPDATE)) {
                bGenerateMapForAllEmpires = true;
            }

            char pszUpdateReport [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
            sprintf(pszUpdateReport, "%s %i has started", pszGameClassName, iGameNumber);
            global.WriteReport(TRACE_INFO, pszUpdateReport);
        }
    }

    if (bGenerateMapForAllEmpires)
    {
        GET_GAME_MAP(strGameMap, iGameClass, iGameNumber);
        iErrCode = t_pCache->CreateEmpty(GAME_MAP, strGameMap);
        if (iErrCode == ERROR_TABLE_ALREADY_EXISTS)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);

        int* piEmpKey;
        unsigned int iNumKeys;

        if (pgoGameOptions != NULL && pgoGameOptions->iNumPrearrangedTeams > 0)
        {
            iNumKeys = pgoGameOptions->iNumEmpires;
            piEmpKey = (int*)StackAlloc(iNumKeys * sizeof(int));

            // Randomize team order
            const unsigned int iNumTeams = pgoGameOptions->iNumPrearrangedTeams;
            PrearrangedTeam* paRandomTeams = (PrearrangedTeam*)StackAlloc(iNumTeams * sizeof(PrearrangedTeam));

            memcpy(paRandomTeams, pgoGameOptions->paPrearrangedTeam, iNumTeams * sizeof(PrearrangedTeam));
            Algorithm::Randomize<PrearrangedTeam>(paRandomTeams, iNumTeams);

            unsigned int iIndex = 0;
            for (unsigned int i = 0; i < pgoGameOptions->iNumPrearrangedTeams; i ++)
            {
                const PrearrangedTeam& team = paRandomTeams[i];
                for (unsigned int j = 0; j < team.iNumEmpires; j ++)
                {
                    piEmpKey[iIndex ++] = team.piEmpireKey[j];
                }
                // Randomize team members
                Algorithm::Randomize<int>(piEmpKey + iIndex - team.iNumEmpires, team.iNumEmpires);
            }
        }
        else
        {

            Variant* pvKey = NULL;
            AutoFreeData freeKeys(pvKey);
            iErrCode = t_pCache->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, NULL, &pvKey, &iNumKeys);
            RETURN_ON_ERROR(iErrCode);
            
            piEmpKey = (int*)StackAlloc(iNumKeys * sizeof(int));
            for (i = 0; i < iNumKeys; i ++)
            {
                piEmpKey[i] = pvKey[i].GetInteger();
            }

            // Randomize empires on the map
            Algorithm::Randomize<int>(piEmpKey, iNumKeys);
        }

        // Add empires to map
        iErrCode = AddEmpiresToMap(iGameClass, iGameNumber, piEmpKey, iNumKeys, gfoFairness);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteOr(strGameData, GameData::State, GAME_MAP_GENERATED);
        RETURN_ON_ERROR(iErrCode);
        
        iGameState |= GAME_MAP_GENERATED;
    }

    // Send some messages
    if (iCurrentNumEmpires > 1)
    {
        unsigned int iNumEmpires;
        Variant* pvEmpireKey = NULL;
        AutoFreeData freeEmpKeys(pvEmpireKey);
        iErrCode = t_pCache->ReadColumn (
            strGameEmpires, 
            GameEmpires::EmpireKey, 
            NULL,
            &pvEmpireKey, 
            &iNumEmpires
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        // Insert rows into dip tables if DipExposed is true
        if (vGameClassOptions.GetInteger() & EXPOSED_DIPLOMACY)
        {
            iErrCode = t_pCache->CreateEmpty(GAME_EMPIRE_DIPLOMACY, strGameEmpireDiplomacy);
            RETURN_ON_ERROR(iErrCode);

            Variant pvDiplomacy[GameEmpireDiplomacy::NumColumns] =
            {
                iGameClass,
                iGameNumber,
                iEmpireKey,
                NO_KEY,
                WAR,
                WAR,
                WAR,
                0,
                0,
                0,
                0,
            };

            char pszDiplomacy [256];

            for (i = 0; i < iNumEmpires; i ++)
            {
                if (pvEmpireKey[i].GetInteger() != iEmpireKey)
                {
                    pvDiplomacy[GameEmpireDiplomacy::iEmpireKey] = iEmpireKey;
                    pvDiplomacy[GameEmpireDiplomacy::iReferenceEmpireKey] = pvEmpireKey[i];
                    
                    // Insert iterated player into empire's table
                    iErrCode = t_pCache->InsertRow(strGameEmpireDiplomacy, GameEmpireDiplomacy::Template, pvDiplomacy, &iKey);
                    RETURN_ON_ERROR(iErrCode);
                    
                    // Insert empire into iterated player's table
                    COPY_GAME_EMPIRE_DIPLOMACY (pszDiplomacy, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
                    pvDiplomacy[GameEmpireDiplomacy::iEmpireKey] = pvEmpireKey[i].GetInteger();
                    pvDiplomacy[GameEmpireDiplomacy::iReferenceEmpireKey] = iEmpireKey;

                    iErrCode = t_pCache->InsertRow(pszDiplomacy, GameEmpireDiplomacy::Template, pvDiplomacy, &iKey);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }

        if (bSendMessages)
        {
            // Broadcast player's entry, if more than one player in game
            if (vGameOptions.GetInteger() & GAME_NAMES_LISTED) {
                strMessage = BEGIN_STRONG;
                strMessage += vEmpireName.GetCharPtr();
                strMessage += END_STRONG " has joined the game";
            } else {
                strMessage = "A new empire has joined the game";
            }
            Assert(strMessage.GetCharPtr());

            if (!strDuplicateIPList.IsBlank()) {
                strMessage += "\n";
                strMessage += strDuplicateIPList;
            }

            if (!strDuplicateIdList.IsBlank()) {
                strMessage += "\n";
                strMessage += strDuplicateIdList;
            }

            if (bStarted) {
                strMessage += "\nThe game has " BEGIN_STRONG "started" END_STRONG;
            }
            
            if (bClosed) {
                strMessage += "\nThe game has " BEGIN_STRONG "closed" END_STRONG;
            }

            if (bUnPaused) {
                strMessage += "\nThe game was " BEGIN_STRONG "unpaused" END_STRONG;
            }

            if (bPaused) {
                strMessage += "\nThe game is " BEGIN_STRONG "paused" END_STRONG;
            }
            
            for (i = 0; i < iNumEmpires; i ++)
            {
                if (pvEmpireKey[i].GetInteger() != iEmpireKey)
                {
                    iErrCode = SendGameMessage (
                        iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), strMessage.GetCharPtr(), 
                        SYSTEM, MESSAGE_SYSTEM | MESSAGE_BROADCAST, NULL_TIME);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }
    }

    // Get num updates
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vNumUpdates);
    RETURN_ON_ERROR(iErrCode);

    *piNumUpdates = vNumUpdates.GetInteger();
    
    // We're done adding the empire
    char pszUpdateReport [128 + MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    sprintf (
        pszUpdateReport,
        "%s entered %s %i",
        vEmpireName.GetCharPtr(),
        pszGameClassName,
        iGameNumber
        );

    global.WriteReport(TRACE_INFO, pszUpdateReport);

    return iErrCode;
}


int GameEngine::SetEnterGameIPAddress (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszIPAddress)
{
    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    return t_pCache->WriteData(strGameEmpireData, GameEmpireData::EnterGameIPAddress, pszIPAddress);
}

int GameEngine::IsGamePasswordCorrect(int iGameClass, int iGameNumber, const char* pszPassword)
{
    int iErrCode;
    GET_GAME_DATA(strGameData, iGameClass, iGameNumber);

    Variant vActualPasswordHash;
    iErrCode = t_pCache->ReadData(strGameData, GameData::PasswordHash, &vActualPasswordHash);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAME_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    Variant vTestPasswordHash = (const char*)NULL;
    if (!String::IsBlank(pszPassword))
    {
        iErrCode = ComputePasswordHash(pszPassword, &vTestPasswordHash);
        RETURN_ON_ERROR(iErrCode);
    }

    if (String::StrCmp(vActualPasswordHash, vTestPasswordHash) != 0)
    {
        return ERROR_PASSWORD;
    }
    return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *pbExist -> true iff game exists
//
// Determine if a certain game is the active game list

int GameEngine::DoesGameExist(int iGameClass, int iGameNumber, bool* pbExist)
{
    const char* ppszColumns[] = { SystemActiveGames::GameClass, SystemActiveGames::GameNumber };
    const Variant pvData[] = { iGameClass, iGameNumber };

    unsigned int iNumKeys;
    int iErrCode = t_pCache->GetEqualKeys(SYSTEM_ACTIVE_GAMES, ppszColumns, pvData, countof(ppszColumns), NULL, &iNumKeys);
    if (iErrCode == OK)
    {
        *pbExist = true;
    }
    else if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        *pbExist = false;
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
    }
    return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *piNumUpdates -> Number of updates in a game

int GameEngine::GetNumUpdates (int iGameClass, int iGameNumber, int* piNumUpdates) {

    int iErrCode;

    Variant vNumUpdates;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(pszGameData, GameData::NumUpdates, &vNumUpdates);
    RETURN_ON_ERROR(iErrCode);

    *piNumUpdates = vNumUpdates.GetInteger();

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *piNumUpdates -> Number of updates before game closes

int GameEngine::GetNumUpdatesBeforeGameCloses (int iGameClass, int iGameNumber, int* piNumUpdates) {

    int iErrCode;

    Variant vNumUpdates;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(pszGameData, GameData::NumUpdatesBeforeGameCloses, &vNumUpdates);
    RETURN_ON_ERROR(iErrCode);

    *piNumUpdates = vNumUpdates.GetInteger();
    
    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *piOptions -> Game option mask

int GameEngine::GetGameOptions (int iGameClass, int iGameNumber, int* piOptions) {

    int iErrCode;

    Variant vValue;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(pszGameData, GameData::Options, &vValue);
    RETURN_ON_ERROR(iErrCode);

    *piOptions = vValue.GetInteger();
    
    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *psDelay -> First update delay

int GameEngine::GetFirstUpdateDelay (int iGameClass, int iGameNumber, Seconds* psDelay) {

    int iErrCode;

    Variant vValue;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(pszGameData, GameData::FirstUpdateDelay, &vValue);
    RETURN_ON_ERROR(iErrCode);

    *psDelay = vValue.GetInteger();
    
    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber - > Gamenumber
//
// Output:
// *piNumEmpires -> Number of empires in game
//
// Return the number of empires still remaining a given game

int GameEngine::GetNumEmpiresInGame (int iGameClass, int iGameNumber, unsigned int* piNumEmpires)
{
    GET_GAME_EMPIRES(strGameEmpires, iGameClass, iGameNumber);
    return t_pCache->GetNumCachedRows(strGameEmpires, piNumEmpires);
}


// Input:
// iGameClass -> Gameclass
// iGameNumber - > Gamenumber
//
// Output:
// *piNumDeadEmpires -> Number of dead empires in game
//
// Return the number of empires still remaining a given game

int GameEngine::GetNumDeadEmpiresInGame (int iGameClass, int iGameNumber, unsigned int* piNumDeadEmpires)
{
    GET_GAME_NUKED_EMPIRES(strGameDeadEmpires, iGameClass, iGameNumber);
    return t_pCache->GetNumCachedRows(strGameDeadEmpires, piNumDeadEmpires);
}


// Input:
// iGameClass -> Gameclass
//
// Output:
// *piNumEmpiresNeeded -> Number of empires needed
//
// Return the number of empires needed to start a game

int GameEngine::GetNumEmpiresNeededForGame (int iGameClass, int* piNumEmpiresNeeded) {

    Variant vNumEmpiresNeeded;
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MinNumEmpires,
        &vNumEmpiresNeeded
        );
    RETURN_ON_ERROR(iErrCode);

    *piNumEmpiresNeeded = vNumEmpiresNeeded.GetInteger();
    
    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber - > Gamenumber
//
// Output:
// *piUpdatedEmpires -> Number of empires who have hit end turn
//
// Returns the number of updated and idle empires in a game

int GameEngine::GetNumUpdatedEmpires (int iGameClass, int iGameNumber, int* piUpdatedEmpires) {

    Variant vTemp;
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(strGameData, GameData::NumEmpiresUpdated, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piUpdatedEmpires = vTemp.GetInteger();
    
    return iErrCode;
}


// Input:
// iGameClass -> GameClass
// iGameNumber -> GameNumber
//
// Output:
// **ppvEmpireName -> Array of names
// *piNumEmpires -> Number of names returned
//
// Return the names of the active empires who are in the given game                             

int GameEngine::GetEmpiresInGame(int iGameClass, int iGameNumber, Variant** ppvEmpireKey, unsigned int* piNumEmpires)
{
    GET_GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);
    
    int iErrCode = t_pCache->ReadColumn(pszEmpires, GameEmpires::EmpireKey, NULL, ppvEmpireKey, piNumEmpires);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetEmpiresInGame(int iGameClass, int iGameNumber, Variant*** pppvEmpiresInGame, unsigned int* piNumEmpires)
{
    GET_GAME_EMPIRES(pszEmpires, iGameClass, iGameNumber);

    const char* ppszColumns[] =
    {
        GameEmpires::GameClass, 
        GameEmpires::GameNumber, 
        GameEmpires::EmpireKey, 
        GameEmpires::EmpireName
    };

    int iErrCode = t_pCache->ReadColumns(pszEmpires, countof(ppszColumns), ppszColumns, NULL, pppvEmpiresInGame, piNumEmpires);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Input:
// iGameClass -> GameClass
// iGameNumber -> GameNumber
//
// Output:
// *pbPaused -> True if game is paused, false if not
//
// Return if the given game is paused by the players of the administrator

int GameEngine::IsGamePaused (int iGameClass, int iGameNumber, bool* pbPaused) {

    Variant vTemp;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *pbPaused = (vTemp.GetInteger() & PAUSED) || (vTemp.GetInteger() & ADMIN_PAUSED);
    
    return iErrCode;
}

// Input:
// iGameClass -> GameClass
// iGameNumber -> GameNumber
//
// Output:
// *pbAdminPaused -> True if game was paused by an admin, false if not
//
// Return if the given game was paused by an admin

int GameEngine::IsGameAdminPaused (int iGameClass, int iGameNumber, bool* pbAdminPaused) {

    Variant vTemp;
    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *pbAdminPaused = (vTemp.GetInteger() & ADMIN_PAUSED) != 0;
    
    return iErrCode;
}

void GameEngine::GetCoordinates(const char* pszCoord, int* piX, int* piY)
{
    int iNum = sscanf(pszCoord, "%i,%i", piX, piY);
    Assert(iNum == 2);
}

void GameEngine::GetCoordinates (int iX, int iY, char* pszCoord)
{
    sprintf(pszCoord, "%i,%i", iX, iY);
}

int GameEngine::CheckGameForEndConditions(int iGameClass, int iGameNumber, const char* pszAdminMessage, bool* pbEndGame)
{
    int iErrCode;
    unsigned int iEmpKey;
    Variant vEmpireKey;

    iErrCode = CheckGameForAllyOut(iGameClass, iGameNumber, pbEndGame);
    RETURN_ON_ERROR(iErrCode);

    if (*pbEndGame)
    {
        // Ally out
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
        char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];

        iErrCode = GetGameClassName (iGameClass, pszGameClassName);
        RETURN_ON_ERROR(iErrCode);

        GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
        
        // Send victory message to all remaining empires
        sprintf (
            pszMessage, 
            "Congratulations! You have won %s %i", 
            pszGameClassName, 
            iGameNumber
            );
        
        iEmpKey = NO_KEY;
        while (true)
        {
            iErrCode = t_pCache->GetNextKey(strGameEmpires, iEmpKey, &iEmpKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
            RETURN_ON_ERROR(iErrCode);
            
            // Best effort send messages
            if (!String::IsBlank (pszAdminMessage))
            {
                iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszAdminMessage, SYSTEM, MESSAGE_SYSTEM);
                RETURN_ON_ERROR(iErrCode);
            }
            
            iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
            RETURN_ON_ERROR(iErrCode);
            
            // Best effort update empires' statistics
            iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, vEmpireKey.GetInteger());
            RETURN_ON_ERROR(iErrCode);
        }

        // Best effort cleanup game
        iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_WIN);
        RETURN_ON_ERROR(iErrCode);

        return iErrCode;
    }

    iErrCode = CheckGameForDrawOut(iGameClass, iGameNumber, pbEndGame);
    RETURN_ON_ERROR(iErrCode);
    
    if (*pbEndGame)
    {
        // Draw out
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
        char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];

        iErrCode = GetGameClassName (iGameClass, pszGameClassName);
        RETURN_ON_ERROR(iErrCode);

        GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

        // Send draw message to all remaining empires
        sprintf(pszMessage, "You have drawn %s %i", pszGameClassName, iGameNumber);

        iEmpKey = NO_KEY;
        while (true)
        {
            iErrCode = t_pCache->GetNextKey (strGameEmpires, iEmpKey, &iEmpKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            iErrCode = t_pCache->ReadData(strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
            RETURN_ON_ERROR(iErrCode);
            
            // Best effort send messages
            if (!String::IsBlank (pszAdminMessage))
            {
                iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszAdminMessage, SYSTEM, MESSAGE_SYSTEM);
                RETURN_ON_ERROR(iErrCode);
            }
            
            iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
            RETURN_ON_ERROR(iErrCode);

            // Best effort update empires' statistics
            iErrCode = UpdateScoresOnDraw (iGameClass, iGameNumber, vEmpireKey.GetInteger());
            RETURN_ON_ERROR(iErrCode);
        }
        
        // Best effort cleanup game
        iErrCode = CleanupGame(iGameClass, iGameNumber, GAME_RESULT_DRAW);
        RETURN_ON_ERROR(iErrCode);
        
        return iErrCode;
    }

    return iErrCode;
}


int GameEngine::CheckGameForAllyOut (int iGameClass, int iGameNumber, bool* pbAlly) {

    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    int iErrCode;
    unsigned int iNumRows, iNumEmpires, iKey, iEmpKey;
    Variant vEmpireKey, vDipStatus;

    bool bAlly = true;

    iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    iEmpKey = NO_KEY;
    while (true)
    {
        iErrCode = t_pCache->GetNextKey (strGameEmpires, iEmpKey, &iEmpKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        RETURN_ON_ERROR(iErrCode);
        
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
        RETURN_ON_ERROR(iErrCode);
        
        GET_GAME_EMPIRE_DIPLOMACY(pszDiplomacy, iGameClass, iGameNumber, vEmpireKey.GetInteger());
        iErrCode = t_pCache->GetNumCachedRows(pszDiplomacy, &iNumRows);
        RETURN_ON_ERROR(iErrCode);

        if (iNumRows != iNumEmpires - 1)
        {
            bAlly = false;
            break;
        }

        iKey = NO_KEY;
        while (true)
        {
            iErrCode = t_pCache->GetNextKey (pszDiplomacy, iKey, &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(pszDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
            RETURN_ON_ERROR(iErrCode);

            if (vDipStatus.GetInteger() != ALLIANCE)
            {
                bAlly = false;
                break;
            }
        }

        if (!bAlly)
        {
            break;
        }
    }

    *pbAlly = bAlly;

    return iErrCode;
}

    
int GameEngine::CheckGameForDrawOut (int iGameClass, int iGameNumber, bool* pbDraw) {

    int iErrCode;
    unsigned int iNumEmpires;
    Variant vTemp;

    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    *pbDraw = false;

    unsigned int iRequesting;
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumRequestingDraw, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iRequesting = vTemp.GetInteger();

    iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    Assert(iNumEmpires > 0);
    Assert(iRequesting <= iNumEmpires);
    if (iRequesting == iNumEmpires)
    {
        // Draw out iff someone isn't idle
        bool bIdle;
        iErrCode = AreAllEmpiresIdle (iGameClass, iGameNumber, &bIdle);
        RETURN_ON_ERROR(iErrCode);

        *pbDraw = !bIdle;
    }

    return iErrCode;
}

int GameEngine::AreAllEmpiresIdle (int iGameClass, int iGameNumber, bool* pbIdle) {

    int iErrCode;
    Variant vTemp;

    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    *pbIdle = true;

    // Figure out idle policy
    iErrCode = GetGameClassProperty (iGameClass, SystemGameClassData::NumUpdatesForIdle, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    unsigned int iNumUpdatesForIdle = vTemp.GetInteger();

    unsigned int iKey = NO_KEY;
    while (true)
    {
        iErrCode = t_pCache->GetNextKey (strGameEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strGameEmpires, iKey, GameEmpires::EmpireKey, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        int iEmpireKey = vTemp.GetInteger();

        int iEmpireOptions;
        iErrCode = GetEmpireOptions (iGameClass, iGameNumber, iEmpireKey, &iEmpireOptions);
        RETURN_ON_ERROR(iErrCode);

        if (iEmpireOptions & RESIGNED)
        {
            continue;
        }

        char pszGameEmpireData [256];
        COPY_GAME_EMPIRE_DATA (pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

        iErrCode = t_pCache->ReadData(pszGameEmpireData, GameEmpireData::NumUpdatesIdle, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        unsigned int iNumUpdatesIdle = vTemp.GetInteger();

        if (iNumUpdatesIdle >= iNumUpdatesForIdle)
        {
            continue;
        }

        // A non-idle empire has been found
        *pbIdle = false;
        break;
    }

    return iErrCode;
}

int GameEngine::PauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) {

    // Get the time
    UTCTime tNow;
    Time::GetTime (&tNow);

    return PauseGameInternal (iGameClass, iGameNumber, tNow, bAdmin, bBroadcast);
}

int GameEngine::PauseGameAt (int iGameClass, int iGameNumber, const UTCTime& tNow) {

    return PauseGameInternal (iGameClass, iGameNumber, tNow, false, false);
}

int GameEngine::PauseGameInternal (int iGameClass, int iGameNumber, const UTCTime& tNow, bool bAdmin, bool bBroadcast)
{
    Variant vTemp;
    int iErrCode, iState;

    bool bFlag;
    const char* pszMessage;

    ICachedTable* pGameData = NULL;
    AutoRelease<ICachedTable> release(pGameData);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Make sure game exists
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
    RETURN_ON_ERROR(iErrCode);
    if (!bFlag)
    {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    // Get update period
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    Seconds sUpdatePeriod = vTemp.GetInteger();

    int iGameClassOptions;
    iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
    RETURN_ON_ERROR(iErrCode);

    // Can only pause games that have started
    iErrCode = GetGameState (iGameClass, iGameNumber, &iState);
    RETURN_ON_ERROR(iErrCode);

    if (!(iState & STARTED))
    {
        return OK;
    }

    iErrCode = t_pCache->GetTable(strGameData, &pGameData);
    RETURN_ON_ERROR(iErrCode);

    // Read the state again - it's cheap, and the state may have changed
    iErrCode = pGameData->ReadData(GameData::State, &iState);
    RETURN_ON_ERROR(iErrCode);
    Assert(iState & STARTED);

    if (bAdmin && !(iState & ADMIN_PAUSED))
    {
        iErrCode = pGameData->WriteOr(GameData::State, ADMIN_PAUSED);
        RETURN_ON_ERROR(iErrCode);
    }

    // Might already be paused
    if (iState & PAUSED)
    {
        return OK;
    }

    int iNumUpdates;
    iErrCode = pGameData->ReadData(GameData::NumUpdates, &iNumUpdates);
    RETURN_ON_ERROR(iErrCode);

    Seconds sFirstUpdateDelay = 0;
    if (iNumUpdates == 0)
    {
        int iTemp;
        iErrCode = pGameData->ReadData(GameData::FirstUpdateDelay, &iTemp);
        RETURN_ON_ERROR(iErrCode);
        sFirstUpdateDelay = iTemp;
    }

    UTCTime tLastUpdateTime;
    iErrCode = pGameData->ReadData(GameData::LastUpdateTime, &tLastUpdateTime);
    RETURN_ON_ERROR(iErrCode);

    bool bWeekends = (iGameClassOptions & WEEKEND_UPDATES) != 0;
    Seconds sAfterWeekendDelay = 0;
    if (!bWeekends)
    {
        iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::AfterWeekendDelay, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        sAfterWeekendDelay = vTemp.GetInteger();
    }

    //
    // Pause the game
    //

    iErrCode = pGameData->WriteOr(GameData::State, PAUSED);
    RETURN_ON_ERROR(iErrCode);

    UTCTime tNextUpdateTime;
    GetNextUpdateTime (
        tLastUpdateTime,
        sUpdatePeriod,
        iNumUpdates,
        sFirstUpdateDelay,
        sAfterWeekendDelay,
        bWeekends,
        &tNextUpdateTime
        );

    // Note - sSecondsUntil can be negative
    Seconds sSecondsUntil = Time::GetSecondDifference (tNextUpdateTime, tNow);

    // Write down seconds until next update
    iErrCode = pGameData->WriteData(GameData::SecondsUntilNextUpdateWhilePaused, sSecondsUntil);
    RETURN_ON_ERROR(iErrCode);

    // Broadcast message
    if (bBroadcast)
    {
        pszMessage = bAdmin ? "The game was paused by an administrator" : "The game is now paused";

        iErrCode = BroadcastGameMessage(iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::UnpauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) {

    int iErrCode, iTemp;
    const char* pszMessage;
    Variant vTemp;

    ICachedTable* pGameData = NULL;
    AutoRelease<ICachedTable> release(pGameData);

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    // Get gameclass update options
    int iGameClassOptions;
    iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
    RETURN_ON_ERROR(iErrCode);

    // Assumption - the number of empires in the game can't change while we're here
    // This is because we only call this while holding a read lock on the game
    unsigned int iNumEmpires;
    iErrCode = t_pCache->GetNumCachedRows(strEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    // Get update period
    Seconds sUpdatePeriod = 0;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    sUpdatePeriod = vTemp.GetInteger();

    iErrCode = t_pCache->GetTable(strGameData, &pGameData);
    RETURN_ON_ERROR(iErrCode);

    int iState;
    iErrCode = pGameData->ReadData(GameData::State, &iState);
    RETURN_ON_ERROR(iErrCode);

    if (!(iState & PAUSED))
    {
        return OK;
    }

    if (bAdmin)
    {
        if (!(iState & ADMIN_PAUSED))
        {
            return OK;
        }

        unsigned int iNumRequestingPause;
        iErrCode = pGameData->ReadData(GameData::NumRequestingPause, (int*) &iNumRequestingPause);
        RETURN_ON_ERROR(iErrCode);

        // Try to unpause the game
        iErrCode = pGameData->WriteAnd(GameData::State, ~ADMIN_PAUSED);
        RETURN_ON_ERROR(iErrCode);

        if (iNumEmpires == iNumRequestingPause)
        {
            // Still paused
            iErrCode = pGameData->WriteOr(GameData::State, PAUSED);
            RETURN_ON_ERROR(iErrCode);

            return iErrCode;
        }

        // Fall through to unpause
        pszMessage = "The game was unpaused by an administrator";
    }
    else
    {
        pszMessage = "The game is no longer paused";
    }

    // Get num updates
    int iNumUpdates;
    iErrCode = pGameData->ReadData(GameData::NumUpdates, &iNumUpdates);
    RETURN_ON_ERROR(iErrCode);

    // Get first update delay
    Seconds sFirstUpdateDelay = 0;
    if (iNumUpdates == 0) {

        iErrCode = pGameData->ReadData(GameData::FirstUpdateDelay, &iTemp);
        RETURN_ON_ERROR(iErrCode);
        sFirstUpdateDelay = iTemp;
    }

    // Get seconds until next update when game was paused
    Seconds sSecondsUntilNextUpdate;

    iErrCode = pGameData->ReadData(GameData::SecondsUntilNextUpdateWhilePaused, &iTemp);
    RETURN_ON_ERROR(iErrCode);
    sSecondsUntilNextUpdate = iTemp;

    UTCTime tNow, tNewLastUpdateTime;
    Time::GetTime (&tNow);

    GetLastUpdateTimeForPausedGame (
        tNow,
        sSecondsUntilNextUpdate,
        sUpdatePeriod,
        iNumUpdates,
        sFirstUpdateDelay,
        &tNewLastUpdateTime
        );

    iErrCode = pGameData->WriteData(GameData::LastUpdateTime, tNewLastUpdateTime);
    RETURN_ON_ERROR(iErrCode);

    // No longer paused
    iErrCode = pGameData->WriteAnd(GameData::State, ~PAUSED);
    RETURN_ON_ERROR(iErrCode);

    // Broadcast message
    if (bBroadcast)
    {
        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::PauseAllGames()
{
    int iErrCode;

    Variant** ppvActiveGame = NULL;
    AutoFreeData free_ppvActiveGame(ppvActiveGame);
    unsigned int iNumGames;

    iErrCode = GetActiveGames(&ppvActiveGame, &iNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = CacheGameData((const Variant**)ppvActiveGame, NO_KEY, iNumGames);
        RETURN_ON_ERROR(iErrCode);

        for (unsigned int i = 0; i < iNumGames; i ++)
        {
            int iGameClass = ppvActiveGame[i][0].GetInteger();
            int iGameNumber = ppvActiveGame[i][1].GetInteger();

            // Flush remaining updates
            bool bUpdated;
            iErrCode = CheckGameForUpdates(iGameClass, iGameNumber, &bUpdated);
            RETURN_ON_ERROR(iErrCode);

            // Pause the game
            bool bExists = true;
            if (bUpdated)
            {
                iErrCode = DoesGameExist(iGameClass, iGameNumber, &bExists);
                RETURN_ON_ERROR(iErrCode);
            }

            if (bExists)
            {
                iErrCode = PauseGame (iGameClass, iGameNumber, true, true);
                RETURN_ON_ERROR(iErrCode);
            }
        }    
    }

    return iErrCode;
}

int GameEngine::UnpauseAllGames()
{
    int iErrCode;

    Variant** ppvActiveGame = NULL;
    AutoFreeData free_ppvActiveGame(ppvActiveGame);

    unsigned int iNumGames;
    iErrCode = GetActiveGames(&ppvActiveGame, &iNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        iErrCode = CacheGameData((const Variant**)ppvActiveGame, NO_KEY, iNumGames);
        RETURN_ON_ERROR(iErrCode);

        for (unsigned int i = 0; i < iNumGames; i ++)
        {
            int iGameClass = ppvActiveGame[i][0].GetInteger();
            int iGameNumber = ppvActiveGame[i][1].GetInteger();

            iErrCode = UnpauseGame(iGameClass, iGameNumber, true, true);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

int GameEngine::LogEmpireIntoGame (int iGameClass, int iGameNumber, int iEmpireKey, int* piIdleUpdates) {

    int iErrCode;

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> release(pTable);
    
    // Update lastLogin, logged in this update (if necessary)
    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->GetTable(strGameEmpireData, &pTable);
    RETURN_ON_ERROR(iErrCode);

    Variant vLastLogin;
    iErrCode = pTable->ReadData(GameEmpireData::LastLogin, &vLastLogin);
    RETURN_ON_ERROR(iErrCode);

    UTCTime tTime;
    Time::GetTime(&tTime);

    if (Time::GetSecondDifference(tTime, vLastLogin.GetInteger64()) >= LAST_LOGIN_PRECISION_IN_SECONDS)
    {
        // Set last login
        iErrCode = pTable->WriteData(GameEmpireData::LastLogin, tTime);
        RETURN_ON_ERROR(iErrCode);
    }

    Variant vOptions;
    iErrCode = pTable->ReadData(GameEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    if (!(vOptions.GetInteger() & LOGGED_IN_THIS_UPDATE))
    {
        // Set logged in this update
        iErrCode = pTable->WriteOr(GameEmpireData::Options, LOGGED_IN_THIS_UPDATE);
        RETURN_ON_ERROR(iErrCode);
    }

    // Read idle updates
    iErrCode = pTable->ReadData(GameEmpireData::NumUpdatesIdle, piIdleUpdates);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::RuinGame (int iGameClass, int iGameNumber, const char* pszWinnerName) {

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    int iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
    {
        return iErrCode;
    }
    RETURN_ON_ERROR(iErrCode);

    // Prepare ruin message for all remaining empires
    char pszMessage[128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    sprintf(pszMessage, "You ruined out of %s %i", pszGameClassName, iGameNumber);

    // Get empires
    Variant* pvEmpireKey = NULL;
    AutoFreeData free(pvEmpireKey);
    unsigned int i, iNumEmpires;

    GET_GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadColumn (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        NULL,
        &pvEmpireKey, 
        &iNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumEmpires; i ++)
    {
        iErrCode = RuinEmpire (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), pszMessage);
        RETURN_ON_ERROR(iErrCode);
    }

    for (i = 0; i < iNumEmpires; i ++)
    {
        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), EMPIRE_RUINED, NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    // Kill the game
    iErrCode = CleanupGame(iGameClass, iGameNumber, pszWinnerName == NULL ? GAME_RESULT_RUIN : GAME_RESULT_WIN, pszWinnerName);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::ResignGame(int iGameClass, int iGameNumber)
{
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    int iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
    {
        return iErrCode;
    }
    RETURN_ON_ERROR(iErrCode);

    // Prepare resignation message for all remaining empires
    char pszMessage[128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    sprintf(pszMessage, "You resigned out of %s %i", pszGameClassName, iGameNumber);

    // Get empires
    Variant* pvEmpireKey = NULL;
    AutoFreeData free(pvEmpireKey);

    unsigned int i, iNumEmpires;

    GET_GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadColumn (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        NULL,
        &pvEmpireKey, 
        &iNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumEmpires; i ++)
    {
        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), EMPIRE_RESIGNED, NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    // Kill the game
    iErrCode = CleanupGame(iGameClass, iGameNumber, GAME_RESULT_NONE, NULL);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetResignedEmpiresInGame (int iGameClass, int iGameNumber, unsigned int** ppiEmpireKey, unsigned int* piNumResigned)
{
    int iErrCode;
    bool bResigned;

    unsigned int i, iNumEmpires;

    *ppiEmpireKey = NULL;
    *piNumResigned = 0;

    GET_GAME_DATA(strGameData, iGameClass, iGameNumber);

    Variant vResigned;
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumEmpiresResigned, &vResigned);
    RETURN_ON_ERROR(iErrCode);

    if (vResigned.GetInteger() == 0)
    {
        return OK;
    }

    GET_GAME_EMPIRES(strGameEmpires, iGameClass, iGameNumber);

    // Get empires
    Variant* pvEmpireKey = NULL;
    AutoFreeData free(pvEmpireKey);

    iErrCode = t_pCache->ReadColumn (
        strGameEmpires,
        GameEmpires::EmpireKey,
        NULL,
        &pvEmpireKey,
        &iNumEmpires
        );

    RETURN_ON_ERROR(iErrCode);

    // Output
    unsigned int* piEmpireKey = new unsigned int[iNumEmpires];
    Assert(piEmpireKey);
    Algorithm::AutoDelete<unsigned int> free_piEmpireKey(piEmpireKey, true);

    unsigned int iNumResigned = 0;
    for (i = 0; i < iNumEmpires; i ++)
    {
        iErrCode = HasEmpireResignedFromGame (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), &bResigned);
        RETURN_ON_ERROR(iErrCode);
            
        if (bResigned)
        {
            piEmpireKey[iNumResigned ++] = pvEmpireKey[i].GetInteger();
        }
    }

    Assert((int)iNumResigned == vResigned.GetInteger());

    *ppiEmpireKey = piEmpireKey;
    piEmpireKey = NULL;
    *piNumResigned = iNumResigned;

    return iErrCode;
}

int GameEngine::GetBridierRankPotentialGainLoss (int iGameClass, int iGameNumber, int iEmpireKey, int* piGain, int* piLoss)
{
    int iErrCode, iEmpireRank = 0, iEmpireIndex = 0, iFoeRank = 0, iFoeIndex = 0, iTemp;
    unsigned int iNumEmpires;

    Variant v0Key, v1Key, vRank, vIndex;

    *piGain = 10;
    *piLoss = -10;

    GET_GAME_EMPIRES(pszGameEmpires, iGameClass, iGameNumber);

#ifdef _DEBUG

    int iOptions;
    iErrCode = GetGameOptions(iGameClass, iGameNumber, &iOptions);
    RETURN_ON_ERROR(iErrCode);
    Assert(iOptions & GAME_COUNT_FOR_BRIDIER);

#endif

    iErrCode = t_pCache->GetNumCachedRows(pszGameEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    Assert(iNumEmpires <= 2);

    unsigned int iFirstKey;
    iErrCode = t_pCache->GetNextKey(pszGameEmpires, NO_KEY, &iFirstKey);
    RETURN_ON_ERROR(iErrCode);

    // Read 1st empire
    iErrCode = t_pCache->ReadData(pszGameEmpires, iFirstKey, GameEmpires::EmpireKey, &v0Key);
    RETURN_ON_ERROR(iErrCode);

    if (iNumEmpires == 1 && v0Key.GetInteger() == iEmpireKey)
    {
        // We're alone in the game
        *piGain = -1;
        *piLoss = -1;
        return OK;
    }

    GET_GAME_EMPIRE_DATA(strGameEmpireData1, iGameClass, iGameNumber, v0Key.GetInteger());
    
    iErrCode = t_pCache->ReadData(strGameEmpireData1, GameEmpireData::InitialBridierRank, &vRank);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadData(strGameEmpireData1, GameEmpireData::InitialBridierIndex, &vIndex);
    RETURN_ON_ERROR(iErrCode);

    if (v0Key.GetInteger() == iEmpireKey) {

        iEmpireRank = vRank.GetInteger();
        iEmpireIndex = vIndex.GetInteger();
    
    } else {

        iFoeRank = vRank.GetInteger();
        iFoeIndex = vIndex.GetInteger();
    }

    // Read 2nd empire
    if (iNumEmpires == 2)
    {
        unsigned int iNextKey;
        iErrCode = t_pCache->GetNextKey(pszGameEmpires, iFirstKey, &iNextKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(pszGameEmpires, iNextKey, GameEmpires::EmpireKey, &v1Key);
        RETURN_ON_ERROR(iErrCode);

        if (v0Key.GetInteger() != iEmpireKey && v1Key.GetInteger() != iEmpireKey) {

            // Someone beat us into the game
            *piGain = -1;
            *piLoss = -1;
            return OK;
        }
        
        GET_GAME_EMPIRE_DATA (strGameEmpireData2, iGameClass, iGameNumber, v1Key.GetInteger());
        
        iErrCode = t_pCache->ReadData(strGameEmpireData2, GameEmpireData::InitialBridierRank, &vRank);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(strGameEmpireData2, GameEmpireData::InitialBridierIndex, &vIndex);
        RETURN_ON_ERROR(iErrCode);
        
        if (v1Key.GetInteger() == iEmpireKey) {
            
            iEmpireRank = vRank.GetInteger();
            iEmpireIndex = vIndex.GetInteger();
            
        } else {
            
            iFoeRank = vRank.GetInteger();
            iFoeIndex = vIndex.GetInteger();
        }
    
    } else {

        Assert(v0Key.GetInteger() != iEmpireKey);

        iErrCode = GetBridierScore (iEmpireKey, &iEmpireRank, &iEmpireIndex);
        RETURN_ON_ERROR(iErrCode);
    }

    BridierObject::GetScoreChanges (
        iEmpireRank, 
        iEmpireIndex, 
        iFoeRank, 
        iFoeIndex,
        piGain, 
        &iTemp, 
        &iTemp, 
        &iTemp
        );

    BridierObject::GetScoreChanges (
        iFoeRank, 
        iFoeIndex,
        iEmpireRank, 
        iEmpireIndex, 
        &iTemp, 
        &iTemp, 
        piLoss, 
        &iTemp
        );

    *piLoss = - (*piLoss);

    return iErrCode;
}

int GameEngine::IsSpectatorGame (int iGameClass, int iGameNumber, bool* pbSpectatorGame) {

    int iErrCode;
    Variant vGameOptions;

    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(pszGameData, GameData::Options, &vGameOptions);
    RETURN_ON_ERROR(iErrCode);

    *pbSpectatorGame = (vGameOptions.GetInteger() & GAME_ALLOW_SPECTATORS) != 0;
    return iErrCode;
}

int GameEngine::AddToLatestGames(const Variant* pvColumns) {

    int iErrCode;
    ICachedTable* pGames = NULL;
    AutoRelease<ICachedTable> rel(pGames);

    unsigned int iNumRows;
    Variant vGames;

    // Read limit
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::NumGamesInLatestGameList, &vGames);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->GetTable(SYSTEM_LATEST_GAMES, &pGames);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGames->GetNumCachedRows(&iNumRows);
    RETURN_ON_ERROR(iErrCode);

    if (iNumRows == (unsigned int)vGames.GetInteger())
    {
        UTCTime tEnded, tOldestTime;
        unsigned int iKey = NO_KEY, iOldestKey = NO_KEY;

        Time::GetTime (&tOldestTime);
        Time::AddSeconds (tOldestTime, ONE_YEAR_IN_SECONDS, &tOldestTime);

        // Delete oldest game from table
        while (true)
        {
            iErrCode = pGames->GetNextKey (iKey, &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND)
                break;

            RETURN_ON_ERROR(iErrCode);

            iErrCode = pGames->ReadData(iKey, SystemLatestGames::Ended, &tEnded);
            RETURN_ON_ERROR(iErrCode);

            if (Time::OlderThan (tEnded, tOldestTime))
            {
                tOldestTime = tEnded;
                iOldestKey = iKey;
            }
        }

        if (iOldestKey != NO_KEY)
        {
            // Delete the oldest game
            iErrCode = pGames->DeleteRow(iOldestKey);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Finally, insert the new row
    iErrCode = pGames->InsertRow(SystemLatestGames::Template, pvColumns, NULL);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetNumUniqueEmpiresInGames(unsigned int* piNumEmpires)
{
    int iErrCode;

    *piNumEmpires = 0;

    Variant** ppvActiveGame = NULL;
    AutoFreeData free_ppvActiveGame(ppvActiveGame);
    unsigned int iNumGames;

    iErrCode = GetActiveGames(&ppvActiveGame, &iNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        iErrCode = CacheGameEmpireTables((const Variant**)ppvActiveGame, iNumGames);
        RETURN_ON_ERROR(iErrCode);
        
        HashTable<unsigned int, unsigned int, GenericHashValue<unsigned int>, GenericEquals <unsigned int> > htEmpires(NULL, NULL);
        bool ret = htEmpires.Initialize(250);
        Assert(ret);

        for (unsigned int i = 0; i < iNumGames; i ++)
        {
            int iGameClass = ppvActiveGame[i][0].GetInteger();
            int iGameNumber = ppvActiveGame[i][1].GetInteger();

            GET_GAME_EMPIRES(strGameEmpires, iGameClass, iGameNumber);

            Variant* pvEmpireKey = NULL;
            AutoFreeData free_pvEmpireKey(pvEmpireKey);

            unsigned int iNumEmpires;
            iErrCode = t_pCache->ReadColumn(strGameEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
            RETURN_ON_ERROR(iErrCode);

            for (unsigned int j = 0; j < iNumEmpires; j ++)
            {
                int iEmpireKey = pvEmpireKey[j].GetInteger();
                if (!htEmpires.Contains(iEmpireKey))
                {
                    ret = htEmpires.Insert(iEmpireKey, iEmpireKey);
                    Assert(ret);
                }
            }

            RETURN_ON_ERROR(iErrCode);
        }

        *piNumEmpires = htEmpires.GetNumElements();
    }

    return iErrCode;
}

int GameEngine::GetNumRecentActiveEmpiresInGames(unsigned int* piNumEmpires)
{
    // TODO - 377 - GetNumRecentActiveEmpiresInGames needs implementation
    *piNumEmpires = 0;
    return OK;
}