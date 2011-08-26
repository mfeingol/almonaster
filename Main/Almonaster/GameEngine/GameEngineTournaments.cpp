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
#include "Global.h"

int GameEngine::GetOwnedTournaments (int iEmpireKey, unsigned int** ppiTournamentKey, Variant** ppvName, 
                                     unsigned int* piNumTournaments) {

    int iErrCode;

    ICachedTable* pTournaments = NULL;

    if (ppiTournamentKey != NULL) {
        *ppiTournamentKey = NULL;
    }

    if (ppvName != NULL) {
        *ppvName = NULL;
    }

    *piNumTournaments = 0;

    iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pTournaments);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Query
    if (ppvName == NULL) {

        iErrCode = pTournaments->GetEqualKeys(
            SystemTournaments::Owner,
            iEmpireKey,
            ppiTournamentKey,
            piNumTournaments
            );

    } else {

        iErrCode = pTournaments->ReadColumnWhereEqual (
            SystemTournaments::Owner,
            iEmpireKey,
            SystemTournaments::Name,
            ppiTournamentKey,
            ppvName,
            piNumTournaments
            );
    }

    SafeRelease (pTournaments);

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


int GameEngine::GetJoinedTournaments(int iEmpireKey, Variant** ppvTournamentKey, Variant** ppvName, unsigned int* piNumTournaments)
{
    int iErrCode;
    unsigned int i, iNumTournaments;
    Variant* pvTournamentKey = NULL;

    GET_SYSTEM_EMPIRE_TOURNAMENTS(strEmpTournaments, iEmpireKey);

    ICachedTable* pReadTable = NULL;

    if (ppvTournamentKey != NULL)
        *ppvTournamentKey = NULL;

    if (ppvName != NULL)
        *ppvName = NULL;

    *piNumTournaments = 0;

    // Read keys
    iErrCode = t_pCache->GetTable(strEmpTournaments, &pReadTable);
    if (iErrCode != OK)
        goto Cleanup;

    if (ppvTournamentKey == NULL && ppvName == NULL)
    {
        iErrCode = pReadTable->GetNumCachedRows(&iNumTournaments);
    }
    else
    {
        iErrCode = pReadTable->ReadColumn(SystemEmpireTournaments::TournamentKey, NULL, &pvTournamentKey, &iNumTournaments);
    }

    SafeRelease (pReadTable);

    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (ppvName != NULL) {

        *ppvName = new Variant[iNumTournaments];
        Assert(*ppvName);

        iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pReadTable);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        for (i = 0; i < iNumTournaments; i ++) {

            iErrCode = pReadTable->ReadData(pvTournamentKey[i].GetInteger(), SystemTournaments::Name, (*ppvName) + i);
            if (iErrCode != OK) {
                goto Cleanup;
            }
        }

        SafeRelease (pReadTable);
    }

    if (ppvTournamentKey != NULL)
    {
        *ppvTournamentKey = pvTournamentKey;
        pvTournamentKey = NULL;
    }

    *piNumTournaments = iNumTournaments;

Cleanup:

    SafeRelease (pReadTable);

    if (pvTournamentKey != NULL) {
        t_pCache->FreeData(pvTournamentKey);
    }

    if (iErrCode != OK && ppvName != NULL && *ppvName != NULL) {
        delete [] (*ppvName);
    }

    return iErrCode;
}


int GameEngine::CreateTournament (Variant* pvTournamentData, unsigned int* piTournamentKey) {

    int iErrCode;
    unsigned int iKey;

    bool bCreated = false;
    char pszTable [256];

    ICachedTable* pTournaments = NULL;

    int iOwner = pvTournamentData [SystemTournaments::iOwner].GetInteger();
    Variant vLimit;

    if (iOwner != SYSTEM) {

        iErrCode = GetSystemProperty (SystemData::MaxNumPersonalTournaments, &vLimit);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pTournaments);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Make sure we're under the limit for personal tournaments
    if (iOwner != SYSTEM) {

        unsigned int iNumEqual;

        iErrCode = pTournaments->GetEqualKeys(
            SystemTournaments::Owner, 
            iOwner,
            NULL,
            &iNumEqual
            );

        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            goto Cleanup;
        }

        if (iNumEqual >= (unsigned int) vLimit.GetInteger()) {
            iErrCode = ERROR_TOO_MANY_TOURNAMENTS;
            goto Cleanup;
        }
    }   

    // Make sure tournament name is unique
    iErrCode = pTournaments->GetFirstKey(
        SystemTournaments::Name, 
        pvTournamentData[SystemTournaments::iName].GetCharPtr(),
        &iKey
        );

    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        iErrCode = ERROR_TOURNAMENT_ALREADY_EXISTS;
        goto Cleanup;
    }

    // Insert the row
    iErrCode = pTournaments->InsertRow(SystemTournaments::Template, pvTournamentData, piTournamentKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    SafeRelease (pTournaments);

    bCreated = true;

    // Create the tables
    GET_SYSTEM_TOURNAMENT_TEAMS (pszTable, *piTournamentKey);

    iErrCode = t_pCache->CreateTable (pszTable, SystemTournamentTeams::Template);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    GET_SYSTEM_TOURNAMENT_EMPIRES (pszTable, *piTournamentKey);

    iErrCode = t_pCache->CreateTable (pszTable, SystemTournamentEmpires::Template);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    GET_SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszTable, *piTournamentKey);

    iErrCode = t_pCache->CreateTable (pszTable, SystemTournamentActiveGames::Template);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    GET_SYSTEM_TOURNAMENT_LATEST_GAMES (pszTable, *piTournamentKey);

    iErrCode = t_pCache->CreateTable (pszTable, SystemTournamentLatestGames::Template);
    if (iErrCode != OK) {
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pTournaments);

    // Best effort cleanup
    if (iErrCode != OK && bCreated) {
        DeleteTournament (pvTournamentData[SystemTournaments::iOwner].GetInteger(), *piTournamentKey, false);
    }

    return iErrCode;
}

