<%

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

int iNumGames;
Variant** ppvGameData = NULL;
AutoFreeData free_ppvGameData(ppvGameData);

iErrCode = GetSystemLatestGames (&iNumGames, &ppvGameData);
RETURN_ON_ERROR(iErrCode);

if (iNumGames == 0)
{
    %><p><h3>No games have been recorded on this server</h3><%
}
else
{
    int i;

    UTCTime* ptTime = (UTCTime*) StackAlloc (iNumGames * sizeof (UTCTime));
    Variant** ppvData = (Variant**) StackAlloc (iNumGames * sizeof (Variant*));

    // Sort by timestamp of end
    for (i = 0; i < iNumGames; i ++) {
        ptTime[i] = ppvGameData[i][SystemLatestGames::iEnded].GetInteger64();
        ppvData[i] = ppvGameData[i];
    }

    Algorithm::QSortTwoDescending<UTCTime, Variant*> (ptTime, ppvData, iNumGames);

    // Start off page
    if (iNumGames != 1) {
        %><p><h3>These are the latest <% Write (iNumGames); %> games recorded on the server:</h3><%
    } else {
        %><p><h3>This is the latest game recorded on the server:</h3><%
    }

    %><p><table width="90%"><%
    %><tr><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Game</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Created</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Ended</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Updates</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Result</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Survivors</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Losers</th><%

    %></tr><%

    for (i = 0; i < iNumGames; i ++) {

        const char* pszList;

        int iSec, iMin, Hour, iDay, iMonth, iYear;
        DayOfWeek dayOfWeek;
        char pszDate [64];

        %><tr><%

        // Name
        %><td align="center"><%
        Write (ppvData[i][SystemLatestGames::iName].GetCharPtr()); %> <%
        Write (ppvData[i][SystemLatestGames::iNumber].GetInteger());
        %></td><%

        // Created
        %><td align="center"><%

        Time::GetDate (
            ppvData[i][SystemLatestGames::iCreated].GetInteger64(),
            &iSec, &iMin, &Hour, &dayOfWeek, &iDay, &iMonth, &iYear
            );

        sprintf(pszDate, "%s, %i %s %i", 
            Time::GetAbbreviatedDayOfWeekName (dayOfWeek), iDay, Time::GetAbbreviatedMonthName (iMonth), iYear);

        m_pHttpResponse->WriteText (pszDate);

        %></td><%

        // Ended
        %><td align="center"><%

        Time::GetDate (
            ppvData[i][SystemLatestGames::iEnded].GetInteger64(),
            &iSec, &iMin, &Hour, &dayOfWeek, &iDay, &iMonth, &iYear
            );

        sprintf(pszDate, "%s, %i %s %i", 
            Time::GetAbbreviatedDayOfWeekName (dayOfWeek), iDay, Time::GetAbbreviatedMonthName (iMonth), iYear);

        m_pHttpResponse->WriteText (pszDate);

        %></td><%

        // Updates
        %><td align="center"><%
        Write (ppvData[i][SystemLatestGames::iUpdates].GetInteger());
        %></td><%

        // Result
        %><td align="center"><%

        switch (ppvData[i][SystemLatestGames::iResult].GetInteger()) {

        case GAME_RESULT_RUIN:
            %>Ruin<%
            break;

        case GAME_RESULT_WIN:
            %>Win<%
            break;

        case GAME_RESULT_DRAW:
            %>Draw<%
            break;

        default:
            %>None<%
            break;
        }
        %></td><%

        // Survivors
        %><td align="center" width="20%"><%

        pszList = ppvData[i][SystemLatestGames::iWinners].GetCharPtr();

        if (!String::IsBlank (pszList)) {
            Write (pszList);
        }
        %></td><%

        // Losers
        %><td align="center" width="20%"><%

        pszList = ppvData[i][SystemLatestGames::iLosers].GetCharPtr();

        if (!String::IsBlank (pszList)) {
            Write (pszList);
        }
        %></td><%


        %></tr><%
    }

    %></table><%
}


iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>