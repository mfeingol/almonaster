//
// Almonaster.dll:  a component of Almonaster
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

#include "GameEngine.h"

// Delete a ship from a game
int GameEngine::DeleteShip (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey)
{
    int iErrCode;
    unsigned int iFleetKey;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    Variant vCancelBuild, vState, vTechKey, vMaxBR;

    // If independent, just delete ship from table
    if (iEmpireKey == INDEPENDENT)
    {
        GET_GAME_EMPIRE_SHIPS(strIndependentShips, iGameClass, iGameNumber, INDEPENDENT);

        // Get ship's locaton
        Variant vPlanetKey;
        iErrCode = t_pCache->ReadData(strIndependentShips, iShipKey, GameEmpireShips::CurrentPlanet, &vPlanetKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->Increment(strGameMap, vPlanetKey, GameMap::NumUncloakedShips, -1);
        RETURN_ON_ERROR(iErrCode);

#ifdef _DEBUG
        Variant vTemp;
        iErrCode = t_pCache->ReadData(strGameMap, vPlanetKey, GameMap::NumUncloakedShips, &vTemp);
        Assert(iErrCode == OK && vTemp >= 0);
#endif
        iErrCode = t_pCache->DeleteRow(strIndependentShips, iShipKey);
        RETURN_ON_ERROR(iErrCode);

        return OK;
    }

    GET_GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    // Was ship just built?
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vCancelBuild);
    RETURN_ON_ERROR(iErrCode);

    // Was ship in a fleet?
    Variant vTemp;
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iFleetKey = vTemp.GetInteger();

    // Was ship cloaked?
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::State, &vState);
    RETURN_ON_ERROR(iErrCode);

    // Get ship's type
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::Type, &vTechKey);
    RETURN_ON_ERROR(iErrCode);

    // Get ship's location
    Variant vPlanetKey;
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::CurrentPlanet, &vPlanetKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
    RETURN_ON_ERROR(iErrCode);

    // If ship is in a fleet, remove it
    if (iFleetKey != NO_KEY) {
        
        GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

        // Get ship's max mil
        float fMaxMil = vMaxBR.GetFloat() * vMaxBR.GetFloat();
        
        // Get ship's current mil
        Variant vCurMil;
        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vCurMil);
        RETURN_ON_ERROR(iErrCode);

        float fCurMil = vCurMil.GetFloat() * vCurMil.GetFloat();
        
        // Reduce fleet's current strength
        iErrCode = t_pCache->Increment(
            strEmpireFleets, 
            iFleetKey, 
            GameEmpireFleets::CurrentStrength, 
            - fCurMil
            );
        RETURN_ON_ERROR(iErrCode);

        // Reduce fleet's max strength
        iErrCode = t_pCache->Increment(
            strEmpireFleets, 
            iFleetKey, 
            GameEmpireFleets::MaxStrength, 
            - fMaxMil
            );
        RETURN_ON_ERROR(iErrCode);
    }

    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    // Increase pop at planet if ship is colony and is a cancel-build
    if (vTechKey.GetInteger() == COLONY && vCancelBuild.GetInteger() != 0) {

        Variant vOldPopLostToColonies, vMin, vFuel, vPop, vMaxPop, vPopLostToColonies, vTotalAg, vTotalPop, vMaxAgRatio, vCost;

        int iPlanetKey = vPlanetKey.GetInteger();

        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::ColonyBuildCost, &vCost);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Pop, &vPop);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->Increment(
            strGameMap,
            iPlanetKey,
            GameMap::PopLostToColonies,
            - vCost.GetInteger(),
            &vPopLostToColonies
            );

        RETURN_ON_ERROR(iErrCode);

        Assert(vCost.GetInteger() >= 0);
        Assert(vPopLostToColonies.GetInteger() >= 0);
        Assert(vPopLostToColonies.GetInteger() - vCost.GetInteger() >= 0);

        iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalAg, &vTotalAg);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalPop, &vTotalPop);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxAgRatio, 
            &vMaxAgRatio
            );
        RETURN_ON_ERROR(iErrCode);

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
        iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextTotalPop, iNextPopDiff, &vOldNextPop);
        RETURN_ON_ERROR(iErrCode);

        // Get planet data
        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
        RETURN_ON_ERROR(iErrCode);
        
        iDiff = min (vOldNextPop.GetInteger() + iNextPopDiff, vMin.GetInteger()) - 
                min (vOldNextPop.GetInteger(), vMin.GetInteger());
        
        if (iDiff != 0) {
            iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextMin, iDiff);
            RETURN_ON_ERROR(iErrCode);
        }
        
        iDiff = min (vOldNextPop.GetInteger() + iNextPopDiff, vFuel.GetInteger()) - 
                min (vOldNextPop.GetInteger(), vFuel.GetInteger());
        
        if (iDiff != 0) {
            iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextFuel, iDiff);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Decrease empire's resource usage
    unsigned int iEmpireMapKey;
    iErrCode = t_pCache->GetFirstKey(strEmpireMap, GameEmpireMap::PlanetKey, vPlanetKey, &iEmpireMapKey);
    RETURN_ON_ERROR(iErrCode);

    if (vCancelBuild.GetInteger() != 0) {
        
        // Decrease number of builds
        iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NumBuilds, -1);
        RETURN_ON_ERROR(iErrCode);

        // Decrease total build
        int iBuildCost = GetBuildCost (vTechKey.GetInteger(), vMaxBR.GetFloat());
        
        iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::TotalBuild, - iBuildCost);
        RETURN_ON_ERROR(iErrCode);

        if (vState.GetInteger() & CLOAKED) {
            
            iErrCode = t_pCache->Increment(strEmpireMap, iEmpireMapKey, GameEmpireMap::NumCloakedBuildShips, -1);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->Increment(strGameMap, vPlanetKey, GameMap::NumCloakedBuildShips, -1);
            RETURN_ON_ERROR(iErrCode);
            
        } else {
            
            iErrCode = t_pCache->Increment(strEmpireMap, iEmpireMapKey, GameEmpireMap::NumUncloakedBuildShips, -1);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->Increment(strGameMap, vPlanetKey, GameMap::NumUncloakedBuildShips, -1);
            RETURN_ON_ERROR(iErrCode);
        }

        iErrCode = t_pCache->Increment(
            strEmpireData, 
            GameEmpireData::NextMaintenance, 
            - GetMaintenanceCost (vTechKey.GetInteger(), vMaxBR.GetFloat())
            );
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->Increment(
            strEmpireData, 
            GameEmpireData::NextFuelUse, 
            - GetFuelCost (vTechKey.GetInteger(), vMaxBR.GetFloat())
            );
        RETURN_ON_ERROR(iErrCode);

    } else {
        
        int iMaintCost = GetMaintenanceCost (vTechKey.GetInteger(), vMaxBR.GetFloat());
        int iFuelCost = GetFuelCost (vTechKey.GetInteger(), vMaxBR.GetFloat());

        iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::TotalMaintenance, - iMaintCost);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::TotalFuelUse, - iFuelCost);
        RETURN_ON_ERROR(iErrCode);
        
#ifdef _DEBUG
        Variant vTotalFuelUse;
        iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalFuelUse, &vTotalFuelUse);
        Assert(vTotalFuelUse.GetInteger() >= 0);
#endif
        if (vState.GetInteger() & CLOAKED) {
            
            iErrCode = t_pCache->Increment(strEmpireMap, iEmpireMapKey, GameEmpireMap::NumCloakedShips, -1);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->Increment(strGameMap, vPlanetKey, GameMap::NumCloakedShips, -1);
            RETURN_ON_ERROR(iErrCode);
            
        } else {
            
            iErrCode = t_pCache->Increment(strEmpireMap, iEmpireMapKey, GameEmpireMap::NumUncloakedShips, -1);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->Increment(strGameMap, vPlanetKey, GameMap::NumUncloakedShips, -1);
            RETURN_ON_ERROR(iErrCode);
        }
    }
    
    // Delete ship from table
    iErrCode = t_pCache->DeleteRow(strEmpireShips, iShipKey);
    RETURN_ON_ERROR(iErrCode);

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
        iNextTotalPop, iErrCode, iNumTrades, iNumAlliances, iPercentFirstTradeIncrease, 
        iPercentNextTradeIncrease, iGameClassOptions;

    float fTechLevel, fMaxAgRatio, fMaxTechDev;

    Variant vTemp;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
    RETURN_ON_ERROR(iErrCode);

    if (iGameClassOptions & VISIBLE_DIPLOMACY) {
        iErrCode = GetNumEmpiresAtDiplomaticStatusNextUpdate (iGameClass, iGameNumber, iEmpireKey, NULL, NULL, &iNumTrades, &iNumAlliances);
        RETURN_ON_ERROR(iErrCode);
    } else {
        iErrCode = GetNumEmpiresAtDiplomaticStatus (iGameClass, iGameNumber, iEmpireKey, NULL, NULL, &iNumTrades, &iNumAlliances);
        RETURN_ON_ERROR(iErrCode);
    }

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    iErrCode = t_pCache->GetTable(strGameEmpireData, &pTable);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::TotalBuild, &iBuild);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::TotalMaintenance, &iMaint);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::TotalFuelUse, &iFuelUse);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::TotalAg, &iAg);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::TotalFuel, &iFuel);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::TotalMin, &iMin);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::TechLevel, &fTechLevel);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::BonusAg, &iBonusAg);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::BonusFuel, &iBonusFuel);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::BonusMin, &iBonusMin);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::TotalPop, &iTotalPop);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::NextMaintenance, &iNextMaint);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::NextFuelUse, &iNextFuelUse);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::NextMin, &iNextMinAdjustment);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::NextFuel, &iNextFuelAdjustment);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(GameEmpireData::NextTotalPop, &iNextTotalPop);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxTechDev, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    fMaxTechDev = vTemp.GetFloat();

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    fMaxAgRatio = vTemp.GetFloat();

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::PercentFirstTradeIncrease, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iPercentFirstTradeIncrease = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::PercentNextTradeIncrease, &vTemp);
    RETURN_ON_ERROR(iErrCode);
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
        iNumTrades + iNumAlliances, iNextAg, iNextMin, iNextFuel, iPercentFirstTradeIncrease, 
        iPercentNextTradeIncrease, &iNextBonusAg, &iNextBonusMin, &iNextBonusFuel
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

