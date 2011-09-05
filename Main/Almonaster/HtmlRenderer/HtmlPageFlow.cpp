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

int HtmlRenderer::OpenSystemPage(bool bFileUpload)
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

    int iErrCode = WriteSystemMessages();
    if (iErrCode != OK)
        return iErrCode;

    WriteSeparatorString(m_iSeparatorKey);

    return iErrCode;
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
        (GetEmpirePersonalGameClasses(m_iEmpireKey, NULL, NULL, &iNumber) == OK && iNumber > 0))
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

int HtmlRenderer::OpenGamePage()
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
    return WriteGameHeaderString();
}

void HtmlRenderer::WriteGameTitleString()
{
    const char* pszEmpireName = m_vEmpireName.GetCharPtr();
    m_pHttpResponse->WriteText(pszEmpireName);
    if (pszEmpireName[strlen (pszEmpireName) - 1] == 's')
    {
        OutputText ("' ");
    }
    else
    {
        OutputText ("'s ");
    }

    m_pHttpResponse->WriteText(PageName[m_pgPageId]);
    OutputText (": ");
    WriteVersionString();
}

int HtmlRenderer::WriteGameHeaderString()
{
    int iErrCode;

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
    iErrCode = PostGamePageInformation();
    if(iErrCode != OK)
        return iErrCode;

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
    iErrCode = WriteGameMessages();
    if (iErrCode != OK)
        return iErrCode;

    // Last separator
    WriteSeparatorString(m_iSeparatorKey);

    return OK;
}

void HtmlRenderer::WriteGameButtons() {
    
    OutputText ("<table border=\"0\" width=\"90%\"><tr><td width=\"2%\" align=\"left\">");
    
    // Info
    WriteButton (BID_INFO);
    
    OutputText ("</td><td align=\"center\">");
    
    // Map
    WriteButton (BID_MAP);
    
    // Planets
    WriteButton (BID_PLANETS);
    
    // Diplomacy
    WriteButton (BID_DIPLOMACY);
    
    OutputText ("</td><td width=\"2%\" align=\"right\">");
    
    // Exit
    WriteButton (BID_EXIT);
    
    OutputText ("</td></tr><tr><td width=\"2%\" align=\"left\">");
    
    // Options
    WriteButton (BID_OPTIONS);
    
    OutputText ("</td><td align=\"center\">");
    
    // Ships
    WriteButton (BID_SHIPS);
    
    // Build
    WriteButton (BID_BUILD);
    
    // Tech
    WriteButton (BID_TECH);
    
    if (m_iGameState & STARTED) {

        if (m_iGameOptions & DISPLACE_ENDTURN_BUTTON) {
            OutputText ("</td><td width=\"2%\" align=\"right\">");
        }
        
        if (m_iGameOptions & UPDATED) {
            // Unend Turn
            WriteButton (BID_UNENDTURN);
        } else {
            // End Turn
            WriteButton (BID_ENDTURN);
        }
        
    } else {
        
        OutputText ("</td><td width=\"2%\" align=\"right\">");
        WriteButton (BID_QUIT);
    }
    
    OutputText ("</td></tr></table><p>");
}


