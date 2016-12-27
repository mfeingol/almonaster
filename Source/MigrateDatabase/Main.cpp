// MigrateDatabase.cpp : Defines the entry point for the console application.
//

#include "FileDatabaseSource.h"
#include "SqlDatabaseDestination.h"
#include "Transform622to700.h"
#include "RenameUploadedAlienIcons.h"

using namespace System::Configuration;
using namespace System::Collections::Specialized;

int main(int argc, char* argv[])
{
    try
    {
        NameValueCollection^ appSettings = ConfigurationManager::AppSettings;

        System::String^ fileDb = appSettings["FileDatabase"];
        System::String^ sqlDb = appSettings["SqlDatabase"];

        if (System::String::IsNullOrEmpty(fileDb))
        {
            System::Console::WriteLine("FileDatabase setting not found in configuration file");
            return 0;
        }
        else
        {
            System::Console::WriteLine("Using source FileDatabase from '{0}'", fileDb);
        }

        if (System::String::IsNullOrEmpty(sqlDb))
        {
            System::Console::WriteLine("SqlDatabase setting not found in configuration file");
            return 0;
        }
        else
        {
            System::Console::WriteLine("Using destination SqlDatabase from '{0}'", sqlDb);
        }

        System::Console::WriteLine();
        System::Console::WriteLine("Opening databases...");

	    FileDatabaseSource^ source = gcnew FileDatabaseSource(fileDb);
        SqlDatabaseDestination^ dest = gcnew SqlDatabaseDestination(sqlDb);

        Transform622to700^ xform = gcnew Transform622to700(source, dest);
        xform->Transform();

        //RenameUploadedAlienIcons^ rename = gcnew RenameUploadedAlienIcons(source, sqlDb, appSettings["ResourceDirectory"]);
        //rename->Run();
    }
    catch (System::ApplicationException^ e)
    {
        System::Console::WriteLine(e->Message);
    }
}