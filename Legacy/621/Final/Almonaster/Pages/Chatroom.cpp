
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include "../Chatroom/CChatroom.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Chatroom page
int HtmlRenderer::Render_Chatroom() {

	// Almonaster
	// Copyright (c) 1998-2004 Max Attar Feingold (maf6@cornell.edu)
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
	Chatroom* pChatroom = g_pGameEngine->GetChatroom();

	// Make sure we have broadcast rights
	int iErrCode;
	bool bBroadcast = m_iPrivilege >= NOVICE;

	if (bBroadcast) {
	    bBroadcast = (m_iSystemOptions & CAN_BROADCAST) != 0;
	}

	if (m_bOwnPost && !m_bRedirection) {

	    // Check for refresh
	    if (WasButtonPressed (BID_REFRESHMESSAGES)) {
	        bRedirectTest = false;
	    } else {

	        if (m_iPrivilege >= ADMINISTRATOR) {

	            if (WasButtonPressed (BID_CLEARMESSAGES)) {

	                if (pChatroom->ClearMessages() == OK) {
	                    AddMessage ("The chatroom's messages were cleared");
	                } else {
	                    AddMessage ("The chatroom's messages could not be cleared");
	                }
	            }
	        }

	        if (bBroadcast) {

	            // Check for leave
	            if (WasButtonPressed (BID_LEAVETHECHATROOM)) {
	                pChatroom->ExitChatroom (m_vEmpireName.GetCharPtr());
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

	                iInChatroom = pChatroom->EnterChatroom (m_vEmpireName.GetCharPtr()) == OK ? CHATROOM_IN : CHATROOM_OUT;

	                if (iInChatroom == CHATROOM_IN) {

	                    String strFilter;
	                    iErrCode = HTMLFilter (pszMessage, &strFilter, MAX_NUM_SPACELESS_CHARS, false);
	                    if (iErrCode == OK) {
	                        pChatroom->PostMessage (m_vEmpireName.GetCharPtr(), strFilter, 0);
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
	    iInChatroom = pChatroom->EnterChatroom (m_vEmpireName.GetCharPtr()) == OK ? CHATROOM_IN : CHATROOM_OUT;
	}

	// Chatroom
	if (iInChatroom == CHATROOM_OUT) { 
	    
	Write ("<p><strong>There are too many empires in the Chatroom. Please try back later.</strong>", sizeof ("<p><strong>There are too many empires in the Chatroom. Please try back later.</strong>") - 1);
	} else {

	    ChatroomSpeaker* pcsSpeaker = NULL;
	    ChatroomMessage* pcmMessage = NULL;

	    char pszDate [OS::MaxDateLength];

	    unsigned int iNumSpeakers, iNumMessages;

	    Assert (!bBroadcast || iInChatroom == CHATROOM_IN);

	    // Get speaket list
	    iErrCode = pChatroom->GetSpeakers (&pcsSpeaker, &iNumSpeakers);
	    if (iErrCode != OK) {
	        
	Write ("<p>An error occurred reading the speaker list from the chatroom", sizeof ("<p>An error occurred reading the speaker list from the chatroom") - 1);
	goto Cleanup;
	    }

	    iErrCode = pChatroom->GetMessages (&pcmMessage, &iNumMessages);
	    if (iErrCode != OK) {
	        
	Write ("<p>An error occurred reading the message list from the chatroom", sizeof ("<p>An error occurred reading the message list from the chatroom") - 1);
	goto Cleanup;
	    }

	    if (!bBroadcast) {
	        
	Write ("<p><strong>You do not have the right to broadcast messages</strong>", sizeof ("<p><strong>You do not have the right to broadcast messages</strong>") - 1);
	}

	    
	Write ("<p><table border=\"3\" cellspacing=\"0\" cellpadding=\"0\" width=\"90%\"><tr><td bgcolor=\"#331111\"><table cellspacing=\"0\" cellpadding=\"2\" width=\"100%\"><tr><th align=\"left\" valign=\"top\" bgcolor=\"", sizeof ("<p><table border=\"3\" cellspacing=\"0\" cellpadding=\"0\" width=\"90%\"><tr><td bgcolor=\"#331111\"><table cellspacing=\"0\" cellpadding=\"2\" width=\"100%\"><tr><th align=\"left\" valign=\"top\" bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\"><strong>Time</strong></th><th align=\"left\" valign=\"top\" bgcolor=\"", sizeof ("\"><strong>Time</strong></th><th align=\"left\" valign=\"top\" bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\"><strong>Message</strong></th></tr>", sizeof ("\"><strong>Message</strong></th></tr>") - 1);
	if (iNumMessages == 0) {

	        
	Write ("<tr><td colspan=\"2\">&nbsp;</td></tr>", sizeof ("<tr><td colspan=\"2\">&nbsp;</td></tr>") - 1);
	} else {

	        for (i = 0; i < iNumMessages; i ++) {

	            String& strMessageText = pcmMessage[i].strMessageText;
	            String& strSpeaker = pcmMessage[i].strSpeaker;

	            UTCTime tTime = pcmMessage[i].tTime;
	            int iFlags = pcmMessage[i].iFlags;

	            
	Write ("<tr><td align=\"left\" width=\"25%\" bgcolor=\"#331111\"><font color=\"#ffff00\">", sizeof ("<tr><td align=\"left\" width=\"25%\" bgcolor=\"#331111\"><font color=\"#ffff00\">") - 1);
	iErrCode = Time::GetDateString (tTime, pszDate);
	            if (iErrCode != OK) {
	                
	Write ("Time Error", sizeof ("Time Error") - 1);
	} else {
	                Write (pszDate);
	            }
	            
	Write ("</font></td><td align=\"left\" width=\"55%\" bgcolor=\"#331111\"><strong>", sizeof ("</font></td><td align=\"left\" width=\"55%\" bgcolor=\"#331111\"><strong>") - 1);
	if (iFlags & CHATROOM_MESSAGE_SYSTEM) {

	                
	Write ("<font color=\"#ffffff\">&lt;", sizeof ("<font color=\"#ffffff\">&lt;") - 1);
	Write (SYSTEM_MESSAGE_SENDER);
	                
	Write ("&gt;</font></strong><font color=\"#ffffff\">", sizeof ("&gt;</font></strong><font color=\"#ffffff\">") - 1);
	} else {

	                
	Write ("<font color=\"#ffff00\">&lt;", sizeof ("<font color=\"#ffff00\">&lt;") - 1);
	Write (strSpeaker.GetCharPtr(), strSpeaker.GetLength());
	                
	Write ("&gt;</font></strong><font color=\"#ffffaa\">", sizeof ("&gt;</font></strong><font color=\"#ffffaa\">") - 1);
	}

	            Write (strMessageText.GetCharPtr(), strMessageText.GetLength());

	            
	Write ("</font></td></tr>", sizeof ("</font></td></tr>") - 1);
	}
	    }

	    
	Write ("</table></td><td bgcolor=\"#331111\" valign=\"top\" width=\"20%\"><table cellspacing=\"0\" cellpadding=\"2\" width=\"100%\"><tr><th align=\"center\" valign=\"top\" width=\"3%\" bgcolor=\"", sizeof ("</table></td><td bgcolor=\"#331111\" valign=\"top\" width=\"20%\"><table cellspacing=\"0\" cellpadding=\"2\" width=\"100%\"><tr><th align=\"center\" valign=\"top\" width=\"3%\" bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	    
	Write ("\"><strong>Speakers</strong></th></tr><tr><td valign=\"top\" bgcolor=\"#331111\">", sizeof ("\"><strong>Speakers</strong></th></tr><tr><td valign=\"top\" bgcolor=\"#331111\">") - 1);
	for (i = 0; i < iNumSpeakers; i ++) {
	        Write (pcsSpeaker[i].strName.GetCharPtr());
	        
	Write ("<br>", sizeof ("<br>") - 1);
	}

	    
	Write ("</tr></table></td></tr></table>", sizeof ("</tr></table></td></tr></table>") - 1);
	if (bBroadcast) {

	        
	Write ("<p><input type=\"text\" size=\"75%\" name=\"Message\"> ", sizeof ("<p><input type=\"text\" size=\"75%\" name=\"Message\"> ") - 1);
	WriteButton (BID_SPEAK);

	        
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_LEAVETHECHATROOM);
	        WriteButton (BID_REFRESHMESSAGES);
	    }

	    if (m_iPrivilege >= ADMINISTRATOR) {

	        
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CLEARMESSAGES);
	    }

	Cleanup:

	    if (pcsSpeaker != NULL) {
	        pChatroom->FreeSpeakers (pcsSpeaker);
	    }

	    if (pcmMessage != NULL) {
	        pChatroom->FreeMessages (pcmMessage);
	    }
	}

	SYSTEM_CLOSE


}