int GameEngine::GetShipOrders(unsigned int iGameClass, unsigned int iGameNumber, unsigned int iEmpireKey, 
                              unsigned int iShipKey, const ShipOrderShipInfo* pShipInfo, 
                              const ShipOrderGameInfo* pGameInfo, const ShipOrderPlanetInfo* pPlanetInfo, 
                              const GameConfiguration& gcConfig,
                              const BuildLocation* pblLocations, unsigned int iNumLocations,
                              ShipOrder** ppsoOrder, unsigned int* piNumOrders, int* piSelectedOrder) {

    int iErrCode = OK;

    GET_GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    bool pbLink [NUM_CARDINAL_POINTS];
    bool pbMustExplore [NUM_CARDINAL_POINTS];

    char strEmpireMap [256], strEmpireDip [256];
    char pszOrder [MAX_PLANET_NAME_LENGTH + MAX_FLEET_NAME_LENGTH + 256];

    String strPlanetNameString, strNewPlanetName;
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
    AutoFreeShipOrders freeOrders(psoOrders, iNumOrders);

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

        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        fBR = vTemp.GetFloat();

        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        fMaxBR = vTemp.GetFloat();

        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::Type, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iShipType = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::Action, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iSelectedAction = vTemp.GetInteger();

        // Get ship state
        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::State, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iState = vTemp.GetInteger();

        // Check for newly built ship
        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        bBuilding = vTemp.GetInteger() != 0;
    }

    if (pGameInfo != NULL) {

        fMaintRatio = pGameInfo->fMaintRatio;
        iGameClassOptions = pGameInfo->iGameClassOptions;
    
    } else {

        iErrCode = GetEmpireMaintenanceRatio (iGameClass, iGameNumber, iEmpireKey, &fMaintRatio);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
        RETURN_ON_ERROR(iErrCode);
    }

    if (pPlanetInfo != NULL) {

        iPlanetKey = pPlanetInfo->iPlanetKey;
        iPlanetOwner = pPlanetInfo->iOwner;
        pszPlanetName = pPlanetInfo->pszName;
        iLocationX = pPlanetInfo->iX;
        iLocationY = pPlanetInfo->iY;
    
    } else {

        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::CurrentPlanet, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iPlanetKey = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iPlanetOwner = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        Assert(String::AtoHtml (vTemp.GetCharPtr(), &strPlanetNameString, 0, false));
        pszPlanetName = strPlanetNameString.GetCharPtr();

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vTemp);
        RETURN_ON_ERROR(iErrCode);
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

    if (bBuilding)
    {
        BuildLocation* pblOurLocations = NULL;
        Algorithm::AutoDelete<BuildLocation> autoDel (pblOurLocations, true);

        if (pblLocations == NULL) {

            iErrCode = GetBuildLocations (
                iGameClass,
                iGameNumber,
                iEmpireKey,
                NO_KEY,
                &pblOurLocations,
                &iNumLocations
                );

            RETURN_ON_ERROR(iErrCode);

            pblLocations = pblOurLocations;
        }

        // Allocate orders
        iMaxNumOrders = iNumLocations + 1;

        psoOrders = new ShipOrder[iMaxNumOrders];
        Assert(psoOrders);
        memset (psoOrders, 0, iMaxNumOrders * sizeof (ShipOrder));
        Assert(SHIP_ORDER_NORMAL == 0);

        // First, add only the locations at our current planet
        for (i = 0; i < iNumLocations; i ++)
        {
            // Optimization: orders are always collected by planet
            if (iNumOrders > 0 && pblLocations[i].iPlanetKey != iPlanetKey) {
                break;
            }

            if (pblLocations[i].iPlanetKey != iPlanetKey ||
                pblLocations[i].iFleetKey == FLEET_NEWFLEETKEY) {
                continue;
            }

            if (pblLocations[i].iFleetKey == NO_KEY) {
                
                sprintf(pszOrder, "Build at %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);
                psoOrders[iNumOrders].iKey = BUILD_AT;
            
            } else if (bMobile) {
                
                // Get fleet name
                iErrCode = t_pCache->ReadData(
                    strEmpireFleets, pblLocations[i].iFleetKey, GameEmpireFleets::Name, &vTemp
                    );
                RETURN_ON_ERROR(iErrCode);

                // Filter fleet name
                String strFleetName;
                Assert(String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false));

                sprintf(pszOrder, "Build at %s (%i,%i) in fleet %s", 
                    pszPlanetName, iLocationX, iLocationY, strFleetName.GetCharPtr());

                psoOrders[iNumOrders].iKey = pblLocations[i].iFleetKey;

            } else {

                // Immobile ships don't build into fleets
                continue;
            }

            psoOrders[iNumOrders].sotType = SHIP_ORDER_NORMAL;
            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
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

                    RETURN_ON_ERROR(iErrCode);

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
                RETURN_ON_ERROR(iErrCode);

                Assert(String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false));

                iErrCode = GetPlanetCoordinates (iGameClass, iGameNumber, pblLocations[i].iPlanetKey, &iX, &iY);
                RETURN_ON_ERROR(iErrCode);

                iCachedPlanetKey = pblLocations[i].iPlanetKey;
            }

            sprintf(pszOrder, "Build at %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

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
                RETURN_ON_ERROR(iErrCode);

                String strFleetName;
                Assert(String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false));

                strcat (pszOrder, " in fleet ");
                strcat (pszOrder, strFleetName.GetCharPtr());
            
            } else {

                // Immobile ships don't build into fleets
                continue;
            }

            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            iNumOrders ++;
        }

        // Add cancel build order
        psoOrders[iNumOrders].sotType = SHIP_ORDER_NORMAL;
        psoOrders[iNumOrders].iKey = CANCEL_BUILD;
        psoOrders[iNumOrders].pszText = String::StrDup ("Cancel Build");
        iNumOrders ++;
    }
    else
    {   
        /////////////////////////////
        // Non-building ship orders//
        /////////////////////////////

        COPY_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

        unsigned int cMaxNumFleets, cMaxNumPlanets;

        // Calculate the max number of orders we'll ever have
        iErrCode = t_pCache->GetNumCachedRows(strEmpireFleets, &cMaxNumFleets);
        RETURN_ON_ERROR(iErrCode);
    
        iErrCode = t_pCache->GetNumCachedRows(strEmpireMap, &cMaxNumPlanets);
        RETURN_ON_ERROR(iErrCode);

        iMaxNumOrders = 8 + cMaxNumFleets + max(cMaxNumPlanets, NUM_CARDINAL_POINTS) + NUM_SHIP_TYPES;

        psoOrders = new ShipOrder [iMaxNumOrders];
        Assert(psoOrders);
        memset (psoOrders, 0, iMaxNumOrders * sizeof (ShipOrder));
        Assert(SHIP_ORDER_NORMAL == 0);

        ///////////////////
        // Add "Standby" //
        ///////////////////

        sprintf(pszOrder, "Standby at %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

        psoOrders[iNumOrders].iKey = STAND_BY;
        psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
        iNumOrders ++;

        ///////////////////////
        // Add mobile orders //
        ///////////////////////
    
        // Get proxy key of planet from EmpireMap
        iErrCode = t_pCache->GetFirstKey(strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, &iProxyPlanetKey);
        RETURN_ON_ERROR(iErrCode);

        COPY_GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

        // Select only mobile ships
        if (bMobile) {

            //////////
            // Move //
            //////////

            int iLink, iExplored;
            unsigned int iNewPlanetKey;

            // Read explored
            iErrCode = t_pCache->ReadData(strEmpireMap, iProxyPlanetKey, GameEmpireMap::Explored, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            iExplored = vTemp.GetInteger();
        
            // Loop through all cardinal points
            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Link, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            iLink = vTemp.GetInteger();

            ENUMERATE_CARDINAL_POINTS(i) {

                // Is there a link in this direction?
                pbLink[i] = ((iLink & LINK_X[i]) != 0);
                if (pbLink[i]) {

                    pbMustExplore[i] = !(iExplored & EXPLORED_X[i]);
                    if (!pbMustExplore[i]) {
                
                        // Read planet key
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::ColumnNames[GameMap::iNorthPlanetKey + i], &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                        iNewPlanetKey = vTemp.GetInteger();
                        Assert(iNewPlanetKey != NO_KEY);

                        // Get name
                        iErrCode = t_pCache->ReadData(strGameMap, iNewPlanetKey, GameMap::Name, &vTemp);
                        RETURN_ON_ERROR(iErrCode);

                        Assert(String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false));
                    
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
                        iErrCode = t_pCache->GetFirstKey(strEmpireDip, GameEmpireDiplomacy::ReferenceEmpireKey, iPlanetOwner, &iKey);
                        if (iErrCode == ERROR_DATA_NOT_FOUND) {
                            iErrCode = OK;
                        } else {
                        
                            RETURN_ON_ERROR(iErrCode);
                        
                            iErrCode = t_pCache->ReadData(strEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vTemp);
                            RETURN_ON_ERROR(iErrCode);
                            iDipStatus = vTemp.GetInteger();
                        }
                    }
                    
                    if (iDipStatus == WAR) {
                    
                        // Well, we're at war, so if the ship isn't cloaked we can nuke
                        if (!(iState & CLOAKED)) {

                            sprintf(pszOrder, "Nuke %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

                            psoOrders[iNumOrders].iKey = NUKE;
                            psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
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
            
                sprintf(pszOrder, "Explore %s to Planet %i,%i", CARDINAL_STRING[i], iNewX, iNewY);

                psoOrders[iNumOrders].iKey = EXPLORE_NORTH - i;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                iNumOrders ++;
            }
        
            break;

        case COLONY:

            bool bColonize, bSettle;
            iErrCode = GetColonyOrders (iGameClass, iGameNumber, iEmpireKey, iPlanetKey, &bColonize, &bSettle);
            RETURN_ON_ERROR(iErrCode);

            if (bColonize) {

                sprintf(pszOrder, "Colonize %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

                psoOrders[iNumOrders].iKey = COLONIZE;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                iNumOrders ++;
            }

            else if (bSettle) {

                sprintf(pszOrder, "Settle %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

                psoOrders[iNumOrders].iKey = DEPOSIT_POP;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                iNumOrders ++;
            }        
            break;

        case STARGATE:
        
            // Check next BR
            if (fNextBR >= gcConfig.fStargateGateCost) {

                unsigned int iNumPlanets, * piKey = NULL;
                AutoFreeKeys free(piKey);

                int iSrcX = 0, iSrcY = 0, iDestX, iDestY;

                if (gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE) {

                    // Get src planet coords
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vTemp);
                    RETURN_ON_ERROR(iErrCode);
                    GetCoordinates (vTemp.GetCharPtr(), &iSrcX, &iSrcY);
                }

                // Get all owned planets
                iErrCode = t_pCache->GetEqualKeys(
                    strGameMap, 
                    GameMap::Owner, 
                    iEmpireKey, 
                    &piKey, 
                    &iNumPlanets
                    );

                RETURN_ON_ERROR(iErrCode);

                for (i = 0; i < iNumPlanets; i ++) {

                    unsigned int iKey = piKey[i];
                    if (iKey == iPlanetKey) {
                        continue;
                    }

                    // Get coordinates
                    iErrCode = t_pCache->ReadData(strGameMap, iKey, GameMap::Coordinates, &vTemp);
                    RETURN_ON_ERROR(iErrCode);

                    GetCoordinates (vTemp.GetCharPtr(), &iDestX, &iDestY);

                    if (
                    
                        !(gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE) ||

                        fNextBR >= GetGateBRForRange (gcConfig.fStargateRangeFactor, iSrcX, iSrcY, iDestX, iDestY)
                    
                        ) {

                        // Get name
                        iErrCode = t_pCache->ReadData(strGameMap, iKey, GameMap::Name, &vTemp);
                        RETURN_ON_ERROR(iErrCode);

                        Assert(String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false));
                    
                        // Add send to Order
                        sprintf(pszOrder, "Stargate ships to %s (%i,%i)", 
                            strNewPlanetName.GetCharPtr(), iDestX, iDestY);
                    
                        psoOrders[iNumOrders].iKey = iKey;
                        psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                        iNumOrders ++;
                    }
                }
            }

            break;

        case CLOAKER:

            int iActionKey;
            const char* pszCloak;

            if (iState & CLOAKED) {
                iActionKey = UNCLOAK;
                pszCloak = "Uncloak";
            } else {
                iActionKey = CLOAK;
                pszCloak = "Cloak";
            }

            psoOrders[iNumOrders].iKey = iActionKey;
            psoOrders[iNumOrders].pszText = String::StrDup (pszCloak);
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

            RETURN_ON_ERROR(iErrCode);

            if (bTerraform) {
        
                sprintf(pszOrder, "Terraform %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);
            
                psoOrders[iNumOrders].iKey = TERRAFORM;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                iNumOrders ++;
            }

            if (bTerraformAndDismantle) {

                sprintf(pszOrder, "Terraform %s (%i,%i) and dismantle", pszPlanetName, iLocationX, iLocationY);

                psoOrders[iNumOrders].iKey = TERRAFORM_AND_DISMANTLE;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
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

            RETURN_ON_ERROR(iErrCode);

            if (bInvade) {

                sprintf(pszOrder, "Invade %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);

                psoOrders[iNumOrders].iKey = INVADE;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                iNumOrders ++;
            }

            if (bInvadeAndDismantle) {

                sprintf(pszOrder, "Invade %s (%i,%i) and dismantle", pszPlanetName, iLocationX, iLocationY);

                psoOrders[iNumOrders].iKey = INVADE_AND_DISMANTLE;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
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

            RETURN_ON_ERROR(iErrCode);

            if (bAnnihilate) {
            
                sprintf(pszOrder, "Annihilate %s (%i,%i)", pszPlanetName, iLocationX, iLocationY);
            
                psoOrders[iNumOrders].iKey = ANNIHILATE;
                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
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
                iNumOrders ++;
            }
            break;

        case ENGINEER:
        
            // Check next BR
            if (fNextBR >= gcConfig.fEngineerLinkCost) {

                int iExplored;

                iErrCode = t_pCache->ReadData(strEmpireMap, iProxyPlanetKey, GameEmpireMap::Explored, &vTemp);
                RETURN_ON_ERROR(iErrCode);
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

                            iErrCode = t_pCache->ReadData(
                                strGameMap, 
                                iPlanetKey, 
                                GameMap::ColumnNames[GameMap::iNorthPlanetKey + i],
                                &vTemp
                                );
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->ReadData(strGameMap, vTemp.GetInteger(), GameMap::Name, &vTemp);
                            RETURN_ON_ERROR(iErrCode);

                            Assert(String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false));

                            pszNewPlanet = strNewPlanetName.GetCharPtr();
                        }
                    
                    } else {    // if (!pbLink[i])

                        // See if there's a planet at all
                        iErrCode = t_pCache->ReadData(
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::ColumnNames[GameMap::iNorthPlanetKey + i], 
                            &vTemp
                            );
                        RETURN_ON_ERROR(iErrCode);

                        unsigned int iNewPlanetKey = vTemp.GetInteger();
                        if (iNewPlanetKey == NO_KEY){
                            Assert(!(iExplored & EXPLORED_X[i]));
                            continue;
                        }

                        iOrderKey = OPEN_LINK[i];
                        pszOpenClose = "Open";

                        if (!(iExplored & EXPLORED_X[i])) {
                            pszNewPlanet = "Unknown";
                        } else {

                            iErrCode = t_pCache->ReadData(strGameMap, iNewPlanetKey, GameMap::Name, &vTemp);
                            RETURN_ON_ERROR(iErrCode);

                            Assert(String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false));

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
                    iNumOrders ++;

                }   // End cardinal point enumeration
            }

            break;

        case BUILDER:

            // Check next BR
            if (fNextBR >= gcConfig.fBuilderMinBR) {

                ENUMERATE_CARDINAL_POINTS(i) {
                
                    iErrCode = t_pCache->ReadData(
                        strGameMap, 
                        iPlanetKey, 
                        GameMap::ColumnNames[GameMap::iNorthPlanetKey + i], 
                        &vTemp
                        );
                
                    RETURN_ON_ERROR(iErrCode);
                
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
                        iNumOrders ++;
                    }
                }
            }
            break;

        case JUMPGATE:

            // Check next BR
            if (fNextBR >= gcConfig.fJumpgateGateCost) {

                Variant* pvPlanetKey = NULL;
                AutoFreeData free(pvPlanetKey);

                unsigned int iNumPlanets;
                int iSrcX = 0, iSrcY = 0, iDestX, iDestY;

                if (gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE) {

                    // Get src planet coords
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vTemp);
                    RETURN_ON_ERROR(iErrCode);
                    GetCoordinates (vTemp.GetCharPtr(), &iSrcX, &iSrcY);
                }

                // Get all visible planets
                GET_GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

                iErrCode = t_pCache->ReadColumn(
                    strGameEmpireMap, 
                    GameEmpireMap::PlanetKey,
                    NULL,
                    &pvPlanetKey, 
                    &iNumPlanets
                    );

                RETURN_ON_ERROR(iErrCode);

                for (i = 0; i < iNumPlanets; i ++) {

                    unsigned int iThisPlanetKey = pvPlanetKey[i].GetInteger();
                    if (iThisPlanetKey != iPlanetKey) {
                    
                        // Check range
                        iErrCode = t_pCache->ReadData(strGameMap, iThisPlanetKey, GameMap::Coordinates, &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                        GetCoordinates (vTemp.GetCharPtr(), &iDestX, &iDestY);

                        if (
                        
                            !(gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE) ||

                            fNextBR >= GetGateBRForRange (gcConfig.fJumpgateRangeFactor, iSrcX, iSrcY, iDestX, iDestY)
                        
                            ) {
                        
                            // Check annihilated status
                            iErrCode = t_pCache->ReadData(strGameMap, iThisPlanetKey, GameMap::Annihilated, &vTemp);
                            RETURN_ON_ERROR(iErrCode);

                            if (vTemp.GetInteger() == NOT_ANNIHILATED) {

                                // Get name
                                iErrCode = t_pCache->ReadData(strGameMap, iThisPlanetKey, GameMap::Name, &vTemp);
                                RETURN_ON_ERROR(iErrCode);

                                Assert(String::AtoHtml (vTemp.GetCharPtr(), &strNewPlanetName, 0, false));
                            
                                // Add send to Order
                                sprintf(pszOrder, "Jumpgate ships to %s (%i,%i)", strNewPlanetName.GetCharPtr(), iDestX, iDestY);
                            
                                psoOrders[iNumOrders].iKey = iThisPlanetKey;
                                psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                                iNumOrders ++;
                            }
                        }
                    }
                }
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

            GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
        
            // Get developed techs
            iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::TechDevs, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            int iTechs = vTemp.GetInteger();

            ENUMERATE_SHIP_TYPES (i) {

                if (i != (unsigned int) iShipType && (iTechs & TECH_BITS[i])) {

                    sprintf(pszOrder, "Morph into %s", SHIP_TYPE_STRING[i]);
                
                    psoOrders[iNumOrders].iKey = MORPH_ORDER[i];
                    psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
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
            AutoFreeKeys free(piFleetKey);

            iErrCode = t_pCache->GetEqualKeys(
                strEmpireFleets, 
                GameEmpireFleets::CurrentPlanet, 
                iPlanetKey, 
                &piFleetKey, 
                &iNumFleets
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);
            
            if (iNumFleets > 0)
            {
                iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                unsigned int iFleetKey = vTemp.GetInteger();
            
                for (i = 0; i < iNumFleets; i ++) {

                    if (iFleetKey == piFleetKey[i]) {
                    
                        // Remain in own fleet
                        iErrCode = t_pCache->ReadData(
                            strEmpireFleets, 
                            iFleetKey, 
                            GameEmpireFleets::Name, 
                            &vTemp
                            );

                        RETURN_ON_ERROR(iErrCode);

                        // Filter fleet name
                        String strFleetName;
                        Assert(String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false));

                        sprintf(pszOrder, "Remain in fleet %s", strFleetName.GetCharPtr());

                        psoOrders[iNumOrders].iKey = FLEET;
                        psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                        iNumOrders ++;

                        // Leave own fleet
                        sprintf(pszOrder, "Leave fleet %s", strFleetName.GetCharPtr());

                        psoOrders[iNumOrders].iKey = LEAVE_FLEET;
                        psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                        iNumOrders ++;

                    } else {

                        // Join another fleet
                        iErrCode = t_pCache->ReadData(
                            strEmpireFleets, 
                            piFleetKey[i], 
                            GameEmpireFleets::Name, 
                            &vTemp
                            );

                        RETURN_ON_ERROR(iErrCode);

                        // Filter fleet name
                        String strFleetName;
                        Assert(String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false));
                    
                        sprintf(pszOrder, "Join fleet %s", strFleetName.GetCharPtr());

                        psoOrders[iNumOrders].iKey = piFleetKey[i];
                        psoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                        iNumOrders ++;
                    }
                }
            }
        }

        ///////////////////
        // Add dismantle //
        ///////////////////

        psoOrders[iNumOrders].iKey = DISMANTLE;
        psoOrders[iNumOrders].pszText = String::StrDup ("Dismantle");
        iNumOrders ++;
    }

    for (i = 0; i < iNumOrders; i ++)
    {
        Assert(psoOrders[i].pszText);
    }

    *piNumOrders = iNumOrders;
    *piSelectedOrder = iSelectedAction;
    *ppsoOrder = psoOrders;
    psoOrders = NULL;

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

int GameEngine::UpdateShipName(int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, const char* pszNewName)
{
    GET_GAME_EMPIRE_SHIPS(strEmpireShips, iGameClass, iGameNumber, iEmpireKey);

    // Write new name
    int iErrCode = t_pCache->WriteData(strEmpireShips, iShipKey, GameEmpireShips::Name, pszNewName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        iErrCode = ERROR_SHIP_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
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

int GameEngine::UpdateShipOrders (unsigned int iGameClass, unsigned int iGameNumber, unsigned int iEmpireKey, 
                                  unsigned int iShipKey, const ShipOrder& soOrder) {

    GET_GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    
    Variant vTemp;

    // Check ship key
    bool bOldDismantle, bDismantle = false;
    int i, iShipBehavior, iErrCode, iShipState, iOldOrder, iGameClassOptions, iShipType, iShipPlanet;
    int iNewShipOrder = soOrder.iKey;
    unsigned int iOldFleetKey;
    
    // Select by type
    switch (soOrder.sotType) {

    case SHIP_ORDER_MOVE_PLANET:
        iErrCode = MoveShip(iGameClass, iGameNumber, iEmpireKey, iShipKey, soOrder.iKey, NO_KEY);
        if (iErrCode == ERROR_SHIP_DOES_NOT_EXIST)
        {
            return iErrCode;
        }
        RETURN_ON_ERROR(iErrCode);
        return iErrCode;

    case SHIP_ORDER_MOVE_FLEET:
        // We need the location of the destination fleet
        iErrCode = GetFleetProperty(iGameClass, iGameNumber, iEmpireKey, soOrder.iKey, GameEmpireFleets::CurrentPlanet, &vTemp);
        if (iErrCode == ERROR_FLEET_DOES_NOT_EXIST)
        {
            return iErrCode;
        }
        RETURN_ON_ERROR(iErrCode);

        iErrCode = MoveShip(iGameClass, iGameNumber, iEmpireKey, iShipKey, vTemp.GetInteger(), soOrder.iKey);
        if (iErrCode == ERROR_SHIP_DOES_NOT_EXIST)
        {
            return iErrCode;
        }
        RETURN_ON_ERROR(iErrCode);
        return iErrCode;
    }

    Assert(soOrder.sotType == SHIP_ORDER_NORMAL);

    // Check for same order
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::Action, &vTemp);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_SHIP_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    iOldOrder = vTemp.GetInteger();
    if (iOldOrder == iNewShipOrder)
    {
        return OK;
    }

    // Get ship behavior
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::ShipBehavior, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iShipBehavior = vTemp.GetInteger();

    // Get gameclass options
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameClassOptions = vTemp.GetInteger();

    // Get ship state
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iShipState = vTemp.GetInteger();
    
    GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    // Get ship's fleet and location
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::FleetKey, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iOldFleetKey = vTemp.GetInteger();
    
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::CurrentPlanet, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iShipPlanet = vTemp.GetInteger();
    Assert(iShipPlanet != NO_KEY);
    
    // Check for a fleet change for ships built this update
    Variant vBuiltThisUpdate;
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vBuiltThisUpdate);
    RETURN_ON_ERROR(iErrCode);

    if (vBuiltThisUpdate.GetInteger() != 0) {
        
        // Handle a cancel build request
        if (iNewShipOrder == CANCEL_BUILD)
        {
            // Just delete the ship
            iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, iShipKey);
            RETURN_ON_ERROR(iErrCode);
            return iErrCode;
        }

        Variant vCurrentBR;
        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vCurrentBR);
        RETURN_ON_ERROR(iErrCode);

        float fMil = vCurrentBR.GetFloat() * vCurrentBR.GetFloat();
            
        if (iNewShipOrder == BUILD_AT)
        {
            // Set action to build at planet
            iErrCode = t_pCache->WriteData(strEmpireShips, iShipKey, GameEmpireShips::Action, BUILD_AT);
            RETURN_ON_ERROR(iErrCode);
                
            if (iOldFleetKey != NO_KEY)
            {
                // Reduce fleet's strength
                iErrCode = t_pCache->Increment(strEmpireFleets, iOldFleetKey, GameEmpireFleets::CurrentStrength, -fMil);
                RETURN_ON_ERROR(iErrCode);
                    
                iErrCode = t_pCache->Increment(strEmpireFleets, iOldFleetKey, GameEmpireFleets::MaxStrength, -fMil);
                RETURN_ON_ERROR(iErrCode);

                // Set no fleet key
                iErrCode = t_pCache->WriteData(strEmpireShips, iShipKey, GameEmpireShips::FleetKey, (int)NO_KEY);
                RETURN_ON_ERROR(iErrCode);
            }

            return iErrCode;
        }
                
        // We must be building into a fleet...
                
        // Make sure that the fleet is on the same planet as the ship
        Variant vFleetPlanet;
        iErrCode = t_pCache->ReadData(strEmpireFleets, iNewShipOrder, GameEmpireFleets::CurrentPlanet, &vFleetPlanet);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_FLEET_DOES_NOT_EXIST;
        }
        RETURN_ON_ERROR(iErrCode);

        if (iShipPlanet != vFleetPlanet.GetInteger())
        {
            return ERROR_WRONG_PLANET;
        }
                
        if (iOldFleetKey != NO_KEY) {

            // Reduce fleet's strength
            iErrCode = t_pCache->Increment(strEmpireFleets, iOldFleetKey, GameEmpireFleets::CurrentStrength, -fMil);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->Increment(strEmpireFleets, iOldFleetKey, GameEmpireFleets::MaxStrength, -fMil);
            RETURN_ON_ERROR(iErrCode);
        }
                
        // Increase power of new fleet
        iErrCode = t_pCache->Increment(strEmpireFleets, iNewShipOrder, GameEmpireFleets::CurrentStrength, fMil);
        RETURN_ON_ERROR(iErrCode);
                
        iErrCode = t_pCache->Increment(strEmpireFleets, iNewShipOrder, GameEmpireFleets::MaxStrength, fMil);
        RETURN_ON_ERROR(iErrCode);

        // Set action to be "build in fleet"
        iErrCode = t_pCache->WriteData(strEmpireShips, iShipKey, GameEmpireShips::Action, iNewShipOrder);
        RETURN_ON_ERROR(iErrCode);
                
        // Set ship fleet
        iErrCode = t_pCache->WriteData(strEmpireShips, iShipKey, GameEmpireShips::FleetKey, iNewShipOrder);
        RETURN_ON_ERROR(iErrCode);

        // Set the fleet to stand by
        iErrCode = t_pCache->WriteData(strEmpireFleets, iNewShipOrder, GameEmpireFleets::Action, STAND_BY);
        RETURN_ON_ERROR(iErrCode);

        return iErrCode;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////
    // The ship is not being built, so we need to verify the legitimacy of the request //
    /////////////////////////////////////////////////////////////////////////////////////

    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::Type, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iShipType = vTemp.GetInteger();

    // Was a join fleet requested?
    if (IsMobileShip (iShipType) && iNewShipOrder >= 0)
    {
        // Are fleet and ship on the same planet?
        Variant vFleetPlanet;
        iErrCode = t_pCache->ReadData(strEmpireFleets, iNewShipOrder, GameEmpireFleets::CurrentPlanet, &vFleetPlanet);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_SHIP_DOES_NOT_EXIST;
        }
        RETURN_ON_ERROR(iErrCode);

        if (iShipPlanet != vFleetPlanet.GetInteger())
        {
            return ERROR_WRONG_PLANET;
        }
        
        // Set fleet key to new fleet
        iErrCode = t_pCache->WriteData(strEmpireShips, iShipKey, GameEmpireShips::FleetKey, iNewShipOrder);
        RETURN_ON_ERROR(iErrCode);
        
        // Get ship's BR
        Variant vCurrentBR, vMaxBR;
        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vCurrentBR);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
        RETURN_ON_ERROR(iErrCode);
        
        // Add ship's strength to new fleet's strength
        float fCurrentMil = vCurrentBR.GetFloat() * vCurrentBR.GetFloat();
        iErrCode = t_pCache->Increment(strEmpireFleets, iNewShipOrder, GameEmpireFleets::CurrentStrength, fCurrentMil);
        RETURN_ON_ERROR(iErrCode);
        
        float fMaxMil = vMaxBR.GetFloat() * vMaxBR.GetFloat();
        iErrCode = t_pCache->Increment(strEmpireFleets, iNewShipOrder, GameEmpireFleets::MaxStrength, fMaxMil);
        RETURN_ON_ERROR(iErrCode);

        // Remove ship from old fleet
        if (iOldFleetKey != NO_KEY) {
            
            // Update old fleet's strength
            iErrCode = t_pCache->Increment(strEmpireFleets, iOldFleetKey, GameEmpireFleets::CurrentStrength, -fCurrentMil);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->Increment(strEmpireFleets, iOldFleetKey, GameEmpireFleets::MaxStrength, -fMaxMil);
            RETURN_ON_ERROR(iErrCode);
        }

        // Set orders to follow fleet
        iNewShipOrder = FLEET;
    }
    else
    {
        ///////////////////////////////////////////////////////////////////////////////////////////
        // The order wasn't a join fleet,                                                        //
        // so let's check for leave, stand by, dismantle, nuke, remain, move or explore requests //
        ///////////////////////////////////////////////////////////////////////////////////////////

        iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Owner, &vTemp);
        RETURN_ON_ERROR(iErrCode);
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
                    return ERROR_SHIP_IS_NOT_IN_FLEET;
                }

                // Get ship's BR
                Variant vCurrentBR, vMaxBR;
                
                iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::CurrentBR, &vCurrentBR);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
                RETURN_ON_ERROR(iErrCode);

                // Subtract ship's strength from fleet's strength
                float fCurrentMil = vCurrentBR.GetFloat() * vCurrentBR.GetFloat();
                
                iErrCode = t_pCache->Increment(strEmpireFleets, iOldFleetKey, GameEmpireFleets::CurrentStrength, -fCurrentMil);
                RETURN_ON_ERROR(iErrCode);

                // Subtract ship's max strength from fleet's max strength
                fCurrentMil = vMaxBR.GetFloat() * vMaxBR.GetFloat();
                
                iErrCode = t_pCache->Increment(strEmpireFleets, iOldFleetKey, GameEmpireFleets::MaxStrength, -fCurrentMil);
                RETURN_ON_ERROR(iErrCode);

                // Set ship to no fleet
                iErrCode = t_pCache->WriteData(strEmpireShips, iShipKey, GameEmpireShips::FleetKey, (int)NO_KEY);
                RETURN_ON_ERROR(iErrCode);

                // Stand by
                iNewShipOrder = STAND_BY;
            }

            break;

        case FLEET:

            if (iOldFleetKey == NO_KEY)
            {
                return ERROR_SHIP_IS_NOT_IN_FLEET;
            }

