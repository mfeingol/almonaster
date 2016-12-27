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

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    // Always submit tech dev requests, regardless of updates
    int iTechKey;
    const char* pszStart;
    if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Tech")) != NULL && 
        (pszStart = pHttpForm->GetName()) != NULL &&
        sscanf (pszStart, "Tech%d", &iTechKey) == 1) {

        m_bRedirectTest = false;

        iErrCode = RegisterNewTechDevelopment (m_iGameClass, m_iGameNumber, m_iEmpireKey, iTechKey);
        if (iErrCode == ERROR_NO_TECHNOLOGY_AVAILABLE)
        {
            iErrCode = OK;
            AddMessage("You have no undeveloped technologies available");
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);
            AddMessage ("You have developed ");
            AppendMessage (SHIP_TYPE_STRING[iTechKey]);
            AppendMessage (" technology");
        }
    }
}

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
if (!(m_iGameState & STARTED)) {
    %><p>You cannot develop new technologies before the game begins<%
} else {

    if (ShouldDisplayGameRatios())
    {
        iErrCode = WriteRatiosString(NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    int iTechDevs, iTechUndevs;

    iErrCode = GetDevelopedTechs (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        &iTechDevs,
        &iTechUndevs
        );
    RETURN_ON_ERROR(iErrCode);

    int iNumAvailableTechs;
    iErrCode = GetNumAvailableTechs (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumAvailableTechs);
    RETURN_ON_ERROR(iErrCode);

    int i, iNumDevKeys = 0, iNumUndevKeys = 0, piDevKey[NUM_SHIP_TYPES], piUndevKey[NUM_SHIP_TYPES];

    ENUMERATE_TECHS(i) {

        if (iTechDevs & TECH_BITS[i]) {
            piDevKey[iNumDevKeys ++] = i;
        }
        else if (iTechUndevs & TECH_BITS[i]) {
            piUndevKey[iNumUndevKeys ++] = i;
        }
    }

    if (iNumAvailableTechs > 0 && iNumUndevKeys > 0) {
        %><p>You can develop <font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><strong><% 
        Write (iNumAvailableTechs);
        %></strong></font> new<%
        if (iNumAvailableTechs == 1) {
            %> technology:<%
        } else {
            %> technologies:<% 
        }
    } else {

        if (iNumUndevKeys > 0) {
            %><p>You cannot develop any new technologies at the moment:<%
        } else {
            %><p>You have developed all possible technologies:<%
        }
    }

    char pszTech[64];
    bool bShowDesc = (m_iSystemOptions & SHOW_TECH_DESCRIPTIONS) != 0;

    %><p><%

    %><table><%
    %><tr><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Developed technologies:</th><%
    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Undeveloped technologies:</th><%
    %></tr><%

    %><tr><%

    // Developed techs
    %><td align="center" valign="top"><%
    %><table width="100%"><%

    for (i = 0; i < iNumDevKeys; i ++) {

        %><tr><%

        %><td align="center"><%
        %><font size="+1" color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><strong><%
        Write (SHIP_TYPE_STRING[piDevKey[i]]); %></strong></font><%
        %></td><%

        if (bShowDesc) {
            %><td><%
            %><font size="-1"><%
            Write (SHIP_TYPE_DESCRIPTION[piDevKey[i]]);
            %></font><%
            %></td><%
        }

        %></tr><%
    }

    %></table><%
    %></td><%

    // Undeveloped techs
    %><td align="center" valign="top"><%

    if (iNumUndevKeys == 0) {
        %><strong>None</strong><%
    } else {

        %><table width="100%"><%

        for (i = 0; i < iNumUndevKeys; i ++) {

            %><tr><%

            %><td><%

            if (iNumAvailableTechs > 0) {

                sprintf(pszTech, "Tech%i", piUndevKey[i]);

                WriteButtonString (
                    m_iButtonKey,
                    m_iButtonAddress,
                    SHIP_TYPE_STRING[piUndevKey[i]],
                    SHIP_TYPE_STRING[piUndevKey[i]],
                    pszTech
                    );

            } else {

                %><font size="+1" color="#<% Write (m_vBadColor.GetCharPtr()); %>"><strong><%
                Write (SHIP_TYPE_STRING[piUndevKey[i]]); %></strong></font><%
            }

            %></td><%

            if (bShowDesc) {
                %><td><%
                %><font size="-1"><%
                Write (SHIP_TYPE_DESCRIPTION[piUndevKey[i]]);
                %></font><%
                %></td><%
            }

            %></tr><%
        }

        %></table><%
    }

    %></td><%
    %></tr><%
    %></table><%
}

iErrCode = CloseGamePage();
RETURN_ON_ERROR(iErrCode);

%>