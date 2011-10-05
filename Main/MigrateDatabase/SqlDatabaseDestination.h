#pragma once

#include "Interface.h"

using namespace Almonaster::Database::Sql;

ref class SqlDatabaseDestination : public IDataDestination
{
private:
    SqlDatabase^ m_sqlDatabase;
    SqlCommandManager^ m_cmd;

public:
    SqlDatabaseDestination(System::String^ connString);
    ~SqlDatabaseDestination();

    virtual void Clean();
    virtual void Commit();

    virtual __int64 InsertRow(System::String^ tableName, IEnumerable<IDataElement^>^ row);
};