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
			"%s %i was deleted by the administrator %s",
			pszGameClassName,
			iGameNumber,
			vEmpireName.GetCharPtr()
			);
	}

	// Cleanup the game
	iErrCode = CleanupGame (iGameClass, iGameNumber);
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

int GameEngine::CleanupGame (int iGameClass, int iGameNumber) {

	int iErrCode;
	unsigned int i, iNumKeys;

	Variant* pvEmpireKey = NULL, vName;
	
	GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
	GAME_DATA (strGameData, iGameClass, iGameNumber);

	String strList;

	// Notification
	if (m_pUIEventSink != NULL) {
		m_pUIEventSink->OnCleanupGame (iGameClass, iGameNumber);
	}

	//
	// Need to be tolerant of errors...
	//

	// First, update scores if necessary
	iErrCode = UpdateScoresOnGameEnd (iGameClass, iGameNumber);
	Assert (iErrCode == OK);

	iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_DELETING);
	Assert (iErrCode == OK);

	// Delete all remaining empires from the game
	iErrCode = m_pGameData->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, &pvEmpireKey, &iNumKeys);
	if (iErrCode == OK) {

		for (i = 0; i < iNumKeys; i ++) {

			if (m_scConfig.bLogGameEndings &&
				m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, pvEmpireKey[i], SystemEmpireData::Name, &vName) == OK) {
				
				strList += vName.GetCharPtr();
				if (i < iNumKeys - 1) {
					strList += ", ";
				}
			}

			// Best effort
			iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			Assert (iErrCode == OK);
		}

		m_pGameData->FreeData (pvEmpireKey);
	}
	
	if (m_scConfig.bLogGameEndings) {

		char* pszGameClass = (char*) StackAlloc (MAX_FULL_GAME_CLASS_NAME_LENGTH);

		iErrCode = GetGameClassName (iGameClass, pszGameClass);
		if (iErrCode == OK) {

			char* pszMessage = (char*) StackAlloc (strList.GetLength() + 256 + MAX_FULL_GAME_CLASS_NAME_LENGTH);
			
			sprintf (
				pszMessage,
				"%s %i ended with the following empires still alive: %s",
				pszGameClass,
				iGameNumber,
				strList.GetCharPtr() == NULL ? "" : strList.GetCharPtr()
				);
			
			m_pReport->WriteReport (pszMessage);
		}
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

	// Delete GameIndependentShips(I.I) table if necessary
	Variant vGameOptions, vGameClassOptions;

	iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vGameClassOptions);
	if (iErrCode == OK) {
		
		if (vGameClassOptions.GetInteger() & INDEPENDENCE) {

			GET_GAME_INDEPENDENT_SHIPS (pszTable, iGameClass, iGameNumber);

			iErrCode = m_pGameData->DeleteTable (pszTable);
			Assert (iErrCode == OK);
		}
	}

	iErrCode = m_pGameData->ReadData (strGameData, GameData::Options, &vGameOptions);
	if (iErrCode == OK) {

		if (vGameOptions.GetInteger() & GAME_ENFORCE_SECURITY) {

			GET_GAME_SECURITY (pszTable, iGameClass, iGameNumber);

			iErrCode = m_pGameData->DeleteTable (pszTable);
			Assert (iErrCode == OK);
		}
	}
	
	// GameData(I.I)
	iErrCode = m_pGameData->DeleteTable (strGameData);
	Assert (iErrCode == OK);

	// Delete game from active game list
	char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
	GetGameClassGameNumber (iGameClass, iGameNumber, pszData);

	unsigned int iKey;
	iErrCode = m_pGameData->GetFirstKey (
		SYSTEM_ACTIVE_GAMES,
		SystemActiveGames::GameClassGameNumber,
		pszData,
		true,
		&iKey
		);

	if (iErrCode == OK && iKey != NO_KEY) {
		iErrCode = m_pGameData->DeleteRow (SYSTEM_ACTIVE_GAMES, iKey);
		Assert (iErrCode == OK);
	}

	else iErrCode = ERROR_GAME_DOES_NOT_EXIST;

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
		Assert (iErrCode2 == OK);
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
		char pszGameEmpires [256];
		
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

				unsigned int iNumEmpires;
				
				GetGameClassGameNumber (
					pszGame, 
					(*ppiGameClass) + *piNumGames, 
					(*ppiGameNumber) + *piNumGames
					);

				GET_GAME_EMPIRES (pszGameEmpires, (*ppiGameClass)[*piNumGames], (*ppiGameNumber)[*piNumGames]);
				
				// Subtle risk of deadlock here...
				//
				// We do this because we're racing against the CreateGame / EnterGame pair
				// For sanity and performance reasons, we want the creator of a game to
				// always be the first empire into the game
				iErrCode = m_pGameData->GetNumRows (pszGameEmpires, &iNumEmpires);
				if (iErrCode == OK && iNumEmpires > 0) {
					(*piNumGames) ++;
				}
			}
		}
Cleanup:
		m_pGameData->FreeKeys (piKey);
	}

	pGames->Release();

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

