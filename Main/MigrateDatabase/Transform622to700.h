#pragma once

#include "Interface.h"
#include "TableNameInfo.h"
#include "TemplateMapper700.h"

using namespace System;
using namespace System::Runtime::InteropServices;

ref class Transform622to700
{
private:
    delegate bool CustomRowTransform(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);

    IDataSource^ m_source;
    IDataDestination^ m_dest;

    TemplateMapper700^ m_templates700;

    Dictionary<System::String^, List<System::Tuple<IDataTable^, TableNameInfo^>^>^>^ m_tables;

    Dictionary<__int64, __int64>^ m_empireKeyMapper;
    Dictionary<System::String^, __int64>^ m_empireNameTo622IdMapper;
    Dictionary<__int64, System::String^>^ m_empire622IdToNameMapper;
    Dictionary<__int64, __int64>^ m_superClassKeyMapper;
    Dictionary<__int64, __int64>^ m_gameClassKeyMapper;
    Dictionary<__int64, __int64>^ m_tournamentKeyMapper;
    Dictionary<__int64, __int64>^ m_alienKeyMapper;
    Dictionary<__int64, __int64>^ m_themeKeyMapper;

    Dictionary<Tuple<__int64, __int64>^, __int64>^ m_teamKeyMapper;

    Dictionary<Tuple<__int64, int, __int64>^, __int64>^ m_planetKeyMapper;

    Dictionary<Tuple<__int64, int, __int64, __int64>^, __int64>^ m_shipKeyMapper;
    Dictionary<Tuple<__int64, int, __int64, __int64>^, __int64>^ m_fleetKeyMapper;

    List<Tuple<int, int>^>^ m_associations;

    System::String^ m_serverFixedHashSalt;

    void TransformTables(System::String^ currentTemplate, System::String^ destTableOverride, System::String^ idColumn,
                         Dictionary<__int64, __int64>^ idMap,
                         Dictionary<System::String^, __int64>^ nameToIdMapper,
                         Dictionary<__int64, System::String^>^ idToNameMapper,
                         Dictionary<Tuple<__int64, __int64>^, __int64>^ tournamentTeamKeyMapper,
                         Dictionary<Tuple<__int64, int, __int64>^, __int64>^ gameIdMapper,
                         Dictionary<Tuple<__int64, int, __int64, __int64>^, __int64>^ gameEmpireIdMapper,
                         CustomRowTransform^ custom);

    bool TransformColumns(TableNameInfo^ nameInfo, TemplateMetadata^ templateMeta, IDataRow^ row,
                          Dictionary<System::String^, __int64>^ nameToIdMapper,
                          Dictionary<__int64, System::String^>^ idToNameMapper,
                          CustomRowTransform^ custom,
                          [Out] List<IDataElement^>^% selected, [Out] List<IDataElement^>^% rejected);

    bool TransformSystemData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemGameClassData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemThemes(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemTournaments(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemEmpireData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemActiveGames(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemEmpireMessages(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemEmpireNukerList(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemEmpireNukedList(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemNukeList(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemLatestGames(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemEmpireActiveGames(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemTournamentTeams(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemTournamentEmpires(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformSystemEmpireTournaments(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameSecurity(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameEmpires(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameDeadEmpires(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameMap(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameEmpireData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameEmpireMessages(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameEmpireMap(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameEmpireDiplomacy(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameEmpireShips(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameIndependentShips(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);
    bool TransformGameEmpireFleets(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted);

    __int64 KeyToId(System::Object^ value)
    {
        return KeyToId((int)value);
    }

    __int64 KeyToId(int key)
    {
        return (__int64)(unsigned int)key;
    }

    int IdToKey(__int64 id)
    {
        return (int)(unsigned int)id;
    }

    __int64 GetTournament622IdByActiveGame(System::String^ gameClassGameNumber);
    __int64 GetTournament622IdByLatestGame(System::String^ name, int number);
    __int64 GetEmpire622IdFromName(System::String^ name);
    System::String^ GetEmpireNameFrom622Id(__int64 id);

    System::String^ ComputePasswordHash(System::String^ password);

    void RemapGameMapPlanetKeys();
    void RemapGameMapPlanetKey(int gameClassKey, int gameNumber, __int64 planetId622, System::String^ column, int referencePlanetKey622);

    bool IsGameActive(int gameClass, int gameNumber);
    void InsertAssociationRows();
    void InsertAvailabilityRow();

    System::String^ ColumnNamesToStringList(List<IDataElement^>^ cols);
    System::String^ ColumnRenamesToStringList(List<Tuple<System::String^, System::String^>^>^ renames);

    void EnsureIdenticalDeletionLists(TemplateMetadata^ templateMeta, List<IDataElement^>^ experimental);

public:
    Transform622to700(IDataSource^ source, IDataDestination^ dest);
    void Transform();
};