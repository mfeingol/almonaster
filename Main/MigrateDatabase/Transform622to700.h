#pragma once

#include "Interface.h"
#include "TableNameInfo.h"
#include "TemplateMapper700.h"

using namespace System::Runtime::InteropServices;

ref class Transform622to700
{
private:
    delegate void CustomRowTransform(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);

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

    void TransformTables(System::String^ currentTemplate, System::String^ destTableOverride, Dictionary<__int64, __int64>^ idMap,
                         Dictionary<System::String^, __int64>^ nameToIdMapper, Dictionary<__int64, System::String^>^ idToNameMapper,
                         CustomRowTransform^ custom);

    void TransformColumns(TemplateMetadata^ templateMeta, IDataRow^ row,
                          Dictionary<System::String^, __int64>^ nameToIdMapper, Dictionary<__int64, System::String^>^ idToNameMapper, CustomRowTransform^ custom,
                          [Out] List<IDataElement^>^% selected, [Out] List<IDataElement^>^% rejected);

    void TransformSystemData(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemGameClassData(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemThemes(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemTournaments(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemEmpireData(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemActiveGames(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemEmpireMessages(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemEmpireNukerList(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemEmpireNukedList(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemNukeList(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemLatestGames(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemEmpireActiveGames(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformSystemTournamentTeams(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameEmpires(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameDeadEmpires(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameMap(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameEmpireMessages(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameEmpireMap(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameEmpireDiplomacy(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameEmpireShips(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameIndependentShips(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);
    void TransformGameEmpireFleets(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted);

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

public:
    Transform622to700(IDataSource^ source, IDataDestination^ dest);
    void Transform();
};