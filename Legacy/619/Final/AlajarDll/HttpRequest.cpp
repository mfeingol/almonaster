// HttpRequest.cpp: implementation of the HttpRequest class.
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

#include "HttpRequest.h"
#include "PageSource.h"
#include "HttpServer.h"

#include "Osal/TempFile.h"
#include "Osal/Algorithm.h"
#include "Osal/HashTable.h"

#include <stdio.h>

#define MAX_REQUEST_LENGTH 16384
#define MAX_STACK_ALLOC 2048
#define DEFAULT_NUM_FORMS 200

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HttpRequest::HttpRequest() {

	m_iNumRefs = 1;

	m_iMethod = UNSUPPORTED_HTTP_METHOD;
	m_iVersion = UNSUPPORTED_HTTP_VERSION;

	m_pszUri = NULL;

	m_iNumHttpForms = 0;
	m_iNumHttpFormsSpace = 0;
	m_ppszHttpFormName = NULL;
	m_ppHttpForms = NULL;

	m_iNumCookies = 0;
	m_ppszCookieName = NULL;

	m_pPageSource = NULL;

	m_pszSeparator = NULL;

	m_bKeepAlive = false;
	m_bCached = false;

	m_iNumFiles = 0;
	m_stNumBytes = 0;

	m_phtHttpFormTable = NULL;
	m_phtCookieTable = NULL;

	m_pSocket = NULL;
	m_pHttpServer = NULL;

	m_stSeparatorSpace = 0;
	m_stUriLength = 0;
	m_stContentLength = 0;

	m_ppCookies = NULL;

	m_stCookieSpace = 0;

	m_msParseTime = 0;

	m_bParsedUriForms = false;
	m_bMultiPartForms = false;
}


HttpRequest::~HttpRequest() {
	
	unsigned int i;

	if (m_pszSeparator != NULL) {
		delete [] m_pszSeparator;
	}

	if (m_pszUri != NULL) {
		delete [] m_pszUri;
	}

	if (m_iNumHttpForms > 0) {

		for (i = 0; i < m_iNumHttpForms; i ++) {
			m_ppHttpForms[i]->Release();
		}
		delete [] m_ppHttpForms;
	}

	else if (m_ppHttpForms != NULL) {
		delete [] m_ppHttpForms;
	}

	if (m_iNumCookies > 0) {

		for (i = 0; i < m_iNumCookies; i ++) {
			m_ppCookies[i]->Release();
		}
		delete [] m_ppCookies;
	}
	
	else if (m_ppCookies != NULL) {
		delete [] m_ppCookies;
	}

	// Release the pagesource
	if (m_pPageSource != NULL) {
		m_pPageSource->Release();
	}

	if (m_phtHttpFormTable != NULL) {
		delete m_phtHttpFormTable;
	}

	if (m_phtCookieTable != NULL) {
		delete m_phtCookieTable;
	}
}

void HttpRequest::SetHttpServer (HttpServer* pHttpServer) {

	m_pHttpServer = pHttpServer;
	Assert (m_pHttpServer != NULL);
}

void HttpRequest::Recycle() {

	unsigned int i;

	m_strLogin.Clear();
	m_strPassword.Clear();

	if (m_pszSeparator != NULL) {
		*m_pszSeparator = '\0';
	}

	if (m_pszUri != NULL) {
		*m_pszUri = '\0';
	}

	m_strFileName.Clear();
	m_strBrowserName.Clear();
	m_strHeaders.Clear();
	m_strHostName.Clear();

	for (i = 0; i < m_iNumHttpForms; i ++) {
		m_ppHttpForms[i]->Release();
	}

	for (i = 0; i < m_iNumCookies; i ++) {
		m_ppCookies[i]->Release();
	}

	m_iNumHttpForms = 0;
	m_iNumCookies = 0;

	if (m_phtHttpFormTable != NULL) {
		m_phtHttpFormTable->Clear();
	}
	if (m_phtCookieTable != NULL) {
		m_phtCookieTable->Clear();
	}

	if (m_pPageSource) {
		m_pPageSource->Release();
		m_pPageSource = NULL;
	}

	m_bKeepAlive = false;
	m_bCached = false;

	m_iMethod = UNSUPPORTED_HTTP_METHOD;
	m_iVersion = UNSUPPORTED_HTTP_VERSION;

	m_stSeparatorLength = 0;
	m_stContentLength = 0;
	m_msParseTime = 0;

	m_iNumFiles = 0;
	m_stNumBytes = 0;

	m_bParsedUriForms = false;
	m_bMultiPartForms = false;
}

MilliSeconds HttpRequest::GetRequestParseTime() const{
	return m_msParseTime;
}

HttpMethod HttpRequest::GetMethod() {
	return m_iMethod;
}

HttpVersion HttpRequest::GetVersion() {
	return m_iVersion;
}

const char* HttpRequest::GetUri() {
	return m_pszUri;
}

const char* HttpRequest::GetFileName() {
	return m_strFileName;
}

const char* HttpRequest::GetBrowserName() {
	return m_strBrowserName;
}

const char* HttpRequest::GetClientIP() {
	return m_strIPAddress;
}

bool HttpRequest::IsCached() const {
	return m_bCached;
}

void HttpRequest::SetSocket (Socket* pSocket) {
	m_pSocket = pSocket;
}

const char* HttpRequest::GetHeaders() {
	return m_strHeaders.GetCharPtr();
}

size_t HttpRequest::GetHeaderLength() {
	return m_strHeaders.GetLength();
}

PageSource* HttpRequest::GetPageSource() const {
	return m_pPageSource;
}

const char* HttpRequest::GetLogin() {
	return m_strLogin;
}

const char* HttpRequest::GetPassword() {
	return m_strPassword;
}

