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
    while (m_hSpeakerTable.GetNextIterator (&htiSpeaker))
    {
        delete htiSpeaker.GetData();
    }
}

int Chatroom::Initialize(const ChatroomConfig& ccConf)
{
    m_ccConf = ccConf;

    int iErrCode = m_rwSpeakerLock.Initialize();
    Assert(iErrCode == OK);

    bool ret = m_hSpeakerTable.Initialize (ccConf.iMaxNumSpeakers);
    Assert(ret);

    return iErrCode;
}

void Chatroom::GetSpeakers(ChatroomSpeaker** ppcsSpeaker, unsigned int* piNumSpeakers)
{
    ChatroomSpeaker* pcsSpeaker = NULL;
    HashTableIterator<const char*, ChatroomSpeaker*> htiSpeaker;
    unsigned int iNumSpeakers = 0;

    *ppcsSpeaker = NULL;
    *piNumSpeakers = 0;

    m_rwSpeakerLock.WaitReader();

    int iAlloc = m_hSpeakerTable.GetNumElements();
    if (iAlloc > 0)
    {
        pcsSpeaker = new ChatroomSpeaker [iAlloc];
        Assert(pcsSpeaker);

        while (m_hSpeakerTable.GetNextIterator (&htiSpeaker))
        {
            const String& strName = htiSpeaker.GetData()->strName;

            pcsSpeaker[iNumSpeakers].strName = strName;
            Assert(pcsSpeaker[iNumSpeakers].strName.GetCharPtr());

            iNumSpeakers ++;
        }

        *ppcsSpeaker = pcsSpeaker;
        *piNumSpeakers = iNumSpeakers;
    }

    m_rwSpeakerLock.SignalReader();
}

void Chatroom::FreeSpeakers (ChatroomSpeaker* pcsSpeaker) {

    if (pcsSpeaker)
    {
        delete [] pcsSpeaker;
    }
}

int Chatroom::GetMessages(ChatroomMessage** ppcmMessage, unsigned int* piNumMessages)
{
    *ppcmMessage = NULL;
    *piNumMessages = 0;

    unsigned int iNumMessages;
    Variant** ppvData = NULL;
    AutoFreeData free(ppvData);

    int iErrCode = t_pCache->ReadColumns(SYSTEM_CHATROOM_DATA, SystemChatroomData::NumColumns, SystemChatroomData::ColumnNames, NULL, &ppvData, &iNumMessages);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        ChatroomMessage* pcmMessage = new ChatroomMessage[iNumMessages];
        Assert(pcmMessage);

        UTCTime* ptPostTime = (UTCTime*)StackAlloc(iNumMessages * sizeof(UTCTime));

        for (unsigned int i = 0; i < iNumMessages; i ++)
        {
            pcmMessage[i].strMessageText = ppvData[i][SystemChatroomData::iMessage].GetCharPtr();
            pcmMessage[i].strSpeaker = ppvData[i][SystemChatroomData::iSpeaker].GetCharPtr();
            pcmMessage[i].tTime = ptPostTime[i] = ppvData[i][SystemChatroomData::iTime].GetInteger64();
            pcmMessage[i].iFlags = ppvData[i][SystemChatroomData::iFlags].GetInteger();

            Assert(pcmMessage[i].strMessageText.GetCharPtr());
            Assert(pcmMessage[i].strSpeaker.GetCharPtr());
        }

        Algorithm::QSortTwoAscending<UTCTime, ChatroomMessage>(ptPostTime, pcmMessage, iNumMessages);

        *ppcmMessage = pcmMessage;
        *piNumMessages = iNumMessages;
    }

    return iErrCode;
}

void Chatroom::FreeMessages(ChatroomMessage* pcmMessage) {

    if (pcmMessage)
    {
        delete [] pcmMessage;
    }
}

int Chatroom::PostMessage(const char* pszSpeakerName, const char* pszMessage, int iFlags) {

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
    return t_pCache->DeleteAllRows(SYSTEM_CHATROOM_DATA);
}

int Chatroom::PostMessageWithTime(const char* pszSpeakerName, const char* pszMessage, const UTCTime& tTime, int iFlags, unsigned int iKey)
{
    bool bTruncated = false;

    if (String::StrLen (pszMessage) > m_ccConf.iMaxMessageLength)
    {
        // Truncate message
        char* pszSubMessage = (char*)StackAlloc(m_ccConf.iMaxMessageLength * sizeof(char) + sizeof(char));
        String::StrnCpy(pszSubMessage, pszMessage, m_ccConf.iMaxMessageLength);
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

    Assert(pvColVal[SystemChatroomData::iSpeaker].GetCharPtr() &&
           pvColVal[SystemChatroomData::iMessage].GetCharPtr());

    // Insert into the table
    int iErrCode = t_pCache->InsertRow(SYSTEM_CHATROOM_DATA, SystemChatroomData::Template, pvColVal, NULL);
    RETURN_ON_ERROR(iErrCode);

    unsigned int iNumMessages;
    iErrCode = t_pCache->GetNumCachedRows(SYSTEM_CHATROOM_DATA, &iNumMessages);
    RETURN_ON_ERROR(iErrCode);

    while (iNumMessages > m_ccConf.iMaxNumMessages)
    {
        unsigned int iOldestMessage;
        iErrCode = GetOldestMessage(&iOldestMessage);
        RETURN_ON_ERROR(iErrCode);

        if (iOldestMessage == NO_KEY)
          break;

        iErrCode = t_pCache->DeleteRow(SYSTEM_CHATROOM_DATA, iOldestMessage);
        RETURN_ON_ERROR(iErrCode);

        iNumMessages --;
    }

    return bTruncated ? WARNING : OK ;
}

int Chatroom::GetOldestMessage(unsigned int* piMessageKey)
{
    *piMessageKey = NO_KEY;

    unsigned int iNumMessages, * piKey = NULL;
    Variant* pvData = NULL;
    AutoFreeKeys auto_piKey(piKey);
    AutoFreeData auto_ppvData(pvData);

    int iErrCode = t_pCache->ReadColumn(SYSTEM_CHATROOM_DATA, SystemChatroomData::Time, &piKey, &pvData, &iNumMessages);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        Algorithm::QSortTwoAscending<Variant, unsigned int>(pvData, piKey, iNumMessages);
        *piMessageKey = piKey[0];
    }

    return iErrCode;
}

