//
// Admin.dll
// Copyright (c) 1999 Max Attar Feingold (maf6@cornell.edu)
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

#include "Admin.h"
#include "Osal/TraceLog.h"

const Uuid CLSID_Admin = { 0x8b631301, 0x8cfa, 0x11d3, { 0xa2, 0x40, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };

int CreateInstance (const Uuid& uuidClsid, const Uuid& uuidIid, void** ppObject) {

    if (uuidClsid == CLSID_Admin) {

        if (uuidIid == IID_IPageSource) {
            *ppObject = (void*) static_cast<IPageSource*> (Admin::CreateInstance());
            return *ppObject == NULL ? ERROR_OUT_OF_MEMORY : OK;
        }

        return ERROR_NO_INTERFACE;
    }

    return ERROR_NO_CLASS;
}

Admin::Admin()
{
    m_pHttpServer = NULL;
    m_pConfig = NULL;
    m_pServerConfig = NULL;
    m_pPageSourceControl = NULL;
    m_pFileCache = NULL;
    m_pszLogin = NULL;
    m_pszPassword = NULL;
    m_chDisplayChars = 0;

    m_iNumRefs = 1;
}

Admin::~Admin()
{
    // Clean up
    if (m_pszLogin != NULL)
    {
        OS::HeapFree(m_pszLogin);
        m_pszLogin = NULL;
    }

    if (m_pszPassword != NULL)
    {
        OS::HeapFree(m_pszPassword);
        m_pszPassword = NULL;
    }
}

Admin* Admin::CreateInstance() {
    
    return new Admin();
}

int Admin::OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pPageSourceControl)
{
    // Weak refs
    m_pHttpServer = pHttpServer;
    m_pPageSourceControl = pPageSourceControl;

    // Strong refs
    m_pFileCache = m_pHttpServer->GetFileCache();
    m_pServerConfig = m_pHttpServer->GetConfigFile();
    m_pConfig = m_pPageSourceControl->GetConfigFile();

    ITraceLog* pReport = m_pPageSourceControl->GetReport();
    AutoRelease<ITraceLog> release_pReport(pReport);

    int iErrCode = m_mServerLock.Initialize();
    if (iErrCode != OK) {
        pReport->Write(TRACE_ERROR, "Could not initialize m_mServerLock");
        return iErrCode;
    }

    // Read login and password from the config file
    char* pszTemp;
    if (m_pConfig->GetParameter ("Login", &pszTemp) != OK) {
        pReport->Write(TRACE_ERROR, "Could not read the Login value from the admin .conf file");
        return ERROR_FAILURE;
    }

    if (pszTemp == NULL || *pszTemp == '\0') {
        pReport->Write(TRACE_ERROR, "The Login value in the admin .conf file cannot be zero length");
        return ERROR_FAILURE;
    }

    m_pszLogin = String::StrDup (pszTemp);
    if (m_pszLogin == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (m_pConfig->GetParameter ("Password", &pszTemp) != OK) {
        pReport->Write(TRACE_ERROR, "Could not read the Password value from the admin .conf file");
        return ERROR_FAILURE;
    }

    // Allow null passwords...
    m_pszPassword = String::StrDup (pszTemp);
    if (m_pszPassword == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    if (m_pConfig->GetParameter ("DisplayChars", &pszTemp) != OK || pszTemp == NULL || *pszTemp == '\0') {
        pReport->Write(TRACE_ERROR, "Could not read the DisplayChars value from the admin .conf file");
        return ERROR_FAILURE;
    }

    m_chDisplayChars = atoi (pszTemp);

    return OK;
}

int Admin::OnFinalize()
{
    SafeRelease(m_pFileCache);
    SafeRelease(m_pServerConfig);
    SafeRelease(m_pConfig);

    return OK;
}

int Admin::OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    pHttpResponse->SetNoBuffering();

    return RenderAdminPage (pHttpRequest, pHttpResponse, "");
}

int Admin::OnPost (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    pHttpResponse->SetNoBuffering();

    String strMessage;

    int iErrCode = HandleAdminPageSubmission (pHttpRequest, pHttpResponse, &strMessage);
    if (iErrCode == OK) {
        iErrCode = RenderAdminPage (pHttpRequest, pHttpResponse, strMessage);
    }

    return iErrCode;
}

int Admin::OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {
    return OK;
}

const char* Admin::GetAuthenticationRealm (IHttpRequest* pHttpRequest) {

    // All resources share the same realm
    return "admin";
}

int Admin::OnBasicAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated) {

    const char* pszUserName = pHttpRequest->GetAuthenticationUserName();

    // Check username - we only allow one user anyway
    if (String::StriCmp (pszUserName, m_pszLogin) != 0) {
        *pbAuthenticated = false;
        return OK;
    }

    // Delegate to default implementation
    return pHttpRequest->BasicAuthenticate (m_pszPassword, pbAuthenticated);
}

int Admin::OnDigestAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated) {

    const char* pszUserName = pHttpRequest->GetAuthenticationUserName();

    // Check username - we only allow one user anyway
    if (String::StriCmp (pszUserName, m_pszLogin) != 0) {
        *pbAuthenticated = false;
        return OK;
    }

    // Delegate to default implementation
    return pHttpRequest->DigestAuthenticate (m_pszPassword, pbAuthenticated);
}