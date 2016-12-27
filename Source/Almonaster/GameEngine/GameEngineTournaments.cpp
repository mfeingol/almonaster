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
    AutoRelease<ICachedTable> rel(pTournaments);

    if (ppiTournamentKey != NULL) {
        *ppiTournamentKey = NULL;
    }

    if (ppvName != NULL) {
        *ppvName = NULL;
    }

    *piNumTournaments = 0;

    iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pTournaments);
    RETURN_ON_ERROR(iErrCode);

    // Query
    if (ppvName == NULL)
    {
        iErrCode = pTournaments->GetEqualKeys(
            SystemTournaments::Owner,
            iEmpireKey,
            ppiTournamentKey,
            piNumTournaments
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = pTournaments->ReadColumnWhereEqual (
            SystemTournaments::Owner,
            iEmpireKey,
            SystemTournaments::Name,
            ppiTournamentKey,
            ppvName,
            piNumTournaments
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);

    }

    return iErrCode;
}


int GameEngine::GetJoinedTournaments(int iEmpireKey, Variant** ppvTournamentKey, Variant** ppvName, unsigned int* piNumTournaments)
{
    int iErrCode;
    unsigned int i, iNumTournaments;
    Variant* pvTournamentKey = NULL;
    AutoFreeData free(pvTournamentKey);

    GET_SYSTEM_EMPIRE_TOURNAMENTS(strEmpTournaments, iEmpireKey);

    ICachedTable* pReadTable = NULL;
    AutoRelease<ICachedTable> rel(pReadTable);

    if (ppvTournamentKey != NULL)
        *ppvTournamentKey = NULL;

    if (ppvName != NULL)
        *ppvName = NULL;

    *piNumTournaments = 0;

    // Read keys
    iErrCode = t_pCache->GetTable(strEmpTournaments, &pReadTable);
    RETURN_ON_ERROR(iErrCode);

    if (ppvTournamentKey == NULL && ppvName == NULL)
    {
        iErrCode = pReadTable->GetNumCachedRows(&iNumTournaments);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = pReadTable->ReadColumn(SystemEmpireTournaments::TournamentKey, NULL, &pvTournamentKey, &iNumTournaments);
        RETURN_ON_ERROR(iErrCode);
    }

    Variant* pvName = NULL;
    Algorithm::AutoDelete<Variant> autoDel(pvName, true);
    if (ppvName)
    {
        pvName = new Variant[iNumTournaments];
        Assert(pvName);

        SafeRelease (pReadTable);
        iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pReadTable);
        RETURN_ON_ERROR(iErrCode);

        for (i = 0; i < iNumTournaments; i ++)
        {
            iErrCode = pReadTable->ReadData(pvTournamentKey[i].GetInteger(), SystemTournaments::Name, pvName + i);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (ppvTournamentKey)
    {
        *ppvTournamentKey = pvTournamentKey;
        pvTournamentKey = NULL;
    }

    if (ppvName)
    {
        *ppvName = pvName;
        pvName = NULL;
    }

    *piNumTournaments = iNumTournaments;

    return iErrCode;
}


int GameEngine::CreateTournament(Variant* pvTournamentData, unsigned int* piTournamentKey)
{
    int iErrCode;
    unsigned int iKey;

    ICachedTable* pTournaments = NULL;
    AutoRelease<ICachedTable> rel(pTournaments);

    int iOwner = pvTournamentData [SystemTournaments::iOwner].GetInteger();
    Variant vLimit;

    if (iOwner != SYSTEM) {

        iErrCode = GetSystemProperty (SystemData::MaxNumPersonalTournaments, &vLimit);
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pTournaments);
    RETURN_ON_ERROR(iErrCode);

    // Make sure we're under the limit for personal tournaments
    if (iOwner != SYSTEM) {

        unsigned int iNumEqual;

        iErrCode = pTournaments->GetEqualKeys(SystemTournaments::Owner, iOwner, NULL, &iNumEqual);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);

        if (iNumEqual >= (unsigned int) vLimit.GetInteger())
        {
            return ERROR_TOO_MANY_TOURNAMENTS;
        }
    }   

    // Make sure tournament name is unique
    iErrCode = pTournaments->GetFirstKey(SystemTournaments::Name, pvTournamentData[SystemTournaments::iName].GetCharPtr(), &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        return ERROR_TOURNAMENT_ALREADY_EXISTS;
    }

    // Insert the row
    iErrCode = pTournaments->InsertRow(SystemTournaments::Template, pvTournamentData, piTournamentKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::DeleteTournament (int iEmpireKey, unsigned int iTournamentKey, bool bOwnerDeleted) {

    int iErrCode, iOwnerKey;
    char pszTable [256], pszMessage [64 + MAX_TOURNAMENT_TEAM_NAME_LENGTH];

    Variant vKey, vTournamentName, vTemp;

    unsigned int i, iGameClasses, iGames, iKey, * piGameClassKey = NULL;
    AutoFreeKeys free(piGameClassKey);

    Assert(!bOwnerDeleted || iEmpireKey != DELETED_EMPIRE_KEY);
    
    // Make sure correct owner
    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vTemp);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_TOURNAMENT_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);

    iOwnerKey = vTemp.GetInteger();
    if (iEmpireKey != iOwnerKey)
    {
        return ERROR_ACCESS_DENIED;
    }

    // Can't delete a tournament with games
    iErrCode = GetTournamentGames (iTournamentKey, NULL, &iGames);
    RETURN_ON_ERROR(iErrCode);

    if (iGames > 0) {

        if (bOwnerDeleted) {

            // Mark the tournament for deletion
            iErrCode = t_pCache->WriteData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, (int) DELETED_EMPIRE_KEY);
            RETURN_ON_ERROR(iErrCode);
        }

        return ERROR_TOURNAMENT_HAS_GAMES;
    }

    // Can't delete a live tournament if it has gameclasses
    if (iEmpireKey != DELETED_EMPIRE_KEY && !bOwnerDeleted) {

        iErrCode = GetTournamentGameClasses (iTournamentKey, NULL, NULL, &iGameClasses);
        RETURN_ON_ERROR(iErrCode);

        if (iGameClasses > 0) {
            return ERROR_TOURNAMENT_HAS_GAMECLASSES;
        }
    }

    // Read name
    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Name, &vTournamentName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->DeleteRow(SYSTEM_TOURNAMENTS, iTournamentKey);
    RETURN_ON_ERROR(iErrCode);

    // Delete the tables
    GET_SYSTEM_TOURNAMENT_TEAMS(strTournamentTeams, iTournamentKey);
    iErrCode = t_pCache->DeleteAllRows(strTournamentTeams);
    RETURN_ON_ERROR(iErrCode);

    sprintf(pszMessage, "The %s tournament was deleted", vTournamentName.GetCharPtr());

    // Delete gameclasses if any left
    iErrCode = t_pCache->GetEqualKeys(
        SYSTEM_GAMECLASS_DATA,
        SystemGameClassData::TournamentKey,
        iTournamentKey,
        &piGameClassKey,
        &iGameClasses
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        for (i = 0; i < iGameClasses; i ++)
        {
            bool bFlag;
            iErrCode = DeleteGameClass (piGameClassKey[i], &bFlag);
            RETURN_ON_ERROR(iErrCode);
        }
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
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(pszTable, iKey, SystemTournamentEmpires::EmpireKey, &vKey);
        RETURN_ON_ERROR(iErrCode);

        GET_SYSTEM_EMPIRE_TOURNAMENTS(strEmpireTournaments, vKey.GetInteger());
        iErrCode = t_pCache->GetFirstKey(strEmpireTournaments, SystemEmpireTournaments::TournamentKey, iTournamentKey, &iEmpireTourneyKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->DeleteRow(pszEmpireTournaments, iEmpireTourneyKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = SendSystemMessage(vKey.GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
    }

    GET_SYSTEM_TOURNAMENT_EMPIRES(strTournamentEmpires, iTournamentKey);
    iErrCode = t_pCache->DeleteAllRows(strTournamentEmpires);
    RETURN_ON_ERROR(iErrCode);

    //
    // Notify the UI
    //

    global.GetEventSink()->OnDeleteTournament (iTournamentKey);

    return iErrCode;
}

int GameEngine::GetTournamentData (unsigned int iTournamentKey, Variant** ppvTournamentData)
{
    int iErrCode = t_pCache->ReadRow (SYSTEM_TOURNAMENTS, iTournamentKey, ppvTournamentData);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_TOURNAMENT_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::GetTournamentName(unsigned int iTournamentKey, Variant* pvTournamentName)
{
    int iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Name, pvTournamentName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_TOURNAMENT_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::GetTournamentOwner(unsigned int iTournamentKey, unsigned int* piOwnerKey)
{
    Variant vOwner;
    int iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vOwner);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_TOURNAMENT_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);
    *piOwnerKey = vOwner.GetInteger();
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

    iErrCode = t_pCache->WriteData(SYSTEM_TOURNAMENTS, iTournamentKey, pszColumn, pszString);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::InviteEmpireIntoTournament(unsigned int iTournamentKey, int iOwnerKey, int iSourceKey, int iInviteKey)
{
    int iErrCode;

    // Verify access
    Variant vOwnerKey;
    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vOwnerKey);
    RETURN_ON_ERROR(iErrCode);

    if (vOwnerKey.GetInteger() != iOwnerKey) {
        return ERROR_ACCESS_DENIED;
    }

    GET_SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);
    
    // Make sure empire isn't already in the tournament
    unsigned int iKey;
    iErrCode = t_pCache->GetFirstKey(pszEmpires, SystemTournamentEmpires::EmpireKey, iInviteKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        return ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT;
    }

    char pszTournamentKey [64];
    sprintf(pszTournamentKey, "%i.%i", iTournamentKey, iOwnerKey);

    UTCTime tTime;
    Time::GetTime (&tTime);

    int iFlags = 0;

    Variant pvData[SystemEmpireMessages::NumColumns];

    pvData[SystemEmpireMessages::iEmpireKey] = iInviteKey;
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
    }
    else
    {
        iErrCode = GetEmpireName(iUseSourceKey, pvData + SystemEmpireMessages::iSourceName);
        RETURN_ON_ERROR(iErrCode);
    }

    pvData[SystemEmpireMessages::iTimeStamp] = tTime;
    pvData[SystemEmpireMessages::iFlags] = iFlags;
    pvData[SystemEmpireMessages::iText] = (const char*) NULL;
    pvData[SystemEmpireMessages::iType] = MESSAGE_TOURNAMENT_INVITATION;
    pvData[SystemEmpireMessages::iData] = pszTournamentKey;
    Assert(pvData[SystemEmpireMessages::iData].GetCharPtr());

    return DeliverSystemMessage (iInviteKey, pvData);
}

