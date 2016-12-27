#include "RenameUploadedAlienIcons.h"

using namespace System::Data;
using namespace System::IO;

const int UPLOADED_ICON = -50;

RenameUploadedAlienIcons::RenameUploadedAlienIcons(IDataSource^ source, System::String^ connString, System::String^ resourceDirectory)
{
    m_source = source;
    m_database = gcnew SqlDatabase(connString);
    m_cmd = m_database->CreateCommandManager();
    m_resourceDirectory = resourceDirectory;
}

void RenameUploadedAlienIcons::Run()
{
    RenameUploadedEmpireIcons();
}

void RenameUploadedAlienIcons::RenameUploadedEmpireIcons()
{
    // First, rename all uploaded files
    System::String^ alienDir = Path::Combine(m_resourceDirectory, "alienuploads");
    array<System::String^>^ files = Directory::GetFiles(alienDir);

    for each (System::String^ file in files)
    {
        System::String^ directoryName = Path::GetDirectoryName(file);
        System::String^ fileName = Path::GetFileName(file);
        System::String^ tempFileName = System::String::Format("Old_{0}", fileName);

        System::Console::WriteLine("Renaming {0} to {1}", fileName, tempFileName);
        File::Move(file, Path::Combine(directoryName, tempFileName));
    }

    // Enumerate empires with uploaded icons
    int cRenamed = 0, cSkipped = 0, cEmpires = 0;
    for each (IDataTable^ table in m_source)
    {
        if (System::String::Compare(table->Name, "SystemEmpireData") == 0)
        {
            for each (IDataRow^ row in table)
            {
                cEmpires++;

                int cFound = 0, alienKey = -1;
                __int64 oldEmpireId = row->Id;
                System::String^ name = nullptr;

                for each (IDataElement^ data in row)
                {
                    if (System::String::Compare(data->Name, "Name") == 0)
                    {
                        cFound++;
                        name = (System::String^)data->Value;
                    }

                    else if (System::String::Compare(data->Name, "AlienKey") == 0)
                    {
                        cFound++;
                        alienKey = (int)data->Value;
                    }
                }

                if (cFound != 2)
                {
                    throw gcnew System::ApplicationException("Column not found");
                }

                if (cEmpires % 10 == 0)
                {
                    System::Console::Write("Processed {0} empires\r", cEmpires);
                }

                if (alienKey == UPLOADED_ICON)
                {
                    // Find new id
                    BulkTableReadRequestColumn col;
                    col.ColumnName = "Name";
                    col.ColumnValue = name;

                    List<BulkTableReadRequestColumn>^ cols = gcnew List<BulkTableReadRequestColumn>();
                    cols->Add(col);

                    BulkTableReadRequest read;
                    read.TableName = "SystemEmpireData";
                    read.Columns = cols;
                    read.CrossJoin = nullptr;

                    List<BulkTableReadRequest>^ reqs = gcnew List<BulkTableReadRequest>();
                    reqs->Add(read);

                    IEnumerable<BulkTableReadResult^>^ bulkRead = m_cmd->BulkRead(reqs);

                    __int64 newId = -1;
                    for each (BulkTableReadResult^ result in bulkRead)
                    {
                        newId = (__int64)result->Rows[0]["Id"];
                        break;
                    }

                    System::String^ oldName = System::String::Format("Old_alien{0}.gif", oldEmpireId);
                    System::String^ newName = System::String::Format("alien{0}.gif", newId);

                    if (!File::Exists(Path::Combine(alienDir, oldName)))
                    {
                        System::Console::WriteLine("SKIPPING rename of {0} to {1}", oldName, newName);
                        cSkipped ++;
                    }
                    else
                    {
                        System::Console::WriteLine("Renaming {0} to {1}", oldName, newName);

                        File::Move(Path::Combine(alienDir, oldName), Path::Combine(alienDir, newName));
                        cRenamed ++;
                    }
                }
            }
        }
    }

    System::Console::WriteLine("Fixed up {0} uploaded alien icons, skipped {1}", cRenamed, cSkipped);
}