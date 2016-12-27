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

// TODO: transactions?

// Delete a ship from a game
int GameEngine::DeleteShip (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, bool bSafe) {

	int iErrCode, iErrCode2;
	unsigned int iKey;

	GAME_MAP (strGameMap, iGameClass, iGameNumber);

	Variant vCancelBuild, vFleetKey, vState, vTechKey, vPlanetKey, vMaxBR;
	NamedMutex nmMutex;

	if (!bSafe) {
		LockEmpireShips (iGameClass, iGameNumber, iEmpireKey, &nmMutex);
	}

	// If independent, just delete ship from table
	if (iEmpireKey == INDEPENDENT) {

		GAME_INDEPENDENT_SHIPS (strIndependentShips, iGameClass, iGameNumber);

		// Get ship's locaton
		Variant vPlanetKey;
		iErrCode = m_pGameData->ReadData (strIndependentShips, iShipKey, GameEmpireShips::CurrentPlanet, &vPlanetKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->Increment (strGameMap, vPlanetKey, GameMap::NumUncloakedShips, -1);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

#ifdef _DEBUG
		Variant vTemp;
		iErrCode = m_pGameData->ReadData (strGameMap, vPlanetKey, GameMap::NumUncloakedShips, &vTemp);
		Assert (iErrCode == OK && vTemp >= 0);
#endif

		iErrCode = m_pGameData->DeleteRow (strIndependentShips, iShipKey);

		if (iErrCode != OK) {
			Assert (false);
			
			// Compensate
			iErrCode2 = m_pGameData->Increment (strGameMap, vPlanetKey, GameMap::NumUncloakedShips, 1);
			Assert (iErrCode2 == OK);
		}

		goto Cleanup;
	}

	GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

	// Was ship just built?
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vCancelBuild);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Was ship in a fleet?
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vFleetKey);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Was ship cloaked?
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::State, &vState);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Get ship's type
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::Type, &vTechKey);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Get ship's locaton
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentPlanet, &vPlanetKey);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// If ship is in a fleet, remove it
	if (vFleetKey.GetInteger() != NO_KEY) {
		
		GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

		// Get ship's max mil
		float fMaxMil = vMaxBR.GetFloat() * vMaxBR.GetFloat();
		
		// Get ship's current mil
		Variant vCurMil;
		iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vCurMil);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		// Reduce fleet's current strength
		iErrCode = m_pGameData->Increment (
			strEmpireFleets, 
			vFleetKey.GetInteger(), 
			GameEmpireFleets::CurrentStrength, 
			- vCurMil.GetFloat() * vCurMil.GetFloat()
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		// Reduce fleet's max strength
		iErrCode = m_pGameData->Increment (
			strEmpireFleets, 
			vFleetKey.GetInteger(), 
			GameEmpireFleets::MaxStrength, 
			- fMaxMil
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		// Decrement number of "BuildShips" if necessary
		if (vCancelBuild.GetInteger() != 0) {
			iErrCode = m_pGameData->Increment (strEmpireFleets, vFleetKey, GameEmpireFleets::BuildShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
	}

	GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

	// Increase pop at planet if ship is colony and is a cancel-build
	if (vTechKey.GetInteger() == COLONY && vCancelBuild.GetInteger() != 0) {

		Variant vOldPopLostToColonies, vMin, vFuel, vPop, vMaxPop, vPopLostToColonies, vTotalAg, vTotalPop, 
			vMaxAgRatio, vCost;

		int iPlanetKey = vPlanetKey.GetInteger();

		iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::ColonyBuildCost, &vCost);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Pop, &vPop);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->Increment(
			strGameMap,
			iPlanetKey,
			GameMap::PopLostToColonies,
			- vCost.GetInteger(),
			&vPopLostToColonies
			);

		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		Assert (vCost.GetInteger() >= 0);
		Assert (vPopLostToColonies.GetInteger() >= 0);
		Assert (vPopLostToColonies.GetInteger() - vCost.GetInteger() >= 0);

		iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalAg, &vTotalAg);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalPop, &vTotalPop);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->ReadData (
			SYSTEM_GAMECLASS_DATA, 
			iGameClass, 
			SystemGameClassData::MaxAgRatio, 
			&vMaxAgRatio
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		// Calculate ag ratio
		float fAgRatio = GetAgRatio (vTotalAg.GetInteger(), vTotalPop.GetInteger(), vMaxAgRatio.GetFloat());

		// Calculate what next pop will be after this
		int iNewNextPop = GetNextPopulation (
			vPop.GetInteger() - vPopLostToColonies.GetInteger() + vCost.GetInteger(),
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
		int iDiff, iNextPopDiff = iNewNextPop - iOldNextPop;

		// Change next pop
		Variant vOldNextPop;
		iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextTotalPop, iNextPopDiff, &vOldNextPop);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		// Get planet data
		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iDiff = min (vOldNextPop.GetInteger() + iNextPopDiff, vMin.GetInteger()) - 
				min (vOldNextPop.GetInteger(), vMin.GetInteger());
		
		if (iDiff != 0) {
			iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextMin, iDiff);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
		
		iDiff = min (vOldNextPop.GetInteger() + iNextPopDiff, vFuel.GetInteger()) - 
				min (vOldNextPop.GetInteger(), vFuel.GetInteger());
		
		if (iDiff != 0) {
			iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextFuel, iDiff);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
	}

	// Decrease empire's resource usage
	iErrCode = m_pGameData->GetFirstKey (strEmpireMap, GameEmpireMap::PlanetKey, vPlanetKey, false, &iKey);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (vCancelBuild.GetInteger() != 0) {
		
		// Decrease number of builds
		iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NumBuilds, -1);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		// Decrease total build
		int iBuildCost = GetBuildCost (vTechKey.GetInteger(), vMaxBR.GetFloat());
		
		iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::TotalBuild, - iBuildCost);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (vState.GetInteger() & CLOAKED) {
			
			iErrCode = m_pGameData->Increment (strEmpireMap, iKey, GameEmpireMap::NumCloakedBuildShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->Increment (strGameMap, vPlanetKey, GameMap::NumCloakedBuildShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
		} else {
			
			iErrCode = m_pGameData->Increment (strEmpireMap, iKey, GameEmpireMap::NumUncloakedBuildShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->Increment (strGameMap, vPlanetKey, GameMap::NumUncloakedBuildShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}

		iErrCode = m_pGameData->Increment (
			strEmpireData, 
			GameEmpireData::NextMaintenance, 
			- GetMaintenanceCost (vTechKey.GetInteger(), vMaxBR.GetFloat())
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->Increment (
			strEmpireData, 
			GameEmpireData::NextFuelUse, 
			- GetFuelCost (vTechKey.GetInteger(), vMaxBR.GetFloat())
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

	} else {
		
		int iMaintCost = GetMaintenanceCost (vTechKey.GetInteger(), vMaxBR.GetFloat());
		int iFuelCost = GetFuelCost (vTechKey.GetInteger(), vMaxBR.GetFloat());

		iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::TotalMaintenance, - iMaintCost);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::TotalFuelUse, - iFuelCost);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
#ifdef _DEBUG
		Variant vTotalFuelUse;
		iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalFuelUse, &vTotalFuelUse);
		Assert (vTotalFuelUse.GetInteger() >= 0);
#endif
		if (vState.GetInteger() & CLOAKED) {
			
			iErrCode = m_pGameData->Increment (strEmpireMap, iKey, GameEmpireMap::NumCloakedShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->Increment (strGameMap, vPlanetKey, GameMap::NumCloakedShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
		} else {
			
			iErrCode = m_pGameData->Increment (strEmpireMap, iKey, GameEmpireMap::NumUncloakedShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->Increment (strGameMap, vPlanetKey, GameMap::NumUncloakedShips, -1);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
	}
	
	// Delete ship from table
	iErrCode = m_pGameData->DeleteRow (strEmpireShips, iShipKey);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

Cleanup:

	if (!bSafe) {
		UnlockEmpireShips (nmMutex);
	}

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *pfMaintRatio -> Maintenance ratio
// *pfFuelRatio -> Fuel ratio
// *pfTechLevel -> Tech level
// *pfTechDev -> Tech development
// *piBR -> BR
//
// Returns the data listed above

int GameEngine::GetShipRatios (int iGameClass, int iGameNumber, int iEmpireKey, float* pfMaintRatio, 
							   float* pfFuelRatio, float* pfTechLevel, float* pfTechDev, int* piBR) {

	int iBuild, iMaint, iFuel, iBonusFuel, iMin, iBonusMin, iFuelUse;
	Variant vMaxTechDev;

	GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

	IReadTable* pTable;
	int iErrCode = m_pGameData->GetTableForReading (
		strGameEmpireData,
		&pTable
		);
	if (iErrCode != OK) {
		return iErrCode;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalBuild, &iBuild);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalMaintenance, &iMaint);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalFuelUse, &iFuelUse);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalFuel, &iFuel);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::BonusFuel, &iBonusFuel);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalMin, &iMin);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::BonusMin, &iBonusMin);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TechLevel, pfTechLevel);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::MaxTechDev, 
		&vMaxTechDev
		);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	// Return values
	*pfMaintRatio = GetMaintenanceRatio (iMin + iBonusMin, iMaint, iBuild);
	
	*pfFuelRatio = GetFuelRatio (iFuel + iBonusFuel, iFuelUse);
	
	*pfTechDev = GetTechDevelopment (iFuel + iBonusFuel, iMin + iBonusMin, iMaint, iBuild, iFuelUse, 
		vMaxTechDev.GetFloat());
	
	*piBR = GetBattleRank (*pfTechLevel);

Cleanup:

	pTable->Release();

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *pfNextMaintRatio -> Next maintenance ratio
// *pfNextFuelRatio -> Next fuel ratio
// *pfNextTechLevel -> Next tech level
// *pfNextTechDev -> Next tech development
// *piNextBR -> Next BR
//
// Returns the data listed above

