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

#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

using namespace System::Collections::Generic;

class SqlDatabaseBridge : public IDatabase
{
private:
    gcroot<SqlDatabase^> m_sqlDatabase;
    int m_iOptions;

public:
	SqlDatabaseBridge();
	~SqlDatabaseBridge();

	static SqlDatabaseBridge* CreateInstance();

    // IDatabase
    IMPLEMENT_INTERFACE(IDatabase);

    int Initialize(const char* pszConnString, unsigned int iOptions);

    // Connections
    IDatabaseConnection* CreateConnection();

    // Utilities
    int Backup(IDatabaseBackupNotificationSink* pSink, bool bCheckFirst);
    unsigned int DeleteOldBackups(Seconds iNumSecondsOld);

    IDatabaseBackupEnumerator* GetBackupEnumerator();

    int RestoreBackup(IDatabaseBackup* pBackup);
    int DeleteBackup(IDatabaseBackup* pBackup);

    ////////////////
    // Accounting //
    ////////////////

    const char* GetConnectionString();

    unsigned int GetNumTables();
    unsigned int GetNumTemplates();

    ITableEnumerator* GetTableEnumerator();
    
    int Flush();
    int Check();

    // Stats
    int GetStatistics(DatabaseStatistics* pdsStats);
    unsigned int GetOptions();
};
