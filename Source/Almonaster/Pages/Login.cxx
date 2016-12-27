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

// Get objects
IHttpForm* pHttpForm;

const char* pszPrintEmpireName = NULL;
char pszStandardizedName [MAX_EMPIRE_NAME_LENGTH + 1];

int iErrCode = OK;

m_iEmpireKey = NO_KEY;
m_vEmpireName = (const char*) NULL;

// Check for submission
if (m_pHttpRequest->GetMethod() == GET)
{
    // Look for auto-login cookie
    ICookie* pAutoLogonEmpire, * pPasswordCookie;
    unsigned int iAutoLogonKey = NO_KEY;

    pAutoLogonEmpire = m_pHttpRequest->GetCookie(AUTOLOGON_EMPIREKEY_COOKIE);
    pPasswordCookie = m_pHttpRequest->GetCookie(AUTOLOGON_PASSWORD_COOKIE);

    if (pAutoLogonEmpire && pAutoLogonEmpire->GetValue() && pPasswordCookie)
    {
        iAutoLogonKey = pAutoLogonEmpire->GetUIntValue();
        if (iAutoLogonKey != NO_KEY)
        {
            Variant vValue;
            unsigned int iResults;
            iErrCode = CacheEmpire(iAutoLogonKey, &iResults);
            RETURN_ON_ERROR(iErrCode);

            if (iResults > 0)
            {
                // Authenticate
                String strActualHash;
                iErrCode = GetAutologonPasswordHash(iAutoLogonKey, &strActualHash);
                RETURN_ON_ERROR(iErrCode);
            
                if (String::StrCmp(strActualHash, pPasswordCookie->GetValue()) == 0)
                {
                    m_iEmpireKey = iAutoLogonKey;
                    m_bAutoLogon = true;

                    bool bLoggedIn;
                    iErrCode = HtmlLoginEmpire(&bLoggedIn);
                    RETURN_ON_ERROR(iErrCode);
                    if (bLoggedIn)
                    {
                        bool bInitialized;
                        iErrCode = InitializeEmpire(true, &bInitialized);
                        RETURN_ON_ERROR(iErrCode);
                        if (bInitialized)
                        {
                            // Yay!
                            return Redirect(ACTIVE_GAME_LIST);
                        }
                    }
                }
            }
        }

        // Autologon failed
        m_iEmpireKey = NO_KEY;
        AddMessage("Autologon failed and was disabled");

        // Best effort delete invalid cookies
        if (pAutoLogonEmpire)
        {
            m_pHttpResponse->DeleteCookie(AUTOLOGON_EMPIREKEY_COOKIE, NULL);
        }

        if (pPasswordCookie)
        {
            m_pHttpResponse->DeleteCookie(AUTOLOGON_PASSWORD_COOKIE, NULL);
        }
    }
}

