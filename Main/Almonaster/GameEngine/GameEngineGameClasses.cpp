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

    ICachedTable* pGameClasses = NULL;
    ICachedTable* pTournaments = NULL;

    AutoRelease<ICachedTable> release1(pGameClasses);
    AutoRelease<ICachedTable> release2(pTournaments);

    int iOwner, iErrCode;
    Variant vOwnerName, vTournamentName, vGameClassName;

    iErrCode = t_pCache->GetTable(SYSTEM_GAMECLASS_DATA, &pGameClasses);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameClasses->ReadData(iGameClass, SystemGameClassData::Name, &vGameClassName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameClasses->ReadData(iGameClass, SystemGameClassData::Owner, &iOwner);
    RETURN_ON_ERROR(iErrCode);

    switch (iOwner) {
        
    case SYSTEM:
        strcpy(pszName, vGameClassName.GetCharPtr());
        break;

    case TOURNAMENT:

        int iTournamentKey;

        iErrCode = pGameClasses->ReadData(iGameClass, SystemGameClassData::TournamentKey, &iTournamentKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pTournaments);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = pTournaments->ReadData(iTournamentKey, SystemTournaments::Name, &vTournamentName);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = pTournaments->ReadData(iTournamentKey, SystemTournaments::Owner, &iOwner);
        RETURN_ON_ERROR(iErrCode);

        if (iOwner == SYSTEM)
        {
            sprintf(pszName, "%s [%s]", vGameClassName.GetCharPtr(), vTournamentName.GetCharPtr());
        }
        else
        {
            iErrCode = pTournaments->ReadData(iTournamentKey, SystemTournaments::OwnerName, &vOwnerName);
            RETURN_ON_ERROR(iErrCode);

            sprintf(pszName, "%s [%s (%s)]", vGameClassName.GetCharPtr(), vTournamentName.GetCharPtr(), vOwnerName.GetCharPtr());
        }

        break;

    case PERSONAL_GAME:
    case DELETED_EMPIRE_KEY:
    default:

        iErrCode = pGameClasses->ReadData(iGameClass, SystemGameClassData::OwnerName, &vOwnerName);
        RETURN_ON_ERROR(iErrCode);
        
        sprintf(pszName, "%s (%s)", vGameClassName.GetCharPtr(), vOwnerName.GetCharPtr());
        break;
    }

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
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::NumSecPerUpdate, 
        &vTemp
        );

    RETURN_ON_ERROR(iErrCode);

    *piNumSeconds = vTemp.GetInteger();
    return iErrCode;
}


int GameEngine::GetMaxNumAllies (int iGameClass, int* piMaxNumAllies) {

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MaxNumAlliances, 
        &vTemp
        );

    RETURN_ON_ERROR(iErrCode);
    
    *piMaxNumAllies = vTemp.GetInteger();
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
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::OpenGameNum, 
        &vTemp
        );

    RETURN_ON_ERROR(iErrCode);
        
    *piGameNumber = vTemp.GetInteger();
    return iErrCode;
}


// Input:
// iGameClass -> Gameclass key
//
// Output:
// *pbDeleted -> true if deleted, false if just marked for deletion
//
// Halt the given gameclass and delete it if no active games are left

int GameEngine::DeleteGameClass(int iGameClass, bool* pbDeleted) {

    *pbDeleted = false;

    // Make sure gameclass has no active games
    Variant vNumActiveGames;
    int iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumActiveGames, &vNumActiveGames);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    if (vNumActiveGames.GetInteger() != 0)
    {
        // Mark the gameclass for deletion
        iErrCode = t_pCache->WriteOr(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, GAMECLASS_MARKED_FOR_DELETION);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        // Get owner
        Variant vOwner;
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Owner, &vOwner);
        RETURN_ON_ERROR(iErrCode);

        if (vOwner.GetInteger() == SYSTEM)
        {
            unsigned int iKey;
            
            // Decrement super class counter
            Variant vSuperClassKey;
            iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::SuperClassKey, &vSuperClassKey);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->GetFirstKey(SYSTEM_SYSTEM_GAMECLASS_DATA, SystemSystemGameClassData::GameClass, iGameClass, &iKey);
            RETURN_ON_ERROR(iErrCode);

            Assert(vSuperClassKey.GetInteger() != NO_KEY && vSuperClassKey.GetInteger() != TOURNAMENT && vSuperClassKey.GetInteger() != PERSONAL_GAME);

            iErrCode = t_pCache->Increment(SYSTEM_SUPERCLASS_DATA, vSuperClassKey.GetInteger(), SystemSuperClassData::NumGameClasses, -1);
            RETURN_ON_ERROR(iErrCode);
            
            // Delete row
            iErrCode = t_pCache->DeleteRow(SYSTEM_SYSTEM_GAMECLASS_DATA, iKey);
            RETURN_ON_ERROR(iErrCode);

            *pbDeleted = true;
        }

        // Delete row from SystemGameClassData
        iErrCode = t_pCache->DeleteRow(SYSTEM_GAMECLASS_DATA, iGameClass);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

