#include "FileDatabaseTableEnumerator.h"
#include "FileDatabaseTable.h"

FileDatabaseTableEnumerator::FileDatabaseTableEnumerator(IDatabase* pDatabase)
{
    m_pDatabase = pDatabase;
    m_pDatabase->AddRef();

    m_pTableEnumerator = m_pDatabase->GetTableEnumerator();
    m_pTableEnumerator->AddRef();

    m_index = -1;
    m_maxIndex = m_pTableEnumerator->GetNumTables();
}

FileDatabaseTableEnumerator::~FileDatabaseTableEnumerator()
{
    SafeRelease(m_pDatabase);
    SafeRelease(m_pTableEnumerator);
}

void FileDatabaseTableEnumerator::Reset()
{
    m_index = -1;
}

bool FileDatabaseTableEnumerator::MoveNext()
{
    m_index++;
    if (m_index >= m_maxIndex)
        return false;

    const char* pszName = m_pTableEnumerator->GetTableNames()[m_index];
    m_current = gcnew FileDatabaseTable(m_pDatabase, pszName);
    return true;
}