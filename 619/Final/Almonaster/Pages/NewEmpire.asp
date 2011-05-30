<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

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

int iEmpireKey = NO_KEY, iParentEmpireKey = NO_KEY, iErrCode = OK;
bool bFlag, bVerified = false, bRepost = false;

IHttpForm* pHttpForm;

// Get keys
if ((pHttpForm = m_pHttpRequest->GetForm ("ButtonKey")) == NULL) {
	AddMessage ("Missing ButtonKey form");
	return Redirect (LOGIN);
}

m_iButtonKey = pHttpForm->GetIntValue();

if ((pHttpForm = m_pHttpRequest->GetForm ("TextColor")) == NULL) {
	AddMessage ("Missing TextColor form");
	return Redirect (LOGIN);
}

m_vTextColor = pHttpForm->GetValue();

if ((pHttpForm = m_pHttpRequest->GetForm ("GoodColor")) == NULL) {
	AddMessage ("Missing GoodColor form");
	return Redirect (LOGIN);
}

m_vGoodColor = pHttpForm->GetValue();

if ((pHttpForm = m_pHttpRequest->GetForm ("BadColor")) == NULL) {
	AddMessage ("Missing BadColor form");
	return Redirect (LOGIN);
}

m_vBadColor = pHttpForm->GetValue();