// Input:
// iGameClass -> Gameclass key
//
// Halt the given gameclass

int GameEngine::HaltGameClass(int iGameClass)
{
    int iErrCode = t_pCache->WriteOr(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, GAMECLASS_HALTED);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_GAMECLASS_DOES_NOT_EXIST;

    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}


// Input:
// iGameClass -> Gameclass key
//
// Unhalt the given gameclass

int GameEngine::UnhaltGameClass (int iGameClass)
{
    int iErrCode;

    // Is gameclass halted?
    Variant vHalted;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vHalted);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }

    if (!(vHalted.GetInteger() & GAMECLASS_HALTED))
    {
        return ERROR_GAMECLASS_NOT_HALTED;
    }

    // Mark the gameclass as no longer halted
    iErrCode = t_pCache->WriteAnd(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, ~GAMECLASS_HALTED);
    RETURN_ON_ERROR(iErrCode);

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

    bool bDynamic = (pvGameClassData[SystemGameClassData::iOptions].GetInteger() & DYNAMIC_GAMECLASS) != 0;
    int iErrCode, iSuperClass;

    unsigned int iGameClass, iTournamentKey = NO_KEY;

    Assert(iCreator != TOURNAMENT);

    // Check name length
    if (String::StrLen (pvGameClassData[SystemGameClassData::iName].GetCharPtr()) > MAX_GAME_CLASS_NAME_LENGTH)
    {
        return ERROR_NAME_IS_TOO_LONG;
    }

    // Check description length
    if (String::StrLen (pvGameClassData[SystemGameClassData::iDescription].GetCharPtr()) > MAX_THEME_DESCRIPTION_LENGTH)
    {
        return ERROR_DESCRIPTION_IS_TOO_LONG;
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
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->GetEqualKeys(
                SYSTEM_GAMECLASS_DATA,
                SystemGameClassData::TournamentKey,
                pvGameClassData[SystemGameClassData::iTournamentKey],
                NULL,
                &iNumEqual
                );
            
            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);
            
            if (iNumEqual >= (unsigned int) vLimit.GetInteger())
            {
                return ERROR_TOO_MANY_GAMECLASSES_IN_TOURNAMENT;
            }
            
        } else {

            GET_SYSTEM_EMPIRE_DATA(strEmpireData, iCreator);

            // Simple access check
            Variant vTemp;
            iErrCode = t_pCache->ReadData(strEmpireData, iCreator, SystemEmpireData::Privilege, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            
            if (bDynamic) {
                
                if (vTemp.GetInteger() < PRIVILEGE_FOR_PERSONAL_GAMES)
                {
                    return ERROR_INSUFFICIENT_PRIVILEGE;
                }
                pvGameClassData[SystemGameClassData::iOptions] = pvGameClassData[SystemGameClassData::iOptions].GetInteger() | GAMECLASS_MARKED_FOR_DELETION;
                
            } else {
                
                if (vTemp.GetInteger() < PRIVILEGE_FOR_PERSONAL_GAMECLASSES)
                {
                    return ERROR_INSUFFICIENT_PRIVILEGE;
                }
                
                // Number of gameclasses check
                unsigned int iHas;
                iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::MaxNumPersonalGameClasses, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->GetEqualKeys(
                    SYSTEM_GAMECLASS_DATA,
                    SystemGameClassData::Owner,
                    iCreator,
                    NULL,
                    &iHas
                    );
                
                if (iErrCode == ERROR_DATA_NOT_FOUND)
                {
                    iErrCode = OK;
                }
                RETURN_ON_ERROR(iErrCode);
                
                if ((int) iHas >= vTemp.GetInteger())
                {
                    return ERROR_TOO_MANY_GAMECLASSES;
                }
            }
        }
    }

    // Set num active games to zero
    pvGameClassData[SystemGameClassData::iNumActiveGames] = 0;

    // Make sure tournament exists
    iTournamentKey = pvGameClassData[SystemGameClassData::iTournamentKey].GetInteger();
    if (iTournamentKey != NO_KEY)
    {
        Variant vTemp;
        iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Icon, &vTemp);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_TOURNAMENT_DOES_NOT_EXIST;
        }
    }

    // Make sure name isn't a duplicate
    iErrCode = t_pCache->GetFirstKey(SYSTEM_GAMECLASS_DATA, SystemGameClassData::Name, pvGameClassData[SystemGameClassData::iName].GetCharPtr(), &iGameClass);
    if (iErrCode == OK)
    {
        return ERROR_GAMECLASS_ALREADY_EXISTS;
    }
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    // Check super class
    if (IS_KEY(iSuperClass))
    {
        // Increment NumGameClasses in superclass, if applicable
        iErrCode = t_pCache->Increment(SYSTEM_SUPERCLASS_DATA, iSuperClass, SystemSuperClassData::NumGameClasses, 1);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            return ERROR_GAMECLASS_HAS_NO_SUPERCLASS;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    // Insert new row into SystemGameClassData and obtain key to that row
    iErrCode = t_pCache->InsertRow(SYSTEM_GAMECLASS_DATA, SystemGameClassData::Template, pvGameClassData, &iGameClass);
    RETURN_ON_ERROR(iErrCode);

    if (pvGameClassData[SystemGameClassData::iOwner].GetInteger() == SYSTEM)
    {
        // Add row to SystemSystemGameClassData
        Variant vKey = iGameClass;
        iErrCode = t_pCache->InsertRow(SYSTEM_SYSTEM_GAMECLASS_DATA, SystemSystemGameClassData::Template, &vKey, NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    *piGameClass = iGameClass;

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

    // Is gameclass really marked for deletion?
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vOptions);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    if (!(vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION))
    {
        return ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION;
    }

    // Mark the gameclass as not halted
    iErrCode = t_pCache->WriteAnd(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, ~GAMECLASS_MARKED_FOR_DELETION        );
    RETURN_ON_ERROR(iErrCode);

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
    ICachedTable* pSystemGameClassData = NULL;
    AutoRelease<ICachedTable> release(pSystemGameClassData);

    Variant* pvData;
    AutoFreeData free(pvData);

    *ppiKey = NULL;
    *ppbHalted = NULL;
    *ppbDeleted = NULL;
    *piNumKeys = 0;

    iErrCode = t_pCache->ReadColumn(
        SYSTEM_SYSTEM_GAMECLASS_DATA, 
        SystemSystemGameClassData::GameClass, 
        NULL, 
        &pvData, 
        (unsigned int*) piNumKeys
        );
    
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }

    if (*piNumKeys > 0)
    {
        bool* pbHalted = new bool[*piNumKeys];
        Assert(pbHalted);
        Algorithm::AutoDelete<bool>(pbHalted, true);

        bool* pbDeleted = new bool[*piNumKeys];
        Assert(pbDeleted);
        Algorithm::AutoDelete<bool>(pbDeleted, true);

        int* piKey = new int[*piNumKeys];
        Assert(piKey);
        Algorithm::AutoDelete<int>(piKey, true);

        iErrCode = t_pCache->GetTable(SYSTEM_GAMECLASS_DATA, &pSystemGameClassData);
        RETURN_ON_ERROR(iErrCode);

        Variant vOptions;
        for (int i = 0; i < *piNumKeys; i ++)
        {
            iErrCode = pSystemGameClassData->ReadData(pvData[i].GetInteger(), SystemGameClassData::Options, &vOptions);
            RETURN_ON_ERROR(iErrCode);

            pbHalted[i] = (vOptions.GetInteger() & GAMECLASS_HALTED) != 0;
            pbDeleted[i] = (vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION) != 0;
            piKey[i] = pvData[i].GetInteger();
        }

        *ppbHalted = pbHalted;
        pbHalted = NULL;

        *ppbDeleted = pbDeleted;
        pbDeleted = NULL;

        *ppiKey = piKey;
        piKey = NULL;
    }

    return iErrCode;
}

