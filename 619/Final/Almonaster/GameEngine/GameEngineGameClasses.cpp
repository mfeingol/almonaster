//
// Almonaster.dll:  a component of Almonaster 2.0
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
// iGameClass -> Integer key of a gameclass
//
// Output:
// *pstrName -> Gameclass name
//
// Return the name of a gameclass

int GameEngine::GetGameClassName (int iGameClass, char pszName [MAX_FULL_GAME_CLASS_NAME_LENGTH]) {

	IReadTable* pGameClasses = NULL;

	int iOwner, iErrCode;
	const char* pszGameClassName = NULL;

	iErrCode = m_pGameData->GetTableForReading (SYSTEM_GAMECLASS_DATA, &pGameClasses);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

#ifdef _DEBUG

	// Avoid assertion
	bool bExists;
	iErrCode = pGameClasses->DoesRowExist (iGameClass, &bExists);if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (!bExists) {
		iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
		goto Cleanup;
	}

#endif

	iErrCode = pGameClasses->ReadData (iGameClass, SystemGameClassData::Name, &pszGameClassName);
	if (iErrCode != OK) {
		Assert (false);
		if (iErrCode == ERROR_UNKNOWN_ROW_KEY) {
			iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
		}
		goto Cleanup;
	}

	iErrCode = pGameClasses->ReadData (iGameClass, SystemGameClassData::Owner, &iOwner);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (iOwner == SYSTEM) {

		strcpy (pszName, pszGameClassName);
	
	} else {

		const char* pszOwnerName = NULL;

		iErrCode = pGameClasses->ReadData (iGameClass, SystemGameClassData::OwnerName, &pszOwnerName);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		sprintf (pszName, "%s (%s)", pszGameClassName, pszOwnerName);
	}

Cleanup:

	if (pGameClasses != NULL) {
		pGameClasses->Release();
	}


	return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piNumSeconds -> Seconds in update period
//
// Get the game class' update period

int GameEngine::GetGameClassUpdatePeriod (int iGameClass, Seconds* piNumSeconds) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::NumSecPerUpdate, 
		&vTemp
		);

	if (iErrCode == OK) {
		*piNumSeconds = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piGameNumber -> Next game number
//
// Get the game class' next game number (no promises)

int GameEngine::GetNextGameNumber (int iGameClass, int* piGameNumber) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::OpenGameNum, 
		&vTemp
		);

	if (iErrCode == OK) {
		*piGameNumber = vTemp.GetInteger();
	}

	return iErrCode;

}


// Input:
// iGameClass -> Gameclass key
//
// Output:
// *pbDeleted -> true if deleted, false if just marked for deletion
//
// Halt the given gameclass and delete it if no active games are left

