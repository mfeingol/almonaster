// PageSource.h: interface for the PageSource class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar.dll
// Copyright (C) 1998-1999 Max Attar Feingold (maf6@cornell.edu)
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

#if !defined(AFX_PAGESOURCE_H__1F955CC5_AD9D_11D1_9C61_0060083E8062__INCLUDED_)
#define AFX_PAGESOURCE_H__1F955CC5_AD9D_11D1_9C61_0060083E8062__INCLUDED_

#include "Config.h"
#include "CachedFile.h"

#include "Osal/Thread.h"
#include "Osal/LinkedList.h"
#include "Osal/Library.h"
#include "Osal/FifoQueue.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/Event.h"
#include "Osal/File.h"


typedef int (*Fxn_CreateInstance) (const Uuid&, const Uuid&, void**);

class HttpServer;
class HttpRequest;
class HttpResponse;

class PageSource : public IPageSourceControl, public IPageSource, public ILog, public IReport {

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

    File m_fLogFile;
    File m_fReportFile;

    Mutex m_mLogMutex;
    Mutex m_mReportMutex;

    UTCTime m_tLogTime;
    UTCTime m_tReportTime;

    // Configuration data
    bool m_bBrowsingAllowed;
    bool m_bDefaultFile;
    bool m_bBasicAuthentication;
    bool m_bUseSSI;
    bool m_bUsePageSourceLibrary;
    bool m_bUseLogging;
    bool m_bUseCommonLogFormat;

    bool m_bOverrideGet;
    bool m_bOverridePost;

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

    // Restart call
    static int THREAD_CALL RestartPageSource (void* pVoid);

    // Concurrency
    bool m_bLocked;
    Mutex m_mLock;

    // Single threaded
    bool m_bSingleThreaded;

    ThreadSafeFifoQueue<HttpResponse*> m_tsfqResponseQueue;
    Thread m_tSTAThread;
    Event m_eSTAEvent;

    // Utilities
    void Clean();
    void Reset();

    inline bool Enter();
    inline void Exit (bool bLocked);
    
    int OpenReport();
    int OpenLog();

    void GetReportFileName (char pszFileName[OS::MaxFileNameLength]);
    void GetLogFileName (char pszFileName[OS::MaxFileNameLength]);

    size_t GetFileTail (File& fFile, Mutex& mMutex, const UTCTime& tTime, char* pszBuffer, size_t stNumChars);

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
    bool UseBasicAuthentication();
    
    bool IsGetAllowed (HttpRequest* pHttpRequest);

    bool OverrideGet();
    bool OverridePost();

    bool OverrideError (HttpStatus sStatus);

    bool UsePageSourceLibrary();
    bool UseSSI();
    bool UseLogging();
    bool UseCommonLogFormat();
    ConfigFile* GetCounterFile();

    bool IsIPAddressAllowedAccess (const char* pszIPAddress);
    bool IsUserAgentAllowedAccess (const char* pszUserAgent);

    bool IsSingleThreaded();
    int QueueResponse (HttpResponse* pHttpResponse);
    HttpResponse* DeQueueResponse();

    static int THREAD_CALL ThreadSTALoop (void* pVoid);
    int STALoop();

    void ReportMessage (const char* pszMessage);
    void LogMessage (const char* pszMessage);


    DECLARE_IOBJECT;

    // IPageSource
    int OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pControl);
    int OnFinalize();

    int OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);
    int OnPost (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);

    int OnBasicAuthenticate (const char* pszLogin, const char* pszPassword, bool* pbAuthenticated);

    int OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);

    // IPageSourceControl
    bool IsDefault();

    const char* GetName();
    IConfigFile* GetConfigFile();

    IReport* GetReport();
    ILog* GetLog();

    void LockWithNoThreads();
    void LockWithSingleThread();
    void ReleaseLock();

    int Restart();
    int Shutdown();

    unsigned int IncrementCounter (const char* pszName);
    unsigned int GetCounterValue (const char* pszName);

    // ILog
    size_t GetLogTail (char* pszBuffer, size_t stNumChars);

    // IReport
    int WriteReport (const char* pszMessage);
    size_t GetReportTail (char* pszBuffer, size_t stNumChars);
};


#endif // !defined(AFX_PAGESOURCE_H__1F955CC5_AD9D_11D1_9C61_0060083E8062__INCLUDED_)