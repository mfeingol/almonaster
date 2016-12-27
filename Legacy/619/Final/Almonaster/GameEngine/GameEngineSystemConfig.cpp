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

// Output:
// *pstrServerName -> Server name
//
// Return the name of the server

int GameEngine::GetServerName (Variant* pvServerName) {

	return m_pGameData->ReadData (SYSTEM_DATA, SystemData::ServerName, pvServerName);
}


// Input:
// strServerName -> Server name
//
// Set the name of the server

int GameEngine::SetServerName (const char* pszServerName) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::ServerName, pszServerName);
}


// Input:
// iTech -> Tech key
//
// Output:
// *pvShipName -> Default ship name
//
// Return the server's default ship name for a tech

int GameEngine::GetDefaultShipName (int iTech, Variant* pvShipName) {

	if (iTech < FIRST_SHIP || iTech > LAST_SHIP) {
		return ERROR_WRONG_TECHNOLOGY;
	}

	return m_pGameData->ReadData (SYSTEM_DATA, SYSTEM_DATA_SHIP_NAME_COLUMN [iTech], pvShipName);
}


// Output:
//	*piBackground
//	*piLivePlanet
//	*piDeadPlanet
//	*piButtons
//	*piSeparator
//	*piHorz
//	*piVert
//	*piColor
//
// Return the server's default UI keys

int GameEngine::GetDefaultUIKeys (int* piBackground, int* piLivePlanet, int* piDeadPlanet, int* piButtons,
								  int* piSeparator, int* piHorz, int* piVert, int* piColor) {

	IReadTable* pTable;
	int iErrCode = m_pGameData->GetTableForReading (SYSTEM_DATA, &pTable);
	if (iErrCode != OK) {
		return iErrCode;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIBackground, piBackground);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUILivePlanet, piLivePlanet);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIDeadPlanet, piDeadPlanet);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIButtons, piButtons);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUISeparator, piSeparator);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIHorz, piHorz);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIVert, piVert);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (SystemData::DefaultUIColor, piColor);
	if (iErrCode != OK) {
		goto Cleanup;
	}

Cleanup:

	pTable->Release();

	return iErrCode;
}

// Output:
// *piMaxNumPersonalGameClasses -> Maximum number of personal game classes
//
// Get the maximum number of personal game classes

int GameEngine::GetMaxNumPersonalGameClasses (int* piMaxNumPersonalGameClasses) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxNumPersonalGameClasses, &vTemp);
	
	if (iErrCode == OK) {
		*piMaxNumPersonalGameClasses = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input
// fScore -> Points
//
// Set the Almonaster score required for adepthood

int GameEngine::SetAdeptScore (float fScore) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ApprenticeScore, &vTemp);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (vTemp.GetFloat() > fScore) {
		return ERROR_INVALID_ARGUMENT;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_DATA, SystemData::AdeptScore, fScore);
	if (iErrCode != OK) {
		return iErrCode;
	}

	return ScanEmpiresOnScoreChanges();
}


// Output
// *pfScore -> Points
//
// Get the Almonaster score required for adepthood

int GameEngine::GetAdeptScore (float* pfScore) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AdeptScore, &vTemp);

	if (iErrCode == OK) {
		*pfScore = vTemp.GetFloat();
	}

	return iErrCode;
}


// Input
// fScore -> Points
//
// Set the Almonaster score required for apprenticeship

int GameEngine::SetApprenticeScore (float fScore) {

	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AdeptScore, &vTemp);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (vTemp.GetFloat() < fScore) {
		return ERROR_INVALID_ARGUMENT;
	}

	iErrCode = m_pGameData->WriteData (SYSTEM_DATA, SystemData::ApprenticeScore, fScore);
	if (iErrCode != OK) {
		return iErrCode;
	}

	return ScanEmpiresOnScoreChanges();
}

// When adept or apprentice score changes, this function calls

