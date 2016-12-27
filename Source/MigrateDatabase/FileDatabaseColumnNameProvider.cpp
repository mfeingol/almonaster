#include "FileDatabaseColumnNameProvider.h"
#include "GameEngineSchema622.h"

//
// This is a workaround for the lack of column names as part of the formal templates used in the 622 FileDatabase
//

FileDatabaseColumnNameProvider::FileDatabaseColumnNameProvider()
{
    m_templateMap = gcnew Dictionary<System::String^, array<System::String^>^>();
    Populate();
}

array<System::String^>^ ToArray(const char** ppszString, unsigned int numStrings)
{
    array<System::String^>^ strings = gcnew array<System::String^>(numStrings);

    for (unsigned int i = 0; i < numStrings; i ++)
    {
        strings[i] = gcnew System::String(ppszString[i]);
    }

    return strings;
}

void FileDatabaseColumnNameProvider::Populate()
{
    m_templateMap->Add(gcnew System::String(SystemData::Template.Name), ToArray(SystemData::ColumnNames, SystemData::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemEmpireData::Template.Name), ToArray(SystemEmpireData::ColumnNames, SystemEmpireData::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemGameClassData::Template.Name), ToArray(SystemGameClassData::ColumnNames, SystemGameClassData::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemAlienIcons::Template.Name), ToArray(SystemAlienIcons::ColumnNames, SystemAlienIcons::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemSystemGameClassData::Template.Name), ToArray(SystemSystemGameClassData::ColumnNames, SystemSystemGameClassData::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemSuperClassData::Template.Name), ToArray(SystemSuperClassData::ColumnNames, SystemSuperClassData::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemThemes::Template.Name), ToArray(SystemThemes::ColumnNames, SystemThemes::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemActiveGames::Template.Name), ToArray(SystemActiveGames::ColumnNames, SystemActiveGames::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemLatestGames::Template.Name), ToArray(SystemLatestGames::ColumnNames, SystemLatestGames::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemEmpireMessages::Template.Name), ToArray(SystemEmpireMessages::ColumnNames, SystemEmpireMessages::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemEmpireNukeList::Template.Name), ToArray(SystemEmpireNukeList::ColumnNames, SystemEmpireNukeList::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemNukeList::Template.Name), ToArray(SystemNukeList::ColumnNames, SystemNukeList::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemEmpireActiveGames::Template.Name), ToArray(SystemEmpireActiveGames::ColumnNames, SystemEmpireActiveGames::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemTournaments::Template.Name), ToArray(SystemTournaments::ColumnNames, SystemTournaments::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemTournamentTeams::Template.Name), ToArray(SystemTournamentTeams::ColumnNames, SystemTournamentTeams::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemTournamentEmpires::Template.Name), ToArray(SystemTournamentEmpires::ColumnNames, SystemTournamentEmpires::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemTournamentActiveGames::Template.Name), ToArray(SystemTournamentActiveGames::ColumnNames, SystemTournamentActiveGames::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemEmpireTournaments::Template.Name), ToArray(SystemEmpireTournaments::ColumnNames, SystemEmpireTournaments::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemAlmonasterScoreTopList::Template.Name), ToArray(SystemAlmonasterScoreTopList::ColumnNames, SystemAlmonasterScoreTopList::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemClassicScoreTopList::Template.Name), ToArray(SystemClassicScoreTopList::ColumnNames, SystemClassicScoreTopList::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemBridierScoreTopList::Template.Name), ToArray(SystemBridierScoreTopList::ColumnNames, SystemBridierScoreTopList::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemBridierScoreEstablishedTopList::Template.Name), ToArray(SystemBridierScoreEstablishedTopList::ColumnNames, SystemBridierScoreEstablishedTopList::NumColumns));
    m_templateMap->Add(gcnew System::String(SystemChatroomData::Template.Name), ToArray(SystemChatroomData::ColumnNames, SystemChatroomData::NumColumns));
    m_templateMap->Add(gcnew System::String(GameData::Template.Name), ToArray(GameData::ColumnNames, GameData::NumColumns));
    m_templateMap->Add(gcnew System::String(GameSecurity::Template.Name), ToArray(GameSecurity::ColumnNames, GameSecurity::NumColumns));
    m_templateMap->Add(gcnew System::String(GameEmpires::Template.Name), ToArray(GameEmpires::ColumnNames, GameEmpires::NumColumns));
    m_templateMap->Add(gcnew System::String(GameDeadEmpires::Template.Name), ToArray(GameDeadEmpires::ColumnNames, GameDeadEmpires::NumColumns));
    m_templateMap->Add(gcnew System::String(GameMap::Template.Name), ToArray(GameMap::ColumnNames, GameMap::NumColumns));
    m_templateMap->Add(gcnew System::String(GameEmpireData::Template.Name), ToArray(GameEmpireData::ColumnNames, GameEmpireData::NumColumns));
    m_templateMap->Add(gcnew System::String(GameEmpireMessages::Template.Name), ToArray(GameEmpireMessages::ColumnNames, GameEmpireMessages::NumColumns));
    m_templateMap->Add(gcnew System::String(GameEmpireMap::Template.Name), ToArray(GameEmpireMap::ColumnNames, GameEmpireMap::NumColumns));
    m_templateMap->Add(gcnew System::String(GameEmpireDiplomacy::Template.Name), ToArray(GameEmpireDiplomacy::ColumnNames, GameEmpireDiplomacy::NumColumns));
    m_templateMap->Add(gcnew System::String(GameEmpireShips::Template.Name), ToArray(GameEmpireShips::ColumnNames, GameEmpireShips::NumColumns));
    m_templateMap->Add(gcnew System::String(GameEmpireFleets::Template.Name), ToArray(GameEmpireFleets::ColumnNames, GameEmpireFleets::NumColumns));
    m_templateMap->Add(gcnew System::String(GameIndependentShips::Template.Name), ToArray(GameIndependentShips::ColumnNames, GameIndependentShips::NumColumns));
}

array<System::String^>^ FileDatabaseColumnNameProvider::GetColumnNames(const char* pszTemplateName)
{
    return m_templateMap[gcnew System::String(pszTemplateName)];
}