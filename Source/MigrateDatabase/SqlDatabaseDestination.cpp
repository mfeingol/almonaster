#include "SqlDatabaseDestination.h"
#include "TemplateMapper700.h"

using namespace System::Data;

SqlDatabaseDestination::SqlDatabaseDestination(System::String^ connString)
{
    m_sqlDatabase = gcnew SqlDatabase(connString);
    m_sqlDatabase->CreateIfNecessary();
    m_cmd = m_sqlDatabase->CreateCommandManager();
}

SqlDatabaseDestination::~SqlDatabaseDestination()
{
    delete m_cmd;
    delete m_sqlDatabase;
}

SqlDbType GetSqlDbType(System::Object^ obj)
{
    System::Type^ type = obj->GetType();
    if (type == System::String::typeid)
    {
        return SqlDbType::NVarChar;
    }
    else if (type == System::Int32::typeid)
    {
        return SqlDbType::Int;
    }
    else if (type == System::Double::typeid)
    {
        return SqlDbType::Float;
    }
    else if (type == System::Int64::typeid)
    {
        return SqlDbType::BigInt;
    }
    else
    {
        throw gcnew System::ApplicationException(System::String::Format("Invalid object type {0}", type->ToString()));
    }
}

void SqlDatabaseDestination::Clean()
{
    // Template and table names are identical in 700...
    TemplateMapper700^ templates = gcnew TemplateMapper700();

    for each (TemplateMetadata^ meta in templates)
    {
        // ... with a couple of exceptions
        if (meta->Name == "SystemEmpireNukeList")
        {
            // The one exception
            m_cmd->DeleteAllRows("SystemEmpireNukerList");
            m_cmd->DeleteAllRows("SystemEmpireNukedList");
        }
        else
        {
            m_cmd->DeleteAllRows(meta->Name);
        }
    }
}

void SqlDatabaseDestination::Commit()
{
    m_cmd->SetComplete();
}

__int64 SqlDatabaseDestination::InsertRow(System::String^ tableName, IEnumerable<IDataElement^>^ row)
{
    List<InsertValue>^ insertRow = gcnew List<InsertValue>();
    for each (IDataElement^ data in row)
    {
        InsertValue insert = { GetSqlDbType(data->Value), data->Value };
        insertRow->Add(insert);
    }

    List<IEnumerable<InsertValue>^>^ insertList = gcnew List<IEnumerable<InsertValue>^>();
    insertList->Add(insertRow);

    return m_cmd->Insert(tableName, insertList)[0];
}

void SqlDatabaseDestination::BulkWrite(IEnumerable<BulkTableWriteRequest>^ write)
{
    m_cmd->BulkWrite(write, "Id");
}