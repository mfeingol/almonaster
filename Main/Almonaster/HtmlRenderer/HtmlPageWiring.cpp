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

typedef int (*RenderFunction)(HtmlRenderer*);
typedef void (*RegisterCacheFunction)(HtmlRenderer*, Vector<TableCacheEntry>& cache);
typedef int (*AfterCacheFunction)(HtmlRenderer*);

//
// Wiring
//

int Fxn_Render_ActiveGameList(HtmlRenderer* pThis)
{
    return pThis->Render_ActiveGameList();
}

int Fxn_Render_Login(HtmlRenderer* pThis)
{
    return pThis->Render_Login();
}

int Fxn_Render_NewEmpire(HtmlRenderer* pThis)
{
    return pThis->Render_NewEmpire();
}

int Fxn_Render_OpenGameList(HtmlRenderer* pThis)
{
    return pThis->Render_OpenGameList();
}

int Fxn_Render_SystemGameList(HtmlRenderer* pThis)
{
    return pThis->Render_SystemGameList();
}

int Fxn_Render_ProfileEditor(HtmlRenderer* pThis)
{
    return pThis->Render_ProfileEditor();
}

int Fxn_Render_TopLists(HtmlRenderer* pThis)
{
    return pThis->Render_TopLists();
}

int Fxn_Render_ProfileViewer(HtmlRenderer* pThis)
{
    return pThis->Render_ProfileViewer();
}

int Fxn_Render_ServerAdministrator(HtmlRenderer* pThis)
{
    return pThis->Render_ServerAdministrator();
}

int Fxn_Render_EmpireAdministrator(HtmlRenderer* pThis)
{
    return pThis->Render_EmpireAdministrator();
}

int Fxn_Render_GameAdministrator(HtmlRenderer* pThis)
{
    return pThis->Render_GameAdministrator();
}

int Fxn_Render_ThemeAdministrator(HtmlRenderer* pThis)
{
    return pThis->Render_ThemeAdministrator();
}

int Fxn_Render_PersonalGameClasses(HtmlRenderer* pThis)
{
    return pThis->Render_PersonalGameClasses();
}

int Fxn_Render_Chatroom(HtmlRenderer* pThis)
{
    return pThis->Render_Chatroom();
}

int Fxn_Render_SystemServerRules(HtmlRenderer* pThis)
{
    return pThis->Render_SystemServerRules();
}

int Fxn_Render_SystemFAQ(HtmlRenderer* pThis)
{
    return pThis->Render_SystemFAQ();
}

int Fxn_Render_SystemNews(HtmlRenderer* pThis)
{
    return pThis->Render_SystemNews();
}

int Fxn_Render_Info(HtmlRenderer* pThis)
{
    return pThis->Render_Info();
}

int Fxn_Render_Tech(HtmlRenderer* pThis)
{
    return pThis->Render_Tech();
}

int Fxn_Render_Diplomacy(HtmlRenderer* pThis)
{
    return pThis->Render_Diplomacy();
}

int Fxn_Render_Map(HtmlRenderer* pThis)
{
    return pThis->Render_Map();
}

int Fxn_Render_Planets(HtmlRenderer* pThis)
{
    return pThis->Render_Planets();
}

int Fxn_Render_Options(HtmlRenderer* pThis)
{
    return pThis->Render_Options();
}

int Fxn_Render_Build(HtmlRenderer* pThis)
{
    return pThis->Render_Build();
}

int Fxn_Render_Ships(HtmlRenderer* pThis)
{
    return pThis->Render_Ships();
}

int Fxn_Render_GameServerRules(HtmlRenderer* pThis)
{
    return pThis->Render_GameServerRules();
}

int Fxn_Render_GameFAQ(HtmlRenderer* pThis)
{
    return pThis->Render_GameFAQ();
}

int Fxn_Render_GameNews(HtmlRenderer* pThis)
{
    return pThis->Render_GameNews();
}

int Fxn_Render_GameProfileViewer(HtmlRenderer* pThis)
{
    return pThis->Render_GameProfileViewer();
}

int Fxn_Render_Quit(HtmlRenderer* pThis)
{
    return pThis->Render_Quit();
}

