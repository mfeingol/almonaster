
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "Osal/Algorithm.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the ProfileViewer page
int HtmlRenderer::Render_ProfileViewer() {

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

	int iErrCode, iProfileViewerPage = 0, iTargetEmpireKey = NO_KEY,
		iNumSearchColumns = 0, * piSearchEmpireKey = NULL, iLastKey = 0, iMaxNumHits = 0,
		iNumSearchEmpires = 0, iGameClassKey = NO_KEY;

	unsigned int piSearchColName [MAX_NUM_SEARCH_COLUMNS];
	Variant pvSearchColData1 [MAX_NUM_SEARCH_COLUMNS];
	Variant pvSearchColData2 [MAX_NUM_SEARCH_COLUMNS];

	const char* ppszFormName [MAX_NUM_SEARCH_COLUMNS];
	const char* ppszColName1 [MAX_NUM_SEARCH_COLUMNS];
	const char* ppszColName2 [MAX_NUM_SEARCH_COLUMNS];

	if (m_iReserved != NO_KEY) {
		iTargetEmpireKey = m_iReserved;
		iProfileViewerPage = 2;
	}

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

		if (!WasButtonPressed (BID_CANCEL)) {

			if ((pHttpForm = m_pHttpRequest->GetForm ("ProfileViewerPage")) == NULL) {
				goto Redirection;
			}
			int iProfileViewerPageSubmit = pHttpForm->GetIntValue();

			switch (iProfileViewerPageSubmit) {

			case 0:
				{

				if (WasButtonPressed (BID_SEARCH)) {

	SearchResults:

					iErrCode = HandleSearchSubmission (
						piSearchColName, 
						pvSearchColData1,
						pvSearchColData2,
						ppszFormName,
						ppszColName1,
						ppszColName2,

						&iNumSearchColumns,

						&piSearchEmpireKey,
						&iNumSearchEmpires,
						&iLastKey,
						&iMaxNumHits
						);


					switch (iErrCode) {

					case OK:
					case ERROR_TOO_MANY_HITS:

						if (iNumSearchEmpires > 0) {

							if (iNumSearchEmpires == 1 && iLastKey == NO_KEY) {
								iTargetEmpireKey = piSearchEmpireKey[0];
								g_pGameEngine->FreeKeys (piSearchEmpireKey);
								iProfileViewerPage = 2;
							} else {
								iProfileViewerPage = 1;
							}
							break;
						}

						// Fall through if 0 empires

					case ERROR_DATA_NOT_FOUND:

						AddMessage ("No empires matched your search criteria");
						break;

					case ERROR_INVALID_ARGUMENT:

						// A form was missing
						goto Redirection;

					case ERROR_INVALID_QUERY:

						AddMessage ("You submitted an invalid query");
						goto Redirection;

					default:

						{
						char pszMessage [64];
						sprintf (pszMessage, "Error %i occurred", iErrCode);
						AddMessage (pszMessage);
						}
						break;
					}
				}

				}

				break;

			case 1:
				{

				unsigned int iHash;
				const char* pszStart;
				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewProfile")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "ViewProfile.%d.%d", &iTargetEmpireKey, &iHash) == 2) {

					if (!VerifyEmpireNameHash (iTargetEmpireKey, iHash)) {
						AddMessage ("That empire no longer exists");
					} else {
						iProfileViewerPage = 2;
					}

					bRedirectTest = false;
				}

				if (WasButtonPressed (BID_SEARCH)) {
					goto SearchResults;
				}

				}

				break;

			case 2:

				{

				// Get key
				if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpireKey")) == NULL) {
					goto Redirection;
				}
				const char* pszTargetEmpire = pHttpForm->GetValue();
				iTargetEmpireKey = pHttpForm->GetIntValue();

				// Send messages
				if (WasButtonPressed (BID_SENDMESSAGE)) {

					if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL) {
						goto Redirection;
					}
					const char* pszMessage = pHttpForm->GetValue();

					Variant vSentName;
					if (pszMessage != NULL && pszTargetEmpire != NULL) {

						iErrCode = g_pGameEngine->SendSystemMessage (iTargetEmpireKey, pszMessage, m_iEmpireKey);
						switch (iErrCode) {

						case OK:
							Check (g_pGameEngine->GetEmpireName (iTargetEmpireKey, &vSentName));
							AddMessage ("Your message was sent to ");
							AppendMessage (vSentName.GetCharPtr());
							break;

						case ERROR_CANNOT_SEND_MESSAGE:

							AddMessage ("You are not allowed to send system messages");
							break;

						case ERROR_EMPIRE_DOES_NOT_EXIST:

							AddMessage ("That empire no longer exists");
							break;

						default:
							AddMessage ("Your message could not be sent due to error ");
							AppendMessage (iErrCode);
							break;
						}

					} else {

						AddMessage ("Your message was blank");
					}

					bRedirectTest = false;
					iProfileViewerPage = 2;
					break;
				}

				// View PGC
				if (WasButtonPressed (BID_VIEWEMPIRESGAMECLASSES)) {
					bRedirectTest = false;
					iProfileViewerPage = 4;
					break;
				}

				// View nuke history
				if (WasButtonPressed (BID_VIEWEMPIRESNUKEHISTORY)) {
					bRedirectTest = false;
					iProfileViewerPage = 5;
					break;
				}

				}

				break;

			case 4:
				{

				int iGameNumber;
				bool bFlag = false;

				const char* pszStart;
				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

					GameOptions goOptions;

					bRedirectTest = false;

					// Check for advanced
					char pszAdvanced [128];
					sprintf (pszAdvanced, "Advanced%i", iGameClassKey);

					if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
						iProfileViewerPage = 6;
						break;
					}

					iErrCode = g_pGameEngine->GetDefaultGameOptions (iGameClassKey, &goOptions);
					if (iErrCode != OK) {
						AddMessage ("Could not read default game options");
						goto Redirection;
					}

					// Create the game
					iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

					HANDLE_CREATE_GAME_OUTPUT (iErrCode);
				}

				// Test for gameclass deletions and undeletions
				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("DeleteGameClass")) != NULL && 
					(pszStart = pHttpForm->GetName()) != NULL &&
					sscanf (pszStart, "DeleteGameClass%d", &iGameClassKey) == 1) {

					bRedirectTest = false;

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
								AddMessage ("An error occurred deleting the gameclass: ");
								AppendMessage (iErrCode);
							}
						}
					}
				}

				else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UndeleteGameClass")) != NULL && 
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

								AddMessage ("An error occurred undeleting the gameclass: ");
								AppendMessage (iErrCode);
								break;
							}
						}
					}

					bRedirectTest = false;
				}

				else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("HaltGameClass")) != NULL && 
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
								AddMessage ("An error occurred halting the gameclass: ");
								AppendMessage (iErrCode);
							}
						}
					}

					bRedirectTest = false;
				}

				else if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("UnhaltGameClass")) != NULL && 
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

								AddMessage ("An error occurred unhalting the gameclass: ");
								AppendMessage (iErrCode);
								break;
							}
						}
					}

					bRedirectTest = false;
				}

				}
				break;

			case 5:

				{

				}
				break;

			case 6:

				// Check for choose
				if (WasButtonPressed (BID_STARTGAME) || WasButtonPressed (BID_BLOCK)) {

					int iGameNumber;

					GameOptions goOptions;
					InitGameOptions (&goOptions);

					bRedirectTest = false;

					if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
						iProfileViewerPage = 0;
						break;
					}
					iGameClassKey = pHttpForm->GetIntValue();

					iErrCode = ParseGameConfigurationForms (iGameClassKey, NULL, m_iEmpireKey, &goOptions);
					if (iErrCode != OK) {
						iProfileViewerPage = 6;
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
			}

		} else {
			bRedirectTest = false;
		}
	} 

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)

	// Individual page stuff starts here
	switch (iProfileViewerPage) {

	case 1:
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"1\">", sizeof ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"1\">") - 1);
	Assert (piSearchEmpireKey != NULL);

		RenderSearchResults (
			piSearchColName,
			pvSearchColData1,
			pvSearchColData2,
			ppszFormName,
			ppszColName1,
			ppszColName2,
			iNumSearchColumns,
			piSearchEmpireKey,
			iNumSearchEmpires,
			iLastKey,
			iMaxNumHits
			);

		g_pGameEngine->FreeKeys (piSearchEmpireKey);

		}

		break;

	case 0:
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"0\">") - 1);
	bool bShowAdvanced = m_iPrivilege >= APPRENTICE;
		if (bShowAdvanced) {
			Check (g_pGameEngine->GetEmpireOption (m_iEmpireKey, SHOW_ADVANCED_SEARCH_INTERFACE, &bShowAdvanced));
		}

		Check (RenderSearchForms (bShowAdvanced));

		}

		break;

	case 2:
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"2\">", sizeof ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"2\">") - 1);
	WriteProfile (iTargetEmpireKey); 
		
	Write ("<p>", sizeof ("<p>") - 1);
	}

		break;

	case 4:
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"4\">", sizeof ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"4\">") - 1);
	WritePersonalGameClasses (iTargetEmpireKey);

		}

		break;

	case 5:
		{

		
	Write ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"5\">", sizeof ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"5\">") - 1);
	Assert (iTargetEmpireKey != NO_KEY);
		WriteNukeHistory (iTargetEmpireKey);

		}

		break;

	case 6:

		int iGameNumber;
		char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

		Check (g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName));
		Check (g_pGameEngine->GetNextGameNumber (iGameClassKey, &iGameNumber));

		
	Write ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"6\"><input type=\"hidden\" name=\"GameClassKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"ProfileViewerPage\" value=\"6\"><input type=\"hidden\" name=\"GameClassKey\" value=\"") - 1);
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
	}

	SYSTEM_CLOSE


}