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

#include "Osal/Algorithm.h"

#include "GameEngine.h"

#include <stdio.h>

//
// Generic work
//

int GameEngine::GetEmpireOption (int iEmpireKey, unsigned int iFlag, bool* pbOption) {

	Variant vOptions;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vOptions);

	if (iErrCode == OK) {
		*pbOption = (vOptions.GetInteger() & iFlag) != 0;
	}

	return iErrCode;
}

int GameEngine::SetEmpireOption (int iEmpireKey, unsigned int iFlag, bool bOption) {

	if (bOption) {
		return m_pGameData->WriteOr (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, iFlag);
	} else {
		return m_pGameData->WriteAnd (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, ~iFlag);
	}
}

// Input:
// pszEmpireName -> Empire name
// pszPassword -> Empire password
// iPrivilege -> Privilege level;  overridden if iParentKey != NO_KEY
// iParentKey -> Empire whose credentials are to be inherited
//
// Output:
// *piEmpireKey -> Key assigned to new empire (NO_KEY if none)
//
// Create a new empire

int GameEngine::CreateEmpire (const char* pszEmpireName, const char* pszPassword, int iPrivilege,
							  int iParentKey, int* piEmpireKey) {

	NamedMutex nmParentMutex;
	unsigned int iKey, i;

	bool bParentLocked = false;

	int iOptions = 0, iAlmonasterScoreSignificance = 0, iErrCode;
	float fScore;

	IReadTable* pTable = NULL;

	*piEmpireKey = NO_KEY;

	char pszTable [512];

	// Make sure empire creation is enabled
	Variant vTemp;

	iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::Options, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (!(vTemp.GetInteger() & NEW_EMPIRES_ENABLED)) {
		return ERROR_DISABLED;
	}

	// Make sure name is not reserved
	for (i = 0; i < NUM_RESERVED_EMPIRE_NAMES; i ++) {

		if (String::StriCmp (pszEmpireName, RESERVED_EMPIRE_NAMES[i]) == 0) {
			return ERROR_RESERVED_EMPIRE_NAME;
		}
	}

	// Get current time
	UTCTime tTime;
	Time::GetTime (&tTime);

	// Declare array for insertion
	Variant pvColVal [SystemEmpireData::NumColumns];

	LockEmpires();

	// Make sure that an empire of the same name doesn't exist
	iErrCode = m_pGameData->GetFirstKey (
		SYSTEM_EMPIRE_DATA, 
		SystemEmpireData::Name, 
		pszEmpireName, 
		true, 
		&iKey
		);

	if (iErrCode != ERROR_DATA_NOT_FOUND) {
		
		if (iErrCode == OK) {
			iErrCode = ERROR_EMPIRE_ALREADY_EXISTS;
			goto Cleanup;
		}
		
		Assert (false);
		goto Cleanup;
	}

	// Deal with empire inheritance
	if (iParentKey != NO_KEY) {

		bool bExist;

		bParentLocked = true;
		LockEmpire (iParentKey, &nmParentMutex);

		iErrCode = m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iParentKey, &bExist);
		if (iErrCode != OK || !bExist) {
			iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
			goto Cleanup;
		}

		unsigned int iNumRows;
		SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iParentKey)

		iErrCode = m_pGameData->GetNumRows (pszTable, &iNumRows);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (iNumRows > 0) {
			iErrCode = ERROR_COULD_NOT_DELETE_EMPIRE;
			goto Cleanup;
		}

		// Inherit privilege
		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iParentKey, SystemEmpireData::Privilege, &vTemp);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (vTemp.GetInteger() == ADMINISTRATOR) {
			iErrCode = ERROR_COULD_NOT_DELETE_ADMINISTRATOR;
			goto Cleanup;
		}
		iPrivilege = vTemp.GetInteger();

		// Inherit Almonaster score
		iErrCode = m_pGameData->ReadData (
			SYSTEM_EMPIRE_DATA, 
			iParentKey, 
			SystemEmpireData::AlmonasterScore, 
			&vTemp
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		fScore = vTemp.GetFloat();

		// Inherit Almonaster score significance
		iErrCode = m_pGameData->ReadData (
			SYSTEM_EMPIRE_DATA, 
			iParentKey, 
			SystemEmpireData::AlmonasterScoreSignificance, 
			&vTemp
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iAlmonasterScoreSignificance = vTemp.GetInteger();

		// Propagate can't broadcast flag
		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iParentKey, SystemEmpireData::Options, &vTemp);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (vTemp.GetInteger() & CAN_BROADCAST) {
			iOptions |= CAN_BROADCAST;
		}

	} else {

		fScore = ALMONASTER_INITIAL_SCORE;
		iOptions |= CAN_BROADCAST;
	}
	
	// All empires get confirm on enter by default
	iOptions |= CONFIRM_ON_ENTER_OR_QUIT_GAME;

	// All empires use both types of game page password hashing by default
	iOptions |= IP_ADDRESS_PASSWORD_HASHING | SESSION_ID_PASSWORD_HASHING;

	// All empires have ships in up close map views turned on by default but not in planet views...
	iOptions |= SHIPS_ON_MAP_SCREEN;

	// Highlight ships by default on map
	iOptions |= SHIP_MAP_HIGHLIGHTING;

	// Get SystemData table
	iErrCode = m_pGameData->GetTableForReading (SYSTEM_DATA, &pTable);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Insert row into SystemEmpireData
	pvColVal[SystemEmpireData::Name] = pszEmpireName;
	pvColVal[SystemEmpireData::Password] = pszPassword;

	if (pvColVal[SystemEmpireData::Name].GetCharPtr() == NULL ||
		pvColVal[SystemEmpireData::Password].GetCharPtr() == NULL) {

		iErrCode = ERROR_OUT_OF_MEMORY;
		goto Cleanup;
	}

	pvColVal[SystemEmpireData::Privilege] = iPrivilege;
	pvColVal[SystemEmpireData::RealName] = "";
	pvColVal[SystemEmpireData::Email] = "";
	pvColVal[SystemEmpireData::WebPage] = "";
	pvColVal[SystemEmpireData::Quote] = "";

	iErrCode = pTable->ReadData (SystemData::DefaultAlien, &(pvColVal[SystemEmpireData::AlienKey]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIIndependentPlanet, &(pvColVal[SystemEmpireData::UIIndependentPlanet]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	pvColVal[SystemEmpireData::Wins] = 0;
	pvColVal[SystemEmpireData::Nukes] = 0;
	pvColVal[SystemEmpireData::Nuked] = 0;
	pvColVal[SystemEmpireData::LastLoginTime] = tTime;
	pvColVal[SystemEmpireData::Draws] = 0;
	pvColVal[SystemEmpireData::MaxEcon] = 0;
	pvColVal[SystemEmpireData::MaxMil] = 0;
	pvColVal[SystemEmpireData::IPAddress] = "";
	pvColVal[SystemEmpireData::Ruins] = 0;

	iErrCode = pTable->ReadData (
		SystemData::DefaultMaxNumSystemMessages, 
		&(pvColVal[SystemEmpireData::MaxNumSystemMessages])
		);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	pvColVal[SystemEmpireData::ClassicScore] = (float) 0.0;
	pvColVal[SystemEmpireData::AlmonasterScore] = fScore;

	iErrCode = pTable->ReadData (SystemData::DefaultUIButtons, &(pvColVal[SystemEmpireData::UIButtons]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIBackground, &(pvColVal[SystemEmpireData::UIBackground]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUILivePlanet, &(pvColVal[SystemEmpireData::UILivePlanet]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIDeadPlanet, &(pvColVal[SystemEmpireData::UIDeadPlanet]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUISeparator, &(pvColVal[SystemEmpireData::UISeparator]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	pvColVal[SystemEmpireData::AlmonasterTheme] = INDIVIDUAL_ELEMENTS;
	pvColVal[SystemEmpireData::AlternativeGraphicsPath] = "";

	ENUMERATE_SHIP_TYPES (i) {

		iErrCode = pTable->ReadData (
			SYSTEM_DATA_SHIP_NAME_COLUMN[i], 
			pvColVal + SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[i]
			);
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIHorz, &(pvColVal[SystemEmpireData::UIHorz]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIVert, &(pvColVal[SystemEmpireData::UIVert]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIColor, &(pvColVal[SystemEmpireData::UIColor]));
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	pvColVal[SystemEmpireData::CustomTableColor] = "000000";
	pvColVal[SystemEmpireData::Options] = iOptions;
	pvColVal[SystemEmpireData::MaxNumShipsBuiltAtOnce] = 10;
	pvColVal[SystemEmpireData::CreationTime] = tTime;
	pvColVal[SystemEmpireData::NumLogins] = 0;
	pvColVal[SystemEmpireData::Browser] = "";

	pvColVal[SystemEmpireData::CustomTextColor] = "000000";
	pvColVal[SystemEmpireData::CustomGoodColor] = "000000";
	pvColVal[SystemEmpireData::CustomBadColor] = "000000";
	pvColVal[SystemEmpireData::CustomPrivateMessageColor] = "000000";
	pvColVal[SystemEmpireData::CustomBroadcastMessageColor] = "000000";
	pvColVal[SystemEmpireData::SessionId] = NO_SESSION_ID;
	pvColVal[SystemEmpireData::DefaultBuilderPlanet] = HOMEWORLD_DEFAULT_BUILDER_PLANET;
	pvColVal[SystemEmpireData::DefaultMessageTarget] = MESSAGE_TARGET_NONE;

	pvColVal[SystemEmpireData::AlmonasterScoreSignificance] = iAlmonasterScoreSignificance;
	pvColVal[SystemEmpireData::VictorySneer] = "";

	pvColVal[SystemEmpireData::BridierRank] = BRIDIER_INITIAL_RANK;
	pvColVal[SystemEmpireData::BridierIndex] = BRIDIER_INITIAL_INDEX;
	pvColVal[SystemEmpireData::LastBridierActivity] = tTime;

	pTable->Release();
	pTable = NULL;

	iErrCode = m_pGameData->InsertRow (SYSTEM_EMPIRE_DATA, pvColVal, &iKey);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Return value
	*piEmpireKey = iKey;

	// Create a system messages table for the new empire
	GET_SYSTEM_EMPIRE_MESSAGES (pszTable, iKey);
	iErrCode = m_pGameData->CreateTable (pszTable, SystemEmpireMessages::Template.Name);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	// Create "SystemEmpireActiveGames(I)" table
	GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iKey);
	iErrCode = m_pGameData->CreateTable (pszTable, SystemEmpireActiveGames::Template.Name);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	// Create SystemEmpireNukedList(I) table
	GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iKey);
	iErrCode = m_pGameData->CreateTable (pszTable, SystemEmpireNukeList::Template.Name);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	// Create SystemEmpireNukerList(I) table
	GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iKey);
	iErrCode = m_pGameData->CreateTable (pszTable, SystemEmpireNukeList::Template.Name);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	// Add to top lists
	ENUMERATE_SCORING_SYSTEMS (i) {

		iErrCode = UpdateTopListOnIncrease ((ScoringSystem) i, iKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	}

	if (iParentKey != NO_KEY) {
		
		// This should succeed, but it's not fatal
		int iErrCode2 = RemoveEmpire (iParentKey);
		Assert (iErrCode2 == OK);
	}

	// Notification
	if (m_pUIEventSink != NULL) {
		m_pUIEventSink->OnCreateEmpire (iKey);
	}

Cleanup:

	if (iErrCode != OK && *piEmpireKey != NO_KEY) {

		// Best effort delete the new empire's database stuff
		m_pGameData->DeleteRow (SYSTEM_EMPIRE_DATA, iKey);
		
		GET_SYSTEM_EMPIRE_MESSAGES (pszTable, iKey)
		m_pGameData->DeleteTable (pszTable);

		GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iKey)
		m_pGameData->DeleteTable (pszTable);

		GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iKey)
		m_pGameData->DeleteTable (pszTable);

		GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iKey)
		m_pGameData->DeleteTable (pszTable);

		ENUMERATE_SCORING_SYSTEMS (i) {
			UpdateTopListOnDeletion ((ScoringSystem) i, iKey);
		}
	}

	UnlockEmpires();

	if (bParentLocked) {
		UnlockEmpire (nmParentMutex);
	}

	if (pTable != NULL) {
		pTable->Release();
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *pstrName -> Name
//
// Return an empire's name

int GameEngine::GetEmpireName (int iEmpireKey, Variant* pvName) {

	bool bExists;
	if (m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iEmpireKey, &bExists) != OK || !bExists) {
		return ERROR_EMPIRE_DOES_NOT_EXIST;
	}

	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Name, pvName);
}


// Input:
// iEmpireKey -> Integer key of empire
// strName -> New name
//
// Update an empire's name

int GameEngine::SetEmpireName (int iEmpireKey, const char* pszName) {

	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Name, pszName);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> New password
//
// Update an empire's password via admin interface

int GameEngine::SetEmpirePassword (int iEmpireKey, const char* pszPassword) {

	if (iEmpireKey == ROOT_KEY) {
		return ERROR_CANNOT_MODIFY_ROOT;
	}

	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, pszPassword);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> New password
//
// Update an empire's password from ProfileEditor

int GameEngine::ChangeEmpirePassword (int iEmpireKey, const char* pszPassword) {

	if (iEmpireKey == GUEST_KEY) {
		return ERROR_CANNOT_MODIFY_GUEST;
	}

	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, pszPassword);
}


// Input:
// iEmpireKey -> Integer key of empire
// strName -> New real name
//
// Update an empire's real name

int GameEngine::SetEmpireRealName (int iEmpireKey, const char* pszRealName) {

	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::RealName, pszRealName);
}


// Input:
// iEmpireKey -> Integer key of empire
// strName -> New email address
//
// Update an empire's email address

int GameEngine::SetEmpireEmail (int iEmpireKey, const char* pszEmail) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Email, pszEmail);
}


// Input:
// iEmpireKey -> Integer key of empire
// strName -> New web page
//
// Update an empire's web page

int GameEngine::SetEmpireWebPage (int iEmpireKey, const char* pszWebPage) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::WebPage, pszWebPage);
}

