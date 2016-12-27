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

IHttpForm* pHttpForm;

int iPersonalGameClassesPage = 0;
unsigned int i, iGameClassKey = NO_KEY;

char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (WasButtonPressed (BID_CANCEL)) {
        m_bRedirectTest = false;
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
                m_bRedirectTest = false;

                if (m_iSystemOptions & CONFIRM_IMPORTANT_CHOICES) {
                    iPersonalGameClassesPage = 3;
                    goto Redirection;
                }

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                RETURN_ON_ERROR(iErrCode);

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                    ) {

                    iErrCode = DeleteGameClass (iGameClassKey, &bFlag);
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

                goto Redirection;
            }

            if (WasButtonPressed (BID_UNDELETEGAMECLASS) &&
                (pHttpForm = m_pHttpRequest->GetForm ("UndelGC")) != NULL) {

                iGameClassKey = pHttpForm->GetIntValue();

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                if (iErrCode == OK) {

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
                }

                m_bRedirectTest = false;
                goto Redirection;
            }

            if (WasButtonPressed (BID_HALTGAMECLASS) &&
                (pHttpForm = m_pHttpRequest->GetForm ("HaltGC")) != NULL) {

                iGameClassKey = pHttpForm->GetIntValue();

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
                goto Redirection;
            }

            if (WasButtonPressed (BID_UNHALTGAMECLASS) &&
                (pHttpForm = m_pHttpRequest->GetForm ("UnhaltGC")) != NULL) {

                iGameClassKey = pHttpForm->GetIntValue();

                unsigned int iOwnerKey;
                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                RETURN_ON_ERROR(iErrCode);

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
                        RETURN_ON_ERROR(iErrCode);
                        break;
                    }
                }

                m_bRedirectTest = false;
                goto Redirection;
            }

            // Handle game start
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

                m_bRedirectTest = false;

                // Check for advanced
                char pszAdvanced [64];
                sprintf(pszAdvanced, "Advanced%i", iGameClassKey);

                if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
                    iPersonalGameClassesPage = 1;
                    break;
                }

                GameOptions goOptions;
                iErrCode = GetDefaultGameOptions (iGameClassKey, &goOptions);
                if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
                {
                    AddMessage ("That gameclass no longer exists");
                    goto Redirection;
                }
                RETURN_ON_ERROR(iErrCode);

                goOptions.iNumEmpires = 1;
                goOptions.piEmpireKey = &m_iEmpireKey;

                // Create the game
                iErrCode = CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);
                HANDLE_CREATE_GAME_OUTPUT (iErrCode);
            }

            // Handle new gameclass creation
            if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && WasButtonPressed (BID_CREATENEWGAMECLASS)) {
                iPersonalGameClassesPage = 2;
                m_bRedirectTest = false;
                goto Redirection;
            }

        case 2:

            // Handle new gameclass creation
            if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES &&
                WasButtonPressed (BID_CREATENEWGAMECLASS)) {

                m_bRedirectTest = false;

                bool bProcessed;
                iErrCode = ProcessCreateGameClassForms(m_iEmpireKey, NO_KEY, &bProcessed);
                RETURN_ON_ERROR(iErrCode);
                if (!bProcessed)
                {
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

                iErrCode = GetGameClassOwner (iGameClassKey, &iOwnerKey);
                RETURN_ON_ERROR(iErrCode);

                if (m_iEmpireKey == iOwnerKey ||
                    (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != global.GetRootKey() || m_iEmpireKey == global.GetRootKey()))
                    ) {

                    iErrCode = DeleteGameClass (iGameClassKey, &bFlag);
                    if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
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

        case 1:

            // Check for choose
            if (WasButtonPressed (BID_START) || WasButtonPressed (BID_BLOCK)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
                    iPersonalGameClassesPage = 0;
                    break;
                }
                iGameClassKey = pHttpForm->GetIntValue();

                GameOptions goOptions;
                InitGameOptions (&goOptions);

                iErrCode = ParseGameConfigurationForms (iGameClassKey, NO_KEY, NULL, &goOptions);
                if (iErrCode == WARNING)
                {
                    iPersonalGameClassesPage = 1;
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

switch (iPersonalGameClassesPage) {

case 0:
    {
    unsigned int iNumGameClasses, * piGameClassKey = NULL;
    AutoFreeKeys free_piGameClassKey(piGameClassKey);

    int* piOptions;
    Variant* pvName = NULL, vMaxNumPGC;
    AutoFreeData free_pvName(pvName);

    unsigned int iNumHalted = 0, iNumUnhalted = 0, iNumMarkedForDeletion = 0;

    %><input type="hidden" name="PersonalGameClassesPage" value="0"><%

    if (m_iPrivilege < PRIVILEGE_FOR_PERSONAL_GAMECLASSES) {

        %><p><h3>You must be an <%
        Write (PRIVILEGE_STRING [PRIVILEGE_FOR_PERSONAL_GAMECLASSES]);
        %> to create personal GameClasses</h3><%

        float fScore;
        iErrCode = GetScoreForPrivilege (PRIVILEGE_FOR_PERSONAL_GAMECLASSES, &fScore);
        RETURN_ON_ERROR(iErrCode);

        %><p>(To reach <strong><%
        Write (PRIVILEGE_STRING [PRIVILEGE_FOR_PERSONAL_GAMECLASSES]);
        %></strong> status, you need an <em>Almonaster</em> score of <strong><%
        Write (fScore);
        %></strong>)<%
    }

    iErrCode = GetSystemProperty (SystemData::MaxNumPersonalGameClasses, &vMaxNumPGC);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpirePersonalGameClasses (m_iEmpireKey, &piGameClassKey, &pvName, &iNumGameClasses);
    RETURN_ON_ERROR(iErrCode);

    if (iNumGameClasses == 0)
    {
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

        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && iNumGameClasses >= (unsigned int)vMaxNumPGC.GetInteger()) {
            %> and can create no more<%
        }

        piOptions = (int*) StackAlloc (iNumGameClasses * sizeof (int));

        for (i = 0; i < iNumGameClasses; i ++) {

            iErrCode = GetGameClassOptions (piGameClassKey[i], piOptions + i);
            RETURN_ON_ERROR(iErrCode);

            if (piOptions[i] & GAMECLASS_HALTED) {
                iNumHalted ++;
            } else {
                iNumUnhalted ++;
            }

            if (piOptions[i] & GAMECLASS_MARKED_FOR_DELETION) {
                iNumMarkedForDeletion ++;
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

        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && iNumGameClasses < (unsigned int)vMaxNumPGC.GetInteger()) {
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

        WriteSystemGameListHeader (m_vTableColor.GetCharPtr());
        for (i = 0; i < iNumGameClasses; i ++)
        {
            Variant* pvGameClassInfo = NULL;
            AutoFreeData free_pvGameClassInfo(pvGameClassInfo);

            // Read game class data
            iErrCode = GetGameClassData (piGameClassKey[i], &pvGameClassInfo);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = WriteSystemGameListData(piGameClassKey[i], pvGameClassInfo);
            RETURN_ON_ERROR(iErrCode);
        }

        %></table><%
    }

    }
    break;

case 1:

    int iGameNumber;

    iErrCode = GetGameClassName (iGameClassKey, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNextGameNumber (iGameClassKey, &iGameNumber);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="PersonalGameClassesPage" value="1"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%

    %><h3>Advanced game creation options:<p><%

    Write (pszGameClassName); %> <% Write (iGameNumber);

    %></h3><p><%

    iErrCode = RenderGameConfiguration(iGameClassKey, NO_KEY);
    RETURN_ON_ERROR(iErrCode);

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_START);

    break;

case 2:

    %><input type="hidden" name="PersonalGameClassesPage" value="2"><%

    %><p><h3>Create a new GameClass:</h3><% 
    iErrCode = WriteCreateGameClassString (m_iEmpireKey, NO_KEY, false);
    RETURN_ON_ERROR(iErrCode);
    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_CREATENEWGAMECLASS);

    break;

case 3:

    %><input type="hidden" name="PersonalGameClassesPage" value="3"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%

    iErrCode = GetGameClassName (iGameClassKey, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    %><p>Are you sure you want to delete <strong><% Write (pszGameClassName); %></strong>?<%

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_DELETEGAMECLASS);

    break;

default:
    Assert(false);
    break;
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>