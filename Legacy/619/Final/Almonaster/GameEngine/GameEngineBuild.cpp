//
// Almonaster.dll:  a component of Almonaster 2.0
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
// iTechKey -> Integer key of ship type
// iNumShips -> Number of ships to be built
// pszShipName -> Name to be given to ships
// fBR -> Max BR of ships
// iPlanetKey -> Key of planet on which ships are to be built
// iFleetKey -> Key of fleet into which ships are to be build
//
// Output:
// *piResult:
//	OK -> Ships were built
//	ERROR_GAME_HAS_NOT_STARTED -> Game hasn't started yet
//	ERROR_WRONG_OWNER -> Empire doesn't own the planet
//	ERROR_INSUFFICIENT_POPULATION -> Planet is not a builder
//	ERROR_WRONG_TECHNOLOGY -> Tech cannot be used
//	ERROR_INSUFFICIENT_TECH_LEVEL -> BR is too high
//  ERROR_WRONG_NUMBER_OF_SHIPS
// 
// Build ships of a given specification

// TODO: use transaction

int GameEngine::BuildNewShips (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey, int iNumShips, 
							   const char* pszShipName, float fBR, int iPlanetKey, int iFleetKey) {

	// Make sure game has started
	Variant vStarted;

	GAME_DATA (strGameData, iGameClass, iGameNumber);
	
	int iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vStarted);

	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (!(vStarted.GetInteger() & STARTED)) {
		return ERROR_GAME_HAS_NOT_STARTED;
	}

	// Make sure planet belongs to empire
	Variant vOwner;
	GAME_MAP (strGameMap, iGameClass, iGameNumber);
	iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);

	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (vOwner != iEmpireKey) {
		return ERROR_WRONG_OWNER;
	}

	// Make sure fleet exists
	char strEmpireFleets [256];
	if (iFleetKey != NO_KEY) {

		bool bExists;

		GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
		iErrCode = m_pGameData->DoesRowExist (strEmpireFleets, iFleetKey, &bExists);

		if (iErrCode != OK || !bExists) {
			return ERROR_FLEET_DOES_NOT_EXIST;
		}
	}

	// Get max num ships
	Variant vMaxNumShips;
	iErrCode = m_pGameData->ReadData (
		SYSTEM_EMPIRE_DATA,
		iEmpireKey,
		SystemEmpireData::MaxNumShipsBuiltAtOnce,
		&vMaxNumShips
		);

	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	// Check num ships
	if (iNumShips < 1 || iNumShips > vMaxNumShips.GetInteger()) {
		return ERROR_WRONG_NUMBER_OF_SHIPS;
	}

	// Make sure planet has enough pop to build
	Variant vPop, vBuilderPopLevel;

	iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Pop, &vPop);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::BuilderPopLevel, 
		&vBuilderPopLevel
		);

	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (vPop.GetInteger() < vBuilderPopLevel.GetInteger()) {
		return ERROR_INSUFFICIENT_POPULATION;
	}

	// Get ship behavior
	Variant vShipBehavior, vColonyMultipliedBuildFactor, vColonySimpleBuildFactor;

	iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ShipBehavior, &vShipBehavior);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ColonyMultipliedBuildFactor, &vColonyMultipliedBuildFactor);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ColonySimpleBuildFactor, &vColonySimpleBuildFactor);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	// Make sure planet has enough pop to build all the colonies requested (if any)
	Variant vPopLostToColonies;
	int iNewPopLostToColonies = 0;

	if (iTechKey == COLONY) {
		
		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::PopLostToColonies, &vPopLostToColonies);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		Assert (vPopLostToColonies.GetInteger() >= 0);

		iNewPopLostToColonies = iNumShips * GetColonyPopulationBuildCost (
			vShipBehavior.GetInteger(), 
			vColonyMultipliedBuildFactor.GetFloat(), 
			vColonySimpleBuildFactor.GetInteger(),
			fBR
			);

		Assert (iNewPopLostToColonies >= 0);

		if (vPop.GetInteger() - vPopLostToColonies.GetInteger() - iNewPopLostToColonies < 0) {
			return ERROR_INSUFFICIENT_POPULATION_FOR_COLONIES;
		}
	}

	// Make sure empire has the tech requested
	unsigned int iKey;
	GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

	Variant vTech;
	iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TechDevs, &vTech);

	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (!(vTech.GetInteger() & TECH_BITS [iTechKey])) {
		return ERROR_WRONG_TECHNOLOGY;
	}

	// Make sure BR is legal
	if (fBR < 1.0) {
		return ERROR_INVALID_TECH_LEVEL;
	}

	iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TechLevel, &vTech);

	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	if (GetBattleRank (vTech) < (int) fBR) {
		return ERROR_INVALID_TECH_LEVEL;
	}

	// Prepare data for insertion
	GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);

	int iBuildAction = (iFleetKey == NO_KEY) ? BUILD_AT : iFleetKey;
	Variant pvColVal[] = {
		pszShipName,
		iTechKey,
		fBR,
		fBR,
		iPlanetKey,
		iBuildAction,
		iFleetKey,
		1,
		iTechKey == MORPHER ? MORPH_ENABLED : 0,	// Set morpher flag
		NO_KEY,
		iNewPopLostToColonies / iNumShips
	};

	// Increment number of build ships on given system
	iErrCode = m_pGameData->GetFirstKey (strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, false, &iKey);
	if (iErrCode != OK) {
		Assert (false);
		return ERROR_WRONG_OWNER;
	}

	// Increment number of builds
	iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NumBuilds, iNumShips);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	//
	// Increment number of build ships on given system
	//

	// If ships are cloakers, increment cloaked ships counter and set cloaked to true
	if (iTechKey == CLOAKER && (vShipBehavior.GetInteger() & CLOAKER_CLOAK_ON_BUILD)) {

		pvColVal[GameEmpireShips::State] = pvColVal[GameEmpireShips::State].GetInteger() | CLOAKED;
		
		iErrCode = m_pGameData->Increment (strEmpireMap, iKey, GameEmpireMap::NumCloakedBuildShips, iNumShips);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumCloakedBuildShips, iNumShips);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

	} else {

		iErrCode = m_pGameData->Increment (strEmpireMap, iKey, GameEmpireMap::NumUncloakedBuildShips, iNumShips);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumUncloakedBuildShips, iNumShips);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
	}

	// Insert ships into table
	iErrCode = m_pGameData->InsertDuplicateRows (strEmpireShips, pvColVal, iNumShips);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	// Build into fleet if specified
	if (iFleetKey != NO_KEY) {

		// Increase power of fleet
		float fMil = (float) iNumShips * fBR * fBR;
		
		iErrCode = m_pGameData->Increment (strEmpireFleets, iFleetKey, GameEmpireFleets::CurrentStrength, fMil);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		iErrCode = m_pGameData->Increment (strEmpireFleets, iFleetKey, GameEmpireFleets::MaxStrength, fMil);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		// Increment number of buildships
		iErrCode = m_pGameData->Increment (strEmpireFleets, iFleetKey, GameEmpireFleets::BuildShips, iNumShips);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		// Set the fleet to stand by
		iErrCode = m_pGameData->WriteData (strEmpireFleets, iFleetKey, GameEmpireFleets::Action, STAND_BY);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
	}

	// Increase build total
	int iBuild = GetBuildCost (iTechKey, fBR) * iNumShips;
	iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::TotalBuild, iBuild);
	Assert (iErrCode == OK);

	// Increase next maintenance and fuel totals
	iErrCode = m_pGameData->Increment (
		strEmpireData, 
		GameEmpireData::NextMaintenance, 
		iNumShips * GetMaintenanceCost (iTechKey, fBR)
		);
	Assert (iErrCode == OK);

	iErrCode = m_pGameData->Increment (
		strEmpireData, 
		GameEmpireData::NextFuelUse, 
		iNumShips * GetFuelCost (iTechKey, fBR)
		);
	Assert (iErrCode == OK);

	// Set last builder
	iErrCode = m_pGameData->WriteData (
		strEmpireData,
		GameEmpireData::LastBuilderPlanet,
		iPlanetKey
		);
	Assert (iErrCode == OK);

	// Reduce pop and resources at planet if ships are colony
	if (iTechKey == COLONY) {

		Variant vOldColsBuilding, vMin, vFuel, vMaxPop, vTotalAg, vTotalPop;

		iNewPopLostToColonies += vPopLostToColonies.GetInteger();
		Assert (iNewPopLostToColonies >= 0);

		// Increment pop lost to colony build counter
		iErrCode = m_pGameData->WriteData (
			strGameMap, 
			iPlanetKey, 
			GameMap::PopLostToColonies, 
			iNewPopLostToColonies
			);
		Assert (iErrCode == OK);

		// Calculate pop change difference
		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
		Assert (iErrCode == OK);
		iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalAg, &vTotalAg);
		Assert (iErrCode == OK);
		iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalPop, &vTotalPop);
		Assert (iErrCode == OK);

		Variant vMaxAgRatio;
		iErrCode = m_pGameData->ReadData (
			SYSTEM_GAMECLASS_DATA, 
			iGameClass, 
			SystemGameClassData::MaxAgRatio, 
			&vMaxAgRatio
			);
		Assert (iErrCode == OK);

		float fAgRatio = GetAgRatio (vTotalAg.GetInteger(), vTotalPop.GetInteger(), vMaxAgRatio.GetFloat());

		// Calculate what next pop will be after this
		int iNewNextPop = GetNextPopulation (
			vPop.GetInteger() - iNewPopLostToColonies,
			fAgRatio
			);

		if (iNewNextPop > vMaxPop.GetInteger()) {
			iNewNextPop = vMaxPop.GetInteger();
		}

		// Calculate what next pop used to be
		int iOldNextPop = GetNextPopulation (
			vPop.GetInteger() - vPopLostToColonies.GetInteger(),
			fAgRatio
			);

		if (iOldNextPop > vMaxPop.GetInteger()) {
			iOldNextPop = vMaxPop.GetInteger();
		}

		// This is the difference
		int iNextPopDiff = iNewNextPop - iOldNextPop;

		// Change next pop
		Variant vOldNextPop;
		iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextTotalPop, iNextPopDiff, &vOldNextPop);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		// Get planet data
		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		int iDiff;

		iDiff = min (vOldNextPop.GetInteger() + iNextPopDiff, vMin.GetInteger()) - 
				min (vOldNextPop.GetInteger(), vMin.GetInteger());
		
		if (iDiff != 0) {
			iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextMin, iDiff);
			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}
		}
		
		iDiff = min (vOldNextPop.GetInteger() + iNextPopDiff, vFuel.GetInteger()) - 
			min (vOldNextPop.GetInteger(), vFuel.GetInteger());
		
		if (iDiff != 0) {
			iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextFuel, iDiff);
			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}
		}
	}

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumBuilds -> Number of builds
//
// Get the number of ships being built by the empire in the given game

int GameEngine::GetNumBuilds (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumBuilds) {

	Variant vTemp;
	GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireKey)

	int iErrCode = m_pGameData->ReadData (pszEmpireData, GameEmpireData::NumBuilds, &vTemp);
	if (iErrCode == OK) {
		*piNumBuilds = vTemp.GetInteger();
	}

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Cancel build on all ships being built by the empire in the given game

int GameEngine::CancelAllBuilds (int iGameClass, int iGameNumber, int iEmpireKey) {

	GAME_EMPIRE_SHIPS (strShips, iGameClass, iGameNumber, iEmpireKey);

	unsigned int i, iNumShips, * piShipKey = NULL;

	int iErrCode = m_pGameData->GetEqualKeys (
		strShips, 
		GameEmpireShips::BuiltThisUpdate, 
		1, 
		false, 
		&piShipKey, 
		&iNumShips
		);

	if (iNumShips > 0) {

		for (i = 0; i < iNumShips; i ++) {

			// Best effort
			iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, piShipKey[i], false);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
	}

Cleanup:

	if (piShipKey != NULL) {
		m_pGameData->FreeKeys (piShipKey);
	}

	return iErrCode;
}