#pragma once

ref class TableNameInfo
{
public:

    enum class TableType
    {
        System,
        SystemEmpire,
        SystemTournament,
        Game,
        GameEmpire,
    };

    TableNameInfo(System::String^ tableName);

    property System::String^ Name;
    property TableType Type;
    property int GameClass;
    property int GameNumber;
    property int EmpireKey;
    property int TournamentKey;
};