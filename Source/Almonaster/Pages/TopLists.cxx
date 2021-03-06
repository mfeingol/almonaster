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
iErrCode = InitializeEmpire(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

unsigned int i;

int iTopListPage = 0;
ScoringSystem ssListType = ALMONASTER_SCORE;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (!WasButtonPressed (BID_CANCEL)) {

        if ((pHttpForm = m_pHttpRequest->GetForm ("TopListPage")) == NULL) {
            goto Redirection;
        }
        int iTopListPageSubmit = pHttpForm->GetIntValue();

        switch (iTopListPageSubmit) {

        case 0:

            if (WasButtonPressed (BID_ALMONASTERSCORE)) {
                iTopListPage = 1;
                ssListType = ALMONASTER_SCORE;
            }

            else if (WasButtonPressed (BID_CLASSICSCORE)) {
                iTopListPage = 1;
                ssListType = CLASSIC_SCORE;
            }

            else if (WasButtonPressed (BID_BRIDIERSCORE)) {
                iTopListPage = 1;
                ssListType = BRIDIER_SCORE;
            }

            else if (WasButtonPressed (BID_BRIDIERSCOREESTABLISHED)) {
                iTopListPage = 1;
                ssListType = BRIDIER_SCORE_ESTABLISHED;
            }

            else {
                iTopListPage = 0;
            }

            break;

        case 1:
            {

            }
            break;

        default:
            Assert(false);
        }

    } else {
        m_bRedirectTest = false;
    }
} 

Redirection:
if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
    iErrCode = RedirectOnSubmit(&pageRedirect, &bRedirected);
    RETURN_ON_ERROR(iErrCode);
    if (bRedirected)
    {
        return Redirect(pageRedirect);
    }
}

iErrCode = OpenSystemPage(false);
RETURN_ON_ERROR(iErrCode);

