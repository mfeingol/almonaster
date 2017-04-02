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

#include "SqlDatabaseBridge.h"
#include "SqlDatabaseConnection.h"
#include "Utils.h"

using namespace System::Data;
using namespace System::Data::SqlClient;
using namespace System::IO;
using namespace System::Runtime::InteropServices;

SqlDatabaseBridge::SqlDatabaseBridge()
    :
    m_iNumRefs(1),
    m_pTrace(NULL)
{
}

SqlDatabaseBridge::~SqlDatabaseBridge()
{
    g_pTrace = NULL;
    SafeRelease(m_pTrace);
}

SqlDatabaseBridge* SqlDatabaseBridge::CreateInstance()
{
    return new SqlDatabaseBridge();
}

int SqlDatabaseBridge::Initialize(const char* pszConnString, ITraceLog* pTrace)
{
    m_pTrace = pTrace;
    m_pTrace->AddRef();

    // HACKHACK - Major hackery to enable static Trace() methods
    g_pTrace = m_pTrace;

    m_sqlDatabase = gcnew SqlDatabase(gcnew System::String(pszConnString));

    int iErrCode = OK;
    try
    {
        m_sqlDatabase->CreateIfNecessary();
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        iErrCode = ERROR_DATABASE_EXCEPTION;
    }
    return iErrCode;
}

IDatabaseConnection* SqlDatabaseBridge::CreateConnection(TransactionIsolationLevel isoLevel)
{
    try
    {
        return new SqlDatabaseConnection(m_sqlDatabase, isoLevel);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return NULL;
    }
}