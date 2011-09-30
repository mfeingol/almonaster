#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"

#using <SqlDatabase.dll>
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
    int CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate);
    int DeleteTable(const char* pszTableName);

    // Row operations
    int GetNumPhysicalRows(const char* pszTableName, unsigned int* piNumRows);

    // Searches
    int GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);
    int GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, const OrderByDefinition& sdOrderBy, unsigned int** ppiKey, unsigned int* piNumHits);
};