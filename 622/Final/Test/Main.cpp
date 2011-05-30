#include <stdio.h>
#include <math.h>
#include <conio.h>

#include "Osal/Algorithm.h"
#include "Osal/File.h"
#include "Osal/String.h"
#include "Osal/Thread.h"
#include "Osal/Crypto.h"
#include "Osal/MemoryMappedFile.h"

#include "Sorting.h"
#include "TestDatabase.h"
#include "RWLocks.h"

static const char* pszDay[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
    "ERR"
};

static const char* pszDayOfWeek[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "ERROR"
};


static const char* pszMonth[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

void FormatTime (char* pszDateString, tm* ptmTime)
{
    sprintf (
        pszDateString,
        "%.2d:%.2d:%.2d %s, %.2d %s %.4d",
        ptmTime->tm_hour,
        ptmTime->tm_min,
        ptmTime->tm_sec,
        pszDay [ptmTime->tm_wday],
        ptmTime->tm_mday,
        pszMonth[ptmTime->tm_mon],
        ptmTime->tm_year + 1900
        );
}

DWORD filter (DWORD code, _EXCEPTION_POINTERS *ep)
{
    if (code == EXCEPTION_ACCESS_VIOLATION)
    {
        fprintf(stderr, "EXCEPTION_ACCESS_VIOLATION\n");
    }
    else if (code == STATUS_GUARD_PAGE_VIOLATION)
    {
        fprintf(stderr, "STATUS_GUARD_PAGE_VIOLATION\n");
    }
    else if (code == STATUS_STACK_OVERFLOW)
    {
        fprintf(stderr, "STATUS_STACK_OVERFLOW\n");
    }
    else
    {
        fprintf(stderr, "%d\n", code);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

HANDLE ghEvent = NULL;
HANDLE ghEvent2 = NULL;
LPVOID gpGuardPage = NULL;

extern void StackSmash2();

void StackSmash()
{
    char p [16];
    p[15] = 2;

    StackSmash2();
}

void StackSmash2()
{
    StackSmash();
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
    SYSTEM_INFO sSysInfo;
    GetSystemInfo(&sSysInfo);

    DWORD dwPageSize = sSysInfo.dwPageSize;

    DWORD_PTR dwStackElement = (DWORD_PTR) &dwPageSize;
    DWORD_PTR dwOffset = dwStackElement % dwPageSize;

    gpGuardPage = (LPVOID) (dwStackElement - dwOffset - 4);

    SetEvent (ghEvent);
    WaitForSingleObject (ghEvent2, INFINITE);
    CloseHandle (ghEvent2);

    __try
    {
        StackSmash();
    }
    __except (filter(GetExceptionCode(), GetExceptionInformation()))
    {
        fprintf(stderr, "Handling exception2\n");
    }

    fprintf(stderr, "Thread exiting\n");

    return 0;
}

// 0x12345000
// 0x12345678
// 0x12346000

int main() {

    ghEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
    ghEvent2 = CreateEvent (NULL, FALSE, FALSE, NULL);

    HANDLE hThread;
    DWORD dwThreadId;
    hThread = CreateThread (NULL, 4096, ThreadProc, NULL, STACK_SIZE_PARAM_IS_A_RESERVATION, &dwThreadId);
    
    WaitForSingleObject (ghEvent, INFINITE);
    CloseHandle (ghEvent);

    __try
    {
        *(char*) gpGuardPage = 2;
    }
    __except (filter(GetExceptionCode(), GetExceptionInformation()))
    {
        fprintf(stderr, "Handling exception\n");
    }

    SetEvent (ghEvent2);
    CloseHandle (hThread);

    Sleep (60 * 1000);
}





class IMathService : public IUnknown
{
public:
    virtual HRESULT Add (int iX, int iY, int* piResult) = 0;
};

class IChannelCredentials : public IUnknown
{
public:
    virtual HRESULT SetSspiCredentials (LPCWSTR pwszDomain, 
                                        LPCWSTR pswzUserName,
                                        LPCWSTR pwszPassword,
                                        DWORD dwImpLevel) = 0;
};

GUID IID_IChannelCredentials = GUID_NULL;
GUID IID_IMathService = GUID_NULL;

void Foo()
{
    HRESULT hr;
    int iResult;

    IMathService* pIMathService;
    IChannelCredentials* pIChannelCreds;

    hr = CoGetObject (L"service:address= http://localhost/MathService, " \
                      L"binding=wsProfileBinding,bindingconfig=Binding1",
                      NULL, IID_IMathService, (void**) &pIMathService);

    if (SUCCEEDED (hr))
    {
        hr = pIMathService->QueryInterface (IID_IChannelCredentials, (void**) &pIChannelCreds);
        if (SUCCEEDED (hr))
        {
            hr = pIChannelCreds->SetSspiCredentials (L"REDMOND", 
                                                     L"mfeingol", 
                                                     L"password", 
                                                     RPC_C_IMP_LEVEL_IDENTIFY);

            pIChannelCreds->Release();
            
            if (SUCCEEDED (hr))
            {
                hr = pIMathService->Add (3, 5, &iResult);
            }
        }

        pIMathService->Release();
    }
}



