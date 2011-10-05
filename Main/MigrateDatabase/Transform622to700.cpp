#include "Transform622to700.h"
#include "FileDatabaseElement.h"

using namespace System::Text;

System::String^ ColumnNamesToStringList(List<IDataElement^>^ cols);
System::String^ ColumnRenamesToStringList(List<Tuple<System::String^, System::String^>^>^ renames);

void EnsureIdenticalDeletionLists(TemplateMetadata^ templateMeta, List<IDataElement^>^ experimental);

// Stolen from GameEngineConstants.h, etc
#define SUPERCLASS_KEY_TOURNAMENT ((unsigned int) 0xfffffffc)
#define SUPERCLASS_KEY_PERSONAL_GAME ((unsigned int) 0xfffffffb)
#define NO_KEY ((unsigned int) 0xffffffff)
#define SYSTEM_KEY ((unsigned int) 0xffffffed)
#define INDEPENDENT_KEY ((unsigned int) 0xfffffffd)

Transform622to700::Transform622to700(IDataSource^ source, IDataDestination^ dest)
{
    m_source = source;
    m_dest = dest;

    m_tables = gcnew Dictionary<System::String^, List<Tuple<IDataTable^, TableNameInfo^>^>^>();
    m_templates700 = gcnew TemplateMapper700();

    m_empireKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_empireNameTo622IdMapper = gcnew Dictionary<System::String^, __int64>();
    m_empire622IdToNameMapper = gcnew Dictionary<__int64, System::String^>();
    m_superClassKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_superClassKeyMapper[SUPERCLASS_KEY_TOURNAMENT] = SUPERCLASS_KEY_TOURNAMENT;
    m_superClassKeyMapper[SUPERCLASS_KEY_PERSONAL_GAME] = SUPERCLASS_KEY_PERSONAL_GAME;

    m_gameClassKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_tournamentKeyMapper = gcnew Dictionary<__int64, __int64>();
}

