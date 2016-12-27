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
#include "Osal/ReadWriteLock.h"

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD

#include "HttpForm.h"
#include "Cookie.h"

// Error codes
#define ERROR_MALFORMED_REQUEST (-70000000)
#define ERROR_UNSUPPORTED_HTTP_VERSION (-70000001)
#define ERROR_UNSUPPORTED_HTTP_METHOD (-70000002)

class PageSource;
class PageSourceHashValue;
class PageSourceEquals;
template <class CKey, class CData, class HashValue, class Equals> class HashTable;

class HttpServer;

class HttpRequest : public IHttpRequest {

private:

	// Method, version, Uri
	HttpMethod m_iMethod;
	HttpVersion m_iVersion;
	
	char* m_pszUri;
	size_t m_stUriLength;

	// Uri as hard drive path
	String m_strFileName;

	// Browsername
	String m_strBrowserName;

	// Headers
	String m_strHeaders;

	// Client IP address
	String m_strIPAddress;

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
	String m_strLogin;
	String m_strPassword;

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
	const char* GetBrowserName();
	const char* GetClientIP();

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

	const char* GetLogin();
	const char* GetPassword();

	const char* GetHeaders();
	size_t GetHeaderLength();
};

class HttpRequestAllocator {
public:
	static HttpRequest* New() { return new HttpRequest(); }
	static void Delete (HttpRequest* pHttpRequest) { delete pHttpRequest; }
};

#endif // !defined(AFX_HTTPREQUEST_H__7E3E4056_A569_11D1_9C4E_0060083E8062__INCLUDED_)