#ifdef _DEBUG

            {

            Variant vFleetPlanet;

            iErrCode = t_pCache->ReadData(
                strEmpireFleets, 
                iOldFleetKey, 
                GameEmpireFleets::CurrentPlanet,
                &vFleetPlanet
                );

            RETURN_ON_ERROR(iErrCode);

            Assert(vFleetPlanet.GetInteger() == iShipPlanet);
            
            }
#endif

            break;

        case MOVE_NORTH:
        case MOVE_EAST:
        case MOVE_SOUTH:
        case MOVE_WEST:

            {
                // Make sure the ship is mobile
                if (!IsMobileShip (iShipType))
                {
                    return ERROR_CANNOT_MOVE;
                }
                
                // Get proxy key for empiremap
                unsigned int iPlanetProxyKey;
                iErrCode = t_pCache->GetFirstKey(
                    strEmpireMap, 
                    GameEmpireMap::PlanetKey, 
                    iShipPlanet, 
                    &iPlanetProxyKey
                    );

                RETURN_ON_ERROR(iErrCode);

                Variant vLink;
                iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Link, &vLink);
                RETURN_ON_ERROR(iErrCode);

                i = MOVE_NORTH - iNewShipOrder;

                // Make sure there's a link                     
                if (!(vLink.GetInteger() & LINK_X[i]))
                {
                    return ERROR_CANNOT_MOVE;
                }

                // Make sure we've explored the planet
                iErrCode = t_pCache->ReadData(
                    strEmpireMap, 
                    iPlanetProxyKey, 
                    GameEmpireMap::Explored, 
                    &vLink
                    );

                RETURN_ON_ERROR(iErrCode);
                        
                if (!(vLink.GetInteger() & EXPLORED_X[i]))
                {
                    return ERROR_CANNOT_MOVE;
                }
