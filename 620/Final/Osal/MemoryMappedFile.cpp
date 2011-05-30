// MemoryMappedFile.cpp: implementation of the MemoryMappedFile class.
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
#include "MemoryMappedFile.h"
#undef OSAL_BUILD

#ifdef __LINUX__
#include "File.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MemoryMappedFile::MemoryMappedFile() {

#ifdef __WIN32__
    m_hMappedFile = NULL;
#endif

    m_hFile = INVALID_HANDLE_VALUE;

    m_pBaseAddress = NULL;
    m_stSize = 0;
    m_iFlags = 0;
}

MemoryMappedFile::~MemoryMappedFile() {

    Close();
}

bool MemoryMappedFile::IsOpen() {

#ifdef __LINUX__
	return m_hFile != INVALID_HANDLE_VALUE;
#else if defined __WIN32__
	return m_hMappedFile != NULL && m_hFile != INVALID_HANDLE_VALUE;
#endif
}

int MemoryMappedFile::Close() {

#ifdef __LINUX__

    if (m_pBaseAddress)
        munmap(m_pBaseAddress, m_stSize);

    if (m_hFile != INVALID_HANDLE_VALUE)
        close(m_hFile);

#else if defined __WIN32__

    if (m_pBaseAddress != NULL) {
        ::UnmapViewOfFile (m_pBaseAddress);
    }

    if (m_hMappedFile != INVALID_HANDLE_VALUE && m_hMappedFile != NULL) {
        ::CloseHandle (m_hMappedFile);
        m_hMappedFile = NULL;
    }

    if (m_hFile != INVALID_HANDLE_VALUE && m_hFile != NULL) {
        ::CloseHandle (m_hFile);
    }
#endif

    m_pBaseAddress = NULL;
    m_hFile = INVALID_HANDLE_VALUE;
    m_stSize = 0;
    m_iFlags = 0;

    return OK;
}

int MemoryMappedFile::Flush() {

#ifdef __LINUX__
    if (m_hFile != INVALID_HANDLE_VALUE)
		return (msync(m_pBaseAddress, m_stSize, MS_SYNC | MS_INVALIDATE) == 0) ? OK : ERROR_FAILURE;

#else if defined __WIN32__

    if (m_hMappedFile != NULL && m_hMappedFile != INVALID_HANDLE_VALUE) {
        return ::FlushViewOfFile (m_pBaseAddress, m_stSize) ? OK : ERROR_FAILURE;
    }
#endif

    return ERROR_FAILURE;
}

int MemoryMappedFile::OpenMappedFile (size_t stSize) {

#ifdef __LINUX__
	Assert (m_hFile != INVALID_HANDLE_VALUE);

    // make sure underlying file is big enough
    struct stat st;
    if (fstat(m_hFile, &st) < 0)
		return ERROR_FAILURE;

    if (st.st_size < stSize)
    {
        if (ftruncate(m_hFile, stSize) < 0)
            return ERROR_FAILURE;
    }

    m_pBaseAddress = mmap(0, stSize, m_bReadOnly ? PROT_READ : PROT_READ | PROT_WRITE, 
                          MAP_SHARED, m_hFile, 0);

    // Hmm, not very 64-bit friendly...
    if ((int) m_pBaseAddress == -1)
    {
        m_pBaseAddress = NULL;
		Close();
		return ERROR_FAILURE;
    }

	m_stSize = stSize;
	return OK;

#else if defined __WIN32__

    Assert (m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE);

    LARGE_INTEGER li;
    li.QuadPart = stSize;

    DWORD flProtect;

    if (m_iFlags & MEMMAP_READONLY) {
        flProtect = PAGE_READONLY;
    } else {
        flProtect = PAGE_READWRITE;
    }

    m_hMappedFile = ::CreateFileMapping (
        m_hFile,
        NULL,
        flProtect,
        li.HighPart,
        li.LowPart,
        NULL
        );

    if (m_hMappedFile == NULL) {
        Close();
        return ERROR_FAILURE;
    }

    DWORD dwAccess;

    if (m_iFlags & MEMMAP_READONLY) {
        dwAccess = FILE_MAP_READ;
    } else {
        dwAccess = FILE_MAP_ALL_ACCESS;
    }

    m_pBaseAddress = ::MapViewOfFile (
        m_hMappedFile,
        dwAccess,
        0,
        0,
        0
        );

    if (m_pBaseAddress == NULL) {
        Assert (false);
        Close();
        return ERROR_FAILURE;
    }

    m_stSize = stSize;
    return OK;
#endif
}


