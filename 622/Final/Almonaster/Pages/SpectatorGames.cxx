<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
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

INITIALIZE_EMPIRE

IHttpForm* pHttpForm;

int i, iErrCode, iSpectatorGamesPage = 0, iGameNumber = -1;
unsigned int iGameClassKey = NO_KEY, iClickedPlanetKey = NO_KEY, iClickedProxyPlanetKey = NO_KEY;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if ((pHttpForm = m_pHttpRequest->GetForm ("SpectSubPage")) == NULL) {
        goto Redirection;
    } else {

        IHttpForm* pHttpFormGame = NULL;
        const char* pszSpectator = NULL;

        if ((pHttpFormGame = m_pHttpRequest->GetForm ("GameClassKey")) != NULL) {
            iGameClassKey = pHttpFormGame->GetUIntValue();
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
                iGameClassKey != NO_KEY && iGameNumber != -1
                ) {

                iSpectatorGamesPage = 2;
                bRedirectTest = false;
            }

            // View Empire Information
            if (WasButtonPressed (BID_VIEWEMPIREINFORMATION) &&
                iGameClassKey != NO_KEY && iGameNumber != -1) {
                iSpectatorGamesPage = 3;
                bRedirectTest = false;
            }

            break;

        case 2:

            // View map
            if (WasButtonPressed (BID_VIEWMAP) &&
                iGameClassKey != NO_KEY && iGameNumber != -1) {
                iSpectatorGamesPage = 1;
                bRedirectTest = false;
            }

            // View Empire Information
            if (WasButtonPressed (BID_VIEWEMPIREINFORMATION) &&
                iGameClassKey != NO_KEY && iGameNumber != -1) {
                iSpectatorGamesPage = 3;
                bRedirectTest = false;
            }

            break;

        case 3:

            // View map
            if (WasButtonPressed (BID_VIEWMAP) &&
                iGameClassKey != NO_KEY && iGameNumber != -1) {
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

    if (g_pGameEngine->WaitGameReader (iGameClassKey, iGameNumber, NO_KEY, NULL) != OK) {

        AddMessage ("That game no longer exists");
        iSpectatorGamesPage = 0;

    } else {

        // Make sure game exists
        iErrCode = g_pGameEngine->DoesGameExist (iGameClassKey, iGameNumber, &bFlag);
        if (iErrCode != OK || !bFlag) {

            g_pGameEngine->SignalGameReader (iGameClassKey, iGameNumber, NO_KEY, NULL);

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

    %><input type="hidden" name="SpectSubPage" value="0"><%

    // Get open games
    int iNumClosedGames = 0, * piGameClass, * piGameNumber;

    iErrCode = g_pGameEngine->GetClosedGames (&piGameClass, &piGameNumber, &iNumClosedGames);
    if (iErrCode != OK) {
        %><h3>The list of spectator games could not be read. The error was <% Write (iErrCode); %></h3><% 
    }

    else if (iNumClosedGames == 0) {
        %><h3>There are no spectator games on this server</h3><% 
    }

    else {

        // Update the open games
        int j, iGameClass, iGameNumber;

        int* piSuperClassKey, iNumSuperClasses;
        bool bDraw = false;
        Check (g_pGameEngine->GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses));

        if (iNumSuperClasses == 0) {
            %><h3>There are no spectator games on this server</h3><% 
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
                if (g_pGameEngine->CheckGameForUpdates (iGameClass, iGameNumber, false, &bFlag) == OK &&
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
                %><h3>There are no spectator games available</h3><%
            } else {

                %><p><h3>View a spectator game:</h3><%
                Variant* pvGameClassInfo = NULL;
                int iBegin, iCurrentGameClass, iNumGamesInSuperClass, iNumToSort;

                if (ppiTable [iNumSuperClasses][iNumClosedGames] > 0) {

                    %><p><h3>Personal Games:</h3><%
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

                    %></table><%
                }

                Variant vName;
                for (i = 0; i < iNumSuperClasses; i ++) {

                    if (ppiTable [i][iNumClosedGames] > 0 && 
                        g_pGameEngine->GetSuperClassName (piSuperClassKey[i], &vName) == OK) {

                        %><p><h3><% Write (vName.GetCharPtr()); %>:</h3><%
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
    break;

case 1:

    %><input type="hidden" name="SpectSubPage" value="1"><%

    bool bTrue;

    iErrCode = g_pGameEngine->IsSpectatorGame (iGameClassKey, iGameNumber, &bTrue);
    if (iErrCode != OK || !bTrue) {
        %><h3>That game is not available to spectators</h3><%
    } else {

        %><p><%

        WriteButton (BID_VIEWEMPIREINFORMATION);

        %><h3><p>Spectator map from <% Write (pszGameClassName); %> <% Write (iGameNumber); %>:</h3><%

        iErrCode = RenderMap (
            iGameClassKey,
            iGameNumber,
            m_iEmpireKey,
            false,
            NULL,
            true
            );

        if (iErrCode == ERROR_NO_PLANETS_AVAILABLE) {
            %><h3>The game has no planets available to spectators</h3><%
        }

        else if (iErrCode != OK) {
            %><h3>Error <% Write (iErrCode); %> rendering game map</h3><%
        }

        %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%
        %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%
    }

    g_pGameEngine->SignalGameReader (iGameClassKey, iGameNumber, NO_KEY, NULL);

    break;

case 2:

    %><input type="hidden" name="SpectSubPage" value="2"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    {

    IDatabase* pDatabase = NULL;
    Variant* pvPlanetData = NULL;

    int iGameClassOptions, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;
    unsigned int iLivePlanetKey, iDeadPlanetKey;
    bool bTrue;

    GAME_MAP (pszGameMap, iGameClassKey, iGameNumber);

    iErrCode = g_pGameEngine->IsSpectatorGame (iGameClassKey, iGameNumber, &bTrue);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (!bTrue) {
        %><h3>That game is not available to spectators</h3><%
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

    iErrCode = pDatabase->ReadRow (pszGameMap, iClickedPlanetKey, &pvPlanetData);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    %><p><%

    WriteButton (BID_VIEWEMPIREINFORMATION);
    WriteButton (BID_VIEWMAP);

    %><p><h3>Spectator planet from <% Write (pszGameClassName); %> <% Write (iGameNumber); %>:</h3><%

    %><p><table width="90%"><%

    m_iGameState |= STARTED | GAME_MAP_GENERATED;
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
        pvPlanetData,
        &bTrue
        );

    g_pGameEngine->SignalGameReader (iGameClassKey, iGameNumber, NO_KEY, NULL);

    %></table><%

Cleanup:

    if (pvPlanetData != NULL) {
        pDatabase->FreeData (pvPlanetData);
    }

    SafeRelease (pDatabase);

    if (iErrCode != OK) {
        %><h3>Error <% Write (iErrCode); %> rendering page</h3><%
    }

    }
    break;

case 3:

    %><input type="hidden" name="SpectSubPage" value="3"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    %><p><%

    WriteButton (BID_VIEWMAP);

    %><p><h3>Spectator empire information from <% Write (pszGameClassName); %> <% Write (iGameNumber); %>:</h3><%

    %><p><table width="90%"><%

    RenderEmpireInformation (iGameClassKey, iGameNumber, false);

    g_pGameEngine->SignalGameReader (iGameClassKey, iGameNumber, NO_KEY, NULL);

    break;

default:

    Assert (false);
    break;
}

SYSTEM_CLOSE

%>