int GameEngine::ScanEmpiresOnScoreChanges() {

	unsigned int* piEmpireKey, iNumEmpires, i;

	int iErrCode = m_pGameData->GetAllKeys (
		SYSTEM_EMPIRE_DATA,
		&piEmpireKey,
		&iNumEmpires
		);

	if (iErrCode == OK) {

		for (i = 0; i < iNumEmpires; i ++) {
			// Best effort
			iErrCode = CalculatePrivilegeLevel (piEmpireKey[i]);
		}

		m_pGameData->FreeKeys (piEmpireKey);
	}

	return OK;
}


// Output
// *pfScore -> Points
//
// Get the Almonaster score required for apprenticeship

int GameEngine::GetApprenticeScore (float* pfScore) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ApprenticeScore, &vTemp);

	if (iErrCode == OK) {
		*pfScore = vTemp.GetFloat();
	}

	return iErrCode;
}


// Input:
// iMaxNumSystemMessages -> Maximum number of system messages
//
// Set the maximum number of system messages

int GameEngine::SetMaxNumSystemMessages (int iMaxNumSystemMessages) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxNumSystemMessages, iMaxNumSystemMessages);
}


// Input:
// iMaxNumGameMessages -> Maximum number of game messages
//
// Set the maximum number of game messages

int GameEngine::SetMaxNumGameMessages (int iMaxNumGameMessages) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxNumGameMessages, iMaxNumGameMessages);
}


// Input:
// iDefaultMaxNumSystemMessages -> Default maximum number of system messages
//
// Set the default maximum number of system messages

int GameEngine::SetDefaultMaxNumSystemMessages (int iDefaultMaxNumSystemMessages) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultMaxNumSystemMessages, iDefaultMaxNumSystemMessages);
}


// Input:
// iDefaultMaxNumGameMessages -> Default maximum number of game messages
//
// Set the default maximum number of game messages

int GameEngine::SetDefaultMaxNumGameMessages (int iDefaultMaxNumGameMessages) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultMaxNumGameMessages, iDefaultMaxNumGameMessages);
}

// Input:
// iMaxNumPersonalGameClasses -> Maximum number of personal game classes
//
// Set the maximum number of personal game classes

int GameEngine::SetMaxNumPersonalGameClasses (int iMaxNumPersonalGameClasses) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxNumPersonalGameClasses, iMaxNumPersonalGameClasses);
}


// Input:
// iShipKey -> Tech key
// pszShipName -> New default ship name
//
// Set the default name for the given tech

int GameEngine::SetDefaultShipName (int iShipKey, const char* pszShipName) {

	if (iShipKey < FIRST_SHIP || iShipKey > LAST_SHIP) {
		return ERROR_WRONG_SHIP_TYPE;
	}

	return m_pGameData->WriteData (SYSTEM_DATA, SYSTEM_DATA_SHIP_NAME_COLUMN [iShipKey], pszShipName);
}


// Input:
// iKey -> Default key
//
// Set the default background key

int GameEngine::SetDefaultBackgroundKey (int iKey) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultUIBackground, iKey);
}


// Input:
// iKey -> Default key
//
// Set the default live planet key

int GameEngine::SetDefaultLivePlanetKey (int iKey) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultUILivePlanet, iKey);
}


// Input:
// iKey -> Default key
//
// Set the default dead planet key

int GameEngine::SetDefaultDeadPlanetKey (int iKey) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultUIDeadPlanet, iKey);
}


// Input:
// iKey -> Default key
//
// Set the default button key

int GameEngine::SetDefaultButtonKey (int iKey) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultUIButtons, iKey);
}


// Input:
// iKey -> Default key
//
// Set the default separator key

int GameEngine::SetDefaultSeparatorKey (int iKey) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultUISeparator, iKey);
}


// Input:
// iKey -> Default key
//
// Set the default horz key

int GameEngine::SetDefaultHorzKey (int iKey) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultUIHorz, iKey);
}


// Input:
// iKey -> Default key
//
// Set the default vert key

int GameEngine::SetDefaultVertKey (int iKey) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultUIVert, iKey);
}


