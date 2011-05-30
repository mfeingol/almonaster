// HttpServer.cpp: implementation of the HttpServer class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar 1.0:  a web server
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

#include "HttpServer.h"

#include "Osal/ConfigFile.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HttpServer::HttpServer() {

    m_iNumRefs = 1;
    m_bRunning = false;
    m_pConfigFile = NULL;

    m_pStartupSink = NULL;
    m_pShutdownSink = NULL;
    m_pRestartSink = NULL;
    m_pThreadPool = NULL;

    m_pDefaultPageSource = NULL;
    m_pPageSourceTable = NULL;

    m_pHttpRequestCache = NULL;
    m_pHttpResponseCache = NULL;
    m_pSocketCache = NULL;
    m_pLogMessageCache = NULL;

    m_pFileCache = NULL;

    m_pSocket = NULL;

    m_pszServerName = "Alajar 1.58";
    m_stServerNameLength = sizeof ("Alajar 1.58") - 1;

    Time::GetTime (&m_tLogDate);

    Initialize();

    // Initialize winsock library
    if (Socket::Initialize() != OK) {
        ReportEvent ("Could not initialize socket library!");
    }
}

HttpServer::~HttpServer() {

    Clean();

    m_tsfqLogQueue.Clear();

    m_fReportFile.Close();

    if (m_pConfigFile != NULL) {
        m_pConfigFile->Release();
    }

    if (m_pLogMessageCache != NULL) {
        delete m_pLogMessageCache;
    }

    // Close winsock library
    Socket::Finalize();
}

unsigned int HttpServer::AddRef() {

    return Algorithm::AtomicIncrement (&m_iNumRefs);
}

unsigned int HttpServer::Release() {

    unsigned int iRefCount = Algorithm::AtomicDecrement (&m_iNumRefs);
    if (iRefCount == 0) {

        if (!m_bExit && m_bRunning) {

            // Shutdown
            Shutdown();
            m_tMainThread.WaitForTermination();
        }

        delete this;
    }

    return iRefCount;
}

int HttpServer::QueryInterface (const Uuid& iidInterface, void** ppInterface) {

    if (iidInterface == IID_IHttpServer) {
        *ppInterface = static_cast<IHttpServer*> (this);
        AddRef();
        return OK;
    }

    if (iidInterface == IID_IObject) {
        *ppInterface = static_cast<IObject*> (this);
        AddRef();
        return OK;
    }

    *ppInterface = NULL;
    return ERROR_NO_INTERFACE;
}

void HttpServer::Initialize() {

    m_bExit = false;
    m_bLogExit = false;
    m_bRestart = false;
}

void HttpServer::Clean() {

    // Delete thread pool
    if (m_pThreadPool != NULL) {
        delete m_pThreadPool;
        m_pThreadPool = NULL;
    }

    // Shut down and delete page sources
    if (m_bRunning) {
        ReportEvent ("Shutting down page sources");
        ShutdownPageSources();
        ReportEvent ("Shutting down server");

        m_bRunning = false;
    }
    
    if (m_pFileCache != NULL) {
        m_pFileCache->Release();
        m_pFileCache = NULL;
    }

    // Delete pagesource table
    if (m_pPageSourceTable != NULL) {
        delete m_pPageSourceTable;
        m_pPageSourceTable = NULL;
    }

    if (m_pSocket != NULL) {
        delete m_pSocket;
        m_pSocket = NULL;
    }

    if (m_pHttpRequestCache != NULL) {
        delete m_pHttpRequestCache;
        m_pHttpRequestCache = NULL;
    }

    if (m_pHttpResponseCache != NULL) {
        delete m_pHttpResponseCache;
        m_pHttpResponseCache = NULL;
    }

    if (m_pSocketCache != NULL) {
        delete m_pSocketCache;
        m_pSocketCache = NULL;
    }
}

HttpServer* HttpServer::CreateInstance() {

    HttpServer* pHttpServer = new HttpServer();
    if (pHttpServer == NULL) {
        return NULL;
    }

    if (pHttpServer->Init() != OK) {
        delete pHttpServer;
        return NULL;
    }

    return pHttpServer;
}

int HttpServer::Init() {

    int iErrCode;

    if (!m_tsfqLogQueue.Initialize()) {
        return ERROR_OUT_OF_MEMORY;
    }

    iErrCode = m_evLogEvent.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mShutdown.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mStatMutex.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mHttpObjectCacheLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mSocketCacheLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mLogMessageCacheLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = m_mReportMutex.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_rwPageSourceTableLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    return OK;
}

PageSource* HttpServer::GetPageSource (const char* pszPageSourceName) {
    
    PageSource* pPageSource = NULL;
    
    m_rwPageSourceTableLock.WaitReader();
    bool bFound = m_pPageSourceTable->FindFirst (pszPageSourceName, &pPageSource);
    m_rwPageSourceTableLock.SignalReader();

    if (bFound) {
        pPageSource->AddRef();
        return pPageSource;
    }

    return NULL;
}


///////////////////////
// Server operations //
///////////////////////

int HttpServer::StartServer (void* pvServer) {
    
    HttpServer* pThis = (HttpServer*) pvServer;
    int iErrCode = pThis->StartServer();
    
    if (iErrCode != OK) {

        // Shut down the logging thread
        pThis->ReportEvent ("Shutting down the logging thread");
    
        pThis->m_bLogExit = true;
        pThis->m_evLogEvent.Signal();
        pThis->m_tLogThread.WaitForTermination();
        
        // Flush messages
        pThis->LogLoop();
        
        // Notify starter of problems
        if (pThis->m_pStartupSink != NULL) {
            pThis->m_pStartupSink->OnStartup (iErrCode);
            pThis->m_pStartupSink->Release();
            pThis->m_pStartupSink = NULL;
        }
    }
    
    return iErrCode;
}

