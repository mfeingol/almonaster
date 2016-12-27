// Alajar.h: interface for the CachedFile class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar
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

#if !defined(AFX_ALAJAR_H__C25D88C3_44DB_11D2_9EEE_0060083E8062__INCLUDED_)
#define AFX_ALAJAR_H__C25D88C3_44DB_11D2_9EEE_0060083E8062__INCLUDED_

#ifndef ALAJAR_EXPORT
#ifdef ALAJAR_BUILD
#define ALAJAR_EXPORT EXPORT
#else
#define ALAJAR_EXPORT IMPORT
#endif
#endif

#include "Osal/IObject.h"
#include "Osal/Variant.h"
#include "Osal/TraceLog.h"

////////////
// Export //
////////////

// Allowed values:
//
// CLSID: CLSID_HttpServer:
// IID's: IID_IHttpServer
//
// CLSID: CLSID_ConfigFile
// IID's: IID_IConfigFile

ALAJAR_EXPORT int AlajarCreateInstance (const Uuid& clsidClassId, const Uuid& iidInterface, void** ppObject);

///////////
// Types //
///////////

ALAJAR_EXPORT extern const Uuid CLSID_HttpServer;
ALAJAR_EXPORT extern const Uuid CLSID_ConfigFile;

ALAJAR_EXPORT extern const Uuid IID_ICachedFile;
ALAJAR_EXPORT extern const Uuid IID_IFileCache;
ALAJAR_EXPORT extern const Uuid IID_ITraceLogReader;
ALAJAR_EXPORT extern const Uuid IID_IConfigFile;
ALAJAR_EXPORT extern const Uuid IID_IPageSourceControl;
ALAJAR_EXPORT extern const Uuid IID_IHttpForm;
ALAJAR_EXPORT extern const Uuid IID_ICookie;
ALAJAR_EXPORT extern const Uuid IID_IHttpRequest;
ALAJAR_EXPORT extern const Uuid IID_IHttpResponse;
ALAJAR_EXPORT extern const Uuid IID_IHttpServer;
ALAJAR_EXPORT extern const Uuid IID_IStartupSink;
ALAJAR_EXPORT extern const Uuid IID_IShutdownSink;
ALAJAR_EXPORT extern const Uuid IID_IPageSourceEnumerator;
ALAJAR_EXPORT extern const Uuid IID_IPageSource;

enum HttpFormType {
    SIMPLE_FORM,
    FILE_FORM,
    LARGE_SIMPLE_FORM,
    UNSUPPORTED_FORM_TYPE
};

enum HttpMethod {
    GET,
    POST,
    PUT,
    HEAD,
    TRACE,
    UNSUPPORTED_HTTP_METHOD
};

static const char* const HttpMethodText[] = {
    "GET",
    "POST",
    "PUT",
    "HEAD",
    "TRACE",
    "UNSUPPORTED"
};

enum HttpVersion {
    HTTP09,
    HTTP10,
    HTTP11,
    UNSUPPORTED_HTTP_VERSION
};

enum HttpStatus {
    HTTP_200,
    HTTP_301,
    HTTP_304,
    HTTP_400,
    HTTP_401,
    HTTP_403,
    HTTP_404,
    HTTP_409,
    HTTP_500,
    HTTP_501,
    HTTP_503,
    HTTP_505,
    UNSUPPORTED_HTTP_STATUS
};

enum HttpStatusReason {
    HTTP_REASON_NONE,
    HTTP_REASON_IPADDRESS_BLOCKED,
    HTTP_REASON_USER_AGENT_BLOCKED,
    HTTP_REASON_GET_REFERER_BLOCKED,
    HTTP_REASON_STALE_NONCE,
};

#define NUM_STATUS_CODES ((unsigned int) UNSUPPORTED_HTTP_STATUS + 1)

static const int HttpStatusValue[] = {
    200,
    301,
    304,
    400,
    401,
    403,
    404,
    409,
    500,
    501,
    503,
    505,
    666
};

static const char* const HttpStatusText[] = {
    "200 OK",
    "301 Moved Permanently",
    "304 Not Modified",
    "400 Bad Request",
    "401 Not Authorized",
    "403 Forbidden",
    "404 Not Found",
    "409 Conflict",
    "500 Internal Server Error",
    "501 Not Implemented",
    "503 Service Unavailable",
    "505 HTTP Version Not Supported",
    "666 HTTP Nonexistent Error"
};

