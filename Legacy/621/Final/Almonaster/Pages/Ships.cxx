<% #include "Osal/Algorithm.h"
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include "Database.h"

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

INITIALIZE_GAME

int iErrCode;

bool bMapGenerated = (m_iGameState & GAME_MAP_GENERATED) != 0;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    // Make sure the game has started and cancel wasn't pressed
    if (bMapGenerated && !WasButtonPressed (BID_CANCEL)) {

        iErrCode = HandleShipMenuSubmissions();
        if (iErrCode != OK) {
            AddMessage ("Error handling ship menu submissions");
        }
    }

    // Handle cancel all builds
    if (WasButtonPressed (BID_CANCELALLBUILDS)) {
        GameCheck (g_pGameEngine->CancelAllBuilds (m_iGameClass, m_iGameNumber, m_iEmpireKey));
        bRedirectTest = false;
    }
}

GAME_REDIRECT_ON_SUBMIT

GAME_OPEN

int iGameClassOptions;
GameCheck (g_pGameEngine->GetGameClassOptions (m_iGameClass, &iGameClassOptions));

// Individual page stuff starts here
if (!bMapGenerated) {

    if (iGameClassOptions & GENERATE_MAP_FIRST_UPDATE) {
        %><p>You cannot use ships before the map is generated<%
    } else {
        %><p>You cannot use ships before the game starts<%
    }

} else {

    IDatabase* pDatabase = g_pGameEngine->GetDatabase();

    int iBR;
    float fMaintRatio, fNextMaintRatio;

    if (m_iGameRatios >= RATIOS_DISPLAY_ON_RELEVANT_SCREENS) {

        RatioInformation ratInfo;
        GameCheck (WriteRatiosString (&ratInfo));

        iBR = ratInfo.iBR;
        fMaintRatio = ratInfo.fMaintRatio;
        fNextMaintRatio = ratInfo.fNextMaintRatio;

    } else {

        GameCheck (g_pGameEngine->GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR));
        GameCheck (g_pGameEngine->GetEmpireMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fMaintRatio));
        GameCheck (g_pGameEngine->GetEmpireNextMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fNextMaintRatio));
    }

    // Render ships
    RenderShips (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        iBR,
        fMaintRatio,
        fNextMaintRatio,
        NULL,
        false,
        NULL,
        NULL
        );

    pDatabase->Release();

    int iNumBuilds;
    GameCheck (g_pGameEngine->GetNumBuilds (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumBuilds));

    if (iNumBuilds > 0) {
        %><p><%
        WriteButton (BID_CANCELALLBUILDS);
    }
}

GAME_CLOSE

%>