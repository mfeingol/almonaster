// HttpServer.h: interface for the HttpServer class.
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

#if !defined(AFXHttpServer_H__7E3E4053_A569_11D1_9C4E_0060083E8062__INCLUDED_)
#define AFXHttpServer_H__7E3E4053_A569_11D1_9C4E_0060083E8062__INCLUDED_

#include "FileCache.h"
#include "PageSourceEnumerator.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpThreadPool.h"

#include "Osal/TempFile.h"
#include "Osal/HashTable.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/FifoQueue.h"
#include "Osal/ObjectCache.h"

// Coalesce
#define COALESCE_REQUESTS  25
#define COALESCE_PERIOD_MS 30000

// Log structures
#define MAX_LOG_MESSAGE (4096 + OS::MaxFileNameLength)

enum LogMessageType { LOG_MESSAGE, REPORT_MESSAGE };
struct LogMessage {
	LogMessageType MessageType;
	PageSource* PageSource;
	char Text [MAX_LOG_MESSAGE];
};

class SocketAllocator {
public:
	static Socket* New() { return new Socket(); }
	static void Delete (Socket* pSocket) { delete pSocket; }
};

class LogMessageAllocator {
public:
	static LogMessage* New() { return new LogMessage; }
	static void Delete (LogMessage* pLogMessage) { delete pLogMessage; }
};

class HttpServer : public IHttpServer, public IReport {

private:

	unsigned int m_iNumRefs;

	// True if closing server
	bool m_bExit;

	// True if the server started up properly
	bool m_bRunning;

	// True if we're restarting
	bool m_bRestart;

	// Host name
	char m_pszHostName [256];

	// IP address
	char m_pszIPAddress [20];

	// Listener port
	short m_siPort;

	// Listener socket
	Socket* m_pSocket;

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

	ObjectCache<Socket, SocketAllocator>* m_pSocketCache;
	Mutex m_mSocketCacheLock;

	ObjectCache<LogMessage, LogMessageAllocator>* m_pLogMessageCache;
	Mutex m_mLogMessageCacheLock;

	// Report
	File m_fReportFile;
	Mutex m_mReportMutex;

	// Config file
	IConfigFile* m_pConfigFile;

	// Page sources
	PageSource* m_pDefaultPageSource;
	HashTable<const char*, PageSource*, PageSourceHashValue, PageSourceEquals>* m_pPageSourceTable;

	ReadWriteLock m_rwPageSourceTableLock;

	int Run();

	static int THREAD_CALL StartServer (void* pvServer);
	int StartServer();

	int ConfigurePageSources();
	void ShutdownPageSources();

	int WWWServe();

	// Event reporting
	void ReportEvent (const char* pszEvent);

	// Close down a pagesource
	static int THREAD_CALL ClosePageSource (void* pVoid);

	// Log thread stuff
	Thread m_tLogThread;
	bool m_bLogExit;

	UTCTime m_tLogDate;

	ThreadSafeFifoQueue<LogMessage*> m_tsfqLogQueue;

	LogMessage* GetNextLogMessage();
	int GetNumLogMessages();
	static int THREAD_CALL LogThread (void* pVoid);
	int LogLoop();

	// Shutdown
	Event m_evLogEvent;

	HttpServer();
	~HttpServer();

	void Clean();
	void Initialize();

	void ReadStatistics();
	void WriteStatistics (const UTCTime& tDate);

public:

	static HttpServer* CreateInstance();

	int DeletePageSource (const char* pszPageSourceName);
	bool IsDefaultPageSource (PageSource* pPageSource);
	PageSource* GetDefaultPageSource();
	PageSource* GetPageSource (const char* pszPageSourceName);
	HttpThreadPool* GetThreadPool();

	void GetStatisticsFileName (char* pszStatName, const UTCTime& tTime = 0);

	int GetHttpObjects (HttpRequest** ppHttpRequest, HttpResponse** ppHttpResponse);
	void ReleaseHttpObjects (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse);

	static int THREAD_CALL ThreadExec (void* pVoid);

	void FinishRequest (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse, Socket* pSocket, 
		HttpServerStatistics* psThreadstats, int iErrCode);

	void StatisticsAndLog (HttpRequest* pHttpRequest, HttpResponse* pHttpResponse, Socket* pSocket, 
		HttpServerStatistics* psThreadstats, int iErrCode);

	void CoalesceStatistics (HttpServerStatistics* psThreadstats);

	static bool DifferentDays (const UTCTime& tOldTime, const UTCTime& tNewTime);

	// Messages
	int PostMessage (LogMessage* plmMessage);

	LogMessage* GetLogMessage();
	void FreeLogMessage (LogMessage* plmMessage);

	// IHttpServer
	DECLARE_IOBJECT;

	short GetPort();
	
	const char* GetHostName();
	const char* GetIPAddress();
	const char* GetServerName();
	
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

	IReport* GetReport();
	IConfigFile* GetConfigFile();
	IFileCache* GetFileCache();

	int Start (IConfigFile* pConfigFile, IStartupSink* pStartupSink, IShutdownSink* pShutdownSink);
	int Restart (IRestartSink* pRestartSink);
	int Shutdown();

	IPageSourceEnumerator* EnumeratePageSources();
	IPageSourceControl* GetPageSourceByName (const char* pszName);

	// IReport
	int WriteReport (const char* pszMessage);
	size_t GetReportTail (char* pszBuffer, size_t stNumChars);
};

/*size_t AppendString (char** ppszBuffer, const char* pszNewString, size_t stTotalLength);
char* EqualsToSlash (const char* pszInput);*/


#endif // !defined(AFXHttpServer_H__7E3E4053_A569_11D1_9C4E_0060083E8062__INCLUDED_)