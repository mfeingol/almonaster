
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the PersonalGameClasses page
int HtmlRenderer::Render_PersonalGameClasses() {

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

	IHttpForm* pHttpForm;

	// Make sure that the unprivileged don't abuse this:
	if (m_iPrivilege < ADEPT) {
		AddMessage ("You are not authorized to view this page");
		return Redirect (LOGIN);
	}

	int i, iErrCode, iPersonalGameClassesPage = 0, iGameClassKey = NO_KEY;

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

		if (WasButtonPressed (BID_CANCEL)) {
			bRedirectTest = false;
		} else {

			int iPersonalGameClassesPageSubmit;

			if ((pHttpForm = m_pHttpRequest->GetForm ("PersonalGameClassesPage")) == NULL) {
				goto Redirection;
			}
			iPersonalGameClassesPageSubmit = pHttpForm->GetIntValue();

			int iGameNumber;
			bool bFlag = false;

			const char* pszStart;

			switch (iPersonalGameClassesPageSubmit) {

			case 0:

				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("DeleteGameClass")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "DeleteGameClass%d", &iGameClassKey) == 1) {

					int iOwnerKey;
					iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
					if (iErrCode == OK) {

						if (m_iEmpireKey == iOwnerKey ||
							(m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
							) {

							iErrCode = g_pGameEngine->DeleteGameClass (iGameClassKey, &bFlag);

							if (iErrCode == OK) {
								if (bFlag) {
									AddMessage ("The GameClass was deleted");
								} else {
									AddMessage ("The GameClass has been marked for deletion");
								}
							}
							else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
								AddMessage ("The GameClass no longer exists");
							}
							else {
								AddMessage ("The gameclass could not be deleted; the error was ");
								AppendMessage (iErrCode);
							}
						}
					}

					bRedirectTest = false;
					goto Redirection;
				}

				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UndeleteGameClass")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "UndeleteGameClass%d", &iGameClassKey) == 1) {

					int iOwnerKey;
					iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
					if (iErrCode == OK) {

						if (m_iEmpireKey == iOwnerKey ||
							(m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
							) {

							iErrCode = g_pGameEngine->UndeleteGameClass (iGameClassKey);
							switch (iErrCode) {

							case OK:

								AddMessage ("The GameClass was undeleted");
								break;

							case ERROR_GAMECLASS_DOES_NOT_EXIST:

								AddMessage ("The GameClass no longer exists");
								break;

							case ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION:

								AddMessage ("The GameClass was not marked for deletion");
								break;

							default:

								AddMessage ("The gameclass could not be undeleted; the error was ");
								AppendMessage (iErrCode);
								break;
							}
						}
					}

					bRedirectTest = false;
					goto Redirection;
				}

				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("HaltGameClass")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "HaltGameClass%d", &iGameClassKey) == 1) {

					int iOwnerKey;
					iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
					if (iErrCode == OK) {

						if (m_iEmpireKey == iOwnerKey ||
							(m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
							) {

							iErrCode = g_pGameEngine->HaltGameClass (iGameClassKey);

							if (iErrCode == OK) {
								AddMessage ("The GameClass was halted");
							}
							else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
								AddMessage ("The GameClass no longer exists");
							}
							else {
								AddMessage ("The gameclass could not be halted; the error was ");
								AppendMessage (iErrCode);
							}
						}
					}

					bRedirectTest = false;
					goto Redirection;
				}

				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UnhaltGameClass")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "UnhaltGameClass%d", &iGameClassKey) == 1) {

					int iOwnerKey;
					iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
					if (iErrCode == OK) {

						if (m_iEmpireKey == iOwnerKey ||
							(m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
							) {

							iErrCode = g_pGameEngine->UnhaltGameClass (iGameClassKey);
							switch (iErrCode) {

							case OK:

								AddMessage ("The GameClass was unhalted");
								break;

							case ERROR_GAMECLASS_DOES_NOT_EXIST:

								AddMessage ("The GameClass no longer exists");
								break;

							case ERROR_GAMECLASS_NOT_HALTED:

								AddMessage ("The GameClass was not halted");
								break;

							default:

								AddMessage ("The gameclass could not be unhalted; the error was ");
								AppendMessage (iErrCode);
								break;
							}
						}
					}

					bRedirectTest = false;
					goto Redirection;
				}

				// Handle game start
				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

					bRedirectTest = false;

					// Check for advanced
					char pszAdvanced [64];
					sprintf (pszAdvanced, "Advanced%i", iGameClassKey);

					if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
						iPersonalGameClassesPage = 1;
						break;
					}

					GameOptions goOptions;
					iErrCode = g_pGameEngine->GetDefaultGameOptions (iGameClassKey, &goOptions);
					if (iErrCode != OK) {

						if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
							AddMessage ("That gameclass no longer exists");
						} else {
							AddMessage ("Could not read default game options; the error was ");
							AppendMessage (iErrCode);
						}
						goto Redirection;
					}

					// Create the game
					iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

					HANDLE_CREATE_GAME_OUTPUT (iErrCode);
				}

				// Handle new gameclass creation
				if (m_iPrivilege >= ADEPT && 
					WasButtonPressed (BID_CREATENEWGAMECLASS)) {

					iErrCode = ProcessCreateGameClassForms (m_iEmpireKey);

					bRedirectTest = false;
					goto Redirection;
				}

				break;

			case 1:

				// Check for choose
				if (WasButtonPressed (BID_STARTGAME) || WasButtonPressed (BID_BLOCK)) {

					bRedirectTest = false;

					if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
						iPersonalGameClassesPage = 0;
						break;
					}
					iGameClassKey = pHttpForm->GetIntValue();

					GameOptions goOptions;
					InitGameOptions (&goOptions);

					iErrCode = ParseGameConfigurationForms (iGameClassKey, NULL, m_iEmpireKey, &goOptions);
					if (iErrCode != OK) {
						iPersonalGameClassesPage = 1;
						break;
					}

					// Create the game
					iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

					ClearGameOptions (&goOptions);

					HANDLE_CREATE_GAME_OUTPUT (iErrCode);
				}

				break;

			default:

				Assert (false);
				break;
			}
		}
	}

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)

	switch (iPersonalGameClassesPage) {

	case 0:

		{

		
	Write ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"0\">") - 1);
	int iNumGameClasses;
		int* piGameClassKey;

		iErrCode = g_pGameEngine->GetEmpireGameClassKeys (m_iEmpireKey, &piGameClassKey, &iNumGameClasses);

		if (iErrCode != OK) {
			
	Write ("<p><h3>Your Personal GameClasses could not be read from the database. The error was ", sizeof ("<p><h3>Your Personal GameClasses could not be read from the database. The error was ") - 1);
	Write (iErrCode); 
	Write ("</h3>", sizeof ("</h3>") - 1);
	}

		else if (iNumGameClasses == 0) {
			
	Write ("<p><h3>You have no Personal GameClasses</h3>", sizeof ("<p><h3>You have no Personal GameClasses</h3>") - 1);
	}

		else {

			
	Write ("<p><h3>Start a new game:</h3>", sizeof ("<p><h3>Start a new game:</h3>") - 1);
	Variant* pvGameClassInfo = NULL;

			WriteSystemGameListHeader (m_vTableColor.GetCharPtr());
			for (i = 0; i < iNumGameClasses; i ++) {

				// Read game class data
				if (g_pGameEngine->GetGameClassData (piGameClassKey[i], &pvGameClassInfo) == OK) {

					// Best effort
					iErrCode = WriteSystemGameListData (
						piGameClassKey[i], 
						pvGameClassInfo
						);
				}

				if (pvGameClassInfo != NULL) {
					g_pGameEngine->FreeData (pvGameClassInfo);
					pvGameClassInfo = NULL;
				}
			}

			
	Write ("</table>", sizeof ("</table>") - 1);
	g_pGameEngine->FreeKeys (piGameClassKey);
		}

		int iMaxNumPGC;
		iErrCode = g_pGameEngine->GetMaxNumPersonalGameClasses (&iMaxNumPGC);

		if (iErrCode == OK && iNumGameClasses < iMaxNumPGC) {
			
	Write ("<p><h3>Create a new GameClass:</h3>", sizeof ("<p><h3>Create a new GameClass:</h3>") - 1);
	WriteCreateGameClassString (m_iEmpireKey, false);
			
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CREATENEWGAMECLASS);
		}

		}
		break;

	case 1:

		int iGameNumber;
		char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

		Check (g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName));
		Check (g_pGameEngine->GetNextGameNumber (iGameClassKey, &iGameNumber));

		
	Write ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"1\"><input type=\"hidden\" name=\"GameClassKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"1\"><input type=\"hidden\" name=\"GameClassKey\" value=\"") - 1);
	Write (iGameClassKey); 
	Write ("\"><h3>Advanced game creation options<p>", sizeof ("\"><h3>Advanced game creation options<p>") - 1);
	Write (pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (iGameNumber);

		
	Write ("</h3><p>", sizeof ("</h3><p>") - 1);
	RenderGameConfiguration (iGameClassKey);

		
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);
		WriteButton (BID_STARTGAME);

		break;

	default:

		Assert (false);
		break;
	}

	SYSTEM_CLOSE


}