int GameEngine::DeleteTournament (int iEmpireKey, unsigned int iTournamentKey, bool bOwnerDeleted) {

    int iErrCode, iOwnerKey;
    char pszTable [256], pszMessage [64 + MAX_TOURNAMENT_TEAM_NAME_LENGTH];

    Variant vKey, vTournamentName, vTemp;

    unsigned int i, iGameClasses, iGames, iKey, * piGameClassKey = NULL;

    Assert (!bOwnerDeleted || iEmpireKey != DELETED_EMPIRE_KEY);
    
    // Make sure correct owner
    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vTemp);
    if (iErrCode != OK)
    {
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
            iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
        goto Cleanup;
    }
    iOwnerKey = vTemp.GetInteger();

    if (iEmpireKey != iOwnerKey) {
        iErrCode = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

    // Can't delete a tournament with games
    iErrCode = GetTournamentGames (iTournamentKey, NULL, NULL, &iGames);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (iGames > 0) {

        if (bOwnerDeleted) {

            // Mark the tournament for deletion
            iErrCode = t_pCache->WriteData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, (int) DELETED_EMPIRE_KEY);
            if (iErrCode != OK) {
                goto Cleanup;
            }
        }

        iErrCode = ERROR_TOURNAMENT_HAS_GAMES;

        goto Cleanup;
    }

    // Can't delete a live tournament if it has gameclasses
    if (iEmpireKey != DELETED_EMPIRE_KEY && !bOwnerDeleted) {

        iErrCode = GetTournamentGameClasses (iTournamentKey, NULL, NULL, &iGameClasses);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iGameClasses > 0) {
            iErrCode = ERROR_TOURNAMENT_HAS_GAMECLASSES;
            goto Cleanup;
        }
    }

    // Read name
    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Name, &vTournamentName);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pCache->DeleteRow(SYSTEM_TOURNAMENTS, iTournamentKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Delete the tables
    GET_SYSTEM_TOURNAMENT_TEAMS (pszTable, iTournamentKey);
    // TODOTODO - iErrCode = t_pCache->DeleteTable (pszTable);

    GET_SYSTEM_TOURNAMENT_EMPIRES (pszTable, iTournamentKey);
    sprintf (pszMessage, "The %s tournament was deleted", vTournamentName.GetCharPtr());

    // Try to delete gameclasses if any left
    iErrCode = t_pCache->GetEqualKeys(
        SYSTEM_GAMECLASS_DATA,
        SystemGameClassData::TournamentKey,
        iTournamentKey,
        &piGameClassKey,
        &iGameClasses
        );

    if (iErrCode != ERROR_DATA_NOT_FOUND)
    {
        if (iErrCode != OK)
            goto Cleanup;

        // Best effort
        for (i = 0; i < iGameClasses; i ++)
        {
            bool bFlag;
            iErrCode = DeleteGameClass (piGameClassKey[i], &bFlag);
            if (iErrCode != OK)
                goto Cleanup;
        }

        t_pCache->FreeKeys(piGameClassKey);
    }

    // Remove empires from tournament
    iKey = NO_KEY;
    while (true)
    {
        unsigned int iEmpireTourneyKey = NO_KEY;
        char pszEmpireTournaments [256];

        iErrCode = t_pCache->GetNextKey (pszTable, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
            break;
        }

        if (iErrCode != OK)
            goto Cleanup;

        iErrCode = t_pCache->ReadData(pszTable, iKey, SystemTournamentEmpires::EmpireKey, &vKey);
        if (iErrCode != OK)
            goto Cleanup;

        GET_SYSTEM_EMPIRE_TOURNAMENTS(strEmpireTournaments, vKey.GetInteger());
        iErrCode = t_pCache->GetFirstKey(strEmpireTournaments, SystemEmpireTournaments::TournamentKey, iTournamentKey, &iEmpireTourneyKey);
        if (iErrCode != OK)
            goto Cleanup;

        iErrCode = t_pCache->DeleteRow(pszEmpireTournaments, iEmpireTourneyKey);
        if (iErrCode != OK)
            goto Cleanup;

        iErrCode = SendSystemMessage(vKey.GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
        if (iErrCode != OK)
            goto Cleanup;
    }

    // TODOTODO - iErrCode = t_pCache->DeleteTable (pszTable);

    GET_SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszTable, iTournamentKey);
    Assert(t_pCache->GetNumCachedRows(pszTable, &iGameClasses) == OK && iGameClasses == 0);
    // TODOTODO - iErrCode = t_pCache->DeleteTable (pszTable);

    GET_SYSTEM_TOURNAMENT_LATEST_GAMES (pszTable, iTournamentKey);
    // TODOTODO - iErrCode = t_pCache->DeleteTable (pszTable);

    //
    // Notify the UI
    //

    global.GetEventSink()->OnDeleteTournament (iTournamentKey);

Cleanup:

    return iErrCode;
}