#ifdef _DEBUG
                // Make sure there's a planet
                Variant vNewPlanetKey;
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    iShipPlanet, 
                    GameMap::ColumnNames[GameMap::iNorthPlanetKey + i], 
                    &vNewPlanetKey
                    );

                Assert(iErrCode == OK && vNewPlanetKey != NO_KEY);
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
                    return ERROR_CANNOT_EXPLORE;
                }
                
                // Get proxy key for empiremap
                unsigned int iPlanetProxyKey;
                iErrCode = t_pCache->GetFirstKey(
                    strEmpireMap, 
                    GameEmpireMap::PlanetKey, 
                    iShipPlanet, 
                    &iPlanetProxyKey
                    );

                RETURN_ON_ERROR(iErrCode);
                
                Variant vLink;

                i = EXPLORE_NORTH - iNewShipOrder;
                        
                // Check for link
                iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Link, &vLink);
                RETURN_ON_ERROR(iErrCode);
                
                if (!(vLink.GetInteger() & LINK_X[i]))
                {
                    return ERROR_CANNOT_EXPLORE;
                }
                
                // Check for explored
                iErrCode = t_pCache->ReadData(
                    strEmpireMap, 
                    iPlanetProxyKey, 
                    GameEmpireMap::Explored, 
                    &vLink
                    );

                RETURN_ON_ERROR(iErrCode);
                
                if (vLink.GetInteger() & EXPLORED_X[i])
                {
                    return ERROR_CANNOT_EXPLORE;
                }
                
