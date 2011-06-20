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
// iGameClass -> Integer key of a gameclass
//
// Output:
// *pstrName -> Gameclass name
//
// Return the name of a gameclass

int GameEngine::GetGameClassName (int iGameClass, char pszName [MAX_FULL_GAME_CLASS_NAME_LENGTH]) {

    IReadTable* pGameClasses = NULL;

    int iOwner, iErrCode;
    const char* pszGameClassName, * pszOwnerName;

    iErrCode = t_pConn->GetTableForReading (SYSTEM_GAMECLASS_DATA, &pGameClasses);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

#ifdef _DEBUG

    // Avoid assertion
    bool bExists;
    iErrCode = pGameClasses->DoesRowExist (iGameClass, &bExists);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!bExists) {
        iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        goto Cleanup;
    }

#endif

    iErrCode = pGameClasses->ReadData (iGameClass, SystemGameClassData::Name, &pszGameClassName);
    if (iErrCode != OK) {
        Assert (false);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY) {
            iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        }
        goto Cleanup;
    }

    iErrCode = pGameClasses->ReadData (iGameClass, SystemGameClassData::Owner, &iOwner);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    switch (iOwner) {
        
    case SYSTEM:

        strcpy (pszName, pszGameClassName);
        break;

    case TOURNAMENT:

        int iTournamentKey;
        const char* pszTournamentName;

        iErrCode = pGameClasses->ReadData (iGameClass, SystemGameClassData::TournamentKey, &iTournamentKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pGameClasses);

        iErrCode = t_pConn->GetTableForReading (SYSTEM_TOURNAMENTS, &pGameClasses);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pGameClasses->ReadData (iTournamentKey, SystemTournaments::Name, &pszTournamentName);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pGameClasses->ReadData (iTournamentKey, SystemTournaments::Owner, &iOwner);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iOwner == SYSTEM) {

            sprintf (pszName, "%s [%s]", pszGameClassName, pszTournamentName);

        } else {

            iErrCode = pGameClasses->ReadData (iTournamentKey, SystemTournaments::OwnerName, &pszOwnerName);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            sprintf (pszName, "%s [%s (%s)]", pszGameClassName, pszTournamentName, pszOwnerName);
        }

        break;

    case PERSONAL_GAME:
    case DELETED_EMPIRE_KEY:
    default:

        iErrCode = pGameClasses->ReadData (iGameClass, SystemGameClassData::OwnerName, &pszOwnerName);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        sprintf (pszName, "%s (%s)", pszGameClassName, pszOwnerName);
        break;
    }

Cleanup:

    SafeRelease (pGameClasses);

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piNumSeconds -> Seconds in update period
//
// Get the game class' update period

int GameEngine::GetGameClassUpdatePeriod (int iGameClass, Seconds* piNumSeconds) {

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::NumSecPerUpdate, 
        &vTemp
        );

    if (iErrCode == OK) {
        *piNumSeconds = vTemp.GetInteger();
    }

    return iErrCode;
}


int GameEngine::GetMaxNumAllies (int iGameClass, int* piMaxNumAllies) {

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MaxNumAlliances, 
        &vTemp
        );

    if (iErrCode == OK) {
        *piMaxNumAllies = vTemp.GetInteger();
    }

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piGameNumber -> Next game number
//
// Get the game class' next game number (no promises)

int GameEngine::GetNextGameNumber (int iGameClass, int* piGameNumber) {

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::OpenGameNum, 
        &vTemp
        );

    if (iErrCode == OK) {
        *piGameNumber = vTemp.GetInteger();
    }

    return iErrCode;

}


// Input:
// iGameClass -> Gameclass key
//
// Output:
// *pbDeleted -> true if deleted, false if just marked for deletion
//
// Halt the given gameclass and delete it if no active games are left

