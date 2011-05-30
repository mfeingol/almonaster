<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include <stdio.h>

// Almonaster
// Copyright (c) 1998-2004 Max Attar Feingold (maf6@cornell.edu)
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

int i, iErrCode;
bool bConfirmPage = false;
int iGameClassKey = NO_KEY, iGameNumber = NO_KEY;
const char* pszPassword = NULL;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    const char* pszStart = NULL;
    if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Enter")) != NULL && 
        (pszStart = pHttpForm->GetName()) != NULL &&
        sscanf (pszStart, "Enter%d.%d", &iGameClassKey, &iGameNumber) == 2) {

        char pszForm[128];
        sprintf (pszForm, "Pass%i.%i", iGameClassKey, iGameNumber);

        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) != NULL) {
            pszPassword = pHttpForm->GetValue();
        }

        bool bConfirm = false;

        if (m_pHttpRequest->GetForm ("Confirm") == NULL) {
            bConfirm = (m_iSystemOptions & CONFIRM_IMPORTANT_CHOICES) != 0;
        }

        if (bConfirm) {
            bConfirmPage = true;
        } else {

            // Enter game!
            int iNumUpdatesTranspired;
            iErrCode = g_pGameEngine->EnterGame (
                iGameClassKey,
                iGameNumber,
                m_iEmpireKey,
                pszPassword == NULL ? "" : pszPassword,
                &iNumUpdatesTranspired,
                true,
                false,
                NULL,
                NULL
                );

            HANDLE_ENTER_GAME_OUTPUT (iErrCode);
        }
    }
}

SYSTEM_REDIRECT_ON_SUBMIT

if (bConfirmPage) {

    bool bFlag;

    // Make sure game exists
    iErrCode = g_pGameEngine->DoesGameExist (iGameClassKey, iGameNumber, &bFlag);
    if (iErrCode != OK || !bFlag) {
        AddMessage ("That game no longer exists");
        bConfirmPage = false;
    }
}

SYSTEM_OPEN (false)

