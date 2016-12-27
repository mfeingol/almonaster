<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

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

IHttpForm* pHttpForm;

int i, iErrCode, iPersonalGameClassesPage = 0;
unsigned int iGameClassKey = NO_KEY;

char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (WasButtonPressed (BID_CANCEL)) {
        bRedirectTest = false;
    } else {

        int iPersonalGameClassesPageSubmit;

        if ((pHttpForm = m_pHttpRequest->GetForm ("PersonalGameClassesPage")) == NULL) {
            goto Redirection;
        }
        iPersonalGameClassesPageSubmit = pHttpForm->GetIntValue();

        int iGameNumber;
        bool bFlag = false;

        const char* pszStart;

        switch (iPersonalGameClassesPageSubmit) {

        case 0:

            if (WasButtonPressed (BID_DELETEGAMECLASS) &&
                (pHttpForm = m_pHttpRequest->GetForm ("DelGC")) != NULL) {

                iGameClassKey = pHttpForm->GetIntValue();
                bRedirectTest = false;

                if (m_iSystemOptions & CONFIRM_IMPORTANT_CHOICES) {
                    iPersonalGameClassesPage = 3;
                    goto Redirection;
                }

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
                            AddMessage ("The gameclass could not be deleted; the error was ");
                            AppendMessage (iErrCode);
                        }
                    }
                }

                goto Redirection;
            }

            if (WasButtonPressed (BID_UNDELETEGAMECLASS) &&
                (pHttpForm = m_pHttpRequest->GetForm ("UndelGC")) != NULL) {

                iGameClassKey = pHttpForm->GetIntValue();

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

                            AddMessage ("The gameclass could not be undeleted; the error was ");
                            AppendMessage (iErrCode);
                            break;
                        }
                    }
                }

                bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_HALTGAMECLASS) &&
                (pHttpForm = m_pHttpRequest->GetForm ("HaltGC")) != NULL) {

                iGameClassKey = pHttpForm->GetIntValue();

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
                            AddMessage ("The gameclass could not be halted; the error was ");
                            AppendMessage (iErrCode);
                        }
                    }
                }

                bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_UNHALTGAMECLASS) &&
                (pHttpForm = m_pHttpRequest->GetForm ("UnhaltGC")) != NULL) {

                iGameClassKey = pHttpForm->GetIntValue();

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

                            AddMessage ("The gameclass could not be unhalted; the error was ");
                            AppendMessage (iErrCode);
                            break;
                        }
                    }
                }

                bRedirectTest = false;
                goto Redirection;
            }

            // Handle game start
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

                bRedirectTest = false;

                // Check for advanced
                char pszAdvanced [64];
                sprintf (pszAdvanced, "Advanced%i", iGameClassKey);

                if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                    iPersonalGameClassesPage = 1;
                    break;
                }

                GameOptions goOptions;
                iErrCode = g_pGameEngine->GetDefaultGameOptions (iGameClassKey, &goOptions);
                if (iErrCode != OK) {

                    if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
                        AddMessage ("That gameclass no longer exists");
                    } else {
                        AddMessage ("Could not read default game options; the error was ");
                        AppendMessage (iErrCode);
                    }
                    goto Redirection;
                }

                goOptions.iNumEmpires = 1;
                goOptions.piEmpireKey = &m_iEmpireKey;

                // Create the game
                iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

                HANDLE_CREATE_GAME_OUTPUT (iErrCode);
            }

            // Handle new gameclass creation
            if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && WasButtonPressed (BID_CREATENEWGAMECLASS)) {
                iPersonalGameClassesPage = 2;
                bRedirectTest = false;
                goto Redirection;
            }

        case 2:

            // Handle new gameclass creation
            if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES &&
                WasButtonPressed (BID_CREATENEWGAMECLASS)) {

                bRedirectTest = false;
                if (ProcessCreateGameClassForms (m_iEmpireKey, NO_KEY) != OK) {
                    iPersonalGameClassesPage = 2;
                }
                goto Redirection;
            }

            break;

        case 3:

            unsigned int iOwnerKey;

            pHttpForm = m_pHttpRequest->GetForm ("GameClassKey");
            if (pHttpForm != NULL && WasButtonPressed (BID_DELETEGAMECLASS)) {

                iGameClassKey = pHttpForm->GetUIntValue();

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
                            AddMessage ("The gameclass could not be deleted; the error was ");
                            AppendMessage (iErrCode);
                        }
                    }
                }
            }

        case 1:

            // Check for choose
            if (WasButtonPressed (BID_START) || WasButtonPressed (BID_BLOCK)) {

                bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
                    iPersonalGameClassesPage = 0;
                    break;
                }
                iGameClassKey = pHttpForm->GetIntValue();

                GameOptions goOptions;
                InitGameOptions (&goOptions);

                iErrCode = ParseGameConfigurationForms (iGameClassKey, NO_KEY, NULL, &goOptions);
                if (iErrCode != OK) {
                    iPersonalGameClassesPage = 1;
                    break;
                }

                goOptions.iNumEmpires = 1;
                goOptions.piEmpireKey = &m_iEmpireKey;

                // Create the game
                iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

                ClearGameOptions (&goOptions);

                HANDLE_CREATE_GAME_OUTPUT (iErrCode);
            }

            break;

        default:

            Assert (false);
            break;
        }
    }
}

