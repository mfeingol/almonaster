//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <stdio.h>
#include <math.h>

#include "Osal/File.h"
#include "Osal/TempFile.h"

#include "HtmlRenderer.h"
#include "Global.h"

#include "../MapGen/MapFairnessEvaluator.h"

/*

  <script language="JavaScript">
  <!--
  function Open(text){
  var newWindow;
  newWindow=open("", "SensitiveWindow","toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes,resizable=yes,copyhistory=no,width=600,height=180");
  newWindow.document.writeln (text);
  newWindow.document.close();
  }
  -->
  </script>
      
        onMouseOver="Open ('Wombat: 3 Sciences, 2 Attacks<p>Beluga: 1 Science, 3 Troopships');"
        
*/

//
// Static members
//

bool HtmlRenderer::ms_bLocksInitialized = false;

ReadWriteLock HtmlRenderer::ms_mTextFileLock;

AlmonasterStatistics HtmlRenderer::m_sStats;

UTCTime HtmlRenderer::m_stEmpiresInGamesCheck;
UTCTime HtmlRenderer::m_stServerNewsLastUpdate;

Mutex HtmlRenderer::m_slockEmpiresInGames;
unsigned int HtmlRenderer::m_siNumGamingEmpires;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HtmlRenderer::HtmlRenderer(PageId pgPageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse)
{
    m_pgPageId = pgPageId;
    m_pHttpRequest = pHttpRequest;
    m_pHttpResponse = pHttpResponse;

    m_bRedirectTest = true;
   
    m_vEmpireName = (const char*) NULL;
    m_vPreviousIPAddress = (const char*) NULL;

    m_pszGameClassName[0] = '\0';
    
    m_iEmpireKey = NO_KEY;
    m_iGameClass = NO_KEY;
    m_iGameNumber = -1;
    m_iTournamentKey = NO_KEY;
    m_iButtonKey = NO_KEY;
    m_iButtonAddress = -1;
    m_iBackgroundKey = NO_KEY;
    m_iBackgroundAddress = -1;
    m_iSeparatorKey = NO_KEY;
    m_iPrivilege = NOVICE;
    m_iAlienKey = NO_KEY;
    m_iAlienAddress = -1;
    m_iThemeKey = NO_KEY;
    m_iGameState = 0;
    m_iGameRatios = RATIOS_DISPLAY_NEVER;
    m_iReserved = NO_KEY;

    m_iSystemOptions = 0;
    m_iSystemOptions2 = 0;
    m_iGameOptions = 0;
    
    m_bRedirection = false;
    m_bRepeatedButtons = false;
    m_bTimeDisplay = false;
    m_bOwnPost = false;
    m_bLoggedIntoGame = false;
    m_bAuthenticated = false;
    m_bAutoLogon = false;
    
    m_sSecondsUntil = 0;
    m_sSecondsSince = 0;
    Time::StartTimer (&m_tmTimer);

    m_bNotifiedProfileLink = false;
    m_bNotifiedTournamentInvitation = false;
    m_bNotifiedTournamentJoinRequest = false;
    
    m_i64SessionId = NO_SESSION_ID;

    m_iNumOldUpdates = 0;

    // Bit of a hack, but it works and saves lots of time and space
    m_systemEmpireCol.Name = SystemEmpireActiveGames::EmpireKey;
    m_systemEmpireCol.Data = NO_KEY;

    m_systemTournamentCol.Name = SystemEmpireTournaments::TournamentKey;
    m_systemTournamentCol.Data = NO_KEY;

    m_gameCols[0].Name = GameData::GameClass;
    m_gameCols[0].Data = NO_KEY;
    m_gameCols[1].Name = GameData::GameNumber;
    m_gameCols[1].Data = -1;

    m_gameEmpireCols[0] = m_gameCols[0];
    m_gameEmpireCols[1] = m_gameCols[1];
    m_gameEmpireCols[2].Name = GameEmpireData::EmpireKey;
    m_gameEmpireCols[2].Data = NO_KEY;
}

