// Thread.cpp: implementation of the Thread class.
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
#include "Thread.h"
#undef OSAL_BUILD

#include <process.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define INVALID_THREAD (HANDLE) -1
#define INVALID_THREAD_ID (0xfffffff)

int Thread::m_piPriority [] = { THREAD_PRIORITY_LOWEST,
								THREAD_PRIORITY_BELOW_NORMAL,
								THREAD_PRIORITY_NORMAL,
								THREAD_PRIORITY_ABOVE_NORMAL,
								THREAD_PRIORITY_HIGHEST,
								THREAD_PRIORITY_TIME_CRITICAL,
								THREAD_PRIORITY_NORMAL
							  };

Thread::Thread () {

	m_hThread = INVALID_THREAD;
	m_uiThreadID = INVALID_THREAD_ID;
}

Thread::~Thread() {

	if (m_hThread != INVALID_THREAD) {
		::CloseHandle (m_hThread);
	}
}

int Thread::Start (ThreadFunction pFunctionName, void* pData, ThreadPriority iPriority) {

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
		&m_uiThreadID
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
}

int Thread::WaitForTermination (MilliSeconds msWait) {

	if (m_hThread == INVALID_THREAD) {
		return ERROR_FAILURE;
	}

	DWORD dwRetVal = ::WaitForSingleObject (m_hThread, msWait);

	if (dwRetVal == WAIT_TIMEOUT) {
		return WARNING;
	}

	return OK;
}


int Thread::Suspend() {

	if (m_hThread == INVALID_THREAD) {
		return ERROR_FAILURE;
	}

	return ::SuspendThread (m_hThread) == -1 ? ERROR_FAILURE : OK;
}

int Thread::Resume() {

	if (m_hThread == INVALID_THREAD) {
		return ERROR_FAILURE;
	}

	return ::ResumeThread (m_hThread) == -1 ? ERROR_FAILURE : OK;
}

int Thread::SetPriority (ThreadPriority iPriority) {

	if (m_hThread == INVALID_THREAD) {
		return ERROR_FAILURE;
	}

	if (::SetThreadPriority (m_hThread, m_piPriority [iPriority])) {
		return OK;
	}

	return ERROR_FAILURE;
}

Thread::ThreadPriority Thread::GetPriority() {

	if (m_hThread == INVALID_THREAD) {
		return InvalidPriority;
	}

	int i, iPriority = ::GetThreadPriority (m_hThread);

	for (i = 0; i < 6; i ++) {

		if (iPriority == m_piPriority[i]) {
			return (Thread::ThreadPriority) ((int) LowestPriority + i);
		}
	}

	return InvalidPriority;
}

bool Thread::IsAlive() {

	DWORD dwExitCode;
	if (m_hThread == INVALID_THREAD || !::GetExitCodeThread (m_hThread, &dwExitCode)) {
		return false;
	}

	return dwExitCode == STILL_ACTIVE;
}

int Thread::GetReturnValue (int* piReturnValue) {

	DWORD dwExitCode;
	if (!::GetExitCodeThread (m_hThread, &dwExitCode)) {
		return ERROR_FAILURE;
	}

	*piReturnValue = dwExitCode;
	
	return *piReturnValue == STILL_ACTIVE ? ERROR_FAILURE : OK;
}

int Thread::Terminate() {

	BOOL bRetVal = ::TerminateThread (m_hThread, 666);
	m_hThread = INVALID_THREAD;
	m_uiThreadID = INVALID_THREAD_ID;
	return bRetVal ? OK : ERROR_FAILURE;
}

int Thread::PostMessage (int iValueOne, int iValueTwo, int iValueThree) {

	if (m_uiThreadID == INVALID_THREAD_ID) {
		return ERROR_FAILURE;
	}

	return ::PostThreadMessage (m_uiThreadID, iValueOne, iValueTwo, iValueThree) ? OK : ERROR_FAILURE;
}

int Thread::PostMessage (int iValueOne, int iValueTwo) {

	return PostMessage (iValueOne, iValueTwo, 0);
}

int Thread::PostMessage (int iValue) {

	return PostMessage (iValue, 0, 0);
}

int Thread::GetMessage (int* piValueOne, int* piValueTwo, int* piValueThree, UTCTime* ptTime) {

	MSG msgMessage;
	
	if (::GetMessage (&msgMessage, NULL, 0, 0) == -1) {
		return ERROR_FAILURE;
	}
	
	*piValueOne = msgMessage.message;
	*piValueOne = msgMessage.wParam;
	*piValueTwo = msgMessage.lParam;
	*ptTime = msgMessage.time;

	return OK;
}

int Thread::GetMessage (int* piValueOne, int* piValueTwo, UTCTime* ptTime) {

	int iTemp;
	return GetMessage (piValueOne, piValueTwo, &iTemp, ptTime);
}

int Thread::GetMessage (int* piValue, UTCTime* ptTime) {

	int iTemp, iTemp2;
	return GetMessage (piValue, &iTemp, &iTemp2, ptTime);
}

void Thread::GetCurrentThread (Thread* ptThread) {
	
	ptThread->m_hThread = ::GetCurrentThread();
	ptThread->m_uiThreadID = ::GetCurrentThreadId();
}