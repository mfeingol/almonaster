//
// Almonaster.dll:  a component of Almonaster
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

#include "HtmlRenderer.h"

void HtmlRenderer::RenderShips (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                int iBR, float fMaintRatio, ShipsInMapScreen* pShipsInMap, 
                                bool bInMapOrPlanets) {
    
    GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, iEmpireKey);


    IDatabase* pDatabase = g_pGameEngine->GetDatabase();
    Assert (pDatabase != NULL);

    // Read ship location column
    unsigned int* piShipKey = NULL, * piFleetKey = NULL, iNumShips = 0, iNumFleets = 0, iNumFleetShips = 0;
    int* piShipLoc = NULL, * piFleetLoc = NULL, iErrCode;
    
    unsigned int** ppiFleetShips = NULL, * piNumShipsInFleet = NULL, i, j, iNumOrders;
    
    int* piOrderKey = NULL, iSelectedOrder, iLastX = 0, iLastY = 0, iLastLocation = NO_KEY;
    String* pstrOrderText = NULL;
    
    Variant vPlanetName, * pvFleetData = NULL;

    const char* pszTableColor = m_vTableColor.GetCharPtr();
    size_t stTableColorLen = strlen (pszTableColor);

    bool bShipString = true;
    bool bFleetString = true;
    bool bOpenTableRow = bInMapOrPlanets;
    bool bCloseTableRow = false;
    
    GameConfiguration gcConfig;
    iErrCode = g_pGameEngine->GetGameConfiguration (&gcConfig);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (pShipsInMap == NULL) {

        IReadTable* pRead = NULL;

        iErrCode = pDatabase->GetTableForReading (pszShips, &pRead);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pRead->ReadColumn (
            GameEmpireShips::CurrentPlanet, 
            &piShipKey,
            &piShipLoc, 
            &iNumShips
            );

        SafeRelease (pRead);
        
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pDatabase->GetTableForReading (pszFleets, &pRead);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pRead->ReadColumn (
            GameEmpireFleets::CurrentPlanet, 
            &piFleetKey, 
            &piFleetLoc, 
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
        iErrCode = pDatabase->GetEqualKeys (
            pszShips,
            GameEmpireShips::CurrentPlanet,
            pShipsInMap->iPlanetKey,
            false,
            &piShipKey,
            &iNumShips
            );
        
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pDatabase->GetEqualKeys (
            pszFleets,
            GameEmpireFleets::CurrentPlanet,
            pShipsInMap->iPlanetKey,
            false,
            &piFleetKey,
            &iNumFleets
            );
        
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
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
            Algorithm::QSortTwoAscending<int, unsigned int> (piShipLoc, piShipKey, iNumShips);
        }

        if (bOpenTableRow) {

            bOpenTableRow = false;
            bCloseTableRow = true;
            OutputText (
                "<tr><td>&nbsp;</td></tr>"\
                "<tr><td></td><td align=\"center\" colspan=\"10\">"
                );
        }
        
        // Allocate space for fleet data
        if (iNumFleets > 0) {
            
            if (pShipsInMap == NULL) {
                Algorithm::QSortTwoAscending<int, unsigned int> (piFleetLoc, piFleetKey, iNumFleets);
            }

            ppiFleetShips = (unsigned int**) StackAlloc (iNumFleets * sizeof (unsigned int*));
            piNumShipsInFleet = (unsigned int*) StackAlloc (iNumFleets * sizeof (unsigned int));
            
            for (i = 0; i < iNumFleets; i ++) {
                
                iErrCode = g_pGameEngine->GetNumShipsInFleet (
                    m_iGameClass,
                    m_iGameNumber,
                    m_iEmpireKey,
                    piFleetKey[i],
                    (int*) piNumShipsInFleet + i
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
            
            OutputText ("<p><table width=\"90%\">");

            if (bShipString) {
                bShipString = false;
                OutputText ("<tr><td align=\"center\" colspan=\"7\">Ships:</td></tr>");
            }

            OutputText ("<tr><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Ship</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">BR</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Next BR</th><th bgcolor=\"");
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

            Variant vTemp;
            
            iErrCode = pDatabase->ReadData (pszShips, piShipKey[i], GameEmpireShips::FleetKey, &vTemp);
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

                Variant* pvData;

                iErrCode = pDatabase->ReadRow (pszShips, piShipKey[i], &pvData);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = WriteShip (
                    gcConfig,
                    pvData,
                    pShipsInMap == NULL ? i : pShipsInMap->iCurrentShip, 
                    piShipKey[i], 
                    iBR, fMaintRatio, pShipsInMap == NULL ? piShipLoc[i] : pShipsInMap->iPlanetKey, 
                    iLastLocation, iLastX, iLastY, vPlanetName
                    );

                pDatabase->FreeData (pvData);
                
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
                "<tr><td></td><td align=\"center\" colspan=\"10\">"
                );
        }
        
        unsigned int iNumShipsInFleet;
        int iPercentage, iIndex;
        float fCurrentStrength, fMaxStrength;
        
        String strHtml;

        for (i = 0; i < iNumFleets; i ++) {
            
            iErrCode = pDatabase->ReadRow (pszFleets, piFleetKey[i], &pvFleetData);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = g_pGameEngine->GetFleetOrders (m_iGameClass, m_iGameNumber, m_iEmpireKey, piFleetKey[i],
                gcConfig, &piOrderKey, &pstrOrderText, &iSelectedOrder, (int*) &iNumOrders);
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            Assert (iNumOrders > 1);
            
            OutputText (
                "<p>"\
                "<table cellspacing=\"2\" width=\"90%\" cellspacing=\"0\" cellpadding=\"0\">"
                );

            if (bFleetString) {
                bFleetString = false;
                OutputText ("<tr><td align=\"center\" colspan=\"6\">Fleets:</td></tr>");
            }

            OutputText (
                "<tr>"\
                "<th align=\"left\" bgcolor=\""
                );
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
            OutputText ("\">Fleet</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Ships</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\" colspan=\"2\">Strength</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Location</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
            OutputText ("\">Orders</th></tr>");
            
            iErrCode = HTMLFilter (pvFleetData[GameEmpireFleets::Name].GetCharPtr(), &strHtml, 0, false);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            OutputText ("<tr align=\"left\"><td><input type=\"text\" name=\"FleetName");
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
            
#ifdef _DEBUG
            if (ppiFleetShips == NULL || ppiFleetShips[i] == NULL) {
                iNumShipsInFleet = 0;
            } else {
                iNumShipsInFleet = ppiFleetShips[i][0] - 1;
            }
            
            Assert ((piNumShipsInFleet == NULL && iNumShipsInFleet == 0) ||
                (piNumShipsInFleet[i] == iNumShipsInFleet));
#endif
            
            if (piNumShipsInFleet == NULL) {
                iNumShipsInFleet = 0;
            } else {
                iNumShipsInFleet = piNumShipsInFleet[i];
            }
            
            OutputText ("\"></td><td align=\"center\">");
            m_pHttpResponse->WriteText (iNumShipsInFleet);
            OutputText ("</td><td align=\"center\" colspan=\"2\">");
            
            fCurrentStrength = pvFleetData[GameEmpireFleets::CurrentStrength].GetFloat();
            fMaxStrength = pvFleetData[GameEmpireFleets::MaxStrength].GetFloat();
            
            if (fMaxStrength == (float) 0.0) {
                OutputText ("0 / 0 (0%)");
            } else {
                iPercentage = (int) (100 * fCurrentStrength / fMaxStrength);
                if (iPercentage == 100) {
                    OutputText ("<font color=\"#");
                    m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                    OutputText ("\">");
                } else {
                    OutputText ("<font color=\"#");
                    m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                    OutputText ("\">");
                }

                m_pHttpResponse->WriteText (fCurrentStrength);
                OutputText (" / ");
                m_pHttpResponse->WriteText (fMaxStrength);

                OutputText (" (");
                m_pHttpResponse->WriteText (iPercentage);
                OutputText ("%)</font>");
            }
            
            OutputText ("</td><input type=\"hidden\" name=\"FleetSelectedOrder");
            m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (iSelectedOrder);
            OutputText ("\"><td align=\"center\">");
            
            if (pShipsInMap != NULL  || piFleetLoc[i] != iLastLocation) {
                
                iLastLocation = pShipsInMap == NULL ? piFleetLoc[i] : pShipsInMap->iPlanetKey;
                
                iErrCode = g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, iLastLocation, &vPlanetName);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, iLastLocation, &iLastX, &iLastY);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
            
            m_pHttpResponse->WriteText (vPlanetName.GetCharPtr());
            OutputText (" (");
            m_pHttpResponse->WriteText (iLastX);
            OutputText (",");
            m_pHttpResponse->WriteText (iLastY); 
            OutputText (")</td><td align=\"center\">");
            
            if (iNumOrders == 1) {
                OutputText ("<strong>");
                m_pHttpResponse->WriteText (pstrOrderText[0]);
                OutputText ("</strong><input type=\"hidden\" name=\"FleetOrder");
                m_pHttpResponse->WriteText (i);
                OutputText ("\" value=\"");
                m_pHttpResponse->WriteText (piOrderKey[0]);
                OutputText ("\">");
                
            } else {
                
                OutputText ("<select name=\"FleetOrder");
                m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
                OutputText ("\">");
                
                for (j = 0; j < iNumOrders; j ++) {
                    if (iSelectedOrder == piOrderKey[j]) {
                        OutputText ("<option selected value=\"");
                        m_pHttpResponse->WriteText (piOrderKey[j]);
                        OutputText ("\">");
                        m_pHttpResponse->WriteText (pstrOrderText[j]);
                    } else {
                        OutputText ("<option value=\"");
                        m_pHttpResponse->WriteText (piOrderKey[j]);
                        OutputText ("\">");
                        m_pHttpResponse->WriteText (pstrOrderText[j]);
                    }
                }
                OutputText ("</select>");
            }
            OutputText ("</td></tr>");
            
            if (iNumShipsInFleet > 0) {
                
                OutputText ("<tr><th bgcolor=\"");
                m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
                OutputText ("\">Ship</th><th bgcolor=\"");
                m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
                OutputText ("\">BR</th><th bgcolor=\"");
                m_pHttpResponse->WriteText (pszTableColor, stTableColorLen); 
                OutputText ("\">Next BR</th><th bgcolor=\"");
                m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
                OutputText ("\">Max BR</th><th bgcolor=\"");
                m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
                OutputText ("\">Type</th><th bgcolor=\"");
                m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
                OutputText ("\">Orders</th></tr>");
                
                for (j = 1; j < ppiFleetShips[i][0]; j ++) {
                    
                    Variant* pvData;

                    iIndex = ppiFleetShips[i][j];

                    iErrCode = pDatabase->ReadRow (pszShips, piShipKey[iIndex], &pvData);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    iErrCode = WriteShip (
                        gcConfig,
                        pvData, 
                        pShipsInMap == NULL ? iIndex : pShipsInMap->iCurrentShip, 
                        piShipKey[iIndex], iBR, fMaintRatio, 
                        pShipsInMap == NULL ? piShipLoc[iIndex] : pShipsInMap->iPlanetKey, 
                        iLastLocation, iLastX, iLastY, vPlanetName, true
                        );

                    pDatabase->FreeData (pvData);
                    
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    if (pShipsInMap != NULL) {
                        pShipsInMap->iCurrentShip ++;
                    }
                }
            }
            
            OutputText ("</table>");
            
            if (pShipsInMap != NULL) {
                pShipsInMap->iCurrentFleet ++;
            }
            
            pDatabase->FreeData (pvFleetData);
            pvFleetData = NULL;
        }
        
        delete [] pstrOrderText;
        pstrOrderText = NULL;
        
        delete [] piOrderKey;
        piOrderKey = NULL;
    }
    
Cleanup:

    if (bCloseTableRow) {
        OutputText ("</td></tr>");
    }
    
    if (pvFleetData != NULL) {
        pDatabase->FreeData (pvFleetData);
    }
    
    if (pstrOrderText != NULL) {
        delete [] pstrOrderText;
    }
    
    if (piOrderKey != NULL) {
        delete [] piOrderKey;
    }
    
    if (iNumShips > 0) {
        
        if (pShipsInMap == NULL) {
            pDatabase->FreeData (piShipLoc);
        }
        if (piShipKey != NULL) {
            pDatabase->FreeKeys (piShipKey);
        }
    }
    
    if (iNumFleets > 0) {

        if (pShipsInMap == NULL) {
            pDatabase->FreeData (piFleetLoc);
        }
        if (piFleetKey != NULL) {
            pDatabase->FreeKeys (piFleetKey);
        }
    }

    SafeRelease (pDatabase);
    
    if (iErrCode != OK) {
        AddMessage ("Error in RenderShips()");
    }
}