// Handle submissions from NewEmpire page
if (!m_bRedirection &&
	(pHttpForm = m_pHttpRequest->GetForm ("PageId")) != NULL &&
	pHttpForm->GetIntValue() == m_pgPageId) {

	bRepost = true;

	// Get login
	if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireName")) == NULL) {
		AddMessage ("Missing EmpireName form");
		return Redirect (LOGIN);
	}
	m_vEmpireName = pHttpForm->GetValue();

	if (m_vEmpireName.GetCharPtr() == NULL) {
		AddMessage ("Out of memory");
		return Redirect (LOGIN);
	}

	// Check for cancel
	if (WasButtonPressed (BID_CANCEL)) {
		return Redirect (LOGIN);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireProxy")) == NULL) {
		AddMessage ("Missing EmpireProxy form");
		return Redirect (LOGIN);
	}
	iEmpireKey = pHttpForm->GetIntValue();

	// Get password
	if ((pHttpForm = m_pHttpRequest->GetForm ("Password")) == NULL) {
		AddMessage ("Missing Password form");
		return Redirect (LOGIN);
	}
	m_vPassword = pHttpForm->GetValue();

	if (m_vPassword.GetCharPtr() == NULL) {
		AddMessage ("Out of memory");
		return Redirect (LOGIN);
	}

	if (m_iButtonKey == INDIVIDUAL_ELEMENTS) {
		iErrCode = g_pGameEngine->GetEmpireButtonKey (iEmpireKey, &m_iButtonKey);
	}

	if (WasButtonPressed (BID_CREATEEMPIRE)) {

		if ((pHttpForm = m_pHttpRequest->GetForm ("PasswordCopy")) == NULL) {
			AddMessage ("Missing PasswordCopy form");
			return Redirect (LOGIN);
		}

		if (String::StrCmp (m_vPassword.GetCharPtr(), pHttpForm->GetValue()) != 0) {
			AddMessage ("Your password wasn't verified correctly");
		} else {

			bVerified = true;

			// Get parent empire's name and password
			if ((pHttpForm = m_pHttpRequest->GetForm ("ParentEmpireName")) == NULL) {
				AddMessage ("Missing ParentEmpireName form");
				return Redirect (LOGIN);
			}

			// Make sure the name is valid (if it was submitted)
			const char* pszParentName = pHttpForm->GetValue();
			char pszStandardParentName [MAX_EMPIRE_NAME_LENGTH + 1];
			if (pszParentName != NULL) {

				if (VerifyEmpireName (pszParentName) == OK &&
					StandardizeEmpireName (pszParentName, pszStandardParentName) == OK) {

					if ((pHttpForm = m_pHttpRequest->GetForm ("ParentPassword")) == NULL) {
						AddMessage ("Missing ParentPassword form");
						return Redirect (LOGIN);
					}

					if (VerifyPassword (pHttpForm->GetValue()) == OK) {

						Variant vParentName;
						iErrCode = g_pGameEngine->DoesEmpireExist (
							pszStandardParentName, 
							&bFlag, 
							&iParentEmpireKey, 
							&vParentName
							);

						if (!bFlag) {
							AddMessage ("The parent empire ");
							AppendMessage (pszStandardParentName);
							AppendMessage (" does not exist");
						} else {

							iErrCode = g_pGameEngine->IsPasswordCorrect (iParentEmpireKey, pHttpForm->GetValue());

							if (iErrCode != OK) {
								AddMessage ("That was the wrong password for the parent empire");
							}
						}
					}

				} else {
					AddMessage ("The parent empire does not exist");
				}
			}

			const char* pszEmpireName, * pszPassword;

			if (m_strMessage.IsBlank()) {

				// Empire name
				if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireName")) == NULL) {
					AddMessage ("Missing EmpireName form");
					return Redirect (LOGIN);
				}
				pszEmpireName = pHttpForm->GetValue();

				// Empire password
				if ((pHttpForm = m_pHttpRequest->GetForm ("Password")) == NULL) {
					AddMessage ("Missing Password form");
					return Redirect (LOGIN);
				}
				pszPassword = pHttpForm->GetValue();

				// Check for no new empires allowed
				int iOptions;
				iErrCode = g_pGameEngine->GetSystemOptions (&iOptions);
				if (iErrCode != OK || !(iOptions & NEW_EMPIRES_ENABLED)) {

					// Get reason
					Variant vReason;
					String strMessage = "New empire creation is disabled at this time. ";

					if (g_pGameEngine->GetEmpireCreationDisabledReason (&vReason) == OK) {
						strMessage += vReason.GetCharPtr();
					}
					AddMessage (strMessage);

				} else {

					iErrCode = g_pGameEngine->CreateEmpire (
						pszEmpireName, 
						pszPassword, 
						NOVICE, 
						iParentEmpireKey,
						&iEmpireKey
						);

					switch (iErrCode) {

					case OK:

						{

						const char* pszString;

						// Real name
						if ((pHttpForm = m_pHttpRequest->GetForm ("RealName")) == NULL) {
							AddMessage ("Missing RealName form");
							return Redirect (LOGIN);
						} else {

							pszString = pHttpForm->GetValue();
							if (pszString == NULL) {
								pszString = "";
							}

							else if (strlen (pszString) > MAX_REAL_NAME_LENGTH) {
								AddMessage ("Your real name was too long");
							}

							else {

								// Best effort
								iErrCode = g_pGameEngine->SetEmpireRealName (iEmpireKey, pszString);
								if (iErrCode != OK) {
									AddMessage ("Your real name could not be written. The error was ");
									AppendMessage (iErrCode);
								}
							}
						}

						// Email address
						if ((pHttpForm = m_pHttpRequest->GetForm ("Email")) == NULL) {
							AddMessage ("Missing Email form");
						} else {

							pszString = pHttpForm->GetValue();
							if (pszString == NULL) {
								pszString = "";
							}

							else if (strlen (pszString) > MAX_EMAIL_LENGTH) {
								AddMessage ("Your Email address was too long");
							}

							else {

								// Best effort
								iErrCode = g_pGameEngine->SetEmpireEmail (iEmpireKey, pszString);
								if (iErrCode != OK) {
									AddMessage ("Your email address could not be written. The error was ");
									AppendMessage (iErrCode);
								}
							}
						}

						// URL
						if ((pHttpForm = m_pHttpRequest->GetForm ("WebPageURL")) == NULL) {
							AddMessage ("Missing WebPageURL form");
							return Redirect (LOGIN);
						} else {

							pszString = pHttpForm->GetValue();
							if (pszString == NULL) {
								pszString = "";
							}

							else if (strlen (pszString) > MAX_WEB_PAGE_LENGTH) {
								AddMessage ("Your WebPage URL was too long");
								return Redirect (NEW_EMPIRE);
							}

							else {

								// Best effort
								iErrCode = g_pGameEngine->SetEmpireWebPage (iEmpireKey, pszString);
								if (iErrCode != OK) {
									AddMessage ("Your web page url could not be written. The error was ");
									AppendMessage (iErrCode);
								}
							}
						}

						ReportEmpireCreation (g_pReport, pszEmpireName);
						SendWelcomeMessage (iEmpireKey, pszEmpireName);

						m_iEmpireKey = iEmpireKey;
						m_iReserved = 0;

						iErrCode = LoginEmpire();
						if (iErrCode == OK) {
							return Redirect (ACTIVE_GAME_LIST);
						}

						AddMessage ("Login failed");
						return Redirect (LOGIN);

						}

					case ERROR_DISABLED:

						{

						String strMessage = "The server is denying all new empire creation attempts at this time. ";
						Variant vReason;
						Check (g_pGameEngine->GetEmpireCreationDisabledReason (&vReason));
						const char* pszReason = vReason.GetCharPtr();
						if (pszReason == NULL || *pszReason == '\0') {
							strMessage += "Please try back later.";
						} else {
							strMessage += pszReason;
						}
						AddMessage (strMessage);

						}
						break;

					case ERROR_COULD_NOT_DELETE_EMPIRE:

						AddMessage ("The parent empire is in at least one active game and could not be deleted");
						break;

					case ERROR_COULD_NOT_DELETE_ADMINISTRATOR:

						AddMessage ("The parent empire is an administrator and cannot be inherited from");
						break;

					case ERROR_RESERVED_EMPIRE_NAME:

						AddMessage ("The given empire name is reserved");
						break;

					case ERROR_EMPIRE_ALREADY_EXISTS:

						AddMessage ("The given empire name is already in use");
						break;

					default:

						AddMessage ("An unknown error occurred creating the empire ");
						AppendMessage (iErrCode);
						break;
					}
				}
			}
		}
	}
}

