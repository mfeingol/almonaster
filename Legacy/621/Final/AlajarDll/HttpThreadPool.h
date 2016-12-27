// HttpThreadPool.h: interface for the ThreadPool class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar.dll
// Copyright (C) 1998-1999 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#if !defined(AFX_HTTPTHREADPOOL_H__F1404ED1_FCC1_11D1_81D8_00C04FB9359A__INCLUDED_)
#define AFX_HTTPTHREADPOOL_H__F1404ED1_FCC1_11D1_81D8_00C04FB9359A__INCLUDED_

#include "Osal/Thread.h"
#include "Osal/FifoQueue.h"
#include "Osal/Mutex.h"
#include "Osal/Socket.h"
#include "Osal/Event.h"

#define SHUTDOWN_THREAD_POOL ((Socket*) 0xffffffff)

class HttpServer;

class HttpPoolThread : public Thread {
public:
    unsigned int iThreadIndex;
    HttpServer* pHttpServer;
    bool bTimingOut;

    HttpPoolThread() {
        bTimingOut = false;
    }
};

class HttpThreadPool {
private:

    unsigned int m_iNumThreads;
    unsigned int m_iInitNumThreads;
    unsigned int m_iMaxNumThreads;
    unsigned int m_iNumIdleThreads;

    unsigned int m_iMaxNumTasksQueued;
    unsigned int m_iNumTimingOutThreads;

    Mutex m_mThreadListLock;

    HttpPoolThread* m_pThreadBlock;
    HttpPoolThread** m_ppThreads;
    
    ThreadSafeFifoQueue<Socket*> m_tsfqTaskQueue;
    Event m_eThreadEvent;

    HttpServer* m_pHttpServer;

    static int THREAD_CALL ThreadExec (void* pVoid);

    int RunWorkerThread (HttpPoolThread* pThread);

public:

    HttpThreadPool (HttpServer* pHttpServer, unsigned int iInitNumThreads, unsigned int iMaxNumThreads);
    ~HttpThreadPool();

    int Start();
    int Stop();

    unsigned int GetNumThreads();

    unsigned int GetNumQueuedTasks();
    unsigned int GetMaxNumQueuedTasks();

    int QueueTask (Socket* pSocket);

    // Thread fxns
    Socket* WaitForTask (HttpPoolThread* pThread);
};

#endif // !defined(AFX_HTTPTHREADPOOL_H__F1404ED1_FCC1_11D1_81D8_00C04FB9359A__INCLUDED_)