int HtmlRenderer::HandleShipMenuSubmissions() {
    
    int iErrCode, i, iKey, iOldOrderKey, iNewOrderKey, iNumShips, iNumFleets, iRealNumber;
    
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
    iErrCode = g_pGameEngine->GetNumShips (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iRealNumber);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    if (iNumShips <= iRealNumber) {
        
        char pszForm [128];

        // Protect UpdateShipOrders call
        NamedMutex nmShipMutex;
        g_pGameEngine->LockEmpireShips (m_iGameClass, m_iGameNumber, m_iEmpireKey, &nmShipMutex);
        
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
                        iErrCode = g_pGameEngine->UpdateShipName (
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
            sprintf (pszForm, "ShipSelectedOrder%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            iOldOrderKey = pHttpForm->GetIntValue();
            
            // Get new ship order
            sprintf (pszForm, "ShipOrder%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                continue;
            }
            iNewOrderKey = pHttpForm->GetIntValue();
            
            if (iOldOrderKey != iNewOrderKey) {
                
                // Get ship key if necessary
                if (iKey == NO_KEY) {
                    sprintf (pszForm, "ShipKey%i", i);
                    if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                        continue;
                    }
                    iKey = pHttpForm->GetIntValue();
                }
                
                // Update ship order, best effort
                iErrCode = g_pGameEngine->UpdateShipOrders (
                    m_iGameClass, 
                    m_iGameNumber, 
                    m_iEmpireKey, 
                    iKey, 
                    iNewOrderKey
                    );
            }
        }   // End ships loop

        g_pGameEngine->UnlockEmpireShips (nmShipMutex);
    }
    
    // Get number of fleets
    if ((pHttpForm = m_pHttpRequest->GetForm ("NumFleets")) == NULL) {
        return OK;
    }
    iNumFleets = pHttpForm->GetIntValue();
    
    // Danger!
    iErrCode = g_pGameEngine->GetNumFleets (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iRealNumber);
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
                
                if (!ShipOrFleetNameFilter (pszNewName)) {
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
                        iErrCode = g_pGameEngine->UpdateFleetName (
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
            iNewOrderKey = pHttpForm->GetIntValue();
            
            if (iOldOrderKey != iNewOrderKey) {
                
                // Get fleet key if necessary
                if (iKey == NO_KEY) {
                    sprintf (pszForm, "FleetKey%i", i);
                    if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                        continue;
                    }
                    iKey = pHttpForm->GetIntValue();
                }
                
                // Update fleet orders, best effort
                iErrCode = g_pGameEngine->UpdateFleetOrders (
                    m_iGameClass, 
                    m_iGameNumber, 
                    m_iEmpireKey, 
                    iKey,
                    iNewOrderKey
                    );
            }
        }   // End fleets loop
    }
    
    return OK;
}

