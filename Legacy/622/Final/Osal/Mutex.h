// Mutex.h: interface for the Mutex class.
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

#if !defined(AFX_MUTEX_H__93D4C495_ABB4_11D1_9C5E_0060083E8062__INCLUDED_)
#define AFX_MUTEX_H__93D4C495_ABB4_11D1_9C5E_0060083E8062__INCLUDED_

#include "OS.h"
#include "Time.h"

#ifdef __LINUX__
#include "Algorithm.h"
#include "HashTable.h"

typedef class Mutex *NamedMutex;

#else if defined __WIN32__
typedef HANDLE NamedMutex;
#endif

class OSAL_EXPORT Mutex {
protected:

#ifdef __LINUX__
	pthread_mutex_t m_critsec;
    static Mutex *FindNamedMutex(const char* pszLockName);
#else if defined __WIN32__
    CRITICAL_SECTION m_critsec;
#endif

    bool m_fInit;

public:

    Mutex();
    ~Mutex();

    int Initialize();

    void Wait();
    bool TryWait();
    void Signal();

    static int Wait (const char* pszLockName, NamedMutex* pNamedMutex);
    static int TryWait (const char* pszLockName, NamedMutex* pNamedMutex);
    static int Signal (const NamedMutex& nmMutex);
};

#endif // !defined(AFX_MUTEX_H__93D4C495_ABB4_11D1_9C5E_0060083E8062__INCLUDED_)