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

//
// Linux specific
//

#ifdef __LINUX__

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>

#define WINAPI 
#define _MAX_PATH PATH_MAX
typedef void *HANDLE;
typedef bool BOOL;

typedef struct timeb _timeb;
typedef unsigned long DWORD;
#define _ftime ftime
#define TRUE true

#define _DEBUG
#define Assert(x) \
	do { if (!(x)) { fprintf(stderr, "Assertion %s failed at %s:%d: in function %s\n", #x, __FILE__, __LINE__, __FUNCTION__); } } while (0)

#define INVALID_SOCKET (~0)
#define ERROR_WRONG_PASSWORD 1323
typedef u_int SOCKET;

// ignores radix. assumes 10
#define itoa(i, buf, radix) (sprintf(buf, "%d", i) ? buf : buf)
#define ultoa(i, buf, radix) _ultoa(i, buf, radix)
#define _ultoa(i, buf, radix) (sprintf(buf, "%u", i) ? buf : buf)
#define _ui64toa(i, buf, radix) (sprintf(buf, "%llu", i) ? buf : buf)
#define _i64toa(i, buf, radix) (sprintf(buf, "%lld", i) ? buf : buf)

#define _atoi64 atoll
#define stricmp strcasecmp
#define _strnicmp strncasecmp
#define strnicmp strncasecmp
#define min(a,b)   (((a) < (b)) ? (a) : (b))
#define max(a,b)   (((a) > (b)) ? (a) : (b))

typedef long long int int64;
typedef unsigned long long int uint64;

#define EXPORT
#define IMPORT
#define CONTROL_CALL
#define THREAD_CALL

#define INVALID_HANDLE_VALUE (-1)

#endif


//
// Win32 specific
//
#if defined WIN32 || defined _WIN32

// Compile for NT4SP3 or later
#define _WIN32_WINNT 0x0403

#define __WIN32__

#ifdef _WIN64
#define __WIN64__
#endif

#include <windows.h>
#include <malloc.h>
#include <crtdbg.h>

#define WAIT_INFINITE INFINITE

#undef ERROR_ACCESS_DENIED

#pragma warning (disable : 4100 4127 4512 4786) 

typedef __int64 int64;
typedef unsigned __int64 uint64;

#define CONTROL_CALL WINAPI
#define THREAD_CALL WINAPI

#define Assert(x) _ASSERT(x)

#endif  // WIN32


//
// Global definitions
//

struct Uuid {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
};

typedef int MilliSeconds;
typedef int Seconds;

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

#define CPU_SPEED_UNAVAILABLE ((unsigned int) -1)
#define INVALID_NUM_PROCESSORS ((unsigned int) -1)

typedef void (CONTROL_CALL *ControlFunction)();
typedef int (THREAD_CALL *ThreadFunction)(void*);

typedef char Byte;

#define StackAlloc(stNumBytes) alloca (stNumBytes)

#define countof(x) (sizeof (x) / sizeof (x[0]))

//
// Errors
//

#define OK (0)
#define WARNING (1)

#define ERROR_FAILURE (-1)
#define ERROR_OUT_OF_MEMORY (-2)
#define ERROR_OUT_OF_DISK_SPACE (-3)
#define ERROR_NO_CLASS (-4)
#define ERROR_NO_INTERFACE (-5)
#define ERROR_NOT_IMPLEMENTED (-6)
#define ERROR_INVALID_ARGUMENT (-7)
#define ERROR_ACCESS_DENIED (-8)
#define ERROR_SMALL_BUFFER (-9)

//
// Namespace
//

class String;

namespace OS {

    enum _MaxStringLength {
        MaxFileNameLength = _MAX_PATH + 1,
        MaxMimeTypeLength = MaxFileNameLength,
        MaxUuidLength = sizeof ("{00000300-0000-0000-C000-000000000046}"),
        MaxProcessorInfoLength = 512,
        MaxOSVersionLength = 512,
        MaxTimeZoneLength = 512,
        MaxTimeLength = 16,
        MaxDateLength = 64,
        MaxGMTDateLength = 64,
        MaxCookieDateLength = 64,
        MaxSmtpDateLength = 64
    };

    OSAL_EXPORT int GetOSVersion (char pszOSVersion[OS::MaxOSVersionLength]);

    OSAL_EXPORT int GetMemoryStatistics (size_t* pstTotalPhysicalMemory, 
        size_t* pstTotalFreePhysicalMemory, size_t* pstTotalVirtualMemory, 
        size_t* pstTotalFreeVirtualMemory);
    
    OSAL_EXPORT int GetProcessMemoryStatistics (size_t* pstPhysicalMemoryInUse, size_t* pstVirtualMemoryInUse);
    OSAL_EXPORT int GetProcessTimeStatistics (Seconds* piUptime, Seconds* piCPUTime);

    OSAL_EXPORT int GetNumProcessors();
    OSAL_EXPORT int GetProcessorInformation (char pszProcessorInformation[OS::MaxProcessorInfoLength], 
        unsigned int* piNumProcessors, unsigned int* piMHz);

    OSAL_EXPORT void Sleep (MilliSeconds iMs = 0);

    OSAL_EXPORT void Exit (int iExitCode = 0);

    OSAL_EXPORT void Alert (const char* pszMessage);

    OSAL_EXPORT int GetApplicationFileName (char pszFileName[OS::MaxFileNameLength]);
    OSAL_EXPORT int GetApplicationDirectory (char pszFileName[OS::MaxFileNameLength]);

    OSAL_EXPORT int SetBreakHandler (ControlFunction pfxnControlHandler);

    OSAL_EXPORT int UuidFromString (const char* pszString, Uuid* puuidUuid);

#ifdef __WIN32__
    OSAL_EXPORT int StringFromUuid (const Uuid& uuidUuid, char pszString[OS::MaxUuidLength]);
    OSAL_EXPORT int CreateUuid (Uuid* puuidUuid);
#endif

    OSAL_EXPORT void* HeapAlloc (size_t stNumBytes);
    OSAL_EXPORT void HeapFree (void* pMemory);
};

#endif // !defined(AFX_OS_H__CB52E516_F346_11D1_9DAF_0060083E8062__INCLUDED_)