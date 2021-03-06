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

int iErrCode;

bool bInitialized;
iErrCode = InitializeEmpire(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

bool bConfirmPage = false;
int iGameClassKey = NO_KEY, iGameNumber = NO_KEY;
const char* pszPassword = NULL;
unsigned int i;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    const char* pszStart = NULL;
    if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Enter")) != NULL && 
        (pszStart = pHttpForm->GetName()) != NULL &&
        sscanf (pszStart, "Enter%d.%d", &iGameClassKey, &iGameNumber) == 2) {

        char pszForm[128];
        sprintf(pszForm, "Pass%i.%i", iGameClassKey, iGameNumber);

        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) != NULL)
        {
            pszPassword = pHttpForm->GetValue();
        }

        bool bConfirm = false;
        if (m_pHttpRequest->GetForm ("Confirm") == NULL)
        {
            bConfirm = (m_iSystemOptions & CONFIRM_IMPORTANT_CHOICES) != 0;
        }

        if (bConfirm)
        {
            iErrCode = CacheGameData(&iGameClassKey, &iGameNumber, NO_KEY, 1);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = IsGamePasswordCorrect(iGameClassKey, iGameNumber, pszPassword);
            if (iErrCode == ERROR_GAME_DOES_NOT_EXIST)
            {
                AddMessage("That game no longer exists");
                return Redirect(m_pgPageId);
            }
            if (iErrCode == ERROR_PASSWORD)
            {
                AddMessage("Your password was not accepted");
            }
            else
            {
                RETURN_ON_ERROR(iErrCode);
                bConfirmPage = true;
            }
        }
        else
        {
            iErrCode = CacheAllGameTables(iGameClassKey, iGameNumber);
            RETURN_ON_ERROR(iErrCode);

            // Enter game!
            int iNumUpdatesTranspired;
            iErrCode = EnterGame (
                iGameClassKey,
                iGameNumber,
                m_iEmpireKey,
                pszPassword == NULL ? "" : pszPassword,
                NULL,
                &iNumUpdatesTranspired,
                true,
                false,
                true
                );

            HANDLE_ENTER_GAME_OUTPUT (iErrCode);
        }
    }
}

if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
    iErrCode = RedirectOnSubmit(&pageRedirect, &bRedirected);
    RETURN_ON_ERROR(iErrCode);
    if (bRedirected)
    {
        return Redirect(pageRedirect);
    }
}

iErrCode = OpenSystemPage(false);
RETURN_ON_ERROR(iErrCode);

