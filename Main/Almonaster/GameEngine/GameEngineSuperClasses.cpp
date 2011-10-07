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
// pszName -> Name of superclass
//
// *piKey -> Key of newly created superclass
//
// Create a new super class

int GameEngine::CreateSuperClass (const char* pszName, int* piKey)
{
    unsigned int iKey = NO_KEY;
    int iErrCode = t_pCache->GetFirstKey(SYSTEM_SUPERCLASS_DATA, SystemSuperClassData::Name, pszName, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        return ERROR_SUPERCLASS_ALREADY_EXISTS;
    }

    Variant pvSuperClass[SystemSuperClassData::NumColumns] =
    {
        pszName,
    };
    Assert(pvSuperClass[SystemSuperClassData::iName].GetCharPtr());
        
    iErrCode = t_pCache->InsertRow(SYSTEM_SUPERCLASS_DATA, SystemSuperClassData::Template, pvSuperClass, (unsigned int*) piKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Input:
// iSuperClassKey -> Key of superclass to be deleted
//
// Output:
// *pbResult -> true if superclass was deleted, false if the superclass still had game classes associated with it
//
// Delete a super class 

int GameEngine::DeleteSuperClass(int iSuperClassKey, bool* pbResult)
{
    *pbResult = false;

    unsigned int iGameClassKey;
    int iErrCode = t_pCache->GetFirstKey(SYSTEM_GAMECLASS_DATA, SystemGameClassData::SuperClassKey, iSuperClassKey, &iGameClassKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        // No gameclasses belong to the superclass, so delete it
        iErrCode = t_pCache->DeleteRow(SYSTEM_SUPERCLASS_DATA, iSuperClassKey);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_SUPERCLASS_DOES_NOT_EXIST;
        }
        RETURN_ON_ERROR(iErrCode);
        *pbResult = true;
    }
    
    return iErrCode;
}

// Output:
// **ppiKey -> Keys
// **ppvName -> Names
// **piNumSuperClasses -> Actual number of superclasses
//
// Return names and keys of superclasses

int GameEngine::GetSuperClassKeys(unsigned int** ppiKey, Variant** ppvName, unsigned int* piNumSuperClasses)
{
    int iErrCode = t_pCache->ReadColumn(SYSTEM_SUPERCLASS_DATA, SystemSuperClassData::Name, ppiKey, ppvName, piNumSuperClasses);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Output:
// **ppiKey -> Keys
// **piNumSuperClasses -> Actual number of superclasses
//
// Return names and keys of superclasses

int GameEngine::GetSuperClassKeys(unsigned int** ppiKey, unsigned int* piNumSuperClasses)
{
    int iErrCode = t_pCache->GetAllKeys (SYSTEM_SUPERCLASS_DATA, ppiKey, piNumSuperClasses);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iKey -> Superclass key
//
// Output:
// *pvName -> Name of superclass
//
// Return name of given superclasses

int GameEngine::GetSuperClassName (int iKey, Variant* pvName) {

    return t_pCache->ReadData(SYSTEM_SUPERCLASS_DATA, iKey, SystemSuperClassData::Name, pvName);
}

int GameEngine::RenameSuperClass(int iKey, const char* pszNewName)
{
    size_t stLen = String::StrLen (pszNewName);
    if (stLen < 1 || stLen > MAX_SUPER_CLASS_NAME_LENGTH)
        return ERROR_INVALID_ARGUMENT;

    int iErrCode = t_pCache->WriteData(SYSTEM_SUPERCLASS_DATA, iKey, SystemSuperClassData::Name, pszNewName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_SUPERCLASS_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}