int GameEngine::CreateGame (int iGameClass, int iEmpireKey, const GameOptions& goGameOptions, int* piGameNumber) {

	int iNumUpdates, iErrCode;
	Variant vTemp, vOptions, vHalted, vPrivilege, vEmpireScore, vGameNumber, vMaxNumActiveGames;
	
	bool bGameClassLocked = false, bEmpireLocked = false, bGameLocked = false, 
		bIncrementedActiveGameCount = false, bDeleteRequired = false;

	char strTableName [512], strGameData [256];

	UTCTime tTime;
	Time::GetTime (&tTime);

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
	NamedMutex nmGameClassMutex, nmEmpireMutex, nmGameMutex;

	LockGameClass (iGameClass, &nmGameClassMutex);
	LockEmpire (iEmpireKey, &nmEmpireMutex);

	bGameClassLocked = bEmpireLocked = true;

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

	// Make sure empire isn't halted
	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vHalted);
	if (iErrCode != OK) {
		Assert (false);
		goto OnError;
	}

	if (vHalted.GetInteger() & EMPIRE_MARKED_FOR_DELETION) {
		iErrCode = ERROR_EMPIRE_IS_HALTED;
		goto OnError;
	}

	// Make sure empire is at least a novice
	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, &vPrivilege);
	if (iErrCode != OK) {
		Assert (false);
		goto OnError;
	}

	if (vPrivilege.GetInteger() < NOVICE) {
		iErrCode = ERROR_INSUFFICIENT_PRIVILEGE;
		goto OnError;
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
		LockGame (iGameClass, *piGameNumber, &nmGameMutex);
		bGameLocked = true;
		
		iErrCode = m_pGameData->InsertRow (SYSTEM_ACTIVE_GAMES, pvActiveGameData);
		if (iErrCode != OK) {
			Assert (false);
			goto OnError;
		}
		
		UnlockGameClass (nmGameClassMutex);
		bGameClassLocked = false;

	}	// Scope


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
		Variant vEmpireName;

		iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
		if (iErrCode != OK) {
			vEmpireName = "";
		}

		// Add row to GameData(I.I)
		Variant pvGameData [GameData::NumColumns] = {
			0,		// 0 is max num empires so far
			0,		// 0 updates
			tTime,	// Last updated now
			STILL_OPEN | GAME_CREATING,		// State
			MAX_COORDINATE,		// MinX
			0,		// 0 empires updated
			goGameOptions.pszPassword,	// Password
			MIN_COORDINATE,		// MaxX
			MAX_COORDINATE,		// MinY
			0,					// Zero paused
			MIN_COORDINATE,		// MaxY
			0,
			tTime,		// LastUpdateCheck
			tTime,		// CreationTime
			0,			// NumPlanetsPerEmpire
			0,			// HWAg
			0,			// AvgAg
			0,			// HWMin
			0,			// AvgMin
			0,			// HWFuel
			0,			// AvgFuel
			0,			// NumEmpiresResigned
			goGameOptions.iOptions,						// Options
			goGameOptions.iNumUpdatesBeforeGameCloses,	// NumUpdatesBeforeGameCloses
			goGameOptions.sFirstUpdateDelay,			// FirstUpdateDelay
			goGameOptions.pszEnterGameMessage,			// EnterGameMessage
			vEmpireName.GetCharPtr(),					// CreatorName
			goGameOptions.fMinAlmonasterScore,	// MinAlmonasterScore,
			goGameOptions.fMaxAlmonasterScore,	// MaxAlmonasterScore,
			goGameOptions.fMinClassicScore,		// MinClassicScore,
			goGameOptions.fMaxClassicScore,		// MaxClassicScore,
			goGameOptions.iMinBridierRank,		// MinBridierRank,
			goGameOptions.iMaxBridierRank,		// MaxBridierRank,
			goGameOptions.iMinBridierIndex,		// MinBridierIndex,
			goGameOptions.iMaxBridierIndex,		// MaxBridierIndex,
			goGameOptions.iMinBridierRankGain,	// MinBridierRankGain
			goGameOptions.iMaxBridierRankGain,	// MaxBridierRankGain
			goGameOptions.iMinWins,				// MinWins,
			goGameOptions.iMaxWins,				// MaxWins,
		};

		iErrCode = m_pGameData->InsertRow (strGameData, pvGameData);
		if (iErrCode != OK) {
			Assert (false);
			goto OnError;
		}

	}	// Scope

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

			int64 i64SessionId;
			unsigned int iRowEmpireKey = goGameOptions.pSecurity[i].iEmpireKey;

			// Lock empire
			NamedMutex nmLock;
			LockEmpire (iRowEmpireKey, &nmLock);

			// Get empire name
			iErrCode = GetEmpireName (iRowEmpireKey, pvGameSec + GameSecurity::EmpireName);
			if (iErrCode != OK) {
				UnlockEmpire (nmLock);
				continue;
			}

			// Check name - ignore if not matching
			if (stricmp (
				pvGameSec[GameSecurity::EmpireName].GetCharPtr(), 
				goGameOptions.pSecurity[i].pszEmpireName
				) != 0) {
				UnlockEmpire (nmLock);
				continue;
			}

			// Get empire ip address
			iErrCode = GetEmpireIPAddress (iRowEmpireKey, pvGameSec + GameSecurity::IPAddress);
			if (iErrCode != OK) {
				UnlockEmpire (nmLock);
				continue;
			}

			// Get empire session id
			iErrCode = GetEmpireSessionId (iRowEmpireKey, &i64SessionId);
			if (iErrCode != OK) {
				UnlockEmpire (nmLock);
				continue;
			}

			// Unlock empire
			UnlockEmpire (nmLock);

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

	// Add the game to our game table
	iErrCode = AddToGameTable (iGameClass, *piGameNumber);
	if (iErrCode != OK) {
		Assert (false);
		goto OnError;
	}

	// Unlock
	Assert (bEmpireLocked && bGameLocked && !bGameClassLocked);

	UnlockEmpire (nmEmpireMutex);
	UnlockGame (nmGameMutex);

	// Notification
	if (m_pUIEventSink != NULL) {
		m_pUIEventSink->OnCreateGame (iGameClass, *piGameNumber);
	}

	// Enter game
	return EnterGame (iGameClass, *piGameNumber, iEmpireKey, goGameOptions.pszPassword, &iNumUpdates);