#ifdef _DEBUG
                // Make sure there's a planet
                Variant vNewPlanetKey;
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    iShipPlanet, 
                    GameMap::ColumnNames[GameMap::iNorthPlanetKey + i], 
                    &vNewPlanetKey
                    );
                Assert(iErrCode == OK && vNewPlanetKey != NO_KEY);
#endif
            }   // End indent

            break;

        case NUKE:

            // Make sure ship is mobile
            if (!IsMobileShip (iShipType)) {
                return ERROR_CANNOT_NUKE;
            }
            
            // Make sure ship is not cloaked                
            if (iShipState & CLOAKED) {
                return ERROR_CANNOT_NUKE;
            }

            // Make sure not an illegal nuking sci
            if (iShipType == SCIENCE && (iGameClassOptions & DISABLE_SCIENCE_SHIPS_NUKING)) {
                return ERROR_CANNOT_NUKE;
            }
            
            // Make sure not nuking our own planet or a system planet         
            if (iPlanetOwner == SYSTEM || iPlanetOwner == iEmpireKey) {
                return ERROR_CANNOT_NUKE;
            }
            
            // Check diplomatic status with owner
            if (iPlanetOwner != INDEPENDENT) {
                
                GET_GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
                
                // Make sure the diplomatic status is war
                Variant vDipStatus;
                unsigned int iKey;
                
                iErrCode = t_pCache->GetFirstKey(
                    strDiplomacy, 
                    GameEmpireDiplomacy::ReferenceEmpireKey, 
                    iPlanetOwner, 
                    &iKey
                    );

                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->ReadData(
                    strDiplomacy, 
                    iKey, 
                    GameEmpireDiplomacy::CurrentStatus, 
                    &vDipStatus
                    );

                RETURN_ON_ERROR(iErrCode);
                
                if (vDipStatus.GetInteger() != WAR) {
                    return ERROR_CANNOT_NUKE;
                }
            }

            break;

        case COLONIZE:
            
            // Make sure the ship is a colony
            if (iShipType != COLONY) {
                return ERROR_WRONG_SHIP_TYPE;
            }
            
            // Make sure the planet has 0 pop and wasn't annihilated and doesn't belong to us
            iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Pop, &vTemp);
            RETURN_ON_ERROR(iErrCode);

            if (vTemp.GetInteger() != 0) {
                return ERROR_CANNOT_COLONIZE;
            }
            
            iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Owner, &vTemp);
            RETURN_ON_ERROR(iErrCode);

            if ((unsigned int) vTemp.GetInteger() == iEmpireKey) {
                return ERROR_CANNOT_COLONIZE;
            }
            
            iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Annihilated, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            
            if (vTemp.GetInteger() != NOT_ANNIHILATED) {
                return ERROR_CANNOT_COLONIZE;
            }

            bDismantle = true;
            
            break;

        case DEPOSIT_POP:
               
            // Make sure the ship is a colony
            if (iShipType != COLONY) {
                return ERROR_WRONG_SHIP_TYPE;
            }

            // Make sure it's our planet
            if (iPlanetOwner != iEmpireKey) {
                return ERROR_CANNOT_SETTLE;
            }
            
            break;
            
        case CLOAK:
            
            // Make sure the ship is a cloaker
            if (iShipType != CLOAKER) {
                return ERROR_WRONG_SHIP_TYPE;
            }
            
            // Make sure it's not cloaked
            if (iShipState & CLOAKED) {
                return ERROR_CANNOT_CLOAK;
            }
            
            break;
            
        case UNCLOAK:

            // Make sure the ship is a cloaker
            if (iShipType != CLOAKER) {
                return ERROR_WRONG_SHIP_TYPE;
            }
            
            // Make sure it's cloaked
            if (!(iShipState & CLOAKED)) {
                return ERROR_CANNOT_UNCLOAK;
            }
            
            break;

        case TERRAFORM_AND_DISMANTLE:
        
            bDismantle = true;
            // No break

        case TERRAFORM:

            if ((iShipBehavior & TERRAFORMER_DISABLE_SURVIVAL) && !bDismantle)
            {
                return ERROR_CANNOT_TERRAFORM;
            }
            else
            {
                // Make sure the ship is a terraformer
                if (iShipType != TERRAFORMER) {
                    return ERROR_WRONG_SHIP_TYPE;
                }

                if (iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {

                    if (iPlanetOwner != iEmpireKey) {
                        break;
                    }
                }

                Variant vAg, vMin, vFuel;

                // Make sure that there's something to terraform
                iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Ag, &vAg);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Fuel, &vFuel);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Minerals, &vMin);
                RETURN_ON_ERROR(iErrCode);

                if (vAg.GetInteger() >= vFuel.GetInteger() && vAg.GetInteger() >= vMin.GetInteger()) {
                    return ERROR_CANNOT_TERRAFORM;
                }
            }
            
            break;
            
        case INVADE_AND_DISMANTLE:
        
            bDismantle = true;
            // No break

        case INVADE:
            {

            if (iNewShipOrder == INVADE && (iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL)) {
                return ERROR_CANNOT_INVADE;
            }

            // Make sure the ship is a troopship
            if (iShipType != TROOPSHIP) {
                return ERROR_WRONG_SHIP_TYPE;
            }
            
            // Make sure the planet belongs to someone else or is independent
            if (iPlanetOwner == SYSTEM || iPlanetOwner == iEmpireKey) {
                return ERROR_CANNOT_INVADE;
            }
            
            if (iPlanetOwner != INDEPENDENT) {
                
                GET_GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
                
                // Make sure the diplomatic status is war
                Variant vDipStatus;
                unsigned int iKey;
                iErrCode = t_pCache->GetFirstKey(
                    strDiplomacy, 
                    GameEmpireDiplomacy::ReferenceEmpireKey, 
                    iPlanetOwner, 
                    &iKey
                    );
                
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->ReadData(strDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
                RETURN_ON_ERROR(iErrCode);
                
                if (vDipStatus.GetInteger() != WAR) {
                    return ERROR_CANNOT_INVADE;
                }
            }
            
            }
            break;
            
        case ANNIHILATE:
            {
                Variant vHW;

                // Make sure the ship is a doomsday
                if (iShipType != DOOMSDAY) {
                    return ERROR_WRONG_SHIP_TYPE;
                }
                
                // Make sure the planet can be annihilated
                if (iPlanetOwner != SYSTEM && iPlanetOwner != INDEPENDENT) {
                    
                    if (iPlanetOwner == iEmpireKey) {

                        if (iGameClassOptions & DISABLE_SUICIDAL_DOOMSDAYS) {
                            return ERROR_CANNOT_ANNIHILATE;
                        }
                        
                        iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::HomeWorld, &vHW);
                        RETURN_ON_ERROR(iErrCode);
                        
                        if (vHW.GetInteger() == HOMEWORLD) {
                            return ERROR_CANNOT_ANNIHILATE;
                        }
                        
                    } else {
                        
                        GET_GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
                        
                        // Make sure the diplomatic status is war
                        Variant vDipStatus;
                        unsigned int iKey;
                        iErrCode = t_pCache->GetFirstKey(
                            strDiplomacy, 
                            GameEmpireDiplomacy::ReferenceEmpireKey, 
                            iPlanetOwner, 
                            &iKey
                            );

                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->ReadData(strDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
                        RETURN_ON_ERROR(iErrCode);
                        
                        if (vDipStatus.GetInteger() != WAR) {

                            if (!(iGameClassOptions & USE_UNFRIENDLY_DOOMSDAYS)) {
                                return ERROR_CANNOT_ANNIHILATE;
                            }

                            iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::HomeWorld, &vHW);
                            RETURN_ON_ERROR(iErrCode);
                            
                            if (vHW.GetInteger() == HOMEWORLD) {
                                return ERROR_CANNOT_ANNIHILATE;
                            }
                        }
                    }
                }
            }
            
            break;

        case DETONATE:

            // Make sure ship is a minefield
            if (iShipType != MINEFIELD) {
                return ERROR_WRONG_SHIP_TYPE;
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
                    return ERROR_WRONG_SHIP_TYPE;
                }

                int iDirection = OPEN_LINK_NORTH - iNewShipOrder;

                Assert(iDirection >= NORTH && iDirection <= WEST);
                
                // Make sure the link doesn't exist
                Variant vLink;
                iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Link, &vLink);
                RETURN_ON_ERROR(iErrCode);

                if (vLink.GetInteger() & LINK_X[iDirection]) {
                    return ERROR_CANNOT_OPEN_LINK;
                }
                
                // Make sure planet exists
                Variant vNeighbourKey;
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    iShipPlanet, 
                    GameMap::ColumnNames[GameMap::iNorthPlanetKey + iDirection], 
                    &vNeighbourKey
                    );

                RETURN_ON_ERROR(iErrCode);
                
                if (vNeighbourKey == NO_KEY) {
                    return ERROR_CANNOT_OPEN_LINK;
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
                    return ERROR_WRONG_SHIP_TYPE;
                }

                int iDirection = CLOSE_LINK_NORTH - iNewShipOrder;
                Assert(iDirection >= NORTH && iDirection <= WEST);
                
                // Make sure the link doesn't exist
                Variant vLink;
                iErrCode = t_pCache->ReadData(strGameMap, iShipPlanet, GameMap::Link, &vLink);
                RETURN_ON_ERROR(iErrCode);
                
                if (!(vLink.GetInteger() & LINK_X[iDirection])) {
                    return ERROR_CANNOT_CLOSE_LINK;
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
                    return ERROR_WRONG_SHIP_TYPE;
                }

                int iDirection = CREATE_PLANET_NORTH - iNewShipOrder;
                Assert(iDirection >= NORTH && iDirection <= WEST);

                // Make sure planet doesn't exist
                Variant vPlanetKey;
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    iShipPlanet, 
                    GameMap::ColumnNames[GameMap::iNorthPlanetKey + iDirection], 
                    &vPlanetKey
                    );

                RETURN_ON_ERROR(iErrCode);

                if (vPlanetKey.GetInteger() != NO_KEY) {
                    return ERROR_PLANET_EXISTS;
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
                    return ERROR_WRONG_SHIP_TYPE;
                }

                // Make sure ship isn't cloaked
                if (iShipState & CLOAKED) {
                    return ERROR_CANNOT_MORPH;
                }

                // Get tech
                int iMorphTech = MORPH_TECH (iNewShipOrder);

                GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

                // Make sure tech has been developed
                iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::TechDevs, &vTemp);
                RETURN_ON_ERROR(iErrCode);

                if (!(vTemp.GetInteger() & TECH_BITS[iMorphTech])) {
                    return ERROR_NO_TECHNOLOGY_AVAILABLE;
                }
            }
            break;

        default:

            {
                // Ship must be a stargate or a jumpgate
                if (iShipType != STARGATE && iShipType != JUMPGATE) {
                    return ERROR_WRONG_SHIP_TYPE;
                }
                
                // Make sure planet isn't the current planet the stargate is on
                if (iShipPlanet == iNewShipOrder) {
                    return ERROR_CANNOT_GATE;
                }

                if (iShipType == STARGATE) {

                    // Make sure we own the planet
                    iErrCode = t_pCache->ReadData(strGameMap, iNewShipOrder, GameMap::Owner, &vTemp);
                    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
                    {
                        return ERROR_CANNOT_GATE;
                    }
                    RETURN_ON_ERROR(iErrCode);

                    if ((unsigned int) vTemp.GetInteger() != iEmpireKey) {
                        return ERROR_CANNOT_GATE;
                    }

                } else {

                    unsigned int iPlanetProxyKey;

                    // Make sure we can see the planet
                    iErrCode = t_pCache->GetFirstKey(strEmpireMap, GameEmpireMap::PlanetKey, iNewShipOrder, &iPlanetProxyKey);
                    if (iErrCode == ERROR_DATA_NOT_FOUND)
                    {
                        return ERROR_CANNOT_GATE;
                    }
                    RETURN_ON_ERROR(iErrCode);

                    // Enforce annihilation rules
                    iErrCode = t_pCache->ReadData(strGameMap, iNewShipOrder, GameMap::Annihilated, &vTemp);
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (vTemp.GetInteger() != NOT_ANNIHILATED) {
                        return ERROR_CANNOT_GATE;
                    }
                }

                // We're not going to bother to enforce range rules here
                // People can feel free to submit custom data that targets
                // any possible planet.  However, range _will_ be enforced during
                // the update algorithm

                iErrCode = t_pCache->WriteData(
                    strEmpireShips, 
                    iShipKey, 
                    GameEmpireShips::GateDestination, 
                    iNewShipOrder
                    );

                RETURN_ON_ERROR(iErrCode);
                
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
        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->Increment(
            strEmpireData, 
            GameEmpireData::NextMaintenance, 
            - GetMaintenanceCost (iShipType, vMaxBR)
            );
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->Increment(
            strEmpireData, 
            GameEmpireData::NextFuelUse, 
            - GetFuelCost (iShipType, vMaxBR)
            );
        RETURN_ON_ERROR(iErrCode);   
    }

    else if (bOldDismantle && !bDismantle) {

        // Fix up predictions if we used to be dismantling and now we aren't
        Variant vMaxBR;
        iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::MaxBR, &vMaxBR);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->Increment(
            strEmpireData, 
            GameEmpireData::NextMaintenance, 
            GetMaintenanceCost (iShipType, vMaxBR)
            );

        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->Increment(
            strEmpireData, 
            GameEmpireData::NextFuelUse, 
            GetFuelCost (iShipType, vMaxBR.GetFloat())
            );

        RETURN_ON_ERROR(iErrCode);
    }
    
    // If we got here, then the order given was approved
    iErrCode = t_pCache->WriteData(strEmpireShips, iShipKey, GameEmpireShips::Action, iNewShipOrder);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetNumShips(int iGameClass, int iGameNumber, int iEmpireKey, int* piNumShips)
{
    GET_GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);
    return t_pCache->GetNumCachedRows(pszShips, (unsigned int*) piNumShips);
}

