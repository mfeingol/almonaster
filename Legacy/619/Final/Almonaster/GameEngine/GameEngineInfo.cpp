//
// GameEngine.dll:  a component of Almonaster 2.0
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "GameEngine.h"

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppvEmpData -> Empire data
// 0	NumPlanets				INT			Number of systems empire owns
// 1	TotalAg					INT			Aggregate ag empire has
// 2	TotalFuel				INT			Aggregate fuel empire has
// 3	TotalMin				INT			Aggregate min empire has
// 4	TechLevel				FLOAT		Technological level of empire
// 5	TotalPop				INT			Aggregate pop empire has
// 6	TotalBuild				INT			Aggregate build
// 7	TotalMaintenance		INT			Aggregate maintenance
// 8	TotalFuelUse			INT			Aggregate fuel
// 9	LastLogin				INT			Last login time (last time a page was loaded from this game)
// 10	IPAddress				STRING		Empire's IP address
// 11	Updated					INT			1 if empire has updates, 0 if not
// 12	NumUnreadGameMessages	INT			Number of unread game messages on empire's queue
// 13	NumAvailableTechUndevs	INT			Number of available technologies remaining to be developed
// 14	Econ					INT			Empire's econ level
// 15	Mil						FLOAT		Empire's military level (sum of ship's mils)
// 16	TargetPop				INT			Empire's target pop
// 17	HomeWorld				INT			Key of empire's homeworld
// 18	NumUpdatesIdle			INT			Number of updates empire has been idle
// 19	MaxBR					INT			Max BR level empire has had
// 20	AgBonus					INT			Ag bonus from trade
// 21	FuelBonus				INT			Fuel bonus from trade
// 22	MinBonus				INT			Min bonus from trade

//
// *piNumShips -> Num ships
// *piBattleRank -> BR
// *piMilVal -> Mil
// *pfTechDev -> Tech development
// *pfMaintRatio -> Maintenance ratio
// *pfFuelRatio -> Fuel ratio
// *pfAgRatio -> Ag ratio
// *pfHypMaintRatio -> Next update's maintenance ratio
// *pfHypFuelRatio -> Next update's fuel ratio
// *pfHypAgRatio -> Next update's ag ratio
// *pfNextTechIncrease -> Next update's tech increase
//
// Returns "Info" data about an empire

