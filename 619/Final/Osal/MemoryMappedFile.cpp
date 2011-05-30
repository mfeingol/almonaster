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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MemoryMappedFile::MemoryMappedFile() {

	m_hMappedFile = NULL;
	m_hFile = INVALID_HANDLE_VALUE;

	m_pBaseAddress = NULL;
	m_stSize = 0;

	m_bCopy = false;
	m_bReadOnly = false;
}

MemoryMappedFile::~MemoryMappedFile() {

	Close();
}

bool MemoryMappedFile::IsOpen() {

	return m_hMappedFile != NULL && m_hFile != INVALID_HANDLE_VALUE;
}

int MemoryMappedFile::Close() {

	if (m_pBaseAddress != NULL) {
		::UnmapViewOfFile (m_pBaseAddress);
		m_pBaseAddress = NULL;
	}

	if (m_hMappedFile != INVALID_HANDLE_VALUE && m_hMappedFile != NULL) {

		::CloseHandle (m_hMappedFile);
		m_hMappedFile = NULL;
	}

	if (!m_bCopy && m_hFile != INVALID_HANDLE_VALUE && m_hFile != NULL) {
		::CloseHandle (m_hFile);
	}

	m_hFile = INVALID_HANDLE_VALUE;
	m_stSize = 0;
	m_bReadOnly = false;

	return OK;
}

int MemoryMappedFile::Flush() {

	if (m_hMappedFile != NULL && m_hMappedFile != INVALID_HANDLE_VALUE) {
		return ::FlushViewOfFile (m_pBaseAddress, m_stSize) ? OK : ERROR_FAILURE;
	}

	return ERROR_FAILURE;
}


int MemoryMappedFile::OpenMappedFile (size_t stSize, bool bReadOnly) {

	m_bReadOnly = bReadOnly;

	Assert (m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE);

	m_hMappedFile = ::CreateFileMapping (
		m_hFile,
		NULL,
		m_bReadOnly ? PAGE_READONLY : PAGE_READWRITE,
		0,
		stSize,	// Not 64 bit compliant!
		NULL
		);

	if (m_hMappedFile == NULL) {
		Close();
		return ERROR_FAILURE;
	}

	m_pBaseAddress = ::MapViewOfFile (
		m_hMappedFile,
		m_bReadOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS,
		0,
		0,
		0
		);

	if (m_pBaseAddress == NULL) {
		Close();
		return ERROR_FAILURE;
	}

	m_stSize = stSize;
	return OK;
}


int MemoryMappedFile::OpenNew (const char* pszFileName, size_t stSize) {

	// Align on 8 byte boundary
	m_stSize = (stSize + 7) & ~7;

	m_hFile = ::CreateFile (
		pszFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_FLAG_RANDOM_ACCESS,
		NULL
		);

	if (m_hFile == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}

	return OpenMappedFile (m_stSize);
}

int MemoryMappedFile::OpenExisting (const char* pszFileName, bool bReadOnly) {

	m_hFile = ::CreateFile (
		pszFileName,
		bReadOnly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_RANDOM_ACCESS,
		NULL
		);

	if (m_hFile == INVALID_HANDLE_VALUE) {
		if (::GetLastError() == ERROR_SHARING_VIOLATION) {
			return WARNING;
		} else {
			return ERROR_FAILURE;
		}
	}

	return OpenMappedFile (::GetFileSize (m_hFile, NULL), bReadOnly);	// Not 64 bit compliant!
}

int MemoryMappedFile::Resize (size_t stNewSize) {

	if (m_bCopy) {
		return ERROR_FAILURE;
	}

	// Align on 8 byte boundary
	m_stSize = (stNewSize + 7) & ~7;
		
	::UnmapViewOfFile (m_pBaseAddress);
	::CloseHandle (m_hMappedFile);
		
	return OpenMappedFile (m_stSize);
}

int MemoryMappedFile::Expand (size_t stIncrement) {
	return Resize (m_stSize + stIncrement);
}

inline size_t MemoryMappedFile::GetSize() const {
	return m_stSize;
}

inline void* MemoryMappedFile::GetAddress() {
	return m_pBaseAddress;
}

inline void* MemoryMappedFile::GetEndOfFile() const {
	return (char*) m_pBaseAddress + m_stSize;
}

int MemoryMappedFile::CreateCopyOnWriteMapping (MemoryMappedFile* pmmfCopy) const {

	int iErrCode;

	if (m_bReadOnly) {

		iErrCode = ERROR_FAILURE;

	} else {

		iErrCode = OK;

		pmmfCopy->Close();

		pmmfCopy->m_bCopy = true;
		pmmfCopy->m_hFile = m_hFile;
		pmmfCopy->m_stSize = m_stSize;
		
		pmmfCopy->m_hMappedFile = ::CreateFileMapping (
			pmmfCopy->m_hFile,
			NULL,
			PAGE_WRITECOPY,
			0,
			pmmfCopy->m_stSize,	// Not 64 bit compliant!
			NULL
			);
		
		if (pmmfCopy->m_hMappedFile == NULL) {
			
			pmmfCopy->Close();
			iErrCode = ERROR_FAILURE;

		} else {

			pmmfCopy->m_pBaseAddress = ::MapViewOfFile (
				pmmfCopy->m_hMappedFile,
				FILE_MAP_COPY,
				0,
				0,
				0
				);
			
			if (pmmfCopy->m_pBaseAddress == NULL) {
				
				pmmfCopy->Close();	
				iErrCode = ERROR_FAILURE;
			}
		}
	}

	return iErrCode;
}