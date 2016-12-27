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

int i, iErrCode;

bool bGameStarted = (m_iGameState & STARTED) != 0;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

	if (bGameStarted) {

		iErrCode = HandleShipMenuSubmissions();
		if (iErrCode != OK) {
			AddMessage ("Error handling ship menu submissions");
			goto Redirection;
		}

		// Handle planet name change or maxpop change submissions
		int iNumTestPlanets;
		if ((pHttpForm = m_pHttpRequest->GetForm ("NumOurPlanets")) == NULL) {
			GameCheck (g_pGameEngine->GetNumVisitedPlanets (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumTestPlanets));
		} else {
			iNumTestPlanets = pHttpForm->GetIntValue();
		}

		const char* pszOldPlanetName, * pszNewPlanetName;
		int iOldMaxPop, iNewMaxPop, iUpdatePlanetKey;

		char pszForm [256];

		for (i = 0; i < iNumTestPlanets; i ++) {

			iUpdatePlanetKey = NO_KEY;

			// Get original name
			sprintf (pszForm, "OldPlanetName%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				goto Redirection;
			}
			pszOldPlanetName = pHttpForm->GetValue();

			// Get new name
			sprintf (pszForm, "NewPlanetName%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				goto Redirection;
			}
			pszNewPlanetName = pHttpForm->GetValue();

			if (String::StrCmp (pszOldPlanetName, pszNewPlanetName) != 0) {

				size_t stLength;

				if (pszNewPlanetName == NULL) {
					stLength = 0;
				} else {
					stLength = strlen (pszNewPlanetName);
				}

				if (stLength > MAX_PLANET_NAME_LENGTH) {
					AddMessage ("The submitted planet name was too long");
				} else {

					// Get planet key
					sprintf (pszForm, "KeyPlanet%i", i);
					if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
						goto Redirection;
					}
					iUpdatePlanetKey = pHttpForm->GetIntValue();

					// Best effort
					g_pGameEngine->RenamePlanet (
						m_iGameClass,
						m_iGameNumber,
						m_iEmpireKey,
						iUpdatePlanetKey, 
						pszNewPlanetName
						);
				}
			}

			// Get original MaxPop
			sprintf (pszForm, "OldMaxPop%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				goto Redirection;
			}
			iOldMaxPop = pHttpForm->GetIntValue();

			// Get new MaxPop
			sprintf (pszForm, "NewMaxPop%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				goto Redirection;
			}
			iNewMaxPop = pHttpForm->GetIntValue();

			if (iOldMaxPop != iNewMaxPop) {

				// Get planet key
				if (iUpdatePlanetKey == NO_KEY) {

					sprintf (pszForm, "KeyPlanet%i", i);

					if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
						goto Redirection;
					}
					iUpdatePlanetKey = pHttpForm->GetIntValue();
				}

				if (g_pGameEngine->SetPlanetMaxPop (
					m_iGameClass,
					m_iGameNumber,
					m_iEmpireKey,
					iUpdatePlanetKey, 
					iNewMaxPop
					) != OK
					) {
					AddMessage ("The planet's Max Pop could not be set");
				}
			}
		}
	}
}

GAME_REDIRECT_ON_SUBMIT

GAME_OPEN

// Individual page stuff starts here
int iNumPlanets, iLivePlanetKey, iDeadPlanetKey;

GameCheck (g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey));

