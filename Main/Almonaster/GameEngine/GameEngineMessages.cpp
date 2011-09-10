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

int GameEngine::SendSystemMessage(int iTargetEmpireKey, const char* pszMessage, int iSourceEmpireKey, int iFlags)
{
    int iErrCode;

    // Make sure source can broadcast && send system messages
    if (iSourceEmpireKey != SYSTEM)
    {
        bool bBroadcast;
        iErrCode = GetEmpireOption(iSourceEmpireKey, CAN_BROADCAST, &bBroadcast);
        RETURN_ON_ERROR(iErrCode);
        if (!bBroadcast)
        {
            return ERROR_CANNOT_SEND_MESSAGE;
        }
    }

    // Make sure target empire exists
    unsigned int iNumResults;
    iErrCode = CacheEmpire(iTargetEmpireKey, &iNumResults);
    RETURN_ON_ERROR(iErrCode);
    if (iNumResults == 0)
    {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    Variant pvData [SystemEmpireMessages::NumColumns];

    UTCTime tTime;
    Time::GetTime (&tTime);
    
    // Insert the message into the table
    pvData[SystemEmpireMessages::iEmpireKey] = iTargetEmpireKey;
    pvData[SystemEmpireMessages::iUnread] = MESSAGE_UNREAD;
    pvData[SystemEmpireMessages::iSourceKey] = iSourceEmpireKey;

    if (iSourceEmpireKey == SYSTEM)
    {
        Assert (iFlags & MESSAGE_SYSTEM);
        pvData[SystemEmpireMessages::iSourceName] = (const char*)NULL;
        pvData[SystemEmpireMessages::iSourceSecret] = 0;
    }
    else
    {
        GET_SYSTEM_EMPIRE_DATA(strEmpire, iSourceEmpireKey);
        
        iErrCode = t_pCache->ReadData(strEmpire, iSourceEmpireKey, SystemEmpireData::Name, pvData + SystemEmpireMessages::iSourceName);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strEmpire, iSourceEmpireKey, SystemEmpireData::SecretKey, pvData + SystemEmpireMessages::iSourceSecret);
        RETURN_ON_ERROR(iErrCode);
    }

    pvData[SystemEmpireMessages::iTimeStamp] = tTime;
    pvData[SystemEmpireMessages::iFlags] = iFlags;
    pvData[SystemEmpireMessages::iText] = pszMessage;
    pvData[SystemEmpireMessages::iType] = MESSAGE_NORMAL;
    pvData[SystemEmpireMessages::iData] = (const char*)NULL;
    Assert(pvData[SystemEmpireMessages::iText].GetCharPtr());

    return DeliverSystemMessage(iTargetEmpireKey, pvData);
}