if (bConfirmPage)
{
    //
    // Don't want a page switch system because guest empires would need to submit an extra form
    //

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = GetGameClassName (iGameClassKey, pszGameClassName);
    if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
    {
        AddMessage ("That game no longer exists");
        return Redirect (OPEN_GAME_LIST);
    }
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="Pass<% Write (iGameClassKey); %>.<% Write (iGameNumber); %>" value="<% 
    if (pszPassword != NULL)
    {
        Write(pszPassword);
    } %>"><input type="hidden" name="Confirm" value="1"><%

    %><p>Are you sure that you want to enter <strong><%
    Write(pszGameClassName);
    %> <% Write(iGameNumber); %></strong>?<p><%
    %>If you enter a game, you are responsible for updating until the game ends.<%
    %><p><%

    WriteButton (BID_CANCEL);

    char pszEnter[128];
    sprintf(pszEnter, "Enter%i.%i", iGameClassKey, iGameNumber);

    WriteButtonString(m_iButtonKey, m_iButtonAddress, ButtonName[BID_ENTER], ButtonText[BID_ENTER], pszEnter);

} else {

    // Get open games
    unsigned int iNumOpenGames = 0;
    Variant** ppvGame = NULL;
    AutoFreeData free_ppvGame(ppvGame);

    iErrCode = GetOpenGames(&ppvGame, &iNumOpenGames);
    RETURN_ON_ERROR(iErrCode);

    if (iNumOpenGames == 0)
    {
        %><h3>There are no open games on this server</h3><% 
    }
    else
    {
        // Update the open games
        unsigned int* piSuperClassKey, iNumSuperClasses, j;
        AutoFreeKeys free_piSuperClassKey(piSuperClassKey);
        bool bDraw = false;
        iErrCode = GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses);
        RETURN_ON_ERROR(iErrCode);

        if (iNumSuperClasses == 0)
        {
            %><h3>There are no open games on this server</h3><% 
        }
        else
        {
            // Create the game table
            int** ppiTable = (int**) StackAlloc ((iNumSuperClasses + 1) * 3 * sizeof (int*));
            int** ppiGameClass = ppiTable + iNumSuperClasses + 1;
            int** ppiGameNumber = ppiGameClass + iNumSuperClasses + 1;

            // Create the game
            for (i = 0; i < iNumSuperClasses + 1; i ++)
            {
                ppiTable[i] = (int*) StackAlloc ((iNumOpenGames + 1) * 3 * sizeof (int));
                ppiTable[i][iNumOpenGames] = 0;

                ppiGameClass[i] = ppiTable[i] + iNumOpenGames + 1;
                ppiGameNumber[i] = ppiGameClass[i] + iNumOpenGames + 1;
            }

            // Fill in the table
            unsigned int iSuperClassKey;
            bool bFlag, bIdle = false;

            // Cache GameData for each game
            iErrCode = CacheGameData((const Variant**)ppvGame, NO_KEY, iNumOpenGames);
            RETURN_ON_ERROR(iErrCode);

            for (i = 0; i < (int)iNumOpenGames; i ++)
            {
                int iOpenGameClass = ppvGame[i][0].GetInteger();
                int iOpenGameNumber = ppvGame[i][1].GetInteger();

                // Check everything
                iErrCode = CheckGameForUpdates(iOpenGameClass, iOpenGameNumber, &bFlag);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = DoesGameExist(iOpenGameClass, iOpenGameNumber, &bFlag);
                RETURN_ON_ERROR(iErrCode);
                
                if (bFlag)
                {
                    iErrCode = IsGameOpen (iOpenGameClass, iOpenGameNumber, &bFlag);
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (bFlag)
                    {
                        iErrCode = IsEmpireInGame (iOpenGameClass, iOpenGameNumber, m_iEmpireKey, &bFlag);
                        RETURN_ON_ERROR(iErrCode);
                        
                        if (!bFlag)
                        {
                            iErrCode = GetGameClassSuperClassKey (iOpenGameClass, &iSuperClassKey);
                            RETURN_ON_ERROR(iErrCode);

                            GameAccessDeniedReason rReason;
                            iErrCode = GameAccessCheck(iOpenGameClass, iOpenGameNumber, m_iEmpireKey, NULL, VIEW_GAME, &bFlag, &rReason);
                            RETURN_ON_ERROR(iErrCode);

                            if (!bFlag)
                            {
                                if (rReason == ACCESS_DENIED_IDLE_EMPIRE)
                                {
                                    bIdle = true;
                                }
                            }
                            else
                            {
                                for (j = 0; j < iNumSuperClasses; j ++)
                                {
                                    if (piSuperClassKey[j] == iSuperClassKey)
                                    {
                                        // We found a match, so write down the game in question
                                        ppiTable [j][ppiTable [j][iNumOpenGames]] = i;

                                        ppiGameClass[j][ppiTable [j][iNumOpenGames]] = iOpenGameClass;
                                        ppiGameNumber[j][ppiTable [j][iNumOpenGames]] = iOpenGameNumber;

                                        ppiTable [j][iNumOpenGames] ++;
                                        bDraw = true;
                                        break;
                                    }
                                }

                                if (j == iNumSuperClasses)
                                {
                                    // No superclass was found, so it must be a personal game
                                    ppiTable [iNumSuperClasses][ppiTable [iNumSuperClasses][iNumOpenGames]] = i;

                                    ppiGameClass[iNumSuperClasses][ppiTable [iNumSuperClasses][iNumOpenGames]] = iOpenGameClass;
                                    ppiGameNumber[iNumSuperClasses][ppiTable [iNumSuperClasses][iNumOpenGames]] = iOpenGameNumber;

                                    ppiTable [iNumSuperClasses][iNumOpenGames] ++;
                                    bDraw = true;
                                }
                            }
                        }
                    }
                }
            }

            if (bIdle) {
                %><h3>Some open games are unavailable to your empire because it is idling in an active game</h3><%
            }

            if (!bDraw) {

                if (!bIdle) {
                    %><h3>There are no open games available to your empire</h3><%
                }
            } else {

                %><p><h3>Join an open game:</h3><%

                int iBegin, iCurrentGameClass, iNumToSort;
                unsigned int iNumGamesInSuperClass;

                if (ppiTable [iNumSuperClasses][iNumOpenGames] > 0)
                {
                    %><p><h3>Personal Games:</h3><%
                    WriteOpenGameListHeader(m_vTableColor.GetCharPtr());

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

                    for (j = 0; j < (unsigned int)ppiTable[iNumSuperClasses][iNumOpenGames]; j ++)
                    {
                        int iOpenGameClass = ppiGameClass[iNumSuperClasses][j];
                        int iOpenGameNumber = ppiGameNumber[iNumSuperClasses][j];

                        Variant* pvGameClassInfo = NULL;
                        AutoFreeData free_pvGameClassInfo(pvGameClassInfo);

                        iErrCode = GetGameClassData(iOpenGameClass, &pvGameClassInfo);
                        RETURN_ON_ERROR(iErrCode);

                        // Best effort
                        iErrCode = WriteOpenGameListData(iOpenGameClass, iOpenGameNumber, pvGameClassInfo);
                        RETURN_ON_ERROR(iErrCode);
                    }

                    %></table><%
                }

                Variant vName;
                for (i = 0; i < iNumSuperClasses; i ++)
                {
                    if (ppiTable [i][iNumOpenGames] > 0)
                    {
                        iErrCode = GetSuperClassName(piSuperClassKey[i], &vName);
                        RETURN_ON_ERROR(iErrCode);
                        
                        %><p><h3><% Write (vName.GetCharPtr()); %>:</h3><%
                        WriteOpenGameListHeader(m_vTableColor.GetCharPtr());

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

                        for (j = 0; j < iNumGamesInSuperClass; j ++)
                        {
                            int iOpenGameClass = ppiGameClass[i][j];
                            int iOpenGameNumber = ppiGameNumber[i][j];

                            Variant* pvGameClassInfo = NULL;
                            AutoFreeData free_pvGameClassInfo(pvGameClassInfo);

                            iErrCode = GetGameClassData(iOpenGameClass, &pvGameClassInfo);
                            RETURN_ON_ERROR(iErrCode);
                            
                            iErrCode = WriteOpenGameListData(iOpenGameClass, iOpenGameNumber, pvGameClassInfo);
                            RETURN_ON_ERROR(iErrCode);
                        }

                        %></table><%
                    }
                }
            }
        }
    }
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>