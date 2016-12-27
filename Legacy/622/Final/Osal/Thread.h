// Thread.h: interface for the Thread class.
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

#if !defined(AFX_THREAD_H__17166655_AC76_11D1_9C5F_0060083E8062__INCLUDED_)
#define AFX_THREAD_H__17166655_AC76_11D1_9C5F_0060083E8062__INCLUDED_

#include "OS.h"
#include "Time.h"

#ifdef __LINUX__
#include <pthread.h>
#endif

class OSAL_EXPORT Thread {
protected:

#ifdef __LINUX__
    struct pthread_start_routine
    {
        ThreadFunction pFunctionName;
        void *pData;
        Thread *thread;
    };
	pthread_t m_hThread;
    bool m_bInit;
    bool m_bAlive;
    struct pthread_start_routine *start;
    static void *Thread::PthreadStartThreadFunction(void *arg);
#else if defined __WIN32__
	HANDLE m_hThread;
	unsigned int m_uiThreadId;
#endif

    static int m_piPriority [7];

public:

    Thread();
    ~Thread();

    enum ThreadPriority { 
        LowestPriority, 
        LowerPriority, 
        NormalPriority, 
        HigherPriority, 
        HighestPriority,
        CriticalPriority,
        InvalidPriority
    };

    int Start (ThreadFunction pFunctionName, void* pData, ThreadPriority iPriority = NormalPriority);

#ifdef __WIN32__
    int WaitForTermination (MilliSeconds msWait = WAIT_INFINITE);
#else if defined __LINUX__
	int WaitForTermination ();
#endif

#ifdef __WIN32__
    int Suspend();
#endif
    int Resume();
    int Terminate();

    bool IsAlive();
    unsigned int GetThreadId();

    int SetPriority (ThreadPriority iPriority);
    ThreadPriority GetPriority();

#ifdef __WIN32__
    int GetReturnValue (int* piReturnValue);
#endif

    static void GetCurrentThread (Thread* ptThread);
};

#endif // !defined(AFX_THREAD_H__17166655_AC76_11D1_9C5F_0060083E8062__INCLUDED_)