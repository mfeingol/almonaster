// PageSource.cpp: implementation of the PageSource class.
//
//////////////////////////////////////////////////////////////////////
//
// HttpObjects.dll:  a component of Alajar 1.0
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

#include "PageSource.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"

#include "Osal/Algorithm.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PageSource::AccessControlElement::AccessControlElement() {
}

int PageSource::AccessControlElement::Initialize (const char* pszName, bool bWildCard) {

    pszString = String::StrDup (pszName);
    if (pszString == NULL && !String::IsBlank (pszName)) {
        return ERROR_OUT_OF_MEMORY;
    }

    stLength = String::StrLen (pszName),
    bHasWildCard = bWildCard;

    return OK;
}

PageSource::AccessControlElement::~AccessControlElement() {

    if (pszString != NULL) {
        OS::HeapFree (pszString);
    }
}

PageSource::PageSource (HttpServer* pHttpServer) {

    m_pHttpServer = pHttpServer;

    m_pszName = NULL;

    m_pszLibraryName = NULL;
    m_pszConfigFileName = NULL;

    m_pPageSource = NULL;

    m_pcfConfig = NULL;
    m_pReport = NULL;
    m_pLog = NULL;

    m_bRestart = false;
    m_bScrewed = false;
    m_bShutdown = false;
    m_bIsDefault = false;

    m_iNumRefs = 1; // AddRef
    m_iNumThreads = 0;

    m_reportTracelevel = TRACE_WARNING;

    m_bFilterGets = false;

    m_atAuthType = AUTH_NONE;
    m_iDigestNonceLifetime = 0;

    Reset();
}