int GameEngine::GetNextShipRatios (int iGameClass, int iGameNumber, int iEmpireKey, float* pfNextMaintRatio, 
								   float* pfNextFuelRatio, float* pfNextTechLevel, float* pfNextTechDev, 
								   int* piNextBR) {

	float fTechLevel, fTechDev;
	int iBuild, iMaint, iFuel, iMin, iFuelUse, iNextMaint, iNextFuelUse, iNextMin, iNextFuel, iBonusFuel, 
		iBonusMin;

	Variant vMaxTechDev;

	GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

	IReadTable* pTable;
	int iErrCode = m_pGameData->GetTableForReading (
		strGameEmpireData, 
		&pTable
		);
	if (iErrCode != OK) {
		return iErrCode;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalBuild, &iBuild);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalMaintenance, &iMaint);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalFuelUse, &iFuelUse);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalFuel, &iFuel);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TotalMin, &iMin);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::TechLevel, &fTechLevel);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::BonusFuel, &iBonusFuel);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::BonusMin, &iBonusMin);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::NextMaintenance, &iNextMaint);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::NextFuelUse, &iNextFuelUse);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::NextMin, &iNextMin);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = pTable->ReadData (GameEmpireData::NextFuel, &iNextFuel);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::MaxTechDev, 
		&vMaxTechDev
		);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	// Return values
	*pfNextMaintRatio = GetMaintenanceRatio (
		iMin + iBonusMin + iNextMin, 
		iMaint + iNextMaint,
		0
		);

	*pfNextFuelRatio = GetFuelRatio (
		iFuel + iBonusFuel + iNextFuel, 
		iFuelUse + iNextFuelUse
		);

	fTechDev = GetTechDevelopment (iFuel + iBonusFuel, iMin + iBonusMin, iMaint, iBuild, iFuelUse, 
		vMaxTechDev.GetFloat());
	*pfNextTechLevel = fTechLevel + fTechDev;
	*piNextBR = GetBattleRank (*pfNextTechLevel);

	*pfNextTechDev = GetTechDevelopment (
		iFuel + iBonusFuel + iNextFuel, 
		iMin + iBonusMin + iNextMin, 
		iMaint + iNextMaint, 
		0, 
		iFuelUse + iNextFuelUse, 
		vMaxTechDev.GetFloat()
		);

Cleanup:

	pTable->Release();

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iShipKey -> Key of ship
// fBR -> Current BR of ship
// iPlanetKey -> Key of ship's location planet
// iLocationX -> Current X location of ship
// iLocationY -> Current Y location of ship
// iShipType -> Type of ship
//
// Output:
// **ppiOrderKey -> Integer keys of orders
// **ppstrOrderText -> Text of orders
// *piNumOrders -> Number of orders
// *piSelected -> Key of selected order
//
// Returns the orders a given ship has, as well as which order was selected

