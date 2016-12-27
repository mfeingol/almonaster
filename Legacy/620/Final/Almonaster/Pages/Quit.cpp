
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Quit page
int HtmlRenderer::Render_Quit() {

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

	INITIALIZE_GAME

	int iErrCode;
	bool bConfirm = (m_iSystemOptions & CONFIRM_IMPORTANT_CHOICES) != 0;

	if ((m_bOwnPost && !m_bRedirection) || !bConfirm) {

	    PageId pageRedirect = OPTIONS;

	    if (WasButtonPressed (BID_CANCEL)) {

	        // Cancelled - redirect to options
	        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
	        m_pgeLock = NULL;
	        return Redirect (OPTIONS);
	    }

	    else if (WasButtonPressed (BID_RESIGN)) {

	        // Upgrade to write lock
	        GameEmpireLock* pgeLock = m_pgeLock;
	        m_pgeLock = NULL;

	        if (g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, pgeLock) != OK || 
	            g_pGameEngine->WaitGameWriter (m_iGameClass, m_iGameNumber) != OK) {

	            AddMessage ("That game no longer exists");
	            pageRedirect = ACTIVE_GAME_LIST;

	        } else {

	            if (!(m_iGameState & STARTED)) {
	                AddMessage ("You cannot resign until the game starts");
	            } else {

	                // Resign
	                iErrCode = g_pGameEngine->ResignEmpireFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey);
	                if (iErrCode != OK) {
	                    AddMessage ("You could not resign from the game; the error was ");
	                    AppendMessage (iErrCode);
	                } else {

	                    pageRedirect = ACTIVE_GAME_LIST;
	                    AddMessage ("You resigned from ");
	                    AppendMessage (m_pszGameClassName);
	                    AppendMessage (" ");
	                    AppendMessage (m_iGameNumber);

	                    // Add to report
	                    SystemConfiguration scConfig;
	                    if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {
	                        
	                        char pszReport [MAX_EMPIRE_NAME_LENGTH + MAX_GAME_CLASS_NAME_LENGTH + 128];
	                        sprintf (pszReport, "%s resigned from %s %i", 
	                            m_vEmpireName.GetCharPtr(), m_pszGameClassName, m_iGameNumber);
	                        g_pReport->WriteReport (pszReport);
	                    }

	                    // Make sure we still exist after quitting
	                    bool bFlag;
	                    iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bFlag, NULL);
	                    if (iErrCode != OK || !bFlag) {
	                        pageRedirect = LOGIN;
	                        AddMessage ("The empire ");
	                        AppendMessage (m_vEmpireName.GetCharPtr());
	                        AppendMessage ("has been deleted");
	                    }
	                }
	            }

	            // Release write lock we took above
	            g_pGameEngine->SignalGameWriter(m_iGameClass, m_iGameNumber);

	            // Check game for updates - redirect will handle error
	            bool bFlag;
	            iErrCode = g_pGameEngine->CheckGameForUpdates (m_iGameClass, m_iGameNumber, true, &bFlag);
	        }

	        return Redirect (pageRedirect);
	    }

	    else if (WasButtonPressed (BID_QUIT)) {

	        // Upgrade to write lock
	        GameEmpireLock* pgeLock = m_pgeLock;
	        m_pgeLock = NULL;

	        if (g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, pgeLock) != OK || 
	            g_pGameEngine->WaitGameWriter (m_iGameClass, m_iGameNumber) != OK) {

	            AddMessage ("That game no longer exists");
	            pageRedirect = ACTIVE_GAME_LIST;

	        } else {

	            if (m_iGameState & STARTED) {
	                AddMessage ("You cannot quit because the game has started");
	            } else {

	                // Quit
	                iErrCode = g_pGameEngine->QuitEmpireFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey);
	                if (iErrCode != OK) {
	                    AddMessage ("You could not quit from the game; the error was ");
	                    AppendMessage (iErrCode);
	                } else {

	                    pageRedirect = ACTIVE_GAME_LIST;
	                    AddMessage ("You quit from ");
	                    AppendMessage (m_pszGameClassName);
	                    AppendMessage (" ");
	                    AppendMessage (m_iGameNumber);

	                    // Add to report
	                    SystemConfiguration scConfig;
	                    if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {
	                        
	                        char pszReport [MAX_EMPIRE_NAME_LENGTH + MAX_GAME_CLASS_NAME_LENGTH + 128];
	                        sprintf (pszReport, "%s quit from %s %i", 
	                            m_vEmpireName.GetCharPtr(), m_pszGameClassName, m_iGameNumber);
	                        g_pReport->WriteReport (pszReport);
	                    }

	                    // Make sure we still exist after quitting
	                    bool bFlag;
	                    iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bFlag, NULL);

	                    if (iErrCode != OK || !bFlag) {
	                        pageRedirect = LOGIN;
	                        AddMessage ("The empire ");
	                        AppendMessage (m_vEmpireName.GetCharPtr());
	                        AppendMessage (" has been deleted");
	                    }
	                }
	            }

	            // Release write lock we took above
	            g_pGameEngine->SignalGameWriter(m_iGameClass, m_iGameNumber);
	        }

	        return Redirect (pageRedirect);
	    }

	    else if (WasButtonPressed (BID_SURRENDER)) {

	        // Make sure this is allowed
	        SurrenderType sType = SC30_SURRENDER;

	        int iOptions;
	        iErrCode = g_pGameEngine->GetGameClassOptions (m_iGameClass, &iOptions);
	        if (iErrCode != OK || !(iOptions & USE_SC30_SURRENDERS)) {

	            // See if two empires are left - otherwise, we've been lied to
	            int iNumEmpires;
	            iErrCode = g_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires);
	            if (iErrCode != OK || iNumEmpires != 2) {

	                g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
	                m_pgeLock = NULL;
	                return Redirect (OPTIONS);
	            }

	            sType = NORMAL_SURRENDER;
	        }

	        // Upgrade to write lock
	        GameEmpireLock* pgeLock = m_pgeLock;
	        m_pgeLock = NULL;

	        if (g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, pgeLock) != OK || 
	            g_pGameEngine->WaitGameWriter (m_iGameClass, m_iGameNumber) != OK) {

	            AddMessage ("That game no longer exists");
	            pageRedirect = ACTIVE_GAME_LIST;

	        } else {

	            if (m_iGameState & STILL_OPEN) {
	                AddMessage ("You cannot surrender until the game has closed");
	            } else {

	                // Surrender
	                iErrCode = g_pGameEngine->SurrenderEmpireFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, sType);
	                if (iErrCode != OK) {
	                    AddMessage ("You could not surrender from the game; the error was ");
	                    AppendMessage (iErrCode);
	                } else {

	                    pageRedirect = ACTIVE_GAME_LIST;
	                    AddMessage ("You surrendered from ");
	                    AppendMessage (m_pszGameClassName);
	                    AppendMessage (" ");
	                    AppendMessage (m_iGameNumber);

	                    // Make sure we still exist after surrendering
	                    bool bFlag;
	                    iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bFlag, NULL);

	                    if (iErrCode != OK || !bFlag) {
	                        pageRedirect = LOGIN;
	                        AddMessage ("The empire ");
	                        AppendMessage (m_vEmpireName.GetCharPtr());
	                        AppendMessage (" has been deleted");
	                    }
	                }
	            }

	            // Release write lock we took above
	            g_pGameEngine->SignalGameWriter(m_iGameClass, m_iGameNumber);
	        }

	        return Redirect (pageRedirect);
	    }

	    else {

	        // Redirect to quit or info
	        if (WasButtonPressed (BID_QUIT)) {
	            m_iReserved = BID_QUIT;
	        }
	    }
	}

	GAME_REDIRECT_ON_SUBMIT

	GAME_OPEN

	// Individual page starts here

	// Determine state
	switch (m_iReserved) {

	case BID_RESIGN:

	    
	Write ("<p>Are you sure that you want to resign from <strong>", sizeof ("<p>Are you sure that you want to resign from <strong>") - 1);
	Write (m_pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (m_iGameNumber); 
	Write ("</strong>?<br>You will not be able to resume the game and your empire will fall into ruin or be nuked.<p>", sizeof ("</strong>?<br>You will not be able to resume the game and your empire will fall into ruin or be nuked.<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_RESIGN);

	    break;

	case BID_QUIT:

	    
	Write ("<p>Are you sure that you want to quit from <strong>", sizeof ("<p>Are you sure that you want to quit from <strong>") - 1);
	Write (m_pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (m_iGameNumber); 
	Write ("</strong>?<p>", sizeof ("</strong>?<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_QUIT);

	    break;

	case BID_SURRENDER:

	    
	Write ("<p>Are you sure that you want to surrender from <strong>", sizeof ("<p>Are you sure that you want to surrender from <strong>") - 1);
	Write (m_pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (m_iGameNumber); 
	Write ("</strong>?<p>", sizeof ("</strong>?<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_SURRENDER);

	    break;

	default:

	    break;
	}

	GAME_CLOSE


}