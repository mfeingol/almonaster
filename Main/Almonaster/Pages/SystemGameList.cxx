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

if (InitializeEmpire(false) != OK)
{
    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

int iSystemGameListPage = 0, iGameClassKey = NO_KEY;
const char* pszPassword = NULL;

// Handle a submission
unsigned int i, j;
int iErrCode;

if (m_bOwnPost && !m_bRedirection) {

    int iSystemGameListPageSubmit;
    const char* pszStart;

    GameOptions goOptions;
    InitGameOptions (&goOptions);

    if ((pHttpForm = m_pHttpRequest->GetForm ("SystemGameListPage")) == NULL) {
        goto Redirection;
    }
    iSystemGameListPageSubmit = pHttpForm->GetIntValue();

    switch (iSystemGameListPageSubmit) {

    case 0:

        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMES && WasButtonPressed (BID_STARTCUSTOMGAME)) {
            iSystemGameListPage = 2;
            m_bRedirectTest = false;
            break;
        }

        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
            (pszStart = pHttpForm->GetName()) != NULL &&
            sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

            m_bRedirectTest = false;

            // Check for advanced option
            char pszAdvanced [128];
            sprintf(pszAdvanced, "Advanced%i", iGameClassKey);

            if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                iSystemGameListPage = 1;
                break;
            }

            iErrCode = GetDefaultGameOptions (iGameClassKey, &goOptions);
            if (iErrCode != OK) {
                AddMessage ("Could not read default game options");
                goto Redirection;
            }

            goto CreateGame;
        }

        break;

    case 1:

        // Check for choose
        if (WasButtonPressed (BID_START) || WasButtonPressed (BID_BLOCK)) {

            m_bRedirectTest = false;

            if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
                iSystemGameListPage = 0;
                break;
            }

            iGameClassKey = pHttpForm->GetIntValue();

            iErrCode = ParseGameConfigurationForms (iGameClassKey, NO_KEY, NULL, &goOptions);
            if (iErrCode != OK) {
                ClearGameOptions (&goOptions);
                iSystemGameListPage = 1;
                break;
            }

        CreateGame:
            {

            // Test for entry into gameclass
            int iGameNumber;

            // Check password
            if (!String::IsBlank (pszPassword) && VerifyPassword (pszPassword) != OK) {
                ClearGameOptions (&goOptions);
                AddMessage ("Your password contained an invalid character");
                goto Redirection;
            }

            goOptions.iNumEmpires = 1;
            goOptions.piEmpireKey = &m_iEmpireKey;

            // Create the game
            iErrCode = CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

            ClearGameOptions (&goOptions);

            HANDLE_CREATE_GAME_OUTPUT (iErrCode);
            }

        }

        break;

    case 2:
        {

        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMES &&
            (WasButtonPressed (BID_START) || WasButtonPressed (BID_BLOCK))) {

            int iGameClassKey, iGameNumber;
            bool bGameCreated;

            iErrCode = ProcessCreateDynamicGameClassForms (
                m_iEmpireKey,
                &iGameClassKey,
                &iGameNumber,
                &bGameCreated
                );

            if (bGameCreated) {
                HANDLE_CREATE_GAME_OUTPUT (iErrCode);
            } else {
                iSystemGameListPage = 2;
            }
        }

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
    Check(RedirectOnSubmit(&pageRedirect, &bRedirected));
    if (bRedirected)
    {
        return Redirect(pageRedirect);
    }
}

Check(OpenSystemPage(false));

