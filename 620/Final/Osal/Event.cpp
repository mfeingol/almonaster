// Event.cpp: implementation of the Event class.
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
#include "Event.h"
#undef OSAL_BUILD

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Event::Event() {

#ifdef __LINUX__
    pthread_cond_init(&m_hEvent, NULL);
    pthread_mutex_init(&m_Lock, NULL);

#else if defined __WIN32__
    m_hEvent = ::CreateEvent (NULL, FALSE, FALSE, NULL);    // Auto-reset event, initially non-signalled
#endif
}

Event::~Event() {

#ifdef __LINUX__
    pthread_cond_destroy(&m_hEvent);
#else if defined __WIN32__

    if (m_hEvent != NULL) {
        ::CloseHandle (m_hEvent);
    }
#endif
}

int Event::Signal() {

#ifdef __LINUX__

    pthread_mutex_lock(&m_Lock);
    // is signal good enough? or should it be broadcast?
    pthread_cond_signal(&m_hEvent);
    pthread_mutex_unlock(&m_Lock);
    return OK;

#else if defined __WIN32__

    if (m_hEvent == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    return ::SetEvent (m_hEvent) ? OK : ERROR_FAILURE;
#endif
}

int Event::Wait() {

#ifdef __LINUX__

    pthread_mutex_lock(&m_Lock);
    pthread_cond_wait(&m_hEvent, &m_Lock);
    pthread_mutex_unlock(&m_Lock);
    return OK;

#else if defined __WIN32__

    return Wait (WAIT_INFINITE);
    
#endif
}

int Event::Wait (MilliSeconds iWait) {

#ifdef __LINUX__

    struct timespec ts;
    time_t now = time(NULL);
    ts.tv_nsec = iWait % 1000 * 1000000;
    ts.tv_sec = now + iWait / 1000;
    pthread_mutex_lock(&m_Lock);
    pthread_cond_timedwait(&m_hEvent, &m_Lock, &ts);
    pthread_mutex_unlock(&m_Lock);

#else if defined __WIN32__

    if (m_hEvent == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    DWORD dwRetVal = ::WaitForSingleObject (m_hEvent, (DWORD) iWait);
    if (dwRetVal == WAIT_TIMEOUT) {
        return WARNING;
    }
    return OK;
#endif
}