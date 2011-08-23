<%
#include <stdio.h>

// Almonaster
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
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

if (InitializeEmpire(false) != OK)
{
    return Redirect(LOGIN);
}

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
        iErrCode = IsEmpireInGame (iGameClassKey, iGameNumber, m_iEmpireKey, &bFlag);
        if (iErrCode == ERROR_GAME_DOES_NOT_EXIST) {
            AddMessage ("That game no longer exists");
            return Redirect (ACTIVE_GAME_LIST);
        }
        Check (iErrCode);

        if (!bFlag) {
            AddMessage ("Your empire is not in that game");
            return Redirect (ACTIVE_GAME_LIST);
        }

        HasEmpireResignedFromGame (iGameClassKey, iGameNumber, m_iEmpireKey, &bFlag);
        if (iErrCode == ERROR_GAME_DOES_NOT_EXIST) {
            AddMessage ("That game no longer exists");
            return Redirect (ACTIVE_GAME_LIST);
        }
        Check (iErrCode);

        if (bFlag) {
            AddMessage ("Your empire has resigned from that game");
            return Redirect (ACTIVE_GAME_LIST);
        }

        // Go to info screen!
        m_iGameClass = iGameClassKey;
        m_iGameNumber = iGameNumber;

        return Redirect (INFO);
    }
}

if (m_bRedirectTest)
{
    PageId pageRedirect;
    if (RedirectOnSubmit (&pageRedirect))
    {
        return Redirect (pageRedirect);
    }
}

OpenSystemPage(false);

// Begin individual page
int iNumGames, * piGameClassKey, * piGameNumber;
Check(GetEmpireActiveGames(m_iEmpireKey, &piGameClassKey, &piGameNumber, &iNumGames));

// Check for updates in active games
if (iNumGames > 0)
{
    // Cache GameData for each
    TableCacheEntry* peGameData = (TableCacheEntry*)StackAlloc(iNumGames * 2 * sizeof(TableCacheEntry));
    for (i = 0; i < iNumGames; i ++)
    {
        char* pszTable;
        
        pszTable = (char*)StackAlloc(countof(_GAME_DATA) + 64);
        GET_GAME_DATA(pszTable, piGameClassKey[i], piGameNumber[i]);

        peGameData[i].pszTableName = pszTable;
        peGameData[i].iKey = NO_KEY;
        peGameData[i].iNumColumns = 0;
        peGameData[i].pcColumns = NULL;

        pszTable = (char*)StackAlloc(countof(_GAME_EMPIRE_DATA) + 96);
        GET_GAME_EMPIRE_DATA(pszTable, piGameClassKey[i], piGameNumber[i], m_iEmpireKey);

        peGameData[iNumGames + i].pszTableName = pszTable;
        peGameData[iNumGames + i].iKey = NO_KEY;
        peGameData[iNumGames + i].iNumColumns = 0;
        peGameData[iNumGames + i].pcColumns = NULL;
    }

    Check(t_pCache->Cache(peGameData, iNumGames * 2));

    bool bUpdateOccurred, bReloadGameList = false;
    for (i = 0; i < iNumGames; i ++)
    {
        iErrCode = CheckGameForUpdates(piGameClassKey[i], piGameNumber[i], false, &bUpdateOccurred);
        if (iErrCode != OK || (iErrCode == OK && bUpdateOccurred))
        {
            bReloadGameList = true;
        }
    }

    if (bReloadGameList)
    {
        delete [] piGameClassKey;
        delete [] piGameNumber;

        Check(GetEmpireActiveGames(m_iEmpireKey, &piGameClassKey, &piGameNumber, &iNumGames));
    }
}