// Individual page stuff starts here
switch (iSystemGameListPage) {

case 0:
    {

    %><input type="hidden" name="SystemGameListPage" value="0"><%

    unsigned int iNumGameClasses, * piGameClassKey = NULL;
    Check (GetStartableSystemGameClassKeys (&piGameClassKey, &iNumGameClasses));

    Algorithm::AutoDelete<unsigned int> autoDelete (piGameClassKey);

    if (iNumGameClasses == 0) {
        %><h3>There are no system game classes on this server</h3><%
    } else {

        // Get superclass list
        unsigned int* piSuperClassKey, iNumSuperClasses = 0;
        Check (GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses));

        bool bDraw = false;
        int** ppiTable = NULL;

        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMES) {

            %><p><h3>Start a personal game:</h3><%
            %><table width="90%" cellspacing="1" cellpadding="2"><%
            %><tr><th colspan="2" align="center" bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>"><%
            %><font size="3">Personal game</font></th><%

            %></tr><tr><td width="20%"><font size="3">Personal game</font><p><%
            WriteButton (BID_STARTCUSTOMGAME);
            %></td><%

            %><td>Click on the Start Game button to create a personal game with the specs that you desire. <%
            %>You will be presented with a menu that allows you to define the characteristics of the new <%
            %>game. Please read the documentation and familiarize yourself with the different Almonaster features <%
            %>before you create a personal game.<%

            %></td></tr></table><%
        }

        if (iNumSuperClasses > 0) {

            // Build superclass-gameclass table
            ppiTable = (int**) StackAlloc (iNumSuperClasses * sizeof (int*));
            for (i = 0; i < iNumSuperClasses; i ++) {
                ppiTable[i] = (int*) StackAlloc ((iNumGameClasses + 1) * sizeof (int));
                ppiTable [i][iNumGameClasses] = 0;
            }

            for (i = 0; i < iNumGameClasses; i ++)
            {
                unsigned int iSuperClassKey;
                iErrCode = GetGameClassSuperClassKey (piGameClassKey[i], &iSuperClassKey);
                if (iErrCode == OK) {

                    for (j = 0; j < iNumSuperClasses; j ++) {
                        if (piSuperClassKey[j] == iSuperClassKey) {
                            ppiTable [j][ppiTable[j][iNumGameClasses]] = piGameClassKey[i];
                            ppiTable [j][iNumGameClasses] ++;
                            bDraw = true;
                            break;
                        }
                    }

                    if (j == iNumSuperClasses) {
                        // No superclass was found, so something went wrong
                        AddMessage ("Error: GameClass ");
                        AppendMessage (piGameClassKey[i]);
                        AppendMessage (" does not have a SuperClass");
                        Redirect (m_pgPageId);
                    }
                }
            }
        }

        if (!bDraw) {
            %><h3>There are no system game classes available to your empire</h3><%
        } else {

            if (m_iPrivilege < PRIVILEGE_FOR_PERSONAL_GAMES) {
                %><h3>Start a new game:</h3><%
            }

            Variant* pvGameClassInfo = NULL, vName;
            int iGameClass;

            for (i = 0; i < iNumSuperClasses; i ++) {

                if (ppiTable [i][iNumGameClasses] > 0 &&
                    GetSuperClassName (piSuperClassKey[i], &vName) == OK) {

                    %><p><h3><% Write (vName.GetCharPtr()); %>:</h3><%

                    WriteSystemGameListHeader (m_vTableColor.GetCharPtr());

                    for (j = 0; j < (unsigned int)ppiTable[i][iNumGameClasses]; j ++) {

                        iGameClass = ppiTable[i][j];

                        // Read game class data
                        if (GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

                            // Best effort
                            iErrCode = WriteSystemGameListData (
                                iGameClass,
                                pvGameClassInfo
                                );
                        }
                        if (pvGameClassInfo != NULL) {
                            t_pCache->FreeData (pvGameClassInfo);
                            pvGameClassInfo = NULL;
                        }
                    }
                    %></table><%
                } 
            }
        }

        // Clean up
        if (iNumSuperClasses > 0) {
            t_pCache->FreeKeys (piSuperClassKey);
        }
    }

    }
    break;

case 1:

    int iGameNumber;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    Check (GetGameClassName (iGameClassKey, pszGameClassName));
    Check (GetNextGameNumber (iGameClassKey, &iGameNumber));

    %><input type="hidden" name="SystemGameListPage" value="1"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%

    %><h3>Advanced game creation options:<p><%

    Write (pszGameClassName); %> <% Write (iGameNumber);

    %></h3><p><%

    RenderGameConfiguration (iGameClassKey, NO_KEY);

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_START);

    break;

case 2:
    {

    %><input type="hidden" name="SystemGameListPage" value="2"><%

    %><h3>Personal Game Configuration</h3><%

    %><p><table width="60%"><tr><td align="center"><%
    %>Choose the characteristics of your new personal game:<%
    %></td></tr></table><%

    WriteCreateGameClassString (m_iEmpireKey, NO_KEY, true);

    %><p><%
    %><h3>Advanced Game Configuration</h3><%

    RenderGameConfiguration (NO_KEY, NO_KEY);

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_START);

    }
    break;

default:

    Assert(false);
    break;
}

CloseSystemPage();;

%>