// Individual page stuff starts here
switch (iTopListPage) {

case 0:
Page0:
    {
    %><input type="hidden" name="TopListPage" value="0"><%

    unsigned int iNumEmpires;
    iErrCode = GetNumEmpiresOnServer (&iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    %><p>There <%
    if (iNumEmpires == 1) {
        %>is <strong>1</strong> registered empire<%
    } else {
        %>are <strong><% Write (iNumEmpires); %></strong> registered empires<%
    }
    %> on the server<%

    %><p>View the top <strong><% Write (TOPLIST_SIZE); %></strong> empire<%
    if (TOPLIST_SIZE != 1) {
        %>s<%
    }
    %> in:<%

    %><p><table width="70%"><%

    %><tr><td><font face="sans-serif" size="-1"><%

    %>The <strong>Almonaster Score</strong> is a scoring system designed by Max Attar Feingold. <%
    %>It attempts to give a reasonable <%
    %>approximation of the quality of an empire, as determined by its record on the server. In particular, <%
    %>the Almonaster Score considers the quality and experience of the empires nuked by the empire and the <%
    %>empires who nuked it.<%

    %></font></td><td>&nbsp;</td><td align="center"><% WriteButton (BID_ALMONASTERSCORE); %></td></tr><%

    %><tr><td colspan="3">&nbsp;</td></tr><%

    %><tr><td><font face="sans-serif" size="-1"><%

    %>The <strong>Classic Score</strong> is the primary scoring system used by classic Stellar Crisis. <%
    %>It increases when an <%
    %>empire wins or draws a game or nukes another empire, and decreases when an empire is nuked or ruins <%
    %>out of a game.<%

    %></font></td><td>&nbsp;</td><td align="center"><% WriteButton (BID_CLASSICSCORE); %></td></tr><%


    %><tr><td colspan="3">&nbsp;</td></tr><%


    %><tr><td><font face="sans-serif" size="-1"><%

    %>The <strong>Bridier Score</strong> is a scoring system designed for chess and adapted for Stellar <%
    %>Crisis 3.x by <%
    %>Jerome Zago. It attempts to perform more or less the same evaluation of an <%
    %>empire's quality that the Almonaster Score does, but only for selected one-on-one games. This <%
    %>particular list considers empires with a Bridier Index smaller than <% Write (BRIDIER_TOPLIST_INDEX); %>.<%

    %></font></td><td>&nbsp;</td><td align="center"><% WriteButton (BID_BRIDIERSCORE); %></td></tr><%


    %><tr><td colspan="3">&nbsp;</td></tr><%


    %><tr><td><font face="sans-serif" size="-1"><%

    %>The <strong>Established Bridier Score</strong> is is the same as the Bridier Score, with one exception: <%
    %>it only considers <%
    %>'established' empires, which are those with a Bridier Index of <%
    Write (BRIDIER_ESTABLISHED_TOPLIST_INDEX); %>.<%

    %></font></td><td>&nbsp;</td><td align="center"><% WriteButton (BID_BRIDIERSCOREESTABLISHED); %></td></tr><%


    %></table><%

    }
    break;

case 1:
    {

    unsigned int* piEmpireKey = NULL, iNumEmpires;
    AutoFreeKeys free_piEmpireKey(piEmpireKey);

    const char* pszTableColor = m_vTableColor.GetCharPtr();
    unsigned int iDataColumn1 = -1, iDataColumn2 = -1;

    iErrCode = GetTopList(ssListType, &piEmpireKey, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    if (iNumEmpires == 0) {
        %><p><strong>No empires are currently on the <% Write (TOPLIST_NAME [ssListType]); %> list</strong><%
        goto Page0;
    }

    %><input type="hidden" name="TopListPage" value="1"><% 

    %><p><h3>Top <strong><% Write (iNumEmpires); %></strong> empire<%
    if (iNumEmpires != 1) {
        %>s<%
    }
    %> on the <% Write (TOPLIST_NAME [ssListType]); %> list:</h3><%

    %><p><table width="90%" cellspacing="1" cellpadding="2"><%

    %><tr><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Rank</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Empire</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Icon</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Name</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Wins</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Nukes</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Nuked</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Draws</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Ruins</th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>"><%

    switch (ssListType) {

    case ALMONASTER_SCORE:
        iDataColumn1 = SystemEmpireData::iAlmonasterScore;
        iDataColumn2 = SystemEmpireData::iAlmonasterScoreSignificance;
        %>Score</th><%
        %><th align="center" bgcolor="<% Write (pszTableColor); %>">Significance</th><%
        break;

    case CLASSIC_SCORE:
        iDataColumn1 = SystemEmpireData::iClassicScore;
        %>Score<%
        break;

    case BRIDIER_SCORE:
    case BRIDIER_SCORE_ESTABLISHED:
        iDataColumn1 = SystemEmpireData::iBridierRank;
        iDataColumn2 = SystemEmpireData::iBridierIndex;

        // Best effort ask for Bridier time bomb scan
        iErrCode = TriggerBridierTimeBombIfNecessary();
        Assert(iErrCode == OK);

        %>Rank</th><%
        %><th align="center" bgcolor="<% Write (pszTableColor); %>">Index<%
        break;

    default:
        Assert(false);
        break;
    }
    %></th><%
    %><th align="center" bgcolor="<% Write (pszTableColor); %>">Games</th><%    
    %></tr><%

    String strName, strEmail;

    iErrCode = CacheEmpireAndActiveGames(piEmpireKey, iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumEmpires; i ++)
    {
        unsigned int iNumActiveGames;
        Variant* pvEmpData = NULL;
        AutoFreeData free_pvEmpData(pvEmpData);

        iErrCode = GetEmpireData(piEmpireKey[i], &pvEmpData, &iNumActiveGames);
        RETURN_ON_ERROR(iErrCode);

        %><tr><td align="center"><strong><% Write (i + 1); %></strong></td><%
        %><td align="center"><% Write (pvEmpData[SystemEmpireData::iName].GetCharPtr()); %></td><%
        %><td align="center"><%

        char pszProfile[128 + MAX_EMPIRE_NAME_LENGTH];
        sprintf(pszProfile, "View the profile of %s", pvEmpData[SystemEmpireData::iName].GetCharPtr());

        iErrCode = WriteProfileAlienString (
            pvEmpData[SystemEmpireData::iAlienKey].GetInteger(),
            pvEmpData[SystemEmpireData::iAlienAddress].GetInteger(),
            piEmpireKey[i],
            pvEmpData[SystemEmpireData::iName].GetCharPtr(),
            0,
            "ProfileLink",
            pszProfile,
            true,
            true
            );
        RETURN_ON_ERROR(iErrCode);

        %></td><td align="center"><%

        HTMLFilter(pvEmpData[SystemEmpireData::iRealName].GetCharPtr(), &strName, 0, false);
        if (!strName.IsBlank()) {

            HTMLFilter(pvEmpData[SystemEmpireData::iEmail].GetCharPtr(), &strEmail, 0, false);

            if (!strEmail.IsBlank()) {
                %><a href="mailto:<% Write (strEmail.GetCharPtr(), strEmail.GetLength()); %>"><%
            }

            Write (strName.GetCharPtr(), strName.GetLength());

            if (!strEmail.IsBlank()) {
                %></a><%
            }
        }

        %></td><td align="center"><% Write (pvEmpData[SystemEmpireData::iWins].GetInteger());
        %></td><td align="center"><% Write (pvEmpData[SystemEmpireData::iNukes].GetInteger());
        %></td><td align="center"><% Write (pvEmpData[SystemEmpireData::iNuked].GetInteger());
        %></td><td align="center"><% Write (pvEmpData[SystemEmpireData::iDraws].GetInteger());
        %></td><td align="center"><% Write (pvEmpData[SystemEmpireData::iRuins].GetInteger());

        Assert(iDataColumn1 >= 0);
        %></td><td align="center"><% Write (pvEmpData[iDataColumn1]);

        switch (ssListType) {

        case ALMONASTER_SCORE:
            Assert(iDataColumn2 >= 0);
            %></td><td align="center"><% Write (pvEmpData[iDataColumn2].GetInteger());
            break;

        case BRIDIER_SCORE:
        case BRIDIER_SCORE_ESTABLISHED:
            Assert(iDataColumn2 >= 0);
            %></td><td align="center"><% Write(pvEmpData[iDataColumn2]);
            break;
        }
        %></td><%
        %></td><td align="center"><% Write(iNumActiveGames); %></td><%
        %></tr><%
    }
    %></table><%

    NotifyProfileLink();

    }
    break;

default:
    Assert(false);
    break;
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>