void HtmlRenderer::WriteGameNextUpdateString()
{
    int iErrCode;

    OutputText ("<p>");
    
    if (m_iNumNewUpdates > 0) {
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (m_iNumNewUpdates);
        OutputText ("</strong> update");
        if (m_iNumNewUpdates != 1) {
            OutputText ("s");
        }

        OutputText (", next ");

    } else {

        OutputText ("First update ");
    }

    unsigned int iNumEmpires;
    iErrCode = GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        return;
    }
    
    if (!(m_iGameOptions & COUNTDOWN) || (m_iGameState & PAUSED) || !(m_iGameState & STARTED)) {
        
        if (!(m_iGameState & STARTED)) {
            
            int iNumNeeded, iTotal, 
            
            iErrCode = GetNumEmpiresNeededForGame (m_iGameClass, &iNumNeeded);
            if (iErrCode != OK) {
                Assert (false);
                return;
            }
            
            iTotal = iNumNeeded - iNumEmpires;
            
            OutputText ("when <strong>");
            m_pHttpResponse->WriteText (iTotal);
            OutputText ("</strong>");
            if (iTotal == 1) {
                OutputText (" more empire joins");
            } else {
                OutputText (" more empires join");
            }

        } else {
            
            OutputText ("in ");
            WriteTime (m_sSecondsUntil);
            
            if (m_iGameState & PAUSED) {
                if (m_iGameState & ADMIN_PAUSED) {
                    OutputText (" (<strong>paused by an administrator</strong>)");
                } else {
                    OutputText (" (<strong>paused</strong>)");
                }
            }
        }
        
    } else {
        
        OutputText (
            
            "in <input name=\"jtimer\" size=\"22\"><script><!--\n"\
            "var t = (new Date()).getTime()/1000+");
        
        m_pHttpResponse->WriteText (m_sSecondsUntil - 1);
        OutputText (
            
            ";\n"\
            "function count() {\n"\
            "var next = '';\n"\
            "var pre = '';\n"\
            "var now = new Date();\n"\
            "var sec = Math.floor(t-now.getTime()/1000);\n"\
            "if (sec < 1) {\n"\
            "if (sec == 0) {\n"\
            "document.forms[0].jtimer.value='The update is occurring...';\n"\
            "setTimeout('count()',2000); return;\n"\
            "}\n"\
            "document.forms[0].jtimer.value='The update occurred';\n");
        
        if (m_iGameOptions & AUTO_REFRESH) {
            OutputText ("setTimeout('document.forms[0].submit()', 1000);\n");
        }
        
        OutputText (
            
            "return;\n"\
            "}\n"\
            "var hrs = Math.floor(sec/3600);\n"\
            "sec -= hrs*3600;\n"\
            "var min = Math.floor(sec/60);\n"\
            "sec -= min*60;\n"\
            "if (hrs) { next += hrs+' hr'; if (hrs != 1) next += 's'; }\n"\
            "if (min) { if (hrs) next += ', '; next += min+' min'; }\n"\
            "if (sec) { if (hrs || min) next += ', '; next += sec+' sec'; }\n"\
            "document.forms[0].jtimer.value=pre+next;\n"\
            "setTimeout('count()',500);\n"\
            "}\n"\
            "count(); // --></script>");
    }

    if (m_iGameState & STARTED) {

        int iUpdated;
        iErrCode = GetNumUpdatedEmpires (m_iGameClass, m_iGameNumber, &iUpdated);
        if (iErrCode != OK) {
            Assert (false);
            return;
        }

        if (m_iGameOptions & COUNTDOWN) {
            OutputText (" ");
        } else {
            OutputText (", ");
        }

        OutputText ("<strong>");
        m_pHttpResponse->WriteText (iUpdated);
        OutputText ("</strong> of <strong>");
        m_pHttpResponse->WriteText (iNumEmpires);
        OutputText ("</strong> ready");
    }

    if (m_iGameState & PAUSED && 
        m_iGameState & STILL_OPEN && 
        !(m_iGameState & ADMIN_PAUSED)) {
        OutputText("<p>(The game is still open, and will unpause if another empire joins)");
    }
    
    OutputText ("<p>");
}


