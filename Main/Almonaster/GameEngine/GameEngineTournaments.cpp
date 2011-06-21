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

    IReadTable* pTournaments = NULL;

    if (ppiTournamentKey != NULL) {
        *ppiTournamentKey = NULL;
    }

    if (ppvName != NULL) {
        *ppvName = NULL;
    }

    *piNumTournaments = 0;

    iErrCode = t_pConn->GetTableForReading (SYSTEM_TOURNAMENTS, &pTournaments);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Query
    if (ppvName == NULL) {

        iErrCode = pTournaments->GetEqualKeys (
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


int GameEngine::GetJoinedTournaments (int iEmpireKey, unsigned int** ppiTournamentKey, Variant** ppvName, 
                                      unsigned int* piNumTournaments) {

    int iErrCode;
    unsigned int* piTournamentKey = NULL, i, iNumTournaments;

    SYSTEM_EMPIRE_TOURNAMENTS (pszEmpTournaments, iEmpireKey);

    IReadTable* pReadTable = NULL;

    //
    if (ppiTournamentKey != NULL) {
        *ppiTournamentKey = NULL;
    }

    if (ppvName != NULL) {
        *ppvName = NULL;
    }

    *piNumTournaments = 0;

    // Read keys
    iErrCode = t_pConn->GetTableForReading (pszEmpTournaments, &pReadTable);
    if (iErrCode != OK) {

        if (iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
            iErrCode = OK;
        }
        else Assert (false);
        goto Cleanup;
    }

    if (ppiTournamentKey == NULL && ppvName == NULL) {

        iErrCode = pReadTable->GetNumRows (&iNumTournaments);
    
    } else {

        iErrCode = pReadTable->ReadColumn (
            SystemEmpireTournaments::TournamentKey,
            (int**) &piTournamentKey,
            &iNumTournaments
            );
    }

    SafeRelease (pReadTable);

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (ppvName != NULL) {

        *ppvName = new Variant [iNumTournaments];
        if (*ppvName == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        iErrCode = t_pConn->GetTableForReading (SYSTEM_TOURNAMENTS, &pReadTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        for (i = 0; i < iNumTournaments; i ++) {

            iErrCode = pReadTable->ReadData (piTournamentKey[i], SystemTournaments::Name, (*ppvName) + i);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        SafeRelease (pReadTable);
    }

    if (ppiTournamentKey != NULL) {
        *ppiTournamentKey = piTournamentKey;
        piTournamentKey = NULL;
    }

    *piNumTournaments = iNumTournaments;

Cleanup:

    SafeRelease (pReadTable);

    if (piTournamentKey != NULL) {
        t_pConn->FreeData(piTournamentKey);
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

    IWriteTable* pTournaments = NULL;

    int iOwner = pvTournamentData [SystemTournaments::iOwner].GetInteger();
    Variant vLimit;

    if (iOwner != SYSTEM) {

        iErrCode = GetSystemProperty (SystemData::MaxNumPersonalTournaments, &vLimit);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    iErrCode = t_pConn->GetTableForWriting (SYSTEM_TOURNAMENTS, &pTournaments);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Make sure we're under the limit for personal tournaments
    if (iOwner != SYSTEM) {

        unsigned int iNumEqual;

        iErrCode = pTournaments->GetEqualKeys (
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
    iErrCode = pTournaments->GetFirstKey (
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

    iErrCode = t_pConn->CreateTable (pszTable, SystemTournamentTeams::Template);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    GET_SYSTEM_TOURNAMENT_EMPIRES (pszTable, *piTournamentKey);

    iErrCode = t_pConn->CreateTable (pszTable, SystemTournamentEmpires::Template);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    GET_SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszTable, *piTournamentKey);

    iErrCode = t_pConn->CreateTable (pszTable, SystemTournamentActiveGames::Template);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    GET_SYSTEM_TOURNAMENT_LATEST_GAMES (pszTable, *piTournamentKey);

    iErrCode = t_pConn->CreateTable (pszTable, SystemTournamentLatestGames::Template);
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
    bool bFlag;

    Assert (!bOwnerDeleted || iEmpireKey != DELETED_EMPIRE_KEY);
    
    // Check existence
    iErrCode = t_pConn->DoesRowExist (SYSTEM_TOURNAMENTS, iTournamentKey, &bFlag);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (!bFlag) {
        iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Make sure correct owner
    iErrCode = t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vTemp);
    if (iErrCode != OK) {
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
            iErrCode = t_pConn->WriteData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, (int) DELETED_EMPIRE_KEY);
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
    iErrCode = t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Name, &vTournamentName);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->DeleteRow (SYSTEM_TOURNAMENTS, iTournamentKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Delete the tables
    GET_SYSTEM_TOURNAMENT_TEAMS (pszTable, iTournamentKey);

    iErrCode = t_pConn->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    GET_SYSTEM_TOURNAMENT_EMPIRES (pszTable, iTournamentKey);

    sprintf (pszMessage, "The %s tournament was deleted", vTournamentName.GetCharPtr());

    // Try to delete gameclasses if any left
    iErrCode = t_pConn->GetEqualKeys (
        SYSTEM_GAMECLASS_DATA,
        SystemGameClassData::TournamentKey,
        iTournamentKey,
        &piGameClassKey,
        &iGameClasses
        );

    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Best effort
        for (i = 0; i < iGameClasses; i ++) {

            iErrCode = DeleteGameClass (piGameClassKey[i], &bFlag);
            Assert (iErrCode == OK);
        }

        t_pConn->FreeKeys(piGameClassKey);
    }

    // Remove empires from tournament
    iKey = NO_KEY;
    while (true) {

        unsigned int iEmpireTourneyKey = NO_KEY;
        char pszEmpireTournaments [256];

        iErrCode = t_pConn->GetNextKey (pszTable, iKey, &iKey);
        if (iErrCode != OK) {
            Assert (iErrCode == ERROR_DATA_NOT_FOUND);
            break;
        }

        iErrCode = t_pConn->ReadData (pszTable, iKey, SystemTournamentEmpires::EmpireKey, &vKey);
        if (iErrCode != OK) {
            Assert (false);
            break;
        }

        GET_SYSTEM_EMPIRE_TOURNAMENTS (pszEmpireTournaments, vKey.GetInteger());

        iErrCode = t_pConn->GetFirstKey (
            pszEmpireTournaments, 
            SystemEmpireTournaments::TournamentKey,
            iTournamentKey,
            &iEmpireTourneyKey
            );

        if (iErrCode != OK) {
            Assert (false);
            continue;
        }

        iErrCode = t_pConn->DeleteRow (pszEmpireTournaments, iEmpireTourneyKey);
        if (iErrCode != OK) {
            Assert (false);
            continue;
        }

        SendSystemMessage (vKey.GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
    }

    iErrCode = t_pConn->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    GET_SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszTable, iTournamentKey);

    Assert (t_pConn->GetNumRows (pszTable, &iGameClasses) == OK && iGameClasses == 0);

    iErrCode = t_pConn->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    GET_SYSTEM_TOURNAMENT_LATEST_GAMES (pszTable, iTournamentKey);

    iErrCode = t_pConn->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    //
    // Notify the UI
    //

    global.GetEventSink()->OnDeleteTournament (iTournamentKey);

Cleanup:

    return iErrCode;
}

int GameEngine::GetTournamentData (unsigned int iTournamentKey, Variant** ppvTournamentData) {

    return t_pConn->ReadRow (SYSTEM_TOURNAMENTS, iTournamentKey, ppvTournamentData);
}

int GameEngine::GetTournamentName (unsigned int iTournamentKey, Variant* pvTournamentName) {

    int iErrCode;
    bool bFlag;

    iErrCode = t_pConn->DoesRowExist (SYSTEM_TOURNAMENTS, iTournamentKey, &bFlag);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!bFlag) {
        return ERROR_TOURNAMENT_DOES_NOT_EXIST;
    }

    return t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Name, pvTournamentName);
}

int GameEngine::GetTournamentOwner (unsigned int iTournamentKey, unsigned int* piOwnerKey) {

    int iErrCode;
    
    IReadTable* pTable = NULL;

    iErrCode = t_pConn->GetTableForReading (SYSTEM_TOURNAMENTS, &pTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    bool bExists;
    iErrCode = pTable->DoesRowExist (iTournamentKey, &bExists);
    if (iErrCode != OK || !bExists) {
        iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (iTournamentKey, SystemTournaments::Owner, (int*) piOwnerKey);

Cleanup:

    SafeRelease (pTable);

    return iErrCode;
}

int GameEngine::GetTournamentDescription (unsigned int iTournamentKey, Variant* pvTournamentDesc) {

    return t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Description, pvTournamentDesc);
}

int GameEngine::SetTournamentDescription (unsigned int iTournamentKey, const char* pszTournamentDesc) {

    return SetTournamentString (iTournamentKey, SystemTournaments::Description, pszTournamentDesc);
}

int GameEngine::GetTournamentUrl (unsigned int iTournamentKey, Variant* pvTournamentUrl) {

    return t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::WebPage, pvTournamentUrl);
}

int GameEngine::SetTournamentUrl (unsigned int iTournamentKey, const char* pszTournamentUrl) {

    return SetTournamentString (iTournamentKey, SystemTournaments::WebPage, pszTournamentUrl);
}

int GameEngine::GetTournamentNews (unsigned int iTournamentKey, Variant* pvTournamentNews) {

    return t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::News, pvTournamentNews);
}

int GameEngine::SetTournamentNews (unsigned int iTournamentKey, const char* pszTournamentNews) {

    return SetTournamentString (iTournamentKey, SystemTournaments::News, pszTournamentNews);
}

int GameEngine::SetTournamentString (unsigned int iTournamentKey, const char* pszColumn, const char* pszString) {

    int iErrCode;
    IWriteTable* pWrite = NULL;

    iErrCode = t_pConn->GetTableForWriting (SYSTEM_TOURNAMENTS, &pWrite);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pWrite->WriteData (iTournamentKey, pszColumn, pszString);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pWrite);

    return iErrCode;
}

int GameEngine::InviteEmpireIntoTournament (unsigned int iTournamentKey, int iOwnerKey, int iSourceKey,
                                            int iInviteKey) {

    int iErrCode;
    unsigned int iKey;

    Variant pvData[SystemEmpireMessages::NumColumns], vOwnerKey;

    // Verify access
    iErrCode = t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vOwnerKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (vOwnerKey.GetInteger() != iOwnerKey) {
        return ERROR_ACCESS_DENIED;
    }

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);
    
    // Make sure empire isn't already in the tournament
    iErrCode = t_pConn->GetFirstKey (pszEmpires, SystemTournamentEmpires::EmpireKey, iInviteKey, &iKey);
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

    if (iOwnerKey == SYSTEM) {

        iErrCode = GetEmpireName (iSourceKey, pvData + SystemEmpireMessages::iSource);
        if (iErrCode != OK) {
            iFlags |= MESSAGE_SYSTEM;
            pvData[SystemEmpireMessages::iSource] = (const char*) NULL;
        }
    }
    
    else if (GetEmpireName (iOwnerKey, pvData + SystemEmpireMessages::iSource) != OK) {
        pvData[SystemEmpireMessages::iSource] = "Unknown";
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
    iErrCode = t_pConn->GetFirstKey (
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
    iErrCode = t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Owner, &vOwnerKey);
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
    iErrCode = GetEmpireName (iEmpireKey, pvData + SystemEmpireMessages::iSource);
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

int GameEngine::HandleEmpireTournamentAddition (int iEmpireKey, int iMessageKey, int iMessageType, 
                                                bool bAccept) {

    int iErrCode, iOwnerKey, iInviteKey, iSendMessageKey = NO_KEY, iVal;
    unsigned int iTournamentKey;
    const char* pszFormatString = NULL, * pszYesNo = NULL;

    bool bExists;
    Variant vVal, vEmpireName, vTourneyName;

    IReadTable* pTable = NULL;
    IWriteTable* pMessages = NULL;

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    bool bSendMessage = false;

    iErrCode = t_pConn->GetTableForWriting (pszMessages, &pMessages);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
            iErrCode = ERROR_MESSAGE_DOES_NOT_EXIST;
        }
        return iErrCode;
    }

    iErrCode = pMessages->DoesRowExist (iMessageKey, &bExists);
    if (iErrCode != OK || !bExists) {
        iErrCode = ERROR_MESSAGE_DOES_NOT_EXIST;
        goto Cleanup;
    }

    iErrCode = pMessages->ReadData (iMessageKey, SystemEmpireMessages::Type, &iVal);
    if (iErrCode != OK || iVal != iMessageType) {
        iErrCode = ERROR_MESSAGE_DOES_NOT_EXIST;
        goto Cleanup;
    }

    iErrCode = pMessages->ReadData (iMessageKey, SystemEmpireMessages::Data, &vVal);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pMessages->DeleteRow (iMessageKey);
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

    iErrCode = t_pConn->GetTableForReading (SYSTEM_TOURNAMENTS, &pTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->DoesRowExist (iTournamentKey, &bExists);
    if (iErrCode != OK || !bExists) {
        iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Verify ownership
    iErrCode = pTable->ReadData (iTournamentKey, SystemTournaments::Owner, &vVal);
    if (iErrCode != OK) {
        iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Get name 
    iErrCode = pTable->ReadData (iTournamentKey, SystemTournaments::Name, &vTourneyName);
    if (iErrCode != OK) {
        iErrCode = ERROR_TOURNAMENT_DOES_NOT_EXIST;
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

int GameEngine::AddEmpireToTournament (unsigned int iTournamentKey, int iInviteKey) {

    int iErrCode;
    bool bExists;
    unsigned int iProxyKey;

    Variant vVal;

    IWriteTable* pWriteTable = NULL;
    
    // Join the tournament!
    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);
    SYSTEM_EMPIRE_TOURNAMENTS (pszTourneys, iInviteKey);
    
    // Add to SystemTournamentEmpires
    Variant pvData[SystemTournamentEmpires::NumColumns] = {
        iInviteKey,     // EmpireKey
            NO_KEY,         // TeamKey
            0,              // Wins
            0,              // Nukes
            0,              // Nuked
            0,              // Draws
            0,              // Ruins
    };
    
    iErrCode = t_pConn->GetTableForWriting (pszEmpires, &pWriteTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pWriteTable->GetFirstKey (SystemTournamentEmpires::EmpireKey, iInviteKey, &iProxyKey);
    if (iErrCode == OK) {
        iErrCode = ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT;
        goto Cleanup;
    }
    
    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pWriteTable->InsertRow (SystemTournamentEmpires::Template, pvData, &iProxyKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    SafeRelease (pWriteTable);
    
    // Add to SystemEmpireTournaments
    bExists = t_pConn->DoesTableExist (pszTourneys);
    if (!bExists) {
        
        // Fault in table
        iErrCode = t_pConn->CreateTable (pszTourneys, SystemEmpireTournaments::Template);
        if (iErrCode != OK && iErrCode != ERROR_TABLE_ALREADY_EXISTS) {
            t_pConn->DeleteRow (pszEmpires, iProxyKey);
            Assert (false);
            goto Cleanup;
        }
    }
    
    vVal = iTournamentKey;
    
    iErrCode = t_pConn->InsertRow (pszTourneys, SystemEmpireTournaments::Template, &vVal, NULL);
    if (iErrCode != OK) {
        t_pConn->DeleteRow (pszEmpires, iProxyKey);
        Assert (false);
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

    IWriteTable* pEmpires = NULL;

    Variant vTourneyName, * pvGameClassGameNumber = NULL;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);
    SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszGames, iTournamentKey);
    SYSTEM_EMPIRE_TOURNAMENTS (pszTourneys, iDeleteKey);

    char pszMessage [MAX_TOURNAMENT_TEAM_NAME_LENGTH + 96];

    iErrCode = GetTournamentName (iTournamentKey, &vTourneyName);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = t_pConn->GetFirstKey (pszEmpires, SystemTournamentEmpires::EmpireKey, iDeleteKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->ReadColumn (
        pszGames, 
        SystemTournamentActiveGames::GameClassGameNumber,
        &pvGameClassGameNumber,
        &iNumGames
        );

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
    iErrCode = t_pConn->DeleteRow (pszEmpires, iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // No more tournament for empire
    iErrCode = t_pConn->GetTableForWriting (pszTourneys, &pEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpires->GetFirstKey (SystemEmpireTournaments::TournamentKey, (int) iTournamentKey, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpires->DeleteRow (iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pEmpires);
        
    sprintf (
        pszMessage, 
        "You have been removed from the %s tournament",
        vTourneyName.GetCharPtr()
        );
    
    SendSystemMessage (iDeleteKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);

Cleanup:

    SafeRelease (pEmpires);

    if (pvGameClassGameNumber != NULL) {
        t_pConn->FreeData(pvGameClassGameNumber);
    }

    return iErrCode;
}

int GameEngine::GetTournamentGameClasses (unsigned int iTournamentKey, unsigned int** ppiGameClassKey,
                                          Variant** ppvName, unsigned int* piNumKeys) {

    int iErrCode;

    IReadTable* pTable = NULL;

    if (ppiGameClassKey != NULL) {
        *ppiGameClassKey = NULL;
    }

    if (ppvName != NULL) {
        *ppvName = NULL;
    }
    *piNumKeys = 0;

    iErrCode = t_pConn->GetTableForReading (SYSTEM_GAMECLASS_DATA, &pTable);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Query
    if (ppvName == NULL) {

        iErrCode = pTable->GetEqualKeys (
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

    iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::TournamentKey, &vValue);
    if (iErrCode == OK) {
        *piTournamentKey = vValue.GetInteger();
    }

    return iErrCode;
}

int GameEngine::GetTournamentEmpires (unsigned int iTournamentKey, unsigned int** ppiEmpireKey,
                                      unsigned int** ppiTeamKey, Variant** ppvName, unsigned int* piNumKeys) {

    int iErrCode;
    unsigned int i;

    IReadTable* pTable = NULL;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pConn->GetTableForReading (pszEmpires, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (ppiEmpireKey != NULL) {

        iErrCode = pTable->ReadColumn (SystemTournamentEmpires::EmpireKey, (int**) ppiEmpireKey, piNumKeys);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            goto Cleanup;
        }

    } else {

        iErrCode = pTable->GetNumRows (piNumKeys);
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (ppiTeamKey != NULL) {

        unsigned int iNumKeys2;

        iErrCode = pTable->ReadColumn (SystemTournamentEmpires::TeamKey, (int**) ppiTeamKey, &iNumKeys2);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }

        Assert (iNumKeys2 == *piNumKeys);
    }

    SafeRelease (pTable);

    if (ppvName != NULL) {

        *ppvName = new Variant [*piNumKeys];
        if (*ppvName == NULL) {

            t_pConn->FreeData(*ppiEmpireKey);
            *ppiEmpireKey = NULL;

            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        for (i = 0; i < *piNumKeys; i ++) {

            int ec = GetEmpireName ((*ppiEmpireKey)[i], (*ppvName) + i);
            if (ec != OK) {
                (*ppiEmpireKey)[i] = (*ppiEmpireKey)[*piNumKeys - 1];
                i --;
            }
        }
    }

Cleanup:

    if (iErrCode != OK || (iErrCode == OK && *piNumKeys == 0)) {
        
        if (ppiEmpireKey != NULL && *ppiEmpireKey != NULL) {
            t_pConn->FreeData(*ppiEmpireKey);
            *ppiEmpireKey = NULL;
        }
        
        if (ppvName != NULL && *ppvName != NULL) {
            delete [] (*ppvName);
            *ppvName = NULL;
        }
        
        if (ppiTeamKey != NULL && *ppiTeamKey != NULL) {
            t_pConn->FreeData(*ppiTeamKey);
            *ppiTeamKey = NULL;
        }
    }

    SafeRelease (pTable);

    return iErrCode;
}


int GameEngine::GetAvailableTournamentEmpires (unsigned int iTournamentKey, unsigned int** ppiEmpireKey,
                                               unsigned int** ppiTeamKey, Variant** ppvName, 
                                               unsigned int* piNumKeys) {

    int iErrCode, iOptions;
    unsigned int* piTeamKey = NULL, iNumKeys = 0, i, * piEmpireKey = NULL;
    Variant* pvName = NULL;

    iErrCode = GetTournamentEmpires (
        iTournamentKey,
        ppiEmpireKey == NULL ? NULL : &piEmpireKey,
        ppiTeamKey == NULL ? NULL : &piTeamKey,
        ppvName == NULL ? NULL : &pvName,
        &iNumKeys
        );

    if (iErrCode != OK) {
        goto Cleanup;
    }

    for (i = 0; i < iNumKeys; i ++) {

        if (GetEmpireOptions2 (piEmpireKey[i], &iOptions) != OK || 
            (iOptions & UNAVAILABLE_FOR_TOURNAMENTS)) {

            // Remove from list
            if (piEmpireKey != NULL) {
                piEmpireKey[i] = piEmpireKey [iNumKeys - 1];
            }

            if (piTeamKey != NULL) {
                piTeamKey[i] = piTeamKey [iNumKeys - 1];
            }

            if (pvName != NULL) {
                pvName[i] = pvName [iNumKeys - 1];
            }

            iNumKeys --;
        }
    }

Cleanup:

    if (iErrCode == OK && iNumKeys == 0) {

        if (piEmpireKey != NULL) {
            t_pConn->FreeData(piEmpireKey);
            piEmpireKey = NULL;
        }

        if (piTeamKey != NULL) {
            t_pConn->FreeData(piTeamKey);
            piTeamKey = NULL;
        }

        if (pvName != NULL) {
            delete [] pvName;
            pvName = NULL;
        }
    }

    if (ppiEmpireKey != NULL) {
        *ppiEmpireKey = piEmpireKey;
    }

    if (ppiTeamKey != NULL) {
        *ppiTeamKey = piTeamKey;
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

    IReadTable* pTable = NULL;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pConn->GetTableForReading (pszEmpires, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->GetEqualKeys (SystemTournamentEmpires::TeamKey, iTeamKey, &piProxyKey, piNumKeys);
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

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    if (ppvName == NULL) {

        if (ppiTeamKey == NULL) {
            iErrCode = t_pConn->GetNumRows (pszTeams, piNumKeys);
        } else {
            iErrCode = t_pConn->GetAllKeys (pszTeams, ppiTeamKey, piNumKeys);
        }

    } else {

        iErrCode = t_pConn->ReadColumn (pszTeams, SystemTournamentTeams::Name, ppiTeamKey, ppvName, piNumKeys);
    }

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}

int GameEngine::CreateTournamentTeam (unsigned int iTournamentKey, Variant* pvTeamData, unsigned int* piTeamKey) {

    int iErrCode;
    unsigned int iKey;

    IWriteTable* pTable = NULL;

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    iErrCode = t_pConn->GetTableForWriting (pszTeams, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Make sure team doesn't already exist
    iErrCode = pTable->GetFirstKey (
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

    IWriteTable* pTable = NULL;

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);
    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pConn->GetTableForWriting (pszEmpires, &pTable);
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

            iErrCode = pTable->WriteData (iKey, SystemTournamentEmpires::TeamKey, (int) NO_KEY);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    SafeRelease (pTable);

    iErrCode = t_pConn->DeleteRow (pszTeams, iTeamKey);
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

    IWriteTable* pWriteTable = NULL;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    if (iTeamKey != NO_KEY) {

        bool bExists;
        SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

        iErrCode = t_pConn->DoesRowExist (pszTeams, iTeamKey, &bExists);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (!bExists) {
            iErrCode = ERROR_TOURNAMENT_TEAM_DOES_NOT_EXIST;
            goto Cleanup;
        }
    }

    iErrCode = t_pConn->GetTableForWriting (pszEmpires, &pWriteTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pWriteTable->GetFirstKey (SystemTournamentEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pWriteTable->WriteData (iKey, SystemTournamentEmpires::TeamKey, (int) iTeamKey);
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

    return t_pConn->ReadRow (pszTeams, iTeamKey, ppvTournamentTeamData);
}

int GameEngine::GetTournamentEmpireData (unsigned int iTournamentKey, unsigned int iEmpireKey, 
                                         Variant** ppvTournamentEmpireData) {

    int iErrCode;
    unsigned int iKey;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pConn->GetFirstKey (
        pszEmpires, 
        SystemTournamentEmpires::EmpireKey,
        iEmpireKey,
        &iKey
        );

    if (iErrCode == OK) {
        iErrCode = t_pConn->ReadRow (pszEmpires, iKey, ppvTournamentEmpireData);
    }

    return iErrCode;
}


int GameEngine::GetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                              Variant* pvTournamentTeamDesc) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pConn->ReadData (pszTeams, iTeamKey, SystemTournamentTeams::Description, pvTournamentTeamDesc);
}

int GameEngine::SetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                              const char* pszTournamentTeamDesc) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pConn->WriteData (pszTeams, iTeamKey, SystemTournamentTeams::Description, pszTournamentTeamDesc);
}

int GameEngine::GetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                      Variant* pvTournamentTeamUrl) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pConn->ReadData (pszTeams, iTeamKey, SystemTournamentTeams::WebPage, pvTournamentTeamUrl);
}

int GameEngine::SetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, 
                                      const char* pszTournamentTeamUrl) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pConn->WriteData (pszTeams, iTeamKey, SystemTournamentTeams::WebPage, pszTournamentTeamUrl);
}

int GameEngine::GetTournamentIcon (unsigned int iTournamentKey, unsigned int* piIcon) {

    int iErrCode;
    Variant vVal;

    iErrCode = t_pConn->ReadData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Icon, &vVal);
    if (iErrCode == OK) {
        *piIcon = vVal.GetInteger();
    }

    return iErrCode;
}

int GameEngine::SetTournamentIcon (unsigned int iTournamentKey, unsigned int iIcon) {

    return t_pConn->WriteData (SYSTEM_TOURNAMENTS, iTournamentKey, SystemTournaments::Icon, iIcon);
}

int GameEngine::GetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int* piIcon) {

    int iErrCode;
    Variant vVal;

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    iErrCode = t_pConn->ReadData (pszTeams, iTeamKey, SystemTournamentTeams::Icon, &vVal);
    if (iErrCode == OK) {
        *piIcon = vVal.GetInteger();
    }

    return iErrCode;
}

int GameEngine::SetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int iIcon) {

    SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

    return t_pConn->WriteData (pszTeams, iTeamKey, SystemTournamentTeams::Icon, iIcon);
}


int GameEngine::GetTournamentGames (unsigned int iTournamentKey, int** ppiGameClass, int** ppiGameNumber, 
                                    unsigned int* piNumGames) {
    int iErrCode;
    unsigned int iKey = NO_KEY, i = 0;

    IReadTable* pTable = NULL;

    SYSTEM_TOURNAMENT_ACTIVE_GAMES (pszGames, iTournamentKey);

    Assert (ppiGameClass != NULL && ppiGameNumber != NULL || ppiGameClass == NULL && ppiGameNumber == NULL);

    if (ppiGameClass == NULL) {
        return t_pConn->GetNumRows (pszGames, piNumGames);
    }

    iErrCode = t_pConn->GetTableForReading (pszGames, &pTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->GetNumRows (piNumGames);
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