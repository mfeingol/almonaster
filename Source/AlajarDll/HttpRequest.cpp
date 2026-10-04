// HttpRequest.cpp: implementation of the HttpRequest class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar.dll:  a component of Alajar 1.0
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

#include "HttpRequest.h"
#include "PageSource.h"
#include "HttpServer.h"

#include "Osal/Algorithm.h"
#include "Osal/Crypto.h"
#include "Osal/HashTable.h"
#include "Osal/TempFile.h"

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
    m_vVersion = UNSUPPORTED_HTTP_VERSION;
    m_atAuth = AUTH_NONE;

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

    m_bCanonicalPath = false;

    m_bConnectionHeaderParsed = false;
    m_bContentTypeHeaderParsed = false;
    m_bContentLengthHeaderParsed = false;
    m_bCookieHeaderParsed = false;
    m_bIfModifiedSinceHeaderParsed = false;
    m_bHostHeaderParsed = false;
    m_bUserAgentHeaderParsed = false;
    m_bRefererHeaderParsed = false;
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

    m_strAuthUserName.Clear();
    m_strAuthNonce.Clear();
    m_strAuthRealm.Clear();
    m_strAuthDigestUri.Clear();
    m_strAuthRequestResponse.Clear();
    m_strNonceCount.Clear();
    m_strCNonce.Clear();
    m_strQop.Clear();

    if (m_pszSeparator != NULL) {
        *m_pszSeparator = '\0';
    }

    if (m_pszUri != NULL) {
        *m_pszUri = '\0';
    }

    m_pszFileName[0] = '\0';
    m_strBrowserName.Clear();
    m_strHeaders.Clear();
    m_strHostName.Clear();
    m_strReferer.Clear();

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
    m_vVersion = UNSUPPORTED_HTTP_VERSION;
    m_atAuth = AUTH_NONE;

    m_stSeparatorLength = 0;
    m_stContentLength = 0;
    m_msParseTime = 0;

    m_iNumFiles = 0;
    m_stNumBytes = 0;

    m_bParsedUriForms = false;
    m_bMultiPartForms = false;

    m_bCanonicalPath = false;

    m_bConnectionHeaderParsed = false;
    m_bContentTypeHeaderParsed = false;
    m_bContentLengthHeaderParsed = false;
    m_bCookieHeaderParsed = false;
    m_bIfModifiedSinceHeaderParsed = false;
    m_bHostHeaderParsed = false;
    m_bUserAgentHeaderParsed = false;
    m_bRefererHeaderParsed = false;
}

MilliSeconds HttpRequest::GetRequestParseTime() const{
    return m_msParseTime;
}

HttpMethod HttpRequest::GetMethod() {
    return m_iMethod;
}

HttpVersion HttpRequest::GetVersion() {
    return m_vVersion;
}

const char* HttpRequest::GetUri() {
    return m_pszUri;
}

const char* HttpRequest::GetFileName() {
    return m_pszFileName;
}

bool HttpRequest::IsFileNameCanonical() {
    return m_bCanonicalPath;
}

const char* HttpRequest::GetBrowserName() {
    return m_strBrowserName.GetCharPtr();
}

const char* HttpRequest::GetClientIP() {
    return m_strIPAddress.GetCharPtr();
}

const char* HttpRequest::GetReferer() {
    return m_strReferer.GetCharPtr();
}

