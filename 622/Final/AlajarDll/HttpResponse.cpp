// HttpResponse.cpp: implementation of the HttpResponse class.
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

#include "HttpResponse.h"
#include "HttpServer.h"

#include <stdio.h>
#include <string.h>

#include "Osal/Algorithm.h"
#include "Osal/Crypto.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define Check(x) iErrCode = x; if (iErrCode != OK) { return iErrCode; }

#define INITIAL_BUFFER_SIZE 4096
#define CHUNK_SIZE 3172

#define RESPONSE_BUFFER_PRE_PADDING 10  /* Not 64 bit compliant! */
#define RESPONSE_BUFFER_TOTAL_PADDING (RESPONSE_BUFFER_PRE_PADDING + 3)

HttpResponse::HttpResponse() {

    m_iNumRefs = 1;

    m_sStatus = HTTP_200;
    m_rReason = HTTP_REASON_NONE;
    
    m_stLength = 0;
    m_stRealLength = 0;

    m_pszResponseData = NULL;
    m_pszRealResponseData = NULL;
    m_pszMimeType = NULL;
    m_pszRedirectUri = NULL;

    m_iNumCookiesSet = 0;
    m_iNumCookiesDel = 0;
    m_iSetCookieSpace = 0;
    m_iDelCookieSpace = 0;

    m_ppszCookieSetName = NULL;
    m_ppszCookieSetValue = NULL;
    m_ppszCookieSetPath = NULL;
    m_piCookieTTL = NULL;

    m_ppszCookieDelName = NULL;
    m_ppszCookieDelPath = NULL;

    m_pHttpRequest = NULL;
    m_pHttpServer = NULL;

    m_pCachedFile = NULL;

    m_bNoErrorCallback = false;

    m_pPageSource = NULL;

    m_rType = RESPONSE_NONE;

    m_stCookieSpace = 0;
    m_stPageSourceNameLength = 0;
    m_msResponseTime = 0;

    m_stResponseLength = 0;
    m_bNoBuffering = false;
    m_bHeadersSent = false;
    m_bConnectionClosed = false;

    m_iResponseHttpVersion = HTTP11;

    m_ppszCustomLogMessages = NULL;
    m_iNumCustomLogMessages = 0;
    m_iCustomLogMessageSpace = 0;

    m_pszCustomHeaders = NULL;
    m_stCustomHeaderLength = 0;
}

void HttpResponse::SetHttpObjects (HttpServer* pHttpServer, HttpRequest* pHttpRequest) {

    m_pHttpRequest = pHttpRequest;
    Assert (m_pHttpRequest != NULL);

    m_pHttpServer = pHttpServer;
    Assert (m_pHttpServer != NULL);
}

void HttpResponse::Recycle() {

    ClearObject();

    m_sStatus = HTTP_200;
    m_rReason = HTTP_REASON_NONE;
    Time::ZeroTime (&m_tNow);

    m_stPageSourceNameLength = 0;

    m_pPageSource = NULL;

    m_bNoBuffering = false;
    m_bHeadersSent = false;
    m_bNoErrorCallback = false;
    m_bConnectionClosed = false;

    m_stCookieSpace = 0;

    m_iNumCookiesSet = 0;
    m_iNumCookiesDel = 0;
    m_msResponseTime = 0;
    m_stResponseLength = 0;
}

void HttpResponse::ClearObject() {

    m_rType = RESPONSE_NONE;    
    m_stLength = 0;

    if (m_pszMimeType != NULL) {
        OS::HeapFree (m_pszMimeType);
        m_pszMimeType = NULL;
    }

    if (m_pCachedFile != NULL) {
        m_pCachedFile->Release();
        m_pCachedFile = NULL;
    }

    for (unsigned int i = 0; i < m_iNumCustomLogMessages; i ++) {
        OS::HeapFree (m_ppszCustomLogMessages[i]);
    }
    m_iNumCustomLogMessages = 0;

    if (m_pszCustomHeaders != NULL) {
        delete [] m_pszCustomHeaders;
        m_pszCustomHeaders = NULL;
    }

    m_stCustomHeaderLength = 0;
}

HttpResponse::~HttpResponse() {

    if (m_pszRealResponseData != NULL) {
        delete [] m_pszRealResponseData;
    }

    if (m_pszMimeType != NULL) {
        OS::HeapFree (m_pszMimeType);
    }

    if (m_pszRedirectUri != NULL) {
        OS::HeapFree (m_pszRedirectUri);
    }

    unsigned int i;
    for (i = 0; i < m_iNumCookiesSet; i ++) {
        if (m_ppszCookieSetName[i] != NULL) {
            delete [] m_ppszCookieSetName[i];
        }
    }

    if (m_ppszCookieSetName != NULL) {
        delete [] m_ppszCookieSetName;
    }

    if (m_piCookieTTL != NULL) {
        delete [] m_piCookieTTL;
    }

    for (i = 0; i < m_iNumCookiesDel; i ++) {
        if (m_ppszCookieDelName[i] != NULL) {
            delete [] m_ppszCookieDelName[i];
        }
    }

    if (m_ppszCookieDelName != NULL) {
        delete [] m_ppszCookieDelName;
    }

    if (m_pCachedFile != NULL) {
        m_pCachedFile->Release();
    }

    for (i = 0; i < m_iNumCustomLogMessages; i ++) {
        OS::HeapFree (m_ppszCustomLogMessages[i]);
    }

    if (m_ppszCustomLogMessages != NULL) {
        delete [] m_ppszCustomLogMessages;
    }

    Assert (m_pszCustomHeaders == NULL);
}

HttpStatus HttpResponse::GetStatusCode() {
    return m_sStatus;
}

int HttpResponse::SetStatusCode (HttpStatus sStatus) {
    
    if (sStatus > HTTP_505 || 
        sStatus < HTTP_200 ||

        // Use SetRedirect to set this one
        sStatus == HTTP_301) {

        return ERROR_INVALID_ARGUMENT;
    }

    if (m_bHeadersSent) {
        return ERROR_FAILURE;
    }

    InternalSetStatusCode (sStatus);

    return OK;
}

HttpStatusReason HttpResponse::GetStatusCodeReason() {
    return m_rReason;
}

int HttpResponse::SetStatusCodeReason (HttpStatusReason rReason) {

    m_rReason = rReason;
    return OK;
}

int HttpResponse::SetNoBuffering() {

    int iErrCode;

    if (m_bNoBuffering || m_pHttpRequest->GetVersion() < HTTP11 || m_iResponseHttpVersion < HTTP11) {
        return ERROR_FAILURE;
    }

    m_sStatus = HTTP_200;
    m_rType = RESPONSE_BUFFER;

    m_bNoBuffering = true;

    // Check to see if we need to send a chunk
    if (m_stLength >= CHUNK_SIZE) {
        Check (SendChunkFromBuffer());
    }

    return OK;
}

int HttpResponse::Clear() {

    if (m_bHeadersSent) {
        return ERROR_FAILURE;
    }

    ClearObject();

    return OK;
}

int HttpResponse::Flush() {

    int iErrCode;

    if (!m_bNoBuffering || m_pHttpRequest->GetVersion() < HTTP11 || m_iResponseHttpVersion < HTTP11) {
        return ERROR_FAILURE;
    }

    // Write headers if necessary
    if (!m_bHeadersSent) {
        Check (Send());
    }

    // Send remaining chunk
    Check (SendChunkFromBuffer());

    return OK;
}

int HttpResponse::SendChunkFromBuffer() {

    int iErrCode;

    Assert (m_bNoBuffering);

    if (m_stLength == 0) {
        return OK;
    }

    // Write headers if necessary
    if (!m_bHeadersSent) {
        Check (Send());
    }

    Assert (m_pszResponseData != NULL);

    char pszChunkLength [128];
    String::UI64toA (m_stLength, pszChunkLength, 16);
    
    size_t stLen = strlen (pszChunkLength);
    Assert (stLen <= RESPONSE_BUFFER_PRE_PADDING - 2);  // Gotta be, or it's > 0x7fffffff
    
    char* pszBegin = m_pszResponseData - stLen - 2;
    memcpy (pszBegin, pszChunkLength, stLen);
    
    m_pszResponseData [-2] = '\r';
    m_pszResponseData [-1] = '\n';
    m_pszResponseData [m_stLength] = '\r';
    m_pszResponseData [m_stLength + 1] = '\n';

    size_t stSend = m_stLength + stLen + 4, stSent;
    
    Check (m_pSocket->Send (pszBegin, stSend, &stSent));
    Assert (stSend == stSent);

    m_stResponseLength += stSent;

    // No more buffer
    m_stLength = 0;

    return OK;
}

