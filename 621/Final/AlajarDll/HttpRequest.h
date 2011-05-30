// HttpRequest.h: interface for the HttpRequest class.
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

#if !defined(AFX_HTTPREQUEST_H__7E3E4056_A569_11D1_9C4E_0060083E8062__INCLUDED_)
#define AFX_HTTPREQUEST_H__7E3E4056_A569_11D1_9C4E_0060083E8062__INCLUDED_

#include "Osal/HashTable.h"
#include "Osal/Socket.h"
#include "Osal/String.h"

#define ALAJAR_BUILD
#include "Alajar.h"
#include "PageSource.h"
#undef ALAJAR_BUILD

#include "HttpForm.h"
#include "Cookie.h"

// Error codes
#define ERROR_MALFORMED_REQUEST (-70000000)
#define ERROR_UNSUPPORTED_HTTP_VERSION (-70000001)
#define ERROR_UNSUPPORTED_HTTP_METHOD (-70000002)

#define MAX_URI_LEN (1024 * 4)
#define DIGEST_HASH_TEXT_SIZE 33

class PageSource;
class PageSourceHashValue;
class PageSourceEquals;

class HttpServer;

class HttpRequest : public IHttpRequest {

private:

    // Method, version, Uri
    HttpMethod m_iMethod;
    HttpVersion m_vVersion;
    HttpAuthenticationType m_atAuth;
    
    char* m_pszUri;
    size_t m_stUriLength;

    // Uri as hard drive path
    char m_pszFileName [OS::MaxFileNameLength + 1];
    bool m_bCanonicalPath;

    // Browsername
    String m_strBrowserName;

    // Headers
    String m_strHeaders;

    // Client IP address
    String m_strIPAddress;

    // Referer
    String m_strReferer;

    // Name of host requested by client
    String m_strHostName;

    // True if file requested is cached (and last modification date of file is non-null also)
    bool m_bCached;

    // Separator text
    char* m_pszSeparator;
    size_t m_stSeparatorLength;
    size_t m_stSeparatorSpace;

    // Content length
    size_t m_stContentLength;

    // Authorization
    String m_strAuthUserName;        // Basic and Digest
    String m_strAuthPassword;        // Basic
    String m_strAuthRealm;           // Basic and Digest

    String m_strAuthNonce;           // Digest
    String m_strAuthDigestUri;       // Digest
    String m_strAuthRequestResponse; // Digest

    String m_strNonceCount;          // Digest
    String m_strCNonce;              // Digest
    String m_strQop;                 // Digest

    // Parse time
    MilliSeconds m_msParseTime;

    class RequestHashValue {
    public:
        static int GetHashValue (const char* pszKey, unsigned int iNumBuckets, const void* pHashHint);
    };

    class RequestEquals {
    public:
        static bool Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint);
    };

    // Forms
    unsigned int m_iNumHttpForms;
    unsigned int m_iNumHttpFormsSpace;
    const char** m_ppszHttpFormName;
    HttpForm** m_ppHttpForms;

    HashTable<const char*, HttpForm*, RequestHashValue, RequestEquals>* m_phtHttpFormTable;

    // Cookies
    unsigned int m_iNumCookies;
    const char** m_ppszCookieName;
    Cookie** m_ppCookies;

    size_t m_stCookieSpace;

    HashTable<const char*, Cookie*, RequestHashValue, RequestEquals>* m_phtCookieTable;

    // Keep-alive?
    bool m_bKeepAlive;

    // Server
    HttpServer* m_pHttpServer;

    // PageSource
    PageSource* m_pPageSource;

    // Socket
    Socket* m_pSocket;

    // Uploaded files
    unsigned int m_iNumFiles;
    size_t m_stNumBytes;

    bool m_bParsedUriForms;
    bool m_bMultiPartForms;
    
    char* ParseAuthenticationAttribute (char* pszCursor, String* pstrValue);
    int ParseRequestHeader (char* pszLine);
    int ParseHeader (char* pszLine);

    int HandleSimpleForms (char* pszBuffer, size_t stBufferSize, size_t stNumBytesInBuffer);
    int HandleLargeSimpleForm (char* pszBuffer, size_t stBufferSize, size_t stBytesParsed, size_t* pstCurrentPos, 
        size_t* pstBytesParsed);

    int HandleMultipartForms (char* pszBuffer, size_t stNumBytes);
    
    int HandleMultiPartFormsInBuffer (size_t iNumBytes, char* pszBuffer, size_t* pstBytesProcessed, 
        size_t* pstBytesRemaining);
    int HandleBigMultiPartForm (size_t stNumBytesRemaining, char* pszBuffer, size_t* pstNumBytesProcessed, 
        size_t* pstBytesNumRemaining);

    int ParseForms (char* pszFormStart, size_t* pstParsed, bool bLastBytes);

    int AddHttpForm (HttpFormType ftFormType, const char* pszHttpFormName, const char* pszHttpFormValue, 
        const char* pszFileName);

    int ComputeA1Hash (const char* pszPassword, char pszA1 [DIGEST_HASH_TEXT_SIZE]);
    int ComputeA2Hash (char pszA2 [DIGEST_HASH_TEXT_SIZE]);
    int ComputeFinalHash (
        const char pszA1[DIGEST_HASH_TEXT_SIZE], const char pszA2[DIGEST_HASH_TEXT_SIZE], char pszFinal[DIGEST_HASH_TEXT_SIZE]);

public:

    HttpRequest();
    ~HttpRequest();

    void SetHttpServer (HttpServer* pHttpServer);

    void Recycle();

    MilliSeconds GetRequestParseTime() const;

    unsigned int GetNumFilesUploaded();
    size_t GetNumBytesInUploadedFiles();

    int ParseHeaders();

    bool GetKeepAlive() const;
    PageSource* GetPageSource() const;

    bool IsCached() const;

    bool ParsedUriForms() const;
    const char* GetParsedUriForms() const;

    void SetSocket (Socket* pSocket);

    // IHttpRequest
    IMPLEMENT_INTERFACE (IHttpRequest);

    HttpMethod GetMethod();
    HttpVersion GetVersion();

    const char* GetUri();
    const char* GetFileName();
    bool IsFileNameCanonical();
    const char* GetBrowserName();
    const char* GetClientIP();
    const char* GetReferer();

    unsigned int GetNumForms();
    IHttpForm* GetForm (unsigned int iIndex);

    const char** GetFormNames();
    IHttpForm* GetForm (const char* pszName);
    IHttpForm* GetFormBeginsWith (const char* pszName);

    unsigned int GetNumCookies();
    ICookie* GetCookie (unsigned int iIndex);

    const char** GetCookieNames();
    ICookie* GetCookie (const char* pszName);
    ICookie* GetCookieBeginsWith (const char* pszName);

    const char* GetAuthenticationUserName();
    const char* GetAuthenticationNonce();

    int BasicAuthenticate (const char* pszPassword, bool* pbAuthenticated);
    int DigestAuthenticate (const char* pszPassword, bool* pbAuthenticated);

    const char* GetHeaders();
    size_t GetHeaderLength();
};

class HttpRequestAllocator {
public:
    static HttpRequest* New() { return new HttpRequest(); }
    static void Delete (HttpRequest* pHttpRequest) { delete pHttpRequest; }
};

#endif // !defined(AFX_HTTPREQUEST_H__7E3E4056_A569_11D1_9C4E_0060083E8062__INCLUDED_)