else if (!m_bRedirection)
{
    const char* pszEmpireName, * pszPassword;

    // Get empire name, validate it
    pHttpForm = m_pHttpRequest->GetForm("EmpireName");
    if (pHttpForm == NULL)
    {
        goto Text;
    }
    pszEmpireName = pHttpForm->GetValue();
    if (!VerifyEmpireName (pszEmpireName) || !StandardizeEmpireName (pszEmpireName, pszStandardizedName))
    {
        goto Text;
    }
    pszPrintEmpireName = pszStandardizedName;

    // Get password form, validate it
    pHttpForm = m_pHttpRequest->GetForm("Password");
    if (pHttpForm == NULL)
    {
        goto Text;
    }
    pszPassword = pHttpForm->GetValue();
    if (!VerifyPassword(pszPassword))
    {
        goto Text;
    }

    // See if the empire exists
    iErrCode = LookupEmpireByName(pszStandardizedName, &m_iEmpireKey, NULL, NULL);
    RETURN_ON_ERROR(iErrCode);

    // Check if they clicked the CreateEmpire button
    if (m_pHttpRequest->GetFormBeginsWith("CreateEmpire"))
    {
        if (m_iEmpireKey != NO_KEY)
        {
            AddMessage ("The ");
            AppendMessage (pszPrintEmpireName);
            AppendMessage (" empire already exists");
            goto Text;
        }

        // A new empire, so redirect to NewEmpire
        m_vEmpireName = pszStandardizedName;
        return Redirect(NEW_EMPIRE);
    }

    // Check if they clicked the Login button or hit return
    if (m_pHttpRequest->GetFormBeginsWith("BLogin") || m_pHttpRequest->GetFormBeginsWith("TransDot"))
    {
        if (m_iEmpireKey == NO_KEY)
        {
            // A new empire, so redirect to NewEmpire
            m_vEmpireName = pszStandardizedName;
            Assert(m_vEmpireName.GetCharPtr());
            return Redirect(NEW_EMPIRE);
        }

        // Check password
        iErrCode = IsPasswordCorrect(m_iEmpireKey, pszPassword);
        if (iErrCode == ERROR_PASSWORD)
        {
            char pszBuffer [128 + MAX_EMPIRE_NAME_LENGTH];
            sprintf(pszBuffer, "That was not the right password for the %s empire", pszEmpireName);
            AddMessage(pszBuffer);
            
            ReportLoginFailure(m_vEmpireName.GetCharPtr());
            goto Text;
        }
        RETURN_ON_ERROR(iErrCode);

         // Make sure access is allowed
        int iOptions;
        iErrCode = GetSystemOptions(&iOptions);
        RETURN_ON_ERROR(iErrCode);

        bool bLoggedIn;
        iErrCode = HtmlLoginEmpire(&bLoggedIn);
        RETURN_ON_ERROR(iErrCode);
        if (bLoggedIn)
        {
            bool bInitialized;
            iErrCode = InitializeEmpire(false, &bInitialized);
            RETURN_ON_ERROR(iErrCode);
            if (bInitialized)
            {
                // Yay!
                return Redirect(ACTIVE_GAME_LIST);
            }
            m_vEmpireName = (const char*) NULL;
        }
    }
}

Text:

// Get a cookie for last empire used's graphics
ICookie* pCookie = m_pHttpRequest->GetCookie(LAST_EMPIRE_USED_COOKIE);
if (pCookie != NULL && pCookie->GetValue() != NULL)
{
    m_iEmpireKey = pCookie->GetIntValue();

    unsigned int iResults;
    iErrCode = CacheEmpire(m_iEmpireKey, &iResults);
    RETURN_ON_ERROR(iErrCode);

    if (iResults == 0)
    {
        m_iEmpireKey = NO_KEY;
    }
    else if (pszPrintEmpireName == NULL)
    {
        pszPrintEmpireName = m_vEmpireName.GetCharPtr();
    }
}

if (m_iEmpireKey == NO_KEY)
{
    unsigned int iLivePlanetKey, iDeadPlanetKey, iHorz, iVert, iColor;

    iErrCode = GetDefaultUIKeys (
        &m_iBackgroundKey,
        &m_iBackgroundAddress,
        &iLivePlanetKey,
        &iDeadPlanetKey,
        &m_iButtonKey,
        &m_iButtonAddress,
        &m_iSeparatorKey,
        &iHorz,
        &iVert,
        &iColor
        );
    RETURN_ON_ERROR(iErrCode);

    if (iColor == NULL_THEME)
    {
        m_vTextColor = DEFAULT_TEXT_COLOR;
        m_vGoodColor = DEFAULT_GOOD_COLOR;
        m_vBadColor = DEFAULT_BAD_COLOR;

    } else {

        iErrCode = GetThemeTextColor (iColor, &m_vTextColor);
        RETURN_ON_ERROR(iErrCode);
        m_vTextColor = DEFAULT_TEXT_COLOR;
        
        iErrCode = GetThemeGoodColor (iColor, &m_vGoodColor);
        RETURN_ON_ERROR(iErrCode);
        m_vGoodColor = DEFAULT_GOOD_COLOR;
        
        iErrCode = GetThemeBadColor (iColor, &m_vBadColor);
        RETURN_ON_ERROR(iErrCode);
        m_vBadColor = DEFAULT_BAD_COLOR;
    }
}
else
{
    Variant vValue;
    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vValue);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetUIData(vValue.GetInteger());
    RETURN_ON_ERROR(iErrCode);
}

