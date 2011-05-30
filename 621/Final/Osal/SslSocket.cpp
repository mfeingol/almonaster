// Socket.cpp: implementation of the Socket class.
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

#define OSAL_BUILD
#include "SslSocket.h"
#undef OSAL_BUILD

#ifdef _WIN32

typedef void (__cdecl *FXN_OpenSSL_add_all_algorithms)();
typedef int (__cdecl *FXN_SSL_library_init)();

typedef SSL_CTX* (__cdecl *FXN_SSL_CTX_new) (SSL_METHOD*);
typedef void (__cdecl *FXN_SSL_CTX_free) (SSL_CTX*);
typedef int (__cdecl *FXN_SSL_CTX_check_private_key) (SSL_CTX*);
typedef int (__cdecl *FXN_SSL_CTX_use_certificate_file) (SSL_CTX*, const char*, int);
typedef int (__cdecl *FXN_SSL_CTX_use_PrivateKey_file) (SSL_CTX*, const char*, int);
typedef SSL_METHOD* (__cdecl *FXN_SSL_server_method) (void);
typedef SSL* (__cdecl *FXN_SSL_new) (SSL_CTX*);
typedef void (__cdecl *FXN_SSL_free) (SSL*);
typedef int (__cdecl *FXN_SSL_accept) (SSL*);
typedef int (__cdecl *FXN_SSL_set_fd) (SSL*, int);
typedef int (__cdecl *FXN_SSL_read) (SSL*, void*, int);
typedef int (__cdecl *FXN_SSL_write) (SSL*, const void*, int);
typedef long (__cdecl *FXN_ERR_get_error)();
typedef void (__cdecl *FXN_ERR_error_string_n)(unsigned long, char*, size_t);

class AutoLoadOpenSsl {
private:

    HMODULE m_hModuleSsl;
    HMODULE m_hModuleLib;
    bool m_bEnabled;

public:

    FXN_SSL_CTX_new Dyn_SSL_CTX_new;
    FXN_SSL_CTX_free Dyn_SSL_CTX_free;
    FXN_SSL_CTX_check_private_key Dyn_SSL_CTX_check_private_key;
    FXN_SSL_CTX_use_certificate_file Dyn_SSL_CTX_use_certificate_file;
    FXN_SSL_CTX_use_PrivateKey_file Dyn_SSL_CTX_use_PrivateKey_file;
    FXN_SSL_server_method Dyn_SSL_server_method;
    FXN_SSL_new Dyn_SSL_new;
    FXN_SSL_free Dyn_SSL_free;
    FXN_SSL_accept Dyn_SSL_accept;
    FXN_SSL_set_fd Dyn_SSL_set_fd;
    FXN_SSL_read Dyn_SSL_read;
    FXN_SSL_write Dyn_SSL_write;
    FXN_ERR_get_error Dyn_ERR_get_error;
    FXN_ERR_error_string_n Dyn_ERR_error_string_n;

