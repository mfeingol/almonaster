#pragma once

#include "Interface.h"
#include "FileDatabase.h"
#include "Osal/Library.h"

ref class FileDatabaseSource : public IDataSource
{
private:
    Library* m_pLibFileDb;
    IDatabase* m_pDatabase;

public:
    FileDatabaseSource(System::String^ databaseName);
    ~FileDatabaseSource();

    virtual System::Collections::IEnumerator^ GetEnumerator_nongeneric() = System::Collections::IEnumerable::GetEnumerator;
    virtual IEnumerator<IDataTable^>^ GetEnumerator();
};