if (bConfirmPage) {

    //
    // Don't want a page switch system because guest empires would need to submit an extra form
    //

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName);
    if (iErrCode != OK) {
        AddMessage ("That game no longer exists");
        return Redirect (OPEN_GAME_LIST);
    }

    %><input type="hidden" name="Pass<% Write (iGameClassKey); %>.<% Write (iGameNumber); %>" value="<% 
    if (pszPassword != NULL) {
        Write (pszPassword);
    } %>"><input type="hidden" name="Confirm" value="1"><%

    %><p>Are you sure that you want to enter <strong><%
    Write (pszGameClassName);
    %> <% Write (iGameNumber); %></strong>?<p><%
    %>If you enter a game, you are responsible for updating until the game ends.<%
    %><p><%

    WriteButton (BID_CANCEL);

    char pszEnter[128];
    sprintf (pszEnter, "Enter%i.%i", iGameClassKey, iGameNumber);

    WriteButtonString (m_iButtonKey, ButtonName[BID_ENTER], ButtonText[BID_ENTER], pszEnter);

} else {

    // Get open games
    int iNumOpenGames = 0, * piGameClass, * piGameNumber;

    iErrCode = g_pGameEngine->GetOpenGames (&piGameClass, &piGameNumber, &iNumOpenGames);
    if (iErrCode != OK) {
        %><h3>The open game list could not be read. The error was <% Write (iErrCode); %></h3><% 
    }

    else if (iNumOpenGames == 0) {
        %><h3>There are no open games on this server</h3><% 
    }

    else {

        // Update the open games
        int j, iGameClass, iGameNumber;

        int* piSuperClassKey, iNumSuperClasses;
        bool bDraw = false;
        Check (g_pGameEngine->GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses));

        if (iNumSuperClasses == 0) {
            %><h3>There are no open games on this server</h3><% 
        } else {

            // Create the game table
            int** ppiTable = (int**) StackAlloc ((iNumSuperClasses + 1) * 3 * sizeof (int*));
            int** ppiGameClass = ppiTable + iNumSuperClasses + 1;
            int** ppiGameNumber = ppiGameClass + iNumSuperClasses + 1;

            // Create the game
            for (i = 0; i < iNumSuperClasses + 1; i ++) {
                ppiTable[i] = (int*) StackAlloc ((iNumOpenGames + 1) * 3 * sizeof (int));
                ppiTable[i][iNumOpenGames] = 0;

                ppiGameClass[i] = ppiTable[i] + iNumOpenGames + 1;
                ppiGameNumber[i] = ppiGameClass[i] + iNumOpenGames + 1;
            }

            // Fill in the table
            int iSuperClassKey;
            bool bFlag;

            for (i = 0; i < iNumOpenGames; i ++) {

                iGameClass = piGameClass[i];
                iGameNumber = piGameNumber[i];

                // Check everything
                if (g_pGameEngine->CheckGameForUpdates (iGameClass, iGameNumber, false, &bFlag) == OK &&
                    g_pGameEngine->DoesGameExist (iGameClass, iGameNumber, &bFlag) == OK && bFlag &&
                    g_pGameEngine->IsGameOpen (iGameClass, iGameNumber, &bFlag) == OK && bFlag &&
                    g_pGameEngine->IsEmpireInGame (iGameClass, iGameNumber, m_iEmpireKey, &bFlag) == OK && !bFlag &&
                    g_pGameEngine->GameAccessCheck (iGameClass, iGameNumber, m_iEmpireKey, NULL, VIEW_GAME, &bFlag) == OK && bFlag &&
                    g_pGameEngine->GetGameClassSuperClassKey (iGameClass, &iSuperClassKey) == OK) {

                    for (j = 0; j < iNumSuperClasses; j ++) {
                        if (piSuperClassKey[j] == iSuperClassKey) {

                            // We found a match, so write down the game in question
                            ppiTable [j][ppiTable [j][iNumOpenGames]] = i;

                            ppiGameClass[j][ppiTable [j][iNumOpenGames]] = iGameClass;
                            ppiGameNumber[j][ppiTable [j][iNumOpenGames]] = iGameNumber;

                            ppiTable [j][iNumOpenGames] ++;
                            bDraw = true;
                            break;
                        }
                    }

                    if (j == iNumSuperClasses) {

                        // No superclass was found, so it must be a personal game
                        ppiTable [iNumSuperClasses][ppiTable [iNumSuperClasses][iNumOpenGames]] = i;

                        ppiGameClass[iNumSuperClasses][ppiTable [iNumSuperClasses][iNumOpenGames]] = iGameClass;
                        ppiGameNumber[iNumSuperClasses][ppiTable [iNumSuperClasses][iNumOpenGames]] = iGameNumber;

                        ppiTable [iNumSuperClasses][iNumOpenGames] ++;
                        bDraw = true;
                    }
                }
            }

            if (!bDraw) {
                %><h3>There are no open games available to your empire</h3><%
            } else {

                %><p><h3>Join an open game:</h3><%
                Variant* pvGameClassInfo = NULL;
                int iBegin, iCurrentGameClass, iNumGamesInSuperClass, iNumToSort;

                if (ppiTable [iNumSuperClasses][iNumOpenGames] > 0) {

                    %><p><h3>Personal Games:</h3><%
                    WriteOpenGameListHeader (m_vTableColor.GetCharPtr());

                    iNumGamesInSuperClass = ppiTable[iNumSuperClasses][iNumOpenGames];

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

                    for (j = 0; j < ppiTable[iNumSuperClasses][iNumOpenGames]; j ++) {

                        iGameClass = ppiGameClass[iNumSuperClasses][j];
                        iGameNumber = ppiGameNumber[iNumSuperClasses][j];

                        if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

                            // Best effort
                            iErrCode = WriteOpenGameListData (
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

                    if (ppiTable [i][iNumOpenGames] > 0 && 
                        g_pGameEngine->GetSuperClassName (piSuperClassKey[i], &vName) == OK) {

                        %><p><h3><% Write (vName.GetCharPtr()); %>:</h3><%
                        WriteOpenGameListHeader (m_vTableColor.GetCharPtr());

                        iNumGamesInSuperClass = ppiTable[i][iNumOpenGames];

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

                                WriteOpenGameListData (
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
            }

            g_pGameEngine->FreeKeys (piSuperClassKey);
        }

        delete [] piGameClass;
        delete [] piGameNumber;
    }
}

SYSTEM_CLOSE

%>