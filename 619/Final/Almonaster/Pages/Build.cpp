
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Build page
int HtmlRenderer::Render_Build() {

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
		
	Write ("<input type=\"hidden\" name=\"NumTechs\" value=\"0\"><p>You cannot build ships or fleets before the game begins", sizeof ("<input type=\"hidden\" name=\"NumTechs\" value=\"0\"><p>You cannot build ships or fleets before the game begins") - 1);
	} else {

		int iBR;
		float fMaintRatio;
		WriteRatiosString (&iBR, &fMaintRatio); 
	Write ("<p>", sizeof ("<p>") - 1);
	WriteSeparatorString (m_iSeparatorKey);

		int* piBuilderKey, iNumBuilders;
		GameCheck (g_pGameEngine->GetBuilderPlanetKeys (m_iGameClass, m_iGameNumber, m_iEmpireKey, &piBuilderKey, 
			&iNumBuilders));

		if (iNumBuilders == 0) {
			
	Write ("<p>You have no builder planets to build ships on<input type=\"hidden\" name=\"NumTechs\" value=\"0\">", sizeof ("<p>You have no builder planets to build ships on<input type=\"hidden\" name=\"NumTechs\" value=\"0\">") - 1);
	} else {

			Algorithm::AutoDelete<int> autopiBuilderKey (piBuilderKey, true);

			if (iBR < 1) {
				
	Write ("<p><strong>You lack the technology level to build ships</strong><input type=\"hidden\" name=\"NumTechs\" value=\"0\">", sizeof ("<p><strong>You lack the technology level to build ships</strong><input type=\"hidden\" name=\"NumTechs\" value=\"0\">") - 1);
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

					
	Write ("<input type=\"hidden\" name=\"LocationPlanetKey", sizeof ("<input type=\"hidden\" name=\"LocationPlanetKey") - 1);
	Write (iNumLocations); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (piBuilderKey[i]); 
	Write ("\"><input type=\"hidden\" name=\"LocationFleetKey", sizeof ("\"><input type=\"hidden\" name=\"LocationFleetKey") - 1);
	Write (iNumLocations); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (NO_KEY); 
	Write ("\">", sizeof ("\">") - 1);
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

							
	Write ("<input type=\"hidden\" name=\"LocationPlanetKey", sizeof ("<input type=\"hidden\" name=\"LocationPlanetKey") - 1);
	Write (iNumLocations); 
							
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (piBuilderKey[i]); 
							
	Write ("\"><input type=\"hidden\" name=\"LocationFleetKey", sizeof ("\"><input type=\"hidden\" name=\"LocationFleetKey") - 1);
	Write (iNumLocations); 
							
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (piFleetKey [ppiFleetTable [i][j]]); 
	Write ("\">", sizeof ("\">") - 1);
	iNumLocations ++;
						}
					}
				} // End builder loop

				
	Write ("<input type=\"hidden\" name=\"NumLocations\" value=\"", sizeof ("<input type=\"hidden\" name=\"NumLocations\" value=\"") - 1);
	Write (iNumLocations); 
	Write ("\">", sizeof ("\">") - 1);
	// Get techs
				int iTechDevs, iTechUndevs, iMaxNumShipsBuiltAtOnce;

				if (g_pGameEngine->GetDevelopedTechs (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iTechDevs, &iTechUndevs) != OK ||
					g_pGameEngine->GetEmpireMaxNumShipsBuiltAtOnce (m_iEmpireKey, &iMaxNumShipsBuiltAtOnce) != OK
					) {

					if (iNumFleets > 0) {
						g_pGameEngine->FreeKeys (piFleetKey);
					}
					
	Write ("<p>An error occurred rendering the Build page", sizeof ("<p>An error occurred rendering the Build page") - 1);
	goto Close;
				}

				int iNumTechs = g_pGameEngine->GetNumTechs (iTechDevs);
				if (iNumTechs > 0) {

					
	Write ("<input type=\"hidden\" name=\"NumTechs\" value=\"", sizeof ("<input type=\"hidden\" name=\"NumTechs\" value=\"") - 1);
	Write (iNumTechs); 
	Write ("\"><p>", sizeof ("\"><p>") - 1);
	bool bVisible = true;
					if (g_pGameEngine->GetGameClassVisibleBuilds (m_iGameClass, &bVisible) != OK) {
						if (iNumFleets > 0) {
							g_pGameEngine->FreeKeys (piFleetKey);
						}
						
	Write ("<p>An error occurred rendering the Build page", sizeof ("<p>An error occurred rendering the Build page") - 1);
	goto Close;
					}

					if (bVisible) {
						
	Write ("Builds are <strong>visible</strong>", sizeof ("Builds are <strong>visible</strong>") - 1);
	} else {
						
	Write ("Builds are <strong>invisible</strong>", sizeof ("Builds are <strong>invisible</strong>") - 1);
	}

					
	Write ("<p><table width=\"70%\"><tr><th bgcolor=\"", sizeof ("<p><table width=\"70%\"><tr><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
					
	Write ("\" align=\"left\">Type</th><th bgcolor=\"", sizeof ("\" align=\"left\">Type</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
					
	Write ("\">Number</th><th bgcolor=\"", sizeof ("\">Number</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
					
	Write ("\">Name</th><th bgcolor=\"", sizeof ("\">Name</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
					
	Write ("\">BR</th><th bgcolor=\"", sizeof ("\">BR</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
					
	Write ("\">Location:</th></tr>", sizeof ("\">Location:</th></tr>") - 1);
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
							
	Write ("<input type=\"hidden\" name=\"TechKey", sizeof ("<input type=\"hidden\" name=\"TechKey") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (i); 
	Write ("\"><tr><td>", sizeof ("\"><tr><td>") - 1);
	Write (SHIP_TYPE_STRING[i]); 
	Write ("</td><td align=\"center\"><select name=\"NumShips", sizeof ("</td><td align=\"center\"><select name=\"NumShips") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" size=\"1\">", sizeof ("\" size=\"1\">") - 1);
	for (j = 0; j < iMin; j ++) {
								
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

							if (iMaxNumShipsBuiltAtOnce > 15) {
								for (j = 20; j <= iMaxNumShipsBuiltAtOnce; j += 10) {
									
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
							}
							
	Write ("</select></td><td align=\"center\"><input type=\"text\" size=\"20\" maxlength=\"20\" name=\"ShipName", sizeof ("</select></td><td align=\"center\"><input type=\"text\" size=\"20\" maxlength=\"20\" name=\"ShipName") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength()); 
	Write ("\"></td><td align=\"center\">", sizeof ("\"></td><td align=\"center\">") - 1);
	if (iBR == 1) {
								Write (iBR);
								
	Write ("<input type=\"hidden\" name=\"ShipBR", sizeof ("<input type=\"hidden\" name=\"ShipBR") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" value=\"1\">", sizeof ("\" value=\"1\">") - 1);
	} else {
								
	Write ("<select name=\"ShipBR", sizeof ("<select name=\"ShipBR") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" size=\"1\">", sizeof ("\" size=\"1\">") - 1);
	if (iBR < 100) {

									for (j = 1; j < iBR; j ++) {
										
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
									
	Write ("<option selected>", sizeof ("<option selected>") - 1);
	Write (iBR); 
	Write ("</select>", sizeof ("</select>") - 1);
	} else {

									for (j = 1; j < 10; j ++) {
										
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

									int iStep = iBR / 100;

									for (j = 10; j < iBR - 10; j += iStep) {
										
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

									for (j = iBR - 10; j < iBR; j ++) {
										
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

									
	Write ("<option selected>", sizeof ("<option selected>") - 1);
	Write (iBR); 
	Write ("</select>", sizeof ("</select>") - 1);
	}
							}
							
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	if (iNumLocations == 1) {
								Write (pstrLocationName[0]);
								
	Write ("<input type=\"hidden\" name=\"ShipLocation", sizeof ("<input type=\"hidden\" name=\"ShipLocation") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" value=\"0\">", sizeof ("\" value=\"0\">") - 1);
	} else {

								
	Write ("<select name=\"ShipLocation", sizeof ("<select name=\"ShipLocation") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" size=\"1\">", sizeof ("\" size=\"1\">") - 1);
	for (j = 0; j < iNumLocations; j ++) {
									if (!pbFleetLocation[j] || (g_pGameEngine->IsMobileShip (i))) {
										
	Write ("<option", sizeof ("<option") - 1);
	if (j == iSelectedLocation) {
											
	Write (" selected", sizeof (" selected") - 1);
	}
										
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (j); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pstrLocationName[j]); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
								}
								
	Write ("</select>", sizeof ("</select>") - 1);
	}
							
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	iNumBuildableTechs ++;
						}
					} 

					Assert (iNumBuildableTechs == iNumTechs);

					
	Write ("</table><p>", sizeof ("</table><p>") - 1);
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

			
	Write ("<p><center>Create a new fleet:<p><table><tr><td bgcolor=\"", sizeof ("<p><center>Create a new fleet:<p><table><tr><td bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\"><strong>Name:</strong></td><td bgcolor=\"", sizeof ("\" align=\"center\"><strong>Name:</strong></td><td bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\"><strong>Location:</strong></td></tr><tr><td><input type=\"text\" size=\"20\" maxlength=\"", sizeof ("\" align=\"center\"><strong>Location:</strong></td></tr><tr><td><input type=\"text\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_FLEET_NAME_LENGTH); 
			
	Write ("\" name=\"NewFleetName\"></td><td>", sizeof ("\" name=\"NewFleetName\"></td><td>") - 1);
	int iX, iY;

			if (iNumPlanets == 1) {

				GameCheck (g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, piPlanetKey[0], &vPlanetName));
				GameCheck (g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piPlanetKey[0], &iX, &iY));

				Write (vPlanetName.GetCharPtr()); 
	Write (" (", sizeof (" (") - 1);
	Write (iX); 
	Write (",", sizeof (",") - 1);
	Write (iY); 
	Write (")<input type=\"hidden\" name=\"NewFleetLocation\" value=\"", sizeof (")<input type=\"hidden\" name=\"NewFleetLocation\" value=\"") - 1);
	Write (piPlanetKey[0]); 
	Write ("\">", sizeof ("\">") - 1);
	} else {

				
	Write ("<select name=\"NewFleetLocation\">", sizeof ("<select name=\"NewFleetLocation\">") - 1);
	for (i = 0; i < iNumPlanets; i ++) {
					GameCheck (g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, piPlanetKey[i], &vPlanetName));
					GameCheck (g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piPlanetKey[i], &iX, &iY));
					
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piPlanetKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (vPlanetName.GetCharPtr()); 
	Write (" (", sizeof (" (") - 1);
	Write (iX); 
	Write (",", sizeof (",") - 1);
	Write (iY); 
	Write (")</option>", sizeof (")</option>") - 1);
	}
				
	Write ("</select>", sizeof ("</select>") - 1);
	}
			
	Write ("</td></tr></table><p>", sizeof ("</td></tr></table><p>") - 1);
	WriteButton (BID_CANCEL);
		}
	}

	Close:

	GAME_CLOSE


}