int GameEngine::GetShipOrders (int iGameClass, int iGameNumber, int iEmpireKey, const GameConfiguration& gcConfig, 
							   int iShipKey, int iShipType, float fBR, float fMaintRatio, int iPlanetKey, 
							   int iLocationX, int iLocationY, int** ppiOrderKey, String** ppstrOrderText, 
							   int* piNumOrders, int* piSelected) {

	GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
	GAME_MAP (strGameMap, iGameClass, iGameNumber);
	GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

	bool pbLink[NUM_CARDINAL_POINTS];
	bool pbMustExplore [NUM_CARDINAL_POINTS];

	char strEmpireMap [256], strEmpireDip [256];

	unsigned int iNumFleets, iNumPlanets, iMaxNumOrders, i, iProxyPlanetKey;
	int iNewX, iNewY, iOrderKey, iErrCode;

	Variant vTemp, vPlanetName, vOwner, vState, vGameClassOptions;

	char pszOrder [MAX_PLANET_NAME_LENGTH + MAX_FLEET_NAME_LENGTH + 256];

	*ppiOrderKey = NULL;
	*ppstrOrderText = NULL;

	// Get ship's next BR
	float fNextBR = GetShipNextBR (fBR, fMaintRatio);
	
	// Get max BR
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (fNextBR > vTemp.GetFloat()) {
		fNextBR = vTemp.GetFloat();
	}

	// Get currently selected order
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::Action, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	*piSelected = vTemp.GetInteger();

	// Get gameclass options
	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		&vGameClassOptions
		);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Get planet name
	iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Get ship state
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::State, &vState);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Check for newly built ship
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (vTemp.GetInteger() != 0) {

		// Number of order == number of fleets + 2
		unsigned int i, iNumRows;
		iErrCode = m_pGameData->GetNumRows (strEmpireFleets, &iNumRows);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		*ppiOrderKey = new int [iNumRows + 2];
		if (*ppiOrderKey == NULL) {
			iErrCode = ERROR_OUT_OF_MEMORY;
			goto Cleanup;
		}

		*ppstrOrderText = new String [iNumRows + 2];
		if (*ppstrOrderText == NULL) {
			iErrCode = ERROR_OUT_OF_MEMORY;
			goto Cleanup;
		}

		sprintf (pszOrder, "Build at %s (%i,%i)", vPlanetName.GetCharPtr(), iLocationX, iLocationY);

		(*ppiOrderKey)[0] = BUILD_AT;
		(*ppstrOrderText)[0] = pszOrder;
		*piNumOrders = 1;
		
		// If the ship isn't stationary, add move into all fleets located at planet
		if (IsMobileShip (iShipType)) {

			unsigned int* piFleetKey, iNumFleets, iTemp;
			iErrCode = m_pGameData->GetEqualKeys (
				strEmpireFleets, 
				GameEmpireFleets::CurrentPlanet, 
				iPlanetKey, 
				false, 
				&piFleetKey, 
				&iTemp
				);

			if (iErrCode != OK) {
				
				if (iErrCode == ERROR_DATA_NOT_FOUND) {
					iErrCode = OK;
				} else {
					Assert (false);
					goto Cleanup;
				}
			}
		
			iNumFleets = min (iTemp, iNumRows);

			Variant vFleetName;
			for (i = 0; i < iNumFleets; i ++) {
				
				iErrCode = m_pGameData->ReadData (strEmpireFleets, piFleetKey[i], GameEmpireFleets::Name, &vFleetName);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				sprintf (pszOrder, "%s in fleet %s", (*ppstrOrderText)[0].GetCharPtr(), vFleetName.GetCharPtr());

				(*ppiOrderKey)[*piNumOrders] = piFleetKey[i];
				(*ppstrOrderText)[*piNumOrders] = pszOrder;
				(*piNumOrders) ++;
			}

			if (iTemp > 0) {
				m_pGameData->FreeKeys (piFleetKey);
			}
		}

		// Add cancel build order
		(*ppiOrderKey)[*piNumOrders] = CANCEL_BUILD;
		(*ppstrOrderText)[*piNumOrders] = "Cancel Build";
		(*piNumOrders) ++;

		return iErrCode;
	}
	
	//////////////////////////////////////////////////////
	// Here begin the regular ship orders in Almonaster //
	//////////////////////////////////////////////////////

	GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
	
	////////////////////////////////
	// Allocate memory for orders //
	////////////////////////////////
	
	// Calculate the max number of orders we'll ever have
	iErrCode = m_pGameData->GetNumRows (strEmpireFleets, &iNumFleets);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	iErrCode = m_pGameData->GetNumRows (strEmpireMap, &iNumPlanets);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	iMaxNumOrders = 8 + iNumFleets + max (iNumPlanets, NUM_CARDINAL_POINTS) + NUM_SHIP_TYPES;
	
	*ppiOrderKey = new int [iMaxNumOrders];
	if (*ppiOrderKey == NULL) {
		iErrCode = ERROR_OUT_OF_MEMORY;
		goto Cleanup;
	}
	
	*ppstrOrderText = new String [iMaxNumOrders];
	if (*ppstrOrderText == NULL) {
		iErrCode = ERROR_OUT_OF_MEMORY;
		goto Cleanup;
	}
	
	///////////////////
	// Add "Standby" //
	///////////////////
	
	sprintf (pszOrder, "Standby at %s (%i,%i)", vPlanetName.GetCharPtr(), iLocationX, iLocationY);

	(*ppiOrderKey)[0] = STAND_BY;
	(*ppstrOrderText)[0] = pszOrder;
	*piNumOrders = 1;
	
	///////////////////////
	// Add mobile orders //
	///////////////////////
	
	// Get proxy key of planet from EmpireMap
	iErrCode = m_pGameData->GetFirstKey (strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, false, &iProxyPlanetKey);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	GET_GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

	// Select only mobile ships
	if (IsMobileShip (iShipType)) {
		
		//////////
		// Move //
		//////////

		Variant vLink, vExplored, vNewPlanetKey;
		Variant vNewPlanetName;

		// Read explored
		iErrCode = m_pGameData->ReadData (
			strEmpireMap, 
			iProxyPlanetKey, 
			GameEmpireMap::Explored, 
			&vExplored
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		// Loop through all cardinal points
		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Link, &vLink);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		ENUMERATE_CARDINAL_POINTS(i) {

			// Is there a link in this direction?
			pbLink[i] = ((vLink.GetInteger() & LINK_X[i]) != 0);
			if (pbLink[i]) {

				pbMustExplore[i] = !(vExplored.GetInteger() & EXPLORED_X[i]);
				if (!pbMustExplore[i]) {
				
					// Read planet key
					iErrCode = m_pGameData->ReadData (
						strGameMap, 
						iPlanetKey, 
						GameMap::NorthPlanetKey + i, 
						&vNewPlanetKey
						);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					Assert (vNewPlanetKey.GetInteger() != NO_KEY);
					
					// Get name
					iErrCode = GetPlanetName (iGameClass, iGameNumber, vNewPlanetKey, &vNewPlanetName);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					AdvanceCoordinates (iLocationX, iLocationY, &iNewX, &iNewY, i);

					// Add order
					iOrderKey = MOVE_NORTH - i;

					sprintf (
						pszOrder, 
						"Move %s to %s (%i,%i)", 
						CARDINAL_STRING[i], 
						vNewPlanetName.GetCharPtr(), 
						iNewX, 
						iNewY
						);

					(*ppiOrderKey)[*piNumOrders] = iOrderKey;
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;
				}

			} else {

				pbMustExplore[i] = false;
			}
		}	// End cardinal loop
		

		//////////
		// Nuke //
		//////////
				
		// Check the owner
		iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		if (vOwner.GetInteger() != iEmpireKey && vOwner.GetInteger() != SYSTEM) {
			
			// We're on someone else's planet, so check the dip status
			Variant vDipStatus;

			if (vOwner.GetInteger() == INDEPENDENT) {
				vDipStatus = WAR;
			} else {
				
				unsigned int iKey;
				iErrCode = m_pGameData->GetFirstKey (strEmpireDip, GameEmpireDiplomacy::EmpireKey, vOwner, false, &iKey);
				
				if (iErrCode == ERROR_DATA_NOT_FOUND) {
					iErrCode = OK;
					vDipStatus = WAR;
				} else {
					
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					iErrCode = m_pGameData->ReadData (strEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
				}
			}
				
			if (vDipStatus.GetInteger() == WAR) {
				
				// Well, we're at war, so if the ship isn't cloaked we can nuke
				if (!(vState.GetInteger() & CLOAKED)) {

					sprintf (
						pszOrder, 
						"Nuke %s (%i,%i)", 
						vPlanetName.GetCharPtr(), 
						iLocationX, 
						iLocationY
						);
					
					(*ppiOrderKey)[*piNumOrders] = NUKE;
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;
				}
			}
		}
	}	// End if mobile ship


	/////////////////////////
	// Add Special Actions //
	/////////////////////////

	switch (iShipType) {

	case SCIENCE:

		{
			int piExplore [] = {
				EXPLORE_NORTH,
				EXPLORE_EAST,
				EXPLORE_SOUTH,
				EXPLORE_WEST
			};
			
			ENUMERATE_CARDINAL_POINTS(i) {
				
				if (pbMustExplore[i]) {
#ifdef _DEBUG
					Variant vNewPlanetKey;
					iErrCode = m_pGameData->ReadData (
						strGameMap, 
						iPlanetKey, 
						GameMap::NorthPlanetKey + i, 
						&vNewPlanetKey
						);
					Assert (vNewPlanetKey.GetInteger() != NO_KEY);
#endif		
					AdvanceCoordinates (iLocationX, iLocationY, &iNewX, &iNewY, i);
					
					sprintf (
						pszOrder, 
						"Explore %s to Planet %i,%i", 
						CARDINAL_STRING[i],
						iNewX, 
						iNewY
						);

					(*ppiOrderKey)[*piNumOrders] = piExplore[i];
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;
				}
			}
		}
		
		break;

	case COLONY:
		
		{
			Variant vPop, vAnnihilated, vOwner;
			
			iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Pop, &vPop);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Annihilated, &vAnnihilated);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			// We can colonize a planet if the pop is zero, we don't own it and it hasn't been annihilated
			if (vPop.GetInteger() == 0 && 
				vOwner.GetInteger() != iEmpireKey && 
				vAnnihilated.GetInteger() == NOT_ANNIHILATED
				) {
				
				// Colonize planet
				if (!(gcConfig.iShipBehavior & COLONY_DISABLE_SURVIVAL)) {

					sprintf (pszOrder, "Colonize %s (%i,%i)", vPlanetName.GetCharPtr(), iLocationX, iLocationY);

					(*ppiOrderKey)[*piNumOrders] = COLONIZE;
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;
				}

				// Colonize planet and dismantle
				sprintf (pszOrder, "Colonize %s (%i,%i) and dismantle", vPlanetName.GetCharPtr(), iLocationX, iLocationY);

				(*ppiOrderKey)[*piNumOrders] = COLONIZE_AND_DISMANTLE;
				(*ppstrOrderText)[*piNumOrders] = pszOrder;
				(*piNumOrders) ++;

				if (!(gcConfig.iShipBehavior & COLONY_DISABLE_SETTLES)) {
				
					// Colonize or deposit pop
					if (!(gcConfig.iShipBehavior & COLONY_DISABLE_SURVIVAL)) {
					
						sprintf (pszOrder, "Colonize or settle %s (%i,%i)", vPlanetName.GetCharPtr(), iLocationX, iLocationY);
						
						(*ppiOrderKey)[*piNumOrders] = COLONIZE_OR_DEPOSIT_POP;
						(*ppstrOrderText)[*piNumOrders] = pszOrder;
						(*piNumOrders) ++;
					}
					
					// Colonize or deposit pop and dismantle
					sprintf (pszOrder, "Colonize or settle %s (%i,%i) and dismantle", vPlanetName.GetCharPtr(), iLocationX, iLocationY);
					
					(*ppiOrderKey)[*piNumOrders] = COLONIZE_OR_DEPOSIT_POP_AND_DISMANTLE;
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;
				}

			} else {
				
				// Maybe we can deposit pop on our planet
				if (vOwner.GetInteger() == iEmpireKey) {
					
					Variant vMaxPop;
					iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					// Deposit population if the maxpop of the planet is greater than the current pop
					if (vMaxPop.GetInteger() > vPop.GetInteger() && 
						!(gcConfig.iShipBehavior & COLONY_DISABLE_SETTLES)) {

						sprintf (pszOrder, "Settle %s (%i,%i)", vPlanetName.GetCharPtr(), iLocationX, iLocationY);

						(*ppiOrderKey)[*piNumOrders] = DEPOSIT_POP;
						(*ppstrOrderText)[*piNumOrders] = pszOrder;
						(*piNumOrders) ++;

						sprintf (pszOrder, "Settle %s (%i,%i) and dismantle", vPlanetName.GetCharPtr(), iLocationX, iLocationY);

						(*ppiOrderKey)[*piNumOrders] = DEPOSIT_POP_AND_DISMANTLE;
						(*ppstrOrderText)[*piNumOrders] = pszOrder;
						(*piNumOrders) ++;
					}
				}
			}
		}
		
		break;

	case STARGATE:
		
		// Check next BR
		if (fNextBR >= gcConfig.fStargateGateCost) {

			Variant vNewName, vTemp;

			unsigned int iNumPlanets, * piKey, iKey;
			int iSrcX = 0, iSrcY = 0, iDestX, iDestY;

			if (gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE) {

				// Get src planet coords
				iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vTemp);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				GetCoordinates (vTemp.GetCharPtr(), &iSrcX, &iSrcY);
			}

			// Get all owned planets
			iErrCode = m_pGameData->GetEqualKeys (
				strGameMap, 
				GameMap::Owner, 
				iEmpireKey, 
				false, 
				&piKey, 
				&iNumPlanets
				);

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			for (i = 0; i < iNumPlanets; i ++) {

				iKey = piKey[i];

				if (iKey != (unsigned int) iPlanetKey) {
					
					// Get coordinates
					iErrCode = m_pGameData->ReadData (strGameMap, iKey, GameMap::Coordinates, &vTemp);
					if (iErrCode != OK) {
						Assert (false);
						m_pGameData->FreeKeys (piKey);
						goto Cleanup;
					}

					GetCoordinates (vTemp.GetCharPtr(), &iDestX, &iDestY);

					if (
						
						!(gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE) ||

						fNextBR >= GetGateBRForRange (gcConfig.fStargateRangeFactor, iSrcX, iSrcY, iDestX, iDestY)
						
						) {

						// Get name
						iErrCode = m_pGameData->ReadData (strGameMap, iKey, GameMap::Name, &vNewName);
						if (iErrCode != OK) {
							Assert (false);
							m_pGameData->FreeKeys (piKey);
							goto Cleanup;
						}
						
						// Add send to Order
						sprintf (pszOrder, "Stargate ships to %s (%i,%i)", vNewName.GetCharPtr(), iDestX, iDestY);
						
						(*ppiOrderKey)[*piNumOrders] = iKey;
						(*ppstrOrderText)[*piNumOrders] = pszOrder;
						(*piNumOrders) ++;
					}
				}
			}

			m_pGameData->FreeKeys (piKey);
		}

		break;

	case CLOAKER:
		
		{
			if (vState.GetInteger() & CLOAKED) {
				
				// Uncloak
				(*ppiOrderKey)[*piNumOrders] = UNCLOAK;
				(*ppstrOrderText)[*piNumOrders] = "Uncloak";
				(*piNumOrders) ++;

			} else {
				
				// Cloak
				(*ppiOrderKey)[*piNumOrders] = CLOAK;
				(*ppstrOrderText)[*piNumOrders] = "Cloak";
				(*piNumOrders) ++;
			}
		}

		break;

	case TERRAFORMER:
		
		{
			if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {

				Variant vOwner;

				iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vOwner.GetInteger() != iEmpireKey) {
					break;
				}
			}

			// All planets can be terraformed, if their statistics allow it 
			// (even enemy-owned or uncolonized ones)

			Variant vAg, vMin, vFuel;
			
			iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Ag, &vAg);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			if (vFuel.GetInteger() > vAg.GetInteger() || vMin.GetInteger() > vAg.GetInteger()) {

				if (!(gcConfig.iShipBehavior & TERRAFORMER_DISABLE_SURVIVAL)) {
					
					sprintf (pszOrder, "Terraform %s (%i,%i)", vPlanetName.GetCharPtr(), iLocationX, iLocationY);
					
					(*ppiOrderKey)[*piNumOrders] = TERRAFORM;
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;
				}

				sprintf (pszOrder, "Terraform %s (%i,%i) and dismantle", vPlanetName.GetCharPtr(), iLocationX, iLocationY);

				(*ppiOrderKey)[*piNumOrders] = TERRAFORM_AND_DISMANTLE;
				(*ppstrOrderText)[*piNumOrders] = pszOrder;
				(*piNumOrders) ++;
			}
		}

		break;

	case TROOPSHIP:

		{
			// Only enemy planets can be invaded
			Variant vOwner, vDipStatus;
			unsigned int iKey;

			iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			if (vOwner.GetInteger() != iEmpireKey && vOwner.GetInteger() != SYSTEM) {

				if (vOwner.GetInteger() == INDEPENDENT) {
					vDipStatus = WAR;
				} else {
					
					iErrCode = m_pGameData->GetFirstKey (strEmpireDip, GameEmpireDiplomacy::EmpireKey, vOwner, false, &iKey);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					iErrCode = m_pGameData->ReadData (strEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
				}

				if (vDipStatus.GetInteger() == WAR) {
					
					// Invade
					if (!(gcConfig.iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL)) {

						sprintf (pszOrder, "Invade %s (%i,%i)", vPlanetName.GetCharPtr(), iLocationX, iLocationY);
						
						(*ppiOrderKey)[*piNumOrders] = INVADE;
						(*ppstrOrderText)[*piNumOrders] = pszOrder;
						(*piNumOrders) ++;
					}

					sprintf (pszOrder, "Invade %s (%i,%i) and dismantle", vPlanetName.GetCharPtr(), iLocationX, iLocationY);

					(*ppiOrderKey)[*piNumOrders] = INVADE_AND_DISMANTLE;
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;
				}
			}
		}

		break;

	case DOOMSDAY:
	
		{
			// Get owner
			Variant vOwner, vHW;

			iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			if (vOwner.GetInteger() == iEmpireKey) {
				
				if (vGameClassOptions.GetInteger() & DISABLE_SUICIDAL_DOOMSDAYS) {
					break;
				}
				
				iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::HomeWorld, &vHW);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (vHW.GetInteger() == HOMEWORLD) {
					break;
				}

			} else { 
				
				if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {

					// Get dip status with owner
					unsigned int iKey;
					Variant vDipStatus;
					
					iErrCode = m_pGameData->GetFirstKey (strEmpireDip, GameEmpireDiplomacy::EmpireKey, vOwner, false, &iKey);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					iErrCode = m_pGameData->ReadData (strEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					if (vDipStatus.GetInteger() != WAR) {

						if (!(vGameClassOptions.GetInteger() & USE_UNFRIENDLY_DOOMSDAYS)) {
							break;
						}

						iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::HomeWorld, &vHW);
						if (iErrCode != OK) {
							Assert (false);
							goto Cleanup;
						}
						
						if (vHW.GetInteger() == HOMEWORLD) {
							break;
						}
					}
				}
			}
			
			sprintf (pszOrder, "Annihilate %s (%i,%i)", vPlanetName.GetCharPtr(), iLocationX, iLocationY);
			
			(*ppiOrderKey)[*piNumOrders] = ANNIHILATE;
			(*ppstrOrderText)[*piNumOrders] = pszOrder;
			(*piNumOrders) ++;
		}

		break;

	case MINEFIELD:

		if (!(gcConfig.iShipBehavior & MINEFIELD_DISABLE_DETONATE)) {

			(*ppiOrderKey)[*piNumOrders] = DETONATE;
			(*ppstrOrderText)[*piNumOrders] = "Detonate ";
			(*ppstrOrderText)[*piNumOrders] += SHIP_TYPE_STRING_LOWERCASE [MINEFIELD];
			(*piNumOrders) ++;
		}
		break;

	case ENGINEER:
		
		// Check next BR
		if (fNextBR >= gcConfig.fEngineerLinkCost) {

			Variant vNewName, vKey, vExplored;

			iErrCode = m_pGameData->ReadData (
				strEmpireMap, 
				iProxyPlanetKey, 
				GameEmpireMap::Explored, 
				&vExplored
				);

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			ENUMERATE_CARDINAL_POINTS(i) {
				
				AdvanceCoordinates (iLocationX, iLocationY, &iNewX, &iNewY, i);

				// If there's a link, then offer to open it, else offer to close it
				if (pbLink[i]) {
					
					(*ppiOrderKey)[*piNumOrders] = CLOSE_LINK[i];
					
					if (vExplored.GetInteger() & EXPLORED_X[i]) {
						
						iErrCode = m_pGameData->ReadData (
							strGameMap, 
							iPlanetKey, 
							GameMap::NorthPlanetKey + i, 
							&vKey
							);
						if (iErrCode != OK) {
							Assert (false);
							goto Cleanup;
						}
						
						if (vKey.GetInteger() != NO_KEY) {
							
							iErrCode = m_pGameData->ReadData (strGameMap, vKey, GameMap::Name, &vNewName);
							if (iErrCode != OK) {
								Assert (false);
								goto Cleanup;
							}

							sprintf (
								pszOrder, 
								"Close %s link to %s (%i,%i)", 
								CARDINAL_STRING[i],
								vNewName.GetCharPtr(), 
								iNewX, 
								iNewY
								);
							
							(*ppstrOrderText)[*piNumOrders] = pszOrder;
						}

					} else {
						
						sprintf (
							pszOrder, 
							"Close %s link to Unknown (%i,%i)", 
							CARDINAL_STRING[i],
							iNewX, 
							iNewY
							);

						(*ppstrOrderText)[*piNumOrders] = pszOrder;
					}
					
					(*piNumOrders) ++;
					
				} else {
					
					iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::NorthPlanetKey + i, &vKey);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					if (vKey.GetInteger() != NO_KEY) {
						
						(*ppiOrderKey)[*piNumOrders] = OPEN_LINK[i];
						
						if (vExplored.GetInteger() & EXPLORED_X[i]) {
							
							iErrCode = m_pGameData->ReadData (strGameMap, vKey, GameMap::Name, &vNewName);
							if (iErrCode != OK) {
								Assert (false);
								goto Cleanup;
							}

							sprintf (
								pszOrder, 
								"Open %s link to %s (%i,%i)", 
								CARDINAL_STRING[i],
								vNewName.GetCharPtr(), 
								iNewX, 
								iNewY
								);
							
							(*ppstrOrderText)[*piNumOrders] = pszOrder;

						} else {

							sprintf (
								pszOrder, 
								"Open %s link to Unknown (%i,%i)", 
								CARDINAL_STRING[i],
								iNewX, 
								iNewY
								);

							(*ppstrOrderText)[*piNumOrders] = pszOrder;
						}

						(*piNumOrders) ++;
					}
				}
			}	// End enumeration
		}

		break;

	case BUILDER:

		// Check next BR
		if (fNextBR >= gcConfig.fBuilderMinBR) {

			Variant vKey;
			
			ENUMERATE_CARDINAL_POINTS(i) {
				
				iErrCode = m_pGameData->ReadData (
					strGameMap, 
					iPlanetKey, 
					GameMap::NorthPlanetKey + i, 
					&vKey
					);
				
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (vKey.GetInteger() == NO_KEY) {
					
					AdvanceCoordinates (iLocationX, iLocationY, &iNewX, &iNewY, i);
					
					sprintf (
						pszOrder, 
						"Create new planet to the %s (%i,%i)", 
						CARDINAL_STRING[i],
						iNewX,
						iNewY
						);
					
					(*ppiOrderKey)[*piNumOrders] = CREATE_PLANET_LINK[i];
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;
				}
			}
		}
		break;

	case JUMPGATE:

		// Check next BR
		if (fNextBR >= gcConfig.fJumpgateGateCost) {

			Variant vNewName, vTemp, * pvPlanetKey;

			unsigned int iNumPlanets, iKey;
			int iSrcX = 0, iSrcY = 0, iDestX, iDestY;

			if (gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE) {

				// Get src planet coords
				iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vTemp);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				GetCoordinates (vTemp.GetCharPtr(), &iSrcX, &iSrcY);
			}

			// Get all visible planets
			GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

			iErrCode = m_pGameData->ReadColumn (
				strGameEmpireMap, 
				GameEmpireMap::PlanetKey, 
				&pvPlanetKey, 
				&iNumPlanets
				);

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			for (i = 0; i < iNumPlanets; i ++) {

				iKey = pvPlanetKey[i].GetInteger();

				if (iKey != (unsigned int) iPlanetKey) {
					
					// Check range
					iErrCode = m_pGameData->ReadData (strGameMap, iKey, GameMap::Coordinates, &vTemp);
					if (iErrCode != OK) {
						Assert (false);
						m_pGameData->FreeData (pvPlanetKey);
						goto Cleanup;
					}

					GetCoordinates (vTemp.GetCharPtr(), &iDestX, &iDestY);

					if (
						
						!(gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE) ||

						fNextBR >= GetGateBRForRange (gcConfig.fJumpgateRangeFactor, iSrcX, iSrcY, iDestX, iDestY)
						
						) {
						
						// Check annihilated status
						iErrCode = m_pGameData->ReadData (strGameMap, iKey, GameMap::Annihilated, &vTemp);
						if (iErrCode != OK) {
							Assert (false);
							m_pGameData->FreeData (pvPlanetKey);
							goto Cleanup;
						}

						if (vTemp.GetInteger() == NOT_ANNIHILATED) {

							// Get name
							iErrCode = m_pGameData->ReadData (strGameMap, iKey, GameMap::Name, &vNewName);
							if (iErrCode != OK) {
								Assert (false);
								m_pGameData->FreeData (pvPlanetKey);
								goto Cleanup;
							}
							
							// Add send to Order
							sprintf (pszOrder, "Jumpgate ships to %s (%i,%i)", vNewName.GetCharPtr(), iDestX, iDestY);
							
							(*ppiOrderKey)[*piNumOrders] = iKey;
							(*ppstrOrderText)[*piNumOrders] = pszOrder;
							(*piNumOrders) ++;
						}
					}
				}
			}

			m_pGameData->FreeData (pvPlanetKey);
		}

		break;

	default:
		
		// Nothing to add
		break;
	}

	/////////////////////////
	// Add Morpher options //
	/////////////////////////

	if (vState.GetInteger() & MORPH_ENABLED && 
		!(vState.GetInteger() & CLOAKED) &&		// Cloaked morphers can't morph
		fNextBR >= gcConfig.fMorpherCost) {

		Variant vTechs;

		GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
		
		// Get developed techs
		iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::TechDevs, &vTechs);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		ENUMERATE_SHIP_TYPES (i) {

			if ((int) i != iShipType && vTechs.GetInteger() & TECH_BITS[i]) {

				sprintf (
						pszOrder, 
						"Morph into %s",
						SHIP_TYPE_STRING[i]
						);
				
				(*ppiOrderKey)[*piNumOrders] = MORPH_ORDER[i];
				(*ppstrOrderText)[*piNumOrders] = pszOrder;
				(*piNumOrders) ++;
			}
		}		
	}
	
	/////////////////////
	// Add Fleet Joins //
	/////////////////////
	
	if (IsMobileShip (iShipType)) {
		
		// Get all fleets located at planet
		unsigned int* piFleetKey, iNumFleets;
		iErrCode = m_pGameData->GetEqualKeys (
			strEmpireFleets, 
			GameEmpireFleets::CurrentPlanet, 
			iPlanetKey, 
			false, 
			&piFleetKey, 
			&iNumFleets
			);
		
		if (iErrCode != OK) {
			
			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				iErrCode = OK;
			} else {

				Assert (false);
				goto Cleanup;
			}
		}
		
		if (iNumFleets > 0) {
			
			Variant vFleetKey, vFleetName;
			iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vFleetKey);
			if (iErrCode != OK) {
				Assert (false);
				m_pGameData->FreeKeys (piFleetKey);
				goto Cleanup;
			}
			
			for (i = 0; i < iNumFleets; i ++) {
				
				if (vFleetKey.GetInteger() == (int) piFleetKey[i]) {
					
					// Remain in own fleet
					iErrCode = m_pGameData->ReadData (
						strEmpireFleets, 
						vFleetKey.GetInteger(), 
						GameEmpireFleets::Name, 
						&vFleetName
						);

					if (iErrCode != OK) {
						Assert (false);
						m_pGameData->FreeKeys (piFleetKey);
						goto Cleanup;
					}
					
					sprintf (pszOrder, "Remain in fleet %s", vFleetName.GetCharPtr());

					(*ppiOrderKey)[*piNumOrders] = FLEET;
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
					(*piNumOrders) ++;

					// Leave own fleet
					sprintf (pszOrder, "Leave fleet %s", vFleetName.GetCharPtr());

					(*ppiOrderKey)[*piNumOrders] = LEAVE_FLEET;
					(*ppstrOrderText)[*piNumOrders] = pszOrder;

				} else {

					// Join another fleet
					iErrCode = m_pGameData->ReadData (
						strEmpireFleets, 
						piFleetKey[i], 
						GameEmpireFleets::Name, 
						&vFleetName
						);

					if (iErrCode != OK) {
						Assert (false);
						m_pGameData->FreeKeys (piFleetKey);
						goto Cleanup;
					}
					
					sprintf (pszOrder, "Join fleet %s", vFleetName.GetCharPtr());

					(*ppiOrderKey)[*piNumOrders] = piFleetKey[i];
					(*ppstrOrderText)[*piNumOrders] = pszOrder;
				}

				(*piNumOrders) ++;
			}
			
			m_pGameData->FreeKeys (piFleetKey);
		}
	}

	///////////////////
	// Add dismantle //
	///////////////////

	(*ppiOrderKey)[*piNumOrders] = DISMANTLE;
	(*ppstrOrderText)[*piNumOrders] = "Dismantle";
	(*piNumOrders) ++;

