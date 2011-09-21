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

int HtmlRenderer::RenderMiniMap (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey) {

    int iErrCode, iMapMinX = 0, iMapMaxX = 0, iMapMinY = 0, iMapMaxY = 0;

    unsigned int iNumHorz, iNumVert, i, j, iNumPlanets, iLivePlanetKey, iDeadPlanetKey, * piProxyKey = NULL;
    AutoFreeKeys free_piProxyKey(piProxyKey);

    Variant* pvPlanetKey = NULL;
    AutoFreeData free_pvPlanetKey(pvPlanetKey);

    // Get empire's preferences
    iErrCode = GetEmpirePlanetIcons(iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);
    RETURN_ON_ERROR(iErrCode);

    // Get map geography information
    iErrCode = GetMapLimits (
        iGameClass,
        iGameNumber,
        iEmpireKey,
        &iMapMinX,
        &iMapMaxX,
        &iMapMinY,
        &iMapMaxY
        );
    
    RETURN_ON_ERROR(iErrCode);

    iNumHorz = iMapMaxX - iMapMinX + 1;
    iNumVert = iMapMaxY - iMapMinY + 1;

    // Allocate the memory for the grid
    MiniMapEntry* pMiniMapEntries = new MiniMapEntry [iNumHorz * iNumVert];
    Assert(pMiniMapEntries);
    Algorithm::AutoDelete<MiniMapEntry> free_pMiniMapEntries(pMiniMapEntries, true);

    MiniMapEntry** ppMiniMap = (MiniMapEntry**) StackAlloc (iNumHorz * sizeof (MiniMapEntry*));

    // Initialize the grid
    for (i = 0; i < iNumHorz; i ++) {

        ppMiniMap[i] = pMiniMapEntries + i * iNumVert;
        for (j = 0; j < iNumVert; j ++) {
            ppMiniMap[i][j].iiIcon = ICON_NONE;
            ppMiniMap[i][j].iOwnerKey = NO_KEY;
            ppMiniMap[i][j].iAlienKey = NO_KEY;
            ppMiniMap[i][j].iPlanetKey = NO_KEY;
            ppMiniMap[i][j].iPlanetProxyKey = NO_KEY;
        }
    }

    // Get visited planets
    iErrCode = GetVisitedPlanetKeys (
        iGameClass, 
        iGameNumber, 
        iEmpireKey,
        &pvPlanetKey,
        &piProxyKey, 
        &iNumPlanets
        );

    RETURN_ON_ERROR(iErrCode);

    Assert(iNumPlanets > 0);

    // Put all planets onto the grid
    for (i = 0; i < iNumPlanets; i ++) {

        int iX, iY;
        unsigned int iPlanetKey = pvPlanetKey[i].GetInteger();

        Variant vValue;

        iErrCode = GetPlanetCoordinates (iGameClass, iGameNumber, iPlanetKey, &iX, &iY);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetPlanetProperty (iGameClass, iGameNumber, iPlanetKey, GameMap::Owner, &vValue);
        RETURN_ON_ERROR(iErrCode);

        Assert(iX >= iMapMinX && iX <= iMapMaxX);
        Assert(iY >= iMapMinY && iY <= iMapMaxY);

        unsigned int iIndexX = iX - iMapMinX;
        unsigned int iIndexY = iMapMaxY - iY;

        Assert(iIndexX < iNumHorz);
        Assert(iIndexY < iNumVert);

        ppMiniMap[iIndexX][iIndexY].iOwnerKey = vValue.GetInteger();
        ppMiniMap[iIndexX][iIndexY].iPlanetKey = iPlanetKey;
        ppMiniMap[iIndexX][iIndexY].iPlanetProxyKey = piProxyKey[i];
        ppMiniMap[iIndexX][iIndexY].iX = iX;
        ppMiniMap[iIndexX][iIndexY].iY = iY;

        switch (vValue.GetInteger()) {
            
        case SYSTEM:

            iErrCode = GetPlanetProperty (iGameClass, iGameNumber, iPlanetKey, GameMap::Annihilated, &vValue);
            RETURN_ON_ERROR(iErrCode);

            if (vValue.GetInteger() == NOT_ANNIHILATED) {
                ppMiniMap[iIndexX][iIndexY].iiIcon = ICON_LIVEPLANET;
            } else {
                ppMiniMap[iIndexX][iIndexY].iiIcon = ICON_DEADPLANET;
            }
            break;

        case INDEPENDENT:

            ppMiniMap[iIndexX][iIndexY].iiIcon = ICON_INDEPENDENT;
            break;

        default:

            iErrCode = GetEmpireProperty (vValue.GetInteger(), SystemEmpireData::AlienKey, &vValue);
            RETURN_ON_ERROR(iErrCode);

            ppMiniMap[iIndexX][iIndexY].iiIcon = ICON_EMPIREPLANET;
            ppMiniMap[iIndexX][iIndexY].iAlienKey = vValue.GetInteger();
            break;
        }
    }

    // Render first row
    OutputText (
        "<table>"\
        "<tr>"\
        "<td></td>"\
        );

    for (i = 0; i < iNumHorz; i ++) {
        OutputText ("<th>");
        m_pHttpResponse->WriteText (iMapMinX + i);
        OutputText ("</th>");
    }
    OutputText (
        "<td></td>"\
        "</tr>"
        );

    // Render all rows
    for (i = 0; i < iNumVert; i ++) {

        OutputText ("<tr><th>");
        m_pHttpResponse->WriteText (iMapMaxY - i);
        OutputText ("</th>");

        for (j = 0; j < iNumHorz; j ++) {
            
            // Render a planet
            OutputText ("<td>");
            if (ppMiniMap[j][i].iiIcon != ICON_NONE) {
                RenderMiniPlanet (ppMiniMap[j][i], iEmpireKey, iLivePlanetKey, iDeadPlanetKey);
            }
            OutputText ("</td>");
        }

        OutputText ("<th>");
        m_pHttpResponse->WriteText (iMapMaxY - i);
        OutputText ("</th></tr>");
    }

    // Render last row
    OutputText (
        "<tr>"\
        "<td></td>"\
        );

    for (i = 0; i < iNumHorz; i ++) {
        OutputText ("<th>");
        m_pHttpResponse->WriteText (iMapMinX + i);
        OutputText ("</th>");
    }

    OutputText (
        "<td></td>"\
        "</tr>"\
        "</table>"
        );

    return iErrCode;
}

