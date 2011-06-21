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

#include "Osal/Time.h"

#include "CChatroom.h"
#include "GameEngine.h"

#include <stdio.h>

Chatroom::Chatroom() 
    : 
    m_hSpeakerTable(NULL, NULL)
{
}

Chatroom::~Chatroom() {

    HashTableIterator<const char*, ChatroomSpeaker*> htiSpeaker;
    while (m_hSpeakerTable.GetNextIterator (&htiSpeaker)) {
        delete htiSpeaker.GetData();
    }
}

int Chatroom::Initialize(const ChatroomConfig& ccConf) {

    m_ccConf = ccConf;

    int iErrCode = m_rwSpeakerLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (!m_hSpeakerTable.Initialize (ccConf.iMaxNumSpeakers)) {
        return ERROR_OUT_OF_MEMORY;
    }

    return iErrCode;
}

int Chatroom::GetSpeakers(ChatroomSpeaker** ppcsSpeaker, unsigned int* piNumSpeakers) {

    ChatroomSpeaker* pcsSpeaker = NULL;
    HashTableIterator<const char*, ChatroomSpeaker*> htiSpeaker;
    unsigned int iNumSpeakers = 0;
    int iErrCode = OK;

    *ppcsSpeaker = NULL;
    *piNumSpeakers = 0;

    m_rwSpeakerLock.WaitReader();

    int iAlloc = m_hSpeakerTable.GetNumElements();
    if (iAlloc == 0) {
        goto Cleanup;
    }

    pcsSpeaker = new ChatroomSpeaker [iAlloc];
    if (pcsSpeaker == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    while (m_hSpeakerTable.GetNextIterator (&htiSpeaker)) {

        String& strName = htiSpeaker.GetData()->strName;

        pcsSpeaker[iNumSpeakers].strName = strName;
        if (pcsSpeaker[iNumSpeakers].strName.GetCharPtr() == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        iNumSpeakers ++;
    }

Cleanup:

    m_rwSpeakerLock.SignalReader();

    if (iErrCode == OK) {

        *ppcsSpeaker = pcsSpeaker;
        *piNumSpeakers = iNumSpeakers;
    
    }
    else if (pcsSpeaker != NULL) {

        delete [] pcsSpeaker;
    }

    return iErrCode;
}

void Chatroom::FreeSpeakers (ChatroomSpeaker* pcsSpeaker) {

    if (pcsSpeaker != NULL) {
        delete [] pcsSpeaker;
    }
}

int Chatroom::GetMessages(ChatroomMessage** ppcmMessage, unsigned int* piNumMessages)
{
    unsigned int iNumMessages = 0;
    ChatroomMessage* pcmMessage = NULL;

    Variant** ppvData;
    int iErrCode = t_pConn->ReadColumns(SYSTEM_CHATROOM_DATA, SystemChatroomData::NumColumns, SystemChatroomData::ColumnNames, &ppvData, &iNumMessages);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else if (iErrCode == OK)
    {
        pcmMessage = new ChatroomMessage[iNumMessages];
        if (pcmMessage == NULL)
        {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        for (unsigned int i = 0; i < iNumMessages; i ++)
        {
            pcmMessage[i].strMessageText = ppvData[i][SystemChatroomData::iMessage].GetCharPtr();
            pcmMessage[i].strSpeaker = ppvData[i][SystemChatroomData::iSpeaker].GetCharPtr();
            pcmMessage[i].tTime = ppvData[i][SystemChatroomData::iTime].GetInteger64();
            pcmMessage[i].iFlags = ppvData[i][SystemChatroomData::iFlags].GetInteger();
        }

        t_pConn->FreeData(ppvData);
    }

Cleanup:

    *ppcmMessage = pcmMessage;
    *piNumMessages = iNumMessages;

    return iErrCode;
}

void Chatroom::FreeMessages (ChatroomMessage* pcmMessage) {

    if (pcmMessage != NULL) {
        delete [] pcmMessage;
    }
}

int Chatroom::PostMessage (const char* pszSpeakerName, const char* pszMessage, int iFlags) {

    if (pszMessage == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    if (pszSpeakerName == NULL && !(iFlags & CHATROOM_MESSAGE_SYSTEM)) {
        return ERROR_INVALID_ARGUMENT;
    }

    UTCTime tTime;
    Time::GetTime (&tTime);

    return PostMessageWithTime (pszSpeakerName, pszMessage, tTime, iFlags, NO_KEY);
}

int Chatroom::ClearMessages()
{
    return t_pConn->DeleteAllRows(SYSTEM_CHATROOM_DATA);
}

int Chatroom::PostMessageWithTime(const char* pszSpeakerName, const char* pszMessage, const UTCTime& tTime, int iFlags, unsigned int iKey)
{
    bool bTruncated = false;

    if (String::StrLen (pszMessage) > m_ccConf.iMaxMessageLength)
    {
        // Truncate message
        char* pszSubMessage = (char*)StackAlloc(m_ccConf.iMaxMessageLength * sizeof(char) + sizeof(char));
        strcpy(pszSubMessage, pszMessage);
        pszMessage = pszSubMessage;
        bTruncated = true;
    }

    Variant pvColVal[SystemChatroomData::NumColumns] =
    { 
        iFlags,
        tTime, 
        pszSpeakerName, 
        pszMessage,
    };

    if (pvColVal[SystemChatroomData::iSpeaker].GetCharPtr() == NULL ||
        pvColVal[SystemChatroomData::iMessage].GetCharPtr() == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    // Insert into the table
    int iErrCode = t_pConn->InsertRow(SYSTEM_CHATROOM_DATA, SystemChatroomData::Template, pvColVal, NULL);
    if (iErrCode != OK)
    {
        return iErrCode;
    }

    return bTruncated ? WARNING : OK ;
}


int Chatroom::EnterChatroom (const char* pszSpeakerName) {

    if (pszSpeakerName == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    int iErrCode = OK;
    bool bInAlready = false;
    ChatroomSpeaker* pSpeaker;

    LinkedList<const char*> liDelList;
    ListIterator<const char*> liIterator;

    const char* pszName;
    char* pszPost = NULL;

    HashTableIterator<const char*, ChatroomSpeaker*> htiSpeaker;

    UTCTime tCurrentTime, tTimeOut, tTimeOutTime;
    Time::GetTime (&tCurrentTime);
    Time::SubtractSeconds (tCurrentTime, m_ccConf.sTimeOut, &tTimeOut);

    m_rwSpeakerLock.WaitWriter();

    // See if we're in already
    if (m_hSpeakerTable.FindFirst (pszSpeakerName, &pSpeaker)) {
        pSpeaker->tTime = tCurrentTime;
        bInAlready = true;
    }

    char* pszTimeOut = NULL;
    if (m_ccConf.bPostSystemMessages) {
        pszTimeOut = (char*) StackAlloc (64 + m_ccConf.cchMaxSpeakerNameLen);
    }

    // Check for speaker timeouts
    while (m_hSpeakerTable.GetNextIterator (&htiSpeaker)) {

        // Check for timeout
        if (htiSpeaker.GetData()->tTime < tTimeOut) {

            pszName = htiSpeaker.GetData()->strName.GetCharPtr();
            liDelList.PushLast (pszName);

            Time::AddSeconds (htiSpeaker.GetData()->tTime, m_ccConf.sTimeOut, &tTimeOutTime);

            if (m_ccConf.bPostSystemMessages) {

                sprintf (pszTimeOut, "%s timed out of the chatroom", pszName);
                iErrCode = PostMessageWithTime (NULL, pszTimeOut, tTimeOutTime, CHATROOM_MESSAGE_SYSTEM, NO_KEY);
                if (iErrCode != OK) {
                    goto Cleanup;
                }
            }
        }
    }

    if (!bInAlready) {
        
        // Make sure we aren't at the max number of speakers
        if (m_hSpeakerTable.GetNumElements() >= m_ccConf.iMaxNumSpeakers) {
            iErrCode = ERROR_TOO_MANY_SPEAKERS;
            goto Cleanup;
        }

        pSpeaker = new ChatroomSpeaker;
        if (pSpeaker == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        pSpeaker->strName = pszSpeakerName;
        if (pSpeaker->strName.GetCharPtr() == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        pSpeaker->tTime = tCurrentTime;

        if (!m_hSpeakerTable.Insert (pSpeaker->strName.GetCharPtr(), pSpeaker)) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        if (m_ccConf.bPostSystemMessages) {

            size_t stLen = strlen (pszSpeakerName);
            size_t stLen2 = sizeof (" joined the chatroom");

            pszPost = (char*) StackAlloc (stLen + stLen2);
            
            memcpy (pszPost, pszSpeakerName, stLen);
            memcpy (pszPost + stLen, " joined the chatroom", stLen2);
        }
    }

Cleanup:

    if (pszPost != NULL) {
        iErrCode = PostMessage (NULL, pszPost, CHATROOM_MESSAGE_SYSTEM);
    }

    // Clean up the timeouts
    while (liDelList.GetNextIterator (&liIterator)) {
        if (m_hSpeakerTable.DeleteFirst (liIterator.GetData(), NULL, &pSpeaker)) {
            delete pSpeaker;
        }
    }

    m_rwSpeakerLock.SignalWriter();

    return iErrCode;
}


int Chatroom::ExitChatroom (const char* pszSpeakerName) {

    if (pszSpeakerName == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    ChatroomSpeaker* pSpeaker;
    int iErrCode = ERROR_NOT_IN_CHATROOM;

    m_rwSpeakerLock.WaitWriter();
    bool bRetVal = m_hSpeakerTable.DeleteFirst (pszSpeakerName, NULL, &pSpeaker);
    m_rwSpeakerLock.SignalWriter();

    if (bRetVal) {

        delete pSpeaker;
        iErrCode = OK;

        if (m_ccConf.bPostSystemMessages) {

            size_t stLen = strlen (pszSpeakerName);
            size_t stLen2 = sizeof (" left the chatroom");

            char* pszPost = (char*) StackAlloc (stLen + stLen2);
            
            memcpy (pszPost, pszSpeakerName, stLen);
            memcpy (pszPost + stLen, " left the chatroom", stLen2);

            iErrCode = PostMessage (NULL, pszPost, CHATROOM_MESSAGE_SYSTEM);
        }
    }

    return iErrCode;
}

unsigned int Chatroom::GetMaxNumMessages() {
    return m_ccConf.iMaxNumMessages;
}

unsigned int Chatroom::GetMaxNumSpeakers() {
    return m_ccConf.iMaxNumSpeakers;
}

Seconds Chatroom::GetTimeOut() {
    return m_ccConf.sTimeOut;
}

unsigned int Chatroom::SpeakerHashValue::GetHashValue (const char* pszSpeaker, unsigned int iNumBuckets, 
                                                       const void* pHashHint) {

    return Algorithm::GetStringHashValue (pszSpeaker, iNumBuckets, true);
}

bool Chatroom::SpeakerEquals::Equals (const char* pszSpeakerLeft, const char* pszSpeakerRight, 
                                      const void* pEqualsHint) {

    bool bCaseSensitive = pEqualsHint != NULL;

    if (bCaseSensitive) {
        return strcmp (pszSpeakerLeft, pszSpeakerRight) == 0;
    } else {
        return _stricmp (pszSpeakerLeft, pszSpeakerRight) == 0;
    }
}