int HtmlRenderer::PostGamePageInformation()
{
    int64 i64PasswordHash = 0;
    int iErrCode = GetPasswordHashForGamePage(m_tNewSalt, &i64PasswordHash);
    if (iErrCode != OK) {
        return iErrCode;
    }

    OutputText ("<input type=\"hidden\" name=\"EmpireKey\" value=\"");
    m_pHttpResponse->WriteText (m_iEmpireKey);
    OutputText ("\"><input type=\"hidden\" name=\"Password\" value=\"");
    m_pHttpResponse->WriteText (i64PasswordHash);
    OutputText ("\"><input type=\"hidden\" name=\"Salt\" value=\"");
    m_pHttpResponse->WriteText (m_tNewSalt);
    OutputText ("\"><input type=\"hidden\" name=\"GameClass\" value=\"");
    m_pHttpResponse->WriteText (m_iGameClass);
    OutputText ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"");
    m_pHttpResponse->WriteText (m_iGameNumber);
    OutputText ("\"><input type=\"hidden\" name=\"Updates\" value=\"");
    m_pHttpResponse->WriteText (m_iNumNewUpdates);
    OutputText ("\"><input type=\"hidden\" name=\"Auto\" value=\"0\">");

    return OK;
}

void HtmlRenderer::CloseGamePage()
{
    OutputText ("<p>");
    WriteSeparatorString (m_iSeparatorKey);
    OutputText ("<p><strong><font size=\"3\">");
    
    if (m_bRepeatedButtons) {
        WriteGameButtons();
        OutputText ("<p>");
    }
    
    WriteContactLine();
    
    WriteButton (BID_SERVERNEWS);
    WriteButton (BID_SERVERINFORMATION);
    WriteButton (BID_DOCUMENTATION);

    OutputText ("<br>");

    WriteButton (BID_CONTRIBUTIONS);
    WriteButton (BID_CREDITS);
    WriteButton (BID_TOS);

    MilliSeconds msTime = GetTimerCount();
    
    OnPageRender (msTime);
    
    OutputText ("<p>");
    WriteVersionString();
    OutputText ("<br>Script time: ");
    m_pHttpResponse->WriteText ((int) msTime);
    OutputText (" ms</font></strong></center></form></body></html>");
}