// Input:
// iEmpireKey -> Integer key of empire
// iBackgroundKey -> New key
//
// Update an empire's background key

int GameEngine::SetEmpireBackgroundKey (int iEmpireKey, int iBackgroundKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIBackground, iBackgroundKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iLivePlanetKey -> New key
//
// Update an empire's live planet key

int GameEngine::SetEmpireLivePlanetKey (int iEmpireKey, int iLivePlanetKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UILivePlanet, iLivePlanetKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iDeadPlanetKey -> New key
//
// Update an empire's dead planet key

int GameEngine::SetEmpireDeadPlanetKey (int iEmpireKey, int iDeadPlanetKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIDeadPlanet, iDeadPlanetKey);
}

// Input:
// iEmpireKey -> Integer key of empire
// iButtonKey -> New key
//
// Update an empire's background key

int GameEngine::SetEmpireButtonKey (int iEmpireKey, int iButtonKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIButtons, iButtonKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iSeparatorKey -> New key
//
// Update an empire's separator key

int GameEngine::SetEmpireSeparatorKey (int iEmpireKey, int iSeparatorKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UISeparator, iSeparatorKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iHorzKey -> New key
//
// Update an empire's horizontal key

int GameEngine::SetEmpireHorzKey (int iEmpireKey, int iHorzKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIHorz, iHorzKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iVertKey -> New key
//
// Update an empire's vertical key

int GameEngine::SetEmpireVertKey (int iEmpireKey, int iVertKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIVert, iVertKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iColorKey -> New color key
//
// Update an empire's color key

int GameEngine::SetEmpireColorKey (int iEmpireKey, int iColorKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIColor, iColorKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// strTableColor -> New custom table color
//
// Update an empire's custom table color

int GameEngine::SetEmpireCustomTableColor (int iEmpireKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomTableColor, pszColor);
}

int GameEngine::SetEmpireCustomTextColor (int iEmpireKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomTextColor, pszColor);
}

int GameEngine::SetEmpireCustomGoodColor (int iEmpireKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomGoodColor, pszColor);
}

int GameEngine::SetEmpireCustomBadColor (int iEmpireKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomBadColor, pszColor);
}

int GameEngine::SetEmpireCustomPrivateMessageColor (int iEmpireKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomPrivateMessageColor, pszColor);
}

int GameEngine::SetEmpireCustomBroadcastMessageColor (int iEmpireKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomBroadcastMessageColor, pszColor);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *pstrTableColor -> Custom table color
//
// Get an empire's custom table color

int GameEngine::GetEmpireCustomTableColor (int iEmpireKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomTableColor, pvColor);
}