int HttpRequest::ParseRequestHeader (char* pszLine) {

	int iErrCode;

	// Parse the first line
	char* pszTemp = strstr (pszLine, " ");

	if (pszTemp == NULL) {
		return ERROR_MALFORMED_REQUEST;
	}

	char* pszMethod = pszLine;
	*pszTemp = '_';

	char* pszVer = strstr (pszTemp, " ");
	
	if (pszVer == NULL) {
		return ERROR_MALFORMED_REQUEST;
	}

	char* pszUri = pszTemp + 1;
	*pszTemp = '\0';

	*pszVer = '\0';
	pszVer ++;		// Safe because there's a \0 at the end of the buffer

	// Handle method
	if (stricmp (pszMethod, "GET") == 0) {
		m_iMethod = GET;
	}
	else if (stricmp (pszMethod, "POST") == 0) {
		m_iMethod = POST;
	}
	else if (stricmp (pszMethod, "PUT") == 0) {
		m_iMethod = PUT;
	}
	else if (stricmp (pszMethod, "HEAD") == 0) {
		m_iMethod = HEAD;
	}
	else if (stricmp (pszMethod, "TRACE") == 0) {
		m_iMethod = TRACE;
	}
	else return ERROR_UNSUPPORTED_HTTP_METHOD;

	// Handle version
	if (stricmp (pszVer, "HTTP/1.1") == 0) {
		m_iVersion = HTTP11;
	}
	else if (stricmp (pszVer, "HTTP/1.0") == 0) {
		m_iVersion = HTTP10;
	}
	else if (stricmp (pszVer, "HTTP/0.9") == 0) {
		m_iVersion = HTTP09;
	}
	else return ERROR_UNSUPPORTED_HTTP_VERSION;

	// Check Uri
	if (*pszUri != '/') {
		return ERROR_MALFORMED_REQUEST;
	}

	// Handle Uri
	if (m_pszUri == NULL) {

		m_pszUri = new char [50];
		if (m_pszUri == NULL) {
			return ERROR_OUT_OF_MEMORY;
		}

		m_stUriLength = 50;
	}

	while (Algorithm::WWWEscapeCharToAscii (pszUri, m_pszUri, m_stUriLength) == ERROR_INVALID_ARGUMENT &&
		m_stUriLength < 102400) {
		
		m_stUriLength *= 3;
		delete [] m_pszUri;
		m_pszUri = new char [m_stUriLength];
		if (m_pszUri == NULL) {
			return ERROR_OUT_OF_MEMORY;
		}
	}
	strcpy (pszUri, m_pszUri);

	// The first word of the URI is the name of the PageSource
	char* pszFormStart = strstr (pszUri, "?");
	if (pszFormStart != NULL) {
		
		m_bParsedUriForms = true;

		// Remove forms from object's URI
		m_pszUri [pszFormStart - pszUri] = '\0';

		// Null out form start '?'
		*pszFormStart = '\0';
		pszFormStart ++;

		// Handle form submissions via URI
		size_t stParsed;
		iErrCode = ParseForms (pszFormStart, &stParsed, true);
		if (iErrCode != OK) {
			return iErrCode;
		}
	}

	char* pszNextSlash = strstr (pszUri + 1, "/");
	if (pszNextSlash != NULL) {
		*pszNextSlash = '\0';
		pszNextSlash ++;
	}

	// Look up page source and calculate file name
	m_pPageSource = m_pHttpServer->GetPageSource (pszUri + 1);	// Adds a reference on the pagesource
	if (m_pPageSource == NULL) {

		// Must be the default, then
		m_pPageSource = m_pHttpServer->GetDefaultPageSource();	// Adds a reference on the pagesource

		// Build path
		m_strFileName = m_pPageSource->GetBasePath();
		if (m_strFileName.GetCharPtr() == NULL) {
			return ERROR_OUT_OF_MEMORY;
		}

		m_strFileName += (m_pszUri + 1);

	} else {

		// Build path
		m_strFileName = m_pPageSource->GetBasePath();
		if (m_strFileName.GetCharPtr() == NULL) {
			return ERROR_OUT_OF_MEMORY;
		}

		if (pszNextSlash != NULL) {
			m_strFileName += pszNextSlash;
		}
	}

	return OK;
}


