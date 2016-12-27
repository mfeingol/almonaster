<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

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

bool bConfirm;
iErrCode = g_pGameEngine->GetEmpireOption (m_iEmpireKey, CONFIRM_ON_ENTER_OR_QUIT_GAME, &bConfirm);

if (iErrCode != OK) {
	Assert (false);
	bConfirm = true;
}

if ((m_bOwnPost && !m_bRedirection) || !bConfirm) {

	PageId pageRedirect = INFO;

	if (WasButtonPressed (BID_CANCEL)) {

		// Cancelled - redirect to info
		g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
		return Redirect (INFO);
	}

	else if (WasButtonPressed (BID_RESIGN)) {

		// Upgrade to write lock
		if (g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber) != OK || 
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

					// Make sure we still exist after quitting
					bool bFlag;
					iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bFlag);

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
			iErrCode = g_pGameEngine->CheckGameForUpdates (m_iGameClass, m_iGameNumber, &bFlag);
		}

		return Redirect (pageRedirect);
	}

	else if (WasButtonPressed (BID_QUIT)) {

		// Upgrade to write lock
		if (g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber) != OK || 
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

					// Make sure we still exist after quitting
					bool bFlag;
					iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bFlag);

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
		int iOptions;
		iErrCode = g_pGameEngine->GetGameClassOptions (m_iGameClass, &iOptions);

		if (iErrCode != OK || !(iOptions & USE_SC30_SURRENDERS)) {
			g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
			return Redirect (OPTIONS);
		}

		// Upgrade to write lock
		if (g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber) != OK || 
			g_pGameEngine->WaitGameWriter (m_iGameClass, m_iGameNumber) != OK) {

			AddMessage ("That game no longer exists");
			pageRedirect = ACTIVE_GAME_LIST;

		} else {

			if (!(m_iGameState & STARTED)) {
				AddMessage ("You cannot surrender until the game starts");
			} else {

				// Surrender
				iErrCode = g_pGameEngine->SurrenderEmpireFromGame30Style (m_iGameClass, m_iGameNumber, m_iEmpireKey);
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
					iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bFlag);

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

	%><p>Are you sure that you want to resign from <strong><%
	Write (m_pszGameClassName); %> <% Write (m_iGameNumber); %></strong>?<%
	%><br>You will not be able to resume the game and your empire will fall into ruin or be nuked.<p><%

	WriteButton (BID_CANCEL);
	WriteButton (BID_RESIGN);

	break;

case BID_QUIT:

	%><p>Are you sure that you want to quit from <strong><%
	Write (m_pszGameClassName); %> <% Write (m_iGameNumber); %></strong>?<p><%

	WriteButton (BID_CANCEL);
	WriteButton (BID_QUIT);

	break;

case BID_SURRENDER:

	%><p>Are you sure that you want to surrender from <strong><%
	Write (m_pszGameClassName); %> <% Write (m_iGameNumber); %></strong>?<p><%

	WriteButton (BID_CANCEL);
	WriteButton (BID_SURRENDER);

	break;

default:

	break;
}

GAME_CLOSE

%>