void Transform622to700::Transform()
{
    Console::WriteLine("Cleaning destination database...");
    m_dest->Clean();
    Console::WriteLine("Destination database cleaned");

    Console::WriteLine();
    Console::WriteLine("Examining source tables...");

    // Categorize tables
    unsigned int cTables = 0;
    for each (IDataTable^ table in m_source)
    {
        TableNameInfo^ nameInfo = gcnew TableNameInfo(table->Name);
        List<Tuple<IDataTable^, TableNameInfo^>^>^ list;
        if (!m_tables->TryGetValue(nameInfo->Name, list))
        {
            list = gcnew List<Tuple<IDataTable^, TableNameInfo^>^>();
            m_tables[nameInfo->Name] = list;
        }

        list->Add(Tuple::Create(table, nameInfo));
        cTables++;

        if (cTables % 10 == 0)
        {
            Console::Write("Found {0} source tables from {1} templates\r", cTables, m_tables->Count);
        }
    }

    Console::WriteLine("Found {0} source tables from {1} templates", cTables, m_tables->Count);

    // SystemData
    TransformTables("SystemData", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemData));

    // SystemEmpireData
    TransformTables("SystemEmpireData", nullptr, m_empireKeyMapper, m_empireNameTo622IdMapper, m_empire622IdToNameMapper, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireData));

    // SystemSuperClassData
    // (MUST run before SystemGameClassData)
    TransformTables("SystemSuperClassData", nullptr, m_superClassKeyMapper, nullptr, nullptr, nullptr);

    // SystemGameClassData
    TransformTables("SystemGameClassData", nullptr, m_gameClassKeyMapper, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemGameClassData));

    // SystemAlienIcons
    TransformTables("SystemAlienIcons", nullptr, nullptr, nullptr, nullptr, nullptr);

    // SystemAlienIcons
    TransformTables("SystemThemes", nullptr, nullptr, nullptr, nullptr, nullptr);

    // SystemTournaments
    // (MUST run before SystemActiveGames)
    TransformTables("SystemTournaments", nullptr, m_tournamentKeyMapper, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemTournaments));

    // SystemActiveGames
    TransformTables("SystemActiveGames", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemActiveGames));

    // SystemEmpireMessages
    TransformTables("SystemEmpireMessages", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireMessages));

    // SystemEmpireNukerList
    TransformTables("SystemEmpireNukerList", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireNukerList));

    // SystemEmpireNukedList
    TransformTables("SystemEmpireNukedList", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireNukedList));

    // SystemNukeList
    TransformTables("SystemNukeList", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemNukeList));

    // SystemLatestGames
    TransformTables("SystemLatestGames", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemLatestGames));

    // SystemEmpireActiveGames
    TransformTables("SystemEmpireActiveGames", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireActiveGames));

    // SystemTournamentTeams
    TransformTables("SystemTournamentTeams", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemTournamentTeams));

    // SystemTournamentEmpires
    TransformTables("SystemTournamentEmpires", nullptr, nullptr, nullptr, nullptr, nullptr);

    // SystemEmpireTournaments
    TransformTables("SystemEmpireTournaments", nullptr, nullptr, nullptr, nullptr, nullptr);

    // SystemChatroomData
    TransformTables("SystemChatroomData", nullptr, nullptr, nullptr, nullptr, nullptr);

    // GameData
    TransformTables("GameData", nullptr, nullptr, nullptr, nullptr, nullptr);

    // GameSecurity
    TransformTables("GameSecurity", nullptr, nullptr, nullptr, nullptr, nullptr);

    // GameEmpires
    TransformTables("GameEmpires", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpires));

    // GameDeadEmpires
    TransformTables("GameDeadEmpires", "GameNukedEmpires", nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameDeadEmpires));

    // GameMap
    TransformTables("GameMap", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameMap));

    // GameMap
    TransformTables("GameEmpireData", nullptr, nullptr, nullptr, nullptr, nullptr);

    // GameEmpireMessages
    TransformTables("GameEmpireMessages", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireMessages));

    // GameEmpireMap
    TransformTables("GameEmpireMap", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireMap));

    // GameEmpireDiplomacy
    TransformTables("GameEmpireDiplomacy", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireDiplomacy));

    // GameEmpireShips
    TransformTables("GameEmpireShips", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireShips));

    // GameIndependentShips
    TransformTables("GameIndependentShips", "GameEmpireShips", nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameIndependentShips));

    // GameEmpireFleets
    TransformTables("GameEmpireFleets", nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireFleets));

    // TODOTODO - SystemAvailability row

    // TODOTODO - review schemas for more foreign keys that need updating

    //
    // Deletions
    //

    m_tables->Remove("SystemSystemGameClassData");
    m_tables->Remove("SystemTournamentActiveGames");
    m_tables->Remove("SystemTournamentLatestGames");

    m_tables->Remove("SystemAlmonasterScoreTopList");
    m_tables->Remove("SystemClassicScoreTopList");
    m_tables->Remove("SystemBridierScoreTopList");
    m_tables->Remove("SystemBridierScoreEstablishedTopList");
    
    //
    // Ensure all tables were processed
    //

    if (m_tables->Count > 0)
    {
        Console::WriteLine();
        Console::WriteLine("ERROR - Finished processing tables, but {0} kinds remain", m_tables->Count);
        throw gcnew ApplicationException();
    }
    else
    {
        //
        // Done!
        //

        Console::WriteLine();
        Console::WriteLine("Committing changes in destination database...");
        m_dest->Commit();
    }

    //else if (nameInfo->Name == "GameIndependentShips")
    //{
    //    // Map 622::GameIndependentShips rows to 700::GameShips rows
    //}
}