int GameEngine::DeleteGameClass (int iGameClass, bool* pbDeleted) {

	// Lock gameclass
	NamedMutex nmMutex;
	LockGameClass (iGameClass, &nmMutex);

	// Check for gameclass
	bool bExists;
	int iErrCode = m_pGameData->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
	if (iErrCode != OK || !bExists) {
		UnlockGameClass (nmMutex);
		return ERROR_GAMECLASS_DOES_NOT_EXIST;
	}

	// Check for active games
	*pbDeleted = !DoesGameClassHaveActiveGames (iGameClass);

	if (*pbDeleted) {

		// Get owner
		Variant vOwner;
		iErrCode = m_pGameData->ReadData (
			SYSTEM_GAMECLASS_DATA, 
			iGameClass, 
			SystemGameClassData::Owner, 
			&vOwner
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (vOwner.GetInteger() == SYSTEM) {

			unsigned int iKey;
			
			// Decrement super class counter and delete
			Variant vSuperClassKey;
			iErrCode = m_pGameData->ReadData (
				SYSTEM_GAMECLASS_DATA, 
				iGameClass, 
				SystemGameClassData::SuperClassKey, 
				&vSuperClassKey
				);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			iErrCode = m_pGameData->GetFirstKey (
				SYSTEM_SYSTEM_GAMECLASS_DATA, 
				SystemSystemGameClassData::GameClass, 
				iGameClass, 
				false, 
				&iKey
				);

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			iErrCode = m_pGameData->Increment (
				SYSTEM_SUPERCLASS_DATA, 
				vSuperClassKey.GetInteger(), 
				SystemSuperClassData::NumGameClasses, 
				-1
				);

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->DeleteRow (SYSTEM_SYSTEM_GAMECLASS_DATA, iKey);
			if (iErrCode != OK) {

				Assert (false);

				// Best effort compensate
				int iErrCode2 = m_pGameData->Increment (
					SYSTEM_SUPERCLASS_DATA, 
					vSuperClassKey.GetInteger(), 
					SystemSuperClassData::NumGameClasses, 
					1
					);

				Assert (iErrCode2 == OK);

				goto Cleanup;
			}

		}

		// Delete row from SystemGameClassData
		iErrCode = m_pGameData->DeleteRow (SYSTEM_GAMECLASS_DATA, iGameClass);
		Assert (iErrCode == OK);

	} else {
		
		// Mark the gameclass for deletion
		iErrCode = m_pGameData->WriteOr (
			SYSTEM_GAMECLASS_DATA, 
			iGameClass, 
			SystemGameClassData::Options, 
			GAMECLASS_MARKED_FOR_DELETION
			);
		Assert (iErrCode == OK);
	}

Cleanup:

	// Release lock
	UnlockGameClass (nmMutex);

	return iErrCode;
}

bool GameEngine::DoesGameClassHaveActiveGames (int iGameClass) {

	Variant vNumActiveGames;

	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA,
		iGameClass,
		SystemGameClassData::NumActiveGames,
		&vNumActiveGames
		);

	if (iErrCode != OK || vNumActiveGames.GetInteger() > 0) {
		return true;
	}

	return false;
}


// Input:
// iGameClass -> Gameclass key
//
// Halt the given gameclass

int GameEngine::HaltGameClass (int iGameClass) {

	// Lock gameclass
	NamedMutex nmMutex;
	LockGameClass (iGameClass, &nmMutex);

	// Check for gameclass
	bool bExists;
	int iErrCode = m_pGameData->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
	if (iErrCode != OK || !bExists) {
		UnlockGameClass (nmMutex);
		return ERROR_GAMECLASS_DOES_NOT_EXIST;
	}

	// Mark the gameclass as halted
	iErrCode = m_pGameData->WriteOr (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		GAMECLASS_HALTED
		);
	Assert (iErrCode == OK);

	// Unlock
	UnlockGameClass (nmMutex);

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass key
//
// Unhalt the given gameclass

int GameEngine::UnhaltGameClass (int iGameClass) {

	Variant vHalted;

	// Lock gameclass
	NamedMutex nmMutex;
	LockGameClass (iGameClass, &nmMutex);

	// Check for gameclass
	bool bExists;
	int iErrCode = m_pGameData->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
	if (iErrCode != OK || !bExists) {
		iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
		goto Cleanup;
	}

	// Is gameclass halted?
	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		&vHalted
		);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}


	if (!(vHalted.GetInteger() & GAMECLASS_HALTED)) {
		iErrCode = ERROR_GAMECLASS_NOT_HALTED;
		goto Cleanup;
	}

	// Mark the gameclass as no longer halted
	iErrCode = m_pGameData->WriteAnd (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		~GAMECLASS_HALTED
		);
	Assert (iErrCode == OK);

Cleanup:

	// Unlock
	UnlockGameClass (nmMutex);

	return iErrCode;
}




// Input:
// *pvGameClassData -> Row if SystemGameClassData
//
// Output:
// *piGameClass -> New gameclass key
//
// Do the following:
//	Add a row to SystemGameClassData
//	Create SystemGameInitialTechs(I) Table
//	Create SystemGameDevelopableTechs(I) Table
//	Add rows to each
//	Increment NumGameClass counter in SystemData