int HtmlRenderer::InitializeGame (PageId* ppageRedirect) {
    
    int iErrCode;
    bool bFlag;
    
    PageId pgSrcPageId;

    if (m_iPrivilege <= GUEST) {
        return ERROR_ACCESS_DENIED;
    }

    IHttpForm* pHttpForm = m_pHttpRequest->GetForm("PageId");
    pgSrcPageId = (pHttpForm != NULL) ? (PageId) pHttpForm->GetIntValue() : LOGIN;

    // If an auto-submission, get game class and number from forms
    if (pgSrcPageId == m_pgPageId) {
        
        // Get game class
        if (m_iGameClass == NO_KEY)
        {
            AddMessage ("Missing GameClass form");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }
        
        // Get game number
        if (m_iGameNumber == -1)
        {
            AddMessage ("Missing GameNumber form");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }
        
        // Get old update count
        if ((pHttpForm = m_pHttpRequest->GetForm ("Updates")) == NULL) {
            AddMessage ("Missing Updates form");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }
        m_iNumOldUpdates = pHttpForm->GetIntValue();    
    }

    // Verify empire's presence in game
    iErrCode = IsEmpireInGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
    if (iErrCode != OK || !bFlag) {

        if (iErrCode == ERROR_GAME_DOES_NOT_EXIST) {
            AddMessage ("That game no longer exists");
        } else {
            AddMessage ("You are no longer in that game");
        }
        *ppageRedirect = ACTIVE_GAME_LIST;
        return ERROR_FAILURE;
    }
    
    // Set some variables if we're coming from a non-game page
    if (pgSrcPageId == m_pgPageId || (pgSrcPageId != m_pgPageId && !IsGamePage (pgSrcPageId))) {

        Variant vTemp;

        // Get game options
        iErrCode = GetEmpireOptions (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_iGameOptions);
        if (iErrCode != OK) {
            AddMessage ("That empire no longer exists");
            *ppageRedirect = LOGIN;
            return ERROR_FAILURE;
        }

        // Get gameclass name
        iErrCode = GetGameClassName (m_iGameClass, m_pszGameClassName);
        if (iErrCode != OK) {
            AddMessage ("That game no longer exists");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }

        // Set some flags
        m_bRepeatedButtons = (m_iGameOptions & GAME_REPEATED_BUTTONS) != 0;
        m_bTimeDisplay = (m_iGameOptions & GAME_DISPLAY_TIME) != 0;

        // Get game ratios
        iErrCode = GetEmpireGameProperty (
            m_iGameClass,
            m_iGameNumber,
            m_iEmpireKey,
            GameEmpireData::GameRatios,
            &vTemp
            );

        if (iErrCode != OK) {
            AddMessage ("That game no longer exists");
            *ppageRedirect = LOGIN;
            return ERROR_FAILURE;
        }

        m_iGameRatios = vTemp.GetInteger();
    }
    
    ///////////////////////
    // Check for updates //
    ///////////////////////
    
    bool bUpdate;
    if (CheckGameForUpdates (m_iGameClass, m_iGameNumber, false, &bUpdate) != OK)
    {
        // Remove update message after update
        if (bUpdate && m_strMessage.Equals ("You are now ready for an update")) {
            m_strMessage.Clear();
        }
        
        AddMessage ("The game ended");
        *ppageRedirect = ACTIVE_GAME_LIST;
        return ERROR_FAILURE;
    }
    
    // Re-verify empire's presence in game
    iErrCode = IsEmpireInGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
    if (iErrCode != OK || !bFlag) {

        AddMessage ("You are no longer in that game");
        *ppageRedirect = ACTIVE_GAME_LIST;
        return ERROR_FAILURE;
    }
    
    // Verify not resigned
    iErrCode = HasEmpireResignedFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
    if (iErrCode != OK || bFlag) {
        
        AddMessage ("Your empire has resigned from that game");
        *ppageRedirect = ACTIVE_GAME_LIST;
        return ERROR_FAILURE;
    }
    
    // Log empire into game if not an auto submission
    IHttpForm* pHttpAuto = m_pHttpRequest->GetForm ("Auto");
    if (pHttpAuto == NULL || pHttpAuto->GetIntValue() == 0 && !m_bLoggedIntoGame) {
        
        int iNumUpdatesIdle;
        iErrCode = LogEmpireIntoGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumUpdatesIdle);       
        if (iErrCode != OK) {
            AddMessage ("Your empire could not be logged into the game");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }

        m_bLoggedIntoGame = true;

        if (iNumUpdatesIdle > 0 && !WasButtonPressed (BID_EXIT)) {

            // Check for all empires idle case
            bool bIdle;
            iErrCode = AreAllEmpiresIdle (m_iGameClass, m_iGameNumber, &bIdle);
            if (iErrCode != OK) {
                AddMessage ("That game no longer exists");
                *ppageRedirect = ACTIVE_GAME_LIST;
                return ERROR_FAILURE;
            }

            if (bIdle) {

                AddMessage ("All empires in the game are idle this update, including yours");
                AddMessage ("Pausing, drawing and ending turn will not take effect until the next update");

                if (m_iGameOptions & REQUEST_DRAW) {

                    IHttpForm* pHttpForm;
                    if (m_pgPageId != OPTIONS ||
                        (pHttpForm = m_pHttpRequest->GetForm ("Draw")) == NULL ||
                        pHttpForm->GetIntValue() != 0
                        ) {

                        AddMessage ("Your empire is requesting draw, so the game may end in a draw next update");
                    }
                }
            }
        }
    }
    
    // Remove update message after update
    if (bUpdate && m_strMessage.Equals ("You are now ready for an update")) {
        m_strMessage.Clear();
    }
    
    // Get game update information
    if (GetGameUpdateData (
        m_iGameClass, 
        m_iGameNumber, 
        &m_sSecondsSince, 
        &m_sSecondsUntil, 
        &m_iNumNewUpdates, 
        &m_iGameState
        ) != OK
        ) {
        
        Assert (false);

        AddMessage ("The game no longer exists");
        *ppageRedirect = ACTIVE_GAME_LIST;
        return ERROR_FAILURE;
    }

    // Hack for when games update
    if (bUpdate) {
        m_iGameOptions &= ~UPDATED;
    }
    
    return OK;
}