int MemoryMappedFile::OpenNew (const char* pszFileName, size_t stSize, unsigned int iFlags) {

    // Input
    if (iFlags & MEMMAP_READONLY) {
        return ERROR_INVALID_ARGUMENT;
    }

    m_iFlags = iFlags;

    // Align on 8 byte boundary
    m_stSize = (stSize + 7) & ~7;

#ifdef __LINUX__

	m_hFile = open(pszFileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (m_hFile < 0)
    {
        m_hFile = INVALID_HANDLE_VALUE;
		return ERROR_FAILURE;
    }

#else if defined __WIN32__

    DWORD dwFlags = 0;

    if (iFlags & MEMMAP_WRITETHROUGH) {
        dwFlags |= FILE_FLAG_WRITE_THROUGH;
    }

    m_hFile = ::CreateFile (
        pszFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        dwFlags,
        NULL
        );

    if (m_hFile == INVALID_HANDLE_VALUE) {
        return ERROR_FAILURE;
    }
#endif

    return OpenMappedFile (m_stSize);
}

int MemoryMappedFile::OpenExisting (const char* pszFileName, unsigned int iFlags) {

    m_iFlags = iFlags;

#ifdef __LINUX__

	m_hFile = open(pszFileName, O_RDWR);
    if (m_hFile < 0)
		return ERROR_FAILURE;

    int iErrCode = File::GetFileSize (pszFileName, &m_stSize);
    if (iErrCode != OK)
        return iErrCode;

	return OpenMappedFile (m_stSize, iFlags);

#else if defined __WIN32__

    DWORD dwAccess = GENERIC_READ;
    DWORD dwFlags = 0;

    if (!(iFlags & MEMMAP_READONLY)) {
        dwAccess |= GENERIC_WRITE;
    }

    if (iFlags & MEMMAP_WRITETHROUGH) {
        dwFlags |= FILE_FLAG_WRITE_THROUGH;
    }

    m_hFile = ::CreateFile (
        pszFileName,
        dwAccess,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        dwFlags,
        NULL
        );

    if (m_hFile == INVALID_HANDLE_VALUE) {
        if (::GetLastError() == ERROR_SHARING_VIOLATION) {
            return WARNING;
        } else {
            return ERROR_FAILURE;
        }
    }

    LARGE_INTEGER li;
    if (!::GetFileSizeEx (m_hFile, &li)) {
        return ERROR_FAILURE;
    }

    return OpenMappedFile ((size_t) li.QuadPart);

#endif
}

int MemoryMappedFile::Resize (size_t stNewSize) {

#ifdef __LINUX__
    munmap(m_pBaseAddress, m_stSize);
#else if defined __WIN32__
	::UnmapViewOfFile (m_pBaseAddress);
	::CloseHandle (m_hMappedFile);
    m_hMappedFile = NULL;
#endif
    m_pBaseAddress = NULL;

    // Align on 8 byte boundary
    m_stSize = (stNewSize + 7) & ~7;
        
    return OpenMappedFile (m_stSize);
}

int MemoryMappedFile::Expand (size_t stIncrement) {
    return Resize (m_stSize + stIncrement);
}

size_t MemoryMappedFile::GetSize() const {
    return m_stSize;
}

void* MemoryMappedFile::GetAddress() {
    return m_pBaseAddress;
}

void* MemoryMappedFile::GetEndOfFile() const {
    return (char*) m_pBaseAddress + m_stSize;
}