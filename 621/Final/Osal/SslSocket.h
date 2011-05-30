// Socket.h: interface for the Socket class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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
#include "openssl/ssl.h"

class OSAL_EXPORT SslContext {
private:
    SSL_CTX* m_pCtx;

public:
    SslContext();
    ~SslContext();

    int Initialize (const char* pszCertFile, const char* pszKeyFile);
    SSL_CTX* GetContext();
};

class OSAL_EXPORT SslSocket : public Socket {
private:
    SSL_CTX* m_pCtx;
    SSL* m_pSsl;

    virtual size_t SocketSend (const void* pData, size_t cbSend);
    virtual size_t SocketRecv (void* pData, size_t stNumBytes);
    virtual size_t SocketPeek (void* pData, size_t stNumBytes);

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