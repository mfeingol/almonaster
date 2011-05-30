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

#include "Osal/Time.h"

#include "CChatroom.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

Chatroom::Chatroom (const ChatroomConfig& ccConf) : 
    m_hSpeakerTable (NULL, NULL) {

    m_ccConf = ccConf;
}

Chatroom::~Chatroom() {

    ChatroomMessage* pMessage = NULL;
    while (m_mqMessageQueue.Pop (&pMessage)) {
        delete pMessage;
    }
    
    HashTableIterator<const char*, ChatroomSpeaker*> htiSpeaker;
    while (m_hSpeakerTable.GetNextIterator (&htiSpeaker)) {
        delete htiSpeaker.GetData();
    }
}

int Chatroom::Initialize() {

    int iErrCode;

    iErrCode = m_rwMessageLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_rwSpeakerLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    return m_hSpeakerTable.Initialize (m_ccConf.iMaxNumSpeakers) ? OK : ERROR_OUT_OF_MEMORY;
}


int Chatroom::GetSpeakers (ChatroomSpeaker** ppcsSpeaker, unsigned int* piNumSpeakers) {

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

int Chatroom::GetMessages (ChatroomMessage** ppcmMessage, unsigned int* piNumMessages) {

    int iNumMessages = 0, iErrCode = OK;

    ChatroomMessage* pcmMessage = NULL;
    QueueIterator<ChatroomMessage*> qiIterator;
    ChatroomMessage* pMessage;

    *ppcmMessage = NULL;
    *piNumMessages = 0;

    m_rwMessageLock.WaitReader();

    int iAlloc = m_mqMessageQueue.GetNumElements();
    if (iAlloc == 0) {
        goto Cleanup;
    }

    pcmMessage = new ChatroomMessage [iAlloc];
    if (pcmMessage == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    while (m_mqMessageQueue.GetNextIterator (&qiIterator)) {

        pMessage = qiIterator.GetData();

        pcmMessage[iNumMessages].strMessageText = pMessage->strMessageText;
        if (pcmMessage[iNumMessages].strMessageText.GetCharPtr() == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        if (pMessage->strSpeaker.GetCharPtr() == NULL) {

            pcmMessage[iNumMessages].strSpeaker = (const char*) NULL;
        
        } else {

            pcmMessage[iNumMessages].strSpeaker = pMessage->strSpeaker;
            if (pcmMessage[iNumMessages].strSpeaker.GetCharPtr() == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
        }

        pcmMessage[iNumMessages].tTime = pMessage->tTime;
        pcmMessage[iNumMessages].iFlags = pMessage->iFlags;

        iNumMessages ++;
    }

Cleanup:

    m_rwMessageLock.SignalReader();

    if (iErrCode == OK) {

        *ppcmMessage = pcmMessage;
        *piNumMessages = iNumMessages;
    }
    else if (pcmMessage != NULL) {

        delete [] pcmMessage;
    }

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

    return PostMessageWithTime (pszSpeakerName, pszMessage, tTime, iFlags);
}

int Chatroom::ClearMessages() {

    m_rwMessageLock.WaitWriter();
    m_mqMessageQueue.Clear();
    m_rwMessageLock.SignalWriter();

    return OK;
}

int Chatroom::PostMessageWithTime (const char* pszSpeakerName, const char* pszMessage, const UTCTime& tTime,
                                   int iFlags) {

    bool bTruncated = false;

    ChatroomMessage* pMessage = new ChatroomMessage;
    if (pMessage == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (String::StrLen (pszMessage) > m_ccConf.iMaxMessageLength) {

        // Truncate message
        char* pszSubMessage = (char*) StackAlloc (m_ccConf.iMaxMessageLength * sizeof (char) + sizeof (char));
        memcpy (pszSubMessage, pszMessage, m_ccConf.iMaxMessageLength);
        pszSubMessage [m_ccConf.iMaxMessageLength] = '\0';

        pMessage->strMessageText = pszSubMessage;

        bTruncated = true;

    } else {

        // Copy message
        pMessage->strMessageText = pszMessage;
    }

    if (pMessage->strMessageText.GetCharPtr() == NULL) {
        delete pMessage;
        return ERROR_OUT_OF_MEMORY;
    }

    if (pszSpeakerName == NULL) {
        
        pMessage->strSpeaker = (const char*) NULL;
    
    } else {

        pMessage->strSpeaker = pszSpeakerName;
        if (pMessage->strSpeaker.GetCharPtr() == NULL) {
            delete pMessage;
            return ERROR_OUT_OF_MEMORY;
        }
    }

    pMessage->tTime = tTime;
    pMessage->iFlags = iFlags;

    m_rwMessageLock.WaitWriter();

    if (!m_mqMessageQueue.Push (pMessage)) {
        m_rwMessageLock.SignalWriter();
        delete pMessage;
        return ERROR_OUT_OF_MEMORY;
    }

    pMessage = NULL;

    if (m_mqMessageQueue.GetNumElements() > m_ccConf.iMaxNumMessages) {

        bool bRetVal = m_mqMessageQueue.Pop (&pMessage);
        Assert (bRetVal);
    }

    m_rwMessageLock.SignalWriter();

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
                iErrCode = PostMessageWithTime (NULL, pszTimeOut, tTimeOutTime, 0);
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
        return stricmp (pszSpeakerLeft, pszSpeakerRight) == 0;
    }
}