// Input:
// iKey -> Default key
//
// Set the default color key

int GameEngine::SetDefaultColorKey (int iKey) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultUIColor, iKey);
}


// Input:
// iSize -> New max size icons
//
// Set the new max icon size

int GameEngine::SetMaxIconSize (int iSize) {
	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxIconSize, iSize);
}


// Output:
// *piSize -> Max icon size
//
// Get the max icon size

int GameEngine::GetMaxIconSize (int* piSize) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxIconSize, &vTemp);

	if (iErrCode == OK) {
		*piSize = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input:
// iOption -> Bit mask of the option to be toggled
// bFlag -> True or false
//
// Set a server option

int GameEngine::SetSystemOption (int iOption, bool bFlag) {
	
	if (bFlag) {
		return m_pGameData->WriteOr (SYSTEM_DATA, SystemData::Options, iOption);
	} else {
		return m_pGameData->WriteAnd (SYSTEM_DATA, SystemData::Options, ~iOption);
	}
}

// Output:
// *pvReason -> Reason for the disabled option
//
// Get the reason for the disabled option

int GameEngine::GetLoginsDisabledReason (Variant* pvReason) {

	return m_pGameData->ReadData (SYSTEM_DATA, SystemData::LoginsDisabledReason, pvReason);
}


// Output:
// *pvReason -> Reason for the disabled option
//
// Get the reason for the disabled option

int GameEngine::GetEmpireCreationDisabledReason (Variant* pvReason) {

	return m_pGameData->ReadData (SYSTEM_DATA, SystemData::NewEmpiresDisabledReason, pvReason);
}


// Output:
// *pvReason -> Reason for the disabled option
//
// Get the reason for the disabled option

int GameEngine::GetGameCreationDisabledReason (Variant* pvReason) {

	return m_pGameData->ReadData (SYSTEM_DATA, SystemData::NewGamesDisabledReason, pvReason);
}


// Output:
// *pvReason -> Reason for the disabled option
//
// Get the reason for the disabled option

int GameEngine::GetAccessDisabledReason (Variant* pvReason) {

	return m_pGameData->ReadData (SYSTEM_DATA, SystemData::AccessDisabledReason, pvReason);
}


// Input:
// pszReason -> Reason for the disabled option
//
// Set the reason for the disabled option

int GameEngine::SetLoginsDisabledReason (const char* pszReason) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::LoginsDisabledReason, pszReason);
}


// Input:
// pszReason -> Reason for the disabled option
//
// Set the reason for the disabled option

int GameEngine::SetEmpireCreationDisabledReason (const char* pszReason) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::NewEmpiresDisabledReason, pszReason);
}


// Input:
// pszReason -> Reason for the disabled option
//
// Set the reason for the disabled option

int GameEngine::SetGameCreationDisabledReason (const char* pszReason) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::NewGamesDisabledReason, pszReason);
}


// Input:
// pszReason -> Reason for the disabled option
//
// Set the reason for the disabled option

int GameEngine::SetAccessDisabledReason (const char* pszReason) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::AccessDisabledReason, pszReason);
}


int GameEngine::GetMaxResourcesPerPlanet (int* piMaxResourcesPerPlanet) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxResourcesPerPlanet, &vValue);

	if (iErrCode == OK) {
		*piMaxResourcesPerPlanet = vValue.GetInteger();
	}

	return iErrCode;
}


int GameEngine::SetMaxResourcesPerPlanet (int iMaxResourcesPerPlanet) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxResourcesPerPlanet, iMaxResourcesPerPlanet);
}


int GameEngine::GetMaxResourcesPerPlanetPersonal (int* piMaxResourcesPerPlanet) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxResourcesPerPlanetPersonal, &vValue);

	if (iErrCode == OK) {
		*piMaxResourcesPerPlanet = vValue.GetInteger();
	}

	return iErrCode;
}


