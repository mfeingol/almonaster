// OS.cpp: implementation of the OS namespace.
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
#include "String.h"
#undef OSAL_BUILD

#include <stdio.h>

static HINSTANCE g_hPSApi = NULL;
static HINSTANCE g_hKernel32 = NULL;

// Fxn pointers
typedef struct _PROCESS_MEMORY_COUNTERS {
    DWORD cb;
    DWORD PageFaultCount;
    DWORD PeakWorkingSetSize;
    DWORD WorkingSetSize;
    DWORD QuotaPeakPagedPoolUsage;
    DWORD QuotaPagedPoolUsage;
    DWORD QuotaPeakNonPagedPoolUsage;
    DWORD QuotaNonPagedPoolUsage;
    DWORD PagefileUsage;
    DWORD PeakPagefileUsage;
} PROCESS_MEMORY_COUNTERS;

typedef BOOL (WINAPI *Fxn_GetProcessMemoryInfo) (
	HANDLE Process,								// handle to the process
	PROCESS_MEMORY_COUNTERS* ppsmemCounters,	// structure that receives information
	DWORD cb									// size of the structure
	);

typedef BOOL (WINAPI *Fxn_IsProcessorFeaturePresent) (
	DWORD ProcessorFeature  // specifies the processor feature
	);

typedef WINBASEAPI BOOL (WINAPI *Fxn_InitializeCriticalSectionAndSpinCount) (
    LPCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount
    );

Fxn_GetProcessMemoryInfo pGetProcessMemoryInfo = NULL;
Fxn_IsProcessorFeaturePresent pIsProcessorFeaturePresent = NULL;
Fxn_InitializeCriticalSectionAndSpinCount pInitializeCriticalSectionAndSpinCount = NULL;

class AutoLibrary {
public:
	AutoLibrary() {
		g_hPSApi = ::LoadLibrary ("psapi.dll");
		g_hKernel32 = ::LoadLibrary ("kernel32.dll");
		
		if (g_hPSApi != NULL) {
			pGetProcessMemoryInfo = (Fxn_GetProcessMemoryInfo) ::GetProcAddress (g_hPSApi, "GetProcessMemoryInfo");
		}

		if (g_hKernel32 != NULL) {
			pIsProcessorFeaturePresent = (Fxn_IsProcessorFeaturePresent) ::GetProcAddress (
				g_hKernel32, 
				"IsProcessorFeaturePresent"
				);

			pInitializeCriticalSectionAndSpinCount = (Fxn_InitializeCriticalSectionAndSpinCount)
				::GetProcAddress (g_hKernel32, "InitializeCriticalSectionAndSpinCount");
		}
	}

	~AutoLibrary() {
		if (g_hPSApi != NULL) {
			::FreeLibrary (g_hPSApi);
		}
		if (g_hKernel32 != NULL) {
			::FreeLibrary (g_hKernel32);
		}
	}
};

AutoLibrary autoLibrary;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int OS::GetOSVersion (char pszOSVersion[OS::MaxOSVersionLength]) {
	
	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	
	if (!::GetVersionEx (&osVer)) {
		return ERROR_FAILURE;
	}

	switch (osVer.dwPlatformId) {

	case VER_PLATFORM_WIN32_WINDOWS:
		
		if (osVer.dwMinorVersion == 0) {
			sprintf (pszOSVersion, "Windows 95 %s", osVer.szCSDVersion);
		} else {
			sprintf (pszOSVersion, "Windows 98 %s", osVer.szCSDVersion);
		}
		break;

	case VER_PLATFORM_WIN32_NT:

		sprintf (
			pszOSVersion, 
			"Windows NT %d.%d %s (Build %d)",
			osVer.dwMajorVersion,
			osVer.dwMinorVersion,
			osVer.szCSDVersion,
			osVer.dwBuildNumber
			);

		break;
	
	default:

		StrNCpy (pszOSVersion, "Unknown OS");
		break;
	}

	return OK;
}

int OS::GetMemoryStatistics (unsigned int* piTotalPhysicalMemory, unsigned int* piTotalFreePhysicalMemory, 
							 unsigned int* piTotalVirtualMemory, unsigned int* piTotalFreeVirtualMemory) {

	MEMORYSTATUS msStatus;
	::GlobalMemoryStatus (&msStatus);

	*piTotalPhysicalMemory = msStatus.dwTotalPhys;
	*piTotalFreePhysicalMemory = msStatus.dwAvailPhys;
	*piTotalVirtualMemory = msStatus.dwTotalPageFile;
	*piTotalFreeVirtualMemory = msStatus.dwAvailPageFile;

	return OK;
}

