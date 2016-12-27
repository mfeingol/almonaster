// Thread.cpp: implementation of the Thread class.
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
#include "Thread.h"
#undef OSAL_BUILD

#ifdef __WIN32__
#include <process.h>
#endif

#ifdef __LINUX__

void *Thread::PthreadStartThreadFunction(void *arg)
{
    // need to convert function type from what pthread library expects to what
    // this library needs to use
    struct pthread_start_routine *start = (struct pthread_start_routine *) arg;
    start->pFunctionName(start->pData);
    if (start->thread)
    {
        start->thread->m_bAlive = false;
        start->thread->start = NULL;
    }
    delete start;
    return NULL;
}

#endif

#ifdef __WIN32__

#define INVALID_THREAD ((HANDLE) -1)
#define INVALID_THREAD_ID (0xfffffff)

int Thread::m_piPriority [] = { THREAD_PRIORITY_LOWEST,
                                THREAD_PRIORITY_BELOW_NORMAL,
                                THREAD_PRIORITY_NORMAL,
                                THREAD_PRIORITY_ABOVE_NORMAL,
                                THREAD_PRIORITY_HIGHEST,
                                THREAD_PRIORITY_TIME_CRITICAL,
                                THREAD_PRIORITY_NORMAL
                              };
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Thread::Thread() {
#ifdef __LINUX__
    start = NULL;
    m_bInit = false;
#else if defined __WIN32__
    m_hThread = INVALID_THREAD;
    m_uiThreadId = INVALID_THREAD_ID;
#endif
}

Thread::~Thread() {
#ifdef __LINUX__
    if (start)
        start->thread = NULL;
#else if defined __WIN32__
	if (m_hThread != INVALID_THREAD) {
		::CloseHandle (m_hThread);
	}
#endif
}

int Thread::Start (ThreadFunction pFunctionName, void* pData, ThreadPriority iPriority) {

#ifdef __LINUX__
    if (m_bInit)
    {
        // Seems that code in AlajarDll/HttpThreadPool.cpp is broken
        // What if thread has already been started? Can't just start another one
        Assert(false);
		return ERROR_FAILURE;
    }

    start = new pthread_start_routine;

    start->pFunctionName = pFunctionName;
    start->pData = pData;
    start->thread = this;

    m_bAlive = true;

    if (pthread_create(&m_hThread, NULL, PthreadStartThreadFunction, start))
        return ERROR_FAILURE;

    m_bInit = true;

    return OK;

#else if defined __WIN32__

    // Don't leak
    if (m_hThread != INVALID_THREAD) {
        ::CloseHandle (m_hThread);
    }

    unsigned int iFlags = iPriority == NormalPriority ? 0 : CREATE_SUSPENDED;

    m_hThread = (HANDLE) _beginthreadex (
        NULL,
        0, 
        (unsigned int (__stdcall *)(void*)) pFunctionName, 
        pData, 
        iFlags, 
        &m_uiThreadId
        );

    if (m_hThread == INVALID_THREAD) {
        return ERROR_FAILURE;
    }

    if (iPriority == NormalPriority) {
        return OK;
    }

    // Best effort
    if (!::SetThreadPriority (m_hThread, m_piPriority [iPriority]) || ::ResumeThread (m_hThread) == -1) {
        TerminateThread (m_hThread, 666);
        m_hThread = INVALID_THREAD;
        return ERROR_FAILURE;
    }
    return OK;
#endif
}


#ifdef __LINUX__
int Thread::WaitForTermination ()
{
    // can't set timeout on Linux
    if (!m_bInit)
		return ERROR_FAILURE;

    if (!pthread_join(m_hThread, NULL))
		return ERROR_FAILURE;

    return OK;
}

#else if defined __WIN32__
int Thread::WaitForTermination (MilliSeconds msWait) {

	if (m_hThread == INVALID_THREAD) {
		return ERROR_FAILURE;
	}

	DWORD dwRetVal = ::WaitForSingleObjectEx(m_hThread, msWait, TRUE);
	if (dwRetVal == WAIT_TIMEOUT) {
		return WARNING;
	}

    return dwRetVal == WAIT_OBJECT_0 ? OK : ERROR_FAILURE;
}
#endif

#ifdef __WIN32__
int Thread::Suspend() {

    if (m_hThread == INVALID_THREAD) {
        return ERROR_FAILURE;
    }

    return ::SuspendThread (m_hThread) == -1 ? ERROR_FAILURE : OK;
}
#endif

int Thread::Resume() {
#ifdef __LINUX__
    // not implemented
    return ERROR_FAILURE;
#else if defined __WIN32__
    if (m_hThread == INVALID_THREAD) {
        return ERROR_FAILURE;
    }
    return ::ResumeThread (m_hThread) == -1 ? ERROR_FAILURE : OK;
#endif
}

int Thread::SetPriority (ThreadPriority iPriority) {
#ifdef __LINUX__
    // not implemented
    return OK;
#else if defined __WIN32__
    if (m_hThread == INVALID_THREAD) {
        return ERROR_FAILURE;
    }

    if (::SetThreadPriority (m_hThread, m_piPriority [iPriority])) {
        return OK;
    }

    return ERROR_FAILURE;
#endif
}

Thread::ThreadPriority Thread::GetPriority() {
#ifdef __LINUX__
    // not implemented
	return InvalidPriority;
#else if defined __WIN32__
    if (m_hThread == INVALID_THREAD) {
        return InvalidPriority;
    }

    int i, iPriority = ::GetThreadPriority (m_hThread);
    for (i = 0; i < sizeof (m_piPriority) / sizeof (m_piPriority[0]); i ++) {

        if (iPriority == m_piPriority[i]) {
            return (Thread::ThreadPriority) ((int) LowestPriority + i);
        }
    }
    return InvalidPriority;
#endif
}

bool Thread::IsAlive() {
#ifdef __LINUX__
    if (!m_bInit)
		return false;

    return m_bAlive;

#else if defined __WIN32__

    DWORD dwExitCode;
    if (m_hThread == INVALID_THREAD || !::GetExitCodeThread (m_hThread, &dwExitCode)) {
        return false;
    }
    return dwExitCode == STILL_ACTIVE;
#endif
}

unsigned int Thread::GetThreadId() {
#ifdef __WIN32__
    return m_uiThreadId;
#else if defined __LINUX__
    return m_hThread;
#endif
}

#ifdef __WIN32__
int Thread::GetReturnValue (int* piReturnValue) {

    DWORD dwExitCode;
    if (!::GetExitCodeThread (m_hThread, &dwExitCode)) {
        return ERROR_FAILURE;
    }

    *piReturnValue = dwExitCode;
    
    return *piReturnValue == STILL_ACTIVE ? ERROR_FAILURE : OK;
}
#endif

int Thread::Terminate() {

#ifdef __LINUX__
    if (!m_bInit)
		return ERROR_FAILURE;

    m_bInit = false;
    return (pthread_cancel(m_hThread) == 0) ? OK : ERROR_FAILURE;

#else if defined __WIN32__

    BOOL bRetVal = ::TerminateThread (m_hThread, 666);
    m_hThread = INVALID_THREAD;
    m_uiThreadId = INVALID_THREAD_ID;
    return bRetVal ? OK : ERROR_FAILURE;
#endif
}

void Thread::GetCurrentThread (Thread* ptThread) {
#ifdef __LINUX__
	ptThread->m_hThread = pthread_self();
#else if defined __WIN32__
	ptThread->m_hThread = ::GetCurrentThread();
	ptThread->m_uiThreadId = ::GetCurrentThreadId();
#endif
}