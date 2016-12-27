//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "../MapGen/Dijkstra.h"

int HtmlRenderer::RenderMap (int iGameClass, int iGameNumber, int iEmpireKey, bool bAdmin, const PartialMapInfo* pPartialMapInfo, bool bSpectators)
{
    Variant* pvPlanetKey = NULL, * pvFreePlanetKey = NULL, * pvEmpireKey = NULL, vTemp, pvEasyWayOut[9], * pvPlanetData = NULL;
    AutoFreeData free_pvPlanetKey(pvFreePlanetKey);
    AutoFreeData free_pvEmpireKey(pvEmpireKey);
    AutoFreeData free_pvPlanetData(pvPlanetData);
    
    int iErrCode, iMinX, iMaxX, iMinY, iMaxY, iNumJumps, iMapMinX = 0, iMapMaxX = 0, iMapMinY = 0, iMapMaxY = 0;
    
    unsigned int iNumPlanets, * piPlanetKey = NULL, i, j, * piProxyKey = NULL, * piFreeProxyKey = NULL, iLivePlanetKey, iDeadPlanetKey;
    AutoFreeKeys free_piPlanetKey(piPlanetKey);
    AutoFreeKeys free_piProxyKey(piFreeProxyKey);

    size_t stTemp;
    
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    unsigned int piEasyProxyKeys[9], iGridX, iGridY, iGridLocX, iGridLocY, iNumEmpires;

    int iHorzKey, iVertKey, iNumOwnShips, iNumOtherShips, iAlienKey, iAccountingNumOtherShips, iDiplomacyLevel,
        iCenterX = MAX_COORDINATE, iCenterY = MAX_COORDINATE, iNumUncloakedShips, iNumCloakedShips, 
        iNumUncloakedBuildShips, iNumCloakedBuildShips, iWeOffer, iTheyOffer, iCurrent, iX, iY, iOwner, iLink, 
        iProxyKey, iPlanetKey;
    
    bool bPartialMapShortcut = false;
    
    String* pstrGrid = NULL, ** ppstrGrid, strPlanetString, strImage, strHorz, strVert, strFilter, strAltTag;
    Algorithm::AutoDelete<String> free_pstrGrid(pstrGrid, true);

    char* pszHorz, * pszVert;
    const char* pszColor;

    bool bLinkNorth, bLinkEast, bLinkSouth, bLinkWest, bVisible, bIndependence, bSensitive, bMapColoring, bShipColoring, bHighlightShips;
    
    Assert(!(bSpectators && bAdmin));

    if (!bAdmin && !bSpectators) {

        int iOptions = m_iGameOptions;

        bSensitive = (iOptions & SENSITIVE_MAPS) != 0;
        bMapColoring = (iOptions & MAP_COLORING) != 0;
        bShipColoring = (iOptions & SHIP_MAP_COLORING) != 0;
        bHighlightShips = (iOptions & SHIP_MAP_HIGHLIGHTING) != 0;
        
        // Get map geography information
        iErrCode = GetMapLimits(iGameClass, iGameNumber, iEmpireKey, &iMapMinX, &iMapMaxX, &iMapMinY, &iMapMaxY);
        RETURN_ON_ERROR(iErrCode);

        if (pPartialMapInfo != NULL && 
            pPartialMapInfo->iCenterKey != PARTIAL_MAP_NATURAL_CENTER && 
            pPartialMapInfo->iXRadius == 1 && 
            pPartialMapInfo->iYRadius == 1
            ) {
            
            bPartialMapShortcut = true;
            
            // Scan 8 surrounding planets
            iErrCode = GetVisitedSurroundingPlanetKeys (
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
            
            RETURN_ON_ERROR(iErrCode);
            
            Assert(iNumPlanets > 0);
            pvPlanetKey = pvEasyWayOut;
            piProxyKey = piEasyProxyKeys;
           
        } else {
            
            iErrCode = GetVisitedPlanetKeys(iGameClass, iGameNumber, iEmpireKey, &pvPlanetKey, &piProxyKey, &iNumPlanets);
            RETURN_ON_ERROR(iErrCode);

            Assert(iNumPlanets > 0);
            pvFreePlanetKey = pvPlanetKey;
            piFreeProxyKey = piProxyKey;
            
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
        iErrCode = GetMapLimits(iGameClass, iGameNumber,&iMapMinX, &iMapMaxX, &iMapMinY, &iMapMaxY);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->GetAllKeys(strGameMap, &piPlanetKey, &iNumPlanets);
        RETURN_ON_ERROR(iErrCode);
        
        iMinX = iMapMinX;
        iMaxX = iMapMaxX; 
        iMinY = iMapMinY;
        iMaxY = iMapMaxY;
    }
    
    // Partial map menus
    if (pPartialMapInfo != NULL) {

        Assert(!bSpectators);
        
        if (!bPartialMapShortcut) {
            
            // Get radius data
            if (pPartialMapInfo->iCenterKey == PARTIAL_MAP_NATURAL_CENTER) {
                
                iCenterX = iMapMinX + (iMapMaxX - iMapMinX) / 2;
                iCenterY = iMapMinY + (iMapMaxY - iMapMinY) / 2;
                
            } else {
                
                iErrCode = GetPlanetCoordinates(iGameClass, iGameNumber, pPartialMapInfo->iCenterKey, &iCenterX, &iCenterY);
                RETURN_ON_ERROR(iErrCode);
            }
            
            if (pPartialMapInfo->iXRadius != PARTIAL_MAP_UNLIMITED_RADIUS) {
                iMinX = iCenterX - pPartialMapInfo->iXRadius;
                iMaxX = iCenterX + pPartialMapInfo->iXRadius;
                Assert(iMinX >= 0);
            }
            
            if (pPartialMapInfo->iYRadius != PARTIAL_MAP_UNLIMITED_RADIUS) {
                iMinY = iCenterY - pPartialMapInfo->iYRadius;
                iMaxY = iCenterY + pPartialMapInfo->iYRadius;
                Assert(iMinY >= 0);
            }
        }
        
        // Draw table
        if (!pPartialMapInfo->bDontShowPartialOptions) {
            
            unsigned int iValue, iMaxXRadius, iMaxYRadius;
            
            Assert(iCenterX != MAX_COORDINATE);
            Assert(iCenterY != MAX_COORDINATE);
            
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
                
                iErrCode = GetPlanetNameWithCoordinates(strGameMap, pvPlanetKey[i].GetInteger(), pszName);
                RETURN_ON_ERROR(iErrCode);
                
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
    Assert(pstrGrid);
    ppstrGrid = (String**) StackAlloc (iGridX * sizeof (String*));
    
    for (i = 0; i < iGridX; i ++) {
        ppstrGrid[i] = pstrGrid + i * iGridY;
    }
    
    int iLivePlanetAddress, iDeadPlanetAddress;
    if (m_iThemeKey == INDIVIDUAL_ELEMENTS) {
        
        Variant vValue;

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIHorz, &vValue);
        RETURN_ON_ERROR(iErrCode);
        iHorzKey = vValue.GetInteger();

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIVert, &vValue);
        RETURN_ON_ERROR(iErrCode);
        iVertKey = vValue.GetInteger();

        iErrCode = GetEmpirePlanetIcons(m_iEmpireKey, &iLivePlanetKey, &iLivePlanetAddress, &iDeadPlanetKey, &iDeadPlanetAddress);
        RETURN_ON_ERROR(iErrCode);
        
    } else {
        
        iHorzKey = iVertKey = iLivePlanetKey = iDeadPlanetKey = m_iThemeKey;

        iErrCode = GetThemeAddress(iLivePlanetKey, &iLivePlanetAddress);
        RETURN_ON_ERROR(iErrCode);
        iErrCode = GetThemeAddress(iDeadPlanetKey, &iDeadPlanetAddress);
        RETURN_ON_ERROR(iErrCode);
    }
    
    int iHorzAddress, iVertAddress;
    iErrCode = GetThemeAddress(iHorzKey, &iHorzAddress);
    RETURN_ON_ERROR(iErrCode);
    iErrCode = GetThemeAddress(iVertKey, &iVertAddress);
    RETURN_ON_ERROR(iErrCode);

    GetHorzString(iHorzKey, iHorzAddress, &strHorz, false);
    GetVertString(iVertKey, iVertAddress, &strVert, false);
    
    pszHorz = (char*) StackAlloc (strHorz.GetLength() + 100);
    pszVert = (char*) StackAlloc (strVert.GetLength() + 100);
    
    //sprintf(pszHorz, "<td align=\"center\">%s</td>", strHorz.GetCharPtr());
    //sprintf(pszVert, "<td align=\"center\">%s</td>", strVert.GetCharPtr());
    
    stTemp = sizeof ("<td align=\"center\">") - 1;
    
    memcpy (pszHorz, "<td align=\"center\">", stTemp);
    memcpy (pszVert, "<td align=\"center\">", stTemp);
    
    memcpy (pszHorz + stTemp, strHorz.GetCharPtr(), strHorz.GetLength());
    memcpy (pszVert + stTemp, strVert.GetCharPtr(), strVert.GetLength());
    
    memcpy (pszHorz + stTemp + strHorz.GetLength(), "</td>", sizeof ("</td>"));
    memcpy (pszVert + stTemp + strVert.GetLength(), "</td>", sizeof ("</td>"));
    
    int iOptions;
    iErrCode = GetGameClassOptions(iGameClass, &iOptions);
    RETURN_ON_ERROR(iErrCode);
    
    bVisible = (iOptions & VISIBLE_BUILDS) != 0;
    bIndependence = (iOptions & INDEPENDENCE) != 0;

    // Loop through all planets, placing them on grid
    for (i = 0; i < iNumPlanets; i ++) {
        
        if (!bAdmin && !bSpectators) {
            
            iPlanetKey = pvPlanetKey[i].GetInteger();
            iProxyKey = piProxyKey[i];
            
            if (pvPlanetData)
            {
                t_pCache->FreeData (pvPlanetData);
                pvPlanetData = NULL;
            }

            iErrCode = t_pCache->ReadRow (strGameMap, iPlanetKey, &pvPlanetData);
            RETURN_ON_ERROR(iErrCode);
            
            GetCoordinates(pvPlanetData[GameMap::iCoordinates].GetCharPtr(), &iX, &iY);
            
            // Partial map filtering
            if (pPartialMapInfo != NULL && (iX > iMaxX || iX < iMinX || iY > iMaxY || iY < iMinY))
            {
                continue;
            }
            
            iErrCode = t_pCache->ReadData(strGameEmpireMap, iProxyKey, GameEmpireMap::NumUncloakedShips, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            iNumUncloakedShips = vTemp.GetInteger();
            
            iErrCode = t_pCache->ReadData(strGameEmpireMap, iProxyKey, GameEmpireMap::NumCloakedShips, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            iNumCloakedShips = vTemp.GetInteger();
            
            iErrCode = t_pCache->ReadData(strGameEmpireMap, iProxyKey, GameEmpireMap::NumUncloakedBuildShips, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            iNumUncloakedBuildShips = vTemp.GetInteger();
            
            iErrCode = t_pCache->ReadData(strGameEmpireMap, iProxyKey, GameEmpireMap::NumCloakedBuildShips, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            iNumCloakedBuildShips = vTemp.GetInteger();

            iNumOwnShips = iNumUncloakedShips + iNumCloakedShips + iNumUncloakedBuildShips + iNumCloakedBuildShips;
            
        } else {
            
            iPlanetKey = piPlanetKey[i];
            iProxyKey = 0;
            
            iErrCode = t_pCache->ReadRow (strGameMap, iPlanetKey, &pvPlanetData);
            RETURN_ON_ERROR(iErrCode);
            
            GetCoordinates (pvPlanetData[GameMap::iCoordinates].GetCharPtr(), &iX, &iY);
            
            iNumOwnShips = iNumUncloakedShips = iNumCloakedShips = iNumUncloakedBuildShips = iNumCloakedBuildShips = 0;
        }
        
        // Main map loop
        iNumOtherShips = pvPlanetData[GameMap::iNumUncloakedShips].GetInteger() - iNumUncloakedShips;
        if (bVisible || bAdmin) {
            iNumOtherShips += pvPlanetData[GameMap::iNumUncloakedBuildShips].GetInteger() - iNumUncloakedBuildShips;
        }
        
        Assert(iNumOtherShips >= 0 && iNumOwnShips >= 0);
        
        iLink = pvPlanetData[GameMap::iLink].GetInteger();

        bLinkNorth = (iLink & LINK_NORTH) != 0;
        bLinkEast  = (iLink & LINK_EAST) != 0;
        bLinkSouth = (iLink & LINK_SOUTH) != 0;
        bLinkWest  = (iLink & LINK_WEST) != 0;
        
        iNumJumps = (bLinkNorth ? 1:0) + (bLinkEast ? 1:0) + (bLinkSouth ? 1:0) + (bLinkWest ? 1:0);
        iOwner = pvPlanetData[GameMap::iOwner].GetInteger();
        
        // Get grid coordinates     
        iGridLocX = (iX - iMinX) * 3 + 1;
        iGridLocY = (iMaxY - iY) * 3 + 1;
        
        Assert(iGridLocX >= 0 && iGridLocY >= 0 && iGridLocX < iGridX && iGridLocY < iGridY);
        
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
            
            RETURN_ON_ERROR(iErrCode);
        }
        
        // Get planet string        
        if (pvPlanetData[GameMap::iAnnihilated].GetInteger() != NOT_ANNIHILATED)
        {
            GetDeadPlanetButtonString (iDeadPlanetKey, iDeadPlanetAddress, iPlanetKey, iProxyKey, strAltTag, NULL, &strPlanetString);
            pszColor = NULL;
        }
        else
        {
            switch (iOwner)
            {
            case SYSTEM:
                GetLivePlanetButtonString(iLivePlanetKey, iLivePlanetAddress, iPlanetKey, iProxyKey, strAltTag, NULL, &strPlanetString);
                pszColor = NULL;
                break;
                
            case INDEPENDENT:
                GetIndependentPlanetButtonString(iPlanetKey, iProxyKey, strAltTag, NULL, &strPlanetString);
                pszColor = bMapColoring ? m_vBadColor.GetCharPtr() : NULL;
                break;
                
            default:
                iErrCode = GetEmpireProperty(iOwner, SystemEmpireData::AlienKey, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                iAlienKey = vTemp.GetInteger();

                iErrCode = GetEmpireProperty(iOwner, SystemEmpireData::AlienAddress, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                int iAlienAddress = vTemp.GetInteger();
                
                iErrCode = GetAlienPlanetButtonString(
                    iAlienKey,
                    iAlienAddress,
                    iOwner, 
                    iOwner == iEmpireKey, 
                    iPlanetKey, 
                    iProxyKey, 
                    strAltTag,
                    NULL,
                    &strPlanetString
                    );
                RETURN_ON_ERROR(iErrCode);
                
                if (iOwner == iEmpireKey) {
                    pszColor = bMapColoring ? m_vGoodColor.GetCharPtr() : NULL;
                } else {
                    
                    if (!bMapColoring) {
                        pszColor = NULL;
                    } else {
                        
                        iErrCode = GetVisibleDiplomaticStatus(
                            iGameClass,
                            iGameNumber,
                            iEmpireKey,
                            iOwner,
                            &iWeOffer,
                            &iTheyOffer,
                            &iCurrent,
                            NULL
                            );
                        RETURN_ON_ERROR(iErrCode);

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
        
        HTMLFilter (pvPlanetData[GameMap::iName].GetCharPtr(), &strFilter, 0, false);
        
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
        ppstrGrid[iGridLocX][iGridLocY] += pvPlanetData[GameMap::iMinerals].GetInteger();
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
        ppstrGrid[iGridLocX][iGridLocY] += pvPlanetData[GameMap::iFuel].GetInteger();
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr><tr><td align=\"center\"><font size=\"1\"";
        
        if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += ">";
        ppstrGrid[iGridLocX][iGridLocY] += pvPlanetData[GameMap::iAg].GetInteger();
        ppstrGrid[iGridLocX][iGridLocY] += "</font></td><td align=\"center\"><font size=\"1\"";
        
        if (pszColor != NULL) {
            ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
            ppstrGrid[iGridLocX][iGridLocY] += pszColor;
            ppstrGrid[iGridLocX][iGridLocY] += "\"";
        }
        
        ppstrGrid[iGridLocX][iGridLocY] += ">";
        ppstrGrid[iGridLocX][iGridLocY] += pvPlanetData[GameMap::iPop].GetInteger();
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
        
        if (bShipColoring)
        {
            iErrCode = GetLowestDiplomacyLevelForShipsOnPlanet(
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

            RETURN_ON_ERROR(iErrCode);
            Assert(iAccountingNumOtherShips == iNumOtherShips);
            
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
    
    return iErrCode;
}

int HtmlRenderer::GetAlienPlanetButtonString(unsigned int iAlienKey, int iAlienAddress, unsigned int iEmpireKey, bool bBorder, int iPlanetKey, 
                                             int iProxyKey, const char* pszAlt, const char* pszExtraTag, String* pstrAlienButtonString)
{
    int iErrCode = OK;

    *pstrAlienButtonString = "<input type=\"image\" border=\"";
    *pstrAlienButtonString += (bBorder ? 1:0);
    *pstrAlienButtonString += "\" src=\"" BASE_RESOURCE_DIR;
    
    if (iAlienKey == UPLOADED_ICON)
    {
        if (m_iSystemOptions2 & BLOCK_UPLOADED_ICONS)
        {
            Variant vAddress;
            iErrCode = GetSystemProperty(SystemData::DefaultAlienAddress, &vAddress);
            RETURN_ON_ERROR(iErrCode);

            *pstrAlienButtonString += BASE_ALIEN_DIR ALIEN_NAME;
            *pstrAlienButtonString += vAddress.GetInteger();
        }
        else
        {
            *pstrAlienButtonString += BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME;
            *pstrAlienButtonString += iEmpireKey;
        }

    }
    else
    {
        *pstrAlienButtonString += BASE_ALIEN_DIR ALIEN_NAME;
        *pstrAlienButtonString += iAlienAddress;
    }
    
    *pstrAlienButtonString += DEFAULT_IMAGE_EXTENSION "\" name=\"Planet";
    *pstrAlienButtonString += iPlanetKey;
    *pstrAlienButtonString += ".";
    *pstrAlienButtonString += iProxyKey;
    
    *pstrAlienButtonString += "\"";
    
    if (!String::IsBlank (pszAlt))
    {
        *pstrAlienButtonString += " alt=\"";
        *pstrAlienButtonString += pszAlt;
        *pstrAlienButtonString += "\"";
    }

    if (pszExtraTag != NULL)
    {
        *pstrAlienButtonString += " ";
        *pstrAlienButtonString += pszExtraTag;
    }
    
    *pstrAlienButtonString += ">";
    return iErrCode;
}

void HtmlRenderer::WriteAlienButtonString(unsigned int iAlienKey, int iAddress, bool bBorder, const char* pszNamePrefix, const char* pszAuthorName) {
    
    OutputText ("<input type=\"image\" border=\"");
    m_pHttpResponse->WriteText (bBorder ? 1:0);
    OutputText ("\" src=\"" BASE_RESOURCE_DIR);
    OutputText (BASE_ALIEN_DIR ALIEN_NAME);
    m_pHttpResponse->WriteText(iAddress);
    OutputText(DEFAULT_IMAGE_EXTENSION "\" name=\"");
    m_pHttpResponse->WriteText(pszNamePrefix);
    m_pHttpResponse->WriteText(iAlienKey);
    OutputText ("\" alt=\"Alien ");
    m_pHttpResponse->WriteText(iAddress);
    OutputText (" by ");
    m_pHttpResponse->WriteText(pszAuthorName);
    OutputText ("\">");
}

void HtmlRenderer::GetLivePlanetButtonString (unsigned int iLivePlanetKey, int iLivePlanetAddress, int iPlanetKey, int iProxyKey, 
                                              const char* pszAlt, const char* pszExtraTag,
                                              String* pstrLivePlanet) {
    
    switch (iLivePlanetKey)
    {
    case NULL_THEME:
        *pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR LIVE_PLANET_NAME "\" name=\"Planet";
        break;
        
    case ALTERNATIVE_PATH:
        *pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"";
        *pstrLivePlanet += m_vLocalPath.GetCharPtr();
        *pstrLivePlanet += "/" LIVE_PLANET_NAME "\" name=\"Planet";
        break;
        
    default:
        *pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR;
        *pstrLivePlanet += iLivePlanetAddress;
        *pstrLivePlanet += "/" LIVE_PLANET_NAME "\" name=\"Planet";
        break;
    }
    
    *pstrLivePlanet += iPlanetKey;
    *pstrLivePlanet += ".";
    *pstrLivePlanet += iProxyKey;
    *pstrLivePlanet += "\"";
    
    if (!String::IsBlank (pszAlt)) {
        
        *pstrLivePlanet += " alt=\"";
        *pstrLivePlanet += pszAlt;
        *pstrLivePlanet += "\"";
    }

    if (pszExtraTag != NULL) {
        *pstrLivePlanet += " ";
        *pstrLivePlanet += pszExtraTag;
    }
    
    *pstrLivePlanet += ">";
}

void HtmlRenderer::GetDeadPlanetButtonString(unsigned int iDeadPlanetKey, int iDeadPlanetAddress, int iPlanetKey, int iProxyKey, 
                                             const char* pszAlt, const char* pszExtraTag, String* pstrDeadPlanet)
{
    switch (iDeadPlanetKey)
    {
    case NULL_THEME:
        *pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR DEAD_PLANET_NAME "\" name=\"Planet";
        break;
        
    case ALTERNATIVE_PATH:
        *pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"";
        *pstrDeadPlanet += m_vLocalPath.GetCharPtr();
        *pstrDeadPlanet += "/" DEAD_PLANET_NAME "\" name=\"Planet";
        break;
        
    default:
        *pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR;
        *pstrDeadPlanet += iDeadPlanetAddress;
        *pstrDeadPlanet += "/" DEAD_PLANET_NAME "\" name=\"Planet";
        break;
    }
    
    *pstrDeadPlanet += iPlanetKey;
    *pstrDeadPlanet += ".";
    *pstrDeadPlanet += iProxyKey;
    *pstrDeadPlanet += "\"";
    
    if (!String::IsBlank (pszAlt)) {
        
        *pstrDeadPlanet += " alt=\"";
        *pstrDeadPlanet += pszAlt;
        *pstrDeadPlanet += "\"";
    }

    if (pszExtraTag != NULL) {
        *pstrDeadPlanet += " ";
        *pstrDeadPlanet += pszExtraTag;
    }
    
    *pstrDeadPlanet += ">";
}

void HtmlRenderer::GetIndependentPlanetButtonString (int iPlanetKey, int iProxyKey, const char* pszAlt, 
                                                     const char* pszExtraTag, String* pstrPlanetString) {
    
    *pstrPlanetString = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR INDEPENDENT_PLANET_NAME "\" name=\"Planet";
    *pstrPlanetString += iPlanetKey;
    *pstrPlanetString += ".";
    *pstrPlanetString += iProxyKey;
    *pstrPlanetString += "\"";
    
    if (!String::IsBlank (pszAlt)) {
        
        *pstrPlanetString += " alt=\"";
        *pstrPlanetString += pszAlt;
        *pstrPlanetString += "\"";
    }

    if (pszExtraTag != NULL) {
        *pstrPlanetString += " ";
        *pstrPlanetString += pszExtraTag;
    }
    
    *pstrPlanetString += ">";
}

void HtmlRenderer::WriteLivePlanetString(unsigned int iLivePlanetKey, int iLivePlanetAddress)
{
    switch (iLivePlanetKey)
    {
    case NULL_THEME:
        OutputText ("<img src=\"" BASE_RESOURCE_DIR LIVE_PLANET_NAME "\">");
        break;
        
    case ALTERNATIVE_PATH:
        OutputText ("<img src=\"");
        m_pHttpResponse->WriteText(m_vLocalPath.GetCharPtr());
        OutputText ("/" LIVE_PLANET_NAME "\">");
        break;
        
    default:
        OutputText ("<img src=\"" BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText(iLivePlanetAddress);
        OutputText ("/" LIVE_PLANET_NAME "\">");
        break;
    }
}

void HtmlRenderer::WriteDeadPlanetString(unsigned int iDeadPlanetKey, int iDeadPlanetAddress)
{
    switch (iDeadPlanetKey)
    {
    case NULL_THEME:
        OutputText ("<img src=\"" BASE_RESOURCE_DIR DEAD_PLANET_NAME "\">");
        break;
        
    case ALTERNATIVE_PATH:
        OutputText ("<img src=\"");
        m_pHttpResponse->WriteText(m_vLocalPath.GetCharPtr());
        OutputText ("/" DEAD_PLANET_NAME "\">");
        break;
        
    default:
        OutputText ("<img src=\"" BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText(iDeadPlanetAddress);
        OutputText ("/" DEAD_PLANET_NAME "\">");
        break;
    }
}

void HtmlRenderer::WriteIndependentPlanetString() {
    
    OutputText ("<img src=\"" BASE_RESOURCE_DIR INDEPENDENT_PLANET_NAME "\">");
}

int HtmlRenderer::WriteUpClosePlanetString (unsigned int iEmpireKey, int iPlanetKey, int iProxyPlanetKey, 
                                            unsigned int iLivePlanetKey, int iLivePlanetAddress, unsigned int iDeadPlanetKey, int iDeadPlanetAddress,
                                            int iPlanetCounter, bool bVisibleBuilds, int iGoodAg, int iBadAg, 
                                            int iGoodMin, int iBadMin, int iGoodFuel, int iBadFuel, 
                                            float fEmpireAgRatio, bool bIndependence, 
                                            bool bAdmin, bool bSpectator, const Variant* pvPlanetData,
                                            bool* pbOurPlanet) {
    
    int iErrCode = OK, i;
    unsigned int j;

    bool bMapColoring = !bAdmin && !bSpectator; 
    const char* pszTableColor = m_vTableColor.GetCharPtr();
    
    String strFilter;
    
    if (!(m_iGameState & GAME_MAP_GENERATED)) {
        
        // Lay down the default "planet lost in outer space" table
        OutputText ("<tr><th>&nbsp;</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Name</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Location</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Owner</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Min</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Fuel</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Ag</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Pop</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Max Pop</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Next Pop</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\" align=\"left\">Jumps</th></tr><tr><td align=\"center\">");
        
        iErrCode = WriteProfileAlienString (
            m_iAlienKey,
            m_iAlienAddress,
            iEmpireKey,
            m_vEmpireName.GetCharPtr(),
            0, 
            "ProfileLink",
            "View your profile",
            false,
            false
            );
        RETURN_ON_ERROR(iErrCode);
        
        OutputText ("</td><td align=\"center\"><strong>");
        m_pHttpResponse->WriteText (m_vEmpireName.GetCharPtr());    // Name
        OutputText ("</strong></td><td align=\"center\">None yet</td><td align=\"center\"><strong>");
        m_pHttpResponse->WriteText (m_vEmpireName.GetCharPtr());
        OutputText ("</strong></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"left\">None yet</td></tr>");
        
        return OK;
    }
    
    OutputText ("<tr><th>&nbsp;</th><th align=\"left\" bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Name</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Location</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Owner</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Min</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Fuel</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Ag</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Pop</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Max Pop</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Next Pop</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Jumps</th></tr><tr><td align=\"center\">");
    
    int iData, iWeOffer, iTheyOffer, iCurrent, iAlienKey;
    unsigned int iAnnihilated = pvPlanetData[GameMap::iAnnihilated].GetInteger();
    unsigned int iOwner = pvPlanetData[GameMap::iOwner].GetInteger();

    String strPlanet;
    Variant vEmpireName;
    
    if (iOwner == SYSTEM) {
        
        iCurrent = TRUCE;
        
        if (iAnnihilated == 0) {
            WriteLivePlanetString(iLivePlanetKey, iLivePlanetAddress);
        } else {
            WriteDeadPlanetString(iDeadPlanetKey, iDeadPlanetAddress);
        }

    } else {

        if (iOwner == INDEPENDENT) {
            
            iCurrent = bMapColoring ? WAR : TRUCE;
            WriteIndependentPlanetString();
            
        } else {
            
            if (iOwner == iEmpireKey) {
                
                iCurrent = bMapColoring ? ALLIANCE : TRUCE;
                
                iErrCode = WriteProfileAlienString (
                    m_iAlienKey,
                    m_iAlienAddress,
                    iEmpireKey,
                    m_vEmpireName.GetCharPtr(),
                    0, 
                    "ProfileLink",
                    "View your profile",
                    false,
                    false
                    );
                RETURN_ON_ERROR(iErrCode);
                
            } else {
                
                if (!bMapColoring) {
                    iCurrent = TRUCE;
                } else {
                    
                    iErrCode = GetVisibleDiplomaticStatus(
                        m_iGameClass, 
                        m_iGameNumber, 
                        iEmpireKey, 
                        iOwner, 
                        &iWeOffer, 
                        &iTheyOffer, 
                        &iCurrent,
                        NULL
                        );

                    RETURN_ON_ERROR(iErrCode);
                }
                
                Variant vTemp;

                iErrCode = GetEmpireProperty(iOwner, SystemEmpireData::AlienKey, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                iAlienKey = vTemp.GetInteger();

                iErrCode = GetEmpireProperty(iOwner, SystemEmpireData::AlienAddress, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                int iAlienAddress = vTemp.GetInteger();
                
                iErrCode = GetEmpireName(iOwner, &vEmpireName);
                RETURN_ON_ERROR(iErrCode);
                
                char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
                sprintf(pszProfile, "View the profile of %s", vEmpireName.GetCharPtr());
                
                iErrCode = WriteProfileAlienString (
                    iAlienKey,
                    iAlienAddress,
                    iOwner,
                    vEmpireName.GetCharPtr(),
                    0, 
                    "ProfileLink",
                    pszProfile,
                    false,
                    true
                    );
                RETURN_ON_ERROR(iErrCode);
                
                NotifyProfileLink();
            }
        }
    }
    
    OutputText ("</td>");
    
    HTMLFilter (pvPlanetData[GameMap::iName].GetCharPtr(), &strFilter, 0, false);
    
    if (iOwner != SYSTEM && iOwner == iEmpireKey && !bAdmin) {
        
        OutputText ("<td><input type=\"text\" size=\"15\" maxlength=\"");
        m_pHttpResponse->WriteText (MAX_PLANET_NAME_LENGTH);
        OutputText ("\" name=\"NewPlanetName");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\"value=\"");
        m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
        OutputText ("\"><input type=\"hidden\" name=\"OldPlanetName");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
        OutputText ("\"><input type=\"hidden\" name=\"OldMaxPop");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (pvPlanetData[GameMap::iMaxPop].GetInteger());
        OutputText ("\"></td>");

        OutputText ("<input type=\"hidden\" name=\"KeyPlanet");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (iPlanetKey);
        OutputText ("\">");
        
        *pbOurPlanet = true;
        
    } else {
        
        OutputText ("<td align=\"left\"><strong>");
        WriteStringByDiplomacy (strFilter.GetCharPtr(), iCurrent);
        OutputText ("</strong>");
        
        if (iAnnihilated != NOT_ANNIHILATED) {
            
            OutputText ("<br>(Quarantined <strong>");
            
            if (iAnnihilated == ANNIHILATED_FOREVER) {
                OutputText ("forever</strong>)");
            } else {
                
                m_pHttpResponse->WriteText (iAnnihilated);
                OutputText ("</strong> update");
                
                if (iAnnihilated == 1) {
                    OutputText (")");
                } else {
                    OutputText ("s)");
                }
            }
        }
        
        else if (pvPlanetData[GameMap::iHomeWorld].GetInteger() != HOMEWORLD &&
                 pvPlanetData[GameMap::iHomeWorld].GetInteger() != NOT_HOMEWORLD)
        {
            OutputText ("<br>(Surrendered)");
        }
        
        OutputText ("</td>");
        
        *pbOurPlanet = false;
    }
    
    // Coordinates
    OutputText ("<td align=\"center\">");
    WriteStringByDiplomacy (pvPlanetData[GameMap::iCoordinates].GetCharPtr(), iCurrent);
    OutputText ("</td><td align=\"center\">");
    
    // Owner name
    if (iOwner != SYSTEM) {
        
        Variant vEmpireName;
        const char* pszEmpireName;
        
        if (iOwner == iEmpireKey) {
            pszEmpireName = m_vEmpireName.GetCharPtr();
        } else {
            
            if (iOwner == INDEPENDENT) {
                pszEmpireName = INDEPENDENT_NAME;
            } else {
                
                iErrCode = GetEmpireName (iOwner, &vEmpireName);
                RETURN_ON_ERROR(iErrCode);
                pszEmpireName = vEmpireName.GetCharPtr();
            }
        }
        
        WriteStringByDiplomacy (pszEmpireName, iCurrent);
        
        OutputText ("</td>");
        
    } else {
        OutputText ("-</td>");
    }
    
    // Minerals
    OutputText ("<td align=\"center\">");
    
    iData = pvPlanetData[GameMap::iMinerals].GetInteger();
    if (iData < iBadMin) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (iData);
        OutputText ("</font>");
    } else {
        if (iData > iGoodMin) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iData);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (iData);
        }
    }
    OutputText ("</td><td align=\"center\">");
    
    iData = pvPlanetData[GameMap::iFuel].GetInteger();
    if (iData < iBadFuel) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (iData);
        OutputText ("</font>");
    } else {
        if (iData > iGoodFuel) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iData);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (iData);
        }
    }
    OutputText ("</td><td align=\"center\">");
    
    int iAg = pvPlanetData[GameMap::iAg].GetInteger();
    if (iAg < iBadAg) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (iAg);
        OutputText ("</font>");
    } else {
        if (iAg > iGoodAg) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iAg);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (iAg); 
        }
    }
    OutputText ("</td><td align=\"center\">");
    
    iData = pvPlanetData[GameMap::iPop].GetInteger();
    if (iData == iAg) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (iData);
        OutputText ("</font>");
    } else {
        if (iData > iAg || iData == 0) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iData);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (iData); 
        }
    }
    
    OutputText ("</td><td align=\"center\">");
    
    if (iOwner != SYSTEM && iOwner == iEmpireKey && !bAdmin) {
        
        OutputText ("<input type=\"text\" size=\"4\" maxlength=\"4\" name=\"NewMaxPop");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\"value=\"");
        m_pHttpResponse->WriteText (pvPlanetData[GameMap::iMaxPop].GetInteger());
        OutputText ("\">");
        
    } else {
        
        if (iOwner != SYSTEM && bAdmin) {
            m_pHttpResponse->WriteText (pvPlanetData[GameMap::iMaxPop].GetInteger());
        } else {
            OutputText ("-");
        }
    }
    OutputText ("</td><td align=\"center\">");
    
    if (iOwner == SYSTEM) {
        OutputText ("0");
    }
    
    else if (iOwner == iEmpireKey || bAdmin) {
        
        int iCost = pvPlanetData[GameMap::iPopLostToColonies].GetInteger();
        int iPop = pvPlanetData[GameMap::iPop].GetInteger();
        
        Assert(iCost >= 0 && iCost <= iPop);
        
        int iNextPop = GetNextPopulation(iPop - iCost, fEmpireAgRatio);
        if (iNextPop > pvPlanetData[GameMap::iMaxPop].GetInteger()) {
            iNextPop = pvPlanetData[GameMap::iMaxPop].GetInteger();
        }
        
        iData = iNextPop;
        if (iData == iAg) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iData);
            OutputText ("</font>");
        } else {
            if (iData > iAg || iData == 0) {
                OutputText ("<font color=\"");
                m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                OutputText ("\">");
                m_pHttpResponse->WriteText (iData);
                OutputText ("</font>");
            } else {
                m_pHttpResponse->WriteText (iData); 
            }
        }
    }
    
    else {
        
        OutputText ("-");
    }
    
    OutputText ("</td><td align=\"center\">");
    
    int iX, iY;
    GetCoordinates (pvPlanetData[GameMap::iCoordinates].GetCharPtr(), &iX, &iY);
    
    int iLink = pvPlanetData[GameMap::iLink].GetInteger();
    bool bLinkNorth = (iLink & LINK_NORTH) != 0;
    bool bLinkEast  = (iLink & LINK_EAST) != 0;
    bool bLinkSouth = (iLink & LINK_SOUTH) != 0;
    bool bLinkWest  = (iLink & LINK_WEST) != 0;
    
    int iNumJumps = (bLinkNorth ? 1:0) + (bLinkEast ? 1:0) + (bLinkSouth ? 1:0) + (bLinkWest ? 1:0);
    
    if (iNumJumps > 0) {
        
        int piJumpX [NUM_CARDINAL_POINTS];
        int piJumpY [NUM_CARDINAL_POINTS];
        int piCardinalPoint [NUM_CARDINAL_POINTS];
        
        int iNeighbourKey, iCounter = 0;
        
        if (bLinkNorth) {
            piJumpX[iCounter] = iX;
            piJumpY[iCounter] = iY + 1;
            piCardinalPoint [iCounter] = NORTH;
            iCounter ++;
        }
        
        if (bLinkEast) {
            piJumpX[iCounter] = iX + 1;
            piJumpY[iCounter] = iY;
            piCardinalPoint [iCounter] = EAST;
            iCounter ++;
        }
        
        if (bLinkSouth) {
            piJumpX[iCounter] = iX;
            piJumpY[iCounter] = iY - 1;
            piCardinalPoint [iCounter] = SOUTH;
            iCounter ++;
        }
        
        if (bLinkWest) {
            piJumpX[iCounter] = iX - 1;
            piJumpY[iCounter] = iY;
            piCardinalPoint [iCounter] = WEST;
            iCounter ++;
        }
        
        Assert(iCounter == iNumJumps);

        Variant vTemp;
        String strPlanetName;

        for (i = 0; i < iNumJumps; i ++) {
            
            // Get neighbouring planet's key
            iErrCode = GetNeighbourPlanetKey (
                m_iGameClass, 
                m_iGameNumber, 
                iPlanetKey, 
                piCardinalPoint[i], 
                &iNeighbourKey
                );
            
            RETURN_ON_ERROR(iErrCode);
            
            Assert(iNeighbourKey != NO_KEY);
            
            if (!bAdmin && !bSpectator) {
                
                iErrCode = GetPlanetNameWithSecurity (
                    m_iGameClass, 
                    m_iGameNumber, 
                    iEmpireKey, 
                    iNeighbourKey, 
                    &vTemp
                    );
                RETURN_ON_ERROR(iErrCode);

            } else {
                
                iErrCode = GetPlanetName (
                    m_iGameClass, 
                    m_iGameNumber, 
                    iNeighbourKey, 
                    &vTemp
                    );
                RETURN_ON_ERROR(iErrCode);
            }

            Assert(vTemp.GetCharPtr());
            String::AtoHtml(vTemp.GetCharPtr(), &strPlanetName, 0, false);
            Assert(strPlanetName.GetCharPtr());

            OutputText ("<strong>");
            m_pHttpResponse->WriteText (CARDINAL_STRING[piCardinalPoint[i]]);
            OutputText ("</strong>: ");

            if (!strPlanetName.IsBlank()) {
                m_pHttpResponse->WriteText (strPlanetName.GetCharPtr(), strPlanetName.GetLength());
            }
            OutputText (" (");
            m_pHttpResponse->WriteText (piJumpX[i]);
            OutputText (",");
            m_pHttpResponse->WriteText (piJumpY[i]);
            OutputText (")<br>");
        }
        
    } else {
        OutputText ("<strong>None</strong>");
    }
    
    OutputText ("</td></tr>");
    
    int iTotalNumShips = 
        pvPlanetData[GameMap::iNumUncloakedShips].GetInteger() + 
        pvPlanetData[GameMap::iNumCloakedShips].GetInteger() + 
        pvPlanetData[GameMap::iNumUncloakedBuildShips].GetInteger() + 
        pvPlanetData[GameMap::iNumCloakedBuildShips].GetInteger();
    
    if (iTotalNumShips > 0)
    {
        Vector<unsigned int> vecOwnerData;

        iErrCode = GetPlanetShipOwnerData (
            m_iGameClass, 
            m_iGameNumber, 
            iEmpireKey, 
            iPlanetKey, 
            iProxyPlanetKey, 
            iTotalNumShips, 
            bVisibleBuilds, 
            bIndependence, 
            vecOwnerData
            );
        RETURN_ON_ERROR(iErrCode);

        const unsigned int* piOwnerData = vecOwnerData.GetData();
        int iNumOwners = piOwnerData[0];
        if (iNumOwners > 0)
        {
            m_pHttpResponse->WriteText (
                "<tr><td></td><td></td>"\
                "<td colspan=\"10\">"\
                "<table>"\
                "<tr>"\
                "<td></td>"\
                "<th></th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor);
            OutputText ("\">Empire</th>");

            bool pbTech [NUM_SHIP_TYPES];
            memset (pbTech, 0, sizeof (pbTech));

            int** ppiNumTechsTable = (int**) StackAlloc (iNumOwners * sizeof (int*));
            int* piTemp = (int*) StackAlloc (iNumOwners * NUM_SHIP_TYPES * sizeof (int));
            for (i = 0; i < iNumOwners; i ++) {

                ppiNumTechsTable[i] = piTemp + i * NUM_SHIP_TYPES;
                memset (ppiNumTechsTable[i], 0, NUM_SHIP_TYPES * sizeof (int));
            }
            unsigned int* piOwnerKey = (unsigned int*) StackAlloc (2 * iNumOwners * sizeof (unsigned int));
            unsigned int* piOwnerShips = piOwnerKey + iNumOwners;

            unsigned int iBase = 1;
            for (i = 0; i < iNumOwners; i ++) {

                piOwnerKey[i] = piOwnerData [iBase];
                piOwnerShips[i] = piOwnerData [iBase + 1];

                unsigned int iNumOwnerTechs = piOwnerData [iBase + 2];
                
                for (j = 0; j < iNumOwnerTechs; j ++) {

                    unsigned int iBaseIndex = iBase + 3 + j * 2;
                    unsigned int iType = piOwnerData [iBaseIndex];

                    ppiNumTechsTable [i][iType] = piOwnerData [iBaseIndex + 1];
                    pbTech [iType] = true;
                }
                
                iBase += 3 + 2 * iNumOwnerTechs;
            }
            
            ENUMERATE_SHIP_TYPES (i)
            {
                if (pbTech[i]) {
                    OutputText ("<th bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor);
                    OutputText ("\">");
                    m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
                    OutputText ("</th>");
                }
            }
            OutputText ("</tr>");
            
            int iDip, iWeOffer, iTheyOffer;
            
            Variant vEmpireName;
            const char* pszEmpireName;
            
            for (i = 0; i < iNumOwners; i ++) {
                
                OutputText ("<tr><td align=\"right\">");
                
                if (piOwnerKey[i] != INDEPENDENT) {
                    
                    if (piOwnerKey[i] == iEmpireKey) {
                        
                        iErrCode = WriteProfileAlienString (
                            m_iAlienKey,
                            m_iAlienAddress,
                            iEmpireKey,
                            m_vEmpireName.GetCharPtr(),
                            0, 
                            "ProfileLink",
                            "View your profile",
                            false,
                            false
                            );
                        RETURN_ON_ERROR(iErrCode);

                    } else {
                        
                        Variant vTemp;

                        iErrCode = GetEmpireProperty (piOwnerKey[i], SystemEmpireData::AlienKey, &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                        iAlienKey = vTemp.GetInteger();

                        iErrCode = GetEmpireProperty (piOwnerKey[i], SystemEmpireData::AlienAddress, &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                        int iAlienAddress = vTemp.GetInteger();

                        iErrCode = GetEmpireName (piOwnerKey[i], &vEmpireName);
                        RETURN_ON_ERROR(iErrCode);

                        char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
                        sprintf(pszProfile, "View the profile of %s", vEmpireName.GetCharPtr());
                        
                        iErrCode = WriteProfileAlienString (
                            iAlienKey,
                            iAlienAddress,
                            piOwnerKey[i],
                            vEmpireName.GetCharPtr(),
                            0, 
                            "ProfileLink",
                            pszProfile,
                            false,
                            true
                            );
                        RETURN_ON_ERROR(iErrCode);
                        
                        NotifyProfileLink();
                    }
                    
                } else {
                    
                    WriteIndependentPlanetString();
                }
                
                OutputText ("</td>");
                
                if (piOwnerKey[i] == iEmpireKey) {
                    
                    iDip = ALLIANCE;
                    pszEmpireName = m_vEmpireName.GetCharPtr();
                    
                } else {
                    
                    if (piOwnerKey[i] == INDEPENDENT) {
                        iDip = WAR;
                        pszEmpireName = INDEPENDENT_NAME;
                    } else {
                        
                        iErrCode = GetEmpireName (piOwnerKey[i], &vEmpireName);
                        RETURN_ON_ERROR(iErrCode);
                        pszEmpireName = vEmpireName.GetCharPtr();
                        
                        if (!bMapColoring) {
                            iDip = TRUCE;
                        } else {
                            
                            iErrCode = GetVisibleDiplomaticStatus(
                                m_iGameClass, 
                                m_iGameNumber, 
                                iEmpireKey, 
                                piOwnerKey[i], 
                                &iWeOffer, 
                                &iTheyOffer, 
                                &iDip,
                                NULL
                                );
                            RETURN_ON_ERROR(iErrCode);
                        }
                    }
                }
                
                if (iDip == WAR) {
                    
                    OutputText ("<td align=\"center\"><font color=\"");
                    m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                    OutputText ("\"><strong>");
                    m_pHttpResponse->WriteText (piOwnerShips[i]);
                    OutputText ("</strong></font></td><td align=\"center\"><strong>");
                    m_pHttpResponse->WriteText (pszEmpireName);
                    OutputText ("</strong></td>");
                    
                    ENUMERATE_SHIP_TYPES (j) {
                        if (pbTech[j]) {
                            OutputText ("<td align=\"center\"><font color=\"");
                            m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                            OutputText ("\">");
                            m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
                            OutputText ("</font></td>");
                        }
                    }
                    
                } else {
                    
                    if (iDip == ALLIANCE) {
                        OutputText ("<td align=\"center\"><font color=\"");
                        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                        OutputText ("\"><strong>");
                        m_pHttpResponse->WriteText (piOwnerShips[i]);
                        OutputText ("</strong></font></td><td align=\"center\"><strong>");
                        m_pHttpResponse->WriteText (pszEmpireName);
                        OutputText ("</strong></td>");
                        
                        ENUMERATE_SHIP_TYPES (j) {
                            if (pbTech[j]) {
                                OutputText ("<td align=\"center\"><font color=\"");
                                m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                                OutputText ("\">");
                                m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
                                OutputText ("</font></td>");
                            }
                        }
                        
                    } else {
                        
                        OutputText ("<td align=\"center\"><strong>");
                        m_pHttpResponse->WriteText (piOwnerShips[i]);
                        OutputText ("</strong></font></td><td align=\"center\"><strong>");
                        m_pHttpResponse->WriteText (pszEmpireName);
                        OutputText ("</strong></td>");

                        ENUMERATE_SHIP_TYPES (j) {
                            if (pbTech[j]) {
                                OutputText ("<td align=\"center\">");
                                m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
                                OutputText ("</td>");
                            }
                        }
                    }
                }
                OutputText ("</tr>");
            }
            OutputText ("</table></td></tr>");
        }
    }
    
    return iErrCode;
}