Cleanup:

	// Allocation check
	if (iErrCode == OK) {

		for (i = 0; i < (unsigned int) *piNumOrders; i ++) {

			if ((*ppstrOrderText)[i].GetCharPtr() == NULL) {
				iErrCode = ERROR_OUT_OF_MEMORY;
				break;
			}
		}
	}

	if (iErrCode != OK) {
		
		if (*ppiOrderKey != NULL) {
			delete [] *ppiOrderKey;
			*ppiOrderKey = NULL;
		}

		if (*ppstrOrderText != NULL) {
			delete [] *ppstrOrderText;
			*ppstrOrderText = NULL;
		}

		*piNumOrders = 0;
	} 

	return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iShipKey-> Integer key of ship
// pszNewName -> New name for ship
//
// Updates the name of a ship

int GameEngine::UpdateShipName (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, 
								const char* pszNewName) {

	GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);

	// Check ship key
	bool bShipExists;
	if (m_pGameData->DoesRowExist (strEmpireShips, iShipKey, &bShipExists) != OK || !bShipExists) {
		return ERROR_SHIP_DOES_NOT_EXIST;
	}

	// Write new name
	return m_pGameData->WriteData (strEmpireShips, iShipKey, GameEmpireShips::Name, pszNewName);
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iShipKey -> Integer key of ship
// iNewShipOrder -> Integer key of ship action
//
// returns:
//	ERROR_SHIP_DOES_NOT_EXIST -> Ship doesn't exist
//	ERROR_FLEET_DOES_NOT_EXIST -> Fleet doesn't exist
//	ERROR_WRONG_PLANET -> Ship and fleet are on different planets
//	ERROR_CANNOT_MOVE -> Ship cannot move in the requested direction
//	ERROR_CANNOT_EXPLORE -> Ship cannot explore in the requested direction
//	ERROR_CANNOT_COLONIZE -> Ship cannot colonize the requested planet
//	ERROR_CANNOT_SETTLE -> Ship cannot deposit pop on the requested planet
//	ERROR_WRONG_SHIP_TYPE -> Ship cannot perform special action
//	ERROR_CANNOT_CLOAK -> Ship is already cloaked
//	ERROR_CANNOT_UNCLOAK -> Ship is already uncloaked
//	ERROR_CANNOT_TERRAFORM -> Ship cannot terraform
//	ERROR_CANNOT_NUKE -> Ship cannot nuke
//	ERROR_CANNOT_ANNIHILATE -> Ship cannot annihilate
//	ERROR_CANNOT_OPEN_LINK -> Ship cannot open link
//	ERROR_CANNOT_CLOSE_LINK -> Ship cannot close link
//	ERROR_UNKNOWN_ORDER -> Unknown order
//	ERROR_CANNOT_GATE -> Ship cannot gate to planet
//	ERROR_CANNOT_INVADE -> Ship cannot invade
//	ERROR_PLANET_EXISTS -> Ship cannot create planet because it already exists
//	ERROR_CANNOT_MORPH -> Ship cannot morph because it was cloaked
//
// Updates the a given ship's current orders

