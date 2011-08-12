//
// Almonaster.dll:a component of Almonaster
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

// Function Pointers
static inline int THREAD_CALL Fxn_ActiveGameList (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ActiveGameList();
}

static inline int THREAD_CALL Fxn_Login (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Login();
}

static inline int THREAD_CALL Fxn_NewEmpire (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_NewEmpire();
}

static inline int THREAD_CALL Fxn_OpenGameList (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_OpenGameList();
}

static inline int THREAD_CALL Fxn_SystemGameList (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemGameList();
}

static inline int THREAD_CALL Fxn_ProfileEditor (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ProfileEditor();
}

static inline int THREAD_CALL Fxn_TopLists (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_TopLists();
}

static inline int THREAD_CALL Fxn_ProfileViewer (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ProfileViewer();
}

static inline int THREAD_CALL Fxn_ServerAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ServerAdministrator();
}

static inline int THREAD_CALL Fxn_EmpireAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_EmpireAdministrator();
}

static inline int THREAD_CALL Fxn_GameAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameAdministrator();
}

static inline int THREAD_CALL Fxn_ThemeAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ThemeAdministrator();
}

static inline int THREAD_CALL Fxn_PersonalGameClasses (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_PersonalGameClasses();
}

static inline int THREAD_CALL Fxn_Chatroom (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Chatroom();
}

static inline int THREAD_CALL Fxn_SystemServerRules (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemServerRules();
}

static inline int THREAD_CALL Fxn_SystemFAQ (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemFAQ();
}

static inline int THREAD_CALL Fxn_SystemNews (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemNews();
}

static inline int THREAD_CALL Fxn_Info (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Info();
}

static inline int THREAD_CALL Fxn_Tech (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Tech();
}

static inline int THREAD_CALL Fxn_Diplomacy (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Diplomacy();
}

static inline int THREAD_CALL Fxn_Map (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Map();
}

static inline int THREAD_CALL Fxn_Planets (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Planets();
}

static inline int THREAD_CALL Fxn_Options (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Options();
}

static inline int THREAD_CALL Fxn_Build (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Build();
}

static inline int THREAD_CALL Fxn_Ships (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Ships();
}

static inline int THREAD_CALL Fxn_GameServerRules (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameServerRules();
}

static inline int THREAD_CALL Fxn_GameFAQ (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameFAQ();
}

static inline int THREAD_CALL Fxn_GameNews (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameNews();
}

static inline int THREAD_CALL Fxn_GameProfileViewer (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameProfileViewer();
}

static inline int THREAD_CALL Fxn_Quit (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Quit();
}

static inline int THREAD_CALL Fxn_LatestNukes (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_LatestNukes();
}

static inline int THREAD_CALL Fxn_SpectatorGames (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SpectatorGames();
}

static inline int THREAD_CALL Fxn_GameContributions (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameContributions();
}

static inline int THREAD_CALL Fxn_GameCredits (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameCredits();
}

static inline int THREAD_CALL Fxn_SystemContributions (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemContributions();
}

static inline int THREAD_CALL Fxn_SystemCredits (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemCredits();
}

static inline int THREAD_CALL Fxn_LatestGames (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_LatestGames();
}

static inline int THREAD_CALL Fxn_TournamentAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_TournamentAdministrator();
}

static inline int THREAD_CALL Fxn_PersonalTournaments (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_PersonalTournaments();
}

static inline int THREAD_CALL Fxn_Tournaments (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Tournaments();
}

static inline int THREAD_CALL Fxn_GameTos (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameTos();
}

static inline int THREAD_CALL Fxn_SystemTos (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemTos();
}

const ThreadFunction g_pfxnRenderPage[] = {
    NULL,
    Fxn_ActiveGameList,
    Fxn_Login,
    Fxn_NewEmpire,
    Fxn_OpenGameList,
    Fxn_SystemGameList,
    Fxn_ProfileEditor,
    Fxn_TopLists,
    Fxn_ProfileViewer,
    Fxn_ServerAdministrator,
    Fxn_EmpireAdministrator,
    Fxn_GameAdministrator,
    Fxn_ThemeAdministrator,
    Fxn_PersonalGameClasses,
    Fxn_Chatroom,
    Fxn_SystemServerRules,
    Fxn_SystemFAQ,
    Fxn_SystemNews,
    Fxn_Info,
    Fxn_Tech,
    Fxn_Diplomacy,
    Fxn_Map,
    Fxn_Planets,
    Fxn_Options,
    Fxn_Build,
    Fxn_Ships,
    Fxn_GameServerRules,
    Fxn_GameFAQ,
    Fxn_GameNews,
    Fxn_GameProfileViewer,
    Fxn_GameContributions,
    Fxn_GameCredits,
    Fxn_GameTos,
    Fxn_Quit,
    Fxn_LatestNukes,
    Fxn_SpectatorGames,
    Fxn_SystemContributions,
    Fxn_SystemCredits,
    Fxn_LatestGames,
    Fxn_TournamentAdministrator,
    Fxn_PersonalTournaments,
    Fxn_Tournaments,
    Fxn_SystemTos,
    Fxn_Tournaments,
    NULL
};

