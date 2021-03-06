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
    *pbExist = false;

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, &vTemp);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return OK;
    RETURN_ON_ERROR(iErrCode);

    *pbExist = true;
    return iErrCode;
}

// Input:
// *pvData:
//
// Output:
// *piKey -> Integer key of theme
//
// Insert a theme and return its new key

int GameEngine::CreateTheme(Variant* pvData, unsigned int* piKey, int* piAddress)
{
    int iErrCode;
    unsigned int iKey;

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    *piKey = NO_KEY;

    iErrCode = t_pCache->GetTable(SYSTEM_THEMES, &pTable);
    RETURN_ON_ERROR(iErrCode);

    // Make sure there isn't a name collision
    iErrCode = pTable->GetFirstKey(SystemThemes::Name, pvData[SystemThemes::iName], &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        return ERROR_THEME_ALREADY_EXISTS;
    }

    // Pick an address
    Variant* pvAddress = NULL;
    AutoFreeData free_pvAddress(pvAddress);

    unsigned int iNumThemes;
    iErrCode = t_pCache->ReadColumn(SYSTEM_THEMES, SystemThemes::Address, NULL, &pvAddress, &iNumThemes);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    int iNewAddress = FIRST_ADDRESS;
    for (unsigned int i = 0; i < iNumThemes; i ++)
    {
        if (iNewAddress <= pvAddress[i].GetInteger())
        {
            iNewAddress = pvAddress[i].GetInteger() + 1;
        }
    }

    pvData[SystemThemes::iAddress] = *piAddress = iNewAddress;

    iErrCode = pTable->InsertRow(SystemThemes::Template, pvData, &iKey);
    RETURN_ON_ERROR(iErrCode);

    *piKey = iKey;

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
    RETURN_ON_ERROR(iErrCode);

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
    RETURN_ON_ERROR(iErrCode);

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

int GameEngine::GetThemeAddress(int iThemeKey, int* piAddress)
{
    int iErrCode = OK;

    switch (iThemeKey)
    {
    case NULL_THEME:
    case ALTERNATIVE_PATH:
        *piAddress = -1;
        break;
    default:
        Variant vAddress;
        iErrCode = t_pCache->ReadData(SYSTEM_THEMES, iThemeKey, SystemThemes::Address, &vAddress);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_THEME_DOES_NOT_EXIST;
        }
        RETURN_ON_ERROR(iErrCode);
        *piAddress = vAddress;
        break;
    }
    return iErrCode;
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

int SetThemeOption(int iThemeKey, int iOption, bool bSet)
{

    int iErrCode;
    if (bSet)
    {
        iErrCode = t_pCache->WriteOr(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, iOption);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = t_pCache->WriteAnd(SYSTEM_THEMES, iThemeKey, SystemThemes::Options, ~iOption);
        RETURN_ON_ERROR(iErrCode);
    }
    return iErrCode;
}

int GameEngine::SetThemeBackground (int iThemeKey, bool bExists)
{
    return SetThemeOption(iThemeKey, THEME_BACKGROUND, bExists);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the LivePlanet key of the theme

int GameEngine::SetThemeLivePlanet (int iThemeKey, bool bExists)
{
    return SetThemeOption(iThemeKey, THEME_LIVE_PLANET, bExists);
}

// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the theme key of the theme

int GameEngine::SetThemeDeadPlanet (int iThemeKey, bool bExists)
{
    return SetThemeOption(iThemeKey, THEME_DEAD_PLANET, bExists);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the separator key of the theme

int GameEngine::SetThemeSeparator (int iThemeKey, bool bExists)
{
    return SetThemeOption(iThemeKey, THEME_SEPARATOR, bExists);
}

// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the button key of the theme

int GameEngine::SetThemeButtons (int iThemeKey, bool bExists)
{
    return SetThemeOption(iThemeKey, THEME_BUTTONS, bExists);
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

int GameEngine::SetThemeHorz (int iThemeKey, bool bExists)
{
    return SetThemeOption(iThemeKey, THEME_HORZ, bExists);
}


// Input:
// iThemeKey -> Key of theme
// bExists -> Feature exists
//
// Set the horz key of the theme

int GameEngine::SetThemeVert (int iThemeKey, bool bExists)
{
    return SetThemeOption(iThemeKey, THEME_VERT, bExists);
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
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIBackground, 
            iThemeKey
            );
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UILivePlanet, 
            iThemeKey
            );
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIDeadPlanet, 
            iThemeKey
            );
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UISeparator, 
            iThemeKey
            );
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIHorz, 
            iThemeKey
            );
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIVert, 
            iThemeKey
            );
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(
            strEmpire, 
            iEmpireKey, 
            SystemEmpireData::UIColor, 
            iThemeKey
            );
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->WriteData(
        strEmpire, 
        iEmpireKey, 
        SystemEmpireData::AlmonasterTheme, 
        iThemeKey
        );
   RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}