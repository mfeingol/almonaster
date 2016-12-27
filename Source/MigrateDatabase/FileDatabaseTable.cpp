#include "FileDatabaseTable.h"
#include "FileDatabaseRowEnumerator.h"

FileDatabaseTable::FileDatabaseTable(IDatabase* pDatabase, const char* pszTableName)
{
    m_pDatabase = pDatabase;
    m_pDatabase->AddRef();

    m_tableName = gcnew System::String(pszTableName);

    IReadTable* pTable;
    int iErrCode = pDatabase->GetTableForReading(pszTableName, &pTable);
    THROW_ON_ERROR(iErrCode);
    m_pTable = pTable;

    ITemplate* pTemplate;
    iErrCode = pDatabase->GetTemplateForTable(pszTableName, &pTemplate);
    THROW_ON_ERROR(iErrCode);
    m_pTemplate = pTemplate;
}

FileDatabaseTable::~FileDatabaseTable()
{
    SafeRelease(m_pTemplate);
    SafeRelease(m_pTable);
    SafeRelease(m_pDatabase);
}

System::Collections::IEnumerator^ FileDatabaseTable::GetEnumerator_nongeneric()
{
    return GetEnumerator();
}

IEnumerator<IDataRow^>^ FileDatabaseTable::GetEnumerator()
{
    return gcnew FileDatabaseRowEnumerator(m_pDatabase, m_pTable, m_pTemplate);
}