int GameEngine::GetEmpireGameInfo (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvEmpData,
								   int* piNumShips, int* piBattleRank, int* piMilVal, float* pfTechDev, 
								   float* pfMaintRatio, float* pfFuelRatio, float* pfAgRatio, 
								   float* pfHypMaintRatio, float* pfHypFuelRatio, float* pfHypAgRatio,
								   float* pfNextTechIncrease) {

	int iErrCode;

	GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_SHIPS (strGameEmpireShips, iGameClass, iGameNumber, iEmpireKey);

	// Get the extras
	iErrCode = m_pGameData->GetNumRows (strGameEmpireShips, (unsigned int*) piNumShips);
	if (iErrCode != OK) {
		return iErrCode;
	}

	// Read the GameEmpireData
	Variant* pvEmpData;
	int iNextMin, iNextFuel, iNextMaint, iNextFuelUse;

	iErrCode = m_pGameData->ReadRow (strEmpireData, &pvEmpData);
	if (iErrCode != OK) {
		return iErrCode;
	}
	
	*piBattleRank = GetBattleRank (pvEmpData[GameEmpireData::TechLevel].GetFloat());
	*piMilVal = GetMilitaryValue (pvEmpData[GameEmpireData::Mil].GetFloat());

	Variant vMaxTechDev, vMaxAgRatio;
	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::MaxTechDev, 
		&vMaxTechDev
		);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::MaxAgRatio, 
		&vMaxAgRatio
		);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	// Tech dev
	*pfTechDev = GetTechDevelopment (
		pvEmpData[GameEmpireData::TotalFuel].GetInteger() + 
		pvEmpData[GameEmpireData::BonusFuel].GetInteger(), 
		
		pvEmpData[GameEmpireData::TotalMin].GetInteger() + 
		pvEmpData[GameEmpireData::BonusMin].GetInteger(), 
		
		pvEmpData[GameEmpireData::TotalMaintenance].GetInteger(), 

		pvEmpData[GameEmpireData::TotalBuild].GetInteger(), 
		
		pvEmpData[GameEmpireData::TotalFuelUse].GetInteger(), 
		
		vMaxTechDev.GetFloat()
		
		);

	// Maintenance ratio
	*pfMaintRatio = GetMaintenanceRatio (
		pvEmpData[GameEmpireData::TotalMin].GetInteger() + 
		pvEmpData[GameEmpireData::BonusMin].GetInteger(), 

		pvEmpData[GameEmpireData::TotalMaintenance].GetInteger(), 
		
		pvEmpData[GameEmpireData::TotalBuild].GetInteger()
		
		);
	
	// Fuel ratio
	*pfFuelRatio = GetFuelRatio (
		pvEmpData[GameEmpireData::TotalFuel].GetInteger() +
		pvEmpData[GameEmpireData::BonusFuel].GetInteger(),
		pvEmpData[GameEmpireData::TotalFuelUse].GetInteger()
		);

	// Ag ratio
	*pfAgRatio = GetAgRatio (
		pvEmpData[GameEmpireData::TotalAg].GetInteger() +
		pvEmpData[GameEmpireData::BonusAg].GetInteger(),
		pvEmpData[GameEmpireData::TotalPop].GetInteger(),
		vMaxAgRatio.GetFloat()
		);
	
	// Hypotheticals
	iNextMin = 
		pvEmpData[GameEmpireData::TotalMin].GetInteger() + 
		pvEmpData[GameEmpireData::BonusMin].GetInteger() + 
		pvEmpData[GameEmpireData::NextMin].GetInteger();

	iNextFuel = 
		pvEmpData[GameEmpireData::TotalFuel].GetInteger() + 
		pvEmpData[GameEmpireData::BonusFuel].GetInteger() + 
		pvEmpData[GameEmpireData::NextFuel].GetInteger();
	
	iNextMaint = 
		pvEmpData[GameEmpireData::TotalMaintenance].GetInteger() + 
		pvEmpData[GameEmpireData::NextMaintenance].GetInteger();

	iNextFuelUse = 
		pvEmpData[GameEmpireData::TotalFuelUse].GetInteger() + 
		pvEmpData[GameEmpireData::NextFuelUse].GetInteger();

	Assert (iNextMin >= 0 && iNextFuel >= 0 && iNextMaint >= 0 && iNextFuelUse >= 0);

	*pfHypMaintRatio = GetMaintenanceRatio (
		iNextMin, 
		iNextMaint,
		0
		);

	*pfHypFuelRatio = GetFuelRatio (
		iNextFuel, 
		iNextFuelUse
		);

	*pfHypAgRatio = GetAgRatio (
		pvEmpData[GameEmpireData::TotalAg].GetInteger() + 
		pvEmpData[GameEmpireData::BonusAg].GetInteger(), 
		pvEmpData[GameEmpireData::NextTotalPop].GetInteger(),
		vMaxAgRatio.GetFloat()
		);

	*pfNextTechIncrease = GetTechDevelopment (
		iNextFuel, 
		iNextMin, 
		iNextMaint, 
		0, 
		iNextFuelUse, 
		vMaxTechDev.GetFloat()
		);

	*ppvEmpData = pvEmpData;

Cleanup:

	if (iErrCode != OK) {
		delete [] pvEmpData;
	}

	return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Empire key
//
// Output:
// *pfAgRatio -> Empire's ag ratio in a particular game
//
// Return an empire's ag ratio in a particular game

int GameEngine::GetEmpireAgRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfAgRatio) {

	IReadTable* pGameEmpireData;
	
	GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireKey);

	int iErrCode = m_pGameData->GetTableForReading (pszEmpireData, &pGameEmpireData);
	if (iErrCode != OK) {
		return iErrCode;
	}

	int iTotalAg, iBonusAg, iTotalPop;

	iErrCode = pGameEmpireData->ReadData (GameEmpireData::TotalAg, &iTotalAg);
	if (iErrCode != OK) {
		pGameEmpireData->Release();
		return iErrCode;
	}

	iErrCode = pGameEmpireData->ReadData (GameEmpireData::BonusAg, &iBonusAg);
	if (iErrCode != OK) {
		pGameEmpireData->Release();
		return iErrCode;
	}

	iErrCode = pGameEmpireData->ReadData (GameEmpireData::TotalPop, &iTotalPop);
	if (iErrCode != OK) {
		pGameEmpireData->Release();
		return iErrCode;
	}

	pGameEmpireData->Release();

	Variant vMaxAgRatio;
	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::MaxAgRatio, 
		&vMaxAgRatio
		);
	Assert (iErrCode == OK);

	if (iErrCode == OK) {
		*pfAgRatio = GetAgRatio (iTotalAg + iBonusAg, iTotalPop, vMaxAgRatio.GetFloat());
	}

	return iErrCode;
}