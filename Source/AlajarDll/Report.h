// Report.h: interface for the Report class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar.dll
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

#pragma once

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD
#include "Osal/AsyncFile.h"

enum ReportFlags
{
    WRITE_NONE      = 0x00000000,
    WRITE_DATE      = 0x00000001,
    WRITE_TIME      = 0x00000002,
    WRITE_THREAD_ID = 0x00000004,
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

