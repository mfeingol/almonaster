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

#ifdef __WIN32__

#include <stdio.h>
#include "File.h"
#include "Algorithm.h"

SslContext::SslContext() {
    memset(&m_serverCreds, 0, sizeof(m_serverCreds));
}

SslContext::~SslContext() {
    FreeCredentialsHandle(&m_serverCreds);
}

BYTE* SslContext::ReadFile(const char* pszFileName, size_t* pcbFile) {

    *pcbFile = 0;

    File fFile;
    int iErrCode = fFile.OpenRead(pszFileName);
    if (iErrCode != OK)
        return NULL;

    size_t cbSize;
    iErrCode = fFile.GetSize(&cbSize);
    if (iErrCode != OK)
        return NULL;

    BYTE* pbFile = new BYTE[cbSize];
    if (pbFile == NULL)
        return NULL;

    iErrCode = fFile.Read(pbFile, cbSize, pcbFile);
    if (iErrCode != OK) {
        delete [] pbFile;
        return NULL;
    }

    return pbFile;
}

class AutoClosePCCERT_CONTEXT {
private:
    PCCERT_CONTEXT m_ctx;
public:
    AutoClosePCCERT_CONTEXT(PCCERT_CONTEXT ctx) {
        m_ctx = ctx;
    }

    ~AutoClosePCCERT_CONTEXT() {
        CertFreeCertificateContext(m_ctx);
    }
};

class AutoCloseHCERTSTORE{
private:
    HCERTSTORE m_store;
public:
    AutoCloseHCERTSTORE(HCERTSTORE store) {
        m_store = store;
    }

    ~AutoCloseHCERTSTORE() {
        CertCloseStore(m_store, 0);
    }
};