int HttpRequest::ParseHeader (char* pszLine) {
	
	int iErrCode;
	bool bFreeBuf;

	UTCTime tLastModified;

	char* pszValue = NULL, * pszTemp = NULL, * pszDecode = NULL, * pszBuf;

	size_t stLineLen = strlen (pszLine);
	if (stLineLen >= MAX_STACK_ALLOC) {
		
		pszBuf = new char [stLineLen + 1];
		if (pszBuf == NULL) {
			return ERROR_OUT_OF_MEMORY;
		}

		bFreeBuf = true;

	} else {

		pszBuf = (char*) StackAlloc (stLineLen + 1);
		bFreeBuf = false;
	}

	// Separate header name from value
	strncpy (pszBuf, pszLine, stLineLen + 1);

	char* pszEnd = pszBuf + stLineLen;
	char* pszHeader = strtok (pszBuf, " ");
	pszValue = strtok (NULL, "");
	
	if (pszValue == NULL) {
		pszValue = "";
	}

	// Handle headers
	switch (*pszLine) {
		
	// Authorization
	case 'A':
	case 'a':

		if (stricmp (pszHeader, "Authorization:") == 0) {

			const size_t stBasicSpaceLen = sizeof ("Basic");

			if (strnicmp (pszValue, "Basic ", stBasicSpaceLen)  == 0) {

				pszTemp = pszValue + stBasicSpaceLen;
				size_t stEncodeLen = strlen (pszTemp) + 1;

				if (stEncodeLen > 128) {
					pszDecode = new char [stEncodeLen];
					if (pszDecode == NULL) {
						iErrCode = ERROR_OUT_OF_MEMORY;
						goto Cleanup;
					}
				} else {
					pszDecode = (char*) StackAlloc (stEncodeLen);
				}

				Algorithm::DecodeBase64 (pszTemp, pszDecode);

				pszTemp = strtok (pszDecode, ":");				
				m_strLogin = pszTemp;
				
				pszTemp = strtok (NULL, "");
				m_strPassword = pszTemp;

				if (stEncodeLen > 128) {
					delete [] pszDecode;
				}
			}
		}
		
		break;
		
	// Connection, Content-Length, Cookie
	case 'C':
	case 'c':
		
		if (stricmp (pszHeader, "Connection:") == 0) {
			m_bKeepAlive = (stricmp (pszValue, "Keep-Alive") == 0);
		}
		else if (stricmp (pszHeader, "Content-Type:") == 0) {
			
			// Check for multipart
			if (strstr (pszValue, "multipart/form-data") != NULL) {
				
				m_bMultiPartForms = true;

				// Get boundary
				if (m_pszSeparator == NULL) {

					m_stSeparatorSpace = strlen (pszValue) + 1;

					m_pszSeparator = new char [m_stSeparatorSpace];
					if (m_pszSeparator == NULL) {
						iErrCode = ERROR_OUT_OF_MEMORY;
						goto Cleanup;
					}
				
				} else {

					size_t stLength = strlen (pszValue);

					if (m_stSeparatorSpace <= stLength) {

						delete [] m_pszSeparator;

						m_stSeparatorSpace = stLength + 1;

						m_pszSeparator = new char [m_stSeparatorSpace];
						if (m_pszSeparator == NULL) {
							iErrCode = ERROR_OUT_OF_MEMORY;
							goto Cleanup;
						}
					}
				}

				strcpy (m_pszSeparator, "--");

				// Parse to end of separator
				pszValue += sizeof ("multipart/form-data; boundary=") - 1;

				if (pszValue > pszEnd) {
					iErrCode = ERROR_MALFORMED_REQUEST;
					goto Cleanup;
				}

				m_stSeparatorLength = strlen (pszValue) + 1;

				if (*pszValue == '\"' && pszValue[m_stSeparatorLength - 2] == '\"') {

					// Fucking Opera 3.50 puts quotes around the separator
					m_stSeparatorLength -= 2;
					strncpy (m_pszSeparator + 2, pszValue + 1, m_stSeparatorLength - 1);
					m_pszSeparator[m_stSeparatorLength + 1] = '\0';

				} else {

					// Normal case: no quotes
					strncpy (m_pszSeparator + 2, pszValue, m_stSeparatorLength);
					m_stSeparatorLength ++;
				}
			}
		}
		else if (stricmp (pszHeader, "Content-Length:") == 0) {
			m_stContentLength = atoi (pszValue);
		}
		else if (stricmp (pszHeader, "Cookie:") == 0) {
			
			// Count the number of semicolons
			unsigned int i, iNumSemicolons = 1;

			char* pszTemp = strstr (pszValue, ";");
			while (pszTemp != NULL) {
				iNumSemicolons ++;
				pszTemp = strstr (pszTemp + 1, ";");
			}

			// Allocate space for cookie primitives
			char** ppszCookie;

			if (iNumSemicolons > 100) {

				ppszCookie = new char* [iNumSemicolons];
				if (ppszCookie == NULL) {
					iErrCode = ERROR_OUT_OF_MEMORY;
					goto Cleanup;
				}

			} else {
				ppszCookie = (char**) StackAlloc (iNumSemicolons * sizeof (char*));
			}

			if (m_stCookieSpace < iNumSemicolons) {

				if (m_ppCookies != NULL) {
					delete [] m_ppCookies;
				}
	
				m_ppCookies = new Cookie* [iNumSemicolons * 2];
				if (m_ppCookies == NULL) {
					iErrCode = ERROR_OUT_OF_MEMORY;
					if (iNumSemicolons > 100) {
						delete [] ppszCookie;
					}
					goto Cleanup;
				}

				m_ppszCookieName = (const char**) m_ppCookies + iNumSemicolons;
				m_stCookieSpace = iNumSemicolons;
			}
			
			// Tokenize by semicolons
			unsigned int iNumCookies = 0;
			
			pszTemp = strtok (pszValue, ";");
			while (pszTemp != NULL) {
				
				ppszCookie[iNumCookies] = pszTemp;
				pszTemp = strtok (NULL, ";");

				iNumCookies ++;
			}

			// Make sure we found something
			if (iNumCookies > 0) {

				Assert (iNumCookies <= iNumSemicolons);
				
				// Tokenize by equals
				Cookie* pCookie, * pMasterCookie;
				char* pszCookieName, * pszCookieValue;

				// Initialize cookie table
				if (m_phtCookieTable == NULL) {

					m_phtCookieTable = new HashTable<const char*, Cookie*, RequestHashValue, RequestEquals> (NULL, NULL);
					if (m_phtCookieTable == NULL || !m_phtCookieTable->Initialize (iNumCookies)) {
						iErrCode = ERROR_OUT_OF_MEMORY;
						if (iNumSemicolons > 100) {
							delete [] ppszCookie;
						}
						goto Cleanup;
					}
				}

				m_iNumCookies = 0;
				for (i = 0; i < iNumCookies; i ++) {
					
					pszCookieName = strtok (ppszCookie[i], "=");

					if (pszCookieName == NULL) {
						pszCookieName = '\0';
					} else {
					
						// Adjust for space after semicolon
						if (i > 0 && *pszCookieName == ' ') {
							pszCookieName ++;
						}
					}

					pszCookieValue = strtok (NULL, "");	// Could be NULL, but that's fine

					// Does the cookie already exist?
					pCookie = Cookie::CreateInstance (pszCookieName, pszCookieValue);
					if (pCookie == NULL) {
						iErrCode = ERROR_OUT_OF_MEMORY;
						goto Cleanup;
					}

					if (m_phtCookieTable->FindFirst (pszCookieName, &pMasterCookie)) {

						// Add a subcookie
						iErrCode = pMasterCookie->AddSubCookie (pCookie);
						if (iErrCode != OK) {
							pCookie->Release();
							if (iNumSemicolons > 100) {
								delete [] ppszCookie;
							}
							goto Cleanup;
						}

					} else {

						// Insert a new master cookie
						const char* pszSafeCookieName = pCookie->GetName();

						if (!m_phtCookieTable->Insert (pszSafeCookieName, pCookie)) {
							pCookie->Release();
							iErrCode = ERROR_OUT_OF_MEMORY;
							if (iNumSemicolons > 100) {
								delete [] ppszCookie;
							}
							goto Cleanup;
						}
						
						// Put pointer in array
						m_ppCookies [m_iNumCookies] = pCookie;
						m_ppszCookieName [m_iNumCookies] = pszSafeCookieName;

						m_iNumCookies ++;
					}
				}
			}

			if (iNumSemicolons > 100) {
				delete [] ppszCookie;
			}
		}

		break;
		
	// Host
	case 'H':
	case 'h':
		
		if (stricmp (pszHeader, "Host:") == 0) {
			m_strHostName = pszValue;
		}
		
		break;
		
	case 'I':
	case 'i':

		if (stricmp (pszHeader, "If-Modified-Since") == 0) {
			m_bCached = !File::WasFileModifiedAfter (m_strFileName.GetCharPtr(), pszValue, &tLastModified);
		}
		
		break;
		
	// User-Agent
	case 'U':
	case 'u':
		
		if (stricmp (pszHeader, "User-Agent:") == 0) {
			m_strBrowserName = pszValue;
		}
		break;
		
	default:

		// Ignore unhandled headers
		break;
	}

Cleanup:

	if (bFreeBuf) {
		delete [] pszBuf;
	}

	return OK;
}

