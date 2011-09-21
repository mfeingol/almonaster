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

IHttpForm* pSearchForm, * pHttpForm;

int iEmpireAdminPage = 0;
unsigned int iLastKey = NO_KEY, * piSearchEmpireKey = NULL, iNumSearchEmpires = 0, iTargetEmpireKey = NO_KEY;

// Make sure that the unprivileged don't abuse this:
if (m_iPrivilege < ADMINISTRATOR) {
    AddMessage ("You are not authorized to view this page");
    return Redirect (LOGIN);
}

RangeSearchColumnDefinition sc [MAX_NUM_SEARCH_COLUMNS];
RangeSearchDefinition sd;
sd.pscColumns = sc;

const char* pszFormName [MAX_NUM_SEARCH_COLUMNS];
const char* pszColName1 [MAX_NUM_SEARCH_COLUMNS];
const char* pszColName2 [MAX_NUM_SEARCH_COLUMNS];

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (!WasButtonPressed (BID_CANCEL)) {

        if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireAdminPage")) == NULL) {
            goto Redirection;
        }
        int iEmpireAdminPageSubmit = pHttpForm->GetIntValue();
        pHttpForm = NULL;

        switch (iEmpireAdminPageSubmit) {

        case 0:
            {

            // Broadcast
            if (WasButtonPressed (BID_SENDMESSAGE)) {

                const char* pszString;

                if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) != NULL && 
                    (pszString = pHttpForm->GetValue()) != NULL &&
                    *pszString != '\0') {

                    iErrCode = SendMessageToAll (m_iEmpireKey, pszString);

                    if (iErrCode == OK) {
                        AddMessage ("Your message was broadcast to all empires");
                    } else {
                        AddMessage ("Your message could not be broadcast to all empires");
                    }

                } else {
                    AddMessage ("Your message was blank");
                }

                m_bRedirectTest = false;

            } else {

                if (WasButtonPressed (BID_SEARCH)) {
SearchResults:
                    iEmpireAdminPage = 0;

                    iErrCode = HandleSearchSubmission (
                        sd,
                        pszFormName,
                        pszColName1,
                        pszColName2,
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
                                piSearchEmpireKey = NULL;
                                iEmpireAdminPage = 3;
                            } else {
                                iEmpireAdminPage = 1;
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
                bool bVerified;
                iErrCode = VerifyEmpireNameHash(iTargetEmpireKey, iHash, &bVerified);
                RETURN_ON_ERROR(iErrCode);

                if (!bVerified) {
                    AddMessage ("That empire no longer exists");
                } else {
                    iEmpireAdminPage = 3;
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

            if (WasButtonPressed (BID_CHANGEEMPIRESPASSWORD)) {

                // Change password
                if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpire")) == NULL) {
                    goto Redirection;
                }
                iTargetEmpireKey = pHttpForm->GetIntValue();

                if ((pHttpForm = m_pHttpRequest->GetForm ("NewPass")) == NULL || 
                    (pSearchForm = m_pHttpRequest->GetForm ("NewPass2")) == NULL) {
                    goto Redirection;
                }

                if (iTargetEmpireKey != m_iEmpireKey) {

                    if (iTargetEmpireKey == global.GetRootKey()) {
                        AddMessage ("You cannot update root's password");
                        return Redirect (m_pgPageId);
                    }

                    Variant vOldPassword;
                    int iPrivilege;
                    const char* pszValue = pHttpForm->GetValue(), * pszValue2 = pSearchForm->GetValue();

                    if (pszValue == NULL || *pszValue == '\0' ||
                        pszValue2 == NULL || *pszValue2 == '\0') {
                        AddMessage ("Blank passwords are not allowed");
                        return Redirect (m_pgPageId);
                    }

                    if (GetEmpirePassword (iTargetEmpireKey, &vOldPassword) != OK ||
                        GetEmpirePrivilege (iTargetEmpireKey, &iPrivilege) != OK) {
                        AddMessage ("The empire no longer exists");
                        return Redirect (m_pgPageId);
                    }

                    if (iPrivilege >= ADMINISTRATOR && m_iEmpireKey != global.GetRootKey()) {
                        AddMessage ("You cannot change an administrator's password");
                        return Redirect (m_pgPageId);
                    }

                    if (String::StrCmp (vOldPassword.GetCharPtr(), pszValue) == 0) {
                        AddMessage ("The new password was the same as the old one");
                    } else {
                        if (!VerifyPassword(pszValue)) {
                            return Redirect (m_pgPageId);
                        }

                        // Confirm
                        if (strcmp (pszValue, pszValue2) != 0) {
                            AddMessage ("The new password was not confirmed");
                            return Redirect (m_pgPageId);
                        }

                        iErrCode = SetEmpirePassword (iTargetEmpireKey, pszValue);
                        if (iErrCode == OK) {
                            AddMessage ("The empire's password was successfully changed");
                        } else {
                            AddMessage ("The empire's password could not be changed");
                            return Redirect (m_pgPageId);
                        }
                    }
                }
            }

            }
            break;

        case 3:

            {

            if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpire")) == NULL) {
                goto Redirection;
            }

            iTargetEmpireKey = pHttpForm->GetIntValue();

            // Change privilege, scores
            int iValue, iOldValue;
            float fValue, fOldValue;
            bool bValue, bOldValue;
            
            if ((pHttpForm = m_pHttpRequest->GetForm ("FixedPriv")) == NULL) {
                goto Redirection;
            }
            bValue = pHttpForm->GetIntValue() != 0;

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldFixedPriv")) == NULL) {
                goto Redirection;
            }
            bOldValue = pHttpForm->GetIntValue() != 0;
            
            if (bValue != bOldValue) {
            
                if (iTargetEmpireKey == m_iEmpireKey) {
                    AddMessage ("You cannot change your own privilege settings");
                }

                else if (iTargetEmpireKey == global.GetRootKey()) {
                    AddMessage ("You cannot change " ROOT_NAME "'s privilege settings");
                }

                else if (iTargetEmpireKey == global.GetGuestKey()) {
                    AddMessage ("You cannot change " GUEST_NAME "'s privilege settings");
                }

                else {

                    if (SetEmpireOption2 (iTargetEmpireKey, ADMINISTRATOR_FIXED_PRIVILEGE, bValue) == OK) {
                        AddMessage ("The empire's privilege settings were successfully updated");
                    } else {
                        AddMessage ("The empire's privilege settings could not be updated");
                    }
                }
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewPriv")) == NULL) {
                goto Redirection;
            }
            iValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldPriv")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iValue != iOldValue) {

                if (iTargetEmpireKey == m_iEmpireKey) {
                    AddMessage ("You cannot change your own privilege level");
                }

                else if (iTargetEmpireKey == global.GetRootKey()) {
                    AddMessage ("You cannot change " ROOT_NAME "'s privilege level");
                }

                else if (iTargetEmpireKey == global.GetGuestKey()) {
                    AddMessage ("You cannot change " GUEST_NAME "'s privilege level");
                }

                else if (!IS_VALID_PRIVILEGE (iValue)) {
                    AddMessage ("Invalid privilege");
                }

                else {

                    if (SetEmpirePrivilege (iTargetEmpireKey, iValue) == OK) {
                        AddMessage ("The empire's privilege level was successfully updated");
                    } else {
                        AddMessage ("The empire's privilege level could not be updated");
                    }
                }
            }

            // Score
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewAScore")) == NULL) {
                goto Redirection;
            }
            fValue = pHttpForm->GetFloatValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldAScore")) == NULL) {
                goto Redirection;
            }
            fOldValue = pHttpForm->GetFloatValue();

            if (fValue != fOldValue) {

                if (iTargetEmpireKey == m_iEmpireKey) {
                    AddMessage ("You cannot change your own Almonaster score");
                }

                else if (iTargetEmpireKey == global.GetRootKey()) {
                    AddMessage ("You cannot change " ROOT_NAME "'s Almonaster score");
                }

                else if (iTargetEmpireKey == global.GetGuestKey()) {
                    AddMessage ("You cannot change " GUEST_NAME "'s Almonaster score");
                }

                else {

                    if (SetEmpireAlmonasterScore (iTargetEmpireKey, fValue) == OK) {
                        AddMessage ("The empire's Almonaster score was successfully updated");
                    } else {
                        AddMessage ("The submitted Almonaster score was incorrect");
                    }
                }
            }

            // Significance
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewASignificance")) == NULL) {
                goto Redirection;
            }
            iValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldASignificance")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iValue != iOldValue) {

                if (iValue < 0) {
                    AddMessage ("The submitted Almonaster significance was invalid");
                }

                else if (iTargetEmpireKey == m_iEmpireKey) {
                    AddMessage ("You cannot change your own Almonaster significance");
                }

                else if (iTargetEmpireKey == global.GetRootKey()) {
                    AddMessage ("You cannot change " ROOT_NAME "'s Almonaster significance");
                }

                else if (iTargetEmpireKey == global.GetGuestKey()) {
                    AddMessage ("You cannot change " GUEST_NAME "'s Almonaster significance");
                }

                else {

                    EmpireCheck (SetEmpireProperty (iTargetEmpireKey, SystemEmpireData::AlmonasterScoreSignificance, iValue));
                    AddMessage ("The empire's Almonaster significance was successfully updated");
                }
            }

            // Broadcast
            if ((pHttpForm = m_pHttpRequest->GetForm ("Broadcast")) == NULL) {
                goto Redirection;
            }
            bValue = (pHttpForm->GetIntValue() != 0);

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldBroadcast")) == NULL) {
                goto Redirection;
            }
            bOldValue = (pHttpForm->GetIntValue() != 0);

            if (bValue != bOldValue) {

                if (iTargetEmpireKey == m_iEmpireKey) {
                    AddMessage ("You cannot change your own broadcast setting");
                }

                else if (iTargetEmpireKey == global.GetRootKey()) {
                    AddMessage ("You cannot change " ROOT_NAME "'s broadcast setting");
                }

                else if (iTargetEmpireKey == global.GetGuestKey()) {
                    AddMessage ("You cannot change " GUEST_NAME "'s broadcast setting");
                }

                else {

                    if (SetEmpireOption (iTargetEmpireKey, CAN_BROADCAST, bValue) == OK) {
                        if (bValue) {
                            AddMessage ("The empire can now broadcast messages");
                        } else {
                            AddMessage ("The empire can no longer broadcast messages");
                        }
                    } else {
                        AddMessage ("The empire does not exist");
                    }
                }
            }

            if (WasButtonPressed (BID_RESET)) {

                iErrCode = ResetEmpireSessionId (iTargetEmpireKey);
                if (iErrCode == OK) {
                    AddMessage ("The empire's session id will be reset on its next login");
                } else {
                    AddMessage ("The empire's session id could not be reset");
                }

                iEmpireAdminPage = 3;
                m_bRedirectTest = false;
            }

            // Lookup or Update
            if (WasButtonPressed (BID_LOOKUP) || WasButtonPressed (BID_UPDATE)) {
                m_bRedirectTest = false;
                iEmpireAdminPage = 3;
                break;
            }

            // Change password
            if (WasButtonPressed (BID_CHANGEEMPIRESPASSWORD) && iTargetEmpireKey != m_iEmpireKey) {
                iEmpireAdminPage = 2;
                m_bRedirectTest = false;
                break;
            }

            // Obliterate empire
            if (WasButtonPressed (BID_OBLITERATEEMPIRE)) {

                if (iTargetEmpireKey == m_iEmpireKey) {
                    AddMessage ("You cannot obliterate yourself");
                }

                else if (iTargetEmpireKey == global.GetRootKey()) {
                    AddMessage ("You cannot obliterate " ROOT_NAME);
                }

                else if (iTargetEmpireKey == global.GetGuestKey()) {
                    AddMessage ("You cannot obliterate " GUEST_NAME);
                }

                else {

                    iEmpireAdminPage = 4;
                }

                m_bRedirectTest = false;
                break;
            }

            }

            break;

        case 4:
            {

            // Obliterate empire
            if (WasButtonPressed (BID_OBLITERATEEMPIRE)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpire")) == NULL) {
                    goto Redirection;
                }
                iTargetEmpireKey = pHttpForm->GetIntValue();

                if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpireSecret")) == NULL) {
                    goto Redirection;
                }
                int64 i64TargetEmpireSecret = pHttpForm->GetInt64Value();

                if (iTargetEmpireKey == m_iEmpireKey) {
                    AddMessage ("You cannot obliterate yourself");
                }

                else if (iTargetEmpireKey == global.GetRootKey()) {
                    AddMessage ("You cannot obliterate " ROOT_NAME);
                }

                else if (iTargetEmpireKey == global.GetGuestKey()) {
                    AddMessage ("You cannot obliterate " GUEST_NAME);
                }

                else {

                    iErrCode = ObliterateEmpire (iTargetEmpireKey, i64TargetEmpireSecret, m_iEmpireKey);

                    if (iErrCode == ERROR_EMPIRE_DOES_NOT_EXIST) {
                        AddMessage ("The empire no longer exists");
                    } else {
                        if (iErrCode == OK) {
                            AddMessage ("The empire was successfully obliterated from the server");
                        } else {
                            char pszMessage [256];
                            sprintf(pszMessage, "Error %i occurred while obliterating the empire", iErrCode);
                            AddMessage (pszMessage);
                        }
                    }
                }

                m_bRedirectTest = false;
                break;
            }

            }
            break;

        default:

            Assert(false);
        }

    } else {
        m_bRedirectTest = false;
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

switch (iEmpireAdminPage) {

case 0:
    {

    %><input type="hidden" name="EmpireAdminPage" value="0"><%

    RenderSearchForms (true);

    %><p>Broadcast a message to all empires:<p><%
    %><textarea name="Message" rows="7" cols="60" wrap="virtual"></textarea><p><%

    WriteButton (BID_SENDMESSAGE);

    }

    break;

case 1:
    {

    %><input type="hidden" name="EmpireAdminPage" value="1"><%

    Assert(piSearchEmpireKey != NULL);

    RenderSearchResults (
        sd,
        pszFormName,
        pszColName1,
        pszColName2,
        piSearchEmpireKey,
        iNumSearchEmpires,
        iLastKey
        );

    t_pCache->FreeKeys (piSearchEmpireKey);

    }

    break;

case 2:

    {
    %><input type="hidden" name="EmpireAdminPage" value="2"><%
    %><input type="hidden" name="TargetEmpire" value="<% Write (iTargetEmpireKey); %>"><%

    Variant vTargetPassword, vTargetName;
    if (GetEmpirePassword (iTargetEmpireKey, &vTargetPassword) != OK ||
        GetEmpireName (iTargetEmpireKey, &vTargetName) != OK) {
        %><strong>The empire no longer exists</strong><%
    } else {

        %><p><strong>Change <% Write (vTargetName.GetCharPtr()); %>'s password:</strong><%
        %><p><table><tr><td><strong>New password</strong></td><td><%
        %><input type="password" name="NewPass" size="20" maxlength="<% Write (MAX_PASSWORD_LENGTH); 
            %>" value="<% Write (vTargetPassword.GetCharPtr()); %>"></td></tr><%
        %><tr><td><strong>Confirm password</strong></td><td><%
        %><input type="password" name="NewPass2" size="20" maxlength="<% Write (MAX_PASSWORD_LENGTH); 
            %>" value="<% Write (vTargetPassword.GetCharPtr()); %>"></td></tr></table><%
    }

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_CHANGEEMPIRESPASSWORD);

    }
    break;

case 3:
    {

    %><input type="hidden" name="EmpireAdminPage" value="3"><%
    %><input type="hidden" name="TargetEmpire" value="<% Write (iTargetEmpireKey); %>"><%

    Check(WriteProfile(m_iEmpireKey, iTargetEmpireKey, true, false, false));

    %><p><% 

    if (m_iEmpireKey != iTargetEmpireKey) {

        int iTargetPrivilege;
        iErrCode = GetEmpirePrivilege (iTargetEmpireKey, &iTargetPrivilege);

        if (iErrCode == OK && iTargetPrivilege < ADMINISTRATOR || m_iEmpireKey == global.GetRootKey()) {

            WriteButton (BID_CHANGEEMPIRESPASSWORD);

            if (iTargetEmpireKey != global.GetGuestKey() && iTargetEmpireKey != global.GetRootKey()) {
                WriteButton (BID_OBLITERATEEMPIRE);
            }
        }
    }

    %><p><%
    WriteButton (BID_CANCEL);
    WriteButton (BID_UPDATE);

    }

    break;

case 4:
    {

    Variant vEmpireName, vSecretKey;

    %><input type="hidden" name="EmpireAdminPage" value="4"><%
    %><input type="hidden" name="TargetEmpire" value="<% Write (iTargetEmpireKey); %>"><%

    %><p><%

    if (GetEmpireName (iTargetEmpireKey, &vEmpireName) != OK ||
        GetEmpireProperty (iTargetEmpireKey, SystemEmpireData::SecretKey, &vSecretKey) != OK) {
        %>That empire no longer exists<%
        %><p><% WriteButton (BID_CANCEL);
    } else {
        %><input type="hidden" name="TargetEmpireSecret" value="<% Write (vSecretKey.GetInteger64()); %>"><%
        %>Are you sure you want to obliterate the <% Write (vEmpireName.GetCharPtr()); %> empire?<%
        %><p><%
        WriteButton (BID_CANCEL);
        WriteButton (BID_OBLITERATEEMPIRE);
    }

    }
    break;

default:

    Assert(false);
    break;
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>