#pragma once

#include "Interface.h"

using namespace Almonaster::Database::Sql;

ref class RenameUploadedAlienIcons
{
private:
    IDataSource^ m_source;
    SqlDatabase^ m_database;
    SqlCommandManager^ m_cmd;

    System::String^ m_resourceDirectory;

    void RenameUploadedEmpireIcons();
public:
    RenameUploadedAlienIcons(IDataSource^ source, System::String^ connString, System::String^ resourceDirectory);
    void Run();
};