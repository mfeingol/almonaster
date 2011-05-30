//
// Almonaster.dll:  a component of Almonaster 2.0
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "HtmlRenderer.h"

#include "Osal/File.h"
#include "Osal/Algorithm.h"
#include "Osal/TempFile.h"

#include <stdio.h>
#include <math.h>

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
ReadWriteLock HtmlRenderer::ms_mNewsFileLock;
ReadWriteLock HtmlRenderer::ms_mIntroUpperFileLock;
ReadWriteLock HtmlRenderer::ms_mIntroLowerFileLock;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


HtmlRenderer::HtmlRenderer (PageId pageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {

    m_pHttpRequest = pHttpRequest;
    m_pHttpResponse = pHttpResponse;
    
    m_pgPageId = pageId;
    
    m_vEmpireName = (const char*) NULL;
    m_vPassword = (const char*) NULL;
    m_vPreviousIPAddress = (const char*) NULL;

    m_i64SecretKey = 0;

    m_tOldSalt = NULL_TIME;
    Time::GetTime (&m_tNewSalt);
    
    *m_pszGameClassName = '\0';
    
    m_iEmpireKey = NO_KEY;
    m_iGameClass = NO_KEY;
    m_iGameNumber = -1;
    m_iButtonKey = NO_KEY;
    m_iBackgroundKey = NO_KEY;
    m_iSeparatorKey = NO_KEY;
    m_iPrivilege = NOVICE;
    m_iAlienKey = NO_KEY;
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
    m_bLoggedIn = false;
    m_bAuthenticated = false;
    
    m_sSecondsUntil = 0;
    m_sSecondsSince = 0;
    Time::StartTimer (&m_tmTimer);

    m_bNotifiedProfileLink = false;
    m_bNotifiedTournamentInvitation = false;
    m_bNotifiedTournamentJoinRequest = false;
    
    m_i64SessionId = NO_SESSION_ID;

    m_iNumOldUpdates = 0;

    m_pgeLock = NULL;
}

HtmlRenderer::~HtmlRenderer() {

    Assert (m_pgeLock == NULL);
}

int HtmlRenderer::Initialize() {

    int iErrCode;

    iErrCode = ms_mNewsFileLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = ms_mIntroUpperFileLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }
    iErrCode = ms_mIntroLowerFileLock.Initialize();
    if (iErrCode != OK) {
        return iErrCode;
    }

    return OK;
}

int HtmlRenderer::Render() {
    
    Assert (m_pgPageId > MIN_PAGE_ID && m_pgPageId < MAX_PAGE_ID);
    return g_pfxnRenderPage[m_pgPageId] (this);
}

int HtmlRenderer::Redirect (PageId pageId) {
    
    Assert (pageId > MIN_PAGE_ID && pageId < MAX_PAGE_ID);
    
    m_bRedirection = true;
    m_pgPageId = pageId;
    
    // Best effort
    m_pHttpResponse->Clear();
    
    // Call the function
    return g_pfxnRenderPage[pageId] (this);
}

void HtmlRenderer::ShutdownServer () {
    
    g_pHttpServer->Shutdown();
    AddMessage ("The server will shut down now");
}

void HtmlRenderer::RestartServer () {
    
    g_pHttpServer->Restart (NULL);
    AddMessage ("The server will now restart");
}

void HtmlRenderer::RestartAlmonaster () {
    
    g_pPageSourceControl->Restart();
    AddMessage ("Almonaster will now restart");
}

bool HtmlRenderer::IsGamePage (PageId pageId) {
    
    return (pageId >= INFO && pageId <= QUIT);
}

void HtmlRenderer::WriteBackgroundImageSrc (int iThemeKey) {
    
    // No Null Theme background image
    OutputText (BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iThemeKey);
    OutputText ("/" BACKGROUND_IMAGE);
}

void HtmlRenderer::WriteLivePlanetImageSrc (int iThemeKey) {
    
    if (iThemeKey == NULL_THEME) {
        OutputText (BASE_RESOURCE_DIR LIVE_PLANET_NAME);
    } else {
        OutputText (BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iThemeKey);
        OutputText ("/" LIVE_PLANET_NAME);
    }
}

void HtmlRenderer::WriteDeadPlanetImageSrc (int iThemeKey) {
    
    if (iThemeKey == NULL_THEME) {
        OutputText (BASE_RESOURCE_DIR DEAD_PLANET_NAME);
    } else {
        OutputText (BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iThemeKey);
        OutputText ("/" DEAD_PLANET_NAME);
    }
}

void HtmlRenderer::WriteSeparatorSrc (int iThemeKey) {
    
    // No Null Theme separator
    
    OutputText (BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iThemeKey);
    OutputText ("/" SEPARATOR_IMAGE);
}

void HtmlRenderer::WriteHorzSrc (int iThemeKey) {
    
    if (iThemeKey == NULL_THEME) {
        OutputText (BASE_RESOURCE_DIR HORZ_LINE_NAME);
    } else {
        OutputText (BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iThemeKey);
        OutputText ("/" HORZ_LINE_NAME);
    }
}

void HtmlRenderer::WriteVertSrc (int iThemeKey) {
    
    if (iThemeKey == NULL_THEME) {
        OutputText (BASE_RESOURCE_DIR VERT_LINE_NAME);
    } else {
        OutputText (BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iThemeKey);
        OutputText ("/" VERT_LINE_NAME);
    }
}

int HtmlRenderer::GetHorzString (int iThemeKey, String* pstrString, bool bBlowup) {
    
    switch (iThemeKey) {
        
    case NULL_THEME:
        
        if (bBlowup) {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR HORZ_LINE_NAME "\">";
        }
        break;
        
    case ALTERNATIVE_PATH:
        
        *pstrString = "<img src=\"";
        *pstrString += m_vLocalPath.GetCharPtr();
        
        if (bBlowup) {
            *pstrString += "/" HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString += "/" HORZ_LINE_NAME "\">";
        }
        break;
        
    default:
        
        if (bBlowup) {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR;
            *pstrString += iThemeKey;
            *pstrString += "/" HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR;
            *pstrString += iThemeKey;
            *pstrString += "/" HORZ_LINE_NAME "\">";
        }
        break;
    }
    
    if (pstrString->GetCharPtr() == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    return OK;
}

int HtmlRenderer::GetVertString (int iThemeKey, String* pstrString, bool bBlowup) {
    
    switch (iThemeKey) {
        
    case NULL_THEME:
        
        if (bBlowup) {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR VERT_LINE_NAME "\">";
        }
        break;
        
    case ALTERNATIVE_PATH:
        
        *pstrString = "<img src=\"";
        *pstrString += m_vLocalPath.GetCharPtr();
        
        if (bBlowup) {
            *pstrString += "/" VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString += "/" VERT_LINE_NAME "\">";
        }
        break;
        
    default:
        
        *pstrString = "<img src=\"" BASE_RESOURCE_DIR;
        *pstrString += iThemeKey;
        
        if (bBlowup) {
            *pstrString += "/" VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString += "/" VERT_LINE_NAME "\">";
        }
        break;
    }
    
    if (pstrString->GetCharPtr() == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    return OK;
}


int HtmlRenderer::GetUIData (int iThemeKey) {
    
    int iErrCode = OK;
    Variant vValue;
    
    if (iThemeKey == INDIVIDUAL_ELEMENTS) {
        
        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIButtons, &vValue);
        if (iErrCode != OK) {
            return iErrCode;
        }
        m_iButtonKey = vValue.GetInteger();
        
        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIBackground, &vValue);
        if (iErrCode != OK) {
            return iErrCode;
        }
        m_iBackgroundKey = vValue.GetInteger();
        
        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UISeparator, &vValue);
        if (iErrCode != OK) {
            return iErrCode;
        }
        m_iSeparatorKey = vValue.GetInteger();

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue);
        if (iErrCode != OK) {
            return iErrCode;
        }
        
        iErrCode = GetTextColorData (vValue.GetInteger());
        if (iErrCode != OK) {
            return iErrCode;
        }
        
    } else {
        
        m_iButtonKey = iThemeKey;
        m_iBackgroundKey = iThemeKey;
        m_iSeparatorKey = iThemeKey;
        
        if (iThemeKey == NULL_THEME) {
            
            iErrCode = GetTextColorData (NULL_THEME);
            if (iErrCode != OK) {
                return iErrCode;
            }
        }
        
        else if (iThemeKey == ALTERNATIVE_PATH) {

            iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue);
            if (iErrCode != OK) {
                return iErrCode;
            }
            
            iErrCode = GetTextColorData (vValue.GetInteger());
            if (iErrCode != OK) {
                return iErrCode;
            }

            iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlternativeGraphicsPath, &m_vLocalPath);
            if (iErrCode != OK) {
                return iErrCode;
            }
        }
        
        else {
            
            iErrCode = GetTextColorData (iThemeKey);
            if (iErrCode != OK) {
                return iErrCode;
            }
        }
    }
    
    return iErrCode;
}


int HtmlRenderer::GetTextColorData (int iEmpireColorKey) {
    
    int iErrCode;
    
    switch (iEmpireColorKey) {
        
    case NULL_THEME:
        
        m_vTableColor = DEFAULT_TABLE_COLOR;
        m_vTextColor = DEFAULT_TEXT_COLOR;
        m_vGoodColor = DEFAULT_GOOD_COLOR;
        m_vBadColor = DEFAULT_BAD_COLOR;
        m_vPrivateMessageColor = DEFAULT_PRIVATE_MESSAGE_COLOR;
        m_vBroadcastMessageColor = DEFAULT_BROADCAST_MESSAGE_COLOR;
        break;
        
    case CUSTOM_COLORS:

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTableColor, &m_vTableColor);
        if (iErrCode != OK) {
            return iErrCode;
        }

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTextColor, &m_vTextColor);
        if (iErrCode != OK) {
            return iErrCode;
        }

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomGoodColor, &m_vGoodColor);
        if (iErrCode != OK) {
            return iErrCode;
        }

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBadColor, &m_vBadColor);
        if (iErrCode != OK) {
            return iErrCode;
        }

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomPrivateMessageColor, &m_vPrivateMessageColor);
        if (iErrCode != OK) {
            return iErrCode;
        }

        iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBroadcastMessageColor, &m_vBroadcastMessageColor);
        if (iErrCode != OK) {
            return iErrCode;
        }

        break;
        
    default:
        
        iErrCode = g_pGameEngine->GetThemeTextColor (iEmpireColorKey, &m_vTextColor);
        if (iErrCode != OK) {
            return iErrCode;
        }
        
        iErrCode = g_pGameEngine->GetThemeGoodColor (iEmpireColorKey, &m_vGoodColor);
        if (iErrCode != OK) {
            return iErrCode;
        }
        
        iErrCode = g_pGameEngine->GetThemeBadColor (iEmpireColorKey, &m_vBadColor);
        if (iErrCode != OK) {
            return iErrCode;
        }
        
        iErrCode = g_pGameEngine->GetThemePrivateMessageColor (iEmpireColorKey, &m_vPrivateMessageColor);
        if (iErrCode != OK) {
            return iErrCode;
        }
        
        iErrCode = g_pGameEngine->GetThemeBroadcastMessageColor (iEmpireColorKey, &m_vBroadcastMessageColor);
        if (iErrCode != OK) {
            return iErrCode;
        }
        
        iErrCode = g_pGameEngine->GetThemeTableColor (iEmpireColorKey, &m_vTableColor);
        if (iErrCode != OK) {
            return iErrCode;
        }
        break;
    }
    
    return OK;
}

bool HtmlRenderer::IsColor (const char* pszColor) {
    
    if (String::StrLen (pszColor) != MAX_COLOR_LENGTH) {
        return false;
    }
    
    int i;
    char szChar;
    
    for (i = 0; i < MAX_COLOR_LENGTH; i ++) {
        
        szChar = pszColor[i];
        
        if (!(szChar >= '0' && szChar <= '9') &&
            !(szChar >= 'a' && szChar <= 'f') && 
            !(szChar >= 'A' && szChar <= 'F')
            ) {
            return false;
        }
    }
    
    return true;
}


void HtmlRenderer::WriteButtonImageSrc (int iRealThemeKey, const char* pszButtonName) {
    
    m_pHttpResponse->WriteText (BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iRealThemeKey);
    OutputText ("/");
    m_pHttpResponse->WriteText (pszButtonName);
    m_pHttpResponse->WriteText (DEFAULT_IMAGE_EXTENSION);
}

void HtmlRenderer::WriteThemeDownloadSrc (int iRealThemeKey, const char* pszFileName) {
    
    m_pHttpResponse->WriteText (BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iRealThemeKey);
    OutputText ("/");
    m_pHttpResponse->WriteText (pszFileName);
}

int HtmlRenderer::StandardizeEmpireName (const char* pszName, char pszFinalName[MAX_EMPIRE_NAME_LENGTH + 1]) {
    
    char pszCopy [MAX_EMPIRE_NAME_LENGTH + 1];
    strcpy (pszCopy, pszName);
    
    char* pszNameCopy = pszCopy;
    
    // Remove beginning spaces
    size_t i = 0, iLen = strlen (pszNameCopy);
    while (i < iLen && pszNameCopy[i] == ' ') {
        pszNameCopy ++;
        iLen --;
    }
    
    if (i == iLen) {
        AddMessage ("An empire name must contain more than just spaces");
        return ERROR_FAILURE;
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
        return ERROR_FAILURE;
    }
    
    // Name is valid
    strcpy (pszFinalName, pszNameCopy);
    
    return OK;
}

int HtmlRenderer::VerifyPassword (const char* pszPassword, bool bPrintErrors) {
    
    if (pszPassword == NULL || *pszPassword == '\0') {
        if (bPrintErrors) {
            AddMessage ("Passwords cannot be blank");
        }
        return ERROR_FAILURE;
    }
    
    size_t i, stLength = strlen (pszPassword);
    if (stLength > MAX_PASSWORD_LENGTH) {
        
        if (bPrintErrors) {
            char pszText [256];
            sprintf (pszText, "Passwords cannot be longer than %i characters", MAX_PASSWORD_LENGTH);
            AddMessage (pszText);
        }
        
        return ERROR_FAILURE;
    }
    
    // Make sure the characters are permitted
    for (i = 0; i < stLength; i ++) {
        if (pszPassword[i] < FIRST_VALID_PASSWORD_CHAR || 
            pszPassword[i] > LAST_VALID_PASSWORD_CHAR) {
            if (bPrintErrors) {
                AddMessage ("The password contains an invalid character");
            }
            return ERROR_FAILURE;
        }
    }
    
    // We're ok
    return OK;
}

int HtmlRenderer::VerifyEmpireName (const char* pszEmpireName, bool bPrintErrors) {
    
    char c;
    
    if (pszEmpireName == NULL || *pszEmpireName == '\0') {
        if (bPrintErrors) {
            AddMessage ("Empire names cannot be blank");
        }
        return ERROR_FAILURE;
    }
    
    size_t i, stLength = strlen (pszEmpireName);
    if (stLength > MAX_EMPIRE_NAME_LENGTH) {
        
        if (bPrintErrors) {
            
            char pszText [128];
            sprintf (pszText, "Empire names cannot be longer than %i characters", MAX_EMPIRE_NAME_LENGTH );
            
            AddMessage (pszText);
        }
        return ERROR_FAILURE;
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
            return ERROR_FAILURE;
        }
    }
    
    // We're ok
    return OK;
}

int HtmlRenderer::VerifyCategoryName (const char* pszCategory, const char* pszName, size_t stMaxLen,
                                      bool bPrintErrors) {
    
    char c;
    
    if (pszName == NULL || *pszName == '\0') {
        if (bPrintErrors) {
            AddMessage (pszCategory);
            AddMessage (" names cannot be blank");
        }
        return ERROR_FAILURE;
    }
    
    size_t i, stLength = strlen (pszName);
    if (stLength > stMaxLen) {
        
        if (bPrintErrors) {
            AddMessage (pszCategory);
            AddMessage (" names cannot be longer than ");
            AddMessage ((int) stMaxLen);
            AddMessage (" characters");
        }
        
        return ERROR_FAILURE;
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
            return ERROR_FAILURE;
        }
    }
    
    // We're ok
    return OK;
}


void HtmlRenderer::WriteVersionString() {
    
    m_pHttpResponse->WriteText (g_pGameEngine->GetSystemVersion());
    OutputText (" @ ");
    
    Variant vServerName;
    int iErrCode = g_pGameEngine->GetSystemProperty (SystemData::ServerName, &vServerName);
    if (iErrCode == OK) {   
        m_pHttpResponse->WriteText (vServerName.GetCharPtr());
    }
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
        m_pHttpResponse->WriteText (m_iBackgroundKey);
        OutputText ("/" BACKGROUND_IMAGE "\"");
        break;
    }
    
    if (m_iSystemOptions & FIXED_BACKGROUNDS) {
        OutputText (" bgproperties=\"fixed\"");
    }
    
    OutputText (" text=\"#");
    m_pHttpResponse->WriteText (m_vTextColor.GetCharPtr());
    
    OutputText ("\" link=\"#" DEFAULT_LINK_COLOR "\" alink=\"#"\
        DEFAULT_ALINK_COLOR "\" vlink=\"#" DEFAULT_VLINK_COLOR "\"");
    
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

void HtmlRenderer::WriteAlmonasterBanner() {
    
    OutputText ("<img alt=\"Almonaster\" align=\"center\" src=\"" BASE_RESOURCE_DIR ALMONASTER_BANNER_IMAGE "\">");
}

void HtmlRenderer::WriteContactLine() {

    Variant vEmail;
    int iErrCode = g_pGameEngine->GetSystemProperty (SystemData::AdminEmail, &vEmail);
    if (iErrCode == OK) {

        String strFilter;
        
        OutputText ("<p><strong>Contact the <a href=\"mailto:");
        if (HTMLFilter (vEmail.GetCharPtr(), &strFilter, 0, false) == OK) {
            m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
        }
        OutputText ("\">administrator</a> if you have problems or suggestions</strong><p>");
        
    } else {
        
        OutputText ("<p>Could not read the administrator's email address<p>");
    }
}

