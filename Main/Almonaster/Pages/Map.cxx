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

bool bMapGenerated = (m_iGameState & GAME_MAP_GENERATED) != 0;
bool bGameStarted = (m_iGameState & STARTED) != 0;
bool bForceFullMap = false;

int iMapSubPage = 0, iClickedPlanetKey = NO_KEY, iClickedProxyPlanetKey = NO_KEY;

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
        bool bPartialMaps;
        PartialMapInfo pmiPartialMapInfo;
        unsigned int iNewValue;

        iErrCode = GetEmpirePartialMapData (
            m_iGameClass,
            m_iGameNumber,
            m_iEmpireKey,
            &bPartialMaps,
            &pmiPartialMapInfo.iCenterKey,
            &pmiPartialMapInfo.iXRadius,
            &pmiPartialMapInfo.iYRadius
            );

        RETURN_ON_ERROR(iErrCode);

        if (bPartialMaps)
        {
            int iMinX = MAX_COORDINATE, iMaxX = MAX_COORDINATE, iMinY = MAX_COORDINATE, 
                iMaxY = MAX_COORDINATE, iCenterX = MAX_COORDINATE, iCenterY = MAX_COORDINATE;

            unsigned int iMaxRadius;

            pHttpForm = m_pHttpRequest->GetForm ("Center");
            if (pHttpForm != NULL)
            {
                iNewValue = pHttpForm->GetIntValue();
                if (iNewValue != pmiPartialMapInfo.iCenterKey)
                {
                    bool bUpdate = true;
                    if (iNewValue != PARTIAL_MAP_NATURAL_CENTER)
                    {
                        iErrCode = HasEmpireVisitedPlanet(m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue, &bUpdate);
                        RETURN_ON_ERROR(iErrCode);
                    }

                    if (bUpdate)
                    {
                        iErrCode = SetEmpireGameProperty(m_iGameClass, m_iGameNumber, m_iEmpireKey, GameEmpireData::PartialMapCenter, iNewValue);
                        RETURN_ON_ERROR(iErrCode);
                        AddMessage ("Your partial map center was updated");
                    }
                    else
                    {
                        AddMessage ("Invalid map center");
                    }
                }
            }

            pHttpForm = m_pHttpRequest->GetForm ("iXRadius");
            if (pHttpForm != NULL)
            {
                iNewValue = pHttpForm->GetIntValue();
                if (iNewValue != pmiPartialMapInfo.iXRadius)
                {
                    bool bUpdate = true;
                    if (iNewValue != PARTIAL_MAP_UNLIMITED_RADIUS)
                    {
                        if (iMinX == MAX_COORDINATE)
                        {
                            iErrCode = GetMapLimits(m_iGameClass, m_iGameNumber, m_iEmpireKey, &iMinX, &iMaxX, &iMinY, &iMaxY);
                            RETURN_ON_ERROR(iErrCode);

                            if (pmiPartialMapInfo.iCenterKey == PARTIAL_MAP_NATURAL_CENTER)
                            {
                                iCenterX = iMinX + (iMaxX - iMinX) / 2;
                                iCenterY = iMinY + (iMaxY - iMinY) / 2;
                            }
                            else
                            {
                                iErrCode = GetPlanetCoordinates(m_iGameClass, m_iGameNumber, pmiPartialMapInfo.iCenterKey, &iCenterX, &iCenterY);
                                RETURN_ON_ERROR(iErrCode);
                            }
                        }

                        Assert(iCenterX != MAX_COORDINATE);
                        Assert(iMinX != MAX_COORDINATE);
                        Assert(iMaxX != MAX_COORDINATE);

                        iMaxRadius = max(iCenterX - iMinX, iMaxX - iCenterX);
                        if (iNewValue < 1 || iNewValue > iMaxRadius)
                        {
                            bUpdate = false;
                        }
                    }

                    if (bUpdate)
                    {
                        iErrCode = SetEmpireGameProperty(m_iGameClass, m_iGameNumber, m_iEmpireKey, GameEmpireData::PartialMapXRadius, iNewValue);
                        RETURN_ON_ERROR(iErrCode);
                        AddMessage ("Your X radius was updated");
                    }
                    else
                    {
                        AddMessage ("Invalid X radius");
                    }
                }
            }

            pHttpForm = m_pHttpRequest->GetForm ("iYRadius");
            if (pHttpForm != NULL)
            {
                iNewValue = pHttpForm->GetIntValue();
                if (iNewValue != pmiPartialMapInfo.iYRadius)
                {
                    bool bUpdate = true;
                    if (iNewValue != PARTIAL_MAP_UNLIMITED_RADIUS)
                    {
                        if (iMinX == MAX_COORDINATE)
                        {
                            iErrCode = GetMapLimits(m_iGameClass, m_iGameNumber, m_iEmpireKey, &iMinX, &iMaxX, &iMinY, &iMaxY);
                            RETURN_ON_ERROR(iErrCode);
    
                            if (pmiPartialMapInfo.iCenterKey == PARTIAL_MAP_NATURAL_CENTER)
                            {
                                iCenterX = iMinX + (iMaxX - iMinX) / 2;
                                iCenterY = iMinY + (iMaxY - iMinY) / 2;
                            }
                            else
                            {
                                iErrCode = GetPlanetCoordinates(m_iGameClass, m_iGameNumber, pmiPartialMapInfo.iCenterKey, &iCenterX, &iCenterY);
                                RETURN_ON_ERROR(iErrCode);
                            }
                        }

                        Assert(iCenterY != MAX_COORDINATE);
                        Assert(iMinY != MAX_COORDINATE);
                        Assert(iMaxY != MAX_COORDINATE);

                        iMaxRadius = max(iCenterY - iMinY, iMaxY - iCenterY);

                        if (iNewValue < 1 || iNewValue > iMaxRadius)
                        {
                            bUpdate = false;
                        }
                    }

                    if (bUpdate)
                    {
                        iErrCode = SetEmpireGameProperty(m_iGameClass, m_iGameNumber, m_iEmpireKey, GameEmpireData::PartialMapYRadius, iNewValue);
                        RETURN_ON_ERROR(iErrCode);
                        AddMessage ("Your Y radius was updated");
                    }
                    else
                    {
                        AddMessage ("Invalid Y radius");
                    }
                }
            }
        }   // End if partial maps

        // Planet click
        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2)
        {
            iMapSubPage = 1;
            m_bRedirectTest = false;
            goto Redirection;
        }
        
        // View minimaps
        if (bMapGenerated && WasButtonPressed (BID_VIEWMINIMAP)) {
            iMapSubPage = 2;
            m_bRedirectTest = false;
            goto Redirection;
        }           

        break;

    case 1:

        {

        if (bMapGenerated)
        {
            iErrCode = HandleShipMenuSubmissions();
            RETURN_ON_ERROR(iErrCode);

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

                        iErrCode = RenamePlanet(
                            m_iGameClass,
                            m_iGameNumber,
                            m_iEmpireKey,
                            iUpdatePlanetKey,
                            pszNewPlanetName
                            );

                        RETURN_ON_ERROR(iErrCode);
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

                if (iOldMaxPop != iNewMaxPop)
                {
                    iErrCode = SetPlanetMaxPop(m_iGameClass, m_iGameNumber, m_iEmpireKey, iUpdatePlanetKey, iNewMaxPop);
                    RETURN_ON_ERROR(iErrCode);
                }
            }

            // Build click
            if (WasButtonPressed (BID_MINIBUILD)) {

                iErrCode = HandleMiniBuild(iUpdatePlanetKey);
                RETURN_ON_ERROR(iErrCode);

                //iClickedPlanetKey = iUpdatePlanetKey;
                //iMapSubPage = 1;
                m_bRedirectTest = false;
                break;
            }

            if (WasButtonPressed (BID_UPDATE)) {

                //iClickedPlanetKey = iUpdatePlanetKey;
                //iMapSubPage = 1;
                m_bRedirectTest = false;
                break;
            }

            // Planet click
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

                //iMapSubPage = 1;
                m_bRedirectTest = false;
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
            m_bRedirectTest = false;
            goto Redirection;
        }
    
        // Planet click
        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

            iMapSubPage = 1;
            m_bRedirectTest = false;
            goto Redirection;
        }
        break;

    default:
        Assert(false);
    }
}

