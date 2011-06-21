<% #include "Almonaster.h"
#include "GameEngine.h"
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

INITIALIZE_EMPIRE

// Make sure that the unprivileged don't abuse this:
if (m_iPrivilege < ADMINISTRATOR) {
    AddMessage ("You are not authorized to view this page");
    return Redirect (LOGIN);
}

bRedirectTest = false;  // No warning
return Render_TournamentManager (SYSTEM);
}

int HtmlRenderer::Render_TournamentManager (unsigned int iOwnerKey) {

bool bRedirectTest = true;
IHttpForm* pHttpForm;

int iErrCode, iTAdminPage = 0, iIconSelect = 0;
int iGameNumber = 0;

unsigned int i, iTournamentKey = NO_KEY, iGameClass = NO_KEY, iGameClassKey = NO_KEY, iTeamKey = NO_KEY,
    iClickedPlanetKey = NO_KEY, iDeleteEmpire = NO_KEY;
    
const char* pszInviteEmpire = NULL;
bool bAdvanced = false;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentAdminPage")) == NULL) {
        goto Redirection;
    }
    int iTournamentAdminPageSubmit = pHttpForm->GetIntValue();

    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentKey")) != NULL) {

        unsigned int iRealOwner;
        iTournamentKey = pHttpForm->GetUIntValue();

        // Simple security check
        iErrCode = GetTournamentOwner (iTournamentKey, &iRealOwner);
        if (iErrCode != OK || iRealOwner != iOwnerKey) {
            AddMessage ("Tournament ownership verification failed");
            iTAdminPage = 1;
            goto Redirection;
        }
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TeamKey")) != NULL) {
        iTeamKey = pHttpForm->GetIntValue();
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) != NULL) {
        iGameClassKey = pHttpForm->GetUIntValue();
    }

    // Get game class, game number
    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClass")) != NULL) {
        iGameClass = pHttpForm->GetUIntValue();
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("GameNumber")) != NULL) {
        iGameNumber = pHttpForm->GetIntValue();
    }

    if (iGameClass != NO_KEY) {

        unsigned int iCheckTournament;

        iErrCode = GetGameClassTournament (iGameClass, &iCheckTournament);
        if (iErrCode != OK || iCheckTournament != iTournamentKey) {
            AddMessage ("Game ownership verification failed");
            iTAdminPage = 1;
            goto Redirection;
        }
    }

    //
    switch (iTournamentAdminPageSubmit) {

    case 0:

        if (WasButtonPressed (BID_CREATENEWTOURNAMENT)) {
            iTAdminPage = 1;
            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_DELETETOURNAMENT)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("DelTournament")) == NULL) {
                goto Redirection;
            }

            iErrCode = DeleteTournament (iOwnerKey, pHttpForm->GetIntValue(), false);
            switch (iErrCode) {

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

                AddMessage ("The tournament could not be deleted. The error was ");
                AppendMessage (iErrCode);
                break;
            }

            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_ADMINISTERTOURNAMENT)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("AdminTournament")) == NULL) {
                goto Redirection;
            }

            iTournamentKey = pHttpForm->GetUIntValue();
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

    case 1:

        if (WasButtonPressed (BID_CREATENEWTOURNAMENT)) {

            iErrCode = ProcessCreateTournament (iOwnerKey);
            if (iErrCode == OK) {
                iTAdminPage = 0;
            } else {
                iTAdminPage = 1;    // Repaint form
            }

            bRedirectTest = false;
            goto Redirection;
        }
        break;

    case 2:
        {

        Variant vOldString;
        bool bFlag;
        unsigned int iRealTourney;

        if (iTournamentKey == NO_KEY) {
            iTAdminPage = 1;
            goto Redirection;
        }

        // Description
        if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentDescription")) == NULL) {
            goto Redirection;
        }

        if (GetTournamentDescription (iTournamentKey, &vOldString) == OK &&
            String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

            iErrCode = SetTournamentDescription (iTournamentKey, pHttpForm->GetValue());
            if (iErrCode == OK) {
                AddMessage ("The tournament description was updated");
            } else {
                AddMessage ("The tournament description could not be updated. The error was ");
                AppendMessage (iErrCode);
            }
        }

        // Url
        if ((pHttpForm = m_pHttpRequest->GetForm ("WebPageURL")) == NULL) {
            goto Redirection;
        }

        if (GetTournamentUrl (iTournamentKey, &vOldString) == OK &&
            String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

            iErrCode = SetTournamentUrl (iTournamentKey, pHttpForm->GetValue());
            if (iErrCode == OK) {
                AddMessage ("The tournament webpage was updated");
            } else {
                AddMessage ("The tournament webpage could not be updated. The error was ");
                AppendMessage (iErrCode);
            }
        }

        // News
        if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentNews")) == NULL) {
            goto Redirection;
        }

        if (GetTournamentNews (iTournamentKey, &vOldString) == OK &&
            String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

            char* pszBuffer = NULL;

            if (String::StrLen (pHttpForm->GetValue()) > MAX_TOURNAMENT_NEWS_LENGTH) {

                pszBuffer = (char*) StackAlloc (MAX_TOURNAMENT_NEWS_LENGTH + 1);
                memcpy (pszBuffer, pHttpForm->GetValue(), MAX_TOURNAMENT_NEWS_LENGTH);
                pszBuffer [MAX_TOURNAMENT_NEWS_LENGTH] = '\0';

            } else {

                pszBuffer = (char*) pHttpForm->GetValue();
            }

            iErrCode = SetTournamentNews (iTournamentKey, pszBuffer);
            if (iErrCode == OK) {
                AddMessage ("The tournament news was updated");

                if (pszBuffer != pHttpForm->GetValue()) {
                    AppendMessage (". Your news submission was truncated because it was too long");
                }

            } else {
                AddMessage ("The tournament news could not be updated. The error was ");
                AppendMessage (iErrCode);
            }
        }

        // Handle icon selection request
        if (WasButtonPressed (BID_CHOOSE)) {

            bRedirectTest = false;

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
            bRedirectTest = false;
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

            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_DELETEEMPIRE)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("DelEmp")) == NULL) {
                goto Redirection;
            }
            iDeleteEmpire = pHttpForm->GetUIntValue();

            iTAdminPage = 5;
            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_CREATETEAM)) {
            iTAdminPage = 8;
            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_DELETETEAM)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("DelTeam")) == NULL) {
                goto Redirection;
            }

            iErrCode = DeleteTournamentTeam (iTournamentKey, pHttpForm->GetIntValue());
            if (iErrCode == OK) {
                AddMessage ("The team was deleted");
            } else {
                AddMessage ("The team could not be deleted. The error was ");
                AppendMessage (iErrCode);
            }

            iTAdminPage = 2;
            bRedirectTest = false;
            goto Redirection;
        }

        // GameClass stuff
        if (WasButtonPressed (BID_DELETEGAMECLASS)) {

            bRedirectTest = false;

            if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteGC")) == NULL) {
                goto Redirection;
            }

            // Verify ownership
            iErrCode = GetGameClassTournament (pHttpForm->GetIntValue(), &iRealTourney);
            if (iErrCode != OK || iRealTourney != iTournamentKey) {
                AddMessage ("Gameclass ownership verification failed");
                iTAdminPage = 1;
                goto Redirection;
            }

            iErrCode = DeleteGameClass (pHttpForm->GetIntValue(), &bFlag);

            if (iErrCode == OK) {
                if (bFlag) {
                    AddMessage ("The GameClass was deleted");
                } else {
                    AddMessage ("The GameClass has been marked for deletion");
                }
            } else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
                AddMessage ("The GameClass no longer exists");
            }
            else {
                char pszMessage [128];
                sprintf (pszMessage, "Error %i occurred deleting the gameclass", iErrCode);
                AddMessage (pszMessage);
            }

            iTAdminPage = 2;
            goto Redirection;
        }

        if (WasButtonPressed (BID_UNDELETEGAMECLASS)) {

            bRedirectTest = false;

            if ((pHttpForm = m_pHttpRequest->GetForm ("UndeleteGC")) == NULL) {
                goto Redirection;
            }

            // Verify ownership
            iErrCode = GetGameClassTournament (pHttpForm->GetIntValue(), &iRealTourney);
            if (iErrCode != OK || iRealTourney != iTournamentKey) {
                AddMessage ("Gameclass ownership verification failed");
                iTAdminPage = 1;
                goto Redirection;
            }

            iErrCode = UndeleteGameClass (pHttpForm->GetIntValue());
            switch (iErrCode) {

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

                {
                char pszMessage [256];
                sprintf (pszMessage, "Error %i occurred undeleting the GameClass", iErrCode);
                AddMessage (pszMessage);
                }

                break;
            }

            iTAdminPage = 2;
            goto Redirection;
        }

        // Handle game class halt
        if (WasButtonPressed (BID_HALTGAMECLASS)) {

            bRedirectTest = false;

            if ((pHttpForm = m_pHttpRequest->GetForm ("HaltGC")) == NULL) {
                goto Redirection;
            }
            
            // Verify ownership
            iErrCode = GetGameClassTournament (pHttpForm->GetIntValue(), &iRealTourney);
            if (iErrCode != OK || iRealTourney != iTournamentKey) {
                AddMessage ("Gameclass ownership verification failed");
                iTAdminPage = 1;
                goto Redirection;
            }

            iErrCode = HaltGameClass (pHttpForm->GetIntValue());
            if (iErrCode == OK) {
                AddMessage ("The GameClass was halted");
            }
            else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
                AddMessage ("The GameClass no longer exists");
            }
            else {
                char pszMessage [256];
                sprintf (pszMessage, "Error %i occurred halting the GameClass", iErrCode);
                AddMessage (pszMessage);
            }

            iTAdminPage = 2;
            goto Redirection;
        }

        // Handle game class unhalting
        if (WasButtonPressed (BID_UNHALTGAMECLASS)) {

            bRedirectTest = false;

            if ((pHttpForm = m_pHttpRequest->GetForm ("UnhaltGC")) == NULL) {
                goto Redirection;
            }

            // Verify ownership
            iErrCode = GetGameClassTournament (pHttpForm->GetIntValue(), &iRealTourney);
            if (iErrCode != OK || iRealTourney != iTournamentKey) {
                AddMessage ("Gameclass ownership verification failed");
                iTAdminPage = 1;
                goto Redirection;
            }

            iErrCode = UnhaltGameClass (pHttpForm->GetIntValue());
            switch (iErrCode) {

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

                {
                char pszMessage [256];
                sprintf (pszMessage, "Error %i occurred unhalting the GameClass", iErrCode);
                AddMessage (pszMessage);
                }

                break;
            }

            iTAdminPage = 2;
            goto Redirection;
        }

        if (WasButtonPressed (BID_START)) {
            iTAdminPage = 6;
            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_ADMINISTERTEAM)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("AdminTeam")) == NULL) {
                goto Redirection;
            }

            iTeamKey = pHttpForm->GetIntValue();

            iTAdminPage = 9;
            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_VIEWGAMEINFORMATION)) {

            iTAdminPage = 12;
            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_UPDATE)) {
            iTAdminPage = 2;
            goto Redirection;
        }

        }
        break;

    case 3:

        if (iTournamentKey == NO_KEY) {
            iTAdminPage = 1;
            goto Redirection;
        }

        // Handle new gameclass creation
        if (WasButtonPressed (BID_CREATENEWGAMECLASS)) {

            bRedirectTest = false;
            if (ProcessCreateGameClassForms (iOwnerKey, iTournamentKey) != OK) {
                iTAdminPage = 3;
            } else {
                // Back to administer page
                iTAdminPage = 2;
            }
            goto Redirection;
        }

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 2;
            goto Redirection;
        }

        break;

    case 4:

        if (iTournamentKey == NO_KEY) {
            iTAdminPage = 1;
            goto Redirection;
        }

        if (WasButtonPressed (BID_INVITEEMPIRE)) {

            bRedirectTest = false;

            if ((pHttpForm = m_pHttpRequest->GetForm ("InviteEmpireKey")) == NULL) {
                goto Redirection;
            }

            iErrCode = InviteEmpireIntoTournament (iTournamentKey, iOwnerKey, m_iEmpireKey, pHttpForm->GetIntValue());
            if (iErrCode != OK) {

                if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT) {
                    AddMessage ("The empire is already in the tournament");
                }

                else {
                    AddMessage ("The empire could not be invited");
                }

            } else {
                AddMessage ("The invitation was sent to the empire");
            }
        }

        // Lookup
        if (WasButtonPressed (BID_LOOKUP)) {
            bRedirectTest = false;
            iTAdminPage = 4;
            break;
        }

        // Back to administer page
        iTAdminPage = 2;

        break;

    case 5:

        if (iTournamentKey == NO_KEY) {
            iTAdminPage = 1;
            goto Redirection;
        }

        if (WasButtonPressed (BID_DELETEEMPIRE)) {

            bRedirectTest = false;

            if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteEmpireKey")) == NULL) {
                goto Redirection;
            }

            iErrCode = DeleteEmpireFromTournament (iTournamentKey, pHttpForm->GetIntValue());
            if (iErrCode == OK) {
                AddMessage ("The empire was deleted from the tournament");
            } else if (iErrCode == ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT) {
                AddMessage ("The empire is not longer in the tournament");
            } else if (iErrCode == ERROR_EMPIRE_IS_IN_GAMES) {
                AddMessage ("The empire could not be deleted from the tournament because it is still in a tournament game");
            } else {
                AddMessage ("The empire was not deleted from the tournament. The error was ");
                AppendMessage (iErrCode);
            }
        }

        // Lookup
        if (WasButtonPressed (BID_LOOKUP)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteEmpireKey")) == NULL) {
                goto Redirection;
            }

            iDeleteEmpire = pHttpForm->GetUIntValue();

            bRedirectTest = false;
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

            bRedirectTest = false;

            // Check for advanced option
            char pszAdvanced [128];
            sprintf (pszAdvanced, "Advanced%i", iGameClassKey);

            if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                bAdvanced = true;
            }

            iTAdminPage = 7;
            goto Redirection;
        }

        }
        break;

    case 7:

        if (iTournamentKey == NO_KEY) {
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
            sprintf (pszAdvanced, "Advanced%i", iGameClassKey);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                bAdvanced = true;
            }

            iErrCode = StartTournamentGame (iTournamentKey, iTeamOptions, bAdvanced);
            if (iErrCode == OK || iErrCode == ERROR_GAMECLASS_IS_NOT_IN_TOURNAMENT) {
                iTAdminPage = 2;
            } else {
                iTAdminPage = 7;
            }
            goto Redirection;
        }

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 2;
            goto Redirection;
        }

        break;

    case 8:

        if (iTournamentKey == NO_KEY) {
            iTAdminPage = 1;
            goto Redirection;
        }

        // Handle new team creation
        if (WasButtonPressed (BID_CREATETEAM)) {

            bRedirectTest = false;
            if (ProcessCreateTournamentTeam (iTournamentKey) != OK) {
                iTAdminPage = 8;
            } else {
                // Back to administer page
                iTAdminPage = 2;
            }
            goto Redirection;
        }

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 2;
            goto Redirection;
        }

        break;

    case 9:
        {

        Variant vOldString;

        if (iTournamentKey == NO_KEY) {
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

        if (GetTournamentTeamDescription (iTournamentKey, iTeamKey, &vOldString) == OK &&
            String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

            iErrCode = SetTournamentTeamDescription (iTournamentKey, iTeamKey, pHttpForm->GetValue());
            if (iErrCode == OK) {
                AddMessage ("The team description was updated");
            } else {
                AddMessage ("The team description could not be updated. The error was ");
                AppendMessage (iErrCode);
            }
        }

        // Url
        if ((pHttpForm = m_pHttpRequest->GetForm ("TeamWebPageURL")) == NULL) {
            goto Redirection;
        }

        if (GetTournamentTeamUrl (iTournamentKey, iTeamKey, &vOldString) == OK &&
            String::StrCmp (vOldString.GetCharPtr(), pHttpForm->GetValue()) != 0) {

            iErrCode = SetTournamentTeamUrl (iTournamentKey, iTeamKey, pHttpForm->GetValue());
            if (iErrCode == OK) {
                AddMessage ("The team webpage was updated");
            } else {
                AddMessage ("The team webpage could not be updated. The error was ");
                AppendMessage (iErrCode);
            }
        }

        // Handle icon selection request
        if (WasButtonPressed (BID_CHOOSE)) {

            bRedirectTest = false;

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

            iErrCode = SetEmpireTournamentTeam (iTournamentKey, pHttpForm->GetIntValue(), iTeamKey);
            if (iErrCode == OK) {
                AddMessage ("The empire was added to the team");
            } else {
                AddMessage ("The empire could not be added to the team. The error was ");
                AppendMessage (iErrCode);
            }

            bRedirectTest = false;
            iTAdminPage = 9;
            goto Redirection;
        }

        if (WasButtonPressed (BID_DELETEEMPIRE)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("DelFromTeam")) == NULL) {
                goto Redirection;
            }

            iErrCode = SetEmpireTournamentTeam (iTournamentKey, pHttpForm->GetIntValue(), NO_KEY);
            if (iErrCode == OK) {
                AddMessage ("The empire was deleted from the team");
            } else {
                AddMessage ("The empire could not be deleted from the team. The error was ");
                AppendMessage (iErrCode);
            }

            bRedirectTest = false;
            iTAdminPage = 9;
            goto Redirection;
        }

        if (WasButtonPressed (BID_UPDATE)) {
            bRedirectTest = false;
            iTAdminPage = 9;
            goto Redirection;
        }

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 2;
            goto Redirection;
        }

        }
        break;

    case 10:
        {

        unsigned int iOldIcon, iIcon;

        if (GetTournamentIcon (iTournamentKey, &iOldIcon) == OK &&
            HandleIconSelection (&iIcon, BASE_UPLOADED_TOURNAMENT_ICON_DIR, iTournamentKey, NO_KEY) == OK) {

            if (iIcon == UPLOADED_ICON) {

                if (iOldIcon != UPLOADED_ICON) {
                    iErrCode = SetTournamentIcon (iTournamentKey, UPLOADED_ICON);
                    if (iErrCode != OK) {
                        AddMessage ("The icon could not be set");
                    }
                }
            }

            else if (iOldIcon != iIcon) {

                iErrCode = SetTournamentIcon (iTournamentKey, iIcon);
                if (iErrCode == OK) {
                    AddMessage ("The icon was updated");
                } else {
                    AddMessage ("That icon no longer exists");
                }
            }

            else {
                AddMessage ("That was the same icon");
            }

            bRedirectTest = false;
        }

        iTAdminPage = 2;
        goto Redirection;

        }
        break;

    case 11:

        {

        unsigned int iOldIcon, iIcon;

        if (GetTournamentTeamIcon (iTournamentKey, iTeamKey, &iOldIcon) == OK &&
            HandleIconSelection (&iIcon, BASE_UPLOADED_TOURNAMENT_TEAM_ICON_DIR, iTournamentKey, iTeamKey) == OK) {

            if (iIcon == UPLOADED_ICON) {

                if (iOldIcon != UPLOADED_ICON) {
                    iErrCode = SetTournamentTeamIcon (iTournamentKey, iTeamKey, UPLOADED_ICON);
                    if (iErrCode != OK) {
                        AddMessage ("The icon could not be set");
                    }
                }
            }

            else if (iOldIcon != iIcon) {

                iErrCode = SetTournamentTeamIcon (iTournamentKey, iTeamKey, iIcon);
                if (iErrCode == OK) {
                    AddMessage ("The icon was updated");
                } else {
                    AddMessage ("That icon no longer exists");
                }
            }

            else {
                AddMessage ("That was the same icon");
            }

            bRedirectTest = false;
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
            sscanf (pszGame, "AdministerGame%d.%d", &iGameClass, &iGameNumber) == 2) {

            bRedirectTest = false;
            iTAdminPage = 13;
            goto Redirection;
        }

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 2;
            goto Redirection;
        }

        }
        break;

    case 13:
        {

        bool bExist;
        const char* pszMessage;

        if (iGameClass == NO_KEY) {
            iTAdminPage = 2;
            goto Redirection;
        }

        // View map
        if (WasButtonPressed (BID_VIEWMAP)) {
            bRedirectTest = false;
            iTAdminPage = 14;
            goto Redirection;
        }

        // Force update
        if (WasButtonPressed (BID_FORCEUPDATE)) {

            if (ForceUpdate (iGameClass, iGameNumber) == OK) {
                AddMessage ("The game was forcibly updated");
            } else {
                AddMessage ("The game no longer exists");
            }

            if (DoesGameExist (iGameClass, iGameNumber, &bExist) == OK && bExist) {
                iTAdminPage = 13;
            } else {
                iTAdminPage = 12;
            }

            bRedirectTest = false;
            goto Redirection;
        }

        // Delete empire from game
        if (WasButtonPressed (BID_DELETEEMPIRE)) {

            pHttpForm = m_pHttpRequest->GetForm ("DeleteEmpireKey");
            if (pHttpForm != NULL) {

                int iTargetEmpireKey = pHttpForm->GetIntValue();

                iErrCode = ERROR_FAILURE;
                if (!(m_iGameState & STARTED)) {

                    iErrCode = QuitEmpireFromGame (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                    if (iErrCode == ERROR_GAME_HAS_STARTED) {

                        // Try remove
                        iErrCode = RemoveEmpireFromGame (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                    }

                } else {

                    if (m_iGameState & STILL_OPEN) {
                        iErrCode = RemoveEmpireFromGame (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                    }
                }

                if (iErrCode == OK) {
                    AddMessage ("The empire was deleted from the game");
                } else {
                    AddMessage ("The empire could not be deleted from the game");
                }
            }

            if (DoesGameExist (iGameClass, iGameNumber, &bExist) == OK && bExist) {
                iTAdminPage = 13;
            } else {
                iTAdminPage = 12;
            }

            bRedirectTest = false;
            goto Redirection;
        }

        // Restore resigned empire to game
        if (WasButtonPressed (BID_RESTOREEMPIRE)) {

            pHttpForm = m_pHttpRequest->GetForm ("RestoreEmpireKey");
            if (pHttpForm != NULL) {

                int iTargetEmpireKey = pHttpForm->GetIntValue();

                if (!(m_iGameState & STARTED)) {

                    iErrCode = UnresignEmpire (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                    if (iErrCode == OK) {
                        AddMessage ("The empire was restored");
                    } else {
                        char pszMessage [256];
                        sprintf (pszMessage, "Error %i occurred restoring the empire", iErrCode);
                        AddMessage (pszMessage);
                    }
                }
            }

            if (DoesGameExist (iGameClass, iGameNumber, &bExist) == OK && bExist) {
                iTAdminPage = 13;
            } else {
                iTAdminPage = 12;
            }

            bRedirectTest = false;
            goto Redirection;
        }

        // Check for view empire info
        if (WasButtonPressed (BID_VIEWEMPIREINFORMATION)) {

            iTAdminPage = 15;
            bRedirectTest = false;
            goto Redirection;
        }

        // Pause game
        if (WasButtonPressed (BID_PAUSEGAME)) {

            // Flush remaining updates
            bool bExists;
            iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, true, &bExists);

            // Best effort pause the game
            if (iErrCode == OK)
            {
                iErrCode = PauseGame (iGameClass, iGameNumber, true, true);
            }

            if (iErrCode == OK) {
                AddMessage ("The game is now paused");
                iTAdminPage = 13;
            } else {
                AddMessage ("The game no longer exists");
                iTAdminPage = 12;
            }

            bRedirectTest = false;
            goto Redirection;
        }

        // Unpause game
        if (WasButtonPressed (BID_UNPAUSEGAME)) {

            iErrCode = UnpauseGame (iGameClass, iGameNumber, true, true);
            if (iErrCode == OK) {
                AddMessage ("The game is no longer paused");
                iTAdminPage = 13;
            } else {
                AddMessage ("The game no longer exists");
                iTAdminPage = 12;
            }

            bRedirectTest = false;
            goto Redirection;
        }

        // Broadcast message
        if (WasButtonPressed (BID_SENDMESSAGE)) {

            bRedirectTest = false;

            if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL) {
                goto Redirection;
            }
            pszMessage = pHttpForm->GetValue();

            if ((iErrCode = BroadcastGameMessage (
                iGameClass,
                iGameNumber,
                pszMessage,
                m_iEmpireKey,
                MESSAGE_BROADCAST | MESSAGE_TOURNAMENT_ADMINISTRATOR)
                ) == OK) {
                AddMessage ("Your message was broadcast to all empires in the game");
            } else {
                AddMessage ("The game no longer exists");
            }

            iTAdminPage = 13;
            goto Redirection;
        }

        // Kill game
        if (WasButtonPressed (BID_KILLGAME)) {
            bRedirectTest = false;
            iTAdminPage = 17;
            goto Redirection;
        }

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 12;
            goto Redirection;
        }

        }
        break;

    case 14:
        {

        if (iGameClass == NO_KEY) {
            goto Redirection;
        }

        const char* pszStart;
        int iClickedProxyPlanetKey;

        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

            // We clicked on a planet
            iTAdminPage = 16;
            bRedirectTest = false;
            goto Redirection;
        }

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 13;
            goto Redirection;
        }

        }
        break;

    case 15:

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 13;
            goto Redirection;
        }

        break;

    case 16:

        // View map
        if (WasButtonPressed (BID_VIEWMAP)) {

            bRedirectTest = false;
            iTAdminPage = 14;
            goto Redirection;
        }

    case 17:

        // Kill game
        if (WasButtonPressed (BID_KILLGAME)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("DoomMessage")) == NULL) {
                break;
            }
            const char* pszMessage = pHttpForm->GetValue();

            if (DeleteGame (iGameClass, iGameNumber, m_iEmpireKey, pszMessage, 0) == OK) {
                AddMessage ("The game was deleted");
            } else {
                AddMessage ("The game no longer exists");
            }

            bRedirectTest = false;
            iTAdminPage = 12;
            goto Redirection;
        }

        if (WasButtonPressed (BID_CANCEL)) {
            bRedirectTest = false;
            iTAdminPage = 13;
            goto Redirection;
        }

        break;

    default:

        Assert (false);
        break;
    }
}