void HtmlRenderer::WriteSeparatorString (int iSeparatorKey) {
    
    switch (iSeparatorKey) {
        
    case NULL_THEME:
        OutputText (DEFAULT_SEPARATOR_STRING);
        break;
        
    case ALTERNATIVE_PATH:
        
        OutputText ("<img src=\"");
        m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
        OutputText ("/" SEPARATOR_IMAGE "\" width=\"90%\" height=\"16\">");
        break;
        
    default:
        
        OutputText ("<img src=\"" BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iSeparatorKey);     
        OutputText ("/" SEPARATOR_IMAGE "\" width=\"90%\" height=\"16\">");
        break;
    }
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

void HtmlRenderer::WriteTime (Seconds sNumSeconds) {
    
    int iHrs = 0, iMin = 0;
    
    if (sNumSeconds < 0) {
        OutputText ("<strong>Error: ");
        m_pHttpResponse->WriteText (sNumSeconds);
        OutputText ("</strong>");
        return;
    }
    
    if (sNumSeconds == 0) {
        OutputText ("<strong>0</strong> sec");
        return;
    }
    
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
}

int HtmlRenderer::ConvertTime (Seconds sNumSeconds, char pszTime[MAX_HTML_TIME_LENGTH]) {
    
    int iHrs = 0, iMin = 0;
    *pszTime = '\0';
    
    if (sNumSeconds < 0) {
        sprintf (pszTime, "<strong>Error: %i</strong>", sNumSeconds);
        return ERROR_INVALID_ARGUMENT;
    }
    
    if (sNumSeconds == 0) {
        StrNCpy (pszTime, "<strong>0</strong> sec");
        return OK;
    }
    
    if (sNumSeconds >= 3600) {
        
        iHrs = sNumSeconds / 3600;
        sNumSeconds -= iHrs * 3600;
        
        sprintf (pszTime, "<strong>%i</strong> hr", iHrs);
        
        if (iHrs != 1) {
            StrNCat (pszTime, "s");
        }
    }
    
    if (sNumSeconds >= 60) {
        
        iMin = sNumSeconds / 60;
        sNumSeconds -= iMin * 60;
        
        if (iHrs > 0) {
            StrNCat (pszTime, ", ");
        }
        
        sprintf (pszTime + strlen (pszTime), "<strong>%i</strong> min", iMin);
    }
    
    if (sNumSeconds > 0) {
        
        if (iMin > 0 || iHrs > 0) {
            StrNCat (pszTime, ", ");
        }
        
        sprintf (pszTime + strlen (pszTime), "<strong>%i</strong> sec", sNumSeconds);
    }
    
    return OK;
}

void HtmlRenderer::WriteEmpireIcon (int iIconKey, int iEmpireKey, const char* pszAlt, bool bVerifyUpload) {

    WriteIcon (iIconKey, iEmpireKey, NO_KEY, pszAlt, BASE_UPLOADED_ALIEN_DIR, bVerifyUpload);
}


void HtmlRenderer::WriteIcon (int iIconKey, int iEntityKey, int iEntityKey2,
                              const char* pszAlt, const char* pszUploadDir, bool bVerifyUpload) {
    
    if (iIconKey != UPLOADED_ICON) {

        OutputText ("<img src=\"" BASE_RESOURCE_DIR BASE_ALIEN_DIR ALIEN_NAME);

    } else {
        
        bool bDisplay = true;
        
        if (bVerifyUpload) {
            
            // Make sure file exists
            char pszDestFileName[OS::MaxFileNameLength];

            if (iEntityKey2 != NO_KEY) {

                sprintf (
                    pszDestFileName, 
                    "%s/%s/" ALIEN_NAME "%i.%i" DEFAULT_IMAGE_EXTENSION, 
                    g_pszResourceDir,
                    pszUploadDir,
                    iEntityKey,
                    iEntityKey2
                    );

            } else {
            
                sprintf (
                    pszDestFileName, 
                    "%s/%s/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
                    g_pszResourceDir,
                    pszUploadDir,
                    iEntityKey
                    );
            }
            
            if (!File::DoesFileExist (pszDestFileName)) {
                bDisplay = false;
            }
        }
        
        if (bDisplay) {
            
            OutputText ("<img src=\"" BASE_RESOURCE_DIR);
            m_pHttpResponse->WriteText (pszUploadDir);
            OutputText ("/" ALIEN_NAME);

            if (iEntityKey2 != NO_KEY) {
                m_pHttpResponse->WriteText (iEntityKey);
                OutputText (".");
                iIconKey = iEntityKey2;
            } else {
                iIconKey = iEntityKey;
            }
            
        } else {
            
            // Get default icon
            Variant vIconKey;
            int iErrCode = g_pGameEngine->GetSystemProperty (SystemData::DefaultAlien, &vIconKey);
            if (iErrCode == OK) {
                iIconKey = vIconKey.GetInteger();
            } else {
                iIconKey = 1;
            }
            
            OutputText ("<img src=\"" BASE_RESOURCE_DIR BASE_ALIEN_DIR ALIEN_NAME);
        }
    }
    
    m_pHttpResponse->WriteText (iIconKey);
    OutputText (DEFAULT_IMAGE_EXTENSION "\"");
    
    if (!String::IsBlank (pszAlt)) {
        OutputText (" alt=\"");
        m_pHttpResponse->WriteText (pszAlt);
        OutputText ("\"");
    }
    
    OutputText (">");
}

void HtmlRenderer::WriteProfileAlienString (int iAlienKey, int iEmpireKey, 
                                            const char* pszEmpireName, int iBorder, const char* pszFormName, 
                                            const char* pszAlt, bool bVerifyUpload, bool bKeyAndHash) {
    
    OutputText ("<input type=\"image\" border=\"");
    m_pHttpResponse->WriteText (iBorder);
    OutputText ("\" src=\"" BASE_RESOURCE_DIR);
    
    if (iAlienKey == UPLOADED_ICON) {

        Assert (iEmpireKey != NO_KEY);
        
        bool bDisplay = true;
        
        if (bVerifyUpload) {

            // Make sure file exists
            char pszDestFileName[OS::MaxFileNameLength];
            
            sprintf (
                pszDestFileName, 
                "%s/" BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
                g_pszResourceDir,
                iEmpireKey
                );
            
            if (!File::DoesFileExist (pszDestFileName)) {
                bDisplay = false;
            }
        }
        
        if (bDisplay) {
            
            OutputText (BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME);
            m_pHttpResponse->WriteText (iEmpireKey);
            OutputText (DEFAULT_IMAGE_EXTENSION);
            
        } else {
            
            // Get default icon
            Variant vIconKey;
            int iErrCode = g_pGameEngine->GetSystemProperty (SystemData::DefaultAlien, &vIconKey);
            if (iErrCode == OK) {
                iAlienKey = vIconKey.GetInteger();
            } else {
                iAlienKey = 1;
            }
            
            OutputText (BASE_ALIEN_DIR ALIEN_NAME);
            m_pHttpResponse->WriteText (iAlienKey);
            OutputText (DEFAULT_IMAGE_EXTENSION);
        }
        
    } else {
        
        OutputText (BASE_ALIEN_DIR ALIEN_NAME);
        m_pHttpResponse->WriteText (iAlienKey);
        OutputText (DEFAULT_IMAGE_EXTENSION);
    }
    
    OutputText ("\" name=\"");
    m_pHttpResponse->WriteText (pszFormName);
    
    if (bKeyAndHash) {
        OutputText (".");
        m_pHttpResponse->WriteText (iEmpireKey);
        OutputText (".");
        m_pHttpResponse->WriteText (Algorithm::GetStringHashValue (pszEmpireName, EMPIRE_NAME_HASH_LIMIT, true));
    }
    
    OutputText ("\" alt=\"");
    m_pHttpResponse->WriteText (pszAlt);
    OutputText ("\">");
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

int HtmlRenderer::HTMLFilter (const char* pszSource, String* pstrFiltered, size_t stNumChars, bool bAddMarkups) {
    
    *pstrFiltered = "";
    if (!String::IsBlank (pszSource)) {
        return String::AtoHtml (pszSource, pstrFiltered, stNumChars, bAddMarkups) != NULL ? OK : ERROR_OUT_OF_MEMORY;
    }
    
    return OK;
}

bool HtmlRenderer::VerifyGIF (const char* pszFileName) {
    
    File fGifFile;
    
    if (fGifFile.OpenRead (pszFileName) != OK) {
        AddMessage ("The uploaded file could not be opened");
        return false;
    }
    
    const size_t stHeaderSize = 10;
    char pszBuffer [stHeaderSize];
    size_t stNumBytes, stSize;
    
    // Read the gif header
    if (fGifFile.Read (pszBuffer, stHeaderSize, &stNumBytes) != OK) {
        AddMessage ("The uploaded file appears to be damaged");
        fGifFile.Close();
        return false;
    }
    
    // Close the gif file
    fGifFile.Close();
    
    if (stNumBytes != stHeaderSize) {
        AddMessage ("The uploaded file is too small");
        return false;
    }
    
    if (File::GetFileSize (pszFileName, &stSize) != OK) {
        AddMessage ("The uploaded file could not be opened");
        return false;
    }
    
    Variant vMaxIconSize;
    int iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxIconSize, &vMaxIconSize);
    if (iErrCode != OK) {
        Assert (false);
        AddMessage ("The max icon size could could not be read");
        return false;
    }
    
    if (stSize > (size_t) vMaxIconSize.GetInteger()) {
        
        char pszError [512];
        sprintf (
            pszError, 
            "The uploaded file is larger than the upper limit (%i KB)", 
            (int) (vMaxIconSize.GetInteger() / 1024)
            );
        AddMessage (pszError);
        return false;
    }
    
    // Ensure gif89a
    if (strncmp (ICON_FORMAT, pszBuffer, sizeof (ICON_FORMAT) - 1) != 0) {
        AddMessage ("The uploaded file is not in " ICON_FORMAT " format");
        return false;
    }
    
    // Get size of image
    short siX = *(short*) (pszBuffer + 6);
    short siY = *(short*) (pszBuffer + 8);
    
    if (siX != ICON_WIDTH || siY != ICON_HEIGHT) {
        
        char pszError [512];
        sprintf (
            pszError, 
            "The uploaded " ICON_FORMAT " file is not %ix%i",
            ICON_WIDTH,
            ICON_HEIGHT
            );
        
        AddMessage (pszError);
        return false;
    }
    
    return true;
}


int HtmlRenderer::CopyUploadedIcon (const char* pszFileName, const char* pszUploadDir, int iKey1, int iKey2) {
    
    int iErrCode;
    char pszDestFileName[OS::MaxFileNameLength];

    Assert (iKey1 != NO_KEY);

    if (iKey2 == NO_KEY) {
    
        sprintf (
            pszDestFileName, 
            "%s/%s/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION,
            g_pszResourceDir,
            pszUploadDir,
            iKey1
            );

    } else {

        sprintf (
            pszDestFileName, 
            "%s/%s/" ALIEN_NAME "%i.%i" DEFAULT_IMAGE_EXTENSION,
            g_pszResourceDir,
            pszUploadDir,
            iKey1,
            iKey2
            );
    }       

    // TODO:  need a better solution for this
    unsigned int iTries = 0;
    while (iTries < 20) {
        
        g_pFileCache->ReleaseFile (pszDestFileName);
        
        iErrCode = File::CopyFile (pszFileName, pszDestFileName);
        if (iErrCode == OK) {
            return OK;
        }
        
        iTries ++;
        OS::Sleep (250);
    }
    
    Assert (false);
    return ERROR_FAILURE;
}

int HtmlRenderer::CopyNewAlien (const char* pszFileName, int iAlienKey) {
    
    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_ALIEN_DIR ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
        g_pszResourceDir,
        iAlienKey
        );
    
    g_pFileCache->ReleaseFile (pszDestFileName);
    
    return File::CopyFile (pszFileName, pszDestFileName);
}

int HtmlRenderer::DeleteAlien (int iAlienKey) {
    
    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_ALIEN_DIR ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
        g_pszResourceDir,
        iAlienKey
        );
    
    g_pFileCache->ReleaseFile (pszDestFileName);
    
    return File::DeleteFile (pszDestFileName);
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


bool HtmlRenderer::VerifyEmpireNameHash (int iEmpireKey, unsigned int iHash) {
    
    int iErrCode;
    unsigned int iRealHash;
    Variant vName;
    
    iErrCode = g_pGameEngine->GetEmpireName (iEmpireKey, &vName);
    if (iErrCode != OK) {
        return false;
    }
    
    iRealHash = Algorithm::GetStringHashValue (vName.GetCharPtr(), EMPIRE_NAME_HASH_LIMIT, true);
    
    return iRealHash == iHash;
}

void HtmlRenderer::WriteGameTitleString() {

    const char* pszEmpireName = m_vEmpireName.GetCharPtr();
    
    m_pHttpResponse->WriteText (pszEmpireName);
    if (pszEmpireName [strlen (pszEmpireName) - 1] == 's') {
        OutputText ("' ");
    } else {
        OutputText ("'s ");
    }

    m_pHttpResponse->WriteText (PageName [m_pgPageId]);
    OutputText (": ");
    WriteVersionString();
}