    AutoLoadOpenSsl() {

        m_bEnabled = false;
        m_hModuleSsl = m_hModuleLib = NULL;

        // Load libraries
        m_hModuleLib = LoadLibraryW (L"libeay32.dll");
        if (m_hModuleLib == NULL) {
            return;
        }

        m_hModuleSsl = LoadLibraryW (L"ssleay32.dll");
        if (m_hModuleSsl == NULL) {
            return;
        }

        // Initialize
        FXN_OpenSSL_add_all_algorithms Dyn_OpenSSL_add_all_algorithms = (FXN_OpenSSL_add_all_algorithms) 
            GetProcAddress (m_hModuleLib, "OPENSSL_add_all_algorithms_noconf");

        if (Dyn_OpenSSL_add_all_algorithms == NULL) {
            return;
        }

        FXN_SSL_library_init Dyn_SSL_library_init = (FXN_SSL_library_init) 
            GetProcAddress (m_hModuleSsl, "SSL_library_init");

        if (Dyn_OpenSSL_add_all_algorithms == NULL ||
            Dyn_SSL_library_init == NULL) {
            return;
        }

        //SSL_load_error_strings();
        Dyn_SSL_library_init();
        Dyn_OpenSSL_add_all_algorithms();

        // Get all proc addresses
        Dyn_SSL_CTX_new = (FXN_SSL_CTX_new) GetProcAddress (m_hModuleSsl, "SSL_CTX_new");
        Dyn_SSL_CTX_free = (FXN_SSL_CTX_free) GetProcAddress (m_hModuleSsl, "SSL_CTX_free");
        Dyn_SSL_CTX_check_private_key = (FXN_SSL_CTX_check_private_key) GetProcAddress (m_hModuleSsl, "SSL_CTX_check_private_key");
        Dyn_SSL_CTX_use_certificate_file = (FXN_SSL_CTX_use_certificate_file) GetProcAddress (m_hModuleSsl, "SSL_CTX_use_certificate_file");
        Dyn_SSL_CTX_use_PrivateKey_file = (FXN_SSL_CTX_use_PrivateKey_file) GetProcAddress (m_hModuleSsl, "SSL_CTX_use_PrivateKey_file");
        Dyn_SSL_server_method = (FXN_SSL_server_method) GetProcAddress (m_hModuleSsl, "SSLv23_server_method");
        Dyn_SSL_new = (FXN_SSL_new) GetProcAddress (m_hModuleSsl, "SSL_new");
        Dyn_SSL_free = (FXN_SSL_free) GetProcAddress (m_hModuleSsl, "SSL_free");
        Dyn_SSL_accept = (FXN_SSL_accept) GetProcAddress (m_hModuleSsl, "SSL_accept");
        Dyn_SSL_set_fd = (FXN_SSL_set_fd) GetProcAddress (m_hModuleSsl, "SSL_set_fd");
        Dyn_SSL_read = (FXN_SSL_read) GetProcAddress (m_hModuleSsl, "SSL_read");
        Dyn_SSL_write = (FXN_SSL_write) GetProcAddress (m_hModuleSsl, "SSL_write");
        Dyn_ERR_get_error = (FXN_ERR_get_error) GetProcAddress (m_hModuleLib, "ERR_get_error");
        Dyn_ERR_error_string_n = (FXN_ERR_error_string_n) GetProcAddress (m_hModuleLib, "ERR_error_string_n");

        if (Dyn_SSL_CTX_new == NULL ||
            Dyn_SSL_CTX_free == NULL ||
            Dyn_SSL_CTX_check_private_key == NULL ||
            Dyn_SSL_CTX_use_certificate_file == NULL ||
            Dyn_SSL_CTX_use_PrivateKey_file == NULL ||
            Dyn_SSL_server_method == NULL ||
            Dyn_SSL_new == NULL ||
            Dyn_SSL_free == NULL ||
            Dyn_SSL_accept == NULL ||
            Dyn_SSL_set_fd == NULL ||
            Dyn_SSL_read == NULL ||
            Dyn_SSL_write == NULL ||
            Dyn_ERR_get_error == NULL ||
            Dyn_ERR_error_string_n == NULL
            ) {
            return;
        }        

        m_bEnabled = true;
    }

    bool IsEnabled() {
        return m_bEnabled;
    }
};
AutoLoadOpenSsl Ssl;

#define SSL_CTX_new Ssl.Dyn_SSL_CTX_new
#define SSL_CTX_free Ssl.Dyn_SSL_CTX_free
#define SSL_CTX_check_private_key Ssl.Dyn_SSL_CTX_check_private_key
#define SSL_CTX_use_certificate_file Ssl.Dyn_SSL_CTX_use_certificate_file
#define SSL_CTX_use_PrivateKey_file Ssl.Dyn_SSL_CTX_use_PrivateKey_file
#define SSL_server_method Ssl.Dyn_SSL_server_method
#define SSL_new Ssl.Dyn_SSL_new
#define SSL_free Ssl.Dyn_SSL_free
#define SSL_accept Ssl.Dyn_SSL_accept
#define SSL_set_fd Ssl.Dyn_SSL_set_fd
#define SSL_read Ssl.Dyn_SSL_read
#define SSL_write Ssl.Dyn_SSL_write
#define ERR_get_error Ssl.Dyn_ERR_get_error
#define ERR_error_string_n Ssl.Dyn_ERR_error_string_n