// Get graphics used by Login screen
if (m_bRedirection) {

	pHttpForm = m_pHttpRequest->GetForm ("EmpireKey");
	if (pHttpForm == NULL) {
		// AddMessage ("Missing EmpireKey form");
		return Redirect (LOGIN);
	}
	iEmpireKey = pHttpForm->GetIntValue();

} else {

	pHttpForm = m_pHttpRequest->GetForm ("EmpireProxy");
	if (pHttpForm == NULL) {
		// AddMessage ("Missing EmpireProxy form");
		return Redirect (LOGIN);
	}
	iEmpireKey = pHttpForm->GetIntValue();
}

pHttpForm = m_pHttpRequest->GetForm ("BackgroundKey");
if (pHttpForm == NULL) {
	AddMessage ("Missing BackgroundKey form");
	return Redirect (LOGIN);
}
m_iBackgroundKey = pHttpForm->GetIntValue();

pHttpForm = m_pHttpRequest->GetForm ("SeparatorKey");
if (pHttpForm == NULL) {
	AddMessage ("Missing SeparatorKey form");
	return Redirect (LOGIN);
}
m_iSeparatorKey = pHttpForm->GetIntValue();

%><html><head><title><% WriteSystemTitleString(); %></title></head><%

WriteBodyString (-1);

%><center><h1>New Empire</h1><%

// Get a cookie for last empire used's graphics
ICookie* pCookie = m_pHttpRequest->GetCookie ("LastEmpireUsed");

if (pCookie != NULL && pCookie->GetValue() != NULL) {

	iEmpireKey = pCookie->GetIntValue();
	iErrCode = g_pGameEngine->DoesEmpireExist (iEmpireKey, &bFlag);
	if (!bFlag || iErrCode != OK) {
		iEmpireKey = NO_KEY;
	}
}

%><p><%

WriteSeparatorString (m_iSeparatorKey);

if (!m_strMessage.IsBlank()) {
	%><p><strong><%
	Write (m_strMessage);
	%></strong><%
}

OpenForm();

// POST password and graphics information to NewEmpire page in case we need to return 
%><input type="hidden" name="EmpireProxy" value="<% Write (iEmpireKey); %>"><%
%><input type="hidden" name="EmpireName" value="<% Write (m_vEmpireName); %>"><%
%><input type="hidden" name="Password" value="<% Write (m_vPassword); %>"><%
%><input type="hidden" name="ButtonKey" value="<% Write (m_iButtonKey); %>"><%
%><input type="hidden" name="BackgroundKey" value="<% Write (m_iBackgroundKey); %>"><%
%><input type="hidden" name="SeparatorKey" value="<% Write (m_iSeparatorKey); %>"><%
%><input type="hidden" name="TextColor" value="<% Write (m_vTextColor.GetCharPtr()); %>"><%
%><input type="hidden" name="GoodColor" value="<% Write (m_vGoodColor.GetCharPtr()); %>"><%
%><input type="hidden" name="BadColor" value="<% Write (m_vBadColor.GetCharPtr()); %>"><%