int GameEngine::CreateGameClass (Variant* pvGameClassData, int* piGameClass) {

	bool bFlag, bDynamic = (pvGameClassData[SystemGameClassData::Options].GetInteger() & DYNAMIC_GAMECLASS) != 0;
	int iErrCode, iOwner = pvGameClassData[SystemGameClassData::Owner].GetInteger(), iSuperClass = NO_KEY;

	// Make sure empire has privilege and has less than the max num of gameclasses
	if (iOwner != SYSTEM) {

		// Simple access check
		Variant vTemp;
		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iOwner, SystemEmpireData::Privilege, &vTemp);
		if (iErrCode != OK) {
			return iErrCode;
		}

		if (bDynamic) {

			if (vTemp.GetInteger() < APPRENTICE) {
				return ERROR_INSUFFICIENT_PRIVILEGE;
			}

			pvGameClassData[SystemGameClassData::Options] = 
				pvGameClassData[SystemGameClassData::Options].GetInteger() | GAMECLASS_MARKED_FOR_DELETION;

		} else {

			if (vTemp.GetInteger() < ADEPT) {
				return ERROR_INSUFFICIENT_PRIVILEGE;
			}
			
			// Number of gameclasses check
			unsigned int iHas;
			iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxNumPersonalGameClasses, &vTemp);
			if (iErrCode != OK) {
				return iErrCode;
			}

			iErrCode = m_pGameData->GetEqualKeys (
				SYSTEM_GAMECLASS_DATA,
				SystemGameClassData::Owner,
				iOwner,
				false,
				NULL,
				&iHas
				);

			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				iHas = 0;
			}

			else if (iErrCode != OK) {
				return iErrCode;
			}

			if ((int) iHas >= vTemp.GetInteger()) {
				return ERROR_TOO_MANY_GAMECLASSES;
			}
		}

	} else {
		
		iSuperClass = pvGameClassData[SystemGameClassData::SuperClassKey].GetInteger();
		if (iSuperClass == NO_KEY) {
			return ERROR_GAMECLASS_HAS_NO_SUPERCLASS;
		}
	}

	// Check name length
	if (String::StrLen (pvGameClassData[SystemGameClassData::Name].GetCharPtr()) > MAX_GAME_CLASS_NAME_LENGTH) {
		return ERROR_NAME_IS_TOO_LONG;
	}

	// Check description length
	if (String::StrLen (pvGameClassData[SystemGameClassData::Description].GetCharPtr()) > MAX_THEME_DESCRIPTION_LENGTH) {
		return ERROR_DESCRIPTION_IS_TOO_LONG;
	}

	// Set num active games to zero
	pvGameClassData[SystemGameClassData::NumActiveGames] = 0;

	// Lock
	LockGameClasses();
	LockSuperClasses();

	// Make sure name isn't a duplicate
	unsigned int iGameClass;
	iErrCode = m_pGameData->GetFirstKey (
		SYSTEM_GAMECLASS_DATA, 
		SystemGameClassData::Name, 
		pvGameClassData[SystemGameClassData::Name].GetCharPtr(), 
		true, 
		&iGameClass
		);

	if (iGameClass != NO_KEY) {
		iErrCode = ERROR_GAMECLASS_ALREADY_EXISTS;
		goto Cleanup;
	}

	// Check super class
	if (iSuperClass != NO_KEY) {

		iErrCode = m_pGameData->DoesRowExist (SYSTEM_SUPERCLASS_DATA, iSuperClass, &bFlag);
		if (iErrCode != OK || !bFlag) {
			iErrCode = ERROR_GAMECLASS_HAS_NO_SUPERCLASS;
			goto Cleanup;
		}

		// Increment NumGameClasses in superclass, if applicable
		iErrCode = m_pGameData->Increment (
			SYSTEM_SUPERCLASS_DATA, 
			iSuperClass,
			SystemSuperClassData::NumGameClasses,
			1
			);

		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	// Reserved fields
	pvGameClassData[SystemGameClassData::iRESERVED0] = 0;
	pvGameClassData[SystemGameClassData::iRESERVED1] = 0;
	pvGameClassData[SystemGameClassData::fRESERVED2] = (float) 0.0;
	pvGameClassData[SystemGameClassData::fRESERVED3] = (float) 0.0;

	// Insert new row into SystemGameClassData and obtain key to that row
	iErrCode = m_pGameData->InsertRow (SYSTEM_GAMECLASS_DATA, pvGameClassData, &iGameClass);
	if (iErrCode != OK) {
		
		if (iSuperClass != NO_KEY) {
			
			// Best effort compensate
			int iErrCode2 = m_pGameData->Increment (
				SYSTEM_SUPERCLASS_DATA, 
				iSuperClass,
				SystemSuperClassData::NumGameClasses,
				-1
				);
			Assert (iErrCode2 == OK);
		}
		goto Cleanup;
	}

	if (iOwner == SYSTEM) {

		Variant vKey = iGameClass;

		// Add row to SystemSystemGameClassData
		iErrCode = m_pGameData->InsertRow (SYSTEM_SYSTEM_GAMECLASS_DATA, &vKey);

		if (iErrCode != OK) {

			// Best effort compensate
			int iErrCode2 = DeleteGameClass (iGameClass, &bFlag);
			Assert (iErrCode2 == OK);

			goto Cleanup;
		}

	}

	*piGameClass = iGameClass;

Cleanup:

	UnlockSuperClasses();
	UnlockGameClasses();

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass key
//
// returns OK if undeleted, WARNING if doesn't exist
//
// Unhalt the given gameclass

int GameEngine::UndeleteGameClass (int iGameClass) {

	int iErrCode;
	Variant vOptions;

	NamedMutex nmMutex;
	LockGameClass (iGameClass, &nmMutex);

	// Test if gameclass exists
	bool bExist;
	iErrCode = m_pGameData->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExist);
	if (iErrCode != OK || !bExist) {
		iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
		goto Cleanup;
	}

	// Is gameclass really marked for deletion?
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

	if (!(vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION)) {
		iErrCode = ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION;
		goto Cleanup;
	}

	// Mark the gameclass as not halted
	iErrCode = m_pGameData->WriteAnd (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		~GAMECLASS_MARKED_FOR_DELETION
		);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

Cleanup:

	UnlockGameClass (nmMutex);

	return iErrCode;
}