int SslContext::Initialize (const char* pszCertFile, const char* pszKeyFile) {

    DWORD cbData;

    // Open the public key
    size_t cbCert;
    BYTE* pbCert = ReadFile(pszCertFile, &cbCert);
    if (pbCert == NULL)
        return ERROR_FAILURE;

    Algorithm::AutoDelete<BYTE> autoCert(pbCert, true);

    PCCERT_CONTEXT phPublicKey = CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
        pbCert, 
        cbCert);
    if (phPublicKey == NULL)
        return ERROR_FAILURE;

    AutoClosePCCERT_CONTEXT autoPublicKey(phPublicKey);

    // Open the private key file
    size_t cbPfx;
    BYTE* pbPfx = ReadFile(pszKeyFile, &cbPfx);
    if (pbPfx == NULL)
        return ERROR_FAILURE;

    Algorithm::AutoDelete<BYTE> autodelete(pbPfx, true);

    CRYPT_DATA_BLOB pfxBlob;
    pfxBlob.cbData = (DWORD)cbPfx;
    pfxBlob.pbData = pbPfx;

    HCERTSTORE hTempStore = PFXImportCertStore(&pfxBlob, NULL, 0);
    if (hTempStore == NULL)
        return ERROR_FAILURE;

    AutoCloseHCERTSTORE autoTempStore(hTempStore);

    // Make sure the two match
    cbData = 0;
    CertGetCertificateContextProperty(phPublicKey, CERT_SHA1_HASH_PROP_ID, NULL, &cbData);
    BYTE* pbPublicKeySha1Hash = (BYTE*)StackAlloc(cbData);

    if (!CertGetCertificateContextProperty(phPublicKey, CERT_SHA1_HASH_PROP_ID, pbPublicKeySha1Hash, &cbData))
        return ERROR_FAILURE;

    PCCERT_CONTEXT phPrivateKey = NULL;
    while ((phPrivateKey = CertEnumCertificatesInStore(hTempStore, phPrivateKey)) != NULL) {

        cbData = 0;
        CertGetCertificateContextProperty(phPrivateKey, CERT_SHA1_HASH_PROP_ID, NULL, &cbData);
        BYTE* pbPrivateKeySha1Hash = (BYTE*)StackAlloc(cbData);

        if (!CertGetCertificateContextProperty(phPrivateKey, CERT_SHA1_HASH_PROP_ID, pbPrivateKeySha1Hash, &cbData)) {
            CertFreeCertificateContext(phPrivateKey);
            return ERROR_FAILURE;
        }

        if (memcmp(pbPublicKeySha1Hash, pbPrivateKeySha1Hash, cbData) == 0) {

            // Make sure the final cert handle has an associated private key
            cbData = 0;
            CertGetCertificateContextProperty(phPrivateKey, CERT_KEY_PROV_INFO_PROP_ID, NULL, &cbData);

            if (cbData > 0) {
                CRYPT_KEY_PROV_INFO* pinfo = (CRYPT_KEY_PROV_INFO*) StackAlloc(cbData);
                if (CertGetCertificateContextProperty(phPrivateKey, CERT_KEY_PROV_INFO_PROP_ID, pinfo, &cbData))
                    break;
            }
        }
    }

    // Make sure we have a handle to the private key
    if (phPrivateKey == NULL)
        return ERROR_FAILURE;

    AutoClosePCCERT_CONTEXT autoPrivateKey(phPrivateKey);

    SCHANNEL_CRED sChannelCred = {0};
    sChannelCred.dwVersion = SCHANNEL_CRED_VERSION;
    sChannelCred.dwFlags = SCH_CRED_NO_SYSTEM_MAPPER;
    sChannelCred.cCreds = 1;
    sChannelCred.paCred = &phPrivateKey;
    sChannelCred.grbitEnabledProtocols = SP_PROT_SSL3_CLIENT | SP_PROT_SSL3_SERVER |
                                         SP_PROT_TLS1_CLIENT | SP_PROT_TLS1_SERVER;

    SECURITY_STATUS ss = AcquireCredentialsHandle(
        NULL,
		UNISP_NAME,
		SECPKG_CRED_INBOUND,
		NULL,
		&sChannelCred,
		NULL,
		NULL,
        &m_serverCreds,
		NULL);       

    if (ss != SEC_E_OK)
        return ERROR_FAILURE;

    PSecPkgInfo psecInfo;
    ss = QuerySecurityPackageInfo(UNISP_NAME, &psecInfo);
    if (ss != SEC_E_OK)
        return ERROR_FAILURE;

    m_ulMaxInitialChunkSize = psecInfo->cbMaxToken;
    FreeContextBuffer(psecInfo);

    return OK;
}

SslSocket::SslSocket (SslContext* pSslContext) {

    m_pSslContext = pSslContext;
    SecInvalidateHandle(&m_ctxHandle);

    m_pIncompleteData = NULL;
    m_cbIncompleteData = 0;

    m_pLeftoverPlainText = NULL;
    m_cbLeftoverPlainText = 0;

    m_bConnected = false;
    m_cbMaxChunkSize = 0;
    m_cbHeaderSize = 0;
    m_cbTrailerSize = 0;
}

SslSocket::~SslSocket() {
    Clear();
}

void SslSocket::Clear() {

    // We don't own the SSL context
    m_pSslContext = NULL;

    if (SecIsValidHandle(&m_ctxHandle)) {
        DeleteSecurityContext(&m_ctxHandle);
        SecInvalidateHandle(&m_ctxHandle);
    }

    delete [] m_pIncompleteData;
    m_pIncompleteData = NULL;
    m_cbIncompleteData = 0;

    delete [] m_pLeftoverPlainText;
    m_pLeftoverPlainText = NULL;
    m_cbLeftoverPlainText = 0;
}

