<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include "../Chatroom/CChatroom.h"

// Almonaster 2.0
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

INITIALIZE_EMPIRE

enum InChatroom { CHATROOM_IN, CHATROOM_OUT, CHATROOM_UNCHECKED };

IHttpForm* pHttpForm;

unsigned int i;
InChatroom iInChatroom = CHATROOM_UNCHECKED;

// Make sure we have broadcast rights
int iErrCode;
bool bBroadcast = m_iPrivilege >= NOVICE;

if (bBroadcast) {
	iErrCode = g_pGameEngine->GetEmpireOption (m_iEmpireKey, CAN_BROADCAST, &bBroadcast);
	if (iErrCode != OK) {
		AddMessage ("Error ");
		AppendMessage (iErrCode);
		bBroadcast = false;
	}
}

if (m_bOwnPost && !m_bRedirection) {

	// Check for refresh
	if (WasButtonPressed (BID_REFRESHMESSAGES)) {
		bRedirectTest = false;
	} else {

		if (m_iPrivilege >= ADMINISTRATOR) {

			if (WasButtonPressed (BID_CLEARMESSAGES)) {

				if (g_pChatroom->ClearMessages() == OK) {
					AddMessage ("The chatroom's messages were cleared");
				} else {
					AddMessage ("The chatroom's messages could not be cleared");
				}
			}
		}

		if (bBroadcast) {

			// Check for leave
			if (WasButtonPressed (BID_LEAVETHECHATROOM)) {
				g_pChatroom->ExitChatroom (m_vEmpireName.GetCharPtr());
				AddMessage ("You left the chatroom");
				return Redirect (ACTIVE_GAME_LIST);
			}

			// Check for speak
			const char* pszMessage = NULL;
			pHttpForm = m_pHttpRequest->GetForm ("Message");
			if (pHttpForm != NULL) {
				pszMessage = pHttpForm->GetValue();
			}

			if (WasButtonPressed (BID_SPEAK)) {
				bRedirectTest = false;
			} else {
				if (pszMessage == NULL) {
					goto Redirection;
				}
			}

			if (pszMessage != NULL && *pszMessage != '\0') {

				iInChatroom = g_pChatroom->EnterChatroom (m_vEmpireName.GetCharPtr()) == OK ? CHATROOM_IN : CHATROOM_OUT;

				if (iInChatroom == CHATROOM_IN) {

					String strFilter;
					iErrCode = HTMLFilter (pszMessage, &strFilter, MAX_NUM_SPACELESS_CHARS, false);
					if (iErrCode == OK) {
						g_pChatroom->PostMessage (m_vEmpireName.GetCharPtr(), strFilter);
					} else {
						AddMessage ("Not enough memory to post your message");
					}
				}

			} else {
				AddMessage ("You had nothing to say");
			}
		}
	}
}

SYSTEM_REDIRECT_ON_SUBMIT

SYSTEM_OPEN (false)

// Enter the chatroom
if (bBroadcast && iInChatroom == CHATROOM_UNCHECKED) {
	iInChatroom = g_pChatroom->EnterChatroom (m_vEmpireName.GetCharPtr()) == OK ? CHATROOM_IN : CHATROOM_OUT;
}

// Chatroom
if (iInChatroom == CHATROOM_OUT) { 
	%><p><strong>There are too many empires in the Chatroom. Please try back later.</strong><% 

} else {

	String* pstrMessage = NULL, * pstrSpeakerName = NULL, * pstrSpeakerInRoomName = NULL;
	UTCTime* ptTime = NULL;

	char pszDate [OS::MaxDateLength];

	unsigned int iNumSpeakers, iNumMessages;
	bool bSystem;

	Assert (!bBroadcast || iInChatroom == CHATROOM_IN);

	// Get speaket list
	iErrCode = g_pChatroom->GetSpeakers (&pstrSpeakerInRoomName, &iNumSpeakers);
	if (iErrCode != OK) {
		%><p>An error occurred reading the speaker list from the chatroom<%
		goto Cleanup;
	}

	iErrCode = g_pChatroom->GetMessages (&pstrMessage, &pstrSpeakerName, &ptTime, &iNumMessages);
	if (iErrCode != OK) {
		%><p>An error occurred reading the message list from the chatroom<%
		goto Cleanup;
	}

	if (!bBroadcast) {
		%><p><strong>You do not have the right to broadcast messages</strong><%
	}

	%><p><table border="3" cellspacing="0" cellpadding="0" width="90%"><tr><%
	%><td bgcolor="#331111"><table cellspacing="0" <%
	%>cellpadding="2" width="100%"><tr><th align="left" valign="top" bgcolor="<% 
	Write (m_vTableColor.GetCharPtr()); %>"><strong>Time</strong></th><th align="left" valign="top" bgcolor="<% 
	Write (m_vTableColor.GetCharPtr()); %>"><strong>Message</strong></th></tr><%

	for (i = 0; i < iNumMessages; i ++) { 

		%><tr><td align="left" width="25%" bgcolor="#331111"><font color="#ffff00"><% 
		iErrCode = Time::GetDateString (ptTime[i], pszDate);
		if (iErrCode != OK) {
			%><p>An error occurred converting a date string<%
			goto Cleanup;
		}
		Write (pszDate); 
		%></font></td><td align="left" width="55%" bgcolor="#331111"><strong><% 

		bSystem = pstrSpeakerName[i].Equals (SYSTEM_MESSAGE_SENDER);
		if (bSystem) { 
			%><font color="#ffffff"><%
		} else { 

			%><font color="#ffff00"><%
		}

		%>&lt;<%

		Write (pstrSpeakerName[i]); %>&gt;</font></strong><% 

		if (bSystem) { 
			%><font color="#ffffff"><% 
		} else {
			%><font color="#ffffaa"><% 
		}

		Write (pstrMessage[i]); %></font></td></tr><% 

	}

	%></table></td><td bgcolor="#331111" valign="top" width="20%"><%
	%><table cellspacing="0" cellpadding="2" width="100%"><%
	%><tr><th align="center" valign="top" width="3%" bgcolor="<% 
	Write (m_vTableColor.GetCharPtr()); 
	%>"><strong>Speakers</strong></th></tr><tr><td valign="top" bgcolor="#331111"><% 

	for (i = 0; i < iNumSpeakers; i ++) {
		Write (pstrSpeakerInRoomName[i]);
		%><br><%
	}

	%></tr></table></td></tr></table><%

	if (bBroadcast) {

		%><p><input type="text" size="75%" name="Message"> <% 
		WriteButton (BID_SPEAK);

		%><p><%

		WriteButton (BID_LEAVETHECHATROOM);
		WriteButton (BID_REFRESHMESSAGES);
	}

	if (m_iPrivilege >= ADMINISTRATOR) {

		%><p><%

		WriteButton (BID_CLEARMESSAGES);
	}

Cleanup:

	if (pstrSpeakerInRoomName != NULL) {
		g_pChatroom->FreeSpeakers (pstrSpeakerInRoomName);
	}

	if (pstrMessage != NULL) {
		g_pChatroom->FreeMessages (pstrMessage, pstrSpeakerName, ptTime);
	}
}

SYSTEM_CLOSE

%>