int Fxn_Render_LatestNukes(HtmlRenderer* pThis)
{
    return pThis->Render_LatestNukes();
}

int Fxn_Render_SpectatorGames(HtmlRenderer* pThis)
{
    return pThis->Render_SpectatorGames();
}

int Fxn_Render_GameContributions(HtmlRenderer* pThis)
{
    return pThis->Render_GameContributions();
}

int Fxn_Render_GameCredits(HtmlRenderer* pThis)
{
    return pThis->Render_GameCredits();
}

int Fxn_Render_SystemContributions(HtmlRenderer* pThis)
{
    return pThis->Render_SystemContributions();
}

int Fxn_Render_SystemCredits(HtmlRenderer* pThis)
{
    return pThis->Render_SystemCredits();
}

int Fxn_Render_LatestGames(HtmlRenderer* pThis)
{
    return pThis->Render_LatestGames();
}

int Fxn_Render_TournamentAdministrator(HtmlRenderer* pThis)
{
    return pThis->Render_TournamentAdministrator();
}

int Fxn_Render_PersonalTournaments(HtmlRenderer* pThis)
{
    return pThis->Render_PersonalTournaments();
}

int Fxn_Render_Tournaments(HtmlRenderer* pThis)
{
    return pThis->Render_Tournaments();
}

int Fxn_Render_GameTos(HtmlRenderer* pThis)
{
    return pThis->Render_GameTos();
}

int Fxn_Render_SystemTos(HtmlRenderer* pThis)
{
    return pThis->Render_SystemTos();
}

const RenderFunction g_pfxnRenderPage[] = {
    NULL,
    Fxn_Render_ActiveGameList,
    Fxn_Render_Login,
    Fxn_Render_NewEmpire,
    Fxn_Render_OpenGameList,
    Fxn_Render_SystemGameList,
    Fxn_Render_ProfileEditor,
    Fxn_Render_TopLists,
    Fxn_Render_ProfileViewer,
    Fxn_Render_ServerAdministrator,
    Fxn_Render_EmpireAdministrator,
    Fxn_Render_GameAdministrator,
    Fxn_Render_ThemeAdministrator,
    Fxn_Render_PersonalGameClasses,
    Fxn_Render_Chatroom,
    Fxn_Render_SystemServerRules,
    Fxn_Render_SystemFAQ,
    Fxn_Render_SystemNews,
    Fxn_Render_Info,
    Fxn_Render_Tech,
    Fxn_Render_Diplomacy,
    Fxn_Render_Map,
    Fxn_Render_Planets,
    Fxn_Render_Options,
    Fxn_Render_Build,
    Fxn_Render_Ships,
    Fxn_Render_GameServerRules,
    Fxn_Render_GameFAQ,
    Fxn_Render_GameNews,
    Fxn_Render_GameProfileViewer,
    Fxn_Render_GameContributions,
    Fxn_Render_GameCredits,
    Fxn_Render_GameTos,
    Fxn_Render_Quit,
    Fxn_Render_LatestNukes,
    Fxn_Render_SpectatorGames,
    Fxn_Render_SystemContributions,
    Fxn_Render_SystemCredits,
    Fxn_Render_LatestGames,
    Fxn_Render_TournamentAdministrator,
    Fxn_Render_PersonalTournaments,
    Fxn_Render_Tournaments,
    Fxn_Render_SystemTos,
    Fxn_Render_Tournaments,
    NULL
};

void Fxn_RegisterCache_ActiveGameList(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_ActiveGameList(cache);
}

void Fxn_RegisterCache_Login(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Login(cache);
}

void Fxn_RegisterCache_NewEmpire(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_NewEmpire(cache);
}

void Fxn_RegisterCache_OpenGameList(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_OpenGameList(cache);
}

void Fxn_RegisterCache_SystemGameList(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_SystemGameList(cache);
}

void Fxn_RegisterCache_ProfileEditor(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_ProfileEditor(cache);
}

void Fxn_RegisterCache_TopLists(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_TopLists(cache);
}

void Fxn_RegisterCache_ProfileViewer(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_ProfileViewer(cache);
}

