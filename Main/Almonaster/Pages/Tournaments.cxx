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

if (InitializeEmpire(false) != OK)
{
    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

int iErrCode, iTournamentPage = 0;
unsigned int iTournamentKey = m_iReserved;

if (iTournamentKey != NO_KEY) {
    iTournamentPage = 1;
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
            sscanf (pszStart, "ViewTourneyInfo%d", &iTournamentKey) == 1) {

            m_bRedirectTest = false;
            iTournamentPage = 1;
            goto Redirection;
        }

        break;

    case 1:

        if (WasButtonPressed (BID_JOIN)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentKey")) == NULL) {
                goto Redirection;
            }

            iTournamentKey = pHttpForm->GetIntValue();

            iErrCode = InviteSelfIntoTournament (iTournamentKey, m_iEmpireKey);
            if (iErrCode == OK) {
                AddMessage ("A request to join the tournament was sent to the tournament owner");

                // Figure out where to redirect next
                unsigned int iTourneyOwner;
                iErrCode = GetTournamentOwner (iTournamentKey, &iTourneyOwner);
                if (iErrCode == OK) {
                    
                    if (iTourneyOwner == m_iEmpireKey) {
                        return Redirect (PERSONAL_TOURNAMENTS);
                    }
                }

            } else if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT) {
                AddMessage ("You are already in the tournament");
            } else if (iErrCode == ERROR_TOURNAMENT_DOES_NOT_EXIST) {
                AddMessage ("That tournament no longer exists");
            } else {
                AddMessage ("Error ");
                AppendMessage (iErrCode);
                AppendMessage (" occurred");
            }

            iTournamentPage = 1;
            m_bRedirectTest = false;
        }

        if (WasButtonPressed (BID_QUIT)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentKey")) == NULL) {
                goto Redirection;
            }

            iTournamentKey = pHttpForm->GetIntValue();

            iErrCode = DeleteEmpireFromTournament (iTournamentKey, m_iEmpireKey);
            if (iErrCode == OK) {
                AddMessage ("Your empire was deleted from the tournament");

                // Figure out where to redirect next
                unsigned int iTourneyOwner;
                iErrCode = GetTournamentOwner (iTournamentKey, &iTourneyOwner);
                if (iErrCode == OK) {
                    
                    if (iTourneyOwner == m_iEmpireKey) {
                        return Redirect (PERSONAL_TOURNAMENTS);
                    }
                }

            } else if (iErrCode == ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT) {
                AddMessage ("Your empire was no longer in the tournament");
            } else if (iErrCode == ERROR_EMPIRE_IS_IN_GAMES) {
                AddMessage ("Your empire could not be deleted from the tournament because it is still in a tournament game");
            } else {
                AddMessage ("Your empire could not be deleted from the tournament. The error was ");
                AppendMessage (iErrCode);
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
    PageId pageRedirect;
    if (RedirectOnSubmit (&pageRedirect))
    {
        return Redirect (pageRedirect);
    }
}

Check(OpenSystemPage(false));

// Individual page stuff starts here
unsigned int* piTournamentKey = NULL, iTournaments = 0;

switch (iTournamentPage) {

case 0:

    %><input type="hidden" name="TournamentsPage" value="0"><%

    // List all system tournaments
    iErrCode = GetOwnedTournaments (SYSTEM, &piTournamentKey, NULL, &iTournaments);
    if (iErrCode != OK) {
        %><p>Error <% Write (iErrCode); %> occurred<%
    }

    if (iTournaments == 0) {
        %><p><h3>There are no system tournaments</h3><%
    }

    else {

        %><p>There <% Write (iTournaments == 1 ? "is" : "are"); %> <strong><%
        Write (iTournaments); %></strong> system tournament<%

        if (iTournaments != 1) {
            %>s<%
        }
        %>:</h3><%

        RenderTournaments (piTournamentKey, iTournaments, true);
    }

    break;

case 1:

    %><input type="hidden" name="TournamentsPage" value="1"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%

    RenderTournamentDetailed (iTournamentKey);

    break;

default:

    Assert (false);
    break;
}


// Cleanup

if (piTournamentKey != NULL) {
    t_pCache->FreeKeys (piTournamentKey);
}

CloseSystemPage();

%>