void Transform622to700::TransformTables(System::String^ currentTemplate, System::String^ destTableOverride, Dictionary<__int64, __int64>^ idMapper, 
                                        Dictionary<System::String^, __int64>^ nameToIdMapper, Dictionary<__int64, System::String^>^ idToNameMapper,
                                        CustomRowTransform^ custom)
{
    Console::WriteLine();
    Console::WriteLine("Processing tables of kind {0}...", currentTemplate);

    List<Tuple<IDataTable^, TableNameInfo^>^>^ tableList;
    if (!m_tables->TryGetValue(currentTemplate, tableList))
    {
        Console::WriteLine("No tables found");
        return;
    }

    for each (Tuple<IDataTable^, TableNameInfo^>^ tuple in tableList)
    {
        IDataTable^ table = tuple->Item1;
        TableNameInfo^ nameInfo = tuple->Item2;

        System::String^ destTable = destTableOverride;
        if (destTable == nullptr)
        {
            destTable = nameInfo->Name;
        }
        TemplateMetadata^ templateMeta = m_templates700[destTable];

        Console::WriteLine("\tProcessing table {0}: type is {1}, destination is {2}", table->Name, nameInfo->Type, destTable);
        System::String^ deletedCols = ColumnNamesToStringList(templateMeta->DeletedColumns);
        if (!System::String::IsNullOrEmpty(deletedCols))
        {
            Console::WriteLine("\tDeleted columns: {0}", deletedCols);
        }
        System::String^ renamedCols = ColumnRenamesToStringList(templateMeta->RenamedColumns);
        if (!System::String::IsNullOrEmpty(renamedCols))
        {
            Console::WriteLine("\tRenamed columns: {0}", renamedCols);
        }

        unsigned int cRows = 0;
        for each (IDataRow^ row in table)
        {
            List<IDataElement^>^ accepted, ^ rejected;
            TransformColumns(templateMeta, row, nameToIdMapper, idToNameMapper, custom, accepted, rejected);
            EnsureIdenticalDeletionLists(templateMeta, rejected);

            switch (nameInfo->Type)
            {
            case TableType::System:
                break;
            case TableType::SystemEmpire:
                accepted->Insert(0, gcnew FileDatabaseElement("EmpireKey", IdToKey(m_empireKeyMapper[KeyToId(nameInfo->EmpireKey)])));
                break;
            case TableType::SystemTournament:
                accepted->Insert(0, gcnew FileDatabaseElement("TournamentKey", IdToKey(m_tournamentKeyMapper[KeyToId(nameInfo->TournamentKey)])));
                break;
            case TableType::Game:
                accepted->Insert(0, gcnew FileDatabaseElement("GameClass", IdToKey(m_gameClassKeyMapper[KeyToId(nameInfo->GameClass)])));               
                accepted->Insert(1, gcnew FileDatabaseElement("GameNumber", nameInfo->GameNumber));
                break;
            case TableType::GameEmpire:
                accepted->Insert(0, gcnew FileDatabaseElement("GameClass", IdToKey(m_gameClassKeyMapper[KeyToId(nameInfo->GameClass)])));
                accepted->Insert(1, gcnew FileDatabaseElement("GameNumber", nameInfo->GameNumber));
                accepted->Insert(2, gcnew FileDatabaseElement("EmpireKey", IdToKey(m_empireKeyMapper[KeyToId(nameInfo->EmpireKey)])));
                break;
            }

            __int64 id622 = row->Id;
            __int64 id700 = m_dest->InsertRow(nameInfo->Name, accepted);

            if (idMapper)
            {
                idMapper->Add(id622, id700);
            }

            cRows++;
            if (cRows % 10 == 0)
            {
                Console::Write("\tInserted {0} row(s)\r", cRows);
            }
        }

        Console::WriteLine("\tInserted {0} row(s)", cRows);
    }

    m_tables->Remove(currentTemplate);
}