// Input:
// lNumKeys -> Number of gameclasses
//
// Output:
// **ppiKey -> Keys corresponding to the different gameclasses
// **ppbHalted -> true if game has been halted, false otherwise
// *piNumKeys -> Number of keys returned
//
// Return the integer keys corresponding to each system gameclass, including those which have been halted

int GameEngine::GetSystemGameClassKeys (int** ppiKey, bool** ppbHalted, bool** ppbDeleted, int* piNumKeys) {

	Variant* pvData;

	*ppiKey = NULL;
	*ppbHalted = NULL;
	*ppbDeleted = NULL;
	*piNumKeys = 0;

	int i, iErrCode = m_pGameData->ReadColumn (
		SYSTEM_SYSTEM_GAMECLASS_DATA, 
		SystemSystemGameClassData::GameClass, 
		NULL, 
		&pvData, 
		(unsigned int*) piNumKeys
		);
	
	if (iErrCode == ERROR_DATA_NOT_FOUND) {
		return OK;
	}

	if (*piNumKeys > 0) {

		IReadTable* pSystemGameClassData;
		int iNumClasses = 0;

		iErrCode = m_pGameData->GetTableForReading (SYSTEM_GAMECLASS_DATA, &pSystemGameClassData);
		if (iErrCode != OK) {
			return iErrCode;
		}

		*ppbHalted = new bool [*piNumKeys];
		*ppbDeleted = new bool [*piNumKeys];
		*ppiKey = new int [*piNumKeys];

		Variant vOptions;
		for (i = 0; i < *piNumKeys; i ++) {

			iErrCode = pSystemGameClassData->ReadData (
				pvData[i].GetInteger(), 
				SystemGameClassData::Options, 
				&vOptions
				);

			if (iErrCode == OK) {
			
				(*ppbHalted)[iNumClasses] = (vOptions.GetInteger() & GAMECLASS_HALTED) != 0;
				(*ppbDeleted)[iNumClasses] = (vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION) != 0;
				(*ppiKey)[iNumClasses] = pvData[i].GetInteger();

				iNumClasses ++;
			}
		}

		pSystemGameClassData->Release();
		*piNumKeys = iNumClasses;

		if (iNumClasses == 0) {

			delete [] *ppbHalted;
			*ppbHalted = NULL;

			delete [] *ppbDeleted;
			*ppbDeleted = NULL;

			delete [] *ppiKey;
			*ppiKey = NULL;
		}
		
		m_pGameData->FreeData (pvData);
	}

	return iErrCode;
}