int GameEngine::GetEmpireCustomTextColor (int iEmpireKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomTextColor, pvColor);
}

int GameEngine::GetEmpireCustomGoodColor (int iEmpireKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomGoodColor, pvColor);
}

int GameEngine::GetEmpireCustomBadColor (int iEmpireKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomBadColor, pvColor);
}

int GameEngine::GetEmpireCustomPrivateMessageColor (int iEmpireKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomPrivateMessageColor, pvColor);
}

int GameEngine::GetEmpireCustomBroadcastMessageColor (int iEmpireKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::CustomBroadcastMessageColor, pvColor);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piLivePlanetKey -> Live planet key
//
// Get an empire's live planet key

int GameEngine::GetEmpireLivePlanetKey (int iEmpireKey, int* piLivePlanetKey) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UILivePlanet, &vTemp);
	
	if (iErrCode == OK) {
		*piLivePlanetKey = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piDeadPlanetKey -> Dead planet key
//
// Get an empire's dead planet key

int GameEngine::GetEmpireDeadPlanetKey (int iEmpireKey, int* piDeadPlanetKey) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIDeadPlanet, &vTemp);
	
	if (iErrCode == OK) {
		*piDeadPlanetKey = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
// iThemeKey -> New key
//
// Update an empire's theme key

int GameEngine::SetEmpireThemeKey (int iEmpireKey, int iThemeKey) {
	return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterTheme, iThemeKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPath -> New path
//
// Update an empire's alternative graphics path

int GameEngine::SetEmpireAlternativeGraphicsPath (int iEmpireKey, const char* pszPath) {

	return m_pGameData->WriteData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::AlternativeGraphicsPath, 
		pszPath
		);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *pstrPath -> Path
//
// Get an empire's alternative graphics path

int GameEngine::GetEmpireAlternativeGraphicsPath (int iEmpireKey, Variant* pvPath) {
	
	return m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::AlternativeGraphicsPath, 
		pvPath
		);
}


// Input:
// iEmpireKey -> Empire Key
// iMaxNumSavedMessages -> Number of messages to be saved
//
// Set the empire's MaxNumSavedSystemMessages parameter

int GameEngine::SetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int iMaxNumSavedMessages) {
	
	Variant vMaxNum;
	unsigned int iNumMessages;

	unsigned int * piKey = NULL;
	Variant* pvTimeStamp = NULL;
	
	SYSTEM_EMPIRE_MESSAGES (strSystemEmpireMessages, iEmpireKey);

	// Lock message table
	NamedMutex nmMutex;
	LockEmpireSystemMessages (iEmpireKey, &nmMutex);
	
	// Get num messages and current max number of messages
	int iErrCode = m_pGameData->GetNumRows (strSystemEmpireMessages, &iNumMessages);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::MaxNumSystemMessages, 
		&vMaxNum
		);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Set the max number of messages
	iErrCode = m_pGameData->WriteData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::MaxNumSystemMessages, 
		iMaxNumSavedMessages
		);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// If we're going to be over the limit, trim the list of unread messages
	if (vMaxNum.GetInteger() > iMaxNumSavedMessages && 
		iNumMessages > (unsigned int) iMaxNumSavedMessages) {
		
		// Get the oldest messages' keys		
		iErrCode = m_pGameData->ReadColumn (
			strSystemEmpireMessages,
			SystemEmpireMessages::TimeStamp,
			&piKey,
			&pvTimeStamp,
			&iNumMessages
			);

		if (iErrCode == ERROR_DATA_NOT_FOUND) {
			iErrCode = OK;
		} else {

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			Assert (iNumMessages > 0);

			// Sort the messages by age
			Algorithm::QSortTwoAscending<Variant, int> (pvTimeStamp, (int*) piKey, iNumMessages);
			
			// Delete read messages until we're below the limit
			int i = 0;
			Variant vUnread;

			while (iNumMessages > (unsigned int) iMaxNumSavedMessages) {
				
				// Has message been read
				iErrCode = m_pGameData->ReadData (
					strSystemEmpireMessages, 
					piKey[i], 
					SystemEmpireMessages::Unread, 
					&vUnread
					);
				
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (vUnread.GetInteger() != 0) {
					continue;
				}
					
				iErrCode = m_pGameData->DeleteRow (strSystemEmpireMessages, piKey[i]);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				iNumMessages --;
				
				i ++;
			}
		}
	}
	
