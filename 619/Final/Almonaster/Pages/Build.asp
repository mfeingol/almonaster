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

int i, j, iErrCode;

// Handle a submission
Variant vPlanetName;

bool bGameStarted = (m_iGameState & STARTED) != 0;

if (m_bOwnPost && !m_bRedirection) {

	// Make sure cancel wasn't pressed
	// Discard submission if update counts don't match
	if (bGameStarted && !WasButtonPressed (BID_CANCEL) && m_iNumNewUpdates == m_iNumOldUpdates) {

		int iNumShipTypes, iNumShips, iTechKey, iShipBR, iLocationKey, iPlanetKey, iFleetKey;
		String strTechName;
		const char* pszShipName;

		// Get num techs
		if ((pHttpForm = m_pHttpRequest->GetForm ("NumTechs")) == NULL) {
			goto Redirection;
		}
		iNumShipTypes = pHttpForm->GetIntValue();

		bool bError = false, bBuilt = false;
		char pszForm [256];
		char pszMessage [2048];

		for (i = 0; i < iNumShipTypes; i ++) {

			sprintf (pszForm, "NumShips%i", i);

			// Num ships
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				goto Redirection;
			}
			iNumShips = pHttpForm->GetIntValue();

			if (iNumShips > 0 && iNumShips < 101) {

				// We're building ships, so get the tech key
				sprintf (pszForm, "TechKey%i", i);
				if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
					goto Redirection;
				}
				iTechKey = pHttpForm->GetIntValue();

				// Get the BR
				sprintf (pszForm, "ShipBR%i", i);
				if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
					goto Redirection;
				}
				iShipBR = pHttpForm->GetIntValue();

				// Get ship name
				sprintf (pszForm, "ShipName%i", i);
				if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
					goto Redirection;
				}
				pszShipName = pHttpForm->GetValue();
				if (pszShipName == NULL) {
					pszShipName = "";
				}

				if (!ShipOrFleetNameFilter (pszShipName)) {
					AddMessage ("Illegal ship name");
				} else {

					if (strlen (pszShipName) > MAX_SHIP_NAME_LENGTH) {
						AddMessage ("The ship name is too long");
					} else {

						// Get location key
						sprintf (pszForm, "ShipLocation%i", i);
						if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
							goto Redirection;
						}
						iLocationKey = pHttpForm->GetIntValue();

						// Get planet key
						sprintf (pszForm, "LocationPlanetKey%i", iLocationKey);
						if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
							goto Redirection;
						}
						iPlanetKey = pHttpForm->GetIntValue();

						// Get fleet key
						sprintf (pszForm, "LocationFleetKey%i", iLocationKey);
						if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
							goto Redirection;
						}
						iFleetKey = pHttpForm->GetIntValue();

						iErrCode = g_pGameEngine->BuildNewShips (m_iGameClass, m_iGameNumber, m_iEmpireKey, 
							iTechKey, iNumShips, pszShipName, (float) iShipBR, iPlanetKey, iFleetKey);

						switch (iErrCode) {

						case OK:
							{

							iErrCode = g_pGameEngine->GetPlanetName (
								m_iGameClass, 
								m_iGameNumber, 
								iPlanetKey, 
								&vPlanetName
								);

							if (iErrCode == OK) {

								sprintf (
									pszMessage,
									"%i BR %i %s ship%s built at %s",
									iNumShips,
									iShipBR,
									SHIP_TYPE_STRING[iTechKey],
									(iNumShips == 1) ? " was" : "s were",
									vPlanetName.GetCharPtr()
									);

								if (iFleetKey != NO_KEY) {

									iErrCode = g_pGameEngine->GetFleetName (
										m_iGameClass,
										m_iGameNumber,
										m_iEmpireKey,
										iFleetKey,
										&vPlanetName
										);

									strcat (pszMessage, " in fleet ");
									strcat (pszMessage, vPlanetName.GetCharPtr());
								}

								bBuilt = true;
								AddMessage (pszMessage);
							}

							}
							break;

						case ERROR_GAME_HAS_NOT_STARTED:

							AddMessage ("You cannot build before the game starts");
							bError = true;
							break;

						case ERROR_WRONG_OWNER:

							iErrCode = g_pGameEngine->GetPlanetName (
								m_iGameClass,
								m_iGameNumber,
								iPlanetKey,
								&vPlanetName
								);

							if (iErrCode == OK) {
								sprintf (pszMessage, "You no longer own planet %s", vPlanetName.GetCharPtr());
								AddMessage (pszMessage);
							}

							bError = true;
							break;

						case ERROR_INSUFFICIENT_POPULATION:

							iErrCode = g_pGameEngine->GetPlanetName (
								m_iGameClass, 
								m_iGameNumber, 
								iPlanetKey, 
								&vPlanetName
								);

							if (iErrCode == OK) {
								sprintf (pszMessage, "Planet %s lacks the population needed to build ships", vPlanetName.GetCharPtr());
								AddMessage (pszMessage);
							}

							bError = true;
							break;

						case ERROR_WRONG_TECHNOLOGY:

							if (iTechKey >= FIRST_SHIP && iTechKey <= LAST_SHIP) {
								sprintf (pszMessage, "You cannot build %s ships", SHIP_TYPE_STRING[iTechKey]);
								AddMessage (pszMessage);
							} else {
								AddMessage ("That technology does not exist");
							}

							bError = true;
							break;

						case ERROR_INSUFFICIENT_POPULATION_FOR_COLONIES:

							iErrCode = g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, iPlanetKey, &vPlanetName);
							if (iErrCode == OK) {

								sprintf (
									pszMessage, 
									"Planet %s lacks the population level needed to build %i %s",
									vPlanetName.GetCharPtr(),
									iNumShips,
									(iNumShips == 1) ? " colony" : " colonies"
									);

								AddMessage (pszMessage);
							}

							bError = true;
							break;

						case ERROR_INVALID_TECH_LEVEL:

							sprintf (pszMessage, "You cannot build BR %i ships", iShipBR);
							AddMessage (pszMessage);

							bError = true;
							break;

						case ERROR_WRONG_NUMBER_OF_SHIPS:

							AddMessage ("You cannot build that number of ships");
							bError = true;
							break;

						default:

							sprintf (pszMessage, "Error %i occurred while building ships", iErrCode);
							AddMessage (pszMessage);

							bError = true;
							break;
						}
					}
				}
			}
		}

		if (iNumShipTypes > 0) {

			if (!bError && bBuilt) {

				// Add maintenance ratio, tech development values
				float fMaintRatio, fFuelRatio, fTechLevel, fTechDev;
				int iBR;
				GameCheck (g_pGameEngine->GetShipRatios (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fMaintRatio, 
					&fFuelRatio, &fTechLevel, &fTechDev, &iBR));

				sprintf (pszMessage, "Your Maintenance Ratio is: %f", fMaintRatio);
				AddMessage (pszMessage);

				sprintf (pszMessage, "Your Tech Development is : %f", fTechDev);
				AddMessage (pszMessage);
			}
		}

		// Check for a fleet name
		if ((pHttpForm = m_pHttpRequest->GetForm ("NewFleetName")) == NULL) {
			goto Redirection;
		}

		const char* pszTemp;
		if ((pszTemp = pHttpForm->GetValue()) != NULL) {

			if (!ShipOrFleetNameFilter (pszTemp)) {
				AddMessage ("Illegal fleet name");
			} else {

				if (strlen (pszTemp) > MAX_FLEET_NAME_LENGTH) {
					AddMessage ("The new fleet's name is too long");
				} else {

					if ((pHttpForm = m_pHttpRequest->GetForm ("NewFleetLocation")) == NULL) {
						goto Redirection;
					}
					iPlanetKey = pHttpForm->GetIntValue();

					iErrCode = g_pGameEngine->CreateNewFleet (
						m_iGameClass,
						m_iGameNumber,
						m_iEmpireKey,
						pszTemp,
						iPlanetKey
						);

					switch (iErrCode) {
					case OK:

						sprintf (pszMessage, "Fleet %s was created", pszTemp);
						AddMessage (pszMessage);
						break;

					case ERROR_NAME_IS_IN_USE:

						sprintf (pszMessage, "Fleet %s already exists", pszTemp);
						AddMessage (pszMessage);
						break;

					case ERROR_EMPTY_NAME:

						AddMessage ("The fleet name was empty");
						break;

					case ERROR_ORPHANED_FLEET:

						{
						const char* pszPlanetName;

						iErrCode = g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, iPlanetKey, &vPlanetName);
						if (iErrCode == OK) {
							pszPlanetName = vPlanetName.GetCharPtr();
						} else {
							pszPlanetName = "Unknown";
						}

						sprintf (pszMessage, "You cannot create a fleet on the planet %s", pszPlanetName);
						AddMessage (pszMessage);

						}
						break;

					default:

						sprintf (pszMessage, "Error %i occurred while creating a fleet", iErrCode);
						AddMessage (pszMessage);

						bError = true;
						break;
					}
				}
			}
		}
	}
}