int HtmlRenderer::WriteShip (const GameConfiguration& gcConfig, const Variant* pvData, int i, int iShipKey, 
                             int iBR, float fMaintRatio, int iShipLoc, int& iLastLocation, 
                             int& iLastX, int& iLastY, Variant& vPlanetName, bool bFleet) {
    
    int iErrCode, * piOrderKey, iNumOrders, iSelectedOrder, iType, j, iState;
    
    String* pstrOrderText;

    float fCurrentBR, fMaxBR, fNextBR;
    
    OutputText ("<tr><td align=\"center\"><input type=\"hidden\" name=\"ShipKey");
    m_pHttpResponse->WriteText (i);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (iShipKey);
    
    OutputText ("\"><input type=\"text\" name=\"ShipName");
    m_pHttpResponse->WriteText (i);
    
    String strHtml;
    iErrCode = HTMLFilter (pvData[GameEmpireShips::Name].GetCharPtr(), &strHtml, 0, false);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strHtml, strHtml.GetLength());
    OutputText ("\" size=\"15\" maxlength=\"");
    m_pHttpResponse->WriteText (MAX_SHIP_NAME_LENGTH);
    OutputText ("\"><input type=\"hidden\" name=\"OldShipName");
    m_pHttpResponse->WriteText (i);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strHtml, strHtml.GetLength());
    OutputText ("\"></td><td align=\"center\"><font color=\"");

    fCurrentBR = pvData[GameEmpireShips::CurrentBR].GetFloat();
    fMaxBR = pvData[GameEmpireShips::MaxBR].GetFloat();

    if (fCurrentBR < fMaxBR) {
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
    } else {
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
    }
    OutputText ("\">");
    m_pHttpResponse->WriteText (fCurrentBR);
    OutputText ("</font></td><td align=\"center\">");

    fNextBR = fMaintRatio * fCurrentBR;

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
    OutputText ("</font></td><td align=\"center\"><font color=\"");
    if (fMaxBR < (float) iBR) {
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
    } else {
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
    }
    OutputText ("\">");
    m_pHttpResponse->WriteText (fMaxBR);
    OutputText ("</font></td>");
    
    if (!bFleet) {
        
        if (iShipLoc != iLastLocation) {
            
            iLastLocation = iShipLoc;
            
            iErrCode = g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, iLastLocation, &vPlanetName);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            iErrCode = g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, iLastLocation, &iLastX, &iLastY);
            if (iErrCode != OK) {
                goto Cleanup;
            }
        }
        
        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (vPlanetName.GetCharPtr());
        OutputText (" (");
        m_pHttpResponse->WriteText (iLastX);
        OutputText (",");
        m_pHttpResponse->WriteText (iLastY);
        OutputText (")</td>");
    }
    
    iType = pvData[GameEmpireShips::Type].GetInteger();
    iState = pvData[GameEmpireShips::State].GetInteger();
    
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
    iErrCode = g_pGameEngine->GetShipOrders (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        gcConfig,
        iShipKey,
        iType,
        fCurrentBR,
        fMaintRatio,
        iLastLocation,
        iLastX,
        iLastY,
        &piOrderKey,
        &pstrOrderText,
        &iNumOrders,
        &iSelectedOrder
        );
    
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    Assert (iNumOrders > 1);
    
    OutputText ("<select name=\"ShipOrder");
    m_pHttpResponse->WriteText (i);
    OutputText ("\">");
    
    for (j = 0; j < iNumOrders; j ++) {
        
        if (iSelectedOrder == piOrderKey[j] ||
            iSelectedOrder == GATE_SHIPS && 
            pvData[GameEmpireShips::GateDestination].GetInteger() == piOrderKey[j]) {
            OutputText ("<option selected value=\"");
            m_pHttpResponse->WriteText (piOrderKey[j]);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pstrOrderText[j]);
        } else {
            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (piOrderKey[j]);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pstrOrderText[j]);
        }
    }
    OutputText ("</select></td></tr><input type=\"hidden\" name=\"ShipSelectedOrder");
    m_pHttpResponse->WriteText (i);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (iSelectedOrder);
    OutputText ("\">");
    
    delete [] piOrderKey;
    delete [] pstrOrderText;
    
Cleanup:
    
    return iErrCode;
}