SYSTEM_REDIRECT_ON_SUBMIT

SYSTEM_OPEN (false)

switch (iPersonalGameClassesPage) {

case 0:

    {

    int iNumGameClasses, * piGameClassKey = NULL, * piOptions;
    Variant* pvName = NULL, vMaxNumPGC;

    unsigned int iNumHalted = 0, iNumUnhalted = 0, iNumMarkedForDeletion = 0;

    %><input type="hidden" name="PersonalGameClassesPage" value="0"><%

    if (m_iPrivilege < PRIVILEGE_FOR_PERSONAL_GAMECLASSES) {

        %><p><h3>You must be an <%
        Write (PRIVILEGE_STRING [PRIVILEGE_FOR_PERSONAL_GAMECLASSES]);
        %> to create personal GameClasses</h3><%

        float fScore;
        if (g_pGameEngine->GetScoreForPrivilege (PRIVILEGE_FOR_PERSONAL_GAMECLASSES, &fScore) == OK) {

            %><p>(To reach <strong><%
            Write (PRIVILEGE_STRING [PRIVILEGE_FOR_PERSONAL_GAMECLASSES]);
            %></strong> status, you need an <em>Almonaster</em> score of <strong><%
            Write (fScore);
            %></strong>)<%
        }
    }

    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxNumPersonalGameClasses, &vMaxNumPGC));
    Check (g_pGameEngine->GetEmpirePersonalGameClasses (m_iEmpireKey, &piGameClassKey, &pvName, &iNumGameClasses));

    if (iNumGameClasses == 0) {

        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES) {

            %><p>You have <strong>0</strong> personal GameClasses<%

            if (vMaxNumPGC.GetInteger() > 0) {
                %><p><table width="75%"><%
                %><tr><%
                %><td>Create a new GameClass:</td><%
                %><td><% WriteButton (BID_CREATENEWGAMECLASS); %></td><%
                %></tr><%
                %></table><%
            }
        }

    } else {

        %><p>You have <strong><% Write (iNumGameClasses); %></strong> personal GameClass<%
        if (iNumGameClasses != 1) {
            %>es<%
        }

        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && iNumGameClasses >= vMaxNumPGC.GetInteger()) {
            %> and can create no more<%
        }

        piOptions = (int*) StackAlloc (iNumGameClasses * sizeof (int));

        for (i = 0; i < iNumGameClasses; i ++) {

            iErrCode = g_pGameEngine->GetGameClassOptions (piGameClassKey[i], piOptions + i);
            if (iErrCode != OK) {
                piOptions[i] = 0;
            } else {

                if (piOptions[i] & GAMECLASS_HALTED) {
                    iNumHalted ++;
                } else {
                    iNumUnhalted ++;
                }

                if (piOptions[i] & GAMECLASS_MARKED_FOR_DELETION) {
                    iNumMarkedForDeletion ++;
                }
            }
        }

        %><p><table><%

        // Delete
        if (iNumGameClasses - iNumMarkedForDeletion > 0) {

            %><tr><%
            %><td>Delete a personal GameClass</td><%
            %><td><select name="DelGC"><% 
            for (i = 0; i < iNumGameClasses; i ++) {

                if (!(piOptions[i] & GAMECLASS_MARKED_FOR_DELETION)) {

                    %><option value="<% Write (piGameClassKey[i]); %>"><%
                    Write (pvName[i].GetCharPtr());
                    %></option><%
                }
            }
            %></select><%
            %></td><%
            %><td><%
            WriteButton (BID_DELETEGAMECLASS);
            %></td><%
            %></tr><%
        }

        // Undelete
        if (iNumMarkedForDeletion > 0) {

            %><tr><%
            %><td>Undelete a personal GameClass</td><%
            %><td><select name="UndelGC"><% 
            for (i = 0; i < iNumGameClasses; i ++) {

                if (piOptions[i] & GAMECLASS_MARKED_FOR_DELETION) {

                    %><option value="<% Write (piGameClassKey[i]); %>"><%
                    Write (pvName[i].GetCharPtr());
                    %></option><%
                }
            }
            %></select><%
            %></td><%
            %><td><%
            WriteButton (BID_UNDELETEGAMECLASS);
            %></td><%
            %></tr><%
        }

        // Halt
        if (iNumUnhalted > 0) {

            %><tr><%
            %><td>Halt a personal GameClass</td><%
            %><td><select name="HaltGC"><% 
            for (i = 0; i < iNumGameClasses; i ++) {

                if (!(piOptions[i] & GAMECLASS_HALTED)) {

                    %><option value="<% Write (piGameClassKey[i]); %>"><%
                    Write (pvName[i].GetCharPtr());
                    %></option><%
                }
            }
            %></select><%
            %></td><%
            %><td><%
            WriteButton (BID_HALTGAMECLASS);
            %></td><%
            %></tr><%
        }

        // Halt
        if (iNumHalted > 0) {

            %><tr><%
            %><td>Unhalt a personal GameClass</td><%
            %><td><select name="UnhaltGC"><% 
            for (i = 0; i < iNumGameClasses; i ++) {

                if (piOptions[i] & GAMECLASS_HALTED) {

                    %><option value="<% Write (piGameClassKey[i]); %>"><%
                    Write (pvName[i].GetCharPtr());
                    %></option><%
                }
            }
            %></select><%
            %></td><%
            %><td><%
            WriteButton (BID_UNHALTGAMECLASS);
            %></td><%
            %></tr><%
        }

        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && iNumGameClasses < vMaxNumPGC.GetInteger()) {
            %><tr><%
            %><td>Create a new GameClass</td><%
            %><td></td><%
            %><td><% WriteButton (BID_CREATENEWGAMECLASS); %></td><%
            %></tr><%
        }

        %></table><%

        //
        // Start games
        //

        %><p><h3>Start a new game:</h3><%

        Variant* pvGameClassInfo = NULL;

        WriteSystemGameListHeader (m_vTableColor.GetCharPtr());
        for (i = 0; i < iNumGameClasses; i ++) {

            // Read game class data
            if (g_pGameEngine->GetGameClassData (piGameClassKey[i], &pvGameClassInfo) == OK) {

                // Best effort
                iErrCode = WriteSystemGameListData (
                    piGameClassKey[i], 
                    pvGameClassInfo
                    );
            }

            if (pvGameClassInfo != NULL) {
                g_pGameEngine->FreeData (pvGameClassInfo);
                pvGameClassInfo = NULL;
            }
        }

        %></table><%

        g_pGameEngine->FreeKeys (piGameClassKey);
        g_pGameEngine->FreeData (pvName);
    }

    }
    break;

case 1:

    int iGameNumber;

    Check (g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName));
    Check (g_pGameEngine->GetNextGameNumber (iGameClassKey, &iGameNumber));

    %><input type="hidden" name="PersonalGameClassesPage" value="1"><%
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

    %><input type="hidden" name="PersonalGameClassesPage" value="2"><%

    %><p><h3>Create a new GameClass:</h3><% 
    WriteCreateGameClassString (m_iEmpireKey, NO_KEY, false);
    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_CREATENEWGAMECLASS);

    break;

case 3:

    %><input type="hidden" name="PersonalGameClassesPage" value="3"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%

    Check (g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName));

    %><p>Are you sure you want to delete <strong><% Write (pszGameClassName); %></strong>?<%

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_DELETEGAMECLASS);

    break;

default:

    Assert (false);
    break;
}

SYSTEM_CLOSE

%>