SYSTEM_REDIRECT_ON_SUBMIT

SYSTEM_OPEN ((iTAdminPage == 10 || iTAdminPage == 11) && iIconSelect == 1)

// Individual page stuff starts here
switch (iTAdminPage) {

Start:
case 0:

    %><input type="hidden" name="TournamentAdminPage" value="0"><%

    WriteTournamentAdministrator (iOwnerKey);
    if (iOwnerKey != SYSTEM) {
        Assert (iOwnerKey == m_iEmpireKey);
        WritePersonalTournaments();
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

    if (iTournamentKey == NO_KEY) {
        goto Start;
    }

    %><input type="hidden" name="TournamentAdminPage" value="2"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%

    WriteAdministerTournament (iTournamentKey);

    break;

case 3:
    {

    Variant vName;

    if (iTournamentKey == NO_KEY) {
        goto Start;
    }

    iErrCode = GetTournamentName (iTournamentKey, &vName);
    if (iErrCode != OK) {
        %><p><strong>That tournament does not exist</strong><%
        goto Start;
    }

    %><input type="hidden" name="TournamentAdminPage" value="3"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%

    %><h3>Create a new GameClass for the <% Write (vName.GetCharPtr()); %> tournament</h3><p><%

    WriteCreateGameClassString (iOwnerKey, iTournamentKey, false);

    %><p><%
    WriteButton (BID_CANCEL);
    WriteButton (BID_CREATENEWGAMECLASS);

    }
    break;

case 4:
    {

    unsigned int iInviteKey = NO_KEY;
    bool bExists = false;
    Variant vInviteName;

    if (pszInviteEmpire == NULL) {

        pHttpForm = m_pHttpRequest->GetForm ("InviteEmpireKey");
        if (pHttpForm != NULL && pHttpForm->GetValue() != NULL) {

            bExists = true;
            iInviteKey = pHttpForm->GetIntValue();
            iErrCode = GetEmpireName (iInviteKey, &vInviteName);
 
        } else {

            iErrCode = ERROR_FAILURE;
        }

    } else {

        iErrCode = DoesEmpireExist (
            pszInviteEmpire,
            &bExists,
            &iInviteKey,
            &vInviteName,
            NULL
            );
    }

    if (iErrCode != OK || !bExists) {
        %><p><strong>That empire does not exist</strong><%
        goto Admin;
    }

    %><input type="hidden" name="TournamentAdminPage" value="4"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="InviteEmpireKey" value="<% Write (iInviteKey); %>"><%

    %><p>Do you wish to invite <strong><% Write (vInviteName.GetCharPtr()); 
    %></strong> to join your tournament?<%

    %><p><%
    WriteButton (BID_CANCEL);
    WriteButton (BID_INVITEEMPIRE);

    %><p><%
    WriteSeparatorString (m_iSeparatorKey);

    WriteProfile (m_iEmpireKey, iInviteKey, false, false, false);

    }
    break;

case 5:
    {

    Variant vDeleteName;

    iErrCode = GetEmpireName (iDeleteEmpire, &vDeleteName);
    if (iErrCode != OK) {
        %><p><strong>That empire does not exist</strong><%
        goto Start;
    }

    %><input type="hidden" name="TournamentAdminPage" value="5"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="DeleteEmpireKey" value="<% Write (iDeleteEmpire); %>"><%

    %><p>Do you wish to delete <strong><% Write (vDeleteName.GetCharPtr()); 
    %></strong> from your tournament?<%

    %><p><%
    WriteButton (BID_CANCEL);
    WriteButton (BID_DELETEEMPIRE);

    %><p><%
    WriteSeparatorString (m_iSeparatorKey);

    WriteProfile (m_iEmpireKey, iDeleteEmpire, false, false, false);

    }
    break;

case 6:
    {

    Variant* pvGameClassInfo = NULL, vName;
    unsigned int* piGameClassKey = NULL, iNumGameClasses;

    iErrCode = GetTournamentGameClasses (
        iTournamentKey,
        &piGameClassKey,
        NULL,
        &iNumGameClasses
        );

    if (iErrCode != OK) {
        %><p><strong>The tournament does not exist</strong><%
        goto Start;
    }

    if (iNumGameClasses == 0) {
        %><p><strong>The tournament has no gameclasses</strong><%
        goto Start;
    }

    iErrCode = GetTournamentName (iTournamentKey, &vName);
    if (iErrCode != OK) {
        %><p><strong>That tournament does not exist</strong><%
        goto Start;
    }

    %><input type="hidden" name="TournamentAdminPage" value="6"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%

    %><h3>Start a new game for the <% Write (vName.GetCharPtr()); %> tournament:</h3><%

    WriteSystemGameListHeader (m_vTableColor.GetCharPtr());

    for (i = 0; i < iNumGameClasses; i ++) {

        if (GetGameClassData (piGameClassKey[i], &pvGameClassInfo) == OK) {

            // Best effort
            iErrCode = WriteSystemGameListData (piGameClassKey[i], pvGameClassInfo);
            FreeData (pvGameClassInfo);
        }
    }
    %></table><%

    }
    break;

case 7:
    {

    Variant vName, * pvEmpireName = NULL, * pvTeamName = NULL;

    int iGameNumber, iMaxNumEmpires, iGameClassOptions, iDipLevel;
    unsigned int* piTeamEmpireKey = NULL, * piTeamKey = NULL, iNumEmpires, iNumTeams, * piEmpireKey = NULL;

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    if (GetGameClassName (iGameClassKey, pszGameClassName) != OK ||
        GetGameClassOptions (iGameClassKey, &iGameClassOptions) != OK ||
        GetGameClassDiplomacyLevel (iGameClassKey, &iDipLevel) != OK ||
        GetMaxNumEmpires (iGameClassKey, &iMaxNumEmpires) != OK ||
        GetNextGameNumber (iGameClassKey, &iGameNumber) != OK) {
        %><p><strong>That gameclass does not exist</strong><%
        goto Start;
    }

    if (GetTournamentName (iTournamentKey, &vName) != OK ||
        GetAvailableTournamentEmpires (iTournamentKey, &piEmpireKey, &piTeamEmpireKey, &pvEmpireName, &iNumEmpires) != OK ||
        GetTournamentTeams (iTournamentKey, &piTeamKey, &pvTeamName, &iNumTeams) != OK) {
        %><p><strong>That tournament does not exist</strong><%
        goto Start;
    }

    if ((unsigned int) iMaxNumEmpires > iNumEmpires) {
        %><p><strong>The tournament does not have enough available empires to start the game</strong><%
        goto Admin;
    }

    %><input type="hidden" name="TournamentAdminPage" value="7"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%

    %><h3>Start a new game for the <% Write (vName.GetCharPtr()); %> tournament: <%
    Write (pszGameClassName); %> <% Write (iGameNumber); %></h3><%
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

                if (piTeamEmpireKey[j] == piTeamKey[i]) {
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
            %><input type="checkbox" name="EmpireSel<% Write (piEmpireKey[i]); %>"><%
            %> <% Write (pvEmpireName[i].GetCharPtr());

            if (piTeamEmpireKey[i] == NO_KEY) {
                %> (<em>Unaffiliated</em>)<%
            } else {

                for (j = 0; j < iNumTeams; j ++) {
                    if (piTeamEmpireKey[i] == piTeamKey[j]) {
                        %> (<strong><% Write (pvTeamName[j].GetCharPtr()); %></strong>)<%
                        break;
                    }
                }

                Assert (j < iNumTeams);
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

        RenderGameConfiguration (iGameClassKey, iTournamentKey);
    }

    %><p><%
    WriteButton (BID_CANCEL);
    WriteButton (BID_START);

    // Cleanup
    if (piEmpireKey != NULL) {
        FreeData (piEmpireKey);
    }

    if (piTeamEmpireKey != NULL) {
        FreeData (piTeamEmpireKey);
    }

    if (pvEmpireName != NULL) {
        delete [] pvEmpireName;
    }

    if (piTeamKey != NULL) {
        FreeKeys (piTeamKey);
    }

    if (pvTeamName != NULL) {
        FreeData (pvTeamName);
    }

    }
    break;

case 8:
    {

    Variant vName;

    if (iTournamentKey == NO_KEY) {
        goto Start;
    }

    iErrCode = GetTournamentName (iTournamentKey, &vName);
    if (iErrCode != OK) {
        %><p><strong>That tournament does not exist</strong><%
        goto Start;
    }

    %><input type="hidden" name="TournamentAdminPage" value="8"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%

    %><h3>Create a team for the <% Write (vName.GetCharPtr()); %> tournament</h3><p><%

    WriteCreateTournamentTeam (iTournamentKey);

    %><p><%
    WriteButton (BID_CANCEL);
    WriteButton (BID_CREATETEAM);

    }
    break;

case 9:

    if (iTournamentKey == NO_KEY) {
        goto Start;
    }

    if (iTeamKey == NO_KEY) {
        goto Admin;
    }

    %><input type="hidden" name="TournamentAdminPage" value="9"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="TeamKey" value="<% Write (iTeamKey); %>"><%

    WriteAdministerTournamentTeam (iTournamentKey, iTeamKey);

    break;

case 10:

    if (iTournamentKey == NO_KEY) {
        goto Start;
    }

    unsigned int iIcon;
    iErrCode = GetTournamentIcon (iTournamentKey, &iIcon);
    if (iErrCode != OK) {
        %><p><strong>That tournament does not exist</strong><%
        goto Start;
    }

    %><input type="hidden" name="TournamentAdminPage" value="10"><p><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%

    WriteTournamentIcon (iIcon, iTournamentKey, "The current tournament icon", false);
    %><p><%

    WriteIconSelection (iIconSelect, iIcon, "tournament");

    break;

case 11:

    if (iTournamentKey == NO_KEY) {
        goto Start;
    }

    if (iTeamKey == NO_KEY) {
        goto Admin;
    }

    iErrCode = GetTournamentTeamIcon (iTournamentKey, iTeamKey, &iIcon);
    if (iErrCode != OK) {
        %><p><strong>That team does not exist</strong><%
        goto Start;
    }

    %><input type="hidden" name="TournamentAdminPage" value="11"><p><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="TeamKey" value="<% Write (iTeamKey); %>"><%

    WriteTournamentTeamIcon (iIcon, iTournamentKey, iTeamKey, "The current team icon", false);
    %><p><%

    WriteIconSelection (iIconSelect, iIcon, "team");

    break;

AllGames:
case 12:
    {

    int* piGameClass, * piGameNumber;
    unsigned int iNumActiveGames;

    Variant vName;

    if (iTournamentKey == NO_KEY) {
        goto Start;
    }

    iErrCode = GetTournamentName (iTournamentKey, &vName);
    if (iErrCode != OK) {
        %><p><strong>That tournament does not exist</strong><%
        goto Start;
    }

    iErrCode = GetTournamentGames (iTournamentKey, &piGameClass, &piGameNumber, &iNumActiveGames);
    if (iErrCode != OK) {
        %><p><strong>That tournament does not exist</strong><%
        goto Start;
    }

    %><input type="hidden" name="TournamentAdminPage" value="12"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%

    %><p><h3><%
    Write (vName.GetCharPtr());

    %> tournament active games:</h3><%

    WriteActiveGameAdministration (piGameClass, piGameNumber, iNumActiveGames, 0, 0, false);

    if (piGameClass != NULL) {
        delete [] piGameClass;
    }

    if (piGameNumber != NULL) {
        delete [] piGameNumber;
    }

    %><p><% WriteButton (BID_CANCEL);

    }
    break;

case 13:

    %><input type="hidden" name="TournamentAdminPage" value="13"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%

    WriteAdministerGame (iGameClass, iGameNumber, false);

    %><p><% WriteButton (BID_CANCEL);

    break;

case 14:

    if (iTournamentKey == NO_KEY || iGameClass == NO_KEY) {
        goto Start;
    }

    bool bStarted;

    iErrCode = HasGameStarted (iGameClass, iGameNumber, &bStarted);
    if (iErrCode != OK) {
        goto AllGames;
    }

    if (!bStarted) {
        goto AllGames;
    }

    %><input type="hidden" name="TournamentAdminPage" value="14"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    RenderMap (iGameClass, iGameNumber, NO_KEY, true, NULL, false);

    %><p><% WriteButton (BID_CANCEL);

    break;

case 15:

    {
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        pszGameClassName[0] = '\0';
    }

    %><input type="hidden" name="TournamentAdminPage" value="15"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    %><p>Empire information for <%

    Write (pszGameClassName); %> <% Write (iGameNumber); %>:<p><%

    RenderEmpireInformation (iGameClass, iGameNumber, true);

    WriteButton (BID_CANCEL);

    }
    break;

case 16:
    {

    if (iTournamentKey == NO_KEY || iGameClass == NO_KEY || iClickedPlanetKey == NO_KEY) {
        goto Start;
    }

    Variant vOptions;

    unsigned int iLivePlanetKey, iDeadPlanetKey;
    int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;

    bool bStarted, bFalse;

    Variant* pvPlanetData = NULL;
    GAME_MAP(pszGameMap, iGameClass, iGameNumber);

    iErrCode = HasGameStarted (iGameClass, iGameNumber, &bStarted);
    if (iErrCode != OK) {
        AddMessage ("That game no longer exists");
        goto AllGames;
    }

    if (!bStarted) {
        AddMessage ("The game hasn't started yet, so it has no map");
        goto AllGames;
    }

    iErrCode = GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);
    if (iErrCode != OK) {
        AddMessage ("That game no longer exists");
        goto Cleanup;
    }

    iErrCode = GetGoodBadResourceLimits (
        iGameClass,
        iGameNumber,
        &iGoodAg,
        &iBadAg,
        &iGoodMin,
        &iBadMin,
        &iGoodFuel,
        &iBadFuel
        );

    if (iErrCode != OK) {
        AddMessage ("That game no longer exists");
        goto Cleanup;
    }

    iErrCode = t_pConn->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vOptions);
    if (iErrCode != OK) {
        AddMessage ("That game no longer exists");
        goto Cleanup;
    }

    iErrCode = t_pConn->ReadRow (pszGameMap, iClickedPlanetKey, &pvPlanetData);
    if (iErrCode != OK) {
        AddMessage ("That game no longer exists");
        goto Cleanup;
    }

    m_iGameState |= STARTED | GAME_MAP_GENERATED;
    m_iGameClass = iGameClass;
    m_iGameNumber = iGameNumber;

    %><input type="hidden" name="TournamentAdminPage" value="16"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    %><p><table width="90%"><%

    // Best effort
    WriteUpClosePlanetString (NO_KEY, iClickedPlanetKey, 
        0, iLivePlanetKey, iDeadPlanetKey, 0, true, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel,
        1.0, (vOptions.GetInteger() & INDEPENDENCE) != 0, true, false, pvPlanetData, &bFalse);

Cleanup:

    if (pvPlanetData != NULL) {
        t_pConn->FreeData (pvPlanetData);
    }

    if (iErrCode != OK) {
        goto AllGames;
    }

    %></table><p><%

    WriteButton (BID_VIEWMAP);

    }
    break;

case 17:

    {

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        goto AllGames;
    }

    %><input type="hidden" name="TournamentAdminPage" value="17"><%
    %><input type="hidden" name="TournamentKey" value="<% Write (iTournamentKey); %>"><%
    %><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    %><p><table width="65%"><%
    %><tr><td align="center"><%
    %>Are you sure you want to kill <strong><% Write (pszGameClassName); %> <% Write (iGameNumber); 
    %></strong>?<p>If so, please send a message to its participants:<%
    %></td></tr></table><%
    %><p><textarea name="DoomMessage" rows="5" cols="45" wrap="physical"></textarea><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_KILLGAME);

    }
    break;

default:

    Assert (false);
    break;
}

SYSTEM_CLOSE

%>