// PageSource.cpp: implementation of the PageSource class.
//
//////////////////////////////////////////////////////////////////////
//
// HttpObjects.dll:  a component of Alajar 1.0
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#define ON_FINALIZE ((HttpResponse*) 0xfffffff)


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

    m_bRestart = false;
    m_bScrewed = false;
    m_bShutdown = false;
    m_bIsDefault = false;

    m_iNumRefs = 1; // AddRef

    m_iNumThreads = 0;

    m_bFilterGets = false;

    Reset();
}

int PageSource::Init() {

    int iErrCode;

    m_pcfConfig = Config::CreateInstance();
    if (m_pcfConfig == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    iErrCode = m_mLogMutex.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mReportMutex.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
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

    return OK;
}

void PageSource::Reset() {

    m_bLocked = false;

    m_bBrowsingAllowed = false;
    m_bDefaultFile = false;
    m_bBasicAuthentication = false;
    m_bOverrideGet = false;
    m_bOverridePost = false;
    m_bIsDefault = false;

    memset (m_pbOverrideError, 0, sizeof (m_pbOverrideError));
    memset (m_ppCachedFile, 0, sizeof (m_ppCachedFile));

    m_bUseSSI = false;
    m_bUsePageSourceLibrary = false;
    m_bUseLogging = false;
    m_bUseCommonLogFormat = false;

    m_pszDefaultFile = NULL;

    m_pPageSource = NULL;

    m_bFilterGets = false;

    m_bSingleThreaded = false;

    m_ptsfqResponseQueue = NULL;
    m_pSTAThread = NULL;
    m_pSTAEvent = NULL;

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
        m_pPageSource->Release();
    }

    if (m_bSingleThreaded) {

        // Thread has already exited
        if (m_pSTAEvent != NULL) {
            delete m_pSTAEvent;
        }

        if (m_pSTAThread != NULL) {
            delete m_pSTAThread;
        }

        if (m_ptsfqResponseQueue != NULL) {
            delete m_ptsfqResponseQueue;
        }
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
    m_fLogFile.Close();
    m_fReportFile.Close();
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
    if (iidInterface == IID_ILog) {                             
        *ppInterface = (void*) static_cast<ILog*> (this);           
        AddRef();                                                       
        return OK;                                                      
    }
    if (iidInterface == IID_IReport) {                              
        *ppInterface = (void*) static_cast<IReport*> (this);            
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

    // Initialize PageSource
    return OnInitialize (m_pHttpServer, this);
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

IReport* PageSource::GetReport() {
    return this;
}

ILog* PageSource::GetLog() {
    return this;
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

bool PageSource::UseBasicAuthentication() {
    return m_bBasicAuthentication;
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

bool PageSource::IsSingleThreaded() {
    return m_bSingleThreaded;
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

    if (m_bSingleThreaded) {
        return false;
    }
    
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

    if (!m_bSingleThreaded) {

        if (bLocked) {
            m_mLock.Signal();
        }
        Algorithm::AtomicDecrement (&m_iNumThreads);
    }
}

// IPageSource
int PageSource::OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pControl) {
    
    if (m_bSingleThreaded) {

        // Start up the STA thread
        int iErrCode = m_pSTAThread->Start (ThreadSTALoop, this, Thread::NormalPriority);
        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    return m_pPageSource->OnInitialize (pHttpServer, pControl);
}

int PageSource::OnFinalize() {

    if (m_pPageSource == NULL) {
        return OK;
    }

    if (m_bSingleThreaded) {

        // Post finalization message and wait for thread to exit
        int iErrCode = QueueResponse (ON_FINALIZE);
        if (iErrCode != OK) {
            return iErrCode;
        }

        m_pSTAThread->WaitForTermination();

        return OK;
    }

    int iErrCode = m_pPageSource->OnFinalize();

    // Release the page source
    m_pPageSource->Release();
    m_pPageSource = NULL;

    return iErrCode;
}

int PageSource::OnBasicAuthenticate (const char* pszLogin, const char* pszPassword, bool* pbAuthenticated) {

    bool bLocked = Enter();
    int iErrCode = m_pPageSource->OnBasicAuthenticate (pszLogin, pszPassword, pbAuthenticated);

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

    // 401File
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
    if (m_pcfConfig->GetParameter ("UseSSI", &pszRhs) == OK && pszRhs != NULL) {
        
        m_bUseSSI = atoi (pszRhs) != 0;

    } else {
        *pstrErrorMessage = "The UseDefaultFile value could not be read";
        return ERROR_FAILURE;
    }

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
        
    // PageSourceLibrary, PageSourceClsid
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
        
        // UseBasicAuthentication
        if (m_pcfConfig->GetParameter ("UseBasicAuthentication", &pszRhs) == OK && pszRhs != NULL) {
            m_bBasicAuthentication = atoi (pszRhs) != 0;
        } else {
            *pstrErrorMessage = "The UseBasicAuthentication value could not be read";
            return ERROR_FAILURE;
        }
        
        // Threading model      
        if (m_pcfConfig->GetParameter ("Threading", &pszRhs) == OK && pszRhs != NULL) {
            
            if (stricmp (pszRhs, "Free") == 0) {
                m_bSingleThreaded = false;
            }
            else if (stricmp (pszRhs, "Single") == 0) {
                m_bSingleThreaded = true;
            }
            else {
                *pstrErrorMessage = "The Threading value was invalid";
                return ERROR_FAILURE;
            }
        } else {
            *pstrErrorMessage = "The Threading value could not be read";
            return ERROR_FAILURE;
        }

        if (m_bSingleThreaded) {

            m_ptsfqResponseQueue = new ThreadSafeFifoQueue<HttpResponse*>();
            m_pSTAThread = new Thread();
            m_pSTAEvent = new Event();

            if (m_ptsfqResponseQueue == NULL || m_pSTAThread == NULL || m_pSTAEvent == NULL) {
                *pstrErrorMessage = "The server is out of memory";
                return ERROR_OUT_OF_MEMORY;
            }

            if (!m_ptsfqResponseQueue->Initialize()) {
                *pstrErrorMessage = "The server is out of memory";
                return ERROR_OUT_OF_MEMORY;
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
            pThis->WriteReport ((String) "Error restarting page source. " + strErrorMessage);
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

    if (stricmp (m_pszName, "Default") == 0 ||
        stricmp (m_pszName, "Admin") == 0) {
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

    if (m_bSingleThreaded) {
        return;
    }

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

    if (m_bSingleThreaded) {
        return;
    }

    m_bLocked = true;
    m_mShutdownLock.Wait();
    m_mLock.Wait();

    // Spin until 1 thread
    while (m_iNumThreads > 1) {
        OS::Sleep (200);
    }

    m_mShutdownLock.Signal();
}

void PageSource::ReleaseLock() {

    if (m_bSingleThreaded) {
        return;
    }

    m_mLock.Signal();
    m_bLocked = false;
}

int PageSource::QueueResponse (HttpResponse* pHttpResponse) {

    // Entrust request, response pair to STA thread
    m_ptsfqResponseQueue->Push (pHttpResponse);
    m_pSTAEvent->Signal();

    return OK;
}

HttpResponse* PageSource::DeQueueResponse() {

    HttpResponse* pHttpResponse;
    return m_ptsfqResponseQueue->Pop (&pHttpResponse) ? pHttpResponse : NULL;
}

int PageSource::ThreadSTALoop (void* pVoid) {

    return ((PageSource*) pVoid)->STALoop();
}

int PageSource::STALoop() {

    int iErrCode;

    HttpResponse* pHttpResponse;
    HttpRequest* pHttpRequest;
    Socket* pSocket;

    Timer tmTimer;
    Time::StartTimer (&tmTimer);

    HttpServerStatistics sThreadStats;
    memset (&sThreadStats, 0, sizeof (HttpServerStatistics));

    bool bStay = true;

    // Loop forever
    while (bStay) {

        // Wait on event
        m_pSTAEvent->Wait();

        while (true) {
            
            // Look for work
            pHttpResponse = DeQueueResponse();
            if (pHttpResponse == NULL) {
                break;
            }
            
            else if (pHttpResponse == ON_FINALIZE) {
                iErrCode = m_pPageSource->OnFinalize();
                bStay = false;
                break;
            }
            
            else {
                
                pHttpRequest = pHttpResponse->GetHttpRequest();
                pSocket = pHttpResponse->GetSocket();
                
                // Send response
                iErrCode = pHttpResponse->RespondPrivate();
                
                // End request
                m_pHttpServer->FinishRequest (pHttpRequest, pHttpResponse, pSocket, &sThreadStats, iErrCode);

                if (sThreadStats.NumRequests % COALESCE_REQUESTS == 0 || 
                    Time::GetTimerCount (tmTimer) > COALESCE_PERIOD_MS) {
                    
                    m_pHttpServer->CoalesceStatistics (&sThreadStats);
                    Time::StartTimer (&tmTimer);
                }
            }
        }
    }

    m_pHttpServer->CoalesceStatistics (&sThreadStats);

    return OK;
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
    m_cfCounterFile.SetParameter (pszName, itoa (iNewValue + 1, pszNewValue, 10));

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

    Time::GetDate (m_tReportTime, &iSec, &iMin, &iHour, &iDay, &iMonth, &iYear);

    sprintf (pszFileName, "%s/%s_%i_%s_%s.report", m_pHttpServer->GetReportPath(), m_pszName, iYear, 
        String::ItoA (iMonth, pszMonth, 10, 2), String::ItoA (iDay, pszDay, 10, 2));
}

void PageSource::GetLogFileName (char pszFileName[OS::MaxFileNameLength]) {

    int iSec, iMin, iHour, iDay, iMonth, iYear;
    char pszMonth[20], pszDay[20];

    Time::GetDate (m_tLogTime, &iSec, &iMin, &iHour, &iDay, &iMonth, &iYear);

    sprintf (pszFileName, "%s/%s_%i_%s_%s.log", m_pHttpServer->GetLogPath(), m_pszName, iYear, 
        String::ItoA (iMonth, pszMonth, 10, 2), String::ItoA (iDay, pszDay, 10, 2));
}

void PageSource::LogMessage (const char* pszMessage) {

    // All log file operations are best effort
    UTCTime tNow;
    Time::GetTime (&tNow);

    m_mLogMutex.Wait();

    if (HttpServer::DifferentDays (m_tLogTime, tNow)) {

        m_tLogTime = tNow;

        char pszFileName [OS::MaxFileNameLength];
        GetLogFileName (pszFileName);

        m_fLogFile.Close();
        m_fLogFile.OpenAppend (pszFileName);
    }

    if (m_fLogFile.IsOpen()) {
        m_fLogFile.Write (pszMessage);
        m_fLogFile.WriteEndLine();
    }

    m_mLogMutex.Signal();
}

void PageSource::ReportMessage (const char* pszMessage) {

    // All log file operations are best effort
    UTCTime tNow;
    Time::GetTime (&tNow);

    m_mReportMutex.Wait();

    if (HttpServer::DifferentDays (m_tReportTime, tNow)) {

        m_tReportTime = tNow;

        char pszFileName [OS::MaxFileNameLength];
        GetReportFileName (pszFileName);

        m_fReportFile.Close();
        m_fReportFile.OpenWrite (pszFileName);
    }

    m_fReportFile.Write (pszMessage);
    m_fReportFile.WriteEndLine();

    m_mReportMutex.Signal();
}


size_t PageSource::GetFileTail (File& fFile, Mutex& mMutex, const UTCTime& tTime, char* pszBuffer, 
                                size_t stNumChars) {

    UTCTime tZero;
    Time::ZeroTime (&tZero);

    size_t stFilePtr;

    *pszBuffer = '\0';

    if (tTime == tZero) {
        return 0;
    }

    size_t stRetVal = 0;

    mMutex.Wait();

    if (fFile.GetFilePointer (&stFilePtr) == OK) {

        if (stNumChars > stFilePtr) {
            stNumChars = stFilePtr;
        }

        if (fFile.SetFilePointer (stFilePtr - stNumChars) == OK) {

            if (fFile.Read (pszBuffer, stNumChars, &stRetVal) == OK) {
                pszBuffer[stRetVal] = '\0';
            }

            fFile.SetFilePointer (stFilePtr);
        }
    }

    mMutex.Signal();

    return stRetVal;
}


// ILog
size_t PageSource::GetLogTail (char* pszBuffer, size_t stNumChars) {

    return GetFileTail (m_fLogFile, m_mLogMutex, m_tLogTime, pszBuffer, stNumChars);
}

// IReport
int PageSource::WriteReport (const char* pszMessage) {

    size_t stLength = String::StrLen (pszMessage);

    if (stLength > MAX_LOG_MESSAGE) {
        return ERROR_INVALID_ARGUMENT;
    }

    ::LogMessage* plmMessage = m_pHttpServer->GetLogMessage();
    if (plmMessage == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    plmMessage->lmtMessageType = REPORT_MESSAGE;
    plmMessage->pPageSource = this;
    strncpy (plmMessage->pszText, pszMessage, stLength + 1);

    AddRef();

    int iErrCode = m_pHttpServer->PostMessage (plmMessage);
    if (iErrCode != OK) {

        Release();
        m_pHttpServer->FreeLogMessage (plmMessage);
    }

    return iErrCode;
}

size_t PageSource::GetReportTail (char* pszBuffer, size_t stNumChars) {

    return GetFileTail (m_fReportFile, m_mReportMutex, m_tReportTime, pszBuffer, stNumChars);
}