#include "FileDatabaseElementEnumerator.h"

FileDatabaseElementEnumerator::FileDatabaseElementEnumerator(ITemplate* pTemplate, array<System::Object^>^ data)
{
    m_index = -1;
    m_data = gcnew array<IDataElement^>(data->Length);

    FileTemplateDescription desc;
    int iErrCode = pTemplate->GetDescription(&desc);
    THROW_ON_ERROR(iErrCode);

    array<System::String^>^ columnNames = s_columnNameProvider->GetColumnNames(desc.Name);

    for (unsigned int i = 0; i < desc.NumColumns; i ++)
    {
        m_data[i] = gcnew FileDatabaseElement(columnNames[i], data[i]);
    }
}

FileDatabaseElementEnumerator::~FileDatabaseElementEnumerator()
{
}

void FileDatabaseElementEnumerator::Reset()
{
    m_index = -1;
}

bool FileDatabaseElementEnumerator::MoveNext()
{
    m_index ++;
    if (m_index >= m_data->Length)
        return false;
    return true;
}