if (bGameStarted) {

	Variant* pvPlanetKey;
	int* piProxyKey, iCounter = 0;
	bool bOurPlanet;
	GameCheck (g_pGameEngine->GetVisitedPlanetKeys (
		m_iGameClass,
		m_iGameNumber,
		m_iEmpireKey,
		&pvPlanetKey,
		&piProxyKey, 
		&iNumPlanets)
		);

	if (iNumPlanets > 0) {

		GAME_MAP (strGameMap, m_iGameClass, m_iGameNumber);
		Variant vOptions;

		int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;

		float fAgRatio;

		bool bShips = false;
		ShipsInMapScreen simShipsInMap = { NO_KEY, 0, 0 };

		IDatabase* pDatabase = g_pGameEngine->GetDatabase();
		IReadTable* pGameMap = NULL, * pShips = NULL, * pFleets = NULL;

		void** ppPlanetData = NULL;

		int iBR = 0;
		float fMaintRatio = 0;

		iErrCode = pDatabase->GetTableForReading (strGameMap, &pGameMap);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		// Visible builds?
		iErrCode = pDatabase->ReadData (SYSTEM_GAMECLASS_DATA, m_iGameClass, SystemGameClassData::Options, &vOptions);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = GetGoodBadResourceLimits (
			m_iGameClass,
			m_iGameNumber,
			&iGoodAg,
			&iBadAg,
			&iGoodMin, 
			&iBadMin,
			&iGoodFuel,
			&iBadFuel
			);

		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = g_pGameEngine->GetEmpireAgRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fAgRatio);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = g_pGameEngine->GetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIPS_ON_PLANETS_SCREEN, &bShips);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (bShips) {

			iErrCode = g_pGameEngine->GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			iErrCode = g_pGameEngine->GetEmpireMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fMaintRatio);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}

		for (i = 0; i < iNumPlanets; i ++) {

			iErrCode = pGameMap->ReadRow (pvPlanetKey[i].GetInteger(), &ppPlanetData);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			%><p><table width="90%"><%

			iErrCode = WriteUpClosePlanetString (m_iEmpireKey, pvPlanetKey[i].GetInteger(), 
				piProxyKey[i], iLivePlanetKey, iDeadPlanetKey, iCounter, 
				(vOptions.GetInteger() & VISIBLE_BUILDS) != 0, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, 
				iBadFuel, fAgRatio, (vOptions.GetInteger() & INDEPENDENCE) != 0, false, false,
				ppPlanetData, &bOurPlanet
				);

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			// Render ships
			if (bShips) {

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

				%><tr></tr><tr><td></td><td align="center" colspan="10"><%

				simShipsInMap.iPlanetKey = pvPlanetKey[i].GetInteger();

				// Render ships
				RenderShips (
					pShips,
					pFleets,
					iBR,
					fMaintRatio,
					&simShipsInMap
					);

				pShips->Release();
				pShips = NULL;

				pFleets->Release();
				pFleets = NULL;

				%></td></tr><%
			}

			%></table><%

			pDatabase->FreeData (ppPlanetData);
			ppPlanetData = NULL;

			if (bOurPlanet) {
				iCounter ++;
			}
		}

Cleanup:

		if (ppPlanetData != NULL) {
			pDatabase->FreeData (ppPlanetData);
		}

		if (pvPlanetKey != NULL) {
			g_pGameEngine->FreeData (pvPlanetKey);
		}

		if (piProxyKey != NULL) {
			g_pGameEngine->FreeKeys ((unsigned int*) piProxyKey);
		}

		if (pGameMap != NULL) {
			pGameMap->Release();
		}

		if (pShips != NULL) {
			pShips->Release();
		}

		if (pFleets != NULL) {
			pFleets->Release();
		}

		pDatabase->Release();

		if (iErrCode != OK) {
			%>Error rendering up-close planet view. The error was <% Write (iErrCode);
		}
		else if (bShips) {
			%><input type="hidden" name="NumShips" value="<% Write (simShipsInMap.iCurrentShip); %>"><%
			%><input type="hidden" name="NumFleets" value="<% Write (simShipsInMap.iCurrentFleet); %>"><%
		}
	}

	if (iErrCode == OK) {
		%><input type="hidden" name="NumOurPlanets" value="<% Write (iCounter); %>"><%
	}

} else {

	%><p><table width="90%"><%

	bool bTrue;
	iErrCode = WriteUpClosePlanetString (NO_KEY, NO_KEY, NO_KEY, 
		iLivePlanetKey, iDeadPlanetKey, 0, false, 0, 0, 0, 0, 0, 0, 0.0, false, false, false, NULL, &bTrue);

	if (iErrCode != OK) {
		%>Error rendering up-close planet view. The error was <% Write (iErrCode);
	}

	%></table><%
}

GAME_CLOSE

%>