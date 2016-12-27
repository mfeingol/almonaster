// AsyncFile.cpp: implementation of the AsyncFile class.
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
#include "AsyncFile.h"
#undef OSAL_BUILD

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AsyncFile::AsyncFile()
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_hReadFile = INVALID_HANDLE_VALUE;
}

AsyncFile::~AsyncFile()
{
    Close();
}

int AsyncFile::OpenAppend(const char* pszFileName)
{
    Close();

    m_hFile = CreateFileA(pszFileName, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return ERROR_FAILURE;
    }

    m_hReadFile = CreateFileA(pszFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
    if (m_hReadFile == INVALID_HANDLE_VALUE)
    {
        Close();
        return ERROR_FAILURE;
    }

    if (SetFilePointer(m_hFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
    {
        Close();
        return ERROR_FAILURE;
    }

    return OK;
}

void AsyncFile::Close()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    if (m_hReadFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hReadFile = INVALID_HANDLE_VALUE;
    }
}

void CALLBACK CompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    delete [] (char*)lpOverlapped->Pointer;
    delete lpOverlapped;
}

int AsyncFile::Write(const void* pBuffer, unsigned int cbSize)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return ERROR_FAILURE;
    }

    char* bufCopy = new char[cbSize];
    if (!bufCopy)
    {
        return ERROR_OUT_OF_MEMORY;
    }
    memcpy(bufCopy, pBuffer, cbSize);

    LPOVERLAPPED overlapped = new OVERLAPPED;
    memset(overlapped, 0, sizeof(OVERLAPPED));
    overlapped->Offset = 0xffffffff;
    overlapped->OffsetHigh = 0xffffffff;
    overlapped->Pointer = bufCopy;

    if (!WriteFileEx(m_hFile, bufCopy, cbSize, overlapped, CompletionRoutine))
    {
        return ERROR_FAILURE;
    }

    // Trigger any pending APCs; otherwise, they'll be triggered the next time the thread sleeps or blocks
    WaitForSingleObjectEx(m_hFile, 0, TRUE);

    return OK;
}

int AsyncFile::Tail(void* pBuffer, unsigned int* pcbSize)
{
    LARGE_INTEGER liSize;
    BOOL ret = GetFileSizeEx(m_hReadFile, &liSize);
    if (!ret)
    {
        return ERROR_FAILURE;
    }
    
    int64 i64Size = liSize.QuadPart;
    int64 i64Offset = *pcbSize;
    if (i64Size < i64Offset)
    {
        i64Offset = i64Size;
    }

    LARGE_INTEGER liDistanceToMove;
    liDistanceToMove.QuadPart = -i64Offset;
    if (SetFilePointerEx(m_hReadFile, liDistanceToMove, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
    {
        return ERROR_FAILURE;
    }

    DWORD cbRead;
    if (!ReadFile(m_hReadFile, pBuffer, *pcbSize, &cbRead, NULL))
    {
        return ERROR_FAILURE;
    }
    
    *pcbSize = cbRead;
    return OK;
}