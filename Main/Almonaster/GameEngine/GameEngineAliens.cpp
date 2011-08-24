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


// Output:
// *piNumAliens -> Number of aliens
//
// Return the number of alien icons currently registered as valid empire icons

int GameEngine::GetNumAliens(unsigned int* piNumAliens)
{
    return t_pCache->GetNumRows(SYSTEM_ALIEN_ICONS, piNumAliens);
}

// Output:
// **ppvData ->
// Keys
// Names
// *piNumAliens -> Number of aliens
//
// Return the system's current alien keys and author names

int GameEngine::GetAlienKeys(Variant*** pppvData, unsigned int* piNumAliens)
{
    const char* pszColumns[] =
    {
        SystemAlienIcons::AlienKey, 
        SystemAlienIcons::AuthorName
    };

    return t_pCache->ReadColumns(SYSTEM_ALIEN_ICONS, countof(pszColumns), pszColumns, NULL, pppvData, piNumAliens);
}


// Input:
// iAlienKey -> Key of new alien
// pszAuthorName -> Name of author
//
// Create a new alien icon

int GameEngine::CreateAlienIcon (int iAlienKey, const char* pszAuthorName) {

    unsigned int iKey;
    int iErrCode = t_pCache->GetFirstKey(SYSTEM_ALIEN_ICONS, SystemAlienIcons::AlienKey, iAlienKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND || iKey == NO_KEY) {

        Variant pvArray [SystemAlienIcons::NumColumns] = {
            iAlienKey,
            pszAuthorName,
        };

        iErrCode = t_pCache->InsertRow (SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvArray, &iKey);
        Assert (iErrCode == OK);

    } else {

        if (iErrCode == OK) {
            iErrCode = ERROR_ALIEN_ICON_ALREADY_EXISTS;
        }

        else Assert (false);
    }

    return iErrCode;
}


// Input:
// iAlienKey -> Key of alien icon
//
// Delete an alien icon

int GameEngine::DeleteAlienIcon (int iAlienKey) {

    unsigned int iKey;
    int iErrCode;

    Variant vDefaultAlien;

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::DefaultAlien, &vDefaultAlien);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vDefaultAlien.GetInteger() == iAlienKey) {
        iErrCode = ERROR_DEFAULT_ALIEN_ICON;
        goto Cleanup;
    }

    iErrCode = t_pCache->GetFirstKey(SYSTEM_ALIEN_ICONS, SystemAlienIcons::AlienKey, iAlienKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND || iKey == NO_KEY) {
        iErrCode = ERROR_ALIEN_ICON_DOES_NOT_EXIST;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Delete the icon!
    iErrCode = t_pCache->DeleteRow(SYSTEM_ALIEN_ICONS, iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    return iErrCode;
}

// Input:
// iEmpireKey -> Empire's integer key
// iAlienKey -> Empire's new alien icon key
//
// Return the empire's alien icon key

int GameEngine::SetEmpireAlienKey (int iEmpireKey, int iAlienKey) {

    int iErrCode = OK;

    if (iAlienKey == UPLOADED_ICON) {

        iErrCode = t_pCache->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlienKey, UPLOADED_ICON);
    
    } else {

        unsigned int iKey;
        iErrCode = t_pCache->GetFirstKey(SYSTEM_ALIEN_ICONS, SystemAlienIcons::AlienKey, iAlienKey, &iKey);
        if (iKey == NO_KEY) {

            Assert (iErrCode == ERROR_DATA_NOT_FOUND);
            iErrCode = ERROR_ALIEN_ICON_DOES_NOT_EXIST;

        } else {
        
            iErrCode = t_pCache->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlienKey, iAlienKey);
        }

    }

    return iErrCode;
}


// Input:
// iAlienKey -> Alien icon key
//
// Return the alien's author name

int GameEngine::GetAlienAuthorName (int iAlienKey, Variant* pvAuthorName) {

    int iErrCode;
    unsigned int iKey;

    iErrCode = t_pCache->GetFirstKey(SYSTEM_ALIEN_ICONS, SystemAlienIcons::AlienKey, iAlienKey, &iKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    return t_pCache->ReadData(SYSTEM_ALIEN_ICONS, iKey, SystemAlienIcons::AuthorName, pvAuthorName);
}
