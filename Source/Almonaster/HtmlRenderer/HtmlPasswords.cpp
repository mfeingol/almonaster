//
// Almonaster.dll:  a component of Almonaster
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

#include "HtmlRenderer.h"

#include "Osal/Crypto.h"

//
// Authentication
//

int HtmlRenderer::HtmlLoginEmpire(bool* pbLoggedIn)
{
    //
    // If this function is called, it means we've been authenticated
    //

    *pbLoggedIn = false;

    int iErrCode = OK;
    Variant vValue;

    // Password and autologon logins start a new session. Switching to an associated empire counts as a
    // login for that empire, but it needs no password: it rides on the current session, so it keeps that
    // session's login time. Otherwise switching back and forth would keep a session alive past MAX_SESSION_LENGTH
    if (m_i64LoginTime == NO_LOGIN_TIME)
    {
        UTCTime tNow;
        Time::GetTime(&tNow);
        m_i64LoginTime = Time::GetUnixTime(tNow);
    }

    if (m_vEmpireName.GetType() != V_STRING || String::IsBlank(m_vEmpireName))
    {
        iErrCode = GetEmpireName (m_iEmpireKey, &m_vEmpireName);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // Get last login time
    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::LastLoginTime, &vValue);
    RETURN_ON_ERROR(iErrCode);
    UTCTime lastLoginTime = vValue.GetInteger64();

    // We're already authenticated, so register a login
    iErrCode = LoginEmpire(m_iEmpireKey, m_pHttpRequest->GetBrowserName(), m_pHttpRequest->GetClientIP());
    if (iErrCode == ERROR_DISABLED)
    {
        String strMessage = "The server is refusing logins at this time. ";
            
        Variant vReason;
        iErrCode = GetSystemProperty(SystemData::LoginsDisabledReason, &vReason);
        RETURN_ON_ERROR(iErrCode);

        const char* pszReason = NULL;
            
        if (vReason.GetType() == V_STRING) {
            pszReason = vReason.GetCharPtr();
        }

        if (pszReason == NULL || *pszReason == '\0') {
            strMessage += "Please try back later.";
        } else {
            strMessage += pszReason;
        }
        AddMessage (strMessage);
            
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);
    
    // Get theme key
    int iThemeKey;

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vValue);
    RETURN_ON_ERROR(iErrCode);
    iThemeKey = vValue.GetInteger();
        
    iErrCode = GetUIData (iThemeKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Privilege, &vValue);
    RETURN_ON_ERROR(iErrCode);
    m_iPrivilege = vValue.GetInteger();

    iErrCode = GetEmpireProperty(m_iEmpireKey, SystemEmpireData::AlienKey, &vValue);
    RETURN_ON_ERROR(iErrCode);
    m_iAlienKey = vValue.GetInteger();

    iErrCode = GetEmpireProperty(m_iEmpireKey, SystemEmpireData::AlienAddress, &vValue);
    RETURN_ON_ERROR(iErrCode);
    m_iAlienAddress = vValue.GetInteger();

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::IPAddress, &m_vPreviousIPAddress);
    RETURN_ON_ERROR(iErrCode);
        
    // Set a cookie for the last empire id used on the server
    char pszEmpireKey[128];
    String::UItoA(m_iEmpireKey, pszEmpireKey, 10);

    iErrCode = m_pHttpResponse->CreateCookie(LAST_EMPIRE_USED_COOKIE, pszEmpireKey, ONE_MONTH_IN_SECONDS, NULL);
    RETURN_ON_ERROR(iErrCode);

    AddMessage("Welcome back, ");
    AppendMessage(m_vEmpireName.GetCharPtr());

    if (Time::OlderThan(lastLoginTime, m_stServerNewsLastUpdate))
    {
        AddMessage("The Server News page was updated since your last login");
    }

    // Add to report
    ReportLoginSuccess(m_vEmpireName.GetCharPtr(), m_bAutoLogon);
    
    // Have a ticket
    m_bAuthenticated = true;

    *pbLoggedIn = true;
    return iErrCode;
}

int HtmlRenderer::InitializeEmpireInGame(bool bAutoLogon, bool* pbInitialized)
{
    int iErrCode = InitializeEmpire(bAutoLogon, pbInitialized);
    RETURN_ON_ERROR(iErrCode);

    // Guests can't access games
    if (*pbInitialized)
    {
        *pbInitialized = m_iPrivilege != GUEST;
    }
    return iErrCode;
}