int HtmlRenderer::WriteGameHeaderString() {
    
    // Open form
    int iErrCode = OK;

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
    
    if (pszEmpireName [strlen (pszEmpireName) - 1] == 's') {
        OutputText ("' ");
    } else {
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
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    // Buttons
    WriteGameButtons();
    
    // Write local time
    if (m_bTimeDisplay) {
        
        char pszDateString [OS::MaxDateLength];
        
        int iErrCode = Time::GetDateString (pszDateString);
        if (iErrCode == OK) {
            
            OutputText ("Server time is <strong>");
            m_pHttpResponse->WriteText (pszDateString);
            OutputText ("</strong><p>");
        }
    }
    
    // Next update
    WriteGameNextUpdateString();
    
    // Messages
    WriteGameMessages();
    
    // Last separator
    WriteSeparatorString (m_iSeparatorKey);

    return iErrCode;
}

bool HtmlRenderer::RedirectOnSubmitGame (PageId* ppageRedirect) {
    
    int iErrCode;
    bool bFlag;
    
    IHttpForm* pHttpForm;

    Assert (m_pgPageId > MIN_PAGE_ID && m_pgPageId < MAX_PAGE_ID);
    
    if (WasButtonPressed (PageButtonId[m_pgPageId])) {
        return false;
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
            iErrCode = g_pGameEngine->SetEmpireReadyForUpdate (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
            Assert (iErrCode == OK);
            
            // Redirect to same page
            if (bFlag) {
                AddMessage ("You are now ready for an update");
            }
            *ppageRedirect = m_pgPageId;
            goto True;
        
        } else {

            // Tell empire that his end turn didn't work
            AddMessage ("Your end turn was too late");
            goto False;
        }
    }
    
    if (WasButtonPressed (BID_UNENDTURN)) {
        
        if (m_iNumNewUpdates == m_iNumOldUpdates) {

            // Unend turn
            iErrCode = g_pGameEngine->SetEmpireNotReadyForUpdate (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
            
            // Redirect to same page
            if (bFlag) {
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
    
    pHttpForm = m_pHttpRequest->GetForm ("ProfileLink.x");
    if (pHttpForm != NULL) {
        
        m_iReserved = m_iEmpireKey;
        *ppageRedirect = GAME_PROFILE_VIEWER;
        goto True;
    }
    
    if (NotifiedProfileLink()) {
        
        pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ProfileLink");
        if (pHttpForm != NULL) {
            
            const char* pszProfile = pHttpForm->GetName();
            
            Assert (pszProfile != NULL);
            
            int iViewProfileEmpireKey;
            unsigned int iHash;
            
            if (sscanf (pszProfile, "ProfileLink.%d.%d.x", &iViewProfileEmpireKey, &iHash) == 2 &&
                VerifyEmpireNameHash (iViewProfileEmpireKey, iHash)) {
                
                m_iReserved = iViewProfileEmpireKey;
                *ppageRedirect = GAME_PROFILE_VIEWER;
                goto True;
                
            } else {
                
                AddMessage ("That empire no longer exists");
            }
        }
    }

False:

    return false;
    
True:
    
    g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
    m_pgeLock = NULL;

    return true;
}

int HtmlRenderer::InitializeGame (PageId* ppageRedirect) {
    
    int iErrCode;
    bool bFlag;
    
    PageId pgSrcPageId;
    IHttpForm* pHttpForm = m_pHttpRequest->GetForm ("PageId");
    
    pgSrcPageId = (pHttpForm != NULL) ? (PageId) pHttpForm->GetIntValue() : LOGIN;
    
    // If an auto-submission, get game class and number from forms
    if (pgSrcPageId == m_pgPageId) {
        
        // Get game class
        if ((pHttpForm = m_pHttpRequest->GetForm ("GameClass")) == NULL) {
            AddMessage ("Missing GameClass form");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }
        m_iGameClass = pHttpForm->GetIntValue();
        
        // Get game number
        if ((pHttpForm = m_pHttpRequest->GetForm ("GameNumber")) == NULL) {
            AddMessage ("Missing GameNumber form");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }
        m_iGameNumber = pHttpForm->GetIntValue();
        
        // Get old update count
        if ((pHttpForm = m_pHttpRequest->GetForm ("Updates")) == NULL) {
            AddMessage ("Missing Updates form");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }
        m_iNumOldUpdates = pHttpForm->GetIntValue();    
    }

    // Verify empire's presence in game
    iErrCode = g_pGameEngine->IsEmpireInGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
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
        iErrCode = g_pGameEngine->GetEmpireOptions (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_iGameOptions);
        if (iErrCode != OK) {
            AddMessage ("That empire no longer exists");
            *ppageRedirect = LOGIN;
            return ERROR_FAILURE;
        }

        // Get gameclass name
        iErrCode = g_pGameEngine->GetGameClassName (m_iGameClass, m_pszGameClassName);
        if (iErrCode != OK) {
            AddMessage ("That game no longer exists");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }

        // Set some flags
        m_bRepeatedButtons = (m_iGameOptions & GAME_REPEATED_BUTTONS) != 0;
        m_bTimeDisplay = (m_iGameOptions & GAME_DISPLAY_TIME) != 0;

        // Get game ratios
        iErrCode = g_pGameEngine->GetEmpireGameProperty (
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
    if (g_pGameEngine->CheckGameForUpdates (m_iGameClass, m_iGameNumber, &bUpdate) != OK ||
        g_pGameEngine->WaitGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_pgeLock) != OK
        ) {
        
        // Remove update message after update
        if (bUpdate && m_strMessage.Equals ("You are now ready for an update")) {
            m_strMessage.Clear();
        }
        
        AddMessage ("The game ended");
        *ppageRedirect = ACTIVE_GAME_LIST;
        return ERROR_FAILURE;
    }
    
    // Re-verify empire's presence in game
    iErrCode = g_pGameEngine->IsEmpireInGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
    if (iErrCode != OK || !bFlag) {

        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
        m_pgeLock = NULL;

        AddMessage ("You are no longer in that game");
        *ppageRedirect = ACTIVE_GAME_LIST;
        return ERROR_FAILURE;
    }
    
    // Verify not resigned
    iErrCode = g_pGameEngine->HasEmpireResignedFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
    if (iErrCode != OK || bFlag) {
        
        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
        m_pgeLock = NULL;

        AddMessage ("Your empire has resigned from that game");
        *ppageRedirect = ACTIVE_GAME_LIST;
        return ERROR_FAILURE;
    }
    
    // Log empire into game if not an auto submission
    IHttpForm* pHttpAuto = m_pHttpRequest->GetForm ("Auto");
    if (pHttpAuto == NULL || pHttpAuto->GetIntValue() == 0 && !m_bLoggedIn) {
        
        iErrCode = g_pGameEngine->LogEmpireIntoGame (
            m_iGameClass, 
            m_iGameNumber, 
            m_iEmpireKey
            );
        
        if (iErrCode != OK) {
            AddMessage ("Your empire could not be logged into the game");
            *ppageRedirect = ACTIVE_GAME_LIST;
            return ERROR_FAILURE;
        }
        
        m_bLoggedIn = true;
    }
    
    // Remove update message after update
    if (bUpdate && m_strMessage.Equals ("You are now ready for an update")) {
        m_strMessage.Clear();
    }
    
    // Get game update information
    if (g_pGameEngine->GetGameUpdateData (
        m_iGameClass, 
        m_iGameNumber, 
        &m_sSecondsSince, 
        &m_sSecondsUntil, 
        &m_iNumNewUpdates, 
        &m_iGameState
        ) != OK
        ) {
        
        Assert (false);

        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
        m_pgeLock = NULL;

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


void HtmlRenderer::WriteGameNextUpdateString() {

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
    
    if (!(m_iGameOptions & COUNTDOWN) || (m_iGameState & PAUSED) || !(m_iGameState & STARTED)) {
        
        if (!(m_iGameState & STARTED)) {
            
            int iNumEmpires, iNumNeeded, iTotal, 
                
            iErrCode = g_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires);
            if (iErrCode != OK) {
                Assert (false);
                return;
            }
            
            iErrCode = g_pGameEngine->GetNumEmpiresNeededForGame (m_iGameClass, &iNumNeeded);
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
    
    OutputText ("<p>");
}


int HtmlRenderer::PostGamePageInformation() {
    
    int64 i64PasswordHash = 0;
    int iErrCode = GetPasswordHashForGamePage (m_tNewSalt, &i64PasswordHash);
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

void HtmlRenderer::CloseGamePage() {
    
    // Unlock the game by decrementing the thread count
    g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
    m_pgeLock = NULL;

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
    
    MilliSeconds msTime = GetTimerCount();
    
    OnPageRender (msTime);
    
    OutputText ("<p>");
    WriteVersionString();
    OutputText ("<br>Script time: ");
    m_pHttpResponse->WriteText ((int) msTime);
    OutputText (" ms</font></strong></center></form></body></html>");
}

void HtmlRenderer::GetAlienButtonString (int iAlienKey, int iEmpireKey, bool bBorder, int iPlanetKey, 
                                         int iProxyKey, const char* pszAlt, const char* pszExtraTag,
                                         String* pstrAlienButtonString) {
    
    *pstrAlienButtonString = "<input type=\"image\" border=\"";
    *pstrAlienButtonString += (bBorder ? 1:0);
    *pstrAlienButtonString += "\" src=\"" BASE_RESOURCE_DIR;
    
    if (iAlienKey == NO_KEY) {
        *pstrAlienButtonString += BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME;
        *pstrAlienButtonString += iEmpireKey;
    } else {
        *pstrAlienButtonString += BASE_ALIEN_DIR ALIEN_NAME;
        *pstrAlienButtonString += iAlienKey;
    }
    
    *pstrAlienButtonString += DEFAULT_IMAGE_EXTENSION "\" name=\"Planet";
    *pstrAlienButtonString += iPlanetKey;
    *pstrAlienButtonString += ".";
    *pstrAlienButtonString += iProxyKey;
    
    *pstrAlienButtonString += "\"";
    
    if (!String::IsBlank (pszAlt)) {
        
        *pstrAlienButtonString += " alt=\"";
        *pstrAlienButtonString += pszAlt;
        *pstrAlienButtonString += "\"";
    }

    if (pszExtraTag != NULL) {
        *pstrAlienButtonString += " ";
        *pstrAlienButtonString += pszExtraTag;
    }
    
    *pstrAlienButtonString += ">";
}

void HtmlRenderer::WriteAlienButtonString (int iAlienKey, bool bBorder, const char* pszNamePrefix,
                                           const char* pszAuthorName) {
    
    OutputText ("<input type=\"image\" border=\"");
    m_pHttpResponse->WriteText (bBorder ? 1:0);
    OutputText ("\" src=\"" BASE_RESOURCE_DIR);
    OutputText (BASE_ALIEN_DIR ALIEN_NAME);
    m_pHttpResponse->WriteText (iAlienKey);
    OutputText (DEFAULT_IMAGE_EXTENSION "\" name=\"");
    m_pHttpResponse->WriteText (pszNamePrefix);
    m_pHttpResponse->WriteText (iAlienKey);
    OutputText ("\" alt=\"Alien ");
    m_pHttpResponse->WriteText (iAlienKey);
    OutputText (" by ");
    m_pHttpResponse->WriteText (pszAuthorName);
    OutputText ("\">");
}

void HtmlRenderer::GetLivePlanetButtonString (int iLivePlanetKey, int iPlanetKey, int iProxyKey, 
                                              const char* pszAlt, const char* pszExtraTag,
                                              String* pstrLivePlanet) {
    
    switch (iLivePlanetKey) {
        
    case NULL_THEME:
        
        *pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"" \
            BASE_RESOURCE_DIR LIVE_PLANET_NAME "\" name=\"Planet";
        break;
        
    case ALTERNATIVE_PATH:
        
        *pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"";
        *pstrLivePlanet += m_vLocalPath.GetCharPtr();
        *pstrLivePlanet += "/" LIVE_PLANET_NAME "\" name=\"Planet";
        break;
        
    default:
        
        *pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR;
        *pstrLivePlanet += iLivePlanetKey;
        *pstrLivePlanet += "/" LIVE_PLANET_NAME "\" name=\"Planet";
        break;
    }
    
    *pstrLivePlanet += iPlanetKey;
    *pstrLivePlanet += ".";
    *pstrLivePlanet += iProxyKey;
    *pstrLivePlanet += "\"";
    
    if (!String::IsBlank (pszAlt)) {
        
        *pstrLivePlanet += " alt=\"";
        *pstrLivePlanet += pszAlt;
        *pstrLivePlanet += "\"";
    }

    if (pszExtraTag != NULL) {
        *pstrLivePlanet += " ";
        *pstrLivePlanet += pszExtraTag;
    }
    
    *pstrLivePlanet += ">";
}

void HtmlRenderer::GetDeadPlanetButtonString (int iDeadPlanetKey, int iPlanetKey, int iProxyKey, 
                                              const char* pszAlt, const char* pszExtraTag,
                                              String* pstrDeadPlanet) {
    
    switch (iDeadPlanetKey) {   
        
    case NULL_THEME:
        
        *pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"" \
            BASE_RESOURCE_DIR DEAD_PLANET_NAME "\" name=\"Planet";
        break;
        
    case ALTERNATIVE_PATH:
        
        *pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"";
        *pstrDeadPlanet += m_vLocalPath.GetCharPtr();
        *pstrDeadPlanet += "/" DEAD_PLANET_NAME "\" name=\"Planet";
        break;
        
    default:
        
        *pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR;
        *pstrDeadPlanet += iDeadPlanetKey;
        *pstrDeadPlanet += "/" DEAD_PLANET_NAME "\" name=\"Planet";
        break;
    }
    
    *pstrDeadPlanet += iPlanetKey;
    *pstrDeadPlanet += ".";
    *pstrDeadPlanet += iProxyKey;
    *pstrDeadPlanet += "\"";
    
    if (!String::IsBlank (pszAlt)) {
        
        *pstrDeadPlanet += " alt=\"";
        *pstrDeadPlanet += pszAlt;
        *pstrDeadPlanet += "\"";
    }

    if (pszExtraTag != NULL) {
        *pstrDeadPlanet += " ";
        *pstrDeadPlanet += pszExtraTag;
    }
    
    *pstrDeadPlanet += ">";
}

void HtmlRenderer::GetIndependentPlanetButtonString (int iPlanetKey, int iProxyKey, const char* pszAlt, 
                                                     const char* pszExtraTag, String* pstrPlanetString) {
    
    *pstrPlanetString = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR INDEPENDENT_PLANET_NAME "\" name=\"Planet";
    *pstrPlanetString += iPlanetKey;
    *pstrPlanetString += ".";
    *pstrPlanetString += iProxyKey;
    *pstrPlanetString += "\"";
    
    if (!String::IsBlank (pszAlt)) {
        
        *pstrPlanetString += " alt=\"";
        *pstrPlanetString += pszAlt;
        *pstrPlanetString += "\"";
    }

    if (pszExtraTag != NULL) {
        *pstrPlanetString += " ";
        *pstrPlanetString += pszExtraTag;
    }
    
    *pstrPlanetString += ">";
}

void HtmlRenderer::WriteLivePlanetString (int iLivePlanetKey) {
    
    switch (iLivePlanetKey) {
        
    case NULL_THEME:
        
        OutputText ("<img src=\"" BASE_RESOURCE_DIR LIVE_PLANET_NAME "\">");
        break;
        
    case ALTERNATIVE_PATH:
        
        OutputText ("<img src=\"");
        m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
        OutputText ("/" LIVE_PLANET_NAME "\">");
        break;
        
    default:
        
        OutputText ("<img src=\"" BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iLivePlanetKey);
        OutputText ("/" LIVE_PLANET_NAME "\">");
        break;
    }
}

void HtmlRenderer::WriteDeadPlanetString (int iDeadPlanetKey) {
    
    switch (iDeadPlanetKey) {
        
    case NULL_THEME:
        
        OutputText ("<img src=\"" BASE_RESOURCE_DIR DEAD_PLANET_NAME "\">");
        break;
        
    case ALTERNATIVE_PATH:
        
        OutputText ("<img src=\"");
        m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
        OutputText ("/" DEAD_PLANET_NAME "\">");
        break;
        
    default:
        
        OutputText ("<img src=\"" BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iDeadPlanetKey);
        OutputText ("/" DEAD_PLANET_NAME "\">");
        break;
    }
}

void HtmlRenderer::WriteIndependentPlanetString() {
    
    OutputText ("<img src=\"" BASE_RESOURCE_DIR INDEPENDENT_PLANET_NAME "\">");
}

int HtmlRenderer::WriteUpClosePlanetString (int iEmpireKey, int iPlanetKey, int iProxyPlanetKey, 
                                            int iLivePlanetKey, int iDeadPlanetKey, 
                                            int iPlanetCounter, bool bVisibleBuilds, int iGoodAg, int iBadAg, 
                                            int iGoodMin, int iBadMin, int iGoodFuel, int iBadFuel, 
                                            float fEmpireAgRatio, bool bIndependence, 
                                            bool bAdmin, bool bSpectator, const Variant* pvPlanetData,
                                            bool* pbOurPlanet) {
    
    int iErrCode = OK, i, j;

    bool bMapColoring = !bAdmin && !bSpectator; 
    const char* pszTableColor = m_vTableColor.GetCharPtr();
    
    String strFilter;
    
    if (!(m_iGameState & GAME_MAP_GENERATED)) {
        
        // Lay down the default "planet lost in outer space" table
        OutputText ("<tr><th>&nbsp;</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Name</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Location</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Owner</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Min</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Fuel</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Ag</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Pop</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Max Pop</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">Next Pop</th><th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\" align=\"left\">Jumps</th></tr><tr><td align=\"center\">");
        
        WriteProfileAlienString (
            m_iAlienKey,
            iEmpireKey,
            m_vEmpireName.GetCharPtr(),
            0, 
            "ProfileLink",
            "View your profile",
            false,
            false
            );
        
        OutputText ("</td><td align=\"center\"><strong>");
        m_pHttpResponse->WriteText (m_vEmpireName.GetCharPtr());    // Name
        OutputText ("</strong></td><td align=\"center\">None yet</td><td align=\"center\"><strong>");
        m_pHttpResponse->WriteText (m_vEmpireName.GetCharPtr());
        OutputText ("</strong></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">0</font></td><td align=\"left\">None yet</td></tr>");
        
        return OK;
    }
    
    OutputText ("<tr><th>&nbsp;</th><th align=\"left\" bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Name</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Location</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Owner</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Min</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Fuel</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Ag</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Pop</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Max Pop</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Next Pop</th><th bgcolor=\"");
    m_pHttpResponse->WriteText (pszTableColor);
    OutputText ("\">Jumps</th></tr><tr><td align=\"center\">");
    
    int iData, iAlienKey, iOwner = pvPlanetData[GameMap::Owner].GetInteger();
    int iAnnihilated = pvPlanetData[GameMap::Annihilated].GetInteger();
    
    String strPlanet;
    int iWeOffer, iTheyOffer, iCurrent;
    
    Variant vEmpireName;
    
    if (iOwner == SYSTEM) {
        
        iCurrent = TRUCE;
        
        if (iAnnihilated == 0) {
            WriteLivePlanetString (iLivePlanetKey);
        } else {
            WriteDeadPlanetString (iDeadPlanetKey);
        }
        
    } else {
        
        if (iOwner == INDEPENDENT) {
            
            iCurrent = bMapColoring ? WAR : TRUCE;
            WriteIndependentPlanetString();
            
        } else {
            
            if (iOwner == iEmpireKey) {
                
                iCurrent = bMapColoring ? ALLIANCE : TRUCE;
                
                WriteProfileAlienString (
                    m_iAlienKey,
                    iEmpireKey,
                    m_vEmpireName.GetCharPtr(),
                    0, 
                    "ProfileLink",
                    "View your profile",
                    false,
                    false
                    );
                
            } else {
                
                if (!bMapColoring) {
                    iCurrent = TRUCE;
                } else {
                    
                    iErrCode = g_pGameEngine->GetDiplomaticStatus (
                        m_iGameClass, 
                        m_iGameNumber, 
                        iEmpireKey, 
                        iOwner, 
                        &iWeOffer, 
                        &iTheyOffer, 
                        &iCurrent
                        );
                    
                    if (iErrCode == ERROR_DATA_NOT_FOUND) {
                        iCurrent = WAR;
                    }
                    
                    else if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                }
                
                Variant vTemp;
                iErrCode = g_pGameEngine->GetEmpireProperty (iOwner, SystemEmpireData::AlienKey, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                iAlienKey = vTemp.GetInteger();
                
                iErrCode = g_pGameEngine->GetEmpireName (iOwner, &vEmpireName);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
                sprintf (pszProfile, "View the profile of %s", vEmpireName.GetCharPtr());
                
                WriteProfileAlienString (
                    iAlienKey,
                    iOwner,
                    vEmpireName.GetCharPtr(),
                    0, 
                    "ProfileLink",
                    pszProfile,
                    false,
                    true
                    );
                
                NotifyProfileLink();
            }
        }
    }
    
    OutputText ("</td>");
    
    if (HTMLFilter (pvPlanetData[GameMap::Name].GetCharPtr(), &strFilter, 0, false) != OK) {
        return ERROR_OUT_OF_MEMORY;
    }

    OutputText ("<input type=\"hidden\" name=\"KeyPlanet");
    m_pHttpResponse->WriteText (iPlanetCounter);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (iPlanetKey);
    OutputText ("\">");
    
    if (iOwner != SYSTEM && iOwner == iEmpireKey && !bAdmin) {
        
        OutputText ("<td><input type=\"text\" size=\"15\" maxlength=\"");
        m_pHttpResponse->WriteText (MAX_PLANET_NAME_LENGTH);
        OutputText ("\" name=\"NewPlanetName");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\"value=\"");
        m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
        OutputText ("\"><input type=\"hidden\" name=\"OldPlanetName");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
        OutputText ("\"><input type=\"hidden\" name=\"OldMaxPop");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (pvPlanetData[GameMap::MaxPop].GetInteger());
        OutputText ("\"></td>");
        
        *pbOurPlanet = true;
        
    } else {
        
        OutputText ("<td align=\"left\"><strong>");
        WriteStringByDiplomacy (strFilter.GetCharPtr(), iCurrent);
        OutputText ("</strong>");
        
        if (iAnnihilated != NOT_ANNIHILATED) {
            
            OutputText ("<br>(Quarantined <strong>");
            
            if (iAnnihilated == ANNIHILATED_FOREVER) {
                OutputText ("forever</strong>)");
            } else {
                
                m_pHttpResponse->WriteText (iAnnihilated);
                OutputText ("</strong> update");
                
                if (iAnnihilated == 1) {
                    OutputText (")");
                } else {
                    OutputText ("s)");
                }
            }
        }
        
        else if (pvPlanetData[GameMap::HomeWorld].GetInteger() >= ROOT_KEY) {
            OutputText ("<br>(Surrendered)");
        }
        
        OutputText ("</td>");
        
        *pbOurPlanet = false;
    }
    
    // Coordinates
    OutputText ("<td align=\"center\">");
    WriteStringByDiplomacy (pvPlanetData[GameMap::Coordinates].GetCharPtr(), iCurrent);
    OutputText ("</td><td align=\"center\">");
    
    // Owner name
    if (iOwner != SYSTEM) {
        
        Variant vEmpireName;
        const char* pszEmpireName;
        
        if (iOwner == iEmpireKey) {
            pszEmpireName = m_vEmpireName.GetCharPtr();
        } else {
            
            if (iOwner == INDEPENDENT) {
                pszEmpireName = INDEPENDENT_NAME;
            } else {
                
                iErrCode = g_pGameEngine->GetEmpireName (iOwner, &vEmpireName);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                pszEmpireName = vEmpireName.GetCharPtr();
            }
        }
        
        WriteStringByDiplomacy (pszEmpireName, iCurrent);
        
        OutputText ("</td>");
        
    } else {
        OutputText ("-</td>");
    }
    
    // Minerals
    OutputText ("<td align=\"center\">");
    
    iData = pvPlanetData[GameMap::Minerals].GetInteger();
    if (iData < iBadMin) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (iData);
        OutputText ("</font>");
    } else {
        if (iData > iGoodMin) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iData);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (iData);
        }
    }
    OutputText ("</td><td align=\"center\">");
    
    iData = pvPlanetData[GameMap::Fuel].GetInteger();
    if (iData < iBadFuel) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (iData);
        OutputText ("</font>");
    } else {
        if (iData > iGoodFuel) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iData);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (iData);
        }
    }
    OutputText ("</td><td align=\"center\">");
    
    int iAg = pvPlanetData[GameMap::Ag].GetInteger();
    if (iAg < iBadAg) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (iAg);
        OutputText ("</font>");
    } else {
        if (iAg > iGoodAg) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iAg);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (iAg); 
        }
    }
    OutputText ("</td><td align=\"center\">");
    
    iData = pvPlanetData[GameMap::Pop].GetInteger();
    if (iData == iAg) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">");
        m_pHttpResponse->WriteText (iData);
        OutputText ("</font>");
    } else {
        if (iData > iAg || iData == 0) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iData);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (iData); 
        }
    }
    
    OutputText ("</td><td align=\"center\">");
    
    if (iOwner != SYSTEM && iOwner == iEmpireKey && !bAdmin) {
        
        OutputText ("<input type=\"text\" size=\"4\" maxlength=\"4\" name=\"NewMaxPop");
        m_pHttpResponse->WriteText (iPlanetCounter);
        OutputText ("\"value=\"");
        m_pHttpResponse->WriteText (pvPlanetData[GameMap::MaxPop].GetInteger());
        OutputText ("\">");
        
    } else {
        
        if (iOwner != SYSTEM && bAdmin) {
            m_pHttpResponse->WriteText (pvPlanetData[GameMap::MaxPop].GetInteger());
        } else {
            OutputText ("-");
        }
    }
    OutputText ("</td><td align=\"center\">");
    
    if (iOwner == SYSTEM) {
        OutputText ("0");
    }
    
    else if (iOwner == iEmpireKey || bAdmin) {
        
        int iCost = pvPlanetData[GameMap::PopLostToColonies].GetInteger();
        int iPop = pvPlanetData[GameMap::Pop].GetInteger();
        
        Assert (iCost >= 0 && iCost <= iPop);
        
        int iNextPop = g_pGameEngine->GetNextPopulation (
            iPop - iCost,
            fEmpireAgRatio
            );
        
        if (iNextPop > pvPlanetData[GameMap::MaxPop].GetInteger()) {
            iNextPop = pvPlanetData[GameMap::MaxPop].GetInteger();
        }
        
        iData = iNextPop;
        if (iData == iAg) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (iData);
            OutputText ("</font>");
        } else {
            if (iData > iAg || iData == 0) {
                OutputText ("<font color=\"");
                m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                OutputText ("\">");
                m_pHttpResponse->WriteText (iData);
                OutputText ("</font>");
            } else {
                m_pHttpResponse->WriteText (iData); 
            }
        }
    }
    
    else {
        
        OutputText ("-");
    }
    
    OutputText ("</td><td align=\"center\">");
    
    int iX, iY;
    g_pGameEngine->GetCoordinates (pvPlanetData[GameMap::Coordinates].GetCharPtr(), &iX, &iY);
    
    int iLink = pvPlanetData[GameMap::Link].GetInteger();
    bool bLinkNorth = (iLink & LINK_NORTH) != 0;
    bool bLinkEast  = (iLink & LINK_EAST) != 0;
    bool bLinkSouth = (iLink & LINK_SOUTH) != 0;
    bool bLinkWest  = (iLink & LINK_WEST) != 0;
    
    int iNumJumps = (bLinkNorth ? 1:0) + (bLinkEast ? 1:0) + (bLinkSouth ? 1:0) + (bLinkWest ? 1:0);
    
    if (iNumJumps > 0) {
        
        int piJumpX [NUM_CARDINAL_POINTS];
        int piJumpY [NUM_CARDINAL_POINTS];
        int piCardinalPoint [NUM_CARDINAL_POINTS];
        
        int iNeighbourKey, iCounter = 0;
        
        if (bLinkNorth) {
            piJumpX[iCounter] = iX;
            piJumpY[iCounter] = iY + 1;
            piCardinalPoint [iCounter] = NORTH;
            iCounter ++;
        }
        
        if (bLinkEast) {
            piJumpX[iCounter] = iX + 1;
            piJumpY[iCounter] = iY;
            piCardinalPoint [iCounter] = EAST;
            iCounter ++;
        }
        
        if (bLinkSouth) {
            piJumpX[iCounter] = iX;
            piJumpY[iCounter] = iY - 1;
            piCardinalPoint [iCounter] = SOUTH;
            iCounter ++;
        }
        
        if (bLinkWest) {
            piJumpX[iCounter] = iX - 1;
            piJumpY[iCounter] = iY;
            piCardinalPoint [iCounter] = WEST;
            iCounter ++;
        }
        
        Assert (iCounter == iNumJumps);

        Variant vTemp;
        String strPlanetName;

        for (i = 0; i < iNumJumps; i ++) {
            
            // Get neighbouring planet's key
            iErrCode = g_pGameEngine->GetNeighbourPlanetKey (
                m_iGameClass, 
                m_iGameNumber, 
                iPlanetKey, 
                piCardinalPoint[i], 
                &iNeighbourKey
                );
            
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            Assert (iNeighbourKey != NO_KEY);
            
            if (!bAdmin) {
                
                iErrCode = g_pGameEngine->GetPlanetNameWithSecurity (
                    m_iGameClass, 
                    m_iGameNumber, 
                    iEmpireKey, 
                    iNeighbourKey, 
                    &vTemp
                    );

            } else {
                
                iErrCode = g_pGameEngine->GetPlanetName (
                    m_iGameClass, 
                    m_iGameNumber, 
                    iNeighbourKey, 
                    &vTemp
                    );
            }

            if (String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false) == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }
            
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            OutputText ("<strong>");
            m_pHttpResponse->WriteText (CARDINAL_STRING[piCardinalPoint[i]]);
            OutputText ("</strong>: ");
            m_pHttpResponse->WriteText (strPlanetName.GetCharPtr(), strPlanetName.GetLength());
            OutputText (" (");
            m_pHttpResponse->WriteText (piJumpX[i]);
            OutputText (",");
            m_pHttpResponse->WriteText (piJumpY[i]);
            OutputText (")<br>");
        }
        
    } else {
        OutputText ("<strong>None</strong>");
    }
    
    OutputText ("</td></tr>");
    
    int iTotalNumShips = 
        pvPlanetData[GameMap::NumUncloakedShips].GetInteger() + 
        pvPlanetData[GameMap::NumCloakedShips].GetInteger() + 
        pvPlanetData[GameMap::NumUncloakedBuildShips].GetInteger() + 
        pvPlanetData[GameMap::NumCloakedBuildShips].GetInteger();
    
    if (iTotalNumShips > 0) {
        
        int* piOwnerData;
        iErrCode = g_pGameEngine->GetPlanetShipOwnerData (
            m_iGameClass, 
            m_iGameNumber, 
            iEmpireKey, 
            iPlanetKey, 
            iProxyPlanetKey, 
            iTotalNumShips, 
            bVisibleBuilds, 
            bIndependence, 
            &piOwnerData
            );
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        int iNumOwners = piOwnerData[0];
        if (iNumOwners > 0) {
            
            m_pHttpResponse->WriteText (
                "<tr><td></td><td></td><td colspan=\"10\"><table><tr><td></td><td bgcolor=\""
                );
            
            m_pHttpResponse->WriteText (pszTableColor);
            OutputText ("\" align=\"center\"><strong>Empire</strong></td>");
            
            bool pbTech [NUM_SHIP_TYPES];
            ENUMERATE_SHIP_TYPES (i) {
                pbTech[i] = false;
            }
            int** ppiNumTechsTable = (int**) StackAlloc (iNumOwners * sizeof (int*));
            int* piTemp = (int*) StackAlloc (iNumOwners * NUM_SHIP_TYPES * sizeof (int));
            for (i = 0; i < iNumOwners; i ++) {
                ppiNumTechsTable[i] = &(piTemp [i * NUM_SHIP_TYPES]);
                ENUMERATE_SHIP_TYPES (j) {
                    ppiNumTechsTable[i][j] = 0;
                }
            }
            int* piOwnerKey = (int*) StackAlloc (iNumOwners * sizeof (int));
            
            int iBase = 1, iNumOwnerTechs, iType;
            
            for (i = 0; i < iNumOwners; i ++) {
                
                piOwnerKey[i] = piOwnerData [iBase];
                iNumOwnerTechs = piOwnerData [iBase + 1];
                
                for (j = 0; j < iNumOwnerTechs; j ++) {
                    iType = piOwnerData [iBase + 2 + j * 2];
                    pbTech [iType] = true;
                    ppiNumTechsTable [i][iType] = piOwnerData [iBase + 3 + j * 2];
                }
                
                iBase += 2 + 2 * iNumOwnerTechs;
            }
            
            ENUMERATE_SHIP_TYPES (i) {
                if (pbTech[i]) {
                    OutputText ("<td bgcolor=\"");
                    m_pHttpResponse->WriteText (pszTableColor);
                    OutputText ("\" align=\"center\"><strong>");
                    m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
                    OutputText ("</strong></td>");
                }
            }
            OutputText ("</tr>");
            
            int iDip, iWeOffer, iTheyOffer;
            
            Variant vEmpireName;
            const char* pszEmpireName;
            
            for (i = 0; i < iNumOwners; i ++) {
                
                OutputText ("<tr><td align=\"right\">");
                
                if (piOwnerKey[i] != INDEPENDENT) {
                    
                    if (piOwnerKey[i] == iEmpireKey) {
                        
                        WriteProfileAlienString (
                            m_iAlienKey,
                            iEmpireKey,
                            m_vEmpireName.GetCharPtr(),
                            0, 
                            "ProfileLink",
                            "View your profile",
                            false,
                            false
                            );
                        
                    } else {
                        
                        Variant vTemp;
                        iErrCode = g_pGameEngine->GetEmpireProperty (piOwnerKey[i], SystemEmpireData::AlienKey, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            delete [] piOwnerData;
                            return iErrCode;
                        }
                        iAlienKey = vTemp.GetInteger();
                        
                        iErrCode = g_pGameEngine->GetEmpireName (piOwnerKey[i], &vEmpireName);
                        if (iErrCode != OK) {
                            Assert (false);
                            delete [] piOwnerData;
                            return iErrCode;
                        }
                        
                        char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
                        sprintf (pszProfile, "View the profile of %s", vEmpireName.GetCharPtr());
                        
                        WriteProfileAlienString (
                            iAlienKey,
                            piOwnerKey[i],
                            vEmpireName.GetCharPtr(),
                            0, 
                            "ProfileLink",
                            pszProfile,
                            false,
                            true
                            );
                        
                        NotifyProfileLink();
                    }
                    
                } else {
                    
                    WriteIndependentPlanetString();
                }
                
                OutputText ("</td>");
                
                if (piOwnerKey[i] == iEmpireKey) {
                    
                    iDip = ALLIANCE;
                    pszEmpireName = m_vEmpireName.GetCharPtr();
                    
                } else {
                    
                    if (piOwnerKey[i] == INDEPENDENT) {
                        iDip = WAR;
                        pszEmpireName = INDEPENDENT_NAME;
                    } else {
                        
                        iErrCode = g_pGameEngine->GetEmpireName (piOwnerKey[i], &vEmpireName);
                        if (iErrCode != OK) {
                            Assert (false);
                            delete [] piOwnerData;
                            return iErrCode;
                        }
                        
                        pszEmpireName = vEmpireName.GetCharPtr();
                        
                        if (!bMapColoring) {
                            iDip = TRUCE;
                        } else {
                            
                            iErrCode = g_pGameEngine->GetDiplomaticStatus (
                                m_iGameClass, 
                                m_iGameNumber, 
                                iEmpireKey, 
                                piOwnerKey[i], 
                                &iWeOffer, 
                                &iTheyOffer, 
                                &iDip
                                );
                            
                            // Error means they haven't met
                            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                                iErrCode = OK;
                                iDip = WAR;
                            }
                            
                            else if (iErrCode != OK) {
                                Assert (false);
                                delete [] piOwnerData;
                                return iErrCode;
                            }
                        }
                    }
                }
                
                if (iDip == WAR) {
                    
                    OutputText ("<td align=\"center\"><font color=\"");
                    m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                    OutputText ("\"><strong>");
                    m_pHttpResponse->WriteText (pszEmpireName);
                    OutputText ("</strong></font></td>");
                    
                    ENUMERATE_SHIP_TYPES (j) {
                        if (pbTech[j]) {
                            OutputText ("<td align=\"center\"><font color=\"");
                            m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                            OutputText ("\">");
                            m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
                            OutputText ("</font></td>");
                        }
                    }
                    
                } else {
                    
                    if (iDip == ALLIANCE) {
                        OutputText ("<td align=\"center\"><font color=\"");
                        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                        OutputText ("\"><strong>");
                        m_pHttpResponse->WriteText (pszEmpireName);
                        OutputText ("</strong></font></td>");
                        
                        ENUMERATE_SHIP_TYPES (j) {
                            if (pbTech[j]) {
                                OutputText ("<td align=\"center\"><font color=\"");
                                m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                                OutputText ("\">");
                                m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
                                OutputText ("</font></td>");
                            }
                        }
                        
                    } else {
                        
                        OutputText ("<td align=\"center\"><strong>");
                        m_pHttpResponse->WriteText (pszEmpireName);
                        OutputText ("</strong></td>");
                        
                        ENUMERATE_SHIP_TYPES (j) {
                            if (pbTech[j]) {
                                OutputText ("<td align=\"center\">");
                                m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
                                OutputText ("</td>");
                            }
                        }
                    }
                }
                OutputText ("</tr>");
            }
            OutputText ("</table></td></tr>");
        }
        
        delete [] piOwnerData;
    }
    
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
    
    int iErrCode = g_pGameEngine->GetGameAveragePlanetResources (iGameClass, iGameNumber, &iAg, &iMin, &iFuel);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    *piGoodAg = (int) (iAg * 1.5);
    *piBadAg  = iAg / 2;
    
    *piGoodMin = (int) (iMin * 1.5);
    *piBadMin  = iAg / 2;
    
    *piGoodFuel = (int) (iFuel * 1.5);
    *piBadFuel  = iAg / 2;
    
    return OK;
}


void HtmlRenderer::SearchForDuplicateIPAddresses (int iGameClass, int iGameNumber) {
    
    int* piDuplicateKeys;
    unsigned int * piNumDuplicatesInList, iNumDuplicates;
    
    char pszBuffer [512];
    
    int iErrCode = g_pGameEngine->SearchForDuplicates (
        iGameClass, 
        iGameNumber,
        SystemEmpireData::IPAddress,
        GameEmpireData::EnterGameIPAddress,
        &piDuplicateKeys,
        &piNumDuplicatesInList,
        &iNumDuplicates
        );
    
    if (iErrCode != OK) {
        
        sprintf (pszBuffer, "Error %i occurred searching for empires with the same IP address", iErrCode);
        AddMessage (pszBuffer);
        
    } else {
        
        if (iNumDuplicates == 0) {
            AddMessage ("No </strong>empires with the same IP address were found<strong>");
        } else {
            
            if (iNumDuplicates == 1) {
                
                AddMessage ("1 </strong>duplicate IP address was found:<strong>");
                
            } else {
                
                sprintf (pszBuffer, "%i</strong> duplicate IP addresses were found:<strong>", iNumDuplicates);
                AddMessage (pszBuffer);
            }
            
            Variant vName;
            String strList;
            unsigned int iIndex = 0, iLimit, i, j;
            
            for (i = 0; i < iNumDuplicates; i ++) {
                
                strList.Clear();
                
                iLimit = iIndex + piNumDuplicatesInList[i] - 1;
                
                for (j = iIndex; j < iLimit; j ++) {
                    
                    iErrCode = g_pGameEngine->GetEmpireName (piDuplicateKeys[j], &vName);
                    if (iErrCode == OK) {
                        
                        strList += vName.GetCharPtr();
                        if (j == iLimit - 1) {
                            strList += " </strong>and<strong> ";
                        } else {
                            strList += ", ";
                        }
                    }
                }
                iIndex = iLimit + 1;
                
                iErrCode = g_pGameEngine->GetEmpireName (piDuplicateKeys[j], &vName);
                if (iErrCode == OK) {
                    strList += vName.GetCharPtr();
                }
                
                strList += " </strong>have the same IP address<strong>";
                
                AddMessage (strList);
            }
            
            // Free memory
            delete [] piNumDuplicatesInList;
            delete [] piDuplicateKeys;
        }
    }
}