int GameEngine::GetTournamentData (unsigned int iTournamentKey, Variant** ppvTournamentData) {

    return t_pCache->ReadRow (SYSTEM_TOURNAMENTS, iTournamentKey, ppvTournamentData);
}

int GameEngine::GetTournamentName(unsigned int iTournamentKey, Variant* pvTournamentName)
{
    int iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Name, pvTournamentName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
    return iErrCode;
}

int GameEngine::GetTournamentOwner(unsigned int iTournamentKey, unsigned int* piOwnerKey)
{
    Variant vOwner;
    int iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vOwner);
    if (iErrCode == OK)
        *piOwnerKey = vOwner.GetInteger();
    else if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
    return iErrCode;
}

int GameEngine::GetTournamentDescription (unsigned int iTournamentKey, Variant* pvTournamentDesc) {

    return t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Description, pvTournamentDesc);
}

int GameEngine::SetTournamentDescription (unsigned int iTournamentKey, const char* pszTournamentDesc) {

    return SetTournamentString (iTournamentKey, SystemTournaments::Description, pszTournamentDesc);
}

int GameEngine::GetTournamentUrl (unsigned int iTournamentKey, Variant* pvTournamentUrl) {

    return t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::WebPage, pvTournamentUrl);
}

int GameEngine::SetTournamentUrl (unsigned int iTournamentKey, const char* pszTournamentUrl) {

    return SetTournamentString (iTournamentKey, SystemTournaments::WebPage, pszTournamentUrl);
}

int GameEngine::GetTournamentNews (unsigned int iTournamentKey, Variant* pvTournamentNews) {

    return t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::News, pvTournamentNews);
}

int GameEngine::SetTournamentNews (unsigned int iTournamentKey, const char* pszTournamentNews) {

    return SetTournamentString (iTournamentKey, SystemTournaments::News, pszTournamentNews);
}

int GameEngine::SetTournamentString (unsigned int iTournamentKey, const char* pszColumn, const char* pszString) {

    int iErrCode;
    ICachedTable* pWrite = NULL;

    iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pWrite);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pWrite->WriteData(iTournamentKey, pszColumn, pszString);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pWrite);

    return iErrCode;
}

int GameEngine::InviteEmpireIntoTournament(unsigned int iTournamentKey, int iOwnerKey, int iSourceKey, int iInviteKey)
{
    int iErrCode;

    Variant pvData[SystemEmpireMessages::NumColumns], vOwnerKey;

    // Verify access
    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vOwnerKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (vOwnerKey.GetInteger() != iOwnerKey) {
        return ERROR_ACCESS_DENIED;
    }

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);
    
    // Make sure empire isn't already in the tournament
    unsigned int iKey;
    iErrCode = t_pCache->GetFirstKey(pszEmpires, SystemTournamentEmpires::EmpireKey, iInviteKey, &iKey);
    if (iErrCode == OK) {
        return ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT;
    }

    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        return ERROR_TOURNAMENT_DOES_NOT_EXIST;
    }

    char pszTournamentKey [64];
    sprintf (pszTournamentKey, "%i.%i", iTournamentKey, iOwnerKey);

    UTCTime tTime;
    Time::GetTime (&tTime);

    int iFlags = 0;

    pvData[SystemEmpireMessages::iUnread] = MESSAGE_UNREAD;

    unsigned int iUseSourceKey;
    if (iOwnerKey == SYSTEM)
    {
        if (iSourceKey == SYSTEM)
        {
            iUseSourceKey = SYSTEM;
        }
        else
        {
            iUseSourceKey = iSourceKey;
        }
    }
    else
    {
        iUseSourceKey = iOwnerKey;
    }

    pvData[SystemEmpireMessages::iSourceKey] = iUseSourceKey;
    if (iUseSourceKey == SYSTEM)
    {
        iFlags |= MESSAGE_SYSTEM;
        pvData[SystemEmpireMessages::iSourceName] = (const char*)NULL;
        pvData[SystemEmpireMessages::iSourceSecret] = 0;
    }
    else
    {
        iErrCode = GetEmpireName(iUseSourceKey, pvData + SystemEmpireMessages::iSourceName);
        if (iErrCode != OK)
            return iErrCode;

        iErrCode = GetEmpireProperty(iUseSourceKey, SystemEmpireData::SecretKey, pvData + SystemEmpireMessages::iSourceSecret);
        if (iErrCode != OK)
            return iErrCode;
    }

    pvData[SystemEmpireMessages::iTimeStamp] = tTime;
    pvData[SystemEmpireMessages::iFlags] = iFlags;
    pvData[SystemEmpireMessages::iText] = (const char*) NULL;
    pvData[SystemEmpireMessages::iType] = MESSAGE_TOURNAMENT_INVITATION;
    pvData[SystemEmpireMessages::iData] = pszTournamentKey;

    return DeliverSystemMessage (iInviteKey, pvData);
}

