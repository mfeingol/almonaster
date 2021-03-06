// Report.cpp: implementation of the Report class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar.dll:  a component of Alajar 1.0
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
    
    int iSec = 0, iMin = 0, iHour = 0, iDay = 0, iMonth = 0, iYear = 0;
    DayOfWeek day;

    if (m_flags & (WRITE_DATE | WRITE_TIME))
    {
        Time::GetDate(&iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);
    }

    char pszDate[64] = {0};
    if (m_flags & WRITE_DATE)
    {
        char pszDay[5], pszMonth[5];
        snprintf(pszDate, countof(pszDate)-1, "%s-%s-%i", String::ItoA(iMonth, pszMonth, 10, 2), String::ItoA(iDay, pszDay, 10, 2), iYear);
    }

    char pszTime[64] = {0};
    if (m_flags & WRITE_TIME)
    {
        char pszHour[5], pszMin[5], pszSec[5];
        snprintf(pszTime, countof(pszTime)-1, "%s:%s:%s", String::ItoA(iHour, pszHour, 10, 2), String::ItoA(iMin, pszMin, 10, 2), String::ItoA(iSec, pszSec, 10, 2));
    }

    char pszDateTime[128] = {0};
    if ((m_flags & WRITE_DATE) && (m_flags & WRITE_TIME))
    {
        snprintf(pszDateTime, countof(pszDateTime)-1, "[%s,%s]", pszDate, pszTime);
    }
    else if (m_flags & WRITE_DATE)
    {
        snprintf(pszDateTime, countof(pszDateTime)-1, "[%s]", pszDate);
    }
    else if (m_flags & WRITE_TIME)
    {
        snprintf(pszDateTime, countof(pszDateTime)-1, "[%s]", pszTime);
    }

    char pszThreadId[64] = {0};
    if (m_flags & WRITE_THREAD_ID)
    {
        Thread tThread;
        Thread::GetCurrentThread(&tThread);
        snprintf(pszThreadId, countof(pszThreadId)-1, "[0x%x]", tThread.GetThreadId());
    }

    const size_t chLen = strlen(pszMessage) + 64;
    char* pszText = (char*)StackAlloc(chLen);
    memset(pszText, 0, chLen);

    const char* pszSpace = "";
    if (m_flags)
    {
        pszSpace = " ";
    }

    snprintf(pszText, chLen-1, "%s%s%s%s%s", pszDateTime, pszThreadId, pszSpace, pszMessage, PLATFORM_LINE_BREAK);
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