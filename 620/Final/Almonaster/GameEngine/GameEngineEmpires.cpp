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

#include "Osal/Algorithm.h"
#include "Osal/Crypto.h"

#include "GameEngine.h"

#include <stdio.h>

//
// Generic work
//

int GameEngine::GetEmpireOption (int iEmpireKey, unsigned int iFlag, bool* pbOption) {

    Variant vOptions;
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vOptions);

    if (iErrCode == OK) {
        *pbOption = (vOptions.GetInteger() & iFlag) != 0;
    }

    return iErrCode;
}

int GameEngine::SetEmpireOption (int iEmpireKey, unsigned int iFlag, bool bOption) {

    if (bOption) {
        return m_pGameData->WriteOr (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, iFlag);
    } else {
        return m_pGameData->WriteAnd (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, ~iFlag);
    }
}

int GameEngine::GetEmpireOption2 (int iEmpireKey, unsigned int iFlag, bool* pbOption) {

    Variant vOptions;
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options2, &vOptions);

    if (iErrCode == OK) {
        *pbOption = (vOptions.GetInteger() & iFlag) != 0;
    }

    return iErrCode;
}

int GameEngine::SetEmpireOption2 (int iEmpireKey, unsigned int iFlag, bool bOption) {

    if (bOption) {
        return m_pGameData->WriteOr (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options2, iFlag);
    } else {
        return m_pGameData->WriteAnd (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options2, ~iFlag);
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

    NamedMutex nmParentMutex;
    unsigned int iKey, i;

    bool bParentLocked = false;

    int iOptions = 0, iAlmonasterScoreSignificance = 0, iErrCode;
    float fScore;
    Variant vTemp;

    IReadTable* pTable = NULL;

    *piEmpireKey = NO_KEY;

    // Make sure empire creation is enabled
    if (!bBypassDisabled) {

        iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::Options, &vTemp);
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

    // Get current time
    UTCTime tTime;
    Time::GetTime (&tTime);

    // Declare array for insertion
    Variant pvColVal [SystemEmpireData::NumColumns];

    LockEmpires();

    // Make sure that an empire of the same name doesn't exist
    iErrCode = m_pGameData->GetFirstKey (
        SYSTEM_EMPIRE_DATA, 
        SystemEmpireData::Name, 
        pszEmpireName, 
        true, 
        &iKey
        );

    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        
        if (iErrCode == OK) {
            iErrCode = ERROR_EMPIRE_ALREADY_EXISTS;
            goto Cleanup;
        }
        
        Assert (false);
        goto Cleanup;
    }

    // Deal with empire inheritance
    if (iParentKey != NO_KEY) {

        bool bExist;

        bParentLocked = true;
        LockEmpire (iParentKey, &nmParentMutex);

        iErrCode = m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iParentKey, &bExist);
        if (iErrCode != OK || !bExist) {
            iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
            goto Cleanup;
        }

        unsigned int iNumRows = 0;
        SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iParentKey);

        if (m_pGameData->DoesTableExist (pszTable)) {

            iErrCode = m_pGameData->GetNumRows (pszTable, &iNumRows);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        if (iNumRows > 0) {
            iErrCode = ERROR_COULD_NOT_DELETE_EMPIRE;
            goto Cleanup;
        }

        // Inherit privilege
        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iParentKey, SystemEmpireData::Privilege, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (vTemp.GetInteger() == ADMINISTRATOR) {
            iErrCode = ERROR_COULD_NOT_DELETE_ADMINISTRATOR;
            goto Cleanup;
        }
        iPrivilege = vTemp.GetInteger();

        // Inherit Almonaster score
        iErrCode = m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iParentKey, 
            SystemEmpireData::AlmonasterScore, 
            &vTemp
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        fScore = vTemp.GetFloat();

        // Inherit Almonaster score significance
        iErrCode = m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iParentKey, 
            SystemEmpireData::AlmonasterScoreSignificance, 
            &vTemp
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iAlmonasterScoreSignificance = vTemp.GetInteger();

        // Propagate can't broadcast flag
        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iParentKey, SystemEmpireData::Options, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (vTemp.GetInteger() & CAN_BROADCAST) {
            iOptions |= CAN_BROADCAST;
        }

    } else {

        fScore = ALMONASTER_INITIAL_SCORE;
        iOptions |= CAN_BROADCAST;
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


    // Get SystemData table
    iErrCode = m_pGameData->GetTableForReading (SYSTEM_DATA, &pTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Insert row into SystemEmpireData
    pvColVal[SystemEmpireData::Name] = pszEmpireName;
    pvColVal[SystemEmpireData::Password] = pszPassword;

    if (pvColVal[SystemEmpireData::Name].GetCharPtr() == NULL ||
        pvColVal[SystemEmpireData::Password].GetCharPtr() == NULL) {

        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    pvColVal[SystemEmpireData::Privilege] = iPrivilege;
    pvColVal[SystemEmpireData::RealName] = "";
    pvColVal[SystemEmpireData::Email] = "";
    pvColVal[SystemEmpireData::WebPage] = "";
    pvColVal[SystemEmpireData::Quote] = "";

    iErrCode = pTable->ReadData (SystemData::DefaultAlien, &(pvColVal[SystemEmpireData::AlienKey]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIIndependentPlanet, &(pvColVal[SystemEmpireData::UIIndependentPlanet]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    pvColVal[SystemEmpireData::Wins] = 0;
    pvColVal[SystemEmpireData::Nukes] = 0;
    pvColVal[SystemEmpireData::Nuked] = 0;
    pvColVal[SystemEmpireData::LastLoginTime] = tTime;
    pvColVal[SystemEmpireData::Draws] = 0;
    pvColVal[SystemEmpireData::MaxEcon] = 0;
    pvColVal[SystemEmpireData::MaxMil] = 0;
    pvColVal[SystemEmpireData::IPAddress] = "";
    pvColVal[SystemEmpireData::Ruins] = 0;

    iErrCode = pTable->ReadData (
        SystemData::DefaultMaxNumSystemMessages, 
        &(pvColVal[SystemEmpireData::MaxNumSystemMessages])
        );
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    pvColVal[SystemEmpireData::ClassicScore] = (float) 0.0;
    pvColVal[SystemEmpireData::AlmonasterScore] = fScore;

    iErrCode = pTable->ReadData (SystemData::DefaultUIButtons, &(pvColVal[SystemEmpireData::UIButtons]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIBackground, &(pvColVal[SystemEmpireData::UIBackground]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUILivePlanet, &(pvColVal[SystemEmpireData::UILivePlanet]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIDeadPlanet, &(pvColVal[SystemEmpireData::UIDeadPlanet]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUISeparator, &(pvColVal[SystemEmpireData::UISeparator]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    pvColVal[SystemEmpireData::AlmonasterTheme] = INDIVIDUAL_ELEMENTS;
    pvColVal[SystemEmpireData::AlternativeGraphicsPath] = "";

    ENUMERATE_SHIP_TYPES (i) {

        iErrCode = pTable->ReadData (
            SYSTEM_DATA_SHIP_NAME_COLUMN[i], 
            pvColVal + SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[i]
            );
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIHorz, &(pvColVal[SystemEmpireData::UIHorz]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIVert, &(pvColVal[SystemEmpireData::UIVert]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (SystemData::DefaultUIColor, &(pvColVal[SystemEmpireData::UIColor]));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    pvColVal[SystemEmpireData::CustomTableColor] = "000000";
    pvColVal[SystemEmpireData::Options] = iOptions;
    pvColVal[SystemEmpireData::MaxNumShipsBuiltAtOnce] = 10;
    pvColVal[SystemEmpireData::CreationTime] = tTime;
    pvColVal[SystemEmpireData::NumLogins] = 0;
    pvColVal[SystemEmpireData::Browser] = "";

    pvColVal[SystemEmpireData::CustomTextColor] = "000000";
    pvColVal[SystemEmpireData::CustomGoodColor] = "000000";
    pvColVal[SystemEmpireData::CustomBadColor] = "000000";
    pvColVal[SystemEmpireData::CustomPrivateMessageColor] = "000000";
    pvColVal[SystemEmpireData::CustomBroadcastMessageColor] = "000000";
    pvColVal[SystemEmpireData::SessionId] = NO_SESSION_ID;
    pvColVal[SystemEmpireData::DefaultBuilderPlanet] = HOMEWORLD_DEFAULT_BUILDER_PLANET;
    pvColVal[SystemEmpireData::DefaultMessageTarget] = MESSAGE_TARGET_BROADCAST;

    pvColVal[SystemEmpireData::AlmonasterScoreSignificance] = iAlmonasterScoreSignificance;
    pvColVal[SystemEmpireData::VictorySneer] = "";

    pvColVal[SystemEmpireData::BridierRank] = BRIDIER_INITIAL_RANK;
    pvColVal[SystemEmpireData::BridierIndex] = BRIDIER_INITIAL_INDEX;
    pvColVal[SystemEmpireData::LastBridierActivity] = tTime;

    pvColVal[SystemEmpireData::PrivateEmail] = "";
    pvColVal[SystemEmpireData::Location] = "";
    pvColVal[SystemEmpireData::IMId] = "";
    pvColVal[SystemEmpireData::GameRatios] = RATIOS_DISPLAY_ON_RELEVANT_SCREENS;

    // Generate a secret key for the empire
    int64 i64SecretKey = 0;
    iErrCode = Crypto::GetRandomData ((Byte*) &i64SecretKey, sizeof (i64SecretKey));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    pvColVal[SystemEmpireData::SecretKey] = i64SecretKey;

    pvColVal[SystemEmpireData::Options2] = 0;

    SafeRelease (pTable);

    iErrCode = m_pGameData->InsertRow (SYSTEM_EMPIRE_DATA, pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Return value
    *piEmpireKey = iKey;

/*
    // Create a system messages table for the new empire
    GET_SYSTEM_EMPIRE_MESSAGES (pszTable, iKey);
    iErrCode = m_pGameData->CreateTable (pszTable, SystemEmpireMessages::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Create "SystemEmpireActiveGames(I)" table
    GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iKey);
    iErrCode = m_pGameData->CreateTable (pszTable, SystemEmpireActiveGames::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Create SystemEmpireNukedList(I) table
    GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iKey);
    iErrCode = m_pGameData->CreateTable (pszTable, SystemEmpireNukeList::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Create SystemEmpireNukerList(I) table
    GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iKey);
    iErrCode = m_pGameData->CreateTable (pszTable, SystemEmpireNukeList::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
*/

    // Add to top lists
    ENUMERATE_SCORING_SYSTEMS (i) {

        ScoringSystem ss = (ScoringSystem) i;
        if (HasTopList (ss)) {

            iErrCode = UpdateTopListOnIncrease (ss, iKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    if (iParentKey != NO_KEY) {
        
        // This should succeed, but it's not fatal
        int iErrCode2 = RemoveEmpire (iParentKey);
        Assert (iErrCode2 == OK);
    }

    // Notification
    if (m_pUIEventSink != NULL) {
        m_pUIEventSink->OnCreateEmpire (iKey);
    }

Cleanup:

    SafeRelease (pTable);

    if (iErrCode != OK && *piEmpireKey != NO_KEY) {

        // Best effort delete the new empire's database stuff
        m_pGameData->DeleteRow (SYSTEM_EMPIRE_DATA, iKey);
/*      
        GET_SYSTEM_EMPIRE_MESSAGES (pszTable, iKey)
        m_pGameData->DeleteTable (pszTable);

        GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iKey)
        m_pGameData->DeleteTable (pszTable);

        GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iKey)
        m_pGameData->DeleteTable (pszTable);

        GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iKey)
        m_pGameData->DeleteTable (pszTable);
*/
        ENUMERATE_SCORING_SYSTEMS (i) {

            ScoringSystem ss = (ScoringSystem) i;
            if (HasTopList (ss)) {
                int iErrCode2 = UpdateTopListOnDeletion (ss, iKey);
                Assert (iErrCode2 == OK);
            }
        }
    }

    UnlockEmpires();

    if (bParentLocked) {
        UnlockEmpire (nmParentMutex);
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

int GameEngine::GetEmpireName (int iEmpireKey, Variant* pvName) {

    bool bExists;
    if (m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iEmpireKey, &bExists) != OK || !bExists) {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Name, pvName);
}

int GameEngine::GetEmpireName (int iEmpireKey, char pszName [MAX_EMPIRE_NAME_LENGTH + 1]) {

    int iErrCode;
    IReadTable* pTable = NULL;

    const char* pszDataName;

    iErrCode = m_pGameData->GetTableForReading (SYSTEM_EMPIRE_DATA, &pTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (iEmpireKey, SystemEmpireData::Name, &pszDataName);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    strcpy (pszName, pszDataName);

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

    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Name, pszName);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> New password
//
// Update an empire's password via admin interface

int GameEngine::SetEmpirePassword (int iEmpireKey, const char* pszPassword) {

    if (iEmpireKey == ROOT_KEY) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, pszPassword);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> New password
//
// Update an empire's password from ProfileEditor

int GameEngine::ChangeEmpirePassword (int iEmpireKey, const char* pszPassword) {

    if (iEmpireKey == GUEST_KEY) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, pszPassword);
}


// Input:
// iEmpireKey -> Integer key of empire
// iBackgroundKey -> New key
//
// Update an empire's background key

int GameEngine::SetEmpireBackgroundKey (int iEmpireKey, int iBackgroundKey) {
    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIBackground, iBackgroundKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iLivePlanetKey -> New key
//
// Update an empire's live planet key

int GameEngine::SetEmpireLivePlanetKey (int iEmpireKey, int iLivePlanetKey) {
    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UILivePlanet, iLivePlanetKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iDeadPlanetKey -> New key
//
// Update an empire's dead planet key

int GameEngine::SetEmpireDeadPlanetKey (int iEmpireKey, int iDeadPlanetKey) {
    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIDeadPlanet, iDeadPlanetKey);
}

// Input:
// iEmpireKey -> Integer key of empire
// iButtonKey -> New key
//
// Update an empire's background key

int GameEngine::SetEmpireButtonKey (int iEmpireKey, int iButtonKey) {
    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIButtons, iButtonKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iSeparatorKey -> New key
//
// Update an empire's separator key

int GameEngine::SetEmpireSeparatorKey (int iEmpireKey, int iSeparatorKey) {
    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UISeparator, iSeparatorKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iHorzKey -> New key
//
// Update an empire's horizontal key

int GameEngine::SetEmpireHorzKey (int iEmpireKey, int iHorzKey) {
    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIHorz, iHorzKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iVertKey -> New key
//
// Update an empire's vertical key

int GameEngine::SetEmpireVertKey (int iEmpireKey, int iVertKey) {
    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIVert, iVertKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iColorKey -> New color key
//
// Update an empire's color key

int GameEngine::SetEmpireColorKey (int iEmpireKey, int iColorKey) {
    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIColor, iColorKey);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piLivePlanetKey -> Live planet key
//
// Get an empire's live planet key

int GameEngine::GetEmpireLivePlanetKey (int iEmpireKey, int* piLivePlanetKey) {

    Variant vTemp;
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UILivePlanet, &vTemp);
    
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

int GameEngine::GetEmpireDeadPlanetKey (int iEmpireKey, int* piDeadPlanetKey) {

    Variant vTemp;
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIDeadPlanet, &vTemp);
    
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

int GameEngine::SetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int iMaxNumSavedMessages) {
    
    int iErrCode;

    unsigned int iNumMessages, * piKey = NULL;
    Variant* pvTimeStamp = NULL, vMaxNum;
    
    SYSTEM_EMPIRE_MESSAGES (strSystemEmpireMessages, iEmpireKey);

    // Lock message table
    NamedMutex nmMutex;
    LockEmpireSystemMessages (iEmpireKey, &nmMutex);
    
    // Get num messages and current max number of messages
    if (!m_pGameData->DoesTableExist (strSystemEmpireMessages)) {
        iNumMessages = 0;
    } else {

        iErrCode = m_pGameData->GetNumRows (strSystemEmpireMessages, &iNumMessages);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::MaxNumSystemMessages, 
        &vMaxNum
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Set the max number of messages
    iErrCode = m_pGameData->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::MaxNumSystemMessages, 
        iMaxNumSavedMessages
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // If we're going to be over the limit, trim the list of unread messages
    if (vMaxNum.GetInteger() > iMaxNumSavedMessages && 
        iNumMessages > (unsigned int) iMaxNumSavedMessages) {
        
        // Get the oldest messages' keys        
        iErrCode = m_pGameData->ReadColumn (
            strSystemEmpireMessages,
            SystemEmpireMessages::TimeStamp,
            &piKey,
            &pvTimeStamp,
            &iNumMessages
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        } else {

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            Assert (iNumMessages > 0);

            // Sort the messages by age
            Algorithm::QSortTwoAscending<Variant, int> (pvTimeStamp, (int*) piKey, iNumMessages);
            
            // Delete read messages until we're below the limit
            int i = 0;
            Variant vUnread;

            while (iNumMessages > (unsigned int) iMaxNumSavedMessages) {
                
                // Has message been read
                iErrCode = m_pGameData->ReadData (
                    strSystemEmpireMessages, 
                    piKey[i], 
                    SystemEmpireMessages::Unread, 
                    &vUnread
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (vUnread.GetInteger() == MESSAGE_UNREAD) {
                    continue;
                }
                    
                iErrCode = m_pGameData->DeleteRow (strSystemEmpireMessages, piKey[i]);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iNumMessages --;
                
                i ++;
            }
        }
    }
    
Cleanup:

    // Unlock the messages table
    UnlockEmpireSystemMessages (nmMutex);

    if (piKey != NULL) {
        m_pGameData->FreeKeys (piKey);
    }

    if (pvTimeStamp != NULL) {
        m_pGameData->FreeData (pvTimeStamp);
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

int GameEngine::UpdateEmpireQuote (int iEmpireKey, const char* pszQuote, bool* pbTruncated) {

    return UpdateEmpireString (iEmpireKey, SystemEmpireData::Quote, pszQuote, MAX_QUOTE_LENGTH, pbTruncated);
}

int GameEngine::UpdateEmpireVictorySneer (int iEmpireKey, const char* pszSneer, bool* pbTruncated) {

    return UpdateEmpireString (iEmpireKey, SystemEmpireData::VictorySneer, pszSneer, MAX_VICTORY_SNEER_LENGTH, pbTruncated);
}

int GameEngine::UpdateEmpireString (int iEmpireKey, int iColumn, const char* pszString, size_t stMaxLen, 
                                    bool* pbTruncated) {

    IWriteTable* pWriteTable = NULL;

    *pbTruncated = false;

    const char* pszOldString;

    int iErrCode = m_pGameData->GetTableForWriting (SYSTEM_EMPIRE_DATA, &pWriteTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pWriteTable->ReadData (iEmpireKey, iColumn, &pszOldString);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (String::StrCmp (pszString, pszOldString) != 0) {

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

        iErrCode = pWriteTable->WriteData (iEmpireKey, iColumn, pszString);

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


int GameEngine::SendVictorySneer (int iWinnerKey, const char* pszWinnerName, int iLoserKey) {

    int iErrCode;
    Variant vSneer;

    iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA,
        iWinnerKey,
        SystemEmpireData::VictorySneer,
        &vSneer
        );

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

        iErrCode = SendSystemMessage (
            iLoserKey,
            strMessage.GetCharPtr(),
            SYSTEM
            );
        
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

int GameEngine::DeleteEmpire (int iEmpireKey) {

    int iErrCode = OK;

    NamedMutex nmMutex;
    LockEmpire (iEmpireKey, &nmMutex);

    // Read active games total
    unsigned int iNumGames = 0;
    SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszGames)) {

        iErrCode = m_pGameData->GetNumRows (pszGames, &iNumGames);
        if (iErrCode != OK) {
            iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
            goto Cleanup;
        }
    }

    // Check for presence of empire in active games
    if (iNumGames > 0) {
        
        // Mark empire ready for deletion
        iErrCode = m_pGameData->WriteOr (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey, 
            SystemEmpireData::Options, 
            EMPIRE_MARKED_FOR_DELETION
            );
        
        if (iErrCode == OK) {
            iErrCode = ERROR_EMPIRE_IS_IN_GAMES;
        }
        
    } else {        
        
        // Delete empire now!
        LockEmpires();
        iErrCode = RemoveEmpire (iEmpireKey);
        UnlockEmpires();
    }

Cleanup:

    UnlockEmpire (nmMutex);

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
    
    Variant vTemp;
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxNumSystemMessages, &vTemp);
    
    if (iErrCode == OK) {
        *piMaxNumSavedMessages = vTemp.GetInteger();
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
//
// Delete an empire and "ruin" it out of all games

int GameEngine::ObliterateEmpire (int iEmpireKey, int iKillerEmpire) {

    int iErrCode;

    if (iEmpireKey == ROOT_KEY) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    if (iEmpireKey == GUEST_KEY) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    if (iKillerEmpire == NO_KEY) {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    // Read active games total
    unsigned int iNumGames; 
    Variant* pvGame;
    
    NamedMutex nmMutex;
    LockEmpire (iEmpireKey, &nmMutex);

    SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszGames)) {

        iErrCode = m_pGameData->ReadColumn (
            pszGames, 
            SystemEmpireActiveGames::GameClassGameNumber,
            &pvGame,
            &iNumGames
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        } else {

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Ruin empire out of each game
            bool bFlag;
            unsigned int i;
            int iGameClass, iGameNumber;
            
            char strGameData [128];

            for (i = 0; i < iNumGames; i ++) {
                
                GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);
                
                GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

                // Pretend we're an update
                iErrCode = WaitGameWriter (iGameClass, iGameNumber);
                if (iErrCode != OK) {
                    continue;   // Game must be dead
                }

                // Is empire in the game
                iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
                if (iErrCode != OK) {
                    Assert (false);
                    goto EndGame;
                }
                
                if (bFlag) {
                    
                    // Try to quit the empire from the game nicely
                    iErrCode = QuitEmpireFromGameInternal (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
                    if (iErrCode != OK) {
                        
                        // The empire couldn't be removed nicely,
                        // so let's delete the empire the hard way
                        iErrCode = RemoveEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto EndGame;
                        }
                    }
                }
    EndGame:
                // Always try to unlock game
                SignalGameWriter (iGameClass, iGameNumber);
            }
            
            FreeData (pvGame);
        }
    }

    // Delete empire from server now
    LockEmpires();
    iErrCode = RemoveEmpire (iEmpireKey);
    UnlockEmpires();

Cleanup:

    UnlockEmpire (nmMutex);

    return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire to be deleted

int GameEngine::RemoveEmpire (int iEmpireKey) {

    int iErrCode = OK, iErrCode2;
    char pszTable [256];

    // Delete empire's SystemEmpireActiveGames(I) table
    GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszTable)) {
        iErrCode2 = m_pGameData->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Best effort delete / halt all personal gameclasses
    int* piGameClassKey;
    unsigned int i, iNumKeys;
    
    iErrCode2 = GetEmpirePersonalGameClasses (iEmpireKey, &piGameClassKey, NULL, (int*) &iNumKeys);
    if (iErrCode2 == OK && iNumKeys > 0) {
        
        bool bDelete;
        for (i = 0; i < iNumKeys; i ++) {
            
            iErrCode2 = DeleteGameClass (piGameClassKey[i], &bDelete);
            Assert (iErrCode2 == OK);
            
            if (iErrCode2 == OK && !bDelete) {

                iErrCode = m_pGameData->WriteData (
                    SYSTEM_GAMECLASS_DATA,
                    piGameClassKey[i],
                    SystemGameClassData::Owner,
                    DELETED_EMPIRE_KEY
                    );
                Assert (iErrCode == OK);
            }
        }
        
        m_pGameData->FreeKeys ((unsigned int*) piGameClassKey);
    }

    // Best effort delete nuked table
    GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszTable)) {
        iErrCode2 = m_pGameData->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Best effort delete nuker table
    GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszTable)) {
        iErrCode2 = m_pGameData->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Delete tournaments owned by empire or mark them for deletion
    unsigned int* piTournamentDelKey = NULL, iNumDelTournaments;

    iErrCode = m_pGameData->GetEqualKeys (
        SYSTEM_TOURNAMENTS,
        SystemTournaments::Owner,
        iEmpireKey,
        false,
        &piTournamentDelKey,
        &iNumDelTournaments
        );

    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        for (i = 0; i < iNumDelTournaments; i ++) {

            iErrCode = DeleteTournament (iEmpireKey, piTournamentDelKey[i], true);
            Assert (iErrCode == OK || iErrCode == ERROR_TOURNAMENT_HAS_GAMES);
        }

        m_pGameData->FreeKeys (piTournamentDelKey);
    }

    // Delete empire from tournaments it was in
    GET_SYSTEM_EMPIRE_TOURNAMENTS (pszTable, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszTable)) {

        unsigned int iKey = NO_KEY;
        Variant vKey;

        while (true) {

            iErrCode = m_pGameData->GetNextKey (pszTable, iKey, &iKey);
            if (iErrCode != OK) {
                Assert (iErrCode == ERROR_DATA_NOT_FOUND);
                iErrCode = OK;
                break;
            }

            iErrCode = m_pGameData->ReadData (pszTable, iKey, SystemEmpireTournaments::TournamentKey, &vKey);
            if (iErrCode == OK) {
                iErrCode = DeleteEmpireFromTournament (vKey.GetInteger(), iEmpireKey);
                Assert (iErrCode == OK);
            }
        }

        iErrCode = m_pGameData->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Best effort delete messages table
    GET_SYSTEM_EMPIRE_MESSAGES (pszTable, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszTable)) {
        iErrCode2 = m_pGameData->DeleteTable (pszTable);
        Assert (iErrCode2 == OK);
    }

    // Delete row from SystemEmpireData table
    iErrCode = m_pGameData->DeleteRow (SYSTEM_EMPIRE_DATA, iEmpireKey);
    Assert (iErrCode == OK);

    // Notification
    if (m_pUIEventSink != NULL) {
        m_pUIEventSink->OnDeleteEmpire (iEmpireKey);
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
                                 Variant* pvEmpireName) {

    int iErrCode = OK;

    *pbExists = false;
    *piEmpireKey = NO_KEY;

    iErrCode = m_pGameData->GetFirstKey (
        SYSTEM_EMPIRE_DATA, 
        SystemEmpireData::Name, 
        pszName, 
        true, 
        (unsigned int*) piEmpireKey
        );

    // Fetch proper capitalization
    if (iErrCode == OK) {

        Assert (*piEmpireKey != NO_KEY);
        
        *pbExists = true;
        
        if (pvEmpireName != NULL) {

            iErrCode = m_pGameData->ReadData (
                SYSTEM_EMPIRE_DATA, 
                *piEmpireKey, 
                SystemEmpireData::Name, 
                pvEmpireName
                );
        }
    }

    else if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

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
    IReadTable* pEmps = NULL;

    iErrCode = m_pGameData->GetTableForReading (SYSTEM_EMPIRE_DATA, &pEmps);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pEmps->DoesRowExist (iEmpireKey, pbExists);
    if (iErrCode == OK) {

        if (pvEmpireName != NULL && *pbExists) {
            iErrCode = pEmps->ReadData (iEmpireKey, SystemEmpireData::Name, pvEmpireName);
        }
    }

    pEmps->Release();

    return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *pbMatch-> true if key matches name
//
// Determines if a given empire key matches a given empire name

int GameEngine::DoesEmpireKeyMatchName (int iEmpireKey, const char* pszEmpireName, bool* pbMatch) {

    int iErrCode;

    // Check row
    iErrCode = m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iEmpireKey, pbMatch);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (*pbMatch) {
        
        Variant vEmpireName;
        
        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Name, &vEmpireName);
        if (iErrCode != OK) {
            return iErrCode;
        }

        *pbMatch = (String::StriCmp (pszEmpireName, vEmpireName.GetCharPtr()) == 0);
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> Password to be tested
//
// Given an empire name and a password, determines if that password is correct for that empire
// Return OK if yes, an error if no

int GameEngine::IsPasswordCorrect (int iEmpireKey, const char* pszPassword) {

    int iErrCode;
    Variant vPassword;

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, &vPassword);
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

    // Make sure logins are allowed
    int iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::Options, &vLogins);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Get the time
    UTCTime tTime;
    Time::GetTime (&tTime);

    NamedMutex nmMutex;
    LockEmpire (iEmpireKey, &nmMutex);

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, &vPriv);
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

    if (String::StrLen (pszBrowser) > MAX_BROWSER_NAME_LENGTH) {
        
        char pszCutBrowser [MAX_BROWSER_NAME_LENGTH + 1];
        memcpy (pszCutBrowser, pszBrowser, MAX_BROWSER_NAME_LENGTH);
        pszCutBrowser [MAX_BROWSER_NAME_LENGTH] = '\0';

        iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Browser, pszCutBrowser);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

    } else {
        
        iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Browser, pszBrowser);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Write IP address
    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::IPAddress, pszIPAddress);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Write LastLoginTime
    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::LastLoginTime, tTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Increment the number of logins
    iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::NumLogins, 1);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    UnlockEmpire (nmMutex);

    // Outside the lock...  if we're an admin, best effort test all closed games for an update
    if (iErrCode == OK) {
        
        // Notification
        if (m_pUIEventSink != NULL) {
            m_pUIEventSink->OnLoginEmpire (iEmpireKey);
        }
        
        if (vPriv.GetInteger() == ADMINISTRATOR) {
            SendLongRunningQueryMessage (CheckAllGamesForUpdatesMsg, NULL);
        }
    }

    return iErrCode;
}


int GameEngine::CheckAllGamesForUpdatesMsg (LongRunningQueryMessage* pMessage) {

    return pMessage->pGameEngine->CheckAllGamesForUpdates();
}

// Output:
// *piNumEmpires -> Number of empires
//
// Returns the number of empires currently registered on the server

int GameEngine::GetNumEmpiresOnServer (int* piNumEmpires) {

    return m_pGameData->GetNumRows (SYSTEM_EMPIRE_DATA, (unsigned int*) piNumEmpires);
}

// Input:
// iEmpireKey -> Empire Key
//
// Undeletes an empire marked for deletion

int GameEngine::UndeleteEmpire (int iEmpireKey) {

    Variant vTemp;

    NamedMutex nmEmpireMutex;
    LockEmpire (iEmpireKey, &nmEmpireMutex);

    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vTemp);
    if (iErrCode == OK && vTemp.GetInteger() & EMPIRE_MARKED_FOR_DELETION) {

        iErrCode = m_pGameData->WriteAnd (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey, 
            SystemEmpireData::Options, 
            ~EMPIRE_MARKED_FOR_DELETION
            );

    } else {
        iErrCode = ERROR_CANNOT_UNDELETE_EMPIRE;
    }

    UnlockEmpire (nmEmpireMutex);

    return iErrCode;
}

// Input:
// iEmpireKey -> Integer key of empire
//
// Sets all of an empire's statistics to their default values

int GameEngine::BlankEmpireStatistics (int iEmpireKey) {

    int iErrCode;

    char pszTable [256];
    Variant vOldClassicScore, vOldAlmonasterScore, vOldBridierIndex;

    if (iEmpireKey == ROOT_KEY) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    if (iEmpireKey == GUEST_KEY) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    NamedMutex nmEmpireMutex, nmBridierMutex;
    LockEmpire (iEmpireKey, &nmEmpireMutex);

    // Get old scores
    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::ClassicScore, &vOldClassicScore);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vOldAlmonasterScore);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    //
    // Blank Bridier Score
    //

    // Lock
    LockEmpireBridier (iEmpireKey, &nmBridierMutex);

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierIndex, &vOldBridierIndex);
    if (iErrCode != OK) {
        UnlockEmpireBridier (nmBridierMutex);
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierRank, BRIDIER_INITIAL_RANK);
    if (iErrCode != OK) {
        UnlockEmpireBridier (nmBridierMutex);
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierIndex, BRIDIER_INITIAL_INDEX);
    
    // Unlock
    UnlockEmpireBridier (nmBridierMutex);

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Blank statistics
    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Wins, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Nukes, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Nuked, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Draws, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Ruins, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxEcon, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxMil, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Blank Classic Score
    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::ClassicScore, CLASSIC_INITIAL_SCORE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Blank Almonaster Score, Significance and privilege level
    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, ALMONASTER_INITIAL_SCORE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScoreSignificance, 0);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, NOVICE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Blank nuke history fields
    GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszTable)) {

        iErrCode = m_pGameData->DeleteAllRows (pszTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszTable)) {

        iErrCode = m_pGameData->DeleteAllRows (pszTable);
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

    UnlockEmpire (nmEmpireMutex);

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

        iErrCode = m_pGameData->GetEqualKeys (
            SYSTEM_GAMECLASS_DATA,
            SystemGameClassData::Owner,
            iEmpireKey,
            false,
            (unsigned int**) ppiGameClassKey,
            (unsigned int*) piNumKeys
            );
    
    } else {

        iErrCode = m_pGameData->ReadColumnWhereEqual (
            SYSTEM_GAMECLASS_DATA,
            SystemGameClassData::Owner,
            iEmpireKey,
            false,
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

int GameEngine::GetEmpireData (int iEmpireKey, Variant** ppvEmpData, int* piNumActiveGames) {

    SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey);

    *ppvEmpData = NULL;
    *piNumActiveGames = 0;

    // Get num active games
    if (m_pGameData->DoesTableExist (pszGames)) {

        int iErrCode = m_pGameData->GetNumRows (pszGames, (unsigned int*) piNumActiveGames);
        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    // Get data
    return m_pGameData->ReadRow (SYSTEM_EMPIRE_DATA, iEmpireKey, ppvEmpData);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiGameClass -> Array of game classes
// **ppiGameNumber -> Array of game numbers
//
// Returns the gameclasses and gamenumbers of the games an empire is currently in

int GameEngine::GetEmpireActiveGames (int iEmpireKey, int** ppiGameClass, int** ppiGameNumber, 
                                      int* piNumGames) {

    int iErrCode = OK;

    IReadTable* pGames = NULL;
    char** ppszGames = NULL;

    *ppiGameClass = NULL;
    *ppiGameNumber = NULL;
    *piNumGames = 0;

    SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey);

    if (m_pGameData->DoesTableExist (pszGames)) {

        iErrCode = m_pGameData->GetTableForReading (pszGames, &pGames);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pGames->ReadColumn (
            SystemEmpireActiveGames::GameClassGameNumber,
            &ppszGames,
            (unsigned int*) piNumGames
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }

        else if (iErrCode == OK) {

            *ppiGameClass = new int [*piNumGames];
            if (*ppiGameClass == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }

            *ppiGameNumber = new int [*piNumGames];
            if (*ppiGameNumber == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                delete [] (*ppiGameClass);
                *ppiGameClass = NULL;
                goto Cleanup;
            }
            
            for (int i = 0; i < *piNumGames; i ++) {
                GetGameClassGameNumber (ppszGames[i], &((*ppiGameClass)[i]), &((*ppiGameNumber)[i]));
            }
        }

        else Assert (false);
    }

Cleanup:

    SafeRelease (pGames);

    if (ppszGames != NULL) {
        m_pGameData->FreeData (ppszGames);
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
    iErrCode = m_pGameData->GetFirstKey (
        pszEmpires, 
        GameEmpires::EmpireKey, 
        iEmpireKey, 
        false, 
        &iKey
        );
    
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

int GameEngine::GetEmpirePrivilege (int iEmpireKey, int* piPrivilege) {

    Variant vPrivilege;
    
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, &vPrivilege);
    
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

int GameEngine::SetEmpirePrivilege (int iEmpireKey, int iPrivilege) {

    if (iEmpireKey == ROOT_KEY) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    if (iEmpireKey == GUEST_KEY) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    if (!IsLegalPrivilege (iPrivilege)) {
        return ERROR_INVALID_PRIVILEGE;
    }

    int iErrCode = m_pGameData->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::Privilege, 
        iPrivilege
        );
    
    if (iErrCode == OK) {

        char pszMessage [1024];
        sprintf (
            pszMessage, 
            "Your privilege level was changed to %s by an administrator", 
            PRIVILEGE_STRING[iPrivilege]
            );

        // Send a message to the affected empire
        iErrCode = SendSystemMessage (
            iEmpireKey,
            pszMessage,
            SYSTEM
            );
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

int GameEngine::GetEmpireAlmonasterScore (int iEmpireKey, float* pfAlmonasterScore) {

    Variant vAlmonasterScore;
    
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vAlmonasterScore);
    
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

int GameEngine::SetEmpireAlmonasterScore (int iEmpireKey, float fAlmonasterScore) {

    if (iEmpireKey == ROOT_KEY) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    if (iEmpireKey == GUEST_KEY) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    if (fAlmonasterScore < ALMONASTER_MIN_SCORE || fAlmonasterScore > ALMONASTER_MAX_SCORE) {
        return ERROR_INVALID_ARGUMENT;
    }
    
    Variant vOldScore;
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vOldScore);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (fAlmonasterScore == vOldScore.GetFloat()) {
        return OK;
    }

    iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, fAlmonasterScore);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = CalculatePrivilegeLevel (iEmpireKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (fAlmonasterScore > vOldScore.GetFloat()) {
        iErrCode = UpdateTopListOnIncrease (ALMONASTER_SCORE, iEmpireKey);
    } else {
        iErrCode = UpdateTopListOnDecrease (ALMONASTER_SCORE, iEmpireKey);
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

int GameEngine::GetEmpirePassword (int iEmpireKey, Variant* pvPassword) {
    return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Password, pvPassword);
}


// Input:
// iEmpireKey -> Key of empire
// strColumn -> Name of column
//
// Output:
// *pvData -> Data column
//
// Get empire data column from SystemEmpireData

int GameEngine::GetEmpireDataColumn (int iEmpireKey, unsigned int iColumn, Variant* pvData) {
    return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, iColumn, pvData);
}


// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *piNumLogins -> Number of successful logins
//
// Get the number of successful logins performed by the empire

int GameEngine::GetNumLogins (int iEmpireKey, int* piNumLogins) {

    Variant vTemp;
    int iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::NumLogins, &vTemp);
    
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

int GameEngine::GetEmpireIPAddress (int iEmpireKey, Variant* pvIPAddress) {

    return m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::IPAddress,
        pvIPAddress
        );
}


// Input:
// iEmpireKey -> Key of empire
// pszIPAddress -> New IP address
//
// Set an empire's IP address

int GameEngine::SetEmpireIPAddress (int iEmpireKey, const char* pszIPAddress) {

    return m_pGameData->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::IPAddress,
        pszIPAddress
        );
}

int GameEngine::GetEmpireSessionId (int iEmpireKey, int64* pi64SessionId) {

    int iErrCode;
    IReadTable* pSystemEmpireData;

    iErrCode = m_pGameData->GetTableForReading (SYSTEM_EMPIRE_DATA, &pSystemEmpireData);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pSystemEmpireData->ReadData (iEmpireKey, SystemEmpireData::SessionId, pi64SessionId);

    pSystemEmpireData->Release();

    return iErrCode;
}

int GameEngine::SetEmpireSessionId (int iEmpireKey, int64 i64SessionId) {

    return m_pGameData->WriteData (
        SYSTEM_EMPIRE_DATA,
        iEmpireKey,
        SystemEmpireData::SessionId,
        i64SessionId
        );
}


int GameEngine::GetEmpireDefaultBuilderPlanet (int iEmpireKey, int* piDefaultBuildPlanet) {

    Variant vValue;
    int iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::DefaultBuilderPlanet,
        &vValue
        );

    if (iErrCode == OK) {
        *piDefaultBuildPlanet = vValue.GetInteger();
    }

    return iErrCode;
}


int GameEngine::SetEmpireDefaultBuilderPlanet (int iEmpireKey, int iDefaultBuildPlanet) {

    if (iDefaultBuildPlanet > NO_DEFAULT_BUILDER_PLANET ||
        iDefaultBuildPlanet < LAST_BUILDER_DEFAULT_BUILDER_PLANET) {
        return ERROR_INVALID_ARGUMENT;
    }

    return m_pGameData->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::DefaultBuilderPlanet,
        iDefaultBuildPlanet
        );
}


int GameEngine::GetEmpireDefaultMessageTarget (int iEmpireKey, int* piMessageTarget) {

    Variant vData;
    
    int iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::DefaultMessageTarget, 
        &vData
        );

    if (iErrCode == OK) {
        *piMessageTarget = vData.GetInteger();
    }

    return iErrCode;
}


int GameEngine::SetEmpireDefaultMessageTarget (int iEmpireKey, int iMessageTarget) {

    if (iMessageTarget < MESSAGE_TARGET_NONE || iMessageTarget > MESSAGE_TARGET_LAST_USED) {
        return ERROR_INVALID_ARGUMENT;
    }

    return m_pGameData->WriteData (
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

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vTemp);
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

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options2, &vTemp);
    if (iErrCode == OK) {
        *piOptions = vTemp.GetInteger();
    }

    return iErrCode;
}


int GameEngine::GetEmpireLastBridierActivity (int iEmpireKey, UTCTime* ptTime) {

    int iErrCode;
    Variant vTemp;

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::LastBridierActivity, &vTemp);
    if (iErrCode == OK) {
        *ptTime = vTemp.GetUTCTime();
    }

    return iErrCode;
}

int GameEngine::GetEmpireProperty (int iEmpireKey, unsigned int iProperty, Variant* pvProperty) {

    return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, iProperty, pvProperty);
}

int GameEngine::SetEmpireProperty (int iEmpireKey, unsigned int iProperty, const Variant& vProperty) {

    return m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, iProperty, vProperty);
}

int GameEngine::IsEmpireIdleInSomeGame (int iEmpireKey, bool* pfIdle) {

    int iErrCode, iGameClass, iGameNumber;
    unsigned int i, iNumGames = 0;
    Variant* pvGame = NULL, vNumUpdatesIdle;

    char pszGameData [256];

    SYSTEM_EMPIRE_ACTIVE_GAMES (pszGames, iEmpireKey);

    *pfIdle = false;

    iErrCode = m_pGameData->ReadColumn (pszGames, SystemEmpireActiveGames::GameClassGameNumber, &pvGame, &iNumGames);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            goto Cleanup;
        }
    }

    for (i = 0; i < iNumGames; i ++) {

        GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);
        GET_GAME_EMPIRE_DATA (pszGameData, iGameClass, iGameNumber, iEmpireKey);

        iErrCode = m_pGameData->ReadData (pszGameData, GameEmpireData::NumUpdatesIdle, &vNumUpdatesIdle);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (vNumUpdatesIdle.GetInteger() > 0) {
            *pfIdle = true;
            goto Cleanup;
        }
    }

Cleanup:

    if (pvGame != NULL) {
        m_pGameData->FreeData (pvGame);
    }

    return iErrCode;
}