int PageSource::Init() {

    int iErrCode;

    iErrCode = m_mShutdownLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mCounterLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_reportMutex.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_logMutex.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    m_pcfConfig = Config::CreateInstance();
    if (m_pcfConfig == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    return OK;
}

void PageSource::Reset() {

    m_bLocked = false;

    m_bBrowsingAllowed = false;
    m_bDefaultFile = false;
    m_bOverrideGet = false;
    m_bOverridePost = false;
    m_bIsDefault = false;

    m_atAuthType = AUTH_NONE;

    memset (m_pbOverrideError, 0, sizeof (m_pbOverrideError));
    memset (m_ppCachedFile, 0, sizeof (m_ppCachedFile));

    m_bUseSSI = false;
    m_bUsePageSourceLibrary = false;
    m_bUseLogging = false;
    m_bUseCommonLogFormat = false;

    m_pszDefaultFile = NULL;

    m_pPageSource = NULL;

    m_bFilterGets = false;

    Time::ZeroTime (&m_tLogTime);
    Time::ZeroTime (&m_tReportTime);
}

PageSource::~PageSource() {

    Clean();

    if (m_pcfConfig != NULL) {
        m_pcfConfig->Release();
    }

    if (m_pszLibraryName != NULL) {
        OS::HeapFree (m_pszLibraryName);
    }

    if (m_pszName != NULL) {
        delete [] m_pszName;
    }

    if (m_pszConfigFileName != NULL) {
        OS::HeapFree (m_pszConfigFileName);
    }
}

void PageSource::Clean() {

    if (m_pPageSource != NULL) {
        m_pPageSource->OnFinalize();
        m_pPageSource->Release();
        m_pPageSource = NULL;
    }

    for (unsigned int i = 0; i < NUM_STATUS_CODES; i ++) {
        
        if (m_ppCachedFile[i] != NULL) {
            m_ppCachedFile[i]->Release();
        }
    }

    if (m_pszDefaultFile != NULL) {
        OS::HeapFree (m_pszDefaultFile);
    }

    m_cfCounterFile.Close();

    // Clear security lists
    ClearACEList (&m_llAllowIPAddressList);
    ClearACEList (&m_llDenyIPAddressList);
    ClearACEList (&m_llAllowUserAgentList);
    ClearACEList (&m_llDenyUserAgentList);

    // Clear GET filters
    ClearACEList (&m_llDenyGetExts);
    ClearACEList (&m_llAllowReferer);

    // Close files
    SafeRelease(m_pReport);
    SafeRelease(m_pLog);

    Time::ZeroTime (&m_tLogTime);
    Time::ZeroTime (&m_tReportTime);
}

void PageSource::ClearACEList (ACEList* pllList) {

    ListIterator<AccessControlElement*> liIt;
    while (pllList->PopFirst (&liIt)) {
        delete liIt.GetData();
    }
    pllList->Clear();
}

//
// IObject
//
unsigned int PageSource::AddRef() {

    return Algorithm::AtomicIncrement (&m_iNumRefs);
}

unsigned int PageSource::Release() {

    unsigned int iNumRefs = Algorithm::AtomicDecrement (&m_iNumRefs);
    if (iNumRefs == 0) {
        delete this;
    }
    return iNumRefs;
}       

int PageSource::QueryInterface (const Uuid& iidInterface, void** ppInterface) {

    if (iidInterface == IID_IObject) {
        *ppInterface = (void*) static_cast<IObject*> (this);            
        AddRef();                                                       
        return OK;                                                      
    }   
    if (iidInterface == IID_IPageSourceControl) {                               
        *ppInterface = (void*) static_cast<IPageSourceControl*> (this);         
        AddRef();                                                       
        return OK;                                                      
    }                                                                   
    if (iidInterface == IID_IPageSource) {                              
        *ppInterface = (void*) static_cast<IPageSource*> (this);            
        AddRef();                                                       
        return OK;                                                      
    }

    *ppInterface = NULL;                                                
    return ERROR_NO_INTERFACE;                                          
}

///////////////
// Configure //
///////////////

int PageSource::Restart() {

    m_mShutdownLock.Wait();
    if (m_bShutdown || m_bRestart) {
        m_mShutdownLock.Signal();
        return ERROR_FAILURE;
    }

    m_bRestart = true;

    // Start a new thread that restarts us
    Thread tRestart;
    int iErrCode = tRestart.Start (PageSource::RestartPageSource, this);

    m_mShutdownLock.Signal();

    return iErrCode;
}

bool PageSource::IsRestarting() {
    return m_bRestart;
}

bool PageSource::IsWorking() {
    return !m_bScrewed;
}

// pszLibraryName -> full name of the dll
// pszNameSpace -> Prefix of the functions in the dll
// pszPageSourceName -> Name of the page source
int PageSource::Initialize (const char* pszLibraryName, const char* pszClsid) {

    if (m_pszLibraryName == NULL && pszLibraryName != NULL) {
        m_pszLibraryName = String::StrDup (pszLibraryName);
        if (m_pszLibraryName == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
    }

    if (pszLibraryName == NULL || pszClsid == NULL) {
        return ERROR_FAILURE;
    }

    // Get the clsid
    Uuid uuidClsid;
    int iErrCode = OS::UuidFromString (pszClsid, &uuidClsid);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Load the library
    iErrCode = m_libPageSource.Open (pszLibraryName);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Get the export
    Fxn_CreateInstance pCreateInstance = (Fxn_CreateInstance) m_libPageSource.GetExport ("CreateInstance");
    if (pCreateInstance == NULL) {
        m_libPageSource.Close();
        return ERROR_FAILURE;
    }

    // Call CreateInstance
    iErrCode = pCreateInstance (uuidClsid, IID_IPageSource, (void**) &m_pPageSource);
    if (iErrCode != OK) {
        m_libPageSource.Close();
        return iErrCode;
    }

    // Initialize underlying PageSource
    return OnInitialize(m_pHttpServer, this);
}

static const char* g_pszSeparator = ";";

int PageSource::SetAccess (ACEList* pllAccessList, const char* pszAccessList) {

    Assert (pszAccessList != NULL);

    char* pszCopy = (char*) StackAlloc (strlen (pszAccessList) + 1);
    strcpy (pszCopy, pszAccessList);

    char* pszTemp = strtok (pszCopy, g_pszSeparator);
    while (pszTemp != NULL) {

        int iErrCode;
        bool bWildCard = false;

        AccessControlElement* paceInsert = new AccessControlElement();
        if (paceInsert == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        char* pszStar = strstr (pszTemp, "*");

        if (pszStar != NULL) {
            *pszStar = '\0';
            bWildCard = true;
        }

        iErrCode = paceInsert->Initialize (pszTemp, bWildCard);
        if (iErrCode != OK) {
            return iErrCode;
        }
            
        if (!pllAccessList->PushLast (paceInsert)) {
            delete paceInsert;
            return ERROR_OUT_OF_MEMORY;
        }

        pszTemp = strtok (NULL, g_pszSeparator);
    }

    return OK;
}


////////////////////////
// Configuration data //
////////////////////////

bool PageSource::IsDefault() {
    return m_bIsDefault;
}

const char* PageSource::GetName() {
    return m_pszName;
}

IConfigFile* PageSource::GetConfigFile() {
    Assert (m_pcfConfig != NULL);
    m_pcfConfig->AddRef();
    return m_pcfConfig;
}

ITraceLog* PageSource::GetReport()
{
    UTCTime tNow;
    Time::GetTime(&tNow);

    ITraceLog* pReturn;

    m_reportMutex.Wait();
    if (HttpServer::DifferentDays(m_tReportTime, tNow))
    {
        m_tReportTime = tNow;

        char pszFileName[OS::MaxFileNameLength];
        GetReportFileName(pszFileName);

        Report* pNew = new Report();
        if (pNew)
        {
            int iErrCode = pNew->Initialize((ReportFlags)(WRITE_DATE_TIME | WRITE_THREAD_ID), m_reportTracelevel, pszFileName);
            if (iErrCode == OK)
            {
                SafeRelease(m_pReport);
                m_pReport = pNew;
            }
        }
    }
    pReturn = m_pReport;
    pReturn->AddRef();
    m_reportMutex.Signal();

    return pReturn;
}

ITraceLog* PageSource::GetLog()
{
    UTCTime tNow;
    Time::GetTime(&tNow);

    ITraceLog* pReturn;

    m_logMutex.Wait();
    if (HttpServer::DifferentDays(m_tLogTime, tNow))
    {
        m_tLogTime = tNow;

        char pszFileName[OS::MaxFileNameLength];
        GetLogFileName(pszFileName);

        Report* pNew = new Report();
        if (pNew)
        {
            int iErrCode = pNew->Initialize(WRITE_NONE, m_reportTracelevel, pszFileName);
            if (iErrCode == OK)
            {
                SafeRelease(m_pLog);
                m_pLog = pNew;
            }
        }
    }
    pReturn = m_pLog;
    pReturn->AddRef();
    m_logMutex.Signal();

    return pReturn;
}

const char* PageSource::GetBasePath() {
    return m_pszBasePath;
}

const char* PageSource::GetDefaultFile() {
    return m_pszDefaultFile;
}

ICachedFile* PageSource::GetErrorFile (HttpStatus sStatus) {
    if (m_ppCachedFile[sStatus] != NULL) {
        m_ppCachedFile[sStatus]->AddRef();
    }
    return m_ppCachedFile[sStatus];
}

bool PageSource::AllowDirectoryBrowsing() {
    return m_bBrowsingAllowed;
}

bool PageSource::UseDefaultFile() {
    return m_bDefaultFile;
}

HttpAuthenticationType PageSource::GetAuthenticationType() {
    return m_atAuthType;
}

Seconds PageSource::GetDigestAuthenticationNonceLifetime() {
    return m_iDigestNonceLifetime;
}

const char* PageSource::GetAuthenticationRealm (IHttpRequest* pHttpRequest) {
    if (m_pPageSource == NULL)
        return NULL;
    return m_pPageSource->GetAuthenticationRealm (pHttpRequest);
}

bool PageSource::OverrideGet() {
    return m_bOverrideGet;
}

bool PageSource::OverridePost() {
    return m_bOverridePost;
}

bool PageSource::OverrideError (HttpStatus sStatus) {
    return m_pbOverrideError[sStatus];
}

bool PageSource::UsePageSourceLibrary() {
    return m_bUsePageSourceLibrary;
}

bool PageSource::UseSSI() {
    return m_bUseSSI;
}

bool PageSource::UseLogging() {
    return m_bUseLogging;
}

bool PageSource::UseCommonLogFormat() {
    return m_bUseCommonLogFormat;
}

ConfigFile* PageSource::GetCounterFile() {
    return &m_cfCounterFile;
}

bool PageSource::IsIPAddressAllowedAccess (const char* pszIPAddress) {

    return IsAllowedAccess (m_llAllowIPAddressList, m_llDenyIPAddressList, pszIPAddress);
}

bool PageSource::IsUserAgentAllowedAccess (const char* pszUserAgent) {

    return IsAllowedAccess (m_llAllowUserAgentList, m_llDenyUserAgentList, pszUserAgent);
}

bool PageSource::IsAllowedAccess (const ACEList& llAllowList, const ACEList& llDenyList, const char* pszString) {

    if (llAllowList.GetNumElements() > 0) {     
        return IsStringInACEList (llAllowList, pszString);
    }

    if (llDenyList.GetNumElements() > 0) {
        return !IsStringInACEList (llDenyList, pszString);
    }

    return true;
}

bool PageSource::IsGetAllowed (HttpRequest* pHttpRequest) {

    // If we're not filtering, allow
    if (!m_bFilterGets) {
        return true;
    }

    // Get the file requested
    const char* pszFileName = pHttpRequest->GetFileName();

    // Allow null file requests, whatever that means
    if (pszFileName == NULL) {
        return true;
    }

    // Get the extension - find the last dot and skip it
    const char* pszExt = strrchr (pszFileName, '.');
    if (pszExt == NULL) {

        // No extension, so allow
        return true;
    }
    pszExt ++;

    // See if it's a file extension of interest.  If not, allow
    if (!IsStringInACEList (m_llDenyGetExts, pszExt)) {
        return true;
    }

    // Get the referer
    const char* pszReferer = pHttpRequest->GetReferer();

    // Deny null referers
    if (pszReferer == NULL) {
        return false;
    }

    // See if it's an allowed referer.  If so, allow
    if (IsStringInACEList (m_llAllowReferer, pszReferer)) {
        return true;
    }

    // Deny
    return false;
}

bool PageSource::IsStringInACEList (const ACEList& llList, const char* pszString) {

    ListIterator<AccessControlElement*> liIt;
    while (llList.GetNextIterator (&liIt)) {

        AccessControlElement* pAce = liIt.GetData();
        if (pAce->bHasWildCard) {

            if (String::StrniCmp (pAce->pszString, pszString, pAce->stLength) == 0) {
                return true;
            }
            
        } else {

            if (String::StriCmp (pAce->pszString, pszString) == 0) {
                return true;
            }
        }
    }

    return false;
}


/////////////
// Methods //
/////////////

bool PageSource::Enter() {

    Algorithm::AtomicIncrement (&m_iNumThreads);
    if (!m_bLocked) {
        return false;
    }

    Algorithm::AtomicDecrement (&m_iNumThreads);
    m_mLock.Wait();
    Algorithm::AtomicIncrement (&m_iNumThreads);

    return true;
}

void PageSource::Exit (bool bLocked) {

    if (bLocked) {
        m_mLock.Signal();
    }
    Algorithm::AtomicDecrement (&m_iNumThreads);
}

// IPageSource
int PageSource::OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pControl) {
    return m_pPageSource->OnInitialize (pHttpServer, pControl);
}

int PageSource::OnFinalize() {

    if (m_pPageSource == NULL) {
        return OK;
    }

    int iErrCode = m_pPageSource->OnFinalize();
    SafeRelease (m_pPageSource);
    return iErrCode;
}

int PageSource::OnBasicAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated) {

    bool bLocked = Enter();
    int iErrCode = m_pPageSource->OnBasicAuthenticate (pHttpRequest, pbAuthenticated);

    Exit (bLocked);
    return iErrCode;
}

int PageSource::OnDigestAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated) {

    bool bLocked = Enter();
    int iErrCode = m_pPageSource->OnDigestAuthenticate (pHttpRequest, pbAuthenticated);

    Exit (bLocked);
    return iErrCode;
}