Cleanup:

	// Unlock the messages table
	UnlockEmpireSystemMessages (nmMutex);

	if (piKey != NULL) {
		m_pGameData->FreeKeys (piKey);
	}

	if (pvTimeStamp != NULL) {
		m_pGameData->FreeData (pvTimeStamp);
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
// *pszQuote -> New quote string
//
// Output:
// *pbUpdate -> true if quote was update, false if not
//
// Update the empire's quote if a new quote was input

int GameEngine::UpdateEmpireQuote (int iEmpireKey, const char* pszQuote) {

	return UpdateEmpireString (iEmpireKey, SystemEmpireData::Quote, pszQuote);
}

int GameEngine::UpdateEmpireVictorySneer (int iEmpireKey, const char* pszSneer) {

	return UpdateEmpireString (iEmpireKey, SystemEmpireData::VictorySneer, pszSneer);
}

int GameEngine::UpdateEmpireString (int iEmpireKey, int iColumn, const char* pszString) {


	IWriteTable* pWriteTable = NULL;
	IReadTable* pReadTable = NULL;

	const char* pszOldString;

	int iErrCode = m_pGameData->GetTableForWriting (SYSTEM_EMPIRE_DATA, &pWriteTable);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pWriteTable->QueryInterface (IID_IReadTable, (void**) &pReadTable);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pReadTable->ReadData (iEmpireKey, iColumn, &pszOldString);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (String::StrCmp (pszString, pszOldString) != 0) {
		iErrCode = pWriteTable->WriteData (iEmpireKey, iColumn, pszString);
	} else {
		iErrCode = WARNING;
	}

Cleanup:

	if (pWriteTable != NULL) {
		pWriteTable->Release();
	}

	if (pReadTable != NULL) {
		pReadTable->Release();
	}

	return iErrCode;
}


int GameEngine::SendVictorySneer (int iWinnerKey, const char* pszWinnerName, int iLoserKey) {

	int iErrCode;
	Variant vSneer;

	iErrCode = m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA,
		iWinnerKey,
		SystemEmpireData::VictorySneer,
		&vSneer
		);

	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (!String::IsBlank (vSneer.GetCharPtr())) {

		Variant vWinnerName;

		if (pszWinnerName == NULL) {

			iErrCode = GetEmpireName (iWinnerKey, &vWinnerName);
			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}
			pszWinnerName = vWinnerName.GetCharPtr();
		}

		String strMessage = "The victory sneer for " BEGIN_STRONG;
		
		strMessage += pszWinnerName;
		strMessage += END_STRONG " says:\n\n";

		strMessage.AppendHtml (vSneer.GetCharPtr(), 0, true);

		iErrCode = SendSystemMessage (
			iLoserKey,
			strMessage.GetCharPtr(),
			SYSTEM
			);
		
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
	}

	return iErrCode;
}

// Input:
// iEmpireKey -> Empire Key
//
// Marks an empire ready for deletion or deletes it if it is in no active games

int GameEngine::DeleteEmpire (int iEmpireKey) {

	int iErrCode = OK;

	NamedMutex nmMutex;
	LockEmpire (iEmpireKey, &nmMutex);

	// Read active games total
	unsigned int iNumGames;
	SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey)

	iErrCode = m_pGameData->GetNumRows (pszGames, &iNumGames);
	
	if (iErrCode != OK) {
		iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
	} else {

		// Check for presence of empire in active games
		if (iNumGames > 0) {
			
			// Mark empire ready for deletion
			iErrCode = m_pGameData->WriteOr (
				SYSTEM_EMPIRE_DATA, 
				iEmpireKey, 
				SystemEmpireData::Options, 
				EMPIRE_MARKED_FOR_DELETION
				);
			
			if (iErrCode == OK) {
				iErrCode = ERROR_EMPIRE_IS_IN_GAMES;
			}
			
		} else {		
			
			// Delete empire now!
			LockEmpires();
			iErrCode = RemoveEmpire (iEmpireKey);
			UnlockEmpires();
		}
	}

	UnlockEmpire (nmMutex);

	return iErrCode;
}

// Input:
// iEmpireKey -> Empire Key
//
// Output
// *pvRealName -> Return string
//
// Get the empire's real name

int GameEngine::GetEmpireRealName (int iEmpireKey, Variant* pvRealName) {

	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::RealName, pvRealName);
}


// Input:
// iEmpireKey -> Empire Key
//
// Output
// *pvEmail -> Return string
//
// Get the empire's email address

int GameEngine::GetEmpireEmail (int iEmpireKey, Variant* pvEmail) {

	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Email, pvEmail);
}


// Input:
// iEmpireKey -> Empire Key
//
// Output
// *pvWebPage -> Return string
//
// Get the empire's web page

int GameEngine::GetEmpireWebPage (int iEmpireKey, Variant* pvWebPage) {

	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::WebPage, pvWebPage);
}


// Input:
// iEmpireKey -> Empire Key
//
// Output
// *piMaxNumSavedMessages -> Return value
//
// Get the empire's max num saved system messages

int GameEngine::GetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int* piMaxNumSavedMessages) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxNumSystemMessages, &vTemp);
	
	if (iErrCode == OK) {
		*piMaxNumSavedMessages = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
//
// Delete an empire and "ruin" it out of all games

int GameEngine::ObliterateEmpire (int iEmpireKey, int iKillerEmpire) {

	if (iEmpireKey == ROOT_KEY) {
		return ERROR_CANNOT_MODIFY_ROOT;
	}

	if (iEmpireKey == GUEST_KEY) {
		return ERROR_CANNOT_MODIFY_GUEST;
	}

	if (iKillerEmpire == NO_KEY) {
		return ERROR_EMPIRE_DOES_NOT_EXIST;
	}

	// Read active games total
	unsigned int iNumGames;	
	Variant* pvGame;
	
	NamedMutex nmMutex;
	LockEmpire (iEmpireKey, &nmMutex);

	SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey)

	int iErrCode = m_pGameData->ReadColumn (
		pszGames, 
		SystemEmpireActiveGames::GameClassGameNumber,
		&pvGame,
		&iNumGames
		);

	if (iErrCode == ERROR_DATA_NOT_FOUND) {
		iErrCode = OK;
	} else {

		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		// Ruin empire out of each game
		NamedMutex nmGameMutex;
		bool bFlag;
		unsigned int i;
		int iGameClass, iGameNumber;
		
		char strGameData [128];

		for (i = 0; i < iNumGames; i ++) {
			
			GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);
			
			GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

			// Pretend we're an update
			iErrCode = WaitGameWriter (iGameClass, iGameNumber);
			if (iErrCode != OK) {
				continue;	// Game must be dead
			}

			// Lock the game
			LockGame (iGameClass, iGameNumber, &nmGameMutex);

			// Is empire in the game
			iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
			if (iErrCode != OK) {
				Assert (false);
				goto EndGame;
			}
			
			if (bFlag) {
				
				// Try to quit the empire from the game nicely
				iErrCode = QuitEmpireFromGameInternal (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
				
				if (iErrCode != OK) {
					
					// The empire couldn't be removed nicely,
					// so let's delete the empire the hard way
					iErrCode = RemoveEmpireFromGameInternal (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
					if (iErrCode != OK) {
						Assert (false);
						goto EndGame;
					}
				}
			}
EndGame:
			UnlockGame (nmGameMutex);

			// Always try to unlock game
			SignalGameWriter (iGameClass, iGameNumber);
		}
		
		FreeData (pvGame);
	}

	// Delete empire from server now
	LockEmpires();
	iErrCode = RemoveEmpire (iEmpireKey);
	UnlockEmpires();

Cleanup:

	UnlockEmpire (nmMutex);

	return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire to be deleted

int GameEngine::RemoveEmpire (int iEmpireKey) {

	int iErrCode = OK, iErrCode2;

	// Delete row from SystemEmpireData table
	iErrCode = m_pGameData->DeleteRow (SYSTEM_EMPIRE_DATA, iEmpireKey);
	if (iErrCode != OK) {
		// Empire probably no longer exists
		return iErrCode;
	}

	// Best effort delete messages table
	SYSTEM_EMPIRE_MESSAGES (pszTable, iEmpireKey);
	iErrCode2 = m_pGameData->DeleteTable (pszTable);
	Assert (iErrCode2 == OK);

	// Delete empire's SystemEmpireActiveGames(I) table
	GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iEmpireKey)
	iErrCode2 = m_pGameData->DeleteTable (pszTable);
	Assert (iErrCode2 == OK);

	// Best effort delete / halt all personal gameclasses
	int* piGameClassKey;
	int i, iNumKeys;
	
	iErrCode2 = GetEmpireGameClassKeys (iEmpireKey, &piGameClassKey, &iNumKeys);
	if (iErrCode2 == OK && iNumKeys > 0) {
		
		bool bDelete;
		for (i = 0; i < iNumKeys; i ++) {
			
			iErrCode2 = DeleteGameClass (piGameClassKey[i], &bDelete);
			Assert (iErrCode2 == OK);
			
			if (iErrCode2 == OK && !bDelete) {

				iErrCode = m_pGameData->WriteData (
					SYSTEM_GAMECLASS_DATA,
					piGameClassKey[i],
					SystemGameClassData::Owner,
					DELETED_EMPIRE_KEY
					);
				Assert (iErrCode == OK);
			}
		}
		
		FreeKeys (piGameClassKey);
	}

	// Best effort delete nuked table
	GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iEmpireKey);
	iErrCode2 = m_pGameData->DeleteTable (pszTable);
	Assert (iErrCode2 == OK);

	// Best effort delete nuker table
	GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iEmpireKey);
	iErrCode2 = m_pGameData->DeleteTable (pszTable);
	Assert (iErrCode2 == OK);

	// Notification
	if (m_pUIEventSink != NULL) {
		m_pUIEventSink->OnDeleteEmpire (iEmpireKey);
	}

	// Remove from top lists
	ENUMERATE_SCORING_SYSTEMS (i) {

		iErrCode2 = UpdateTopListOnDeletion ((ScoringSystem) i, iEmpireKey);
		Assert (iErrCode2 == OK);
	}

	return iErrCode;
}


