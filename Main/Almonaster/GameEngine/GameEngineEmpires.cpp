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

#include "Osal/Algorithm.h"
#include "Osal/Crypto.h"

#include "GameEngine.h"
#include "Global.h"

#include <stdio.h>

//
// Generic work
//

int GameEngine::GetEmpireOption (int iEmpireKey, unsigned int iFlag, bool* pbOption) {

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    Variant vOptions;
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Options, &vOptions);

    if (iErrCode == OK) {
        *pbOption = (vOptions.GetInteger() & iFlag) != 0;
    }

    return iErrCode;
}

int GameEngine::SetEmpireOption (int iEmpireKey, unsigned int iFlag, bool bOption) {

    if (bOption) {
        return t_pConn->WriteOr (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, iFlag);
    } else {
        return t_pConn->WriteAnd (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, ~iFlag);
    }
}

int GameEngine::GetEmpireOption2 (int iEmpireKey, unsigned int iFlag, bool* pbOption) {

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    Variant vOptions;
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Options2, &vOptions);

    if (iErrCode == OK) {
        *pbOption = (vOptions.GetInteger() & iFlag) != 0;
    }

    return iErrCode;
}

int GameEngine::SetEmpireOption2 (int iEmpireKey, unsigned int iFlag, bool bOption) {

    if (bOption) {
        return t_pConn->WriteOr (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options2, iFlag);
    } else {
        return t_pConn->WriteAnd (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options2, ~iFlag);
    }
}

// Input:
// pszEmpireName -> Empire name
// pszPassword -> Empire password
// iPrivilege -> Privilege level;  overridden if iParentKey != NO_KEY
// iParentKey -> Empire whose credentials are to be inherited
//
// Output:
// *piEmpireKey -> Key assigned to new empire (NO_KEY if none)
//
// Create a new empire