int SslSocket::Close() {

    SECURITY_STATUS ss;

    if (SecIsValidHandle(&m_ctxHandle)) {

        DWORD dwShutdown = SCHANNEL_SHUTDOWN;

        SecBuffer inBuffer;
        inBuffer.BufferType = SECBUFFER_TOKEN;
        inBuffer.cbBuffer = sizeof(DWORD);
        inBuffer.pvBuffer = &dwShutdown;

        SecBufferDesc inBufferDesc;
        inBufferDesc.ulVersion = SECBUFFER_VERSION;
        inBufferDesc.cBuffers = 1;
        inBufferDesc.pBuffers = &inBuffer;

        ss = ApplyControlToken(&m_ctxHandle, &inBufferDesc);
        if (ss == SEC_E_OK) {

            ULONG ulSSPIOutFlags = 0;

            SecBuffer outBuffer;
            outBuffer.BufferType = SECBUFFER_TOKEN;
            outBuffer.cbBuffer = 0;
            outBuffer.pvBuffer = NULL;

            SecBufferDesc outBufferDesc;
            outBufferDesc.ulVersion = SECBUFFER_VERSION;
            outBufferDesc.cBuffers = 1;
            outBufferDesc.pBuffers = &outBuffer;

            ss = AcceptSecurityContext(
                &m_pSslContext->m_serverCreds,
                &m_ctxHandle,
                NULL,
                ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT   | ASC_REQ_CONFIDENTIALITY |
                ASC_REQ_EXTENDED_ERROR  | ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_STREAM,
                SECURITY_NATIVE_DREP,
                NULL,
                &outBufferDesc,
                &ulSSPIOutFlags,
                NULL);

            if (ss == SEC_E_OK && outBuffer.cbBuffer > 0 && outBuffer.pvBuffer != NULL)
                // Best effort
                SendSecBuffer(outBuffer);

            FreeContextBuffer(outBuffer.pvBuffer);
        }
    }

    m_bConnected = false;

    Clear();
    return Socket::Close();
}

Socket* SslSocket::Accept() {

    SslSocket* pSocket = new SslSocket (m_pSslContext);
    if (pSocket == NULL) {
        return NULL;
    }

    if (Socket::Accept (pSocket) != OK) {
        delete pSocket;
        return NULL;
    }

    return pSocket;
}