if (iNumGames == 0) {
    %><p><h3>You are not in any games</h3><% 
} else {

    int* piSuperClassKey, iSuperClassKey, iNumSuperClasses, iNumRenderGames = 0;

    Check (GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses));

    if (iNumSuperClasses > 0) {

        const int iNumAllocationSuperClasses = iNumSuperClasses + 2;
        const int iNumAllocationGames = iNumGames + 1;

        const int ACTIVE_PERSONAL_GAMES = iNumSuperClasses;
        const int ACTIVE_TOURNAMENT_GAMES = iNumSuperClasses + 1;

        // Create the game table
        int** ppiTable = (int**) StackAlloc (iNumAllocationSuperClasses * 3 * sizeof (int*));
        int** ppiGameClass = ppiTable + iNumAllocationSuperClasses;
        int** ppiGameNumber = ppiGameClass + iNumAllocationSuperClasses;

        for (i = 0; i < iNumAllocationSuperClasses; i ++) {

            ppiTable[i] = (int*) StackAlloc (iNumAllocationGames * 3 * sizeof (int));
            ppiTable[i][iNumGames] = 0;

            ppiGameClass[i] = ppiTable[i] + iNumAllocationGames;
            ppiGameNumber[i] = ppiGameClass[i] + iNumAllocationGames;
        }

        // Fill in the table
        for (i = 0; i < iNumGames; i ++) {

            bool bFlag;

            iErrCode = DoesGameExist (piGameClassKey[i], piGameNumber[i], &bFlag);
            if (iErrCode != OK || !bFlag) {
                continue;
            }

            iErrCode = HasEmpireResignedFromGame (piGameClassKey[i], piGameNumber[i], m_iEmpireKey, &bFlag);
            if (iErrCode != OK || bFlag) {
                continue;
            }

            iErrCode = GetGameClassSuperClassKey (piGameClassKey[i], &iSuperClassKey);
            if (iErrCode != OK) {
                continue;
            }

            iNumRenderGames ++;

            int iSuperClassIndex = NO_KEY;

            if (iSuperClassKey == TOURNAMENT) {
                iSuperClassIndex = ACTIVE_TOURNAMENT_GAMES;
            }

            else if (iSuperClassKey == PERSONAL_GAME) {
                iSuperClassIndex = ACTIVE_PERSONAL_GAMES;
            }

            else for (int j = 0; j < iNumSuperClasses; j ++) {

                if (piSuperClassKey[j] == iSuperClassKey) {
                    iSuperClassIndex = j;
                    break;
                }
            }

            Assert (iSuperClassIndex != NO_KEY);

            int& iNumGamesSoFar = ppiTable [iSuperClassIndex][iNumGames];

            // We found a match, so write down the game in question
            ppiTable [iSuperClassIndex][iNumGamesSoFar] = i;

            ppiGameClass[iSuperClassIndex][iNumGamesSoFar] = piGameClassKey[i];
            ppiGameNumber[iSuperClassIndex][iNumGamesSoFar] = piGameNumber[i];

            iNumGamesSoFar ++;
        }

        %><p><h3><%

        if (iNumRenderGames == 0) {
            %>You are not in any games</h3><% 
        } else {

            int iGameClass, iGameNumber, iBegin, iNumToSort, iCurrentGameClass;
            Variant* pvGameClassInfo = NULL, vName;

            %>You are in the following games:</h3><% 

            //
            // Display all games
            //
            for (i = iNumAllocationSuperClasses - 1; i >= 0; i --) {

                if (ppiTable [i][iNumGames] == 0) continue;

                %><p><h3><%

                if (i == ACTIVE_PERSONAL_GAMES) {
                    %>Personal Games<%
                }

                else if (i == ACTIVE_TOURNAMENT_GAMES) {
                    %>Tournament Games<%
                }

                else if (GetSuperClassName (piSuperClassKey[i], &vName) == OK) { 
                    Write (vName.GetCharPtr());
                }

                else { %>Unknown SuperClass<% }

                %>:</h3><%

                WriteActiveGameListHeader (m_vTableColor.GetCharPtr());

                int j, iNumGamesInSuperClass = ppiTable [i][iNumGames];

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

                    if (GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {
                        WriteActiveGameListData (
                            iGameClass,
                            iGameNumber,
                            pvGameClassInfo
                            );
                    }

                    if (pvGameClassInfo != NULL) {
                        FreeData (pvGameClassInfo);
                        pvGameClassInfo = NULL;
                    }
                }
                %></table><% 
            }
        }

        FreeKeys (piSuperClassKey);
    }

    delete [] piGameClassKey;
    delete [] piGameNumber;
} 

CloseSystemPage();

%>