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
    m_fInit = false;
}

Mutex::~Mutex() {
    if (m_fInit) {
#ifdef __LINUX__
        pthread_mutex_destroy(&m_critsec);
#else if defined __WIN32__
        ::DeleteCriticalSection (&m_critsec);
#endif
    }
}

int Mutex::Initialize() {

    int iErrCode;

    if (m_fInit) {
        Assert (false);
        return ERROR_FAILURE;
    }

#ifdef __LINUX__
    iErrCode = pthread_mutex_init(&m_critsec, NULL) == 0 ? OK : ERROR_FAILURE;
#else if defined __WIN32__
    BOOL f = ::InitializeCriticalSectionAndSpinCount (&m_critsec, 3000 * OS::GetNumProcessors());
    iErrCode = f ? OK : ERROR_OUT_OF_MEMORY;
#endif

    if (iErrCode == OK) {
        m_fInit = true;
    }
    return iErrCode;
}

void Mutex::Wait() {
    Assert (m_fInit);
#ifdef __LINUX__
    pthread_mutex_lock(&m_critsec);
#else if defined __WIN32__
    ::EnterCriticalSection (&m_critsec);
#endif
}

bool Mutex::TryWait() {
    Assert (m_fInit);
#ifdef __LINUX__
    return pthread_mutex_trylock(&m_critsec) != EBUSY;
#else if defined __WIN32__
    return ::TryEnterCriticalSection (&m_critsec) != FALSE;
#endif
}

void Mutex::Signal() {
    Assert (m_fInit);
#ifdef __LINUX__
    pthread_mutex_unlock(&m_critsec);
#else if defined __WIN32__
    ::LeaveCriticalSection (&m_critsec);
#endif
}

int Mutex::Wait (const char* pszLockName, NamedMutex* pNamedMutex) {

#ifdef __LINUX__
    *pNamedMutex = FindNamedMutex(pszLockName);
    if (pNamedMutex == NULL)
        return ERROR_FAILURE;
    (*pNamedMutex)->Wait();
	return OK;

#else if defined __WIN32__

    *pNamedMutex = ::CreateMutex (NULL, FALSE, pszLockName);
    if (*pNamedMutex == NULL) {
        Assert (false);
        return ERROR_FAILURE;
    }
    return ::WaitForSingleObject (*pNamedMutex, INFINITE) == WAIT_OBJECT_0 ? OK : ERROR_FAILURE;
#endif
}

int Mutex::TryWait (const char* pszLockName, NamedMutex* pNamedMutex) {

#ifdef __LINUX__
    *pNamedMutex = FindNamedMutex(pszLockName);
    if (pNamedMutex == NULL)
        return ERROR_FAILURE;
    return (*pNamedMutex)->TryWait() ? OK : WARNING;

#else if defined __WIN32__

    *pNamedMutex = ::CreateMutex (NULL, FALSE, pszLockName);
    if (*pNamedMutex == NULL) {
        Assert (false);
        return ERROR_FAILURE;
    }

    DWORD dwRetVal = ::WaitForSingleObject (*pNamedMutex, 0);
    if (dwRetVal == WAIT_FAILED) {
        Assert (false);
        return ERROR_FAILURE;
    }
    return dwRetVal != WAIT_TIMEOUT ? OK : WARNING;
#endif
}

int Mutex::Signal (const NamedMutex& nmMutex) {

#ifdef __LINUX__
    nmMutex->Signal();
	return OK;

#else if defined __WIN32__

    if (!::ReleaseMutex (nmMutex)) {
        Assert (false);
        return ERROR_FAILURE;
    }

    ::CloseHandle (nmMutex);
    return OK;
#endif
}


#ifdef __LINUX__

//
// Hashtable for named mutexes in Linux
//

class NamedMutexHashValue
{
public:
    static unsigned int GetHashValue(const char *pszKey, unsigned int iNumBuckets, const void *pHashHint);
};

class NamedMutexEquals
{
public:
    static bool Equals (const char *pszLeft, const char *pszRight, const void *pEqualsHint);
};

#define NAMED_MUTEX_HASHTABLE_NUM_BUCKETS 23

unsigned int NamedMutexHashValue::GetHashValue(const char *pszKey, 
                                                  unsigned int iNumBuckets, const void *pHashHint)
{
    return Algorithm::GetStringHashValue(pszKey, iNumBuckets, false);
}

bool NamedMutexEquals::Equals(const char *pszLeft, const char *pszRight, const void *pEqualsHint)
{
    return strcmp(pszLeft, pszRight) == 0;
}

Mutex *Mutex::FindNamedMutex(const char* pszLockName)
{
    static Mutex *NamedMutexLock = new Mutex();
    static HashTable<const char *, Mutex *, NamedMutexHashValue, NamedMutexEquals> *phtNamedMutexHashTable = NULL;

    Mutex *mutex;
    NamedMutexLock->Wait();

    if (phtNamedMutexHashTable == NULL)
    {
        phtNamedMutexHashTable = new HashTable<const char *, Mutex *, NamedMutexHashValue, NamedMutexEquals> (NULL, NULL);
        phtNamedMutexHashTable->Initialize(NAMED_MUTEX_HASHTABLE_NUM_BUCKETS);
    }

    if (!phtNamedMutexHashTable->FindFirst(pszLockName, &mutex))
    {
        // not found
        mutex = new Mutex();
        phtNamedMutexHashTable->Insert(String::StrDup(pszLockName), mutex);
    }

    NamedMutexLock->Signal();

    return mutex;
}

#endif