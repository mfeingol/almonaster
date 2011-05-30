//
// Almonaster.dll:  a component of Almonaster
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

#include "HtmlRenderer.h"


int HtmlRenderer::RenderMap (int iGameClass, int iGameNumber, int iEmpireKey, bool bAdmin, 
                             const PartialMapInfo* pPartialMapInfo, bool bSpectators) {
    
    Variant* pvPlanetKey = NULL, vOptions, * pvEmpireKey = NULL, vTemp, pvEasyWayOut[9], * pvPlanetData = NULL;
    
    int iErrCode, iMinX, iMaxX, iMinY, iMaxY, iNumJumps, iMapMinX = 0, iMapMaxX = 0, iMapMinY = 0, iMapMaxY = 0;
    unsigned int iNumPlanets, * piPlanetKey = NULL, i, j, * piProxyKey = NULL, iLivePlanetKey, iDeadPlanetKey;

    size_t stTemp;
    
    IDatabase* pDatabase = g_pGameEngine->GetDatabase();
    
    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    unsigned int piEasyProxyKeys[9], iGridX, iGridY, iGridLocX, iGridLocY, iNumEmpires;

    int iHorzKey, iVertKey, iNumOwnShips, iNumOtherShips, iAlienKey, iAccountingNumOtherShips, iDiplomacyLevel,
        iCenterX = MAX_COORDINATE, iCenterY = MAX_COORDINATE, iNumUncloakedShips, iNumCloakedShips, 
        iNumUncloakedBuildShips, iNumCloakedBuildShips, iWeOffer, iTheyOffer, iCurrent, iX, iY, iOwner, iLink, 
        iProxyKey, iPlanetKey;
    
    bool bPartialMapShortcut = false;
    
    String* pstrGrid = NULL, ** ppstrGrid, strPlanetString, strImage, strHorz, strVert, strFilter, strAltTag;
    
    char* pszHorz, * pszVert;
    const char* pszColor;

    bool bLinkNorth, bLinkEast, bLinkSouth, bLinkWest, bVisible, bIndependence, bSensitive, bMapColoring, 
        bShipColoring, bHighlightShips;
    

    Assert (!(bSpectators && bAdmin));

    if (!bAdmin && !bSpectators) {

        int iOptions = m_iGameOptions;

        bSensitive = (iOptions & SENSITIVE_MAPS) != 0;
        bMapColoring = (iOptions & MAP_COLORING) != 0;
        bShipColoring = (iOptions & SHIP_MAP_COLORING) != 0;
        bHighlightShips = (iOptions & SHIP_MAP_HIGHLIGHTING) != 0;
        
        // Get map geography information
        iErrCode = g_pGameEngine->GetMapLimits (
            iGameClass, 
            iGameNumber, 
            iEmpireKey, 
            &iMapMinX, 
            &iMapMaxX, 
            &iMapMinY, 
            &iMapMaxY
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (pPartialMapInfo != NULL && 
            pPartialMapInfo->iCenterKey != PARTIAL_MAP_NATURAL_CENTER && 
            pPartialMapInfo->iXRadius == 1 && 
            pPartialMapInfo->iYRadius == 1
            ) {
            
            bPartialMapShortcut = true;
            
            // Scan 8 surrounding planets
            iErrCode = g_pGameEngine->GetVisitedSurroundingPlanetKeys (
                iGameClass, 
                iGameNumber, 
                iEmpireKey,
                pPartialMapInfo->iCenterKey,
                pvEasyWayOut,
                (int*) piEasyProxyKeys,
                (int*) &iNumPlanets,
                &iCenterX,
                &iCenterY,
                &iMinX, 
                &iMaxX, 
                &iMinY, 
                &iMaxY
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            pvPlanetKey = pvEasyWayOut;
            piProxyKey = piEasyProxyKeys;
            
            Assert (iNumPlanets > 0);
            
        } else {
            
            iErrCode = g_pGameEngine->GetVisitedPlanetKeys (
                iGameClass, 
                iGameNumber, 
                iEmpireKey,
                &pvPlanetKey,
                &piProxyKey, 
                &iNumPlanets
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            Assert (iNumPlanets > 0);
            
            iMinX = iMapMinX;
            iMaxX = iMapMaxX; 
            iMinY = iMapMinY;
            iMaxY = iMapMaxY;
        }
        
    } else {
        
        // Options
        int iOptions = m_iSystemOptions;

        bSensitive = (iOptions & SENSITIVE_MAPS) != 0;
        bMapColoring = false;
        bShipColoring = false;
        bHighlightShips = (iOptions & SHIP_MAP_HIGHLIGHTING) != 0;

        // Get map geography information
        iErrCode = g_pGameEngine->GetMapLimits (
            iGameClass, 
            iGameNumber,
            &iMapMinX, 
            &iMapMaxX, 
            &iMapMinY, 
            &iMapMaxY
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pDatabase->GetAllKeys (strGameMap, &piPlanetKey, &iNumPlanets);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iMinX = iMapMinX;
        iMaxX = iMapMaxX; 
        iMinY = iMapMinY;
        iMaxY = iMapMaxY;
    }
    
    // Partial map menus
    if (pPartialMapInfo != NULL) {

        Assert (!bSpectators);
        
        if (!bPartialMapShortcut) {
            
            // Get radius data
            if (pPartialMapInfo->iCenterKey == PARTIAL_MAP_NATURAL_CENTER) {
                
                iCenterX = iMapMinX + (iMapMaxX - iMapMinX) / 2;
                iCenterY = iMapMinY + (iMapMaxY - iMapMinY) / 2;
                
            } else {
                
                iErrCode = g_pGameEngine->GetPlanetCoordinates (
                    iGameClass,
                    iGameNumber,
                    pPartialMapInfo->iCenterKey,
                    &iCenterX,
                    &iCenterY
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
            
            if (pPartialMapInfo->iXRadius != PARTIAL_MAP_UNLIMITED_RADIUS) {
                iMinX = iCenterX - pPartialMapInfo->iXRadius;
                iMaxX = iCenterX + pPartialMapInfo->iXRadius;
                Assert (iMinX >= 0);
            }
            
            if (pPartialMapInfo->iYRadius != PARTIAL_MAP_UNLIMITED_RADIUS) {
                iMinY = iCenterY - pPartialMapInfo->iYRadius;
                iMaxY = iCenterY + pPartialMapInfo->iYRadius;
                Assert (iMinY >= 0);
            }
        }
        
        // Draw table
        if (!pPartialMapInfo->bDontShowPartialOptions) {
            
            unsigned int iValue, iMaxXRadius, iMaxYRadius;
            
            Assert (iCenterX != MAX_COORDINATE);
            Assert (iCenterY != MAX_COORDINATE);
            
            iMaxXRadius = max (iCenterX - iMapMinX, iMapMaxX - iCenterX);
            iMaxYRadius = max (iCenterY - iMapMinY, iMapMaxY - iCenterY);
            
            OutputText ("<p><table><tr><th bgcolor=\"#");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\">Partial map center</th><th bgcolor=\"#");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\">Partial map X radius</th><th bgcolor=\"#");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\">Partial map Y radius</th></tr><tr><td><select name=\"Center\"><option");
            
            if (pPartialMapInfo->iCenterKey == PARTIAL_MAP_NATURAL_CENTER) {
                OutputText (" selected");
            }
            OutputText (" value=\"");
            m_pHttpResponse->WriteText (PARTIAL_MAP_NATURAL_CENTER);
            OutputText ("\">The map's natural center (");
            m_pHttpResponse->WriteText (iCenterX);
            OutputText (",");
            m_pHttpResponse->WriteText (iCenterY);
            OutputText (")</option>");
            
            for (i = 0; i < iNumPlanets; i ++) {

                char pszName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH];
                
                OutputText ("<option");
                if (pPartialMapInfo->iCenterKey == (unsigned int) pvPlanetKey[i].GetInteger()) {
                    OutputText (" selected");
                }
                OutputText (" value=\"");
                m_pHttpResponse->WriteText (pvPlanetKey[i].GetInteger());
                OutputText ("\">");
                
                iErrCode = g_pGameEngine->GetPlanetNameWithCoordinates (
                    strGameMap, 
                    pvPlanetKey[i].GetInteger(), 
                    pszName
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                m_pHttpResponse->WriteText (pszName);
                OutputText ("</option>");
            }
            OutputText ("</select></td><td><select name=\"iXRadius\"><option");
            
            if (pPartialMapInfo->iXRadius == PARTIAL_MAP_UNLIMITED_RADIUS) {
                OutputText (" selected");
            }
            
            OutputText (" value=\"");
            m_pHttpResponse->WriteText (PARTIAL_MAP_UNLIMITED_RADIUS);
            OutputText ("\">As large as possible</option>");
            
            for (iValue = 1; iValue <= iMaxXRadius; iValue ++) {
                
                OutputText ("<option");
                if (pPartialMapInfo->iXRadius == iValue) {
                    OutputText (" selected");
                }
                OutputText (" value=\"");
                m_pHttpResponse->WriteText (iValue);
                OutputText ("\">");
                m_pHttpResponse->WriteText (iValue);
                OutputText ("</option>");
            }
            OutputText ("</select></td><td><select name=\"iYRadius\"><option");
            
            // Y radius
            if (pPartialMapInfo->iYRadius == PARTIAL_MAP_UNLIMITED_RADIUS) {
                OutputText (" selected");
            }
            
            OutputText (" value=\"");
            m_pHttpResponse->WriteText (PARTIAL_MAP_UNLIMITED_RADIUS);
            OutputText ("\">As large as possible</option>");
            
            for (iValue = 1; iValue <= iMaxYRadius; iValue ++) {
                
                OutputText ("<option");
                if (pPartialMapInfo->iYRadius == iValue) {
                    OutputText (" selected");
                }
                OutputText (" value=\"");
                m_pHttpResponse->WriteText (iValue);
                OutputText ("\">");
                m_pHttpResponse->WriteText (iValue);
                OutputText ("</option>");
            }
            OutputText ("</select></td></tr></table>");
        }
    }
    
    OutputText ("<p>Click on a planet for a closer view:<p>");
    
    // We have the end points, so generate the grid
    iGridX = (iMaxX - iMinX + 1) * 3;
    iGridY = (iMaxY - iMinY + 1) * 3;
    
    // Allocate grid
    pstrGrid = new String [iGridX * iGridY];
    ppstrGrid = (String**) StackAlloc (iGridX * sizeof (String*));
    
    for (i = 0; i < iGridX; i ++) {
        ppstrGrid[i] = pstrGrid + i * iGridY;
    }
    
    if (m_iThemeKey == INDIVIDUAL_ELEMENTS) {
        
        Variant vValue;

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIHorz, &vValue);
        if (iErrCode != OK) {
            return iErrCode;
        }
        iHorzKey = vValue.GetInteger();

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIVert, &vValue);
        if (iErrCode != OK) {
            return iErrCode;
        }
        iVertKey = vValue.GetInteger();

        iErrCode = g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
    } else {
        
        iHorzKey = iVertKey = iLivePlanetKey = iDeadPlanetKey = m_iThemeKey;
    }
    
    iErrCode = GetHorzString (iHorzKey, &strHorz, false);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = GetVertString (iVertKey, &strVert, false);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    pszHorz = (char*) StackAlloc (strHorz.GetLength() + 100);
    pszVert = (char*) StackAlloc (strVert.GetLength() + 100);
    
    //sprintf (pszHorz, "<td align=\"center\">%s</td>", strHorz.GetCharPtr());
    //sprintf (pszVert, "<td align=\"center\">%s</td>", strVert.GetCharPtr());
    
    stTemp = sizeof ("<td align=\"center\">") - 1;
    
    memcpy (pszHorz, "<td align=\"center\">", stTemp);
    memcpy (pszVert, "<td align=\"center\">", stTemp);
    
    memcpy (pszHorz + stTemp, strHorz.GetCharPtr(), strHorz.GetLength());
    memcpy (pszVert + stTemp, strVert.GetCharPtr(), strVert.GetLength());
    
    memcpy (pszHorz + stTemp + strHorz.GetLength(), "</td>", sizeof ("</td>"));
    memcpy (pszVert + stTemp + strVert.GetLength(), "</td>", sizeof ("</td>"));
    
    iErrCode = pDatabase->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vOptions
        );
    
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    bVisible = (vOptions.GetInteger() & VISIBLE_BUILDS) != 0;
    bIndependence = (vOptions.GetInteger() & INDEPENDENCE) != 0;

    // Loop through all planets, placing them on grid
    for (i = 0; i < iNumPlanets; i ++) {
        
        if (!bAdmin && !bSpectators) {
            
            iPlanetKey = pvPlanetKey[i].GetInteger();
            iProxyKey = piProxyKey[i];
            
            iErrCode = pDatabase->ReadRow (strGameMap, iPlanetKey, &pvPlanetData);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            g_pGameEngine->GetCoordinates (pvPlanetData[GameMap::Coordinates].GetCharPtr(), &iX, &iY);
            
            // Partial map filtering
            if (pPartialMapInfo != NULL && (iX > iMaxX || iX < iMinX || iY > iMaxY || iY < iMinY)) {
                pDatabase->FreeData (pvPlanetData);
                pvPlanetData = NULL;
                continue;
            }
            
            iErrCode = pDatabase->ReadData (strGameEmpireMap, iProxyKey, GameEmpireMap::NumUncloakedShips, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            iNumUncloakedShips = vTemp.GetInteger();
            
            iErrCode = pDatabase->ReadData (strGameEmpireMap, iProxyKey, GameEmpireMap::NumCloakedShips, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            iNumCloakedShips = vTemp.GetInteger();
            
            iErrCode = pDatabase->ReadData (strGameEmpireMap, iProxyKey, GameEmpireMap::NumUncloakedBuildShips, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            iNumUncloakedBuildShips = vTemp.GetInteger();
            
            iErrCode = pDatabase->ReadData (strGameEmpireMap, iProxyKey, GameEmpireMap::NumCloakedBuildShips, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            iNumCloakedBuildShips = vTemp.GetInteger();

            iNumOwnShips = iNumUncloakedShips + iNumCloakedShips + iNumUncloakedBuildShips + iNumCloakedBuildShips;
            
        } else {
            
            iPlanetKey = piPlanetKey[i];
            iProxyKey = 0;
            
            iErrCode = pDatabase->ReadRow (strGameMap, iPlanetKey, &pvPlanetData);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            g_pGameEngine->GetCoordinates (pvPlanetData[GameMap::Coordinates].GetCharPtr(), &iX, &iY);
            
            iNumOwnShips = iNumUncloakedShips = iNumCloakedShips = iNumUncloakedBuildShips = 
                iNumCloakedBuildShips = 0;
        }
        
        // Main map loop
        iNumOtherShips = pvPlanetData[GameMap::NumUncloakedShips].GetInteger() - iNumUncloakedShips;
        if (bVisible || bAdmin) {
            iNumOtherShips += pvPlanetData[GameMap::NumUncloakedBuildShips].GetInteger() - iNumUncloakedBuildShips;
        }
        
        Assert (iNumOtherShips >= 0 && iNumOwnShips >= 0);
        
        iLink = pvPlanetData[GameMap::Link].GetInteger();

        bLinkNorth = (iLink & LINK_NORTH) != 0;
        bLinkEast  = (iLink & LINK_EAST) != 0;
        bLinkSouth = (iLink & LINK_SOUTH) != 0;
        bLinkWest  = (iLink & LINK_WEST) != 0;
        
        iNumJumps = (bLinkNorth ? 1:0) + (bLinkEast ? 1:0) + (bLinkSouth ? 1:0) + (bLinkWest ? 1:0);
        
        iOwner = pvPlanetData[GameMap::Owner].GetInteger();
        
        // Get grid coordinates     
        iGridLocX = (iX - iMinX) * 3 + 1;
        iGridLocY = (iMaxY - iY) * 3 + 1;
        
        Assert (iGridLocX >= 0 && iGridLocY >= 0 && iGridLocX < iGridX && iGridLocY < iGridY);
        
        if (bSensitive) {
            
            iErrCode = GetSensitiveMapText (
                iGameClass, 
                iGameNumber, 
                bAdmin ? SYSTEM : (bSpectators ? GUEST : iEmpireKey),
                iPlanetKey,
                iProxyKey, 
                bVisible || bAdmin,
                bIndependence,
                pvPlanetData,
                &strAltTag
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
        
        // Get planet string        
        if (pvPlanetData[GameMap::Annihilated].GetInteger() != NOT_ANNIHILATED) {
            
            GetDeadPlanetButtonString (iDeadPlanetKey, iPlanetKey, iProxyKey, strAltTag, NULL, &strPlanetString);
            pszColor = NULL;
            
        } else {
            
            switch (iOwner) {
                
            case SYSTEM:
                
                GetLivePlanetButtonString (iLivePlanetKey, iPlanetKey, iProxyKey, strAltTag, NULL, &strPlanetString);
                pszColor = NULL;
                
                break;
                
            case INDEPENDENT:
                
                GetIndependentPlanetButtonString (iPlanetKey, iProxyKey, strAltTag, NULL, &strPlanetString);
                pszColor = bMapColoring ? m_vBadColor.GetCharPtr() : NULL;
                
                break;
                
            default:
                
                iErrCode = g_pGameEngine->GetEmpireProperty (iOwner, SystemEmpireData::AlienKey, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                iAlienKey = vTemp.GetInteger();
                
                GetAlienButtonString (
                    iAlienKey, 
                    iOwner, 
                    iOwner == iEmpireKey, 
                    iPlanetKey, 
                    iProxyKey, 
                    strAltTag,
                    NULL,
                    &strPlanetString
                    );
                
                if (iOwner == iEmpireKey) {
                    pszColor = bMapColoring ? m_vGoodColor.GetCharPtr() : NULL;
                } else {
                    
                    if (!bMapColoring) {
                        pszColor = NULL;
                    } else {
                        
                        iErrCode = g_pGameEngine->GetDiplomaticStatus (
                            iGameClass,
                            iGameNumber,
                            iEmpireKey,
                            iOwner,
                            &iWeOffer,
                            &iTheyOffer,
                            &iCurrent
                            );
                        
                        if (iErrCode == ERROR_DATA_NOT_FOUND) {
                            iCurrent = WAR;
                        }
                        
                        else if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        switch (iCurrent) {
                            
                        case WAR:
                            pszColor = m_vBadColor.GetCharPtr();
                            break;
                        case ALLIANCE:
                            pszColor = m_vGoodColor.GetCharPtr();
                            break;
                        default:
                            pszColor = NULL;
                            break;
                        }
                    }
                }
                
                break;              
            }
        }
        
        if (HTMLFilter (pvPlanetData[GameMap::Name].GetCharPtr(), &strFilter, 0, false) != OK) {
            strFilter.Clear();
        }
        
        // Put planet on grid, record planet index
        ppstrGrid[iGridLocX][iGridLocY] =
            
            "<td><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\"><tr><td align=\"center\">"\
            "<font size=\"1\"";
        
        if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        // Planet name
        ppstrGrid[iGridLocX][iGridLocY] += ">"; 
        ppstrGrid[iGridLocX][iGridLocY] += pvPlanetData[GameMap::Minerals].GetInteger();
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td><td rowspan=\"3\">";
        
        ppstrGrid[iGridLocX][iGridLocY] += strPlanetString;
        
        
        // Write planet
        ppstrGrid[iGridLocX][iGridLocY] += "</td><td align=\"center\"><font size=\"1\"";
        
        if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += ">";
        ppstrGrid[iGridLocX][iGridLocY] += pvPlanetData[GameMap::Fuel].GetInteger();
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr><tr><td align=\"center\"><font size=\"1\"";
        
        if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += ">";
        ppstrGrid[iGridLocX][iGridLocY] += pvPlanetData[GameMap::Ag].GetInteger();
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td><td align=\"center\"><font size=\"1\"";
        
        if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += ">";
        ppstrGrid[iGridLocX][iGridLocY] += pvPlanetData[GameMap::Pop].GetInteger();
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr><tr><td align=\"center\"><font size=\"1\"";
        
        if (bShipColoring) {
            
            if (iNumOwnShips > 0) {
                
                ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
                ppstrGrid[iGridLocX][iGridLocY] += m_vGoodColor.GetCharPtr();
                ppstrGrid[iGridLocX][iGridLocY] += "\"";
            }
        }
        
        else if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += ">";
        
        if (bHighlightShips) {
            
            if (iNumOwnShips > 0) {
                ppstrGrid[iGridLocX][iGridLocY] += "<font size=\"2\">(<strong>";
                ppstrGrid[iGridLocX][iGridLocY] += iNumOwnShips;
                ppstrGrid[iGridLocX][iGridLocY] += "</strong>)</font>";
            } else {
                ppstrGrid[iGridLocX][iGridLocY] += "(0)";
            }
            
        } else {
            
            if (iNumOwnShips > 0) {
                ppstrGrid[iGridLocX][iGridLocY] += "(";
                ppstrGrid[iGridLocX][iGridLocY] += iNumOwnShips;
                ppstrGrid[iGridLocX][iGridLocY] += ")";
            } else {
                ppstrGrid[iGridLocX][iGridLocY] += "(0)";
            }
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td><td align=\"center\"><font size=\"1\"";
        
        if (bShipColoring) {
            
            iErrCode = g_pGameEngine->GetLowestDiplomacyLevelForShipsOnPlanet (
                iGameClass,
                iGameNumber,
                iEmpireKey,
                iPlanetKey,
                bVisible,
                pvEmpireKey,
                iNumEmpires,
                &iAccountingNumOtherShips,
                &iDiplomacyLevel,
                &pvEmpireKey
                );
            
            Assert (iAccountingNumOtherShips == iNumOtherShips);
            
            if (iAccountingNumOtherShips > 0) {
                
                ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
                
                switch (iDiplomacyLevel) {
                    
                case WAR:
                    ppstrGrid[iGridLocX][iGridLocY] += m_vBadColor.GetCharPtr();
                    break;
                case ALLIANCE:
                    ppstrGrid[iGridLocX][iGridLocY] += m_vGoodColor.GetCharPtr();
                    break;
                default:
                    ppstrGrid[iGridLocX][iGridLocY] += m_vTextColor.GetCharPtr();
                    break;
                }
                
                ppstrGrid[iGridLocX][iGridLocY] += "\"";
            }
        }
        
        else if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += ">";
        
        
        if (bHighlightShips) {
            
            if (iNumOtherShips > 0) {
                ppstrGrid[iGridLocX][iGridLocY] += "<font size=\"2\">(<strong>";
                ppstrGrid[iGridLocX][iGridLocY] += iNumOtherShips;
                ppstrGrid[iGridLocX][iGridLocY] += "</strong>)</font>";
            } else {
                ppstrGrid[iGridLocX][iGridLocY] += "(0)";
            }
            
        } else {
            
            if (iNumOtherShips > 0) {
                ppstrGrid[iGridLocX][iGridLocY] += "(";
                ppstrGrid[iGridLocX][iGridLocY] += iNumOtherShips;
                ppstrGrid[iGridLocX][iGridLocY] += ")";
            } else {
                ppstrGrid[iGridLocX][iGridLocY] += "(0)";
            }
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr><tr><td colspan=\"3\" align=\"center\">"\
            "<font size=\"1\" face=\"" DEFAULT_PLANET_NAME_FONT "\"";
        
        if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += ">";
        ppstrGrid[iGridLocX][iGridLocY] += strFilter;
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr></table></td>";
        
        // Place links on grid
        if (iNumJumps > 0) {
            
            if (bLinkNorth) {
                ppstrGrid [iGridLocX][iGridLocY - 1] = pszVert;
                
                if (iGridLocY > 3) {
                    if (!ppstrGrid [iGridLocX][iGridLocY - 3].IsBlank()) {
                        ppstrGrid [iGridLocX][iGridLocY - 2] = pszVert;
                    }
                }
                iNumJumps --;
            }
            
            if (bLinkEast) {
                ppstrGrid [iGridLocX + 1][iGridLocY] = pszHorz;
                if (iGridLocX < iGridX - 3) {
                    if (!ppstrGrid [iGridLocX + 3][iGridLocY].IsBlank()) {
                        ppstrGrid [iGridLocX + 2][iGridLocY] = pszHorz;
                    }
                }
                iNumJumps --;
            }
            
            if (bLinkSouth) {
                ppstrGrid [iGridLocX][iGridLocY + 1] = pszVert;
                if (iGridLocY < iGridY - 3) {
                    if (!ppstrGrid [iGridLocX][iGridLocY + 3].IsBlank()) {
                        ppstrGrid [iGridLocX][iGridLocY + 2] = pszVert;
                    }
                }
                iNumJumps --;
            }
            
            if (bLinkWest) {
                ppstrGrid [iGridLocX - 1][iGridLocY] = pszHorz;
                if (iGridLocX > 3) {
                    if (!ppstrGrid [iGridLocX - 3][iGridLocY].IsBlank()) {
                        ppstrGrid [iGridLocX - 2][iGridLocY] = pszHorz;
                    }
                }
            }
        }
        
        pDatabase->FreeData (pvPlanetData);
        pvPlanetData = NULL;
    }
    
    // Write out map
    OutputText ("<table cellpadding=\"0\" cellspacing=\"0\">"); 
    
    // Parse grid
    for (j = 0; j < iGridY; j ++) {
        
        OutputText ("<tr>");
        
        for (i = 0; i < iGridX; i ++) {
            
            if (ppstrGrid[i][j].IsBlank()) {
                
                if (i % 3 == 1 && j % 3 == 1) { 
                    OutputText ("<td width=\"50\" height=\"50\">&nbsp;</td>");
                } else {
                    OutputText ("<td></td>");
                }
            } else {
                m_pHttpResponse->WriteText (ppstrGrid[i][j]);
            }
        }
        OutputText ("</tr>");
    }
    
    OutputText ("</table>");
    
    
Cleanup:

    if (!bAdmin && !bSpectators) {
        
        if (pvPlanetKey != pvEasyWayOut) {
            pDatabase->FreeData (pvPlanetKey);
        }

        if (piProxyKey != piEasyProxyKeys) {
            pDatabase->FreeKeys (piProxyKey);
        }
        
    } else {
            
        if (piPlanetKey != NULL) {
            pDatabase->FreeKeys (piPlanetKey);
        }
    }
    
    if (pvEmpireKey != NULL) {
        pDatabase->FreeData (pvEmpireKey);
    }
    
    if (pvPlanetData != NULL) {
        pDatabase->FreeData (pvPlanetData);
    }
    
    if (pstrGrid != NULL) {
        delete [] pstrGrid;
    }

    pDatabase->Release();
    
    return iErrCode;
}