// TODO: transaction

int GameEngine::UpdateShipOrders (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, 
								  int iNewShipOrder) {

	GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
	GAME_MAP (strGameMap, iGameClass, iGameNumber);
	
	Variant vTemp, vOldShipOrder, vOldFleetKey, vShipPlanet, vShipType, vGameClassOptions;

	// Check ship key
	bool bShipExists, bOldDismantle, bDismantle = false;
	int i, iShipBehavior, iErrCode, iShipState, iOldOrder;

	NamedMutex nmShipMutex;
	LockEmpireShips (iGameClass, iGameNumber, iEmpireKey, &nmShipMutex);
	
	iErrCode = m_pGameData->DoesRowExist (strEmpireShips, iShipKey, &bShipExists);
	if (iErrCode != OK || !bShipExists) {
		iErrCode = ERROR_SHIP_DOES_NOT_EXIST;
		goto Cleanup;
	}

	// Check for same order
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::Action, &vOldShipOrder);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (vOldShipOrder.GetInteger() == iNewShipOrder) {
		iErrCode = ERROR_SAME_SHIP_ORDER;
		goto Cleanup;
	}

	// Get ship behavior
	iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ShipBehavior, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iShipBehavior = vTemp.GetInteger();

	// Get gameclass options
	iErrCode = m_pGameData->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		&vGameClassOptions
		);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Get ship state
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::State, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iShipState = vTemp.GetInteger();
	
	GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
	GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

	// Get ship's fleet and location
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vOldFleetKey);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentPlanet, &vShipPlanet);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	Assert (vShipPlanet.GetInteger() != NO_KEY);
	
	// Check for a fleet change for ships built this update
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vTemp);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (vTemp.GetInteger() != 0) {
		
		// Handle a cancel build request
		if (iNewShipOrder == CANCEL_BUILD) {
			
			// Just delete the ship
			iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, iShipKey, true);
			goto Cleanup;

		} else {
			
			Variant vCurrentBR;
			iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vCurrentBR);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			float fMil = vCurrentBR.GetFloat() * vCurrentBR.GetFloat();
			
			if (iNewShipOrder == BUILD_AT) {
				
				// Set action to build at planet
				iErrCode = m_pGameData->WriteData (strEmpireShips, iShipKey, GameEmpireShips::Action, BUILD_AT);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (vOldFleetKey != NO_KEY) {
					
					// Reduce fleet's strength
					iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::CurrentStrength, -fMil);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::MaxStrength, -fMil);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					// Decrement number of "BuildShips"
					iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::BuildShips, -1);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					// Set no fleet key
					iErrCode = m_pGameData->WriteData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, NO_KEY);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
				}

				goto Cleanup;
				
			} else {
				
				// We must be building into a fleet, so check that the key is right
				iErrCode = m_pGameData->DoesRowExist (strEmpireFleets, iNewShipOrder, &bShipExists);
				if (iErrCode != OK || !bShipExists) {
					iErrCode = ERROR_FLEET_DOES_NOT_EXIST;
					goto Cleanup;
				}
				
				// Make sure that the fleet is on the same planet as the ship
				Variant vFleetPlanet;
				iErrCode = m_pGameData->ReadData (
					strEmpireFleets, 
					iNewShipOrder, 
					GameEmpireFleets::CurrentPlanet, 
					&vFleetPlanet
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vShipPlanet.GetInteger() != vFleetPlanet.GetInteger()) {
					iErrCode = ERROR_WRONG_PLANET;
					goto Cleanup;
				}
				
				if (vOldFleetKey.GetInteger() != NO_KEY) {

					// Reduce fleet's strength
					iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::CurrentStrength, -fMil);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}

					iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::MaxStrength, -fMil);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}

					// Decrement number of "BuildShips"
					iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::BuildShips, -1);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
				}
				
				// Increase power of new fleet
				iErrCode = m_pGameData->Increment (strEmpireFleets, iNewShipOrder, GameEmpireFleets::CurrentStrength, fMil);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				iErrCode = m_pGameData->Increment (strEmpireFleets, iNewShipOrder, GameEmpireFleets::MaxStrength, fMil);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				// Increment number of "buildships"
				iErrCode = m_pGameData->Increment (strEmpireFleets, iNewShipOrder, GameEmpireFleets::BuildShips, 1);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				// Set action to be "build in fleet"
				iErrCode = m_pGameData->WriteData (
					strEmpireShips, 
					iShipKey, 
					GameEmpireShips::Action, 
					iNewShipOrder
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				// Set ship fleet
				iErrCode = m_pGameData->WriteData (
					strEmpireShips, 
					iShipKey, 
					GameEmpireShips::FleetKey, 
					iNewShipOrder
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				// Set the fleet to stand by
				iErrCode = m_pGameData->WriteData (
					strEmpireFleets, 
					iNewShipOrder, 
					GameEmpireFleets::Action, 
					STAND_BY
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				goto Cleanup;
			}
			
		}
	}
	
	/////////////////////////////////////////////////////////////////////////////////////
	// The ship is not being built, so we need to verify the legitimacy of the request //
	/////////////////////////////////////////////////////////////////////////////////////

	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::Type, &vShipType);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Was a join fleet requested?
	if (IsMobileShip (vShipType.GetInteger()) && iNewShipOrder >= 0) {

		// Does fleet exist?
		iErrCode = m_pGameData->DoesRowExist (strEmpireFleets, iNewShipOrder, &bShipExists);
		if (!bShipExists) {
			iErrCode = ERROR_FLEET_DOES_NOT_EXIST;
			goto Cleanup;
		}

		// Are fleet and ship on the same planet?
		Variant vFleetPlanet;
		iErrCode = m_pGameData->ReadData (
			strEmpireFleets,
			iNewShipOrder,
			GameEmpireFleets::CurrentPlanet,
			&vFleetPlanet
			);

		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		if (vShipPlanet.GetInteger() != vFleetPlanet.GetInteger()) {
			iErrCode = ERROR_WRONG_PLANET;
			goto Cleanup;
		}
		
		// Set fleet key to new fleet
		iErrCode = m_pGameData->WriteData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, iNewShipOrder);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		// Get ship's BR
		Variant vCurrentBR, vMaxBR;
		iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vCurrentBR);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		// Add ship's strength to new fleet's strength
		float fCurrentMil = vCurrentBR.GetFloat() * vCurrentBR.GetFloat();
		iErrCode = m_pGameData->Increment (strEmpireFleets, iNewShipOrder, GameEmpireFleets::CurrentStrength, fCurrentMil);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		float fMaxMil = vMaxBR.GetFloat() * vMaxBR.GetFloat();
		iErrCode = m_pGameData->Increment (strEmpireFleets, iNewShipOrder, GameEmpireFleets::MaxStrength, fMaxMil);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		// Remove ship from old fleet
		if (vOldFleetKey != NO_KEY) {
			
			// Update old fleet's strength
			iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::CurrentStrength, -fCurrentMil);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::MaxStrength, -fMaxMil);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}

		// Set orders to follow fleet
		iNewShipOrder = FLEET;
		
	} else {
		
		///////////////////////////////////////////////////////////////////////////////////////////
		// The order wasn't a join fleet,														 //
		// so let's check for leave, stand by, dismantle, nuke, remain, move or explore requests //
		///////////////////////////////////////////////////////////////////////////////////////////

		switch (iNewShipOrder) {

		case STAND_BY:

			break;

		case DISMANTLE:

			bDismantle = true;
			break;

		// Leave fleet
		case LEAVE_FLEET:

			{
				if (vOldFleetKey.GetInteger() == NO_KEY) {
					iErrCode = ERROR_SHIP_IS_NOT_IN_FLEET;
					goto Cleanup;
				}

				// Get ship's BR
				Variant vCurrentBR, vMaxBR;
				
				iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vCurrentBR);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				// Subtract ship's strength from fleet's strength
				float fCurrentMil = vCurrentBR.GetFloat() * vCurrentBR.GetFloat();
				
				iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::CurrentStrength, -fCurrentMil);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				// Subtract ship's max strength from fleet's max strength
				fCurrentMil = vMaxBR.GetFloat() * vMaxBR.GetFloat();
				
				iErrCode = m_pGameData->Increment (strEmpireFleets, vOldFleetKey, GameEmpireFleets::MaxStrength, -fCurrentMil);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				// Set ship to no fleet
				iErrCode = m_pGameData->WriteData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, NO_KEY);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				// Stand by
				iNewShipOrder = STAND_BY;
			}

			break;

		case FLEET:

			if (vOldFleetKey.GetInteger() == NO_KEY) {
				iErrCode = ERROR_SHIP_IS_NOT_IN_FLEET;
				goto Cleanup;
			}

