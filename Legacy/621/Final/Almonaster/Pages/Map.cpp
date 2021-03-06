
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Map page
int HtmlRenderer::Render_Map() {

	// Almonaster
	// Copyright (c) 1998-2004 Max Attar Feingold (maf6@cornell.edu)
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

	bool bMapGenerated = (m_iGameState & GAME_MAP_GENERATED) != 0;
	bool bGameStarted = (m_iGameState & STARTED) != 0;
	bool bForceFullMap = false;

	int iErrCode, iMapSubPage = 0, iClickedPlanetKey = NO_KEY, iClickedProxyPlanetKey = NO_KEY;

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

	    if ((pHttpForm = m_pHttpRequest->GetForm ("MapSubPage")) == NULL || WasButtonPressed (BID_CANCEL)) {
	        goto Redirection;
	    }

	    const char* pszOldPlanetName, * pszNewPlanetName;
	    int iOldMaxPop, iNewMaxPop;
	    unsigned int iUpdatePlanetKey;

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
	        }   // End if partial maps

	EndPartialMaps:

	        // Planet click
	        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
	            (pszStart = pHttpForm->GetName()) != NULL &&
	            sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

	            iMapSubPage = 1;
	            bRedirectTest = false;
	            goto Redirection;
	        }
	        
	        // View minimaps
	        if (bMapGenerated && WasButtonPressed (BID_VIEWMINIMAP)) {
	            iMapSubPage = 2;
	            bRedirectTest = false;
	            goto Redirection;
	        }           

	        break;

	    case 1:

	        {

	        if (bMapGenerated) {

	            iErrCode = HandleShipMenuSubmissions();
	            if (iErrCode != OK) {
	                AddMessage ("Error handling ship menu submissions");
	                break;
	            }

	            // Get planet key
	            if ((pHttpForm = m_pHttpRequest->GetForm ("KeyPlanet0")) == NULL) {
	                goto Redirection;
	            }
	            iUpdatePlanetKey = pHttpForm->GetUIntValue();

	            iClickedPlanetKey = iUpdatePlanetKey;
	            iMapSubPage = 1;

	            // Get original name
	            if ((pHttpForm = m_pHttpRequest->GetForm ("OldPlanetName0")) != NULL) {

	                // Get old name
	                pszOldPlanetName = pHttpForm->GetValue();

	                // Get new name
	                if ((pHttpForm = m_pHttpRequest->GetForm ("NewPlanetName0")) == NULL) {
	                    goto Redirection;
	                }
	                pszNewPlanetName = pHttpForm->GetValue();

	                if (String::StrCmp (pszOldPlanetName, pszNewPlanetName) != 0) {

	                    if (String::IsWhiteSpace (pszNewPlanetName)) {
	                        AddMessage ("Blank planet names are not allowed");
	                    }
	                    else if (strlen (pszNewPlanetName) > MAX_PLANET_NAME_LENGTH) {
	                        AddMessage ("The new planet name was too long");
	                    } else {

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

	            // Build click
	            if (WasButtonPressed (BID_MINIBUILD)) {

	                HandleMiniBuild (iUpdatePlanetKey);

	                //iClickedPlanetKey = iUpdatePlanetKey;
	                //iMapSubPage = 1;
	                bRedirectTest = false;
	                break;
	            }

	            if (WasButtonPressed (BID_UPDATE)) {

	                //iClickedPlanetKey = iUpdatePlanetKey;
	                //iMapSubPage = 1;
	                bRedirectTest = false;
	                break;
	            }

	            // Planet click
	            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
	                (pszStart = pHttpForm->GetName()) != NULL &&
	                sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

	                //iMapSubPage = 1;
	                bRedirectTest = false;
	                break;
	            }
	        }

	        }
	        break;
	        
	    case 2:
	    
	        // View map
	        if (WasButtonPressed (BID_VIEWMAP)) {
	            iMapSubPage = 0;
	            bForceFullMap = true;
	            bRedirectTest = false;
	            goto Redirection;
	        }
	    
	        // Planet click
	        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
	            (pszStart = pHttpForm->GetName()) != NULL &&
	            sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

	            iMapSubPage = 1;
	            bRedirectTest = false;
	            goto Redirection;
	        }
	        break;

	    default:
	        Assert (false);
	    }
	}

	GAME_REDIRECT_ON_SUBMIT

	GAME_OPEN

	// Individual page stuff starts here
	if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
	    GameCheck (WriteRatiosString (NULL));
	}

	Variant vMiniMaps = MINIMAPS_NEVER;
	if (bMapGenerated && iMapSubPage == 0) {

	    GameCheck (g_pGameEngine->GetEmpireGameProperty (
	        m_iGameClass,
	        m_iGameNumber,
	        m_iEmpireKey,
	        GameEmpireData::MiniMaps,
	        &vMiniMaps
	        ));
	        
	    if (!bForceFullMap && vMiniMaps.GetInteger() == MINIMAPS_PRIMARY) {
	        iMapSubPage = 2;
	    }
	}

	//
	// Main switch
	//
	switch (iMapSubPage) {
	RenderWholeMap:
	case 0:

	    
	Write ("<input type=\"hidden\" name=\"MapSubPage\" value=\"0\" ID=\"Hidden1\">", sizeof ("<input type=\"hidden\" name=\"MapSubPage\" value=\"0\" ID=\"Hidden1\">") - 1);
	// Handle case where game hasn't started yet
	    if (!bMapGenerated) {

	        // Draw fake intro map
	        
	Write ("<input type=\"hidden\" name=\"MapSubPage\" value=\"0\"><p>Click on a planet for a closer view:<p><table cellspacing=\"0\" cellpadding=\"0\"><tr><td align=\"center\"><font size=\"1\">0</font></td><td rowspan=\"3\">", sizeof ("<input type=\"hidden\" name=\"MapSubPage\" value=\"0\"><p>Click on a planet for a closer view:<p><table cellspacing=\"0\" cellpadding=\"0\"><tr><td align=\"center\"><font size=\"1\">0</font></td><td rowspan=\"3\">") - 1);
	String strAlienButtonString;
	        GetAlienPlanetButtonString (m_iAlienKey, m_iEmpireKey, true, 0, 0, NULL, NULL, &strAlienButtonString);

	        m_pHttpResponse->WriteText (strAlienButtonString.GetCharPtr(), strAlienButtonString.GetLength());

	        
	Write ("</td><td align=\"center\"><font size=\"1\">0</font></td></tr><tr><td align=\"center\"><font size=\"1\">0</font></td><td align=\"center\"><font size=\"1\">0</font></td></tr><tr><td align=\"center\"><font size=\"1\">(0)</font></td><td align=\"center\"><font size=\"1\">(0)</font></td></tr><tr><td colspan=\"3\" align=\"center\"><font size=\"1\">", sizeof ("</td><td align=\"center\"><font size=\"1\">0</font></td></tr><tr><td align=\"center\"><font size=\"1\">0</font></td><td align=\"center\"><font size=\"1\">0</font></td></tr><tr><td align=\"center\"><font size=\"1\">(0)</font></td><td align=\"center\"><font size=\"1\">(0)</font></td></tr><tr><td colspan=\"3\" align=\"center\"><font size=\"1\">") - 1);
	Write (m_vEmpireName.GetCharPtr()); 
	Write ("</font></td></tr></table>", sizeof ("</font></td></tr></table>") - 1);
	} else {
	        
	        if (vMiniMaps.GetInteger() != MINIMAPS_NEVER) {
	            
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_VIEWMINIMAP);
	        }

	        // Get partial map info
	        PartialMapInfo pmiPartialMapInfo;
	        bool bPartialMaps = false;

	        GameCheck (g_pGameEngine->GetEmpirePartialMapData (
	            m_iGameClass,
	            m_iGameNumber,
	            m_iEmpireKey,
	            &bPartialMaps,
	            &pmiPartialMapInfo.iCenterKey,
	            &pmiPartialMapInfo.iXRadius,
	            &pmiPartialMapInfo.iYRadius
	            ));

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

	    bool bTrue, bOurPlanet = false;
	    unsigned int iNumShipsRendered = 0, iNumFleetsRendered = 0;

	    unsigned int iLivePlanetKey, iDeadPlanetKey;
	    iErrCode = g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);

	    if (iErrCode != OK) {
	        AddMessage ("Error reading empire's planet icons from database");
	        goto RenderWholeMap;
	    }

	    if (!bMapGenerated) {

	        
	Write ("<input type=\"hidden\" name=\"MapSubPage\" value=\"1\"><p><table width=\"90%\">", sizeof ("<input type=\"hidden\" name=\"MapSubPage\" value=\"1\"><p><table width=\"90%\">") - 1);
	iErrCode = WriteUpClosePlanetString (
	            m_iEmpireKey, NO_KEY, NO_KEY, iLivePlanetKey, iDeadPlanetKey, 0, false, 
	            0, 0, 0, 0, 0, 0, 0.0, false, false, false, NULL, &bTrue
	            );

	        if (iErrCode != OK) {
	            
	Write ("Error rendering up-close planet view. The error was ", sizeof ("Error rendering up-close planet view. The error was ") - 1);
	Write (iErrCode);
	        }

	    } else {

	        IDatabase* pDatabase;

	        int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;
	        Variant* pvPlanetData = NULL;
	        float fAgRatio;

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
	            AddMessage ("Database error "); AppendMessage (iErrCode);
	            pDatabase->Release();
	            goto RenderWholeMap;
	        }

	        iErrCode = pDatabase->ReadRow (pszGameMap, iClickedPlanetKey, &pvPlanetData);
	        if (iErrCode != OK) {
	            AddMessage ("Database error "); AppendMessage (iErrCode);
	            pDatabase->Release();
	            goto RenderWholeMap;
	        }

	        
	Write ("<input type=\"hidden\" name=\"MapSubPage\" value=\"1\"><p><table width=\"90%\">", sizeof ("<input type=\"hidden\" name=\"MapSubPage\" value=\"1\"><p><table width=\"90%\">") - 1);
	iErrCode = WriteUpClosePlanetString (
	            m_iEmpireKey, iClickedPlanetKey, 
	            iClickedProxyPlanetKey, iLivePlanetKey, iDeadPlanetKey, 0, 
	            (vOptions.GetInteger() & VISIBLE_BUILDS) != 0, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel,
	            iBadFuel, fAgRatio, (vOptions.GetInteger() & INDEPENDENCE) != 0, false, false, pvPlanetData, 
	            &bOurPlanet
	            );

	        pDatabase->FreeData (pvPlanetData);
	        SafeRelease (pDatabase);

	        if (iErrCode != OK) {
	            
	Write ("Error rendering up-close planet view. The error was ", sizeof ("Error rendering up-close planet view. The error was ") - 1);
	Write (iErrCode);
	            break;
	        }

	        if (!bOurPlanet) {
	            
	Write ("<input type=\"hidden\" name=\"KeyPlanet0\" value=\"", sizeof ("<input type=\"hidden\" name=\"KeyPlanet0\" value=\"") - 1);
	Write (iClickedPlanetKey); 
	Write ("\">", sizeof ("\">") - 1);
	}

	        // Render ships
	        if (m_iGameOptions & SHIPS_ON_MAP_SCREEN) {

	            ShipsInMapScreen simShipsInMap = { iClickedPlanetKey, 0, 0 };

	            int iBR;
	            float fMaintRatio, fNextMaintRatio;

	            GameCheck (g_pGameEngine->GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR));
	            GameCheck (g_pGameEngine->GetEmpireMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fMaintRatio));
	            GameCheck (g_pGameEngine->GetEmpireNextMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fNextMaintRatio));

	            // Render ships
	            RenderShips (
	                m_iGameClass,
	                m_iGameNumber,
	                m_iEmpireKey,
	                iBR,
	                fMaintRatio,
	                fNextMaintRatio,
	                &simShipsInMap,
	                true,
	                &iNumShipsRendered,
	                &iNumFleetsRendered
	                );

	            
	Write ("<input type=\"hidden\" name=\"NumShips\" value=\"", sizeof ("<input type=\"hidden\" name=\"NumShips\" value=\"") - 1);
	Write (simShipsInMap.iCurrentShip); 
	Write ("\"><input type=\"hidden\" name=\"NumFleets\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"NumFleets\" value=\"") - 1);
	Write (simShipsInMap.iCurrentFleet); 
	Write ("\">", sizeof ("\">") - 1);
	}

	        // Render build
	        if (m_iGameOptions & BUILD_ON_MAP_SCREEN) {  
	            RenderMiniBuild (iClickedPlanetKey, true);
	        }

	        if (m_iGameOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) {

	            
	Write ("<tr><td>&nbsp;</td></tr><tr><td></td><td align=\"center\" colspan=\"10\">", sizeof ("<tr><td>&nbsp;</td></tr><tr><td></td><td align=\"center\" colspan=\"10\">") - 1);
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

	    
	Write ("</table><p>", sizeof ("</table><p>") - 1);
	if (bOurPlanet || iNumShipsRendered > 0 || iNumFleetsRendered > 0) {
	        WriteButton (BID_CANCEL);
	        WriteButton (BID_UPDATE);
	    }

	    }
	    break;

	case 2:

	    
	Write ("<input type=\"hidden\" name=\"MapSubPage\" value=\"2\"><p>", sizeof ("<input type=\"hidden\" name=\"MapSubPage\" value=\"2\"><p>") - 1);
	WriteButton (BID_VIEWMAP);
	    
	Write ("<p>Click on a planet for a closer view:<p>", sizeof ("<p>Click on a planet for a closer view:<p>") - 1);
	RenderMiniMap (m_iGameClass, m_iGameNumber, m_iEmpireKey);
	    break;

	default:

	    Assert (false);
	    break;
	}

	GAME_CLOSE


}