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
// iThemeKey -> Key of theme
//
// Output:
// *pbExist -> true if theme exists, false otherwise
//
// Determine if a given theme exists

int GameEngine::DoesThemeExist (int iThemeKey, bool* pbExist) {
	return m_pGameData->DoesRowExist (SYSTEM_THEMES, iThemeKey, pbExist);
}

// Input:
// *psaData:
//	[0] -> Name				String
//	[1] -> AuthorName		String
//	[2] -> Version			String
//	[3] -> AuthorEmail		String
//	[4] -> Description		String
//	[5] -> FileName			String
//	[6] -> Background		Int
//	[7] -> LivePlanet		Int
//	[8] -> DeadPlanet		Int
//	[9] -> Separator		Int
//	[10] -> Buttons			Int
//	[11] -> Horz			Int
//	[12] -> Vert			Int
//	[13] -> TableColor		String
//
// Output:
// *pvKey -> Integer key of theme
//
// Insert a theme and return its new key

int GameEngine::CreateTheme (Variant* pvData, int* piKey) {

	LockThemes();

	// Make sure there isn't a name collision
	int iErrCode = m_pGameData->GetFirstKey (SYSTEM_THEMES, SystemThemes::Name, pvData[0], true, 
		(unsigned int*) piKey);

	if (iErrCode == OK) {
		*piKey = NO_KEY;
		iErrCode = ERROR_THEME_ALREADY_EXISTS;
	}
	
	else if (iErrCode == ERROR_DATA_NOT_FOUND) {
		iErrCode = m_pGameData->InsertRow (SYSTEM_THEMES, pvData, (unsigned int*) piKey);
	}

	UnlockThemes();

	return iErrCode;
}


// Output:
// *piNumThemes -> Number of themes
//
// Return the number of registered system themes

int GameEngine::GetNumThemes (int* piNumThemes) {

	return m_pGameData->GetNumRows (SYSTEM_THEMES, (unsigned int*) piNumThemes);
}


// Output:
// **ppiNumThemeKey -> Array of keys
// *piNumKeys -> Number of keys returned
//
// Return integer keys of all system themes

int GameEngine::GetThemeKeys (int** ppiThemeKey, int* piNumKeys) {

	int iErrCode = m_pGameData->GetAllKeys (
		SYSTEM_THEMES, 
		(unsigned int**) ppiThemeKey, 
		(unsigned int*) piNumKeys
		);

	if (iErrCode == ERROR_DATA_NOT_FOUND) {
		iErrCode = OK;
	}

	return iErrCode;
}


// Output:
// **ppiNumThemeKey -> Array of keys
// *piNumKeys -> Number of keys returned
//
// Return integer keys and name of all full system themes

int GameEngine::GetFullThemeKeys (int** ppiThemeKey, int* piNumKeys) {

	unsigned int i, iNumKeys, * piProxyKey;

	*ppiThemeKey = NULL;
	*piNumKeys = 0;

	IReadTable* pThemes;
	int iOptions, iErrCode = m_pGameData->GetTableForReading (SYSTEM_THEMES, &pThemes);
	if (iErrCode != OK) {
		return iErrCode;
	}

	iErrCode = pThemes->GetAllKeys (&piProxyKey, &iNumKeys);

	if (iErrCode == OK) {

		*ppiThemeKey = new int [iNumKeys];
		if (*ppiThemeKey == NULL) {
			iErrCode = ERROR_OUT_OF_MEMORY;
			goto Cleanup;
		}

		for (i = 0; i < iNumKeys; i ++) {

			iErrCode = pThemes->ReadData (piProxyKey[i], SystemThemes::Options, &iOptions);
			
			if (iErrCode == OK) {
				
				if (iOptions == ALL_THEME_OPTIONS) {				
					(*ppiThemeKey)[(*piNumKeys) ++] = piProxyKey[i];
				}

			} else {

				Assert (false);
				iErrCode = OK;
			}
		}

		m_pGameData->FreeKeys (piProxyKey);
	}

	else if (iErrCode == ERROR_DATA_NOT_FOUND) {
		iErrCode = OK;
	}

Cleanup:

	pThemes->Release();

	return iErrCode;
}