int HttpResponse::SendChunk (const void* pData, size_t stDataLen) {

    int iErrCode;

    Assert (pData != NULL && stDataLen > 0);

    char pszChunkLength [128];
    String::UI64toA (stDataLen, pszChunkLength, 16);

    size_t stLen = strlen (pszChunkLength), stSent;
    Assert (stLen <= RESPONSE_BUFFER_PRE_PADDING - 2 && stLen < 126);   // Gotta be, or it's > 0x7fffffff

    pszChunkLength [stLen] = '\r';
    pszChunkLength [stLen + 1] = '\n';

    Check (m_pSocket->Send (pszChunkLength, stLen + 2, &stSent));
    m_stResponseLength += stSent;

    Check (m_pSocket->Send (pData, stDataLen, &stSent));
    m_stResponseLength += stSent;

    Check (m_pSocket->Send ("\r\n", sizeof ("\r\n") - 1, &stSent));
    m_stResponseLength += stSent;

    return OK;
}

int HttpResponse::FlushChunkFromBuffer() {

    int iErrCode;
    size_t stSent;

    Check (SendChunkFromBuffer());
    Check (m_pSocket->Send ("0\r\n\r\n", sizeof ("0\r\n\r\n") - 1, &stSent));

    m_stResponseLength += stSent;

    return OK;
}

int HttpResponse::WriteText (const char* pszData) {

    return WriteText (pszData, strlen (pszData));
}

