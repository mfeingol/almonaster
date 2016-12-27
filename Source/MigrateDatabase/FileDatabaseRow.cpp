#include "FileDatabaseRow.h"
#include "FileDatabaseElementEnumerator.h"

System::Object^ Convert(const Variant& v)
{
    switch (v.GetType()) {
    case V_INT:
        return v.GetInteger();
    case V_INT64:
        return v.GetInteger64();
    case V_FLOAT:
        return (double)v.GetFloat();
    case V_STRING:
        return gcnew System::String(v.GetString());
    default:
        return nullptr;
    }
}

FileDatabaseRow::FileDatabaseRow(ITemplate* pTemplate, unsigned int key, const Variant* pvRowData, unsigned int iNumColumns)
{
    m_pTemplate = pTemplate;
    m_pTemplate->AddRef();

    this->Id = key;

    m_data = gcnew array<System::Object^>(iNumColumns);
    for (unsigned int i = 0; i < iNumColumns; i ++)
    {
        m_data[i] = Convert(pvRowData[i]);
    }
}

FileDatabaseRow::~FileDatabaseRow()
{
    SafeRelease(m_pTemplate);
}

System::Collections::IEnumerator^ FileDatabaseRow::GetEnumerator_nongeneric()
{
    return GetEnumerator();
}

IEnumerator<IDataElement^>^ FileDatabaseRow::GetEnumerator()
{
    return gcnew FileDatabaseElementEnumerator(m_pTemplate, m_data);
}