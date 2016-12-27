
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include <stdio.h>

#include "Osal/Algorithm.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the ProfileEditor page
int HtmlRenderer::Render_ProfileEditor() {

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

	enum AutoLogonVars {
		NO_AUTOLOGON = -1,
		MAYBE_AUTOLOGON = -2
	};

	INITIALIZE_EMPIRE

	IHttpForm* pHttpForm;

	int i, iErrCode, iProfileEditorPage = 0, iInfoThemeKey = NO_KEY, iAlienSelect = 0, 
		iNewButtonKey = m_iButtonKey;
	const char* pszGraphicsPath = NULL;

	int iAutoLogonSelected = MAYBE_AUTOLOGON;

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

		if ((pHttpForm = m_pHttpRequest->GetForm ("ProfileEditorPage")) == NULL) {
			goto Redirection;
		}
		int iProfileEditorPageSubmit = pHttpForm->GetIntValue();

		if (WasButtonPressed (BID_CANCEL)) {

			if (iProfileEditorPageSubmit == 7) {
				bRedirectTest = false;
				iProfileEditorPage = 3;
			}

		} else {

			switch (iProfileEditorPageSubmit) {
			case 0:
				{

				Variant vVerify;
				const char* pszNewValue, * pszVerify;
				int iNewValue, iVerify, iValue, iEmpireOptions;

				bool bIPAddress, bSessionId, bValue, bValue2, bNewValue, bNewValue2;

				EmpireCheck (g_pGameEngine->GetEmpireOptions (m_iEmpireKey, &iEmpireOptions));

				// Handle empire name recasing
				if ((pHttpForm = m_pHttpRequest->GetForm ("RecasedEmpireName")) == NULL) {
					goto Redirection;
				}
				pszNewValue = pHttpForm->GetValue();

				if (String::StrCmp (pszNewValue, m_vEmpireName.GetCharPtr()) != 0) {
					if (String::StriCmp (pszNewValue, m_vEmpireName.GetCharPtr()) == 0) {

						EmpireCheck (g_pGameEngine->SetEmpireName (m_iEmpireKey, pszNewValue));
						AddMessage ("Your empire name was recased");
						m_vEmpireName = pszNewValue;

					} else {
						AddMessage ("You can only recase your empire's name, not rename it");
					}
				}

				// Handle password change
				if (m_iEmpireKey != GUEST_KEY) {

					if ((pHttpForm = m_pHttpRequest->GetForm ("NewPassword")) == NULL) {
						goto Redirection;
					}
					pszNewValue = pHttpForm->GetValue();

					if ((pHttpForm = m_pHttpRequest->GetForm ("VerifyPassword")) == NULL) {
						goto Redirection;
					}
					pszVerify = pHttpForm->GetValue();

					if (String::StrCmp (pszVerify, m_vPassword.GetCharPtr()) != 0 && 
						String::StrCmp (pszNewValue, pszVerify) == 0) {

						if (VerifyPassword (pszNewValue) == OK) {

							iErrCode = g_pGameEngine->ChangeEmpirePassword (m_iEmpireKey, pszNewValue);
							if (iErrCode == ERROR_CANNOT_MODIFY_GUEST) {
								AddMessage (GUEST_NAME "'s password can only be changed by an administrator");
							} else {

								AddMessage ("Your password was changed");
								m_vPassword = pszNewValue;

								ICookie* pCookie = m_pHttpRequest->GetCookie (AUTOLOGON_EMPIREKEY_COOKIE);

								if (pCookie != NULL && pCookie->GetValue() != NULL) {

									if (pCookie->GetIntValue() == m_iEmpireKey) {

										char pszText [128];
										m_pHttpResponse->CreateCookie (
											AUTOLOGON_PASSWORD_COOKIE,
											String::I64toA (GetPasswordHash(), pszText, 10),
											ONE_YEAR_IN_SECONDS,
											NULL
											);
									}
								}
							}
						}
					}
				}

				// Handle RealName change
				if ((pHttpForm = m_pHttpRequest->GetForm ("RealName")) == NULL) {
					goto Redirection;
				}
				pszNewValue = pHttpForm->GetValue();

				if (pszNewValue == NULL) {
					pszNewValue = "";
				}

				EmpireCheck (g_pGameEngine->GetEmpireRealName (m_iEmpireKey, &vVerify));

				if (strcmp (pszNewValue, vVerify.GetCharPtr()) != 0) {
					if (g_pGameEngine->SetEmpireRealName (m_iEmpireKey, pszNewValue) == OK) {
						AddMessage ("Your real name was changed");
					} else {
						AddMessage ("Your real name could not be changed");
					}
				}

				// Handle Email change
				if ((pHttpForm = m_pHttpRequest->GetForm ("Email")) == NULL) {
					goto Redirection;
				}
				pszNewValue = pHttpForm->GetValue();

				if (pszNewValue == NULL) {
					pszNewValue = "";
				}

				EmpireCheck (g_pGameEngine->GetEmpireEmail (m_iEmpireKey, &vVerify));

				if (strcmp (pszNewValue, vVerify.GetCharPtr()) != 0) {
					if (g_pGameEngine->SetEmpireEmail (m_iEmpireKey, pszNewValue) == OK) {
						AddMessage ("Your email address was changed");
					} else {
						AddMessage ("Your email address could not be changed");
					}
				}

				// Handle WebPage change
				if ((pHttpForm = m_pHttpRequest->GetForm ("WebPage")) == NULL) {
					goto Redirection;
				}
				pszNewValue = pHttpForm->GetValue();

				if (pszNewValue == NULL) {
					pszNewValue = "";
				}

				EmpireCheck (g_pGameEngine->GetEmpireWebPage (m_iEmpireKey, &vVerify));

				if (strcmp (pszNewValue, vVerify.GetCharPtr()) != 0) {
					if (g_pGameEngine->SetEmpireWebPage (m_iEmpireKey, pszNewValue) == OK) {
						AddMessage ("Your web page URL was changed");
					} else {
						AddMessage ("Your web page URL could not be changed");
					}
				}

				// Handle MaxNumSavedMessages change
				if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumSavedMessages")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				EmpireCheck (g_pGameEngine->GetEmpireMaxNumSavedSystemMessages (m_iEmpireKey, &iVerify));

				if (iNewValue != iVerify) {

					int iLimit;
					EmpireCheck (g_pGameEngine->GetSystemLimitOnSavedSystemMessages (&iLimit));

					if (iNewValue > iLimit) {
						AddMessage ("Illegal maximum number of saved system messages");
					} else {
						EmpireCheck (g_pGameEngine->SetEmpireMaxNumSavedSystemMessages (m_iEmpireKey, iNewValue));
						AddMessage ("Your maximum number of saved messages was changed");
					}
				}

				// Handle MaxNumShipsBuiltAtOnce change
				if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumShipsBuiltAtOnce")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				EmpireCheck (g_pGameEngine->GetEmpireMaxNumShipsBuiltAtOnce (m_iEmpireKey, &iVerify));

				if (iNewValue != iVerify) {

					if (iNewValue > 100) {
						AddMessage ("Illegal maximum number of ships built at once");
					} else {
						EmpireCheck (g_pGameEngine->SetEmpireMaxNumShipsBuiltAtOnce (m_iEmpireKey, iNewValue));
						AddMessage ("Your maximum number of ships built at once was updated");
					}
				}

				// Handle DefaultBuilderPlanet
				if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultBuilderPlanet")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				EmpireCheck (g_pGameEngine->GetEmpireDefaultBuilderPlanet (m_iEmpireKey, &iVerify));

				if (iNewValue != iVerify) {

					iErrCode = g_pGameEngine->SetEmpireDefaultBuilderPlanet (m_iEmpireKey, iNewValue);
					if (iErrCode == OK) {
						AddMessage ("Your default builder planet was updated");
					} else {
						AddMessage ("Your default builder planet could not be updated; the error was ");
						AppendMessage (iErrCode);
					}
				}

				// Handle IndependentGifts
				if ((pHttpForm = m_pHttpRequest->GetForm ("IndependentGifts")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

				if ((iNewValue != 0) != bValue) {

					iErrCode = g_pGameEngine->SetEmpireOption (m_iEmpireKey, REJECT_INDEPENDENT_SHIP_GIFTS, !bValue);
					if (iErrCode == OK) {
						AddMessage ("Your independent gift option was updated");
					} else {
						AddMessage ("Your independent gift option was updated; the error was ");
						AppendMessage (iErrCode);
					}
				}

				// Handle Confirm
				if ((pHttpForm = m_pHttpRequest->GetForm ("Confirm")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bSessionId = (iEmpireOptions & CONFIRM_ON_ENTER_OR_QUIT_GAME) != 0;
				if (bSessionId != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, CONFIRM_ON_ENTER_OR_QUIT_GAME, !bSessionId));

					if (bSessionId) {
						AddMessage ("You will no longer be prompted to confirm game entry");
					} else {
						AddMessage ("You will now be prompted to confirm game entry");
					}
				}

				// Handle autologon
				if ((pHttpForm = m_pHttpRequest->GetForm ("AutoLogonSel")) == NULL) {
					goto Redirection;
				}
				iVerify = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("AutoLogon")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iVerify != iNewValue) {

					if (iNewValue == NO_AUTOLOGON) {

						iAutoLogonSelected = NO_AUTOLOGON;
						AddMessage ("Autologon is now off");

						m_pHttpResponse->DeleteCookie (AUTOLOGON_EMPIREKEY_COOKIE, NULL);
						m_pHttpResponse->DeleteCookie (AUTOLOGON_PASSWORD_COOKIE, NULL);

					} else {

						if (iNewValue != m_iEmpireKey) {
							AddMessage ("Invalid autologon submission");
							goto Quote;
						}

						// Set cookies (expire in a year)
						char pszText [128];
						m_pHttpResponse->CreateCookie (
							AUTOLOGON_EMPIREKEY_COOKIE,
							itoa (iNewValue, pszText, 10),
							ONE_YEAR_IN_SECONDS,
							NULL
							);

						uint64 ui64Hash = GetPasswordHash();

						m_pHttpResponse->CreateCookie (
							AUTOLOGON_PASSWORD_COOKIE,
							String::I64toA (ui64Hash, pszText, 10),
							ONE_YEAR_IN_SECONDS,
							NULL
							);

						AddMessage ("Autologon is now on for ");
						AppendMessage (m_vEmpireName.GetCharPtr());

						iAutoLogonSelected = m_iEmpireKey;
					}
				}

	Quote:
				// Handle quote change
				const char* pszString;

				if ((pHttpForm = m_pHttpRequest->GetForm ("Quote")) == NULL) {
					goto Redirection;
				}
				pszString = pHttpForm->GetValue();

				if (pszString == NULL) {
					iErrCode = g_pGameEngine->UpdateEmpireQuote (m_iEmpireKey, "");
				} else {

					if (strlen (pszString) > MAX_QUOTE_LENGTH) {

						AddMessage ("Your quote will be truncated to ");
						AppendMessage (MAX_QUOTE_LENGTH);
						AppendMessage (" characters");

						char* pszTruncate = (char*) StackAlloc (MAX_QUOTE_LENGTH + 1);

						strncpy (pszTruncate, pszString, MAX_QUOTE_LENGTH);
						pszTruncate [MAX_QUOTE_LENGTH] = '\0';

						iErrCode = g_pGameEngine->UpdateEmpireQuote (m_iEmpireKey, pszTruncate);

					} else {

						iErrCode = g_pGameEngine->UpdateEmpireQuote (m_iEmpireKey, pszString);
					}
				}

				switch (iErrCode) {
				case OK:
					AddMessage ("Your quote was updated");
					break;
				case WARNING:
					break;
				case ERROR_STRING_IS_TOO_LONG:
					AddMessage ("Your quote is too long");
					break;
				default:
					AddMessage ("Your quote could not be updated");
					Assert (false);
					break;
				}

				// Handle victory sneer change
				if ((pHttpForm = m_pHttpRequest->GetForm ("VictorySneer")) == NULL) {
					goto Redirection;
				}
				pszString = pHttpForm->GetValue();

				if (pszString == NULL) {
					iErrCode = g_pGameEngine->UpdateEmpireVictorySneer (m_iEmpireKey, "");
				} else {

					if (strlen (pszString) > MAX_VICTORY_SNEER_LENGTH) {

						AddMessage ("Your victory sneer will be truncated to ");
						AppendMessage (MAX_QUOTE_LENGTH);
						AppendMessage (" characters");

						char* pszTruncate = (char*) StackAlloc (MAX_VICTORY_SNEER_LENGTH + 1);

						strncpy (pszTruncate, pszString, MAX_VICTORY_SNEER_LENGTH);
						pszTruncate [MAX_VICTORY_SNEER_LENGTH] = '\0';

						iErrCode = g_pGameEngine->UpdateEmpireVictorySneer (m_iEmpireKey, pszTruncate);

					} else {

						iErrCode = g_pGameEngine->UpdateEmpireVictorySneer (m_iEmpireKey, pszString);
					}
				}

				switch (iErrCode) {
				case OK:
					AddMessage ("Your victory sneer was updated");
					break;
				case WARNING:
					break;
				case ERROR_STRING_IS_TOO_LONG:
					AddMessage ("Your victory sneer is too long");
					break;
				default:
					AddMessage ("Your victory sneer could not be updated");
					Assert (false);
					break;
				}

				// Handle repeated buttons
				if ((pHttpForm = m_pHttpRequest->GetForm ("RepeatedButtons")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bNewValue = (iNewValue & SYSTEM_REPEATED_BUTTONS) != 0;
				bNewValue2 = (iNewValue & GAME_REPEATED_BUTTONS) != 0;

				bValue = (iEmpireOptions & SYSTEM_REPEATED_BUTTONS) != 0;
				bValue2 = (iEmpireOptions & GAME_REPEATED_BUTTONS) != 0;

				if ((bNewValue != bValue) || (bNewValue2 != bValue2)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SYSTEM_REPEATED_BUTTONS, bNewValue));
					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, GAME_REPEATED_BUTTONS, bNewValue2));

					if (bNewValue || bNewValue2) {
						if (bNewValue) {
							AddMessage ("System");
						}
						if (bNewValue2) {
							if (bNewValue) {
								AppendMessage (" and game");
							} else {
								AddMessage ("Game");
							}
						}
						AppendMessage (" command buttons are now repeated");

					} else {

						AppendMessage ("Command buttons are no longer repeated");
					}

					m_bRepeatedButtons = bNewValue;	// Profile Editor is a system page
				}

				// Handle server time display
				if ((pHttpForm = m_pHttpRequest->GetForm ("TimeDisplay")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bNewValue = (iNewValue & SYSTEM_DISPLAY_TIME) != 0;
				bNewValue2 = (iNewValue & GAME_DISPLAY_TIME) != 0;

				bValue = (iEmpireOptions & SYSTEM_DISPLAY_TIME) != 0;
				bValue2 = (iEmpireOptions & GAME_DISPLAY_TIME) != 0;

				if ((bNewValue != bValue) || (bNewValue2 != bValue2)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SYSTEM_DISPLAY_TIME, bNewValue));
					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, GAME_DISPLAY_TIME, bNewValue2));

					if (bNewValue || bNewValue2) {
						AddMessage ("Server time display is now enabled for");
						if (bNewValue) {
							AppendMessage (" system");
						}
						if (bNewValue2) {
							if (bNewValue) {
								AppendMessage (" and");
							}
							AppendMessage (" game");
						}
						AppendMessage (" pages");

					} else {

						AppendMessage ("Server time display is disabled for all pages");
					}

					m_bTimeDisplay = bNewValue;	// Profile Editor is a system page
				}

				// Handle AutoRefresh
				if ((pHttpForm = m_pHttpRequest->GetForm ("AutoRefresh")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & AUTO_REFRESH) != 0;
				if (bValue != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, AUTO_REFRESH, !bValue));
					AddMessage ("Refresh on update countdown is ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" on by default");
				}

				// Handle Countdown
				if ((pHttpForm = m_pHttpRequest->GetForm ("Countdown")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & COUNTDOWN) != 0;
				if (bValue != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, COUNTDOWN, !bValue));
					AddMessage ("Visual update countdown ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" on by default");
				}

				// Handle map coloring
				if ((pHttpForm = m_pHttpRequest->GetForm ("MapColoring")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & MAP_COLORING) != 0;
				if (bValue != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, MAP_COLORING, !bValue));
					AddMessage ("Map coloring by diplomatic status is ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" on by default");
				}

				// Handle ship map coloring
				if ((pHttpForm = m_pHttpRequest->GetForm ("ShipMapColoring")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & SHIP_MAP_COLORING) != 0;
				if (bValue != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHIP_MAP_COLORING, !bValue));
					AddMessage ("Ship coloring by diplomatic status is ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" on by default");
				}

				// Handle ship map highlighting
				if ((pHttpForm = m_pHttpRequest->GetForm ("ShipHighlighting")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & SHIP_MAP_HIGHLIGHTING) != 0;
				if (bValue != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHIP_MAP_HIGHLIGHTING, !bValue));
					AddMessage ("Ship highlighting on the map screen is ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" on by default");
				}

				// Handle sensitive maps
				if ((pHttpForm = m_pHttpRequest->GetForm ("SensitiveMaps")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & SENSITIVE_MAPS) != 0;
				if (bValue != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SENSITIVE_MAPS, !bValue));
					AddMessage ("Sensitive maps are ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" on by default");
				}

				// Handle partial maps
				if ((pHttpForm = m_pHttpRequest->GetForm ("PartialMaps")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & PARTIAL_MAPS) != 0;
				if (bValue != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, PARTIAL_MAPS, !bValue));
					AddMessage ("Partial maps are ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" on by default");
				}

				// LocalMaps
				if ((pHttpForm = m_pHttpRequest->GetForm ("LocalMaps")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;
				if (bValue != (iNewValue != 0)) {
					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, LOCAL_MAPS_IN_UPCLOSE_VIEWS, !bValue));
					AddMessage ("Local maps will ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" be displayed by default in up-close map views");
				}

				// MessageTarget
				if ((pHttpForm = m_pHttpRequest->GetForm ("MessageTarget")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				EmpireCheck (g_pGameEngine->GetEmpireDefaultMessageTarget (m_iEmpireKey, &iValue));
				if (iValue != iNewValue) {
					iErrCode = g_pGameEngine->SetEmpireDefaultMessageTarget (m_iEmpireKey, iNewValue);
					if (iErrCode == OK) {
						AddMessage ("The default message target was updated");
					} else {
						AddMessage ("The default message target could not be updated; the error was ");
						AppendMessage (iErrCode);
					}
				}

				// UpCloseShips
				if ((pHttpForm = m_pHttpRequest->GetForm ("UpCloseShips")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & SHIPS_ON_MAP_SCREEN) != 0;
				if (bValue != ((iNewValue & SHIPS_ON_MAP_SCREEN)!= 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHIPS_ON_MAP_SCREEN, !bValue));
					AddMessage ("Ship menus will ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" be displayed by default in map page planet views");
				}

				bValue = (iEmpireOptions & SHIPS_ON_PLANETS_SCREEN) != 0;
				if (bValue != ((iNewValue & SHIPS_ON_PLANETS_SCREEN)!= 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHIPS_ON_PLANETS_SCREEN, !bValue));
					AddMessage ("Ship menus will ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" be displayed by default on the planets page");
				}

				// Handle fixed backgrounds
				if ((pHttpForm = m_pHttpRequest->GetForm ("FixedBg")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bValue = (iEmpireOptions & FIXED_BACKGROUNDS) != 0;
				if (bValue != (iNewValue != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, FIXED_BACKGROUNDS, !bValue));
					AddMessage ("Fixed backgrounds are ");
					if (iNewValue == 0) {
						AppendMessage ("no longer");
					} else {
						AppendMessage ("now");
					}
					AppendMessage (" on");
					m_bFixedBackgrounds = !bValue;
				}

				// Password hashing
				if ((pHttpForm = m_pHttpRequest->GetForm ("Hashing")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bIPAddress = (iEmpireOptions & IP_ADDRESS_PASSWORD_HASHING) != 0;
				bSessionId = (iEmpireOptions & SESSION_ID_PASSWORD_HASHING) != 0;

				if (bIPAddress != ((iNewValue & IP_ADDRESS_PASSWORD_HASHING) != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, IP_ADDRESS_PASSWORD_HASHING, !bIPAddress));
					AddMessage ("Game screen password hashing with IP address is now ");
					AppendMessage (bIPAddress ? "off" : "on");
					m_bHashPasswordWithIPAddress = !bIPAddress;
				}

				if (bSessionId != ((iNewValue & SESSION_ID_PASSWORD_HASHING) != 0)) {

					EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SESSION_ID_PASSWORD_HASHING, !bSessionId));
					AddMessage ("Game screen password hashing with Session Id is now ");
					AppendMessage (bIPAddress ? "off" : "on");
					m_bHashPasswordWithSessionId = !bSessionId;
				}

				// AdvancedSearch
				if (m_iPrivilege >= APPRENTICE) {

					if ((pHttpForm = m_pHttpRequest->GetForm ("AdvancedSearch")) != NULL) {

						iNewValue = pHttpForm->GetIntValue();

						bValue = (iEmpireOptions & SHOW_ADVANCED_SEARCH_INTERFACE) != 0;
						if (bValue != (iNewValue != 0)) {

							EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHOW_ADVANCED_SEARCH_INTERFACE, !bValue));
							AddMessage ("The advanced search interface will ");
							if (iNewValue == 0) {
								AppendMessage ("no longer");
							} else {
								AppendMessage ("now");
							}
							AppendMessage (" be displayed");
						}
					}
				}

				// DisplayFatalUpdates
				if ((pHttpForm = m_pHttpRequest->GetForm ("DisplayFatalUpdates")) != NULL) {

					iNewValue = pHttpForm->GetIntValue();

					bValue = (iEmpireOptions & DISPLAY_FATAL_UPDATE_MESSAGES) != 0;
					if (bValue != (iNewValue != 0)) {

						EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, DISPLAY_FATAL_UPDATE_MESSAGES, !bValue));
						AddMessage ("Update messages on empire obliteration will ");
						if (iNewValue == 0) {
							AppendMessage ("no longer");
						} else {
							AppendMessage ("now");
						}
						AppendMessage (" be displayed");
					}
				}

				// Handle ship name changes
				Variant vOldShipName;
				int iUpdate = 0;

				char pszForm[128];

				ENUMERATE_SHIP_TYPES(i) {

					sprintf (pszForm, "ShipName%i", i);
					if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
						goto Redirection;
					}
					pszNewValue = pHttpForm->GetValue();
					if (pszNewValue == NULL) {
						pszNewValue = "";
					}

					if (!ShipOrFleetNameFilter (pszNewValue)) {

						AddMessage ("The new default name for ");
						AppendMessage (SHIP_TYPE_STRING[i]);
						AppendMessage (" is illegal");

					} else {

						if (strlen (pszNewValue) > MAX_SHIP_NAME_LENGTH) {

							AddMessage ("The new default name for ");
							AppendMessage (SHIP_TYPE_STRING[i]);
							AppendMessage (" is too long");

						} else {

							EmpireCheck (g_pGameEngine->GetDefaultEmpireShipName (m_iEmpireKey, i, &vOldShipName));

							if (strcmp (vOldShipName.GetCharPtr(), pszNewValue) != 0 &&
								g_pGameEngine->SetDefaultEmpireShipName (m_iEmpireKey, i, pszNewValue) == OK) {
								iUpdate ++;
							}
						}
					}
				}

				if (iUpdate > 0) {
					if (iUpdate == 1) {
						AddMessage ("A default ship name was changed");
					} else {
						AddMessage ("The default ship names were changed");
					}
				}

				// Handle alien selection request
				if (WasButtonPressed (BID_CHOOSEALIEN)) {
					bRedirectTest = false;
					iProfileEditorPage = 1;

					if ((pHttpForm = m_pHttpRequest->GetForm ("AlienSelect")) == NULL) {
						goto Redirection;
					} else {
						iAlienSelect = pHttpForm->GetIntValue();
					}
					break;
				}

				// Handle alien selection request
				if (WasButtonPressed (BID_VIEWMESSAGES)) {
					bRedirectTest = false;
					iProfileEditorPage = 2;
					break;
				}

				// Handle theme selection
				if (WasButtonPressed (BID_CHOOSETHEME)) {

					bRedirectTest = false;
					if ((pHttpForm = m_pHttpRequest->GetForm ("GraphicalTheme")) == NULL) {
						goto Redirection;
					}
					iNewValue = pHttpForm->GetIntValue();

					int iOldThemeKey;
					switch (iNewValue) {

					case NULL_THEME:

						EmpireCheck (g_pGameEngine->GetEmpireThemeKey (m_iEmpireKey, &iOldThemeKey));

						if (iOldThemeKey != NULL_THEME) {

							EmpireCheck (g_pGameEngine->SetEmpireThemeKey (m_iEmpireKey, NULL_THEME));
							EmpireCheck (GetUIData (NULL_THEME));

							AddMessage ("You have selected the Null Theme");
						}

						break;

					case INDIVIDUAL_ELEMENTS:

						iProfileEditorPage = 3;
						break;

					case ALTERNATIVE_PATH:
						iProfileEditorPage = 4;
						break;

					default:
						{

						bool bExist;
						iErrCode = g_pGameEngine->DoesThemeExist (iNewValue, &bExist);
						if (iErrCode != OK || !bExist) {
							AddMessage ("That theme doesn't exist");
						} else {

							EmpireCheck (g_pGameEngine->GetEmpireThemeKey (m_iEmpireKey, &iOldThemeKey));

							if (iOldThemeKey != iNewValue) {

								EmpireCheck (g_pGameEngine->SetEmpireThemeKey (m_iEmpireKey, iNewValue));
								EmpireCheck (GetUIData (iNewValue));

								AddMessage ("You have selected a new theme");
							}
						}

						}
						break;
					}
				}

				// Handle delete empire request
				if (WasButtonPressed (BID_DELETEEMPIRE)) {

					if (m_iEmpireKey == ROOT_KEY) {
						AddMessage (ROOT_NAME " cannot commit suicide");
						break;
					}

					else if (m_iEmpireKey == GUEST_KEY) {
						AddMessage (GUEST_NAME " cannot commit suicide");
						break;
					}

					else {
						bRedirectTest = false;
						iProfileEditorPage = 5;
						break;
					}
				}

				// Handle undelete empire request
				if (WasButtonPressed (BID_UNDELETEEMPIRE)) {

					switch (g_pGameEngine->UndeleteEmpire (m_iEmpireKey)) {

					case ERROR_CANNOT_UNDELETE_EMPIRE:

						AddMessage ("Your empire cannot be undeleted");
						break;

					case OK:

						AddMessage ("Your empire is no longer marked for deletion"); 
						m_bHalted = false;
						break;

					default:

						AddMessage ("An unexpected error occurred");
					}

					break;
				}

				// Handle blank empire stats request
				if (WasButtonPressed (BID_BLANKEMPIRESTATISTICS)) {

					if (m_iEmpireKey == ROOT_KEY) {
						AddMessage (ROOT_NAME "'s statistics cannot be blanked");
						break;
					} else if (m_iEmpireKey == GUEST_KEY) {
						AddMessage (GUEST_NAME "'s statistics cannot be blanked");
						break;
					} else {
						bRedirectTest = false;
						iProfileEditorPage = 6;
						break;
					}
				}

				}
				break;

			case 1:
				{

				if ((pHttpForm = m_pHttpRequest->GetForm ("WhichAlien")) == NULL) {
					goto Redirection;
				}
				if (pHttpForm->GetIntValue() == 1) {

					if (!WasButtonPressed (BID_CHOOSE)) {
						iProfileEditorPage = 0;
					} else {

						// Icon uploads
						if ((pHttpForm = m_pHttpRequest->GetForm ("IconFile")) == NULL) {
							goto Redirection;
						}

						const char* pszFileName = pHttpForm->GetValue();
						if (pszFileName == NULL) {

							AddMessage ("You didn't upload a file");
							iProfileEditorPage = 0;
							bRedirectTest = false;

						} else {

							if (VerifyGIF (pszFileName)) {

								// The gif was OK, so get a unique key and copy it to its destination
								if (CopyUploadedAlien (pszFileName, m_iEmpireKey) != OK) {
									AddMessage ("The file was uploaded, but could not be copied. "\
										"Contact the administrator");
								} else {

									// Set the empire's alien key to UPLOADED_ICON
									EmpireCheck (g_pGameEngine->SetEmpireAlienKey (m_iEmpireKey, UPLOADED_ICON));
									m_iAlienKey = UPLOADED_ICON;
									AddMessage ("Your icon was uploaded successfully");
								}
							}
						}
					}

				} else {

					const char* pszStart;
					if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Alien")) != NULL && 
						(pszStart = pHttpForm->GetName()) != NULL &&
						sscanf (pszStart, "Alien%d", &i) == 1) {

						int iAlienKey;
						Check (g_pGameEngine->GetEmpireAlienKey (m_iEmpireKey, &iAlienKey));

						if (i != iAlienKey) {

							if (g_pGameEngine->SetEmpireAlienKey (m_iEmpireKey, i) == OK) {
								m_iAlienKey = i;
								AddMessage ("Your alien icon was updated");
							} else {
								AddMessage ("The alien icon no longer exists");
							}
							iProfileEditorPage = 0;
							bRedirectTest = false;
						} else {
							AddMessage ("That was the same icon");
						}
					}
				}

				}
				break;

			case 2:
				{

				int iNumTestMessages, iMessageKey, iDeletedMessages = 0;

				// Get number of messages
				if ((pHttpForm = m_pHttpRequest->GetForm ("NumSavedSystemMessages")) == NULL) {
					goto Redirection;
				}
				iNumTestMessages = pHttpForm->GetIntValue();

				// Check for delete all
				char pszForm [128];

				if (WasButtonPressed (BID_ALL)) {

					bRedirectTest = false;

					for (i = 0; i < iNumTestMessages; i ++) {

						// Get message key
						sprintf (pszForm, "MsgKey%i", i);
						if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
							goto Redirection;
						}
						iMessageKey = pHttpForm->GetIntValue();

						// Delete message
						if (g_pGameEngine->DeleteSystemMessage (m_iEmpireKey, iMessageKey) == OK) {
							iDeletedMessages ++;
						}
					}

				} else {

					// Check for delete selection
					if (WasButtonPressed (BID_SELECTION)) {

						bRedirectTest = false;

						for (i = 0; i < iNumTestMessages; i ++) {

							// Get selected status of message's delete checkbox
							sprintf (pszForm, "DelChBx%i", i);
							if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) != NULL) {

								// Get message key
								sprintf (pszForm, "MsgKey%i", i);
								if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
									goto Redirection;
								}
								iMessageKey = pHttpForm->GetIntValue();

								// Delete message
								if (g_pGameEngine->DeleteSystemMessage (m_iEmpireKey, iMessageKey) == OK) {
									iDeletedMessages ++;
								}
							}
						}

					} else {

						// Check for delete system messages
						Variant vSender;
						char pszForm [128];
						if (WasButtonPressed (BID_SYSTEM)) {

							bRedirectTest = false;

							for (i = 0; i < iNumTestMessages; i ++) {

								// Get message key
								sprintf (pszForm, "MsgKey%i", i);
								if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
									goto Redirection;
								}
								iMessageKey = pHttpForm->GetIntValue();

								if (g_pGameEngine->GetSystemMessageSender (m_iEmpireKey, iMessageKey, &vSender) == OK &&
									String::StrCmp (vSender.GetCharPtr(), SYSTEM_MESSAGE_SENDER) == 0 &&
									g_pGameEngine->DeleteSystemMessage (m_iEmpireKey, iMessageKey) == OK) {
									iDeletedMessages ++;
								}
							}

						} else {

							// Check for delete empire message
							if (WasButtonPressed (BID_DELETEEMPIRE)) {

								bRedirectTest = false;

								// Get target empire
								if ((pHttpForm = m_pHttpRequest->GetForm ("SelectedEmpire")) == NULL) {
									goto Redirection;
								}
								const char* pszSrcEmpire = pHttpForm->GetValue();

								for (i = 0; i < iNumTestMessages; i ++) {

									sprintf (pszForm, "MsgKey%i", i);
									if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
										goto Redirection;
									}
									iMessageKey = pHttpForm->GetIntValue();

									if (g_pGameEngine->GetSystemMessageSender (m_iEmpireKey, iMessageKey, &vSender) == OK &&
										String::StrCmp (vSender.GetCharPtr(), pszSrcEmpire) == 0 &&
										g_pGameEngine->DeleteSystemMessage (m_iEmpireKey, iMessageKey) == OK) {
										iDeletedMessages ++;
									}
								}
							}
						}
					}
				}

				if (iDeletedMessages > 0) {
					AddMessage (iDeletedMessages);
					AppendMessage (" system message");
					AppendMessage (iDeletedMessages == 1 ? " was deleted" : "s were deleted");
				}

				}
				break;

			case 3:
				{

				int iNewValue, iLivePlanetKey, iDeadPlanetKey, iColorKey, iThemeKey;

				// Handle graphical theme updates
				bool bUpdate = false, bColorError = false;

				EmpireCheck (g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey));
				EmpireCheck (g_pGameEngine->GetEmpireColorKey (m_iEmpireKey, &iColorKey));

				// Background
				if ((pHttpForm = m_pHttpRequest->GetForm ("Background")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iNewValue != m_iBackgroundKey) {
					EmpireCheck (g_pGameEngine->SetEmpireBackgroundKey (m_iEmpireKey, iNewValue));
					m_iBackgroundKey = iNewValue;
					bUpdate = true;
				}

				// LivePlanet
				if ((pHttpForm = m_pHttpRequest->GetForm ("LivePlanet")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iNewValue != iLivePlanetKey) {
					EmpireCheck (g_pGameEngine->SetEmpireLivePlanetKey (m_iEmpireKey, iNewValue));
					bUpdate = true;
				}

				// DeadPlanet
				if ((pHttpForm = m_pHttpRequest->GetForm ("DeadPlanet")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iNewValue != iDeadPlanetKey) {
					EmpireCheck (g_pGameEngine->SetEmpireDeadPlanetKey (m_iEmpireKey, iNewValue));
					bUpdate = true;
				}

				// Button
				if ((pHttpForm = m_pHttpRequest->GetForm ("Button")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iNewValue != m_iButtonKey) {
					EmpireCheck (g_pGameEngine->SetEmpireButtonKey (m_iEmpireKey, iNewValue));
					iNewButtonKey = iNewValue;
					bUpdate = true;
				}

				// Separator
				if ((pHttpForm = m_pHttpRequest->GetForm ("Separator")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iNewValue != m_iSeparatorKey) {
					EmpireCheck (g_pGameEngine->SetEmpireSeparatorKey (m_iEmpireKey, iNewValue));
					bUpdate = true;
				}

				// Get horz, vert keys
				int iHorzKey, iVertKey;
				EmpireCheck (g_pGameEngine->GetEmpireHorzKey (m_iEmpireKey, &iHorzKey));
				EmpireCheck (g_pGameEngine->GetEmpireVertKey (m_iEmpireKey, &iVertKey));

				// Horz
				if ((pHttpForm = m_pHttpRequest->GetForm ("Horz")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iNewValue != iHorzKey) {
					EmpireCheck (g_pGameEngine->SetEmpireHorzKey (m_iEmpireKey, iNewValue));
					bUpdate = true;
				}

				// Vert
				if ((pHttpForm = m_pHttpRequest->GetForm ("Vert")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iNewValue != iVertKey) {
					EmpireCheck (g_pGameEngine->SetEmpireVertKey (m_iEmpireKey, iNewValue));
					bUpdate = true;
				}

				// Color
				if ((pHttpForm = m_pHttpRequest->GetForm ("Color")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if (iNewValue == CUSTOM_COLORS) {

					const char* pszNewValue;

					// Update text color
					if ((pHttpForm = m_pHttpRequest->GetForm ("CustomTextColor")) == NULL) {
						goto Redirection;
					}
					pszNewValue = pHttpForm->GetValue();

					if (IsColor (pszNewValue)) {
						EmpireCheck (g_pGameEngine->SetEmpireCustomTextColor (m_iEmpireKey, pszNewValue));
						m_vTableColor = pszNewValue;
					} else {
						AddMessage ("You must submit a valid text color");
						bColorError = true;
					}

					// Update good color
					if ((pHttpForm = m_pHttpRequest->GetForm ("CustomGoodColor")) == NULL) {
						goto Redirection;
					}
					pszNewValue = pHttpForm->GetValue();

					if (IsColor (pszNewValue)) {
						EmpireCheck (g_pGameEngine->SetEmpireCustomGoodColor (m_iEmpireKey, pszNewValue));
						m_vGoodColor = pszNewValue;
					} else {
						AddMessage ("You must submit a valid good color");
						bColorError = true;
					}

					// Update bad color
					if ((pHttpForm = m_pHttpRequest->GetForm ("CustomBadColor")) == NULL) {
						goto Redirection;
					}
					pszNewValue = pHttpForm->GetValue();

					if (IsColor (pszNewValue)) {
						EmpireCheck (g_pGameEngine->SetEmpireCustomBadColor (m_iEmpireKey, pszNewValue));
						m_vBadColor = pszNewValue;
					} else {
						AddMessage ("You must submit a valid bad color");
						bColorError = true;
					}

					// Update private message color
					if ((pHttpForm = m_pHttpRequest->GetForm ("CustomMessageColor")) == NULL) {
						goto Redirection;
					}
					pszNewValue = pHttpForm->GetValue();

					if (IsColor (pszNewValue)) {
						EmpireCheck (g_pGameEngine->SetEmpireCustomPrivateMessageColor (m_iEmpireKey, pszNewValue));
						m_vPrivateMessageColor = pszNewValue;
					} else {
						AddMessage ("You must submit a valid message color");
						bColorError = true;
					}

					// Update broadcast message color
					if ((pHttpForm = m_pHttpRequest->GetForm ("CustomBroadcastColor")) == NULL) {
						goto Redirection;
					}
					pszNewValue = pHttpForm->GetValue();

					if (IsColor (pszNewValue)) {
						EmpireCheck (g_pGameEngine->SetEmpireCustomBroadcastMessageColor (m_iEmpireKey, pszNewValue));
						m_vBroadcastMessageColor = pszNewValue;
					} else {
						AddMessage ("You must submit a valid broadcast color");
						bColorError = true;
					}

					// Update table color
					if ((pHttpForm = m_pHttpRequest->GetForm ("CustomTableColor")) == NULL) {
						goto Redirection;
					}
					pszNewValue = pHttpForm->GetValue();

					if (IsColor (pszNewValue)) {
						EmpireCheck (g_pGameEngine->SetEmpireCustomTableColor (m_iEmpireKey, pszNewValue));
						m_vTableColor = pszNewValue;
					} else {
						AddMessage ("You must submit a valid table color");
						bColorError = true;
					}
				}

				EmpireCheck (g_pGameEngine->GetEmpireThemeKey (m_iEmpireKey, &iThemeKey));

				if (iNewValue != iColorKey) {

					if (!bColorError) {
						EmpireCheck (g_pGameEngine->SetEmpireColorKey (m_iEmpireKey, iNewValue));
						bUpdate = true;
					} else {

						// We need a color key from somewhere - use the previous theme
						if (iThemeKey != INDIVIDUAL_ELEMENTS && iThemeKey != ALTERNATIVE_PATH) {
							EmpireCheck (g_pGameEngine->SetEmpireColorKey (m_iEmpireKey, iThemeKey));
						}

						// No need for an else;  we'll keep using what we had before
					}
				}

				if (!bUpdate) {
					EmpireCheck (g_pGameEngine->GetEmpireThemeKey (m_iEmpireKey, &iNewValue));
					if (iNewValue != INDIVIDUAL_ELEMENTS) {
						AddMessage ("You are now using individual UI elements");
						bUpdate = true;
					}
				}

				EmpireCheck (g_pGameEngine->SetEmpireThemeKey (m_iEmpireKey, INDIVIDUAL_ELEMENTS));
				EmpireCheck (GetUIData (INDIVIDUAL_ELEMENTS));

				const char* pszStart;
				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ThemeInfo")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "ThemeInfo%d", &iInfoThemeKey) == 1) {

					iProfileEditorPage = 7;
					bRedirectTest = false;
					break;
				}

				}
				break;

			case 4:

				if (WasButtonPressed (BID_CHOOSE)) {

					if ((pHttpForm = m_pHttpRequest->GetForm ("GraphicsPath")) == NULL) {
						goto Redirection;
					}

					pszGraphicsPath = pHttpForm->GetValue();
					if (pszGraphicsPath == NULL) {
						AddMessage ("You must submit a valid alternative graphics path");
						iProfileEditorPage = 0;
					} else {
						iProfileEditorPage = 8;
					}

					break;
				}
				break;

			case 8:
				{

				if (WasButtonPressed (BID_CHOOSE)) {

					// Read the path
					if ((pHttpForm = m_pHttpRequest->GetForm ("GraphicsPath")) == NULL) {
						goto Redirection;
					}

					const char* pszPath = pHttpForm->GetValue();

					if (pszPath == NULL) {
						AddMessage ("You must submit a valid alternative graphics path");
						iProfileEditorPage = 0;
					} else {

						size_t stLength = strlen (pszPath);

						if (stLength > MAX_GRAPHICS_ALTERNATIVE_PATH_LENGTH) {
							AddMessage ("Your alternative graphics path is too long");
						} else {
							EmpireCheck (g_pGameEngine->SetEmpireThemeKey (m_iEmpireKey, ALTERNATIVE_PATH));
							EmpireCheck (g_pGameEngine->SetEmpireAlternativeGraphicsPath (m_iEmpireKey, pszPath));
							EmpireCheck (GetUIData (ALTERNATIVE_PATH));
						}
					}
				}

				}

				break;

			case 5:
				{

				iErrCode = g_pGameEngine->DeleteEmpire (m_iEmpireKey);
				switch (iErrCode) {

				case OK:

					AddMessage ("The empire ");
					AppendMessage (m_vEmpireName.GetCharPtr());
					AppendMessage (" was deleted");
					return Redirect (LOGIN);

				case ERROR_EMPIRE_IS_IN_GAMES:

					AddMessage ("Your empire has been marked for deletion");
					m_bHalted = true;
					break;

				case ERROR_EMPIRE_DOES_NOT_EXIST:

					AddMessage ("The empire ");
					AppendMessage (m_vEmpireName.GetCharPtr());
					AppendMessage (" no longer exists");
					return Redirect (LOGIN);

				default:

					Assert (false);
					AddMessage ("An unexpected error occurred: ");
					AppendMessage (iErrCode);
					return Redirect (m_pgPageId);
				}

				}
				break;

			case 6:

				EmpireCheck (g_pGameEngine->BlankEmpireStatistics (m_iEmpireKey));
				AddMessage ("Your empire's statistics have been blanked");
				break;

			case 7:

				// Nothing to do
				break;

			default:
				Assert (false);
			}
		}	// End if not cancel
	} 

	Redirection:
	if (bRedirectTest && !m_bRedirection) {
		PageId pageRedirect;
		if (RedirectOnSubmit (&pageRedirect)) {
			m_iButtonKey = iNewButtonKey;
			return Redirect (pageRedirect);
		}
	}
	iNewButtonKey = m_iButtonKey;

	SYSTEM_OPEN (iProfileEditorPage == 1 && iAlienSelect == 1)

	// Individual page stuff starts here
	switch (iProfileEditorPage) {

	case 0:
		{

		Variant* pvEmpireData;
		int iNumActiveGames, iOptions, iNumSystemMessages, iMaxNumSystemMessages, * piThemeKey, iNumThemes, 
			iValue, j;
		bool bIP, bID, bConfirm;
		size_t stLen;

		String strFilter;

		EmpireCheck (g_pGameEngine->GetEmpireData (m_iEmpireKey, &pvEmpireData, &iNumActiveGames));

		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"0\"><p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"0\"><p>") - 1);
	WriteProfileAlienString (
			pvEmpireData[SystemEmpireData::AlienKey].GetInteger(),
			m_iEmpireKey,
			m_vEmpireName.GetCharPtr(),
			0,
			"ProfileLink",
			"View your profile",
			false,
			false
			);

		
	Write (" <font size=\"+2\">", sizeof (" <font size=\"+2\">") - 1);
	Write (m_vEmpireName.GetCharPtr());
		
	Write ("</font><p><table width=\"80%\"><tr><td align=\"left\">Empire name:</td><td align=\"left\"><input type=\"text\" name=\"RecasedEmpireName\" size=\"", sizeof ("</font><p><table width=\"80%\"><tr><td align=\"left\">Empire name:</td><td align=\"left\"><input type=\"text\" name=\"RecasedEmpireName\" size=\"") - 1);
	stLen = strlen (m_vEmpireName.GetCharPtr());
			Write (stLen); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (stLen); 
			
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (m_vEmpireName.GetCharPtr()); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	if (m_iEmpireKey != GUEST_KEY) {

			
	Write ("<tr><td align=\"left\">Password:</td><td align=\"left\"><input type=\"password\" name=\"NewPassword\" size=\"20\" maxlength=\"", sizeof ("<tr><td align=\"left\">Password:</td><td align=\"left\"><input type=\"password\" name=\"NewPassword\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_PASSWORD_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (m_vPassword.GetCharPtr()); 
	Write ("\"></td></tr><tr><td align=\"left\">Verify password:</td><td align=\"left\"><input type=\"password\" name=\"VerifyPassword\" size=\"20\" maxlength=\"", sizeof ("\"></td></tr><tr><td align=\"left\">Verify password:</td><td align=\"left\"><input type=\"password\" name=\"VerifyPassword\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_PASSWORD_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (m_vPassword.GetCharPtr()); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


		if (HTMLFilter (pvEmpireData[SystemEmpireData::RealName].GetCharPtr(), &strFilter, 0, false) == OK) {

			
	Write ("<tr><td align=\"left\">Real name:</td><td><input type=\"text\" name=\"RealName\" size=\"40\" maxlength=\"", sizeof ("<tr><td align=\"left\">Real name:</td><td><input type=\"text\" name=\"RealName\" size=\"40\" maxlength=\"") - 1);
	Write (MAX_REAL_NAME_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
			
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


		if (HTMLFilter (pvEmpireData[SystemEmpireData::Email].GetCharPtr(), &strFilter, 0, false) == OK) {

			
	Write ("<tr><td align=\"left\">E-mail address:</td><td><input type=\"text\" name=\"Email\" size=\"40\" maxlength=\"", sizeof ("<tr><td align=\"left\">E-mail address:</td><td><input type=\"text\" name=\"Email\" size=\"40\" maxlength=\"") - 1);
	Write (MAX_EMAIL_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
			
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


		if (HTMLFilter (pvEmpireData[SystemEmpireData::WebPage].GetCharPtr(), &strFilter, 0, false) == OK) {

			
	Write ("<tr><td align=\"left\">Web page URL:</td><td><input type=\"text\" name=\"WebPage\" size=\"60\" maxlength=\"", sizeof ("<tr><td align=\"left\">Web page URL:</td><td><input type=\"text\" name=\"WebPage\" size=\"60\" maxlength=\"") - 1);
	Write (MAX_WEB_PAGE_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
			
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}

		
	Write ("<tr><td>Choose an alien icon:</td><td><select name=\"AlienSelect\">", sizeof ("<tr><td>Choose an alien icon:</td><td><select name=\"AlienSelect\">") - 1);
	if (pvEmpireData[SystemEmpireData::AlienKey] == NO_KEY) {
			
	Write ("<option value=\"0\">An alien icon from the system set</option><option selected value=\"1\">An uploaded alien icon</option>", sizeof ("<option value=\"0\">An alien icon from the system set</option><option selected value=\"1\">An uploaded alien icon</option>") - 1);
	} else {
			
	Write ("<option selected value=\"0\">An alien icon from the system set</option><option value=\"1\">An uploaded alien icon</option>", sizeof ("<option selected value=\"0\">An alien icon from the system set</option><option value=\"1\">An uploaded alien icon</option>") - 1);
	}

		
	Write ("</select>", sizeof ("</select>") - 1);
	WriteButton (BID_CHOOSEALIEN);

		
	Write ("</tr><tr><td>System messages saved:<td>", sizeof ("</tr><tr><td>System messages saved:<td>") - 1);
	iErrCode = g_pGameEngine->GetNumSystemMessages (m_iEmpireKey, &iNumSystemMessages);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iNumSystemMessages > 0) {
			Write (iNumSystemMessages);
			
	Write ("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp", sizeof ("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp") - 1);
	WriteButton (BID_VIEWMESSAGES);
		} else {
			
	Write ("None", sizeof ("None") - 1);
	}

		
	Write ("</tr><tr><td>Maximum saved system messages:</td><td><select name=\"MaxNumSavedMessages\">", sizeof ("</tr><tr><td>Maximum saved system messages:</td><td><select name=\"MaxNumSavedMessages\">") - 1);
	iErrCode = g_pGameEngine->GetSystemLimitOnSavedSystemMessages (&iMaxNumSystemMessages);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		for (i = 0; i <= iMaxNumSystemMessages; i += 10) {
			
	Write ("<option", sizeof ("<option") - 1);
	if (pvEmpireData[SystemEmpireData::MaxNumSystemMessages].GetInteger() == i) {
				
	Write (" selected ", sizeof (" selected ") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
		
	Write ("</select></td></tr><tr><td>Almonaster graphical theme:</td><td><select name=\"GraphicalTheme\"><option", sizeof ("</select></td></tr><tr><td>Almonaster graphical theme:</td><td><select name=\"GraphicalTheme\"><option") - 1);
	if (pvEmpireData[SystemEmpireData::AlmonasterTheme].GetInteger() == INDIVIDUAL_ELEMENTS) { 
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (INDIVIDUAL_ELEMENTS); 
	Write ("\">Individual Graphical Elements</option><option", sizeof ("\">Individual Graphical Elements</option><option") - 1);
	if (pvEmpireData[SystemEmpireData::AlmonasterTheme].GetInteger() == ALTERNATIVE_PATH) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (ALTERNATIVE_PATH); 
	Write ("\">Graphics from an alternative path</option><option", sizeof ("\">Graphics from an alternative path</option><option") - 1);
	if (pvEmpireData[SystemEmpireData::AlmonasterTheme].GetInteger() == NULL_THEME) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (NULL_THEME); 
	Write ("\">Null Theme</option>", sizeof ("\">Null Theme</option>") - 1);
	iErrCode = g_pGameEngine->GetFullThemeKeys (&piThemeKey, &iNumThemes);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iNumThemes > 0) {

			Variant vThemeName;
			for (i = 0; i < iNumThemes; i ++) {
				if (g_pGameEngine->GetThemeName (piThemeKey[i], &vThemeName) == OK) {
					
	Write ("<option ", sizeof ("<option ") - 1);
	if (pvEmpireData[SystemEmpireData::AlmonasterTheme].GetInteger() == piThemeKey[i]) {
						
	Write (" selected", sizeof (" selected") - 1);
	}
					
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (piThemeKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (vThemeName.GetCharPtr()); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
			}

			g_pGameEngine->FreeKeys (piThemeKey);
		}
		
	Write ("</select> ", sizeof ("</select> ") - 1);
	WriteButton (BID_CHOOSETHEME); 
		
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	bool bFlag;


		
	Write ("<tr><td>Placement of command buttons:</td><td><select name=\"RepeatedButtons\">", sizeof ("<tr><td>Placement of command buttons:</td><td><select name=\"RepeatedButtons\">") - 1);
	iValue = pvEmpireData[SystemEmpireData::Options].GetInteger() & (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS);

		
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == 0) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">At top of screen only</option><option", sizeof ("\">At top of screen only</option><option") - 1);
	if (iValue == (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS)) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS); 
	Write ("\">At top and bottom of screen on system and game pages by default</option><option", sizeof ("\">At top and bottom of screen on system and game pages by default</option><option") - 1);
	if (iValue == SYSTEM_REPEATED_BUTTONS) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (SYSTEM_REPEATED_BUTTONS); 
	Write ("\">At top and bottom of screen only on system pages</option><option", sizeof ("\">At top and bottom of screen only on system pages</option><option") - 1);
	if (iValue == GAME_REPEATED_BUTTONS) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (GAME_REPEATED_BUTTONS); 
	Write ("\">At top and bottom of screen only on game pages by default</option></select></tr><tr><td>Display server time:</td><td><select name=\"TimeDisplay\">", sizeof ("\">At top and bottom of screen only on game pages by default</option></select></tr><tr><td>Display server time:</td><td><select name=\"TimeDisplay\">") - 1);
	iValue = pvEmpireData[SystemEmpireData::Options].GetInteger() & (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME);

		
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME)) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME); 
	Write ("\">On system pages and on game pages by default</option><option", sizeof ("\">On system pages and on game pages by default</option><option") - 1);
	if (iValue == SYSTEM_DISPLAY_TIME) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (SYSTEM_DISPLAY_TIME); 
	Write ("\">Only on system pages</option><option", sizeof ("\">Only on system pages</option><option") - 1);
	if (iValue == GAME_DISPLAY_TIME) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (GAME_DISPLAY_TIME); 
	Write ("\">Only on game pages by default</option><option", sizeof ("\">Only on game pages by default</option><option") - 1);
	if (iValue == 0) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">On neither system pages nor game pages by default</option></select></tr><tr><td>Fixed backgrounds <em>(requires Internet Explorer)</em>:</td><td><select name=\"FixedBg\"><option", sizeof ("\">On neither system pages nor game pages by default</option></select></tr><tr><td>Fixed backgrounds <em>(requires Internet Explorer)</em>:</td><td><select name=\"FixedBg\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & FIXED_BACKGROUNDS) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On</option><option", sizeof ("\">On</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off</option></select></tr><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"AutoRefresh\"><option", sizeof ("\">Off</option></select></tr><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"AutoRefresh\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & AUTO_REFRESH) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On by default</option><option", sizeof ("\">On by default</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off by default</option></select></tr><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"Countdown\"><option", sizeof ("\">Off by default</option></select></tr><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"Countdown\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & COUNTDOWN) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On by default</option><option", sizeof ("\">On by default</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off by default</option></select></tr><tr><td>Map coloring by diplomatic status:</td><td><select name=\"MapColoring\"><option", sizeof ("\">Off by default</option></select></tr><tr><td>Map coloring by diplomatic status:</td><td><select name=\"MapColoring\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & MAP_COLORING) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On by default</option><option", sizeof ("\">On by default</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off by default</option></select></tr><tr><td>Ship coloring by diplomatic status on map screen:</td><td><select name=\"ShipMapColoring\"><option", sizeof ("\">Off by default</option></select></tr><tr><td>Ship coloring by diplomatic status on map screen:</td><td><select name=\"ShipMapColoring\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SHIP_MAP_COLORING) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On by default</option><option", sizeof ("\">On by default</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off by default</option></select></tr><tr><td>Ship highlighting on map screen:</td><td><select name=\"ShipHighlighting\"><option", sizeof ("\">Off by default</option></select></tr><tr><td>Ship highlighting on map screen:</td><td><select name=\"ShipHighlighting\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SHIP_MAP_HIGHLIGHTING) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On by default</option><option", sizeof ("\">On by default</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off by default</option></select></tr><tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name=\"SensitiveMaps\"><option", sizeof ("\">Off by default</option></select></tr><tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name=\"SensitiveMaps\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SENSITIVE_MAPS) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On by default</option><option", sizeof ("\">On by default</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off by default</option></select></tr><tr><td>Partial maps:</td><td><select name=\"PartialMaps\"><option", sizeof ("\">Off by default</option></select></tr><tr><td>Partial maps:</td><td><select name=\"PartialMaps\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & PARTIAL_MAPS) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On by default</option><option", sizeof ("\">On by default</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off by default</option></select></tr>", sizeof ("\">Off by default</option></select></tr>") - 1);
	iOptions = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SHIPS_ON_MAP_SCREEN) != 0 ? SHIPS_ON_MAP_SCREEN : 0;

		if ((pvEmpireData[SystemEmpireData::Options].GetInteger() & SHIPS_ON_PLANETS_SCREEN) != 0) {
			iOptions |= SHIPS_ON_PLANETS_SCREEN;
		}

		
	Write ("<tr><td>Display ship menus in planet views:</td><td><select name=\"UpCloseShips\"><option ", sizeof ("<tr><td>Display ship menus in planet views:</td><td><select name=\"UpCloseShips\"><option ") - 1);
	if (iOptions == (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN)) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN); 
	Write ("\">Ship menus on both map and planets screens by default</option><option ", sizeof ("\">Ship menus on both map and planets screens by default</option><option ") - 1);
	if (iOptions == SHIPS_ON_MAP_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_MAP_SCREEN); 
	Write ("\">Ship menus on map screen by default</option><option ", sizeof ("\">Ship menus on map screen by default</option><option ") - 1);
	if (iOptions == SHIPS_ON_PLANETS_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_PLANETS_SCREEN); 
	Write ("\">Ship menus on planets screen by default</option><option ", sizeof ("\">Ship menus on planets screen by default</option><option ") - 1);
	if (iOptions == 0) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"0\">No ship menus in planet views by default</option></select></td></tr><tr><td>Display local maps in up-close map views:</td><td><select name=\"LocalMaps\"><option", sizeof ("value=\"0\">No ship menus in planet views by default</option></select></td></tr><tr><td>Display local maps in up-close map views:</td><td><select name=\"LocalMaps\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;

		if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">On by default</option><option", sizeof ("\">On by default</option><option") - 1);
	if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Off by default</option></select></tr>", sizeof ("\">Off by default</option></select></tr>") - 1);
	iValue = pvEmpireData[SystemEmpireData::DefaultMessageTarget].GetInteger();

		
	Write ("<tr><td>Default message target:</td><td><select name=\"MessageTarget\"><option", sizeof ("<tr><td>Default message target:</td><td><select name=\"MessageTarget\"><option") - 1);
	if (iValue == MESSAGE_TARGET_NONE) {
			
	Write (" selected", sizeof (" selected") - 1);
	}
		
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_NONE); 
	Write ("\">None</option><option", sizeof ("\">None</option><option") - 1);
	if (iValue == MESSAGE_TARGET_BROADCAST) {
			
	Write (" selected", sizeof (" selected") - 1);
	}
		
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_BROADCAST); 
	Write ("\">Broadcast</option><option", sizeof ("\">Broadcast</option><option") - 1);
	if (iValue == MESSAGE_TARGET_WAR) {
			
	Write (" selected", sizeof (" selected") - 1);
	}
		
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_WAR); 
	Write ("\">All at War</option><option", sizeof ("\">All at War</option><option") - 1);
	if (iValue == MESSAGE_TARGET_TRUCE) {
			
	Write (" selected", sizeof (" selected") - 1);
	}
		
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_TRUCE); 
	Write ("\">All at Truce</option><option", sizeof ("\">All at Truce</option><option") - 1);
	if (iValue == MESSAGE_TARGET_TRADE) {
			
	Write (" selected", sizeof (" selected") - 1);
	}
		
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_TRADE); 
	Write ("\">All at Trade</option><option", sizeof ("\">All at Trade</option><option") - 1);
	if (iValue == MESSAGE_TARGET_ALLIANCE) {
			
	Write (" selected", sizeof (" selected") - 1);
	}
		
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_ALLIANCE); 
	Write ("\">All at Alliance</option><option", sizeof ("\">All at Alliance</option><option") - 1);
	if (iValue == MESSAGE_TARGET_LAST_USED) {
			
	Write (" selected", sizeof (" selected") - 1);
	}
		
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_LAST_USED); 
	Write ("\">Last target used</option></select></td></tr><tr><td>Maximum number of ships built at once:</td><td><select name=\"MaxNumShipsBuiltAtOnce\">", sizeof ("\">Last target used</option></select></td></tr><tr><td>Maximum number of ships built at once:</td><td><select name=\"MaxNumShipsBuiltAtOnce\">") - 1);
	iValue = pvEmpireData[SystemEmpireData::MaxNumShipsBuiltAtOnce].GetInteger();

		for (i = 5; i < 16; i ++) {
			
	Write ("<option ", sizeof ("<option ") - 1);
	if (iValue == i) {
				
	Write ("selected ", sizeof ("selected ") - 1);
	}
			
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

		for (i = 20; i < 101; i += 10) {
			
	Write ("<option ", sizeof ("<option ") - 1);
	if (iValue == i) {
				
	Write ("selected ", sizeof ("selected ") - 1);
	}
			
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
		
	Write ("</select></td></tr><tr><td>Default builder planet:</td><td><select name=\"DefaultBuilderPlanet\">", sizeof ("</select></td></tr><tr><td>Default builder planet:</td><td><select name=\"DefaultBuilderPlanet\">") - 1);
	iValue = pvEmpireData[SystemEmpireData::DefaultBuilderPlanet].GetInteger();

		
	Write ("<option ", sizeof ("<option ") - 1);
	if (iValue == HOMEWORLD_DEFAULT_BUILDER_PLANET) {
			
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (HOMEWORLD_DEFAULT_BUILDER_PLANET); 
	Write ("\">Homeworld</option><option ", sizeof ("\">Homeworld</option><option ") - 1);
	if (iValue == LAST_BUILDER_DEFAULT_BUILDER_PLANET) {
			
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (LAST_BUILDER_DEFAULT_BUILDER_PLANET); 
	Write ("\">Last builder planet used</option><option ", sizeof ("\">Last builder planet used</option><option ") - 1);
	if (iValue == NO_DEFAULT_BUILDER_PLANET) {
			
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (NO_DEFAULT_BUILDER_PLANET); 
	Write ("\">No default builder planet</option></select></td></tr><tr><td>Independent ship gifts:</td><td><select name=\"IndependentGifts\"><option", sizeof ("\">No default builder planet</option></select></td></tr><tr><td>Independent ship gifts:</td><td><select name=\"IndependentGifts\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

		if (!bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Accept by default</option><option", sizeof ("\">Accept by default</option><option") - 1);
	if (bFlag) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">Reject by default</option></select></tr><tr><td>Game screen password hashing:</td><td><select name=\"Hashing\"><option", sizeof ("\">Reject by default</option></select></tr><tr><td>Game screen password hashing:</td><td><select name=\"Hashing\"><option") - 1);
	bIP = (pvEmpireData[SystemEmpireData::Options].GetInteger() & IP_ADDRESS_PASSWORD_HASHING) != 0;
		bID = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SESSION_ID_PASSWORD_HASHING) != 0;

		if (bIP && bID) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (IP_ADDRESS_PASSWORD_HASHING | SESSION_ID_PASSWORD_HASHING); 
	Write ("\">Use both IP address and Session Id</option><option", sizeof ("\">Use both IP address and Session Id</option><option") - 1);
	if (bIP && !bID) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (IP_ADDRESS_PASSWORD_HASHING); 
	Write ("\">Use only IP address</option><option", sizeof ("\">Use only IP address</option><option") - 1);
	if (bID && !bIP) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (SESSION_ID_PASSWORD_HASHING); 
	Write ("\">Use only Session Id</option><option", sizeof ("\">Use only Session Id</option><option") - 1);
	if (!bIP && !bID) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Use neither (insecure)</option></select></tr>", sizeof ("\">Use neither (insecure)</option></select></tr>") - 1);
	if (m_iPrivilege >= APPRENTICE) {

			
	Write ("<tr><td>Profile Viewer search interface:</td><td><select name=\"AdvancedSearch\"><option", sizeof ("<tr><td>Profile Viewer search interface:</td><td><select name=\"AdvancedSearch\"><option") - 1);
	bConfirm = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SHOW_ADVANCED_SEARCH_INTERFACE) != 0;

			if (bConfirm) {
				
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Show advanced search interface</option><option", sizeof (" value=\"1\">Show advanced search interface</option><option") - 1);
	if (!bConfirm) {
				
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Show simple search interface</option></select></tr>", sizeof (" value=\"0\">Show simple search interface</option></select></tr>") - 1);
	}

		
	Write ("<tr><td>Update messages on empire obliteration:</td><td><select name=\"DisplayFatalUpdates\"><option", sizeof ("<tr><td>Update messages on empire obliteration:</td><td><select name=\"DisplayFatalUpdates\"><option") - 1);
	bConfirm = (pvEmpireData[SystemEmpireData::Options].GetInteger() & DISPLAY_FATAL_UPDATE_MESSAGES) != 0;

		if (bConfirm) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Always display update messages on empire obliteration</option><option", sizeof (" value=\"1\">Always display update messages on empire obliteration</option><option") - 1);
	if (!bConfirm) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Never display update messages on empire obliteration</option></select></tr><tr><td>Prompt to confirm on enter or quit game:</td><td><select name=\"Confirm\"><option", sizeof (" value=\"0\">Never display update messages on empire obliteration</option></select></tr><tr><td>Prompt to confirm on enter or quit game:</td><td><select name=\"Confirm\"><option") - 1);
	bConfirm = (pvEmpireData[SystemEmpireData::Options].GetInteger() & CONFIRM_ON_ENTER_OR_QUIT_GAME) != 0;

		if (bConfirm) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Always confirm on enter or quit game</option><option", sizeof (" value=\"1\">Always confirm on enter or quit game</option><option") - 1);
	if (!bConfirm) {
			
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Never confirm on enter or quit game</option></select></tr><tr><td>Empire autologon:</td><td><select name=\"AutoLogon\"><option value=\"", sizeof (" value=\"0\">Never confirm on enter or quit game</option></select></tr><tr><td>Empire autologon:</td><td><select name=\"AutoLogon\"><option value=\"") - 1);
	Write (m_iEmpireKey); 
	Write ("\">Use the current empire (", sizeof ("\">Use the current empire (") - 1);
	Write (m_vEmpireName.GetCharPtr()); 
	Write (") to autologon</option>", sizeof (") to autologon</option>") - 1);
	if (iAutoLogonSelected == MAYBE_AUTOLOGON) {

			int iAutoLogonKey = NO_KEY;
			ICookie* pCookie = m_pHttpRequest->GetCookie (AUTOLOGON_EMPIREKEY_COOKIE);

			if (pCookie != NULL && pCookie->GetValue() != NULL) {

				iAutoLogonKey = pCookie->GetIntValue();

				if (iAutoLogonKey != m_iEmpireKey) {

					Variant vName;
					iErrCode = g_pGameEngine->GetEmpireName (iAutoLogonKey, &vName);
					if (iErrCode != OK) {
						iAutoLogonSelected = m_iEmpireKey;
						goto EndCookie;
					}

					
	Write ("<option selected value=\"", sizeof ("<option selected value=\"") - 1);
	Write (iAutoLogonKey); 
	Write ("\">Use the ", sizeof ("\">Use the ") - 1);
	Write (vName.GetCharPtr()); 
	Write (" empire to autologon</option>", sizeof (" empire to autologon</option>") - 1);
	}

				iAutoLogonSelected = iAutoLogonKey;
			}
		}

	EndCookie:

		
	Write ("<option ", sizeof ("<option ") - 1);
	if (iAutoLogonSelected == NO_AUTOLOGON || iAutoLogonSelected == MAYBE_AUTOLOGON) {
			iAutoLogonSelected = NO_AUTOLOGON;
			
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (NO_AUTOLOGON); 
	Write ("\">Do not autologon</option></select><input type=\"hidden\" name=\"AutoLogonSel\" value=\"", sizeof ("\">Do not autologon</option></select><input type=\"hidden\" name=\"AutoLogonSel\" value=\"") - 1);
	Write (iAutoLogonSelected); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	if (HTMLFilter (pvEmpireData[SystemEmpireData::Quote].GetCharPtr(), &strFilter, 0, false) == OK) {

			
	Write ("<tr><td align=\"left\">Quote:<br></td><td><textarea name=\"Quote\" cols=\"50\" rows=\"6\">", sizeof ("<tr><td align=\"left\">Quote:<br></td><td><textarea name=\"Quote\" cols=\"50\" rows=\"6\">") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
			
	Write ("</textarea></td></tr>", sizeof ("</textarea></td></tr>") - 1);
	}

		if (HTMLFilter (pvEmpireData[SystemEmpireData::VictorySneer].GetCharPtr(), &strFilter, 0, false) == OK) {

			
	Write ("<tr><td align=\"left\">Victory Sneer:<br>(Sent to your opponents when you nuke them)</td><td><textarea name=\"VictorySneer\" cols=\"50\" rows=\"4\">", sizeof ("<tr><td align=\"left\">Victory Sneer:<br>(Sent to your opponents when you nuke them)</td><td><textarea name=\"VictorySneer\" cols=\"50\" rows=\"4\">") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
			
	Write ("</textarea></td></tr>", sizeof ("</textarea></td></tr>") - 1);
	}

		
	Write ("</table><p>", sizeof ("</table><p>") - 1);
	if (m_iEmpireKey != ROOT_KEY && m_iEmpireKey != GUEST_KEY) {
			WriteButton (BID_BLANKEMPIRESTATISTICS);
		}

		if (!(pvEmpireData[SystemEmpireData::Options].GetInteger() & EMPIRE_MARKED_FOR_DELETION)) {

			if (m_iEmpireKey != ROOT_KEY && m_iEmpireKey != GUEST_KEY) {
				WriteButton (BID_DELETEEMPIRE);
			}

		} else {

			WriteButton (BID_UNDELETEEMPIRE);
		}

		
	Write ("<h3>Default ship names:</h3><p><table width=\"60%\">", sizeof ("<h3>Default ship names:</h3><p><table width=\"60%\">") - 1);
	for (i = FIRST_SHIP; i < NUM_SHIP_TYPES / 2; i ++) {

			
	Write ("<tr>", sizeof ("<tr>") - 1);
	if (HTMLFilter (pvEmpireData[SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[i]].GetCharPtr(), &strFilter, 0, false) == OK) {

				
	Write ("<td>", sizeof ("<td>") - 1);
	Write (SHIP_TYPE_STRING[i]); 
	Write (":</td><td><input type=\"text\" size=\"12\" maxlength=\"", sizeof (":</td><td><input type=\"text\" size=\"12\" maxlength=\"") - 1);
	Write (MAX_SHIP_NAME_LENGTH); 
	Write ("\" name=\"ShipName", sizeof ("\" name=\"ShipName") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength()); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	}

			j = i + NUM_SHIP_TYPES / 2;

			if (HTMLFilter (pvEmpireData[SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[j]].GetCharPtr(), &strFilter, 0, false) == OK) {

				
	Write ("<td>", sizeof ("<td>") - 1);
	Write (SHIP_TYPE_STRING[j]); 
	Write (":</td><td><input type=\"text\" size=\"12\" maxlength=\"", sizeof (":</td><td><input type=\"text\" size=\"12\" maxlength=\"") - 1);
	Write (MAX_SHIP_NAME_LENGTH); 
	Write ("\" name=\"ShipName", sizeof ("\" name=\"ShipName") - 1);
	Write (j); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength()); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	}

			
	Write ("</tr>", sizeof ("</tr>") - 1);
	}

		
	Write ("</table><p>", sizeof ("</table><p>") - 1);
	Cleanup:

		if (iErrCode != OK) {
			
	Write ("<p>Error ", sizeof ("<p>Error ") - 1);
	Write (iErrCode); 
	Write (" occurred rendering your profile<p>", sizeof (" occurred rendering your profile<p>") - 1);
	}

		WriteButton (BID_CANCEL);

		g_pGameEngine->FreeData (pvEmpireData);
		}

		break;

	case 1:
		{

		int iAlienKey;
		EmpireCheck (g_pGameEngine->GetEmpireAlienKey (m_iEmpireKey, &iAlienKey));

		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"1\"><p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"1\"><p>") - 1);
	WriteAlienString (iAlienKey, m_iEmpireKey, "Your current alien icon", false);
		
	Write ("<p>", sizeof ("<p>") - 1);
	switch (iAlienSelect) {
		case 0:
			{

			int iNumAliens;
			Variant** ppvAlienData;
			Check (g_pGameEngine->GetAlienKeys (&ppvAlienData, &iNumAliens));

			
	Write ("<input type=\"hidden\" name=\"WhichAlien\" value=\"0\"><p>Choose an alien icon for your empire:<p></center>", sizeof ("<input type=\"hidden\" name=\"WhichAlien\" value=\"0\"><p>Choose an alien icon for your empire:<p></center>") - 1);
	for (i = 0; i < iNumAliens; i ++) {
				WriteAlienButtonString (
					ppvAlienData[i][0].GetInteger(),
					iAlienKey == ppvAlienData[i][0],
					ppvAlienData[i][1].GetCharPtr()
					);
			}

			
	Write ("<center>", sizeof ("<center>") - 1);
	if (iNumAliens > 0) {
				g_pGameEngine->FreeData (ppvAlienData);
			}
			
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);

			}
			break;

		case 1:

			// Icon upload form
			
	Write ("<input type=\"hidden\" name=\"WhichAlien\" value=\"1\"><p><table width=\"60%\"><tr><td>In order to be accepted, the size of your icon must be less than 10KB and it must be a correctly formatted 40x40 transparent .gif (GIF89a). The icon that you upload will overwrite any previous icons you may have uploaded to the server.<center><p><input type=\"file\" name=\"IconFile\" size=\"40\"></td></tr></table><p>", sizeof ("<input type=\"hidden\" name=\"WhichAlien\" value=\"1\"><p><table width=\"60%\"><tr><td>In order to be accepted, the size of your icon must be less than 10KB and it must be a correctly formatted 40x40 transparent .gif (GIF89a). The icon that you upload will overwrite any previous icons you may have uploaded to the server.<center><p><input type=\"file\" name=\"IconFile\" size=\"40\"></td></tr></table><p>") - 1);
	WriteButton (BID_CANCEL);
			WriteButton (BID_CHOOSE);

			break;

		default:
			Assert (false);
		}

		}

		break;

	case 2:
		{

		Variant** ppvMessage;
		int* piMessageKey, iNumMessages, j, iNumNames = 0;
		bool bSystem = false, bFound;
		const char* pszFontColor = NULL;

		EmpireCheck (g_pGameEngine->GetSavedSystemMessages (m_iEmpireKey, &piMessageKey, &ppvMessage, &iNumMessages));

		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"2\">", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"2\">") - 1);
	if (iNumMessages == 0) {
			
	Write ("<p>You have no saved system messages.", sizeof ("<p>You have no saved system messages.") - 1);
	} else {

			// Sort
			UTCTime* ptTime = (UTCTime*) StackAlloc (iNumMessages * sizeof (UTCTime));
			int* piKey = (int*) StackAlloc (iNumMessages * sizeof (int));

			for (i = 0; i < iNumMessages; i ++) {
				piKey[i] = i;
				ptTime[i] = ppvMessage[i][0].GetUTCTime();
			}

			Algorithm::QSortTwoDescending<UTCTime, int> (ptTime, piKey, iNumMessages);

			// Display
			String* pstrNameList = new String [iNumMessages];
			if (pstrNameList == NULL) {
				
	Write ("<p>Server is out of memory", sizeof ("<p>Server is out of memory") - 1);
	} else {

				Algorithm::AutoDelete<String> autopstrNameList (pstrNameList, true);

				
	Write ("<p>You have <strong>", sizeof ("<p>You have <strong>") - 1);
	Write (iNumMessages); 
	Write ("</strong> saved system message", sizeof ("</strong> saved system message") - 1);
	if (iNumMessages != 1) {
					
	Write ("s", sizeof ("s") - 1);
	}

				
	Write (":<p><input type=\"hidden\" name=\"NumSavedSystemMessages\" value=\"", sizeof (":<p><input type=\"hidden\" name=\"NumSavedSystemMessages\" value=\"") - 1);
	Write (iNumMessages); 
	Write ("\"><table width=\"45%\">", sizeof ("\"><table width=\"45%\">") - 1);
	const char* pszSender;
				char pszDate [OS::MaxDateLength];

				for (i = 0; i < iNumMessages; i ++) {

					pszSender = ppvMessage[piKey[i]][1].GetCharPtr();

					
	Write ("<input type=\"hidden\" name=\"MsgKey", sizeof ("<input type=\"hidden\" name=\"MsgKey") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (piMessageKey[piKey[i]]); 
					
	Write ("\"><input type=\"hidden\" name=\"MsgSrc", sizeof ("\"><input type=\"hidden\" name=\"MsgSrc") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (pszSender);
					
	Write ("\"><tr><td>Time: ", sizeof ("\"><tr><td>Time: ") - 1);
	iErrCode = Time::GetDateString (ppvMessage[piKey[i]][0], pszDate);
					if (iErrCode != OK) {
						
	Write ("Could not read date", sizeof ("Could not read date") - 1);
	} else {
						Write (pszDate);
					}

					 
	Write ("<br>Sender: ", sizeof ("<br>Sender: ") - 1);
	if (String::StrCmp (pszSender, SYSTEM_MESSAGE_SENDER) == 0) {

						bSystem = true;
						
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (SYSTEM_MESSAGE_SENDER); 
	Write ("</strong>", sizeof ("</strong>") - 1);
	} else {

						
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (pszSender); 
	Write ("</strong>", sizeof ("</strong>") - 1);
	// Find name in lists
						bFound = false;
						for (j = 0; j < iNumNames; j ++) {
							if (pstrNameList[j].Equals (pszSender)) {
								bFound = true;
								break;
							}
						}
						// Add name to list if not found
						if (!bFound) {
							pstrNameList[iNumNames] = pszSender;
							iNumNames ++;
						}
					}

					if (ppvMessage[piKey[i]][2] != 0) {
						
	Write (" (broadcast)", sizeof (" (broadcast)") - 1);
	pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
					} else {
						pszFontColor = m_vPrivateMessageColor.GetCharPtr();
					}

					
	Write ("<br>Delete: <input type=\"checkbox\" name=\"DelChBx", sizeof ("<br>Delete: <input type=\"checkbox\" name=\"DelChBx") - 1);
	Write (i); 
					
	Write ("\"></td></tr><tr><td><font size=\"", sizeof ("\"></td></tr><tr><td><font size=\"") - 1);
	Write (DEFAULT_MESSAGE_FONT_SIZE); 
					
	Write ("\" face=\"", sizeof ("\" face=\"") - 1);
	Write (DEFAULT_MESSAGE_FONT); 
	Write ("\" color=\"#", sizeof ("\" color=\"#") - 1);
	Write (pszFontColor); 
	Write ("\">", sizeof ("\">") - 1);
	WriteFormattedMessage (ppvMessage[piKey[i]][3].GetCharPtr());
					
	Write ("</font></td></tr><tr><td>&nbsp;</td></tr>", sizeof ("</font></td></tr><tr><td>&nbsp;</td></tr>") - 1);
	}

				
	Write ("</table><p>Delete messages:<p>", sizeof ("</table><p>Delete messages:<p>") - 1);
	WriteButton (BID_ALL);
				WriteButton (BID_SELECTION);

				if (bSystem) {
					WriteButton (BID_SYSTEM);
				}
				if (iNumNames > 0) {
					WriteButton (BID_EMPIRE);
					
	Write ("<select name=\"SelectedEmpire\">", sizeof ("<select name=\"SelectedEmpire\">") - 1);
	for (j = 0; j < iNumNames; j ++) {
						
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (pstrNameList[j]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pstrNameList[j]); 
	Write ("</option>", sizeof ("</option>") - 1);
	} 
	Write ("</select>", sizeof ("</select>") - 1);
	}
			}

			g_pGameEngine->FreeData (ppvMessage);
			g_pGameEngine->FreeKeys (piMessageKey);
		}

		}
		break;

	case 3:
		{

		int iThemeKey, iLivePlanetKey, iDeadPlanetKey, iColorKey, iHorzKey, iVertKey, iBackgroundKey, iSeparatorKey,
			iButtonKey;

		EmpireCheck (g_pGameEngine->GetEmpireThemeKey (m_iEmpireKey, &iThemeKey));

		switch (iThemeKey) {

		case INDIVIDUAL_ELEMENTS:

			EmpireCheck (g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey));
			EmpireCheck (g_pGameEngine->GetEmpireHorzKey (m_iEmpireKey, &iHorzKey));
			EmpireCheck (g_pGameEngine->GetEmpireVertKey (m_iEmpireKey, &iVertKey));
			EmpireCheck (g_pGameEngine->GetEmpireColorKey (m_iEmpireKey, &iColorKey));

			iBackgroundKey = m_iBackgroundKey;
			iSeparatorKey = m_iSeparatorKey;
			iButtonKey = m_iButtonKey;

			break;

		case ALTERNATIVE_PATH:

			EmpireCheck (g_pGameEngine->GetDefaultUIKeys (
				&iBackgroundKey,
				&iLivePlanetKey,
				&iDeadPlanetKey,
				&iButtonKey,
				&iSeparatorKey,
				&iHorzKey,
				&iVertKey,
				&iColorKey
				));

			break;

		default:

			iBackgroundKey = iSeparatorKey = iButtonKey = iLivePlanetKey = iDeadPlanetKey = iHorzKey = iVertKey = 
				iColorKey = iThemeKey;
			break;
		}

		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"3\"><p>Choose your individual UI elements:<p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"3\"><p>Choose your individual UI elements:<p>") - 1);
	iErrCode = RenderThemeInfo (iBackgroundKey, iLivePlanetKey, 
			iDeadPlanetKey, iSeparatorKey, iButtonKey, iHorzKey, iVertKey, iColorKey);

		if (iErrCode != OK) {
			
	Write ("Theme information could not be rendered. The error was ", sizeof ("Theme information could not be rendered. The error was ") - 1);
	Write (iErrCode); 
	Write ("<p>", sizeof ("<p>") - 1);
	} else {

			int iColorKey;
			Variant vTextColor, vGoodColor, vBadColor, vPrivateColor, vBroadcastColor, vCustomTableColor;

			EmpireCheck (g_pGameEngine->GetEmpireCustomTextColor (m_iEmpireKey, &vTextColor));
			EmpireCheck (g_pGameEngine->GetEmpireCustomGoodColor (m_iEmpireKey, &vGoodColor));
			EmpireCheck (g_pGameEngine->GetEmpireCustomBadColor (m_iEmpireKey, &vBadColor));
			EmpireCheck (g_pGameEngine->GetEmpireCustomPrivateMessageColor (m_iEmpireKey, &vPrivateColor));
			EmpireCheck (g_pGameEngine->GetEmpireCustomBroadcastMessageColor (m_iEmpireKey, &vBroadcastColor));
			EmpireCheck (g_pGameEngine->GetEmpireCustomTableColor (m_iEmpireKey, &vCustomTableColor));

			EmpireCheck (g_pGameEngine->GetEmpireColorKey (m_iEmpireKey, &iColorKey));

			
	Write ("<tr><td>Custom text colors</td><td>&nbsp;</td><td>Text color:<br><input type=\"text\" name=\"CustomTextColor\" size=\"", sizeof ("<tr><td>Custom text colors</td><td>&nbsp;</td><td>Text color:<br><input type=\"text\" name=\"CustomTextColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vTextColor.GetCharPtr()); 
	Write ("\"><br>Good color:<br><input type=\"text\" name=\"CustomGoodColor\" size=\"", sizeof ("\"><br>Good color:<br><input type=\"text\" name=\"CustomGoodColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vGoodColor.GetCharPtr()); 
	Write ("\"><br>Bad color:<br><input type=\"text\" name=\"CustomBadColor\" size=\"", sizeof ("\"><br>Bad color:<br><input type=\"text\" name=\"CustomBadColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vBadColor.GetCharPtr()); 
	Write ("\"><br>Message color:<br><input type=\"text\" name=\"CustomMessageColor\" size=\"", sizeof ("\"><br>Message color:<br><input type=\"text\" name=\"CustomMessageColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vPrivateColor.GetCharPtr()); 
	Write ("\"><br>Broadcast color:<br><input type=\"text\" name=\"CustomBroadcastColor\" size=\"", sizeof ("\"><br>Broadcast color:<br><input type=\"text\" name=\"CustomBroadcastColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vBroadcastColor.GetCharPtr()); 
	Write ("\"><br>Table color:<br><input type=\"text\" name=\"CustomTableColor\" size=\"", sizeof ("\"><br>Table color:<br><input type=\"text\" name=\"CustomTableColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vCustomTableColor.GetCharPtr()); 
	Write ("\"></td></tr><tr><td>&nbsp;</td><td>&nbsp;</td><td bgcolor=\"", sizeof ("\"></td></tr><tr><td>&nbsp;</td><td>&nbsp;</td><td bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\"><input", sizeof ("\" align=\"center\"><input") - 1);
	if (iColorKey == CUSTOM_COLORS) {
				
	Write (" checked", sizeof (" checked") - 1);
	}
			
	Write (" type=\"radio\" name=\"Color\" value=\"", sizeof (" type=\"radio\" name=\"Color\" value=\"") - 1);
	Write (CUSTOM_COLORS); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}

		
	Write ("</table><p>", sizeof ("</table><p>") - 1);
	WriteButton (BID_CANCEL);

		}
		break;

	case 4: 
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"4\"><p>Enter a directory on your local disk or on a network where a full Almonaster theme can be found:", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"4\"><p>Enter a directory on your local disk or on a network where a full Almonaster theme can be found:") - 1);
	Variant vPath;
		EmpireCheck (g_pGameEngine->GetEmpireAlternativeGraphicsPath (m_iEmpireKey, &vPath));

		
	Write ("<p><input type=\"text\" name=\"GraphicsPath\" size=\"50\" maxlength=\"", sizeof ("<p><input type=\"text\" name=\"GraphicsPath\" size=\"50\" maxlength=\"") - 1);
	Write (MAX_GRAPHICS_ALTERNATIVE_PATH_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vPath.GetCharPtr()); 
	Write ("\"><p>E.g:</strong> <strong>file://C:/Almonaster/MyCoolTheme</strong> or <strong>http://www.myisp.net/~myusername/MyTheme</strong><p><table width=\"60%\"><tr><td>Make sure the path is valid and the theme is complete, or else your browser will have problems displaying images. If you provide a local file path and you log into the server from a computer that doesn't have the same same theme directory, then the images won't be displayed either.<p>If you haven't done so already, you should go back and choose text colors from the Individual Graphical Elements page that match the style of your theme.<p>In short, hit cancel unless you know what you are doing.</td></tr></table><p>", sizeof ("\"><p>E.g:</strong> <strong>file://C:/Almonaster/MyCoolTheme</strong> or <strong>http://www.myisp.net/~myusername/MyTheme</strong><p><table width=\"60%\"><tr><td>Make sure the path is valid and the theme is complete, or else your browser will have problems displaying images. If you provide a local file path and you log into the server from a computer that doesn't have the same same theme directory, then the images won't be displayed either.<p>If you haven't done so already, you should go back and choose text colors from the Individual Graphical Elements page that match the style of your theme.<p>In short, hit cancel unless you know what you are doing.</td></tr></table><p>") - 1);
	WriteButton (BID_CANCEL);
		WriteButton (BID_CHOOSE);

		int* piThemeKey, iNumThemes;
		Check (g_pGameEngine->GetThemeKeys (&piThemeKey, &iNumThemes));

		if (iNumThemes == 0) {
			
	Write ("<p>There are no themes available for download", sizeof ("<p>There are no themes available for download") - 1);
	} else {

			
	Write ("<p>You can download the following themes:<p><table><tr><td><ul>", sizeof ("<p>You can download the following themes:<p><table><tr><td><ul>") - 1);
	Variant* pvThemeData;
			bool bElement;

			int iOptions;

			for (i = 0; i < iNumThemes; i ++) {

				if (g_pGameEngine->GetThemeData (piThemeKey[i], &pvThemeData) != OK) {
					continue;
				}

				
	Write ("<li><a href=\"", sizeof ("<li><a href=\"") - 1);
	WriteThemeDownloadSrc (piThemeKey[i], pvThemeData[SystemThemes::FileName].GetCharPtr());

				
	Write ("\">", sizeof ("\">") - 1);
	Write (pvThemeData[SystemThemes::Name].GetCharPtr()); 
	Write ("</a> (", sizeof ("</a> (") - 1);
	iOptions = pvThemeData[SystemThemes::Options].GetInteger();

				bElement = false;
				if (iOptions & THEME_BACKGROUND) { bElement = true;
					
	Write ("Background", sizeof ("Background") - 1);
	}

				if (iOptions & THEME_LIVE_PLANET) {
					if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
						bElement = true;
					}
					
	Write ("Live Planet", sizeof ("Live Planet") - 1);
	}

				if (iOptions & THEME_DEAD_PLANET) {
					if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
						bElement = true;
					} 
					
	Write ("Dead Planet", sizeof ("Dead Planet") - 1);
	}

				if (iOptions & THEME_SEPARATOR) {
					if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
						bElement = true;
					}
					
	Write ("Separator", sizeof ("Separator") - 1);
	}

				if (iOptions & THEME_BUTTONS) {
					if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
						bElement = true;
					}
					
	Write ("Buttons", sizeof ("Buttons") - 1);
	}

				if (iOptions & THEME_HORZ) {
					if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
						bElement = true;
					}
					
	Write ("Horizontal Bar", sizeof ("Horizontal Bar") - 1);
	}

				if (iOptions & THEME_VERT) {
					if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
						bElement = true;
					}
					
	Write ("Vertical Bar", sizeof ("Vertical Bar") - 1);
	}

				
	Write (")</li>", sizeof (")</li>") - 1);
	g_pGameEngine->FreeData (pvThemeData);
			}

			
	Write ("</ul></td></tr></table>", sizeof ("</ul></td></tr></table>") - 1);
	g_pGameEngine->FreeKeys (piThemeKey);

			
	Write ("<p>A complete theme has a Background, a Live Planet, a Dead Planet, a Separator, Buttons, a Horizontal Bar and a Vertical Bar</strong><p>", sizeof ("<p>A complete theme has a Background, a Live Planet, a Dead Planet, a Separator, Buttons, a Horizontal Bar and a Vertical Bar</strong><p>") - 1);
	}

		}
		break;

	case 5:
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"5\"><p>Are you sure you want to delete your empire? The data cannot be recovered if the empire is deleted.<p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"5\"><p>Are you sure you want to delete your empire? The data cannot be recovered if the empire is deleted.<p>") - 1);
	WriteButton (BID_CANCEL);
		WriteButton (BID_DELETEEMPIRE);

		}
		break;

	case 6:
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"6\"><p>Are you sure you want to blank your empire's statistics?<br>The data cannot be recovered.<p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"6\"><p>Are you sure you want to blank your empire's statistics?<br>The data cannot be recovered.<p>") - 1);
	WriteButton (BID_CANCEL);
		WriteButton (BID_BLANKEMPIRESTATISTICS);

		}
		break;

	case 7:
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"7\">", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"7\">") - 1);
	DisplayThemeData (iInfoThemeKey);

		}
		break;

	case 8:

		{
		// Alternative graphics path test
		
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"8\"><input type=\"hidden\" name=\"GraphicsPath\" value=\"", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"8\"><input type=\"hidden\" name=\"GraphicsPath\" value=\"") - 1);
	Write (pszGraphicsPath); 
	Write ("\"><p>If you see broken images below, press the Cancel button. Otherwise press Choose to select the alternative path:<p><img src=\"", sizeof ("\"><p>If you see broken images below, press the Cancel button. Otherwise press Choose to select the alternative path:<p><img src=\"") - 1);
	Write (pszGraphicsPath); 
	Write ("/", sizeof ("/") - 1);
	Write (LIVE_PLANET_NAME); 
	Write ("\"><img src=\"", sizeof ("\"><img src=\"") - 1);
	Write (pszGraphicsPath); 
	Write ("/", sizeof ("/") - 1);
	Write (DEAD_PLANET_NAME); 
	Write ("\"><p>", sizeof ("\"><p>") - 1);
	WriteButton (BID_CANCEL);
		WriteButton (BID_CHOOSE);

		}
		break;

	default:

		Assert (false);
	}

	SYSTEM_CLOSE


}