Redirection:
if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
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
if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS)
{
    iErrCode = WriteRatiosString(NULL);
    RETURN_ON_ERROR(iErrCode);
}

Variant vMiniMaps = MINIMAPS_NEVER;
if (bMapGenerated && iMapSubPage == 0)
{
    iErrCode = GetEmpireGameProperty(m_iGameClass, m_iGameNumber, m_iEmpireKey, GameEmpireData::MiniMaps, &vMiniMaps);
    RETURN_ON_ERROR(iErrCode);
        
    if (!bForceFullMap && vMiniMaps.GetInteger() == MINIMAPS_PRIMARY)
    {
        iMapSubPage = 2;
    }
}

//
// Main switch
//
switch (iMapSubPage)
{
case 0:

    %><input type="hidden" name="MapSubPage" value="0" ID="Hidden1"><%

    // Handle case where game hasn't started yet
    if (!bMapGenerated)
    {
        // Draw fake intro map
        %><input type="hidden" name="MapSubPage" value="0"><%
        %><p>Click on a planet for a closer view:<%

        %><p><table cellspacing="0" cellpadding="0"><%
        %><tr><%
        %><td align="center"><font size="1">0</font></td><%
        %><td rowspan="3"><% 

        String strAlienButtonString;
        iErrCode = GetAlienPlanetButtonString(m_iAlienKey, m_iAlienAddress, m_iEmpireKey, true, 0, 0, NULL, NULL, &strAlienButtonString);
        RETURN_ON_ERROR(iErrCode);

        m_pHttpResponse->WriteText (strAlienButtonString.GetCharPtr(), strAlienButtonString.GetLength());

        %></td><td align="center"><font size="1">0</font></td></tr><%
        %><tr><td align="center"><font size="1">0</font></td><td align="center"><font size="1">0</font></td><%
        %></tr><tr><td align="center"><font size="1">(0)</font></td><td align="center"><%
        %><font size="1">(0)</font></td><%
        %></tr><%
        %><tr><%
        %><td colspan="3" align="center"><font size="1"><% Write (m_vEmpireName.GetCharPtr()); %></font><%
        %></td><%
        %></tr><%
        %></table><%
    }
    else
    {
        if (vMiniMaps.GetInteger() != MINIMAPS_NEVER)
        {
            %><p><% WriteButton(BID_VIEWMINIMAP);
        }

        // Get partial map info
        PartialMapInfo pmiPartialMapInfo;
        bool bPartialMaps = false;

        iErrCode = GetEmpirePartialMapData (
            m_iGameClass,
            m_iGameNumber,
            m_iEmpireKey,
            &bPartialMaps,
            &pmiPartialMapInfo.iCenterKey,
            &pmiPartialMapInfo.iXRadius,
            &pmiPartialMapInfo.iYRadius
            );
        RETURN_ON_ERROR(iErrCode);

        pmiPartialMapInfo.bDontShowPartialOptions = false;

        iErrCode = RenderMap (
            m_iGameClass,
            m_iGameNumber,
            m_iEmpireKey,
            false,
            bPartialMaps ? &pmiPartialMapInfo : NULL,
            false
            );
        RETURN_ON_ERROR(iErrCode);
    }

    break;

case 1:
    {

    bool bTrue, bOurPlanet = false;
    unsigned int iNumShipsRendered = 0, iNumFleetsRendered = 0;

    unsigned int iLivePlanetKey, iDeadPlanetKey;
    iErrCode = GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);
    RETURN_ON_ERROR(iErrCode);

    if (!bMapGenerated)
    {
        %><input type="hidden" name="MapSubPage" value="1"><%

        %><p><table width="90%"><%

        iErrCode = WriteUpClosePlanetString (
            m_iEmpireKey, NO_KEY, NO_KEY, iLivePlanetKey, iDeadPlanetKey, 0, false, 
            0, 0, 0, 0, 0, 0, 0.0, false, false, false, NULL, &bTrue
            );
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        GET_GAME_MAP (pszGameMap, m_iGameClass, m_iGameNumber);

        // Make sure we've explored that planet
        iErrCode = HasEmpireExploredPlanet(m_iGameClass, m_iGameNumber, m_iEmpireKey, iClickedPlanetKey, &bTrue);
        RETURN_ON_ERROR(iErrCode);

        if (!bTrue)
        {
            AddMessage ("You have not explored that planet yet");
            return Redirect(INFO);
        }

        int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;
        iErrCode = GetGoodBadResourceLimits(m_iGameClass, m_iGameNumber, &iGoodAg, &iBadAg, &iGoodMin, &iBadMin, &iGoodFuel, &iBadFuel);
        RETURN_ON_ERROR(iErrCode);

        float fAgRatio;
        iErrCode = GetEmpireAgRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fAgRatio);
        RETURN_ON_ERROR(iErrCode);

        Variant vOptions;
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, m_iGameClass, SystemGameClassData::Options, &vOptions);
        RETURN_ON_ERROR(iErrCode);

        Variant* pvPlanetData = NULL;
        AutoFreeData free_pvPlanetData(pvPlanetData);

        iErrCode = t_pCache->ReadRow (pszGameMap, iClickedPlanetKey, &pvPlanetData);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="MapSubPage" value="1"><%

        %><p><table width="90%"><%

        iErrCode = WriteUpClosePlanetString (
            m_iEmpireKey, iClickedPlanetKey, 
            iClickedProxyPlanetKey, iLivePlanetKey, iDeadPlanetKey, 0, 
            (vOptions.GetInteger() & VISIBLE_BUILDS) != 0, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel,
            iBadFuel, fAgRatio, (vOptions.GetInteger() & INDEPENDENCE) != 0, false, false, pvPlanetData, 
            &bOurPlanet
            );
        RETURN_ON_ERROR(iErrCode);

        if (!bOurPlanet)
        {
            %><input type="hidden" name="KeyPlanet0" value="<% Write(iClickedPlanetKey); %>"><%
        }

        // Render ships
        if (m_iGameOptions & SHIPS_ON_MAP_SCREEN)
        {
            ShipsInMapScreen simShipsInMap = { iClickedPlanetKey, 0, 0 };

            int iBR;
            float fMaintRatio, fNextMaintRatio;

            iErrCode = GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetEmpireMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fMaintRatio);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetEmpireNextMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fNextMaintRatio);
            RETURN_ON_ERROR(iErrCode);

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
                &iNumShipsRendered,
                &iNumFleetsRendered
                );
            RETURN_ON_ERROR(iErrCode);

            %><input type="hidden" name="NumShips" value="<% Write (simShipsInMap.iCurrentShip); %>"><%
            %><input type="hidden" name="NumFleets" value="<% Write (simShipsInMap.iCurrentFleet); %>"><%
        }

        // Render build
        if (m_iGameOptions & BUILD_ON_MAP_SCREEN)
        {
            iErrCode = RenderMiniBuild(iClickedPlanetKey, true);
            RETURN_ON_ERROR(iErrCode);
        }

        if (m_iGameOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) {

            %><tr><td>&nbsp;</td></tr><%
            %><tr><td></td><td align="center" colspan="10"><%

            // Render local map
            PartialMapInfo pmiPartialMapInfo = { iClickedPlanetKey, 1, 1, true };

            iErrCode = RenderMap(m_iGameClass, m_iGameNumber, m_iEmpireKey, false, &pmiPartialMapInfo, false);
            RETURN_ON_ERROR(iErrCode);

            %></td></tr><%
        }
    }

    %></table><p><%

    if (bOurPlanet || iNumShipsRendered > 0 || iNumFleetsRendered > 0)
    {
        WriteButton(BID_CANCEL);
        WriteButton(BID_UPDATE);
    }

    }
    break;

case 2:

    %><input type="hidden" name="MapSubPage" value="2"><%
    
    %><p><%
    WriteButton(BID_VIEWMAP);
    %><p>Click on a planet for a closer view:<p><%
    
    iErrCode = RenderMiniMap (m_iGameClass, m_iGameNumber, m_iEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    break;

default:

    Assert(false);
    break;
}

iErrCode = CloseGamePage();
RETURN_ON_ERROR(iErrCode);

%>