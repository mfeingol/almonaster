// Library.cpp: implementation of the Library class.
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
#include "Library.h"
#undef OSAL_BUILD

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Library::Library() {

	m_hLibrary = NULL;
}


Library::~Library() {

	Close();
}


int Library::Open (const char* pszLibrary) {

	m_hLibrary = ::LoadLibrary (pszLibrary);

	return m_hLibrary == NULL ? ERROR_FAILURE : OK;
}

int Library::Close() {

	if (m_hLibrary == NULL) {
		return ERROR_FAILURE;
	}
		
	::FreeLibrary (m_hLibrary);
	m_hLibrary = NULL;

	return OK;
}

bool Library::IsOpen() {

	return m_hLibrary != NULL;
}

void* Library::GetExport (const char* pszExport) {

	return ::GetProcAddress (m_hLibrary, pszExport);
}