int GameEngine::InviteSelfIntoTournament (unsigned int iTournamentKey, int iEmpireKey) {

    int iErrCode;
    unsigned int iKey;

    Variant pvData[SystemEmpireMessages::NumColumns], vOwnerKey;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);
    
    // Make sure empire isn't already in the tournament
    iErrCode = t_pCache->GetFirstKey(
        pszEmpires, 
        SystemTournamentEmpires::EmpireKey, 
        iEmpireKey, 
        &iKey
        );

    if (iErrCode == OK) {
        return ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT;
    }

    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        return ERROR_TOURNAMENT_DOES_NOT_EXIST;
    }

    // Find owner
    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vOwnerKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Bit of a hack... Use 'root' if system tournament
    if (vOwnerKey.GetInteger() == SYSTEM) {
        vOwnerKey = global.GetRootKey();
    }

    char pszTournamentKey [64];
    sprintf (pszTournamentKey, "%i.%i", iTournamentKey, iEmpireKey);

    UTCTime tTime;
    Time::GetTime (&tTime);

    // Unread
    pvData[SystemEmpireMessages::iUnread] = MESSAGE_UNREAD;

    // Source
    pvData[SystemEmpireMessages::iSourceKey] = iEmpireKey;

    iErrCode = GetEmpireName(iEmpireKey, pvData + SystemEmpireMessages::iSourceName);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::SecretKey, pvData + SystemEmpireMessages::iSourceSecret);
    if (iErrCode != OK) {
        return iErrCode;
    }

    pvData[SystemEmpireMessages::iTimeStamp] = tTime;
    pvData[SystemEmpireMessages::iFlags] = 0;
    pvData[SystemEmpireMessages::iText] = (const char*) NULL;
    pvData[SystemEmpireMessages::iType] = MESSAGE_TOURNAMENT_JOIN_REQUEST;
    pvData[SystemEmpireMessages::iData] = pszTournamentKey;

    return DeliverSystemMessage (vOwnerKey.GetInteger(), pvData);
}

int GameEngine::RespondToTournamentInvitation (int iInviteKey, int iMessageKey, bool bAccept) {

    return HandleEmpireTournamentAddition (iInviteKey, iMessageKey, MESSAGE_TOURNAMENT_INVITATION, bAccept);
}

