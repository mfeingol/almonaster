//
// GameEngine.dll:  a component of Almonaster
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
// Violently delete a game without warning.

int GameEngine::DeleteGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage,
                            int iReason) {

    unsigned int i, iNumEmpires;

    Variant* pvEmpireKey = NULL;
    int iErrCode;

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        Assert (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST);
        return iErrCode;
    }

    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadColumn (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        &pvEmpireKey, 
        &iNumEmpires
        );

    // Tolerance...
    if (iErrCode != OK || iErrCode == ERROR_DATA_NOT_FOUND || iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
        iNumEmpires = 0;
        iErrCode = OK;
    } else if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    char pszTemp [512 + MAX_FULL_GAME_CLASS_NAME_LENGTH];

    if (iEmpireKey == SYSTEM) {
        
        switch (iReason) {
        
        case SYSTEM_SHUTDOWN:
            sprintf (
                pszTemp,
                "%s %i was deleted after a system restart because the server was down too long",
                pszGameClassName,
                iGameNumber
                );
            break;
        
        case MAP_CREATION_ERROR:
            sprintf (
                pszTemp,
                "%s %i was deleted because of an error during map creation",
                pszGameClassName,
                iGameNumber
                );
            break;

        case PASSWORD_PROTECTED:
            sprintf (
                pszTemp,
                "%s %i was deleted because it was password protected and only one empire was playing",
                pszGameClassName,
                iGameNumber
                );
            break;

        case CREATION_FAILED:
            sprintf (
                pszTemp,
                "%s %i was deleted because there was a failure during game creation",
                pszGameClassName,
                iGameNumber
                );
            break;
        
        case GAME_UPDATING:
            sprintf (
                pszTemp,
                "%s %i was deleted because it was interrupted during an update",
                pszGameClassName,
                iGameNumber
                );
            break;

        case GAME_CREATING:
            sprintf (
                pszTemp,
                "%s %i was deleted because it was interrupted during game creation",
                pszGameClassName,
                iGameNumber
                );
            break;
        
        case GAME_ADDING_EMPIRE:
            sprintf (
                pszTemp,
                "%s %i was deleted because it was interrupted during empire addition",
                pszGameClassName,
                iGameNumber
                );
            break;

        case GAME_DELETING_EMPIRE:
            sprintf (
                pszTemp,
                "%s %i was deleted because it was interrupted during empire deletion",
                pszGameClassName,
                iGameNumber
                );
            break;

        case GAME_DELETING:
            sprintf (
                pszTemp,
                "%s %i was deleted because it was interrupted during game deletion",
                pszGameClassName,
                iGameNumber
                );
            break;

        case TABLE_VERIFICATION_ERROR:
            sprintf (
            pszTemp,
                "%s %i was deleted because its tables were corrupted",
                pszGameClassName,
                iGameNumber
                );
            break;

        default:
            Assert (false);
            sprintf (
                pszTemp,
                "%s %i was deleted for an unknown reason",
                pszGameClassName,
                iGameNumber
                );
            break;
        }

    } else {

        Variant vEmpireName;
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        if (iErrCode != OK) {
            vEmpireName = "";
        }

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
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Best effort send messages
    for (i = 0; i < iNumEmpires; i ++) {
        SendSystemMessage (pvEmpireKey[i], pszTemp, SYSTEM, MESSAGE_SYSTEM);
    }

    // Best effort send the message from the admin  
    if (!String::IsBlank (pszMessage)) {
        for (i = 0; i < iNumEmpires; i ++) {
            SendSystemMessage (pvEmpireKey[i], pszMessage, iEmpireKey, MESSAGE_ADMINISTRATOR);
        }
    }

Cleanup:

    // Clean up
    if (pvEmpireKey != NULL) {
        t_pConn->FreeData(pvEmpireKey);
    }

    // Cleanup the game
    return iErrCode;
}


// Delete game tables:
//
// GameEmpires(I.I)
// GameData(I.I)
// GameMap(I.I)
//
// Delete player tables
// Delete game from open/closed lists
//
// Delete game's data and tables

