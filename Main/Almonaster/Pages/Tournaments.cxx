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

IHttpForm* pHttpForm;

int iTournamentPage = 0;

if (m_iReserved != NO_KEY)
{
    m_iTournamentKey = m_iReserved;
    if (m_iTournamentKey != NO_KEY)
    {
        iTournamentPage = 1;
    }
}

if (m_bOwnPost && !m_bRedirection) {

    const char* pszStart;

    if (WasButtonPressed (BID_CANCEL)) {
        m_bRedirectTest = false;
        goto Redirection;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentsPage")) == NULL) {
        goto Redirection;
    }
    int iTournamentsPageSubmit = pHttpForm->GetIntValue();

    //
    switch (iTournamentsPageSubmit) {

    case 0:

        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewTourneyInfo")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "ViewTourneyInfo%d", &m_iTournamentKey) == 1) {

            m_bRedirectTest = false;
            iTournamentPage = 1;
            goto Redirection;
        }

        break;

    case 1:

        if (WasButtonPressed (BID_JOIN)) {

            iErrCode = InviteSelfIntoTournament (m_iTournamentKey, m_iEmpireKey);
            if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT)
            {
                AddMessage ("You are already in the tournament");
            }
            else if (iErrCode == ERROR_TOURNAMENT_DOES_NOT_EXIST)
            {
                AddMessage ("That tournament no longer exists");
            }
            else
            {
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("A request to join the tournament was sent to the tournament owner");
            }

            // Figure out where to redirect next
            unsigned int iTourneyOwner;
            iErrCode = GetTournamentOwner (m_iTournamentKey, &iTourneyOwner);
            RETURN_ON_ERROR(iErrCode);
                    
            if (iTourneyOwner == m_iEmpireKey)
            {
                return Redirect(PERSONAL_TOURNAMENTS);
            }

            iTournamentPage = 1;
            m_bRedirectTest = false;
        }

        if (WasButtonPressed (BID_QUIT))
        {
            iErrCode = DeleteEmpireFromTournament (m_iTournamentKey, m_iEmpireKey);
            if (iErrCode == ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT) {
                AddMessage ("Your empire was no longer in the tournament");
            } else if (iErrCode == ERROR_EMPIRE_IS_IN_GAMES) {
                AddMessage ("Your empire could not be deleted from the tournament because it is still in a tournament game");
            } else {
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Your empire was deleted from the tournament");

                // Figure out where to redirect next
                unsigned int iTourneyOwner;
                iErrCode = GetTournamentOwner(m_iTournamentKey, &iTourneyOwner);
                RETURN_ON_ERROR(iErrCode);
                    
                if (iTourneyOwner == m_iEmpireKey) {
                    return Redirect(PERSONAL_TOURNAMENTS);
                }
            }

            iTournamentPage = 1;
            m_bRedirectTest = false;
        }

        break;

    default:
        break;
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
unsigned int* piTournamentKey = NULL, iTournaments = 0;
AutoFreeKeys free_piTournamentKey(piTournamentKey);

switch (iTournamentPage)
{
case 0:

    %><input type="hidden" name="TournamentsPage" value="0"><%

    // List all system tournaments
    iErrCode = GetOwnedTournaments (SYSTEM, &piTournamentKey, NULL, &iTournaments);
    RETURN_ON_ERROR(iErrCode);

    if (iTournaments == 0)
    {
        %><p><h3>There are no system tournaments</h3><%
    }
    else
    {
        %><p>There <% Write (iTournaments == 1 ? "is" : "are"); %> <strong><%
        Write (iTournaments); %></strong> system tournament<%

        if (iTournaments != 1)
        {
            %>s<%
        }
        %>:</h3><%

        iErrCode = RenderTournaments(piTournamentKey, iTournaments, true);
        RETURN_ON_ERROR(iErrCode);
    }

    break;

case 1:

    %><input type="hidden" name="TournamentsPage" value="1"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%

    iErrCode = RenderTournamentDetailed (m_iTournamentKey);
    RETURN_ON_ERROR(iErrCode);

    break;

default:
    Assert(false);
    break;
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>