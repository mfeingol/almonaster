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
typedef void (*CacheFunction)(HtmlRenderer*, Vector<TableCacheEntry>& cache);

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

const CacheFunction g_pfxnRegisterCachePage[] = {
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

//
// Wiring
//

void HtmlRenderer::ReadStandardForms()
{
    IHttpForm* pHttpForm;
   
    if ((pHttpForm = m_pHttpRequest->GetForm("EmpireKey")) != NULL)
    {
        m_iEmpireKey = pHttpForm->GetIntValue();
    }

    if (IsGamePage(m_pgPageId))
    {
        // Get game class
        if ((pHttpForm = m_pHttpRequest->GetForm ("GameClass")) != NULL)
        {
            m_iGameClass = pHttpForm->GetIntValue();
        }

        // Get game number
        if ((pHttpForm = m_pHttpRequest->GetForm ("GameNumber")) != NULL)
        {
            m_iGameNumber = pHttpForm->GetIntValue();
        }
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

int HtmlRenderer::CacheTables(Vector<TableCacheEntry>& cache)
{
    // Prefetch tables of interest
    return t_pCache->Cache(cache.GetData(), cache.GetNumElements());
}

int HtmlRenderer::Render()
{
    Assert(m_pgPageId > MIN_PAGE_ID && m_pgPageId < MAX_PAGE_ID);

    ReadStandardForms();

    Vector<TableCacheEntry> cache;
    GatherCacheTables(m_pgPageId, cache);
    
    global.TlsOpenConnection();
    
    int iErrCode = CacheTables(cache);
    if (iErrCode == OK)
    {
        // Render the page
        iErrCode = g_pfxnRenderPage[m_pgPageId](this);
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
    int iErrCode = CacheTables(cache);
    if (iErrCode == OK)
    {
        // Render the page
        iErrCode = g_pfxnRenderPage[m_pgPageId](this);
    }
    return iErrCode;
}

//static inline IPage* CreateActiveGameList(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    ActiveGameListPage* pPage = new ActiveGameListPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateLogin(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    LoginPage* pPage = new LoginPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateNewEmpire(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    NewEmpirePage* pPage = new NewEmpirePage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateOpenGameList(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    OpenGameListPage* pPage = new OpenGameListPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateSystemGameList(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    SystemGameListPage* pPage = new SystemGameListPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateProfileEditor(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    ProfileEditorPage* pPage = new ProfileEditorPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateTopLists(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    TopListsPage* pPage = new TopListsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateProfileViewer(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    ProfileViewerPage* pPage = new ProfileViewerPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateServerAdministrator(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    ServerAdministratorPage* pPage = new ServerAdministratorPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateEmpireAdministrator(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    EmpireAdministratorPage* pPage = new EmpireAdministratorPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateGameAdministrator(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    GameAdministratorPage* pPage = new GameAdministratorPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateThemeAdministrator(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    ThemeAdministratorPage* pPage = new ThemeAdministratorPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreatePersonalGameClasses(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    PersonalGameClassesPage* pPage = new PersonalGameClassesPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateChatroom(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    ChatroomPage* pPage = new ChatroomPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateSystemServerRules(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    SystemServerRulesPage* pPage = new SystemServerRulesPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateSystemFAQ(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    SystemFAQPage* pPage = new SystemFAQPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateSystemNews(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    SystemNewsPage* pPage = new SystemNewsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateInfo(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    InfoPage* pPage = new InfoPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateTech(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    TechPage* pPage = new TechPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateDiplomacy(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    DiplomacyPage* pPage = new DiplomacyPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateMap(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    MapPage* pPage = new MapPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreatePlanets(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    PlanetsPage* pPage = new PlanetsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateOptions(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    OptionsPage* pPage = new OptionsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateBuild(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    BuildPage* pPage = new BuildPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateShips(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    ShipsPage* pPage = new ShipsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateGameServerRules(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    GameServerRulesPage* pPage = new GameServerRulesPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateGameFAQ(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    GameFAQPage* pPage = new GameFAQPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateGameNews(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    GameNewsPage* pPage = new GameNewsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateGameProfileViewer(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    GameProfileViewerPage* pPage = new GameProfileViewerPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateQuit(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    QuitPage* pPage = new QuitPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateLatestNukes(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    LatestNukesPage* pPage = new LatestNukesPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateSpectatorGames(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    SpectatorGamesPage* pPage = new SpectatorGamesPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateGameContributions(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    GameContributionsPage* pPage = new GameContributionsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateGameCredits(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    GameCreditsPage* pPage = new GameCreditsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateSystemContributions(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    SystemContributionsPage* pPage = new SystemContributionsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateSystemCredits(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    SystemCreditsPage* pPage = new SystemCreditsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateLatestGames(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    LatestGamesPage* pPage = new LatestGamesPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateTournamentAdministrator(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    TournamentAdministratorPage* pPage = new TournamentAdministratorPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreatePersonalTournaments(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    PersonalTournamentsPage* pPage = new PersonalTournamentsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateTournaments(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    TournamentsPage* pPage = new TournamentsPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateGameTos(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    GameTosPage* pPage = new GameTosPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//static inline IPage* CreateSystemTos(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    SystemTosPage* pPage = new SystemTosPage();
//    pPage->Initialize(pgePageId, pHttpRequest, pHttpResponse, bRedirected);
//    return pPage;
//}
//
//typedef IPage* (*CreatePageFunction)(PageId pgePageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected);
//
//const CreatePageFunction g_pfxnCreatePage[] = {
//    NULL,
//    CreateActiveGameList,
//    CreateLogin,
//    CreateNewEmpire,
//    CreateOpenGameList,
//    CreateSystemGameList,
//    CreateProfileEditor,
//    CreateTopLists,
//    CreateProfileViewer,
//    CreateServerAdministrator,
//    CreateEmpireAdministrator,
//    CreateGameAdministrator,
//    CreateThemeAdministrator,
//    CreatePersonalGameClasses,
//    CreateChatroom,
//    CreateSystemServerRules,
//    CreateSystemFAQ,
//    CreateSystemNews,
//    CreateInfo,
//    CreateTech,
//    CreateDiplomacy,
//    CreateMap,
//    CreatePlanets,
//    CreateOptions,
//    CreateBuild,
//    CreateShips,
//    CreateGameServerRules,
//    CreateGameFAQ,
//    CreateGameNews,
//    CreateGameProfileViewer,
//    CreateGameContributions,
//    CreateGameCredits,
//    CreateGameTos,
//    CreateQuit,
//    CreateLatestNukes,
//    CreateSpectatorGames,
//    CreateSystemContributions,
//    CreateSystemCredits,
//    CreateLatestGames,
//    CreateTournamentAdministrator,
//    CreatePersonalTournaments,
//    CreateTournaments,
//    CreateSystemTos,
//    CreateTournaments,
//    NULL
//};
//
//IPage* HtmlRenderer::CreatePage(PageId pageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, bool bRedirected)
//{
//    Assert(pageId > MIN_PAGE_ID && pageId < MAX_PAGE_ID);
//    return g_pfxnCreatePage[pageId](pageId, pHttpRequest, pHttpResponse, bRedirected);
//}
//
//int HtmlRenderer::Redirect(PageId pageId)
//{
//    // Best effort
//    m_pHttpResponse->Clear();
//
//    Assert(pageId > MIN_PAGE_ID && pageId < MAX_PAGE_ID);
//    IPage* pNewPage = CreatePage(pageId, m_pHttpRequest, m_pHttpResponse, true);
//    Assert(pNewPage);
//
//    int iErrCode = pNewPage->Render();
//    SafeRelease(pNewPage);
//
//    return iErrCode;
//}