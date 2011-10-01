// OS.cpp: implementation of the OS namespace.
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
#include "String.h"
#undef OSAL_BUILD

#include <stdio.h>

#ifdef __LINUX__
#include <sys/utsname.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#endif

#ifdef __WIN32__
#include <psapi.h>
#endif


//
// Stuff to run at initialization time
//

class OsalInitialization {
private:

    OsalInitialization() {

#ifdef _WIN32

        HMODULE hModule = GetModuleHandleW (L"kernel32.dll");
        if (hModule != NULL) {

            typedef BOOL (__stdcall *Fxn_HeapSetInfo) (HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T);

            Fxn_HeapSetInfo HeapSetInfo = (Fxn_HeapSetInfo) GetProcAddress (hModule, "HeapSetInformation");
            if (HeapSetInfo != NULL) {

                ULONG ulLFH = 2;

                //BOOL f = 
                HeapSetInfo (
                    GetProcessHeap(),
                    HeapCompatibilityInformation,
                    &ulLFH,
                    sizeof (ulLFH)
                    );
            }
        }
#endif

    }

    static OsalInitialization init;
};

OsalInitialization OsalInitialization::init;

//
// OS functionality
//

int OS::GetOSVersion (char pszOSVersion[OS::MaxOSVersionLength]) {

#ifdef __LINUX__
    struct utsname name;
    if (uname(&name) < 0)
        return ERROR_FAILURE;
    snprintf(pszOSVersion, OS::MaxOSVersionLength, "%s %s %s", name.sysname, name.release, name.machine);
    return OK;

#else if defined __WIN32__

    OSVERSIONINFOEX osVer;
    osVer.dwOSVersionInfoSize = sizeof (osVer);
    
    if (!::GetVersionEx ((LPOSVERSIONINFO) &osVer)) {
        return ERROR_FAILURE;
    }

    switch (osVer.dwPlatformId) {

    case VER_PLATFORM_WIN32_WINDOWS:

        // This platform is deprecated, dammit
        Assert (!"Please upgrade to a real OS");

        if (osVer.dwMinorVersion == 0) {
            snprintf (pszOSVersion, OS::MaxOSVersionLength, "Windows 95 %s", osVer.szCSDVersion);
        } else {
            snprintf (pszOSVersion, OS::MaxOSVersionLength, "Windows 98 %s", osVer.szCSDVersion);
        }
        break;

    case VER_PLATFORM_WIN32_NT:

        snprintf (
            pszOSVersion,
            OS::MaxOSVersionLength,
            "Windows NT %d.%d %s (Build %d)",
            osVer.dwMajorVersion,
            osVer.dwMinorVersion,
            osVer.szCSDVersion,
            osVer.dwBuildNumber
            );

        break;
    
    default:

        strncpy (pszOSVersion, "Unknown OS", OS::MaxOSVersionLength);
        break;
    }

    pszOSVersion [OS::MaxOSVersionLength - 1] = '\0';

    return OK;
#endif
}

