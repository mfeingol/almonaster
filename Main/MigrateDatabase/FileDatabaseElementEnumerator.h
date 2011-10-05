#pragma once

#include "FileDatabase.h"
#include "Interface.h"
#include "FileDatabaseElement.h"
#include "FileDatabaseColumnNameProvider.h"

ref class FileDatabaseElementEnumerator : public IEnumerator<IDataElement^>
{
private:
    static FileDatabaseColumnNameProvider^ s_columnNameProvider = gcnew FileDatabaseColumnNameProvider();

    array<IDataElement^>^ m_data;
    int m_index;

public:
    FileDatabaseElementEnumerator(ITemplate* pTemplate, array<System::Object^>^ data);
    ~FileDatabaseElementEnumerator();

    property IDataElement^ Current
    {
        virtual IDataElement^ get() = IEnumerator<IDataElement^>::Current::get
        {
            return m_data[m_index];
        }
    }

    property System::Object^ Current_nongeneric
    {
        virtual System::Object^ get() = System::Collections::IEnumerator::Current::get
        {
            return m_data[m_index];
        }
    }

    virtual void Reset() = IEnumerator<IDataElement^>::Reset;
    virtual bool MoveNext() = IEnumerator<IDataElement^>::MoveNext;
};