int HtmlRenderer::StaticInitialize()
{
    int iErrCode = OK;

    if (!ms_bLocksInitialized) {

        iErrCode = ms_mTextFileLock.Initialize();
        Assert(iErrCode == OK);

        iErrCode = m_slockEmpiresInGames.Initialize();
        Assert(iErrCode == OK);

        ms_bLocksInitialized = true;
    }

    m_stEmpiresInGamesCheck = NULL_TIME;
    m_siNumGamingEmpires = 0;

    char pszFileName[OS::MaxFileNameLength];
    sprintf(pszFileName, "%s/" NEWS_FILE, global.GetResourceDir());

    if (File::DoesFileExist(pszFileName))
    {
        iErrCode = File::GetLastModifiedTime (pszFileName, &m_stServerNewsLastUpdate);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

void HtmlRenderer::ShutdownServer() {
    
    global.GetHttpServer()->Shutdown();
    AddMessage ("The server will shut down now");
}

void HtmlRenderer::RestartServer() {
    
    global.GetHttpServer()->Restart(NULL);
    AddMessage ("The server will now restart");
}

void HtmlRenderer::RestartAlmonaster() {
    
    global.GetPageSourceControl()->Restart();
    AddMessage ("Almonaster will now restart");
}

bool HtmlRenderer::IsGamePage (PageId pageId) {
    
    return (pageId >= INFO && pageId <= QUIT);
}

bool HtmlRenderer::ShouldDisplayGameRatios()
{
    switch (g_pageGameRatioSetting[m_pgPageId])
    {
    case RATIOS_DISPLAY_NEVER:
        // If the page says 'never', we never display ratios
        return false;
    case RATIOS_DISPLAY_ON_RELEVANT_SCREENS:
        // If the page says 'relevant', we display if the user selected relevant or always
        return m_iGameRatios >= RATIOS_DISPLAY_ON_RELEVANT_SCREENS;
    case RATIOS_DISPLAY_ALWAYS:
        // If the page says 'always', we display if the user selected always
        return m_iGameRatios >= RATIOS_DISPLAY_ALWAYS;
    default:
        Assert(false);
        return false;
    }
}

bool HtmlRenderer::StandardizeEmpireName (const char* pszName, char pszFinalName[MAX_EMPIRE_NAME_LENGTH + 1])
{
    char pszCopy [MAX_EMPIRE_NAME_LENGTH + 1];
    strcpy (pszCopy, pszName);
    
    char* pszNameCopy = pszCopy;
    
    // Remove beginning spaces
    size_t i = 0, iLen = strlen (pszNameCopy);
    while (i < iLen && pszNameCopy[i] == ' ') {
        pszNameCopy ++;
        iLen --;
    }
    
    if (i == iLen)
    {
        AddMessage ("An empire name must contain more than just spaces");
        return false;
    }
    
    // Remove trailing spaces
    i = iLen - 1;
    while (i != -1 && pszNameCopy[i] == ' ') {
        pszNameCopy[i] = '\0';
        i --;
        iLen --;
    }
    
    // Remove intermediate spaces
    if (iLen > 1) {
        i = iLen - 1;
        while (i != 0) {
            if (pszNameCopy[i] == ' ' && pszNameCopy[i - 1] == ' ') {
                strcpy (pszNameCopy + i - 1, pszNameCopy + i);
                iLen --;
            }
            i --;
        }
    }
    
    // Make sure there's something left
    if (*pszNameCopy == '\0') {
        AddMessage ("An empire name must contain more than just spaces");
        return false;
    }
    
    // Name is valid
    strcpy (pszFinalName, pszNameCopy);
    
    return true;
}

bool HtmlRenderer::VerifyPassword (const char* pszPassword, bool bPrintErrors) {
    
    if (pszPassword == NULL || *pszPassword == '\0')
    {
        if (bPrintErrors)
        {
            AddMessage ("Passwords cannot be blank");
        }
        return false;
    }
    
    size_t i, stLength = strlen(pszPassword);
    if (stLength > MAX_EMPIRE_PASSWORD_LENGTH)
    {
        if (bPrintErrors)
        {
            char pszText [256];
            sprintf(pszText, "Passwords cannot be longer than %i characters", MAX_EMPIRE_PASSWORD_LENGTH);
            AddMessage (pszText);
        }
        return false;
    }
    
    // Make sure the characters are permitted
    for (i = 0; i < stLength; i ++)
    {
        if (pszPassword[i] < FIRST_VALID_PASSWORD_CHAR || pszPassword[i] > LAST_VALID_PASSWORD_CHAR)
        {
            if (bPrintErrors)
            {
                AddMessage ("The password contains an invalid character");
            }
            return false;
        }
    }
    
    // We're ok
    return true;
}

bool HtmlRenderer::VerifyEmpireName (const char* pszEmpireName, bool bPrintErrors)
{
    char c;
    
    if (pszEmpireName == NULL || *pszEmpireName == '\0') {
        if (bPrintErrors) {
            AddMessage ("Empire names cannot be blank");
        }
        return false;
    }
    
    size_t i, stLength = strlen (pszEmpireName);
    if (stLength > MAX_EMPIRE_NAME_LENGTH) {
        
        if (bPrintErrors) {
            
            char pszText [128];
            sprintf(pszText, "Empire names cannot be longer than %i characters", MAX_EMPIRE_NAME_LENGTH );
            
            AddMessage (pszText);
        }
        return false;
    }
    
    // Make sure the characters are permitted
    for (i = 0; i < stLength; i ++) {
        
        c = pszEmpireName[i];
        
        if (c < FIRST_VALID_EMPIRE_NAME_CHAR || 
            c > LAST_VALID_EMPIRE_NAME_CHAR ||
            c == '\"' ||
            c == ',' ||
            c == '>' ||
            c == '<' ||
            c == '(' ||
            c == ')' ||
            c == '&' ||
            c == '#'
            ) {
            
            if (bPrintErrors) {
                AddMessage ("The empire name contains an invalid character");
            }
            return false;
        }
    }
    
    // We're ok
    return true;
}

bool HtmlRenderer::VerifyCategoryName(const char* pszCategory, const char* pszName, size_t stMaxLen, bool bPrintErrors)
{
    char c;
    
    if (String::IsWhiteSpace (pszName)) {
        if (bPrintErrors) {
            AddMessage (pszCategory);
            AppendMessage (" names cannot be blank");
        }
        return false;
    }
    
    size_t i, stLength = strlen (pszName);
    if (stLength > stMaxLen) {
        
        if (bPrintErrors) {
            AddMessage (pszCategory);
            AppendMessage (" names cannot be longer than ");
            AppendMessage ((int) stMaxLen);
            AppendMessage (" characters");
        }
        
        return false;
    }
    
    // Make sure the characters are permitted
    for (i = 0; i < stLength; i ++) {
        
        c = pszName[i];
        
        if (c < FIRST_VALID_CATEGORY_NAME_CHAR || 
            c > LAST_VALID_CATEGORY_NAME_CHAR ||
            c == '\\' ||
            c == '/' ||
            c == ':' ||
            c == '*' ||
            c == '?' ||
            c == '\"' ||
            c == '<' ||
            c == '>' ||
            c == '|'
            ) {
            
            if (bPrintErrors) {
                AddMessage ("The provided name contains an invalid character");
            }
            return false;
        }
    }
    
    // We're ok
    return true;
}

int HtmlRenderer::WriteVersionString() {
    
    m_pHttpResponse->WriteText (GetSystemVersion());
    OutputText (" @ ");
    
    Variant vServerName;
    int iErrCode = GetSystemProperty (SystemData::ServerName, &vServerName);
    RETURN_ON_ERROR(iErrCode);
        
    m_pHttpResponse->WriteText (vServerName.GetCharPtr());

    return iErrCode;
}

void HtmlRenderer::WriteBodyString (Seconds iSecondsUntil) {

    bool bAutoRefresh = (m_iGameOptions & AUTO_REFRESH) && iSecondsUntil >= 0;

    if (!bAutoRefresh && 
        IsGamePage (m_pgPageId) && 
        !(m_iGameState & STARTED) &&
        (m_iSystemOptions2 & REFRESH_UNSTARTED_GAME_PAGES)
        ) {
        
        bAutoRefresh = true;
        iSecondsUntil = UNSTARTED_GAMEPAGE_REFRESH_SEC;
    }
    
    OutputText ("<body bgcolor=\"#" DEFAULT_BG_COLOR "\"");
    
    switch (m_iBackgroundKey) {
        
    case NULL_THEME:
        break;
        
    case ALTERNATIVE_PATH:
        OutputText (" background=\"");
        m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
        OutputText ("/" BACKGROUND_IMAGE "\"");
        break;
        
    default:
        OutputText (" background=\"" BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText(m_iBackgroundAddress);
        OutputText ("/" BACKGROUND_IMAGE "\"");
        break;
    }
    
    if (m_iSystemOptions & FIXED_BACKGROUNDS) {
        OutputText (" bgproperties=\"fixed\"");
    }
    
    OutputText (" text=\"#");
    m_pHttpResponse->WriteText (m_vTextColor.GetCharPtr());
    
    OutputText ("\" link=\"#" DEFAULT_LINK_COLOR "\" alink=\"#" DEFAULT_ALINK_COLOR "\" vlink=\"#" DEFAULT_VLINK_COLOR "\"");
    
    if (bAutoRefresh) {
        
        OutputText (" onLoad=\"setTimeout ('AutoRefresh()', ");
        m_pHttpResponse->WriteText (iSecondsUntil * 1000);
        OutputText (")\"><script><!--\n"\
            "function AutoRefresh() {\n"\
            "document.forms[0].Auto.value=\"1\";\n"\
            "document.forms[0].submit();\n"\
            "} //--></script>"
            );
        
    } else {
        
        OutputText (">");
    }
}

int HtmlRenderer::WriteContactLine()
{
    OutputText ("<p><strong>Contact the ");

    Variant vEmail;
    String strFilter;
    int iErrCode = GetSystemProperty(SystemData::AdminEmail, &vEmail);
    RETURN_ON_ERROR(iErrCode);

    if (!String::IsBlank(vEmail.GetCharPtr()))
    {
        HTMLFilter(vEmail.GetCharPtr(), &strFilter, 0, false);
        
        size_t i;
        const size_t cLength = strFilter.GetLength();

        // Use XOR 'encryption' to hide the admin's email address from spam harvesters.
        // We do this only on the login screen which can be accessed without authentication

        if (m_pgPageId == LOGIN) {

            const char* pszPlainText = strFilter.GetCharPtr();
            char* pszKey = (char*)StackAlloc(cLength + 1);
            char* pszCypherText = (char*)StackAlloc(cLength + 1);
            
            for (i = 0; i < cLength; i ++) {
                pszKey[i] = Algorithm::GetRandomASCIIChar();
            }
            pszKey[i] = '\0';

            for (i = 0; i < cLength; i ++) {
                pszCypherText[i] = (char) pszPlainText[i] ^ pszKey[i];
            }
            pszCypherText[i] = '\0';

            OutputText("<script type='text/javascript'><!--\n");

            OutputText("var key=\"");
            m_pHttpResponse->WriteText(pszKey);
            OutputText("\";");

            OutputText("var txt=new Array(");
            for (i = 0; i < cLength; i ++) {
                m_pHttpResponse->WriteText((int)pszCypherText[i]);
                if (i < cLength - 1)
                    OutputText(",");
            }
            OutputText("); var e=\"\"; for(var i=0;i<");

            m_pHttpResponse->WriteText((int)cLength);

            OutputText(
                ";i++){" \
                "e+=String.fromCharCode(key.charCodeAt(i)^txt[i]);}" \
                "document.write('<a href=\"javascript:void(0)\" " \
                "onclick=\"window.location=\\'m\\u0061il\\u0074o\\u003a'+e+'?subject=Almonaster'+'\\'\">'" \
                "+'administrator<\\/a>');" \
                "\n--></script><noscript>administrator</noscript>"
                );
        } else {

            OutputText("<a href=\"mailto:");
            m_pHttpResponse->WriteText(strFilter.GetCharPtr());
            OutputText("?subject=Almonaster\">administrator</a>");
        }

    } else {

        OutputText("administrator");
    }
        
    OutputText(" if you have problems or suggestions</strong><p>");

    return iErrCode;
}

void HtmlRenderer::OpenForm() {
    
    OutputText ("<form method=\"post\"><input type=\"hidden\" name=\"PageId\" value=\"");
    m_pHttpResponse->WriteText ((int) m_pgPageId);
    OutputText ("\">");
}


#define MSG_WELCOME "Welcome to Almonaster, "
#define MSG_NEWTOTHEGAME \
    ".<br>If you are new to the game, you should read the documentation before you begin to play." \
    "<br>You can use the Profile Editor to change the way your empire looks to others"

void HtmlRenderer::SendWelcomeMessage (const char* pszEmpireName) {

    const size_t stLen = strlen (pszEmpireName);
    const size_t stLen2 = sizeof (MSG_WELCOME) - 1;
    const size_t stLen3 = sizeof (MSG_NEWTOTHEGAME);

    char* pszBuffer = (char*) StackAlloc (stLen + stLen2 + stLen3);
    char* pszBuffer2 = pszBuffer + stLen2;
    
    memcpy (pszBuffer, MSG_WELCOME, stLen2);
    memcpy (pszBuffer2, pszEmpireName, stLen);
    memcpy (pszBuffer2 + stLen, MSG_NEWTOTHEGAME, stLen3);

    AddMessage (pszBuffer);
}

void HtmlRenderer::HashIPAddress (const char* pszIPAddress, char* pszHashedIPAddress) {
    
    // Hash the string
    unsigned int iHashValue = Algorithm::GetStringHashValue (
        pszIPAddress, 
        0x78af383d,     // A random but large positive number
        false
        );
    
    // Usually the upper byte will be a very low number.  Add some deterministic "noise"
    unsigned char* pszIP = (unsigned char*) &iHashValue;
    pszIP[3] = (unsigned char) (pszIP[3] ^ pszIP[0] ^ pszIP[1] ^ pszIP[2]);
    
    sprintf (
        pszHashedIPAddress, 
        "%u.%u.%u.%u", 
        pszIP[0], 
        pszIP[1], 
        pszIP[2], 
        pszIP[3]
        );
}

void HtmlRenderer::WriteFormattedMessage (const char* pszText) {
    
    size_t stLength = String::StrLen (pszText);
    
    char* pszFinalText = (char*) StackAlloc (stLength * 4);
    *pszFinalText = '\0';
    
    // Replace all \n's with <br>'s
    char* pszBase = (char*) pszText + stLength, 
        * pszTemp = (char*) pszText, 
        * pszFind;
    
    while (true) {
        
        pszFind = strstr (pszTemp, "\n");
        
        if (pszFind == NULL) {
            strcat (pszFinalText, pszTemp);
            break;
        }
        
        *pszFind = '\0';
        strcat (pszFinalText, pszTemp);
        strcat (pszFinalText, "<br>");
        *pszFind = '\n';
        
        pszTemp = pszFind + sizeof (char);
        
        if (pszTemp >= pszBase) {
            break;
        }
    }
    
    // Replace all \t's with four &nbsp;'s
    stLength = strlen (pszFinalText);
    pszBase = pszFinalText + stLength * sizeof (char), pszTemp = pszFinalText;
    
    while (true) {
        
        pszFind = strstr (pszTemp, "\t");
        
        if (pszFind == NULL) {
            m_pHttpResponse->WriteText (pszTemp);
            break;
        }
        
        *pszFind = '\0';
        m_pHttpResponse->WriteText (pszTemp);
        OutputText ("&nbsp;&nbsp;&nbsp;&nbsp;");
        pszTemp = pszFind + sizeof (char);
        *pszFind = '\t';
        
        if (pszTemp >= pszBase) {
            break;
        }
    }
}

void HtmlRenderer::WriteTime (Seconds sSeconds) {
     
    if (sSeconds == 0) {
        OutputText ("<strong>0</strong> sec");
        return;
    }

    int iHrs = 0, iMin = 0;
    Seconds sNumSeconds = abs(sSeconds);

    if (sNumSeconds >= 3600) {
        iHrs = sNumSeconds / 3600;
        sNumSeconds -= iHrs * 3600;
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (iHrs);
        OutputText ("</strong> hr");
        if (iHrs != 1) {
            OutputText ("s");
        }
    } else {
        OutputText ("");
    }
    
    if (sNumSeconds >= 60) {
        iMin = sNumSeconds / 60;
        sNumSeconds -= iMin * 60;
        
        if (iHrs > 0) {
            OutputText (", ");
        }
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (iMin);
        OutputText ("</strong> min");
    }
    
    if (sNumSeconds > 0) {
        if (iMin > 0 || iHrs > 0) {
            OutputText (", ");
        }
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (sNumSeconds);
        OutputText ("</strong> sec");
    }

    if (sSeconds < 0) {
        OutputText (" ago");
    }
}

void HtmlRenderer::NotifyProfileLink() {
    
    if (!m_bNotifiedProfileLink) {
        m_bNotifiedProfileLink = true;
        OutputText ("<input type=\"hidden\" name=\"HintProfileLink\" value=\"1\">");
    }
}

bool HtmlRenderer::NotifiedProfileLink() {
    
    return m_pHttpRequest->GetForm ("HintProfileLink") != NULL;
}

void HtmlRenderer::NotifyTournamentInvitation (int iMessageKey, int iTournamentKey) {

    if (!m_bNotifiedTournamentInvitation) {
        OutputText ("<input type=\"hidden\" name=\"HintTournamentInvite\" value=\"");
        m_pHttpResponse->WriteText (iMessageKey);
        OutputText (".");
        m_pHttpResponse->WriteText (iTournamentKey);
        OutputText ("\">");
        m_bNotifiedTournamentInvitation = true;
    }
}

bool HtmlRenderer::NotifiedTournamentInvitation() {

    return m_pHttpRequest->GetForm ("HintTournamentInvite") != NULL;
}

void HtmlRenderer::NotifyTournamentJoinRequest (int iMessageKey, int iTournamentKey) {

    if (!m_bNotifiedTournamentJoinRequest) {
        OutputText ("<input type=\"hidden\" name=\"HintTournamentJoinRequest\" value=\"");
        m_pHttpResponse->WriteText (iMessageKey);
        OutputText (".");
        m_pHttpResponse->WriteText (iTournamentKey);
        OutputText ("\">");
        m_bNotifiedTournamentJoinRequest = true;
    }
}

bool HtmlRenderer::NotifiedTournamentJoinRequest() {

    return m_pHttpRequest->GetForm ("HintTournamentJoinRequest") != NULL;
}

void HtmlRenderer::HTMLFilter (const char* pszSource, String* pstrFiltered, size_t stNumChars, bool bAddMarkups) {
    
    *pstrFiltered = "";
    if (!String::IsBlank (pszSource))
    {
        char* pszRet = String::AtoHtml(pszSource, pstrFiltered, stNumChars, bAddMarkups);
        Assert(pszRet);
    }
}

unsigned int HtmlRenderer::GetEmpireNameHashValue (const char* pszEmpireName) {
    
    return Algorithm::GetStringHashValue (pszEmpireName, 0xffffffff, true);
}

void HtmlRenderer::WriteStringByDiplomacy (const char* pszString, int iDiplomacy) {
    
    switch (iDiplomacy) {
        
    case WAR:
        
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\"><strong>");
        m_pHttpResponse->WriteText (pszString);
        OutputText ("</strong></font>");
        break;
        
    case ALLIANCE:
        
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\"><strong>");
        m_pHttpResponse->WriteText (pszString);
        OutputText ("</strong></font>");
        break;
        
    default:
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (pszString);
        OutputText ("</strong>");
        break;
    }
}

int HtmlRenderer::VerifyEmpireNameHash (int iEmpireKey, unsigned int iHash, bool* pbVerified)
{
    Variant vName;
    int iErrCode = GetEmpireName (iEmpireKey, &vName);
    RETURN_ON_ERROR(iErrCode);
    
    unsigned int iRealHash = Algorithm::GetStringHashValue (vName.GetCharPtr(), EMPIRE_NAME_HASH_LIMIT, true);
    *pbVerified = iRealHash == iHash;

    return iErrCode;
}

bool HtmlRenderer::ShipOrFleetNameFilter (const char* pszName) {
    
    if (pszName == NULL || *pszName == '\0') {
        return true;
    }
    
    // Check for commas
    if (strstr (pszName, ",") != NULL) {
        return false;
    }
    
    size_t i, stLen = strlen (pszName);
    
    for (i = 0; i < stLen; i ++) {
        
        if ((pszName[i] < FIRST_VALID_SHIP_OR_FLEET_NAME_CHAR ||
            pszName[i] > LAST_VALID_SHIP_OR_FLEET_CHAR)
            ||
            pszName[i] == ','
            ) {
            
            return false;
        }
    }
    
    return true;
}


int HtmlRenderer::GetGoodBadResourceLimits (int iGameClass, int iGameNumber, int* piGoodAg, int* piBadAg, int* piGoodMin,
                                            int* piBadMin, int* piGoodFuel, int* piBadFuel) {
    
    int iAg, iMin, iFuel;
    
    int iErrCode = GetGameAveragePlanetResources (iGameClass, iGameNumber, &iAg, &iMin, &iFuel);
    RETURN_ON_ERROR(iErrCode);
    
    *piGoodAg = (int) (iAg * 1.5);
    *piBadAg  = iAg / 2;
    
    *piGoodMin = (int) (iMin * 1.5);
    *piBadMin  = iAg / 2;
    
    *piGoodFuel = (int) (iFuel * 1.5);
    *piBadFuel  = iAg / 2;
    
    return OK;
}

int HtmlRenderer::SearchForDuplicateEmpires(int iGameClass, int iGameNumber, DuplicateType type)
{
    const char* pszSystemEmpireDataColumn, * pszGameEmpireDataColumn, * pszName;
    switch (type)
    {
    case IP_ADDRESS:
        pszName = "IP address";
        pszSystemEmpireDataColumn = SystemEmpireData::IPAddress;
        pszGameEmpireDataColumn = GameEmpireData::EnterGameIPAddress;
        break;
    case SESSION_ID:
        pszName = "Session Id";
        pszSystemEmpireDataColumn = SystemEmpireData::SessionId;
        pszGameEmpireDataColumn = NULL;
        break;
    default:
        Assert(false);
        return ERROR_INVALID_ARGUMENT;
    }

    int* piDuplicateKeys = NULL;
    unsigned int * piNumDuplicatesInList = NULL, iNumDuplicates;
   
    Algorithm::AutoDelete<int> free_piDuplicateKeys(piDuplicateKeys, true);
    Algorithm::AutoDelete<unsigned int> free_piNumDuplicatesInList(piNumDuplicatesInList, true);

    char pszBuffer [512];
    
    int iErrCode = SearchForDuplicates (
        iGameClass, 
        iGameNumber,
        pszSystemEmpireDataColumn,
        pszGameEmpireDataColumn,
        &piDuplicateKeys,
        &piNumDuplicatesInList,
        &iNumDuplicates
        );

    RETURN_ON_ERROR(iErrCode);
    
    if (iNumDuplicates == 0)
    {
        sprintf(pszBuffer, "No </strong>empires with the same %s were found<strong>", pszName);
        AddMessage(pszBuffer);
    }
    else
    {
        if (iNumDuplicates == 1)
        {
            sprintf(pszBuffer, "1</strong> group of empires with the same %s was found:<strong>", pszName);
        }
        else
        {
            sprintf(pszBuffer, "%i</strong> groups of empires with the same %s were found:<strong>", iNumDuplicates, pszName);
        }
        AddMessage(pszBuffer);
            
        Variant vName;
        String strList;
        unsigned int iIndex = 0, iLimit, i, j;
            
        for (i = 0; i < iNumDuplicates; i ++) {
                
            strList.Clear();
                
            iLimit = iIndex + piNumDuplicatesInList[i] - 1;
                
            for (j = iIndex; j < iLimit; j ++) {
                    
                iErrCode = GetEmpireName (piDuplicateKeys[j], &vName);
                RETURN_ON_ERROR(iErrCode);
                        
                strList += vName.GetCharPtr();
                if (j == iLimit - 1) {
                    strList += " </strong>and<strong> ";
                } else {
                    strList += ", ";
                }
            }
            iIndex = iLimit + 1;
                
            iErrCode = GetEmpireName(piDuplicateKeys[j], &vName);
            RETURN_ON_ERROR(iErrCode);

            strList += vName.GetCharPtr();
                
            strList += " </strong>have the same ";
            strList += pszName;
            strList += "<strong>";
                
            AddMessage(strList);
        }
    }

    return iErrCode;
}

void HtmlRenderer::ReportLoginFailure(const char* pszEmpireName)
{
    char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);
    sprintf(pszMessage, "Logon failure for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
        
    global.WriteReport(TRACE_WARNING, pszMessage);
}

void HtmlRenderer::ReportLoginSuccess (const char* pszEmpireName, bool bAutoLogon)
{
    char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);

    if (bAutoLogon) {
        sprintf(pszMessage, "Autologon success for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
    } else {
        sprintf(pszMessage, "Logon success for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
    }
    global.WriteReport(TRACE_INFO, pszMessage);
}


void HtmlRenderer::ReportEmpireCreation (const char* pszEmpireName)
{
    char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);
    sprintf(pszMessage, "Creation success for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
        
    global.WriteReport (TRACE_INFO, pszMessage);
}

int HtmlRenderer::InitializeSessionId(bool* pbUpdateSessionId, bool* pbUpdateCookie)
{
    int iErrCode;

    // Check for force reset
    if (m_iSystemOptions & RESET_SESSION_ID)
    {
        iErrCode = GetNewSessionId(&m_i64SessionId);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = EndResetEmpireSessionId(m_iEmpireKey);
        RETURN_ON_ERROR(iErrCode);

        *pbUpdateSessionId = *pbUpdateCookie = true;
        return OK;
    }
    
    *pbUpdateSessionId = *pbUpdateCookie = false;
    
    // First, get the empire's session id
    iErrCode = GetEmpireSessionId(m_iEmpireKey, &m_i64SessionId);
    RETURN_ON_ERROR(iErrCode);

    // Now, see if the request brought a cookie with a session id
    ICookie* pCookie = m_pHttpRequest->GetCookie(SESSION_ID_COOKIE);
    int64 i64CookieSessionId = NO_SESSION_ID;
    
    if (pCookie)
    {
        i64CookieSessionId = pCookie->GetInt64Value();
    }
    
    if (i64CookieSessionId == NO_SESSION_ID)
    {
        if (m_i64SessionId == NO_SESSION_ID)
        {
            // Generate a new session id
            iErrCode = GetNewSessionId (&m_i64SessionId);
            RETURN_ON_ERROR(iErrCode);
            
            *pbUpdateSessionId = true;
        }
        
        *pbUpdateCookie = true;
    }
    else
    {
        // We have a cookie with a valid value
        if (i64CookieSessionId != m_i64SessionId)
        {
            // For some reason the cookie has a different value than the stored value
            // This could happen if a player is on a new machine and uses a different empire
            // to generate a new value, uses the original empire, then reverts to the old machine 
            // with the old empire.  We'll believe the cookie's value, in this case, to ensure that
            // all empires on the same machine have the same session id
            //
            // This decision will have to be revised if we absolutely require that session ids be unique
            // in the future for some feature or other, since people can easily edit their cookie files and 
            // change the values
            
            m_i64SessionId = i64CookieSessionId;
            
            // Write the empire's new session id
            *pbUpdateSessionId = true;
        }
    }
    
    return iErrCode;
}

int HtmlRenderer::WriteCreateGameClassString (int iEmpireKey, unsigned int iTournamentKey, bool bPersonalGame)
{
    int iErrCode;
    unsigned int i, iMaxNumShips;

    IHttpForm* pHttpForm;

    if (iTournamentKey != NO_KEY) {
        Assert(!bPersonalGame);
    }

    // Conserve last settings variables
    String strName, strDesc;

    int iSelSuperClass = NO_KEY, iSelNumActiveGames, iSelMinNumEmpires, iSelMaxNumEmpires, 
        iSelMinNumPlanets, iSelMaxNumPlanets, iSelHoursPU, iSelMinsPU, iSelSecsPU,
        iSelMinAg, iSelMaxAg, iSelMinMin, iSelMaxMin, iSelMinFuel, iSelMaxFuel,
        iSelMinHWAg, iSelMaxHWAg, iSelMinHWMin, iSelMaxHWMin, iSelMinHWFuel, iSelMaxHWFuel,
        iSelPopLevel, iSelDip, iSelSurrenders, iSelMaxNumTruces, iSelStaticTruces,
        iSelMaxNumTrades, iSelStaticTrades, iSelMaxNumAllies, iSelStaticAllies,
        iSelUpdatesIdle, iSelRuins, iSelUpdatesRuin, iSelInitShip, iSelDevShip,
        iSelMapExposed, iSelDipShareLevel, iMapGeneration, iNumInitialTechDevs;

    float fSelInitTechLevel, fSelTechIncrease, fSelMaxAgRatio;

    bool bSelActiveGames, bSelWeekend, bSelDraws, bSelPermanentAlliances, bSelBreakAlliances,
        bSelSubjective, bFriendlyGates, bFriendlyScis, bSuicidalDoomsdays, bUnfriendlyDoomsdays, bClassicDoomsdays,
        bSelIndependence, bSelPrivate, bSelVisibleBuilds, bSelVisibleDiplomacy, bSelDipExposed, 
        bSelDisconnectedMaps;


    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassName")) != NULL)
    {
        HTMLFilter (pHttpForm->GetValue(), &strName, 0, false);
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassDescription")) != NULL)
    {
        HTMLFilter (pHttpForm->GetValue(), &strDesc, 0, false);
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("SuperClassKey")) != NULL) {
        iSelSuperClass = pHttpForm->GetIntValue();
    }

    // Name
    OutputText ("<p><table width=\"90%\"><tr><td>Name:</td><td><input type=\"text\" size=\"");
    m_pHttpResponse->WriteText (MAX_GAME_CLASS_NAME_LENGTH);
    OutputText ("\" maxlength=\"");
    m_pHttpResponse->WriteText (MAX_GAME_CLASS_NAME_LENGTH);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strName.GetCharPtr(), strName.GetLength());
    OutputText ("\" name=\"GameClassName\"></td></tr>");
    
    // Description
    OutputText (
        "<tr><td>Description:</td><td>"\
        "<textarea rows=\"3\" cols=\"50\" wrap=\"virtual\" name=\"GameClassDescription\">"
        );

    m_pHttpResponse->WriteText (strDesc.GetCharPtr(), strDesc.GetLength());
        
    OutputText ("</textarea></td></tr>");

    Variant vMaxResourcesPerPlanet, vMaxInitialTechLevel, vMaxTechDev, vUnlimitedEmpirePrivilege = ADMINISTRATOR;
    int iMinNumSecsPerUpdate, iMaxNumSecsPerUpdate, iMaxNumEmpires, iMaxNumPlanets, iActualMaxNumEmpires;
    
    if (iEmpireKey == SYSTEM) {
        
        Variant* pvSuperClassName = NULL;
        AutoFreeData free_pvSuperClassName(pvSuperClassName);

        unsigned int* piSuperClassKey, iNumSuperClasses;
        AutoFreeKeys free_piSuperClassKey(piSuperClassKey);

        iErrCode = GetSuperClassKeys(&piSuperClassKey, &pvSuperClassName, &iNumSuperClasses);
        RETURN_ON_ERROR(iErrCode);
        
        if (iNumSuperClasses > 0) {
            
            OutputText ("<tr><td>Super Class:</td><td><select name=\"SuperClassKey\">");
            
            for (i = 0; i < iNumSuperClasses; i ++)
            {       
                OutputText ("<option");

                if ((unsigned int)iSelSuperClass == piSuperClassKey[i]) {
                    OutputText (" selected");
                }

                OutputText (" value=\"");
                m_pHttpResponse->WriteText (piSuperClassKey[i]);
                OutputText ("\">");
                m_pHttpResponse->WriteText (pvSuperClassName[i].GetCharPtr());
                OutputText ("</option>");
            }
        }
        
        OutputText ("</select></td></tr>");
        
        iErrCode = GetMinNumSecsPerUpdateForSystemGameClass (&iMinNumSecsPerUpdate);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumSecsPerUpdateForSystemGameClass (&iMaxNumSecsPerUpdate);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumEmpiresForSystemGameClass (&iMaxNumEmpires);
        RETURN_ON_ERROR(iErrCode);

        iActualMaxNumEmpires = UNLIMITED_EMPIRES;
        
        iErrCode = GetMaxNumPlanetsForSystemGameClass (&iMaxNumPlanets);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::MaxResourcesPerPlanet, &vMaxResourcesPerPlanet);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::MaxInitialTechLevel, &vMaxInitialTechLevel);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::MaxTechDev, &vMaxTechDev);
        RETURN_ON_ERROR(iErrCode);
        
    } else {
        
        iErrCode = GetMinNumSecsPerUpdateForPersonalGameClass (&iMinNumSecsPerUpdate);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumSecsPerUpdateForPersonalGameClass (&iMaxNumSecsPerUpdate);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumEmpiresForPersonalGameClass (&iMaxNumEmpires);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::PrivilegeForUnlimitedEmpires, &vUnlimitedEmpirePrivilege);
         RETURN_ON_ERROR(iErrCode);

        if (m_iPrivilege >= vUnlimitedEmpirePrivilege.GetInteger()) {
            iActualMaxNumEmpires = UNLIMITED_EMPIRES;
        } else {
            iActualMaxNumEmpires = min (iMaxNumEmpires, 8);
        }

        iErrCode = GetMaxNumPlanetsForPersonalGameClass (&iMaxNumPlanets);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, &vMaxResourcesPerPlanet);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::MaxInitialTechLevelPersonal, &vMaxInitialTechLevel);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::MaxTechDevPersonal, &vMaxTechDev);
        RETURN_ON_ERROR(iErrCode);
    }

    // Conserve settings
    if ((pHttpForm = m_pHttpRequest->GetForm ("ActiveGames")) != NULL) {
        bSelActiveGames = pHttpForm->GetIntValue() != 0;
    } else {
        bSelActiveGames = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("NumActiveGames")) != NULL) {
        iSelNumActiveGames = pHttpForm->GetIntValue();
    } else {
        iSelNumActiveGames = 5;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumEmpires")) != NULL) {
        iSelMinNumEmpires = pHttpForm->GetIntValue();
    } else {
        iSelMinNumEmpires = 2;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumEmpires")) != NULL) {

        if (String::StriCmp (pHttpForm->GetValue(), UNLIMITED_STRING) == 0) {
            iSelMaxNumEmpires = UNLIMITED_EMPIRES;
        } else {
            iSelMaxNumEmpires = pHttpForm->GetIntValue();
        }
    } else {
        iSelMaxNumEmpires = iActualMaxNumEmpires;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumPlanets")) != NULL) {
        iSelMinNumPlanets = pHttpForm->GetIntValue();
    } else {
        iSelMinNumPlanets = min (4, iMaxNumPlanets);
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumPlanets")) != NULL) {
        iSelMaxNumPlanets = pHttpForm->GetIntValue();
    } else {
        iSelMaxNumPlanets = min (4, iMaxNumPlanets);
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("InitTechLevel")) != NULL) {
        fSelInitTechLevel = pHttpForm->GetFloatValue();
    } else {
        fSelInitTechLevel = (float) 1.0;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TechIncrease")) != NULL) {
        fSelTechIncrease = pHttpForm->GetFloatValue();
    } else {
        fSelTechIncrease = (float) 1.5;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("NumInitTechDevs")) != NULL) {
        iNumInitialTechDevs = pHttpForm->GetIntValue();
    } else {
        iNumInitialTechDevs = 1;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("HoursPU")) != NULL) {
        iSelHoursPU = pHttpForm->GetIntValue();
    } else {
        iSelHoursPU = 0;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinsPU")) != NULL) {
        iSelMinsPU = pHttpForm->GetIntValue();
    } else {
        iSelMinsPU = 3;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("SecsPU")) != NULL) {
        iSelSecsPU = pHttpForm->GetIntValue();
    } else {
        iSelSecsPU = 0;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("Weekend")) != NULL) {
        bSelWeekend = pHttpForm->GetIntValue() != 0;
    } else {
        bSelWeekend = true;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinAg")) != NULL) {
        iSelMinAg = pHttpForm->GetIntValue();
    } else {
        iSelMinAg = min (33, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAg")) != NULL) {
        iSelMaxAg = pHttpForm->GetIntValue();
    } else {
        iSelMaxAg = min (33, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinMin")) != NULL) {
        iSelMinMin = pHttpForm->GetIntValue();
    } else {
        iSelMinMin = min (33, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxMin")) != NULL) {
        iSelMaxMin = pHttpForm->GetIntValue();
    } else {
        iSelMaxMin = min (33, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinFuel")) != NULL) {
        iSelMinFuel = pHttpForm->GetIntValue();
    } else {
        iSelMinFuel = min (33, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxFuel")) != NULL) {
        iSelMaxFuel = pHttpForm->GetIntValue();
    } else {
        iSelMaxFuel = min (33, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWAg")) != NULL) {
        iSelMinHWAg = pHttpForm->GetIntValue();
    } else {
        iSelMinHWAg = min (100, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWAg")) != NULL) {
        iSelMaxHWAg = pHttpForm->GetIntValue();
    } else {
        iSelMaxHWAg = min (100, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWMin")) != NULL) {
        iSelMinHWMin = pHttpForm->GetIntValue();
    } else {
        iSelMinHWMin = min (100, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWMin")) != NULL) {
        iSelMaxHWMin = pHttpForm->GetIntValue();
    } else {
        iSelMaxHWMin = min (100, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWFuel")) != NULL) {
        iSelMinHWFuel = pHttpForm->GetIntValue();
    } else {
        iSelMinHWFuel = min (100, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWFuel")) != NULL) {
        iSelMaxHWFuel = pHttpForm->GetIntValue();
    } else {
        iSelMaxHWFuel = min (100, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumShips")) != NULL) {
        iMaxNumShips = pHttpForm->GetIntValue();
    } else {
        iMaxNumShips = INFINITE_SHIPS;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("PopLevel")) != NULL) {
        iSelPopLevel = pHttpForm->GetIntValue();
    } else {
        iSelPopLevel = min (50, vMaxResourcesPerPlanet.GetInteger());
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAgRatio")) != NULL) {
        fSelMaxAgRatio = pHttpForm->GetFloatValue();
    } else {
        fSelMaxAgRatio = MAX_RATIO;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("Dip")) != NULL) {
        iSelDip = pHttpForm->GetIntValue();
    } else {
        iSelDip = WAR | TRUCE | TRADE | ALLIANCE;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("Draws")) != NULL) {
        bSelDraws = pHttpForm->GetIntValue() != 0;
    } else {
        bSelDraws = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("Surrenders")) != NULL) {
        iSelSurrenders = pHttpForm->GetIntValue();
    } else {
        iSelSurrenders = 2;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTruces")) != NULL) {
        iSelMaxNumTruces = pHttpForm->GetIntValue();
    } else {
        iSelMaxNumTruces = UNRESTRICTED_DIPLOMACY;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTruces")) != NULL) {
        iSelStaticTruces = pHttpForm->GetIntValue();
    } else {
        iSelStaticTruces = 2;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTrades")) != NULL) {
        iSelMaxNumTrades = pHttpForm->GetIntValue();
    } else {
        iSelMaxNumTrades = UNRESTRICTED_DIPLOMACY;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTrades")) != NULL) {
        iSelStaticTrades = pHttpForm->GetIntValue();
    } else {
        iSelStaticTrades = 2;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumAllies")) != NULL) {
        iSelMaxNumAllies = pHttpForm->GetIntValue();
    } else {
        iSelMaxNumAllies = UNRESTRICTED_DIPLOMACY;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("StaticAllies")) != NULL) {
        iSelStaticAllies = pHttpForm->GetIntValue();
    } else {
        iSelStaticAllies = 2;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("PermanentAlliances")) != NULL) {
        bSelPermanentAlliances = pHttpForm->GetIntValue() != 0;
    } else {
        bSelPermanentAlliances = iTournamentKey != NO_KEY;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("BreakAlliances")) != NULL) {
        bSelBreakAlliances = pHttpForm->GetIntValue() != 0;
    } else {
        bSelBreakAlliances = iTournamentKey != NO_KEY;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesIdle")) != NULL) {
        iSelUpdatesIdle = pHttpForm->GetIntValue();
    } else {
        iSelUpdatesIdle = 2;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("Ruins")) != NULL) {
        iSelRuins = pHttpForm->GetIntValue();
    } else {
        iSelRuins = RUIN_ALMONASTER;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesRuin")) != NULL) {
        iSelUpdatesRuin = pHttpForm->GetIntValue();
    } else {
        iSelUpdatesRuin = 12;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MapGen")) != NULL) {
        iMapGeneration = pHttpForm->GetIntValue();
    } else {
        iMapGeneration = 0;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("Subjective")) != NULL) {
        
        bSelSubjective = pHttpForm->GetIntValue() != 0;

        // Little bit of a hack here.  We assume the presence of the Subjective
        // form tells us if this is a submission re-render or a new form
        iSelInitShip = iSelDevShip = 0;

        ENUMERATE_TECHS(i) {

            char pszShip [64];

            sprintf(pszShip, "InitShip%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszShip)) != NULL) {
                iSelInitShip |= TECH_BITS[i];
            }

            sprintf(pszShip, "DevShip%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszShip)) != NULL) {
                iSelDevShip |= TECH_BITS[i];
            }
        }

    } else {

        bSelSubjective = false;
        iSelInitShip = TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
        iSelDevShip = ALL_CLASSIC_TECHS;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("FriendlyGates")) != NULL) {
        bFriendlyGates = pHttpForm->GetIntValue() != 0;
    } else {
        bFriendlyGates = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("FriendlyScis")) != NULL) {
        bFriendlyScis = pHttpForm->GetIntValue() != 0;
    } else {
        bFriendlyScis = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("SuicidalDoomsdays")) != NULL) {
        bSuicidalDoomsdays = pHttpForm->GetIntValue() != 0;
    } else {
        bSuicidalDoomsdays = true;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("UnfriendlyDoomsdays")) != NULL) {
        bUnfriendlyDoomsdays = pHttpForm->GetIntValue() != 0;
    } else {
        bUnfriendlyDoomsdays = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("ClassicDoomsdays")) != NULL) {
        bClassicDoomsdays = pHttpForm->GetIntValue() != 0;
    } else {
        bClassicDoomsdays = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("Independence")) != NULL) {
        bSelIndependence = pHttpForm->GetIntValue() != 0;
    } else {
        bSelIndependence = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("Private")) != NULL) {
        bSelPrivate = pHttpForm->GetIntValue() != 0;
    } else {
        bSelPrivate = true;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleBuilds")) != NULL) {
        bSelVisibleBuilds = pHttpForm->GetIntValue() != 0;
    } else {
        bSelVisibleBuilds = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleDiplomacy")) != NULL) {
        bSelVisibleDiplomacy = pHttpForm->GetIntValue() != 0;
    } else {
        bSelVisibleDiplomacy = true;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("DipExposed")) != NULL) {
        bSelDipExposed = pHttpForm->GetIntValue() != 0;
    } else {
        bSelDipExposed = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("MapExposed")) != NULL) {
        iSelMapExposed = pHttpForm->GetIntValue();
    } else {
        iSelMapExposed = 0;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("DisconnectedMaps")) != NULL) {
        bSelDisconnectedMaps = pHttpForm->GetIntValue() != 0;
    } else {
        bSelDisconnectedMaps = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("DipShareLevel")) != NULL) {
        iSelDipShareLevel = pHttpForm->GetIntValue();
    } else {
        iSelDipShareLevel = NO_DIPLOMACY;
    }

    // Continue rendering
    if (bPersonalGame) {
        
        OutputText ("<input type=\"hidden\" name=\"ActiveGames\" value=\"0\">");
        
    } else {
        
        OutputText (
            "<tr>"\
            "<td>Number of simultaneously active games:</td><td><select name=\"ActiveGames\">"\
            "<option"
            );

        if (!bSelActiveGames) {
            OutputText (" selected");
        }
            
        OutputText (" value=\"0\">Unlimited number of active games</option><option");

        if (bSelActiveGames) {
            OutputText (" selected");
        }

        OutputText (
            " value=\"1\">Limited number of active games</option></select>"\
            " Limit: <input type=\"text\" size=\"4\" maxlength=\"10\" name=\"NumActiveGames\" value=\""
            );

        m_pHttpResponse->WriteText (iSelNumActiveGames);
            
        OutputText ("\"></td></tr>");
    }
    
    OutputText (
        "<tr>"\
        "<td>Number of empires <em>(<strong>2</strong> to <strong>"
        );
    
    m_pHttpResponse->WriteText (iMaxNumEmpires);
    
    OutputText (
        "</strong>)</em>:</td>"\
        "<td>From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinNumEmpires\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMinNumEmpires);

    OutputText ("\"> to <input type=\"text\" size=\"");

    if (iEmpireKey != SYSTEM && m_iPrivilege < vUnlimitedEmpirePrivilege.GetInteger()) {
        OutputText ("4");
    } else {
        OutputText ("7");
    }

    OutputText ("\" maxlength=\"20\" name=\"MaxNumEmpires\" value=\""
        );
    
    if (iSelMaxNumEmpires == UNLIMITED_EMPIRES) {
        OutputText (UNLIMITED_STRING);
    } else {
        m_pHttpResponse->WriteText (iSelMaxNumEmpires);
    }

    OutputText (
        "\">"\
        "</td></tr>"\
        
        "<tr>"\
        "<td>Number of planets per empire <em>(<strong>1</strong> to <strong>"
        );
    
    m_pHttpResponse->WriteText (iMaxNumPlanets);
    
    OutputText (
        "</strong>)</em>:</td>"\
        "<td>From "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinNumPlanets\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMinNumPlanets);

    OutputText (
        "\">"\
        " to "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxNumPlanets\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMaxNumPlanets);

    OutputText (
        "\">"\
        "</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Map generation:</td>"\
        "<td><select name=\"MapGen\">"\
        "<option"
        );

    if (iMapGeneration == 0) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Generate the map when the game starts</option>"\
        "<option"
        );

    if (iMapGeneration == GENERATE_MAP_FIRST_UPDATE) {
        OutputText (" selected");
    }

    OutputText (" value=\"");
    m_pHttpResponse->WriteText (GENERATE_MAP_FIRST_UPDATE);
    OutputText (
        "\">Generate the map on the first update</option>"\
        "</select>"\
        "</td>"\
        "</tr>"\

        "<tr>"\
        "<td>Initial tech level <em>(<strong>1.0</strong> to <strong>")
        
        m_pHttpResponse->WriteText (vMaxInitialTechLevel.GetFloat());
    
    OutputText (
        "</strong>)</em>:</td>"\
        "<td><input type=\"text\" size=\"6\" maxlength=\"20\" name=\"InitTechLevel\" value=\""
        );
    
    m_pHttpResponse->WriteText (fSelInitTechLevel);

    OutputText (
        "\"></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Tech increase per update <em>(<strong>0.0</strong> to <strong>");
    
    m_pHttpResponse->WriteText (vMaxTechDev.GetFloat());
    
    OutputText (
        "</strong>)</em>:</td>"\
        "<td><input type=\"text\" size=\"6\" maxlength=\"20\" name=\"TechIncrease\" value=\""
        );
        
    m_pHttpResponse->WriteText (fSelTechIncrease);

    OutputText (
        "\"></td>"\
        "</tr>"\


        "<tr>"\
        "<td>Initial tech developments <em>(");
    
    OutputText ("<strong>0</strong> to the number of developable techs");
    
    OutputText (
        ")</em>:</td>"\
        "<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"NumInitTechDevs\" value=\""
        );

    m_pHttpResponse->WriteText (iNumInitialTechDevs);

    OutputText (
        "\"></td>"\
        "</tr>"
        
        "<tr>"\
        "<td>Update period <em>(");
    
    WriteTime (iMinNumSecsPerUpdate);
    OutputText (" to ");
    WriteTime (iMaxNumSecsPerUpdate);
    
    OutputText (
        ")</em>:</td>"\
        "<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"HoursPU\" value=\""
        );

        m_pHttpResponse->WriteText (iSelHoursPU);
        
    OutputText (
        "\"> hours, "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinsPU\" value=\""
        );

        m_pHttpResponse->WriteText (iSelMinsPU);
        
    OutputText (
        "\"> minutes, "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SecsPU\" value=\""
        );

        m_pHttpResponse->WriteText (iSelSecsPU);

    OutputText (
        "\"> seconds</td>"\
        "</tr>"\

        "<tr>"\
        "<td>Weekend updates:</td>"\
        "<td><select name=\"Weekend\">"\
        "<option"
        );

    if (bSelWeekend) {
        OutputText (" selected");
    }
        
    OutputText (
        " value=\"1\">Update on weekends</option>"\
        "<option"
        );

    if (!bSelWeekend) {
        OutputText (" selected");
    }
        
    OutputText (
        " value=\"0\">Don't update on weekends</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Average planet resources <em>(<strong>1</strong> to <strong>"
        );
    
    m_pHttpResponse->WriteText (vMaxResourcesPerPlanet.GetInteger());
    
    OutputText (
        "</strong> each)</em>:</td>"\
        "<td>"\
        "From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinAg\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMinAg);
    
    OutputText (
        "\">"\
        " to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxAg\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMaxAg);
    
    OutputText (
        "\"> agriculture"\
        "</td></tr><tr><td></td><td>"\
        "From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinMin\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMinMin);
    
    OutputText (
        "\">"\
        " to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxMin\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMaxMin);
    
    OutputText (
        "\"> minerals"\
        "</td></tr><tr><td></td><td>"\
        "From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinFuel\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMinFuel);
    
    OutputText (
        "\">"\
        " to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxFuel\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMaxFuel);
    
    OutputText (
        "\"> fuel"\
        "</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Average homeworld resources<br><em>(from the population needed to build to "\
        "<strong>");
    
    m_pHttpResponse->WriteText (vMaxResourcesPerPlanet.GetInteger());
    
    OutputText (
        "</strong>)</em>:</td>"\
        "<td>"\
        "From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinHWAg\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMinHWAg);
    
    OutputText (
        "\">"\
        " to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxHWAg\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMaxHWAg);
    
    OutputText (
        "\"> agriculture"\
        "</td></tr><tr><td></td><td>"\
        "From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinHWMin\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMinHWMin);
    
    OutputText (
        "\">"\
        " to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxHWMin\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMaxHWMin);
    
    OutputText (
        "\"> minerals"\
        "</td></tr><tr><td></td><td>"\
        "From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinHWFuel\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMinHWFuel);
    
    OutputText (
        "\">"\
        " to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxHWFuel\" value=\""
        );

    m_pHttpResponse->WriteText (iSelMaxHWFuel);
    
    OutputText (
        "\"> fuel"\
        "</td>"\
        "</tr>"\

        "<tr>"\
        "<td>Population needed to build<br><em>(<strong>1</strong> "\
        "to the minimum homeworld agriculture)</em>:</td>"\
        "<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PopLevel\" value=\""
        );

    m_pHttpResponse->WriteText (iSelPopLevel);
    
    OutputText (
        "\"></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Maximum Agriculture Ratio <em>(<strong>");
    
    m_pHttpResponse->WriteText (MIN_MAX_AG_RATIO);
    
    OutputText ("</strong> to <strong>");
    
    m_pHttpResponse->WriteText (MAX_RATIO);
    
    OutputText (
        "</strong>)</em>:</td>"\
        "<td><input type=\"text\" size=\"8\" maxlength=\"20\" name=\"MaxAgRatio\" value=\"");
    
    m_pHttpResponse->WriteText (fSelMaxAgRatio);
    
    OutputText (
        "\"></td>"\
        "</tr>"\

        "<tr>"\
        "<td>Maximum number of ships per empire:</td>"\
        "<td><input type=\"text\" size=\"7\" maxlength=\"20\" name=\"MaxNumShips\" value=\""
        );

    if (iMaxNumShips == INFINITE_SHIPS) {
        OutputText (UNLIMITED_STRING);
    } else {
        m_pHttpResponse->WriteText (iMaxNumShips);
    }

    OutputText (
        "\"></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Diplomacy:</td>"\
        "<td><select name=\"Dip\">"\
        "<option"
        );

    if (iSelDip == WAR) {
        OutputText (" selected");
    }
    
    OutputText (
        " value=\""
        );
    
    m_pHttpResponse->WriteText (WAR);
    OutputText (
        
        "\">War</option>"\
        "<option"
        );
    
    if (iSelDip == (WAR | TRUCE)) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (WAR | TRUCE);
    OutputText (
        
        "\">War, Truce</option>"\
        "<option"
        );
    
    if (iSelDip == (WAR | TRADE)) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (WAR | TRADE);
    OutputText (
        
        "\">War, Trade</option>"\
        "<option"
        );
    
    if (iSelDip == (WAR | ALLIANCE)) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (WAR | ALLIANCE);
    OutputText (
        
        "\">War, Alliance</option>"\
        "<option"
        );
    
    if (iSelDip == (WAR | TRUCE | TRADE)) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (WAR | TRUCE | TRADE);
    OutputText (
        
        "\">War, Truce, Trade</option>"\
        "<option"
        );
    
    if (iSelDip == (WAR | TRUCE | ALLIANCE)) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (WAR | TRUCE | ALLIANCE);
    OutputText (
        
        "\">War, Truce, Alliance</option>"\
        "<option"
        );
    
    if (iSelDip == (WAR | TRADE | ALLIANCE)) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (WAR | TRADE | ALLIANCE);
    OutputText (
        
        "\">War, Trade, Alliance</option>"\
        "<option"
        );
    
    if (iSelDip == (WAR | TRUCE | TRADE | ALLIANCE)) {
        OutputText (" selected");
    }

    OutputText (
        " value=\""
        );
    
    m_pHttpResponse->WriteText (WAR | TRUCE | TRADE | ALLIANCE);
    
    OutputText (
        "\">War, Truce, Trade, Alliance</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Draw settings:</td>"\
        "<td><select name=\"Draws\">"\
        "<option"
        );
    
    if (bSelDraws) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Allow draws</option>"\
        "<option"
        );
    
    if (!bSelDraws) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">No draws</option>"\
        "</select> "\
        "</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Surrender settings:</td>"\
        "<td><select name=\"Surrenders\">"\
        "<option"
        );
    
    if (iSelSurrenders == 1) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Allow surrenders always</option>"\
        "<option"
        );
    
    if (iSelSurrenders == 2) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"2\">Allow surrenders when two empires remain</option>"\
        "<option"
        );
    
    if (iSelSurrenders == 3) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"3\">Allow classic SC-style surrenders</option>"\
        "<option"
        );
    
    if (iSelSurrenders == 0) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Never allow surrenders</option>"\
        "</select>"\
        "</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Maximum number of truces allowed:</td>"\
        "<td><select name=\"MaxNumTruces\">"\
        "<option"
        );
    
    if (iSelMaxNumTruces == UNRESTRICTED_DIPLOMACY) {
        OutputText (" selected");
    }

    OutputText (
        " value=\""
        );
    
    m_pHttpResponse->WriteText (UNRESTRICTED_DIPLOMACY);
    
    OutputText (
        "\">No restrictions</option>"\
        "<option"
        );
    
    if (iSelMaxNumTruces == FAIR_DIPLOMACY) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (FAIR_DIPLOMACY);
    
    OutputText (
        "\">A fair number: (N-2)/2</option>"\
        "<option"
        );
    
    if (iSelMaxNumTruces == STATIC_RESTRICTION) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (STATIC_RESTRICTION);
    
    OutputText (
        "\">A static limit</option></select> "\
        "Static limit: "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"StaticTruces\" value=\""
        );

    m_pHttpResponse->WriteText (iSelStaticTruces);
    
    OutputText (
        "\"></td></tr>"\
        
        "<tr>"\
        "<td>Maximum number of trades allowed:</td>"\
        "<td><select name=\"MaxNumTrades\">"\
        "<option"
        );
    
    if (iSelMaxNumTrades == UNRESTRICTED_DIPLOMACY) {
        OutputText (" selected");
    }

    OutputText (
        " value=\""
        );
    
    m_pHttpResponse->WriteText (UNRESTRICTED_DIPLOMACY);
    
    OutputText (
        "\">No restrictions</option>"\
        "<option"
        );
    
    if (iSelMaxNumTrades == FAIR_DIPLOMACY) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (FAIR_DIPLOMACY);
    
    OutputText (
        "\">A fair number: (N-2)/2</option>"\
        "<option"
        );
    
    if (iSelMaxNumTrades == STATIC_RESTRICTION) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (STATIC_RESTRICTION);
    
    OutputText ("\">A static limit</option></select> "\
        "Static limit: "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"StaticTrades\" value=\""
        );

    m_pHttpResponse->WriteText (iSelStaticTrades);
    
    OutputText (
        "\"></td></tr>"\
        
        "<tr>"\
        "<td>Maximum number of alliances allowed:</td>"\
        "<td><select name=\"MaxNumAllies\">"\
        "<option"
        );
    
    if (iSelMaxNumAllies == UNRESTRICTED_DIPLOMACY) {
        OutputText (" selected");
    }

    OutputText (
        " value=\""
        );
    
    m_pHttpResponse->WriteText (UNRESTRICTED_DIPLOMACY);
    
    m_pHttpResponse->WriteText (
        "\">No restrictions</option>"\
        "<option"
        );
    
    if (iSelMaxNumAllies == FAIR_DIPLOMACY) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (FAIR_DIPLOMACY);
    
    m_pHttpResponse->WriteText (
        "\">A fair number: (N-2)/2</option>"\
        "<option"
        );
    
    if (iSelMaxNumAllies == STATIC_RESTRICTION) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    
    m_pHttpResponse->WriteText (STATIC_RESTRICTION);
    
    OutputText (
        
        "\">A static limit</option></select> "\
        "Static limit: "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"StaticAllies\" value=\""
        );
    
    m_pHttpResponse->WriteText (iSelStaticAllies);
    
    OutputText (
        "\"> "\
        
        "<select name=\"PermanentAlliances\">"\
        "<option"
        );
    
    if (!bSelPermanentAlliances) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Limit is for concurrent alliances</option>"\
        "<option"
        );
    
    if (bSelPermanentAlliances) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Limit is for the entire game</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Breakable alliances:</td>"\
        "<td>"\
        "<select name=\"BreakAlliances\">"\
        "<option"
        );
    
    if (!bSelBreakAlliances) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Alliances are breakable</option>"\
        "<option"
        );
    
    if (bSelBreakAlliances) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Alliances are unbreakable</option>"\
        "</select>"\
        "</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Updates before empires become idle "\
        "<em>(<strong>1</strong> to <strong>"
        );

    m_pHttpResponse->WriteText (MAX_NUM_UPDATES_BEFORE_IDLE);
        
    OutputText (
        "</strong></em>):</td>"\
        "<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"UpdatesIdle\" value=\""
        );

    m_pHttpResponse->WriteText (iSelUpdatesIdle);
    
    OutputText (
        "\"></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Ruin settings:</td><td>"\
        "<select name=\"Ruins\">"\
        "<option"
        );
    
    if (iSelRuins == 0) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Empires never fall into ruin, except when they're all idle for "
        );

    m_pHttpResponse->WriteText (NUM_UPDATES_FOR_GAME_RUIN);
    
    OutputText (
        " updates</option>"\
        "<option"
        );
    
    if (iSelRuins == RUIN_CLASSIC_SC) {
        OutputText (" selected");
    }

    OutputText (
        " value=\""
        );

    m_pHttpResponse->WriteText (RUIN_CLASSIC_SC);
    
    OutputText ("\">Simple ruins: ruin after a number of idle updates</option>"\
        "<option"
        );
    
    if (iSelRuins == RUIN_ALMONASTER) {
        OutputText (" selected");
    }

    OutputText (
        " value=\""
        );

    m_pHttpResponse->WriteText (RUIN_ALMONASTER);
    
    OutputText (
        "\">Complex ruins: ruin if only one empire is awake and more than two played the game</option>"\
        "</select></td>"\
        "</tr>"\

        "<tr>"\
        "<td>Number of idle updates before empires ruin<br>"\
        "<em>(Updates before idle to <strong>"
        );

    m_pHttpResponse->WriteText (MAX_NUM_UPDATES_BEFORE_RUIN);
        
    OutputText (
        "</strong>)</em>:</td><td>"\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"UpdatesRuin\" value=\""
        );

    m_pHttpResponse->WriteText (iSelUpdatesRuin);
    
    OutputText (
        "\"></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Econ and mil views of other empires:</td>"\
        "<td><select name=\"Subjective\">"\
        "<option"
        );
    
    if (bSelSubjective) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Subjective views: econ and mil represent only what you can see</option>"\
        "<option"
        );
    
    if (!bSelSubjective) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Objective views: econ and mil represent an empire's real totals</option>"\
        "</select>"\
        "</td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Tech availability:</td>"\
        "<td><table><tr><td>"\
        "Initially available:</td><td>"
        );
    
    ENUMERATE_TECHS(i) {
        
        if (i != FIRST_SHIP) {
            OutputText ("<br>");
        }
        
        OutputText ("<input name=\"InitShip");
        m_pHttpResponse->WriteText (i);
        OutputText ("\" type=\"checkbox\"");

        if (iSelInitShip & TECH_BITS[i]) {
            OutputText (" checked");
        }
        OutputText ("> ");
        m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
    }
    
    OutputText ("</td><td>Developable:</td><td>");
    
    ENUMERATE_TECHS(i) {
        
        if (i != FIRST_SHIP) {
            OutputText ("<br>");
        }
        
        OutputText ("<input name=\"DevShip");
        m_pHttpResponse->WriteText (i);
        OutputText ("\" type=\"checkbox\"");
        if (iSelDevShip & TECH_BITS[i]) {
            OutputText (" checked");
        }
        OutputText ("> ");
        m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
    }

    OutputText ("</td></tr></table></td>"\
        "</tr>"\

        "<tr>"\
        "<td>Gate behavior:</td>"\
        "<td><select name=\"FriendlyGates\">"\
        "<option"
        );

    if (!bFriendlyGates) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Jumpgates and stargates cannot gate allied ships</option>"\
        "<option"
        );

    if (bFriendlyGates) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Jumpgates and stargates can gate allied ships</option>"\
        "</select></td>"\
        "</tr>"\

        "</tr>"\

        "<tr>"\
        "<td>Science behavior:</td>"\
        "<td><select name=\"FriendlyScis\">"\
        "<option"
        );

    if (!bFriendlyScis) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Science ships can nuke enemy planets</option>"\
        "<option"
        );

    if (bFriendlyScis) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Science ships cannot nuke enemy planets</option>"\
        "</select></td>"\
        "</tr>"\

        "<tr>"\
        "<td>Doomsday behavior:</td>"\
        "<td><select name=\"SuicidalDoomsdays\">"\
        "<option"
        );
    
    if (bSuicidalDoomsdays) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Doomsdays can annihilate their owner's own planets</option>"\
        "<option"
        );
    
    if (!bSuicidalDoomsdays) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Doomsdays cannot annihilate their owner's own planets</option>"\
        "</select><br>"\

        "<select name=\"UnfriendlyDoomsdays\">"\
        "<option"
        );
    
    if (bUnfriendlyDoomsdays) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Doomsdays can annihilate any planet belonging to other empires (except non-warring homeworlds)</option>"\
        "<option"
        );
    
    if (!bUnfriendlyDoomsdays) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Doomsdays can only annihilate planets belonging to warring empires</option>"\
        "</select><br>"\

        "<select name=\"ClassicDoomsdays\">"\
        "<option"
        );
    
    if (!bClassicDoomsdays) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Doomsdays annihilate planets for a limited quarantine time</option>"\
        "<option"
        );

    if (bClassicDoomsdays) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Doomsdays annihilate planets permanently, as in classic SC</option>"\
        "</select></td>"\
        "</tr>"\

        "<tr>"\
        "<td>Independence:</td>"\
        "<td><select name=\"Independence\">"\
        "<option"
        );
    
    if (bSelIndependence) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Independent planets and ships</option>"\
        "<option"
        );
    
    if (!bSelIndependence) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">No independent planets or ships</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Private messages:</td>"\
        "<td><select name=\"Private\">"\
        "<option"
        );
    
    if (bSelPrivate) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Allow private messages</option>"\
        "<option"
        );
    
    if (!bSelPrivate) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Don't allow private messages</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Build visibility:</td>"\
        "<td><select name=\"VisibleBuilds\">"\
        "<option"
        );
    
    if (bSelVisibleBuilds) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Visible builds</option>"\
        "<option"
        );
    
    if (!bSelVisibleBuilds) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Invisible builds</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Diplomacy changes visible before updates:</td>"\
        "<td><select name=\"VisibleDiplomacy\">"\
        "<option"
        );
    
    if (bSelVisibleDiplomacy) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Visible diplomacy</option>"\
        "<option"
        );
    
    if (!bSelVisibleDiplomacy) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Invisible diplomacy</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Diplomacy initially exposed:</td>"\
        "<td><select name=\"DipExposed\">"\
        "<option"
        );
    
    if (bSelDipExposed) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Diplomacy exposed</option>"\
        "<option"
        );
    
    if (!bSelDipExposed) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Diplomacy not exposed</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Map initial state:</td>"\
        "<td><select name=\"MapExposed\">"\
        "<option"
        );
    
    if (iSelMapExposed == EXPOSED_MAP) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    m_pHttpResponse->WriteText (EXPOSED_MAP);
    OutputText (
        "\">Map exposed</option>"\
        "<option"
        );
    
    if (iSelMapExposed == FULLY_COLONIZED_MAP) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    m_pHttpResponse->WriteText (FULLY_COLONIZED_MAP);
    OutputText (
        "\">Map fully colonized</option>"\
        "<option"
        );
    
    if (iSelMapExposed == (EXPOSED_MAP | FULLY_COLONIZED_MAP)) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    m_pHttpResponse->WriteText (EXPOSED_MAP | FULLY_COLONIZED_MAP);
    OutputText ("\">Map exposed and fully colonized</option>"\
        "<option"
        );
    
    if (iSelMapExposed == 0) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Map not exposed or fully colonized</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Disconnected maps (requires engineers):</td>"\
        "<td><select name=\"DisconnectedMaps\">"\
        "<option"
        );
    
    if (!bSelDisconnectedMaps) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"0\">Connected maps</option>"\
        "<option"
        );
    
    if (bSelDisconnectedMaps) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"1\">Disconnected maps</option>"\
        "</select></td>"\
        "</tr>"\
        
        "<tr>"\
        "<td>Map sharing:</td>"\
        "<td><select name=\"DipShareLevel\"><option"
        );
    
    if (iSelDipShareLevel == TRUCE) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    m_pHttpResponse->WriteText (TRUCE);
    OutputText ("\">Maps shared at " TRUCE_STRING "</option><option"
        );
    
    if (iSelDipShareLevel == TRADE) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    m_pHttpResponse->WriteText (TRADE);
    OutputText ("\">Maps shared at " TRADE_STRING "</option><option"
        );
    
    if (iSelDipShareLevel == ALLIANCE) {
        OutputText (" selected");
    }

    OutputText (" value=\"");
    m_pHttpResponse->WriteText (ALLIANCE);
    OutputText ("\">Maps shared at " ALLIANCE_STRING "</option><option"
        );
    
    if (iSelDipShareLevel == NO_DIPLOMACY) {
        OutputText (" selected");
    }

    OutputText (" value=\"");
    m_pHttpResponse->WriteText (NO_DIPLOMACY);
    OutputText (
        "\">Maps not shared</option></select></td>"\
        "</tr>"\
        "</table>");
    
    return iErrCode;
}

