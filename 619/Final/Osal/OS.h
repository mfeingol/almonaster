// OS.h: interface for the OS class.
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

#if !defined(AFX_OS_H__CB52E516_F346_11D1_9DAF_0060083E8062__INCLUDED_)
#define AFX_OS_H__CB52E516_F346_11D1_9DAF_0060083E8062__INCLUDED_

// Windows specific stuff
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <malloc.h>
#include <crtdbg.h>

#undef ERROR_ACCESS_DENIED

#pragma warning (disable : 4127 4100) 

typedef int MilliSeconds;
typedef int Seconds;

typedef __int64 int64;
typedef unsigned __int64 uint64;

#ifndef EXPORT
#define EXPORT __declspec (dllexport)
#endif

#ifndef IMPORT
#define IMPORT __declspec (dllimport)
#endif

#ifndef OSAL_EXPORT
#ifdef OSAL_BUILD
#define OSAL_EXPORT EXPORT
#else
#define OSAL_EXPORT IMPORT
#endif
#endif

struct Uuid {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
};

#define OK 0
#define WARNING (1)

#define ERROR_FAILURE (-1)
#define ERROR_OUT_OF_MEMORY (-2)
#define ERROR_OUT_OF_DISK_SPACE (-3)
#define ERROR_NO_CLASS (-4)
#define ERROR_NO_INTERFACE (-5)
#define ERROR_NOT_IMPLEMENTED (-6)
#define ERROR_INVALID_ARGUMENT (-7)
#define ERROR_ACCESS_DENIED (-8)

#define Assert(x) _ASSERT(x)

#define CPU_SPEED_UNAVAILABLE (0xffffffff)

#pragma warning (disable: 4786)

#define CONTROL_CALL WINAPI
typedef void (CONTROL_CALL *ControlFunction)();

#define StackAlloc(stNumBytes) _alloca (stNumBytes)

class String;

namespace OS {

	enum _MaxStringLength {
		MaxFileNameLength = _MAX_PATH + 1,
		MaxMimeTypeLength = MaxFileNameLength,
		MaxUuidLength = sizeof ("{00000300-0000-0000-C000-000000000046}"),
		MaxProcessorInfoLength = 4096,
		MaxOSVersionLength = 4096,
		MaxTimeZoneLength = 4096,
		MaxTimeLength = 16,
		MaxDateLength = 128,
		MaxGMTDateLength = 192,
		MaxCookieDateLength = 256,
	};

	OSAL_EXPORT int GetOSVersion (char pszOSVersion[OS::MaxOSVersionLength]);

	OSAL_EXPORT int GetMemoryStatistics (size_t* pstTotalPhysicalMemory, 
		size_t* pstTotalFreePhysicalMemory, size_t* pstTotalVirtualMemory, 
		size_t* pstTotalFreeVirtualMemory);
	
	OSAL_EXPORT int GetProcessMemoryStatistics (size_t* pstPhysicalMemoryInUse, size_t* pstVirtualMemoryInUse);

	OSAL_EXPORT int GetProcessTimeStatistics (Seconds* piUptime, Seconds* piCPUTime);

	OSAL_EXPORT int GetProcessorInformation (char pszProcessorInformation[OS::MaxProcessorInfoLength], 
		unsigned int* piNumProcessors, bool* pbMMX, unsigned int* piMHz);

	OSAL_EXPORT void Sleep (MilliSeconds iMs = 0);

	OSAL_EXPORT void Exit (int iExitCode = 0);

	OSAL_EXPORT void Alert (const char* pszMessage);

	OSAL_EXPORT int GetApplicationFileName (char pszFileName[OS::MaxFileNameLength]);
	OSAL_EXPORT int GetApplicationDirectory (char pszFileName[OS::MaxFileNameLength]);

	OSAL_EXPORT int SetBreakHandler (ControlFunction pfxnControlHandler);

	OSAL_EXPORT int UuidFromString (const char* pszString, Uuid* puuidUuid);
	OSAL_EXPORT int StringFromUuid (const Uuid& uuidUuid, char pszString[OS::MaxUuidLength]);
	OSAL_EXPORT int CreateUuid (Uuid* puuidUuid);

	OSAL_EXPORT void* HeapAlloc (size_t stNumBytes);
	OSAL_EXPORT void HeapFree (void* pMemory);

	OSAL_EXPORT void InitializeThreadLocalStorage();
	OSAL_EXPORT void FinalizeThreadLocalStorage();
	OSAL_EXPORT void SetThreadLocalStorageValue (void* pVoid);
	OSAL_EXPORT void* GetThreadLocalStorageValue();
};

#endif // !defined(AFX_OS_H__CB52E516_F346_11D1_9DAF_0060083E8062__INCLUDED_)