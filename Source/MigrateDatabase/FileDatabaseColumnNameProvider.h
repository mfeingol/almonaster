#pragma once

using namespace System::Collections::Generic;

ref class FileDatabaseColumnNameProvider
{
private:
    Dictionary<System::String^, array<System::String^>^>^ m_templateMap;
    void Populate();

public:
    FileDatabaseColumnNameProvider();

    array<System::String^>^ GetColumnNames(const char* pszTemplateName);
};

