// Event.cpp: implementation of the Event class.
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
#include "Event.h"
#undef OSAL_BUILD

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Event::Event() {
#ifdef __LINUX__
    m_bInit = false;
#else if defined __WIN32__
    m_hEvent = NULL;
#endif
}

Event::~Event() {

#ifdef __LINUX__
    if (m_bInit)
        pthread_cond_destroy(&m_hEvent);
#else if defined __WIN32__
    if (m_hEvent != NULL) {
        ::CloseHandle (m_hEvent);
    }
#endif
}

int Event::Initialize() {

    int iErrCode = OK;

#ifdef __LINUX__
    pthread_cond_init(&m_hEvent, NULL);
    pthread_mutex_init(&m_Lock, NULL);
    m_bInit = true;

#else if defined __WIN32__
    // Auto-reset event, initially non-signalled
    m_hEvent = ::CreateEvent (NULL, FALSE, FALSE, NULL);
    if (m_hEvent == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
    }
#endif

    return iErrCode;
}


int Event::Signal() {

#ifdef __LINUX__
    Assert(m_bInit);
    pthread_mutex_lock(&m_Lock);
    // is signal good enough? or should it be broadcast?
    pthread_cond_signal(&m_hEvent);
    pthread_mutex_unlock(&m_Lock);
    return OK;

#else if defined __WIN32__
    Assert (m_hEvent != NULL);
    return ::SetEvent (m_hEvent) ? OK : ERROR_FAILURE;
#endif
}

int Event::Wait() {

#ifdef __LINUX__
    Assert (m_bInit);

    pthread_mutex_lock(&m_Lock);
    pthread_cond_wait(&m_hEvent, &m_Lock);
    pthread_mutex_unlock(&m_Lock);
    return OK;

#else if defined __WIN32__
    Assert (m_hEvent != NULL);
    return Wait (WAIT_INFINITE);
#endif
}

int Event::Wait (MilliSeconds iWait) {


#ifdef __LINUX__

    Assert (m_bInit);

    int retval;
    struct timespec ts;
    time_t now = time(NULL);
    ts.tv_nsec = iWait % 1000 * 1000000;
    ts.tv_sec = now + iWait / 1000;
    pthread_mutex_lock(&m_Lock);
    retval = pthread_cond_timedwait(&m_hEvent, &m_Lock, &ts);
    pthread_mutex_unlock(&m_Lock);

    if (retval == ETIMEDOUT)
        return WARNING;
    else
        return OK;

#else if defined __WIN32__

    Assert (m_hEvent != NULL);

    DWORD dwRetVal = ::WaitForSingleObject(m_hEvent, (DWORD)iWait);
    switch (dwRetVal)
    {
    case WAIT_OBJECT_0:
        return OK;
    case WAIT_TIMEOUT:
        return WARNING;
    case WAIT_ABANDONED:
    case WAIT_FAILED:
    default:
        return ERROR_FAILURE;
    }
#endif
}
