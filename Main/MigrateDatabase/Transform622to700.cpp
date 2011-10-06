#include "Osal/Algorithm.h"
#include "Osal/Crypto.h"

#include "Transform622to700.h"
#include "FileDatabaseElement.h"

#include <msclr/auto_handle.h>
#include <msclr/marshal.h>

using namespace msclr::interop;
using namespace System::IO;
using namespace System::Text;

// Stolen from GameEngineConstants.h, etc
#define SUPERCLASS_KEY_TOURNAMENT ((unsigned int) 0xfffffffc)
#define SUPERCLASS_KEY_PERSONAL_GAME ((unsigned int) 0xfffffffb)
#define NO_KEY ((unsigned int) 0xffffffff)
#define SYSTEM_KEY ((unsigned int) 0xffffffed)
#define INDEPENDENT_KEY ((unsigned int) 0xfffffffd)
#define TOURNAMENT_KEY ((unsigned int) 0xfffffffc)
#define PERSONAL_GAME ((unsigned int) 0xfffffffb)

#define INDIVIDUAL_ELEMENTS ((unsigned int)-10)
#define ALTERNATIVE_PATH    ((unsigned int)-20)
#define NULL_THEME          ((unsigned int)-30)
#define CUSTOM_COLORS       ((unsigned int)-40)
#define UPLOADED_ICON       ((unsigned int)-50)

#define HOMEWORLD (-1)
#define NOT_HOMEWORLD (-2)

enum BuilderPlanets
{
    NO_DEFAULT_BUILDER_PLANET = -1,
    HOMEWORLD_DEFAULT_BUILDER_PLANET = -2,
    LAST_BUILDER_DEFAULT_BUILDER_PLANET = -3    
};

