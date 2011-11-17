// PageSource.h: interface for the PageSource class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar.dll
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

#pragma once

#include "Config.h"
#include "CachedFile.h"
#include "Report.h"

#include "Osal/Thread.h"
#include "Osal/LinkedList.h"
#include "Osal/Library.h"
#include "Osal/FifoQueue.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/Event.h"
#include "Osal/File.h"
#include "Osal/AsyncFile.h"

typedef int (*Fxn_CreateInstance) (const Uuid&, const Uuid&, void**);

typedef enum HttpAuthenticationType {
    AUTH_BASIC,
    AUTH_DIGEST,
    AUTH_NONE
};

class HttpServer;
class HttpRequest;
class HttpResponse;
class PageSource;

class ReportWrapper : public ITraceLog, public ITraceLogReader
{
private:
    PageSource* m_pPageSource;

    IMPLEMENT_TWO_INTERFACES(ITraceLog, ITraceLogReader);

public:
    ReportWrapper(PageSource* pPageSource);
    virtual int Write(TraceInfoLevel level, const char* pszMessage);
    virtual int GetTail(char* pszBuffer, unsigned int cbSize);
};

class PageSource : public IPageSourceControl, public IPageSource
{
private:

    unsigned int m_iNumRefs;
    unsigned int m_iNumThreads;

    // Queued for restart?
    bool m_bRestart;
    bool m_bScrewed;    // True if a restart failed
    bool m_bShutdown;

    bool m_bIsDefault;

    // Name of page source
    char* m_pszName;

    HttpServer* m_pHttpServer;

    char m_pszBasePath [OS::MaxFileNameLength];

    char* m_pszDefaultFile;
    char* m_pszConfigFileName;

    ConfigFile m_cfCounterFile;

    Report* m_pLog;
    Report* m_pReport;
    ReportWrapper* m_pReportWrapper;

    UTCTime m_tLogTime;
    UTCTime m_tReportTime;

    TraceInfoLevel m_reportTracelevel;

    // Configuration data
    bool m_bBrowsingAllowed;
    bool m_bDefaultFile;
    bool m_bUsePageSourceLibrary;
    bool m_bUseLogging;
    bool m_bUseCommonLogFormat;

    bool m_bOverrideGet;
    bool m_bOverridePost;

    HttpAuthenticationType m_atAuthType;
    Seconds m_iDigestNonceLifetime;

    bool m_pbOverrideError [NUM_STATUS_CODES];
    CachedFile* m_ppCachedFile [NUM_STATUS_CODES];

    // Page source lib
    Library m_libPageSource;
    IPageSource* m_pPageSource;

    class AccessControlElement {
    private:
        AccessControlElement& operator=(AccessControlElement& aceElement);

    public:

        AccessControlElement();
        ~AccessControlElement();

        int Initialize (const char* pszName, bool bWildCard);

        char* pszString;
        size_t stLength;
        bool bHasWildCard;
    };

    // Access Control Lists
    typedef LinkedList<AccessControlElement*> ACEList;

    ACEList m_llAllowIPAddressList;
    ACEList m_llDenyIPAddressList;

    ACEList m_llAllowUserAgentList;
    ACEList m_llDenyUserAgentList;

    int SetAccess (ACEList* pllAccessList, const char* pszAccessList);
    bool IsAllowedAccess (const ACEList& llAllowList, const ACEList& llDenyList, const char* pszString);
    bool IsStringInACEList (const ACEList& llList, const char* pszString);

    void ClearACEList (ACEList* pllList);

    // GET filters
    ACEList m_llDenyGetExts;
    ACEList m_llAllowReferer;

    bool m_bFilterGets;

    // Library name
    char* m_pszLibraryName;

    // Config file
    Config* m_pcfConfig;

    // Shutdown lock
    Mutex m_mShutdownLock;

    // Counter lock
    ReadWriteLock m_mCounterLock;

    // Report and log locks
    Mutex m_reportMutex;
    Mutex m_logMutex;

    // Restart call
    static int THREAD_CALL RestartPageSource (void* pVoid);

    // Concurrency
    bool m_bLocked;
    Mutex m_mLock;

    // Utilities
    void Clean();
    void Reset();

    inline bool Enter();
    inline void Exit (bool bLocked);
    
    void GetReportFileName (char pszFileName[OS::MaxFileNameLength]);
    void GetLogFileName (char pszFileName[OS::MaxFileNameLength]);

public:

    PageSource (HttpServer* pHttpServer);
    ~PageSource();

    int Init();

    bool IsRestarting();
    bool IsWorking();
    
    // Configure
    int Configure (const char* pszConfigFileName, String* pstrErrorMessage);

    int Initialize (const char* pszLibraryName, const char* pszClsid);

    // Configuration data
    const char* GetBasePath();
    const char* GetDefaultFile();

    ICachedFile* GetErrorFile (HttpStatus sStatus);

    bool AllowDirectoryBrowsing();
    bool UseDefaultFile();

    HttpAuthenticationType GetAuthenticationType();
    Seconds GetDigestAuthenticationNonceLifetime();

    const char* GetAuthenticationRealm (IHttpRequest* pHttpRequest);

    bool IsGetAllowed (HttpRequest* pHttpRequest);

    bool OverrideGet();
    bool OverridePost();

    bool OverrideError (HttpStatus sStatus);

    bool UsePageSourceLibrary();
    bool UseLogging();
    bool UseCommonLogFormat();
    ConfigFile* GetCounterFile();

    bool IsIPAddressAllowedAccess (const char* pszIPAddress);
    bool IsUserAgentAllowedAccess (const char* pszUserAgent);

    ITraceLog* GetReportInternal();

    DECLARE_IOBJECT;

    // IPageSource
    int OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pControl);
    int OnFinalize();

    int OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);
    int OnPost (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);
    int OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);

    int OnBasicAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated);
    int OnDigestAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated);

    // IPageSourceControl
    bool IsDefault();

    const char* GetName();
    IConfigFile* GetConfigFile();

    ITraceLog* GetReport();
    ITraceLog* GetLog();

    void LockWithNoThreads();
    void LockWithSingleThread();
    void ReleaseLock();

    int Restart();
    int Shutdown();

    unsigned int IncrementCounter (const char* pszName);
    unsigned int GetCounterValue (const char* pszName);
};