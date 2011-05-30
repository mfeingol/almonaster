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

Chatroom::Chatroom (unsigned int iMaxNumMessages, 
					unsigned int iMaxNumSpeakers, 
					unsigned int iMaxMessageLength, 
					Seconds iTimeOut
					) : m_hSpeakerTable (NULL, NULL) {

	m_iMaxNumMessages = iMaxNumMessages;
	m_iMaxNumSpeakers = iMaxNumSpeakers;
	m_iMaxMessageLength = iMaxMessageLength;

	m_sTimeOut = iTimeOut;
}

Chatroom::~Chatroom() {

	Message* pMessage = NULL;
	while (m_mqMessageQueue.Pop (&pMessage)) {
		delete pMessage;
	}
	
	HashTableIterator<const char*, Speaker*> htiSpeaker;
	while (m_hSpeakerTable.GetNextIterator (&htiSpeaker)) {
		delete htiSpeaker.GetData();
	}
}

int Chatroom::Initialize() {

	return m_hSpeakerTable.Initialize (m_iMaxNumSpeakers) ? OK : ERROR_OUT_OF_MEMORY;
}


int Chatroom::GetSpeakers (String** ppstrSpeakerName, unsigned int* piNumSpeakers) {

	int iErrCode = OK;

	HashTableIterator<const char*, Speaker*> htiSpeaker;
	unsigned int iNumSpeakers = 0;

	String* pstrSpeakerName = new String [m_iMaxNumSpeakers];
	if (pstrSpeakerName == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	m_rwSpeakerLock.WaitReader();

	while (m_hSpeakerTable.GetNextIterator (&htiSpeaker)) {

		pstrSpeakerName[iNumSpeakers] = htiSpeaker.GetData()->Name;

		if (pstrSpeakerName[iNumSpeakers].GetCharPtr() == NULL) {
			iErrCode = ERROR_OUT_OF_MEMORY;
			goto ErrorExit;
		}

		iNumSpeakers ++;
	}

	m_rwSpeakerLock.SignalReader();

	*ppstrSpeakerName = pstrSpeakerName;
	*piNumSpeakers = iNumSpeakers;

	return iErrCode;

ErrorExit:

	m_rwSpeakerLock.SignalReader();
	delete [] pstrSpeakerName;

	return iErrCode;
}

void Chatroom::FreeSpeakers (String* pstrSpeakerName) {

	if (pstrSpeakerName != NULL) {
		delete [] pstrSpeakerName;
	}
}

int Chatroom::GetMessages (String** ppstrMessage, String** ppstrSpeakerName, UTCTime** pptTime, 
						   unsigned int* piNumMessages) {

	int iErrCode = OK;

	int iNumMessages = m_mqMessageQueue.GetNumElements(), i = 0;

	QueueIterator<Message*> qiIterator;
	Message* pMessage;

	String* pstrMessage = new String [iNumMessages * 2];
	if (pstrMessage == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	String* pstrSpeakerName = pstrMessage + iNumMessages;
	UTCTime* ptTime = new UTCTime [iNumMessages];
	if (ptTime == NULL) {
		delete [] pstrMessage;
		return ERROR_OUT_OF_MEMORY;
	}

	m_rwMessageLock.WaitReader();

	while (m_mqMessageQueue.GetNextIterator (&qiIterator)) {

		pMessage = qiIterator.GetData();

		pstrMessage[i] = pMessage->MessageText;
		pstrSpeakerName[i] = pMessage->Speaker;
		ptTime[i] = pMessage->Time;

		if (pstrMessage[i].GetCharPtr() == NULL ||
			pstrSpeakerName[i].GetCharPtr() == NULL) {

			iErrCode = ERROR_OUT_OF_MEMORY;
			goto ErrorExit;
		}

		i ++;
	}

	m_rwMessageLock.SignalReader();

	*piNumMessages = iNumMessages;
	*ppstrMessage = pstrMessage;
	*ppstrSpeakerName = pstrSpeakerName;
	*pptTime = ptTime;

	return iErrCode;

ErrorExit:

	m_rwMessageLock.SignalReader();

	delete [] pstrMessage;
	delete [] ptTime;

	return iErrCode;
}


void Chatroom::FreeMessages (String* pstrMessage, String* pstrSpeakerName, UTCTime* ptTime) {

	if (pstrMessage != NULL) {
		delete [] pstrMessage;
	}

	// pstrSpeakerName is a part of pstrMessage

	if (ptTime != NULL) {
		delete [] ptTime;
	}
}


int Chatroom::PostMessage (const char* pszSpeakerName, const char* pszMessage) {

	if (pszSpeakerName == NULL || pszMessage == NULL) {
		return ERROR_INVALID_ARGUMENT;
	}

	UTCTime tTime;
	Time::GetTime (&tTime);

	return PostMessageWithTime (pszSpeakerName, pszMessage, tTime);
}

int Chatroom::ClearMessages() {

	m_rwMessageLock.WaitWriter();
	m_mqMessageQueue.Clear();
	m_rwMessageLock.SignalWriter();

	return OK;
}

int Chatroom::PostMessageWithTime (const char* pszSpeakerName, const char* pszMessage, const UTCTime& tTime) {

	bool bTruncated = false;

	Message* pMessage = new Message;
	if (pMessage == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	if (String::StrLen (pszMessage) > m_iMaxMessageLength) {

		// Truncate message
		char* pszSubMessage = (char*) StackAlloc (m_iMaxMessageLength * sizeof (char) + sizeof (char));
		memcpy (pszSubMessage, pszMessage, m_iMaxMessageLength);
		pszSubMessage [m_iMaxMessageLength] = '\0';

		pMessage->MessageText = pszSubMessage;

		bTruncated = true;

	} else {

		// Copy message
		pMessage->MessageText = pszMessage;
	}

	pMessage->Speaker = pszSpeakerName;

	if (pMessage->MessageText.GetCharPtr() == NULL ||
		pMessage->Speaker.GetCharPtr() == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	pMessage->Time = tTime;

	m_rwMessageLock.WaitWriter();

	if (!m_mqMessageQueue.Push (pMessage)) {
		delete pMessage;
		return ERROR_OUT_OF_MEMORY;
	}

	pMessage = NULL;

	if (m_mqMessageQueue.GetNumElements() > m_iMaxNumMessages) {

		bool bRetVal = m_mqMessageQueue.Pop (&pMessage);
		Assert (bRetVal);
	}

	m_rwMessageLock.SignalWriter();

	if (pMessage != NULL) {
		delete pMessage;
	}

	return bTruncated ? WARNING : OK ;
}


int Chatroom::EnterChatroom (const char* pszSpeakerName) {

	if (pszSpeakerName == NULL) {
		return ERROR_INVALID_ARGUMENT;
	}

	int iErrCode = OK;

	bool bInAlready = false;

	Speaker* pSpeaker;

	LinkedList<const char*> liDelList;
	ListIterator<const char*> liIterator;

	const char* pszName;
	char* pszPost = NULL;

	HashTableIterator<const char*, Speaker*> htiSpeaker;

	UTCTime tCurrentTime, tTimeOut, tTimeOutTime;
	Time::GetTime (&tCurrentTime);
	Time::SubtractSeconds (tCurrentTime, m_sTimeOut, &tTimeOut);

	m_rwSpeakerLock.WaitWriter();

	// See if we're in already
	if (m_hSpeakerTable.FindFirst (pszSpeakerName, &pSpeaker)) {
		pSpeaker->Time = tCurrentTime;
		bInAlready = true;
	}

	char pszTimeOut [64 + MAX_EMPIRE_NAME_LENGTH];

	// Check for speaker timeouts
	while (m_hSpeakerTable.GetNextIterator (&htiSpeaker)) {

		// Check for timeout
		if (htiSpeaker.GetData()->Time < tTimeOut) {

			pszName = htiSpeaker.GetData()->Name.GetCharPtr();
			liDelList.PushLast (pszName);

			Time::AddSeconds (htiSpeaker.GetData()->Time, m_sTimeOut, &tTimeOutTime);

			sprintf (pszTimeOut, "%s timed out of the chatroom", pszName);

			iErrCode = PostMessageWithTime (
				SYSTEM_MESSAGE_SENDER, 
				pszTimeOut,
				tTimeOutTime
				);

			if (iErrCode != OK) {
				goto Cleanup;
			}
		}
	}

	if (!bInAlready) {
		
		// Make sure we aren't at the max number of speakers
		if (m_hSpeakerTable.GetNumElements() >= m_iMaxNumSpeakers) {
			iErrCode = ERROR_TOO_MANY_SPEAKERS;
			goto Cleanup;
		}

		pSpeaker = new Speaker;
		if (pSpeaker == NULL) {
			iErrCode = ERROR_OUT_OF_MEMORY;
			goto Cleanup;
		}

		pSpeaker->Name = pszSpeakerName;
		if (pSpeaker->Name.GetCharPtr() == NULL) {
			iErrCode = ERROR_OUT_OF_MEMORY;
			goto Cleanup;
		}

		pSpeaker->Time = tCurrentTime;

		if (!m_hSpeakerTable.Insert (pSpeaker->Name.GetCharPtr(), pSpeaker)) {
			iErrCode = ERROR_OUT_OF_MEMORY;
			goto Cleanup;
		}
		
		size_t stLen = strlen (pszSpeakerName);
		size_t stLen2 = sizeof (" joined the chatroom");

		pszPost = (char*) StackAlloc (stLen + stLen2);
		
		memcpy (pszPost, pszSpeakerName, stLen);
		memcpy (pszPost + stLen, " joined the chatroom", stLen2);
	}

Cleanup:

	if (pszPost != NULL) {
		iErrCode = PostMessage (SYSTEM_MESSAGE_SENDER, pszPost);
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

	Speaker* pSpeaker;
	int iErrCode = ERROR_NOT_IN_CHATROOM;

	m_rwSpeakerLock.WaitWriter();

	bool bRetVal = m_hSpeakerTable.DeleteFirst (pszSpeakerName, NULL, &pSpeaker);

	m_rwSpeakerLock.SignalWriter();

	if (bRetVal) {

		delete pSpeaker;

		size_t stLen = strlen (pszSpeakerName);
		size_t stLen2 = sizeof (" left the chatroom");

		char* pszPost = (char*) StackAlloc (stLen + stLen2);
		
		memcpy (pszPost, pszSpeakerName, stLen);
		memcpy (pszPost + stLen, " left the chatroom", stLen2);

		iErrCode = PostMessage (SYSTEM_MESSAGE_SENDER, pszPost);
	}

	return iErrCode;
}

unsigned int Chatroom::GetMaxNumMessages() {
	return m_iMaxNumMessages;
}

unsigned int Chatroom::GetMaxNumSpeakers() {
	return m_iMaxNumSpeakers;
}

Seconds Chatroom::GetTimeOut() {
	return m_sTimeOut;
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