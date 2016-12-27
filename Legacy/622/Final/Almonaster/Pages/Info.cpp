
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Info page
int HtmlRenderer::Render_Info() {

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


	Write ("<p><font size=\"4\"><strong>Game Information</strong></font><p><table width=\"90%\"><tr><td><strong>Updates:</strong></td><td>", sizeof ("<p><font size=\"4\"><strong>Game Information</strong></font><p><table width=\"90%\"><tr><td><strong>Updates:</strong></td><td>") - 1);
	if (bGameStarted) {
	    Write (m_iNumNewUpdates);
	} else { 
	    
	Write ("The game has not started", sizeof ("The game has not started") - 1);
	}


	Write ("</td><td align=\"right\"><strong>Last update:</strong></td><td align=\"center\">", sizeof ("</td><td align=\"right\"><strong>Last update:</strong></td><td align=\"center\">") - 1);
	if (bGameStarted && m_iNumNewUpdates > 0) {
	    WriteTime (m_sSecondsSince);
	    
	Write (" ago", sizeof (" ago") - 1);
	} else {
	    
	Write ("None", sizeof ("None") - 1);
	} 
	Write ("</td><td align=\"right\"><strong>Next update:</strong></td><td align=\"right\">", sizeof ("</td><td align=\"right\"><strong>Next update:</strong></td><td align=\"right\">") - 1);
	if (bGameStarted) {
	    WriteTime (m_sSecondsUntil);
	} else {
	    
	    int iMinNumEmpires, iTotal;
	    iErrCode = g_pGameEngine->GetNumEmpiresRequiredForGameToStart (m_iGameClass, &iMinNumEmpires);
	    if (iErrCode != OK) {
	        goto Cleanup;
	    }

	    iTotal = iMinNumEmpires - iActiveEmpires;
	    
	Write ("When <strong><font color=\"#", sizeof ("When <strong><font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (iTotal); 
	    
	Write ("</font></strong> more empire", sizeof ("</font></strong> more empire") - 1);
	Write (iTotal == 1 ? " joins" : "s join");
	} 
	Write ("</td></tr></table><br><table width=\"90%\"><tr><td align=\"center\">You are ", sizeof ("</td></tr></table><br><table width=\"90%\"><tr><td align=\"center\">You are ") - 1);
	if (m_iGameOptions & UPDATED) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">ready</font>", sizeof ("\">ready</font>") - 1);
	} else { 
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\"><strong>not ready</strong></font>", sizeof ("\"><strong>not ready</strong></font>") - 1);
	} 
	Write (" for an update</td><td align=\"center\">", sizeof (" for an update</td><td align=\"center\">") - 1);
	iErrCode = g_pGameEngine->IsGameOpen (m_iGameClass, m_iGameNumber, &bIsOpen);
	if (iErrCode != OK) {
	    goto Cleanup;
	}

	if (bIsOpen) {
	    
	Write ("The game is <strong><font color=", sizeof ("The game is <strong><font color=") - 1);
	Write (m_vBadColor.GetCharPtr());
	Write (">open</font></strong>", sizeof (">open</font></strong>") - 1);
	} else {
	    
	Write ("The game is <strong><font color=", sizeof ("The game is <strong><font color=") - 1);
	Write (m_vGoodColor.GetCharPtr());
	Write (">closed</font></strong>", sizeof (">closed</font></strong>") - 1);
	} 

	if (m_iGameState & ADMIN_PAUSED) {
	    
	Write (" and <strong><font color=\"#", sizeof (" and <strong><font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">paused by an administrator</font></strong>", sizeof ("\">paused by an administrator</font></strong>") - 1);
	}

	else if (m_iGameState & PAUSED) {
	    
	Write (" and <strong><font color=\"#", sizeof (" and <strong><font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr());
	Write ("\">paused</font></strong>", sizeof ("\">paused</font></strong>") - 1);
	}


	Write ("</td><td align=\"center\"><strong><font color=\"#", sizeof ("</td><td align=\"center\"><strong><font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (iUpdatedEmpires); 
	Write ("</font></strong> of <strong><font color=\"#", sizeof ("</font></strong> of <strong><font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (iActiveEmpires); 
	Write ("</font></strong> empire", sizeof ("</font></strong> empire") - 1);
	if (iActiveEmpires != 1) {
	    
	Write ("s", sizeof ("s") - 1);
	}

	if (iUpdatedEmpires == 1 || iActiveEmpires == 1) {
	    
	Write (" is", sizeof (" is") - 1);
	} else {
	    
	Write (" are", sizeof (" are") - 1);
	} 
	Write (" ready for an update</td></tr></table><p><font size=\"4\"><strong>Empire Totals</strong><p><table width=\"90%\"><tr><td width = \"20%\"><strong>Minerals:</strong></td><td>", sizeof (" ready for an update</td></tr></table><p><font size=\"4\"><strong>Empire Totals</strong><p><table width=\"90%\"><tr><td width = \"20%\"><strong>Minerals:</strong></td><td>") - 1);
	iMin = pvEmpireData[GameEmpireData::TotalMin].GetInteger() + pvEmpireData[GameEmpireData::BonusMin].GetInteger();
	Write (iMin); 
	Write ("</td><td>", sizeof ("</td><td>") - 1);
	iMinUsed = pvEmpireData[GameEmpireData::TotalBuild].GetInteger() + pvEmpireData[GameEmpireData::TotalMaintenance].GetInteger();
	if (iMin == 0) {
	    iRatio = 0;
	} else {
	    iRatio = (int) (100 * iMinUsed) / iMin;
	}
	if (iRatio > 100) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	}
	Write (iRatio); 
	Write ("%</font> in use</td><td><strong>Tech Level:</strong></td><td>", sizeof ("%</font> in use</td><td><strong>Tech Level:</strong></td><td>") - 1);
	Write (pvEmpireData[GameEmpireData::TechLevel]); 
	Write (" (BR ", sizeof (" (BR ") - 1);
	Write (iBattleRank); 
	Write (")</td><td width=\"20%\"><strong>Economic Level:</strong></td><td>", sizeof (")</td><td width=\"20%\"><strong>Economic Level:</strong></td><td>") - 1);
	Write (pvEmpireData[GameEmpireData::Econ]); 
	Write ("</td></tr><tr><td><strong>Fuel:</strong></td><td>", sizeof ("</td></tr><tr><td><strong>Fuel:</strong></td><td>") - 1);
	iFuel = pvEmpireData[GameEmpireData::TotalFuel].GetInteger() + pvEmpireData[GameEmpireData::BonusFuel].GetInteger();
	Write (iFuel); 
	Write ("</td><td>", sizeof ("</td><td>") - 1);
	if (iFuel == 0) {
	    iRatio = 0;
	} else {
	    iRatio = (100 * pvEmpireData[GameEmpireData::TotalFuelUse].GetInteger()) / iFuel;
	}
	if (iRatio > 100) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	}
	Write (iRatio);

	Write ("%</font> in use</td><td><strong>Tech Increase:</strong></td><td>", sizeof ("%</font> in use</td><td><strong>Tech Increase:</strong></td><td>") - 1);
	if (fTechDev < (float) 0.0) {

	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fTechDev); 
	Write (" of ", sizeof (" of ") - 1);
	Write (fMaxTechDev); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {

	    if (fTechDev == fMaxTechDev) {
	        
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fTechDev); 
	Write (" of ", sizeof (" of ") - 1);
	Write (fMaxTechDev); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	        Write (fTechDev); 
	Write (" of ", sizeof (" of ") - 1);
	Write (fMaxTechDev);
	    }
	}

	Write ("</td><td><strong>Military Level:</strong></td><td>", sizeof ("</td><td><strong>Military Level:</strong></td><td>") - 1);
	Write (iMilVal); 
	Write ("</td></tr><tr><td><strong>Agriculture:</strong></td><td>", sizeof ("</td></tr><tr><td><strong>Agriculture:</strong></td><td>") - 1);
	iAg = pvEmpireData[GameEmpireData::TotalAg].GetInteger() + pvEmpireData[GameEmpireData::BonusAg].GetInteger();
	Write (iAg); 
	Write ("</td><td>", sizeof ("</td><td>") - 1);
	if (iAg == 0) {
	    iRatio = 0;
	} else {
	    iRatio = (100 * pvEmpireData[GameEmpireData::TotalPop].GetInteger()) / iAg;
	}
	if (iRatio > 100) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	}
	Write (iRatio);

	Write ("%</font> in use</td><td></td></tr><tr><td><strong>Population:</strong></td><td>", sizeof ("%</font> in use</td><td></td></tr><tr><td><strong>Population:</strong></td><td>") - 1);
	Write (pvEmpireData[GameEmpireData::TotalPop].GetInteger()); 
	Write ("</td><td>", sizeof ("</td><td>") - 1);
	if (pvEmpireData[GameEmpireData::TargetPop].GetInteger() == 0) {
	    iRatio = 0;
	} else {
	    iRatio = (100 * pvEmpireData[GameEmpireData::TotalPop].GetInteger() / pvEmpireData[GameEmpireData::TargetPop].GetInteger());
	}
	if (iRatio > 100) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	}
	Write (iRatio); 

	Write ("%</font> of target</td><td><strong>Next Tech Level:</strong></td><td>", sizeof ("%</font> of target</td><td><strong>Next Tech Level:</strong></td><td>") - 1);
	fNextTechLevel = pvEmpireData[GameEmpireData::TechLevel].GetFloat() + fTechDev;
	iNextBattleRank = g_pGameEngine->GetBattleRank (fNextTechLevel);

	if (iNextBattleRank > iBattleRank) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fNextTechLevel); 
	Write (" (BR ", sizeof (" (BR ") - 1);
	Write (iNextBattleRank); 
	Write (")</font>", sizeof (")</font>") - 1);
	} else {
	    if (iNextBattleRank < iBattleRank) {
	        
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fNextTechLevel); 
	Write (" (BR ", sizeof (" (BR ") - 1);
	Write (iNextBattleRank); 
	Write (")</font>", sizeof (")</font>") - 1);
	} else {
	        Write (fNextTechLevel); 
	Write (" (BR ", sizeof (" (BR ") - 1);
	Write (iNextBattleRank); 
	Write (")", sizeof (")") - 1);
	}
	} 
	Write ("</td><td><strong>Planets:</strong></td><td>", sizeof ("</td><td><strong>Planets:</strong></td><td>") - 1);
	Write (pvEmpireData[GameEmpireData::NumPlanets]); 
	Write ("</td></tr><tr><td><strong>Target Population:</strong></td><td>", sizeof ("</td></tr><tr><td><strong>Target Population:</strong></td><td>") - 1);
	Write (pvEmpireData[GameEmpireData::TargetPop]); 
	Write ("</td><td>", sizeof ("</td><td>") - 1);
	if (iAg == 0) {
	    iRatio = 0;
	} else {
	    iRatio = (100 * pvEmpireData[GameEmpireData::TargetPop].GetInteger()) / iAg;
	}
	if (iRatio > 100) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	} else {

	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	}

	Write (iRatio); 
	Write ("%</font> of agriculture</td><td><strong>Next Tech Increase:</strong></td><td>", sizeof ("%</font> of agriculture</td><td><strong>Next Tech Increase:</strong></td><td>") - 1);
	if (fNextTechIncrease > (float) 0.0) {
	    Write (fNextTechIncrease); 
	Write (" of ", sizeof (" of ") - 1);
	Write (fMaxTechDev);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fNextTechIncrease); 
	Write (" of ", sizeof (" of ") - 1);
	Write (fMaxTechDev); 
	Write ("</font>", sizeof ("</font>") - 1);
	} 
	Write ("</td><td><strong>Ships:</strong></td><td>", sizeof ("</td><td><strong>Ships:</strong></td><td>") - 1);
	Write (iNumShips); 
	Write ("</td></tr></table><p><font size=\"4\"><strong>Ratios and Usage</strong></font><p><table width=\"90%\"><tr><td><strong>Maintenance Ratio:</strong></td><td>", sizeof ("</td></tr></table><p><font size=\"4\"><strong>Ratios and Usage</strong></font><p><table width=\"90%\"><tr><td><strong>Maintenance Ratio:</strong></td><td>") - 1);
	if (fMaintRatio < (float) 1.0) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fMaintRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fMaintRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	}

	Write ("</td><td><strong>Fuel Ratio:</strong></td><td>", sizeof ("</td><td><strong>Fuel Ratio:</strong></td><td>") - 1);
	if (fFuelRatio < (float) 1.0) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fFuelRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fFuelRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	}

	Write ("</td><td><strong>Agriculture ratio:</strong></td><td>", sizeof ("</td><td><strong>Agriculture ratio:</strong></td><td>") - 1);
	if (fAgRatio < (float) 1.0) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fAgRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fAgRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	}

	Write ("</td></tr><tr><td><strong>Next Maintenance Ratio:</strong></td><td>", sizeof ("</td></tr><tr><td><strong>Next Maintenance Ratio:</strong></td><td>") - 1);
	if (fHypMaintRatio < (float) 1.0) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fHypMaintRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fHypMaintRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	}

	Write ("</td><td><strong>Next Fuel Ratio:</strong></td><td>", sizeof ("</td><td><strong>Next Fuel Ratio:</strong></td><td>") - 1);
	if (fHypFuelRatio < (float) 1.0) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fHypFuelRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fHypFuelRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	}

	Write ("</td><td><strong>Next Agriculture Ratio:</strong></td><td>", sizeof ("</td><td><strong>Next Agriculture Ratio:</strong></td><td>") - 1);
	if (fHypAgRatio < (float) 1.0) {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vBadColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fHypAgRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	    
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (m_vGoodColor.GetCharPtr()); 
	Write ("\">", sizeof ("\">") - 1);
	Write (fHypAgRatio); 
	Write ("</font>", sizeof ("</font>") - 1);
	}

	Write ("</td></tr><tr><td><strong>Total Maintenance Cost:</strong></td><td>", sizeof ("</td></tr><tr><td><strong>Total Maintenance Cost:</strong></td><td>") - 1);
	Write (pvEmpireData[GameEmpireData::TotalMaintenance]); 
	Write ("</td><td><strong>Total Fuel Use:</strong></td><td>", sizeof ("</td><td><strong>Total Fuel Use:</strong></td><td>") - 1);
	Write (pvEmpireData[GameEmpireData::TotalFuelUse]); 
	Write ("</td><td><strong>Total Build Cost:</strong></td><td>", sizeof ("</td><td><strong>Total Build Cost:</strong></td><td>") - 1);
	Write (pvEmpireData[GameEmpireData::TotalBuild]);  
	Write ("</td></tr></table><p><font size=\"4\"><strong>Game Settings</strong></font><p><table width=\"90%\"><tr><td><strong>Maximum Agriculture Ratio:</strong></td><td>", sizeof ("</td></tr></table><p><font size=\"4\"><strong>Game Settings</strong></font><p><table width=\"90%\"><tr><td><strong>Maximum Agriculture Ratio:</strong></td><td>") - 1);
	Write (vMaxAgRatio.GetFloat()); 
	Write ("</td><td><strong>Ship Limit:</strong></td><td>", sizeof ("</td><td><strong>Ship Limit:</strong></td><td>") - 1);
	if (iMaxNumShips == INFINITE_SHIPS) {
	    
	Write ("None", sizeof ("None") - 1);
	} else {
	    Write (iMaxNumShips); 
	Write (" ships", sizeof (" ships") - 1);
	}
	 
	Write ("</td><td><strong>Population Needed to Build Ships:</strong></td><td>", sizeof ("</td><td><strong>Population Needed to Build Ships:</strong></td><td>") - 1);
	Write (vPopNeeded.GetInteger()); 
	Write ("</td></tr></table>", sizeof ("</td></tr></table>") - 1);
	Cleanup:

	g_pGameEngine->FreeData (pvEmpireData);

	if (iErrCode != OK) {
	    
	Write ("<p>Error ", sizeof ("<p>Error ") - 1);
	Write (iErrCode); 
	Write ("occurred reading from the database", sizeof ("occurred reading from the database") - 1);
	}

	GAME_CLOSE


}