void HtmlRenderer::RenderMiniPlanet (const MiniMapEntry& mmEntry, unsigned int iEmpireKey, unsigned int iLivePlanetKey, unsigned int iDeadPlanetKey)
{
    String strPlanetString;

    char pszCoord [MAX_COORDINATE_LENGTH + 1];
    GetCoordinates (mmEntry.iX, mmEntry.iY, pszCoord);

    switch (mmEntry.iiIcon) {

    case ICON_LIVEPLANET:

        GetLivePlanetButtonString (iLivePlanetKey, mmEntry.iPlanetKey, mmEntry.iPlanetProxyKey, pszCoord, "width=\"75%\"", &strPlanetString);
        break;

    case ICON_DEADPLANET:

        GetLivePlanetButtonString (iDeadPlanetKey, mmEntry.iPlanetKey, mmEntry.iPlanetProxyKey, pszCoord, "width=\"75%\"", &strPlanetString);
        break;

    case ICON_EMPIREPLANET:

        GetAlienPlanetButtonString (
            mmEntry.iAlienKey, 
            mmEntry.iOwnerKey, 
            mmEntry.iOwnerKey == iEmpireKey, 
            mmEntry.iPlanetKey, 
            mmEntry.iPlanetProxyKey, 
            pszCoord,
            "width=\"75%\"",
            &strPlanetString
            );

        break;

    case ICON_INDEPENDENT:

        GetIndependentPlanetButtonString (mmEntry.iPlanetKey, mmEntry.iPlanetProxyKey, pszCoord, "width=\"75%\"", &strPlanetString);
        break;
    }

    m_pHttpResponse->WriteText (strPlanetString.GetCharPtr(), strPlanetString.GetLength());
}