int PageSource::OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {
    
    bool bLocked = Enter();
    int iErrCode = m_pPageSource->OnGet (pHttpRequest, pHttpResponse);

    Exit (bLocked);
    return iErrCode;
}

int PageSource::OnPost (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {
    
    bool bLocked = Enter();
    int iErrCode = m_pPageSource->OnPost (pHttpRequest, pHttpResponse);

    Exit (bLocked);
    return iErrCode;
}

int PageSource::OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {
    
    if (m_pPageSource == NULL)
        return OK;

    bool bLocked = Enter();
    int iErrCode = m_pPageSource->OnError (pHttpRequest, pHttpResponse);

    Exit (bLocked);
    return iErrCode;
}

int PageSource::Configure (const char* pszConfigFileName, String* pstrErrorMessage) {

    char* pszRhs;
    size_t stLength;

    if (m_pszConfigFileName == NULL && pszConfigFileName != NULL) {

        m_pszConfigFileName = String::StrDup (pszConfigFileName);

        if (m_pszConfigFileName == NULL) {
            *pstrErrorMessage = "The server is out of memory";
            return ERROR_OUT_OF_MEMORY;
        }
    }

    // Open PageSource config file
    int iErrCode = m_pcfConfig->Open (m_pszConfigFileName) != OK;
    if (iErrCode != OK) {
        *pstrErrorMessage = "The config file was invalid";
        return iErrCode;
    }

    // ReportTraceLevel
    if (m_pcfConfig->GetParameter("ReportTraceLevel", &pszRhs) == OK && pszRhs != NULL)
    {
        if (String::StriCmp(pszRhs, "verbose") == 0)
        {
            m_reportTracelevel = TRACE_VERBOSE;
        }
        else if (String::StriCmp(pszRhs, "info") == 0)
        {
            m_reportTracelevel = TRACE_INFO;
        }
        else if (String::StriCmp(pszRhs, "warning") == 0)
        {
            m_reportTracelevel = TRACE_WARNING;
        }
        else if (String::StriCmp(pszRhs, "error") == 0)
        {
            m_reportTracelevel = TRACE_ERROR;
        }
        // Else just use the default
    }

    // The pagesource's name is the name of the file sans the last extension
    if (m_pszName == NULL) {

        pszRhs = strrchr (m_pszConfigFileName, '.');
        
        const char* pszStart = strrchr (m_pszConfigFileName, '/');
        if (pszStart == NULL) {
            pszStart = m_pszConfigFileName;
        } else {
            pszStart ++;
        }

        stLength = pszRhs - pszStart;
        
        m_pszName = new char [stLength + 1];
        if (m_pszName == NULL) {
            *pstrErrorMessage = "The server is out of memory";
            return ERROR_OUT_OF_MEMORY;
        }

        strncpy (m_pszName, pszStart, stLength);
        m_pszName[stLength] = '\0';

        if (*m_pszName == '\0') {
            *pstrErrorMessage = "The page source didn't have a valid name";
            return ERROR_FAILURE;
        }
    }

    m_bIsDefault = strcmp (m_pszName, "Default") == 0;


    //
    // Error files
    //

    char pszErrorFile [OS::MaxFileNameLength];

    // 401File
    if (m_pcfConfig->GetParameter ("401File", &pszRhs) == OK && pszRhs == NULL) {

        *pstrErrorMessage = "The 401File value could not be read";
        return ERROR_FAILURE;
    
    } else {

        if (File::ResolvePath (pszRhs, pszErrorFile) == ERROR_FAILURE) {
            *pstrErrorMessage = (String) "Error: The 401File value was invalid:" + pszRhs;
            return ERROR_FAILURE;
        }

        m_ppCachedFile[HTTP_401] = CachedFile::CreateInstance();

        if (m_ppCachedFile[HTTP_401]->Open (pszErrorFile) != OK) {
            m_ppCachedFile[HTTP_401]->Release();
            m_ppCachedFile[HTTP_401] = NULL;
            *pstrErrorMessage = (String) "The 401File " + pszErrorFile + " could not be opened";
            return ERROR_FAILURE;
        }
    }

    // 403File
    if (m_pcfConfig->GetParameter ("403File", &pszRhs) == OK && pszRhs == NULL) {

        *pstrErrorMessage = "The 403File value could not be read";
        return ERROR_FAILURE;
    
    } else {

        if (File::ResolvePath (pszRhs, pszErrorFile) == ERROR_FAILURE) {
            *pstrErrorMessage = (String) "Error: The 403File value was invalid:" + pszRhs;
            return ERROR_FAILURE;
        }

        m_ppCachedFile[HTTP_403] = CachedFile::CreateInstance();

        if (m_ppCachedFile[HTTP_403]->Open (pszErrorFile) != OK) {
            m_ppCachedFile[HTTP_403]->Release();
            m_ppCachedFile[HTTP_403] = NULL;
            *pstrErrorMessage = (String) "The 403File " + pszErrorFile + " could not be opened";
            return ERROR_FAILURE;
        }
    }

    // 404File
    if (m_pcfConfig->GetParameter ("404File", &pszRhs) == OK && pszRhs == NULL) {

        *pstrErrorMessage = "The 404File value could not be read";
        return ERROR_FAILURE;
    
    } else {

        if (File::ResolvePath (pszRhs, pszErrorFile) == ERROR_FAILURE) {
            *pstrErrorMessage = (String) "Error: The 404File value was invalid:" + pszRhs;
            return ERROR_FAILURE;
        }

        m_ppCachedFile[HTTP_404] = CachedFile::CreateInstance();

        if (m_ppCachedFile[HTTP_404]->Open (pszErrorFile) != OK) {
            m_ppCachedFile[HTTP_404]->Release();
            m_ppCachedFile[HTTP_404] = NULL;
            *pstrErrorMessage = (String) "The 404File " + pszErrorFile + " could not be opened";
            return ERROR_FAILURE;
        }
    }

    // 500File
    if (m_pcfConfig->GetParameter ("500File", &pszRhs) == OK && pszRhs == NULL) {

        *pstrErrorMessage = "The 500File value could not be read";
        return ERROR_FAILURE;
    
    } else {

        if (File::ResolvePath (pszRhs, pszErrorFile) == ERROR_FAILURE) {
            *pstrErrorMessage = (String) "Error: The 500File value was invalid:" + pszRhs;
            return ERROR_FAILURE;
        }

        m_ppCachedFile[HTTP_500] = CachedFile::CreateInstance();

        if (m_ppCachedFile[HTTP_500]->Open (pszErrorFile) != OK) {
            m_ppCachedFile[HTTP_500]->Release();
            m_ppCachedFile[HTTP_500] = NULL;
            *pstrErrorMessage = (String) "The 500File " + pszErrorFile + " could not be opened";
            return ERROR_FAILURE;
        }
    }

    // 501File
    if (m_pcfConfig->GetParameter ("501File", &pszRhs) == OK && pszRhs == NULL) {

        *pstrErrorMessage = "The 501File value could not be read";
        return ERROR_FAILURE;
    
    } else {

        if (File::ResolvePath (pszRhs, pszErrorFile) == ERROR_FAILURE) {
            *pstrErrorMessage = (String) "Error: The 501File value was invalid:" + pszRhs;
            return ERROR_FAILURE;
        }

        m_ppCachedFile[HTTP_501] = CachedFile::CreateInstance();

        if (m_ppCachedFile[HTTP_501]->Open (pszErrorFile) != OK) {
            m_ppCachedFile[HTTP_501]->Release();
            m_ppCachedFile[HTTP_501] = NULL;
            *pstrErrorMessage = (String) "The 501File " + pszErrorFile + " could not be opened";
            return ERROR_FAILURE;
        }
    }
        
    // Override401
    if (m_pcfConfig->GetParameter ("Override401", &pszRhs) == OK && pszRhs != NULL) {
        m_pbOverrideError[HTTP_401] = atoi (pszRhs) != 0;
    } else {
        *pstrErrorMessage = "The Override401 value could not be read";
        return ERROR_FAILURE;
    }

    // Override403
    if (m_pcfConfig->GetParameter ("Override403", &pszRhs) == OK && pszRhs != NULL) {
        m_pbOverrideError[HTTP_403] = atoi (pszRhs) != 0;
    } else {
        *pstrErrorMessage = "The Override403 value could not be read";
        return ERROR_FAILURE;
    }

    // Override404
    if (m_pcfConfig->GetParameter ("Override404", &pszRhs) == OK && pszRhs != NULL) {
        m_pbOverrideError[HTTP_404] = atoi (pszRhs) != 0;
    } else {
        *pstrErrorMessage = "The Override404 value could not be read";
        return ERROR_FAILURE;
    }

    // Override500
    if (m_pcfConfig->GetParameter ("Override500", &pszRhs) == OK && pszRhs != NULL) {
        m_pbOverrideError[HTTP_500] = atoi (pszRhs) != 0;
    } else {
        *pstrErrorMessage = "The Override500 value could not be read";
        return ERROR_FAILURE;
    }

    // AllowDirectoryBrowsing
    if (m_pcfConfig->GetParameter ("AllowDirectoryBrowsing", &pszRhs) == OK && pszRhs != NULL) {
        m_bBrowsingAllowed = atoi (pszRhs) != 0;
    } else {
        *pstrErrorMessage = "The AllowDirectoryBrowsing value could not be read";
        return ERROR_FAILURE;
    }

    // BasePath
    if (m_pcfConfig->GetParameter ("BasePath", &pszRhs) == OK && pszRhs != NULL) {

        if (File::ResolvePath (pszRhs, m_pszBasePath) == ERROR_FAILURE) {
            *pstrErrorMessage = (String) "Error: The BasePath value was invalid:" + pszRhs;
            return ERROR_FAILURE;
        }

        if (!File::DoesDirectoryExist (m_pszBasePath) && File::CreateDirectory (m_pszBasePath) != OK) {
            *pstrErrorMessage = (String) "Error: The BasePath directory " + m_pszBasePath + 
                " could not be created";
            return ERROR_FAILURE;
        }

    } else {
        *pstrErrorMessage = "The BasePath value could not be read";
        return ERROR_FAILURE;
    }

    // Add terminating slash
    if (m_pszBasePath [strlen (m_pszBasePath) - 1] != '/') {
        strcat (m_pszBasePath, "/");
    }
    
    // DefaultFile
    if (m_pcfConfig->GetParameter ("DefaultFile", &pszRhs) == OK && pszRhs != NULL) {
        m_pszDefaultFile = String::StrDup (pszRhs);
        if (m_pszDefaultFile == NULL) {
            *pstrErrorMessage = "The server is out of memory";
            return ERROR_OUT_OF_MEMORY;
        }
    } else {
        *pstrErrorMessage = "The DefaultFile value could not be read";
        return ERROR_FAILURE;
    }

    // UseDefaultFile
    if (m_pcfConfig->GetParameter ("UseDefaultFile", &pszRhs) == OK && pszRhs != NULL) {
        m_bDefaultFile = atoi (pszRhs) != 0;
    } else {
        *pstrErrorMessage = "The UseDefaultFile value could not be read";
        return ERROR_FAILURE;
    }
        
    // UseSSI
    /*
    if (m_pcfConfig->GetParameter ("UseSSI", &pszRhs) == OK && pszRhs != NULL) {
        
        m_bUseSSI = atoi (pszRhs) != 0;

    } else {
        *pstrErrorMessage = "The UseSSI value could not be read";
        return ERROR_FAILURE;
    }
    */

    // Set up counter file
    sprintf (pszErrorFile, "%s/%s.counters", m_pHttpServer->GetCounterPath(), m_pszName);

    iErrCode = m_cfCounterFile.Open (pszErrorFile);
    if (iErrCode != OK && iErrCode != WARNING) {
        *pstrErrorMessage = "Error: Could not open the counter file";
        return ERROR_FAILURE;
    }
    iErrCode = OK;
    
    // UseLogging
    if (m_pcfConfig->GetParameter ("UseLogging", &pszRhs) == OK && pszRhs != NULL) {
        m_bUseLogging = atoi (pszRhs) != 0;
    } else {
        *pstrErrorMessage = "The UseLogging value could not be read";
        return ERROR_FAILURE;
    }

    if (m_bUseLogging) {
        
        // UseCommonLogFormat
        if (m_pcfConfig->GetParameter ("UseCommonLogFormat", &pszRhs) == OK && pszRhs != NULL) {
            m_bUseCommonLogFormat = atoi (pszRhs) != 0;
        } else {
            *pstrErrorMessage = "The UseCommonLogFormat value could not be read";
            return ERROR_FAILURE;
        }
    }
        
    // OnlyAllowIPAddressAccess
    if (m_pcfConfig->GetParameter ("OnlyAllowIPAddressAccess", &pszRhs) != OK) {
        *pstrErrorMessage = "The OnlyAllowIPAddressAccess value could not be read";
        return ERROR_FAILURE;
    }
    if (!String::IsBlank (pszRhs)) {

        iErrCode = SetAccess (&m_llAllowIPAddressList, pszRhs);
        if (iErrCode != OK) {
            *pstrErrorMessage = "The OnlyAllowIPAddressAccess list could not be processed";
            return iErrCode;
        }

    } else {
        
        // DenyIPAddressAccess
        if (m_pcfConfig->GetParameter ("DenyIPAddressAccess", &pszRhs) != OK) {
            *pstrErrorMessage = "The DenyIPAddressAccess value could not be read";
            return ERROR_FAILURE;
        }
        if (!String::IsBlank (pszRhs)) {

            iErrCode = SetAccess (&m_llDenyIPAddressList, pszRhs);
            if (iErrCode != OK) {
                *pstrErrorMessage = "The DenyIPAddressAccess list could not be processed";
                return iErrCode;
            }
        }
    }
        
    // OnlyAllowUserAgentAccess
    if (m_pcfConfig->GetParameter ("OnlyAllowUserAgentAccess", &pszRhs) != OK) {
        *pstrErrorMessage = "The OnlyAllowUserAgentAccess value could not be read";
        return ERROR_FAILURE;
    }
    if (!String::IsBlank (pszRhs)) {

        iErrCode = SetAccess (&m_llAllowUserAgentList, pszRhs);
        if (iErrCode != OK) {
            *pstrErrorMessage = "The OnlyAllowUserAgentAccess list could not be processed";
            return iErrCode;
        }

    } else {
        
        // DenyUserAgentAccess
        if (m_pcfConfig->GetParameter ("DenyUserAgentAccess", &pszRhs) != OK) {
            *pstrErrorMessage = "The DenyUserAgentAccess value could not be read";
            return ERROR_FAILURE;
        }
        if (!String::IsBlank (pszRhs)) {
            
            iErrCode = SetAccess (&m_llDenyUserAgentList, pszRhs);
            if (iErrCode != OK) {
                *pstrErrorMessage = "The DenyUserAgentAccess list could not be processed";
                return iErrCode;
            }
        }
    }

    // FilterGets
    iErrCode = m_pcfConfig->GetParameter ("FilterGets", &pszRhs);
    if (iErrCode != OK) {
        *pstrErrorMessage = "Could not read the FilterGets value from the configuration file";
        return iErrCode;
    }
    m_bFilterGets = atoi (pszRhs) != 0;

    if (m_bFilterGets) {

        // FilterGetExtensions
        iErrCode = m_pcfConfig->GetParameter ("FilterGetExtensions", &pszRhs);
        if (iErrCode != OK) {
            *pstrErrorMessage = "Could not read the FilterGetExtensions value from the configuration file";
            return iErrCode;
        }

        if (!String::IsBlank (pszRhs)) {

            iErrCode = SetAccess (&m_llDenyGetExts, pszRhs);
            if (iErrCode != OK) {
                *pstrErrorMessage = "The FilterGetExtensions list could not be processed";
                return iErrCode;
            }
        }

        // FilterGetExtensions
        iErrCode = m_pcfConfig->GetParameter ("FilterGetAllowReferers", &pszRhs);
        if (iErrCode != OK) {
            *pstrErrorMessage = "Could not read the FilterGetAllowReferers value from the configuration file";
            return iErrCode;
        }

        if (!String::IsBlank (pszRhs)) {

            iErrCode = SetAccess (&m_llAllowReferer, pszRhs);
            if (iErrCode != OK) {
                *pstrErrorMessage = "The FilterGetAllowReferers list could not be processed";
                return iErrCode;
            }
        }
    }
        
    // UsePageSourceLibrary
    if (m_pcfConfig->GetParameter ("UsePageSourceLibrary", &pszRhs) == OK && pszRhs != NULL) {
        m_bUsePageSourceLibrary = atoi (pszRhs) != 0;
    } else {
        *pstrErrorMessage = "The UsePageSourceLibrary value could not be read";
        return ERROR_FAILURE;
    }
        
    if (m_bUsePageSourceLibrary) {
        
        // OverrideGet
        if (m_pcfConfig->GetParameter ("OverrideGet", &pszRhs) == OK && pszRhs != NULL) {
            m_bOverrideGet = atoi (pszRhs) != 0;
        } else {
            *pstrErrorMessage = "The OverrideGet value could not be read";
            return ERROR_FAILURE;
        }
        
        // OverridePost
        if (m_pcfConfig->GetParameter ("OverridePost", &pszRhs) == OK && pszRhs != NULL) {
            m_bOverridePost = atoi (pszRhs) != 0;
        } else {
            *pstrErrorMessage = "The OverridePost value could not be read";
            return ERROR_FAILURE;
        }
        
        // UseAuthentication
        if (m_pcfConfig->GetParameter ("UseAuthentication", &pszRhs) == OK && pszRhs != NULL) {

            if (_stricmp (pszRhs, "basic") == 0) {
                m_atAuthType = AUTH_BASIC;
            } else if (_stricmp (pszRhs, "digest") == 0) {
                m_atAuthType = AUTH_DIGEST;
            } else if (_stricmp (pszRhs, "none") == 0) {
                m_atAuthType = AUTH_NONE;
            } else {
                *pstrErrorMessage = "The UseAuthentication value is invalid";
                return ERROR_FAILURE;
            }

        } else {
            *pstrErrorMessage = "The UseAuthentication value could not be read";
            return ERROR_FAILURE;
        }

        if (m_atAuthType == AUTH_DIGEST) {

            if (m_pcfConfig->GetParameter ("DigestAuthenticationNonceLifetime", &pszRhs) == OK && pszRhs != NULL) {
                
                m_iDigestNonceLifetime = atoi (pszRhs);
                if (m_iDigestNonceLifetime < 1) {
                    *pstrErrorMessage = "The DigestAuthenticationNonceLifetime value is invalid";
                    return ERROR_FAILURE;
                }

            } else {
                *pstrErrorMessage = "The DigestAuthenticationNonceLifetime value could not be read";
                return ERROR_FAILURE;
            }
        }

        // Library
        char* pszLibrary;
        if (m_pcfConfig->GetParameter ("PageSourceLibrary", &pszLibrary) != OK || pszLibrary == NULL) {
            *pstrErrorMessage = "The PageSourceLibrary value could not be read";
            return ERROR_FAILURE;
        }
        
        char* pszClsid;
        if (m_pcfConfig->GetParameter ("PageSourceClsid", &pszClsid) != OK || pszClsid == NULL) {
            *pstrErrorMessage = "The PageSourceClsid value could not be read";
            return ERROR_FAILURE;
        }

        // Load library
        size_t stPageSourcePathLen = strlen (m_pHttpServer->GetPageSourcePath());
        char* pszLibName = (char*) StackAlloc (stPageSourcePathLen + strlen (pszLibrary) + 2);

        strcpy (pszLibName, m_pHttpServer->GetPageSourcePath());
        strcat (pszLibName, "/");
        strcat (pszLibName, pszLibrary);
                    
        iErrCode = Initialize (pszLibName, pszClsid);

        if (iErrCode != OK) {
            *pstrErrorMessage = "The PageSource did not initialize properly.";
            return iErrCode;
        }
    }

    return iErrCode;
}

int PageSource::RestartPageSource (void* pVoid) {

    PageSource* pThis = (PageSource*) pVoid;

    // Take the lock
    pThis->m_mShutdownLock.Wait();

    if (!pThis->m_bShutdown) {
        
        // Wait until we have zero callers
        while (pThis->m_iNumThreads > 0) {
            OS::Sleep (100);
        }

        // Reencarnation
        pThis->Clean();     // Delete everything stateful
        pThis->Reset();     // Set everything to original null state
        
        pThis->m_pcfConfig->Close();
        
        String strErrorMessage;
        int iErrCode = pThis->Configure (NULL, &strErrorMessage);
        
        if (iErrCode != OK) {
            pThis->m_bScrewed = true;
            ITraceLog* pReport = pThis->GetReport();
            pReport->Write(TRACE_ERROR, (String)"Error restarting page source. " + strErrorMessage);
            SafeRelease(pReport);
        } else {
            pThis->m_bScrewed = false;
        }
    }
    
    // Not restarting anymore
    pThis->m_bRestart = false;

    pThis->m_mShutdownLock.Signal();
    
    // If the restart failed, we just have to wait for another call to Restart()
    return OK;
}

int PageSource::Shutdown() {

    if (_stricmp (m_pszName, "Default") == 0 ||
        _stricmp (m_pszName, "Admin") == 0) {
        return ERROR_FAILURE;
    }

    m_mShutdownLock.Wait();

    if (m_bShutdown) {
        m_mShutdownLock.Signal();
        return ERROR_FAILURE;
    }

    m_bShutdown = true;
    m_bRestart = false;

    m_mShutdownLock.Signal();

    return m_pHttpServer->DeletePageSource (m_pszName);
}

void PageSource::LockWithNoThreads() {

    m_bLocked = true;
    m_mShutdownLock.Wait();
    m_mLock.Wait();

    // Spin until no threads
    while (m_iNumThreads > 0) {
        OS::Sleep (100);
    }

    m_mShutdownLock.Signal();
}

void PageSource::LockWithSingleThread() {

    m_bLocked = true;
    m_mShutdownLock.Wait();
    m_mLock.Wait();

    // Spin until 1 thread
    while (m_iNumThreads > 1) {
        OS::Sleep (100);
    }

    m_mShutdownLock.Signal();
}

void PageSource::ReleaseLock() {

    m_mLock.Signal();
    m_bLocked = false;
}

unsigned int PageSource::IncrementCounter (const char* pszName) {

    unsigned int iNewValue;
    char* pszOldValue, pszNewValue [256];

    // Lock
    m_mCounterLock.WaitWriter();

    // Read old value
    if (m_cfCounterFile.GetParameter (pszName, &pszOldValue) == OK && pszOldValue != NULL) {
        iNewValue = atoi (pszOldValue) + 1;
    } else {
        iNewValue = 1;
    }

    // Assign new value
    m_cfCounterFile.SetParameter (pszName, _itoa (iNewValue + 1, pszNewValue, 10));

    // Unlock
    m_mCounterLock.SignalWriter();

    return iNewValue;
}

unsigned int PageSource::GetCounterValue (const char* pszName) {

    int iErrCode;
    unsigned int iValue;
    char* pszValue;

    // Read value
    m_mCounterLock.WaitReader();
    iErrCode = m_cfCounterFile.GetParameter (pszName, &pszValue);
    m_mCounterLock.SignalReader();

    if (iErrCode == OK && pszValue != NULL) {
        iValue = atoi (pszValue);
    } else {
        iValue = 0;
    }

    return iValue;
}

void PageSource::GetReportFileName (char pszFileName[OS::MaxFileNameLength]) {
    
    int iSec, iMin, iHour, iDay, iMonth, iYear;
    char pszMonth[20], pszDay[20];
    DayOfWeek day;

    Time::GetDate (m_tReportTime, &iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);

    sprintf (pszFileName, "%s/%s_%i_%s_%s.report", m_pHttpServer->GetReportPath(), m_pszName, iYear, 
        String::ItoA (iMonth, pszMonth, 10, 2), String::ItoA (iDay, pszDay, 10, 2));
}

void PageSource::GetLogFileName (char pszFileName[OS::MaxFileNameLength]) {

    int iSec, iMin, iHour, iDay, iMonth, iYear;
    char pszMonth[20], pszDay[20];
    DayOfWeek day;

    Time::GetDate (m_tLogTime, &iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);

    sprintf (pszFileName, "%s/%s_%i_%s_%s.log", m_pHttpServer->GetLogPath(), m_pszName, iYear, 
        String::ItoA (iMonth, pszMonth, 10, 2), String::ItoA (iDay, pszDay, 10, 2));
}