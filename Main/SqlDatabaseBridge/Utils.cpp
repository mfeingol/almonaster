#include "Utils.h"
#include "SqlDatabase.h"

using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace System::Runtime::InteropServices;

System::Object^ Convert(const Variant& v)
{
    switch (v.GetType()) {
    case V_INT:
        return v.GetInteger();
    case V_INT64:
        return v.GetInteger64();
    case V_FLOAT:
        return v.GetFloat();
    case V_STRING:
        return gcnew System::String(v.GetString());
    default:
        Assert(false);
        return nullptr;
    }
}

void Convert(System::Object^ object, Variant* pv)
{
    System::Type^ t = object->GetType();
    if (t == System::String::typeid)
    {
        System::IntPtr intPtr = Marshal::StringToCoTaskMemAnsi((System::String^)object);
        *pv = (char*)(void*)intPtr;
        Marshal::FreeCoTaskMem(intPtr);
    }
    else if (t == System::Int32::typeid)
    {
        *pv = (int)object;
    }
    else if (t == System::Double::typeid)
    {
        *pv = (float)(double)object;
    }
    else if (t == System::Int64::typeid)
    {
        *pv = (int64)object;
    }
    else
    {
        Assert(false);
    }
}

SqlDbType Convert(VariantType type)
{
    switch(type)
    {
    case V_STRING:
        return SqlDbType::NVarChar;
    case V_INT:
        return SqlDbType::Int;
    case V_FLOAT:
        return SqlDbType::Float;
    case V_INT64:
        return SqlDbType::BigInt;
    default:
        Assert(false);
        return SqlDbType::Int;
    }
}

unsigned int* ConvertIdsToKeys(IEnumerable<int64>^ ids, unsigned int* piCount)
{
    unsigned int* piKey = NULL;
    unsigned int iCount = Enumerable::Count(ids);
    if (iCount > 0)
    {
        piKey = new unsigned int[iCount];
        Assert(piKey != NULL);

        unsigned int i = 0;
        for each (int64 id in ids)
        {
            piKey[i++] = (unsigned int)id;
        }
    }

    *piCount = iCount;
    return piKey;
}

void Trace(System::String^ fmt, ... array<System::Object^>^ params)
{
    //System::Diagnostics::Trace::WriteLine(System::String::Format(fmt, params));
}