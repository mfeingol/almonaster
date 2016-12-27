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

int iErrCode;

bool bInitialized;
iErrCode = InitializeEmpireInGame(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
bool bRedirected;
iErrCode = InitializeGame(&pageRedirect, &bRedirected);
RETURN_ON_ERROR(iErrCode);
if (bRedirected)
{
    return Redirect(pageRedirect);
}

IHttpForm* pHttpForm;

unsigned int i;

bool bMapGenerated = (m_iGameState & GAME_MAP_GENERATED) != 0;
bool bGameStarted = (m_iGameState & STARTED) != 0;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (bMapGenerated) {

        iErrCode = HandleShipMenuSubmissions();
        RETURN_ON_ERROR(iErrCode);

        // Handle planet name change or maxpop change submissions
        unsigned int iNumTestPlanets;
        if ((pHttpForm = m_pHttpRequest->GetForm ("NumOurPlanets")) == NULL) {
            iErrCode = GetNumVisitedPlanets (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumTestPlanets);
            RETURN_ON_ERROR(iErrCode);
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
            sprintf(pszForm, "OldPlanetName%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            pszOldPlanetName = pHttpForm->GetValue();

            // Get new name
            sprintf(pszForm, "NewPlanetName%i", i);
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
                    iErrCode = RenamePlanet(m_iGameClass, m_iGameNumber, m_iEmpireKey, iUpdatePlanetKey, pszNewPlanetName);
                    RETURN_ON_ERROR(iErrCode);
                }
            }

            // Get original MaxPop
            sprintf(pszForm, "OldMaxPop%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iOldMaxPop = pHttpForm->GetIntValue();

            // Get new MaxPop
            sprintf(pszForm, "NewMaxPop%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iNewMaxPop = pHttpForm->GetIntValue();

            if (iOldMaxPop != iNewMaxPop) {

                // Get planet key
                if (iUpdatePlanetKey == NO_KEY) {

                    sprintf(pszForm, "KeyPlanet%i", i);

                    if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                        goto Redirection;
                    }
                    iUpdatePlanetKey = pHttpForm->GetIntValue();
                }

                iErrCode = SetPlanetMaxPop(
                    m_iGameClass,
                    m_iGameNumber,
                    m_iEmpireKey,
                    iUpdatePlanetKey, 
                    iNewMaxPop
                    );
                RETURN_ON_ERROR(iErrCode);
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
                    iErrCode = HandleMiniBuild(iUpdatePlanetKey);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }
    }
}

Redirection:
if (m_bRedirectTest)
{
    iErrCode = RedirectOnSubmitGame(&pageRedirect, &bRedirected);
    RETURN_ON_ERROR(iErrCode);
    if (bRedirected)
    {
        return Redirect (pageRedirect);
    }
}

iErrCode = OpenGamePage();
RETURN_ON_ERROR(iErrCode);

// Individual page stuff starts here
unsigned int iLivePlanetKey, iDeadPlanetKey, iNumPlanets;
int iLivePlanetAddress, iDeadPlanetAddress;
iErrCode = GetEmpirePlanetIcons(m_iEmpireKey, &iLivePlanetKey, &iLivePlanetAddress, &iDeadPlanetKey, &iDeadPlanetAddress);
RETURN_ON_ERROR(iErrCode);

if (bGameStarted && ShouldDisplayGameRatios())
{
    iErrCode = WriteRatiosString(NULL);
    RETURN_ON_ERROR(iErrCode);
}

if (bMapGenerated) {

    Variant* pvPlanetKey;
    AutoFreeData free_pvPlanetKey(pvPlanetKey);

    unsigned int* piProxyKey, iCounter = 0;
    AutoFreeKeys free_piProxyKey(piProxyKey);

    bool bOurPlanet;
    iErrCode = GetVisitedPlanetKeys (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        &pvPlanetKey,
        &piProxyKey, 
        &iNumPlanets
        );
    RETURN_ON_ERROR(iErrCode);

    if (iNumPlanets > 0) {

        GET_GAME_MAP(strGameMap, m_iGameClass, m_iGameNumber);
        Variant vOptions;

        int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;

        float fAgRatio;

        bool bShips = false;
        ShipsInMapScreen simShipsInMap = { NO_KEY, 0, 0 };

        int iBR = 0;
        float fMaintRatio = 0.0, fNextMaintRatio = 0.0;

        // Visible builds?
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, m_iGameClass, SystemGameClassData::Options, &vOptions);
        RETURN_ON_ERROR(iErrCode);

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
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireAgRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fAgRatio);
        RETURN_ON_ERROR(iErrCode);

        bShips = (m_iGameOptions & SHIPS_ON_PLANETS_SCREEN) != 0;

        if (bShips) {

            iErrCode = GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetEmpireMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fMaintRatio);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetEmpireNextMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fNextMaintRatio);
            RETURN_ON_ERROR(iErrCode);
        }

        for (i = 0; i < iNumPlanets; i ++)
        {
            Variant* pvPlanetData = NULL;
            AutoFreeData free_pvPlanetData(pvPlanetData);

            iErrCode = t_pCache->ReadRow (strGameMap, pvPlanetKey[i].GetInteger(), &pvPlanetData);
            RETURN_ON_ERROR(iErrCode);

            %><p><table width="90%"><%

            iErrCode = WriteUpClosePlanetString (m_iEmpireKey, pvPlanetKey[i].GetInteger(), 
                piProxyKey[i], iLivePlanetKey, iLivePlanetAddress, iDeadPlanetKey, iDeadPlanetAddress, iCounter, 
                (vOptions.GetInteger() & VISIBLE_BUILDS) != 0, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, 
                iBadFuel, fAgRatio, (vOptions.GetInteger() & INDEPENDENCE) != 0, false, false,
                pvPlanetData, &bOurPlanet
                );
            RETURN_ON_ERROR(iErrCode);

            // Render ships
            if (bShips)
            {
                simShipsInMap.iPlanetKey = pvPlanetKey[i].GetInteger();

                // Render ships
                iErrCode = RenderShips (
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
                RETURN_ON_ERROR(iErrCode);
            }

            if (m_iGameOptions & BUILD_ON_PLANETS_SCREEN)
            {
                iErrCode = RenderMiniBuild (pvPlanetKey[i].GetInteger(), false);
                RETURN_ON_ERROR(iErrCode);
            }

            %></table><%

            if (bOurPlanet)
            {
                iCounter ++;
            }
        }

        %><p><%

        WriteButton (BID_CANCEL);

        if (bShips)
        {
            %><input type="hidden" name="NumShips" value="<% Write (simShipsInMap.iCurrentShip); %>"><%
            %><input type="hidden" name="NumFleets" value="<% Write (simShipsInMap.iCurrentFleet); %>"><%
        }
    }

    %><input type="hidden" name="NumOurPlanets" value="<% Write (iCounter); %>"><%
}
else
{
    %><p><table width="90%"><%

    bool bTrue;
    iErrCode = WriteUpClosePlanetString (m_iEmpireKey, NO_KEY, NO_KEY, iLivePlanetKey, iLivePlanetAddress, iDeadPlanetKey, iDeadPlanetAddress, 
                                         0, false, 0, 0, 0, 0, 0, 0, 0.0, false, false, false, NULL, &bTrue);
    RETURN_ON_ERROR(iErrCode);

    %></table><%
}

iErrCode = CloseGamePage();
RETURN_ON_ERROR(iErrCode);

%>