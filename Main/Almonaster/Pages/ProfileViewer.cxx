<%
#include "Osal/Algorithm.h"
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

if (InitializeEmpire(false) != OK)
{
    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

int iErrCode, iProfileViewerPage = 0;
unsigned int iTargetEmpireKey = NO_KEY, * piSearchEmpireKey = NULL, iLastKey = 0, iNumSearchEmpires = 0, iGameClassKey = NO_KEY;

RangeSearchColumnDefinition sc [MAX_NUM_SEARCH_COLUMNS];
RangeSearchDefinition sd;
sd.pscColumns = sc;

const char* ppszFormName [MAX_NUM_SEARCH_COLUMNS];
const char* ppszColName1 [MAX_NUM_SEARCH_COLUMNS];
const char* ppszColName2 [MAX_NUM_SEARCH_COLUMNS];

if (m_iReserved != NO_KEY) {
    iTargetEmpireKey = m_iReserved;
    iProfileViewerPage = 2;
}

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (WasButtonPressed (BID_CANCEL)) {
        m_bRedirectTest = false;
    } else {

        if ((pHttpForm = m_pHttpRequest->GetForm ("ProfileViewerPage")) == NULL) {
            goto Redirection;
        }
        int iProfileViewerPageSubmit = pHttpForm->GetIntValue();

        switch (iProfileViewerPageSubmit) {

        case 0:
            {

            if (WasButtonPressed (BID_SEARCH)) {

SearchResults:

                iErrCode = HandleSearchSubmission (
                    sd,
                    ppszFormName,
                    ppszColName1,
                    ppszColName2,
                    &piSearchEmpireKey,
                    &iNumSearchEmpires,
                    &iLastKey
                    );

                switch (iErrCode) {

                case OK:
                case ERROR_TOO_MANY_HITS:

                    if (iNumSearchEmpires > 0) {

                        if (iNumSearchEmpires == 1 && iLastKey == NO_KEY) {
                            iTargetEmpireKey = piSearchEmpireKey[0];
                            t_pCache->FreeKeys (piSearchEmpireKey);
                            iProfileViewerPage = 2;
                        } else {
                            iProfileViewerPage = 1;
                        }
                        break;
                    }

                    // Fall through if 0 empires

                case ERROR_DATA_NOT_FOUND:

                    AddMessage ("No empires matched your search criteria");
                    break;

                case ERROR_INVALID_ARGUMENT:

                    // A form was missing
                    goto Redirection;

                case ERROR_INVALID_QUERY:

                    AddMessage ("You submitted an invalid query");
                    goto Redirection;

                default:

                    {
                    char pszMessage [64];
                    sprintf(pszMessage, "Error %i occurred", iErrCode);
                    AddMessage (pszMessage);
                    }
                    break;
                }
            }

            }

            break;

        case 1:
            {

            unsigned int iHash;
            const char* pszStart;
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewProfile")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "ViewProfile.%d.%d", &iTargetEmpireKey, &iHash) == 2) {

                if (!VerifyEmpireNameHash (iTargetEmpireKey, iHash)) {
                    AddMessage ("That empire no longer exists");
                } else {
                    iProfileViewerPage = 2;
                }

                m_bRedirectTest = false;
            }

            if (WasButtonPressed (BID_SEARCH)) {
                goto SearchResults;
            }

            }

            break;

        case 2:

            {

            // Get key
            if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpireKey")) == NULL) {
                goto Redirection;
            }
            const char* pszTargetEmpire = pHttpForm->GetValue();
            iTargetEmpireKey = pHttpForm->GetIntValue();

            // Send messages
            if (m_iPrivilege > GUEST && WasButtonPressed (BID_SENDMESSAGE)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL) {
                    goto Redirection;
                }
                const char* pszMessage = pHttpForm->GetValue();

                Variant vSentName;
                if (pszMessage != NULL && pszTargetEmpire != NULL)
                {
                    iErrCode = CacheEmpireAndMessages(iTargetEmpireKey);
                    if (iErrCode != OK)
                        return iErrCode;

                    iErrCode = SendSystemMessage(iTargetEmpireKey, pszMessage, m_iEmpireKey, 0);
                    switch (iErrCode) {

                    case OK:
                        Check (GetEmpireName (iTargetEmpireKey, &vSentName));
                        AddMessage ("Your message was sent to ");
                        AppendMessage (vSentName.GetCharPtr());
                        break;

                    case ERROR_CANNOT_SEND_MESSAGE:

                        AddMessage ("You are not allowed to send system messages");
                        break;

                    case ERROR_EMPIRE_DOES_NOT_EXIST:

                        AddMessage ("That empire no longer exists");
                        break;

                    default:
                        AddMessage ("Your message could not be sent due to error ");
                        AppendMessage (iErrCode);
                        return iErrCode;
                    }

                } else {

                    AddMessage ("Your message was blank");
                }

                m_bRedirectTest = false;
                iProfileViewerPage = 2;
                break;
            }

            // View PGC
            if (WasButtonPressed (BID_VIEWEMPIRESGAMECLASSES)) {
                m_bRedirectTest = false;
                iProfileViewerPage = 4;
                break;
            }

            // View nuke history
            if (WasButtonPressed (BID_VIEWEMPIRESNUKEHISTORY)) {
                m_bRedirectTest = false;
                iProfileViewerPage = 5;
                break;
            }

            // View PT
            if (WasButtonPressed (BID_VIEWEMPIRESTOURNAMENTS)) {
                m_bRedirectTest = false;
                iProfileViewerPage = 7;
                break;
            }

            // Lookup
            if (WasButtonPressed (BID_LOOKUP)) {
                m_bRedirectTest = false;
                iProfileViewerPage = 2;
                break;
            }

            // Login
            if (WasButtonPressed (BID_LOGIN))
            {
                pHttpForm = m_pHttpRequest->GetForm ("Switch");
                if (pHttpForm != NULL)
                {
                    unsigned int iSwitch = pHttpForm->GetIntValue();

                    bool bAuth;
                    Check(CheckAssociation(m_iEmpireKey, iSwitch, &bAuth));
                    if (bAuth)
                    {
                        m_iReserved = 0;
                        m_i64SecretKey = 0;
                        m_vPassword = (const char*)NULL;
                        m_vEmpireName = (const char*)NULL;
                        m_iEmpireKey = iSwitch;

                        Check(CacheEmpire(iSwitch));
                        Check(HtmlLoginEmpire());
                        Check(InitializeEmpire(false));
                        return Redirect (ACTIVE_GAME_LIST);
                    }
                    else
                    {
                        AddMessage ("Access denied");
                    }
                }
            }

            }

            break;

        case 4:
            {

            int iGameNumber;
            bool bFlag = false;

            const char* pszStart;
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

                GameOptions goOptions;

                m_bRedirectTest = false;

                // Check for advanced
                char pszAdvanced [128];
                sprintf(pszAdvanced, "Advanced%i", iGameClassKey);

                if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                    iProfileViewerPage = 6;
                    break;
                }

                iErrCode = GetDefaultGameOptions (iGameClassKey, &goOptions);
                if (iErrCode != OK) {
                    AddMessage ("Could not read default game options");
                    goto Redirection;
                }

                goOptions.iNumEmpires = 1;
                goOptions.piEmpireKey = &m_iEmpireKey;

                // Create the game
                iErrCode = CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

                HANDLE_CREATE_GAME_OUTPUT (iErrCode);
            }

            // Test for gameclass deletions and undeletions
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("DeleteGameClass")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "DeleteGameClass%d", &iGameClassKey) == 1) {

                m_bRedirectTest = false;

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                if (iErrCode == OK) {

                    if (m_iEmpireKey == iOwnerKey ||
                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                        ) {

                        iErrCode = DeleteGameClass (iGameClassKey, &bFlag);

                        if (iErrCode == OK) {
                            if (bFlag) {
                                AddMessage ("The GameClass was deleted");
                            } else {
                                AddMessage ("The GameClass has been marked for deletion");
                            }
                        }
                        else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
                            AddMessage ("The GameClass no longer exists");
                        }
                        else {
                            AddMessage ("An error occurred deleting the gameclass: ");
                            AppendMessage (iErrCode);
                        }
                    }
                }
            }

            else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UndeleteGameClass")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "UndeleteGameClass%d", &iGameClassKey) == 1) {

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                if (iErrCode == OK) {

                    if (m_iEmpireKey == iOwnerKey ||
                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                        ) {

                        iErrCode = UndeleteGameClass (iGameClassKey);
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

                            AddMessage ("An error occurred undeleting the gameclass: ");
                            AppendMessage (iErrCode);
                            break;
                        }
                    }
                }

                m_bRedirectTest = false;
            }

            else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("HaltGameClass")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "HaltGameClass%d", &iGameClassKey) == 1) {

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                if (iErrCode == OK) {

                    if (m_iEmpireKey == iOwnerKey ||
                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                        ) {

                        iErrCode = HaltGameClass (iGameClassKey);

                        if (iErrCode == OK) {
                            AddMessage ("The GameClass was halted");
                        }
                        else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
                            AddMessage ("The GameClass no longer exists");
                        }
                        else {
                            AddMessage ("An error occurred halting the gameclass: ");
                            AppendMessage (iErrCode);
                        }
                    }
                }

                m_bRedirectTest = false;
            }

            else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UnhaltGameClass")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "UnhaltGameClass%d", &iGameClassKey) == 1) {

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                if (iErrCode == OK) {

                    if (m_iEmpireKey == iOwnerKey ||
                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                        ) {

                        iErrCode = UnhaltGameClass (iGameClassKey);
                        switch (iErrCode) {

                        case OK:

                            AddMessage ("The GameClass was unhalted");
                            break;

                        case ERROR_GAMECLASS_DOES_NOT_EXIST:

                            AddMessage ("The GameClass no longer exists");
                            break;

                        case ERROR_GAMECLASS_NOT_HALTED:

                            AddMessage ("The GameClass was not halted");
                            break;

                        default:

                            AddMessage ("An error occurred unhalting the gameclass: ");
                            AppendMessage (iErrCode);
                            break;
                        }
                    }
                }

                m_bRedirectTest = false;
            }

            }
            break;

        case 5:

            {

            }
            break;

        case 6:

            // Check for choose
            if (WasButtonPressed (BID_START) || WasButtonPressed (BID_BLOCK)) {

                int iGameNumber;

                GameOptions goOptions;
                InitGameOptions (&goOptions);

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
                    iProfileViewerPage = 0;
                    break;
                }
                iGameClassKey = pHttpForm->GetIntValue();

                iErrCode = ParseGameConfigurationForms (iGameClassKey, NO_KEY, NULL, &goOptions);
                if (iErrCode != OK) {
                    iProfileViewerPage = 6;
                    break;
                }

                goOptions.iNumEmpires = 1;
                goOptions.piEmpireKey = &m_iEmpireKey;

                // Create the game
                iErrCode = CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

                ClearGameOptions (&goOptions);

                HANDLE_CREATE_GAME_OUTPUT (iErrCode);
            }

            break;

        case 7:
            {
            const char* pszStart = NULL;

            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewTourneyInfo")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "ViewTourneyInfo%d", &m_iReserved) == 1) {
                return Redirect (TOURNAMENTS);
            }

            }
            break;

        default:
            Assert (false);
        }
    }
} 

