// MigrateDatabase.cpp : Defines the entry point for the console application.
//

#include "FileDatabaseSource.h"
#include "SqlDatabaseDestination.h"
#include "Transform622to700.h"

#using "System.Configuration.dll"
using namespace System;
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
            Console::WriteLine("FileDatabase setting not found in configuration file");
            return 0;
        }
        else
        {
            Console::WriteLine("Using source FileDatabase from '{0}'", fileDb);
        }

        if (System::String::IsNullOrEmpty(sqlDb))
        {
            Console::WriteLine("SqlDatabase setting not found in configuration file");
            return 0;
        }
        else
        {
            Console::WriteLine("Using destination SqlDatabase from '{0}'", sqlDb);
        }

        Console::WriteLine();
        Console::WriteLine("Opening databases...");

	    FileDatabaseSource^ source = gcnew FileDatabaseSource(fileDb);
        SqlDatabaseDestination^ dest = gcnew SqlDatabaseDestination(sqlDb);

        Transform622to700^ xform = gcnew Transform622to700(source, dest);
        xform->Transform();
    }
    catch (ApplicationException^ e)
    {
        Console::WriteLine(e->Message);
    }
}