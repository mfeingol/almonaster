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

#if !defined(AFX_CHATROOM_H__C33C0206_602F_11D1_9D22_0060083E8062__INCLUDED_)
#define AFX_CHATROOM_H__C33C0206_602F_11D1_9D22_0060083E8062__INCLUDED_

#include "Osal/Time.h"
#include "Osal/String.h"
#include "Osal/FifoQueue.h"
#include "Osal/HashTable.h"
#include "Osal/ReadWriteLock.h"

class Chatroom {
private:
	
	class Message {
	public:
		String MessageText;
		String Speaker;
		UTCTime Time;
	};

	class Speaker {
	public:
		String Name;
		UTCTime Time;
	};

	class SpeakerHashValue {
	public:
		static unsigned int GetHashValue (const char* pszSpeaker, unsigned int iNumBuckets, const void* pHashHint);
	};

	class SpeakerEquals {
	public:
		static bool Equals (const char* pszSpeakerLeft, const char* pszSpeakerRight, const void* pEqualsHint);
	};
	
	// Structures
	FifoQueue<Message*> m_mqMessageQueue;
	HashTable<const char*, Speaker*, SpeakerHashValue, SpeakerEquals> m_hSpeakerTable;

	// Locks
	ReadWriteLock m_rwMessageLock;
	ReadWriteLock m_rwSpeakerLock;

	// Rules
	Seconds m_sTimeOut;

	unsigned int m_iMaxNumSpeakers;
	unsigned int m_iMaxNumMessages;
	unsigned int m_iMaxMessageLength;

	int PostMessageWithTime (const char* pszSpeakerName, const char* pszMessage, const UTCTime& tTime);

public:

	Chatroom (unsigned int iMaxNumMessages, unsigned int iMaxNumSpeakers, unsigned int iMaxMessageLength, 
		Seconds iTimeOut);

	~Chatroom();

	int Initialize();

	int GetSpeakers (String** ppstrSpeakerName, unsigned int* piNumSpeakers);
	void FreeSpeakers (String* pstrSpeakerName);

	int GetMessages (String** ppstrMessage, String** ppstrSpeakerName, UTCTime** pptTime, unsigned int* piNumMessages);
	void FreeMessages (String* pstrMessage, String* pstrSpeakerName, UTCTime* ptTime);

	int PostMessage (const char* pszSpeakerName, const char* pszMessage);

	int ClearMessages();

	int EnterChatroom (const char* pszSpeakerName);
	int ExitChatroom (const char* pszSpeakerName);

	unsigned int GetMaxNumMessages();
	unsigned int GetMaxNumSpeakers();
	
	Seconds GetTimeOut();
};

#endif // !defined(AFX_CHATROOM_H__C33C0206_602F_11D1_9D22_0060083E8062__INCLUDED_)