Transform622to700::Transform622to700(IDataSource^ source, IDataDestination^ dest)
{
    m_source = source;
    m_dest = dest;

    m_tables = gcnew Dictionary<System::String^, List<Tuple<IDataTable^, TableNameInfo^>^>^>();
    m_templates700 = gcnew TemplateMapper700();

    m_empireKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_empireKeyMapper[SYSTEM_KEY] = SYSTEM_KEY;

    m_empireNameTo622IdMapper = gcnew Dictionary<System::String^, __int64>();
    m_empire622IdToNameMapper = gcnew Dictionary<__int64, System::String^>();

    m_superClassKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_superClassKeyMapper[SUPERCLASS_KEY_TOURNAMENT] = SUPERCLASS_KEY_TOURNAMENT;
    m_superClassKeyMapper[SUPERCLASS_KEY_PERSONAL_GAME] = SUPERCLASS_KEY_PERSONAL_GAME;

    m_alienKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_alienKeyMapper[UPLOADED_ICON] = UPLOADED_ICON;

    m_themeKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_themeKeyMapper[INDIVIDUAL_ELEMENTS] = INDIVIDUAL_ELEMENTS;
    m_themeKeyMapper[ALTERNATIVE_PATH] = ALTERNATIVE_PATH;
    m_themeKeyMapper[NULL_THEME] = NULL_THEME;
    m_themeKeyMapper[CUSTOM_COLORS] = CUSTOM_COLORS;

    m_teamKeyMapper = gcnew Dictionary<Tuple<__int64, __int64>^, __int64>();

    m_planetKeyMapper = gcnew Dictionary<Tuple<__int64, int, __int64>^, __int64>();

    m_shipKeyMapper = gcnew Dictionary<Tuple<__int64, int, __int64, __int64>^, __int64>();
    m_fleetKeyMapper = gcnew Dictionary<Tuple<__int64, int, __int64, __int64>^, __int64>();

    m_gameClassKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_tournamentKeyMapper = gcnew Dictionary<__int64, __int64>();
    m_tournamentKeyMapper[NO_KEY] = NO_KEY;

    m_associations = gcnew List<Tuple<int, int>^>();

    m_serverFixedHashSalt = nullptr;
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

    // SystemAlienIcons
    // (MUST run before SystemData and SystemGameClassData)
    TransformTables("SystemAlienIcons", nullptr, "AlienKey", m_alienKeyMapper, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // SystemThemes
    // (MUST run before SystemData and SystemGameClassData)
    TransformTables("SystemThemes", nullptr, nullptr, m_themeKeyMapper, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemThemes));

    // SystemData
    // (MUST run before SystemEmpireData)
    TransformTables("SystemData", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemData));

    // SystemEmpireData
    TransformTables("SystemEmpireData", nullptr, nullptr, m_empireKeyMapper, m_empireNameTo622IdMapper, m_empire622IdToNameMapper, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireData));

    // SystemSuperClassData
    // (MUST run before SystemGameClassData)
    TransformTables("SystemSuperClassData", nullptr, nullptr, m_superClassKeyMapper, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // SystemTournaments
    // (MUST run before SystemActiveGames and SystemGameClassData)
    TransformTables("SystemTournaments", nullptr, nullptr, m_tournamentKeyMapper, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemTournaments));

    // SystemGameClassData
    TransformTables("SystemGameClassData", nullptr, nullptr, m_gameClassKeyMapper, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemGameClassData));

    // SystemActiveGames
    TransformTables("SystemActiveGames", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemActiveGames));

    // SystemEmpireMessages
    TransformTables("SystemEmpireMessages", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireMessages));

    // SystemEmpireNukerList
    TransformTables("SystemEmpireNukerList", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireNukerList));

    // SystemEmpireNukedList
    TransformTables("SystemEmpireNukedList", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireNukedList));

    // SystemNukeList
    TransformTables("SystemNukeList", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemNukeList));

    // SystemLatestGames
    TransformTables("SystemLatestGames", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemLatestGames));

    // SystemEmpireActiveGames
    TransformTables("SystemEmpireActiveGames", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireActiveGames));

    // SystemTournamentTeams
    // (MUST run before SystemTournamentEmpires)
    TransformTables("SystemTournamentTeams", nullptr, nullptr, nullptr, nullptr, nullptr, m_teamKeyMapper, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemTournamentTeams));

    // SystemTournamentEmpires
    TransformTables("SystemTournamentEmpires", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemTournamentEmpires));

    // SystemEmpireTournaments
    TransformTables("SystemEmpireTournaments", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformSystemEmpireTournaments));

    // SystemChatroomData
    TransformTables("SystemChatroomData", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    // GameData
    TransformTables("GameData", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameData));

    // GameSecurity
    TransformTables("GameSecurity", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameSecurity));

    // GameEmpires
    TransformTables("GameEmpires", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpires));

    // GameDeadEmpires
    TransformTables("GameDeadEmpires", "GameNukedEmpires", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameDeadEmpires));

    // GameMap
    TransformTables("GameMap", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_planetKeyMapper, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameMap));

    // GameEmpireData
    TransformTables("GameEmpireData", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireData));

    // GameEmpireMessages
    TransformTables("GameEmpireMessages", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireMessages));

    // GameEmpireMap
    TransformTables("GameEmpireMap", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireMap));

    // GameEmpireDiplomacy
    TransformTables("GameEmpireDiplomacy", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireDiplomacy));

    // GameEmpireFleets
    // (MUST run before GameEmpireShips)
    TransformTables("GameEmpireFleets", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_fleetKeyMapper, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireFleets));

    // GameEmpireShips
    TransformTables("GameEmpireShips", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_shipKeyMapper, gcnew CustomRowTransform(this, &Transform622to700::TransformGameEmpireShips));

    // GameIndependentShips
    TransformTables("GameIndependentShips", "GameEmpireShips", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, m_shipKeyMapper, gcnew CustomRowTransform(this, &Transform622to700::TransformGameIndependentShips));

    // GameMap cardinal point keys
    RemapGameMapPlanetKeys();

    // SystemEmpireAssociations
    InsertAssociationRows();

    // SystemAvailability
    InsertAvailabilityRow();

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
        Console::WriteLine("Committing changes to destination database...");
        m_dest->Commit();
    }
}