// Output:
// **ppiKey -> Keys corresponding to the different gameclasses
// *piNumKeys -> Number of keys returned
//
// Returns the integer keys corresponding to each unhalted admin gameclass

int GameEngine::GetStartableSystemGameClassKeys(unsigned int** ppiKey, unsigned int* piNumKeys)
{
    int iErrCode;
    unsigned int i, iNumGameClasses;
    Variant* pvGameClassKey = NULL;
    AutoFreeData free(pvGameClassKey);

    ICachedTable* pSystemSystemGameClassData = NULL;
    ICachedTable* pSystemGameClassData = NULL;

    AutoRelease<ICachedTable> release1(pSystemSystemGameClassData);
    AutoRelease<ICachedTable> release2(pSystemGameClassData);

    *ppiKey = NULL;
    *piNumKeys = 0;

    iErrCode = t_pCache->GetTable(SYSTEM_SYSTEM_GAMECLASS_DATA, &pSystemSystemGameClassData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemSystemGameClassData->ReadColumn(
        SystemSystemGameClassData::GameClass,
        NULL,
        &pvGameClassKey,
        &iNumGameClasses
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        unsigned int* piKey = new unsigned int[iNumGameClasses];
        Assert(piKey);
        Algorithm::AutoDelete<unsigned int> del(piKey, true);

        iErrCode = t_pCache->GetTable(SYSTEM_GAMECLASS_DATA, &pSystemGameClassData);
        RETURN_ON_ERROR(iErrCode);

        *piNumKeys = 0;

        // Filter halted gameclasses
        for (i = 0; i < iNumGameClasses; i ++)
        {
            int iOptions;
            iErrCode = pSystemGameClassData->ReadData(pvGameClassKey[i].GetInteger(), SystemGameClassData::Options, &iOptions);
            RETURN_ON_ERROR(iErrCode);

            if (!(iOptions & GAMECLASS_HALTED) && !(iOptions & GAMECLASS_MARKED_FOR_DELETION))
            {
                piKey[(*piNumKeys) ++] = pvGameClassKey[i].GetInteger();
            }
        }

        *ppiKey = piKey;
        piKey = NULL;
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

int GameEngine::GetGameClassSuperClassKey (int iGameClass, unsigned int* piSuperClassKey) {

    int iErrCode = OK;

    Variant vSuperClassKey;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::SuperClassKey, &vSuperClassKey);
    RETURN_ON_ERROR(iErrCode);

    *piSuperClassKey = vSuperClassKey.GetInteger();
    
    return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
// iSuperClassKey -> Integer key of a superclass
//
// Set the key of the superclass that the gameclass belongs to

int GameEngine::SetGameClassSuperClassKey(int iGameClass, unsigned int iSuperClassKey)
{
    Variant vTemp;
    int iErrCode;

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::SuperClassKey, &vTemp);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(SYSTEM_SUPERCLASS_DATA, vTemp.GetInteger(), SystemSuperClassData::NumGameClasses, -1);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(SYSTEM_SUPERCLASS_DATA, iSuperClassKey, SystemSuperClassData::NumGameClasses, 1);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_SUPERCLASS_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->WriteData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::SuperClassKey, (int)iSuperClassKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *pvInfo -> Gameclass data
//
// Returns data corresponding to a given gameclass.

int GameEngine::GetGameClassData (int iGameClass, Variant** ppvData)
{
    return t_pCache->ReadRow (SYSTEM_GAMECLASS_DATA, iGameClass, ppvData);
}


// Input:
// iGameClass -> Integer key of a gameclass
//
// Output:
// *piOwner -> Owner of the gameclass
//
// Return the key of the owner of the given gameclass

int GameEngine::GetGameClassOwner(int iGameClass, unsigned int* piOwner)
{
    int iErrCode;
    Variant vOwner;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Owner, &vOwner);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);
    
    *piOwner = vOwner.GetInteger();
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

    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::InitialTechDevs,
        &vTech
        );
    RETURN_ON_ERROR(iErrCode);

    *piInitialTechs = vTech.GetInteger();

    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DevelopableTechDevs,
        &vTech
        );
    RETURN_ON_ERROR(iErrCode);

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
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MinNumEmpires, 
        &vNumEmpires
        );
    RETURN_ON_ERROR(iErrCode);

    *piNumEmpiresRequired = vNumEmpires.GetInteger();
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
    int iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxTechDev, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *pfMaxTechIncrease = vTemp.GetFloat();
    return iErrCode;
}


