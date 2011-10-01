#include "Report.h"
#include "Osal/Thread.h"

Report::Report()
{
    m_iNumRefs = 1;
    m_flags = WRITE_NONE;
    m_reportTracelevel = TRACE_WARNING;
}

int Report::Initialize(ReportFlags flags, TraceInfoLevel level, const char* pszFile)
{
    m_flags = flags;
    m_reportTracelevel = level;
    return m_fFile.OpenAppend(pszFile);
}

int Report::Write(TraceInfoLevel level, const char* pszMessage)
{
    if (level < m_reportTracelevel)
    {
        return WARNING;
    }
    
    int iSec, iMin, iHour, iDay, iMonth, iYear;
    DayOfWeek day;
    Time::GetDate(&iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);

    Thread tThread;
    Thread::GetCurrentThread(&tThread);

    const size_t chLen = strlen(pszMessage) + 64;
    char* pszText = (char*)StackAlloc(chLen);
    memset(pszText, 0, chLen);
    char pszDay[5], pszHour[5], pszMin[5], pszSec[5], pszMonth[5];

    if (m_flags & (WRITE_DATE_TIME | WRITE_THREAD_ID))
    {
        snprintf(pszText, chLen, "[%s-%s-%i,%s:%s:%s][0x%x] %s%s",
                 String::ItoA(iMonth, pszMonth, 10, 2), 
                 String::ItoA(iDay, pszDay, 10, 2),
                 iYear,
                 String::ItoA(iHour, pszHour, 10, 2), 
                 String::ItoA(iMin, pszMin, 10, 2),
                 String::ItoA(iSec, pszSec, 10, 2),
                 tThread.GetThreadId(),
                 pszMessage,
                 PLATFORM_LINE_BREAK);
    }
    else if (m_flags & WRITE_DATE_TIME)
    {
        snprintf(pszText, chLen, "[%s-%s-%i,%s:%s:%s] %s%s",
                 String::ItoA(iMonth, pszMonth, 10, 2), 
                 String::ItoA(iDay, pszDay, 10, 2),
                 iYear,
                 String::ItoA(iHour, pszHour, 10, 2), 
                 String::ItoA(iMin, pszMin, 10, 2),
                 String::ItoA(iSec, pszSec, 10, 2),
                 pszMessage,
                 PLATFORM_LINE_BREAK);
    }
    else if (m_flags & WRITE_THREAD_ID)
    {
        snprintf(pszText, chLen, "[0x%x] %s%s",
                 tThread.GetThreadId(),
                 pszMessage,
                 PLATFORM_LINE_BREAK);
    }
    else
    {
        snprintf(pszText, chLen, "%s%s", pszMessage, PLATFORM_LINE_BREAK);
    }
 
    return m_fFile.Write(pszText, (unsigned int)strlen(pszText));
}

int Report::GetTail(char* pszBuffer, unsigned int cbSize)
{
    unsigned int cbRead = cbSize - 1;
    int iErrCode = m_fFile.Tail(pszBuffer, &cbRead);
    if (iErrCode != OK)
        return iErrCode;

    pszBuffer[cbRead] = '\0';
    return OK;
}