int OS::GetMemoryStatistics (size_t* pstTotalPhysicalMemory, size_t* pstTotalFreePhysicalMemory, 
                             size_t* pstTotalVirtualMemory, size_t* pstTotalFreeVirtualMemory) {

#ifdef __LINUX__

    // Some code taken from Wine
    FILE *meminfo;
    meminfo = fopen("/proc/meminfo", "r");
    if (!meminfo)
    {
        *pstTotalPhysicalMemory = 16 * 1024 * 1024;
        *pstTotalFreePhysicalMemory = 16 * 1024 * 1024;
        *pstTotalVirtualMemory = 16 * 1024 * 1024;
        *pstTotalFreeVirtualMemory = 16 * 1024 * 1024;
        return OK;
    }

    char buffer[256];
    size_t total, used, free, shared, buffers, cached;
    size_t totalPhys, availPhys, totalVirtual, availVirtual;

    totalPhys = availPhys = 0;
    totalVirtual = availVirtual = 0;
    while (fgets(buffer, sizeof(buffer), meminfo))
    {
        /* old style /proc/meminfo ... */
        if (sscanf(buffer, "Mem: %d %d %d %d %d %d", &total, &used, &free, &shared, &buffers, &cached))
        {
            totalPhys += total;
            availPhys += free + buffers + cached;
        }
        if (sscanf(buffer, "Swap: %d %d %d", &total, &used, &free))
        {
            totalVirtual += total;
            availVirtual += free;
        }

        /* new style /proc/meminfo ... */
        if (sscanf(buffer, "MemTotal: %d", &total))
            totalPhys = total*1024;
        if (sscanf(buffer, "MemFree: %d", &free))
            availPhys = free*1024;
        if (sscanf(buffer, "SwapTotal: %d", &total))
            totalVirtual = total*1024;
        if (sscanf(buffer, "SwapFree: %d", &free))
            availVirtual = free*1024;
        if (sscanf(buffer, "Buffers: %d", &buffers))
            availPhys += buffers*1024;
        if (sscanf(buffer, "Cached: %d", &cached))
            availPhys += cached*1024;
    }
    fclose(meminfo);

    *pstTotalPhysicalMemory = totalPhys;
    *pstTotalFreePhysicalMemory = availPhys;
    *pstTotalVirtualMemory = totalVirtual;
    *pstTotalFreeVirtualMemory = availVirtual;

#else if defined __WIN32__

    MEMORYSTATUS memStatus;
    ::GlobalMemoryStatus (&memStatus);

    *pstTotalPhysicalMemory = memStatus.dwTotalPhys;
    *pstTotalFreePhysicalMemory = memStatus.dwAvailPhys;
    *pstTotalVirtualMemory = memStatus.dwTotalPageFile;
    *pstTotalFreeVirtualMemory = memStatus.dwAvailPageFile;

    return OK;
#endif
}


int OS::GetNumProcessors() {

    static int g_iNumProcessors = INVALID_NUM_PROCESSORS;

#ifdef __WIN32__

    // I think we can safely cache this...
    if (g_iNumProcessors == INVALID_NUM_PROCESSORS) {

        SYSTEM_INFO siInfo;
        ::GetSystemInfo (&siInfo);
        g_iNumProcessors = siInfo.dwNumberOfProcessors;
    }
    return g_iNumProcessors;

#else if defined __LINUX__

    char szProcessorInformation[MaxProcessorInfoLength];
    unsigned int iMHz;
    GetProcessorInformation(szProcessorInformation, (unsigned int *)&g_iNumProcessors, &iMHz);
    return g_iNumProcessors;

#endif
}

int OS::GetProcessorInformation (char pszProcessorInformation[MaxProcessorInfoLength], 
                                 unsigned int* piNumProcessors, unsigned int* piMHz) {

#ifdef __LINUX__
    // some code from Wine
    FILE *cpuinfo;
    char line[256];

    cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo)
        return ERROR_FAILURE;

    *pbMMX = false;
    *piMHz = 0;
    *piNumProcessors = 1;

    while (fgets(line, sizeof(line), cpuinfo) != NULL)
    {
        char	*s,*value;

        /* NOTE: the ':' is the only character we can rely on */
        if (!(value = strchr(line,':')))
            continue;
        /* terminate the valuename */
        *value++ = '\0';
        /* skip any leading spaces */
        while (*value==' ') value++;
        if ((s=strchr(value,'\n')))
            *s='\0';

        if (!strncasecmp(line, "model name", strlen("model name")))
        {
            strncpy(pszProcessorInformation, value, MaxProcessorInfoLength);
        }
        if (!strncasecmp(line, "cpu MHz", strlen("cpu MHz")))
        {
            *piMHz = atoi(value);
        }
        if (!strncasecmp(line,"processor",strlen("processor"))) {
            /* processor number counts up... */
            unsigned int x;

            if (sscanf(value,"%d",&x))
                *piNumProcessors = x+1;
        }
    }

    fclose(cpuinfo);
    return OK;