int HtmlRenderer::ProcessCreateGameClassForms(unsigned int iOwnerKey, unsigned int iTournamentKey, bool* pbProcessed)
{
    int iErrCode, iGameClass;

    *pbProcessed = false;

    Variant pvSubmitArray [SystemGameClassData::NumColumns];

    // Verify security
    if (iTournamentKey != NO_KEY) {

        unsigned int iRealOwner;

        iErrCode = GetTournamentOwner (iTournamentKey, &iRealOwner);
        if (iErrCode == ERROR_TOURNAMENT_DOES_NOT_EXIST)
        {
            AddMessage("That tournament no longer exists");
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);

        if (iRealOwner != iOwnerKey)
        {
            AddMessage ("That empire does not own that tournament");
            return OK;
        }
    }

    // Parse the forms
    bool bParsed;
    iErrCode = ParseCreateGameClassForms (pvSubmitArray, iOwnerKey, iTournamentKey, false, &bParsed);
    RETURN_ON_ERROR(iErrCode);

    if (bParsed)
    {
        // Create the gameclass, finally
        iErrCode = CreateGameClass (iOwnerKey, pvSubmitArray, &iGameClass);
        switch (iErrCode) {

        case OK:
            AddMessage ("The GameClass was created");
            *pbProcessed = true;
            break;
        case ERROR_GAMECLASS_ALREADY_EXISTS:
            AddMessage ("The GameClass name already exists");
            break;
        case ERROR_NAME_IS_TOO_LONG:
            AddMessage ("The GameClass name is too long");
            break;
        case ERROR_DESCRIPTION_IS_TOO_LONG:
            AddMessage ("The GameClass description is too long");
            break;
        case ERROR_TOO_MANY_GAMECLASSES_IN_TOURNAMENT:
            AddMessage ("The limit for GameClasses in a personal tournament has been exceeded");
            break;
        default:
            RETURN_ON_ERROR(iErrCode);
            break;
        }
        iErrCode = OK;
    }
    
    return iErrCode;
}

