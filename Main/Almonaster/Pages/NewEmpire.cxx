<%

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

int iErrCode = OK;
unsigned int iEmpireKey = NO_KEY, iParentEmpireKey = NO_KEY;
bool bRepost = false;

// Make sure this is allowed

int iOptions;
iErrCode = GetSystemOptions (&iOptions);
RETURN_ON_ERROR(iErrCode);

if (!(iOptions & NEW_EMPIRES_ENABLED))
{
    String strMessage = "New empires cannot be created on this server at the moment";
    Variant vReason;

    iErrCode = GetSystemProperty(SystemData::NewEmpiresDisabledReason, &vReason);
    RETURN_ON_ERROR(iErrCode);

    const char* pszReason = vReason.GetCharPtr();
    if (!String::IsBlank(pszReason))
    {
        strMessage += ". ";
        strMessage += pszReason;
    }

    Assert(strMessage.GetCharPtr());
    AddMessage(strMessage);

    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

// Get keys
if ((pHttpForm = m_pHttpRequest->GetForm ("ButtonKey")) == NULL)
{
    return ERROR_MISSING_FORM;
}
m_iButtonKey = pHttpForm->GetUIntValue();

iErrCode = GetThemeAddress(m_iButtonKey, &m_iButtonAddress);
if (iErrCode == ERROR_THEME_DOES_NOT_EXIST)
{
    iErrCode = OK;
}
RETURN_ON_ERROR(iErrCode);

if ((pHttpForm = m_pHttpRequest->GetForm ("TextColor")) == NULL)
{
    return ERROR_MISSING_FORM;
}
m_vTextColor = pHttpForm->GetValue();
Assert(m_vTextColor.GetCharPtr());

if ((pHttpForm = m_pHttpRequest->GetForm ("GoodColor")) == NULL)
{
    return ERROR_MISSING_FORM;
}
m_vGoodColor = pHttpForm->GetValue();
Assert(m_vGoodColor.GetCharPtr());

if ((pHttpForm = m_pHttpRequest->GetForm ("BadColor")) == NULL)
{
    return ERROR_MISSING_FORM;
}
m_vBadColor = pHttpForm->GetValue();
Assert(m_vBadColor.GetCharPtr());

// Get submitted password
if ((pHttpForm = m_pHttpRequest->GetForm("Password")) == NULL)
{
    return ERROR_MISSING_FORM;
}
const char* pszSubmittedPassword = pHttpForm->GetValue();
if (!VerifyPassword(pszSubmittedPassword))
{
    AddMessage ("The password is invalid");
    return Redirect (LOGIN);
}

// Handle submissions from NewEmpire page
if (!m_bRedirection && (pHttpForm = m_pHttpRequest->GetForm ("PageId")) != NULL && pHttpForm->GetIntValue() == m_pgPageId)
{
    bRepost = true;
    char pszStandardizedName [MAX_EMPIRE_NAME_LENGTH + 1];

    // Check for cancel
    if (WasButtonPressed (BID_CANCEL))
    {
        return Redirect (LOGIN);
    }

    // Get empire name
    if ((pHttpForm = m_pHttpRequest->GetForm("EmpireName")) == NULL)
    {
        return ERROR_MISSING_FORM;
    }
    const char* pszName = pHttpForm->GetValue();
    if (String::IsBlank(pszName) || !VerifyEmpireName(pszName) || !StandardizeEmpireName(pszName, pszStandardizedName))
    {
        AddMessage ("The empire name is invalid");
        return Redirect (LOGIN);
    }
    m_vEmpireName = pszStandardizedName;
    Assert(m_vEmpireName.GetCharPtr());

    if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireProxy")) == NULL)
    {
        return ERROR_MISSING_FORM;
    }
    iEmpireKey = pHttpForm->GetUIntValue();

    if (m_iButtonKey == INDIVIDUAL_ELEMENTS)
    {
        Variant vValue;
        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIButtons, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_iButtonKey = vValue.GetInteger();

        iErrCode = GetThemeAddress(m_iButtonKey, &m_iButtonAddress);
        RETURN_ON_ERROR(iErrCode);
    }

    if (WasButtonPressed(BID_CREATEEMPIRE))
    {
        if ((pHttpForm = m_pHttpRequest->GetForm ("PasswordCopy")) == NULL)
        {
            return ERROR_MISSING_FORM;
        }

        if (String::StrCmp(pszSubmittedPassword, pHttpForm->GetValue()) != 0)
        {
            AddMessage ("Your password wasn't verified correctly");
        }
        else
        {
            bool bCreateEmpire = false;

            // Get parent empire's name and password
            if ((pHttpForm = m_pHttpRequest->GetForm ("ParentEmpireName")) == NULL)
            {
                return ERROR_MISSING_FORM;
            }

            // Make sure the name is valid (if it was submitted)
            const char* pszParentName = pHttpForm->GetValue();
            if (!pszParentName)
            {
                bCreateEmpire = true;
            }
            else
            {
                char pszStandardParentName[MAX_EMPIRE_NAME_LENGTH + 1];
                if (!VerifyEmpireName(pszParentName) || !StandardizeEmpireName(pszParentName, pszStandardParentName))
                {
                    AddMessage("The parent empire does not exist");
                }
                else
                {
                    if ((pHttpForm = m_pHttpRequest->GetForm ("ParentPassword")) == NULL)
                    {
                        return ERROR_MISSING_FORM;
                    }

                    if (!VerifyPassword(pHttpForm->GetValue()))
                    {
                        AddMessage("That was the wrong password for the parent empire");
                    }
                    else
                    {
                        iErrCode = LookupEmpireByName(pszStandardParentName, &iParentEmpireKey, NULL, NULL);
                        RETURN_ON_ERROR(iErrCode);

                        if (iParentEmpireKey == NO_KEY)
                        {
                            AddMessage("The parent empire ");
                            AppendMessage(pszStandardParentName);
                            AppendMessage(" does not exist");
                        }
                        else
                        {
                            iErrCode = IsPasswordCorrect(iParentEmpireKey, pHttpForm->GetValue());
                            if (iErrCode == ERROR_PASSWORD)
                            {
                                AddMessage("That was the wrong password for the parent empire");
                            }
                            else
                            {
                                RETURN_ON_ERROR(iErrCode);
                                bCreateEmpire = true;
                            }
                        }
                    }
                }
            }

            if (bCreateEmpire)
            {
                iErrCode = CreateEmpire(m_vEmpireName, pszSubmittedPassword, NOVICE, iParentEmpireKey, false, &iEmpireKey);
                switch (iErrCode)
                {
                case OK:
                    ReportEmpireCreation(m_vEmpireName);
                    SendWelcomeMessage(m_vEmpireName);

                    m_iEmpireKey = iEmpireKey;
                    m_iReserved = 0;

                    bool bLoggedIn;
                    iErrCode = HtmlLoginEmpire(&bLoggedIn);
                    RETURN_ON_ERROR(iErrCode);
                    if (bLoggedIn)
                    {
                        return Redirect(ACTIVE_GAME_LIST);
                    }

                    AddMessage ("Login failed");
                    return Redirect (LOGIN);

                case ERROR_DISABLED:
                    {
                    String strMessage = "The server is denying all new empire creation attempts at this time. ";
                    Variant vReason;

                    iErrCode = GetSystemProperty (SystemData::NewEmpiresDisabledReason, &vReason);
                    RETURN_ON_ERROR(iErrCode);

                    const char* pszReason = vReason.GetCharPtr();
                    if (pszReason == NULL || *pszReason == '\0') {
                        strMessage += "Please try back later.";
                    } else {
                        strMessage += pszReason;
                    }
                    AddMessage (strMessage);

                    }
                    break;

                case ERROR_COULD_NOT_DELETE_EMPIRE:
                    AddMessage ("The parent empire is in at least one active game and could not be deleted");
                    break;

                case ERROR_COULD_NOT_DELETE_ADMINISTRATOR:
                    AddMessage ("The parent empire is an administrator and cannot be inherited from");
                    break;

                case ERROR_RESERVED_EMPIRE_NAME:
                    AddMessage ("The given empire name is reserved");
                    break;

                case ERROR_EMPIRE_ALREADY_EXISTS:
                    AddMessage ("The given empire name is already in use");
                    break;

                case ERROR_EMPIRE_DOES_NOT_EXIST:
                    AddMessage ("The parent empire does not exist");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
            }
        }
    }
}

// Get graphics used by Login screen
if (m_bRedirection) {

    pHttpForm = m_pHttpRequest->GetForm ("EmpireKey");
    if (pHttpForm == NULL) {
        // AddMessage ("Missing EmpireKey form");
        return Redirect (LOGIN);
    }
    iEmpireKey = pHttpForm->GetUIntValue();

} else {

    pHttpForm = m_pHttpRequest->GetForm ("EmpireProxy");
    if (pHttpForm == NULL) {
        // AddMessage ("Missing EmpireProxy form");
        return Redirect (LOGIN);
    }
    iEmpireKey = pHttpForm->GetUIntValue();
}

pHttpForm = m_pHttpRequest->GetForm ("BackgroundKey");
if (pHttpForm == NULL) {
    AddMessage ("Missing BackgroundKey form");
    return Redirect (LOGIN);
}
m_iBackgroundKey = pHttpForm->GetUIntValue();

iErrCode = GetThemeAddress(m_iBackgroundKey, &m_iBackgroundAddress);
if (iErrCode == ERROR_THEME_DOES_NOT_EXIST)
{
    iErrCode = OK;
}
RETURN_ON_ERROR(iErrCode);

pHttpForm = m_pHttpRequest->GetForm ("SeparatorKey");
if (pHttpForm == NULL) {
    AddMessage ("Missing SeparatorKey form");
    return Redirect (LOGIN);
}
m_iSeparatorKey = pHttpForm->GetUIntValue();

%><html><%
%><head><%
%><title><%
iErrCode = WriteSystemTitleString();
RETURN_ON_ERROR(iErrCode);
%></title><%
%></head><%

WriteBodyString(-1);

%><center><h1>Create a New Empire</h1><%
%><p><%

int iSeparatorAddress;
iErrCode = GetThemeAddress(m_iSeparatorKey, &iSeparatorAddress);
RETURN_ON_ERROR(iErrCode);

WriteSeparatorString(m_iSeparatorKey, iSeparatorAddress);

if (!m_strMessage.IsBlank()) {
    %><p><strong><%
    Write (m_strMessage);
    %></strong><%
}

OpenForm();

// Re-POST password and graphics information to NewEmpire page in case we need to return 
%><input type="hidden" name="EmpireProxy" value="<% Write (iEmpireKey); %>"><%
%><input type="hidden" name="EmpireName" value="<% Write (m_vEmpireName); %>"><%
if (pszSubmittedPassword && strlen(pszSubmittedPassword) <= MAX_EMPIRE_PASSWORD_LENGTH)
{
    %><input type="hidden" name="Password" value="<% Write(pszSubmittedPassword); %>"><%
}
%><input type="hidden" name="ButtonKey" value="<% Write (m_iButtonKey); %>"><%
%><input type="hidden" name="BackgroundKey" value="<% Write (m_iBackgroundKey); %>"><%
%><input type="hidden" name="SeparatorKey" value="<% Write (m_iSeparatorKey); %>"><%
%><input type="hidden" name="TextColor" value="<% Write (m_vTextColor.GetCharPtr()); %>"><%
%><input type="hidden" name="GoodColor" value="<% Write (m_vGoodColor.GetCharPtr()); %>"><%
%><input type="hidden" name="BadColor" value="<% Write (m_vBadColor.GetCharPtr()); %>"><%


%><p><table width="75%"><tr><td colspan="2"><%

%>If you are under 13 years old, please do not continue playing on this server. <%
%>This game is not designed to comply with <%
%>US <a href="http://www.ftc.gov/bcp/conline/edcams/coppa/intro.htm">legislation</a> <%
%>concerning minors. In particular, the constraints imposed by this law on user interface and profile design <%
%>are excessively broad. Therefore, children under 13 are encouraged to find entertainment elsewhere.<%

%><p>If you are over 13 years old, please retype your password to confirm it:<%

%></td></tr><%
%></table><%

%><p><table width="50%"><%

%><tr><%
%><td><strong>Empire name</strong>:</td><%
%><td><strong><% Write (m_vEmpireName.GetCharPtr()); %></strong></td><%
%></tr><%

%><tr><td><strong>Password</strong>:</td><td><strong><% 

size_t i, stLength = strlen(pszSubmittedPassword);
for (i = 0; i < stLength; i ++) {
    %>*<%
}
%></strong></td></tr><%

%><tr><td><strong>Verify your password</strong>:</td><%
%><td><input type="password" name="PasswordCopy" size="20" maxlength="<% Write(MAX_EMPIRE_PASSWORD_LENGTH); %>"></td></tr></table><p><% 

WriteButton(BID_CANCEL);
WriteButton(BID_CREATEEMPIRE);

%><p><table width="75%"><%
%><tr><%
%><td>If you are an experienced player and you already have an empire registered on this server, you can <%
%>inherit that empire's privilege level, Almonaster score and significance with this empire. The empire that you inherit <%
%>from <strong>will be deleted</strong>. If you wish to do this, <%
%>type in the name and password of the empire you will be inheriting from:</strong></td><%
%></tr><%
%></table><%

%><table width="50%"><%
%><tr><td>&nbsp;</td></tr><%

%><tr><%
%><td><strong>Parent empire's name:</strong></td><%
%><td><input type="text" name="ParentEmpireName" size="20" maxlength="<% Write (MAX_EMPIRE_NAME_LENGTH); %>"<%

const char* pszParentName;

if (bRepost &&
    (pHttpForm = m_pHttpRequest->GetForm ("ParentEmpireName")) != NULL &&
    (pszParentName = pHttpForm->GetValue()) != NULL &&
    VerifyEmpireName (pszParentName, false))
{
    %> value="<% Write (pszParentName); %>"<%;
}
%>></td><%
%></tr><%

%><tr><td><strong>Parent empire's password:</strong></td><%
%><td><input type="password" name="ParentPassword" size="20" maxlength="<% Write(MAX_EMPIRE_PASSWORD_LENGTH); %>"></td><%

%></tr></table><%

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>