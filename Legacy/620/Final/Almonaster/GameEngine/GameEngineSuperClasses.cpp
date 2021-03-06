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
// pszName -> Name of superclass
//
// *piKey -> Key of newly created superclass
//
// Create a new super class

int GameEngine::CreateSuperClass (const char* pszName, int* piKey) {
    
    LockSuperClasses();

    unsigned int iKey = NO_KEY;
    int iErrCode = m_pGameData->GetFirstKey (
        SYSTEM_SUPERCLASS_DATA, 
        SystemSuperClassData::Name, 
        pszName, 
        true, 
        &iKey
        );
    
    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        iErrCode = ERROR_SUPERCLASS_ALREADY_EXISTS;
        goto Cleanup;
    }

    {
        Variant pvSuperClass[] = {
            pszName,
            0
        };

        if (pvSuperClass[SystemSuperClassData::Name].GetCharPtr() == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->InsertRow (SYSTEM_SUPERCLASS_DATA, pvSuperClass, (unsigned int*) piKey);
        Assert (iErrCode == OK);
    }

Cleanup:

    UnlockSuperClasses();

    return iErrCode;
}


// Input:
// iSuperClassKey -> Key of superclass to be deleted
//
// Output:
// *pbResult -> true if superclass was deleted, false if the superclass still had game classes associated with it
//
// Delete a super class 

int GameEngine::DeleteSuperClass (int iSuperClassKey, bool* pbResult) {

    Variant vGameClasses;

    int iErrCode = m_pGameData->ReadData (
        SYSTEM_SUPERCLASS_DATA, 
        iSuperClassKey, 
        SystemSuperClassData::NumGameClasses,
        &vGameClasses
        );
    if (iErrCode != OK) {
        return ERROR_SUPERCLASS_DOES_NOT_EXIST;
    }

    if (vGameClasses.GetInteger() == 0) {
        iErrCode = m_pGameData->DeleteRow (SYSTEM_SUPERCLASS_DATA, iSuperClassKey);
        *pbResult = (iErrCode == OK);
    } else {
        *pbResult = false;
    }
    
    return iErrCode;
}


// Output:
// **ppiKey -> Keys
// **ppvName -> Names
// **piNumSuperClasses -> Actual number of superclasses
//
// Return names and keys of superclasses

int GameEngine::GetSuperClassKeys (int** ppiKey, Variant** ppvName, int* piNumSuperClasses) {

    int iErrCode = m_pGameData->ReadColumn (
        SYSTEM_SUPERCLASS_DATA, 
        SystemSuperClassData::Name, 
        (unsigned int**) ppiKey, 
        ppvName, 
        (unsigned int*) piNumSuperClasses
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Output:
// **ppiKey -> Keys
// **piNumSuperClasses -> Actual number of superclasses
//
// Return names and keys of superclasses

int GameEngine::GetSuperClassKeys (int** ppiKey, int* piNumSuperClasses) {

    int iErrCode = m_pGameData->GetAllKeys (
        SYSTEM_SUPERCLASS_DATA, 
        (unsigned int**) ppiKey, 
        (unsigned int*) piNumSuperClasses
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

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

    return m_pGameData->ReadData (SYSTEM_SUPERCLASS_DATA, iKey, SystemSuperClassData::Name, pvName);
}

int GameEngine::RenameSuperClass (int iKey, const char* pszNewName) {

    int iErrCode;
    bool bExists;

    size_t stLen = String::StrLen (pszNewName);

    if (stLen < 1 || stLen > MAX_SUPER_CLASS_NAME_LENGTH) {
        return ERROR_INVALID_ARGUMENT;
    }

    LockSuperClasses();

    iErrCode = m_pGameData->DoesRowExist (SYSTEM_SUPERCLASS_DATA, iKey, &bExists);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (!bExists) {
        iErrCode = ERROR_SUPERCLASS_DOES_NOT_EXIST;
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_SUPERCLASS_DATA, iKey, SystemSuperClassData::Name, pszNewName);
    Assert (iErrCode == OK);

Cleanup:

    UnlockSuperClasses();

    return iErrCode;
}