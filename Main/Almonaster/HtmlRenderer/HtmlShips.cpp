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

int HtmlRenderer::PopulatePlanetInfo (unsigned int iGameClass, unsigned int iGameNumber, unsigned int iShipPlanet,
                                      ShipOrderPlanetInfo& planetInfo, String& strPlanetName) {

    ICachedTable* pMap = NULL;
    Variant vTemp;

    GAME_MAP (strMap, iGameClass, iGameNumber);

    int iErrCode = t_pCache->GetTable(strMap, &pMap);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pMap->ReadData (iShipPlanet, GameMap::Name, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (String::AtoHtml(vTemp.GetCharPtr(), &strPlanetName, 0, false) == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }
    planetInfo.pszName = strPlanetName.GetCharPtr();
    planetInfo.iPlanetKey = iShipPlanet;

    iErrCode = pMap->ReadData (iShipPlanet, GameMap::Owner, (int*) &planetInfo.iOwner);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pMap->ReadData (iShipPlanet, GameMap::Coordinates, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    GetCoordinates(vTemp.GetCharPtr(), &planetInfo.iX, &planetInfo.iY);

Cleanup:

    SafeRelease(pMap);

    return iErrCode;
}

void HtmlRenderer::RenderShips (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                int iBR, float fMaintRatio, float fNextMaintRatio, ShipsInMapScreen* pShipsInMap, 
                                bool bInMapOrPlanets, unsigned int* piNumShips, unsigned int* piNumFleets) {
    
    GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, iEmpireKey);

    ICachedTable* pRead = NULL;

    // Read ship location column
    FleetOrder* pfoOrders = NULL;

    int iErrCode;
    unsigned int* piShipKey = NULL, * piFleetKey = NULL, iNumShips = 0, iNumFleets = 0, iNumFleetShips = 0;
    Variant* pvShipLoc = NULL, * pvFleetLoc = NULL;
    
    unsigned int** ppiFleetShips = NULL, * piNumShipsInFleet = NULL, i, j, iNumOrders = 0;

    Variant * pvFleetData = NULL, * pvShipData = NULL, vTemp;

    const char* pszTableColor = m_vTableColor.GetCharPtr();
    size_t stTableColorLen = strlen (pszTableColor);

    bool bShipString = true;
    bool bFleetString = true;
    bool bOpenTableRow = bInMapOrPlanets;
    bool bCloseTableRow = false;

    String strPlanetName;
    ShipOrderPlanetInfo planetInfo;
    ShipOrderShipInfo shipInfo;
    ShipOrderGameInfo gameInfo;

    planetInfo.iOwner = NO_KEY;
    planetInfo.iPlanetKey = NO_KEY;
    planetInfo.iX = 0;
    planetInfo.iY = 0;
    planetInfo.pszName = NULL;

    BuildLocation* pblLocations = NULL;
    Algorithm::AutoDelete<BuildLocation> autoDel (pblLocations, true);
    unsigned int iNumLocations = 0;

    bool bReadLocations = false;

    char pszExpandButton [64];

    iErrCode = GetGameClassOptions (m_iGameClass, &gameInfo.iGameClassOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    gameInfo.fMaintRatio = fMaintRatio;
    gameInfo.fNextMaintRatio = fNextMaintRatio;

    GameConfiguration gcConfig;
    iErrCode = GetGameConfiguration (&gcConfig);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (pShipsInMap == NULL) {

        iErrCode = t_pCache->GetTable(pszShips, &pRead);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pRead->ReadColumn (
            GameEmpireShips::CurrentPlanet, 
            &piShipKey,
            &pvShipLoc, 
            &iNumShips
            );

        SafeRelease (pRead);
        
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->GetTable(pszFleets, &pRead);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pRead->ReadColumn (
            GameEmpireFleets::CurrentPlanet, 
            &piFleetKey, 
            &pvFleetLoc, 
            &iNumFleets
            );

        SafeRelease (pRead);
        
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
        
        OutputText ("<input type=\"hidden\" name=\"NumShips\" value=\"");
        m_pHttpResponse->WriteText (iNumShips);
        OutputText ("\">");
        
        OutputText ("<input type=\"hidden\" name=\"NumFleets\" value=\"");
        m_pHttpResponse->WriteText (iNumFleets);
        OutputText ("\">");
        
    } else {
        
        // Single planet render
        iErrCode = t_pCache->GetEqualKeys (
            pszShips,
            GameEmpireShips::CurrentPlanet,
            pShipsInMap->iPlanetKey,
            &piShipKey,
            &iNumShips
            );
        
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = t_pCache->GetEqualKeys (
            pszFleets,
            GameEmpireFleets::CurrentPlanet,
            pShipsInMap->iPlanetKey,
            &piFleetKey,
            &iNumFleets
            );
        
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (piNumShips != NULL) {
        *piNumShips = iNumShips;
    }

    if (piNumFleets != NULL) {
        *piNumFleets = iNumFleets;
    }
    
    if (iNumShips == 0) {
        
        if (pShipsInMap == NULL) {
            OutputText ("<p>You have no ships in service");
        }
        
    } else {

        if (pShipsInMap == NULL) {

            // Report number of ships
            OutputText ("<p>You have <strong>");
            m_pHttpResponse->WriteText (iNumShips);
            OutputText ("</strong> ships in service:");

            // Sort ships by location
            Algorithm::QSortTwoAscending<Variant, unsigned int>(pvShipLoc, piShipKey, iNumShips);
        }

        if (bOpenTableRow) {

            bOpenTableRow = false;
            bCloseTableRow = true;
            OutputText (
                "<tr><td>&nbsp;</td></tr>"\
                "<tr><td></td><td align=\"center\" colspan=\"11\">"
                );
        }
        
        // Allocate space for fleet data
        if (iNumFleets > 0) {
            
            if (pShipsInMap == NULL) {
                Algorithm::QSortTwoAscending<Variant, unsigned int>(pvFleetLoc, piFleetKey, iNumFleets);
            }

            ppiFleetShips = (unsigned int**) StackAlloc (iNumFleets * sizeof (unsigned int*));
            piNumShipsInFleet = (unsigned int*) StackAlloc (iNumFleets * sizeof (unsigned int));
            
            for (i = 0; i < iNumFleets; i ++) {
                
                iErrCode = GetNumShipsInFleet (
                    m_iGameClass,
                    m_iGameNumber,
                    m_iEmpireKey,
                    piFleetKey[i],
                    piNumShipsInFleet + i,
                    NULL
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (piNumShipsInFleet[i] == 0) {
                    ppiFleetShips[i] = NULL;
                } else {
                    ppiFleetShips[i] = (unsigned int*) StackAlloc ((piNumShipsInFleet[i] + 1) * sizeof (unsigned int));
                    ppiFleetShips[i][0] = 1;
                    
                    iNumFleetShips += piNumShipsInFleet[i];
                }
            }
        }
        
        if (iNumFleetShips < iNumShips) {
            
            OutputText ("<p><table width=\"89%\">");

            if (bShipString && iNumFleets > 0) {
                bShipString = false;
                OutputText ("<tr><th align=\"center\" colspan=\"8\">Ships:</th></tr>");
            }

            OutputText ("<tr><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Ship</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">BR</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Next BR</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">After BR</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Max BR</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Location</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Type</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
            OutputText ("\">Orders</th></tr>");
        }
        
        // Process ships!
        unsigned int iFleetKey, iLastFleetKey = NO_KEY, iProxyFleetKey = 0, iLastProxyFleetKey = 0;

        for (i = 0; i < iNumShips; i ++) {
            
            iErrCode = t_pCache->ReadData(pszShips, piShipKey[i], GameEmpireShips::FleetKey, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            iFleetKey = vTemp.GetInteger();
            
            if (iFleetKey != NO_KEY) {
                
                if (iFleetKey == iLastFleetKey) {
                    iProxyFleetKey = iLastProxyFleetKey;
                } else {
                    
                    // Find fleet key
                    for (j = 0; j < iNumFleets; j ++) {
                        if (piFleetKey[j] == iFleetKey) {
                            iProxyFleetKey = iLastProxyFleetKey = j;
                            iLastFleetKey = iFleetKey;
                            break;
                        }
                    }
                    
                    if (j == iNumFleets) {
                        continue;
                    }
                }
                
                // Add to fleet
                ppiFleetShips[iProxyFleetKey][ppiFleetShips[iProxyFleetKey][0]] = i;
                ppiFleetShips[iProxyFleetKey][0] ++;
                
            } else {

                iErrCode = t_pCache->ReadRow (pszShips, piShipKey[i], &pvShipData);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                // ShipOrderPlanetInfo
                unsigned int iShipPlanet = pvShipData[GameEmpireShips::iCurrentPlanet].GetInteger();
                if (iShipPlanet != planetInfo.iPlanetKey) {

                    iErrCode = PopulatePlanetInfo (
                        m_iGameClass, m_iGameNumber, iShipPlanet, planetInfo, strPlanetName);

                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                }

                // ShipOrderShipInfo
                shipInfo.iShipType = pvShipData[GameEmpireShips::iType].GetInteger();
                shipInfo.iSelectedAction = pvShipData[GameEmpireShips::iAction].GetInteger();
                shipInfo.iState = pvShipData[GameEmpireShips::iState].GetInteger();
                shipInfo.fBR = pvShipData[GameEmpireShips::iCurrentBR].GetFloat();
                shipInfo.fMaxBR = pvShipData[GameEmpireShips::iMaxBR].GetFloat();
                shipInfo.bBuilding = pvShipData[GameEmpireShips::iBuiltThisUpdate].GetInteger() != 0;

                if (shipInfo.bBuilding && !bReadLocations) {

                    Assert (pblLocations == NULL);

                    iErrCode = GetBuildLocations (
                        m_iGameClass,
                        m_iGameNumber,
                        m_iEmpireKey,
                        NO_KEY,
                        &pblLocations,
                        &iNumLocations
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    bReadLocations = true;
                }

                // Finally...
                iErrCode = WriteShip (
                    piShipKey[i],
                    pvShipData,
                    pShipsInMap == NULL ? i : pShipsInMap->iCurrentShip,
                    false,
                    gcConfig,
                    planetInfo,
                    shipInfo,
                    gameInfo,
                    pblLocations,
                    iNumLocations
                    );

                t_pCache->FreeData (pvShipData);
                pvShipData = NULL;
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (pShipsInMap != NULL) {
                    pShipsInMap->iCurrentShip ++;
                }
            }
        }   // End process ship loop
        
        if (iNumFleetShips < iNumShips) {
            OutputText ("</table>");
            
            if (iNumFleetShips > 0) {
                OutputText ("<p>");
                
                if (pShipsInMap == NULL) {
                    WriteSeparatorString (m_iSeparatorKey);
                }
            }
        }
    }   // End if numships > 0
    
    
    // Process fleets!
    if (iNumFleets > 0) {

        if (bOpenTableRow) {

            bOpenTableRow = false;
            bCloseTableRow = true;
            OutputText (
                "<tr><td>&nbsp;</td></tr>"\
                "<tr><td></td><td align=\"center\" colspan=\"11\">"
                );
        }
        
        unsigned int iNumShipsInFleet;
        int iPercentage, iIndex;
        float fCurrentStrength, fMaxStrength, fNextStrength, fAfterMaxStrength;
        
        String strHtml;

        OutputText ("<table cellspacing=\"2\" width=\"89%\" cellspacing=\"0\" cellpadding=\"0\">");

        for (i = 0; i < iNumFleets; i ++) {

            unsigned int iSelectedOrder;
            bool bCollapsed;

            unsigned int piShipsByType [NUM_SHIP_TYPES];
            memset (piShipsByType, 0, sizeof (piShipsByType));
            
            iErrCode = t_pCache->ReadRow (pszFleets, piFleetKey[i], &pvFleetData);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = GetFleetOrders (
                m_iGameClass, 
                m_iGameNumber, 
                m_iEmpireKey, 
                piFleetKey[i],
                gcConfig, 
                &pfoOrders, 
                &iNumOrders, 
                &iSelectedOrder
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            Assert (iNumOrders > 1);

            // Gather information
            bCollapsed = (pvFleetData[GameEmpireFleets::iFlags].GetInteger() & FLEET_COLLAPSED_DISPLAY) != 0;
            
#ifdef _DEBUG
            if (ppiFleetShips == NULL || ppiFleetShips[i] == NULL) {
                iNumShipsInFleet = 0;
            } else {
                iNumShipsInFleet = ppiFleetShips[i][0] - 1;
            }

            if (piNumShipsInFleet == NULL) {
                Assert (iNumShipsInFleet == 0);
            } else {
                Assert (piNumShipsInFleet[i] == iNumShipsInFleet);
            }
            Assert (iNumShipsInFleet == (unsigned int) pvFleetData[GameEmpireFleets::iNumShips].GetInteger());
#endif
            if (piNumShipsInFleet == NULL) {
                iNumShipsInFleet = 0;
            } else {
                iNumShipsInFleet = piNumShipsInFleet[i];
            }

            // Render
            OutputText ("<tr><td>&nbsp;</td></tr>");

            if (bFleetString && iNumFleetShips < iNumShips) {
                bFleetString = false;
                OutputText ("<tr><th align=\"center\" colspan=\"8\">Fleets:</th></tr>");
            }

            OutputText ("<tr><th></th><th align=\"left\" bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
            OutputText ("\">Fleet</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Strength</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Next</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">After</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Max</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Location</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Orders</th></tr>");
            
            iErrCode = HTMLFilter (pvFleetData[GameEmpireFleets::iName].GetCharPtr(), &strHtml, 0, false);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            OutputText ("<tr align=\"left\"><td>");

            if (iNumShipsInFleet > 0) {

                ButtonId bid;

                if (bCollapsed) {
                    sprintf (pszExpandButton, "FltClpse+%i", piFleetKey[i]);
                    bid = BID_PLUS;
                } else {
                    sprintf (pszExpandButton, "FltClpse-%i", piFleetKey[i]);
                    bid = BID_MINUS;
                }
                WriteButtonString (m_iButtonKey, ButtonName[bid], ButtonText[bid], pszExpandButton);
            }

            OutputText ("</td><td><input type=\"text\" name=\"FleetName");
            m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
            OutputText ("\"size=\"15\" maxlength=\"");
            m_pHttpResponse->WriteText (MAX_FLEET_NAME_LENGTH);
            OutputText ("\"><input type=\"hidden\" name =\"OldFleetName");
            m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
            OutputText ("\" value=\""); 
            m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
            OutputText ("\"><input type=\"hidden\" name=\"FleetKey");
            m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (piFleetKey[i]);
            
            OutputText ("\"></td><td align=\"center\">");
            
            fCurrentStrength = pvFleetData[GameEmpireFleets::iCurrentStrength].GetFloat();
            fMaxStrength = pvFleetData[GameEmpireFleets::iMaxStrength].GetFloat();

            fNextStrength = fAfterMaxStrength = 0;

            // Determine next and after next strength
            if (iNumShipsInFleet > 0) {

                Assert (pRead == NULL);
                iErrCode = t_pCache->GetTable(pszShips, &pRead);
                if (iErrCode == OK) {

                    unsigned int iLoopGuard = ppiFleetShips[i][0];
                    int iType;

                    for (j = 1; j < iLoopGuard; j ++) {

                        float fBR, fMaxBR;

                        iIndex = ppiFleetShips[i][j];

                        iErrCode = pRead->ReadData (piShipKey[iIndex], GameEmpireShips::CurrentBR, &fBR);
                        if (iErrCode != OK) {
                            break;
                        }

                        iErrCode = pRead->ReadData (piShipKey[iIndex], GameEmpireShips::MaxBR, &fMaxBR);
                        if (iErrCode != OK) {
                            break;
                        }

                        float fNextBR = fBR * fMaintRatio;
                        if (fNextBR >= fMaxBR) {
                            fNextBR = fMaxBR;
                        }
                        fNextStrength += fNextBR * fNextBR;

                        float fAfterNextBR = fNextBR * fNextMaintRatio;
                        if (fAfterNextBR >= fMaxBR) {
                            fAfterNextBR = fMaxBR;
                        }
                        fAfterMaxStrength += fAfterNextBR * fAfterNextBR;

                        if (bCollapsed) {

                            iErrCode = pRead->ReadData (piShipKey[iIndex], GameEmpireShips::Type, &iType);
                            if (iErrCode != OK) {
                                break;
                            }

                            Assert (iType < countof (piShipsByType));
                            piShipsByType [iType] ++;
                        }
                    }

                    SafeRelease (pRead);
                }
            }

            if (iErrCode != OK) {
                Assert (false);
                fNextStrength = fAfterMaxStrength = fCurrentStrength;
            }

            if (fMaxStrength == (float) 0.0) {
                OutputText ("-");
            } else {
                iPercentage = (int) (100 * fCurrentStrength / fMaxStrength);

                OutputText ("<font color=\"#");
                if (iPercentage == 100) {
                    m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                } else {
                    m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                }
                OutputText ("\">");
                m_pHttpResponse->WriteText (iPercentage);
                OutputText ("%</font>");
            }

            // Next strength
            OutputText ("</td><td align=\"center\">");
            if (fMaxStrength == (float) 0.0) {
                OutputText ("-");
            } else {
                iPercentage = (int) (100 * fNextStrength / fMaxStrength);

                OutputText ("<font color=\"#");
                if (iPercentage == 100) {
                    m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                } else {
                    m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                }
                OutputText ("\">");
                m_pHttpResponse->WriteText (iPercentage);
                OutputText ("%</font>");
            }

            // After Next strength
            OutputText ("</td><td align=\"center\">");
            if (fAfterMaxStrength == (float) 0.0) {
                OutputText ("-");
            } else {
                iPercentage = (int) (100 * fAfterMaxStrength / fMaxStrength);

                OutputText ("<font color=\"#");
                if (iPercentage == 100) {
                    m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                } else {
                    m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                }
                OutputText ("\">");
                m_pHttpResponse->WriteText (iPercentage);
                OutputText ("%</font>");
            }

            // Max Strength
            OutputText ("</td><td align=\"center\">");

            if (fMaxStrength == (float) 0.0) {
                OutputText ("-");
            } else {
                m_pHttpResponse->WriteText (fMaxStrength);
            }

            OutputText ("</td><input type=\"hidden\" name=\"FleetSelectedOrder");
            m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (pfoOrders[iSelectedOrder].iKey);
            OutputText ("\"><td align=\"center\">");
            
            unsigned int iLocation = pShipsInMap == NULL ? pvFleetLoc[i].GetInteger() : pShipsInMap->iPlanetKey;
            if (iLocation != planetInfo.iPlanetKey) {

                iErrCode = PopulatePlanetInfo (
                    m_iGameClass, m_iGameNumber, iLocation, planetInfo, strPlanetName);

                if (iErrCode != OK) {
                    goto Cleanup;
                }
            }

            m_pHttpResponse->WriteText (strPlanetName.GetCharPtr(), strPlanetName.GetLength());
            OutputText (" (");
            m_pHttpResponse->WriteText (planetInfo.iX);
            OutputText (",");
            m_pHttpResponse->WriteText (planetInfo.iY); 
            OutputText (")</td><td align=\"center\">");
            
            if (iNumOrders == 1) {

                OutputText ("<strong>");
                m_pHttpResponse->WriteText (pfoOrders[0].pszText);
                OutputText ("</strong><input type=\"hidden\" name=\"FleetOrder");
                m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
                OutputText ("\" value=\"");
                m_pHttpResponse->WriteText (pfoOrders[0].iKey);
                OutputText (".");
                m_pHttpResponse->WriteText (pfoOrders[0].fotType);
                OutputText ("\">");
                
            } else {
                
                OutputText ("<select name=\"FleetOrder");
                m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
                OutputText ("\">");
                
                for (j = 0; j < iNumOrders; j ++) {

                    OutputText ("<option ");
                    if (iSelectedOrder == j) {
                        OutputText ("selected ");
                    }
                    OutputText ("value=\"");
                    m_pHttpResponse->WriteText (pfoOrders[j].iKey);
                    OutputText (".");
                    m_pHttpResponse->WriteText (pfoOrders[j].fotType);
                    OutputText ("\">");
                    m_pHttpResponse->WriteText (pfoOrders[j].pszText);
                    OutputText ("</option>");
                }
                OutputText ("</select>");
            }
            OutputText ("</td></tr>");

            if (iNumShipsInFleet > 0) {

                if (bCollapsed) {

                    OutputText ("<tr><td></td><td colspan=\"7\"><strong>");
                    m_pHttpResponse->WriteText (iNumShipsInFleet);
                    OutputText ("</strong> ship");
                    if (iNumShipsInFleet != 1) {
                        OutputText ("s");
                    }
                    OutputText (": ");

                    bool bPrinted = false;
                    ENUMERATE_SHIP_TYPES (j) {

                        if (piShipsByType[j] == 0) {
                            continue;
                        }

                        if (!bPrinted) {
                            bPrinted = true;
                        } else {
                            OutputText (", ");
                        }

                        OutputText ("<strong>");
                        m_pHttpResponse->WriteText (piShipsByType[j]);
                        OutputText ("</strong> ");

                        if (piShipsByType[j] == 1) {
                            m_pHttpResponse->WriteText (SHIP_TYPE_STRING_LOWERCASE[j]);
                        } else {
                            m_pHttpResponse->WriteText (SHIP_TYPE_STRING_LOWERCASE_PLURAL[j]);
                        }
                    }
                    
                    OutputText ("</td></tr>");

                } else {

                    OutputText ("<tr><th></th><th bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
                    OutputText ("\">");

                    m_pHttpResponse->WriteText (iNumShipsInFleet);
                    OutputText (" ship");
                    if (iNumShipsInFleet != 1) {
                        OutputText ("s");
                    }

                    OutputText ("</th><th bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
                    OutputText ("\">BR</th><th bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
                    OutputText ("\">Next BR</th><th bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
                    OutputText ("\">After BR</th><th bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
                    OutputText ("\">Max BR</th><th bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
                    OutputText ("\">Type</th><th bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
                    OutputText ("\">Orders</th></tr>");
                    
                    for (j = 1; j < ppiFleetShips[i][0]; j ++) {

                        iIndex = ppiFleetShips[i][j];

                        iErrCode = t_pCache->ReadRow (pszShips, piShipKey[iIndex], &pvShipData);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        // ShipOrderPlanetInfo
                        unsigned int iShipPlanet = pvShipData[GameEmpireShips::iCurrentPlanet].GetInteger();
                        if (iShipPlanet != planetInfo.iPlanetKey) {

                            iErrCode = PopulatePlanetInfo (
                                m_iGameClass, m_iGameNumber, iShipPlanet, planetInfo, strPlanetName);

                            if (iErrCode != OK) {
                                goto Cleanup;
                            }
                        }

                        // ShipOrderShipInfo
                        shipInfo.iShipType = pvShipData[GameEmpireShips::iType].GetInteger();
                        shipInfo.iSelectedAction = pvShipData[GameEmpireShips::iAction].GetInteger();
                        shipInfo.iState = pvShipData[GameEmpireShips::iState].GetInteger();
                        shipInfo.fBR = pvShipData[GameEmpireShips::iCurrentBR].GetFloat();
                        shipInfo.fMaxBR = pvShipData[GameEmpireShips::iMaxBR].GetFloat();
                        shipInfo.bBuilding = pvShipData[GameEmpireShips::iBuiltThisUpdate].GetInteger() != 0;

                        if (shipInfo.bBuilding && !bReadLocations) {

                            Assert (pblLocations == NULL);

                            iErrCode = GetBuildLocations (
                                m_iGameClass,
                                m_iGameNumber,
                                m_iEmpireKey,
                                NO_KEY,
                                &pblLocations,
                                &iNumLocations
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

                            bReadLocations = true;
                        }

                        // Finally...
                        iErrCode = WriteShip (
                            piShipKey[iIndex],
                            pvShipData,
                            pShipsInMap == NULL ? iIndex : pShipsInMap->iCurrentShip,
                            true,
                            gcConfig,
                            planetInfo,
                            shipInfo,
                            gameInfo,
                            pblLocations,
                            iNumLocations
                            );

                        t_pCache->FreeData (pvShipData);
                        pvShipData = NULL;
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        if (pShipsInMap != NULL) {
                            pShipsInMap->iCurrentShip ++;
                        }
                    }
                }
            }
            
            if (pShipsInMap != NULL) {
                pShipsInMap->iCurrentFleet ++;
            }
            
            t_pCache->FreeData (pvFleetData);
            pvFleetData = NULL;

            FreeFleetOrders (pfoOrders, iNumOrders);
            pfoOrders = NULL;
        }

        OutputText ("</table>");
    }
    
Cleanup:

    if (bCloseTableRow) {
        OutputText ("</td></tr>");
    }
    
    if (pvFleetData != NULL) {
        t_pCache->FreeData (pvFleetData);
    }

    if (pvShipData != NULL) {
        t_pCache->FreeData (pvShipData);
    }

    if (pfoOrders != NULL) {
        FreeFleetOrders (pfoOrders, iNumOrders);
    }
    
    if (iNumShips > 0) {
        
        if (pShipsInMap == NULL) {
            t_pCache->FreeData (pvShipLoc);
        }
        if (piShipKey != NULL) {
            t_pCache->FreeKeys (piShipKey);
        }
    }
    
    if (iNumFleets > 0) {

        if (pShipsInMap == NULL) {
            t_pCache->FreeData (pvFleetLoc);
        }
        if (piFleetKey != NULL) {
            t_pCache->FreeKeys (piFleetKey);
        }
    }

    if (iErrCode != OK) {
        AddMessage ("Error in RenderShips()");
    }
}

int HtmlRenderer::HandleShipMenuSubmissions() {
    
    int iErrCode, i, iKey, iOldOrderKey, iNewOrderKey, iNewOrderType, iNumShips, iNumFleets, iRealNumber;
    
    IHttpForm* pHttpForm;
    const char* pszOldName, * pszNewName;
    
    // Discard submission if update counts don't match
    if (m_iNumNewUpdates != m_iNumOldUpdates) {
        return OK;
    }
    
    // Get number of ships
    if ((pHttpForm = m_pHttpRequest->GetForm ("NumShips")) == NULL) {
        return OK;
    }
    iNumShips = pHttpForm->GetIntValue();
    
    // Danger!
    iErrCode = GetNumShips (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iRealNumber);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    if (iNumShips <= iRealNumber) {
        
        char pszForm [128];

        for (i = 0; i < iNumShips; i ++) {
            
            iKey = NO_KEY;
            
            // Get old ship name
            sprintf (pszForm, "OldShipName%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            pszOldName = pHttpForm->GetValue();
            if (pszOldName == NULL) {
                pszOldName = "";
            }
            
            // Get new ship name
            sprintf (pszForm, "ShipName%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            pszNewName = pHttpForm->GetValue();
            if (pszNewName == NULL) {
                pszNewName = "";
            }
            
            if (strcmp (pszOldName, pszNewName) != 0) {
                
                if (!ShipOrFleetNameFilter (pszNewName)) {
                    AddMessage ("The ship name contains an illegal character");
                } else {
                    
                    if (strlen (pszNewName) > MAX_SHIP_NAME_LENGTH) {
                        AddMessage ("The ship name is too long");
                    } else {
                        
                        // Get ship key
                        sprintf (pszForm, "ShipKey%i", i);
                        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                            continue;
                        }
                        iKey = pHttpForm->GetIntValue();
                        
                        // Update ship name, best effort
                        iErrCode = UpdateShipName (
                            m_iGameClass, 
                            m_iGameNumber, 
                            m_iEmpireKey, 
                            iKey, 
                            pszNewName
                            );
                    }
                }
            }
            
            // Get old ship order
            sprintf (pszForm, "ShipSelO%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            iOldOrderKey = pHttpForm->GetIntValue();
            
            // Get new ship order
            sprintf (pszForm, "ShipOrder%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            const char* pszTest = pHttpForm->GetValue();
            if (pszTest != NULL && 
                sscanf (pszTest, "%i.%i", &iNewOrderKey, &iNewOrderType) == 2 &&
                iNewOrderType >= SHIP_ORDER_TYPE_FIRST &&
                iNewOrderType <= SHIP_ORDER_TYPE_LAST) {
            
                if (iNewOrderType != SHIP_ORDER_NORMAL || iOldOrderKey != iNewOrderKey) {
                
                    // Get ship key if necessary
                    if (iKey == NO_KEY) {
                        sprintf (pszForm, "ShipKey%i", i);
                        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                            continue;
                        }
                        iKey = pHttpForm->GetIntValue();
                    }
                    
                    // Update ship order, best effort
                    ShipOrder soOrder;
                    soOrder.iKey = iNewOrderKey;
                    soOrder.pszText = NULL;
                    soOrder.sotType = (ShipOrderType) iNewOrderType;

                    iErrCode = UpdateShipOrders (
                        m_iGameClass, 
                        m_iGameNumber, 
                        m_iEmpireKey, 
                        iKey, 
                        soOrder
                        );
                }
            }
        }   // End ships loop
    }
    
    // Get number of fleets
    if ((pHttpForm = m_pHttpRequest->GetForm ("NumFleets")) == NULL) {
        return OK;
    }
    iNumFleets = pHttpForm->GetIntValue();
    
    // Danger!
    iErrCode = GetNumFleets (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iRealNumber);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    if (iNumFleets <= iRealNumber) {
        
        char pszForm [128];
        
        for (i = 0; i < iNumFleets; i ++) {
            
            iKey = NO_KEY;
            
            // Get old fleet name
            sprintf (pszForm, "OldFleetName%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            pszOldName = pHttpForm->GetValue();
            if (pszOldName == NULL) {
                pszOldName = "";
            }
            
            // Get new fleet name
            sprintf (pszForm, "FleetName%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            pszNewName = pHttpForm->GetValue();
            if (pszNewName == NULL) {
                pszNewName = "";
            }
            
            if (strcmp (pszOldName, pszNewName) != 0) {
                
                if (String::IsBlank (pszNewName) || !ShipOrFleetNameFilter (pszNewName)) {
                    AddMessage ("Illegal fleet name");
                } else {
                    
                    if (strlen (pszNewName) > MAX_FLEET_NAME_LENGTH) {
                        AddMessage ("The fleet name is too long");
                    } else {
                        
                        // Get fleet key
                        sprintf (pszForm, "FleetKey%i", i);
                        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                            continue;
                        }
                        iKey = pHttpForm->GetIntValue();
                        
                        // Update fleet name, best effort
                        iErrCode = UpdateFleetName (
                            m_iGameClass, 
                            m_iGameNumber, 
                            m_iEmpireKey, 
                            iKey, 
                            pszNewName
                            );
                    }
                }
            }
            
            // Get old fleet order
            sprintf (pszForm, "FleetSelectedOrder%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            iOldOrderKey = pHttpForm->GetIntValue();
            
            // Get new fleet order
            sprintf (pszForm, "FleetOrder%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            const char* pszTest = pHttpForm->GetValue();
            if (pszTest != NULL && 
                sscanf (pszTest, "%i.%i", &iNewOrderKey, &iNewOrderType) == 2 &&
                iNewOrderType >= FLEET_ORDER_TYPE_FIRST &&
                iNewOrderType <= FLEET_ORDER_TYPE_LAST) {

                if (iNewOrderType != FLEET_ORDER_NORMAL || iOldOrderKey != iNewOrderKey) {
                    
                    // Get fleet key if necessary
                    if (iKey == NO_KEY) {
                        sprintf (pszForm, "FleetKey%i", i);
                        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                            continue;
                        }
                        iKey = pHttpForm->GetIntValue();
                    }

                    FleetOrder foOrder;
                    foOrder.fotType = (FleetOrderType) iNewOrderType;
                    foOrder.iKey = iNewOrderKey;
                    foOrder.pszText = NULL;

                    // Update fleet orders, best effort
                    iErrCode = UpdateFleetOrders (
                        m_iGameClass, 
                        m_iGameNumber, 
                        m_iEmpireKey, 
                        iKey,
                        foOrder
                        );

                    if (iErrCode != OK && iErrCode != ERROR_FLEET_DOES_NOT_EXIST) {

                        if (iErrCode == ERROR_CANNOT_NUKE) {
                            AddMessage("The fleet cannot be set to nuke");
                        } else {
                            AddMessage("At least one ship in the fleet could not be moved to the new planet");
                        }
                    }
                    iErrCode = OK;
                }
            }
        }   // End fleets loop
    }

    // Check for expand/collapse
    pHttpForm = m_pHttpRequest->GetFormBeginsWith ("FltClpse");
    if (pHttpForm != NULL) {

        char cSign;
        unsigned int iFleetKey;

        if (sscanf (pHttpForm->GetName(), "FltClpse%c%d", &cSign, &iFleetKey) == 2) {

            // Best effort
            iErrCode = SetFleetFlag (
                m_iGameClass, 
                m_iGameNumber, 
                m_iEmpireKey,
                iFleetKey,
                FLEET_COLLAPSED_DISPLAY,
                cSign == '-'
                );
        }
    }
    
    return OK;
}

int HtmlRenderer::WriteShip (unsigned int iShipKey, const Variant* pvShipData, unsigned int iIndex, bool bFleet,
                             const GameConfiguration& gcConfig, const ShipOrderPlanetInfo& planetInfo, 
                             const ShipOrderShipInfo& shipInfo, const ShipOrderGameInfo& gameInfo,
                             const BuildLocation* pblLocations, unsigned int iNumLocations) {

    int iErrCode, iType, iState, iSelectedOrder;
    float fCurrentBR, fMaxBR, fNextBR, fAfterNextBR;

    ShipOrder* psoOrder = NULL;
    unsigned int iNumOrders = 0, j;

    OutputText ("<input type=\"hidden\" name=\"ShipKey");
    m_pHttpResponse->WriteText (iIndex);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (iShipKey);
    OutputText ("\"><tr>");

    if (bFleet) {
        OutputText ("<td></td>");
    }
    
    OutputText ("<td align=\"center\"><input type=\"text\" name=\"ShipName");
    m_pHttpResponse->WriteText (iIndex);
    
    String strHtml;
    iErrCode = HTMLFilter (pvShipData[GameEmpireShips::iName].GetCharPtr(), &strHtml, 0, false);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strHtml, strHtml.GetLength());
    OutputText ("\" size=\"15\" maxlength=\"");
    m_pHttpResponse->WriteText (MAX_SHIP_NAME_LENGTH);
    OutputText ("\"><input type=\"hidden\" name=\"OldShipName");
    m_pHttpResponse->WriteText (iIndex);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strHtml, strHtml.GetLength());
    OutputText ("\"></td><td align=\"center\"><font color=\"");

    fCurrentBR = pvShipData[GameEmpireShips::iCurrentBR].GetFloat();
    fMaxBR = pvShipData[GameEmpireShips::iMaxBR].GetFloat();

    if (fCurrentBR < fMaxBR) {
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
    } else {
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
    }
    OutputText ("\">");
    m_pHttpResponse->WriteText (fCurrentBR);
    OutputText ("</font></td><td align=\"center\">");

    // Next BR
    fNextBR = gameInfo.fMaintRatio * fCurrentBR;
    OutputText ("<font color=\"");
    if (fNextBR < fMaxBR) {
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (fNextBR);
    } else {
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (fMaxBR);
    }

    // After Next BR
    fAfterNextBR = gameInfo.fNextMaintRatio * fNextBR;
    OutputText ("</font></td><td align=\"center\"><font color=\"");
    if (fAfterNextBR < fMaxBR) {
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (fAfterNextBR);
    } else {
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (fMaxBR);
    }

    // Max BR
    OutputText ("</font></td><td align=\"center\"><font color=\"");
    if (fMaxBR < shipInfo.fBR) {
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
    } else {
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
    }
    OutputText ("\">");
    m_pHttpResponse->WriteText (fMaxBR);
    OutputText ("</font></td>");
    
    if (!bFleet) {

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (planetInfo.pszName);
        OutputText (" (");
        m_pHttpResponse->WriteText (planetInfo.iX);
        OutputText (",");
        m_pHttpResponse->WriteText (planetInfo.iY);
        OutputText (")</td>");
    }
    
    iType = pvShipData[GameEmpireShips::iType].GetInteger();
    iState = pvShipData[GameEmpireShips::iState].GetInteger();
    
    OutputText ("<td align=\"center\">");

    if (iState & CLOAKED) {

        OutputText ("<strike>");
        m_pHttpResponse->WriteText (SHIP_TYPE_STRING[iType]);
        OutputText ("</strike>");
    
    } else {

        m_pHttpResponse->WriteText (SHIP_TYPE_STRING [iType]);
    }

    if ((iState & MORPH_ENABLED) && iType != MORPHER) {
        OutputText (" (<em>");
        m_pHttpResponse->WriteText (SHIP_TYPE_STRING [MORPHER]);
        OutputText ("</em>)");
    }

    OutputText ("</td><td align=\"center\">");
    
    // Get orders
    iErrCode = GetShipOrders (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        iShipKey,
        &shipInfo,
        &gameInfo,
        &planetInfo,
        gcConfig,
        pblLocations,
        iNumLocations,
        &psoOrder,
        &iNumOrders,
        &iSelectedOrder
        );
    
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    Assert (iNumOrders > 1);
    
    OutputText ("<select name=\"ShipOrder");
    m_pHttpResponse->WriteText (iIndex);
    OutputText ("\">");
    
    for (j = 0; j < iNumOrders; j ++) {

        OutputText ("<option ");
        
        if (psoOrder[j].sotType == SHIP_ORDER_NORMAL &&
            (iSelectedOrder == psoOrder[j].iKey || 
            (iSelectedOrder == GATE_SHIPS && 
            pvShipData[GameEmpireShips::iGateDestination].GetInteger() == psoOrder[j].iKey))) {

            OutputText ("selected ");
        }

        OutputText ("value=\"");
        m_pHttpResponse->WriteText (psoOrder[j].iKey);
        OutputText (".");
        m_pHttpResponse->WriteText (psoOrder[j].sotType);
        OutputText ("\">");
        m_pHttpResponse->WriteText (psoOrder[j].pszText);
    }
    OutputText (
        "</select></td>"\
        "</tr>"
        "<input type=\"hidden\" name=\"ShipSelO");
    m_pHttpResponse->WriteText (iIndex);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (iSelectedOrder);
    OutputText ("\">");
    
Cleanup:

    if (psoOrder != NULL) {
        FreeShipOrders (psoOrder, iNumOrders);
    }

    return iErrCode;
}