#ifdef _DEBUG

			{

			Variant vFleetPlanet;

			iErrCode = m_pGameData->ReadData (
				strEmpireFleets, 
				vOldFleetKey.GetInteger(), 
				GameEmpireFleets::CurrentPlanet,
				&vFleetPlanet
				);

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			Assert (vFleetPlanet.GetInteger() == vShipPlanet.GetInteger());
			
			}
#endif

			break;

		case MOVE_NORTH:
		case MOVE_EAST:
		case MOVE_SOUTH:
		case MOVE_WEST:

			{
				// Make sure the ship is mobile
				if (!IsMobileShip (vShipType.GetInteger())) {
					iErrCode = ERROR_CANNOT_MOVE;
					goto Cleanup;
				}
				
				// Get proxy key for empiremap
				unsigned int iPlanetProxyKey;
				iErrCode = m_pGameData->GetFirstKey (
					strEmpireMap, 
					GameEmpireMap::PlanetKey, 
					vShipPlanet, 
					false, 
					&iPlanetProxyKey
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				Variant vLink;
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Link, &vLink);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				i = MOVE_NORTH - iNewShipOrder;

				// Make sure there's a link						
				if (!(vLink.GetInteger() & LINK_X[i])) {
					iErrCode = ERROR_CANNOT_MOVE;
					goto Cleanup;
				}

				// Make sure we've explored the planet
				iErrCode = m_pGameData->ReadData (
					strEmpireMap, 
					iPlanetProxyKey, 
					GameEmpireMap::Explored, 
					&vLink
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
						
				if (!(vLink.GetInteger() & EXPLORED_X[i])) {
					iErrCode = ERROR_CANNOT_MOVE;
					goto Cleanup;
				}
#ifdef _DEBUG
				// Make sure there's a planet
				Variant vNewPlanetKey;
				iErrCode = m_pGameData->ReadData (
					strGameMap, 
					vShipPlanet, 
					GameMap::NorthPlanetKey + i, 
					&vNewPlanetKey
					);

				Assert (iErrCode == OK && vNewPlanetKey != NO_KEY);
#endif
			}	// End indent

			break;

		case EXPLORE_NORTH:
		case EXPLORE_EAST:
		case EXPLORE_SOUTH:
		case EXPLORE_WEST:

			{			
				// Make sure the ship is a sci
				if (vShipType.GetInteger() != SCIENCE) {
					iErrCode = ERROR_CANNOT_EXPLORE;
					goto Cleanup;
				}
				
				// Get proxy key for empiremap
				unsigned int iPlanetProxyKey;
				iErrCode = m_pGameData->GetFirstKey (
					strEmpireMap, 
					GameEmpireMap::PlanetKey, 
					vShipPlanet, 
					false, 
					&iPlanetProxyKey
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				Variant vLink;

				i = EXPLORE_NORTH - iNewShipOrder;
						
				// Check for link
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Link, &vLink);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (!(vLink.GetInteger() & LINK_X[i])) {
					iErrCode = ERROR_CANNOT_EXPLORE;
					goto Cleanup;
				}
				
				// Check for explored
				iErrCode = m_pGameData->ReadData (
					strEmpireMap, 
					iPlanetProxyKey, 
					GameEmpireMap::Explored, 
					&vLink
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (vLink.GetInteger() & EXPLORED_X[i]) {
					iErrCode = ERROR_CANNOT_EXPLORE;
					goto Cleanup;
				}
				
#ifdef _DEBUG
				// Make sure there's a planet
				Variant vNewPlanetKey;
				iErrCode = m_pGameData->ReadData (
					strGameMap, 
					vShipPlanet, 
					GameMap::NorthPlanetKey + i, 
					&vNewPlanetKey
					);
				Assert (iErrCode == OK && vNewPlanetKey != NO_KEY);
#endif
			}	// End indent

			break;

		case NUKE:

			{
				// Make sure ship is mobile
				if (!IsMobileShip (vShipType.GetInteger())) {
					iErrCode = ERROR_CANNOT_NUKE;
					goto Cleanup;
				}
				
				// Make sure ship is not cloaked				
				if (iShipState & CLOAKED) {
					iErrCode = ERROR_CANNOT_NUKE;
					goto Cleanup;
				}
				
				// Make sure not nuking own planet or system planet			
				Variant vOwner;
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Owner, &vOwner);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (vOwner.GetInteger() == SYSTEM || vOwner.GetInteger() == iEmpireKey) {
					iErrCode = ERROR_CANNOT_NUKE;
					goto Cleanup;
				}
				
				// Check diplomatic status with owner
				if (vOwner.GetInteger() != INDEPENDENT) {
					
					GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
					
					// Make sure the diplomatic status is war
					Variant vDipStatus;
					unsigned int iKey;
					
					iErrCode = m_pGameData->GetFirstKey (
						strDiplomacy, 
						GameEmpireDiplomacy::EmpireKey, 
						vOwner, 
						false, 
						&iKey
						);

					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					iErrCode = m_pGameData->ReadData (
						strDiplomacy, 
						iKey, 
						GameEmpireDiplomacy::CurrentStatus, 
						&vDipStatus
						);

					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					if (vDipStatus.GetInteger() != WAR) {
						iErrCode = ERROR_CANNOT_INVADE;
						goto Cleanup;
					}
				}
			}

			break;
			
		
		case COLONIZE_OR_DEPOSIT_POP_AND_DISMANTLE:

			if (iShipBehavior & COLONY_DISABLE_SETTLES) {
				iErrCode = ERROR_CANNOT_SETTLE;
				goto Cleanup;
			}
			// No break

		case COLONIZE_AND_DISMANTLE:

			bDismantle = true;
			// No break
		
		case COLONIZE_OR_DEPOSIT_POP:

			if (iNewShipOrder == COLONIZE_OR_DEPOSIT_POP) {
				
				if (iShipBehavior & COLONY_DISABLE_SETTLES) {
					iErrCode = ERROR_CANNOT_SETTLE;
					goto Cleanup;
				}
			}
			// No break

		case COLONIZE:

			if (!bDismantle && (iShipBehavior & COLONY_DISABLE_SURVIVAL)) {
				iErrCode = ERROR_CANNOT_COLONIZE;
				goto Cleanup;
			}

			{				
				// Make sure the ship is a colony
				if (vShipType.GetInteger() != COLONY) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}

				// Make sure the planet has 0 pop and wasn't annihilated and doesn't belong to us
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet.GetInteger(), GameMap::Pop, &vTemp);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vTemp.GetInteger() != 0) {
					iErrCode = ERROR_CANNOT_COLONIZE;
					goto Cleanup;
				}

				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Owner, &vTemp);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vTemp.GetInteger() == iEmpireKey) {
					iErrCode = ERROR_CANNOT_COLONIZE;
					goto Cleanup;
				}
				
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Annihilated, &vTemp);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vTemp.GetInteger() != NOT_ANNIHILATED) {
					iErrCode = ERROR_CANNOT_COLONIZE;
					goto Cleanup;
				}
			}
			
			break;

		case DEPOSIT_POP_AND_DISMANTLE:

			bDismantle = true;
			// No break
		
		case DEPOSIT_POP:

			if (iShipBehavior & COLONY_DISABLE_SETTLES) {
				iErrCode = ERROR_CANNOT_SETTLE;
				goto Cleanup;
			}
	
			{
				Variant vPop, vMaxPop;
				
				// Make sure the ship is a colony
				if (vShipType.GetInteger() != COLONY) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}
				
				// Make sure the planet has less pop than maxpop
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Pop, &vPop);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::MaxPop, &vMaxPop);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				// Make sure it's our planet
				Variant vOwner;
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Owner, &vOwner);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vOwner.GetInteger() != iEmpireKey) {
					iErrCode = ERROR_CANNOT_SETTLE;
					goto Cleanup;
				}

				if (vPop.GetInteger() >= vMaxPop.GetInteger()) {
					iErrCode = ERROR_CANNOT_SETTLE;
					goto Cleanup;
				}
			}
			
			break;
			
		case CLOAK:
			{
				// Make sure the ship is a cloaker
				if (vShipType.GetInteger() != CLOAKER) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}

				if (iShipState & CLOAKED) {
					iErrCode = ERROR_CANNOT_CLOAK;
					goto Cleanup;
				}
			}
			
			break;
			
		case UNCLOAK:
			{
				// Make sure the ship is a cloaker
				if (vShipType.GetInteger() != CLOAKER) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}

				if (!(iShipState & CLOAKED)) {
					iErrCode = ERROR_CANNOT_UNCLOAK;
					goto Cleanup;
				}
			}
			
			break;

		case TERRAFORM_AND_DISMANTLE:
		
			bDismantle = true;
			// No break

		case TERRAFORM:

			if ((iShipBehavior & TERRAFORMER_DISABLE_SURVIVAL) && !bDismantle) {
			
				iErrCode = ERROR_CANNOT_TERRAFORM;
				goto Cleanup;

			} else {

				// Make sure the ship is a terraformer
				if (vShipType.GetInteger() != TERRAFORMER) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}

				if (iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {

					Variant vOwner;

					iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Owner, &vOwner);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}

					if (vOwner.GetInteger() != iEmpireKey) {
						break;
					}
				}

				Variant vAg, vMin, vFuel;

				// Make sure that there's something to terraform
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Ag, &vAg);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Fuel, &vFuel);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Minerals, &vMin);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vAg.GetInteger() >= vFuel.GetInteger() && vAg.GetInteger() >= vMin.GetInteger()) {
					iErrCode = ERROR_CANNOT_TERRAFORM;
					goto Cleanup;
				}
			}
			
			break;
			
		case INVADE_AND_DISMANTLE:
		
			bDismantle = true;
			// No break

		case INVADE:
		
			if ((iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL) && !bDismantle) {
				
				iErrCode = ERROR_CANNOT_INVADE;
				goto Cleanup;

			} else {

				// Make sure the ship is a troopship
				if (vShipType.GetInteger() != TROOPSHIP) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}
				
				// Make sure the planet belongs to someone else or is independent
				Variant vOwner;
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Owner, &vOwner);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vOwner.GetInteger() == SYSTEM || vOwner.GetInteger() == iEmpireKey) {
					iErrCode = ERROR_CANNOT_INVADE;
					goto Cleanup;
				}
				
				if (vOwner != INDEPENDENT) {
					
					GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
					
					// Make sure the diplomatic status is war
					Variant vDipStatus;
					unsigned int iKey;
					iErrCode = m_pGameData->GetFirstKey (
						strDiplomacy, 
						GameEmpireDiplomacy::EmpireKey, 
						vOwner, 
						false, 
						&iKey
						);

					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					iErrCode = m_pGameData->ReadData (strDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					if (vDipStatus.GetInteger() != WAR) {
						iErrCode = ERROR_CANNOT_INVADE;
						goto Cleanup;
					}
				}
			}
			
			break;
			
		case ANNIHILATE:
			{
				// Make sure the ship is a doomsday
				if (vShipType.GetInteger() != DOOMSDAY) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}
				
				// Make sure the planet can be annihilated
				Variant vOwner, vHW;
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Owner, &vOwner);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {
					
					if (vOwner.GetInteger() == iEmpireKey) {

						if (vGameClassOptions.GetInteger() & DISABLE_SUICIDAL_DOOMSDAYS) {
							iErrCode = ERROR_CANNOT_ANNIHILATE;
							goto Cleanup;
						}
						
						iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::HomeWorld, &vHW);
						if (iErrCode != OK) {
							Assert (false);
							goto Cleanup;
						}
						
						if (vHW.GetInteger() == HOMEWORLD) {
							iErrCode = ERROR_CANNOT_ANNIHILATE;
							goto Cleanup;
						}
						
					} else {
						
						GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
						
						// Make sure the diplomatic status is war
						Variant vDipStatus;
						unsigned int iKey;
						iErrCode = m_pGameData->GetFirstKey (
							strDiplomacy, 
							GameEmpireDiplomacy::EmpireKey, 
							vOwner, 
							false, 
							&iKey
							);

						if (iErrCode != OK) {
							Assert (false);
							goto Cleanup;
						}
						
						iErrCode = m_pGameData->ReadData (strDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
						if (iErrCode != OK) {
							Assert (false);
							goto Cleanup;
						}
						
						if (vDipStatus.GetInteger() != WAR) {

							if (!(vGameClassOptions.GetInteger() & USE_UNFRIENDLY_DOOMSDAYS)) {
								iErrCode = ERROR_CANNOT_ANNIHILATE;
								goto Cleanup;
							}

							iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::HomeWorld, &vHW);
							if (iErrCode != OK) {
								Assert (false);
								goto Cleanup;
							}
							
							if (vHW.GetInteger() == HOMEWORLD) {
								iErrCode = ERROR_CANNOT_ANNIHILATE;
								goto Cleanup;
							}
						}
					}
				}
			}
			
			break;

		case DETONATE:

			// Make sure ship is a minefield
			if (vShipType.GetInteger() != MINEFIELD) {
				iErrCode = ERROR_WRONG_SHIP_TYPE;
				goto Cleanup;
			}
			break;
			
		case OPEN_LINK_NORTH:
		case OPEN_LINK_EAST:
		case OPEN_LINK_SOUTH:
		case OPEN_LINK_WEST:
			{
				// Make sure ship is an engineer
				if (vShipType.GetInteger() != ENGINEER) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}

				int iDirection = OPEN_LINK_NORTH - iNewShipOrder;

				Assert (iDirection >= NORTH && iDirection <= WEST);
				
				// Make sure the link doesn't exist
				Variant vLink;
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet, GameMap::Link, &vLink);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vLink.GetInteger() & LINK_X[iDirection]) {
					iErrCode = ERROR_CANNOT_OPEN_LINK;
					goto Cleanup;
				}
				
				// Make sure planet exists
				Variant vNeighbourKey;
				iErrCode = m_pGameData->ReadData (
					strGameMap, 
					vShipPlanet, 
					GameMap::NorthPlanetKey + iDirection, 
					&vNeighbourKey
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (vNeighbourKey == NO_KEY) {
					iErrCode = ERROR_CANNOT_OPEN_LINK;
					goto Cleanup;
				}
			}
			
			break;
			
		case CLOSE_LINK_NORTH:
		case CLOSE_LINK_EAST:
		case CLOSE_LINK_SOUTH:
		case CLOSE_LINK_WEST:
			{
				// Make sure ship is an engineer
				if (vShipType.GetInteger() != ENGINEER) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}

				int iDirection = CLOSE_LINK_NORTH - iNewShipOrder;
				Assert (iDirection >= NORTH && iDirection <= WEST);
				
				// Make sure the link doesn't exist
				Variant vLink;
				iErrCode = m_pGameData->ReadData (strGameMap, vShipPlanet.GetInteger(), GameMap::Link, &vLink);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (!(vLink.GetInteger() & LINK_X[iDirection])) {
					iErrCode = ERROR_CANNOT_CLOSE_LINK;
					goto Cleanup;
				}
				
				// If there's a link, then the planet exists and we're fine
			}
			
			break;

		case CREATE_PLANET_NORTH:
		case CREATE_PLANET_EAST:
		case CREATE_PLANET_SOUTH:
		case CREATE_PLANET_WEST:

			{
				// Make sure ship is a builder
				if (vShipType.GetInteger() != BUILDER) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}

				int iDirection = CREATE_PLANET_NORTH - iNewShipOrder;
				Assert (iDirection >= NORTH && iDirection <= WEST);

				// Make sure planet doesn't exist
				Variant vPlanetKey;
				iErrCode = m_pGameData->ReadData (
					strGameMap, 
					vShipPlanet.GetInteger(), 
					GameMap::NorthPlanetKey + iDirection, 
					&vPlanetKey
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (vPlanetKey.GetInteger() != NO_KEY) {
					iErrCode = ERROR_PLANET_EXISTS;
					goto Cleanup;
				}
			}

			break;

		case MORPH_ATTACK:
		case MORPH_SCIENCE:
		case MORPH_COLONY:
		case MORPH_STARGATE:
		case MORPH_CLOAKER:
		case MORPH_SATELLITE:
		case MORPH_TERRAFORMER:
		case MORPH_TROOPSHIP:
		case MORPH_DOOMSDAY:
		case MORPH_MINEFIELD:
		case MORPH_MINESWEEPER:
		case MORPH_ENGINEER:
		case MORPH_CARRIER:
		case MORPH_BUILDER:
		case MORPH_MORPHER:
		case MORPH_JUMPGATE:

			{
				// Make sure ship is a morpher
				if (!(iShipState & MORPH_ENABLED)) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}

				// Make sure ship isn't cloaked
				if (iShipState & CLOAKED) {
					iErrCode = ERROR_CANNOT_MORPH;
					goto Cleanup;
				}

				// Get tech
				int iMorphTech = MORPH_ATTACK - iNewShipOrder;

				GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

				// Make sure tech has been developed
				iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::TechDevs, &vTemp);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}

				if (!(vTemp.GetInteger() & TECH_BITS[iMorphTech])) {
					iErrCode = ERROR_NO_TECHNOLOGY_AVAILABLE;
					goto Cleanup;
				}
			}
			break;

		default:

			{
				// Ship must be a stargate or a jumpgate
				if (vShipType.GetInteger() != STARGATE && vShipType.GetInteger() != JUMPGATE) {
					iErrCode = ERROR_WRONG_SHIP_TYPE;
					goto Cleanup;
				}
				
				// Make sure planet isn't the current planet the stargate is on
				if (vShipPlanet.GetInteger() == iNewShipOrder) {
					iErrCode = ERROR_CANNOT_GATE;
					goto Cleanup;
				}

				// Make sure planet exists
				bool bPlanetExists;
				iErrCode = m_pGameData->DoesRowExist (strGameMap, iNewShipOrder, &bPlanetExists);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (!bPlanetExists) {
					iErrCode = ERROR_CANNOT_GATE;
					goto Cleanup;
				}
				
				if (vShipType.GetInteger() == STARGATE) {

					// Make sure we own the planet
					Variant vOwner;
					iErrCode = m_pGameData->ReadData (strGameMap, iNewShipOrder, GameMap::Owner, &vOwner);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					if (vOwner.GetInteger() != iEmpireKey) {
						iErrCode = ERROR_CANNOT_GATE;
						goto Cleanup;
					}

				} else {

					unsigned int iPlanetProxyKey;

					// Make sure we can see the planet
					iErrCode = m_pGameData->GetFirstKey (
						strEmpireMap,
						GameEmpireMap::PlanetKey,
						iNewShipOrder, 
						false,
						&iPlanetProxyKey
						);
					
					if (iErrCode == ERROR_DATA_NOT_FOUND) {
						iErrCode = ERROR_CANNOT_GATE;
						goto Cleanup;
					}
					
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}

					Assert (iPlanetProxyKey != NO_KEY);

					// Enforce annihilation rules
					iErrCode = m_pGameData->ReadData (strGameMap, iNewShipOrder, GameMap::Annihilated, &vTemp);
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					if (vTemp.GetInteger() != NOT_ANNIHILATED) {
						iErrCode = ERROR_CANNOT_GATE;
						goto Cleanup;
					}
				}

				// We're not going to bother to enforce range rules here
				// People can feel free to submit custom data that targets
				// any possible planet.  However, range _will_ be enforced during
				// the update algorithm

				iErrCode = m_pGameData->WriteData (
					strEmpireShips, 
					iShipKey, 
					GameEmpireShips::GateDestination, 
					iNewShipOrder
					);

				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				iNewShipOrder = GATE_SHIPS;
			}
		}	// End switch
					
		// (If the ship diverges from its fleet, it'll be caught later in the update algorithm)

	}	// End if join fleet

	iOldOrder = vOldShipOrder.GetInteger();
	
	bOldDismantle = 
		iOldOrder == DISMANTLE ||
		iOldOrder == COLONIZE_AND_DISMANTLE ||
		iOldOrder == DEPOSIT_POP_AND_DISMANTLE ||
		iOldOrder == COLONIZE_OR_DEPOSIT_POP_AND_DISMANTLE ||
		iOldOrder == TERRAFORM_AND_DISMANTLE ||
		iOldOrder == INVADE_AND_DISMANTLE;

	if (bDismantle && !bOldDismantle) {
		
		// Fix up predictions if we didn't used to be dismantling and now are
		Variant vMaxBR;
		iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = m_pGameData->Increment (
			strEmpireData, 
			GameEmpireData::NextMaintenance, 
			- GetMaintenanceCost (vShipType, vMaxBR)
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = m_pGameData->Increment (
			strEmpireData, 
			GameEmpireData::NextFuelUse, 
			- GetFuelCost (vShipType, vMaxBR)
			);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}	
	}

	else if (bOldDismantle && !bDismantle) {

		// Fix up predictions if we used to be dismantling and now we aren't
		Variant vMaxBR;
		iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = m_pGameData->Increment (
			strEmpireData, 
			GameEmpireData::NextMaintenance, 
			GetMaintenanceCost (vShipType, vMaxBR)
			);

		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = m_pGameData->Increment (
			strEmpireData, 
			GameEmpireData::NextFuelUse, 
			GetFuelCost (vShipType.GetInteger(), vMaxBR.GetFloat())
			);

		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	}
	
	// If we got here, then the order given was approved
	iErrCode = m_pGameData->WriteData (strEmpireShips, iShipKey, GameEmpireShips::Action, iNewShipOrder);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

