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

#pragma once

#include "SqlDatabase.h"
#include "Osal/Variant.h"
#include "Osal/TraceLog.h"

using namespace System::Data;

System::Object^ Convert(const Variant& v);
void Convert(System::Object^ object, Variant* pv);
SqlDbType Convert(VariantType type);
IsolationLevel Convert(TransactionIsolationLevel);
System::Object^ Increment(System::Object^ original, const Variant& inc);

unsigned int* ConvertIdsToKeys(System::Collections::Generic::IEnumerable<System::Object^>^ ids, unsigned int* piCount);

extern ITraceLog* g_pTrace;
void Trace(TraceInfoLevel level, const char* pszFormat, ...);

void TraceException(System::Exception^ e);