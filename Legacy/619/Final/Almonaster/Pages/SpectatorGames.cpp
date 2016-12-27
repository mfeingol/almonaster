
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the SpectatorGames page
int HtmlRenderer::Render_SpectatorGames() {

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

	int i, iErrCode, iSpectatorGamesPage = 0, iGameClassKey = NO_KEY, iGameNumber = NO_KEY,
		iClickedPlanetKey = NO_KEY, iClickedProxyPlanetKey = NO_KEY;

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

		if ((pHttpForm = m_pHttpRequest->GetForm ("SpectSubPage")) == NULL) {
			goto Redirection;
		} else {

			IHttpForm* pHttpFormGame = NULL;
			const char* pszSpectator = NULL;

			if ((pHttpFormGame = m_pHttpRequest->GetForm ("GameClassKey")) != NULL) {
				iGameClassKey = pHttpFormGame->GetIntValue();
			}

			if ((pHttpFormGame = m_pHttpRequest->GetForm ("GameNumber")) != NULL) {
				iGameNumber = pHttpFormGame->GetIntValue();
			}

			switch (pHttpForm->GetIntValue()) {

			case 0:

				{

				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Spectate")) != NULL && 
					(pszSpectator = pHttpForm->GetName()) != NULL &&
					sscanf (pszSpectator, "Spectate%d.%d", &iGameClassKey, &iGameNumber) == 2) {

					iSpectatorGamesPage = 1;
					bRedirectTest = false;
				}

				}
				break;

			case 1:

				// Planet click
				if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
					(pszSpectator = pHttpForm->GetName()) != NULL &&
					sscanf (pszSpectator, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2 &&
					iGameClassKey != NO_KEY && iGameNumber != NO_KEY
					) {

					iSpectatorGamesPage = 2;
					bRedirectTest = false;
				}

				// View Empire Information
				if (WasButtonPressed (BID_VIEWEMPIREINFORMATION) &&
					iGameClassKey != NO_KEY && iGameNumber != NO_KEY) {
					iSpectatorGamesPage = 3;
					bRedirectTest = false;
				}

				break;

			case 2:

				// View map
				if (WasButtonPressed (BID_VIEWMAP) &&
					iGameClassKey != NO_KEY && iGameNumber != NO_KEY) {
					iSpectatorGamesPage = 1;
					bRedirectTest = false;
				}

				// View Empire Information
				if (WasButtonPressed (BID_VIEWEMPIREINFORMATION) &&
					iGameClassKey != NO_KEY && iGameNumber != NO_KEY) {
					iSpectatorGamesPage = 3;
					bRedirectTest = false;
				}

				break;

			case 3:

				// View map
				if (WasButtonPressed (BID_VIEWMAP) &&
					iGameClassKey != NO_KEY && iGameNumber != NO_KEY) {
					iSpectatorGamesPage = 1;
					bRedirectTest = false;
				}

				break;

			default:
				break;
			}
		}
	}

	SYSTEM_REDIRECT_ON_SUBMIT

	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH] = "";

	if (iSpectatorGamesPage > 0) {

		bool bFlag;

		if (g_pGameEngine->WaitGameReader (iGameClassKey, iGameNumber) != OK) {

			AddMessage ("That game no longer exists");
			iSpectatorGamesPage = 0;

		} else {

			// Make sure game exists
			iErrCode = g_pGameEngine->DoesGameExist (iGameClassKey, iGameNumber, &bFlag);
			if (iErrCode != OK || !bFlag) {

				g_pGameEngine->SignalGameReader (iGameClassKey, iGameNumber);

				AddMessage ("That game no longer exists");
				iSpectatorGamesPage = 0;

			} else {

				iErrCode = g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName);
				if (iErrCode != OK) {
					pszGameClassName[0] = '\0';
				}
			}
		}
	}

	SYSTEM_OPEN (false)

	switch (iSpectatorGamesPage) {

	case 0:

		{

		
	Write ("<input type=\"hidden\" name=\"SpectSubPage\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"SpectSubPage\" value=\"0\">") - 1);
	// Get open games
		int iNumClosedGames = 0, * piGameClass, * piGameNumber;

		iErrCode = g_pGameEngine->GetClosedGames (&piGameClass, &piGameNumber, &iNumClosedGames);
		if (iErrCode != OK) {
			
	Write ("<h3>The list of spectator games could not be read. The error was ", sizeof ("<h3>The list of spectator games could not be read. The error was ") - 1);
	Write (iErrCode); 
	Write ("</h3>", sizeof ("</h3>") - 1);
	}

		else if (iNumClosedGames == 0) {
			
	Write ("<h3>There are no closed spectator games on this server</h3>", sizeof ("<h3>There are no closed spectator games on this server</h3>") - 1);
	}

		else {

			// Update the open games
			int j, iGameClass, iGameNumber;

			int* piSuperClassKey, iNumSuperClasses;
			bool bDraw = false;
			Check (g_pGameEngine->GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses));

			if (iNumSuperClasses == 0) {
				
	Write ("<h3>There are no spectator games on this server</h3>", sizeof ("<h3>There are no spectator games on this server</h3>") - 1);
	} else {

				// Create the game table
				int** ppiTable = (int**) StackAlloc ((iNumSuperClasses + 1) * 3 * sizeof (int*));
				int** ppiGameClass = ppiTable + iNumSuperClasses + 1;
				int** ppiGameNumber = ppiGameClass + iNumSuperClasses + 1;

				// Create the game
				for (i = 0; i < iNumSuperClasses + 1; i ++) {
					ppiTable[i] = (int*) StackAlloc ((iNumClosedGames + 1) * 3 * sizeof (int));
					ppiTable[i][iNumClosedGames] = 0;

					ppiGameClass[i] = ppiTable[i] + iNumClosedGames + 1;
					ppiGameNumber[i] = ppiGameClass[i] + iNumClosedGames + 1;
				}

				// Fill in the table
				int iSuperClassKey;
				bool bFlag;

				for (i = 0; i < iNumClosedGames; i ++) {

					iGameClass = piGameClass[i];
					iGameNumber = piGameNumber[i];

					// Check everything
					if (g_pGameEngine->CheckGameForUpdates (iGameClass, iGameNumber, &bFlag) == OK &&
						g_pGameEngine->DoesGameExist (iGameClass, iGameNumber, &bFlag) == OK && bFlag &&
						g_pGameEngine->IsSpectatorGame (iGameClass, iGameNumber, &bFlag) == OK && bFlag &&
						g_pGameEngine->GetGameClassSuperClassKey (iGameClass, &iSuperClassKey) == OK) {

						for (j = 0; j < iNumSuperClasses; j ++) {
							if (piSuperClassKey[j] == iSuperClassKey) {

								// We found a match, so write down the game in question
								ppiTable [j][ppiTable [j][iNumClosedGames]] = i;

								ppiGameClass[j][ppiTable [j][iNumClosedGames]] = iGameClass;
								ppiGameNumber[j][ppiTable [j][iNumClosedGames]] = iGameNumber;

								ppiTable [j][iNumClosedGames] ++;
								bDraw = true;
								break;
							}
						}

						if (j == iNumSuperClasses) {

							// No superclass was found, so it must be a personal game
							ppiTable [iNumSuperClasses][ppiTable [iNumSuperClasses][iNumClosedGames]] = i;

							ppiGameClass[iNumSuperClasses][ppiTable [iNumSuperClasses][iNumClosedGames]] = iGameClass;
							ppiGameNumber[iNumSuperClasses][ppiTable [iNumSuperClasses][iNumClosedGames]] = iGameNumber;

							ppiTable [iNumSuperClasses][iNumClosedGames] ++;
							bDraw = true;
						}
					}
				}

				if (!bDraw) {
					
	Write ("<h3>There are no spectator games available</h3>", sizeof ("<h3>There are no spectator games available</h3>") - 1);
	} else {

					
	Write ("<p><h3>View a spectator game:</h3>", sizeof ("<p><h3>View a spectator game:</h3>") - 1);
	Variant* pvGameClassInfo = NULL;
					int iBegin, iCurrentGameClass, iNumGamesInSuperClass, iNumToSort;

					if (ppiTable [iNumSuperClasses][iNumClosedGames] > 0) {

						
	Write ("<p><h3>Personal Games:</h3>", sizeof ("<p><h3>Personal Games:</h3>") - 1);
	WriteSpectatorGameListHeader (m_vTableColor.GetCharPtr());

						iNumGamesInSuperClass = ppiTable[iNumSuperClasses][iNumClosedGames];

						// Sort games by gameclass
						Algorithm::QSortThreeAscending<int, int, int> (
							ppiGameClass[iNumSuperClasses],
							ppiGameNumber[iNumSuperClasses],
							ppiTable[iNumSuperClasses],
							iNumGamesInSuperClass
							);

						// Sort games by gamenumber
						iBegin = 0;
						iCurrentGameClass = ppiGameClass[iNumSuperClasses][0];

						for (j = 1; j < iNumGamesInSuperClass; j ++) {

							if (ppiGameClass[iNumSuperClasses][j] != iCurrentGameClass) {

								iNumToSort = j - iBegin;
								if (iNumToSort > 1) {
									Algorithm::QSortThreeAscending<int, int, int> (
										ppiGameNumber[iNumSuperClasses] + iBegin,
										ppiGameClass[iNumSuperClasses] + iBegin,
										ppiTable[iNumSuperClasses] + iBegin,
										iNumToSort
										);
								}

								iBegin = j;
								iCurrentGameClass = ppiGameClass[iNumSuperClasses][j];
							}
						}

						iNumToSort = j - iBegin;
						if (iNumToSort > 1) {

							Algorithm::QSortThreeAscending<int, int, int> (
								ppiGameNumber[iNumSuperClasses] + iBegin,
								ppiGameClass[iNumSuperClasses] + iBegin,
								ppiTable[iNumSuperClasses] + iBegin,
								iNumToSort
								);
						}

						for (j = 0; j < ppiTable[iNumSuperClasses][iNumClosedGames]; j ++) {

							iGameClass = ppiGameClass[iNumSuperClasses][j];
							iGameNumber = ppiGameNumber[iNumSuperClasses][j];

							if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

								// Best effort
								iErrCode = WriteSpectatorGameListData (iGameClass, iGameNumber, pvGameClassInfo);
							}

							if (pvGameClassInfo != NULL) {
								g_pGameEngine->FreeData (pvGameClassInfo);
								pvGameClassInfo = NULL;
							}
						}

						
	Write ("</table>", sizeof ("</table>") - 1);
	}

					Variant vName;
					for (i = 0; i < iNumSuperClasses; i ++) {

						if (ppiTable [i][iNumClosedGames] > 0 && 
							g_pGameEngine->GetSuperClassName (piSuperClassKey[i], &vName) == OK) {

							
	Write ("<p><h3>", sizeof ("<p><h3>") - 1);
	Write (vName.GetCharPtr()); 
	Write (":</h3>", sizeof (":</h3>") - 1);
	WriteSpectatorGameListHeader (m_vTableColor.GetCharPtr());

							iNumGamesInSuperClass = ppiTable[i][iNumClosedGames];

							// Sort games by gameclass
							Algorithm::QSortThreeAscending<int, int, int> (
								ppiGameClass[i],
								ppiGameNumber[i],
								ppiTable[i],
								iNumGamesInSuperClass
								);

							// Sort games by gamenumber
							iBegin = 0;
							iCurrentGameClass = ppiGameClass[i][0];

							for (j = 1; j < iNumGamesInSuperClass; j ++) {

								if (ppiGameClass[i][j] != iCurrentGameClass) {

									iNumToSort = j - iBegin;
									if (iNumToSort > 1) {
										Algorithm::QSortThreeAscending<int, int, int> (
											ppiGameNumber[i] + iBegin,
											ppiGameClass[i] + iBegin,
											ppiTable[i] + iBegin,
											iNumToSort
											);
									}

									iBegin = j;
									iCurrentGameClass = ppiGameClass[i][j];
								}
							}

							iNumToSort = j - iBegin;
							if (iNumToSort > 1) {

								Algorithm::QSortThreeAscending<int, int, int> (
									ppiGameNumber[i] + iBegin,
									ppiGameClass[i] + iBegin,
									ppiTable[i] + iBegin,
									iNumToSort
									);
							}

							for (j = 0; j < iNumGamesInSuperClass; j ++) {

								iGameClass = ppiGameClass[i][j];
								iGameNumber = ppiGameNumber[i][j];

								if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

									WriteSpectatorGameListData (
										iGameClass, 
										iGameNumber, 
										pvGameClassInfo
										);
								}

								if (pvGameClassInfo != NULL) {
									g_pGameEngine->FreeData (pvGameClassInfo);
									pvGameClassInfo = NULL;
								}
							}

							
	Write ("</table>", sizeof ("</table>") - 1);
	}
					}
				}

				g_pGameEngine->FreeKeys (piSuperClassKey);
			}

			delete [] piGameClass;
			delete [] piGameNumber;
		}

		}
		break;

	case 1:

		
	Write ("<input type=\"hidden\" name=\"SpectSubPage\" value=\"1\">", sizeof ("<input type=\"hidden\" name=\"SpectSubPage\" value=\"1\">") - 1);
	bool bTrue;

		iErrCode = g_pGameEngine->IsSpectatorGame (iGameClassKey, iGameNumber, &bTrue);
		if (iErrCode != OK || !bTrue) {
			
	Write ("<h3>That game is not available to spectators</h3>", sizeof ("<h3>That game is not available to spectators</h3>") - 1);
	} else {

			
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_VIEWEMPIREINFORMATION);

			
	Write ("<h3><p>Spectator map from ", sizeof ("<h3><p>Spectator map from ") - 1);
	Write (pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (iGameNumber); 
	Write (":</h3>", sizeof (":</h3>") - 1);
	iErrCode = RenderMap (
				iGameClassKey,
				iGameNumber,
				m_iEmpireKey,
				false,
				NULL,
				true
				);

			if (iErrCode == ERROR_NO_PLANETS_AVAILABLE) {
				
	Write ("<h3>The game has no planets available to spectators</h3>", sizeof ("<h3>The game has no planets available to spectators</h3>") - 1);
	}

			else if (iErrCode != OK) {
				
	Write ("<h3>Error ", sizeof ("<h3>Error ") - 1);
	Write (iErrCode); 
	Write (" rendering game map</h3>", sizeof (" rendering game map</h3>") - 1);
	}

			
	Write ("<input type=\"hidden\" name=\"GameClassKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"GameClassKey\" value=\"") - 1);
	Write (iGameClassKey); 
	Write ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"") - 1);
	Write (iGameNumber); 
	Write ("\">", sizeof ("\">") - 1);
	}

		g_pGameEngine->SignalGameReader (iGameClassKey, iGameNumber);

		break;

	case 2:

		
	Write ("<input type=\"hidden\" name=\"SpectSubPage\" value=\"2\"><input type=\"hidden\" name=\"GameClassKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"SpectSubPage\" value=\"2\"><input type=\"hidden\" name=\"GameClassKey\" value=\"") - 1);
	Write (iGameClassKey); 
	Write ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"") - 1);
	Write (iGameNumber); 
	Write ("\">", sizeof ("\">") - 1);
	{

		IDatabase* pDatabase = NULL;
		IReadTable* pGameMap = NULL;
		void** ppPlanetData = NULL;

		int iGameClassOptions, iLivePlanetKey, iDeadPlanetKey, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;
		bool bTrue;

		GAME_MAP (pszGameMap, iGameClassKey, iGameNumber);

		iErrCode = g_pGameEngine->IsSpectatorGame (iGameClassKey, iGameNumber, &bTrue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (!bTrue) {
			
	Write ("<h3>That game is not available to spectators</h3>", sizeof ("<h3>That game is not available to spectators</h3>") - 1);
	goto Cleanup;
		}

		iErrCode = g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		iErrCode = g_pGameEngine->GetGameClassOptions (iGameClassKey, &iGameClassOptions);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		iErrCode = GetGoodBadResourceLimits (
			iGameClassKey,
			iGameNumber,
			&iGoodAg,
			&iBadAg,
			&iGoodMin,
			&iBadMin,
			&iGoodFuel,
			&iBadFuel
			);

		if (iErrCode != OK) {
			goto Cleanup;
		}

		pDatabase = g_pGameEngine->GetDatabase();
		Assert (pDatabase);

		iErrCode = pDatabase->GetTableForReading (pszGameMap, &pGameMap);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		iErrCode = pGameMap->ReadRow (iClickedPlanetKey, &ppPlanetData);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_VIEWEMPIREINFORMATION);
		WriteButton (BID_VIEWMAP);

		
	Write ("<p><h3>Spectator planet from ", sizeof ("<p><h3>Spectator planet from ") - 1);
	Write (pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (iGameNumber); 
	Write (":</h3><p><table width=\"90%\">", sizeof (":</h3><p><table width=\"90%\">") - 1);
	m_iGameState |= STARTED;
		m_iGameClass = iGameClassKey;
		m_iGameNumber = iGameNumber;

		iErrCode = WriteUpClosePlanetString (
			NO_KEY,
			iClickedPlanetKey,
			iClickedProxyPlanetKey,
			iLivePlanetKey,
			iDeadPlanetKey,
			0,
			(iGameClassOptions & VISIBLE_BUILDS) != 0,
			iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel,
			(float) 0.0, 
			(iGameClassOptions & INDEPENDENCE) != 0, 
			false,
			true,
			ppPlanetData,
			&bTrue
			);

		g_pGameEngine->SignalGameReader (iGameClassKey, iGameNumber);

		
	Write ("</table>", sizeof ("</table>") - 1);
	Cleanup:

		if (ppPlanetData != NULL) {
			pDatabase->FreeData (ppPlanetData);
		}

		if (pGameMap != NULL) {
			pGameMap->Release();
		}

		if (pDatabase != NULL) {
			pDatabase->Release();
		}

		if (iErrCode != OK) {
			
	Write ("<h3>Error ", sizeof ("<h3>Error ") - 1);
	Write (iErrCode); 
	Write (" rendering page</h3>", sizeof (" rendering page</h3>") - 1);
	}

		}
		break;

	case 3:

		
	Write ("<input type=\"hidden\" name=\"SpectSubPage\" value=\"3\"><input type=\"hidden\" name=\"GameClassKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"SpectSubPage\" value=\"3\"><input type=\"hidden\" name=\"GameClassKey\" value=\"") - 1);
	Write (iGameClassKey); 
	Write ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"") - 1);
	Write (iGameNumber); 
	Write ("\"><p>", sizeof ("\"><p>") - 1);
	WriteButton (BID_VIEWMAP);

		
	Write ("<p><h3>Spectator empire information from ", sizeof ("<p><h3>Spectator empire information from ") - 1);
	Write (pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (iGameNumber); 
	Write (":</h3><p><table width=\"90%\">", sizeof (":</h3><p><table width=\"90%\">") - 1);
	RenderEmpireInformation (iGameClassKey, iGameNumber, false);

		g_pGameEngine->SignalGameReader (iGameClassKey, iGameNumber);

		break;

	default:

		Assert (false);
		break;
	}

	SYSTEM_CLOSE


}