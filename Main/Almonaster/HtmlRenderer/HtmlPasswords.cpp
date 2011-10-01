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
    
    if (m_vEmpireName.GetType() != V_STRING || 
        m_vEmpireName.GetCharPtr() == NULL ||
        *m_vEmpireName.GetCharPtr() == '\0') {
        
        iErrCode = GetEmpireName (m_iEmpireKey, &m_vEmpireName);
        RETURN_ON_ERROR(iErrCode);
    }
    
    if (m_vPassword.GetType() != V_STRING || 
        m_vPassword.GetCharPtr() == NULL ||
        *m_vPassword.GetCharPtr() == '\0') {
        
        iErrCode = GetEmpirePassword(m_iEmpireKey, &m_vPassword);
        RETURN_ON_ERROR(iErrCode);
    }

    if (m_i64SecretKey == 0) {

        Variant vValue;
        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::SecretKey, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_i64SecretKey = vValue.GetInteger64();
    }
    
    // Get last login time
    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::LastLoginTime, &vValue);
    RETURN_ON_ERROR(iErrCode);
    UTCTime lastLoginTime = vValue.GetInteger64();

    // We're authenticated, so register a login
    iErrCode = LoginEmpire (m_iEmpireKey, m_pHttpRequest->GetBrowserName(), m_pHttpRequest->GetClientIP());
    if (iErrCode != OK)
    {
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
        else
        {
            RETURN_ON_ERROR(iErrCode);
        }
    }
    else
    {
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

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlienKey, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_iAlienKey = vValue.GetInteger();

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::IPAddress, &m_vPreviousIPAddress);
        RETURN_ON_ERROR(iErrCode);
        
        // Set a cookie for the last empire id (expires in a year)
        char pszEmpireKey [128];
        iErrCode = m_pHttpResponse->CreateCookie (
            LAST_EMPIRE_USED_COOKIE,
            String::UItoA (m_iEmpireKey, pszEmpireKey, 10),
            ONE_YEAR_IN_SECONDS, 
            NULL
            );
        RETURN_ON_ERROR(iErrCode);
    }

    AddMessage ("Welcome back, ");
    AppendMessage (m_vEmpireName.GetCharPtr());

    if (Time::OlderThan (lastLoginTime, m_stServerNewsLastUpdate)) {
        AddMessage ("The Server News page was updated since your last login");
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

            // Get empire's password
            iErrCode = GetEmpirePassword (m_iEmpireKey, &m_vPassword);
            RETURN_ON_ERROR(iErrCode);
            
            // Get empire's recorded IP address
            iErrCode = GetEmpireIPAddress(m_iEmpireKey, &m_vPreviousIPAddress);
            RETURN_ON_ERROR(iErrCode);

            // Get empire's secret key
            Variant vValue;
            iErrCode = GetEmpireProperty(m_iEmpireKey, SystemEmpireData::SecretKey, &vValue);
            RETURN_ON_ERROR(iErrCode);
            m_i64SecretKey = vValue.GetInteger64();
            
            // Try to read a hashed password
            if ((pHttpForm = m_pHttpRequest->GetForm ("Password")) != NULL) {
                
                int64 i64SubmittedPasswordHash = pHttpForm->GetInt64Value();
                int64 i64RealPasswordHash = 0;

                // Read salt
                if ((pHttpForm = m_pHttpRequest->GetForm ("Salt")) == NULL) {
                    AddMessage ("The password was not salted");
                    return ERROR_MISSING_FORM;
                }
                m_tOldSalt = pHttpForm->GetUInt64Value();

                // Make sure salt is valid
                if (Time::GetSecondDifference (m_tNewSalt, m_tOldSalt) >= DAY_LENGTH_IN_SECONDS) {
                    AddMessage("Your session has timed out. Please log in again");
                    return OK;
                }

                // Authenticate
                if (!IsGamePage (m_pgPageId))
                {
                    iErrCode = GetPasswordHashForSystemPage (m_tOldSalt, &i64RealPasswordHash);
                    RETURN_ON_ERROR(iErrCode);
                }
                else
                {
                    iErrCode = GetPasswordHashForGamePage (m_tOldSalt, &i64RealPasswordHash);
                    RETURN_ON_ERROR(iErrCode);
                }

                if (i64RealPasswordHash != i64SubmittedPasswordHash)
                {
                    char pszBuffer [256 + MAX_EMPIRE_NAME_LENGTH];
                    sprintf(pszBuffer, "That was the wrong password for the %s empire", m_vEmpireName.GetCharPtr());
                    
                    AddMessage (pszBuffer);
                    return OK;
                }
            }
            else
            {
                // Try to use a cleartext password
                const char* pszPassword;
                if ((pHttpForm = m_pHttpRequest->GetForm ("ClearTextPassword")) == NULL ||
                    (pszPassword = pHttpForm->GetValue()) == NULL ||
                    strcmp (pszPassword, m_vPassword.GetCharPtr()) != 0
                    ) {
                    
                    char pszBuffer [256 + MAX_EMPIRE_NAME_LENGTH];
                    sprintf(pszBuffer, "That was the wrong password for the %s empire", m_vEmpireName.GetCharPtr());
                    
                    AddMessage (pszBuffer);
                    return OK;
                }
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

int HtmlRenderer::GetPasswordHashForAutologon (int64* pi64Hash) {

    int iErrCode;
    Crypto::HashMD5 hash;
    size_t stHashLen;

    // Password
    const char* pszPassword = m_vPassword.GetCharPtr();
    iErrCode = hash.HashData (pszPassword, strlen (pszPassword));
    RETURN_ON_ERROR(iErrCode);

    // Browser
    const char* pszBrowser = m_pHttpRequest->GetBrowserName();
    if (pszBrowser != NULL)
    {
        iErrCode = hash.HashData (pszBrowser, strlen (pszBrowser));
        RETURN_ON_ERROR(iErrCode);
    }

    // Secret key
    Assert(m_i64SecretKey != 0);
    iErrCode = hash.HashData (&m_i64SecretKey, sizeof (m_i64SecretKey));
    RETURN_ON_ERROR(iErrCode);

    // Done
    iErrCode = hash.GetHashSize (&stHashLen);
    RETURN_ON_ERROR(iErrCode);

    void* pbHashData = StackAlloc(stHashLen);
    iErrCode = hash.GetHash(pbHashData, stHashLen);
    RETURN_ON_ERROR(iErrCode);

    Assert(stHashLen >= sizeof (int64));
    *pi64Hash = *(int64*) pbHashData;

    return iErrCode;
}

int HtmlRenderer::GetPasswordHashForGamePage(const UTCTime& tSalt, int64* pi64Hash)
{
    int iErrCode;
    Crypto::HashMD5 hash;
    size_t stHashLen;

    // Password
    const char* pszPassword = m_vPassword.GetCharPtr();
    iErrCode = hash.HashData (pszPassword, strlen (pszPassword));
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

    // Secret key
    Assert(m_i64SecretKey != 0);
    iErrCode = hash.HashData ((Byte*) &m_i64SecretKey, sizeof (m_i64SecretKey));
    RETURN_ON_ERROR(iErrCode);

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

    // Done
    iErrCode = hash.GetHashSize (&stHashLen);
    RETURN_ON_ERROR(iErrCode);

    void* pbHashData = StackAlloc (stHashLen);
    iErrCode = hash.GetHash (pbHashData, stHashLen);
    RETURN_ON_ERROR(iErrCode);

    Assert(stHashLen >= sizeof (int64));
    *pi64Hash = *(int64*) pbHashData;

    return iErrCode;
}

int HtmlRenderer::GetPasswordHashForSystemPage (const UTCTime& tSalt, int64* pi64Hash) {

    int iErrCode;
    Crypto::HashMD5 hash;
    size_t stHashLen;

    // Password
    const char* pszPassword = m_vPassword.GetCharPtr();
    iErrCode = hash.HashData (pszPassword, strlen (pszPassword));
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

    // Secret key
    Assert(m_i64SecretKey != 0);
    iErrCode = hash.HashData (&m_i64SecretKey, sizeof (m_i64SecretKey));
    RETURN_ON_ERROR(iErrCode);

    // Done
    iErrCode = hash.GetHashSize (&stHashLen);
    RETURN_ON_ERROR(iErrCode);

    void* pbHashData = StackAlloc (stHashLen);
    iErrCode = hash.GetHash (pbHashData, stHashLen);
    RETURN_ON_ERROR(iErrCode);

    Assert(stHashLen >= sizeof (int64));
    *pi64Hash = *(int64*) pbHashData;

    return iErrCode;
}