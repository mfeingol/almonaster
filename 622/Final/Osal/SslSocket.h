// Socket.h: interface for the Socket class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA  02111-1307, USA.

#if !defined(AFX_SSLSOCKET_H__E0FAE59D_A3F7_11D1_9C48_0060083E8062__INCLUDED_)
#define AFX_SSLSOCKET_H__E0FAE59D_A3F7_11D1_9C48_0060083E8062__INCLUDED_

#include "Socket.h"

#ifdef __WIN32__

#include <schnlsp.h>
#define SECURITY_WIN32
#include <security.h>

class OSAL_EXPORT SslContext {
    
    friend class SslSocket;

private:
    CredHandle m_serverCreds;
    unsigned long m_ulMaxInitialChunkSize;

    BYTE* ReadFile(const char* pszFileName, size_t* pcbFile);

public:
    SslContext();
    ~SslContext();

    int Initialize (const char* pszCertFile, const char* pszKeyFile, const char* pszKeyFilePassword);
};

class OSAL_EXPORT SslSocket : public Socket {

private:

    SslContext* m_pSslContext;

    CtxtHandle m_ctxHandle;

    bool m_bConnected;
    size_t m_cbMaxChunkSize;
    size_t m_cbHeaderSize;
    size_t m_cbTrailerSize;

    BYTE* m_pIncompleteData;
    size_t m_cbIncompleteData;

    BYTE* m_pLeftoverPlainText;
    size_t m_cbLeftoverPlainText;

    void Clear();
    int SendSecBuffer(const SecBuffer& secBuffer);
    size_t PlainTextSocketSend(const void* pData, size_t cbData);
    size_t SocketSendChunk(const void* pData, size_t cbSend);
    int DecryptMessageWrapper(BYTE* pbData, size_t cbCypherData, size_t* pcbPlainData);

    virtual size_t SocketSend (const void* pData, size_t cbSend);
    virtual size_t SocketRecv (void* pData, size_t stNumBytes);

public:

    SslSocket (SslContext* pSslContext);
    virtual ~SslSocket();

    virtual int Close();

    virtual Socket* Accept();
    virtual int Negotiate();

    virtual int Connect (const char* pszAddress, short siPort);
};

#else

#include "openssl/ssl.h"

class OSAL_EXPORT SslContext {

    friend class SslSocket;

private:
    SSL_CTX* m_pCtx;
    SSL_CTX* GetContext();

public:
    SslContext();
    ~SslContext();

    int Initialize (const char* pszCertFile, const char* pszKeyFile);
};

class OSAL_EXPORT SslSocket : public Socket {
private:
    SSL_CTX* m_pCtx;
    SSL* m_pSsl;

    virtual size_t SocketSend (const void* pData, size_t cbSend);
    virtual size_t SocketRecv (void* pData, size_t stNumBytes);

public:

    SslSocket (SslContext* pSslContext);
    SslSocket (SSL* pSsl);
    virtual ~SslSocket();

    virtual int Close();

    virtual Socket* Accept();
    virtual int Negotiate();

    virtual int Connect (const char* pszAddress, short siPort);
};

#endif

#endif