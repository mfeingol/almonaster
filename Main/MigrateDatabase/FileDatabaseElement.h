#pragma once

#include "Interface.h"

ref class FileDatabaseElement : public IDataElement
{
public:
    FileDatabaseElement(System::String^ columnName, System::Object^ value);

    virtual property System::String^ Name;
    virtual property System::Object^ Value;
};