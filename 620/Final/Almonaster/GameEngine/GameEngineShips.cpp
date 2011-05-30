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

#include "Osal/OS.h"

// TODO: transactions?

// Delete a ship from a game
int GameEngine::DeleteShip (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey) {

    int iErrCode, iErrCode2;
    unsigned int iKey, iFleetKey;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    Variant vTemp, vCancelBuild, vState, vTechKey, vPlanetKey, vMaxBR;

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
    iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iFleetKey = vTemp.GetInteger();

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
    if (iFleetKey != NO_KEY) {
        
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

        float fCurMil = vCurMil.GetFloat() * vCurMil.GetFloat();
        
        // Reduce fleet's current strength
        iErrCode = m_pGameData->Increment (
            strEmpireFleets, 
            iFleetKey, 
            GameEmpireFleets::CurrentStrength, 
            - fCurMil
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Reduce fleet's max strength
        iErrCode = m_pGameData->Increment (
            strEmpireFleets, 
            iFleetKey, 
            GameEmpireFleets::MaxStrength, 
            - fMaxMil
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Reduce number of ships
        iErrCode = m_pGameData->Increment (
            strEmpireFleets, 
            iFleetKey, 
            GameEmpireFleets::NumShips, 
            -1
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Decrement number of "BuildShips" if necessary
        if (vCancelBuild.GetInteger() != 0) {
            iErrCode = m_pGameData->Increment (strEmpireFleets, iFleetKey, GameEmpireFleets::BuildShips, -1);
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

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *pRatInfo -> Ratio information
//
// Returns ratio and next ratio information

int GameEngine::GetRatioInformation (int iGameClass, int iGameNumber, int iEmpireKey, RatioInformation* pRatInfo) {

    int iBuild, iMaint, iAg, iFuel, iMin, iFuelUse, iNextMaint, iNextFuelUse, 
        iNextMinAdjustment, iNextFuelAdjustment, iBonusAg, iBonusFuel, iBonusMin, iTotalPop, 
        iNextTotalPop, iErrCode, iNumTrades, iPercentFirstTradeIncrease, iPercentNextTradeIncrease;

    float fTechLevel, fMaxAgRatio, fMaxTechDev;

    Variant vTemp;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    IReadTable* pTable = NULL;
    iErrCode = m_pGameData->GetTableForReading (strGameEmpireData, &pTable);
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

    iErrCode = pTable->ReadData (GameEmpireData::TotalAg, &iAg);
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

    iErrCode = pTable->ReadData (GameEmpireData::BonusAg, &iBonusAg);
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

    iErrCode = pTable->ReadData (GameEmpireData::TotalPop, &iTotalPop);
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

    iErrCode = pTable->ReadData (GameEmpireData::NextMin, &iNextMinAdjustment);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (GameEmpireData::NextFuel, &iNextFuelAdjustment);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (GameEmpireData::NextTotalPop, &iNextTotalPop);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (GameEmpireData::NumTrades, &iNumTrades);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    SafeRelease (pTable);

    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxTechDev, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    fMaxTechDev = vTemp.GetFloat();

    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    fMaxAgRatio = vTemp.GetFloat();

    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::PercentFirstTradeIncrease, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    iPercentFirstTradeIncrease = vTemp.GetInteger();

    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::PercentNextTradeIncrease, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iPercentNextTradeIncrease = vTemp.GetInteger();

    // Current
    pRatInfo->fAgRatio = GetAgRatio (iAg + iBonusAg, iTotalPop, fMaxAgRatio);

    pRatInfo->fMaintRatio = GetMaintenanceRatio (iMin + iBonusMin, iMaint, iBuild);
    
    pRatInfo->fFuelRatio = GetFuelRatio (iFuel + iBonusFuel, iFuelUse);

    pRatInfo->fTechLevel = fTechLevel;

    pRatInfo->fTechDev = GetTechDevelopment (
        iFuel + iBonusFuel, 
        iMin + iBonusMin, 
        iMaint, 
        iBuild, 
        iFuelUse, 
        fMaxTechDev
        );

    pRatInfo->iBR = GetBattleRank (fTechLevel);


    //
    // Next ratios
    //

    // First, calculate next bonus ratios

    int iNextAg = iAg;
    int iNextMin = iMin + iNextMinAdjustment;
    int iNextFuel = iFuel + iNextFuelAdjustment;

    int iNextBonusAg, iNextBonusMin, iNextBonusFuel;

    CalculateTradeBonuses (
        iNumTrades, iNextAg, iNextMin, iNextFuel, iPercentFirstTradeIncrease, iPercentNextTradeIncrease,
        &iNextBonusAg, &iNextBonusMin, &iNextBonusFuel
        );

    pRatInfo->fNextAgRatio = GetAgRatio (iNextAg + iNextBonusAg, iNextTotalPop, fMaxAgRatio);

    pRatInfo->fNextMaintRatio = GetMaintenanceRatio (
        iNextMin + iNextBonusMin, 
        iMaint + iNextMaint,
        0
        );

    pRatInfo->fNextFuelRatio = GetFuelRatio (
        iNextFuel + iNextBonusFuel, 
        iFuelUse + iNextFuelUse
        );

    pRatInfo->fNextTechLevel = fTechLevel + pRatInfo->fTechDev;

    pRatInfo->fNextTechDev = GetTechDevelopment (
        iNextFuel + iNextBonusFuel,
        iNextMin + iNextBonusMin, 
        iMaint + iNextMaint, 
        0,
        iFuelUse + iNextFuelUse, 
        fMaxTechDev
        );

    pRatInfo->iNextBR = GetBattleRank (pRatInfo->fNextTechLevel);

Cleanup:

    SafeRelease (pTable);

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

int GameEngine::GetShipOrders (unsigned int iGameClass, unsigned int iGameNumber, unsigned int iEmpireKey, 
                               unsigned int iShipKey, const ShipOrderShipInfo* pShipInfo, 
                               const ShipOrderGameInfo* pGameInfo, const ShipOrderPlanetInfo* pPlanetInfo, 
                               const GameConfiguration& gcConfig,
                               const BuildLocation* pblLocations, unsigned int iNumLocations,
                               ShipOrder** ppsoOrder, unsigned int* piNumOrders, int* piSelectedOrder) {

    int iErrCode = OK;

    GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    bool pbLink [NUM_CARDINAL_POINTS];
    bool pbMustExplore [NUM_CARDINAL_POINTS];

    char strEmpireMap [256], strEmpireDip [256];
    char pszOrder [MAX_PLANET_NAME_LENGTH + MAX_FLEET_NAME_LENGTH + 256];

    String strPlanetNameString, strNewPlanetName, strFleetName;
    Variant vTemp;

    const char* pszPlanetName;
    bool bBuilding, bMobile;
    unsigned int iMaxNumOrders, i, iProxyPlanetKey, iPlanetKey, iPlanetOwner;
    int iNewX, iNewY, iShipType, iLocationX, iLocationY, iGameClassOptions, iSelectedAction, iState;
    float fBR, fMaxBR, fMaintRatio, fNextBR;

    *ppsoOrder = NULL;
    *piNumOrders = 0;
    *piSelectedOrder = STAND_BY;

    ShipOrder* psoOrders = NULL;
    unsigned int iNumOrders = 0;

    //
    // Collect info
    //
    if (pShipInfo != NULL) {

        iShipType = pShipInfo->iShipType;
        iSelectedAction = pShipInfo->iSelectedAction;
        iState = pShipInfo->iState;
        fBR = pShipInfo->fBR;
        fMaxBR = pShipInfo->fMaxBR;
        bBuilding = pShipInfo->bBuilding;

    } else {

        iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        fBR = vTemp.GetFloat();

        iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        fMaxBR = vTemp.GetFloat();

        iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::Type, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iShipType = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::Action, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iSelectedAction = vTemp.GetInteger();

        // Get ship state
        iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::State, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iState = vTemp.GetInteger();

        // Check for newly built ship
        iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        bBuilding = vTemp.GetInteger() != 0;
    }

    if (pGameInfo != NULL) {

        fMaintRatio = pGameInfo->fMaintRatio;
        iGameClassOptions = pGameInfo->iGameClassOptions;
    
    } else {

        iErrCode = GetEmpireMaintenanceRatio (iGameClass, iGameNumber, iEmpireKey, &fMaintRatio);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (pPlanetInfo != NULL) {

        iPlanetKey = pPlanetInfo->iPlanetKey;
        iPlanetOwner = pPlanetInfo->iOwner;
        pszPlanetName = pPlanetInfo->pszName;
        iLocationX = pPlanetInfo->iX;
        iLocationY = pPlanetInfo->iY;
    
    } else {

        iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentPlanet, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iPlanetKey = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iPlanetOwner = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Name, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (String::AtoHtml (vTemp.GetCharPtr(), &strPlanetNameString, 0, false) == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            Assert (false);
            goto Cleanup;
        }
        pszPlanetName = strPlanetNameString.GetCharPtr();

        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        GetCoordinates (vTemp.GetCharPtr(), &iLocationX, &iLocationY);
    }

    // Get ship's next BR
    fNextBR = GetShipNextBR (fBR, fMaintRatio);    
    if (fNextBR > fMaxBR) {
        fNextBR = fMaxBR;
    }

    // Is mobile ship?
    bMobile = IsMobileShip (iShipType);

    /////////////////////////
    // Building ship orders//
    /////////////////////////

    if (bBuilding) {

        BuildLocation* pblOurLocations = NULL;
        Algorithm::AutoDelete<BuildLocation> autoDel (pblOurLocations, true);

        String strFleetName;

        if (pblLocations == NULL) {

            iErrCode = GetBuildLocations (
                iGameClass,
                iGameNumber,
                iEmpireKey,
                NO_KEY,
                &pblOurLocations,
                &iNumLocations
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            pblLocations = pblOurLocations;
        }

        // Allocate orders
        iMaxNumOrders = iNumLocations + 1;

        psoOrders = new ShipOrder [iMaxNumOrders];
        if (psoOrders == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }
        memset (psoOrders, 0, iMaxNumOrders * sizeof (ShipOrder));
        Assert (SHIP_ORDER_NORMAL == 0);

        // First, add only the locations at our current planet
        for (i = 0; i < iNumLocations; i ++) {

            // Optimization: orders are always collected by planet
            if (iNumOrders > 0 && pblLocations[i].iPlanetKey != iPlanetKey) {
                break;
            }

            if (pblLocations[i].iPlanetKey != iPlanetKey ||
                pblLocations[i].iFleetKey == FLEET_NEWFLEETKEY) {
                continue;
            }

            if (pblLocations[i].iFleetKey == NO_KEY) {
                
                sprintf (pszOrder, "Build at %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);
                psoOrders[iNumOrders].iKey = BUILD_AT;
            
            } else if (bMobile) {
                
                // Get fleet name
                iErrCode = m_pGameData->ReadData (
                    strEmpireFleets, pblLocations[i].iFleetKey, GameEmpireFleets::Name, &vTemp
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                // Filter fleet name
                if (String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false) == NULL) {
                    iErrCode = ERROR_OUT_OF_MEMORY;
                    goto Cleanup;
                }

                sprintf (pszOrder, "Build at %s (%i,%i) in fleet %s", 
                    pszPlanetName, iLocationX, iLocationY, strFleetName.GetCharPtr());

                psoOrders[iNumOrders].iKey = pblLocations[i].iFleetKey;
            
            } else {

                // Immobile ships don't build into fleets
                continue;
            }

            psoOrders[iNumOrders].sotType = SHIP_ORDER_NORMAL;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }

        // Second, add planets not our own
        int iX = 0, iY = 0;
        unsigned int iCachedPlanetKey = NO_KEY;
        String strPlanetName;
        bool bNoPop = false;

        for (i = 0; i < iNumLocations; i ++) {

            // Ignore our planet and 'new fleet' locations
            if (pblLocations[i].iPlanetKey == iPlanetKey ||
                pblLocations[i].iFleetKey == FLEET_NEWFLEETKEY) {
                continue;
            }

            // If not the same as the planet in the previous loop, read some planet info
            if (iCachedPlanetKey == pblLocations[i].iPlanetKey) {

                if (bNoPop) continue;

            } else {

                // If the current ship is a colony, check the destination planet's population
                if (iShipType == COLONY) {

                    unsigned int iPop;
                    iErrCode = GetPlanetPopulationWithColonyBuilds (
                        iGameClass,
                        iGameNumber,
                        iEmpireKey,
                        pblLocations[i].iPlanetKey,
                        &iPop
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    unsigned int iCost = GetColonyPopulationBuildCost (
                        gcConfig.iShipBehavior, 
                        gcConfig.fColonyMultipliedBuildFactor, 
                        gcConfig.iColonySimpleBuildFactor, 
                        fBR
                        );

                    if (iPop < iCost) {
                        bNoPop = true;
                        continue;
                    }
                }
                bNoPop = false;

                iErrCode = GetPlanetName (iGameClass, iGameNumber, pblLocations[i].iPlanetKey, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false) == NULL) {
                    iErrCode = ERROR_OUT_OF_MEMORY;
                    goto Cleanup;
                }

                iErrCode = GetPlanetCoordinates (iGameClass, iGameNumber, pblLocations[i].iPlanetKey, &iX, &iY);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iCachedPlanetKey = pblLocations[i].iPlanetKey;
            }

            sprintf (pszOrder, "Build at %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

            if (pblLocations[i].iFleetKey == NO_KEY) {

                psoOrders[iNumOrders].iKey = pblLocations[i].iPlanetKey;
                psoOrders[iNumOrders].sotType = SHIP_ORDER_MOVE_PLANET;
            
            } else if (bMobile) {

                psoOrders[iNumOrders].iKey = pblLocations[i].iFleetKey;
                psoOrders[iNumOrders].sotType = SHIP_ORDER_MOVE_FLEET;

                iErrCode = GetFleetProperty (
                    iGameClass, iGameNumber, iEmpireKey, pblLocations[i].iFleetKey, 
                    GameEmpireFleets::Name, &vTemp
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false) == NULL) {
                    iErrCode = ERROR_OUT_OF_MEMORY;
                    goto Cleanup;
                }

                strcat (pszOrder, " in fleet ");
                strcat (pszOrder, strFleetName.GetCharPtr());

            } else {

                // Immobile ships don't build into fleets
                continue;
            }

            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }

        // Add cancel build order
        psoOrders[iNumOrders].sotType = SHIP_ORDER_NORMAL;
        psoOrders[iNumOrders].iKey = CANCEL_BUILD;
        psoOrders[iNumOrders].pszText = String::StrDup ("Cancel Build");
        if (psoOrders[iNumOrders].pszText == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }
        iNumOrders ++;

        goto NoErrorCleanup;
    }
    
    /////////////////////////////
    // Non-building ship orders//
    /////////////////////////////

    GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    unsigned int iNumFleets, iMaxNumPlanets;

    // Calculate the max number of orders we'll ever have
    iErrCode = m_pGameData->GetNumRows (strEmpireFleets, &iNumFleets);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = m_pGameData->GetNumRows (strEmpireMap, &iMaxNumPlanets);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iMaxNumOrders = 8 + iNumFleets + max (iMaxNumPlanets, NUM_CARDINAL_POINTS) + NUM_SHIP_TYPES;

    psoOrders = new ShipOrder [iMaxNumOrders];
    if (psoOrders == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }
    memset (psoOrders, 0, iMaxNumOrders * sizeof (ShipOrder));
    Assert (SHIP_ORDER_NORMAL == 0);

    ///////////////////
    // Add "Standby" //
    ///////////////////

    sprintf (pszOrder, "Standby at %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

    psoOrders[iNumOrders].iKey = STAND_BY;
    psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
    if (psoOrders[iNumOrders].pszText == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }
    iNumOrders ++;

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
    if (bMobile) {

        //////////
        // Move //
        //////////

        int iLink, iExplored;
        unsigned int iNewPlanetKey;
        String strNewPlanetName;

        // Read explored
        iErrCode = m_pGameData->ReadData (strEmpireMap, iProxyPlanetKey, GameEmpireMap::Explored, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iExplored = vTemp.GetInteger();
        
        // Loop through all cardinal points
        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Link, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iLink = vTemp.GetInteger();

        ENUMERATE_CARDINAL_POINTS(i) {

            // Is there a link in this direction?
            pbLink[i] = ((iLink & LINK_X[i]) != 0);
            if (pbLink[i]) {

                pbMustExplore[i] = !(iExplored & EXPLORED_X[i]);
                if (!pbMustExplore[i]) {
                
                    // Read planet key
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::NorthPlanetKey + i, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iNewPlanetKey = vTemp.GetInteger();
                    Assert (iNewPlanetKey != NO_KEY);

                    // Get name
                    iErrCode = m_pGameData->ReadData (strGameMap, iNewPlanetKey, GameMap::Name, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    if (String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false) == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    AdvanceCoordinates (iLocationX, iLocationY, &iNewX, &iNewY, i);

                    // Add order
                    sprintf (
                        pszOrder, 
                        "Move %s to %s (%i,%i)", 
                        CARDINAL_STRING[i], 
                        strNewPlanetName.GetCharPtr(), 
                        iNewX, 
                        iNewY
                        );

                    psoOrders[iNumOrders].iKey = MOVE_NORTH - i;
                    psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                    if (psoOrders[iNumOrders].pszText == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                    iNumOrders ++;
                }

            } else {

                pbMustExplore[i] = false;
            }

        }   // End cardinal points loop
        

        //////////
        // Nuke //
        //////////

        // Make sure not an illegal nuking sci
        if (iShipType != SCIENCE || !(iGameClassOptions & DISABLE_SCIENCE_SHIPS_NUKING)) {

            // Check the owner
            if (iPlanetOwner != iEmpireKey && iPlanetOwner != SYSTEM) {
                
                // We're on someone else's planet, so check the dip status
                int iDipStatus = WAR;
                if (iPlanetOwner != INDEPENDENT) {
                    
                    unsigned int iKey;
                    iErrCode = m_pGameData->GetFirstKey (
                        strEmpireDip, GameEmpireDiplomacy::EmpireKey, iPlanetOwner, false, &iKey
                        );

                    if (iErrCode == ERROR_DATA_NOT_FOUND) {
                        iErrCode = OK;
                    } else {
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->ReadData (
                            strEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vTemp
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        iDipStatus = vTemp.GetInteger();
                    }
                }
                    
                if (iDipStatus == WAR) {
                    
                    // Well, we're at war, so if the ship isn't cloaked we can nuke
                    if (!(iState & CLOAKED)) {

                        sprintf (pszOrder, "Nuke %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

                        psoOrders[iNumOrders].iKey = NUKE;
                        psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                        if (psoOrders[iNumOrders].pszText == NULL) {
                            iErrCode = ERROR_OUT_OF_MEMORY;
                            goto Cleanup;
                        }
                        iNumOrders ++;
                    }
                }

            }   // If we can nuke the planet

        }   // End if mobile ship

    }   // End if not sci or sci can nuke


    /////////////////////////
    // Add Special Actions //
    /////////////////////////

    switch (iShipType) {

    case SCIENCE:

        ENUMERATE_CARDINAL_POINTS(i) {
            
            if (!pbMustExplore[i]) {
                continue;
            }

            AdvanceCoordinates (iLocationX, iLocationY, &iNewX, &iNewY, i);
            
            sprintf (pszOrder, "Explore %s to Planet %i,%i", CARDINAL_STRING[i], iNewX, iNewY);

            psoOrders[iNumOrders].iKey = EXPLORE_NORTH - i;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }
        
        break;

    case COLONY:

        bool bColonize, bSettle;
        iErrCode = GetColonyOrders (iGameClass, iGameNumber, iEmpireKey, iPlanetKey, &bColonize, &bSettle);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (bColonize) {

            sprintf (pszOrder, "Colonize %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

            psoOrders[iNumOrders].iKey = COLONIZE;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }

        else if (bSettle) {

            sprintf (pszOrder, "Settle %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

            psoOrders[iNumOrders].iKey = DEPOSIT_POP;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }        
        break;

    case STARGATE:
        
        // Check next BR
        if (fNextBR >= gcConfig.fStargateGateCost) {

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
                if (iKey == iPlanetKey) {
                    continue;
                }

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
                    iErrCode = m_pGameData->ReadData (strGameMap, iKey, GameMap::Name, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        m_pGameData->FreeKeys (piKey);
                        goto Cleanup;
                    }

                    if (String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false) == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        Assert (false);
                        m_pGameData->FreeKeys (piKey);
                        goto Cleanup;
                    }
                    
                    // Add send to Order
                    sprintf (pszOrder, "Stargate ships to %s (%i,%i)", 
                        strNewPlanetName.GetCharPtr(), iDestX, iDestY);
                    
                    psoOrders[iNumOrders].iKey = iKey;
                    psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                    if (psoOrders[iNumOrders].pszText == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                    iNumOrders ++;
                }
            }

            m_pGameData->FreeKeys (piKey);
        }

        break;

    case CLOAKER:

        int iKey;
        const char* pszCloak;

        if (iState & CLOAKED) {
            iKey = UNCLOAK;
            pszCloak = "Uncloak";
        } else {
            iKey = CLOAK;
            pszCloak = "Cloak";
        }

        psoOrders[iNumOrders].iKey = iKey;
        psoOrders[iNumOrders].pszText = String::StrDup (pszCloak);
        if (psoOrders[iNumOrders].pszText == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }
        iNumOrders ++;

        break;

    case TERRAFORMER:
    
        bool bTerraform, bTerraformAndDismantle;

        iErrCode = GetTerraformerOrders (
            iGameClass,
            iGameNumber,
            iEmpireKey,
            iPlanetKey,
            gcConfig,
            &bTerraform,
            &bTerraformAndDismantle
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (bTerraform) {
        
            sprintf (pszOrder, "Terraform %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);
            
            psoOrders[iNumOrders].iKey = TERRAFORM;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }

        if (bTerraformAndDismantle) {

            sprintf (pszOrder, "Terraform %s (%i,%i) and dismantle", pszPlanetName, iLocationX, iLocationY);

            psoOrders[iNumOrders].iKey = TERRAFORM_AND_DISMANTLE;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }

        break;

    case TROOPSHIP:

        bool bInvade, bInvadeAndDismantle;

        iErrCode = GetTroopshipOrders (
            iGameClass,
            iGameNumber,
            iEmpireKey,
            iPlanetKey,
            gcConfig,
            &bInvade,
            &bInvadeAndDismantle
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (bInvade) {

            sprintf (pszOrder, "Invade %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

            psoOrders[iNumOrders].iKey = INVADE;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }

        if (bInvadeAndDismantle) {

            sprintf (pszOrder, "Invade %s (%i,%i) and dismantle", pszPlanetName, iLocationX, iLocationY);

            psoOrders[iNumOrders].iKey = INVADE_AND_DISMANTLE;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }

        break;

    case DOOMSDAY:
    
        bool bAnnihilate;

        iErrCode = GetDoomsdayOrders (
            iGameClass,
            iGameNumber,
            iEmpireKey,
            iPlanetKey,
            gcConfig,
            iGameClassOptions,
            &bAnnihilate
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (bAnnihilate) {
            
            sprintf (pszOrder, "Annihilate %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);
            
            psoOrders[iNumOrders].iKey = ANNIHILATE;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }

        break;

    case MINEFIELD:

        if (!(gcConfig.iShipBehavior & MINEFIELD_DISABLE_DETONATE)) {

            sprintf (
                pszOrder, "Detonate %s at %s (%i,%i)", 
                SHIP_TYPE_STRING_LOWERCASE [MINEFIELD], 
                pszPlanetName, iLocationX, iLocationY
                );

            psoOrders[iNumOrders].iKey = DETONATE;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            if (psoOrders[iNumOrders].pszText == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }
            iNumOrders ++;
        }
        break;

    case ENGINEER:
        
        // Check next BR
        if (fNextBR >= gcConfig.fEngineerLinkCost) {

            int iExplored;

            iErrCode = m_pGameData->ReadData (strEmpireMap, iProxyPlanetKey, GameEmpireMap::Explored, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            iExplored = vTemp.GetInteger();

            ENUMERATE_CARDINAL_POINTS(i) {
                
                AdvanceCoordinates (iLocationX, iLocationY, &iNewX, &iNewY, i);

                int iOrderKey;
                const char* pszNewPlanet;
                const char* pszOpenClose;

                // If there's a link, then offer to open it, else offer to close it
                if (pbLink[i]) {

                    iOrderKey = CLOSE_LINK[i];
                    pszOpenClose = "Close";

                    if (!(iExplored & EXPLORED_X[i])) {
                        pszNewPlanet = "Unknown";
                    } else {

                        iErrCode = m_pGameData->ReadData (
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::NorthPlanetKey + i, 
                            &vTemp
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        iErrCode = m_pGameData->ReadData (strGameMap, vTemp.GetInteger(), GameMap::Name, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        if (String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false) == NULL) {
                            iErrCode = ERROR_OUT_OF_MEMORY;
                            Assert (false);
                            goto Cleanup;
                        }

                        pszNewPlanet = strNewPlanetName.GetCharPtr();
                    }
                    
                } else {    // if (!pbLink[i])

                    // See if there's a planet at all
                    iErrCode = m_pGameData->ReadData (
                        strGameMap, 
                        iPlanetKey, 
                        GameMap::NorthPlanetKey + i, 
                        &vTemp
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    unsigned int iNewPlanetKey = vTemp.GetInteger();
                    if (iNewPlanetKey == NO_KEY){
                        Assert (!(iExplored & EXPLORED_X[i]));
                        continue;
                    }

                    iOrderKey = OPEN_LINK[i];
                    pszOpenClose = "Open";

                    if (!(iExplored & EXPLORED_X[i])) {
                        pszNewPlanet = "Unknown";
                    } else {

                        iErrCode = m_pGameData->ReadData (strGameMap, iNewPlanetKey, GameMap::Name, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        if (String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false) == NULL) {
                            iErrCode = ERROR_OUT_OF_MEMORY;
                            Assert (false);
                            goto Cleanup;
                        }

                        pszNewPlanet = strNewPlanetName.GetCharPtr();
                    }
                
                }   // End if (!pbLink[i])

                sprintf (
                    pszOrder,
                    "%s %s link to %s (%i,%i)",
                    pszOpenClose,
                    CARDINAL_STRING[i],
                    pszNewPlanet,
                    iNewX,
                    iNewY
                    );

                psoOrders[iNumOrders].iKey = iOrderKey;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                if (psoOrders[iNumOrders].pszText == NULL) {
                    iErrCode = ERROR_OUT_OF_MEMORY;
                    goto Cleanup;
                }
                iNumOrders ++;

            }   // End cardinal point enumeration
        }

        break;

    case BUILDER:

        // Check next BR
        if (fNextBR >= gcConfig.fBuilderMinBR) {

            ENUMERATE_CARDINAL_POINTS(i) {
                
                iErrCode = m_pGameData->ReadData (
                    strGameMap, 
                    iPlanetKey, 
                    GameMap::NorthPlanetKey + i, 
                    &vTemp
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (vTemp.GetInteger() == NO_KEY) {
                    
                    AdvanceCoordinates (iLocationX, iLocationY, &iNewX, &iNewY, i);
                    
                    sprintf (
                        pszOrder, 
                        "Create new planet to the %s (%i,%i)", 
                        CARDINAL_STRING[i],
                        iNewX,
                        iNewY
                        );

                    psoOrders[iNumOrders].iKey = CREATE_PLANET_LINK[i];
                    psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                    if (psoOrders[iNumOrders].pszText == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                    iNumOrders ++;
                }
            }
        }
        break;

    case JUMPGATE:

        // Check next BR
        if (fNextBR >= gcConfig.fJumpgateGateCost) {

            Variant* pvPlanetKey = NULL;

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
                if (iKey != iPlanetKey) {
                    
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
                            iErrCode = m_pGameData->ReadData (strGameMap, iKey, GameMap::Name, &vTemp);
                            if (iErrCode != OK) {
                                Assert (false);
                                m_pGameData->FreeData (pvPlanetKey);
                                goto Cleanup;
                            }

                            if (String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false) == NULL) {
                                iErrCode = ERROR_OUT_OF_MEMORY;
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            // Add send to Order
                            sprintf (pszOrder, "Jumpgate ships to %s (%i,%i)", 
                                strNewPlanetName.GetCharPtr(), iDestX, iDestY);
                            
                            psoOrders[iNumOrders].iKey = iKey;
                            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                            if (psoOrders[iNumOrders].pszText == NULL) {
                                iErrCode = ERROR_OUT_OF_MEMORY;
                                goto Cleanup;
                            }
                            iNumOrders ++;
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

    if ((iState & MORPH_ENABLED) && 
        !(iState & CLOAKED) &&     // Cloaked morphers can't morph
        fNextBR >= gcConfig.fMorpherCost) {

        GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
        
        // Get developed techs
        iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::TechDevs, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        int iTechs = vTemp.GetInteger();

        ENUMERATE_SHIP_TYPES (i) {

            if (i != (unsigned int) iShipType && (iTechs & TECH_BITS[i])) {

                sprintf (pszOrder, "Morph into %s", SHIP_TYPE_STRING[i]);
                
                psoOrders[iNumOrders].iKey = MORPH_ORDER[i];
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                if (psoOrders[iNumOrders].pszText == NULL) {
                    iErrCode = ERROR_OUT_OF_MEMORY;
                    goto Cleanup;
                }
                iNumOrders ++;
            }
        }       
    }
    
    /////////////////////
    // Add Fleet Joins //
    /////////////////////
    
    if (IsMobileShip (iShipType)) {
        
        // Get all fleets located at planet
        unsigned int* piFleetKey = NULL, iNumFleets;
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

            iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                m_pGameData->FreeKeys (piFleetKey);
                goto Cleanup;
            }
            unsigned int iFleetKey = vTemp.GetInteger();
            
            for (i = 0; i < iNumFleets; i ++) {

                if (iFleetKey == piFleetKey[i]) {
                    
                    // Remain in own fleet
                    iErrCode = m_pGameData->ReadData (
                        strEmpireFleets, 
                        iFleetKey, 
                        GameEmpireFleets::Name, 
                        &vTemp
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        m_pGameData->FreeKeys (piFleetKey);
                        goto Cleanup;
                    }

                    // Filter fleet name
                    if (String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false) == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        m_pGameData->FreeKeys (piFleetKey);
                        goto Cleanup;
                    }

                    sprintf (pszOrder, "Remain in fleet %s", strFleetName.GetCharPtr());

                    psoOrders[iNumOrders].iKey = FLEET;
                    psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                    if (psoOrders[iNumOrders].pszText == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                    iNumOrders ++;

                    // Leave own fleet
                    sprintf (pszOrder, "Leave fleet %s", strFleetName.GetCharPtr());

                    psoOrders[iNumOrders].iKey = LEAVE_FLEET;
                    psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                    if (psoOrders[iNumOrders].pszText == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                    iNumOrders ++;

                } else {

                    // Join another fleet
                    iErrCode = m_pGameData->ReadData (
                        strEmpireFleets, 
                        piFleetKey[i], 
                        GameEmpireFleets::Name, 
                        &vTemp
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        m_pGameData->FreeKeys (piFleetKey);
                        goto Cleanup;
                    }

                    // Filter fleet name
                    if (String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false) == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        m_pGameData->FreeKeys (piFleetKey);
                        goto Cleanup;
                    }
                    
                    sprintf (pszOrder, "Join fleet %s", strFleetName.GetCharPtr());

                    psoOrders[iNumOrders].iKey = piFleetKey[i];
                    psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                    if (psoOrders[iNumOrders].pszText == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                    iNumOrders ++;
                }
            }
            
            m_pGameData->FreeKeys (piFleetKey);
        }
    }

    ///////////////////
    // Add dismantle //
    ///////////////////

    psoOrders[iNumOrders].iKey = DISMANTLE;
    psoOrders[iNumOrders].pszText = String::StrDup ("Dismantle");
    if (psoOrders[iNumOrders].pszText == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }
    iNumOrders ++;

NoErrorCleanup:

    *piNumOrders = iNumOrders;
    *piSelectedOrder = iSelectedAction;
    *ppsoOrder = psoOrders;
    psoOrders = NULL;

Cleanup:

    // Allocation check
    if (psoOrders != NULL) {
        FreeShipOrders (psoOrders, iNumOrders);
    }

    return iErrCode;
}

void GameEngine::FreeShipOrders (ShipOrder* psoOrders, unsigned int iNumOrders) {

    for (unsigned int i = 0; i < iNumOrders; i ++) {
        if (psoOrders[i].pszText != NULL) {
            OS::HeapFree (psoOrders[i].pszText);
        }
    }
    delete [] psoOrders;
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
//  ERROR_SHIP_DOES_NOT_EXIST -> Ship doesn't exist
//  ERROR_FLEET_DOES_NOT_EXIST -> Fleet doesn't exist
//  ERROR_WRONG_PLANET -> Ship and fleet are on different planets
//  ERROR_CANNOT_MOVE -> Ship cannot move in the requested direction
//  ERROR_CANNOT_EXPLORE -> Ship cannot explore in the requested direction
//  ERROR_CANNOT_COLONIZE -> Ship cannot colonize the requested planet
//  ERROR_CANNOT_SETTLE -> Ship cannot deposit pop on the requested planet
//  ERROR_WRONG_SHIP_TYPE -> Ship cannot perform special action
//  ERROR_CANNOT_CLOAK -> Ship is already cloaked
//  ERROR_CANNOT_UNCLOAK -> Ship is already uncloaked
//  ERROR_CANNOT_TERRAFORM -> Ship cannot terraform
//  ERROR_CANNOT_NUKE -> Ship cannot nuke
//  ERROR_CANNOT_ANNIHILATE -> Ship cannot annihilate
//  ERROR_CANNOT_OPEN_LINK -> Ship cannot open link
//  ERROR_CANNOT_CLOSE_LINK -> Ship cannot close link
//  ERROR_UNKNOWN_ORDER -> Unknown order
//  ERROR_CANNOT_GATE -> Ship cannot gate to planet
//  ERROR_CANNOT_INVADE -> Ship cannot invade
//  ERROR_PLANET_EXISTS -> Ship cannot create planet because it already exists
//  ERROR_CANNOT_MORPH -> Ship cannot morph because it was cloaked
//
// Updates the a given ship's current orders

// TODO: transaction

int GameEngine::UpdateShipOrders (unsigned int iGameClass, unsigned int iGameNumber, unsigned int iEmpireKey, 
                                  unsigned int iShipKey, const ShipOrder& soOrder) {

    GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    
    Variant vTemp;

    // Check ship key
    bool bShipExists, bOldDismantle, bDismantle = false;
    int i, iShipBehavior, iErrCode, iShipState, iOldOrder, iGameClassOptions, iShipType, iShipPlanet;
    int iNewShipOrder = soOrder.iKey;
    unsigned int iOldFleetKey;
    
    iErrCode = m_pGameData->DoesRowExist (strEmpireShips, iShipKey, &bShipExists);
    if (iErrCode != OK || !bShipExists) {
        iErrCode = ERROR_SHIP_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Select by type
    switch (soOrder.sotType) {

    case SHIP_ORDER_MOVE_PLANET:

        iErrCode = MoveShip (
            iGameClass,
            iGameNumber,
            iEmpireKey,
            iShipKey,
            soOrder.iKey,
            NO_KEY
            );

        goto Cleanup;

    case SHIP_ORDER_MOVE_FLEET:

        // We need the location of the destination fleet
        unsigned int iDestPlanet;
        iErrCode = GetFleetProperty (
            iGameClass, iGameNumber, iEmpireKey, soOrder.iKey, GameEmpireFleets::CurrentPlanet, &vTemp);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        iDestPlanet = vTemp.GetInteger();

        iErrCode = MoveShip (
            iGameClass,
            iGameNumber,
            iEmpireKey,
            iShipKey,
            iDestPlanet,
            soOrder.iKey
            );

        goto Cleanup;
    }

    Assert (soOrder.sotType == SHIP_ORDER_NORMAL);

    // Check for same order
    iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::Action, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iOldOrder = vTemp.GetInteger();

    if (iOldOrder == iNewShipOrder) {
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
    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iGameClassOptions = vTemp.GetInteger();

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
    iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iOldFleetKey = vTemp.GetInteger();
    
    iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::CurrentPlanet, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iShipPlanet = vTemp.GetInteger();
    Assert (iShipPlanet != NO_KEY);
    
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
            iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, iShipKey);
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
                
                if (iOldFleetKey != NO_KEY) {
                    
                    // Reduce fleet's strength
                    iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::CurrentStrength, -fMil);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::MaxStrength, -fMil);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    // Decrement number of "BuildShips"
                    iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::BuildShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    // Decrement number of ships
                    iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::NumShips, -1);
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

                if (iShipPlanet != vFleetPlanet.GetInteger()) {
                    iErrCode = ERROR_WRONG_PLANET;
                    goto Cleanup;
                }
                
                if (iOldFleetKey != NO_KEY) {

                    // Reduce fleet's strength
                    iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::CurrentStrength, -fMil);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::MaxStrength, -fMil);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    // Decrement number of "BuildShips"
                    iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::BuildShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    // Decrement number of ships
                    iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::NumShips, -1);
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

                // Increment number of ships
                iErrCode = m_pGameData->Increment (strEmpireFleets, iNewShipOrder, GameEmpireFleets::NumShips, 1);
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

    iErrCode = m_pGameData->ReadData (strEmpireShips, iShipKey, GameEmpireShips::Type, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iShipType = vTemp.GetInteger();

    // Was a join fleet requested?
    if (IsMobileShip (iShipType) && iNewShipOrder >= 0) {

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
        
        if (iShipPlanet != vFleetPlanet.GetInteger()) {
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

        iErrCode = m_pGameData->Increment (strEmpireFleets, iNewShipOrder, GameEmpireFleets::NumShips, 1);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Remove ship from old fleet
        if (iOldFleetKey != NO_KEY) {
            
            // Update old fleet's strength
            iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::CurrentStrength, -fCurrentMil);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::MaxStrength, -fMaxMil);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::NumShips, -1);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        // Set orders to follow fleet
        iNewShipOrder = FLEET;
        
    } else {
        
        ///////////////////////////////////////////////////////////////////////////////////////////
        // The order wasn't a join fleet,                                                        //
        // so let's check for leave, stand by, dismantle, nuke, remain, move or explore requests //
        ///////////////////////////////////////////////////////////////////////////////////////////

        iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Owner, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        unsigned int iPlanetOwner = vTemp.GetInteger();

        switch (iNewShipOrder) {

        case STAND_BY:
            break;

        case DISMANTLE:

            bDismantle = true;
            break;

        // Leave fleet
        case LEAVE_FLEET:

            {
                if (iOldFleetKey == NO_KEY) {
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
                
                iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::CurrentStrength, -fCurrentMil);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                // Subtract ship's max strength from fleet's max strength
                fCurrentMil = vMaxBR.GetFloat() * vMaxBR.GetFloat();
                
                iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::MaxStrength, -fCurrentMil);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iErrCode = m_pGameData->Increment (strEmpireFleets, iOldFleetKey, GameEmpireFleets::NumShips, -1);
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

            if (iOldFleetKey == NO_KEY) {
                iErrCode = ERROR_SHIP_IS_NOT_IN_FLEET;
                goto Cleanup;
            }

#ifdef _DEBUG

            {

            Variant vFleetPlanet;

            iErrCode = m_pGameData->ReadData (
                strEmpireFleets, 
                iOldFleetKey, 
                GameEmpireFleets::CurrentPlanet,
                &vFleetPlanet
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            Assert (vFleetPlanet.GetInteger() == iShipPlanet);
            
            }
#endif

            break;

        case MOVE_NORTH:
        case MOVE_EAST:
        case MOVE_SOUTH:
        case MOVE_WEST:

            {
                // Make sure the ship is mobile
                if (!IsMobileShip (iShipType)) {
                    iErrCode = ERROR_CANNOT_MOVE;
                    goto Cleanup;
                }
                
                // Get proxy key for empiremap
                unsigned int iPlanetProxyKey;
                iErrCode = m_pGameData->GetFirstKey (
                    strEmpireMap, 
                    GameEmpireMap::PlanetKey, 
                    iShipPlanet, 
                    false, 
                    &iPlanetProxyKey
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                Variant vLink;
                iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Link, &vLink);
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
                    iShipPlanet, 
                    GameMap::NorthPlanetKey + i, 
                    &vNewPlanetKey
                    );

                Assert (iErrCode == OK && vNewPlanetKey != NO_KEY);
#endif
            }   // End indent

            break;

        case EXPLORE_NORTH:
        case EXPLORE_EAST:
        case EXPLORE_SOUTH:
        case EXPLORE_WEST:

            {           
                // Make sure the ship is a sci
                if (iShipType != SCIENCE) {
                    iErrCode = ERROR_CANNOT_EXPLORE;
                    goto Cleanup;
                }
                
                // Get proxy key for empiremap
                unsigned int iPlanetProxyKey;
                iErrCode = m_pGameData->GetFirstKey (
                    strEmpireMap, 
                    GameEmpireMap::PlanetKey, 
                    iShipPlanet, 
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
                iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Link, &vLink);
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
                    iShipPlanet, 
                    GameMap::NorthPlanetKey + i, 
                    &vNewPlanetKey
                    );
                Assert (iErrCode == OK && vNewPlanetKey != NO_KEY);
#endif
            }   // End indent

            break;

        case NUKE:

            // Make sure ship is mobile
            if (!IsMobileShip (iShipType)) {
                iErrCode = ERROR_CANNOT_NUKE;
                goto Cleanup;
            }
            
            // Make sure ship is not cloaked                
            if (iShipState & CLOAKED) {
                iErrCode = ERROR_CANNOT_NUKE;
                goto Cleanup;
            }

            // Make sure not an illegal nuking sci
            if (iShipType == SCIENCE && (iGameClassOptions & DISABLE_SCIENCE_SHIPS_NUKING)) {
                iErrCode = ERROR_CANNOT_NUKE;
                goto Cleanup;
            }
            
            // Make sure not nuking our own planet or a system planet         
            if (iPlanetOwner == SYSTEM || iPlanetOwner == iEmpireKey) {
                iErrCode = ERROR_CANNOT_NUKE;
                goto Cleanup;
            }
            
            // Check diplomatic status with owner
            if (iPlanetOwner != INDEPENDENT) {
                
                GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
                
                // Make sure the diplomatic status is war
                Variant vDipStatus;
                unsigned int iKey;
                
                iErrCode = m_pGameData->GetFirstKey (
                    strDiplomacy, 
                    GameEmpireDiplomacy::EmpireKey, 
                    iPlanetOwner, 
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

            break;

        case COLONIZE:
            
            // Make sure the ship is a colony
            if (iShipType != COLONY) {
                iErrCode = ERROR_WRONG_SHIP_TYPE;
                goto Cleanup;
            }
            
            // Make sure the planet has 0 pop and wasn't annihilated and doesn't belong to us
            iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Pop, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (vTemp.GetInteger() != 0) {
                iErrCode = ERROR_CANNOT_COLONIZE;
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Owner, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if ((unsigned int) vTemp.GetInteger() == iEmpireKey) {
                iErrCode = ERROR_CANNOT_COLONIZE;
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Annihilated, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (vTemp.GetInteger() != NOT_ANNIHILATED) {
                iErrCode = ERROR_CANNOT_COLONIZE;
                goto Cleanup;
            }

            bDismantle = true;

            break;

        case DEPOSIT_POP:
               
            // Make sure the ship is a colony
            if (iShipType != COLONY) {
                iErrCode = ERROR_WRONG_SHIP_TYPE;
                goto Cleanup;
            }

            // Make sure it's our planet
            if (iPlanetOwner != iEmpireKey) {
                iErrCode = ERROR_CANNOT_SETTLE;
                goto Cleanup;
            }
            
            break;
            
        case CLOAK:
            
            // Make sure the ship is a cloaker
            if (iShipType != CLOAKER) {
                iErrCode = ERROR_WRONG_SHIP_TYPE;
                goto Cleanup;
            }
            
            // Make sure it's not cloaked
            if (iShipState & CLOAKED) {
                iErrCode = ERROR_CANNOT_CLOAK;
                goto Cleanup;
            }
            
            break;
            
        case UNCLOAK:

            // Make sure the ship is a cloaker
            if (iShipType != CLOAKER) {
                iErrCode = ERROR_WRONG_SHIP_TYPE;
                goto Cleanup;
            }
            
            // Make sure it's cloaked
            if (!(iShipState & CLOAKED)) {
                iErrCode = ERROR_CANNOT_UNCLOAK;
                goto Cleanup;
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
                if (iShipType != TERRAFORMER) {
                    iErrCode = ERROR_WRONG_SHIP_TYPE;
                    goto Cleanup;
                }

                if (iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {

                    if (iPlanetOwner != iEmpireKey) {
                        break;
                    }
                }

                Variant vAg, vMin, vFuel;

                // Make sure that there's something to terraform
                iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Ag, &vAg);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Fuel, &vFuel);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Minerals, &vMin);
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
            {

            if (iNewShipOrder == INVADE && (iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL)) {
                iErrCode = ERROR_CANNOT_INVADE;
                goto Cleanup;
            }

            // Make sure the ship is a troopship
            if (iShipType != TROOPSHIP) {
                iErrCode = ERROR_WRONG_SHIP_TYPE;
                goto Cleanup;
            }
            
            // Make sure the planet belongs to someone else or is independent
            if (iPlanetOwner == SYSTEM || iPlanetOwner == iEmpireKey) {
                iErrCode = ERROR_CANNOT_INVADE;
                goto Cleanup;
            }
            
            if (iPlanetOwner != INDEPENDENT) {
                
                GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
                
                // Make sure the diplomatic status is war
                Variant vDipStatus;
                unsigned int iKey;
                iErrCode = m_pGameData->GetFirstKey (
                    strDiplomacy, 
                    GameEmpireDiplomacy::EmpireKey, 
                    iPlanetOwner, 
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
                Variant vHW;

                // Make sure the ship is a doomsday
                if (iShipType != DOOMSDAY) {
                    iErrCode = ERROR_WRONG_SHIP_TYPE;
                    goto Cleanup;
                }
                
                // Make sure the planet can be annihilated
                if (iPlanetOwner != SYSTEM && iPlanetOwner != INDEPENDENT) {
                    
                    if (iPlanetOwner == iEmpireKey) {

                        if (iGameClassOptions & DISABLE_SUICIDAL_DOOMSDAYS) {
                            iErrCode = ERROR_CANNOT_ANNIHILATE;
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::HomeWorld, &vHW);
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
                            iPlanetOwner, 
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

                            if (!(iGameClassOptions & USE_UNFRIENDLY_DOOMSDAYS)) {
                                iErrCode = ERROR_CANNOT_ANNIHILATE;
                                goto Cleanup;
                            }

                            iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::HomeWorld, &vHW);
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
            if (iShipType != MINEFIELD) {
                iErrCode = ERROR_WRONG_SHIP_TYPE;
                goto Cleanup;
            }

            bDismantle = true;
            break;
            
        case OPEN_LINK_NORTH:
        case OPEN_LINK_EAST:
        case OPEN_LINK_SOUTH:
        case OPEN_LINK_WEST:
            {
                // Make sure ship is an engineer
                if (iShipType != ENGINEER) {
                    iErrCode = ERROR_WRONG_SHIP_TYPE;
                    goto Cleanup;
                }

                int iDirection = OPEN_LINK_NORTH - iNewShipOrder;

                Assert (iDirection >= NORTH && iDirection <= WEST);
                
                // Make sure the link doesn't exist
                Variant vLink;
                iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Link, &vLink);
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
                    iShipPlanet, 
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
                if (iShipType != ENGINEER) {
                    iErrCode = ERROR_WRONG_SHIP_TYPE;
                    goto Cleanup;
                }

                int iDirection = CLOSE_LINK_NORTH - iNewShipOrder;
                Assert (iDirection >= NORTH && iDirection <= WEST);
                
                // Make sure the link doesn't exist
                Variant vLink;
                iErrCode = m_pGameData->ReadData (strGameMap, iShipPlanet, GameMap::Link, &vLink);
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
                if (iShipType != BUILDER) {
                    iErrCode = ERROR_WRONG_SHIP_TYPE;
                    goto Cleanup;
                }

                int iDirection = CREATE_PLANET_NORTH - iNewShipOrder;
                Assert (iDirection >= NORTH && iDirection <= WEST);

                // Make sure planet doesn't exist
                Variant vPlanetKey;
                iErrCode = m_pGameData->ReadData (
                    strGameMap, 
                    iShipPlanet, 
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
                int iMorphTech = MORPH_TECH (iNewShipOrder);

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
                if (iShipType != STARGATE && iShipType != JUMPGATE) {
                    iErrCode = ERROR_WRONG_SHIP_TYPE;
                    goto Cleanup;
                }
                
                // Make sure planet isn't the current planet the stargate is on
                if (iShipPlanet == iNewShipOrder) {
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
                
                if (iShipType == STARGATE) {

                    // Make sure we own the planet
                    iErrCode = m_pGameData->ReadData (strGameMap, iNewShipOrder, GameMap::Owner, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    if ((unsigned int) vTemp.GetInteger() != iEmpireKey) {
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
        }   // End switch
                    
        // (If the ship diverges from its fleet, it'll be caught later in the update algorithm)

    }   // End if join fleet

    bOldDismantle = 
        iOldOrder == DISMANTLE ||
        iOldOrder == TERRAFORM_AND_DISMANTLE ||
        iOldOrder == INVADE_AND_DISMANTLE ||
        iOldOrder == COLONIZE ||
        iOldOrder == DETONATE;

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
            - GetMaintenanceCost (iShipType, vMaxBR)
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->Increment (
            strEmpireData, 
            GameEmpireData::NextFuelUse, 
            - GetFuelCost (iShipType, vMaxBR)
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
            GetMaintenanceCost (iShipType, vMaxBR)
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->Increment (
            strEmpireData, 
            GameEmpireData::NextFuelUse, 
            GetFuelCost (iShipType, vMaxBR.GetFloat())
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
    
    return iErrCode;
}

int GameEngine::GetNumShips (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumShips) {

    GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->GetNumRows (pszShips, (unsigned int*) piNumShips);
}

int GameEngine::GetNumFleets (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumFleets) {

    GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, iEmpireKey);

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

int GameEngine::GetColonyOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                 unsigned int iPlanetKey, bool* pbColonize, bool* pbSettle) {

    int iErrCode, iPop, iAnnihilated;
    unsigned int iOwner;

    IReadTable* pMap = NULL;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    *pbColonize = *pbSettle = false;

    iErrCode = m_pGameData->GetTableForReading (strGameMap, &pMap);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pMap->ReadData (iPlanetKey, GameMap::Owner, (int*) &iOwner);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pMap->ReadData (iPlanetKey, GameMap::Pop, &iPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pMap->ReadData (iPlanetKey, GameMap::Annihilated, &iAnnihilated);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // We can colonize a planet if the pop is zero, we don't own it and it hasn't been annihilated
    if (iPop == 0 && iOwner != iEmpireKey && iAnnihilated == NOT_ANNIHILATED) {
        *pbColonize = true;
    }
    
    else if (iOwner == iEmpireKey) {

        // Maybe we can deposit pop on our planet
        int iMaxPop;
        iErrCode = pMap->ReadData (iPlanetKey, GameMap::MaxPop, &iMaxPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iMaxPop > iPop) {
            *pbSettle = true;
        }
    }

Cleanup:

    SafeRelease (pMap);

    return iErrCode;
}

int GameEngine::GetTerraformerOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                      unsigned int iPlanetKey, const GameConfiguration& gcConfig,
                                      bool* pbTerraform, bool* pbTerraformAndDismantle) {

    int iErrCode;
    Variant vAg, vMin, vFuel;
    
    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    
    *pbTerraform = *pbTerraformAndDismantle = false;

    if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {

        Variant vOwner;

        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (vOwner.GetInteger() != (int) iEmpireKey) {
            return OK;
        }
    }

    // All planets can be terraformed, if their statistics allow it 
    // (even enemy-owned or uncolonized ones)

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
            *pbTerraform = true;
        }

        *pbTerraformAndDismantle = true;
    }

Cleanup:
    
    return iErrCode;
}

int GameEngine::GetTroopshipOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                    unsigned int iPlanetKey, const GameConfiguration& gcConfig,
                                    bool* pbInvade, bool* pbInvadeAndDismantle) {
                                    
    int iErrCode;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    *pbInvade = *pbInvadeAndDismantle = false;

    // Only enemy planets can be invaded
    Variant vOwner, vDipStatus;
    unsigned int iKey;

    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vOwner.GetInteger() != (int) iEmpireKey && vOwner.GetInteger() != SYSTEM) {

        if (vOwner.GetInteger() == INDEPENDENT) {
            vDipStatus = WAR;
        } else {

            GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

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

                *pbInvade = true;
            }

            *pbInvadeAndDismantle = true;
        }
    }

Cleanup:

    return iErrCode;
}

int GameEngine::GetDoomsdayOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                   unsigned int iPlanetKey, const GameConfiguration& gcConfig,
                                   int iGameClassOptions, bool* pbAnnihilate) {
    int iErrCode;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    *pbAnnihilate = false;

    // Get owner
    Variant vOwner, vHW;

    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vOwner.GetInteger() == (int) iEmpireKey) {

        if (iGameClassOptions & DISABLE_SUICIDAL_DOOMSDAYS) {
            return OK;
        }

        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::HomeWorld, &vHW);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (vHW.GetInteger() == HOMEWORLD) {
            return OK;
        }

    } else { 

        if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {

            // Get dip status with owner
            unsigned int iKey;
            Variant vDipStatus;

            GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

            iErrCode = m_pGameData->GetFirstKey (
                strEmpireDip,
                GameEmpireDiplomacy::EmpireKey,
                vOwner,
                false,
                &iKey
                );

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

                if (!(iGameClassOptions & USE_UNFRIENDLY_DOOMSDAYS)) {
                    return OK;
                }

                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::HomeWorld, &vHW);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (vHW.GetInteger() == HOMEWORLD) {
                    return OK;
                }
            }
        }
    }

    *pbAnnihilate = true;

Cleanup:

    return iErrCode;
}

int GameEngine::MoveShip (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
                          unsigned int iShipKey, unsigned int iPlanetKey, unsigned int iFleetKey) {
    
    int iErrCode, iBuiltThisUpdate, iType;
    bool bFlag;
    IReadTable* pShips = NULL;

    GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);

    // You want either a planet, or a fleet, or both
    Assert (iPlanetKey != NO_KEY || iFleetKey != NO_KEY);

    iErrCode = m_pGameData->GetTableForReading (pszShips, &pShips);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pShips->ReadData (iShipKey, GameEmpireShips::BuiltThisUpdate, &iBuiltThisUpdate);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get type
    iErrCode = pShips->ReadData (iShipKey, GameEmpireShips::Type, &iType);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Check for immobile ships being moved into fleets
    if (iFleetKey != NO_KEY) {

        if (!IsMobileShip (iType)) {
            iErrCode = ERROR_SHIP_CANNOT_JOIN_FLEET;
            goto Cleanup;
        }
    }

    if (iPlanetKey != NO_KEY) {

        Variant vName;
        float fBR;
        int iBuilt;

        // We're moving planets, so we must be a build ship
        // The checks in BuildNewShips will cover any other problems we might have
        if (iBuiltThisUpdate == 0) {
            iErrCode = ERROR_SHIP_ALREADY_BUILT;
            goto Cleanup;
        }

        // Get name
        iErrCode = pShips->ReadData (iShipKey, GameEmpireShips::Name, &vName);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Get BR
        iErrCode = pShips->ReadData (iShipKey, GameEmpireShips::MaxBR, &fBR);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pShips);

        // If the current ship is a colony, check the destination planet's population
        if (iType == COLONY) {

            unsigned int iPop;
            iErrCode = GetPlanetPopulationWithColonyBuilds (
                iGameClass,
                iGameNumber,
                iEmpireKey,
                iPlanetKey,
                &iPop
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            GameConfiguration gcConfig;
            iErrCode = GetGameConfiguration (&gcConfig);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            unsigned int iCost = GetColonyPopulationBuildCost (
                gcConfig.iShipBehavior, 
                gcConfig.fColonyMultipliedBuildFactor, 
                gcConfig.iColonySimpleBuildFactor, 
                fBR
                );

            if (iPop < iCost) {
                iErrCode = ERROR_INSUFFICIENT_POPULATION_FOR_COLONIES;
                goto Cleanup;
            }
        }

        // Delete the old ship first, because of a possible build limit
        iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, iShipKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = BuildNewShips (
            iGameClass,
            iGameNumber,
            iEmpireKey,
            iType,
            1,
            vName.GetCharPtr(),
            fBR,
            iPlanetKey,
            iFleetKey,
            &iBuilt,
            &bFlag
            );

        if (iErrCode != OK) {
            // TODO - maybe try to recreate the old ship...?
            Assert (false);
            goto Cleanup;
        }
    }
    
    else if (iFleetKey != NO_KEY) {

        SafeRelease (pShips);

        // This is actually easy - all we do is submit an order to switch to the new fleet
        ShipOrder soOrder;
        soOrder.iKey = iFleetKey;
        soOrder.pszText = NULL;
        soOrder.sotType = SHIP_ORDER_NORMAL;

        iErrCode = UpdateShipOrders (iGameClass, iGameNumber, iEmpireKey, iShipKey, soOrder);
    }

Cleanup:

    SafeRelease (pShips);

    return iErrCode;
}


int GameEngine::GetUnaffiliatedMobileShipsAtPlanet (unsigned int iGameClass, unsigned int iGameNumber,
                                                    unsigned int iEmpireKey, unsigned int iPlanetKey,
                                                    unsigned int** ppiShipKey, unsigned int* piNumShips) {
    int iErrCode;

    IReadTable* pIReadShips = NULL;
    unsigned int* piShipKey = NULL, iNumHits = 0, iStopKey;

    GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);

    SearchColumn sc[2];
    sc[0].iColumn = GameEmpireShips::CurrentPlanet;
    sc[0].iFlags = 0;
    sc[0].vData = iPlanetKey;
    sc[0].vData2 = iPlanetKey;

    sc[1].iColumn = GameEmpireShips::FleetKey;
    sc[1].iFlags = 0;
    sc[1].vData = NO_KEY;
    sc[1].vData2 = NO_KEY;

    SearchDefinition sd;
    sd.iMaxNumHits = 0;
    sd.iSkipHits = 0;
    sd.iStartKey = NO_KEY;
    sd.iNumColumns = countof (sc);
    sd.pscColumns = sc;

    if (ppiShipKey != NULL) {
        *ppiShipKey = NULL;
    }
    *piNumShips = 0;

    iErrCode = m_pGameData->GetTableForReading (pszShips, &pIReadShips);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pIReadShips->GetSearchKeys (
        sd,
        &piShipKey,
        &iNumHits,
        &iStopKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        Assert (iNumHits == 0);
        iErrCode = OK;
    } else {

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        for (unsigned int i = 0; i < iNumHits; i ++) {

            int iType;
            iErrCode = pIReadShips->ReadData (piShipKey[i], GameEmpireShips::Type, &iType);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (!IsMobileShip (iType)) {

                iNumHits --;
                piShipKey[i] = piShipKey [iNumHits];
                i --;
            }
        }

        if (iNumHits > 0 && ppiShipKey != NULL) {
            *ppiShipKey = piShipKey;
            piShipKey = NULL;
        }
    }

    *piNumShips = iNumHits;

Cleanup:

    SafeRelease (pIReadShips);

    if (piShipKey != NULL) {
        m_pGameData->FreeKeys (piShipKey);
    }

    return iErrCode;
}

int GameEngine::HasUnaffiliatedMobileShipsAtPlanet (unsigned int iGameClass, unsigned int iGameNumber,
                                                    unsigned int iEmpireKey, unsigned int iPlanetKey,
                                                    bool* pbFlag) {
    int iErrCode;
    IReadTable* pIReadShips = NULL;

    unsigned int* piShipKey = NULL, iNumHits = 0, iStopKey;

    GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);

    SearchColumn sc[2];

    sc[0].iColumn = GameEmpireShips::FleetKey;
    sc[0].iFlags = 0;
    sc[0].vData = NO_KEY;
    sc[0].vData2 = NO_KEY;

    sc[1].iColumn = GameEmpireShips::CurrentPlanet;
    sc[1].iFlags = 0;
    sc[1].vData = iPlanetKey;
    sc[1].vData2 = iPlanetKey;

    SearchDefinition sd;
    sd.iMaxNumHits = 0;
    sd.iSkipHits = 0;
    sd.iStartKey = NO_KEY;
    sd.iNumColumns = countof (sc);
    sd.pscColumns = sc;

    *pbFlag = false;

    iErrCode = m_pGameData->GetTableForReading (pszShips, &pIReadShips);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pIReadShips->GetSearchKeys (
        sd,
        &piShipKey,
        &iNumHits,
        &iStopKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        Assert (iNumHits == 0);
        iErrCode = OK;
    } else {

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        for (unsigned int i = 0; i < iNumHits; i ++) {

            int iType;
            iErrCode = pIReadShips->ReadData (piShipKey[i], GameEmpireShips::Type, &iType);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (IsMobileShip (iType)) {
               *pbFlag = true;
               break;
            }
        }
    }

Cleanup:

    SafeRelease (pIReadShips);

    if (piShipKey != NULL) {
        m_pGameData->FreeKeys (piShipKey);
    }

    return iErrCode;
}