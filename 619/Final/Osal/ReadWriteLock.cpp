// ReadWriteLock.cpp: implementation of the ReadWriteLock class.
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
#include "ReadWriteLock.h"
#include "Algorithm.h"
#undef OSAL_BUILD

#define NO_THREAD 0x87654321

#define YIELD_MS 100

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ReadWriteLock::ReadWriteLock() {

	m_iNumReaders = 0;
	m_iNumWriters = 0;

	m_dwLastThreadLocked = NO_THREAD;
}

void ReadWriteLock::WaitReader() {

	Algorithm::AtomicIncrement (&m_iNumReaders);

	if (m_iNumWriters > 0) {

		Algorithm::AtomicDecrement (&m_iNumReaders);
		Lock();
		Algorithm::AtomicIncrement (&m_iNumReaders);
	}
}

void ReadWriteLock::SignalReader() {

	if (m_dwLastThreadLocked == ::GetCurrentThreadId()) {
		Unlock();
	}

	int iNumReaders = Algorithm::AtomicDecrement (&m_iNumReaders);
	Assert (iNumReaders != 0);
}

void ReadWriteLock::WaitWriter() {

	Lock();

	while (m_iNumReaders > 0) {
		OS::Sleep (YIELD_MS);
	}

	m_iNumWriters ++;

	while (m_iNumReaders > 0) {
		OS::Sleep (YIELD_MS);
	}
}

void ReadWriteLock::SignalWriter() {

	Assert (m_dwLastThreadLocked == ::GetCurrentThreadId());
	Assert (m_iNumWriters == 1);

	m_iNumWriters --;
	Unlock();
}

void ReadWriteLock::WaitReaderWriter() {

	Lock();
}

void ReadWriteLock::SignalReaderWriter() {

	Assert (m_dwLastThreadLocked == ::GetCurrentThreadId());
	Assert (m_iNumWriters == 0);

	Unlock();
}

void ReadWriteLock::UpgradeReaderWriter() {

	Assert (m_dwLastThreadLocked == ::GetCurrentThreadId());
	Assert (m_iNumWriters == 0);

	m_iNumWriters ++;

	while (m_iNumReaders > 0) {
		OS::Sleep (YIELD_MS);
	}
}

void ReadWriteLock::DowngradeReaderWriter() {

	Assert (m_dwLastThreadLocked == ::GetCurrentThreadId());
	Assert (m_iNumWriters == 1);

	m_iNumWriters --;
}

//
// Utility
//

inline void ReadWriteLock::Lock() {

	m_mLock.Wait();
	m_dwLastThreadLocked = ::GetCurrentThreadId();
}

inline void ReadWriteLock::Unlock() {

	m_dwLastThreadLocked = NO_THREAD;
	m_mLock.Signal();
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