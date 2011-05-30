<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

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

int iErrCode = OK;

INITIALIZE_EMPIRE

//if (m_bOwnPost && !m_bRedirection) {
//}

SYSTEM_REDIRECT_ON_SUBMIT

SYSTEM_OPEN (false)

int iNumNukes;
Variant** ppvNukeData;

if (g_pGameEngine->GetSystemNukeHistory (&iNumNukes, &ppvNukeData) != OK) {

    %><p><strong>The latest nukes could not be read</strong><%
}

else if (iNumNukes == 0) {

    %><p><h3>No nukes have been recorded on this server</h3><%
}

else {

    int i;

    NotifyProfileLink();

    char pszDateString [OS::MaxDateLength];
    char pszEmpire [256 + MAX_EMPIRE_NAME_LENGTH];

    UTCTime* ptTime = (UTCTime*) StackAlloc (iNumNukes * sizeof (UTCTime));
    Variant** ppvData = (Variant**) StackAlloc (iNumNukes * sizeof (Variant*));

    // Sort by timestamp
    for (i = 0; i < iNumNukes; i ++) {
        ptTime[i] = ppvNukeData[i][SystemNukeList::TimeStamp].GetUTCTime();
        ppvData[i] = ppvNukeData[i];
    }

    Algorithm::QSortTwoDescending<UTCTime, Variant*> (ptTime, ppvData, iNumNukes);

    if (iNumNukes != 1) {
        %><p><h3>These are the latest <% Write (iNumNukes); %> nukes recorded on the server:</h3><%
    } else {
        %><p><h3>This is the latest nuke recorded on the server:</h3><%
    }

    %><p><table width="90%"><%
    %><tr><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Nuker</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Icon</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Nuked</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Icon</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Game</th><%

    %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center"><%
    %>Time</th></tr><%

    for (i = 0; i < iNumNukes; i ++) {

        %><tr><%

        %><td align="center"><strong><%
        Write (ppvData[i][SystemNukeList::NukerEmpireName].GetCharPtr()); %></strong></td><%

        %><td align="center"><%

        sprintf (
            pszEmpire, 
            "View the profile of %s",
            ppvData[i][SystemNukeList::NukerEmpireName].GetCharPtr()
            );

        WriteProfileAlienString (
            ppvData[i][SystemNukeList::NukerAlienKey].GetInteger(), 
            ppvData[i][SystemNukeList::NukerEmpireKey].GetInteger(),
            ppvData[i][SystemNukeList::NukerEmpireName].GetCharPtr(),
            0,
            "ProfileLink",
            pszEmpire,
            true,
            true
            );

        %></td><%

        %><td align="center"><strong><%
        Write (ppvData[i][SystemNukeList::NukedEmpireName].GetCharPtr()); %></strong></td><%

        %><td align="center"><%

        sprintf (
            pszEmpire, 
            "View the profile of %s",
            ppvData[i][SystemNukeList::NukedEmpireName].GetCharPtr()
            );

        WriteProfileAlienString (
            ppvData[i][SystemNukeList::NukedAlienKey].GetInteger(), 
            ppvData[i][SystemNukeList::NukedEmpireKey].GetInteger(),
            ppvData[i][SystemNukeList::NukedEmpireName].GetCharPtr(),
            0,
            "ProfileLink",
            pszEmpire,
            true,
            true
            );

        %></td><%

        %><td align="center"><% Write (ppvData[i][SystemNukeList::GameClassName].GetCharPtr()); %> <%
        Write (ppvData[i][SystemNukeList::GameNumber].GetInteger()); %></td><%

        %><td align="center"><%

        iErrCode = Time::GetDateString (
            ppvData[i][SystemNukeList::TimeStamp].GetUTCTime(), 
            pszDateString
            );

        if (iErrCode == OK) {
            m_pHttpResponse->WriteText (pszDateString);
        } else {
            OutputText ("Unknown");
        }

        %></td><%

        %></tr><%
    }

    %></table><%

    g_pGameEngine->FreeData (ppvNukeData);
}


SYSTEM_CLOSE

%>