// ReadWriteLock.cpp: implementation of the ReadWriteLock class.
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
#include "ReadWriteLock.h"
#include "Algorithm.h"
#undef OSAL_BUILD

#define NO_THREAD 0x87654321

#define HELD_EXCLUSIVE      (0x00000001)
#define HELD_SPECULATIVE    (0x00000002)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ReadWriteLock::ReadWriteLock() {

    m_iNumReaders = 0;
    m_iNumWriters = 0;

    m_iFlags = 0;
    m_tidThread = NO_THREAD;
}

int ReadWriteLock::Initialize() {

    int iErrCode;

    iErrCode = m_mLock.Initialize();
    if (iErrCode == OK) {
        iErrCode = m_eEvent.Initialize();
    }
    return iErrCode;
}

void ReadWriteLock::WaitReader() {

    Assert (m_iNumReaders >= 0);
    Algorithm::AtomicIncrement (&m_iNumReaders);

    if (m_iNumWriters > 0) {

        Assert (m_tidThread != GetCurrentThreadId());

        if (Algorithm::AtomicDecrement (&m_iNumReaders) == 0) {
            m_eEvent.Signal();
        }

        m_mLock.Wait();
        Algorithm::AtomicIncrement (&m_iNumReaders);
        m_mLock.Signal();
    }

    Assert (m_tidThread != GetCurrentThreadId());
    Assert (!(m_iFlags & HELD_EXCLUSIVE));
}

void ReadWriteLock::SignalReader() {

    Assert (m_iNumReaders > 0);
    Assert (m_tidThread != GetCurrentThreadId());

    int iNumReaders = Algorithm::AtomicDecrement (&m_iNumReaders);
    if (iNumReaders == 0 && m_iNumWriters > 0) {
        m_eEvent.Signal();
    }
    Assert (iNumReaders >= 0);
}

void ReadWriteLock::WaitWriter() {

    m_mLock.Wait();

    Assert (m_iNumWriters == 0);
    m_iNumWriters ++;
    Assert (m_iNumWriters == 1);
    Assert (m_iNumReaders >= 0);

    while (m_iNumReaders > 0) {
        m_eEvent.Wait();
    }
    Assert (m_iNumWriters == 1);

    m_tidThread = GetCurrentThreadId();
    m_iFlags |= HELD_EXCLUSIVE;
}

void ReadWriteLock::SignalWriter() {

    Assert (m_tidThread == GetCurrentThreadId());
    Assert (m_iFlags & HELD_EXCLUSIVE);
    Assert (m_iNumWriters == 1);

    m_tidThread = NO_THREAD;
    m_iFlags &= ~HELD_EXCLUSIVE;

    m_iNumWriters --;
    Assert (m_iNumWriters == 0);

    m_mLock.Signal();
}

void ReadWriteLock::WaitReaderWriter() {

    m_mLock.Wait();

    m_tidThread = GetCurrentThreadId();
    m_iFlags |= HELD_SPECULATIVE;
}

void ReadWriteLock::SignalReaderWriter() {

    Assert (m_tidThread == GetCurrentThreadId());
    Assert (m_iFlags & HELD_SPECULATIVE);
    Assert (!(m_iFlags & HELD_EXCLUSIVE));
    Assert (m_iNumWriters == 0);

    m_tidThread = NO_THREAD;
    m_iFlags &= ~HELD_SPECULATIVE;

    m_mLock.Signal();
}

void ReadWriteLock::UpgradeReaderWriter() {

    Assert (m_tidThread == GetCurrentThreadId());
    Assert (m_iFlags & HELD_SPECULATIVE);
    Assert (m_iNumWriters == 0);

    m_iNumWriters ++;

    while (m_iNumReaders > 0) {
        m_eEvent.Wait();
    }

    Assert (m_tidThread == GetCurrentThreadId());
    Assert (m_iNumReaders == 0);

    m_iFlags |= HELD_EXCLUSIVE;
}

void ReadWriteLock::DowngradeReaderWriter() {

    Assert (m_tidThread == GetCurrentThreadId());
    Assert (m_iFlags & HELD_SPECULATIVE);
    Assert (m_iFlags & HELD_EXCLUSIVE);

    Assert (m_iNumWriters == 1);
    m_iNumWriters --;
    Assert (m_iNumWriters == 0);

    m_iFlags &= ~HELD_EXCLUSIVE;
}

bool ReadWriteLock::HeldExclusive (Thread* pThread) {

    if (!(m_iFlags & HELD_EXCLUSIVE)) {
        return false;
    }

    return Held (pThread);
}

bool ReadWriteLock::HeldSpeculative (Thread* pThread) {

    if (!(m_iFlags & HELD_SPECULATIVE)) {
        return false;
    }

    return Held (pThread);
}

bool ReadWriteLock::Held (Thread* pThread) {

    unsigned int iThreadId;
    
    if (pThread == NULL) {
        iThreadId = GetCurrentThreadId();
    } else {
        iThreadId = pThread->GetThreadId();
    }

    return m_tidThread == iThreadId;
}

//
// Autolocks
//

AutoReadLock::AutoReadLock (ReadWriteLock* prwLock) {

    Assert (prwLock != NULL);
    m_prwLock = prwLock;
    m_prwLock->WaitReader();
}

AutoReadLock::~AutoReadLock() {

    m_prwLock->SignalReader();
}

AutoWriteLock::AutoWriteLock (ReadWriteLock* prwLock) {

    Assert (prwLock != NULL);
    m_prwLock = prwLock;
    m_prwLock->WaitWriter();
}

AutoWriteLock::~AutoWriteLock() {

    m_prwLock->SignalWriter();
}