void Transform622to700::TransformTables(System::String^ currentTemplate,
                                        System::String^ destTableOverride,
                                        System::String^ idColumn,
                                        Dictionary<__int64, __int64>^ idMapper,
                                        Dictionary<System::String^, __int64>^ nameToIdMapper,
                                        Dictionary<__int64, System::String^>^ idToNameMapper,
                                        Dictionary<Tuple<__int64, __int64>^, __int64>^ tournamentTeamKeyMapper,
                                        Dictionary<Tuple<__int64, int, __int64>^, __int64>^ gameIdMapper,
                                        Dictionary<Tuple<__int64, int, __int64, __int64>^, __int64>^ gameEmpireIdMapper,
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

        switch (nameInfo->Type)
        {
        case TableType::Game:
        case TableType::GameEmpire:
            if (!IsGameActive(nameInfo->GameClass, nameInfo->GameNumber))
            {
                Console::WriteLine("Skipping table {0}", table->Name);
                continue;
            }
            break;
        }

        unsigned int cRows = 0;
        for each (IDataRow^ row in table)
        {
            List<IDataElement^>^ accepted, ^ rejected;
            if (!TransformColumns(nameInfo, templateMeta, row, nameToIdMapper, idToNameMapper, custom, accepted, rejected))
            {
                Console::WriteLine("Skipping row {0}", row->Id);
                continue;
            }
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
                if (idColumn)
                {
                    bool bFound = false;
                    for each (IDataElement^ record in row)
                    {
                        if (System::String::Compare(record->Name, idColumn) == 0)
                        {
                            id622 = (int)record->Value;
                            bFound = true;
                            break;
                        }
                    }
                    if (!bFound)
                        throw gcnew ApplicationException("ID column not found");
                }

                idMapper->Add(id622, id700);
            }

            if (gameIdMapper)
            {
                gameIdMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, id622)] = id700;
            }

            if (gameEmpireIdMapper)
            {
                int iGameClass = nameInfo->GameClass;
                int iGameNumber = nameInfo->GameNumber;
                int iEmpireKey;
                
                if (nameInfo->Name == "GameIndependentEmpires")
                {
                    iEmpireKey = INDEPENDENT_KEY;
                }
                else
                {
                    iEmpireKey = nameInfo->EmpireKey;
                }

                gameEmpireIdMapper[Tuple::Create(KeyToId(iGameClass), iGameNumber, KeyToId(iEmpireKey), id622)] = id700;
            }

            if (tournamentTeamKeyMapper)
            {
                tournamentTeamKeyMapper[Tuple::Create(KeyToId(nameInfo->TournamentKey), row->Id)] = id700;
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

bool Transform622to700::TransformColumns(TableNameInfo^ nameInfo, TemplateMetadata^ templateMeta, IDataRow^ row,
                                         Dictionary<System::String^, __int64>^ nameToIdMapper,
                                         Dictionary<__int64, System::String^>^ idToNameMapper,
                                         CustomRowTransform^ custom,
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

        // Check for renames first
        bool found = false;
        for each (Tuple<System::String^, System::String^>^ tuple in templateMeta->RenamedColumns)
        {
            if (System::String::Compare(record->Name, tuple->Item1) == 0)
            {
                record = gcnew FileDatabaseElement(tuple->Item2, record->Value);
                found = true;
                break;
            }
        }

        if (!found)
        {
            // Make sure we're keeping the column
            for each (TemplateColumnMetadata^ column in templateMeta->Columns)
            {
                if (System::String::Compare(column->Name, record->Name) == 0)
                {
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

    bool keep = true;
    if (custom)
    {
        keep = custom(nameInfo, row, selected);
    }
    return keep;
}

//
// Custom transformation
//

bool Transform622to700::TransformSystemData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int defaultAlienKey = 0, systemMessagesAlienKey = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "DefaultAlienAddress") == 0)
        {
            cFound++;
            defaultAlienKey = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "SystemMessagesAlienAddress") == 0)
        {
            cFound++;
            systemMessagesAlienKey = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "DefaultUIButtons") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "DefaultUIBackground") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "DefaultUILivePlanet") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "DefaultUIDeadPlanet") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "DefaultUISeparator") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "DefaultUIHorz") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "DefaultUIVert") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "DefaultUIColor") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }
    }

    // Generate fixed hash salt
    char pBuffer[32];
    int iErrCode = Crypto::GetRandomData(pBuffer, sizeof(pBuffer));
    THROW_ON_ERROR(iErrCode);

    size_t cch = Algorithm::GetEncodeBase64Size(sizeof(pBuffer));
    char* pszBase64 = (char*)StackAlloc(cch);
    iErrCode = Algorithm::EncodeBase64(pBuffer, sizeof(pBuffer), pszBase64, cch);
    THROW_ON_ERROR(iErrCode);

    m_serverFixedHashSalt = gcnew System::String(pszBase64);

    accepted->Insert(0, gcnew FileDatabaseElement("DefaultAlienKey", defaultAlienKey));
    accepted->Insert(3, gcnew FileDatabaseElement("FixedHashSalt", m_serverFixedHashSalt));
    accepted->Insert(102, gcnew FileDatabaseElement("SystemMessagesAlienKey", systemMessagesAlienKey));

    if (cFound != 10)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemGameClassData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "Owner") == 0)
        {
            cFound++;
            unsigned int key = (int)data->Value;
            if (key != TOURNAMENT_KEY && key != PERSONAL_GAME)
            {
                data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
            }
        }

        else if (System::String::Compare(data->Name, "TournamentKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_tournamentKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "SuperClassKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_superClassKeyMapper[KeyToId(data->Value)]);
        }
    }

    if (cFound != 3)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemThemes(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    accepted->Insert(0, gcnew FileDatabaseElement("Address", IdToKey(original->Id)));
    return true;
}