Cleanup:

	UnlockEmpireShips (nmShipMutex);
	
	return iErrCode;
}

// TODO transaction?

int GameEngine::ChangeShipTypeOrMaxBR (const char* pszShips, const char* pszEmpireData, int iEmpireKey, 
									   int iShipKey, int iOldShipType, int iNewShipType, float fBRChange) {

	int iErrCode, iOld, iNew;
	float fNewMaxBR;
	Variant vOldMaxBR;

	unsigned int iCurrentBRColumn, iMaxBRColumn, iTypeColumn;

	if (iEmpireKey == INDEPENDENT) {

		iCurrentBRColumn = GameIndependentShips::CurrentBR;
		iMaxBRColumn = GameIndependentShips::MaxBR;
		iTypeColumn = GameIndependentShips::Type;

	} else {

		iCurrentBRColumn = GameEmpireShips::CurrentBR;
		iMaxBRColumn = GameEmpireShips::MaxBR;
		iTypeColumn = GameEmpireShips::Type;
	}

	if (iOldShipType != iNewShipType) {

		// Set new type
		iErrCode = m_pGameData->WriteData (pszShips, iShipKey, iTypeColumn, iNewShipType);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
	}

	if (fBRChange != 0.0) {
	
		// Change ship stats
		iErrCode = m_pGameData->Increment (pszShips, iShipKey, iCurrentBRColumn, fBRChange);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
		
		iErrCode = m_pGameData->Increment (pszShips, iShipKey, iMaxBRColumn, fBRChange, &vOldMaxBR);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
	
	} else {

		iErrCode = m_pGameData->ReadData (pszShips, iShipKey, iMaxBRColumn, &vOldMaxBR);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
	}

	if (iEmpireKey != INDEPENDENT) {

		fNewMaxBR = vOldMaxBR.GetFloat() + fBRChange;

		// Change empire maintenance
		iOld = GetMaintenanceCost (iOldShipType, vOldMaxBR.GetFloat());
		iNew = GetMaintenanceCost (iNewShipType, fNewMaxBR);

		iErrCode = m_pGameData->Increment (pszEmpireData, GameEmpireData::TotalMaintenance, iNew - iOld);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		// Change empire fuel use
		iOld = GetFuelCost (iOldShipType, vOldMaxBR.GetFloat());
		iNew = GetFuelCost (iNewShipType, fNewMaxBR);

		iErrCode = m_pGameData->Increment (pszEmpireData, GameEmpireData::TotalFuelUse, iNew - iOld);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

#ifdef _DEBUG
		Variant vTotalFuelUse;
		iErrCode = m_pGameData->ReadData (pszEmpireData, GameEmpireData::TotalFuelUse, &vTotalFuelUse);
		Assert (vTotalFuelUse.GetInteger() >= 0);

		Variant vCurrentBR;
		iErrCode = m_pGameData->ReadData (pszShips, iShipKey, GameEmpireShips::CurrentBR, &vCurrentBR);
		Assert (vCurrentBR.GetFloat() > 0.0);
#endif

	}

	return iErrCode;
}

