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

#include "GameEngine.h"

// Input:
// iEmpireKey -> Integer key of destination empire
// pszMessage -> Message to be sent to empire
// iSource -> Sender of message
// bBroadcast -> true if message was broadcasted, false if sent
//
// Add a System Message to a given empire's queue.

int GameEngine::SendSystemMessage (int iEmpireKey, const char* pszMessage, int iSource, bool bBroadcast) {

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
    iErrCode = DoesEmpireExist (iEmpireKey, &bFlag);
    if (iErrCode != OK || !bFlag) {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    // Insert the message into the table
    pvData[SystemEmpireMessages::Unread] = MESSAGE_UNREAD;
    
    int iFlags = 0;
    
    if (bBroadcast) {
        iFlags |= MESSAGE_BROADCAST;
    }

    if (iSource == SYSTEM) {
        iFlags |= MESSAGE_SYSTEM;
        pvData[SystemEmpireMessages::Source] = (const char*) NULL;
    } else {
        
        if (m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iSource, 
            SystemEmpireData::Name, 
            pvData + SystemEmpireMessages::Source
            ) != OK) {          
            pvData[SystemEmpireMessages::Source] = "Unknown";
        }

        if (pvData[SystemEmpireMessages::Source].GetCharPtr() == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
    }

    pvData[SystemEmpireMessages::TimeStamp] = tTime;
    pvData[SystemEmpireMessages::Flags] = iFlags;
    pvData[SystemEmpireMessages::Text] = pszMessage;
    pvData[SystemEmpireMessages::Type] = MESSAGE_NORMAL;
    pvData[SystemEmpireMessages::Data] = (const char*) NULL;

    if (pvData[SystemEmpireMessages::Text].GetCharPtr() == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    return DeliverSystemMessage (iEmpireKey, pvData);
}


int GameEngine::DeliverSystemMessage (int iEmpireKey, const Variant* pvData) {

    unsigned int i, iKey, iNumRows;
    int iNumMessages, iNumUnreadMessages, iErrCode;

    Variant vMaxNum;

    SYSTEM_EMPIRE_MESSAGES (strMessages, iEmpireKey);

    // Lock
    NamedMutex nmMutex;
    LockEmpireSystemMessages (iEmpireKey, &nmMutex);

    if (!m_pGameData->DoesTableExist (strMessages)) {

        iErrCode = m_pGameData->CreateTable (strMessages, SystemEmpireMessages::Template.Name);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Insert row
    iErrCode = m_pGameData->InsertRow (strMessages, pvData, &iKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    //////////////////////////
    // Trim excess messages //
    //////////////////////////

    iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::MaxNumSystemMessages, 
        &vMaxNum
        );
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Get num messages
    iErrCode = m_pGameData->GetNumRows (strMessages, (unsigned int*) &iNumMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Get unread message count
    iErrCode = GetNumUnreadSystemMessages (iEmpireKey, &iNumUnreadMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Check for message overflow
    if ((iNumMessages - iNumUnreadMessages) > vMaxNum.GetInteger()) {
        
        // Read timestamps from table
        unsigned int* piKey;
        Variant* pvTime, vUnread;

        int iMin, iLowestKey;

        iErrCode = m_pGameData->ReadColumn (
            strMessages, 
            SystemEmpireMessages::TimeStamp, 
            &piKey, 
            &pvTime, 
            &iNumRows
            );
        if (iErrCode != OK) {
            goto Cleanup;
        }

        // Find oldest message
        iMin = pvTime[0];
        iLowestKey = piKey[0];

        for (i = 1; i < iNumRows; i ++) {
            if (pvTime[i] < iMin) {
                iMin = pvTime[i];
                iLowestKey = piKey[i];
            }
        }

        // Delete the oldest message if it's unread
        iErrCode = m_pGameData->ReadData (strMessages, iLowestKey, SystemEmpireMessages::Unread, &vUnread);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        if (vUnread.GetInteger() == MESSAGE_READ) {
            iErrCode = m_pGameData->DeleteRow (strMessages, iLowestKey);
            if (iErrCode != OK) {
                goto Cleanup;
            }
        }

        FreeData (pvTime);
        FreeKeys (piKey);
    }

Cleanup:

    UnlockEmpireSystemMessages (nmMutex);

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumber -> Number of messages in message queue
//
// Returns the number of system messages the empire has in its queue

int GameEngine::GetNumSystemMessages (int iEmpireKey, int* piNumber) {

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    if (!m_pGameData->DoesTableExist (pszMessages)) {
        *piNumber = 0;
        return OK;
    }

    return m_pGameData->GetNumRows (pszMessages, (unsigned int*) piNumber);
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumber -> Number of unread messages message in system queue
//
// Returns the number of unread system messages the empire has its queue

int GameEngine::GetNumUnreadSystemMessages (int iEmpireKey, int* piNumber) {

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    if (!m_pGameData->DoesTableExist (pszMessages)) {
        *piNumber = 0;
        return OK;
    }

    int iErrCode = m_pGameData->GetEqualKeys (
        pszMessages, 
        SystemEmpireMessages::Unread, 
        MESSAGE_UNREAD,
        false,
        NULL,
        (unsigned int*) piNumber
        );

    return iErrCode == ERROR_DATA_NOT_FOUND ? OK : iErrCode;
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

        iErrCode = m_pGameData->GetNextKey (SYSTEM_EMPIRE_DATA, iDestKey, &iDestKey);
        if (iErrCode != OK) {
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            } else Assert (false);
            break;
        }

        // Best effort
        SendSystemMessage (iDestKey, pszMessage, iEmpireKey, true);
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

int GameEngine::GetSavedSystemMessages (int iEmpireKey, int** ppiMessageKey, Variant*** pppvData, 
                                        int* piNumMessages) {

    const unsigned int piColumns[] = {
        SystemEmpireMessages::Unread,
        SystemEmpireMessages::Source,
        SystemEmpireMessages::TimeStamp,
        SystemEmpireMessages::Flags,
        SystemEmpireMessages::Text,
        SystemEmpireMessages::Type,
    };

    const unsigned int iNumColumns = sizeof (piColumns) / sizeof (piColumns[0]);

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    if (!m_pGameData->DoesTableExist (pszMessages)) {
        *ppiMessageKey = NULL;
        *pppvData = NULL;
        *piNumMessages = 0;
        return OK;
    }

    int iErrCode = m_pGameData->ReadColumns (
        pszMessages,
        iNumColumns,
        piColumns,
        (unsigned int**) ppiMessageKey,
        pppvData,
        (unsigned int*) piNumMessages
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
// Return the first unread message in the given empire's queue.  Mark it unread and 
// delete it if there should be zero messages on the queue

int GameEngine::GetUnreadSystemMessages (int iEmpireKey, Variant*** pppvMessage, unsigned int** ppiMessageKey,
                                         int* piNumMessages) {

    SYSTEM_EMPIRE_MESSAGES (strMessages, iEmpireKey);

    *pppvMessage = NULL;
    *ppiMessageKey = NULL;
    *piNumMessages = 0;

    int iErrCode, iErrCode2;
    unsigned int i = 0, iNumMessages;
    Variant vMaxNumMessages, * pvTimeStamp = NULL, ** ppvMessage = NULL;
    UTCTime* ptTime;

    if (!m_pGameData->DoesTableExist (strMessages)) {
        return OK;
    }

    NamedMutex nmMutex;
    LockEmpireSystemMessages (iEmpireKey, &nmMutex);

    iErrCode = m_pGameData->GetEqualKeys (
        strMessages,
        SystemEmpireMessages::Unread,
        MESSAGE_UNREAD,
        false,
        ppiMessageKey,
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

    *piNumMessages = (int) iNumMessages;
    *pppvMessage = ppvMessage;

    ptTime = (UTCTime*) StackAlloc (iNumMessages * sizeof (UTCTime));

    for (i = 0; i < iNumMessages; i ++) {

        iErrCode = m_pGameData->ReadRow (
            strMessages,
            (*ppiMessageKey)[i],
            &ppvMessage[i]
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        ptTime[i] = ppvMessage[i][SystemEmpireMessages::TimeStamp].GetUTCTime();

        if (ppvMessage[i][SystemEmpireMessages::Type].GetInteger() == MESSAGE_NORMAL) {

            iErrCode = m_pGameData->WriteData (
                strMessages,
                (*ppiMessageKey)[i],
                SystemEmpireMessages::Unread,
                MESSAGE_READ
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    // Sort the read messages oldest to newest
    Algorithm::QSortThreeAscending <UTCTime, Variant*, unsigned int> (
        ptTime, *pppvMessage, *ppiMessageKey, *piNumMessages
        );

    // All messages have been read, so we need to make sure that if we have more saved messages 
    // than the max, we delete the oldest messages (best effort)
    iErrCode2 = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::MaxNumSystemMessages, 
        &vMaxNumMessages
        );
    if (iErrCode2 != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode2 = m_pGameData->GetNumRows (strMessages, &iNumMessages);
    if (iErrCode2 != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Delete stale messages
    if ((int) iNumMessages > vMaxNumMessages.GetInteger()) {

        unsigned int* piKey = NULL;
        int iCurrentNumMessages = (int) iNumMessages;
        int iMaxNumMessages = vMaxNumMessages.GetInteger();

        iErrCode = m_pGameData->ReadColumn (
            strMessages,
            SystemEmpireMessages::TimeStamp,
            &piKey,
            &pvTimeStamp,
            &iNumMessages
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        Assert (iNumMessages > 0);

        Algorithm::QSortTwoAscending<Variant, unsigned int> (pvTimeStamp, piKey, iNumMessages);

        for (i = 0; i < iNumMessages && iCurrentNumMessages > iMaxNumMessages; i ++) {

            // Delete the oldest key
            iErrCode = m_pGameData->DeleteRow (strMessages, piKey[i]);
            if (iErrCode != OK) {
                m_pGameData->FreeKeys (piKey);
                Assert (false);
                goto Cleanup;
            }

            iCurrentNumMessages --;
        }

        m_pGameData->FreeKeys (piKey);
    }

Cleanup:

    UnlockEmpireSystemMessages (nmMutex);

    if (iErrCode != OK) {

        unsigned int j;

        for (j = 0; j < i; j ++) {
            m_pGameData->FreeData (ppvMessage[j]);
        }

        if (ppvMessage != NULL) {
            delete [] ppvMessage;
        }

        if (*ppiMessageKey != NULL) {
            m_pGameData->FreeKeys (*ppiMessageKey);
            *ppiMessageKey = NULL;
        }
    }

    if (pvTimeStamp != NULL) {
        m_pGameData->FreeData (pvTimeStamp);
    }

    return iErrCode;
}

                                            
// Input:
// iEmpireKey -> Empire's integer key
// iKey -> Key of a system message
//
// Delete a given message from the message queue.  Message has always been read.

int GameEngine::DeleteSystemMessage (int iEmpireKey, int iKey) {

    SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);

    NamedMutex nmMutex;
    LockEmpireSystemMessages (iEmpireKey, &nmMutex);

    int iErrCode = m_pGameData->DeleteRow (pszMessages, (unsigned int) iKey);

    UnlockEmpireSystemMessages (nmMutex);

    return iErrCode;
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

int GameEngine::GetNumGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumber) {

    GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->GetNumRows (
        pszMessages, 
        (unsigned int*) piNumber
        );
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

int GameEngine::GetNumUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumber) {

    GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetEqualKeys (
        pszMessages, 
        GameEmpireMessages::Unread, 
        MESSAGE_UNREAD,
        false,
        NULL,
        (unsigned int*) piNumber
        );

    return iErrCode == ERROR_DATA_NOT_FOUND ? OK : iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of destination empire
// pszMessage -> Message to be sent to empire
// iSourceKey -> Integer key of empire who sent the message
// bBroadcast -> True if messages was broadcasted
//
// Add a game message to a given empire's queue.

int GameEngine::SendGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, 
                                 int iSourceKey, bool bBroadcast, bool bUpdateMessage, const UTCTime& tSendTime) {
    
    unsigned int i;
    int iErrCode, iNumMessages, iNumUnreadMessages;

    UTCTime tTime = tSendTime;
    
    // Make sure private messages are allowed
    if (iSourceKey != SYSTEM && !bBroadcast) {

        Variant vOptions;
        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::Options,
            &vOptions
            );

        if (iErrCode != OK) {
            Assert (false);
            return ERROR_EMPIRE_DOES_NOT_EXIST;
        }

        if (!(vOptions.GetInteger() & PRIVATE_MESSAGES)) {
            return ERROR_CANNOT_SEND_MESSAGE;
        }
    }

    // Make sure both empires are still in the game
    bool bFlag;
    
    iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
    if (iErrCode != OK || !bFlag) {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }

    if (iSourceKey != SYSTEM) {

        iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iSourceKey, &bFlag);
        if (iErrCode != OK || !bFlag) {
            return ERROR_EMPIRE_IS_NOT_IN_GAME;
        }
    }

    // Is empire ignoring other empire?
    if (iSourceKey != SYSTEM) {

        iErrCode = GetEmpireIgnoreMessages (iGameClass, iGameNumber, iEmpireKey, iSourceKey, &bFlag);
        if (iErrCode != ERROR_EMPIRE_IS_NOT_IN_DIPLOMACY && (iErrCode != OK || bFlag)) {
            return ERROR_EMPIRE_IS_IGNORING_SENDER;
        }

        if (bBroadcast) {

            iErrCode = GetEmpireOption (iGameClass, iGameNumber, iEmpireKey, IGNORE_BROADCASTS, &bFlag);
            if (iErrCode != OK || bFlag) {
                return ERROR_EMPIRE_IS_IGNORING_BROADCASTS;
            }
        }
    }

    // Okay
    GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
    
    Variant vMaxNum;

    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::MaxNumGameMessages, &vMaxNum);
    if (iErrCode != OK) {
        return iErrCode;
    }

    int iFlags = 0;

    // Get time if necessary
    if (tTime == NULL_TIME) {
        Time::GetTime (&tTime);
    }

    // Insert the message into the table
    Variant pvData [GameEmpireMessages::NumColumns];
    
    pvData[GameEmpireMessages::Unread] = MESSAGE_UNREAD;
    
    if (iSourceKey == SYSTEM) {
        
        iFlags |= MESSAGE_SYSTEM;
        pvData[GameEmpireMessages::Source] = (const char*) NULL;
    
    } else {
        
        if (m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iSourceKey, 
            SystemEmpireData::Name, 
            pvData + GameEmpireMessages::Source
            ) != OK) {
            
            pvData[GameEmpireMessages::Source] = "Unknown";
        }

        if (pvData[GameEmpireMessages::Source].GetCharPtr() == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
    }
    
    pvData[GameEmpireMessages::TimeStamp] = tTime;
    
    if (bBroadcast) {
        iFlags |= MESSAGE_BROADCAST;
    }

    if (bUpdateMessage) {
        iFlags |= MESSAGE_UPDATE;
    }
    
    pvData[GameEmpireMessages::Flags] = iFlags;
    pvData[GameEmpireMessages::Text] = pszMessage;

    if (pvData[GameEmpireMessages::Text].GetCharPtr() == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    unsigned int* piKey = NULL;
    Variant* pvTime = NULL;

    // Lock
    NamedMutex nmMutex;
    LockEmpireGameMessages (iGameClass, iGameNumber, iEmpireKey, &nmMutex);

    iErrCode = m_pGameData->InsertRow (strGameEmpireMessages, pvData);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Erase first message if > MaxNumGameMessages on stack         
    iErrCode = m_pGameData->GetNumRows (strGameEmpireMessages, (unsigned int*) &iNumMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetNumUnreadGameMessages (iGameClass, iGameNumber, iEmpireKey, &iNumUnreadMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Check for message overflow, best effort
    if ((iNumMessages - iNumUnreadMessages) > vMaxNum.GetInteger()) {
        
        // Read timestamps from table
        unsigned int iNumRows, iKey;
        Variant vUnread;

        UTCTime tMinTime;

        int iErrCode2 = m_pGameData->ReadColumn (
            strGameEmpireMessages, 
            GameEmpireMessages::TimeStamp, 
            &piKey,
            &pvTime, 
            &iNumRows
            );

        if (iErrCode2 != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Find oldest message
        tMinTime = pvTime[0].GetUTCTime();
        iKey = piKey[0];
        
        for (i = 1; i < iNumRows; i ++) {
            if (pvTime[i].GetUTCTime() < tMinTime) {
                tMinTime = pvTime[i].GetUTCTime();
                iKey = piKey[i];
            }
        }
        
        // Delete the oldest message if it's unread.
        // Otherwise they all must be unread so we should leave things alone.
        iErrCode2 = m_pGameData->ReadData (strGameEmpireMessages, iKey, GameEmpireMessages::Unread, &vUnread);
        if (iErrCode2 != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (vUnread.GetInteger() == MESSAGE_READ) {
            iErrCode2 = m_pGameData->DeleteRow (strGameEmpireMessages, iKey);
            if (iErrCode2 != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }
    
Cleanup:

    UnlockEmpireGameMessages (nmMutex);

    if (pvTime != NULL) {
        m_pGameData->FreeData (pvTime);
    }

    if (piKey != NULL) {
        m_pGameData->FreeKeys (piKey);
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

int GameEngine::GetSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiMessageKey,
                                      Variant*** ppvData, int* piNumMessages) {

    int iErrCode;

    unsigned int piColumns[] = {
        GameEmpireMessages::Text,
        GameEmpireMessages::Source,
        GameEmpireMessages::TimeStamp,
        GameEmpireMessages::Flags
    };

    GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = m_pGameData->ReadColumns (
        pszMessages, 
        4,
        piColumns,
        (unsigned int**) ppiMessageKey,
        ppvData,
        (unsigned int*) piNumMessages
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
                                       int* piNumMessages) {

    GAME_EMPIRE_MESSAGES (strMessages, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    *pppvMessage = NULL;
    *piNumMessages = 0;

    int iErrCode, iErrCode2;
    unsigned int* piKey = NULL, i = 0, iNumMessages;
    Variant vMaxNumMessages, * pvTimeStamp = NULL, ** ppvMessage = NULL;
    UTCTime* ptTime;

    NamedMutex nmMutex;
    LockEmpireGameMessages (iGameClass, iGameNumber, iEmpireKey, &nmMutex);

    iErrCode = m_pGameData->GetEqualKeys (
        strMessages,
        GameEmpireMessages::Unread,
        MESSAGE_UNREAD,
        false,
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

    *piNumMessages = (int) iNumMessages;
    *pppvMessage = ppvMessage;

    for (i = 0; i < iNumMessages; i ++) {

        iErrCode = m_pGameData->ReadRow (
            strMessages,
            piKey[i],
            &ppvMessage[i]
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        ptTime[i] = ppvMessage[i][GameEmpireMessages::TimeStamp].GetUTCTime();

        iErrCode = m_pGameData->WriteData (
            strMessages,
            piKey[i],
            GameEmpireMessages::Unread,
            MESSAGE_READ
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Sort the read messages oldest to newest
    Algorithm::QSortTwoAscending <UTCTime, Variant*> (ptTime, *pppvMessage, *piNumMessages);

    // All messages have been read, so we need to make sure that if we have more saved messages 
    // than the max, we delete the oldest messages (best effort)
    iErrCode2 = m_pGameData->ReadData (
        strGameEmpireData,
        GameEmpireData::MaxNumGameMessages,
        &vMaxNumMessages
        );
    if (iErrCode2 != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode2 = m_pGameData->GetNumRows (strMessages, &iNumMessages);
    if (iErrCode2 != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Cleanup
    m_pGameData->FreeKeys (piKey);
    piKey = NULL;

    // Delete stale messages
    if ((int) iNumMessages > vMaxNumMessages.GetInteger()) {

        int iCurrentNumMessages = (int) iNumMessages;
        int iMaxNumMessages = vMaxNumMessages.GetInteger();

        iErrCode = m_pGameData->ReadColumn (
            strMessages,
            GameEmpireMessages::TimeStamp,
            &piKey,
            &pvTimeStamp,
            &iNumMessages
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        Algorithm::QSortTwoAscending<Variant, unsigned int> (pvTimeStamp, piKey, iNumMessages);

        for (i = 0; i < iNumMessages && iCurrentNumMessages > iMaxNumMessages; i ++) {

            // Delete the oldest key
            iErrCode = m_pGameData->DeleteRow (strMessages, piKey[i]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iCurrentNumMessages --;
        }
    }

Cleanup:

    UnlockEmpireGameMessages (nmMutex);

    if (iErrCode != OK) {

        unsigned int j;

        for (j = 0; j < i; j ++) {
            m_pGameData->FreeData (ppvMessage[j]);
        }

        if (ppvMessage != NULL) {
            delete [] ppvMessage;
        }
    }

    if (piKey != NULL) {
        m_pGameData->FreeKeys (piKey);
    }

    if (pvTimeStamp != NULL) {
        m_pGameData->FreeData (pvTimeStamp);
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

int GameEngine::DeleteGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, int iKey) {

    GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    NamedMutex nmMutex;
    LockEmpireGameMessages (iGameClass, iGameNumber, iEmpireKey, &nmMutex);

    int iErrCode = m_pGameData->DeleteRow (pszMessages, iKey);

    UnlockEmpireGameMessages (nmMutex);

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iSourceKey -> Integer key of empire who broadcasted the message
// pszMessage -> Message to be broadcasted
// bAdmin -> True if the message is a broadcast from an administrator who might not be in the game
//
// Broadcast a game message to everyone the game

int GameEngine::BroadcastGameMessage (int iGameClass, int iGameNumber, const char* pszMessage, int iSourceKey,
                                      bool bAdmin) {

    unsigned int i;

    int iErrCode, iErrCode2;
    
    if (iSourceKey != SYSTEM && !bAdmin) {
        
        // Make sure source can broadcast && send system messages
        bool bBroadcast;
        iErrCode = GetEmpireOption (iSourceKey, CAN_BROADCAST, &bBroadcast);
        if (!bBroadcast) {
            return ERROR_CANNOT_SEND_MESSAGE;
        }
        
        // Make sure empire is still game
        bool bInGame = false;
        iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iSourceKey, &bInGame);
        if (iErrCode != OK || !bInGame) {
            return ERROR_EMPIRE_IS_NOT_IN_GAME;
        }
    }
    
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    
    unsigned int iNumEmpires;
    Variant* pvKey, vIgnore;
    iErrCode = m_pGameData->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, &pvKey, &iNumEmpires);
    
    if (iErrCode == OK && iNumEmpires > 0) {

        for (i = 0; i < iNumEmpires; i ++) {
            
            iErrCode2 = SendGameMessage (
                iGameClass, iGameNumber, pvKey[i].GetInteger(), pszMessage, iSourceKey, true, false, NULL_TIME
                );
            // Ignore error message here
        }
    
        m_pGameData->FreeData (pvKey);
    }

    else Assert (false);

    return iErrCode;
}

int GameEngine::GetSystemMessageProperty (int iEmpireKey, int iMessageKey, unsigned int iColumn, 
                                          Variant* pvProperty) {

    SYSTEM_EMPIRE_MESSAGES (strMessages, iEmpireKey);

    bool bExists;
    int iErrCode = m_pGameData->DoesRowExist (strMessages, iMessageKey, &bExists);

    if (iErrCode == OK && bExists) {
        return m_pGameData->ReadData (strMessages, iMessageKey, iColumn, pvProperty);
    }

    return ERROR_MESSAGE_DOES_NOT_EXIST;
}

int GameEngine::GetGameMessageProperty (int iGameClass, int iGameNumber, int iEmpireKey, int iMessageKey, 
                                        unsigned int iColumn, Variant* pvProperty) {

    GAME_EMPIRE_MESSAGES (strMessages, iGameClass, iGameNumber, iEmpireKey);

    bool bExists;
    int iErrCode = m_pGameData->DoesRowExist (strMessages, iMessageKey, &bExists);

    if (iErrCode == OK && bExists) {
        return m_pGameData->ReadData (strMessages, iMessageKey, iColumn, pvProperty);
    }

    return ERROR_MESSAGE_DOES_NOT_EXIST;
}

int GameEngine::SendFatalUpdateMessage (int iGameClass, int iGameNumber, int iEmpireKey, 
                                        const char* pszGameClassName, const String& strUpdateMessage) {

    bool bAlive;

    int iErrCode = DoesEmpireExist (iEmpireKey, &bAlive);
    if (iErrCode != OK || !bAlive) {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    char* pszFullMessage = (char*) StackAlloc (
        strUpdateMessage.GetLength() + 
        MAX_FULL_GAME_CLASS_NAME_LENGTH +
        256
        );

    sprintf (
        pszFullMessage,
        "You were obliterated from %s %i. "\
        "This is the update message from your last update in the game:" NEW_PARAGRAPH "%s",
        pszGameClassName,
        iGameNumber,
        strUpdateMessage.GetCharPtr()
        );

    return SendSystemMessage (iEmpireKey, pszFullMessage, SYSTEM);
}