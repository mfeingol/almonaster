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

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    Variant vOptions;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    *pbOption = (vOptions.GetInteger() & iFlag) != 0;
    
    return iErrCode;
}

int GameEngine::SetEmpireOption (int iEmpireKey, unsigned int iFlag, bool bOption)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    if (bOption)
    {
        return t_pCache->WriteOr(strEmpire, iEmpireKey, SystemEmpireData::Options, iFlag);
    }
    else
    {
        return t_pCache->WriteAnd(strEmpire, iEmpireKey, SystemEmpireData::Options, ~iFlag);
    }
}

int GameEngine::GetEmpireOption2 (int iEmpireKey, unsigned int iFlag, bool* pbOption) {

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    Variant vOptions;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Options2, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    *pbOption = (vOptions.GetInteger() & iFlag) != 0;
    
    return iErrCode;
}

int GameEngine::SetEmpireOption2 (int iEmpireKey, unsigned int iFlag, bool bOption)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);
    if (bOption)
    {
        return t_pCache->WriteOr(strEmpires, iEmpireKey, SystemEmpireData::Options2, iFlag);
    }
    else
    {
        return t_pCache->WriteAnd(strEmpires, iEmpireKey, SystemEmpireData::Options2, ~iFlag);
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

int GameEngine::CreateEmpire(const char* pszEmpireName, const char* pszPassword, int iPrivilege,
                             unsigned int iParentKey, bool bBypassDisabled, unsigned int* piEmpireKey)
{
    unsigned int i, iKey = NO_KEY;

    int64 i64SecretKey = 0;

    int iOptions = 0, iAlmonasterScoreSignificance, iErrCode;
    float fScore;
    Variant vTemp, pvColVal[SystemEmpireData::NumColumns];

    ICachedTable* pSystemData = NULL;
    AutoRelease<ICachedTable> releaseSystem(pSystemData);

    ICachedTable* pNewEmpire = NULL;
    AutoRelease<ICachedTable> releaseEmpire(pNewEmpire);

    *piEmpireKey = NO_KEY;

    // Make sure empire creation is enabled
    if (!bBypassDisabled)
    {
        iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::Options, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if (!(vTemp.GetInteger() & NEW_EMPIRES_ENABLED))
            return ERROR_DISABLED;
    }

    // Make sure name is not reserved
    for (i = 0; i < NUM_RESERVED_EMPIRE_NAMES; i ++)
    {
        if (String::StriCmp(pszEmpireName, RESERVED_EMPIRE_NAMES[i]) == 0)
            return ERROR_RESERVED_EMPIRE_NAME;
    }

    // Get current time
    UTCTime tTime;
    Time::GetTime(&tTime);

    // Deal with empire inheritance
    if (iParentKey == NO_KEY)
    {
        fScore = ALMONASTER_INITIAL_SCORE;
        iAlmonasterScoreSignificance = 0;

        iOptions |= CAN_BROADCAST;
    }
    else
    {
        iErrCode = CacheEmpire(iParentKey);
        RETURN_ON_ERROR(iErrCode);

        // Read parent empire's secret key
        GET_SYSTEM_EMPIRE_DATA(strParentEmpire, iParentKey);
        iErrCode = t_pCache->ReadData(strParentEmpire, iParentKey, SystemEmpireData::SecretKey, &vTemp);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
            return ERROR_EMPIRE_DOES_NOT_EXIST;
        RETURN_ON_ERROR(iErrCode);
        
        i64SecretKey = vTemp.GetInteger64();

        // Make sure we can delete the parent empire
        unsigned int iNumGames;
        iErrCode = GetEmpireActiveGames(iParentKey, NULL, &iNumGames);
        RETURN_ON_ERROR(iErrCode);

        if (iNumGames > 0)
        {
            return ERROR_COULD_NOT_DELETE_EMPIRE;
        }

        // Inherit privilege
        iErrCode = t_pCache->ReadData(strParentEmpire, iParentKey, SystemEmpireData::Privilege, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        iPrivilege = vTemp.GetInteger();
        if (iPrivilege == ADMINISTRATOR)
        {
            return ERROR_COULD_NOT_DELETE_ADMINISTRATOR;
        }

        // Inherit Almonaster score
        iErrCode = t_pCache->ReadData(strParentEmpire, iParentKey, SystemEmpireData::AlmonasterScore, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        fScore = vTemp.GetFloat();

        // Inherit Almonaster score significance
        iErrCode = t_pCache->ReadData(strParentEmpire, iParentKey, SystemEmpireData::AlmonasterScoreSignificance, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iAlmonasterScoreSignificance = vTemp.GetInteger();

        // Propagate broadcast flag
        iErrCode = t_pCache->ReadData(strParentEmpire, iParentKey, SystemEmpireData::Options, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if (vTemp.GetInteger() & CAN_BROADCAST)
        {
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

    // Generate a random secret key for the empire
    i64SecretKey = 0;
    iErrCode = Crypto::GetRandomData(&i64SecretKey, sizeof(i64SecretKey));
    RETURN_ON_ERROR(iErrCode);

    pvColVal[SystemEmpireData::iName] = pszEmpireName;

    iErrCode = ComputePasswordHash(pszPassword,  pvColVal + SystemEmpireData::iPasswordHash);
    RETURN_ON_ERROR(iErrCode);

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
    pvColVal[SystemEmpireData::iIPAddress] = (const char*)NULL;
    pvColVal[SystemEmpireData::iRuins] = 0;
    pvColVal[SystemEmpireData::iClassicScore] = (float) 0.0;
    pvColVal[SystemEmpireData::iAlmonasterScore] = fScore;
    pvColVal[SystemEmpireData::iAlmonasterTheme] = INDIVIDUAL_ELEMENTS;
    pvColVal[SystemEmpireData::iAlternativeGraphicsPath] = (const char*)NULL;
    pvColVal[SystemEmpireData::iSecretKey] = i64SecretKey;
    pvColVal[SystemEmpireData::iOptions2] = 0;
    pvColVal[SystemEmpireData::iGender] = EMPIRE_GENDER_UNKNOWN;
    pvColVal[SystemEmpireData::iAge] = EMPIRE_AGE_UNKNOWN;
    pvColVal[SystemEmpireData::iCustomTableColor] = "000000";
    pvColVal[SystemEmpireData::iOptions] = iOptions;
    pvColVal[SystemEmpireData::iMaxNumShipsBuiltAtOnce] = 10;
    pvColVal[SystemEmpireData::iCreationTime] = tTime;
    pvColVal[SystemEmpireData::iNumLogins] = 0;
    pvColVal[SystemEmpireData::iBrowser] = (const char*)NULL;
    pvColVal[SystemEmpireData::iCustomTextColor] = "000000";
    pvColVal[SystemEmpireData::iCustomGoodColor] = "000000";
    pvColVal[SystemEmpireData::iCustomBadColor] = "000000";
    pvColVal[SystemEmpireData::iCustomPrivateMessageColor] = "000000";
    pvColVal[SystemEmpireData::iCustomBroadcastMessageColor] = "000000";
    pvColVal[SystemEmpireData::iSessionId] = NO_SESSION_ID;
    pvColVal[SystemEmpireData::iDefaultBuilderPlanet] = HOMEWORLD_DEFAULT_BUILDER_PLANET;
    pvColVal[SystemEmpireData::iDefaultMessageTarget] = MESSAGE_TARGET_BROADCAST;
    pvColVal[SystemEmpireData::iAlmonasterScoreSignificance] = iAlmonasterScoreSignificance;
    pvColVal[SystemEmpireData::iVictorySneer] = (const char*)NULL;
    pvColVal[SystemEmpireData::iBridierRank] = BRIDIER_INITIAL_RANK;
    pvColVal[SystemEmpireData::iBridierIndex] = BRIDIER_INITIAL_INDEX;
    pvColVal[SystemEmpireData::iLastBridierActivity] = tTime;
    pvColVal[SystemEmpireData::iPrivateEmail] = (const char*)NULL;
    pvColVal[SystemEmpireData::iLocation] = (const char*)NULL;
    pvColVal[SystemEmpireData::iIMId] = (const char*)NULL;
    pvColVal[SystemEmpireData::iGameRatios] = RATIOS_DISPLAY_ON_RELEVANT_SCREENS;

    Assert(pvColVal[SystemEmpireData::iName].GetCharPtr() &&
           pvColVal[SystemEmpireData::iPasswordHash].GetCharPtr() &&
           pvColVal[SystemEmpireData::iCustomTableColor].GetCharPtr() &&
           pvColVal[SystemEmpireData::iCustomTextColor].GetCharPtr() &&
           pvColVal[SystemEmpireData::iCustomGoodColor].GetCharPtr() &&
           pvColVal[SystemEmpireData::iCustomBadColor].GetCharPtr() &&
           pvColVal[SystemEmpireData::iCustomPrivateMessageColor].GetCharPtr() &&
           pvColVal[SystemEmpireData::iCustomBroadcastMessageColor].GetCharPtr());

    // Read defaults from SystemData table
    iErrCode = t_pCache->GetTable(SYSTEM_DATA, &pSystemData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultAlienKey, pvColVal + SystemEmpireData::iAlienKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultAlienAddress, pvColVal + SystemEmpireData::iAlienAddress);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIIndependentPlanet, pvColVal + SystemEmpireData::iUIIndependentPlanet);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultMaxNumSystemMessages, pvColVal + SystemEmpireData::iMaxNumSystemMessages);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIButtons, pvColVal + SystemEmpireData::iUIButtons);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIBackground, pvColVal + SystemEmpireData::iUIBackground);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultUILivePlanet, pvColVal + SystemEmpireData::iUILivePlanet);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIDeadPlanet, pvColVal + SystemEmpireData::iUIDeadPlanet);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultUISeparator, pvColVal + SystemEmpireData::iUISeparator);
    RETURN_ON_ERROR(iErrCode);

    ENUMERATE_SHIP_TYPES(i)
    {
        iErrCode = pSystemData->ReadData(SYSTEM_DATA_SHIP_NAME_COLUMN[i], pvColVal + SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN_INDEX[i]);
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIHorz, pvColVal + SystemEmpireData::iUIHorz);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIVert, pvColVal + SystemEmpireData::iUIVert);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystemData->ReadData(SystemData::DefaultUIColor, pvColVal + SystemEmpireData::iUIColor);
    RETURN_ON_ERROR(iErrCode);

    // Make sure that an empire with the same name doesn't exist
    unsigned int iTestKey;
    iErrCode = LookupEmpireByName(pszEmpireName, &iTestKey, NULL, NULL, &pNewEmpire);
    RETURN_ON_ERROR(iErrCode);

    if (iTestKey != NO_KEY)
    {
        return ERROR_EMPIRE_ALREADY_EXISTS;
    }

    // Insert row into SystemEmpireData
    iErrCode = pNewEmpire->InsertRow(SystemEmpireData::Template, pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Populate the cache with the row we just inserted
    iErrCode = CacheEmpire(iKey);
    RETURN_ON_ERROR(iErrCode);

    // Delete parent empire
    if (iParentKey != NO_KEY)
    {
        iErrCode = DeleteEmpire (iParentKey, &i64SecretKey, false, false);
        RETURN_ON_ERROR(iErrCode);
    }

    // Notification
    global.GetEventSink()->OnCreateEmpire (iKey);

    // Return value
    *piEmpireKey = iKey;

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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Name, pvName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::GetEmpireName (int iEmpireKey, char pszName [MAX_EMPIRE_NAME_LENGTH + 1])
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Name, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    strcpy(pszName, vTemp.GetCharPtr());

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
// strName -> New name
//
// Update an empire's name

int GameEngine::SetEmpireName(int iEmpireKey, const char* pszName)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Name, pszName);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> New password
//
// Update an empire's password via admin interface

