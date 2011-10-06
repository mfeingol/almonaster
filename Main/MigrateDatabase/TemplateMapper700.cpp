#include "TemplateMapper700.h"
#include "GameEngineSchema700.h"
#include "FileDatabaseElement.h"

using namespace System;

TemplateMetadata^ CreateTemplateMetadata(const TemplateDescription& desc)
{
    TemplateMetadata^ meta = gcnew TemplateMetadata(gcnew System::String(desc.Name));

    for (unsigned int i = 0; i < desc.NumColumns; i ++)
    {
        TemplateColumnMetadata^ colMeta = gcnew TemplateColumnMetadata();
        colMeta->Name = gcnew System::String(desc.ColumnNames[i]);
        meta->Columns->Add(colMeta);
    }

    return meta;
}

TemplateMapper700::TemplateMapper700()
{
    m_templates = gcnew Dictionary<System::String^, TemplateMetadata^>();

    m_templates["SystemActiveGames"] = CreateTemplateMetadata(SystemActiveGames::Template);
    m_templates["SystemActiveGames"]->DeletedColumns->Add(gcnew FileDatabaseElement("GameClassGameNumber", nullptr));
    m_templates["SystemActiveGames"]->RenamedColumns->Add(Tuple::Create("State", "Open"));

    m_templates["SystemAlienIcons"] = CreateTemplateMetadata(SystemAlienIcons::Template);
    m_templates["SystemAlienIcons"]->RenamedColumns->Add(Tuple::Create("AlienKey", "Address"));

    m_templates["SystemAvailability"] = CreateTemplateMetadata(SystemAvailability::Template);
    m_templates["SystemChatroomData"] = CreateTemplateMetadata(SystemChatroomData::Template);
    
    m_templates["SystemData"] = CreateTemplateMetadata(SystemData::Template);
    m_templates["SystemData"]->DeletedColumns->Add(gcnew FileDatabaseElement("LastShutdownTime", nullptr));
    m_templates["SystemData"]->RenamedColumns->Add(Tuple::Create("DefaultAlien", "DefaultAlienAddress"));
    m_templates["SystemData"]->RenamedColumns->Add(Tuple::Create("SystemMessagesAlienKey", "SystemMessagesAlienAddress"));

    m_templates["SystemEmpireActiveGames"] = CreateTemplateMetadata(SystemEmpireActiveGames::Template);
    m_templates["SystemEmpireActiveGames"]->DeletedColumns->Add(gcnew FileDatabaseElement("GameClassGameNumber", nullptr));

    m_templates["SystemEmpireAssociations"] = CreateTemplateMetadata(SystemEmpireAssociations::Template);

    m_templates["SystemEmpireData"] = CreateTemplateMetadata(SystemEmpireData::Template);
    m_templates["SystemEmpireData"]->DeletedColumns->Add(gcnew FileDatabaseElement("Associations", nullptr));
    m_templates["SystemEmpireData"]->DeletedColumns->Add(gcnew FileDatabaseElement("Password", nullptr));
    m_templates["SystemEmpireData"]->RenamedColumns->Add(Tuple::Create("AlienKey", "AlienAddress"));

    m_templates["SystemEmpireMessages"] = CreateTemplateMetadata(SystemEmpireMessages::Template);
    m_templates["SystemEmpireMessages"]->RenamedColumns->Add(Tuple::Create("Source", "SourceName"));

    m_templates["SystemEmpireNukedList"] = CreateTemplateMetadata(SystemEmpireNukeList::Template);
    m_templates["SystemEmpireNukedList"]->RenamedColumns->Add(Tuple::Create("AlienKey", "AlienAddress"));
    m_templates["SystemEmpireNukedList"]->RenamedColumns->Add(Tuple::Create("EmpireKey", "ReferenceEmpireKey"));

    m_templates["SystemEmpireNukerList"] = CreateTemplateMetadata(SystemEmpireNukeList::Template);
    m_templates["SystemEmpireNukerList"]->RenamedColumns->Add(Tuple::Create("AlienKey", "AlienAddress"));

    m_templates["SystemEmpireTournaments"] = CreateTemplateMetadata(SystemEmpireTournaments::Template);

    m_templates["SystemGameClassData"] = CreateTemplateMetadata(SystemGameClassData::Template);
    m_templates["SystemGameClassData"]->DeletedColumns->Add(gcnew FileDatabaseElement("fRESERVED2", nullptr));
    m_templates["SystemGameClassData"]->DeletedColumns->Add(gcnew FileDatabaseElement("fRESERVED3", nullptr));

    m_templates["SystemLatestGames"] = CreateTemplateMetadata(SystemLatestGames::Template);
    
    m_templates["SystemNukeList"] = CreateTemplateMetadata(SystemNukeList::Template);
    m_templates["SystemNukeList"]->RenamedColumns->Add(Tuple::Create("NukerAlienKey", "NukerAlienAddress"));
    m_templates["SystemNukeList"]->RenamedColumns->Add(Tuple::Create("NukedAlienKey", "NukedAlienAddress"));

    m_templates["SystemSuperClassData"] = CreateTemplateMetadata(SystemSuperClassData::Template);
    m_templates["SystemThemes"] = CreateTemplateMetadata(SystemThemes::Template);
    m_templates["SystemTournamentEmpires"] = CreateTemplateMetadata(SystemTournamentEmpires::Template);
    
    m_templates["SystemTournaments"] = CreateTemplateMetadata(SystemTournaments::Template);
    m_templates["SystemTournaments"]->RenamedColumns->Add(Tuple::Create("Icon", "IconAddress"));

    m_templates["SystemTournamentTeams"] = CreateTemplateMetadata(SystemTournamentTeams::Template);
    m_templates["SystemTournamentTeams"]->RenamedColumns->Add(Tuple::Create("Icon", "IconAddress"));

    m_templates["GameData"] = CreateTemplateMetadata(GameData::Template);
    m_templates["GameData"]->DeletedColumns->Add(gcnew FileDatabaseElement("LastUpdateCheck", nullptr));
    m_templates["GameData"]->DeletedColumns->Add(gcnew FileDatabaseElement("Password", nullptr));

    m_templates["GameSecurity"] = CreateTemplateMetadata(GameSecurity::Template);
    m_templates["GameEmpires"] = CreateTemplateMetadata(GameEmpires::Template);

    m_templates["GameNukedEmpires"] = CreateTemplateMetadata(GameNukedEmpires::Template);
    m_templates["GameNukedEmpires"]->RenamedColumns->Add(Tuple::Create("Key", "NukedEmpireKey"));
    m_templates["GameNukedEmpires"]->RenamedColumns->Add(Tuple::Create("Icon", "AlienAddress"));

    m_templates["GameMap"] = CreateTemplateMetadata(GameMap::Template);
    
    m_templates["GameEmpireData"] = CreateTemplateMetadata(GameEmpireData::Template);
    m_templates["GameEmpireData"]->RenamedColumns->Add(Tuple::Create("NumAlliancesLeaked", "NumNukedAllies"));
    m_templates["GameEmpireData"]->DeletedColumns->Add(gcnew FileDatabaseElement("NumTruces", nullptr));
    m_templates["GameEmpireData"]->DeletedColumns->Add(gcnew FileDatabaseElement("NumTrades", nullptr));
    m_templates["GameEmpireData"]->DeletedColumns->Add(gcnew FileDatabaseElement("NumAlliances", nullptr));

    m_templates["GameEmpireMessages"] = CreateTemplateMetadata(GameEmpireMessages::Template);
    m_templates["GameEmpireMessages"]->RenamedColumns->Add(Tuple::Create("Source", "SourceName"));

    m_templates["GameEmpireMap"] = CreateTemplateMetadata(GameEmpireMap::Template);
    m_templates["GameEmpireMap"]->DeletedColumns->Add(gcnew FileDatabaseElement("RESERVED0", nullptr));
    m_templates["GameEmpireMap"]->DeletedColumns->Add(gcnew FileDatabaseElement("RESERVED1", nullptr));
    m_templates["GameEmpireMap"]->DeletedColumns->Add(gcnew FileDatabaseElement("RESERVED2", nullptr));

    m_templates["GameEmpireDiplomacy"] = CreateTemplateMetadata(GameEmpireDiplomacy::Template);
    m_templates["GameEmpireDiplomacy"]->RenamedColumns->Add(Tuple::Create("EmpireKey", "ReferenceEmpireKey"));
    m_templates["GameEmpireDiplomacy"]->RenamedColumns->Add(Tuple::Create("VirtualStatus", "DipOfferLastUpdate"));

    m_templates["GameEmpireShips"] = CreateTemplateMetadata(GameEmpireShips::Template);

    m_templates["GameEmpireFleets"] = CreateTemplateMetadata(GameEmpireFleets::Template);
    m_templates["GameEmpireFleets"]->DeletedColumns->Add(gcnew FileDatabaseElement("NumShips", nullptr));
    m_templates["GameEmpireFleets"]->DeletedColumns->Add(gcnew FileDatabaseElement("BuildShips", nullptr));
}

System::Collections::IEnumerator^ TemplateMapper700::GetEnumerator_nongeneric()
{
    return GetEnumerator();
}

IEnumerator<TemplateMetadata^>^ TemplateMapper700::GetEnumerator()
{
    return m_templates->Values->GetEnumerator();
}