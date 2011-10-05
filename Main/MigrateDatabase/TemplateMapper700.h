#pragma once

#include "Interface.h"

using namespace System::Collections::Generic;

ref class TemplateColumnMetadata
{
public:
    property System::String^ Name;
};

ref class TemplateMetadata
{
public:
    TemplateMetadata(System::String^ name)
    {
        this->Name = name;
        this->Columns = gcnew List<TemplateColumnMetadata^>();
        this->DeletedColumns = gcnew List<IDataElement^>();
        this->RenamedColumns = gcnew List<System::Tuple<System::String^, System::String^>^>();
    }

    property System::String^ Name;
    property List<TemplateColumnMetadata^>^ Columns;
    property List<IDataElement^>^ DeletedColumns;
    property List<System::Tuple<System::String^, System::String^>^>^ RenamedColumns;
};

ref class TemplateMapper700 : IEnumerable<TemplateMetadata^>
{
private:
    Dictionary<System::String^, TemplateMetadata^>^ m_templates;

public:
    TemplateMapper700();

    property TemplateMetadata^ default[System::String^]
    {
	    TemplateMetadata^ get(System::String^ templateName)
        {
            return m_templates[templateName];
        }
    }

    virtual System::Collections::IEnumerator^ GetEnumerator_nongeneric() = System::Collections::IEnumerable::GetEnumerator;
    virtual IEnumerator<TemplateMetadata^>^ GetEnumerator();
};