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

#include "Almonaster.h"
#include "Global.h"

#include "HtmlRenderer.h"
#include "Chatroom/CChatroom.h"
#include "SqlDatabase.h"

#include "Osal/File.h"
#include "Osal/Vector.h"

///////////////////////////////
// PageSource implementation //
///////////////////////////////

Almonaster::Almonaster()
{
    m_bIsDefault = false;

    m_iNumRefs = 1;

    m_pszUri1 = NULL;
    m_pszUri2 = NULL;

    memset (&HtmlRenderer::m_sStats, 0, sizeof (AlmonasterStatistics));
}

Almonaster::~Almonaster()
{
    if (!m_bIsDefault && m_pszUri1 != NULL)
    {
        delete [] m_pszUri1;
    }
}

Almonaster* Almonaster::CreateInstance()
{
    return new Almonaster();
}

int Almonaster::OnInitialize(IHttpServer* pHttpServer, IPageSourceControl* pPageSourceControl)
{
    // Initialize global state
    int iErrCode = global.Initialize(pHttpServer, pPageSourceControl);
    if (iErrCode != OK)
    {
        return iErrCode;
    }

    // Weak ref
    IReport* pReport = global.GetReport();

    // Prepare URI's
    m_bIsDefault = pPageSourceControl->IsDefault();

    if (m_bIsDefault) {

        m_pszUri1 = "/";
        m_pszUri2 = "";

    } else {

        const char* pszPageSourceName = pPageSourceControl->GetName();
        size_t stLen = strlen (pszPageSourceName) + 1;

        m_pszUri1 = new char [stLen * 2 + 3];
        if (m_pszUri1 == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        m_pszUri1[0] = '/';
        memcpy (m_pszUri1 + 1, pszPageSourceName, stLen);

        m_pszUri2 = m_pszUri1 + stLen + 1;
        memcpy (m_pszUri2, m_pszUri1, stLen);
        m_pszUri2[stLen] = '/';
        m_pszUri2[stLen + 1] = '\0';
    }

    // Initialize HtmlRenderer statics
    iErrCode = HtmlRenderer::StaticInitialize();
    if (iErrCode != OK)
    {
        pReport->WriteReport ("Error: HtmlRenderer::Initialize failed");
        iErrCode = ERROR_FAILURE;
        goto Cleanup;
    }

Cleanup:

    if (iErrCode != OK)
    {
        pReport->WriteReport ("Almonaster could not be initialize successfully");
    }
    else
    {
        pReport->WriteReport ("Finished initializing GameEngine");

        pReport->WriteReport ("Almonaster will now begin");
        pReport->WriteReport ("===================================================");
    }

    return iErrCode;
}

// A get will mostly be a request for an image file or the login screen
// We should only override requests for "/" or for the page source name
// followed by a "/"
int Almonaster::OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    // Look at the URI and override only /almonaster/ requests
    const char* pszUri = pHttpRequest->GetUri();

    if (String::StriCmp (m_pszUri1, pszUri) != 0 &&
        String::StriCmp (m_pszUri2, pszUri) != 0
        ) {

        // No override
        return OK;
    }

    return OnPost (pHttpRequest, pHttpResponse);
}

// Here we know that someone has POSTED information to our page source,
// so we need to reply with some page or other
int Almonaster::OnPost(IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse)
{
    int iErrCode;
    PageId pageId = LOGIN;

    // Determine the submission page
    IHttpForm* pHttpForm = pHttpRequest->GetForm("PageId");
    if (pHttpForm != NULL)
    {
        int iValue = pHttpForm->GetIntValue();
        if (iValue > MIN_PAGE_ID && iValue < MAX_PAGE_ID)
        {
            pageId = (PageId)iValue;
        }
    }

    //// Enable chunked encoding if so configured if the page requested is a doc page
    //if (pageId == SYSTEM_DOCUMENTATION || pageId == GAME_DOCUMENTATION)
    //{
    //    pHttpResponse->SetNoBuffering();
    //}

    // Render the page
    HtmlRenderer renderer(pageId, pHttpRequest, pHttpResponse);
    iErrCode = renderer.Render();

    return iErrCode;
}


int Almonaster::OnFinalize() {
    
    IReport* pReport = global.GetReport();

    pReport->WriteReport ("Shutting down Almonaster");
    global.Close();
    pReport->WriteReport ("Finished shutting down Almonaster");
    pReport->WriteReport ("===================================================");

    return OK;
}

int Almonaster::OnAccessDenied (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    HttpStatusReason rReason = pHttpResponse->GetStatusCodeReason();

    pHttpResponse->WriteText (
        "<html>"\
        "<head><title>Access denied</title></head>"\
        "<body>"\
        "<center><h1>Access denied</h1></center><p>"
        );
    
    switch (rReason) {
        
    case HTTP_REASON_IPADDRESS_BLOCKED:
        
        pHttpResponse->WriteText (
            "Your IP address has been banned by the administrator. "
            );
        break;
        
    case HTTP_REASON_USER_AGENT_BLOCKED:
        
        pHttpResponse->WriteText (
            "You cannot use your web browser with Almonaster. Please use a supported web browser. "
            );
        break;

    case HTTP_REASON_GET_REFERER_BLOCKED:

        pHttpResponse->WriteText (
            "You cannot download images from Almonaster from outside the game pages. "
            );
        break;

    default:

        break;
    }

    pHttpResponse->WriteText (
        "Contact the administrator for more details" \
        "</body>"\
        "</html>"
        );

    return OK;
}

int Almonaster::OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    // We're only interested in 403 - forbidden errors
    // These indicate another site leeching bandwidth from us
    if (pHttpResponse->GetStatusCode() == HTTP_403) {
        return OnAccessDenied (pHttpRequest, pHttpResponse);
    }

    return OK;
}

const char* Almonaster::GetAuthenticationRealm (IHttpRequest* pHttpRequest) {
    return NULL;
}

int Almonaster::OnBasicAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated) {
    *pbAuthenticated = false;
    return OK;
}

int Almonaster::OnDigestAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated) {  
    *pbAuthenticated = false;
    return OK;
}