Redirection:
if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
    Check(RedirectOnSubmit(&pageRedirect, &bRedirected));
    if (bRedirected)
    {
        return Redirect(pageRedirect);
    }
}

Check(OpenSystemPage(false));

// Individual page stuff starts here
switch (iProfileViewerPage) {

case 1:
    {

    %><input type="hidden" name="ProfileViewerPage" value="1"><%

    Assert (piSearchEmpireKey != NULL);

    RenderSearchResults (
        sd,
        ppszFormName,
        ppszColName1,
        ppszColName2,
        piSearchEmpireKey,
        iNumSearchEmpires,
        iLastKey
        );

    t_pCache->FreeKeys (piSearchEmpireKey);

    }

    break;

case 0:
    {

    %><input type="hidden" name="ProfileViewerPage" value="0"><%

    bool bShowAdvanced = m_iPrivilege >= PRIVILEGE_FOR_ADVANCED_SEARCH;
    if (bShowAdvanced) {
        bShowAdvanced = (m_iSystemOptions & SHOW_ADVANCED_SEARCH_INTERFACE) != 0;
    }

    RenderSearchForms (bShowAdvanced);

    }

    break;

case 2:
    {

    %><input type="hidden" name="ProfileViewerPage" value="2"><%
    Check(WriteProfile(m_iEmpireKey, iTargetEmpireKey, false, true, true)); 
    %><p><%

    }

    break;

case 4:
    {

    %><input type="hidden" name="ProfileViewerPage" value="4"><% 

    WritePersonalGameClasses (iTargetEmpireKey);

    }

    break;

case 5:
    {

    %><input type="hidden" name="ProfileViewerPage" value="5"><%

    Assert (iTargetEmpireKey != NO_KEY);
    WriteNukeHistory (iTargetEmpireKey);

    }

    break;

case 6:

    int iGameNumber;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    Check (GetGameClassName (iGameClassKey, pszGameClassName));
    Check (GetNextGameNumber (iGameClassKey, &iGameNumber));

    %><input type="hidden" name="ProfileViewerPage" value="6"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%

    %><h3>Advanced game creation options:<p><%

    Write (pszGameClassName); %> <% Write (iGameNumber);

    %></h3><p><%

    RenderGameConfiguration (iGameClassKey, NO_KEY);

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_START);

    break;

case 7:

    %><input type="hidden" name="ProfileViewerPage" value="7"><%

    Assert (iTargetEmpireKey != NO_KEY);
    Check(WritePersonalTournaments(iTargetEmpireKey));

    break;

default:
    Assert (false);
}

CloseSystemPage();

%>