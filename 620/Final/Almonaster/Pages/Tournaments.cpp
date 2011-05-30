
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Tournaments page
int HtmlRenderer::Render_Tournaments() {

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

	IHttpForm* pHttpForm;

	int iErrCode, iTournamentPage = 0;
	unsigned int iTournamentKey = m_iReserved;

	if (iTournamentKey != NO_KEY) {
	    iTournamentPage = 1;
	}

	if (m_bOwnPost && !m_bRedirection) {

	    const char* pszStart;

	    if (WasButtonPressed (BID_CANCEL)) {
	        bRedirectTest = false;
	        goto Redirection;
	    }

	    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentsPage")) == NULL) {
	        goto Redirection;
	    }
	    int iTournamentsPageSubmit = pHttpForm->GetIntValue();

	    //
	    switch (iTournamentsPageSubmit) {

	    case 0:

	        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewTourneyInfo")) != NULL && 
	            (pszStart = pHttpForm->GetName()) != NULL &&
	            sscanf (pszStart, "ViewTourneyInfo%d", &iTournamentKey) == 1) {

	            bRedirectTest = false;
	            iTournamentPage = 1;
	            goto Redirection;
	        }

	        break;

	    case 1:

	        if (WasButtonPressed (BID_JOIN)) {

	            if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentKey")) == NULL) {
	                goto Redirection;
	            }

	            iTournamentKey = pHttpForm->GetIntValue();

	            iErrCode = g_pGameEngine->InviteSelfIntoTournament (iTournamentKey, m_iEmpireKey);
	            if (iErrCode == OK) {
	                AddMessage ("A request to join the tournament was sent to the tournament owner");
	            } else if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT) {
	                AddMessage ("You are already in the tournament");
	            } else if (iErrCode == ERROR_TOURNAMENT_DOES_NOT_EXIST) {
	                AddMessage ("That tournament no longer exists");
	            } else {
	                AddMessage ("Error ");
	                AppendMessage (iErrCode);
	                AppendMessage (" occurred");
	            }

	            bRedirectTest = false;
	        }

	        if (WasButtonPressed (BID_QUIT)) {

	            if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentKey")) == NULL) {
	                goto Redirection;
	            }

	            iTournamentKey = pHttpForm->GetIntValue();

	            iErrCode = g_pGameEngine->DeleteEmpireFromTournament (iTournamentKey, m_iEmpireKey);
	            if (iErrCode == OK) {
	                AddMessage ("Your empire was deleted from the tournament");
	            } else if (iErrCode == ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT) {
	                AddMessage ("Your empire was no longer in the tournament");
	            } else if (iErrCode == ERROR_EMPIRE_IS_IN_GAMES) {
	                AddMessage ("Your empire could not be deleted from the tournament because it is still in a tournament game");
	            } else {
	                AddMessage ("Your empire could not be deleted from the tournament. The error was ");
	                AppendMessage (iErrCode);
	            }

	            bRedirectTest = false;
	        }

	        break;

	    default:
	        break;
	    }
	}

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)

	// Individual page stuff starts here
	unsigned int* piTournamentKey = NULL, iTournaments = 0;

	switch (iTournamentPage) {

	case 0:

	    
	Write ("<input type=\"hidden\" name=\"TournamentsPage\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"TournamentsPage\" value=\"0\">") - 1);
	// List all system tournaments
	    iErrCode = g_pGameEngine->GetOwnedTournaments (SYSTEM, &piTournamentKey, NULL, &iTournaments);
	    if (iErrCode != OK) {
	        
	Write ("<p>Error ", sizeof ("<p>Error ") - 1);
	Write (iErrCode); 
	Write (" occurred", sizeof (" occurred") - 1);
	}

	    if (iTournaments == 0) {
	        
	Write ("<p><h3>There are no system tournaments</h3>", sizeof ("<p><h3>There are no system tournaments</h3>") - 1);
	}

	    else {

	        
	Write ("<p>There ", sizeof ("<p>There ") - 1);
	Write (iTournaments == 1 ? "is" : "are"); 
	Write (" <strong>", sizeof (" <strong>") - 1);
	Write (iTournaments); 
	Write ("</strong> system tournament", sizeof ("</strong> system tournament") - 1);
	if (iTournaments != 1) {
	            
	Write ("s", sizeof ("s") - 1);
	}
	        
	Write (":</h3>", sizeof (":</h3>") - 1);
	RenderTournaments (piTournamentKey, iTournaments, true);
	    }

	    break;

	case 1:

	    
	Write ("<input type=\"hidden\" name=\"TournamentsPage\" value=\"1\"><input type=\"hidden\" name=\"TournamentKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"TournamentsPage\" value=\"1\"><input type=\"hidden\" name=\"TournamentKey\" value=\"") - 1);
	Write (iTournamentKey); 
	Write ("\">", sizeof ("\">") - 1);
	RenderTournamentDetailed (iTournamentKey);

	    break;

	default:

	    Assert (false);
	    break;
	}


	// Cleanup

	if (piTournamentKey != NULL) {
	    g_pGameEngine->FreeKeys (piTournamentKey);
	}

	SYSTEM_CLOSE


}