
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Map page
int HtmlRenderer::Render_Map() {

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

	bool bGameStarted = (m_iGameState & STARTED) != 0;

	int iErrCode, iMapSubPage = 0, iClickedPlanetKey = 0, iClickedProxyPlanetKey = 0;

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

		if ((pHttpForm = m_pHttpRequest->GetForm ("MapSubPage")) == NULL) {
			goto Redirection;
		} else {

			const char* pszOldPlanetName, * pszNewPlanetName;
			int iOldMaxPop, iNewMaxPop, iUpdatePlanetKey;

			switch (pHttpForm->GetIntValue()) {

			case 0:

				const char* pszStart;
				bool bPartialMaps, bFlag;
				PartialMapInfo pmiPartialMapInfo;
				unsigned int iNewValue;

				iErrCode = g_pGameEngine->GetEmpirePartialMapData (
					m_iGameClass,
					m_iGameNumber,
					m_iEmpireKey,
					&bPartialMaps,
					&pmiPartialMapInfo.iCenterKey,
					&pmiPartialMapInfo.iXRadius,
					&pmiPartialMapInfo.iYRadius
					);

				if (iErrCode != OK) {
					AddMessage ("Error reading partial map data");
					break;
				}

				if (bPartialMaps) {

					int iMinX = MAX_COORDINATE, iMaxX = MAX_COORDINATE, iMinY = MAX_COORDINATE, 
						iMaxY = MAX_COORDINATE, iCenterX = MAX_COORDINATE, iCenterY = MAX_COORDINATE;

					unsigned int iMaxRadius;

					pHttpForm = m_pHttpRequest->GetForm ("Center");
					if (pHttpForm != NULL) {

						iNewValue = pHttpForm->GetIntValue();
						if (iNewValue != pmiPartialMapInfo.iCenterKey) {

							if (iNewValue != PARTIAL_MAP_NATURAL_CENTER) {

								iErrCode = g_pGameEngine->HasEmpireVisitedPlanet (
									m_iGameClass,
									m_iGameNumber,
									m_iEmpireKey,
									iNewValue,
									&bFlag
									);

								if (iErrCode != OK || !bFlag) {
									AddMessage ("Your map center could not be updated");
									goto iXRadius;
								}
							}

							iErrCode = g_pGameEngine->SetEmpirePartialMapCenter (
								m_iGameClass,
								m_iGameNumber,
								m_iEmpireKey,
								iNewValue
								);

							if (iErrCode == OK) {
								AddMessage ("Your partial map center was updated");
							} else {
								AddMessage ("Your partial map center could not be updated");
							}
						}
					}

	iXRadius:

					pHttpForm = m_pHttpRequest->GetForm ("iXRadius");
					if (pHttpForm != NULL) {

						iNewValue = pHttpForm->GetIntValue();
						if (iNewValue != pmiPartialMapInfo.iXRadius) {

							if (iNewValue != PARTIAL_MAP_UNLIMITED_RADIUS) {

								if (iMinX == MAX_COORDINATE) {

									iErrCode = g_pGameEngine->GetMapLimits (
										m_iGameClass,
										m_iGameNumber,
										m_iEmpireKey,
										&iMinX,
										&iMaxX,
										&iMinY,
										&iMaxY
										);

									if (iErrCode != OK) {
										AddMessage ("Error reading map limits");
										break;
									} else {

										if (pmiPartialMapInfo.iCenterKey == PARTIAL_MAP_NATURAL_CENTER) {

											iCenterX = iMinX + (iMaxX - iMinX) / 2;
											iCenterY = iMinY + (iMaxY - iMinY) / 2;

										} else {

											iErrCode = g_pGameEngine->GetPlanetCoordinates (
												m_iGameClass,
												m_iGameNumber,
												pmiPartialMapInfo.iCenterKey,
												&iCenterX,
												&iCenterY
												);

											if (iErrCode != OK) {
												AddMessage ("Error reading map limits");
												break;
											}
										}
									}
								}

								Assert (iCenterX != MAX_COORDINATE);
								Assert (iMinX != MAX_COORDINATE);
								Assert (iMaxX != MAX_COORDINATE);

								iMaxRadius = max (iCenterX - iMinX, iMaxX - iCenterX);

								if (iNewValue < 1 || iNewValue > iMaxRadius) {
									AddMessage ("Invalid X radius");
									goto iYRadius;
								}
							}

							if ((iErrCode = g_pGameEngine->SetEmpirePartialMapXRadius (m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue)) == OK) {
								AddMessage ("Your X radius was updated");
							} else {
								AddMessage ("Your X radius could not be updated");
							}
						}
					}

	iYRadius:
					pHttpForm = m_pHttpRequest->GetForm ("iYRadius");
					if (pHttpForm != NULL) {

						iNewValue = pHttpForm->GetIntValue();
						if (iNewValue != pmiPartialMapInfo.iYRadius) {

							if (iNewValue != PARTIAL_MAP_UNLIMITED_RADIUS) {

								if (iMinX == MAX_COORDINATE) {

									iErrCode = g_pGameEngine->GetMapLimits (
										m_iGameClass,
										m_iGameNumber,
										m_iEmpireKey,
										&iMinX,
										&iMaxX,
										&iMinY,
										&iMaxY
										);

									if (iErrCode != OK) {
										AddMessage ("Error reading map limits");
										break;
									} else {

										if (pmiPartialMapInfo.iCenterKey == PARTIAL_MAP_NATURAL_CENTER) {

											iCenterX = iMinX + (iMaxX - iMinX) / 2;
											iCenterY = iMinY + (iMaxY - iMinY) / 2;

										} else {

											iErrCode = g_pGameEngine->GetPlanetCoordinates (
												m_iGameClass,
												m_iGameNumber,
												pmiPartialMapInfo.iCenterKey,
												&iCenterX,
												&iCenterY
												);

											if (iErrCode != OK) {
												AddMessage ("Error reading planet coordinates");
												break;
											}
										}
									}
								}

								Assert (iCenterY != MAX_COORDINATE);
								Assert (iMinY != MAX_COORDINATE);
								Assert (iMaxY != MAX_COORDINATE);

								iMaxRadius = max (iCenterY - iMinY, iMaxY - iCenterY);

								if (iNewValue < 1 || iNewValue > iMaxRadius) {
									AddMessage ("Invalid Y radius");
									goto EndPartialMaps;
								}
							}

							if ((iErrCode = g_pGameEngine->SetEmpirePartialMapYRadius (m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue)) == OK) {
								AddMessage ("Your Y radius was updated");
							} else {
								AddMessage ("Your Y radius could not be updated");
							}
						}
					}
				}	// End if partial maps

	EndPartialMaps:

				// Planet click
				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

					iMapSubPage = 1;
					bRedirectTest = false;
				}

				break;

			case 1:

				{

				if (bGameStarted) {

					iErrCode = HandleShipMenuSubmissions();
					if (iErrCode != OK) {
						AddMessage ("Error handling ship menu submissions");
						break;
					}

					iUpdatePlanetKey = NO_KEY;

					// Get original name
					if ((pHttpForm = m_pHttpRequest->GetForm ("OldPlanetName0")) != NULL) {

						pszOldPlanetName = pHttpForm->GetValue();

						// Get new name
						if ((pHttpForm = m_pHttpRequest->GetForm ("NewPlanetName0")) == NULL) {
							goto Redirection;
						}
						pszNewPlanetName = pHttpForm->GetValue();

						if (String::StrCmp (pszOldPlanetName, pszNewPlanetName) != 0) {

							if (pszNewPlanetName == NULL) {
								pszNewPlanetName = "";
							}

							if (strlen (pszNewPlanetName) > MAX_PLANET_NAME_LENGTH) {
								AddMessage ("The submitted planet name was too long");
							} else {

								// Get planet key
								if ((pHttpForm = m_pHttpRequest->GetForm ("KeyPlanet0")) == NULL) {
									goto Redirection;
								}
								iUpdatePlanetKey = pHttpForm->GetIntValue();

								iErrCode = g_pGameEngine->RenamePlanet (
									m_iGameClass,
									m_iGameNumber,
									m_iEmpireKey,
									iUpdatePlanetKey,
									pszNewPlanetName
									);

								if (iErrCode != OK) {
									AddMessage ("Error renaming planet");
									break;
								}
							}
						}

						// Get original MaxPop
						if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxPop0")) == NULL) {
							goto Redirection;
						}
						iOldMaxPop = pHttpForm->GetIntValue();

						// Get new MaxPop
						if ((pHttpForm = m_pHttpRequest->GetForm ("NewMaxPop0")) == NULL) {
							goto Redirection;
						}
						iNewMaxPop = pHttpForm->GetIntValue();

						if (iOldMaxPop != iNewMaxPop) {

							// Get planet key
							if (iUpdatePlanetKey == NO_KEY) {

								if ((pHttpForm = m_pHttpRequest->GetForm ("KeyPlanet0")) == NULL) {
									goto Redirection;
								}
								iUpdatePlanetKey = pHttpForm->GetIntValue();
							}

							iErrCode = g_pGameEngine->SetPlanetMaxPop (
								m_iGameClass,
								m_iGameNumber,
								m_iEmpireKey, 
								iUpdatePlanetKey,
								iNewMaxPop
								);

							if (iErrCode != OK) {

								if (iErrCode == ERROR_PLANET_DOES_NOT_BELONG_TO_EMPIRE) {
									AddMessage ("Error setting planet pop: the planet does not belong to your empire");
								} else {
									AddMessage ("Error setting planet population");
								}
								break;
							}
						}
					}

					// Planet click
					if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
						(pszStart = pHttpForm->GetName()) != NULL &&
						sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

						iMapSubPage = 1;
						bRedirectTest = false;
						break;
					}
				}

				}
				break;

			default:
				Assert (false);
			}
		}
	}

	GAME_REDIRECT_ON_SUBMIT

	GAME_OPEN

	// Individual page stuff starts here
	int iNumPlanets;

	switch (iMapSubPage) {
	RenderWholeMap:
	case 0:

		
	Write ("<input type=\"hidden\" name=\"MapSubPage\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"MapSubPage\" value=\"0\">") - 1);
	// Draw fake intro map 
		iNumPlanets = 1; 

		// Handle case when game hasn't started yet
		if (!bGameStarted) {

			
	Write ("<p>Click on a planet for a closer view:<p><table cellspacing=\"0\" cellpadding=\"0\"><tr><td align=\"center\"><font size=\"1\">0</font></td><td rowspan=\"3\">", sizeof ("<p>Click on a planet for a closer view:<p><table cellspacing=\"0\" cellpadding=\"0\"><tr><td align=\"center\"><font size=\"1\">0</font></td><td rowspan=\"3\">") - 1);
	String strAlienButtonString;
			GetAlienButtonString (m_iAlienKey, m_iEmpireKey, true, 0, 0, NULL, &strAlienButtonString);

			m_pHttpResponse->WriteText (strAlienButtonString.GetCharPtr(), strAlienButtonString.GetLength());

			
	Write ("</td><td align=\"center\"><font size=\"1\">0</font></td></tr><tr><td align=\"center\"><font size=\"1\">0</font></td><td align=\"center\"><font size=\"1\">0</font></td></tr><tr><td align=\"center\"><font size=\"1\">(0)</font></td><td align=\"center\"><font size=\"1\">(0)</font></td></tr><tr><td colspan=\"3\" align=\"center\"><font size=\"1\">", sizeof ("</td><td align=\"center\"><font size=\"1\">0</font></td></tr><tr><td align=\"center\"><font size=\"1\">0</font></td><td align=\"center\"><font size=\"1\">0</font></td></tr><tr><td align=\"center\"><font size=\"1\">(0)</font></td><td align=\"center\"><font size=\"1\">(0)</font></td></tr><tr><td colspan=\"3\" align=\"center\"><font size=\"1\">") - 1);
	Write (m_vEmpireName.GetCharPtr()); 
	Write ("</font></td></tr></table>", sizeof ("</font></td></tr></table>") - 1);
	} else {

			// Get partial map info
			PartialMapInfo pmiPartialMapInfo;
			bool bPartialMaps = false;

			iErrCode = g_pGameEngine->GetEmpirePartialMapData (
				m_iGameClass,
				m_iGameNumber,
				m_iEmpireKey,
				&bPartialMaps,
				&pmiPartialMapInfo.iCenterKey,
				&pmiPartialMapInfo.iXRadius,
				&pmiPartialMapInfo.iYRadius
				);

			if (iErrCode != OK) {
				AddMessage ("Error reading partial map data. The error was ");
				AppendMessage (iErrCode);
			}

			pmiPartialMapInfo.bDontShowPartialOptions = false;

			iErrCode = RenderMap (
				m_iGameClass,
				m_iGameNumber,
				m_iEmpireKey,
				false,
				bPartialMaps ? &pmiPartialMapInfo : NULL,
				false
				);

			if (iErrCode != OK) {
				AddMessage ("Error rendering map data. The error was ");
				AppendMessage (iErrCode);
			}
		}

		break;

	case 1:
		{

		bool bTrue;

		int iLivePlanetKey, iDeadPlanetKey;
		iErrCode = g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);

		if (iErrCode != OK) {
			AddMessage ("Error reading empire's planet icons from database");
			goto RenderWholeMap;
		}

		if (!bGameStarted) {

			
	Write ("<input type=\"hidden\" name=\"MapSubPage\" value=\"1\"><p><table width=\"90%\">", sizeof ("<input type=\"hidden\" name=\"MapSubPage\" value=\"1\"><p><table width=\"90%\">") - 1);
	iErrCode = WriteUpClosePlanetString (
				NO_KEY, NO_KEY, NO_KEY, iLivePlanetKey, iDeadPlanetKey, 0, false, 
				0, 0, 0, 0, 0, 0, 0.0, false, false, false, NULL, &bTrue
				);

			if (iErrCode != OK) {
				
	Write ("Error rendering up-close planet view. The error was ", sizeof ("Error rendering up-close planet view. The error was ") - 1);
	Write (iErrCode);
			}

		} else {

			IDatabase* pDatabase;
			IReadTable* pGameMap;

			int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel, iGameOptions;
			void** ppPlanetData;
			float fAgRatio;

			bool bFlag;

			Variant vOptions;

			GAME_MAP (pszGameMap, m_iGameClass, m_iGameNumber);

			// Make sure we've explored that planet
			iErrCode = g_pGameEngine->HasEmpireExploredPlanet (
				m_iGameClass,
				m_iGameNumber,
				m_iEmpireKey, 
				iClickedPlanetKey,
				&bTrue
				);

			if (iErrCode != OK) {
				AddMessage ("Error determining if empire has explored planet. The error was ");
				AppendMessage (iErrCode);
				goto RenderWholeMap;
			}

			if (!bTrue) {
				AddMessage ("You have not explored that planet yet");
				goto RenderWholeMap;
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
				AddMessage ("Error reading resource limits for good/bad colors. The error was ");
				AppendMessage (iErrCode);
				goto RenderWholeMap;
			}

			iErrCode = g_pGameEngine->GetEmpireAgRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fAgRatio);
			if (iErrCode != OK) {
				AddMessage ("Error reading empire's ag ratio. The error was ");
				AppendMessage (iErrCode);
				goto RenderWholeMap;
			}

			pDatabase = g_pGameEngine->GetDatabase();

			iErrCode = pDatabase->ReadData (SYSTEM_GAMECLASS_DATA, m_iGameClass, SystemGameClassData::Options, &vOptions);
			if (iErrCode != OK) {
				AddMessage ("Database error ");
				AppendMessage (iErrCode);
				pDatabase->Release();
				goto RenderWholeMap;
			}

			iErrCode = pDatabase->GetTableForReading (pszGameMap, &pGameMap);
			if (iErrCode != OK) {
				AddMessage ("Database error ");
				AppendMessage (iErrCode);
				pDatabase->Release();
				goto RenderWholeMap;
			}

			iErrCode = pGameMap->ReadRow (iClickedPlanetKey, &ppPlanetData);
			if (iErrCode != OK) {
				Assert (false);
				AddMessage ("Database error ");
				AppendMessage (iErrCode);
				pGameMap->Release();
				pDatabase->Release();
				goto RenderWholeMap;
			}

			
	Write ("<input type=\"hidden\" name=\"MapSubPage\" value=\"1\"><p><table width=\"90%\">", sizeof ("<input type=\"hidden\" name=\"MapSubPage\" value=\"1\"><p><table width=\"90%\">") - 1);
	iErrCode = WriteUpClosePlanetString (
				m_iEmpireKey, iClickedPlanetKey, 
				iClickedProxyPlanetKey, iLivePlanetKey, iDeadPlanetKey, 0, 
				(vOptions.GetInteger() & VISIBLE_BUILDS) != 0, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel,
				iBadFuel, fAgRatio, (vOptions.GetInteger() & INDEPENDENCE) != 0, false, false, ppPlanetData, &bTrue
				);

			pDatabase->FreeData (ppPlanetData);
			pGameMap->Release();

			if (iErrCode != OK) {
				
	Write ("Error rendering up-close planet view. The error was ", sizeof ("Error rendering up-close planet view. The error was ") - 1);
	Write (iErrCode);
				pDatabase->Release();
				break;
			}

			// Render ships
			iErrCode = g_pGameEngine->GetEmpireOptions (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iGameOptions);
			if (iErrCode != OK) {

				AddMessage ("Error reading empire ships on map screen setting. The error was ");
				AppendMessage (iErrCode);

			} else {

				bFlag = (iGameOptions & SHIPS_ON_MAP_SCREEN) != 0;
				if (bFlag) {

					ShipsInMapScreen simShipsInMap = { iClickedPlanetKey, 0, 0 };

					IReadTable* pShips = NULL, * pFleets = NULL;

					int iBR;
					float fMaintRatio;

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

					
	Write ("<tr></tr><tr><td></td><td align=\"center\" colspan=\"10\">", sizeof ("<tr></tr><tr><td></td><td align=\"center\" colspan=\"10\">") - 1);
	// Render ships
					RenderShips (
						pShips,
						pFleets,
						iBR,
						fMaintRatio,
						&simShipsInMap
						);

				Cleanup:

					if (pShips != NULL) {
						pShips->Release();
					}

					if (pFleets != NULL) {
						pFleets->Release();
					}

					if (iErrCode == OK) {

						
	Write ("<input type=\"hidden\" name=\"NumShips\" value=\"", sizeof ("<input type=\"hidden\" name=\"NumShips\" value=\"") - 1);
	Write (simShipsInMap.iCurrentShip); 
	Write ("\"><input type=\"hidden\" name=\"NumFleets\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"NumFleets\" value=\"") - 1);
	Write (simShipsInMap.iCurrentFleet); 
	Write ("\">", sizeof ("\">") - 1);
	}

					
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}
			}

			pDatabase->Release();

			bFlag = (iGameOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;
			if (bFlag) {

				
	Write ("<tr></tr><tr><td></td><td align=\"center\" colspan=\"10\">", sizeof ("<tr></tr><tr><td></td><td align=\"center\" colspan=\"10\">") - 1);
	// Render local map
				PartialMapInfo pmiPartialMapInfo = { iClickedPlanetKey, 1, 1, true };

				iErrCode = RenderMap (
					m_iGameClass,
					m_iGameNumber,
					m_iEmpireKey,
					false,
					&pmiPartialMapInfo,
					false
					);

				if (iErrCode != OK) {
					AddMessage ("Error rendering map data. The error was ");
					AppendMessage (iErrCode);
				}

				
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}
		}

		
	Write ("</table>", sizeof ("</table>") - 1);
	}
		break;

	default:

		Assert (false);
	}

	GAME_CLOSE


}