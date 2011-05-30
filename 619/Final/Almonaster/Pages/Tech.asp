<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include <stdio.h>

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

IHttpForm* pHttpForm;

int iErrCode;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

	// Always submit tech dev requests, regardless of updates
	int iTechKey;
	const char* pszStart;
	if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Tech")) != NULL && 
		(pszStart = pHttpForm->GetName()) != NULL &&
		sscanf (pszStart, "Tech%d", &iTechKey) == 1) {

		bRedirectTest = false;

		iErrCode = g_pGameEngine->RegisterNewTechDevelopment (m_iGameClass, m_iGameNumber, m_iEmpireKey, iTechKey);
		if (iErrCode == OK) {
			AddMessage ("You have developed ");
		} else {
			AddMessage ("You could not develop ");
		}
		AppendMessage (SHIP_TYPE_STRING[iTechKey]);
		AppendMessage (" technology");
	}
}

GAME_REDIRECT_ON_SUBMIT

GAME_OPEN

// Individual page stuff starts here
if (!(m_iGameState & STARTED)) {
	%><p>You cannot develop new technologies before the game begins<%
} else {

	int iBR;
	float fMaintRatio;

	WriteRatiosString (&iBR, &fMaintRatio);

	%><p><%
	WriteSeparatorString (m_iSeparatorKey);

	int iTechDevs, iTechUndevs;

	GameCheck (g_pGameEngine->GetDevelopedTechs (
		m_iGameClass,
		m_iGameNumber,
		m_iEmpireKey,
		&iTechDevs,
		&iTechUndevs
		));

	int iNumAvailableTechs;
	GameCheck (g_pGameEngine->GetNumAvailableTechs (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumAvailableTechs));

	int i, iNumDevKeys = 0, iNumUndevKeys = 0, piDevKey[NUM_SHIP_TYPES], piUndevKey[NUM_SHIP_TYPES];

	ENUMERATE_TECHS(i) {

		if (iTechDevs & TECH_BITS[i]) {
			piDevKey[iNumDevKeys ++] = i;
		}
		else if (iTechUndevs & TECH_BITS[i]) {
			piUndevKey[iNumUndevKeys ++] = i;
		}
	}

	if (iNumAvailableTechs > 0 && iNumUndevKeys > 0) {
		%><p>You can develop <font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><strong><% 
		Write (iNumAvailableTechs);
		%></strong></font> new<%
		if (iNumAvailableTechs == 1) {
			%> technology:<%
		} else {
			%> technologies:<% 
		}
	} else {
		%><p>You cannot develop any new technologies<%
	}

	%><p><table width="60%" cellpadding="4"><tr><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); 
	%>">Developed technologies:</th><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); 
	%>">Undeveloped technologies:</th></tr><%

	int iMax = max (iNumDevKeys, iNumUndevKeys);
	bool bWritten = false;

	char pszTech[64];

	for (i = 0; i < iMax; i ++) {

		%><tr><%

		if (i >= iNumDevKeys) {
			%><td></td><%
		} else {
			%><td align="center"><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><strong><%
			Write (SHIP_TYPE_STRING[piDevKey[i]]); %></strong></font></td><%
		}

		if (!bWritten && iNumUndevKeys == 0) {
			bWritten = true;
			%><td align="center"><strong>None</strong></td><%
		} else {

			if (i >= iNumUndevKeys) {
				%><td></td><%
			} else {

				if (iNumAvailableTechs > 0) {
					%><td align="center"><%

					sprintf (pszTech, "Tech%i", piUndevKey[i]);

					WriteButtonString (
						m_iButtonKey,
						SHIP_TYPE_STRING[piUndevKey[i]],
						SHIP_TYPE_STRING[piUndevKey[i]],
						pszTech
						);
					%></td><%
				} else {
					%><td align="center"><%
					%><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><strong><%
					Write (SHIP_TYPE_STRING[piUndevKey[i]]); %></strong></font></td><%
				}
			}
		} %></tr><%
	}

	%></table><%
}

GAME_CLOSE

%>