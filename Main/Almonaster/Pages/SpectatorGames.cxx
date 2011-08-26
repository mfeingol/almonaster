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

int iErrCode, iSpectatorGamesPage = 0, iGameNumber = -1;
unsigned int i, iGameClassKey = NO_KEY, iClickedPlanetKey = NO_KEY, iClickedProxyPlanetKey = NO_KEY;

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
                m_bRedirectTest = false;
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
                m_bRedirectTest = false;
            }

            // View Empire Information
            if (WasButtonPressed (BID_VIEWEMPIREINFORMATION) &&
                iGameClassKey != NO_KEY && iGameNumber != -1) {
                iSpectatorGamesPage = 3;
                m_bRedirectTest = false;
            }

            break;

        case 2:

            // View map
            if (WasButtonPressed (BID_VIEWMAP) &&
                iGameClassKey != NO_KEY && iGameNumber != -1) {
                iSpectatorGamesPage = 1;
                m_bRedirectTest = false;
            }

            // View Empire Information
            if (WasButtonPressed (BID_VIEWEMPIREINFORMATION) &&
                iGameClassKey != NO_KEY && iGameNumber != -1) {
                iSpectatorGamesPage = 3;
                m_bRedirectTest = false;
            }

            break;

        case 3:

            // View map
            if (WasButtonPressed (BID_VIEWMAP) &&
                iGameClassKey != NO_KEY && iGameNumber != -1) {
                iSpectatorGamesPage = 1;
                m_bRedirectTest = false;
            }

            break;

        default:
            break;
        }
    }
}

Redirection:
if (m_bRedirectTest)
{
    PageId pageRedirect;
    if (RedirectOnSubmit (&pageRedirect))
    {
        return Redirect (pageRedirect);
    }
}

char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH] = "";

if (iSpectatorGamesPage > 0)
{
    // Make sure game exists
    bool bFlag;
    iErrCode = DoesGameExist (iGameClassKey, iGameNumber, &bFlag);
    if (iErrCode != OK || !bFlag) {

        AddMessage ("That game no longer exists");
        iSpectatorGamesPage = 0;

    } else {

        iErrCode = GetGameClassName (iGameClassKey, pszGameClassName);
        if (iErrCode != OK) {
            pszGameClassName[0] = '\0';
        }
    }
}

Check(OpenSystemPage(false));

switch (iSpectatorGamesPage) {

case 0:

    {

    %><input type="hidden" name="SpectSubPage" value="0"><%

    // Get open games
    int * piGameClass, * piGameNumber;
    unsigned int iNumClosedGames = 0;
    iErrCode = GetClosedGames(&piGameClass, &piGameNumber, &iNumClosedGames);
    if (iErrCode != OK) {
        %><h3>The list of spectator games could not be read. The error was <% Write (iErrCode); %></h3><% 
    }

    else if (iNumClosedGames == 0) {
        %><h3>There are no spectator games on this server</h3><% 
    }

    else {

        // Update the open games
        bool bDraw = false;
        int iGameClass, iGameNumber;
        unsigned int* piSuperClassKey, j, iNumSuperClasses;
        Check (GetSuperClassKeys (&piSuperClassKey, &iNumSuperClasses));

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
            unsigned int iSuperClassKey;
            bool bFlag;

            for (i = 0; i < (int)iNumClosedGames; i ++) {

                iGameClass = piGameClass[i];
                iGameNumber = piGameNumber[i];

                // Check everything
                if (CheckGameForUpdates (iGameClass, iGameNumber, false, &bFlag) == OK &&
                    DoesGameExist (iGameClass, iGameNumber, &bFlag) == OK && bFlag &&
                    IsSpectatorGame (iGameClass, iGameNumber, &bFlag) == OK && bFlag &&
                    GetGameClassSuperClassKey (iGameClass, &iSuperClassKey) == OK) {

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
                int iBegin, iCurrentGameClass;
                unsigned int iNumGamesInSuperClass, iNumToSort;

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

                    for (j = 0; j < (unsigned int)ppiTable[iNumSuperClasses][iNumClosedGames]; j ++) {

                        iGameClass = ppiGameClass[iNumSuperClasses][j];
                        iGameNumber = ppiGameNumber[iNumSuperClasses][j];

                        if (GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

                            // Best effort
                            iErrCode = WriteSpectatorGameListData (iGameClass, iGameNumber, pvGameClassInfo);
                        }

                        if (pvGameClassInfo != NULL) {
                            t_pCache->FreeData (pvGameClassInfo);
                            pvGameClassInfo = NULL;
                        }
                    }

                    %></table><%
                }

                Variant vName;
                for (i = 0; i < iNumSuperClasses; i ++) {

                    if (ppiTable [i][iNumClosedGames] > 0 && 
                        GetSuperClassName (piSuperClassKey[i], &vName) == OK) {

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

                            if (GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

                                WriteSpectatorGameListData (
                                    iGameClass, 
                                    iGameNumber, 
                                    pvGameClassInfo
                                    );
                            }

                            if (pvGameClassInfo != NULL) {
                                t_pCache->FreeData (pvGameClassInfo);
                                pvGameClassInfo = NULL;
                            }
                        }

                        %></table><%
                    }
                }
            }

            t_pCache->FreeKeys (piSuperClassKey);
        }

        delete [] piGameClass;
        delete [] piGameNumber;
    }

    }
    break;

case 1:

    %><input type="hidden" name="SpectSubPage" value="1"><%

    bool bTrue;

    iErrCode = IsSpectatorGame (iGameClassKey, iGameNumber, &bTrue);
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

    break;

case 2:

    %><input type="hidden" name="SpectSubPage" value="2"><%
    %><input type="hidden" name="GameClassKey" value="<% Write (iGameClassKey); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    {

    Variant* pvPlanetData = NULL;

    int iGameClassOptions, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;
    unsigned int iLivePlanetKey, iDeadPlanetKey;
    bool bTrue;

    GAME_MAP (pszGameMap, iGameClassKey, iGameNumber);

    iErrCode = IsSpectatorGame (iGameClassKey, iGameNumber, &bTrue);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (!bTrue) {
        %><h3>That game is not available to spectators</h3><%
        goto Cleanup;
    }

    iErrCode = GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = GetGameClassOptions (iGameClassKey, &iGameClassOptions);
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

    iErrCode = t_pCache->ReadRow (pszGameMap, iClickedPlanetKey, &pvPlanetData);
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

    %></table><%

Cleanup:

    if (pvPlanetData != NULL) {
        t_pCache->FreeData (pvPlanetData);
    }

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

    break;

default:

    Assert (false);
    break;
}

CloseSystemPage();

%>