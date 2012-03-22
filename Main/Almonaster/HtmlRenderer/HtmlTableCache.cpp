//
// Almonaster.dll: a component of Almonaster
// Copyright(c) 1998 Max Attar Feingold(maf6@cornell.edu)
//
// This program is free software;you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation;either version 2
// of the License, or(at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY;without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program;if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA02111-1307, USA.

#include "HtmlRenderer.h"

static const TableCacheEntry systemData = { { SYSTEM_DATA, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemThemes = { { SYSTEM_THEMES, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemGameClassData = { { SYSTEM_GAMECLASS_DATA, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemSuperClassData = { { SYSTEM_SUPERCLASS_DATA, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemActiveGames = { { SYSTEM_ACTIVE_GAMES, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemLatestGames = { { SYSTEM_LATEST_GAMES, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemTournaments = { { SYSTEM_TOURNAMENTS, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemChatRoomData = { { SYSTEM_CHATROOM_DATA, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemNukeList = { { SYSTEM_NUKE_LIST, NO_KEY, 0, NULL }, NULL, NULL, NULL };
static const TableCacheEntry systemAlienIcons = { { SYSTEM_ALIEN_ICONS, NO_KEY, 0, NULL }, NULL, NULL, NULL };

void Cache(Vector<TableCacheEntry>& cache, const TableCacheEntry& entry)
{
    int iErrCode = cache.Add(entry);
    Assert(iErrCode == OK);
}

void HtmlRenderer::GatherCacheTablesForSystemPage(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemData);
    Cache(cache, systemThemes);
    Cache(cache, systemGameClassData);
    Cache(cache, systemSuperClassData);
    Cache(cache, systemTournaments);

    if (m_iEmpireKey != NO_KEY)
    {
        m_systemEmpireCol.Data = m_iEmpireKey;

        const TableCacheEntry systemEmpireDataN = { { SYSTEM_EMPIRE_DATA, m_iEmpireKey, 0, NULL }, NULL, NULL, NULL };
        Cache(cache, systemEmpireDataN);

        const TableCacheEntry systemEmpireMessagesN = { { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
        Cache(cache, systemEmpireMessagesN);

        const TableCacheEntry systemEmpireTournamentsN = { { SYSTEM_EMPIRE_TOURNAMENTS, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
        Cache(cache, systemEmpireTournamentsN);
    }

    if (m_iTournamentKey != NO_KEY)
    {
        m_systemTournamentCol.Data = m_iTournamentKey;
    }
}

void HtmlRenderer::GatherCacheTablesForGamePage(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemData);
    Cache(cache, systemThemes);
    Cache(cache, systemGameClassData);
    Cache(cache, systemActiveGames);
    Cache(cache, systemTournaments);

    TableCacheEntry systemEmpireDataN = { { SYSTEM_EMPIRE_DATA, m_iEmpireKey, 0, NULL }, NULL, NULL, NULL };
    Cache(cache, systemEmpireDataN);

    m_systemEmpireCol.Data = m_iEmpireKey;
    
    m_gameCols[0].Data = m_iGameClass;
    m_gameCols[1].Data = m_iGameNumber;

    m_gameEmpireCols[0].Data = m_iGameClass;
    m_gameEmpireCols[1].Data = m_iGameNumber;
    m_gameEmpireCols[2].Data = m_iEmpireKey;

    if (m_iGameClass != NO_KEY && m_iGameNumber != -1)
    {
        const TableCacheEntry gameData = { { GAME_DATA, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, NULL, NULL };
        Cache(cache, gameData);

        const TableCacheEntry gameEmpires = { { GAME_EMPIRES, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, NULL, NULL };
        Cache(cache, gameEmpires);

        const TableCacheEntry allGameEmpireData = { { GAME_EMPIRE_DATA, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireData::EmpireKey, NULL };
        Cache(cache, allGameEmpireData);

        const TableCacheEntry gameEmpireMessages = { { GAME_EMPIRE_MESSAGES, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
        Cache(cache, gameEmpireMessages);
    }

    RegisterCacheRatioTablesIfNecessary(cache);
}

int HtmlRenderer::AfterCacheTablesForGamePage()
{
    return AfterCacheRatioTablesIfNecessary();
}

void HtmlRenderer::RegisterCache_ActiveGameList(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemActiveGames);

    TableCacheEntry systemEmpireActiveGamesN = { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireActiveGamesN);
}

void HtmlRenderer::RegisterCache_Login(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_NewEmpire(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_OpenGameList(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemActiveGames);

    const TableCacheEntry systemEmpireActiveGamesN = { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireActiveGamesN);
}

void HtmlRenderer::RegisterCache_SystemGameList(Vector<TableCacheEntry>& cache)
{
    // Needed because when you enter a game, the system needs to know if you're in any games, and if so, whether you're idle
    const TableCacheEntry systemEmpireActiveGamesN = { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireActiveGamesN);

    // Needed to insert a row when entering a game
    Cache(cache, systemActiveGames);
}

void HtmlRenderer::RegisterCache_ProfileEditor(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry systemEmpireAssociationsN = { { SYSTEM_EMPIRE_ASSOCIATIONS, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireAssociationsN);
}

void HtmlRenderer::RegisterCache_TopLists(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_ProfileViewer(Vector<TableCacheEntry>& cache)
{
    // Needed to render personal game lists
    Cache(cache, systemActiveGames);

    // Needed because when you enter a game, the system needs to know if you're in any games, and if so, whether you're idle
    Assert(m_iEmpireKey != NO_KEY);

    const TableCacheEntry systemEmpireActiveGamesN = { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireActiveGamesN);

    // Needed to determine whether to display nuke history
    const TableCacheEntry systemEmpireNukerListN = { { SYSTEM_EMPIRE_NUKER_LIST, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireNukerListN);

    const TableCacheEntry systemEmpireNukedListN = { { SYSTEM_EMPIRE_NUKED_LIST, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireNukedListN);

    // Log in as associated empire
    const TableCacheEntry systemEmpireAssociationsN = { { SYSTEM_EMPIRE_ASSOCIATIONS, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireAssociationsN);
}

void HtmlRenderer::RegisterCache_ServerAdministrator(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemAlienIcons);
}

void HtmlRenderer::RegisterCache_EmpireAdministrator(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameAdministrator(Vector<TableCacheEntry>& cache)
{
    // Needed to insert a row when entering a game
    Cache(cache, systemActiveGames);
}

void HtmlRenderer::RegisterCache_ThemeAdministrator(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_PersonalGameClasses(Vector<TableCacheEntry>& cache)
{
    // Needed because when you enter a game, the system needs to know if you're in any games, and if so, whether you're idle
    const TableCacheEntry systemEmpireActiveGamesN = { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireActiveGamesN);

    // Needed to insert a row when entering a game
    Cache(cache, systemActiveGames);
}

void HtmlRenderer::RegisterCache_Chatroom(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemChatRoomData);
}

void HtmlRenderer::RegisterCache_SystemServerRules(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemActiveGames);
}

void HtmlRenderer::RegisterCache_SystemFAQ(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemNews(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Info(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry gameEmpireShips = { { GAME_EMPIRE_SHIPS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols}, NULL, NULL, NULL };
    Cache(cache, gameEmpireShips);

    // Ratios line
    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);
}

int HtmlRenderer::AfterCache_Info()
{
    int iErrCode;
  
    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, m_iEmpireKey, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_SHIPS);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_DIPLOMACY);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

void HtmlRenderer::RegisterCache_Tech(Vector<TableCacheEntry>& cache)
{
    // Ratios line
    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);
}

int HtmlRenderer::AfterCache_Tech()
{
    return CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_DIPLOMACY);
}

void HtmlRenderer::RegisterCache_Diplomacy(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);

    m_crossJoinEntry.RightColumnName = GameEmpireDiplomacy::ReferenceEmpireKey;
    m_crossJoinEntry.Table.Name = GAME_EMPIRE_DIPLOMACY;
    m_crossJoinEntry.Table.Key = NO_KEY;
    m_crossJoinEntry.Table.NumColumns = countof(m_gameEmpireCols);
    m_crossJoinEntry.Table.Columns = m_gameEmpireCols;

    const TableCacheEntry systemEmpireDataContacts = { { SYSTEM_EMPIRE_DATA, NO_KEY, 0, NULL}, NULL, ID_COLUMN_NAME, &m_crossJoinEntry };
    Cache(cache, systemEmpireDataContacts);
}

int HtmlRenderer::AfterCache_Diplomacy()
{
    return CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_DIPLOMACY);
}

void HtmlRenderer::RegisterCache_Map(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry gameMap = { { GAME_MAP, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, NULL };
    Cache(cache, gameMap);

    // Need access to everyone's map and ships
    const TableCacheEntry allGameEmpireMaps = { { GAME_EMPIRE_MAP, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireMap::EmpireKey, NULL };
    Cache(cache, allGameEmpireMaps);

    const TableCacheEntry allGameEmpireShips = { { GAME_EMPIRE_SHIPS, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireShips::EmpireKey, NULL };
    Cache(cache, allGameEmpireShips);

    // Ship view
    const TableCacheEntry gameEmpireFleets = { { GAME_EMPIRE_FLEETS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireFleets);

    // When another empire's ship appears on a planet, we need their SystemEmpireData row
    m_crossJoinEntry.RightColumnName = GameEmpireData::EmpireKey;
    m_crossJoinEntry.Table.Name = GAME_EMPIRE_DATA;
    m_crossJoinEntry.Table.Key = NO_KEY;
    m_crossJoinEntry.Table.NumColumns = countof(m_gameCols);
    m_crossJoinEntry.Table.Columns = m_gameCols;

    const TableCacheEntry systemEmpireDataFromGame = { { SYSTEM_EMPIRE_DATA, NO_KEY, 0, NULL}, NULL, ID_COLUMN_NAME, &m_crossJoinEntry };
    Cache(cache, systemEmpireDataFromGame);

    // Diplomacy coloring and ratios
    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);
}

int HtmlRenderer::AfterCache_Map()
{
    int iErrCode;
  
    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, m_iEmpireKey, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_FLEETS);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_MAP | EMPTY_GAME_EMPIRE_SHIPS | EMPTY_GAME_EMPIRE_DIPLOMACY);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

void HtmlRenderer::RegisterCache_Planets(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry gameMap = { { GAME_MAP, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, NULL, NULL };
    Cache(cache, gameMap);

    // Need access to everyone's map and ships
    const TableCacheEntry allGameEmpireMaps = { { GAME_EMPIRE_MAP, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireMap::EmpireKey, NULL };
    Cache(cache, allGameEmpireMaps);

    const TableCacheEntry allGameEmpireShips = { { GAME_EMPIRE_SHIPS, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireShips::EmpireKey, NULL };
    Cache(cache, allGameEmpireShips);

    // Ship view
    const TableCacheEntry gameEmpireFleets = { { GAME_EMPIRE_FLEETS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireFleets);

    // Diplomacy coloring and ratios
    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);

    // When another empire's ship appears on a planet, we need their SystemEmpireData row
    m_crossJoinEntry.RightColumnName = GameEmpireData::EmpireKey;
    m_crossJoinEntry.Table.Name = GAME_EMPIRE_DATA;
    m_crossJoinEntry.Table.Key = NO_KEY;
    m_crossJoinEntry.Table.NumColumns = countof(m_gameCols);
    m_crossJoinEntry.Table.Columns = m_gameCols;

    const TableCacheEntry systemEmpireDataFromGame = { { SYSTEM_EMPIRE_DATA, NO_KEY, 0, NULL}, NULL, ID_COLUMN_NAME, &m_crossJoinEntry };
    Cache(cache, systemEmpireDataFromGame);
}

int HtmlRenderer::AfterCache_Planets()
{
    int iErrCode;
  
    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, m_iEmpireKey, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_FLEETS);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_MAP | EMPTY_GAME_EMPIRE_SHIPS | EMPTY_GAME_EMPIRE_DIPLOMACY);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

void HtmlRenderer::RegisterCache_Options(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry gameMap = { { GAME_MAP, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, NULL, NULL };
    Cache(cache, gameMap);

    // Ratios line
    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);
}

int HtmlRenderer::AfterCache_Options()
{
    int iErrCode;

    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_DIPLOMACY);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

void HtmlRenderer::RegisterCache_Build(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry gameMap = { { GAME_MAP, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, NULL, NULL };
    Cache(cache, gameMap);

    const TableCacheEntry gameEmpireMap = { { GAME_EMPIRE_MAP, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireMap);

    const TableCacheEntry gameEmpireShips = { { GAME_EMPIRE_SHIPS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireShips);

    const TableCacheEntry gameEmpireFleets = { { GAME_EMPIRE_FLEETS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireFleets);

    // Ratios line
    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);
}

int HtmlRenderer::AfterCache_Build()
{
    int iErrCode;
  
    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, m_iEmpireKey, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_MAP | EMPTY_GAME_EMPIRE_SHIPS | EMPTY_GAME_EMPIRE_FLEETS);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_DIPLOMACY);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

void HtmlRenderer::RegisterCache_Ships(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry gameEmpireShips = { { GAME_EMPIRE_SHIPS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireShips);

    const TableCacheEntry gameEmpireFleets = { { GAME_EMPIRE_FLEETS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireFleets);

    // Ratios line
    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);

    // Cancel builds
    const TableCacheEntry gameMap = { { GAME_MAP, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, NULL, NULL };
    Cache(cache, gameMap);

    const TableCacheEntry gameEmpireMap = { { GAME_EMPIRE_MAP, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireMap);
}

int HtmlRenderer::AfterCache_Ships()
{
    int iErrCode;
  
    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, m_iEmpireKey, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_SHIPS | EMPTY_GAME_EMPIRE_FLEETS | EMPTY_GAME_EMPIRE_MAP);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_DIPLOMACY);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

void HtmlRenderer::RegisterCache_GameServerRules(Vector<TableCacheEntry>& cache)
{
}

int HtmlRenderer::AfterCache_GameServerRules()
{
    return OK;
}

void HtmlRenderer::RegisterCache_GameFAQ(Vector<TableCacheEntry>& cache)
{
}

int HtmlRenderer::AfterCache_GameFAQ()
{
    return OK;
}

void HtmlRenderer::RegisterCache_GameNews(Vector<TableCacheEntry>& cache)
{
}

int HtmlRenderer::AfterCache_GameNews()
{
    return OK;
}

void HtmlRenderer::RegisterCache_GameContributions(Vector<TableCacheEntry>& cache)
{
}

int HtmlRenderer::AfterCache_GameContributions()
{
    return OK;
}

void HtmlRenderer::RegisterCache_GameCredits(Vector<TableCacheEntry>& cache)
{
}

int HtmlRenderer::AfterCache_GameCredits()
{
    return OK;
}

void HtmlRenderer::RegisterCache_GameProfileViewer(Vector<TableCacheEntry>& cache)
{
    const TableCacheEntry systemEmpireAssociationsN = { { SYSTEM_EMPIRE_ASSOCIATIONS, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireAssociationsN);

    const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
    Cache(cache, allGameEmpireDiplomacy);
}

int HtmlRenderer::AfterCache_GameProfileViewer()
{
    int iErrCode;
  
    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_DIPLOMACY);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

void HtmlRenderer::RegisterCache_Quit(Vector<TableCacheEntry>& cache)
{
    TableCacheEntry systemEmpireActiveGamesN = { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_systemEmpireCol }, NULL, NULL, NULL };
    Cache(cache, systemEmpireActiveGamesN);
    
    // When you resign, interesting things happen to your stuff
    const TableCacheEntry gameEmpireShips = { { GAME_EMPIRE_SHIPS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireShips);

    const TableCacheEntry gameEmpireFleets = { { GAME_EMPIRE_FLEETS, NO_KEY, countof(m_gameEmpireCols), m_gameEmpireCols }, NULL, NULL, NULL };
    Cache(cache, gameEmpireFleets);

    const TableCacheEntry allGameEmpireMaps = { { GAME_EMPIRE_MAP, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireMap::EmpireKey, NULL };
    Cache(cache, allGameEmpireMaps);
}

int HtmlRenderer::AfterCache_Quit()
{
    int iErrCode;
  
    iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, m_iEmpireKey, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_SHIPS | EMPTY_GAME_EMPIRE_FLEETS | EMPTY_GAME_EMPIRE_MAP);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

void HtmlRenderer::RegisterCache_LatestNukes(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemNukeList);
}

void HtmlRenderer::RegisterCache_SpectatorGames(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemActiveGames);
}

void HtmlRenderer::RegisterCache_SystemContributions(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemCredits(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_LatestGames(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemLatestGames);
}

void HtmlRenderer::RegisterCache_TournamentAdministrator(Vector<TableCacheEntry>& cache)
{
    // Cache tables for tournament
    if (m_iTournamentKey != NO_KEY)
    {
        const TableCacheEntry systemTournamentEmpires = { { SYSTEM_TOURNAMENT_EMPIRES, NO_KEY, 1, &m_systemTournamentCol }, NULL, NULL, NULL };
        Cache(cache, systemTournamentEmpires);

        const TableCacheEntry systemTournamentTeamsN = { { SYSTEM_TOURNAMENT_TEAMS, NO_KEY, 1, &m_systemTournamentCol }, NULL, NULL, NULL };
        Cache(cache, systemTournamentTeamsN);
    }

    Cache(cache, systemActiveGames);
}

void HtmlRenderer::RegisterCache_PersonalTournaments(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemActiveGames);
}

void HtmlRenderer::RegisterCache_Tournaments(Vector<TableCacheEntry>& cache)
{
    // Cache tables for tournament
    if (m_iTournamentKey != NO_KEY)
    {
        const TableCacheEntry systemTournamentEmpires = { { SYSTEM_TOURNAMENT_EMPIRES, NO_KEY, 1, &m_systemTournamentCol }, NULL, NULL, NULL };
        Cache(cache, systemTournamentEmpires);

        const TableCacheEntry systemTournamentTeamsN = { { SYSTEM_TOURNAMENT_TEAMS, NO_KEY, 1, &m_systemTournamentCol }, NULL, NULL, NULL };
        Cache(cache, systemTournamentTeamsN);
    }

    Cache(cache, systemActiveGames);
}

void HtmlRenderer::RegisterCache_GameTos(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemTos(Vector<TableCacheEntry>& cache)
{
}

// ...

int HtmlRenderer::AfterCache_ActiveGameList()
{
    return OK;
}

int HtmlRenderer::AfterCache_Login()
{
    return OK;
}

int HtmlRenderer::AfterCache_NewEmpire()
{
    return OK;
}

int HtmlRenderer::AfterCache_OpenGameList()
{
    return OK;
}

int HtmlRenderer::AfterCache_SystemGameList()
{
    return OK;
}

int HtmlRenderer::AfterCache_ProfileEditor()
{
    return OK;
}

int HtmlRenderer::AfterCache_TopLists()
{
    return OK;
}

int HtmlRenderer::AfterCache_ProfileViewer()
{
    return OK;
}

int HtmlRenderer::AfterCache_ServerAdministrator()
{
    return OK;
}

int HtmlRenderer::AfterCache_EmpireAdministrator()
{
    return OK;
}

int HtmlRenderer::AfterCache_GameAdministrator()
{
    return OK;
}

int HtmlRenderer::AfterCache_ThemeAdministrator()
{
    return OK;
}

int HtmlRenderer::AfterCache_PersonalGameClasses()
{
    return OK;
}

int HtmlRenderer::AfterCache_Chatroom()
{
    return OK;
}

int HtmlRenderer::AfterCache_SystemServerRules()
{
    return OK;
}

int HtmlRenderer::AfterCache_SystemFAQ()
{
    return OK;
}

int HtmlRenderer::AfterCache_SystemNews()
{
    return OK;
}

int HtmlRenderer::AfterCache_LatestNukes()
{
    return OK;
}

int HtmlRenderer::AfterCache_SpectatorGames()
{
    return OK;
}

int HtmlRenderer::AfterCache_SystemContributions()
{
    return OK;
}

int HtmlRenderer::AfterCache_SystemCredits()
{
    return OK;
}

int HtmlRenderer::AfterCache_LatestGames()
{
    return OK;
}

int HtmlRenderer::AfterCache_TournamentAdministrator()
{
    return OK;
}

int HtmlRenderer::AfterCache_PersonalTournaments()
{
    return OK;
}

int HtmlRenderer::AfterCache_Tournaments()
{
    return OK;
}

int HtmlRenderer::AfterCache_GameTos()
{
    return OK;
}

int HtmlRenderer::AfterCache_SystemTos()
{
    return OK;
}

void HtmlRenderer::RegisterCacheRatioTablesIfNecessary(Vector<TableCacheEntry>& cache)
{
    if (ShouldDisplayGameRatios())
    {
        // Ratios line
        const TableCacheEntry allGameEmpireDiplomacy = { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(m_gameCols), m_gameCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL };
        Cache(cache, allGameEmpireDiplomacy);
    }
}

int HtmlRenderer::AfterCacheRatioTablesIfNecessary()
{
    if (ShouldDisplayGameRatios())
    {
        int iErrCode;

        iErrCode = CreateEmptyGameCacheEntries(m_iGameClass, m_iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_DIPLOMACY);
        RETURN_ON_ERROR(iErrCode);
    }
    return OK;
}