int HtmlRenderer::Render()
{
    Assert (m_pgPageId > MIN_PAGE_ID && m_pgPageId < MAX_PAGE_ID);
    return g_pfxnRenderPage[m_pgPageId] (this);
}

int HtmlRenderer::Redirect (PageId pageId)
{
    Assert (pageId > MIN_PAGE_ID && pageId < MAX_PAGE_ID);
    
    m_bRedirection = true;
    m_pgPageId = pageId;
    
    // Best effort
    m_pHttpResponse->Clear();
    
    // Call the function
    return g_pfxnRenderPage[pageId] (this);
}

void HtmlRenderer::OpenSystemPage(bool bFileUpload)
{
    m_pHttpResponse->WriteText("<html><head><title>");

    WriteSystemTitleString();
    m_pHttpResponse->WriteText("</title></head>");
    WriteBodyString(-1);

    m_pHttpResponse->WriteText("<center>");

    WriteSystemHeaders(bFileUpload);

    PostSystemPageInformation();

    WriteSystemButtons(m_iButtonKey, m_iPrivilege);

    if (m_bTimeDisplay)
    { 
        char pszDateString [OS::MaxDateLength];
        if (Time::GetDateString(pszDateString) == OK)
        {
            OutputText("Server time is <strong>");
            m_pHttpResponse->WriteText(pszDateString);
            OutputText("</strong><p>");
        } 
    } 

    WriteSystemMessages();

    WriteSeparatorString(m_iSeparatorKey);
}

void HtmlRenderer::WriteSystemTitleString() {

    if (m_pgPageId != LOGIN && m_pgPageId != NEW_EMPIRE)
    {
        const char* pszEmpireName = m_vEmpireName.GetCharPtr();
        if (pszEmpireName != NULL)
        {
            m_pHttpResponse->WriteText (pszEmpireName);
            if (pszEmpireName [strlen (pszEmpireName) - 1] == 's')
            {
                OutputText ("' ");
            }
            else 
            {
                OutputText ("'s ");
            }
        }
    }

    m_pHttpResponse->WriteText (PageName [m_pgPageId]);
    OutputText (": ");
    WriteVersionString();
}

void HtmlRenderer::WriteSystemHeaders(bool bFileUpload)
{ 
    if (bFileUpload)
    {
        OutputText ("<form method=\"post\" enctype=\"multipart/form-data\">");
    } 
    else 
    {
        OutputText ("<form method=\"post\">");
    }

    OutputText ("<input type=\"hidden\" name=\"PageId\" value=\"");
    m_pHttpResponse->WriteText ((int) m_pgPageId);
    OutputText ("\">");




    // TODOTODOTODO - HACKHACKHACK
    OutputText("<p><strong><font size=\"3\">");
    OutputText("Script time: ");
    MilliSeconds msTime = GetTimerCount();
    m_pHttpResponse->WriteText((int) msTime);
    OutputText(" ms</font></strong>");



    const char* pszName = m_vEmpireName.GetCharPtr();

    WriteProfileAlienString (
        m_iAlienKey, 
        m_iEmpireKey, 
        m_vEmpireName.GetCharPtr(),
        0,
        "ProfileLink",
        "View your profile",
        false,
        false
        );

    OutputText(" <font size=\"+3\"><strong>");
    m_pHttpResponse->WriteText (pszName);

    if (pszName [strlen (pszName) - 1] == 's')
    {
        OutputText ("' ");
    }
    else
    {
        OutputText ("'s ");
    }
    m_pHttpResponse->WriteText (PageName [m_pgPageId]);

    if (m_iSystemOptions & EMPIRE_MARKED_FOR_DELETION)
    {
        OutputText (" (Empire marked for deletion)");
    }

    OutputText ("</strong></font><p>");
}

void HtmlRenderer::PostSystemPageInformation()
{
    int64 i64PasswordHash = 0;
    int iErrCode = GetPasswordHashForSystemPage (m_tNewSalt, &i64PasswordHash);
    Assert(iErrCode == OK);

    OutputText ("<input type=\"hidden\" name=\"EmpireKey\" value=\"");
    m_pHttpResponse->WriteText (m_iEmpireKey);
    OutputText ("\"><input type=\"hidden\" name=\"Password\" value=\"");
    m_pHttpResponse->WriteText (i64PasswordHash);
    OutputText ("\"><input type=\"hidden\" name=\"Salt\" value=\"");
    m_pHttpResponse->WriteText (m_tNewSalt);
    OutputText ("\">");
}