int iOptions;
iErrCode = GetSystemOptions(&iOptions);
RETURN_ON_ERROR(iErrCode);

%><html><%
%><head><%
%><title><%
iErrCode = WriteSystemTitleString();
RETURN_ON_ERROR(iErrCode);
%></title><%
%></head><%

WriteBodyString(-1);
OpenForm();

// POST graphics information to NewEmpire page
%><input type="hidden" name="ButtonKey" value="<% Write (m_iButtonKey); %>"><%
%><input type="hidden" name="BackgroundKey" value="<% Write (m_iBackgroundKey); %>"><%
%><input type="hidden" name="SeparatorKey" value="<% Write (m_iSeparatorKey); %>"><%
%><input type="hidden" name="EmpireKey" value="<% Write (m_iEmpireKey); %>"><%
%><input type="hidden" name="TextColor" value="<% Write (m_vTextColor.GetCharPtr()); %>"><%
%><input type="hidden" name="GoodColor" value="<% Write (m_vGoodColor.GetCharPtr()); %>"><%
%><input type="hidden" name="BadColor" value="<% Write (m_vBadColor.GetCharPtr()); %>"><%

%><center><%

%><table align="center" width="90%" cellpadding="0" cellspacing="0"><%
%><tr><%
%><td width="42%"><%

WriteIntro();

%></td><td width="58%" align="center"><%

WriteAlmonasterBanner();

%><p><% WriteIntroUpper (false);

if (!m_strMessage.IsBlank()) {
    %><p><strong><%
    Write (m_strMessage.GetCharPtr(), m_strMessage.GetLength());
    %></strong><%
}

if (!(iOptions & LOGINS_ENABLED)) {

    %><p><strong>The server is denying all logins at this time. <%

    Variant vReason;
    const char* pszReason = NULL;

    iErrCode = GetSystemProperty (SystemData::LoginsDisabledReason, &vReason);
    RETURN_ON_ERROR(iErrCode);
    pszReason = vReason.GetCharPtr();

    if (String::IsBlank (pszReason)) {
        %>Please try back later.<%
    } else {
        Write (pszReason);
    }
    %></strong><%

} else {

    %><p><table align="center"><tr><%

    %><td align="right"><strong>Empire Name:</strong></td><%
    %><td><%
    %><input type="text" size="20" tabindex="32767" maxlength="<% Write (MAX_EMPIRE_NAME_LENGTH); %>" name="EmpireName"<% 

    if (pszPrintEmpireName)
    {
        %> value="<% Write(pszPrintEmpireName); %>"<%
    }
    %>></td><%

    %></tr><tr><%

    %><td align="right"><strong>Password:</strong></td><%
    %><td><%
    %><input type="password" size="20" tabindex="32767" maxlength="<% Write(MAX_EMPIRE_PASSWORD_LENGTH); %>" name="Password"><%
    %></td><%

    %></tr><%
    %></table><%
    %><p><%

    %><input border="0" type="image" src="<% Write (BASE_RESOURCE_DIR TRANSPARENT_DOT); %>" name="TransDot"><%

    if (iOptions & NEW_EMPIRES_ENABLED) {
        WriteButton (BID_CREATEEMPIRE); %> <%
    } else {
        %>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<%
    }
    WriteButton (BID_LOGIN);
}

%><p><% WriteIntroLower (false);

iErrCode = WriteContactLine();
RETURN_ON_ERROR(iErrCode);

%></td><%
%></tr></table><%

%></center></form></body></html>