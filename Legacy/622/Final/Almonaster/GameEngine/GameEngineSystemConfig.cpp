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


int GameEngine::GetSystemProperty (int iColumn, Variant* pvProperty) {

    return m_pGameData->ReadData (SYSTEM_DATA, iColumn, pvProperty);
}

int GameEngine::SetSystemProperty (int iColumn, const Variant& vProperty) {

    return m_pGameData->WriteData (SYSTEM_DATA, iColumn, vProperty);
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

int GameEngine::GetDefaultUIKeys (unsigned int* piBackground, unsigned int* piLivePlanet, 
                                  unsigned int* piDeadPlanet, unsigned int* piButtons,
                                  unsigned int* piSeparator, unsigned int* piHorz, unsigned int* piVert, 
                                  unsigned int* piColor) {

    int iErrCode;
    IReadTable* pTable = NULL;

    iErrCode = m_pGameData->GetTableForReading (SYSTEM_DATA, &pTable);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIBackground, (int*) piBackground);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUILivePlanet, (int*) piLivePlanet);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIDeadPlanet, (int*) piDeadPlanet);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIButtons, (int*) piButtons);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUISeparator, (int*) piSeparator);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIHorz, (int*) piHorz);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIVert, (int*) piVert);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIColor, (int*) piColor);
    if (iErrCode != OK) {
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pTable);

    return iErrCode;
}


// Input
// fScore -> Points
//
// Set the Almonaster score required for a given privilege level

int GameEngine::SetScoreForPrivilege (Privilege privLevel, float fScore) {

    int iErrCode;

    Variant vTemp, vLowerScore, vHigherScore;
    unsigned int iWriteColumn;

    switch (privLevel) {

    case APPRENTICE:

        vLowerScore = fScore;
        iWriteColumn = SystemData::ApprenticeScore;
        iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AdeptScore, &vHigherScore);
        break;

    case ADEPT:

        vHigherScore = fScore;
        iWriteColumn = SystemData::AdeptScore;
        iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AdeptScore, &vLowerScore);
        break;

    default:

        Assert (false);
        return ERROR_INVALID_ARGUMENT;
    }

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (vLowerScore.GetFloat() > vHigherScore.GetFloat()) {
        return ERROR_INVALID_ARGUMENT;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_DATA, iWriteColumn, fScore);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return ScanEmpiresOnScoreChanges();
}


// Output
// *pfScore -> Points
//
// Get the Almonaster score required for a privilege level

int GameEngine::GetScoreForPrivilege (Privilege privLevel, float* pfScore) {
    
    int iErrCode;
    Variant vTemp;
    unsigned int iReadColumn;

    switch (privLevel) {

    case APPRENTICE:
        iReadColumn = SystemData::ApprenticeScore;
        break;

    case ADEPT:
        iReadColumn = SystemData::AdeptScore;
        break;

    default:
        Assert (false);
        return ERROR_INVALID_ARGUMENT;
    }

    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, iReadColumn, &vTemp);
    if (iErrCode == OK) {
        *pfScore = vTemp.GetFloat();
    }

    return iErrCode;
}


//
// When adept or apprentice score changes, this function is called
//
int GameEngine::ScanEmpiresOnScoreChanges() {

    int iErrCode;
    unsigned int iEmpireKey = NO_KEY;

    while (true) {

        iErrCode = m_pGameData->GetNextKey (SYSTEM_EMPIRE_DATA, iEmpireKey, &iEmpireKey);
        if (iErrCode != OK) {
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            } else Assert (false);
            break;
        }

        // Best effort
        CalculatePrivilegeLevel (iEmpireKey);
    }

    return OK;
}

int GameEngine::GetSystemOptions (int* piOptions) {

    Variant vTemp;

    int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::Options, &vTemp);
    if (iErrCode == OK) {
        *piOptions = vTemp.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetDefaultGameOptions (int iGameClass, GameOptions* pgoOptions) {

    int iErrCode, iOptions, iMaxNumEmpires;
    Variant vValue;

    Assert (iGameClass != NO_KEY);

    bool bExists;
    iErrCode = m_pGameData->DoesRowExist (SYSTEM_GAMECLASS_DATA, iGameClass, &bExists);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (!bExists) {
        return ERROR_GAMECLASS_DOES_NOT_EXIST;
    }

    // iNumUpdatesBeforeGameCloses
    iErrCode = GetSystemProperty (SystemData::DefaultNumUpdatesBeforeClose, &vValue);
    if (iErrCode != OK) {
        return iErrCode;
    }

    pgoOptions->iNumUpdatesBeforeGameCloses = vValue.GetInteger();

    // iOptions
    pgoOptions->iOptions = 0;

    iErrCode = GetSystemOptions (&iOptions);
    if (iErrCode != OK) {
        return iErrCode;
    }

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

        int iGameClassOptions;

        iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
        if (iErrCode != OK) {
            return iErrCode;
        }

        if ((iGameClassOptions & EXPOSED_SPECTATORS) == EXPOSED_SPECTATORS) {
            pgoOptions->iOptions |= GAME_ALLOW_SPECTATORS;
        }
    }

    if (iOptions & DEFAULT_BRIDIER_GAMES) {

        iErrCode = GetMaxNumEmpires (iGameClass, &iMaxNumEmpires);
        if (iErrCode != OK) {
            return iErrCode;
        }

        if (iMaxNumEmpires == 2) {
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

    return m_pGameData->ReadData (SYSTEM_DATA, SYSTEM_DATA_SHIP_NAME_COLUMN [iTech], pvShipName);
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

    return m_pGameData->WriteData (SYSTEM_DATA, SYSTEM_DATA_SHIP_NAME_COLUMN [iShipKey], pszShipName);
}

int GameEngine::SetSystemOption (int iOption, bool bFlag) {
    
    if (bFlag) {
        return m_pGameData->WriteOr (SYSTEM_DATA, SystemData::Options, iOption);
    } else {
        return m_pGameData->WriteAnd (SYSTEM_DATA, SystemData::Options, ~iOption);
    }
}