int HtmlRenderer::RedirectOnSubmit(PageId* ppageRedirect, bool* pbRedirected)
{
    if (!(m_iSystemOptions2 & EMPIRE_ACCEPTED_TOS)) {

        if (m_bRedirection && m_pgPageId == SYSTEM_TERMS_OF_SERVICE) {
            *pbRedirected = false;
            return OK;
        }

        if (m_pgPageId != SYSTEM_TERMS_OF_SERVICE) {
            *ppageRedirect = SYSTEM_TERMS_OF_SERVICE;
            *pbRedirected = true;
            return OK;
        }
        *pbRedirected = false;
        return OK;
    }

    if (m_bRedirection || WasButtonPressed (PageButtonId[m_pgPageId])) {
        *pbRedirected = false;
        return OK;
    }

    if (WasButtonPressed (BID_ACTIVEGAMELIST)) {
        *ppageRedirect = ACTIVE_GAME_LIST;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_OPENGAMELIST)) {
        *ppageRedirect = OPEN_GAME_LIST;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_SYSTEMGAMELIST)) {
        *ppageRedirect = SYSTEM_GAME_LIST;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_PROFILEVIEWER)) {
        *ppageRedirect = PROFILE_VIEWER;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_PROFILEEDITOR)) {
        *ppageRedirect = PROFILE_EDITOR;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_TOPLISTS)) {
        *ppageRedirect = TOP_LISTS;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_SPECTATORGAMES)) {
        *ppageRedirect = SPECTATOR_GAMES;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_TOURNAMENTS)) {
        m_iReserved = NO_KEY;
        *ppageRedirect = TOURNAMENTS;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_LATESTGAMES)) {
        *ppageRedirect = LATEST_GAMES;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_LATESTNUKES)) {
        *ppageRedirect = LATEST_NUKES;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_CHATROOM)) {
        *ppageRedirect = CHATROOM;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_EXIT)) {
        *ppageRedirect = LOGIN;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_PERSONALGAMECLASSES)) {
        *ppageRedirect = PERSONAL_GAME_CLASSES;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_PERSONALTOURNAMENTS)) {
        *ppageRedirect = PERSONAL_TOURNAMENTS;
        *pbRedirected = true;
        return OK;
    }

    if (m_iPrivilege >= ADMINISTRATOR) {

        if (WasButtonPressed (BID_SERVERADMINISTRATOR)) {
            *ppageRedirect = SERVER_ADMINISTRATOR;
            *pbRedirected = true;
            return OK;
        }

        if (WasButtonPressed (BID_GAMEADMINISTRATOR)) {
            *ppageRedirect = GAME_ADMINISTRATOR;
            *pbRedirected = true;
            return OK;
        }

        if (WasButtonPressed (BID_EMPIREADMINISTRATOR)) {
            *ppageRedirect = EMPIRE_ADMINISTRATOR;
            *pbRedirected = true;
            return OK;
        }

        if (WasButtonPressed (BID_THEMEADMINISTRATOR)) {
            *ppageRedirect = THEME_ADMINISTRATOR;
            *pbRedirected = true;
            return OK;
        }

        if (WasButtonPressed (BID_TOURNAMENTADMINISTRATOR)) {
            *ppageRedirect = TOURNAMENT_ADMINISTRATOR;
            *pbRedirected = true;
            return OK;
        }
    }

    if (WasButtonPressed (BID_SERVERINFORMATION)) {
        *ppageRedirect = SYSTEM_SERVER_INFORMATION;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_DOCUMENTATION)) {
        *ppageRedirect = SYSTEM_DOCUMENTATION;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_SERVERNEWS)) {
        *ppageRedirect = SYSTEM_NEWS;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_CONTRIBUTIONS)) {
        *ppageRedirect = SYSTEM_CONTRIBUTIONS;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_CREDITS)) {
        *ppageRedirect = SYSTEM_CREDITS;
        *pbRedirected = true;
        return OK;
    }

    if (WasButtonPressed (BID_TOS)) {
        *ppageRedirect = SYSTEM_TERMS_OF_SERVICE;
        *pbRedirected = true;
        return OK;
    }

    IHttpForm* pHttpForm = m_pHttpRequest->GetForm ("ProfileLink.x");
    if (pHttpForm != NULL)
    {
        m_iReserved = m_iEmpireKey;
        *ppageRedirect = PROFILE_VIEWER;
        *pbRedirected = true;
        return OK;
    }

    if (NotifiedProfileLink()) {

        pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ProfileLink");
        if (pHttpForm != NULL)
        {
            const char* pszProfile = pHttpForm->GetName();
            Assert (pszProfile != NULL);

            int iViewProfileEmpireKey;
            unsigned int iHash;

            if (sscanf(pszProfile, "ProfileLink.%d.%d.x", &iViewProfileEmpireKey, &iHash) == 2)
            {
                unsigned int iResults;
                int iErrCode = CacheEmpire(iViewProfileEmpireKey, &iResults);
                if (iErrCode != OK)
                    return iErrCode;

                if (iResults == 0 || !VerifyEmpireNameHash(iViewProfileEmpireKey, iHash))
                {
                    AddMessage("That empire no longer exists");
                    *pbRedirected = false;
                    return OK;
                }

                m_iReserved = iViewProfileEmpireKey;
                *ppageRedirect = PROFILE_VIEWER;
                *pbRedirected = true;
                return OK;
            }
        }
    }

    if (NotifiedTournamentInvitation()) {

        int iErrCode, iMessageKey;
        unsigned int iTournamentKey;

        pHttpForm = m_pHttpRequest->GetForm ("HintTournamentInvite");
        if (pHttpForm != NULL && sscanf (pHttpForm->GetValue(), "%i.%i", &iMessageKey, &iTournamentKey) == 2)
        {
            if (WasButtonPressed (BID_VIEWTOURNAMENTINFORMATION)) {
                m_iReserved = iTournamentKey;
                *ppageRedirect = TOURNAMENTS;
                *pbRedirected = true;
                return OK;
            }

            iErrCode = CacheTournamentTables(&iTournamentKey, 1);
            if (iErrCode != OK)
                return iErrCode;

            bool bAccept = WasButtonPressed (BID_ACCEPT);
            bool bDecline = bAccept ? false : WasButtonPressed (BID_DECLINE);

            if (bAccept || bDecline) {

                iErrCode = RespondToTournamentInvitation(m_iEmpireKey, iMessageKey, bAccept);
                if (iErrCode == OK)
                {
                    if (bAccept)
                    {
                        AddMessage ("You have joined the tournament");
                    }
                    else
                    {
                        AddMessage ("You have declined to join the tournament");
                    }
                }
                else
                {
                    if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT)
                    {
                        AddMessage ("You are already in the tournament");
                    }
                    else if (iErrCode == ERROR_MESSAGE_DOES_NOT_EXIST)
                    {
                        AddMessage ("You have already responded to that message");
                    }
                    else return iErrCode;
                }
            }
        }
    }

    if (NotifiedTournamentJoinRequest()) {

        int iErrCode, iMessageKey;
        unsigned int iTournamentKey;

        pHttpForm = m_pHttpRequest->GetForm("HintTournamentJoinRequest");
        if (pHttpForm != NULL && sscanf (pHttpForm->GetValue(), "%i.%i", &iMessageKey, &iTournamentKey) == 2) {

            if (WasButtonPressed (BID_VIEWTOURNAMENTINFORMATION))
            {
                m_iReserved = iTournamentKey;
                *ppageRedirect = TOURNAMENTS;
                *pbRedirected = true;
                return OK;
            }

            bool bAccept = WasButtonPressed (BID_ACCEPT);
            bool bDecline = bAccept ? false : WasButtonPressed (BID_DECLINE);

            if (bAccept || bDecline)
            {
                iErrCode = CacheTournamentTables(&iTournamentKey, 1);
                if (iErrCode != OK)
                    return iErrCode;

                iErrCode = RespondToTournamentJoinRequest(m_iEmpireKey, iMessageKey, bAccept);
                if (iErrCode == OK)
                {
                    if (bAccept)
                    {
                        AddMessage ("You have accepted the empire into the tournament");
                    }
                    else
                    {
                        AddMessage ("You have declined to accept the empire into the tournament");
                    }
                }
                else
                {
                    if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT)
                    {
                        AddMessage ("The empire is already in the tournament");
                    }
                    else if (iErrCode == ERROR_MESSAGE_DOES_NOT_EXIST)
                    {
                        AddMessage ("You have already responded to that message");
                    }
                    else return iErrCode;
                }
            }
        }
    }

    *pbRedirected = false;
    return OK;
}