int GameEngine::SetEmpirePassword(unsigned int iEmpireKey, const char* pszPassword)
{
    int iErrCode;

    if (iEmpireKey == global.GetRootKey())
    {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    Variant vPasswordHash;
    iErrCode = ComputePasswordHash(pszPassword, &vPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::PasswordHash, vPasswordHash);
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> New password
//
// Update an empire's password from ProfileEditor

int GameEngine::ChangeEmpirePassword(unsigned int iEmpireKey, const char* pszPassword)
{
    int iErrCode;

    if (iEmpireKey == global.GetGuestKey())
    {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    Variant vPasswordHash;
    iErrCode = ComputePasswordHash(pszPassword, &vPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::PasswordHash, vPasswordHash);
}

int GameEngine::ComputePasswordHash(const char* pszPassword, Variant* pvPasswordHash)
{
    int iErrCode;

    Crypto::HashSHA256 hash;

    iErrCode = hash.HashData(pszPassword, strlen(pszPassword));
    RETURN_ON_ERROR(iErrCode);

    Variant vFixedSalt;
    iErrCode = GetSystemProperty(SystemData::FixedHashSalt, &vFixedSalt);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = hash.HashData(vFixedSalt.GetCharPtr(), strlen(vFixedSalt));
    RETURN_ON_ERROR(iErrCode);

    size_t cbSize;
    iErrCode = hash.GetHashSize(&cbSize);
    RETURN_ON_ERROR(iErrCode);

    void* pBuffer = StackAlloc(cbSize);
    iErrCode = hash.GetHash(pBuffer, cbSize);
    RETURN_ON_ERROR(iErrCode);

    size_t cch = Algorithm::GetEncodeBase64Size(cbSize);
    char* pszBase64 = (char*)StackAlloc(cch);
    iErrCode = Algorithm::EncodeBase64(pBuffer, cbSize, pszBase64, cch);
    RETURN_ON_ERROR(iErrCode);

    pvPasswordHash->Clear();
    *pvPasswordHash = pszBase64;
    Assert(pvPasswordHash->GetCharPtr());

    return iErrCode;
}

// Input:
// iEmpireKey -> Integer key of empire
// iBackgroundKey -> New key
//
// Update an empire's background key

int GameEngine::SetEmpireBackgroundKey (int iEmpireKey, int iBackgroundKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::UIBackground, iBackgroundKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iLivePlanetKey -> New key
//
// Update an empire's live planet key

int GameEngine::SetEmpireLivePlanetKey (int iEmpireKey, int iLivePlanetKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::UILivePlanet, iLivePlanetKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iDeadPlanetKey -> New key
//
// Update an empire's dead planet key

int GameEngine::SetEmpireDeadPlanetKey (int iEmpireKey, int iDeadPlanetKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::UIDeadPlanet, iDeadPlanetKey);
}

// Input:
// iEmpireKey -> Integer key of empire
// iButtonKey -> New key
//
// Update an empire's background key

int GameEngine::SetEmpireButtonKey (int iEmpireKey, int iButtonKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::UIButtons, iButtonKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iSeparatorKey -> New key
//
// Update an empire's separator key

int GameEngine::SetEmpireSeparatorKey (int iEmpireKey, int iSeparatorKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::UISeparator, iSeparatorKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iHorzKey -> New key
//
// Update an empire's horizontal key

int GameEngine::SetEmpireHorzKey (int iEmpireKey, int iHorzKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::UIHorz, iHorzKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iVertKey -> New key
//
// Update an empire's vertical key

int GameEngine::SetEmpireVertKey (int iEmpireKey, int iVertKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::UIVert, iVertKey);
}


// Input:
// iEmpireKey -> Integer key of empire
// iColorKey -> New color key
//
// Update an empire's color key

int GameEngine::SetEmpireColorKey (int iEmpireKey, int iColorKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::UIColor, iColorKey);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piLivePlanetKey -> Live planet key
//
// Get an empire's live planet key

int GameEngine::GetEmpireLivePlanetKey (int iEmpireKey, int* piLivePlanetKey) {

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::UILivePlanet, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    
    *piLivePlanetKey = vTemp.GetInteger();

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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::UIDeadPlanet, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piDeadPlanetKey = vTemp.GetInteger();
    
    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
// iMaxNumSavedMessages -> Number of messages to be saved
//
// Set the empire's MaxNumSavedSystemMessages parameter

int GameEngine::SetEmpireMaxNumSavedSystemMessages(int iEmpireKey, unsigned int iMaxNumSavedMessages) {
    
    int iErrCode;

    Variant vTemp;
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::MaxNumSystemMessages, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    unsigned int iMaxNum = vTemp.GetInteger();

    // Set the max number of messages
    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::MaxNumSystemMessages, (int)iMaxNumSavedMessages);
    RETURN_ON_ERROR(iErrCode);

    if (iMaxNumSavedMessages >= iMaxNum)
    {
        // Done
        return OK;
    }

    ICachedTable* pMessages = NULL;
    AutoRelease<ICachedTable> release(pMessages);

    GET_SYSTEM_EMPIRE_MESSAGES(strSystemEmpireMessages, iEmpireKey);
    iErrCode = t_pCache->GetTable(strSystemEmpireMessages, &pMessages);
    RETURN_ON_ERROR(iErrCode);

    // Get num messages
    unsigned int iNumMessages;
    iErrCode = pMessages->GetNumCachedRows(&iNumMessages);
    RETURN_ON_ERROR(iErrCode);

    Variant* pvTimeStamp = NULL;
    AutoFreeData free(pvTimeStamp);

    unsigned int* piKey = NULL;
    AutoFreeKeys freeKeys(piKey);

    // If we're going to be over the limit, trim the list of unread messages
    if (iMaxNum > iMaxNumSavedMessages && iNumMessages > iMaxNumSavedMessages)
    {
        // Get the oldest messages' keys        
        unsigned int iNumReadMessages;
        iErrCode = pMessages->ReadColumn(SystemEmpireMessages::TimeStamp, &piKey, &pvTimeStamp, &iNumReadMessages);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            // Done
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);
        Assert(iNumReadMessages == iNumMessages);

        // Sort the messages by age
        Algorithm::QSortTwoAscending<Variant, unsigned int>(pvTimeStamp, piKey, iNumReadMessages);
        
        // Delete read messages until we're below the limit
        unsigned int i, iCurrentNumMessages = iNumReadMessages;
        int iUnread;

        for (i = 0; i < iNumReadMessages && iCurrentNumMessages > iMaxNumSavedMessages; i ++)
        {
            // Has message been read?
            iErrCode = pMessages->ReadData(piKey[i], SystemEmpireMessages::Unread, &iUnread);
            RETURN_ON_ERROR(iErrCode);
            
            if (iUnread == MESSAGE_UNREAD)
            {
                continue;
            }

            iErrCode = pMessages->DeleteRow(piKey[i]);
            RETURN_ON_ERROR(iErrCode);

            iCurrentNumMessages --;
            if (iCurrentNumMessages == 0)
            {
                break;
            }
        }
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
    ICachedTable* pWriteTable = NULL;
    AutoRelease<ICachedTable> release(pWriteTable);

    *pbTruncated = false;

    Variant vTemp;

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    int iErrCode = t_pCache->GetTable(strEmpire, &pWriteTable);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pWriteTable->ReadData(iEmpireKey, pszColumn, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    if (String::StrCmp(pszString, vTemp.GetCharPtr()) == 0)
    {
        iErrCode = WARNING;
    }
    else
    {
        char* pszNew = NULL;
        Algorithm::AutoDelete<char> free_pszNew(pszNew, true);

        if (strlen(pszString) >= stMaxLen)
        {
            pszNew = new char[stMaxLen + 1];
            Assert(pszNew);
            memcpy(pszNew, pszString, stMaxLen);
            pszNew[stMaxLen] = '\0';

            *pbTruncated = true;
            pszString = pszNew;
        }

        iErrCode = pWriteTable->WriteData(iEmpireKey, pszColumn, pszString);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::SendVictorySneer(int iWinnerKey, const char* pszWinnerName, int iLoserKey)
{
    int iErrCode;
    Variant vSneer;

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iWinnerKey);
    iErrCode = t_pCache->ReadData(strEmpire, iWinnerKey, SystemEmpireData::VictorySneer, &vSneer);
    RETURN_ON_ERROR(iErrCode);

    if (!String::IsBlank (vSneer.GetCharPtr())) {

        Variant vWinnerName;
        if (pszWinnerName == NULL)
        {
            iErrCode = GetEmpireName (iWinnerKey, &vWinnerName);
            RETURN_ON_ERROR(iErrCode);
            pszWinnerName = vWinnerName.GetCharPtr();
        }

        String strMessage = "The victory sneer for " BEGIN_STRONG;
        
        strMessage += pszWinnerName;
        strMessage += END_STRONG " says:\n\n";

        strMessage.AppendHtml (vSneer.GetCharPtr(), 0, true);

        iErrCode = SendSystemMessage(iLoserKey, strMessage.GetCharPtr(), SYSTEM, MESSAGE_SYSTEM);
        if (iErrCode == ERROR_EMPIRE_DOES_NOT_EXIST)
            iErrCode = OK;
        RETURN_ON_ERROR(iErrCode);
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
    RETURN_ON_ERROR(iErrCode);
    if (!bExists)
    {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }
    
    // Verify correct empire
    if (pi64SecretKey != NULL)
    {
        GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::SecretKey, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if (vTemp.GetInteger64() != *pi64SecretKey)
        {
            return ERROR_EMPIRE_DOES_NOT_EXIST;
        }
    }

    // Read active games total
    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszGames, iEmpireKey);
    iErrCode = t_pCache->GetNumCachedRows(pszGames, &iNumGames);
    RETURN_ON_ERROR(iErrCode);

    if (iNumGames == 0)
    {
        // Delete empire now
        iErrCode = RemoveEmpire (iEmpireKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        // Mark empire for deletion?
        if (bMarkOnFailure)
        {
            iErrCode = SetEmpireOption (iEmpireKey, EMPIRE_MARKED_FOR_DELETION, true);
            RETURN_ON_ERROR(iErrCode);
        }

        if (bDeletePersonal)
        {
            GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

            static const char* s_pszPersonalInfoStringCols[] = 
            {
                SystemEmpireData::RealName,
                SystemEmpireData::Email,
                SystemEmpireData::WebPage,
                SystemEmpireData::Quote,
                SystemEmpireData::VictorySneer,
                SystemEmpireData::PrivateEmail,
                SystemEmpireData::Location,
                SystemEmpireData::IMId,
            };

            for (size_t i = 0; i < countof (s_pszPersonalInfoStringCols); i ++)
            {
                iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, s_pszPersonalInfoStringCols[i], (const char*)NULL);
                RETURN_ON_ERROR(iErrCode);
            }

            iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Gender, EMPIRE_GENDER_UNKNOWN);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Age, EMPIRE_AGE_UNKNOWN);
            RETURN_ON_ERROR(iErrCode);
        }

        iErrCode = ERROR_EMPIRE_IS_IN_GAMES;
    }

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
    
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::MaxNumSystemMessages, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumSavedMessages = vTemp.GetInteger();
    
    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
//
// Delete an empire and "ruin" it out of all games

int GameEngine::ObliterateEmpire(unsigned int iEmpireKey, int64 i64SecretKey, unsigned int iKillerEmpire)
{
    int iErrCode;

    if (iEmpireKey == global.GetRootKey()) {
        return ERROR_CANNOT_MODIFY_ROOT;
    }

    if (iEmpireKey == global.GetGuestKey()) {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    Assert(iEmpireKey != NO_KEY);
    Assert(iKillerEmpire != NO_KEY);
    
    bool bFlag;

    // Check secret key
    Variant vTemp;
    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::SecretKey, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    if (i64SecretKey != vTemp.GetInteger64())
    {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    // Mark empire for deletion
    // Do this under the lock so we don't race with the enter game code
    iErrCode = SetEmpireOption (iEmpireKey, EMPIRE_MARKED_FOR_DELETION, true);
    RETURN_ON_ERROR(iErrCode);

    // At this point, the empire cannot have entered any new games
    // Nuke him out of any game he might be in
    unsigned int iNumGames; 
    Variant** ppvActiveGames = NULL;
    AutoFreeData free(ppvActiveGames);

    iErrCode = GetEmpireActiveGames(iEmpireKey, &ppvActiveGames, &iNumGames);
    RETURN_ON_ERROR(iErrCode);

    if (iNumGames > 0)
    {
        iErrCode = CacheGameEmpireData(iEmpireKey, (const Variant**)ppvActiveGames, iNumGames);
        RETURN_ON_ERROR(iErrCode);

        for (unsigned int i = 0; i < iNumGames; i ++)
        {
            int iGameClass = ppvActiveGames[i][0].GetInteger();
            int iGameNumber = ppvActiveGames[i][1].GetInteger();

            // Is empire in the game
            iErrCode = IsEmpireInGame(iGameClass, iGameNumber, iEmpireKey, &bFlag);
            RETURN_ON_ERROR(iErrCode);

            if (bFlag)
            {
                // Try to quit the empire from the game nicely
                iErrCode = QuitEmpireFromGameInternal (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
                if (iErrCode == ERROR_GAME_HAS_STARTED)
                {
                    // The empire couldn't be removed nicely, so let's try it the hard way
                    iErrCode = RemoveEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
                    RETURN_ON_ERROR(iErrCode);
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }
    }

    // Try to kill the empire the old fashioned way if he's still around
    iErrCode = DoesEmpireExist(iEmpireKey, &bFlag, NULL);
    RETURN_ON_ERROR(iErrCode);
    
    if (bFlag)
    {
        iErrCode = DeleteEmpire (iEmpireKey, &i64SecretKey, true, false);
        if (iErrCode == ERROR_EMPIRE_DOES_NOT_EXIST || iErrCode == ERROR_EMPIRE_IS_IN_GAMES)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Key of empire to be deleted

int GameEngine::RemoveEmpire(int iEmpireKey)
{
    int iErrCode;

    // Best effort delete / halt all personal gameclasses
    unsigned int* piGameClassKey = NULL, i, iNumKeys;
    AutoFreeKeys freeKeys(piGameClassKey);

    iErrCode = GetEmpirePersonalGameClasses(iEmpireKey, &piGameClassKey, NULL, &iNumKeys);
    RETURN_ON_ERROR(iErrCode);

    if (iNumKeys > 0)
    {
        for (i = 0; i < iNumKeys; i ++)
        {
            bool bDelete;
            iErrCode = DeleteGameClass (piGameClassKey[i], &bDelete);
            RETURN_ON_ERROR(iErrCode);
            
            if (!bDelete)
            {
                iErrCode = t_pCache->WriteData(SYSTEM_GAMECLASS_DATA, piGameClassKey[i], SystemGameClassData::Owner, (int)DELETED_EMPIRE_KEY);
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    // Clean nuked table
    GET_SYSTEM_EMPIRE_NUKED_LIST(strNuked, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strNuked);
    RETURN_ON_ERROR(iErrCode);

    // Clean nuker table
    GET_SYSTEM_EMPIRE_NUKER_LIST(strNuker, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strNuker);
    RETURN_ON_ERROR(iErrCode);

    // Delete tournaments owned by empire or mark them for deletion
    unsigned int* piTournamentDelKey = NULL, iNumDelTournaments;
    AutoFreeKeys freeTournamentKeys(piTournamentDelKey);

    iErrCode = t_pCache->GetEqualKeys(SYSTEM_TOURNAMENTS, SystemTournaments::Owner, iEmpireKey, &piTournamentDelKey, &iNumDelTournaments);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        for (i = 0; i < iNumDelTournaments; i ++)
        {
            iErrCode = DeleteTournament(iEmpireKey, piTournamentDelKey[i], true);
            if (iErrCode == ERROR_TOURNAMENT_HAS_GAMES)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Delete empire from tournaments it was in
    GET_SYSTEM_EMPIRE_TOURNAMENTS(strEmpireTournaments, iEmpireKey);
    while (true)
    {
        unsigned int iKey = NO_KEY;
        iErrCode = t_pCache->GetNextKey(strEmpireTournaments, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
            break;

        RETURN_ON_ERROR(iErrCode);

        Variant vKey;
        iErrCode = t_pCache->ReadData(strEmpireTournaments, iKey, SystemEmpireTournaments::TournamentKey, &vKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = DeleteEmpireFromTournament(vKey.GetInteger(), iEmpireKey);
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->DeleteAllRows(strEmpireTournaments);
    RETURN_ON_ERROR(iErrCode);

    // Remove empire's messages
    GET_SYSTEM_EMPIRE_MESSAGES(strEmpireMessages, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strEmpireMessages);
    RETURN_ON_ERROR(iErrCode);
    
    // Remove empire associations
    iErrCode = RemoveDeadEmpireAssociations(iEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    // Notification
    global.GetEventSink()->OnDeleteEmpire (iEmpireKey);

    // Delete row from SystemEmpireData table
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->DeleteRow(strEmpire, iEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::LookupEmpireByName(const char* pszName, unsigned int* piEmpireKey, Variant* pvName, int64* pi64SecretKey)
{
    return LookupEmpireByName(pszName, piEmpireKey, pvName, pi64SecretKey, NULL);
}

// Input:
// iEmpireKey -> Key of empire
//
// Output:
// *pbExists -> true if exists, false if not
//
// Determines if a given empire key exists

int GameEngine::DoesEmpireExist (unsigned int iEmpireKey, bool* pbExists, Variant* pvEmpireName)
{
    *pbExists = false;

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    unsigned int iNumRows;
    int iErrCode = t_pCache->GetNumCachedRows(strEmpire, &iNumRows);
    RETURN_ON_ERROR(iErrCode);

    if (iNumRows == 0)
        return OK;

    *pbExists = true;

    if (pvEmpireName)
    {
        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Name, pvEmpireName);
        RETURN_ON_ERROR(iErrCode);
    }

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

int GameEngine::CheckSecretKey (unsigned int iEmpireKey, int64 i64SecretKey, bool* pbMatch, int64* pi64SessionId, Variant* pvIPAddress)
{
    int iErrCode;
    int64 i64StoredSecretKey;

    ICachedTable* pEmpires = NULL;
    AutoRelease<ICachedTable> release(pEmpires);

    *pbMatch = false;

    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);

    iErrCode = t_pCache->GetTable(strEmpires, &pEmpires);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpires->ReadData(iEmpireKey, SystemEmpireData::SecretKey, &i64StoredSecretKey);
    RETURN_ON_ERROR(iErrCode);

    if (i64SecretKey == i64StoredSecretKey)
    {
        *pbMatch = true;

        if (pi64SessionId != NULL) {

            iErrCode = pEmpires->ReadData(iEmpireKey, SystemEmpireData::SessionId, pi64SessionId);
            RETURN_ON_ERROR(iErrCode);
        }

        if (pvIPAddress != NULL) {

            iErrCode = pEmpires->ReadData(iEmpireKey, SystemEmpireData::IPAddress, pvIPAddress);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
// pszPassword -> Password to be tested
//
// Given an empire key and a password, determines if that password is correct for that empire
// Return OK if yes, an error if no

int GameEngine::IsPasswordCorrect(int iEmpireKey, const char* pszPassword)
{
    int iErrCode;

    Variant vTestPasswordHash;
    iErrCode = ComputePasswordHash(pszPassword, &vTestPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    Variant vActualPasswordHash;
    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::PasswordHash, &vActualPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    return String::StrCmp(vActualPasswordHash, vTestPasswordHash) == 0 ? OK : ERROR_PASSWORD;
}


// Input:
// iEmpireKey -> Integer key of empire
// pszBrowser -> Browser used to request login
//
// Perform a login for an empire and update volatile parameters.  Should be called after 
// the empire's password has been validated and the empire's key has been obtained.

int GameEngine::LoginEmpire (int iEmpireKey, const char* pszBrowser, const char* pszIPAddress) {

    Variant vLogins, vPriv = NOVICE;

    ICachedTable* pEmpire = NULL;
    AutoRelease<ICachedTable> release(pEmpire);

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    // Make sure logins are allowed
    int iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::Options, &vLogins);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->GetTable(strEmpire, &pEmpire);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpire->ReadData(iEmpireKey, SystemEmpireData::Privilege, &vPriv);
    RETURN_ON_ERROR(iErrCode);

    if (!(vLogins.GetInteger() & LOGINS_ENABLED))
    {
        // Only admins can pass...
        if (vPriv.GetInteger() < ADMINISTRATOR)
        {
            return ERROR_DISABLED;
        }
    }

    // Get the time
    UTCTime tTime;
    Time::GetTime(&tTime);

    if (String::StrLen (pszBrowser) > MAX_BROWSER_NAME_LENGTH)
    {
        char pszCutBrowser [MAX_BROWSER_NAME_LENGTH + 1];
        memcpy(pszCutBrowser, pszBrowser, MAX_BROWSER_NAME_LENGTH);
        pszCutBrowser [MAX_BROWSER_NAME_LENGTH] = '\0';

        iErrCode = pEmpire->WriteData(iEmpireKey, SystemEmpireData::Browser, pszCutBrowser);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = pEmpire->WriteData(iEmpireKey, SystemEmpireData::Browser, pszBrowser);
        RETURN_ON_ERROR(iErrCode);
    }

    // Write IP address
    iErrCode = pEmpire->WriteData(iEmpireKey, SystemEmpireData::IPAddress, pszIPAddress);
    RETURN_ON_ERROR(iErrCode);

    // Write LastLoginTime
    iErrCode = pEmpire->WriteData(iEmpireKey, SystemEmpireData::LastLoginTime, tTime);
    RETURN_ON_ERROR(iErrCode);

    // Increment the number of logins
    iErrCode = pEmpire->Increment(iEmpireKey, SystemEmpireData::NumLogins, 1);
    RETURN_ON_ERROR(iErrCode);

    // Notification
    global.GetEventSink()->OnLoginEmpire (iEmpireKey);
        
    if (vPriv.GetInteger() >= ADMINISTRATOR)
    {
        global.GetAsyncManager()->QueueTask(CheckAllGamesForUpdatesMsg, NULL);
    }

    return iErrCode;
}


int GameEngine::CheckAllGamesForUpdatesMsg(AsyncTask* pMessage)
{
    GameEngine gameEngine;

    int iErrCode = gameEngine.CacheForCheckAllGamesForUpdates();
    RETURN_ON_ERROR(iErrCode);

    iErrCode = gameEngine.CheckAllGamesForUpdates();
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Output:
// *piNumEmpires -> Number of empires
//
// Returns the number of empires currently registered on the server

int GameEngine::GetNumEmpiresOnServer(unsigned int* piNumEmpires)
{
    return t_pConn->GetNumPhysicalRows(SYSTEM_EMPIRE_DATA, piNumEmpires);
}

// Input:
// iEmpireKey -> Empire Key
//
// Undeletes an empire marked for deletion

int GameEngine::UndeleteEmpire(int iEmpireKey)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    int iErrCode;
    Variant vTemp;

    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Options, &vTemp);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);
    
    if (vTemp.GetInteger() & EMPIRE_MARKED_FOR_DELETION)
    {
        iErrCode = t_pCache->WriteAnd(strEmpire, iEmpireKey, SystemEmpireData::Options, ~EMPIRE_MARKED_FOR_DELETION);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = ERROR_CANNOT_UNDELETE_EMPIRE;
    }

    return iErrCode;
}

// Input:
// iEmpireKey -> Integer key of empire
//
// Sets all of an empire's statistics to their default values

int GameEngine::BlankEmpireStatistics(unsigned int iEmpireKey)
{
    int iErrCode;

    if (iEmpireKey == global.GetGuestKey())
    {
        return ERROR_CANNOT_MODIFY_GUEST;
    }

    // Get old scores
    Variant vPriv, vOldClassicScore, vOldAlmonasterScore, vOldBridierIndex;
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::ClassicScore, &vOldClassicScore);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScore, &vOldAlmonasterScore);
    RETURN_ON_ERROR(iErrCode);

    //
    // Blank Bridier Score
    //

    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::BridierIndex, &vOldBridierIndex);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::BridierRank, BRIDIER_INITIAL_RANK);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::BridierIndex, BRIDIER_INITIAL_INDEX);
    RETURN_ON_ERROR(iErrCode);

    //
    // Blank statistics
    //
    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Wins, 0);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Nukes, 0);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Nuked, 0);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Draws, 0);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Ruins, 0);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::MaxEcon, 0);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::MaxMil, 0);
    RETURN_ON_ERROR(iErrCode);

    // Blank Classic Score
    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::ClassicScore, CLASSIC_INITIAL_SCORE);
    RETURN_ON_ERROR(iErrCode);
    
    // Blank Almonaster Score, Significance and privilege level
    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScore, ALMONASTER_INITIAL_SCORE);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScoreSignificance, 0);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, &vPriv);
    RETURN_ON_ERROR(iErrCode);

    if (vPriv.GetInteger() != ADMINISTRATOR)
    {
        iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, NOVICE);
        RETURN_ON_ERROR(iErrCode);
    }

    // Blank nuke history fields
    GET_SYSTEM_EMPIRE_NUKER_LIST(strNukerList, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strNukerList);
    RETURN_ON_ERROR(iErrCode);

    GET_SYSTEM_EMPIRE_NUKED_LIST(strNukedList, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strNukedList);
    RETURN_ON_ERROR(iErrCode);

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

int GameEngine::GetEmpirePersonalGameClasses (int iEmpireKey, unsigned int** ppiGameClassKey, Variant** ppvName, unsigned int* piNumKeys) {

    int iErrCode;

    if (ppvName == NULL) {

        iErrCode = t_pCache->GetEqualKeys(
            SYSTEM_GAMECLASS_DATA,
            SystemGameClassData::Owner,
            iEmpireKey,
            ppiGameClassKey,
            piNumKeys
            );
    
    } else {

        iErrCode = t_pCache->ReadColumnWhereEqual (
            SYSTEM_GAMECLASS_DATA,
            SystemGameClassData::Owner,
            iEmpireKey,
            SystemGameClassData::Name,
            ppiGameClassKey,
            ppvName,
            piNumKeys
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

int GameEngine::GetEmpireData(int iEmpireKey, Variant** ppvEmpData, unsigned int* piNumActiveGames)
{
    int iErrCode;

    *ppvEmpData = NULL;

    if (piNumActiveGames)
    {
        *piNumActiveGames = 0;

        GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszActiveGames, iEmpireKey);
        iErrCode = t_pCache->GetNumCachedRows(pszActiveGames, (unsigned int*)piNumActiveGames);
        RETURN_ON_ERROR(iErrCode);
    }

    // Get data
    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);
    iErrCode = t_pCache->ReadRow(strEmpires, iEmpireKey, ppvEmpData);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiGameClass -> Array of game classes
// **ppiGameNumber -> Array of game numbers
//
// Returns the gameclasses and gamenumbers of the games an empire is currently in

int GameEngine::GetEmpireActiveGames(int iEmpireKey, Variant*** pppvActiveGames, unsigned int* piNumGames)
{
    int iErrCode;

    if (pppvActiveGames)
        *pppvActiveGames = NULL;
    *piNumGames = 0;

    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(strGames, iEmpireKey);

    if (pppvActiveGames)
    {
        const char* ppszColumns[] = { SystemEmpireActiveGames::GameClass, SystemEmpireActiveGames::GameNumber };

        iErrCode = t_pCache->ReadColumns(strGames, countof(ppszColumns), ppszColumns, NULL, pppvActiveGames, piNumGames);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = t_pCache->GetNumCachedRows(strGames, piNumGames);
        RETURN_ON_ERROR(iErrCode);
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

int GameEngine::IsEmpireInGame(int iGameClass, int iGameNumber, int iEmpireKey, bool* pbInGame)
{
    int iErrCode;
    bool bGame;

    *pbInGame = false;

    // Check game
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bGame);
    RETURN_ON_ERROR(iErrCode);
    if (!bGame)
    {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    GET_GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);
    unsigned int iKey;
    iErrCode = t_pCache->GetFirstKey(pszEmpires, GameEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        *pbInGame = true;
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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, &vPrivilege);
    RETURN_ON_ERROR(iErrCode);

    *piPrivilege = vPrivilege.GetInteger();
    
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

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    int iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, iPrivilege);
    RETURN_ON_ERROR(iErrCode);

    char pszMessage [512];
    sprintf (
        pszMessage,
        "Your privilege level was changed to %s by an administrator",
        PRIVILEGE_STRING [iPrivilege]
        );

    // Send a message to the affected empire
    iErrCode = SendSystemMessage(iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
    RETURN_ON_ERROR(iErrCode);

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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScore, &vAlmonasterScore);
    RETURN_ON_ERROR(iErrCode);

    *pfAlmonasterScore = vAlmonasterScore.GetFloat();
    
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

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    
    Variant vOldScore;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScore, &vOldScore);
    RETURN_ON_ERROR(iErrCode);

    if (fAlmonasterScore == vOldScore.GetFloat())
    {
        return OK;
    }

    iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScore, fAlmonasterScore);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CalculatePrivilegeLevel (iEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->ReadData(strEmpire, iEmpireKey, pszColumn, pvData);
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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::NumLogins, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piNumLogins = vTemp.GetInteger();
    
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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::IPAddress, pvIPAddress);
}


// Input:
// iEmpireKey -> Key of empire
// pszIPAddress -> New IP address
//
// Set an empire's IP address

int GameEngine::SetEmpireIPAddress (int iEmpireKey, const char* pszIPAddress)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::IPAddress, pszIPAddress);
}

int GameEngine::GetEmpireSessionId (int iEmpireKey, int64* pi64SessionId)
{
    Variant vProp;
    int iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::SessionId, &vProp);
    RETURN_ON_ERROR(iErrCode);

    *pi64SessionId = vProp.GetInteger64();
    return iErrCode;
}

int GameEngine::SetEmpireSessionId(int iEmpireKey, int64 i64SessionId)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::SessionId, i64SessionId);
}


int GameEngine::GetEmpireDefaultBuilderPlanet (int iEmpireKey, int* piDefaultBuildPlanet)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    Variant vValue;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::DefaultBuilderPlanet, &vValue);
    RETURN_ON_ERROR(iErrCode);

    *piDefaultBuildPlanet = vValue.GetInteger();

    return iErrCode;
}


int GameEngine::SetEmpireDefaultBuilderPlanet(int iEmpireKey, int iDefaultBuildPlanet)
{
    if (iDefaultBuildPlanet > NO_DEFAULT_BUILDER_PLANET || iDefaultBuildPlanet < LAST_BUILDER_DEFAULT_BUILDER_PLANET)
    {
        return ERROR_INVALID_ARGUMENT;
    }

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::DefaultBuilderPlanet, iDefaultBuildPlanet);
}