int GameEngine::HandleEmpireTournamentAddition(int iEmpireKey, int iMessageKey, int iMessageType, bool bAccept)
{
    int iErrCode, iOwnerKey, iInviteKey, iSendMessageKey = NO_KEY, iVal;
    unsigned int iTournamentKey;
    const char* pszFormatString = NULL, * pszYesNo = NULL;

    Variant vVal, vEmpireName, vTourneyName;

    ICachedTable* pTable = NULL;
    ICachedTable* pMessages = NULL;

    bool bSendMessage = false;

    GET_SYSTEM_EMPIRE_MESSAGES(strMessages, iEmpireKey);
    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    if (iErrCode != OK)
    {
        goto Cleanup;
    }

    iErrCode = pMessages->ReadData (iMessageKey, SystemEmpireMessages::Type, &iVal);
    if (iErrCode != OK)
    {
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
            iErrCode = ERROR_MESSAGE_DOES_NOT_EXIST;
        goto Cleanup;
    }
        
    if (iVal != iMessageType)
    {
        iErrCode = ERROR_MESSAGE_DOES_NOT_EXIST;
        goto Cleanup;
    }

    iErrCode = pMessages->ReadData (iMessageKey, SystemEmpireMessages::Data, &vVal);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pMessages->DeleteRow(iMessageKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pMessages);

    if (iMessageType == MESSAGE_TOURNAMENT_INVITATION) {

        if (sscanf (vVal.GetCharPtr(), "%i.%i", &iTournamentKey, &iOwnerKey) != 2) {
            Assert (false);
            iErrCode = ERROR_FAILURE;
            goto Cleanup;
        }

        iInviteKey = iEmpireKey;
        iSendMessageKey = iOwnerKey;

        if (iSendMessageKey == SYSTEM) {
            iSendMessageKey = global.GetRootKey();
        }

        pszFormatString = "%s has %s the %s tournament";
        pszYesNo = bAccept ? "joined" : "declined to join";

    } else {

        if (sscanf (vVal.GetCharPtr(), "%i.%i", &iTournamentKey, &iInviteKey) != 2) {
            Assert (false);
            iErrCode = ERROR_FAILURE;
            goto Cleanup;
        }

        iOwnerKey = iEmpireKey;
        iSendMessageKey = iInviteKey;

        pszFormatString = "%s has %s you to join the %s tournament", 
        pszYesNo = bAccept ? "allowed" : "declined to allow";
    }

    // Get the active empire's name
    iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Verify ownership
    iErrCode = pTable->ReadData(iTournamentKey, SystemTournaments::Owner, &vVal);
    if (iErrCode != OK)
    {
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
            iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Get name 
    iErrCode = pTable->ReadData(iTournamentKey, SystemTournaments::Name, &vTourneyName);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    SafeRelease (pTable);

    if (vVal.GetInteger() != iOwnerKey) {

        if (vVal.GetInteger() == SYSTEM) {

            int iPrivilege;
            iErrCode = GetEmpirePrivilege (iOwnerKey, &iPrivilege);
            if (iErrCode != OK) {
                iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
                goto Cleanup;
            }

            if (iPrivilege >= ADMINISTRATOR) {
                iOwnerKey = SYSTEM;
            }
        }

        if (vVal.GetInteger() != iOwnerKey) {
            iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
            goto Cleanup;
        }
    }

    // Finally, do something
    if (bAccept) {

        iErrCode = AddEmpireToTournament (iTournamentKey, iInviteKey);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    bSendMessage = true;

Cleanup:

    SafeRelease (pTable);
    SafeRelease (pMessages);

    if (bSendMessage) {
        
        char pszMessage [MAX_EMPIRE_NAME_LENGTH + MAX_TOURNAMENT_TEAM_NAME_LENGTH + 64];
        
        sprintf (
            pszMessage, 
            pszFormatString, 
            vEmpireName.GetCharPtr(), 
            pszYesNo, 
            vTourneyName.GetCharPtr()
            );
        
        SendSystemMessage (iSendMessageKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
    }

    return iErrCode;
}

int GameEngine::AddEmpireToTournament(unsigned int iTournamentKey, int iInviteKey)
{
    int iErrCode;
    unsigned int iProxyKey;

    Variant vVal;

    ICachedTable* pWriteTable = NULL;
    
    // Join the tournament!
    SYSTEM_TOURNAMENT_EMPIRES(pszEmpires, iTournamentKey);
    GET_SYSTEM_EMPIRE_TOURNAMENTS(pszTourneys, iInviteKey);
    
    // Add to SystemTournamentEmpires
    Variant pvData[SystemTournamentEmpires::NumColumns] = 
    {
        iInviteKey,     // EmpireKey
        NO_KEY,         // TeamKey
        0,              // Wins
        0,              // Nukes
        0,              // Nuked
        0,              // Draws
        0,              // Ruins
    };
    
    iErrCode = t_pCache->GetTable(pszEmpires, &pWriteTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pWriteTable->GetFirstKey(SystemTournamentEmpires::EmpireKey, iInviteKey, &iProxyKey);
    if (iErrCode == OK) {
        iErrCode = ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT;
        goto Cleanup;
    }
    
    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        goto Cleanup;
    }
    
    iErrCode = pWriteTable->InsertRow (SystemTournamentEmpires::Template, pvData, &iProxyKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    // Add to SystemEmpireTournaments
    {
    Variant pvSystemEmpireTournaments[SystemEmpireTournaments::NumColumns] = 
    {
        iInviteKey,
        iTournamentKey,
    };
    
    iErrCode = t_pCache->InsertRow(pszTourneys, SystemEmpireTournaments::Template, pvSystemEmpireTournaments, NULL);
    if (iErrCode != OK)
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pWriteTable);

    return iErrCode;
}

int GameEngine::RespondToTournamentJoinRequest (int iEmpireKey, int iMessageKey, bool bAccept) {

    return HandleEmpireTournamentAddition (iEmpireKey, iMessageKey, MESSAGE_TOURNAMENT_JOIN_REQUEST, bAccept);
}


int GameEngine::DeleteEmpireFromTournament (unsigned int iTournamentKey, int iDeleteKey) {

    int iErrCode;
    unsigned int iKey, iNumGames, i;

    ICachedTable* pEmpires = NULL;

    Variant vTourneyName, * pvGameClassGameNumber = NULL;

    SYSTEM_TOURNAMENT_EMPIRES(pszEmpires, iTournamentKey);
    SYSTEM_TOURNAMENT_ACTIVE_GAMES(pszGames, iTournamentKey);
    GET_SYSTEM_EMPIRE_TOURNAMENTS(pszTourneys, iDeleteKey);

    char pszMessage [MAX_TOURNAMENT_TEAM_NAME_LENGTH + 96];

    iErrCode = GetTournamentName (iTournamentKey, &vTourneyName);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = t_pCache->GetFirstKey(pszEmpires, SystemTournamentEmpires::EmpireKey, iDeleteKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pCache->ReadColumn(pszGames, SystemTournamentActiveGames::GameClassGameNumber, NULL, &pvGameClassGameNumber, &iNumGames);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        } else {
            Assert (false);
            goto Cleanup;
        }
    }

    for (i = 0; i < iNumGames; i ++) {

        bool bInGame;
        int iGameClass, iGameNumber;
    
        GetGameClassGameNumber (pvGameClassGameNumber[i].GetCharPtr(), &iGameClass, &iGameNumber);

        iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iDeleteKey, &bInGame);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (bInGame) {
            iErrCode = ERROR_EMPIRE_IS_IN_GAMES;
            goto Cleanup;
        }
    }

    // No more empire in tournament
    iErrCode = t_pCache->DeleteRow(pszEmpires, iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // No more tournament for empire
    iErrCode = t_pCache->GetTable(pszTourneys, &pEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpires->GetFirstKey(SystemEmpireTournaments::TournamentKey, iTournamentKey, &iKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pEmpires->DeleteRow(iKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    SafeRelease (pEmpires);
        
    sprintf (
        pszMessage, 
        "You have been removed from the %s tournament",
        vTourneyName.GetCharPtr()
        );
    
    iErrCode = SendSystemMessage (iDeleteKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);

Cleanup:

    SafeRelease (pEmpires);

    if (pvGameClassGameNumber != NULL) {
        t_pCache->FreeData(pvGameClassGameNumber);
    }

    return iErrCode;
}

int GameEngine::GetTournamentGameClasses (unsigned int iTournamentKey, unsigned int** ppiGameClassKey,
                                          Variant** ppvName, unsigned int* piNumKeys) {

    int iErrCode;

    ICachedTable* pTable = NULL;

    if (ppiGameClassKey != NULL) {
        *ppiGameClassKey = NULL;
    }

    if (ppvName != NULL) {
        *ppvName = NULL;
    }
    *piNumKeys = 0;

    iErrCode = t_pCache->GetTable(SYSTEM_GAMECLASS_DATA, &pTable);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Query
    if (ppvName == NULL) {

        iErrCode = pTable->GetEqualKeys(
            SystemGameClassData::TournamentKey,
            iTournamentKey,
            ppiGameClassKey,
            piNumKeys
            );

    } else {

        iErrCode = pTable->ReadColumnWhereEqual (
            SystemGameClassData::TournamentKey,
            iTournamentKey,
            SystemGameClassData::Name,
            ppiGameClassKey,
            ppvName,
            piNumKeys
            );
    }

    SafeRelease (pTable);

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}

int GameEngine::GetGameClassTournament (int iGameClass, unsigned int* piTournamentKey) {

    int iErrCode;
    Variant vValue;

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::TournamentKey, &vValue);
    if (iErrCode == OK) {
        *piTournamentKey = vValue.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetTournamentEmpires(unsigned int iTournamentKey, Variant** ppvEmpireKey, Variant** ppvTeamKey, Variant** ppvName, unsigned int* piNumKeys)
{
    int iErrCode;
    unsigned int i;

    ICachedTable* pTable = NULL;

    SYSTEM_TOURNAMENT_EMPIRES(pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetTable(pszEmpires, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (ppvEmpireKey != NULL) {

        iErrCode = pTable->ReadColumn(SystemTournamentEmpires::EmpireKey, NULL, ppvEmpireKey, piNumKeys);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            goto Cleanup;
        }

    } else {

        iErrCode = pTable->GetNumCachedRows(piNumKeys);
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (ppvTeamKey != NULL) {

        unsigned int iNumKeys2;

        iErrCode = pTable->ReadColumn(SystemTournamentEmpires::TeamKey, NULL, ppvTeamKey, &iNumKeys2);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }

        Assert (iNumKeys2 == *piNumKeys);
    }

    SafeRelease (pTable);

    if (ppvName != NULL) {

        *ppvName = new Variant[*piNumKeys];
        Assert(*ppvName);

        for (i = 0; i < *piNumKeys; i ++) {

            int ec = GetEmpireName ((*ppvEmpireKey)[i], (*ppvName) + i);
            if (ec != OK) {
                (*ppvEmpireKey)[i] = (*ppvEmpireKey)[*piNumKeys - 1];
                i --;
            }
        }
    }

Cleanup:

    if (iErrCode != OK || (iErrCode == OK && *piNumKeys == 0)) {
        
        if (ppvEmpireKey != NULL && *ppvEmpireKey != NULL) {
            t_pCache->FreeData(*ppvEmpireKey);
            *ppvEmpireKey = NULL;
        }
        
        if (ppvName != NULL && *ppvName != NULL) {
            delete [] (*ppvName);
            *ppvName = NULL;
        }
        
        if (ppvTeamKey != NULL && *ppvTeamKey != NULL) {
            t_pCache->FreeData(*ppvTeamKey);
            *ppvTeamKey = NULL;
        }
    }

    SafeRelease (pTable);

    return iErrCode;
}


int GameEngine::GetAvailableTournamentEmpires (unsigned int iTournamentKey, Variant** ppvEmpireKey,
                                               Variant** ppvTeamKey, Variant** ppvName, unsigned int* piNumKeys) {

    int iErrCode, iOptions;
    unsigned int iNumKeys, i;
    Variant* pvName = NULL, * pvTeamKey = NULL, * pvEmpireKey = NULL;

    iErrCode = GetTournamentEmpires(
        iTournamentKey,
        ppvEmpireKey == NULL ? NULL : &pvEmpireKey,
        ppvTeamKey == NULL ? NULL : &pvTeamKey,
        ppvName == NULL ? NULL : &pvName,
        &iNumKeys
        );

    if (iErrCode != OK) {
        goto Cleanup;
    }

    for (i = 0; i < iNumKeys; i ++) {

        if (GetEmpireOptions2 (pvEmpireKey[i], &iOptions) != OK || 
            (iOptions & UNAVAILABLE_FOR_TOURNAMENTS)) {

            // Remove from list
            if (pvEmpireKey != NULL) {
                pvEmpireKey[i] = pvEmpireKey [iNumKeys - 1];
            }

            if (pvTeamKey != NULL) {
                pvTeamKey[i] = pvTeamKey [iNumKeys - 1];
            }

            if (pvName != NULL) {
                pvName[i] = pvName [iNumKeys - 1];
            }

            iNumKeys --;
        }
    }

Cleanup:

    if (iErrCode == OK && iNumKeys == 0) {

        if (pvEmpireKey != NULL) {
            t_pCache->FreeData(pvEmpireKey);
            pvEmpireKey = NULL;
        }

        if (pvTeamKey != NULL) {
            t_pCache->FreeData(pvTeamKey);
            pvTeamKey = NULL;
        }

        if (pvName != NULL) {
            delete [] pvName;
            pvName = NULL;
        }
    }

    if (ppvEmpireKey != NULL) {
        *ppvEmpireKey = pvEmpireKey;
    }

    if (ppvTeamKey != NULL) {
        *ppvTeamKey = pvTeamKey;
    }

    if (ppvName != NULL) {
        *ppvName = pvName;
    }

    *piNumKeys = iNumKeys;

    return iErrCode;
}


int GameEngine::GetTournamentTeamEmpires (unsigned int iTournamentKey, unsigned int iTeamKey, int** ppiEmpireKey, 
                                          Variant** ppvName, unsigned int* piNumKeys) {

    int iErrCode;
    unsigned int i, * piProxyKey = NULL;

    ICachedTable* pTable = NULL;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetTable(pszEmpires, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->GetEqualKeys(SystemTournamentEmpires::TeamKey, iTeamKey, &piProxyKey, piNumKeys);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (ppiEmpireKey != NULL) {

        *ppiEmpireKey = new int [*piNumKeys];
        if (*ppiEmpireKey == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        for (i = 0; i < *piNumKeys; i ++) {

            iErrCode = pTable->ReadData (piProxyKey[i], SystemTournamentEmpires::EmpireKey, (*ppiEmpireKey) + i);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }   

    SafeRelease (pTable);

    if (ppvName != NULL) {

        *ppvName = new Variant [*piNumKeys];
        if (*ppvName == NULL) {

            if (ppiEmpireKey != NULL) {
                delete [] (*ppiEmpireKey);
                *ppiEmpireKey = NULL;
            }

            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        for (i = 0; i < *piNumKeys; i ++) {

            int ec = GetEmpireName ((*ppiEmpireKey)[i], (*ppvName) + i);
            if (ec!= OK) {
                (*ppiEmpireKey)[i] = (*ppiEmpireKey)[*piNumKeys - 1];
                i --;
            }
        }
    }

Cleanup:
    
    if (iErrCode != OK || (iErrCode == OK && *piNumKeys == 0)) {
        
        if (ppiEmpireKey != NULL && *ppiEmpireKey != NULL) {
            delete [] (*ppiEmpireKey);
            *ppiEmpireKey = NULL;
        }
        
        if (ppvName != NULL && *ppvName != NULL) {
            delete [] (*ppvName);
            *ppvName = NULL;
        }
    }
    
    SafeRelease (pTable);

    return iErrCode;
}


int GameEngine::GetTournamentTeams (unsigned int iTournamentKey, unsigned int** ppiTeamKey, Variant** ppvName, 
                                    unsigned int* piNumKeys) {

    int iErrCode;

    SYSTEM_TOURNAMENT_TEAMS(pszTeams, iTournamentKey);

    if (ppvName == NULL) {

        if (ppiTeamKey == NULL) {
            iErrCode = t_pCache->GetNumCachedRows(pszTeams, piNumKeys);
        } else {
            iErrCode = t_pCache->GetAllKeys (pszTeams, ppiTeamKey, piNumKeys);
        }

    } else {

        iErrCode = t_pCache->ReadColumn (pszTeams, SystemTournamentTeams::Name, ppiTeamKey, ppvName, piNumKeys);
    }

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}

int GameEngine::CreateTournamentTeam (unsigned int iTournamentKey, Variant* pvTeamData, unsigned int* piTeamKey) {

    int iErrCode;
    unsigned int iKey;

    ICachedTable* pTable = NULL;

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    iErrCode = t_pCache->GetTable(pszTeams, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Make sure team doesn't already exist
    iErrCode = pTable->GetFirstKey(
        SystemTournamentTeams::Name, 
        pvTeamData[SystemTournamentTeams::iName].GetCharPtr(),
        &iKey
        );

    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        iErrCode = ERROR_TOURNAMENT_TEAM_ALREADY_EXISTS;
        goto Cleanup;
    }

    // Create team
    iErrCode = pTable->InsertRow (SystemTournamentTeams::Template, pvTeamData, piTeamKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pTable);

    return iErrCode;

}

int GameEngine::DeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) {

    int iErrCode, iTryTeamKey;
    unsigned int iKey = NO_KEY;

    ICachedTable* pTable = NULL;

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);
    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetTable(pszEmpires, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Remove empires from team
    while (pTable->GetNextKey (iKey, &iKey) != ERROR_DATA_NOT_FOUND) {

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pTable->ReadData (iKey, SystemTournamentEmpires::TeamKey, &iTryTeamKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if ((unsigned int) iTryTeamKey == iTeamKey) {

            iErrCode = pTable->WriteData(iKey, SystemTournamentEmpires::TeamKey, (int) NO_KEY);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    SafeRelease (pTable);

    iErrCode = t_pCache->DeleteRow(pszTeams, iTeamKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    global.GetEventSink()->OnDeleteTournamentTeam (iTournamentKey, iTeamKey);

Cleanup:

    SafeRelease (pTable);

    return iErrCode;
}

int GameEngine::SetEmpireTournamentTeam (unsigned int iTournamentKey, int iEmpireKey, unsigned int iTeamKey) {

    int iErrCode;
    unsigned int iKey;

    ICachedTable* pWriteTable = NULL;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    if (iTeamKey != NO_KEY)
    {
        // Ensure team exists
        SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);
        Variant vTemp;
        iErrCode = t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::Draws, &vTemp);
        if (iErrCode != OK)
        {
            if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
                iErrCode = ERROR_TOURNAMENT_TEAM_DOES_NOT_EXIST;
            goto Cleanup;
        }
    }

    iErrCode = t_pCache->GetTable(pszEmpires, &pWriteTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pWriteTable->GetFirstKey(SystemTournamentEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pWriteTable->WriteData(iKey, SystemTournamentEmpires::TeamKey, (int) iTeamKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pWriteTable);

    return iErrCode;
}

int GameEngine::GetTournamentTeamData (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                       Variant** ppvTournamentTeamData) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->ReadRow (pszTeams, iTeamKey, ppvTournamentTeamData);
}

int GameEngine::GetTournamentEmpireData (unsigned int iTournamentKey, unsigned int iEmpireKey, 
                                         Variant** ppvTournamentEmpireData) {

    int iErrCode;
    unsigned int iKey;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetFirstKey(
        pszEmpires, 
        SystemTournamentEmpires::EmpireKey,
        iEmpireKey,
        &iKey
        );

    if (iErrCode == OK) {
        iErrCode = t_pCache->ReadRow (pszEmpires, iKey, ppvTournamentEmpireData);
    }

    return iErrCode;
}


int GameEngine::GetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                              Variant* pvTournamentTeamDesc) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::Description, pvTournamentTeamDesc);
}

int GameEngine::SetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                              const char* pszTournamentTeamDesc) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->WriteData(pszTeams, iTeamKey, SystemTournamentTeams::Description, pszTournamentTeamDesc);
}

int GameEngine::GetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                      Variant* pvTournamentTeamUrl) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::WebPage, pvTournamentTeamUrl);
}

int GameEngine::SetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                      const char* pszTournamentTeamUrl) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->WriteData(pszTeams, iTeamKey, SystemTournamentTeams::WebPage, pszTournamentTeamUrl);
}

int GameEngine::GetTournamentIcon (unsigned int iTournamentKey, unsigned int* piIcon) {

    int iErrCode;
    Variant vVal;

    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Icon, &vVal);
    if (iErrCode == OK) {
        *piIcon = vVal.GetInteger();
    }

    return iErrCode;
}

int GameEngine::SetTournamentIcon (unsigned int iTournamentKey, unsigned int iIcon) {

    return t_pCache->WriteData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Icon, iIcon);
}

int GameEngine::GetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int* piIcon) {

    int iErrCode;
    Variant vVal;

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    iErrCode = t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::Icon, &vVal);
    if (iErrCode == OK) {
        *piIcon = vVal.GetInteger();
    }

    return iErrCode;
}

