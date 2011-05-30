// Socket.cpp: implementation of the Socket class.
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

#define OSAL_BUILD
#include "Socket.h"
#undef OSAL_BUILD

#ifdef __LINUX__
#define SOCKET_ERROR -1
#define SD_SEND 1
#define SD_BOTH 2
#include <fcntl.h>
#endif

#ifdef __WIN32__
// Defined in winsock2.h.
// However, including this header doesn't bring in these definitions.  Odd.
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02
#endif

//#define RECV_LOGGING
#undef  RECV_LOGGING

//#define SEND_LOGGING
#undef  SEND_LOGGING

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Socket::Socket() {

    // Set socket and port numbers to invalid values
    m_Socket = INVALID_SOCKET;
    m_siPort = -1;

    // Set addresses structures to be all zeros
    memset ((void*) &m_saOurAddr, '\0', sizeof (m_saOurAddr));
    memset ((void*) &m_saTheirAddr, '\0', sizeof (m_saTheirAddr));

    *m_pszTheirIP = '\0';
    *m_pszTheirHostName = '\0';

    m_stNumBytesSent = 0;
    m_stNumBytesRecvd = 0;
}

Socket::~Socket() {

    Close();
}

int Socket::Initialize() {

#ifdef __WIN32__
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD (2, 0);
    if (WSAStartup (wVersionRequested, &wsaData) != 0) {
        return ERROR_FAILURE;
    }
    
    if (LOBYTE (wsaData.wVersion) < 2 || HIBYTE (wsaData.wVersion) != 0) {
        WSACleanup();
        return ERROR_FAILURE;
    }
#endif
    return OK;
}

int Socket::Finalize() {
#ifdef __WIN32__
    return (WSACleanup() == SOCKET_ERROR) ? ERROR_FAILURE : OK;
#else if defined __LINUX__
    return OK;
#endif
}

