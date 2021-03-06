<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

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

int iErrCode;

INITIALIZE_EMPIRE

INITIALIZE_GAME

GAME_REDIRECT_ON_SUBMIT

GAME_OPEN

bool bGameStarted = (m_iGameState & STARTED) != 0, bIsOpen;

// Individual page stuff starts here
String strTime;
Variant* pvEmpireData = NULL, vMaxAgRatio, vPopNeeded;
int iNumShips, iBattleRank, iMilVal, iMin, iFuel, iAg, iUpdatedEmpires, iNextBattleRank, iActiveEmpires, iRatio,
    iMinUsed, iMaxNumShips;

float fTechDev, fMaintRatio, fFuelRatio, fAgRatio, fHypAgRatio, fHypMaintRatio, fHypFuelRatio, 
    fNextTechIncrease, fNextTechLevel, fMaxTechDev;

GameCheck (g_pGameEngine->GetNumUpdatedEmpires (m_iGameClass, m_iGameNumber, &iUpdatedEmpires));
GameCheck (g_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iActiveEmpires));

GameCheck (g_pGameEngine->GetEmpireGameInfo (m_iGameClass, m_iGameNumber, m_iEmpireKey, &pvEmpireData, &iNumShips,
    &iBattleRank, &iMilVal, &fTechDev, &fMaintRatio, &fFuelRatio, &fAgRatio, &fHypMaintRatio, &fHypFuelRatio, 
    &fHypAgRatio, &fNextTechIncrease, &iMaxNumShips));

GameCheck (g_pGameEngine->GetGameClassMaxTechIncrease (m_iGameClass, &fMaxTechDev));

GameCheck (g_pGameEngine->GetGameClassProperty (m_iGameClass, SystemGameClassData::MaxAgRatio, &vMaxAgRatio));
GameCheck (g_pGameEngine->GetGameClassProperty (m_iGameClass, SystemGameClassData::BuilderPopLevel, &vPopNeeded));

if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
    GameCheck (WriteRatiosString (NULL));
}

%><p><font size="4"><strong>Game Information</strong></font><%
%><p><table width="90%"><tr><td><strong>Updates:</strong></td><td><%

if (bGameStarted) {
    Write (m_iNumNewUpdates);
} else { 
    %>The game has not started<%
}

%></td><td align="right"><strong>Last update:</strong></td><td align="center"><%

if (bGameStarted && m_iNumNewUpdates > 0) {
    WriteTime (m_sSecondsSince);
    %> ago<%
} else {
    %>None<%
} %></td><td align="right"><strong>Next update:</strong></td><td align="right"><%

if (bGameStarted) {
    WriteTime (m_sSecondsUntil);
} else {
    
    int iMinNumEmpires, iTotal;
    iErrCode = g_pGameEngine->GetNumEmpiresRequiredForGameToStart (m_iGameClass, &iMinNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iTotal = iMinNumEmpires - iActiveEmpires;
    %>When <strong><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><% Write (iTotal); 
    %></font></strong> more empire<% Write (iTotal == 1 ? " joins" : "s join");
} %></td></tr></table><%

%><br><table width="90%"><tr><td align="center">You are <% 

if (m_iGameOptions & UPDATED) {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>">ready</font><%
} else { 
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><strong>not ready</strong></font><% 
} %> for an update</td><td align="center"><%

iErrCode = g_pGameEngine->IsGameOpen (m_iGameClass, m_iGameNumber, &bIsOpen);
if (iErrCode != OK) {
    goto Cleanup;
}

if (bIsOpen) {
    %>The game is <strong><font color=<% Write (m_vBadColor.GetCharPtr());%>>open</font></strong><% 
} else {
    %>The game is <strong><font color=<% Write (m_vGoodColor.GetCharPtr());%>>closed</font></strong><%
} 

if (m_iGameState & ADMIN_PAUSED) {
    %> and <strong><font color="#<% Write (m_vBadColor.GetCharPtr()); %>">paused by an administrator</font></strong><%
}

else if (m_iGameState & PAUSED) {
    %> and <strong><font color="#<% Write (m_vGoodColor.GetCharPtr());%>">paused</font></strong><%
}

%></td><%

%><td align="center"><strong><% 


%><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><%
Write (iUpdatedEmpires); %></font></strong> of <strong><%
%><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><% 
Write (iActiveEmpires); %></font></strong> empire<%

if (iActiveEmpires != 1) {
    %>s<%
}

if (iUpdatedEmpires == 1 || iActiveEmpires == 1) {
    %> is<%
} else {
    %> are<%
} %> ready for an update<%

%></td></tr></table><p><font size="4"><strong>Empire Totals</strong><%

%><p><table width="90%"><%

%><tr><%

%><td width = "20%"><strong>Minerals:</strong></td><td><%
iMin = pvEmpireData[GameEmpireData::TotalMin].GetInteger() + pvEmpireData[GameEmpireData::BonusMin].GetInteger();
Write (iMin); %></td><td><%
iMinUsed = pvEmpireData[GameEmpireData::TotalBuild].GetInteger() + pvEmpireData[GameEmpireData::TotalMaintenance].GetInteger();
if (iMin == 0) {
    iRatio = 0;
} else {
    iRatio = (int) (100 * iMinUsed) / iMin;
}
if (iRatio > 100) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><%
}
Write (iRatio); %>%</font> in use</td><%