static const char* const HttpStatusErrorText[] = {
    "",
    "",
    "",
    "<html>\n<head><title>Error 400</title></head>\n<body>\n<h2>Error 400: Bad Request.</h2>\n<p>The request contained bad syntax.\n</body>\n</html>",
    "<html>\n<head><title>Error 401</title></head>\n<body>\n<h2>Error 401: Unauthorized.</h2>\n<p>The resource requires authentication before access can be granted.\n</body>\n</html>",
    "<html>\n<head><title>Error 403</title></head>\n<body>\n<h2>Error 403: Forbidden.</h2>\n<p>The server understood the request but will not fulfill it.\n</body>\n</html>",
    "<html>\n<head><title>Error 404</title></head>\n<body>\n<h2>Error 404: Not Found.</h2>\n<p>The indicated resource was not found on the server.\n</body>\n</html>",
    "<html>\n<head><title>Error 409</title></head>\n<body>\n<h2>Error 409: Conflict.</h2>\n<p>A conflict in the current state of the resource prevented the request from being fulfilled.\n</body>\n</html>",
    "<html>\n<head><title>Error 500</title></head>\n<body>\n<h2>Error 500: Internal Server Error.</h2>\n<p>General server error encountered.\n</body>\n</html>",
    "<html>\n<head><title>Error 501</title></head>\n<body>\n<h2>Error 501: Not Implemented.</h2>\n<p>The requested method is not supported by the server.\n</body>\n</html>",
    "<html>\n<head><title>Error 503</title></head>\n<body>\n<h2>Error 503: Service Unavailable.</h2>\n<p>The server cannot fulfill the request at the moment. This is an indication of temporary conditions such as overloading or server maintenance.\n</body>\n</html>",
    "<html>\n<head><title>Error 505</title></head>\n<body>\n<h2>Error 505: HTTP Version Not Supported.</h2>\n<p>The server does not or is unwilling to support the HTTP version of the request.\n</body>\n</html>",
    "<html>\n<head><title>Error 666</title></head>\n<body>\n<h2>Error 666: Nonexistent Error.</h2>\n<p>This error does not exist.\n</body>\n</html>",
};

static const size_t HttpStatusErrorTextLength[] = {
    0,
    0,
    0,
    sizeof ("<html>\n<head><title>Error 400</title></head>\n<body>\n<h2>Error 400: Bad Request.</h2>\n<p>The request contained bad syntax.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 401</title></head>\n<body>\n<h2>Error 401: Unauthorized.</h2>\n<p>The resource requires authentication before access can be granted.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 403</title></head>\n<body>\n<h2>Error 403: Forbidden.</h2>\n<p>The server understood the request but will not fulfill it.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 404</title></head>\n<body>\n<h2>Error 404: Not Found.</h2>\n<p>The indicated resource was not found on the server.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 409</title></head>\n<body>\n<h2>Error 409: Conflict.</h2>\n<p>A conflict in the current state of the resource prevented the request from being fulfilled.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 500</title></head>\n<body>\n<h2>Error 500: Internal Server Error.</h2>\n<p>General server error encountered.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 501</title></head>\n<body>\n<h2>Error 501: Not Implemented.</h2>\n<p>The requested method is not supported by the server.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 503</title></head>\n<body>\n<h2>Error 503: Service Unavailable.</h2>\n<p>The server cannot fulfill the request at the moment. This is an indication of temporary conditions such as overloading or server maintenance.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 505</title></head>\n<body>\n<h2>Error 505: HTTP Version Not Supported.</h2>\n<p>The server does not or is unwilling to support the HTTP version of the request.\n</body>\n</html>") - 1,
    sizeof ("<html>\n<head><title>Error 666</title></head>\n<body>\n<h2>Error 666: Nonexistent Error.</h2>\n<p>This error does not exist.\n</body>\n</html>") - 1
};


// Statistics structures
struct HttpServerStatistics {

    UTCTime StartupTime;

    unsigned int NumRequests;
    unsigned int NumQueuedRequests;
    unsigned int MaxNumQueuedRequests;

    unsigned int NumGets;
    unsigned int NumPosts;
    unsigned int NumPuts;
    unsigned int NumHeads;
    unsigned int NumTraces;
    unsigned int NumUndefinedMethods;

    unsigned int Num200s;
    unsigned int Num301s;
    unsigned int Num304s;
    unsigned int Num400s;
    unsigned int Num401s;
    unsigned int Num403s;
    unsigned int Num404s;
    unsigned int Num409s;
    unsigned int Num500s;
    unsigned int Num501s;
    unsigned int Num503s;
    unsigned int Num505s;
    unsigned int NumUndefinedResponses;

    unsigned int NumForms;
    unsigned int NumSimpleForms;
    unsigned int NumFileForms;
    uint64 NumBytesInFileForms;

    uint64 NumBytesSent;
    uint64 NumBytesReceived;

