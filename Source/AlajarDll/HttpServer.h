// HttpServer.h: interface for the HttpServer class.
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

#include "FileCache.h"
#include "PageSourceEnumerator.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpThreadPool.h"
#include "Report.h"

#include "Osal/AsyncFile.h"
#include "Osal/TempFile.h"
#include "Osal/HashTable.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/FifoQueue.h"
#include "Osal/ObjectCache.h"
#include "Osal/SslSocket.h"

// Version
#define ALAJAR_VERSION 1.8.7

// Coalesce
#define COALESCE_REQUESTS  25
#define COALESCE_PERIOD_MS 30000

class SocketAllocator {
public:
    static Socket* New() { return new Socket(); }
    static void Delete (Socket* pSocket) { delete pSocket; }
};

class HttpServer : public IHttpServer
{
private:

    unsigned int m_iNumRefs;

    // True if closing server
    bool m_bExit;

    // True if the server started up properly
    bool m_bRunning;

    // True if we're restarting
    bool m_bRestart;

    // IP address, Host name
    char m_pszIPAddress [20];
    char m_pszHostName [128];    

    // Listener ports
    short m_siPort;
    short m_siSslPort;
    bool m_bRedirectHttpToHttps;

    // Listener sockets
    Socket* m_pSocket;

    SslContext* m_pSslContext;
    Socket* m_pSslSocket;

    // Name of server
    const char* m_pszServerName;

    // Length of server name
    size_t m_stServerNameLength;

    // Threadpool
    HttpThreadPool* m_pThreadPool;
    Thread::ThreadPriority m_iDefaultPriority;

    // Main thread
    Thread m_tMainThread;

    // Shutdown
    Mutex m_mShutdown;

    IStartupSink* m_pStartupSink;
    IShutdownSink* m_pShutdownSink;
    IRestartSink* m_pRestartSink;

    // Statistics
    Mutex m_mStatMutex;
    HttpServerStatistics m_stats;

    // Nonces
    Uuid m_uuidUniqueIdentifier;

    // Paths
    char m_pszCounterPath [OS::MaxFileNameLength];
    char m_pszLogPath [OS::MaxFileNameLength];
    char m_pszPageSourcePath [OS::MaxFileNameLength];
    char m_pszConfigPath [OS::MaxFileNameLength];
    char m_pszReportPath [OS::MaxFileNameLength];
    char m_pszStatisticsPath [OS::MaxFileNameLength];

    // Filecache
    FileCache* m_pFileCache;

    // Object caches
    ObjectCache<HttpRequest, HttpRequestAllocator>* m_pHttpRequestCache;
    ObjectCache<HttpResponse, HttpResponseAllocator>* m_pHttpResponseCache;
    Mutex m_mHttpObjectCacheLock;

    // Config file
    IConfigFile* m_pConfigFile;

    // Report file
    TraceInfoLevel m_reportTracelevel;
    UTCTime m_tReportTime;
    Report* m_pReport;
    Mutex m_reportMutex;

    UTCTime m_tStatsTime;

    void GetReportFileName (char pszFileName[OS::MaxFileNameLength]);
    void ReportEvent(const char* pszMessage);

    // Page sources
    PageSource* m_pDefaultPageSource;
    HashTable<const char*, PageSource*, PageSourceHashValue, PageSourceEquals>* m_pPageSourceTable;

    ReadWriteLock m_rwPageSourceTableLock;

    int Run();

    static int THREAD_CALL StartServer (void* pvServer);
    int StartServer();

    int ConfigurePageSources();
    void ShutdownPageSources();

    // Close down a pagesource
    static int THREAD_CALL ClosePageSource (void* pVoid);

    HttpServer();
    ~HttpServer();

    void Clean();
    int Init();

    void ReadStatistics();
    void WriteStatistics (const UTCTime& tDate);

public:

    static HttpServer* CreateInstance();

    int DeletePageSource (const char* pszPageSourceName);
    bool IsDefaultPageSource (PageSource* pPageSource);
    PageSource* GetDefaultPageSource();
    PageSource* GetPageSource (const char* pszPageSourceName);
    HttpThreadPool* GetThreadPool();

    void GetStatisticsFileName (char* pszStatName, const UTCTime& tTime);

    int GetHttpObjects (HttpRequest** ppHttpRequest, HttpResponse** ppHttpResponse);
    void ReleaseHttpObjects (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse);

    void FinishRequest (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse, Socket* pSocket, 
        HttpServerStatistics* psThreadstats, int iErrCode);

    void StatisticsAndLog (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse, Socket* pSocket, 
        HttpServerStatistics* psThreadstats, int iErrCode);

    void CoalesceStatistics (HttpServerStatistics* psThreadstats);

    static bool DifferentDays (const UTCTime& tOldTime, const UTCTime& tNewTime);

    int WWWServe (HttpPoolThread* pThread);

    // IHttpServer
    DECLARE_IOBJECT;

    short GetHttpPort();
    short GetHttpsPort();
    
    const char* GetHostName();
    const char* GetIPAddress();
    const char* GetServerName();
    const Uuid& GetUniqueIdentifier();

    unsigned int GetNumThreads();

    unsigned int GetNumQueuedRequests();
    unsigned int GetMaxNumQueuedRequests();

    unsigned int GetStatistics (HttpServerStatistics* pStatistics);

    unsigned int GetNumPageSources();

    const char* GetCounterPath() { return m_pszCounterPath; }
    const char* GetLogPath() { return m_pszLogPath; }
    const char* GetPageSourcePath() { return m_pszPageSourcePath; }
    const char* GetConfigPath() { return m_pszConfigPath; }
    const char* GetReportPath() { return m_pszReportPath; }
    const char* GetStatisticsPath() { return m_pszStatisticsPath; }

    ITraceLog* GetReport();
    IConfigFile* GetConfigFile();
    IFileCache* GetFileCache();

    int Start (IConfigFile* pConfigFile, IStartupSink* pStartupSink, IShutdownSink* pShutdownSink);
    int Restart (IRestartSink* pRestartSink);
    int Shutdown();

    IPageSourceEnumerator* EnumeratePageSources();
    IPageSourceControl* GetPageSourceByName (const char* pszName);
};
