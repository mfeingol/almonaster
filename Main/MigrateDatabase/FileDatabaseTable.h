#pragma once

#include "FileDatabase.h"
#include "Interface.h"

ref class FileDatabaseTable : public IDataTable
{
private:
    IDatabase* m_pDatabase;
    IReadTable* m_pTable;
    ITemplate* m_pTemplate;
    System::String^ m_tableName;

public:
    FileDatabaseTable(IDatabase* pDatabase, const char* pszTableName);
    ~FileDatabaseTable();

    property System::String^ Name
    {
        virtual System::String^ get()
        {
            return m_tableName;
        }
    }

    virtual System::Collections::IEnumerator^ GetEnumerator_nongeneric() = System::Collections::IEnumerable::GetEnumerator;
    virtual IEnumerator<IDataRow^>^ GetEnumerator();
};