// Input:
// pszName -> Name of empire
//
// Output:
// *pbExists -> true if exists, false if not
// *piEmpireKey -> Key of empire
// *pvEmpireName -> Proper capitalization of empire name
//
// Determines if a given empire name already exists

int GameEngine::DoesEmpireExist (const char* pszName, bool* pbExists, int* piEmpireKey, 
								 Variant* pvEmpireName) {

	int iErrCode = OK;

	*pbExists = false;
	*piEmpireKey = NO_KEY;

	iErrCode = m_pGameData->GetFirstKey (
		SYSTEM_EMPIRE_DATA, 
		SystemEmpireData::Name, 
		pszName, 
		true, 
		(unsigned int*) piEmpireKey
		);

	// Fetch proper capitalization
	if (iErrCode == OK) {

		Assert (*piEmpireKey != NO_KEY);
		
		*pbExists = true;
		
		if (pvEmpireName != NULL) {

			iErrCode = m_pGameData->ReadData (
				SYSTEM_EMPIRE_DATA, 
				*piEmpireKey, 
				SystemEmpireData::Name, 
				pvEmpireName
				);
		}

	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *pbExists -> true if exists, false if not
// *pvEmpireName -> Name of empire (undefined if empire doesn't exist)
//
// Determines if a given empire key exists

int GameEngine::DoesEmpireExist (int iEmpireKey, bool* pbExists, Variant* pvEmpireName) {

	int iErrCode = m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iEmpireKey, pbExists);

	if (iErrCode && *pbExists) {
		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Name, pvEmpireName);
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *pbExists -> true if exists, false if not
//
// Determines if a given empire key exists

int GameEngine::DoesEmpireExist (int iEmpireKey, bool* pbExists) {
	return m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iEmpireKey, pbExists);
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *pbMatch-> true if key matches name
//
// Determines if a given empire key matches a given empire name

int GameEngine::DoesEmpireKeyMatchName (int iEmpireKey, const char* pszEmpireName, bool* pbMatch) {

	int iErrCode;

	// Check row
	iErrCode = m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iEmpireKey, pbMatch);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (*pbMatch) {
		
		Variant vEmpireName;
		
		iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Name, &vEmpireName);
		if (iErrCode != OK) {
			return iErrCode;
		}

		*pbMatch = (String::StriCmp (pszEmpireName, vEmpireName.GetCharPtr()) == 0);
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> Password to be tested
//
// Given an empire name and a password, determines if that password is correct for that empire
// Return OK if yes, an error if no

int GameEngine::IsPasswordCorrect (int iEmpireKey, const char* pszPassword) {

	int iErrCode;
	Variant vPassword;

	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, &vPassword);
	if (iErrCode != OK) {
		return iErrCode;
	}

	return String::StrCmp (vPassword.GetCharPtr(), pszPassword) == 0 ? OK : ERROR_WRONG_PASSWORD;
}


// Input:
// iEmpireKey -> Integer key of empire
// pszBrowser -> Browser used to request login
//
// Perform a login for an empire and update volatile parameters.  Should be called after 
// the empire's password has been validated and the empire's key has been obtained.

int GameEngine::LoginEmpire (int iEmpireKey, const char* pszBrowser) {

	Variant vLogins, vPriv;

	bool bGoodDatabase = true;

	char pszBadTable [512];
	
	// Make sure logins are allowed
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::Options, &vLogins);
	if (iErrCode != OK) {
		return iErrCode;
	}

	// Get the time
	UTCTime tTime;
	Time::GetTime (&tTime);

	NamedMutex nmMutex;
	LockEmpire (iEmpireKey, &nmMutex);

	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, &vPriv);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	if (!(vLogins.GetInteger() & LOGINS_ENABLED)) {

		// Only admins can pass...
		if (vPriv.GetInteger() < ADMINISTRATOR) {
			iErrCode = ERROR_DISABLED;
			goto Cleanup;
		}
	}
	
	//
	// Verify tables
	//

	// SystemEmpireMessages(I)
	GET_SYSTEM_EMPIRE_MESSAGES (pszBadTable, iEmpireKey);
	
	if (!m_pGameData->DoesTableExist (pszBadTable)) {
		bGoodDatabase = false;
		goto EndCheck;
	}
	
	// SystemEmpireNukedList(I)
	GET_SYSTEM_EMPIRE_NUKED_LIST (pszBadTable, iEmpireKey);
	
	if (!m_pGameData->DoesTableExist (pszBadTable)) {
		bGoodDatabase = false;
		goto EndCheck;
	}
	
	// SystemEmpireNukerList(I)
	GET_SYSTEM_EMPIRE_NUKER_LIST (pszBadTable, iEmpireKey);
	
	if (!m_pGameData->DoesTableExist (pszBadTable)) {
		bGoodDatabase = false;
		goto EndCheck;
	}
	
	// SystemEmpireActiveGames(I)
	GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszBadTable, iEmpireKey);
	
	if (!m_pGameData->DoesTableExist (pszBadTable)) {
		bGoodDatabase = false;
		goto EndCheck;
	}
	