// Output:
// **ppiKey -> Keys corresponding to the different gameclasses
// *piNumKeys -> Number of keys returned
//
// Returns the integer keys corresponding to each unhalted admin gameclass

int GameEngine::GetStartableSystemGameClassKeys (int** ppiKey, int* piNumKeys) {

	unsigned int i, iNumGameClasses;

	IReadTable* pSystemSystemGameClassData, * pSystemGameClassData;

	*ppiKey = NULL;
	*piNumKeys = 0;

	int iErrCode = m_pGameData->GetTableForReading (SYSTEM_SYSTEM_GAMECLASS_DATA, &pSystemSystemGameClassData);
	if (iErrCode != OK) {
		return iErrCode;
	}

	iErrCode = m_pGameData->GetTableForReading (SYSTEM_GAMECLASS_DATA, &pSystemGameClassData);
	if (iErrCode != OK) {
		pSystemSystemGameClassData->Release();
		return iErrCode;
	}

	int* piGameClassKey;

	iErrCode = pSystemSystemGameClassData->ReadColumn (
		SystemSystemGameClassData::GameClass,
		&piGameClassKey,
		&iNumGameClasses
		);

	if (iErrCode == ERROR_DATA_NOT_FOUND) {
		iErrCode = OK;
	} else {

		if (iErrCode != OK) {
			goto Cleanup;
		}

		*ppiKey = new int [iNumGameClasses];
		*piNumKeys = 0;

		// Filter halted gameclasses
		int iOptions;
		for (i = 0; i < iNumGameClasses; i ++) {

			iErrCode = pSystemGameClassData->ReadData (
				piGameClassKey[i],
				SystemGameClassData::Options, 
				&iOptions
				);
			
			if (iErrCode == OK &&
				!(iOptions & GAMECLASS_HALTED) &&
				!(iOptions & GAMECLASS_MARKED_FOR_DELETION)) {
				(*ppiKey)[(*piNumKeys) ++] = piGameClassKey[i];
			}
		}
		
		m_pGameData->FreeData (piGameClassKey);

		if (*piNumKeys == 0) {
			delete [] (*ppiKey);
			*ppiKey = NULL;
		}
	}

Cleanup:

	pSystemSystemGameClassData->Release();
	pSystemGameClassData->Release();

	return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piSuperClassKey -> Integer key of a superclass
//
// Return the key of the superclass that the gameclass belongs to

int GameEngine::GetGameClassSuperClassKey (int iGameClass, int* piSuperClassKey) {

	int iErrCode = OK;

	Variant vSuperClassKey;
	iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::SuperClassKey, &vSuperClassKey);
	
	if (iErrCode == OK) {
		*piSuperClassKey = vSuperClassKey.GetInteger();
	}

	return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
// iSuperClassKey -> Integer key of a superclass
//
// Set the key of the superclass that the gameclass belongs to

int GameEngine::SetGameClassSuperClassKey (int iGameClass, int iSuperClassKey) {

	Variant vTemp;
	int iErrCode;

	LockSuperClasses();
	
	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::SuperClassKey, 
		&vTemp
		);

	if (iErrCode != OK) {
		iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
		goto Cleanup;
	}

	iErrCode = m_pGameData->Increment (
		SYSTEM_SUPERCLASS_DATA, 
		vTemp.GetInteger(), 
		SystemSuperClassData::NumGameClasses, 
		-1
		);
	if (iErrCode != OK) {
		iErrCode = ERROR_GAMECLASS_HAS_NO_SUPERCLASS;
		goto Cleanup;
	}

	iErrCode = m_pGameData->Increment (
		SYSTEM_SUPERCLASS_DATA, 
		iSuperClassKey,
		SystemSuperClassData::NumGameClasses, 
		1
		);
	if (iErrCode != OK) {
		iErrCode = ERROR_SUPERCLASS_DOES_NOT_EXIST;

		int iErrCode2 = m_pGameData->Increment (
			SYSTEM_SUPERCLASS_DATA, 
			vTemp.GetInteger(), 
			SystemSuperClassData::NumGameClasses, 
			1
			);
		Assert (iErrCode2 == OK);

		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::SuperClassKey, 
		iSuperClassKey
		);
	Assert (iErrCode == OK);

Cleanup:

	UnlockSuperClasses();

	return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *pvInfo -> Gameclass data
//
// Returns data corresponding to a given gameclass.

int GameEngine::GetGameClassData (int iGameClass, Variant** ppvData) {

	return m_pGameData->ReadRow (SYSTEM_GAMECLASS_DATA, iGameClass, ppvData);
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piOwner -> Owner of the gameclass
//
// Return the key of the owner of the given gameclass

int GameEngine::GetGameClassOwner (int iGameClass, int* piOwner) {
	
	int iErrCode;
	bool bExists;

	iErrCode = m_pGameData->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
	if (iErrCode != OK || !bExists) {
		return ERROR_GAMECLASS_DOES_NOT_EXIST;
	}

	Variant vOwner;
	iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Owner, &vOwner);
	
	if (iErrCode == OK) {
		*piOwner = vOwner.GetInteger();
	}

	return iErrCode;
}

// Input:
// iGameClass -> Integer key of gameclass
//
// Output:
// *piInitialTechs -> Bitmap of initial techs
// *piDevelopableTechs -> Bitmap of developable techs
//
// Return the bitmaps of initial developable techs

int GameEngine::GetDevelopableTechs (int iGameClass, int* piInitialTechs, int* piDevelopableTechs) {

	int iErrCode;
	Variant vTech;

	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::InitialTechDevs,
		&vTech
		);
	if (iErrCode != OK) {
		return iErrCode;
	}

	*piInitialTechs = vTech.GetInteger();

	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::DevelopableTechDevs,
		&vTech
		);
	if (iErrCode != OK) {
		return iErrCode;
	}

	*piDevelopableTechs = vTech.GetInteger();

	return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