GAME_REDIRECT_ON_SUBMIT

GAME_OPEN

// Individual page stuff starts here
if (!bGameStarted) {
	%><input type="hidden" name="NumTechs" value="0"><%
	%><p>You cannot build ships or fleets before the game begins<%
} else {

	int iBR;
	float fMaintRatio;
	WriteRatiosString (&iBR, &fMaintRatio); %><p><% 
	WriteSeparatorString (m_iSeparatorKey);

	int* piBuilderKey, iNumBuilders;
	GameCheck (g_pGameEngine->GetBuilderPlanetKeys (m_iGameClass, m_iGameNumber, m_iEmpireKey, &piBuilderKey, 
		&iNumBuilders));

	if (iNumBuilders == 0) {
		%><p>You have no builder planets to build ships on<input type="hidden" name="NumTechs" value="0"><%
	} else {

		Algorithm::AutoDelete<int> autopiBuilderKey (piBuilderKey, true);

		if (iBR < 1) {
			%><p><strong>You lack the technology level to build ships</strong><input type="hidden" name="NumTechs" value="0"><%
		} else {

			// Get fleet locations
			int* piFleetKey, iNumFleets;
			GameCheck (g_pGameEngine->GetEmpireFleetKeys (
				m_iGameClass, 
				m_iGameNumber, 
				m_iEmpireKey, 
				&piFleetKey, 
				&iNumFleets
				));

			int** ppiFleetTable = (int**) StackAlloc (iNumBuilders * sizeof (int*));
			for (i = 0; i < iNumBuilders; i ++) { 
				ppiFleetTable[i] = (int*) StackAlloc ((iNumFleets + 1) * sizeof (int));
				ppiFleetTable[i][iNumFleets] = 0;
			}

			Variant* pvFleetName = NULL;
			Algorithm::AutoDelete<Variant> autopstrFleetName (pvFleetName, true);

			if (iNumFleets > 0) {

				pvFleetName = new Variant [iNumFleets];

				int iPlanetKey;
				for (i = 0; i < iNumFleets; i ++) {

					// Get fleet name
					if (g_pGameEngine->GetFleetName (m_iGameClass, m_iGameNumber, m_iEmpireKey, piFleetKey[i],
						&pvFleetName[i]) != OK ||
						g_pGameEngine->GetFleetLocation (m_iGameClass, m_iGameNumber, m_iEmpireKey, 
							piFleetKey[i], &iPlanetKey) != OK
						) {
						continue;
					}

					for (j = 0; j < iNumBuilders; j ++) {
						if (iPlanetKey == piBuilderKey[j]) {
							ppiFleetTable [j][ppiFleetTable [j][iNumFleets]] = i;
							ppiFleetTable [j][iNumFleets] ++;
							break;
						}
					}
				}
			}

			int iAlloc = iNumBuilders * (iNumFleets + 1);
			String* pstrLocationName = new String [iAlloc];
			Algorithm::AutoDelete<String> autostrLocationName (pstrLocationName, true);
			bool* pbFleetLocation = (bool*) StackAlloc (iAlloc * sizeof (bool));

			int iX, iY, iNumLocations = 0, iValue, iRealPlanet, iSelectedLocation = NO_KEY;

			char pszLocation [1024];

			iErrCode = g_pGameEngine->GetEmpireDefaultBuilderPlanet (
				m_iGameClass,
				m_iGameNumber,
				m_iEmpireKey,
				&iValue,
				&iRealPlanet
				);
			Assert (iErrCode == OK);

			for (i = 0; i < iNumBuilders; i ++) {

				if (g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, piBuilderKey[i], &vPlanetName) != OK ||
					g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piBuilderKey[i], &iX, &iY) != OK
					) {
					continue;
				}

				sprintf (
					pszLocation,
					"%s (%i,%i)",
					vPlanetName.GetCharPtr(),
					iX,
					iY
					);

				pstrLocationName [iNumLocations] = pszLocation;
				pbFleetLocation [iNumLocations] = false;

				%><input type="hidden" name="LocationPlanetKey<% Write (iNumLocations); %>" value="<% 
				Write (piBuilderKey[i]); %>"><input type="hidden" name="LocationFleetKey<% 
				Write (iNumLocations); %>" value="<% Write (NO_KEY); %>"><%

				if (iRealPlanet == piBuilderKey[i]) {
					iSelectedLocation = iNumLocations;
				}

				iNumLocations ++;

				if (ppiFleetTable [i][iNumFleets] > 0) {

					for (j = 0; j < ppiFleetTable [i][iNumFleets]; j ++) {

						sprintf (
							pszLocation,
							"%s (%i,%i) in fleet %s",
							vPlanetName.GetCharPtr(),
							iX,
							iY,
							pvFleetName [ppiFleetTable[i][j]].GetCharPtr()
							);

						pstrLocationName[iNumLocations] = pszLocation;
						pbFleetLocation[iNumLocations] = true; 

						%><input type="hidden" name="LocationPlanetKey<% Write (iNumLocations); 
						%>" value="<% Write (piBuilderKey[i]); 
						%>"><input type="hidden" name="LocationFleetKey<% Write (iNumLocations); 
						%>" value="<% Write (piFleetKey [ppiFleetTable [i][j]]); %>"><%

						iNumLocations ++;
					}
				}
			} // End builder loop

			%><input type="hidden" name="NumLocations" value="<% Write (iNumLocations); %>"><%

			// Get techs
			int iTechDevs, iTechUndevs, iMaxNumShipsBuiltAtOnce;

			if (g_pGameEngine->GetDevelopedTechs (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iTechDevs, &iTechUndevs) != OK ||
				g_pGameEngine->GetEmpireMaxNumShipsBuiltAtOnce (m_iEmpireKey, &iMaxNumShipsBuiltAtOnce) != OK
				) {

				if (iNumFleets > 0) {
					g_pGameEngine->FreeKeys (piFleetKey);
				}
				%><p>An error occurred rendering the Build page<%
				goto Close;
			}

			int iNumTechs = g_pGameEngine->GetNumTechs (iTechDevs);
			if (iNumTechs > 0) {

				%><input type="hidden" name="NumTechs" value="<% Write (iNumTechs); %>"><p><%

				bool bVisible = true;
				if (g_pGameEngine->GetGameClassVisibleBuilds (m_iGameClass, &bVisible) != OK) {
					if (iNumFleets > 0) {
						g_pGameEngine->FreeKeys (piFleetKey);
					}
					%><p>An error occurred rendering the Build page<%
					goto Close;
				}

				if (bVisible) {
					%>Builds are <strong>visible</strong><%
				} else {
					%>Builds are <strong>invisible</strong><%
				}

				%><p><table width="70%"><%
				%><tr><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); 
				%>" align="left">Type</th><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); 
				%>">Number</th><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); 
				%>">Name</th><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); 
				%>">BR</th><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); 
				%>">Location:</th></tr><%

				Variant vDefaultShipName;
				String strFilter;
				int iNumBuildableTechs = 0, iMin = min (iMaxNumShipsBuiltAtOnce + 1, 16);

				ENUMERATE_TECHS (i) {

					if (iTechDevs & TECH_BITS[i]) {

						if (g_pGameEngine->GetDefaultEmpireShipName (
							m_iEmpireKey, 
							i, 
							&vDefaultShipName
							) != OK ||

							HTMLFilter (
								vDefaultShipName.GetCharPtr(), 
								&strFilter,
								0,
								false
								) != OK) {
							
							continue;
						}
						%><input type="hidden" name="TechKey<% Write (iNumBuildableTechs); %>"<%
						%> value="<% Write (i); %>"><%

						%><tr><%
						%><td><% Write (SHIP_TYPE_STRING[i]); %></td><%

						%><td align="center"><%
						%><select name="NumShips<% Write (iNumBuildableTechs); %>" size="1"><%

						for (j = 0; j < iMin; j ++) {
							%><option><% Write (j); %></option><%
						}

						if (iMaxNumShipsBuiltAtOnce > 15) {
							for (j = 20; j <= iMaxNumShipsBuiltAtOnce; j += 10) {
								%><option><% Write (j); %></option><%
							}
						}
						%></select></td><%

						%><td align="center"><%
						%><input type="text" size="20" maxlength="20" name="ShipName<%
						Write (iNumBuildableTechs); %>"<%

						%> value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength()); %>"></td><%

						%><td align="center"><%
						if (iBR == 1) {
							Write (iBR);
							%><input type="hidden" name="ShipBR<% Write (iNumBuildableTechs); %>" value="1"><%
						} else {
							%><select name="ShipBR<% Write (iNumBuildableTechs); %>" size="1"><%

							if (iBR < 100) {

								for (j = 1; j < iBR; j ++) {
									%><option><% Write (j); %></option><%
								}
								%><option selected><% Write (iBR); %></select><%

							} else {

								for (j = 1; j < 10; j ++) {
									%><option><% Write (j); %></option><%
								}

								int iStep = iBR / 100;

								for (j = 10; j < iBR - 10; j += iStep) {
									%><option><% Write (j); %></option><%
								}

								for (j = iBR - 10; j < iBR; j ++) {
									%><option><% Write (j); %></option><%
								}

								%><option selected><% Write (iBR); %></select><%
							}
						}
						%></td><td align="center"><%

						if (iNumLocations == 1) {
							Write (pstrLocationName[0]);
							%><input type="hidden" name="ShipLocation<% Write (iNumBuildableTechs); %>" value="0"><%
						} else {

							%><select name="ShipLocation<% Write (iNumBuildableTechs); %>" size="1"><%
							for (j = 0; j < iNumLocations; j ++) {
								if (!pbFleetLocation[j] || (g_pGameEngine->IsMobileShip (i))) {
									%><option<%
									if (j == iSelectedLocation) {
										%> selected<%
									}
									%> value="<% Write (j); %>"><% Write (pstrLocationName[j]); %></option><%
								}
							}
							%></select><%
						}
						%></td></tr><%

						iNumBuildableTechs ++;
					}
				} 

				Assert (iNumBuildableTechs == iNumTechs);

				%></table><p><%
			}

			if (iNumFleets > 0) {
				g_pGameEngine->FreeKeys (piFleetKey);
			}
		}

	} // End iNumBuilders > 1

	// Get fleet locations
	int* piPlanetKey, iNumPlanets;
	GameCheck (g_pGameEngine->GetNewFleetLocations (m_iGameClass, m_iGameNumber, m_iEmpireKey, &piPlanetKey, 
		&iNumPlanets));

	if (iNumPlanets > 0) {

		Algorithm::AutoDelete<int> autopiPlanetKey (piPlanetKey, true);

		%><p><center>Create a new fleet:<p><table><tr><td bgcolor="<% 
		Write (m_vTableColor.GetCharPtr()); %>" align="center"><strong>Name:</strong></td><td bgcolor="<% 
		Write (m_vTableColor.GetCharPtr()); %>" align="center"><strong>Location:</strong></td></tr><tr><td><%
		%><input type="text" size="20" maxlength="<% Write (MAX_FLEET_NAME_LENGTH); 
		%>" name="NewFleetName"></td><td><%

		int iX, iY;

		if (iNumPlanets == 1) {

			GameCheck (g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, piPlanetKey[0], &vPlanetName));
			GameCheck (g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piPlanetKey[0], &iX, &iY));

			Write (vPlanetName.GetCharPtr()); %> (<% Write (iX); %>,<% Write (iY); %>)<%
			%><input type="hidden" name="NewFleetLocation" value="<% Write (piPlanetKey[0]); %>"><%
		} else {

			%><select name="NewFleetLocation"><%
			for (i = 0; i < iNumPlanets; i ++) {
				GameCheck (g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, piPlanetKey[i], &vPlanetName));
				GameCheck (g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piPlanetKey[i], &iX, &iY));
				%><option value="<% Write (piPlanetKey[i]); %>"><% Write (vPlanetName.GetCharPtr()); %> (<%
				Write (iX); %>,<% Write (iY); %>)</option><% 
			}
			%></select><%
		}
		%></td></tr></table><p><%
		WriteButton (BID_CANCEL);
	}
}

Close:

GAME_CLOSE

%>