int HttpServer::StartServer() {
    
    int iErrCode, iNumFiles;
    char* pszRhs;

    bool bFileCache;
    char pszReportFileName [OS::MaxFileNameLength];

    unsigned int iNumHttpObjects, iInitNumThreads, iMaxNumThreads;
    PageSource* pPageSource;

    HashTableIterator<const char*, PageSource*> htiPageSource;
    
    // First, get report
    if (m_pConfigFile->GetParameter ("ReportPath", &pszRhs) == OK && pszRhs != NULL) {
        
        if (File::ResolvePath (pszRhs, m_pszReportPath) == ERROR_FAILURE) {
            ReportEvent ((String) "Error: The ReportPath value was invalid:" + pszRhs);
            goto ErrorExit;
        }
        
        if (!File::DoesDirectoryExist (m_pszReportPath)) {

            if (File::CreateDirectory (m_pszReportPath) != OK) {
                ReportEvent ((String) "Error: Could not create ReportPath directory " + m_pszReportPath);
                goto ErrorExit;
            }
        }

    } else {
        
        ReportEvent ("Error: Could not read the ReportPath from Alajar.conf");
        goto ErrorExit;
    }
    
    // Open report file
    sprintf (pszReportFileName, "%s/Alajar.report", m_pszReportPath);

    if (m_fReportFile.OpenWrite (pszReportFileName) != OK) {
        ReportEvent ("Error: Could not open the report file");
        goto ErrorExit;
    }
        
    // Print intro screen
    ReportEvent (m_pszServerName);
    ReportEvent ("Copyright (c) 1998-2002 Max Attar Feingold");
    ReportEvent ("");
    ReportEvent ("Alajar comes with ABSOLUTELY NO WARRANTY.");
    ReportEvent ("This is free software, and you are welcome");
    ReportEvent ("to redistribute it under certain conditions.");
    ReportEvent ("View License.txt for more details.");
    ReportEvent ("");
    ReportEvent ("Initializing server");
    
    // Counter path
    if (m_pConfigFile->GetParameter ("CounterPath", &pszRhs) == OK && pszRhs != NULL) {
        
        if (File::ResolvePath (pszRhs, m_pszCounterPath) == ERROR_FAILURE) {
            ReportEvent ((String) "Error: The CounterPath value was invalid:" + pszRhs);
            goto ErrorExit;
        }
        
        if (!File::DoesDirectoryExist (m_pszCounterPath)) {
            if (File::CreateDirectory (m_pszCounterPath) != OK) {
                ReportEvent ((String) "Error: Could not create CounterPath directory " + m_pszCounterPath);
                goto ErrorExit;
            }
            ReportEvent ((String) "The CounterPath " + m_pszCounterPath + " was created");
        }
    } else { 
        ReportEvent ("Error: Could not read the CounterPath from Alajar.conf");
        goto ErrorExit;
    }
    
    ReportEvent ((String) "Storing counter files in " + m_pszCounterPath);
    
    // Log path
    if (m_pConfigFile->GetParameter ("LogPath", &pszRhs) == OK && pszRhs != NULL) {
        
        if (File::ResolvePath (pszRhs, m_pszLogPath) == ERROR_FAILURE) {
            ReportEvent ((String) "Error: The LogPath value was invalid:" + pszRhs);
            goto ErrorExit;
        }
        
        if (!File::DoesDirectoryExist (m_pszLogPath)) {
            if (File::CreateDirectory (m_pszLogPath) != OK) {
                ReportEvent ((String) "Error: Could not create LogPath directory " + m_pszLogPath);
                goto ErrorExit;
            }
            ReportEvent ((String) "Warning: The LogPath " + m_pszLogPath + " was created");
        }
    } else {
        ReportEvent ("Error: Could not read the LogPath from Alajar.conf");
        goto ErrorExit;
    }
    
    ReportEvent ((String) "Storing log files in " + m_pszLogPath);
    
    // PageSourcePath
    if (m_pConfigFile->GetParameter ("PageSourcePath", &pszRhs) == OK && pszRhs != NULL) {
        
        if (File::ResolvePath (pszRhs, m_pszPageSourcePath) == ERROR_FAILURE) {
            ReportEvent ((String) "Error: The PageSourcePath value was invalid:" + pszRhs);
            goto ErrorExit;
        }
        
        if (!File::DoesDirectoryExist (m_pszPageSourcePath)) {
            if (File::CreateDirectory (m_pszPageSourcePath) != OK) {
                ReportEvent ((String) "Error: Could not create PageSourcePath directory " + m_pszPageSourcePath);
                goto ErrorExit;
            }
            ReportEvent ((String) "Warning: The PageSourcePath " + m_pszPageSourcePath + " was created");
        }
    } else {
        ReportEvent ("Error: Could not read the PageSourcePath from Alajar.conf");
        goto ErrorExit;
    }
    
    ReportEvent ((String) "Reading page sources from " + m_pszPageSourcePath);
    
    if (m_pConfigFile->GetParameter ("ConfigPath", &pszRhs) == OK && pszRhs != NULL) {
        
        if (File::ResolvePath (pszRhs, m_pszConfigPath) == ERROR_FAILURE) {
            ReportEvent ((String) "Error: The ConfigPath value was invalid:" + pszRhs);
            goto ErrorExit;
        }
        
        if (!File::DoesDirectoryExist (m_pszConfigPath)) {
            ReportEvent ((String) "Error: The ConfigPath " + m_pszConfigPath + " could not be found");
            goto ErrorExit;
        }
    } else {
        ReportEvent ("Error: Could not read the ConfigPath from Alajar.conf");
        goto ErrorExit;
    }

    ReportEvent ((String) "Reading config files from " + m_pszConfigPath);

    if (m_pConfigFile->GetParameter ("StatisticsPath", &pszRhs) == OK && pszRhs != NULL) {
        
        if (File::ResolvePath (pszRhs, m_pszStatisticsPath) == ERROR_FAILURE) {
            ReportEvent ((String) "Error: The StatisticsPath value was invalid:" + pszRhs);
            goto ErrorExit;
        }
        
        if (!File::DoesDirectoryExist (m_pszStatisticsPath)) {
            ReportEvent ((String) "Error: The StatisticsPath " + m_pszStatisticsPath + " could not be found");
            goto ErrorExit;
        }
    } else {
        ReportEvent ("Error: Could not read the StatisticsPath from Alajar.conf");
        goto ErrorExit;
    }
    
    if (m_pConfigFile->GetParameter ("Port", &pszRhs) == OK && pszRhs != NULL) {
        m_siPort = (short) atoi (pszRhs);
    } else {
        ReportEvent ("Error: Could not read the Port from Alajar.conf");
        goto ErrorExit;
    }

    ReportEvent ((String) "Reading statistics from " + m_pszStatisticsPath);

    ReadStatistics();
    
    if (m_pConfigFile->GetParameter ("FileCache", &pszRhs) == OK && pszRhs != NULL) {
        bFileCache = (atoi (pszRhs) == 1);
    } else {
        ReportEvent ("Error: Could not read FileCache flag from Alajar.conf");
        goto ErrorExit;
    }

    if (m_pConfigFile->GetParameter ("ApproxNumFilesInFileCache", &pszRhs) == OK && pszRhs != NULL) {
        iNumFiles = atoi (pszRhs);
        if (iNumFiles < 1) {
            ReportEvent ("Error: Could not read the ApproxNumFilesInFileCache from Alajar.conf");
            goto ErrorExit;
        }
        m_pFileCache = FileCache::CreateInstance (iNumFiles, bFileCache);
    } else {
        ReportEvent ("Error: Could not read the ApproxNumFilesInFileCache from Alajar.conf");
        goto ErrorExit;
    }

    if (bFileCache) {
        ReportEvent ((String) "The file cache is on, optimized for " + iNumFiles + " files");
    } else {
        ReportEvent ("The file cache is off");
    }
    
    if (m_pConfigFile->GetParameter ("InitNumThreads", &pszRhs) == OK && pszRhs != NULL) {
        iInitNumThreads = atoi (pszRhs);
    } else {
        ReportEvent ("Error: Could not read InitNumThreads from Alajar.conf");
        goto ErrorExit;
    }

    if (m_pConfigFile->GetParameter ("MaxNumThreads", &pszRhs) == OK && pszRhs != NULL) {
        iMaxNumThreads = atoi (pszRhs);
    } else {
        ReportEvent ("Error: Could not read MaxNumThreads from Alajar.conf");
        goto ErrorExit;
    }

    if (iMaxNumThreads < iInitNumThreads) {
        ReportEvent ("MaxNumThreads cannot be less than InitNumThreads in Alajar.conf");
        goto ErrorExit;
    }

    
    ReportEvent ((String) "The threadpool is using " + iInitNumThreads + " to " + iMaxNumThreads + 
        " server thread" + (iMaxNumThreads != 1 ? "s" : ""));
    
    if (m_pConfigFile->GetParameter ("DefaultThreadPriority", &pszRhs) == OK && pszRhs != NULL) {
        
        if (stricmp ("Lowest", pszRhs) == 0) {
            ReportEvent ("The server threads are running at the lowest priority");
            m_iDefaultPriority = Thread::LowestPriority;
        }
        else if (stricmp ("Lower", pszRhs) == 0) {
            ReportEvent ("The server threads are running at a lower priority");
            m_iDefaultPriority = Thread::LowerPriority;
        }
        else if (stricmp ("Normal", pszRhs) == 0) {
            ReportEvent ("The server threads are running at normal priority");
            m_iDefaultPriority = Thread::NormalPriority;
        }
        else if (stricmp ("Higher", pszRhs) == 0) {
            ReportEvent ("The server threads are running at a higher priority");
            m_iDefaultPriority = Thread::HigherPriority;
        }
        else if (stricmp ("Highest", pszRhs) == 0) {
            ReportEvent ("The server threads are running at the highest priority");
            m_iDefaultPriority = Thread::HighestPriority;
        }
        else {
            ReportEvent ("Error: DefaultThreadPriority has an incorrect value in Alajar.conf");
            goto ErrorExit;
        }
    } else {
        ReportEvent ("Error: Could not read DefaultThreadPriority from Alajar.conf");
        goto ErrorExit;
    }
    
    // Create the threadpool
    m_pThreadPool = new HttpThreadPool (this, iInitNumThreads, iMaxNumThreads);
    if (m_pThreadPool == NULL) {
        ReportEvent ("The server is out of memory");
        goto ErrorExit;
    }
    
    // Get network info
    iErrCode = Socket::GetOurHostName (m_pszHostName, sizeof (m_pszHostName));
    if (iErrCode != OK) {
        ReportEvent ((String) "Could not get host name: error " + iErrCode + " occurred");
        goto ErrorExit;
    }
    
    iErrCode = Socket::GetIPAddressFromHostName (m_pszHostName, m_pszIPAddress, sizeof (m_pszIPAddress));
    if (iErrCode != OK) {
        ReportEvent ((String) "Could not get IP address from host name: error " + iErrCode + " occurred");
        goto ErrorExit;
    }

    // Needed for reports
    if (m_pLogMessageCache == NULL) {

        m_pLogMessageCache = new ObjectCache<LogMessage, LogMessageAllocator>();
        if (m_pLogMessageCache == NULL || !m_pLogMessageCache->Initialize (iMaxNumThreads)) {
            
            ReportEvent ("The server is out of memory");
            goto ErrorExit;
        }
    }

    // Configure PageSources
    iErrCode = ConfigurePageSources();
    if (iErrCode != OK) {
        ReportEvent ("An error occurred while configuring a PageSource");
        goto ErrorExit;
    }

    // Calculate size of http object cache
    iNumHttpObjects = iMaxNumThreads;

    while (m_pPageSourceTable->GetNextIterator (&htiPageSource)) {      
        pPageSource = htiPageSource.GetData();
        if (pPageSource->IsSingleThreaded()) {
            iNumHttpObjects += 3;
        }
    }

    // Initialize objects
    m_pHttpRequestCache = new ObjectCache<HttpRequest, HttpRequestAllocator>();
    m_pHttpResponseCache = new ObjectCache<HttpResponse, HttpResponseAllocator>();
    m_pSocketCache = new ObjectCache<Socket, SocketAllocator>();

    m_pSocket = new Socket();

    if (m_pHttpRequestCache == NULL ||
        m_pHttpResponseCache == NULL ||
        m_pSocketCache == NULL ||
        m_pSocket == NULL ||

        !m_pHttpRequestCache->Initialize (iNumHttpObjects) ||
        !m_pHttpResponseCache->Initialize (iNumHttpObjects) ||
        !m_pSocketCache->Initialize (iNumHttpObjects)

        ) {

        ReportEvent ("The server is out of memory");
        goto ErrorExit;
    }

    iErrCode = m_pSocket->Open();
    if (iErrCode != OK) {
        ReportEvent ((String) "Socket error " + Socket::GetLastError() + ": Unable to open socket");
        goto ErrorExit;
    }

    // Set the socket to listen
    ReportEvent ((String) "Listening on port " + (int) m_siPort);
    
    iErrCode = m_pSocket->Listen (m_siPort);
    if (iErrCode != OK) {
        
        ReportEvent ((String) "Socket error " + Socket::GetLastError() + ": Could not listen at port " + 
            (int) m_siPort);
        
        ShutdownPageSources();
        
        goto ErrorExit;
        
    }
    
    // Let's go!
    return Run();

ErrorExit:

    m_pConfigFile->Release();
    m_pConfigFile = NULL;

    return ERROR_FAILURE;

}