void HtmlRenderer::SearchForDuplicateSessionIds (int iGameClass, int iGameNumber) {
    
    int* piDuplicateKeys;
    unsigned int * piNumDuplicatesInList, iNumDuplicates;
    
    char pszBuffer [512];
    
    int iErrCode = g_pGameEngine->SearchForDuplicates (
        iGameClass, 
        iGameNumber,
        SystemEmpireData::SessionId,
        NO_KEY,
        &piDuplicateKeys,
        &piNumDuplicatesInList,
        &iNumDuplicates
        );
    
    if (iErrCode != OK) {
        
        sprintf (pszBuffer, "Error %i occurred searching for empires with the same Session Id", iErrCode);
        AddMessage (pszBuffer);
        
    } else {
        
        if (iNumDuplicates == 0) {
            AddMessage ("No </strong>empires with the same Session Id were found<strong>");
        } else {
            
            if (iNumDuplicates == 1) {
                
                AddMessage ("1 </strong>duplicate Session Id was found:<strong>");
                
            } else {
                
                sprintf (pszBuffer, "%i</strong> duplicate Session Ids were found:<strong>", iNumDuplicates);
                AddMessage (pszBuffer);
            }
            
            Variant vName;
            String strList;
            unsigned int iIndex = 0, iLimit, i, j;
            
            for (i = 0; i < iNumDuplicates; i ++) {
                
                strList.Clear();
                
                iLimit = iIndex + piNumDuplicatesInList[i] - 1;
                
                for (j = iIndex; j < iLimit; j ++) {
                    
                    iErrCode = g_pGameEngine->GetEmpireName (piDuplicateKeys[j], &vName);
                    if (iErrCode == OK) {
                        
                        strList += vName.GetCharPtr();
                        if (j == iLimit - 1) {
                            strList += " </strong>and<strong> ";
                        } else {
                            strList += ", ";
                        }
                    }
                }
                iIndex = iLimit + 1;
                
                iErrCode = g_pGameEngine->GetEmpireName (piDuplicateKeys[j], &vName);
                if (iErrCode == OK) {
                    strList += vName.GetCharPtr();
                }
                
                strList += " </strong>have the same Session Id<strong>";
                
                AddMessage (strList);
            }
            
            // Free memory
            delete [] piNumDuplicatesInList;
            delete [] piDuplicateKeys;
        }
    }
}


void HtmlRenderer::ReportLoginFailure (IReport* pReport, const char* pszEmpireName) {
    
    SystemConfiguration scConfig;
    if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {
        
        char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);
        sprintf (pszMessage, "Logon failure for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
        
        pReport->WriteReport (pszMessage);
    }
}

void HtmlRenderer::ReportLoginSuccess (IReport* pReport, const char* pszEmpireName) {
    
    SystemConfiguration scConfig;
    if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {
        
        char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);
        sprintf (pszMessage, "Logon success for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
        
        pReport->WriteReport (pszMessage);
    }
}


void HtmlRenderer::ReportEmpireCreation (IReport* pReport, const char* pszEmpireName) {
    
    SystemConfiguration scConfig;
    if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {
        
        char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);
        sprintf (pszMessage, "Creation success for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
        
        pReport->WriteReport (pszMessage);
    }
}

void HtmlRenderer::WriteSystemTitleString() {

    const char* pszEmpireName = m_vEmpireName.GetCharPtr();

    if (pszEmpireName != NULL) {

        m_pHttpResponse->WriteText (pszEmpireName);
        if (pszEmpireName [strlen (pszEmpireName) - 1] == 's') {
            OutputText ("' ");
        } else {
            OutputText ("'s ");
        }
    }

    m_pHttpResponse->WriteText (PageName [m_pgPageId]);
    OutputText (": ");
    WriteVersionString();
}

void HtmlRenderer::WriteSystemHeaders (bool bFileUpload) {
    
    if (bFileUpload) {
        OutputText ("<form method=\"post\" enctype=\"multipart/form-data\">");
    } else {
        OutputText ("<form method=\"post\">");
    }
    
    OutputText ("<input type=\"hidden\" name=\"PageId\" value=\"");
    m_pHttpResponse->WriteText ((int) m_pgPageId);
    OutputText ("\">");
    
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
    
    OutputText (" <font size=\"+3\"><strong>");
    m_pHttpResponse->WriteText (pszName);
    
    if (pszName [strlen (pszName) - 1] == 's') {
        OutputText ("' ");
    } else {
        OutputText ("'s ");
    }
    m_pHttpResponse->WriteText (PageName [m_pgPageId]);
    
    if (m_iSystemOptions & EMPIRE_MARKED_FOR_DELETION) {
        OutputText (" (Empire marked for deletion)");
    }
    
    OutputText ("</strong></font><p>");
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
        (g_pGameEngine->GetEmpirePersonalGameClasses (m_iEmpireKey, NULL, NULL, (int*) &iNumber) == OK && iNumber > 0)
        ) {

        WriteButton (BID_PERSONALGAMECLASSES);
    }

    // Personal Tournaments
    if (iPrivilege >= PRIVILEGE_FOR_PERSONAL_TOURNAMENTS ||
        (g_pGameEngine->GetOwnedTournaments (m_iEmpireKey, NULL, NULL, &iNumber) == OK && iNumber > 0)
        ) {

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



void HtmlRenderer::WriteBackupMessage() {
    
    int iErrCode;

    String strTimeElapsed, strTimeRemaining;

    DatabaseBackupStage dbsStage;
    
    unsigned int iNumber;
    Seconds iElapsedTime;

    g_pGameEngine->GetDatabaseBackupProgress (&dbsStage, &iElapsedTime, &iNumber);

    char pszElapsed [MAX_HTML_TIME_LENGTH] = "";

    iErrCode = ConvertTime (iElapsedTime, pszElapsed);
    if (iErrCode != OK) {
        pszElapsed[0] = '\0';
    }

    AddMessage ("Access to the server is denied. The database is being backed up.");

    switch (dbsStage) {

    case DATABASE_BACKUP_TABLES:

        AppendMessage ("<br>");
        AppendMessage ((int) iNumber);
        AppendMessage (" tables are being written to disk.");
        break;

    case DATABASE_BACKUP_TEMPLATES:

        AppendMessage ("<br>");
        AppendMessage ((int) iNumber);
        AppendMessage (" templates are being written to disk.");
        break;

    case DATABASE_BACKUP_VARIABLE_LENGTH_DATA:

        AppendMessage ("<br>The variable length data is being written to disk.");
        break;

    case DATABASE_BACKUP_METADATA:

        AppendMessage ("<br>The variable length data is being written to disk.");
        break;

    default:

        break;
    }

    if (pszElapsed[0] != '\0') {

        AppendMessage ("<br>Elapsed time is ");
        AppendMessage (pszElapsed);
    }
}

int HtmlRenderer::InitializeSessionId (bool* pbUpdateSessionId, bool* pbUpdateCookie) {

    int iErrCode;

    // Check for force reset
    if (m_iSystemOptions & RESET_SESSION_ID) {

        iErrCode = g_pGameEngine->GetNewSessionId (&m_i64SessionId);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = g_pGameEngine->EndResetEmpireSessionId (m_iEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        *pbUpdateSessionId = *pbUpdateCookie = true;
        return OK;
    }
    
    *pbUpdateSessionId = *pbUpdateCookie = false;
    
    // First, get the empire's session id
    iErrCode = g_pGameEngine->GetEmpireSessionId (m_iEmpireKey, &m_i64SessionId);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Now, see if the request brought a cookie with a session id
    ICookie* pCookie = m_pHttpRequest->GetCookie ("SessionId");
    int64 i64CookieSessionId = NO_SESSION_ID;
    
    if (pCookie != NULL) {
        i64CookieSessionId = pCookie->GetInt64Value();
    }
    
    if (i64CookieSessionId == NO_SESSION_ID) {
        
        if (m_i64SessionId == NO_SESSION_ID) {
            
            // Generate a new session id
            iErrCode = g_pGameEngine->GetNewSessionId (&m_i64SessionId);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            *pbUpdateSessionId = true;
        }
        
        *pbUpdateCookie = true;
        
    } else {
        
        // We have a cookie with a valid value
        if (i64CookieSessionId != m_i64SessionId) {
            
            // For some reason the cookie has a different value than the stored value
            // This could happen if a player is on a new machine and uses a different empire
            // to generate a new value, uses the original empire, then reverts to the old machine 
            // with the old empire.  We'll believe the cookie's value, in this case, to ensure that
            // all empires on the same machine have the same session id
            //
            // This decision will have to be revised if we absolutely require that session id's be unique
            // in the future for some feature or other, since people can easily edit their cookie files and 
            // change the values
            
            m_i64SessionId = i64CookieSessionId;
            
            // Write the empire's new session id
            *pbUpdateSessionId = true;
        }
    }
    
    return iErrCode;
}



bool HtmlRenderer::RedirectOnSubmit (PageId* ppageRedirect) {
    
    if (WasButtonPressed (PageButtonId[m_pgPageId])) {
        return false;
    }
    
    if (WasButtonPressed (BID_ACTIVEGAMELIST)) {
        *ppageRedirect = ACTIVE_GAME_LIST;
        return true;
    }
    
    if (WasButtonPressed (BID_OPENGAMELIST)) {
        *ppageRedirect = OPEN_GAME_LIST;
        return true;
    }
    
    if (WasButtonPressed (BID_SYSTEMGAMELIST)) {
        *ppageRedirect = SYSTEM_GAME_LIST;
        return true;
    }
    
    if (WasButtonPressed (BID_PROFILEVIEWER)) {
        *ppageRedirect = PROFILE_VIEWER;
        return true;
    }
    
    if (WasButtonPressed (BID_PROFILEEDITOR)) {
        *ppageRedirect = PROFILE_EDITOR;
        return true;
    }
    
    if (WasButtonPressed (BID_TOPLISTS)) {
        *ppageRedirect = TOP_LISTS;
        return true;
    }

    if (WasButtonPressed (BID_SPECTATORGAMES)) {
        *ppageRedirect = SPECTATOR_GAMES;
        return true;
    }

    if (WasButtonPressed (BID_TOURNAMENTS)) {
        m_iReserved = NO_KEY;
        *ppageRedirect = TOURNAMENTS;
        return true;
    }

    if (WasButtonPressed (BID_LATESTGAMES)) {
        *ppageRedirect = LATEST_GAMES;
        return true;
    }
    
    if (WasButtonPressed (BID_LATESTNUKES)) {
        *ppageRedirect = LATEST_NUKES;
        return true;
    }
    
    if (WasButtonPressed (BID_CHATROOM)) {
        *ppageRedirect = CHATROOM;
        return true;
    }
    
    if (WasButtonPressed (BID_EXIT)) {
        if (m_pgeLock != NULL) {
            m_pgeLock->SetActive (false);
        }
        *ppageRedirect = LOGIN;
        return true;
    }
    
    if (WasButtonPressed (BID_PERSONALGAMECLASSES)) {
        *ppageRedirect = PERSONAL_GAME_CLASSES;
        return true;
    }

    if (WasButtonPressed (BID_PERSONALTOURNAMENTS)) {
        *ppageRedirect = PERSONAL_TOURNAMENTS;
        return true;
    }
    
    if (m_iPrivilege >= ADMINISTRATOR) {
        
        if (WasButtonPressed (BID_SERVERADMINISTRATOR)) {
            *ppageRedirect = SERVER_ADMINISTRATOR;
            return true;
        }
        
        if (WasButtonPressed (BID_GAMEADMINISTRATOR)) {
            *ppageRedirect = GAME_ADMINISTRATOR;
            return true;
        }
        
        if (WasButtonPressed (BID_EMPIREADMINISTRATOR)) {
            *ppageRedirect = EMPIRE_ADMINISTRATOR;
            return true;
        }
        
        if (WasButtonPressed (BID_THEMEADMINISTRATOR)) {
            *ppageRedirect = THEME_ADMINISTRATOR;
            return true;
        }

        if (WasButtonPressed (BID_TOURNAMENTADMINISTRATOR)) {
            *ppageRedirect = TOURNAMENT_ADMINISTRATOR;
            return true;
        }
    }
    
    if (WasButtonPressed (BID_SERVERINFORMATION)) {
        *ppageRedirect = SYSTEM_SERVER_INFORMATION;
        return true;
    }
    
    if (WasButtonPressed (BID_DOCUMENTATION)) {
        *ppageRedirect = SYSTEM_DOCUMENTATION;
        return true;
    }
    
    if (WasButtonPressed (BID_SERVERNEWS)) {
        *ppageRedirect = SYSTEM_NEWS;
        return true;
    }

    if (WasButtonPressed (BID_CONTRIBUTIONS)) {
        *ppageRedirect = SYSTEM_CONTRIBUTIONS;
        return true;
    }
    
    if (WasButtonPressed (BID_CREDITS)) {
        *ppageRedirect = SYSTEM_CREDITS;
        return true;
    }
    
    IHttpForm* pHttpForm = m_pHttpRequest->GetForm ("ProfileLink.x");
    if (pHttpForm != NULL) {
        
        m_iReserved = m_iEmpireKey;
        *ppageRedirect = PROFILE_VIEWER;
        return true;
    }
    
    if (NotifiedProfileLink()) {
        
        pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ProfileLink");
        if (pHttpForm != NULL) {
            
            const char* pszProfile = pHttpForm->GetName();
            
            Assert (pszProfile != NULL);
            
            int iViewProfileEmpireKey;
            unsigned int iHash;

            if (sscanf (pszProfile, "ProfileLink.%d.%d.x", &iViewProfileEmpireKey, &iHash) == 2 &&
                VerifyEmpireNameHash (iViewProfileEmpireKey, iHash)) {
                
                m_iReserved = iViewProfileEmpireKey;
                *ppageRedirect = PROFILE_VIEWER;
                return true;
                
            } else {
                
                AddMessage ("That empire no longer exists");
            }
        }
    }

    if (NotifiedTournamentInvitation()) {

        int iErrCode, iMessageKey, iTournamentKey;

        pHttpForm = m_pHttpRequest->GetForm ("HintTournamentInvite");
        if (pHttpForm != NULL && 
            sscanf (pHttpForm->GetValue(), "%i.%i", &iMessageKey, &iTournamentKey) == 2) {

            if (WasButtonPressed (BID_VIEWTOURNAMENTINFORMATION)) {
                m_iReserved = iTournamentKey;
                *ppageRedirect = TOURNAMENTS;
                return true;
            }

            bool bAccept = WasButtonPressed (BID_ACCEPT);
            bool bDecline = bAccept ? false : WasButtonPressed (BID_DECLINE);

            if (bAccept || bDecline) {

                iErrCode = g_pGameEngine->RespondToTournamentInvitation (m_iEmpireKey, iMessageKey, bAccept);
                if (iErrCode != OK) {

                    if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT) {
                        AddMessage ("You are already in the tournament");
                    }

                    else if (iErrCode == ERROR_MESSAGE_DOES_NOT_EXIST) {
                        AddMessage ("You have already responded to that message");
                    }
                    
                    else {
                        AddMessage ("Your response could not be processed. The tournament may no longer exist");
                    }

                } else {
                    
                    if (bAccept) {
                        AddMessage ("You have joined the tournament");
                    } else {
                        AddMessage ("You have declined to join the tournament");
                    }
                }
            }
        }
    }

    if (NotifiedTournamentJoinRequest()) {

        int iErrCode, iMessageKey, iTournamentKey;

        pHttpForm = m_pHttpRequest->GetForm ("HintTournamentJoinRequest");
        if (pHttpForm != NULL && 
            sscanf (pHttpForm->GetValue(), "%i.%i", &iMessageKey, &iTournamentKey) == 2) {

            if (WasButtonPressed (BID_VIEWTOURNAMENTINFORMATION)) {
                m_iReserved = iTournamentKey;
                *ppageRedirect = TOURNAMENTS;
                return true;
            }

            bool bAccept = WasButtonPressed (BID_ACCEPT);
            bool bDecline = bAccept ? false : WasButtonPressed (BID_DECLINE);

            if (bAccept || bDecline) {

                iErrCode = g_pGameEngine->RespondToTournamentJoinRequest (m_iEmpireKey, iMessageKey, bAccept);
                if (iErrCode != OK) {

                    if (iErrCode == ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT) {
                        AddMessage ("The empire is already in the tournament");
                    }

                    else if (iErrCode == ERROR_MESSAGE_DOES_NOT_EXIST) {
                        AddMessage ("You have already responded to that message");
                    }
                    
                    else {
                        AddMessage ("Your response could not be processed. The tournament may no longer exist");
                    }

                } else {

                    if (bAccept) {
                        AddMessage ("You have accepted the empire into tournament");
                    } else {
                        AddMessage ("You have declined to accept the empire into the tournament");
                    }
                }
            }
        }
    }
    
    return false;
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
    }
    
    OutputText ("<p><strong><font size=\"3\">");
    WriteVersionString();
    OutputText ("<br>Script time: ");
    
    MilliSeconds msTime = GetTimerCount();
    
    OnPageRender (msTime);
    
    m_pHttpResponse->WriteText ((int) msTime);
    OutputText (" ms</font></strong></center></form></body></html>");
}

int HtmlRenderer::PostSystemPageInformation() {
    
    int64 i64PasswordHash = 0;
    int iErrCode = GetPasswordHashForSystemPage (m_tNewSalt, &i64PasswordHash);
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    OutputText ("<input type=\"hidden\" name=\"EmpireKey\" value=\"");
    m_pHttpResponse->WriteText (m_iEmpireKey);
    OutputText ("\"><input type=\"hidden\" name=\"Password\" value=\"");
    m_pHttpResponse->WriteText (i64PasswordHash);
    OutputText ("\"><input type=\"hidden\" name=\"Salt\" value=\"");
    m_pHttpResponse->WriteText (m_tNewSalt);
    OutputText ("\">");

    return OK;
}


void HtmlRenderer::WriteCreateGameClassString (int iEmpireKey, unsigned int iTournamentKey, bool bPersonalGame) {

    int* piSuperClassKey, iNumSuperClasses, i, iErrCode, iMaxNumShips;
    Variant* pvSuperClassName;

    IHttpForm* pHttpForm;

    if (iTournamentKey != NO_KEY) {
        Assert (!bPersonalGame);
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


    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassName")) != NULL) {
        if (HTMLFilter (pHttpForm->GetValue(), &strName, 0, false) != OK) {
            strName = "";
        }
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassDescription")) != NULL) {
        if (HTMLFilter (pHttpForm->GetValue(), &strDesc, 0, false) != OK) {
            strDesc = "";
        }
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
        "<textarea rows=\"3\" cols=\"50\" wrap=\"physical\" name=\"GameClassDescription\">"
        );

    m_pHttpResponse->WriteText (strDesc.GetCharPtr(), strDesc.GetLength());
        
    OutputText ("</textarea></td></tr>");

    Variant vMaxResourcesPerPlanet, vMaxInitialTechLevel, vMaxTechDev,
        vUnlimitedEmpirePrivilege = ADMINISTRATOR;
    
    int iMinNumSecsPerUpdate, iMaxNumSecsPerUpdate, iMaxNumEmpires, iMaxNumPlanets, iActualMaxNumEmpires;
    
    if (iEmpireKey == SYSTEM) {
        
        iErrCode = g_pGameEngine->GetSuperClassKeys (&piSuperClassKey, &pvSuperClassName, &iNumSuperClasses);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        if (iNumSuperClasses > 0) {
            
            OutputText ("<tr><td>Super Class:</td><td><select name=\"SuperClassKey\">");
            
            for (i = 0; i < iNumSuperClasses; i ++) {       
                OutputText ("<option");

                if (iSelSuperClass == piSuperClassKey[i]) {
                    OutputText (" selected");
                }

                OutputText (" value=\"");
                m_pHttpResponse->WriteText (piSuperClassKey[i]);
                OutputText ("\">");
                m_pHttpResponse->WriteText (pvSuperClassName[i].GetCharPtr());
                OutputText ("</option>");
            }
            
            g_pGameEngine->FreeKeys ((unsigned int*) piSuperClassKey);
            g_pGameEngine->FreeData (pvSuperClassName);
        }
        
        OutputText ("</select></td></tr>");
        
        iErrCode = g_pGameEngine->GetMinNumSecsPerUpdateForSystemGameClass (&iMinNumSecsPerUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumSecsPerUpdateForSystemGameClass (&iMaxNumSecsPerUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumEmpiresForSystemGameClass (&iMaxNumEmpires);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iActualMaxNumEmpires = UNLIMITED_EMPIRES;
        
        iErrCode = g_pGameEngine->GetMaxNumPlanetsForSystemGameClass (&iMaxNumPlanets);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxResourcesPerPlanet, &vMaxResourcesPerPlanet);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxInitialTechLevel, &vMaxInitialTechLevel);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxTechDev, &vMaxTechDev);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
    } else {
        
        iErrCode = g_pGameEngine->GetMinNumSecsPerUpdateForPersonalGameClass (&iMinNumSecsPerUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumSecsPerUpdateForPersonalGameClass (&iMaxNumSecsPerUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumEmpiresForPersonalGameClass (&iMaxNumEmpires);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::PrivilegeForUnlimitedEmpires, &vUnlimitedEmpirePrivilege);

        if (m_iPrivilege >= vUnlimitedEmpirePrivilege.GetInteger()) {
            iActualMaxNumEmpires = UNLIMITED_EMPIRES;
        } else {
            iActualMaxNumEmpires = min (iMaxNumEmpires, 8);
        }

        iErrCode = g_pGameEngine->GetMaxNumPlanetsForPersonalGameClass (&iMaxNumPlanets);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, &vMaxResourcesPerPlanet);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxInitialTechLevelPersonal, &vMaxInitialTechLevel);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxTechDevPersonal, &vMaxTechDev);
        if (iErrCode != OK) {
            goto Cleanup;
        }
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

            sprintf (pszShip, "InitShip%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszShip)) != NULL) {
                iSelInitShip |= TECH_BITS[i];
            }

            sprintf (pszShip, "DevShip%i", i);
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
        iSelMapExposed = pHttpForm->GetIntValue() != 0;
    } else {
        iSelMapExposed = 0;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("DisconnectedMaps")) != NULL) {
        bSelDisconnectedMaps = pHttpForm->GetIntValue() != 0;
    } else {
        bSelDisconnectedMaps = false;
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("DipShareLevel")) != NULL) {
        iSelDipShareLevel = pHttpForm->GetIntValue() != 0;
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

    OutputText (
        " value=\"");
    m_pHttpResponse->WriteText (ALLIANCE);
    OutputText ("\">Maps shared at " ALLIANCE_STRING "</option><option"
        );
    
    if (iSelDipShareLevel == NO_DIPLOMACY) {
        OutputText (" selected");
    }

    OutputText (
        " value=\"");
    m_pHttpResponse->WriteText (NO_DIPLOMACY);
    OutputText ("\">Maps not shared</option></select></td>"\
        
        "</tr>"\
        
        "</table>");
    
    return;
    
Cleanup:
    
    OutputText ("<p>An error occurred reading data from the system database. Please contact the administrator<p>");
}


