// ReadWriteLock.h: interface for the ReadWriteLock class.
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

#if !defined(AFX_READWRITELOCK_H__857FDA13_4D1F_11D3_A18F_0050047FE2E2__INCLUDED_)
#define AFX_READWRITELOCK_H__857FDA13_4D1F_11D3_A18F_0050047FE2E2__INCLUDED_

#include "Mutex.h"
#include "Event.h"
#include "Thread.h"

class OSAL_EXPORT ReadWriteLock {
private:

    int m_iNumReaders;
    int m_iNumWriters;
    int m_iFlags;

#ifdef __LINUX__
    pthread_t m_tidThread;
    pthread_t GetCurrentThreadId() { return pthread_self(); }
#else if defined __WIN32__
	DWORD m_tidThread;
    DWORD GetCurrentThreadId() { return ::GetCurrentThreadId(); }
#endif

    Mutex m_mLock;
    Event m_eEvent;

    bool Held (Thread* pThread);

public:

    ReadWriteLock();

    int Initialize();

    void WaitReader();
    void SignalReader();

    void WaitWriter();
    void SignalWriter();

    void WaitReaderWriter();
    void SignalReaderWriter();

    void UpgradeReaderWriter();
    void DowngradeReaderWriter();

    bool HeldExclusive (Thread* pThread);
    bool HeldSpeculative (Thread* pThread);
};

class AutoReadLock {
protected:
    ReadWriteLock* m_prwLock;
public:
    AutoReadLock (ReadWriteLock* prwLock);
    ~AutoReadLock();
};

class AutoWriteLock {
protected:
    ReadWriteLock* m_prwLock;
public:
    AutoWriteLock (ReadWriteLock* prwLock);
    ~AutoWriteLock();
};

#endif // !defined(AFX_READWRITELOCK_H__857FDA13_4D1F_11D3_A18F_0050047FE2E2__INCLUDED_)
