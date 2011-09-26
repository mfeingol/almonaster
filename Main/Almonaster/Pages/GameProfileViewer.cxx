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
iErrCode = InitializeEmpireInGame(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
bool bRedirected;
iErrCode = InitializeGame(&pageRedirect, &bRedirected);
RETURN_ON_ERROR(iErrCode);
if (bRedirected)
{
    return Redirect(pageRedirect);
}

IHttpForm* pHttpForm;

int iProfilePage = 1, iTargetEmpireKey = NO_KEY, iGameClassKey = NO_KEY;

bool bGameStarted = (m_iGameState & STARTED) != 0;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    // Get target empire
    pHttpForm = m_pHttpRequest->GetForm ("TargetEmpireKey");
    if (pHttpForm == NULL) {
        AddMessage ("Missing TargetEmpireKey form");
        return Redirect (INFO);
    }
    iTargetEmpireKey = pHttpForm->GetIntValue();

    // Handle submissions
    if ((pHttpForm = m_pHttpRequest->GetForm ("ProfilePage")) == NULL) {
        return Redirect (LOGIN);
    }

    int iProfilePageSubmit = pHttpForm->GetIntValue();

    switch (iProfilePageSubmit) {

    case 1:
        {

        // Check for Nuke history
        if (WasButtonPressed (BID_VIEWEMPIRESNUKEHISTORY)) {

            iProfilePage = 2;
            m_bRedirectTest = false;
            break;
        }

        // Check for personal game classes
        if (WasButtonPressed (BID_VIEWEMPIRESGAMECLASSES)) {

            iProfilePage = 3;
            m_bRedirectTest = false;
            break;
        }

        // Check for personal tournaments
        if (WasButtonPressed (BID_VIEWEMPIRESTOURNAMENTS)) {

            iProfilePage = 5;
            m_bRedirectTest = false;
            break;
        }

        // Lookup
        if (WasButtonPressed (BID_LOOKUP)) {
            iProfilePage = 1;
            m_bRedirectTest = false;
            break;
        }

        // Login
        if (WasButtonPressed (BID_LOGIN)) {

            pHttpForm = m_pHttpRequest->GetForm ("Switch");
            if (pHttpForm != NULL) {

                unsigned int iSwitch = pHttpForm->GetIntValue();

                bool bAuth;
                iErrCode = CheckAssociation(m_iEmpireKey, iSwitch, &bAuth);
                RETURN_ON_ERROR(iErrCode);

                if (!bAuth)
                {
                    AddMessage ("Access denied");
                    break;
                }
                else
                {
                    m_iReserved = 0;
                    m_i64SecretKey = 0;
                    m_vPassword = 0;
                    m_vEmpireName = 0;

                    m_iEmpireKey = iSwitch;

                    bool bLoggedIn;
                    iErrCode = HtmlLoginEmpire(&bLoggedIn);
                    RETURN_ON_ERROR(iErrCode);

                    if (!bLoggedIn)
                    {
                        return Redirect(LOGIN);
                    }

                    bool bInitialized;
                    iErrCode = InitializeEmpire(false, &bInitialized);
                    RETURN_ON_ERROR(iErrCode);

                    if (!bInitialized)
                    {
                        return Redirect(LOGIN);
                    }
                    else
                    {
                        return Redirect(ACTIVE_GAME_LIST);
                    }
                }
            }
        }

        // Send messages
        if (WasButtonPressed (BID_SENDMESSAGE)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL) {
                goto Redirection;
            }

            char pszBuffer [1024];
            const char* pszMessage = pHttpForm->GetValue();

            Variant vSentName;
            if (pszMessage != NULL)
            {
                iErrCode = SendSystemMessage (iTargetEmpireKey, pszMessage, m_iEmpireKey, 0);
                switch (iErrCode) {

                case OK:
                    iErrCode = GetEmpireName (iTargetEmpireKey, &vSentName);
                    RETURN_ON_ERROR(iErrCode);
                    sprintf(pszBuffer, "Your message was sent to %s", vSentName.GetCharPtr());
                    AddMessage (pszBuffer);
                    break;

                case ERROR_CANNOT_SEND_MESSAGE:
                    AddMessage ("You are not allowed to send system messages");
                    break;

                case ERROR_EMPIRE_DOES_NOT_EXIST:
                    AddMessage ("That empire no longer exists");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                }

            } else {

                AddMessage ("Your message was blank");
            }

            m_bRedirectTest = false;
            iProfilePage = 1;

            break;
        }

        }
        break;

    case 2:

        {

        // Redirection will handle it

        }
        break;

    case 3:

        {

        bool bFlag = false;

        const char* pszStart;

        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

            m_bRedirectTest = false;

            int iGameNumber;
            GameOptions goOptions;

            // Check for advanced
            char pszAdvanced [128];
            sprintf(pszAdvanced, "Advanced%i", iGameClassKey);

            if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                iProfilePage = 4;
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

        else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("DeleteGameClass")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "DeleteGameClass%d", &iGameClassKey) == 1) {

            unsigned int iOwnerKey;
            iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
            RETURN_ON_ERROR(iErrCode);

            if (m_iEmpireKey == iOwnerKey || (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))) {

                iErrCode = DeleteGameClass (iGameClassKey, &bFlag);
                RETURN_ON_ERROR(iErrCode);

                if (bFlag) {
                    AddMessage ("The GameClass was deleted");
                } else {
                    AddMessage ("The GameClass has been marked for deletion");
                }
            }
            
            iProfilePage = 3;
            m_bRedirectTest = false;
        }

        else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UndeleteGameClass")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "UndeleteGameClass%d", &iGameClassKey) == 1) {

            unsigned int iOwnerKey;
            iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
            RETURN_ON_ERROR(iErrCode);

            if (m_iEmpireKey == iOwnerKey || (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey())))
            {
                iErrCode = UndeleteGameClass (iGameClassKey);
                switch (iErrCode) {

                case OK:
                    AddMessage ("The GameClass was undeleted");
                    break;

                case ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION:
                    AddMessage ("The GameClass was not marked for deletion");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
            }

            iProfilePage = 3;
            m_bRedirectTest = false;
        }

        else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("HaltGameClass")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "HaltGameClass%d", &iGameClassKey) == 1) {

            unsigned int iOwnerKey;
            iErrCode = GetGameClassOwner(iGameClassKey, &iOwnerKey);
            RETURN_ON_ERROR(iErrCode);

            if (m_iEmpireKey == iOwnerKey || (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey())))
            {
                iErrCode = HaltGameClass (iGameClassKey);
                if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
                {
                    iErrCode = OK;
                    AddMessage ("The GameClass no longer exists");
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The GameClass was halted");
                }
            }

            iProfilePage = 3;
            m_bRedirectTest = false;
        }

        else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UnhaltGameClass")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "UnhaltGameClass%d", &iGameClassKey) == 1) {

            unsigned int iOwnerKey;
            iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
            RETURN_ON_ERROR(iErrCode);

            if (m_iEmpireKey == iOwnerKey || (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey())))
            {
                iErrCode = UnhaltGameClass (iGameClassKey);
                switch (iErrCode) {

                case OK:
                    AddMessage ("The GameClass was unhalted");
                    break;

                case ERROR_GAMECLASS_NOT_HALTED:
                    AddMessage ("The GameClass was not halted");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
            }

            iProfilePage = 3;
            m_bRedirectTest = false;
        }

        }
        break;

    case 4:

        // Check for choose
        if (WasButtonPressed (BID_START) || WasButtonPressed (BID_BLOCK)) {

            m_bRedirectTest = false;

            int iGameNumber;

            if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
                iProfilePage = 0;
                break;
            }
            iGameClassKey = pHttpForm->GetIntValue();

            GameOptions goOptions;
            InitGameOptions (&goOptions);

            iErrCode = ParseGameConfigurationForms (iGameClassKey, NO_KEY, NULL, &goOptions);
            if (iErrCode == WARNING)
            {
                iErrCode = OK;
                iProfilePage = 4;
                break;
            }
            RETURN_ON_ERROR(iErrCode);

            // Release game read lock
            goOptions.iNumEmpires = 1;
            goOptions.piEmpireKey = &m_iEmpireKey;

            // Create the game
            iErrCode = CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

            ClearGameOptions (&goOptions);

            HANDLE_CREATE_GAME_OUTPUT(iErrCode);
        }

        break;

    case 5:
        {
        const char* pszStart = NULL;

        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewTourneyInfo")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "ViewTourneyInfo%d", &m_iReserved) == 1) {

            // Release game read lock
            return Redirect (TOURNAMENTS);
        }

        }
        break;

    default:

        Assert(false);
        break;
    }

} else {

    if (m_iReserved != NO_KEY) {
        iTargetEmpireKey = m_iReserved;
        m_iReserved = NO_KEY;
    } else {

        pHttpForm = m_pHttpRequest->GetForm ("TargetEmpireKey");
        if (pHttpForm != NULL) {
            iTargetEmpireKey = pHttpForm->GetIntValue();
        } else {
            iTargetEmpireKey = m_iEmpireKey;
        }
    }
}

