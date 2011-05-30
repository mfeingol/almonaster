//
// GameEngine.dll:  a component of Almonaster
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

int GameEngine::GetNumAliens (int* piNumAliens) {

    return m_pGameData->GetNumRows (SYSTEM_ALIEN_ICONS, (unsigned int*) piNumAliens);
}

// Output:
// **ppvData ->
// Keys
// Names
// *piNumAliens -> Number of aliens
//
// Return the system's current alien keys and author names

int GameEngine::GetAlienKeys (Variant*** pppvData, int* piNumAliens) {

    const unsigned int piColumns[] = {
        SystemAlienIcons::AlienKey, 
        SystemAlienIcons::AuthorName
    };

    return m_pGameData->ReadColumns (
        SYSTEM_ALIEN_ICONS, 
        countof (piColumns),
        piColumns, 
        pppvData, 
        (unsigned int*) piNumAliens
        );
}


// Input:
// iAlienKey -> Key of new alien
// pszAuthorName -> Name of author
//
// Create a new alien icon

int GameEngine::CreateAlienIcon (int iAlienKey, const char* pszAuthorName) {

    LockAlienIcons();

    unsigned int iKey;
    int iErrCode = m_pGameData->GetFirstKey (
        SYSTEM_ALIEN_ICONS, 
        SystemAlienIcons::AlienKey, 
        iAlienKey, 
        false, 
        &iKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND || iKey == NO_KEY) {

        Variant pvArray [SystemAlienIcons::NumColumns] = {
            iAlienKey,
            pszAuthorName,
        };

        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvArray, &iKey);
        Assert (iErrCode == OK);

    } else {

        if (iErrCode == OK) {
            iErrCode = ERROR_ALIEN_ICON_ALREADY_EXISTS;
        }

        else Assert (false);
    }

    UnlockAlienIcons();

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

    LockAlienIcons();

    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::DefaultAlien, &vDefaultAlien);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vDefaultAlien.GetInteger() == iAlienKey) {
        iErrCode = ERROR_DEFAULT_ALIEN_ICON;
        goto Cleanup;
    }

    iErrCode = m_pGameData->GetFirstKey (SYSTEM_ALIEN_ICONS, SystemAlienIcons::AlienKey, iAlienKey, false, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND || iKey == NO_KEY) {
        iErrCode = ERROR_ALIEN_ICON_DOES_NOT_EXIST;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Delete the icon!
    iErrCode = m_pGameData->DeleteRow (SYSTEM_ALIEN_ICONS, iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    UnlockAlienIcons();

    return iErrCode;
}

// Input:
// iEmpireKey -> Empire's integer key
// iAlienKey -> Empire's new alien icon key
//
// Return the empire's alien icon key

int GameEngine::SetEmpireAlienKey (int iEmpireKey, int iAlienKey) {

    int iErrCode = OK;

    LockAlienIcons();

    if (iAlienKey == UPLOADED_ICON) {

        iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlienKey, UPLOADED_ICON);
    
    } else {

        unsigned int iKey;
        iErrCode = m_pGameData->GetFirstKey (
            SYSTEM_ALIEN_ICONS, 
            SystemAlienIcons::AlienKey, 
            iAlienKey, 
            false, 
            &iKey
            );

        if (iKey == NO_KEY) {

            Assert (iErrCode == ERROR_DATA_NOT_FOUND);
            iErrCode = ERROR_ALIEN_ICON_DOES_NOT_EXIST;

        } else {
        
            iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlienKey, iAlienKey);
        }

    }

    UnlockAlienIcons();

    return iErrCode;
}


// Input:
// iAlienKey -> Alien icon key
//
// Return the alien's author name

int GameEngine::GetAlienAuthorName (int iAlienKey, Variant* pvAuthorName) {

    int iErrCode;
    unsigned int iKey;

    iErrCode = m_pGameData->GetFirstKey (SYSTEM_ALIEN_ICONS, SystemAlienIcons::AlienKey, iAlienKey, false, &iKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    return m_pGameData->ReadData (SYSTEM_ALIEN_ICONS, iKey, SystemAlienIcons::AuthorName, pvAuthorName);
}
