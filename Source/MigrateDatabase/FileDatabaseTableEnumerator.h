#pragma once

#include "Interface.h"
#include "FileDatabase.h"

ref class FileDatabaseTableEnumerator : public IEnumerator<IDataTable^>
{
private:
    IDatabase* m_pDatabase;
    ITableEnumerator* m_pTableEnumerator;
    IDataTable^ m_current;

    int m_index;
    int m_maxIndex;

public:
    FileDatabaseTableEnumerator(IDatabase* pDatabase);
    ~FileDatabaseTableEnumerator();

    property IDataTable^ Current
    {
        virtual IDataTable^ get() = IEnumerator<IDataTable^>::Current::get
        {
            return m_current;
        }
    }

    property System::Object^ Current_nongeneric
    {
        virtual System::Object^ get() = System::Collections::IEnumerator::Current::get
        {
            return m_current;
        }
    }

    virtual void Reset() = IEnumerator<IDataTable^>::Reset;
    virtual bool MoveNext() = IEnumerator<IDataTable^>::MoveNext;
};