const char* HttpRequest::GetHost() {
    return m_strHostName.GetCharPtr();
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

const char* HttpRequest::GetAuthenticationUserName() {
    return m_strAuthUserName;
}

const char* HttpRequest::GetAuthenticationNonce() {
    return m_strAuthNonce;
}

int HttpRequest::BasicAuthenticate (const char* pszPassword, bool* pbAuthenticated) {

    Assert (pbAuthenticated != NULL);
    *pbAuthenticated = false;

    // We don't do blank passwords
    if (String::IsBlank (pszPassword))
        return OK;

    // We accept either a basic auth header or no header
    if (m_atAuth != AUTH_BASIC && m_atAuth != AUTH_NONE)
        return OK;

    // Check user's arguments
    if (m_strAuthUserName.IsBlank() || 
        m_strAuthPassword.IsBlank())
        
        return ERROR_FAILURE;

    *pbAuthenticated = strcmp (m_strAuthPassword.GetCharPtr(), pszPassword) == 0;
    return OK;
}

int HttpRequest::DigestAuthenticate (const char* pszPassword, bool* pbAuthenticated) {

    int iErrCode;

    Assert (pbAuthenticated != NULL);
    *pbAuthenticated = false;

    // We don't do blank passwords
    if (String::IsBlank (pszPassword))
        return OK;

    // We accept either a digest auth header or no header
    if (m_atAuth != AUTH_DIGEST && m_atAuth != AUTH_NONE)
        return OK;

    // Check user's arguments
    if (m_strAuthUserName.IsBlank() || 
        m_strAuthRealm.IsBlank() || 
        m_strAuthNonce.IsBlank() ||
        m_strAuthDigestUri.IsBlank() ||
        m_strAuthRequestResponse.IsBlank() ||
        m_strNonceCount.IsBlank() ||
        m_strCNonce.IsBlank() ||
        m_strQop.IsBlank())

        return ERROR_FAILURE;

    // Compute A1 hash
    char pszA1Hash[DIGEST_HASH_TEXT_SIZE];
    iErrCode = ComputeA1Hash (pszPassword, pszA1Hash);
    if (iErrCode != OK)
        return iErrCode;

    // Compute A2 hash
    char pszA2Hash[DIGEST_HASH_TEXT_SIZE];
    iErrCode = ComputeA2Hash (pszA2Hash);
    if (iErrCode != OK)
        return iErrCode;
    
    // Compute final hash
    char pszFinalHash[DIGEST_HASH_TEXT_SIZE];
    iErrCode = ComputeFinalHash (pszA1Hash, pszA2Hash, pszFinalHash);
    if (iErrCode != OK)
        return iErrCode;

    const char* pszRequestHash = m_strAuthRequestResponse.GetCharPtr();
    *pbAuthenticated = strcmp (pszFinalHash, pszRequestHash) == 0;

    return OK;    
}

int HttpRequest::ComputeA1Hash (const char* pszPassword, char pszA1 [DIGEST_HASH_TEXT_SIZE]) {

    int iErrCode;
    Crypto::HashMD5 hash;

    iErrCode = hash.HashData (m_strAuthUserName.GetCharPtr(), m_strAuthUserName.GetLength());
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (":", sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (m_strAuthRealm.GetCharPtr(), m_strAuthRealm.GetLength());
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (":", sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (pszPassword, strlen (pszPassword));
    if (iErrCode != OK)
        return iErrCode;

    char pbHash [MD5_HASH_SIZE];
    iErrCode = hash.GetHash (pbHash, sizeof (pbHash));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = Algorithm::HexEncode (pbHash, MD5_HASH_SIZE, pszA1, DIGEST_HASH_TEXT_SIZE);
    if (iErrCode != OK)
        return iErrCode;

    return OK;
}

int HttpRequest::ComputeA2Hash (char pszA2 [DIGEST_HASH_TEXT_SIZE]) {

    int iErrCode;
    Crypto::HashMD5 hash;

    const char* pszMethod = HttpMethodText [m_iMethod];
    iErrCode = hash.HashData (pszMethod, strlen (pszMethod));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (":", sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (m_strAuthDigestUri.GetCharPtr(), m_strAuthDigestUri.GetLength());
    if (iErrCode != OK)
        return iErrCode;

    char pbHash [MD5_HASH_SIZE];
    iErrCode = hash.GetHash (pbHash, sizeof (pbHash));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = Algorithm::HexEncode (pbHash, MD5_HASH_SIZE, pszA2, DIGEST_HASH_TEXT_SIZE);
    if (iErrCode != OK)
        return iErrCode;

    return OK;
}

int HttpRequest::ComputeFinalHash (const char pszA1[DIGEST_HASH_TEXT_SIZE], const char pszA2[DIGEST_HASH_TEXT_SIZE], char pszFinal[DIGEST_HASH_TEXT_SIZE]) {

    int iErrCode;
    Crypto::HashMD5 hash;

    iErrCode = hash.HashData (pszA1, (DIGEST_HASH_TEXT_SIZE - 1) * sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (":", sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (m_strAuthNonce.GetCharPtr(), m_strAuthNonce.GetLength());
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (":", sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (m_strNonceCount.GetCharPtr(), m_strNonceCount.GetLength());
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (":", sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (m_strCNonce.GetCharPtr(), m_strCNonce.GetLength());
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (":", sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (m_strQop.GetCharPtr(), m_strQop.GetLength());
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (":", sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = hash.HashData (pszA2, (DIGEST_HASH_TEXT_SIZE - 1) * sizeof (char));
    if (iErrCode != OK)
        return iErrCode;

    char pbHash [MD5_HASH_SIZE];
    iErrCode = hash.GetHash (pbHash, sizeof (pbHash));
    if (iErrCode != OK)
        return iErrCode;

    iErrCode = Algorithm::HexEncode (pbHash, MD5_HASH_SIZE, pszFinal, DIGEST_HASH_TEXT_SIZE);
    if (iErrCode != OK)
        return iErrCode;

    return OK;
}

int HttpRequest::ParseRequestHeader (char* pszLine) {

    int iErrCode;
    size_t stLen;

    char pszFilePath [OS::MaxFileNameLength + 1];

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
    pszVer ++;      // Safe because there's a \0 at the end of the buffer

    // Handle method
    if (_stricmp (pszMethod, HttpMethodText[GET]) == 0) {
        m_iMethod = GET;
    }
    else if (_stricmp (pszMethod, HttpMethodText[POST]) == 0) {
        m_iMethod = POST;
    }
    else if (_stricmp (pszMethod, HttpMethodText[PUT]) == 0) {
        m_iMethod = PUT;
    }
    else if (_stricmp (pszMethod, HttpMethodText[HEAD]) == 0) {
        m_iMethod = HEAD;
    }
    else if (_stricmp (pszMethod, HttpMethodText[TRACE]) == 0) {
        m_iMethod = TRACE;
    }
    else return ERROR_UNSUPPORTED_HTTP_METHOD;

    // Handle version
    if (_stricmp (pszVer, "HTTP/1.1") == 0) {
        m_vVersion = HTTP11;
    }
    else if (_stricmp (pszVer, "HTTP/1.0") == 0) {
        m_vVersion = HTTP10;
    }
    else if (_stricmp (pszVer, "HTTP/0.9") == 0) {
        m_vVersion = HTTP09;
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

    iErrCode = Algorithm::UnescapeString (pszUri, m_pszUri, m_stUriLength);
    if (iErrCode != OK) {

        if (iErrCode != ERROR_SMALL_BUFFER) {
            return iErrCode;
        }

        stLen = strlen (pszUri);
        if (stLen >= MAX_URI_LEN) {
            return ERROR_MALFORMED_REQUEST;
        }
        stLen ++;
        
        delete [] m_pszUri;
        m_stUriLength = 0;

        m_pszUri = new char [stLen];
        if (m_pszUri == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
        m_stUriLength = stLen;

        iErrCode = Algorithm::UnescapeString (pszUri, m_pszUri, stLen);
        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    // Disallow URIs with carriage returns
    if (strstr (m_pszUri, "\n") != NULL) {
        return ERROR_MALFORMED_REQUEST;
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
    m_pPageSource = m_pHttpServer->GetPageSource (pszUri + 1);  // Adds a reference on the pagesource
    if (m_pPageSource == NULL) {

        // Must be the default, then
        m_pPageSource = m_pHttpServer->GetDefaultPageSource();  // Adds a reference on the pagesource
        Assert (m_pPageSource != NULL);

        // Build path
        strncpy (pszFilePath, m_pPageSource->GetBasePath(), OS::MaxFileNameLength);
        pszFilePath [OS::MaxFileNameLength] = '\0';

        char* pszUseURI = m_pszUri;
        if (pszUseURI[0] == '/' || pszUseURI[0] == '\\') {
            pszUseURI ++;
        }
        strncat (pszFilePath, pszUseURI, OS::MaxFileNameLength - strlen (pszFilePath));

    } else {

        // Build path
        strncpy (pszFilePath, m_pPageSource->GetBasePath(), OS::MaxFileNameLength);
        pszFilePath [OS::MaxFileNameLength] = '\0';

        if (pszNextSlash != NULL) {
            strncat (pszFilePath, pszNextSlash, OS::MaxFileNameLength - strlen (pszFilePath));
        }
    }

    // Null cap, in any case
    pszFilePath [OS::MaxFileNameLength] = '\0';

    // Canonicalize if possible
    if (File::ResolvePath (pszFilePath, m_pszFileName) == OK) {
        m_bCanonicalPath = true;
    } else {
        strcpy (m_pszFileName, pszFilePath);
    }

    return OK;
}


int HttpRequest::ParseHeader (char* pszLine) {
    
    int iErrCode = OK;
    bool bFreeBuf;

    UTCTime tLastModified;

    char* pszValue = NULL, * pszBuf;

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
    memcpy (pszBuf, pszLine, stLineLen + 1);

    char* pszEnd = pszBuf + stLineLen;
    char* pszHeader = strtok (pszBuf, " ");
    pszValue = strtok (NULL, "");
    
    if (pszValue == NULL) {
        pszValue = "";
    }

    // Handle headers
    switch (pszLine[0]) {
        
    // Authorization
    case 'A':
    case 'a':

        if (_stricmp (pszHeader, "Authorization:") == 0) {

            if (pszValue[0] == 'B') {

                const size_t cchBasicSpaceLen = countof ("Basic ") - 1;

                if (_strnicmp (pszValue, "Basic ", cchBasicSpaceLen)  == 0) {
                    
                    // Disallow duplicate headers
                    if (m_atAuth != AUTH_NONE) {
                        iErrCode = ERROR_MALFORMED_REQUEST;
                        goto Cleanup;
                    }

                    m_atAuth = AUTH_BASIC;

                    char* pszTemp = pszValue + cchBasicSpaceLen;
                    size_t cbEncodeLen = strlen (pszTemp)+ 1;

                    char* pszDecode = new char [cbEncodeLen + 1];
                    Algorithm::AutoDelete<char> autoDeleteDecode (pszDecode, true);

                    if (pszDecode == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }

                    size_t cbDecoded;
                    iErrCode = Algorithm::DecodeBase64 (pszTemp, pszDecode, cbEncodeLen, &cbDecoded);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                    Assert (cbDecoded <= cbEncodeLen);
                    pszDecode [cbDecoded] = '\0';

                    pszTemp = strtok (pszDecode, ":");
                    if (pszTemp == NULL) {
                        iErrCode = ERROR_MALFORMED_REQUEST;
                        goto Cleanup;
                    }
                    m_strAuthUserName = pszTemp;

                    pszTemp = strtok (NULL, "");
                    if (pszTemp == NULL) {
                        iErrCode = ERROR_MALFORMED_REQUEST;
                        goto Cleanup;
                    }
                    m_strAuthPassword = pszTemp;
                }
            }

            else if (pszValue[0] == 'D') {

                const size_t cchDigestSpaceLen = countof ("Digest ") - 1;

                if (String::StrniCmp (pszValue, "Digest ", cchDigestSpaceLen) == 0) {

                    // Disallow duplicate headers
                    if (m_atAuth != AUTH_NONE) {
                        iErrCode = ERROR_MALFORMED_REQUEST;
                        goto Cleanup;
                    }

                    m_atAuth = AUTH_DIGEST;

                    char* pszAttribute = pszValue + cchDigestSpaceLen;
                    char* pszEndAttribute = pszValue + strlen (pszValue);
                    while (pszAttribute < pszEndAttribute) {

                        if (String::StrniCmp (pszAttribute, "username", countof ("username") - 1) == 0) {
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, &m_strAuthUserName);
                        }

                        else if (String::StrniCmp (pszAttribute, "realm", countof ("realm") - 1) == 0) {
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, &m_strAuthRealm);
                        }

                        else if (String::StrniCmp (pszAttribute, "nonce", countof ("nonce") - 1) == 0) {
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, &m_strAuthNonce);
                        }

                        else if (String::StrniCmp (pszAttribute, "uri", countof ("uri") - 1) == 0) {
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, &m_strAuthDigestUri);
                        }

                        else if (String::StrniCmp (pszAttribute, "response", countof ("response") - 1) == 0) {
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, &m_strAuthRequestResponse);
                        }

                        else if (String::StrniCmp (pszAttribute, "nc", countof ("nc") - 1) == 0) {
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, &m_strNonceCount);
                        }

                        else if (String::StrniCmp (pszAttribute, "cnonce", countof ("cnonce") - 1) == 0) {
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, &m_strCNonce);
                        }

                        else if (String::StrniCmp (pszAttribute, "qop", countof ("qop") - 1) == 0) {
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, &m_strQop);
                        }

                        else {

                            // Unrecognized attribute
                            pszAttribute = ParseAuthenticationAttribute (pszAttribute, NULL);
                        }
                    }
                }
            }
        }
        
        break;
        
    // Connection, Content-Length, Cookie
    case 'C':
    case 'c':
        
        if (_stricmp (pszHeader, "Connection:") == 0) {

            if (m_bConnectionHeaderParsed) {
                iErrCode = ERROR_MALFORMED_REQUEST;
                goto Cleanup;
            }
            m_bConnectionHeaderParsed = true;
            m_bKeepAlive = (_stricmp (pszValue, "Keep-Alive") == 0);
        }
        else if (_stricmp (pszHeader, "Content-Type:") == 0) {
            
            if (m_bContentTypeHeaderParsed) {
                iErrCode = ERROR_MALFORMED_REQUEST;
                goto Cleanup;
            }
            m_bContentTypeHeaderParsed = true;

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

                size_t cchBoundary = strlen (pszValue);

                // Opera 3.50 puts quotes around the separator
                if (cchBoundary >= 2 && pszValue[0] == '\"' && pszValue[cchBoundary - 1] == '\"') {
                    pszValue ++;
                    cchBoundary -= 2;
                }

                if (cchBoundary == 0) {
                    iErrCode = ERROR_MALFORMED_REQUEST;
                    goto Cleanup;
                }

                // m_stSeparatorSpace is larger than the whole header value, so "--" + boundary + '\0' fits
                memcpy (m_pszSeparator + 2, pszValue, cchBoundary);
                m_pszSeparator[cchBoundary + 2] = '\0';
                m_stSeparatorLength = cchBoundary + 2;
            }
        }
        else if (_stricmp (pszHeader, "Content-Length:") == 0) {

            if (m_bContentLengthHeaderParsed) {
                iErrCode = ERROR_MALFORMED_REQUEST;
                goto Cleanup;
            }
            m_bContentLengthHeaderParsed = true;

            m_stContentLength = atoi (pszValue);
        }
        else if (_stricmp (pszHeader, "Cookie:") == 0) {

            if (m_bCookieHeaderParsed) {
                iErrCode = ERROR_MALFORMED_REQUEST;
                goto Cleanup;
            }
            m_bCookieHeaderParsed = true;
            
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
                m_ppszCookieName = NULL;
                m_stCookieSpace = 0;

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

                // Initialize cookie table
                if (m_phtCookieTable == NULL) {

                    m_phtCookieTable = new HashTable<const char*, Cookie*, RequestHashValue, RequestEquals> (NULL, NULL);
                    if (m_phtCookieTable == NULL || !m_phtCookieTable->Initialize (iNumCookies)) {
                        delete m_phtCookieTable;
                        m_phtCookieTable = NULL;
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        if (iNumSemicolons > 100) {
                            delete [] ppszCookie;
                        }
                        goto Cleanup;
                    }
                }

                m_iNumCookies = 0;
                for (i = 0; i < iNumCookies; i ++) {

                    Cookie* pCookie = NULL, * pMasterCookie = NULL;

                    char* pszEquals = strstr (ppszCookie[i], "=");
                    if (pszEquals == NULL) {
                        // Sheesh, I don't know...
                        continue;
                    }
                    *pszEquals = '\0';

                    char* pszCookieName = ppszCookie[i];
                    char* pszCookieValue = pszEquals + 1;

                    // Adjust for space after semicolon
                    // Some browsers do that...
                    if (i > 0 && *pszCookieName == ' ') {
                        pszCookieName ++;
                    }

                    // Create a new cookie object
                    pCookie = Cookie::CreateInstance (pszCookieName, pszCookieValue);
                    if (pCookie == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        if (iNumSemicolons > 100) {
                            delete [] ppszCookie;
                        }
                        goto Cleanup;
                    }

                    // Does the cookie already exist?
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
        
        if (_stricmp (pszHeader, "Host:") == 0 && !String::IsBlank (pszValue)) {

            if (m_bHostHeaderParsed) {
                iErrCode = ERROR_MALFORMED_REQUEST;
                goto Cleanup;
            }
            m_bHostHeaderParsed = true;

            m_strHostName = pszValue;
            if (m_strHostName.GetCharPtr() == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
        }
        
        break;
        
    case 'I':
    case 'i':

        if (_stricmp(pszHeader, "If-Modified-Since:") == 0 && !String::IsBlank(pszValue)) {

            if (m_bIfModifiedSinceHeaderParsed) {
                iErrCode = ERROR_MALFORMED_REQUEST;
                goto Cleanup;
            }
            m_bIfModifiedSinceHeaderParsed = true;

            // Only static files can be served from the client's cache.
            // Page sources that override GET generate their responses dynamically
            if (m_pPageSource != NULL && !m_pPageSource->OverrideGet()) {
                m_bCached = !File::WasFileModifiedAfter(m_pszFileName, pszValue, &tLastModified);
            }
        }

        break;
        
    // User-Agent
    case 'U':
    case 'u':
        
        if (_stricmp (pszHeader, "User-Agent:") == 0 && !String::IsBlank (pszValue)) {

            if (m_bUserAgentHeaderParsed) {
                iErrCode = ERROR_MALFORMED_REQUEST;
                goto Cleanup;
            }
            m_bUserAgentHeaderParsed = true;

            m_strBrowserName = pszValue;
            if (m_strBrowserName.GetCharPtr() == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
        }
        break;
        
    case 'R':
    case 'r':

        if (_stricmp (pszHeader, "Referer:") == 0 && !String::IsBlank (pszValue)) {

            if (m_bRefererHeaderParsed) {
                iErrCode = ERROR_MALFORMED_REQUEST;
                goto Cleanup;
            }
            m_bRefererHeaderParsed = true;

            m_strReferer = pszValue;
            if (m_strReferer.GetCharPtr() == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
        }

    default:

        // Ignore unhandled headers
        break;
    }

Cleanup:

    if (bFreeBuf)
        delete [] pszBuf;

    return iErrCode;
}

// username="foo", response="bar"
char* HttpRequest::ParseAuthenticationAttribute (char* pszCursor, String* pstrValue) {

    // Advance to equals at end of name
    while (*pszCursor != '\0' && *pszCursor != '=')
        pszCursor ++;

    // Skip equals and spaces and new lines
    while (*pszCursor == '=' || *pszCursor == ' ' || *pszCursor == '\r' || *pszCursor == '\n')
        pszCursor ++;

    // Skip initial quote
    if (*pszCursor == '\"')
        pszCursor ++;

    // Find the end of the value - could be end of string, a comma or an end quote
    char* pszTemp = pszCursor;
    while (*pszTemp != '\0' && *pszTemp != ',' && *pszTemp != '\"')
        pszTemp ++;

    char cRestore = *pszTemp;
    *pszTemp = '\0';

    if (pstrValue != NULL) {
        *pstrValue = pszCursor;
    }

    *pszTemp = cRestore;

    // Find the next attribute
    while (*pszTemp == ',' || *pszTemp == '\"' || *pszTemp == ' ' || *pszTemp == '\r' || *pszTemp == '\n')
        pszTemp ++;

    return pszTemp;
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

    char* pszBuffer = new char[MAX_REQUEST_LENGTH + 1];
    if (pszBuffer == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }
    Algorithm::AutoDelete<char> auto_pszBuffer(pszBuffer, true);

    char pszLine[MAX_REQUEST_LENGTH + 1];
    size_t stNumBytes, stBeginRecv = 0, stLineLength;
    bool bEndHeaders = false, bFirstLine = true;

    char* pszBegin, * pszEnd, * pszEndMarker;

    // Recv as big a block of data as possible
    // This loop will terminate when the end of the headers is received
    while (true) {

        if (stBeginRecv >= MAX_REQUEST_LENGTH) {
            return ERROR_MALFORMED_REQUEST;
        }

        iErrCode = m_pSocket->Recv (pszBuffer + stBeginRecv, MAX_REQUEST_LENGTH - stBeginRecv, &stNumBytes);
        if (iErrCode != OK) {
            return ERROR_SOCKET_CLOSED;
        }
        stBeginRecv += stNumBytes;

        pszBuffer[stBeginRecv] = '\0';
        pszBegin = pszBuffer;
        pszEnd = strstr (pszBuffer, "\r\n");
        pszEndMarker = pszBuffer + stBeginRecv;

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
            memmove(pszBuffer, pszBegin, stBeginRecv);
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
            return ERROR_MALFORMED_REQUEST;
        }

        // No more than 10MB, sorry
        if (m_stContentLength > 10 * 1024 * 1024)
        {
            return ERROR_MALFORMED_REQUEST;
        }

        // Reallocate the buffer if necessary, keeping any body bytes that arrived with the headers
        if (m_stContentLength > MAX_REQUEST_LENGTH)
        {
            char* pszNewBuffer = new char[m_stContentLength + 1];
            if (pszNewBuffer == NULL)
            {
                return ERROR_OUT_OF_MEMORY;
            }

            memcpy(pszNewBuffer, pszBuffer, stBeginRecv);
            delete[] pszBuffer;
            pszBuffer = pszNewBuffer;
        }

        // Receive the rest of the data
        while (m_stContentLength > stBeginRecv)
        {
            iErrCode = m_pSocket->Recv(pszBuffer + stBeginRecv, m_stContentLength - stBeginRecv, &stNumBytes);
            if (iErrCode != OK) {
                return ERROR_SOCKET_CLOSED;
            }
            stBeginRecv += stNumBytes;
        }

        // The whole body is now in the buffer. Ignore anything the client sent past it
        pszBuffer[m_stContentLength] = '\0';

        // What kind of forms?
        if (m_pszSeparator == NULL || m_pszSeparator[0] == '\0') {
            iErrCode = HandleSimpleForms (pszBuffer);
        } else {
            iErrCode = HandleMultipartForms (pszBuffer, m_stContentLength);
        }

        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    Assert (iErrCode == OK);

    // Check results
    if (m_vVersion == UNSUPPORTED_HTTP_VERSION) {
        return ERROR_UNSUPPORTED_HTTP_VERSION;
    }
    
    if (m_iMethod == UNSUPPORTED_HTTP_METHOD) {
        return ERROR_UNSUPPORTED_HTTP_METHOD;
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


int HttpRequest::HandleSimpleForms (char* pszBuffer) {

    // The entire body has already been received and null-terminated
    size_t stParsed;
    return ParseForms (pszBuffer, &stParsed, true);
}


int HttpRequest::AddHttpForm (HttpFormType ftFormType, const char* pszHttpFormName, 
                              const char* pszHttpFormValue, const char* pszFileName) {

    int iErrCode = OK;

    HttpForm* pHttpForm, * pMasterHttpForm;
    bool bRetVal;

    Assert (pszHttpFormName != NULL);
    Assert (ftFormType >= SIMPLE_FORM && ftFormType <= LARGE_SIMPLE_FORM);

    if (pszHttpFormValue != NULL && pszHttpFormValue[0] == '\0') {
        pszHttpFormValue = NULL;
    }

    if (pszFileName != NULL && pszFileName[0] == '\0') {
        pszFileName = NULL;
    }

    // Initialize the form hash table
    if (m_phtHttpFormTable == NULL) {
        
        m_phtHttpFormTable = new HashTable<const char*, HttpForm*, RequestHashValue, RequestEquals> (NULL, NULL);
        if (m_phtHttpFormTable == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        if (!m_phtHttpFormTable->Initialize (DEFAULT_NUM_FORMS)) {
            delete m_phtHttpFormTable;
            m_phtHttpFormTable = NULL;
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

        // Resize? Do this before inserting into the table, so a failure can't leave a released form there
        if (m_iNumHttpFormsSpace == m_iNumHttpForms) {

            unsigned int iNumForms = m_iNumHttpForms == 0 ? 10 : m_iNumHttpForms * 2;

            // Forms in the first half, names in the second
            HttpForm** ppHttpForms = new HttpForm* [iNumForms * 2];
            if (ppHttpForms == NULL) {
                pHttpForm->Release();
                return ERROR_OUT_OF_MEMORY;
            }

            const char** ppszHttpFormName = (const char**) ppHttpForms + iNumForms;

            if (m_iNumHttpForms > 0) {
                memcpy (ppHttpForms, m_ppHttpForms, m_iNumHttpForms * sizeof (HttpForm*));
                memcpy (ppszHttpFormName, m_ppszHttpFormName, m_iNumHttpForms * sizeof (const char*));
            }

            if (m_ppHttpForms != NULL) {
                delete [] m_ppHttpForms;
            }

            m_ppHttpForms = ppHttpForms;
            m_ppszHttpFormName = ppszHttpFormName;

            m_iNumHttpFormsSpace = iNumForms;
        }

        // Insert a new master form
        const char* pszSafeHttpFormName = pHttpForm->GetName();

        bRetVal = m_phtHttpFormTable->Insert (pszSafeHttpFormName, pHttpForm);
        if (!bRetVal) {
            pHttpForm->Release();
            return ERROR_OUT_OF_MEMORY;
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
                if (stLastValueLen >= 2 &&
                    pszValue [stLastValueLen - 2] == '\r' &&
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

    // The entire body has already been received and null-terminated at pszBuffer[stNumBytes].
    // Each part looks like this:
    //
    // --separator\r\n
    // Content-Disposition: form-data; name="XXX"[; filename="YYY"\r\nContent-Type: ZZZ]\r\n
    // \r\n
    // data\r\n
    //
    // and the last part is followed by --separator--\r\n

    int iErrCode;
    TempFile tfTempFile;

    const size_t cchDisposition = countof ("Content-Disposition: form-data; name=") - 1;
    const size_t cchFileName = countof ("; filename=\"") - 1;

    char* pszEndMarker = pszBuffer + stNumBytes;
    char* pszEnd = Algorithm::memstr (pszBuffer, m_pszSeparator, stNumBytes);

    while (pszEnd != NULL) {

        // Jump the separator and "\r\n"
        if ((size_t) (pszEndMarker - pszEnd) <= m_stSeparatorLength + 2) {
            break;
        }
        char* pszPart = pszEnd + m_stSeparatorLength + 2;

        // Find the separator that terminates this part
        char* pszNext = Algorithm::memstr (pszPart, m_pszSeparator, pszEndMarker - pszPart);
        if (pszNext == NULL) {
            break;
        }

        // The part's data is followed by "\r\n" before the next separator.
        // Null-terminate the part there so nothing below can parse past it
        if ((size_t) (pszNext - pszPart) < cchDisposition + 2) {
            return ERROR_MALFORMED_REQUEST;
        }
        char* pszPartEnd = pszNext - 2;
        *pszPartEnd = '\0';

        // Jump over the Content-Disposition: form-data; name=
        char* pszCursor = pszPart + cchDisposition;
        char* pszFormName, * pszFileName = NULL;
        const char* pszFormValue = NULL;
        HttpFormType ftFormType;

        if (*pszCursor == '\"') {
            pszCursor ++;
            pszFormName = pszCursor;
            pszCursor = strchr (pszCursor, '\"');
        } else {
            // Some browsers like Lynx don't use quotes
            pszFormName = pszCursor;
            pszCursor = strstr (pszCursor, "\r\n");
        }

        if (pszCursor == NULL) {
            return ERROR_MALFORMED_REQUEST;
        }
        *pszCursor = '\0';
        pszCursor ++;

        // Determine form type
        if (*pszCursor == ';') {

            ftFormType = FILE_FORM;

            if (strncmp (pszCursor, "; filename=\"", cchFileName) != 0) {
                return ERROR_MALFORMED_REQUEST;
            }
            pszCursor += cchFileName;

            // Find the end quote and cap it with a null character
            char* pszQuote = strchr (pszCursor, '\"');
            if (pszQuote == NULL) {
                return ERROR_MALFORMED_REQUEST;
            }
            *pszQuote = '\0';

            // That's the file name on the client side
            pszFileName = pszCursor;

            if (*pszFileName == '\0') {

                pszFileName = NULL;

            } else {

                // An upload!
                m_iNumFiles ++;

                // Ignore the content-type and find the data
                char* pszData = strstr (pszQuote + 1, "\r\n\r\n");
                if (pszData == NULL) {
                    return ERROR_MALFORMED_REQUEST;
                }
                pszData += 4;

                if (pszData < pszPartEnd) {

                    // Write the data to a temporary file
                    iErrCode = tfTempFile.Open();
                    if (iErrCode != OK) {
                        return iErrCode;
                    }

                    iErrCode = tfTempFile.Write (pszData, pszPartEnd - pszData);
                    tfTempFile.Close();

                    if (iErrCode != OK) {
                        tfTempFile.Delete();
                        return iErrCode;
                    }

                    pszFormValue = tfTempFile.GetName();

                    size_t stFileSize;
                    iErrCode = File::GetFileSize (pszFormValue, &stFileSize);
                    if (iErrCode != OK) {
                        File::DeleteFile (pszFormValue);
                        return iErrCode;
                    }

                    m_stNumBytes += stFileSize;
                }
            }

        } else {

            // It's a regular form. Jump over the "\r\n\r\n"
            ftFormType = SIMPLE_FORM;

            if (pszPartEnd - pszCursor < 4) {
                return ERROR_MALFORMED_REQUEST;
            }
            pszFormValue = pszCursor + 4;
        }

        // Add form to table
        iErrCode = AddHttpForm (ftFormType, pszFormName, pszFormValue, pszFileName);
        if (iErrCode != OK) {
            return iErrCode;
        }

        // Move on to the next part
        pszEnd = pszNext;
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