int Chatroom::EnterChatroom(const char* pszSpeakerName)
{
    if (pszSpeakerName == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    int iErrCode = OK;
    bool bInAlready = false;
    ChatroomSpeaker* pSpeaker;

    LinkedList<const char*> liDelList;
    ListIterator<const char*> liIterator;

    const char* pszName;

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

            if (m_ccConf.bPostSystemMessages)
            {
                sprintf(pszTimeOut, "%s timed out of the chatroom", pszName);
                iErrCode = PostMessageWithTime(NULL, pszTimeOut, tTimeOutTime, CHATROOM_MESSAGE_SYSTEM, NO_KEY);
                GOTO_CLEANUP_ON_ERROR(iErrCode);
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
        Assert(pSpeaker);

        pSpeaker->strName = pszSpeakerName;
        Assert(pSpeaker->strName.GetCharPtr());

        pSpeaker->tTime = tCurrentTime;

        bool ret = m_hSpeakerTable.Insert (pSpeaker->strName.GetCharPtr(), pSpeaker);
        Assert(ret);

        if (m_ccConf.bPostSystemMessages)
        {
            size_t stLen = strlen (pszSpeakerName);
            size_t stLen2 = sizeof (" joined the chatroom");

            char* pszPost = (char*)StackAlloc(stLen + stLen2);
            
            memcpy(pszPost, pszSpeakerName, stLen);
            memcpy(pszPost + stLen, " joined the chatroom", stLen2);

            iErrCode = PostMessage(NULL, pszPost, CHATROOM_MESSAGE_SYSTEM);
            GOTO_CLEANUP_ON_ERROR(iErrCode);
        }
    }

Cleanup:

    // Clean up the timeouts
    while (liDelList.GetNextIterator (&liIterator))
    {
        if (m_hSpeakerTable.DeleteFirst (liIterator.GetData(), NULL, &pSpeaker))
        {
            delete pSpeaker;
        }
    }

    m_rwSpeakerLock.SignalWriter();

    return iErrCode;
}


int Chatroom::ExitChatroom (const char* pszSpeakerName)
{
    if (pszSpeakerName == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    ChatroomSpeaker* pSpeaker;
    int iErrCode = ERROR_NOT_IN_CHATROOM;

    m_rwSpeakerLock.WaitWriter();
    bool bRetVal = m_hSpeakerTable.DeleteFirst (pszSpeakerName, NULL, &pSpeaker);
    m_rwSpeakerLock.SignalWriter();

    if (bRetVal)
    {
        delete pSpeaker;
        iErrCode = OK;

        if (m_ccConf.bPostSystemMessages)
        {
            size_t stLen = strlen (pszSpeakerName);
            size_t stLen2 = sizeof (" left the chatroom");

            char* pszPost = (char*) StackAlloc (stLen + stLen2);
            
            memcpy (pszPost, pszSpeakerName, stLen);
            memcpy (pszPost + stLen, " left the chatroom", stLen2);

            iErrCode = PostMessage (NULL, pszPost, CHATROOM_MESSAGE_SYSTEM);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

unsigned int Chatroom::GetMaxNumMessages()
{
    return m_ccConf.iMaxNumMessages;
}

unsigned int Chatroom::GetMaxNumSpeakers()
{
    return m_ccConf.iMaxNumSpeakers;
}

Seconds Chatroom::GetTimeOut()
{
    return m_ccConf.sTimeOut;
}

unsigned int Chatroom::SpeakerHashValue::GetHashValue(const char* pszSpeaker, unsigned int iNumBuckets, const void* pHashHint)
{
    return Algorithm::GetStringHashValue (pszSpeaker, iNumBuckets, true);
}

bool Chatroom::SpeakerEquals::Equals(const char* pszSpeakerLeft, const char* pszSpeakerRight, const void* pEqualsHint)
{
    bool bCaseSensitive = pEqualsHint != NULL;

    if (bCaseSensitive)
    {
        return String::StrCmp(pszSpeakerLeft, pszSpeakerRight) == 0;
    }
    else
    {
        return String::StriCmp(pszSpeakerLeft, pszSpeakerRight) == 0;
    }
}