void Fxn_RegisterCache_ServerAdministrator(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_ServerAdministrator(cache);
}

void Fxn_RegisterCache_EmpireAdministrator(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_EmpireAdministrator(cache);
}

void Fxn_RegisterCache_GameAdministrator(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_GameAdministrator(cache);
}

void Fxn_RegisterCache_ThemeAdministrator(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_ThemeAdministrator(cache);
}

void Fxn_RegisterCache_PersonalGameClasses(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_PersonalGameClasses(cache);
}

void Fxn_RegisterCache_Chatroom(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Chatroom(cache);
}

void Fxn_RegisterCache_SystemServerRules(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_SystemServerRules(cache);
}

void Fxn_RegisterCache_SystemFAQ(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_SystemFAQ(cache);
}

void Fxn_RegisterCache_SystemNews(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_SystemNews(cache);
}

void Fxn_RegisterCache_Info(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Info(cache);
}

void Fxn_RegisterCache_Tech(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Tech(cache);
}

void Fxn_RegisterCache_Diplomacy(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Diplomacy(cache);
}

void Fxn_RegisterCache_Map(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Map(cache);
}

void Fxn_RegisterCache_Planets(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Planets(cache);
}

void Fxn_RegisterCache_Options(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Options(cache);
}

void Fxn_RegisterCache_Build(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Build(cache);
}

void Fxn_RegisterCache_Ships(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Ships(cache);
}

void Fxn_RegisterCache_GameServerRules(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_GameServerRules(cache);
}

void Fxn_RegisterCache_GameFAQ(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_GameFAQ(cache);
}

void Fxn_RegisterCache_GameNews(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_GameNews(cache);
}

void Fxn_RegisterCache_GameProfileViewer(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_GameProfileViewer(cache);
}

void Fxn_RegisterCache_Quit(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Quit(cache);
}

void Fxn_RegisterCache_LatestNukes(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_LatestNukes(cache);
}

void Fxn_RegisterCache_SpectatorGames(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_SpectatorGames(cache);
}

void Fxn_RegisterCache_GameContributions(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_GameContributions(cache);
}

void Fxn_RegisterCache_GameCredits(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_GameCredits(cache);
}

void Fxn_RegisterCache_SystemContributions(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_SystemContributions(cache);
}

void Fxn_RegisterCache_SystemCredits(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_SystemCredits(cache);
}

void Fxn_RegisterCache_LatestGames(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_LatestGames(cache);
}

void Fxn_RegisterCache_TournamentAdministrator(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_TournamentAdministrator(cache);
}

void Fxn_RegisterCache_PersonalTournaments(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_PersonalTournaments(cache);
}

void Fxn_RegisterCache_Tournaments(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_Tournaments(cache);
}

void Fxn_RegisterCache_GameTos(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_GameTos(cache);
}

void Fxn_RegisterCache_SystemTos(HtmlRenderer* pThis, Vector<TableCacheEntry>& cache)
{
    pThis->RegisterCache_SystemTos(cache);
}