int OS::GetProcessorInformation (char pszProcessorInformation[MaxProcessorInfoLength], 
								 unsigned int* piNumProcessors, bool* pbMMX, unsigned int* piMHz) {

	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

	if (!::GetVersionEx (&osVer)) {
		return ERROR_FAILURE;
	}

	SYSTEM_INFO siInfo;
	::GetSystemInfo (&siInfo);

	*piNumProcessors = siInfo.dwNumberOfProcessors;
	
	if (pIsProcessorFeaturePresent != NULL) {
		*pbMMX = pIsProcessorFeaturePresent (PF_MMX_INSTRUCTIONS_AVAILABLE) == TRUE;
	} else {
		*pbMMX = false;
	}

	HKEY hKey = NULL;
	if (::RegOpenKeyExW (
		HKEY_LOCAL_MACHINE,
		L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		0,
		KEY_READ,
		&hKey
		) != ERROR_SUCCESS
		) {

		*piMHz = CPU_SPEED_UNAVAILABLE;

	} else {

		DWORD dwSize = sizeof (DWORD), dwValue;
		if (::RegQueryValueExW (
			hKey,
			L"~MHz",
			0,
			NULL,
			(LPBYTE) &dwValue,
			&dwSize
			) != ERROR_SUCCESS) {

			*piMHz = CPU_SPEED_UNAVAILABLE;

		} else {

			*piMHz = dwValue;
		}

		::RegCloseKey (hKey);
	}

	switch (osVer.dwPlatformId) {
	
	case VER_PLATFORM_WIN32_WINDOWS:
		
		// Win9X
		switch (siInfo.dwProcessorType) {

		case PROCESSOR_INTEL_386:
			StrNCpy (pszProcessorInformation, "Intel 80386 or compatible");
			break;
		case PROCESSOR_INTEL_486:
			StrNCpy (pszProcessorInformation, "Intel 80486 or compatible");
			break;
		case PROCESSOR_INTEL_PENTIUM:
			StrNCpy (pszProcessorInformation, "Intel Pentium or compatible");
			break;
		default:
			StrNCpy (pszProcessorInformation, "Unknown processor");
			break;
		}
		return OK;

	case VER_PLATFORM_WIN32_NT:
		
		// NT
		char pszProc [1024], pszRev [1024];
		if (::GetEnvironmentVariable ("PROCESSOR_IDENTIFIER", pszProc, sizeof (pszProc)) == 0) {
			return ERROR_FAILURE;
		}
			
		if (::GetEnvironmentVariable ("PROCESSOR_REVISION", pszRev, sizeof (pszRev)) == 0) {
			strcpy (pszProcessorInformation, pszProc);
		} else {
			sprintf (pszProcessorInformation, "%s revision %s", pszProc, pszRev);
		}

		return OK;

	default:

		Assert (false);
		return ERROR_FAILURE;
	}
}

void OS::Sleep (MilliSeconds iMs) {
	::Sleep (iMs);
}

void OS::Exit (int iExitCode) {
	exit (iExitCode);
}

void OS::Alert (const char* pszMessage) {

    CHAR   szMsg[256];
    HANDLE  hEventSource;
    LPCSTR  lpszStrings[2];

	DWORD dwErr = ::GetLastError();
	
	// Use event logging to log the error.
	//

	char pszProcessId[96];
	sprintf (pszProcessId, "ProcessID %d", ::GetCurrentProcessId());

	hEventSource = ::RegisterEventSource(NULL, pszProcessId);
	
	sprintf(szMsg, "%s error: %d", "Osal", dwErr);
	lpszStrings[0] = szMsg;
	lpszStrings[1] = pszMessage;
	
	if (hEventSource != NULL) {
		::ReportEvent(
			hEventSource,         // handle of event source
			EVENTLOG_ERROR_TYPE,  // event type
			0,                    // event category
			0,                    // event ID
			NULL,                 // current user's SID
			2,                    // strings in lpszStrings
			0,                    // no bytes of raw data
			lpszStrings,           // array of error strings
			NULL);                // no raw data
		
		(VOID) ::DeregisterEventSource(hEventSource);
	}
}