int GameEngine::GetNumFleets(int iGameClass, int iGameNumber, int iEmpireKey, int* piNumFleets)
{
    GET_GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, iEmpireKey);
    return t_pCache->GetNumCachedRows(pszFleets, (unsigned int*) piNumFleets);
}

int GameEngine::ChangeShipCloakingState (int iShipKey, int iPlanetKey, bool bCloaked, 
                                         const char* strEmpireShips, const char* strEmpireMap, 
                                         const char* strGameMap) {
    
    int iErrCode;
    unsigned int iProxyKey;

    int iCloakedIncrement, iUnCloakedIncrement;

#ifdef _DEBUG

    Variant vOldState, vNumShips;
    iErrCode = t_pCache->ReadData(strEmpireShips, iShipKey, GameEmpireShips::State, &vOldState);
    RETURN_ON_ERROR(iErrCode);

#endif
    
    // Set ship as cloaked
    if (bCloaked) {

#ifdef _DEBUG
        Assert(!(vOldState.GetInteger() & CLOAKED));
#endif
        iErrCode = t_pCache->WriteOr(
            strEmpireShips,
            iShipKey, 
            GameEmpireShips::State, 
            CLOAKED
            );
        RETURN_ON_ERROR(iErrCode);

        iCloakedIncrement = 1;
        iUnCloakedIncrement = -1;
    
    } else {

#ifdef _DEBUG
        Assert(vOldState.GetInteger() & CLOAKED);
#endif

        iErrCode = t_pCache->WriteAnd(
            strEmpireShips,
            iShipKey, 
            GameEmpireShips::State, 
            ~CLOAKED
            );
        RETURN_ON_ERROR(iErrCode);

        iCloakedIncrement = -1;
        iUnCloakedIncrement = 1;
    }

    // Change number of cloaked ships
    iErrCode = t_pCache->GetFirstKey(
        strEmpireMap, 
        GameEmpireMap::PlanetKey, 
        iPlanetKey, 
        &iProxyKey
        );

    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->Increment(strEmpireMap, iProxyKey, GameEmpireMap::NumCloakedShips, iCloakedIncrement);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->Increment(strEmpireMap, iProxyKey, GameEmpireMap::NumUncloakedShips, iUnCloakedIncrement);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->Increment(strGameMap, iPlanetKey, GameMap::NumCloakedShips, iCloakedIncrement);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->Increment(strGameMap, iPlanetKey, GameMap::NumUncloakedShips, iUnCloakedIncrement);
    RETURN_ON_ERROR(iErrCode);

#ifdef _DEBUG

    iErrCode = t_pCache->ReadData(strEmpireMap, iProxyKey, GameEmpireMap::NumUncloakedShips, &vNumShips);
    RETURN_ON_ERROR(iErrCode);

    if (bCloaked) {
        Assert(vNumShips.GetInteger() >= 0);
    } else {
        Assert(vNumShips.GetInteger() >= 1);
    }

    iErrCode = t_pCache->ReadData(strEmpireMap, iProxyKey, GameEmpireMap::NumCloakedShips, &vNumShips);
    RETURN_ON_ERROR(iErrCode);

    if (bCloaked) {
        Assert(vNumShips.GetInteger() >= 1);
    } else {
        Assert(vNumShips.GetInteger() >= 0);
    }

    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::NumUncloakedShips, &vNumShips);
    RETURN_ON_ERROR(iErrCode);

    if (bCloaked) {
        Assert(vNumShips.GetInteger() >= 0);
    } else {
        Assert(vNumShips.GetInteger() >= 1);
    }

    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::NumCloakedShips, &vNumShips);
    RETURN_ON_ERROR(iErrCode);

    if (bCloaked) {
        Assert(vNumShips.GetInteger() >= 1);
    } else {
        Assert(vNumShips.GetInteger() >= 0);
    }