int SslSocket::Negotiate() {

    const unsigned long cbMaxInitialChunk = m_pSslContext->m_ulMaxInitialChunkSize;

    size_t cbExtra = 0;
    BYTE* pbRecvBuffer = new BYTE[cbMaxInitialChunk];
    if (pbRecvBuffer == NULL)
        return ERROR_OUT_OF_MEMORY;

    Algorithm::AutoDelete<BYTE> autoDelete (pbRecvBuffer, true);

    SecBuffer inSecBuffers[2];
    SecBufferDesc inBuffer;
    inBuffer.ulVersion = SECBUFFER_VERSION;
    inBuffer.cBuffers = countof(inSecBuffers);
    inBuffer.pBuffers = inSecBuffers;

    SecBuffer outSecBuffer;
    SecBufferDesc outBuffer;
    outBuffer.ulVersion = SECBUFFER_VERSION;
    outBuffer.cBuffers = 1;
    outBuffer.pBuffers = &outSecBuffer;

    int iErrCode = OK;
    SECURITY_STATUS ss = SEC_I_CONTINUE_NEEDED;
    bool bRecv = true;

    // Loop until we're done, one way or another
    while (ss == SEC_I_CONTINUE_NEEDED && iErrCode == OK) {

        ULONG ulSSPIOutFlags = 0;

        size_t cbRecvd = 0;
        
        if (bRecv) {
            cbRecvd = Socket::SocketRecv(pbRecvBuffer + cbExtra, cbMaxInitialChunk - cbExtra);
            if (cbRecvd == SOCKET_ERROR || cbRecvd == 0) {
                iErrCode = ERROR_FAILURE;
                break;
            }
        }

        const size_t cbInputBuffer = cbRecvd + cbExtra;

        inSecBuffers[0].BufferType = SECBUFFER_TOKEN;
        inSecBuffers[0].cbBuffer = (unsigned long) cbInputBuffer;
        inSecBuffers[0].pvBuffer = pbRecvBuffer;

        inSecBuffers[1].BufferType = SECBUFFER_EMPTY;
        inSecBuffers[1].cbBuffer = 0;
        inSecBuffers[1].pvBuffer = NULL;

        outSecBuffer.BufferType = SECBUFFER_TOKEN;
        outSecBuffer.cbBuffer = 0;
        outSecBuffer.pvBuffer = NULL;

        ss = AcceptSecurityContext(
            &m_pSslContext->m_serverCreds,
            SecIsValidHandle(&m_ctxHandle) ? &m_ctxHandle : NULL,
            &inBuffer,
            ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT   | ASC_REQ_CONFIDENTIALITY |
            ASC_REQ_EXTENDED_ERROR  | ASC_REQ_ALLOCATE_MEMORY | ASC_REQ_STREAM,
            SECURITY_NATIVE_DREP,
            &m_ctxHandle,
            &outBuffer,
            &ulSSPIOutFlags,
            NULL);

        switch (ss) {

        case SEC_E_OK:

            if (outSecBuffer.cbBuffer > 0 &&
                outSecBuffer.pvBuffer != NULL) {
                Assert(outSecBuffer.BufferType == SECBUFFER_TOKEN);
                iErrCode = SendSecBuffer(outSecBuffer);
            }
            else Assert(outSecBuffer.BufferType == SECBUFFER_EMPTY);

            // Compute max sizes
            SecPkgContext_StreamSizes sizes;
            if (QueryContextAttributes(&m_ctxHandle, SECPKG_ATTR_STREAM_SIZES, &sizes) != SEC_E_OK) {
                iErrCode = ERROR_FAILURE;
            } else {

                m_cbMaxChunkSize = sizes.cbMaximumMessage;
                m_cbHeaderSize = sizes.cbHeader;
                m_cbTrailerSize = sizes.cbTrailer;

                Assert(inSecBuffers[0].BufferType == SECBUFFER_TOKEN);
                if (inSecBuffers[1].BufferType == SECBUFFER_EXTRA) {

                    Assert(inSecBuffers[1].cbBuffer > 0);
                    Assert(inSecBuffers[1].pvBuffer == NULL);
                    Assert(m_pIncompleteData == NULL);
                    Assert(m_cbIncompleteData == 0);

                    // Save the extra data for the next Recv
                    size_t cbIncomplete = inSecBuffers[1].cbBuffer;
                    m_pIncompleteData = new BYTE[cbIncomplete];
                    if (m_pIncompleteData == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                    } else {
                        m_cbIncompleteData = cbIncomplete;
                        memcpy(m_pIncompleteData, pbRecvBuffer + cbInputBuffer - cbIncomplete, cbIncomplete);
                    }
                }
                else Assert(inSecBuffers[1].BufferType == SECBUFFER_EMPTY);
            }

            break;

        case SEC_E_INCOMPLETE_MESSAGE:

            // Receive some more data and try that again
            bRecv = true;
            cbExtra += cbRecvd;
            break;

        case SEC_I_CONTINUE_NEEDED:

            // We can get an empty out buffer and an extra in buffer
            if (outSecBuffer.BufferType == SECBUFFER_EMPTY) {

                if (inSecBuffers[1].BufferType == SECBUFFER_EXTRA) {

                    Assert (inSecBuffers[1].cbBuffer > 0);
                    Assert (inSecBuffers[1].pvBuffer == NULL);

                    // Try again using this data
                    size_t cbIncomplete = inSecBuffers[1].cbBuffer;
                    memmove(pbRecvBuffer, pbRecvBuffer + cbInputBuffer - cbIncomplete, cbIncomplete);
                    cbExtra = cbIncomplete;

                    // Don't receive any more data for the time being, unless we're not making progress
                    bRecv = !bRecv;
                
                } else {

                    // What to do...?  Receive some more, I guess
                    cbExtra = 0;
                    bRecv = true;
                }

            } else {

                Assert(outSecBuffer.BufferType == SECBUFFER_TOKEN);
                Assert(outSecBuffer.cbBuffer != 0);
                Assert(outSecBuffer.pvBuffer != NULL);

                // Send a buffer back to the client and try that again
                iErrCode = SendSecBuffer(outSecBuffer);

                // Receive some data
                cbExtra = 0;
                bRecv = true;
            }

            break;

        default:

            //printf("AcceptSecurityContext returned 0x%x\n", ss);

            // Best effort send back information to the client
            if (outSecBuffer.cbBuffer != 0 && (ulSSPIOutFlags & ISC_RET_EXTENDED_ERROR))
                SendSecBuffer(outSecBuffer);

            // We failed, regardless
            iErrCode = ERROR_FAILURE;
            break;
        }

        FreeContextBuffer(outSecBuffer.pvBuffer);
    }

    m_bConnected = iErrCode == OK;

    return iErrCode;
}