/////////////
// Threads //
/////////////

int HttpServer::Run() {

    int iErrCode = OK;
    Socket* pRequestSocket;

    // Initialize exit notification environment
    m_bExit = false;
    m_bLogExit = false;
    m_bRestart = false;

    ReportEvent ("");
    ReportEvent ("Initializing page sources");

    // Initialize the ThreadPool
    ReportEvent ("Starting thread pool");
    iErrCode = m_pThreadPool->Start();
    if (iErrCode != OK) {
        ReportEvent ("Unable to start thread pool");
        return iErrCode;
    }

    // Initialize the log thread
    ReportEvent ("Starting logging thread");
    iErrCode = m_tLogThread.Start (HttpServer::LogThread, this, Thread::LowerPriority);
    if (iErrCode != OK) {
        ReportEvent ("Unable to start logging thread");
        return iErrCode;
    }

    // Loop until we receive the order to finish
    ReportEvent ("Server started");
    
    // Notify world that we started up
    if (m_pStartupSink != NULL) {
        m_pStartupSink->OnStartup (OK);
        m_pStartupSink->Release();
        m_pStartupSink = NULL;
    }

    // Set priority higher than average
    Thread tSelf;
    Thread::GetCurrentThread (&tSelf);
    tSelf.SetPriority (Thread::HigherPriority);

    // Loop forever
    while (true) {

        // Get a socket
        m_mSocketCacheLock.Wait();
        pRequestSocket = m_pSocketCache->GetObject();
        m_mSocketCacheLock.Signal();

        if (pRequestSocket == NULL) {
            // Out of memory!
            ReportEvent ("Server is out of memory!");
            break;
        }

        // Accept the incoming connection and put the socket into the task queue
        if (m_pSocket->Accept (pRequestSocket) != OK ||
            m_pThreadPool->QueueTask (pRequestSocket) != OK) {

            m_mSocketCacheLock.Wait();
            m_pSocketCache->ReleaseObject (pRequestSocket);
            m_mSocketCacheLock.Signal();
        }

        // Check for exit
        if (m_bExit) {
            break;
        }
    }

    // Shut down the thread pool
    ReportEvent ("Shutting down the threadpool");
    
    unsigned int iRetries = 0;
    while (m_pThreadPool->Stop() != OK && iRetries < 10) {
        iRetries ++;
        OS::Sleep (1000);
    }

    if (iRetries == 10) {
        ReportEvent ("Could not shut down the thread pool");
    }

    // Write statistics
    ReportEvent ("Writing statistics file");
    WriteStatistics (m_tLogDate);

    // Close down listener socket, if necessary
    if (m_pSocket->IsConnected()) {
        m_pSocket->Close();
    }

    if (m_bRestart) {

        // We're being asked to restart
        Clean();
        Initialize();

        m_fReportFile.Close();
        m_pConfigFile->Refresh();

        // Shut down the logging thread
        ReportEvent ("Shutting down the logging thread");
    
        m_bLogExit = true;
        m_evLogEvent.Signal();
        m_tLogThread.WaitForTermination();

        iErrCode = StartServer();

        if (iErrCode == OK) {

            m_bRunning = true;
            m_bRestart = false;

        } else {
            
            // Shouldn't happen...
            Assert (false);
            Clean();
            
            // Shut down
            IShutdownSink* pShutdownSink = m_pShutdownSink;
            m_pShutdownSink = NULL;
            
            if (pShutdownSink != NULL) {
                pShutdownSink->OnShutdown (OK);
                pShutdownSink->Release();
            }
        }

        if (m_pRestartSink != NULL) {
            m_pRestartSink->OnRestart (iErrCode);
            m_pRestartSink->Release();
            m_pRestartSink = NULL;
        }

    } else {

        Clean();

        // Shut down the logging thread
        ReportEvent ("Shutting down the logging thread");
    
        m_bLogExit = true;
        m_evLogEvent.Signal();
        m_tLogThread.WaitForTermination();

        IShutdownSink* pShutdownSink = m_pShutdownSink;
        m_pShutdownSink = NULL;

        if (pShutdownSink != NULL) {
            pShutdownSink->OnShutdown (OK);
            pShutdownSink->Release();
        }
    }

    return iErrCode;

    // Main thread exits here
}



/*
        // Handle the HTTP request
        while (true) {

            // Handle the HTTP request
            iErrCode = SendHttpResponse (pssSocket, &httpRequest);
            if (iErrCode != OK) {
                break;
            }

            // Exit loop if no keep-alive
            if (!httpRequest.GetKeepAlive()) {
                
                m_rwPageSourceTableLock.WaitReader();
                iErrCode = httpRequest.ParseHeaders (pssSocket, m_pDefaultPageSource, m_pPageSourceTable);
                m_rwPageSourceTableLock.SignalReader();
                
                if (iErrCode != OK) {
                    break;
                }
            }
        }
*/