%><td><strong>Tech Level:</strong></td><td><% 
Write (pvEmpireData[GameEmpireData::TechLevel]); %> (BR <% Write (iBattleRank); %>)</td><%

%><td width="20%"><strong>Economic Level:</strong></td><td><% Write (pvEmpireData[GameEmpireData::Econ]); %></td><%

%></tr><tr><%

%><td><strong>Fuel:</strong></td><td><% 
iFuel = pvEmpireData[GameEmpireData::TotalFuel].GetInteger() + pvEmpireData[GameEmpireData::BonusFuel].GetInteger();
Write (iFuel); %></td><td><%
if (iFuel == 0) {
    iRatio = 0;
} else {
    iRatio = (100 * pvEmpireData[GameEmpireData::TotalFuelUse].GetInteger()) / iFuel;
}
if (iRatio > 100) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><%
}
Write (iRatio);
%>%</font> in use</td><%

%><td><strong>Tech Increase:</strong></td><td><%
if (fTechDev < (float) 0.0) {

    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
    Write (fTechDev); %> of <% Write (fMaxTechDev); %></font><%

} else {

    if (fTechDev == fMaxTechDev) {
        %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><%
        Write (fTechDev); %> of <% Write (fMaxTechDev); %></font><%
    } else {
        Write (fTechDev); %> of <% Write (fMaxTechDev);
    }
}
%></td><%

%><td><strong>Military Level:</strong></td><td><% Write (iMilVal); %></td><%

%></tr><tr><%

%><td><strong>Agriculture:</strong></td><td><%
iAg = pvEmpireData[GameEmpireData::TotalAg].GetInteger() + pvEmpireData[GameEmpireData::BonusAg].GetInteger();
Write (iAg); %></td><td><%
if (iAg == 0) {
    iRatio = 0;
} else {
    iRatio = (100 * pvEmpireData[GameEmpireData::TotalPop].GetInteger()) / iAg;
}
if (iRatio > 100) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><%
}
Write (iRatio);
%>%</font> in use</td><%

%><td></td><%

%></tr><tr><%

%><td><strong>Population:</strong></td><td><% Write (pvEmpireData[GameEmpireData::TotalPop].GetInteger()); %></td><td><%
if (pvEmpireData[GameEmpireData::TargetPop].GetInteger() == 0) {
    iRatio = 0;
} else {
    iRatio = (100 * pvEmpireData[GameEmpireData::TotalPop].GetInteger() / pvEmpireData[GameEmpireData::TargetPop].GetInteger());
}
if (iRatio > 100) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><%
}
Write (iRatio); 
%>%</font> of target</td><%

%><td><strong>Next Tech Level:</strong></td><td><% 
fNextTechLevel = pvEmpireData[GameEmpireData::TechLevel].GetFloat() + fTechDev;
iNextBattleRank = g_pGameEngine->GetBattleRank (fNextTechLevel);

if (iNextBattleRank > iBattleRank) {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><% 
    Write (fNextTechLevel); %> (BR <% Write (iNextBattleRank); %>)</font><%
} else {
    if (iNextBattleRank < iBattleRank) {
        %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><% Write (fNextTechLevel); %> (BR <% 
        Write (iNextBattleRank); %>)</font><%
    } else {
        Write (fNextTechLevel); %> (BR <% Write (iNextBattleRank); %>)<%
    }
} %></td><%

