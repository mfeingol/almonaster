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


int GameEngine::GetSystemProperty(const char* pszColumn, Variant* pvProperty)
{
    return t_pCache->ReadData(SYSTEM_DATA, pszColumn, pvProperty);
}

int GameEngine::SetSystemProperty(const char*  pszColumn, const Variant& vProperty)
{
    return t_pCache->WriteData(SYSTEM_DATA, pszColumn, vProperty);
}


// Output:
//  *piBackground
//  *piLivePlanet
//  *piDeadPlanet
//  *piButtons
//  *piSeparator
//  *piHorz
//  *piVert
//  *piColor
//
// Return the server's default UI keys

int GameEngine::GetDefaultUIKeys(unsigned int* piBackground, int* piBackgroundAddress, unsigned int* piLivePlanet, 
                                 unsigned int* piDeadPlanet, unsigned int* piButtons, int* piButtonAddress,
                                 unsigned int* piSeparator, unsigned int* piHorz, unsigned int* piVert, unsigned int* piColor)
{
    int iErrCode;

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    iErrCode = t_pCache->GetTable(SYSTEM_DATA, &pTable);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(SystemData::DefaultUIBackground, (int*) piBackground);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(SystemData::DefaultUILivePlanet, (int*) piLivePlanet);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(SystemData::DefaultUIDeadPlanet, (int*) piDeadPlanet);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(SystemData::DefaultUIButtons, (int*) piButtons);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(SystemData::DefaultUISeparator, (int*) piSeparator);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(SystemData::DefaultUIHorz, (int*) piHorz);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(SystemData::DefaultUIVert, (int*) piVert);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(SystemData::DefaultUIColor, (int*) piColor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetThemeAddress(*piButtons, piButtonAddress);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetThemeAddress(*piBackground, piBackgroundAddress);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input
// fScore -> Points
//
// Set the Almonaster score required for a given privilege level

int GameEngine::SetScoreForPrivilege (Privilege privLevel, float fScore) {

    int iErrCode;

    Variant vTemp, vLowerScore, vHigherScore;
    const char* pszWriteColumn;

    switch (privLevel) {

    case APPRENTICE:

        vLowerScore = fScore;
        pszWriteColumn = SystemData::ApprenticeScore;
        iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::AdeptScore, &vHigherScore);
        RETURN_ON_ERROR(iErrCode);
        break;

    case ADEPT:

        vHigherScore = fScore;
        pszWriteColumn = SystemData::AdeptScore;
        iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::AdeptScore, &vLowerScore);
        RETURN_ON_ERROR(iErrCode);
        break;

    default:

        Assert(false);
        return ERROR_INVALID_ARGUMENT;
    }

    if (vLowerScore.GetFloat() > vHigherScore.GetFloat()) {
        return ERROR_INVALID_ARGUMENT;
    }

    iErrCode = t_pCache->WriteData(SYSTEM_DATA, pszWriteColumn, fScore);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = ScanEmpiresOnScoreChanges();
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Output
// *pfScore -> Points
//
// Get the Almonaster score required for a privilege level

int GameEngine::GetScoreForPrivilege (Privilege privLevel, float* pfScore) {
    
    int iErrCode;
    Variant vTemp;
    const char* pszReadColumn;

    switch (privLevel) {

    case APPRENTICE:
        pszReadColumn = SystemData::ApprenticeScore;
        break;

    case ADEPT:
        pszReadColumn = SystemData::AdeptScore;
        break;

    default:
        Assert(false);
        return ERROR_INVALID_ARGUMENT;
    }

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, pszReadColumn, &vTemp);
    RETURN_ON_ERROR(iErrCode);
        
    *pfScore = vTemp.GetFloat();
    return iErrCode;
}


//
// When adept or apprentice score changes, this function is called
//
int GameEngine::ScanEmpiresOnScoreChanges()
{
    // TODO - 609 - Rewrite ScanEmpiresOnScoreChanges
    //int iErrCode;
    //unsigned int iEmpireKey = NO_KEY;

    //while (true) {

    //    iErrCode = t_pCache->GetNextKey (SYSTEM_EMPIRE_DATA, iEmpireKey, &iEmpireKey);
    //    if (iErrCode != OK) {
    //        if (iErrCode == ERROR_DATA_NOT_FOUND) {
    //            iErrCode = OK;
    //        } else Assert(false);
    //        break;
    //    }

    //    // Best effort
    //    CalculatePrivilegeLevel (iEmpireKey);
    //}

    return OK;
}

int GameEngine::GetSystemOptions (int* piOptions) {

    Variant vTemp;

    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    
    *piOptions = vTemp.GetInteger();

    return iErrCode;
}