void HtmlRenderer::WriteSystemButtons (int iButtonKey, int iPrivilege) {

    unsigned int iNumber;

    // ActiveGameList
    WriteButton (BID_ACTIVEGAMELIST);

    // OpenGameList
    WriteButton (BID_OPENGAMELIST);

    // SystemGameList
    WriteButton (BID_SYSTEMGAMELIST);

    // ProfileViewer
    OutputText ("<br>");
    WriteButton (BID_PROFILEVIEWER);

    // ProfileEditor
    WriteButton (BID_PROFILEEDITOR);

    // Top Lists
    WriteButton (BID_TOPLISTS);

    // Chatroom
    WriteButton (BID_CHATROOM);

    // Exit
    WriteButton (BID_EXIT);

    OutputText ("<br>");

    // Latest nukes
    WriteButton (BID_SPECTATORGAMES);
    WriteButton (BID_TOURNAMENTS);
    WriteButton (BID_LATESTGAMES);
    WriteButton (BID_LATESTNUKES);

    OutputText ("<br>");

    // Personal Game Classes
    if (iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES ||
        (GetEmpirePersonalGameClasses(m_iEmpireKey, NULL, NULL, (int*) &iNumber) == OK && iNumber > 0))
    {
        WriteButton (BID_PERSONALGAMECLASSES);
    }

    // Personal Tournaments
    if (iPrivilege >= PRIVILEGE_FOR_PERSONAL_TOURNAMENTS ||
        (GetOwnedTournaments (m_iEmpireKey, NULL, NULL, &iNumber) == OK && iNumber > 0))
    {
        WriteButton (BID_PERSONALTOURNAMENTS);
    }

    if (iPrivilege >= ADMINISTRATOR) {

        // Server Administrator
        OutputText ("<p>");
        WriteButton (BID_SERVERADMINISTRATOR);

        // Game Administrator
        WriteButton (BID_GAMEADMINISTRATOR);

        // Empire Administrator
        OutputText ("<br>");
        WriteButton (BID_EMPIREADMINISTRATOR);

        // Theme Administrator
        WriteButton (BID_THEMEADMINISTRATOR);

        // Tournament Administrator
        OutputText ("<br>");
        WriteButton (BID_TOURNAMENTADMINISTRATOR);
    }

    OutputText ("<p>");
}

void HtmlRenderer::OpenGamePage()
{
    m_pHttpResponse->WriteText("<html><head><title>");
    WriteGameTitleString();
    m_pHttpResponse->WriteText("</title></head>");

    if (m_iGameState & PAUSED)
    {
        WriteBodyString(-1);
    }
    else
    {
        WriteBodyString(m_sSecondsUntil);
    } 

    m_pHttpResponse->WriteText("<center>");
    WriteGameHeaderString();
}

void HtmlRenderer::WriteGameTitleString()
{
    const char* pszEmpireName = m_vEmpireName.GetCharPtr();

    m_pHttpResponse->WriteText (pszEmpireName);
    if (pszEmpireName [strlen (pszEmpireName) - 1] == 's')
    {
        OutputText ("' ");
    }
    else
    {
        OutputText ("'s ");
    }

    m_pHttpResponse->WriteText (PageName [m_pgPageId]);
    OutputText (": ");
    WriteVersionString();
}

void HtmlRenderer::WriteGameHeaderString()
{
    // Open form
    OutputText ("<form method=\"post\"><input type=\"hidden\" name=\"PageId\" value=\"");
    m_pHttpResponse->WriteText ((int) m_pgPageId);
    OutputText ("\">");

    const char* pszEmpireName = m_vEmpireName.GetCharPtr();

    WriteProfileAlienString (
        m_iAlienKey, 
        m_iEmpireKey, 
        m_vEmpireName.GetCharPtr(),
        0,
        "ProfileLink",
        "View your profile",
        false,
        false
        );

    OutputText (" <font size=\"+3\"><strong>");
    m_pHttpResponse->WriteText (pszEmpireName);

    if (pszEmpireName [strlen (pszEmpireName) - 1] == 's')
    {
        OutputText ("' ");
    }
    else
    {
        OutputText ("'s ");
    }

    m_pHttpResponse->WriteText (PageName[m_pgPageId]);
    OutputText (" : ");
    m_pHttpResponse->WriteText (m_pszGameClassName);
    OutputText (" ");
    m_pHttpResponse->WriteText (m_iGameNumber);
    OutputText ("</strong></font><p>");

    // Informational forms
    int iErrCode = PostGamePageInformation();
    Assert(iErrCode == OK);

    // Buttons
    WriteGameButtons();

    // Write local time
    if (m_bTimeDisplay)
    {
        char pszDateString [OS::MaxDateLength];
        if (Time::GetDateString (pszDateString) == OK)
        {
            OutputText ("Server time is <strong>");
            m_pHttpResponse->WriteText (pszDateString);
            OutputText ("</strong>");
        }
    }

    // Next update
    WriteGameNextUpdateString();

    // Messages
    WriteGameMessages();

    // Last separator
    WriteSeparatorString(m_iSeparatorKey);
}