bool Transform622to700::TransformSystemTournaments(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int icon = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "Owner") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "IconAddress") == 0)
        {
            cFound++;
            icon = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }
    }

    accepted->Insert(5, gcnew FileDatabaseElement("Icon", icon));

    if (cFound != 2)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemEmpireData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int alienKey = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "AlienAddress") == 0)
        {
            cFound++;
            alienKey = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "UIButtons") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "UIBackground") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "UILivePlanet") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "UIDeadPlanet") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "UISeparator") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "UIHorz") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "UIVert") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "UIColor") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "AlmonasterTheme") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_themeKeyMapper[KeyToId(data->Value)]);
        }
    }

    System::String^ passwordHash = nullptr;

    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "Password") == 0)
        {
            cFound++;

            passwordHash = ComputePasswordHash((System::String^)data->Value);
        }

        else if (System::String::Compare(data->Name, "Associations") == 0)
        {
            cFound++;

            System::String^ oldAssoc = (System::String^)data->Value;
            if (!System::String::IsNullOrEmpty(oldAssoc))
            {
                // Convert Associations column to SystemEmpireAssociations insertions
                array<System::String^>^ split = oldAssoc->Split(';');
                for each (System::String^ assoc in split)
                {
                    m_associations->Add(Tuple::Create(IdToKey(original->Id), Int32::Parse(assoc)));
                }
            }
        }
    }

    accepted->Insert(1, gcnew FileDatabaseElement("PasswordHash", passwordHash));
    accepted->Insert(7, gcnew FileDatabaseElement("AlienKey", alienKey));

    if (cFound != 12)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemActiveGames(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    System::String^ gameClassGameNumber = nullptr;
    int cFound = 0;

    int gameClass = 0, gameNumber = 0;

    for each (IDataElement^ data in original)
    {
        // Split GameClassGameNumber into two integer columns
        if (System::String::Compare(data->Name, "GameClassGameNumber") == 0)
        {
            cFound++;

            // (We need this for later)
            gameClassGameNumber = (System::String^)data->Value;
            
            array<System::String^>^ split = gameClassGameNumber->Split('.');
            
            gameClass = IdToKey(m_gameClassKeyMapper[KeyToId(Int32::Parse(split[0]))]);
            gameNumber = Int32::Parse(split[1]);
        }
    }

    accepted->Insert(0, gcnew FileDatabaseElement("GameClass", gameClass));
    accepted->Insert(1, gcnew FileDatabaseElement("GameNumber", gameNumber));

    // Add TournamentKey column
    __int64 tournamentId = GetTournament622IdByActiveGame(gameClassGameNumber);
    if (tournamentId != KeyToId(NO_KEY))
    {
        tournamentId = m_tournamentKeyMapper[tournamentId];
    }
    accepted->Add(gcnew FileDatabaseElement("TournamentKey", IdToKey(tournamentId)));

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemEmpireMessages(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    // Add SourceKey
    int cFound = 0;

    int sourceKey = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "SourceName") == 0)
        {
            cFound++;

            __int64 id = GetEmpire622IdFromName((System::String^)data->Value);
            if (id != KeyToId(NO_KEY))
            {
                id = m_empireKeyMapper[id];
            }
            sourceKey = IdToKey(id);
        }

        else if (System::String::Compare(data->Name, "Data") == 0)
        {
            cFound++;

            System::String^ value = (System::String^)data->Value;
            array<System::String^>^ split = value->Split('.');
            if (split->Length == 2)
            {
                int iTournamentKey = Int32::Parse(split[0]);
                int iEmpireKey = Int32::Parse(split[1]);

                __int64 tournamentId = KeyToId(iTournamentKey);
                if (!m_tournamentKeyMapper->ContainsKey(tournamentId))
                {
                    // Just delete the row
                    return false;
                }

                iTournamentKey = IdToKey(m_tournamentKeyMapper[KeyToId(iTournamentKey)]);
                iEmpireKey = IdToKey(m_empireKeyMapper[KeyToId(iEmpireKey)]);

                data->Value = System::String::Format("{0}.{1}", iTournamentKey, iEmpireKey);
            }
        }
    }

    accepted->Insert(1, gcnew FileDatabaseElement("SourceKey", sourceKey));

    if (cFound != 2)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemEmpireNukerList(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int alienKey = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "AlienAddress") == 0)
        {
            cFound++;
            alienKey = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }
    }
    
    accepted->Insert(0, gcnew FileDatabaseElement("AlienKey", alienKey));

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemEmpireNukedList(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int alienKey = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "AlienAddress") == 0)
        {
            cFound++;
            alienKey = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }
    }

    accepted->Insert(0, gcnew FileDatabaseElement("AlienKey", alienKey));

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemNukeList(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int nukerAlienKey = 0, nukedAlienKey = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "NukerAlienAddress") == 0)
        {
            cFound++;
            nukerAlienKey = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "NukedAlienAddress") == 0)
        {
            cFound++;
            nukedAlienKey = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "NukerEmpireKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "NukedEmpireKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
        }
    }

    accepted->Insert(0, gcnew FileDatabaseElement("NukerAlienKey", nukerAlienKey));
    accepted->Insert(4, gcnew FileDatabaseElement("NukedAlienKey", nukedAlienKey));


    if (cFound != 4)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemLatestGames(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    System::String^ name;
    int number = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "Name") == 0)
        {
            cFound++;
            name = (System::String^)data->Value;
        }
        else if (System::String::Compare(data->Name, "Number") == 0)
        {
            cFound++;
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

    if (cFound != 2)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemEmpireActiveGames(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int gameClass = 0, gameNumber = 0;

    for each (IDataElement^ data in original)
    {
        // Split GameClassGameNumber into two integer columns
        if (System::String::Compare(data->Name, "GameClassGameNumber") == 0)
        {
            cFound++;

            System::String^ gameClassGameNumber = (System::String^)data->Value;

            array<System::String^>^ split = gameClassGameNumber->Split('.');
            gameClass = IdToKey(m_gameClassKeyMapper[KeyToId(Int32::Parse(split[0]))]);
            gameNumber = Int32::Parse(split[1]);
        }
    }

    accepted->Insert(0, gcnew FileDatabaseElement("GameClass", gameClass));
    accepted->Insert(1, gcnew FileDatabaseElement("GameNumber", gameNumber));

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemTournamentTeams(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int icon = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "IconAddress") == 0)
        {
            cFound++;
            icon = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }
    }

    accepted->Insert(3, gcnew FileDatabaseElement("Icon", icon));

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemTournamentEmpires(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "EmpireKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "TeamKey") == 0)
        {
            cFound++;
            int teamKey = (int)data->Value;
            if (teamKey != NO_KEY)
            {
                data->Value = IdToKey(m_teamKeyMapper[Tuple::Create(KeyToId(nameInfo->TournamentKey), KeyToId(teamKey))]);
            }
        }
    }

    if (cFound != 2)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformSystemEmpireTournaments(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "TournamentKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_tournamentKeyMapper[KeyToId(data->Value)]);
        }
    }

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    System::String^ password;

    for each (IDataElement^ data in original)
    {
        if (System::String::Compare(data->Name, "Password") == 0)
        {
            cFound++;
            password = (System::String^)data->Value;
        }
    }

    System::String^ hash = System::String::Empty;
    if (!System::String::IsNullOrEmpty(password))
    {
        hash = ComputePasswordHash(password);
    }
    accepted->Insert(6, gcnew FileDatabaseElement("PasswordHash", hash));

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameSecurity(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "EmpireKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
        }
    }

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameEmpires(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    System::String^ name = nullptr;

    for each (IDataElement^ data in accepted)
    {
        // Add EmpireName, map EmpireKey
        if (System::String::Compare(data->Name, "EmpireKey") == 0)
        {
            cFound++;

            name = GetEmpireNameFrom622Id(KeyToId(data->Value));
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
        }
    }

    accepted->Add(gcnew FileDatabaseElement("EmpireName", name));

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameDeadEmpires(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int alienKey = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "NukedEmpireKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
        }

        else if (System::String::Compare(data->Name, "AlienAddress") == 0)
        {
            cFound++;
            alienKey = IdToKey(m_alienKeyMapper[KeyToId(data->Value)]);
        }
    }

    accepted->Insert(2, gcnew FileDatabaseElement("AlienKey", alienKey));

    if (cFound != 2)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameMap(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "Owner") == 0)
        {
            cFound++;

            __int64 id = KeyToId(data->Value);
            if (id != KeyToId(SYSTEM_KEY) && id != KeyToId(INDEPENDENT_KEY))
            {
                data->Value = IdToKey(m_empireKeyMapper[id]);
            }
        }

        else if (System::String::Compare(data->Name, "HomeWorld") == 0)
        {
            cFound++;

            int hw = (int)data->Value;
            if (hw != HOMEWORLD && hw != NOT_HOMEWORLD)
            {
                data->Value = IdToKey(m_empireKeyMapper[KeyToId(hw)]);
            }
        }
    }

    if (cFound != 2)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameEmpireData(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "HomeWorld") == 0)
        {
            cFound++;

            int hw = (int)data->Value;
            if (hw != NO_KEY)
            {
                data->Value = IdToKey(m_planetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(hw))]);
            }
        }

        else if (System::String::Compare(data->Name, "DefaultBuilderPlanet") == 0)
        {
            cFound++;

            int key = (int)data->Value;
            if (key != NO_DEFAULT_BUILDER_PLANET && key != HOMEWORLD_DEFAULT_BUILDER_PLANET && key != LAST_BUILDER_DEFAULT_BUILDER_PLANET)
            {
                data->Value = IdToKey(m_planetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(data->Value))]);
            }
        }

        else if (System::String::Compare(data->Name, "LastBuilderPlanet") == 0)
        {
            cFound++;

            int key = (int)data->Value;
            if (key != NO_KEY)
            {
                data->Value = IdToKey(m_planetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(data->Value))]);
            }
        }
    }

    if (cFound != 3)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameEmpireMessages(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    int sourceKey = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "SourceName") == 0)
        {
            cFound++;

            __int64 id = GetEmpire622IdFromName((System::String^)data->Value);
            if (id != KeyToId(NO_KEY))
            {
                id = m_empireKeyMapper[id];
            }
            sourceKey = IdToKey(id);
        }
    }

    accepted->Insert(1, gcnew FileDatabaseElement("SourceKey", sourceKey));

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameEmpireMap(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "PlanetKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_planetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(data->Value))]);
        }
    }

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameEmpireDiplomacy(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "ReferenceEmpireKey") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_empireKeyMapper[KeyToId(data->Value)]);
        }
    }

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameEmpireShips(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "CurrentPlanet") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_planetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(data->Value))]);
        }

        else if (System::String::Compare(data->Name, "FleetKey") == 0)
        {
            cFound++;

            int key = (int)data->Value;
            if (key != NO_KEY)
            {
                data->Value = IdToKey(m_fleetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(nameInfo->EmpireKey), KeyToId(data->Value))]);
            }
        }
    }

    if (cFound != 2)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameIndependentShips(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "CurrentPlanet") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_planetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(data->Value))]);
        }

        else if (System::String::Compare(data->Name, "GateDestination") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_planetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(data->Value))]);
        }

        else if (System::String::Compare(data->Name, "FleetKey") == 0)
        {
            cFound++;

            int key = (int)data->Value;
            if (key != NO_KEY)
            {
                data->Value = IdToKey(m_fleetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(nameInfo->EmpireKey), KeyToId(data->Value))]);
            }
        }
    }

    // Add EmpireKey
    accepted->Insert(0, gcnew FileDatabaseElement("EmpireKey", (int)INDEPENDENT_KEY));

    // Add ColonyBuildCost
    int zero = 0;
    accepted->Add(gcnew FileDatabaseElement("ColonyBuildCost", zero));

    if (cFound != 3)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
}

