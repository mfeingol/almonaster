#include "FileDatabaseSource.h"
#include "FileDatabaseTableEnumerator.h"

#include <msclr/auto_handle.h>
#include <msclr/marshal.h>

using namespace msclr;
using namespace msclr::interop;

const Uuid CLSID_Database = { 0x51593574, 0x72fb, 0x4363, { 0xa9, 0x1d, 0x25, 0x33, 0x38, 0xe8, 0x14, 0x68 } };
const Uuid IID_IDatabase_local = { 0x6538a8d0, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };

FileDatabaseSource::FileDatabaseSource(System::String^ databaseName)
{
    m_pDatabase = NULL;
    m_pLibFileDb = NULL;

    m_pLibFileDb = new Library();

    int iErrCode;
    iErrCode = m_pLibFileDb->Open("FileDatabase.dll");
    THROW_ON_ERROR(iErrCode);

    Fxn_CreateInstance CreateInstance = (Fxn_CreateInstance)m_pLibFileDb->GetExport("CreateInstance");
    if (!CreateInstance)
    {
        throw gcnew System::ApplicationException("Unable to get FileDatabase CreateInstance export");
    }

    IDatabase* pDatabase;
    iErrCode = CreateInstance(CLSID_Database, IID_IDatabase_local, (void**)&pDatabase);
    THROW_ON_ERROR(iErrCode);
    m_pDatabase = pDatabase;

    msclr::auto_handle<marshal_context> context = gcnew marshal_context();
    const char* pszDatabaseName = context->marshal_as<const char*>(databaseName);
    iErrCode = m_pDatabase->Initialize(pszDatabaseName, 0);
    THROW_ON_ERROR(iErrCode);
}

FileDatabaseSource::~FileDatabaseSource()
{
    SafeRelease(m_pDatabase);
    delete m_pLibFileDb;
}

System::Collections::IEnumerator^ FileDatabaseSource::GetEnumerator_nongeneric()
{
    return GetEnumerator();
}

IEnumerator<IDataTable^>^ FileDatabaseSource::GetEnumerator()
{
    return gcnew FileDatabaseTableEnumerator(m_pDatabase);
}