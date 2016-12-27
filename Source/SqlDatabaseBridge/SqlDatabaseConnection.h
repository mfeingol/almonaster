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

#include <vcclr.h>
#include "SqlDatabase.h"

using namespace Almonaster::Database::Sql;

#include "TableCacheCollection.h"
#include "Utils.h"

class SqlDatabaseConnection : public IDatabaseConnection
{
private:
    gcroot<SqlDatabase^> m_sqlDatabase;
    gcroot<SqlCommandManager^> m_cmd;

    TableCacheCollection m_viewCollection;

public:
    SqlDatabaseConnection(SqlDatabase^ sqlDatabase, TransactionIsolationLevel isoLevel);
    ~SqlDatabaseConnection();

    // IDatabaseConnection
    IMPLEMENT_INTERFACE(IDatabaseConnection);
    
    int Commit();
    ICachedTableCollection* GetCache();

    // Table operations
    int DoesTableExist(const char* pszTableName, bool* pbExist);

    // Row operations
    int GetNumPhysicalRows(const char* pszTableName, unsigned int* piNumRows);

    // Searches
    int GetSearchKeys(const char* pszTableName, const SearchDefinition& sd, const OrderByDefinition* psdOrderBy,
                      unsigned int** ppiKey, unsigned int* piNumHits, bool* pbMore);
};