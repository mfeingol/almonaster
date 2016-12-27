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

#if !defined(AFX_ADMIN_H__C25D88C3_44DB_11D2_9EEE_0060083E8062__INCLUDED_)
#define AFX_ADMIN_H__C25D88C3_44DB_11D2_9EEE_0060083E8062__INCLUDED_

#include "Alajar.h"
#include "Osal/Mutex.h"

extern "C" EXPORT int CreateInstance (const Uuid& uuidClsid, const Uuid& uuidIid, void** ppObject);

class Admin : public IPageSource {

private:

    Admin();

    IHttpServer* m_pHttpServer;
    IReport* m_pReport;
    IReport* m_pServerReport;
    IConfigFile* m_pConfig;
    IConfigFile* m_pServerConfig;
    ILog* m_pLog;
    IPageSourceControl* m_pPageSourceControl;
    IFileCache* m_pFileCache;

    char* m_pszLogin;
    char* m_pszPassword;
    size_t m_stDisplayChars;

    int ConvertTime (Seconds iNumSeconds, String* pstrTime);

    // Locks
    Mutex m_mServerLock;

public:

    static Admin* CreateInstance();

    int RenderAdminPage (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, const String& strMessage);

    int HandleAdminPageSubmission (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, 
        String* pstrMessage);

    IMPLEMENT_INTERFACE (IPageSource);

    int OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pControl);
    int OnFinalize();

    int OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);
    int OnPost (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);
    int OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);

    const char* GetAuthenticationRealm (IHttpRequest* pHttpRequest);

    int OnBasicAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated);
    int OnDigestAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated);
};

#endif