#endif

SslContext::SslContext() {

    m_pCtx = NULL;
}

SslContext::~SslContext() {

    if (m_pCtx != NULL) {
        SSL_CTX_free (m_pCtx);
        m_pCtx = NULL;
    }
}

SSL_CTX* SslContext::GetContext() {
    return m_pCtx;
}

int SslContext::Initialize (const char* pszCertFile, const char* pszKeyFile) {

#ifdef _WIN32
    if (!Ssl.IsEnabled()) {
        return ERROR_FAILURE;
    }
#endif

    SSL_METHOD* pMethod = SSL_server_method();
    if (pMethod == NULL) {
        return ERROR_FAILURE;
    }

    SSL_CTX* pCtx = SSL_CTX_new (pMethod);
    if (pCtx == NULL) {
        /*
        char buf[256];
        long lErr = ERR_get_error();
        ERR_error_string_n(lErr, buf, countof(buf));
        printf(buf);
        */
        return ERROR_FAILURE;
    }

    if (SSL_CTX_use_certificate_file (pCtx, pszCertFile, SSL_FILETYPE_PEM) != 1) {
        SSL_CTX_free (pCtx);
        return ERROR_FAILURE;
    }

    if (SSL_CTX_use_PrivateKey_file (pCtx, pszKeyFile, SSL_FILETYPE_PEM) != 1) {
        SSL_CTX_free (pCtx);
        return ERROR_FAILURE;
    }

    if (!SSL_CTX_check_private_key (pCtx)) {
        SSL_CTX_free (pCtx);
        return ERROR_FAILURE;
    }

    m_pCtx = pCtx;

    return OK;
}

SslSocket::SslSocket (SslContext* pSslContext) {
    m_pCtx = pSslContext->GetContext();
    m_pSsl = NULL;
}

SslSocket::SslSocket (SSL* pSsl) {
    m_pCtx = NULL;
    m_pSsl = pSsl;
}

SslSocket::~SslSocket() {

    if (m_pSsl != NULL) {
        SSL_free (m_pSsl);
        m_pSsl = NULL;
    }
}

int SslSocket::Connect (const char* pszAddress, short siPort) {

    // TODO - Client-side SSL support
    Assert (false);
    return ERROR_NOT_IMPLEMENTED;
}

int SslSocket::Close() {

    if (m_pSsl != NULL) {
        SSL_free (m_pSsl);
        m_pSsl = NULL;
    }

    return Socket::Close();
}

Socket* SslSocket::Accept() {

    Assert (m_Socket != INVALID_SOCKET);

    SSL* pSsl = SSL_new (m_pCtx);
    if (pSsl == NULL) {
        return NULL;
    }

    SslSocket* pSocket = new SslSocket (pSsl);
    if (pSocket == NULL) {
        SSL_free (pSsl);
        return NULL;
    }

    if (Socket::Accept (pSocket) != OK) {
        delete pSocket;
        return NULL;
    }

    return pSocket;
}

int SslSocket::Negotiate() {

    if (SSL_set_fd (m_pSsl, m_Socket) != 1) {
        return ERROR_FAILURE;
    }

    int ret = SSL_accept (m_pSsl);
    if (ret != 1) {
        return ERROR_FAILURE;
    }

    return OK;
}

size_t SslSocket::SocketSend (const void* pData, size_t cbSend) {
    return SSL_write (m_pSsl, pData, cbSend);
}

size_t SslSocket::SocketRecv (void* pData, size_t stNumBytes) {

    if (m_pSsl == NULL) {
        return 0;
    }
    return SSL_read (m_pSsl, pData, stNumBytes);
}

size_t SslSocket::SocketPeek (void* pData, size_t stNumBytes) {
    return recv (m_Socket, (char*) pData, (int) stNumBytes, MSG_PEEK);
}