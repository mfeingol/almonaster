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

#if !defined(AFX_CHATROOM_H__C33C0206_602F_11D1_9D22_0060083E8062__INCLUDED_)
#define AFX_CHATROOM_H__C33C0206_602F_11D1_9D22_0060083E8062__INCLUDED_

#include "Osal/Time.h"
#include "Osal/String.h"
#include "Osal/FifoQueue.h"
#include "Osal/HashTable.h"
#include "Osal/ReadWriteLock.h"
#include "SqlDatabase.h"

struct ChatroomConfig {
    size_t cchMaxSpeakerNameLen;
    Seconds sTimeOut;
    unsigned int iMaxNumMessages;
    unsigned int iMaxNumSpeakers;
    unsigned int iMaxMessageLength;
    bool bPostSystemMessages;
};

struct ChatroomMessage {
    String strMessageText;
    String strSpeaker;
    UTCTime tTime;
    int iFlags;
    unsigned int iKey;
};

struct ChatroomSpeaker {
    String strName;
    UTCTime tTime;
};

// iFlags
#define CHATROOM_MESSAGE_SYSTEM (0x00000001)

class Chatroom {
private:

    class SpeakerHashValue {
    public:
        static unsigned int GetHashValue (const char* pszSpeaker, unsigned int iNumBuckets, const void* pHashHint);
    };

    class SpeakerEquals {
    public:
        static bool Equals (const char* pszSpeakerLeft, const char* pszSpeakerRight, const void* pEqualsHint);
    };
    
    // Structures
    HashTable<const char*, ChatroomSpeaker*, SpeakerHashValue, SpeakerEquals> m_hSpeakerTable;

    int InitializeFromDatabase();

    // Locks
    ReadWriteLock m_rwSpeakerLock;

    // Rules
    ChatroomConfig m_ccConf;
    int PostMessageWithTime(const char* pszSpeakerName,
                            const char* pszMessage,
                            const UTCTime& tTime, 
                            int iFlags,
                            unsigned int iKey);

public:

    Chatroom();
    ~Chatroom();

    int Initialize(const ChatroomConfig& ccConf);

    int GetSpeakers(ChatroomSpeaker** ppcsSpeakers, unsigned int* piNumSpeakers);
    void FreeSpeakers(ChatroomSpeaker* pcsSpeakers);

    int GetMessages(ChatroomMessage** ppcmMessages, unsigned int* piNumMessages);
    void FreeMessages(ChatroomMessage* pcmMessages);

    int PostMessage(const char* pszSpeakerName, const char* pszMessage, int iFlags);

    int ClearMessages();

    int EnterChatroom(const char* pszSpeakerName);
    int ExitChatroom(const char* pszSpeakerName);

    unsigned int GetMaxNumMessages();
    unsigned int GetMaxNumSpeakers();
    
    Seconds GetTimeOut();
};

#endif // !defined(AFX_CHATROOM_H__C33C0206_602F_11D1_9D22_0060083E8062__INCLUDED_)