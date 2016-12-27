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

    // Make sure that the unprivileged don't abuse this:
    if (m_iPrivilege < ADMINISTRATOR) {
        AddMessage ("You are not authorized to view this page");
        return Redirect (LOGIN);
    }

    m_bRedirectTest = false;  // No warning
    iErrCode = Render_TournamentManager (SYSTEM);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int HtmlRenderer::Render_TournamentManager(unsigned int iOwnerKey)
{
    m_bRedirectTest = true;
    IHttpForm* pHttpForm;

    int iErrCode, iTAdminPage = 0, iIconSelect = 0;

    unsigned int i, iGameClassKey = NO_KEY, iTeamKey = NO_KEY, iClickedPlanetKey = NO_KEY, iDeleteEmpire = NO_KEY;
    
    const char* pszInviteEmpire = NULL;
    bool bAdvanced = false;

    // Handle a submission
    if (m_bOwnPost && !m_bRedirection) {

        if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentAdminPage")) == NULL) {
            goto Redirection;
        }
        int iTournamentAdminPageSubmit = pHttpForm->GetIntValue();

        if (m_iTournamentKey != NO_KEY)
        {
            unsigned int iRealOwner;

            // Simple security check
            iErrCode = GetTournamentOwner(m_iTournamentKey, &iRealOwner);
            RETURN_ON_ERROR(iErrCode);

            if (iRealOwner != iOwnerKey)
            {
                AddMessage ("Tournament ownership verification failed");
                iTAdminPage = 1;
                goto Redirection;
            }

            iErrCode = CacheTournamentTables(&m_iTournamentKey, 1);
            RETURN_ON_ERROR(iErrCode);
        }

        if ((pHttpForm = m_pHttpRequest->GetForm ("TeamKey")) != NULL) {
            iTeamKey = pHttpForm->GetIntValue();
        }

        if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) != NULL) {
            iGameClassKey = pHttpForm->GetUIntValue();
        }

        // Get game class, game number
        if (m_iGameClass != NO_KEY)
        {
            unsigned int iCheckTournament;

            iErrCode = GetGameClassTournament(m_iGameClass, &iCheckTournament);
            RETURN_ON_ERROR(iErrCode);
            
            if (iCheckTournament != m_iTournamentKey) {
                AddMessage ("Game ownership verification failed");
                iTAdminPage = 1;
                goto Redirection;
            }
        }

        //
        switch (iTournamentAdminPageSubmit) {

        case 0:
        {
            if (WasButtonPressed (BID_CREATENEWTOURNAMENT)) {
                iTAdminPage = 1;
                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_DELETETOURNAMENT)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("DelTournament")) == NULL) {
                    goto Redirection;
                }

                iErrCode = DeleteTournament(iOwnerKey, pHttpForm->GetIntValue(), false);
                switch (iErrCode)
                {
                case OK:
                    AddMessage ("The tournament was deleted");
                    break;

                case ERROR_TOURNAMENT_HAS_GAMECLASSES:
                    AddMessage ("You cannot delete a tournament while it has gameclasses");
                    break;

                case ERROR_TOURNAMENT_HAS_GAMES:
                    AddMessage ("You cannot delete a tournament while it has active games");
                    break;

                case ERROR_ACCESS_DENIED:
                    AddMessage ("You do not have permission to delete that tournament");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }

                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_ADMINISTERTOURNAMENT))
            {
                iTAdminPage = 2;
                goto Redirection;
            }

            const char* pszStart;
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewTourneyInfo")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "ViewTourneyInfo%d", &m_iReserved) == 1) {
                return Redirect (TOURNAMENTS);
            }

            break;
        }

        case 1:

            if (WasButtonPressed (BID_CREATENEWTOURNAMENT)) {

                bool bCreated;
                iErrCode = ProcessCreateTournament(iOwnerKey, &bCreated);
                RETURN_ON_ERROR(iErrCode);

                if (bCreated) {
                    iTAdminPage = 0;
                } else {
                    iTAdminPage = 1;    // Repaint form
                }

                m_bRedirectTest = false;
                goto Redirection;
            }
            break;

        case 2:
            {

            Variant vOldString;
            bool bFlag;
            unsigned int iRealTourney;

            if (m_iTournamentKey == NO_KEY) {
                iTAdminPage = 1;
                goto Redirection;
            }

            // Description
            if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentDescription")) == NULL) {
                goto Redirection;
            }

            if (GetTournamentDescription (m_iTournamentKey, &vOldString) == OK &&
                String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

                iErrCode = SetTournamentDescription (m_iTournamentKey, pHttpForm->GetValue());
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The tournament description was updated");
            }

            // Url
            if ((pHttpForm = m_pHttpRequest->GetForm ("WebPageURL")) == NULL) {
                goto Redirection;
            }

            if (GetTournamentUrl(m_iTournamentKey, &vOldString) == OK &&
                String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

                iErrCode = SetTournamentUrl(m_iTournamentKey, pHttpForm->GetValue());
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The tournament webpage was updated");
            }

            // News
            if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentNews")) == NULL) {
                goto Redirection;
            }

            if (GetTournamentNews(m_iTournamentKey, &vOldString) == OK &&
                String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

                char* pszBuffer = NULL;

                if (String::StrLen (pHttpForm->GetValue()) > MAX_TOURNAMENT_NEWS_LENGTH) {

                    pszBuffer = (char*) StackAlloc (MAX_TOURNAMENT_NEWS_LENGTH + 1);
                    memcpy (pszBuffer, pHttpForm->GetValue(), MAX_TOURNAMENT_NEWS_LENGTH);
                    pszBuffer [MAX_TOURNAMENT_NEWS_LENGTH] = '\0';

                } else {

                    pszBuffer = (char*) pHttpForm->GetValue();
                }

                iErrCode = SetTournamentNews (m_iTournamentKey, pszBuffer);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The tournament news was updated");

                if (pszBuffer != pHttpForm->GetValue()) {
                    AppendMessage (". Your news submission was truncated because it was too long");
                }
            }

            // Handle icon selection request
            if (WasButtonPressed (BID_CHOOSE)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("IconSelect")) == NULL) {
                    goto Redirection;
                } else {
                    iTAdminPage = 10;
                    iIconSelect = pHttpForm->GetIntValue();
                }
                break;
            }

            if (WasButtonPressed (BID_CREATENEWGAMECLASS)) {
                iTAdminPage = 3;
                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_INVITEEMPIRE)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("InviteEmpireName")) == NULL) {
                    goto Redirection;
                }
                pszInviteEmpire = pHttpForm->GetValue();
                if (String::IsBlank (pszInviteEmpire)) {
                    AddMessage ("You cannot invite a blank empire");
                    iTAdminPage = 2;
                } else {
                    iTAdminPage = 4;
                }

                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_DELETEEMPIRE)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("DelEmp")) == NULL) {
                    goto Redirection;
                }
                iDeleteEmpire = pHttpForm->GetUIntValue();

                iTAdminPage = 5;
                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_CREATETEAM)) {
                iTAdminPage = 8;
                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_DELETETEAM)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("DelTeam")) == NULL) {
                    goto Redirection;
                }

                iErrCode = DeleteTournamentTeam(m_iTournamentKey, pHttpForm->GetIntValue());
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The team was deleted");

                iTAdminPage = 2;
                m_bRedirectTest = false;
                goto Redirection;
            }

            // GameClass stuff
            if (WasButtonPressed (BID_DELETEGAMECLASS)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteGC")) == NULL) {
                    goto Redirection;
                }

                // Verify ownership
                iErrCode = GetGameClassTournament (pHttpForm->GetIntValue(), &iRealTourney);
                RETURN_ON_ERROR(iErrCode);
                if (iRealTourney != m_iTournamentKey) {
                    AddMessage ("Gameclass ownership verification failed");
                    iTAdminPage = 1;
                    goto Redirection;
                }

                iErrCode = DeleteGameClass (pHttpForm->GetIntValue(), &bFlag);
                if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
                {
                    AddMessage ("The GameClass no longer exists");
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                    if (bFlag) {
                        AddMessage ("The GameClass was deleted");
                    } else {
                        AddMessage ("The GameClass has been marked for deletion");
                    }
                }

                iTAdminPage = 2;
                goto Redirection;
            }

            if (WasButtonPressed (BID_UNDELETEGAMECLASS)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("UndeleteGC")) == NULL) {
                    goto Redirection;
                }

                // Verify ownership
                iErrCode = GetGameClassTournament (pHttpForm->GetIntValue(), &iRealTourney);
                RETURN_ON_ERROR(iErrCode);
                if (iRealTourney != m_iTournamentKey) {
                    AddMessage ("Gameclass ownership verification failed");
                    iTAdminPage = 1;
                    goto Redirection;
                }

                iErrCode = UndeleteGameClass (pHttpForm->GetIntValue());
                switch (iErrCode)
                {
                case OK:
                    AddMessage ("The GameClass was undeleted");
                    break;

                case ERROR_GAMECLASS_DOES_NOT_EXIST:
                    AddMessage ("The GameClass no longer exists");
                    break;

                case ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION:
                    AddMessage ("The GameClass was not marked for deletion");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }

                iTAdminPage = 2;
                goto Redirection;
            }

            // Handle game class halt
            if (WasButtonPressed (BID_HALTGAMECLASS)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("HaltGC")) == NULL) {
                    goto Redirection;
                }
            
                // Verify ownership
                iErrCode = GetGameClassTournament (pHttpForm->GetIntValue(), &iRealTourney);
                RETURN_ON_ERROR(iErrCode);
                if (iRealTourney != m_iTournamentKey)
                {
                    AddMessage ("Gameclass ownership verification failed");
                    iTAdminPage = 1;
                    goto Redirection;
                }

                iErrCode = HaltGameClass (pHttpForm->GetIntValue());
                if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
                {
                    AddMessage ("The GameClass no longer exists");
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The GameClass was halted");
                }

                iTAdminPage = 2;
                goto Redirection;
            }

            // Handle game class unhalting
            if (WasButtonPressed (BID_UNHALTGAMECLASS)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("UnhaltGC")) == NULL) {
                    goto Redirection;
                }

                // Verify ownership
                iErrCode = GetGameClassTournament (pHttpForm->GetIntValue(), &iRealTourney);
                RETURN_ON_ERROR(iErrCode);
                if (iRealTourney != m_iTournamentKey) {
                    AddMessage ("Gameclass ownership verification failed");
                    iTAdminPage = 1;
                    goto Redirection;
                }

                iErrCode = UnhaltGameClass (pHttpForm->GetIntValue());
                switch (iErrCode)
                {
                case OK:
                    AddMessage ("The GameClass is no longer halted");
                    break;

                case ERROR_GAMECLASS_DOES_NOT_EXIST:
                    AddMessage ("The GameClass no longer exists");
                    break;

                case ERROR_GAMECLASS_NOT_HALTED:
                    AddMessage ("The GameClass was not halted");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }

                iTAdminPage = 2;
                goto Redirection;
            }

            if (WasButtonPressed (BID_START)) {
                iTAdminPage = 6;
                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_ADMINISTERTEAM)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("AdminTeam")) == NULL) {
                    goto Redirection;
                }

                iTeamKey = pHttpForm->GetIntValue();

                iTAdminPage = 9;
                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_VIEWGAMEINFORMATION)) {

                iTAdminPage = 12;
                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_UPDATE)) {
                iTAdminPage = 2;
                goto Redirection;
            }

            }
            break;

        case 3:

            if (m_iTournamentKey == NO_KEY) {
                iTAdminPage = 1;
                goto Redirection;
            }

            // Handle new gameclass creation
            if (WasButtonPressed (BID_CREATENEWGAMECLASS)) {

                m_bRedirectTest = false;

                bool bProcessed;
                iErrCode = ProcessCreateGameClassForms(iOwnerKey, m_iTournamentKey, &bProcessed);
                RETURN_ON_ERROR(iErrCode);
                if (!bProcessed)
                {
                    iTAdminPage = 3;
                }
                else
                {
                    // Back to administer page
                    iTAdminPage = 2;
                }
                goto Redirection;
            }

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 2;
                goto Redirection;
            }

            break;

        case 4:

            if (m_iTournamentKey == NO_KEY) {
                iTAdminPage = 1;
                goto Redirection;
            }

            if (WasButtonPressed (BID_INVITEEMPIRE)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("InviteEmpireKey")) == NULL) {
                    goto Redirection;
                }

                iErrCode = InviteEmpireIntoTournament (m_iTournamentKey, iOwnerKey, m_iEmpireKey, pHttpForm->GetIntValue());
                if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT)
                {
                    AddMessage ("The empire is already in the tournament");
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The invitation was sent to the empire");
                }
            }

            // Lookup
            if (WasButtonPressed (BID_LOOKUP)) {
                m_bRedirectTest = false;
                iTAdminPage = 4;
                break;
            }

            // Back to administer page
            iTAdminPage = 2;

            break;

        case 5:

            if (m_iTournamentKey == NO_KEY) {
                iTAdminPage = 1;
                goto Redirection;
            }

            if (WasButtonPressed (BID_DELETEEMPIRE)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteEmpireKey")) == NULL) {
                    goto Redirection;
                }

                iErrCode = DeleteEmpireFromTournament (m_iTournamentKey, pHttpForm->GetIntValue());
                switch (iErrCode)
                {
                case OK:
                    AddMessage ("The empire was deleted from the tournament");
                    break;
                case ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT:
                    AddMessage ("The empire is not longer in the tournament");
                    break;
                case ERROR_EMPIRE_IS_IN_GAMES:
                    AddMessage ("The empire could not be deleted from the tournament because it is still in a tournament game");
                    break;
                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
            }

            // Lookup
            if (WasButtonPressed (BID_LOOKUP)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteEmpireKey")) == NULL) {
                    goto Redirection;
                }

                iDeleteEmpire = pHttpForm->GetUIntValue();

                m_bRedirectTest = false;
                iTAdminPage = 5;
                break;
            }

            // Back to administer page
            else iTAdminPage = 2;

            break;

        case 6:
            {

            const char* pszStart;

            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

                m_bRedirectTest = false;

                // Check for advanced option
                char pszAdvanced [128];
                sprintf(pszAdvanced, "Advanced%i", iGameClassKey);

                if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                    bAdvanced = true;
                }

                iTAdminPage = 7;
                goto Redirection;
            }

            }
            break;

        case 7:

            if (m_iTournamentKey == NO_KEY) {
                iTAdminPage = 1;
                goto Redirection;
            }

            if (WasButtonPressed (BID_START)) {

                int iTeamOptions = 0;

                if ((pHttpForm = m_pHttpRequest->GetForm ("TeamOptions")) != NULL) {
                    iTeamOptions = pHttpForm->GetIntValue();
                }

                // Check for advanced option
                char pszAdvanced [128];
                sprintf(pszAdvanced, "Advanced%i", iGameClassKey);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                    bAdvanced = true;
                }

                iErrCode = CacheTournamentEmpiresForGame(m_iTournamentKey);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = StartTournamentGame(m_iTournamentKey, iTeamOptions, bAdvanced);
                switch (iErrCode)
                {
                case OK:
                    iTAdminPage = 2;
                    break;
                case ERROR_TOO_MANY_EMPIRES:
                case ERROR_NOT_ENOUGH_EMPIRES:
                case ERROR_ALLIANCE_LIMIT_EXCEEDED:
                case ERROR_COULD_NOT_START_GAME:
                    iTAdminPage = 7;
                    break;
                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
                goto Redirection;
            }

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 2;
                goto Redirection;
            }

            break;

        case 8:

            if (m_iTournamentKey == NO_KEY)
            {
                iTAdminPage = 1;
                goto Redirection;
            }

            // Handle new team creation
            if (WasButtonPressed (BID_CREATETEAM))
            {
                m_bRedirectTest = false;
                bool bCreated;
                iErrCode = ProcessCreateTournamentTeam(m_iTournamentKey, &bCreated);
                RETURN_ON_ERROR(iErrCode);

                if (!bCreated)
                {
                    iTAdminPage = 8;
                }
                else
                {
                    // Back to administer page
                    iTAdminPage = 2;
                }
                goto Redirection;
            }

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 2;
                goto Redirection;
            }

            break;

        case 9:
            {

            Variant vOldString;

            if (m_iTournamentKey == NO_KEY) {
                iTAdminPage = 1;
                goto Redirection;
            }

            if (iTeamKey == NO_KEY) {
                iTAdminPage = 2;
                goto Redirection;
            }

            // Description
            if ((pHttpForm = m_pHttpRequest->GetForm ("TeamDescription")) == NULL) {
                goto Redirection;
            }

            iErrCode = GetTournamentTeamDescription(m_iTournamentKey, iTeamKey, &vOldString);
            RETURN_ON_ERROR(iErrCode);
            
            if (String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0)
            {
                iErrCode = SetTournamentTeamDescription (m_iTournamentKey, iTeamKey, pHttpForm->GetValue());
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The team description was updated");
            }

            // Url
            if ((pHttpForm = m_pHttpRequest->GetForm ("TeamWebPageURL")) == NULL) {
                goto Redirection;
            }

            iErrCode = GetTournamentTeamUrl (m_iTournamentKey, iTeamKey, &vOldString);
            RETURN_ON_ERROR(iErrCode);
            
            if (String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

                iErrCode = SetTournamentTeamUrl (m_iTournamentKey, iTeamKey, pHttpForm->GetValue());
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The team webpage was updated");
            }

            // Handle icon selection request
            if (WasButtonPressed (BID_CHOOSE)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("IconSelect")) == NULL) {
                    goto Redirection;
                } else {
                    iTAdminPage = 11;
                    iIconSelect = pHttpForm->GetIntValue();
                }
                break;
            }

            if (WasButtonPressed (BID_ADDEMPIRE)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("JoinTeam")) == NULL) {
                    goto Redirection;
                }

                iErrCode = SetEmpireTournamentTeam (m_iTournamentKey, pHttpForm->GetIntValue(), iTeamKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The empire was added to the team");

                m_bRedirectTest = false;
                iTAdminPage = 9;
                goto Redirection;
            }

            if (WasButtonPressed (BID_DELETEEMPIRE)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("DelFromTeam")) == NULL) {
                    goto Redirection;
                }

                iErrCode = SetEmpireTournamentTeam (m_iTournamentKey, pHttpForm->GetIntValue(), NO_KEY);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The empire was deleted from the team");

                m_bRedirectTest = false;
                iTAdminPage = 9;
                goto Redirection;
            }

            if (WasButtonPressed (BID_UPDATE)) {
                m_bRedirectTest = false;
                iTAdminPage = 9;
                goto Redirection;
            }

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 2;
                goto Redirection;
            }

            }
            break;

        case 10:
            {

            // An extra I/O, but it almost never happens
            iErrCode = CacheSystemAlienIcons();
            RETURN_ON_ERROR(iErrCode);

            unsigned int iOldIcon, iIcon;
            int iOldAddress;

            iErrCode = GetTournamentIcon (m_iTournamentKey, &iOldIcon, &iOldAddress);
            RETURN_ON_ERROR(iErrCode);

            bool bHandled;
            iErrCode = HandleIconSelection (&iIcon, BASE_UPLOADED_TOURNAMENT_ICON_DIR, m_iTournamentKey, NO_KEY, &bHandled);
            RETURN_ON_ERROR(iErrCode);

            if (bHandled)
            {
                if (iIcon == UPLOADED_ICON)
                {
                    if (iOldIcon != UPLOADED_ICON)
                    {
                        iErrCode = SetTournamentIcon (m_iTournamentKey, UPLOADED_ICON);
                        RETURN_ON_ERROR(iErrCode);
                    }
                }
                else if (iOldIcon != iIcon)
                {
                    iErrCode = SetTournamentIcon (m_iTournamentKey, iIcon);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The icon was updated");
                }
                else
                {
                    AddMessage ("That was the same icon");
                }

                m_bRedirectTest = false;
            }

            iTAdminPage = 2;
            goto Redirection;

            }
            break;

    case 11:

            {

            unsigned int iOldIcon, iIcon;
            int iOldAddress;

            // An extra I/O, but it almost never happens
            iErrCode = CacheSystemAlienIcons();
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetTournamentTeamIcon(m_iTournamentKey, iTeamKey, &iOldIcon, &iOldAddress);
            RETURN_ON_ERROR(iErrCode);

            bool bHandled;
            iErrCode = HandleIconSelection (&iIcon, BASE_UPLOADED_TOURNAMENT_TEAM_ICON_DIR, m_iTournamentKey, iTeamKey, &bHandled);
            RETURN_ON_ERROR(iErrCode);

            if (bHandled)
            {
                if (iIcon == UPLOADED_ICON) {

                    if (iOldIcon != UPLOADED_ICON)
                    {
                        iErrCode = SetTournamentTeamIcon (m_iTournamentKey, iTeamKey, UPLOADED_ICON);
                        RETURN_ON_ERROR(iErrCode);
                    }
                }

                else if (iOldIcon != iIcon) {

                    iErrCode = SetTournamentTeamIcon (m_iTournamentKey, iTeamKey, iIcon);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The icon was updated");
                }

                else {
                    AddMessage ("That was the same icon");
                }

                m_bRedirectTest = false;
            }

            iTAdminPage = 9;
            goto Redirection;

            }
            break;

    case 12:

            {

            const char* pszGame;

            // Administer game
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("AdministerGame")) != NULL && 
                (pszGame = pHttpForm->GetName()) != NULL &&
                sscanf (pszGame, "AdministerGame%d.%d", &m_iGameClass, &m_iGameNumber) == 2) {

                m_bRedirectTest = false;
                iTAdminPage = 13;
                goto Redirection;
            }

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 2;
                goto Redirection;
            }

            }
            break;

    case 13:
            {

            bool bExist;
            const char* pszMessage;

            if (m_iGameClass == NO_KEY) {
                iTAdminPage = 2;
                goto Redirection;
            }

            // View map
            if (WasButtonPressed (BID_VIEWMAP)) {
                m_bRedirectTest = false;
                iTAdminPage = 14;
                goto Redirection;
            }

            // Force update
            if (WasButtonPressed (BID_FORCEUPDATE)) {

                iErrCode = ForceUpdate(m_iGameClass, m_iGameNumber);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The game was forcibly updated");
                
                iErrCode = DoesGameExist(m_iGameClass, m_iGameNumber, &bExist);
                RETURN_ON_ERROR(iErrCode);
                
                if (bExist) {
                    iTAdminPage = 13;
                } else {
                    iTAdminPage = 12;
                }

                m_bRedirectTest = false;
                goto Redirection;
            }

            // Delete empire from game
            if (WasButtonPressed (BID_DELETEEMPIRE)) {

                pHttpForm = m_pHttpRequest->GetForm ("DeleteEmpireKey");
                if (pHttpForm != NULL)
                {
                    int iTargetEmpireKey = pHttpForm->GetIntValue();

                    iErrCode = CacheAllGameTables(m_iGameClass, m_iGameNumber);
                    RETURN_ON_ERROR(iErrCode);

                    if (m_iGameState & STARTED)
                    {
                        iErrCode = RemoveEmpireFromGame (m_iGameClass, m_iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                        switch (iErrCode)
                        {
                        case OK:
                            AddMessage("The empire was deleted from the game");
                            break;

                        case ERROR_EMPIRE_IS_NOT_IN_GAME:
                            AddMessage("The empire is no longer in this game");
                            iErrCode = OK;
                            break;

                        default:
                            RETURN_ON_ERROR(iErrCode);
                            break;
                        }
                    }
                    else
                    {
                        iErrCode = QuitEmpireFromGame(m_iGameClass, m_iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                        switch (iErrCode)
                        {
                        case OK:
                            AddMessage("The empire was deleted from the game");
                            break;
                        case ERROR_EMPIRE_IS_NOT_IN_GAME:
                            AddMessage("The empire is no longer in this game");
                            iErrCode = OK;
                            break;
            
                        case ERROR_GAME_HAS_STARTED:
                            iErrCode = RemoveEmpireFromGame(m_iGameClass, m_iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                            switch (iErrCode)
                            {
                            case OK:
                                AddMessage("The empire was deleted from the game");
                                break;

                            case ERROR_EMPIRE_IS_NOT_IN_GAME:
                                AddMessage("The empire is no longer in this game");
                                break;

                            default:
                               RETURN_ON_ERROR(iErrCode);
                               break;
                            }
                            break;

                        default:
                            RETURN_ON_ERROR(iErrCode);
                            break;
                        }
                    }
                }

                iErrCode = DoesGameExist(m_iGameClass, m_iGameNumber, &bExist);
                RETURN_ON_ERROR(iErrCode);
                if (bExist)
                {
                    iTAdminPage = 13;
                }
                else
                {
                    iTAdminPage = 12;
                }

                m_bRedirectTest = false;
                goto Redirection;
            }

            // Restore resigned empire to game
            if (WasButtonPressed (BID_RESTOREEMPIRE)) {

                pHttpForm = m_pHttpRequest->GetForm ("RestoreEmpireKey");
                if (pHttpForm != NULL) {

                    int iTargetEmpireKey = pHttpForm->GetIntValue();

                    if (!(m_iGameState & STARTED)) {

                        iErrCode = UnresignEmpire (m_iGameClass, m_iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                        RETURN_ON_ERROR(iErrCode);
                        AddMessage ("The empire was restored");
                    }
                }

                iErrCode = DoesGameExist(m_iGameClass, m_iGameNumber, &bExist);
                RETURN_ON_ERROR(iErrCode);
                
                if (bExist) {
                    iTAdminPage = 13;
                } else {
                    iTAdminPage = 12;
                }

                m_bRedirectTest = false;
                goto Redirection;
            }

            // Check for view empire info
            if (WasButtonPressed (BID_VIEWEMPIREINFORMATION)) {

                iTAdminPage = 15;
                m_bRedirectTest = false;
                goto Redirection;
            }

            // Pause game
            if (WasButtonPressed (BID_PAUSEGAME)) {

                // Flush remaining updates
                bool bUpdated, bExists = true;
                iErrCode = CheckGameForUpdates(m_iGameClass, m_iGameNumber, &bUpdated);
                RETURN_ON_ERROR(iErrCode);

                if (bUpdated)
                {
                    iErrCode = DoesGameExist(m_iGameClass, m_iGameNumber, &bExists);
                    RETURN_ON_ERROR(iErrCode);
                }

                if (bExists)
                {
                    // Best effort pause the game
                    iErrCode = PauseGame(m_iGameClass, m_iGameNumber, true, true);
                    RETURN_ON_ERROR(iErrCode);

                    AddMessage ("The game is now paused");
                    iTAdminPage = 13;
                }
                else
                {
                    AddMessage ("The game no longer exists");
                    iTAdminPage = 12;
                }

                m_bRedirectTest = false;
                goto Redirection;
            }

            // Unpause game
            if (WasButtonPressed (BID_UNPAUSEGAME)) {

                iErrCode = UnpauseGame(m_iGameClass, m_iGameNumber, true, true);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The game is no longer paused");
                iTAdminPage = 13;

                m_bRedirectTest = false;
                goto Redirection;
            }

            // Broadcast message
            if (WasButtonPressed (BID_SENDMESSAGE)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL) {
                    goto Redirection;
                }
                pszMessage = pHttpForm->GetValue();

                iErrCode = CacheGameTablesForBroadcast(m_iGameClass, m_iGameNumber);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = BroadcastGameMessage(m_iGameClass, m_iGameNumber, pszMessage, m_iEmpireKey, MESSAGE_BROADCAST | MESSAGE_TOURNAMENT_ADMINISTRATOR);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Your message was broadcast to all empires in the game");

                iTAdminPage = 13;
                goto Redirection;
            }

            // Kill game
            if (WasButtonPressed (BID_KILLGAME)) {
                m_bRedirectTest = false;
                iTAdminPage = 17;
                goto Redirection;
            }

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 12;
                goto Redirection;
            }

            }
            break;

    case 14:
            {

            if (m_iGameClass == NO_KEY) {
                goto Redirection;
            }

            const char* pszStart;
            int iClickedProxyPlanetKey;

            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

                // We clicked on a planet
                iTAdminPage = 16;
                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 13;
                goto Redirection;
            }

            }
            break;

    case 15:

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 13;
                goto Redirection;
            }

            break;

    case 16:

            // View map
            if (WasButtonPressed (BID_VIEWMAP)) {

                m_bRedirectTest = false;
                iTAdminPage = 14;
                goto Redirection;
            }

    case 17:

            // Kill game
            if (WasButtonPressed (BID_KILLGAME))
            {
                if ((pHttpForm = m_pHttpRequest->GetForm ("DoomMessage")) == NULL) {
                    break;
                }
                const char* pszMessage = pHttpForm->GetValue();

                iErrCode = DeleteGame(m_iGameClass, m_iGameNumber, m_iEmpireKey, pszMessage, REASON_NONE);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The game was deleted");

                m_bRedirectTest = false;
                iTAdminPage = 12;
                goto Redirection;
            }

            if (WasButtonPressed (BID_CANCEL)) {
                m_bRedirectTest = false;
                iTAdminPage = 13;
                goto Redirection;
            }

            break;

        default:
            Assert(false);
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

    iErrCode = OpenSystemPage((iTAdminPage == 10 || iTAdminPage == 11) && iIconSelect == 1);
    RETURN_ON_ERROR(iErrCode);

    // Individual page stuff starts here
    switch (iTAdminPage)
    {
    Start:
    case 0:

        %><input type="hidden" name="TournamentAdminPage" value="0"><%

        iErrCode = WriteTournamentAdministrator (iOwnerKey);
        RETURN_ON_ERROR(iErrCode);
        if (iOwnerKey != SYSTEM)
        {
            Assert(iOwnerKey == m_iEmpireKey);
            iErrCode = WritePersonalTournaments();
            RETURN_ON_ERROR(iErrCode);
        }
        break;

    case 1:

        %><input type="hidden" name="TournamentAdminPage" value="1"><%

        %><p><h3>Create a new <%
        if (iOwnerKey == SYSTEM) {
            %>system<%
        } else {
            %>personal<%
        }
        %> tournament</h3><%

        WriteCreateTournament (iOwnerKey);

        %><p><%
        WriteButton (BID_CANCEL);
        WriteButton (BID_CREATENEWTOURNAMENT);

        break;

    Admin:
    case 2:

        if (m_iTournamentKey == NO_KEY) {
            goto Start;
        }

        %><input type="hidden" name="TournamentAdminPage" value="2"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%

        iErrCode = WriteAdministerTournament (m_iTournamentKey);
        RETURN_ON_ERROR(iErrCode);
        break;

    case 3:
        {

        if (m_iTournamentKey == NO_KEY)
        {
            goto Start;
        }

        Variant vName;
        iErrCode = GetTournamentName(m_iTournamentKey, &vName);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="3"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%

        %><h3>Create a new GameClass for the <% Write (vName.GetCharPtr()); %> tournament</h3><p><%

        iErrCode = WriteCreateGameClassString(iOwnerKey, m_iTournamentKey, false);
        RETURN_ON_ERROR(iErrCode);

        %><p><%
        WriteButton (BID_CANCEL);
        WriteButton (BID_CREATENEWGAMECLASS);

        }
        break;

    case 4:
        {

        unsigned int iInviteKey = NO_KEY;
        Variant vInviteName;

        if (pszInviteEmpire == NULL)
        {
            pHttpForm = m_pHttpRequest->GetForm ("InviteEmpireKey");
            if (pHttpForm != NULL && pHttpForm->GetValue() != NULL)
            {
                iInviteKey = pHttpForm->GetIntValue();
                iErrCode = GetEmpireName (iInviteKey, &vInviteName);
                RETURN_ON_ERROR(iErrCode);
            }
        }
        else
        {
            iErrCode = LookupEmpireByName(pszInviteEmpire, &iInviteKey, &vInviteName, NULL);
            RETURN_ON_ERROR(iErrCode);
        }

        if (iInviteKey == NO_KEY) {
            %><p><strong>That empire does not exist</strong><%
            goto Admin;
        }

        %><input type="hidden" name="TournamentAdminPage" value="4"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="InviteEmpireKey" value="<% Write (iInviteKey); %>"><%

        %><p>Do you wish to invite <strong><% Write (vInviteName.GetCharPtr()); 
        %></strong> to join your tournament?<%

        %><p><%
        WriteButton (BID_CANCEL);
        WriteButton (BID_INVITEEMPIRE);

        %><p><%
        
        int iSeparatorAddress;
        iErrCode = GetThemeAddress(m_iSeparatorKey, &iSeparatorAddress);
        RETURN_ON_ERROR(iErrCode);

        WriteSeparatorString(m_iSeparatorKey, iSeparatorAddress);

        iErrCode = WriteProfile(m_iEmpireKey, iInviteKey, false, false, false);
        RETURN_ON_ERROR(iErrCode);

        }
        break;

    case 5:
        {

        Variant vDeleteName;

        iErrCode = GetEmpireName (iDeleteEmpire, &vDeleteName);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="5"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="DeleteEmpireKey" value="<% Write (iDeleteEmpire); %>"><%

        %><p>Do you wish to delete <strong><% Write (vDeleteName.GetCharPtr()); 
        %></strong> from your tournament?<%

        %><p><%
        WriteButton (BID_CANCEL);
        WriteButton (BID_DELETEEMPIRE);

        %><p><%

        int iSeparatorAddress;
        iErrCode = GetThemeAddress(m_iSeparatorKey, &iSeparatorAddress);
        RETURN_ON_ERROR(iErrCode);

        WriteSeparatorString(m_iSeparatorKey, iSeparatorAddress);

        iErrCode = WriteProfile(m_iEmpireKey, iDeleteEmpire, false, false, false);
        RETURN_ON_ERROR(iErrCode);

        }
        break;

    case 6:
        {

        unsigned int* piGameClassKey = NULL, iNumGameClasses;
        AutoFreeKeys free_piGameClassKey(piGameClassKey);

        iErrCode = GetTournamentGameClasses(m_iTournamentKey, &piGameClassKey, NULL, &iNumGameClasses);
        RETURN_ON_ERROR(iErrCode);

        if (iNumGameClasses == 0)
        {
            %><p><strong>The tournament has no gameclasses</strong><%
            goto Start;
        }

        Variant vName;
        iErrCode = GetTournamentName (m_iTournamentKey, &vName);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="6"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%

        %><h3>Start a new game for the <% Write (vName.GetCharPtr()); %> tournament:</h3><%

        WriteSystemGameListHeader (m_vTableColor.GetCharPtr());

        for (i = 0; i < iNumGameClasses; i ++)
        {
            Variant* pvGameClassInfo = NULL;
            AutoFreeData free_pvGameClassInfo(pvGameClassInfo);

            iErrCode = GetGameClassData (piGameClassKey[i], &pvGameClassInfo);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = WriteSystemGameListData (piGameClassKey[i], pvGameClassInfo);
            RETURN_ON_ERROR(iErrCode);
        }
        %></table><%

        }
        break;

    case 7:
        {

        int iMaxNumEmpires, iGameClassOptions, iDipLevel;
        unsigned int iNumEmpires;

        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

        iErrCode = GetGameClassName (iGameClassKey, pszGameClassName);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetGameClassOptions (iGameClassKey, &iGameClassOptions);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetGameClassDiplomacyLevel (iGameClassKey, &iDipLevel);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetMaxNumEmpires (iGameClassKey, &iMaxNumEmpires);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetNextGameNumber (iGameClassKey, &m_iGameNumber);
        RETURN_ON_ERROR(iErrCode);
        
        Variant vName;
        iErrCode = GetTournamentName (m_iTournamentKey, &vName);
        RETURN_ON_ERROR(iErrCode);
        
        Variant* pvTeamEmpireKey = NULL, * pvEmpireKey = NULL, * pvEmpireName = NULL;
        AutoFreeData free_pvTeamEmpireKey(pvTeamEmpireKey);
        AutoFreeData free_pvEmpireKey(pvEmpireKey);
        Algorithm::AutoDelete<Variant> free_pvEmpireName(pvEmpireName, true);

        iErrCode = CacheTournamentEmpireTables(m_iTournamentKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetAvailableTournamentEmpires(m_iTournamentKey, &pvEmpireKey, &pvTeamEmpireKey, &pvEmpireName, &iNumEmpires);
        RETURN_ON_ERROR(iErrCode);
        
        Variant* pvTeamName = NULL;
        AutoFreeData free_pvTeamName(pvTeamName);

        unsigned int iNumTeams, * piTeamKey = NULL;
        AutoFreeKeys free_piTeamKey(piTeamKey);

        iErrCode = GetTournamentTeams(m_iTournamentKey, &piTeamKey, &pvTeamName, &iNumTeams);
        RETURN_ON_ERROR(iErrCode);
        
        if ((unsigned int) iMaxNumEmpires > iNumEmpires)
        {
            %><p><strong>The tournament does not have enough available empires to start the game</strong><%
            goto Admin;
        }

        %><input type="hidden" name="TournamentAdminPage" value="7"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%

        %><h3>Start a new game for the <% Write (vName.GetCharPtr()); %> tournament: <%
        Write (pszGameClassName); %> <% Write (m_iGameNumber); %></h3><%
        %><h3>Select <% Write (iMaxNumEmpires); %> empires for the game:</h3><%

        // Empire choices
        bool bEmpDisplay = false, bTeamDisplay = false;

        %><p><table><%

        %><tr><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Teams</th><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>">Empires</th><%
        %></tr><%

        %><tr><%
        %><td><%
        %><table width="100%"><%

        unsigned int j, iMaxNumTeamEmps = 0;

        if (iNumTeams > 0) {

            for (i = 0; i < iNumTeams; i ++) {

                unsigned int iNumTeamEmps = 0;
                for (j = 0; j < iNumEmpires; j ++) {

                    if ((unsigned int)pvTeamEmpireKey[j].GetInteger() == piTeamKey[i]) {
                        iNumTeamEmps ++;
                        if (iNumTeamEmps > iMaxNumTeamEmps) {
                            iMaxNumTeamEmps = iNumTeamEmps;
                        }
                    }
                }

                if (iNumTeamEmps > 0) {

                    bTeamDisplay = true;

                    %><tr><%
                    %><td><%
                    %><input type="checkbox" name="TeamSel<% Write (piTeamKey[i]); %>"><%
                    %> <% Write (pvTeamName[i].GetCharPtr()); 
                    %><br>(<strong><% Write (iNumTeamEmps); %></strong> available empire<%
                    if (iNumTeamEmps != 1) {
                        %>s<%
                    }
                    %>)</td><%
                    %></tr><%
                }
            }
        }

        if (!bTeamDisplay) {
            %><tr><%
            %><td align="center"><%
            %><strong>-</strong><%
            %></td><%
            %></tr><%
        }

        %></table><%
        %></td><%

        %><td><%
        %><table><%

        if (iNumEmpires > 0) {

            bEmpDisplay = true;

            for (i = 0; i < iNumEmpires; i ++) {

                %><tr><%
                %><td><%
                %><input type="checkbox" name="EmpireSel<% Write (pvEmpireKey[i].GetInteger()); %>"><%
                %> <% Write (pvEmpireName[i].GetCharPtr());

                if (pvTeamEmpireKey[i].GetInteger() == NO_KEY) {
                    %> (<em>Unaffiliated</em>)<%
                } else {

                    for (j = 0; j < iNumTeams; j ++) {
                        if ((unsigned int)pvTeamEmpireKey[i].GetInteger() == piTeamKey[j]) {
                            %> (<strong><% Write (pvTeamName[j].GetCharPtr()); %></strong>)<%
                            break;
                        }
                    }

                    Assert(j < iNumTeams);
                }

                %></td><%
                %></tr><%
            }
        }

        if (!bEmpDisplay) {
            %><tr><%
            %><td align="center"><%
            %><strong>-</strong><%
            %></td><%
            %></tr><%
        }

        %></table><%
        %></td><%
        %></tr><%
        %></table><%

        // Team options
        if (bTeamDisplay && ((iDipLevel & ALLIANCE) || !(iGameClassOptions & EXPOSED_DIPLOMACY)) && iMaxNumTeamEmps > 1) {

            %><h3>Team options:</h3><p><%

            %><select name="TeamOptions"><%
            %><option value="0">No special team options</option><%

            if (!(iGameClassOptions & EXPOSED_DIPLOMACY)) {
                %><option <%

                if (!(iDipLevel & ALLIANCE)) {
                    %>selected <%
                }

                %>value="<% Write (TEAM_PREARRANGED_DIPLOMACY); %>"><%
                %>Empires begin the game having already met their teammates<%
                %></option><%
            }

            if (iDipLevel & ALLIANCE) {
                %><option selected value="<% Write (TEAM_PREARRANGED_DIPLOMACY | TEAM_PREARRANGED_ALLIANCES); %>"><%
                %>Empires begin the game already allied with their teammates<%
                %></option><%
            }

            %></select><%
        }

        // Advanced options
        if (bAdvanced) {

            %><input type="hidden" name="Advanced<% Write(iGameClassKey); %>" value="1"><%
            %><h3>Advanced game creation options:</h3><p><%

            iErrCode = RenderGameConfiguration(iGameClassKey, m_iTournamentKey);
            RETURN_ON_ERROR(iErrCode);
        }

        %><p><%
        WriteButton (BID_CANCEL);
        WriteButton (BID_START);

        }
        break;

    case 8:
        {

        if (m_iTournamentKey == NO_KEY) {
            goto Start;
        }

        Variant vName;
        iErrCode = GetTournamentName (m_iTournamentKey, &vName);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="8"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%

        %><h3>Create a team for the <% Write (vName.GetCharPtr()); %> tournament</h3><p><%

        WriteCreateTournamentTeam(m_iTournamentKey);

        %><p><%
        WriteButton (BID_CANCEL);
        WriteButton (BID_CREATETEAM);

        }
        break;

    case 9:

        if (m_iTournamentKey == NO_KEY) {
            goto Start;
        }

        if (iTeamKey == NO_KEY) {
            goto Admin;
        }

        %><input type="hidden" name="TournamentAdminPage" value="9"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="TeamKey" value="<% Write (iTeamKey); %>"><%

        iErrCode = WriteAdministerTournamentTeam (m_iTournamentKey, iTeamKey);
        RETURN_ON_ERROR(iErrCode);

        break;

    case 10:

        if (m_iTournamentKey == NO_KEY) {
            goto Start;
        }

        // An extra I/O, but it almost never happens
        iErrCode = CacheSystemAlienIcons();
        RETURN_ON_ERROR(iErrCode);

        unsigned int iIcon;
        int iIconAddress;
        iErrCode = GetTournamentIcon (m_iTournamentKey, &iIcon, &iIconAddress);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="10"><p><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%

        iErrCode = WriteTournamentIcon(iIcon, iIconAddress, m_iTournamentKey, "The current tournament icon", false);
        RETURN_ON_ERROR(iErrCode);
        %><p><%

        iErrCode = WriteIconSelection(iIconSelect, iIcon, "tournament");
        RETURN_ON_ERROR(iErrCode);

        break;

    case 11:

        if (m_iTournamentKey == NO_KEY) {
            goto Start;
        }

        if (iTeamKey == NO_KEY) {
            goto Admin;
        }

        // An extra I/O, but it almost never happens
        iErrCode = CacheSystemAlienIcons();
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetTournamentTeamIcon(m_iTournamentKey, iTeamKey, &iIcon, &iIconAddress);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="11"><p><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="TeamKey" value="<% Write (iTeamKey); %>"><%

        iErrCode = WriteTournamentTeamIcon(iIcon, iIconAddress, m_iTournamentKey, iTeamKey, "The current team icon", false);
        RETURN_ON_ERROR(iErrCode);
        %><p><%

        iErrCode = WriteIconSelection (iIconSelect, iIcon, "team");
        RETURN_ON_ERROR(iErrCode);

        break;

    AllGames:
    case 12:
        {

        Variant vName;

        if (m_iTournamentKey == NO_KEY) {
            goto Start;
        }

        iErrCode = GetTournamentName(m_iTournamentKey, &vName);
        RETURN_ON_ERROR(iErrCode);

        Variant** ppvGame = NULL;
        AutoFreeData free_ppvGame(ppvGame);
        unsigned int iNumActiveGames;

        iErrCode = GetTournamentGames(m_iTournamentKey, &ppvGame, &iNumActiveGames);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="12"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%

        %><p><h3><%
        Write(vName.GetCharPtr());

        %> tournament active games:</h3><%

        iErrCode = WriteActiveGameAdministration((const Variant**)ppvGame, iNumActiveGames, 0, 0, false);
        RETURN_ON_ERROR(iErrCode);

        %><p><% WriteButton (BID_CANCEL);

        }
        break;

case 13:

        %><input type="hidden" name="TournamentAdminPage" value="13"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%

        iErrCode = WriteAdministerGame (m_iGameClass, m_iGameNumber, false);
        RETURN_ON_ERROR(iErrCode);

        %><p><% WriteButton (BID_CANCEL);

        break;

case 14:

        if (m_iTournamentKey == NO_KEY || m_iGameClass == NO_KEY) {
            goto Start;
        }

        bool bStarted;
        iErrCode = HasGameStarted (m_iGameClass, m_iGameNumber, &bStarted);
        RETURN_ON_ERROR(iErrCode);

        if (!bStarted)
        {
            goto AllGames;
        }

        %><input type="hidden" name="TournamentAdminPage" value="14"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="GameClass" value="<% Write (m_iGameClass); %>"><%
        %><input type="hidden" name="GameNumber" value="<% Write (m_iGameNumber); %>"><%

        iErrCode = RenderMap(m_iGameClass, m_iGameNumber, NO_KEY, true, NULL, false);
        RETURN_ON_ERROR(iErrCode);

        %><p><% WriteButton (BID_CANCEL);

        break;

case 15:

        {
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

        iErrCode = GetGameClassName (m_iGameClass, pszGameClassName);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="15"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="GameClass" value="<% Write (m_iGameClass); %>"><%
        %><input type="hidden" name="GameNumber" value="<% Write (m_iGameNumber); %>"><%

        %><p>Empire information for <%

        Write(pszGameClassName); %> <% Write(m_iGameNumber); %>:<p><%

        iErrCode = RenderEmpireInformation(m_iGameClass, m_iGameNumber, true);
        RETURN_ON_ERROR(iErrCode);

        WriteButton (BID_CANCEL);

        }
        break;

case 16:
        {

        if (m_iTournamentKey == NO_KEY || m_iGameClass == NO_KEY || iClickedPlanetKey == NO_KEY) {
            goto Start;
        }

        Variant vOptions;

        unsigned int iLivePlanetKey, iDeadPlanetKey;
        int iLivePlanetAddress, iDeadPlanetAddress;
        int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;

        bool bFalse;

        GET_GAME_MAP(pszGameMap, m_iGameClass, m_iGameNumber);

        iErrCode = HasGameStarted (m_iGameClass, m_iGameNumber, &bStarted);
        RETURN_ON_ERROR(iErrCode);

        if (!bStarted)
        {
            AddMessage ("The game hasn't started yet, so it has no map");
            goto AllGames;
        }

        iErrCode = GetEmpirePlanetIcons(m_iEmpireKey, &iLivePlanetKey, &iLivePlanetAddress, &iDeadPlanetKey, &iDeadPlanetAddress);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetGoodBadResourceLimits (
            m_iGameClass,
            m_iGameNumber,
            &iGoodAg,
            &iBadAg,
            &iGoodMin,
            &iBadMin,
            &iGoodFuel,
            &iBadFuel
            );

        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, m_iGameClass, SystemGameClassData::Options, &vOptions);
        RETURN_ON_ERROR(iErrCode);

        Variant* pvPlanetData = NULL;
        AutoFreeData free_pvPlanetData(pvPlanetData);

        iErrCode = t_pCache->ReadRow (pszGameMap, iClickedPlanetKey, &pvPlanetData);
        RETURN_ON_ERROR(iErrCode);

        m_iGameState |= STARTED | GAME_MAP_GENERATED;
        m_iGameClass = m_iGameClass;
        m_iGameNumber = m_iGameNumber;

        %><input type="hidden" name="TournamentAdminPage" value="16"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="GameClass" value="<% Write (m_iGameClass); %>"><%
        %><input type="hidden" name="GameNumber" value="<% Write (m_iGameNumber); %>"><%

        %><p><table width="90%"><%

        iErrCode = WriteUpClosePlanetString (NO_KEY, iClickedPlanetKey, 
            0, iLivePlanetKey, iLivePlanetAddress, iDeadPlanetKey, iDeadPlanetAddress, 0, true, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel,
            1.0, (vOptions.GetInteger() & INDEPENDENCE) != 0, true, false, pvPlanetData, &bFalse);
        RETURN_ON_ERROR(iErrCode);

        %></table><p><%

        WriteButton (BID_VIEWMAP);

        }
        break;

case 17:

        {

        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

        iErrCode = GetGameClassName (m_iGameClass, pszGameClassName);
        RETURN_ON_ERROR(iErrCode);

        %><input type="hidden" name="TournamentAdminPage" value="17"><%
        %><input type="hidden" name="TournamentKey" value="<% Write (m_iTournamentKey); %>"><%
        %><input type="hidden" name="GameClass" value="<% Write (m_iGameClass); %>"><%
        %><input type="hidden" name="GameNumber" value="<% Write (m_iGameNumber); %>"><%

        %><p><table width="65%"><%
        %><tr><td align="center"><%
        %>Are you sure you want to kill <strong><% Write (pszGameClassName); %> <% Write (m_iGameNumber); 
        %></strong>?<p>If so, please send a message to its participants:<%
        %></td></tr></table><%
        %><p><textarea name="DoomMessage" rows="5" cols="45" wrap="physical"></textarea><p><%

        WriteButton (BID_CANCEL);
        WriteButton (BID_KILLGAME);

        }
        break;

    default:
        Assert(false);
        break;
    }

    iErrCode = CloseSystemPage();
    RETURN_ON_ERROR(iErrCode);
%>