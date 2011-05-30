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

#if !defined(AFX_SOCKET_H__E0FAE59D_A3F7_11D1_9C48_0060083E8062__INCLUDED_)
#define AFX_SOCKET_H__E0FAE59D_A3F7_11D1_9C48_0060083E8062__INCLUDED_

#include <windows.h>

#include "OS.h"

#define NET_ADDRESS_SIZE 128

class OSAL_EXPORT Socket {

public:

	Socket();
	~Socket();

	static void FreeSocket (Socket* pSocket);

	// Initialize the socket
	int Open();

	// Assign a socket to listen as the given port for a connection
	int Listen (short siPort);
	
	// After listening, wait until a connection is made and return a new socket object
	// that can be used to send data to and recv data from the client
	Socket* Accept();
	int Accept (Socket* pSocket);

	// Connect to a computer at a given port.  The address can be an 
	// ip address ("127.0.0.1") or a domain name ("www.cs.cornell.edu").  
	// Domain names trigger DNS lookups, which may be slow
	int Connect (const char* pszAddress, short siPort);

	// Return the IP address of the computer the socket is connected to.
	// '\0' if not connected
	const char* GetTheirIP();

	// Return the domain name of the computer the socket is connected to.
	// This function does a reverse DNS lookup so it may be very slow
	// '\0' if not connected
	const char* GetTheirDomainName();

	// Return the port currently being used by the socket.  -1 if not connected
	short GetPort();

	// Send a null terminated string
	int Send (const char* pszData, size_t* pstNumBytesSent);

	// Send iNumBytes of data from the start of the given buffer
	int Send (const void* pData, size_t stNumBytes, size_t* pstNumBytesSent);

	// Send primitives
	int Send (int iData, size_t* pstNumBytesSent);
	int Send (unsigned int iData, size_t* pstNumBytesSent);
	int Send (float fData, size_t* pstNumBytesSent);

	// Receive iNumBytes of data and store it in the given buffer
	// (which must be pre-allocated).
	// *piNumBytesRecvd returns with the number of bytes actually received.
	int Recv (void* pData, size_t stNumBytes, size_t* pstNumBytesRecvd);

	// Same as Recv, but doesn't remove data from the buffer
	int Peek (void* pData, size_t stNumBytes, size_t* pstNumBytesPeeked);

	// Close the socket and return its resources to the OS
	int Close();

	// Return true if the socket hasn't been closed yet
	bool IsConnected();

	// Set the socket to block when recv'ing (this is the default behavior)
	int SetBlockingMode();
	
	// Set the socket to not block when recv-ing
	int SetNonBlockingMode();

	// Set the timeout on recv, in milliseconds
	int SetRecvTimeOut (MilliSeconds iTimeOut);

	// Get num bytes sent and recvd
	size_t GetNumBytesSent();
	size_t GetNumBytesReceived();

	// Reset the byte counts
	void ResetStatistics();

	// Return the last error noticed by the OS sockets library
	static int GetLastError();

	// Return an IP address, given a host name
	static int GetIPAddressFromHostName (const char* pszHostName, char* pszIP, size_t stLen);

	// Return a host name, given an IP address
	static int GetHostNameFromIPAddress (const char* pszIP, char* pszHostName, size_t stLen);

	// Return our host name
	static int GetOurHostName (char* pszHostName, size_t stLength);

	// Initialize the OS sockets library
	static int Initialize();
	
	// Close down the OS sockets library
	static int Finalize();

	/*
	int SendWithRunLengthCompression (void* pData, size_t stDataLength);
	int RecvWithRunLengthCompression (char* pszData, size_t stNumBytes, 
		size_t* pstNumBytesRecvd);
	int GetSizeOfRunLengthCompressionBuffer (size_t* pstSize);
	*/

private:

	// Internal socket
	SOCKET m_Socket;
	
	// Ports
	short m_siPort;

	// Addresses
	struct sockaddr_in m_saOurAddr;
	struct sockaddr_in m_saTheirAddr;

	// Their addresses
	char m_pszTheirIP [NET_ADDRESS_SIZE];
	char m_pszTheirHostName [NET_ADDRESS_SIZE];

	// Accounting
	size_t m_stNumBytesSent;
	size_t m_stNumBytesRecvd;
};

#endif // !defined(AFX_SOCKET_H__E0FAE59D_A3F7_11D1_9C48_0060083E8062__INCLUDED_)