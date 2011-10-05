#pragma once

using namespace System;

enum class TableType
{
    System,
    SystemEmpire,
    SystemTournament,
    Game,
    GameEmpire,
};

ref class TableNameInfo
{
public:
    TableNameInfo(System::String^ tableName);

    property System::String^ Name;
    property TableType Type;
    property int GameClass;
    property int GameNumber;
    property int EmpireKey;
    property int TournamentKey;
};