int HttpServer::WWWServe (HttpPoolThread* pSelf) {

    int iErrCode = OK;
    bool bSingleThreaded;

    HttpRequest* pHttpRequest;
    HttpResponse* pHttpResponse;

    Socket* pSocket;
    PageSource* pPageSource;

    // Stats
    Timer tmTimer;
    Time::StartTimer (&tmTimer);

    HttpServerStatistics sThreadStats;
    memset (&sThreadStats, 0, sizeof (HttpServerStatistics));

    // Randomize
    Algorithm::InitializeThreadRandom (pSelf->GetThreadId());

    // Loop 'forever'
    while (true) {

        // Get a new task
        pSocket = m_pThreadPool->WaitForTask (pSelf);

        // Check for exit
        if (pSocket == NULL) {
            break;
        }

        // Best effort reset the socket's statistics
        pSocket->ResetStatistics();

        // Get request, response objects
        iErrCode = GetHttpObjects (&pHttpRequest, &pHttpResponse);
        if (iErrCode != OK) {
            goto OnError;
        }
            
        // Set sockets
        pHttpRequest->SetSocket (pSocket);
        pHttpResponse->SetSocket (pSocket);
        
        // Parse the request
        iErrCode = pHttpRequest->ParseHeaders();
        
        // Check for parsing errors
        switch (iErrCode) {
            
        case OK:
            break;
            
        case ERROR_UNSUPPORTED_HTTP_VERSION:
            pHttpResponse->SetStatusCode (HTTP_505);
            break;
            
        case ERROR_UNSUPPORTED_HTTP_METHOD:
            pHttpResponse->SetStatusCode (HTTP_501);
            break;

        case ERROR_OUT_OF_MEMORY:
        case ERROR_OUT_OF_DISK_SPACE:
            pHttpResponse->SetStatusCode (HTTP_503);
            break;
            
        default:
            pHttpResponse->SetStatusCode (HTTP_400);
            break;
        }
        
        // Clean up if free threaded
        pPageSource = pHttpRequest->GetPageSource();
        bSingleThreaded = pPageSource != NULL && pPageSource->IsSingleThreaded();
        
        // Best effort response
        iErrCode = pHttpResponse->Respond();
        
        if (!bSingleThreaded) {

            FinishRequest (pHttpRequest, pHttpResponse, pSocket, &sThreadStats, iErrCode);

            if (sThreadStats.NumRequests % COALESCE_REQUESTS == 0 ||
                Time::GetTimerCount (tmTimer) > COALESCE_PERIOD_MS) {
                
                CoalesceStatistics (&sThreadStats);
                Time::StartTimer (&tmTimer);
            }
        }
    }

    CoalesceStatistics (&sThreadStats);

    return OK;

OnError:

    pSocket->Close();
    
    m_mSocketCacheLock.Wait();
    m_pSocketCache->ReleaseObject (pSocket);
    m_mSocketCacheLock.Signal();
    
    ReportEvent ("Could not obtain a request or a response object. The server may be out of memory");

    return iErrCode;
}

void HttpServer::FinishRequest (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse, Socket* pSocket,
                                HttpServerStatistics* psThreadStats, int iErrCode) {

    StatisticsAndLog (pHttpRequest, pHttpResponse, pSocket, psThreadStats, iErrCode);
    
    if (pHttpResponse->ConnectionClosed()) {
        
        // Free socket
        pSocket->Close();

        m_mSocketCacheLock.Wait();
        m_pSocketCache->ReleaseObject (pSocket);
        m_mSocketCacheLock.Signal();

    } else {
        
        // Put the socket back onto the queue
        m_pThreadPool->QueueTask (pSocket);
    }
    
    // Give the objects back to the cache
    ReleaseHttpObjects (pHttpRequest, pHttpResponse);
}


