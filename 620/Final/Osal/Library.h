// Library.h: interface for the Library class.
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

#if !defined(AFX_LIBRARY_H__EBDC80C3_8CD6_11D3_A240_0050047FE2E2__INCLUDED_)
#define AFX_LIBRARY_H__EBDC80C3_8CD6_11D3_A240_0050047FE2E2__INCLUDED_

#include "OS.h"

class OSAL_EXPORT Library {

#ifdef __LINUX__
	void *m_hLibrary;
#else if defined __WIN32__
	HMODULE m_hLibrary;
#endif

    void Clean();

public:
    Library();
    ~Library();

    int Open (const char* pszLibrary);
    int Close();

    bool IsOpen();

    void* GetExport (const char* pszExport);
};

#endif // !defined(AFX_LIBRARY_H__EBDC80C3_8CD6_11D3_A240_0050047FE2E2__INCLUDED_)