OnError:

	int iErrCode2;

	if (bEmpireLocked) {
		UnlockEmpire (nmEmpireMutex);
	}

	if (bGameClassLocked) {
		UnlockGameClass (nmGameClassMutex);
	}

	if (bGameLocked) {
		UnlockGame (nmGameMutex);
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
						   int* piNumUpdates) {

	GAME_DATA (strGameData, iGameClass, iGameNumber);
	GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

	SYSTEM_EMPIRE_ACTIVE_GAMES (strEmpireActiveGames, iEmpireKey);

	GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_SHIPS (strGameEmpireShips, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

	String strDuplicateIPList, strDuplicateIdList, strDuplicateList, strMessage;
	bool bWarn, bBlock, bFlag, bAddedToGame = false, bClosed = false, bStarted = false, bUnPaused = false;

	int iErrCode, iNumTechs, iDefaultOptions, iGameState;
	unsigned int iCurrentNumEmpires = -1, i, iKey = NO_KEY;

	Variant vGameClassOptions, vHalted, vPassword, vPrivilege, vMinScore, vMaxScore, vEmpireScore, vTemp, 
		vMaxNumEmpires, vStillOpen, vNumUpdates, vMaxTechDev, vEmpireOptions, vEmpireName, vDiplomacyLevel,
		vGameOptions;

	bool bEmpireLocked = true, bGameLocked = true, bPauseLocked = false;

	Variant pvGameEmpireData [GameEmpireData::NumColumns];

	// Get time
	UTCTime tTime;
	Time::GetTime (&tTime);

	// Lock
	NamedMutex nmGameMutex, nmEmpireMutex, nmPause;

	LockGame (iGameClass, iGameNumber, &nmGameMutex);
	LockEmpire (iEmpireKey, &nmEmpireMutex);

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
	iErrCode = DoesEmpireExist (iEmpireKey, &bFlag);
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
	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vHalted);
	if (iErrCode != OK) {
		iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
		goto OnError;
	}
	
	if (vHalted.GetInteger() & EMPIRE_MARKED_FOR_DELETION) {
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

		// Get empire name
		iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
		if (iErrCode != OK) {
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
					char* pszMessage = (char*) StackAlloc (1024 + MAX_EMPIRE_NAME_LENGTH + strDuplicateList.GetLength());
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
					char* pszMessage = (char*) StackAlloc (1024 + MAX_EMPIRE_NAME_LENGTH + strDuplicateList.GetLength());
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

		// Check for pause
		LockPauseGame (iGameClass, iGameNumber, &nmPause);
		bPauseLocked = true;

		iErrCode = IsGamePaused (iGameClass, iGameNumber, &bFlag);
		if (iErrCode != OK) {
			Assert (false);
			goto OnError;
		}

		if (bFlag) {

			iErrCode = UnpauseGame (iGameClass, iGameNumber, false, false);
			if (iErrCode != OK) {
				Assert (false);
				goto OnError;
			}

			bUnPaused = true;
		}

		UnlockPauseGame (nmPause);
		bPauseLocked = false;
		
		// Trigger all remaining updates
		UnlockGame (nmGameMutex);
		bGameLocked = false;
	
		// Best effort
		iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, &bFlag);
		Assert (iErrCode == OK);

		LockGame (iGameClass, iGameNumber, &nmGameMutex);
		bGameLocked = true;
	}
	
	// Make sure updates didn't end the game
	iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
	if (iErrCode != OK || !bFlag) {
		iErrCode = ERROR_GAME_DOES_NOT_EXIST;
		goto OnError;
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
		
		if (vMaxNumEmpires.GetInteger() == (int) iCurrentNumEmpires) {
			
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
	pvGameEmpireData[GameEmpireData::Options] = 0;
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
		&pvGameEmpireData[GameEmpireData::TechUndevs]
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


	pvGameEmpireData[GameEmpireData::NumAvailableTechUndevs] = 
		min (GetNumTechs (pvGameEmpireData[GameEmpireData::TechUndevs].GetInteger()), iNumTechs);
	pvGameEmpireData[GameEmpireData::Econ] = 1;													// Econ
	pvGameEmpireData[GameEmpireData::Mil] = (float) 0.0;										// Mil
	pvGameEmpireData[GameEmpireData::TargetPop] = 0;
	pvGameEmpireData[GameEmpireData::HomeWorld] = NO_KEY;										// HWKey
	pvGameEmpireData[GameEmpireData::NumUpdatesIdle] = 0;									// 0 updates idle
	pvGameEmpireData[GameEmpireData::MaxBR] = GetBattleRank ((float) pvGameEmpireData[4]);	// Max BR
	pvGameEmpireData[GameEmpireData::BonusAg] = 0;					// 0 bonus ag
	pvGameEmpireData[GameEmpireData::BonusMin] = 0;					// 0 bonus min
	pvGameEmpireData[GameEmpireData::BonusFuel] = 0;				// 0 bonus fuel
	pvGameEmpireData[GameEmpireData::NumBuilds] = 0;				// No ships being built
	pvGameEmpireData[GameEmpireData::MinX] = MAX_COORDINATE;		// MinX
	pvGameEmpireData[GameEmpireData::MaxX] = MIN_COORDINATE;		// MaxX
	pvGameEmpireData[GameEmpireData::MinY] = MAX_COORDINATE;		// MinY
	pvGameEmpireData[GameEmpireData::MaxY] = MIN_COORDINATE;		// MaxY
	pvGameEmpireData[GameEmpireData::NextMaintenance] = 0;			// NextMaintenance
	pvGameEmpireData[GameEmpireData::NextFuelUse] = 0;				// NextFuelUse
	pvGameEmpireData[GameEmpireData::NextTotalPop] = pvGameEmpireData[GameEmpireData::TotalMin]; // NextMin
	pvGameEmpireData[GameEmpireData::NextMin] = 0;					// NextFuel
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

	iErrCode = m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::Options, 
		&vEmpireOptions
		);

	if (iErrCode != OK) {
		Assert (false);
		goto OnError;
	}

	// Grab default game options from empire's options
	iDefaultOptions = vEmpireOptions.GetInteger() & (
		AUTO_REFRESH | COUNTDOWN | GAME_REPEATED_BUTTONS | MAP_COLORING | SHIP_MAP_COLORING |
		SHIP_MAP_HIGHLIGHTING | SENSITIVE_MAPS | PARTIAL_MAPS | SHIPS_ON_MAP_SCREEN | 
		SHIPS_ON_PLANETS_SCREEN | LOCAL_MAPS_IN_UPCLOSE_VIEWS | GAME_DISPLAY_TIME | REJECT_INDEPENDENT_SHIP_GIFTS
		);

	pvGameEmpireData[GameEmpireData::Options] = 
		pvGameEmpireData[GameEmpireData::Options].GetInteger() | iDefaultOptions;

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
	UnlockEmpire (nmEmpireMutex);
	bEmpireLocked = false;

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
			pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + MAX_ENTER_GAME_MESSAGE_LENGTH + 128);

			sprintf (
				(char*) pszMessage,
				"%s, the creator of this game, says the following:" NEW_PARAGRAPH "%s",
				vCreatorName.GetCharPtr(),
				vTemp.GetCharPtr()
				);
		}

		iErrCode = SendGameMessage (iGameClass, iGameNumber, iEmpireKey, pszMessage, SYSTEM);
		if (iErrCode != OK) {
			Assert (false);
			goto OnError;
		}
	}

	if (iCurrentNumEmpires > 1) {

		unsigned int iNumEmpires;
		Variant* pvEmpireKey;
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
				
				if (pvEmpireKey[i] != iEmpireKey) {
					
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
		
		for (i = 0; i < iNumEmpires; i ++) {

			if (pvEmpireKey[i].GetInteger() != iEmpireKey) {
				
				// Best effort
				iErrCode = SendGameMessage (
					iGameClass, 
					iGameNumber, 
					pvEmpireKey[i], 
					strMessage, 
					SYSTEM, 
					true
					);
				Assert (iErrCode == OK);
			}
		}
		
		// Clean up
		m_pGameData->FreeData (pvEmpireKey);
	}

	/////////////////////////
	// Game starting stuff //
	/////////////////////////

	// Has game started?	
	if (iGameState & STARTED) {
		
		// Add self to map;  game has already started
		iErrCode = AddEmpiresToMap (iGameClass, iGameNumber, &iEmpireKey, 1, &bFlag);
		if (iErrCode != OK) {

			int iErrCode2;

			if (bFlag) {

				Assert (false);
				
				// Catastrophic - delete the game
				iErrCode2 = CleanupGame (iGameClass, iGameNumber);
				Assert (iErrCode2 == OK);
			
			} else {

				iErrCode2 = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey);
				Assert (iErrCode2 == OK);
			}

			UnlockGame (nmGameMutex);
			
			return ERROR_COULD_NOT_CREATE_PLANETS;
		}
		
	} else {
		
		// Are we the trigger for the game to begin?
		unsigned int iNumActiveEmpires, iNumKeys;
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

			// Write max num empires
			iErrCode = m_pGameData->WriteData (strGameData, GameData::MaxNumEmpires, iCurrentNumEmpires);
			if (iErrCode != OK) {
				Assert (false);
				goto OnError;
			}
			
			// Add all players to map
			Variant* pvKey;
			int* piEmpKey;
			
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
				int iErrCode2 = CleanupGame (iGameClass, iGameNumber);
				Assert (iErrCode2 == OK);
				
				UnlockGame (nmGameMutex);
				m_pGameData->FreeData (pvKey);
				
				return ERROR_COULD_NOT_CREATE_PLANETS;
			}

			m_pGameData->FreeData (pvKey);
		}
	}

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
	
	UnlockGame (nmGameMutex);

	// Entry was sucessful	
	return OK;

