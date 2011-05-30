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

#include <math.h>

#include "GameEngine.h"

#include "../Scoring/BridierScore.h"

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

    bool bExists;
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bExists);
    if (iErrCode != OK || !bExists) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = m_pGameData->ReadColumn (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        &pvEmpireKey, 
        &iNumEmpires
        );

    // Tolerance...
    if (iErrCode != OK) {
        Assert (false);
        iNumEmpires = 0;
        iErrCode = OK;
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
        SendSystemMessage (pvEmpireKey[i], pszTemp, SYSTEM);
    }

    // Best effort send the message from the admin  
    if (!String::IsBlank (pszMessage)) {
        for (i = 0; i < iNumEmpires; i ++) {
            SendSystemMessage (pvEmpireKey[i], pszMessage, iEmpireKey);
        }
    }

Cleanup:

    // Clean up
    if (pvEmpireKey != NULL) {
        m_pGameData->FreeData (pvEmpireKey);
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
    if (m_pUIEventSink != NULL) {
        m_pUIEventSink->OnCleanupGame (iGameClass, iGameNumber);
    }

    //
    // Need to be tolerant of errors...
    //

    // Get game state
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vGameState);
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

    iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_DELETING);
    Assert (iErrCode == OK);

    // Delete all remaining empires from the game
    iKey = NO_KEY;
    while (true) {

        iErrCode = m_pGameData->GetNextKey (strGameEmpires, iKey, &iKey);
        if (iErrCode != OK) {   
            Assert (iErrCode == ERROR_DATA_NOT_FOUND);
            break;
        }

        iErrCode = m_pGameData->ReadData (strGameEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
        if (iErrCode == OK) {

            iErrCode = m_pGameData->ReadData (
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
    if (m_scConfig.bReport) {

        char* pszMessage = (char*) StackAlloc (strList.GetLength() + 80 + MAX_FULL_GAME_CLASS_NAME_LENGTH);
        
        sprintf (
            pszMessage,
            "%s %i ended with the following empires still alive: %s",
            pszGameClass,
            iGameNumber,
            strList.GetCharPtr() == NULL ? "" : strList.GetCharPtr()
            );
        
        m_pReport->WriteReport (pszMessage);
    }

    // Add to latest games
    if (vGameState.GetInteger() & STARTED) {

        Variant pvLatestGame [SystemLatestGames::NumColumns];

        UTCTime tNow;
        Time::GetTime (&tNow);

        // Name
        pvLatestGame[SystemLatestGames::Name] = pszGameClass;

        // Number
        pvLatestGame[SystemLatestGames::Number] = iGameNumber;

        // Created
        iErrCode = m_pGameData->ReadData (
            strGameData, 
            GameData::CreationTime, 
            pvLatestGame + SystemLatestGames::Created
            );

        if (iErrCode != OK) {
            pvLatestGame[SystemLatestGames::Created] = tNow;
        }

        // Ended
        pvLatestGame[SystemLatestGames::Ended] = tNow;

        // Updates
        iErrCode = m_pGameData->ReadData (
            strGameData, 
            GameData::NumUpdates, 
            pvLatestGame + SystemLatestGames::Updates
            );

        if (iErrCode != OK) {
            pvLatestGame[SystemLatestGames::Updates] = 0;
        }

        // Result
        pvLatestGame[SystemLatestGames::Result] = (int) grResult;

        // Winner list
        pvLatestGame[SystemLatestGames::Winners] = strList;

        // Loser list
        iKey = NO_KEY;
        strList.Clear();

        while (true) {
            
            iErrCode = m_pGameData->GetNextKey (strGameDeadEmpires, iKey, &iKey);
            if (iErrCode != OK) {
                Assert (iErrCode == ERROR_DATA_NOT_FOUND);
                break;
            }
            
            iErrCode = m_pGameData->ReadData (strGameDeadEmpires, iKey, GameDeadEmpires::Name, &vName); 
            if (iErrCode == OK) {

                if (!strList.IsBlank()) {
                    strList += ", ";
                }
                strList += vName.GetCharPtr();
            }
        }

        pvLatestGame[SystemLatestGames::Losers] = strList;

        iErrCode = AddToLatestGames (pvLatestGame, iTournamentKey);
        Assert (iErrCode == OK);
    }


    ////////////////////////////////////////
    // Best effort delete all game tables //
    ////////////////////////////////////////

    char pszTable [512];

    // GameEmpires(I.I)
    iErrCode = m_pGameData->DeleteTable (strGameEmpires);
    Assert (iErrCode == OK);

    // GameMap(I.I)
    GET_GAME_MAP (pszTable, iGameClass, iGameNumber);
    iErrCode = m_pGameData->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    // GameIndependentShips(I.I)
    Variant vGameOptions, vGameClassOptions;

    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vGameClassOptions);
    if (iErrCode == OK) {
        
        if (vGameClassOptions.GetInteger() & INDEPENDENCE) {

            GET_GAME_INDEPENDENT_SHIPS (pszTable, iGameClass, iGameNumber);

            iErrCode = m_pGameData->DeleteTable (pszTable);
            Assert (iErrCode == OK);
        }
    }

    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::TournamentKey, &vTournamentKey);
    if (iErrCode != OK) {
        Assert (false);
        vTournamentKey = NO_KEY;
    }

    // GameSecurity(I.I)
    iErrCode = m_pGameData->ReadData (strGameData, GameData::Options, &vGameOptions);
    if (iErrCode == OK) {

        if (vGameOptions.GetInteger() & GAME_ENFORCE_SECURITY) {

            GET_GAME_SECURITY (pszTable, iGameClass, iGameNumber);

            iErrCode = m_pGameData->DeleteTable (pszTable);
            Assert (iErrCode == OK);
        }
    }

    // GameDeadEmpires(I.I)
    GET_GAME_DEAD_EMPIRES (pszTable, iGameClass, iGameNumber);
    iErrCode = m_pGameData->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    // GameData(I.I)
    iErrCode = m_pGameData->DeleteTable (strGameData);
    Assert (iErrCode == OK);

    // Delete game from active game list
    char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
    GetGameClassGameNumber (iGameClass, iGameNumber, pszData);

    iErrCode = m_pGameData->GetFirstKey (
        SYSTEM_ACTIVE_GAMES,
        SystemActiveGames::GameClassGameNumber,
        pszData,
        false,
        &iKey
        );

    if (iErrCode == OK && iKey != NO_KEY) {
        iErrCode = m_pGameData->DeleteRow (SYSTEM_ACTIVE_GAMES, iKey);
        Assert (iErrCode == OK);
    }

    else iErrCode = ERROR_GAME_DOES_NOT_EXIST;

    // Delete game from tournament active game list
    if (iTournamentKey != NO_KEY) {

        SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszGames, iTournamentKey);

        iErrCode = m_pGameData->GetFirstKey (
            pszGames,
            SystemTournamentActiveGames::GameClassGameNumber,
            pszData,
            false,
            &iKey
            );

        if (iErrCode == OK && iKey != NO_KEY) {

            iErrCode = m_pGameData->DeleteRow (pszGames, iKey);
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

    // Remove game from game table
    int iErrCode2 = RemoveFromGameTable (iGameClass, iGameNumber);
    Assert (iErrCode2 == OK);

    // Decrement number of games in gameclass
    iErrCode2 = m_pGameData->Increment (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::NumActiveGames,
        -1
        );
    Assert (iErrCode2 == OK);

    // Best effort attempt to delete gameclass if it's marked for deletion
    if (vGameClassOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION) {
    
        bool bDeleted;
        int iErrCode2 = DeleteGameClass (iGameClass, &bDeleted);
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

    int iErrCode = m_pGameData->ReadData (pszGameData, GameData::CreationTime, &vTime);

    if (iErrCode == OK) {
        *ptCreationTime = vTime.GetUTCTime();
    }

    return iErrCode;
}


int GameEngine::GetGameState (int iGameClass, int iGameNumber, int* piGameState) {

    Variant vValue;
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    int iErrCode = m_pGameData->ReadData (pszGameData, GameData::State, &vValue);

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

    return m_pGameData->GetNumRows (SYSTEM_ACTIVE_GAMES, (unsigned int*) piNumGames);
}


// Output:
// *piNumGames -> Number of games
//
// Return the current number of open games on the server

int GameEngine::GetNumOpenGames (int* piNumGames) {

    int iErrCode = m_pGameData->GetEqualKeys (
        SYSTEM_ACTIVE_GAMES, 
        SystemActiveGames::State, 
        STILL_OPEN, 
        false, 
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

    int iErrCode = m_pGameData->GetEqualKeys (
        SYSTEM_ACTIVE_GAMES, 
        SystemActiveGames::State, 
        0, 
        false, 
        NULL,
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
    int iErrCode = m_pGameData->ReadColumn (
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

        m_pGameData->FreeData (pvGames);
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

    iErrCode = m_pGameData->GetTableForReading (SYSTEM_ACTIVE_GAMES, &pGames);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = pGames->GetEqualKeys (
        SystemActiveGames::State,
        bOpen ? STILL_OPEN : 0,
        false,
        &piKey,
        &iNumKeys
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    else if (iErrCode == OK) {

        const char* pszGame;
        
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
        
        for (i = 0; i < iNumKeys; i ++) {
            
            iErrCode = pGames->ReadData (piKey[i], SystemActiveGames::GameClassGameNumber, &pszGame);
            if (iErrCode == OK) {

                GetGameClassGameNumber (
                    pszGame, 
                    (*ppiGameClass) + *piNumGames, 
                    (*ppiGameNumber) + *piNumGames
                    );

                (*piNumGames) ++;
            }
        }
    }

Cleanup:

    SafeRelease (pGames);

    if (piKey != NULL) {
        m_pGameData->FreeKeys (piKey);
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

    int iErrCode = m_pGameData->ReadData (pszGameData, GameData::State, &vOpen);

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

    int iErrCode = m_pGameData->ReadData (pszGameData, GameData::State, &vStarted);
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

    int iErrCode = m_pGameData->ReadData (pszGameData, GameData::Password, &vPassword);

    if (iErrCode == OK) {
        *pbProtected = !String::IsBlank (vPassword.GetCharPtr());
    }
    
    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
//
// Output:
// *pvPassword -> The game's password; blank if there is none
//
// Return a game's password

int GameEngine::GetGamePassword (int iGameClass, int iGameNumber, Variant* pvPassword) {

    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    return m_pGameData->ReadData (pszGameData, GameData::Password, pvPassword);
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// pszPassword -> The game's new password; can be blank
//
// Change a game's password

int GameEngine::SetGamePassword (int iGameClass, int iGameNumber, const char* pszNewPassword) {

    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    return m_pGameData->WriteData (pszGameData, GameData::Password, pszNewPassword);
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

    bool bGameClassLocked = false, bEmpireLocked = false, bIncrementedActiveGameCount = false, 
        bDeleteRequired = false, bGameLocked = true;

    char strTableName [256], strGameData [256];

    UTCTime tTime;
    Time::GetTime (&tTime);

    const unsigned int* piEmpireKey = goGameOptions.piEmpireKey;
    const unsigned int iNumEmpires = goGameOptions.iNumEmpires;

    Assert (iEmpireCreator != TOURNAMENT || (piEmpireKey != NULL && iNumEmpires > 0));

    // Make sure new game creation is enabled
    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!(vTemp.GetInteger() & NEW_GAMES_ENABLED)) {
        return ERROR_DISABLED;
    }

    // Lock gameclass
    NamedMutex nmGameClassMutex;
    NamedMutex* pnmEmpireMutex = (NamedMutex*) StackAlloc (iNumEmpires * sizeof (NamedMutex));
    
    int* piCopy = (int*) StackAlloc (iNumEmpires * sizeof (int));
    memcpy (piCopy, piEmpireKey, iNumEmpires * sizeof (int));
    Algorithm::QSortAscending<int> (piCopy, iNumEmpires);

    // Lock empires before gameclass - prevent deadlock by sorting empire list
    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = LockEmpire (piCopy[i], pnmEmpireMutex + i);
        if (iErrCode != OK) {
            Assert (false);
            for (j = i - 1; j < i; j --) {
                UnlockEmpire (pnmEmpireMutex + j);
            }
            return iErrCode;
        }
    }
    bEmpireLocked = true;

    iErrCode = LockGameClass (iGameClass, &nmGameClassMutex);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    bGameClassLocked = true;

    // Test for gameclass halt
    iErrCode = m_pGameData->ReadData (
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
    iErrCode = m_pGameData->ReadData (
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

        iErrCode = m_pGameData->ReadData (
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
    iErrCode = m_pGameData->Increment (
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
        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, piEmpireKey[i], SystemEmpireData::Options, &vHalted);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        if (vHalted.GetInteger() & EMPIRE_MARKED_FOR_DELETION) {
            iErrCode = ERROR_EMPIRE_IS_HALTED;
            goto OnError;
        }

        // Make sure empire is at least a novice
        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, piEmpireKey[i], SystemEmpireData::Privilege, &vPrivilege);
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
    iErrCode = m_pGameData->Increment (
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
        
        if (pvActiveGameData[SystemActiveGames::GameClassGameNumber].GetCharPtr() == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto OnError;
        }

        // Add row to open games list
        iErrCode = m_pGameData->InsertRow (SYSTEM_ACTIVE_GAMES, pvActiveGameData);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        UnlockGameClass (nmGameClassMutex);
        bGameClassLocked = false;

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

            iErrCode = m_pGameData->InsertRow (pszGames, pvActiveGameData);
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

    iErrCode = m_pGameData->CreateTable (strGameData, GameData::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

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
        };

        iErrCode = m_pGameData->InsertRow (strGameData, pvGameData);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

    }   // Scope

    bDeleteRequired = true;

    // Create GameSecurity(I.I) table if necessary
    if (goGameOptions.iOptions & GAME_ENFORCE_SECURITY) {

        unsigned int i;

        Assert (goGameOptions.iOptions > 0);

        GET_GAME_SECURITY (strTableName, iGameClass, *piGameNumber);

        iErrCode = m_pGameData->CreateTable (strTableName, GameSecurity::Template.Name);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        // Populate rows
        Variant pvGameSec [GameSecurity::NumColumns];

        for (i = 0; i < goGameOptions.iNumSecurityEntries; i ++) {

            int64 i64SessionId = 0;
            unsigned int iRowEmpireKey = goGameOptions.pSecurity[i].iEmpireKey;

            // Lock empire
            NamedMutex nmLock;
            iErrCode = LockEmpire (iRowEmpireKey, &nmLock);
            if (iErrCode != OK) {
                Assert (false);
                continue;
            }

            // Get empire name
            iErrCode = GetEmpireName (iRowEmpireKey, pvGameSec + GameSecurity::EmpireName);
            if (iErrCode == OK) {

                // Check name - ignore if not matching
                if (stricmp (
                    pvGameSec[GameSecurity::EmpireName].GetCharPtr(), 
                    goGameOptions.pSecurity[i].pszEmpireName
                    ) != 0) {
                    iErrCode = ERROR_FAILURE;
                } else {

                    // Get empire ip address and session id
                    iErrCode = GetEmpireIPAddress (iRowEmpireKey, pvGameSec + GameSecurity::IPAddress);
                    if (iErrCode == OK) {
                        iErrCode = GetEmpireSessionId (iRowEmpireKey, &i64SessionId);
                    }
                }
            }

            // Unlock empire
            UnlockEmpire (nmLock);

            if (iErrCode != OK) {
                continue;
            }

            // Set remaining columns
            pvGameSec [GameSecurity::EmpireKey] = iRowEmpireKey;
            pvGameSec [GameSecurity::Options] = goGameOptions.pSecurity[i].iOptions;
            pvGameSec [GameSecurity::SessionId] = i64SessionId;

            // Insert row
            iErrCode = m_pGameData->InsertRow (strTableName, pvGameSec);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
        }
    }

    // Create "GameEmpires(I.I)" table
    GET_GAME_EMPIRES (strTableName, iGameClass, *piGameNumber);
    iErrCode = m_pGameData->CreateTable (strTableName, GameEmpires::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameDeadEmpires(I.I)" table
    GET_GAME_DEAD_EMPIRES (strTableName, iGameClass, *piGameNumber);
    iErrCode = m_pGameData->CreateTable (strTableName, GameDeadEmpires::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameMap(I.I)" table
    GET_GAME_MAP (strTableName, iGameClass, *piGameNumber);
    iErrCode = m_pGameData->CreateTable (strTableName, GameMap::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create GameIndependentShips(I.I) table if necessary
    if (vOptions.GetInteger() & INDEPENDENCE) {

        GAME_INDEPENDENT_SHIPS (strTableName, iGameClass, *piGameNumber);

        iErrCode = m_pGameData->CreateTable (
            strTableName, 
            GameIndependentShips::Template.Name
            );
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
    }

    // No longer creating
    iErrCode = m_pGameData->WriteAnd (
        strGameData,
        GameData::State,
        ~GAME_CREATING
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Unlock
    Assert (bEmpireLocked && !bGameClassLocked);

    for (i = 0; i < iNumEmpires; i ++) {
        UnlockEmpire (pnmEmpireMutex[i]);
    }
    bEmpireLocked = false;

    // Add the game to our game table
    iErrCode = AddToGameTable (iGameClass, *piGameNumber);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    iErrCode = WaitGameWriter (iGameClass, *piGameNumber);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    bGameLocked = true;

    // Enter game
    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = EnterGame (iGameClass, *piGameNumber, piEmpireKey[i], goGameOptions.pszPassword, &iNumUpdates, iEmpireCreator != TOURNAMENT, true);
        if (iErrCode != OK) {
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
                        pvInsert [GameEmpireDiplomacy::EmpireKey] = aTeam.piEmpireKey[k];

                        iErrCode = m_pGameData->InsertRow (strTableName, pvInsert);
                        if (iErrCode != OK) {
                            goto OnError;
                        }

                        GET_GAME_EMPIRE_DIPLOMACY (strTableName, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);
                        pvInsert [GameEmpireDiplomacy::EmpireKey] = aTeam.piEmpireKey[j];

                        iErrCode = m_pGameData->InsertRow (strTableName, pvInsert);
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

            iErrCode = m_pGameData->ReadData (
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

                // Set each empire to alliance teammate's diplomacy table
                for (i = 0; i < goGameOptions.iNumPrearrangedTeams; i ++) {

                    const PrearrangedTeam& aTeam = goGameOptions.paPrearrangedTeam[i];
                    unsigned int iKey, iNumAtAlliance = aTeam.iNumEmpires - 1;

                    if (iNumAtAlliance > (unsigned int) iMaxAlliances) {
                        iMaxAlliances = aTeam.iNumEmpires - 1;
                    }

                    for (j = 0; j < aTeam.iNumEmpires; j ++) {

                        GET_GAME_EMPIRE_DATA (strTableName, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);

                        iErrCode = m_pGameData->WriteData (strTableName, GameEmpireData::NumTruces, iNumAtAlliance);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }

                        iErrCode = m_pGameData->WriteData (strTableName, GameEmpireData::NumTrades, iNumAtAlliance);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }

                        iErrCode = m_pGameData->WriteData (strTableName, GameEmpireData::NumAlliances, iNumAtAlliance);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }

                        for (k = j + 1; k < aTeam.iNumEmpires; k ++) {

                            char pszDip1 [256], pszDip2 [256];

                            GET_GAME_EMPIRE_DIPLOMACY (pszDip1, iGameClass, *piGameNumber, aTeam.piEmpireKey[j]);

                            iErrCode = m_pGameData->GetFirstKey (
                                pszDip1,
                                GameEmpireDiplomacy::EmpireKey,
                                aTeam.piEmpireKey[k],
                                false,
                                &iKey
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = m_pGameData->WriteData (pszDip1, iKey, GameEmpireDiplomacy::DipOffer, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = m_pGameData->WriteData (pszDip1, iKey, GameEmpireDiplomacy::CurrentStatus, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = m_pGameData->WriteData (pszDip1, iKey, GameEmpireDiplomacy::VirtualStatus, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = m_pGameData->WriteOr (pszDip1, iKey, GameEmpireDiplomacy::State, ONCE_ALLIED_WITH);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            GET_GAME_EMPIRE_DIPLOMACY (pszDip2, iGameClass, *piGameNumber, aTeam.piEmpireKey[k]);

                            iErrCode = m_pGameData->GetFirstKey (
                                pszDip2,
                                GameEmpireDiplomacy::EmpireKey,
                                aTeam.piEmpireKey[j],
                                false,
                                &iKey
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = m_pGameData->WriteData (pszDip2, iKey, GameEmpireDiplomacy::DipOffer, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = m_pGameData->WriteData (pszDip2, iKey, GameEmpireDiplomacy::CurrentStatus, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = m_pGameData->WriteData (pszDip2, iKey, GameEmpireDiplomacy::VirtualStatus, ALLIANCE);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            iErrCode = m_pGameData->WriteOr (pszDip2, iKey, GameEmpireDiplomacy::State, ONCE_ALLIED_WITH);
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
                                    iNumEmpires,
                                    piEmpireKey,
                                    ALLIANCE
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

    // Send notification messages for tournament games
    if (iEmpireCreator == TOURNAMENT) {
        
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];
        char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];
        
        iErrCode = GetGameClassName (iGameClass, pszGameClassName);
        if (iErrCode == OK) {
            
            sprintf (
                pszMessage,
                "The tournament game %s %i has been started",
                pszGameClassName,
                *piGameNumber
                );
            
            for (i = 0; i < iNumEmpires; i ++) {
                
                int iErrCode2 = SendSystemMessage (piEmpireKey[i], pszMessage, SYSTEM);
                Assert (iErrCode2 == OK);
            }
        }
    }

    if (bGameLocked) {
        SignalGameWriter (iGameClass, *piGameNumber);
    }

    // UI notification
    if (m_pUIEventSink != NULL) {
        m_pUIEventSink->OnCreateGame (iGameClass, *piGameNumber);
    }

    return OK;

OnError:

    int iErrCode2;

    if (bEmpireLocked) {
        for (i = iNumEmpires - 1; i < iNumEmpires; i --) {
            UnlockEmpire (pnmEmpireMutex[i]);
        }
    }

    if (bGameClassLocked) {
        UnlockGameClass (nmGameClassMutex);
    }

    if (!bIncrementedActiveGameCount && bDeleteRequired) {

        // Balance the cleanup decrement
        iErrCode2 = m_pGameData->Increment (
            SYSTEM_GAMECLASS_DATA,
            iGameClass,
            SystemGameClassData::NumActiveGames,
            1
            );
        Assert (iErrCode2 == OK);
    }

    // Best effort delete the game
    if (bDeleteRequired) {

        iErrCode2 = DeleteGame (iGameClass, *piGameNumber, SYSTEM, "", CREATION_FAILED);
        Assert (iErrCode2 == OK);
    }

    if (bGameLocked) {
        SignalGameWriter (iGameClass, *piGameNumber);
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

int GameEngine::EnterGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszPassword, 
                           int* piNumUpdates, bool bSendMessages, bool bCreatingGame) {

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
    bool bWarn, bBlock, bFlag, bAddedToGame = false, bClosed = false, bStarted = false, bPaused = false,
        bUnPaused = false, bUnlockGame = false;

    int iErrCode, iNumTechs, iDefaultOptions, iGameState, iSystemOptions;
    unsigned int iCurrentNumEmpires = -1, i, iKey = NO_KEY;

    Variant vGameClassOptions, vHalted, vPassword, vPrivilege, vMinScore, vMaxScore, vEmpireScore, vTemp, 
        vMaxNumEmpires, vStillOpen, vNumUpdates, vMaxTechDev, vEmpireOptions, vEmpireName, vDiplomacyLevel,
        vGameOptions;

    bool bEmpireLocked = true, bGenerateMapForAllEmpires = false;

    Variant pvGameEmpireData [GameEmpireData::NumColumns];

    NamedMutex nmEmpireMutex;
    if (!bCreatingGame) {
        iErrCode = LockEmpire (iEmpireKey, &nmEmpireMutex);
        if (iErrCode != OK) {
            goto OnError;
        }
    }

    // Get time
    UTCTime tTime;
    Time::GetTime (&tTime);

    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
    if (iErrCode != OK || !bFlag) {
        iErrCode = ERROR_GAME_DOES_NOT_EXIST;
        goto OnError;
    }

    iErrCode = m_pGameData->ReadData (
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

    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vDiplomacyLevel
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    iErrCode = m_pGameData->ReadData (strGameData, GameData::Options, &vGameOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Make sure we still exist, kill game if not and we just created it and we're alone
    iErrCode = DoesEmpireExist (iEmpireKey, &bFlag, &vEmpireName);
    if (iErrCode != OK || !bFlag) {
        iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
        goto OnError;
    }

    // Make sure game still exists, get num empires in game
    iErrCode = m_pGameData->GetNumRows (strGameEmpires, &iCurrentNumEmpires);
    if (iErrCode != OK) {
        iErrCode = ERROR_GAME_DOES_NOT_EXIST;
        goto OnError;
    }

    // Make sure empire isn't halted
    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vEmpireOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    if (vEmpireOptions.GetInteger() & EMPIRE_MARKED_FOR_DELETION) {
        iErrCode = ERROR_EMPIRE_IS_HALTED;
        goto OnError;
    }

    // Make sure empire hasn't entered already  
    iErrCode = m_pGameData->GetFirstKey (strGameEmpires, GameEmpires::EmpireKey, iEmpireKey, false, &iKey);
    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        if (iErrCode == OK) {
            iErrCode = ERROR_ALREADY_IN_GAME;
        }
        goto OnError;
    }

    // Make sure empire wasn't nuked out
    iErrCode = m_pGameData->GetFirstKey (strGameDeadEmpires, GameDeadEmpires::Name, vEmpireName, true, &iKey);
    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        if (iErrCode != OK) {
            goto OnError;
        }

        iErrCode = m_pGameData->ReadData (strGameDeadEmpires, iKey, GameDeadEmpires::EmpireKey, &vTemp);
        if (iErrCode != OK) {
            goto OnError;
        }

        if (iEmpireKey == vTemp.GetInteger()) {
            iErrCode = ERROR_WAS_ALREADY_IN_GAME;
            goto OnError;
        }
    }
    
    // Make sure game is still open
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
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
    iErrCode = m_pGameData->ReadData (strGameData, GameData::Password, &vPassword);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    if (!String::IsBlank (vPassword.GetCharPtr()) &&
         String::StrCmp (vPassword.GetCharPtr(), pszPassword) != 0) {
        iErrCode = ERROR_WRONG_PASSWORD;
        goto OnError;
    }

    // Check security if empire is first in
    if (iCurrentNumEmpires > 0) {

        iErrCode = GameAccessCheck (iGameClass, iGameNumber, iEmpireKey, NULL, ENTER_GAME, &bFlag);
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
                    iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, false);
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
                    iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, false);
                    Assert (iErrCode == OK);
                    
                    iErrCode = ERROR_DUPLICATE_SESSION_ID;
                    goto OnError;
                }
            }
        }

        // Check for 'pause on start' option
        if (!(iSystemOptions & PAUSE_GAMES_BY_DEFAULT)) {

            // Unpause the game - a new empire came in and he's not paused by default
            iErrCode = IsGamePaused (iGameClass, iGameNumber, &bUnPaused);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iErrCode = UnpauseGame (iGameClass, iGameNumber, false, false);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
        }
        
        // Try to trigger all remaining updates
        if (!bCreatingGame) {
            iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, &bFlag);
            Assert (iErrCode == OK);
        }
    }

    // Lock the game for writing if necessary
    if (!bCreatingGame) {

        iErrCode = WaitGameWriter (iGameClass, iGameNumber);
        if (iErrCode != OK) {
            goto OnError;
        }
        bUnlockGame = true;
    
        // Make sure the game didn't end
        iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
        if (iErrCode != OK || !bFlag) {
            iErrCode = ERROR_GAME_DOES_NOT_EXIST;
            goto OnError;
        }

        // Make sure game is still open
        iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
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
    iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_ADDING_EMPIRE);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Insert row into GameEmpires(I.I) table
    vTemp = iEmpireKey;
    iErrCode = m_pGameData->InsertRow (strGameEmpires, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    iCurrentNumEmpires ++;
    bAddedToGame = true;
    
    // Increment MaxNumEmpires count if game has started
    if (iGameState & STARTED) {

        iErrCode = m_pGameData->ReadData (strGameData, GameData::MaxNumEmpires, &vMaxNumEmpires);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        if ((int) iCurrentNumEmpires > vMaxNumEmpires.GetInteger()) {

            iErrCode = m_pGameData->WriteData (strGameData, GameData::MaxNumEmpires, iCurrentNumEmpires);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
        }
    }
    
    // Close game if we're the last to enter    
    if (iGameState & STILL_OPEN) {

        iErrCode = m_pGameData->ReadData (
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
            iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~STILL_OPEN);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iGameState &= ~STILL_OPEN;
            
            // Delete from open list
            char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
            GetGameClassGameNumber (iGameClass, iGameNumber, pszData);
            
            unsigned int iGameKey;
            iErrCode = m_pGameData->GetFirstKey (
                SYSTEM_ACTIVE_GAMES, 
                SystemActiveGames::GameClassGameNumber,
                pszData, 
                true,
                &iGameKey
                );

            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iErrCode = m_pGameData->WriteData (SYSTEM_ACTIVE_GAMES, iGameKey, SystemActiveGames::State, 0);
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

    if (!m_pGameData->DoesTableExist (strEmpireActiveGames)) {

        iErrCode = m_pGameData->CreateTable (strEmpireActiveGames, SystemEmpireActiveGames::Template.Name);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
    }

    iErrCode = m_pGameData->InsertRow (strEmpireActiveGames, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    ////////////////////////////
    // Create empire's tables //
    ////////////////////////////
    
    // Create "GameEmpireData(I.I.I)" table
    iErrCode = m_pGameData->CreateTable (strGameEmpireData, GameEmpireData::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    // Insert data into GameEmpireData(I.I.I) table
    pvGameEmpireData[GameEmpireData::NumPlanets] = 1;
    pvGameEmpireData[GameEmpireData::TotalAg] = 0;
    pvGameEmpireData[GameEmpireData::TotalFuel] = 0;
    pvGameEmpireData[GameEmpireData::TotalMin] = 0;
    pvGameEmpireData[GameEmpireData::TotalPop] = 0;
    
    // Initial tech level
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::InitialTechLevel, 
        pvGameEmpireData + GameEmpireData::TechLevel
        );
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    // Add tech to initial tech if empire is a late-comer
    iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    if (vNumUpdates.GetInteger() > 0) {

        Variant vPercentTechIncreaseForLatecomers;

        iErrCode = m_pGameData->ReadData (
            SYSTEM_DATA,
            SystemData::PercentTechIncreaseForLatecomers,
            &vPercentTechIncreaseForLatecomers
            );

        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxTechDev, 
            &vMaxTechDev
            );
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        pvGameEmpireData[GameEmpireData::TechLevel] += GetLateComerTechIncrease (
            vPercentTechIncreaseForLatecomers.GetInteger(),
            vNumUpdates.GetInteger(), 
            vMaxTechDev.GetFloat()
            );
    }

    pvGameEmpireData[GameEmpireData::TotalBuild] = 0;
    pvGameEmpireData[GameEmpireData::TotalMaintenance] = 0;
    pvGameEmpireData[GameEmpireData::TotalFuelUse] = 0;
    pvGameEmpireData[GameEmpireData::LastLogin] = tTime;
    pvGameEmpireData[GameEmpireData::EnterGameIPAddress] = "";
    pvGameEmpireData[GameEmpireData::PartialMapCenter] = PARTIAL_MAP_NATURAL_CENTER;
    pvGameEmpireData[GameEmpireData::PartialMapXRadius] = PARTIAL_MAP_UNLIMITED_RADIUS;
    pvGameEmpireData[GameEmpireData::PartialMapYRadius] = PARTIAL_MAP_UNLIMITED_RADIUS;

    // Initial number of techs to be developed is the min of the initial BR and the number
    // of developable techs that a game class allows
    iNumTechs = (int) sqrt (pvGameEmpireData[GameEmpireData::TechLevel].GetFloat());

    // TechDevs
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::InitialTechDevs, 
        &pvGameEmpireData[GameEmpireData::TechDevs]
        );
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    // TechUndevs
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DevelopableTechDevs, 
        pvGameEmpireData + GameEmpireData::TechUndevs
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // NumAvailableTechUndevs
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::NumInitialTechDevs, 
        pvGameEmpireData + GameEmpireData::NumAvailableTechUndevs
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }
    
    // Filter out already developed techs
    ENUMERATE_TECHS(i) {
        
        if (pvGameEmpireData[GameEmpireData::TechDevs].GetInteger() & TECH_BITS[i]) {
            
            Assert (pvGameEmpireData[GameEmpireData::TechUndevs].GetInteger() & TECH_BITS[i]);
            
            pvGameEmpireData[GameEmpireData::TechUndevs] = 
                pvGameEmpireData[GameEmpireData::TechUndevs].GetInteger() & ~(TECH_BITS[i]);
        }
    }

    // Fill in the rest of the data
    pvGameEmpireData[GameEmpireData::Econ] = 1;                                                 // Econ
    pvGameEmpireData[GameEmpireData::Mil] = (float) 0.0;                                        // Mil
    pvGameEmpireData[GameEmpireData::TargetPop] = 0;
    pvGameEmpireData[GameEmpireData::HomeWorld] = NO_KEY;                                       // HWKey
    pvGameEmpireData[GameEmpireData::NumUpdatesIdle] = 0;                                   // 0 updates idle
    pvGameEmpireData[GameEmpireData::MaxBR] = GetBattleRank ((float) pvGameEmpireData[4]);  // Max BR
    pvGameEmpireData[GameEmpireData::BonusAg] = 0;                  // 0 bonus ag
    pvGameEmpireData[GameEmpireData::BonusMin] = 0;                 // 0 bonus min
    pvGameEmpireData[GameEmpireData::BonusFuel] = 0;                // 0 bonus fuel
    pvGameEmpireData[GameEmpireData::NumBuilds] = 0;                // No ships being built
    pvGameEmpireData[GameEmpireData::MinX] = MAX_COORDINATE;        // MinX
    pvGameEmpireData[GameEmpireData::MaxX] = MIN_COORDINATE;        // MaxX
    pvGameEmpireData[GameEmpireData::MinY] = MAX_COORDINATE;        // MinY
    pvGameEmpireData[GameEmpireData::MaxY] = MIN_COORDINATE;        // MaxY
    pvGameEmpireData[GameEmpireData::NextMaintenance] = 0;          // NextMaintenance
    pvGameEmpireData[GameEmpireData::NextFuelUse] = 0;              // NextFuelUse
    pvGameEmpireData[GameEmpireData::NextTotalPop] = pvGameEmpireData[GameEmpireData::TotalMin]; // NextMin
    pvGameEmpireData[GameEmpireData::NextMin] = 0;                  // NextFuel
    pvGameEmpireData[GameEmpireData::NextFuel] = 0;
    pvGameEmpireData[GameEmpireData::NumTruces] = 0;
    pvGameEmpireData[GameEmpireData::NumTrades] = 0;
    pvGameEmpireData[GameEmpireData::NumAlliances] = 0;

    // Select default message target
    iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::DefaultMessageTarget, 
        &pvGameEmpireData[GameEmpireData::DefaultMessageTarget]
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    pvGameEmpireData[GameEmpireData::LastMessageTargetMask] = 0;

    switch (pvGameEmpireData[GameEmpireData::DefaultMessageTarget].GetInteger()) {

    case MESSAGE_TARGET_TRUCE:

        if (!(vDiplomacyLevel.GetInteger() & TRUCE)) {
            pvGameEmpireData[GameEmpireData::DefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;

    case MESSAGE_TARGET_TRADE:

        if (!(vDiplomacyLevel.GetInteger() & TRADE)) {
            pvGameEmpireData[GameEmpireData::DefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;

    case MESSAGE_TARGET_ALLIANCE:

        if (!(vDiplomacyLevel.GetInteger() & ALLIANCE)) {
            pvGameEmpireData[GameEmpireData::DefaultMessageTarget] = MESSAGE_TARGET_NONE;
        }
        break;
    }

    // Get default number of saved game messages
    iErrCode = m_pGameData->ReadData (
        SYSTEM_DATA, 
        SystemData::DefaultMaxNumGameMessages, 
        &pvGameEmpireData[GameEmpireData::MaxNumGameMessages]
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Grab default game options from empire's options
    iDefaultOptions = vEmpireOptions.GetInteger() & (
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

    pvGameEmpireData[GameEmpireData::Options] = iDefaultOptions;

    // Blank notepad
    pvGameEmpireData[GameEmpireData::Notepad] = "";

    // Default builder planet is empire's default
    iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::DefaultBuilderPlanet, 
        &pvGameEmpireData[GameEmpireData::DefaultBuilderPlanet]
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    pvGameEmpireData[GameEmpireData::LastBuilderPlanet] = NO_KEY;

    pvGameEmpireData [GameEmpireData::MaxEcon] = 0;
    pvGameEmpireData [GameEmpireData::MaxMil] = 0;
    pvGameEmpireData [GameEmpireData::NumAlliancesLeaked] = 0;

    if (vGameOptions.GetInteger() & GAME_COUNT_FOR_BRIDIER) {

        int iRank, iIndex;

        iErrCode = GetBridierScore (iEmpireKey, &iRank, &iIndex);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        pvGameEmpireData [GameEmpireData::InitialBridierRank] = iRank;
        pvGameEmpireData [GameEmpireData::InitialBridierIndex] = iIndex;

    } else {

        pvGameEmpireData [GameEmpireData::InitialBridierRank] = 0;
        pvGameEmpireData [GameEmpireData::InitialBridierIndex] = 0;
    }

    iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::GameRatios, 
        pvGameEmpireData + GameEmpireData::GameRatios
        );

    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    pvGameEmpireData[GameEmpireData::MiniMaps] = MINIMAPS_NEVER;

    ///////////////////////////////
    // Insert GameEmpireData row //
    ///////////////////////////////

    iErrCode = m_pGameData->InsertRow (strGameEmpireData, pvGameEmpireData);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameEmpireMessages(I.I.I)" table
    iErrCode = m_pGameData->CreateTable (strGameEmpireMessages, GameEmpireMessages::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameEmpireMap(I.I.I)" table
    iErrCode = m_pGameData->CreateTable (strGameEmpireMap, GameEmpireMap::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Unlock the empire
    if (!bCreatingGame) {
        UnlockEmpire (nmEmpireMutex);
        bEmpireLocked = false;
    }

    ////////////////////////////////////////////////
    // Create the rest of the new empire's tables //
    ////////////////////////////////////////////////

    // Create "GameEmpireShips(I.I.I)" table
    iErrCode = m_pGameData->CreateTable (strGameEmpireShips, GameEmpireShips::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameEmpireFleets(I.I.I)" table
    iErrCode = m_pGameData->CreateTable (strGameEmpireFleets, GameEmpireFleets::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Create "GameEmpireDiplomacy(I.I.I)" table
    iErrCode = m_pGameData->CreateTable (strGameEmpireDiplomacy, GameEmpireDiplomacy::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    // Send empire welcome message
    iErrCode = m_pGameData->ReadData (strGameData, GameData::EnterGameMessage, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    if (!String::IsBlank (vTemp.GetCharPtr())) {

        Variant vCreatorName;
        const char* pszMessage;

        iErrCode = m_pGameData->ReadData (strGameData, GameData::CreatorName, &vCreatorName);
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

        iErrCode = SendGameMessage (iGameClass, iGameNumber, iEmpireKey, pszMessage, SYSTEM, false, false, NULL_TIME);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }

        if (pszMessage != vTemp.GetCharPtr()) {
            delete [] (char*) pszMessage;
        }
    }

    /////////////////////////
    // Game starting stuff //
    /////////////////////////

    // Has game started?    
    if (iGameState & STARTED) {

        // Add new empire to map if the map has been generated already
        if (iGameState & GAME_MAP_GENERATED) {

            iErrCode = AddEmpiresToMap (iGameClass, iGameNumber, &iEmpireKey, 1, &bFlag);
            if (iErrCode != OK) {

                Assert (false);

                if (bFlag) {
                    
                    // Catastrophic - delete the game
                    int iErrCode2 = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
                    Assert (iErrCode2 == OK);
                
                } else {

                    int iErrCode2 = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_GAME_ENTRY_ERROR, NULL);
                    Assert (iErrCode2 == OK);
                }

                if (!bCreatingGame) {
                    SignalGameWriter (iGameClass, iGameNumber);
                }
                
                return ERROR_COULD_NOT_CREATE_PLANETS;
            }
        }
        
    } else {
        
        // Are we the trigger for the game to begin?
        unsigned int iNumActiveEmpires;
        Variant vMinNumEmpires;

        iErrCode = m_pGameData->GetNumRows (strGameEmpires, &iNumActiveEmpires);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        iErrCode = m_pGameData->ReadData (
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
            iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, STARTED);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iGameState |= STARTED;
            
            // Set last update time
            iErrCode = m_pGameData->WriteData (strGameData, GameData::LastUpdateTime, tTime);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            
            // Set number of updates to zero
            iErrCode = m_pGameData->WriteData (strGameData, GameData::NumUpdates, 0);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            // Set max num empires
            iErrCode = m_pGameData->WriteData (strGameData, GameData::MaxNumEmpires, iCurrentNumEmpires);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            // Set paused if necessary
            if (iSystemOptions & PAUSE_GAMES_BY_DEFAULT) {

                bPaused = true;

                iErrCode = m_pGameData->WriteData (strGameData, GameData::NumRequestingPause, iCurrentNumEmpires);
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
        }
    }

    if (bGenerateMapForAllEmpires) {

        Variant* pvKey;
        int* piEmpKey;
        unsigned int iNumKeys;
        
        iErrCode = m_pGameData->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, &pvKey, &iNumKeys);
        if (iErrCode != OK) {
            Assert (false);
            goto OnError;
        }
        
        // Add empires to map
        piEmpKey = (int*) StackAlloc (iNumKeys * sizeof (int));
        for (i = 0; i < iNumKeys; i ++) {
            piEmpKey[i] = pvKey[i].GetInteger();
        }
        
        // Randomize empires
        Algorithm::Randomize<int> (piEmpKey, iNumKeys);
        
        iErrCode = AddEmpiresToMap (iGameClass, iGameNumber, piEmpKey, iNumKeys, &bFlag);
        if (iErrCode != OK) {
            
            Assert (false);
            
            // Abort: this is catastrophic
            int iErrCode2 = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
            Assert (iErrCode2 == OK);

            m_pGameData->FreeData (pvKey);
            
            if (!bCreatingGame) {
                SignalGameWriter (iGameClass, iGameNumber);
            }

            return ERROR_COULD_NOT_CREATE_PLANETS;
        }
        
        m_pGameData->FreeData (pvKey);
        
        iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_MAP_GENERATED);
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
        iErrCode = m_pGameData->ReadColumn (
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
                    
                    pvDiplomacy[GameEmpireDiplomacy::EmpireKey] = pvEmpireKey[i];
                    
                    // Insert iterated player into empire's table
                    iErrCode = m_pGameData->InsertRow (strGameEmpireDiplomacy, pvDiplomacy, &iKey);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }
                    
                    GET_GAME_EMPIRE_DIPLOMACY (pszDiplomacy, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

                    // Insert empire into iterated player's table
                    pvDiplomacy[GameEmpireDiplomacy::EmpireKey] = iEmpireKey;

                    iErrCode = m_pGameData->InsertRow (
                        pszDiplomacy, 
                        pvDiplomacy, 
                        &iKey
                        );

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
                        iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), strMessage.GetCharPtr(), SYSTEM, true, 
                        false, NULL_TIME);
                }
            }
        }
        
        // Clean up
        m_pGameData->FreeData (pvEmpireKey);
        pvEmpireKey = NULL;
    }

    // Get num updates
    iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    *piNumUpdates = vNumUpdates.GetInteger();
    
    // We're done adding the empire
    iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_ADDING_EMPIRE);
    if (iErrCode != OK) {
        Assert (false);
        goto OnError;
    }

    if (bUnlockGame) {
        SignalGameWriter (iGameClass, iGameNumber);
    }

    // Entry was sucessful  
    return OK;

OnError:

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
            iErrCode2 = m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_ADDING_EMPIRE);
            Assert (iErrCode2 == OK);
        }
    }

    if (bEmpireLocked) {
        UnlockEmpire (nmEmpireMutex);
    }

    if (bUnlockGame) {
        SignalGameWriter (iGameClass, iGameNumber);
    }

    return iErrCode;
}


int GameEngine::SetEnterGameIPAddress (int iGameClass, int iGameNumber, int iEmpireKey, 
                                       const char* pszIPAddress) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->WriteData (
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

int GameEngine::DoesGameExist (int iGameClass, int iGameNumber, bool* pbExist) {
    
    GameObject* pGameObject = GetGameObject (iGameClass, iGameNumber);

    if (pGameObject != NULL) {
        pGameObject->Release();
        *pbExist = true;
    } else {
        *pbExist = false;
    }

    return OK;
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

    iErrCode = m_pGameData->ReadData (pszGameData, GameData::NumUpdates, &vNumUpdates);
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

    iErrCode = m_pGameData->ReadData (pszGameData, GameData::NumUpdatesBeforeGameCloses, &vNumUpdates);
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

    iErrCode = m_pGameData->ReadData (pszGameData, GameData::Options, &vValue);
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

    iErrCode = m_pGameData->ReadData (pszGameData, GameData::FirstUpdateDelay, &vValue);
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

    return m_pGameData->GetNumRows (strGameEmpires, (unsigned int*) piNumEmpires);
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
    int iErrCode = m_pGameData->ReadData (
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

    int iErrCode = m_pGameData->ReadData (strGameData, GameData::NumEmpiresUpdated, &vTemp);
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

    int iErrCode = m_pGameData->ReadColumn (
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

    int iErrCode = m_pGameData->ReadData (pszGameData, GameData::State, &vTemp);

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

    int iErrCode = m_pGameData->ReadData (pszGameData, GameData::State, &vTemp);
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
            
            iErrCode = m_pGameData->GetNextKey (strGameEmpires, iEmpKey, &iEmpKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            iErrCode = m_pGameData->ReadData (strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            // Best effort send messages
            if (!String::IsBlank (pszAdminMessage)) {
                iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszAdminMessage, SYSTEM);
                Assert (iErrCode == OK);
            }
            
            iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszMessage, SYSTEM);
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
            
            iErrCode = m_pGameData->GetNextKey (strGameEmpires, iEmpKey, &iEmpKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            iErrCode = m_pGameData->ReadData (strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            // Best effort send messages
            if (!String::IsBlank (pszAdminMessage)) {
                iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszAdminMessage, SYSTEM);
                Assert (iErrCode == OK);
            }
            
            iErrCode = SendSystemMessage (vEmpireKey.GetInteger(), pszMessage, SYSTEM);
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

    iErrCode = m_pGameData->GetNumRows (strGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iEmpKey = NO_KEY;
    while (true) {

        iErrCode = m_pGameData->GetNextKey (strGameEmpires, iEmpKey, &iEmpKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iErrCode = m_pGameData->ReadData (strGameEmpires, iEmpKey, GameEmpires::EmpireKey, &vEmpireKey);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        GET_GAME_EMPIRE_DIPLOMACY (pszDiplomacy, iGameClass, iGameNumber, vEmpireKey.GetInteger());

        iErrCode = m_pGameData->GetNumRows (pszDiplomacy, &iNumRows);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iNumRows != iNumEmpires - 1) {
            bAlly = false;
            break;
        }

        iKey = NO_KEY;
        while (true) {

            iErrCode = m_pGameData->GetNextKey (pszDiplomacy, iKey, &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            if (iErrCode != OK) {
                goto Cleanup;
            }

            iErrCode = m_pGameData->ReadData (
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
    Variant vNum;

    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    iErrCode = m_pGameData->ReadData (strGameData, GameData::NumRequestingDraw, &vNum);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->GetNumRows (strGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    *pbDraw = ((unsigned int) vNum.GetInteger() >= iNumEmpires);

Cleanup:

    return iErrCode;
}

int GameEngine::PauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) {

    int iErrCode, iSecondsSince, iSecondsUntil, iNumUpdates, iState;

    bool bFlag;
    const char* pszMessage;

    IWriteTable* pGameData = NULL;
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Get the time
    UTCTime tTime;
    Time::GetTime (&tTime);

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
    Variant vSecPerUpdate;
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

    // Get game information
    iErrCode = GetGameUpdateData (
        iGameClass, 
        iGameNumber, 
        &iSecondsSince, 
        &iSecondsUntil, 
        &iNumUpdates, 
        &iState
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->GetTableForWriting (strGameData, &pGameData);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = pGameData->ReadData (GameData::State, &iState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (bAdmin && !(iState & ADMIN_PAUSED)) {

        iErrCode = pGameData->WriteOr (GameData::State, ADMIN_PAUSED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    
    if (iState & PAUSED) {
        goto Cleanup;
    }

    // Pause the game

    // Normalize for weekend delays
    if (iSecondsUntil > vSecPerUpdate.GetInteger()) {

        if (iNumUpdates == 0) {

            int iDelay;
            iErrCode = pGameData->ReadData (GameData::FirstUpdateDelay, &iDelay);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            int iNext = vSecPerUpdate.GetInteger() + iDelay;
            if (iSecondsUntil > iNext) {
                iSecondsUntil = iNext;
            }

        } else {

            iSecondsUntil = vSecPerUpdate.GetInteger();
        }
    }
    
    iErrCode = pGameData->WriteOr (GameData::State, PAUSED);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Write down remaining seconds
    iErrCode = pGameData->WriteData (GameData::SecondsSinceLastUpdateWhilePaused, iSecondsSince);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pGameData);

    // Best effort broadcast message
    if (bBroadcast) {
        
        pszMessage = bAdmin ? "The game was paused by an administrator" : "The game is now paused";

        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, false);
        Assert (iErrCode == OK);
    }

Cleanup:

    SafeRelease (pGameData);

    return iErrCode;
}


int GameEngine::UnpauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) {

    int iErrCode;
    const char* pszMessage;
    Variant vTemp;

    IWriteTable* pGameData = NULL;

    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    unsigned int iNumEmpires;
    iErrCode = m_pGameData->GetNumRows (strEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    iErrCode = m_pGameData->GetTableForWriting (strGameData, &pGameData);
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

    int iSecsSince;
    
    // Get seconds since update that game was paused
    iErrCode = pGameData->ReadData (GameData::SecondsSinceLastUpdateWhilePaused, &iSecsSince);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get time
    UTCTime tNow, tNewLastUpdateTime;
    Time::GetTime (&tNow);

    // New last update time is 'now' minus time since last update when paused
    Time::SubtractSeconds (tNow, iSecsSince, &tNewLastUpdateTime);

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
        BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, false);
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
    
    int iErrCode = m_pGameData->ReadColumn (
        SYSTEM_ACTIVE_GAMES,
        SystemActiveGames::GameClassGameNumber,
        &pvGame,
        &iNumKeys
        );
    
    if (iErrCode == OK) {
        
        for (i = 0; i < iNumKeys; i ++) {
            
            GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

            // Flush remaining updates
            iErrCode = CheckGameForUpdates (
                iGameClass,
                iGameNumber,
                &bExists
                );

            // Best effort pause the game
            if (iErrCode == OK && DoesGameExist (iGameClass, iGameNumber, &bExists) == OK && bExists) {

                if (WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL) == OK) {
                    iErrCode = PauseGame (iGameClass, iGameNumber, true, true);
                    Assert (iErrCode == OK);
                    SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
                }
            }
        }
        
        m_pGameData->FreeData (pvGame);
    
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
    
    int iErrCode = m_pGameData->ReadColumn (
        SYSTEM_ACTIVE_GAMES,
        SystemActiveGames::GameClassGameNumber,
        &pvGame,
        &iNumKeys
        );
    
    if (iErrCode == OK) {
        
        for (i = 0; i < iNumKeys; i ++) {
            
            GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

            if (WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL) == OK) {

                // Best effort
                iErrCode = UnpauseGame (iGameClass, iGameNumber, true, true);
                Assert (iErrCode == OK);

                SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
            }
        }
        
        m_pGameData->FreeData (pvGame);
    
    } else {
        
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert (false);
    }

    return iErrCode;
}


int GameEngine::AddToGameTable (int iGameClass, int iGameNumber) {

    int iErrCode;
    char pszGame [128];
    sprintf (pszGame, "%i.%i", iGameClass, iGameNumber);

    GameObject* pGameObject = new GameObject();
    if (pGameObject == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    iErrCode = pGameObject->Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pGameObject->SetName (pszGame);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    m_rwGameObjectTableLock.WaitWriter();
    bool bRetVal = m_htGameObjectTable.Insert (pGameObject->GetName(), pGameObject);
    m_rwGameObjectTableLock.SignalWriter();

    if (!bRetVal) {
        pGameObject->Release();
        return ERROR_OUT_OF_MEMORY;
    }

    return OK;
}


int GameEngine::RemoveFromGameTable (int iGameClass, int iGameNumber) {

    char pszGame [128];
    sprintf (pszGame, "%i.%i", iGameClass, iGameNumber);

    GameObject* pGameObject;

    m_rwGameObjectTableLock.WaitWriter();
    bool bRetVal = m_htGameObjectTable.DeleteFirst (pszGame, NULL, &pGameObject);

    if (bRetVal) {
        pGameObject->Release();
    }
    m_rwGameObjectTableLock.SignalWriter();

    return bRetVal ? OK : ERROR_GAME_DOES_NOT_EXIST;
}

GameObject* GameEngine::GetGameObject (int iGameClass, int iGameNumber) {

    GameObject* pGameObject = NULL;

    char pszGame [128];
    sprintf (pszGame, "%i.%i", iGameClass, iGameNumber);

    m_rwGameObjectTableLock.WaitReader();
    bool bRetVal = m_htGameObjectTable.FindFirst (pszGame, &pGameObject);

    if (bRetVal) {
        pGameObject->AddRef();
    }

    m_rwGameObjectTableLock.SignalReader();
    
    return pGameObject;
}

int GameEngine::LogEmpireIntoGame (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    IWriteTable* pTable = NULL;

    UTCTime tTime;
    Time::GetTime (&tTime);
    
    // Update LastLogin, NumUpdatesIdle, IP Address
    iErrCode = m_pGameData->GetTableForWriting (strGameEmpireData, &pTable);
    if (iErrCode != OK) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }
    
    // Num updates idle
    iErrCode = pTable->WriteData (GameEmpireData::NumUpdatesIdle, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
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

    iErrCode = m_pGameData->ReadColumn (
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
        m_pGameData->FreeData (pvEmpireKey);
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
    iErrCode = m_pGameData->ReadData (pszTable, GameData::NumEmpiresResigned, &vResigned);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vResigned.GetInteger() == 0) {
        goto Cleanup;
    }

    GET_GAME_EMPIRES (pszTable, iGameClass, iGameNumber);

    // Get empires
    iErrCode = m_pGameData->ReadColumn (
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
        m_pGameData->FreeData (pvEmpireKey);
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

    iErrCode = m_pGameData->GetNumRows (pszGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    Assert (iNumEmpires <= 2);

    // Read 1st empire
    iErrCode = m_pGameData->ReadData (pszGameEmpires, 0, GameEmpires::EmpireKey, &v0Key);
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
    
    iErrCode = m_pGameData->ReadData (pszGameData, GameEmpireData::InitialBridierRank, &vRank);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    iErrCode = m_pGameData->ReadData (pszGameData, GameEmpireData::InitialBridierIndex, &vIndex);
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

        iErrCode = m_pGameData->ReadData (pszGameEmpires, 1, GameEmpires::EmpireKey, &v1Key);
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
        
        iErrCode = m_pGameData->ReadData (pszGameData, GameEmpireData::InitialBridierRank, &vRank);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->ReadData (pszGameData, GameEmpireData::InitialBridierIndex, &vIndex);
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

    iErrCode = m_pGameData->ReadData (pszGameData, GameData::Options, &vGameOptions);
    if (iErrCode != OK) {
        return iErrCode;
    }

    *pbSpectatorGame = (vGameOptions.GetInteger() & GAME_ALLOW_SPECTATORS) != 0;
    return iErrCode;
}

int GameEngine::AddToLatestGames (const Variant* pvColumns, unsigned int iTournamentKey) {

    int iErrCode = OK;

    if (iTournamentKey != NO_KEY) {

        SYSTEM_TOURNAMENT_LATEST_GAMES (pszGames, iTournamentKey);
        iErrCode = AddToLatestGames (pszGames, pvColumns);
    }

    return iErrCode != OK  ? iErrCode : AddToLatestGames (SYSTEM_LATEST_GAMES, pvColumns);
}

int GameEngine::AddToLatestGames (const char* pszTable, const Variant* pvColumns) {

    int iErrCode;
    IWriteTable* pGames = NULL;

    unsigned int iNumRows;
    Variant vGames;

    // Read limit
    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::NumGamesInLatestGameList, &vGames);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Lock table
    iErrCode = m_pGameData->GetTableForWriting (pszTable, &pGames);
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
            iErrCode = pGames->DeleteRow (iOldestKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    // Finally, insert the new row
    iErrCode = pGames->InsertRow (pvColumns);
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

    unsigned int iNumEmpiresSoFar = 0;
    unsigned int iKey = NO_KEY;

    char pszGameEmpires [256];

    while (true) {

        const char* pszGame;
        unsigned int iNumEmpiresThisGame;
        int iGameClass, iGameNumber;

        iErrCode = m_pGameData->GetTableForReading (SYSTEM_ACTIVE_GAMES, &pGames);
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

        iErrCode = pGames->ReadData (iKey, SystemActiveGames::GameClassGameNumber, &pszGame);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        GetGameClassGameNumber (pszGame, &iGameClass, &iGameNumber);

        SafeRelease (pGames);

        GET_GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

        iErrCode = m_pGameData->GetNumRows (pszGameEmpires, &iNumEmpiresThisGame);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iNumEmpiresSoFar += iNumEmpiresThisGame;
    }

    *piNumEmpires = iNumEmpiresSoFar;

Cleanup:

    SafeRelease (pGames);

    return iErrCode;
}