void Transform622to700::TransformColumns(TemplateMetadata^ templateMeta, IDataRow^ row,
                                         Dictionary<System::String^, __int64>^ nameToIdMapper, Dictionary<__int64, System::String^>^ idToNameMapper, CustomRowTransform^ custom,
                                         [Out] List<IDataElement^>^% selected, [Out] List<IDataElement^>^% rejected)
{
    selected = gcnew List<IDataElement^>();
    rejected = gcnew List<IDataElement^>();

    for each (IDataElement^ record in row)
    {
        if (idToNameMapper && System::String::Compare(record->Name, "Name") == 0)
        {
            System::String^ name = (System::String^)record->Value;
            nameToIdMapper->Add(name, row->Id);
            idToNameMapper->Add(row->Id, name);
        }

        bool found = false;
        for each (TemplateColumnMetadata^ column in templateMeta->Columns)
        {
            if (System::String::Compare(column->Name, record->Name) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            // Check for renames
            for each (Tuple<System::String^, System::String^>^ tuple in templateMeta->RenamedColumns)
            {
                if (System::String::Compare(record->Name, tuple->Item1) == 0)
                {
                    record = gcnew FileDatabaseElement(tuple->Item2, record->Value);
                    found = true;
                    break;
                }
            }
        }

        if (found)
        {
            selected->Add(record);
        }
        else
        {
            rejected->Add(record);
        }
    }

    if (custom)
    {
        custom(row, selected);
    }
}

//
// Custom transformation
//

void Transform622to700::TransformSystemData(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - add DefaultAlienKey
    // TODOTODO - add SystemMessagesAlienKey
}

void Transform622to700::TransformSystemGameClassData(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "SuperClassKey") == 0)
        {
            data->Value = IdToKey(m_superClassKeyMapper[KeyToId(data->Value)]);
            break;
        }
    }
}

void Transform622to700::TransformSystemTournaments(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - add Icon
}

void Transform622to700::TransformSystemEmpireData(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - convert Associations column to SystemEmpireAssociations insertions
    // TODOTODO - add AlienKey
}

void Transform622to700::TransformSystemActiveGames(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    System::String^ gameClassGameNumber = nullptr;
    for each (IDataElement^ data in original)
    {
        // Split GameClassGameNumber into two integer columns
        if (System::String::Compare(data->Name, "GameClassGameNumber") == 0)
        {
            // (We need this for later)
            gameClassGameNumber = (System::String^)data->Value;
            
            array<System::String^>^ split = gameClassGameNumber->Split('.');
            accepted->Insert(0, gcnew FileDatabaseElement("GameClass", IdToKey(m_gameClassKeyMapper[KeyToId(Int32::Parse(split[0]))])));
            accepted->Insert(1, gcnew FileDatabaseElement("GameNumber", Int32::Parse(split[1])));
        }
    }

    // Add TournamentKey column
    __int64 tournamentId = GetTournament622IdByActiveGame(gameClassGameNumber);
    if (tournamentId != KeyToId(NO_KEY))
    {
        tournamentId = m_tournamentKeyMapper[tournamentId];
    }
    accepted->Add(gcnew FileDatabaseElement("TournamentKey", IdToKey(tournamentId)));
}

void Transform622to700::TransformSystemEmpireMessages(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // Add SourceKey
    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "Source") == 0)
        {
            __int64 id = GetEmpire622IdFromName((System::String^)data->Value);
            if (id != KeyToId(NO_KEY))
            {
                id = m_empireKeyMapper[id];
            }
            accepted->Insert(1, gcnew FileDatabaseElement("SourceKey", IdToKey(id)));
        }
    }

    // TODOTODO - Data may contain keys in string form?
}

void Transform622to700::TransformSystemEmpireNukerList(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - add AlienKey
}

void Transform622to700::TransformSystemEmpireNukedList(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - add AlienKey
}

void Transform622to700::TransformSystemNukeList(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - add NukerAlienKey
    // TODOTODO - add NukedAlienKey
}

void Transform622to700::TransformSystemLatestGames(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    System::String^ name;
    int number;

    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "Name") == 0)
        {
            name = (System::String^)data->Value;
        }
        else if (System::String::Compare(data->Name, "Number") == 0)
        {
            number = (int)data->Value;
        }
    }

    // Add TournamentKey column
    __int64 tournamentId = GetTournament622IdByLatestGame(name, number);
    if (tournamentId != KeyToId(NO_KEY))
    {
        tournamentId = m_tournamentKeyMapper[tournamentId];
    }
    accepted->Add(gcnew FileDatabaseElement("TournamentKey", IdToKey(tournamentId)));
}