#else if defined __WIN32__

    OSVERSIONINFOEX osVer;
    osVer.dwOSVersionInfoSize = sizeof (osVer);

    if (!::GetVersionEx ((OSVERSIONINFO*) &osVer)) {
        return ERROR_FAILURE;
    }

    SYSTEM_INFO siInfo;
    ::GetSystemInfo (&siInfo);

    *piNumProcessors = siInfo.dwNumberOfProcessors;

    static unsigned int g_iMHz = CPU_SPEED_UNAVAILABLE;
    if (g_iMHz == CPU_SPEED_UNAVAILABLE) {

        HKEY hKey = NULL;
        if (::RegOpenKeyExW (
            HKEY_LOCAL_MACHINE,
            L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            0,
            KEY_READ,
            &hKey
            ) != ERROR_SUCCESS
            ) {

            g_iMHz = CPU_SPEED_UNAVAILABLE;

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

                g_iMHz = CPU_SPEED_UNAVAILABLE;

            } else {

                g_iMHz = dwValue;
            }

            ::RegCloseKey (hKey);
        }
    }

    *piMHz = g_iMHz;

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
#endif

}

void OS::Sleep (MilliSeconds iMs) {
#ifdef __LINUX__
    usleep(iMs * 1000);
#else if defined __WIN32__
    ::SleepEx(iMs, TRUE);
#endif
}

void OS::Exit (int iExitCode) {
    exit(iExitCode);
}

void OS::Alert (const char* pszMessage) {

#ifdef __LINUX__
    openlog("Osal", LOG_PERROR, LOG_USER);
    syslog(LOG_ERR, pszMessage);

#else if defined __WIN32__

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
#endif
}

int OS::GetProcessMemoryStatistics (size_t* pstPhysicalMemoryInUse, size_t* pstVirtualMemoryInUse) {

#ifdef __LINUX__
    char buf[256];
    int fd = open("/proc/self/statm", O_RDONLY);
    if (fd < 0)
        return ERROR_FAILURE;

    if (read(fd, buf, sizeof(buf)) < 0)
    {
        close(fd);
        return ERROR_FAILURE;
    }

    sscanf(buf, "%d %d", pstVirtualMemoryInUse, pstPhysicalMemoryInUse);

    // actually they are in units of pages. convert to bytes (assume 4K pages)
    *pstPhysicalMemoryInUse *= 4 * 1024;
    *pstVirtualMemoryInUse *= 4 * 1024;

    close(fd);

    return OK;

#else if defined __WIN32__

    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo (::GetCurrentProcess(), &pmc, sizeof (pmc))) {
        return ERROR_FAILURE;
    }

    *pstPhysicalMemoryInUse = pmc.WorkingSetSize;
    *pstVirtualMemoryInUse = pmc.PagefileUsage;

    return OK;
#endif
}

int OS::GetProcessTimeStatistics (Seconds* piUptime, Seconds* piCPUTime) {

#ifdef __LINUX__
    // not really implemented
    *piUptime = 0;
    *piCPUTime = 0;
    return ERROR_FAILURE;

#else if defined __WIN32__

    FILETIME ftCreationTime, ftExitTime, ftKernelTime, ftUserTime, ftCurrentTime;
    SYSTEMTIME stCurrentTime;

    ::GetSystemTime (&stCurrentTime);

    if (!::GetProcessTimes (::GetCurrentProcess(), &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime) ||
        !::SystemTimeToFileTime (&stCurrentTime, &ftCurrentTime)) {
        *piUptime = 0;
        *piCPUTime = 0;
        return ERROR_FAILURE;
    }

    *piUptime = (Seconds) (
        (((ULARGE_INTEGER*) &ftCurrentTime)->QuadPart - ((ULARGE_INTEGER*) &ftCreationTime)->QuadPart) / 10000000
        );

    *piCPUTime = (Seconds) (
        (((ULARGE_INTEGER*) &ftKernelTime)->QuadPart + ((ULARGE_INTEGER*) &ftUserTime)->QuadPart) / 10000000
        );

    return OK;

#endif
}

