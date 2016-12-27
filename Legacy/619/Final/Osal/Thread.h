// Thread.h: interface for the Thread class.
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

#if !defined(AFX_THREAD_H__17166655_AC76_11D1_9C5F_0060083E8062__INCLUDED_)
#define AFX_THREAD_H__17166655_AC76_11D1_9C5F_0060083E8062__INCLUDED_

#include <windows.h>

#include "OS.h"
#include "Time.h"

#define THREAD_CALL WINAPI
typedef int (THREAD_CALL *ThreadFunction)(void*);

class OSAL_EXPORT Thread {

protected:

	HANDLE m_hThread;
	unsigned int m_uiThreadID;

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
	int WaitForTermination (MilliSeconds msWait = INFINITE);

	int Suspend();
	int Resume();
	bool IsAlive();

	int SetPriority (ThreadPriority iPriority);
	ThreadPriority GetPriority();

	int GetReturnValue (int* piReturnValue);

	int Terminate();

	int PostMessage (int iValueOne, int iValueTwo, int iValueThree);
	int PostMessage (int iValueOne, int iValueTwo);
	int PostMessage (int iValue);

	static int GetMessage (int* piValueOne, int* piValueTwo, int* piValueThree, UTCTime* ptTime);
	static int GetMessage (int* piValueOne, int* piValueTwo, UTCTime* ptTime);
	static int GetMessage (int* piValue, UTCTime* ptTime);

	static void GetCurrentThread (Thread* ptThread);
};

#endif // !defined(AFX_THREAD_H__17166655_AC76_11D1_9C5F_0060083E8062__INCLUDED_)