void Transform622to700::TransformSystemEmpireActiveGames(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    for each (IDataElement^ data in original)
    {
        // Split GameClassGameNumber into two integer columns
        if (System::String::Compare(data->Name, "GameClassGameNumber") == 0)
        {
            System::String^ gameClassGameNumber = (System::String^)data->Value;

            array<System::String^>^ split = gameClassGameNumber->Split('.');
            accepted->Insert(0, gcnew FileDatabaseElement("GameClass", IdToKey(m_gameClassKeyMapper[KeyToId(Int32::Parse(split[0]))])));
            accepted->Insert(1, gcnew FileDatabaseElement("GameNumber", Int32::Parse(split[1])));
        }
    }
}

void Transform622to700::TransformSystemTournamentTeams(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - Add Icon
}

void Transform622to700::TransformGameEmpires(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    for each (IDataElement^ data in original)
    {
        // Add EmpireName, map EmpireKey
        if (System::String::Compare(data->Name, "EmpireKey") == 0)
        {
            System::String^ name = GetEmpireNameFrom622Id(KeyToId(data->Value));
            accepted->Add(gcnew FileDatabaseElement("EmpireName", name));

            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
            break;
        }
    }
}

void Transform622to700::TransformGameDeadEmpires(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // Map Key
    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "Key") == 0)
        {
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
            break;
        }
    }

    // TODOTODO - add AlienKey
}

void Transform622to700::TransformGameMap(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // Map Owner
    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "Key") == 0)
        {
            __int64 id = KeyToId(data->Value);
            if (id != KeyToId(SYSTEM_KEY) && id != KeyToId(INDEPENDENT_KEY))
            {
                data->Value = IdToKey(m_empireKeyMapper[id]);
            }
            break;
        }
    }

    // TODOTODO - handle North/South/East/WestPlanetKey values - need two passes
}

void Transform622to700::TransformGameEmpireMessages(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // Add SourceKey
    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "Source") == 0)
        {
            __int64 id = GetEmpire622IdFromName((System::String^)data->Value);
            if (id != KeyToId(NO_KEY))
            {
                id = m_empireKeyMapper[id];
            }
            accepted->Insert(1, gcnew FileDatabaseElement("SourceKey", IdToKey(id)));
        }
    }
}

void Transform622to700::TransformGameEmpireMap(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - look up PlanetKey
}

void Transform622to700::TransformGameEmpireDiplomacy(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // Map EmpireKey
    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "EmpireKey") == 0)
        {
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
            break;
        }
    }
}

void Transform622to700::TransformGameEmpireShips(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - map CurrentPlanet
    // TODOTODO - map FleetKey
}

void Transform622to700::TransformGameIndependentShips(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - map CurrentPlanet
    // TODOTODO - map FleetKey

    // Add EmpireKey
    accepted->Add(gcnew FileDatabaseElement("EmpireKey", (int)INDEPENDENT_KEY));

    // Add ColonyBuildCost
    int zero = 0;
    accepted->Add(gcnew FileDatabaseElement("ColonyBuildCost", zero));
}

void Transform622to700::TransformGameEmpireFleets(IEnumerable<IDataElement^>^ original, List<IDataElement^>^ accepted)
{
    // TODOTODO - map CurrentPlanet
}

//
// Utility
//

__int64 Transform622to700::GetTournament622IdByActiveGame(System::String^ gameClassGameNumber)
{
    for each (Tuple<IDataTable^, TableNameInfo^>^ tuple in m_tables["SystemTournamentActiveGames"])
    {
        for each (IDataRow^ row in tuple->Item1)
        {
            for each (IDataElement^ data in row)
            {
                if (System::String::Compare(data->Name, "GameClassGameNumber") == 0)
                {
                    System::String^ test = (System::String^)data->Value;
                    if (System::String::Compare(gameClassGameNumber, test) == 0)
                    {
                        return row->Id;
                    }
                }
            }
        }
    }

    return KeyToId(NO_KEY);
}

