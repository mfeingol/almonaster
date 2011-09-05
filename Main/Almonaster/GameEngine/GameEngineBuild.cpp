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
//  OK -> Ships were built
//  ERROR_GAME_HAS_NOT_STARTED -> Game hasn't started yet
//  ERROR_WRONG_OWNER -> Empire doesn't own the planet
//  ERROR_INSUFFICIENT_POPULATION -> Planet is not a builder
//  ERROR_WRONG_TECHNOLOGY -> Tech cannot be used
//  ERROR_INSUFFICIENT_TECH_LEVEL -> BR is too high
//  ERROR_WRONG_NUMBER_OF_SHIPS
// 
// Build ships of a given specification

// Note - not idempotent on failure
int GameEngine::BuildNewShips (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey, unsigned int iNumShips, 
                               const char* pszShipName, float fBR, int iPlanetKey, int iFleetKey, 
                               int* piNumShipsBuilt, bool* pbBuildReduced) {

    int iErrCode;
    Variant vGameState, vTemp, vPopLostToColonies, vShipBehavior, vColonyMultipliedBuildFactor, vPop, 
        vColonySimpleBuildFactor;

    unsigned int iProxyPlanetKey;

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    GET_GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    *piNumShipsBuilt = 0;
    *pbBuildReduced = false;

    // Validate input
    if (iTechKey < FIRST_SHIP || iTechKey > LAST_SHIP) {
        return ERROR_INVALID_ARGUMENT;
    }

    if (iNumShips < 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    if (fBR < 1.0) {
        return ERROR_INVALID_ARGUMENT;
    }

    if (iFleetKey != NO_KEY && !IsMobileShip (iTechKey)) {
        return ERROR_SHIP_CANNOT_JOIN_FLEET;
    }

    // Make sure game has started
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vGameState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(vGameState.GetInteger() & GAME_MAP_GENERATED)) {
        iErrCode = ERROR_GAME_HAS_NOT_STARTED;
        goto Cleanup;
    }

    // Make sure planet belongs to empire
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vTemp.GetInteger() != iEmpireKey) {
        iErrCode = ERROR_WRONG_OWNER;
        goto Cleanup;
    }

    // Get max num ships built at once
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::MaxNumShipsBuiltAtOnce, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (iNumShips < 1 || iNumShips > (unsigned int)vTemp.GetInteger()) {
        iErrCode = ERROR_WRONG_NUMBER_OF_SHIPS;
        goto Cleanup;
    }

    // Make sure planet has enough pop to build
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Pop, &vPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::BuilderPopLevel, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vPop.GetInteger() < vTemp.GetInteger()) {
        iErrCode = ERROR_INSUFFICIENT_POPULATION;
        goto Cleanup;
    }

    // Get ship behavior
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::ShipBehavior, &vShipBehavior);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::ColonyMultipliedBuildFactor, &vColonyMultipliedBuildFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::ColonySimpleBuildFactor, &vColonySimpleBuildFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Make sure planet has enough pop to build all the colonies requested (if any)
    unsigned int iPopLostToColoniesPerShip = 0;
    if (iTechKey == COLONY) {

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::PopLostToColonies, &vPopLostToColonies);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        Assert (vPopLostToColonies.GetInteger() >= 0);

        int iPopAvailable = vPop.GetInteger() - vPopLostToColonies.GetInteger();
        Assert (iPopAvailable >= 0);

        iPopLostToColoniesPerShip = GetColonyPopulationBuildCost (
            vShipBehavior.GetInteger(), 
            vColonyMultipliedBuildFactor.GetFloat(), 
            vColonySimpleBuildFactor.GetInteger(),
            fBR
            );

        Assert (iPopLostToColoniesPerShip >= 0);

        if (iPopLostToColoniesPerShip * iNumShips > (unsigned int)iPopAvailable) {

            iNumShips = iPopAvailable / iPopLostToColoniesPerShip;
            if (iNumShips == 0) {   
                iErrCode = ERROR_INSUFFICIENT_POPULATION_FOR_COLONIES;
                goto Cleanup;
            }

            *pbBuildReduced = true;
        }
    }

    // Make sure fleet exists and is at same planet
    if (iFleetKey != NO_KEY)
    {
        Variant vCurrentPlanet;
        iErrCode = t_pCache->ReadData(strEmpireFleets, iFleetKey, GameEmpireFleets::CurrentPlanet, &vCurrentPlanet);
        if (iErrCode != OK)
        {
            if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
            {
                iErrCode = ERROR_FLEET_DOES_NOT_EXIST;
            }
            goto Cleanup;
        }

        if (vCurrentPlanet.GetInteger() != iPlanetKey)
        {
            iErrCode = ERROR_FLEET_NOT_ON_PLANET;
            goto Cleanup;
        }
    }

    // Check for ship limits
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MaxNumShips, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vTemp.GetInteger() != INFINITE_SHIPS) {

        unsigned int iNumAvailableShips;
        iErrCode = t_pCache->GetNumCachedRows(strEmpireShips, &iNumAvailableShips);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iNumAvailableShips = vTemp.GetInteger() - iNumAvailableShips;
        Assert (iNumAvailableShips >= 0);

        if (iNumShips > iNumAvailableShips) {

            if (iNumAvailableShips < 1) {
                iErrCode = ERROR_SHIP_LIMIT_REACHED;
                goto Cleanup;
            }

            iNumShips = iNumAvailableShips;
            *pbBuildReduced = true;
        }
    }

    // At this point, we've already decided how many ships to build
    *piNumShipsBuilt = iNumShips;

    // Make sure empire has the tech requested
    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TechDevs, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(vTemp.GetInteger() & TECH_BITS [iTechKey])) {
        iErrCode = ERROR_WRONG_TECHNOLOGY;
        goto Cleanup;
    }

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TechLevel, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (GetBattleRank (vTemp.GetFloat()) < (int) fBR) {
        iErrCode = ERROR_INVALID_TECH_LEVEL;
        goto Cleanup;
    }

    {   // Scope

    // Prepare data for insertion
    Variant pvColVal[GameEmpireShips::NumColumns] = 
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        pszShipName,
        iTechKey,
        fBR,
        fBR,
        iPlanetKey,
        (iFleetKey == NO_KEY) ? BUILD_AT : iFleetKey,
        iFleetKey,
        1,
        iTechKey == MORPHER ? MORPH_ENABLED : 0,    // Set morpher flag
        NO_KEY,
        iPopLostToColoniesPerShip
    };

    if (pszShipName == NULL) {

        // Use default
        iErrCode = GetDefaultEmpireShipName (iEmpireKey, iTechKey, pvColVal + GameEmpireShips::iName);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // If ships are cloaked, set cloaked to true
    if (iTechKey == CLOAKER && (vShipBehavior.GetInteger() & CLOAKER_CLOAK_ON_BUILD)) {
        pvColVal[GameEmpireShips::iState] = pvColVal[GameEmpireShips::iState].GetInteger() | CLOAKED;
    }

    // Insert ships into table
    iErrCode = t_pCache->InsertDuplicateRows(strEmpireShips, GameEmpireShips::Template, pvColVal, iNumShips);
    if (iErrCode != OK)
    {
        goto Cleanup;
    }

    }   // Scope

    // Increment number of build ships on given system
    iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NumBuilds, iNumShips);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Make sure that planet is on our map
    // We know this already, but we need the proxy key
    iErrCode = t_pCache->GetFirstKey(strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, &iProxyPlanetKey);
    if (iErrCode != OK) {
        Assert (false);
        iErrCode = ERROR_WRONG_OWNER;
        goto Cleanup;
    }

    // If ships are cloaked, increment counts of ships on planet
    if (iTechKey == CLOAKER && (vShipBehavior.GetInteger() & CLOAKER_CLOAK_ON_BUILD)) {

        iErrCode = t_pCache->Increment(strEmpireMap, iProxyPlanetKey, GameEmpireMap::NumCloakedBuildShips, iNumShips);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->Increment(strGameMap, iPlanetKey, GameMap::NumCloakedBuildShips, iNumShips);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

    } else {

        iErrCode = t_pCache->Increment(strEmpireMap, iProxyPlanetKey, GameEmpireMap::NumUncloakedBuildShips, iNumShips);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->Increment(strGameMap, iPlanetKey, GameMap::NumUncloakedBuildShips, iNumShips);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Build into fleet if specified
    if (iFleetKey != NO_KEY) {

        // Increase power of fleet
        float fMil = (float) iNumShips * fBR * fBR;
        
        iErrCode = t_pCache->Increment(strEmpireFleets, iFleetKey, GameEmpireFleets::CurrentStrength, fMil);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->Increment(strEmpireFleets, iFleetKey, GameEmpireFleets::MaxStrength, fMil);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Increment number of buildships
        iErrCode = t_pCache->Increment(strEmpireFleets, iFleetKey, GameEmpireFleets::BuildShips, iNumShips);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Increment number of ships
        iErrCode = t_pCache->Increment(strEmpireFleets, iFleetKey, GameEmpireFleets::NumShips, iNumShips);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Set the fleet to stand by
        iErrCode = t_pCache->WriteData(strEmpireFleets, iFleetKey, GameEmpireFleets::Action, STAND_BY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Increase build total
    iErrCode = t_pCache->Increment(
        strEmpireData, 
        GameEmpireData::TotalBuild, 
        GetBuildCost (iTechKey, fBR) * iNumShips
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Increase next maintenance and fuel totals
    iErrCode = t_pCache->Increment(
        strEmpireData, 
        GameEmpireData::NextMaintenance, 
        iNumShips * GetMaintenanceCost (iTechKey, fBR)
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pCache->Increment(
        strEmpireData, 
        GameEmpireData::NextFuelUse, 
        iNumShips * GetFuelCost (iTechKey, fBR)
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Set last builder
    iErrCode = t_pCache->WriteData(
        strEmpireData,
        GameEmpireData::LastBuilderPlanet,
        iPlanetKey
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Reduce pop and resources at planet if ships are colony
    if (iTechKey == COLONY) {

        Variant vOldColsBuilding, vMin, vFuel, vMaxPop, vTotalAg, vTotalPop;
        const int iPopLostToTheseShips = iPopLostToColoniesPerShip * iNumShips;

        // Increment pop lost to colony build counter
        iErrCode = t_pCache->Increment(
            strGameMap, 
            iPlanetKey, 
            GameMap::PopLostToColonies, 
            iPopLostToTheseShips
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        const int iNewPopLostToColonies = vPopLostToColonies.GetInteger() + iPopLostToTheseShips;

        // Calculate pop change difference
        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalAg, &vTotalAg);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalPop, &vTotalPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->ReadData(
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxAgRatio, 
            &vTemp
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        float fAgRatio = GetAgRatio (vTotalAg.GetInteger(), vTotalPop.GetInteger(), vTemp.GetFloat());

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
        iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextTotalPop, iNextPopDiff, &vOldNextPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Get planet data
        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        int iDiff;

        iDiff = min (vOldNextPop.GetInteger() + iNextPopDiff, vMin.GetInteger()) - 
                min (vOldNextPop.GetInteger(), vMin.GetInteger());
        
        if (iDiff != 0) {
            iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextMin, iDiff);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
        
        iDiff = min (vOldNextPop.GetInteger() + iNextPopDiff, vFuel.GetInteger()) - 
            min (vOldNextPop.GetInteger(), vFuel.GetInteger());
        
        if (iDiff != 0) {
            iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextFuel, iDiff);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
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
// *piNumBuilds -> Number of builds
//
// Get the number of ships being built by the empire in the given game

int GameEngine::GetNumBuilds (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumBuilds) {

    Variant vTemp;
    GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireKey)

    int iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::NumBuilds, &vTemp);
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

    GET_GAME_EMPIRE_SHIPS (strShips, iGameClass, iGameNumber, iEmpireKey);

    unsigned int i, iNumShips, * piShipKey = NULL;

    int iErrCode = t_pCache->GetEqualKeys(
        strShips, 
        GameEmpireShips::BuiltThisUpdate, 
        1, 
        &piShipKey, 
        &iNumShips
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
        goto Cleanup;
    }

    if (iNumShips > 0) {

        for (i = 0; i < iNumShips; i ++) {

            // Best effort
            iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, piShipKey[i]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:

    if (piShipKey != NULL) {
        t_pCache->FreeKeys(piShipKey);
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiBuilderKey -> Integer keys of builders
// *piNumBuilders -> Number of builders
//
// Returns the builder planets a given empire has

int GameEngine::GetBuilderPlanetKeys(unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
                                     unsigned int** ppiBuilderKey, unsigned int* piNumBuilders)
{
    int iErrCode;

    ICachedTable* pMap = NULL;
    unsigned int* piBuilderKey = NULL, iNumBuilders;

    *ppiBuilderKey = NULL;
    *piNumBuilders = 0;

    // Get builder level
    Variant vBuilderPopLevel;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::BuilderPopLevel, &vBuilderPopLevel);
    if (iErrCode != OK)
        goto Cleanup;

    // Search for matches
    GET_GAME_MAP(strGameMap, iGameClass, iGameNumber);
    iErrCode = t_pCache->GetTable(strGameMap, &pMap);
    if (iErrCode != OK)
        goto Cleanup;

    iErrCode = pMap->GetEqualKeys(GameMap::Owner, iEmpireKey, &piBuilderKey, &iNumBuilders);
    if (iErrCode != OK)
    {
        if (iErrCode == ERROR_DATA_NOT_FOUND)
            iErrCode = OK;
        return iErrCode;
    }

    unsigned int iIndex = 0;
    for (unsigned int i = 0; i < iNumBuilders; i ++)
    {
        Variant vPop;
        iErrCode = pMap->ReadData(piBuilderKey[i], GameMap::Pop, &vPop);
        if (iErrCode != OK)
            goto Cleanup;

        if (vPop.GetInteger() >= vBuilderPopLevel.GetInteger())
        {
            piBuilderKey[iIndex] = piBuilderKey[i];
            iIndex ++;
        }
    }

    *ppiBuilderKey = piBuilderKey;
    piBuilderKey = NULL;
    *piNumBuilders = iIndex;

Cleanup:

    if (piBuilderKey)
        t_pCache->FreeKeys(piBuilderKey);

    SafeRelease(pMap);

    return iErrCode;
}


int GameEngine::GetBuildLocations (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
                                   unsigned int iPlanetKey, BuildLocation** ppblBuildLocation,
                                   unsigned int* piNumLocations) {

    int iErrCode;
    unsigned int* piBuilderKey = NULL, iNumBuilders, iNumLocations, i, iRemainingFleets;

    BuildLocation* pblBuildLocation = NULL;

    GET_GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, iEmpireKey);

    *ppblBuildLocation = NULL;
    *piNumLocations = 0;

    // Multi-planet scenario
    if (iPlanetKey == NO_KEY) {

        iErrCode = GetBuilderPlanetKeys (iGameClass, iGameNumber, iEmpireKey, &piBuilderKey, &iNumBuilders);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iNumBuilders == 0) {
            // We're done
            goto Cleanup;
        }

    } else {

        // Single-planet scenario
        iNumBuilders = 1;
        piBuilderKey = &iPlanetKey;
    }

    // Get number of fleets
    // (For the single-planet case, we could query for the exact number
    // as done below, but it's less code this way. Besides, no one uses that many fleets...)
    unsigned int iNumFleets;
    iErrCode = t_pCache->GetNumCachedRows(pszFleets, &iNumFleets);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Heuristic: we can't have more locations than builders * 2 + fleets
    unsigned int iNumLocationsAllocated = iNumBuilders  * 2 + iNumFleets;

    pblBuildLocation = new BuildLocation [iNumLocationsAllocated];
    if (pblBuildLocation == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    iNumLocations = 0;
    iRemainingFleets = iNumFleets;
    for (i = 0; i < iNumBuilders; i ++) {

        unsigned int *piFleetKey = NULL, iNumFleetsOnPlanet, j;

        // Add self, no fleet
        Assert (iNumLocations < iNumLocationsAllocated);
        pblBuildLocation [iNumLocations].iPlanetKey = piBuilderKey[i];
        pblBuildLocation [iNumLocations].iFleetKey = NO_KEY;
        iNumLocations ++;

        // Add self, new fleet
        Assert (iNumLocations < iNumLocationsAllocated);
        pblBuildLocation [iNumLocations].iPlanetKey = piBuilderKey[i];
        pblBuildLocation [iNumLocations].iFleetKey = FLEET_NEWFLEETKEY;
        iNumLocations ++;

        // Find all fleets on this planet
        if (iRemainingFleets > 0) {

            // This is accelerated by an index, thankfully
            iErrCode = t_pCache->GetEqualKeys(
                pszFleets,
                GameEmpireFleets::CurrentPlanet,
                piBuilderKey[i],
                &piFleetKey,
                &iNumFleetsOnPlanet
                );

            if (iErrCode == OK) {

                Assert (iNumFleetsOnPlanet > 0);
                iRemainingFleets -= iNumFleetsOnPlanet;
                Assert (iRemainingFleets < iNumFleets);

                for (j = 0; j < iNumFleetsOnPlanet; j ++) {

                    Assert (iNumLocations < iNumLocationsAllocated);
                    pblBuildLocation [iNumLocations].iPlanetKey = piBuilderKey[i];
                    pblBuildLocation [iNumLocations].iFleetKey = piFleetKey[j];
                    iNumLocations ++;
                }

                t_pCache->FreeKeys(piFleetKey);
            }

            else if (iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                goto Cleanup;
            }

            else iErrCode = OK;
        }
    }

    *ppblBuildLocation = pblBuildLocation;
    pblBuildLocation = NULL;

    *piNumLocations = iNumLocations;

Cleanup:

    if (pblBuildLocation != NULL) {
        delete [] pblBuildLocation;
    }

    if (piBuilderKey != NULL && piBuilderKey != &iPlanetKey) {
        t_pCache->FreeKeys(piBuilderKey);
    }

    return iErrCode;
}

int GameEngine::IsPlanetBuilder (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                                 unsigned int iPlanetKey, bool* pbBuilder) {

    int iErrCode;
    Variant vTemp, vPop;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    *pbBuilder = false;

    // Make sure planet belongs to empire
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if ((unsigned int) vTemp.GetInteger() != iEmpireKey) {
        goto Cleanup;
    }

    // Make sure planet has enough pop to build
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Pop, &vPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::BuilderPopLevel, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vPop.GetInteger() < vTemp.GetInteger()) {
        goto Cleanup;
    }

    *pbBuilder = true;

Cleanup:

    return iErrCode;
}