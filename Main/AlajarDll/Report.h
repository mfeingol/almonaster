#pragma once

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD
#include "Osal/AsyncFile.h"

enum ReportFlags
{
    WRITE_NONE      = 0x00000000,
    WRITE_DATE_TIME = 0x00000001,
    WRITE_THREAD_ID = 0x00000002,
};

class Report : public ITraceLog, public ITraceLogReader
{
private:
    TraceInfoLevel m_reportTracelevel;
    AsyncFile m_fFile;
    ReportFlags m_flags;

public:
    Report();

    IMPLEMENT_TWO_INTERFACES(ITraceLog, ITraceLogReader);

    int Initialize(ReportFlags flags, TraceInfoLevel level, const char* pszFile);

    int Write(TraceInfoLevel level, const char* pszMessage);
    int GetTail(char* pszBuffer, unsigned int cbSize);
};

