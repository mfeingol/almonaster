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
// iThemeKey -> Key of theme
//
// Output:
// *pbExist -> true if theme exists, false otherwise
//
// Determine if a given theme exists

int GameEngine::DoesThemeExist(int iThemeKey, bool* pbExist)
{
    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, &vTemp);
    *pbExist = iErrCode == OK;

    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        iErrCode = OK;
    return iErrCode;
}

// Input:
// *pvData:
//
// Output:
// *piKey -> Integer key of theme
//
// Insert a theme and return its new key

int GameEngine::CreateTheme (Variant* pvData, unsigned int* piKey) {

    int iErrCode;
    unsigned int iKey;
    ICachedTable* pTable = NULL;

    *piKey = NO_KEY;

    iErrCode = t_pCache->GetTable(SYSTEM_THEMES, &pTable);
    if (iErrCode != OK) {
        Assert(false);
        goto Cleanup;
    }

    // Make sure there isn't a name collision
    iErrCode = pTable->GetFirstKey(SystemThemes::Name, pvData[SystemThemes::iName], &iKey);
    if (iErrCode == OK) {
        iErrCode = ERROR_THEME_ALREADY_EXISTS;
        goto Cleanup;
    }

    iErrCode = pTable->InsertRow(SystemThemes::Template, pvData, &iKey);
    if (iErrCode != OK) {
        Assert(false);
        goto Cleanup;
    }

    *piKey = iKey;

Cleanup:

    SafeRelease (pTable);

    return iErrCode;
}

// Output:
// **ppiNumThemeKey -> Array of keys
// *piNumKeys -> Number of keys returned
//
// Return integer keys of all system themes

int GameEngine::GetThemeKeys(unsigned int** ppiThemeKey, unsigned int* piNumKeys)
{
    int iErrCode = t_pCache->GetAllKeys(SYSTEM_THEMES, ppiThemeKey, piNumKeys);
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

int GameEngine::GetFullThemeKeys(unsigned int** ppiThemeKey, unsigned int* piNumKeys)
{
    int iErrCode = t_pCache->GetEqualKeys(SYSTEM_THEMES, SystemThemes::Options, ALL_THEME_OPTIONS, ppiThemeKey, piNumKeys);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

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

    return t_pCache->ReadRow (SYSTEM_THEMES, iThemeKey, ppvThemeData);
}


// Input:
// iThemeKey -> Theme key
//
// Output:
// *pstrThemeName -> Name of the theme
//
// Return the name of the theme

int GameEngine::GetThemeName (int iThemeKey, Variant* pvThemeName) {

    return t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::Name, pvThemeName);
}


// Input:
// iThemeKey -> Theme key
//
// Output:
// *pstrTableColor -> Name of the theme
//
// Return the table color of the theme

int GameEngine::GetThemeTableColor (int iThemeKey, Variant* pvTableColor) {

    return t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::TableColor, pvTableColor);
}


// Input:
// iThemeKey -> Theme key
//
// Output:
// *pvColor -> Color requested
//
// Return the respective color of the theme

int GameEngine::GetThemeTextColor (int iThemeKey, Variant* pvColor) {
    return t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::TextColor, pvColor);
}

int GameEngine::GetThemeGoodColor (int iThemeKey, Variant* pvColor) {
    return t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::GoodColor, pvColor);
}

int GameEngine::GetThemeBadColor (int iThemeKey, Variant* pvColor) {
    return t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::BadColor, pvColor);
}

int GameEngine::GetThemePrivateMessageColor (int iThemeKey, Variant* pvColor) {
    return t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::PrivateMessageColor, pvColor);
}

int GameEngine::GetThemeBroadcastMessageColor (int iThemeKey, Variant* pvColor) {
    return t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::BroadcastMessageColor, pvColor);
}


// Input:
// iThemeKey -> Integer key of theme
//
// Delete a theme

int GameEngine::DeleteTheme (int iThemeKey) {

    return t_pCache->DeleteRow(SYSTEM_THEMES, iThemeKey);
}


// Input:
// iThemeKey -> Key of theme
// pszThemeName -> New name of theme
//
// Set the name of the theme

int GameEngine::SetThemeName (int iThemeKey, const char* pszThemeName) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::Name, pszThemeName);
}


// Input:
// iThemeKey -> Key of theme
// pszVersion -> New name of theme
//
// Set the version of the theme

int GameEngine::SetThemeVersion (int iThemeKey, const char* pszVersion) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::Version, pszVersion);
}


// Input:
// iThemeKey -> Key of theme
// pszFileName -> New name of theme
//
// Set the filename of the theme

int GameEngine::SetThemeFileName (int iThemeKey, const char* pszFileName) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::FileName, pszFileName);
}