int GameEngine::GetNumShips (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumShips) {

	GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);

	return m_pGameData->GetNumRows (pszShips, (unsigned int*) piNumShips);
}

int GameEngine::GetNumFleets (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumFleets) {

	GAME_EMPIRE_SHIPS (pszFleets, iGameClass, iGameNumber, iEmpireKey);

	return m_pGameData->GetNumRows (pszFleets, (unsigned int*) piNumFleets);
}

int GameEngine::ChangeShipCloakingState (int iShipKey, int iPlanetKey, bool bCloaked, 
										 const char* strEmpireShips, const char* strEmpireMap, 
										 const char* strGameMap) {
	
	int iErrCode;
	unsigned int iProxyKey;

	int iCloakedIncrement, iUnCloakedIncrement;

#ifdef _DEBUG

	Variant vOldState, vNumShips;
	iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::State, &vOldState);
	Assert (iErrCode == OK);

#endif
	
	// Set ship as cloaked
	if (bCloaked) {

		Assert (!(vOldState.GetInteger() & CLOAKED));

		iErrCode = m_pGameData->WriteOr (
			strEmpireShips,
			iShipKey, 
			GameEmpireShips::State, 
			CLOAKED
			);

		iCloakedIncrement = 1;
		iUnCloakedIncrement = -1;
	
	} else {

		Assert (vOldState.GetInteger() & CLOAKED);

		iErrCode = m_pGameData->WriteAnd (
			strEmpireShips,
			iShipKey, 
			GameEmpireShips::State, 
			~CLOAKED
			);

		iCloakedIncrement = -1;
		iUnCloakedIncrement = 1;
	}

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	// Change number of cloaked ships
	iErrCode = m_pGameData->GetFirstKey (
		strEmpireMap, 
		GameEmpireMap::PlanetKey, 
		iPlanetKey, 
		false, 
		&iProxyKey
		);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	iErrCode = m_pGameData->Increment (strEmpireMap, iProxyKey, GameEmpireMap::NumCloakedShips, iCloakedIncrement);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	iErrCode = m_pGameData->Increment (strEmpireMap, iProxyKey, GameEmpireMap::NumUncloakedShips, iUnCloakedIncrement);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumCloakedShips, iCloakedIncrement);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumUncloakedShips, iUnCloakedIncrement);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

#ifdef _DEBUG

	iErrCode = m_pGameData->ReadData (strEmpireMap, iProxyKey, GameEmpireMap::NumUncloakedShips, &vNumShips);
	Assert (iErrCode == OK);

	if (bCloaked) {
		Assert (vNumShips.GetInteger() >= 0);
	} else {
		Assert (vNumShips.GetInteger() >= 1);
	}

	iErrCode = m_pGameData->ReadData (strEmpireMap, iProxyKey, GameEmpireMap::NumCloakedShips, &vNumShips);
	Assert (iErrCode == OK);

	if (bCloaked) {
		Assert (vNumShips.GetInteger() >= 1);
	} else {
		Assert (vNumShips.GetInteger() >= 0);
	}

	iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::NumUncloakedShips, &vNumShips);
	Assert (iErrCode == OK);

	if (bCloaked) {
		Assert (vNumShips.GetInteger() >= 0);
	} else {
		Assert (vNumShips.GetInteger() >= 1);
	}

	iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::NumCloakedShips, &vNumShips);
	Assert (iErrCode == OK);

	if (bCloaked) {
		Assert (vNumShips.GetInteger() >= 1);
	} else {
		Assert (vNumShips.GetInteger() >= 0);
	}

#endif
	
Cleanup:

	return iErrCode;
}