//
// Output:
// *piNumEmpiresRequired -> Number of empires needed for game to start
//
// Return the number of empires needed for game to start

int GameEngine::GetNumEmpiresRequiredForGameToStart (int iGameClass, int* piNumEmpiresRequired) {

	Variant vNumEmpires;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::MinNumEmpires, 
		&vNumEmpires
		);

	if (iErrCode == OK) {
		*piNumEmpiresRequired = vNumEmpires.GetInteger();
	}

	return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
//
// Output:
// *pfMaxTechIncrease -> Max gameclass tech increase
//
// Return max tech increase of the gameclass

int GameEngine::GetGameClassMaxTechIncrease (int iGameClass, float* pfMaxTechIncrease) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxTechDev, &vTemp);

	if (iErrCode == OK) {
		*pfMaxTechIncrease = vTemp.GetFloat();
	}

	return iErrCode;
}


int GameEngine::GetGameClassVisibleBuilds (int iGameClass, bool* pbVisible) {

	Variant vOptions;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options,
		&vOptions
		);

	if (iErrCode == OK) {
		*pbVisible = (vOptions.GetInteger() & VISIBLE_BUILDS) != 0;
	}

	return iErrCode;
}

int GameEngine::GetGameClassVisibleDiplomacy (int iGameClass, bool* pbVisible) {

	Variant vOptions;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA,
		iGameClass,
		SystemGameClassData::Options,
		&vOptions
		);

	if (iErrCode == OK) {
		*pbVisible = (vOptions.GetInteger() & VISIBLE_DIPLOMACY) != 0;
	}

	return iErrCode;
}