EndCheck:
	
	if (!bGoodDatabase) {
		
		char* pszError = (char*) StackAlloc (512 + strlen (pszBadTable));
		sprintf (pszError, "Error logging in empire %i; the missing table was %s", iEmpireKey, pszBadTable);
		m_pReport->WriteReport (pszError);
		
		iErrCode = ERROR_DATA_CORRUPTION;
		goto Cleanup;
	}

	if (String::StrLen (pszBrowser) > MAX_BROWSER_NAME_LENGTH) {
		
		char pszCutBrowser [MAX_BROWSER_NAME_LENGTH + 1];
		strncpy (pszCutBrowser, pszBrowser, MAX_BROWSER_NAME_LENGTH);
		pszCutBrowser [MAX_BROWSER_NAME_LENGTH] = '\0';

		iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Browser, pszCutBrowser);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

	} else {
		
		iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Browser, pszBrowser);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	}

	// Write LastLoginTime
	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::LastLoginTime, tTime);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Increment the number of logins
	iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::NumLogins, 1);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

Cleanup:

	UnlockEmpire (nmMutex);

	// Outside the lock...  if we're an admin, best effort test all closed games for an update
	if (iErrCode == OK) {
		
		// Notification
		if (m_pUIEventSink != NULL) {
			m_pUIEventSink->OnLoginEmpire (iEmpireKey);
		}
		
		if (vPriv.GetInteger() == ADMINISTRATOR) {

			int iErrCode2 = CheckAllGamesForUpdates();
			Assert (iErrCode2 == OK);
		}
	}

	return iErrCode;
}


// Output:
// *piNumEmpires -> Number of empires
//
// Returns the number of empires currently registered on the server

int GameEngine::GetNumEmpiresOnServer (int* piNumEmpires) {

	return m_pGameData->GetNumRows (SYSTEM_EMPIRE_DATA, (unsigned int*) piNumEmpires);
}

// Input:
// iEmpireKey -> Empire Key
//
// Undeletes an empire marked for deletion

int GameEngine::UndeleteEmpire (int iEmpireKey) {

	Variant vTemp;

	NamedMutex nmEmpireMutex;
	LockEmpire (iEmpireKey, &nmEmpireMutex);

	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vTemp);
	if (iErrCode == OK && vTemp.GetInteger() & EMPIRE_MARKED_FOR_DELETION) {

		iErrCode = m_pGameData->WriteAnd (
			SYSTEM_EMPIRE_DATA, 
			iEmpireKey, 
			SystemEmpireData::Options, 
			~EMPIRE_MARKED_FOR_DELETION
			);

	} else {
		iErrCode = ERROR_CANNOT_UNDELETE_EMPIRE;
	}

	UnlockEmpire (nmEmpireMutex);

	return iErrCode;
}

// Input:
// iEmpireKey -> Integer key of empire
//
// Sets all of an empire's statistics to their default values