int OS::GetProcessMemoryStatistics (unsigned int* piPhysicalMemoryInUse, unsigned int* piVirtualMemoryInUse) {

	if (pGetProcessMemoryInfo == NULL) {
		return ERROR_FAILURE;
	}

	PROCESS_MEMORY_COUNTERS pmc;
	if (!pGetProcessMemoryInfo (::GetCurrentProcess(), &pmc, sizeof (PROCESS_MEMORY_COUNTERS))) {
		return ERROR_FAILURE;
	}

	*piPhysicalMemoryInUse = pmc.WorkingSetSize;
	*piVirtualMemoryInUse = pmc.PagefileUsage;

	return OK;
}

int OS::GetProcessTimeStatistics (Seconds* piUptime, Seconds* piCPUTime) {

	FILETIME ftCreationTime, ftExitTime, ftKernelTime, ftUserTime, ftCurrentTime;
	SYSTEMTIME stCurrentTime;

	::GetSystemTime (&stCurrentTime);

	if (!::GetProcessTimes (::GetCurrentProcess(), &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime) ||
		!::SystemTimeToFileTime (&stCurrentTime, &ftCurrentTime)) {

		*piUptime = 0;
		*piCPUTime = 0;
		return ERROR_FAILURE;
	}

	*piUptime = (Seconds) (*((unsigned __int64*) &ftCurrentTime) / 10000000 - 
		*((unsigned __int64*) &ftCreationTime) / 10000000);
	*piCPUTime = (Seconds) (*((unsigned __int64*) &ftKernelTime) / 10000000 + 
		*((unsigned __int64*) &ftUserTime) / 10000000);

	return OK;
}

int OS::GetApplicationFileName (char pszFileName[OS::MaxFileNameLength]) {

	DWORD dwError = ::GetModuleFileName (NULL, pszFileName, OS::MaxFileNameLength);
	if (dwError == 0) {
		*pszFileName = '\0';
		return ERROR_FAILURE;
	}

	return OK;
}

int OS::GetApplicationDirectory (char pszFileName[OS::MaxFileNameLength]) {

	DWORD dwError = ::GetModuleFileName (NULL, pszFileName, OS::MaxFileNameLength);
	if (dwError == 0) {
		*pszFileName = '\0';
		return ERROR_FAILURE;
	}

	char* pszLastSlash = strrchr (pszFileName, '\\');
	if (pszLastSlash != NULL) {
		*pszLastSlash = '\0';
	}

	return OK;
}

static ControlFunction g_pfxnBreakHandler = NULL;

BOOL WINAPI ControlHandler (DWORD dwCtrlType) {
    
	switch (dwCtrlType) {
		
	case CTRL_BREAK_EVENT:
	case CTRL_C_EVENT:
		
		if (g_pfxnBreakHandler == NULL) {
			return FALSE;
		}
		g_pfxnBreakHandler();
		return TRUE;
    }
    return FALSE;
}

int OS::SetBreakHandler (ControlFunction pfxnBreakHandler) {

	if (pfxnBreakHandler == NULL) {
		return ERROR_FAILURE;
	}
	g_pfxnBreakHandler = pfxnBreakHandler;
	return ::SetConsoleCtrlHandler (ControlHandler, TRUE) ? OK : ERROR_FAILURE;
}

int OS::UuidFromString (const char* pszUuid, Uuid* puuidUuid) {

	return ::UuidFromString ((unsigned char*) pszUuid, (UUID*) puuidUuid) == RPC_S_OK ? OK : ERROR_FAILURE;
}

int OS::StringFromUuid (const Uuid& uuidUuid, char pszString[OS::MaxUuidLength]) {

	unsigned char* pszRpcString;

	if (::UuidToString ((GUID*) &uuidUuid, &pszRpcString) == RPC_S_OK) {
		
		strcpy (pszString, (const char*) pszRpcString);
		RpcStringFree (&pszRpcString);
	
		return OK;
	}

	return ERROR_FAILURE;
}

int OS::CreateUuid (Uuid* puuidUuid) {

	return ::UuidCreate ((GUID*) puuidUuid) == RPC_S_OK ? OK : ERROR_FAILURE;
}

void* OS::HeapAlloc (size_t stNumBytes) {

	return new char [stNumBytes];
}

void OS::HeapFree (void* pMemory) {

	delete pMemory;
}


DWORD g_dwTls;

void OS::InitializeThreadLocalStorage() {

	g_dwTls = ::TlsAlloc();
}

void OS::FinalizeThreadLocalStorage() {

	::TlsFree (g_dwTls);
}

void OS::SetThreadLocalStorageValue (void* pVoid) {

	::TlsSetValue (g_dwTls, pVoid);
}

void* OS::GetThreadLocalStorageValue() {

	return ::TlsGetValue (g_dwTls);
}