// Mutex.cpp: implementation of the Mutex class.
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
#include "Mutex.h"
#include "String.h"
#undef OSAL_BUILD

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Mutex::Mutex() {
	// TODO: use InitializeCriticalSectionAndSpinCount
	::InitializeCriticalSection (&m_critsec);
}

Mutex::~Mutex() {
	::DeleteCriticalSection (&m_critsec);
}

void Mutex::Wait() {
	::EnterCriticalSection (&m_critsec);
}

bool Mutex::TryWait() {
	return ::TryEnterCriticalSection (&m_critsec) != FALSE;
}

void Mutex::Signal() {
	::LeaveCriticalSection (&m_critsec);
}

int Mutex::Wait (const char* pszLockName, NamedMutex* pNamedMutex) {

	*pNamedMutex = ::CreateMutex (NULL, FALSE, pszLockName);

	if (*pNamedMutex == NULL) {
		Assert (false);
		return ERROR_FAILURE;
	}

	::WaitForSingleObject (*pNamedMutex, INFINITE);

	return OK;
}

int Mutex::TryWait (const char* pszLockName, NamedMutex* pNamedMutex) {

	*pNamedMutex = ::CreateMutex (NULL, FALSE, pszLockName);

	if (*pNamedMutex == NULL) {
		Assert (false);
		return ERROR_FAILURE;
	}

	DWORD dwRetVal = ::WaitForSingleObject (*pNamedMutex, 0);

	return dwRetVal != WAIT_TIMEOUT ? OK : WARNING;
}

int Mutex::Signal (const NamedMutex& nmMutex) {

	if (!::ReleaseMutex (nmMutex)) {
		Assert (false);
		return ERROR_FAILURE;
	}

	::CloseHandle (nmMutex);

	return OK;
}