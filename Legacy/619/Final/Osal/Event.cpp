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
	m_hEvent = ::CreateEvent (NULL, FALSE, FALSE, NULL);	// Auto-reset event, initially non-signalled
}

Event::~Event() {

	if (m_hEvent != NULL) {
		::CloseHandle (m_hEvent);
	}
}

int Event::Signal() {
	
	if (m_hEvent == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	return ::SetEvent (m_hEvent) ? OK : ERROR_FAILURE;
}

int Event::Wait (MilliSeconds iWait) {

	if (m_hEvent == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	DWORD dwRetVal = ::WaitForSingleObject (m_hEvent, (DWORD) iWait);

	if (dwRetVal == WAIT_TIMEOUT) {
		return WARNING;
	}

	return OK;
}

int Event::WaitForSingleSignal (Event* pEventArray, unsigned int iNumEvents, MilliSeconds msWait) {

	unsigned int i;

	HANDLE* phEvents = (HANDLE*) StackAlloc (iNumEvents * sizeof (HANDLE));

	for (i = 0; i < iNumEvents; i ++) {
		phEvents[i] = pEventArray[i].m_hEvent;
		if (phEvents[i] == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
	}

	DWORD dwRetVal = ::WaitForMultipleObjects (
		iNumEvents,
		phEvents,
		FALSE,
		(DWORD) msWait
		);

	if (dwRetVal == WAIT_TIMEOUT) {
		return WARNING;
	}

	return OK;
}

int Event::WaitForMultipleSignals (Event* pEventArray, unsigned int iNumEvents, MilliSeconds msWait) {

	unsigned int i;

	HANDLE* phEvents = (HANDLE*) StackAlloc (iNumEvents * sizeof (HANDLE));

	for (i = 0; i < iNumEvents; i ++) {
		phEvents[i] = pEventArray[i].m_hEvent;
		if (phEvents[i] == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
	}

	DWORD dwRetVal = ::WaitForMultipleObjects (
		iNumEvents,
		phEvents,
		TRUE,
		(DWORD) msWait
		);

	if (dwRetVal == WAIT_TIMEOUT) {
		return WARNING;
	}

	return OK;
}