%><p><table width="75%"><td>Please retype your password to confirm it. Your real name, email address, <%
%>Web Page URL and quote are optional and will be visible to all other players.</strong></td></table><%

%><p><table width="60%"><%

%><tr><td><strong>Empire name</strong>:</td><td><strong><% Write (m_vEmpireName); %></strong></td></tr><%
%><tr><td><strong>Password</strong>:</td><td><strong><% 

size_t i, stLength = m_vPassword.GetLength();
for (i = 0; i < stLength; i ++) {
	%>*<%
}
%></strong></td></tr><%

%><tr><td><strong><%
%>Verify your password (<font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
%>required</font>)</strong>:</td><%
%><td><input type="password" name="PasswordCopy" size="20"<%

if (bRepost && bVerified && (pHttpForm = m_pHttpRequest->GetForm ("PasswordCopy")) != NULL && 
	pHttpForm->GetValue() != NULL) {
	%> value="<% Write (pHttpForm->GetValue()); %>"<%
}

%> maxlength="<% Write (MAX_PASSWORD_LENGTH); %>"></td></tr><tr><%
%><td><strong>Real name (<font color="#<% Write (m_vGoodColor.GetCharPtr()); %>">optional</font><%
%>)</strong>:</td><td><input type="text" name="RealName" size="30"<%

if (bRepost && (pHttpForm = m_pHttpRequest->GetForm ("RealName")) != NULL && pHttpForm->GetValue() != NULL) {
	%> value="<% Write (pHttpForm->GetValue()); %>"<%
} %> maxlength="<% Write (MAX_REAL_NAME_LENGTH); %>"></td></tr><%

%><tr><td><strong>Email address (<font color="#<% Write (m_vGoodColor.GetCharPtr()); %>">optional</font><%
%>)</strong>:</td><td><input type="text" name="Email" size="40"<% 

if (bRepost && (pHttpForm = m_pHttpRequest->GetForm ("Email")) != NULL && pHttpForm->GetValue() != NULL) {
	%> value="<% Write (pHttpForm->GetValue()); %>"<%
} %> maxlength="<% Write (MAX_EMAIL_LENGTH); %>"></td></tr><%

%><tr><td><strong>Web Page URL (<font color="#<% Write (m_vGoodColor.GetCharPtr()); %>">optional</font><%
%>)</strong>:</td><td><input type="text" name="WebPageURL" size="40"<%

if (bRepost && (pHttpForm = m_pHttpRequest->GetForm ("WebPageURL")) != NULL && pHttpForm->GetValue() != NULL) {
	%> value="<% Write (pHttpForm->GetValue()); %>"<%
} %> maxlength="<% Write (MAX_WEB_PAGE_LENGTH); %>"></td></tr></table><p><% 

WriteButton (BID_CANCEL);
WriteButton (BID_CREATEEMPIRE);

%><p><table width="75%"><tr><%
%><td>If you are an experienced player and you already have an empire registered on this server, you can <%
%>inherit that empire's Almonaster Score and Privilege Level with this empire. The empire that you inherit <%
%>from <strong>will be deleted</strong>. If you wish to do this, <%
%>type in the name and password of the empire you will be inheriting from:</strong></td></tr></table><%

%><table width="50%"><tr></tr><%

%><tr><td><strong>Parent empire's name:</strong></td><td><input type="text" name="ParentEmpireName" size="20"<%

%> maxlength="<% Write (MAX_EMPIRE_NAME_LENGTH); %>"<%

const char* pszParentName;

if (bRepost && 
	(pHttpForm = m_pHttpRequest->GetForm ("ParentEmpireName")) != NULL &&
	(pszParentName = pHttpForm->GetValue()) != NULL &&
	VerifyEmpireName (pszParentName, false) == OK) {
	%> value="<% Write (pszParentName); %>"<%;
}
%>></td></tr><%

%><tr><td><strong>Parent empire's password:</strong></td><%
%><td><input type="password" name="ParentPassword" size="20" maxlength="<% 
	Write (MAX_PASSWORD_LENGTH); %>"></td></tr></table><%

CloseSystemPage();

return OK; %>