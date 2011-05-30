
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Login page
int HtmlRenderer::Render_Login() {

	// Almonaster 2.0
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

	// Get objects
	IHttpForm* pHttpForm;

	const char* pszPrintEmpireName = NULL;
	char pszStandardizedName [MAX_EMPIRE_NAME_LENGTH + 1];

	// Check for submission
	bool bFlag;
	int iErrCode;

	m_iEmpireKey = NO_KEY;
	m_vEmpireName = m_vPassword = (const char*) NULL;

	if (m_pHttpRequest->GetMethod() == GET) {

	    if (g_pGameEngine->IsDatabaseBackingUp()) {
	        WriteBackupMessage();
	    } else {

	        // Look for cookies
	        ICookie* pAutoLogonEmpire, * pPasswordCookie;
	        unsigned int iAutoLogonKey = NO_KEY;
	        int64 i64SubmittedPasswordHash = -1, i64RealPasswordHash;

	        pAutoLogonEmpire = m_pHttpRequest->GetCookie (AUTOLOGON_EMPIREKEY_COOKIE);
	        pPasswordCookie = m_pHttpRequest->GetCookie (AUTOLOGON_PASSWORD_COOKIE);

	        if (pAutoLogonEmpire != NULL && pAutoLogonEmpire->GetValue() != NULL) {
	            iAutoLogonKey = pAutoLogonEmpire->GetUIntValue();
	        }

	        if (pPasswordCookie != NULL && pPasswordCookie->GetValue() != NULL) {
	            i64SubmittedPasswordHash = pPasswordCookie->GetInt64Value();
	        }

	        if (iAutoLogonKey != NO_KEY && i64SubmittedPasswordHash != -1) {

	            Variant vValue;

	            if (g_pGameEngine->DoesEmpireExist (iAutoLogonKey, &bFlag, NULL) == OK && 
	                bFlag &&
	                g_pGameEngine->GetEmpirePassword (iAutoLogonKey, &m_vPassword) == OK &&
	                g_pGameEngine->GetEmpireProperty (iAutoLogonKey, SystemEmpireData::SecretKey, &vValue) == OK) {

	                // Authenticate
	                m_i64SecretKey = vValue.GetInteger64();

	                if (GetPasswordHashForAutologon (&i64RealPasswordHash) == OK) {

	                    if (i64RealPasswordHash == i64SubmittedPasswordHash) {

	                        m_iEmpireKey = iAutoLogonKey;
	                        m_bAutoLogon = true;

	                        if (LoginEmpire() == OK && InitializeEmpire (true) == OK) {
	                            return Redirect (ACTIVE_GAME_LIST);
	                        }

	                        AddMessage ("Login failed");
	                    }
	                }
	            }

	            m_iEmpireKey = NO_KEY;
	            m_vPassword = (const char*) NULL;
	            m_i64SecretKey = 0;

	            // Autologon failed
	            AddMessage ("Autologon failed and was disabled");
	        }

	        if (pAutoLogonEmpire != NULL) {
	            m_pHttpResponse->DeleteCookie (AUTOLOGON_EMPIREKEY_COOKIE, NULL);
	        }

	        if (pPasswordCookie != NULL) {
	            m_pHttpResponse->DeleteCookie (AUTOLOGON_PASSWORD_COOKIE, NULL);
	        }
	    }
	}

	else if (!m_bRedirection) {

	    Variant vEmpireName;
	    const char* pszEmpireName, * pszPassword;

	    // Make sure we're not backing up
	    if (g_pGameEngine->IsDatabaseBackingUp()) {
	        WriteBackupMessage();
	        goto Text;
	    }

	    // Get empire name
	    pHttpForm = m_pHttpRequest->GetForm ("EmpireName");
	    if (pHttpForm == NULL) {
	        goto Text;
	    }

	    // Make sure the name is valid
	    pszEmpireName = pHttpForm->GetValue();

	    if (VerifyEmpireName (pszEmpireName) != OK || 
	        StandardizeEmpireName (pszEmpireName, pszStandardizedName) != OK) {
	        goto Text;
	    }

	    pszPrintEmpireName = pszStandardizedName;

	    // Get password
	    pHttpForm = m_pHttpRequest->GetForm ("Password");
	    if (pHttpForm == NULL) {
	        goto Text;
	    }

	    // Make sure password is valid
	    pszPassword = pHttpForm->GetValue();
	    iErrCode = VerifyPassword (pszPassword);
	    if (iErrCode != OK) {
	        goto Text;
	    }

	    // Test empire existence
	    iErrCode = g_pGameEngine->DoesEmpireExist (
	        pszStandardizedName,
	        &bFlag, 
	        &m_iEmpireKey,
	        &vEmpireName,
	        NULL
	        );

	    if (iErrCode != OK) {
	        AddMessage ("GameEngine::DoesEmpireExist returned ");
	        AppendMessage (iErrCode);
	        goto Text;
	    }

	    if (m_pHttpRequest->GetFormBeginsWith ("CreateEmpire")) {

	        if (bFlag) {
	            AddMessage ("The ");
	            AppendMessage (pszPrintEmpireName);
	            AppendMessage (" empire already exists");
	            goto Text;
	        }

	        // We're a new empire, so redirect to NewEmpire
	        m_vEmpireName = pszStandardizedName;
	        m_vPassword = pszPassword;

	        return Redirect (NEW_EMPIRE);
	    }

	    else if (m_pHttpRequest->GetFormBeginsWith ("BLogin") ||
	             m_pHttpRequest->GetFormBeginsWith ("TransDot")) {

	        if (bFlag) {

	            // Check password
	            iErrCode = g_pGameEngine->IsPasswordCorrect (m_iEmpireKey, pszPassword);
	            if (iErrCode != OK) {

	                char pszBuffer [128 + MAX_EMPIRE_NAME_LENGTH];
	                sprintf (
	                    pszBuffer,
	                    "That was not the right password for the %s empire",
	                    vEmpireName.GetCharPtr()
	                    );

	                // Message
	                AddMessage (pszBuffer);

	                // Add to report
	                ReportLoginFailure (g_pReport, vEmpireName.GetCharPtr());

	            } else {

	                m_vEmpireName = vEmpireName;
	                m_vPassword = pszPassword;

	                if (LoginEmpire() == OK && InitializeEmpire (false) == OK) {
	                    AddMessage ("Welcome back, ");
	                    AppendMessage (vEmpireName.GetCharPtr());
	                    return Redirect (ACTIVE_GAME_LIST);
	                }

	                m_vEmpireName = (const char*) NULL;
	                m_vPassword = (const char*) NULL;
	            }

	        } else {

	            // Make sure access is allowed
	            int iOptions;
	            iErrCode = g_pGameEngine->GetSystemOptions (&iOptions);
	            if ((iErrCode != OK || !(iOptions & LOGINS_ENABLED)) && m_iPrivilege < ADMINISTRATOR) {

	                // Get reason
	                AddMessage ("Access is denied to the server at this time. ");

	                Variant vReason;
	                if (g_pGameEngine->GetSystemProperty (SystemData::AccessDisabledReason, &vReason) == OK) {
	                    AppendMessage (vReason.GetCharPtr());
	                }

	            } else {

	                // We're a new empire, so redirect to NewEmpire
	                m_vEmpireName = pszStandardizedName;
	                m_vPassword = pszPassword;

	                return Redirect (NEW_EMPIRE);
	            }
	        }
	    }
	}


	Text:

	if (m_strMessage.IsBlank()) {
	    // Check if we're backing up
	    if (g_pGameEngine->IsDatabaseBackingUp()) {
	        WriteBackupMessage();
	    }
	}

	// Get a cookie for last empire used's graphics
	ICookie* pCookie = m_pHttpRequest->GetCookie (LAST_EMPIRE_USED_COOKIE);
	if (pCookie != NULL && pCookie->GetValue() != NULL) {

	    m_iEmpireKey = pCookie->GetIntValue();
	    iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bFlag, &m_vEmpireName);
	    if (!bFlag || iErrCode != OK) {
	        m_iEmpireKey = NO_KEY;
	    }

	    if (pszPrintEmpireName == NULL) {
	        pszPrintEmpireName = m_vEmpireName.GetCharPtr();
	    }
	}

	if (m_iEmpireKey == NO_KEY) {

	    unsigned int iLivePlanetKey, iDeadPlanetKey, iHorz, iVert, iColor;

	    iErrCode = g_pGameEngine->GetDefaultUIKeys (
	        &m_iBackgroundKey,
	        &iLivePlanetKey,
	        &iDeadPlanetKey,
	        &m_iButtonKey,
	        &m_iSeparatorKey,
	        &iHorz,
	        &iVert,
	        &iColor
	        );

	    if (iErrCode != OK || iColor == NULL_THEME) {

	        m_vTextColor = DEFAULT_TEXT_COLOR;
	        m_vGoodColor = DEFAULT_GOOD_COLOR;
	        m_vBadColor = DEFAULT_BAD_COLOR;

	    } else {

	        iErrCode = g_pGameEngine->GetThemeTextColor (iColor, &m_vTextColor);
	        if (iErrCode != OK) {
	            m_vTextColor = DEFAULT_TEXT_COLOR;
	        }

	        iErrCode = g_pGameEngine->GetThemeGoodColor (iColor, &m_vGoodColor);
	        if (iErrCode != OK) {
	            m_vGoodColor = DEFAULT_GOOD_COLOR;
	        }

	        iErrCode = g_pGameEngine->GetThemeBadColor (iColor, &m_vBadColor);
	        if (iErrCode != OK) {
	            m_vBadColor = DEFAULT_BAD_COLOR;
	        }
	    }

	} else {

	    Variant vValue;
	    iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vValue);
	    if (iErrCode == OK) {
	        GetUIData (vValue.GetInteger());
	    } else {
	        GetUIData (NULL_THEME);
	    }
	}

	int iOptions;
	iErrCode = g_pGameEngine->GetSystemOptions (&iOptions);
	if (iErrCode != OK) {
	    m_pHttpResponse->SetStatusCode (HTTP_500);
	    return iErrCode;
	}


	Write ("<html><head><title>", sizeof ("<html><head><title>") - 1);
	WriteSystemTitleString(); 
	Write ("</title></head>", sizeof ("</title></head>") - 1);
	WriteBodyString (-1);
	OpenForm();

	// POST graphics information to NewEmpire page

	Write ("<input type=\"hidden\" name=\"ButtonKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"ButtonKey\" value=\"") - 1);
	Write (m_iButtonKey); 
	Write ("\"><input type=\"hidden\" name=\"BackgroundKey\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"BackgroundKey\" value=\"") - 1);
	Write (m_iBackgroundKey); 
	Write ("\"><input type=\"hidden\" name=\"SeparatorKey\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"SeparatorKey\" value=\"") - 1);
	Write (m_iSeparatorKey); 
	Write ("\"><input type=\"hidden\" name=\"EmpireKey\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"EmpireKey\" value=\"") - 1);
	Write (m_iEmpireKey); 
	Write ("\"><input type=\"hidden\" name=\"TextColor\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"TextColor\" value=\"") - 1);
	Write (m_vTextColor.GetCharPtr()); 
	Write ("\"><input type=\"hidden\" name=\"GoodColor\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"GoodColor\" value=\"") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\"><input type=\"hidden\" name=\"BadColor\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"BadColor\" value=\"") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\"><center><table align=\"center\" width=\"90%\" cellpadding=\"0\" cellspacing=\"0\"><tr><td width=\"42%\">", sizeof ("\"><center><table align=\"center\" width=\"90%\" cellpadding=\"0\" cellspacing=\"0\"><tr><td width=\"42%\">") - 1);
	WriteIntro();


	Write ("</td><td width=\"58%\" align=\"center\">", sizeof ("</td><td width=\"58%\" align=\"center\">") - 1);
	WriteAlmonasterBanner();


	Write ("<p>", sizeof ("<p>") - 1);
	WriteIntroUpper();

	if (!m_strMessage.IsBlank()) {
	    
	Write ("<p><strong>", sizeof ("<p><strong>") - 1);
	Write (m_strMessage.GetCharPtr(), m_strMessage.GetLength());
	    
	Write ("</strong>", sizeof ("</strong>") - 1);
	}

	if (!(iOptions & LOGINS_ENABLED)) {

	    
	Write ("<p><strong>The server is denying all logins at this time. ", sizeof ("<p><strong>The server is denying all logins at this time. ") - 1);
	Variant vReason;
	    const char* pszReason = NULL;

	    iErrCode = g_pGameEngine->GetSystemProperty (SystemData::LoginsDisabledReason, &vReason);
	    if (iErrCode == OK) {
	        pszReason = vReason.GetCharPtr();
	    }

	    if (String::IsBlank (pszReason)) {
	        
	Write ("Please try back later.", sizeof ("Please try back later.") - 1);
	} else {
	        Write (pszReason);
	    }
	    
	Write ("</strong>", sizeof ("</strong>") - 1);
	} else {

	    
	Write ("<p><table align=\"center\"><tr><td align=\"right\"><strong>Empire Name:</strong></td><td><input type=\"text\" size=\"20\" tabindex=\"32767\" maxlength=\"", sizeof ("<p><table align=\"center\"><tr><td align=\"right\"><strong>Empire Name:</strong></td><td><input type=\"text\" size=\"20\" tabindex=\"32767\" maxlength=\"") - 1);
	Write (MAX_EMPIRE_NAME_LENGTH); 
	Write ("\" name=\"EmpireName\"", sizeof ("\" name=\"EmpireName\"") - 1);
	if (pszPrintEmpireName != NULL) {
	        
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (pszPrintEmpireName); 
	Write ("\"", sizeof ("\"") - 1);
	}
	    
	Write ("></td></tr><tr><td align=\"right\"><strong>Password:</strong></td><td><input type=\"password\" size=\"20\" tabindex=\"32767\" maxlength=\"", sizeof ("></td></tr><tr><td align=\"right\"><strong>Password:</strong></td><td><input type=\"password\" size=\"20\" tabindex=\"32767\" maxlength=\"") - 1);
	Write (MAX_PASSWORD_LENGTH); 
	Write ("\" name=\"Password\"></td></tr></table><p><input border=\"0\" type=\"image\" src=\"", sizeof ("\" name=\"Password\"></td></tr></table><p><input border=\"0\" type=\"image\" src=\"") - 1);
	Write (BASE_RESOURCE_DIR TRANSPARENT_DOT); 
	Write ("\" name=\"TransDot\">", sizeof ("\" name=\"TransDot\">") - 1);
	if (iOptions & NEW_EMPIRES_ENABLED) {
	        WriteButton (BID_CREATEEMPIRE); 
	Write (" ", sizeof (" ") - 1);
	} else {
	        
	Write ("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;", sizeof ("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;") - 1);
	}
	    WriteButton (BID_LOGIN);
	}


	Write ("<p>", sizeof ("<p>") - 1);
	WriteIntroLower();

	WriteContactLine();


	Write ("</td></tr></table></center></form></body></html>", sizeof ("</td></tr></table></center></form></body></html>") - 1);
	return OK; 
}