int GameEngine::CreateEmpire (const char* pszEmpireName, const char* pszPassword, int iPrivilege,
                              unsigned int iParentKey, bool bBypassDisabled, unsigned int* piEmpireKey) {

    unsigned int i, iKey = NO_KEY;

    int64 i64SecretKey = 0;

    int iOptions = 0, iAlmonasterScoreSignificance, iErrCode;
    float fScore;
    Variant vTemp;

    IReadTable* pTable = NULL;
    IReadTable* pSystemData = NULL;

    *piEmpireKey = NO_KEY;

    // Declare array for insertion
    Variant pvColVal [SystemEmpireData::NumColumns];

    // Make sure empire creation is enabled
    if (!bBypassDisabled) {

        iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::Options, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        if (!(vTemp.GetInteger() & NEW_EMPIRES_ENABLED)) {
            return ERROR_DISABLED;
        }
    }

    // Make sure name is not reserved
    for (i = 0; i < NUM_RESERVED_EMPIRE_NAMES; i ++) {

        if (String::StriCmp (pszEmpireName, RESERVED_EMPIRE_NAMES[i]) == 0) {
            return ERROR_RESERVED_EMPIRE_NAME;
        }
    }

    // Make sure that an empire of the same name doesn't exist
    iErrCode = t_pConn->GetFirstKey(SYSTEM_EMPIRE_DATA, SystemEmpireData::Name, pszEmpireName, &iKey);
    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        // Set the key back to NO_KEY, or we'll hit the delete-empire clause in the cleanup
        Assert (iKey != NO_KEY);
        iKey = NO_KEY;

        if (iErrCode == OK) {
            iErrCode = ERROR_EMPIRE_ALREADY_EXISTS;
        } else Assert (false);
        goto Cleanup;
    }

    // Get current time
    UTCTime tTime;
    Time::GetTime (&tTime);

    // Deal with empire inheritance
    if (iParentKey == NO_KEY) {

        fScore = ALMONASTER_INITIAL_SCORE;
        iAlmonasterScoreSignificance = 0;

        iOptions |= CAN_BROADCAST;
    
    } else {

        int iTemp;
        bool bExist;

        // Check that we can delete
        int iNumGames, * piGameClass, * piGameNumber;
        iErrCode = GetEmpireActiveGames(iParentKey, &piGameClass, &piGameNumber, &iNumGames);
        if (iErrCode == OK)
        {
            delete [] piGameClass;
            delete [] piGameNumber;
        }

        if (iErrCode != OK || iNumGames > 0) {
            iErrCode = ERROR_COULD_NOT_DELETE_EMPIRE;
            goto Cleanup;
        }

        iErrCode = t_pConn->GetTableForReading(SYSTEM_EMPIRE_DATA, &pTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pTable->DoesRowExist (iParentKey, &bExist);
        if (iErrCode != OK || !bExist) {
            iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
            goto Cleanup;
        }

        // Remember secret key
        iErrCode = pTable->ReadData(iParentKey, SystemEmpireData::SecretKey, &i64SecretKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Inherit privilege
        iErrCode = pTable->ReadData (iParentKey, SystemEmpireData::Privilege, &iPrivilege);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        if (iPrivilege == ADMINISTRATOR) {
            iErrCode = ERROR_COULD_NOT_DELETE_ADMINISTRATOR;
            goto Cleanup;
        }

        // Inherit Almonaster score
        iErrCode = pTable->ReadData (iParentKey, SystemEmpireData::AlmonasterScore, &fScore);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Inherit Almonaster score significance
        iErrCode = pTable->ReadData (iParentKey, SystemEmpireData::AlmonasterScoreSignificance, &iAlmonasterScoreSignificance);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Propagate broadcast flag
        iErrCode = pTable->ReadData (iParentKey, SystemEmpireData::Options, &iTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pTable);

        if (iTemp & CAN_BROADCAST) {
            iOptions |= CAN_BROADCAST;
        }
    }
    
    // All empires get confirm on enter by default
    iOptions |= CONFIRM_IMPORTANT_CHOICES;

    // All empires use both types of game page password hashing by default
    iOptions |= SESSION_ID_PASSWORD_HASHING;

    // All empires have ships in up close map views turned on by default but not in planet views...
    iOptions |= SHIPS_ON_MAP_SCREEN;

    // Highlight ships by default on map
    iOptions |= SHIP_MAP_HIGHLIGHTING;

    // Show tech descriptions by default
    iOptions |= SHOW_TECH_DESCRIPTIONS;

    // Show server times by default
    iOptions |= SYSTEM_DISPLAY_TIME | GAME_DISPLAY_TIME;

    // Displace End Turn button by default
    iOptions |= DISPLACE_ENDTURN_BUTTON;

    // Local maps in up-close map views
    iOptions |= LOCAL_MAPS_IN_UPCLOSE_VIEWS;

    // Send last update messages
    iOptions |= DISPLAY_FATAL_UPDATE_MESSAGES;


    // Generate a secret key for the empire
    i64SecretKey = 0;
    iErrCode = Crypto::GetRandomData ((Byte*) &i64SecretKey, sizeof (i64SecretKey));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    pvColVal[SystemEmpireData::iName] = pszEmpireName;
    pvColVal[SystemEmpireData::iPassword] = pszPassword;
    pvColVal[SystemEmpireData::iPrivilege] = iPrivilege;
    pvColVal[SystemEmpireData::iRealName] = (const char*) NULL;
    pvColVal[SystemEmpireData::iEmail] = (const char*) NULL;
    pvColVal[SystemEmpireData::iWebPage] = (const char*) NULL;
    pvColVal[SystemEmpireData::iQuote] = (const char*) NULL;
    pvColVal[SystemEmpireData::iWins] = 0;
    pvColVal[SystemEmpireData::iNukes] = 0;
    pvColVal[SystemEmpireData::iNuked] = 0;
    pvColVal[SystemEmpireData::iLastLoginTime] = tTime;
    pvColVal[SystemEmpireData::iDraws] = 0;
    pvColVal[SystemEmpireData::iMaxEcon] = 0;
    pvColVal[SystemEmpireData::iMaxMil] = 0;
    pvColVal[SystemEmpireData::iIPAddress] = (const char*) NULL;
    pvColVal[SystemEmpireData::iRuins] = 0;
    pvColVal[SystemEmpireData::iClassicScore] = (float) 0.0;
    pvColVal[SystemEmpireData::iAlmonasterScore] = fScore;
    pvColVal[SystemEmpireData::iAlmonasterTheme] = INDIVIDUAL_ELEMENTS;
    pvColVal[SystemEmpireData::iAlternativeGraphicsPath] = (const char*) NULL;
    pvColVal[SystemEmpireData::iSecretKey] = i64SecretKey;
    pvColVal[SystemEmpireData::iOptions2] = 0;
    pvColVal[SystemEmpireData::iGender] = EMPIRE_GENDER_UNKNOWN;
    pvColVal[SystemEmpireData::iAge] = EMPIRE_AGE_UNKNOWN;
    pvColVal[SystemEmpireData::iAssociations] = (const char*) NULL;
    pvColVal[SystemEmpireData::iCustomTableColor] = "000000";
    pvColVal[SystemEmpireData::iOptions] = iOptions;
    pvColVal[SystemEmpireData::iMaxNumShipsBuiltAtOnce] = 10;
    pvColVal[SystemEmpireData::iCreationTime] = tTime;
    pvColVal[SystemEmpireData::iNumLogins] = 0;
    pvColVal[SystemEmpireData::iBrowser] = (const char*) NULL;
    pvColVal[SystemEmpireData::iCustomTextColor] = "000000";
    pvColVal[SystemEmpireData::iCustomGoodColor] = "000000";
    pvColVal[SystemEmpireData::iCustomBadColor] = "000000";
    pvColVal[SystemEmpireData::iCustomPrivateMessageColor] = "000000";
    pvColVal[SystemEmpireData::iCustomBroadcastMessageColor] = "000000";
    pvColVal[SystemEmpireData::iSessionId] = NO_SESSION_ID;
    pvColVal[SystemEmpireData::iDefaultBuilderPlanet] = HOMEWORLD_DEFAULT_BUILDER_PLANET;
    pvColVal[SystemEmpireData::iDefaultMessageTarget] = MESSAGE_TARGET_BROADCAST;
    pvColVal[SystemEmpireData::iAlmonasterScoreSignificance] = iAlmonasterScoreSignificance;
    pvColVal[SystemEmpireData::iVictorySneer] = (const char*) NULL;
    pvColVal[SystemEmpireData::iBridierRank] = BRIDIER_INITIAL_RANK;
    pvColVal[SystemEmpireData::iBridierIndex] = BRIDIER_INITIAL_INDEX;
    pvColVal[SystemEmpireData::iLastBridierActivity] = tTime;
    pvColVal[SystemEmpireData::iPrivateEmail] = (const char*) NULL;
    pvColVal[SystemEmpireData::iLocation] = (const char*) NULL;
    pvColVal[SystemEmpireData::iIMId] = (const char*) NULL;
    pvColVal[SystemEmpireData::iGameRatios] = RATIOS_DISPLAY_ON_RELEVANT_SCREENS;

    if (pvColVal[SystemEmpireData::iName].GetCharPtr() == NULL ||
        pvColVal[SystemEmpireData::iPassword].GetCharPtr() == NULL ||
        pvColVal[SystemEmpireData::iCustomTableColor].GetCharPtr() == NULL ||
        pvColVal[SystemEmpireData::iCustomTextColor].GetCharPtr() == NULL ||
        pvColVal[SystemEmpireData::iCustomGoodColor].GetCharPtr() == NULL ||
        pvColVal[SystemEmpireData::iCustomBadColor].GetCharPtr() == NULL ||
        pvColVal[SystemEmpireData::iCustomPrivateMessageColor].GetCharPtr() == NULL ||
        pvColVal[SystemEmpireData::iCustomBroadcastMessageColor].GetCharPtr() == NULL        
        ) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    // Read defaults from SystemData table
    iErrCode = t_pConn->GetTableForReading(SYSTEM_DATA, &pSystemData);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultAlien, pvColVal + SystemEmpireData::iAlienKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIIndependentPlanet, pvColVal + SystemEmpireData::iUIIndependentPlanet);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultMaxNumSystemMessages, pvColVal + SystemEmpireData::iMaxNumSystemMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIButtons, pvColVal + SystemEmpireData::iUIButtons);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIBackground, pvColVal + SystemEmpireData::iUIBackground);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUILivePlanet, pvColVal + SystemEmpireData::iUILivePlanet);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIDeadPlanet, pvColVal + SystemEmpireData::iUIDeadPlanet);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUISeparator, pvColVal + SystemEmpireData::iUISeparator);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ENUMERATE_SHIP_TYPES(i)
    {
        iErrCode = pSystemData->ReadData(SYSTEM_DATA_SHIP_NAME_COLUMN[i], pvColVal + SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN_INDEX[i]);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIHorz, pvColVal + SystemEmpireData::iUIHorz);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIVert, pvColVal + SystemEmpireData::iUIVert);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIColor, pvColVal + SystemEmpireData::iUIColor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pSystemData);

    // Insert row into SystemEmpireData - this will fail if the name is duped
    iErrCode = t_pConn->InsertRow (SYSTEM_EMPIRE_DATA, SystemEmpireData::Template, pvColVal, &iKey);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DUPLICATE_DATA) {
            iErrCode = ERROR_EMPIRE_ALREADY_EXISTS;
        } else Assert (false);
        goto Cleanup;
    }

#ifdef _DEBUG

    char pszTable [256];

    GET_SYSTEM_EMPIRE_MESSAGES (pszTable, iKey);
    Assert (!t_pConn->DoesTableExist (pszTable));

    GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iKey);
    Assert (!t_pConn->DoesTableExist (pszTable));

    GET_SYSTEM_EMPIRE_TOURNAMENTS (pszTable, iKey);
    Assert (!t_pConn->DoesTableExist (pszTable));

#endif

    // Delete parent empire
    if (iParentKey != NO_KEY) {

        iErrCode = DeleteEmpire (iParentKey, &i64SecretKey, false, false);
        if (iErrCode != OK) {
            Assert (iErrCode == ERROR_EMPIRE_IS_IN_GAMES || iErrCode == ERROR_EMPIRE_DOES_NOT_EXIST);
            goto Cleanup;
        }
    }

    // Best effort add to top lists
    ENUMERATE_SCORING_SYSTEMS (i) {

        ScoringSystem ss = (ScoringSystem) i;
        if (HasTopList (ss)) {
            int iErrCode2 = UpdateTopListOnIncrease (ss, iKey);
            Assert (iErrCode2 == OK);
        }
    }

    // Notification
    global.GetEventSink()->OnCreateEmpire (iKey);

    // Return value
    *piEmpireKey = iKey;