Redirection:
if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
    iErrCode = RedirectOnSubmitGame(&pageRedirect, &bRedirected);
    if (bRedirected)
    {
        return Redirect (pageRedirect);
    }
}

iErrCode = OpenGamePage();
RETURN_ON_ERROR(iErrCode);

// Individual page starts here

if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS)
{
    iErrCode = WriteRatiosString (NULL);
    RETURN_ON_ERROR(iErrCode);
}

switch (iProfilePage) {

case 1:
    {

    %><input type="hidden" name="ProfilePage" value="1"><%

    bool bExists;
    iErrCode = DoesEmpireExist (iTargetEmpireKey, &bExists, NULL);
    RETURN_ON_ERROR(iErrCode);

    if (!bExists) {
        %><p>That empire no longer exists<%
    } else {
        iErrCode = WriteProfile(m_iEmpireKey, iTargetEmpireKey, false, false, true);
        RETURN_ON_ERROR(iErrCode);
    }

    }
    break;

case 2:

    {

    %><input type="hidden" name="ProfilePage" value="2"><%
    %><input type="hidden" name="TargetEmpireKey" value="<% Write (iTargetEmpireKey); %>"><%

    Assert(iTargetEmpireKey != NO_KEY);
    iErrCode = WriteNukeHistory (iTargetEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    }
    break;

case 3:

    {

    %><input type="hidden" name="ProfilePage" value="3"><%
    %><input type="hidden" name="TargetEmpireKey" value="<% Write (iTargetEmpireKey); %>"><%

    iErrCode = WritePersonalGameClasses (iTargetEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    %><p><% WriteButton (BID_CANCEL);

    }
    break;

case 4:

    int iGameNumber;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = GetGameClassName (iGameClassKey, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNextGameNumber (iGameClassKey, &iGameNumber);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="ProfilePage" value="4"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%
    %><input type="hidden" name="TargetEmpireKey" value="<% Write (iTargetEmpireKey); %>"><%

    %><h3>Advanced game creation options:<p><%

    Write (pszGameClassName); %> <% Write (iGameNumber);

    %></h3><p><%

    iErrCode = RenderGameConfiguration (iGameClassKey, NO_KEY);
    RETURN_ON_ERROR(iErrCode);

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_START);

    break;

case 5:

    %><input type="hidden" name="ProfilePage" value="5"><%
    %><input type="hidden" name="TargetEmpireKey" value="<% Write (iTargetEmpireKey); %>"><%

    Assert(iTargetEmpireKey != NO_KEY);
    iErrCode = WritePersonalTournaments(iTargetEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    break;

default:

    Assert(false);
    break;
}

iErrCode = CloseGamePage();
RETURN_ON_ERROR(iErrCode);

%>