int SslSocket::SendSecBuffer(const SecBuffer& secBuffer) {

    size_t stSent = PlainTextSocketSend(secBuffer.pvBuffer, secBuffer.cbBuffer);
    if (stSent != secBuffer.cbBuffer)
        return ERROR_FAILURE;

    return OK;
}

size_t SslSocket::PlainTextSocketSend(const void* pData, size_t cbData) {

    size_t cbTotalSent = 0;
    while (cbTotalSent < cbData) {

        size_t cbSent = Socket::SocketSend((BYTE*)pData + cbTotalSent, cbData - cbTotalSent);
        if (cbSent == SOCKET_ERROR)
            return (size_t)SOCKET_ERROR;

        m_stNumBytesSent += cbSent;
        cbTotalSent += cbSent;
    }

    return cbTotalSent;
}

int SslSocket::Connect (const char* pszAddress, short siPort) {

    // TODO - Client-side SSL support
    Assert (false);
    return ERROR_NOT_IMPLEMENTED;
}

// Overridden from base Socket class - this is called on every user send
size_t SslSocket::SocketSend (const void* pData, size_t cbSend) {

    if (!m_bConnected)
        return (size_t)SOCKET_ERROR;

    // We need to send in chunks
    size_t cbTotalSent = 0;
    while(cbTotalSent < cbSend) {

        size_t cbSent = SocketSendChunk(
            (BYTE*)pData + cbTotalSent,
            min(m_cbMaxChunkSize, cbSend - cbTotalSent));
        
        if (cbSent == SOCKET_ERROR)
            return (size_t)SOCKET_ERROR;

        cbTotalSent += cbSent;
    }

    return cbTotalSent;
}