// Input:
// iThemeKey -> Theme key
//
// Output:
// *pvThemeData -> SystemThemes row
//
// Return a theme's data

int GameEngine::GetThemeData (int iThemeKey, Variant** ppvThemeData) {

	return m_pGameData->ReadRow (SYSTEM_THEMES, iThemeKey, ppvThemeData);
}


// Input:
// iThemeKey -> Theme key
//
// Output:
// *pstrThemeName -> Name of the theme
//
// Return the name of the theme

int GameEngine::GetThemeName (int iThemeKey, Variant* pvThemeName) {

	return m_pGameData->ReadData (SYSTEM_THEMES, iThemeKey, SystemThemes::Name, pvThemeName);
}


// Input:
// iThemeKey -> Theme key
//
// Output:
// *pstrTableColor -> Name of the theme
//
// Return the table color of the theme

int GameEngine::GetThemeTableColor (int iThemeKey, Variant* pvTableColor) {

	return m_pGameData->ReadData (SYSTEM_THEMES, iThemeKey, SystemThemes::TableColor, pvTableColor);
}


// Input:
// iThemeKey -> Theme key
//
// Output:
// *pvColor -> Color requested
//
// Return the respective color of the theme

int GameEngine::GetThemeTextColor (int iThemeKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_THEMES, iThemeKey, SystemThemes::TextColor, pvColor);
}

int GameEngine::GetThemeGoodColor (int iThemeKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_THEMES, iThemeKey, SystemThemes::GoodColor, pvColor);
}

int GameEngine::GetThemeBadColor (int iThemeKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_THEMES, iThemeKey, SystemThemes::BadColor, pvColor);
}

int GameEngine::GetThemePrivateMessageColor (int iThemeKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_THEMES, iThemeKey, SystemThemes::PrivateMessageColor, pvColor);
}

int GameEngine::GetThemeBroadcastMessageColor (int iThemeKey, Variant* pvColor) {
	return m_pGameData->ReadData (SYSTEM_THEMES, iThemeKey, SystemThemes::BroadcastMessageColor, pvColor);
}


// Input:
// iThemeKey -> Integer key of theme
//
// Delete a theme

int GameEngine::DeleteTheme (int iThemeKey) {

	LockThemes();

	int iErrCode = m_pGameData->DeleteRow (SYSTEM_THEMES, iThemeKey);

	// TODO: move people using the theme to something else?

	UnlockThemes();

	return iErrCode;
}

// Output:
// *piBackgroundKey -> Default background key
//
// Return the system's default background key

int GameEngine::GetDefaultBackgroundKey (int* piBackgroundKey) {

	Variant vBackgroundKey;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::DefaultUIBackground, &vBackgroundKey);
	
	if (iErrCode == OK) {
		*piBackgroundKey = vBackgroundKey.GetInteger();
	}

	return iErrCode;
}


// Output:
// *piSeparatorKey -> Default separator key
//
// Return the system's default separator key

int GameEngine::GetDefaultSeparatorKey (int* piSeparatorKey) {

	Variant vSeparatorKey;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::DefaultUISeparator, &vSeparatorKey);
	
	if (iErrCode == OK) {
		*piSeparatorKey = vSeparatorKey.GetInteger();
	}

	return iErrCode;
}


// Output:
// *piButtonKey -> Default button key
//
// Return the system's default button key

int GameEngine::GetDefaultButtonKey (int* piButtonKey) {

	Variant vButtonKey;
	int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::DefaultUIButtons, &vButtonKey);

	if (iErrCode == OK) {
		*piButtonKey = vButtonKey.GetInteger();
	}

	return iErrCode;
}


// Input:
// iThemeKey -> Key of theme
// pszThemeName -> New name of theme
//
// Set the name of the theme

int GameEngine::SetThemeName (int iThemeKey, const char* pszThemeName) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::Name, pszThemeName);
}