int OS::GetApplicationFileName (char pszFileName[OS::MaxFileNameLength]) {

#ifdef __LINUX__
    int fd = open("/proc/self/cmdline", O_RDONLY);
    if (fd < 0)
        return ERROR_FAILURE;

    read(fd, pszFileName, OS::MaxFileNameLength);

    close(fd);
    return OK;

#else if defined __WIN32__

    DWORD dwError = ::GetModuleFileName (NULL, pszFileName, OS::MaxFileNameLength);
    if (dwError == 0) {
        *pszFileName = '\0';
        return ERROR_FAILURE;
    }
    return OK;
#endif
}

int OS::GetApplicationDirectory (char pszFileName[OS::MaxFileNameLength]) {

#ifdef __LINUX__
    if (!getcwd(pszFileName, OS::MaxFileNameLength))
        return ERROR_FAILURE;
    return OK;

#else if defined __WIN32__

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
#endif
}

static ControlFunction g_pfxnBreakHandler = NULL;

#ifdef __LINUX__

void ControlHandler(int signum)
{
    if (g_pfxnBreakHandler != NULL)
        g_pfxnBreakHandler();
}

#else if defined __WIN32__

BOOL CONTROL_CALL ControlHandler (DWORD dwCtrlType) {
    
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

#endif

int OS::SetBreakHandler (ControlFunction pfxnBreakHandler) {

    if (pfxnBreakHandler == NULL) {
        return ERROR_FAILURE;
    }
    g_pfxnBreakHandler = pfxnBreakHandler;
#ifdef __LINUX__
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, ControlHandler);
    signal(SIGTERM, ControlHandler);
#else if defined __WIN32__
    return ::SetConsoleCtrlHandler (ControlHandler, TRUE) ? OK : ERROR_FAILURE;
#endif
}

int OS::UuidFromString (const char* pszUuid, Uuid* puuidUuid) {

#ifdef __LINUX__
    char *ptr;

    puuidUuid->Data1 = strtoul(pszUuid, &ptr, 16);
    puuidUuid->Data2 = strtoul(ptr+1, &ptr, 16);
    puuidUuid->Data3 = strtoul(ptr+1, &ptr, 16);

    char tempStr[3];
    tempStr[2] = '\0';
    for (int i = 0; i < 8; i++)
    {
        if (*ptr == '-')
            ptr++;
        tempStr[0] = *ptr++;
        tempStr[1] = *ptr++;
        puuidUuid->Data4[i] = strtoul(tempStr, NULL, 16);
    }
    
    return OK;

#else if defined __WIN32__
    return ::UuidFromString ((unsigned char*) pszUuid, (UUID*) puuidUuid) == RPC_S_OK ? OK : ERROR_FAILURE;
#endif
}

#ifdef __WIN32__

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

void OS::InvariantAssert(bool condition, const char* const pszMessage, const char* const pszFile, const int iLine) {

    if (!condition) {

        char* pszBuf = (char*)_alloca(strlen(pszFile) + 256);
        sprintf(pszBuf, "Assertion failed on thread %d at %s line %d\n", GetCurrentThreadId(), pszFile, iLine);
        OutputDebugStringA(pszBuf);

        if (pszMessage) {
            pszBuf = (char*)_alloca(strlen(pszMessage) + 256);
            sprintf(pszBuf, "Assertion failed on thread %d: %s\n", GetCurrentThreadId(), pszMessage);
            OutputDebugStringA(pszBuf);
        }
        DebugBreak();
    }
}

void* OS::HeapAlloc (size_t cbNumBytes) {
    return ::HeapAlloc(GetProcessHeap(), 0, cbNumBytes);
}

void OS::HeapFree (void* pMemory) {
    ::HeapFree(GetProcessHeap(), 0, pMemory);
}

#endif
