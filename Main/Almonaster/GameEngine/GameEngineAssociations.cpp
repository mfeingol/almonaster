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

int GameEngine::GetAssociations(unsigned int iEmpireKey, Variant** ppvAssoc, unsigned int* piNumAssoc)
{
    GET_SYSTEM_EMPIRE_ASSOCIATIONS(strAssoc, iEmpireKey);
    int iErrCode = t_pCache->ReadColumn(strAssoc, SystemEmpireAssociations::ReferenceEmpireKey, NULL, ppvAssoc, piNumAssoc);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
    }
    return iErrCode;
}

int GameEngine::CheckAssociation(unsigned int iEmpireKey, unsigned int iSwitch, bool* pbAuth)
{
    *pbAuth = false;

    unsigned int iKey;
    GET_SYSTEM_EMPIRE_ASSOCIATIONS(strAssoc, iEmpireKey);
    int iErrCode = t_pCache->GetFirstKey(strAssoc, SystemEmpireAssociations::ReferenceEmpireKey, iSwitch, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        *pbAuth = true;
    }
    return iErrCode;
}

int GameEngine::CreateAssociation(unsigned int iEmpireKey, const char* pszSecondEmpire, const char* pszPassword)
{
    // Find the second empire
    unsigned int iSecondKey;
    int iErrCode = LookupEmpireByName(pszSecondEmpire, &iSecondKey, NULL, NULL);
    RETURN_ON_ERROR(iErrCode);

    // Make sure the empire exists
    if (iSecondKey == NO_KEY)
    {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    // Make sure they're not the same empire
    if (iSecondKey == iEmpireKey)
    {
        return ERROR_DUPLICATE_EMPIRE;
    }

    // Verify the second empire's password
    Variant vPassword;
    GET_SYSTEM_EMPIRE_DATA(strSecondEmpire, iSecondKey);
    iErrCode = t_pCache->ReadData(strSecondEmpire, iSecondKey, SystemEmpireData::Password, &vPassword);
    RETURN_ON_ERROR(iErrCode);

    if (strcmp(pszPassword, vPassword.GetCharPtr()) != 0)
    {
        return ERROR_PASSWORD;
    }

    // Make sure the association doesn't already exist
    bool bExists;
    iErrCode = CheckAssociation(iEmpireKey, iSecondKey, &bExists);
    RETURN_ON_ERROR(iErrCode);

    if (bExists)
    {
        return ERROR_ASSOCIATION_ALREADY_EXISTS;
    }

    // Create the association, both ways
    const Variant pvAssoc1[SystemEmpireAssociations::NumColumns] = 
    {
        iEmpireKey,
        iSecondKey,
    };

    GET_SYSTEM_EMPIRE_ASSOCIATIONS(strAssoc1, iEmpireKey);
    iErrCode = t_pCache->InsertRow(strAssoc1, SystemEmpireAssociations::Template, pvAssoc1, NULL);
    RETURN_ON_ERROR(iErrCode);

    const Variant pvAssoc2[SystemEmpireAssociations::NumColumns] = 
    {
        iSecondKey,
        iEmpireKey,
    };

    GET_SYSTEM_EMPIRE_ASSOCIATIONS(strAssoc2, iSecondKey);
    if (!t_pCache->IsCached(strAssoc2))
    {
        iErrCode = t_pCache->CreateEmpty(SYSTEM_EMPIRE_ASSOCIATIONS, strAssoc2);
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->InsertRow(strAssoc2, SystemEmpireAssociations::Template, pvAssoc2, NULL);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::DeleteAssociation(unsigned int iEmpireKey, unsigned int iSecondEmpireKey)
{
    int iErrCode = CacheMutualAssociations(iEmpireKey, iSecondEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    unsigned int iKey;

    // Remove from first
    GET_SYSTEM_EMPIRE_ASSOCIATIONS(strAssoc1, iEmpireKey);
    iErrCode = t_pCache->GetFirstKey(strAssoc1, SystemEmpireAssociations::ReferenceEmpireKey, iSecondEmpireKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return ERROR_ASSOCIATION_NOT_FOUND;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->DeleteRow(strAssoc1, iKey);
    RETURN_ON_ERROR(iErrCode);

    // Remove from second
    GET_SYSTEM_EMPIRE_ASSOCIATIONS(strAssoc2, iSecondEmpireKey);
    iErrCode = t_pCache->GetFirstKey(strAssoc2, SystemEmpireAssociations::ReferenceEmpireKey, iEmpireKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return ERROR_ASSOCIATION_NOT_FOUND;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->DeleteRow(strAssoc2, iKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::RemoveDeadEmpireAssociations(unsigned int iEmpireKey)
{
    Variant* pvAssoc = NULL;
    AutoFreeData free(pvAssoc);

    unsigned int iNumAssoc;
    int iErrCode = GetAssociations(iEmpireKey, &pvAssoc, &iNumAssoc);
    RETURN_ON_ERROR(iErrCode);

    for (unsigned int i = 0; i < iNumAssoc; i ++)
    {
        iErrCode = DeleteAssociation(iEmpireKey, pvAssoc[i].GetInteger());
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}