int GameEngine::SetMaxResourcesPerPlanetPersonal (int iMaxResourcesPerPlanet) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxResourcesPerPlanetPersonal, iMaxResourcesPerPlanet);
}


int GameEngine::GetMaxInitialTechLevel (float* pfMaxInitialTechLevel) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxInitialTechLevel, &vValue);

	if (iErrCode == OK) {
		*pfMaxInitialTechLevel = vValue.GetFloat();
	}

	return iErrCode;
}


int GameEngine::SetMaxInitialTechLevel (float fMaxInitialTechLevel) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxInitialTechLevel, fMaxInitialTechLevel);
}


int GameEngine::GetMaxInitialTechLevelPersonal (float* pfMaxInitialTechLevel) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxInitialTechLevelPersonal, &vValue);

	if (iErrCode == OK) {
		*pfMaxInitialTechLevel = vValue.GetFloat();
	}

	return iErrCode;
}


int GameEngine::SetMaxInitialTechLevelPersonal (float fMaxInitialTechLevel) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxInitialTechLevelPersonal, fMaxInitialTechLevel);
}


int GameEngine::GetMaxTechDev (float* pfMaxTechDev) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxTechDev, &vValue);

	if (iErrCode == OK) {
		*pfMaxTechDev = vValue.GetFloat();
	}

	return iErrCode;
}


int GameEngine::SetMaxTechDev (float fMaxTechDev) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxTechDev, fMaxTechDev);
}

int GameEngine::GetMaxTechDevPersonal (float* pfMaxTechDev) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxTechDevPersonal, &vValue);

	if (iErrCode == OK) {
		*pfMaxTechDev = vValue.GetFloat();
	}

	return iErrCode;
}


int GameEngine::SetMaxTechDevPersonal (float fMaxTechDev) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxTechDevPersonal, fMaxTechDev);
}

//

int GameEngine::GetNumUpdatesDownBeforeGameIsKilled (int* piNumUpdatesDown) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::NumUpdatesDownBeforeGameIsKilled, &vValue);

	if (iErrCode == OK) {
		*piNumUpdatesDown = vValue.GetInteger();
	}

	return iErrCode;
}

int GameEngine::SetNumUpdatesDownBeforeGameIsKilled (int iNumUpdatesDown) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::NumUpdatesDownBeforeGameIsKilled, iNumUpdatesDown);
}

int GameEngine::GetSecondsForLongtermStatus (Seconds* psSeconds) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::SecondsForLongtermStatus, &vValue);

	if (iErrCode == OK) {
		*psSeconds = (Seconds) vValue.GetInteger();
	}

	return iErrCode;
}

int GameEngine::SetSecondsForLongtermStatus (Seconds sSeconds) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::SecondsForLongtermStatus, sSeconds);
}

int GameEngine::GetAfterWeekendDelay (Seconds* psDelay) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AfterWeekendDelay, &vValue);

	if (iErrCode == OK) {
		*psDelay = vValue.GetInteger();
	}

	return iErrCode;
}

//
int GameEngine::SetAfterWeekendDelay (Seconds sDelay) {

	// Sanity
	if (sDelay >= 60 * 60 * 24 * 5) {
		return ERROR_INVALID_ARGUMENT;
	}

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::AfterWeekendDelay, sDelay);
}

int GameEngine::GetNumNukesListedInNukeHistories (int* piNumNukes) {

	Variant vValue;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::NumNukesListedInNukeHistories, &vValue);

	if (iErrCode == OK) {
		*piNumNukes = vValue.GetInteger();
	}

	return iErrCode;
}

int GameEngine::SetNumNukesListedInNukeHistories (int iNumNukes) {

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::NumNukesListedInNukeHistories, iNumNukes);
}


int GameEngine::GetMaxNumUpdatesBeforeClose (int* piMaxNumUpdates) {
	
	Variant vTemp;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::MaxNumUpdatesBeforeClose, &vTemp);
	
	if (iErrCode == OK) {
		*piMaxNumUpdates = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::SetMaxNumUpdatesBeforeClose (int iMaxNumUpdates) {

	if (iMaxNumUpdates < 0) {
		return ERROR_INVALID_ARGUMENT;
	}

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::MaxNumUpdatesBeforeClose, iMaxNumUpdates);
}