int Socket::Open() {
    
    if ((m_Socket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        return ERROR_FAILURE;
    }   
    return OK;
}

int Socket::Listen (short siPort) {

    // Set our port number
    m_siPort = siPort;
    
    // Configure our address
    m_saOurAddr.sin_family = AF_INET;
    m_saOurAddr.sin_port = htons (m_siPort);
    m_saOurAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind to address
    if (bind (m_Socket, (struct sockaddr*) &m_saOurAddr, sizeof (m_saOurAddr)) == SOCKET_ERROR) {
        return ERROR_FAILURE;
    }

    // Begin listening
    if (listen (m_Socket, SOMAXCONN) == SOCKET_ERROR) {
        return ERROR_FAILURE;
    }

    return OK;
}

int Socket::Accept (Socket* pSocket) {

    // Accept the connection
#ifdef __LINUX__
	socklen_t iAddrLen = (socklen_t) sizeof (m_saTheirAddr);
#else if defined __WIN32__
	int iAddrLen = sizeof (m_saTheirAddr);
#endif

    SOCKET socket = accept (
        m_Socket,
        (struct sockaddr*) &(pSocket->m_saTheirAddr), 
        &iAddrLen
        );

    if (socket == INVALID_SOCKET) {
        return ERROR_FAILURE;
    }

    pSocket->m_Socket = socket;

    // Get their IP address
    strcpy (pSocket->m_pszTheirIP, inet_ntoa (pSocket->m_saTheirAddr.sin_addr));

    return OK;
}

Socket* Socket::Accept() {

    // Create a new Socket for the incoming connection
    Socket* pSocket = new Socket();
    if (pSocket == NULL) {
        return NULL;
    }

    if (Accept (pSocket) != OK) {
        delete pSocket;
        return NULL;
    }

    return pSocket;
}

int Socket::Negotiate() {
    return OK;
}

void Socket::FreeSocket (Socket* pSocket) {

    Assert (pSocket != NULL);
    delete pSocket;
}

int Socket::Connect (const char* pszAddress, short siPort) {

    if (pszAddress == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    ResetStatistics();

    unsigned long ulAddr;
    
    ulAddr = inet_addr (pszAddress);

    if (ulAddr != INADDR_NONE) {
        strcpy (m_pszTheirIP, pszAddress);
    } else {

        // A name was probably given, so perform a DNS lookup
        int iErrCode = GetIPAddressFromHostName (pszAddress, m_pszTheirIP, sizeof (m_pszTheirIP));
        if (iErrCode != OK) {
            return iErrCode;
        }
        ulAddr = inet_addr (m_pszTheirIP);
    }

    m_saTheirAddr.sin_family = AF_INET;
    m_saTheirAddr.sin_port = htons (siPort);
#ifdef __LINUX__
	m_saTheirAddr.sin_addr.s_addr = ulAddr;
#else if defined __WIN32__
	m_saTheirAddr.sin_addr.S_un.S_addr = ulAddr;
#endif

    return (connect (m_Socket, (struct sockaddr*) &m_saTheirAddr, sizeof (m_saTheirAddr)) == SOCKET_ERROR) ? ERROR_FAILURE : OK;
}

int Socket::Recv (void* pData, size_t stNumBytes, size_t* pstNumBytesRecvd) {

    *pstNumBytesRecvd = 0;

    Assert (stNumBytes == (size_t) (int) stNumBytes);
    size_t stNumBytesRecvd = SocketRecv (pData, stNumBytes);

    if (stNumBytesRecvd == SOCKET_ERROR || stNumBytesRecvd == 0) {
        return ERROR_FAILURE;
    }

#ifdef RECV_LOGGING

    char* pszData = (char*) StackAlloc (stNumBytesRecvd + 1);
    memcpy (pszData, pData, stNumBytesRecvd);
    pszData [stNumBytesRecvd] = '\0';

    FILE* pFile = fopen ("C:\\Temp\\In.txt", "a");
    if (pFile != NULL) {
        fprintf (pFile, pszData);
        fclose (pFile);
    }
#endif

    m_stNumBytesRecvd += stNumBytesRecvd;
    *pstNumBytesRecvd = stNumBytesRecvd;

    return OK;
}

int Socket::Send (const char* pszData, size_t* pstNumBytesSent) {

    return Send (pszData, strlen (pszData), pstNumBytesSent);
}

int Socket::Send (const void* pData, size_t stNumBytes, size_t* pstNumBytesSent) {

    *pstNumBytesSent = 0;

    size_t stNumBytesSent, stTotalNumBytesSent;

    Assert (stNumBytes == (size_t) (int) stNumBytes);
    stTotalNumBytesSent = stNumBytesSent = SocketSend (pData, stNumBytes);
    if (stNumBytesSent == SOCKET_ERROR) {
        return ERROR_FAILURE;
    }

    m_stNumBytesSent += stNumBytesSent;

#ifdef SEND_LOGGING
    char* pszPrintData = new char [stNumBytesSent + 1];
    if (pszPrintData != NULL) {
    
        memcpy (pszPrintData, pData, stNumBytesSent);
        pszPrintData [stNumBytesSent] = '\0';

        FILE* pFile = fopen ("C:\\Temp\\Out.txt", "a");
        if (pFile != NULL) {
            fwrite (pszPrintData, sizeof (char), stNumBytesSent, pFile);
            fclose (pFile);
        }
        delete [] pszPrintData;
    }
#endif
    
    if (stNumBytesSent < stNumBytes) {
        
        size_t stCurrentIndex = stNumBytesSent;
        
        Assert (stNumBytes == (size_t) (int) stNumBytes);

        do {
            
            stNumBytesSent = SocketSend ((char*) pData + stCurrentIndex, stNumBytes - stTotalNumBytesSent);
            if (stNumBytesSent == SOCKET_ERROR) {
                return ERROR_FAILURE;
            }
            
            m_stNumBytesSent += stNumBytesSent;
            stTotalNumBytesSent += stNumBytesSent;
            stCurrentIndex += stNumBytesSent;
            
        } while (stNumBytesSent < stNumBytes);
    }

    *pstNumBytesSent = stTotalNumBytesSent;

    return OK;
}

size_t Socket::SocketSend (const void* pData, size_t cbSend) {
    return send (m_Socket, (const char*) pData, (int) cbSend, 0);
}

size_t Socket::SocketRecv (void* pData, size_t cbRecv) {
    return recv (m_Socket, (char*) pData, (int) cbRecv, 0);
}

int Socket::Send (int iData, size_t* pstNumBytesSent) {

    char pszData[64];
#ifdef __LINUX__
    snprintf(pszData, sizeof(pszData), "%d", iData);
#else if defined __WIN32__
	_itoa (iData, pszData, 10);
#endif

    return Send (pszData, strlen (pszData), pstNumBytesSent);
}

int Socket::Send (unsigned int iData, size_t* pstNumBytesSent) {

    char pszData[64];
    _ultoa (iData, pszData, 10);

    return Send (pszData, strlen (pszData), pstNumBytesSent);
}

int Socket::Send (float fData, size_t* pstNumBytesSent) {

    char pszData[64];
    sprintf (pszData, "%f", fData);

    return Send (pszData, strlen (pszData), pstNumBytesSent);
}

const char* Socket::GetTheirIP() {
    return m_pszTheirIP;
}

const char* Socket::GetTheirDomainName() {
    
    if (*m_pszTheirHostName == '\0' && 
        GetHostNameFromIPAddress (m_pszTheirIP, m_pszTheirHostName, sizeof (m_pszTheirHostName)) != OK) {
        strcpy (m_pszTheirHostName, m_pszTheirIP);
    }

    return m_pszTheirHostName;
}

short Socket::GetPort() {
    return m_siPort;
}

int Socket::Close() {

    if (m_Socket == INVALID_SOCKET) {
        return OK;
    }

    // Set the socket to linger on close until unsent data is sent
    struct linger lLinger;
    lLinger.l_linger = 15;
    lLinger.l_onoff = 1;
    if (setsockopt (m_Socket, SOL_SOCKET, SO_LINGER, (char*) &lLinger, sizeof (linger)) == SOCKET_ERROR) {
#ifdef __LINUX__
        close(m_Socket);
#else if defined __WIN32__
		closesocket (m_Socket);
#endif
        m_Socket = INVALID_SOCKET;
        return ERROR_FAILURE;
    }
    
    // Shutdown sends
    shutdown (m_Socket, SD_SEND);

    // Recv until socket closes
    size_t stNumBytesRecvd;
    char pszBuf [512];
    while (true) {

        stNumBytesRecvd = Socket::SocketRecv (pszBuf, sizeof (pszBuf));
        if (stNumBytesRecvd == 0 || stNumBytesRecvd == SOCKET_ERROR) {
            break;
        }
        OS::Sleep();
    }

    // Shutdown sends and recv's
    shutdown (m_Socket, SD_BOTH);

    // Close down the socket
#ifdef __LINUX__
	stNumBytesRecvd = close(m_Socket);
#else if defined __WIN32__
	stNumBytesRecvd = closesocket (m_Socket);
#endif

    // Set socket and port to invalid values
    m_Socket = INVALID_SOCKET;
    m_siPort = 0;

    // Set our address to be all zeros
    memset (&m_saOurAddr, 0, sizeof (m_saOurAddr));

    // Set their address to be all zeros
    memset (&m_saTheirAddr, 0, sizeof (m_saTheirAddr));

    *m_pszTheirIP = '\0';
    *m_pszTheirHostName = '\0';

    return stNumBytesRecvd == SOCKET_ERROR ? ERROR_FAILURE : OK;
}

int Socket::SetKeepAlive (bool bKeepAlive) {

    BOOL bFlag = bKeepAlive ? TRUE : FALSE;
    return setsockopt (m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char*) &bFlag, sizeof (bFlag)) ?
        OK : ERROR_FAILURE;
}

int Socket::SetRecvTimeOut (MilliSeconds iTimeOut) {

    return setsockopt (m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &iTimeOut, sizeof (iTimeOut)) == 0 ? 
        OK : ERROR_FAILURE;
}

int Socket::SetSendTimeOut (MilliSeconds iTimeOut) {

    return setsockopt (m_Socket, SOL_SOCKET, SO_SNDTIMEO, (char*) &iTimeOut, sizeof (iTimeOut)) == 0 ? 
        OK : ERROR_FAILURE;
}

size_t Socket::GetNumBytesSent() {
    return m_stNumBytesSent;
}

size_t Socket::GetNumBytesReceived() {
    return m_stNumBytesRecvd;
}

void Socket::ResetStatistics() {

    m_stNumBytesSent = 0;
    m_stNumBytesRecvd = 0;
}

bool Socket::IsConnected() {
    return (m_Socket != INVALID_SOCKET);
}

int Socket::SetBlockingMode() {
#ifdef __LINUX__
    long flags = fcntl(m_Socket, F_GETFL, 0);
    return (fcntl(m_Socket, F_SETFL, flags & ~O_NONBLOCK) == 0) ? OK : ERROR_FAILURE;
#else if defined __WIN32__
	unsigned long lZero = 0;
	return (ioctlsocket (m_Socket, FIONBIO, &lZero) == 0) ? OK : ERROR_FAILURE;
#endif
}

int Socket::SetNonBlockingMode() {
#ifdef __LINUX__
    long flags = fcntl(m_Socket, F_GETFL, 0);
    return (fcntl(m_Socket, F_SETFL, flags | O_NONBLOCK) == 0) ? OK : ERROR_FAILURE;
#else if defined __WIN32__
	unsigned long lOne = 1;
	return (ioctlsocket (m_Socket, FIONBIO, &lOne) == 0) ? OK : ERROR_FAILURE;
#endif
}

int Socket::GetLastError() {
#ifdef __LINUX__
    return errno;
#else if defined __WIN32__
    return WSAGetLastError();
#endif
}

int Socket::GetIPAddressFromHostName (const char* pszHostName, char* pszIP, size_t stLen) {
    
    if (pszHostName == NULL || pszIP == NULL || stLen < 16) {
        return ERROR_INVALID_ARGUMENT;
    }

    unsigned long ulAddr;
    struct hostent* pheName;
    if ((pheName = gethostbyname (pszHostName)) == NULL) {
        return ERROR_FAILURE;
    }
    
    if ((ulAddr = *((long*) (pheName->h_addr_list[0]))) == INADDR_NONE) {
        return ERROR_FAILURE;
    }
    
    sprintf (pszIP, "%i.%i.%i.%i", (unsigned char) pheName->h_addr_list[0][0], 
        (unsigned char) pheName->h_addr_list[0][1], (unsigned char) pheName->h_addr_list[0][2], 
        (unsigned char) pheName->h_addr_list[0][3]);

    return OK;
}

int Socket::GetHostNameFromIPAddress (const char* pszIP, char* pszHostName, size_t stLen) {

    if (pszIP == NULL || pszHostName == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    unsigned long ulAddr = inet_addr (pszIP);

    // Perform a reverse DNS lookup
    struct hostent* heName = gethostbyaddr ((char*) &ulAddr, sizeof (struct in_addr), AF_INET);

    // If nothing came back, use the IP address as the host name
    if (heName != NULL) {

        size_t stHeNameLen = strlen (heName->h_name);
        
        if (stHeNameLen >= stLen) {
            return ERROR_INVALID_ARGUMENT;
        }
        memcpy (pszHostName, heName->h_name, stHeNameLen + 1);
        return OK;
    }

    return ERROR_FAILURE;
}


int Socket::GetOurHostName (char* pszAddress, size_t stLength) {
    
    if (pszAddress == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }

    Assert (stLength == (size_t) (int) stLength);
    int iErrCode = gethostname (pszAddress, (int) stLength);  
    if (iErrCode == SOCKET_ERROR) {

#ifdef __LINUX__
        if (errno == EFAULT)
            return ERROR_INVALID_ARGUMENT;
#else if defined __WIN32__       
        if (GetLastError() == WSAEFAULT) {
            return ERROR_INVALID_ARGUMENT;
        }
#endif
        return ERROR_FAILURE;
    }

    return OK;
}

int Socket::Select (SocketSet* pSelectSet) {

    unsigned int i, j;

    fd_set set;
    set.fd_count = pSelectSet->iNumSockets;
    
    if (pSelectSet->iNumSockets > FD_SETSIZE) {
        return ERROR_INVALID_ARGUMENT;
    }

    for (i = 0; i < set.fd_count; i ++) {
        set.fd_array[i] = pSelectSet->pSockets[i]->m_Socket;
    }

    int ret = select (0, &set, NULL, NULL, NULL);
    if (ret == SOCKET_ERROR) {
        return ERROR_FAILURE;
    }

    Socket* pSockets [FD_SETSIZE];
    for (i = 0; i < set.fd_count; i ++) {

        SOCKET socket = set.fd_array[i];
        for (j = 0; j < pSelectSet->iNumSockets; j ++) {

            Socket* pSocket = pSelectSet->pSockets[j];
            SOCKET socketSocket = pSocket->m_Socket;

            if (socketSocket == INVALID_SOCKET) {
                return ERROR_FAILURE;
            }

            if (socket == socketSocket) {
                pSockets[i] = pSocket;
                break;
            }
        }

        Assert (j < pSelectSet->iNumSockets);
    }

    pSelectSet->iNumSockets = set.fd_count;
    memcpy (pSelectSet->pSockets, pSockets, set.fd_count * sizeof (Socket*));

    return OK;
}

/*
enum RunLengthCompressionState { COMPRESSED = -1, UNCOMPRESSED = -2, REMAINDER = -3, NONE = -4 };
typedef int T;
#define COMPRESSION_BUFFER_SIZE 32768
#define PARTITION_SIZE 8192

int Socket::SendWithRunLengthCompression (void* pData, size_t stNumBytes) {

    // If data is less than the type chosen, just send the data
    RunLengthCompressionState rlcMessage;

    if (stNumBytes < sizeof (T)) {
        this->Send (&stNumBytes, sizeof (size_t));
        int stTemp = -1;
        this->Send (&stNumBytes, sizeof (size_t));
        return this->Send (pData, stNumBytes);
    }

    bool bCompressed;
    size_t stNumReps,
        stDataLength = (size_t) stNumBytes / sizeof (T),
        stIndex,
        stBufferIndex,
        stNumUncompressed,
        stUncompressedNumStore,
        stDataSrcIndex = 0,
        stPartitionSize,
        stTotalBytes = 0,
        stSentBytes = 0,
        stTemp;

    T ptSendBuffer [COMPRESSION_BUFFER_SIZE / sizeof (T)], * ptData, tInstance;

    // Send the size of the original buffer
    this->Send (&stNumBytes, sizeof (size_t));

    // Work
    while (stDataSrcIndex < stDataLength) {

        stPartitionSize = (size_t) min (PARTITION_SIZE / sizeof (T), stDataLength - stDataSrcIndex);
        if (stPartitionSize == 0) {
            break;
        }

        bCompressed = true;
        stNumReps = 1;
        stIndex = 1;
        stBufferIndex = 0;
        stNumUncompressed = 0;
        stUncompressedNumStore = 0xcccccccc;

        // Get the first data element
        ptData = &(((T*) pData)[stDataSrcIndex]);
        tInstance = *ptData;
        
        while (stIndex < stPartitionSize) {
            
            if (ptData[stIndex] == tInstance) {
                
                stNumReps ++;
                
            } else {
                
                if (stNumReps > 1) {
                    
                    if (stNumUncompressed > 0) {
                        ptSendBuffer [stUncompressedNumStore] = stNumUncompressed;
                        stNumUncompressed = 0;
                    }

                    ptSendBuffer [stBufferIndex] = (T) COMPRESSED;
                    ptSendBuffer [stBufferIndex + 1] = (T) stNumReps;
                    ptSendBuffer [stBufferIndex + 2] = tInstance;
                    
                    stNumReps = 1;
                    stBufferIndex += 3;
                    
                    bCompressed = true;
                    
                } else {
                    
                    if (bCompressed) {
                        ptSendBuffer [stBufferIndex] = (T) UNCOMPRESSED;
                        stUncompressedNumStore = stBufferIndex + 1;
                        stBufferIndex += 2;
                        bCompressed = false;
                    }
                    
                    ptSendBuffer [stBufferIndex] = tInstance;
                    stBufferIndex ++;
                    stNumUncompressed ++;
                }
                
                tInstance = ptData[stIndex];
            }
            
            stIndex ++;
        }
        
        // Set last uncompressed number if necessary
        if (stNumUncompressed > 0) {
            ptSendBuffer [stUncompressedNumStore] = stNumUncompressed;
        }

        // Set last rep(s)
        if (stNumReps == 1) {

            if (bCompressed) {
                stUncompressedNumStore = stBufferIndex + 1;
                ptSendBuffer [stBufferIndex] = (T) UNCOMPRESSED;
                ptSendBuffer [stUncompressedNumStore] = (T) 0;
                stBufferIndex += 2;
            }

            ptSendBuffer [stBufferIndex] = tInstance;
            ptSendBuffer [stUncompressedNumStore] ++;
            stBufferIndex ++;

        } else {

            ptSendBuffer [stBufferIndex] = (T) COMPRESSED;
            ptSendBuffer [stBufferIndex + 1] = (T) stNumReps;
            ptSendBuffer [stBufferIndex + 2] = tInstance;
            stBufferIndex += 3;
        }

        // Send partition size in T's
        stTemp = stBufferIndex * sizeof(T);

        this->Send (&stTemp, sizeof (size_t));
        this->Send (ptSendBuffer, stTemp);

        stDataSrcIndex += PARTITION_SIZE / sizeof (T);

        stTotalBytes += stPartitionSize * sizeof (T);
        stSentBytes += stTemp;
    }

    // Send remainder partition
    size_t stRemainder = stNumBytes - stTotalBytes;
    if (stRemainder > 0) {
        rlcMessage = REMAINDER;

        stTemp = stRemainder + sizeof (RunLengthCompressionState) + sizeof (size_t);
        this->Send (&stTemp, sizeof (size_t));
        this->Send (&rlcMessage, sizeof (RunLengthCompressionState));
        this->Send (&stRemainder, sizeof (stTemp));
        this->Send (&(((char*) pData)[stTotalBytes]), stNumBytes - stTotalBytes);
    }

    return OK;
}


int Socket::RecvWithRunLengthCompression (char* pszData, size_t stNumBytes, size_t* pstNumBytesRecvd) {

    int iErrCode = OK;
    size_t stNumRealBytes = 0, stNumBytesRecvd, stPartitionSize, stNumPartitionElements,
        stCurrentDataIndex = 0, stRemainder, stLimit, i, j, stTemp;

    char pszBuffer [COMPRESSION_BUFFER_SIZE];
    T* ptBuffer = (T*) pszBuffer, * ptData = (T*) pszData, tInstance;

    RunLengthCompressionState iState = NONE;

    while (stNumRealBytes < stNumBytes) {

        // Receive the size of the partition in bytes
        if (this->Recv (&stPartitionSize, sizeof (size_t), &stTemp) != OK || stTemp != sizeof (size_t)) {
            return ERROR_FAILURE;
        }

        stNumBytesRecvd = 0;
        if (stPartitionSize == -1) {
            while (stNumBytesRecvd < stNumBytes) {
                if (this->Recv (&(pszData[stNumBytesRecvd]), 
                    min (COMPRESSION_BUFFER_SIZE, stNumBytes - stNumBytesRecvd), &stTemp) != OK) {
                    return ERROR_FAILURE;
                }
                stNumBytesRecvd += stTemp;
                
                OS::Sleep();
            }

            return OK;
        }

        while (stNumBytesRecvd < stPartitionSize) {

            if (this->Recv (&(pszBuffer[stNumBytesRecvd]), 
                min (COMPRESSION_BUFFER_SIZE, stPartitionSize - stNumBytesRecvd), &stTemp) != OK) {
                return ERROR_FAILURE;
            }
            stNumBytesRecvd += stTemp;
            OS::Sleep();
        }

        stNumPartitionElements = stPartitionSize / sizeof (T);
        stRemainder = 0;
        for (i = 0; i < stNumPartitionElements; ) {
        
            // Get a markup
            switch (ptBuffer[i]) {

            case COMPRESSED:

                stTemp = (size_t) ptBuffer[i + 1];
                tInstance = ptBuffer [i + 2];
                
                for (j = 0; j < stTemp; j ++) {
                    ptData[stCurrentDataIndex + j] = tInstance;
                }
                stCurrentDataIndex += stTemp;

                i += 3;

                break;

            case UNCOMPRESSED:

                stLimit = (size_t) ptBuffer[i + 1];

                for (j = 0; j < stLimit; j ++) {
                    ptData[stCurrentDataIndex + j] = ptBuffer[i + j + 2];
                }
                stCurrentDataIndex += stLimit;
                
                i += stLimit;
                i += 2;
                break;
    
            case REMAINDER:
                
                stRemainder = (int) ptBuffer[i + 1];
                memcpy (&(ptData[stCurrentDataIndex]), &(ptBuffer[i + 2]), stRemainder);
                i = stNumPartitionElements;
                break;
        
            default:
                Assert (false);
                return ERROR_FAILURE;
            }
        }

        stNumRealBytes = stCurrentDataIndex * sizeof (T) + stRemainder;
    }

    *pstNumBytesRecvd = stNumRealBytes;

    return OK;
}

int Socket::GetSizeOfRunLengthCompressionBuffer (size_t* pstSize) {

    size_t stRecv;
    if (this->Recv (pstSize, sizeof (size_t), &stRecv) != OK || stRecv != sizeof (size_t)) {
        *pstSize = 0;
        return ERROR_FAILURE;
    }
    
    return OK;
}

*/