int HtmlRenderer::ProcessCreateGameClassForms (unsigned int iOwnerKey, unsigned int iTournamentKey) {
    
    int iErrCode, iGameClass;

    Variant pvSubmitArray [SystemGameClassData::NumColumns];

    // Verify security
    if (iTournamentKey != NO_KEY) {

        unsigned int iRealOwner;

        iErrCode = g_pGameEngine->GetTournamentOwner (iTournamentKey, &iRealOwner);
        if (iErrCode != OK) {
            AddMessage ("That tournament no longer exists");
            return iErrCode;
        }

        if (iRealOwner != iOwnerKey) {
            AddMessage ("That empire does not own that tournament");
            return ERROR_WRONG_TOURNAMENT_OWNER;
        }
    }


    // Parse the forms
    iErrCode = ParseCreateGameClassForms (pvSubmitArray, iOwnerKey, iTournamentKey, false);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Create the gameclass, finally
    iErrCode = g_pGameEngine->CreateGameClass (iOwnerKey, pvSubmitArray, &iGameClass);
    switch (iErrCode) {

    case OK:
        AddMessage ("The GameClass was created");
        break;
    case ERROR_GAMECLASS_ALREADY_EXISTS:
        AddMessage ("The new GameClass name already exists");
        break;
    case ERROR_NAME_IS_TOO_LONG:
        AddMessage ("The new GameClass name is too long");
        break;
    case ERROR_DESCRIPTION_IS_TOO_LONG:
        AddMessage ("The new GameClass description is too long");
        break;
    case ERROR_TOO_MANY_GAMECLASSES_IN_TOURNAMENT:
        AddMessage ("The limit for GameClasses in a personal tournament has been exceeded");
        break;
    default:
        AddMessage ("The GameClass could not be created; the error was ");
        AppendMessage (iErrCode);
        break;
    }
    
    return iErrCode;
}


int HtmlRenderer::ProcessCreateDynamicGameClassForms (unsigned int iOwnerKey, int* piGameClass, int* piGameNumber, 
                                                      bool* pbGameCreated) {
    
    int iErrCode;
    
    Variant pvSubmitArray [SystemGameClassData::NumColumns];
    
    GameOptions goOptions;
    InitGameOptions (&goOptions);
    
    *pbGameCreated = false;
    
    // Parse the forms
    iErrCode = ParseCreateGameClassForms (pvSubmitArray, iOwnerKey, NO_KEY, true);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    iErrCode = ParseGameConfigurationForms (NO_KEY, NO_KEY, pvSubmitArray, iOwnerKey, &goOptions);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    if (goOptions.sFirstUpdateDelay > pvSubmitArray[SystemGameClassData::NumSecPerUpdate].GetInteger() * 10) {
        AddMessage ("The first update delay is too large");
        iErrCode = ERROR_FAILURE;
        goto Cleanup;
    }
    
    // Dynamic gameclass
    pvSubmitArray[SystemGameClassData::Options] = 
        pvSubmitArray[SystemGameClassData::Options].GetInteger() | DYNAMIC_GAMECLASS;
    
    // Create the gameclass
    iErrCode = g_pGameEngine->CreateGameClass (iOwnerKey, pvSubmitArray, piGameClass);
    if (iErrCode != OK) {
        
        switch (iErrCode) {
            
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
            AddMessage ("The game could not be created; the error was ");
            AppendMessage (iErrCode);
            break;
        }
        
        goto Cleanup;
    }

    // Bridier sanity
    Assert (pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger() == 2 || 
        !(goOptions.iOptions & GAME_COUNT_FOR_BRIDIER));

    // Spectator sanity
    Assert (
        ((pvSubmitArray[SystemGameClassData::Options].GetInteger() & EXPOSED_SPECTATORS) == 
        EXPOSED_SPECTATORS) || 
        !(goOptions.iOptions & GAME_ALLOW_SPECTATORS));

    // Try to create the game
    *pbGameCreated = true;

    goOptions.iNumEmpires = 1;
    goOptions.piEmpireKey = &iOwnerKey;
    
    iErrCode = g_pGameEngine->CreateGame (*piGameClass, iOwnerKey, goOptions, piGameNumber);
    if (iErrCode == OK) {
        
        // Halt the gameclass
        int iErrCode2 = g_pGameEngine->HaltGameClass (*piGameClass);
        Assert (iErrCode2 == OK);
        
    } else {
        
        // Delete the gameclass
        bool bDeleted;
        int iErrCode2 = g_pGameEngine->DeleteGameClass (*piGameClass, &bDeleted);
        Assert (iErrCode2 == OK);
    }

Cleanup:

    ClearGameOptions (&goOptions);

    return iErrCode;
}


