// Mutex.h: interface for the Mutex class.
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

#if !defined(AFX_MUTEX_H__93D4C495_ABB4_11D1_9C5E_0060083E8062__INCLUDED_)
#define AFX_MUTEX_H__93D4C495_ABB4_11D1_9C5E_0060083E8062__INCLUDED_

#include "OS.h"
#include "Time.h"

typedef HANDLE NamedMutex;

class OSAL_EXPORT Mutex {

protected:

	CRITICAL_SECTION m_critsec;

public:
	
	Mutex();
	~Mutex();

	void Wait();
	bool TryWait();
	void Signal();

	static int Wait (const char* pszLockName, NamedMutex* pNamedMutex);
	static int TryWait (const char* pszLockName, NamedMutex* pNamedMutex);
	static int Signal (const NamedMutex& nmMutex);
};

#endif // !defined(AFX_MUTEX_H__93D4C495_ABB4_11D1_9C5E_0060083E8062__INCLUDED_)