OnError:

	if ((iCurrentNumEmpires == 0 || (bAddedToGame && iCurrentNumEmpires == 1)) && (
		iErrCode != ERROR_GAME_DOES_NOT_EXIST && 
		iErrCode != ERROR_GAMECLASS_DOES_NOT_EXIST
		)) {

		// Best effort tear down game
		int iErrCode2 = CleanupGame (iGameClass, iGameNumber);
		Assert (iErrCode2 == OK);

	} else {

		if (bAddedToGame) {

			// Remove empire from game
			int iErrCode2 = DeleteEmpireFromGame (
				iGameClass,
				iGameNumber,
				iEmpireKey
				);
			Assert (iErrCode2 == OK);

			// Keep game alive
			iErrCode2 = m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_ADDING_EMPIRE);
			Assert (iErrCode2 == OK);
		}
	}

	if (bGameLocked) {
		UnlockGame (nmGameMutex);
	}

	if (bEmpireLocked) {
		UnlockEmpire (nmEmpireMutex);
	}

	if (bPauseLocked) {
		UnlockPauseGame (nmPause);
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
// *piPlayers -> Text of Game Message
//
// Return the largest number of empires that were ever a given game

int GameEngine::GetMaxNumActiveEmpiresInGame (int iGameClass, int iGameNumber, int* piEmpires) {
	
	int iErrCode = OK;

	Variant vEmpires;
	GAME_DATA (pszGameData, iGameClass, iGameNumber);

	iErrCode = m_pGameData->ReadData (pszGameData, SystemGameClassData::MaxNumEmpires, &vEmpires);
	if (iErrCode == OK) {
		*piEmpires = vEmpires.GetInteger();
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

	bool bTestAlly = true, bTestDraw = true;

	*pbEndGame = false;

	GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

	unsigned int i, j, iTemp, iNumEmpires;
	Variant* pvEmpireKey, * pvDipStatus;
	
	int iErrCode = m_pGameData->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, &pvEmpireKey, &iNumEmpires);
	if (iErrCode != OK) {
		return iErrCode;
	}

	char pszDiplomacy [256];

	for (i = 0; i < iNumEmpires && (bTestAlly || bTestDraw); i ++) {
		
		GET_GAME_EMPIRE_DIPLOMACY (pszDiplomacy, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

		iErrCode = m_pGameData->ReadColumn (
			pszDiplomacy, 
			GameEmpireDiplomacy::CurrentStatus, 
			&pvDipStatus, 
			&iTemp
			);

		if (iErrCode == ERROR_DATA_NOT_FOUND || iTemp != iNumEmpires - 1) {

			bTestAlly = false;
			bTestDraw = false;

		} else {

			if (iErrCode != OK) {
				return iErrCode;
			}

			Assert (iTemp > 0);

			for (j = 0; j < iTemp && (bTestAlly || bTestDraw); j ++) {
				
				switch (pvDipStatus[j].GetInteger()) {
					
				case DRAW:
					bTestAlly = false;
					break;
					
				case ALLIANCE:
					bTestDraw = false;
					break;
					
				default:
					
					bTestAlly = false;
					bTestDraw = false;
					break;
				}
			}
			
			m_pGameData->FreeData (pvDipStatus);
		}
	}
	
	// Test for ally out
	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
	char pszMessage [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];

	iErrCode = GetGameClassName (iGameClass, pszGameClassName);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (bTestAlly) {
		
		// Send victory message to all remaining empires
		sprintf (
			pszMessage, 
			"Congratulations! You have won %s %i", 
			pszGameClassName, 
			iGameNumber
			);
		
		for (i = 0; i < iNumEmpires; i ++) {	
			
			// Best effort send messages
			if (!String::IsBlank (pszAdminMessage)) {
				iErrCode = SendSystemMessage (pvEmpireKey[i].GetInteger(), pszAdminMessage, SYSTEM);
				Assert (iErrCode == OK);
			}
			
			iErrCode = SendSystemMessage (pvEmpireKey[i].GetInteger(), pszMessage, SYSTEM);
			Assert (iErrCode == OK);
			
			// Best effort update empires' statistics
			iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			Assert (iErrCode == OK);
		}
		
		// Game over
		*pbEndGame = true;
		
		// Best effort
		iErrCode = CleanupGame (iGameClass, iGameNumber);
		Assert (iErrCode == OK);

	}
	
	else if (bTestDraw) {

		// Send draw message to all remaining empires
		sprintf (pszMessage, "You have drawn %s %i", pszGameClassName, iGameNumber);

		for (i = 0; i < iNumEmpires; i ++) {
			
			// Best effort send messages
			if (!String::IsBlank (pszAdminMessage)) {
				iErrCode = SendSystemMessage (pvEmpireKey[i].GetInteger(), pszAdminMessage, SYSTEM);
				Assert (iErrCode == OK);
			}
			
			iErrCode = SendSystemMessage (pvEmpireKey[i].GetInteger(), pszMessage, SYSTEM);
			Assert (iErrCode == OK);

			// Best effort update empires' statistics
			iErrCode = UpdateScoresOnDraw (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			Assert (iErrCode == OK);
		}
		
		// Game over
		*pbEndGame = true;
		
		iErrCode = CleanupGame (iGameClass, iGameNumber);
		Assert (iErrCode == OK);
	}

	return iErrCode;
}

int GameEngine::AdminPauseGame (int iGameClass, int iGameNumber, bool bBroadcast) {

	Variant vTemp;

	int iErrCode = WaitGameReader (iGameClass, iGameNumber);
	if (iErrCode != OK) {
		return iErrCode;
	}

	GAME_DATA (strGameData, iGameClass, iGameNumber);

	NamedMutex nmMutex;
	LockPauseGame (iGameClass, iGameNumber, &nmMutex);
	
	iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	if (!(vTemp.GetInteger() & ADMIN_PAUSED)) {
		iErrCode = PauseGame (iGameClass, iGameNumber, true, bBroadcast);
		Assert (iErrCode == OK);
	}

Cleanup:

	UnlockPauseGame (nmMutex);

	SignalGameReader (iGameClass, iGameNumber);

	return iErrCode;
}

int GameEngine::AdminUnpauseGame (int iGameClass, int iGameNumber, bool bBroadcast) {

	Variant vTemp;

	int iErrCode = WaitGameReader (iGameClass, iGameNumber);
	if (iErrCode != OK) {
		return iErrCode;
	}

	GAME_DATA (strGameData, iGameClass, iGameNumber);

	NamedMutex nmMutex;
	LockPauseGame (iGameClass, iGameNumber, &nmMutex);
	
	iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	if (vTemp.GetInteger() & ADMIN_PAUSED) {
		iErrCode = UnpauseGame (iGameClass, iGameNumber, true, bBroadcast);
		Assert (iErrCode == OK);
	}

Cleanup:

	UnlockPauseGame (nmMutex);

	SignalGameReader (iGameClass, iGameNumber);

	return iErrCode;
}

int GameEngine::PauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) {

	int iErrCode, iSecondsSince, iSecondsUntil, iNumUpdates, iState;

	bool bFlag;
	const char* pszMessage;

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

	// Normalize for weekend delays
	if (iSecondsUntil > vSecPerUpdate.GetInteger()) {

		if (iNumUpdates == 0) {

			Variant vDelay;
			iErrCode = m_pGameData->ReadData (strGameData, GameData::FirstUpdateDelay, &vDelay);
			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}

			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}
			
			if (iSecondsUntil > vSecPerUpdate.GetInteger() + vDelay.GetInteger()) {
				iSecondsUntil = vSecPerUpdate.GetInteger() + vDelay.GetInteger();
			}

		} else {

			iSecondsUntil = vSecPerUpdate.GetInteger();
		}
	}

	if (bAdmin && !(iState & ADMIN_PAUSED)) {

		iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, ADMIN_PAUSED);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
	}
	
	if (iState & PAUSED) {
		return OK;
	}
	
	iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, PAUSED);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}
	
	pszMessage = bAdmin ? "The game was paused by an administrator" : "The game is now paused";
	
	// Write down remaining seconds
	iErrCode = m_pGameData->WriteData (
		strGameData, 
		GameData::SecondsRemainingInUpdateWhilePaused, 
		iSecondsUntil
		);

	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	// Best effort broadcast message
	if (bBroadcast) {
		iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, false);
		Assert (iErrCode == OK);
	}

	return iErrCode;
}