int GameEngine::BlankEmpireStatistics (int iEmpireKey) {

	int iErrCode;

	char pszTable [256];
	Variant vOldClassicScore, vOldAlmonasterScore, vOldBridierIndex;

	if (iEmpireKey == ROOT_KEY) {
		return ERROR_CANNOT_MODIFY_ROOT;
	}

	if (iEmpireKey == GUEST_KEY) {
		return ERROR_CANNOT_MODIFY_GUEST;
	}

	NamedMutex nmEmpireMutex, nmBridierMutex;
	LockEmpire (iEmpireKey, &nmEmpireMutex);

	// Get old scores
	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::ClassicScore, &vOldClassicScore);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vOldAlmonasterScore);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	//
	// Blank Bridier Score
	//

	// Lock
	LockEmpireBridier (iEmpireKey, &nmBridierMutex);

	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierIndex, &vOldBridierIndex);
	if (iErrCode != OK) {
		UnlockEmpireBridier (nmBridierMutex);
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierRank, BRIDIER_INITIAL_RANK);
	if (iErrCode != OK) {
		UnlockEmpireBridier (nmBridierMutex);
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierIndex, BRIDIER_INITIAL_INDEX);
	
	// Unlock
	UnlockEmpireBridier (nmBridierMutex);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Blank statistics
	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Wins, 0);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Nukes, 0);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Nuked, 0);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Draws, 0);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Ruins, 0);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxEcon, 0);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxMil, 0);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Blank Classic Score
	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::ClassicScore, CLASSIC_INITIAL_SCORE);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	// Blank Almonaster Score, Significance and privilege level
	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, ALMONASTER_INITIAL_SCORE);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScoreSignificance, 0);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, NOVICE);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Blank nuke history fields
	GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iEmpireKey);

	iErrCode = m_pGameData->DeleteAllRows (pszTable);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iEmpireKey);

	iErrCode = m_pGameData->DeleteAllRows (pszTable);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Update top lists
	if (vOldClassicScore.GetFloat() > CLASSIC_INITIAL_SCORE) {
		iErrCode = UpdateTopListOnDecrease (CLASSIC_SCORE, iEmpireKey);
	}

	else if (vOldClassicScore.GetFloat() < CLASSIC_INITIAL_SCORE) {
		iErrCode = UpdateTopListOnIncrease (CLASSIC_SCORE, iEmpireKey);
	}

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (vOldAlmonasterScore.GetFloat() > ALMONASTER_INITIAL_SCORE) {

		iErrCode = UpdateTopListOnDecrease (ALMONASTER_SCORE, iEmpireKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	}

	if (vOldBridierIndex.GetInteger() <= BRIDIER_TOPLIST_INDEX) {

		iErrCode = UpdateTopListOnDecrease (BRIDIER_SCORE, iEmpireKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	}

	if (vOldBridierIndex.GetInteger() <= BRIDIER_ESTABLISHED_TOPLIST_INDEX) {

		iErrCode = UpdateTopListOnDecrease (BRIDIER_SCORE_ESTABLISHED, iEmpireKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	}

Cleanup:

	UnlockEmpire (nmEmpireMutex);

	return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppvGameClassKey -> Array of keys
// *piNumKeys -> Number of game classes
//
// Return the keys of the personal gameclasses an empire has created

int GameEngine::GetEmpireGameClassKeys (int iEmpireKey, int** ppiGameClassKey, int* piNumKeys) {

	int iOptions, iErrCode;
	unsigned int* piGameClassKey = NULL, i, iNumKeys;

	IReadTable* pTable = NULL;

	LockGameClasses();

	iErrCode = m_pGameData->GetTableForReading (
		SYSTEM_GAMECLASS_DATA,
		&pTable
		);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
		
	iErrCode = pTable->GetEqualKeys (
		SystemGameClassData::Owner,
		iEmpireKey,
		false,
		&piGameClassKey,
		&iNumKeys
		);

	if (iErrCode == ERROR_DATA_NOT_FOUND) {
		iErrCode = OK;
	}

	else if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Eliminate dynamic gameclasses from enumeration
	for (i = 0; i < iNumKeys; i ++) {

		iErrCode = pTable->ReadData (
			piGameClassKey[i],
			SystemGameClassData::Options,
			&iOptions
			);

		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (iOptions & DYNAMIC_GAMECLASS) {
			piGameClassKey[i] = piGameClassKey[-- iNumKeys];
			i --;
		}
	}

	*piNumKeys = iNumKeys;

Cleanup:

	UnlockGameClasses();

	if (pTable != NULL) {
		pTable->Release();
	}

	if (iErrCode == OK && ppiGameClassKey != NULL) {
		*ppiGameClassKey = (int*) piGameClassKey;
	} else {

		if (piGameClassKey != NULL) {
			m_pGameData->FreeKeys (piGameClassKey);
			piGameClassKey = NULL;
		}
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppvEmpData -> SystemEmpireData array
//
// *piNumActiveGames -> Number of games the empire is active in
// 
// Returns empire data

int GameEngine::GetEmpireData (int iEmpireKey, Variant** ppvEmpData, int* piNumActiveGames) {

	SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey);

	// Get num active games
	int iErrCode = m_pGameData->GetNumRows (pszGames, (unsigned int*) piNumActiveGames);
	if (iErrCode != OK) {
		return iErrCode;
	}

	// Get data
	return m_pGameData->ReadRow (SYSTEM_EMPIRE_DATA, iEmpireKey, ppvEmpData);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiGameClass -> Array of game classes
// **ppiGameNumber -> Array of game numbers
//
// Returns the gameclasses and gamenumbers of the games an empire is currently in

int GameEngine::GetEmpireActiveGames (int iEmpireKey, int** ppiGameClass, int** ppiGameNumber, int* piNumGames) {

	*ppiGameClass = NULL;
	*ppiGameNumber = NULL;
	*piNumGames = 0;

	int iErrCode, i;
	IReadTable* pGames;

	SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey);
	
	iErrCode = m_pGameData->GetTableForReading (pszGames, &pGames);
	if (iErrCode != OK) {
		return iErrCode;
	}

	char** ppszGames;

	iErrCode = pGames->ReadColumn (
		SystemEmpireActiveGames::GameClassGameNumber,
		&ppszGames,
		(unsigned int*) piNumGames
		);

	if (iErrCode == ERROR_DATA_NOT_FOUND) {
		iErrCode = OK;
	}

	else if (iErrCode == OK) {

		*ppiGameClass = new int [*piNumGames];
		*ppiGameNumber = new int [*piNumGames];
		
		for (i = 0; i < *piNumGames; i ++) {
			GetGameClassGameNumber (ppszGames[i], &((*ppiGameClass)[i]), &((*ppiGameNumber)[i]));
		}

		m_pGameData->FreeData (ppszGames);
	}

	pGames->Release();

	return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of empire
//
// Output:
// *pbInGame -> true if in game, false otherwise
//
// Determine if an empire is in a game

int GameEngine::IsEmpireInGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbInGame) {

	int iErrCode = DoesGameExist (iGameClass, iGameNumber, pbInGame);
	if (iErrCode == OK && *pbInGame) {

		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		unsigned int iKey = NO_KEY;
		iErrCode = m_pGameData->GetFirstKey (
			pszEmpires, 
			GameEmpires::EmpireKey, 
			iEmpireKey, 
			false, 
			&iKey
			);

		*pbInGame = (iErrCode == OK && iKey != NO_KEY);

		if (iErrCode == ERROR_DATA_NOT_FOUND) {
			iErrCode = OK;
		}
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piThemeKey -> Key of theme
//
// Return the theme key of the empire

int GameEngine::GetEmpireThemeKey (int iEmpireKey, int* piThemeKey) {

	Variant vThemeKey;
	
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::AlmonasterTheme, 
		&vThemeKey
		);
	
	if (iErrCode == OK) {
		*piThemeKey = vThemeKey.GetInteger();
	}
	
	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piBackgroundKey -> Integer key
//
// Return the background key of the empire

int GameEngine::GetEmpireBackgroundKey (int iEmpireKey, int* piBackgroundKey) {

	Variant vBackgroundKey;
	
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIBackground, &vBackgroundKey);
	
	if (iErrCode == OK) {
		*piBackgroundKey = vBackgroundKey.GetInteger();
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piSeparatorKey -> Integer key
//
// Return the seperator key of the empire

int GameEngine::GetEmpireSeparatorKey (int iEmpireKey, int* piSeparatorKey) {

	Variant vSeparatorKey;
	
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UISeparator, &vSeparatorKey);
	
	if (iErrCode == OK) {
		*piSeparatorKey = vSeparatorKey.GetInteger();
	}
	
	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piButtonKey -> Integer key
//
// Return the button key of the empire

int GameEngine::GetEmpireButtonKey (int iEmpireKey, int* piButtonKey) {

	Variant vButtonKey;
	
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIButtons, &vButtonKey);
	
	if (iErrCode == OK) {
		*piButtonKey = vButtonKey.GetInteger();
	}
	
	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piHorzKey -> Integer key
//
// Return the horizontal bar key of the empire

int GameEngine::GetEmpireHorzKey (int iEmpireKey, int* piHorzKey) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIHorz, &vTemp);
	
	if (iErrCode == OK) {
		*piHorzKey = vTemp.GetInteger();
	}
	
	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piVertKey -> Integer key
//
// Return the vertical bar key of the empire

int GameEngine::GetEmpireVertKey (int iEmpireKey, int* piVertKey) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIVert, &vTemp);
	
	if (iErrCode == OK) {
		*piVertKey = vTemp.GetInteger();
	}
	
	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piTableColorKey -> Table color key
//
// Return the table color key of the empire

int GameEngine::GetEmpireColorKey (int iEmpireKey, int* piColorKey) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIColor, &vTemp);
	
	if (iErrCode == OK) {
		*piColorKey = vTemp.GetInteger();
	}
	
	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piPrivilege -> Privilege level
//
// Return the privilege level of an empire

int GameEngine::GetEmpirePrivilege (int iEmpireKey, int* piPrivilege) {

	Variant vPrivilege;
	
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, &vPrivilege);
	
	if (iErrCode == OK) {
		*piPrivilege = vPrivilege.GetInteger();
	}
	
	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// iPrivilege -> Privilege level
//
// Set the privilege level of an empire

int GameEngine::SetEmpirePrivilege (int iEmpireKey, int iPrivilege) {

	if (iEmpireKey == ROOT_KEY) {
		return ERROR_CANNOT_MODIFY_ROOT;
	}

	if (iEmpireKey == GUEST_KEY) {
		return ERROR_CANNOT_MODIFY_GUEST;
	}

	if (!IsLegalPrivilege (iPrivilege)) {
		return ERROR_INVALID_PRIVILEGE;
	}

	int iErrCode = m_pGameData->WriteData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::Privilege, 
		iPrivilege
		);
	
	if (iErrCode == OK) {

		char pszMessage [1024];
		sprintf (
			pszMessage, 
			"Your privilege level was changed to %s by an administrator", 
			PRIVILEGE_STRING[iPrivilege]
			);

		// Send a message to the affected empire
		iErrCode = SendSystemMessage (
			iEmpireKey,
			pszMessage,
			SYSTEM
			);
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *pfAlmonasterScore -> Almonaster score
//
// Return an empire's Almonaster score

int GameEngine::GetEmpireAlmonasterScore (int iEmpireKey, float* pfAlmonasterScore) {

	Variant vAlmonasterScore;
	
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vAlmonasterScore);
	
	if (iErrCode == OK) {
		*pfAlmonasterScore = vAlmonasterScore.GetFloat();
	}
	
	return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// fAlmonasterScore -> Almonaster score
//
// Set an empire's Almonaster score

int GameEngine::SetEmpireAlmonasterScore (int iEmpireKey, float fAlmonasterScore) {

	if (iEmpireKey == ROOT_KEY) {
		return ERROR_CANNOT_MODIFY_ROOT;
	}

	if (iEmpireKey == GUEST_KEY) {
		return ERROR_CANNOT_MODIFY_GUEST;
	}

	if (fAlmonasterScore < ALMONASTER_MIN_SCORE || fAlmonasterScore > ALMONASTER_MAX_SCORE) {
		return ERROR_INVALID_ARGUMENT;
	}
	
	Variant vOldScore;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vOldScore);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (fAlmonasterScore == vOldScore.GetFloat()) {
		return OK;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, fAlmonasterScore);
	if (iErrCode != OK) {
		return iErrCode;
	}

	iErrCode = CalculatePrivilegeLevel (iEmpireKey);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (fAlmonasterScore > vOldScore.GetFloat()) {
		iErrCode = UpdateTopListOnIncrease (ALMONASTER_SCORE, iEmpireKey);
	} else {
		iErrCode = UpdateTopListOnDecrease (ALMONASTER_SCORE, iEmpireKey);
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *pstrPassword -> Empire's password
//
// Return an empire's password

int GameEngine::GetEmpirePassword (int iEmpireKey, Variant* pvPassword) {
	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, pvPassword);
}


// Input:
// iEmpireKey -> Key of empire
// strColumn -> Name of column
//
// Output:
// *pvData -> Data column
//
// Get empire data column from SystemEmpireData

int GameEngine::GetEmpireDataColumn (int iEmpireKey, unsigned int iColumn, Variant* pvData) {
	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, iColumn, pvData);
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *piNumLogins -> Number of successful logins
//
// Get the number of successful logins performed by the empire

int GameEngine::GetNumLogins (int iEmpireKey, int* piNumLogins) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::NumLogins, &vTemp);
	
	if (iErrCode == OK) {
		*piNumLogins = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *piMaxNumShipsBuiltAtOnce -> Maximum number of ships the empire can build at once
//
// Get the maximum number of ships the empire can build at once

int GameEngine::GetEmpireMaxNumShipsBuiltAtOnce (int iEmpireKey, int* piMaxNumShipsBuiltAtOnce) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::MaxNumShipsBuiltAtOnce, 
		&vTemp
		);
	
	if (iErrCode == OK) {
		*piMaxNumShipsBuiltAtOnce = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
// iMaxNumShipsBuiltAtOnce -> Maximum number of ships the empire can build at once
//
// Set the maximum number of ships the empire can build at once

int GameEngine::SetEmpireMaxNumShipsBuiltAtOnce (int iEmpireKey, int iMaxNumShipsBuiltAtOnce) {

	return m_pGameData->WriteData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::MaxNumShipsBuiltAtOnce, 
		iMaxNumShipsBuiltAtOnce
		);
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// pvIPAddress -> Empire's IP address
//
// Get an empire's IP address

int GameEngine::GetEmpireIPAddress (int iEmpireKey, Variant* pvIPAddress) {

	return m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::IPAddress,
		pvIPAddress
		);
}


// Input:
// iEmpireKey -> Key of empire
// pszIPAddress -> New IP address
//
// Set an empire's IP address

int GameEngine::SetEmpireIPAddress (int iEmpireKey, const char* pszIPAddress) {

	return m_pGameData->WriteData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::IPAddress,
		pszIPAddress
		);
}

int GameEngine::GetEmpireSessionId (int iEmpireKey, int64* pi64SessionId) {

	int iErrCode;
	IReadTable* pSystemEmpireData;

	iErrCode = m_pGameData->GetTableForReading (SYSTEM_EMPIRE_DATA, &pSystemEmpireData);
	if (iErrCode != OK) {
		return iErrCode;
	}

	iErrCode = pSystemEmpireData->ReadData (iEmpireKey, SystemEmpireData::SessionId, pi64SessionId);

	pSystemEmpireData->Release();

	return iErrCode;
}

int GameEngine::SetEmpireSessionId (int iEmpireKey, int64 i64SessionId) {

	return m_pGameData->WriteData (
		SYSTEM_EMPIRE_DATA,
		iEmpireKey,
		SystemEmpireData::SessionId,
		i64SessionId
		);
}


int GameEngine::GetEmpireDefaultBuilderPlanet (int iEmpireKey, int* piDefaultBuildPlanet) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::DefaultBuilderPlanet,
		&vValue
		);

	if (iErrCode == OK) {
		*piDefaultBuildPlanet = vValue.GetInteger();
	}

	return iErrCode;
}


int GameEngine::SetEmpireDefaultBuilderPlanet (int iEmpireKey, int iDefaultBuildPlanet) {

	if (iDefaultBuildPlanet > NO_DEFAULT_BUILDER_PLANET ||
		iDefaultBuildPlanet < LAST_BUILDER_DEFAULT_BUILDER_PLANET) {
		return ERROR_INVALID_ARGUMENT;
	}

	return m_pGameData->WriteData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::DefaultBuilderPlanet,
		iDefaultBuildPlanet
		);
}


