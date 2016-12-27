// MemoryMappedFile.h: interface for the MemoryMappedFile class.
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

#if !defined(AFX_MEMORYMAPPEDFILE_H__05370B53_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)
#define AFX_MEMORYMAPPEDFILE_H__05370B53_2751_11D3_A0DC_0050047FE2E2__INCLUDED_

#include "OS.h"

// Flags
#define MEMMAP_READONLY     (0x00000001)
#define MEMMAP_WRITETHROUGH (0x00000002)

//
// Class
//

class OSAL_EXPORT MemoryMappedFile {
private:

#ifdef __LINUX__
	int m_hFile;
#else if defined __WIN32__
	HANDLE m_hMappedFile;
	HANDLE m_hFile;
#endif

    unsigned int m_iFlags;
    size_t m_stSize;
    void* m_pBaseAddress;

    int OpenMappedFile (size_t stSize);

public:

    MemoryMappedFile();
    ~MemoryMappedFile();

    int OpenNew (const char* pszFileName, size_t stSize, unsigned int iFlags);
    int OpenExisting (const char* pszFileName, unsigned int iFlags);

    bool IsOpen();

    int Close();
    int Flush();

    int Resize (size_t stNewSize);
    int Expand (size_t stIncrement);

    size_t GetSize() const;

    void* GetAddress();
    void* GetEndOfFile() const;
};

#endif // !defined(AFX_MEMORYMAPPEDFILE_H__05370B53_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)