int GameEngine::UnpauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) {

	int iErrCode;
	const char* pszMessage;

	GAME_DATA (strGameData, iGameClass, iGameNumber);

	if (bAdmin) {

		Variant vTemp;
		iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
		if (iErrCode != OK) {
			return ERROR_GAME_DOES_NOT_EXIST;
		}

		if (!(vTemp.GetInteger() & ADMIN_PAUSED)) {
			return ERROR_FAILURE;
		}

		iErrCode = m_pGameData->ReadData (strGameData, GameData::NumPaused, &vTemp);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);
		unsigned int iNumEmpires;

		iErrCode = m_pGameData->GetNumRows (strEmpires, &iNumEmpires);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		// Unpause the game
		iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~ADMIN_PAUSED);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		if (iNumEmpires == (unsigned int) vTemp.GetInteger()) {

			// Still paused
			iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, PAUSED);
			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}

			return OK;
		}

		pszMessage = "The game was unpaused by an administrator";

		// Fall through to unpause

	} else {

		pszMessage = "The game is no longer paused";
	}

	// Best effort broadcast message
	if (bBroadcast) {
		iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, false);
		Assert (iErrCode == OK);
	}

	// No longer paused
	iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~PAUSED);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}
	
	// Set seconds remaining, last update time
	Variant vSecsRemaining;
	iErrCode = m_pGameData->ReadData (
		strGameData, 
		GameData::SecondsRemainingInUpdateWhilePaused, 
		&vSecsRemaining
		);
	
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

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

	UTCTime tTime, tTimeAdd;
	Time::GetTime (&tTime);
	
	Time::AddSeconds (tTime, (Seconds) (vSecsRemaining.GetInteger() - vSecPerUpdate.GetInteger()), &tTimeAdd);

	// If first update delay and first update, subtract first update delay
	Variant vNumUpdates;
	iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (vNumUpdates.GetInteger() == 0) {

		Variant vDelay;
		iErrCode = m_pGameData->ReadData (strGameData, GameData::FirstUpdateDelay, &vDelay);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
	
		tTimeAdd -= vDelay.GetInteger();
		Assert (tTimeAdd <= tTime);
	}

	iErrCode = m_pGameData->WriteData (strGameData, GameData::LastUpdateTime, tTimeAdd);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

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

				iErrCode = AdminPauseGame (iGameClass, iGameNumber, true);
				Assert (iErrCode == OK);
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

			// Best effort
			iErrCode = AdminUnpauseGame (iGameClass, iGameNumber, true);
			Assert (iErrCode == OK);
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

	char pszGame [128];
	sprintf (pszGame, "%i.%i", iGameClass, iGameNumber);

	GameObject* pGameObject = new GameObject();
	if (pGameObject == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	int iErrCode = pGameObject->SetName (pszGame);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	rwGameObjectTableLock.WaitWriter();
	bool bRetVal = m_htGameObjectTable.Insert (pGameObject->GetName(), pGameObject);
	rwGameObjectTableLock.SignalWriter();

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

	rwGameObjectTableLock.WaitWriter();
	bool bRetVal = m_htGameObjectTable.DeleteFirst (pszGame, NULL, &pGameObject);

	if (bRetVal) {
		pGameObject->Release();
	}
	rwGameObjectTableLock.SignalWriter();

	return bRetVal ? OK : ERROR_GAME_DOES_NOT_EXIST;
}

GameObject* GameEngine::GetGameObject (int iGameClass, int iGameNumber) {

	GameObject* pGameObject = NULL;

	char pszGame [128];
	sprintf (pszGame, "%i.%i", iGameClass, iGameNumber);

	rwGameObjectTableLock.WaitReader();
	bool bRetVal = m_htGameObjectTable.FindFirst (pszGame, &pGameObject);

	if (bRetVal) {
		pGameObject->AddRef();
	}

	rwGameObjectTableLock.SignalReader();
	
	return pGameObject;
}

int GameEngine::LogEmpireIntoGame (int iGameClass, int iGameNumber, int iEmpireKey) {

	int iErrCode;

	GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

	IWriteTable* pTable;

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

	pTable->Release();

	return iErrCode;
}

