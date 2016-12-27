// HttpThreadPool.cpp: implementation of the HttpThreadPool class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar 1.0:  a web server
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "HttpThreadPool.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"

#include "Osal/Algorithm.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HttpThreadPool::HttpThreadPool (HttpServer* pHttpServer, unsigned int iInitNumThreads, 
                                unsigned int iMaxNumThreads) {

    m_iNumThreads = m_iInitNumThreads = iInitNumThreads;
    m_iMaxNumThreads = iMaxNumThreads;

    Assert (m_iMaxNumThreads > 0);
    Assert (m_iNumThreads > 0);
    Assert (m_iMaxNumThreads > m_iNumThreads);

    m_iMaxNumTasksQueued = 0;
    m_iNumIdleThreads = 0;
    m_iNumTimingOutThreads = 0;

    m_pThreadBlock = NULL;
    m_ppThreads = NULL;

    m_pHttpServer = pHttpServer;
    Assert (m_pHttpServer != NULL);
}

HttpThreadPool::~HttpThreadPool() {

    if (m_pThreadBlock != NULL) {
        delete [] m_pThreadBlock;
    }

    if (m_ppThreads != NULL) {
        delete [] m_ppThreads;
    }
}

unsigned int HttpThreadPool::GetNumThreads() {
    return m_iNumThreads;
}

unsigned int HttpThreadPool::GetNumQueuedTasks() {
    return m_tsfqTaskQueue.GetNumElements();
}

unsigned int HttpThreadPool::GetMaxNumQueuedTasks() {
    return m_iMaxNumTasksQueued;
}

int HttpThreadPool::Start() {

    int iErrCode;
    unsigned int i, j;

    if (!m_tsfqTaskQueue.Initialize()) {
        return ERROR_OUT_OF_MEMORY;
    }

    iErrCode = m_eThreadEvent.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_mThreadListLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    m_ppThreads = new HttpPoolThread* [m_iMaxNumThreads];
    if (m_ppThreads == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    m_pThreadBlock = new HttpPoolThread [m_iMaxNumThreads];
    if (m_pThreadBlock == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    for (i = 0; i < m_iMaxNumThreads; i ++) {
        m_ppThreads[i] = m_pThreadBlock + i;
        m_ppThreads[i]->iThreadIndex = i;
        m_ppThreads[i]->pHttpServer = m_pHttpServer;
    }

    // Start off the initial number of threads
    for (i = 0; i < m_iNumThreads; i ++) {

        iErrCode = m_ppThreads[i]->Start (ThreadExec, m_ppThreads[i]);
        if (iErrCode != OK) {

            // Stop the threads we've started
            for (j = 0; j < i; j ++) {
                // TODO - really?
                m_ppThreads[i]->Terminate();
            }
            break;
        }
    }

    return iErrCode;
}

int HttpThreadPool::Stop() {

    unsigned int i, iRetries = 0;

    // Send shutdown signal to threads.
    // Try to tolerate transient failures
    for (i = 0; i < m_iMaxNumThreads; i ++) {

        if (!m_tsfqTaskQueue.Push (NULL)) {
            if (++ iRetries == 100) {
                // We couldn't push anything onto the queue.
                // Just give up. We don't want to wait forever below
                return ERROR_OUT_OF_MEMORY;
            }
            OS::Sleep (1000);
            i --;
        }
    }

    // Unfortunately, we have to keep banging on the event 
    // until everyone gets their own shutdown message
    //
    // TODO: use a WaitForMultipleObjects call to allow
    // just one event to be signalled for everyone to wake up
    while (m_iNumThreads > 0) {
        m_eThreadEvent.Signal();
        OS::Sleep (20);
    }

    // Make sure all threads are gone
    for (i = 0; i < m_iMaxNumThreads; i ++) {
        if (m_ppThreads[i]->IsAlive()) {
            m_ppThreads[i]->WaitForTermination();
        }
    }

    return OK;
}

int HttpThreadPool::QueueTask (Socket* pSocket) {

    Assert (pSocket != NULL);

    // Add socket to queue
    if (!m_tsfqTaskQueue.Push (pSocket)) {
        return ERROR_OUT_OF_MEMORY;
    }

    // Check for no idle threads
    if (m_iNumIdleThreads == 0) {
    
        m_mThreadListLock.Wait();

        // Try to start a new thread if we still have room (best effort only)
        if (m_iNumThreads < m_iMaxNumThreads) {

            m_ppThreads[m_iNumThreads]->iThreadIndex = m_iNumThreads;
            m_ppThreads[m_iNumThreads]->pHttpServer = m_pHttpServer;

            unsigned int iNumThreads = m_iNumThreads ++;
            if (m_ppThreads[iNumThreads]->Start (ThreadExec, m_ppThreads[iNumThreads]) != OK) {
                m_iNumThreads --;
            }
        }

        m_mThreadListLock.Signal();
    }

    // Wake up a thread
    m_eThreadEvent.Signal();

    // Doing this outside the lock may lead to inaccuracies, but it's not that important...
    unsigned int iNumTasks = m_tsfqTaskQueue.GetNumElements();
    if (iNumTasks > m_iMaxNumTasksQueued) {
        m_iMaxNumTasksQueued = iNumTasks;
    }

    return OK;
}

int HttpThreadPool::ThreadExec (void* pVoid) {

    HttpPoolThread* pSelf = (HttpPoolThread*) pVoid;
    return pSelf->pHttpServer->GetThreadPool()->RunWorkerThread (pSelf);
}

int HttpThreadPool::RunWorkerThread (HttpPoolThread* pThread) {

    int iErrCode = m_pHttpServer->WWWServe (pThread);

    m_mThreadListLock.Wait();

    // Defragment the thread array by grabbing the last valid thread
    // and moving it to our slot
    unsigned int iSelfThread = pThread->iThreadIndex;
    unsigned int iDefragThread = m_iNumThreads - 1;

    m_ppThreads [iSelfThread] = m_ppThreads [iDefragThread];
    m_ppThreads [iSelfThread]->iThreadIndex = iSelfThread;

    // One thread less in the pool...
    m_iNumThreads --;

    if (pThread->bTimingOut) {
        m_iNumTimingOutThreads --;
    }

    m_mThreadListLock.Signal();

    return iErrCode;
}

Socket* HttpThreadPool::WaitForTask (HttpPoolThread* pSelf) {

    Socket* pSocket = NULL;
    Algorithm::AtomicIncrement (&m_iNumIdleThreads);

    while (true) {

        // Pop socket off queue
        if (m_tsfqTaskQueue.Pop (&pSocket)) {
            break;
        }

        // Wait for the signal that an event arrived
        int iErrCode = m_eThreadEvent.Wait (3 * 60 * 1000);
        if (iErrCode == WARNING) {

            // We timed out, so see if we should idle out
            m_mThreadListLock.Wait();

            bool bTimeout = m_iNumThreads - m_iNumTimingOutThreads > m_iInitNumThreads;
            if (bTimeout) {
                m_iNumTimingOutThreads ++;
            }

            m_mThreadListLock.Signal();

            if (bTimeout) {
                pSelf->bTimingOut = true;
                pSocket = NULL;
                break;
            }
        }
    }

    Algorithm::AtomicDecrement (&m_iNumIdleThreads);
    return pSocket;
}