// Input:
// iThemeKey -> Key of theme
// pszAuthorName -> New name of theme
//
// Set the author name of the theme

int GameEngine::SetThemeAuthorName (int iThemeKey, const char* pszAuthorName) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::AuthorName, pszAuthorName);
}


// Input:
// iThemeKey -> Key of theme
// pszAuthorEmail -> New name of theme
//
// Set the author email of the theme

int GameEngine::SetThemeAuthorEmail (int iThemeKey, const char* pszAuthorEmail) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::AuthorEmail, pszAuthorEmail);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the background key of the theme

int GameEngine::SetThemeBackground (int iThemeKey, bool bExists) {

    if (bExists) {
        return t_pCache->WriteOr(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_BACKGROUND);
    }
    
    return t_pCache->WriteAnd(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_BACKGROUND);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the LivePlanet key of the theme

int GameEngine::SetThemeLivePlanet (int iThemeKey, bool bExists) {

    if (bExists) {
        return t_pCache->WriteOr(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_LIVE_PLANET);
    }
    
    return t_pCache->WriteAnd(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_LIVE_PLANET);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the theme key of the theme

int GameEngine::SetThemeDeadPlanet (int iThemeKey, bool bExists) {

    if (bExists) {
        return t_pCache->WriteOr(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_DEAD_PLANET);
    } else {
        return t_pCache->WriteAnd(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_DEAD_PLANET);
    }
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the separator key of the theme

int GameEngine::SetThemeSeparator (int iThemeKey, bool bExists) {

    if (bExists) {
        return t_pCache->WriteOr(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_SEPARATOR);
    } else {
        return t_pCache->WriteAnd(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_SEPARATOR);
    }
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the button key of the theme

int GameEngine::SetThemeButtons (int iThemeKey, bool bExists) {

    if (bExists) {
        return t_pCache->WriteOr(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_BUTTONS);
    } else {
        return t_pCache->WriteAnd(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_BUTTONS);
    }
}


// Input:
// iThemeKey -> Key of theme
// pszDescription -> New description of theme
//
// Set the description of the theme

int GameEngine::SetThemeDescription (int iThemeKey, const char* pszDescription) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::Description, pszDescription);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the horz key of the theme

int GameEngine::SetThemeHorz (int iThemeKey, bool bExists) {

    if (bExists) {
        return t_pCache->WriteOr(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_HORZ);
    } else {
        return t_pCache->WriteAnd(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_HORZ);
    }
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the horz key of the theme

int GameEngine::SetThemeVert (int iThemeKey, bool bExists) {

    if (bExists) {
        return t_pCache->WriteOr(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, THEME_VERT);
    } else {
        return t_pCache->WriteAnd(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~THEME_VERT);
    }
}


// Input:
// iThemeKey -> Key of theme
// pszColor -> New color
//
// Set the respective color of the theme

int GameEngine::SetThemeTableColor (int iThemeKey, const char* pszColor) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::TableColor, pszColor);
}

int GameEngine::SetThemeTextColor (int iThemeKey, const char* pszColor) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::TextColor, pszColor);
}

int GameEngine::SetThemeGoodColor (int iThemeKey, const char* pszColor) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::GoodColor, pszColor);
}

int GameEngine::SetThemeBadColor (int iThemeKey, const char* pszColor) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::BadColor, pszColor);
}

int GameEngine::SetThemePrivateMessageColor (int iThemeKey, const char* pszColor) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::PrivateMessageColor, pszColor);
}

int GameEngine::SetThemeBroadcastMessageColor (int iThemeKey, const char* pszColor) {
    return t_pCache->WriteData(SYSTEM_THEMES, iThemeKey, SystemThemes::BroadcastMessageColor, pszColor);
}

// Input:
// iEmpireKey -> Integer key of empire
// iThemeKey -> New key
//
// Update an empire's theme key

int GameEngine::SetEmpireThemeKey(int iEmpireKey, int iThemeKey) {

    int iErrCode;
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    if (iThemeKey != INDIVIDUAL_ELEMENTS && iThemeKey != ALTERNATIVE_PATH) {

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIButtons, 
            iThemeKey
            );
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIBackground, 
            iThemeKey
            );
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UILivePlanet, 
            iThemeKey
            );
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIDeadPlanet, 
            iThemeKey
            );
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UISeparator, 
            iThemeKey
            );
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIHorz, 
            iThemeKey
            );
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIVert, 
            iThemeKey
            );
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIColor, 
            iThemeKey
            );
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }

    iErrCode = t_pCache->WriteData(
        strEmpire, 
        iEmpireKey, 
        SystemEmpireData::AlmonasterTheme, 
        iThemeKey
        );
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    return iErrCode;
}