int GameEngine::RuinGame (int iGameClass, int iGameNumber) {

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

		// Best effort
		iErrCode = RuinEmpire (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger(), pszMessage);
		Assert (iErrCode == OK);
	}

Cleanup:

	if (pvEmpireKey != NULL) {
		m_pGameData->FreeData (pvEmpireKey);
	}

	// Kill the game
	iErrCode = CleanupGame (iGameClass, iGameNumber);
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

	NamedMutex nmMutex;
	LockGame (iGameClass, iGameNumber, &nmMutex);

	Variant vResigned;
	iErrCode = m_pGameData->ReadData (pszTable, GameData::NumEmpiresResigned, &vResigned);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (vResigned.GetInteger() == 0) {
		goto Cleanup;
	}

	// Output
	piEmpireKey = new int [vResigned.GetInteger()];
	if (piEmpireKey == NULL) {
		iErrCode = ERROR_OUT_OF_MEMORY;
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

	UnlockGame (nmMutex);

	if (pvEmpireKey != NULL) {
		m_pGameData->FreeData (pvEmpireKey);
	}

	if (iErrCode != OK && piEmpireKey != NULL) {
		delete [] piEmpireKey;
	}

	return iErrCode;

}

int GameEngine::GetGameEntryRestrictions (int iGameClass, int iGameNumber, int* piOptions, 
										  Variant pvRestrictionMin [NUM_ENTRY_SCORE_RESTRICTIONS], 
										  Variant pvRestrictionMax [NUM_ENTRY_SCORE_RESTRICTIONS]) {

	int iErrCode, iOptions;

	Variant vOptions;

	GAME_DATA (pszGameData, iGameClass, iGameNumber);

	// Read game options
	iErrCode = m_pGameData->ReadData (pszGameData, GameData::Options, &vOptions);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iOptions = *piOptions = vOptions.GetInteger();

	// Almonaster
	if (iOptions & GAME_RESTRICT_MIN_ALMONASTER_SCORE) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinAlmonasterScore, pvRestrictionMin + RESTRICT_ALMONASTER_SCORE);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	if (iOptions & GAME_RESTRICT_MAX_ALMONASTER_SCORE) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxAlmonasterScore, pvRestrictionMax + RESTRICT_ALMONASTER_SCORE);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	// Classic
	if (iOptions & GAME_RESTRICT_MIN_CLASSIC_SCORE) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinClassicScore, pvRestrictionMin + RESTRICT_CLASSIC_SCORE);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	if (iOptions & GAME_RESTRICT_MAX_CLASSIC_SCORE) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxClassicScore, pvRestrictionMax + RESTRICT_CLASSIC_SCORE);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	// Bridier Rank
	if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRank, pvRestrictionMin + RESTRICT_BRIDIER_RANK);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRank, pvRestrictionMax + RESTRICT_BRIDIER_RANK);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	// Bridier Rank
	if (iOptions & GAME_RESTRICT_MIN_BRIDIER_INDEX) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierIndex, pvRestrictionMin + RESTRICT_BRIDIER_INDEX);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	if (iOptions & GAME_RESTRICT_MAX_BRIDIER_INDEX) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierIndex, pvRestrictionMax + RESTRICT_BRIDIER_INDEX);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	// Bridier Rank Gain
	if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRankGain, pvRestrictionMin + RESTRICT_BRIDIER_RANK_GAIN);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRankGain, pvRestrictionMax + RESTRICT_BRIDIER_RANK_GAIN);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	// Wins
	if (iOptions & GAME_RESTRICT_MIN_WINS) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinWins, pvRestrictionMin + RESTRICT_WINS);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	if (iOptions & GAME_RESTRICT_MAX_WINS) {

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxWins, pvRestrictionMax + RESTRICT_WINS);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

Cleanup:

	return iErrCode;
}