size_t SslSocket::SocketSendChunk(const void* pData, size_t cbPlainTextSend) {

    Assert(cbPlainTextSend <= m_cbMaxChunkSize);

    BYTE* pbIOBuffer = new BYTE[m_cbHeaderSize + cbPlainTextSend + m_cbTrailerSize];
    if (pbIOBuffer == NULL)
        return (size_t)SOCKET_ERROR;

    Algorithm::AutoDelete<BYTE> autoIOBuffer(pbIOBuffer, true);

    BYTE* pbHeader = pbIOBuffer;
    BYTE* pbChunk = pbIOBuffer + m_cbHeaderSize;
    BYTE* pbTrailer = pbChunk + cbPlainTextSend;

    memcpy(pbChunk, pData, cbPlainTextSend);

    SecBuffer secBuffers[4];

    secBuffers[0].BufferType = SECBUFFER_STREAM_HEADER;
    secBuffers[0].cbBuffer = m_cbHeaderSize;
    secBuffers[0].pvBuffer = pbHeader;

    secBuffers[1].BufferType = SECBUFFER_DATA;
    secBuffers[1].cbBuffer = cbPlainTextSend;
    secBuffers[1].pvBuffer = pbChunk;

    secBuffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
    secBuffers[2].cbBuffer = m_cbTrailerSize;
    secBuffers[2].pvBuffer = pbTrailer;

    secBuffers[3].BufferType = SECBUFFER_EMPTY;
    secBuffers[3].cbBuffer = 0;
    secBuffers[3].pvBuffer = NULL;

    SecBufferDesc secBuffer;
    secBuffer.ulVersion = SECBUFFER_VERSION;
    secBuffer.cBuffers = countof(secBuffers);
    secBuffer.pBuffers = secBuffers;

    size_t cbSent;
    SECURITY_STATUS ss = EncryptMessage(&m_ctxHandle, 0, &secBuffer, 0);
    switch (ss) {

    case SEC_E_OK:
        {

        Assert(secBuffers[0].BufferType == SECBUFFER_STREAM_HEADER);
        Assert(secBuffers[1].BufferType == SECBUFFER_DATA);
        Assert(secBuffers[2].BufferType == SECBUFFER_STREAM_TRAILER);
        Assert(secBuffers[3].BufferType == SECBUFFER_EMPTY);

        size_t cbCypherSend = secBuffers[0].cbBuffer + secBuffers[1].cbBuffer + secBuffers[2].cbBuffer;
        cbSent = PlainTextSocketSend(pbIOBuffer, cbCypherSend);
        if (cbSent != SOCKET_ERROR) {
            Assert(cbSent == cbCypherSend);
            cbSent = cbPlainTextSend;
        }

        }
        break;

    default:

        //printf("EncryptMessage returned 0x%x\n", ss);

        cbSent = (size_t)SOCKET_ERROR;
        break;
    }

    return cbSent;
}

size_t SslSocket::SocketRecv (void* pData, size_t cbData) {

    // If we have some plaintext left over, just return that
    if (m_cbLeftoverPlainText > 0) {

        size_t cbMin = min(m_cbLeftoverPlainText, cbData);
        memcpy(pData, m_pLeftoverPlainText, cbMin);

        if (m_cbLeftoverPlainText > cbMin) {
            memmove(m_pLeftoverPlainText, m_pLeftoverPlainText + cbMin, m_cbLeftoverPlainText - cbMin);
        } else {
            delete [] m_pLeftoverPlainText;
            m_pLeftoverPlainText = NULL;
            m_cbLeftoverPlainText = 0;
        }

        return cbMin;
    }

    if (!m_bConnected)
        return (size_t)SOCKET_ERROR;

    while (true) {

        BYTE* pbCypherText;
        size_t cbCypherText;

        if (m_cbIncompleteData == 0) {

            // Nothing in the buffer...  Recv some more
            pbCypherText = (BYTE*)pData;
            cbCypherText = Socket::SocketRecv(pData, cbData);
            if (cbCypherText == SOCKET_ERROR || cbCypherText == 0)
                return (size_t)SOCKET_ERROR;

        } else {

            // Take ownership of the incomplete data buffer
            pbCypherText = m_pIncompleteData;
            cbCypherText = m_cbIncompleteData;

            m_pIncompleteData = NULL;
            m_cbIncompleteData = 0;
        }

        size_t cbDecrypted;
        int iErrCode = DecryptMessageWrapper(pbCypherText, cbCypherText, &cbDecrypted);
        switch (iErrCode) {

        case OK:
            {

            // If the cyphertext we decrypted was in the user's buffer, we're done
            if (pbCypherText == pData)
                return cbDecrypted;

            // Otherwise, we need to copy it over.
            // We may have some plaintext left over for the next user Recv
            size_t cbMin = min(cbDecrypted, cbData);
            memcpy(pData, pbCypherText, cbMin);

            if (cbDecrypted > cbMin) {

                Assert(m_pLeftoverPlainText == NULL);
                m_pLeftoverPlainText = new BYTE[cbDecrypted - cbMin];
                if (m_pLeftoverPlainText == NULL)
                    return (size_t)SOCKET_ERROR;

                m_cbLeftoverPlainText = cbDecrypted - cbMin;
                memcpy(m_pLeftoverPlainText, pbCypherText + cbMin, m_cbLeftoverPlainText);
            }

            // Delete the old incomplete data buffer
            delete [] pbCypherText;

            return cbMin;

            }
            break;

        case WARNING:
            {

            // Need to recv more for the incomplete data to make sense
            Assert(cbCypherText < m_cbMaxChunkSize);
            BYTE* pbChunk = new BYTE[m_cbMaxChunkSize];
            if (pbChunk == NULL)
                return (size_t)SOCKET_ERROR;

            if (cbCypherText > 0)
                memcpy(pbChunk, pbCypherText, cbCypherText);

            // Receive as large a chunk as possible
            size_t cbRecvd = Socket::SocketRecv(
                pbChunk + cbCypherText, 
                m_cbMaxChunkSize - cbCypherText);
            
            if (cbRecvd == SOCKET_ERROR || cbRecvd == 0)
                return (size_t)SOCKET_ERROR;

            // Delete the old incomplete data buffer
            if (pbCypherText != pData)
                delete [] pbCypherText;

            // Put the buffer back into the incomplete data buffer
            m_pIncompleteData = pbChunk;
            m_cbIncompleteData = cbCypherText + cbRecvd;

            }
            break;

        default:

            // Delete the old incomplete data buffer
            if (pbCypherText != pData)
                delete [] pbCypherText;

            return (size_t)SOCKET_ERROR;
        }
    }    
}

