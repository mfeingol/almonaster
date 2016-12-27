// HttpThreadPool.cpp: implementation of the HttpThreadPool class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar 1.0:  a web server
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

	m_iMaxNumTasksQueued = 0;
	m_iNumIdleThreads = 0;

	m_pThreads = NULL;

	m_pHttpServer = pHttpServer;
	Assert (m_pHttpServer != NULL);
}

HttpThreadPool::~HttpThreadPool() {

	if (m_pThreads != NULL) {
		delete [] m_pThreads;
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

	unsigned int i;

	if (!m_tsfqTaskQueue.Initialize()) {
		return ERROR_OUT_OF_MEMORY;
	}

	m_pThreads = new Thread [m_iMaxNumThreads];
	if (m_pThreads == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	// Start off the initial threads
	for (i = 0; i < m_iNumThreads; i ++) {
		m_pThreads[i].Start (HttpServer::ThreadExec, m_pHttpServer);
	}

	return OK;
}

int HttpThreadPool::Stop() {

	unsigned int i, iRetries = 0;

	// Send shutdown signal to threads
	for (i = 0; i < m_iNumThreads && iRetries < 100; i ++) {
		if (!m_tsfqTaskQueue.Push (SHUTDOWN_THREAD_POOL)) {
			i --;
			iRetries ++;
			OS::Sleep (250);
		}
	}

	if (iRetries == 100) {
		return ERROR_OUT_OF_MEMORY;
	}

	// Tell threads to shut down
	while (m_iNumThreads > 0) {
		m_eThreadEvent.Signal();
		OS::Sleep();
	}

	// Wait for them all to exit
	for (i = 0; i < m_iMaxNumThreads; i ++) {
		if (m_pThreads[i].IsAlive()) {
			m_pThreads[i].WaitForTermination();
		}
	}

	return OK;
}

int HttpThreadPool::QueueTask (Socket* pSocket) {

	int iErrCode;

	Assert (pSocket != NULL);

	// Add socket to queue
	if (!m_tsfqTaskQueue.Push (pSocket)) {
		return ERROR_OUT_OF_MEMORY;
	}

	// Check for no idle threads
	if (m_iNumIdleThreads == 0) {
	
		m_mThreadListLock.Wait();
		
		if (m_iNumThreads < m_iMaxNumThreads) {
			
			// Start a new thread
			iErrCode = m_pThreads[m_iNumThreads].Start (HttpServer::ThreadExec, m_pHttpServer);
			if (iErrCode != OK) {
				m_mThreadListLock.Signal();
				return ERROR_OUT_OF_MEMORY;
			}
			m_iNumThreads ++;
		}

		m_mThreadListLock.Signal();

	}

	// Wake up a thread!
	m_eThreadEvent.Signal();

	// Doing this outside the lock may lead to inaccuracies, but so what?
	unsigned int iNumTasks = m_tsfqTaskQueue.GetNumElements();
	if (iNumTasks > m_iMaxNumTasksQueued) {
		m_iMaxNumTasksQueued = iNumTasks;
	}

	// Check for excessive number of threads
	if (m_iNumIdleThreads > m_iNumThreads / 2 && m_iNumThreads > m_iInitNumThreads) {

		// Send someone a poison pill, best effort
		if (m_tsfqTaskQueue.Push (NULL)) {
			m_eThreadEvent.Signal();
		}
	}

	return OK;
}

Socket* HttpThreadPool::WaitForTask() {

	Socket* pSocket = NULL;

	while (true) {

		// Pop socket off queue
		if (m_tsfqTaskQueue.Pop (&pSocket)) {

			// Exit signal
			if (pSocket == NULL || pSocket == SHUTDOWN_THREAD_POOL) {

				if (m_iNumThreads == m_iInitNumThreads && pSocket != SHUTDOWN_THREAD_POOL) {

					// Oops - got a bit too enthusiastic trimming threads
					continue;
				}

				Algorithm::AtomicDecrement (&m_iNumThreads);
				Assert (pSocket == SHUTDOWN_THREAD_POOL || m_iNumThreads >= m_iInitNumThreads);

				return NULL;
			}

			break;
		}

		Algorithm::AtomicIncrement (&m_iNumIdleThreads);

		// Wait on event
		m_eThreadEvent.Wait();
		
		Algorithm::AtomicDecrement (&m_iNumIdleThreads);

		// Pop socket off queue		
		if (m_tsfqTaskQueue.Pop (&pSocket)) {

			// Exit signal
			if (pSocket == NULL || pSocket == SHUTDOWN_THREAD_POOL) {

				if (m_iNumThreads == m_iInitNumThreads && pSocket != SHUTDOWN_THREAD_POOL) {

					// Oops - got a bit too enthusiastic trimming threads
					continue;
				}

				Algorithm::AtomicDecrement (&m_iNumThreads);
				Assert (pSocket == SHUTDOWN_THREAD_POOL || m_iNumThreads >= m_iInitNumThreads);

				return NULL;
			}

			break;
		}
	}

	return pSocket;
}