int GameEngine::DeleteGameClass (int iGameClass, bool* pbDeleted) {

    // Lock gameclass
    int iErrCode;
    NamedMutex nmMutex;
    iErrCode = LockGameClass (iGameClass, &nmMutex);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Check for gameclass
    bool bExists;
    iErrCode = t_pConn->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
    if (iErrCode != OK || !bExists) {
        iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Check for active games
    *pbDeleted = !DoesGameClassHaveActiveGames (iGameClass);

    if (*pbDeleted) {

        // Get owner
        Variant vOwner;
        iErrCode = t_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::Owner, 
            &vOwner
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (vOwner.GetInteger() == SYSTEM) {

            unsigned int iKey;
            
            // Decrement super class counter
            Variant vSuperClassKey;
            iErrCode = t_pConn->ReadData (
                SYSTEM_GAMECLASS_DATA, 
                iGameClass, 
                SystemGameClassData::SuperClassKey, 
                &vSuperClassKey
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = t_pConn->GetFirstKey (
                SYSTEM_SYSTEM_GAMECLASS_DATA, 
                SystemSystemGameClassData::GameClass, 
                iGameClass, 
                &iKey
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            Assert (
                vSuperClassKey.GetInteger() != NO_KEY && 
                vSuperClassKey.GetInteger() != TOURNAMENT &&
                vSuperClassKey.GetInteger() != PERSONAL_GAME);

            iErrCode = t_pConn->Increment (
                SYSTEM_SUPERCLASS_DATA, 
                vSuperClassKey.GetInteger(), 
                SystemSuperClassData::NumGameClasses, 
                -1
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            // Delete row
            iErrCode = t_pConn->DeleteRow (SYSTEM_SYSTEM_GAMECLASS_DATA, iKey);
            if (iErrCode != OK) {

                Assert (false);

                // Best effort compensate
                int iErrCode2 = t_pConn->Increment (
                    SYSTEM_SUPERCLASS_DATA, 
                    vSuperClassKey.GetInteger(), 
                    SystemSuperClassData::NumGameClasses, 
                    1
                    );
                Assert (iErrCode2 == OK);

                goto Cleanup;
            }

        }

        // Delete row from SystemGameClassData
        iErrCode = t_pConn->DeleteRow (SYSTEM_GAMECLASS_DATA, iGameClass);
        Assert (iErrCode == OK);

    } else {
        
        // Mark the gameclass for deletion
        iErrCode = t_pConn->WriteOr (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::Options, 
            GAMECLASS_MARKED_FOR_DELETION
            );
        Assert (iErrCode == OK);
    }

Cleanup:

    // Release lock
    UnlockGameClass (nmMutex);

    return iErrCode;
}

bool GameEngine::DoesGameClassHaveActiveGames (int iGameClass) {

    Variant vNumActiveGames;

    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::NumActiveGames,
        &vNumActiveGames
        );

    if (iErrCode != OK || vNumActiveGames.GetInteger() > 0) {
        return true;
    }

    return false;
}


// Input:
// iGameClass -> Gameclass key
//
// Halt the given gameclass

int GameEngine::HaltGameClass (int iGameClass) {

    int iErrCode;

    // Lock gameclass
    NamedMutex nmMutex;
    iErrCode = LockGameClass (iGameClass, &nmMutex);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Check for gameclass
    bool bExists;
    iErrCode = t_pConn->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
    if (iErrCode != OK || !bExists) {
        iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Mark the gameclass as halted
    iErrCode = t_pConn->WriteOr (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        GAMECLASS_HALTED
        );
    Assert (iErrCode == OK);

Cleanup:

    // Unlock
    UnlockGameClass (nmMutex);

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass key
//
// Unhalt the given gameclass

int GameEngine::UnhaltGameClass (int iGameClass) {

    int iErrCode;
    Variant vHalted;

    // Lock gameclass
    NamedMutex nmMutex;
    iErrCode = LockGameClass (iGameClass, &nmMutex);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Check for gameclass
    bool bExists;
    iErrCode = t_pConn->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
    if (iErrCode != OK || !bExists) {
        iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Is gameclass halted?
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vHalted
        );
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(vHalted.GetInteger() & GAMECLASS_HALTED)) {
        iErrCode = ERROR_GAMECLASS_NOT_HALTED;
        goto Cleanup;
    }

    // Mark the gameclass as no longer halted
    iErrCode = t_pConn->WriteAnd (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        ~GAMECLASS_HALTED
        );
    Assert (iErrCode == OK);

Cleanup:

    // Unlock
    UnlockGameClass (nmMutex);

    return iErrCode;
}




// Input:
// *pvGameClassData -> Row if SystemGameClassData
//
// Output:
// *piGameClass -> New gameclass key
//
// Do the following:
//  Add a row to SystemGameClassData
//  Create SystemGameInitialTechs(I) Table
//  Create SystemGameDevelopableTechs(I) Table
//  Add rows to each
//  Increment NumGameClass counter in SystemData

int GameEngine::CreateGameClass (int iCreator, Variant* pvGameClassData, int* piGameClass) {

    bool bFlag, bDynamic = (pvGameClassData[SystemGameClassData::iOptions].GetInteger() & DYNAMIC_GAMECLASS) != 0;
    int iErrCode, iSuperClass;

    unsigned int iGameClass, iTournamentKey = NO_KEY;

    NamedMutex nmTournamentLock;
    bool bTournamentLocked = false, bGameClassesLocked = false, bSuperClassesLocked = false;

    Assert (iCreator != TOURNAMENT);

    // Check name length
    if (String::StrLen (pvGameClassData[SystemGameClassData::iName].GetCharPtr()) > MAX_GAME_CLASS_NAME_LENGTH) {
        iErrCode = ERROR_NAME_IS_TOO_LONG;
        goto Cleanup;
    }

    // Check description length
    if (String::StrLen (pvGameClassData[SystemGameClassData::iDescription].GetCharPtr()) > MAX_THEME_DESCRIPTION_LENGTH) {
        iErrCode = ERROR_DESCRIPTION_IS_TOO_LONG;
        goto Cleanup;
    }

    // Make sure empire has privilege and has less than the max num of gameclasses
    if (iCreator == SYSTEM) {

        iSuperClass = pvGameClassData[SystemGameClassData::iSuperClassKey].GetInteger();

    } else {

        iSuperClass = PERSONAL_GAME;

        if (pvGameClassData [SystemGameClassData::iOwner].GetInteger() == TOURNAMENT) {

            // Check the limit on tournament gameclasses
            Variant vLimit;
            unsigned int iNumEqual;

            iErrCode = GetSystemProperty (SystemData::MaxNumGameClassesPerPersonalTournament, &vLimit);
            if (iErrCode != OK) {
                goto Cleanup;
            }

            bGameClassesLocked = true;
            LockGameClasses();

            iErrCode = t_pConn->GetEqualKeys (
                SYSTEM_GAMECLASS_DATA,
                SystemGameClassData::TournamentKey,
                pvGameClassData[SystemGameClassData::iTournamentKey],
                NULL,
                &iNumEqual
                );
            
            if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
                goto Cleanup;
            }
            
            if (iNumEqual >= (unsigned int) vLimit.GetInteger()) {
                iErrCode = ERROR_TOO_MANY_GAMECLASSES_IN_TOURNAMENT;
                goto Cleanup;
            }
            
        } else {

            // Simple access check
            Variant vTemp;
            iErrCode = t_pConn->ReadData (SYSTEM_EMPIRE_DATA, iCreator, SystemEmpireData::Privilege, &vTemp);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            if (bDynamic) {
                
                if (vTemp.GetInteger() < PRIVILEGE_FOR_PERSONAL_GAMES) {
                    return ERROR_INSUFFICIENT_PRIVILEGE;
                }
                
                pvGameClassData[SystemGameClassData::iOptions] = 
                    pvGameClassData[SystemGameClassData::iOptions].GetInteger() | GAMECLASS_MARKED_FOR_DELETION;
                
            } else {
                
                if (vTemp.GetInteger() < PRIVILEGE_FOR_PERSONAL_GAMECLASSES) {
                    iErrCode = ERROR_INSUFFICIENT_PRIVILEGE;
                    goto Cleanup;
                }
                
                // Number of gameclasses check
                unsigned int iHas;
                iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::MaxNumPersonalGameClasses, &vTemp);
                if (iErrCode != OK) {
                    goto Cleanup;
                }
                
                iErrCode = t_pConn->GetEqualKeys (
                    SYSTEM_GAMECLASS_DATA,
                    SystemGameClassData::Owner,
                    iCreator,
                    NULL,
                    &iHas
                    );
                
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    iHas = 0;
                }
                
                else if (iErrCode != OK) {
                    goto Cleanup;
                }
                
                if ((int) iHas >= vTemp.GetInteger()) {
                    iErrCode = ERROR_TOO_MANY_GAMECLASSES;
                    goto Cleanup;
                }
            }
        }
    }

    // Set num active games to zero
    pvGameClassData[SystemGameClassData::iNumActiveGames] = 0;

    // Lock
    if (!bGameClassesLocked) {
        bGameClassesLocked = true;
        LockGameClasses();
    }

    bSuperClassesLocked = true;
    LockSuperClasses();

    // Make sure tournament exists
    iTournamentKey = pvGameClassData[SystemGameClassData::iTournamentKey].GetInteger();
    if (iTournamentKey != NO_KEY) {

        iErrCode = LockTournament (iTournamentKey, &nmTournamentLock);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        bTournamentLocked = true;

        iErrCode = t_pConn->DoesRowExist (SYSTEM_TOURNAMENTS, iTournamentKey, &bFlag);
        if (iErrCode != OK || !bFlag) {
            iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
            goto Cleanup;
        }
    }

    // Make sure name isn't a duplicate
    iErrCode = t_pConn->GetFirstKey (
        SYSTEM_GAMECLASS_DATA, 
        SystemGameClassData::Name, 
        pvGameClassData[SystemGameClassData::iName].GetCharPtr(), 
        &iGameClass
        );

    if (iGameClass != NO_KEY) {
        iErrCode = ERROR_GAMECLASS_ALREADY_EXISTS;
        goto Cleanup;
    }

    // Check super class
    if (IS_KEY (iSuperClass)) {

        iErrCode = t_pConn->DoesRowExist (SYSTEM_SUPERCLASS_DATA, iSuperClass, &bFlag);
        if (iErrCode != OK || !bFlag) {
            iErrCode = ERROR_GAMECLASS_HAS_NO_SUPERCLASS;
            goto Cleanup;
        }

        // Increment NumGameClasses in superclass, if applicable
        iErrCode = t_pConn->Increment (
            SYSTEM_SUPERCLASS_DATA, 
            iSuperClass,
            SystemSuperClassData::NumGameClasses,
            1
            );

        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    // Insert new row into SystemGameClassData and obtain key to that row
    iErrCode = t_pConn->InsertRow (SYSTEM_GAMECLASS_DATA, SystemGameClassData::Template, pvGameClassData, &iGameClass);
    if (iErrCode != OK) {
        
        if (iSuperClass != NO_KEY) {
            
            // Best effort compensate
            int iErrCode2 = t_pConn->Increment (SYSTEM_SUPERCLASS_DATA, iSuperClass, SystemSuperClassData::NumGameClasses, -1);
            Assert (iErrCode2 == OK);
        }
        goto Cleanup;
    }

    if (pvGameClassData[SystemGameClassData::iOwner].GetInteger() == SYSTEM) {

        Variant vKey = iGameClass;

        // Add row to SystemSystemGameClassData
        iErrCode = t_pConn->InsertRow(SYSTEM_SYSTEM_GAMECLASS_DATA, SystemSystemGameClassData::Template, &vKey, NULL);
        if (iErrCode != OK) {

            // Best effort compensate
            int iErrCode2 = DeleteGameClass (iGameClass, &bFlag);
            Assert (iErrCode2 == OK);

            goto Cleanup;
        }
    }

    *piGameClass = iGameClass;

Cleanup:

    if (bTournamentLocked) {
        UnlockTournament (nmTournamentLock);
    }

    if (bSuperClassesLocked) {
        UnlockSuperClasses();
    }

    if (bGameClassesLocked) {
        UnlockGameClasses();
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass key
//
// returns OK if undeleted, WARNING if doesn't exist
//
// Unhalt the given gameclass

int GameEngine::UndeleteGameClass (int iGameClass) {

    int iErrCode;
    Variant vOptions;

    NamedMutex nmMutex;
    iErrCode = LockGameClass (iGameClass, &nmMutex);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Test if gameclass exists
    bool bExist;
    iErrCode = t_pConn->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExist);
    if (iErrCode != OK || !bExist) {
        iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Is gameclass really marked for deletion?
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vOptions
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION)) {
        iErrCode = ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION;
        goto Cleanup;
    }

    // Mark the gameclass as not halted
    iErrCode = t_pConn->WriteAnd (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        ~GAMECLASS_MARKED_FOR_DELETION
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    UnlockGameClass (nmMutex);

    return iErrCode;
}

// Input:
// lNumKeys -> Number of gameclasses
//
// Output:
// **ppiKey -> Keys corresponding to the different gameclasses
// **ppbHalted -> true if game has been halted, false otherwise
// *piNumKeys -> Number of keys returned
//
// Return the integer keys corresponding to each system gameclass, including those which have been halted

int GameEngine::GetSystemGameClassKeys (int** ppiKey, bool** ppbHalted, bool** ppbDeleted, int* piNumKeys) {

    int iErrCode;
    IReadTable* pSystemGameClassData = NULL;

    Variant* pvData;

    *ppiKey = NULL;
    *ppbHalted = NULL;
    *ppbDeleted = NULL;
    *piNumKeys = 0;

    iErrCode = t_pConn->ReadColumn (
        SYSTEM_SYSTEM_GAMECLASS_DATA, 
        SystemSystemGameClassData::GameClass, 
        NULL, 
        &pvData, 
        (unsigned int*) piNumKeys
        );
    
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return OK;
    }

    if (*piNumKeys > 0) {

        int iNumClasses = 0;

        *ppbHalted = new bool [*piNumKeys];
        if (*ppbHalted == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        *ppbDeleted = new bool [*piNumKeys];
        if (*ppbHalted == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        *ppiKey = new int [*piNumKeys];
        if (*ppbHalted == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        iErrCode = t_pConn->GetTableForReading (SYSTEM_GAMECLASS_DATA, &pSystemGameClassData);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        Variant vOptions;
        for (int i = 0; i < *piNumKeys; i ++) {

            iErrCode = pSystemGameClassData->ReadData (
                pvData[i].GetInteger(), 
                SystemGameClassData::Options, 
                &vOptions
                );

            if (iErrCode == OK) {

                (*ppbHalted)[iNumClasses] = (vOptions.GetInteger() & GAMECLASS_HALTED) != 0;
                (*ppbDeleted)[iNumClasses] = (vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION) != 0;
                (*ppiKey)[iNumClasses] = pvData[i].GetInteger();

                iNumClasses ++;
            }
        }

        SafeRelease (pSystemGameClassData);

        *piNumKeys = iNumClasses;
        if (iNumClasses == 0) {

            delete [] (*ppbHalted);
            *ppbHalted = NULL;

            delete [] (*ppbDeleted);
            *ppbDeleted = NULL;

            delete [] (*ppiKey);
            *ppiKey = NULL;
        }
    }

Cleanup:

    SafeRelease (pSystemGameClassData);

    if (pvData != NULL) {
        t_pConn->FreeData(pvData);
    }

    if (iErrCode != OK) {
        
        if (*ppbHalted != NULL) {
            delete [] (*ppbHalted);
            *ppbHalted = NULL;
        }

        if (*ppbDeleted != NULL) {
            delete [] (*ppbDeleted);
            *ppbDeleted = NULL;
        }

        if (*ppiKey != NULL) {
            delete [] (*ppiKey);
            *ppiKey = NULL;
        }

        *piNumKeys = 0;
    }

    return iErrCode;
}


// Output:
// **ppiKey -> Keys corresponding to the different gameclasses
// *piNumKeys -> Number of keys returned
//
// Returns the integer keys corresponding to each unhalted admin gameclass

int GameEngine::GetStartableSystemGameClassKeys (int** ppiKey, int* piNumKeys) {

    unsigned int i, iNumGameClasses;
    int* piGameClassKey = NULL, iErrCode;

    IReadTable* pReadTable = NULL;

    *ppiKey = NULL;
    *piNumKeys = 0;

    iErrCode = t_pConn->GetTableForReading (SYSTEM_SYSTEM_GAMECLASS_DATA, &pReadTable);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pReadTable->ReadColumn (
        SystemSystemGameClassData::GameClass,
        &piGameClassKey,
        &iNumGameClasses
        );

    SafeRelease (pReadTable);

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    } else {

        *ppiKey = new int [iNumGameClasses];
        if (*ppiKey == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        iErrCode = t_pConn->GetTableForReading (SYSTEM_GAMECLASS_DATA, &pReadTable);
        if (iErrCode != OK) {
            return iErrCode;
        }

        *piNumKeys = 0;

        // Filter halted gameclasses
        for (i = 0; i < iNumGameClasses; i ++) {

            int iOptions;

            iErrCode = pReadTable->ReadData (
                piGameClassKey[i],
                SystemGameClassData::Options, 
                &iOptions
                );
            
            if (iErrCode == OK &&
                !(iOptions & GAMECLASS_HALTED) &&
                !(iOptions & GAMECLASS_MARKED_FOR_DELETION)) {
                (*ppiKey)[(*piNumKeys) ++] = piGameClassKey[i];
            }
        }

        SafeRelease (pReadTable);

        t_pConn->FreeData(piGameClassKey);

        if (*piNumKeys == 0) {
            delete [] (*ppiKey);
            *ppiKey = NULL;
        }
    }

Cleanup:

    SafeRelease (pReadTable);

    if (iErrCode != OK && *ppiKey != NULL) {
        delete [] (*ppiKey);
        *ppiKey = NULL;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piSuperClassKey -> Integer key of a superclass
//
// Return the key of the superclass that the gameclass belongs to

int GameEngine::GetGameClassSuperClassKey (int iGameClass, int* piSuperClassKey) {

    int iErrCode = OK;

    Variant vSuperClassKey;
    iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::SuperClassKey, &vSuperClassKey);
    
    if (iErrCode == OK) {
        *piSuperClassKey = vSuperClassKey.GetInteger();
    }

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
// iSuperClassKey -> Integer key of a superclass
//
// Set the key of the superclass that the gameclass belongs to

int GameEngine::SetGameClassSuperClassKey (int iGameClass, int iSuperClassKey) {

    Variant vTemp;
    int iErrCode;

    LockSuperClasses();
    
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::SuperClassKey, 
        &vTemp
        );

    if (iErrCode != OK) {
        iErrCode = ERROR_GAMECLASS_DOES_NOT_EXIST;
        goto Cleanup;
    }

    iErrCode = t_pConn->Increment (
        SYSTEM_SUPERCLASS_DATA, 
        vTemp.GetInteger(), 
        SystemSuperClassData::NumGameClasses, 
        -1
        );
    if (iErrCode != OK) {
        iErrCode = ERROR_GAMECLASS_HAS_NO_SUPERCLASS;
        goto Cleanup;
    }

    iErrCode = t_pConn->Increment (
        SYSTEM_SUPERCLASS_DATA, 
        iSuperClassKey,
        SystemSuperClassData::NumGameClasses, 
        1
        );
    if (iErrCode != OK) {
        iErrCode = ERROR_SUPERCLASS_DOES_NOT_EXIST;

        int iErrCode2 = t_pConn->Increment (
            SYSTEM_SUPERCLASS_DATA, 
            vTemp.GetInteger(), 
            SystemSuperClassData::NumGameClasses, 
            1
            );
        Assert (iErrCode2 == OK);

        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::SuperClassKey, 
        iSuperClassKey
        );
    Assert (iErrCode == OK);

Cleanup:

    UnlockSuperClasses();

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *pvInfo -> Gameclass data
//
// Returns data corresponding to a given gameclass.

int GameEngine::GetGameClassData (int iGameClass, Variant** ppvData) {

    return t_pConn->ReadRow (SYSTEM_GAMECLASS_DATA, iGameClass, ppvData);
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piOwner -> Owner of the gameclass
//
// Return the key of the owner of the given gameclass

int GameEngine::GetGameClassOwner (int iGameClass, unsigned int* piOwner) {
    
    int iErrCode;
    bool bExists;

    iErrCode = t_pConn->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
    if (iErrCode != OK || !bExists) {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }

    Variant vOwner;
    iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Owner, &vOwner);
    
    if (iErrCode == OK) {
        *piOwner = vOwner.GetInteger();
    }

    return iErrCode;
}

// Input:
// iGameClass -> Integer key of gameclass
//
// Output:
// *piInitialTechs -> Bitmap of initial techs
// *piDevelopableTechs -> Bitmap of developable techs
//
// Return the bitmaps of initial developable techs

int GameEngine::GetDevelopableTechs (int iGameClass, int* piInitialTechs, int* piDevelopableTechs) {

    int iErrCode;
    Variant vTech;

    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::InitialTechDevs,
        &vTech
        );
    if (iErrCode != OK) {
        return iErrCode;
    }

    *piInitialTechs = vTech.GetInteger();

    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DevelopableTechDevs,
        &vTech
        );
    if (iErrCode != OK) {
        return iErrCode;
    }

    *piDevelopableTechs = vTech.GetInteger();

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
//
// Output:
// *piNumEmpiresRequired -> Number of empires needed for game to start
//
// Return the number of empires needed for game to start

int GameEngine::GetNumEmpiresRequiredForGameToStart (int iGameClass, int* piNumEmpiresRequired) {

    Variant vNumEmpires;
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MinNumEmpires, 
        &vNumEmpires
        );

    if (iErrCode == OK) {
        *piNumEmpiresRequired = vNumEmpires.GetInteger();
    }

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
//
// Output:
// *pfMaxTechIncrease -> Max gameclass tech increase
//
// Return max tech increase of the gameclass

int GameEngine::GetGameClassMaxTechIncrease (int iGameClass, float* pfMaxTechIncrease) {

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxTechDev, &vTemp);

    if (iErrCode == OK) {
        *pfMaxTechIncrease = vTemp.GetFloat();
    }

    return iErrCode;
}


int GameEngine::GetGameClassVisibleBuilds (int iGameClass, bool* pbVisible) {

    Variant vOptions;
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options,
        &vOptions
        );

    if (iErrCode == OK) {
        *pbVisible = (vOptions.GetInteger() & VISIBLE_BUILDS) != 0;
    }

    return iErrCode;
}

int GameEngine::GetGameClassVisibleDiplomacy (int iGameClass, bool* pbVisible) {

    Variant vOptions;
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );

    if (iErrCode == OK) {
        *pbVisible = (vOptions.GetInteger() & VISIBLE_DIPLOMACY) != 0;
    }

    return iErrCode;
}

int GameEngine::GetMinNumSecsPerUpdateForSystemGameClass (int* piMinNumSecsPerUpdate) {

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::SystemMinSecs, &vTemp);

    if (iErrCode == OK) {
        *piMinNumSecsPerUpdate = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetMaxNumSecsPerUpdateForSystemGameClass (int* piMaxNumSecsPerUpdate) {

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::SystemMaxSecs, &vTemp);

    if (iErrCode == OK) {
        *piMaxNumSecsPerUpdate = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetMaxNumEmpiresForSystemGameClass (int* piMaxNumEmpires) {

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::SystemMaxNumEmpires, &vTemp);

    if (iErrCode == OK) {
        *piMaxNumEmpires = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetMaxNumPlanetsForSystemGameClass (int* piMaxNumPlanets) {

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::SystemMaxNumPlanets, &vTemp);
    
    if (iErrCode == OK) {
        *piMaxNumPlanets = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetMinNumSecsPerUpdateForPersonalGameClass (int* piMinNumSecsPerUpdate) {
    
    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::PersonalMinSecs, &vTemp);
    
    if (iErrCode == OK) {
        *piMinNumSecsPerUpdate = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetMaxNumSecsPerUpdateForPersonalGameClass (int* piMaxNumSecsPerUpdate) {
    
    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::PersonalMaxSecs, &vTemp);
    
    if (iErrCode == OK) {
        *piMaxNumSecsPerUpdate = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetMaxNumEmpiresForPersonalGameClass (int* piMaxNumEmpires) {
    
    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::PersonalMaxNumEmpires, &vTemp);
    
    if (iErrCode == OK) {
        *piMaxNumEmpires = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetMaxNumPlanetsForPersonalGameClass (int* piMaxNumPlanets) {
    
    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::PersonalMaxNumPlanets, &vTemp);
    
    if (iErrCode == OK) {
        *piMaxNumPlanets = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::SetMinNumSecsPerUpdateForSystemGameClass (int iMinNumSecsPerUpdate) {
    
    if (iMinNumSecsPerUpdate < 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::SystemMaxSecs, &vTemp);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (vTemp.GetInteger() < iMinNumSecsPerUpdate) {
        return ERROR_FAILURE;
    }

    return t_pConn->WriteData (SYSTEM_DATA, SystemData::SystemMinSecs, iMinNumSecsPerUpdate);
}

int GameEngine::SetMaxNumSecsPerUpdateForSystemGameClass (int iMaxNumSecsPerUpdate) {
    
    if (iMaxNumSecsPerUpdate < 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::SystemMinSecs, &vTemp);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (vTemp.GetInteger() > iMaxNumSecsPerUpdate) {
        return ERROR_FAILURE;
    }

    return t_pConn->WriteData (SYSTEM_DATA, SystemData::SystemMaxSecs, iMaxNumSecsPerUpdate);
}

int GameEngine::SetMaxNumEmpiresForSystemGameClass (int iMaxNumEmpires) {

    if (iMaxNumEmpires < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    return t_pConn->WriteData (SYSTEM_DATA, SystemData::SystemMaxNumEmpires, iMaxNumEmpires);
}

int GameEngine::SetMaxNumPlanetsForSystemGameClass (int iMaxNumPlanets) {

    if (iMaxNumPlanets < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    return t_pConn->WriteData (SYSTEM_DATA, SystemData::SystemMaxNumPlanets, iMaxNumPlanets);
}

int GameEngine::SetMinNumSecsPerUpdateForPersonalGameClass (int iMinNumSecsPerUpdate) {
    
    if (iMinNumSecsPerUpdate < 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::PersonalMaxSecs, &vTemp);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (vTemp.GetInteger() < iMinNumSecsPerUpdate) {
        return ERROR_FAILURE;
    }
    
    return t_pConn->WriteData (SYSTEM_DATA, SystemData::PersonalMinSecs, iMinNumSecsPerUpdate);
}

int GameEngine::SetMaxNumSecsPerUpdateForPersonalGameClass (int iMaxNumSecsPerUpdate) {
    
    if (iMaxNumSecsPerUpdate < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    Variant vTemp;
    int iErrCode = t_pConn->ReadData (SYSTEM_DATA, SystemData::PersonalMinSecs, &vTemp);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (vTemp.GetInteger() > iMaxNumSecsPerUpdate) {
        return ERROR_FAILURE;
    }

    return t_pConn->WriteData (SYSTEM_DATA, SystemData::PersonalMaxSecs, iMaxNumSecsPerUpdate);
}

int GameEngine::SetMaxNumEmpiresForPersonalGameClass (int iMaxNumEmpires) {
    
    if (iMaxNumEmpires < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    return t_pConn->WriteData (SYSTEM_DATA, SystemData::PersonalMaxNumEmpires, iMaxNumEmpires);
}

int GameEngine::SetMaxNumPlanetsForPersonalGameClass (int iMaxNumPlanets) {
    
    if (iMaxNumPlanets < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    return t_pConn->WriteData (SYSTEM_DATA, SystemData::PersonalMaxNumPlanets, iMaxNumPlanets);
}

// Input:
// iGameClass -> Gameclass
//
// Output:
// *pbPrivateMessages -> true if yes, false if no
//
// Return whether private messages are allowed in this gameclass

int GameEngine::DoesGameClassAllowPrivateMessages (int iGameClass, bool* pbPrivateMessages) {

    Variant vOptions;
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vOptions
        );
    
    if (iErrCode == OK) {
        *pbPrivateMessages = (vOptions.GetInteger() & PRIVATE_MESSAGES) != 0;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
//
// Output:
// *pbSubjective -> true if yes, false if no
//
// Return whether subjective views are enabled in this gameclass

int GameEngine::DoesGameClassHaveSubjectiveViews (int iGameClass, bool* pbSubjective) {

    Variant vOptions;

    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vOptions
        );

    if (iErrCode == OK) {
        *pbSubjective = (vOptions.GetInteger() & SUBJECTIVE_VIEWS) != 0;
    }

    return iErrCode;
}

int GameEngine::GetGameClassDiplomacyLevel (int iGameClass, int* piDiplomacy) {

    Variant vLevel;
    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vLevel
        );
    
    if (iErrCode == OK) {
        *piDiplomacy = vLevel.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetGameClassOptions (int iGameClass, int* piOptions) {

    Variant vOptions;

    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vOptions
        );
    
    if (iErrCode == OK) {
        *piOptions = vOptions.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetMaxNumEmpires (int iGameClass, int* piMaxNumEmpires) {

    Variant vValue;

    int iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MaxNumEmpires, 
        &vValue
        );
    
    if (iErrCode == OK) {
        *piMaxNumEmpires = vValue.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetGameClassProperty (int iGameClass, const char* pszColumn, Variant* pvProperty)
{
    return t_pConn->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, pszColumn, pvProperty);
}

int GameEngine::GetSupportedMapGenerationTypes(int iGameClass, MapGeneration* pmgMapGen) {

    int iErrCode;
    Variant vMinEmps, vMaxEmps;

    iErrCode = GetGameClassProperty(iGameClass, SystemGameClassData::MinNumEmpires, &vMinEmps);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = GetGameClassProperty(iGameClass, SystemGameClassData::MaxNumEmpires, &vMaxEmps);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    Variant vMinPlanets;
    iErrCode = GetGameClassProperty(iGameClass, SystemGameClassData::MinNumPlanets, &vMinPlanets);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    GetSupportedMapGenerationTypes(vMinEmps.GetInteger(), 
                                   vMaxEmps.GetInteger(), 
                                   vMinPlanets.GetInteger(),
                                   pmgMapGen);
    return OK;
}

void GameEngine::GetSupportedMapGenerationTypes(int iMinEmps, int iMaxEmps, int iMinPlanets, 
                                                MapGeneration* pmgMapGen) {
    
    *pmgMapGen = MAPGEN_STANDARD;

    if (iMinEmps == iMaxEmps && iMinPlanets > 1)
        *pmgMapGen = (MapGeneration)(*pmgMapGen | MAPGEN_MIRRORED | MAPGEN_TWISTED);
}