int GameEngine::GetMinNumSecsPerUpdateForSystemGameClass (int* piMinNumSecsPerUpdate) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::SystemMinSecs, &vTemp);

	if (iErrCode == OK) {
		*piMinNumSecsPerUpdate = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetMaxNumSecsPerUpdateForSystemGameClass (int* piMaxNumSecsPerUpdate) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::SystemMaxSecs, &vTemp);

	if (iErrCode == OK) {
		*piMaxNumSecsPerUpdate = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetMaxNumEmpiresForSystemGameClass (int* piMaxNumEmpires) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::SystemMaxNumEmpires, &vTemp);

	if (iErrCode == OK) {
		*piMaxNumEmpires = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetMaxNumPlanetsForSystemGameClass (int* piMaxNumPlanets) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::SystemMaxNumPlanets, &vTemp);
	
	if (iErrCode == OK) {
		*piMaxNumPlanets = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetMinNumSecsPerUpdateForPersonalGameClass (int* piMinNumSecsPerUpdate) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::PersonalMinSecs, &vTemp);
	
	if (iErrCode == OK) {
		*piMinNumSecsPerUpdate = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetMaxNumSecsPerUpdateForPersonalGameClass (int* piMaxNumSecsPerUpdate) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::PersonalMaxSecs, &vTemp);
	
	if (iErrCode == OK) {
		*piMaxNumSecsPerUpdate = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetMaxNumEmpiresForPersonalGameClass (int* piMaxNumEmpires) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::PersonalMaxNumEmpires, &vTemp);
	
	if (iErrCode == OK) {
		*piMaxNumEmpires = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetMaxNumPlanetsForPersonalGameClass (int* piMaxNumPlanets) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::PersonalMaxNumPlanets, &vTemp);
	
	if (iErrCode == OK) {
		*piMaxNumPlanets = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::SetMinNumSecsPerUpdateForSystemGameClass (int iMinNumSecsPerUpdate) {
	
	if (iMinNumSecsPerUpdate < 0) {
		return ERROR_INVALID_ARGUMENT;
	}

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::SystemMaxSecs, &vTemp);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (vTemp.GetInteger() < iMinNumSecsPerUpdate) {
		return ERROR_FAILURE;
	}

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::SystemMinSecs, iMinNumSecsPerUpdate);
}

int GameEngine::SetMaxNumSecsPerUpdateForSystemGameClass (int iMaxNumSecsPerUpdate) {
	
	if (iMaxNumSecsPerUpdate < 0) {
		return ERROR_INVALID_ARGUMENT;
	}

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::SystemMinSecs, &vTemp);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (vTemp.GetInteger() > iMaxNumSecsPerUpdate) {
		return ERROR_FAILURE;
	}

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::SystemMaxSecs, iMaxNumSecsPerUpdate);
}