bool Transform622to700::TransformGameEmpireFleets(TableNameInfo^ nameInfo, IDataRow^ original, List<IDataElement^>^ accepted)
{
    int cFound = 0;

    for each (IDataElement^ data in accepted)
    {
        if (System::String::Compare(data->Name, "CurrentPlanet") == 0)
        {
            cFound++;
            data->Value = IdToKey(m_planetKeyMapper[Tuple::Create(KeyToId(nameInfo->GameClass), nameInfo->GameNumber, KeyToId(data->Value))]);
        }
    }

    if (cFound != 1)
    {
        throw gcnew ApplicationException("Column not found");
    }
    return true;
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

System::String^ Transform622to700::ComputePasswordHash(System::String^ password)
{
    int iErrCode;

    Crypto::HashSHA256 hash;
    msclr::auto_handle<marshal_context> context = gcnew marshal_context();
    
    const char* pszPassword = context->marshal_as<const char*>(password);
    iErrCode = hash.HashData(pszPassword, strlen(pszPassword));
    THROW_ON_ERROR(iErrCode);

    if (m_serverFixedHashSalt == nullptr)
    {
        throw gcnew ApplicationException("m_serverFixedHashSalt must be set before calling ComputePasswordHash");
    }

    const char* pszFixedSalt = context->marshal_as<const char*>(m_serverFixedHashSalt);
    iErrCode = hash.HashData(pszFixedSalt, strlen(pszFixedSalt));
    THROW_ON_ERROR(iErrCode);

    size_t cbSize;
    iErrCode = hash.GetHashSize(&cbSize);
    THROW_ON_ERROR(iErrCode);

    void* pBuffer = StackAlloc(cbSize);
    iErrCode = hash.GetHash(pBuffer, cbSize);
    THROW_ON_ERROR(iErrCode);

    size_t cch = Algorithm::GetEncodeBase64Size(cbSize);
    char* pszBase64 = (char*)StackAlloc(cch);
    iErrCode = Algorithm::EncodeBase64(pBuffer, cbSize, pszBase64, cch);
    THROW_ON_ERROR(iErrCode);

    return gcnew System::String(pszBase64);
}

void Transform622to700::RemapGameMapPlanetKeys()
{
    Console::WriteLine();
    Console::WriteLine("Remapping GameMap cardinal point key columns...");

    int cPlanets = 0;
    for each (IDataTable^ table in m_source)
    {
        TableNameInfo^ nameInfo = gcnew TableNameInfo(table->Name);
        if (System::String::Compare(nameInfo->Name, "GameMap") == 0)
        {
            for each (IDataRow^ row in table)
            {
                int cFound = 0;
                for each (IDataElement^ data in row)
                {
                    if (System::String::Compare(data->Name, "NorthPlanetKey") == 0 ||
                        System::String::Compare(data->Name, "EastPlanetKey") == 0 ||
                        System::String::Compare(data->Name, "SouthPlanetKey") == 0 ||
                        System::String::Compare(data->Name, "WestPlanetKey") == 0)
                    {
                        cFound ++;
                        cPlanets ++;
                        
                        int key = (int)data->Value;
                        if (key != NO_KEY)
                        {
                            RemapGameMapPlanetKey(nameInfo->GameClass, nameInfo->GameNumber, (int)data->Value, data->Name);
                        }
                    }
                }

                if (cFound != 4)
                {
                    throw gcnew ApplicationException("Column not found");
                }
            }
        }
    }

    Console::WriteLine("Remapped {0} cardinal point keys...", cPlanets);
}

void Transform622to700::RemapGameMapPlanetKey(int gameClassKey, int gameNumber, int planetKey, System::String^ column)
{
    array<BulkTableReadRequestColumn>^ cols = gcnew array<BulkTableReadRequestColumn>(2);

    cols[0].ColumnName = "GameClass";
    cols[0].ColumnValue = gameClassKey;
    cols[1].ColumnName = "GameNumber";
    cols[1].ColumnValue = gameNumber;

    __int64 id = m_planetKeyMapper[Tuple::Create(KeyToId(gameClassKey), gameNumber, KeyToId(planetKey))];

    m_dest->WriteRecord("GameMap", cols, column, id);
}

bool Transform622to700::IsGameActive(int gameClassKey, int gameNumber)
{
    System::String^ comp = System::String::Format("{0}.{1}", gameClassKey, gameNumber);

    for each (IDataTable^ table in m_source)
    {
        if (System::String::Compare(table->Name, "SystemActiveGames") == 0)
        {
            for each (IDataRow^ row in table)
            {
                for each (IDataElement^ data in row)
                {
                    if (System::String::Compare(data->Name, "GameClassGameNumber") == 0 && 
                        System::String::Compare((System::String^)data->Value, comp) == 0)
                    {
                        return true;
                    }
                }
            }
            return false;
        }
    }

    return false;
}

void Transform622to700::InsertAssociationRows()
{
    int cAssoc = m_associations->Count;
    if (cAssoc > 0)
    {
        Console::WriteLine();
        Console::WriteLine("Inserting {0} rows into SystemEmpireAssociations", cAssoc);

        for each (Tuple<int, int>^ assoc in m_associations)
        {
            if (assoc->Item1 < assoc->Item2)
            {
                int empireKey = IdToKey(m_empireKeyMapper[KeyToId(assoc->Item1)]);
                int referenceEmpireKey = IdToKey(m_empireKeyMapper[KeyToId(assoc->Item2)]);

                List<IDataElement^>^ row;
                
                row = gcnew List<IDataElement^>();
                row->Add(gcnew FileDatabaseElement("EmpireKey", empireKey));
                row->Add(gcnew FileDatabaseElement("ReferenceEmpireKey", referenceEmpireKey));

                m_dest->InsertRow("SystemEmpireAssociations", row);

                row = gcnew List<IDataElement^>();
                row->Add(gcnew FileDatabaseElement("EmpireKey", referenceEmpireKey));
                row->Add(gcnew FileDatabaseElement("ReferenceEmpireKey", empireKey));

                m_dest->InsertRow("SystemEmpireAssociations", row);
            }
        }
    }
}

void Transform622to700::InsertAvailabilityRow()
{
    // HACKHACK - slight hackery here
    System::String^ fileDb = System::Configuration::ConfigurationManager::AppSettings["FileDatabase"];
    fileDb = Path::Combine(fileDb, "tables.dat");

    DateTime backupTime = File::GetLastWriteTimeUtc(fileDb);
    DateTime startTime(1970, 1, 1, 0, 0, 0, 0);
    TimeSpan diff = backupTime - startTime;
    __int64 lastAvailableUtc = Convert::ToInt64(Math::Abs(diff.TotalSeconds));
 
    List<IDataElement^>^ row = gcnew List<IDataElement^>();
    row->Add(gcnew FileDatabaseElement("LastAvailableTime", lastAvailableUtc));

    Console::WriteLine();
    Console::WriteLine("Inserting SystemAvailability row with LastAvailableTime={0}", backupTime.ToString("u"));

    m_dest->InsertRow("SystemAvailability", row);
}

System::String^ Transform622to700::ColumnNamesToStringList(List<IDataElement^>^ cols)
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

System::String^ Transform622to700::ColumnRenamesToStringList(List<Tuple<System::String^, System::String^>^>^ renames)
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

void Transform622to700::EnsureIdenticalDeletionLists(TemplateMetadata^ templateMeta, List<IDataElement^>^ experimental)
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