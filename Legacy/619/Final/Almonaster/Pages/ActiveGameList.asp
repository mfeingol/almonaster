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

INITIALIZE_EMPIRE

IHttpForm* pHttpForm;

int i, iErrCode = OK;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

	const char* pszStart;
	int iGameClassKey, iGameNumber;
	if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Login")) != NULL && 
		(pszStart = pHttpForm->GetName()) != NULL &&
		sscanf (pszStart, "Login%d.%d", &iGameClassKey, &iGameNumber) == 2) {

		// Make sure empire is in game
		bool bFlag;
		Check (g_pGameEngine->IsEmpireInGame (iGameClassKey, iGameNumber, m_iEmpireKey, &bFlag));

		if (!bFlag) {
			AddMessage ("Your empire is not in that game");
			return Redirect (ACTIVE_GAME_LIST);
		}

		Check (g_pGameEngine->HasEmpireResignedFromGame (iGameClassKey, iGameNumber, m_iEmpireKey, &bFlag));
		if (bFlag) {
			AddMessage ("Your empire has resigned from that game");
			return Redirect (ACTIVE_GAME_LIST);
		}

		// Go to info screen!
		m_iGameClass = iGameClassKey;
		m_iGameNumber = iGameNumber;

		AddMessage ("Welcome back, ");
		AppendMessage (m_vEmpireName.GetCharPtr());

		return Redirect (INFO);
	}
}

SYSTEM_REDIRECT_ON_SUBMIT

SYSTEM_OPEN (false)

// Begin individual page
int iNumGames, * piGameClassKey, * piGameNumber;
Check (g_pGameEngine->GetEmpireActiveGames (m_iEmpireKey, &piGameClassKey, &piGameNumber, &iNumGames));

// Check for updates in active games
if (iNumGames > 0) {

	bool bUpdateOccurred, bReloadGameList = false;
	for (i = 0; i < iNumGames; i ++) {
		Check (g_pGameEngine->CheckGameForUpdates (piGameClassKey[i], piGameNumber[i], &bUpdateOccurred));
		if (bUpdateOccurred) {
			bReloadGameList = true;
		}
	}

	if (bReloadGameList) {

		delete [] piGameClassKey;
		delete [] piGameNumber;

		Check (g_pGameEngine->GetEmpireActiveGames (m_iEmpireKey, &piGameClassKey, &piGameNumber, &iNumGames));
	}
}

if (iNumGames == 0) { 
%><p><h3>You are not in any games</h3><% 

} else {

	int* piSuperClassKey;
	int j, iNumSuperClasses;
	bool bDraw = false;

	Check (g_pGameEngine->GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses));

	if (iNumSuperClasses > 0) {

		// Create the game table
		int** ppiTable = (int**) StackAlloc ((iNumSuperClasses + 1) * 3 * sizeof (int*));
		int** ppiGameClass = ppiTable + iNumSuperClasses + 1;
		int** ppiGameNumber = ppiGameClass + iNumSuperClasses + 1;

		for (i = 0; i < iNumSuperClasses + 1; i ++) {
			ppiTable[i] = (int*) StackAlloc ((iNumGames + 1) * 3 * sizeof (int));
			ppiTable[i][iNumGames] = 0;

			ppiGameClass[i] = ppiTable[i] + iNumGames + 1;
			ppiGameNumber[i] = ppiGameClass[i] + iNumGames + 1;
		}

		// Fill in the table
		int iSuperClassKey;
		bool bExists;
		for (i = 0; i < iNumGames; i ++) {

			iErrCode = g_pGameEngine->DoesGameExist (piGameClassKey[i], piGameNumber[i], &bExists);
			if (iErrCode != OK || !bExists) {
				continue;
			}

			iErrCode = g_pGameEngine->HasEmpireResignedFromGame (piGameClassKey[i], piGameNumber[i], m_iEmpireKey, &bExists);
			if (iErrCode != OK || bExists) {
				continue;
			}

			iErrCode = g_pGameEngine->GetGameClassSuperClassKey (piGameClassKey[i], &iSuperClassKey);
			if (iErrCode == OK) {

				for (j = 0; j < iNumSuperClasses; j ++) {
					if (piSuperClassKey[j] == iSuperClassKey) {

						// We found a match, so write down the game in question
						ppiTable [j][ppiTable [j][iNumGames]] = i;

						ppiGameClass[j][ppiTable [j][iNumGames]] = piGameClassKey[i];
						ppiGameNumber[j][ppiTable [j][iNumGames]] = piGameNumber[i];

						ppiTable [j][iNumGames] ++;
						bDraw = true;
						break;
					}
				}

				if (j == iNumSuperClasses) {

					// No superclass was found, so it must be a personal game
					ppiTable [iNumSuperClasses][ppiTable [iNumSuperClasses][iNumGames]] = i;

					ppiGameClass[iNumSuperClasses][ppiTable [iNumSuperClasses][iNumGames]] = piGameClassKey[i];
					ppiGameNumber[iNumSuperClasses][ppiTable [iNumSuperClasses][iNumGames]] = piGameNumber[i];

					ppiTable [iNumSuperClasses][iNumGames] ++;
					bDraw = true;
				}
			}
		}

		if (bDraw) { 
			%><p><h3>You are in the following games:</h3><% 

			int iGameClass, iGameNumber, iNumGamesInSuperClass, iBegin, iNumToSort, iCurrentGameClass;

			Variant* pvGameClassInfo = NULL;

			// Personal game classes
			if (ppiTable [iNumSuperClasses][iNumGames] > 0) {

				%><p><h3>Personal Games:</h3><% 

				WriteActiveGameListHeader (m_vTableColor.GetCharPtr());

				iNumGamesInSuperClass = ppiTable [iNumSuperClasses][iNumGames];

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

				for (j = 0; j < iNumGamesInSuperClass; j ++) {

					// Get gameclass, gamenumber
					iGameClass = ppiGameClass[iNumSuperClasses][j];
					iGameNumber = ppiGameNumber[iNumSuperClasses][j];

					if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

						// best effort
						iErrCode = WriteActiveGameListData (
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
				%></table><% 

			}

			Variant vName;
			for (i = 0; i < iNumSuperClasses; i ++) {

				if (ppiTable [i][iNumGames] > 0 && 
					g_pGameEngine->GetSuperClassName (piSuperClassKey[i], &vName) == OK) { 

					%><p><h3><% Write (vName.GetCharPtr()); %>:</h3><%
					WriteActiveGameListHeader (m_vTableColor.GetCharPtr());

					iNumGamesInSuperClass = ppiTable [i][iNumGames];

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

						// Get gameclass, gamenumber
						iGameClass = ppiGameClass[i][j];
						iGameNumber = ppiGameNumber[i][j];

						if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {
							WriteActiveGameListData (
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
					%></table><% 
				}
			}
		} else { 
			%><p><h3>You are not in any games</h3><% 
		}
		if (iNumSuperClasses > 0) {
			g_pGameEngine->FreeKeys (piSuperClassKey);
		}
	}

	delete [] piGameClassKey;
	delete [] piGameNumber;
} 

SYSTEM_CLOSE

%>