const RegisterCacheFunction g_pfxnRegisterCachePage[] = {
    NULL,
    Fxn_RegisterCache_ActiveGameList,
    Fxn_RegisterCache_Login,
    Fxn_RegisterCache_NewEmpire,
    Fxn_RegisterCache_OpenGameList,
    Fxn_RegisterCache_SystemGameList,
    Fxn_RegisterCache_ProfileEditor,
    Fxn_RegisterCache_TopLists,
    Fxn_RegisterCache_ProfileViewer,
    Fxn_RegisterCache_ServerAdministrator,
    Fxn_RegisterCache_EmpireAdministrator,
    Fxn_RegisterCache_GameAdministrator,
    Fxn_RegisterCache_ThemeAdministrator,
    Fxn_RegisterCache_PersonalGameClasses,
    Fxn_RegisterCache_Chatroom,
    Fxn_RegisterCache_SystemServerRules,
    Fxn_RegisterCache_SystemFAQ,
    Fxn_RegisterCache_SystemNews,
    Fxn_RegisterCache_Info,
    Fxn_RegisterCache_Tech,
    Fxn_RegisterCache_Diplomacy,
    Fxn_RegisterCache_Map,
    Fxn_RegisterCache_Planets,
    Fxn_RegisterCache_Options,
    Fxn_RegisterCache_Build,
    Fxn_RegisterCache_Ships,
    Fxn_RegisterCache_GameServerRules,
    Fxn_RegisterCache_GameFAQ,
    Fxn_RegisterCache_GameNews,
    Fxn_RegisterCache_GameProfileViewer,
    Fxn_RegisterCache_GameContributions,
    Fxn_RegisterCache_GameCredits,
    Fxn_RegisterCache_GameTos,
    Fxn_RegisterCache_Quit,
    Fxn_RegisterCache_LatestNukes,
    Fxn_RegisterCache_SpectatorGames,
    Fxn_RegisterCache_SystemContributions,
    Fxn_RegisterCache_SystemCredits,
    Fxn_RegisterCache_LatestGames,
    Fxn_RegisterCache_TournamentAdministrator,
    Fxn_RegisterCache_PersonalTournaments,
    Fxn_RegisterCache_Tournaments,
    Fxn_RegisterCache_SystemTos,
    Fxn_RegisterCache_Tournaments,
    NULL
};

int Fxn_AfterCache_ActiveGameList(HtmlRenderer* pThis)
{
    return pThis->AfterCache_ActiveGameList();
}

int Fxn_AfterCache_Login(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Login();
}

int Fxn_AfterCache_NewEmpire(HtmlRenderer* pThis)
{
    return pThis->AfterCache_NewEmpire();
}

int Fxn_AfterCache_OpenGameList(HtmlRenderer* pThis)
{
    return pThis->AfterCache_OpenGameList();
}

int Fxn_AfterCache_SystemGameList(HtmlRenderer* pThis)
{
    return pThis->AfterCache_SystemGameList();
}

int Fxn_AfterCache_ProfileEditor(HtmlRenderer* pThis)
{
    return pThis->AfterCache_ProfileEditor();
}

int Fxn_AfterCache_TopLists(HtmlRenderer* pThis)
{
    return pThis->AfterCache_TopLists();
}

int Fxn_AfterCache_ProfileViewer(HtmlRenderer* pThis)
{
    return pThis->AfterCache_ProfileViewer();
}

int Fxn_AfterCache_ServerAdministrator(HtmlRenderer* pThis)
{
    return pThis->AfterCache_ServerAdministrator();
}

int Fxn_AfterCache_EmpireAdministrator(HtmlRenderer* pThis)
{
    return pThis->AfterCache_EmpireAdministrator();
}

int Fxn_AfterCache_GameAdministrator(HtmlRenderer* pThis)
{
    return pThis->AfterCache_GameAdministrator();
}

int Fxn_AfterCache_ThemeAdministrator(HtmlRenderer* pThis)
{
    return pThis->AfterCache_ThemeAdministrator();
}

int Fxn_AfterCache_PersonalGameClasses(HtmlRenderer* pThis)
{
    return pThis->AfterCache_PersonalGameClasses();
}

int Fxn_AfterCache_Chatroom(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Chatroom();
}

int Fxn_AfterCache_SystemServerRules(HtmlRenderer* pThis)
{
    return pThis->AfterCache_SystemServerRules();
}

int Fxn_AfterCache_SystemFAQ(HtmlRenderer* pThis)
{
    return pThis->AfterCache_SystemFAQ();
}

int Fxn_AfterCache_SystemNews(HtmlRenderer* pThis)
{
    return pThis->AfterCache_SystemNews();
}

int Fxn_AfterCache_Info(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Info();
}

int Fxn_AfterCache_Tech(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Tech();
}

int Fxn_AfterCache_Diplomacy(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Diplomacy();
}

int Fxn_AfterCache_Map(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Map();
}

int Fxn_AfterCache_Planets(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Planets();
}

int Fxn_AfterCache_Options(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Options();
}

int Fxn_AfterCache_Build(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Build();
}

int Fxn_AfterCache_Ships(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Ships();
}