int GameEngine::SetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int iIcon) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->WriteData(pszTeams, iTeamKey, SystemTournamentTeams::Icon, iIcon);
}


int GameEngine::GetTournamentGames (unsigned int iTournamentKey, int** ppiGameClass, int** ppiGameNumber, 
                                    unsigned int* piNumGames) {
    int iErrCode;
    unsigned int iKey = NO_KEY, i = 0;

    ICachedTable* pTable = NULL;

    SYSTEM_TOURNAMENT_ACTIVE_GAMES(pszGames, iTournamentKey);

    Assert (ppiGameClass != NULL && ppiGameNumber != NULL || ppiGameClass == NULL && ppiGameNumber == NULL);

    if (ppiGameClass == NULL)
    {
        return t_pCache->GetNumCachedRows(pszGames, piNumGames);
    }

    iErrCode = t_pCache->GetTable(pszGames, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->GetNumCachedRows(piNumGames);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    *ppiGameClass = new int [*piNumGames];
    *ppiGameNumber = new int [*piNumGames];

    if (*ppiGameClass == NULL || *ppiGameNumber == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    while (true) {

        iErrCode = pTable->GetNextKey (iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        
        if (iErrCode != OK) {
            goto Cleanup;
        }

        Variant vGame;
        iErrCode = pTable->ReadData (iKey, SystemTournamentActiveGames::GameClassGameNumber, &vGame);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        Assert (i < *piNumGames);
        GetGameClassGameNumber (vGame.GetCharPtr(), (*ppiGameClass) + i, (*ppiGameNumber) + i);
        i ++;
    }

    Assert (i == *piNumGames);

Cleanup:

    SafeRelease (pTable);

    if (iErrCode != OK) {

        if (ppiGameClass != NULL && *ppiGameClass != NULL) {
            delete [] (*ppiGameClass);
            *ppiGameClass = NULL;
        }

        if (ppiGameNumber != NULL && *ppiGameNumber != NULL) {
            delete [] (*ppiGameNumber);
            *ppiGameNumber = NULL;
        }
    }

    return iErrCode;
}