int GameEngine::DeliverSystemMessage(int iEmpireKey, const Variant* pvData)
{
    int iErrCode;
    unsigned int iKey, iNumMessages, iNumUnreadMessages;
    Variant vTemp;

    ICachedTable* pMessages = NULL;
    AutoRelease<ICachedTable> rel(pMessages);

    iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::MaxNumSystemMessages, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    const unsigned int iMaxNumMessages = vTemp.GetInteger();

    GET_SYSTEM_EMPIRE_MESSAGES(strMessages, iEmpireKey);
    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    RETURN_ON_ERROR(iErrCode);

    // Insert row
    iErrCode = pMessages->InsertRow(SystemEmpireMessages::Template, pvData, &iKey);
    RETURN_ON_ERROR(iErrCode);

    //////////////////////////
    // Trim excess messages //
    //////////////////////////

    // Get num messages
    iErrCode = pMessages->GetNumCachedRows(&iNumMessages);
    RETURN_ON_ERROR(iErrCode);

    // Get unread message count
    iErrCode = GetNumUnreadSystemMessagesPrivate(pMessages, &iNumUnreadMessages);
    RETURN_ON_ERROR(iErrCode);
    Assert (iNumMessages >= iNumUnreadMessages);

    iErrCode = DeleteOverflowMessages(pMessages, SystemEmpireMessages::TimeStamp, SystemEmpireMessages::Unread, iNumMessages, iNumUnreadMessages, iMaxNumMessages, true);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumber -> Number of messages in message queue
//
// Returns the number of system messages the empire has in its queue

int GameEngine::GetNumSystemMessages(int iEmpireKey, unsigned int* piNumber)
{
    GET_SYSTEM_EMPIRE_MESSAGES(pszMessages, iEmpireKey);
    return t_pCache->GetNumCachedRows(pszMessages, piNumber);
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
    AutoRelease<ICachedTable> rel(pMessages);

    GET_SYSTEM_EMPIRE_MESSAGES(pszMessages, iEmpireKey);
    iErrCode = t_pCache->GetTable(pszMessages, &pMessages);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNumUnreadSystemMessagesPrivate(pMessages, piNumber);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetNumUnreadSystemMessagesPrivate (ICachedTable* pMessages, unsigned int* piNumber)
{
    int iErrCode = pMessages->GetEqualKeys(SystemEmpireMessages::Unread, MESSAGE_UNREAD, NULL, piNumber);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iEmpireKey -> Source of message
// pszMessage -> Message text
//
// Send a message to all empires on the server

int GameEngine::SendMessageToAll (int iEmpireKey, const char* pszMessage) {

    // TODOTODO - Rewrite broadcast to all
    return OK;

    //int iErrCode = OK;
    //unsigned int iDestKey = NO_KEY;

    //// Send messages
    //while (true) {

    //    iErrCode = t_pCache->GetNextKey (SYSTEM_EMPIRE_DATA, iDestKey, &iDestKey);
    //    if (iErrCode != OK) {
    //        if (iErrCode == ERROR_DATA_NOT_FOUND) {
    //            iErrCode = OK;
    //        } else Assert (false);
    //        break;
    //    }

    //    // Best effort
    //    SendSystemMessage (iDestKey, pszMessage, iEmpireKey, MESSAGE_ADMINISTRATOR);
    //}

    //return iErrCode;
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

int GameEngine::GetSavedSystemMessages(int iEmpireKey, unsigned int** ppiMessageKey, Variant*** pppvData, unsigned int* piNumMessages)
{
    GET_SYSTEM_EMPIRE_MESSAGES(strMessages, iEmpireKey);
    int iErrCode = t_pCache->ReadColumns(
        strMessages,
        SystemEmpireMessages::NumColumns,
        SystemEmpireMessages::ColumnNames,
        ppiMessageKey,
        pppvData,
        piNumMessages
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

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

int GameEngine::GetUnreadSystemMessages (int iEmpireKey, Variant*** pppvMessage, unsigned int** ppiMessageKey, unsigned int* piNumMessages)
{
    int iErrCode;
    
    GET_SYSTEM_EMPIRE_MESSAGES(strMessages, iEmpireKey);
    GET_SYSTEM_EMPIRE_DATA(strSystemEmpireData, iEmpireKey);

    unsigned int i, * piMessageKey = NULL;
    AutoFreeKeys freeKeys(piMessageKey);

    ICachedTable* pMessages = NULL;
    AutoRelease<ICachedTable> rel(pMessages);

    *pppvMessage = NULL;
    *ppiMessageKey = NULL;
    *piNumMessages = 0;

    Variant vTemp;
    iErrCode = t_pCache->ReadData(strSystemEmpireData, iEmpireKey, SystemEmpireData::MaxNumSystemMessages, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    unsigned int iMaxNumMessages = vTemp.GetInteger();

    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    RETURN_ON_ERROR(iErrCode);

    // Get all unread messages
    unsigned int iNumMessages;
    iErrCode = pMessages->GetEqualKeys(SystemEmpireMessages::Unread, MESSAGE_UNREAD, &piMessageKey, &iNumMessages);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    Variant** ppvMessage = new Variant*[iNumMessages];
    Assert(ppvMessage);
    Algorithm::AutoDelete<Variant*> del(ppvMessage, true);
    memset(ppvMessage, 0, iNumMessages * sizeof(Variant*));

    UTCTime* ptTime = (UTCTime*)StackAlloc(iNumMessages * sizeof(UTCTime));

    for (i = 0; i < iNumMessages; i ++)
    {
        iErrCode = pMessages->ReadRow(piMessageKey[i], ppvMessage + i);
        GOTO_CLEANUP_ON_ERROR(iErrCode);

        ptTime[i] = ppvMessage[i][SystemEmpireMessages::iTimeStamp].GetInteger64();
        if (ppvMessage[i][SystemEmpireMessages::iType].GetInteger() == MESSAGE_NORMAL)
        {
            iErrCode = pMessages->WriteData(piMessageKey[i], SystemEmpireMessages::Unread, MESSAGE_READ);
            GOTO_CLEANUP_ON_ERROR(iErrCode);
        }
    }

    // Sort the read messages oldest to newest
    Algorithm::QSortThreeAscending<UTCTime, Variant*, unsigned int>(ptTime, ppvMessage, piMessageKey, iNumMessages);

    // All messages have now been read, so we need to make sure that if we have more saved messages 
    // than the max, we delete the oldest messages (best effort)
    unsigned int iTotalNumMessages;
    iErrCode = pMessages->GetNumCachedRows(&iTotalNumMessages);
    GOTO_CLEANUP_ON_ERROR(iErrCode);

    // Delete stale messages
    iErrCode = DeleteOverflowMessages(pMessages, SystemEmpireMessages::TimeStamp, SystemEmpireMessages::Unread, iTotalNumMessages, 0, iMaxNumMessages, false);
    GOTO_CLEANUP_ON_ERROR(iErrCode);

    *piNumMessages = iNumMessages;

    *ppiMessageKey = piMessageKey;
    piMessageKey = NULL;

    *pppvMessage = ppvMessage;
    ppvMessage = NULL;

Cleanup:

    if (ppvMessage)
    {
        for (i = 0; i < iNumMessages; i ++)
        {
            if (ppvMessage[i])
            {
                t_pCache->FreeData(ppvMessage[i]);
                ppvMessage[i] = NULL;
            }
        }
    }

    return iErrCode;
}

                                            
// Input:
// iEmpireKey -> Empire's integer key
// iKey -> Key of a system message
//
// Delete a given message from the message queue.  Message has always been read.

int GameEngine::DeleteSystemMessage(int iEmpireKey, unsigned int iKey)
{
    GET_SYSTEM_EMPIRE_MESSAGES (pszMessages, iEmpireKey);
    return t_pCache->DeleteRow(pszMessages, iKey);
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

int GameEngine::GetNumGameMessages(int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumber)
{
    GET_GAME_EMPIRE_MESSAGES(pszMessages, iGameClass, iGameNumber, iEmpireKey);
    return t_pCache->GetNumCachedRows(pszMessages, piNumber);
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

int GameEngine::GetNumUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumber)
{
    int iErrCode;
    ICachedTable* pMessages = NULL;
    AutoRelease<ICachedTable> rel(pMessages);

    GET_GAME_EMPIRE_MESSAGES(pszMessages, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->GetTable(pszMessages, &pMessages);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNumUnreadGameMessagesPrivate(pMessages, piNumber);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetNumUnreadGameMessagesPrivate (ICachedTable* pMessages, unsigned int* piNumber)
{
    int iErrCode = pMessages->GetEqualKeys(GameEmpireMessages::Unread, MESSAGE_UNREAD, NULL, piNumber);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

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

int GameEngine::SendGameMessage(int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, 
                                int iSourceKey, int iFlags, const UTCTime& tSendTime)
{
    int iErrCode;
    unsigned int iNumMessages, iNumUnreadMessages;

    bool bFlag;
    Variant vTemp;

    UTCTime tTime = tSendTime;
    
    // Make sure private messages are allowed
    if (!(iFlags & (MESSAGE_BROADCAST | MESSAGE_SYSTEM | MESSAGE_ADMINISTRATOR | MESSAGE_TOURNAMENT_ADMINISTRATOR)))
    {
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if (!(vTemp.GetInteger() & PRIVATE_MESSAGES))
        {
            return ERROR_CANNOT_SEND_MESSAGE;
        }
    }

    // Make sure both empires are still in the game
    iErrCode = IsEmpireInGame(iGameClass, iGameNumber, iEmpireKey, &bFlag);
    RETURN_ON_ERROR(iErrCode);
    if (!bFlag)
    {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }

    if (!(iFlags & (MESSAGE_SYSTEM | MESSAGE_ADMINISTRATOR | MESSAGE_TOURNAMENT_ADMINISTRATOR)))
    {
        iErrCode = IsEmpireInGame(iGameClass, iGameNumber, iSourceKey, &bFlag);
        RETURN_ON_ERROR(iErrCode);
        if (!bFlag)
        {
            return ERROR_EMPIRE_IS_NOT_IN_GAME;
        }

        // Is empire ignoring other empire?
        iErrCode = GetEmpireIgnoreMessages(iGameClass, iGameNumber, iEmpireKey, iSourceKey, &bFlag);
        if (iErrCode == ERROR_EMPIRE_IS_NOT_IN_DIPLOMACY)
        {
            return iErrCode;
        }
        RETURN_ON_ERROR(iErrCode);
        if (bFlag)
        {
            return ERROR_EMPIRE_IS_IGNORING_SENDER;
        }

        if (iFlags & MESSAGE_BROADCAST)
        {
            iErrCode = GetEmpireOption (iGameClass, iGameNumber, iEmpireKey, IGNORE_BROADCASTS, &bFlag);
            RETURN_ON_ERROR(iErrCode);
            if (bFlag)
            {
                return ERROR_EMPIRE_IS_IGNORING_BROADCASTS;
            }
        }
    }

    // Okay, go ahead and deliver the message
    GET_GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::MaxNumGameMessages, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    const unsigned int iMaxNumMessages = vTemp.GetInteger();

    // Get time if necessary
    if (tTime == NULL_TIME)
    {
        Time::GetTime (&tTime);
    }

    // Insert the message into the table
    Variant pvData[GameEmpireMessages::NumColumns];
    
    pvData[GameEmpireMessages::iGameClass] = iGameClass;
    pvData[GameEmpireMessages::iGameNumber] = iGameNumber;
    pvData[GameEmpireMessages::iEmpireKey] = iEmpireKey;
    pvData[GameEmpireMessages::iUnread] = MESSAGE_UNREAD;
    pvData[GameEmpireMessages::iSourceKey] = iSourceKey;

    if (iSourceKey == SYSTEM)
    {
        iFlags |= MESSAGE_SYSTEM;

        pvData[GameEmpireMessages::iSourceName] = (const char*)NULL;
        pvData[GameEmpireMessages::iSourceSecret] = (int64)0;
    }
    else
    {
        GET_SYSTEM_EMPIRE_DATA(strEmpire, iSourceKey);
        iErrCode = t_pCache->ReadData(strEmpire, iSourceKey, SystemEmpireData::Name, pvData + GameEmpireMessages::iSourceName);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strEmpire, iSourceKey, SystemEmpireData::SecretKey, pvData + GameEmpireMessages::iSourceSecret);
        RETURN_ON_ERROR(iErrCode);
    }
    
    pvData[GameEmpireMessages::iTimeStamp] = tTime;
    pvData[GameEmpireMessages::iFlags] = iFlags;
    pvData[GameEmpireMessages::iText] = pszMessage;
    Assert(pvData[GameEmpireMessages::iText].GetCharPtr());

    ICachedTable* pMessages = NULL;
    AutoRelease<ICachedTable> rel(pMessages);

    iErrCode = t_pCache->GetTable(strGameEmpireMessages, &pMessages);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pMessages->InsertRow(GameEmpireMessages::Template, pvData, NULL);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pMessages->GetNumCachedRows(&iNumMessages);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNumUnreadGameMessagesPrivate (pMessages, &iNumUnreadMessages);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = DeleteOverflowMessages(pMessages, GameEmpireMessages::TimeStamp, GameEmpireMessages::Unread, iNumMessages, iNumUnreadMessages, iMaxNumMessages, true);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::DeleteOverflowMessages(ICachedTable* pMessages, const char* pszTimeStampColumn, 
                                       const char* pszUnreadColumn, unsigned int iNumMessages, 
                                       unsigned int iNumUnreadMessages, unsigned int iMaxNumMessages, 
                                       bool bCheckUnread)
{
    int iErrCode = OK;

    unsigned int* piKey = NULL;
    AutoFreeKeys freeKeys(piKey);

    Variant* pvTime = NULL;
    AutoFreeData freeData(pvTime);

    // Check for message overflow
    if ((iNumMessages - iNumUnreadMessages) > iMaxNumMessages)
    {
        // Read timestamps from table
        unsigned int iNumRows;
        iErrCode = pMessages->ReadColumn(pszTimeStampColumn, &piKey, &pvTime, &iNumRows);
        RETURN_ON_ERROR(iErrCode);
        Assert (iNumRows == iNumMessages);

        // Sort timestamps
        Algorithm::QSortTwoAscending<Variant, unsigned int>(pvTime, piKey, iNumRows);

        unsigned int i = 0;
        while (i < iNumRows && (iNumMessages - iNumUnreadMessages) > iMaxNumMessages)
        {
            if (bCheckUnread)
            {
                // See if the message is unread
                int iUnread;
                iErrCode = pMessages->ReadData(piKey[i], pszUnreadColumn, &iUnread);
                RETURN_ON_ERROR(iErrCode);

                if (iUnread != MESSAGE_READ)
                {
                    // Nothing younger than this can have been read, so give up
                    break;
                }
            }

            // Delete the message
            iErrCode = pMessages->DeleteRow(piKey[i]);
            RETURN_ON_ERROR(iErrCode);

            iNumMessages --;
            i ++;
        }
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

int GameEngine::GetSavedGameMessages(int iGameClass, int iGameNumber, int iEmpireKey, 
                                     unsigned int** ppiMessageKey, Variant*** ppvData, unsigned int* piNumMessages)
{
    int iErrCode;

    GET_GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadColumns(
        pszMessages, 
        countof(GameEmpireMessages::ColumnNames),
        GameEmpireMessages::ColumnNames,
        ppiMessageKey,
        ppvData,
        piNumMessages
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

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

int GameEngine::GetUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, Variant*** pppvMessage, unsigned int* piNumMessages)
{
    GET_GAME_EMPIRE_MESSAGES (strMessages, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode;
    unsigned int* piKey = NULL, i, iNumMessages = 0, iMaxNumMessages, iTotalNumMessages;
    AutoFreeKeys freeKeys(piKey);

    ICachedTable* pMessages = NULL;
    AutoRelease<ICachedTable> release(pMessages);

    *pppvMessage = NULL;
    *piNumMessages = 0;

    Variant vTemp;
    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::MaxNumGameMessages, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iMaxNumMessages = vTemp.GetInteger();

    iErrCode = t_pCache->GetTable(strMessages, &pMessages);
    RETURN_ON_ERROR(iErrCode);

    // TODOTODO - Use ReadColumnsWhere() call
    iErrCode = pMessages->GetEqualKeys(
        GameEmpireMessages::Unread,
        MESSAGE_UNREAD,
        &piKey,
        &iNumMessages
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }

    RETURN_ON_ERROR(iErrCode);

    Variant** ppvMessage = new Variant*[iNumMessages];
    Assert(ppvMessage);
    Algorithm::AutoDelete<Variant*>(ppvMessage, true);
    memset(ppvMessage, 0, iNumMessages * sizeof(Variant*));

    UTCTime* ptTime = (UTCTime*)StackAlloc(iNumMessages * sizeof(UTCTime));

    for (i = 0; i < iNumMessages; i ++)
    {
        iErrCode = pMessages->ReadRow(piKey[i], ppvMessage + i);
        GOTO_CLEANUP_ON_ERROR(iErrCode);

        ptTime[i] = ppvMessage[i][GameEmpireMessages::iTimeStamp].GetInteger64();

        iErrCode = pMessages->WriteData(piKey[i], GameEmpireMessages::Unread, MESSAGE_READ);
        GOTO_CLEANUP_ON_ERROR(iErrCode);
    }

    // Sort the read messages oldest to newest
    Algorithm::QSortTwoAscending <UTCTime, Variant*> (ptTime, ppvMessage, iNumMessages);

    // All messages have been read, so we need to make sure that if we have more saved messages 
    // than the max, we delete the oldest messages (best effort)
    iErrCode = pMessages->GetNumCachedRows(&iTotalNumMessages);
    GOTO_CLEANUP_ON_ERROR(iErrCode);
    Assert (iTotalNumMessages >= iNumMessages);

    // Delete stale messages
    iErrCode = DeleteOverflowMessages(pMessages, GameEmpireMessages::TimeStamp, GameEmpireMessages::Unread, iTotalNumMessages, 0, iMaxNumMessages, false);
    GOTO_CLEANUP_ON_ERROR(iErrCode);

    *piNumMessages = iNumMessages;

    *pppvMessage = ppvMessage;
    ppvMessage = NULL;

Cleanup:

    if (ppvMessage)
    {
        for (i = 0; i < iNumMessages; i ++) {

            if (ppvMessage[i] != NULL)
            {
                t_pCache->FreeData(ppvMessage[i]);
                ppvMessage[i] = NULL;
            }
        }
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

int GameEngine::DeleteGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iKey)
{
    GET_GAME_EMPIRE_MESSAGES (pszMessages, iGameClass, iGameNumber, iEmpireKey);
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

int GameEngine::BroadcastGameMessage(int iGameClass, int iGameNumber, const char* pszMessage, int iSourceKey, int iFlags)
{
    int iErrCode;

    Assert (iFlags & MESSAGE_BROADCAST);

#ifdef _DEBUG

    if (iFlags & MESSAGE_SYSTEM)
    {
        Assert (!(iFlags & (MESSAGE_ADMINISTRATOR | MESSAGE_TOURNAMENT_ADMINISTRATOR)));
    }

    if (iFlags & MESSAGE_ADMINISTRATOR)
    {
        Assert (!(iFlags & MESSAGE_TOURNAMENT_ADMINISTRATOR));
    }

    if (iSourceKey == SYSTEM)
    {
        Assert (iFlags & MESSAGE_SYSTEM);
    }
    else
    {
        Assert (!(iFlags & MESSAGE_SYSTEM));
    }

#endif

    if (!(iFlags & (MESSAGE_SYSTEM | MESSAGE_ADMINISTRATOR | MESSAGE_TOURNAMENT_ADMINISTRATOR)))
    {
        bool bFlag;
        
        // Make sure source can broadcast && send system messages
        iErrCode = GetEmpireOption (iSourceKey, CAN_BROADCAST, &bFlag);
        RETURN_ON_ERROR(iErrCode);
        if (!bFlag)
        {
            return ERROR_CANNOT_SEND_MESSAGE;
        }
        
        // Make sure empire is still game
        iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iSourceKey, &bFlag);
        RETURN_ON_ERROR(iErrCode);
        if (!bFlag)
        {
            return ERROR_EMPIRE_IS_NOT_IN_GAME;
        }
    }
    
    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
   
    unsigned int iKey = NO_KEY;
    while (true)
    {
        Variant vEmpireKey;

        iErrCode = t_pCache->GetNextKey (strGameEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
            break;
        }
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strGameEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = SendGameMessage(iGameClass, iGameNumber, vEmpireKey.GetInteger(), pszMessage, iSourceKey, iFlags, NULL_TIME);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::GetSystemMessageProperty(int iEmpireKey, unsigned int iMessageKey, const char* pszColumn, Variant* pvProperty)
{
    GET_SYSTEM_EMPIRE_MESSAGES(strMessages, iEmpireKey);
    return GetMessageProperty(strMessages, iMessageKey, pszColumn, pvProperty);
}


int GameEngine::GetGameMessageProperty (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iMessageKey, const char* pszColumn, Variant* pvProperty)
{
    GET_GAME_EMPIRE_MESSAGES(strMessages, iGameClass, iGameNumber, iEmpireKey);
    return GetMessageProperty(strMessages, iMessageKey, pszColumn, pvProperty);
}

int GameEngine::GetMessageProperty(const char* strMessages, unsigned int iMessageKey, const char* pszColumn, Variant* pvProperty)
{
    int iErrCode = t_pCache->ReadData(strMessages, iMessageKey, pszColumn, pvProperty);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        iErrCode = ERROR_MESSAGE_DOES_NOT_EXIST;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
    }
    return iErrCode;
}

int GameEngine::SendFatalUpdateMessage (int iGameClass, int iGameNumber, int iEmpireKey, 
                                        const char* pszGameClassName, const String& strUpdateMessage) {

    int iErrCode;
    bool bAlive;

    iErrCode = DoesEmpireExist (iEmpireKey, &bAlive, NULL);
    RETURN_ON_ERROR(iErrCode);
    if (!bAlive)
    {
        return ERROR_EMPIRE_DOES_NOT_EXIST;
    }

    char* pszFullMessage = new char[strUpdateMessage.GetLength() + MAX_FULL_GAME_CLASS_NAME_LENGTH + 256];
    Assert(pszFullMessage);
    Algorithm::AutoDelete<char>(pszFullMessage, true);

    sprintf (
        pszFullMessage,
        "You were obliterated from %s %i. "\
        "This is the update message from your last update in the game:" NEW_PARAGRAPH "%s",
        pszGameClassName,
        iGameNumber,
        strUpdateMessage.GetCharPtr()
        );

    iErrCode = SendSystemMessage (iEmpireKey, pszFullMessage, SYSTEM, MESSAGE_SYSTEM);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}