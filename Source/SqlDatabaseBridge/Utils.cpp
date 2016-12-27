//
// SqlDatabaseBridge.dll - A database library
// Copyright(c) 1998 Max Attar Feingold(maf6@cornell.edu)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or(at your option) any later version.
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

#include "Utils.h"
#include <msclr/auto_handle.h>
#include <msclr/marshal.h>

using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;

System::Object^ Convert(const Variant& v)
{
    switch (v.GetType()) {
    case V_INT:
        return v.GetInteger();
    case V_INT64:
        return v.GetInteger64();
    case V_FLOAT:
        return (double)v.GetFloat();
    case V_STRING:
        return gcnew System::String(v.GetString());
    default:
        Assert(false);
        return nullptr;
    }
}

void Convert(System::Object^ object, Variant* pv)
{
    System::Type^ t = object->GetType();
    if (t == System::String::typeid)
    {
        msclr::auto_handle<marshal_context> context = gcnew marshal_context();
        *pv = context->marshal_as<const char*>((System::String^)object);
    }
    else if (t == System::Int32::typeid)
    {
        *pv = (int)object;
    }
    else if (t == System::Double::typeid)
    {
        *pv = (float)(double)object;
    }
    else if (t == System::Int64::typeid)
    {
        *pv = (int64)object;
    }
    else
    {
        Assert(false);
    }
}

SqlDbType Convert(VariantType type)
{
    switch(type)
    {
    case V_STRING:
        return SqlDbType::NVarChar;
    case V_INT:
        return SqlDbType::Int;
    case V_FLOAT:
        return SqlDbType::Float;
    case V_INT64:
        return SqlDbType::BigInt;
    default:
        Assert(false);
        return SqlDbType::Int;
    }
}

IsolationLevel Convert(TransactionIsolationLevel isoLevel)
{
    switch (isoLevel)
    {
    case UNSPECIFIED:
        return IsolationLevel::Unspecified;
    case CHAOS:
        return IsolationLevel::Chaos;
    case READ_UNCOMMITTED:
        return IsolationLevel::ReadUncommitted;
    case READ_COMMITTED:
        return IsolationLevel::ReadCommitted;
    case REPEATABLE_READ:
        return IsolationLevel::RepeatableRead;
    case SERIALIZABLE:
        return IsolationLevel::Serializable;
    case SNAPSHOT:
        return IsolationLevel::Snapshot;
    default:
        Assert(false);
        return IsolationLevel::Unspecified;
    }
}

System::Object^ Increment(System::Object^ original, const Variant& inc)
{
    switch(inc.GetType())
    {
    case V_INT:
        return (int)original + inc.GetInteger();
    case V_FLOAT:
        return (double)original + inc.GetFloat();
    case V_INT64:
        return (int64)original + inc.GetInteger64();
    default:
        Assert(false);
        return nullptr;
    }
}

unsigned int* ConvertIdsToKeys(IEnumerable<System::Object^>^ ids, unsigned int* piCount)
{
    unsigned int* piKey = NULL;

    unsigned int iCount = Enumerable::Count(ids);
    if (iCount > 0)
    {
        piKey = new unsigned int[iCount];
        Assert(piKey != NULL);

        unsigned int i = 0;
        for each (System::Object^ id in ids)
        {
            piKey[i++] = (unsigned int)(int64)id;
        }
    }

    *piCount = iCount;
    return piKey;
}

#pragma warning(disable : 4793)
ITraceLog* g_pTrace = NULL;

void Trace(TraceInfoLevel level, const char* pszFormat, ...)
{
    va_list list;
    va_start(list, pszFormat);

    int cchLen = vsnprintf(NULL, 0, pszFormat, list);
    char* pszBuffer = (char*)StackAlloc(cchLen + 1);

    vsprintf(pszBuffer, pszFormat, list);
    g_pTrace->Write(level, pszBuffer);

    va_end(list);
}

void TraceException(System::Exception^ e)
{
    msclr::auto_handle<marshal_context> context = gcnew marshal_context();
    const char* pszMessage = context->marshal_as<const char*>(e->Message);
    Trace(TRACE_ERROR, pszMessage);
}