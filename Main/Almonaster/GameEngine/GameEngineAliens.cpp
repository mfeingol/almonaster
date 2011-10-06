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
    return t_pCache->GetNumCachedRows(SYSTEM_ALIEN_ICONS, piNumAliens);
}

// Output:
// **ppvData ->
// Keys
// Names
// *piNumAliens -> Number of aliens
//
// Return the system's current alien keys and author names

int GameEngine::GetAliens(Variant*** pppvData, unsigned int** ppiKey, unsigned int* piNumAliens)
{
    int iErrCode = t_pCache->ReadColumns(SYSTEM_ALIEN_ICONS, SystemAlienIcons::NumColumns, SystemAlienIcons::ColumnNames, ppiKey, pppvData, piNumAliens);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

// Input:
// iAlienKey -> Key of new alien
// pszAuthorName -> Name of author
//
// Create a new alien icon

int GameEngine::CreateAlienIcon(const char* pszAuthorName, int* piAddress, unsigned int* piKey)
{
    *piAddress = 0;
    *piKey = NO_KEY;

    // Pick an address
    Variant* pvAddress = NULL;
    AutoFreeData free_pvAddress(pvAddress);

    unsigned int iNumIcons;
    int iErrCode = t_pCache->ReadColumn(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Address, NULL, &pvAddress, &iNumIcons);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    int iNewAddress = 1;
    for (unsigned int i = 0; i < iNumIcons; i ++)
    {
        if (iNewAddress <= pvAddress[i].GetInteger())
        {
            iNewAddress = pvAddress[i].GetInteger() + 1;
        }
    }

    Variant pvArray[SystemAlienIcons::NumColumns] = 
    {
        iNewAddress,
        pszAuthorName,
    };

    unsigned int iKey;
    iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvArray, &iKey);
    RETURN_ON_ERROR(iErrCode);

    *piAddress = iNewAddress;
    *piKey = iKey;
    
    return iErrCode;
}

// Input:
// iAlienKey -> Key of alien icon
//
// Delete an alien icon

int GameEngine::DeleteAlienIcon(unsigned int iKey)
{
    int iErrCode;

    Variant vDefaultAlienKey;
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::DefaultAlienKey, &vDefaultAlienKey);
    RETURN_ON_ERROR(iErrCode);

    if (vDefaultAlienKey.GetInteger() == (int)iKey)
    {
        return ERROR_DEFAULT_ALIEN_ICON;
    }

    unsigned int iNumAliens;
    iErrCode = GetNumAliens(&iNumAliens);
    if (iNumAliens == 1)
    {
        return ERROR_LAST_ALIEN_ICON;
    }

    iErrCode = t_pCache->DeleteRow(SYSTEM_ALIEN_ICONS, iKey);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_ALIEN_ICON_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Input:
// iEmpireKey -> Empire's integer key
// iAlienKey -> Empire's new alien icon key
//
// Return the empire's alien icon key

int GameEngine::SetEmpireAlienIcon(int iEmpireKey, unsigned int iKey, int* piAddress)
{
    int iErrCode;
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    *piAddress = -1;
    if (iKey != UPLOADED_ICON)
    {
        Variant vAddress;
        iErrCode = t_pCache->ReadData(SYSTEM_ALIEN_ICONS, iKey, SystemAlienIcons::Address, &vAddress);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_ALIEN_ICON_DOES_NOT_EXIST;
        }
        RETURN_ON_ERROR(iErrCode);
        *piAddress = vAddress.GetInteger();
    }

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::AlienKey, (int)iKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::AlienAddress, *piAddress);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Input:
// iAlienKey -> Alien icon key
//
// Return the alien's author name

int GameEngine::GetAlienData(unsigned int iKey, Variant** ppvData)
{
    int iErrCode = t_pCache->ReadRow(SYSTEM_ALIEN_ICONS, iKey, ppvData);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_ALIEN_ICON_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::GetAlienIconAddress(unsigned int iKey, int* piAddress)
{
    int iErrCode;

    Variant vAddress;
    iErrCode = t_pCache->ReadData(SYSTEM_ALIEN_ICONS, iKey, SystemAlienIcons::Address, &vAddress);
    RETURN_ON_ERROR(iErrCode);

    *piAddress = vAddress.GetInteger();
    return iErrCode;
}