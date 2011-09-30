#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"
#include "Utils.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

class SqlDatabaseReadTable
{
private:
    gcroot<SqlCommandManager^> m_cmd;
    gcroot<System::String^> m_tableName;

public:

    SqlDatabaseReadTable(SqlCommandManager^ cmd, System::String^ tableName);
    ~SqlDatabaseReadTable();

    int GetNumRows(unsigned int* piNumRows);

    int GetSearchKeys(const RangeSearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);
    int GetSearchKeys(const RangeSearchDefinition& sdSearch, const OrderByDefinition& sdOrderBy, unsigned int** ppiKey, unsigned int* piNumHits);
};