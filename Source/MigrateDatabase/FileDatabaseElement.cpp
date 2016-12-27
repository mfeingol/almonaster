#include "FileDatabaseElement.h"

FileDatabaseElement::FileDatabaseElement(System::String^ columnName, System::Object^ value)
{
    this->Name = columnName;
    this->Value = value;
}