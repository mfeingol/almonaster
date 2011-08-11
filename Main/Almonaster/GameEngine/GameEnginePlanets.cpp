//
// GameEngine.dll:  a component of Almonaster
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

#include "Osal/Algorithm.h"

#include "GameEngine.h"

#include "../MapGen/DefaultMapGenerator.h"
#include "../MapGen/MirroredMapGenerator.h"
#include "../MapGen/TwistedMapGenerator.h"
#include "../MapGen/FairMapGenerator.h"

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iPlanetKey -> Key of planet
//
// Output:
// *piPlanetX -> X coordinate
// *piPlanetY -> Y coordinate
//
// Return the coordinates of a planet

int GameEngine::GetPlanetCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, int* piPlanetX, 
                                      int* piPlanetY) {

    Variant vTemp;
    GAME_MAP (pszMap, iGameClass, iGameNumber);

    int iErrCode = t_pConn->ReadData(
        pszMap, 
        iPlanetKey, 
        GameMap::Coordinates, 
        &vTemp
        );
    
    if (iErrCode == OK) {
        GetCoordinates (vTemp.GetCharPtr(), piPlanetX, piPlanetY);
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iPlanetKey -> Key of planet
//
// Output:
// *pvProperty -> Planet property
//
// Return the property of a planet

int GameEngine::GetPlanetProperty (int iGameClass, int iGameNumber, unsigned int iPlanetKey, 
                                   const char* pszProperty, Variant* pvProperty) {

    GAME_MAP (pszMap, iGameClass, iGameNumber);
    return t_pConn->ReadData(pszMap, iPlanetKey, pszProperty, pvProperty);
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iX -> X coordinate
// iY -> Y coordinate
//
// Output:
// piPlanetKey -> Key of planet
//
// Return the key of a planet

int GameEngine::GetPlanetKeyFromCoordinates (int iGameClass, int iGameNumber, int iX, int iY, int* piPlanetKey) {

    int iErrCode;
    unsigned int iKey;

    char pszCoord [MAX_COORDINATE_LENGTH + 1];

    GAME_MAP (pszMap, iGameClass, iGameNumber);

    GetCoordinates (iX, iY, pszCoord);

    iErrCode = t_pConn->GetFirstKey (
        pszMap,
        GameMap::Coordinates,
        pszCoord,
        &iKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
        *piPlanetKey = NO_KEY;
    }

    else {

        if (iErrCode == OK) {
            *piPlanetKey = iKey;
        }

        else Assert (false);
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
//
// Output
// *piLivePlanetKey -> Live planet key
// *piDeadPlanetKey -> Dead planet key
//
// Return the keys the empire uses to represent planets

int GameEngine::GetEmpirePlanetIcons (int iEmpireKey, unsigned int* piLivePlanetKey, 
                                      unsigned int* piLiveDeadPlanetKey) {

    Variant vTemp;
    int iErrCode;

    iErrCode = t_pConn->ReadData(
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::AlmonasterTheme, 
        &vTemp
        );
    
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    if (vTemp.GetInteger() == INDIVIDUAL_ELEMENTS) {
        
        iErrCode = t_pConn->ReadData(SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UILivePlanet, &vTemp);
        if (iErrCode != OK) {
            return iErrCode;
        }
        *piLivePlanetKey = vTemp.GetInteger();
        
        iErrCode = t_pConn->ReadData(SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIDeadPlanet, &vTemp);
        if (iErrCode != OK) {
            return iErrCode;
        }
        *piLiveDeadPlanetKey = vTemp.GetInteger();
        
    } else {
        
        *piLivePlanetKey = *piLiveDeadPlanetKey = vTemp.GetInteger();
    }

    return iErrCode;
}

void GameEngine::AdvanceCoordinates (int iX, int iY, int* piX, int* piY, int cpDirection) {

    *piX = iX;
    *piY = iY;

    switch (cpDirection) {
        
    case NORTH:
        (*piY) ++;
        break;
        
    case EAST:
        (*piX) ++;
        break;
        
    case SOUTH:
        (*piY) --;
        break;
        
    case WEST:
        (*piX) --;
        break;
    }
}


int GameEngine::AddEmpiresToMap (int iGameClass, int iGameNumber, int* piEmpireKey, int iNumEmpires, 
                                 GameFairnessOption gfoFairness, bool* pbCommit) {

    int iErrCode, iMinNumPlanets, iMaxNumPlanets, i, iGameClassOptions;

    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    char pszEmpireData [256];

    Variant** ppvPlanetData = NULL, * pvGameClassData = NULL, * pvGameData = NULL, ** ppvNewPlanetData = NULL, 
        * pvNewPlanetData = NULL, vTotalAg;

    unsigned int* piPlanetKey = NULL, iNumPlanets = 0, iNumNewPlanets;

    IMapGenerator* pMapGen = NULL, * pInner = NULL;

    // Nothing committed yet
    *pbCommit = false;

    // Read gameclass data
    iErrCode = t_pConn->ReadRow (SYSTEM_GAMECLASS_DATA, iGameClass, &pvGameClassData);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iGameClassOptions = pvGameClassData[SystemGameClassData::iOptions].GetInteger();

    // Read game data
    iErrCode = t_pConn->ReadRow(strGameData, NO_KEY, &pvGameData);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    int iGameOptions = pvGameData[GameData::iOptions].GetInteger();

    // Allocate new planet data
    iMinNumPlanets = pvGameClassData[SystemGameClassData::iMinNumPlanets].GetInteger();
    iMaxNumPlanets = pvGameClassData[SystemGameClassData::iMaxNumPlanets].GetInteger();

    // Create a new map generator
    if (iGameOptions & GAME_MIRRORED_MAP) {
        pMapGen = new MirroredMapGenerator(this);
    } else if (iGameOptions & GAME_TWISTED_MAP) {
        pMapGen = new TwistedMapGenerator(this);
    } else {
        pMapGen = new DefaultMapGenerator(this);
    }

    if (pMapGen == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    // Wrap map generator in a fair map generator
    pInner = pMapGen;
    pMapGen = new FairMapGenerator(this, pInner, gfoFairness);

    // Get existing map
    iErrCode = t_pConn->ReadColumns (
        strGameMap,
        GameMap::NumColumns,
        GameMap::ColumnNames,
        &piPlanetKey,
        &ppvPlanetData,
        &iNumPlanets
        );
    
    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        goto Cleanup;
    }

    // Call into the map generator to get new planets
    iErrCode = pMapGen->CreatePlanets(
        iGameClass,
        iGameNumber,
        piEmpireKey,
        iNumEmpires,
        ppvPlanetData,
        iNumPlanets,
        pvGameClassData,
        pvGameData,
        &ppvNewPlanetData,
        &iNumNewPlanets
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = CreateMapFromMapGeneratorData(
        iGameClass,
        iGameNumber,
        piEmpireKey,
        iNumEmpires,
        pvGameClassData,
        pvGameData,
        ppvNewPlanetData,
        iNumNewPlanets,
        piPlanetKey,
        ppvPlanetData,
        iNumPlanets,
        pbCommit        // Assumed initialized, set to true if data is committed
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Handle 'next statistics'
    for (i = 0; i < iNumEmpires; i ++) {

        GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, piEmpireKey[i]);
        iErrCode = t_pConn->ReadData(pszEmpireData, GameEmpireData::TotalAg, &vTotalAg);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = WriteNextStatistics (
            iGameClass, 
            iGameNumber, 
            piEmpireKey[i],
            vTotalAg.GetInteger(), 
            0, // No trades or allies at beginning
            pvGameClassData[SystemGameClassData::iMaxAgRatio].GetFloat()
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

#ifdef _DEBUG
    // Verify map
    iErrCode = VerifyMap (iGameClass, iGameNumber);
    Assert (iErrCode == OK);
    iErrCode = OK;
#endif

Cleanup:

    if (pvGameClassData != NULL) {
        t_pConn->FreeData(pvGameClassData);
    }

    if (pvGameData != NULL) {
        t_pConn->FreeData(pvGameData);
    }

    if (piPlanetKey != NULL) {
        t_pConn->FreeKeys(piPlanetKey);
    }

    if (pvNewPlanetData != NULL) {
        delete [] pvNewPlanetData;
    }

    if (ppvPlanetData != NULL) {
        t_pConn->FreeData(ppvPlanetData);
    }

    if (ppvNewPlanetData != NULL) {
        pMapGen->FreePlanetData(ppvNewPlanetData);
    }

    delete pMapGen;
    delete pInner;

    return iErrCode;    
}

//
// Process output from a map generator
//
int GameEngine::CreateMapFromMapGeneratorData(int iGameClass,
                                              int iGameNumber,
                                              int* piNewEmpireKey,
                                              unsigned int iNumNewEmpires,
                                              Variant* pvGameClassData,
                                              Variant* pvGameData, 
                                              Variant** ppvNewPlanetData,
                                              unsigned int iNumNewPlanets,
                                              unsigned int* piExistingPlanetKey,
                                              Variant** ppvExistingPlanetData,
                                              unsigned int iNumExistingPlanets,
                                              bool* pbCommit) {

    int iErrCode = OK;

    IWriteTable* pWrite = NULL;

    unsigned int i, j;

    int iGameClassOptions, * piNewPlanetKey = NULL, 
        iHomeWorldIndex, iHomeWorldKey, iMinX, iMinY, iMaxX, iMaxY,
        iGameMinX, iGameMaxX, iGameMinY, iGameMaxY;

    Variant* pvEmpireKey = NULL;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    // Transfer new game data to the GameData table
    if (pvGameData != NULL) {

        iErrCode = t_pConn->GetTableForWriting (strGameData, &pWrite);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (
            GameData::NumPlanetsPerEmpire, 
            pvGameData[GameData::iNumPlanetsPerEmpire].GetInteger()
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (GameData::HWAg, pvGameData[GameData::iHWAg].GetInteger());
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (GameData::HWMin, pvGameData[GameData::iHWMin].GetInteger());
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (GameData::HWFuel, pvGameData[GameData::iHWFuel].GetInteger());
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (GameData::AvgAg, pvGameData[GameData::iAvgAg].GetInteger());
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (GameData::AvgMin, pvGameData[GameData::iAvgMin].GetInteger());
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (GameData::AvgFuel, pvGameData[GameData::iAvgFuel].GetInteger());
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pWrite);
    }

    // Initialize some vars
    iGameClassOptions = pvGameClassData[SystemGameClassData::iOptions].GetInteger();

    iMinX = iMinY = MAX_COORDINATE;
    iMaxX = iMaxY = MIN_COORDINATE;

    iHomeWorldKey = iHomeWorldIndex = NO_KEY;

    piNewPlanetKey = (int*) StackAlloc (iNumNewPlanets * sizeof (int));

#ifdef _DEBUG
    memset (piNewPlanetKey, NO_KEY, iNumNewPlanets * sizeof (int));
#endif

    // Point of no return for map
    *pbCommit = true;

    // Perform new planet insertions
    for (i = 0; i < iNumNewPlanets; i ++) {

        iErrCode = CreatePlanetFromMapGeneratorData(
            ppvNewPlanetData[i],
            iGameClass,
            iGameNumber,
            ppvNewPlanetData[i][GameMap::iOwner].GetInteger(),
            iGameClassOptions,
            &iMinX,
            &iMaxX,
            &iMinY,
            &iMaxY,
            piNewPlanetKey + i
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iHomeWorldKey != NO_KEY && iHomeWorldIndex == NO_KEY) {
            iHomeWorldIndex = i;
        }
    }

    //
    // Fix up min, max for gamedata table
    //

    iErrCode = t_pConn->GetTableForWriting (strGameData, &pWrite);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // MinX
    iErrCode = pWrite->ReadData (GameData::MinX, &iGameMinX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (iMinX < iGameMinX) {
        
        iErrCode = pWrite->WriteData (GameData::MinX, iMinX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // MaxX
    iErrCode = pWrite->ReadData (GameData::MaxX, &iGameMaxX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (iMaxX > iGameMaxX) {
        
        iErrCode = pWrite->WriteData (GameData::MaxX, iMaxX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // MinY
    iErrCode = pWrite->ReadData (GameData::MinY, &iGameMinY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (iMinY < iGameMinY) {
        
        iErrCode = pWrite->WriteData (GameData::MinY, iMinY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // MaxY
    iErrCode = pWrite->ReadData (GameData::MaxY, &iGameMaxY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (iMaxY > iGameMaxY) {
        
        iErrCode = pWrite->WriteData (GameData::MaxY, iMaxY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    SafeRelease (pWrite);

    if (iGameClassOptions & EXPOSED_MAP) {

        // Insert all new planets into every empires' map
        unsigned int iTotalNumEmpires;
        iErrCode = t_pConn->ReadColumn(
            strGameEmpires,
            GameEmpires::EmpireKey,
            NULL,
            &pvEmpireKey,
            &iTotalNumEmpires
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        for (i = 0; i < iNumNewPlanets; i ++) {

            for (j = 0; j < iTotalNumEmpires; j ++) {
                
                iErrCode = InsertPlanetIntoGameEmpireData(
                    iGameClass,
                    iGameNumber,
                    pvEmpireKey[j].GetInteger(),
                    piNewPlanetKey[i],
                    ppvNewPlanetData[i],
                    iGameClassOptions
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }

        // Insert all old planets into each new empires' map
        for (i = 0; i < iNumExistingPlanets; i ++) {

            for (j = 0; j < iNumNewEmpires; j ++) {

                iErrCode = InsertPlanetIntoGameEmpireData(
                    iGameClass,
                    iGameNumber,
                    piNewEmpireKey[j],
                    piExistingPlanetKey[i],
                    ppvExistingPlanetData[i],
                    iGameClassOptions
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }
    
    } else {

        // Not exposed maps...
        // Add new planets to their owner's map if they're homeworlds or if the map is fully colonized
        for (i = 0; i < iNumNewPlanets; i ++) {

            int iOwner = ppvNewPlanetData[i][GameMap::iOwner].GetInteger();
            if (iOwner != SYSTEM) {

                bool bHW = ppvNewPlanetData[i][GameMap::iHomeWorld].GetInteger() == HOMEWORLD;
                if (bHW || (iGameClassOptions & FULLY_COLONIZED_MAP)) {

                    Assert(iOwner != SYSTEM);

                    iErrCode = InsertPlanetIntoGameEmpireData(
                        iGameClass,
                        iGameNumber,
                        iOwner,
                        piNewPlanetKey[i],
                        ppvNewPlanetData[i],
                        iGameClassOptions
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
            }
        }
    }

Cleanup:

    if (pvEmpireKey != NULL) {
        t_pConn->FreeData(pvEmpireKey);
    }

    return iErrCode;
}


int GameEngine::CreatePlanetFromMapGeneratorData (Variant* pvPlanetData,
                                                  int iGameClass,
                                                  int iGameNumber,
                                                  int iEmpireKey,
                                                  int iGameClassOptions,
                                                  int* piMinX,
                                                  int* piMaxX,
                                                  int* piMinY,
                                                  int* piMaxY,
                                                  int* piNewPlanetKey
                                                  ) {

    int iErrCode = OK, iX, iY, iNewX, iNewY, iLink, cpDir;

    unsigned int iKey;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    char pszCoord [MAX_COORDINATE_LENGTH + 1];

    unsigned int piNeighbourKey [NUM_CARDINAL_POINTS];
    int piNeighbourDirection [NUM_CARDINAL_POINTS];
    bool pbLink [NUM_CARDINAL_POINTS];

    unsigned int i, iNumNeighbours;

    IWriteTable* pWrite = NULL;

    Assert(iEmpireKey != NO_KEY);

    bool bHomeWorld = pvPlanetData[GameMap::iHomeWorld].GetInteger() == HOMEWORLD;

    // Name
    if (bHomeWorld) {

        iErrCode = t_pConn->ReadData(
            SYSTEM_EMPIRE_DATA,
            iEmpireKey,
            SystemEmpireData::Name,
            pvPlanetData + GameMap::iName
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    
    } else {

        char pszPlanetName [MAX_PLANET_NAME_LENGTH + 1];
        sprintf (pszPlanetName, "Planet %s", pvPlanetData[GameMap::iCoordinates].GetCharPtr());

        pvPlanetData[GameMap::iName] = pszPlanetName;
    }

    //
    // Ag, Minerals, Fuel, Coordinates, Link, HomeWorld are set by generator
    //

    // Pop, MaxPop, Owner
    int iMin = pvPlanetData[GameMap::iMinerals].GetInteger();
    int iFuel = pvPlanetData[GameMap::iFuel].GetInteger();

    int iMaxPop = max (iMin, iFuel);
    if (iMaxPop == 0) {
        iMaxPop = 1;
    }

    pvPlanetData[GameMap::iMaxPop] = iMaxPop;

    int iAg, iPop;
    if (iEmpireKey != SYSTEM && (bHomeWorld || (iGameClassOptions & FULLY_COLONIZED_MAP))) {

        iAg = pvPlanetData[GameMap::iAg].GetInteger();
        iPop = iAg > iMaxPop ? iMaxPop : iAg;

        pvPlanetData[GameMap::iPop] = iPop;
        pvPlanetData[GameMap::iOwner] = iEmpireKey;

    } else {

        iAg = 0;
        iPop = 0;

        pvPlanetData[GameMap::iPop] = 0;
        pvPlanetData[GameMap::iOwner] = SYSTEM;
    }

    // Nuked
    pvPlanetData[GameMap::iNuked] = 0;

    // PopLostToColonies
    pvPlanetData[GameMap::iPopLostToColonies] = 0;

    // NumUncloakedShips, NumCloakedShips,  NumUncloakedBuildShips, NumCloakedBuildShips
    pvPlanetData[GameMap::iNumUncloakedShips] = 0;
    pvPlanetData[GameMap::iNumCloakedShips] = 0;
    pvPlanetData[GameMap::iNumUncloakedBuildShips] = 0;
    pvPlanetData[GameMap::iNumCloakedBuildShips] = 0;

    // Annihilated
    pvPlanetData[GameMap::iAnnihilated] = NOT_ANNIHILATED;

    iLink = pvPlanetData[GameMap::iLink].GetInteger();

    // 3.0 style Surrenders
    pvPlanetData[GameMap::iSurrenderNumAllies] = 0;
    pvPlanetData[GameMap::iSurrenderAlmonasterSignificance] = 0;
    pvPlanetData[GameMap::iSurrenderEmpireSecretKey] = (int64) 0;
    pvPlanetData[GameMap::iSurrenderAlmonasterScore] = (float) 0.0;

    // NorthPlanetKey, EastPlanetKey, SouthPlanetKey, WestPlanetKey,
    GetCoordinates(pvPlanetData[GameMap::iCoordinates].GetCharPtr(), &iX, &iY);

    if (iX < *piMinX) {
        *piMinX = iX;
    }

    if (iX > *piMaxX) {
        *piMaxX = iX;
    }

    if (iY < *piMinY) {
        *piMinY = iY;
    }

    if (iY > *piMaxY) {
        *piMaxY = iY;
    }

    iNumNeighbours = 0;

    ENUMERATE_CARDINAL_POINTS (cpDir) {

        AdvanceCoordinates (iX, iY, &iNewX, &iNewY, cpDir);
        GetCoordinates (iNewX, iNewY, pszCoord);

        iErrCode = t_pConn->GetFirstKey (
            strGameMap,
            GameMap::Coordinates,
            pszCoord,
            &iKey
            );
        
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            
            // Can't assert here because linked planets may not be in map yet
            // Assert (!(iLink & LINK_X[cpDir]));
            pvPlanetData[GameMap::iNorthPlanetKey + cpDir] = NO_KEY;
            
        } else {
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            Assert (iKey != NO_KEY);
            
            pvPlanetData[GameMap::iNorthPlanetKey + cpDir] = iKey;

            piNeighbourKey[iNumNeighbours] = iKey;
            piNeighbourDirection[iNumNeighbours] = cpDir;       
            pbLink[iNumNeighbours] = (iLink & LINK_X[cpDir]) != 0;
            
            iNumNeighbours ++;
        }
    }

    // Insert the planet, finally!
    iErrCode = t_pConn->InsertRow(strGameMap, GameMap::Template, pvPlanetData, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    *piNewPlanetKey = iKey;

    // Fix up neighboring links
    for (i = 0; i < iNumNeighbours; i ++) {

        iErrCode = t_pConn->WriteData (
            strGameMap,
            piNeighbourKey[i],
            GameMap::ColumnNames[GameMap::iNorthPlanetKey + OPPOSITE_CARDINAL_POINT [piNeighbourDirection[i]]],
            (int) iKey
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (pbLink[i]) {

            iErrCode = t_pConn->WriteOr (
                strGameMap,
                piNeighbourKey[i],
                GameMap::Link,
                OPPOSITE_LINK_X [piNeighbourDirection[i]]
            );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    if (iEmpireKey != SYSTEM && (bHomeWorld || (iGameClassOptions & FULLY_COLONIZED_MAP))) {

        // Increment some empire statistics
        GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

        Assert (pWrite == NULL);
        iErrCode = t_pConn->GetTableForWriting (strGameEmpireData, &pWrite);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // NumPlanets
        iErrCode = pWrite->Increment(GameEmpireData::NumPlanets, 1);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // TotalAg
        if (iAg > 0) {

            iErrCode = pWrite->Increment(GameEmpireData::TotalAg, iAg);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        // TotalMin
        iErrCode = pWrite->Increment(GameEmpireData::TotalMin, min (iMin, iPop));
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // TotalFuel
        iErrCode = pWrite->Increment(GameEmpireData::TotalFuel, min (iFuel, iPop));
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // TotalPop
        if (iPop > 0) {

            iErrCode = pWrite->Increment(GameEmpireData::TotalPop, iPop);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        // TargetPop
        iErrCode = pWrite->Increment(GameEmpireData::TargetPop, iMaxPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // HomeWorld
        if (bHomeWorld) {

            iErrCode = pWrite->WriteData(GameEmpireData::HomeWorld, (int)iKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        // TotalAg
        iErrCode = pWrite->ReadData(GameEmpireData::TotalAg, &iAg);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // TotalMin
        iErrCode = pWrite->ReadData(GameEmpireData::TotalMin, &iMin);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // TotalFuel
        iErrCode = pWrite->ReadData(GameEmpireData::TotalFuel, &iFuel);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Update Econ
        iErrCode = pWrite->WriteData(GameEmpireData::Econ, GetEcon(iFuel, iMin, iAg));
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    SafeRelease (pWrite);

    return iErrCode;
}

int GameEngine::InsertPlanetIntoGameEmpireData(int iGameClass, int iGameNumber, int iEmpireKey, 
                                               int iPlanetKey, const Variant* pvPlanetData,
                                               int iGameClassOptions) {

    int iErrCode;

    GAME_MAP(pszGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_MAP(pszGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA(pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    Variant pvOldPlanet[GameEmpireMap::NumColumns];

    IWriteTable* pEmpireData = NULL;

    Assert(iEmpireKey != SYSTEM && iEmpireKey != NO_KEY);
    Assert(iPlanetKey != NO_KEY);

    pvOldPlanet[GameEmpireMap::iPlanetKey] = iPlanetKey;
    pvOldPlanet[GameEmpireMap::iNumUncloakedShips] = 0;
    pvOldPlanet[GameEmpireMap::iNumCloakedBuildShips] = 0;
    pvOldPlanet[GameEmpireMap::iNumUncloakedBuildShips] = 0;
    pvOldPlanet[GameEmpireMap::iNumCloakedShips] = 0;
    
    Variant vKey;
    int iKey, iExplored = 0, cpDir;
    unsigned int iProxyKey;

    if ((iGameClassOptions & EXPOSED_MAP) || (iGameClassOptions & FULLY_COLONIZED_MAP)) {

        ENUMERATE_CARDINAL_POINTS (cpDir) {
            
            iErrCode = t_pConn->ReadData(
                pszGameMap,
                iPlanetKey,
                GameMap::NorthPlanetKey + cpDir,
                &vKey
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iKey = vKey.GetInteger();
            if (iKey != NO_KEY) {
                
                iErrCode = t_pConn->GetFirstKey (
                    pszGameEmpireMap,
                    GameEmpireMap::PlanetKey,
                    iKey,
                    &iProxyKey
                    );
                
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    
                    Assert (iProxyKey == NO_KEY);
                    iErrCode = OK;
                    
                } else {

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    Assert (iProxyKey != NO_KEY);
                    
                    iErrCode = t_pConn->WriteOr (
                        pszGameEmpireMap,
                        iProxyKey,
                        GameEmpireMap::Explored,
                        OPPOSITE_EXPLORED_X[cpDir]
                        );
                    
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    iExplored |= EXPLORED_X[cpDir];
                }
            }
        }
    }
    
    pvOldPlanet [GameEmpireMap::iExplored] = iExplored;
    
    iErrCode = t_pConn->InsertRow (pszGameEmpireMap, GameEmpireMap::Template, pvOldPlanet, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    //
    // Handle max/min settings
    //
    int iMinX, iMaxX, iMinY, iMaxY;

    Assert (pEmpireData == NULL);
    iErrCode = t_pConn->GetTableForWriting(pszGameEmpireData, &pEmpireData);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pEmpireData->ReadData (GameEmpireData::MinX, &iMinX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpireData->ReadData (GameEmpireData::MaxX, &iMaxX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpireData->ReadData (GameEmpireData::MinY, &iMinY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpireData->ReadData (GameEmpireData::MaxY, &iMaxY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    int iX, iY;
    GetCoordinates(pvPlanetData[GameMap::iCoordinates].GetCharPtr(), &iX, &iY);

    if (iX < iMinX) {
        iErrCode = pEmpireData->WriteData (GameEmpireData::MinX, iX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    
    if (iX > iMaxX) {
        iErrCode = pEmpireData->WriteData (GameEmpireData::MaxX, iX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    
    if (iY < iMinY) {
        iErrCode = pEmpireData->WriteData (GameEmpireData::MinY, iY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    
    if (iY > iMaxY) {
        iErrCode = pEmpireData->WriteData (GameEmpireData::MaxY, iY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    SafeRelease(pEmpireData);

    return iErrCode;
}


#ifdef _DEBUG

int GameEngine::VerifyMap (int iGameClass, int iGameNumber) {

    int iErrCode;

    IReadTable* pGameMap = NULL;

    unsigned int* piPlanetKey = NULL, iNumPlanets, iKey, iNumMatches, iNumPlanetsPerEmpire;
    unsigned int* piKey = NULL, iNumEmpires, i;

    int cpDir, iPlanet, iX, iY, iNewX, iNewY, iOwner, * piLink = NULL, iHomeWorld, iNeighbourHW;
    int iGameClassOptions, iGameOptions;

    char pszNewCoord [MAX_COORDINATE_LENGTH + 1];
    Variant vCoord, * pvCoord = NULL;

    bool* pbVisited, bExists;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    
    char strGameEmpireData [256];

    Variant vTemp;

    // Get num empires
    iErrCode = t_pConn->GetNumRows (strGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get gameclass options
    iErrCode = t_pConn->GetViews()->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iGameClassOptions = vTemp.GetInteger();

    // Get some game settings
    iErrCode = t_pConn->ReadData(strGameData, GameData::NumPlanetsPerEmpire, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iNumPlanetsPerEmpire = vTemp.GetInteger();

    iErrCode = t_pConn->ReadData(strGameData, GameData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iGameOptions = vTemp.GetInteger();

    iErrCode = t_pConn->GetTableForReading(strGameMap, &pGameMap);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameMap->ReadColumn(GameMap::Coordinates, &piPlanetKey, &pvCoord, &iNumPlanets);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        Assert (!"Map is empty");
        iErrCode = ERROR_FAILURE;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameMap->ReadColumn (GameMap::Link, NULL, &piLink, &iNumPlanets);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(iGameOptions & (GAME_MIRRORED_MAP | GAME_TWISTED_MAP)) &&
        (iNumPlanets % iNumPlanetsPerEmpire != 0)) {
        Assert (!"Map did not assign the same number of planets per empire");
        iErrCode = ERROR_FAILURE;
        goto Cleanup;
    }

    // Make sure:
    // all planets have different coordinates
    // each planet has a link
    // each planet has a neighbour
    // each link has a planet
    // empty neighbour slots are really empty

    for (i = 0; i < iNumPlanets; i ++) {

        // Make sure coordinates are unique
        iErrCode = pGameMap->GetEqualKeys (
            GameMap::Coordinates,
            pvCoord[i].GetCharPtr(),
            NULL,
            &iNumMatches
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iNumMatches != 1) {
            Assert (!"A planet's coordinates were re-used");
            iErrCode = ERROR_FAILURE;
            goto Cleanup;
        }

        if (piLink[i] == 0 && iNumPlanetsPerEmpire != 1) {
            Assert (!"Planet has no links");
            iErrCode = ERROR_FAILURE;
            goto Cleanup;
        }

        GetCoordinates(pvCoord[i].GetCharPtr(), &iX, &iY);

        iNumMatches = 0;
        
        iErrCode = pGameMap->ReadData (piPlanetKey[i], GameMap::HomeWorld, &iHomeWorld);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        ENUMERATE_CARDINAL_POINTS(cpDir) {

            iErrCode = pGameMap->ReadData (piPlanetKey[i], GameMap::NorthPlanetKey + cpDir, &iPlanet);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            AdvanceCoordinates (iX, iY, &iNewX, &iNewY, cpDir);
            GetCoordinates (iNewX, iNewY, pszNewCoord);
            
            if (iPlanet == NO_KEY) {
                
                // Make sure there's no planet
                iErrCode = pGameMap->GetFirstKey (
                    GameMap::Coordinates,
                    pszNewCoord,
                    &iKey
                    );
                
                if (iErrCode == OK) {
                    
                    Assert (iKey != NO_KEY);
                    
                    Assert (!"Existing planet not listed among a planet's neighbours");
                    iErrCode = ERROR_FAILURE;
                    goto Cleanup;
                }
                
                if (iErrCode != ERROR_DATA_NOT_FOUND) {
                    Assert (false);
                    goto Cleanup;
                }

                // Make sure we're not linked to nowhere
                if (piLink[i] & LINK_X[cpDir]) {

                    Assert (!"Linked planet does not exist");
                    iErrCode = ERROR_FAILURE;
                    goto Cleanup;
                }

            } else {

                iNumMatches ++;

                // Make sure there's a planet
                iErrCode = pGameMap->DoesRowExist (iPlanet, &bExists);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (!bExists) {
                    Assert (!"Unknown planet listed among a planet's neighbours");
                    iErrCode = ERROR_FAILURE;
                    goto Cleanup;
                }

                // Make sure that planet has the right coordinates
                iErrCode = pGameMap->ReadData (iPlanet, GameMap::Coordinates, &vCoord);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (strcmp (vCoord.GetCharPtr(), pszNewCoord) != 0) {
                    Assert (!"Planet has wrong coordinates");
                    iErrCode = ERROR_FAILURE;
                    goto Cleanup;
                }

                if (iHomeWorld == HOMEWORLD && iNumPlanetsPerEmpire > 1) {

                    iErrCode = pGameMap->ReadData (iPlanet, GameMap::HomeWorld, &iNeighbourHW);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    if (iNeighbourHW == HOMEWORLD) {
                        
                        // Make sure there's no link between two homeworlds
                        if (piLink[i] & LINK_X[cpDir]) {
                            Assert (!"Homeworlds are adjacent");
                            iErrCode = ERROR_FAILURE;
                            goto Cleanup;
                        }
                    }
                }
            }

        }   // End enum cardinal pts

        if (iNumMatches == 0 && iNumPlanets != 1) {
            Assert (!"A planet has no neighbours");
            iErrCode = ERROR_FAILURE;
            goto Cleanup;
        }

    }   // End all planets loop

    // Verify all planets are connected
    if (!(iGameClassOptions & DISCONNECTED_MAP)) {

        pbVisited = (bool*) StackAlloc (iNumPlanets * sizeof (bool));
        memset (pbVisited, false, iNumPlanets * sizeof (bool));
        
        iErrCode = DfsTraversePlanets (pGameMap, 0, piLink, pbVisited, iNumPlanets);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        for (i = 0; i < iNumPlanets; i ++) {
            
            if (!pbVisited[i]) {
                Assert (!"The map is not a connected graph");
                iErrCode = ERROR_FAILURE;
                goto Cleanup;
            }
        }
    }

    // Verify each empire has exactly one homeworld
    iErrCode = pGameMap->GetEqualKeys (
        GameMap::HomeWorld,
        HOMEWORLD,
        &piKey,
        &iNumMatches
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (iNumMatches != iNumEmpires) {
        Assert (!"The map does not contain the right number of homeworlds");
        iErrCode = ERROR_FAILURE;
        goto Cleanup;
    }

    SafeRelease (pGameMap);

    for (i = 0; i < iNumMatches; i ++) {

        iErrCode = t_pConn->ReadData(strGameMap, piKey[i], GameMap::Owner, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iOwner = vTemp.GetInteger();

        GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iOwner);

        iErrCode = t_pConn->ReadData(
            strGameEmpireData,
            GameEmpireData::HomeWorld,
            &vTemp
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if ((int) piKey[i] != vTemp.GetInteger()) {
            Assert (!"A homeworld does not match its owner's data");
            iErrCode = ERROR_FAILURE;
            goto Cleanup;
        }
    }

Cleanup:

    SafeRelease (pGameMap);

    if (piPlanetKey != NULL) {
        t_pConn->FreeKeys(piPlanetKey);
    }

    if (piKey != NULL) {
        t_pConn->FreeKeys(piKey);
    }

    if (piLink != NULL) {
        t_pConn->FreeData(piLink);
    }

    if (pvCoord != NULL) {
        t_pConn->FreeData(pvCoord);
    }

    return iErrCode;
}


int GameEngine::DfsTraversePlanets (IReadTable* pGameMap, unsigned int iPlanetKey, int* piLink, bool* pbVisited, 
                                    unsigned int iNumPlanets) {

    int iErrCode, cpDir, iNextPlanetKey;

    Assert (iPlanetKey < iNumPlanets);

    // Don't revisit
    if (pbVisited[iPlanetKey]) {
        return OK;
    }

    // Set visited
    pbVisited[iPlanetKey] = true;

    ENUMERATE_CARDINAL_POINTS(cpDir) {

        if (piLink[iPlanetKey] & LINK_X[cpDir]) {

            // Find key
            iErrCode = pGameMap->ReadData (
                iPlanetKey,
                GameMap::NorthPlanetKey + cpDir,
                &iNextPlanetKey
                );

            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            if (iNextPlanetKey == NO_KEY) {
                Assert (!"Linked planet does not exist");
                return ERROR_FAILURE;
            }

            iErrCode = DfsTraversePlanets (
                pGameMap,
                iNextPlanetKey,
                piLink,
                pbVisited,
                iNumPlanets
                );

            if (iErrCode != OK) {
                return iErrCode;
            }
        }
    }   

    // All done
    return OK;
}

#endif


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iPlanetKey -> Integer key of planet
// iEmpireKey -> Key of querying empire
//
// Output:
// *pvPlanetName -> Name of planet
//
// Return the name of the given planet, if the empire has explored it.  Otherwise, return unknown.

int GameEngine::GetPlanetNameWithSecurity (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                                           Variant* pvPlanetName) {

    int iErrCode = ERROR_DATA_NOT_FOUND;

    if (iEmpireKey != NO_KEY) {

        unsigned int iKey;
        GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
        iErrCode = t_pConn->GetFirstKey (strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, &iKey);
    }

    if (iErrCode == OK) {

        GAME_MAP (strGameMap, iGameClass, iGameNumber);
        
        return t_pConn->ReadData(
            strGameMap, 
            iPlanetKey, 
            GameMap::Name, 
            pvPlanetName
            );
    
    } else {

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            *pvPlanetName = "Unknown";
        }
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iPlanetKey -> Integer key of planet
//
// Output:
// *pvPlanetName -> Name of planet
//
// Return the name of the given planet

int GameEngine::GetPlanetName (int iGameClass, int iGameNumber, int iPlanetKey, Variant* pvPlanetName) {

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    return t_pConn->ReadData(strGameMap, iPlanetKey, GameMap::Name, pvPlanetName);
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iPlanetKey -> Integer key of planet
//
// Output:
// *pbExists -> Yes or no
//
// Does a planet exist

int GameEngine::DoesPlanetExist (int iGameClass, int iGameNumber, int iPlanetKey, bool* pbExists) {

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    int iErrCode = t_pConn->DoesRowExist (strGameMap, iPlanetKey, pbExists);
    if (iErrCode != OK) {
        *pbExists = false;
    }

    return false;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iPlanetKey -> Integer key of planet
// iDirection -> Direction to search
//
// Output:
// *piPlanetKey -> Key of planet
//
// Return the key of the planet located immediately in the given direction from the planet
// whose planet key is given

int GameEngine::GetNeighbourPlanetKey (int iGameClass, int iGameNumber, int iPlanetKey, int iDirection, 
                                       int* piPlanetKey) {

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    Variant vKey;
    int iErrCode = t_pConn->ReadData(strGameMap, iPlanetKey, GameMap::NorthPlanetKey + iDirection, &vKey);

    if (iErrCode == OK) {
        *piPlanetKey = vKey.GetInteger();
    }

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
// iEmpireKey -> Integer key of empire
//
// Output:
// *piMinX -> Min X coordinate of visited planets
// *piMaxX -> Max X coordinate of visited planets
// *piMinY -> Min Y coordinate of visited planets
// *piMaxY -> Max Y coordinate of visited planets
//
// Return the empire's map limits

int GameEngine::GetMapLimits (int iGameClass, int iGameNumber, int iEmpireKey, int* piMinX, int* piMaxX, 
                              int* piMinY, int* piMaxY) {

    IReadTable* pGameEmpireData;

    GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pConn->GetTableForReading(pszEmpireData, &pGameEmpireData);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pGameEmpireData->ReadData (GameEmpireData::MinX, piMinX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameEmpireData->ReadData (GameEmpireData::MaxX, piMaxX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameEmpireData->ReadData (GameEmpireData::MinY, piMinY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameEmpireData->ReadData (GameEmpireData::MaxY, piMaxY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pGameEmpireData);

    return iErrCode;
}


// Input:
// iGameClass -> Integer key of gameclass
// iGameNumber -> Game number
//
// Output:
// *piMinX -> Min X coordinate of all planets
// *piMaxX -> Max X coordinate of all planets
// *piMinY -> Min Y coordinate of all planets
// *piMaxY -> Max Y coordinate of all planets
//
// Return the game's map limits

int GameEngine::GetMapLimits (int iGameClass, int iGameNumber, int* piMinX, int* piMaxX, int* piMinY, 
                              int* piMaxY) {

    IReadTable* pGameData;

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    int iErrCode = t_pConn->GetTableForReading(strGameData, &pGameData);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pGameData->ReadData (GameData::MinX, piMinX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameData->ReadData (GameData::MaxX, piMaxX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameData->ReadData (GameData::MinY, piMinY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameData->ReadData (GameData::MaxY, piMaxY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pGameData);

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppvPlanetKey -> Integer keys of planets in Game Map
// **piEmpireMapKey -> Integer keys of planets in Game Empire Map
// *piNumKeys -> Number of keys
//
// Return the planet keys visited by the empire

int GameEngine::GetVisitedPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvPlanetKey, 
                                      unsigned int** ppiEmpireMapKey, unsigned int* piNumKeys) {

    GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pConn->ReadColumn (
        pszEmpireMap,
        GameEmpireMap::PlanetKey, 
        ppiEmpireMapKey, 
        ppvPlanetKey, 
        piNumKeys
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *piNumVisitedPlanets -> Number of visited planets
//
// Return the number of planets visited by the empire

int GameEngine::GetNumVisitedPlanets (int iGameClass, int iGameNumber, int iEmpireKey, 
                                      unsigned int* piNumVisitedPlanets) {

    GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    return t_pConn->GetNumRows (
        pszEmpireMap, 
        piNumVisitedPlanets
        );
}


int GameEngine::HasEmpireVisitedPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                                        bool* pbVisited) {

    unsigned int iKey;

    GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pConn->GetFirstKey (
        pszEmpireMap,
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        &iKey
        );

    switch (iErrCode) {
        
    case ERROR_DATA_NOT_FOUND:
        
        *pbVisited = false;
        iErrCode = OK;
        break;

    case OK:

        *pbVisited = true;
        break;

    default:

        *pbVisited = false;
        break;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of querying empire
// iPlanetKey -> Integer key of planet in Game Map
// iPlanetProxyKey -> Integer key of planet in Game Empire Map
//
// Output:
// **ppvPlanetData -> Planet data
// **ppiShipOwnerData -> Data about the ships on a given planet.
//  [0] -> Number of owners
//  [1] -> Key of first owner
//  [2] -> Number of ships owner has
//  [3] -> Number of ship types owner has
//  [4] -> First type key
//  [5] -> Number of ships of first type
//  repeat 4-5
//  repeat 1-5
//
// Returns data about a given visited planet

int GameEngine::GetPlanetShipOwnerData (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                                        int iPlanetProxyKey, unsigned int iTotalNumShips, bool bVisibleBuilds, 
                                        bool bIndependence, unsigned int** ppiShipOwnerData) {

    int iErrCode;
    Assert (iTotalNumShips > 0);

    *ppiShipOwnerData = NULL;

    // Get number of empires
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    unsigned int iNumEmpires;
    Variant* pvKey = NULL;
    iErrCode = t_pConn->ReadColumn (strEmpires, GameEmpires::EmpireKey, &pvKey, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    char strTheirShips [256], strTheirEmpireMap [256];

    IReadTable* pShips = NULL, * pMap = NULL;
    int iType, iTemp;
    unsigned int iNumShips, iTotalShips, * piShipKey = NULL, i, j;

    unsigned int iSlotsAllocated = 1 + 
        min (iNumEmpires + (bIndependence ? 1 : 0), iTotalNumShips) *
        (3 + 2 * min (NUM_SHIP_TYPES, iTotalNumShips));

    unsigned int* piShipOwnerData = new unsigned int [iSlotsAllocated];
    if (piShipOwnerData == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }
    piShipOwnerData[0] = 0;
    unsigned int iCounter = 1;

    const char* pszCurrentPlanetCol = GameEmpireShips::CurrentPlanet;
    const char* pszTypeCol = GameEmpireShips::Type;

    // Scan through all players' ship lists
    for (i = 0; i <= iNumEmpires; i ++) {

        iTotalShips = 0;

        if (i != iNumEmpires) {

            unsigned int iKey;
            GET_GAME_EMPIRE_MAP (strTheirEmpireMap, iGameClass, iGameNumber, pvKey[i].GetInteger());

            iErrCode = t_pConn->GetTableForReading(strTheirEmpireMap, &pMap);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (pvKey[i].GetInteger() == iEmpireKey && iPlanetProxyKey != NO_KEY) {
                iKey = iPlanetProxyKey;
            } else {

                iErrCode = pMap->GetFirstKey (GameEmpireMap::PlanetKey, iPlanetKey, &iKey);
                if (iErrCode != ERROR_DATA_NOT_FOUND && iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }

            if (iKey == NO_KEY) {

                iErrCode = OK;  // Not an error

            } else {

                iErrCode = pMap->ReadData (iKey, GameEmpireMap::NumUncloakedShips, &iTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                iTotalShips = iTemp;

                if (iEmpireKey == pvKey[i].GetInteger() || iEmpireKey == SYSTEM) {

                    iErrCode = pMap->ReadData (iKey, GameEmpireMap::NumCloakedShips, &iTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iTotalShips += iTemp;

                    iErrCode = pMap->ReadData (iKey, GameEmpireMap::NumCloakedBuildShips, &iTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iTotalShips += iTemp;
                }

                if (bVisibleBuilds || iEmpireKey == pvKey[i]) {

                    iErrCode = pMap->ReadData (iKey, GameEmpireMap::NumUncloakedBuildShips, &iTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    iTotalShips += iTemp;
                }

                if (iTotalShips > 0) {
                    GET_GAME_EMPIRE_SHIPS (strTheirShips, iGameClass, iGameNumber, pvKey[i].GetInteger());
                }
            }

            SafeRelease (pMap);

        } else if (bIndependence) {

            GET_GAME_INDEPENDENT_SHIPS (strTheirShips, iGameClass, iGameNumber);

            iErrCode = t_pConn->GetNumRows (strTheirShips, &iTotalShips);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            } else if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            pszCurrentPlanetCol = GameIndependentShips::CurrentPlanet;
            pszTypeCol = GameIndependentShips::Type;
        }

        if (iTotalShips > 0) {

            unsigned int iNumDisplayShips = 0;
            
            iErrCode = t_pConn->GetTableForReading(strTheirShips, &pShips);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Scan through ship list
            iErrCode = pShips->GetEqualKeys(pszCurrentPlanetCol, iPlanetKey, &piShipKey, &iNumShips);
            if (iErrCode != OK) {

                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    iErrCode = OK;
                } else {
                    Assert (false);
                    goto Cleanup;
                }

            } else {

                unsigned int piIndex [NUM_SHIP_TYPES];
                memset (piIndex, 0, NUM_SHIP_TYPES * sizeof (unsigned int));

                for (j = 0; j < iNumShips; j ++) {           
                    
                    bool bDisplay = true;

                    // Read ship type
                    iErrCode = pShips->ReadData (piShipKey[j], pszTypeCol, &iType);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    // Invisible builds?
                    if (i != iNumEmpires && !bVisibleBuilds && pvKey[i].GetInteger() != iEmpireKey) {
                        
                        iErrCode = pShips->ReadData (piShipKey[j], GameEmpireShips::BuiltThisUpdate, &iTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        if (iTemp != 0) {
                            bDisplay = false;
                        }
                    }

                    // If ship is cloaked and doesn't belong to empire, don't show it
                    if (i != iNumEmpires && bDisplay && pvKey[i].GetInteger() != iEmpireKey) {

                        iErrCode = pShips->ReadData (piShipKey[j], GameEmpireShips::State, &iTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        if (iTemp & CLOAKED) {
                            bDisplay = false;
                        }
                    }
                    
                    if (bDisplay) {
                        iNumDisplayShips ++;
                        piIndex [iType] ++;
                    }
                
                }   // End ships on planet loop

                if (iNumDisplayShips > 0) {

                    // Increment number of owners
                    piShipOwnerData[0] ++;

                    // Write owner's key
                    Assert (iCounter < iSlotsAllocated);
                    piShipOwnerData[iCounter] = (i == iNumEmpires) ? INDEPENDENT : pvKey[i].GetInteger();
                    iCounter ++;

                    // Write number of ships owner has
                    Assert (iCounter < iSlotsAllocated);
                    piShipOwnerData[iCounter] = iNumDisplayShips;
                    iCounter ++;

                    // Start off type count at zero
                    unsigned int iTypeCountIndex = iCounter;

                    Assert (iCounter < iSlotsAllocated);
                    piShipOwnerData[iCounter] = 0;
                    iCounter ++;

                    ENUMERATE_SHIP_TYPES (iType) {

                        if (piIndex[iType] > 0) {

                            // Increment type count
                            piShipOwnerData[iTypeCountIndex] ++;

                            // Add type and count
                            Assert (iCounter < iSlotsAllocated);
                            piShipOwnerData[iCounter] = iType;
                            iCounter ++;

                            Assert (iCounter < iSlotsAllocated);
                            piShipOwnerData[iCounter] = piIndex[iType];
                            iCounter ++;
                        }
                    }
                }

                t_pConn->FreeKeys(piShipKey);
                piShipKey = NULL;

            }   // End if ships on planet

            SafeRelease (pShips);

        }   // End if empire has ships

    }   // End empire loop

    *ppiShipOwnerData = piShipOwnerData;
    piShipOwnerData = NULL;

Cleanup:

    SafeRelease (pShips);
    SafeRelease (pMap);

    if (piShipOwnerData != NULL) {
        delete [] piShipOwnerData;
    }

    if (pvKey != NULL) {
        t_pConn->FreeData(pvKey);
    }

    if (piShipKey != NULL) {
        t_pConn->FreeKeys(piShipKey);
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Key of empire
// iPlanetKey -> Integer key of a planet
// pszNewName -> New name
//
// Updates the name of a system

int GameEngine::RenamePlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                              const char* pszNewName) {
    
    int iErrCode = OK;

    GAME_MAP (strMap, iGameClass, iGameNumber);

    // Make sure empire still owns planet
    Variant vEmpireKey;
    iErrCode = t_pConn->ReadData(strMap, iPlanetKey, GameMap::Owner, &vEmpireKey);

    if (iErrCode == OK && vEmpireKey.GetInteger() == iEmpireKey) {
        return t_pConn->WriteData (strMap, iPlanetKey, GameMap::Name, pszNewName);
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Key of empire
// iPlanetKey -> Integer key of a planet
// iNewMaxPop -> New max pop
//
// Updates the max pop of a planet

// Note - not idempotent on failure

int GameEngine::SetPlanetMaxPop (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                                 int iNewMaxPop) {
    
    int iErrCode;
    Variant vTemp;

    GAME_MAP (strMap, iGameClass, iGameNumber);

    // Check for negative input
    if (iNewMaxPop < 0) {
        iNewMaxPop = 0;
    }

    // Make sure empire owns planet
    Variant vEmpireKey;
    iErrCode = t_pConn->ReadData(strMap, iPlanetKey, GameMap::Owner, &vEmpireKey);
    if (iErrCode != OK) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    if (vEmpireKey.GetInteger() != iEmpireKey) {
        return ERROR_PLANET_DOES_NOT_BELONG_TO_EMPIRE;
    }

    // Get old maxpop
    int iOldMaxPop;
    iErrCode = t_pConn->ReadData(strMap, iPlanetKey, GameMap::MaxPop, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iOldMaxPop = vTemp.GetInteger();
    
    // Write new maxpop
    iErrCode = t_pConn->WriteData (strMap, iPlanetKey, GameMap::MaxPop, iNewMaxPop);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Update empire's target pop
    int iPopDiff = iNewMaxPop - iOldMaxPop;
    
    // Update empire targetpop
    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pConn->Increment (strEmpireData, GameEmpireData::TargetPop, iPopDiff);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Update NextTotalPop
    int iPlanetPop, iPop, iAg, iBonusAg, iPopLostToColonies;
    float fMaxAgRatio;

    iErrCode = t_pConn->ReadData(strMap, iPlanetKey, GameMap::Pop, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iPlanetPop = vTemp.GetInteger();

    iErrCode = t_pConn->ReadData(strEmpireData, GameEmpireData::TotalAg, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iAg = vTemp.GetInteger();

    iErrCode = t_pConn->ReadData(strEmpireData, GameEmpireData::BonusAg, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iBonusAg = vTemp.GetInteger();

    iErrCode = t_pConn->ReadData(strEmpireData, GameEmpireData::TotalPop, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iPop = vTemp.GetInteger();

    iErrCode = t_pConn->GetViews()->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    fMaxAgRatio = vTemp.GetFloat();

    // Adjust for colony pop loss
    iErrCode = t_pConn->ReadData(strMap, iPlanetKey, GameMap::PopLostToColonies, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iPopLostToColonies = vTemp.GetInteger();
    Assert (iPopLostToColonies >= 0);
    
    // Calculate next pop
    int iNewPlanetPop = GetNextPopulation (
        iPlanetPop - iPopLostToColonies, 
        GetAgRatio (iAg + iBonusAg, iPop, fMaxAgRatio)
        );

    int iOldNewPlanetPop = iNewPlanetPop;
    
    if (iNewPlanetPop > iNewMaxPop) {
        iNewPlanetPop = iNewMaxPop;
    }
    if (iOldNewPlanetPop > iOldMaxPop) {
        iOldNewPlanetPop = iOldMaxPop;
    }

    iPopDiff = iNewPlanetPop - iOldNewPlanetPop;
    if (iPopDiff != 0) {
        iErrCode = t_pConn->Increment (strEmpireData, GameEmpireData::NextTotalPop, iPopDiff);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // Update next min, fuel changes
    int iPlanetMin, iPlanetFuel;

    iErrCode = t_pConn->ReadData(strMap, iPlanetKey, GameMap::Minerals, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iPlanetMin = vTemp.GetInteger();

    iErrCode = t_pConn->ReadData(strMap, iPlanetKey, GameMap::Fuel, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iPlanetFuel = vTemp.GetInteger();

    int iResDiff = 
        min (iPlanetMin, iOldNewPlanetPop + iPopDiff) - 
        min (iPlanetMin, iOldNewPlanetPop);

    iErrCode = t_pConn->Increment (strEmpireData, GameEmpireData::NextMin, iResDiff);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
        
    iResDiff = 
        min (iPlanetFuel, iOldNewPlanetPop + iPopDiff) - 
        min (iPlanetFuel, iOldNewPlanetPop);

    iErrCode = t_pConn->Increment (strEmpireData, GameEmpireData::NextFuel, iResDiff);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Note: the update algorithm will catch ships trying to deposit pop on planets that don't support it.

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Key of empire
// iPlanetKey -> Integer key of a planet
//
// Output:
// *pbExplored -> True if yes, false if no
//
// Determine if an empire has explored a planet in a game

int GameEngine::HasEmpireExploredPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                                         bool* pbExplored) {

    unsigned int iKey;

    GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pConn->GetFirstKey (
        pszEmpireMap,
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        &iKey
        );

    if (iErrCode == OK) {
        *pbExplored = iKey != NO_KEY;
        return OK;
    }

    else if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return OK;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *piAg -> Average ag for a non-HW planet
// *piMin -> Average ag for a non-HW planet
// *piFuel -> Average ag for a non-HW planet
//
// Return the average number of resources per non-HW planet in a game

int GameEngine::GetGameAveragePlanetResources (int iGameClass, int iGameNumber, int* piAg, int* piMin, 
                                               int* piFuel) {

    int iErrCode;
    Variant vTemp;

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadData(strGameData, GameData::AvgAg, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *piAg = vTemp.GetInteger();

    iErrCode = t_pConn->ReadData(strGameData, GameData::AvgMin, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *piMin = vTemp.GetInteger();

    iErrCode = t_pConn->ReadData(strGameData, GameData::AvgFuel, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *piFuel = vTemp.GetInteger();

    return iErrCode;
}

int GameEngine::GetVisitedSurroundingPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, 
                                                 int iCenterKey, Variant* pvPlanetKey, int* piProxyKey,
                                                 int* piNumPlanets, int* piCenterX, int* piCenterY, 
                                                 int* piMinX, int* piMaxX, int* piMinY, int* piMaxY) {

    int i, iErrCode, iNumPlanets = 0, iX = MIN_COORDINATE, iY = MIN_COORDINATE;

    unsigned int iKey;
    int iGameMapKey;
    Variant vCoordinates;
    char pszSearchCoord [128];

    int piPlanetKey[NUM_CARDINAL_POINTS];
    bool pbPlanetInLine[NUM_CARDINAL_POINTS];

    GAME_MAP (pszGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_MAP (pszGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    IReadTable* pRead = NULL;

    iErrCode = t_pConn->GetTableForReading(pszGameMap, &pRead);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Get planets around center
    iErrCode = pRead->ReadData (iCenterKey, GameMap::NorthPlanetKey, piPlanetKey + NORTH);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pRead->ReadData (iCenterKey, GameMap::EastPlanetKey, piPlanetKey + EAST);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pRead->ReadData (iCenterKey, GameMap::SouthPlanetKey, piPlanetKey + SOUTH);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pRead->ReadData (iCenterKey, GameMap::WestPlanetKey, piPlanetKey + WEST);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pRead);

    // First planet is center
    iErrCode = t_pConn->GetTableForReading(pszGameEmpireMap, &pRead);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pRead->GetFirstKey (GameEmpireMap::PlanetKey, iCenterKey, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    pvPlanetKey[0] = iCenterKey;
    piProxyKey[0] = iKey;
    iNumPlanets = 1;

    // Get planets one jump away
    ENUMERATE_CARDINAL_POINTS(i) {

        pbPlanetInLine[i] = false;

        if (piPlanetKey[i] != NO_KEY) {

            iErrCode = pRead->GetFirstKey (GameEmpireMap::PlanetKey, piPlanetKey[i], &iKey);
            if (iErrCode == OK) {
                pvPlanetKey[iNumPlanets] = piPlanetKey[i];
                piProxyKey[iNumPlanets ++] = iKey;
                pbPlanetInLine[i] = true;
            }
        }
    }

    SafeRelease (pRead);
    
    // Get planet NE of center
    iErrCode = t_pConn->GetTableForReading(pszGameMap, &pRead);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (piPlanetKey[NORTH] != NO_KEY) {
        
        iErrCode = pRead->ReadData (piPlanetKey[NORTH], GameMap::EastPlanetKey, &iGameMapKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    else if (piPlanetKey[EAST] != NO_KEY) {
        
        iErrCode = pRead->ReadData (piPlanetKey[EAST], GameMap::NorthPlanetKey, &iGameMapKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    else {
        
        if (iX == MIN_COORDINATE) {
            
            iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &vCoordinates);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
        }

        GetCoordinates (iX + 1, iY + 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey (
            GameMap::Coordinates, 
            pszSearchCoord, 
            (unsigned int*) &iGameMapKey
            );

        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }

    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY) {
        
        iErrCode = t_pConn->GetFirstKey (pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, &iKey);
        if (iErrCode == OK) {
            pbPlanetInLine[NORTH] = pbPlanetInLine[EAST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }
    
    // Get planet SE of center
    iErrCode = t_pConn->GetTableForReading(pszGameMap, &pRead);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (piPlanetKey[SOUTH] != NO_KEY) {
        
        iErrCode = pRead->ReadData (piPlanetKey[SOUTH], GameMap::EastPlanetKey, &iGameMapKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    else if (piPlanetKey[EAST] != NO_KEY) {
        
        iErrCode = pRead->ReadData (piPlanetKey[EAST], GameMap::SouthPlanetKey, &iGameMapKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    else {
        
        if (iX == MIN_COORDINATE) {
            
            iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &vCoordinates);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
        }
        
        GetCoordinates(iX + 1, iY - 1, pszSearchCoord);
        
        iErrCode = pRead->GetFirstKey(GameMap::Coordinates, pszSearchCoord, (unsigned int*)&iGameMapKey);
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }
    
    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY) {
        
        iErrCode = t_pConn->GetFirstKey (pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, &iKey);
        if (iErrCode == OK) {
            pbPlanetInLine[SOUTH] = pbPlanetInLine[EAST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }

    // Get planet SW of center
    iErrCode = t_pConn->GetTableForReading(pszGameMap, &pRead);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (piPlanetKey[SOUTH] != NO_KEY) {

        iErrCode = pRead->ReadData (piPlanetKey[SOUTH], GameMap::WestPlanetKey, &iGameMapKey);
        if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
    }
    else if (piPlanetKey[WEST] != NO_KEY) {

        iErrCode = pRead->ReadData (piPlanetKey[WEST], GameMap::SouthPlanetKey, &iGameMapKey);
        if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
    }
    else {
        
        if (iX == MIN_COORDINATE) {

            iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &vCoordinates);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
        }

        GetCoordinates (iX - 1, iY - 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey (
            GameMap::Coordinates, 
            pszSearchCoord, 
            (unsigned int*) &iGameMapKey
            );
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }

    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY) {
        
        iErrCode = t_pConn->GetFirstKey (pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, &iKey);
        if (iErrCode == OK) {
            pbPlanetInLine[SOUTH] = pbPlanetInLine[WEST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }
    
    // Get planet NW of center
    iErrCode = t_pConn->GetTableForReading(pszGameMap, &pRead);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (piPlanetKey[NORTH] != NO_KEY) {
        
        iErrCode = pRead->ReadData (piPlanetKey[NORTH], GameMap::WestPlanetKey, &iGameMapKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    else if (piPlanetKey[WEST] != NO_KEY) {
        
        iErrCode = pRead->ReadData (piPlanetKey[WEST], GameMap::NorthPlanetKey, &iGameMapKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    else {
        
        if (iX == MIN_COORDINATE) {
            
            iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &vCoordinates);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
        }
        
        GetCoordinates (iX - 1, iY + 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey(GameMap::Coordinates, pszSearchCoord, (unsigned int*) &iGameMapKey);
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }

    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY) {
        
        iErrCode = t_pConn->GetFirstKey (pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, &iKey);
        if (iErrCode == OK && iKey != NO_KEY) {
            pbPlanetInLine[NORTH] = pbPlanetInLine[WEST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }

    // Retvals
    *piNumPlanets = iNumPlanets;

    if (iX == MIN_COORDINATE) {

        iErrCode = t_pConn->GetTableForReading(pszGameMap, &pRead);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &vCoordinates);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pRead);
        
        GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
    }
    
    *piMinX = pbPlanetInLine[WEST] ? iX - 1 : iX;
    *piMaxX = pbPlanetInLine[EAST] ? iX + 1 : iX;
    *piMinY = pbPlanetInLine[SOUTH] ? iY - 1 : iY;
    *piMaxY = pbPlanetInLine[NORTH] ? iY + 1 : iY;

    *piCenterX = iX;
    *piCenterY = iY;

Cleanup:

    SafeRelease (pRead);

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    
    return iErrCode;
}


int GameEngine::GetLowestDiplomacyLevelForShipsOnPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
                                                         int iPlanetKey, bool bVisibleBuilds, 
                                                         Variant* pvEmpireKey, unsigned int& iNumEmpires,
                                                         int* piNumForeignShipsOnPlanet, int* piDiplomacyLevel,
                                                         Variant** ppvEmpireKey) {

    int iErrCode;

    unsigned int iProxyPlanetKey;

    Variant vTotalNumShips, vNumOurShips, vValue;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    // Get total num ships
    iErrCode = t_pConn->ReadData(
        strGameMap,
        iPlanetKey,
        GameMap::NumUncloakedShips,
        &vTotalNumShips
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Get our ships
    iErrCode = t_pConn->GetFirstKey (
        strGameEmpireMap,
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        &iProxyPlanetKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = t_pConn->ReadData(
        strGameEmpireMap,
        iProxyPlanetKey,
        GameEmpireMap::NumUncloakedShips,
        &vNumOurShips
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    if (bVisibleBuilds) {
        
        iErrCode = t_pConn->ReadData(
            strGameMap,
            iPlanetKey,
            GameMap::NumUncloakedBuildShips,
            &vValue
            );
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        if (vValue.GetInteger() > 0) {
            
            vTotalNumShips += vValue.GetInteger();
            
            iErrCode = t_pConn->ReadData(
                strGameEmpireMap,
                iProxyPlanetKey,
                GameEmpireMap::NumUncloakedBuildShips,
                &vValue
                );
            
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            vNumOurShips += vValue.GetInteger();
        }
    }


    // Easy way out
    if (vTotalNumShips.GetInteger() == vNumOurShips.GetInteger()) {

        *piNumForeignShipsOnPlanet = 0;
        *piDiplomacyLevel = WAR;

        return OK;
    }
    
    *piNumForeignShipsOnPlanet = vTotalNumShips.GetInteger() - vNumOurShips.GetInteger();
    
    // Loop through all other empires (sigh)
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    char strTheirMap [256];
    
    unsigned int i, iProxyDipKey;
    Variant vDipLevel;

    bool bLowestDipSet = false;
    int iLowestDip = ALLIANCE;
    
    if (pvEmpireKey == NULL) {

        iErrCode = t_pConn->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, &pvEmpireKey, &iNumEmpires);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        *ppvEmpireKey = pvEmpireKey;
    }

    for (i = 0; i < iNumEmpires; i ++) {
        
        if (pvEmpireKey[i].GetInteger() == iEmpireKey) {
            continue;
        }
        
        GET_GAME_EMPIRE_MAP (strTheirMap, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        iErrCode = t_pConn->GetFirstKey (
            strTheirMap,
            GameEmpireMap::PlanetKey,
            iPlanetKey,
            &iProxyPlanetKey
            );
        
        if (iErrCode != OK) {

            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                continue;
            }

            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = t_pConn->ReadData(
            strTheirMap,
            iProxyPlanetKey,
            GameEmpireMap::NumUncloakedShips,
            &vNumOurShips
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        if (bVisibleBuilds) {

            iErrCode = t_pConn->ReadData(
                strTheirMap,
                iProxyPlanetKey,
                GameEmpireMap::NumUncloakedBuildShips,
                &vValue
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            vNumOurShips += vValue.GetInteger();
        }
        
        if (vNumOurShips.GetInteger() > 0) {

            // Get dip
            iErrCode = t_pConn->GetFirstKey (
                strGameEmpireDiplomacy,
                GameEmpireDiplomacy::EmpireKey,
                pvEmpireKey[i].GetInteger(),
                &iProxyDipKey
                );

            if (iErrCode == OK) {

                iErrCode = t_pConn->ReadData(
                    strGameEmpireDiplomacy,
                    iProxyDipKey,
                    GameEmpireDiplomacy::CurrentStatus,
                    &vDipLevel
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

            } else {

                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    vDipLevel = WAR;
                } else {
                    Assert (false);
                    goto Cleanup;
                }
            }

            if (vDipLevel.GetInteger() <= iLowestDip) {
                iLowestDip = vDipLevel.GetInteger();
                bLowestDipSet = true;
            }
        }
    }

    if (!bLowestDipSet && (*piNumForeignShipsOnPlanet) > 0) {
        // Must be independent ships
        iLowestDip = WAR;
    }

    *piDiplomacyLevel = iLowestDip;

Cleanup:

    return iErrCode;
}


int GameEngine::SetNewMinMaxIfNecessary (int iGameClass, int iGameNumber, int iEmpireKey, int iX, int iY) {

    int iErrCode;
    char pszTable [256];

    Variant vMinX, vMaxX, vMinY, vMaxY;
    const char* iMinXCol, * iMaxXCol, * iMinYCol, * iMaxYCol;

    if (iEmpireKey == NO_KEY) {

        GET_GAME_DATA (pszTable, iGameClass, iGameNumber);

        iMinXCol = GameData::MinX;
        iMaxXCol = GameData::MaxX;
        iMinYCol = GameData::MinY;
        iMaxYCol = GameData::MaxY;

    } else {

        GET_GAME_EMPIRE_DATA (pszTable, iGameClass, iGameNumber, iEmpireKey);

        iMinXCol = GameEmpireData::MinX;
        iMaxXCol = GameEmpireData::MaxX;
        iMinYCol = GameEmpireData::MinY;
        iMaxYCol = GameEmpireData::MaxY;
    }

    iErrCode = t_pConn->ReadData(pszTable, iMinXCol, &vMinX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = t_pConn->ReadData(pszTable, iMaxXCol, &vMaxX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = t_pConn->ReadData(pszTable, iMinYCol, &vMinY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = t_pConn->ReadData(pszTable, iMaxYCol, &vMaxY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iX < vMinX.GetInteger()) {
        iErrCode = t_pConn->WriteData (pszTable, iMinXCol, iX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (iX > vMaxX.GetInteger()) {
        iErrCode = t_pConn->WriteData (pszTable, iMaxXCol, iX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (iY < vMinY.GetInteger()) {
        iErrCode = t_pConn->WriteData (pszTable, iMinYCol, iY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (iY > vMaxY.GetInteger()) {
        iErrCode = t_pConn->WriteData (pszTable, iMaxYCol, iY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    return iErrCode;
}

int GameEngine::GetPlanetPopulationWithColonyBuilds (unsigned int iGameClass, unsigned int iGameNumber,
                                                     unsigned int iEmpireKey, unsigned int iPlanetKey,
                                                     unsigned int* piPop) {

    int iErrCode, iPop, iCost;
    IReadTable* pTable = NULL;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    iErrCode = t_pConn->GetTableForReading(strGameMap, &pTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (iPlanetKey, GameMap::Pop, &iPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pTable->ReadData (iPlanetKey, GameMap::PopLostToColonies, &iCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pTable);

    Assert (iPop >= 0);
    Assert (iCost >= 0);
    iPop -= iCost;
    Assert (iPop >= 0);

    *piPop = iPop;

Cleanup:

    SafeRelease (pTable);

    return iErrCode;
}

/*
int GameEngine::GetSpectatorGameInfo (int iGameClass, int iGameNumber, 
                                      unsigned int** ppiPlanetKey, unsigned int* piNumPlanets, 
                                      int* piMapMinX, int* piMapMaxX, int* piMapMinY, int* piMapMaxY) {
    int iErrCode;

    Variant vGameClassOptions, * pvEmpireKey = NULL, vGameOptions, vGameState, * pvPlanetKey = NULL;
    IReadTable* pReadTable = NULL, * pEmpMap = NULL;

    *ppiPlanetKey = NULL;
    *piNumPlanets = 0;
    *piMapMinX = *piMapMaxX = *piMapMinY = *piMapMaxY = 0;

    GAME_MAP (pszGameMap, iGameClass, iGameNumber);
    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    // Verify game
    iErrCode = t_pConn->ReadData(pszGameData, GameData::Options, &vGameOptions);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!(vGameOptions.GetInteger() & GAME_ALLOW_SPECTATORS)) {
        Assert (false);
        return ERROR_NOT_SPECTATOR_GAME;
    }

    iErrCode = t_pConn->ReadData(pszGameData, GameData::State, &vGameState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (vGameState.GetInteger() & STILL_OPEN) {
        Assert (false);
        return ERROR_GAME_HAS_NOT_CLOSED;
    }

    // Get gameclass options
    iErrCode = t_pConn->GetViews()->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::iOptions, &vGameClassOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vGameClassOptions.GetInteger() & EXPOSED_MAP) {

        // Easy way out
        iErrCode = t_pConn->GetAllKeys (pszGameMap, ppiPlanetKey, piNumPlanets);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pConn->GetTableForReading(pszGameData, &pReadTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pReadTable->ReadData (GameData::MinX, piMapMinX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pReadTable->ReadData (GameData::MaxX, piMapMaxX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pReadTable->ReadData (GameData::MinY, piMapMinY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pReadTable->ReadData (GameData::MaxY, piMapMaxY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        pReadTable->Release();
        pReadTable = NULL;
    
    } else {

        unsigned int i, j, iNumEmpires, iMinNumPlanets = INT_MAX, iNumPlanets, iMinNumPlanetsEmpire = NO_KEY,
            iEmpireKey, iKey;

        char pszEmpireMap [256];

        int iMinX = MAX_COORDINATE, iMaxX = MIN_COORDINATE, iMinY = MAX_COORDINATE, iMaxY = MIN_COORDINATE;

        // Get all empires
        GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

        iErrCode = t_pConn->ReadColumn (
            pszEmpires,
            GameEmpires::EmpireKey,
            &pvEmpireKey,
            &iNumEmpires
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Loop through all empires, and find empire with least number of planets
        for (i = 0; i < iNumEmpires; i ++) {
            
            GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
            
            iErrCode = t_pConn->GetNumRows (pszEmpireMap, &iNumPlanets);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (iNumPlanets < iMinNumPlanets) { 
                iMinNumPlanets = iNumPlanets;
                iMinNumPlanetsEmpire = pvEmpireKey[i].GetInteger();
            }
        }

        Assert (iMinNumPlanets != INT_MAX && iMinNumPlanetsEmpire != NO_KEY);

        // Get all planet keys from min empire
        GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iMinNumPlanetsEmpire);

        iErrCode = t_pConn->ReadColumn (pszEmpireMap, GameEmpireMap::PlanetKey, &pvPlanetKey, &iNumPlanets);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Loop through all empires, 
        for (i = 0; i < iNumEmpires; i ++) {
            
            iEmpireKey = pvEmpireKey[i].GetInteger();
            if (iEmpireKey == iMinNumPlanetsEmpire) {
                continue;
            }
            
            GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);
            
            iErrCode = t_pConn->GetTableForReading(pszEmpireMap, &pEmpMap);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            // Loop through all planets
            for (j = 0; j < iNumPlanets; j ++) {
                
                iErrCode = pEmpMap->GetFirstKey (
                    GameEmpireMap::PlanetKey, 
                    pvPlanetKey[j].GetInteger(),
                    &iKey
                    );
                
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    
                    Assert (iKey == NO_KEY);
                    
                    // Throw away the key
                    iNumPlanets --;
                    pvPlanetKey[j] = pvPlanetKey[iNumPlanets];
                    j --;
                    break;
                }
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                // Keep the key for now
            }

            pEmpMap->Release();
            pEmpMap = NULL;
        }

        if (iNumPlanets == 0) { 
            iErrCode = OK;
            goto Cleanup;
        }

        Assert (pReadTable == NULL);

        *piNumPlanets = iNumPlanets;
        *ppiPlanetKey = (unsigned int*) OS::HeapAlloc (iNumPlanets * sizeof (unsigned int));
        if (*ppiPlanetKey == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        iErrCode = t_pConn->GetTableForReading(pszGameMap, &pReadTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Find min-max
        for (i = 0; i < iNumPlanets; i ++) {

            const char* pszCoord;
            int iX, iY;

            (*ppiPlanetKey)[i] = pvPlanetKey[i].GetInteger();

            iErrCode = pReadTable->ReadData (pvPlanetKey[i].GetInteger(), GameMap::Coordinates, &pszCoord);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            GetCoordinates (pszCoord, &iX, &iY);

            if (iX < iMinX) {
                iMinX = iX;
            }

            if (iX > iMaxX) {
                iMaxX = iX;
            }

            if (iY < iMinY) {
                iMinY = iY;
            }

            if (iY > iMaxY) {
                iMaxY = iY;
            }
        }

        pReadTable->Release();
        pReadTable = NULL;

        *piMapMinX = iMinX;
        *piMapMaxX = iMaxX;
        *piMapMinY = iMinY;
        *piMapMaxY = iMaxY;
    }

Cleanup:

    if (pReadTable != NULL) {
        pReadTable->Release();
    }

    if (pEmpMap != NULL) {
        pEmpMap->Release();
    }

    if (pvEmpireKey != NULL) {
        t_pConn->FreeData(pvEmpireKey);
    }

    if (pvPlanetKey != NULL) {
        t_pConn->FreeData(pvPlanetKey);
    }

    if ((iErrCode != OK || *piNumPlanets == 0) && (*ppiPlanetKey) != NULL) {
        t_pConn->FreeKeys(*ppiPlanetKey);
        *ppiPlanetKey = NULL;
    }
    
    return iErrCode;
}

int GameEngine::IsPlanetSpectatorVisible (int iGameClass, int iGameNumber, int iPlanetKey, bool* pbVisible) {

    int iErrCode;
    unsigned int iKey = NO_KEY, iProxyKey;

    char pszEmpireMap [256];
    
    Variant vEmpireKey;

    GAME_MAP (pszGameMap, iGameClass, iGameNumber);

    iErrCode = t_pConn->DoesRowExist (pszGameMap, iPlanetKey, pbVisible);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!(*pbVisible)) {
        return OK;
    }
    
    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    while (true) {

        iErrCode = t_pConn->GetNextKey (pszEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            return OK;
        }

        iErrCode = t_pConn->ReadData(pszEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, vEmpireKey.GetInteger());

        iErrCode = t_pConn->GetFirstKey (
            pszEmpireMap, 
            GameEmpireMap::PlanetKey,
            iPlanetKey,
            false,
            &iProxyKey
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            *pbVisible = false;
            return OK;
        }
    }

    // Set to true
    return OK;
}
*/