%><td><strong>Planets:</strong></td><td><% Write (pvEmpireData[GameEmpireData::NumPlanets]); %></td><%

%></tr><tr><%

%><td><strong>Target Population:</strong></td><td><% Write (pvEmpireData[GameEmpireData::TargetPop]); %></td><td><%
if (iAg == 0) {
    iRatio = 0;
} else {
    iRatio = (100 * pvEmpireData[GameEmpireData::TargetPop].GetInteger()) / iAg;
}
if (iRatio > 100) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
} else {
%><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><%
}

Write (iRatio); %>%</font> of agriculture</td><%

%><td><strong>Next Tech Increase:</strong></td><td><% 

if (fNextTechIncrease > (float) 0.0) {
    Write (fNextTechIncrease); %> of <% Write (fMaxTechDev);
} else {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
    Write (fNextTechIncrease); %> of <% Write (fMaxTechDev); %></font><%
} %></td><%

%><td><strong>Ships:</strong></td><td><% Write (iNumShips); %></td><%

%></tr></table><%


%><p><font size="4"><strong>Ratios and Usage</strong></font><%
%><p><table width="90%"><%

%><tr><%

%><td><strong>Maintenance Ratio:</strong></td><td><% 

if (fMaintRatio < (float) 1.0) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><% Write (fMaintRatio); %></font><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><% Write (fMaintRatio); %></font><%
}
%></td><%

%><td><strong>Fuel Ratio:</strong></td><td><% 

if (fFuelRatio < (float) 1.0) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><% Write (fFuelRatio); %></font><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><% Write (fFuelRatio); %></font><%
}
%></td><%

%><td><strong>Agriculture ratio:</strong></td><td><%

if (fAgRatio < (float) 1.0) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><% Write (fAgRatio); %></font><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><% Write (fAgRatio); %></font><%
}
%></td><%

%></tr><tr><%

%><td><strong>Next Maintenance Ratio:</strong></td><td><%
if (fHypMaintRatio < (float) 1.0) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><% Write (fHypMaintRatio); %></font><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><% Write (fHypMaintRatio); %></font><%
}
%></td><%

%><td><strong>Next Fuel Ratio:</strong></td><td><% 
if (fHypFuelRatio < (float) 1.0) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><% Write (fHypFuelRatio); %></font><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><% Write (fHypFuelRatio); %></font><%
}
%></td><%

%><td><strong>Next Agriculture Ratio:</strong></td><td><% 
if (fHypAgRatio < (float) 1.0) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><% Write (fHypAgRatio); %></font><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><% Write (fHypAgRatio); %></font><%
}
%></td><%

%></tr><tr><%

%><td><strong>Total Maintenance Cost:</strong></td><%
%><td><% Write (pvEmpireData[GameEmpireData::TotalMaintenance]); %></td><%
%><td><strong>Total Fuel Use:</strong></td><%
%><td><% Write (pvEmpireData[GameEmpireData::TotalFuelUse]); %></td><%
%><td><strong>Total Build Cost:</strong></td><%
%><td><% Write (pvEmpireData[GameEmpireData::TotalBuild]);  %></td><%

%></tr><%
%></table><%


%><p><font size="4"><strong>Game Settings</strong></font><%
%><p><table width="90%"><%

%><tr><%

%><td><strong>Maximum Agriculture Ratio:</strong></td><%
%><td><% Write (vMaxAgRatio.GetFloat()); %></td><%

%><td><strong>Ship Limit:</strong></td><%
%><td><% 

if (iMaxNumShips == INFINITE_SHIPS) {
    %>None<%
} else {
    Write (iMaxNumShips); %> ships<%
}
 %></td><%

%><td><strong>Population Needed to Build Ships:</strong></td><%
%><td><% Write (vPopNeeded.GetInteger()); %></td><%

%></tr><%
%></table><%


Cleanup:

g_pGameEngine->FreeData (pvEmpireData);

if (iErrCode != OK) {
    %><p>Error <% Write (iErrCode); %>occurred reading from the database<%
}

GAME_CLOSE

%>