<%

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
iErrCode = InitializeEmpireInGame(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
bool bRedirected;
iErrCode = InitializeGame(&pageRedirect, &bRedirected);
RETURN_ON_ERROR(iErrCode);
if (bRedirected)
{
    return Redirect(pageRedirect);
}

if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
    iErrCode = RedirectOnSubmitGame(&pageRedirect, &bRedirected);
    RETURN_ON_ERROR(iErrCode);
    if (bRedirected)
    {
        return Redirect (pageRedirect);
    }
}

iErrCode = OpenGamePage();
RETURN_ON_ERROR(iErrCode);

bool bGameStarted = (m_iGameState & STARTED) != 0, bIsOpen;

// Individual page stuff starts here
String strTime;
Variant* pvEmpireData = NULL, vMaxAgRatio, vPopNeeded;
AutoFreeData free_pvEmpireData(pvEmpireData);

int iNumShips, iBattleRank, iMilVal, iMin, iFuel, iAg, iUpdatedEmpires, iNextBattleRank, iRatio, iMinUsed, iMaxNumShips;
unsigned int iActiveEmpires;
float fTechDev, fMaintRatio, fFuelRatio, fAgRatio, fHypAgRatio, fHypMaintRatio, fHypFuelRatio, fNextTechIncrease, fNextTechLevel, fMaxTechDev;

iErrCode = GetNumUpdatedEmpires (m_iGameClass, m_iGameNumber, &iUpdatedEmpires);
RETURN_ON_ERROR(iErrCode);

iErrCode = GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iActiveEmpires);
RETURN_ON_ERROR(iErrCode);

iErrCode = GetEmpireGameInfo (m_iGameClass, m_iGameNumber, m_iEmpireKey, &pvEmpireData, &iNumShips,
    &iBattleRank, &iMilVal, &fTechDev, &fMaintRatio, &fFuelRatio, &fAgRatio, &fHypMaintRatio, &fHypFuelRatio, 
    &fHypAgRatio, &fNextTechIncrease, &iMaxNumShips);
RETURN_ON_ERROR(iErrCode);

iErrCode = GetGameClassMaxTechIncrease (m_iGameClass, &fMaxTechDev);
RETURN_ON_ERROR(iErrCode);

iErrCode = GetGameClassProperty (m_iGameClass, SystemGameClassData::MaxAgRatio, &vMaxAgRatio);
RETURN_ON_ERROR(iErrCode);

iErrCode = GetGameClassProperty (m_iGameClass, SystemGameClassData::BuilderPopLevel, &vPopNeeded);
RETURN_ON_ERROR(iErrCode);

if (bGameStarted && ShouldDisplayGameRatios())
{
    iErrCode = WriteRatiosString(NULL);
    RETURN_ON_ERROR(iErrCode);
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
    iErrCode = GetNumEmpiresRequiredForGameToStart (m_iGameClass, &iMinNumEmpires);
    RETURN_ON_ERROR(iErrCode);

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

iErrCode = IsGameOpen (m_iGameClass, m_iGameNumber, &bIsOpen);
RETURN_ON_ERROR(iErrCode);

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
iMin = pvEmpireData[GameEmpireData::iTotalMin].GetInteger() + pvEmpireData[GameEmpireData::iBonusMin].GetInteger();
Write (iMin); %></td><td><%
iMinUsed = pvEmpireData[GameEmpireData::iTotalBuild].GetInteger() + pvEmpireData[GameEmpireData::iTotalMaintenance].GetInteger();
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
Write (pvEmpireData[GameEmpireData::iTechLevel]); %> (BR <% Write (iBattleRank); %>)</td><%

%><td width="20%"><strong>Economic Level:</strong></td><td><% Write (pvEmpireData[GameEmpireData::iEcon]); %></td><%

%></tr><tr><%

%><td><strong>Fuel:</strong></td><td><% 
iFuel = pvEmpireData[GameEmpireData::iTotalFuel].GetInteger() + pvEmpireData[GameEmpireData::iBonusFuel].GetInteger();
Write (iFuel); %></td><td><%
if (iFuel == 0) {
    iRatio = 0;
} else {
    iRatio = (100 * pvEmpireData[GameEmpireData::iTotalFuelUse].GetInteger()) / iFuel;
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
iAg = pvEmpireData[GameEmpireData::iTotalAg].GetInteger() + pvEmpireData[GameEmpireData::iBonusAg].GetInteger();
Write (iAg); %></td><td><%
if (iAg == 0) {
    iRatio = 0;
} else {
    iRatio = (100 * pvEmpireData[GameEmpireData::iTotalPop].GetInteger()) / iAg;
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

%><td><strong>Population:</strong></td><td><% Write (pvEmpireData[GameEmpireData::iTotalPop].GetInteger()); %></td><td><%
if (pvEmpireData[GameEmpireData::iTargetPop].GetInteger() == 0) {
    iRatio = 0;
} else {
    iRatio = (100 * pvEmpireData[GameEmpireData::iTotalPop].GetInteger() / pvEmpireData[GameEmpireData::iTargetPop].GetInteger());
}
if (iRatio > 100) {
    %><font color="#<% Write (m_vBadColor.GetCharPtr()); %>"><%
} else {
    %><font color="#<% Write (m_vGoodColor.GetCharPtr()); %>"><%
}
Write (iRatio); 
%>%</font> of target</td><%

%><td><strong>Next Tech Level:</strong></td><td><% 
fNextTechLevel = pvEmpireData[GameEmpireData::iTechLevel].GetFloat() + fTechDev;
iNextBattleRank = GetBattleRank (fNextTechLevel);

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

%><td><strong>Planets:</strong></td><td><% Write (pvEmpireData[GameEmpireData::iNumPlanets]); %></td><%

%></tr><tr><%

%><td><strong>Target Population:</strong></td><td><% Write (pvEmpireData[GameEmpireData::iTargetPop]); %></td><td><%
if (iAg == 0) {
    iRatio = 0;
} else {
    iRatio = (100 * pvEmpireData[GameEmpireData::iTargetPop].GetInteger()) / iAg;
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
%><td><% Write (pvEmpireData[GameEmpireData::iTotalMaintenance]); %></td><%
%><td><strong>Total Fuel Use:</strong></td><%
%><td><% Write (pvEmpireData[GameEmpireData::iTotalFuelUse]); %></td><%
%><td><strong>Total Build Cost:</strong></td><%
%><td><% Write (pvEmpireData[GameEmpireData::iTotalBuild]);  %></td><%

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

iErrCode = CloseGamePage();
RETURN_ON_ERROR(iErrCode);

%>