    uint64 TotalRequestParseTime;
    MilliSeconds AverageRequestParseTime;

    uint64 TotalResponseTime;
    MilliSeconds AverageResponseTime;

    unsigned int NumIPAddress403s;
    unsigned int NumUserAgent403s;
    unsigned int NumGETFilter403s;

    unsigned int NumErrors;
};


// Interfaces
class IStartupSink : virtual public IObject {
public:
    virtual void OnStartup (int iErrCode) = 0;
};

class IShutdownSink : virtual public IObject {
public:
    virtual void OnShutdown (int iErrCode) = 0;
};

class IRestartSink : virtual public IObject {
public:
    virtual void OnRestart (int iErrCode) = 0;
};

class ICachedFile : virtual public IObject {
public:

    virtual const char* GetName() = 0;
    virtual const char* GetMimeType() = 0;

    virtual void GetLastModifiedTime (UTCTime* ptLastModifiedTime) = 0;

    virtual size_t GetSize() = 0;
    virtual const void* GetData() = 0;
};

class IFileCache : virtual public IObject {
public:

    virtual ICachedFile* GetFile (const char* pszFileName) = 0;

    virtual int ReleaseFile (const char* pszFileName) = 0;
    virtual int ReleaseAllFiles() = 0;

    virtual bool IsActive() = 0;
    virtual unsigned int GetNumFiles() = 0;
    virtual size_t GetSize() = 0;
};

class ITraceLogReader : virtual public IObject
{
public:
    virtual int GetTail(char* pszBuffer, unsigned int cbSize) = 0;
};

class IConfigFile : virtual public IObject {
public:

    virtual int Open (const char* pszFileName) = 0;
    virtual int Close() = 0;
    virtual int Refresh() = 0;

    virtual unsigned int GetNumParameters() = 0;
    virtual const char** GetParameterNames() = 0;

    virtual int GetParameter (unsigned int iIndex, char** ppszLhs, char** ppszRhs) = 0;
    virtual int GetParameter (const char* pszLhs, char** ppszRhs) = 0;

    virtual int SetParameter (const char* pszNewLhs, const char* pszNewRhs) = 0;
};

class IPageSourceControl : virtual public IObject {
public:

    virtual bool IsDefault() = 0;

    virtual const char* GetName() = 0;
    virtual IConfigFile* GetConfigFile() = 0;

    virtual ITraceLog* GetReport() = 0;
    virtual ITraceLog* GetLog() = 0;

    virtual void LockWithNoThreads() = 0;
    virtual void LockWithSingleThread() = 0;
    virtual void ReleaseLock() = 0;

    virtual int Restart() = 0;
    virtual int Shutdown() = 0;

    virtual unsigned int IncrementCounter (const char* pszName) = 0;
    virtual unsigned int GetCounterValue (const char* pszName) = 0;
};

class IPageSourceEnumerator : virtual public IObject {
public:

    virtual unsigned int GetNumPageSources() = 0;
    virtual IPageSourceControl** GetPageSourceControls() = 0;
};

class IHttpForm : virtual public IObject {
public:
    virtual unsigned int GetNumForms() = 0;
    virtual IHttpForm* GetForm (unsigned int iIndex) = 0;

    virtual const char* GetName() = 0;
    virtual const char* GetValue() = 0;

    virtual int GetIntValue() = 0;
    virtual unsigned int GetUIntValue() = 0;

    virtual int64 GetInt64Value() = 0;
    virtual uint64 GetUInt64Value() = 0;

    virtual float GetFloatValue() = 0;
    virtual UTCTime GetTimeValue() = 0;

    virtual HttpFormType GetType() = 0;
    virtual const char* GetFileName() = 0;
};

class ICookie : virtual public IObject {
public:
    virtual unsigned int GetNumSubCookies() = 0;
    virtual ICookie* GetSubCookie (unsigned int iIndex) = 0;

    virtual const char* GetName() = 0;
    virtual const char* GetValue() = 0;

    virtual int GetIntValue() = 0;
    virtual unsigned int GetUIntValue() = 0;

    virtual int64 GetInt64Value() = 0;
    virtual uint64 GetUInt64Value() = 0;
    
    virtual float GetFloatValue() = 0;
    virtual UTCTime GetTimeValue() = 0;
};

class IHttpRequest : virtual public IObject {
public:
    virtual HttpMethod GetMethod() = 0;
    virtual HttpVersion GetVersion() = 0;

    virtual const char* GetUri() = 0;
    virtual const char* GetFileName() = 0;
    virtual bool IsFileNameCanonical() = 0;
    virtual const char* GetBrowserName() = 0;
    virtual const char* GetClientIP() = 0;
    virtual const char* GetReferer() = 0;