int GameEngine::SetMaxNumEmpiresForSystemGameClass (int iMaxNumEmpires) {

	if (iMaxNumEmpires < 0) {
		return ERROR_INVALID_ARGUMENT;
	}
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::SystemMaxNumEmpires, iMaxNumEmpires);
}

int GameEngine::SetMaxNumPlanetsForSystemGameClass (int iMaxNumPlanets) {

	if (iMaxNumPlanets < 0) {
		return ERROR_INVALID_ARGUMENT;
	}
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::SystemMaxNumPlanets, iMaxNumPlanets);
}

int GameEngine::SetMinNumSecsPerUpdateForPersonalGameClass (int iMinNumSecsPerUpdate) {
	
	if (iMinNumSecsPerUpdate < 0) {
		return ERROR_INVALID_ARGUMENT;
	}

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::PersonalMaxSecs, &vTemp);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (vTemp.GetInteger() < iMinNumSecsPerUpdate) {
		return ERROR_FAILURE;
	}
	
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::PersonalMinSecs, iMinNumSecsPerUpdate);
}

int GameEngine::SetMaxNumSecsPerUpdateForPersonalGameClass (int iMaxNumSecsPerUpdate) {
	
	if (iMaxNumSecsPerUpdate < 0) {
		return ERROR_INVALID_ARGUMENT;
	}
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::PersonalMinSecs, &vTemp);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (vTemp.GetInteger() > iMaxNumSecsPerUpdate) {
		return ERROR_FAILURE;
	}

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::PersonalMaxSecs, iMaxNumSecsPerUpdate);
}

int GameEngine::SetMaxNumEmpiresForPersonalGameClass (int iMaxNumEmpires) {
	
	if (iMaxNumEmpires < 0) {
		return ERROR_INVALID_ARGUMENT;
	}
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::PersonalMaxNumEmpires, iMaxNumEmpires);
}

int GameEngine::SetMaxNumPlanetsForPersonalGameClass (int iMaxNumPlanets) {
	
	if (iMaxNumPlanets < 0) {
		return ERROR_INVALID_ARGUMENT;
	}
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::PersonalMaxNumPlanets, iMaxNumPlanets);
}

// Input:
// iGameClass -> Gameclass
//
// Output:
// *pbPrivateMessages -> true if yes, false if no
//
// Return whether private messages are allowed in this gameclass

int GameEngine::DoesGameClassAllowPrivateMessages (int iGameClass, bool* pbPrivateMessages) {

	Variant vOptions;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		&vOptions
		);
	
	if (iErrCode == OK) {
		*pbPrivateMessages = (vOptions.GetInteger() & PRIVATE_MESSAGES) != 0;
	}

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass
//
// Output:
// *pbSubjective -> true if yes, false if no
//
// Return whether subjective views are enabled in this gameclass

int GameEngine::DoesGameClassHaveSubjectiveViews (int iGameClass, bool* pbSubjective) {

	Variant vOptions;

	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		&vOptions
		);

	if (iErrCode == OK) {
		*pbSubjective = (vOptions.GetInteger() & SUBJECTIVE_VIEWS) != 0;
	}

	return iErrCode;
}

int GameEngine::GetGameClassDiplomacyLevel (int iGameClass, int* piDiplomacy) {

	Variant vLevel;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::DiplomacyLevel, 
		&vLevel
		);
	
	if (iErrCode == OK) {
		*piDiplomacy = vLevel.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetGameClassOptions (int iGameClass, int* piOptions) {

	Variant vOptions;

	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		&vOptions
		);
	
	if (iErrCode == OK) {
		*piOptions = vOptions.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetMaxNumEmpires (int iGameClass, int* piMaxNumEmpires) {

	Variant vValue;

	int iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::MaxNumEmpires, 
		&vValue
		);
	
	if (iErrCode == OK) {
		*piMaxNumEmpires = vValue.GetInteger();
	}

	return iErrCode;
}