// Input:
// iThemeKey -> Key of theme
// pszVersion -> New name of theme
//
// Set the version of the theme

int GameEngine::SetThemeVersion (int iThemeKey, const char* pszVersion) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::Version, pszVersion);
}


// Input:
// iThemeKey -> Key of theme
// pszFileName -> New name of theme
//
// Set the filename of the theme

int GameEngine::SetThemeFileName (int iThemeKey, const char* pszFileName) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::FileName, pszFileName);
}

// Input:
// iThemeKey -> Key of theme
// pszAuthorName -> New name of theme
//
// Set the author name of the theme

int GameEngine::SetThemeAuthorName (int iThemeKey, const char* pszAuthorName) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::AuthorName, pszAuthorName);
}


// Input:
// iThemeKey -> Key of theme
// pszAuthorEmail -> New name of theme
//
// Set the author email of the theme

int GameEngine::SetThemeAuthorEmail (int iThemeKey, const char* pszAuthorEmail) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::AuthorEmail, pszAuthorEmail);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the background key of the theme

int GameEngine::SetThemeBackground (int iThemeKey, bool bExists) {

	if (bExists) {
		return m_pGameData->WriteOr (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_BACKGROUND);
	}
	
	return m_pGameData->WriteAnd (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_BACKGROUND);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the LivePlanet key of the theme

int GameEngine::SetThemeLivePlanet (int iThemeKey, bool bExists) {

	if (bExists) {
		return m_pGameData->WriteOr (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_LIVE_PLANET);
	}
	
	return m_pGameData->WriteAnd (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_LIVE_PLANET);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the theme key of the theme

int GameEngine::SetThemeDeadPlanet (int iThemeKey, bool bExists) {

	if (bExists) {
		return m_pGameData->WriteOr (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_DEAD_PLANET);
	} else {
		return m_pGameData->WriteAnd (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_DEAD_PLANET);
	}
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the separator key of the theme

int GameEngine::SetThemeSeparator (int iThemeKey, bool bExists) {

	if (bExists) {
		return m_pGameData->WriteOr (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_SEPARATOR);
	} else {
		return m_pGameData->WriteAnd (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_SEPARATOR);
	}
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the button key of the theme

int GameEngine::SetThemeButtons (int iThemeKey, bool bExists) {

	if (bExists) {
		return m_pGameData->WriteOr (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_BUTTONS);
	} else {
		return m_pGameData->WriteAnd (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_BUTTONS);
	}
}


// Input:
// iThemeKey -> Key of theme
// pszDescription -> New description of theme
//
// Set the description of the theme

int GameEngine::SetThemeDescription (int iThemeKey, const char* pszDescription) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::Description, pszDescription);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the horz key of the theme

int GameEngine::SetThemeHorz (int iThemeKey, bool bExists) {

	if (bExists) {
		return m_pGameData->WriteOr (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_HORZ);
	} else {
		return m_pGameData->WriteAnd (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_HORZ);
	}
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the horz key of the theme

int GameEngine::SetThemeVert (int iThemeKey, bool bExists) {

	if (bExists) {
		return m_pGameData->WriteOr (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_VERT);
	} else {
		return m_pGameData->WriteAnd (SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_VERT);
	}
}


// Input:
// iThemeKey -> Key of theme
// pszColor -> New color
//
// Set the respective color of the theme

int GameEngine::SetThemeTableColor (int iThemeKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::TableColor, pszColor);
}

int GameEngine::SetThemeTextColor (int iThemeKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::TextColor, pszColor);
}

int GameEngine::SetThemeGoodColor (int iThemeKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::GoodColor, pszColor);
}

int GameEngine::SetThemeBadColor (int iThemeKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::BadColor, pszColor);
}

int GameEngine::SetThemePrivateMessageColor (int iThemeKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::PrivateMessageColor, pszColor);
}

int GameEngine::SetThemeBroadcastMessageColor (int iThemeKey, const char* pszColor) {
	return m_pGameData->WriteData (SYSTEM_THEMES, iThemeKey, SystemThemes::BroadcastMessageColor, pszColor);
}