int HtmlRenderer::InitializeEmpire(bool bAutoLogon, bool* pbInitialized)
{
    *pbInitialized = false;

    int iErrCode;
    IHttpForm* pHttpForm;
    bool bExists;
    
    if (bAutoLogon || ((pHttpForm = m_pHttpRequest->GetForm ("PageId")) != NULL && pHttpForm->GetIntValue() == m_pgPageId))
    {
        m_bOwnPost = true;

        if (!m_bAuthenticated)
        {
            if (m_iEmpireKey != NO_KEY)
            {
                // Make sure empire key exists
                iErrCode = DoesEmpireExist (m_iEmpireKey, &bExists, &m_vEmpireName);
                RETURN_ON_ERROR(iErrCode);
                if (!bExists)
                {
                    AddMessage("That empire no longer exists");
                    return OK;
                }
            }
            else
            {
                // Look for empire name form
                const char* pszName;
                if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireName")) == NULL || (pszName = pHttpForm->GetValue()) == NULL)
                {
                    AddMessage("Missing EmpireKey form");
                    return ERROR_MISSING_FORM;
                }
                
                iErrCode = LookupEmpireByName(pszName, &m_iEmpireKey, &m_vEmpireName, NULL);
                RETURN_ON_ERROR(iErrCode);

                if (m_iEmpireKey == NO_KEY)
                {
                    AddMessage("That empire doesn't exist");
                    return OK;
                }
            }
        }

        // Get empire options
        iErrCode = GetEmpireOptions (m_iEmpireKey, &m_iSystemOptions);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireOptions2 (m_iEmpireKey, &m_iSystemOptions2);
        RETURN_ON_ERROR(iErrCode);

        // Handle session id
        bool bUpdateSessionId, bUpdateCookie;
        iErrCode = InitializeSessionId(&bUpdateSessionId, &bUpdateCookie);
        RETURN_ON_ERROR(iErrCode);

        if (!m_bAuthenticated)
        {
            //
            // Need to authenticate
            //

            // Get empire's recorded IP address
            iErrCode = GetEmpireIPAddress(m_iEmpireKey, &m_vPreviousIPAddress);
            RETURN_ON_ERROR(iErrCode);

            // Try to read a page token
            if ((pHttpForm = m_pHttpRequest->GetForm("Token")) != NULL)
            {
                TokenStatus status;
                iErrCode = VerifyToken(PAGE_TOKEN, m_iEmpireKey, pHttpForm->GetValue(), &status, &m_i64LoginTime);
                RETURN_ON_ERROR(iErrCode);

                if (status == TOKEN_EXPIRED)
                {
                    // A valid autologon cookie for this empire stands in for a fresh autologon
                    bool bAutoLogonValid;
                    iErrCode = IsAutologonCookieValid(m_iEmpireKey, &bAutoLogonValid);
                    RETURN_ON_ERROR(iErrCode);

                    if (!bAutoLogonValid)
                    {
                        AddMessage("Your session has timed out. Please log in again");
                        return OK;
                    }

                    m_bAutoLogon = true;

                    bool bLoggedIn;
                    iErrCode = HtmlLoginEmpire(&bLoggedIn);
                    RETURN_ON_ERROR(iErrCode);
                    if (!bLoggedIn)
                    {
                        return OK;
                    }

                    // Refresh autologon cookies so they don't expire while in use
                    iErrCode = SetAutologonCookies(m_iEmpireKey);
                    RETURN_ON_ERROR(iErrCode);

                    // Don't act on the submission. The browser sends the autologon cookie automatically, so this request
                    // could have been forged by another site using an old token; only a live page token proves intent.
                    // Rendering the page as if it had been redirected to stops pages and RedirectOnSubmit[Game] from
                    // handling buttons, and clearing m_bOwnPost covers the Quit page, which also acts on redirects
                    m_bRedirection = true;
                    m_bOwnPost = false;
                    AddMessage("Your session timed out, and you were automatically logged in. Your submitted actions were discarded.");
                }
                else if (status != TOKEN_VALID)
                {
                    // The player didn't type a password, so don't say it was wrong. This also covers
                    // tokens invalidated by a password change
                    AddMessage("Your session is no longer valid. Please log in again");
                    return OK;
                }
            }
            else if ((pHttpForm = m_pHttpRequest->GetForm("ClearTextPassword")) != NULL)
            {
                // Try to use a cleartext password submitted as a form or query param
                const char* pszPassword = pHttpForm->GetValue();
                {
                    iErrCode = IsPasswordCorrect(m_iEmpireKey, pszPassword);
                    if (iErrCode == ERROR_PASSWORD)
                    {
                        char pszBuffer [256 + MAX_EMPIRE_NAME_LENGTH];
                        sprintf(pszBuffer, "That was the wrong password for the %s empire", m_vEmpireName.GetCharPtr());
                        AddMessage(pszBuffer);
                    }
                    RETURN_ON_ERROR(iErrCode);
                }

                // A password login starts a new session
                UTCTime tNow;
                Time::GetTime(&tNow);
                m_i64LoginTime = Time::GetUnixTime(tNow);
            }
            else
            {
                // No way to authenticate. This includes pages rendered before tokens existed
                AddMessage("Your session is no longer valid. Please log in again");
                return OK;
            }

            m_bAuthenticated = true;
        }
        
        // Update session id in database
        if (bUpdateSessionId)
        {
            // Write the empire's new session id
            iErrCode = SetEmpireSessionId(m_iEmpireKey, m_i64SessionId);
            RETURN_ON_ERROR(iErrCode);
        }
        
        // Update session id cookie
        if (bUpdateCookie)
        {
            // Best effort set a new cookie
            char pszSessionId[128];
            String::I64toA(m_i64SessionId, pszSessionId, 10);
            
            iErrCode = m_pHttpResponse->CreateCookie(SESSION_ID_COOKIE, pszSessionId, ONE_MONTH_IN_SECONDS, NULL);
            RETURN_ON_ERROR(iErrCode);
        }
        
        // Update IP address
        if (String::StrCmp(m_pHttpRequest->GetClientIP(), m_vPreviousIPAddress.GetCharPtr()) != 0)
        {
            iErrCode = SetEmpireIPAddress(m_iEmpireKey, m_pHttpRequest->GetClientIP());
            RETURN_ON_ERROR(iErrCode);
        }
        
        Variant vValue;
        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_iThemeKey = vValue.GetInteger();

        iErrCode = GetEmpirePrivilege (m_iEmpireKey, &m_iPrivilege);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlienKey, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_iAlienKey = vValue.GetInteger();

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlienAddress, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_iAlienAddress = vValue.GetInteger();

        iErrCode = GetUIData (m_iThemeKey);
        RETURN_ON_ERROR(iErrCode);
        
        if (!IsGamePage (m_pgPageId))
        {
            m_bRepeatedButtons = (m_iSystemOptions & SYSTEM_REPEATED_BUTTONS) != 0;
            m_bTimeDisplay = (m_iSystemOptions & SYSTEM_DISPLAY_TIME) != 0;
        }
    }
    else
    {
        bool bEmpireExists;
        iErrCode = DoesEmpireExist (m_iEmpireKey, &bEmpireExists, NULL);
        RETURN_ON_ERROR(iErrCode);
        if (!bEmpireExists) {
            AddMessage("That empire no longer exists");
            return OK;
        }
    }
    
    // Make sure access is allowed
    int iOptions;
    iErrCode = GetSystemOptions(&iOptions);
    RETURN_ON_ERROR(iErrCode);

    if (!(iOptions & ACCESS_ENABLED) && m_iPrivilege < ADMINISTRATOR) {

        // Get reason
        AddMessage ("Access is denied at this time. ");
        
        Variant vReason;
        iErrCode = GetSystemProperty (SystemData::AccessDisabledReason, &vReason);
        RETURN_ON_ERROR(iErrCode);
        
        AppendMessage (vReason.GetCharPtr());
        return OK;
    }
    
    // Add name to web server's log
    iErrCode = m_pHttpResponse->AddCustomLogMessage(m_vEmpireName.GetCharPtr());
    RETURN_ON_ERROR(iErrCode);

    *pbInitialized = true;
    return iErrCode;
}