int GameEngine::GameAccessCheck (int iGameClass, int iGameNumber, int iEmpireKey, 
								 const GameOptions* pgoGameOptions, GameAction gaAction, 
								 bool* pbAccess) {

	int iErrCode, iOptions, iPrivilege;
	Variant vGame, vEmpire;

	char pszGameData [256] = "";

	IReadTable* pGameSec = NULL;
	IWriteTable* pWriteSec = NULL;

	// Fast resolution for admins, guests
	iErrCode = GetEmpirePrivilege (iEmpireKey, &iPrivilege);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	if (iPrivilege == ADMINISTRATOR) {
		*pbAccess = true;
		goto Cleanup;
	}

	if (iPrivilege == GUEST) {
		*pbAccess = gaAction == VIEW_GAME;
		goto Cleanup;
	}

	// Read game options
	if (pgoGameOptions == NULL) {

		// The gameclass already exists, so we can allow owners through quickly
		Variant vGameClassOwner;

		iErrCode = m_pGameData->ReadData (
			SYSTEM_GAMECLASS_DATA, 
			iGameClass, 
			SystemGameClassData::Owner, 
			&vGameClassOwner
			);

		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (vGameClassOwner.GetInteger() == iEmpireKey) {
			*pbAccess = true;
			goto Cleanup;
		}

		GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

		iErrCode = m_pGameData->ReadData (pszGameData, GameData::Options, &vGame);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		iOptions = vGame.GetInteger();
	
	} else {
		
		iOptions = pgoGameOptions->iOptions;
	}

	// Default to allow access
	*pbAccess = true;

	// Almonaster
	if (iOptions & (GAME_RESTRICT_MIN_ALMONASTER_SCORE | GAME_RESTRICT_MAX_ALMONASTER_SCORE)) {

		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vEmpire);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iOptions & GAME_RESTRICT_MIN_ALMONASTER_SCORE) {

			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinAlmonasterScore, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->fMinAlmonasterScore;
			}

			if (vEmpire.GetFloat() < vGame.GetFloat()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}
		
		if (iOptions & GAME_RESTRICT_MAX_ALMONASTER_SCORE) {
			
			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxAlmonasterScore, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}
			
			} else {

				vGame = pgoGameOptions->fMaxAlmonasterScore;
			}

			if (vEmpire.GetFloat() > vGame.GetFloat()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}
	}

	// Classic
	if (iOptions & (GAME_RESTRICT_MIN_CLASSIC_SCORE | GAME_RESTRICT_MAX_CLASSIC_SCORE)) {

		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::ClassicScore, &vEmpire);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iOptions & GAME_RESTRICT_MIN_CLASSIC_SCORE) {

			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinClassicScore, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->fMinClassicScore;
			}

			if (vEmpire.GetFloat() < vGame.GetFloat()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}

		if (iOptions & GAME_RESTRICT_MAX_CLASSIC_SCORE) {
			
			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxClassicScore, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->fMaxClassicScore;
			}

			if (vEmpire.GetFloat() > vGame.GetFloat()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}
	}

	// Bridier Rank
	if (iOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK | GAME_RESTRICT_MAX_BRIDIER_RANK)) {

		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierRank, &vEmpire);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK) {

			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRank, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->iMinBridierRank;
			}

			if (vEmpire.GetInteger() < vGame.GetInteger()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}

		if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK) {
			
			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRank, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->iMaxBridierRank;
			}

			if (vEmpire.GetInteger() > vGame.GetInteger()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}
	}

	// Bridier Index
	if (iOptions & (GAME_RESTRICT_MIN_BRIDIER_INDEX | GAME_RESTRICT_MAX_BRIDIER_INDEX)) {

		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierIndex, &vEmpire);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iOptions & GAME_RESTRICT_MIN_BRIDIER_INDEX) {

			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierIndex, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->iMinBridierIndex;
			}

			if (vEmpire.GetInteger() < vGame.GetInteger()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}

		if (iOptions & GAME_RESTRICT_MAX_BRIDIER_INDEX) {

			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierIndex, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->iMaxBridierIndex;
			}

			if (vEmpire.GetInteger() > vGame.GetInteger()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}
	}

	// Bridier Rank Gain
	if (pgoGameOptions == NULL &&
		iOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN | GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN)) {

		Variant vGameRank, vGameIndex, vOwnerKey;
		int iNukerRankChange, iNukerIndexChange, iNukedRankChange, iNukedIndexChange, iEmpireRank, iEmpireIndex;

		GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

		unsigned int iNumRows;

		iErrCode = m_pGameData->GetNumRows (pszGameEmpires, &iNumRows);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iNumRows > 0) {

			Assert (iNumRows == 1);

			iErrCode = GetBridierScore (iEmpireKey, &iEmpireRank, &iEmpireIndex);
			if (iErrCode != OK) {
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->ReadData (pszGameEmpires, 0, GameEmpires::EmpireKey, &vOwnerKey);
			if (iErrCode != OK) {
				goto Cleanup;
			}
			
			GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, vOwnerKey.GetInteger());
			
			iErrCode = m_pGameData->ReadData (pszEmpireData, GameEmpireData::InitialBridierRank, &vGameRank);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->ReadData (pszEmpireData, GameEmpireData::InitialBridierIndex, &vGameIndex);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			BridierObject::GetScoreChanges (
				vGameRank.GetInteger(), 
				vGameIndex.GetInteger(), 
				iEmpireRank, 
				iEmpireIndex,
				&iNukerRankChange, 
				&iNukerIndexChange, 
				&iNukedRankChange, 
				&iNukedIndexChange
				);
			
			if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN) {
				
				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRankGain, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}
				
				if (iNukerRankChange < vGame.GetInteger()) {
					*pbAccess = false;
					goto Cleanup;
				}
			}
			
			if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN) {
				
				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRankGain, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}
				
				if (iNukerRankChange > vGame.GetInteger()) {
					*pbAccess = false;
					goto Cleanup;
				}
			}
		}
	}

	// Wins
	if (iOptions & (GAME_RESTRICT_MIN_WINS | GAME_RESTRICT_MAX_WINS)) {

		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Wins, &vEmpire);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iOptions & GAME_RESTRICT_MIN_WINS) {

			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinWins, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->iMinWins;
			}

			if (vEmpire.GetInteger() < vGame.GetInteger()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}

		if (iOptions & GAME_RESTRICT_MAX_WINS) {

			if (pgoGameOptions == NULL) {

				iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxWins, &vGame);
				if (iErrCode != OK) {
					goto Cleanup;
				}

			} else {

				vGame = pgoGameOptions->iMaxWins;
			}

			if (vEmpire.GetInteger() > vGame.GetInteger()) {
				*pbAccess = false;
				goto Cleanup;
			}
		}
	}

	// Check for block on specific empire
	if (iOptions & GAME_ENFORCE_SECURITY) {

		LinkedList<unsigned int> llBrokenList;

		unsigned int iKey = NO_KEY, iNumSecEntries = 0;

		int iSecKey, iSecOptions;
		int64 i64SessionId = NO_SESSION_ID, i64EmpireSessionId = NO_SESSION_ID;
		const char* pszIPAddress = NULL, * pszEmpireName = NULL;

		bool bFlag;

		Variant vEmpireIPAddress, vNewIPAddress, vNewSessionId;

		if (pgoGameOptions == NULL) {

			GET_GAME_SECURITY (pszGameData, iGameClass, iGameNumber);

			iErrCode = m_pGameData->GetTableForReading (pszGameData, &pGameSec);
			if (iErrCode != OK) {
				goto Cleanup;
			}
		}

		while (true) {
			
			if (pgoGameOptions == NULL) {

				bool bBroken = false;
			
				// Fetch a row from the table
				iErrCode = pGameSec->GetNextKey (iKey, &iKey);
				if (iErrCode == ERROR_DATA_NOT_FOUND) {
					iErrCode = OK;
					break;
				}

				if (iErrCode != OK) {
					goto Cleanup;
				}
				
				iErrCode = pGameSec->ReadData (iKey, GameSecurity::EmpireKey, &iSecKey);
				if (iErrCode != OK) {
					goto Cleanup;
				}

				if (iSecKey != NO_KEY) {

					iErrCode = pGameSec->ReadData (iKey, GameSecurity::EmpireName, &pszEmpireName);
					if (iErrCode != OK) {
						goto Cleanup;
					}

					iErrCode = DoesEmpireKeyMatchName (iSecKey, pszEmpireName, &bFlag);
					if (iErrCode != OK || !bFlag) {
						iSecKey = NO_KEY;
						bBroken = true;						
					}
				}
				
				iErrCode = pGameSec->ReadData (iKey, GameSecurity::Options, &iSecOptions);
				if (iErrCode != OK) {
					goto Cleanup;
				}

				if (iSecOptions & GAME_SECURITY_CHECK_SESSIONID) {

					iErrCode = pGameSec->ReadData (iKey, GameSecurity::SessionId, &i64SessionId);
					if (iErrCode != OK) {
						goto Cleanup;
					}

					if (iSecKey != NO_KEY) {

						// See if empire's session id is still valid
						iErrCode = m_pGameData->ReadData (
							SYSTEM_EMPIRE_DATA,
							iSecKey,
							SystemEmpireData::SessionId,
							&vNewSessionId
							);
						
						if (iErrCode != OK) {
							iSecKey = NO_KEY;
							iErrCode = OK;
							bBroken = true;
						}
						
						else if (i64SessionId != vNewSessionId.GetInteger64()) {
							bBroken = true;
							i64SessionId = vNewSessionId.GetInteger64();
						}
					}
				}
				
				if (iSecOptions & GAME_SECURITY_CHECK_IPADDRESS) {

					iErrCode = pGameSec->ReadData (iKey, GameSecurity::IPAddress, &pszIPAddress);
					if (iErrCode != OK) {
						goto Cleanup;
					}

					if (iSecKey != NO_KEY) {

						// See if empire's ip address is still valid
						iErrCode = m_pGameData->ReadData (
							SYSTEM_EMPIRE_DATA,
							iSecKey,
							SystemEmpireData::IPAddress,
							&vNewIPAddress
							);

						if (iErrCode != OK) {
							iSecKey = NO_KEY;
							iErrCode = OK;
							bBroken = true;						
						}
						
						else if (strcmp (pszIPAddress, vNewIPAddress.GetCharPtr()) != 0) {
							bBroken = true;
							pszIPAddress = vNewIPAddress.GetCharPtr();
						}
					}
				}
				
				// If the row is broken, take note
				if (bBroken) {

					if (!llBrokenList.PushLast (iKey)) {
						iErrCode = ERROR_OUT_OF_MEMORY;
						goto Cleanup;
					}
				}

			} else {

				// Fetch a row from the options menu
				if (iNumSecEntries == pgoGameOptions->iNumSecurityEntries) {
					break;
				}

				iSecKey = pgoGameOptions->pSecurity[iNumSecEntries].iEmpireKey;
				iSecOptions = pgoGameOptions->pSecurity[iNumSecEntries].iOptions;

				// Don't check session ids and ip addresses here
				iSecOptions &= ~(GAME_SECURITY_CHECK_SESSIONID | GAME_SECURITY_CHECK_IPADDRESS);

				iNumSecEntries ++;
			}
			
			// Check empire key
			if (iEmpireKey == iSecKey) {
				
				// Access denied
				*pbAccess = false;
				break;
			}
			
			// Check session id
			if (iSecOptions & GAME_SECURITY_CHECK_SESSIONID) {

				Assert (i64SessionId != NO_SESSION_ID);
				
				// Fault in session id
				if (i64EmpireSessionId == NO_SESSION_ID) {
					
					iErrCode = GetEmpireSessionId (iEmpireKey, &i64EmpireSessionId);
					if (iErrCode != OK) {
						goto Cleanup;
					}
				}
				
				if (i64EmpireSessionId == i64SessionId) {
					
					// Access denied
					*pbAccess = false;
					break;
				}
			}

			// Check ip address
			if (iSecOptions & GAME_SECURITY_CHECK_IPADDRESS) {

				Assert (pszIPAddress != NULL);
				
				// Fault in ip address
				if (vEmpireIPAddress.GetType() != V_STRING) {
					
					iErrCode = GetEmpireIPAddress (iEmpireKey, &vEmpireIPAddress);
					if (iErrCode != OK) {
						goto Cleanup;
					}
				}
				
				if (strcmp (vEmpireIPAddress.GetCharPtr(), pszIPAddress) == 0) {
					
					// Access denied
					*pbAccess = false;
					break;
				}
			}

		}	// End while loop

		if (llBrokenList.GetNumElements() > 0) {

			ListIterator<unsigned int> li;

			GAME_SECURITY (pszGameSec, iGameClass, iGameNumber);

			pGameSec->Release();
			pGameSec = NULL;

			iErrCode = m_pGameData->GetTableForWriting (pszGameSec, &pWriteSec);
			if (iErrCode != OK) {
				goto Cleanup;
			}

			iErrCode = pWriteSec->QueryInterface (IID_IReadTable, (void**) &pGameSec);
			if (iErrCode != OK) {
				goto Cleanup;
			}

			while (llBrokenList.PopFirst (&li)) {

				iKey = li.GetData();

				iErrCode = pGameSec->ReadData (iKey, GameSecurity::EmpireKey, &iSecKey);
				if (iErrCode != OK) {
					goto Cleanup;
				}

				Assert (iSecKey != NO_KEY);	// Otherwise we shouldn't be broken

				iErrCode = pGameSec->ReadData (iKey, GameSecurity::EmpireName, &pszEmpireName);
				if (iErrCode != OK) {
					goto Cleanup;
				}

				// See if empire's ip address is still valid
				iErrCode = DoesEmpireKeyMatchName (iSecKey, pszEmpireName, &bFlag);
				if (iErrCode != OK || !bFlag) {
					
					// No more empire key
					iErrCode = pWriteSec->WriteData (iKey, GameSecurity::EmpireKey, (int) NO_KEY);
					if (iErrCode != OK) {
						goto Cleanup;
					}
				}
				
				else if (bFlag) {

					iErrCode = m_pGameData->ReadData (
						SYSTEM_EMPIRE_DATA,
						iSecKey,
						SystemEmpireData::SessionId,
						&vNewSessionId
						);

					if (iErrCode != OK) {
						
						// No more empire key
						iErrCode = pWriteSec->WriteData (iKey, GameSecurity::EmpireKey, (int) NO_KEY);
						if (iErrCode != OK) {
							goto Cleanup;
						}

					} else {
						
						iErrCode = pGameSec->ReadData (iKey, GameSecurity::SessionId, &i64SessionId);
						if (iErrCode != OK) {
							goto Cleanup;
						}

						if (i64SessionId != vNewSessionId.GetInteger64()) {
							
							// New Session Id
							iErrCode = pWriteSec->WriteData (iKey, GameSecurity::SessionId, vNewSessionId);
							if (iErrCode != OK) {
								goto Cleanup;
							}
						}

						iErrCode = m_pGameData->ReadData (
							SYSTEM_EMPIRE_DATA,
							iSecKey,
							SystemEmpireData::IPAddress,
							&vNewIPAddress
							);

						if (iErrCode != OK) {
							
							// No more empire key
							iErrCode = pWriteSec->WriteData (iKey, GameSecurity::EmpireKey, (int) NO_KEY);
							if (iErrCode != OK) {
								goto Cleanup;
							}

						} else {
							
							iErrCode = pGameSec->ReadData (iKey, GameSecurity::IPAddress, &pszIPAddress);
							if (iErrCode != OK) {
								goto Cleanup;
							}
							
							if (strcmp (pszIPAddress, vNewIPAddress.GetCharPtr()) != 0) {
							
								// New IP address
								iErrCode = pWriteSec->WriteData (iKey, GameSecurity::IPAddress, vNewIPAddress.GetCharPtr());
								if (iErrCode != OK) {
									goto Cleanup;
								}
							}
						}
					}
				}
			}

			pWriteSec->Release();
			pGameSec->Release();

			pWriteSec = NULL;
			pGameSec = NULL;

			llBrokenList.Clear();
		
		}	// End if any rows were broken

	}	// End if enforce security

Cleanup:

	if (pGameSec != NULL) {
		pGameSec->Release();
	}

	if (pWriteSec != NULL) {
		pWriteSec->Release();
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