int SslSocket::DecryptMessageWrapper(BYTE* pbData, size_t cbCypherData, size_t* pcbPlainData) {

    int iErrCode = OK;

    Assert(cbCypherData > 0);
    *pcbPlainData = 0;

    SecBuffer secBuffers[4];
    secBuffers[0].BufferType = SECBUFFER_DATA;
    secBuffers[0].cbBuffer = cbCypherData;
    secBuffers[0].pvBuffer = pbData;

    for (int i = 1; i < countof(secBuffers); i ++) {
        secBuffers[i].BufferType = SECBUFFER_EMPTY;
        secBuffers[i].cbBuffer = 0;
        secBuffers[i].pvBuffer = NULL;
    }

    SecBufferDesc secBuffer;
    secBuffer.ulVersion = SECBUFFER_VERSION;
    secBuffer.cBuffers = countof(secBuffers);
    secBuffer.pBuffers = secBuffers;

    SECURITY_STATUS ss = DecryptMessage(&m_ctxHandle, &secBuffer, 0, NULL);
    switch (ss) {

    case SEC_E_OK:
    case SEC_I_CONTEXT_EXPIRED:
        {

        SecBuffer* pDataBuffer = NULL, * pExtraBuffer = NULL;
        for(int i = 1; i < countof(secBuffers); i ++) {

            if(secBuffers[i].BufferType == SECBUFFER_DATA) {
                Assert (pDataBuffer == NULL);
                pDataBuffer = secBuffers + i;
            }
            
            if(secBuffers[i].BufferType == SECBUFFER_EXTRA) {
                Assert (pExtraBuffer == NULL);
                pExtraBuffer = secBuffers + i;
            }
        }

        if (pDataBuffer == NULL) {
            *pcbPlainData = 0;
        } else {

            Assert(cbCypherData >= pDataBuffer->cbBuffer);
            *pcbPlainData = pDataBuffer->cbBuffer;
            
            if ((void*)pbData != pDataBuffer->pvBuffer)
                memmove(pbData, pDataBuffer->pvBuffer, pDataBuffer->cbBuffer);
        }

        // Copy any 'extra' data into the incomplete buffer
        if (pExtraBuffer != NULL) {

            Assert(m_pIncompleteData == NULL);
            Assert(pExtraBuffer->cbBuffer > 0);
            Assert(pExtraBuffer->pvBuffer != NULL);

            m_pIncompleteData = new BYTE[pExtraBuffer->cbBuffer];
            if (m_pIncompleteData == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
            } else {
                m_cbIncompleteData = pExtraBuffer->cbBuffer;
                memcpy(m_pIncompleteData, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
            }
        }

        }
        break;

    case SEC_E_INCOMPLETE_MESSAGE:
        // Tell the caller to try again with more data
        iErrCode = WARNING;
        break;

    case SEC_I_RENEGOTIATE:
        // Not supported
        Assert(false);
    default:

        // printf("DecryptMessage returned 0x%x\n", ss);

        iErrCode = ERROR_FAILURE;
        break;
    }

    return iErrCode;
}

#else

#include "Library.h"

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

    Library m_hModuleSsl;
    Library m_hModuleLib;
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

        // Load libraries
        if (m_hModuleLib.Open("libeay32.dll") != OK) {
            return;
        }

        if (m_hModuleSsl.Open("ssleay32.dll") != OK) {
            return;
        }

        // Initialize
        FXN_OpenSSL_add_all_algorithms Dyn_OpenSSL_add_all_algorithms = (FXN_OpenSSL_add_all_algorithms) 
            m_hModuleLib.GetExport ("OPENSSL_add_all_algorithms_noconf");

        if (Dyn_OpenSSL_add_all_algorithms == NULL) {
            return;
        }

        FXN_SSL_library_init Dyn_SSL_library_init = (FXN_SSL_library_init) 
            m_hModuleSsl.GetExport ("SSL_library_init");

        if (Dyn_OpenSSL_add_all_algorithms == NULL ||
            Dyn_SSL_library_init == NULL) {
            return;
        }

        //SSL_load_error_strings();
        Dyn_SSL_library_init();
        Dyn_OpenSSL_add_all_algorithms();

        // Get all proc addresses
        Dyn_SSL_CTX_new = (FXN_SSL_CTX_new) m_hModuleSsl.GetExport ("SSL_CTX_new");
        Dyn_SSL_CTX_free = (FXN_SSL_CTX_free) m_hModuleSsl.GetExport ("SSL_CTX_free");
        Dyn_SSL_CTX_check_private_key = (FXN_SSL_CTX_check_private_key) m_hModuleSsl.GetExport ("SSL_CTX_check_private_key");
        Dyn_SSL_CTX_use_certificate_file = (FXN_SSL_CTX_use_certificate_file) m_hModuleSsl.GetExport ("SSL_CTX_use_certificate_file");
        Dyn_SSL_CTX_use_PrivateKey_file = (FXN_SSL_CTX_use_PrivateKey_file) m_hModuleSsl.GetExport ("SSL_CTX_use_PrivateKey_file");
        Dyn_SSL_server_method = (FXN_SSL_server_method) m_hModuleSsl.GetExport ("SSLv23_server_method");
        Dyn_SSL_new = (FXN_SSL_new) m_hModuleSsl.GetExport ("SSL_new");
        Dyn_SSL_free = (FXN_SSL_free) m_hModuleSsl.GetExport ("SSL_free");
        Dyn_SSL_accept = (FXN_SSL_accept) m_hModuleSsl.GetExport ("SSL_accept");
        Dyn_SSL_set_fd = (FXN_SSL_set_fd) m_hModuleSsl.GetExport ("SSL_set_fd");
        Dyn_SSL_read = (FXN_SSL_read) m_hModuleSsl.GetExport ("SSL_read");
        Dyn_SSL_write = (FXN_SSL_write) m_hModuleSsl.GetExport ("SSL_write");
        Dyn_ERR_get_error = (FXN_ERR_get_error) m_hModuleLib.GetExport ("ERR_get_error");
        Dyn_ERR_error_string_n = (FXN_ERR_error_string_n) m_hModuleLib.GetExport ("ERR_error_string_n");

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

    if (!Ssl.IsEnabled()) {
        return ERROR_FAILURE;
    }

    SSL_METHOD* pMethod = SSL_server_method();
    if (pMethod == NULL) {
        return ERROR_FAILURE;
    }

    SSL_CTX* pCtx = SSL_CTX_new (pMethod);
    if (pCtx == NULL) {
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

#endif