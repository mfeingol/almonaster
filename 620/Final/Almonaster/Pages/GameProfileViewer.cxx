<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

// Almonaster 2.0
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

INITIALIZE_GAME

IHttpForm* pHttpForm;

int iErrCode, iProfilePage = 1, iTargetEmpireKey = NO_KEY, iGameClassKey = NO_KEY;

bool bGameStarted = (m_iGameState & STARTED) != 0;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    // Get target empire
    pHttpForm = m_pHttpRequest->GetForm ("TargetEmpireKey");
    if (pHttpForm == NULL) {
        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
        m_pgeLock = NULL;
        AddMessage ("Missing TargetEmpireKey form");
        return Redirect (INFO);
    }
    iTargetEmpireKey = pHttpForm->GetIntValue();

    // Handle submissions
    if ((pHttpForm = m_pHttpRequest->GetForm ("ProfilePage")) == NULL) {
        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
        m_pgeLock = NULL;
        return Redirect (LOGIN);
    }

    int iProfilePageSubmit = pHttpForm->GetIntValue();

    switch (iProfilePageSubmit) {

    case 1:
        {

        // Check for Nuke history
        if (WasButtonPressed (BID_VIEWEMPIRESNUKEHISTORY)) {

            iProfilePage = 2;
            bRedirectTest = false;
            break;
        }

        // Check for personal game classes
        if (WasButtonPressed (BID_VIEWEMPIRESGAMECLASSES)) {

            iProfilePage = 3;
            bRedirectTest = false;
            break;
        }

        // Check for personal tournaments
        if (WasButtonPressed (BID_VIEWEMPIRESTOURNAMENTS)) {

            iProfilePage = 5;
            bRedirectTest = false;
            break;
        }

        // Lookup
        if (WasButtonPressed (BID_LOOKUP)) {
            iProfilePage = 1;
            bRedirectTest = false;
            break;
        }

        // Send messages
        if (WasButtonPressed (BID_SENDMESSAGE)) {

            if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL) {
                goto Redirection;
            }

            char pszBuffer [1024];
            const char* pszMessage = pHttpForm->GetValue();

            Variant vSentName;
            if (pszMessage != NULL) {

                iErrCode = g_pGameEngine->SendSystemMessage (iTargetEmpireKey, pszMessage, m_iEmpireKey);
                switch (iErrCode) {

                case OK:

                    iErrCode = g_pGameEngine->GetEmpireName (iTargetEmpireKey, &vSentName);
                    if (iErrCode == OK) {
                        sprintf (pszBuffer, "Your message was sent to %s", vSentName.GetCharPtr());
                        AddMessage (pszBuffer);
                    } else {
                        AddMessage ("That empire no longer exists");
                    }
                    break;

                case ERROR_CANNOT_SEND_MESSAGE:

                    AddMessage ("You are not allowed to send system messages");
                    break;

                case ERROR_EMPIRE_DOES_NOT_EXIST:

                    AddMessage ("That empire no longer exists");
                    break;

                default:
                    sprintf (pszBuffer, "Your message could not be sent due to error %i", iErrCode);
                    AddMessage (pszBuffer);
                    break;
                }

            } else {

                AddMessage ("Your message was blank");
            }

            bRedirectTest = false;
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

            bRedirectTest = false;

            int iGameNumber;
            GameOptions goOptions;

            // Check for advanced
            char pszAdvanced [128];
            sprintf (pszAdvanced, "Advanced%i", iGameClassKey);

            if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                iProfilePage = 4;
                break;
            }

            iErrCode = g_pGameEngine->GetDefaultGameOptions (iGameClassKey, &goOptions);
            if (iErrCode != OK) {
                AddMessage ("Could not read default game options");
                goto Redirection;
            }

            // Release game read lock
            g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
            m_pgeLock = NULL;

            goOptions.iNumEmpires = 1;
            goOptions.piEmpireKey = &m_iEmpireKey;

            // Create the game
            iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

            HANDLE_CREATE_GAME_OUTPUT (iErrCode);
        }

        else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("DeleteGameClass")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "DeleteGameClass%d", &iGameClassKey) == 1) {

            unsigned int iOwnerKey;
            iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
            if (iErrCode == OK) {

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
                    ) {

                    iErrCode = g_pGameEngine->DeleteGameClass (iGameClassKey, &bFlag);

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
                        char pszBuffer [256];
                        sprintf (pszBuffer, "Error %i occurred deleting the gameclass", iErrCode);
                        AddMessage (pszBuffer);
                    }
                }
            }

            iProfilePage = 3;
            bRedirectTest = false;
        }

        else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UndeleteGameClass")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "UndeleteGameClass%d", &iGameClassKey) == 1) {

            unsigned int iOwnerKey;
            iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
            if (iErrCode == OK) {

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
                    ) {

                    iErrCode = g_pGameEngine->UndeleteGameClass (iGameClassKey);
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
                        char pszBuffer [256];
                        sprintf (pszBuffer, "The gameclass could not be undeleted; the error was %i", iErrCode);
                        AddMessage (pszBuffer);
                        }
                        break;
                    }
                }
            }

            iProfilePage = 3;
            bRedirectTest = false;
        }

        else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("HaltGameClass")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "HaltGameClass%d", &iGameClassKey) == 1) {

            unsigned int iOwnerKey;
            iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
            if (iErrCode == OK) {

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
                    ) {

                    iErrCode = g_pGameEngine->HaltGameClass (iGameClassKey);

                    if (iErrCode == OK) {
                        AddMessage ("The GameClass was halted");
                    }
                    else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
                        AddMessage ("The GameClass no longer exists");
                    }
                    else {
                        char pszBuffer [256];
                        sprintf (pszBuffer, "Error %i occurred halting the gameclass", iErrCode);
                        AddMessage (pszBuffer);
                    }
                }
            }

            iProfilePage = 3;
            bRedirectTest = false;
        }

        else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UnhaltGameClass")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "UnhaltGameClass%d", &iGameClassKey) == 1) {

            unsigned int iOwnerKey;
            iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
            if (iErrCode == OK) {

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
                    ) {

                    iErrCode = g_pGameEngine->UnhaltGameClass (iGameClassKey);
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

                        {
                        char pszBuffer [256];
                        sprintf (pszBuffer, "The gameclass could not be unhalted; the error was %i", iErrCode);
                        AddMessage (pszBuffer);
                        }

                        break;
                    }
                }
            }

            iProfilePage = 3;
            bRedirectTest = false;
        }

        }
        break;

    case 4:

        // Check for choose
        if (WasButtonPressed (BID_STARTGAME) || WasButtonPressed (BID_BLOCK)) {

            bRedirectTest = false;

            int iGameNumber;

            if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
                iProfilePage = 0;
                break;
            }
            iGameClassKey = pHttpForm->GetIntValue();

            GameOptions goOptions;
            InitGameOptions (&goOptions);

            iErrCode = ParseGameConfigurationForms (iGameClassKey, NO_KEY, NULL, m_iEmpireKey, &goOptions);
            if (iErrCode != OK) {
                iProfilePage = 4;
                break;
            }

            // Release game read lock
            g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
            m_pgeLock = NULL;

            goOptions.iNumEmpires = 1;
            goOptions.piEmpireKey = &m_iEmpireKey;

            // Create the game
            iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

            ClearGameOptions (&goOptions);

            HANDLE_CREATE_GAME_OUTPUT (iErrCode);
        }

        break;

    case 5:
        {
        const char* pszStart = NULL;

        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewTourneyInfo")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "ViewTourneyInfo%d", &m_iReserved) == 1) {

            // Release game read lock
            g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
            m_pgeLock = NULL;
            return Redirect (TOURNAMENTS);
        }

        }
        break;

    default:

        Assert (false);
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