int GameEngine::GetEmpireDefaultMessageTarget (int iEmpireKey, int* piMessageTarget)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    Variant vData;
    int iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::DefaultMessageTarget, &vData);
    RETURN_ON_ERROR(iErrCode);

    *piMessageTarget = vData.GetInteger();
    
    return iErrCode;
}


int GameEngine::SetEmpireDefaultMessageTarget(int iEmpireKey, int iMessageTarget)
{
    if (iMessageTarget < MESSAGE_TARGET_NONE || iMessageTarget > MESSAGE_TARGET_LAST_USED)
    {
        return ERROR_INVALID_ARGUMENT;
    }

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::DefaultMessageTarget, iMessageTarget);
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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piOptions = vTemp.GetInteger();
    
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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Options2, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piOptions = vTemp.GetInteger();
    
    return iErrCode;
}


int GameEngine::GetEmpireLastBridierActivity (int iEmpireKey, UTCTime* ptTime) {

    int iErrCode;
    Variant vTemp;
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::LastBridierActivity, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *ptTime = vTemp.GetInteger64();

    return iErrCode;
}

int GameEngine::GetEmpireProperty (int iEmpireKey, const char* pszColumn, Variant* pvProperty)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->ReadData(strEmpire, iEmpireKey, pszColumn, pvProperty);
}

