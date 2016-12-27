// TraceLog.h: interface for trace logs
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

#pragma once

#include "IObject.h"

enum TraceInfoLevel
{
    TRACE_VERBOSE,
    TRACE_INFO,
    TRACE_WARNING,
    TRACE_ERROR,
    TRACE_ALWAYS,
};

OSAL_EXPORT extern const Uuid IID_ITraceLog;

class ITraceLog : virtual public IObject
{
public:
    virtual int Write(TraceInfoLevel level, const char* pszMessage) = 0;
};