int Fxn_AfterCache_GameServerRules(HtmlRenderer* pThis)
{
    return pThis->AfterCache_GameServerRules();
}

int Fxn_AfterCache_GameFAQ(HtmlRenderer* pThis)
{
    return pThis->AfterCache_GameFAQ();
}

int Fxn_AfterCache_GameNews(HtmlRenderer* pThis)
{
    return pThis->AfterCache_GameNews();
}

int Fxn_AfterCache_GameProfileViewer(HtmlRenderer* pThis)
{
    return pThis->AfterCache_GameProfileViewer();
}

int Fxn_AfterCache_Quit(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Quit();
}

int Fxn_AfterCache_LatestNukes(HtmlRenderer* pThis)
{
    return pThis->AfterCache_LatestNukes();
}

int Fxn_AfterCache_SpectatorGames(HtmlRenderer* pThis)
{
    return pThis->AfterCache_SpectatorGames();
}

int Fxn_AfterCache_GameContributions(HtmlRenderer* pThis)
{
    return pThis->AfterCache_GameContributions();
}

int Fxn_AfterCache_GameCredits(HtmlRenderer* pThis)
{
    return pThis->AfterCache_GameCredits();
}

int Fxn_AfterCache_SystemContributions(HtmlRenderer* pThis)
{
    return pThis->AfterCache_SystemContributions();
}

int Fxn_AfterCache_SystemCredits(HtmlRenderer* pThis)
{
    return pThis->AfterCache_SystemCredits();
}

int Fxn_AfterCache_LatestGames(HtmlRenderer* pThis)
{
    return pThis->AfterCache_LatestGames();
}

int Fxn_AfterCache_TournamentAdministrator(HtmlRenderer* pThis)
{
    return pThis->AfterCache_TournamentAdministrator();
}

int Fxn_AfterCache_PersonalTournaments(HtmlRenderer* pThis)
{
    return pThis->AfterCache_PersonalTournaments();
}

int Fxn_AfterCache_Tournaments(HtmlRenderer* pThis)
{
    return pThis->AfterCache_Tournaments();
}

int Fxn_AfterCache_GameTos(HtmlRenderer* pThis)
{
    return pThis->AfterCache_GameTos();
}

int Fxn_AfterCache_SystemTos(HtmlRenderer* pThis)
{
    return pThis->AfterCache_SystemTos();
}

const AfterCacheFunction g_pfxnAfterCachePage[] = {
    NULL,
    Fxn_AfterCache_ActiveGameList,
    Fxn_AfterCache_Login,
    Fxn_AfterCache_NewEmpire,
    Fxn_AfterCache_OpenGameList,
    Fxn_AfterCache_SystemGameList,
    Fxn_AfterCache_ProfileEditor,
    Fxn_AfterCache_TopLists,
    Fxn_AfterCache_ProfileViewer,
    Fxn_AfterCache_ServerAdministrator,
    Fxn_AfterCache_EmpireAdministrator,
    Fxn_AfterCache_GameAdministrator,
    Fxn_AfterCache_ThemeAdministrator,
    Fxn_AfterCache_PersonalGameClasses,
    Fxn_AfterCache_Chatroom,
    Fxn_AfterCache_SystemServerRules,
    Fxn_AfterCache_SystemFAQ,
    Fxn_AfterCache_SystemNews,
    Fxn_AfterCache_Info,
    Fxn_AfterCache_Tech,
    Fxn_AfterCache_Diplomacy,
    Fxn_AfterCache_Map,
    Fxn_AfterCache_Planets,
    Fxn_AfterCache_Options,
    Fxn_AfterCache_Build,
    Fxn_AfterCache_Ships,
    Fxn_AfterCache_GameServerRules,
    Fxn_AfterCache_GameFAQ,
    Fxn_AfterCache_GameNews,
    Fxn_AfterCache_GameProfileViewer,
    Fxn_AfterCache_GameContributions,
    Fxn_AfterCache_GameCredits,
    Fxn_AfterCache_GameTos,
    Fxn_AfterCache_Quit,
    Fxn_AfterCache_LatestNukes,
    Fxn_AfterCache_SpectatorGames,
    Fxn_AfterCache_SystemContributions,
    Fxn_AfterCache_SystemCredits,
    Fxn_AfterCache_LatestGames,
    Fxn_AfterCache_TournamentAdministrator,
    Fxn_AfterCache_PersonalTournaments,
    Fxn_AfterCache_Tournaments,
    Fxn_AfterCache_SystemTos,
    Fxn_AfterCache_Tournaments,
    NULL
};