int GameEngine::GetGameClassVisibleBuilds (int iGameClass, bool* pbVisible) {

    Variant vOptions;
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options,
        &vOptions
        );
    RETURN_ON_ERROR(iErrCode);

    *pbVisible = (vOptions.GetInteger() & VISIBLE_BUILDS) != 0;
    return iErrCode;
}

int GameEngine::GetGameClassVisibleDiplomacy (int iGameClass, bool* pbVisible) {

    Variant vOptions;
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );

    RETURN_ON_ERROR(iErrCode);
    *pbVisible = (vOptions.GetInteger() & VISIBLE_DIPLOMACY) != 0;
    
    return iErrCode;
}

int GameEngine::GetMinNumSecsPerUpdateForSystemGameClass (int* piMinNumSecsPerUpdate) {

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::SystemMinSecs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMinNumSecsPerUpdate = vTemp.GetInteger();
    
    return iErrCode;
}

int GameEngine::GetMaxNumSecsPerUpdateForSystemGameClass (int* piMaxNumSecsPerUpdate) {

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::SystemMaxSecs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumSecsPerUpdate = vTemp.GetInteger();
    return iErrCode;
}

int GameEngine::GetMaxNumEmpiresForSystemGameClass (int* piMaxNumEmpires) {

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::SystemMaxNumEmpires, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumEmpires = vTemp.GetInteger();
    
    return iErrCode;
}

