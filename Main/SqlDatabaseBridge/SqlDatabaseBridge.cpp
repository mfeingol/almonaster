//
// SqlDatabase.dll - A database library
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
    m_iNumRefs(1)
{
}

SqlDatabaseBridge::~SqlDatabaseBridge()
{
}

SqlDatabaseBridge* SqlDatabaseBridge::CreateInstance()
{
    return new SqlDatabaseBridge();
}

int SqlDatabaseBridge::Initialize(const char* pszConnString)
{
    m_sqlDatabase = gcnew SqlDatabase(gcnew System::String(pszConnString));

    int iErrCode;
    try
    {
        iErrCode = m_sqlDatabase->CreateIfNecessary() ? WARNING : OK;
    }
    catch (SqlDatabaseException^)
    {
        iErrCode = ERROR_DATABASE_EXCEPTION;
    }
    return iErrCode;
}

IDatabaseConnection* SqlDatabaseBridge::CreateConnection(TransactionIsolationLevel isoLevel)
{
    return new SqlDatabaseConnection(m_sqlDatabase, isoLevel);
}