int HttpResponse::WriteText (const char* pszData, size_t stStrlen) {

    int iErrCode;

    if (pszData == NULL || stStrlen == 0) {
        return ERROR_FAILURE;
    }

    m_rType = RESPONSE_BUFFER;

    if (m_bNoBuffering && m_stLength + stStrlen >= CHUNK_SIZE){

        Check (SendChunkFromBuffer());

        if (stStrlen > CHUNK_SIZE) {
            return SendChunk (pszData, stStrlen);
        }
    }

    if (m_pszRealResponseData == NULL) {

        m_stRealLength = INITIAL_BUFFER_SIZE;
        
        m_pszRealResponseData = new char [m_stRealLength + RESPONSE_BUFFER_TOTAL_PADDING];
        if (m_pszRealResponseData == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
        m_pszResponseData = m_pszRealResponseData + RESPONSE_BUFFER_PRE_PADDING;

        memcpy (m_pszResponseData, pszData, stStrlen);
        m_pszResponseData[stStrlen] = '\0';

        m_stLength = stStrlen;

    } else {

        size_t stNewLength = m_stLength + stStrlen;
    
        if (m_stRealLength > stNewLength) {
            
            memcpy (m_pszResponseData + m_stLength, pszData, stStrlen);
            m_pszResponseData[stNewLength] = '\0';

        } else {
            
            m_stRealLength = stNewLength * 2;

            char* pszRealResponseData = new char [m_stRealLength + RESPONSE_BUFFER_TOTAL_PADDING];
            if (pszRealResponseData == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }
            char* pszResponseData = pszRealResponseData + RESPONSE_BUFFER_PRE_PADDING;
            
            strcpy (pszResponseData, m_pszResponseData);
            memcpy (pszResponseData + m_stLength, pszData, stStrlen);
            pszResponseData[stNewLength] = '\0';
            
            delete [] m_pszRealResponseData;

            m_pszRealResponseData = pszRealResponseData;
            m_pszResponseData = pszResponseData; 
        }

        m_stLength = stNewLength;
    }

    return OK;
}

int HttpResponse::WriteData (const void* pszData, size_t stNumBytes) {

    int iErrCode;

    if (pszData == NULL || stNumBytes == 0) {
        return ERROR_FAILURE;
    }
        
    m_rType = RESPONSE_BUFFER;
    
    if (m_bNoBuffering && m_stLength + stNumBytes >= CHUNK_SIZE){
        Check (SendChunkFromBuffer());

        if (stNumBytes > CHUNK_SIZE) {
            return SendChunk (pszData, stNumBytes);
        }
    }

    m_stLength += stNumBytes;
    if (m_pszRealResponseData == NULL) {

        m_stRealLength = INITIAL_BUFFER_SIZE;
        m_pszRealResponseData = new char [m_stRealLength + RESPONSE_BUFFER_TOTAL_PADDING];
        if (m_pszRealResponseData == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
        m_pszResponseData = m_pszRealResponseData + RESPONSE_BUFFER_PRE_PADDING;
        memcpy (m_pszResponseData, pszData, stNumBytes);

    } else {
        
        if (m_stRealLength > m_stLength) {
            
            memcpy (&(m_pszResponseData[m_stLength]), pszData, stNumBytes);
            
        } else {
            
            m_stRealLength = m_stLength * 2;
            
            char* pszRealResponseData = new char [m_stRealLength + RESPONSE_BUFFER_TOTAL_PADDING];
            if (pszRealResponseData == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }
            char* pszResponseData = pszRealResponseData + RESPONSE_BUFFER_PRE_PADDING;
            
            memcpy (pszResponseData, m_pszResponseData, m_stLength);
            memcpy (pszResponseData + m_stLength, pszData, stNumBytes);
            
            delete [] m_pszRealResponseData;

            m_pszRealResponseData = pszRealResponseData;
            m_pszResponseData = pszResponseData; 
        }
    }

    return OK;
}

int HttpResponse::WriteText (int iData) {
    
    char pszData [128];
    return WriteText (_itoa (iData, pszData, 10));
}

int HttpResponse::WriteText (unsigned int iData) {

    char pszData [128];
    return WriteText (String::UItoA (iData, pszData, 10));
}

int HttpResponse::WriteText (float fData) {

    char pszData [128];
    sprintf (pszData, "%.3f", fData);
    return WriteText (pszData);
}

int HttpResponse::WriteText (double dData) {

    char pszData [128];
    sprintf (pszData, "%.3f", dData);
    return WriteText (pszData);
}

int HttpResponse::WriteText (const UTCTime& tTime) {

    char pszBuffer[128];
    Time::UTCTimetoA (tTime, pszBuffer, 10);

    return WriteText (pszBuffer);
}

int HttpResponse::WriteText (const Variant& vData) {

    switch (vData.GetType()) {

    case V_STRING:
        return WriteText (vData.GetCharPtr());
    
    case V_INT:
        return WriteText (vData.GetInteger());

    case V_INT64:
        return WriteText (vData.GetInteger64());
    
    case V_FLOAT:
        return WriteText (vData.GetFloat());
    
    case V_TIME:
        {
            // Get date
            char pszDateString [OS::MaxDateLength];
            Time::GetDateString (vData.GetUTCTime(), pszDateString);
            return WriteText (pszDateString);
        }
    
    default:
        return ERROR_INVALID_ARGUMENT;
    }
}

int HttpResponse::WriteText (int64 iData) {

    char pszData [128];
    return WriteText (String::I64toA (iData, pszData, 10));
}

int HttpResponse::WriteText (uint64 iData) {

    char pszData [128];
    return WriteText (String::UI64toA (iData, pszData, 10));
}

int HttpResponse::WriteTextFile (ICachedFile* pCachedFile) {

    return WriteText ((const char*) pCachedFile->GetData(), pCachedFile->GetSize());
}

int HttpResponse::WriteDataFile (ICachedFile* pCachedFile) {

    return WriteData (pCachedFile->GetData(), pCachedFile->GetSize());
}

int HttpResponse::SetFile (ICachedFile* pCachedFile) {

    if (pCachedFile == NULL || m_bNoBuffering) {
        return ERROR_INVALID_ARGUMENT;
    }

    m_rType = RESPONSE_FILE;
    m_stLength = 0;

    if (m_pCachedFile != NULL) {
        m_pCachedFile->Release();
    }
    m_pCachedFile = pCachedFile;
    m_pCachedFile->AddRef();

    return OK;
}

int HttpResponse::SetMimeType (const char* pszMimeType) {

    if (pszMimeType == NULL) {
        return ERROR_FAILURE;
    }

    if (m_pszMimeType != NULL) {
        OS::HeapFree (m_pszMimeType);
    }
    m_pszMimeType = String::StrDup (pszMimeType);
    if (m_pszMimeType == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    return OK;
}

int HttpResponse::SetRedirect (const char* pszUri) {

    if (pszUri == NULL || m_bNoBuffering) {
        return ERROR_FAILURE;
    }

    m_rType = RESPONSE_REDIRECT;
    m_stLength = 0;

    if (m_pszRedirectUri != NULL) {
        OS::HeapFree (m_pszRedirectUri);
    }
    m_pszRedirectUri = String::StrDup (pszUri);
    if (m_pszRedirectUri == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    InternalSetStatusCode (HTTP_301);

    return OK;
}

int HttpResponse::CreateCookie (const char* pszCookieName, const char* pszCookieValue, Seconds iTTLinSeconds, 
                                const char* pszCookiePath) {

    if (pszCookieName == NULL || pszCookieValue == NULL || (m_bHeadersSent)) {
        return ERROR_FAILURE;
    }

    if (m_iNumCookiesSet == m_iSetCookieSpace) {

        if (m_iSetCookieSpace == 0) {

            m_iSetCookieSpace = 4;
            m_ppszCookieSetName = new char* [m_iSetCookieSpace * 3];
            m_ppszCookieSetValue = m_ppszCookieSetName + m_iSetCookieSpace;
            m_ppszCookieSetPath = m_ppszCookieSetValue + m_iSetCookieSpace;
            m_piCookieTTL  = new Seconds [m_iSetCookieSpace];
            
            if (m_ppszCookieSetName == NULL || m_piCookieTTL == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }
            
        } else {

            int iNewLength = m_iSetCookieSpace * 2;
            char** ppszCookieSetName = new char* [iNewLength * 3];
            char** ppszCookieSetValue = ppszCookieSetName + iNewLength;
            char** ppszCookieSetPath = ppszCookieSetValue + iNewLength;
            Seconds* piCookieTTL  = new Seconds [iNewLength];

            if (ppszCookieSetName == NULL || piCookieTTL == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }

            memcpy (ppszCookieSetName, m_ppszCookieSetName, m_iSetCookieSpace * sizeof (char*));
            memcpy (ppszCookieSetValue, m_ppszCookieSetValue, m_iSetCookieSpace * sizeof (char*));
            memcpy (ppszCookieSetPath, m_ppszCookieSetPath, m_iSetCookieSpace * sizeof (char*));
            memcpy (piCookieTTL, m_piCookieTTL, m_iSetCookieSpace * sizeof (Seconds));

            delete [] m_ppszCookieSetName;
            delete [] m_piCookieTTL;

            m_ppszCookieSetName = ppszCookieSetName;
            m_ppszCookieSetValue = ppszCookieSetValue;
            m_ppszCookieSetPath = ppszCookieSetPath;
            m_piCookieTTL = piCookieTTL;

            m_iSetCookieSpace = iNewLength;
        }
    }

    size_t stName = strlen (pszCookieName);
    size_t stValue = strlen (pszCookieValue);
    size_t stPath = pszCookiePath != NULL ? strlen (pszCookiePath) : 0;

    Assert (m_stPageSourceNameLength > 0);
    m_stCookieSpace += stName + stValue + stPath + 70 + m_stPageSourceNameLength;

    m_ppszCookieSetName[m_iNumCookiesSet] = new char [stName + stValue + 3];
    if (m_ppszCookieSetName[m_iNumCookiesSet] == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    memcpy (m_ppszCookieSetName[m_iNumCookiesSet], pszCookieName, stName + 1);

    m_ppszCookieSetValue[m_iNumCookiesSet] = m_ppszCookieSetName[m_iNumCookiesSet] + stName + 1;
    memcpy (m_ppszCookieSetValue[m_iNumCookiesSet], pszCookieValue, stValue + 1);

    m_piCookieTTL[m_iNumCookiesSet] = iTTLinSeconds;

    if (pszCookiePath == NULL) {
        m_ppszCookieSetPath[m_iNumCookiesSet] = NULL;
    } else {
        m_ppszCookieSetPath[m_iNumCookiesSet] = m_ppszCookieSetValue[m_iNumCookiesSet] + stValue + 1;
        memcpy (m_ppszCookieSetPath[m_iNumCookiesSet], pszCookiePath, stPath);
    }

    m_iNumCookiesSet ++;

    return OK;
}

int HttpResponse::DeleteCookie (const char* pszCookieName, const char* pszCookiePath) {

    if (pszCookieName == NULL || m_bHeadersSent) {
        return ERROR_FAILURE;
    }

    if (m_iNumCookiesDel == m_iDelCookieSpace) {

        if (m_iDelCookieSpace == 0) {
            
            m_iDelCookieSpace = 4;
            m_ppszCookieDelName = new char* [m_iDelCookieSpace * 2];
            if (m_ppszCookieDelName == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }

            m_ppszCookieDelPath = m_ppszCookieDelName + m_iDelCookieSpace;

        } else {

            unsigned int iNewLength = m_iDelCookieSpace * 2;
            char** ppszCookieDelName = new char* [iNewLength * 2];
            if (ppszCookieDelName == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }

            char** ppszCookieDelPath = ppszCookieDelName + iNewLength;

            memcpy (ppszCookieDelName, m_ppszCookieDelName, m_iDelCookieSpace * sizeof (char*));
            memcpy (ppszCookieDelPath, m_ppszCookieDelPath, m_iDelCookieSpace * sizeof (char*));

            delete [] m_ppszCookieDelName;

            m_ppszCookieDelName = ppszCookieDelName;
            m_ppszCookieDelPath = ppszCookieDelPath;

            m_iDelCookieSpace = iNewLength;
        }
    }

    size_t stName = strlen (pszCookieName);
    size_t stPath = pszCookiePath != NULL ? strlen (pszCookiePath) : 0;

    Assert (m_stPageSourceNameLength > 0);
    m_stCookieSpace += stName + 70 + stPath + m_stPageSourceNameLength;

    m_ppszCookieDelName[m_iNumCookiesDel] = new char [stName + stPath + 2];
    if (m_ppszCookieDelName[m_iNumCookiesDel] == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    memcpy (m_ppszCookieDelName[m_iNumCookiesDel], pszCookieName, stName + 1);

    if (pszCookiePath == NULL) {
        m_ppszCookieDelPath[m_iNumCookiesDel] = NULL;
    } else {
        m_ppszCookieDelPath[m_iNumCookiesDel] = m_ppszCookieDelName[m_iNumCookiesDel] + stName + 1;
        memcpy (m_ppszCookieDelPath[m_iNumCookiesDel], pszCookiePath, stPath + 1);
    }

    m_iNumCookiesDel ++;

    return OK;
}

///////////////////
// Response code //
///////////////////

void HttpResponse::InternalSetStatusCode (HttpStatus sStatus) {
    
    m_sStatus = sStatus;

    switch (sStatus) {

    case HTTP_200:

        m_rType = RESPONSE_BUFFER;
        break;

    case HTTP_301:
    
        m_stLength = 0;
        m_rType = RESPONSE_REDIRECT;
        break;
    
    default:

        m_stLength = 0;
        m_rType = RESPONSE_ERROR;
        break;
    }
}

void HttpResponse::SetStatusCodeOnError (HttpStatus sStatus) {

    ClearObject();
    InternalSetStatusCode (sStatus);
}

void HttpResponse::SetNoPageSource() {

    m_pPageSource = NULL;
}

void HttpResponse::SetNoErrorCallback() {

    m_bNoErrorCallback = true;
}

void HttpResponse::SetResponseHttpVersion (HttpVersion iVersion) {
    m_iResponseHttpVersion = iVersion;
}

// Base response function
int HttpResponse::Respond() {

    m_pPageSource = m_pHttpRequest->GetPageSource();
    m_iMethod = m_pHttpRequest->GetMethod();

    return RespondPrivate();
}

int HttpResponse::RespondPrivate() {

    int iErrCode;

    Timer tTimer;
    Time::StartTimer (&tTimer);

    // Get time
    Time::GetTime (&m_tNow);

    if (m_pPageSource != NULL) {

        const char* pszPageSourceName = m_pPageSource->GetName();
        m_stPageSourceNameLength = strlen (pszPageSourceName);

        // Check for name without slash and no forms
        const char* pszUri = m_pHttpRequest->GetUri();
        const char* pszName = pszUri + 1;

        if (_stricmp (pszName, pszPageSourceName) == 0) {

            const char* pszForms;
            size_t stFormLen;

            if (m_pHttpRequest->ParsedUriForms()) {
                pszForms = m_pHttpRequest->GetParsedUriForms();
                stFormLen = strlen (pszForms) + 1;
            } else {
                pszForms = NULL;
                stFormLen = 0;
            }

            char* pszRedirect = new char [m_stPageSourceNameLength + stFormLen + 3];
            if (pszRedirect == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                return iErrCode;
            }
            pszRedirect[0] = '/';

            strcpy (pszRedirect + 1, pszName);
            strcat (pszRedirect, "/");

            if (pszForms != NULL) {
                strcat (pszRedirect, "?");
                strcat (pszRedirect, pszForms);
            }

            SetRedirect (pszRedirect);

            delete [] pszRedirect;
        }
        
        // Check for restarting pagesource
        else if (m_pPageSource->IsRestarting()) {
            
            // 503: page source is restarting
            SetNoPageSource();
            InternalSetStatusCode (HTTP_503);
        }
        
        // Check for hosed pagesource
        else if (!m_pPageSource->IsWorking()) {
            
            // 409: the page source is screwed up
            SetNoPageSource();
            InternalSetStatusCode (HTTP_409);
        }
        
        // Does URI contain the following illegal string? ..
        else if (strstr (pszUri, "..") != NULL) {
            InternalSetStatusCode (HTTP_404);
        }

        // Is IP authorized?
        else if (!m_pPageSource->IsIPAddressAllowedAccess (m_pHttpRequest->GetClientIP())) {
            
            // 403 - forbidden
            InternalSetStatusCode (HTTP_403);
            SetStatusCodeReason (HTTP_REASON_IPADDRESS_BLOCKED);
        }

        // Is user agent allowed?
        else if (!m_pPageSource->IsUserAgentAllowedAccess (m_pHttpRequest->GetBrowserName())) {
            
            // 403 - forbidden
            InternalSetStatusCode (HTTP_403);
            SetStatusCodeReason (HTTP_REASON_USER_AGENT_BLOCKED);
        }
        
        // Authentication?
        else {
            
            bool bAuthenticated = false;

            Assert (!m_bHeadersSent);

            HttpAuthenticationType atAuth = m_pPageSource->GetAuthenticationType();
            switch (atAuth) {

            case AUTH_BASIC:
                iErrCode = m_pPageSource->OnBasicAuthenticate (m_pHttpRequest, &bAuthenticated);
                break;

            case AUTH_DIGEST:

                // Check the nonce before asking the page source to authenticate
                bool bStale;
                iErrCode = CheckDigestAuthenticationNonce (&bStale);
                if (iErrCode == OK) {

                    if (bStale) {
                        // This codepath leaves bAuthenticated as false, so we return 401 with a hint
                        m_rReason = HTTP_REASON_STALE_NONCE;
                    } else {
                        iErrCode = m_pPageSource->OnDigestAuthenticate (m_pHttpRequest, &bAuthenticated);
                    }
                }
                break;

            case AUTH_NONE:
                iErrCode = OK;
                bAuthenticated = true;
                break;

            default:
                iErrCode = ERROR_FAILURE;
                Assert (false);
                break;
            }

            if (iErrCode != OK) {
                SetNoErrorCallback();
                InternalSetStatusCode (HTTP_400);
            }
            
            else if (!bAuthenticated) {
                SetNoErrorCallback();
                InternalSetStatusCode (HTTP_401);
            }
        }
    }

    // Go ahead and send the response
    iErrCode = SendResponse();

    m_msResponseTime = Time::GetTimerCount (tTimer);

    return iErrCode;
}

int HttpResponse::SendResponse() {

    int iErrCode;

    // TRACE gets special treatment
    if (m_iMethod == TRACE) {
        return RespondToTrace();
    }

    if (m_sStatus == HTTP_200) {

        // Process method
        Check (ProcessMethod());
    }

    // Handle GET filtering
    if (GetStatusCode() == HTTP_200 && m_iMethod == GET && m_rType == RESPONSE_FILE) {

        if (!m_pHttpRequest->IsFileNameCanonical()) {
            InternalSetStatusCode (HTTP_404);
        }

        else if (m_pPageSource != NULL && !m_pPageSource->IsGetAllowed (m_pHttpRequest)) {

            // 403 - forbidden
            InternalSetStatusCode (HTTP_403);
            SetStatusCodeReason (HTTP_REASON_GET_REFERER_BLOCKED);
        }
    }

    // Handle page source error overrides
    Check (ProcessErrors());

    // Check for response handling inconsistencies
    Check (ProcessInconsistencies());

    // Finish up
    Check (Send());

    if (m_bNoBuffering) {
        Check (SendChunkFromBuffer());
    }

    return OK;
}

int HttpResponse::ProcessErrors() {

    // Short circuit for null page sources
    if (m_pPageSource == NULL) {
        Assert (m_sStatus != HTTP_200 && m_sStatus != HTTP_301);
        return OK;
    }

    if (m_sStatus != HTTP_200 && m_sStatus != HTTP_301) {

        if (m_pPageSource->OverrideError (m_sStatus)) {

            // Error override?
            if (m_pPageSource->OnError (m_pHttpRequest, this) != OK) {
                
                // Internal error
                if (!m_bHeadersSent) {
                    SetStatusCodeOnError (HTTP_500);
                }
            }

        } else {

            // Custom error file?
            Assert (m_pCachedFile == NULL);
            m_pCachedFile = m_pPageSource->GetErrorFile (m_sStatus);

            if (m_pCachedFile != NULL) {
                m_rType = RESPONSE_FILE;
            }
        }
    }

    return OK;
}

int HttpResponse::ProcessInconsistencies() {

    // Check for HTTP_200 and no file or data
    if (m_sStatus == HTTP_200 &&
        m_pCachedFile == NULL &&
        m_stLength == 0 &&
        !m_bHeadersSent
        ) {

        SetStatusCodeOnError (HTTP_500);
    }

    // Check for HTTP_301 and no redirect
    if (m_sStatus == HTTP_301 &&
        m_pszRedirectUri == NULL
        ) {

        SetStatusCodeOnError (HTTP_500);
    }

    return OK;
}

int HttpResponse::Send() {

    int iErrCode;

    if (m_bHeadersSent) {
        Assert (m_bNoBuffering);
        return FlushChunkFromBuffer();
    }
    
    char pszGMTDateString [OS::MaxGMTDateLength];
    iErrCode = Time::GetGMTDateString (m_tNow, pszGMTDateString);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    unsigned int i;

    const char* pszAuthRealm, * pszAuthDomain;
    if (m_pPageSource != NULL) {
        pszAuthRealm = m_pPageSource->GetAuthenticationRealm (m_pHttpRequest);
        pszAuthDomain = m_pPageSource->GetName();
    } else {
        pszAuthRealm = NULL;
        pszAuthDomain = NULL;
    }

    char pszInt [32];
    char* pszBuffer = (char*) StackAlloc (
        4096 + m_stCookieSpace + m_stCustomHeaderLength + 
        String::StrLen (pszAuthRealm) + String::StrLen (pszAuthDomain));

    ///////////////////////////
    // Send response headers //
    ///////////////////////////

    // Protocol and status
    strcpy (pszBuffer, "HTTP/1.1 ");
    SetResponseHttpVersion (HTTP11);

    strcat (pszBuffer, HttpStatusText[m_sStatus]);

    // Date, server name, connection
    strcat (pszBuffer, "\r\nServer: Alajar/1.80\r\nDate: ");
    strcat (pszBuffer, pszGMTDateString);

    //////////////////////////////////
    // Send status specific headers //
    //////////////////////////////////

    switch (m_sStatus) {

    case HTTP_301:
        
        Assert (m_pPageSource != NULL && m_pszRedirectUri != NULL);

        // Location
        if (m_pHttpServer->IsDefaultPageSource (m_pPageSource)) {
            strcat (pszBuffer, "\r\nLocation: /");
            strcat (pszBuffer, m_pPageSource->GetName());
        } else {
            strcat (pszBuffer, "\r\nLocation: ");
        }

        strcat (pszBuffer, m_pszRedirectUri);
        break;

    case HTTP_401:

        Assert (m_pPageSource != NULL);
        switch (m_pPageSource->GetAuthenticationType()) {

        case AUTH_BASIC:

            // Send basic authentication challenge
            strcat (pszBuffer, "\r\nWWW-Authenticate: basic realm=\"");
            strcat (pszBuffer, pszAuthRealm);
            strcat (pszBuffer, "\"");
            break;

        case AUTH_DIGEST:

            // Create a new nonce
            char pszNonce [NONCE_SIZE];
            iErrCode = CreateDigestAuthenticationNonce (pszNonce);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            // Send digest authentication challenge
            strcat (pszBuffer, "\r\nWWW-Authenticate: Digest realm=\"");
            strcat (pszBuffer, pszAuthRealm);
            strcat (pszBuffer, "\", domain=\"/");
            strcat (pszBuffer, pszAuthDomain);
            strcat (pszBuffer, "\", nonce=\"");
            strcat (pszBuffer, pszNonce);
            strcat (pszBuffer, "\", stale=");
            
            if (m_rReason == HTTP_REASON_STALE_NONCE) {
                strcat (pszBuffer, "true");
            } else {
                strcat (pszBuffer, "false");
            }

            strcat (pszBuffer, ", algorithm=MD5, qop=\"auth\"");
            break;

        default:
            Assert (false);
            break;
        }
        break;
    }

    // Last modified, Expires, Cache-Control
    if (m_rType != RESPONSE_REDIRECT && 
        m_rType != RESPONSE_ERROR &&
        m_sStatus == HTTP_200) {

        if (m_rType == RESPONSE_FILE) {

            Assert (m_pCachedFile != NULL);

            UTCTime tTime;
            char pszTime [OS::MaxGMTDateLength];

            m_pCachedFile->GetLastModifiedTime (&tTime);

            iErrCode = Time::GetGMTDateString (tTime, pszTime);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            // File's last modified time
            strcat (pszBuffer, "\r\nLast-Modified: ");
            strcat (pszBuffer, pszTime);

            // Expires in a year
            strcat (pszBuffer, "\r\nExpires: ");

            Time::AddSeconds (m_tNow, 60 * 60 * 24 * 365, &tTime);
            iErrCode = Time::GetGMTDateString (tTime, pszTime);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            strcat (pszBuffer, pszTime);

            // Cache-control is public
            strcat (pszBuffer, "\r\nCache-Control: public");

        } else {

            Assert (m_rType == RESPONSE_BUFFER);

            // Last modified this second
            strcat (pszBuffer, "\r\nLast-Modified: ");
            strcat (pszBuffer, pszGMTDateString);

            // Expires this second
            strcat (pszBuffer, "\r\nExpires: ");
            strcat (pszBuffer, pszGMTDateString);

            // Cache-control is private
            //
            // It might seem more appropriate to use no-cache or no-store, but
            // they have the effect of preventing the back button in IE6 from working
            strcat (pszBuffer, "\r\nCache-Control: private");
        }
    }

    if (m_rType != RESPONSE_REDIRECT) {

        strcat (pszBuffer, "\r\nContent-Type: ");

        // Custom mime type has precedence
        if (m_pszMimeType != NULL) {

            strcat (pszBuffer, m_pszMimeType);

        } else {

            // Next comes response file's type
            if (m_rType == RESPONSE_FILE) {

                Assert (m_pCachedFile != NULL);

                // File's mime type
                strcat (pszBuffer, m_pCachedFile->GetMimeType());

            } else {

                // Default to text/html
                strcat (pszBuffer, "text/html");        
            }
        }

        // Content length
        if (m_bNoBuffering && m_rType == RESPONSE_BUFFER) {

            // Chunked encoding
            strcat (pszBuffer, "\r\nTransfer-Encoding: chunked");

        } else {

            // Content length
            strcat (pszBuffer, "\r\nContent-Length: ");

            if (m_rType == RESPONSE_FILE) {

                Assert (m_pCachedFile != NULL);

                // Send length of cached file
                strcat (pszBuffer, String::UI64toA (m_pCachedFile->GetSize(), pszInt, 10));
            }
            else if (m_rType == RESPONSE_BUFFER) {

                // Send length of data buffer
                strcat (pszBuffer, String::UI64toA (m_stLength, pszInt, 10));
            }
            else if (m_rType == RESPONSE_ERROR) {

                // Send length of error string
                strcat (pszBuffer, String::UI64toA (HttpStatusErrorTextLength[m_sStatus], pszInt, 10));
            }
        }

        // Accept ranges is none
        strcat (pszBuffer, "\r\nAccept-Ranges: none");
    }

    // Connection
    // TODO - blocks reusing connections for HTTP 1.1
    m_bConnectionClosed = true || !m_pHttpRequest->GetKeepAlive();
    if (m_bConnectionClosed && m_iResponseHttpVersion >= HTTP11) {
        strcat (pszBuffer, "\r\nConnection: close");
    }
    /*else {
        strcat (pszBuffer, "\r\nConnection: Keep-Alive");
    }*/

    // Send custom headers
    if (m_pszCustomHeaders != NULL) {
        strcat (pszBuffer, m_pszCustomHeaders);
    }

    //////////////////
    // Send cookies //
    //////////////////

    if (m_iNumCookiesSet > 0 || m_iNumCookiesDel > 0) {

        Assert (m_pPageSource != NULL);

        char pszCookieExpirationDate [OS::MaxCookieDateLength];

        // Set cookies
        for (i = 0; i < m_iNumCookiesSet; i ++) {
            
            UTCTime tTimePlus;
            Time::AddSeconds (m_tNow, m_piCookieTTL[i], &tTimePlus);
            
            iErrCode = Time::GetCookieDateString (tTimePlus, pszCookieExpirationDate);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            strcat (pszBuffer, "\r\nSet-Cookie: ");
            strcat (pszBuffer, m_ppszCookieSetName[i]);
            strcat (pszBuffer, "=");
            strcat (pszBuffer, m_ppszCookieSetValue[i]);
            strcat (pszBuffer, "; httponly; expires=");
            strcat (pszBuffer, pszCookieExpirationDate);
            strcat (pszBuffer, "; path=");
            
            if (!m_pHttpServer->IsDefaultPageSource (m_pPageSource)) {
                strcat (pszBuffer, "/");
                strcat (pszBuffer, m_pPageSource->GetName());
            }
            
            if (m_ppszCookieSetPath[i] != NULL) {
                strcat (pszBuffer, m_ppszCookieSetPath[i]);
            } else {
                strcat (pszBuffer, "/");
            }
        }
        
        // Delete cookies
        for (i = 0; i < m_iNumCookiesDel; i ++) {
            
            UTCTime tTimePlus;
            Time::SubtractSeconds (m_tNow, 24 * 60 * 60, &tTimePlus);
            
            iErrCode = Time::GetCookieDateString (tTimePlus, pszCookieExpirationDate);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            strcat (pszBuffer, "\r\nSet-Cookie: ");
            strcat (pszBuffer, m_ppszCookieDelName[i]);
            strcat (pszBuffer, "=null");
            strcat (pszBuffer, "; expires=");
            strcat (pszBuffer, pszCookieExpirationDate);
            strcat (pszBuffer, "; path=");
            
            if (!m_pHttpServer->IsDefaultPageSource (m_pPageSource)) {
                strcat (pszBuffer, "/");
                strcat (pszBuffer, m_pPageSource->GetName());
            }
            
            if (m_ppszCookieDelPath[i] != NULL) {
                strcat (pszBuffer, m_ppszCookieDelPath[i]);
            } else {
                strcat (pszBuffer, "/");
            }
        }
    }

    // We've now officially sent headers
    m_bHeadersSent = true;

    ///////////////
    // Send data //
    ///////////////

    // Header end marker
    strcat (pszBuffer, "\r\n\r\n");

    // Send
    size_t stSend = strlen (pszBuffer), stSent;
    Check (m_pSocket->Send (pszBuffer, stSend, &stSent));

    m_stResponseLength += stSent;

    Assert ((m_bNoBuffering && m_rType == RESPONSE_BUFFER) || !m_bNoBuffering);

    // Send data
    if (m_pHttpRequest->GetMethod() != HEAD) {

        if (!m_bNoBuffering) {

            switch (m_rType) {
            
            case RESPONSE_FILE:

                Assert (m_pCachedFile != NULL);

                // Send data from file
                Check (m_pSocket->Send (m_pCachedFile->GetData(), m_pCachedFile->GetSize(), &stSent));

                m_stResponseLength += stSent;
                break;
            
            case RESPONSE_BUFFER:
                
                // Send data from response buffer
                Check (m_pSocket->Send (m_pszResponseData, m_stLength, &stSent));

                m_stResponseLength += stSent;
                break;

            case RESPONSE_ERROR:

                Assert (m_sStatus > HTTP_200 && m_sStatus < UNSUPPORTED_HTTP_STATUS);

                // Send default error text
                Check (m_pSocket->Send (HttpStatusErrorText[m_sStatus], HttpStatusErrorTextLength[m_sStatus], &stSent));

                m_stResponseLength += stSent;
                break;

            case RESPONSE_REDIRECT:

                // No data to send
                break;

            default:

                // Huh?
                Assert (false);
                return ERROR_FAILURE;
            }
        }
    }

    return OK;
}

int HttpResponse::CreateDigestAuthenticationNonce (char pszNonce [NONCE_SIZE]) {

    int iErrCode;

    const size_t cbNonceSize = sizeof (int64) + MD5_HASH_SIZE;
    char pbNonce [cbNonceSize];

    // The first 8 bytes are a timestamp (seconds since 1970)
    int64 utNow = Time::GetUnixTime (m_tNow);
    memcpy (pbNonce, &utNow, sizeof (int64));  // No guarantee that buffer is properly aligned...

    // The rest of the bytes are the inner MD5 hash
    iErrCode = CreateDigestAuthenticationInnerNonce (utNow, pbNonce + sizeof (int64));
    if (iErrCode != OK)
        return iErrCode;

    // Convert the whole thing to base64
    Assert (Algorithm::GetEncodeBase64Size (cbNonceSize) <= NONCE_SIZE);
    return Algorithm::EncodeBase64 (pbNonce, cbNonceSize, pszNonce, NONCE_SIZE);
}

int HttpResponse::CreateDigestAuthenticationInnerNonce (int64 utTime, char pbInnerNonce [MD5_HASH_SIZE]) {

    int iErrCode;

    Crypto::HashMD5 hash;

    // Add time to the hash
    iErrCode = hash.HashData (&utTime, sizeof (utTime));
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Add user agent to the hash
    const char* pszUserAgent = m_pHttpRequest->GetBrowserName();
    if (pszUserAgent != NULL) {
        iErrCode = hash.HashData (pszUserAgent, strlen (pszUserAgent));
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    // We could add the IP address, but that would probably force re-authentication
    // when corporate firewalls get in the way

    // Add instance-global server-specific data
    // If the server restarts, it will get a new unique identifier.
    // This will cause existing nonces to be declared stale, but clients
    // will not have to reauthenticate
    const Uuid& uuidUnique = m_pHttpServer->GetUniqueIdentifier();
    iErrCode = hash.HashData (&uuidUnique, sizeof (uuidUnique));
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return hash.GetHash (pbInnerNonce, MD5_HASH_SIZE);
}

int HttpResponse::CheckDigestAuthenticationNonce (bool* pbStale) {

    int iErrCode;

    // If the nonce is not present, we just ignore this check
    // We'll send the right 401 challenge as a result of authenticating and failing
    const char* pszNonce = m_pHttpRequest->GetAuthenticationNonce();
    if (pszNonce == NULL) {
        *pbStale = false;
        return OK;
    }

    char pbNonce [sizeof (int64) + MD5_HASH_SIZE];

    *pbStale = true;

    // Decode the nonce to bytes
    size_t cbDecoded;
    iErrCode = Algorithm::DecodeBase64 (pszNonce, pbNonce, sizeof (pbNonce), &cbDecoded);
    if (iErrCode != OK)
        return iErrCode;

    // Make sure we got the size we were expecting
    if (cbDecoded != sizeof (pbNonce))
        return OK;

    // Get the time embedded in the nonce
    int64 utTime;
    memcpy (&utTime, pbNonce, sizeof (int64));

    // See if the time is within range
    Seconds iNonceLifetime = m_pPageSource->GetDigestAuthenticationNonceLifetime();

    UTCTime utcTime;
    Time::FromUnixTime (utTime, &utcTime);

    if (Time::GetSecondDifference (m_tNow, utcTime) > iNonceLifetime)
        return OK;  // Stale nonce
    
    // Compute the rest of the nonce and check it for accuracy
    char pbCheckNonce [MD5_HASH_SIZE];
    iErrCode = CreateDigestAuthenticationInnerNonce (utTime, pbCheckNonce);
    if (iErrCode != OK)
        return iErrCode;

    // This is usually caused by a stale nonce (e.g., a process restart)
    if (memcmp (pbNonce + sizeof (int64), pbCheckNonce, MD5_HASH_SIZE) != 0)
        return OK;

    *pbStale = false;

    return OK;
}

int HttpResponse::RespondToTrace() {

    int iErrCode;
    size_t stHeaderLength = m_pHttpRequest->GetHeaderLength(), stSent;

    char pszInt [40];
    char pszBuffer [160] = "200 OK\r\nConnection: close\r\nContent-Type: message/http\r\nContent-Length: ";

    strcat (pszBuffer, String::UI64toA (stHeaderLength, pszInt, 10));
    strcat (pszBuffer, "\r\n\r\n");

    Check (m_pSocket->Send (pszBuffer, strlen (pszBuffer), &stSent));
    m_stResponseLength += stSent;

    Check (m_pSocket->Send (m_pHttpRequest->GetHeaders(), stHeaderLength, &stSent))
    m_stResponseLength += stSent;

    return OK;
}

int HttpResponse::ProcessMethod() {

    switch (m_iMethod) {

    case GET:
    case HEAD:

        return ProcessGet();

    case POST:

        return ProcessPost();
        break;

    case UNSUPPORTED_HTTP_METHOD:

        return OK;

    default:

        Assert (false);
        return ERROR_FAILURE;
    }
}

int HttpResponse::ProcessGet() {

    // Check for null page source
    if (m_pPageSource == NULL) {
        return OK;
    }

    // Check for cached already
    if (m_pHttpRequest->IsCached() && m_pHttpRequest->GetMethod() != HEAD) {

        SetNoErrorCallback();
        InternalSetStatusCode (HTTP_304);
        return OK;
    }

    if (m_pPageSource->OverrideGet()) {

        // Call the page source
        if (m_pPageSource->OnGet (m_pHttpRequest, this) != OK) {
            
            if (!m_bHeadersSent) {
                SetStatusCodeOnError (HTTP_500);
            }
            return OK;
        }

        // Return if the page source did something meaningful
        // Otherwise fall through to regular get
        if (m_rType != RESPONSE_NONE) {
            return OK;
        }
    }

    // Handle request like a traditional server
    return ProcessGetFile (m_pHttpRequest->GetFileName());
}


int HttpResponse::ProcessGetFile (const char* pszFileName) {

    IFileCache* pFileCache;

    switch (File::GetFileType (pszFileName)) {
        
    case FILETYPE_DIRECTORY:

        // Return directory get
        return ProcessGetDirectory (m_pHttpRequest->GetFileName());

    case FILETYPE_FILE:

        // A file GET has been requested
        pFileCache = m_pHttpServer->GetFileCache();
        Assert (pFileCache != NULL);

        m_pCachedFile = pFileCache->GetFile (pszFileName);
        
        if (m_pCachedFile == NULL) {
            InternalSetStatusCode (HTTP_404);
        } else {
            InternalSetStatusCode (HTTP_200);
            m_rType = RESPONSE_FILE;
        }
        break;

    case FILETYPE_ERROR:
    default:

        InternalSetStatusCode (HTTP_404);
        break;
    }

    return OK;
}


int HttpResponse::ProcessGetDirectory (const char* pszDirName) {

    int iErrCode = OK;
    InternalSetStatusCode (HTTP_200);

    const char* pszDefaultFile = m_pPageSource->GetDefaultFile();

    // Look for default file
    if (pszDefaultFile != NULL) {

        // Build path to default file
        char pszFileName [OS::MaxFileNameLength];

        strcpy (pszFileName, pszDirName);
        if (pszDirName[strlen (pszDirName) - 1] != '/') {
            strcat (pszFileName, "/");
        }
        strcat (pszFileName, pszDefaultFile);
        
        if (File::GetFileType (pszFileName) == FILETYPE_FILE) {
            return ProcessGetFile (pszFileName);
        }
    }
    
    // Maybe we can browse the directory
    if (!m_pPageSource->AllowDirectoryBrowsing()) {
        InternalSetStatusCode (HTTP_403);
    } else {
            
        // Create directory index and send it
        TempFile tfFile;
        if (BuildDirectoryIndex (pszDirName, &tfFile) != OK) {
            
            SetStatusCodeOnError (HTTP_500);

        } else {
        
            // Close the temp file
            tfFile.Close();

            // Open it as a cached file
            CachedFile* pCachedFile = CachedFile::CreateInstance();
            if (pCachedFile == NULL) {

                SetStatusCodeOnError (HTTP_500);

            } else {

                iErrCode = pCachedFile->Open (tfFile.GetName());
                if (iErrCode != OK) {
                    
                    // 404
                    pCachedFile->Release();
                    InternalSetStatusCode (HTTP_404);
                    
                } else {
                    
                    // Set the mime type to the right thing
                    iErrCode = pCachedFile->SetMimeType ("text/html");
                    if (iErrCode != OK) {
                        pCachedFile->Release();
                        SetStatusCodeOnError (HTTP_500);
                    }
                    
                    m_pCachedFile = pCachedFile;
                    m_rType = RESPONSE_FILE;
                }
            }
        }
    }

    return OK;
}


int HttpResponse::BuildDirectoryIndex (const char* pszDirName, TempFile* ptfTempFile) {

    char pszTempBuf [128];

    // Build directory search path string
    char pszDirSearchName [OS::MaxFileNameLength];

    strcpy (pszDirSearchName, pszDirName);
    if (pszDirSearchName[strlen (pszDirSearchName) - 1] != '/') {
        strcat (pszDirSearchName, "/");
    }
    strcat (pszDirSearchName, "*.*");

    unsigned int i, iNumFiles;
    const char** ppszFileName;

    FileEnumerator fEnum;

    int iErrCode = File::EnumerateFiles (pszDirSearchName, &fEnum);
    if (iErrCode != OK) {
        return iErrCode;
    }

    ppszFileName = fEnum.GetFileNames();
    iNumFiles = fEnum.GetNumFiles();

    // Open tempfile
    iErrCode = ptfTempFile->Open();
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    /////////////////////////////
    // Write index to tempfile //
    /////////////////////////////

    char* pszFixedUri;
    const char* pszSentUri = m_pHttpRequest->GetUri();

    size_t stUriLen = strlen (pszSentUri);

    if (pszSentUri[stUriLen - 1] == '/') {
        pszFixedUri = (char*) pszSentUri;
    } else {

        pszFixedUri = (char*) StackAlloc (stUriLen + 2);
        
        memcpy (pszFixedUri, pszSentUri, stUriLen);
        pszFixedUri[stUriLen] = '/';
        pszFixedUri[stUriLen + 1] = '\0';
    }

    ptfTempFile->Write ("<html><head><title>Directory index of ");
    ptfTempFile->Write (pszFixedUri);
    ptfTempFile->Write ("</title></head><body>");
    ptfTempFile->WriteEndLine();
    ptfTempFile->Write ("<h1>Directory index of ");
    ptfTempFile->Write (pszFixedUri);
    ptfTempFile->Write ("</h1><p>");
    ptfTempFile->WriteEndLine();

    // Get files and directories
    ptfTempFile->Write ("<table width = \"100%\">\n");

    UTCTime tLastModified;
    char pszLastModified[OS::MaxGMTDateLength];
    size_t stSize;

    ptfTempFile->Write ("<a href = \"../\">Parent directory</a><p>");
    ptfTempFile->WriteEndLine();

    for (i = 0; i < iNumFiles; i ++) {

        strcpy (pszDirSearchName, pszDirName);
        strcat (pszDirSearchName, "/");
        strcat (pszDirSearchName, ppszFileName[i]);
        
        ptfTempFile->Write ("<tr>");
        ptfTempFile->WriteEndLine();
        ptfTempFile->Write ("<td><a href=\"");
        ptfTempFile->Write (pszFixedUri);
        ptfTempFile->Write (ppszFileName[i]);

        // Is file a directory?
        switch (File::GetFileType (pszDirSearchName)) {

        case FILETYPE_DIRECTORY:

            ptfTempFile->Write ("/");
            ptfTempFile->Write ("\">");
            ptfTempFile->Write (ppszFileName[i]);
            ptfTempFile->Write ("/</a></td>");
            ptfTempFile->WriteEndLine();
            ptfTempFile->Write ("<td>&nbsp;</td><td>&nbsp;</td>");

            break;

        case FILETYPE_FILE:

            ptfTempFile->Write ("\">");
            ptfTempFile->Write (ppszFileName[i]);
            
            iErrCode = File::GetFileSize (pszDirSearchName, &stSize);
            if (iErrCode != OK) {
                return iErrCode;
            }
            
            sprintf (
                pszTempBuf, 
                "</a></td>\n<td align = \"right\">%u</td><td align = \"left\"> bytes</td>", 
                stSize
                );
            ptfTempFile->Write (pszTempBuf);

            break;

        case FILETYPE_ERROR:
        default:

            return ERROR_FAILURE;
        }
        
        ptfTempFile->Write ("<td>");

        iErrCode = File::GetLastModifiedTime (pszDirSearchName, &tLastModified);
        if (iErrCode != OK) {
            return iErrCode;
        }

        iErrCode = Time::GetGMTDateString (tLastModified, pszLastModified);
        if (iErrCode != OK) {
            return iErrCode;
        }
        
        ptfTempFile->Write (pszLastModified);           
        ptfTempFile->Write ("</td>\n</tr>");
        ptfTempFile->WriteEndLine();
    }
    
    ptfTempFile->Write ("</table></body></html>");

    return OK;
}


/*
        // Run SSI on html files
        if (pHttpRequest->GetPageSource()->UseSSI() && 
                stricmp (pfFile->GetMimeType(), "text/html") == 0) {
    
                // Assume that there is enough memory to keep the HTML file in memory
                char* pszNewBuffer;
                iErrCode = RunSSI (pssSocket, pHttpRequest, pszUri, pfFile, &pszNewBuffer);

                if (iErrCode != OK) {
                    iErrCode = Send500Response (pssSocket, pHttpRequest, pHttpResponse);
                } else {
                    iErrCode = SendResponse (HTTP_200, pssSocket, pHttpRequest, pHttpResponse, "text/html", 
                        pszNewBuffer, strlen (pszNewBuffer));
                    delete [] pszNewBuffer;
                }

            } else {

                // Send requested file
                iErrCode = SendResponse (HTTP_200, pssSocket, pHttpRequest, pHttpResponse, pfFile);
            }

            // Release the file
            pfFile->Release();
        }


int HttpServer::RunSSI (Socket* pssSocket, HttpRequest* pHttpRequest, const char* pszUri, ICachedFile* pfFile, 
                        char** ppszNewBuffer) {

    size_t stFileLen = pfFile->GetSize(), stCurLen = 0;
    const char* pszBuffer = (const char*) pfFile->GetData();

    int iCounter;
    bool bReplacement;

    const char* pszEndBuffer = pszBuffer + stFileLen, * pszRemainder = pszBuffer;
    char szTemp, * pszReplacement, pszCounter [25], * pszMarker;

    String strOSVersion, strTime, strDate;

    // Loop until entire file is processed
    *ppszNewBuffer = NULL;
    while (true) {

        // Scan for begin marker
        bReplacement = false;
        pszMarker = strstr (pszRemainder, m_pszSSIBegin);

        if (pszMarker == NULL) {

            // Append remainder of buffer to new buffer and exit
            stCurLen = AppendString (ppszNewBuffer, pszRemainder, stCurLen);
            break;

        } else {

            // Advance the marker;  exit loop if beyond scope of buffer
            szTemp = pszMarker[0];
            pszReplacement = pszMarker;
            pszMarker[0] = '\0';
            pszMarker += m_iSSIBegin;
            if (pszMarker >= pszEndBuffer) {
                break;
            }

            // Save scanned characters into new buffer
            stCurLen = AppendString (ppszNewBuffer, pszRemainder, stCurLen);

            // Time
            if (strncmp (pszMarker, m_pszSSITime, m_iSSITime) == 0) {
                if (strTime.IsBlank()) {
                    Time::GetTimeString (&strTime);
                }
                stCurLen = AppendString (ppszNewBuffer, strTime, stCurLen);
                pszRemainder = pszMarker += m_iSSITime;
                bReplacement = true;
            }

            // Date
            else if (strncmp (pszMarker, m_pszSSIDate, m_iSSIDate) == 0) {
                if (strDate.IsBlank()) {
                    Time::GetDateString (&strDate);
                }
                stCurLen = AppendString (ppszNewBuffer, strDate, stCurLen);
                pszRemainder = pszMarker += m_iSSIDate;
                bReplacement = true;
            }

            // Counter
            else if (strncmp (pszMarker, m_pszSSICounter, m_iSSICounter) == 0) {
                iCounter = RunSSICounter (pHttpRequest->GetPageSource(), pszUri);
                sprintf (pszCounter, "%i", iCounter);
                stCurLen = AppendString (ppszNewBuffer, pszCounter, stCurLen);
                pszRemainder = pszMarker += m_iSSICounter;
                bReplacement = true;
            }

            // DayOfWeek
            else if (strncmp (pszMarker, m_pszSSIDayOfWeek, m_iSSIDayOfWeek) == 0) {
                stCurLen = AppendString (ppszNewBuffer, Time::GetDayOfWeek(), stCurLen);
                pszRemainder = pszMarker += m_iSSIDayOfWeek;
                bReplacement = true;
            }

            // ClientIP
            else if (strncmp (pszMarker, m_pszSSIClientIP, m_iSSIClientIP) == 0) {
                stCurLen = AppendString (ppszNewBuffer, pssSocket->GetTheirIP(), stCurLen);
                pszRemainder = pszMarker += m_iSSIClientIP;
                bReplacement = true;
            }

            // ClientDomainName
            else if (strncmp (pszMarker, m_pszSSIClientDomainName, m_iSSIClientDomainName) == 0) {
                stCurLen = AppendString (ppszNewBuffer, pssSocket->GetTheirDomainName(), stCurLen);
                pszRemainder = pszMarker += m_iSSIClientDomainName;
                bReplacement = true;
            }

            // ClientBrowser
            else if (strncmp (pszMarker, m_pszSSIClientBrowser, m_iSSIClientBrowser) == 0) {
                stCurLen = AppendString (ppszNewBuffer, pHttpRequest->GetBrowserName(), stCurLen);
                pszRemainder = pszMarker += m_iSSIClientBrowser;
                bReplacement = true;
            }

            // ServerDomainName
            else if (strncmp (pszMarker, m_pszSSIServerDomainName, m_iSSIServerDomainName) == 0) {
                stCurLen = AppendString (ppszNewBuffer, m_pszHostName, stCurLen);
                pszRemainder = pszMarker += m_iSSIServerDomainName;
                bReplacement = true;
            }

            // ServerIP
            else if (strncmp (pszMarker, m_pszSSIServerIP, m_iSSIServerIP) == 0) {
                stCurLen = AppendString (ppszNewBuffer, m_pszIPAddress, stCurLen);
                pszRemainder = pszMarker += m_iSSIServerIP;
                bReplacement = true;
            }

            // ServerOS
            else if (strncmp (pszMarker, m_pszSSIServerOS, m_iSSIServerOS) == 0) {
                if (strOSVersion.IsBlank()) {
                    OS::GetOSVersion (&strOSVersion);
                }
                stCurLen = AppendString (ppszNewBuffer, strOSVersion, stCurLen);
                pszRemainder = pszMarker += m_iSSIServerOS;
                bReplacement = true;
            }

            // ServerVersion
            else if (strncmp (pszMarker, m_pszSSIServerVersion, m_iSSIServerVersion) == 0) {
                stCurLen = AppendString (ppszNewBuffer, m_pszServerName, stCurLen);
                pszRemainder = pszMarker += m_iSSIServerVersion;
                bReplacement = true;
            }

            // No match
            else {
                stCurLen = AppendString (ppszNewBuffer, m_pszSSIBegin, stCurLen);
                pszRemainder = pszMarker;
            }

            pszReplacement[0] = szTemp;
        }
    }
    
    return OK;
}




int HttpServer::RunSSICounter (PageSource* pPageSource, const char* pszUri) {
    
    ConfigFile* pCounter = pPageSource->GetCounterFile();
    char* pszFurther = EqualsToSlash (pszUri);
    
    int iCounter;
    char* pszText;
    if (pCounter->GetParameter (pszFurther, &pszText) == OK) {
        iCounter = atoi (pszText) + 1;
    } else {
        iCounter = 1;
    }

    LogMessage* plmMessage = new LogMessage;
    plmMessage->MessageType = SSI_COUNTER_MESSAGE;
    plmMessage->PageSource = pPageSource;
    strcpy (plmMessage->Text, pszFurther);
    
    // Add a log message to the queue
    m_mLogMessageMutex.Wait();
    m_pfqLogQueue->Push (plmMessage);
    m_mLogMessageMutex.Signal();

    // Wake up the log message thread
    m_tLogThread.Resume();
    
    delete [] pszFurther;

    return iCounter;
}



  
    // SSI
    m_pszSSIBegin = "<!--";
    m_iSSIBegin = strlen (m_pszSSIBegin);

    m_pszSSIEnd = "-->";
    m_iSSIEnd = strlen (m_pszSSIEnd);

    m_pszSSITime = new char [5 + m_iSSIEnd];
    strcpy (m_pszSSITime, "Time");
    strcat (m_pszSSITime, m_pszSSIEnd);
    m_iSSITime = strlen (m_pszSSITime);

    m_pszSSIDate = new char [5 + m_iSSIEnd];
    strcpy (m_pszSSIDate, "Date");
    strcat (m_pszSSIDate, m_pszSSIEnd);
    m_iSSIDate = strlen (m_pszSSIDate);

    m_pszSSIDayOfWeek = new char [10 + m_iSSIEnd];
    strcpy (m_pszSSIDayOfWeek, "DayOfWeek");
    strcat (m_pszSSIDayOfWeek, m_pszSSIEnd);
    m_iSSIDayOfWeek = strlen (m_pszSSIDayOfWeek);

    m_pszSSIClientIP = new char [9 + m_iSSIEnd];
    strcpy (m_pszSSIClientIP, "ClientIP");
    strcat (m_pszSSIClientIP, m_pszSSIEnd);
    m_iSSIClientIP = strlen (m_pszSSIClientIP);

    m_pszSSIClientDomainName = new char [17 + m_iSSIEnd];
    strcpy (m_pszSSIClientDomainName, "ClientDomainName");
    strcat (m_pszSSIClientDomainName, m_pszSSIEnd);
    m_iSSIClientDomainName = strlen (m_pszSSIClientDomainName);

    m_pszSSIClientBrowser = new char [14 + m_iSSIEnd];
    strcpy (m_pszSSIClientBrowser, "ClientBrowser");
    strcat (m_pszSSIClientBrowser, m_pszSSIEnd);
    m_iSSIClientBrowser = strlen (m_pszSSIClientBrowser);

    m_pszSSIServerIP = new char [9 + m_iSSIEnd];
    strcpy (m_pszSSIServerIP, "ServerIP");
    strcat (m_pszSSIServerIP, m_pszSSIEnd);
    m_iSSIServerIP = strlen (m_pszSSIServerIP);

    m_pszSSIServerDomainName = new char [17 + m_iSSIEnd];
    strcpy (m_pszSSIServerDomainName, "ServerDomainName");
    strcat (m_pszSSIServerDomainName, m_pszSSIEnd);
    m_iSSIServerDomainName = strlen (m_pszSSIServerDomainName);

    m_pszSSIServerOS = new char [9 + m_iSSIEnd];
    strcpy (m_pszSSIServerOS, "ServerOS");
    strcat (m_pszSSIServerOS, m_pszSSIEnd);
    m_iSSIServerOS = strlen (m_pszSSIServerOS);

    m_pszSSIServerVersion = new char [14 + m_iSSIEnd];
    strcpy (m_pszSSIServerVersion, "ServerVersion");
    strcat (m_pszSSIServerVersion, m_pszSSIEnd);
    m_iSSIServerVersion = strlen (m_pszSSIServerVersion);

    m_pszSSICounter = new char [8 + m_iSSIEnd];
    strcpy (m_pszSSICounter, "Counter");
    strcat (m_pszSSICounter, m_pszSSIEnd);
    m_iSSICounter = strlen (m_pszSSICounter);
*/

int HttpResponse::ProcessPost() {

    // Check for turned off page source
    if (m_pPageSource == NULL) {
        return OK;
    }

    if (m_pPageSource->OverridePost()) {

        // Call the pagesource function
        if (m_pPageSource->OnPost (m_pHttpRequest, this) != OK) {
            
            if (!m_bHeadersSent) {
                SetStatusCodeOnError (HTTP_500);
            }
            return OK;
        }

        // If the page source didn't do anything, fall through
        if (m_rType != RESPONSE_NONE) {
            return OK;
        }       

    }

    // Treat POST as GET
    return ProcessGet();
}

HttpRequest* HttpResponse::GetHttpRequest() const {
    return m_pHttpRequest;
}

void HttpResponse::SetSocket (Socket* pSocket) {

    m_pSocket = pSocket;
    Assert (m_pSocket != NULL);
}

Socket* HttpResponse::GetSocket() const {
    return m_pSocket;
}

bool HttpResponse::ConnectionClosed() const {
    return m_bConnectionClosed;
}

MilliSeconds HttpResponse::GetResponseTime() const {
    return m_msResponseTime;
}

size_t HttpResponse::GetResponseLength() const {
    return m_stResponseLength;
}

int HttpResponse::AddHeader (const char* pszHeaderName, const char* pszHeaderValue) {

    Assert (pszHeaderName != NULL);

    size_t stNameLen = String::StrLen (pszHeaderName);
    size_t stValueLen = String::StrLen (pszHeaderValue);

    size_t stTotalLen = stNameLen + stValueLen;

    if (stTotalLen == 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    stTotalLen += sizeof ("\r\n: ") - 1;

    if (pszHeaderValue == NULL) {
        pszHeaderValue = "";
    }

    if (m_pszCustomHeaders == NULL) {

        Assert (m_stCustomHeaderLength == 0);

        m_pszCustomHeaders = new char [stTotalLen + 1];
        if (m_pszCustomHeaders == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        sprintf (m_pszCustomHeaders, "\r\n%s: %s", pszHeaderName, pszHeaderValue);
        m_stCustomHeaderLength = stTotalLen;

        return OK;
    }

    char* pszCustomHeaders = new char [m_stCustomHeaderLength + stTotalLen + 1];
    if (m_pszCustomHeaders == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    sprintf (pszCustomHeaders, "%s\r\n%s: %s", m_pszCustomHeaders, pszHeaderName, pszHeaderValue);
    m_stCustomHeaderLength += stTotalLen;

    return OK;
}

int HttpResponse::AddCustomLogMessage (const char* pszCustomLogMessage) {

    if (m_ppszCustomLogMessages == NULL) {
        m_iCustomLogMessageSpace = 5;
        m_ppszCustomLogMessages = new char* [5];
        if (m_ppszCustomLogMessages == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
    }
    else if (m_iCustomLogMessageSpace == m_iNumCustomLogMessages) {

        unsigned int iCustomLogMessageSpace = m_iCustomLogMessageSpace * 2;

        char** ppszTemp = new char* [m_iCustomLogMessageSpace];
        if (ppszTemp == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        memcpy (ppszTemp, m_ppszCustomLogMessages, m_iNumCustomLogMessages * sizeof (char*));
        delete [] m_ppszCustomLogMessages;

        m_ppszCustomLogMessages = ppszTemp;
        m_iCustomLogMessageSpace = iCustomLogMessageSpace;
    }

    m_ppszCustomLogMessages[m_iNumCustomLogMessages ++] = String::StrDup (pszCustomLogMessage);

    return OK;
}

int HttpResponse::GetCustomLogMessages (const char*** pppszCustomLogMessages, 
                                        unsigned int* piNumCustomLogMessages) {

    *pppszCustomLogMessages = (const char**) m_ppszCustomLogMessages;
    *piNumCustomLogMessages = m_iNumCustomLogMessages;

    return OK;
}