
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the SystemTos page
int HtmlRenderer::Render_SystemTos() {

	// Almonaster
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

	int iErrCode = OK;
	int iTosPage = 0;

	IHttpForm* pHttpForm;

	INITIALIZE_EMPIRE

	if (m_bOwnPost && !m_bRedirection) {

	    if ((pHttpForm = m_pHttpRequest->GetForm ("TosPage")) == NULL) {
	        goto Redirection;
	    }
	    int iTosPageSubmit = pHttpForm->GetIntValue();

	    switch (iTosPageSubmit) {
	    case 0:

	        if (WasButtonPressed (BID_TOS_ACCEPT)) {

	            EmpireCheck (g_pGameEngine->SetEmpireOption2 (m_iEmpireKey, EMPIRE_ACCEPTED_TOS, true));
	            m_iSystemOptions2 |= EMPIRE_ACCEPTED_TOS;

	            AddMessage ("You accepted the Terms of Service");

	            SystemConfiguration scConfig;
		        if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {
		            char pszText [MAX_EMPIRE_NAME_LENGTH + 128];
		            sprintf (pszText, "%s accepted the Terms of Service", m_vEmpireName.GetCharPtr());
	                g_pReport->WriteReport (pszText);
		        }

	            return Redirect (ACTIVE_GAME_LIST);
	        }

	        if (WasButtonPressed (BID_TOS_DECLINE)) {
	            iTosPage = 1;
	            bRedirectTest = false;

	            SystemConfiguration scConfig;
		        if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {
		            char pszText [MAX_EMPIRE_NAME_LENGTH + 128];
		            sprintf (pszText, "%s declined the Terms of Service", m_vEmpireName.GetCharPtr());
	                g_pReport->WriteReport (pszText);
		        }
	        }
	        break;

	    case 1:

	        if (WasButtonPressed (BID_TOS_DECLINE)) {
	            
	            // Best effort
	            g_pGameEngine->DeleteEmpire (m_iEmpireKey, NULL, true, false);
	            return Redirect (LOGIN);
	        }

	        break;

	    default:
	        break;
	    }
	}

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)


	Write ("<input type=\"hidden\" name=\"TosPage\" value=\"", sizeof ("<input type=\"hidden\" name=\"TosPage\" value=\"") - 1);
	Write (iTosPage); 
	Write ("\">", sizeof ("\">") - 1);
	// Individual page stuff starts here
	switch (iTosPage) {

	case 0:

	    WriteTOS();
	    break;

	case 1:

	    WriteConfirmTOSDecline();
	    break;
	}

	SYSTEM_CLOSE


}