#pragma once

#include "Interface.h"
#include "FileDatabase.h"
#include "FileDatabaseRow.h"

ref class FileDatabaseRowEnumerator : public IEnumerator<IDataRow^>
{
private:
    IDatabase* m_pDatabase;
    IReadTable* m_pTable;
    ITemplate* m_pTemplate;

    unsigned int m_numColumns;

    unsigned int m_currentKey;
    FileDatabaseRow^ m_current;

public:
    FileDatabaseRowEnumerator(IDatabase* pDatabase, IReadTable* pTable, ITemplate* pTemplate);
    ~FileDatabaseRowEnumerator();

    property IDataRow^ Current
    {
        virtual IDataRow^ get() = IEnumerator<IDataRow^>::Current::get
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

    virtual void Reset() = IEnumerator<IDataRow^>::Reset;
    virtual bool MoveNext() = IEnumerator<IDataRow^>::MoveNext;
};