int HttpServer::ConfigurePageSources() {

    // Clean up previous PageSources if necessary
    if (m_pPageSourceTable != NULL) {
        delete m_pPageSourceTable;
        m_pPageSourceTable = NULL;
    }

    if (m_pDefaultPageSource != NULL) {
        delete m_pDefaultPageSource;
        m_pDefaultPageSource = NULL;
    }

    // Read config files
    char pszConfigFilePath [OS::MaxFileNameLength];
    strcpy (pszConfigFilePath, m_pszConfigPath);
    strcat (pszConfigFilePath, "/*.conf");

    FileEnumerator fEnum;

    unsigned int iNumPageSources, i;
    const char** ppszFileName;

    int iErrCode = File::EnumerateFiles (pszConfigFilePath, &fEnum);
    if (iErrCode != OK) {
        return iErrCode;
    }

    ppszFileName = fEnum.GetFileNames();
    iNumPageSources = fEnum.GetNumFiles();

    // Initialize pagesource table
    Assert (m_pPageSourceTable == NULL);

    m_pPageSourceTable = new HashTable<const char*, PageSource*, PageSourceHashValue, PageSourceEquals>(NULL, NULL);
    if (m_pPageSourceTable == NULL || !m_pPageSourceTable->Initialize (iNumPageSources + 1)) {
        return ERROR_OUT_OF_MEMORY;
    }

    ///////////////////////////////////
    // Get data for all page sources //
    ///////////////////////////////////

    PageSource* pPageSource;
    const char* pszPageSourceName;

    String strErrorMessage;
    
    bool bError = false;

    for (i = 0; i < iNumPageSources; i ++) {
        
        strcpy (pszConfigFilePath, m_pszConfigPath);
        strcat (pszConfigFilePath, "/");
        strcat (pszConfigFilePath, ppszFileName[i]);
        
        pPageSource = new PageSource (this);
        if (pPageSource == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        iErrCode = pPageSource->Init();
        if (iErrCode != OK) {
            delete pPageSource;
            return iErrCode;
        }
        
        if (pPageSource->Configure (pszConfigFilePath, &strErrorMessage) == OK) {

            // We have a good page source here
            pszPageSourceName = pPageSource->GetName();

            ReportEvent ((String) "\t" + pszPageSourceName);
            
            if (stricmp (pPageSource->GetName(), "Default") == 0) {
                m_pDefaultPageSource = pPageSource;
            } else {
                m_pPageSourceTable->Insert (pszPageSourceName, pPageSource);
            }

        } else {

            // Fatal error:  a pagesource failed to start up
            ReportEvent ((String) "An error occurred initializing the pagesource from " + ppszFileName[i] + ". " +
                strErrorMessage);

            pPageSource->Release();
            bError = true;
            break;
        }
    }

    if (bError) {
        
        // Shut down all valid pagesources
        if (m_pDefaultPageSource != NULL) {
            m_pDefaultPageSource->Release();
            m_pDefaultPageSource = NULL;
        }

        PageSource* pPageSource;
        HashTableIterator<const char*, PageSource*> htiPageSource;
        if (m_pPageSourceTable->GetNextIterator (&htiPageSource)) {
            while (m_pPageSourceTable->Delete (&htiPageSource, NULL, &pPageSource)) {
                pPageSource->Release();
            }
        }

    } else {

        if (m_pDefaultPageSource == NULL) {
            ReportEvent ((String) "Error: Could not find the default page source");
            bError = true;
            
            PageSource* pPageSource;
            HashTableIterator<const char*, PageSource*> htiPageSource;
            if (m_pPageSourceTable->GetNextIterator (&htiPageSource)) {
                while (m_pPageSourceTable->Delete (&htiPageSource, NULL, &pPageSource)) {
                    pPageSource->Release();
                }
            }
        }
    }

    return bError ? ERROR_FAILURE : OK;
}


void HttpServer::ShutdownPageSources() {

    Thread tDefaultThread;

    // Close down default pagesource
    if (m_pDefaultPageSource != NULL) {

        ReportEvent ((String) "\t" + m_pDefaultPageSource->GetName());
        tDefaultThread.Start (HttpServer::ClosePageSource, m_pDefaultPageSource);
        m_pDefaultPageSource = NULL;
    }

    // Close down the rest of the pagesources
    if (m_pPageSourceTable != NULL) {

        unsigned int iNumPageSources = m_pPageSourceTable->GetNumElements();

        if (iNumPageSources > 0) {
            
            // Create a new thread for each page source shut down
            // Tolerate out of memory errors
            PageSource* pPageSource;
            HashTableIterator<const char*, PageSource*> htiPageSource;

            Thread* ptThread = new Thread [iNumPageSources];

            if (ptThread != NULL) {
            
                unsigned int i = 0;
                while (m_pPageSourceTable->GetNextIterator (&htiPageSource)) {
                    
                    pPageSource = htiPageSource.GetData();
                    ReportEvent ((String) "\t" + pPageSource->GetName());

                    Assert (i < iNumPageSources);
                    
                    ptThread[i].Start (HttpServer::ClosePageSource, pPageSource);
                    i ++;
                }
                
                // Clear the hash table
                m_pPageSourceTable->Clear();
                
                // Wait until all pagesources have completed their finalization
                for (i = 0; i < iNumPageSources; i ++) {
                    ptThread[i].WaitForTermination();
                }
                
                delete [] ptThread;
            
            } else {

                // We're really hurting for RAM...
                while (m_pPageSourceTable->GetNextIterator (&htiPageSource)) {
                    
                    pPageSource = htiPageSource.GetData();
                    ReportEvent (pPageSource->GetName());
                    HttpServer::ClosePageSource (pPageSource);
                }
            }
        }
    }

    tDefaultThread.WaitForTermination();
}

LogMessage* HttpServer::GetNextLogMessage() {

    LogMessage* plmLogMessage;
    return m_tsfqLogQueue.Pop (&plmLogMessage) ? plmLogMessage : NULL;
}

int HttpServer::GetNumLogMessages() {
    return m_tsfqLogQueue.GetNumElements();
}


int THREAD_CALL HttpServer::LogThread (void* pVoid) {

    HttpServer* pThis = (HttpServer*) pVoid;

    return pThis->LogLoop();
}

int HttpServer::LogLoop() {

    LogMessage* plmMessage;
    PageSource* pPageSource;

    while (true) {
        
        plmMessage = GetNextLogMessage();
        
        if (plmMessage == NULL) {

            if (m_bLogExit) {
                
                if (GetNumLogMessages() == 0) {
                    break;
                }

            } else {

                // Wait for something to happen
                m_evLogEvent.Wait();
            }

        } else {

            pPageSource = plmMessage->pPageSource;

            switch (plmMessage->lmtMessageType) {

            case LOG_MESSAGE:

                pPageSource->LogMessage (plmMessage->pszText);
                break;

            case REPORT_MESSAGE:

                pPageSource->ReportMessage (plmMessage->pszText);
                break;

            default:

                Assert (false);
            }

            pPageSource->Release();
            FreeLogMessage (plmMessage);
        }
    }

    return OK;
}

/*
size_t AppendString (char** ppszBuffer, const char* pszNewString, size_t stTotalLength) {

    size_t iNewLength, iNewStringLength = strlen (pszNewString);
    
    if (*ppszBuffer == NULL || stTotalLength == 0) {
    
        *ppszBuffer = new char [iNewStringLength + 1];
        strcpy (*ppszBuffer, pszNewString);
        iNewLength = iNewStringLength;
    
    } else {

        size_t iOldLength = strlen (*ppszBuffer);
        iNewLength = iOldLength + iNewStringLength;

        if (iNewLength >= stTotalLength) {

            char* pszTemp = new char [iNewLength + 1];

            strcpy (pszTemp, *ppszBuffer);
            strcat (pszTemp, pszNewString);

            delete [] *ppszBuffer;
            *ppszBuffer = pszTemp;

        } else {

            strcpy (&((*ppszBuffer)[iOldLength]), pszNewString);
        }
    }
    
    return iNewLength;
}

char* EqualsToSlash (const char* pszInput) {
    
    if (pszInput == NULL) {
        return NULL;
    }
    
    int i, iCurPos = 0, stLength = strlen (pszInput);
    
    char* pszNewString = new char [stLength + 1];
    
    for (i = 0; i < stLength; i ++) {
        
        if (pszInput[i] == '=') {
            pszNewString[iCurPos] = '/';
        } else {
            pszNewString[iCurPos] = pszInput[i];
        }
        
        iCurPos ++;
    }
    
    pszNewString [iCurPos] = '\0';
    
    return pszNewString;
}
*/

IFileCache* HttpServer::GetFileCache() {

    Assert (m_pFileCache != NULL);
    m_pFileCache->AddRef();
    return m_pFileCache;
}

void HttpServer::ReportEvent (const char* pszEvent) {
    
    printf (pszEvent);
    printf ("\n");

    WriteReport (pszEvent);
}

int HttpServer::ClosePageSource (void* pVoid) {

    PageSource* pPageSource = (PageSource*) pVoid;

    pPageSource->OnFinalize();
    pPageSource->Release();

    return OK;
}

short HttpServer::GetPort() {
    return m_siPort;
}

const char* HttpServer::GetHostName() {
    return m_pszHostName;
}

const char* HttpServer::GetIPAddress() {
    return m_pszIPAddress;
}

const char* HttpServer::GetServerName() {
    return m_pszServerName;
}

unsigned int HttpServer::GetNumThreads() {
    return m_pThreadPool->GetNumThreads();
}

unsigned int HttpServer::GetNumQueuedRequests() {
    return m_pThreadPool->GetMaxNumQueuedTasks();
}

unsigned int HttpServer::GetMaxNumQueuedRequests() {
    return m_pThreadPool->GetMaxNumQueuedTasks();
}

unsigned int HttpServer::GetStatistics (HttpServerStatistics* pStatistics) {
    
    m_mStatMutex.Wait();
    *pStatistics = m_stats;
    m_mStatMutex.Signal();

    pStatistics->NumQueuedRequests = m_pThreadPool->GetNumQueuedTasks();
    pStatistics->MaxNumQueuedRequests = m_pThreadPool->GetMaxNumQueuedTasks();

    return OK;
}

unsigned int HttpServer::GetNumPageSources() {
    return m_pPageSourceTable->GetNumElements() + 1;
}

IReport* HttpServer::GetReport() {
    return this;
}

IConfigFile* HttpServer::GetConfigFile() {

    if (m_pConfigFile != NULL) {
        m_pConfigFile->AddRef();
    }
    return m_pConfigFile;
}

int HttpServer::Start (IConfigFile* pConfigFile, IStartupSink* pStartupSink, IShutdownSink* pShutdownSink) {

    m_mShutdown.Wait();
    if (m_bRunning) {
        m_mShutdown.Signal();
        return WARNING;
    }
    if (pConfigFile == NULL) {
        m_mShutdown.Signal();
        return ERROR_FAILURE;
    }

    m_bRunning = true;
    m_mShutdown.Signal();

    Assert (m_pShutdownSink == NULL);
    m_pStartupSink = pStartupSink;
    if (pStartupSink != NULL) {
        pStartupSink->AddRef();
    }

    Assert (m_pShutdownSink == NULL);
    m_pShutdownSink = pShutdownSink;
    if (pShutdownSink != NULL) {
        pShutdownSink->AddRef();
    }

    m_pConfigFile = pConfigFile;
    m_pConfigFile->AddRef();

    return m_tMainThread.Start (&HttpServer::StartServer, this);
}

int HttpServer::Restart (IRestartSink* pRestartSink) {

    m_mShutdown.Wait();
    if (m_bRestart) {
        m_mShutdown.Signal();
        return WARNING;
    }
    m_bRestart = true;
    m_mShutdown.Signal();

    Assert (m_pRestartSink == NULL);
    m_pRestartSink = pRestartSink;
    if (pRestartSink != NULL) {
        pRestartSink->AddRef();
    }

    // Close the main socket
    if (m_pSocket != NULL) {
        m_pSocket->Close();
    }

    // Tell the main thread to exit and restart
    m_bExit = true;

    // Return to caller; probably someone in the threadpool
    return OK;
}

int HttpServer::Shutdown() {

    m_mShutdown.Wait();

    if (m_bExit) {
        m_mShutdown.Signal();
        return WARNING;
    }

    // Close the main socket
    if (m_pSocket != NULL) {
        m_pSocket->Close();
    }

    // Tell the main thread to exit
    m_bExit = true;
    m_mShutdown.Signal();

    // Return to caller; probably someone in the threadpool
    return OK;
}

int HttpServer::DeletePageSource (const char* pszPageSourceName) {

    if (stricmp (pszPageSourceName, "Default") == 0 ||
        stricmp (pszPageSourceName, "Admin") == 0) {
        return ERROR_FAILURE;
    }

    PageSource* pPageSource;

    m_rwPageSourceTableLock.WaitWriter();
    bool bRetVal = m_pPageSourceTable->DeleteFirst (pszPageSourceName, NULL, &pPageSource);
    m_rwPageSourceTableLock.SignalWriter();

    // Release to destruct
    if (bRetVal) {
        pPageSource->Release();
        return OK;
    }
    
    return ERROR_FAILURE;
}

bool HttpServer::IsDefaultPageSource (PageSource* pPageSource) {
    return m_pDefaultPageSource == pPageSource;
}

PageSource* HttpServer::GetDefaultPageSource() {
    m_pDefaultPageSource->AddRef();
    return m_pDefaultPageSource;
}

HttpThreadPool* HttpServer::GetThreadPool() {
    return m_pThreadPool;
}

IPageSourceEnumerator* HttpServer::EnumeratePageSources() {

    m_rwPageSourceTableLock.WaitReader();
    PageSourceEnumerator* pPageSourceEnumerator = PageSourceEnumerator::CreateInstance (m_pPageSourceTable, 
        m_pDefaultPageSource);
    m_rwPageSourceTableLock.SignalReader();

    return pPageSourceEnumerator;
}


IPageSourceControl* HttpServer::GetPageSourceByName (const char* pszName) {

    PageSource* pPageSource = NULL;

    if (stricmp (pszName, "Default") == 0) {
        m_pDefaultPageSource->AddRef();
        return m_pDefaultPageSource;
    }

    m_rwPageSourceTableLock.WaitReader();
    bool bFound = m_pPageSourceTable->FindFirst (pszName, &pPageSource);
    m_rwPageSourceTableLock.SignalReader();

    if (bFound) {
        pPageSource->AddRef();
        return pPageSource;
    }

    return NULL;
}

int HttpServer::GetHttpObjects (HttpRequest** ppHttpRequest, HttpResponse** ppHttpResponse) {

    m_mHttpObjectCacheLock.Wait();
    HttpRequest*  pHttpRequest  = m_pHttpRequestCache->GetObject();
    HttpResponse* pHttpResponse = m_pHttpResponseCache->GetObject();
    m_mHttpObjectCacheLock.Signal();

    if (pHttpRequest == NULL) {

        if (pHttpResponse != NULL) {
            m_pHttpResponseCache->ReleaseObject (pHttpResponse);
        }
        return ERROR_OUT_OF_MEMORY;
    }

    if (pHttpResponse == NULL) {

        m_pHttpRequestCache->ReleaseObject (pHttpRequest);
        return ERROR_OUT_OF_MEMORY;
    }

    pHttpRequest->SetHttpServer (this);
    pHttpResponse->SetHttpObjects (this, pHttpRequest);

    *ppHttpRequest = pHttpRequest;
    *ppHttpResponse = pHttpResponse;

    return OK;
}

void HttpServer::ReleaseHttpObjects (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse) {

    Assert (pHttpRequest != NULL && pHttpResponse != NULL);

    pHttpRequest->Recycle();
    pHttpResponse->Recycle();

    m_mHttpObjectCacheLock.Wait();
    m_pHttpRequestCache->ReleaseObject (pHttpRequest);
    m_pHttpResponseCache->ReleaseObject (pHttpResponse);
    m_mHttpObjectCacheLock.Signal();
}

void HttpServer::StatisticsAndLog (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse, Socket* pSocket,
                                   HttpServerStatistics* psThreadStats, int iErrCode) {

    unsigned int* puiRef;

    HttpMethod mMethod = pHttpRequest->GetMethod();
    HttpStatus sStatus = pHttpResponse->GetStatusCode();

    // Update Statistics
    if (iErrCode != OK) {
        psThreadStats->NumErrors ++;
    }

    psThreadStats->NumRequests ++;

    puiRef = &psThreadStats->NumGets + (size_t) mMethod;
    (*puiRef) ++;

    puiRef = &psThreadStats->Num200s + (size_t) sStatus;
    (*puiRef) ++;

    if (sStatus == HTTP_403) {

        switch (pHttpResponse->GetStatusCodeReason()) {

        case HTTP_REASON_IPADDRESS_BLOCKED:

            psThreadStats->NumIPAddress403s ++;
            break;

        case HTTP_REASON_USER_AGENT_BLOCKED:

            psThreadStats->NumUserAgent403s ++;
            break;

        case HTTP_REASON_GET_REFERER_BLOCKED:
            
            psThreadStats->NumGETFilter403s ++;
            break;
        }
    }

    unsigned int iNumForms = pHttpRequest->GetNumForms();
    unsigned int iNumFileForms = pHttpRequest->GetNumFilesUploaded();

    psThreadStats->NumForms += iNumForms;
    psThreadStats->NumSimpleForms += iNumForms - iNumFileForms;
    psThreadStats->NumFileForms += iNumFileForms;

    psThreadStats->NumBytesInFileForms += pHttpRequest->GetNumBytesInUploadedFiles();
    psThreadStats->NumBytesSent += pSocket->GetNumBytesSent();
    psThreadStats->NumBytesReceived += pSocket->GetNumBytesReceived();

    psThreadStats->TotalRequestParseTime += pHttpRequest->GetRequestParseTime();
    psThreadStats->TotalResponseTime += pHttpResponse->GetResponseTime();

    // Update log
    PageSource* pPageSource = pHttpRequest->GetPageSource();
    if (pPageSource != NULL && pPageSource->UseLogging()) {

        // Log requests should never exceed 2KB
        char pszHour[20], pszMin[20], pszSec[20];

        // Get num uploads
        int iNumUploads = pHttpRequest->GetNumFilesUploaded();

        // Get IP address
        const char* pszIP = pHttpRequest->GetClientIP();
        if (pszIP == NULL) {
            pszIP = "Unknown IP address";
        }

        // Get Uri
        const char* pszUri = pHttpRequest->GetUri();
        if (pszUri == NULL) {
            pszUri = "Unknown URI";
        }

        LogMessage* plmMessage = GetLogMessage();
        if (plmMessage != NULL) {

            plmMessage->lmtMessageType = LOG_MESSAGE;
            plmMessage->pPageSource = pPageSource;
            
            // Stabilize pagesource
            pPageSource->AddRef();

            // Get date
            int iSec, iMin, iHour, iDay, iMonth, iYear;
            Time::GetDate (&iSec, &iMin, &iHour, &iDay, &iMonth, &iYear);
            
            // Write log message
            if (pPageSource->UseCommonLogFormat()) {
                
                // Get uri forms
                const char* pszForms = pHttpRequest->GetParsedUriForms();
                if (pszForms == NULL) {
                    pszForms = "";
                }
                
                const char* pszQ;
                if (*pszForms == '\0') {
                    pszQ = "";
                } else {
                    pszQ = "?";
                }
                
                const char* pszUserName = pHttpRequest->GetLogin();
                if (pszUserName == NULL || *pszUserName == '\0') {
                    pszUserName = "-";
                }
                
                char pszDay [20];
                char pszBias [20];

                int iBias;
                Time::GetTimeZoneBias (&iBias);
                
                snprintf (
                    plmMessage->pszText, sizeof (plmMessage->pszText) / sizeof (plmMessage->pszText[0]),
                    "%s - %s [%s/%s/%d:%s:%s:%s %s] \"%s %s%s%s\" %d %u",
                    pszIP,
                    pszUserName,
                    String::ItoA (iDay, pszDay, 10, 2),
                    Time::GetAbbreviatedMonthName (iMonth),
                    iYear,
                    String::ItoA (iHour, pszHour, 10, 2),
                    String::ItoA (iMin, pszMin, 10, 2),
                    String::ItoA (iSec, pszSec, 10, 2),
                    String::ItoA (iBias, pszBias, 10, 4),
                    HttpMethodText [mMethod],
                    pszUri,
                    pszQ,
                    pszForms,
                    HttpStatusValue [sStatus],
                    pHttpResponse->GetResponseLength()
                    );

                plmMessage->pszText [sizeof (plmMessage->pszText) / sizeof (plmMessage->pszText[0]) - 1] = '\0';
                
            } else {
                
                // Get user agent
                const char* pszBrowser = pHttpRequest->GetBrowserName();
                if (pszBrowser == NULL) {
                    pszBrowser = "Unknown Browser";
                }
                
                if (iNumUploads == 0) {
                    
                    snprintf (
                        plmMessage->pszText, sizeof (plmMessage->pszText) / sizeof (plmMessage->pszText[0]),
                        "[%s:%s:%s] %s\t%s\t%i\t%s\t%s", 
                        String::ItoA (iHour, pszHour, 10, 2),
                        String::ItoA (iMin, pszMin, 10, 2),
                        String::ItoA (iSec, pszSec, 10, 2),
                        pszIP, 
                        HttpMethodText[mMethod], 
                        HttpStatusValue [sStatus], 
                        pszUri,
                        pszBrowser
                        );
                    
                } else {
                    
                    snprintf (
                        plmMessage->pszText, sizeof (plmMessage->pszText) / sizeof (plmMessage->pszText[0]),
                        "[%s:%s:%s] %s\t%s\t%i\t%s\t%s\t%d uploa%s (%u bytes)", 
                        String::ItoA (iHour, pszHour, 10, 2),
                        String::ItoA (iMin, pszMin, 10, 2),
                        String::ItoA (iSec, pszSec, 10, 2),
                        pszIP,
                        HttpMethodText[mMethod],
                        HttpStatusValue [sStatus],
                        pszUri,
                        pszBrowser,
                        iNumUploads, iNumUploads == 1 ? "d" : "ds",
                        pHttpRequest->GetNumBytesInUploadedFiles()
                        );
                }

                plmMessage->pszText [sizeof (plmMessage->pszText) / sizeof (plmMessage->pszText[0]) - 1] = '\0';
                
                unsigned int i, iNumCustomLogMessages;
                const char** ppszCustomLogMessages;
                
                pHttpResponse->GetCustomLogMessages (&ppszCustomLogMessages, &iNumCustomLogMessages);
                
                for (i = 0; i < iNumCustomLogMessages; i ++) {
                    
                    strncat (
                        plmMessage->pszText,
                        "\t", 
                        sizeof (plmMessage->pszText) / sizeof (plmMessage->pszText[0]) - strlen (plmMessage->pszText)
                        );

                    strncat (
                        plmMessage->pszText, 
                        ppszCustomLogMessages[i],
                        sizeof (plmMessage->pszText) / sizeof (plmMessage->pszText[0]) - strlen (plmMessage->pszText)
                        );
                }
            }
            
            // Add log message to the queue
            if (PostMessage (plmMessage) != OK) {

                plmMessage->pPageSource->Release();
                FreeLogMessage (plmMessage);
            }
        }
    }
}

void HttpServer::CoalesceStatistics (HttpServerStatistics* psThreadStats) {

    UTCTime tNow;

    m_mStatMutex.Wait();

    // Check for daily refresh
    Time::GetTime (&tNow);
    if (DifferentDays (m_tLogDate, tNow)) {

        // Flush and reinitialize
        if (m_stats.NumRequests == 0) {
            m_stats.AverageRequestParseTime = 0;
        } else {
            m_stats.AverageRequestParseTime = (MilliSeconds) (m_stats.TotalRequestParseTime / m_stats.NumRequests);
        }

        WriteStatistics (m_tLogDate);
        m_tLogDate = tNow;
        
        UTCTime tTime = m_stats.StartupTime;
        memset (&m_stats, 0, sizeof (HttpServerStatistics));
        m_stats.StartupTime = tTime;
    }


    m_stats.NumErrors += psThreadStats->NumErrors;
    m_stats.NumRequests += psThreadStats->NumRequests;

    m_stats.NumGets += psThreadStats->NumGets;
    m_stats.NumPosts += psThreadStats->NumPosts;
    m_stats.NumPuts += psThreadStats->NumPuts;
    m_stats.NumHeads += psThreadStats->NumHeads;
    m_stats.NumTraces += psThreadStats->NumTraces;
    m_stats.NumUndefinedMethods += psThreadStats->NumUndefinedMethods;

    m_stats.Num200s += psThreadStats->Num200s;
    m_stats.Num301s += psThreadStats->Num301s;
    m_stats.Num304s += psThreadStats->Num304s;
    m_stats.Num400s += psThreadStats->Num400s;
    m_stats.Num401s += psThreadStats->Num401s;
    m_stats.Num403s += psThreadStats->Num403s;
    m_stats.Num404s += psThreadStats->Num404s;
    m_stats.Num409s += psThreadStats->Num409s;
    m_stats.Num500s += psThreadStats->Num500s;
    m_stats.Num501s += psThreadStats->Num501s;
    m_stats.Num503s += psThreadStats->Num503s;
    m_stats.Num505s += psThreadStats->Num505s;
    m_stats.NumUndefinedResponses += psThreadStats->NumUndefinedResponses;

    m_stats.NumForms += psThreadStats->NumForms;
    m_stats.NumSimpleForms += psThreadStats->NumSimpleForms;
    m_stats.NumFileForms += psThreadStats->NumFileForms;

    m_stats.NumBytesInFileForms += psThreadStats->NumBytesInFileForms;
    m_stats.NumBytesSent += psThreadStats->NumBytesSent;
    m_stats.NumBytesReceived += psThreadStats->NumBytesReceived;

    m_stats.TotalRequestParseTime += psThreadStats->TotalRequestParseTime;
    m_stats.TotalResponseTime += psThreadStats->TotalResponseTime;

    if (m_stats.NumRequests == 0) {
        m_stats.AverageRequestParseTime = 0;
        m_stats.AverageResponseTime = 0;
    } else {
        m_stats.AverageRequestParseTime = (MilliSeconds) (m_stats.TotalRequestParseTime / m_stats.NumRequests);
        m_stats.AverageResponseTime = (MilliSeconds) (m_stats.TotalResponseTime / m_stats.NumRequests);
    }

    m_stats.NumUserAgent403s += psThreadStats->NumUserAgent403s;
    m_stats.NumIPAddress403s += psThreadStats->NumIPAddress403s;
    m_stats.NumGETFilter403s += psThreadStats->NumGETFilter403s;

    m_mStatMutex.Signal();

    // Reset thread's stats
    memset (psThreadStats, 0, sizeof (HttpServerStatistics));
}

void HttpServer::ReadStatistics() {

    char pszStatFile [OS::MaxFileNameLength];
    GetStatisticsFileName (pszStatFile);

    Time::GetTime (&m_tLogDate);

    MemoryMappedFile mfStats;
    if (mfStats.OpenExisting (pszStatFile, true) != OK) {

        // No file - reset to zero
        UTCTime tTime = m_stats.StartupTime;
        memset (&m_stats, 0, sizeof (HttpServerStatistics));
        m_stats.StartupTime = tTime;

    } else {

        // Copy to null terminated buffer
        size_t stSize = mfStats.GetSize();

        char* pszBuffer = (char*) StackAlloc (stSize + 1);
        memcpy (pszBuffer, mfStats.GetAddress(), stSize);
        pszBuffer[stSize] = '\0';

        // Scan
        if (sscanf (
            pszBuffer,
            "NumRequests = %u\r\n"\
            "NumQueuedRequests = %u\r\n"\
            "MaxNumQueuedRequests = %u\r\n"\
            "NumGets = %u\r\n"\
            "NumPosts = %u\r\n"\
            "NumPuts = %u\r\n"\
            "NumHeads = %u\r\n"\
            "NumTraces = %u\r\n"\
            "NumUndefinedMethods = %u\r\n"\
            "Num200s = %u\r\n"\
            "Num301s = %u\r\n"\
            "Num304s = %u\r\n"\
            "Num400s = %u\r\n"\
            "Num401s = %u\r\n"\
            "Num403s = %u\r\n"\
            "Num404s = %u\r\n"\
            "Num409s = %u\r\n"\
            "Num500s = %u\r\n"\
            "Num501s = %u\r\n"\
            "Num503s = %u\r\n"\
            "Num505s = %u\r\n"\
            "NumUndefinedResponses = %u\r\n"\
            "NumForms = %u\r\n"\
            "NumSimpleForms = %u\r\n"\
            "NumFileForms = %u\r\n"\
            "NumBytesInFileForms = %I64u\r\n"\
            "NumBytesSent = %I64u\r\n"\
            "NumBytesReceived = %I64u\r\n"\
            "TotalRequestParseTime = %I64u\r\n"\
            "TotalResponseTime = %I64u\r\n"\
            "NumUserAgent403s = %u\r\n"\
            "NumIPAddress403s = %u\r\n"\
            "NumGETFilter403s = %u\r\n"\
            "NumErrors = %u",

            &m_stats.NumRequests,
            &m_stats.NumQueuedRequests,
            &m_stats.MaxNumQueuedRequests,
            &m_stats.NumGets,
            &m_stats.NumPosts,
            &m_stats.NumPuts,
            &m_stats.NumHeads,
            &m_stats.NumTraces,
            &m_stats.NumUndefinedMethods,
            &m_stats.Num200s,
            &m_stats.Num301s,
            &m_stats.Num304s,
            &m_stats.Num400s,
            &m_stats.Num401s,
            &m_stats.Num403s,
            &m_stats.Num404s,
            &m_stats.Num409s,
            &m_stats.Num500s,
            &m_stats.Num501s,
            &m_stats.Num503s,
            &m_stats.Num505s,
            &m_stats.NumUndefinedResponses,
            &m_stats.NumForms,
            &m_stats.NumSimpleForms,
            &m_stats.NumFileForms,
            &m_stats.NumBytesInFileForms,
            &m_stats.NumBytesSent,
            &m_stats.NumBytesReceived,
            &m_stats.TotalRequestParseTime,
            &m_stats.TotalResponseTime,
            &m_stats.NumUserAgent403s,
            &m_stats.NumIPAddress403s,
            &m_stats.NumGETFilter403s,
            &m_stats.NumErrors

            ) != 34) {

            memset (&m_stats, 0, sizeof (HttpServerStatistics));

        } else {

            if (m_stats.NumRequests != 0) {
                m_stats.AverageRequestParseTime = (MilliSeconds) (m_stats.TotalRequestParseTime / m_stats.NumRequests);
                m_stats.AverageResponseTime = (MilliSeconds) (m_stats.TotalResponseTime / m_stats.NumRequests);
            } else {
                m_stats.AverageRequestParseTime = 0;
                m_stats.AverageResponseTime = 0;
            }
        }
        mfStats.Close();
    }

    Time::GetTime (&m_stats.StartupTime);
}

void HttpServer::WriteStatistics (const UTCTime& tDate) {

    char pszStatFile [OS::MaxFileNameLength];
    GetStatisticsFileName (pszStatFile, tDate);

    char pszBuffer [4096];
    sprintf (
        pszBuffer,
        "NumRequests = %u\r\n"\
        "NumQueuedRequests = %u\r\n"\
        "MaxNumQueuedRequests = %u\r\n"\
        "NumGets = %u\r\n"\
        "NumPosts = %u\r\n"\
        "NumPuts = %u\r\n"\
        "NumHeads = %u\r\n"\
        "NumTraces = %u\r\n"\
        "NumUndefinedMethods = %u\r\n"\
        "Num200s = %u\r\n"\
        "Num301s = %u\r\n"\
        "Num304s = %u\r\n"\
        "Num400s = %u\r\n"\
        "Num401s = %u\r\n"\
        "Num403s = %u\r\n"\
        "Num404s = %u\r\n"\
        "Num409s = %u\r\n"\
        "Num500s = %u\r\n"\
        "Num501s = %u\r\n"\
        "Num503s = %u\r\n"\
        "Num505s = %u\r\n"\
        "NumUndefinedResponses = %u\r\n"\
        "NumForms = %u\r\n"\
        "NumSimpleForms = %u\r\n"\
        "NumFileForms = %u\r\n"\
        "NumBytesInFileForms = %I64u\r\n"\
        "NumBytesSent = %I64u\r\n"\
        "NumBytesReceived = %I64u\r\n"\
        "TotalRequestParseTime = %I64u\r\n"\
        "TotalResponseTime = %I64u\r\n"\
        "NumUserAgent403s = %u\r\n"\
        "NumIPAddress403s = %u\r\n"\
        "NumGETFilter403s = %u\r\n"\
        "NumErrors = %u",
        
        m_stats.NumRequests,
        m_stats.NumQueuedRequests,
        m_stats.MaxNumQueuedRequests,
        m_stats.NumGets,
        m_stats.NumPosts,
        m_stats.NumPuts,
        m_stats.NumHeads,
        m_stats.NumTraces,
        m_stats.NumUndefinedMethods,
        m_stats.Num200s,
        m_stats.Num301s,
        m_stats.Num304s,
        m_stats.Num400s,
        m_stats.Num401s,
        m_stats.Num403s,
        m_stats.Num404s,
        m_stats.Num409s,
        m_stats.Num500s,
        m_stats.Num501s,
        m_stats.Num503s,
        m_stats.Num505s,
        m_stats.NumUndefinedResponses,
        m_stats.NumForms,
        m_stats.NumSimpleForms,
        m_stats.NumFileForms,
        m_stats.NumBytesInFileForms,
        m_stats.NumBytesSent,
        m_stats.NumBytesReceived,
        m_stats.TotalRequestParseTime,
        m_stats.TotalResponseTime,
        m_stats.NumUserAgent403s,
        m_stats.NumIPAddress403s,
        m_stats.NumGETFilter403s,
        m_stats.NumErrors
        );

    File fStats;
    if (fStats.OpenWrite (pszStatFile) == OK) {
        fStats.Write (pszBuffer);
        fStats.Close();
    }
}

void HttpServer::GetStatisticsFileName (char* pszStatName, const UTCTime& tTime) {
    
    int iSec, iMin, iHour, iDay, iMonth, iYear;
    char pszMonth[20], pszDay[20];

    if (tTime == 0) {
        Time::GetDate (&iSec, &iMin, &iHour, &iDay, &iMonth, &iYear);
    } else {
        Time::GetDate (tTime, &iSec, &iMin, &iHour, &iDay, &iMonth, &iYear);
    }

    sprintf (pszStatName, "%s/Statistics_%i_%s_%s.stat", m_pszStatisticsPath, iYear, 
        String::ItoA (iMonth, pszMonth, 10, 2), String::ItoA (iDay, pszDay, 10, 2));
}

bool HttpServer::DifferentDays (const UTCTime& tOldTime, const UTCTime& tNewTime) {

    if (Time::GetDay (tOldTime) != Time::GetDay (tNewTime)) {
        return true;
    }
    
    Seconds sDiff = Time::GetSecondDifference (tNewTime, tOldTime);
    
    if (sDiff > 24 * 60 * 60 || sDiff < - 24 * 60 * 60) {
        return true;
    }

    return false;
}

int HttpServer::PostMessage (LogMessage* plmMessage) {

    if (m_tsfqLogQueue.Push (plmMessage)) {

        // Wake up the log message thread
        m_evLogEvent.Signal();

        return OK;
    }

    return ERROR_OUT_OF_MEMORY;
}

LogMessage* HttpServer::GetLogMessage() {

    m_mLogMessageCacheLock.Wait();
    LogMessage* pMessage = m_pLogMessageCache->GetObject();
    m_mLogMessageCacheLock.Signal();

    return pMessage;
}

void HttpServer::FreeLogMessage (LogMessage* plmMessage) {
    
    m_mLogMessageCacheLock.Wait();
    m_pLogMessageCache->ReleaseObject (plmMessage);
    m_mLogMessageCacheLock.Signal();
}


int HttpServer::WriteReport (const char* pszMessage) {

    int iErrCode = WARNING;

    if (m_fReportFile.IsOpen()) {

        // Write the time
        int iSec, iMin, iHour, iDay, iMonth, iYear;
        Time::GetDate (&iSec, &iMin, &iHour, &iDay, &iMonth, &iYear);
        
        char pszText[512], pszDay[20], pszHour[20], pszMin[20], pszSec[20], pszMonth[20];
        
        sprintf (pszText, "[%s-%s-%i, %s:%s:%s]\t", String::ItoA (iMonth, pszMonth, 10, 2), 
            String::ItoA (iDay, pszDay, 10, 2), iYear, String::ItoA (iHour, pszHour, 10, 2), 
            String::ItoA (iMin, pszMin, 10, 2), String::ItoA (iSec, pszSec, 10, 2));
        
        m_mReportMutex.Wait();
        
        iErrCode = m_fReportFile.Write (pszText);
        if (iErrCode == OK) {
            iErrCode = m_fReportFile.Write (pszMessage);
            if (iErrCode == OK) {
                iErrCode = m_fReportFile.WriteEndLine();
            }
        }
        
        m_mReportMutex.Signal();
    
    } else {
    
        OS::Alert (pszMessage);
    }

    return iErrCode;
}

size_t HttpServer::GetReportTail (char* pszBuffer, size_t stNumChars) {

    size_t stFilePtr, stRetVal;

    pszBuffer[0] = '\0';

    if (!m_fReportFile.IsOpen()) {
        return 0;
    }

    stRetVal = 0;

    m_mReportMutex.Wait();

    if (m_fReportFile.GetFilePointer (&stFilePtr) == OK) {

        if (stNumChars > stFilePtr) {
            stNumChars = stFilePtr;
        }

        if (m_fReportFile.SetFilePointer (stFilePtr - stNumChars) == OK) {

            if (m_fReportFile.Read (pszBuffer, stNumChars, &stRetVal) == OK) {
                pszBuffer[stRetVal] = '\0';
            }

            m_fReportFile.SetFilePointer (stFilePtr);
        }
    }

    m_mReportMutex.Signal();

    return stRetVal;
}