int GameEngine::GetMaxNumPlanetsForSystemGameClass (int* piMaxNumPlanets) {

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::SystemMaxNumPlanets, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumPlanets = vTemp.GetInteger();
    
    return iErrCode;
}

int GameEngine::GetMinNumSecsPerUpdateForPersonalGameClass (int* piMinNumSecsPerUpdate) {
    
    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::PersonalMinSecs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMinNumSecsPerUpdate = vTemp.GetInteger();

    return iErrCode;
}

int GameEngine::GetMaxNumSecsPerUpdateForPersonalGameClass (int* piMaxNumSecsPerUpdate) {
    
    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::PersonalMaxSecs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumSecsPerUpdate = vTemp.GetInteger();
    return iErrCode;
}

int GameEngine::GetMaxNumEmpiresForPersonalGameClass (int* piMaxNumEmpires) {
    
    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::PersonalMaxNumEmpires, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumEmpires = vTemp.GetInteger();
    return iErrCode;
}

int GameEngine::GetMaxNumPlanetsForPersonalGameClass (int* piMaxNumPlanets) {
    
    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::PersonalMaxNumPlanets, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumPlanets = vTemp.GetInteger();
    return iErrCode;
}

int GameEngine::SetMinNumSecsPerUpdateForSystemGameClass (int iMinNumSecsPerUpdate) {
    
    if (iMinNumSecsPerUpdate < 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::SystemMaxSecs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    if (vTemp.GetInteger() < iMinNumSecsPerUpdate) {
        return ERROR_FAILURE;
    }

    iErrCode = t_pCache->WriteData(SYSTEM_DATA, SystemData::SystemMinSecs, iMinNumSecsPerUpdate);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::SetMaxNumSecsPerUpdateForSystemGameClass (int iMaxNumSecsPerUpdate) {
    
    if (iMaxNumSecsPerUpdate < 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::SystemMinSecs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    if (vTemp.GetInteger() > iMaxNumSecsPerUpdate) {
        return ERROR_FAILURE;
    }

    iErrCode = t_pCache->WriteData(SYSTEM_DATA, SystemData::SystemMaxSecs, iMaxNumSecsPerUpdate);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::SetMaxNumEmpiresForSystemGameClass (int iMaxNumEmpires) {

    if (iMaxNumEmpires < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    return t_pCache->WriteData(SYSTEM_DATA, SystemData::SystemMaxNumEmpires, iMaxNumEmpires);
}

int GameEngine::SetMaxNumPlanetsForSystemGameClass (int iMaxNumPlanets) {

    if (iMaxNumPlanets < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    return t_pCache->WriteData(SYSTEM_DATA, SystemData::SystemMaxNumPlanets, iMaxNumPlanets);
}

int GameEngine::SetMinNumSecsPerUpdateForPersonalGameClass (int iMinNumSecsPerUpdate) {
    
    if (iMinNumSecsPerUpdate < 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::PersonalMaxSecs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    if (vTemp.GetInteger() < iMinNumSecsPerUpdate) {
        return ERROR_FAILURE;
    }
    
    return t_pCache->WriteData(SYSTEM_DATA, SystemData::PersonalMinSecs, iMinNumSecsPerUpdate);
}

int GameEngine::SetMaxNumSecsPerUpdateForPersonalGameClass (int iMaxNumSecsPerUpdate) {
    
    if (iMaxNumSecsPerUpdate < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    Variant vTemp;
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::PersonalMinSecs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    if (vTemp.GetInteger() > iMaxNumSecsPerUpdate) {
        return ERROR_FAILURE;
    }

    return t_pCache->WriteData(SYSTEM_DATA, SystemData::PersonalMaxSecs, iMaxNumSecsPerUpdate);
}

int GameEngine::SetMaxNumEmpiresForPersonalGameClass (int iMaxNumEmpires) {
    
    if (iMaxNumEmpires < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    return t_pCache->WriteData(SYSTEM_DATA, SystemData::PersonalMaxNumEmpires, iMaxNumEmpires);
}

int GameEngine::SetMaxNumPlanetsForPersonalGameClass (int iMaxNumPlanets) {
    
    if (iMaxNumPlanets < 0) {
        return ERROR_INVALID_ARGUMENT;
    }
    return t_pCache->WriteData(SYSTEM_DATA, SystemData::PersonalMaxNumPlanets, iMaxNumPlanets);
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
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vOptions
        );

    RETURN_ON_ERROR(iErrCode);

    *pbPrivateMessages = (vOptions.GetInteger() & PRIVATE_MESSAGES) != 0;
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

    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vOptions
        );

    RETURN_ON_ERROR(iErrCode);

    *pbSubjective = (vOptions.GetInteger() & SUBJECTIVE_VIEWS) != 0;
    return iErrCode;
}

int GameEngine::GetGameClassDiplomacyLevel (int iGameClass, int* piDiplomacy) {

    Variant vLevel;
    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vLevel
        );
    
    RETURN_ON_ERROR(iErrCode);
        
    *piDiplomacy = vLevel.GetInteger();
    return iErrCode;
}

int GameEngine::GetGameClassOptions (int iGameClass, int* piOptions) {

    Variant vOptions;

    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vOptions
        );

    RETURN_ON_ERROR(iErrCode);

    *piOptions = vOptions.GetInteger();
    return iErrCode;
}

int GameEngine::GetMaxNumEmpires (int iGameClass, int* piMaxNumEmpires) {

    Variant vValue;

    int iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MaxNumEmpires, 
        &vValue
        );
    
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumEmpires = vValue.GetInteger();
    return iErrCode;
}

int GameEngine::GetGameClassProperty (int iGameClass, const char* pszColumn, Variant* pvProperty)
{
    return t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, pszColumn, pvProperty);
}

int GameEngine::GetSupportedMapGenerationTypes(int iGameClass, MapGeneration* pmgMapGen) {

    int iErrCode;
    Variant vMinEmps, vMaxEmps;

    iErrCode = GetGameClassProperty(iGameClass, SystemGameClassData::MinNumEmpires, &vMinEmps);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetGameClassProperty(iGameClass, SystemGameClassData::MaxNumEmpires, &vMaxEmps);
    RETURN_ON_ERROR(iErrCode);

    Variant vMinPlanets;
    iErrCode = GetGameClassProperty(iGameClass, SystemGameClassData::MinNumPlanets, &vMinPlanets);
    RETURN_ON_ERROR(iErrCode);

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