int GameEngine::GetDefaultNumUpdatesBeforeClose (int* piDefaultNumUpdates) {
	
	Variant vTemp;

	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::DefaultNumUpdatesBeforeClose, &vTemp);
	if (iErrCode == OK) {
		*piDefaultNumUpdates = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::SetDefaultNumUpdatesBeforeClose (int iDefaultNumUpdates) {

	int iErrCode, iMax;

	if (iDefaultNumUpdates < 0) {
		return ERROR_INVALID_ARGUMENT;
	}

	iErrCode = GetMaxNumUpdatesBeforeClose (&iMax);
	if (iErrCode != OK) {		return iErrCode;
	}

	if (iDefaultNumUpdates > iMax) {
		return ERROR_INVALID_ARGUMENT;
	}

	return m_pGameData->WriteData (SYSTEM_DATA, SystemData::DefaultNumUpdatesBeforeClose, iDefaultNumUpdates);
}


int GameEngine::GetSystemOptions (int* piOptions) {

	Variant vTemp;

	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::Options, &vTemp);
	if (iErrCode == OK) {
		*piOptions = vTemp.GetInteger();
	}

	return iErrCode;
}

int GameEngine::GetDefaultGameOptions (int iGameClass, GameOptions* pgoOptions) {

	int iErrCode, iOptions, iMaxNumEmpires;

	Assert (iGameClass != NO_KEY);

	bool bExists;
	iErrCode = m_pGameData->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (!bExists) {
		return ERROR_GAMECLASS_DOES_NOT_EXIST;
	}

	// iNumUpdatesBeforeGameCloses
	iErrCode = GetDefaultNumUpdatesBeforeClose (&pgoOptions->iNumUpdatesBeforeGameCloses);
	if (iErrCode != OK) {
		return iErrCode;
	}

	// iOptions
	pgoOptions->iOptions = 0;

	iErrCode = GetSystemOptions (&iOptions);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (iOptions & DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
		pgoOptions->iOptions |= GAME_WARN_ON_DUPLICATE_IP_ADDRESS;
	}

	if (iOptions & DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
		pgoOptions->iOptions |= GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS;
	}

	if (iOptions & DEFAULT_WARN_ON_DUPLICATE_SESSION_ID) {
		pgoOptions->iOptions |= GAME_WARN_ON_DUPLICATE_SESSION_ID;
	}

	if (iOptions & DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID) {
		pgoOptions->iOptions |= GAME_BLOCK_ON_DUPLICATE_SESSION_ID;
	}

	if (iOptions & DEFAULT_NAMES_LISTED) {
		pgoOptions->iOptions |= GAME_NAMES_LISTED;
	}

	if (iOptions & DEFAULT_ALLOW_SPECTATORS) {

		int iGameClassOptions;

		iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
		if (iErrCode != OK) {
			return iErrCode;
		}

		if ((iGameClassOptions & EXPOSED_SPECTATORS) == EXPOSED_SPECTATORS) {
			pgoOptions->iOptions |= GAME_ALLOW_SPECTATORS;
		}
	}

	if (iOptions & DEFAULT_BRIDIER_GAMES) {

		iErrCode = GetMaxNumEmpires (iGameClass, &iMaxNumEmpires);
		if (iErrCode != OK) {
			return iErrCode;
		}

		if (iMaxNumEmpires == 2) {
			pgoOptions->iOptions |= GAME_COUNT_FOR_BRIDIER;
		}
	}

	// Everything else
	pgoOptions->pszEnterGameMessage = NULL;
	pgoOptions->pszPassword = NULL;
	pgoOptions->sFirstUpdateDelay = 0;

	pgoOptions->pSecurity = NULL;
	pgoOptions->iNumSecurityEntries = 0;

	return OK;
}