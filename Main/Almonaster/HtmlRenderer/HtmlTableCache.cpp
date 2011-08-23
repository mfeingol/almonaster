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

static TableCacheEntry systemData = { SYSTEM_DATA, NO_KEY, 0, NULL };
static TableCacheEntry systemThemes = { SYSTEM_THEMES, NO_KEY, 0, NULL };
static TableCacheEntry systemGameClassData = { SYSTEM_GAMECLASS_DATA, NO_KEY, 0, NULL };
static TableCacheEntry systemSystemGameClassData = { SYSTEM_SYSTEM_GAMECLASS_DATA, NO_KEY, 0, NULL };
static TableCacheEntry systemSuperClassData = { SYSTEM_SUPERCLASS_DATA, NO_KEY, 0, NULL };
static TableCacheEntry systemActiveGames = { SYSTEM_ACTIVE_GAMES, NO_KEY, 0, NULL };

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
    Cache(cache, systemSystemGameClassData);
    Cache(cache, systemSuperClassData);

    if (m_iEmpireKey != NO_KEY)
    {
        TableCacheEntry systemEmpireDataN = { SYSTEM_EMPIRE_DATA, m_iEmpireKey, 0, NULL };
        Cache(cache, systemEmpireDataN);
    }
}

void HtmlRenderer::GatherCacheTablesForGamePage(Vector<TableCacheEntry>& cache)
{
    Cache(cache, systemData);
    Cache(cache, systemThemes);
    Cache(cache, systemGameClassData);

    if (m_iEmpireKey != NO_KEY)
    {
        TableCacheEntry systemEmpireDataN = { SYSTEM_EMPIRE_DATA, m_iEmpireKey, 0, NULL };
        Cache(cache, systemEmpireDataN);
    }

    if (m_iGameClass != NO_KEY && m_iGameNumber != -1)
    {
        GAME_DATA(pszGameData, m_iGameClass, m_iGameNumber);
        m_strGameData = pszGameData;
        TableCacheEntry gameData = { m_strGameData.GetCharPtr(), NO_KEY, 0, NULL };
        Cache(cache, gameData);
    }
}

void HtmlRenderer::RegisterCache_ActiveGameList(Vector<TableCacheEntry>& cache)
{
    if (m_iEmpireKey != NO_KEY)
    {
        m_activeGameCol.pszColumn = SystemEmpireActiveGames::EmpireKey;
        m_activeGameCol.vData = m_iEmpireKey;
        TableCacheEntry systemEmpireActiveGamesN = { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_activeGameCol };
        Cache(cache, systemEmpireActiveGamesN);
    }
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

    Assert(m_iEmpireKey != NO_KEY);
    m_activeGameCol.pszColumn = SystemEmpireActiveGames::EmpireKey;
    m_activeGameCol.vData = m_iEmpireKey;
    TableCacheEntry systemEmpireActiveGamesN = { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_activeGameCol };
    Cache(cache, systemEmpireActiveGamesN);
}

void HtmlRenderer::RegisterCache_SystemGameList(Vector<TableCacheEntry>& cache)
{
    // Needed because when you enter a game, the system needs to know if you're in any games, and if so, whether you're idle
    Assert(m_iEmpireKey != NO_KEY);
    m_activeGameCol.pszColumn = SystemEmpireActiveGames::EmpireKey;
    m_activeGameCol.vData = m_iEmpireKey;
    TableCacheEntry systemEmpireActiveGamesN = { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &m_activeGameCol };
    Cache(cache, systemEmpireActiveGamesN);
}

void HtmlRenderer::RegisterCache_ProfileEditor(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_TopLists(Vector<TableCacheEntry>& cache)
{
    TableCacheEntry almonasterScore = { SYSTEM_ALMONASTER_SCORE_TOPLIST, NO_KEY };
    Cache(cache, almonasterScore);

    TableCacheEntry classicScore = { SYSTEM_CLASSIC_SCORE_TOPLIST, NO_KEY };
    Cache(cache, classicScore);

    TableCacheEntry bridierScore = { SYSTEM_BRIDIER_SCORE_TOPLIST, NO_KEY };
    Cache(cache, bridierScore);

    TableCacheEntry bridierEstablishedScore = { SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST, NO_KEY };
    Cache(cache, bridierEstablishedScore);
}

void HtmlRenderer::RegisterCache_ProfileViewer(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_ServerAdministrator(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_EmpireAdministrator(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameAdministrator(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_ThemeAdministrator(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_PersonalGameClasses(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Chatroom(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemServerRules(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemFAQ(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemNews(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Info(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Tech(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Diplomacy(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Map(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Planets(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Options(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Build(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Ships(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameServerRules(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameFAQ(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameNews(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameProfileViewer(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Quit(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_LatestNukes(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SpectatorGames(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameContributions(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameCredits(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemContributions(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemCredits(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_LatestGames(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_TournamentAdministrator(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_PersonalTournaments(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_Tournaments(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_GameTos(Vector<TableCacheEntry>& cache)
{
}

void HtmlRenderer::RegisterCache_SystemTos(Vector<TableCacheEntry>& cache)
{
}