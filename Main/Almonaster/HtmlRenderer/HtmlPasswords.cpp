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
        iErrCode = InitializeSessionId (&bUpdateSessionId, &bUpdateCookie);
        RETURN_ON_ERROR(iErrCode);

        if (!m_bAuthenticated)
        {
            //
            // Need to authenticate
            //

            // Get empire's recorded IP address
            iErrCode = GetEmpireIPAddress(m_iEmpireKey, &m_vPreviousIPAddress);
            RETURN_ON_ERROR(iErrCode);

            // Try to read a hashed password
            if ((pHttpForm = m_pHttpRequest->GetForm("Password")) != NULL)
            {
                const char* pszSubmittedPasswordHash = pHttpForm->GetValue();

                // Read salt
                if ((pHttpForm = m_pHttpRequest->GetForm ("Salt")) == NULL)
                {
                    return ERROR_MISSING_FORM;
                }
                UTCTime tOldSalt = pHttpForm->GetUInt64Value();

                // Make sure page hasn't timed out
                UTCTime tNow;
                Time::GetTime(&tNow);

                if (Time::GetSecondDifference(tNow, tOldSalt) >= DAY_LENGTH_IN_SECONDS)
                {
                    AddMessage("Your session has timed out. Please log in again");
                    return OK;
                }

                // Authenticate
                String strActualPasswordHash;
                iErrCode = GetPagePasswordHash(m_pgPageId, m_iEmpireKey, tOldSalt, &strActualPasswordHash);
                RETURN_ON_ERROR(iErrCode);

                if (String::StrCmp(pszSubmittedPasswordHash, strActualPasswordHash) != 0)
                {
                    char pszBuffer [256 + MAX_EMPIRE_NAME_LENGTH];
                    sprintf(pszBuffer, "That was the wrong password for the %s empire", m_vEmpireName.GetCharPtr());
                    
                    AddMessage (pszBuffer);
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
            }
            else
            {
                // No way to authenticate
                char pszBuffer [256 + MAX_EMPIRE_NAME_LENGTH];
                sprintf(pszBuffer, "That was the wrong password for the %s empire", m_vEmpireName.GetCharPtr());
                AddMessage(pszBuffer);
                return OK;
            }

            m_bAuthenticated = true;
        }
        
        // Update session id in database
        if (bUpdateSessionId)
        {
            // Write the empire's new session id
            iErrCode = SetEmpireSessionId (m_iEmpireKey, m_i64SessionId);
            RETURN_ON_ERROR(iErrCode);
        }
        
        // Update session id cookie
        if (bUpdateCookie)
        {
            // Best effort set a new cookie
            char pszSessionId [128];
            String::I64toA (m_i64SessionId, pszSessionId, 10);
            
            iErrCode = m_pHttpResponse->CreateCookie ("SessionId", pszSessionId, 31536000, NULL);
            RETURN_ON_ERROR(iErrCode);
        }
        
        // Update IP address
        if (strcmp(m_pHttpRequest->GetClientIP(), m_vPreviousIPAddress.GetCharPtr()) != 0)
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
        bool bExists;
        iErrCode = DoesEmpireExist (m_iEmpireKey, &bExists, NULL);
        RETURN_ON_ERROR(iErrCode);
        if (!bExists) {
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
// Hashing
//

int HtmlRenderer::GetAutologonPasswordHash(int iEmpireKey, String* pstrHash)
{
    int iErrCode;
    Crypto::HashSHA256 hash;

    // Password hash
    Variant vPasswordHash;
    iErrCode = GetEmpireDataColumn(iEmpireKey, SystemEmpireData::PasswordHash, &vPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = hash.HashData(vPasswordHash.GetCharPtr(), strlen(vPasswordHash));
    RETURN_ON_ERROR(iErrCode);

    // User agent
    const char* pszBrowser = m_pHttpRequest->GetBrowserName();
    if (pszBrowser)
    {
        iErrCode = hash.HashData(pszBrowser, strlen(pszBrowser));
        RETURN_ON_ERROR(iErrCode);
    }

    // Empire's secret key
    Variant vSecretKey;
    iErrCode = GetEmpireDataColumn(iEmpireKey, SystemEmpireData::SecretKey, &vSecretKey);
    RETURN_ON_ERROR(iErrCode);

    int64 i64SecretKey = vSecretKey.GetInteger64();
    iErrCode = hash.HashData(&i64SecretKey, sizeof(i64SecretKey));
    RETURN_ON_ERROR(iErrCode);

    // Fixed server salt
    Variant vFixedSalt;
    iErrCode = GetSystemProperty(SystemData::FixedHashSalt, &vFixedSalt);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = hash.HashData(vFixedSalt, strlen(vFixedSalt));
    RETURN_ON_ERROR(iErrCode);

    // Done
    size_t cbSize;
    iErrCode = hash.GetHashSize(&cbSize);
    RETURN_ON_ERROR(iErrCode);

    void* pBuffer = StackAlloc(cbSize);
    iErrCode = hash.GetHash(pBuffer, cbSize);
    RETURN_ON_ERROR(iErrCode);

    size_t cch = Algorithm::GetEncodeBase64Size(cbSize);
    char* pszBase64 = (char*)StackAlloc(cch);
    iErrCode = Algorithm::EncodeBase64(pBuffer, cbSize, pszBase64, cch);
    RETURN_ON_ERROR(iErrCode);

    pstrHash->Clear();
    *pstrHash = pszBase64;
    Assert(pstrHash->GetCharPtr());

    return iErrCode;
}

int HtmlRenderer::GetPagePasswordHash(PageId page, int iEmpireKey, const UTCTime& tSalt, String* pstrHash)
{
    int iErrCode;
    Crypto::HashSHA256 hash;

    // Password
    Variant vPasswordHash;
    iErrCode = GetEmpireDataColumn(iEmpireKey, SystemEmpireData::PasswordHash, &vPasswordHash);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = hash.HashData(vPasswordHash.GetCharPtr(), strlen(vPasswordHash));
    RETURN_ON_ERROR(iErrCode);

    // Salt
    Assert(tSalt != NULL_TIME);
    iErrCode = hash.HashData (&tSalt, sizeof (tSalt));
    RETURN_ON_ERROR(iErrCode);

    // Browser
    const char* pszBrowser  = m_pHttpRequest->GetBrowserName();
    if (pszBrowser != NULL)
    {
        iErrCode = hash.HashData (pszBrowser, strlen (pszBrowser));
        RETURN_ON_ERROR(iErrCode);
    }

    // Empire's secret key
    Variant vSecretKey;
    iErrCode = GetEmpireDataColumn(iEmpireKey, SystemEmpireData::SecretKey, &vSecretKey);
    RETURN_ON_ERROR(iErrCode);

    int64 i64SecretKey = vSecretKey.GetInteger64();
    iErrCode = hash.HashData(&i64SecretKey, sizeof(i64SecretKey));
    RETURN_ON_ERROR(iErrCode);

    // Fixed server salt
    Variant vFixedSalt;
    iErrCode = GetSystemProperty(SystemData::FixedHashSalt, &vFixedSalt);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = hash.HashData(vFixedSalt, strlen(vFixedSalt));
    RETURN_ON_ERROR(iErrCode);

    if (IsGamePage(page))
    {
        // IP Address
        if (m_iSystemOptions & IP_ADDRESS_PASSWORD_HASHING)
        {
            const char* pszIPAddress = m_pHttpRequest->GetClientIP();
            iErrCode = hash.HashData (pszIPAddress, strlen (pszIPAddress));
            RETURN_ON_ERROR(iErrCode);
        }

        // Session Id
        if (m_iSystemOptions & SESSION_ID_PASSWORD_HASHING)
        {
            iErrCode = hash.HashData (&m_i64SessionId, sizeof (m_i64SessionId));
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Done
    size_t cbSize;
    iErrCode = hash.GetHashSize(&cbSize);
    RETURN_ON_ERROR(iErrCode);

    void* pBuffer = StackAlloc(cbSize);
    iErrCode = hash.GetHash(pBuffer, cbSize);
    RETURN_ON_ERROR(iErrCode);

    size_t cch = Algorithm::GetEncodeBase64Size(cbSize);
    char* pszBase64 = (char*)StackAlloc(cch);
    iErrCode = Algorithm::EncodeBase64(pBuffer, cbSize, pszBase64, cch);
    RETURN_ON_ERROR(iErrCode);

    pstrHash->Clear();
    *pstrHash = pszBase64;
    Assert(pstrHash->GetCharPtr());

    return iErrCode;
}