#endif
    
    return iErrCode;
}

int GameEngine::GetColonyOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                 unsigned int iPlanetKey, bool* pbColonize, bool* pbSettle) {

    int iErrCode, iPop, iAnnihilated;
    unsigned int iOwner;

    ICachedTable* pMap = NULL;
    AutoRelease<ICachedTable> rel(pMap);

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    *pbColonize = *pbSettle = false;

    iErrCode = t_pCache->GetTable(strGameMap, &pMap);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pMap->ReadData(iPlanetKey, GameMap::Owner, (int*) &iOwner);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pMap->ReadData(iPlanetKey, GameMap::Pop, &iPop);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pMap->ReadData(iPlanetKey, GameMap::Annihilated, &iAnnihilated);
    RETURN_ON_ERROR(iErrCode);

    // We can colonize a planet if the pop is zero, we don't own it and it hasn't been annihilated
    if (iPop == 0 && iOwner != iEmpireKey && iAnnihilated == NOT_ANNIHILATED) {
        *pbColonize = true;
    }
    
    else if (iOwner == iEmpireKey) {

        // Maybe we can deposit pop on our planet
        int iMaxPop;
        iErrCode = pMap->ReadData(iPlanetKey, GameMap::MaxPop, &iMaxPop);
        RETURN_ON_ERROR(iErrCode);

        if (iMaxPop > iPop) {
            *pbSettle = true;
        }
    }

    return iErrCode;
}

