// HttpResponse.h: interface for the HttpResponse class.
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

#if !defined(AFX_HTTPRESPONSE_H__FE4EBF35_AE05_11D1_9C62_0060083E8062__INCLUDED_)
#define AFX_HTTPRESPONSE_H__FE4EBF35_AE05_11D1_9C62_0060083E8062__INCLUDED_

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD

#include "HttpRequest.h"
#include "FileCache.h"

#include "Osal/Crypto.h"
#include "Osal/Socket.h"
#include "Osal/TempFile.h"
#include "Osal/Variant.h"

enum ResponseType {
    RESPONSE_NONE,
    RESPONSE_ERROR,
    RESPONSE_FILE, 
    RESPONSE_BUFFER, 
    RESPONSE_REDIRECT
};

#define NONCE_SIZE (64)

class HttpServer;

class HttpResponse : public IHttpResponse {

    friend class PageSource;

private:

    HttpStatus m_sStatus;
    HttpStatusReason m_rReason;
    ResponseType m_rType;

    size_t m_stLength;
    size_t m_stRealLength;

    size_t m_stCookieSpace;
    size_t m_stPageSourceNameLength;

    char* m_pszResponseData;
    char* m_pszRealResponseData;
    char* m_pszMimeType;
    char* m_pszRedirectUri;

    // Cookies
    unsigned int m_iNumCookiesSet;
    unsigned int m_iSetCookieSpace;

    unsigned int m_iNumCookiesDel;
    unsigned int m_iDelCookieSpace;

    char** m_ppszCookieSetName;
    char** m_ppszCookieSetValue;
    char** m_ppszCookieSetPath;
    Seconds* m_piCookieTTL;

    char** m_ppszCookieDelName;
    char** m_ppszCookieDelPath;

    UTCTime m_tNow;

    // Response data
    size_t m_stResponseLength;
    bool m_bNoErrorCallback;
    bool m_bNoBuffering;
    bool m_bHeadersSent;
    bool m_bConnectionClosed;

    HttpServer* m_pHttpServer;
    HttpRequest* m_pHttpRequest;

    HttpMethod m_iMethod;
    Socket* m_pSocket;
    PageSource* m_pPageSource;  

    ICachedFile* m_pCachedFile;

    HttpVersion m_iResponseHttpVersion;

    MilliSeconds m_msResponseTime;

    char* m_pszCustomHeaders;
    size_t m_stCustomHeaderLength;

    // Custom log messages
    char** m_ppszCustomLogMessages;

    unsigned int m_iNumCustomLogMessages;
    unsigned int m_iCustomLogMessageSpace;

    // Response functions
    void InternalSetStatusCode (HttpStatus sStatus);
    void SetStatusCodeOnError (HttpStatus sStatus);
    void SetNoPageSource();
    void SetNoErrorCallback();
    void SetResponseHttpVersion (HttpVersion iVersion);

    int CreateDigestAuthenticationNonce (char pszNonce [NONCE_SIZE]);
    int CreateDigestAuthenticationInnerNonce (int64 utTime, char pbInnerNonce [MD5_HASH_SIZE]);
    int CheckDigestAuthenticationNonce (bool* pbStale);

    int SendChunk (const void* pData, size_t stDataLen);
    int SendChunkFromBuffer();
    int FlushChunkFromBuffer();

    int SendResponse();

    int ProcessErrors();
    int ProcessInconsistencies();

    int ProcessMethod();
    int ProcessGet();
    int ProcessPost();

    int ProcessGetFile (const char* pszFileName);
    int ProcessGetDirectory (const char* pszDirName);
    int BuildDirectoryIndex (const char* pszDirName, TempFile* ptfTempFile);

    int Send();

    int RespondToTrace();

    int RespondPrivate();

    HttpRequest* GetHttpRequest() const;

    void ClearObject();

public:

    HttpResponse();
    ~HttpResponse();

    void SetHttpObjects (HttpServer* pHttpServer, HttpRequest* pHttpRequest);

    // Response
    int Respond();

    void SetSocket (Socket* pSocket);
    Socket* GetSocket() const;

    void Recycle();

    bool ConnectionClosed() const;
    MilliSeconds GetResponseTime() const;
    size_t GetResponseLength() const;

    // IHttpResponse
    IMPLEMENT_INTERFACE (IHttpResponse);

    HttpStatus GetStatusCode();
    int SetStatusCode (HttpStatus sStatus);

    HttpStatusReason GetStatusCodeReason();
    int SetStatusCodeReason (HttpStatusReason rReason);

    int SetMimeType (const char* pszMimeType);

    int SetFile (ICachedFile* pCachedFile);
    int SetRedirect (const char* pszUri);

    int SetNoBuffering();

    int Clear();
    int Flush();
    
    int WriteText (const char* pszData);
    int WriteText (const char* pszData, size_t stStrlen);
    int WriteText (int iData);
    int WriteText (unsigned int iData);
    int WriteText (float fData);
    int WriteText (double dData);
    int WriteText (const UTCTime& tTime);
    int WriteText (const Variant& vData);
    int WriteText (int64 iData);
    int WriteText (uint64 iData);

    int WriteData (const void* pszData, size_t stNumBytes);

    int WriteTextFile (ICachedFile* pCachedFile);
    int WriteDataFile (ICachedFile* pCachedFile);
    
    int CreateCookie (const char* pszCookieName, const char* pszCookieValue, Seconds iTTLinSeconds, const char* pszCookiePath);
    int DeleteCookie (const char* pszCookieName, const char* pszCookiePath);

    int AddHeader (const char* pszHeaderName, const char* pszHeaderValue);

    int AddCustomLogMessage (const char* pszCustomLogMessage);
    int GetCustomLogMessages (const char*** pppszCustomLogMessages, unsigned int* piNumCustomLogMessages);
};

class HttpResponseAllocator {
public:
    static HttpResponse* New() { return new HttpResponse(); }
    static void Delete (HttpResponse* pHttpResponse) { delete pHttpResponse; }
};

#endif // !defined(AFX_HTTPRESPONSE_H__FE4EBF35_AE05_11D1_9C62_0060083E8062__INCLUDED_)