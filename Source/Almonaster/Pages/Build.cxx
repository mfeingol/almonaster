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

unsigned int i, j;

Variant vPlanetName;
String strFilter;
char* pszRet;

BuildLocation* pblBuildLocation = NULL;
Algorithm::AutoDelete<BuildLocation> del_pblBuildLocation (pblBuildLocation, true);

String* pstrLocationName = NULL;
Algorithm::AutoDelete<String> del_pstrLocationName(pstrLocationName, true);

bool bMapGenerated = (m_iGameState & GAME_MAP_GENERATED) != 0;

struct NewFleetKey {
    unsigned int iPlanetKey;
    unsigned int iFleetKey;
};

NewFleetKey nfkNewFleet [NUM_SHIP_TYPES];
unsigned int iNumNewFleets = 0;

if (m_bOwnPost && !m_bRedirection) {

    // Make sure cancel wasn't pressed
    // Discard submission if update counts don't match
    if (bMapGenerated && !WasButtonPressed (BID_CANCEL) && m_iNumNewUpdates == m_iNumOldUpdates)
    {
        int iNumShips, iTechKey, iShipBR, iLocationKey;
        unsigned int iFleetKey, iNumShipTypes, iPlanetKey;
        String strTechName;
        const char* pszShipName;

        // Get num techs
        if ((pHttpForm = m_pHttpRequest->GetForm ("NumTechs")) == NULL) {
            goto Redirection;
        }
        iNumShipTypes = pHttpForm->GetIntValue();
        if (iNumShipTypes > NUM_SHIP_TYPES) {
            goto Redirection;
        }

        int iNumShipsBuilt;
        char pszForm [256];
        Variant vFleetName;

        for (i = 0; i < iNumShipTypes; i ++) {

            sprintf(pszForm, "NumShips%i", i);

            // Num ships
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iNumShips = pHttpForm->GetIntValue();

            if (iNumShips > 0) {

                int iX, iY;
                String strFleetName, strPlanetName;

                // We're building ships, so get the tech key
                sprintf(pszForm, "TechKey%i", i);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                iTechKey = pHttpForm->GetIntValue();

                if (iTechKey < FIRST_SHIP || iTechKey > LAST_SHIP) {
                    goto Redirection;
                }

                // Get the BR
                sprintf(pszForm, "ShipBR%i", i);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                iShipBR = pHttpForm->GetIntValue();

                if (iShipBR < 1) {
                    goto Redirection;
                }

                // Get ship name
                sprintf(pszForm, "ShipName%i", i);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                pszShipName = pHttpForm->GetValue();
                if (pszShipName == NULL) {
                    pszShipName = "";
                }

                if (!ShipOrFleetNameFilter (pszShipName)) {
                    AddMessage ("Invalid ship name for ");
                    AppendMessage (SHIP_TYPE_STRING [iTechKey]);
                    continue;
                }

                if (strlen (pszShipName) > MAX_SHIP_NAME_LENGTH) {
                    AddMessage ("The ship name for ");
                    AppendMessage (SHIP_TYPE_STRING [iTechKey]);
                    AppendMessage (" is too long");
                    continue;
                }

                // Get location key
                sprintf(pszForm, "ShipLocation%i", i);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                iLocationKey = pHttpForm->GetIntValue();

                // Get planet key
                sprintf(pszForm, "LocPlanetKey%i", iLocationKey);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                iPlanetKey = pHttpForm->GetUIntValue();

                // Get fleet key
                sprintf(pszForm, "LocFleetKey%i", iLocationKey);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                iFleetKey = pHttpForm->GetUIntValue();

                if (iFleetKey == FLEET_NEWFLEETKEY) {

                    for (j = 0; j < iNumNewFleets; j ++) {

                        if (nfkNewFleet[j].iPlanetKey == iPlanetKey) {
                            iFleetKey = nfkNewFleet[j].iFleetKey;
                            break;
                        }
                    }

                    if (iFleetKey == FLEET_NEWFLEETKEY)
                    {
                        iErrCode = HtmlCreateRandomFleet(iPlanetKey, &iFleetKey);
                        RETURN_ON_ERROR(iErrCode);

                        Assert(iNumNewFleets < NUM_SHIP_TYPES);
                        nfkNewFleet [iNumNewFleets].iPlanetKey = iPlanetKey;
                        nfkNewFleet [iNumNewFleets].iFleetKey = iFleetKey;
                        iNumNewFleets ++;
                    }
                }

                bool bBuildReduced;
                iErrCode = DoesPlanetExist(m_iGameClass, m_iGameNumber, iPlanetKey, &bBuildReduced);
                RETURN_ON_ERROR(iErrCode);

                if (!bBuildReduced)
                {
                    continue;
                }

                iErrCode = GetPlanetCoordinates(m_iGameClass, m_iGameNumber, iPlanetKey, &iX, &iY);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = GetPlanetName(m_iGameClass, m_iGameNumber, iPlanetKey, &vPlanetName);
                RETURN_ON_ERROR(iErrCode);

                HTMLFilter (vPlanetName.GetCharPtr(), &strPlanetName, 0, false);

                if (iFleetKey == NO_KEY) {

                    vFleetName = (const char*) NULL;

                } else {

                    iErrCode = GetFleetProperty (
                        m_iGameClass,
                        m_iGameNumber,
                        m_iEmpireKey,
                        iFleetKey,
                        GameEmpireFleets::Name,
                        &vFleetName
                        );

                    RETURN_ON_ERROR(iErrCode);

                    HTMLFilter (vFleetName.GetCharPtr(), &strFleetName, 0, false);
                }

                iErrCode = BuildNewShips (
                    m_iGameClass,
                    m_iGameNumber,
                    m_iEmpireKey, 
                    iTechKey,
                    iNumShips,
                    pszShipName,
                    (float) iShipBR,
                    iPlanetKey,
                    iFleetKey,
                    &iNumShipsBuilt,
                    &bBuildReduced
                    );

                iErrCode = HandleBuildNewShipsResult(
                    iErrCode,
                    iNumShipsBuilt,
                    iShipBR,
                    iTechKey,
                    vPlanetName.GetCharPtr(),
                    iX,
                    iY,
                    vFleetName.GetCharPtr(),
                    bBuildReduced
                    );
                RETURN_ON_ERROR(iErrCode);
            }
        }

        // Check for a fleet name
        if ((pHttpForm = m_pHttpRequest->GetForm ("NewFleetName")) == NULL) {
            goto Redirection;
        }

        const char* pszTemp;
        if ((pszTemp = pHttpForm->GetValue()) != NULL) {

            if (!ShipOrFleetNameFilter (pszTemp)) {
                AddMessage ("Blank fleet names are not allowed");
            } else {

                if (strlen (pszTemp) > MAX_FLEET_NAME_LENGTH) {
                    AddMessage ("The new fleet's name is too long");
                } else {

                    unsigned int iDontCare;

                    if ((pHttpForm = m_pHttpRequest->GetForm ("NewFleetLocation")) == NULL) {
                        goto Redirection;
                    }
                    iPlanetKey = pHttpForm->GetIntValue();

                    iErrCode = CreateNewFleet (
                        m_iGameClass,
                        m_iGameNumber,
                        m_iEmpireKey,
                        pszTemp,
                        iPlanetKey,
                        &iDontCare
                        );

                    iErrCode = HandleCreateNewFleetResult(iErrCode, pszTemp);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }
    }
}

Redirection:
if (m_bRedirectTest)
{
    GameCheck(RedirectOnSubmitGame(&pageRedirect, &bRedirected));
    if (bRedirected)
    {
        return Redirect (pageRedirect);
    }
}

iErrCode = OpenGamePage();
RETURN_ON_ERROR(iErrCode);

int iGameClassOptions;
iErrCode = GetGameClassOptions (m_iGameClass, &iGameClassOptions);
RETURN_ON_ERROR(iErrCode);

//
// Individual page stuff starts here
//

int iBR;

unsigned int iNumLocations = 0, iNumTechs = 0;

Variant vMaxNumShips, vTemp;

char pszLocation [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH + MAX_FLEET_NAME_LENGTH + 64];

if (!bMapGenerated)
{
    %><input type="hidden" name="NumTechs" value="0"><%

    if (iGameClassOptions & GENERATE_MAP_FIRST_UPDATE) {
        %><p>You cannot build ships or fleets before the map is generated<%
    } else {
        %><p>You cannot build ships or fleets before the game starts<%
    }

    goto Close;
}

if (ShouldDisplayGameRatios())
{
    RatioInformation ratInfo;
    iErrCode = WriteRatiosString(&ratInfo);
    RETURN_ON_ERROR(iErrCode);

    iBR = ratInfo.iBR;
}
else
{
    iErrCode = GetEmpireBR(m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR);
    RETURN_ON_ERROR(iErrCode);
}

if (iBR < 1)
{
    %><p>You do not have the minimum tech level required to build ships<%
    %><input type="hidden" name="NumTechs" value="0"><%
    goto BuildFleet;
}

// Check ship limits
iErrCode = GetGameClassProperty(m_iGameClass, SystemGameClassData::MaxNumShips, &vMaxNumShips);
RETURN_ON_ERROR(iErrCode);

if (vMaxNumShips.GetInteger() != INFINITE_SHIPS)
{
    int iNumShips;
    iErrCode = GetNumShips(m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumShips);
    RETURN_ON_ERROR(iErrCode);

    if (iNumShips >= vMaxNumShips.GetInteger())
    {
        %><p>You have reached the limit of ships that can be built in this game<%
        %><input type="hidden" name="NumTechs" value="0"><%
        goto BuildFleet;
    }
}

// Get build locations
iErrCode = GetBuildLocations(m_iGameClass, m_iGameNumber, m_iEmpireKey, NO_KEY, &pblBuildLocation, &iNumLocations);
RETURN_ON_ERROR(iErrCode);

if (iNumLocations == 0)
{
    %><p>You have no planets capable of building ships<%
    %><input type="hidden" name="NumTechs" value="0"><%
    goto BuildFleet;
}

%><input type="hidden" name="NumLocations" value="<% Write (iNumLocations); %>"><%

// Get default builder planet
int iValue, iRealPlanet;

iErrCode = GetEmpireDefaultBuilderPlanet (
    m_iGameClass,
    m_iGameNumber,
    m_iEmpireKey,
    &iValue,
    &iRealPlanet
    );
RETURN_ON_ERROR(iErrCode);

// Build location strings
pstrLocationName = new String[iNumLocations];
Assert(pstrLocationName);

for (i = 0; i < iNumLocations; i ++)
{
    int iX, iY;
    String strPlanetName, strFleetName;

    unsigned int iPlanetKey = pblBuildLocation[i].iPlanetKey;
    unsigned int iFleetKey  = pblBuildLocation[i].iFleetKey;

    iErrCode = GetPlanetName(m_iGameClass, m_iGameNumber, iPlanetKey, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    
    pszRet = String::AtoHtml(vTemp.GetCharPtr(), &strPlanetName, 0, false);
    Assert(pszRet);

    iErrCode = GetPlanetCoordinates(m_iGameClass, m_iGameNumber, iPlanetKey, &iX, &iY);
    RETURN_ON_ERROR(iErrCode);

    switch (iFleetKey)
    {
    case NO_KEY:

        snprintf (
            pszLocation,
            sizeof (pszLocation),
            "%s (%i,%i)",
            strPlanetName.GetCharPtr(),
            iX,
            iY
            );
        break;

    case FLEET_NEWFLEETKEY:

        snprintf (
            pszLocation,
            sizeof (pszLocation),
            "%s (%i,%i) in new fleet",
            strPlanetName.GetCharPtr(),
            iX,
            iY
            );
        break;
    
    default:

        iErrCode = GetFleetProperty(m_iGameClass, m_iGameNumber, m_iEmpireKey, iFleetKey, GameEmpireFleets::Name, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        
        pszRet = String::AtoHtml(vTemp.GetCharPtr(), &strFleetName, 0, false);
        Assert(pszRet);

        snprintf (
            pszLocation,
            sizeof (pszLocation),
            "%s (%i,%i) in fleet %s",
            strPlanetName.GetCharPtr(),
            iX,
            iY,
            strFleetName.GetCharPtr()
            );
        break;
    }

    pstrLocationName[i] = pszLocation;
    Assert(pstrLocationName[i].GetCharPtr());

    %><input type="hidden" name="LocPlanetKey<% Write (i); %>" value="<% Write (iPlanetKey); %>"><%
    %><input type="hidden" name="LocFleetKey<% Write (i); %>" value="<% Write (iFleetKey); %>"><%
}

//
// Get techs we're allowed to build
//
int iTechDevs, iTechUndevs;
unsigned int iMaxNumShipsBuiltAtOnce;

iErrCode = GetDevelopedTechs (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iTechDevs, &iTechUndevs);
RETURN_ON_ERROR(iErrCode);

iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::MaxNumShipsBuiltAtOnce, &vTemp);
RETURN_ON_ERROR(iErrCode);

iMaxNumShipsBuiltAtOnce = vTemp.GetInteger();
iNumTechs = GetNumTechs (iTechDevs);

if (iNumTechs > 0)
{
    %><input type="hidden" name="NumTechs" value="<% Write (iNumTechs); %>"><p><%

    bool bVisible = true;

    iErrCode = GetGameClassVisibleBuilds (m_iGameClass, &bVisible);
    RETURN_ON_ERROR(iErrCode);

    if (bVisible) {
        %>Builds are <strong>visible</strong><%
    } else {
        %>Builds are <strong>invisible</strong><%
    }

    if (vMaxNumShips.GetInteger() != INFINITE_SHIPS) {

        %>. The limit is <strong><%
        Write (vMaxNumShips.GetInteger());
        %></strong> ship<%
        if (vMaxNumShips.GetInteger() != 1) {
            %>s<%
        }
        %>.<%
    }

    //
    // Render table header
    //

    %><p><table width="70%"><%
    %><tr><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="left">Type</th><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Number</th><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Name</th><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">BR</th><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Location:</th><%
    %></tr><%

    Variant vDefaultShipName;
    unsigned int iNumBuildableTechs = 0, iMin = min (iMaxNumShipsBuiltAtOnce + 1, 16);

    ENUMERATE_TECHS (i) {

        if (iTechDevs & TECH_BITS[i]) {

            iErrCode = GetDefaultEmpireShipName(m_iEmpireKey, i, &vDefaultShipName);
            RETURN_ON_ERROR(iErrCode);

            HTMLFilter(vDefaultShipName.GetCharPtr(), &strFilter, 0, false);

            %><input type="hidden" name="TechKey<% Write (iNumBuildableTechs); %>"<%
            %> value="<% Write (i); %>"><%

            %><tr><%
            %><td><% Write (SHIP_TYPE_STRING[i]);
            %></td><%

            %><td align="center"><%
            %><select name="NumShips<% Write (iNumBuildableTechs); %>" size="1"><%

            for (j = 0; j < iMin; j ++) {
                %><option><% Write (j); %></option><%
            }

            if (iMaxNumShipsBuiltAtOnce > 15) {
                for (j = 20; j <= iMaxNumShipsBuiltAtOnce; j += 10) {
                    %><option><% Write (j); %></option><%
                }
            }
            %></select><%
            %></td><%

            %><td align="center"><%
            %><input type="text" size="20" maxlength="20" name="ShipName<% Write (iNumBuildableTechs); %>"<%
            %> value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength()); %>"><%
            %></td><%

            %><td align="center"><%
            if (iBR == 1) {
                
                Write (iBR);
                %><input type="hidden" name="ShipBR<% Write (iNumBuildableTechs); %>" value="1"><%
            
            } else {

                %><select name="ShipBR<% Write (iNumBuildableTechs); %>" size="1"><%

                if (iBR < 100) {

                    for (j = 1; j < (unsigned int) iBR; j ++) {
                        %><option><% Write (j); %></option><%
                    }
                    %><option selected><% Write (iBR); %></select><%

                } else {

                    for (j = 1; j < 10; j ++) {
                        %><option><% Write (j); %></option><%
                    }

                    int iStep = iBR / 100;

                    for (j = 10; j < (unsigned int) iBR - 10; j += iStep) {
                        %><option><% Write (j); %></option><%
                    }

                    for (j = iBR - 10; j < (unsigned int) iBR; j ++) {
                        %><option><% Write (j); %></option><%
                    }

                    %><option selected><% Write (iBR); %></select><%
                }
            }
            %></td><td align="center"><%

            //
            // Hack to provide one option with no drop-down box for immobile ships
            // when there's one planet to build on, but multiple fleets on that planet
            //
            unsigned int iOneLocIndex = 0, iRealLocations = iNumLocations;

            if (iNumLocations > 1 && !IsMobileShip (i)) {

                iRealLocations = 1;

                unsigned iNonFleetLocations = 0;
                for (j = 0; j < iNumLocations; j ++) {

                    if (pblBuildLocation[j].iFleetKey == NO_KEY) {

                        iOneLocIndex = j;
                        if (++ iNonFleetLocations == 2) {
                            iRealLocations = iNumLocations;
                            break;
                        }
                    }
                }
            }

            if (iRealLocations == 1) {
            
                Write (pstrLocationName[iOneLocIndex].GetCharPtr(), pstrLocationName[iOneLocIndex].GetLength());
                %><input type="hidden" name="ShipLocation<% Write(iNumBuildableTechs); %>" <%
                %>value="<% Write (iOneLocIndex); %>"><%
            
            } else {

                %><select name="ShipLocation<% Write(iNumBuildableTechs); %>" size="1"><%

                for (j = 0; j < iNumLocations; j ++) {

                    if (pblBuildLocation[j].iFleetKey == NO_KEY || (IsMobileShip (i))) {
                        
                        %><option<%
                        if (pblBuildLocation[j].iPlanetKey == (unsigned int) iRealPlanet &&
                            pblBuildLocation[j].iFleetKey == NO_KEY) {
                            %> selected<%
                        }
                        %> value="<% Write (j); %>"><%
                        Write (pstrLocationName[j].GetCharPtr(), pstrLocationName[j].GetLength());
                        %></option><%
                    }
                }
                %></select><%
            }
            %></td></tr><%

            iNumBuildableTechs ++;
        }
    } 

    Assert(iNumBuildableTechs == iNumTechs);

    %></table><p><%
}

BuildFleet:

// Get fleet locations
int* piPlanetKey, iNumPlanets;
iErrCode = GetNewFleetLocations(m_iGameClass, m_iGameNumber, m_iEmpireKey, &piPlanetKey, &iNumPlanets);
RETURN_ON_ERROR(iErrCode);

if (iNumPlanets > 0)
{
    String strPlanetName;
    Algorithm::AutoDelete<int> autopiPlanetKey (piPlanetKey, true);

    %><p><center>Create a new fleet:<%
    %><p><%
    %><table><%
    %><tr><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Name</th><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Location</th><%
    %></tr><%
    %><tr><%
    %><td><%
    %><input type="text" size="20" maxlength="<% Write (MAX_FLEET_NAME_LENGTH); %>" name="NewFleetName"><%
    %></td><%
    %><td><%

    int iX, iY;

    if (iNumPlanets == 1)
    {
        iErrCode = GetPlanetName (m_iGameClass, m_iGameNumber, piPlanetKey[0], &vTemp);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piPlanetKey[0], &iX, &iY);
        RETURN_ON_ERROR(iErrCode);

        pszRet = String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false);
        Assert(pszRet);

        Write (strPlanetName.GetCharPtr(), strPlanetName.GetLength());
        %> (<% Write (iX); %>,<% Write (iY); %>)<%
        %><input type="hidden" name="NewFleetLocation" value="<% Write (piPlanetKey[0]); %>"><%
    
    } else {

        %><select name="NewFleetLocation"><%
        for (i = 0; i < (unsigned int) iNumPlanets; i ++) {

            iErrCode = GetPlanetName (m_iGameClass, m_iGameNumber, piPlanetKey[i], &vTemp);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piPlanetKey[i], &iX, &iY);
            RETURN_ON_ERROR(iErrCode);

            pszRet = String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false);
            Assert(pszRet);

            %><option value="<% Write (piPlanetKey[i]); %>"><%
            Write (strPlanetName.GetCharPtr(), strPlanetName.GetLength());
            %> (<% Write (iX); %>,<% Write (iY); %>)<%
            %></option><% 
        }
        %></select><%
    }
    %></td></tr></table><p><%
    WriteButton (BID_CANCEL);
}

Close:

iErrCode = CloseGamePage();
RETURN_ON_ERROR(iErrCode);

%>