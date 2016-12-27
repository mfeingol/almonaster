#pragma once

#include "Interface.h"
#include "FileDatabase.h"
#include "Osal/Variant.h"

ref class FileDatabaseRow : public IDataRow
{
private:
    ITemplate* m_pTemplate;
    array<System::Object^>^ m_data;

public:
    FileDatabaseRow(ITemplate* pTemplate, unsigned int key, const Variant* pvRowData, unsigned int iNumColumns);
    ~FileDatabaseRow();

    virtual System::Collections::IEnumerator^ GetEnumerator_nongeneric() = System::Collections::IEnumerable::GetEnumerator;
    virtual IEnumerator<IDataElement^>^ GetEnumerator();

    virtual property __int64 Id;
};

