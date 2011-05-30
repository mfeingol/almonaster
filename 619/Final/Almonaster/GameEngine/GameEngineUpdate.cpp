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

// Input:
// iGameClass -> Gameclass
// iGameNumber - > Gamenumber
//
// Output:
// *pbUpdate -> true if an update occurred
//
// Executes an update if one has occurred.  Also updates game lastlogin data

int GameEngine::CheckGameForUpdates (int iGameClass, int iGameNumber, bool* pbUpdate) {

	int iErrCode;

	*pbUpdate = false;

	bool bGameOver = false, bExist, bGameWriter = false;
	int i, iSecondsSince, iSecondsUntil, iNumUpdates, iState, iAdjustment = 0;

	Variant vState, vNumPrevUpdates, vTemp, vOptions, vAfterWeekendDelay, vFirstUpdateDelay;

	GAME_DATA (strGameData, iGameClass, iGameNumber);

	Seconds iPeriod;
	UTCTime tNow, tTime, tUpdateTime;
	
	// Get time
	Time::GetTime (&tNow);

	// Lock game
	NamedMutex nmGameMutex, nmPauseMutex;
	
	LockGame (iGameClass, iGameNumber, &nmGameMutex);
	LockPauseGame (iGameClass, iGameNumber, &nmPauseMutex);

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

		// Check for updates
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

			Assert (iNumUpdates - vNumPrevUpdates.GetInteger() > 0);
			
			// Get update period
			iErrCode = m_pGameData->ReadData (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::NumSecPerUpdate,
				&vTemp
				);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iPeriod = (Seconds) vTemp.GetInteger();
			
			// Get game options
			iErrCode = m_pGameData->ReadData (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::Options,
				&vOptions
				);
			
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			// Get after weekend delay
			if (!(vOptions.GetInteger() & WEEKEND_UPDATES)) {
				
				iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AfterWeekendDelay, &vAfterWeekendDelay);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
			}
			
			// Get first update delay
			if (vNumPrevUpdates.GetInteger() == 0) {
				
				iErrCode = m_pGameData->ReadData (strGameData, GameData::FirstUpdateDelay, &vFirstUpdateDelay);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
			}
			
			// Yes, we're going to update
			*pbUpdate = true;
			
			iErrCode = WaitForUpdate (iGameClass, iGameNumber);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			bGameWriter = true;
			
			// Get last update time
			iErrCode = m_pGameData->ReadData (strGameData, GameData::LastUpdateTime, &vTemp);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			tUpdateTime = vTemp.GetUTCTime();
			
			for (i = 0; i < iNumUpdates - vNumPrevUpdates.GetInteger() && !bGameOver; i ++) {
				
				// Set new update time
				if (vOptions.GetInteger() & WEEKEND_UPDATES) {
					
					iAdjustment = 0;
					
				} else {
					
					Time::AddSeconds (tUpdateTime, iPeriod, &tTime);	

					if (Time::IsWeekendTime (tTime)) {
					
						iAdjustment = Time::GetRemainingWeekendSeconds (tTime) + vAfterWeekendDelay.GetInteger();
						Time::AddSeconds (tUpdateTime, iAdjustment, &tUpdateTime);
					
					} else {
						iAdjustment = 0;
					}
				}

				Time::AddSeconds (tUpdateTime, iPeriod, &tUpdateTime);
				
				// Add update delay to first update
				if (i == 0 && 
					vNumPrevUpdates.GetInteger() == 0 &&
					vFirstUpdateDelay.GetInteger() > iAdjustment) {
					Time::AddSeconds (tUpdateTime, vFirstUpdateDelay.GetInteger() - iAdjustment, &tUpdateTime);
				}

				// This may be slightly overactive
				Assert (tUpdateTime <= tNow);
				
				// Execute an update
				iErrCode = RunUpdate (iGameClass, iGameNumber, tUpdateTime, &bGameOver);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
			}
		}

		// Refresh game state if necessary
		if (*pbUpdate && !bGameOver) {
			
			iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
	}

	// Games can update even if they're paused, as long as all empires have ended turn
	if (!bGameOver && !(vState.GetInteger() & STILL_OPEN)) {

		// Check for all empires hitting end turn when game has already closed
		Variant vNumUpdated;
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
			
			// If all are ready for an update, go for it!
			if (vNumUpdated.GetInteger() < (int) iNumNeeded) {
				
				// Exit the loop
				break;
				
			} else {

				Assert (vNumUpdated.GetInteger() == (int) iNumNeeded);
				
				if (!bGameWriter) {
					
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
	}
	
Cleanup:

	if (iErrCode == OK && !bGameOver) {
		
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

	// Release game locks
	UnlockPauseGame (nmPauseMutex);
	UnlockGame (nmGameMutex);

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
								   int* piNumUpdates, int* piGameState) {

	int iErrCode = OK;

	Variant vSecPerUpdate, vLastUpdateTime, vTemp, vNumUpdates, vOptions, vAfterWeekendDelay, vState,
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

	*piGameState = vState.GetInteger();

	// Easy short-circuit: game hasn't started
	if (!(vState.GetInteger() & STARTED)) {

		// Get first update delay
		iErrCode = m_pGameData->ReadData (strGameData, GameData::FirstUpdateDelay, &vFirstUpdateDelay);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		*piNumUpdates = 0;
		*piSecondsSince = 0;
		*piSecondsUntil = vSecPerUpdate.GetInteger() + vFirstUpdateDelay.GetInteger();

		return OK;
	}

	// Get last update time
	iErrCode = m_pGameData->ReadData (strGameData, GameData::LastUpdateTime, &vLastUpdateTime);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}
	
	// Get num updates
	iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	*piNumUpdates = vNumUpdates.GetInteger();
	
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

	// Get after weekend delay
	iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AfterWeekendDelay, &vAfterWeekendDelay);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	// Get time
	UTCTime tNow;
	Time::GetTime (&tNow);

	int iAdjustment = 0;

	if ((vState.GetInteger() & ADMIN_PAUSED) || (vState.GetInteger() & PAUSED)) {

		iErrCode = m_pGameData->ReadData (
			strGameData, 
			GameData::SecondsRemainingInUpdateWhilePaused, 
			&vTemp
			);

		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		Assert (vTemp.GetInteger() >= 0);
		
		// Adjust for no weekend updates				
		if (!(vOptions.GetInteger() & WEEKEND_UPDATES)) {
			
			UTCTime tTime;

			Time::AddSeconds (tNow, vTemp.GetInteger(), &tTime);
			
			if (Time::IsWeekendTime (tTime)) {
				iAdjustment = (int) Time::GetRemainingWeekendSeconds (tTime) + vAfterWeekendDelay.GetInteger();
				vTemp += iAdjustment;
			}
		}

		// No need to adjust for first update delay - already implicit in SecondsRemainingInUpdateWhilePaused			
		*piSecondsUntil = vTemp.GetInteger();
		*piSecondsSince = Time::GetSecondDifference (tNow, vLastUpdateTime.GetUTCTime());

	} else {

		Seconds sAdjustment = 0;
		UTCTime tNextUpdateTime, tLastUpdateTime;

		// Calculate next update time
		tLastUpdateTime = vLastUpdateTime.GetUTCTime();
		Time::AddSeconds (vLastUpdateTime.GetUTCTime(), vSecPerUpdate.GetInteger(), &tNextUpdateTime);

		// Adjust for first update delay
		if (vNumUpdates.GetInteger() == 0) {

			Seconds sExtraTime = vFirstUpdateDelay.GetInteger();
			if (sExtraTime > 0) {
				Time::AddSeconds (tNextUpdateTime, sExtraTime, &tNextUpdateTime);
			}
		}

		// If no weekend updates and current time is in weekend, push it back 'til Monday
		if (!(vOptions.GetInteger() & WEEKEND_UPDATES) && Time::IsWeekendTime (tNextUpdateTime)) {

			sAdjustment = Time::GetRemainingWeekendSeconds (tNextUpdateTime) + vAfterWeekendDelay.GetInteger();

			Time::AddSeconds (
				tNextUpdateTime,
				sAdjustment,
				&tNextUpdateTime
				);
		}

		// Update until we're set
		while (tNextUpdateTime <= tNow) {

			// An update
			(*piNumUpdates) ++;
			tLastUpdateTime = tNextUpdateTime;

			// Increment time
			Time::AddSeconds (tLastUpdateTime, vSecPerUpdate.GetInteger(), &tNextUpdateTime);
			
			// If no weekend updates and time is on weekend, push the time back until Monday
			if (!(vOptions.GetInteger() & WEEKEND_UPDATES) && Time::IsWeekendTime (tNextUpdateTime)) {

				Time::AddSeconds (
					tNextUpdateTime, 
					Time::GetRemainingWeekendSeconds (tNextUpdateTime) + vAfterWeekendDelay.GetInteger(),
					&tNextUpdateTime
					);
			}
		}
		
		// At this point, tNextUpdateTime will be the next unfulfilled update
		*piSecondsSince = Time::GetSecondDifference (tNow, tLastUpdateTime);
		*piSecondsUntil = Time::GetSecondDifference (tNextUpdateTime, tNow);
	}

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
	Variant vUp;

	GAME_EMPIRE_DATA (strTableName, iGameClass, iGameNumber, iEmpireKey);
	
	NamedMutex nmMutex;
	LockEmpireUpdated (iGameClass, iGameNumber, iEmpireKey, &nmMutex);
	
	iErrCode = m_pGameData->ReadData (strTableName, GameEmpireData::Options, &vUp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	*pbSet = (vUp.GetInteger() & UPDATED) == 0;
	if (*pbSet) {

		GAME_DATA (strGameData, iGameClass, iGameNumber);
		
		iErrCode = m_pGameData->WriteOr (strTableName, GameEmpireData::Options, UPDATED);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		// Increment empire updated count
		iErrCode = m_pGameData->Increment (
			strGameData, 
			GameData::NumEmpiresUpdated, 
			1
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	}

Cleanup:

	UnlockEmpireUpdated (nmMutex);

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Sets an empire to not be ready for the next update.

int GameEngine::SetEmpireNotReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet) {

	int iErrCode = OK;
	Variant vTemp;
	
	GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
	GAME_DATA (strGameData, iGameClass, iGameNumber);
	
	NamedMutex nmMutex;
	LockEmpireUpdated (iGameClass, iGameNumber, iEmpireKey, &nmMutex);
	
	iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::Options, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	*pbSet = (vTemp.GetInteger() & UPDATED) != 0;
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
	}
	
Cleanup:

	UnlockEmpireUpdated (nmMutex);

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

int GameEngine::CheckAllGamesForUpdates() {

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
			CheckGameForUpdates (iGameClass, iGameNumber, &bUpdate);
		}
		
		// Clean up
		m_pGameData->FreeData (pvGame);
	}

	return OK;
}