//
// Wiring
//

void HtmlRenderer::ReadStandardForms()
{
    IHttpForm* pHttpForm;
   
    if ((pHttpForm = m_pHttpRequest->GetForm("EmpireKey")) != NULL)
    {
        m_iEmpireKey = pHttpForm->GetUIntValue();
    }

    if (IsGamePage(m_pgPageId))
    {
        // Get game class
        if ((pHttpForm = m_pHttpRequest->GetForm ("GameClass")) != NULL)
        {
            m_iGameClass = pHttpForm->GetUIntValue();
        }

        // Get game number
        if ((pHttpForm = m_pHttpRequest->GetForm ("GameNumber")) != NULL)
        {
            m_iGameNumber = pHttpForm->GetUIntValue();
        }
    }

    // Get tournament key
    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentKey")) != NULL)
    {
        m_iTournamentKey = pHttpForm->GetUIntValue();
    }
    else if ((pHttpForm = m_pHttpRequest->GetForm ("AdminTournament")) != NULL)
    {
        m_iTournamentKey = pHttpForm->GetUIntValue();
    }
}

void HtmlRenderer::GatherCacheTables(PageId pgPageId, Vector<TableCacheEntry>& cache)
{
    Assert(pgPageId > MIN_PAGE_ID && pgPageId < MAX_PAGE_ID);

    if (IsGamePage(pgPageId))
    {
        GatherCacheTablesForGamePage(cache);
    }
    else
    {
        GatherCacheTablesForSystemPage(cache);
    }

    g_pfxnRegisterCachePage[pgPageId](this, cache);
}

int HtmlRenderer::CacheTables(PageId pgPageId, Vector<TableCacheEntry>& cache)
{
    // Prefetch tables of interest
    int iErrCode = t_pCache->Cache(cache.GetData(), cache.GetNumElements());
    RETURN_ON_ERROR(iErrCode);

    if (IsGamePage(pgPageId))
    {
        iErrCode = AfterCacheTablesForGamePage();
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = g_pfxnAfterCachePage[pgPageId](this);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int HtmlRenderer::Render()
{
    Assert(m_pgPageId > MIN_PAGE_ID && m_pgPageId < MAX_PAGE_ID);

    ReadStandardForms();

    Vector<TableCacheEntry> cache;
    GatherCacheTables(m_pgPageId, cache);
    
    int iErrCode = global.TlsOpenConnection();
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = CacheTables(m_pgPageId, cache);
    if (iErrCode == OK)
    {
        // Render the page
        iErrCode = g_pfxnRenderPage[m_pgPageId](this);
        if (iErrCode == OK)
        {
            iErrCode = global.TlsCommitTransaction();
            if (iErrCode != OK)
            {
                global.WriteReport(TRACE_ERROR, "CommitTransaction failed");
            }
        }
        else
        {
            global.WriteReport(TRACE_ERROR, "Page render failed - aborting transaction");
        }
    }
    else
    {
        global.WriteReport(TRACE_ERROR, "CacheTables failed - aborting transaction");
    }
    global.TlsCloseConnection();
    
    return iErrCode;
}

int HtmlRenderer::Redirect(PageId pageId)
{
    Assert(pageId > MIN_PAGE_ID && pageId < MAX_PAGE_ID);

    // Best effort clear response buffer
    m_pHttpResponse->Clear();

    // Redirect to new page
    m_bRedirection = true;
    m_pgPageId = pageId;

    // Add to cache if needed
    Vector<TableCacheEntry> cache;
    GatherCacheTables(pageId, cache);
    int iErrCode = CacheTables(pageId, cache);
    RETURN_ON_ERROR(iErrCode);

    // Render the page
    iErrCode = g_pfxnRenderPage[m_pgPageId](this);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}