int HtmlRenderer::ProcessCreateDynamicGameClassForms (unsigned int iOwnerKey, int* piGameClass, int* piGameNumber, bool* pbGameCreated)
{
    int iErrCode;
    
    Variant pvSubmitArray [SystemGameClassData::NumColumns];
    
    GameOptions goOptions;
    InitGameOptions(&goOptions);
    AutoClearGameOptions clear(goOptions);
    
    *pbGameCreated = false;
    
    // Parse the forms
    bool bParsed;
    iErrCode = ParseCreateGameClassForms (pvSubmitArray, iOwnerKey, NO_KEY, true, &bParsed);
    RETURN_ON_ERROR(iErrCode);

    if (!bParsed)
    {
        return OK;
    }

    iErrCode = ParseGameConfigurationForms (NO_KEY, NO_KEY, pvSubmitArray, &goOptions);
    if (iErrCode == WARNING)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);
    
    if (goOptions.sFirstUpdateDelay > pvSubmitArray[SystemGameClassData::iNumSecPerUpdate].GetInteger() * 10)
    {
        AddMessage("The first update delay is too large");
        return OK;
    }
    
    // Dynamic gameclass
    pvSubmitArray[SystemGameClassData::iOptions] = pvSubmitArray[SystemGameClassData::iOptions].GetInteger() | DYNAMIC_GAMECLASS;
    
    // Create the gameclass
    iErrCode = CreateGameClass (iOwnerKey, pvSubmitArray, piGameClass);
    if (iErrCode != OK)
    {
        switch (iErrCode)
        {
        case ERROR_GAMECLASS_ALREADY_EXISTS:
            AddMessage ("The new game's name already exists");
            break;
        case ERROR_NAME_IS_TOO_LONG:
            AddMessage ("The new game's name is too long");
            break;
        case ERROR_DESCRIPTION_IS_TOO_LONG:
            AddMessage ("The new GameClass description is too long");
            break;
        case ERROR_TOO_MANY_GAMECLASSES_IN_TOURNAMENT:
            AddMessage ("The limit for GameClasses in a personal tournament has been exceeded");
            break;
        default:
            RETURN_ON_ERROR(iErrCode);
            break;
        }
        return OK;
    }

    // Bridier sanity
    Assert(pvSubmitArray[SystemGameClassData::iMaxNumEmpires].GetInteger() == 2 || !(goOptions.iOptions & GAME_COUNT_FOR_BRIDIER));

    // Spectator sanity
    Assert(
        ((pvSubmitArray[SystemGameClassData::iOptions].GetInteger() & EXPOSED_SPECTATORS) == 
        EXPOSED_SPECTATORS) || 
        !(goOptions.iOptions & GAME_ALLOW_SPECTATORS));

    // Try to create the game
    goOptions.iNumEmpires = 1;
    goOptions.piEmpireKey = &iOwnerKey;
    
    iErrCode = CreateGame (*piGameClass, iOwnerKey, goOptions, piGameNumber);
    RETURN_ON_ERROR(iErrCode);

    *pbGameCreated = true;

    // Halt the gameclass
    iErrCode = HaltGameClass(*piGameClass);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int HtmlRenderer::ParseCreateGameClassForms(Variant* pvSubmitArray, int iOwnerKey, unsigned int iTournamentKey, bool bDynamic, bool* pbParsed)
{
    *pbParsed = false;

    IHttpForm* pHttpForm;
    
    Variant vMaxResourcesPerPlanet;

    int iOptions = 0, iTemp, iMinNumSecsPerUpdate, iMaxNumSecsPerUpdate, iMaxNumEmpires, iMaxNumPlanets, iErrCode,
        iNumInitTechDevs, iNumTechDevs, i, iInitTechDevs = 0, iDevTechDevs = 0, iDip;
    
    char pszTechString [64];    
    const char* pszString;
    
    if (iOwnerKey == SYSTEM) {
        
        iErrCode = GetMinNumSecsPerUpdateForSystemGameClass (&iMinNumSecsPerUpdate);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumSecsPerUpdateForSystemGameClass (&iMaxNumSecsPerUpdate);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumEmpiresForSystemGameClass (&iMaxNumEmpires);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumPlanetsForSystemGameClass (&iMaxNumPlanets);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::MaxResourcesPerPlanet, &vMaxResourcesPerPlanet);
        RETURN_ON_ERROR(iErrCode);
        
    } else {
        
        iErrCode = GetMinNumSecsPerUpdateForPersonalGameClass (&iMinNumSecsPerUpdate);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumSecsPerUpdateForPersonalGameClass (&iMaxNumSecsPerUpdate);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumEmpiresForPersonalGameClass (&iMaxNumEmpires);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetMaxNumPlanetsForPersonalGameClass (&iMaxNumPlanets);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, &vMaxResourcesPerPlanet);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // GameClassName
    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassName")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    pszString = pHttpForm->GetValue();
    if (!pszString)
    {
        AddMessage ("The name cannot be blank");
        return OK;
    }
    
    if (!VerifyCategoryName ("Gameclass", pszString, MAX_GAME_CLASS_NAME_LENGTH, true))
    {
        return OK;
    }
    
    pvSubmitArray[SystemGameClassData::iName] = pszString;
    
    // Description
    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassDescription")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    pszString = pHttpForm->GetValue();
    if (!pszString)
    {
        pvSubmitArray[SystemGameClassData::iDescription] = "";
    }
    else
    {
        if (strlen (pszString) > MAX_GAMECLASS_DESCRIPTION_LENGTH)
        {
            AddMessage ("The description is too long");
            return OK;
        }
        pvSubmitArray[SystemGameClassData::iDescription] = pszString;
    }
    Assert(pvSubmitArray[SystemGameClassData::iDescription].GetCharPtr());

    // MinNumEmpires
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumEmpires")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = pHttpForm->GetIntValue();

    // MaxNumEmpires
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumEmpires")) == NULL) {
        return ERROR_MISSING_FORM;
    }

    if (String::StriCmp (pHttpForm->GetValue(), UNLIMITED_STRING) == 0) {
        pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = UNLIMITED_EMPIRES;
    } else {
        pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = pHttpForm->GetIntValue();
    }

    if (iTournamentKey != NO_KEY && 
        pvSubmitArray[SystemGameClassData::iMinNumEmpires].GetInteger() != 
        pvSubmitArray[SystemGameClassData::iMaxNumEmpires].GetInteger()) {

        AddMessage ("Tournament games must have an equal minimum and maximum number of empires");
        return OK;
    }
    
    // NumPlanetsPerEmpire
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumPlanets")) == NULL) {
        AddMessage ("Missing NumPlanetsPerEmpire form");
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = pHttpForm->GetIntValue();

    // MapGen
    if ((pHttpForm = m_pHttpRequest->GetForm ("MapGen")) == NULL) {
        AddMessage ("Missing MapGen form");
        return ERROR_MISSING_FORM;
    }

    iTemp = pHttpForm->GetIntValue();
    if (iTemp != 0) {

        if (iTemp != GENERATE_MAP_FIRST_UPDATE) {
            AddMessage ("Invalid MapGen value");
            return ERROR_MISSING_FORM;
        }

        if (pvSubmitArray[SystemGameClassData::iMinNumEmpires].GetInteger() == 
            pvSubmitArray[SystemGameClassData::iMaxNumEmpires].GetInteger()) {

            AddMessage ("The map must be generated when the game starts if the number of empires is fixed");
            return OK;
        }

        iOptions |= GENERATE_MAP_FIRST_UPDATE;
    }

    // TechIncrease
    if ((pHttpForm = m_pHttpRequest->GetForm ("TechIncrease")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxTechDev] = pHttpForm->GetFloatValue();
    
    // OpenGameNum
    pvSubmitArray[SystemGameClassData::iOpenGameNum] = 1;
    
    // SecsPerUpdate //

    // NumInitTechDevs
    if ((pHttpForm = m_pHttpRequest->GetForm ("NumInitTechDevs")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = pHttpForm->GetIntValue();
    
    // Hours per update
    if ((pHttpForm = m_pHttpRequest->GetForm ("HoursPU")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * pHttpForm->GetIntValue();
    
    // Minutes per update
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinsPU")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] += 60 * pHttpForm->GetIntValue();
    
    // Seconds per update
    if ((pHttpForm = m_pHttpRequest->GetForm ("SecsPU")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] += pHttpForm->GetIntValue();
    
    // Weekend updates
    if ((pHttpForm = m_pHttpRequest->GetForm ("Weekend")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= WEEKEND_UPDATES;
    }
    
    // InitTechLevel
    if ((pHttpForm = m_pHttpRequest->GetForm ("InitTechLevel")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] = pHttpForm->GetFloatValue();
    
    // BuilderPopLevel
    if ((pHttpForm = m_pHttpRequest->GetForm ("PopLevel")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = pHttpForm->GetIntValue();
    
    // MaxAgRatio
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAgRatio")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = pHttpForm->GetFloatValue();
    
    // MaxNumShips
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumShips")) == NULL) {
        return ERROR_MISSING_FORM;
    }

    if (String::StriCmp (pHttpForm->GetValue(), UNLIMITED_STRING) == 0) {
        pvSubmitArray[SystemGameClassData::iMaxNumShips] = INFINITE_SHIPS;
    } else {

        iTemp = pHttpForm->GetIntValue();

        if (iTemp < 1)
        {
            AddMessage ("Invalid maximum number of ships");
            return OK;
        }
    
        pvSubmitArray[SystemGameClassData::iMaxNumShips] = iTemp;
    }

    // Draws
    if ((pHttpForm = m_pHttpRequest->GetForm ("Draws")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= ALLOW_DRAW;
    }

    // Surrenders
    iTemp = 0;

    if ((pHttpForm = m_pHttpRequest->GetForm ("Surrenders")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    iDip = pHttpForm->GetIntValue();
    
    switch (iDip) {
    case 3:

        iOptions |= USE_SC30_SURRENDERS;
        break;
        
    case 2:

        if (pvSubmitArray[SystemGameClassData::iMaxNumEmpires].GetInteger() == UNLIMITED_EMPIRES ||
            pvSubmitArray[SystemGameClassData::iMaxNumEmpires].GetInteger() > 2) {
            iOptions |= ONLY_SURRENDER_WITH_TWO_EMPIRES;
        } else {
            AddMessage ("Surrenders will always be available because only two empires can join");
        }

        iTemp |= SURRENDER;
        break;
        
    case 1:
        iTemp |= SURRENDER;
        break;
    }
    
    // Dip
    if ((pHttpForm = m_pHttpRequest->GetForm ("Dip")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = pHttpForm->GetIntValue() | iTemp;
    
    iDip = pvSubmitArray[SystemGameClassData::iDiplomacyLevel].GetInteger();
    
    // BreakAlliances
    if ((pHttpForm = m_pHttpRequest->GetForm ("BreakAlliances")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= UNBREAKABLE_ALLIANCES;
    }
    
    // PermanentAlliances
    if ((pHttpForm = m_pHttpRequest->GetForm ("PermanentAlliances")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= PERMANENT_ALLIANCES;
    }
    
    // VisibleDiplomacy
    if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleDiplomacy")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= VISIBLE_DIPLOMACY;
    }

    // FriendlyGates
    if ((pHttpForm = m_pHttpRequest->GetForm ("FriendlyGates")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= USE_FRIENDLY_GATES;
    }

    // FriendlyScis
    if ((pHttpForm = m_pHttpRequest->GetForm ("FriendlyScis")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= DISABLE_SCIENCE_SHIPS_NUKING;
    }
    
    // SuicidalDoomsdays
    if ((pHttpForm = m_pHttpRequest->GetForm ("SuicidalDoomsdays")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() == 0) {
        iOptions |= DISABLE_SUICIDAL_DOOMSDAYS;
    }

    // UnfriendlyDoomsdays
    if ((pHttpForm = m_pHttpRequest->GetForm ("UnfriendlyDoomsdays")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= USE_UNFRIENDLY_DOOMSDAYS;
    }

    // ClassicDoomsdays
    if ((pHttpForm = m_pHttpRequest->GetForm ("ClassicDoomsdays")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= USE_CLASSIC_DOOMSDAYS;
    }
    
    // Independence
    if ((pHttpForm = m_pHttpRequest->GetForm ("Independence")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= INDEPENDENCE;
    }
    
    // MapExposed and FullyCol
    if ((pHttpForm = m_pHttpRequest->GetForm ("MapExposed")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    switch (pHttpForm->GetIntValue()) {
    case 0:
        break;
        
    case EXPOSED_MAP:
        iOptions |= EXPOSED_MAP;
        break;
        
    case FULLY_COLONIZED_MAP:
        iOptions |= FULLY_COLONIZED_MAP;
        break;
        
    case EXPOSED_MAP | FULLY_COLONIZED_MAP:
        iOptions |= EXPOSED_MAP | FULLY_COLONIZED_MAP;
        break;
        
    default:
        return ERROR_MISSING_FORM;
    }
    
    // DisconnectedMaps
    if ((pHttpForm = m_pHttpRequest->GetForm ("DisconnectedMaps")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= DISCONNECTED_MAP;
    }
    
    // DipShareLevel
    if ((pHttpForm = m_pHttpRequest->GetForm ("DipShareLevel")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMapsShared] = pHttpForm->GetIntValue();
    if (pvSubmitArray[SystemGameClassData::iMapsShared] != NO_DIPLOMACY &&
        (pvSubmitArray[SystemGameClassData::iMapsShared] < TRUCE ||
        pvSubmitArray[SystemGameClassData::iMapsShared] > ALLIANCE))
    {
        return ERROR_MISSING_FORM;
    }
    
    // Owner and tournament
    if (iTournamentKey == NO_KEY) {

        if (bDynamic) {
            pvSubmitArray [SystemGameClassData::iOwner] = PERSONAL_GAME;
        } else {
            pvSubmitArray [SystemGameClassData::iOwner] = iOwnerKey;
        }
        pvSubmitArray [SystemGameClassData::iTournamentKey] = NO_KEY;

    } else {
        
        pvSubmitArray [SystemGameClassData::iOwner] = TOURNAMENT;
        pvSubmitArray [SystemGameClassData::iTournamentKey] = iTournamentKey;
    }

    // Owner name
    if (iTournamentKey != NO_KEY)
    {
        iErrCode = GetTournamentName (iTournamentKey, pvSubmitArray + SystemGameClassData::iOwnerName);
        if (iErrCode == ERROR_TOURNAMENT_DOES_NOT_EXIST)
        {
            AddMessage ("That tournament no longer exists");
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        switch (iOwnerKey)
        {
        case SYSTEM:
            pvSubmitArray [SystemGameClassData::iOwnerName] = "";
            break;
        default:
            iErrCode = GetEmpireName(iOwnerKey, pvSubmitArray + SystemGameClassData::iOwnerName);
            RETURN_ON_ERROR(iErrCode);
            break;
        }
    }
    Assert(pvSubmitArray [SystemGameClassData::iOwnerName].GetCharPtr());
    
    // Private messages
    if ((pHttpForm = m_pHttpRequest->GetForm ("Private")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= PRIVATE_MESSAGES;
    }
    
    // DipExposed
    if ((pHttpForm = m_pHttpRequest->GetForm ("DipExposed")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= EXPOSED_DIPLOMACY;
    }
    
    // Visible builds
    if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleBuilds")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= VISIBLE_BUILDS;
    }
    
    // Subjective views
    if ((pHttpForm = m_pHttpRequest->GetForm ("Subjective")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= SUBJECTIVE_VIEWS;
    }
    
    // SuperClassKey
    if (iOwnerKey == SYSTEM && iTournamentKey == NO_KEY) {

        if ((pHttpForm = m_pHttpRequest->GetForm ("SuperClassKey")) == NULL) {
            return ERROR_MISSING_FORM;
        }
        pvSubmitArray[SystemGameClassData::iSuperClassKey] = pHttpForm->GetIntValue();

    } else {

        if (iTournamentKey != NO_KEY) {
            pvSubmitArray[SystemGameClassData::iSuperClassKey] = TOURNAMENT;
        } else {
            pvSubmitArray[SystemGameClassData::iSuperClassKey] = PERSONAL_GAME;
        }
    }

    // MaxNumTruces
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTruces")) == NULL) {
        return ERROR_MISSING_FORM;
    }

    iTemp = pHttpForm->GetIntValue();
    switch (iTemp) {
        
    case UNRESTRICTED_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
        break;
        
    case FAIR_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::iMaxNumTruces] = FAIR_DIPLOMACY;
        break;
        
    case STATIC_RESTRICTION:
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTruces")) == NULL) {
            return ERROR_MISSING_FORM;
        }
        pvSubmitArray[SystemGameClassData::iMaxNumTruces] = pHttpForm->GetIntValue();
        break;
        
    default:
        return ERROR_MISSING_FORM;
    }
    
    // MaxNumTrades
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTrades")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    iTemp = pHttpForm->GetIntValue();
    switch (iTemp) {
        
    case UNRESTRICTED_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
        break;
        
    case FAIR_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::iMaxNumTrades] = FAIR_DIPLOMACY;
        break;
        
    case STATIC_RESTRICTION:
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTrades")) == NULL) {
            return ERROR_MISSING_FORM;
        }
        pvSubmitArray[SystemGameClassData::iMaxNumTrades] = pHttpForm->GetIntValue();
        break;
        
    default:
        return ERROR_MISSING_FORM;
    }
    
    // MaxNumAllies
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumAllies")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    iTemp = pHttpForm->GetIntValue();
    
    switch (iTemp) {
        
    case UNRESTRICTED_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
        break;
        
    case FAIR_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = FAIR_DIPLOMACY;
        break;
        
    case STATIC_RESTRICTION:
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("StaticAllies")) == NULL) {
            return ERROR_MISSING_FORM;
        }
        pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = pHttpForm->GetIntValue();
        break;
        
    default:
        return ERROR_MISSING_FORM;
    }
    
    // MinNumPlanets
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumPlanets")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = pHttpForm->GetIntValue();

    //
    // MaxResources
    //

    // MaxAg
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAg")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = pHttpForm->GetIntValue();
    
    // MaxMin
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxMin")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = pHttpForm->GetIntValue();
    
    // MaxFuel
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxFuel")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = pHttpForm->GetIntValue();
    
    //
    // MaxResourcesHW
    //
    
    // MaxAgHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWAg")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = pHttpForm->GetIntValue();
    
    // MaxMinHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWMin")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = pHttpForm->GetIntValue();
    
    // MaxFuelHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWFuel")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = pHttpForm->GetIntValue();
    
    //
    // MinResources
    //
    
    // MinAg
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinAg")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = pHttpForm->GetIntValue();
    
    // MinMin
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinMin")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = pHttpForm->GetIntValue();
    
    // MinFuel
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinFuel")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = pHttpForm->GetIntValue();
    
    //
    // MinResourcesHW
    //
    
    // MinAgHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWAg")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMinAgHW] = pHttpForm->GetIntValue();
    
    // MinMinHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWMin")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMinMinHW] = pHttpForm->GetIntValue();
    
    // MinFuelHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWFuel")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = pHttpForm->GetIntValue();
    
    // NumUpdatesForIdle
    if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesIdle")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    iTemp = pHttpForm->GetIntValue();
    
    if (iTemp < 1 || iTemp > MAX_NUM_UPDATES_BEFORE_IDLE) {
        AddMessage ("Incorrect value for number of idle updates");
        return OK;
    }
    
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = iTemp;
    
    // RuinFlags, NumUpdatesForRuin
    if ((pHttpForm = m_pHttpRequest->GetForm ("Ruins")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    iTemp = pHttpForm->GetIntValue();
    switch (iTemp)
    {
    case 0:
    case RUIN_CLASSIC_SC:
    case RUIN_ALMONASTER:
        pvSubmitArray[SystemGameClassData::iRuinFlags] = iTemp;
        break;
        
    default:
        return ERROR_MISSING_FORM;
    }
    
    if (iTemp != 0) {
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesRuin")) == NULL) {
            return ERROR_MISSING_FORM;
        }
        
        iTemp = pHttpForm->GetIntValue();
        
        if (iTemp < 1) {
            AddMessage ("Incorrect value for number of idle updates before ruin");
            return OK;
        }

        if (iTemp < pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle].GetInteger()) {
            AddMessage ("The number of idle updates before ruin must be greater than or equal to the number of updates before idle");
            return OK;
        }

        if (iTemp > MAX_NUM_UPDATES_BEFORE_RUIN) {
            AddMessage ("The number of idle updates before ruin is too large");
            return OK;
        }

        pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = iTemp;
    }
    
    // MaxNumActiveGames
    if ((pHttpForm = m_pHttpRequest->GetForm ("ActiveGames")) == NULL) {
        return ERROR_MISSING_FORM;
    }
    
    if (pHttpForm->GetIntValue() == 0) {
        pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    } else {
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("NumActiveGames")) == NULL) {
            return ERROR_MISSING_FORM;
        }
        
        iTemp = pHttpForm->GetIntValue();
        
        if (iTemp < 1) {
            AddMessage ("Incorrect value for number of simultaneous active games");
            return OK;
        }
        
        pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = iTemp;
    }
    
    // Options
    pvSubmitArray[SystemGameClassData::iOptions] = iOptions;
    
    ///////////////////
    // Sanity checks //
    ///////////////////

    iTemp = pvSubmitArray[SystemGameClassData::iMaxNumEmpires].GetInteger();
    
    // Truce - trade - alliance limits
    if (pvSubmitArray[SystemGameClassData::iMaxNumTruces].GetInteger() != UNRESTRICTED_DIPLOMACY &&
        pvSubmitArray[SystemGameClassData::iMaxNumTruces].GetInteger() != FAIR_DIPLOMACY &&

        (pvSubmitArray[SystemGameClassData::iMaxNumTruces].GetInteger() < 1 || 
        
        (iTemp != UNLIMITED_EMPIRES &&
        pvSubmitArray[SystemGameClassData::iMaxNumTruces].GetInteger() >= iTemp)
        )
        ) {
        
        AddMessage ("Invalid maximum number of truces");
        return OK;
    }
    
    if (pvSubmitArray[SystemGameClassData::iMaxNumTrades].GetInteger() != UNRESTRICTED_DIPLOMACY &&
        pvSubmitArray[SystemGameClassData::iMaxNumTrades].GetInteger() != FAIR_DIPLOMACY &&

        (pvSubmitArray[SystemGameClassData::iMaxNumTrades].GetInteger() < 1 || 
        
        (iTemp != UNLIMITED_EMPIRES &&
        pvSubmitArray[SystemGameClassData::iMaxNumTrades].GetInteger() >= iTemp)
        )
        ) {
        
        AddMessage ("Invalid maximum number of trades");
        return OK;
    }
    
    if (pvSubmitArray[SystemGameClassData::iMaxNumAlliances].GetInteger() != UNRESTRICTED_DIPLOMACY &&
        pvSubmitArray[SystemGameClassData::iMaxNumAlliances].GetInteger() != FAIR_DIPLOMACY &&
        
        (pvSubmitArray[SystemGameClassData::iMaxNumAlliances].GetInteger() < 1 || 
        
        (iTemp != UNLIMITED_EMPIRES &&
        pvSubmitArray[SystemGameClassData::iMaxNumAlliances].GetInteger() >= iTemp)
        )
        ) {
        
        AddMessage ("Invalid maximum number of alliances");
        return OK;
    }

    // Number of empires
    if (pvSubmitArray[SystemGameClassData::iMinNumEmpires].GetInteger() < 2) {

        AddMessage ("Incorrect minimum number of empires");
        return OK;
    }

    if (iTemp != UNLIMITED_EMPIRES &&
        (iTemp < pvSubmitArray[SystemGameClassData::iMinNumEmpires].GetInteger() ||
        iTemp > iMaxNumEmpires)) {
        
        AddMessage ("Incorrect maximum number of empires");
        return OK;
    }
    
    // Planets per empire
    if (pvSubmitArray[SystemGameClassData::iMaxNumPlanets].GetInteger() < 1 || 
        pvSubmitArray[SystemGameClassData::iMaxNumPlanets].GetInteger() > iMaxNumPlanets ||
        
        pvSubmitArray[SystemGameClassData::iMinNumPlanets].GetInteger() > 
        pvSubmitArray[SystemGameClassData::iMaxNumPlanets].GetInteger()) {
        
        AddMessage ("Incorrect number of planets per empire");
        return OK;
    }
    
    // Name of gameclass
    if (pvSubmitArray[SystemGameClassData::iName].GetCharPtr() == NULL ||
        *pvSubmitArray[SystemGameClassData::iName].GetCharPtr() == '\0') {
        AddMessage ("GameClass names cannot be blank");
        return OK;
    }
    
    // Tech increase
    if (pvSubmitArray[SystemGameClassData::iMaxTechDev].GetFloat() < (float) 0.0) {
        AddMessage ("Tech increase cannot be negative");
        return OK;
    }
    
    // Tech initial level
    if (pvSubmitArray[SystemGameClassData::iInitialTechLevel].GetFloat() < (float) 1.0) {
        AddMessage ("Initial tech level cannot be less than 1.0");
        return OK;
    }
    
    // PopLevel
    if (pvSubmitArray[SystemGameClassData::iBuilderPopLevel].GetInteger() < 1) {
        AddMessage ("Incorrect population needed to build");
        return OK;
    }
    
    // MaxAgRatio
    if (pvSubmitArray[SystemGameClassData::iMaxAgRatio].GetFloat() < MIN_MAX_AG_RATIO ||
        pvSubmitArray[SystemGameClassData::iMaxAgRatio].GetFloat() > MAX_RATIO) {
        AddMessage ("Incorrect Maximum Agriculture Ratio");
        return OK;
    }
    
    // Resources
    if (pvSubmitArray[SystemGameClassData::iMinAvgAg].GetInteger() < 1 || 
        pvSubmitArray[SystemGameClassData::iMaxAvgAg].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::iMinAvgAg].GetInteger() > 
        pvSubmitArray[SystemGameClassData::iMaxAvgAg].GetInteger()) {
        
        AddMessage ("Incorrect average planet agriculture level");
        return OK;
    }
    
    if (pvSubmitArray[SystemGameClassData::iMinAvgMin].GetInteger() < 1 ||
        pvSubmitArray[SystemGameClassData::iMaxAvgMin].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::iMinAvgMin].GetInteger() > 
        pvSubmitArray[SystemGameClassData::iMaxAvgMin].GetInteger()) {
        
        AddMessage ("Incorrect average planet mineral level");
        return OK;
    }
    
    if (pvSubmitArray[SystemGameClassData::iMinAvgFuel].GetInteger() < 1 ||
        pvSubmitArray[SystemGameClassData::iMaxAvgFuel].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::iMinAvgFuel].GetInteger() > 
        pvSubmitArray[SystemGameClassData::iMaxAvgFuel].GetInteger()) {
        
        AddMessage ("Incorrect average planet fuel level");
        return OK;
    }
    
    
    if (pvSubmitArray[SystemGameClassData::iMinAgHW].GetInteger() < 
        pvSubmitArray[SystemGameClassData::iBuilderPopLevel].GetInteger() ||

        pvSubmitArray[SystemGameClassData::iMaxAgHW].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::iMinAgHW].GetInteger() > 
        pvSubmitArray[SystemGameClassData::iMaxAgHW].GetInteger()) {
        
        AddMessage ("Incorrect homeworld agriculture level");
        return OK;
    }
    
    if (pvSubmitArray[SystemGameClassData::iMinMinHW].GetInteger() < 
        pvSubmitArray[SystemGameClassData::iBuilderPopLevel].GetInteger() ||

        pvSubmitArray[SystemGameClassData::iMaxMinHW].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::iMinMinHW].GetInteger() > 
        pvSubmitArray[SystemGameClassData::iMaxMinHW].GetInteger()) {
        
        AddMessage ("Incorrect homeworld mineral level");
        return OK;
    }
    
    if (pvSubmitArray[SystemGameClassData::iMinFuelHW].GetInteger() < 
        pvSubmitArray[SystemGameClassData::iBuilderPopLevel].GetInteger() ||

        pvSubmitArray[SystemGameClassData::iMaxFuelHW].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::iMinFuelHW].GetInteger() > 
        pvSubmitArray[SystemGameClassData::iMaxFuelHW].GetInteger()) {
        
        AddMessage ("Incorrect homeworld fuel level");
        return OK;
    }
    
    // Num secs per update
    if (pvSubmitArray[SystemGameClassData::iNumSecPerUpdate].GetInteger() < iMinNumSecsPerUpdate || 
        pvSubmitArray[SystemGameClassData::iNumSecPerUpdate].GetInteger() > iMaxNumSecsPerUpdate) {
        
        AddMessage ("Invalid update period");
        return OK;
    }
    
    // Diplomacy
    if (!IsLegalDiplomacyLevel (iDip)) {
        AddMessage ("Illegal Diplomacy level");
        return OK;
    }

    // Subjective views and exposed maps
    iTemp = pvSubmitArray[SystemGameClassData::iOptions].GetInteger();
    if ((iTemp & SUBJECTIVE_VIEWS) && (iTemp & EXPOSED_MAP))
    {
        pvSubmitArray[SystemGameClassData::iOptions] = iTemp & ~SUBJECTIVE_VIEWS;
        AddMessage ("Games with exposed maps cannot have subjective views. This option has been turned off");
    }
    
    // Diplomacy for map shared
    switch (pvSubmitArray[SystemGameClassData::iMapsShared].GetInteger()) {
        
    case NO_DIPLOMACY:
        break;
        
    case TRUCE:
    case TRADE:
    case ALLIANCE:
        
        if (!GameAllowsDiplomacy (
            iDip, 
            pvSubmitArray[SystemGameClassData::iMapsShared].GetInteger())
            ) {
            
            AddMessage ("The shared map diplomacy level must be selectable");
            return OK;
        }
        break;
        
    default:
        return ERROR_MISSING_FORM;
    }

    // Alliance limit options without limits
    if (pvSubmitArray[SystemGameClassData::iMaxNumAlliances].GetInteger() == UNRESTRICTED_DIPLOMACY)
    {
        int iSystemOptions = pvSubmitArray[SystemGameClassData::iOptions].GetInteger();
        if (iSystemOptions & PERMANENT_ALLIANCES)
        {
            AddMessage ("There are no alliance limits, so alliances will not count for the entire game");
            iSystemOptions &= ~PERMANENT_ALLIANCES;
            pvSubmitArray[SystemGameClassData::iOptions] = iSystemOptions;
        }
    }

    // InitDevShips
    iNumInitTechDevs = 0;
    ENUMERATE_TECHS(i) {
        
        sprintf(pszTechString, "InitShip%i", i);
        
        if ((pHttpForm = m_pHttpRequest->GetForm (pszTechString)) != NULL) {
            iInitTechDevs |= TECH_BITS[i];
            iNumInitTechDevs ++;
        }
    }
    
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = iInitTechDevs;
    
    
    // DevShips
    iNumTechDevs = 0;
    ENUMERATE_TECHS(i) {
        
        sprintf(pszTechString, "DevShip%i", i);
        
        if ((pHttpForm = m_pHttpRequest->GetForm (pszTechString)) != NULL) {
            iDevTechDevs |= TECH_BITS[i];
            iNumTechDevs ++;
        }
    }
    
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = iDevTechDevs;

    // Make sure at least one ship type can be developed
    if (iDevTechDevs == 0) {
        AddMessage ("At least one tech must be developable");
        return OK;
    }
    
    // Verify that the initdevships can be developed
    ENUMERATE_TECHS(i) {
        
        if ((iInitTechDevs & TECH_BITS[i]) && !(iDevTechDevs & TECH_BITS[i])) {

            // An initial ship couldn't be developed
            AddMessage ("Techs marked as initially developed must be marked as developable");
            return OK;
        }
    }

    // Check NumInitialTechDevs
    iTemp = iNumTechDevs - iNumInitTechDevs;
    if (pvSubmitArray[SystemGameClassData::iNumInitialTechDevs].GetInteger() > iTemp) {

        pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = iTemp;
        AddMessage ("The initial number of tech developments was adjusted to ");
        AppendMessage (iTemp);
    }

    if (pvSubmitArray[SystemGameClassData::iNumInitialTechDevs].GetInteger() < 0) {
        AddMessage ("Not enough initial tech developments");
        return OK;
    }
    
    // Verify that engineers can be built if disconnected maps are selected
    if ((pvSubmitArray[SystemGameClassData::iOptions].GetInteger() & DISCONNECTED_MAP) &&
        !(iDevTechDevs & TECH_BITS[ENGINEER])
        ) {
        AddMessage ("Engineer tech must be developable if maps are disconnected");
        return OK;
    }
    
    // Verify that if !mapexposed, sci's can be developed
    if (!(pvSubmitArray[SystemGameClassData::iOptions].GetInteger() & EXPOSED_MAP) && 
        !(iDevTechDevs & TECH_SCIENCE)
        ) {
        AddMessage ("Science ships must be developable if maps are not exposed");
        return OK;
    }
    
    // Make sure that minefields aren't selected without minesweepers
    if ((iDevTechDevs & TECH_MINEFIELD) && 
        !(iDevTechDevs & TECH_MINESWEEPER) &&
        pvSubmitArray[SystemGameClassData::iMaxAgRatio].GetFloat() > MAX_MAX_AG_RATIO_WITHOUT_SWEEPERS
        ) {
        AddMessage ("Minefields cannot be developed if minesweepers cannot be developed and the ag ratio is less than ");
        AppendMessage (MAX_MAX_AG_RATIO_WITHOUT_SWEEPERS);
        return OK;
    }
    
    // Make sure that independence wasn't selected with maxnumempires = 2
    if (pvSubmitArray[SystemGameClassData::iOptions].GetInteger() & INDEPENDENCE &&
        pvSubmitArray[SystemGameClassData::iMaxNumEmpires].GetInteger() == 2) {
        
        pvSubmitArray[SystemGameClassData::iOptions] = 
            pvSubmitArray[SystemGameClassData::iOptions].GetInteger() & ~INDEPENDENCE;
        
        AddMessage ("The Independence option was removed because it requires more than two empires in a game.");
    }

    *pbParsed = true;

    return iErrCode;
}

void HtmlRenderer::RenderUnsafeHyperText (const char* pszText, const char* pszUrl)
{
    if (pszUrl != NULL && pszUrl[0] != '\0')
    {
        String strHtml;
        HTMLFilter(pszUrl, &strHtml, 0, false);
        if (!strHtml.IsBlank())
        {
            RenderHyperText (pszText, strHtml.GetCharPtr());
        }
    }
}

void HtmlRenderer::RenderHyperText (const char* pszText, const char* pszUrl)
{
    if (pszUrl != NULL && pszUrl[0] != '\0')
    {
        OutputText ("<a href=\"");
        
        if (String::StrniCmp(pszUrl, "http://", sizeof ("http://") - 1) != 0)
        {
            OutputText ("http://");
        }

        m_pHttpResponse->WriteText (pszUrl);
        OutputText ("\">");
        m_pHttpResponse->WriteText (pszText);
        OutputText ("</a>");
    }
}

int HtmlRenderer::WriteNukeHistory(int iTargetEmpireKey)
{
    Variant vNukeEmpireName, ** ppvNukedData = NULL, ** ppvNukerData = NULL;
    AutoFreeData free_ppvNukedData(ppvNukedData);
    AutoFreeData free_ppvNukerData(ppvNukerData);

    int iNumNuked, iNumNukers, i;
    
    int iErrCode = CacheNukeHistory(iTargetEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireName(iTargetEmpireKey, &vNukeEmpireName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNukeHistory(iTargetEmpireKey, &iNumNuked, &ppvNukedData, &iNumNukers, &ppvNukerData);
    RETURN_ON_ERROR(iErrCode);
        
    OutputText ("<input type=\"hidden\" name=\"NumNukerEmps\" value=\"");
    m_pHttpResponse->WriteText (iNumNukers);
    OutputText ("\"><input type=\"hidden\" name=\"NumNukedEmps\" value=\"");
    m_pHttpResponse->WriteText (iNumNuked);
    OutputText ("\">");
        
    if (iNumNuked == 0 && iNumNukers == 0) { 
        OutputText ("<p><strong>This empire has no nuke history</strong>");
    } else {
            
        NotifyProfileLink();
            
        char pszDateString [OS::MaxDateLength];
            
        int iAlloc = max (iNumNukers, iNumNuked);
            
        UTCTime* ptTime = (UTCTime*) StackAlloc (iAlloc * sizeof (UTCTime));
        Variant** ppvData = (Variant**) StackAlloc (iAlloc * sizeof (Variant*));
            
        if (iNumNuked > 0) {
                
            OutputText ("<p><h3>Empires who were nuked by ");
            m_pHttpResponse->WriteText (vNukeEmpireName.GetCharPtr());
            OutputText (":</h3><p><table width=\"90%\"><tr><th bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
            OutputText ("\" align=\"center\">Empire</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
            OutputText ("\" align=\"center\">Icon</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\" align=\"center\">Game</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\" align=\"center\">Time</th></tr>");
                
            // Sort by timestamp
            for (i = 0; i < iNumNuked; i ++)
            {
                ptTime[i] = ppvNukedData[i][SystemEmpireNukeList::iTimeStamp].GetInteger64();
                ppvData[i] = ppvNukedData[i];
            }
                
            Algorithm::QSortTwoDescending<UTCTime, Variant*> (ptTime, ppvData, iNumNuked);
                
            char pszEmpire [256 + MAX_EMPIRE_NAME_LENGTH];
                
            for (i = 0; i < iNumNuked; i ++)
            {
                OutputText ("<tr><td align=\"center\"><strong>");
                    
                m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::iEmpireName].GetCharPtr()); 
                OutputText("</strong></td><td align=\"center\">");
                    
                sprintf (
                    pszEmpire, 
                    "View the profile of %s",
                    ppvData[i][SystemEmpireNukeList::iEmpireName].GetCharPtr()
                    );
                    
                iErrCode = WriteProfileAlienString (
                    ppvData[i][SystemEmpireNukeList::iAlienKey].GetInteger(),
                    ppvData[i][SystemEmpireNukeList::iAlienAddress].GetInteger(),
                    ppvData[i][SystemEmpireNukeList::iReferenceEmpireKey].GetInteger(),
                    ppvData[i][SystemEmpireNukeList::iEmpireName].GetCharPtr(),
                    0,
                    "ProfileLink",
                    pszEmpire,
                    true,
                    true
                    );
                RETURN_ON_ERROR(iErrCode);
                    
                OutputText ("</td><td align=\"center\">");
                m_pHttpResponse->WriteText(ppvData[i][SystemEmpireNukeList::iGameClassName].GetCharPtr());
                OutputText (" ");
                m_pHttpResponse->WriteText(ppvData[i][SystemEmpireNukeList::iGameNumber].GetInteger());
                OutputText ("</td><td align=\"center\">");
                    
                iErrCode = Time::GetDateString(ppvData[i][SystemEmpireNukeList::iTimeStamp].GetInteger64(), pszDateString);
                Assert(iErrCode == OK);

                m_pHttpResponse->WriteText(pszDateString);
                OutputText ("</td></tr>");
            }
                
            OutputText ("</table>");
        }
            
        if (iNumNukers > 0) {
                
            OutputText ("<p><h3>Empires who nuked ");
            m_pHttpResponse->WriteText (vNukeEmpireName.GetCharPtr());
                
            OutputText (":</h3><p><table width=\"90%\"><tr><th bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\" align=\"center\">Empire</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\" align=\"center\">Icon</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\" align=\"center\">Game</th><th bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\" align=\"center\">Time</th></tr>");
                
            // Sort by timestamp
            for (i = 0; i < iNumNukers; i ++) {
                ptTime[i] = ppvNukerData[i][SystemEmpireNukeList::iTimeStamp].GetInteger64();
                ppvData[i] = ppvNukerData[i];
            }
            Algorithm::QSortTwoDescending<UTCTime, Variant*> (ptTime, ppvData, iNumNukers);
                
            char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
                
            for (i = 0; i < iNumNukers; i ++) {
                    
                OutputText ("<tr><td align=\"center\"><strong>");
                m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::iEmpireName].GetCharPtr()); 
                OutputText ("</strong></td><td align=\"center\">");
                    
                sprintf(pszProfile, "View the profile of %s", ppvData[i][SystemEmpireNukeList::iEmpireName].GetCharPtr());
                    
                iErrCode = WriteProfileAlienString (
                    ppvData[i][SystemEmpireNukeList::iAlienKey].GetInteger(),
                    ppvData[i][SystemEmpireNukeList::iAlienAddress].GetInteger(), 
                    ppvData[i][SystemEmpireNukeList::iReferenceEmpireKey].GetInteger(),
                    ppvData[i][SystemEmpireNukeList::iEmpireName].GetCharPtr(),
                    0,
                    "ProfileLink",
                    pszProfile,
                    true,
                    true
                    );
                RETURN_ON_ERROR(iErrCode);
                    
                OutputText ("</td><td align=\"center\">");
                m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::iGameClassName].GetCharPtr());
                OutputText (" ");
                m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::iGameNumber].GetInteger());
                OutputText ("</td><td align=\"center\">");
                    
                iErrCode = Time::GetDateString(ppvData[i][SystemEmpireNukeList::iTimeStamp].GetInteger64(), pszDateString);
                Assert(iErrCode == OK);

                m_pHttpResponse->WriteText(pszDateString);
                OutputText ("</td></tr>");
            }
                
            OutputText ("</table>");
        }
    }

    return iErrCode;
}

int HtmlRenderer::WritePersonalGameClasses (int iTargetEmpireKey) {
    
    int iErrCode;
    unsigned int i, iNumGameClasses, * piGameClassKey = NULL;
    AutoFreeKeys free_piGameClassKey(piGameClassKey);
    
    iErrCode = GetEmpirePersonalGameClasses (iTargetEmpireKey, &piGameClassKey, NULL, &iNumGameClasses);
    RETURN_ON_ERROR(iErrCode);

    if (iNumGameClasses == 0) {
        OutputText ("<p><strong>There are no personal GameClasses available</strong>");
    } else {
        
        int iGameClass, iInitTechs, iDevTechs;
        Variant* pvGameClassInfo = NULL;
        AutoFreeData free_pvGameClassInfo(pvGameClassInfo);

        OutputText ("<p>There ");

        if (iNumGameClasses == 1) {
            OutputText ("is 1 personal GameClass");
        } else {
            OutputText ("are <strong>");
            m_pHttpResponse->WriteText (iNumGameClasses);
            OutputText ("</strong> personal GameClasses");
        }
            
        OutputText (" available<p><h3>Start a new game:</h3>");
        WriteSystemGameListHeader (m_vTableColor);

        for (i = 0; i < iNumGameClasses; i ++) {

            iGameClass = piGameClassKey[i];

            // Read game class data
            if (pvGameClassInfo)
            {
                t_pCache->FreeData (pvGameClassInfo);
                pvGameClassInfo = NULL;
            }

            iErrCode = GetGameClassData(iGameClass, &pvGameClassInfo);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = GetDevelopableTechs(iGameClass, &iInitTechs, &iDevTechs);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = WriteSystemGameListData (iGameClass, pvGameClassInfo);
            RETURN_ON_ERROR(iErrCode);
        }

        OutputText ("</table>");
    }

    return iErrCode;
}

int HtmlRenderer::WritePersonalTournaments(int iTargetEmpireKey)
{
    int iErrCode;
    unsigned int* piTournamentKey = NULL, iTournaments;

    // List all joined tournaments
    iErrCode = GetOwnedTournaments (iTargetEmpireKey, &piTournamentKey, NULL, &iTournaments);
    RETURN_ON_ERROR(iErrCode);

    if (iTournaments == 0)
    {
        OutputText ("<p><strong>There are no personal Tournaments available</strong>");
    }
    else
    {
        OutputText ("<p>There ");
        if (iTournaments == 1)
        {
            OutputText ("is <strong>1</strong> personal Tournament");
        }
        else
        {
            OutputText ("are <strong>");
            m_pHttpResponse->WriteText (iTournaments);
            OutputText ("</strong> personal Tournaments");
        }
            
        OutputText (" available");

        iErrCode = RenderTournaments(piTournamentKey, iTournaments, false);
        t_pCache->FreeData ((int*)piTournamentKey);   // Not a bug
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int HtmlRenderer::WritePersonalTournaments()
{
    int iErrCode;
    unsigned int* piTournamentKey = NULL, iTournaments;

    // List all joined tournaments
    iErrCode = GetOwnedTournaments (m_iEmpireKey, &piTournamentKey, NULL, &iTournaments);
    RETURN_ON_ERROR(iErrCode);

    if (iTournaments > 0)
    {
        iErrCode = RenderTournaments (piTournamentKey, iTournaments, true);
        t_pCache->FreeData ((int*)piTournamentKey);   // Not a bug
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

void HtmlRenderer::WriteGameAdministratorGameData (const char* pszGameClassName, 
                                                   int iGameNumber, Seconds iSeconds, Seconds iSecondsUntil, 
                                                   int iNumUpdates, bool bOpen, bool bPaused, bool bAdminPaused, 
                                                   bool bStarted, const char* pszGamePasswordHash, 
                                                   Variant** ppvEmpiresInGame, int iNumActiveEmpires, 
                                                   const UTCTime& tCreationTime, bool bAdmin) {
    
    int i;
    Variant vName;
    
    int iErrCode;
    
    OutputText ("<tr><td align=\"left\">");
    m_pHttpResponse->WriteText (pszGameClassName);
    OutputText (" <strong>");
    m_pHttpResponse->WriteText (iGameNumber);
    
    OutputText ("</strong></td><td align=\"center\">");
    
    WriteTime (iSeconds);
    
    OutputText ("</td><td align=\"center\">");
    m_pHttpResponse->WriteText (iNumUpdates);
    
    OutputText ("</td><td align=\"center\">");
    
    char pszDateString [OS::MaxDateLength];
    iErrCode = Time::GetDateString (tCreationTime, pszDateString);
    
    if (iErrCode == OK) {
        m_pHttpResponse->WriteText (pszDateString);
    } else {
        OutputText ("Could not read date string");
    }
    
    OutputText ("</td><td align=\"center\">");
    
    if (bStarted) {
        WriteTime (iSecondsUntil);
    } else {
        OutputText ("-");
    }
    
    OutputText ("</td><td align=\"center\">");
    
    if (bOpen) {
        OutputText ("Open");
        if (bPaused) {
            if (!bAdminPaused) {
                OutputText (" (paused)");
            } else {
                OutputText (" (paused by an admin)");
            }
        } else {
            if (bStarted) {
                OutputText (" (started) ");
            } else {
                OutputText (" (not started) ");
            }
        }
    } else {
        OutputText ("Closed");
        if (bPaused) {
            if (!bAdminPaused) {
                OutputText (" (paused)");
            } else {
                OutputText (" (paused by an admin)");
            }
        }
    }
    
    OutputText ("</td>");
    
    if (bAdmin)
    {
        OutputText ("<td align=\"center\">");
        if (!String::IsBlank(pszGamePasswordHash))
        {
            OutputText("Protected");
        }
        else
        {
            OutputText("N/A");
        }
        OutputText ("</td>");
    }
    
    OutputText ("<td align=\"center\"><strong>");
    m_pHttpResponse->WriteText (iNumActiveEmpires);
    OutputText ("</strong> (");
    
    Assert(iNumActiveEmpires > 0);
    
    for (i = 0; i < iNumActiveEmpires - 1; i ++) {
        m_pHttpResponse->WriteText (ppvEmpiresInGame[i][GameEmpires::iEmpireName].GetCharPtr());
        OutputText (", ");
    }
    m_pHttpResponse->WriteText (ppvEmpiresInGame[i][GameEmpires::iEmpireName].GetCharPtr());
    
    OutputText (")</td>");
}

int HtmlRenderer::RenderThemeInfo (int iBackgroundKey, int iLivePlanetKey, int iDeadPlanetKey, int iSeparatorKey,
                                   int iButtonKey, int iHorzKey, int iVertKey, int iColorKey) {
    
    const int piUIKey[] =
    {
        iBackgroundKey,
        iColorKey,
        iLivePlanetKey, 
        iDeadPlanetKey,
        iButtonKey,
        iSeparatorKey, 
        iHorzKey, 
        iVertKey,
    };
    
    const char* ppszName[] =
    {
        "Name",
        "Background",
        "Text Colors",
        "Live Planet",
        "Dead Planet",
        "Buttons",
        "Separator",
        "Horizontal Link",
        "Vertical Link"
    };
    
    const char* ppszFormName[] = 
    {
        "Background",
        "Color",
        "LivePlanet",
        "DeadPlanet",
        "Button",
        "Separator",
        "Horz",
        "Vert"
    };
    
    const int piThemeBitField[] =
    {
        THEME_BACKGROUND,
        ALL_THEME_OPTIONS,
        THEME_LIVE_PLANET,
        THEME_DEAD_PLANET,
        THEME_BUTTONS,
        THEME_SEPARATOR,
        THEME_HORZ,
        THEME_VERT
    };
    
    const char* pszTableColor = m_vTableColor.GetCharPtr();
    
    unsigned int* piThemeKey, iNumThemes, i, j;
    AutoFreeKeys free_piThemeKey(piThemeKey);

    int iErrCode = GetThemeKeys (&piThemeKey, &iNumThemes);
    RETURN_ON_ERROR(iErrCode);

    if (iNumThemes == 0)
    {
        return OK;
    }
    
    String strHorz, strVert;
    
    GetHorzString(NULL_THEME, -1, &strHorz);
    GetVertString(NULL_THEME, -1, &strVert);
    
    OutputText ("<table><tr>");
    for (i = 0; i < sizeof (ppszName) / sizeof (const char*); i ++) {
        
        OutputText ("<th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\" align=\"center\">");
        m_pHttpResponse->WriteText (ppszName[i]);
        OutputText (":</th>");
    }
    OutputText ("</tr>");
    
    // Null Theme
    OutputText ("<tr>"\
        "<td>Null Theme</td>"\
        "<td bgcolor=\""
        );
    m_pHttpResponse->WriteText (DEFAULT_BG_COLOR);
    OutputText ("\" width=\"120\" height=\"120\">&nbsp;</td>");
    
    // Text Color
    OutputText (
        
        "<td align=\"center\"><table width=\"100\" height=\"100\" cellspacing=\"0\" bgcolor=\"#" DEFAULT_BG_COLOR "\">"\
        
        // Text
        
        "<tr><td align=\"center\"><font color=\"#" DEFAULT_TEXT_COLOR "\">Text</font></td></tr>"\
        
        // Good Color
        
        "<tr><td align=\"center\"><font color=\"#" DEFAULT_GOOD_COLOR "\">Good</font></td></tr>"\
        
        // Bad Color
        
        "<tr><td align=\"center\"><font color=\"#" DEFAULT_BAD_COLOR "\">Bad</font></td></tr>"\
        
        // Private Color
        
        "<tr><td align=\"center\"><font color=\"#" DEFAULT_PRIVATE_MESSAGE_COLOR "\">Private</font></td></tr>"\
        
        // Broadcast Color
        
        "<tr><td align=\"center\"><font color=\"#" DEFAULT_BROADCAST_MESSAGE_COLOR "\">Broadcast</font></td></tr>"\
        
        // Table Color
        
        "<tr><th align=\"center\" bgcolor=\"" DEFAULT_TABLE_COLOR "\"><font color=\"#" DEFAULT_TEXT_COLOR "\">Table</font></th></tr>"\
        
        "</table></td>"
        );
    
    // Live Planet
    OutputText ("<td align=\"center\"><img src=\"");
    WriteLivePlanetImageSrc(NULL_THEME, -1);
    
    OutputText ("\"></td><td align=\"center\"><img src=\"");
    WriteDeadPlanetImageSrc(NULL_THEME, -1);
    
    OutputText ("\"></td><td align=\"center\">");
    WriteButtonString(NULL_THEME, -1, "Login", "Login", "Login");
    
    OutputText ("</td><td align=\"center\">");
    
    WriteSeparatorString(NULL_THEME, -1);
    
    OutputText ("</td><td align=\"center\">");
    m_pHttpResponse->WriteText (strHorz);
    
    OutputText ("</td><td align=\"center\">");
    m_pHttpResponse->WriteText (strVert);
    OutputText ("</td>");
    
    OutputText ("</tr><tr><td>&nbsp;</td>");
    
    for (i = 0; i < sizeof (ppszFormName) / sizeof (const char*); i ++) {
        
        OutputText ("<td bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\" align=\"center\"><input");
        
        if (piUIKey[i] == NULL_THEME) {
            OutputText (" checked");
        }
        
        OutputText (" type=\"radio\" name=\"");
        m_pHttpResponse->WriteText (ppszFormName[i]);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (NULL_THEME);
        OutputText ("\"></td>");
    }
    
    int iOptions;
    
    char pszForm [128];
    
    for (i = 0; i < iNumThemes; i ++)
    {
        Variant* pvThemeData = NULL;
        AutoFreeData free_pvThemeData(pvThemeData);

        iErrCode = GetThemeData(piThemeKey[i], &pvThemeData);
        RETURN_ON_ERROR(iErrCode);
        
        OutputText ("<tr><td width=\"10%\" align=\"center\">");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iName].GetCharPtr());
        OutputText ("<p>");
        
        snprintf (pszForm, sizeof (pszForm), "ThemeInfo%i", piThemeKey[i]);
        WriteButtonString (m_iButtonKey, m_iButtonAddress, "Info", "Theme Info", pszForm); 
        
        OutputText ("</td>");
        
        iOptions = pvThemeData[SystemThemes::iOptions].GetInteger();
        
        // Background
        if (iOptions & THEME_BACKGROUND) {
            
            OutputText ("<td align=\"center\"><img width=\"120\" height=\"120\" src=\"");
            WriteBackgroundImageSrc (piThemeKey[i], pvThemeData[SystemThemes::iAddress]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        // Text colors
        OutputText ("<td align=\"center\"><table width=\"100\" height=\"100\" cellspacing=\"0\"");
        
        if (iOptions & THEME_BACKGROUND) {
            OutputText (" background=\"");
            WriteBackgroundImageSrc (piThemeKey[i], pvThemeData[SystemThemes::iAddress]);
            OutputText ("\"");
        }
        OutputText (">"\
            
            // Text
            "<tr><td align=\"center\"><font color=\"#");
        
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iTextColor].GetCharPtr());
        OutputText ("\">Text</font></td></tr>"\
            
            // Good Color
            "<tr><td align=\"center\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iGoodColor].GetCharPtr());
        OutputText ("\">Good</font></td></tr>"\
            
            // Bad Color
            
            "<tr><td align=\"center\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iBadColor].GetCharPtr());
        OutputText ("\">Bad</font></td></tr>"\
            
            // Private Color
            
            "<tr><td align=\"center\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iPrivateMessageColor].GetCharPtr());
        OutputText ("\">Private</font></td></tr>"\
            
            // Broadcast Color
            
            "<tr><td align=\"center\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iBroadcastMessageColor].GetCharPtr());
        OutputText ("\">Broadcast</font></td></tr>"\
            
            // Table Color
            
            "<tr><th align=\"center\" bgcolor=\"");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iTableColor].GetCharPtr());
        OutputText ("\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iTextColor].GetCharPtr());
        OutputText ("\">Table</font></th></tr>"\
            
            "</table></td>"
            );
        
        // Live Planet
        if (iOptions & THEME_LIVE_PLANET) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteLivePlanetImageSrc (piThemeKey[i], pvThemeData[SystemThemes::iAddress]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_DEAD_PLANET) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteDeadPlanetImageSrc (piThemeKey[i], pvThemeData[SystemThemes::iAddress]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_BUTTONS) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteButtonImageSrc (piThemeKey[i], pvThemeData[SystemThemes::iAddress], "Login");
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_SEPARATOR) {
            OutputText ("<td align=\"center\"><img width=\"200\" src=\"");
            WriteSeparatorSrc (piThemeKey[i], pvThemeData[SystemThemes::iAddress]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_HORZ) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteHorzSrc (piThemeKey[i], pvThemeData[SystemThemes::iAddress]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_VERT) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteVertSrc (piThemeKey[i], pvThemeData[SystemThemes::iAddress]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        OutputText ("</tr><tr><td>&nbsp;</td>");
        
        for (j = 0; j < sizeof (piThemeBitField) / sizeof (int); j ++) {
            
            if (iOptions & piThemeBitField[j]) {
                
                OutputText ("<td bgcolor=\"");
                m_pHttpResponse->WriteText (pszTableColor);
                OutputText ("\" align=\"center\"><input");
                
                if (piUIKey[j] == (int)piThemeKey[i]) {
                    OutputText (" checked");
                }
                
                OutputText (" type=\"radio\" name=\"");
                m_pHttpResponse->WriteText (ppszFormName[j]);
                OutputText ("\" value=\"");
                m_pHttpResponse->WriteText (piThemeKey[i]);
                OutputText ("\"></td>");
                
            } else { 
                OutputText ("<td>&nbsp;</td>");
            }
        }
        
        OutputText ("</tr>");
    }
    
    return iErrCode;
}

int HtmlRenderer::DisplayThemeData (int iThemeKey) {
    
    Variant* pvThemeData = NULL;
    AutoFreeData free_pvThemeData(pvThemeData);

    int iErrCode = GetThemeData (iThemeKey, &pvThemeData);
    RETURN_ON_ERROR(iErrCode);
    
    OutputText ("<p><table width=\"50%\"><tr><td>Theme:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iName].GetCharPtr());
    
    OutputText ("</td></tr><tr><td>Version:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iVersion].GetCharPtr());
    
    OutputText ("</tr><tr><td>Author:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iAuthorName].GetCharPtr());
    
    OutputText ("</td></tr><tr><td>Email:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iAuthorEmail].GetCharPtr());
    
    OutputText ("</td></tr><tr><td>Description:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iDescription].GetCharPtr());
    
    OutputText ("</td></tr><tr><td>Download:</td><td><a href=\"");
    
    WriteThemeDownloadSrc(iThemeKey, pvThemeData[SystemThemes::iAddress], pvThemeData[SystemThemes::iFileName].GetCharPtr());
    
    OutputText ("\">");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::iFileName].GetCharPtr());
    OutputText ("</a></td></tr></table><p>");
    
    WriteButton (BID_CANCEL);
    
    return iErrCode;
}

int HtmlRenderer::GetSensitiveMapText (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey,
                                       int iProxyPlanetKey, bool bVisibleBuilds, bool bIndependence,
                                       const Variant* pvPlanetData, String* pstrAltTag) {
    
    int iErrCode;
    
    int iTotalNumShips = 
        pvPlanetData[GameMap::iNumUncloakedShips].GetInteger() + 
        pvPlanetData[GameMap::iNumCloakedShips].GetInteger() + 
        pvPlanetData[GameMap::iNumUncloakedBuildShips].GetInteger() + 
        pvPlanetData[GameMap::iNumCloakedBuildShips].GetInteger();
    
    *pstrAltTag = "";
    
    if (iTotalNumShips == 0)
    {
        return OK;
    }
    
    Vector<unsigned int> vecOwnerData;
    
    iErrCode = GetPlanetShipOwnerData (
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iPlanetKey,
        iProxyPlanetKey,
        iTotalNumShips,
        bVisibleBuilds,
        bIndependence,
        vecOwnerData
        );
    
    RETURN_ON_ERROR(iErrCode);
    
    const unsigned int* piOwnerData = vecOwnerData.GetData();
    unsigned int iNumOwners = piOwnerData[0];
    if (iNumOwners > 0)
    {
        unsigned int i, j, iType, iNumShips, iOwnerKey, iBase, iNumOwnerTechs, iNumOwnerShips;
        Variant vEmpireName;
        
        iBase = 1;
        
        for (i = 0; i < iNumOwners; i ++) {
            
            if (i > 0) {
                *pstrAltTag += "\n";
            }
            
            iOwnerKey = piOwnerData [iBase];
            
            iNumOwnerShips = piOwnerData [iBase + 1];
            iNumOwnerTechs = piOwnerData [iBase + 2];
            
            Assert(iNumOwnerTechs > 0);
            
            if (iOwnerKey == m_iEmpireKey) {
                *pstrAltTag += m_vEmpireName.GetCharPtr();
            }
            
            else if (iOwnerKey == INDEPENDENT) {
                *pstrAltTag += "Independent";
            }
            
            else {
                
                iErrCode = GetEmpireName (iOwnerKey, &vEmpireName);
                RETURN_ON_ERROR(iErrCode);
                
                *pstrAltTag += vEmpireName.GetCharPtr();
            }
            
            *pstrAltTag += ": ";
            
            for (j = 0; j < iNumOwnerTechs; j ++) {
                
                iType = piOwnerData [iBase + 3 + j * 2];
                iNumShips = piOwnerData [iBase + 4 + j * 2];
                
                Assert(iType >= FIRST_SHIP && iType <= LAST_SHIP);
                Assert(iNumShips > 0);
                
                if (j > 0) {
                    *pstrAltTag += ", ";
                }
                
                *pstrAltTag += iNumShips;
                *pstrAltTag += " ";
                
                if (iNumShips == 1) {
                    *pstrAltTag += SHIP_TYPE_STRING[iType];
                } else {
                    *pstrAltTag += SHIP_TYPE_STRING_PLURAL[iType];
                }
            }
            
            iBase += 3 + 2 * iNumOwnerTechs;
        }
    }
    
    return iErrCode;
}


//
// Events and statistics
//

int HtmlRenderer::OnCreateEmpire (int iEmpireKey) {
    
    // Stats
    Algorithm::AtomicIncrement(&m_sStats.EmpiresCreated);
    
    return OK;
}

int HtmlRenderer::OnDeleteEmpire(int iEmpireKey) {
    
    int iErrCode;
    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
        global.GetResourceDir(),
        iEmpireKey
        );
    
    // Just in case...
    global.GetFileCache()->ReleaseFile (pszDestFileName);
    
    // Best effort attempt to delete an uploaded file
    bool bFileDeleted = File::DeleteFile(pszDestFileName) == OK;

    // Stats
    Algorithm::AtomicIncrement(&m_sStats.EmpiresDeleted);

    // Report
    char pszEmpireName [MAX_EMPIRE_NAME_LENGTH + 1];

    GameEngine gameEngine;
    iErrCode = gameEngine.GetEmpireName(iEmpireKey, pszEmpireName);
    RETURN_ON_ERROR(iErrCode);

    char pszMessage[MAX_EMPIRE_NAME_LENGTH + 256];
    sprintf(pszMessage, "%s was deleted", pszEmpireName);
    global.WriteReport(TRACE_INFO, pszMessage);

    if (bFileDeleted)
    {
        sprintf(pszMessage, "Uploaded icon %i was deleted", iEmpireKey);
        global.WriteReport(TRACE_INFO, pszMessage);
    }
    
    return iErrCode;
}

int HtmlRenderer::OnLoginEmpire (int iEmpireKey) {
    
    Algorithm::AtomicIncrement(&m_sStats.Logins);
    
    return OK;
}

int HtmlRenderer::OnCreateGame (int iGameClass, int iGameNumber) {
    
    Algorithm::AtomicIncrement(&m_sStats.GamesStarted);
    
    return OK;
}

int HtmlRenderer::OnCleanupGame (int iGameClass, int iGameNumber) {
    
    Algorithm::AtomicIncrement(&m_sStats.GamesEnded);
    
    return OK;
}

int HtmlRenderer::OnDeleteTournament (unsigned int iTournamentKey) {

    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_UPLOADED_TOURNAMENT_ICON_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
        global.GetResourceDir(),
        iTournamentKey
        );
    
    // Just in case...
    global.GetFileCache()->ReleaseFile (pszDestFileName);
    
    // Best effort attempt to delete an uploaded file
    bool bFileDeleted = File::DeleteFile(pszDestFileName) == OK;
    if (bFileDeleted)
    {
        char pszMessage[256];
        sprintf(pszMessage, "Uploaded icon %i was deleted", iTournamentKey);
        global.WriteReport(TRACE_INFO, pszMessage);
    }

    return OK;
}

int HtmlRenderer::OnDeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) {

    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_UPLOADED_TOURNAMENT_TEAM_ICON_DIR "/" ALIEN_NAME "%i.%i" DEFAULT_IMAGE_EXTENSION, 
        global.GetResourceDir(),
        iTournamentKey,
        iTeamKey
        );
    
    // Just in case...
    global.GetFileCache()->ReleaseFile (pszDestFileName);
    
    // Best effort attempt to delete an uploaded file
    bool bFileDeleted = File::DeleteFile(pszDestFileName) == OK;
    if (bFileDeleted)
    {
        char pszMessage[256];
        sprintf(pszMessage, "Uploaded icon %i.%i was deleted", iTournamentKey, iTeamKey);
        global.WriteReport(TRACE_INFO, pszMessage);
    }

    return OK;
}

int HtmlRenderer::OnPageRender (MilliSeconds msTime) {
    
    Algorithm::AtomicIncrement(&m_sStats.NumPageScriptRenders);
    Algorithm::AtomicIncrement(&m_sStats.TotalScriptTime, msTime);
    
    return OK;
}

void HtmlRenderer::ZeroStatistics() {
    
    memset (&m_sStats, 0, sizeof (AlmonasterStatistics));
}

static const char* const g_pszEmpireInfoHeadersAdmin[] = {
    "Name",
    "Alien",
    "Econ",
    "Mil",
    "Tech",
    "Planets",
    "Ships",
    "Initial map",
    "War",
    "Truce",
    "Trade",
    "Alliance",
    "Pause",
    "Draw",
    "Last Access",
    "Ready for Update",
};

static const char* const g_pszEmpireInfoHeadersSpectator[] = {
    "Name",
    "Alien",
    "Econ",
    "Mil",
    "Planets",
    "Pause",
    "Draw",
    "Last Access",
    "Ready for Update",
};

static const char* const g_pszDeadEmpireHeaders[] = {
    "Name",
    "Alien",
    "Last Update",
    "Reason",
};

static const bool g_bDeadEmpireHeadersColspan[] = {
    false,
    false,
    true,
    true,
};

int HtmlRenderer::RenderEmpireInformation(int iGameClass, int iGameNumber, bool bAdmin)
{
    int iErrCode, iValue, iFoeKey, iWar, iTruce, iTrade, iAlliance, iUnmet, * piNumUpdatesIdle, * piOptions;
    unsigned int i, iKey, iNumRows, iNumEmpires, iIdleEmpires, iResignedEmpires, iNumTopHeaders;
    float fValue;

    Variant vValue, vTemp;
    UTCTime tCurrentTime, tValue;

    String strWar, strTruce, strTrade, strAlliance;

    const char* pszTableColor = m_vTableColor.GetCharPtr(), * const * pszHeaders;

    char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
    char strGameEmpireData [256], pszGameEmpireShips [256], pszGameEmpireDip [256];

    Time::GetTime (&tCurrentTime);

    Variant** ppvEmpiresInGame = NULL;
    AutoFreeData free_ppvEmpiresInGame(ppvEmpiresInGame);

    iErrCode = GetEmpiresInGame(iGameClass, iGameNumber, &ppvEmpiresInGame, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    piNumUpdatesIdle = (int*)StackAlloc(2 * iNumEmpires * sizeof(int));
    piOptions = piNumUpdatesIdle + iNumEmpires;

    iIdleEmpires = iResignedEmpires = 0;
    for (i = 0; i < iNumEmpires; i ++) {

#ifdef _DEBUG
        iErrCode = CheckTargetPop (iGameClass, iGameNumber, ppvEmpiresInGame[i][GameEmpires::iEmpireKey].GetInteger());
        RETURN_ON_ERROR(iErrCode);
#endif
        COPY_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, ppvEmpiresInGame[i][GameEmpires::iEmpireKey].GetInteger());

        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vValue);
        RETURN_ON_ERROR(iErrCode);
        piOptions[i] = vValue.GetInteger();

        if (piOptions[i] & RESIGNED) {
            iResignedEmpires ++;
        }

        if (piOptions[i] & LOGGED_IN_THIS_UPDATE) {
            piNumUpdatesIdle[i] = 0;
        } else {
            iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::NumUpdatesIdle, &vValue);
            RETURN_ON_ERROR(iErrCode);

            piNumUpdatesIdle[i] = vValue.GetInteger();
            if (piNumUpdatesIdle[i] > 0) {
                iIdleEmpires ++;
            }
        }
    }

    OutputText (
        "<p><table width=\"90%\">"\
        "<tr><td colspan=\"14\" align=\"center\">There are <strong>"
        );

    m_pHttpResponse->WriteText (iNumEmpires);

    OutputText ("</strong> empires in the game");

    if (iResignedEmpires > 0 || iIdleEmpires > 0) {

        OutputText (" (<strong>");

        if (iResignedEmpires > 0) {
            m_pHttpResponse->WriteText (iResignedEmpires);
            OutputText ("</strong> resigned");
        }

        if (iIdleEmpires > 0) {

            if (iResignedEmpires > 0) {
                OutputText (", <strong>");
            }
            m_pHttpResponse->WriteText (iIdleEmpires);
            OutputText ("</strong> idle");
        }
        OutputText (")");
    }

    OutputText (        
        ":</td></tr>"\
        "<tr><td>&nbsp</td></tr>"\
        "<tr>"
        );

    if (bAdmin) {

        pszHeaders = g_pszEmpireInfoHeadersAdmin;
        iNumTopHeaders = sizeof (g_pszEmpireInfoHeadersAdmin) / sizeof (char*);

    } else {

        pszHeaders = g_pszEmpireInfoHeadersSpectator;
        iNumTopHeaders = sizeof (g_pszEmpireInfoHeadersSpectator) / sizeof (char*);
    }

    for (i = 0; i < iNumTopHeaders; i ++) {
        
        OutputText ("<th bgcolor=\"#");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">");
        m_pHttpResponse->WriteText (pszHeaders[i]);
        OutputText ("</th>");
    }

    OutputText ("</tr>");

    NotifyProfileLink();

    for (i = 0; i < iNumEmpires; i ++) {

        int iOptions;
        unsigned int iCurrentEmpireKey = ppvEmpiresInGame[i][GameEmpires::iEmpireKey].GetInteger();

        COPY_GAME_EMPIRE_DATA(strGameEmpireData, iGameClass, iGameNumber, iCurrentEmpireKey);

        OutputText ("<tr>");

        // Options
        iOptions = piOptions[i];

        // Name

        OutputText ("<td align=\"center\">");

        if (iOptions & RESIGNED) {
            OutputText ("<strike>");
        }

        m_pHttpResponse->WriteText(ppvEmpiresInGame[i][GameEmpires::iEmpireName].GetCharPtr());

        if (iOptions & RESIGNED) {
            OutputText ("</strike>");
        }

        OutputText ("</td>");

        // Alien
        iErrCode = GetEmpireProperty(iCurrentEmpireKey, SystemEmpireData::AlienKey, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        unsigned int iAlienKey = vTemp.GetInteger();

        iErrCode = GetEmpireProperty(iCurrentEmpireKey, SystemEmpireData::AlienAddress, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        int iAlienAddress = vTemp.GetInteger();

        OutputText ("<td align=\"center\">");

        sprintf(pszProfile, "View the profile of %s", ppvEmpiresInGame[i][GameEmpires::iEmpireName].GetCharPtr());

        iErrCode = WriteProfileAlienString (
            iAlienKey,
            iAlienAddress,
            iCurrentEmpireKey,
            ppvEmpiresInGame[i][GameEmpires::iEmpireName].GetCharPtr(),
            0,
            "ProfileLink",
            pszProfile,
            false,
            true
            );
        RETURN_ON_ERROR(iErrCode);

        OutputText ("</td>");

        // Econ
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Econ, &vValue);
        RETURN_ON_ERROR(iErrCode);
        iValue = vValue.GetInteger();

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (iValue);
        OutputText ("</td>");

        // Mil
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Mil, &vValue);
        RETURN_ON_ERROR(iErrCode);
        fValue = vValue.GetFloat();

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText(GetMilitaryValue (fValue));
        OutputText ("</td>");

        if (bAdmin)
        {
            // Tech
            iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::TechLevel, &vValue);
            RETURN_ON_ERROR(iErrCode);
            fValue = vValue.GetFloat();

            OutputText ("<td align=\"center\">");
            m_pHttpResponse->WriteText (fValue);
            OutputText ("</td>");
        }

        // Planets
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::NumPlanets, &vValue);
        RETURN_ON_ERROR(iErrCode);
        iValue = vValue.GetInteger();

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (iValue);
        OutputText ("</td>");

        if (bAdmin)
        {
            // Ships
            COPY_GAME_EMPIRE_SHIPS(pszGameEmpireShips, iGameClass, iGameNumber, iCurrentEmpireKey);
            COPY_GAME_EMPIRE_DIPLOMACY(pszGameEmpireDip, iGameClass, iGameNumber, iCurrentEmpireKey);

            unsigned int iShips;
            iErrCode = t_pCache->GetNumCachedRows(pszGameEmpireShips, &iShips);
            RETURN_ON_ERROR(iErrCode);

            OutputText ("<td align=\"center\">");
            m_pHttpResponse->WriteText(iShips);
            OutputText ("</td>");

            Variant vProp;
            iErrCode = GetEmpireGameProperty(
                iGameClass, 
                iGameNumber, 
                iCurrentEmpireKey,
                GameEmpireData::MapFairnessResourcesClaimed,
                &vProp
                );
            RETURN_ON_ERROR(iErrCode);

            OutputText ("<td align=\"center\">");
            if (vProp.GetInteger() == 0) {
                OutputText("N/A");
            } else {
                m_pHttpResponse->WriteText(vProp.GetInteger());
                OutputText(" resources");
            }
            OutputText("</td>");

            iKey = NO_KEY;
            iWar = iTruce = iTrade = iAlliance = iUnmet = 0;
            
            strWar.Clear();
            strTruce.Clear();
            strTrade.Clear();
            strAlliance.Clear();

            while (true)
            {
                String* pStr = &strWar;
                Variant vName;

                iErrCode = t_pCache->GetNextKey (pszGameEmpireDip, iKey, &iKey);
                if (iErrCode == ERROR_DATA_NOT_FOUND)
                {
                    iErrCode = OK;
                    break;
                }
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(pszGameEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vValue);
                RETURN_ON_ERROR(iErrCode);
                iValue = vValue.GetInteger();

                iErrCode = t_pCache->ReadData(pszGameEmpireDip, iKey, GameEmpireDiplomacy::ReferenceEmpireKey, &vValue);
                RETURN_ON_ERROR(iErrCode);
                iFoeKey = vValue.GetInteger();

                iErrCode = GetEmpireName (iFoeKey, &vName);
                RETURN_ON_ERROR(iErrCode);

                switch (iValue) {

                case WAR:
                    pStr = &strWar;
                    iWar ++;
                    break;

                case TRUCE:
                    pStr = &strTruce;
                    iTruce ++;
                    break;

                case TRADE:
                    pStr = &strTrade;
                    iTrade ++;
                    break;

                case ALLIANCE:
                    pStr = &strAlliance;
                    iAlliance ++;
                    break;

                default:
                    Assert(false);
                    break;
                }

                if (!pStr->IsBlank()) {
                    *pStr += ", ";
                }
                *pStr += vName.GetCharPtr();
            }

            iUnmet = iNumEmpires - iWar - iTruce - iTrade - iAlliance - 1;

            // War
            OutputText ("<td align=\"center\">");

            if (iWar > 0) {
                m_pHttpResponse->WriteText (iWar);
                m_pHttpResponse->WriteText (" (");
                m_pHttpResponse->WriteText (strWar, strWar.GetLength());
                m_pHttpResponse->WriteText (")");

                if (iUnmet > 0) {
                    m_pHttpResponse->WriteText (", ");
                }
            }

            if (iUnmet > 0) {
                m_pHttpResponse->WriteText (iUnmet);
                m_pHttpResponse->WriteText (" unmet");
            }
            else if (iWar == 0) {
                m_pHttpResponse->WriteText ("0");
            }

            OutputText ("</td>");

            // Truce
            OutputText ("<td align=\"center\">");
            m_pHttpResponse->WriteText (iTruce);
            if (iTruce > 0) {
                m_pHttpResponse->WriteText (" (");
                m_pHttpResponse->WriteText (strTruce, strTruce.GetLength());
                m_pHttpResponse->WriteText (")");
            }
            OutputText ("</td>");

            // Trade
            OutputText ("<td align=\"center\">");
            m_pHttpResponse->WriteText (iTrade);
            if (iTrade > 0) {
                m_pHttpResponse->WriteText (" (");
                m_pHttpResponse->WriteText (strTrade, strTrade.GetLength());
                m_pHttpResponse->WriteText (")");
            }
            OutputText ("</td>");

            // Alliance
            OutputText ("<td align=\"center\">");
            m_pHttpResponse->WriteText (iAlliance);
            if (iAlliance > 0) {
                m_pHttpResponse->WriteText (" (");
                m_pHttpResponse->WriteText (strAlliance, strAlliance.GetLength());
                m_pHttpResponse->WriteText (")");
            }
            OutputText ("</td>");
        }

        // Pause
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vValue);
        RETURN_ON_ERROR(iErrCode);
        iValue = vValue.GetInteger();

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText ((iValue & REQUEST_PAUSE) != 0 ? "Yes" : "No");
        OutputText ("</td>");

        // Draw
        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText ((iValue & REQUEST_DRAW) != 0 ? "Yes" : "No");
        OutputText ("</td>");

        bool bUpdated = iValue & UPDATED;

        // LastLogin, idle
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::LastLogin, &vValue);
        RETURN_ON_ERROR(iErrCode);
        tValue = vValue.GetInteger64();

        OutputText ("<td align=\"center\">");
        WriteTime (Time::GetSecondDifference (tCurrentTime, tValue));
        OutputText (" ago");

        iValue = piNumUpdatesIdle[i];
        if (iValue > 0) {

            OutputText ("<br>(<strong>");
            m_pHttpResponse->WriteText (iValue);
            OutputText ("</strong> update");

            if (iValue != 1) {
                OutputText ("s");
            }
            OutputText (" idle)");
        }
        OutputText ("</td>");

        // Updated
        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (bUpdated ? "Yes" : "No");
        OutputText ("</td></tr>");
    }

    //
    // Dead empires
    //

    COPY_GAME_NUKED_EMPIRES (strGameEmpireData, iGameClass, iGameNumber);
    iErrCode = t_pCache->GetNumCachedRows(strGameEmpireData, &iNumRows);
    RETURN_ON_ERROR(iErrCode);
    
    if (iNumRows > 0)
    {
        const int iNumHeaders = sizeof (g_pszDeadEmpireHeaders) / sizeof (char*);

        OutputText (
            "<tr><td>&nbsp</td></tr>"\
            "<tr><td colspan=\"14\" align=\"center\">Dead empires:</td></tr>"\
            "<tr><td>&nbsp</td></tr>"\
            "<tr>"
            );

        for (i = 0; i < iNumHeaders; i ++) {

            OutputText ("<th bgcolor=\"#");
            m_pHttpResponse->WriteText (pszTableColor);

            if (g_bDeadEmpireHeadersColspan[i]) {
                OutputText ("\" colspan=\"2");
            }

            OutputText ("\">");
            m_pHttpResponse->WriteText (g_pszDeadEmpireHeaders[i]);
            OutputText ("</th>");
        }

        OutputText ("<th colspan=\"");
        m_pHttpResponse->WriteText (iNumTopHeaders - iNumHeaders);
        OutputText ("\">&nbsp;</th></tr><tr>");

        iKey = NO_KEY;
        while (true) {
            
            const char* pszName;
            int iDeadEmpireKey;
            
            iErrCode = t_pCache->GetNextKey (strGameEmpireData, iKey, &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(strGameEmpireData, iKey, GameNukedEmpires::AlienKey, &vValue);
            RETURN_ON_ERROR(iErrCode);
            unsigned int iAlienKey = vValue.GetInteger();

            iErrCode = t_pCache->ReadData(strGameEmpireData, iKey, GameNukedEmpires::AlienAddress, &vValue);
            RETURN_ON_ERROR(iErrCode);
            int iAlienAddress = vValue.GetInteger();
            
            iErrCode = t_pCache->ReadData(strGameEmpireData, iKey, GameNukedEmpires::NukedEmpireKey, &vValue);
            RETURN_ON_ERROR(iErrCode);
            iDeadEmpireKey = vValue.GetInteger();

            iErrCode = t_pCache->ReadData(strGameEmpireData, iKey, GameNukedEmpires::Name, &vValue);
            RETURN_ON_ERROR(iErrCode);
            pszName = vValue.GetCharPtr();

            OutputText ("<tr><td align=\"center\">");
            m_pHttpResponse->WriteText (pszName);
            OutputText ("</td><td align=\"center\">");
            
            sprintf(pszProfile, "View the profile of %s", pszName);

            iErrCode = WriteProfileAlienString (
                iAlienKey,
                iAlienAddress,
                iDeadEmpireKey,
                pszName,
                0,
                "ProfileLink",
                pszProfile,
                true,
                iDeadEmpireKey != NO_KEY
                );
            RETURN_ON_ERROR(iErrCode);

            OutputText ("</td><td colspan=\"2\" align=\"center\">");

            iErrCode = t_pCache->ReadData(strGameEmpireData, iKey, GameNukedEmpires::Update, &vValue);
            RETURN_ON_ERROR(iErrCode);
            m_pHttpResponse->WriteText (vValue.GetInteger());

            OutputText ("</td><td colspan=\"2\" align=\"center\">");

            iErrCode = t_pCache->ReadData(strGameEmpireData, iKey, GameNukedEmpires::Reason, &vValue);
            RETURN_ON_ERROR(iErrCode);
            m_pHttpResponse->WriteText (REMOVAL_REASON_STRING [vValue.GetInteger()]);

            OutputText ("</td><td colspan=\"");
            m_pHttpResponse->WriteText (iNumTopHeaders - iNumHeaders);
            OutputText ("\">&nbsp;</td></tr><tr>");
        }
    }

    OutputText ("</table>");

    return iErrCode;
}

void HtmlRenderer::WriteCreateTournament (int iEmpireKey) {

    IHttpForm* pHttpForm;

    String strName, strDesc, strUrl;

    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentName")) != NULL)
    {
        HTMLFilter (pHttpForm->GetValue(), &strName, 0, false);
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentDescription")) != NULL)
    {
        HTMLFilter (pHttpForm->GetValue(), &strDesc, 0, false);
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("WebPageURL")) != NULL)
    {
        HTMLFilter (pHttpForm->GetValue(), &strUrl, 0, false);
    }

    // Name
    OutputText (
        "<p>"\
        "<table width=\"90%\">"\
        "<tr>"\
        "<td>Name:</td><td><input type=\"text\" size=\""
        );
    m_pHttpResponse->WriteText (MAX_TOURNAMENT_NAME_LENGTH);
    OutputText ("\" maxlength=\"");
    m_pHttpResponse->WriteText (MAX_TOURNAMENT_NAME_LENGTH);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strName.GetCharPtr(), strName.GetLength());
    OutputText ("\" name=\"TournamentName\"></td></tr>");
    
    // Description
    OutputText (
        "<tr>"\
        "<td>Description:</td><td>"\
        "<textarea rows=\"4\" cols=\"40\" wrap=\"virtual\" name=\"TournamentDescription\">"
        );

    m_pHttpResponse->WriteText (strDesc.GetCharPtr(), strDesc.GetLength());
        
    OutputText (
        "</textarea></td>"\
        "</tr>"\

        "<tr>"\
        "<td>Webpage:</td><td><input type=\"text\" size=\"40\" maxlength=\"");
    m_pHttpResponse->WriteText (MAX_WEB_PAGE_LENGTH);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strUrl.GetCharPtr(), strUrl.GetLength());
    OutputText (
        "\" name=\"WebPageURL\"></td></tr>"\

        "</table>"
        );
}

int HtmlRenderer::ProcessCreateTournament(int iEmpireKey, bool* pbCreated)
{
    *pbCreated = false;

    int iErrCode;
    unsigned int iTournamentKey;

    // Parse the forms
    Variant pvSubmitArray[SystemTournaments::NumColumns];
    bool bParsed;
    iErrCode = ParseCreateTournamentForms (pvSubmitArray, iEmpireKey, &bParsed);
    RETURN_ON_ERROR(iErrCode);

    if (!bParsed)
    {
        return OK;
    }

    // Create the tournament, finally
    iErrCode = CreateTournament (pvSubmitArray, &iTournamentKey);
    switch (iErrCode) {

    case OK:
        *pbCreated = true;
        AddMessage ("The tournament was created");
        break;
    case ERROR_TOURNAMENT_ALREADY_EXISTS:
        AddMessage ("The new tournament name already exists");
        break;
    case ERROR_NAME_IS_TOO_LONG:
        AddMessage ("The new tournament name is too long");
        break;
    case ERROR_DESCRIPTION_IS_TOO_LONG:
        AddMessage ("The new tournament description is too long");
        break;
    case ERROR_TOO_MANY_TOURNAMENTS:
        AddMessage ("The limit of personal tournaments has been exceeded");
        break;
    default:
       RETURN_ON_ERROR(iErrCode);
       break;
    }
    
    return OK;
}

int HtmlRenderer::ParseCreateTournamentForms(Variant* pvSubmitArray, int iEmpireKey, bool* pbParsed)
{
    *pbParsed = false;

    IHttpForm* pHttpForm;
    int iErrCode = OK;

    // Name
    pHttpForm = m_pHttpRequest->GetForm ("TournamentName");
    if (pHttpForm == NULL) {
        return ERROR_MISSING_FORM;
    }

    if (!VerifyCategoryName("Tournament", pHttpForm->GetValue(), MAX_TOURNAMENT_NAME_LENGTH, true))
    {
        return OK;
    }

    pvSubmitArray[SystemTournaments::iName] = pHttpForm->GetValue();

    // Description
    pHttpForm = m_pHttpRequest->GetForm ("TournamentDescription");
    if (pHttpForm == NULL) {
        return ERROR_MISSING_FORM;
    }

    if (String::StrLen (pHttpForm->GetValue()) > MAX_TOURNAMENT_DESCRIPTION_LENGTH) {
        AddMessage ("The description is too long");
        return OK;
    }

    pvSubmitArray[SystemTournaments::iDescription] = pHttpForm->GetValue();

    // URL
    pHttpForm = m_pHttpRequest->GetForm ("WebPageURL");
    if (pHttpForm == NULL) {
        return ERROR_MISSING_FORM;
    }

    if (String::StrLen (pHttpForm->GetValue()) > MAX_WEB_PAGE_LENGTH) {
        return ERROR_MISSING_FORM;
    }

    // Web page
    pvSubmitArray[SystemTournaments::iWebPage] = pHttpForm->GetValue();

    // Owner
    pvSubmitArray[SystemTournaments::iOwner] = iEmpireKey;

    // OwnerName
    if (iEmpireKey == SYSTEM)
    {
        pvSubmitArray[SystemTournaments::iOwnerName] = (const char*) NULL;
    }
    else
    {
        iErrCode = GetEmpireName (iEmpireKey, pvSubmitArray + SystemTournaments::iOwnerName);
        RETURN_ON_ERROR(iErrCode);
    }

    // Icon
    iErrCode = GetSystemProperty(SystemData::DefaultAlienKey, pvSubmitArray + SystemTournaments::iIcon);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemProperty(SystemData::DefaultAlienAddress, pvSubmitArray + SystemTournaments::iIconAddress);
    RETURN_ON_ERROR(iErrCode);

    // News
    pvSubmitArray[SystemTournaments::iNews] = (const char*) NULL;

    *pbParsed = true;
    return iErrCode;
}

int HtmlRenderer::WriteTournamentAdministrator (int iEmpireKey) {

    int iErrCode;
    unsigned int i, iNumTournaments;

    Variant* pvTournamentName = NULL;
    unsigned int* piTournamentKey = NULL;

    AutoFreeData free_pvTournamentName(pvTournamentName);
    AutoFreeKeys free_piTournamentKey(piTournamentKey);

    iErrCode = GetOwnedTournaments (iEmpireKey, &piTournamentKey, &pvTournamentName, &iNumTournaments);
    RETURN_ON_ERROR(iErrCode);

    const char* pszSwitch = iEmpireKey == SYSTEM ? "system" : "personal";

    if (iEmpireKey == SYSTEM) {

        OutputText ("<p>There ");
        if (iNumTournaments == 1) {
            OutputText ("is");
        } else {
            OutputText ("are");
        }

        if (iNumTournaments == 0) {
            OutputText (" no");
        } else {
            OutputText (" <strong>");
            m_pHttpResponse->WriteText (iNumTournaments);
            OutputText ("</strong>");
        }

        OutputText (" system tournament");

    } else {

        OutputText ("<p>You have <strong>");
        m_pHttpResponse->WriteText (iNumTournaments);
        OutputText ("</strong> personal tournament");
    }

    if (iNumTournaments != 1) {
        OutputText ("s");
    }

    if (iEmpireKey == SYSTEM) {
        OutputText (" on the server:");
    }

    OutputText (
        
        "<p><table width=\"75%\">"\

    // Create tournament
        "<tr>"\
        "<td>"\
        "Create a new "
        );

    m_pHttpResponse->WriteText (pszSwitch);

    OutputText (
        " tournament:"\
        "</td><td>"
        );

    WriteButton (BID_CREATENEWTOURNAMENT);
    
    OutputText (
        "</td>"\
        "</tr>"
        );

    if (iNumTournaments > 0)
    {
        // Delete tournament
        OutputText (
            "<tr>"\
            "<td>"\
            "Administer a "
            );

        m_pHttpResponse->WriteText (pszSwitch);

        OutputText (
            " tournament:"\
            "</td><td>"\

            "<select name=\"AdminTournament\">"
            );

        for (i = 0; i < iNumTournaments; i ++) {
            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (piTournamentKey[i]);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pvTournamentName[i].GetCharPtr());
            OutputText ("</option>");
        }

        OutputText ("</select> ");

        WriteButton (BID_ADMINISTERTOURNAMENT);
        OutputText (
            "</td>"\
            "</tr>"\

        // Administer tournament
            "<tr>"\
            "<td>"\
            "Delete a "
            );

        m_pHttpResponse->WriteText (pszSwitch);

        OutputText (
            " tournament:"\
            "</td><td>"\

            "<select name=\"DelTournament\">"\
            );

        for (i = 0; i < iNumTournaments; i ++) {
            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (piTournamentKey[i]);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pvTournamentName[i].GetCharPtr());
            OutputText ("</option>")
        }

        OutputText ("</select> ");

        WriteButton (BID_DELETETOURNAMENT);
        
        OutputText (
            "</td>"\
            "</tr>"
            );
    }

    OutputText ("</table>");
    
    return iErrCode;
}

int HtmlRenderer::WriteIconSelection(int iIconSelect, unsigned int iAlienKey, const char* pszCategory)
{
    int iErrCode = OK;

    Variant** ppvData = NULL;
    AutoFreeData free_ppvData(ppvData);

    unsigned int* piKey = NULL;
    AutoFreeKeys free_piKey(piKey);

    OutputText ("<p><h3>Choose an icon for your ");
    m_pHttpResponse->WriteText (pszCategory);
    
    OutputText (":</h3>");

    switch (iIconSelect) {
    case 0:

        unsigned int iNumAliens;
        iErrCode = GetAliens(&ppvData, &piKey, &iNumAliens);
        RETURN_ON_ERROR(iErrCode);

        OutputText (
            "<input type=\"hidden\" name=\"WhichAlien\" value=\"0\">"\
            "<p><table width=\"75%\"><tr><td>"
            );

        for (unsigned int i = 0; i < iNumAliens; i ++)
        {
            WriteAlienButtonString(piKey[i], ppvData[i][SystemAlienIcons::iAddress], piKey[i] == iAlienKey, "Alien", ppvData[i][SystemAlienIcons::iAuthorName]);
            OutputText (" ");
        }

        OutputText ("</td></tr></table><p>");

        WriteButton (BID_CANCEL);
        break;

    case 1:

        // Icon upload form
        OutputText (
            "<input type=\"hidden\" name=\"WhichAlien\" value=\"1\">"\
            "<p>"\
            "<table width=\"60%\">"\
            "<tr>"\
            "<td>"\
            "In order to be accepted, the size of your icon must be less than 10KB and it must be a correctly "\
            "formatted 40x40 transparent .gif (GIF89a). The icon that you upload will overwrite any "
            "previous icons you might have uploaded to the server."\
            "<center>"\
            "<p>"\
            "<input type=\"file\" name=\"IconFile\" size=\"40\">"\
            "</td>"\
            "</tr>"\
            "</table>"\

            "<p>"
            );

        WriteButton (BID_CANCEL);
        WriteButton (BID_CHOOSE);

        break;

    default:
        break;
    }

    return iErrCode;
}

int HtmlRenderer::HandleIconSelection(unsigned int* piAlienKey, const char* pszUploadDir, unsigned int iKey1, unsigned int iKey2, bool* pbHandled)
{
    *pbHandled = false;

    IHttpForm* pHttpForm;
    if ((pHttpForm = m_pHttpRequest->GetForm ("WhichAlien")) == NULL)
    {
        return ERROR_MISSING_FORM;
    }

    if (pHttpForm->GetUIntValue() == 1)
    {
        if (!WasButtonPressed (BID_CHOOSE))
        {
            return ERROR_MISSING_FORM;
        }
            
        // Icon uploads
        if ((pHttpForm = m_pHttpRequest->GetForm ("IconFile")) == NULL)
        {
            return ERROR_MISSING_FORM;
        }
        
        const char* pszFileName = pHttpForm->GetValue();
        if (pszFileName == NULL)
        {
            AddMessage ("No file was selected");
            return OK;
        }
            
        bool bGoodGIF;
        int iErrCode = VerifyGIF(pszFileName, &bGoodGIF);
        RETURN_ON_ERROR(iErrCode);
        
        if (!bGoodGIF)
        {
            return OK;
        }

        // The gif was OK, so copy it to its destination
        if (!CopyUploadedIcon(pszFileName, pszUploadDir, iKey1, iKey2))
        {
            AddMessage ("The file was uploaded, but could not be copied. Contact the administrator");
            return OK;
        }

        *piAlienKey = UPLOADED_ICON;
        AddMessage ("Your new icon was uploaded successfully");
    }
    else
    {
        const char* pszStart;
        if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith("Alien")) == NULL ||
            (pszStart = pHttpForm->GetName()) == NULL || sscanf (pszStart, "Alien%d", piAlienKey) != 1)
        {
            return OK;
        }
    }

    *pbHandled = true;
    return OK;
}

int HtmlRenderer::WriteActiveGameAdministration(const Variant** ppvGames, unsigned int iNumActiveGames, unsigned int iNumOpenGames, unsigned int iNumClosedGames, bool bAdmin)
{
    int* piGameClass = (int*)StackAlloc(iNumActiveGames * sizeof(int));
    int* piGameNumber = (int*)StackAlloc(iNumActiveGames * sizeof(int));

    for (unsigned int i = 0; i < iNumActiveGames; i ++)
    {
        piGameClass[i] = ppvGames[i][0].GetInteger();
        piGameNumber[i] = ppvGames[i][1].GetInteger();
    }

    if (iNumActiveGames == 0)
    {
        OutputText ("<p>There are no active games");
        if (bAdmin)
        {
            OutputText (" on the server");
        }
        return OK;
    }

    const char* pszTableColor = m_vTableColor.GetCharPtr();
    size_t stLen = strlen (pszTableColor);

    OutputText ("<p>There ");
    if (iNumActiveGames == 1) { 
        OutputText ("is <strong>1</strong> active game");
    } else { 
        OutputText ("are <strong>");
        m_pHttpResponse->WriteText (iNumActiveGames);
        OutputText ("</strong> active games");
    }

    if (bAdmin) {

        OutputText (" (<strong>");
        m_pHttpResponse->WriteText (iNumOpenGames);
        OutputText ("</strong> open, <strong>");
        m_pHttpResponse->WriteText (iNumClosedGames); 
        OutputText ("</strong> closed):");
    } else {
        OutputText (":");
    }

    OutputText ("<p><table width=\"90%\"><tr>");
    OutputText ("<th align=\"center\" bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stLen);
    OutputText ("\"><strong>Game</strong></th>");
    OutputText ("<th align=\"center\" bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stLen);
    OutputText ("\"><strong>Update period</strong></th>");
    OutputText ("<th align=\"center\" bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stLen);
    OutputText ("\"><strong>Updates</strong></th>");
    OutputText ("<th align=\"center\" bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stLen);
    OutputText ("\"><strong>Started</strong></th>");
    OutputText ("<th align=\"center\" bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stLen);
    OutputText ("\"><strong>Time remaining</strong></th>");
    OutputText ("<th align=\"center\" bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stLen);
    OutputText ("\"><strong>State</strong></th>");

    if (bAdmin) {
        OutputText ("<th align=\"center\" bgcolor=\"#");
        m_pHttpResponse->WriteText (pszTableColor, stLen);
        OutputText ("\"><strong>Password</strong></th>");
    }
    
    OutputText ("<th align=\"center\" bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stLen);
    OutputText ("\"><strong>Empires</strong></th>");
    OutputText ("<th align=\"center\" bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stLen);
    OutputText ("\"><strong>Administer Game</strong></th></tr>");

    Variant vGamePasswordHash, vName;
    unsigned int iNumActiveEmpires;
    int iNumUpdates;
    bool bPaused, bOpen, bStarted;

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    // Sort games by gameclass
    Algorithm::QSortTwoAscending<int, int> (piGameClass, piGameNumber, iNumActiveGames);

    // Sort games by gamenumber
    int iBegin = 0, iNumToSort;
    int iCurrentGameClass = piGameClass[0];

    for (unsigned int i = 1; i < iNumActiveGames; i ++)
    {
        if (piGameClass[i] != iCurrentGameClass)
        {
            iNumToSort = i - iBegin;
            if (iNumToSort > 1) {
                Algorithm::QSortTwoAscending<int, int> (piGameNumber + iBegin, piGameClass + iBegin, iNumToSort);
            }

            iBegin = i;
            iCurrentGameClass = piGameClass[i];
        }
    }

    iNumToSort = iNumActiveGames - iBegin;
    if (iNumToSort > 1) {
        Algorithm::QSortTwoAscending<int, int> (piGameNumber + iBegin, piGameClass + iBegin, iNumToSort);
    }
    iCurrentGameClass = piGameClass[0];

    bool bExists, bAdminPaused;
    Seconds iSeconds, iSecondsSince, iSecondsUntil;
    int iGameState;
    UTCTime tCreationTime;
    int iErrCode = OK;

    char pszAdmin [192];

    int iSeparatorAddress;
    iErrCode = GetThemeAddress(m_iSeparatorKey, &iSeparatorAddress);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CacheGameData(piGameClass, piGameNumber, NO_KEY, iNumActiveGames);
    RETURN_ON_ERROR(iErrCode);

    for (unsigned int i = 0; i < iNumActiveGames; i ++)
    {
        bool bUpdate;
        iErrCode = CheckGameForUpdates (piGameClass[i], piGameNumber[i], &bUpdate);
        RETURN_ON_ERROR(iErrCode);

        if (bUpdate)
        {
            iErrCode = DoesGameExist (piGameClass[i], piGameNumber[i], &bExists);
            RETURN_ON_ERROR(iErrCode);
        
            if (!bExists)
                continue;
        }

        iErrCode = GetGameClassName (piGameClass[i], pszGameClassName);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetGameClassUpdatePeriod (piGameClass[i], &iSeconds);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetGameProperty(piGameClass[i], piGameNumber[i], GameData::PasswordHash, &vGamePasswordHash);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetGameCreationTime (piGameClass[i], piGameNumber[i], &tCreationTime);
        RETURN_ON_ERROR(iErrCode);

        Variant** ppvEmpiresInGame = NULL;
        AutoFreeData free_ppvEmpiresInGame(ppvEmpiresInGame);

        iErrCode = GetEmpiresInGame(piGameClass[i], piGameNumber[i], &ppvEmpiresInGame, &iNumActiveEmpires);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetGameUpdateData (piGameClass[i], piGameNumber[i], &iSecondsSince, &iSecondsUntil, &iNumUpdates, &iGameState);
        RETURN_ON_ERROR(iErrCode);

        bPaused = (iGameState & PAUSED) || (iGameState & ADMIN_PAUSED);
        bAdminPaused = (iGameState & ADMIN_PAUSED) != 0;
        bOpen = (iGameState & STILL_OPEN) != 0;
        bStarted = (iGameState & STARTED) != 0;

        if (i > 0 && piGameClass[i] != iCurrentGameClass)
        {
            iCurrentGameClass = piGameClass[i];
            OutputText ("<tr><td align=\"center\" colspan=\"9\">");
            WriteSeparatorString(m_iSeparatorKey, iSeparatorAddress);
            OutputText ("</td></tr>");
        }

        WriteGameAdministratorGameData (pszGameClassName, piGameNumber[i], iSeconds, iSecondsUntil, 
            iNumUpdates, bOpen, bPaused, bAdminPaused, bStarted, 
            vGamePasswordHash.GetCharPtr(), ppvEmpiresInGame, iNumActiveEmpires, tCreationTime, bAdmin);

        OutputText ("<td align=\"center\">"); 

        sprintf(pszAdmin, "AdministerGame%i.%i", piGameClass[i], piGameNumber[i]);

        WriteButtonString (
            m_iButtonKey,
            m_iButtonAddress,
            "AdministerGame",
            "Administer Game", 
            pszAdmin
            );

        OutputText ("</td></tr>");
    }

    OutputText ("</table>");

    return iErrCode;
}

int HtmlRenderer::WriteAdministerGame(int iGameClass, int iGameNumber, bool bAdmin)
{
    bool bStarted, bExists, bPaused, bOpen, bAdminPaused;
    int iErrCode, iNumUpdates, iGameState;
    unsigned int i, iNumActiveEmpires;
    Seconds iSeconds, iSecondsUntil, iSecondsSince;
    UTCTime tCreationTime;

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = CacheGameData(&iGameClass, &iGameNumber, NO_KEY, 1);
    RETURN_ON_ERROR(iErrCode);

    bool bUpdate;
    iErrCode = CheckGameForUpdates(iGameClass, iGameNumber, &bUpdate);
    RETURN_ON_ERROR(iErrCode);

    if (bUpdate)
    {
        iErrCode = DoesGameExist (iGameClass, iGameNumber, &bExists);
        RETURN_ON_ERROR(iErrCode);
        
        if (!bExists)
        {
            OutputText ("<p>The game could not be administered. It may no longer exist<p>");
            WriteButton (BID_CANCEL);
            return OK;
        }
    }

    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetGameClassUpdatePeriod (iGameClass, &iSeconds);
    RETURN_ON_ERROR(iErrCode);
       
    Variant** ppvEmpiresInGame = NULL;
    AutoFreeData free_ppvEmpiresInGame(ppvEmpiresInGame);

    iErrCode = GetEmpiresInGame (iGameClass, iGameNumber, &ppvEmpiresInGame, &iNumActiveEmpires);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetGameCreationTime (iGameClass, iGameNumber, &tCreationTime);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetGameUpdateData (iGameClass, iGameNumber, &iSecondsSince, &iSecondsUntil, &iNumUpdates, &iGameState);
    RETURN_ON_ERROR(iErrCode);

    bPaused = (iGameState & PAUSED) || (iGameState & ADMIN_PAUSED);
    bAdminPaused = (iGameState & ADMIN_PAUSED) != 0;
    bOpen = (iGameState & STILL_OPEN) != 0;
    bStarted = (iGameState & STARTED) != 0;

    OutputText ("<input type=\"hidden\" name=\"GameClass\" value=\"");
    m_pHttpResponse->WriteText (iGameClass);
    OutputText ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"");
    m_pHttpResponse->WriteText (iGameNumber);
    OutputText (
        "\">"\
        "<p><h3>Administer "
        );
    m_pHttpResponse->WriteText (pszGameClassName);
    OutputText (" ");
    m_pHttpResponse->WriteText (iGameNumber);
    OutputText (":</h3>");
    OutputText ("<p>");

    // Read game class data
    Variant* pvGameClassInfo = NULL;
    AutoFreeData free_pvGameClassInfo(pvGameClassInfo);

    iErrCode = GetGameClassData (iGameClass, &pvGameClassInfo);
    RETURN_ON_ERROR(iErrCode);

    OutputText ("<p>");
    WriteGameAdministratorListHeader (m_vTableColor.GetCharPtr());

    iErrCode = WriteGameAdministratorListData (iGameClass, iGameNumber, pvGameClassInfo);
    RETURN_ON_ERROR(iErrCode);

    OutputText ("</table>");

    int iSeparatorAddress;
    iErrCode = GetThemeAddress(m_iSeparatorKey, &iSeparatorAddress);
    RETURN_ON_ERROR(iErrCode);

    OutputText ("<p>");
    WriteSeparatorString(m_iSeparatorKey, iSeparatorAddress);
    OutputText ("<p><table width=\"90%\">");

    // View Map
    if (iGameState & GAME_MAP_GENERATED) {
        OutputText ("<tr><td>View the game's map:</td><td>");
        WriteButton (BID_VIEWMAP);
        OutputText ("</td></tr>");
    }

    // Change game password
    if (bAdmin)
    {
        Variant vGamePasswordHash;
        iErrCode = GetGameProperty(iGameClass, iGameNumber, GameData::PasswordHash, &vGamePasswordHash);
        RETURN_ON_ERROR(iErrCode);

        if (!String::IsBlank(vGamePasswordHash))
        {
            OutputText (
                "<tr><td>Change the game's password:</td>"\
                "<td><input type=\"text\" name=\"NewPassword\" size=\""
                );
            m_pHttpResponse->WriteText (MAX_GAME_PASSWORD_LENGTH);
            OutputText ("\" maxlength=\"");
            m_pHttpResponse->WriteText (MAX_GAME_PASSWORD_LENGTH);
            OutputText ("\"> ");
            WriteButton (BID_CHANGEPASSWORD);
            OutputText ("</td></tr>");
        }
        else
        {
            OutputText (
                "<tr><td>Password protect the game:</td>"\
                "<td><input type=\"text\" name=\"NewPassword\" size=\""
                );
            m_pHttpResponse->WriteText (MAX_GAME_PASSWORD_LENGTH);
            OutputText ("\" maxlength=\"");
            m_pHttpResponse->WriteText (MAX_GAME_PASSWORD_LENGTH);
            OutputText ("\"> ");
            WriteButton (BID_CHANGEPASSWORD);
            OutputText ("</td></tr>");
        }
    }

    // Force game update or pause game
    if (bStarted) {

        OutputText ("<tr><td>Force the game to update:</td><td>");
        WriteButton (BID_FORCEUPDATE);
        OutputText ("</td></tr>");

        if (bAdminPaused) {
            OutputText ("<tr><td>Unpause the game:</td><td>");
            WriteButton (BID_UNPAUSEGAME);
            OutputText ("</td></tr>");
        } else {
            OutputText ("<tr><td>Pause the game:</td><td>");
            WriteButton (BID_PAUSEGAME);
            OutputText ("</td></tr>");
        }

        if (!bPaused) {

            OutputText ("<tr><td>Reset update time:</td><td>");
            WriteButton (BID_RESET);
            OutputText ("</td></tr>");
        }
    }

    // View empire info
    OutputText ("<tr><td>View empire information:</td><td>");
    WriteButton (BID_VIEWEMPIREINFORMATION);
    OutputText ("</td></tr>");

    if (bAdmin) {

        OutputText ("<tr><td>Search for empires with the same IP address:</td><td>");
        WriteButton (BID_SEARCHIPADDRESSES);
        OutputText ("</td></tr>");

        OutputText ("<tr><td>Search for empires with the same Session Id:</td><td>");
        WriteButton (BID_SEARCHSESSIONIDS);
        OutputText ("</td></tr>");
    }

    unsigned int iNumResigned, * piResignedKey = NULL;
    Algorithm::AutoDelete<unsigned int> free_piResignedKey(piResignedKey, true);

    iErrCode = GetResignedEmpiresInGame (iGameClass, iGameNumber, &piResignedKey, &iNumResigned);
    RETURN_ON_ERROR(iErrCode);

    if (iNumResigned > 0)
    {
        OutputText ("<tr><td>Restore a resigned empire to the game:</td><td><select name=\"RestoreEmpireKey\">");

        for (i = 0; i < iNumResigned; i ++)
        {
            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (piResignedKey[i]);
            OutputText ("\">");

            for (unsigned int j = 0; j < iNumActiveEmpires; j ++)
            {
                if (piResignedKey[i] == (unsigned int)ppvEmpiresInGame[j][GameEmpires::iEmpireKey].GetInteger())
                {
                    m_pHttpResponse->WriteText(ppvEmpiresInGame[j][GameEmpires::iEmpireName].GetCharPtr());
                    break;
                }
            }
            
            OutputText ("</option>");
        }

        OutputText (" ");
        WriteButton (BID_RESTOREEMPIRE);
        OutputText ("</td></tr>");
    }

    OutputText ("<tr><td>Delete an empire from the game:</td><td><select name=\"DeleteEmpireKey\">");

    for (i = 0; i < iNumActiveEmpires; i ++)
    {
        OutputText ("<option value=\"");
        m_pHttpResponse->WriteText(ppvEmpiresInGame[i][GameEmpires::iEmpireKey].GetInteger());
        OutputText ("\">");
        m_pHttpResponse->WriteText(ppvEmpiresInGame[i][GameEmpires::iEmpireName].GetCharPtr());
        OutputText ("</option>");
    }
    OutputText (" ");
    WriteButton (BID_DELETEEMPIRE);
    OutputText ("</td></tr>");

    // Broadcast message to all empires
    OutputText (
        "<tr><td>Broadcast a message to all empires in the game:</td>"\
        "<td><textarea name=\"Message\" rows=\"5\" cols=\"45\" wrap=\"virtual\"></textarea></td><td>"
        );
    WriteButton (BID_SENDMESSAGE);
    OutputText (
        "</td>"\
        "</tr>"

    // Kill game
        "<tr><td>Kill the game:</td><td>"
            );

    WriteButton (BID_KILLGAME);

    OutputText (
        "</td>"\
        "</tr>"\
        "</table>"
        );

    return iErrCode;
}

int HtmlRenderer::RenderEmpire (unsigned int iTournamentKey, int iEmpireKey)
{
    int iAlienKey, iErrCode;
    char pszName [MAX_EMPIRE_NAME_LENGTH + 1];
    char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];

    Variant* pvTournamentEmpireData = NULL, vTemp;
    AutoFreeData free_pvTournamentEmpireData(pvTournamentEmpireData);

    iErrCode = GetEmpireName (iEmpireKey, pszName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::AlienKey, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iAlienKey = vTemp.GetInteger();

    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::AlienAddress, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iAlienAddress = vTemp.GetInteger();

    iErrCode = GetTournamentEmpireData (iTournamentKey, iEmpireKey, &pvTournamentEmpireData);
    RETURN_ON_ERROR(iErrCode);

    int iOptions;
    iErrCode = GetEmpireOptions2(iEmpireKey, &iOptions);
    RETURN_ON_ERROR(iErrCode);

    bool bAvailable = !(iOptions & UNAVAILABLE_FOR_TOURNAMENTS);

    // Icon
    OutputText("<tr><td align=\"center\">");

    sprintf(pszProfile, "View the profile of %s", pszName);

    iErrCode = WriteProfileAlienString (
        iAlienKey,
        iAlienAddress,
        iEmpireKey,
        pszName,
        0, 
        "ProfileLink",
        pszProfile,
        false,
        true
        );
    RETURN_ON_ERROR(iErrCode);
                
    NotifyProfileLink();
    
    OutputText (
        "</td>"\
        
    // Name
        "<td align=\"center\">"
        );

    if (!bAvailable)
        OutputText("<strike>");

    m_pHttpResponse->WriteText(pszName);

    if (!bAvailable)
        OutputText("</strike>");

    OutputText (
        "</td>"\

    // Wins
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvTournamentEmpireData[SystemTournamentEmpires::iWins].GetInteger());
    
    OutputText (
        "</td>"\

    // Nukes
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvTournamentEmpireData[SystemTournamentEmpires::iNukes].GetInteger());
    
    OutputText (
        "</td>"\

    // Nuked
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvTournamentEmpireData[SystemTournamentEmpires::iNuked].GetInteger());
    
    OutputText (
        "</td>"\

    // Draws
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvTournamentEmpireData[SystemTournamentEmpires::iDraws].GetInteger());
    
    OutputText (
        "</td>"\

    // Ruins
        "<td align=\"center\">"
        );

    m_pHttpResponse->WriteText (pvTournamentEmpireData[SystemTournamentEmpires::iRuins].GetInteger());
    
    OutputText("</td><td>");

    if (!bAvailable)
        OutputText("Unavailable");

    OutputText("</td></tr>");

    return iErrCode;
}