int GameEngine::CleanupGame (int iGameClass, int iGameNumber, GameResult grResult, const char* pszWinnerName) {

    int iErrCode;
    unsigned int iKey, iTournamentKey;

    Variant vEmpireKey, vName, vGameState, vTournamentKey;

    char pszGameClass [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];

    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GAME_DEAD_EMPIRES (strGameDeadEmpires, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    String strList;

    // Notification
    global.GetEventSink()->OnCleanupGame (iGameClass, iGameNumber);

    //
    // Need to be tolerant of errors...
    //

    // Get game state
    iErrCode = t_pConn->ReadData (strGameData, GameData::State, &vGameState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Get gameclass name
    iErrCode = GetGameClassName (iGameClass, pszGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Get gameclass tournament
    iErrCode = GetGameClassTournament (iGameClass, &iTournamentKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Update scores if necessary
    iErrCode = UpdateScoresOnGameEnd (iGameClass, iGameNumber);
    Assert (iErrCode == OK);

    iErrCode = t_pConn->WriteOr (strGameData, GameData::State, GAME_DELETING);
    Assert (iErrCode == OK);

    // Delete all remaining empires from the game
    iKey = NO_KEY;
    while (true) {

        iErrCode = t_pConn->GetNextKey (strGameEmpires, iKey, &iKey);
        if (iErrCode != OK) {   
            Assert (iErrCode == ERROR_DATA_NOT_FOUND);
            break;
        }

        iErrCode = t_pConn->ReadData (strGameEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
        if (iErrCode == OK) {

            iErrCode = t_pConn->ReadData (
                SYSTEM_EMPIRE_DATA, 
                vEmpireKey.GetInteger(), 
                SystemEmpireData::Name, 
                &vName
                );

            if (iErrCode == OK) {

                if (!strList.IsBlank()) {
                    strList += ", ";
                }
                strList += vName.GetCharPtr();
            }

            // Best effort
            iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, vEmpireKey.GetInteger(), EMPIRE_GAME_ENDED, NULL);
            Assert (iErrCode == OK);
        }
    }

    // Add the winner's name, if provided
    if (pszWinnerName != NULL) {
        if (!strList.IsBlank()) {
            strList += ", ";
        }
        strList += pszWinnerName;
    }
    
    // Report the game ending

    char* pszMessage = (char*) StackAlloc (strList.GetLength() + 80 + MAX_FULL_GAME_CLASS_NAME_LENGTH);
        
    sprintf (
        pszMessage,
        "%s %i ended with the following empires still alive: %s",
        pszGameClass,
        iGameNumber,
        strList.GetCharPtr() == NULL ? "" : strList.GetCharPtr()
        );
        
    global.GetReport()->WriteReport (pszMessage);

    // Add to latest games
    if (vGameState.GetInteger() & STARTED) {

        Variant pvLatestGame [SystemLatestGames::NumColumns];

        UTCTime tNow;
        Time::GetTime (&tNow);

        // Name
        pvLatestGame[SystemLatestGames::iName] = pszGameClass;

        // Number
        pvLatestGame[SystemLatestGames::iNumber] = iGameNumber;

        // Created
        iErrCode = t_pConn->ReadData(strGameData, GameData::CreationTime, pvLatestGame + SystemLatestGames::iCreated);
        if (iErrCode != OK) {
            pvLatestGame[SystemLatestGames::iCreated] = tNow;
        }

        // Ended
        pvLatestGame[SystemLatestGames::iEnded] = tNow;

        // Updates
        iErrCode = t_pConn->ReadData(strGameData, GameData::NumUpdates, pvLatestGame + SystemLatestGames::iUpdates);
        if (iErrCode != OK) {
            pvLatestGame[SystemLatestGames::iUpdates] = 0;
        }

        // Result
        pvLatestGame[SystemLatestGames::iResult] = (int) grResult;

        // Winner list
        pvLatestGame[SystemLatestGames::iWinners] = strList;

        // Loser list
        iKey = NO_KEY;
        strList.Clear();

        while (true) {
            
            iErrCode = t_pConn->GetNextKey (strGameDeadEmpires, iKey, &iKey);
            if (iErrCode != OK) {
                Assert (iErrCode == ERROR_DATA_NOT_FOUND);
                break;
            }
            
            iErrCode = t_pConn->ReadData (strGameDeadEmpires, iKey, GameDeadEmpires::Name, &vName); 
            if (iErrCode == OK) {

                if (!strList.IsBlank()) {
                    strList += ", ";
                }
                strList += vName.GetCharPtr();
            }
        }

        pvLatestGame[SystemLatestGames::iLosers] = strList;

        iErrCode = AddToLatestGames (pvLatestGame, iTournamentKey);
        Assert (iErrCode == OK);
    }


    ////////////////////////////////////////
    // Best effort delete all game tables //
    ////////////////////////////////////////

    char pszTable [512];

    // GameEmpires(I.I)
    iErrCode = t_pConn->DeleteTable (strGameEmpires);
    Assert (iErrCode == OK);

    // GameMap(I.I)
    GET_GAME_MAP (pszTable, iGameClass, iGameNumber);
    iErrCode = t_pConn->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    // GameIndependentShips(I.I)
    Variant vGameOptions, vGameClassOptions;

    iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vGameClassOptions);
    if (iErrCode == OK) {
        
        if (vGameClassOptions.GetInteger() & INDEPENDENCE) {

            GET_GAME_INDEPENDENT_SHIPS (pszTable, iGameClass, iGameNumber);

            iErrCode = t_pConn->DeleteTable (pszTable);
            Assert (iErrCode == OK);
        }
    }

    iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::TournamentKey, &vTournamentKey);
    if (iErrCode != OK) {
        Assert (false);
        vTournamentKey = NO_KEY;
    }

    // GameSecurity(I.I)
    iErrCode = t_pConn->ReadData (strGameData, GameData::Options, &vGameOptions);
    if (iErrCode == OK) {

        if (vGameOptions.GetInteger() & GAME_ENFORCE_SECURITY) {

            GET_GAME_SECURITY (pszTable, iGameClass, iGameNumber);

            iErrCode = t_pConn->DeleteTable (pszTable);
            Assert (iErrCode == OK);
        }
    }

    // GameDeadEmpires(I.I)
    GET_GAME_DEAD_EMPIRES (pszTable, iGameClass, iGameNumber);
    iErrCode = t_pConn->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    // GameData(I.I)
    iErrCode = t_pConn->DeleteTable (strGameData);
    Assert (iErrCode == OK);

    // Delete game from active game list
    char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
    GetGameClassGameNumber (iGameClass, iGameNumber, pszData);

    iErrCode = t_pConn->GetFirstKey (
        SYSTEM_ACTIVE_GAMES,
        SystemActiveGames::GameClassGameNumber,
        pszData,
        &iKey
        );

    if (iErrCode == OK && iKey != NO_KEY) {
        iErrCode = t_pConn->DeleteRow (SYSTEM_ACTIVE_GAMES, iKey);
        Assert (iErrCode == OK);
    }

    else iErrCode = ERROR_GAME_DOES_NOT_EXIST;

    // Delete game from tournament active game list
    if (iTournamentKey != NO_KEY) {

        SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszGames, iTournamentKey);

        iErrCode = t_pConn->GetFirstKey (
            pszGames,
            SystemTournamentActiveGames::GameClassGameNumber,
            pszData,
            &iKey
            );

        if (iErrCode == OK && iKey != NO_KEY) {

            iErrCode = t_pConn->DeleteRow (pszGames, iKey);
            Assert (iErrCode == OK);
        }

        // Check for last game in tournament
        unsigned int iOwner;
        iErrCode = GetTournamentOwner (iTournamentKey, &iOwner);
        if (iErrCode == OK && iOwner == DELETED_EMPIRE_KEY) {

            unsigned int iNumGames;
            iErrCode = GetTournamentGames (iTournamentKey, NULL, NULL, &iNumGames);
            if (iErrCode == OK && iNumGames == 0) {

                iErrCode = DeleteTournament (DELETED_EMPIRE_KEY, iTournamentKey, false);
                Assert (iErrCode == OK);
            }
        }
    }

    // Decrement number of games in gameclass
    iErrCode = t_pConn->Increment (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::NumActiveGames,
        -1
        );
    Assert (iErrCode == OK);

    // Best effort attempt to delete gameclass if it's marked for deletion
    if (vGameClassOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION) {
    
        bool bDeleted;
#ifdef _DEBUG
        int iErrCode2 = 
#endif
        DeleteGameClass (iGameClass, &bDeleted);
        Assert (iErrCode2 == OK || iErrCode2 == ERROR_GAMECLASS_DOES_NOT_EXIST);
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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData (pszGameData, GameData::CreationTime, &vTime);

    if (iErrCode == OK) {
        *ptCreationTime = vTime.GetInteger64();
    }

    return iErrCode;
}


int GameEngine::GetGameState (int iGameClass, int iGameNumber, int* piGameState) {

    Variant vValue;
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData (pszGameData, GameData::State, &vValue);

    if (iErrCode == OK) {
        *piGameState = vValue.GetInteger();
    }

    return iErrCode;
}

// Output:
// *piNumGames -> Number of open games
//
// Return the current number of active (open + closed) games on the server

int GameEngine::GetNumActiveGames (int* piNumGames) {

    return t_pConn->GetNumRows (SYSTEM_ACTIVE_GAMES, (unsigned int*) piNumGames);
}


// Output:
// *piNumGames -> Number of games
//
// Return the current number of open games on the server

int GameEngine::GetNumOpenGames (int* piNumGames) {

    int iErrCode = t_pConn->GetEqualKeys (
        SYSTEM_ACTIVE_GAMES, 
        SystemActiveGames::State, 
        STILL_OPEN, 
        NULL,
        (unsigned int*) piNumGames
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Output:
// *piNumGames -> Number of games
//
// Return the current number of closed games on the server

int GameEngine::GetNumClosedGames (int* piNumGames) {

    int iErrCode = t_pConn->GetEqualKeys (
        SYSTEM_ACTIVE_GAMES, 
        SystemActiveGames::State, 
        0, 
        false, 
        (unsigned int*) piNumGames
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Output:
// **ppiGameClass -> GameClasses of active games
// **ppiGameNumber -> GameNumbers of active games
// *piNumGames -> Number of gamenumbers returned.
//
// Return the keys of the server's active games

int GameEngine::GetActiveGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames) {

    unsigned int i, iNumGames;
    Variant* pvGames;

    *piNumGames = 0;
    *ppiGameNumber = NULL;
    *ppiGameClass = NULL;

    // Get active games
    int iErrCode = t_pConn->ReadColumn (
        SYSTEM_ACTIVE_GAMES,
        SystemActiveGames::GameClassGameNumber,
        &pvGames, 
        &iNumGames
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    else if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *piNumGames = (int) iNumGames;

    if (iNumGames > 0) {

        *ppiGameClass = new int [*piNumGames];
        *ppiGameNumber = new int [*piNumGames];

        for (i = 0; i < iNumGames; i ++) {

            GetGameClassGameNumber (
                pvGames[i].GetCharPtr(), 
                (*ppiGameClass) + i,
                (*ppiGameNumber) + i
                );
        }

        t_pConn->FreeData(pvGames);
    }

    return iErrCode;
}


// Output:
// **ppvGameData -> GameClass.GameNumber strings
// *piNumGames -> Number of gamenumbers returned.
//
// Return the keys of the server's open games

int GameEngine::GetOpenGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames) {

    return GetGames (true, ppiGameClass, ppiGameNumber, piNumGames);
}

int GameEngine::GetClosedGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames) {

    return GetGames (false, ppiGameClass, ppiGameNumber, piNumGames);
}

int GameEngine::GetGames (bool bOpen, int** ppiGameClass, int** ppiGameNumber, int* piNumGames) {

    unsigned int* piKey, iNumKeys, i;
    int iErrCode;
    IReadTable* pGames;

    *piNumGames = 0;
    *ppiGameClass = NULL;
    *ppiGameNumber = NULL;

    iErrCode = t_pConn->GetTableForReading (SYSTEM_ACTIVE_GAMES, &pGames);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = pGames->GetEqualKeys (
        SystemActiveGames::State,
        bOpen ? STILL_OPEN : 0,
        &piKey,
        &iNumKeys
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    else if (iErrCode == OK) {

        *ppiGameClass = new int [iNumKeys];
        if (*ppiGameClass == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        *ppiGameNumber = new int [iNumKeys];
        if (*ppiGameNumber == NULL) {
            delete [] (*ppiGameClass);
            *ppiGameClass = NULL;
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }
        
        for (i = 0; i < iNumKeys; i ++)
        {
            Variant vGame;
            iErrCode = pGames->ReadData (piKey[i], SystemActiveGames::GameClassGameNumber, &vGame);
            if (iErrCode == OK)
            {
                GetGameClassGameNumber(vGame.GetCharPtr(), (*ppiGameClass) + *piNumGames, (*ppiGameNumber) + *piNumGames);
                (*piNumGames) ++;
            }
        }
    }

Cleanup:

    SafeRelease (pGames);

    if (piKey != NULL) {
        t_pConn->FreeKeys(piKey);
    }

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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData (pszGameData, GameData::State, &vOpen);

    if (iErrCode == OK) {
        *pbOpen = (vOpen.GetInteger() & STILL_OPEN) != 0;
    }

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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData (pszGameData, GameData::State, &vStarted);
    if (iErrCode == OK) {
        *pbStarted = (vStarted.GetInteger() & STARTED) != 0;
    }

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

int GameEngine::IsGamePasswordProtected (int iGameClass, int iGameNumber, bool* pbProtected) {

    Variant vPassword;
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData (pszGameData, GameData::Password, &vPassword);

    if (iErrCode == OK) {
        *pbProtected = !String::IsBlank (vPassword.GetCharPtr());
    }
    
    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// pszPassword -> The game's new password; can be blank
//
// Change a game's password

int GameEngine::SetGamePassword (int iGameClass, int iGameNumber, const char* pszNewPassword) {

    GAME_DATA (pszGameData, iGameClass, iGameNumber);
    return t_pConn->WriteData (pszGameData, GameData::Password, pszNewPassword);
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

    GAME_DATA(pszGameData, iGameClass, iGameNumber);
    return t_pConn->ReadData(pszGameData, pszColumn, pvProp);
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iProp -> GameData column name
// vProp -> The game's new property
//
// Change a game property

int GameEngine::SetGameProperty(int iGameClass, int iGameNumber, const char* pszColumn, const Variant& vProp) {

    GAME_DATA (pszGameData, iGameClass, iGameNumber);
    return t_pConn->WriteData (pszGameData, pszColumn, vProp);
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

int GameEngine::CreateGame (int iGameClass, int iEmpireCreator, const GameOptions& goGameOptions, 
                            int* piGameNumber) {

    int iNumUpdates, iErrCode;
    unsigned int i, j, k;

    Variant vTemp, vOptions, vHalted, vPrivilege, vEmpireScore, vGameNumber, vMaxNumActiveGames;

    bool bIncrementedActiveGameCount = false, bDeleteRequired = false, bFlag;
    char strTableName [256], strGameData [256];

    UTCTime tTime;
    Time::GetTime (&tTime);

    const unsigned int* piEmpireKey = goGameOptions.piEmpireKey;
    const unsigned int iNumEmpires = goGameOptions.iNumEmpires;

    Assert (iEmpireCreator != TOURNAMENT || (piEmpireKey != NULL && iNumEmpires > 0));

    // Make sure new game creation is enabled
    iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!(vTemp.GetInteger() & NEW_GAMES_ENABLED)) {
        return ERROR_DISABLED;
    }

    int* piCopy = (int*) StackAlloc (iNumEmpires * sizeof (int));
    memcpy (piCopy, piEmpireKey, iNumEmpires * sizeof (int));
    Algorithm::QSortAscending<int> (piCopy, iNumEmpires);

    // Check empire existence
    for (i = 0; i < iNumEmpires; i ++) {

        if (DoesEmpireExist (piCopy[i], &bFlag, NULL) != OK || !bFlag) {
            iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
            goto OnError;
        }
    }

    // Test for gameclass halt
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );

    if (iErrCode != OK) {
        iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        goto OnError;
    }

    if (vOptions.GetInteger() & GAMECLASS_HALTED) {
        iErrCode = ERROR_GAMECLASS_HALTED;
        goto OnError;
    }

    if (vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION &&
        !(vOptions.GetInteger() & DYNAMIC_GAMECLASS)) {
        iErrCode = ERROR_GAMECLASS_DELETED;
        goto OnError;
    }

    if (goGameOptions.iTournamentKey != NO_KEY) {

        int iOpt;

        // Make sure empires can be entered into tournament game
        for (i = 0; i < iNumEmpires; i ++) {

            if (GetEmpireOptions2 (piEmpireKey[i], &iOpt) != OK || (iOpt & UNAVAILABLE_FOR_TOURNAMENTS)) {
                iErrCode = ERROR_EMPIRE_IS_UNAVAILABLE_FOR_TOURNAMENTS;
                goto OnError;
            }
        }
    }

    // Test for too many games
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::MaxNumActiveGames,
        &vMaxNumActiveGames
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    if (vMaxNumActiveGames.GetInteger() != INFINITE_ACTIVE_GAMES) {

        Variant vNumActiveGames;

        iErrCode = t_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA,
            iGameClass,
            SystemGameClassData::NumActiveGames,
            &vNumActiveGames
            );

        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        if (vNumActiveGames.GetInteger() >= vMaxNumActiveGames.GetInteger()) {

            // Too many games
            iErrCode = ERROR_TOO_MANY_GAMES;
            goto OnError;
        }
    }

    // Increment number of active games
    iErrCode = t_pConn->Increment (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::NumActiveGames,
        1
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    bIncrementedActiveGameCount = true;

    for (i = 0; i < iNumEmpires; i ++) {

        // Make sure empire isn't halted
        iErrCode = t_pConn->ReadData (SYSTEM_EMPIRE_DATA, piEmpireKey[i], SystemEmpireData::Options, &vHalted);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        if (vHalted.GetInteger() & EMPIRE_MARKED_FOR_DELETION) {
            iErrCode = ERROR_EMPIRE_IS_HALTED;
            goto OnError;
        }

        // Make sure empire is at least a novice
        iErrCode = t_pConn->ReadData (SYSTEM_EMPIRE_DATA, piEmpireKey[i], SystemEmpireData::Privilege, &vPrivilege);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        if (vPrivilege.GetInteger() < NOVICE) {
            iErrCode = ERROR_INSUFFICIENT_PRIVILEGE;
            goto OnError;
        }
    }

    // Get unique game number and increment it
    iErrCode = t_pConn->Increment (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::OpenGameNum, 
        1, 
        &vGameNumber
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    *piGameNumber = vGameNumber.GetInteger();

    // Lock particular game
    {
        char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
        GetGameClassGameNumber (iGameClass, vGameNumber.GetInteger(), pszData);
        
        Variant pvActiveGameData[] = {
            pszData,
            STILL_OPEN
        };
        
        if (pvActiveGameData[SystemActiveGames::iGameClassGameNumber].GetCharPtr() == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto OnError;
        }

        // Add row to open games list
        iErrCode = t_pConn->InsertRow(SYSTEM_ACTIVE_GAMES, SystemActiveGames::Template, pvActiveGameData, NULL);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        // If tournament game, insert into tournament active games table
        if (iEmpireCreator == TOURNAMENT) {

            unsigned int iTournamentKey;

            iErrCode = GetGameClassTournament (iGameClass, &iTournamentKey);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            Assert (iTournamentKey != NO_KEY);

            SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszGames, iTournamentKey);
            iErrCode = t_pConn->InsertRow (pszGames, SystemTournamentActiveGames::Template, pvActiveGameData, NULL);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
        }
        
    }   // Scope


    //////////////////////////////
    // Create new system tables //
    //////////////////////////////

    // Create "GameData(I.I)" table
    GET_GAME_DATA (strGameData, iGameClass, *piGameNumber);

    iErrCode = t_pConn->CreateTable (strGameData, GameData::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    bDeleteRequired = true;

    {
        Variant vEmpireName = "";
        if (iEmpireCreator != TOURNAMENT) {
            GetEmpireName (iEmpireCreator, &vEmpireName);
        }

        // Add row to GameData(I.I)
        Variant pvGameData [GameData::NumColumns] = {
            0,      // 0 is max num empires so far
            0,      // 0 updates
            tTime,  // Last updated now
            STILL_OPEN | GAME_CREATING,     // State
            MAX_COORDINATE,     // MinX
            0,      // 0 empires updated
            goGameOptions.pszPassword,  // Password
            MIN_COORDINATE,     // MaxX
            MAX_COORDINATE,     // MinY
            0,                  // Zero paused
            MIN_COORDINATE,     // MaxY
            0,
            tTime,      // LastUpdateCheck
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

        iErrCode = t_pConn->InsertRow (strGameData, GameData::Template, pvGameData, NULL);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

    }   // Scope

    // Create GameSecurity(I.I) table if necessary
    if (goGameOptions.iOptions & GAME_ENFORCE_SECURITY) {

        unsigned int i;

        Assert (goGameOptions.iOptions > 0);

        GET_GAME_SECURITY (strTableName, iGameClass, *piGameNumber);

        iErrCode = t_pConn->CreateTable (strTableName, GameSecurity::Template);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        // Populate rows
        Variant pvGameSec [GameSecurity::NumColumns];

        for (i = 0; i < goGameOptions.iNumSecurityEntries; i ++) {

            IReadTable* pEmpires = NULL;
            unsigned int iRowEmpireKey = goGameOptions.pSecurity[i].iEmpireKey;

            //
            // The idea is to ignore rows that don't resolve to the intended empire
            //

            iErrCode = t_pConn->GetTableForReading (SYSTEM_EMPIRE_DATA, &pEmpires);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iErrCode = pEmpires->DoesRowExist (iRowEmpireKey, &bFlag);
            if (iErrCode != OK || !bFlag) {
                SafeRelease (pEmpires);
                continue;
            }

            // Get empire name, secret key, ip address, and session id
            iErrCode = pEmpires->ReadData(iRowEmpireKey, SystemEmpireData::Name, pvGameSec + GameSecurity::iName);
            if (iErrCode == OK) {
                iErrCode = pEmpires->ReadData(iRowEmpireKey, SystemEmpireData::IPAddress, pvGameSec + GameSecurity::iIPAddress);
                if (iErrCode == OK) {
                    iErrCode = pEmpires->ReadData(iRowEmpireKey, SystemEmpireData::SessionId, pvGameSec + GameSecurity::iSessionId);
                    if (iErrCode == OK) {
                        iErrCode = pEmpires->ReadData(iRowEmpireKey, SystemEmpireData::SecretKey, pvGameSec + GameSecurity::iSecretKey);
                    }
                }
            }

            SafeRelease (pEmpires);

            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            // Check name against string provided by creator
            if (_stricmp (
                pvGameSec[GameSecurity::iName].GetCharPtr(), 
                goGameOptions.pSecurity[i].pszEmpireName
                ) != 0) {

                // The empire was probably deleted - just ignore and continue
                continue;
            }

            // Set remaining columns
            pvGameSec [GameSecurity::iEmpireKey] = iRowEmpireKey;
            pvGameSec [GameSecurity::iOptions] = goGameOptions.pSecurity[i].iOptions;

            // Insert row
            iErrCode = t_pConn->InsertRow (strTableName, GameSecurity::Template, pvGameSec, NULL);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
        }
    }

    // Create "GameEmpires(I.I)" table
    GET_GAME_EMPIRES (strTableName, iGameClass, *piGameNumber);
    iErrCode = t_pConn->CreateTable (strTableName, GameEmpires::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameDeadEmpires(I.I)" table
    GET_GAME_DEAD_EMPIRES (strTableName, iGameClass, *piGameNumber);
    iErrCode = t_pConn->CreateTable (strTableName, GameDeadEmpires::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameMap(I.I)" table
    GET_GAME_MAP (strTableName, iGameClass, *piGameNumber);
    iErrCode = t_pConn->CreateTable (strTableName, GameMap::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create GameIndependentShips(I.I) table if necessary
    if (vOptions.GetInteger() & INDEPENDENCE) {

        GAME_INDEPENDENT_SHIPS (strTableName, iGameClass, *piGameNumber);
        iErrCode = t_pConn->CreateTable (strTableName, GameIndependentShips::Template);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
    }

    // No longer creating
    iErrCode = t_pConn->WriteAnd (strGameData, GameData::State, ~GAME_CREATING);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    for (i = 0; i < iNumEmpires; i ++) {

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

        if (iErrCode != OK)
        {
            goto OnError;
        }
    }

    // Handle team arrangements
    if (goGameOptions.iTeamOptions != 0) {

        // Insert diplomatic entries if necessary
        if (goGameOptions.iTeamOptions & TEAM_PREARRANGED_DIPLOMACY &&
            !(vOptions.GetInteger() & EXPOSED_DIPLOMACY)) {

            Variant pvInsert [GameEmpireDiplomacy::NumColumns] = {
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
            for (i = 0; i < goGameOptions.iNumPrearrangedTeams; i ++) {

                const PrearrangedTeam& aTeam = goGameOptions.paPrearrangedTeam[i];

                for (j = 0; j < aTeam.iNumEmpires; j ++) {

                    for (k = j + 1; k < aTeam.iNumEmpires; k ++) {

                        GET_GAME_EMPIRE_DIPLOMACY (strTableName, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);
                        pvInsert [GameEmpireDiplomacy::iEmpireKey] = aTeam.piEmpireKey[k];

                        iErrCode = t_pConn->InsertRow (strTableName, GameEmpireDiplomacy::Template, pvInsert, NULL);
                        if (iErrCode != OK) {
                            goto OnError;
                        }

                        GET_GAME_EMPIRE_DIPLOMACY (strTableName, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);
                        pvInsert [GameEmpireDiplomacy::iEmpireKey] = aTeam.piEmpireKey[j];

                        iErrCode = t_pConn->InsertRow (strTableName, GameEmpireDiplomacy::Template, pvInsert, NULL);
                        if (iErrCode != OK) {
                            goto OnError;
                        }
                    }
                }
            }

        }   // End if insertions are needed

        if (goGameOptions.iTeamOptions & TEAM_PREARRANGED_ALLIANCES) {

            Variant vDipLevel;
            int iMaxAlliances = 0, iDipLevel;

            iErrCode = GetGameClassDiplomacyLevel (iGameClass, &iDipLevel);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iErrCode = t_pConn->ReadData (
                SYSTEM_GAMECLASS_DATA, 
                iGameClass, 
                SystemGameClassData::MapsShared, 
                &vDipLevel
                );

            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            bool bShared = vDipLevel.GetInteger() == ALLIANCE;

            if (iDipLevel & ALLIANCE) {

                // Set each empire to alliance in teammate's diplomacy table
                for (i = 0; i < goGameOptions.iNumPrearrangedTeams; i ++) {

                    const PrearrangedTeam& aTeam = goGameOptions.paPrearrangedTeam[i];
                    unsigned int iKey, iNumAtAlliance = aTeam.iNumEmpires - 1;

                    if (iNumAtAlliance > (unsigned int) iMaxAlliances) {
                        iMaxAlliances = aTeam.iNumEmpires - 1;
                    }

                    for (j = 0; j < aTeam.iNumEmpires; j ++) {

                        GET_GAME_EMPIRE_DATA (strTableName, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);

                        iErrCode = t_pConn->WriteData (strTableName, GameEmpireData::NumTruces, iNumAtAlliance);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }

                        iErrCode = t_pConn->WriteData (strTableName, GameEmpireData::NumTrades, iNumAtAlliance);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }

                        iErrCode = t_pConn->WriteData (strTableName, GameEmpireData::NumAlliances, iNumAtAlliance);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }

                        for (k = j + 1; k < aTeam.iNumEmpires; k ++) {

                            char pszDip1 [256], pszDip2 [256];

                            GET_GAME_EMPIRE_DIPLOMACY (pszDip1, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);

                            iErrCode = t_pConn->GetFirstKey (
                                pszDip1,
                                GameEmpireDiplomacy::EmpireKey,
                                aTeam.piEmpireKey[k],
                                &iKey
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = t_pConn->WriteData (pszDip1, iKey, GameEmpireDiplomacy::DipOffer, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = t_pConn->WriteData (pszDip1, iKey, GameEmpireDiplomacy::CurrentStatus, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = t_pConn->WriteData (pszDip1, iKey, GameEmpireDiplomacy::VirtualStatus, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = t_pConn->WriteOr (pszDip1, iKey, GameEmpireDiplomacy::State, ONCE_ALLIED_WITH);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            GET_GAME_EMPIRE_DIPLOMACY (pszDip2, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);

                            iErrCode = t_pConn->GetFirstKey (
                                pszDip2,
                                GameEmpireDiplomacy::EmpireKey,
                                aTeam.piEmpireKey[j],
                                &iKey
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = t_pConn->WriteData (pszDip2, iKey, GameEmpireDiplomacy::DipOffer, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = t_pConn->WriteData (pszDip2, iKey, GameEmpireDiplomacy::CurrentStatus, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = t_pConn->WriteData (pszDip2, iKey, GameEmpireDiplomacy::VirtualStatus, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = t_pConn->WriteOr (pszDip2, iKey, GameEmpireDiplomacy::State, ONCE_ALLIED_WITH);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            // Share maps
                            if (bShared) {

                                GAME_EMPIRE_MAP (pszMap1, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);
                                GAME_EMPIRE_MAP (pszMap2, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);

                                GAME_EMPIRE_DATA (pszData1, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);
                                GAME_EMPIRE_DATA (pszData2, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);

                                GAME_MAP (pszMap, iGameClass, *piGameNumber);

                                char* ppszDip[2] = { pszDip1, pszDip2 };
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

                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto OnError;
                                }
                            }
                        }
                    }
                }
                
                int iAllianceLimit;

                iErrCode = GetMaxNumDiplomacyPartners (iGameClass, *piGameNumber, ALLIANCE, &iAllianceLimit);
                if (iErrCode != OK) {
                    Assert (false);
                    goto OnError;
                }

                if (iAllianceLimit != UNRESTRICTED_DIPLOMACY && iMaxAlliances > iAllianceLimit) {
                    iErrCode = ERROR_ALLIANCE_LIMIT_EXCEEDED;
                    goto OnError;
                }
            }
        }
    }

    //
    // No errors from this point on
    //

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];
    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        sprintf (pszGameClassName, "Error %i", iErrCode);
        iErrCode = OK;
    }

    // Send notification messages for tournament games
    if (iEmpireCreator == TOURNAMENT) {

        char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];

        sprintf (
            pszMessage,
            "The tournament game %s %i has been started",
            pszGameClassName,
            *piGameNumber
            );

        for (i = 0; i < iNumEmpires; i ++) {
            int iErrCode2 = SendSystemMessage (piEmpireKey[i], pszMessage, SYSTEM, MESSAGE_SYSTEM);
            Assert (iErrCode2 == OK);
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

    global.GetReport()->WriteReport (pszUpdateReport);

    return OK;

OnError:

    if (!bIncrementedActiveGameCount && bDeleteRequired) {

        // Balance the cleanup decrement
        int iErrCode2 = t_pConn->Increment (
            SYSTEM_GAMECLASS_DATA,
            iGameClass,
            SystemGameClassData::NumActiveGames,
            1
            );
        Assert (iErrCode2 == OK);
    }

    // Best effort delete the game
    if (bDeleteRequired) {

        int iErrCode2 = DeleteGame (iGameClass, *piGameNumber, SYSTEM, "", CREATION_FAILED);
        Assert (iErrCode2 == OK);
    }

    *piGameNumber = 0;

    return iErrCode;
}


// Input:
// iGameClass -> Game class
// iGameNumber -> Game number
// iEmpireKey -> Empire key
// pszPassword -> Password
//
// Output:
// *piNumUpdates -> Number of updates transpired
//
// Make an empire enter an already created game.

int GameEngine::EnterGame(int iGameClass, int iGameNumber, int iEmpireKey, const char* pszPassword,
                          const GameOptions* pgoGameOptions, int* piNumUpdates, 
                          bool bSendMessages, bool bCreatingGame, bool bCheckSecurity) {

    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    SYSTEM_EMPIRE_ACTIVE_GAMES (strEmpireActiveGames, iEmpireKey);

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DEAD_EMPIRES (strGameDeadEmpires, iGameClass, iGameNumber);
    GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_SHIPS (strGameEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    String strDuplicateIPList, strDuplicateIdList, strDuplicateList, strMessage;
    bool bWarn, bBlock, bFlag, bAddedToGame = false, bClosed = false, bStarted = false, bPaused = false, bUnPaused = false;

    int iErrCode, iNumTechs, iDefaultOptions, iGameState, iSystemOptions, iEmpireOptions;
    unsigned int iCurrentNumEmpires = -1, i, iKey = NO_KEY;

    Variant vGameClassOptions, vHalted, vPassword, vPrivilege, vMinScore, vMaxScore, vEmpireScore, vTemp, 
        vMaxNumEmpires, vStillOpen, vNumUpdates, vMaxTechDev, vEmpireName, vDiplomacyLevel,
        vGameOptions, vSecretKey;

    bool bGenerateMapForAllEmpires = false;

    GameFairnessOption gfoFairness;

    Variant pvGameEmpireData[GameEmpireData::NumColumns];

    // Get time
    UTCTime tTime;
    Time::GetTime (&tTime);

    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
    if (iErrCode != OK || !bFlag) {
        iErrCode = ERROR_GAME_DOES_NOT_EXIST;
        goto OnError;
    }

    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vGameClassOptions
        );

    if (iErrCode != OK) {
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY) {
            iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        }
        goto OnError;
    }

    iErrCode = GetSystemOptions (&iSystemOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vDiplomacyLevel
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    iErrCode = t_pConn->ReadData(strGameData, GameData::Options, &vGameOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    iErrCode = t_pConn->ReadData(strGameData, GameData::MapFairness, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    gfoFairness = (GameFairnessOption)vTemp.GetInteger();

    // Make sure we still exist, kill game if not and we just created it and we're alone
    iErrCode = DoesEmpireExist (iEmpireKey, &bFlag, &vEmpireName);
    if (iErrCode != OK || !bFlag) {
        iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
        goto OnError;
    }

    // Get num empires in game
    iErrCode = t_pConn->GetNumRows (strGameEmpires, &iCurrentNumEmpires);
    if (iErrCode != OK) {
        iErrCode = ERROR_GAME_DOES_NOT_EXIST;
        goto OnError;
    }

    // Make sure empire isn't halted
    iErrCode = GetEmpireOptions (iEmpireKey, &iEmpireOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    if (iEmpireOptions & EMPIRE_MARKED_FOR_DELETION) {
        iErrCode = ERROR_EMPIRE_IS_HALTED;
        goto OnError;
    }

    // Make sure empire hasn't entered already  
    iErrCode = t_pConn->GetFirstKey (strGameEmpires, GameEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        if (iErrCode == OK) {
            iErrCode = ERROR_ALREADY_IN_GAME;
        }
        goto OnError;
    }

    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::SecretKey, &vSecretKey);
    if (iErrCode != OK) {
        goto OnError;
    }

    // Make sure empire wasn't nuked out
    iErrCode = t_pConn->GetFirstKey (strGameDeadEmpires, GameDeadEmpires::SecretKey, vSecretKey, &iKey);
    if (iErrCode != ERROR_DATA_NOT_FOUND && iErrCode != ERROR_UNKNOWN_TABLE_NAME) {
        iErrCode = ERROR_WAS_ALREADY_IN_GAME;
        goto OnError;
    }
    
    // Make sure game is still open
    iErrCode = t_pConn->ReadData (strGameData, GameData::State, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    iGameState = vTemp.GetInteger();

    if (!(iGameState & STILL_OPEN)) {
        iErrCode = ERROR_GAME_CLOSED;
        goto OnError;
    }
    
    // Test for correct password
    iErrCode = t_pConn->ReadData (strGameData, GameData::Password, &vPassword);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    if (pszPassword == NULL && pgoGameOptions != NULL)
        pszPassword = pgoGameOptions->pszPassword;
    
    if (!String::IsBlank(vPassword.GetCharPtr()) &&
         String::StrCmp(vPassword.GetCharPtr(), pszPassword) != 0) {
        iErrCode = ERROR_WRONG_PASSWORD;
        goto OnError;
    }

    if (bCheckSecurity) {

        GameAccessDeniedReason reason;
        iErrCode = GameAccessCheck (iGameClass, iGameNumber, iEmpireKey, NULL, ENTER_GAME, &bFlag, &reason);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        if (!bFlag) {
            iErrCode = ERROR_ACCESS_DENIED;
            goto OnError;
        }

        // Search for duplicates        
        bWarn  = (vGameOptions.GetInteger() & GAME_WARN_ON_DUPLICATE_IP_ADDRESS) != 0;
        bBlock = (vGameOptions.GetInteger() & GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS) != 0;
        
        if (bWarn || bBlock) {
            
            int* piDuplicateKeys;
            unsigned int iNumDuplicates;
            
            iErrCode = DoesEmpireHaveDuplicates (
                iGameClass, 
                iGameNumber, 
                iEmpireKey,
                SystemEmpireData::IPAddress,
                &piDuplicateKeys, 
                &iNumDuplicates
                );
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            
            if (iNumDuplicates > 0) {
                
                iErrCode = BuildDuplicateList (piDuplicateKeys, iNumDuplicates, &strDuplicateList);
                if (iErrCode != OK) {
                    Assert (false);
                    delete [] piDuplicateKeys;
                    goto OnError;
                }
                
                // Clean up
                delete [] piDuplicateKeys;
                
                if (bWarn && !bBlock) {
                    
                    // Post message
                    strDuplicateIPList = BEGIN_STRONG;
                    strDuplicateIPList += vEmpireName.GetCharPtr();
                    strDuplicateIPList += END_STRONG " has the same " BEGIN_STRONG "IP address" END_STRONG " as ";
                    strDuplicateIPList += strDuplicateList;
                    
                    if (strDuplicateIPList.GetCharPtr() == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto OnError;
                    }
                }
                else if (!bWarn && bBlock) {
                    iErrCode = ERROR_DUPLICATE_IP_ADDRESS;
                    goto OnError;
                }
                else if (bWarn && bBlock) {

                    // Prepare message
                    char* pszMessage = (char*) StackAlloc (256 + MAX_EMPIRE_NAME_LENGTH + strDuplicateList.GetLength());

                    sprintf (
                        pszMessage, 
                        BEGIN_STRONG "%s attempted to enter the game with the same " BEGIN_STRONG "IP address" END_STRONG " as %s",
                        vEmpireName.GetCharPtr(),
                        strDuplicateList.GetCharPtr()
                        );

                    // Best effort send
                    iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
                    Assert (iErrCode == OK);

                    // Exit with access denied
                    iErrCode = ERROR_DUPLICATE_IP_ADDRESS;
                    goto OnError;
                }
            }
        }
        
        bWarn  = (vGameOptions.GetInteger() & GAME_WARN_ON_DUPLICATE_SESSION_ID) != 0;
        bBlock = (vGameOptions.GetInteger() & GAME_BLOCK_ON_DUPLICATE_SESSION_ID) != 0;
        
        if (bWarn || bBlock) {
            
            int* piDuplicateKeys;
            unsigned int iNumDuplicates;
            
            iErrCode = DoesEmpireHaveDuplicates (
                iGameClass, 
                iGameNumber, 
                iEmpireKey,
                SystemEmpireData::SessionId,
                &piDuplicateKeys, 
                &iNumDuplicates
                );

            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            
            if (iNumDuplicates > 0) {
                
                iErrCode = BuildDuplicateList (piDuplicateKeys, iNumDuplicates, &strDuplicateList);
                if (iErrCode != OK) {
                    Assert (false);
                    delete [] piDuplicateKeys;
                    goto OnError;
                }
                
                // Clean up
                delete [] piDuplicateKeys;
                
                if (bWarn && !bBlock) {
                    
                    // Post message
                    strDuplicateIdList = BEGIN_STRONG;
                    strDuplicateIdList += vEmpireName.GetCharPtr();
                    strDuplicateIdList += END_STRONG " has the same " BEGIN_STRONG "Session Id" END_STRONG " as ";
                    strDuplicateIdList += strDuplicateList;
                    
                    if (strDuplicateIdList.GetCharPtr() == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto OnError;
                    }
                }
                else if (!bWarn && bBlock) {
                    
                    // Exit with access denied
                    iErrCode = ERROR_DUPLICATE_SESSION_ID;
                    goto OnError;
                }
                else if (bWarn && bBlock) {

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
                    Assert (iErrCode == OK);

                    iErrCode = ERROR_DUPLICATE_SESSION_ID;
                    goto OnError;
                }
            }
        }

        // Check for 'pause on start' option
        if (!(iSystemOptions & PAUSE_GAMES_BY_DEFAULT)) {

            // If not admin-paused, unpause the game - a new empire came in and he's not paused by default
            if ((iGameState & PAUSED) && !(iGameState & ADMIN_PAUSED)) {

                bUnPaused = true;

                iErrCode = UnpauseGame (iGameClass, iGameNumber, false, false);
                if (iErrCode != OK) {
                    Assert (false);
                    goto OnError;
                }

                iGameState &= ~PAUSED;
            }
        }
        
        // Try to trigger all remaining updates
        if (!bCreatingGame) {
            iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, true, &bFlag);
            Assert (iErrCode == OK);
        }
    }

    // Lock the game for writing if necessary
    if (!bCreatingGame) {

        // Make sure the game didn't end
        iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
        if (iErrCode != OK || !bFlag) {
            iErrCode = ERROR_GAME_DOES_NOT_EXIST;
            goto OnError;
        }

        // Make sure game is still open
        iErrCode = t_pConn->ReadData (strGameData, GameData::State, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        iGameState = vTemp.GetInteger();

        if (!(iGameState & STILL_OPEN)) {
            iErrCode = ERROR_GAME_CLOSED;
            goto OnError;
        }
    }
    
    // We're adding an empire!
    iErrCode = t_pConn->WriteOr (strGameData, GameData::State, GAME_ADDING_EMPIRE);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Insert row into GameEmpires(I.I) table
    vTemp = iEmpireKey;
    iErrCode = t_pConn->InsertRow (strGameEmpires, GameEmpires::Template, &vTemp, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    iCurrentNumEmpires ++;
    bAddedToGame = true;
    
    // Increment MaxNumEmpires count if game has started
    if (iGameState & STARTED) {

        iErrCode = t_pConn->ReadData (strGameData, GameData::MaxNumEmpires, &vMaxNumEmpires);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        if ((int) iCurrentNumEmpires > vMaxNumEmpires.GetInteger()) {

            iErrCode = t_pConn->WriteData (strGameData, GameData::MaxNumEmpires, iCurrentNumEmpires);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
        }
    }
    
    // Close game if we're the last to enter    
    if (iGameState & STILL_OPEN) {

        iErrCode = t_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxNumEmpires, 
            &vMaxNumEmpires
            );

        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        if (vMaxNumEmpires.GetInteger() != UNLIMITED_EMPIRES &&
            vMaxNumEmpires.GetInteger() == (int) iCurrentNumEmpires) {
            
            bClosed = true;
            
            // Close game
            iErrCode = t_pConn->WriteAnd (strGameData, GameData::State, ~STILL_OPEN);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iGameState &= ~STILL_OPEN;
            
            // Delete from open list
            char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
            GetGameClassGameNumber (iGameClass, iGameNumber, pszData);
            
            unsigned int iGameKey;
            iErrCode = t_pConn->GetFirstKey (
                SYSTEM_ACTIVE_GAMES, 
                SystemActiveGames::GameClassGameNumber,
                pszData, 
                &iGameKey
                );

            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iErrCode = t_pConn->WriteData (SYSTEM_ACTIVE_GAMES, iGameKey, SystemActiveGames::State, 0);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
        }
    }

    // Insert row into "SystemEmpireActiveGames(I)" table   
    char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
    GetGameClassGameNumber (iGameClass, iGameNumber, pszData);
    vTemp = pszData;

    if (vTemp.GetCharPtr() == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto OnError;
    }

    if (!t_pConn->DoesTableExist (strEmpireActiveGames)) {

        iErrCode = t_pConn->CreateTable (strEmpireActiveGames, SystemEmpireActiveGames::Template);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
    }

    iErrCode = t_pConn->InsertRow (strEmpireActiveGames, SystemEmpireActiveGames::Template, &vTemp, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    ////////////////////////////
    // Create empire's tables //
    ////////////////////////////
    
    // Create "GameEmpireData(I.I.I)" table
    iErrCode = t_pConn->CreateTable (strGameEmpireData, GameEmpireData::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    // Insert data into GameEmpireData(I.I.I) table
    pvGameEmpireData[GameEmpireData::iNumPlanets] = 1;
    pvGameEmpireData[GameEmpireData::iTotalAg] = 0;
    pvGameEmpireData[GameEmpireData::iTotalFuel] = 0;
    pvGameEmpireData[GameEmpireData::iTotalMin] = 0;
    pvGameEmpireData[GameEmpireData::iTotalPop] = 0;
    
    // Initial tech level
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::InitialTechLevel, 
        pvGameEmpireData + GameEmpireData::iTechLevel
        );
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    // Add tech to initial tech if empire is a late-comer
    iErrCode = t_pConn->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    if (vNumUpdates.GetInteger() > 0) {

        Variant vPercentTechIncreaseForLatecomers;

        iErrCode = t_pConn->ReadData (
            SYSTEM_DATA,
            SystemData::PercentTechIncreaseForLatecomers,
            &vPercentTechIncreaseForLatecomers
            );

        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        iErrCode = t_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxTechDev, 
            &vMaxTechDev
            );
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

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
    iNumTechs = (int) sqrt (pvGameEmpireData[GameEmpireData::iTechLevel].GetFloat());

    // TechDevs
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::InitialTechDevs, 
        &pvGameEmpireData[GameEmpireData::iTechDevs]
        );
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    // TechUndevs
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DevelopableTechDevs, 
        pvGameEmpireData + GameEmpireData::iTechUndevs
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // NumAvailableTechUndevs
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::NumInitialTechDevs, 
        pvGameEmpireData + GameEmpireData::iNumAvailableTechUndevs
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    // Filter out already developed techs
    ENUMERATE_TECHS(i) {
        
        if (pvGameEmpireData[GameEmpireData::iTechDevs].GetInteger() & TECH_BITS[i]) {
            
            Assert (pvGameEmpireData[GameEmpireData::iTechUndevs].GetInteger() & TECH_BITS[i]);
            
            pvGameEmpireData[GameEmpireData::iTechUndevs] = 
                pvGameEmpireData[GameEmpireData::iTechUndevs].GetInteger() & ~(TECH_BITS[i]);
        }
    }

    // Fill in the rest of the data
    pvGameEmpireData[GameEmpireData::iEcon] = 1;                                                 // Econ
    pvGameEmpireData[GameEmpireData::iMil] = (float) 0.0;                                        // Mil
    pvGameEmpireData[GameEmpireData::iTargetPop] = 0;
    pvGameEmpireData[GameEmpireData::iHomeWorld] = NO_KEY;                                       // HWKey
    pvGameEmpireData[GameEmpireData::iNumUpdatesIdle] = 0;                                   // 0 updates idle
    pvGameEmpireData[GameEmpireData::iMaxBR] = GetBattleRank ((float) pvGameEmpireData[4]);  // Max BR
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
    pvGameEmpireData[GameEmpireData::iNextTotalPop] = pvGameEmpireData[GameEmpireData::iTotalMin]; // NextMin
    pvGameEmpireData[GameEmpireData::iNextMin] = 0;                  // NextFuel
    pvGameEmpireData[GameEmpireData::iNextFuel] = 0;
    pvGameEmpireData[GameEmpireData::iNumTruces] = 0;
    pvGameEmpireData[GameEmpireData::iNumTrades] = 0;
    pvGameEmpireData[GameEmpireData::iNumAlliances] = 0;

    // Select default message target
    iErrCode = t_pConn->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::DefaultMessageTarget, 
        &pvGameEmpireData[GameEmpireData::iDefaultMessageTarget]
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    pvGameEmpireData[GameEmpireData::iLastMessageTargetMask] = 0;

    switch (pvGameEmpireData[GameEmpireData::iDefaultMessageTarget].GetInteger()) {

    case MESSAGE_TARGET_TRUCE:

        if (!(vDiplomacyLevel.GetInteger() & TRUCE)) {
            pvGameEmpireData[GameEmpireData::iDefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;

    case MESSAGE_TARGET_TRADE:

        if (!(vDiplomacyLevel.GetInteger() & TRADE)) {
            pvGameEmpireData[GameEmpireData::iDefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;

    case MESSAGE_TARGET_ALLIANCE:

        if (!(vDiplomacyLevel.GetInteger() & ALLIANCE)) {
            pvGameEmpireData[GameEmpireData::iDefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;
    }

    // Get default number of saved game messages
    iErrCode = t_pConn->ReadData (
        SYSTEM_DATA, 
        SystemData::DefaultMaxNumGameMessages, 
        &pvGameEmpireData[GameEmpireData::iMaxNumGameMessages]
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

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
    iErrCode = t_pConn->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::DefaultBuilderPlanet, 
        &pvGameEmpireData[GameEmpireData::iDefaultBuilderPlanet]
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    pvGameEmpireData[GameEmpireData::iLastBuilderPlanet] = NO_KEY;

    pvGameEmpireData [GameEmpireData::iMaxEcon] = 0;
    pvGameEmpireData [GameEmpireData::iMaxMil] = 0;
    pvGameEmpireData [GameEmpireData::iNumAlliancesLeaked] = 0;

    if (vGameOptions.GetInteger() & GAME_COUNT_FOR_BRIDIER) {

        int iRank, iIndex;

        iErrCode = GetBridierScore (iEmpireKey, &iRank, &iIndex);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        pvGameEmpireData [GameEmpireData::iInitialBridierRank] = iRank;
        pvGameEmpireData [GameEmpireData::iInitialBridierIndex] = iIndex;

    } else {

        pvGameEmpireData [GameEmpireData::iInitialBridierRank] = 0;
        pvGameEmpireData [GameEmpireData::iInitialBridierIndex] = 0;
    }

    iErrCode = t_pConn->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::GameRatios, 
        pvGameEmpireData + GameEmpireData::iGameRatios
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    pvGameEmpireData[GameEmpireData::iMiniMaps] = MINIMAPS_NEVER;
    pvGameEmpireData[GameEmpireData::iMapFairnessResourcesClaimed] = 0;

    ///////////////////////////////
    // Insert GameEmpireData row //
    ///////////////////////////////

    iErrCode = t_pConn->InsertRow (strGameEmpireData, GameEmpireData::Template, pvGameEmpireData, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameEmpireMessages(I.I.I)" table
    iErrCode = t_pConn->CreateTable (strGameEmpireMessages, GameEmpireMessages::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameEmpireMap(I.I.I)" table
    iErrCode = t_pConn->CreateTable (strGameEmpireMap, GameEmpireMap::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    ////////////////////////////////////////////////
    // Create the rest of the new empire's tables //
    ////////////////////////////////////////////////

    // Create "GameEmpireShips(I.I.I)" table
    iErrCode = t_pConn->CreateTable (strGameEmpireShips, GameEmpireShips::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameEmpireFleets(I.I.I)" table
    iErrCode = t_pConn->CreateTable (strGameEmpireFleets, GameEmpireFleets::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameEmpireDiplomacy(I.I.I)" table
    iErrCode = t_pConn->CreateTable (strGameEmpireDiplomacy, GameEmpireDiplomacy::Template);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Send empire welcome message
    iErrCode = t_pConn->ReadData (strGameData, GameData::EnterGameMessage, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    if (!String::IsBlank (vTemp.GetCharPtr())) {

        Variant vCreatorName;
        const char* pszMessage;

        iErrCode = t_pConn->ReadData (strGameData, GameData::CreatorName, &vCreatorName);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        if (String::IsBlank (vCreatorName.GetCharPtr())) {

            // System game
            pszMessage = vTemp.GetCharPtr();

        } else {

            // Someone's game
            pszMessage = new char [MAX_EMPIRE_NAME_LENGTH + strlen (vTemp.GetCharPtr()) + 128];
            if (pszMessage == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto OnError;
            }

            sprintf (
                (char*) pszMessage,
                "%s, the creator of this game, says the following:" NEW_PARAGRAPH "%s",
                vCreatorName.GetCharPtr(),
                vTemp.GetCharPtr()
                );
        }

        iErrCode = SendGameMessage (iGameClass, iGameNumber, iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM, NULL_TIME);

        if (pszMessage != vTemp.GetCharPtr()) {
            delete [] (char*) pszMessage;
        }

        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
    }

    /////////////////////////
    // Game starting stuff //
    /////////////////////////

    // Has game started?    
    if (iGameState & STARTED) {

        // Add new empire to map if the map has been generated already
        if (iGameState & GAME_MAP_GENERATED) {

            iErrCode = AddEmpiresToMap(iGameClass, iGameNumber, &iEmpireKey, 1, gfoFairness, &bFlag);
            if (iErrCode != OK) {

                if (bFlag) {
                    
                    // Catastrophic - delete the game
#ifdef _DEBUG
                    Assert(false);
#endif
                    int iErrCode2 = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
                    Assert (iErrCode2 == OK);
                
                } else {

                    int iErrCode2 = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_GAME_ENTRY_ERROR, NULL);
                    Assert (iErrCode2 == OK);
                }

                return ERROR_COULD_NOT_CREATE_PLANETS;
            }
        }
        
    } else {
        
        // Are we the trigger for the game to begin?
        unsigned int iNumActiveEmpires;
        Variant vMinNumEmpires;

        iErrCode = t_pConn->GetNumRows (strGameEmpires, &iNumActiveEmpires);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        iErrCode = t_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MinNumEmpires, 
            &vMinNumEmpires
            );
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        if ((int) iNumActiveEmpires == vMinNumEmpires.GetInteger()) {
            
            bStarted = true;
            
            // Start game
            iErrCode = t_pConn->WriteOr (strGameData, GameData::State, STARTED);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iGameState |= STARTED;
            
            // Set last update time
            iErrCode = t_pConn->WriteData (strGameData, GameData::LastUpdateTime, tTime);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            
            // Set number of updates to zero
            iErrCode = t_pConn->WriteData (strGameData, GameData::NumUpdates, 0);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            // Set max num empires
            iErrCode = t_pConn->WriteData (strGameData, GameData::MaxNumEmpires, iCurrentNumEmpires);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            // Set paused if necessary
            if (iSystemOptions & PAUSE_GAMES_BY_DEFAULT) {

                bPaused = true;

                iErrCode = t_pConn->WriteData (strGameData, GameData::NumRequestingPause, iCurrentNumEmpires);
                if (iErrCode != OK) {
                    Assert (false);
                    goto OnError;
                }

                iErrCode = PauseGame (iGameClass, iGameNumber, false, false);
                if (iErrCode != OK) {
                    Assert (false);
                    goto OnError;
                }
            }
            
            // Add all players to map if the game isn't configured to do so on the first update
            if (!(vGameClassOptions.GetInteger() & GENERATE_MAP_FIRST_UPDATE)) {
                bGenerateMapForAllEmpires = true;
            }

            char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
            if (GetGameClassName (iGameClass, pszGameClassName) != OK) {
                StrNCpy (pszGameClassName, "Unknown gameclass");
            }

            char pszUpdateReport [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
            sprintf (pszUpdateReport, "%s %i has started", pszGameClassName, iGameNumber);
            global.GetReport()->WriteReport (pszUpdateReport);
        }
    }

    if (bGenerateMapForAllEmpires) {

        int* piEmpKey;
        unsigned int iNumKeys;

        if (pgoGameOptions != NULL && pgoGameOptions->iNumPrearrangedTeams > 0) {

            iNumKeys = pgoGameOptions->iNumEmpires;
            piEmpKey = (int*)StackAlloc(iNumKeys * sizeof(int));

            // Randomize team order
            const unsigned int iNumTeams = pgoGameOptions->iNumPrearrangedTeams;
            PrearrangedTeam* paRandomTeams = (PrearrangedTeam*)StackAlloc(iNumTeams * sizeof(PrearrangedTeam));

            memcpy(paRandomTeams, pgoGameOptions->paPrearrangedTeam, iNumTeams * sizeof(PrearrangedTeam));
            Algorithm::Randomize<PrearrangedTeam>(paRandomTeams, iNumTeams);

            unsigned int iIndex = 0;
            for (unsigned int i = 0; i < pgoGameOptions->iNumPrearrangedTeams; i ++) {
                const PrearrangedTeam& team = paRandomTeams[i];
                for (unsigned int j = 0; j < team.iNumEmpires; j ++)
                    piEmpKey[iIndex ++] = team.piEmpireKey[j];

                // Randomize team members
                Algorithm::Randomize<int>(piEmpKey + iIndex - team.iNumEmpires, team.iNumEmpires);
            }

        } else {

            Variant* pvKey;        
            iErrCode = t_pConn->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, &pvKey, &iNumKeys);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            
            piEmpKey = (int*)StackAlloc(iNumKeys * sizeof(int));
            for (i = 0; i < iNumKeys; i ++) {
                piEmpKey[i] = pvKey[i].GetInteger();
            }

            t_pConn->FreeData(pvKey);

            // Randomize empires on the map
            Algorithm::Randomize<int>(piEmpKey, iNumKeys);
        }

        // Add empires to map
        iErrCode = AddEmpiresToMap (iGameClass, iGameNumber, piEmpKey, iNumKeys, gfoFairness, &bFlag);
        if (iErrCode != OK) {
#ifdef _DEBUG
            Assert(false);
#endif
            // Abort: this is catastrophic
            int iErrCode2 = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
            Assert (iErrCode2 == OK);

            return ERROR_COULD_NOT_CREATE_PLANETS;
        }

        iErrCode = t_pConn->WriteOr (strGameData, GameData::State, GAME_MAP_GENERATED);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        iGameState |= GAME_MAP_GENERATED;
    }

    // Send some messages
    if (iCurrentNumEmpires > 1) {

        unsigned int iNumEmpires;
        Variant* pvEmpireKey = NULL;
        iErrCode = t_pConn->ReadColumn (
            strGameEmpires, 
            GameEmpires::EmpireKey, 
            &pvEmpireKey, 
            &iNumEmpires
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        // Insert rows into dip tables if DipExposed is true
        if (vGameClassOptions.GetInteger() & EXPOSED_DIPLOMACY) {

            Variant pvDiplomacy [GameEmpireDiplomacy::NumColumns] = {
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

            for (i = 0; i < iNumEmpires; i ++) {
                
                if (pvEmpireKey[i].GetInteger() != iEmpireKey) {
                    
                    pvDiplomacy[GameEmpireDiplomacy::iEmpireKey] = pvEmpireKey[i];
                    
                    // Insert iterated player into empire's table
                    iErrCode = t_pConn->InsertRow (strGameEmpireDiplomacy, GameEmpireDiplomacy::Template, pvDiplomacy, &iKey);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }
                    
                    GET_GAME_EMPIRE_DIPLOMACY (pszDiplomacy, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

                    // Insert empire into iterated player's table
                    pvDiplomacy[GameEmpireDiplomacy::iEmpireKey] = iEmpireKey;

                    iErrCode = t_pConn->InsertRow(pszDiplomacy, GameEmpireDiplomacy::Template, pvDiplomacy, &iKey);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }
                }
            }
        }

        if (bSendMessages) {
        
            // Broadcast player's entry, if more than one player in game
            if (vGameOptions.GetInteger() & GAME_NAMES_LISTED) {
                strMessage = BEGIN_STRONG;
                strMessage += vEmpireName.GetCharPtr();
                strMessage += END_STRONG " has joined the game";
            } else {
                strMessage = "A new empire has joined the game";
            }

            if (strMessage.GetCharPtr() == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto OnError;
            }

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
            
            for (i = 0; i < iNumEmpires; i ++) {

                if (pvEmpireKey[i].GetInteger() != iEmpireKey) {
                    
                    // Best effort
                    SendGameMessage (
                        iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), strMessage.GetCharPtr(), 
                        SYSTEM, MESSAGE_SYSTEM | MESSAGE_BROADCAST, NULL_TIME);
                }
            }
        }
        
        // Clean up
        t_pConn->FreeData(pvEmpireKey);
        pvEmpireKey = NULL;
    }

    // Get num updates
    iErrCode = t_pConn->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    *piNumUpdates = vNumUpdates.GetInteger();
    
    // We're done adding the empire
    iErrCode = t_pConn->WriteAnd (strGameData, GameData::State, ~GAME_ADDING_EMPIRE);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    if (GetGameClassName (iGameClass, pszGameClassName) != OK) {
        StrNCpy (pszGameClassName, "Unknown gameclass");
    }

    char pszUpdateReport [128 + MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    sprintf (
        pszUpdateReport,
        "%s entered %s %i",
        vEmpireName.GetCharPtr(),
        pszGameClassName,
        iGameNumber
        );

    global.GetReport()->WriteReport (pszUpdateReport);

    // Entry was sucessful  
    return OK;

OnError:

    // If we weren't called from CreateGame, we have a responsibility to clean up
    if (!bCreatingGame) {

        if ((iCurrentNumEmpires == 0 || (bAddedToGame && iCurrentNumEmpires == 1)) && (
            iErrCode != ERROR_GAME_DOES_NOT_EXIST && 
            iErrCode != ERROR_GAMECLASS_DOES_NOT_EXIST
            )) {

                // Best effort tear down game
                int iErrCode2 = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
                Assert (iErrCode2 == OK);

            } else {

                if (bAddedToGame) {

                    // Remove empire from game
                    int iErrCode2 = DeleteEmpireFromGame (
                        iGameClass,
                        iGameNumber,
                        iEmpireKey,
                        EMPIRE_GAME_ENTRY_ERROR,
                        NULL
                        );
                    Assert (iErrCode2 == OK);

                    // Keep game alive
                    iErrCode2 = t_pConn->WriteAnd (strGameData, GameData::State, ~GAME_ADDING_EMPIRE);
                    Assert (iErrCode2 == OK);
                }
            }
    }

    return iErrCode;
}


int GameEngine::SetEnterGameIPAddress (int iGameClass, int iGameNumber, int iEmpireKey, 
                                       const char* pszIPAddress) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return t_pConn->WriteData (
        strGameEmpireData,
        GameEmpireData::EnterGameIPAddress,
        pszIPAddress
        );
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *pbExist -> true iff game exists
//
// Determined if a certain game is the game lists, 0 if it isn't

int GameEngine::DoesGameExist (int iGameClass, int iGameNumber, bool* pbExist)
{
    GAME_DATA(pszGameData, iGameClass, iGameNumber);
    return t_pConn->DoesTableExist(pszGameData);
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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadData (pszGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode == OK) {
        *piNumUpdates = vNumUpdates.GetInteger();
    }

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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadData (pszGameData, GameData::NumUpdatesBeforeGameCloses, &vNumUpdates);
    if (iErrCode == OK) {
        *piNumUpdates = vNumUpdates.GetInteger();
    }

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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadData (pszGameData, GameData::Options, &vValue);
    if (iErrCode == OK) {
        *piOptions = vValue.GetInteger();
    }

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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadData (pszGameData, GameData::FirstUpdateDelay, &vValue);
    if (iErrCode == OK) {
        *psDelay = vValue.GetInteger();
    }

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

int GameEngine::GetNumEmpiresInGame (int iGameClass, int iGameNumber, int* piNumEmpires) {
    
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    return t_pConn->GetNumRows (strGameEmpires, (unsigned int*) piNumEmpires);
}


// Input:
// iGameClass -> Gameclass
// iGameNumber - > Gamenumber
//
// Output:
// *piNumDeadEmpires -> Number of dead empires in game
//
// Return the number of empires still remaining a given game

int GameEngine::GetNumDeadEmpiresInGame (int iGameClass, int iGameNumber, unsigned int* piNumDeadEmpires) {
    
    GAME_DEAD_EMPIRES(strGameDeadEmpires, iGameClass, iGameNumber);
    return t_pConn->GetNumRows(strGameDeadEmpires, piNumDeadEmpires);
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
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MinNumEmpires,
        &vNumEmpiresNeeded
        );

    if (iErrCode == OK) {
        *piNumEmpiresNeeded = vNumEmpiresNeeded.GetInteger();
    }

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
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData (strGameData, GameData::NumEmpiresUpdated, &vTemp);
    if (iErrCode == OK) {
        *piUpdatedEmpires = vTemp.GetInteger();
    }

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

int GameEngine::GetEmpiresInGame (int iGameClass, int iGameNumber, Variant** ppvEmpireKey, int* piNumEmpires) {

    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadColumn (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        ppvEmpireKey, 
        (unsigned int*) piNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData (pszGameData, GameData::State, &vTemp);

    if (iErrCode == OK) {
        *pbPaused = (vTemp.GetInteger() & PAUSED) || (vTemp.GetInteger() & ADMIN_PAUSED);
    }

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
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData (pszGameData, GameData::State, &vTemp);
    if (iErrCode == OK) {
        *pbAdminPaused = (vTemp.GetInteger() & ADMIN_PAUSED) != 0;
    }

    return iErrCode;
}

void GameEngine::GetGameClassGameNumber (const char* pszGameData, int* piGameClass, int* piGameNumber) {

    int iNum = sscanf (pszGameData, "%i.%i", piGameClass, piGameNumber);
    Assert (iNum == 2);
}

void GameEngine::GetGameClassGameNumber (int iGameClass, int iGameNumber, char* pszGameData) {

    sprintf (pszGameData, "%i.%i", iGameClass, iGameNumber);
}

void GameEngine::GetCoordinates (const char* pszCoord, int* piX, int* piY) {

    int iNum = sscanf (pszCoord, "%i,%i", piX, piY);
    Assert (iNum == 2);
}

void GameEngine::GetCoordinates (int iX, int iY, char* pszCoord) {

    sprintf (pszCoord, "%i,%i", iX, iY);
}


int GameEngine::CheckGameForEndConditions (int iGameClass, int iGameNumber, const char* pszAdminMessage, 
                                           bool* pbEndGame) {

    int iErrCode;
    unsigned int iEmpKey;
    Variant vEmpireKey;

    iErrCode = CheckGameForAllyOut (iGameClass, iGameNumber, pbEndGame);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (*pbEndGame) {
    
        // Ally out
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
        char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];

        iErrCode = GetGameClassName (iGameClass, pszGameClassName);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
        
        // Send victory message to all remaining empires
        sprintf (
            pszMessage, 
            "Congratulations! You have won %s %i", 
            pszGameClassName, 
            iGameNumber
            );
        
        iEmpKey = NO_KEY;
        while (true) {
            
            iErrCode = t_pConn->GetNextKey (strGameEmpires, iEmpKey, &iEmpKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            iErrCode = t_pConn->ReadData (strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            // Best effort send messages
            if (!String::IsBlank (pszAdminMessage)) {
                iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszAdminMessage, SYSTEM, MESSAGE_SYSTEM);
                Assert (iErrCode == OK);
            }
            
            iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
            Assert (iErrCode == OK);
            
            // Best effort update empires' statistics
            iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, vEmpireKey.GetInteger());
            Assert (iErrCode == OK);
        }

        // Best effort cleanup game
        iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_WIN);
        Assert (iErrCode == OK);

        goto Cleanup;
    }

    iErrCode = CheckGameForDrawOut (iGameClass, iGameNumber, pbEndGame);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    if (*pbEndGame) {

        // Draw out
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
        char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];

        iErrCode = GetGameClassName (iGameClass, pszGameClassName);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

        // Send draw message to all remaining empires
        sprintf (pszMessage, "You have drawn %s %i", pszGameClassName, iGameNumber);

        iEmpKey = NO_KEY;
        while (true) {
            
            iErrCode = t_pConn->GetNextKey (strGameEmpires, iEmpKey, &iEmpKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            iErrCode = t_pConn->ReadData (strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            // Best effort send messages
            if (!String::IsBlank (pszAdminMessage)) {
                iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszAdminMessage, SYSTEM, MESSAGE_SYSTEM);
                Assert (iErrCode == OK);
            }
            
            iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
            Assert (iErrCode == OK);

            // Best effort update empires' statistics
            iErrCode = UpdateScoresOnDraw (iGameClass, iGameNumber, vEmpireKey.GetInteger());
            Assert (iErrCode == OK);
        }
        
        // Best effort cleanup game
        iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_DRAW);
        Assert (iErrCode == OK);

        goto Cleanup;
    }

Cleanup:

    return iErrCode;
}


int GameEngine::CheckGameForAllyOut (int iGameClass, int iGameNumber, bool* pbAlly) {

    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    char pszDiplomacy [256];

    int iErrCode;
    unsigned int iNumRows, iNumEmpires, iKey, iEmpKey;
    Variant vEmpireKey, vDipStatus;

    bool bAlly = true;

    iErrCode = t_pConn->GetNumRows (strGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iEmpKey = NO_KEY;
    while (true) {

        iErrCode = t_pConn->GetNextKey (strGameEmpires, iEmpKey, &iEmpKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iErrCode = t_pConn->ReadData (strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        GET_GAME_EMPIRE_DIPLOMACY (pszDiplomacy, iGameClass, iGameNumber, vEmpireKey.GetInteger());

        iErrCode = t_pConn->GetNumRows (pszDiplomacy, &iNumRows);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iNumRows != iNumEmpires - 1) {
            bAlly = false;
            break;
        }

        iKey = NO_KEY;
        while (true) {

            iErrCode = t_pConn->GetNextKey (pszDiplomacy, iKey, &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            if (iErrCode != OK) {
                goto Cleanup;
            }

            iErrCode = t_pConn->ReadData (
                pszDiplomacy, 
                iKey, 
                GameEmpireDiplomacy::CurrentStatus, 
                &vDipStatus
                );

            if (iErrCode != OK) {
                goto Cleanup;
            }

            if (vDipStatus.GetInteger() != ALLIANCE) {
                bAlly = false;
                break;
            }
        }

        if (!bAlly) {
            break;
        }
    }

    *pbAlly = bAlly;

Cleanup:

    return iErrCode;
}

    
int GameEngine::CheckGameForDrawOut (int iGameClass, int iGameNumber, bool* pbDraw) {

    int iErrCode;
    unsigned int iNumEmpires;
    Variant vTemp;

    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    *pbDraw = false;

    unsigned int iRequesting;
    iErrCode = t_pConn->ReadData (strGameData, GameData::NumRequestingDraw, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iRequesting = vTemp.GetInteger();

    iErrCode = t_pConn->GetNumRows (strGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    Assert (iNumEmpires > 0);
    Assert (iRequesting <= iNumEmpires);
    if (iRequesting == iNumEmpires) {
        
        // Draw out iff someone isn't idle
        bool bIdle;
        iErrCode = AreAllEmpiresIdle (iGameClass, iGameNumber, &bIdle);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        *pbDraw = !bIdle;
    }

Cleanup:

    return iErrCode;
}

int GameEngine::AreAllEmpiresIdle (int iGameClass, int iGameNumber, bool* pbIdle) {

    int iErrCode;
    Variant vTemp;

    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    *pbIdle = true;

    // Figure out idle policy
    iErrCode = GetGameClassProperty (iGameClass, SystemGameClassData::NumUpdatesForIdle, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    unsigned int iNumUpdatesForIdle = vTemp.GetInteger();

    unsigned int iKey = NO_KEY;
    while (true) {

        iErrCode = t_pConn->GetNextKey (strGameEmpires, iKey, &iKey);
        if (iErrCode != OK) {
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pConn->ReadData (strGameEmpires, iKey, GameEmpires::EmpireKey, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        int iEmpireKey = vTemp.GetInteger();

        int iEmpireOptions;
        iErrCode = GetEmpireOptions (iGameClass, iGameNumber, iEmpireKey, &iEmpireOptions);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iEmpireOptions & RESIGNED) {
            continue;
        }

        char pszGameEmpireData [256];
        GET_GAME_EMPIRE_DATA (pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

        iErrCode = t_pConn->ReadData (pszGameEmpireData, GameEmpireData::NumUpdatesIdle, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        unsigned int iNumUpdatesIdle = vTemp.GetInteger();

        if (iNumUpdatesIdle >= iNumUpdatesForIdle) {
            continue;
        }

        // A non-idle empire has been found
        *pbIdle = false;
        break;
    }

Cleanup:

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

int GameEngine::PauseGameInternal (int iGameClass, int iGameNumber, const UTCTime& tNow, 
                                   bool bAdmin, bool bBroadcast) {

    Variant vTemp;
    int iErrCode, iState;

    bool bFlag;
    const char* pszMessage;

    IWriteTable* pGameData = NULL;
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Make sure game exists
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!bFlag) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    // Get update period
    iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    Seconds sUpdatePeriod = vTemp.GetInteger();

    int iGameClassOptions;
    iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Can only pause games that have started
    iErrCode = GetGameState (iGameClass, iGameNumber, &iState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!(iState & STARTED)) {
        return OK;
    }

    iErrCode = t_pConn->GetTableForWriting (strGameData, &pGameData);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Read the state again - it's cheap, and the state may have changed
    iErrCode = pGameData->ReadData (GameData::State, &iState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    Assert (iState & STARTED);

    if (bAdmin && !(iState & ADMIN_PAUSED)) {

        iErrCode = pGameData->WriteOr (GameData::State, ADMIN_PAUSED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Might already be paused
    if (iState & PAUSED) {
        goto Cleanup;
    }

    int iNumUpdates;
    iErrCode = pGameData->ReadData (GameData::NumUpdates, &iNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    Seconds sFirstUpdateDelay = 0;
    if (iNumUpdates == 0) {

        int iTemp;
        iErrCode = pGameData->ReadData (GameData::FirstUpdateDelay, &iTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        sFirstUpdateDelay = iTemp;
    }

    UTCTime tLastUpdateTime;
    iErrCode = pGameData->ReadData (GameData::LastUpdateTime, &tLastUpdateTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    bool bWeekends = (iGameClassOptions & WEEKEND_UPDATES) != 0;
    Seconds sAfterWeekendDelay = 0;
    if (!bWeekends) {

        iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::AfterWeekendDelay, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        sAfterWeekendDelay = vTemp.GetInteger();
    }

    //
    // Pause the game
    //

    iErrCode = pGameData->WriteOr (GameData::State, PAUSED);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

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
    iErrCode = pGameData->WriteData (GameData::SecondsUntilNextUpdateWhilePaused, sSecondsUntil);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pGameData);

    // Best effort broadcast message
    if (bBroadcast) {
        
        pszMessage = bAdmin ? "The game was paused by an administrator" : "The game is now paused";

        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        Assert (iErrCode == OK);
    }

Cleanup:

    SafeRelease (pGameData);

    return iErrCode;
}

int GameEngine::UnpauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) {

    int iErrCode, iTemp;
    const char* pszMessage;
    Variant vTemp;

    IWriteTable* pGameData = NULL;

    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    // Get gameclass update options
    int iGameClassOptions;
    iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Assumption - the number of empires in the game can't change while we're here
    // This is because we only call this while holding a read lock on the game
    unsigned int iNumEmpires;
    iErrCode = t_pConn->GetNumRows (strEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Get update period
    Seconds sUpdatePeriod = 0;
    iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    sUpdatePeriod = vTemp.GetInteger();

    iErrCode = t_pConn->GetTableForWriting (strGameData, &pGameData);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    int iState;
    iErrCode = pGameData->ReadData (GameData::State, &iState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(iState & PAUSED)) {
        goto Cleanup;
    }

    if (bAdmin) {

        if (!(iState & ADMIN_PAUSED)) {
            goto Cleanup;
        }

        unsigned int iNumRequestingPause;
        iErrCode = pGameData->ReadData (GameData::NumRequestingPause, (int*) &iNumRequestingPause);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Try to unpause the game
        iErrCode = pGameData->WriteAnd (GameData::State, ~ADMIN_PAUSED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iNumEmpires == iNumRequestingPause) {

            // Still paused
            iErrCode = pGameData->WriteOr (GameData::State, PAUSED);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            goto Cleanup;
        }

        pszMessage = "The game was unpaused by an administrator";

        // Fall through to unpause

    } else {

        pszMessage = "The game is no longer paused";
    }

    // Get num updates
    int iNumUpdates;
    iErrCode = pGameData->ReadData (GameData::NumUpdates, &iNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get first update delay
    Seconds sFirstUpdateDelay = 0;
    if (iNumUpdates == 0) {

        iErrCode = pGameData->ReadData (GameData::FirstUpdateDelay, &iTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        sFirstUpdateDelay = iTemp;
    }

    // Get seconds until next update when game was paused
    Seconds sSecondsUntilNextUpdate;

    iErrCode = pGameData->ReadData (GameData::SecondsUntilNextUpdateWhilePaused, &iTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
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

    iErrCode = pGameData->WriteData (GameData::LastUpdateTime, tNewLastUpdateTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // No longer paused
    iErrCode = pGameData->WriteAnd (GameData::State, ~PAUSED);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pGameData);

    // Best effort broadcast message
    if (bBroadcast) {
        BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
    }

Cleanup:

    SafeRelease (pGameData);

    return iErrCode;
}

int GameEngine::PauseAllGames() {

    unsigned int i, iNumKeys;
    int iGameClass, iGameNumber;
    Variant* pvGame;

    bool bExists;
    
    int iErrCode = t_pConn->ReadColumn (
        SYSTEM_ACTIVE_GAMES,
        SystemActiveGames::GameClassGameNumber,
        &pvGame,
        &iNumKeys
        );
    
    if (iErrCode == OK) {
        
        for (i = 0; i < iNumKeys; i ++) {
            
            GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

            // Flush remaining updates
            iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, true, &bExists);

            // Best effort pause the game
            if (iErrCode == OK && DoesGameExist (iGameClass, iGameNumber, &bExists) == OK && bExists)
            {
                iErrCode = PauseGame (iGameClass, iGameNumber, true, true);
                Assert (iErrCode == OK);
            }
        }
        
        t_pConn->FreeData(pvGame);
    
    } else {
        
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert (false);
    }

    return iErrCode;
}


int GameEngine::UnpauseAllGames() {

    unsigned int i, iNumKeys;
    int iGameClass, iGameNumber;
    Variant* pvGame;
    
    int iErrCode = t_pConn->ReadColumn (
        SYSTEM_ACTIVE_GAMES,
        SystemActiveGames::GameClassGameNumber,
        &pvGame,
        &iNumKeys
        );
    
    if (iErrCode == OK) {
        
        for (i = 0; i < iNumKeys; i ++)
        {
            GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

            // Best effort
            iErrCode = UnpauseGame (iGameClass, iGameNumber, true, true);
            Assert (iErrCode == OK);
        }
        
        t_pConn->FreeData(pvGame);
    
    } else {
        
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert (false);
    }

    return iErrCode;
}

int GameEngine::LogEmpireIntoGame (int iGameClass, int iGameNumber, int iEmpireKey, int* piIdleUpdates) {

    int iErrCode;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    IWriteTable* pTable = NULL;

    UTCTime tTime;
    Time::GetTime (&tTime);
    
    // Update LastLogin, NumUpdatesIdle, IP Address
    iErrCode = t_pConn->GetTableForWriting (strGameEmpireData, &pTable);
    if (iErrCode != OK) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    // Set last login
    iErrCode = pTable->WriteData (GameEmpireData::LastLogin, tTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Set logged in this update
    iErrCode = pTable->WriteOr (GameEmpireData::Options, LOGGED_IN_THIS_UPDATE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Read idle updates
    iErrCode = pTable->ReadData (GameEmpireData::NumUpdatesIdle, piIdleUpdates);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pTable);

    return iErrCode;
}

int GameEngine::RuinGame (int iGameClass, int iGameNumber, const char* pszWinnerName) {

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    int iErrCode;

    // Get game class name
    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        Assert (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST);
        return iErrCode;
    }

    // Prepare ruin message for all remaining empires
    char pszMessage[128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    sprintf (pszMessage, "You ruined out of %s %i", pszGameClassName, iGameNumber);

    // Get empires
    Variant* pvEmpireKey = NULL;
    unsigned int i, iNumEmpires;

    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadColumn (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        &pvEmpireKey, 
        &iNumEmpires
        );

    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        goto Cleanup;
    }

    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = RuinEmpire (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), pszMessage);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    for (i = 0; i < iNumEmpires; i ++) {

        // Best effort
        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), EMPIRE_RUINED, NULL);
        Assert (iErrCode == OK);
    }

Cleanup:

    if (pvEmpireKey != NULL) {
        t_pConn->FreeData(pvEmpireKey);
    }

    // Kill the game
    iErrCode = CleanupGame (
        iGameClass,
        iGameNumber,
        pszWinnerName == NULL ? GAME_RESULT_RUIN : GAME_RESULT_WIN, 
        pszWinnerName
        );

    Assert (iErrCode == OK);

    return iErrCode;
}


int GameEngine::ResignGame (int iGameClass, int iGameNumber) {

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    int iErrCode;

    // Get game class name
    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        Assert (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST);
        return iErrCode;
    }

    // Prepare resignation message for all remaining empires
    char pszMessage[128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    sprintf (pszMessage, "You resigned out of %s %i", pszGameClassName, iGameNumber);

    // Get empires
    Variant* pvEmpireKey = NULL;
    unsigned int i, iNumEmpires;

    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadColumn (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        &pvEmpireKey, 
        &iNumEmpires
        );

    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        goto Cleanup;
    }

    for (i = 0; i < iNumEmpires; i ++) {

        int iEmpireKey = pvEmpireKey[i].GetInteger();

        // Best effort
        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_RESIGNED, NULL);
        Assert (iErrCode == OK);
    }

Cleanup:

    if (pvEmpireKey != NULL) {
        t_pConn->FreeData(pvEmpireKey);
    }

    // Kill the game
    iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE, NULL);
    Assert (iErrCode == OK);

    return iErrCode;
}

int GameEngine::GetResignedEmpiresInGame (int iGameClass, int iGameNumber, int** ppiEmpireKey, 
                                          int* piNumResigned) {

    int iErrCode, * piEmpireKey = NULL, iNumResigned = 0;
    bool bResigned;

    unsigned int i, iNumEmpires;
    Variant* pvEmpireKey = NULL;

    *ppiEmpireKey = NULL;
    *piNumResigned = 0;

    char pszTable [256];

    GET_GAME_DATA (pszTable, iGameClass, iGameNumber);

    Variant vResigned;
    iErrCode = t_pConn->ReadData (pszTable, GameData::NumEmpiresResigned, &vResigned);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vResigned.GetInteger() == 0) {
        goto Cleanup;
    }

    GET_GAME_EMPIRES (pszTable, iGameClass, iGameNumber);

    // Get empires
    iErrCode = t_pConn->ReadColumn (
        pszTable,
        GameEmpires::EmpireKey,
        &pvEmpireKey,
        &iNumEmpires
        );

    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Output
    piEmpireKey = new int [iNumEmpires];
    if (piEmpireKey == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = HasEmpireResignedFromGame (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), &bResigned);
        if (iErrCode != OK) {
            goto Cleanup;
        }
            
        if (bResigned) {
            piEmpireKey[iNumResigned ++] = pvEmpireKey[i].GetInteger();
        }
    }

    Assert (iNumResigned == vResigned.GetInteger());

    *ppiEmpireKey = piEmpireKey;
    *piNumResigned = iNumResigned;

Cleanup:

    if (pvEmpireKey != NULL) {
        t_pConn->FreeData(pvEmpireKey);
    }

    if (iErrCode != OK && piEmpireKey != NULL) {
        delete [] piEmpireKey;
    }

    return iErrCode;

}


int GameEngine::GetBridierRankPotentialGainLoss (int iGameClass, int iGameNumber, int iEmpireKey, 
                                                 int* piGain, int* piLoss) {

    int iErrCode, iEmpireRank = 0, iEmpireIndex = 0, iFoeRank = 0, iFoeIndex = 0, iTemp;
    unsigned int iNumEmpires;

    Variant v0Key, v1Key, vRank, vIndex;
    char pszGameData [256];

    *piGain = 10;
    *piLoss = -10;

    GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

#ifdef _DEBUG

    int iOptions;
    iErrCode = GetGameOptions (iGameClass, iGameNumber, &iOptions);
    if (iErrCode == OK) {
        Assert (iOptions & GAME_COUNT_FOR_BRIDIER);
    }

#endif

    iErrCode = t_pConn->GetNumRows (pszGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    Assert (iNumEmpires <= 2);

    // Read 1st empire
    iErrCode = t_pConn->ReadData (pszGameEmpires, 0, GameEmpires::EmpireKey, &v0Key);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (iNumEmpires == 1 && v0Key.GetInteger() == iEmpireKey) {

        // We're alone in the game
        *piGain = -1;
        *piLoss = -1;
        goto Cleanup;
    }

    GET_GAME_EMPIRE_DATA (pszGameData, iGameClass, iGameNumber, v0Key.GetInteger());
    
    iErrCode = t_pConn->ReadData (pszGameData, GameEmpireData::InitialBridierRank, &vRank);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    iErrCode = t_pConn->ReadData (pszGameData, GameEmpireData::InitialBridierIndex, &vIndex);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (v0Key.GetInteger() == iEmpireKey) {

        iEmpireRank = vRank.GetInteger();
        iEmpireIndex = vIndex.GetInteger();
    
    } else {

        iFoeRank = vRank.GetInteger();
        iFoeIndex = vIndex.GetInteger();
    }

    // Read 2nd empire
    if (iNumEmpires == 2) {

        iErrCode = t_pConn->ReadData (pszGameEmpires, 1, GameEmpires::EmpireKey, &v1Key);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (v0Key.GetInteger() != iEmpireKey && v1Key.GetInteger() != iEmpireKey) {

            // Someone beat us into the game
            *piGain = -1;
            *piLoss = -1;
            goto Cleanup;
        }
        
        GET_GAME_EMPIRE_DATA (pszGameData, iGameClass, iGameNumber, v1Key.GetInteger());
        
        iErrCode = t_pConn->ReadData (pszGameData, GameEmpireData::InitialBridierRank, &vRank);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = t_pConn->ReadData (pszGameData, GameEmpireData::InitialBridierIndex, &vIndex);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        if (v1Key.GetInteger() == iEmpireKey) {
            
            iEmpireRank = vRank.GetInteger();
            iEmpireIndex = vIndex.GetInteger();
            
        } else {
            
            iFoeRank = vRank.GetInteger();
            iFoeIndex = vIndex.GetInteger();
        }
    
    } else {

        Assert (v0Key.GetInteger() != iEmpireKey);

        iErrCode = GetBridierScore (iEmpireKey, &iEmpireRank, &iEmpireIndex);
        if (iErrCode != OK) {
            goto Cleanup;
        }
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

Cleanup:

    return iErrCode;
}

int GameEngine::IsSpectatorGame (int iGameClass, int iGameNumber, bool* pbSpectatorGame) {

    int iErrCode;
    Variant vGameOptions;

    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadData (pszGameData, GameData::Options, &vGameOptions);
    if (iErrCode != OK) {
        return iErrCode;
    }

    *pbSpectatorGame = (vGameOptions.GetInteger() & GAME_ALLOW_SPECTATORS) != 0;
    return iErrCode;
}

int GameEngine::AddToLatestGames (const Variant* pvColumns, unsigned int iTournamentKey) {

    int iErrCode = OK;

    if (iTournamentKey != NO_KEY)
    {
        SYSTEM_TOURNAMENT_LATEST_GAMES (pszGames, iTournamentKey);
        iErrCode = AddToLatestGames(pszGames, SystemTournamentLatestGames::Template, pvColumns);
    }

    return iErrCode != OK ? iErrCode : AddToLatestGames (SYSTEM_LATEST_GAMES, SystemLatestGames::Template, pvColumns);
}

int GameEngine::AddToLatestGames(const char* pszTable, const TemplateDescription& ttTemplate, const Variant* pvColumns) {

    int iErrCode;
    IWriteTable* pGames = NULL;

    unsigned int iNumRows;
    Variant vGames;

    // Read limit
    iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::NumGamesInLatestGameList, &vGames);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Lock table
    iErrCode = t_pConn->GetTableForWriting (pszTable, &pGames);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGames->GetNumRows (&iNumRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iNumRows == (unsigned int) vGames.GetInteger()) {

        UTCTime tEnded, tOldestTime;
        unsigned int iKey = NO_KEY, iOldestKey = NO_KEY;

        Time::GetTime (&tOldestTime);
        Time::AddSeconds (tOldestTime, ONE_YEAR_IN_SECONDS, &tOldestTime);

        // Delete oldest game from table
        while (true) {

            iErrCode = pGames->GetNextKey (iKey, &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                break;
            }

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = pGames->ReadData (iKey, SystemLatestGames::Ended, &tEnded);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (Time::OlderThan (tEnded, tOldestTime)) {
                tOldestTime = tEnded;
                iOldestKey = iKey;
            }
        }

        if (iOldestKey != NO_KEY) {

            // Delete the oldest game
            iErrCode = pGames->DeleteRow(iOldestKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    // Finally, insert the new row
    iErrCode = pGames->InsertRow(ttTemplate, pvColumns, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pGames);

    return iErrCode;
}

int GameEngine::GetNumEmpiresInGames (unsigned int* piNumEmpires) {

    int iErrCode;

    IReadTable* pGames = NULL;
    unsigned int iKey = NO_KEY, iEmpireKey;

    char pszGameEmpires [256];
    Variant vTemp;

    HashTable<unsigned int, unsigned int, GenericHashValue<unsigned int>, GenericEquals <unsigned int> > 
        htEmpires (NULL, NULL);

    if (!htEmpires.Initialize (250)) {
        return ERROR_OUT_OF_MEMORY;
    }

    while (true) {

        Variant vGame;
        int iGameClass, iGameNumber;

        iErrCode = t_pConn->GetTableForReading (SYSTEM_ACTIVE_GAMES, &pGames);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iErrCode = pGames->GetNextKey (iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iErrCode = pGames->ReadData (iKey, SystemActiveGames::GameClassGameNumber, &vGame);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        SafeRelease (pGames);

        GetGameClassGameNumber(vGame.GetCharPtr(), &iGameClass, &iGameNumber);

        unsigned int iProxyKey = NO_KEY;
        GET_GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

        while (true) {

            iErrCode = t_pConn->GetNextKey (pszGameEmpires, iProxyKey, &iProxyKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }
            if (iErrCode != OK) {
                break;
            }

            iErrCode = t_pConn->ReadData (pszGameEmpires, iProxyKey, GameEmpires::EmpireKey, &vTemp);
            if (iErrCode != OK) {
                break;
            }
            iEmpireKey = vTemp.GetInteger();

            if (!htEmpires.FindFirst (iEmpireKey, (unsigned int*) NULL)) {

                if (!htEmpires.Insert (iEmpireKey, iEmpireKey)) {
                    iErrCode = ERROR_OUT_OF_MEMORY;
                    break;
                }
            }
        }

        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    SafeRelease (pGames);

    *piNumEmpires = htEmpires.GetNumElements();

Cleanup:

    SafeRelease (pGames);

    return iErrCode;
}

int GameEngine::GetNumRecentActiveEmpiresInGames(unsigned int* piNumEmpires)
{
    // TODOTODO - Need different way to solve this
    *piNumEmpires = 0;
    return OK;
}