int HttpRequest::ParseHeaders() {

	int iErrCode;

	Timer tTimer;
	Time::StartTimer (&tTimer);

	// Get the client's IP address and domain name
	m_strIPAddress = m_pSocket->GetTheirIP();
	if (m_strIPAddress.GetCharPtr() == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	char pszBuffer [MAX_REQUEST_LENGTH + 1], pszLine [MAX_REQUEST_LENGTH + 1];
	size_t stNumBytes, stBeginRecv = 0, stLineLength;
	bool bEndHeaders = false, bFirstLine = true;

	char* pszBegin, * pszEnd, * pszEndMarker;
	
	// Set the timeout to 10 seconds, just in case
	// TODO:  this blocks Http 1.1
	iErrCode = m_pSocket->SetRecvTimeOut (10000);

	// Recv as big a block of data as possible
	// This loop will terminate when the end of the headers is received
	while ((iErrCode = m_pSocket->Recv (pszBuffer + stBeginRecv, MAX_REQUEST_LENGTH, &stNumBytes)) == OK) {

		pszBuffer[stBeginRecv + stNumBytes] = '\0';
		pszBegin = pszBuffer;
		pszEnd = strstr (pszBuffer, "\r\n");
		pszEndMarker = pszBuffer + stBeginRecv + stNumBytes;

		while (pszEnd != NULL) {

			// Get a line
			stLineLength = (pszEnd - pszBegin);
			memcpy (pszLine, pszBegin, stLineLength);

			pszLine[stLineLength] = '\0';

			// Interpret the header line
			if (*pszLine == '\0') {
				pszBegin += 2;
				bEndHeaders = true;
				break;
			} else {

				if (bFirstLine) {

					bFirstLine = false;
					iErrCode = ParseRequestHeader (pszLine);
					if (iErrCode != OK) {
						return iErrCode;
					}

				} else {
					
					iErrCode = ParseHeader (pszLine);
					if (iErrCode != OK) {
						return iErrCode;
					}
				}
			}

			// Save header
			m_strHeaders += pszLine;
			m_strHeaders += "\n";
			
			// Reset the pointers
			pszBegin = pszEnd + 2;
			if (pszBegin >= pszEndMarker) {
				break;
			}

			/*if (pszEnd + 4 < pszEndMarker &&
				strncmp (pszEnd, "\r\n\r\n", 4) == 0) {
				bEndHeaders = true;
				pszBegin += 2;
				break;
			}*/
			
			pszEnd = strstr (pszBegin, "\r\n");
		}

		// We reached the end of the recv, so save the extra line info, if any
		stBeginRecv = pszEndMarker - pszBegin;
		if (stBeginRecv > 0) {
			memmove (pszBuffer, pszBegin, stBeginRecv);
		}

		pszBuffer[stBeginRecv] = '\0';

		if (bEndHeaders) {
			break;
		}
	}
	
	// Are we handling forms?
	if (m_iMethod == POST) {

		// Make sure they sent a content-length line
		if (m_stContentLength == 0) {
			iErrCode = ERROR_MALFORMED_REQUEST;
		} else {

			Assert (stBeginRecv != 0 || pszBuffer[0] == '\0');

			// What kind of forms?
			if (m_pszSeparator == NULL || m_pszSeparator[0] == '\0') {
				iErrCode = HandleSimpleForms (pszBuffer, MAX_REQUEST_LENGTH, stBeginRecv);
			} else {
				iErrCode = HandleMultipartForms (pszBuffer, stBeginRecv);
			}
		}
	}

	m_msParseTime = Time::GetTimerCount (tTimer);

	return iErrCode;
}


/////////////
// Cookies //
/////////////

unsigned int HttpRequest::GetNumCookies() {
	return m_iNumCookies;
}

ICookie* HttpRequest::GetCookie (unsigned int iIndex) {
	
	if (iIndex >= m_iNumCookies) {
		return NULL;
	}

	Cookie* pCookie;
	bool bFound = m_phtCookieTable->FindFirst (m_ppszCookieName[iIndex], &pCookie);

	Assert (bFound);

	return bFound ? pCookie : NULL;
}

const char** HttpRequest::GetCookieNames() {
	
	return m_ppszCookieName;
}

ICookie* HttpRequest::GetCookie (const char* pszName) {

	if (m_iNumCookies == 0) {
		return NULL;
	}

	Cookie* pCookie;

	return m_phtCookieTable->FindFirst (pszName, &pCookie) ? pCookie : NULL;
}

ICookie* HttpRequest::GetCookieBeginsWith (const char* pszName) {

	if (m_iNumCookies == 0) {
		return NULL;
	}

	size_t stLength = strlen (pszName);
	
	HashTableIterator<const char*, Cookie*> htiIterator;
	
	const char* pszFormName;
	while (m_phtCookieTable->GetNextIterator (&htiIterator)) {

		pszFormName = htiIterator.GetKey();

		if (strlen (pszFormName) >= stLength &&
			strncmp (pszName, pszFormName, stLength) == 0) {
			return htiIterator.GetData();
		}
	}
	
	return NULL;
}

bool HttpRequest::GetKeepAlive() const {
	return m_bKeepAlive;
}


///////////
// Forms //
///////////

unsigned int HttpRequest::GetNumForms() {
	return m_iNumHttpForms;
}

IHttpForm* HttpRequest::GetForm (unsigned int iIndex) {

	if (iIndex >= m_iNumHttpForms) {
		return NULL;
	}

	HttpForm* pHttpForm;
	bool bFound = m_phtHttpFormTable->FindFirst (m_ppszHttpFormName[iIndex], &pHttpForm);

	Assert (bFound);

	return bFound ? pHttpForm : NULL;
}

const char** HttpRequest::GetFormNames() {

	return m_ppszHttpFormName;
}

IHttpForm* HttpRequest::GetForm (const char* pszName) {

	if (m_iNumHttpForms == 0) {
		return NULL;
	}

	HttpForm* pHttpForm;
	return m_phtHttpFormTable->FindFirst (pszName, &pHttpForm) ? pHttpForm : NULL;
}


IHttpForm* HttpRequest::GetFormBeginsWith (const char* pszName) {

	if (m_iNumHttpForms == 0) {
		return NULL;
	}

	size_t stLength = strlen (pszName);

	HashTableIterator<const char*, HttpForm*> htiIterator;
	
	const char* pszFormName;
	while (m_phtHttpFormTable->GetNextIterator (&htiIterator)) {

		pszFormName = htiIterator.GetKey();

		if (strlen (pszFormName) >= stLength &&
			strncmp (pszName, pszFormName, stLength) == 0) {
			return htiIterator.GetData();
		}
	}
	
	return NULL;
}


int HttpRequest::HandleSimpleForms (char* pszBuffer, size_t stBufferSize, size_t stNumBytesInBuffer) {

	int iErrCode = OK;
	size_t stBytesParsed, stBytesIn, stCurrentPos, stTemp;

	iErrCode = ParseForms (pszBuffer, &stBytesParsed, stNumBytesInBuffer >= m_stContentLength);
	if (iErrCode != OK) {
		return iErrCode;
	}

	Assert (stBytesParsed <= stNumBytesInBuffer);
	stCurrentPos = stNumBytesInBuffer - stBytesParsed;

	if (stCurrentPos > 0) {
		memmove (pszBuffer, pszBuffer + stBytesParsed, stCurrentPos);
	}

	while (stBytesParsed < m_stContentLength) {

		if (stCurrentPos == stBufferSize) {

			// The input field is larger than our buffer - stick it in a temp file
			iErrCode = HandleLargeSimpleForm (pszBuffer, stBufferSize, stBytesParsed, &stCurrentPos, &stTemp);
			if (iErrCode != OK) {
				return iErrCode;
			}

			stBytesParsed += stTemp;
			continue;
		}

		iErrCode = m_pSocket->Recv (
			pszBuffer + stCurrentPos, 
			min (stBufferSize - stCurrentPos, m_stContentLength - stBytesParsed), 
			&stBytesIn
			);

		if (iErrCode != OK) {
			return iErrCode;
		}

		Assert (stCurrentPos + stBytesIn < stBufferSize + 1);
		pszBuffer [stCurrentPos + stBytesIn] = '\0';

		iErrCode = ParseForms (
			pszBuffer, 
			&stTemp, 
			stBytesParsed + stCurrentPos + stBytesIn >= m_stContentLength
			);

		if (iErrCode != OK) {
			return iErrCode;
		}

		stBytesParsed += stTemp;
		stCurrentPos += stBytesIn - stTemp;

		if (stCurrentPos > 0) {
			memmove (pszBuffer, pszBuffer + stTemp, stCurrentPos);
		}
	}

	return OK;
}

int HttpRequest::HandleLargeSimpleForm (char* pszBuffer, size_t stBufferSize, size_t stBytesParsed, 
										size_t* pstCurrentPos, size_t* pstBytesParsed) {

	// The format for what we're parsing is key=value[&]
	//
	// The algorithm is to recv and store in a temp file until we're given an & or we hit the content length

	int iErrCode = OK;
	size_t stBytesIn, stLargeBytesDownloaded = stBufferSize;
	char* pszAmpersand = NULL, * pszFormName = NULL;

	const size_t stNewBufferSize = 8191;

	TempFile tTemp;

	// Get the form name
	pszFormName = strstr (pszBuffer, "=");
	if (pszFormName == NULL) {
		return ERROR_MALFORMED_REQUEST;
	}

	// Allocate a temp buffer
	char* pszNewBuffer = new char [stNewBufferSize + 1];
	if (pszNewBuffer == NULL) {
		iErrCode = ERROR_OUT_OF_MEMORY;
		goto Cleanup;
	}

	// Create a temp file
	iErrCode = tTemp.Open();
	if (iErrCode != OK) {
		goto Cleanup;
	}

	// Output current data to temp file
	iErrCode = tTemp.Write (pszFormName + 1, stBufferSize - (pszFormName + 1 - pszBuffer));
	if (iErrCode != OK) {
		goto Cleanup;
	}

	pszFormName[0] = '\0';
	pszFormName = pszBuffer;

	while (true) {

		iErrCode = m_pSocket->Recv (
			pszNewBuffer,
			min (stNewBufferSize, m_stContentLength - stBytesParsed - stLargeBytesDownloaded), 
			&stBytesIn
			);

		if (iErrCode != OK) {
			goto Cleanup;
		}

		pszNewBuffer [stBytesIn] = '\0';
		stLargeBytesDownloaded += stBytesIn;

		// Detect exit condition
		if ((pszAmpersand = strstr (pszNewBuffer, "&")) != NULL ||
			stBytesParsed + stLargeBytesDownloaded >= m_stContentLength) {

			size_t stWrite;

			// Flush the data
			if (pszAmpersand != NULL) {

				stWrite = pszAmpersand - pszNewBuffer;
				*pstCurrentPos = stBytesIn - stWrite - 1;

			} else {

				stWrite = stBytesIn;
				if (pszNewBuffer [stWrite - 2] == '\r' && 
					pszNewBuffer [stWrite - 1] == '\n') {
					stWrite -= 2;
				}

				*pstCurrentPos = 0;
			}

			iErrCode = tTemp.Write (pszNewBuffer, stWrite);
			if (iErrCode != OK) {
				goto Cleanup;
			}

			// Null cap the file
			iErrCode = tTemp.Write ("", 1);
			if (iErrCode != OK) {
				goto Cleanup;
			}

			tTemp.Close();

			iErrCode = AddHttpForm (LARGE_SIMPLE_FORM, pszFormName, tTemp.GetName(), NULL);
			if (iErrCode != OK) {
				goto Cleanup;
			}

			*pstBytesParsed = stLargeBytesDownloaded - *pstCurrentPos;

			// Put extra data back into the provided buffer
			if (pszAmpersand != NULL) {
				memcpy (pszBuffer, pszNewBuffer + stWrite + 1, *pstCurrentPos);
				pszBuffer [*pstCurrentPos] = '\0';
			}

			break;
		}

		// Still plugging - flush data and continue
		iErrCode = tTemp.Write (pszNewBuffer, stBytesIn);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

Cleanup:

	if (pszNewBuffer != NULL) {
		delete [] pszNewBuffer;
	}

	if (iErrCode != OK) {

		if (tTemp.IsOpen()) {
			tTemp.Close();
		}

		tTemp.Delete();
	}

	return iErrCode;
}


int HttpRequest::AddHttpForm (HttpFormType ftFormType, const char* pszHttpFormName, 
							  const char* pszHttpFormValue, const char* pszFileName) {

	int iErrCode = OK;

	HttpForm* pHttpForm, * pMasterHttpForm;
	bool bRetVal;

	Assert (pszHttpFormName != NULL);
	Assert (ftFormType >= SIMPLE_FORM && ftFormType <= LARGE_SIMPLE_FORM);

	if (pszHttpFormValue == NULL || pszHttpFormValue[0] == '\0') {
		pszHttpFormValue = NULL;
	}

	if (pszFileName == NULL || pszFileName[0] == '\0') {
		pszFileName = NULL;
	}

	// Initialize the form hash table
	if (m_phtHttpFormTable == NULL) {
		
		m_phtHttpFormTable = new HashTable<const char*, HttpForm*, RequestHashValue, RequestEquals> (NULL, NULL);
		if (m_phtHttpFormTable == NULL || !m_phtHttpFormTable->Initialize (DEFAULT_NUM_FORMS)) {
			return ERROR_OUT_OF_MEMORY;
		}
	}

	// Allocate a new form
	pHttpForm = HttpForm::CreateInstance (ftFormType, pszHttpFormName, pszHttpFormValue, pszFileName, m_bMultiPartForms);
	if (pHttpForm == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	// Does the form already exist?
	if (m_phtHttpFormTable->FindFirst (pszHttpFormName, &pMasterHttpForm)) {
		
		// Add a subform
		iErrCode = pMasterHttpForm->AddForm (pHttpForm);
		if (iErrCode != OK) {
			pHttpForm->Release();
			return iErrCode;
		}

	} else {

		// Insert a new master form
		const char* pszSafeHttpFormName = pHttpForm->GetName();
		
		bRetVal = m_phtHttpFormTable->Insert (pszSafeHttpFormName, pHttpForm);
		Assert (bRetVal);

		// Resize?
		if (m_iNumHttpFormsSpace == m_iNumHttpForms) {

			if (m_iNumHttpForms == 0) {
				
				m_ppHttpForms = new HttpForm* [20];
				if (m_ppHttpForms == NULL) {
					pHttpForm->Release();
					return ERROR_OUT_OF_MEMORY;
				}

				m_ppszHttpFormName = (const char**) m_ppHttpForms + 10;
				m_iNumHttpFormsSpace = 10;
			
			} else {

				unsigned int iNumForms = m_iNumHttpForms * 2;
			
				HttpForm** ppHttpForms = new HttpForm* [iNumForms * 2];
				if (ppHttpForms == NULL) {
					pHttpForm->Release();
					return ERROR_OUT_OF_MEMORY;
				}

				const char** ppszHttpFormName = (const char**) ppHttpForms + iNumForms;

				memcpy (ppHttpForms, m_ppHttpForms, iNumForms * sizeof (HttpForm*));
				memcpy (ppszHttpFormName, ppszHttpFormName, iNumForms * sizeof (HttpForm*));

				delete [] m_ppHttpForms;

				m_ppHttpForms = ppHttpForms;
				m_ppszHttpFormName = ppszHttpFormName;

				m_iNumHttpFormsSpace = iNumForms;
			}
		}

		// Copy pointers
		m_ppHttpForms [m_iNumHttpForms] = pHttpForm;
		m_ppszHttpFormName [m_iNumHttpForms] = pszSafeHttpFormName;

		m_iNumHttpForms ++;
	}

	return OK;
}

int HttpRequest::ParseForms (char* pszFormStart, size_t* pstParsed, bool bLastBytes) {

	int iErrCode;
	size_t stConsumed = 0;

	Assert (pszFormStart != NULL && pstParsed != NULL);

	char* pszName, * pszValue, * pszNext = pszFormStart;

	while (pszNext != NULL) {
		
		pszName = pszNext;
		
		// Grep for '='
		pszValue = strstr (pszNext, "=");
		if (pszValue == NULL) {
			break;
		}
		
		pszValue[0] = '\0';
		pszValue ++;
		
		// Grep for '&'
		pszNext = strstr (pszValue, "&");
		if (pszNext != NULL) {

			pszNext[0] = '\0';
			pszNext ++;
		
			stConsumed += pszNext - pszName;

		} else {

			if (bLastBytes) {

				size_t stLastValueLen = strlen (pszValue);

				// Last bytes we're going to see
				// Just consume what we have as a form
				stConsumed += pszValue - pszName + stLastValueLen;

				// Some browsers send a \r\n at the end, just for kicks
				if (pszValue [stLastValueLen - 2] == '\r' &&
					pszValue [stLastValueLen - 1] == '\n') {
					pszValue [stLastValueLen - 2] = '\0';
				}

			} else {

				// Submission was broken in an inconvenient place by the browser
				// Break and try again with more recv'd data
				pszValue [-1] = '=';
				break;
			}
		}

		iErrCode = AddHttpForm (SIMPLE_FORM, pszName, pszValue, NULL);

		// Restore data
		pszValue [-1] = '=';
		if (pszNext != NULL) {
			pszNext[-1] = '&';
		}

		if (iErrCode != OK) {
			return iErrCode;
		}
	}

	*pstParsed = stConsumed;

	return OK;
}

int HttpRequest::HandleMultipartForms (char* pszBuffer, size_t stNumBytes) {

	int iErrCode;
	size_t stNumProcessed = 0, stNumRemaining = 0, stNumBytesRecvd, stNumBytesGone = 0, stNetwork;

	if (stNumBytes == m_stContentLength) {

		// Handle the forms in here and we're done!
		return HandleMultiPartFormsInBuffer (stNumBytes, pszBuffer, &stNumProcessed, &stNumRemaining);

	} else {

		// If we received some data in the buffer, try to get forms out of it
		if (stNumBytes > 0) {
			iErrCode = HandleMultiPartFormsInBuffer (stNumBytes, pszBuffer, &stNumProcessed, &stNumRemaining);
		}

		// At this point, we either have no data or incomplete data in the buffer,
		// so we need to recv the remaining data
		stNumBytesGone = stNumProcessed;
		size_t stLimit = m_stContentLength - m_stSeparatorLength - 6;
		
		while (stNumBytesGone < stLimit) {
			
			stNetwork = m_stContentLength - stNumRemaining - stNumBytesGone;
			if (stNetwork > 0) {
				
				iErrCode = m_pSocket->Recv (
					pszBuffer + stNumRemaining, 
					min (stNetwork, MAX_REQUEST_LENGTH - stNumRemaining), &stNumBytesRecvd
					);
				
				if (iErrCode != OK) {
					return iErrCode;
				}
				stNumRemaining += stNumBytesRecvd;
			}

			// Send the new data in to be processed
			iErrCode = HandleMultiPartFormsInBuffer (stNumRemaining, pszBuffer, &stNumProcessed, &stNumRemaining);

			// If nothing happened, then we have a big form in our hands
			if (stNumProcessed != 0) {

				stNumBytesGone += stNumProcessed;

			} else {

				// Try to make sense of it
				iErrCode = HandleBigMultiPartForm (stNumRemaining, pszBuffer, &stNumProcessed, &stNumRemaining);
				if (iErrCode != OK) {
					return iErrCode;
				}

				if (stNumProcessed == 0) {
					return ERROR_FAILURE;
				}

				stNumBytesGone += stNumProcessed;
				if (stNumBytesGone < m_stContentLength) {

					// Run the simple handler now and clean up the rest of the forms after the big one
					iErrCode = HandleMultiPartFormsInBuffer (stNumRemaining, pszBuffer, &stNumProcessed, &stNumRemaining);
					if (iErrCode != OK) {
						return iErrCode;
					}

					stNumBytesGone += stNumProcessed;

					// If the loop doesn't terminate now, we have another big one coming up
				}
			}

		}	// End while loop

		// At this point, we've processed all the data in the message
	}

	return OK;
}


int HttpRequest::HandleBigMultiPartForm (size_t stNumRemaining, char* pszBuffer, size_t* pstNumProcessed, 
										 size_t* pstNumRemaining) {

	int iErrCode;

	HttpFormType ftFormType = UNSUPPORTED_FORM_TYPE;
	size_t stNumBytesRecvd;

	TempFile tfTempFile;
	char* pszEndMarker = pszBuffer + stNumRemaining;

	// Initialize return values
	*pstNumProcessed = 0;
	*pstNumRemaining = stNumRemaining;

	// Peek at the buffer and try to find the form type
	char* pszEnd = pszBuffer + m_stSeparatorLength + 40;
	if (pszEnd >= pszEndMarker) {
		return ERROR_MALFORMED_REQUEST;
	}

	// Find the end quote and cap it with a null character
	char* pszBegin = strstr (pszEnd, "\"");
	if (pszBegin == NULL) {
		return ERROR_MALFORMED_REQUEST;
	}
	*pszBegin = '\0';
	
	// That's our form name
	size_t stFileSize = 0, stFormNameLen = strlen (pszEnd) + 1;

	char* pszFormName, * pszFileName = NULL, * pszNext;

	if (stFormNameLen > MAX_STACK_ALLOC) {

		pszFormName = new char [stFormNameLen];
		if (pszFormName == NULL) {
			return ERROR_OUT_OF_MEMORY;
		}

	} else {

		pszFormName = (char*) StackAlloc (stFormNameLen);
	}

	strncpy (pszFormName, pszEnd, stFormNameLen);

	// Determine form type
	pszEnd += stFormNameLen;
	if (pszEnd[0] == ';') {

		if (strncmp (pszEnd, "; filename=", 11) != 0) {
			iErrCode = ERROR_MALFORMED_REQUEST;
			goto Cleanup;
		}
		
		// Jump over the semicolon space and filename
		pszEnd += 12;
		
		// Find the end quote and cap it with a null character
		pszBegin = strstr (pszEnd, "\"");
		if (pszBegin == NULL) {
			iErrCode = ERROR_MALFORMED_REQUEST;
			goto Cleanup;
		}
		*pszBegin = '\0';
		
		// That's the file name
		pszFileName = pszEnd;		
		
		// Find the beginning of the content-type
		pszEnd += strlen (pszFileName) + 3;
		
		// Ignore the content-type and find the data
		pszEnd = strstr (pszEnd, "\r\n");
		if (pszEnd == NULL) {
			iErrCode = ERROR_MALFORMED_REQUEST;
			goto Cleanup;
		}
		pszEnd += 4;

		// It's a file
		ftFormType = FILE_FORM;
		m_iNumFiles ++;
	
	} else {

		if (strncmp (pszEnd, "\r\n\r\n", 4) != 0) {
			iErrCode = ERROR_MALFORMED_REQUEST;
			goto Cleanup;
		}

		pszEnd += 4;

		// It's a large data form
		ftFormType = SIMPLE_FORM;
	}
		
	// Open a temporary file for the data itself
	tfTempFile.Open();

	// Do we have a closing separator?
	pszNext = Algorithm::memstr (pszEnd, m_pszSeparator, stNumRemaining - (pszEnd - pszBuffer));
	if (pszNext != NULL) {
		
		// Write the data minus the \r\n at the end of the file's data (and of course the last separator)		
		tfTempFile.Write (pszEnd, pszNext - pszEnd);
		
	} else {
		
		// The file is too big for this buffer, so write the first chunk to disk
		tfTempFile.Write (pszEnd, pszEndMarker - pszEnd);
		*pstNumProcessed = stNumRemaining;
		*pstNumRemaining = 0;
		
		// Recv until we have the whole thing
		while (true) {
			
			iErrCode = m_pSocket->Recv (pszBuffer, MAX_REQUEST_LENGTH, &stNumBytesRecvd);
			if (iErrCode != OK) {
				tfTempFile.Close();
				tfTempFile.Delete();
				goto Cleanup;
			}
			
			if (stNumBytesRecvd > 0) {
				
				pszNext = Algorithm::memstr (pszBuffer, m_pszSeparator, stNumBytesRecvd);
				
				if (pszNext == NULL) {
					
					// Another aimless bucketload of data
					tfTempFile.Write (pszBuffer, stNumBytesRecvd);
					(*pstNumProcessed) += stNumBytesRecvd;
					
				} else {
					
					int iNumData = pszNext - pszBuffer - 2;
					
					// EOF found!
					tfTempFile.Write (pszBuffer, iNumData);
					
					// Fix the buffer
					(*pstNumProcessed) += iNumData;
					*pstNumRemaining = stNumBytesRecvd - iNumData;
					memcpy (pszBuffer, pszNext, *pstNumRemaining);
					pszBuffer[*pstNumRemaining] = '\0';
					break;
				}
			}
		}	// End while
	}	// End didn't find a closing token
	
	// Close the temp file
	iErrCode = tfTempFile.GetSize (&stFileSize);
	if (iErrCode != OK) {
		tfTempFile.Close();
		tfTempFile.Delete();
		goto Cleanup;
	}
	
	/*pszFormValue = tfTempFile.GetName();
	iErrCode = File::GetFileSize (pszFormValue, &stFileSize);
	if (iErrCode != OK) {
		File::DeleteFile (pszFormValue);
		goto Cleanup;
	}*/
	
	m_stNumBytes += stFileSize;	

	// Add form
	if (ftFormType == FILE_FORM) {
	
		tfTempFile.Close();
		iErrCode = AddHttpForm (FILE_FORM, pszFormName, tfTempFile.GetName(), pszFileName);
	
	} else {

		// Add a terminating zero, just in case
		iErrCode = tfTempFile.Write ("", 1);
		if (iErrCode != OK) {
			Assert (false);
			tfTempFile.Close();
			tfTempFile.Delete();
			goto Cleanup;
		}

		tfTempFile.Close();

		iErrCode = AddHttpForm (LARGE_SIMPLE_FORM, pszFormName, tfTempFile.GetName(), NULL);
	}

	if (iErrCode != OK) {
		tfTempFile.Delete();
		goto Cleanup;
	}

Cleanup:

	if (stFormNameLen > MAX_STACK_ALLOC) {
		delete [] pszFormName;
	}

	return iErrCode;
}

int HttpRequest::HandleMultiPartFormsInBuffer (size_t stNumBytes, char* pszBuffer, size_t* pstBytesProcessed, 
												size_t* pstBytesRemaining) {

	int iErrCode;

	TempFile tfTempFile;

	size_t stFileSize = 0, stTempLength, stBufferLength = stNumBytes;
	
	HttpFormType ftFormType = UNSUPPORTED_FORM_TYPE;

	const char* pszFormValue;
	char* pszFormName, * pszFileName, * pszEndMarker = pszBuffer + stNumBytes;

	char* pszNext, * pszBegin = pszBuffer;
	char* pszEnd = Algorithm::memstr (pszBegin, m_pszSeparator, stBufferLength);

	*pstBytesProcessed = 0;
	*pstBytesRemaining = stNumBytes;
	
	while (pszEnd != NULL) {

		pszNext = pszEnd + m_stSeparatorLength + 2;
		if (pszNext >= pszEndMarker) {
			break;
		}
		
		pszNext = Algorithm::memstr (pszNext, m_pszSeparator, pszEndMarker - pszNext);
		if (pszNext == NULL) {
			break;
		}

		// We have a form to parse, because there was another form terminating the one we discovered
		stTempLength = pszNext - pszEnd;
		pszEnd += m_stSeparatorLength + 2;	// Jump "\r\n"
		
		// Jump over the Content-Disposition: form-data; name="XXX"
		pszEnd += 37;

		if (pszEnd > pszEndMarker) {
			break;
		}

		if (*pszEnd == '\"') {
			pszEnd ++;

			// Find the end quote
			pszBegin = strstr (pszEnd, "\"");
		
		} else {

			// Some browsers like fucking Lynx don't use quotes
			pszBegin = strstr (pszEnd, "\r\n");
		}

		if (pszBegin == NULL) {
			break;
		}
		
		*pszBegin = '\0';
		
		// That's our form name
		pszFormName = pszEnd;
		
		// Determine form type
		pszEnd += strlen (pszFormName) + 1;

		if (pszEnd > pszEndMarker) {
			break;
		}
		
		if (*pszEnd == ';') {
			
			// It's a file (a short one)
			ftFormType = FILE_FORM;
			
			// Jump over the semicolon space and filename
			pszEnd += 12;
			
			// Find the end quote and cap it with a null character
			pszBegin = strstr (pszEnd, "\"");
			*pszBegin = '\0';

			// That's the file name on the client side
			pszFileName = pszEnd;

			if (*pszFileName == '\0') {

				pszFileName = NULL;
				pszFormValue = NULL;
			
			} else {
				
				// An upload!
				m_iNumFiles ++;

				// Find the beginning of the content-type
				pszEnd += strlen (pszFileName) + 3;
				
				// Ignore the content-type and find the data
				pszEnd = strstr (pszEnd, "\r\n");
				pszEnd += 4;
				
				if (pszEnd < pszNext - 2) {
					
					// Open a temporary file for the data itself
					tfTempFile.Open();
					
					// Write the data minus the \r\n at the end of the buffer
					tfTempFile.Write (pszEnd, pszNext - pszEnd - 2);
					pszFormValue = tfTempFile.GetName();
					tfTempFile.Close();

					iErrCode = File::GetFileSize (pszFormValue, &stFileSize);
					if (iErrCode != OK) {
						File::DeleteFile (pszFormValue);
						return iErrCode;
					}

					m_stNumBytes += stFileSize;

				} else {

					pszFormValue = NULL;
				}
			}
			
		} else {
			
			// It's a regular form
			ftFormType = SIMPLE_FORM;
			
			// Jump over the \0\r\n
			pszEnd += 4;
			
			// Get the form's value
			*(pszNext - 2) = '\0';
			pszFormValue = pszEnd;
			
			// NULL filename
			pszFileName = NULL;
		}
		
		// Add form to table
		iErrCode = AddHttpForm (ftFormType, pszFormName, pszFormValue, pszFileName);
		if (iErrCode != OK) {
			return iErrCode;
		}

		// Leave pszEnd pointing to start of next form
		pszEnd = pszNext;
		*pszEnd = *m_pszSeparator;
		
		// Adjust buffer for next round
		stBufferLength -= stTempLength;

		memcpy (pszBuffer, pszEnd, stBufferLength);
		pszBuffer[stBufferLength] = '\0';
		pszEndMarker = pszBuffer + stBufferLength;
		
		pszBegin = pszEnd = pszBuffer;
		(*pstBytesProcessed) += stTempLength;
		(*pstBytesRemaining) -= stTempLength;
	}

	return OK;
}

unsigned int HttpRequest::GetNumFilesUploaded() {
	return m_iNumFiles;
}

size_t HttpRequest::GetNumBytesInUploadedFiles() {
	return m_stNumBytes;
}

bool HttpRequest::ParsedUriForms() const {
	return m_bParsedUriForms;
}

const char* HttpRequest::GetParsedUriForms() const {

	if (!m_bParsedUriForms || m_pszUri == NULL) {
		return NULL;
	}

	// Hack alert
	return m_pszUri + strlen (m_pszUri) + 1;
}

int HttpRequest::RequestHashValue::GetHashValue (const char* pszKey, unsigned int iNumBuckets, 
												 const void* pHashHint) {
	
	return Algorithm::GetStringHashValue (pszKey, iNumBuckets, false);
}

bool HttpRequest::RequestEquals::Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint) {
	
	// Case sensitive
	return strcmp (pszLeft, pszRight) == 0;
}