    virtual unsigned int GetNumForms() = 0;
    virtual IHttpForm* GetForm (unsigned int iIndex) = 0;
    
    virtual const char** GetFormNames() = 0;
    virtual IHttpForm* GetForm (const char* pszName) = 0;
    virtual IHttpForm* GetFormBeginsWith (const char* pszName) = 0;

    virtual unsigned int GetNumCookies() = 0;
    virtual ICookie* GetCookie (unsigned int iIndex) = 0;

    virtual const char** GetCookieNames() = 0;
    virtual ICookie* GetCookie (const char* pszName) = 0;
    virtual ICookie* GetCookieBeginsWith (const char* pszName) = 0;

    virtual const char* GetAuthenticationUserName() = 0;

    virtual int BasicAuthenticate (const char* pszPassword, bool* pbAuthenticated) = 0;
    virtual int DigestAuthenticate (const char* pszPassword, bool* pbAuthenticated) = 0;

    virtual const char* GetHeaders() = 0;
};

class IHttpResponse : virtual public IObject {
public:

    virtual HttpStatus GetStatusCode() = 0;
    virtual int SetStatusCode (HttpStatus sStatus) = 0;

    virtual HttpStatusReason GetStatusCodeReason() = 0;
    virtual int SetStatusCodeReason (HttpStatusReason rReason) = 0;
    
    virtual int SetMimeType (const char* pszMimeType) = 0;

    virtual int SetFile (ICachedFile* pCachedFile) = 0;
    virtual int SetRedirect (const char* pszUri) = 0;

    virtual int SetNoBuffering() = 0;

    virtual int Clear() = 0;
    virtual int Flush() = 0;

    virtual int WriteData (const void* pszData, size_t stNumBytes) = 0;

    virtual int WriteText (const char* pszData) = 0;
    virtual int WriteText (const char* pszData, size_t stStrlen) = 0;
    virtual int WriteText (int iData) = 0;
    virtual int WriteText (unsigned int uiData) = 0;
    virtual int WriteText (float fData) = 0;
    virtual int WriteText (const Variant& vData) = 0;
    virtual int WriteText (int64 iData) = 0;
    virtual int WriteDate (const UTCTime& tTime) = 0;

    virtual int WriteTextFile (ICachedFile* pCachedFile) = 0;
    virtual int WriteDataFile (ICachedFile* pCachedFile) = 0;

    virtual int CreateCookie (const char* pszCookieName, const char* pszCookieValue, Seconds iTTLinSeconds, const char* pszCookiePath) = 0;
    virtual int DeleteCookie (const char* pszCookieName, const char* pszCookiePath) = 0;

    virtual int AddHeader (const char* pszHeaderName, const char* pszHeaderValue) = 0;

    virtual int AddCustomLogMessage (const char* pszCustomLogMessage) = 0;
    virtual int GetCustomLogMessages (const char*** pppszCustomLogMessages, unsigned int* piNumCustomLogMessages) = 0;
};

class IHttpServer : virtual public IObject {
public:

    virtual short GetHttpPort() = 0;
    virtual short GetHttpsPort() = 0;

    virtual const char* GetHostName() = 0;
    virtual const char* GetIPAddress() = 0;
    virtual const char* GetServerName() = 0;
    
    virtual unsigned int GetNumThreads() = 0;

    virtual unsigned int GetStatistics (HttpServerStatistics* pStatistics) = 0;

    virtual unsigned int GetNumPageSources() = 0;
    virtual IPageSourceEnumerator* EnumeratePageSources() = 0;

    virtual ITraceLog* GetReport() = 0;
    virtual IConfigFile* GetConfigFile() = 0;
    virtual IFileCache* GetFileCache() = 0;

    virtual int Start (IConfigFile* pConfigFile, IStartupSink* pStartupSink, IShutdownSink* pShutdownSink) = 0;
    virtual int Restart (IRestartSink* pRestartSink) = 0;
    virtual int Shutdown() = 0;

    virtual IPageSourceControl* GetPageSourceByName (const char* pszName) = 0;
};

class IPageSource : virtual public IObject {
public:

    virtual int OnInitialize (IHttpServer* pHttpServer, IPageSourceControl* pControl) = 0;
    virtual int OnFinalize() = 0;

    virtual int OnGet (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) = 0;
    virtual int OnPost (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) = 0;
    virtual int OnError (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) = 0;

    virtual int OnBasicAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated) = 0;
    virtual int OnDigestAuthenticate (IHttpRequest* pHttpRequest, bool* pbAuthenticated) = 0;

    virtual const char* GetAuthenticationRealm (IHttpRequest* pHttpRequest) = 0;
};

#endif