__int64 Transform622to700::GetTournament622IdByLatestGame(System::String^ name, int number)
{
    for each (Tuple<IDataTable^, TableNameInfo^>^ tuple in m_tables["SystemTournamentLatestGames"])
    {
        for each (IDataRow^ row in tuple->Item1)
        {
            System::String^ testName;
            int testNumber;

            for each (IDataElement^ data in row)
            {
                if (System::String::Compare(data->Name, "Name") == 0)
                {
                    testName = (System::String^)data->Value;
                }
                else if (System::String::Compare(data->Name, "Number") == 0)
                {
                    testNumber = (int)data->Value;
                }
            }

            if (System::String::Compare(name, testName) == 0 && number == testNumber)
            {
                return KeyToId(tuple->Item2->TournamentKey);
            }
        }
    }

    return KeyToId(NO_KEY);
}

__int64 Transform622to700::GetEmpire622IdFromName(System::String^ name)
{
    __int64 id;
    if (m_empireNameTo622IdMapper->TryGetValue(name, id))
    {
        return id;
    }

    return KeyToId(NO_KEY);
}

System::String^ Transform622to700::GetEmpireNameFrom622Id(__int64 id)
{
    return m_empire622IdToNameMapper[id];
}

System::String^ ColumnNamesToStringList(List<IDataElement^>^ cols)
{
    StringBuilder^ builder = gcnew StringBuilder();
    bool first = true;
    for each (IDataElement^ col in cols)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            builder->Append(", ");
        }
        builder->Append(col->Name);
    }

    return builder->ToString();
}

System::String^ ColumnRenamesToStringList(List<Tuple<System::String^, System::String^>^>^ renames)
{
    StringBuilder^ builder = gcnew StringBuilder();
    bool first = true;
    for each (Tuple<System::String^, System::String^>^ rename in renames)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            builder->Append(", ");
        }
        builder->AppendFormat("{0} -> {1}", rename->Item1, rename->Item2);
    }

    return builder->ToString();
}

//void WriteTableInfo(TableNameInfo^ nameInfo)
//{
//    switch (nameInfo->Type)
//    {
//    case TableType::System:
//        Console::WriteLine("\tSystemTable {0}", nameInfo->Name);
//        break;
//    case TableType::SystemEmpire:
//        Console::WriteLine("\tSystemEmpire {0} - {1}", nameInfo->Name, nameInfo->EmpireKey);
//        break;
//    case TableType::Game:
//        Console::WriteLine("\tGameTable {0} - {1} - {2}", nameInfo->Name, nameInfo->GameClass, nameInfo->GameNumber);
//        break;
//    case TableType::GameEmpire:
//        Console::WriteLine("\tGameEmpireTable {0} - {1} - {2} - {3}", nameInfo->Name, nameInfo->GameClass, nameInfo->GameNumber, nameInfo->EmpireKey);
//        break;
//    }
//}

void EnsureIdenticalDeletionLists(TemplateMetadata^ templateMeta, List<IDataElement^>^ experimental)
{
    List<IDataElement^>^ missing = gcnew List<IDataElement^>();

    for each (IDataElement^ officialData in templateMeta->DeletedColumns)
    {
        bool found = false;
        for each (IDataElement^ experimentalData in experimental)
        {
            if (System::String::Compare(officialData->Name, experimentalData->Name) == 0)
            {
                found = true;
                experimental->Remove(experimentalData);
                break;
            }
        }

        if (!found)
        {
            // Check for renames
            for each (Tuple<System::String^, System::String^>^ tuple in templateMeta->RenamedColumns)
            {
                if (System::String::Compare(officialData->Name, tuple->Item1) == 0)
                {
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            missing->Add(officialData);
        }
    }

    if (missing->Count > 0)
    {
        Console::WriteLine("ERROR - couldn't find expected deleted column(s): {0}", ColumnNamesToStringList(missing));
        throw gcnew ApplicationException();
    }

    if (experimental->Count > 0)
    {
        Console::WriteLine("ERROR - found unexpected deleted column(s): {0}", ColumnNamesToStringList(experimental));
        throw gcnew ApplicationException();
    }
}