Cleanup:

    SafeRelease(pTable);
    SafeRelease(pSystemData);

    // Best effort delete the new empire's row on failure
    if (iErrCode != OK && iKey != NO_KEY) {
        int iErrCode2 = t_pConn->DeleteRow (SYSTEM_EMPIRE_DATA, iKey);
        Assert (iErrCode2 == OK);
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *pstrName -> Name
//
// Return an empire's name

int GameEngine::GetEmpireName (int iEmpireKey, Variant* pvName)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Name, pvName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }
    return iErrCode;
}

int GameEngine::GetEmpireName (int iEmpireKey, char pszName [MAX_EMPIRE_NAME_LENGTH + 1])
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    int iErrCode;
    ICachedReadTable* pTable = NULL;

    Variant vTemp;

    iErrCode = t_pCache->GetTableForReading(strTableName, &pTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData(iEmpireKey, SystemEmpireData::Name, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    strcpy (pszName, vTemp.GetCharPtr());

Cleanup:

    SafeRelease (pTable);

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
// strName -> New name
//
// Update an empire's name

int GameEngine::SetEmpireName (int iEmpireKey, const char* pszName) {

    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Name, pszName);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> New password
//
// Update an empire's password via admin interface

int GameEngine::SetEmpirePassword(unsigned int iEmpireKey, const char* pszPassword)
{
    if (iEmpireKey == global.GetRootKey()) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    return t_pConn->WriteData(SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, pszPassword);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> New password
//
// Update an empire's password from ProfileEditor

int GameEngine::ChangeEmpirePassword(unsigned int iEmpireKey, const char* pszPassword) {

    if (iEmpireKey == global.GetGuestKey()) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    return t_pConn->WriteData(SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, pszPassword);
}


// Input:
// iEmpireKey -> Integer key of empire
// iBackgroundKey -> New key
//
// Update an empire's background key

int GameEngine::SetEmpireBackgroundKey (int iEmpireKey, int iBackgroundKey) {
    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIBackground, iBackgroundKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iLivePlanetKey -> New key
//
// Update an empire's live planet key

int GameEngine::SetEmpireLivePlanetKey (int iEmpireKey, int iLivePlanetKey) {
    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UILivePlanet, iLivePlanetKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iDeadPlanetKey -> New key
//
// Update an empire's dead planet key

int GameEngine::SetEmpireDeadPlanetKey (int iEmpireKey, int iDeadPlanetKey) {
    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIDeadPlanet, iDeadPlanetKey);
}

// Input:
// iEmpireKey -> Integer key of empire
// iButtonKey -> New key
//
// Update an empire's background key

int GameEngine::SetEmpireButtonKey (int iEmpireKey, int iButtonKey) {
    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIButtons, iButtonKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iSeparatorKey -> New key
//
// Update an empire's separator key

int GameEngine::SetEmpireSeparatorKey (int iEmpireKey, int iSeparatorKey) {
    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UISeparator, iSeparatorKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iHorzKey -> New key
//
// Update an empire's horizontal key

int GameEngine::SetEmpireHorzKey (int iEmpireKey, int iHorzKey) {
    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIHorz, iHorzKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iVertKey -> New key
//
// Update an empire's vertical key

int GameEngine::SetEmpireVertKey (int iEmpireKey, int iVertKey) {
    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIVert, iVertKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iColorKey -> New color key
//
// Update an empire's color key

int GameEngine::SetEmpireColorKey (int iEmpireKey, int iColorKey) {
    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIColor, iColorKey);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piLivePlanetKey -> Live planet key
//
// Get an empire's live planet key

int GameEngine::GetEmpireLivePlanetKey (int iEmpireKey, int* piLivePlanetKey) {

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::UILivePlanet, &vTemp);
    
    if (iErrCode == OK) {
        *piLivePlanetKey = vTemp.GetInteger();
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piDeadPlanetKey -> Dead planet key
//
// Get an empire's dead planet key

int GameEngine::GetEmpireDeadPlanetKey (int iEmpireKey, int* piDeadPlanetKey)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::UIDeadPlanet, &vTemp);
    
    if (iErrCode == OK) {
        *piDeadPlanetKey = vTemp.GetInteger();
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
// iMaxNumSavedMessages -> Number of messages to be saved
//
// Set the empire's MaxNumSavedSystemMessages parameter

int GameEngine::SetEmpireMaxNumSavedSystemMessages (int iEmpireKey, unsigned int iMaxNumSavedMessages) {
    
    int iErrCode;

    unsigned int iNumMessages, * piKey = NULL, iMaxNum;
    Variant vTemp;
    UTCTime* ptTimeStamp = NULL;

    IWriteTable* pMessages = NULL;
    
    SYSTEM_EMPIRE_MESSAGES (strSystemEmpireMessages, iEmpireKey);

    iErrCode = t_pConn->ReadData(
        SYSTEM_EMPIRE_DATA,
        iEmpireKey,
        SystemEmpireData::MaxNumSystemMessages,
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iMaxNum = vTemp.GetInteger();

    // Set the max number of messages
    iErrCode = t_pConn->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::MaxNumSystemMessages, 
        iMaxNumSavedMessages
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iMaxNumSavedMessages >= iMaxNum) {
        goto Cleanup;
    }

    // Lock message table
    iErrCode = t_pConn->GetTableForWriting (strSystemEmpireMessages, &pMessages);
    if (iErrCode != OK) {

        if (iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
            iErrCode = OK;
        }
        goto Cleanup;
    }

    // Get num messages
    iErrCode = pMessages->GetNumRows (&iNumMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // If we're going to be over the limit, trim the list of unread messages
    if (iMaxNum > iMaxNumSavedMessages && iNumMessages > iMaxNumSavedMessages) {

        unsigned int iNumReadMessages;
        
        // Get the oldest messages' keys        
        iErrCode = pMessages->ReadColumn(SystemEmpireMessages::TimeStamp, &piKey, &ptTimeStamp, &iNumReadMessages);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            goto Cleanup;
        }

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        Assert (iNumReadMessages == iNumMessages);

        // Sort the messages by age
        Algorithm::QSortTwoAscending<UTCTime, unsigned int> (ptTimeStamp, piKey, iNumReadMessages);
        
        // Delete read messages until we're below the limit
        unsigned int i, iCurrentNumMessages = iNumReadMessages;
        int iUnread;

        for (i = 0; i < iNumReadMessages && iCurrentNumMessages > iMaxNumSavedMessages; i ++) {
            
            // Has message been read
            iErrCode = pMessages->ReadData (piKey[i], SystemEmpireMessages::Unread, &iUnread);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (iUnread == MESSAGE_UNREAD) {
                continue;
            }

            iErrCode = pMessages->DeleteRow (piKey[i]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iCurrentNumMessages --;
            if (iCurrentNumMessages == 0) {
                break;
            }
        }
    }
    
Cleanup:

    // Unlock the messages table
    SafeRelease (pMessages);

    if (piKey != NULL) {
        t_pConn->FreeKeys(piKey);
    }

    if (ptTimeStamp != NULL) {
        t_pConn->FreeData(ptTimeStamp);
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
// *pszQuote -> New quote string
//
// Output:
// *pbUpdate -> true if quote was update, false if not
//
// Update the empire's quote if a new quote was input

int GameEngine::UpdateEmpireQuote (int iEmpireKey, const char* pszQuote, bool* pbTruncated)
{
    return UpdateEmpireString(iEmpireKey, SystemEmpireData::Quote, pszQuote, MAX_QUOTE_LENGTH, pbTruncated);
}

int GameEngine::UpdateEmpireVictorySneer(int iEmpireKey, const char* pszSneer, bool* pbTruncated)
{
    return UpdateEmpireString(iEmpireKey, SystemEmpireData::VictorySneer, pszSneer, MAX_VICTORY_SNEER_LENGTH, pbTruncated);
}

int GameEngine::UpdateEmpireString(int iEmpireKey, const char* pszColumn, const char* pszString, size_t stMaxLen, bool* pbTruncated)
{
    IWriteTable* pWriteTable = NULL;

    *pbTruncated = false;

    Variant vTemp;

    int iErrCode = t_pConn->GetTableForWriting (SYSTEM_EMPIRE_DATA, &pWriteTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pWriteTable->ReadData(iEmpireKey, pszColumn, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (String::StrCmp (pszString, vTemp.GetCharPtr()) != 0) {

        char* pszNew = NULL;

        if (strlen (pszString) >= stMaxLen) {

            pszNew = new char [stMaxLen + 1];
            if (pszNew == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }

            memcpy (pszNew, pszString, stMaxLen);
            pszNew [stMaxLen] = '\0';

            *pbTruncated = true;
            pszString = pszNew;
        }

        iErrCode = pWriteTable->WriteData(iEmpireKey, pszColumn, pszString);

        if (pszNew != NULL) {
            delete [] pszNew;
        }

    } else {

        iErrCode = WARNING;
    }

Cleanup:

    SafeRelease (pWriteTable);

    return iErrCode;
}


int GameEngine::SendVictorySneer (int iWinnerKey, const char* pszWinnerName, int iLoserKey)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iWinnerKey);

    int iErrCode;
    Variant vSneer;

    iErrCode = t_pCache->ReadData(SYSTEM_EMPIRE_DATA, iWinnerKey, SystemEmpireData::VictorySneer, &vSneer);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!String::IsBlank (vSneer.GetCharPtr())) {

        Variant vWinnerName;

        if (pszWinnerName == NULL) {

            iErrCode = GetEmpireName (iWinnerKey, &vWinnerName);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            pszWinnerName = vWinnerName.GetCharPtr();
        }

        String strMessage = "The victory sneer for " BEGIN_STRONG;
        
        strMessage += pszWinnerName;
        strMessage += END_STRONG " says:\n\n";

        strMessage.AppendHtml (vSneer.GetCharPtr(), 0, true);

        iErrCode = SendSystemMessage (iLoserKey, strMessage.GetCharPtr(), SYSTEM, MESSAGE_SYSTEM);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    return iErrCode;
}

// Input:
// iEmpireKey -> Empire Key
//
// Marks an empire ready for deletion or deletes it if it is in no active games

int GameEngine::DeleteEmpire (int iEmpireKey, int64* pi64SecretKey, bool bMarkOnFailure, bool bDeletePersonal) {

    int iErrCode = OK;
    unsigned int iNumGames = 0;
    Variant vTemp;

    bool bExists;

    iErrCode = DoesEmpireExist (iEmpireKey, &bExists, NULL);
    if (iErrCode != OK || !bExists) {
        iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
        goto Cleanup;
    }
    
    // Verify correct empire
    if (pi64SecretKey != NULL) {

        GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
        iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::SecretKey, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (vTemp.GetInteger64() != *pi64SecretKey) {
            iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
            goto Cleanup;
        }
    }

    // Read active games total
    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszGames, iEmpireKey);
    iErrCode = t_pCache->GetNumCachedRows(pszGames, &iNumGames);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (iNumGames > 0) {

        // Mark empire for deletion?
        if (bMarkOnFailure) {

            iErrCode = SetEmpireOption (iEmpireKey, EMPIRE_MARKED_FOR_DELETION, true);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        if (bDeletePersonal) {

            static const char* s_pszPersonalInfoStringCols[] = {
                SystemEmpireData::RealName,
                SystemEmpireData::Email,
                SystemEmpireData::WebPage,
                SystemEmpireData::Quote,
                SystemEmpireData::VictorySneer,
                SystemEmpireData::PrivateEmail,
                SystemEmpireData::Location,
                SystemEmpireData::IMId,
            };

            for (size_t i = 0; i < countof (s_pszPersonalInfoStringCols); i ++) {

                iErrCode = t_pConn->WriteData (
                    SYSTEM_EMPIRE_DATA, iEmpireKey, s_pszPersonalInfoStringCols[i], (const char*) NULL);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }

            iErrCode = t_pConn->WriteData(SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Gender, EMPIRE_GENDER_UNKNOWN);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = t_pConn->WriteData(SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Age, EMPIRE_AGE_UNKNOWN);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        iErrCode = ERROR_EMPIRE_IS_IN_GAMES;
        
    } else {

        // Delete empire now
        iErrCode = RemoveEmpire (iEmpireKey);
    }

Cleanup:

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
//
// Output
// *piMaxNumSavedMessages -> Return value
//
// Get the empire's max num saved system messages

int GameEngine::GetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int* piMaxNumSavedMessages) {
    
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::MaxNumSystemMessages, &vTemp);
    
    if (iErrCode == OK) {
        *piMaxNumSavedMessages = vTemp.GetInteger();
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
//
// Delete an empire and "ruin" it out of all games

int GameEngine::ObliterateEmpire (unsigned int iEmpireKey, int64 i64SecretKey, unsigned int iKillerEmpire) {

    int iErrCode;

    if (iEmpireKey == global.GetRootKey()) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    if (iEmpireKey == global.GetGuestKey()) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    Assert (iEmpireKey != NO_KEY);
    Assert (iKillerEmpire != NO_KEY);
    
    bool bFlag;

    // Check secret key
    Variant vTemp;
    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::SecretKey, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    if (i64SecretKey != vTemp.GetInteger64()) {
        iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Mark empire for deletion
    // Do this under the lock so we don't race with the enter game code
    iErrCode = SetEmpireOption (iEmpireKey, EMPIRE_MARKED_FOR_DELETION, true);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // At this point, the empire cannot have entered any new games
    // Nuke him out of any game he might be in
    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszGames, iEmpireKey);

    unsigned int iNumGames; 
    Variant* pvGame;
    iErrCode = t_pCache->ReadColumn(pszGames, SystemEmpireActiveGames::GameClassGameNumber, NULL, &pvGame, &iNumGames);
    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Ruin empire out of each game
        for (unsigned int i = 0; i < iNumGames; i ++) {

            int iGameClass, iGameNumber;

            GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

            // Is empire in the game
            iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
            if (iErrCode == OK && bFlag) {

                // Try to quit the empire from the game nicely
                iErrCode = QuitEmpireFromGameInternal (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
                if (iErrCode == ERROR_GAME_HAS_STARTED) {
                        
                    // The empire couldn't be removed nicely, so let's try it the hard way
                    iErrCode = RemoveEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
                    Assert (iErrCode == OK);
                }
            }
        }

        t_pConn->FreeData(pvGame);
    }

    // Try to kill the empire the old fashioned way if he's still around
    if (DoesEmpireExist (iEmpireKey, &bFlag, NULL) == OK && bFlag) {

        iErrCode = DeleteEmpire (iEmpireKey, &i64SecretKey, true, false);
        Assert (iErrCode == OK || iErrCode == ERROR_EMPIRE_DOES_NOT_EXIST || iErrCode == ERROR_EMPIRE_IS_IN_GAMES);
    }

Cleanup:

    return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire to be deleted

int GameEngine::RemoveEmpire (int iEmpireKey) {

    int iErrCode = OK, iErrCode2;

    // Best effort delete / halt all personal gameclasses
    int* piGameClassKey;
    unsigned int i, iNumKeys;
    
    iErrCode2 = GetEmpirePersonalGameClasses (iEmpireKey, &piGameClassKey, NULL, (int*) &iNumKeys);
    Assert (iErrCode == OK);

    if (iErrCode2 == OK && iNumKeys > 0) {
        
        bool bDelete;
        for (i = 0; i < iNumKeys; i ++) {
            
            iErrCode2 = DeleteGameClass (piGameClassKey[i], &bDelete);
            Assert (iErrCode2 == OK);
            
            if (iErrCode2 == OK && !bDelete) {

                iErrCode = t_pConn->WriteData (
                    SYSTEM_GAMECLASS_DATA,
                    piGameClassKey[i],
                    SystemGameClassData::Owner,
                    DELETED_EMPIRE_KEY
                    );
                Assert (iErrCode == OK);
            }
        }
        
        t_pConn->FreeKeys((unsigned int*) piGameClassKey);
    }

    // Best effort delete nuked table
    SYSTEM_EMPIRE_NUKED_LIST (pszTable, iEmpireKey);

    if (t_pConn->DoesTableExist (pszTable)) {
        iErrCode2 = t_pConn->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Best effort delete nuker table
    GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iEmpireKey);

    if (t_pConn->DoesTableExist (pszTable)) {
        iErrCode2 = t_pConn->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Delete tournaments owned by empire or mark them for deletion
    unsigned int* piTournamentDelKey = NULL, iNumDelTournaments;

    iErrCode = t_pConn->GetEqualKeys (
        SYSTEM_TOURNAMENTS,
        SystemTournaments::Owner,
        iEmpireKey,
        &piTournamentDelKey,
        &iNumDelTournaments
        );

    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        for (i = 0; i < iNumDelTournaments; i ++) {

            iErrCode = DeleteTournament (iEmpireKey, piTournamentDelKey[i], true);
            Assert (iErrCode == OK || iErrCode == ERROR_TOURNAMENT_HAS_GAMES);
        }

        t_pConn->FreeKeys(piTournamentDelKey);
    }

    // Delete empire from tournaments it was in
    GET_SYSTEM_EMPIRE_TOURNAMENTS (pszTable, iEmpireKey);

    if (t_pConn->DoesTableExist (pszTable)) {

        unsigned int iKey = NO_KEY;
        Variant vKey;

        while (true) {

            iErrCode = t_pConn->GetNextKey (pszTable, iKey, &iKey);
            if (iErrCode != OK) {
                Assert (iErrCode == ERROR_DATA_NOT_FOUND);
                iErrCode = OK;
                break;
            }

            iErrCode = t_pConn->ReadData(pszTable, iKey, SystemEmpireTournaments::TournamentKey, &vKey);
            if (iErrCode == OK) {
                iErrCode = DeleteEmpireFromTournament (vKey.GetInteger(), iEmpireKey);
                Assert (iErrCode == OK);
            }
        }

        iErrCode = t_pConn->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Best effort delete messages table
    GET_SYSTEM_EMPIRE_MESSAGES (pszTable, iEmpireKey);

    if (t_pConn->DoesTableExist (pszTable)) {
        iErrCode2 = t_pConn->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Notification
    global.GetEventSink()->OnDeleteEmpire (iEmpireKey);

    // Delete row from SystemEmpireData table
    IWriteTable* pEmpires = NULL;
    iErrCode = t_pConn->GetTableForWriting (SYSTEM_EMPIRE_DATA, &pEmpires);
    Assert (iErrCode == OK);

    if (iErrCode == OK) {

        iErrCode = RemoveDeadEmpireAssociations (pEmpires, iEmpireKey);
        Assert (iErrCode == OK);

        iErrCode = pEmpires->DeleteRow (iEmpireKey);
        Assert (iErrCode == OK);

        SafeRelease (pEmpires);
    }

    // Remove from top lists
    ENUMERATE_SCORING_SYSTEMS (i) {

        ScoringSystem ss = (ScoringSystem) i;
        if (HasTopList (ss)) {
            iErrCode2 = UpdateTopListOnDeletion (ss, iEmpireKey);
            Assert (iErrCode2 == OK);
        }
    }

    return iErrCode;
}


// Input:
// pszName -> Name of empire
//
// Output:
// *pbExists -> true if exists, false if not
// *piEmpireKey -> Key of empire
// *pvEmpireName -> Proper capitalization of empire name
//
// Determines if a given empire name already exists

int GameEngine::DoesEmpireExist (const char* pszName, bool* pbExists, unsigned int* piEmpireKey, 
                                 Variant* pvEmpireName, int64* piSecretKey) {

    int iErrCode;

    IReadTable* pEmpires = NULL;
    unsigned int iEmpireKey;

    *pbExists = false;
    *piEmpireKey = NO_KEY;

    iErrCode = t_pConn->GetTableForReading(SYSTEM_EMPIRE_DATA, &pEmpires);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = pEmpires->GetFirstKey(SystemEmpireData::Name, pszName, &iEmpireKey);
    if (iErrCode != OK) {

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert (false);
        goto Cleanup;
    }
    Assert (iEmpireKey != NO_KEY);

    // Fetch proper capitalization if required
    if (pvEmpireName != NULL) {

        iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::Name, pvEmpireName);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Fetch secret key if required
    if (piSecretKey != NULL) {

        iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::SecretKey, piSecretKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    SafeRelease (pEmpires);

    *piEmpireKey = iEmpireKey;
    *pbExists = true;

Cleanup:

    SafeRelease (pEmpires);

    return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *pbExists -> true if exists, false if not
//
// Determines if a given empire key exists

int GameEngine::DoesEmpireExist (unsigned int iEmpireKey, bool* pbExists, Variant* pvEmpireName) {

    int iErrCode;
    ICachedReadTable* pEmps = NULL;

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    iErrCode = t_pCache->GetTableForReading(strTableName, &pEmps);
    if (iErrCode == OK)
    {
        if (pvEmpireName != NULL)
        {
            iErrCode = pEmps->ReadData(iEmpireKey, SystemEmpireData::Name, pvEmpireName);
        }
        else
        {
            int iTest;
            iErrCode = pEmps->ReadData(iEmpireKey, SystemEmpireData::Options, &iTest);
        }
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            *pbExists = false;
            iErrCode = OK;
        }
        else if (iErrCode == OK)
        {
            *pbExists = true;
        }
    }

    SafeRelease(pEmps);

    return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
// iSecretKey -> Secret key of empire
//
// Output:
// *pbMatch-> true if key matches name
//
// Determines if a given secret key matches a given empire key

int GameEngine::CheckSecretKey (unsigned int iEmpireKey, int64 i64SecretKey, bool* pbMatch, int64* pi64SessionId, 
                                Variant* pvIPAddress) {

    int iErrCode;
    bool bFlag;
    int64 i64StoredSecretKey;

    IReadTable* pEmpires = NULL;

    *pbMatch = false;

    iErrCode = t_pConn->GetTableForReading(SYSTEM_EMPIRE_DATA, &pEmpires);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Check row
    iErrCode = pEmpires->DoesRowExist (iEmpireKey, &bFlag);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (!bFlag) {
        goto Cleanup;
    }

    iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::SecretKey, &i64StoredSecretKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (i64SecretKey == i64StoredSecretKey) {

        *pbMatch = true;

        if (pi64SessionId != NULL) {

            iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::SessionId, pi64SessionId);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        if (pvIPAddress != NULL) {

            iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::IPAddress, pvIPAddress);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:

    SafeRelease (pEmpires);

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> Password to be tested
//
// Given an empire name and a password, determines if that password is correct for that empire
// Return OK if yes, an error if no

int GameEngine::IsPasswordCorrect (int iEmpireKey, const char* pszPassword) {

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    int iErrCode;
    Variant vPassword;

    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Password, &vPassword);
    if (iErrCode != OK) {
        return iErrCode;
    }

    return String::StrCmp (vPassword.GetCharPtr(), pszPassword) == 0 ? OK : ERROR_PASSWORD;
}


// Input:
// iEmpireKey -> Integer key of empire
// pszBrowser -> Browser used to request login
//
// Perform a login for an empire and update volatile parameters.  Should be called after 
// the empire's password has been validated and the empire's key has been obtained.

int GameEngine::LoginEmpire (int iEmpireKey, const char* pszBrowser, const char* pszIPAddress) {

    Variant vLogins, vPriv = NOVICE;

    IWriteTable* pTable = NULL;

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    // Make sure logins are allowed
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::Options, &vLogins);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Get the time
    UTCTime tTime;
    Time::GetTime (&tTime);

    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Privilege, &vPriv);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (!(vLogins.GetInteger() & LOGINS_ENABLED)) {

        // Only admins can pass...
        if (vPriv.GetInteger() < ADMINISTRATOR) {
            iErrCode = ERROR_DISABLED;
            goto Cleanup;
        }
    }

    iErrCode = t_pConn->GetTableForWriting (SYSTEM_EMPIRE_DATA, &pTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (String::StrLen (pszBrowser) > MAX_BROWSER_NAME_LENGTH) {
        
        char pszCutBrowser [MAX_BROWSER_NAME_LENGTH + 1];
        memcpy (pszCutBrowser, pszBrowser, MAX_BROWSER_NAME_LENGTH);
        pszCutBrowser [MAX_BROWSER_NAME_LENGTH] = '\0';

        iErrCode = pTable->WriteData (iEmpireKey, SystemEmpireData::Browser, pszCutBrowser);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

    } else {
        
        iErrCode = pTable->WriteData (iEmpireKey, SystemEmpireData::Browser, pszBrowser);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Write IP address
    iErrCode = pTable->WriteData (iEmpireKey, SystemEmpireData::IPAddress, pszIPAddress);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Write LastLoginTime
    iErrCode = pTable->WriteData (iEmpireKey, SystemEmpireData::LastLoginTime, tTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Increment the number of logins
    iErrCode = pTable->Increment (iEmpireKey, SystemEmpireData::NumLogins, 1);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pTable);

    // Outside the lock...  if we're an admin, best effort test all closed games for an update
    if (iErrCode == OK) {
        
        // Notification
        global.GetEventSink()->OnLoginEmpire (iEmpireKey);
        
        if (vPriv.GetInteger() >= ADMINISTRATOR) {
            global.GetAsyncManager()->QueueTask(CheckAllGamesForUpdatesMsg, NULL);
        }
    }

    return iErrCode;
}


int GameEngine::CheckAllGamesForUpdatesMsg(AsyncTask* pMessage)
{
    GameEngine gameEngine;
    return gameEngine.CheckAllGamesForUpdates(true);
}

// Output:
// *piNumEmpires -> Number of empires
//
// Returns the number of empires currently registered on the server

int GameEngine::GetNumEmpiresOnServer (int* piNumEmpires) {

    return t_pConn->GetNumRows (SYSTEM_EMPIRE_DATA, (unsigned int*) piNumEmpires);
}

// Input:
// iEmpireKey -> Empire Key
//
// Undeletes an empire marked for deletion

int GameEngine::UndeleteEmpire (int iEmpireKey) {

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    int iErrCode;
    Variant vTemp;

    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Options, &vTemp);
    if (iErrCode == OK && vTemp.GetInteger() & EMPIRE_MARKED_FOR_DELETION) {

        iErrCode = t_pConn->WriteAnd (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey, 
            SystemEmpireData::Options, 
            ~EMPIRE_MARKED_FOR_DELETION
            );

    } else {
        iErrCode = ERROR_CANNOT_UNDELETE_EMPIRE;
    }

    return iErrCode;
}

// Input:
// iEmpireKey -> Integer key of empire
//
// Sets all of an empire's statistics to their default values

int GameEngine::BlankEmpireStatistics(unsigned int iEmpireKey) {

    int iErrCode;
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    char pszTable [256];
    Variant vPriv, vOldClassicScore, vOldAlmonasterScore, vOldBridierIndex;

    if (iEmpireKey == global.GetGuestKey()) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    // Get old scores
    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::ClassicScore, &vOldClassicScore);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::AlmonasterScore, &vOldAlmonasterScore);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    //
    // Blank Bridier Score
    //

    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::BridierIndex, &vOldBridierIndex);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierRank, BRIDIER_INITIAL_RANK);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierIndex, BRIDIER_INITIAL_INDEX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    //
    // Blank statistics
    //
    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Wins, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Nukes, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Nuked, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Draws, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Ruins, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxEcon, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxMil, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Blank Classic Score
    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::ClassicScore, CLASSIC_INITIAL_SCORE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Blank Almonaster Score, Significance and privilege level
    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, ALMONASTER_INITIAL_SCORE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScoreSignificance, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Privilege, &vPriv);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vPriv.GetInteger() != ADMINISTRATOR) {

        iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, NOVICE);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Blank nuke history fields
    GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iEmpireKey);

    if (t_pConn->DoesTableExist (pszTable)) {

        iErrCode = t_pConn->DeleteAllRows (pszTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iEmpireKey);

    if (t_pConn->DoesTableExist (pszTable)) {

        iErrCode = t_pConn->DeleteAllRows (pszTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Update top lists
    if (vOldClassicScore.GetFloat() > CLASSIC_INITIAL_SCORE) {
        iErrCode = UpdateTopListOnDecrease (CLASSIC_SCORE, iEmpireKey);
    }

    else if (vOldClassicScore.GetFloat() < CLASSIC_INITIAL_SCORE) {
        iErrCode = UpdateTopListOnIncrease (CLASSIC_SCORE, iEmpireKey);
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vOldAlmonasterScore.GetFloat() > ALMONASTER_INITIAL_SCORE) {

        iErrCode = UpdateTopListOnDecrease (ALMONASTER_SCORE, iEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (vOldBridierIndex.GetInteger() <= BRIDIER_TOPLIST_INDEX) {

        iErrCode = UpdateTopListOnDecrease (BRIDIER_SCORE, iEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (vOldBridierIndex.GetInteger() <= BRIDIER_ESTABLISHED_TOPLIST_INDEX) {

        iErrCode = UpdateTopListOnDecrease (BRIDIER_SCORE_ESTABLISHED, iEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiGameClassKey -> Array of keys
// **ppvName -> Array of names
// *piNumKeys -> Number of game classes
//
// Return the keys of the personal gameclasses an empire has created

int GameEngine::GetEmpirePersonalGameClasses (int iEmpireKey, int** ppiGameClassKey, Variant** ppvName,
                                              int* piNumKeys) {

    int iErrCode;

    if (ppvName == NULL) {

        iErrCode = t_pConn->GetEqualKeys (
            SYSTEM_GAMECLASS_DATA,
            SystemGameClassData::Owner,
            iEmpireKey,
            (unsigned int**) ppiGameClassKey,
            (unsigned int*) piNumKeys
            );
    
    } else {

        iErrCode = t_pConn->ReadColumnWhereEqual (
            SYSTEM_GAMECLASS_DATA,
            SystemGameClassData::Owner,
            iEmpireKey,
            SystemGameClassData::Name,
            (unsigned int**) ppiGameClassKey,
            ppvName,
            (unsigned int*) piNumKeys
            );
    }

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppvEmpData -> SystemEmpireData array
//
// *piNumActiveGames -> Number of games the empire is active in
// 
// Returns empire data

int GameEngine::GetEmpireData(int iEmpireKey, Variant** ppvEmpData, int* piNumActiveGames)
{
    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszActiveGames, iEmpireKey);

    *ppvEmpData = NULL;
    *piNumActiveGames = 0;

    // Get num active games
    int iErrCode = t_pCache->GetNumCachedRows(pszActiveGames, (unsigned int*)piNumActiveGames);
    if (iErrCode != OK && iErrCode != ERROR_UNKNOWN_TABLE_NAME)
    {
        return iErrCode;
    }

    // Get data
    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);
    return t_pCache->ReadRow(strEmpires, iEmpireKey, ppvEmpData);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiGameClass -> Array of game classes
// **ppiGameNumber -> Array of game numbers
//
// Returns the gameclasses and gamenumbers of the games an empire is currently in

int GameEngine::GetEmpireActiveGames (int iEmpireKey, int** ppiGameClass, int** ppiGameNumber, int* piNumGames) {

    int iErrCode = OK;

    ICachedReadTable* pGames = NULL;
    Variant* pvGames = NULL;

    *ppiGameClass = NULL;
    *ppiGameNumber = NULL;
    *piNumGames = 0;

    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszGames, iEmpireKey);
    iErrCode = t_pCache->GetTableForReading(pszGames, &pGames);
    if (iErrCode == OK)
    {
        iErrCode = pGames->ReadColumn(SystemEmpireActiveGames::GameClassGameNumber, NULL, &pvGames, (unsigned int*)piNumGames);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else if (iErrCode == OK)
        {
            *ppiGameClass = new int[*piNumGames];
            Assert(*ppiGameClass);

            *ppiGameNumber = new int[*piNumGames];
            Assert(*ppiGameNumber);

            for (int i = 0; i < *piNumGames; i ++)
            {
                GetGameClassGameNumber(pvGames[i].GetCharPtr(), &((*ppiGameClass)[i]), &((*ppiGameNumber)[i]));
            }
        }
    }

    SafeRelease (pGames);

    if (pvGames != NULL)
    {
        t_pConn->FreeData(pvGames);
    }

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of empire
//
// Output:
// *pbInGame -> true if in game, false otherwise
//
// Determine if an empire is in a game

int GameEngine::IsEmpireInGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbInGame) {

    int iErrCode;
    bool bGame;

    *pbInGame = false;

    // Check game
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bGame);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (!bGame) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);
    
    unsigned int iKey = NO_KEY;
    iErrCode = t_pConn->GetFirstKey(pszEmpires, GameEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == OK) {
        *pbInGame = true;
    }

    else if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piPrivilege -> Privilege level
//
// Return the privilege level of an empire

int GameEngine::GetEmpirePrivilege(unsigned int iEmpireKey, int* piPrivilege) {

    Variant vPrivilege;
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Privilege, &vPrivilege);
    if (iErrCode == OK) {
        *piPrivilege = vPrivilege.GetInteger();
    }
    
    return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// iPrivilege -> Privilege level
//
// Set the privilege level of an empire

int GameEngine::SetEmpirePrivilege(unsigned int iEmpireKey, int iPrivilege) {

    if (iEmpireKey == global.GetRootKey()) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    if (iEmpireKey == global.GetGuestKey()) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    if (!IsLegalPrivilege (iPrivilege)) {
        return ERROR_INVALID_PRIVILEGE;
    }

    int iErrCode = t_pConn->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::Privilege, 
        iPrivilege
        );
    
    if (iErrCode == OK) {

        char pszMessage [512];
        sprintf (
            pszMessage,
            "Your privilege level was changed to %s by an administrator",
            PRIVILEGE_STRING [iPrivilege]
            );

        // Send a message to the affected empire
        SendSystemMessage (iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// *pfAlmonasterScore -> Almonaster score
//
// Return an empire's Almonaster score

int GameEngine::GetEmpireAlmonasterScore(unsigned int iEmpireKey, float* pfAlmonasterScore) {

    Variant vAlmonasterScore;
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::AlmonasterScore, &vAlmonasterScore);
    
    if (iErrCode == OK) {
        *pfAlmonasterScore = vAlmonasterScore.GetFloat();
    }
    
    return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output:
// fAlmonasterScore -> Almonaster score
//
// Set an empire's Almonaster score

int GameEngine::SetEmpireAlmonasterScore(unsigned int iEmpireKey, float fAlmonasterScore) {

    if (iEmpireKey == global.GetRootKey()) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    if (iEmpireKey == global.GetGuestKey()) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    if (fAlmonasterScore < ALMONASTER_MIN_SCORE || fAlmonasterScore > ALMONASTER_MAX_SCORE) {
        return ERROR_INVALID_ARGUMENT;
    }

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    
    Variant vOldScore;
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::AlmonasterScore, &vOldScore);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (fAlmonasterScore == vOldScore.GetFloat()) {
        return OK;
    }

    iErrCode = t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, fAlmonasterScore);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = CalculatePrivilegeLevel (iEmpireKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (fAlmonasterScore > vOldScore.GetFloat()) {
        iErrCode = UpdateTopListOnIncrease(ALMONASTER_SCORE, iEmpireKey);
    } else {
        iErrCode = UpdateTopListOnDecrease(ALMONASTER_SCORE, iEmpireKey);
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *pstrPassword -> Empire's password
//
// Return an empire's password

int GameEngine::GetEmpirePassword (unsigned int iEmpireKey, Variant* pvPassword)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    return t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Password, pvPassword);
}


// Input:
// iEmpireKey -> Key of empire
// strColumn -> Name of column
//
// Output:
// *pvData -> Data column
//
// Get empire data column from SystemEmpireData

int GameEngine::GetEmpireDataColumn(int iEmpireKey, const char* pszColumn, Variant* pvData)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    return t_pCache->ReadData(strTableName, iEmpireKey, pszColumn, pvData);
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *piNumLogins -> Number of successful logins
//
// Get the number of successful logins performed by the empire

int GameEngine::GetNumLogins (int iEmpireKey, int* piNumLogins)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::NumLogins, &vTemp);
    if (iErrCode == OK) {
        *piNumLogins = vTemp.GetInteger();
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// pvIPAddress -> Empire's IP address
//
// Get an empire's IP address

int GameEngine::GetEmpireIPAddress (int iEmpireKey, Variant* pvIPAddress)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    return t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::IPAddress, pvIPAddress);
}


// Input:
// iEmpireKey -> Key of empire
// pszIPAddress -> New IP address
//
// Set an empire's IP address

int GameEngine::SetEmpireIPAddress (int iEmpireKey, const char* pszIPAddress) {

    return t_pConn->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::IPAddress,
        pszIPAddress
        );
}

int GameEngine::GetEmpireSessionId (int iEmpireKey, int64* pi64SessionId) {

    int iErrCode;
    ICachedReadTable* pSystemEmpireData;

    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    iErrCode = t_pCache->GetTableForReading(strTableName, &pSystemEmpireData);
    if (iErrCode == OK)
    {
        iErrCode = pSystemEmpireData->ReadData(iEmpireKey, SystemEmpireData::SessionId, pi64SessionId);
    }

    SafeRelease(pSystemEmpireData);

    return iErrCode;
}

int GameEngine::SetEmpireSessionId (int iEmpireKey, int64 i64SessionId) {

    return t_pConn->WriteData (
        SYSTEM_EMPIRE_DATA,
        iEmpireKey,
        SystemEmpireData::SessionId,
        i64SessionId
        );
}


int GameEngine::GetEmpireDefaultBuilderPlanet (int iEmpireKey, int* piDefaultBuildPlanet)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    Variant vValue;
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::DefaultBuilderPlanet, &vValue);
    if (iErrCode == OK)
    {
        *piDefaultBuildPlanet = vValue.GetInteger();
    }

    return iErrCode;
}


int GameEngine::SetEmpireDefaultBuilderPlanet (int iEmpireKey, int iDefaultBuildPlanet) {

    if (iDefaultBuildPlanet > NO_DEFAULT_BUILDER_PLANET ||
        iDefaultBuildPlanet < LAST_BUILDER_DEFAULT_BUILDER_PLANET) {
        return ERROR_INVALID_ARGUMENT;
    }

    return t_pConn->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::DefaultBuilderPlanet,
        iDefaultBuildPlanet
        );
}


int GameEngine::GetEmpireDefaultMessageTarget (int iEmpireKey, int* piMessageTarget)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    Variant vData;
    
    int iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::DefaultMessageTarget, &vData);
    if (iErrCode == OK) {
        *piMessageTarget = vData.GetInteger();
    }

    return iErrCode;
}


int GameEngine::SetEmpireDefaultMessageTarget (int iEmpireKey, int iMessageTarget) {

    if (iMessageTarget < MESSAGE_TARGET_NONE || iMessageTarget > MESSAGE_TARGET_LAST_USED) {
        return ERROR_INVALID_ARGUMENT;
    }

    return t_pConn->WriteData (
        SYSTEM_EMPIRE_DATA,
        iEmpireKey,
        SystemEmpireData::DefaultMessageTarget,
        iMessageTarget
        );
}


// Input:
// iEmpireKey -> Key of empire
//
// Tell the login process to reset the empire's session id on next login

int GameEngine::ResetEmpireSessionId (int iEmpireKey) {
    return SetEmpireOption (iEmpireKey, RESET_SESSION_ID, true);
}


// Input:
// iEmpireKey -> Key of empire
//
// Don't reset the empire's session id on next login

int GameEngine::EndResetEmpireSessionId (int iEmpireKey) {
    return SetEmpireOption (iEmpireKey, RESET_SESSION_ID, false);
}

// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *piOptions -> empire options from SystemEmpireData::Options
//
// Return an empire's options

int GameEngine::GetEmpireOptions (int iEmpireKey, int* piOptions) {

    int iErrCode;
    Variant vTemp;
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Options, &vTemp);
    if (iErrCode == OK) {
        *piOptions = vTemp.GetInteger();
    }

    return iErrCode;
}

// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *piOptions -> empire options from SystemEmpireData::Options2
//
// Return an empire's options

int GameEngine::GetEmpireOptions2 (int iEmpireKey, int* piOptions) {

    int iErrCode;
    Variant vTemp;
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::Options2, &vTemp);
    if (iErrCode == OK) {
        *piOptions = vTemp.GetInteger();
    }

    return iErrCode;
}


int GameEngine::GetEmpireLastBridierActivity (int iEmpireKey, UTCTime* ptTime) {

    int iErrCode;
    Variant vTemp;
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);

    iErrCode = t_pCache->ReadData(strTableName, iEmpireKey, SystemEmpireData::LastBridierActivity, &vTemp);
    if (iErrCode == OK) {
        *ptTime = vTemp.GetInteger64();
    }

    return iErrCode;
}

int GameEngine::GetEmpireProperty (int iEmpireKey, const char* pszColumn, Variant* pvProperty)
{
    GET_SYSTEM_EMPIRE_DATA(strTableName, iEmpireKey);
    return t_pCache->ReadData(strTableName, iEmpireKey, pszColumn, pvProperty);
}

int GameEngine::SetEmpireProperty (int iEmpireKey, const char* pszColumn, const Variant& vProperty) {

    return t_pConn->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, pszColumn, vProperty);
}

int GameEngine::IsEmpireIdleInSomeGame (int iEmpireKey, bool* pfIdle) {

    int iErrCode, iGameClass, iGameNumber;
    unsigned int i, iNumGames = 0;
    Variant* pvGame = NULL;

    char pszGameData [256];

    *pfIdle = false;

    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszGames, iEmpireKey);
    iErrCode = t_pCache->ReadColumn(pszGames, SystemEmpireActiveGames::GameClassGameNumber, NULL, &pvGame, &iNumGames);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND || iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
            iErrCode = OK;
            goto Cleanup;
        }
    }

    for (i = 0; i < iNumGames; i ++) {

        GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);
        GET_GAME_EMPIRE_DATA (pszGameData, iGameClass, iGameNumber, iEmpireKey);

        Variant vOptions;
        iErrCode = t_pConn->ReadData(pszGameData, GameEmpireData::Options, &vOptions);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        // Ignore games in which the empire has resigned
        if (vOptions.GetInteger() & RESIGNED) {
            continue;
        }

        if (!(vOptions.GetInteger() & LOGGED_IN_THIS_UPDATE)) {

            Variant vNumUpdatesIdle;
            iErrCode = t_pConn->ReadData(pszGameData, GameEmpireData::NumUpdatesIdle, &vNumUpdatesIdle);
            if (iErrCode != OK) {
                goto Cleanup;
            }

            Variant vNumUpdatesForIdle;
            iErrCode = GetGameClassProperty (iGameClass, SystemGameClassData::NumUpdatesForIdle, &vNumUpdatesForIdle);

            if (vNumUpdatesIdle.GetInteger() >= vNumUpdatesForIdle.GetInteger()) {
                *pfIdle = true;
                goto Cleanup;
            }
        }
    }

Cleanup:

    if (pvGame != NULL) {
        t_pConn->FreeData(pvGame);
    }

    return iErrCode;
}