<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

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

// Check for submission
bool bFlag, bCorrectLogin = false;
int iErrCode;

m_iEmpireKey = NO_KEY;

if (m_pHttpRequest->GetMethod() == GET) {

	if (g_pGameEngine->IsDatabaseBackingUp()) {
		WriteBackupMessage();
	} else {

		// Look for cookies
		ICookie* pAutoLogonEmpire, * pPasswordCookie;
		int iAutoLogonKey = NO_KEY;
		uint64 ui64SubmittedPasswordHash = -1, ui64RealPasswordHash;

		pAutoLogonEmpire = m_pHttpRequest->GetCookie (AUTOLOGON_EMPIREKEY_COOKIE);
		pPasswordCookie = m_pHttpRequest->GetCookie (AUTOLOGON_PASSWORD_COOKIE);

		if (pAutoLogonEmpire != NULL && pAutoLogonEmpire->GetValue() != NULL) {
			iAutoLogonKey = pAutoLogonEmpire->GetIntValue();
		}

		if (pPasswordCookie != NULL && pPasswordCookie->GetValue() != NULL) {
			ui64SubmittedPasswordHash = pPasswordCookie->GetInt64Value();
		}

		if (iAutoLogonKey != NO_KEY && ui64SubmittedPasswordHash != -1) {

			iErrCode = g_pGameEngine->DoesEmpireExist (iAutoLogonKey, &bFlag);
			if (iErrCode == OK && bFlag) {

				iErrCode = g_pGameEngine->GetEmpirePassword (iAutoLogonKey, &m_vPassword);
				if (iErrCode == OK) {

					// Authenticate
					ui64RealPasswordHash = GetPasswordHash();

					if (ui64RealPasswordHash == ui64SubmittedPasswordHash) {

						m_iEmpireKey = iAutoLogonKey;
						iErrCode = LoginEmpire();
						if (iErrCode == OK) {
							return Redirect (ACTIVE_GAME_LIST);
						} else {
							AddMessage ("Login failed");
						}

						m_iEmpireKey = NO_KEY;
					}

					m_vPassword = "";
				}
			}

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

else if (!m_bRedirection &&
	(pHttpForm = m_pHttpRequest->GetForm ("PageId")) != NULL &&
	pHttpForm->GetIntValue() == m_pgPageId) {

	// Make sure we're not backing up
	if (g_pGameEngine->IsDatabaseBackingUp()) {

		WriteBackupMessage();

	} else {

		// Get empire name
		pHttpForm = m_pHttpRequest->GetForm ("EmpireName");
		if (pHttpForm != NULL) {

			// Make sure the name is valid
			const char* pszEmpireName = pHttpForm->GetValue();
			char pszStandardizedName [MAX_EMPIRE_NAME_LENGTH + 1];

			if (VerifyEmpireName (pszEmpireName) == OK &&
				StandardizeEmpireName (pszEmpireName, pszStandardizedName) == OK) {

				bCorrectLogin = true;

				// Get password
				pHttpForm = m_pHttpRequest->GetForm ("Password");
				if (pHttpForm != NULL) {

					// Make sure password is value
					const char* pszPassword = pHttpForm->GetValue();
					iErrCode = VerifyPassword (pszPassword);

					if (iErrCode == OK) {

						// Test empire existence
						iErrCode = g_pGameEngine->DoesEmpireExist (
							pszStandardizedName,
							&bFlag, 
							&m_iEmpireKey,
							&m_vEmpireName
							);

						if (iErrCode == OK && bFlag) {

							// Check password
							iErrCode = g_pGameEngine->IsPasswordCorrect (m_iEmpireKey, pszPassword);
							if (iErrCode != OK) {

								char pszBuffer [128 + MAX_EMPIRE_NAME_LENGTH];
								sprintf (
									pszBuffer,
									"That was not the right password for the %s empire",
									m_vEmpireName.GetCharPtr()
									);

								// Message
								AddMessage (pszBuffer);

								// Add to report
								ReportLoginFailure (g_pReport, m_vEmpireName.GetCharPtr());

							} else {

								m_vPassword = pszPassword;

								iErrCode = LoginEmpire();
								if (iErrCode == OK) {

									m_bAuthenticated = true;

									iErrCode = InitializeEmpire();
									if (iErrCode == OK) {
										return Redirect (ACTIVE_GAME_LIST);
									}
								}
							}

						} else {

							// Make sure access is allowed
							int iOptions;
							iErrCode = g_pGameEngine->GetSystemOptions (&iOptions);
							if ((iErrCode != OK || !(iOptions & LOGINS_ENABLED)) && m_iPrivilege < ADMINISTRATOR) {

								// Get reason
								char pszReason [128 + MAX_REASON_LENGTH] = 
									"Access is denied to the server at this time. ";

								Variant vReason;

								if (g_pGameEngine->GetAccessDisabledReason (&vReason) == OK) {
									strcat (pszReason, vReason.GetCharPtr());
								}
								AddMessage (pszReason);

							} else {

								// We're a new empire, so redirect to NewEmpire
								m_vEmpireName = pszStandardizedName;
								m_vPassword = pszPassword;

								return Redirect (NEW_EMPIRE);
							}
						}
					}
				}
			}
		}
	}
}

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
	iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bFlag);
	if (!bFlag || iErrCode != OK) {
		m_iEmpireKey = NO_KEY;
	}
}

if (m_iEmpireKey == NO_KEY) {

	int iLivePlanetKey, iDeadPlanetKey, iHorz, iVert, iColor;

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

	int iThemeKey;
	iErrCode = g_pGameEngine->GetEmpireThemeKey (m_iEmpireKey, &iThemeKey);
	if (iErrCode == OK) {
		GetUIData (iThemeKey);
	} else {
		GetUIData (NULL_THEME);
	}
}

%><html><head><title><% WriteSystemTitleString(); %></title></head><% 
WriteBodyString (-1);
%><center><% 

WriteAlmonasterBanner();

%><table width="60%"><tr><td><%

WriteIntro();

%></td></tr></table><p><% 

WriteSeparatorString (m_iSeparatorKey);

%><p><table width="60%"><tr><td><%

WriteIntroUpper();

%></td></tr></table><p><% 

if (!m_strMessage.IsBlank()) {
	%><p><strong><%
	Write (m_strMessage.GetCharPtr());
	%></strong><%
}

OpenForm();

// POST graphics information to NewEmpire page
%><input type="hidden" name="ButtonKey" value="<% Write (m_iButtonKey); %>"><%
%><input type="hidden" name="BackgroundKey" value="<% Write (m_iBackgroundKey); %>"><%
%><input type="hidden" name="SeparatorKey" value="<% Write (m_iSeparatorKey); %>"><%
%><input type="hidden" name="EmpireKey" value="<% Write (m_iEmpireKey); %>"><%
%><input type="hidden" name="TextColor" value="<% Write (m_vTextColor.GetCharPtr()); %>"><%
%><input type="hidden" name="GoodColor" value="<% Write (m_vGoodColor.GetCharPtr()); %>"><%
%><input type="hidden" name="BadColor" value="<% Write (m_vBadColor.GetCharPtr()); %>"><%

int iOptions;
Check (g_pGameEngine->GetSystemOptions (&iOptions));
if (!(iOptions & LOGINS_ENABLED)) {

	%><strong>The server is denying all logins at this time. <%

	Variant vReason;
	Check (g_pGameEngine->GetLoginsDisabledReason (&vReason));
	const char* pszReason = vReason.GetCharPtr();
	if (pszReason == NULL || *pszReason == '\0') {
		%>Please try back later.<%
	} else {
		Write (pszReason);
	}
	%></strong><%

} else {

	%><p><table><%

	%><tr><td align="right"><b>Empire Name:</b></td><%
	%><td><input type="text" size="20" maxlength="<% Write (MAX_EMPIRE_NAME_LENGTH); %>" name="EmpireName"<% 

	const char* pszEmpireName = m_vEmpireName.GetCharPtr();

	if (bCorrectLogin && pszEmpireName != NULL && *pszEmpireName != '\0') {
		%> value="<% Write (m_vEmpireName.GetCharPtr()); %>"<%
	}

	%>></td></tr><%

	%><tr><td align="right"><strong>Password:</strong></td><%
	%><td><input type="password" size="20" maxlength="<% Write (MAX_PASSWORD_LENGTH); %>" name="Password"></td><%
	%></tr></table><%

	%><p><% WriteButton (BID_LOGIN);

	%><p><table width="60%"><tr><td><%

	WriteIntroLower();

	%></td></tr></table><%
}

CloseSystemPage();

return OK; %>