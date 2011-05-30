
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "Osal/Algorithm.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Options page
int HtmlRenderer::Render_Options() {

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

	INITIALIZE_EMPIRE

	INITIALIZE_GAME

	IHttpForm* pHttpForm;

	// Handle a submission
	int i, iErrCode, iOptionPage = 0;

	if (m_bOwnPost && !m_bRedirection) {

		// Handle submissions
		if ((pHttpForm = m_pHttpRequest->GetForm ("OptionPage")) == NULL) {
			goto Redirection;
		}
		int iOldValue, iNewValue, iOptionPageSubmit = pHttpForm->GetIntValue();

		switch (iOptionPageSubmit) {

		case 0:

			// Make sure cancel wasn't pressed
			if (!WasButtonPressed (BID_CANCEL)) {

				int iGameOptions, iGameClassOptions;
				bool bFlag, bUpdate;

				GameCheck (g_pGameEngine->GetEmpireOptions (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iGameOptions));
				GameCheck (g_pGameEngine->GetGameClassOptions (m_iGameClass, &iGameClassOptions));

				// Autorefesh
				if ((pHttpForm = m_pHttpRequest->GetForm ("AutoRefresh")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldAutoRefresh")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {

					bUpdate = iNewValue != 0;

					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, AUTO_REFRESH, bUpdate) == OK) {

						m_bAutoRefresh = bUpdate;

						if (bUpdate) {
							AddMessage ("Refresh on update countdown is now enabled");
						} else {
							AddMessage ("Refresh on update countdown is now disabled");
						}
					} else {
						AddMessage ("Your autorefresh setting could not be updated");
					}
				}

	#ifdef PRIVATE_DEBUG_ONLY

				// AutoUpdate
				if (bGameStarted) {
					if ((pHttpForm = m_pHttpRequest->GetForm ("AutoUpdate")) == NULL) {
						goto Redirection;
					}
					iNewValue = pHttpForm->GetIntValue();

					if ((pHttpForm = m_pHttpRequest->GetForm ("OldAutoUpdate")) == NULL) {
						goto Redirection;
					}
					iOldValue = pHttpForm->GetIntValue();

					if (iOldValue != iNewValue) {
						bUpdate = iNewValue != 0;
						iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, AUTO_UPDATE, bUpdate);
						if (iErrCode == OK) {

							if (bUpdate) {

								AddMessage ("You are now in automatic update mode");
								PageId pageRedirect;

								if (RedirectOnSubmitGame (&pageRedirect)) {
									return Redirect (pageRedirect);
								} else {
									g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
									return Redirect (m_pgPageId);
								}
							} else {
								AddMessage ("You are no longer in automatic update mode");
							}
						} else {
							if (iErrCode == ERROR_LAST_EMPIRE_CANNOT_AUTOUPDATE) {
								AddMessage ("All the other empires are auto-updating. You are not allowed to enter autoupdate mode");
							} else {
								AddMessage ("Your autoupdate setting could not be updated");
							}
						}
					}
				}

	#endif

				// RepeatedButtons
				if ((pHttpForm = m_pHttpRequest->GetForm ("RepeatButtons")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldRepeatButtons")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {

					bUpdate = iNewValue != 0;
					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, GAME_REPEATED_BUTTONS, bUpdate) == OK) {

						m_bRepeatedButtons = bUpdate;

						if (bUpdate) {
							AddMessage ("Your command buttons are now repeated");
						} else {
							AddMessage ("Your command buttons are no longer repeated");
						}
					} else {
						AddMessage ("Your command buttons setting could not be updated");
					}
				}

				// Handle server time display
				if ((pHttpForm = m_pHttpRequest->GetForm ("TimeDisplay")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldTimeDisplay")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {

					bUpdate = iNewValue != 0;
					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, GAME_DISPLAY_TIME, bUpdate) == OK) {

						m_bTimeDisplay = bUpdate;

						if (bUpdate) {
							AddMessage ("Server time display is now enabled");
						} else {
							AddMessage ("Server time display is no longer enabled");
						}
					} else {
						AddMessage ("Your server time display setting could not be updated");
					}
				}

				// Countdown
				if ((pHttpForm = m_pHttpRequest->GetForm ("Countdown")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldCountdown")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {

					bUpdate = iNewValue != 0;
					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, COUNTDOWN, bUpdate) == OK) {

						m_bCountdown = bUpdate;

						if (bUpdate) {
							AddMessage ("Update countdown is now enabled");
						} else {
							AddMessage ("Update countdown is now disabled");
						}
					} else {
						AddMessage ("Your update countdown setting could not be updated");
					}
				}

				// Map coloring
				if ((pHttpForm = m_pHttpRequest->GetForm ("MapColoring")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldMapColoring")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {
					bUpdate = iNewValue != 0;
					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, MAP_COLORING, bUpdate) == OK) {

						if (bUpdate) {
							AddMessage ("Map coloring by diplomatic status is now enabled");
						} else {
							AddMessage ("Map coloring by diplomatic status is now disabled");
						}
					} else {
						AddMessage ("Your map coloring setting could not be updated");
					}
				}

				// Ship map coloring
				if ((pHttpForm = m_pHttpRequest->GetForm ("ShipMapColoring")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldShipMapColoring")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {

					bUpdate = iNewValue != 0;
					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIP_MAP_COLORING, bUpdate) == OK) {

						if (bUpdate) {
							AddMessage ("Ship coloring by diplomatic status is now enabled");
						} else {
							AddMessage ("Ship coloring by diplomatic status is now disabled");
						}
					} else {
						AddMessage ("Your ship coloring setting could not be updated");
					}
				}

				// Ship highlighting
				if ((pHttpForm = m_pHttpRequest->GetForm ("ShipHighlighting")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldShipHighlighting")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {

					bUpdate = iNewValue != 0;
					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIP_MAP_HIGHLIGHTING, bUpdate) == OK) {

						if (bUpdate) {
							AddMessage ("Ship highlighting on the map screen is now enabled");
						} else {
							AddMessage ("Ship highlighting on the map screen is now disabled");
						}
					} else {
						AddMessage ("Your ship highlighting setting could not be updated");
					}
				}

				// Sensitive Maps
				if ((pHttpForm = m_pHttpRequest->GetForm ("SensitiveMaps")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldSensitiveMaps")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {
					bUpdate = iNewValue != 0;
					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SENSITIVE_MAPS, bUpdate) == OK) {

						if (bUpdate) {
							AddMessage ("Sensitive maps are now enabled");
						} else {
							AddMessage ("Sensitive maps are now disabled");
						}
					} else {
						AddMessage ("Your sensitive maps setting could not be updated");
					}
				}

				// Partial maps
				if ((pHttpForm = m_pHttpRequest->GetForm ("PartialMaps")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bFlag = (iGameOptions & PARTIAL_MAPS) != 0;
				if (bFlag != (iNewValue != 0)) {

					if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, PARTIAL_MAPS, !bFlag)) == OK) {
						if (!bFlag) {
							AddMessage ("Partial maps are now enabled");
						} else {
							AddMessage ("Partial maps are now disabled");
						}
					} else {
						AddMessage ("Your partial maps setting could not be changed: the error code was ");
						AppendMessage (iErrCode);
					}
				}

				// UpCloseShips
				if ((pHttpForm = m_pHttpRequest->GetForm ("UpCloseShips")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bFlag = (iGameOptions & SHIPS_ON_MAP_SCREEN) != 0;
				if (bFlag != ((iNewValue & SHIPS_ON_MAP_SCREEN) != 0)) {

					if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIPS_ON_MAP_SCREEN, !bFlag)) == OK) {
						if (!bFlag) {
							AddMessage ("Ship menus will now be displayed on map screen planet views");
						} else {
							AddMessage ("Ship menus will no longer be displayed on map screen planet views");
						}
					} else {
						AddMessage ("Your map screen ship menu setting could not be changed: the error code was ");
						AppendMessage (iErrCode);
					}
				}

				bFlag = (iGameOptions & SHIPS_ON_PLANETS_SCREEN) != 0;
				if (bFlag != ((iNewValue & SHIPS_ON_PLANETS_SCREEN) != 0)) {

					if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIPS_ON_PLANETS_SCREEN, !bFlag)) == OK) {
						if (!bFlag) {
							AddMessage ("Ship menus will now be displayed on the planets screen");
						} else {
							AddMessage ("Ship menus will no longer be displayed on the planets screen");
						}
					} else {
						AddMessage ("The planet screen ship menu setting could not be changed: the error code was ");
						AppendMessage (iErrCode);
					}
				}

				// LocalMaps
				if ((pHttpForm = m_pHttpRequest->GetForm ("LocalMaps")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				bFlag = (iGameOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;
				if (bFlag != (iNewValue != 0)) {

					if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, LOCAL_MAPS_IN_UPCLOSE_VIEWS, !bFlag)) == OK) {
						if (!bFlag) {
							AddMessage ("Local maps will now be displayed in up-close map views");
						} else {
							AddMessage ("Local maps will no longer be displayed in up-close map views");
						}
					} else {
						AddMessage ("Your local map setting could not be changed: the error code was ");
						AppendMessage (iErrCode);
					}
				}

				// Handle DefaultBuilderPlanet
				if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultBuilderPlanet")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				int iRealPlanet;
				GameCheck (g_pGameEngine->GetEmpireDefaultBuilderPlanet (
					m_iGameClass,
					m_iGameNumber,
					m_iEmpireKey,
					&iOldValue,
					&iRealPlanet
					));

				if (iNewValue != iOldValue) {

					iErrCode = g_pGameEngine->SetEmpireDefaultBuilderPlanet (m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue);
					if (iErrCode == OK) {
						AddMessage ("The default builder planet was updated");
					} else {
						AddMessage ("The default builder planet could not be updated; the error was ");
						AppendMessage (iErrCode);
					}
				}

				// Handle IndependentGifts
				if (iGameClassOptions & INDEPENDENCE) {

					if ((pHttpForm = m_pHttpRequest->GetForm ("IndependentGifts")) == NULL) {
						goto Redirection;
					}
					iNewValue = pHttpForm->GetIntValue();

					bFlag = (iGameOptions & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;
					if (bFlag != (iNewValue != 0)) {

						if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, REJECT_INDEPENDENT_SHIP_GIFTS, !bFlag)) == OK) {
							if (!bFlag) {
								AddMessage ("Independent ship gifts will now be rejected");
							} else {
								AddMessage ("Independent ship gifts will now be accepted");
							}
						} else {
							AddMessage ("Your independent ship setting could not be changed: the error code was ");
							AppendMessage (iErrCode);
						}
					}
				}

				// Handle MessageTarget
				if ((pHttpForm = m_pHttpRequest->GetForm ("MessageTarget")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				GameCheck (g_pGameEngine->GetEmpireDefaultMessageTarget (
					m_iGameClass,
					m_iGameNumber,
					m_iEmpireKey,
					&iOldValue
					));

				if (iNewValue != iOldValue) {

					iErrCode = g_pGameEngine->SetEmpireDefaultMessageTarget (m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue);
					if (iErrCode == OK) {
						AddMessage ("The default message target was updated");
					} else {
						AddMessage ("The default message target could not be updated; the error was ");
						AppendMessage (iErrCode);
					}
				}

				// MaxSavedMessages
				if ((pHttpForm = m_pHttpRequest->GetForm ("MaxSavedMessages")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxSavedMessages")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {
					if (g_pGameEngine->SetEmpireMaxNumSavedGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, 
						iNewValue) == OK) {

						char pszMessage [256];
						sprintf (pszMessage, "Up to %i game messages will be saved", iNewValue);
						AddMessage (pszMessage);
					} else {
						AddMessage ("Your max saved game messages setting could not be updated");
					}
				}

				// Ignore
				if ((pHttpForm = m_pHttpRequest->GetForm ("Ignore")) == NULL) {
					goto Redirection;
				}
				iNewValue = pHttpForm->GetIntValue();

				if ((pHttpForm = m_pHttpRequest->GetForm ("OldIgnore")) == NULL) {
					goto Redirection;
				}
				iOldValue = pHttpForm->GetIntValue();

				if (iOldValue != iNewValue) {
					if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, IGNORE_BROADCASTS, iNewValue != 0) == OK) {
						if (iNewValue != 0) {
							AddMessage ("You will now ignore all broadcasts");
						} else {
							AddMessage ("You will no longer ignore all broadcasts");
						}
					} else {
						AddMessage ("Your ignore broadcast setting be updated");
					}
				}

				// Notepad
				if ((pHttpForm = m_pHttpRequest->GetForm ("UpdateN")) != NULL) {
					if ((pHttpForm = m_pHttpRequest->GetForm ("Notepad")) == NULL) {
						goto Redirection;
					}
					const char* pszMessage = pHttpForm->GetValue();
					if (pszMessage == NULL || *pszMessage == '\0') {
						iErrCode = g_pGameEngine->SetEmpireNotepad (m_iGameClass, m_iGameNumber, m_iEmpireKey, "");
					} else {
						int iLength = String::StrLen (pszMessage);

						if (iLength > MAX_NOTEPAD_LENGTH) {
							char pszShortenedMessage [MAX_NOTEPAD_LENGTH + 1];
							strncpy (pszShortenedMessage, pszMessage, MAX_NOTEPAD_LENGTH);
							pszShortenedMessage[MAX_NOTEPAD_LENGTH] = '\0';

							iErrCode = g_pGameEngine->SetEmpireNotepad (m_iGameClass, m_iGameNumber, m_iEmpireKey, 
								pszShortenedMessage);

						} else {
							iErrCode = g_pGameEngine->SetEmpireNotepad (m_iGameClass, m_iGameNumber, m_iEmpireKey, 
								pszMessage);
						}

						if (iErrCode == OK) {
							if (iLength > MAX_NOTEPAD_LENGTH) {
								AddMessage ("Your notepad was updated, but the contents were truncated");
							} else {
								AddMessage ("Your notepad was updated");
							}
						} else {
							AddMessage ("Your notepad could not be updated");
						}
					}
				}

				// Check for ViewMessages button press
				if (WasButtonPressed (BID_VIEWMESSAGES)) {
					iOptionPage = 1;
					bRedirectTest = false;
					break;
				}

				// Check for search for empires with duplicate IP's
				if (WasButtonPressed (BID_SEARCHIPADDRESSES)) {

					bRedirectTest = false;
					SearchForDuplicateIPAddresses (m_iGameClass, m_iGameNumber);
					break;
				}

				// Check for search for empires with duplicate session ids
				if (WasButtonPressed (BID_SEARCHSESSIONIDS)) {

					bRedirectTest = false;
					SearchForDuplicateSessionIds (m_iGameClass, m_iGameNumber);
					break;
				}

				// Resign, surrender
				if (m_iGameState & STARTED) {

					if (WasButtonPressed (BID_RESIGN)) {
						g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
						m_iReserved = BID_RESIGN;
						return Redirect (QUIT);
					}

					if (WasButtonPressed (BID_SURRENDER)) {

						// Make sure this is allowed
						if (iErrCode == OK && (iGameClassOptions & USE_SC30_SURRENDERS)) {

							g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
							m_iReserved = BID_SURRENDER;
							return Redirect (QUIT);
						}
					}
				}

			} else {
				bRedirectTest = false;
			}

			break;

		case 1:

			{
				int iNumTestMessages, iMessageKey, iDeletedMessages = 0;

				// Get number of messages
				if ((pHttpForm = m_pHttpRequest->GetForm ("NumSavedGameMessages")) == NULL) {
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
						if (g_pGameEngine->DeleteGameMessage (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey) == OK) {
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
								if (g_pGameEngine->DeleteGameMessage (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey) == OK) {
									iDeletedMessages ++;
								}
							}
						}

					} else {

						// Check for delete system messages
						Variant vSender;
						if (WasButtonPressed (BID_SYSTEM)) {

							bRedirectTest = false;

							for (i = 0; i < iNumTestMessages; i ++) {

								// Get message key
								sprintf (pszForm, "MsgKey%i", i);
								if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
									goto Redirection;
								}
								iMessageKey = pHttpForm->GetIntValue();

								if (g_pGameEngine->GetGameMessageSender (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey, 
									&vSender) == OK &&
									String::StrCmp (vSender.GetCharPtr(), SYSTEM_MESSAGE_SENDER) == 0 &&
									g_pGameEngine->DeleteGameMessage (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey) == OK) {
									iDeletedMessages ++;
								}
							}

						} else {

							// Check for delete empire message
							if (WasButtonPressed (BID_EMPIRE)) {

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

									if (g_pGameEngine->GetGameMessageSender (
										m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey, &vSender) == OK &&
										String::StrCmp (vSender.GetCharPtr(), pszSrcEmpire) == 0 &&
										g_pGameEngine->DeleteGameMessage (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey) == OK) {
										iDeletedMessages ++;
									}
								}
							}
						}
					}
				}

				if (iDeletedMessages > 0) {

					char pszMessage [256];
					sprintf (
						pszMessage,
						"%i game message%s deleted", 
						iDeletedMessages,
						iDeletedMessages == 1 ? " was" : "s were"
						);

					AddMessage (pszMessage);
				}

			}
			break;

		case 2:

			// Nothing submitted
			break;

		default:

			Assert (false);
		}
	}

	GAME_REDIRECT_ON_SUBMIT

	GAME_OPEN

	// Individual page stuff starts here

	bool bSystem = false, bTempSystem, bFlag;
	int j, iNumNames = 0, iValue;

	switch (iOptionPage) {

	case 0:
		{

		int iDiplomacy, iGameClassOptions, iGameOptions;

		GameCheck (g_pGameEngine->GetGameClassDiplomacyLevel (m_iGameClass, &iDiplomacy));
		GameCheck (g_pGameEngine->GetGameClassOptions (m_iGameClass, &iGameClassOptions));
		GameCheck (g_pGameEngine->GetEmpireOptions (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iGameOptions));

		
	Write ("<input type=\"hidden\" name=\"OptionPage\" value=\"0\"><p><p><table width=\"80%\">", sizeof ("<input type=\"hidden\" name=\"OptionPage\" value=\"0\"><p><p><table width=\"80%\">") - 1);
	#ifdef PRIVATE_DEBUG_ONLY

		if (bGameStarted) {
			
	Write ("<tr><td>Automatically ready for updates:</td><td><select name=\"AutoUpdate\">", sizeof ("<tr><td>Automatically ready for updates:</td><td><select name=\"AutoUpdate\">") - 1);
	bool bAutoUpdate = (iGameOptions & AUTO_UPDATE) != 0;
			if (bAutoUpdate) {
				
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
				
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	}
				
	Write ("</select><input type=\"hidden\" name=\"OldAutoUpdate\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldAutoUpdate\" value=\"") - 1);
	Write (bAutoUpdate ? 1:0);
			
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}
	#endif

		
	Write ("<tr><td>Placement of command buttons:</td><td><select name=\"RepeatButtons\">", sizeof ("<tr><td>Placement of command buttons:</td><td><select name=\"RepeatButtons\">") - 1);
	if (m_bRepeatedButtons) { 
			
	Write ("<option value=\"0\">At top of screen only</option><option selected value=\"1\">At top and bottom of screen</option>", sizeof ("<option value=\"0\">At top of screen only</option><option selected value=\"1\">At top and bottom of screen</option>") - 1);
	} else { 
			
	Write ("<option selected value=\"0\">At top of screen only</option><option value=\"1\">At top and bottom of screen</option>", sizeof ("<option selected value=\"0\">At top of screen only</option><option value=\"1\">At top and bottom of screen</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldRepeatButtons\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldRepeatButtons\" value=\"") - 1);
	Write (m_bRepeatedButtons ? 1:0);
		
	Write ("\"></td></tr><tr><td>Server time display:</td><td><select name=\"TimeDisplay\">", sizeof ("\"></td></tr><tr><td>Server time display:</td><td><select name=\"TimeDisplay\">") - 1);
	if (m_bTimeDisplay) { 
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldTimeDisplay\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldTimeDisplay\" value=\"") - 1);
	Write (m_bTimeDisplay ? 1:0);
		
	Write ("\"></td></tr><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"AutoRefresh\">", sizeof ("\"></td></tr><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"AutoRefresh\">") - 1);
	if (m_bAutoRefresh) {
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	}

		
	Write ("</select><input type=\"hidden\" name=\"OldAutoRefresh\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldAutoRefresh\" value=\"") - 1);
	Write (m_bAutoRefresh ? 1:0); 
	Write ("\"></td></tr><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"Countdown\">", sizeof ("\"></td></tr><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"Countdown\">") - 1);
	if (m_bCountdown) { 
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldCountdown\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldCountdown\" value=\"") - 1);
	Write (m_bCountdown ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (iGameOptions & MAP_COLORING) != 0;

		
	Write ("<tr><td>Map coloring by diplomatic status:</td><td><select name=\"MapColoring\">", sizeof ("<tr><td>Map coloring by diplomatic status:</td><td><select name=\"MapColoring\">") - 1);
	if (bFlag) {
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldMapColoring\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldMapColoring\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (iGameOptions & SHIP_MAP_COLORING) != 0;

		
	Write ("<tr><td>Ship coloring by diplomatic status:</td><td><select name=\"ShipMapColoring\">", sizeof ("<tr><td>Ship coloring by diplomatic status:</td><td><select name=\"ShipMapColoring\">") - 1);
	if (bFlag) {
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldShipMapColoring\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldShipMapColoring\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (iGameOptions & SHIP_MAP_HIGHLIGHTING) != 0;

		
	Write ("<tr><td>Ship highlighting on map screen:</td><td><select name=\"ShipHighlighting\">", sizeof ("<tr><td>Ship highlighting on map screen:</td><td><select name=\"ShipHighlighting\">") - 1);
	if (bFlag) {
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldShipHighlighting\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldShipHighlighting\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (iGameOptions & SENSITIVE_MAPS) != 0;

		
	Write ("<tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name=\"SensitiveMaps\">", sizeof ("<tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name=\"SensitiveMaps\">") - 1);
	if (bFlag) {
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldSensitiveMaps\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldSensitiveMaps\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (iGameOptions & PARTIAL_MAPS) != 0;

		
	Write ("<tr><td>Partial maps:</td><td><select name=\"PartialMaps\">", sizeof ("<tr><td>Partial maps:</td><td><select name=\"PartialMaps\">") - 1);
	if (bFlag) {
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select></td></tr>", sizeof ("</select></td></tr>") - 1);
	iValue = iGameOptions & (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN);

		
	Write ("<tr><td>Display ship menus in planet views:</td><td><select name=\"UpCloseShips\"><option ", sizeof ("<tr><td>Display ship menus in planet views:</td><td><select name=\"UpCloseShips\"><option ") - 1);
	if (iValue == (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN)) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN); 
	Write ("\">Ship menus on both map and planets screens</option><option ", sizeof ("\">Ship menus on both map and planets screens</option><option ") - 1);
	if (iValue == SHIPS_ON_MAP_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_MAP_SCREEN); 
	Write ("\">Ship menus on map screen</option><option ", sizeof ("\">Ship menus on map screen</option><option ") - 1);
	if (iValue == SHIPS_ON_PLANETS_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_PLANETS_SCREEN); 
	Write ("\">Ship menus on planets screen</option><option ", sizeof ("\">Ship menus on planets screen</option><option ") - 1);
	if (iValue == 0) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
		
	Write ("value=\"0\">No ship menus in planet views</option></select></td></tr>", sizeof ("value=\"0\">No ship menus in planet views</option></select></td></tr>") - 1);
	bFlag = (iGameOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;

		
	Write ("<tr><td>Display local maps in up-close map views:</td><td><select name=\"LocalMaps\">", sizeof ("<tr><td>Display local maps in up-close map views:</td><td><select name=\"LocalMaps\">") - 1);
	if (bFlag) {
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select></td></tr><tr><td>Default builder planet:</td><td><select name=\"DefaultBuilderPlanet\">", sizeof ("</select></td></tr><tr><td>Default builder planet:</td><td><select name=\"DefaultBuilderPlanet\">") - 1);
	int iRealPlanet;
		GameCheck (g_pGameEngine->GetEmpireDefaultBuilderPlanet (
			m_iGameClass,
			m_iGameNumber,
			m_iEmpireKey,
			&iValue,
			&iRealPlanet
			));

		
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
	Write ("\">No default builder planet</option>", sizeof ("\">No default builder planet</option>") - 1);
	int* piBuilderKey, iNumBuilderKey;

		GameCheck (g_pGameEngine->GetBuilderPlanetKeys (
			m_iGameClass,
			m_iGameNumber,
			m_iEmpireKey,
			&piBuilderKey,
			&iNumBuilderKey
			));

		if (iValue > 0) {

			String strPlanetName, strFilter;
			Algorithm::AutoDelete<int> autopiBuilderKey (piBuilderKey, true);

			for (i = 0; i < iNumBuilderKey; i ++) {

				iErrCode = g_pGameEngine->GetPlanetNameWithCoordinates (
					m_iGameClass,
					m_iGameNumber,
					piBuilderKey[i],
					&strPlanetName
					);

				if (iErrCode == OK) {

					if (HTMLFilter (strPlanetName.GetCharPtr(), &strFilter, 0, false) == OK) {

						
	Write ("<option ", sizeof ("<option ") - 1);
	if (iValue == piBuilderKey[i]) {
							
	Write ("selected ", sizeof ("selected ") - 1);
	}
						
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (piBuilderKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength()); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
				}
			}
		}

		GameCheck (g_pGameEngine->GetEmpireDefaultMessageTarget (
			m_iGameClass,
			m_iGameNumber,
			m_iEmpireKey,
			&iValue
			));

		
	Write ("</select></td></tr>", sizeof ("</select></td></tr>") - 1);
	if (iGameClassOptions & INDEPENDENCE) {

			
	Write ("<tr><td>Independent ship gifts:</td><td><select name=\"IndependentGifts\"><option", sizeof ("<tr><td>Independent ship gifts:</td><td><select name=\"IndependentGifts\"><option") - 1);
	bFlag = (iGameOptions & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

			if (!bFlag) {
				
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Accept independent ship gifts</option><option", sizeof ("\">Accept independent ship gifts</option><option") - 1);
	if (bFlag) {
				
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">Reject independent ship gifts</option></select></tr>", sizeof ("\">Reject independent ship gifts</option></select></tr>") - 1);
	}

		
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
	Write ("\">All at War</option>", sizeof ("\">All at War</option>") - 1);
	if (iDiplomacy & TRUCE) {

			
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == MESSAGE_TARGET_TRUCE) {
				
	Write (" selected", sizeof (" selected") - 1);
	}
			
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_TRUCE); 
	Write ("\">All at Truce</option>", sizeof ("\">All at Truce</option>") - 1);
	}

		if (iDiplomacy & TRADE) {

			
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == MESSAGE_TARGET_TRADE) {
				
	Write (" selected", sizeof (" selected") - 1);
	}
			
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_TRADE); 
	Write ("\">All at Trade</option>", sizeof ("\">All at Trade</option>") - 1);
	}

		if (iDiplomacy & ALLIANCE) {

			
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == MESSAGE_TARGET_ALLIANCE) {
				
	Write (" selected", sizeof (" selected") - 1);
	}
			
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_ALLIANCE); 
	Write ("\">All at Alliance</option>", sizeof ("\">All at Alliance</option>") - 1);
	}

		
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == MESSAGE_TARGET_LAST_USED) {
			
	Write (" selected", sizeof (" selected") - 1);
	}
		
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_LAST_USED); 
	Write ("\">Last target used</option></select></td></tr><tr><td>Game messages saved:<td valign=\"middle\">", sizeof ("\">Last target used</option></select></td></tr><tr><td>Game messages saved:<td valign=\"middle\">") - 1);
	int iNumMessages;

		GameCheck (g_pGameEngine->GetNumGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumMessages)); 
		if (iNumMessages > 0) {
			Write (iNumMessages);
			
	Write ("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp", sizeof ("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp") - 1);
	WriteButton (BID_VIEWMESSAGES);
		} else {
			
	Write ("None", sizeof ("None") - 1);
	}
		
	Write ("</td></tr><tr><td>Maximum saved game messages</td><td><select name=\"MaxSavedMessages\">", sizeof ("</td></tr><tr><td>Maximum saved game messages</td><td><select name=\"MaxSavedMessages\">") - 1);
	int iMaxNumMessages;
		GameCheck (g_pGameEngine->GetEmpireMaxNumSavedGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumMessages));
		GameCheck (g_pGameEngine->GetSystemLimitOnSavedGameMessages (&iMaxNumMessages));

		for (i = 0; i < iMaxNumMessages; i += 10) {
			
	Write ("<option ", sizeof ("<option ") - 1);
	if (i == iNumMessages) {
				
	Write ("selected ", sizeof ("selected ") - 1);
	}
			
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
		
	Write ("</select><input type=\"hidden\" name=\"OldMaxSavedMessages\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldMaxSavedMessages\" value=\"") - 1);
	Write (iNumMessages); 
		
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (iGameOptions & IGNORE_BROADCASTS) != 0;

		
	Write ("<tr><td>Ignore broadcasts:</td><td><select name=\"Ignore\">", sizeof ("<tr><td>Ignore broadcasts:</td><td><select name=\"Ignore\">") - 1);
	if (bFlag) {
			
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
			
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldIgnore\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldIgnore\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr><tr><td>Search for empires with duplicate IP addresses:</td><td>", sizeof ("\"></td></tr><tr><td>Search for empires with duplicate IP addresses:</td><td>") - 1);
	WriteButton (BID_SEARCHIPADDRESSES);
		
	Write ("</td></tr><tr><td>Search for empires with duplicate Session Ids:</td><td>", sizeof ("</td></tr><tr><td>Search for empires with duplicate Session Ids:</td><td>") - 1);
	WriteButton (BID_SEARCHSESSIONIDS);
		
	Write ("</td></tr><tr><td align=\"top\">Keep game notes here:<br>Update: <input type=\"checkbox\" name=\"UpdateN\" value=\"0\"></td><td><textarea name=\"Notepad\" rows=\"8\" cols=\"50\" wrap=\"physical\">", sizeof ("</td></tr><tr><td align=\"top\">Keep game notes here:<br>Update: <input type=\"checkbox\" name=\"UpdateN\" value=\"0\"></td><td><textarea name=\"Notepad\" rows=\"8\" cols=\"50\" wrap=\"physical\">") - 1);
	Variant vNotepad;
		GameCheck (g_pGameEngine->GetEmpireNotepad (m_iGameClass, m_iGameNumber, m_iEmpireKey, &vNotepad));
		Write (vNotepad.GetCharPtr());
		
	Write ("</textarea></td>", sizeof ("</textarea></td>") - 1);
	if (m_iGameState & STARTED) {

			
	Write ("<tr><td>Resign from the game:</td><td>", sizeof ("<tr><td>Resign from the game:</td><td>") - 1);
	WriteButton (BID_RESIGN);
			
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	if (iErrCode == OK && (iGameClassOptions & USE_SC30_SURRENDERS)) {

				
	Write ("<tr><td>Surrender from the game:</td><td>", sizeof ("<tr><td>Surrender from the game:</td><td>") - 1);
	WriteButton (BID_SURRENDER);
				
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}
		}

		
	Write ("</tr></table><p>", sizeof ("</tr></table><p>") - 1);
	WriteButton (BID_CANCEL);

		}
		break;

	case 1:

		{
		int* piMessageKey, iNumMessages;
		Variant** ppvMessage;
		GameCheck (g_pGameEngine->GetSavedGameMessages (
			m_iGameClass,
			m_iGameNumber,
			m_iEmpireKey,
			&piMessageKey,
			&ppvMessage, 
			&iNumMessages
			));

		
	Write ("<input type=\"hidden\" name=\"OptionPage\" value=\"1\">", sizeof ("<input type=\"hidden\" name=\"OptionPage\" value=\"1\">") - 1);
	if (iNumMessages == 0) {
			
	Write ("<p>You have no saved game messages", sizeof ("<p>You have no saved game messages") - 1);
	} else {

			// Sort
			UTCTime* ptTime = (UTCTime*) StackAlloc (iNumMessages * sizeof (UTCTime));
			int* piKey = (int*) StackAlloc (iNumMessages * sizeof (int));

			for (i = 0; i < iNumMessages; i ++) {
				piKey[i] = i;
				ptTime[i] = ppvMessage[i][2].GetUTCTime();
			}

			Algorithm::QSortTwoDescending<UTCTime, int> (ptTime, piKey, iNumMessages);

			// Display
			String* pstrNameList = new String [iNumMessages];
			if (pstrNameList == NULL) {
				
	Write ("<p>The server is out of memory", sizeof ("<p>The server is out of memory") - 1);
	} else {

				Algorithm::AutoDelete<String> autopstrNameList (pstrNameList, true);

				
	Write ("<p>You have <strong>", sizeof ("<p>You have <strong>") - 1);
	Write (iNumMessages); 
	Write ("</strong> saved game message", sizeof ("</strong> saved game message") - 1);
	if (iNumMessages != 1) { 
					
	Write ("s", sizeof ("s") - 1);
	}
				
	Write (":<p><table width=\"45%\"><input type=\"hidden\" name=\"NumSavedGameMessages\" value=\"", sizeof (":<p><table width=\"45%\"><input type=\"hidden\" name=\"NumSavedGameMessages\" value=\"") - 1);
	Write (iNumMessages); 
	Write ("\">", sizeof ("\">") - 1);
	const char* pszSender, * pszFontColor = NULL;
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
					
	Write ("\"><tr><td><strong>Time: </strong> ", sizeof ("\"><tr><td><strong>Time: </strong> ") - 1);
	iErrCode = Time::GetDateString (ppvMessage[piKey[i]][2], pszDate);
					if (iErrCode != OK) {
						
	Write ("Could not read date", sizeof ("Could not read date") - 1);
	} else {
						Write (pszDate);
					}

					
	Write ("<br><strong>Sender: </strong>", sizeof ("<br><strong>Sender: </strong>") - 1);
	if (String::StrCmp (pszSender, SYSTEM_MESSAGE_SENDER) == 0) { 

						bSystem = bTempSystem = true;
						
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (SYSTEM_MESSAGE_SENDER); 
	Write ("</strong>", sizeof ("</strong>") - 1);
	} else {

						bTempSystem = false;

						
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (pszSender); 
	Write ("</strong>", sizeof ("</strong>") - 1);
	// Find name in lists
						bFlag = false;
						for (j = 0; j < iNumNames; j ++) {
							if (pstrNameList[j].Equals (pszSender)) {
								bFlag = true;
								break;
							}
						}

						// Add name to list if not found
						if (!bFlag) {
							pstrNameList[iNumNames] = pszSender;
							iNumNames ++;
						}
					}

					if (ppvMessage[piKey[i]][3] != 0) {
						
	Write (" (broadcast)", sizeof (" (broadcast)") - 1);
	pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
					} else {
						pszFontColor = m_vPrivateMessageColor.GetCharPtr();
					}

					
	Write ("<br><strong>Delete: </strong><input type=\"checkbox\" name=\"DelChBx", sizeof ("<br><strong>Delete: </strong><input type=\"checkbox\" name=\"DelChBx") - 1);
	Write (i); 
					
	Write ("\"></td></tr><tr><td><font size=\"", sizeof ("\"></td></tr><tr><td><font size=\"") - 1);
	Write (DEFAULT_MESSAGE_FONT_SIZE);
					
	Write ("\" face=\"", sizeof ("\" face=\"") - 1);
	Write (DEFAULT_MESSAGE_FONT); 
	Write ("\"", sizeof ("\"") - 1);
	// Game messages from the system suffer from no special coloring
					if (!bTempSystem) {
						
	Write (" color=\"#", sizeof (" color=\"#") - 1);
	Write (pszFontColor); 
	Write ("\"", sizeof ("\"") - 1);
	}
					
	Write (">", sizeof (">") - 1);
	WriteFormattedMessage (ppvMessage[piKey[i]][0].GetCharPtr());

					
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

				g_pGameEngine->FreeData (ppvMessage);
				g_pGameEngine->FreeKeys (piMessageKey);
			}
		}

		break;
		}

	default:

		Assert (false);
	}

	GAME_CLOSE


}