int GameEngine::GetTerraformerOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                      unsigned int iPlanetKey, const GameConfiguration& gcConfig,
                                      bool* pbTerraform, bool* pbTerraformAndDismantle) {

    int iErrCode;
    Variant vAg, vMin, vFuel;
    
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    
    *pbTerraform = *pbTerraformAndDismantle = false;

    if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {

        Variant vOwner;

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
        RETURN_ON_ERROR(iErrCode);

        if (vOwner.GetInteger() != (int) iEmpireKey) {
            return OK;
        }
    }

    // All planets can be terraformed, if their statistics allow it 
    // (even enemy-owned or uncolonized ones)

    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Ag, &vAg);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
    RETURN_ON_ERROR(iErrCode);

    if (vFuel.GetInteger() > vAg.GetInteger() || vMin.GetInteger() > vAg.GetInteger()) {

        if (!(gcConfig.iShipBehavior & TERRAFORMER_DISABLE_SURVIVAL)) {
            *pbTerraform = true;
        }

        *pbTerraformAndDismantle = true;
    }

    return iErrCode;
}

int GameEngine::GetTroopshipOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                    unsigned int iPlanetKey, const GameConfiguration& gcConfig,
                                    bool* pbInvade, bool* pbInvadeAndDismantle) {
                                    
    int iErrCode;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    *pbInvade = *pbInvadeAndDismantle = false;

    // Only enemy planets can be invaded
    Variant vOwner, vDipStatus;

    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    RETURN_ON_ERROR(iErrCode);

    if (vOwner.GetInteger() != (int) iEmpireKey && vOwner.GetInteger() != SYSTEM) {

        if (vOwner.GetInteger() == INDEPENDENT) {
            vDipStatus = WAR;
        } else {

            GET_GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

            unsigned int iKey;
            iErrCode = t_pCache->GetFirstKey(strEmpireDip, GameEmpireDiplomacy::ReferenceEmpireKey, vOwner, &iKey);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
            RETURN_ON_ERROR(iErrCode);
        }

        if (vDipStatus.GetInteger() == WAR) {

            // Invade
            if (!(gcConfig.iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL)) {

                *pbInvade = true;
            }

            *pbInvadeAndDismantle = true;
        }
    }

    return iErrCode;
}

int GameEngine::GetDoomsdayOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                   unsigned int iPlanetKey, const GameConfiguration& gcConfig,
                                   int iGameClassOptions, bool* pbAnnihilate) {
    int iErrCode;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    *pbAnnihilate = false;

    // Get owner
    Variant vOwner, vHW;

    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    RETURN_ON_ERROR(iErrCode);

    if (vOwner.GetInteger() == (int) iEmpireKey) {

        if (iGameClassOptions & DISABLE_SUICIDAL_DOOMSDAYS) {
            return OK;
        }

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::HomeWorld, &vHW);
        RETURN_ON_ERROR(iErrCode);

        if (vHW.GetInteger() == HOMEWORLD) {
            return OK;
        }

    } else { 

        if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {

            // Get dip status with owner
            unsigned int iKey;
            Variant vDipStatus;

            GET_GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

            iErrCode = t_pCache->GetFirstKey(
                strEmpireDip,
                GameEmpireDiplomacy::ReferenceEmpireKey,
                vOwner,
                &iKey
                );

            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
            RETURN_ON_ERROR(iErrCode);

            if (vDipStatus.GetInteger() != WAR) {

                if (!(iGameClassOptions & USE_UNFRIENDLY_DOOMSDAYS)) {
                    return OK;
                }

                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::HomeWorld, &vHW);
                RETURN_ON_ERROR(iErrCode);

                if (vHW.GetInteger() == HOMEWORLD) {
                    return OK;
                }
            }
        }
    }

    *pbAnnihilate = true;

    return iErrCode;
}

int GameEngine::MoveShip(unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
                         unsigned int iShipKey, unsigned int iPlanetKey, unsigned int iFleetKey)
{
    int iErrCode, iBuiltThisUpdate, iType;
    bool bFlag;

    ICachedTable* pShips = NULL;
    AutoRelease<ICachedTable> rel(pShips);

    GET_GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);

    // You want either a planet, or a fleet, or both
    Assert(iPlanetKey != NO_KEY || iFleetKey != NO_KEY);

    iErrCode = t_pCache->GetTable(pszShips, &pShips);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pShips->ReadData(iShipKey, GameEmpireShips::BuiltThisUpdate, &iBuiltThisUpdate);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_SHIP_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);
    
    // Get type
    iErrCode = pShips->ReadData(iShipKey, GameEmpireShips::Type, &iType);
    RETURN_ON_ERROR(iErrCode);

    // Check for immobile ships being moved into fleets
    if (iFleetKey != NO_KEY) {

        if (!IsMobileShip (iType)) {
            return ERROR_SHIP_CANNOT_JOIN_FLEET;
        }
    }

    if (iPlanetKey != NO_KEY) {

        Variant vName;
        float fBR;
        int iBuilt;

        // We're moving planets, so we must be a build ship
        // The checks in BuildNewShips will cover any other problems we might have
        if (iBuiltThisUpdate == 0) {
            return ERROR_SHIP_ALREADY_BUILT;
        }

        // Get name
        iErrCode = pShips->ReadData(iShipKey, GameEmpireShips::Name, &vName);
        RETURN_ON_ERROR(iErrCode);

        // Get BR
        iErrCode = pShips->ReadData(iShipKey, GameEmpireShips::MaxBR, &fBR);
        RETURN_ON_ERROR(iErrCode);

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

            RETURN_ON_ERROR(iErrCode);

            GameConfiguration gcConfig;
            iErrCode = GetGameConfiguration (&gcConfig);
            RETURN_ON_ERROR(iErrCode);

            unsigned int iCost = GetColonyPopulationBuildCost (
                gcConfig.iShipBehavior, 
                gcConfig.fColonyMultipliedBuildFactor, 
                gcConfig.iColonySimpleBuildFactor, 
                fBR
                );

            if (iPop < iCost) {
                return ERROR_INSUFFICIENT_POPULATION_FOR_COLONIES;
            }
        }

        // Delete the old ship first, because of a possible build limit
        iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, iShipKey);
        RETURN_ON_ERROR(iErrCode);

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

        RETURN_ON_ERROR(iErrCode);
    }
    
    else if (iFleetKey != NO_KEY) {

        // This is actually easy - all we do is submit an order to switch to the new fleet
        ShipOrder soOrder;
        soOrder.iKey = iFleetKey;
        soOrder.pszText = NULL;
        soOrder.sotType = SHIP_ORDER_NORMAL;

        iErrCode = UpdateShipOrders (iGameClass, iGameNumber, iEmpireKey, iShipKey, soOrder);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


int GameEngine::GetUnaffiliatedMobileShipsAtPlanet(unsigned int iGameClass, unsigned int iGameNumber,
                                                   unsigned int iEmpireKey, unsigned int iPlanetKey,
                                                   unsigned int** ppiShipKey, unsigned int* piNumShips)
{
    int iErrCode;
    ICachedTable* pShips = NULL;
    AutoRelease<ICachedTable> rel(pShips);

    unsigned int* piShipKey = NULL, iNumShips;
    AutoFreeKeys free(piShipKey);

    if (ppiShipKey)
        *ppiShipKey = NULL;
    *piNumShips = 0;

    GET_GAME_EMPIRE_SHIPS(pszShips, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->GetTable(pszShips, &pShips);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pShips->GetEqualKeys(GameEmpireShips::CurrentPlanet, iPlanetKey, &piShipKey, &iNumShips);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
        return OK;
    RETURN_ON_ERROR(iErrCode);

    unsigned int iIndex = 0;
    for (unsigned int i = 0; i < iNumShips; i ++)
    {
        int iFleetKey;
        iErrCode = pShips->ReadData(piShipKey[i], GameEmpireShips::FleetKey, &iFleetKey);
        RETURN_ON_ERROR(iErrCode);

        if (iFleetKey != NO_KEY)
            continue;

        int iType;
        iErrCode = pShips->ReadData(piShipKey[i], GameEmpireShips::Type, &iType);
        RETURN_ON_ERROR(iErrCode);

        if (!IsMobileShip (iType))
            continue;

        piShipKey[iIndex] = piShipKey[i];
        iIndex ++;
    }

    *piNumShips = iIndex;
    if (ppiShipKey)
    {
        *ppiShipKey = piShipKey;
        piShipKey = NULL; // Don't free here
    }

    return iErrCode;
}