int HtmlRenderer::ParseCreateGameClassForms (Variant* pvSubmitArray, int iOwnerKey, unsigned int iTournamentKey, 
                                             bool bDynamic) {
    
    IHttpForm* pHttpForm;
    
    Variant vMaxResourcesPerPlanet;

    int iOptions = 0, iTemp, iMinNumSecsPerUpdate, iMaxNumSecsPerUpdate, iMaxNumEmpires, iMaxNumPlanets, iErrCode,
        iNumInitTechDevs, iNumTechDevs, i, iInitTechDevs = 0, iDevTechDevs = 0, iDip;
    
    char pszTechString [64];    
    const char* pszString;
    
    if (iOwnerKey == SYSTEM) {
        
        iErrCode = g_pGameEngine->GetMinNumSecsPerUpdateForSystemGameClass (&iMinNumSecsPerUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumSecsPerUpdateForSystemGameClass (&iMaxNumSecsPerUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumEmpiresForSystemGameClass (&iMaxNumEmpires);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumPlanetsForSystemGameClass (&iMaxNumPlanets);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxResourcesPerPlanet, &vMaxResourcesPerPlanet);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
    } else {
        
        iErrCode = g_pGameEngine->GetMinNumSecsPerUpdateForPersonalGameClass (&iMinNumSecsPerUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumSecsPerUpdateForPersonalGameClass (&iMaxNumSecsPerUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumEmpiresForPersonalGameClass (&iMaxNumEmpires);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetMaxNumPlanetsForPersonalGameClass (&iMaxNumPlanets);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, &vMaxResourcesPerPlanet);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }
    
    // GameClassName
    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassName")) == NULL) {
        AddMessage ("Missing GameClassName form");
        return ERROR_FAILURE;
    }
    
    if (pHttpForm->GetValue() == NULL) {
        AddMessage ("The name cannot be blank");
        return ERROR_FAILURE;
    }
    
    pszString = pHttpForm->GetValue();
    
    if (VerifyCategoryName ("Gameclass", pszString, MAX_GAME_CLASS_NAME_LENGTH, true) != OK) {
        return ERROR_FAILURE;
    }
    
    pvSubmitArray[SystemGameClassData::Name] = pszString;
    
    // Description
    if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassDescription")) == NULL) {
        AddMessage ("Missing GameClassDescription form");
        return ERROR_FAILURE;
    }
    
    pszString = pHttpForm->GetValue();
    
    if (pszString == NULL) {
        pvSubmitArray[SystemGameClassData::Description] = "";
    } else {
        
        if (strlen (pszString) > MAX_GAMECLASS_DESCRIPTION_LENGTH) {
            AddMessage ("The description is too long");
            return ERROR_FAILURE;
        }
        
        pvSubmitArray[SystemGameClassData::Description] = pszString;
    }

    // MinNumEmpires
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumEmpires")) == NULL) {
        AddMessage ("Missing MinNumEmpires form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = pHttpForm->GetIntValue();

    // MaxNumEmpires
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumEmpires")) == NULL) {
        AddMessage ("Missing MaxNumEmpires form");
        return ERROR_FAILURE;
    }

    if (String::StriCmp (pHttpForm->GetValue(), UNLIMITED_STRING) == 0) {
        pvSubmitArray[SystemGameClassData::MaxNumEmpires] = UNLIMITED_EMPIRES;
    } else {
        pvSubmitArray[SystemGameClassData::MaxNumEmpires] = pHttpForm->GetIntValue();
    }

    if (iTournamentKey != NO_KEY && 
        pvSubmitArray[SystemGameClassData::MinNumEmpires].GetInteger() != 
        pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger()) {

        AddMessage ("Tournament games must have an equal minimum and maximum number of empires");
        return ERROR_FAILURE;
    }
    
    // NumPlanetsPerEmpire
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumPlanets")) == NULL) {
        AddMessage ("Missing NumPlanetsPerEmpire form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = pHttpForm->GetIntValue();

    // MapGen
    if ((pHttpForm = m_pHttpRequest->GetForm ("MapGen")) == NULL) {
        AddMessage ("Missing MapGen form");
        return ERROR_FAILURE;
    }

    iTemp = pHttpForm->GetIntValue();
    if (iTemp != 0) {

        if (iTemp != GENERATE_MAP_FIRST_UPDATE) {
            AddMessage ("Invalid MapGen value");
            return ERROR_FAILURE;
        }

        if (pvSubmitArray[SystemGameClassData::MinNumEmpires].GetInteger() == 
            pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger()) {

            AddMessage ("The map must be generated when the game starts if the number of empires is fixed");
            return ERROR_FAILURE;
        }

        iOptions |= GENERATE_MAP_FIRST_UPDATE;
    }

    // TechIncrease
    if ((pHttpForm = m_pHttpRequest->GetForm ("TechIncrease")) == NULL) {
        AddMessage ("Missing TechIncrease form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxTechDev] = pHttpForm->GetFloatValue();
    
    // OpenGameNum
    pvSubmitArray[SystemGameClassData::OpenGameNum] = 1;
    
    // SecsPerUpdate //

    // NumInitTechDevs
    if ((pHttpForm = m_pHttpRequest->GetForm ("NumInitTechDevs")) == NULL) {
        AddMessage ("Missing NumInitTechDevs form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = pHttpForm->GetIntValue();
    
    // Hours per update
    if ((pHttpForm = m_pHttpRequest->GetForm ("HoursPU")) == NULL) {
        AddMessage ("Missing HoursPU form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * pHttpForm->GetIntValue();
    
    // Minutes per update
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinsPU")) == NULL) {
        AddMessage ("Missing MinsPU form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] += 60 * pHttpForm->GetIntValue();
    
    // Seconds per update
    if ((pHttpForm = m_pHttpRequest->GetForm ("SecsPU")) == NULL) {
        AddMessage ("Missing SecsPU form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] += pHttpForm->GetIntValue();
    
    // Weekend updates
    if ((pHttpForm = m_pHttpRequest->GetForm ("Weekend")) == NULL) {
        AddMessage ("Missing Weekend form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= WEEKEND_UPDATES;
    }
    
    // InitTechLevel
    if ((pHttpForm = m_pHttpRequest->GetForm ("InitTechLevel")) == NULL) {
        AddMessage ("Missing InitTechLevel form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = pHttpForm->GetFloatValue();
    
    // BuilderPopLevel
    if ((pHttpForm = m_pHttpRequest->GetForm ("PopLevel")) == NULL) {
        AddMessage ("Missing PopLevel form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = pHttpForm->GetIntValue();
    
    // MaxAgRatio
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAgRatio")) == NULL) {
        AddMessage ("Missing MaxAgRatio form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = pHttpForm->GetFloatValue();
    
    // MaxNumShips
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumShips")) == NULL) {
        AddMessage ("Missing MaxNumShips form");
        return ERROR_FAILURE;
    }

    if (String::StriCmp (pHttpForm->GetValue(), UNLIMITED_STRING) == 0) {
        pvSubmitArray[SystemGameClassData::MaxNumShips] = INFINITE_SHIPS;
    } else {

        iTemp = pHttpForm->GetIntValue();

        if (iTemp < 1) {
            AddMessage ("Invalid maximum number of ships");
            return ERROR_FAILURE;
        }
    
        pvSubmitArray[SystemGameClassData::MaxNumShips] = iTemp;
    }

    // Draws
    if ((pHttpForm = m_pHttpRequest->GetForm ("Draws")) == NULL) {
        AddMessage ("Missing Draws form");
        return ERROR_FAILURE;
    }
    
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= ALLOW_DRAW;
    }

    // Surrenders
    iTemp = 0;

    if ((pHttpForm = m_pHttpRequest->GetForm ("Surrenders")) == NULL) {
        AddMessage ("Missing Surrenders form");
        return ERROR_FAILURE;
    }
    
    iDip = pHttpForm->GetIntValue();
    
    switch (iDip) {
    case 3:

        iOptions |= USE_SC30_SURRENDERS;
        break;
        
    case 2:

        if (pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger() == UNLIMITED_EMPIRES ||
            pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger() > 2) {
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
        AddMessage ("Missing Dip form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = pHttpForm->GetIntValue() | iTemp;
    
    iDip = pvSubmitArray[SystemGameClassData::DiplomacyLevel].GetInteger();
    
    // BreakAlliances
    if ((pHttpForm = m_pHttpRequest->GetForm ("BreakAlliances")) == NULL) {
        AddMessage ("Missing BreakAlliances form");
        return ERROR_FAILURE;
    }
    
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= UNBREAKABLE_ALLIANCES;
    }
    
    // PermanentAlliances
    if ((pHttpForm = m_pHttpRequest->GetForm ("PermanentAlliances")) == NULL) {
        AddMessage ("Missing PermanentAlliances form");
        return ERROR_FAILURE;
    }
    
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= PERMANENT_ALLIANCES;
    }
    
    // VisibleDiplomacy
    if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleDiplomacy")) == NULL) {
        AddMessage ("Missing VisibleDiplomacy form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= VISIBLE_DIPLOMACY;
    }

    // FriendlyGates
    if ((pHttpForm = m_pHttpRequest->GetForm ("FriendlyGates")) == NULL) {
        AddMessage ("Missing FriendlyGates form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= USE_FRIENDLY_GATES;
    }

    // FriendlyScis
    if ((pHttpForm = m_pHttpRequest->GetForm ("FriendlyScis")) == NULL) {
        AddMessage ("Missing FriendlyScis form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= DISABLE_SCIENCE_SHIPS_NUKING;
    }
    
    // SuicidalDoomsdays
    if ((pHttpForm = m_pHttpRequest->GetForm ("SuicidalDoomsdays")) == NULL) {
        AddMessage ("Missing SuicidalDoomsdays form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() == 0) {
        iOptions |= DISABLE_SUICIDAL_DOOMSDAYS;
    }

    // UnfriendlyDoomsdays
    if ((pHttpForm = m_pHttpRequest->GetForm ("UnfriendlyDoomsdays")) == NULL) {
        AddMessage ("Missing UnfriendlyDoomsdays form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= USE_UNFRIENDLY_DOOMSDAYS;
    }

    // ClassicDoomsdays
    if ((pHttpForm = m_pHttpRequest->GetForm ("ClassicDoomsdays")) == NULL) {
        AddMessage ("Missing ClassicDoomsdays form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= USE_CLASSIC_DOOMSDAYS;
    }
    
    // Independence
    if ((pHttpForm = m_pHttpRequest->GetForm ("Independence")) == NULL) {
        AddMessage ("Missing Independence form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= INDEPENDENCE;
    }
    
    // MapExposed and FullyCol
    if ((pHttpForm = m_pHttpRequest->GetForm ("MapExposed")) == NULL) {
        AddMessage ("Missing MapExposed form");
        return ERROR_FAILURE;
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
        AddMessage ("Illegal MapExposed value");
        return ERROR_FAILURE;
    }
    
    // DisconnectedMaps
    if ((pHttpForm = m_pHttpRequest->GetForm ("DisconnectedMaps")) == NULL) {
        AddMessage ("Missing DisconnectedMaps form");
        return ERROR_FAILURE;
    }
    
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= DISCONNECTED_MAP;
    }
    
    // DipShareLevel
    if ((pHttpForm = m_pHttpRequest->GetForm ("DipShareLevel")) == NULL) {
        AddMessage ("Missing DipShareLevel form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MapsShared] = pHttpForm->GetIntValue();
    if (pvSubmitArray[SystemGameClassData::MapsShared] != NO_DIPLOMACY &&
        (pvSubmitArray[SystemGameClassData::MapsShared] < TRUCE ||
        pvSubmitArray[SystemGameClassData::MapsShared] > ALLIANCE)) {
        
        AddMessage ("Illegal value for DipShareLevel form");
        return ERROR_FAILURE;
    }
    
    // Owner and tournament
    if (iTournamentKey == NO_KEY) {

        if (bDynamic) {
            pvSubmitArray [SystemGameClassData::Owner] = PERSONAL_GAME;
        } else {
            pvSubmitArray [SystemGameClassData::Owner] = iOwnerKey;
        }
        pvSubmitArray [SystemGameClassData::TournamentKey] = NO_KEY;

    } else {
        
        pvSubmitArray [SystemGameClassData::Owner] = TOURNAMENT;
        pvSubmitArray [SystemGameClassData::TournamentKey] = iTournamentKey;
    }

    // Owner name
    if (iTournamentKey != NO_KEY) {

        iErrCode = g_pGameEngine->GetTournamentName (iTournamentKey, pvSubmitArray + SystemGameClassData::OwnerName);
        if (iErrCode != OK) {
            AddMessage ("That tournament no longer exists");
            return iErrCode;
        }
    
    } else {

        switch (iOwnerKey) {
            
        case SYSTEM:

            if (iTournamentKey == NO_KEY) {
                pvSubmitArray [SystemGameClassData::OwnerName] = "";
                break;
            }

        default:

            iErrCode = g_pGameEngine->GetEmpireName (iOwnerKey, pvSubmitArray + SystemGameClassData::OwnerName);
            if (iErrCode != OK) {
                AddMessage ("The creator's name could not be read");
                return iErrCode;
            }
            break;
        }
    }
    
    // Private messages
    if ((pHttpForm = m_pHttpRequest->GetForm ("Private")) == NULL) {
        AddMessage ("Missing Private form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= PRIVATE_MESSAGES;
    }
    
    // DipExposed
    if ((pHttpForm = m_pHttpRequest->GetForm ("DipExposed")) == NULL) {
        AddMessage ("Missing DipExposed form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= EXPOSED_DIPLOMACY;
    }
    
    // Visible builds
    if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleBuilds")) == NULL) {
        AddMessage ("Missing VisibleBuilds form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= VISIBLE_BUILDS;
    }
    
    // Subjective views
    if ((pHttpForm = m_pHttpRequest->GetForm ("Subjective")) == NULL) {
        AddMessage ("Missing Subjective form");
        return ERROR_FAILURE;
    }
    if (pHttpForm->GetIntValue() != 0) {
        iOptions |= SUBJECTIVE_VIEWS;
    }
    
    // SuperClassKey
    if (iOwnerKey == SYSTEM && iTournamentKey == NO_KEY) {

        if ((pHttpForm = m_pHttpRequest->GetForm ("SuperClassKey")) == NULL) {
            AddMessage ("Missing SuperClassKey form");
            return ERROR_FAILURE;
        }
        pvSubmitArray[SystemGameClassData::SuperClassKey] = pHttpForm->GetIntValue();

    } else {

        if (iTournamentKey != NO_KEY) {
            pvSubmitArray[SystemGameClassData::SuperClassKey] = TOURNAMENT;
        } else {
            pvSubmitArray[SystemGameClassData::SuperClassKey] = PERSONAL_GAME;
        }
    }

    // MaxNumTruces
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTruces")) == NULL) {
        AddMessage ("Missing MaxNumTruces form");
        return ERROR_FAILURE;
    }

    iTemp = pHttpForm->GetIntValue();
    
    switch (iTemp) {
        
    case UNRESTRICTED_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
        break;
        
    case FAIR_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::MaxNumTruces] = FAIR_DIPLOMACY;
        break;
        
    case STATIC_RESTRICTION:
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTruces")) == NULL) {
            AddMessage ("Missing StaticTruces form");
            return ERROR_FAILURE;
        }
        pvSubmitArray[SystemGameClassData::MaxNumTruces] = pHttpForm->GetIntValue();
        break;
        
    default:
        
        AddMessage ("Illegal MaxNumTruces value");
        return ERROR_FAILURE;
    }
    
    // MaxNumTrades
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTrades")) == NULL) {
        AddMessage ("Missing MaxNumTrades form");
        return ERROR_FAILURE;
    }
    iTemp = pHttpForm->GetIntValue();
    
    switch (iTemp) {
        
    case UNRESTRICTED_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
        break;
        
    case FAIR_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::MaxNumTrades] = FAIR_DIPLOMACY;
        break;
        
    case STATIC_RESTRICTION:
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTrades")) == NULL) {
            AddMessage ("Missing StaticTrades form");
            return ERROR_FAILURE;
        }
        pvSubmitArray[SystemGameClassData::MaxNumTrades] = pHttpForm->GetIntValue();
        break;
        
    default:
        
        AddMessage ("Illegal MaxNumTrades value");
        return ERROR_FAILURE;
    }
    
    // MaxNumAllies
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumAllies")) == NULL) {
        AddMessage ("Missing MaxNumAllies form");
        return ERROR_FAILURE;
    }
    iTemp = pHttpForm->GetIntValue();
    
    switch (iTemp) {
        
    case UNRESTRICTED_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
        break;
        
    case FAIR_DIPLOMACY:
        
        pvSubmitArray[SystemGameClassData::MaxNumAlliances] = FAIR_DIPLOMACY;
        break;
        
    case STATIC_RESTRICTION:
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("StaticAllies")) == NULL) {
            AddMessage ("Missing StaticAllies form");
            return ERROR_FAILURE;
        }
        pvSubmitArray[SystemGameClassData::MaxNumAlliances] = pHttpForm->GetIntValue();
        break;
        
    default:
        
        AddMessage ("Illegal MaxNumAllies value");
        return ERROR_FAILURE;
    }
    
    // MinNumPlanets
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumPlanets")) == NULL) {
        AddMessage ("Missing MinNumPlanets form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = pHttpForm->GetIntValue();

    //
    // MaxResources
    //

    // MaxAg
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAg")) == NULL) {
        AddMessage ("Missing MaxAg form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = pHttpForm->GetIntValue();
    
    // MaxMin
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxMin")) == NULL) {
        AddMessage ("Missing MaxMin form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = pHttpForm->GetIntValue();
    
    // MaxFuel
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxFuel")) == NULL) {
        AddMessage ("Missing MaxFuel form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = pHttpForm->GetIntValue();
    
    //
    // MaxResourcesHW
    //
    
    // MaxAgHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWAg")) == NULL) {
        AddMessage ("Missing MaxHWAg form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxAgHW] = pHttpForm->GetIntValue();
    
    // MaxMinHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWMin")) == NULL) {
        AddMessage ("Missing MaxHWMin form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxMinHW] = pHttpForm->GetIntValue();
    
    // MaxFuelHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWFuel")) == NULL) {
        AddMessage ("Missing MaxHWFuel form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = pHttpForm->GetIntValue();
    
    //
    // MinResources
    //
    
    // MinAg
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinAg")) == NULL) {
        AddMessage ("Missing MinAg form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MinAvgAg] = pHttpForm->GetIntValue();
    
    // MinMin
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinMin")) == NULL) {
        AddMessage ("Missing MinMin form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MinAvgMin] = pHttpForm->GetIntValue();
    
    // MinFuel
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinFuel")) == NULL) {
        AddMessage ("Missing MinFuel form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = pHttpForm->GetIntValue();
    
    //
    // MinResourcesHW
    //
    
    // MinAgHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWAg")) == NULL) {
        AddMessage ("Missing MinHWAg form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MinAgHW] = pHttpForm->GetIntValue();
    
    // MinMinHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWMin")) == NULL) {
        AddMessage ("Missing MinHWMin form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MinMinHW] = pHttpForm->GetIntValue();
    
    // MinFuelHW
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWFuel")) == NULL) {
        AddMessage ("Missing MinHWFuel form");
        return ERROR_FAILURE;
    }
    pvSubmitArray[SystemGameClassData::MinFuelHW] = pHttpForm->GetIntValue();
    
    // NumUpdatesForIdle
    if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesIdle")) == NULL) {
        AddMessage ("Missing UpdatesIdle form");
        return ERROR_FAILURE;
    }
    
    iTemp = pHttpForm->GetIntValue();
    
    if (iTemp < 1 || iTemp > MAX_NUM_UPDATES_BEFORE_IDLE) {
        AddMessage ("Incorrect value for number of idle updates");
        return ERROR_FAILURE;
    }
    
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = iTemp;
    
    // RuinFlags, NumUpdatesForRuin
    if ((pHttpForm = m_pHttpRequest->GetForm ("Ruins")) == NULL) {
        AddMessage ("Missing Ruins form");
        return ERROR_FAILURE;
    }
    
    iTemp = pHttpForm->GetIntValue();
    
    switch (iTemp) {
        
    case 0:
    case RUIN_CLASSIC_SC:
    case RUIN_ALMONASTER:
        
        pvSubmitArray[SystemGameClassData::RuinFlags] = iTemp;
        break;
        
    default:
        
        AddMessage ("Incorrect Ruins value");
        return ERROR_FAILURE;
    }
    
    if (iTemp != 0) {
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesRuin")) == NULL) {
            AddMessage ("Missing UpdatesRuin form");
            return ERROR_FAILURE;
        }
        
        iTemp = pHttpForm->GetIntValue();
        
        if (iTemp < 1) {
            AddMessage ("Incorrect value for number of idle updates before ruin");
            return ERROR_FAILURE;
        }

        if (iTemp < pvSubmitArray[SystemGameClassData::NumUpdatesForIdle].GetInteger()) {
            AddMessage ("The number of idle updates before ruin must be greater than or equal to the number of updates before idle");
            return ERROR_FAILURE;
        }

        if (iTemp > MAX_NUM_UPDATES_BEFORE_RUIN) {
            AddMessage ("The number of idle updates before ruin is too large");
            return ERROR_FAILURE;
        }

        pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = iTemp;
    }
    
    // MaxNumActiveGames
    if ((pHttpForm = m_pHttpRequest->GetForm ("ActiveGames")) == NULL) {
        AddMessage ("Missing ActiveGames form");
        return ERROR_FAILURE;
    }
    
    if (pHttpForm->GetIntValue() == 0) {
        pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    } else {
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("NumActiveGames")) == NULL) {
            AddMessage ("Missing NumActiveGames form");
            return ERROR_FAILURE;
        }
        
        iTemp = pHttpForm->GetIntValue();
        
        if (iTemp < 1) {
            AddMessage ("Incorrect value for number of simultaneous active games");
            return ERROR_FAILURE;
        }
        
        pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = iTemp;
    }
    
    // Options
    pvSubmitArray[SystemGameClassData::Options] = iOptions;
    
    ///////////////////
    // Sanity checks //
    ///////////////////

    iTemp = pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger();
    
    // Truce - trade - alliance limits
    if (pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger() != UNRESTRICTED_DIPLOMACY &&
        pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger() != FAIR_DIPLOMACY &&

        (pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger() < 1 || 
        
        (iTemp != UNLIMITED_EMPIRES &&
        pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger() >= iTemp)
        )
        ) {
        
        AddMessage ("Invalid maximum number of truces");
        return ERROR_FAILURE;
    }
    
    if (pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger() != UNRESTRICTED_DIPLOMACY &&
        pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger() != FAIR_DIPLOMACY &&

        (pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger() < 1 || 
        
        (iTemp != UNLIMITED_EMPIRES &&
        pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger() >= iTemp)
        )
        ) {
        
        AddMessage ("Invalid maximum number of trades");
        return ERROR_FAILURE;
    }
    
    if (pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() != UNRESTRICTED_DIPLOMACY &&
        pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() != FAIR_DIPLOMACY &&
        
        (pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() < 1 || 
        
        (iTemp != UNLIMITED_EMPIRES &&
        pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() >= iTemp)
        )
        ) {
        
        AddMessage ("Invalid maximum number of alliances");
        return ERROR_FAILURE;
    }

    // Number of empires
    if (pvSubmitArray[SystemGameClassData::MinNumEmpires].GetInteger() < 2) {

        AddMessage ("Incorrect minimum number of empires");
        return ERROR_FAILURE;
    }

    if (iTemp != UNLIMITED_EMPIRES &&
        (iTemp < pvSubmitArray[SystemGameClassData::MinNumEmpires].GetInteger() ||
        iTemp > iMaxNumEmpires)) {
        
        AddMessage ("Incorrect maximum number of empires");
        return ERROR_FAILURE;
    }
    
    // Planets per empire
    if (pvSubmitArray[SystemGameClassData::MaxNumPlanets].GetInteger() < 1 || 
        pvSubmitArray[SystemGameClassData::MaxNumPlanets].GetInteger() > iMaxNumPlanets ||
        
        pvSubmitArray[SystemGameClassData::MinNumPlanets].GetInteger() > 
        pvSubmitArray[SystemGameClassData::MaxNumPlanets].GetInteger()) {
        
        AddMessage ("Incorrect number of planets per empire");
        return ERROR_FAILURE;
    }
    
    // Name of gameclass
    if (pvSubmitArray[SystemGameClassData::Name].GetCharPtr() == NULL ||
        *pvSubmitArray[SystemGameClassData::Name].GetCharPtr() == '\0') {
        AddMessage ("GameClass names cannot be blank");
        return ERROR_FAILURE;
    }
    
    // Tech increase
    if (pvSubmitArray[SystemGameClassData::MaxTechDev].GetFloat() < (float) 0.0) {
        AddMessage ("Tech increase cannot be negative");
        return ERROR_FAILURE;
    }
    
    // Tech initial level
    if (pvSubmitArray[SystemGameClassData::InitialTechLevel].GetFloat() < (float) 1.0) {
        AddMessage ("Initial tech level cannot be less than 1.0");
        return ERROR_FAILURE;
    }
    
    // PopLevel
    if (pvSubmitArray[SystemGameClassData::BuilderPopLevel].GetInteger() < 1) {
        AddMessage ("Incorrect population needed to build");
        return ERROR_FAILURE;
    }
    
    // MaxAgRatio
    if (pvSubmitArray[SystemGameClassData::MaxAgRatio].GetFloat() < MIN_MAX_AG_RATIO ||
        pvSubmitArray[SystemGameClassData::MaxAgRatio].GetFloat() > MAX_RATIO) {
        AddMessage ("Incorrect Maximum Agriculture Ratio");
        return ERROR_FAILURE;
    }
    
    // Resources
    if (pvSubmitArray[SystemGameClassData::MinAvgAg].GetInteger() < 1 || 
        pvSubmitArray[SystemGameClassData::MaxAvgAg].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::MinAvgAg].GetInteger() > 
        pvSubmitArray[SystemGameClassData::MaxAvgAg].GetInteger()) {
        
        AddMessage ("Incorrect average planet agriculture level");
        return ERROR_FAILURE;
    }
    
    if (pvSubmitArray[SystemGameClassData::MinAvgMin].GetInteger() < 1 ||
        pvSubmitArray[SystemGameClassData::MaxAvgMin].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::MinAvgMin].GetInteger() > 
        pvSubmitArray[SystemGameClassData::MaxAvgMin].GetInteger()) {
        
        AddMessage ("Incorrect average planet mineral level");
        return ERROR_FAILURE;
    }
    
    if (pvSubmitArray[SystemGameClassData::MinAvgFuel].GetInteger() < 1 ||
        pvSubmitArray[SystemGameClassData::MaxAvgFuel].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::MinAvgFuel].GetInteger() > 
        pvSubmitArray[SystemGameClassData::MaxAvgFuel].GetInteger()) {
        
        AddMessage ("Incorrect average planet fuel level");
        return ERROR_FAILURE;
    }
    
    
    if (pvSubmitArray[SystemGameClassData::MinAgHW].GetInteger() < 
        pvSubmitArray[SystemGameClassData::BuilderPopLevel].GetInteger() ||

        pvSubmitArray[SystemGameClassData::MaxAgHW].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::MinAgHW].GetInteger() > 
        pvSubmitArray[SystemGameClassData::MaxAgHW].GetInteger()) {
        
        AddMessage ("Incorrect homeworld agriculture level");
        return ERROR_FAILURE;
    }
    
    if (pvSubmitArray[SystemGameClassData::MinMinHW].GetInteger() < 
        pvSubmitArray[SystemGameClassData::BuilderPopLevel].GetInteger() ||

        pvSubmitArray[SystemGameClassData::MaxMinHW].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::MinMinHW].GetInteger() > 
        pvSubmitArray[SystemGameClassData::MaxMinHW].GetInteger()) {
        
        AddMessage ("Incorrect homeworld mineral level");
        return ERROR_FAILURE;
    }
    
    if (pvSubmitArray[SystemGameClassData::MinFuelHW].GetInteger() < 
        pvSubmitArray[SystemGameClassData::BuilderPopLevel].GetInteger() ||

        pvSubmitArray[SystemGameClassData::MaxFuelHW].GetInteger() > vMaxResourcesPerPlanet.GetInteger() ||
        
        pvSubmitArray[SystemGameClassData::MinFuelHW].GetInteger() > 
        pvSubmitArray[SystemGameClassData::MaxFuelHW].GetInteger()) {
        
        AddMessage ("Incorrect homeworld fuel level");
        return ERROR_FAILURE;
    }
    
    // Num secs per update
    if (pvSubmitArray[SystemGameClassData::NumSecPerUpdate].GetInteger() < iMinNumSecsPerUpdate || 
        pvSubmitArray[SystemGameClassData::NumSecPerUpdate].GetInteger() > iMaxNumSecsPerUpdate) {
        
        AddMessage ("Invalid update period");
        return ERROR_FAILURE;
    }
    
    // Diplomacy
    if (!g_pGameEngine->IsLegalDiplomacyLevel (iDip)) {
        AddMessage ("Illegal Diplomacy level");
        return ERROR_FAILURE;
    }

    // Subjective views and exposed maps
    iTemp = pvSubmitArray[SystemGameClassData::Options].GetInteger();
    if ((iTemp & SUBJECTIVE_VIEWS) && (iTemp & EXPOSED_MAP)) {
        pvSubmitArray[SystemGameClassData::Options] = iTemp & ~SUBJECTIVE_VIEWS;
        AddMessage ("Games with exposed maps cannot have subjective views. "\
            "This option has been turned off");
    }
    
    // Diplomacy for map shared
    switch (pvSubmitArray[SystemGameClassData::MapsShared].GetInteger()) {
        
    case NO_DIPLOMACY:
        break;
        
    case TRUCE:
    case TRADE:
    case ALLIANCE:
        
        if (!g_pGameEngine->GameAllowsDiplomacy (
            iDip, 
            pvSubmitArray[SystemGameClassData::MapsShared].GetInteger())
            ) {
            
            AddMessage ("The shared map diplomacy level must be selectable");
            return ERROR_FAILURE;
        }
        break;
        
    default:
        
        AddMessage ("Illegal shared map diplomacy level");
        return ERROR_FAILURE;
    }
    
    // Alliance limit options without limits
    if (pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() == UNRESTRICTED_DIPLOMACY) {
        
        int iOptions = pvSubmitArray[SystemGameClassData::Options].GetInteger();
        
        if (iOptions & PERMANENT_ALLIANCES) {
            AddMessage ("Permanent alliances have been unselected");
            iOptions &= ~PERMANENT_ALLIANCES;
            pvSubmitArray[SystemGameClassData::Options] = iOptions;
        }
    }
    
    // InitDevShips
    iNumInitTechDevs = 0;
    ENUMERATE_TECHS(i) {
        
        sprintf (pszTechString, "InitShip%i", i);
        
        if ((pHttpForm = m_pHttpRequest->GetForm (pszTechString)) != NULL) {
            iInitTechDevs |= TECH_BITS[i];
            iNumInitTechDevs ++;
        }
    }
    
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = iInitTechDevs;
    
    
    // DevShips
    iNumTechDevs = 0;
    ENUMERATE_TECHS(i) {
        
        sprintf (pszTechString, "DevShip%i", i);
        
        if ((pHttpForm = m_pHttpRequest->GetForm (pszTechString)) != NULL) {
            iDevTechDevs |= TECH_BITS[i];
            iNumTechDevs ++;
        }
    }
    
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = iDevTechDevs;
    
    // Check NumInitialTechDevs
    if (pvSubmitArray[SystemGameClassData::NumInitialTechDevs].GetInteger() > 
        iNumTechDevs - iNumInitTechDevs) {

        AddMessage ("Too many initial tech developments");
        return ERROR_FAILURE;
    }

    if (pvSubmitArray[SystemGameClassData::NumInitialTechDevs].GetInteger() < 0) {
        AddMessage ("Not enough initial tech developments");
        return ERROR_FAILURE;
    }

    // Make sure at least one ship type can be developed
    if (iDevTechDevs == 0) {
        AddMessage ("At least one ship must be developable");
        return ERROR_FAILURE;
    }
    
    // Verify that the initdevships can be developed
    ENUMERATE_TECHS(i) {
        
        if ((iInitTechDevs & TECH_BITS[i]) && !(iDevTechDevs & TECH_BITS[i])) {
            
            // An initial ship couldn't be developed
            AddMessage ("An initial ship could not be developed");
            return ERROR_FAILURE;
        }
    }
    
    // Verify that engineers can be built if disconnected maps are selected
    if ((pvSubmitArray[SystemGameClassData::Options].GetInteger() & DISCONNECTED_MAP) &&
        !(iDevTechDevs & TECH_BITS[ENGINEER])
        ) {
        AddMessage ("Engineer ships must be developable if maps are disconnected");
        return ERROR_FAILURE;
    }
    
    // Verify that if !mapexposed, sci's can be developed
    if (!(pvSubmitArray[SystemGameClassData::Options].GetInteger() & EXPOSED_MAP) && 
        !(iDevTechDevs & TECH_SCIENCE)
        ) {
        AddMessage ("Science ships must be developable if maps are not exposed");
        return ERROR_FAILURE;
    }
    
    // Make sure that minefields aren't selected without minesweepers
    if ((iDevTechDevs & TECH_MINEFIELD) && 
        !(iDevTechDevs & TECH_MINESWEEPER) &&
        pvSubmitArray[SystemGameClassData::MaxAgRatio].GetFloat() > MAX_MAX_AG_RATIO_WITHOUT_SWEEPERS
        ) {
        AddMessage ("Minefields cannot be developed if minesweepers cannot be developed and the ag ratio is less than ");
        AppendMessage (MAX_MAX_AG_RATIO_WITHOUT_SWEEPERS);
        return ERROR_FAILURE;
    }
    
    // Make sure that independence wasn't selected with maxnumempires = 2
    if (pvSubmitArray[SystemGameClassData::Options].GetInteger() & INDEPENDENCE &&
        pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger() == 2) {
        
        pvSubmitArray[SystemGameClassData::Options] = 
            pvSubmitArray[SystemGameClassData::Options].GetInteger() & ~INDEPENDENCE;
        
        AddMessage ("The Independence option was removed because it requires more than two empires in a game.");
    }
    
    return OK;
    
Cleanup:
    
    AddMessage (
        "An error occurred reading data from the system database. Please contact the administrator"
        );
    return iErrCode;
}


int HtmlRenderer::RenderUnsafeHyperText (const char* pszText, const char* pszUrl) {

    int iErrCode;
    String strHtml;

    if (pszUrl == NULL || pszUrl[0] == '\0') {
        return OK;
    }

    iErrCode = HTMLFilter (pszUrl, &strHtml, 0, false);
    if (iErrCode == OK && !strHtml.IsBlank()) {
        iErrCode = RenderHyperText (pszText, strHtml.GetCharPtr());
    }

    return iErrCode;
}

int HtmlRenderer::RenderHyperText (const char* pszText, const char* pszUrl) {

    if (pszUrl == NULL || pszUrl[0] == '\0') {
        return OK;
    }

    OutputText ("<a href=\"");
        
    if (strnicmp (pszUrl, "http://", sizeof ("http://") - 1) != 0) {
        OutputText ("http://");
    }

    m_pHttpResponse->WriteText (pszUrl);
    OutputText ("\">");
    m_pHttpResponse->WriteText (pszText);
    OutputText ("</a>");

    return OK;
}

void HtmlRenderer::WriteFaq() {
    
    OutputText ("<p>Printer-friendly documentation can be found <a href=\"" BASE_RESOURCE_DIR FAQ_FILE "\">here</a>.<p>");
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" FAQ_FILE, g_pszResourceDir);
    
    ICachedFile* pcfFaq = g_pFileCache->GetFile (pszFileName);
    if (pcfFaq == NULL) {
        OutputText ("<p><strong>The documentation file could not be found; please alert your system administrator</strong>");
    } else {
        OutputText ("<p></center>");
        m_pHttpResponse->WriteTextFile (pcfFaq);
        pcfFaq->Release();
        OutputText ("<center>");
    }
}

void HtmlRenderer::WriteServerNews() {
    
    UTCTime tTime;
    
    Time::GetTime (&tTime);
    
    int iDay  = Time::GetDay (tTime);
    int iYear = Time::GetYear (tTime);
    
    const char* pszMonth = Time::GetMonthName (Time::GetMonth (tTime));
    const char* pszDayW  = Time::GetDayOfWeek (tTime);
    
    OutputText ("<p><table><tr><th bgcolor=\"#");
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    OutputText ("\">Today is ");
    m_pHttpResponse->WriteText (pszDayW);
    OutputText (", ");
    m_pHttpResponse->WriteText (pszMonth);
    OutputText (" ");
    m_pHttpResponse->WriteText (iDay);
    OutputText (", ");
    m_pHttpResponse->WriteText (iYear);
    OutputText ("</th></tr></table><p>");
    
    WriteServerNewsFile();
}

void HtmlRenderer::WriteContributions() {

    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" CONTRIBUTIONS_FILE, g_pszResourceDir);

    ICachedFile* pcfFile = g_pFileCache->GetFile (pszFileName);
    if (pcfFile == NULL) {
        OutputText ("<p><strong>The contributions file could not be found; please alert your system administrator</strong>");
    } else {
        m_pHttpResponse->WriteTextFile (pcfFile);
        pcfFile->Release();
    }
}

void HtmlRenderer::WriteCredits() {

    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" CREDITS_FILE, g_pszResourceDir);

    ICachedFile* pcfFile = g_pFileCache->GetFile (pszFileName);
    if (pcfFile == NULL) {
        OutputText ("<p><strong>The credits file could not be found; please alert your system administrator</strong>");
    } else {
        m_pHttpResponse->WriteTextFile (pcfFile);
        pcfFile->Release();
    }
}


void HtmlRenderer::WriteIntro() {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" INTRO_FILE, g_pszResourceDir);
    
    ICachedFile* pcfFile = g_pFileCache->GetFile (pszFileName);
    if (pcfFile == NULL) {
        OutputText ("<p><strong>The intro file could not be found; please alert your system administrator</strong>");
    } else {
        m_pHttpResponse->WriteTextFile (pcfFile);
        pcfFile->Release();
    }
}

void HtmlRenderer::WriteIntroUpper() {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" INTRO_UPPER_FILE, g_pszResourceDir);
    
    ms_mIntroUpperFileLock.WaitReader();
    
    ICachedFile* pcfFile = g_pFileCache->GetFile (pszFileName);
    if (pcfFile != NULL) {
        m_pHttpResponse->WriteTextFile (pcfFile);
        pcfFile->Release();
    }
    
    ms_mIntroUpperFileLock.SignalReader();
}

void HtmlRenderer::WriteIntroLower() {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" INTRO_LOWER_FILE, g_pszResourceDir);
    
    ms_mIntroLowerFileLock.WaitReader();
    
    ICachedFile* pcfFile = g_pFileCache->GetFile (pszFileName);
    if (pcfFile != NULL) {
        m_pHttpResponse->WriteTextFile (pcfFile);
        pcfFile->Release();
    }
    
    ms_mIntroLowerFileLock.SignalReader();
}

void HtmlRenderer::WriteServerNewsFile() {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" NEWS_FILE, g_pszResourceDir);
    
    ms_mNewsFileLock.WaitReader();
    
    ICachedFile* pcfCachedFile = g_pFileCache->GetFile (pszFileName);
    
    if (pcfCachedFile != NULL) {
        m_pHttpResponse->WriteTextFile (pcfCachedFile);
        pcfCachedFile->Release();
    }
    
    ms_mNewsFileLock.SignalReader();
}


bool HtmlRenderer::UpdateIntroUpper (const char* pszText) {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" INTRO_UPPER_FILE, g_pszResourceDir);
    
    ms_mIntroUpperFileLock.WaitWriter();
    
    bool bRetVal = UpdateCachedFile (pszFileName, pszText);
    
    ms_mIntroUpperFileLock.SignalWriter();
    
    return bRetVal;
}

bool HtmlRenderer::UpdateIntroLower (const char* pszText) {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" INTRO_LOWER_FILE, g_pszResourceDir);
    
    ms_mIntroLowerFileLock.WaitWriter();
    
    bool bRetVal = UpdateCachedFile (pszFileName, pszText);
    
    ms_mIntroLowerFileLock.SignalWriter();
    
    return bRetVal;
}

bool HtmlRenderer::UpdateServerNews (const char* pszText) {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" NEWS_FILE, g_pszResourceDir);
    
    ms_mNewsFileLock.WaitWriter();
    
    bool bRetVal = UpdateCachedFile (pszFileName, pszText);
    
    ms_mNewsFileLock.SignalWriter();
    
    return bRetVal;
}

bool HtmlRenderer::UpdateCachedFile (const char* pszFileName, const char* pszText) {
    
    size_t stSize;
    
    ICachedFile* pcfCachedFile = g_pFileCache->GetFile (pszFileName);
    if (pcfCachedFile == NULL || (stSize = pcfCachedFile->GetSize()) == 0) {
        
        if (pszText == NULL || *pszText == '\0') {
            // No change
            return false;
        }
        
        // Null file: write away
        File fCachedFile;
        if (fCachedFile.OpenWrite (pszFileName) != OK) {
            Assert (false);
            return false;
        }
        
        fCachedFile.Write (pszText);
        fCachedFile.Close();
        
        return true;
        
    } else {
        
        // Are we changing?
        const char* pszData = (char*) pcfCachedFile->GetData();
        
        size_t stNewSize = String::StrLen (pszText);
        
        if (stSize != stNewSize || strncmp (pszText, pszData, stNewSize) != 0) {
            
            // File is changing, so get the filecache to release it
            g_pFileCache->ReleaseFile (pszFileName);
            pcfCachedFile->Release();
            
            // Now, replace the file
            File fCachedFile;
            if (fCachedFile.OpenWrite (pszFileName) != OK) {
                Assert (false);
                return false;
            }
            
            fCachedFile.Write (pszText == NULL ? "" : pszText);
            fCachedFile.Close();
            
            return true;
            
        } else {
            
            // Just release the file
            pcfCachedFile->Release();
            
            return false;
        }
    }
}

void HtmlRenderer::WriteNukeHistory (int iTargetEmpireKey) {
    
    Variant vNukeEmpireName;
    
    Variant** ppvNukedData, ** ppvNukerData;
    int iNumNuked, iNumNukers, i;   
    
    if (g_pGameEngine->GetEmpireName (iTargetEmpireKey, &vNukeEmpireName) != OK || 
        g_pGameEngine->GetNukeHistory (iTargetEmpireKey, &iNumNuked, &ppvNukedData, &iNumNukers, 
        &ppvNukerData) != OK) {
        
        OutputText ("<p><strong>The empire's nuke history could not be read</strong>");
        
    } else {
        
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
            
            int iErrCode, iAlloc = max (iNumNukers, iNumNuked);
            
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
                for (i = 0; i < iNumNuked; i ++) {
                    ptTime[i] = ppvNukedData[i][SystemEmpireNukeList::TimeStamp].GetUTCTime();
                    ppvData[i] = ppvNukedData[i];
                }
                
                Algorithm::QSortTwoAscending<UTCTime, Variant*> (ptTime, ppvData, iNumNuked);
                
                char pszEmpire [256 + MAX_EMPIRE_NAME_LENGTH];
                
                for (i = 0; i < iNumNuked; i ++) {
                    
                    OutputText ("<tr><td align=\"center\"><strong>");
                    
                    m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr()); 
                    OutputText ("</strong></td><td align=\"center\">");
                    
                    sprintf (
                        pszEmpire, 
                        "View the profile of %s",
                        ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr()
                        );
                    
                    WriteProfileAlienString (
                        ppvData[i][SystemEmpireNukeList::AlienKey].GetInteger(), 
                        ppvData[i][SystemEmpireNukeList::EmpireKey].GetInteger(),
                        ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr(),
                        0,
                        "ProfileLink",
                        pszEmpire,
                        true,
                        true
                        );
                    
                    OutputText ("</td><td align=\"center\">");
                    m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::GameClassName].GetCharPtr());
                    OutputText (" ");
                    m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::GameNumber].GetInteger());
                    OutputText ("</td><td align=\"center\">");
                    
                    iErrCode = Time::GetDateString (
                        ppvData[i][SystemEmpireNukeList::TimeStamp].GetUTCTime(), 
                        pszDateString
                        );
                    
                    if (iErrCode == OK) {
                        m_pHttpResponse->WriteText (pszDateString);
                    } else {
                        OutputText ("Could not read date string");
                    }
                    
                    OutputText ("</td></tr>");
                }
                
                g_pGameEngine->FreeData (ppvNukedData);
                
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
                    ptTime[i] = ppvNukerData[i][SystemEmpireNukeList::TimeStamp].GetUTCTime();
                    ppvData[i] = ppvNukerData[i];
                }
                Algorithm::QSortTwoAscending<UTCTime, Variant*> (ptTime, ppvData, iNumNukers);
                
                char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
                
                for (i = 0; i < iNumNukers; i ++) {
                    
                    OutputText ("<tr><td align=\"center\"><strong>");
                    m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr()); 
                    OutputText ("</strong></td><td align=\"center\">");
                    
                    sprintf (pszProfile, "View the profile of %s", ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr());
                    
                    WriteProfileAlienString (
                        ppvData[i][SystemEmpireNukeList::AlienKey].GetInteger(), 
                        ppvData[i][SystemEmpireNukeList::EmpireKey].GetInteger(),
                        ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr(),
                        0,
                        "ProfileLink",
                        pszProfile,
                        true,
                        true
                        );
                    
                    OutputText ("</td><td align=\"center\">");
                    m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::GameClassName].GetCharPtr());
                    OutputText (" ");
                    m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::GameNumber].GetInteger());
                    OutputText ("</td><td align=\"center\">");
                    
                    iErrCode = Time::GetDateString (
                        ppvData[i][SystemEmpireNukeList::TimeStamp].GetUTCTime(), 
                        pszDateString
                        );
                    
                    if (iErrCode == OK) {
                        m_pHttpResponse->WriteText (pszDateString);
                    } else {
                        OutputText ("Could not read date string");
                    }
                    
                    OutputText ("</td></tr>");
                }
                
                g_pGameEngine->FreeData (ppvNukerData);
                
                OutputText ("</table>");
            }
        }
    }
}

void HtmlRenderer::WritePersonalGameClasses (int iTargetEmpireKey) {
    
    int i, iErrCode, iNumGameClasses, * piGameClassKey;
    
    iErrCode = g_pGameEngine->GetEmpirePersonalGameClasses (iTargetEmpireKey, &piGameClassKey, NULL, &iNumGameClasses);
    if (iErrCode != OK || iNumGameClasses == 0) {
        OutputText ("<p><strong>There are no personal GameClasses available</strong>");
    } else {
        
        int iGameClass, iInitTechs, iDevTechs;
        Variant* pvGameClassInfo = NULL;

        OutputText ("<p>There ");

        if (iNumGameClasses == 1) {
            OutputText ("is 1 personal GameClass");
        } else {
            OutputText ("are <strong>");
            m_pHttpResponse->WriteText (iNumGameClasses);
            OutputText ("</strong> personal GameClasses");
        }
            
        OutputText (" available:<p><h3>Start a new game:</h3>");
        WriteSystemGameListHeader (m_vTableColor);

        for (i = 0; i < iNumGameClasses; i ++) {

            iGameClass = piGameClassKey[i];

            // Read game class data
            if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK &&
                g_pGameEngine->GetDevelopableTechs (iGameClass, &iInitTechs, &iDevTechs) == OK) {
                
                // Best effort
                iErrCode = WriteSystemGameListData (iGameClass, pvGameClassInfo);
            }
            
            if (pvGameClassInfo != NULL) {
                g_pGameEngine->FreeData (pvGameClassInfo);
                pvGameClassInfo = NULL;
            }
        }

        OutputText ("</table>");

        g_pGameEngine->FreeKeys (piGameClassKey);
    }
}


void HtmlRenderer::WritePersonalTournaments (int iTargetEmpireKey) {

    int iErrCode;
    unsigned int* piTournamentKey = NULL, iTournaments;

    // List all joined tournaments
    iErrCode = g_pGameEngine->GetOwnedTournaments (iTargetEmpireKey, &piTournamentKey, NULL, &iTournaments);
    if (iErrCode != OK || iTournaments == 0) {
        OutputText ("<p><strong>There are no personal Tournaments available</strong>");
    } else {

        OutputText ("<p>There ");

        if (iTournaments == 1) {
            OutputText ("is 1 personal Tournament");
        } else {
            OutputText ("are <strong>");
            m_pHttpResponse->WriteText (iTournaments);
            OutputText ("</strong> personal Tournaments");
        }
            
        OutputText (" available:");

        RenderTournaments (piTournamentKey, iTournaments, false);

        if (piTournamentKey != NULL) {
            g_pGameEngine->FreeData (piTournamentKey);   // Not a bug
        }
    }
}


void HtmlRenderer::WriteGameAdministratorGameData (const char* pszGameClassName, 
                                                   int iGameNumber, Seconds iSeconds, Seconds iSecondsUntil, 
                                                   int iNumUpdates, bool bOpen, bool bPaused, bool bAdminPaused, 
                                                   bool bStarted, const char* pszGamePassword, 
                                                   Variant* pvEmpireName, int iNumActiveEmpires, 
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
    
    if (bAdmin) {

        OutputText ("<td align=\"center\">");
        if (!String::IsBlank (pszGamePassword)) {
            
            String strFilter;
            int iErrCode = HTMLFilter (pszGamePassword, &strFilter, 0, false);
            if (iErrCode != OK) {
                OutputText ("The server is out of memory");
            } else {
                m_pHttpResponse->WriteText (strFilter, strFilter.GetLength());
            }
        }
        OutputText ("</td>");
    }
    
    OutputText ("<td align=\"center\"><strong>");
    m_pHttpResponse->WriteText (iNumActiveEmpires);
    OutputText ("</strong> (");
    
    Assert (iNumActiveEmpires > 0);
    
    for (i = 0; i < iNumActiveEmpires - 1; i ++) {
        m_pHttpResponse->WriteText (pvEmpireName[i].GetCharPtr());
        OutputText (", ");
    }
    m_pHttpResponse->WriteText (pvEmpireName[i].GetCharPtr());
    
    OutputText (")</td>");
}


int HtmlRenderer::RenderThemeInfo (int iBackgroundKey, int iLivePlanetKey, int iDeadPlanetKey, int iSeparatorKey,
                                   int iButtonKey, int iHorzKey, int iVertKey, int iColorKey) {
    
    const int piUIKey[] = {
        iBackgroundKey,
            iColorKey,
            iLivePlanetKey, 
            iDeadPlanetKey,
            iButtonKey,
            iSeparatorKey, 
            iHorzKey, 
            iVertKey,
    };
    
    const char* ppszName[] = {
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
    
    const char* ppszFormName[] = {
        "Background",
            "Color",
            "LivePlanet",
            "DeadPlanet",
            "Button",
            "Separator",
            "Horz",
            "Vert"
    };
    
    const int piThemeBitField[] = {
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
    
    int* piThemeKey, iNumThemes, i, j;
    int iErrCode = g_pGameEngine->GetThemeKeys (&piThemeKey, &iNumThemes);
    if (iErrCode != OK || iNumThemes == 0) {
        return iErrCode;
    }
    
    String strHorz, strVert;
    
    iErrCode = GetHorzString (NULL_THEME, &strHorz);
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    iErrCode = GetVertString (NULL_THEME, &strVert);
    if (iErrCode != OK) {
        return iErrCode;
    }
    
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
    WriteLivePlanetImageSrc (NULL_THEME);
    
    OutputText ("\"></td><td align=\"center\"><img src=\"");
    WriteDeadPlanetImageSrc (NULL_THEME);
    
    OutputText ("\"></td><td align=\"center\">");
    WriteButtonString (NULL_THEME, "Login", "Login", "Login");
    
    OutputText ("</td><td align=\"center\">");
    
    WriteSeparatorString (NULL_THEME);
    
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
    Variant* pvThemeData;
    
    char pszForm [128];
    
    for (i = 0; i < iNumThemes; i ++) {
        
        iErrCode = g_pGameEngine->GetThemeData (piThemeKey[i], &pvThemeData);
        if (iErrCode != OK) {
            return iErrCode;
        }
        
        OutputText ("<tr><td width=\"10%\" align=\"center\">");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::Name].GetCharPtr());
        OutputText ("<p>");
        
        snprintf (pszForm, sizeof (pszForm), "ThemeInfo%i", piThemeKey[i]);
        WriteButtonString (m_iButtonKey, "Info", "Theme Info", pszForm); 
        
        OutputText ("</td>");
        
        iOptions = pvThemeData[SystemThemes::Options].GetInteger();
        
        // Background
        if (iOptions & THEME_BACKGROUND) {
            
            OutputText ("<td align=\"center\"><img width=\"120\" height=\"120\" src=\"");
            WriteBackgroundImageSrc (piThemeKey[i]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        // Text colors
        OutputText ("<td align=\"center\"><table width=\"100\" height=\"100\" cellspacing=\"0\"");
        
        if (iOptions & THEME_BACKGROUND) {
            OutputText (" background=\"");
            WriteBackgroundImageSrc (piThemeKey[i]);
            OutputText ("\"");
        }
        OutputText (">"\
            
            // Text
            "<tr><td align=\"center\"><font color=\"#");
        
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::TextColor].GetCharPtr());
        OutputText ("\">Text</font></td></tr>"\
            
            // Good Color
            "<tr><td align=\"center\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::GoodColor].GetCharPtr());
        OutputText ("\">Good</font></td></tr>"\
            
            // Bad Color
            
            "<tr><td align=\"center\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::BadColor].GetCharPtr());
        OutputText ("\">Bad</font></td></tr>"\
            
            // Private Color
            
            "<tr><td align=\"center\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::PrivateMessageColor].GetCharPtr());
        OutputText ("\">Private</font></td></tr>"\
            
            // Broadcast Color
            
            "<tr><td align=\"center\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::BroadcastMessageColor].GetCharPtr());
        OutputText ("\">Broadcast</font></td></tr>"\
            
            // Table Color
            
            "<tr><th align=\"center\" bgcolor=\"");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::TableColor].GetCharPtr());
        OutputText ("\"><font color=\"#");
        m_pHttpResponse->WriteText (pvThemeData[SystemThemes::TextColor].GetCharPtr());
        OutputText ("\">Table</font></th></tr>"\
            
            "</table></td>"
            );
        
        // Live Planet
        if (iOptions & THEME_LIVE_PLANET) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteLivePlanetImageSrc (piThemeKey[i]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_DEAD_PLANET) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteDeadPlanetImageSrc (piThemeKey[i]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_BUTTONS) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteButtonImageSrc (piThemeKey[i], "Login");
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_SEPARATOR) {
            OutputText ("<td align=\"center\"><img width=\"200\" src=\"");
            WriteSeparatorSrc (piThemeKey[i]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_HORZ) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteHorzSrc (piThemeKey[i]);
            OutputText ("\"></td>");
        } else {
            OutputText ("<td>&nbsp;</td>");
        }
        
        if (iOptions & THEME_VERT) {
            OutputText ("<td align=\"center\"><img src=\"");
            WriteVertSrc (piThemeKey[i]);
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
                
                if (piUIKey[j] == piThemeKey[i]) {
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
        
        g_pGameEngine->FreeData (pvThemeData);
    }
    
    g_pGameEngine->FreeKeys (piThemeKey);
    
    return OK;
}

int HtmlRenderer::DisplayThemeData (int iThemeKey) {
    
    Variant* pvThemeData;
    int iErrCode = g_pGameEngine->GetThemeData (iThemeKey, &pvThemeData);
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    OutputText ("<p><table width=\"50%\"><tr><td>Theme:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::Name].GetCharPtr());
    
    OutputText ("</td></tr><tr><td>Version:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::Version].GetCharPtr());
    
    OutputText ("</tr><tr><td>Author:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::AuthorName].GetCharPtr());
    
    OutputText ("</td></tr><tr><td>Email:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::AuthorEmail].GetCharPtr());
    
    OutputText ("</td></tr><tr><td>Description:</td><td>");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::Description].GetCharPtr());
    
    OutputText ("</td></tr><tr><td>Download:</td><td><a href=\"");
    
    WriteThemeDownloadSrc (iThemeKey, pvThemeData[SystemThemes::FileName].GetCharPtr());
    
    OutputText ("\">");
    m_pHttpResponse->WriteText (pvThemeData[SystemThemes::FileName].GetCharPtr());
    OutputText ("</a></td></tr></table><p>");
    
    WriteButton (BID_CANCEL);
    
    g_pGameEngine->FreeData (pvThemeData);
    
    return iErrCode;
}


int HtmlRenderer::GetSensitiveMapText (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey,
                                       int iProxyPlanetKey, bool bVisibleBuilds, bool bIndependence,
                                       const Variant* pvPlanetData, String* pstrAltTag) {
    
    int iErrCode = OK, iNumOwners;
    
    int iTotalNumShips = 
        pvPlanetData[GameMap::NumUncloakedShips].GetInteger() + 
        pvPlanetData[GameMap::NumCloakedShips].GetInteger() + 
        pvPlanetData[GameMap::NumUncloakedBuildShips].GetInteger() + 
        pvPlanetData[GameMap::NumCloakedBuildShips].GetInteger();
    
    *pstrAltTag = "";
    
    if (iTotalNumShips == 0) {
        goto End;
    }
    
    int* piOwnerData;
    iErrCode = g_pGameEngine->GetPlanetShipOwnerData (
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iPlanetKey,
        iProxyPlanetKey,
        iTotalNumShips,
        bVisibleBuilds,
        bIndependence,
        &piOwnerData
        );
    
    if (iErrCode != OK) {
        Assert (false);
        goto End;
    }
    
    iNumOwners = piOwnerData[0];
    
    if (iNumOwners > 0) {
        
        int i, j, iBase, iNumOwnerTechs, iType, iNumShips;
        unsigned int iOwnerKey;
        Variant vEmpireName;
        
        iBase = 1;
        
        for (i = 0; i < iNumOwners; i ++) {
            
            if (i > 0) {
                *pstrAltTag += "\n";
            }
            
            iOwnerKey = piOwnerData [iBase];
            iNumOwnerTechs = piOwnerData [iBase + 1];
            
            Assert (iNumOwnerTechs > 0);
            
            if (iOwnerKey == m_iEmpireKey) {
                *pstrAltTag += m_vEmpireName.GetCharPtr();
            }
            
            else if (iOwnerKey == INDEPENDENT) {
                *pstrAltTag += "Independent";
            }
            
            else {
                
                iErrCode = g_pGameEngine->GetEmpireName (iOwnerKey, &vEmpireName);
                if (iErrCode != OK) {
                    Assert (false);
                    continue;
                }
                
                *pstrAltTag += vEmpireName.GetCharPtr();
            }
            
            *pstrAltTag += ": ";
            
            for (j = 0; j < iNumOwnerTechs; j ++) {
                
                iType = piOwnerData [iBase + 2 + j * 2];
                iNumShips = piOwnerData [iBase + 3 + j * 2];
                
                Assert (iType >= FIRST_SHIP && iType <= LAST_SHIP);
                Assert (iNumShips > 0);
                
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
            
            iBase += 2 + 2 * iNumOwnerTechs;
        }
    }
    
    delete [] piOwnerData;
    
End:
    
    return iErrCode;
}



//
// Events and statistics
//

AlmonasterStatistics HtmlRenderer::m_sStats;

int HtmlRenderer::OnCreateEmpire (int iEmpireKey) {
    
    // Stats
    Algorithm::AtomicIncrement (&m_sStats.EmpiresCreated);
    
    return OK;
}

int HtmlRenderer::OnDeleteEmpire (int iEmpireKey) {
    
    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
        g_pszResourceDir,
        iEmpireKey
        );
    
    // Just in case...
    g_pFileCache->ReleaseFile (pszDestFileName);
    
    // Attempt to delete an uploaded file
    File::DeleteFile (pszDestFileName);
    
    // Stats
    Algorithm::AtomicIncrement (&m_sStats.EmpiresDeleted);
    
    return OK;
}

int HtmlRenderer::OnLoginEmpire (int iEmpireKey) {
    
    Algorithm::AtomicIncrement (&m_sStats.Logins);
    
    return OK;
}

int HtmlRenderer::OnCreateGame (int iGameClass, int iGameNumber) {
    
    Algorithm::AtomicIncrement (&m_sStats.GamesStarted);
    
    return OK;
}

int HtmlRenderer::OnCleanupGame (int iGameClass, int iGameNumber) {
    
    Algorithm::AtomicIncrement (&m_sStats.GamesEnded);
    
    return OK;
}

int HtmlRenderer::OnDeleteTournament (unsigned int iTournamentKey) {

    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_UPLOADED_TOURNAMENT_ICON_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
        g_pszResourceDir,
        iTournamentKey
        );
    
    // Just in case...
    g_pFileCache->ReleaseFile (pszDestFileName);
    
    // Attempt to delete an uploaded file
    File::DeleteFile (pszDestFileName);

    return OK;
}

int HtmlRenderer::OnDeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) {

    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_UPLOADED_TOURNAMENT_TEAM_ICON_DIR "/" ALIEN_NAME "%i.%i" DEFAULT_IMAGE_EXTENSION, 
        g_pszResourceDir,
        iTournamentKey,
        iTeamKey
        );
    
    // Just in case...
    g_pFileCache->ReleaseFile (pszDestFileName);
    
    // Attempt to delete an uploaded file
    File::DeleteFile (pszDestFileName);

    return OK;
}

int HtmlRenderer::OnPageRender (MilliSeconds msTime) {
    
    Algorithm::AtomicIncrement (&m_sStats.NumPageScriptRenders);
    Algorithm::AtomicIncrement (&m_sStats.TotalScriptTime, msTime);
    
    return OK;
}

void HtmlRenderer::ZeroStatistics() {
    
    memset (&m_sStats, 0, sizeof (AlmonasterStatistics));
}

void HtmlRenderer::InitGameOptions (GameOptions* pgoOptions) {

    memset (pgoOptions, 0, sizeof (GameOptions));
    pgoOptions->iTournamentKey = NO_KEY;
}

void HtmlRenderer::ClearGameOptions (GameOptions* pgoOptions) {

    if (pgoOptions->pSecurity != NULL) {
        delete [] pgoOptions->pSecurity;
    }
}

static const char* const g_pszEmpireInfoHeadersAdmin[] = {
    "Name",
    "Alien",
    "Econ",
    "Mil",
    "Tech Level",
    "Planets",
    "Ships",
    "War",
    "Truce",
    "Trade",
    "Alliance",
    "Pause",
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

void HtmlRenderer::RenderEmpireInformation (int iGameClass, int iGameNumber, bool bAdmin) {

    int i, iErrCode, iNumEmpires, iValue, iFoeKey, iWar, iTruce, iTrade, iAlliance, iUnmet, iNumTopHeaders;
    unsigned int iKey, iNumRows;
    float fValue;
    bool bUpdated;

    Variant* pvEmpireKey = NULL, vValue, vTemp;
    UTCTime tCurrentTime, tValue;

    String strWar, strTruce, strTrade, strAlliance;

    IDatabase* pDatabase = g_pGameEngine->GetDatabase();

    const char* pszTableColor = m_vTableColor.GetCharPtr(), * const * pszHeaders;

    char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
    char strGameEmpireData [256], pszGameEmpireShips [256], pszGameEmpireDip [256];

    Time::GetTime (&tCurrentTime);

    iErrCode = g_pGameEngine->GetEmpiresInGame (iGameClass, iGameNumber, &pvEmpireKey, &iNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    OutputText (
        "<p><table width=\"90%\">"\
        "<tr><td colspan=\"14\" align=\"center\">Active empires:</td></tr>"\
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

        GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

        OutputText ("<tr>");

        // Options
        iErrCode = pDatabase->ReadData (strGameEmpireData, GameEmpireData::Options, &vValue);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        iOptions = vValue.GetInteger();

        // Name
        iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[i].GetInteger(), &vValue);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        OutputText ("<td align=\"center\">");

        if (iOptions & RESIGNED) {
            OutputText ("<strike>");
        }

        m_pHttpResponse->WriteText (vValue.GetCharPtr());

        if (iOptions & RESIGNED) {
            OutputText ("</strike>");
        }

        OutputText ("</td>");

        // Alien
        iErrCode = g_pGameEngine->GetEmpireProperty (pvEmpireKey[i].GetInteger(), SystemEmpireData::AlienKey, &vTemp);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        iValue = vTemp.GetInteger();

        OutputText ("<td align=\"center\">");

        sprintf (pszProfile, "View the profile of %s", vValue.GetCharPtr());

        WriteProfileAlienString (
            iValue,
            pvEmpireKey[i].GetInteger(),
            vValue.GetCharPtr(),
            0,
            "ProfileLink",
            pszProfile,
            false,
            true
            );

        OutputText ("</td>");

        // Econ
        iErrCode = pDatabase->ReadData (strGameEmpireData, GameEmpireData::Econ, &vValue);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        iValue = vValue.GetInteger();

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (iValue);
        OutputText ("</td>");

        // Mil
        iErrCode = pDatabase->ReadData (strGameEmpireData, GameEmpireData::Mil, &vValue);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        fValue = vValue.GetFloat();

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (g_pGameEngine->GetMilitaryValue (fValue));
        OutputText ("</td>");

        if (bAdmin) {

            // TechLevel
            iErrCode = pDatabase->ReadData (strGameEmpireData, GameEmpireData::TechLevel, &vValue);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            fValue = vValue.GetFloat();

            OutputText ("<td align=\"center\">");
            m_pHttpResponse->WriteText (fValue);
            OutputText ("</td>");
        }

        // Planets
        iErrCode = pDatabase->ReadData (strGameEmpireData, GameEmpireData::NumPlanets, &vValue);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        iValue = vValue.GetInteger();

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (iValue);
        OutputText ("</td>");

        if (bAdmin) {

            // Ships
            GET_GAME_EMPIRE_SHIPS (pszGameEmpireShips, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
            GET_GAME_EMPIRE_DIPLOMACY (pszGameEmpireDip, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

            iErrCode = pDatabase->GetNumRows (pszGameEmpireShips, (unsigned int*) &iValue);
            if (iErrCode != OK) {
                goto Cleanup;
            }

            OutputText ("<td align=\"center\">");
            m_pHttpResponse->WriteText (iValue);
            OutputText ("</td>");

            iKey = NO_KEY;
            iWar = iTruce = iTrade = iAlliance = iUnmet = 0;
            
            strWar.Clear();
            strTruce.Clear();
            strTrade.Clear();
            strAlliance.Clear();

            while (true) {

                String* pStr = &strWar;
                Variant vName;

                iErrCode = pDatabase->GetNextKey (pszGameEmpireDip, iKey, &iKey);
                if (iErrCode != OK) {
                    if (iErrCode == ERROR_DATA_NOT_FOUND) {
                        break;
                    }
                    Assert (false);
                    goto Cleanup;
                }

                iErrCode = pDatabase->ReadData (pszGameEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vValue);
                if (iErrCode != OK) {
                    goto Cleanup;
                }
                iValue = vValue.GetInteger();

                iErrCode = pDatabase->ReadData (pszGameEmpireDip, iKey, GameEmpireDiplomacy::EmpireKey, &vValue);
                if (iErrCode != OK) {
                    goto Cleanup;
                }
                iFoeKey = vValue.GetInteger();

                iErrCode = g_pGameEngine->GetEmpireName (iFoeKey, &vName);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

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
                    Assert (false);
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

        // Paused
        iErrCode = pDatabase->ReadData (strGameEmpireData, GameEmpireData::Options, &vValue);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        iValue = vValue.GetInteger();

        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText ((iValue & REQUEST_PAUSE) != 0 ? "Yes" : "No");
        OutputText ("</td>");

        bUpdated = iValue & UPDATED;

        // LastLogin, idle
        iErrCode = pDatabase->ReadData (strGameEmpireData, GameEmpireData::LastLogin, &vValue);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        tValue = vValue.GetUTCTime();

        OutputText ("<td align=\"center\">");
        WriteTime (Time::GetSecondDifference (tCurrentTime, tValue));
        OutputText (" ago");

        iErrCode = pDatabase->ReadData (strGameEmpireData, GameEmpireData::NumUpdatesIdle, &vValue);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        iValue = vValue.GetInteger();

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

    GET_GAME_DEAD_EMPIRES (strGameEmpireData, iGameClass, iGameNumber);

    if (pDatabase->GetNumRows (strGameEmpireData, &iNumRows) == OK && iNumRows > 0) {

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
            int iIcon, iDeadEmpireKey;
            
            iErrCode = pDatabase->GetNextKey (strGameEmpireData, iKey, &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }
            
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            iErrCode = pDatabase->ReadData (strGameEmpireData, iKey, GameDeadEmpires::Icon, &vValue);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            iIcon = vValue.GetInteger();
            
            iErrCode = pDatabase->ReadData (strGameEmpireData, iKey, GameDeadEmpires::EmpireKey, &vValue);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            iDeadEmpireKey = vValue.GetInteger();

            iErrCode = pDatabase->ReadData (strGameEmpireData, iKey, GameDeadEmpires::Name, &vValue);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            pszName = vValue.GetCharPtr();

            OutputText ("<tr><td align=\"center\">");
            m_pHttpResponse->WriteText (pszName);
            OutputText ("</td><td align=\"center\">");
            
            sprintf (pszProfile, "View the profile of %s", pszName);

            WriteProfileAlienString (
                iIcon,
                iDeadEmpireKey,
                pszName,
                0,
                "ProfileLink",
                pszProfile,
                true,
                iDeadEmpireKey != NO_KEY
                );

            OutputText ("</td><td colspan=\"2\" align=\"center\">");

            iErrCode = pDatabase->ReadData (strGameEmpireData, iKey, GameDeadEmpires::Update, &vValue);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            m_pHttpResponse->WriteText (vValue.GetInteger());

            OutputText ("</td><td colspan=\"2\" align=\"center\">");

            iErrCode = pDatabase->ReadData (strGameEmpireData, iKey, GameDeadEmpires::Reason, &vValue);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            m_pHttpResponse->WriteText (REMOVAL_REASON_STRING [vValue.GetInteger()]);

            OutputText ("</td><td colspan=\"");
            m_pHttpResponse->WriteText (iNumTopHeaders - iNumHeaders);
            OutputText ("\">&nbsp;</td></tr><tr>");
        }
    }

    OutputText ("</table>");

Cleanup:

    if (pvEmpireKey != NULL) {
        g_pGameEngine->FreeData (pvEmpireKey);
    }

    SafeRelease (pDatabase);
}

void HtmlRenderer::WriteCreateTournament (int iEmpireKey) {

    IHttpForm* pHttpForm;

    String strName, strDesc, strUrl;

    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentName")) != NULL) {
        if (HTMLFilter (pHttpForm->GetValue(), &strName, 0, false) != OK) {
            strName = "";
        }
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TournamentDescription")) != NULL) {
        if (HTMLFilter (pHttpForm->GetValue(), &strDesc, 0, false) != OK) {
            strDesc = "";
        }
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("WebPageURL")) != NULL) {
        if (HTMLFilter (pHttpForm->GetValue(), &strUrl, 0, false) != OK) {
            strUrl = "";
        }
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
        "<textarea rows=\"4\" cols=\"40\" wrap=\"physical\" name=\"TournamentDescription\">"
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

int HtmlRenderer::ProcessCreateTournament (int iEmpireKey) {

    int iErrCode;
    unsigned int iTournamentKey;

    Variant pvSubmitArray [SystemTournaments::NumColumns];
    
    // Parse the forms
    iErrCode = ParseCreateTournamentForms (pvSubmitArray, iEmpireKey);
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    // Create the tournament, finally
    iErrCode = g_pGameEngine->CreateTournament (pvSubmitArray, &iTournamentKey);
    switch (iErrCode) {

    case OK:
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
        AddMessage ("The tournament could not be created; the error was ");
        AppendMessage (iErrCode);
        break;
    }
    
    return iErrCode;
}

int HtmlRenderer::ParseCreateTournamentForms (Variant* pvSubmitArray, int iEmpireKey) {

    IHttpForm* pHttpForm;

    int iErrCode;

    // Name
    pHttpForm = m_pHttpRequest->GetForm ("TournamentName");
    if (pHttpForm == NULL) {
        AddMessage ("Missing TournamentName form");
        return ERROR_FAILURE;
    }

    if (VerifyCategoryName ("Tournament", pHttpForm->GetValue(), MAX_TOURNAMENT_NAME_LENGTH, true) != OK) {
        return ERROR_FAILURE;
    }

    pvSubmitArray [SystemTournaments::Name] = pHttpForm->GetValue();

    // Description
    pHttpForm = m_pHttpRequest->GetForm ("TournamentDescription");
    if (pHttpForm == NULL) {
        AddMessage ("Missing TournamentDescription form");
        return ERROR_FAILURE;
    }

    if (String::StrLen (pHttpForm->GetValue()) > MAX_TOURNAMENT_DESCRIPTION_LENGTH) {
        AddMessage ("The description is too long");
        return ERROR_FAILURE;
    }

    pvSubmitArray [SystemTournaments::Description] = pHttpForm->GetValue();

    // URL
    pHttpForm = m_pHttpRequest->GetForm ("WebPageURL");
    if (pHttpForm == NULL) {
        AddMessage ("Missing WebPageURL form");
        return ERROR_FAILURE;
    }

    if (String::StrLen (pHttpForm->GetValue()) > MAX_WEB_PAGE_LENGTH) {
        AddMessage ("The Webpage is too long");
        return ERROR_FAILURE;
    }

    // Web page
    pvSubmitArray [SystemTournaments::WebPage] = pHttpForm->GetValue();

    // Owner
    pvSubmitArray [SystemTournaments::Owner] = iEmpireKey;

    // OwnerName
    if (iEmpireKey == SYSTEM) {
        pvSubmitArray [SystemTournaments::OwnerName] = (const char*) NULL;
    } else {
        
        iErrCode = g_pGameEngine->GetEmpireName (iEmpireKey, pvSubmitArray + SystemTournaments::OwnerName);
        if (iErrCode != OK) {
            AddMessage ("Error reading empire name");
            return iErrCode;
        }
    }

    // Icon
    iErrCode = g_pGameEngine->GetSystemProperty (SystemData::DefaultAlien, pvSubmitArray + SystemTournaments::Icon);
    if (iErrCode != OK) {
        AddMessage ("Error reading default alien");
        return iErrCode;
    }

    // News
    pvSubmitArray [SystemTournaments::News] = (const char*) NULL;

    return OK;
}

void HtmlRenderer::WriteTournamentAdministrator (int iEmpireKey) {

    int iErrCode;

    Variant* pvTournamentName = NULL;
    Algorithm::AutoDelete<Variant> autodel (pvTournamentName, true);

    unsigned int i, iNumTournaments, * piTournamentKey = NULL;
    iErrCode = g_pGameEngine->GetOwnedTournaments (iEmpireKey, &piTournamentKey, &pvTournamentName, &iNumTournaments);
    if (iErrCode != OK) {
        OutputText ("<p>Error reading tournament data");
        return;
    }

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
    } else {
        OutputText (":");
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

    if (iNumTournaments > 0) {

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

        g_pGameEngine->FreeKeys (piTournamentKey);
        g_pGameEngine->FreeData (pvTournamentName);
    }

    OutputText ("</table>");
}



void HtmlRenderer::WriteIconSelection (int iIconSelect, int iIcon, const char* pszCategory) {

    int iNumAliens, i, iErrCode;
    Variant** ppvAlienData = NULL;

    OutputText ("<p><h3>Choose an icon for your ");
    m_pHttpResponse->WriteText (pszCategory);
    
    OutputText (":</h3>");

    switch (iIconSelect) {
    case 0:

        iErrCode = g_pGameEngine->GetAlienKeys (&ppvAlienData, &iNumAliens);
        if (iErrCode == OK) {

            OutputText (
                "<input type=\"hidden\" name=\"WhichAlien\" value=\"0\">"\
                "<p><table width=\"75%\"><tr><td>"
                );

            for (i = 0; i < iNumAliens; i ++) {

                WriteAlienButtonString (
                    ppvAlienData[i][0].GetInteger(),
                    iIcon == ppvAlienData[i][0],
                    "Alien",
                    ppvAlienData[i][1].GetCharPtr()
                    );

                OutputText (" ");
            }

            OutputText ("</td></tr></table><p>");
        }

        WriteButton (BID_CANCEL);

        if (ppvAlienData != NULL) {
            g_pGameEngine->FreeData (ppvAlienData);
        }

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
}

int HtmlRenderer::HandleIconSelection (unsigned int* piIcon, const char* pszUploadDir, unsigned int iKey1, 
                                       unsigned int iKey2) {

    IHttpForm* pHttpForm;

    if ((pHttpForm = m_pHttpRequest->GetForm ("WhichAlien")) == NULL) {
        return ERROR_FAILURE;
    }

    if (pHttpForm->GetUIntValue() == 1) {
        
        if (!WasButtonPressed (BID_CHOOSE)) {
            return ERROR_FAILURE;
        }
            
        // Icon uploads
        if ((pHttpForm = m_pHttpRequest->GetForm ("IconFile")) == NULL) {
            return ERROR_FAILURE;
        }
        
        const char* pszFileName = pHttpForm->GetValue();
        if (pszFileName == NULL) {
            AddMessage ("You didn't upload a file");
            return ERROR_FAILURE;
        }
            
        if (VerifyGIF (pszFileName)) {
            
            // The gif was OK, so get a unique key and copy it to its destination
            if (CopyUploadedIcon (pszFileName, pszUploadDir, iKey1, iKey2) != OK) {
            
                AddMessage ("The file was uploaded, but could not be copied. Contact the administrator");
                return ERROR_FAILURE;
            }
                
            *piIcon = UPLOADED_ICON;
            AddMessage ("Your new icon was uploaded successfully");

            return OK;
        }

        return ERROR_FAILURE;
    }

    const char* pszStart;

    if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Alien")) != NULL && 
        (pszStart = pHttpForm->GetName()) != NULL &&
        sscanf (pszStart, "Alien%d", piIcon) == 1) {

        return OK;
    }

    return ERROR_FAILURE;
}


void HtmlRenderer::WriteActiveGameAdministration (int* piGameClass,
                                                  int* piGameNumber,
                                                  unsigned int iNumActiveGames, 
                                                  unsigned int iNumOpenGames,
                                                  unsigned int iNumClosedGames,
                                                  bool bAdmin) {

    int iErrCode;
    unsigned int i;

    if (iNumActiveGames == 0) {
        OutputText ("<p>There are no active games");
        if (bAdmin) {
            OutputText (" on the server");
        }
        return;
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

    Variant* pvEmpireKey = NULL, vGamePassword, vName;
    int iNumActiveEmpires, j, iNumUpdates;
    bool bPaused, bOpen, bStarted;

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    // Sort games by gameclass
    Algorithm::QSortTwoAscending<int, int> (piGameClass, piGameNumber, iNumActiveGames);

    // Sort games by gamenumber
    int iBegin = 0, iNumToSort;
    int iCurrentGameClass = piGameClass[0];

    for (i = 1; i < iNumActiveGames; i ++) {

        if (piGameClass[i] != iCurrentGameClass) {

            iNumToSort = i - iBegin;
            if (iNumToSort > 1) {
                Algorithm::QSortTwoAscending<int, int> (piGameNumber + iBegin, piGameClass + iBegin, iNumToSort);
            }

            iBegin = i;
            iCurrentGameClass = piGameClass[i];
        }
    }

    iNumToSort = i - iBegin;
    if (iNumToSort > 1) {
        Algorithm::QSortTwoAscending<int, int> (piGameNumber + iBegin, piGameClass + iBegin, iNumToSort);
    }
    iCurrentGameClass = piGameClass[0];

    bool bExists, bAdminPaused;
    Seconds iSeconds, iSecondsSince, iSecondsUntil;
    int iGameState;
    UTCTime tCreationTime;

    Variant* pvEmpireName;
    char pszAdmin [192];

    for (i = 0; i < iNumActiveGames; i ++) {

        // Check game for updates
        if (g_pGameEngine->CheckGameForUpdates (
            piGameClass[i],
            piGameNumber[i],
            &bExists
            ) != OK ||

            g_pGameEngine->DoesGameExist (piGameClass[i], piGameNumber[i], &bExists) != OK || !bExists ||
            g_pGameEngine->GetGameClassName (piGameClass[i], pszGameClassName) != OK ||
            g_pGameEngine->GetGameClassUpdatePeriod (piGameClass[i], &iSeconds) != OK ||
            g_pGameEngine->IsGamePaused (piGameClass[i], piGameNumber[i], &bPaused) != OK ||
            g_pGameEngine->IsGameAdminPaused (piGameClass[i], piGameNumber[i], &bAdminPaused) != OK ||
            g_pGameEngine->IsGameOpen (piGameClass[i], piGameNumber[i], &bOpen) != OK ||
            g_pGameEngine->HasGameStarted (piGameClass[i], piGameNumber[i], &bStarted) != OK ||
            g_pGameEngine->GetGamePassword (piGameClass[i], piGameNumber[i], &vGamePassword) != OK ||
            g_pGameEngine->GetGameCreationTime (piGameClass[i], piGameNumber[i], &tCreationTime) != OK ||
            g_pGameEngine->GetEmpiresInGame (piGameClass[i], piGameNumber[i], &pvEmpireKey, 
                &iNumActiveEmpires) != OK ||
            g_pGameEngine->GetGameUpdateData (piGameClass[i], piGameNumber[i], &iSecondsSince, 
                &iSecondsUntil, &iNumUpdates, &iGameState) != OK
            ) {
            continue;
        }

        if (i > 0 && piGameClass[i] != iCurrentGameClass) {
            iCurrentGameClass = piGameClass[i];
            OutputText ("<tr><td align=\"center\" colspan=\"9\">");
            WriteSeparatorString (m_iSeparatorKey);
            OutputText ("</td></tr>");
        }

        pvEmpireName = new Variant [iNumActiveEmpires];
        if (pvEmpireName != NULL) {

            for (j = 0; j < iNumActiveEmpires; j ++) {

                iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[j], pvEmpireName + j);
                if (iErrCode != OK) {
                    pvEmpireName[j] = "";
                }
            }

            WriteGameAdministratorGameData (pszGameClassName, piGameNumber[i], iSeconds, iSecondsUntil, 
                iNumUpdates, bOpen, bPaused, bAdminPaused, bStarted, 
                vGamePassword.GetCharPtr(), pvEmpireName, iNumActiveEmpires, tCreationTime, bAdmin);

            delete [] pvEmpireName;
        }

        OutputText ("<td align=\"center\">"); 

        sprintf (pszAdmin, "AdministerGame%i.%i", piGameClass[i], piGameNumber[i]);

        WriteButtonString (
            m_iButtonKey,
            "AdministerGame",
            "Administer Game", 
            pszAdmin
            );

        g_pGameEngine->FreeData (pvEmpireKey);

        OutputText ("</td></tr>");
    }

    OutputText ("</table>");
}


void HtmlRenderer::WriteAdministerGame (int iGameClass, int iGameNumber, bool bAdmin) {

    bool bStarted, bExists, bPaused, bOpen, bAdminPaused;
    Variant vGamePassword, * pvEmpireKey = NULL;
    int i, iErrCode, iNumUpdates, iNumActiveEmpires, iGameState;
    Seconds iSeconds, iSecondsUntil, iSecondsSince;
    UTCTime tCreationTime;

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    if (g_pGameEngine->CheckGameForUpdates (
        iGameClass,
        iGameNumber,
        &bExists
        ) != OK ||

        g_pGameEngine->DoesGameExist (iGameClass, iGameNumber, &bExists) != OK || !bExists ||
        g_pGameEngine->GetGameClassName (iGameClass, pszGameClassName) != OK ||
        g_pGameEngine->GetGameClassUpdatePeriod (iGameClass, &iSeconds) != OK ||
        g_pGameEngine->IsGamePaused (iGameClass, iGameNumber, &bPaused) != OK ||
        g_pGameEngine->IsGameAdminPaused (iGameClass, iGameNumber, &bAdminPaused) != OK ||
        g_pGameEngine->IsGameOpen (iGameClass, iGameNumber, &bOpen) != OK ||
        g_pGameEngine->HasGameStarted (iGameClass, iGameNumber, &bStarted) != OK ||
        g_pGameEngine->GetGamePassword (iGameClass, iGameNumber, &vGamePassword) != OK ||
        g_pGameEngine->GetEmpiresInGame (iGameClass, iGameNumber, &pvEmpireKey, &iNumActiveEmpires) != OK ||
        g_pGameEngine->GetGameCreationTime (iGameClass, iGameNumber, &tCreationTime) != OK ||
        g_pGameEngine->GetGameUpdateData (iGameClass, iGameNumber, &iSecondsSince, &iSecondsUntil, 
            &iNumUpdates, &iGameState) != OK
        ) {

        OutputText ("<p>The game could not be administered. It may no longer exist<p>");
        WriteButton (BID_CANCEL);
        return;
    }

    Variant* pvEmpireName = new Variant [iNumActiveEmpires];
    if (pvEmpireName == NULL) {
        OutputText ("<p>The server is out of memory");
        return;
    }

    for (i = 0; i < iNumActiveEmpires; i ++) {

        iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[i], pvEmpireName + i);
        if (iErrCode != OK) {
            pvEmpireName[i] = "";
        }
    }

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
    if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

        OutputText ("<p>");
        WriteGameAdministratorListHeader (m_vTableColor.GetCharPtr());

        // Best effort
        iErrCode = WriteGameAdministratorListData (iGameClass, iGameNumber, pvGameClassInfo);

        OutputText ("</table>");
    }

    if (pvGameClassInfo != NULL) {
        g_pGameEngine->FreeData (pvGameClassInfo);
        pvGameClassInfo = NULL;
    }

    OutputText ("<p>");
    WriteSeparatorString (m_iSeparatorKey);
    OutputText ("<p><table width=\"90%\">");

    // View Map
    if (bStarted) {
        OutputText ("<tr><td>View the game's map:</td><td>");
        WriteButton (BID_VIEWMAP);
        OutputText ("</td></tr>");
    }

    // Change game password
    if (bAdmin) {

        const char* pszPassword = vGamePassword.GetCharPtr();
        if (pszPassword != NULL && *pszPassword != '\0') {

            OutputText (
                "<tr><td>Change the game's password:</td>"\
                "<td><input type=\"text\" name=\"NewPassword\" size=\""
                );
            m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
            OutputText ("\" maxlength=\"");
            m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (pszPassword);
            OutputText ("\"> ");
            WriteButton (BID_CHANGEPASSWORD);
            OutputText ("</td></tr>");

        } else {

            OutputText (
                "<tr><td>Password protect the game:</td>"\
                "<td><input type=\"text\" name=\"NewPassword\" size=\""
                );
            m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
            OutputText ("\" maxlength=\"");
            m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
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

    int iNumResigned, * piResignedKey;
    iErrCode = g_pGameEngine->GetResignedEmpiresInGame (iGameClass, iGameNumber, &piResignedKey, &iNumResigned);

    if (iErrCode == OK && iNumResigned > 0) {

        Variant vName;

        OutputText ("<tr><td>Restore a resigned empire to the game:</td><td><select name=\"RestoreEmpireKey\">");

        for (i = 0; i < iNumResigned; i ++) {

            iErrCode = g_pGameEngine->GetEmpireName (piResignedKey[i], &vName);
            if (iErrCode == OK) {

                OutputText ("<option value=\"");
                m_pHttpResponse->WriteText (piResignedKey[i]);
                OutputText ("\">");
                m_pHttpResponse->WriteText (vName.GetCharPtr());
                OutputText ("</option>");
            }
        }

        delete [] piResignedKey;

        OutputText (" ");
        WriteButton (BID_RESTOREEMPIRE);
        OutputText ("</td></tr>");
    }

    OutputText ("<tr><td>Delete an empire from the game:</td><td><select name=\"DeleteEmpireKey\">");

    for (i = 0; i < iNumActiveEmpires; i ++) {
        OutputText ("<option value=\"");
        m_pHttpResponse->WriteText (pvEmpireKey[i].GetInteger());
        OutputText ("\">");
        m_pHttpResponse->WriteText (pvEmpireName[i].GetCharPtr());
        OutputText ("</option>");
    }
    OutputText (" ");
    WriteButton (BID_DELETEEMPIRE);
    OutputText ("</td></tr>");

    // Broadcast message to all empires
    OutputText (
        "<tr><td>Broadcast a message to all empires in the game:</td>"\
        "<td><textarea name=\"Message\" rows=\"5\" cols=\"45\" wrap=\"physical\"></textarea></td><td>"
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

    g_pGameEngine->FreeData (pvEmpireKey);

    delete [] pvEmpireName;
}


void HtmlRenderer::RenderEmpire (unsigned int iTournamentKey, int iEmpireKey) {

    int iAlienKey, iErrCode;
    char pszName [MAX_EMPIRE_NAME_LENGTH + 1];
    char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];

    Variant* pvTournamentEmpireData = NULL, vTemp;

    iErrCode = g_pGameEngine->GetEmpireName (iEmpireKey, pszName);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetEmpireProperty (iEmpireKey, SystemEmpireData::AlienKey, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    iAlienKey = vTemp.GetInteger();

    iErrCode = g_pGameEngine->GetTournamentEmpireData (iTournamentKey, iEmpireKey, &pvTournamentEmpireData);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    OutputText (
        
        "<tr>"\
        
    // Icon
        "<td align=\"center\">"
        );

    sprintf (pszProfile, "View the profile of %s", pszName);

    WriteProfileAlienString (
        iAlienKey,
        iEmpireKey,
        pszName,
        0, 
        "ProfileLink",
        pszProfile,
        false,
        true
        );
                
    NotifyProfileLink();
    
    OutputText (
        "</td>"\
        
    // Name
        "<td align=\"center\">"
        );

    m_pHttpResponse->WriteText (pszName);

    OutputText (
        "</td>"\

    // Wins
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvTournamentEmpireData [SystemTournamentEmpires::Wins].GetInteger());
    
    OutputText (
        "</td>"\

    // Nukes
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvTournamentEmpireData [SystemTournamentEmpires::Nukes].GetInteger());
    
    OutputText (
        "</td>"\

    // Nuked
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvTournamentEmpireData [SystemTournamentEmpires::Nuked].GetInteger());
    
    OutputText (
        "</td>"\

    // Draws
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvTournamentEmpireData [SystemTournamentEmpires::Draws].GetInteger());
    
    OutputText (
        "</td>"\

    // Ruins
        "<td align=\"center\">"
        );

    m_pHttpResponse->WriteText (pvTournamentEmpireData [SystemTournamentEmpires::Ruins].GetInteger());
    
    OutputText (
        "</td>"\

        "</tr>"
        );

Cleanup:

    if (pvTournamentEmpireData != NULL) {
        g_pGameEngine->FreeData (pvTournamentEmpireData);
    }
}