int GameEngine::InviteSelfIntoTournament (unsigned int iTournamentKey, int iEmpireKey) {

    int iErrCode;
    unsigned int iKey;

    Variant pvData[SystemEmpireMessages::NumColumns], vOwnerKey;

    GET_SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);
    
    // Make sure empire isn't already in the tournament
    iErrCode = t_pCache->GetFirstKey(pszEmpires, SystemTournamentEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        return ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT;
    }
    
    // Find owner
    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vOwnerKey);
    RETURN_ON_ERROR(iErrCode);

    // Bit of a hack... Use 'root' for approvals if system tournament
    if (vOwnerKey.GetInteger() == SYSTEM)
    {
        vOwnerKey = global.GetRootKey();
    }

    // Cache the owner key and his message table; no way higher levels can do this for us
    iErrCode = CacheEmpireAndMessages(vOwnerKey.GetInteger());
    RETURN_ON_ERROR(iErrCode);

    char pszTournamentKey [64];
    sprintf(pszTournamentKey, "%i.%i", iTournamentKey, iEmpireKey);

    UTCTime tTime;
    Time::GetTime (&tTime);

    pvData[SystemEmpireMessages::iEmpireKey] = vOwnerKey.GetInteger();
    pvData[SystemEmpireMessages::iUnread] = MESSAGE_UNREAD;
    pvData[SystemEmpireMessages::iSourceKey] = iEmpireKey;

    iErrCode = GetEmpireName(iEmpireKey, pvData + SystemEmpireMessages::iSourceName);
    RETURN_ON_ERROR(iErrCode);

    pvData[SystemEmpireMessages::iTimeStamp] = tTime;
    pvData[SystemEmpireMessages::iFlags] = 0;
    pvData[SystemEmpireMessages::iText] = (const char*) NULL;
    pvData[SystemEmpireMessages::iType] = MESSAGE_TOURNAMENT_JOIN_REQUEST;
    pvData[SystemEmpireMessages::iData] = pszTournamentKey;
    Assert(pvData[SystemEmpireMessages::iData].GetCharPtr());

    return DeliverSystemMessage(vOwnerKey.GetInteger(), pvData);
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
    AutoRelease<ICachedTable> rel1(pTable);

    ICachedTable* pMessages = NULL;
    AutoRelease<ICachedTable> rel2(pMessages);

    GET_SYSTEM_EMPIRE_MESSAGES(strMessages, iEmpireKey);
    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pMessages->ReadData (iMessageKey, SystemEmpireMessages::Type, &iVal);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_MESSAGE_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);
        
    if (iVal != iMessageType)
    {
        return ERROR_MESSAGE_DOES_NOT_EXIST;
    }

    iErrCode = pMessages->ReadData (iMessageKey, SystemEmpireMessages::Data, &vVal);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pMessages->DeleteRow(iMessageKey);
    RETURN_ON_ERROR(iErrCode);

    if (iMessageType == MESSAGE_TOURNAMENT_INVITATION) {

        Assert(sscanf (vVal.GetCharPtr(), "%i.%i", &iTournamentKey, &iOwnerKey) == 2);

        iInviteKey = iEmpireKey;
        iSendMessageKey = iOwnerKey;

        if (iSendMessageKey == SYSTEM) {
            iSendMessageKey = global.GetRootKey();
        }

        pszFormatString = "%s has %s the %s tournament";
        pszYesNo = bAccept ? "joined" : "declined to join";

    } else {

        Assert(sscanf(vVal.GetCharPtr(), "%i.%i", &iTournamentKey, &iInviteKey) == 2);

        iOwnerKey = iEmpireKey;
        iSendMessageKey = iInviteKey;

        pszFormatString = "%s has %s you to join the %s tournament", 
        pszYesNo = bAccept ? "allowed" : "declined to allow";
    }

    // Get the active empire's name
    iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
    RETURN_ON_ERROR(iErrCode);

    SafeRelease (pTable);
    iErrCode = t_pCache->GetTable(SYSTEM_TOURNAMENTS, &pTable);
    RETURN_ON_ERROR(iErrCode);

    // Verify ownership
    iErrCode = pTable->ReadData(iTournamentKey, SystemTournaments::Owner, &vVal);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_TOURNAMENT_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);

    // Get name 
    iErrCode = pTable->ReadData(iTournamentKey, SystemTournaments::Name, &vTourneyName);
    RETURN_ON_ERROR(iErrCode);

    if (vVal.GetInteger() != iOwnerKey) {

        if (vVal.GetInteger() == SYSTEM) {

            int iPrivilege;
            iErrCode = GetEmpirePrivilege (iOwnerKey, &iPrivilege);
            RETURN_ON_ERROR(iErrCode);

            if (iPrivilege >= ADMINISTRATOR) {
                iOwnerKey = SYSTEM;
            }
        }

        if (vVal.GetInteger() != iOwnerKey) {
            return ERROR_TOURNAMENT_DOES_NOT_EXIST;
        }
    }

    // Cache the owner key and his message table; no way higher levels can do this for us
    iErrCode = CacheEmpireMessagesAndTournaments(iSendMessageKey);
    RETURN_ON_ERROR(iErrCode);

    // Finally, do something
    if (bAccept)
    {
        iErrCode = AddEmpireToTournament (iTournamentKey, iInviteKey);
        RETURN_ON_ERROR(iErrCode);
    }

    char pszMessage [MAX_EMPIRE_NAME_LENGTH + MAX_TOURNAMENT_TEAM_NAME_LENGTH + 64];
    sprintf (
        pszMessage, 
        pszFormatString, 
        vEmpireName.GetCharPtr(), 
        pszYesNo, 
        vTourneyName.GetCharPtr()
        );

    iErrCode = SendSystemMessage(iSendMessageKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::AddEmpireToTournament(unsigned int iTournamentKey, int iInviteKey)
{
    int iErrCode;
    unsigned int iProxyKey;

    Variant vVal;

    ICachedTable* pWriteTable = NULL;
    AutoRelease<ICachedTable> rel(pWriteTable);
    
    // Join the tournament!
    GET_SYSTEM_TOURNAMENT_EMPIRES(pszEmpires, iTournamentKey);
    GET_SYSTEM_EMPIRE_TOURNAMENTS(pszTourneys, iInviteKey);
    
    // Add to SystemTournamentEmpires
    Variant pvData[SystemTournamentEmpires::NumColumns] = 
    {
        iTournamentKey, // TournamentKey
        iInviteKey,     // EmpireKey
        NO_KEY,         // TeamKey
        0,              // Wins
        0,              // Nukes
        0,              // Nuked
        0,              // Draws
        0,              // Ruins
    };
    
    iErrCode = t_pCache->GetTable(pszEmpires, &pWriteTable);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = pWriteTable->GetFirstKey(SystemTournamentEmpires::EmpireKey, iInviteKey, &iProxyKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        return ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT;
    }

    iErrCode = pWriteTable->InsertRow(SystemTournamentEmpires::Template, pvData, &iProxyKey);
    RETURN_ON_ERROR(iErrCode);
    
    // Add to SystemEmpireTournaments
    Variant pvSystemEmpireTournaments[SystemEmpireTournaments::NumColumns] = 
    {
        iInviteKey,
        iTournamentKey,
    };
    
    iErrCode = t_pCache->InsertRow(pszTourneys, SystemEmpireTournaments::Template, pvSystemEmpireTournaments, NULL);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::RespondToTournamentJoinRequest (int iEmpireKey, int iMessageKey, bool bAccept) {

    return HandleEmpireTournamentAddition (iEmpireKey, iMessageKey, MESSAGE_TOURNAMENT_JOIN_REQUEST, bAccept);
}


int GameEngine::DeleteEmpireFromTournament (unsigned int iTournamentKey, int iDeleteKey) {

    int iErrCode;
    unsigned int iKey;

    ICachedTable* pEmpires = NULL;
    AutoRelease<ICachedTable> rel(pEmpires);

    Variant vTourneyName;

    GET_SYSTEM_TOURNAMENT_EMPIRES(pszEmpires, iTournamentKey);
    GET_SYSTEM_EMPIRE_TOURNAMENTS(pszTourneys, iDeleteKey);

    char pszMessage [MAX_TOURNAMENT_TEAM_NAME_LENGTH + 96];

    iErrCode = GetTournamentName(iTournamentKey, &vTourneyName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->GetFirstKey(pszEmpires, SystemTournamentEmpires::EmpireKey, iDeleteKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT;
    }
    RETURN_ON_ERROR(iErrCode);

    unsigned int iNumGames;
    Variant** ppvGames = NULL;
    AutoFreeData free_ppvGames(ppvGames);
    iErrCode = GetTournamentGames(iTournamentKey, &ppvGames, &iNumGames);
    RETURN_ON_ERROR(iErrCode);

    for (unsigned int i = 0; i < iNumGames; i ++)
    {
        int iGameClass = ppvGames[i][0].GetInteger();
        int iGameNumber = ppvGames[i][1].GetInteger();
    
        bool bInGame;
        iErrCode = IsEmpireInGame(iGameClass, iGameNumber, iDeleteKey, &bInGame);
        RETURN_ON_ERROR(iErrCode);

        if (bInGame)
        {
            return ERROR_EMPIRE_IS_IN_GAMES;
        }
    }

    // No more empire in tournament
    iErrCode = t_pCache->DeleteRow(pszEmpires, iKey);
    RETURN_ON_ERROR(iErrCode);

    // No more tournament for empire
    iErrCode = t_pCache->GetTable(pszTourneys, &pEmpires);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpires->GetFirstKey(SystemEmpireTournaments::TournamentKey, iTournamentKey, &iKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpires->DeleteRow(iKey);
    RETURN_ON_ERROR(iErrCode);

    SafeRelease (pEmpires);
        
    sprintf (
        pszMessage, 
        "You have been removed from the %s tournament",
        vTourneyName.GetCharPtr()
        );
    
    iErrCode = SendSystemMessage (iDeleteKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetTournamentGameClasses (unsigned int iTournamentKey, unsigned int** ppiGameClassKey,
                                          Variant** ppvName, unsigned int* piNumKeys) {

    int iErrCode;

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    if (ppiGameClassKey != NULL) {
        *ppiGameClassKey = NULL;
    }

    if (ppvName != NULL) {
        *ppvName = NULL;
    }
    *piNumKeys = 0;

    iErrCode = t_pCache->GetTable(SYSTEM_GAMECLASS_DATA, &pTable);
    RETURN_ON_ERROR(iErrCode);

    // Query
    if (ppvName == NULL) {

        iErrCode = pTable->GetEqualKeys(
            SystemGameClassData::TournamentKey,
            iTournamentKey,
            ppiGameClassKey,
            piNumKeys
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
            iErrCode = OK;
        RETURN_ON_ERROR(iErrCode);

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

    return iErrCode;
}

int GameEngine::GetGameClassTournament (int iGameClass, unsigned int* piTournamentKey) {

    int iErrCode;
    Variant vValue;

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::TournamentKey, &vValue);
    RETURN_ON_ERROR(iErrCode);
    
    *piTournamentKey = vValue.GetInteger();

    return iErrCode;
}

int GameEngine::GetTournamentEmpires(unsigned int iTournamentKey, Variant** ppvEmpireKey, Variant** ppvTeamKey, Variant** ppvName, unsigned int* piNumKeys)
{
    int iErrCode;
    unsigned int i;

    Variant* pvEmpireKey = NULL, * pvTeamKey = NULL, * pvName = NULL;
    AutoFreeData free1(pvEmpireKey);
    AutoFreeData free2(pvTeamKey);
    Algorithm::AutoDelete<Variant> del(pvName, true);

    if (ppvEmpireKey)
        *ppvEmpireKey = NULL;

    if (ppvTeamKey)
        *ppvTeamKey = NULL;

    if (ppvName)
        *ppvName = NULL;

    *piNumKeys = 0;

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    GET_SYSTEM_TOURNAMENT_EMPIRES(pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetTable(pszEmpires, &pTable);
    RETURN_ON_ERROR(iErrCode);

    if (ppvEmpireKey)
    {
        iErrCode = pTable->ReadColumn(SystemTournamentEmpires::EmpireKey, NULL, &pvEmpireKey, piNumKeys);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
            return OK;
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = pTable->GetNumCachedRows(piNumKeys);
        RETURN_ON_ERROR(iErrCode);
    }

    if (ppvTeamKey)
    {
        unsigned int iNumKeys2;
        iErrCode = pTable->ReadColumn(SystemTournamentEmpires::TeamKey, NULL, &pvTeamKey, &iNumKeys2);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
            return OK;
        RETURN_ON_ERROR(iErrCode);
    }

    if (ppvName)
    {
        pvName = new Variant[*piNumKeys];
        Assert(pvName);

        for (i = 0; i < *piNumKeys; i ++)
        {
            iErrCode = GetEmpireName(pvEmpireKey[i], pvName + i);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (ppvEmpireKey)
    {
        *ppvEmpireKey = pvEmpireKey;
        pvEmpireKey = NULL;
    }

    if (ppvTeamKey)
    {
        *ppvTeamKey = pvTeamKey;
        pvTeamKey = NULL;
    }

    if (ppvName)
    {
        *ppvName = pvName;
        pvName = NULL;
    }

    return iErrCode;
}


int GameEngine::GetAvailableTournamentEmpires (unsigned int iTournamentKey, Variant** ppvEmpireKey,
                                               Variant** ppvTeamKey, Variant** ppvName, unsigned int* piNumKeys)
{
    int iErrCode, iOptions;
    unsigned int iNumKeys, i;

    Variant* pvName = NULL, * pvTeamKey = NULL, * pvEmpireKey = NULL;
    AutoFreeData free1(pvName);
    AutoFreeData free2(pvTeamKey);
    Algorithm::AutoDelete<Variant> del(pvEmpireKey, true);

    iErrCode = GetTournamentEmpires(
        iTournamentKey,
        &pvEmpireKey,
        !ppvTeamKey ? NULL : &pvTeamKey,
        !ppvName ? NULL : &pvName,
        &iNumKeys
        );

    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumKeys; i ++)
    {
        iErrCode = GetEmpireOptions2(pvEmpireKey[i], &iOptions);
        RETURN_ON_ERROR(iErrCode);
        
        if (iOptions & UNAVAILABLE_FOR_TOURNAMENTS)
        {
            // Remove from list
            pvEmpireKey[i] = pvEmpireKey [iNumKeys - 1];

            if (pvTeamKey != NULL) {
                pvTeamKey[i] = pvTeamKey [iNumKeys - 1];
            }

            if (pvName != NULL) {
                pvName[i] = pvName [iNumKeys - 1];
            }

            iNumKeys --;
        }
    }

    if (ppvEmpireKey)
    {
        *ppvEmpireKey = pvEmpireKey;
        pvEmpireKey = NULL;
    }

    if (ppvTeamKey)
    {
        *ppvTeamKey = pvTeamKey;
        pvTeamKey = NULL;
    }

    if (ppvName)
    {
        *ppvName = pvName;
        pvName = NULL;
    }

    *piNumKeys = iNumKeys;

    return iErrCode;
}


int GameEngine::GetTournamentTeamEmpires (unsigned int iTournamentKey, unsigned int iTeamKey, int** ppiEmpireKey, 
                                          Variant** ppvName, unsigned int* piNumKeys) {

    int iErrCode;
    unsigned int i, * piProxyKey = NULL;
    AutoFreeKeys freeKey(piProxyKey);

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    GET_SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetTable(pszEmpires, &pTable);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->GetEqualKeys(SystemTournamentEmpires::TeamKey, iTeamKey, &piProxyKey, piNumKeys);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
        return OK;
    RETURN_ON_ERROR(iErrCode);

    int* piEmpireKey = NULL;
    Algorithm::AutoDelete<int> free_piEmpireKey(piEmpireKey, true);

    Variant* pvName = NULL;
    Algorithm::AutoDelete<Variant> free_pvName(pvName, true);

    if (ppiEmpireKey)
    {
        piEmpireKey = new int[*piNumKeys];
        Assert(piEmpireKey);

        for (i = 0; i < *piNumKeys; i ++)
        {
            iErrCode = pTable->ReadData (piProxyKey[i], SystemTournamentEmpires::EmpireKey, piEmpireKey + i);
            RETURN_ON_ERROR(iErrCode);
        }
    }   

    if (ppvName)
    {
        pvName = new Variant[*piNumKeys];
        Assert(pvName);

        for (i = 0; i < *piNumKeys; i ++)
        {
            iErrCode = GetEmpireName (piEmpireKey[i], pvName + i);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (ppiEmpireKey)
    {
        *ppiEmpireKey = piEmpireKey;
        piEmpireKey = NULL;
    }

    if (ppvName)
    {
        *ppvName = pvName;
        pvName = NULL;
    }

    return iErrCode;
}


int GameEngine::GetTournamentTeams (unsigned int iTournamentKey, unsigned int** ppiTeamKey, Variant** ppvName, unsigned int* piNumKeys)
{
    int iErrCode;

    GET_SYSTEM_TOURNAMENT_TEAMS(pszTeams, iTournamentKey);

    if (ppvName == NULL)
    {
        if (ppiTeamKey == NULL)
        {
            iErrCode = t_pCache->GetNumCachedRows(pszTeams, piNumKeys);
            RETURN_ON_ERROR(iErrCode);
        }
        else
        {
            iErrCode = t_pCache->GetAllKeys (pszTeams, ppiTeamKey, piNumKeys);
            if (iErrCode == ERROR_DATA_NOT_FOUND)
                iErrCode = OK;
            RETURN_ON_ERROR(iErrCode);
        }
    }
    else
    {
        iErrCode = t_pCache->ReadColumn(pszTeams, SystemTournamentTeams::Name, ppiTeamKey, ppvName, piNumKeys);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
            iErrCode = OK;
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::CreateTournamentTeam(unsigned int iTournamentKey, Variant* pvTeamData, unsigned int* piTeamKey) {

    int iErrCode;
    unsigned int iKey;

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    iErrCode = t_pCache->GetTable(pszTeams, &pTable);
    RETURN_ON_ERROR(iErrCode);

    // Make sure team doesn't already exist
    iErrCode = pTable->GetFirstKey(
        SystemTournamentTeams::Name, 
        pvTeamData[SystemTournamentTeams::iName].GetCharPtr(),
        &iKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        return ERROR_TOURNAMENT_TEAM_ALREADY_EXISTS;
    }

    // Create team
    iErrCode = pTable->InsertRow(SystemTournamentTeams::Template, pvTeamData, piTeamKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;

}

int GameEngine::DeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) {

    int iErrCode, iTryTeamKey;
    unsigned int iKey = NO_KEY;

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);
    GET_SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetTable(pszEmpires, &pTable);
    RETURN_ON_ERROR(iErrCode);

    // Remove empires from team
    while (pTable->GetNextKey (iKey, &iKey) != ERROR_DATA_NOT_FOUND) {

        RETURN_ON_ERROR(iErrCode);

        iErrCode = pTable->ReadData (iKey, SystemTournamentEmpires::TeamKey, &iTryTeamKey);
        RETURN_ON_ERROR(iErrCode);

        if ((unsigned int) iTryTeamKey == iTeamKey) {

            iErrCode = pTable->WriteData(iKey, SystemTournamentEmpires::TeamKey, (int) NO_KEY);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    iErrCode = t_pCache->DeleteRow(pszTeams, iTeamKey);
    RETURN_ON_ERROR(iErrCode);

    global.GetEventSink()->OnDeleteTournamentTeam (iTournamentKey, iTeamKey);

    return iErrCode;
}

int GameEngine::SetEmpireTournamentTeam (unsigned int iTournamentKey, int iEmpireKey, unsigned int iTeamKey) {

    int iErrCode;
    unsigned int iKey;

    ICachedTable* pWriteTable = NULL;
    AutoRelease<ICachedTable> rel(pWriteTable);

    GET_SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    if (iTeamKey != NO_KEY)
    {
        // Ensure team exists
        GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);
        Variant vTemp;
        iErrCode = t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::Draws, &vTemp);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
            return ERROR_TOURNAMENT_TEAM_DOES_NOT_EXIST;
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->GetTable(pszEmpires, &pWriteTable);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pWriteTable->GetFirstKey(SystemTournamentEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pWriteTable->WriteData(iKey, SystemTournamentEmpires::TeamKey, (int) iTeamKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetTournamentTeamData (unsigned int iTournamentKey, unsigned int iTeamKey, Variant** ppvTournamentTeamData)
{
    GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);
    int iErrCode = t_pCache->ReadRow (pszTeams, iTeamKey, ppvTournamentTeamData);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_TOURNAMENT_TEAM_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::GetTournamentEmpireData (unsigned int iTournamentKey, unsigned int iEmpireKey, Variant** ppvTournamentEmpireData)
{
    int iErrCode;
    unsigned int iKey;

    GET_SYSTEM_TOURNAMENT_EMPIRES(pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetFirstKey(
        pszEmpires, 
        SystemTournamentEmpires::EmpireKey,
        iEmpireKey,
        &iKey
        );

    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadRow (pszEmpires, iKey, ppvTournamentEmpireData);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


int GameEngine::GetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                              Variant* pvTournamentTeamDesc) {

    GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::Description, pvTournamentTeamDesc);
}

int GameEngine::SetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                              const char* pszTournamentTeamDesc) {

    GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->WriteData(pszTeams, iTeamKey, SystemTournamentTeams::Description, pszTournamentTeamDesc);
}

int GameEngine::GetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                      Variant* pvTournamentTeamUrl) {

    GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::WebPage, pvTournamentTeamUrl);
}

int GameEngine::SetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, const char* pszTournamentTeamUrl)
{
    GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);
    return t_pCache->WriteData(pszTeams, iTeamKey, SystemTournamentTeams::WebPage, pszTournamentTeamUrl);
}

int GameEngine::GetTournamentIcon (unsigned int iTournamentKey, unsigned int* piIcon, int* piIconAddress)
{
    int iErrCode;
    Variant vVal;

    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Icon, &vVal);
    RETURN_ON_ERROR(iErrCode);
    *piIcon = vVal.GetInteger();

    iErrCode = t_pCache->ReadData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::IconAddress, &vVal);
    RETURN_ON_ERROR(iErrCode);
    *piIconAddress = vVal.GetInteger();

    return iErrCode;
}

int GameEngine::SetTournamentIcon (unsigned int iTournamentKey, unsigned int iIcon)
{
    int iErrCode;

    int iAddress = -1;
    if (iIcon != UPLOADED_ICON)
    {
        iErrCode = GetAlienIconAddress(iIcon, &iAddress);
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->WriteData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Icon, (int)iIcon);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::IconAddress, iAddress);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int* piIcon, int* piAddress)
{
    int iErrCode;
    Variant vVal;

    GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    iErrCode = t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::Icon, &vVal);
    RETURN_ON_ERROR(iErrCode);
    *piIcon = vVal.GetInteger();

    iErrCode = t_pCache->ReadData(pszTeams, iTeamKey, SystemTournamentTeams::IconAddress, &vVal);
    RETURN_ON_ERROR(iErrCode);
    *piAddress = vVal.GetInteger();

    return iErrCode;
}

int GameEngine::SetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int iIcon)
{
    GET_SYSTEM_TOURNAMENT_TEAMS(strTeams, iTournamentKey);

    int iErrCode;

    int iAddress = -1;
    if (iIcon != UPLOADED_ICON)
    {
        iErrCode = GetAlienIconAddress(iIcon, &iAddress);
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->WriteData(strTeams, iTeamKey, SystemTournamentTeams::Icon, (int)iIcon);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strTeams, iTeamKey, SystemTournamentTeams::IconAddress, iAddress);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetTournamentGames(unsigned int iTournamentKey, Variant*** pppvGames, unsigned int* piNumGames)
{
    int iErrCode;

    if (pppvGames)
        *pppvGames = NULL;

    const char* ppszColumns[] = { SystemActiveGames::GameClass, SystemActiveGames::GameNumber };

    iErrCode = t_pCache->ReadColumnsWhereEqual(SYSTEM_ACTIVE_GAMES, SystemActiveGames::TournamentKey, iTournamentKey, 
                                               ppszColumns, countof(ppszColumns), NULL, pppvGames, piNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}