void HtmlRenderer::CloseSystemPage() {
    
    String strFilter;
    
    int iButtonKey = m_iButtonKey, iPrivilege = m_iPrivilege;
    
    OutputText ("<p>");
    WriteSeparatorString (m_iSeparatorKey);
    OutputText ("<p>");
    
    if (m_bRepeatedButtons) {
        WriteSystemButtons (iButtonKey, iPrivilege);
    }
    
    WriteContactLine();
    
    if (m_pgPageId != LOGIN && m_pgPageId != NEW_EMPIRE) {
        
        OutputText ("<p>");
        
        WriteButton (BID_SERVERNEWS);
        WriteButton (BID_SERVERINFORMATION);
        WriteButton (BID_DOCUMENTATION);

        OutputText ("<br>");

        WriteButton (BID_CONTRIBUTIONS);
        WriteButton (BID_CREDITS);
        WriteButton (BID_TOS);
    }
    
    OutputText("<p><strong><font size=\"3\">");
    WriteVersionString();
    
    OutputText("<br>Script time: ");
    MilliSeconds msTime = GetTimerCount();
    m_pHttpResponse->WriteText((int) msTime);
    OutputText(" ms</font></strong>");

    OutputText("</center></form></body></html>");
    OnPageRender(msTime);
}

int HtmlRenderer::RedirectOnSubmitGame(PageId* ppageRedirect, bool* pbRedirected)
{
    int iErrCode;
    bool bFlag;
    
    IHttpForm* pHttpForm;

    Assert (m_pgPageId > MIN_PAGE_ID && m_pgPageId < MAX_PAGE_ID);
    
    /*if (WasButtonPressed (PageButtonId[m_pgPageId])) {
        goto False;
    }*/

    if (!(m_iSystemOptions2 & EMPIRE_ACCEPTED_TOS)) {

        if (m_bRedirection && m_pgPageId == SYSTEM_TERMS_OF_SERVICE) {
            goto False;
        }

        if (m_pgPageId != SYSTEM_TERMS_OF_SERVICE) {
            *ppageRedirect = SYSTEM_TERMS_OF_SERVICE;
            goto True;
        }
        goto False;
    }

    if (m_bRedirection) {
        goto False;
    }

    if (WasButtonPressed (BID_INFO)) {
        *ppageRedirect = INFO;
        goto True;
    }
    
    if (WasButtonPressed (BID_BUILD)) {
        *ppageRedirect = BUILD;
        goto True;
    }
    
    if (WasButtonPressed (BID_TECH)) {
        *ppageRedirect = TECH;
        goto True;
    }
    
    if (WasButtonPressed (BID_OPTIONS)) {
        *ppageRedirect = OPTIONS;
        goto True;
    }
    
    if (WasButtonPressed (BID_SHIPS)) {
        *ppageRedirect = SHIPS;
        goto True;
    }
    
    if (WasButtonPressed (BID_PLANETS)) {
        *ppageRedirect = PLANETS;
        goto True;
    }
    
    if (WasButtonPressed (BID_MAP)) {
        *ppageRedirect = MAP;
        goto True;
    }
    
    if (WasButtonPressed (BID_DIPLOMACY)) {
        *ppageRedirect = DIPLOMACY;
        goto True;
    }
    
    if (WasButtonPressed (BID_ENDTURN)) {

        if (m_iNumNewUpdates == m_iNumOldUpdates) {
        
            // End turn
            iErrCode = SetEmpireReadyForUpdate(m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
            if (iErrCode != OK)
                return iErrCode;
            
            // Redirect to same page
            if (bFlag)
            {
                AddMessage ("You are now ready for an update");
            }
            *ppageRedirect = m_pgPageId;
            goto True;
        
        }
        else
        {
            // Tell empire that his end turn didn't work
            AddMessage ("Your end turn was too late");
            goto False;
        }
    }
    
    if (WasButtonPressed (BID_UNENDTURN))
    {
        if (m_iNumNewUpdates == m_iNumOldUpdates)
        {
            // Unend turn
            iErrCode = SetEmpireNotReadyForUpdate(m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
            if (iErrCode != OK)
                return iErrCode;

            // Redirect to same page
            if (bFlag)
            {
                AddMessage ("You are no longer ready for an update");
            }
            *ppageRedirect = m_pgPageId;
            goto True;

        } else {

            // Tell empire that his end turn didn't work
            AddMessage ("Your unend turn was too late");
            goto False;
        }
    }
    
    if (WasButtonPressed (BID_EXIT)) {
        *ppageRedirect = ACTIVE_GAME_LIST;
        goto True;
    }
    
    if (WasButtonPressed (BID_QUIT)) {
        m_iReserved = BID_QUIT;
        *ppageRedirect = QUIT;
        goto True;
    }
    
    if (WasButtonPressed (BID_SERVERINFORMATION)) {
        *ppageRedirect = GAME_SERVER_INFORMATION;
        goto True;
    }
    
    if (WasButtonPressed (BID_DOCUMENTATION)) {
        *ppageRedirect = GAME_DOCUMENTATION;
        goto True;
    }
    
    if (WasButtonPressed (BID_SERVERNEWS)) {
        *ppageRedirect = GAME_NEWS;
        goto True;
    }

    if (WasButtonPressed (BID_CONTRIBUTIONS)) {
        *ppageRedirect = GAME_CONTRIBUTIONS;
        goto True;
    }
    
    if (WasButtonPressed (BID_CREDITS)) {
        *ppageRedirect = GAME_CREDITS;
        goto True;
    }

    if (WasButtonPressed (BID_TOS)) {
        *ppageRedirect = GAME_TERMS_OF_SERVICE;
        goto True;
    }
    
    pHttpForm = m_pHttpRequest->GetForm ("ProfileLink.x");
    if (pHttpForm != NULL)
    {
        m_iReserved = m_iEmpireKey;
        *ppageRedirect = GAME_PROFILE_VIEWER;
        goto True;
    }
    
    if (NotifiedProfileLink()) {

        pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ProfileLink");
        if (pHttpForm != NULL)
        {
            const char* pszProfile = pHttpForm->GetName();
            Assert (pszProfile != NULL);

            int iViewProfileEmpireKey;
            unsigned int iHash;

            if (sscanf(pszProfile, "ProfileLink.%d.%d.x", &iViewProfileEmpireKey, &iHash) == 2)
            {
                unsigned int iResults;
                int iErrCode = CacheEmpire(iViewProfileEmpireKey, &iResults);
                if (iErrCode != OK)
                    return iErrCode;

                if (iResults == 0 || !VerifyEmpireNameHash(iViewProfileEmpireKey, iHash))
                {
                    AddMessage("That empire no longer exists");
                    goto False;
                }

                m_iReserved = iViewProfileEmpireKey;
                *ppageRedirect = GAME_PROFILE_VIEWER;
                goto True;
            }
        }
    }

False:

    *pbRedirected = false;
    return OK;
    
True:
    
    *pbRedirected = true;
    return OK;
}