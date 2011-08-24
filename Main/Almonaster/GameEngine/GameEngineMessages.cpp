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

#include "GameEngine.h"

// Input:
// iEmpireKey -> Integer key of destination empire
// pszMessage -> Message to be sent to empire
// iSource -> Sender of message
// bBroadcast -> true if message was broadcast, false if sent
//
// Add a System Message to a given empire's queue.

int GameEngine::SendSystemMessage (int iEmpireKey, const char* pszMessage, int iSource, int iFlags) {

    int iErrCode;

    // Make sure source can broadcast && send system messages
    if (iSource != SYSTEM) {

        bool bBroadcast;
        iErrCode = GetEmpireOption (iSource, CAN_BROADCAST, &bBroadcast);
        if (iErrCode != OK || !bBroadcast) {
            return ERROR_CANNOT_SEND_MESSAGE;
        }
    }

    Variant pvData [SystemEmpireMessages::NumColumns];

    UTCTime tTime;
    Time::GetTime (&tTime);
    
    // Make sure empire exists
    bool bFlag;
    iErrCode = DoesEmpireExist (iEmpireKey, &bFlag, NULL);
    if (iErrCode != OK || !bFlag) {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    // Insert the message into the table
    pvData[SystemEmpireMessages::iUnread] = MESSAGE_UNREAD;

    if (iSource == SYSTEM) {
        Assert (iFlags & MESSAGE_SYSTEM);
        pvData[SystemEmpireMessages::iSource] = (const char*) NULL;
    } else {
        
        if (t_pCache->ReadData(
            SYSTEM_EMPIRE_DATA, 
            iSource, 
            SystemEmpireData::Name, 
            pvData + SystemEmpireMessages::iSource
            ) != OK) {          
            pvData[SystemEmpireMessages::iSource] = "Unknown";
        }

        if (pvData[SystemEmpireMessages::iSource].GetCharPtr() == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
    }

    pvData[SystemEmpireMessages::iTimeStamp] = tTime;
    pvData[SystemEmpireMessages::iFlags] = iFlags;
    pvData[SystemEmpireMessages::iText] = pszMessage;
    pvData[SystemEmpireMessages::iType] = MESSAGE_NORMAL;
    pvData[SystemEmpireMessages::iData] = (const char*) NULL;

    if (pvData[SystemEmpireMessages::iText].GetCharPtr() == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    return DeliverSystemMessage (iEmpireKey, pvData);
}


int GameEngine::DeliverSystemMessage (int iEmpireKey, const Variant* pvData) {

    int iErrCode;
    unsigned int iKey, iNumMessages, iNumUnreadMessages;
    Variant vTemp;

    ICachedTable* pMessages = NULL;

    SYSTEM_EMPIRE_MESSAGES (strMessages, iEmpireKey);

    // Lock
    if (!t_pCache->DoesTableExist (strMessages)) {

        iErrCode = t_pCache->CreateTable (strMessages, SystemEmpireMessages::Template);
        if (iErrCode != OK && iErrCode != ERROR_TABLE_ALREADY_EXISTS) {
            Assert (false);
            goto Cleanup;
        }
    }

    iErrCode = t_pCache->ReadData(
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::MaxNumSystemMessages, 
        &vTemp
        );
    if (iErrCode != OK) {
        goto Cleanup;
    }

    const unsigned int iMaxNumMessages = vTemp.GetInteger();

    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Insert row
    iErrCode = pMessages->InsertRow (SystemEmpireMessages::Template, pvData, &iKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    //////////////////////////
    // Trim excess messages //
    //////////////////////////

    // Get num messages
    iErrCode = pMessages->GetNumRows (&iNumMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Get unread message count
    iErrCode = GetNumUnreadSystemMessagesPrivate (pMessages, &iNumUnreadMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    Assert (iNumMessages >= iNumUnreadMessages);

    iErrCode = DeleteOverflowMessages (
        pMessages, SystemEmpireMessages::TimeStamp, SystemEmpireMessages::Unread, iNumMessages, 
        iNumUnreadMessages, iMaxNumMessages, true);

Cleanup:

    SafeRelease (pMessages);

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumber -> Number of messages in message queue
//
// Returns the number of system messages the empire has in its queue

int GameEngine::GetNumSystemMessages (int iEmpireKey, unsigned int* piNumber) {

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    if (!t_pCache->DoesTableExist (pszMessages)) {
        *piNumber = 0;
        return OK;
    }

    return t_pCache->GetNumRows (pszMessages, piNumber);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumber -> Number of unread messages message in system queue
//
// Returns the number of unread system messages the empire has its queue

int GameEngine::GetNumUnreadSystemMessages (int iEmpireKey, unsigned int* piNumber) {

    int iErrCode;
    ICachedTable* pMessages = NULL;

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    iErrCode = t_pCache->GetTable(pszMessages, &pMessages);
    if (iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
        *piNumber = 0;
        return OK;
    }

    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = GetNumUnreadSystemMessagesPrivate (pMessages, piNumber);

    SafeRelease (pMessages);

    return iErrCode;
}

int GameEngine::GetNumUnreadSystemMessagesPrivate (ICachedTable* pMessages, unsigned int* piNumber) {

    int iErrCode;

    iErrCode = pMessages->GetEqualKeys (
        SystemEmpireMessages::Unread, 
        MESSAGE_UNREAD,
        NULL,
        piNumber
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Source of message
// pszMessage -> Message text
//
// Send a message to all empires on the server

int GameEngine::SendMessageToAll (int iEmpireKey, const char* pszMessage) {

    int iErrCode = OK;
    unsigned int iDestKey = NO_KEY;

    // Send messages
    while (true) {

        iErrCode = t_pCache->GetNextKey (SYSTEM_EMPIRE_DATA, iDestKey, &iDestKey);
        if (iErrCode != OK) {
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            } else Assert (false);
            break;
        }

        // Best effort
        SendSystemMessage (iDestKey, pszMessage, iEmpireKey, MESSAGE_ADMINISTRATOR);
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// ***pppvData -> Columns:
// SystemEmpireMessages::TimeStamp
// SystemEmpireMessages::Source
// SystemEmpireMessages::Broadcast
// SystemEmpireMessages::Text
//
// *ppiMessageKey -> Array of keys
// *piNumMessages -> Number of messages returned
//
// Return the saved messages in an empire's queue

const char* g_pszSystemMessageColumns[] = {
    SystemEmpireMessages::Unread,
    SystemEmpireMessages::Source,
    SystemEmpireMessages::TimeStamp,
    SystemEmpireMessages::Flags,
    SystemEmpireMessages::Text,
    SystemEmpireMessages::Type,
};

const unsigned int g_iNumSystemMessageColumns = countof (g_pszSystemMessageColumns);

int GameEngine::GetSavedSystemMessages (int iEmpireKey, unsigned int** ppiMessageKey, Variant*** pppvData, 
                                        unsigned int* piNumMessages) {

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    if (!t_pCache->DoesTableExist (pszMessages)) {
        *ppiMessageKey = NULL;
        *pppvData = NULL;
        *piNumMessages = 0;
        return OK;
    }

    int iErrCode = t_pCache->ReadColumns (
        pszMessages,
        g_iNumSystemMessageColumns,
        g_pszSystemMessageColumns,
        ppiMessageKey,
        pppvData,
        piNumMessages
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *pppvMessage -> Array of messages
// Unread
// Source
// TimeStamp
// Broadcast
// Text
//
// *piNumMessages -> Number of messages
//
// Return unread messages in the given empire's queue.  Mark it unread and 
// delete it if there should be zero messages on the queue

int GameEngine::GetUnreadSystemMessages (int iEmpireKey, Variant*** pppvMessage, unsigned int** ppiMessageKey,
                                         unsigned int* piNumMessages) {

    int iErrCode;
    
    SYSTEM_EMPIRE_MESSAGES(strMessages, iEmpireKey);
    GET_SYSTEM_EMPIRE_DATA(strSystemEmpireData, iEmpireKey);

    unsigned int i, * piMessageKey = NULL, * piKey = NULL, iNumMessages = 0, iTotalNumMessages;
    Variant vTemp, ** ppvMessage = NULL;

    UTCTime* ptTime = NULL, * ptTimeStamp = NULL;

    ICachedTable* pMessages = NULL;

    *pppvMessage = NULL;
    *ppiMessageKey = NULL;
    *piNumMessages = 0;

    iErrCode = t_pCache->ReadData(strSystemEmpireData, iEmpireKey, SystemEmpireData::MaxNumSystemMessages, &vTemp);
    if (iErrCode != OK)
    {
        Assert (false);
        goto Cleanup;
    }
    unsigned int iMaxNumMessages = vTemp.GetInteger();

    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    if (iErrCode != OK) {

        if (iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
            iErrCode = OK;
        }
        goto Cleanup;
    }

    // Get all unread messages
    iErrCode = pMessages->GetEqualKeys (
        SystemEmpireMessages::Unread,
        MESSAGE_UNREAD,
        &piMessageKey,
        &iNumMessages
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ppvMessage = new Variant* [iNumMessages];
    if (ppvMessage == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }
    memset (ppvMessage, 0, iNumMessages * sizeof (Variant*));

    ptTime = (UTCTime*) StackAlloc (iNumMessages * sizeof (UTCTime));

    for (i = 0; i < iNumMessages; i ++) {

        iErrCode = pMessages->ReadRow (piMessageKey[i], ppvMessage + i);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        ptTime[i] = ppvMessage[i][SystemEmpireMessages::iTimeStamp].GetInteger64();

        if (ppvMessage[i][SystemEmpireMessages::iType].GetInteger() == MESSAGE_NORMAL) {

            iErrCode = pMessages->WriteData (piMessageKey[i], SystemEmpireMessages::Unread, MESSAGE_READ);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    // Sort the read messages oldest to newest
    Algorithm::QSortThreeAscending <UTCTime, Variant*, unsigned int> (
        ptTime, ppvMessage, piMessageKey, iNumMessages
        );

    // All messages have now been read, so we need to make sure that if we have more saved messages 
    // than the max, we delete the oldest messages (best effort)
    iErrCode = pMessages->GetNumRows (&iTotalNumMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Delete stale messages
    iErrCode = DeleteOverflowMessages (
        pMessages, SystemEmpireMessages::TimeStamp, SystemEmpireMessages::Unread, iTotalNumMessages, 
        0, iMaxNumMessages, false);

    *piNumMessages = iNumMessages;

    *ppiMessageKey = piMessageKey;
    piMessageKey = NULL;

    *pppvMessage = ppvMessage;
    ppvMessage = NULL;

Cleanup:

    SafeRelease (pMessages);

    if (ppvMessage != NULL) {

        for (i = 0; i < iNumMessages; i ++) {

            if (ppvMessage[i] != NULL) {
                t_pCache->FreeData(ppvMessage[i]);
            }
        }

        delete [] ppvMessage;
    }

    if (piMessageKey != NULL) {
        t_pCache->FreeKeys(piMessageKey);
    }

    if (ptTimeStamp != NULL) {
        t_pCache->FreeData(ptTimeStamp);
    }

    if (piKey != NULL) {
        t_pCache->FreeKeys(piKey);
    }

    return iErrCode;
}

                                            
// Input:
// iEmpireKey -> Empire's integer key
// iKey -> Key of a system message
//
// Delete a given message from the message queue.  Message has always been read.

int GameEngine::DeleteSystemMessage (int iEmpireKey, unsigned int iKey) {

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    return t_pCache->DeleteRow(pszMessages, (unsigned int) iKey);
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumber -> Number of messages in message queue
//
// Returns the number of games messages the empire has its queue

int GameEngine::GetNumGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumber) {

    GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    return t_pCache->GetNumRows (pszMessages, piNumber);
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumber -> Number of unread messages message in queue
//
// Returns the number of unread game messages the empire has its queue

int GameEngine::GetNumUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, 
                                          unsigned int* piNumber) {

    int iErrCode;
    ICachedTable* pMessages = NULL;

    GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->GetTable(pszMessages, &pMessages);
    if (iErrCode == OK) {
        iErrCode = GetNumUnreadGameMessagesPrivate (pMessages, piNumber);
        SafeRelease (pMessages);
    }

    return iErrCode;
}

int GameEngine::GetNumUnreadGameMessagesPrivate (ICachedTable* pMessages, unsigned int* piNumber) {

    int iErrCode;

    iErrCode = pMessages->GetEqualKeys (
        GameEmpireMessages::Unread, 
        MESSAGE_UNREAD,
        NULL,
        piNumber
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of destination empire
// pszMessage -> Message to be sent to empire
// iSourceKey -> Integer key of empire who sent the message
// bBroadcast -> True if messages was broadcast
//
// Add a game message to a given empire's queue.

int GameEngine::SendGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, 
                                 int iSourceKey, int iFlags, const UTCTime& tSendTime) {

    int iErrCode;
    unsigned int iNumMessages, iNumUnreadMessages;

    bool bFlag;
    Variant vTemp;

    UTCTime tTime = tSendTime;
    
    // Make sure private messages are allowed
    if (!(iFlags & (MESSAGE_BROADCAST | MESSAGE_SYSTEM | MESSAGE_ADMINISTRATOR | MESSAGE_TOURNAMENT_ADMINISTRATOR))) {

        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return ERROR_EMPIRE_DOES_NOT_EXIST;
        }

        if (!(vTemp.GetInteger() & PRIVATE_MESSAGES)) {
            return ERROR_CANNOT_SEND_MESSAGE;
        }
    }

    // Make sure both empires are still in the game
    iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
    if (iErrCode != OK || !bFlag) {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }

    if (!(iFlags & (MESSAGE_SYSTEM | MESSAGE_ADMINISTRATOR | MESSAGE_TOURNAMENT_ADMINISTRATOR))) {

        iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iSourceKey, &bFlag);
        if (iErrCode != OK || !bFlag) {
            return ERROR_EMPIRE_IS_NOT_IN_GAME;
        }

        // Is empire ignoring other empire?
        iErrCode = GetEmpireIgnoreMessages (iGameClass, iGameNumber, iEmpireKey, iSourceKey, &bFlag);
        if (iErrCode != ERROR_EMPIRE_IS_NOT_IN_DIPLOMACY && (iErrCode != OK || bFlag)) {
            return ERROR_EMPIRE_IS_IGNORING_SENDER;
        }

        if (iFlags & MESSAGE_BROADCAST) {

            iErrCode = GetEmpireOption (iGameClass, iGameNumber, iEmpireKey, IGNORE_BROADCASTS, &bFlag);
            if (iErrCode != OK || bFlag) {
                return ERROR_EMPIRE_IS_IGNORING_BROADCASTS;
            }
        }
    }

    // Okay, go ahead and deliver the message
    GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::MaxNumGameMessages, &vTemp);
    if (iErrCode != OK) {
        return iErrCode;
    }
    const unsigned int iMaxNumMessages = vTemp.GetInteger();

    // Get time if necessary
    if (tTime == NULL_TIME) {
        Time::GetTime (&tTime);
    }

    // Insert the message into the table
    Variant pvData[GameEmpireMessages::NumColumns];

    pvData[GameEmpireMessages::iUnread] = MESSAGE_UNREAD;
    
    if (iSourceKey == SYSTEM) {
        
        iFlags |= MESSAGE_SYSTEM;
        pvData[GameEmpireMessages::iSource] = (const char*) NULL;
    
    } else {

        if (t_pCache->ReadData(
            SYSTEM_EMPIRE_DATA, 
            iSourceKey, 
            SystemEmpireData::Name, 
            pvData + GameEmpireMessages::iSource
            ) != OK) {
            
            pvData[GameEmpireMessages::iSource] = "Unknown";
        }

        if (pvData[GameEmpireMessages::iSource].GetCharPtr() == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
    }
    
    pvData[GameEmpireMessages::iTimeStamp] = tTime;
    pvData[GameEmpireMessages::iFlags] = iFlags;
    pvData[GameEmpireMessages::iText] = pszMessage;

    if (pvData[GameEmpireMessages::iText].GetCharPtr() == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    ICachedTable* pMessages = NULL;

    // Lock
    iErrCode = t_pCache->GetTable(strGameEmpireMessages, &pMessages);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pMessages->InsertRow(GameEmpireMessages::Template, pvData, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pMessages->GetNumRows (&iNumMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetNumUnreadGameMessagesPrivate (pMessages, &iNumUnreadMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = DeleteOverflowMessages (
        pMessages, GameEmpireMessages::TimeStamp, GameEmpireMessages::Unread, iNumMessages, 
        iNumUnreadMessages, iMaxNumMessages, true);

Cleanup:

    SafeRelease (pMessages);

    return iErrCode;
}

int GameEngine::DeleteOverflowMessages(ICachedTable* pMessages, const char* pszTimeStampColumn, 
                                       const char* pszUnreadColumn, unsigned int iNumMessages, 
                                       unsigned int iNumUnreadMessages, unsigned int iMaxNumMessages, 
                                       bool bCheckUnread)
{
    int iErrCode = OK;
    unsigned int* piKey = NULL;
    Variant* pvTime = NULL;

    // Check for message overflow, best effort
    if ((iNumMessages - iNumUnreadMessages) > iMaxNumMessages) {
        
        // Read timestamps from table
        unsigned int iNumRows;
        iErrCode = pMessages->ReadColumn (pszTimeStampColumn, &piKey, &pvTime, &iNumRows);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        Assert (iNumRows == iNumMessages);

        // Sort timestamps
        Algorithm::QSortTwoAscending<Variant, unsigned int>(pvTime, piKey, iNumRows);

        unsigned int i = 0;
        while (i < iNumRows && (iNumMessages - iNumUnreadMessages) > iMaxNumMessages) {

            // See if the message is unread
            if (bCheckUnread) {

                int iUnread;
                iErrCode = pMessages->ReadData (piKey[i], pszUnreadColumn, &iUnread);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (iUnread != MESSAGE_READ) {
                    // Nothing younger than this can have been read, so give up
                    break;
                }
            }

            // Delete the message
            iErrCode = pMessages->DeleteRow(piKey[i]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iNumMessages --;
            i ++;
        }
    }
    
Cleanup:

    if (pvTime != NULL) {
        t_pCache->FreeData(pvTime);
    }

    if (piKey != NULL) {
        t_pCache->FreeKeys(piKey);
    }

    return iErrCode;
}

// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of empire
//
// Output:
// ppvData -> Columns:
// GameEmpireMessages::Text,
// GameEmpireMessages::Source,
// GameEmpireMessages::TimeStamp,
// GameEmpireMessages::Flags
//
// ppiMessageKey -> Key array
//
// *piNumMessages -> Number of messages returned
//
// Return the saved game messages in an empire's queue

const char* g_pszGameMessageColumns[] = {
    GameEmpireMessages::Text,
    GameEmpireMessages::Source,
    GameEmpireMessages::TimeStamp,
    GameEmpireMessages::Flags
};

const unsigned int g_iNumGameMessageColumns = countof (g_pszGameMessageColumns);

int GameEngine::GetSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, 
                                      unsigned int** ppiMessageKey, Variant*** ppvData, 
                                      unsigned int* piNumMessages) {

    int iErrCode;

    GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadColumns (
        pszMessages, 
        g_iNumGameMessageColumns,
        g_pszGameMessageColumns,
        ppiMessageKey,
        ppvData,
        piNumMessages
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of empire
//
// Output:
// *pppvMessage -> Array of messages
// Unread
// Source
// TimeStamp
// Broadcast
// Text
//
// *piNumMessages -> Number of messages
//
// Return the first unread message in the given empire's queue.  Mark it unread and 
// delete it if there should be zero messages on the queue

int GameEngine::GetUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, Variant*** pppvMessage, 
                                       unsigned int* piNumMessages) {

    GAME_EMPIRE_MESSAGES (strMessages, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode;
    unsigned int* piKey = NULL, i, iNumMessages = 0, iMaxNumMessages, iTotalNumMessages;

    Variant vTemp, ** ppvMessage = NULL;
    UTCTime* ptTime = NULL, * ptTimeStamp = NULL;

    ICachedTable* pMessages = NULL;

    *pppvMessage = NULL;
    *piNumMessages = 0;

    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::MaxNumGameMessages, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iMaxNumMessages = vTemp.GetInteger();

    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pMessages->GetEqualKeys (
        GameEmpireMessages::Unread,
        MESSAGE_UNREAD,
        &piKey,
        &iNumMessages
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ppvMessage = new Variant* [iNumMessages];
    if (ppvMessage == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    ptTime = (UTCTime*) StackAlloc (iNumMessages * sizeof (UTCTime));

    for (i = 0; i < iNumMessages; i ++) {

        iErrCode = pMessages->ReadRow (piKey[i], ppvMessage + i);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        ptTime[i] = ppvMessage[i][GameEmpireMessages::iTimeStamp].GetInteger64();

        iErrCode = pMessages->WriteData (piKey[i], GameEmpireMessages::Unread, MESSAGE_READ);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Sort the read messages oldest to newest
    Algorithm::QSortTwoAscending <UTCTime, Variant*> (ptTime, ppvMessage, iNumMessages);

    // All messages have been read, so we need to make sure that if we have more saved messages 
    // than the max, we delete the oldest messages (best effort)
    iErrCode = pMessages->GetNumRows (&iTotalNumMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    Assert (iTotalNumMessages >= iNumMessages);

    // Delete stale messages
    iErrCode = DeleteOverflowMessages (
        pMessages, GameEmpireMessages::TimeStamp, GameEmpireMessages::Unread, iTotalNumMessages, 
        0, iMaxNumMessages, false);

    *piNumMessages = iNumMessages;

    *pppvMessage = ppvMessage;
    ppvMessage = NULL;

Cleanup:

    SafeRelease (pMessages);

    if (ppvMessage != NULL) {

        for (i = 0; i < iNumMessages; i ++) {

            if (ppvMessage[i] != NULL) {
                t_pCache->FreeData(ppvMessage[i]);
            }
        }

        delete [] ppvMessage;
    }

    if (piKey != NULL) {
        t_pCache->FreeKeys(piKey);
    }

    if (ptTimeStamp != NULL) {
        t_pCache->FreeData(ptTimeStamp);
    }

    return iErrCode;
}

        
// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Empire's integer key
// iKey -> Key of a game message
//
// Delete a given message from the queue

int GameEngine::DeleteGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iKey) {

    GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    return t_pCache->DeleteRow(pszMessages, iKey);
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iSourceKey -> Integer key of empire who broadcast the message
// pszMessage -> Message to be broadcast
// bAdmin -> True if the message is a broadcast from an administrator who might not be in the game
//
// Broadcast a game message to everyone in the game

int GameEngine::BroadcastGameMessage (int iGameClass, int iGameNumber, const char* pszMessage, int iSourceKey,
                                      int iFlags) {

    int iErrCode;

    Assert (iFlags & MESSAGE_BROADCAST);

#ifdef _DEBUG

    if (iFlags & MESSAGE_SYSTEM) {
        Assert (!(iFlags & (MESSAGE_ADMINISTRATOR | MESSAGE_TOURNAMENT_ADMINISTRATOR)));
    }

    if (iFlags & MESSAGE_ADMINISTRATOR) {
        Assert (!(iFlags & MESSAGE_TOURNAMENT_ADMINISTRATOR));
    }

    if (iSourceKey == SYSTEM) {
        Assert (iFlags & MESSAGE_SYSTEM);
    } else {
        Assert (!(iFlags & MESSAGE_SYSTEM));
    }

#endif

    if (!(iFlags & (MESSAGE_SYSTEM | MESSAGE_ADMINISTRATOR | MESSAGE_TOURNAMENT_ADMINISTRATOR))) {

        bool bFlag;
        
        // Make sure source can broadcast && send system messages
        iErrCode = GetEmpireOption (iSourceKey, CAN_BROADCAST, &bFlag);
        if (iErrCode != OK || !bFlag) {
            return ERROR_CANNOT_SEND_MESSAGE;
        }
        
        // Make sure empire is still game
        iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iSourceKey, &bFlag);
        if (iErrCode != OK || !bFlag) {
            return ERROR_EMPIRE_IS_NOT_IN_GAME;
        }
    }
    
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
   
    unsigned int iKey = NO_KEY, iSent = 0;
    while (true) {

        Variant vEmpireKey;

        iErrCode = t_pCache->GetNextKey (strGameEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }

        if (iErrCode != OK) {
            Assert (false);
            break;
        }

        iErrCode = t_pCache->ReadData(strGameEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
        if (iErrCode == OK) {

            iErrCode = SendGameMessage (
                iGameClass, iGameNumber, vEmpireKey.GetInteger(), pszMessage, iSourceKey, iFlags, NULL_TIME
                );

            if (iErrCode == OK) {
                iSent ++;
            }
        }
    }

    return iSent > 0 ? OK : ERROR_FAILURE;
}

int GameEngine::GetSystemMessageProperty (int iEmpireKey, unsigned int iMessageKey, const char* pszColumn, Variant* pvProperty) {

    SYSTEM_EMPIRE_MESSAGES (strMessages, iEmpireKey);
    return GetMessageProperty (strMessages, iMessageKey, pszColumn, pvProperty);
}


int GameEngine::GetGameMessageProperty (int iGameClass, int iGameNumber, int iEmpireKey, 
                                        unsigned int iMessageKey, const char* pszColumn, Variant* pvProperty) {

    GAME_EMPIRE_MESSAGES (strMessages, iGameClass, iGameNumber, iEmpireKey);
    return GetMessageProperty (strMessages, iMessageKey, pszColumn, pvProperty);
}

int GameEngine::GetMessageProperty (const char* strMessages, unsigned int iMessageKey, const char* pszColumn, 
                                    Variant* pvProperty) {

    int iErrCode;
    bool bExists;

    ICachedTable* pMessages = NULL;

    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pMessages->DoesRowExist(iMessageKey, &bExists);
    if (iErrCode == OK) {

        if (!bExists) {
            iErrCode = ERROR_MESSAGE_DOES_NOT_EXIST;
        } else {
            iErrCode = pMessages->ReadData (iMessageKey, pszColumn, pvProperty);
        }
    }

    SafeRelease (pMessages);

    return iErrCode;
}

int GameEngine::SendFatalUpdateMessage (int iGameClass, int iGameNumber, int iEmpireKey, 
                                        const char* pszGameClassName, const String& strUpdateMessage) {

    int iErrCode;
    bool bAlive;

    iErrCode = DoesEmpireExist (iEmpireKey, &bAlive, NULL);
    if (iErrCode != OK || !bAlive) {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    char* pszFullMessage = new char [strUpdateMessage.GetLength() + MAX_FULL_GAME_CLASS_NAME_LENGTH + 256];
    if (pszFullMessage == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    sprintf (
        pszFullMessage,
        "You were obliterated from %s %i. "\
        "This is the update message from your last update in the game:" NEW_PARAGRAPH "%s",
        pszGameClassName,
        iGameNumber,
        strUpdateMessage.GetCharPtr()
        );

    iErrCode = SendSystemMessage (iEmpireKey, pszFullMessage, SYSTEM, MESSAGE_SYSTEM);

    delete [] pszFullMessage;
    return iErrCode;
}