<% #include "Osal/Algorithm.h"
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include "Database.h"

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

bool bGameStarted = (m_iGameState & STARTED) != 0;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

	// Make sure the game has started and cancel wasn't pressed
	if (bGameStarted && !WasButtonPressed (BID_CANCEL)) {

		iErrCode = HandleShipMenuSubmissions();
		if (iErrCode != OK) {
			AddMessage ("Error handling ship menu submissions");
		}
	}

	// Handle cancel all builds
	if (WasButtonPressed (BID_CANCELALLBUILDS)) {
		iErrCode = g_pGameEngine->CancelAllBuilds (m_iGameClass, m_iGameNumber, m_iEmpireKey);
		bRedirectTest = false;
	}
}

GAME_REDIRECT_ON_SUBMIT

GAME_OPEN

// Individual page stuff starts here
if (!bGameStarted) {
	%><p>You cannot use ships before the game begins<%
} else {

	IDatabase* pDatabase = g_pGameEngine->GetDatabase();
	IReadTable* pShips = NULL, * pFleets = NULL;

	int iBR;
	float fMaintRatio;

	WriteRatiosString (&iBR, &fMaintRatio);

	%><p><%
	WriteSeparatorString (m_iSeparatorKey);

	GAME_EMPIRE_SHIPS (pszGameEmpireShips, m_iGameClass, m_iGameNumber, m_iEmpireKey);
	GAME_EMPIRE_FLEETS (pszGameEmpireFleets, m_iGameClass, m_iGameNumber, m_iEmpireKey);

	iErrCode = pDatabase->GetTableForReading (pszGameEmpireShips, &pShips);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pDatabase->GetTableForReading (pszGameEmpireFleets, &pFleets);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Render ships
	RenderShips (
		pShips,
		pFleets,
		iBR,
		fMaintRatio,
		NULL
		);

Cleanup:

	if (pShips != NULL) {
		pShips->Release();
	}

	if (pFleets != NULL) {
		pFleets->Release();
	}

	pDatabase->Release();

	int iNumBuilds;
	GameCheck (g_pGameEngine->GetNumBuilds (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumBuilds));

	if (iNumBuilds > 0) {
		%><p><%
		WriteButton (BID_CANCELALLBUILDS);
	}
}

GAME_CLOSE

%>