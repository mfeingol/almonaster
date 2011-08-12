<% #include "Osal/Algorithm.h"
#include "Almonaster.h"
#include "GameEngine.h"
#include "SqlDatabase.h"

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

if (InitializeEmpireInGame(false) != OK)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
if (InitializeGame(&pageRedirect) != OK)
{
    return Redirect(pageRedirect);
}

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
        GameCheck (CancelAllBuilds (m_iGameClass, m_iGameNumber, m_iEmpireKey));
        bRedirectTest = false;
    }
}

if (bRedirectTest)
{
    PageId pageRedirect;
    if (RedirectOnSubmitGame (&pageRedirect))
    {
        return Redirect (pageRedirect);
    }
}

OpenGamePage();

int iGameClassOptions;
GameCheck (GetGameClassOptions (m_iGameClass, &iGameClassOptions));

// Individual page stuff starts here
if (!bMapGenerated) {

    if (iGameClassOptions & GENERATE_MAP_FIRST_UPDATE) {
        %><p>You cannot use ships before the map is generated<%
    } else {
        %><p>You cannot use ships before the game starts<%
    }

} else {

    int iBR;
    float fMaintRatio, fNextMaintRatio;

    if (m_iGameRatios >= RATIOS_DISPLAY_ON_RELEVANT_SCREENS) {

        RatioInformation ratInfo;
        GameCheck (WriteRatiosString (&ratInfo));

        iBR = ratInfo.iBR;
        fMaintRatio = ratInfo.fMaintRatio;
        fNextMaintRatio = ratInfo.fNextMaintRatio;

    } else {

        GameCheck (GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR));
        GameCheck (GetEmpireMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fMaintRatio));
        GameCheck (GetEmpireNextMaintenanceRatio (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fNextMaintRatio));
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

    int iNumBuilds;
    GameCheck (GetNumBuilds (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumBuilds));

    if (iNumBuilds > 0) {
        %><p><%
        WriteButton (BID_CANCELALLBUILDS);
    }
}

CloseGamePage();

%>