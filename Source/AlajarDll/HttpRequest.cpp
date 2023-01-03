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
        m_pszUri = new char [stLen];
        if (m_pszUri == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
        
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

        char* pszUseURI = m_pszUri;
        if (pszUseURI[0] == '/' || pszUseURI[0] == '\\') {
            pszUseURI ++;
        }
        strncat (pszFilePath, pszUseURI, OS::MaxFileNameLength - strlen (pszFilePath));

    } else {

        // Build path
        strncpy (pszFilePath, m_pPageSource->GetBasePath(), OS::MaxFileNameLength);

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

                m_stSeparatorLength = strlen (pszValue) + 1;

                if (*pszValue == '\"' && pszValue[m_stSeparatorLength - 2] == '\"') {

                    // Opera 3.50 puts quotes around the separator
                    m_stSeparatorLength -= 2;
                    strncpy (m_pszSeparator + 2, pszValue + 1, m_stSeparatorLength - 1);
                    m_pszSeparator[m_stSeparatorLength + 1] = '\0';

                } else {

                    // Normal case: no quotes
                    memcpy (m_pszSeparator + 2, pszValue, m_stSeparatorLength);
                    m_stSeparatorLength ++;
                }
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

            // TODO - if the method is overridden, this might be a bit ambitious
            if (_stricmp(pszHeader, "If-Modified-Since") == 0 && !String::IsBlank(pszValue)) {
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

        iErrCode = m_pSocket->Recv (pszBuffer + stBeginRecv, MAX_REQUEST_LENGTH, &stNumBytes);
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

        // No more than 1MB, sorry
        if (m_stContentLength > 1024 * 1024 * 1024)
        {
            return ERROR_MALFORMED_REQUEST;
        }

        // Reallocate the buffer if necessary
        if (m_stContentLength > MAX_REQUEST_LENGTH + 1)
        {
            delete[] pszBuffer;
            pszBuffer = new char[m_stContentLength + 1];
            if (pszBuffer == NULL)
            {
                return ERROR_OUT_OF_MEMORY;
            }
        }

        // Receive the rest of the data
        while (m_stContentLength > stBeginRecv)
        {
            iErrCode = m_pSocket->Recv(pszBuffer + stBeginRecv, m_stContentLength - stBeginRecv, &stNumBytes);
            if (iErrCode != OK) {
                return ERROR_SOCKET_CLOSED;
            }
            stBeginRecv += stNumBytes;
            pszBuffer[stBeginRecv] = '\0';
        }

        // What kind of forms?
        if (m_pszSeparator == NULL || m_pszSeparator[0] == '\0') {
            iErrCode = HandleSimpleForms (pszBuffer, MAX_REQUEST_LENGTH, stBeginRecv);
        } else {
            iErrCode = HandleMultipartForms (pszBuffer, stBeginRecv);
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
            if (iErrCode != OK) {
                return iErrCode;
            }

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

        }   // End while loop

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

    // Find the start of the form name
    char* pszName = strstr(pszBuffer + m_stSeparatorLength, "name=");
    if (pszName == NULL) {
        return ERROR_MALFORMED_REQUEST;
    }

    // Find the begin quote
    char* pszBeginQuote = strstr(pszName, "\"");
    if (pszBeginQuote == NULL) {
        return ERROR_MALFORMED_REQUEST;
    }

    // Find the end quote and cap it with a null character
    char* pszForm = pszBeginQuote + 1;
    char* pszEndQuote = strstr(pszForm, "\"");
    if (pszEndQuote == NULL) {
        return ERROR_MALFORMED_REQUEST;
    }
    *pszEndQuote = '\0';
    
    // That's our form name
    size_t stFormNameLen = strlen(pszForm) + 1;
    char* pszFormName;
    if (stFormNameLen > MAX_STACK_ALLOC)
    {
        pszFormName = new char [stFormNameLen];
        if (pszFormName == NULL)
        {
            return ERROR_OUT_OF_MEMORY;
        }
    }
    else
    {
        pszFormName = (char*) StackAlloc (stFormNameLen);
    }
    strcpy(pszFormName, pszForm);

    char* pszFileName = NULL, * pszNext;
    size_t stFileSize = 0;

    // Determine form type
    char* pszType = pszForm + stFormNameLen;
    if (pszType[0] == ';') {

        if (strncmp (pszType, "; filename=", 11) != 0) {
            iErrCode = ERROR_MALFORMED_REQUEST;
            goto Cleanup;
        }
        
        // Jump over the semicolon space and filename
        pszType += 12;
        
        // Find the end quote and cap it with a null character
        char* pszBegin = strstr (pszType, "\"");
        if (pszBegin == NULL) {
            iErrCode = ERROR_MALFORMED_REQUEST;
            goto Cleanup;
        }
        *pszBegin = '\0';
        
        // That's the file name
        pszFileName = pszType;       
        
        // Find the beginning of the content-type
        pszType += strlen (pszFileName) + 3;
        
        // Ignore the content-type and find the data
        pszType = strstr (pszType, "\r\n");
        if (pszType == NULL) {
            iErrCode = ERROR_MALFORMED_REQUEST;
            goto Cleanup;
        }
        pszType += 4;

        // It's a file
        ftFormType = FILE_FORM;
        m_iNumFiles ++;
    
    } else {

        if (strncmp (pszType, "\r\n\r\n", 4) != 0) {
            iErrCode = ERROR_MALFORMED_REQUEST;
            goto Cleanup;
        }

        pszType += 4;

        // It's a large data form
        ftFormType = SIMPLE_FORM;
    }
        
    // Open a temporary file for the data itself
    tfTempFile.Open();

    // Do we have a closing separator?
    pszNext = Algorithm::memstr(pszType, m_pszSeparator, stNumRemaining - (pszType - pszBuffer));
    if (pszNext != NULL)
    {
        // Write the data minus the \r\n at the end of the file's data (and of course the last separator)       
        tfTempFile.Write(pszType, pszNext - pszType);
    }
    else
    {
        // The file is too big for this buffer, so write the first chunk to disk
        tfTempFile.Write(pszType, pszEndMarker - pszType);
        *pstNumProcessed = stNumRemaining;
        *pstNumRemaining = 0;
        
        // Recv until we have the whole thing
        while (true)
        {
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
                    
                    size_t stNumData = pszNext - pszBuffer - 2;
                    
                    // EOF found!
                    iErrCode = tfTempFile.Write (pszBuffer, stNumData);
                    if (iErrCode != OK) {
                        tfTempFile.Close();
                        tfTempFile.Delete();
                        goto Cleanup;
                    }

                    // Fix the buffer
                    (*pstNumProcessed) += stNumData;
                    *pstNumRemaining = stNumBytesRecvd - stNumData;
                    memcpy (pszBuffer, pszNext, *pstNumRemaining);
                    pszBuffer[*pstNumRemaining] = '\0';
                    break;
                }
            }
        }   // End while
    }   // End didn't find a closing token
    
    // Close the temp file
    iErrCode = tfTempFile.GetSize (&stFileSize);
    if (iErrCode != OK) {
        tfTempFile.Close();
        tfTempFile.Delete();
        goto Cleanup;
    }

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
        pszEnd += m_stSeparatorLength + 2;  // Jump "\r\n"
        
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