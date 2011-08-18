<%
#include <stdio.h>

// Almonaster
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

if (InitializeEmpireInGame(false) != OK)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
if (InitializeGame(&pageRedirect) != OK)
{
    return Redirect(pageRedirect);
}

IHttpForm* pHttpForm;

int iErrCode;
unsigned int i;

bool bMapGenerated = (m_iGameState & GAME_MAP_GENERATED) != 0;
bool bGameStarted = (m_iGameState & STARTED) != 0;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (bMapGenerated) {

        iErrCode = HandleShipMenuSubmissions();
        if (iErrCode != OK) {
            AddMessage ("Error handling ship menu submissions");
            goto Redirection;
        }

        // Handle planet name change or maxpop change submissions
        unsigned int iNumTestPlanets;
        if ((pHttpForm = m_pHttpRequest->GetForm ("NumOurPlanets")) == NULL) {
            GameCheck (GetNumVisitedPlanets (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumTestPlanets));
        } else {
            iNumTestPlanets = pHttpForm->GetIntValue();
        }

        const char* pszOldPlanetName, * pszNewPlanetName;
        int iOldMaxPop, iNewMaxPop, iUpdatePlanetKey;

        char pszForm [256];
        bool bBuildTest = true;

        for (i = 0; i < iNumTestPlanets; i ++) {

            // Get planet key
            snprintf (pszForm, sizeof (pszForm), "KeyPlanet%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iUpdatePlanetKey = pHttpForm->GetUIntValue();

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

                if (String::IsWhiteSpace (pszNewPlanetName)) {
                    AddMessage ("Blank planet names are not allowed");
                }
                else if (strlen (pszNewPlanetName) > MAX_PLANET_NAME_LENGTH) {
                    AddMessage ("The new planet name was too long");
                } else {

                    // Best effort
                    if (RenamePlanet (
                        m_iGameClass,
                        m_iGameNumber,
                        m_iEmpireKey,
                        iUpdatePlanetKey, 
                        pszNewPlanetName
                        ) != OK) {

                    }
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

                if (SetPlanetMaxPop (
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

            // Build
            if (bBuildTest) {

                if (m_iButtonKey == NULL_THEME) {
                    snprintf (pszForm, sizeof (pszForm), "MiniBuild%i", iUpdatePlanetKey);
                } else {
                    snprintf (pszForm, sizeof (pszForm), "MiniBuild%i.x", iUpdatePlanetKey);
                }

                pHttpForm = m_pHttpRequest->GetForm (pszForm);
                if (pHttpForm != NULL) {

                    bBuildTest = false;
                    m_bRedirectTest = false;

                    // Do build
                    HandleMiniBuild (iUpdatePlanetKey);
                }
            }
        }
    }
}

Redirection:
if (m_bRedirectTest)
{
    PageId pageRedirect;
    if (RedirectOnSubmitGame (&pageRedirect))
    {
        return Redirect (pageRedirect);
    }
}

OpenGamePage();

// Individual page stuff starts here
unsigned int iLivePlanetKey, iDeadPlanetKey, iNumPlanets;

GameCheck (GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey));

if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
    GameCheck (WriteRatiosString (NULL));
}

if (bMapGenerated) {

    Variant* pvPlanetKey;
    unsigned int* piProxyKey, iCounter = 0;
    bool bOurPlanet;
    GameCheck (GetVisitedPlanetKeys (
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

        Variant* pvPlanetData = NULL;

        int iBR = 0;
        float fMaintRatio = 0.0, fNextMaintRatio = 0.0;

        // Visible builds?
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, m_iGameClass, SystemGameClassData::Options, &vOptions);
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

        iErrCode = GetEmpireAgRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fAgRatio);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        bShips = (m_iGameOptions & SHIPS_ON_PLANETS_SCREEN) != 0;

        if (bShips) {

            iErrCode = GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = GetEmpireMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fMaintRatio);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = GetEmpireNextMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fNextMaintRatio);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        for (i = 0; i < iNumPlanets; i ++) {

            iErrCode = t_pConn->ReadRow (strGameMap, pvPlanetKey[i].GetInteger(), &pvPlanetData);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            %><p><table width="90%"><%

            iErrCode = WriteUpClosePlanetString (m_iEmpireKey, pvPlanetKey[i].GetInteger(), 
                piProxyKey[i], iLivePlanetKey, iDeadPlanetKey, iCounter, 
                (vOptions.GetInteger() & VISIBLE_BUILDS) != 0, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, 
                iBadFuel, fAgRatio, (vOptions.GetInteger() & INDEPENDENCE) != 0, false, false,
                pvPlanetData, &bOurPlanet
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Render ships
            if (bShips) {

                simShipsInMap.iPlanetKey = pvPlanetKey[i].GetInteger();

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
                    NULL,
                    NULL
                    );
            }

            if (m_iGameOptions & BUILD_ON_PLANETS_SCREEN) {  
                RenderMiniBuild (pvPlanetKey[i].GetInteger(), false);
            }

            %></table><%

            t_pConn->FreeData (pvPlanetData);
            pvPlanetData = NULL;

            if (bOurPlanet) {
                iCounter ++;
            }
        }

        %><p><%

        WriteButton (BID_CANCEL);

Cleanup:

        if (pvPlanetData != NULL) {
            t_pConn->FreeData (pvPlanetData);
        }

        if (pvPlanetKey != NULL) {
            FreeData (pvPlanetKey);
        }

        if (piProxyKey != NULL) {
            FreeKeys ((unsigned int*) piProxyKey);
        }

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
    iErrCode = WriteUpClosePlanetString (m_iEmpireKey, NO_KEY, NO_KEY, 
        iLivePlanetKey, iDeadPlanetKey, 0, false, 0, 0, 0, 0, 0, 0, 0.0, false, false, false, NULL, &bTrue);

    if (iErrCode != OK) {
        %>Error rendering up-close planet view. The error was <% Write (iErrCode);
    }

    %></table><%
}

CloseGamePage();

%>