//
// Tokens
//
// A token has the form <empire key>.<kind>.<login time>.<expiry>.<signature>. The signature is an HMAC keyed with the
// server's TokenSigningKey over the rest of the token, the empire's password hash and, for page tokens, the client
// attributes that the empire's logins are bound to. Covering the password hash means that changing an empire's password
// invalidates all of its tokens. Nothing is stored server-side
//
// Each page issues a fresh page token, so the expiry only limits idle time. The login time is carried forward from
// token to token, and page tokens stop being accepted MAX_SESSION_LENGTH after it
//

static int HashTokenField(Crypto::HmacSHA256& hmac, const void* pbData, size_t cbData)
{
    // Length-prefix each field so that bytes can't be shifted from one field to the next
    unsigned int cbField = (unsigned int)cbData;
    int iErrCode = hmac.HashData(&cbField, sizeof(cbField));
    RETURN_ON_ERROR(iErrCode);

    if (cbData > 0)
    {
        iErrCode = hmac.HashData(pbData, cbData);
        RETURN_ON_ERROR(iErrCode);
    }
    return iErrCode;
}

int HtmlRenderer::GetTokenSignature(TokenKind kind, unsigned int iEmpireKey, const char* pszPayload, String* pstrSignature)
{
    int iErrCode;
    Crypto::HmacSHA256 hmac;

    iErrCode = hmac.Initialize(global.GetTokenKey(), global.GetTokenKeySize());
    RETURN_ON_ERROR(iErrCode);

    // Empire key, kind and expiry
    iErrCode = HashTokenField(hmac, pszPayload, strlen(pszPayload));
    RETURN_ON_ERROR(iErrCode);

    // Password
    Variant vPasswordHash;
    iErrCode = GetEmpireDataColumn(iEmpireKey, SystemEmpireData::PasswordHash, &vPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    const char* pszPasswordHash = vPasswordHash.GetCharPtr();
    iErrCode = HashTokenField(hmac, pszPasswordHash, String::StrLen(pszPasswordHash));
    RETURN_ON_ERROR(iErrCode);

    if (kind == PAGE_TOKEN)
    {
        // Browser. Autologon tokens leave this out: it changes with every browser update, which would silently break autologon
        const char* pszBrowser = m_pHttpRequest->GetBrowserName();
        iErrCode = HashTokenField(hmac, pszBrowser, String::StrLen(pszBrowser));
        RETURN_ON_ERROR(iErrCode);

        if (IsGamePage(m_pgPageId))
        {
            // IP Address
            if (m_iSystemOptions & IP_ADDRESS_PASSWORD_HASHING)
            {
                const char* pszIPAddress = m_pHttpRequest->GetClientIP();
                iErrCode = HashTokenField(hmac, pszIPAddress, String::StrLen(pszIPAddress));
                RETURN_ON_ERROR(iErrCode);
            }

            // Session Id
            if (m_iSystemOptions & SESSION_ID_PASSWORD_HASHING)
            {
                iErrCode = HashTokenField(hmac, &m_i64SessionId, sizeof(m_i64SessionId));
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    // Done
    unsigned char pbMac[HMAC_SHA256_SIZE];
    iErrCode = hmac.GetHash(pbMac, sizeof(pbMac));
    RETURN_ON_ERROR(iErrCode);

    char pszBase64[64];
    Assert(Algorithm::GetEncodeBase64Size(sizeof(pbMac)) <= sizeof(pszBase64));
    iErrCode = Algorithm::EncodeBase64(pbMac, sizeof(pbMac), pszBase64, sizeof(pszBase64));
    RETURN_ON_ERROR(iErrCode);

    pstrSignature->Clear();
    *pstrSignature = pszBase64;
    if (pstrSignature->GetCharPtr() == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    return iErrCode;
}

int HtmlRenderer::FormatToken(TokenKind kind, unsigned int iEmpireKey, int64 i64LoginTime, int64 i64Expires, String* pstrToken)
{
    int iErrCode;

    char pszPayload[128];
    sprintf(pszPayload, "%u.%c.%lld.%lld", iEmpireKey, (char)kind, i64LoginTime, i64Expires);

    String strSignature;
    iErrCode = GetTokenSignature(kind, iEmpireKey, pszPayload, &strSignature);
    RETURN_ON_ERROR(iErrCode);

    pstrToken->Clear();
    *pstrToken = pszPayload;
    *pstrToken += ".";
    *pstrToken += strSignature;
    if (pstrToken->GetCharPtr() == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    return iErrCode;
}

int HtmlRenderer::CreateToken(TokenKind kind, unsigned int iEmpireKey, int64 i64LoginTime, Seconds sLifetime, String* pstrToken)
{
    if (i64LoginTime == NO_LOGIN_TIME)
    {
        // Don't silently start a new session
        Assert(false);
        return ERROR_FAILURE;
    }

    UTCTime tNow;
    Time::GetTime(&tNow);

    return FormatToken(kind, iEmpireKey, i64LoginTime, Time::GetUnixTime(tNow) + sLifetime, pstrToken);
}

int HtmlRenderer::VerifyToken(TokenKind kind, unsigned int iEmpireKey, const char* pszToken, TokenStatus* pStatus, int64* pi64LoginTime)
{
    int iErrCode = OK;
    *pStatus = TOKEN_INVALID;

    if (pszToken == NULL)
    {
        return iErrCode;
    }

    // Parse the payload. The token is re-created from these values and compared in full, so
    // anything that isn't exactly what the server would have issued fails to verify
    unsigned int iTokenEmpireKey;
    char cKind;
    int64 i64LoginTime, i64Expires;
    if (sscanf(pszToken, "%u.%c.%lld.%lld.", &iTokenEmpireKey, &cKind, &i64LoginTime, &i64Expires) != 4 ||
        iTokenEmpireKey != iEmpireKey ||
        cKind != (char)kind)
    {
        return iErrCode;
    }

    String strExpected;
    iErrCode = FormatToken(kind, iEmpireKey, i64LoginTime, i64Expires, &strExpected);
    RETURN_ON_ERROR(iErrCode);

    if (!Crypto::ConstantTimeEquals(pszToken, strExpected.GetCharPtr()))
    {
        return iErrCode;
    }

    // Only report expiry for tokens that the server actually issued
    UTCTime tNow;
    Time::GetTime(&tNow);
    int64 i64Now = Time::GetUnixTime(tNow);

    if (i64Now >= i64Expires)
    {
        *pStatus = TOKEN_EXPIRED;
        return iErrCode;
    }

    // Renewing page tokens can't keep a session alive forever
    if (kind == PAGE_TOKEN && i64Now - i64LoginTime >= MAX_SESSION_LENGTH)
    {
        *pStatus = TOKEN_EXPIRED;
        return iErrCode;
    }

    *pStatus = TOKEN_VALID;
    if (pi64LoginTime != NULL)
    {
        *pi64LoginTime = i64LoginTime;
    }
    return iErrCode;
}

int HtmlRenderer::SetAutologonCookies(unsigned int iEmpireKey)
{
    int iErrCode;

    char pszEmpireKey[128];
    String::UItoA(iEmpireKey, pszEmpireKey, 10);

    iErrCode = m_pHttpResponse->CreateCookie(AUTOLOGON_EMPIREKEY_COOKIE, pszEmpireKey, AUTOLOGON_COOKIE_TTL, NULL);
    RETURN_ON_ERROR(iErrCode);

    // Autologon tokens aren't part of a session: each autologon starts a new one
    UTCTime tNow;
    Time::GetTime(&tNow);

    String strToken;
    iErrCode = CreateToken(AUTOLOGON_TOKEN, iEmpireKey, Time::GetUnixTime(tNow), AUTOLOGON_COOKIE_TTL, &strToken);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = m_pHttpResponse->CreateCookie(AUTOLOGON_PASSWORD_COOKIE, strToken, AUTOLOGON_COOKIE_TTL, NULL);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Checks whether the request carries autologon cookies for the given empire, and that they hold a valid autologon token
int HtmlRenderer::IsAutologonCookieValid(unsigned int iEmpireKey, bool* pbValid)
{
    *pbValid = false;

    ICookie* pEmpireCookie = m_pHttpRequest->GetCookie(AUTOLOGON_EMPIREKEY_COOKIE);
    ICookie* pTokenCookie = m_pHttpRequest->GetCookie(AUTOLOGON_PASSWORD_COOKIE);

    if (pEmpireCookie == NULL || pEmpireCookie->GetValue() == NULL || pTokenCookie == NULL ||
        pEmpireCookie->GetUIntValue() != iEmpireKey)
    {
        return OK;
    }

    TokenStatus status;
    int iErrCode = VerifyToken(AUTOLOGON_TOKEN, iEmpireKey, pTokenCookie->GetValue(), &status, NULL);
    RETURN_ON_ERROR(iErrCode);

    *pbValid = status == TOKEN_VALID;
    return iErrCode;
}
