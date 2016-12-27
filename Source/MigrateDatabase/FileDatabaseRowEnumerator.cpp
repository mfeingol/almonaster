#include "FileDatabaseRowEnumerator.h"

FileDatabaseRowEnumerator::FileDatabaseRowEnumerator(IDatabase* pDatabase, IReadTable* pTable, ITemplate* pTemplate)
{
    m_pDatabase = pDatabase;
    m_pDatabase->AddRef();

    m_pTable = pTable;
    m_pTable->AddRef();

    m_pTemplate = pTemplate;
    m_pTemplate->AddRef();

    m_currentKey = NO_KEY;
    m_current = nullptr;

    FileTemplateDescription desc;
    int iErrCode = m_pTemplate->GetDescription(&desc);
    THROW_ON_ERROR(iErrCode);

    m_numColumns = desc.NumColumns;
}

FileDatabaseRowEnumerator::~FileDatabaseRowEnumerator()
{
    SafeRelease(m_pTemplate);
    SafeRelease(m_pTable);
    SafeRelease(m_pDatabase);
}

void FileDatabaseRowEnumerator::Reset()
{
    m_currentKey = NO_KEY;
}

bool FileDatabaseRowEnumerator::MoveNext()
{
    int iErrCode;

    unsigned int key = m_currentKey;
    iErrCode = m_pTable->GetNextKey(key, &key);
    m_currentKey = key;
    if (iErrCode == ERROR_DATA_NOT_FOUND)
        return false;
    THROW_ON_ERROR(iErrCode);

    Variant* pvRowData;
    iErrCode = m_pTable->ReadRow(m_currentKey, &pvRowData);
    THROW_ON_ERROR(iErrCode);

    m_current = gcnew FileDatabaseRow(m_pTemplate, m_currentKey, pvRowData, m_numColumns);

    m_pDatabase->FreeData(pvRowData);
    return true;
}