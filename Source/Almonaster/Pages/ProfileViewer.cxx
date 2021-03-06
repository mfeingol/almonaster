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

int iErrCode;

bool bInitialized;
iErrCode = InitializeEmpire(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

int iProfileViewerPage = 0;
unsigned int iTargetEmpireKey = NO_KEY, * piSearchEmpireKey = NULL, iNumSearchEmpires = 0, iGameClassKey = NO_KEY;
bool bMoreHits = false;
AutoFreeKeys free_piSearchEmpireKey(piSearchEmpireKey);

SearchColumnDefinition sc [MAX_NUM_SEARCH_COLUMNS];
SearchDefinition sd;
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

            if (WasButtonPressed (BID_SEARCH))
            {
SearchResults:
                iErrCode = HandleSearchSubmission (
                    sd,
                    ppszFormName,
                    ppszColName1,
                    ppszColName2,
                    &piSearchEmpireKey,
                    &iNumSearchEmpires,
                    &bMoreHits
                    );

                if (iErrCode == ERROR_INVALID_QUERY)
                {
                    AddMessage("Invalid query");
                    goto Redirection;
                }
                RETURN_ON_ERROR(iErrCode);
                        
                if (iNumSearchEmpires == 0)
                {
                    AddMessage("No empires matched your search criteria");
                }
                else if (iNumSearchEmpires == 1 && !bMoreHits)
                {
                    iTargetEmpireKey = piSearchEmpireKey[0];
                    iProfileViewerPage = 2;
                }
                else
                {
                    iProfileViewerPage = 1;
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
                sscanf (pszStart, "ViewProfile.%d.%d", &iTargetEmpireKey, &iHash) == 2)
            {
                iErrCode = CacheProfileData(iTargetEmpireKey);
                RETURN_ON_ERROR(iErrCode);

                bool bVerified;
                iErrCode = VerifyEmpireNameHash(iTargetEmpireKey, iHash, &bVerified);
                RETURN_ON_ERROR(iErrCode);

                if (!bVerified) {
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
                    iErrCode = SendSystemMessage(iTargetEmpireKey, pszMessage, m_iEmpireKey, 0);
                    switch (iErrCode) {

                    case OK:
                        iErrCode = GetEmpireName(iTargetEmpireKey, &vSentName);
                        RETURN_ON_ERROR(iErrCode);
                        AddMessage("Your message was sent to ");
                        AppendMessage(vSentName.GetCharPtr());
                        break;

                    case ERROR_CANNOT_SEND_MESSAGE:
                        AddMessage("You are not allowed to send system messages");
                        break;

                    case ERROR_EMPIRE_DOES_NOT_EXIST:
                        AddMessage("That empire no longer exists");
                        break;

                    case ERROR_STRING_IS_TOO_LONG:
                        AddMessage ("The message was too long");
                        break;

                    default:
                        RETURN_ON_ERROR(iErrCode);
                        break;
                    }
                }
                else
                {
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
                    iErrCode = CheckAssociation(m_iEmpireKey, iSwitch, &bAuth);
                    RETURN_ON_ERROR(iErrCode);
                    if (!bAuth)
                    {
                        AddMessage("Access denied");
                    }
                    else
                    {
                        m_iReserved = 0;
                        m_vEmpireName = (const char*)NULL;
                        m_iEmpireKey = iSwitch;

                        iErrCode = CacheEmpire(iSwitch);
                        RETURN_ON_ERROR(iErrCode);

                        bool bLoggedIn;
                        iErrCode = HtmlLoginEmpire(&bLoggedIn);
                        RETURN_ON_ERROR(iErrCode);
                        if (bLoggedIn)
                        {
                            iErrCode = InitializeEmpire(false, &bInitialized);
                            RETURN_ON_ERROR(iErrCode);
                            if (bInitialized)
                            {
                                // Yay!
                                return Redirect(ACTIVE_GAME_LIST);
                            }
                        }

                        AddMessage("Login failed");
                    }
                }
                break;
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
                RETURN_ON_ERROR(iErrCode);

                goOptions.iNumEmpires = 1;
                goOptions.piEmpireKey = &m_iEmpireKey;

                // Create the game
                iErrCode = CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);
                HANDLE_CREATE_GAME_OUTPUT(iErrCode);
            }

            // Test for gameclass deletions and undeletions
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("DeleteGameClass")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "DeleteGameClass%d", &iGameClassKey) == 1) {

                m_bRedirectTest = false;

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                RETURN_ON_ERROR(iErrCode);

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                    ) {

                    iErrCode = DeleteGameClass(iGameClassKey, &bFlag);
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
                }
            }

            else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UndeleteGameClass")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "UndeleteGameClass%d", &iGameClassKey) == 1) {

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                RETURN_ON_ERROR(iErrCode);

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                    ) {

                    iErrCode = UndeleteGameClass (iGameClassKey);
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
                }

                m_bRedirectTest = false;
            }

            else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("HaltGameClass")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "HaltGameClass%d", &iGameClassKey) == 1) {

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                RETURN_ON_ERROR(iErrCode);

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                    ) {

                    iErrCode = HaltGameClass (iGameClassKey);
                    if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
                    {
                        AddMessage ("The GameClass no longer exists");
                    }
                    else
                    {
                        RETURN_ON_ERROR(iErrCode);
                        AddMessage ("The GameClass was halted");
                    }
                }

                m_bRedirectTest = false;
            }

            else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UnhaltGameClass")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "UnhaltGameClass%d", &iGameClassKey) == 1) {

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                RETURN_ON_ERROR(iErrCode);

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                    ) {

                    iErrCode = UnhaltGameClass (iGameClassKey);
                    switch (iErrCode)
                    {
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
                        RETURN_ON_ERROR(iErrCode);
                        break;
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
                if (iErrCode == WARNING)
                {
                    iProfileViewerPage = 6;
                    break;
                }
                RETURN_ON_ERROR(iErrCode);

                goOptions.iNumEmpires = 1;
                goOptions.piEmpireKey = &m_iEmpireKey;

                // Create the game
                iErrCode = CreateGame(iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);
                ClearGameOptions(&goOptions);
                HANDLE_CREATE_GAME_OUTPUT(iErrCode);
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
            Assert(false);
            break;
        }
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
switch (iProfileViewerPage) {

case 1:
    {

    %><input type="hidden" name="ProfileViewerPage" value="1"><%

    Assert(piSearchEmpireKey != NULL);

    iErrCode = RenderSearchResults (
        sd,
        ppszFormName,
        ppszColName1,
        ppszColName2,
        piSearchEmpireKey,
        iNumSearchEmpires,
        bMoreHits
        );

    RETURN_ON_ERROR(iErrCode);
    }

    break;

case 0:
    {

    %><input type="hidden" name="ProfileViewerPage" value="0"><%

    bool bShowAdvanced = m_iPrivilege >= PRIVILEGE_FOR_ADVANCED_SEARCH;
    if (bShowAdvanced)
    {
        bShowAdvanced = (m_iSystemOptions & SHOW_ADVANCED_SEARCH_INTERFACE) != 0;
    }

    iErrCode = RenderSearchForms(bShowAdvanced);
    RETURN_ON_ERROR(iErrCode);

    }

    break;

case 2:
    {

    %><input type="hidden" name="ProfileViewerPage" value="2"><%
    iErrCode = WriteProfile(m_iEmpireKey, iTargetEmpireKey, false, true, true);
    RETURN_ON_ERROR(iErrCode);
    %><p><%

    }

    break;

case 4:
    {

    %><input type="hidden" name="ProfileViewerPage" value="4"><% 

    iErrCode = WritePersonalGameClasses (iTargetEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    }

    break;

case 5:
    {

    %><input type="hidden" name="ProfileViewerPage" value="5"><%

    Assert(iTargetEmpireKey != NO_KEY);
    iErrCode = WriteNukeHistory (iTargetEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    }

    break;

case 6:

    int iGameNumber;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = GetGameClassName (iGameClassKey, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNextGameNumber (iGameClassKey, &iGameNumber);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="ProfileViewerPage" value="6"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%

    %><h3>Advanced game creation options:<p><%

    Write (pszGameClassName); %> <% Write (iGameNumber);

    %></h3><p><%

    iErrCode = RenderGameConfiguration (iGameClassKey, NO_KEY);
    RETURN_ON_ERROR(iErrCode);

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_START);

    break;

case 7:

    %><input type="hidden" name="ProfileViewerPage" value="7"><%

    Assert(iTargetEmpireKey != NO_KEY);
    iErrCode = WritePersonalTournaments(iTargetEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    break;

default:
    Assert(false);
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>