int GameEngine::SetEmpireProperty(int iEmpireKey, const char* pszColumn, const Variant& vProperty)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, pszColumn, vProperty);
}

int GameEngine::IsEmpireIdleInSomeGame(int iEmpireKey, bool* pfIdle)
{
    int iErrCode;
    unsigned int i, iNumGames = 0;

    Variant** ppvActiveGames = NULL;
    AutoFreeData free(ppvActiveGames);

    *pfIdle = false;

    iErrCode = GetEmpireActiveGames(iEmpireKey, &ppvActiveGames, &iNumGames);
    RETURN_ON_ERROR(iErrCode);

    if (iNumGames == 0)
    {
        return OK;
    }

    iErrCode = CacheGameEmpireData(iEmpireKey, (const Variant**)ppvActiveGames, iNumGames);
    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumGames; i ++)
    {
        int iGameClass = ppvActiveGames[i][0].GetInteger();
        int iGameNumber = ppvActiveGames[i][1].GetInteger();

        GET_GAME_EMPIRE_DATA(strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

        Variant vOptions;
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vOptions);
        RETURN_ON_ERROR(iErrCode);

        // Ignore games in which the empire has resigned
        if (vOptions.GetInteger() & RESIGNED)
        {
            continue;
        }

        if (!(vOptions.GetInteger() & LOGGED_IN_THIS_UPDATE))
        {
            Variant vNumUpdatesIdle;
            iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::NumUpdatesIdle, &vNumUpdatesIdle);
            RETURN_ON_ERROR(iErrCode);

            Variant vNumUpdatesForIdle;
            iErrCode = GetGameClassProperty(iGameClass, SystemGameClassData::NumUpdatesForIdle, &vNumUpdatesForIdle);
            RETURN_ON_ERROR(iErrCode);
            
            if (vNumUpdatesIdle.GetInteger() >= vNumUpdatesForIdle.GetInteger())
            {
                *pfIdle = true;
                return OK;
            }
        }
    }

    return iErrCode;
}