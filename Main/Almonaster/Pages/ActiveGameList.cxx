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

int iErrCode = OK;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    const char* pszStart;
    int iGameClassKey, iGameNumber;
    if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Login")) != NULL && 
        (pszStart = pHttpForm->GetName()) != NULL &&
        sscanf (pszStart, "Login%d.%d", &iGameClassKey, &iGameNumber) == 2)
    {
        // Go to info screen!
        m_iGameClass = iGameClassKey;
        m_iGameNumber = iGameNumber;

        return Redirect(INFO);
    }
}

if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
    Check(RedirectOnSubmit(&pageRedirect, &bRedirected));
    if (bRedirected)
    {
        return Redirect(pageRedirect);
    }
}

Check(OpenSystemPage(false));

// Begin individual page
unsigned int i, iNumGames;
int* piGameClassKey, * piGameNumber;
Check(GetEmpireActiveGames(m_iEmpireKey, &piGameClassKey, &piGameNumber, &iNumGames));

// Check for updates in active games
if (iNumGames > 0)
{
    Check(CacheGameData(piGameClassKey, piGameNumber, m_iEmpireKey, iNumGames));

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

    unsigned int* piSuperClassKey, iSuperClassKey, iNumSuperClasses, iNumRenderGames = 0;

    Check (GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses));

    if (iNumSuperClasses > 0) {

        const unsigned int iNumAllocationSuperClasses = iNumSuperClasses + 2;
        const unsigned int iNumAllocationGames = iNumGames + 1;

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

            else for (unsigned int j = 0; j < iNumSuperClasses; j ++) {

                if (piSuperClassKey[j] == iSuperClassKey) {
                    iSuperClassIndex = j;
                    break;
                }
            }

            Assert(iSuperClassIndex != NO_KEY);

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

            Check(CacheGameData(piGameClassKey, piGameNumber, m_iEmpireKey, iNumGames));

            %>You are in the following games:</h3><% 

            //
            // Display all games
            //
            for (int iIndex = iNumAllocationSuperClasses - 1; iIndex >= 0; iIndex --) {

                if (ppiTable[iIndex][iNumGames] == 0) continue;

                %><p><h3><%

                if (iIndex == ACTIVE_PERSONAL_GAMES) {
                    %>Personal Games<%
                }

                else if (iIndex == ACTIVE_TOURNAMENT_GAMES) {
                    %>Tournament Games<%
                }

                else if (GetSuperClassName(piSuperClassKey[iIndex], &vName) == OK) { 
                    Write (vName.GetCharPtr());
                }

                else { %>Unknown SuperClass<% }

                %>:</h3><%

                WriteActiveGameListHeader (m_vTableColor.GetCharPtr());

                int j, iNumGamesInSuperClass = ppiTable[iIndex][iNumGames];

                // Sort games by gameclass
                Algorithm::QSortThreeAscending<int, int, int> (
                    ppiGameClass[iIndex],
                    ppiGameNumber[iIndex],
                    ppiTable[iIndex],
                    iNumGamesInSuperClass
                    );

                // Sort games by gamenumber
                iBegin = 0;
                iCurrentGameClass = ppiGameClass[iIndex][0];

                for (j = 1; j < iNumGamesInSuperClass; j ++) {

                    if (ppiGameClass[iIndex][j] != iCurrentGameClass) {

                        iNumToSort = j - iBegin;
                        if (iNumToSort > 1)
                        {
                            Algorithm::QSortThreeAscending<int, int, int> (
                                ppiGameNumber[iIndex] + iBegin,
                                ppiGameClass[iIndex] + iBegin,
                                ppiTable[iIndex] + iBegin,
                                iNumToSort
                                );
                        }

                        iBegin = j;
                        iCurrentGameClass = ppiGameClass[iIndex][j];
                    }
                }

                iNumToSort = j - iBegin;
                if (iNumToSort > 1)
                {
                    Algorithm::QSortThreeAscending<int, int, int> (
                        ppiGameNumber[iIndex] + iBegin,
                        ppiGameClass[iIndex] + iBegin,
                        ppiTable[iIndex] + iBegin,
                        iNumToSort
                        );
                }

                for (j = 0; j < iNumGamesInSuperClass; j ++)
                {
                    iGameClass = ppiGameClass[iIndex][j];
                    iGameNumber = ppiGameNumber[iIndex][j];

                    if (GetGameClassData (iGameClass, &pvGameClassInfo) == OK)
                    {
                        WriteActiveGameListData(iGameClass, iGameNumber, pvGameClassInfo);
                    }

                    if (pvGameClassInfo != NULL)
                    {
                        t_pCache->FreeData (pvGameClassInfo);
                        pvGameClassInfo = NULL;
                    }
                }
                %></table><% 
            }
        }

        t_pCache->FreeKeys (piSuperClassKey);
    }

    delete [] piGameClassKey;
    delete [] piGameNumber;
} 

CloseSystemPage();

%>