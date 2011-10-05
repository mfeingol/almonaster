#include "TableNameInfo.h"

TableNameInfo::TableNameInfo(System::String^ tableName)
{
    array<wchar_t>^ digits = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9' };

    int index = tableName->IndexOfAny(digits);
    if (index == -1)
    {
        this->Type = TableType::System;
        this->Name = tableName;
    }
    else
    {
        this->Name = tableName->Substring(0, index);
        System::String^ remainder = tableName->Substring(index);

        array<System::String^>^ split = remainder->Split('.');
        switch (split->Length)
        {
        case 1:

            if (this->Name->StartsWith("SystemTournament"))
            {
                this->Type = TableType::SystemTournament;
                this->TournamentKey = Int32::Parse(split[0]);
            }
            else
            {
                this->Type = TableType::SystemEmpire;
                this->EmpireKey = Int32::Parse(split[0]);
            }
            break;

        case 2:
            this->Type = TableType::Game;
            this->GameClass = Int32::Parse(split[0]);
            this->GameNumber = Int32::Parse(split[1]);
            break;

        case 3:
            this->Type = TableType::GameEmpire;
            this->GameClass = Int32::Parse(split[0]);
            this->GameNumber = Int32::Parse(split[1]);
            this->EmpireKey = Int32::Parse(split[2]);
            break;

        default:
            throw gcnew System::ApplicationException(System::String::Format("Unrecognized table {0}", tableName));
        }
    }
}