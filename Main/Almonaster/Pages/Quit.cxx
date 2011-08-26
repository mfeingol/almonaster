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

if (InitializeEmpireInGame(false) != OK)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
if (InitializeGame(&pageRedirect) != OK)
{
    return Redirect(pageRedirect);
}

int iErrCode;
bool bConfirm = (m_iSystemOptions & CONFIRM_IMPORTANT_CHOICES) != 0;

if ((m_bOwnPost && !m_bRedirection) || !bConfirm) {

    PageId pageRedirect = OPTIONS;

    if (WasButtonPressed (BID_CANCEL)) {

        // Cancelled - redirect to options
        return Redirect (OPTIONS);
    }

    else if (WasButtonPressed (BID_RESIGN)) {

        if (!(m_iGameState & STARTED)) {
            AddMessage ("You cannot resign until the game starts");
        } else {

            // Resign
            iErrCode = ResignEmpireFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey);
            if (iErrCode != OK) {
                AddMessage ("You could not resign from the game; the error was ");
                AppendMessage (iErrCode);
            } else {

                pageRedirect = ACTIVE_GAME_LIST;
                AddMessage ("You resigned from ");
                AppendMessage (m_pszGameClassName);
                AppendMessage (" ");
                AppendMessage (m_iGameNumber);

                // Add to report
                char pszReport [MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];
                sprintf(pszReport, "%s resigned from %s %i", m_vEmpireName.GetCharPtr(), m_pszGameClassName, m_iGameNumber);
                global.GetReport()->WriteReport (pszReport);

                // Make sure we still exist after quitting
                bool bFlag;
                iErrCode = DoesEmpireExist (m_iEmpireKey, &bFlag, NULL);
                if (iErrCode != OK || !bFlag) {
                    pageRedirect = LOGIN;
                    AddMessage ("The empire ");
                    AppendMessage (m_vEmpireName.GetCharPtr());
                    AppendMessage ("has been deleted");
                }
            }

            // Check game for updates - redirect will handle error
            bool bFlag;
            iErrCode = CheckGameForUpdates (m_iGameClass, m_iGameNumber, true, &bFlag);
        }

        return Redirect (pageRedirect);
    }

    else if (WasButtonPressed (BID_QUIT)) {

        if (m_iGameState & STARTED) {
            AddMessage ("You cannot quit because the game has started");
        } else {

            // Quit
            iErrCode = QuitEmpireFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey);
            if (iErrCode != OK) {
                AddMessage ("You could not quit from the game; the error was ");
                AppendMessage (iErrCode);
            } else {

                pageRedirect = ACTIVE_GAME_LIST;
                AddMessage ("You quit from ");
                AppendMessage (m_pszGameClassName);
                AppendMessage (" ");
                AppendMessage (m_iGameNumber);

                // Add to report
                char pszReport [MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];
                sprintf (pszReport, "%s quit from %s %i", m_vEmpireName.GetCharPtr(), m_pszGameClassName, m_iGameNumber);
                global.GetReport()->WriteReport (pszReport);

                // Make sure we still exist after quitting
                bool bFlag;
                iErrCode = DoesEmpireExist (m_iEmpireKey, &bFlag, NULL);

                if (iErrCode != OK || !bFlag) {
                    pageRedirect = LOGIN;
                    AddMessage ("The empire ");
                    AppendMessage (m_vEmpireName.GetCharPtr());
                    AppendMessage (" has been deleted");
                }
            }
        }

        return Redirect (pageRedirect);
    }

    else if (WasButtonPressed (BID_SURRENDER)) {

        // Make sure this is allowed
        SurrenderType sType = SC30_SURRENDER;

        int iOptions;
        iErrCode = GetGameClassOptions (m_iGameClass, &iOptions);
        if (iErrCode != OK || !(iOptions & USE_SC30_SURRENDERS)) {

            // See if two empires are left - otherwise, we've been lied to
            unsigned int iNumEmpires;
            iErrCode = GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires);
            if (iErrCode != OK || iNumEmpires != 2)
            {
                return Redirect (OPTIONS);
            }

            sType = NORMAL_SURRENDER;
        }

        if (m_iGameState & STILL_OPEN) {
            AddMessage ("You cannot surrender until the game has closed");
        } else {

            // Surrender
            iErrCode = SurrenderEmpireFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, sType);
            if (iErrCode != OK) {
                AddMessage ("You could not surrender from the game; the error was ");
                AppendMessage (iErrCode);
            } else {

                pageRedirect = ACTIVE_GAME_LIST;
                AddMessage ("You surrendered from ");
                AppendMessage (m_pszGameClassName);
                AppendMessage (" ");
                AppendMessage (m_iGameNumber);

                // Make sure we still exist after surrendering
                bool bFlag;
                iErrCode = DoesEmpireExist (m_iEmpireKey, &bFlag, NULL);

                if (iErrCode != OK || !bFlag) {
                    pageRedirect = LOGIN;
                    AddMessage ("The empire ");
                    AppendMessage (m_vEmpireName.GetCharPtr());
                    AppendMessage (" has been deleted");
                }
            }
        }

        return Redirect (pageRedirect);
    }

    else {

        // Redirect to quit or info
        if (WasButtonPressed (BID_QUIT)) {
            m_iReserved = BID_QUIT;
        }
    }
}

if (m_bRedirectTest)
{
    PageId pageRedirect;
    if (RedirectOnSubmitGame (&pageRedirect))
    {
        return Redirect (pageRedirect);
    }
}

GameCheck(OpenGamePage());

// Individual page starts here

// Determine state
switch (m_iReserved) {

case BID_RESIGN:

    %><p>Are you sure that you want to resign from <strong><%
    Write (m_pszGameClassName); %> <% Write (m_iGameNumber); %></strong>?<%
    %><br>You will not be able to resume the game and your empire will fall into ruin or be nuked.<p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_RESIGN);

    break;

case BID_QUIT:

    %><p>Are you sure that you want to quit from <strong><%
    Write (m_pszGameClassName); %> <% Write (m_iGameNumber); %></strong>?<p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_QUIT);

    break;

case BID_SURRENDER:

    %><p>Are you sure that you want to surrender from <strong><%
    Write (m_pszGameClassName); %> <% Write (m_iGameNumber); %></strong>?<p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_SURRENDER);

    break;

default:

    break;
}

CloseGamePage();

%>