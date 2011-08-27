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
    m_iOptions(0),
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

int SqlDatabaseBridge::Initialize(const char* pszConnString, unsigned int iOptions)
{
    // TODOTODO - options?
    m_iOptions = iOptions;
    m_sqlDatabase = gcnew SqlDatabase(gcnew System::String(pszConnString));

    int iErrCode;
    try
    {
        iErrCode = m_sqlDatabase->CreateIfNecessary() ? WARNING : OK;
    }
    catch (SqlDatabaseException^)
    {
        iErrCode = ERROR_FAILURE;
    }
    return iErrCode;
}

IDatabaseConnection* SqlDatabaseBridge::CreateConnection(TransactionIsolationLevel isoLevel)
{
    return new SqlDatabaseConnection(m_sqlDatabase, isoLevel);
}

// TODOTODO - Backups

// Backup
int SqlDatabaseBridge::Backup(IDatabaseBackupNotificationSink* pSink, bool bCheckFirst)
{
    return OK;
}

unsigned int SqlDatabaseBridge::DeleteOldBackups(Seconds iNumSecondsOld)
{
    return 0;
}

IDatabaseBackupEnumerator* SqlDatabaseBridge::GetBackupEnumerator()
{
    return NULL;
}

int SqlDatabaseBridge::RestoreBackup(IDatabaseBackup* pBackup)
{
    return OK;
}

int SqlDatabaseBridge::DeleteBackup(IDatabaseBackup* pBackup)
{
    return OK;
}

////////////////
// Accounting //
////////////////

const char* SqlDatabaseBridge::GetConnectionString()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

unsigned int SqlDatabaseBridge::GetOptions()
{
    return m_iOptions;
}

unsigned int SqlDatabaseBridge::GetNumTables()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

unsigned int SqlDatabaseBridge::GetNumTemplates()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

ITableEnumerator* SqlDatabaseBridge::GetTableEnumerator()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

int SqlDatabaseBridge::Flush()
{
    // TODOTODO - Needs implementation
    return OK;
}

int SqlDatabaseBridge::Check()
{
    // TODOTODO - Needs implementation
    return OK;
}

// Stats
int SqlDatabaseBridge::GetStatistics(DatabaseStatistics* pdsStats)
{
    // TODOTODO - Needs implementation
    memset(pdsStats, 0, sizeof(*pdsStats));
    return OK;
}