GAME_REDIRECT_ON_SUBMIT

GAME_OPEN

// Individual page starts here

if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
    GameCheck (WriteRatiosString (NULL));
}

switch (iProfilePage) {

case 1:
    {

    %><input type="hidden" name="ProfilePage" value="1"><%

    bool bExists;
    iErrCode = g_pGameEngine->DoesEmpireExist (iTargetEmpireKey, &bExists, NULL);
    if (iErrCode != OK || !bExists) {
        %><p>That empire no longer exists<%
    } else {
        WriteProfile (iTargetEmpireKey, false, false, true); 
    }

    }
    break;

case 2:

    {

    %><input type="hidden" name="ProfilePage" value="2"><%
    %><input type="hidden" name="TargetEmpireKey" value="<% Write (iTargetEmpireKey); %>"><%

    Assert (iTargetEmpireKey != NO_KEY);
    WriteNukeHistory (iTargetEmpireKey);

    }
    break;

case 3:

    {

    %><input type="hidden" name="ProfilePage" value="3"><%
    %><input type="hidden" name="TargetEmpireKey" value="<% Write (iTargetEmpireKey); %>"><%

    WritePersonalGameClasses (iTargetEmpireKey);

    %><p><% WriteButton (BID_CANCEL);

    }
    break;

case 4:

    int iGameNumber;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    Check (g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName));
    Check (g_pGameEngine->GetNextGameNumber (iGameClassKey, &iGameNumber));

    %><input type="hidden" name="ProfilePage" value="4"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%
    %><input type="hidden" name="TargetEmpireKey" value="<% Write (iTargetEmpireKey); %>"><%

    %><h3>Advanced game creation options:<p><%

    Write (pszGameClassName); %> <% Write (iGameNumber);

    %></h3><p><%

    RenderGameConfiguration (iGameClassKey, NO_KEY);

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_STARTGAME);

    break;

case 5:

    %><input type="hidden" name="ProfilePage" value="5"><%
    %><input type="hidden" name="TargetEmpireKey" value="<% Write (iTargetEmpireKey); %>"><%

    Assert (iTargetEmpireKey != NO_KEY);
    WritePersonalTournaments (iTargetEmpireKey);

    break;

default:

    Assert (false);
    break;
}

GAME_CLOSE

%>