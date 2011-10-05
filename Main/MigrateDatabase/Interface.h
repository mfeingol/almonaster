#pragma once

#include "Osal/OS.h"

using namespace System::Collections::Generic;

typedef int (*Fxn_CreateInstance)(const Uuid& clsidClassId, const Uuid& iidInterface, void** ppObject);

#define THROW_ON_ERROR(iErrCode)    \
    if (iErrCode != OK)             \
    {                               \
        throw gcnew System::ApplicationException(gcnew System::String(System::String::Format("Error {0}", iErrCode)));    \
    }

interface class IDataElement
{
public:
    property System::String^ Name;
    property System::Object^ Value;
};

interface class IDataRow : IEnumerable<IDataElement^>
{
public:
    property __int64 Id;
};

interface class IDataTable : IEnumerable<IDataRow^>
{
public:
    property System::String^ Name
    {
        System::String^ get();
    }
};

interface class IDataSource : IEnumerable<IDataTable^>
{
};

interface class IDataDestination
{
public:
    void Clean();
    void Commit();

    __int64 InsertRow(System::String^ tableName, IEnumerable<IDataElement^>^ row);
};