int GameEngine::GetDefaultGameOptions(int iGameClass, GameOptions* pgoOptions)
{
    int iErrCode;

    Assert(iGameClass != NO_KEY);

    // iNumUpdatesBeforeGameCloses
    Variant vValue;
    iErrCode = GetSystemProperty(SystemData::DefaultNumUpdatesBeforeClose, &vValue);
    RETURN_ON_ERROR(iErrCode);

    pgoOptions->iNumUpdatesBeforeGameCloses = vValue.GetInteger();

    // iOptions
    pgoOptions->iOptions = 0;

    int iOptions;
    iErrCode = GetSystemOptions(&iOptions);
    RETURN_ON_ERROR(iErrCode);

    if (iOptions & DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
        pgoOptions->iOptions |= GAME_WARN_ON_DUPLICATE_IP_ADDRESS;
    }

    if (iOptions & DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
        pgoOptions->iOptions |= GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS;
    }

    if (iOptions & DEFAULT_WARN_ON_DUPLICATE_SESSION_ID) {
        pgoOptions->iOptions |= GAME_WARN_ON_DUPLICATE_SESSION_ID;
    }

    if (iOptions & DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID) {
        pgoOptions->iOptions |= GAME_BLOCK_ON_DUPLICATE_SESSION_ID;
    }

    if (iOptions & DEFAULT_NAMES_LISTED) {
        pgoOptions->iOptions |= GAME_NAMES_LISTED;
    }

    if (iOptions & DEFAULT_RESTRICT_IDLE_EMPIRES) {
        pgoOptions->iOptions |= GAME_RESTRICT_IDLE_EMPIRES;
    }

    if (iOptions & DEFAULT_ALLOW_SPECTATORS) {

        Variant vGameClassOptions;
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vGameClassOptions);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
            return ERROR_GAMECLASS_DOES_NOT_EXIST;
        RETURN_ON_ERROR(iErrCode);
        
        if ((vGameClassOptions.GetInteger() & EXPOSED_SPECTATORS) == EXPOSED_SPECTATORS) {
            pgoOptions->iOptions |= GAME_ALLOW_SPECTATORS;
        }
    }

    if (iOptions & DEFAULT_BRIDIER_GAMES)
    {
        Variant vMaxNumEmpires;
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxNumEmpires, &vMaxNumEmpires);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
            return ERROR_GAMECLASS_DOES_NOT_EXIST;
        RETURN_ON_ERROR(iErrCode);
        
        if (vMaxNumEmpires.GetInteger() == 2)
        {
            pgoOptions->iOptions |= GAME_COUNT_FOR_BRIDIER;
        }
    }

    pgoOptions->gfoFairness = GAME_FAIRNESS_RANDOM;

    // Everything else
    pgoOptions->pszEnterGameMessage = NULL;
    pgoOptions->pszPassword = NULL;
    pgoOptions->sFirstUpdateDelay = 0;

    pgoOptions->pSecurity = NULL;
    pgoOptions->iNumSecurityEntries = 0;

    pgoOptions->iNumEmpires = 0;
    pgoOptions->piEmpireKey = NULL;
    pgoOptions->iTournamentKey = NO_KEY;
    pgoOptions->iNumPrearrangedTeams = 0;
    pgoOptions->paPrearrangedTeam = NULL;

    return OK;
}

// Input:
// iTech -> Tech key
//
// Output:
// *pvShipName -> Default ship name
//
// Return the server's default ship name for a tech

int GameEngine::GetDefaultShipName (int iTech, Variant* pvShipName) {

    if (iTech < FIRST_SHIP || iTech > LAST_SHIP) {
        return ERROR_WRONG_TECHNOLOGY;
    }

    return t_pCache->ReadData(SYSTEM_DATA, SYSTEM_DATA_SHIP_NAME_COLUMN [iTech], pvShipName);
}


// Input:
// iShipKey -> Tech key
// pszShipName -> New default ship name
//
// Set the default name for the given tech

int GameEngine::SetDefaultShipName (int iShipKey, const char* pszShipName) {

    if (iShipKey < FIRST_SHIP || iShipKey > LAST_SHIP) {
        return ERROR_WRONG_SHIP_TYPE;
    }

    return t_pCache->WriteData(SYSTEM_DATA, SYSTEM_DATA_SHIP_NAME_COLUMN [iShipKey], pszShipName);
}

int GameEngine::SetSystemOption (int iOption, bool bFlag) {
    
    int iErrCode;
    if (bFlag) {
        iErrCode = t_pCache->WriteOr(SYSTEM_DATA, SystemData::Options, iOption);
        RETURN_ON_ERROR(iErrCode);
    } else {
        iErrCode = t_pCache->WriteAnd(SYSTEM_DATA, SystemData::Options, ~iOption);
        RETURN_ON_ERROR(iErrCode);
    }
    return iErrCode;
}