int GameEngine::GetEmpireDefaultMessageTarget (int iEmpireKey, int* piMessageTarget) {

	Variant vData;
	
	int iErrCode = m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA, 
		iEmpireKey, 
		SystemEmpireData::DefaultMessageTarget, 
		&vData
		);

	if (iErrCode == OK) {
		*piMessageTarget = vData.GetInteger();
	}

	return iErrCode;
}


int GameEngine::SetEmpireDefaultMessageTarget (int iEmpireKey, int iMessageTarget) {

	if (iMessageTarget < MESSAGE_TARGET_NONE || iMessageTarget > MESSAGE_TARGET_LAST_USED) {
		return ERROR_INVALID_ARGUMENT;
	}

	return m_pGameData->WriteData (
		SYSTEM_EMPIRE_DATA,
		iEmpireKey,
		SystemEmpireData::DefaultMessageTarget,
		iMessageTarget
		);
}


// Input:
// iEmpireKey -> Key of empire
//
// Tell the login process to reset the empire's session id on next login

int GameEngine::ResetEmpireSessionId (int iEmpireKey) {
	return SetEmpireOption (iEmpireKey, RESET_SESSION_ID, true);
}


// Input:
// iEmpireKey -> Key of empire
//
// Don't reset the empire's session id on next login

int GameEngine::EndResetEmpireSessionId (int iEmpireKey) {
	return SetEmpireOption (iEmpireKey, RESET_SESSION_ID, false);
}

// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *piOptions -> empire options from SystemEmpireData::Options
//
// Return an empire's options

int GameEngine::GetEmpireOptions (int iEmpireKey, int* piOptions) {

	int iErrCode;
	Variant vTemp;

	iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vTemp);
	if (iErrCode == OK) {
		*piOptions = vTemp.GetInteger();
	}

	return iErrCode;
}