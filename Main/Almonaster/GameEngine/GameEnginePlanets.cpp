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

int GameEngine::GetPlanetCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, int* piPlanetX, int* piPlanetY) {

    Variant vTemp;
    GET_GAME_MAP (pszMap, iGameClass, iGameNumber);

    int iErrCode = t_pCache->ReadData(pszMap, iPlanetKey, GameMap::Coordinates, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    GetCoordinates (vTemp.GetCharPtr(), piPlanetX, piPlanetY);
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

int GameEngine::GetPlanetProperty(int iGameClass, int iGameNumber, unsigned int iPlanetKey, const char* pszProperty, Variant* pvProperty)
{
    GET_GAME_MAP (pszMap, iGameClass, iGameNumber);
    return t_pCache->ReadData(pszMap, iPlanetKey, pszProperty, pvProperty);
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

int GameEngine::GetPlanetKeyFromCoordinates (int iGameClass, int iGameNumber, int iX, int iY, int* piPlanetKey)
{
    *piPlanetKey = NO_KEY;

    char pszCoord [MAX_COORDINATE_LENGTH + 1];
    GetCoordinates(iX, iY, pszCoord);

    GET_GAME_MAP (pszMap, iGameClass, iGameNumber);
    unsigned int iKey;
    int iErrCode = t_pCache->GetFirstKey(pszMap, GameMap::Coordinates, pszCoord, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);
            
    *piPlanetKey = iKey;
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

    iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::AlmonasterTheme, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    
    if (vTemp.GetInteger() == INDIVIDUAL_ELEMENTS)
    {
        iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::UILivePlanet, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        *piLivePlanetKey = vTemp.GetInteger();
        
        iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::UIDeadPlanet, &vTemp);
        RETURN_ON_ERROR(iErrCode);
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


int GameEngine::AddEmpiresToMap(int iGameClass, int iGameNumber, int* piEmpireKey, int iNumEmpires, GameFairnessOption gfoFairness)
{
    int iErrCode, i;

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    Variant** ppvPlanetData = NULL, * pvGameClassData = NULL, * pvGameData = NULL, ** ppvNewPlanetData = NULL, vTotalAg;
    AutoFreeData free1(pvGameClassData);
    AutoFreeData free2(pvGameData);
    AutoFreeData free3(ppvPlanetData);
    AutoFreeData free4(ppvNewPlanetData);

    unsigned int* piPlanetKey = NULL, iNumPlanets = 0, iNumNewPlanets;
    AutoFreeKeys freeKeys(piPlanetKey);

    IMapGenerator* pMapGen = NULL, * pInner = NULL;
    Algorithm::AutoDelete<IMapGenerator> auto1(pMapGen, false);
    Algorithm::AutoDelete<IMapGenerator> auto2(pInner, false);

    // Read gameclass data
    iErrCode = t_pCache->ReadRow (SYSTEM_GAMECLASS_DATA, iGameClass, &pvGameClassData);
    RETURN_ON_ERROR(iErrCode);

    // Read game data
    iErrCode = t_pCache->ReadRow(strGameData, NO_KEY, &pvGameData);
    RETURN_ON_ERROR(iErrCode);
    int iGameOptions = pvGameData[GameData::iOptions].GetInteger();

    // Create a new map generator
    if (iGameOptions & GAME_MIRRORED_MAP) {
        pMapGen = new MirroredMapGenerator(this);
    } else if (iGameOptions & GAME_TWISTED_MAP) {
        pMapGen = new TwistedMapGenerator(this);
    } else {
        pMapGen = new DefaultMapGenerator(this);
    }
    Assert(pMapGen);

    // Wrap map generator in a fair map generator
    pInner = pMapGen;
    pMapGen = new FairMapGenerator(pInner, gfoFairness);

    // Get existing map
    iErrCode = t_pCache->ReadColumns (
        strGameMap,
        GameMap::NumColumns,
        GameMap::ColumnNames,
        &piPlanetKey,
        &ppvPlanetData,
        &iNumPlanets
        );
    
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

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
    RETURN_ON_ERROR(iErrCode);

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
        iNumPlanets
        );

    RETURN_ON_ERROR(iErrCode);

    // Handle 'next statistics'
    for (i = 0; i < iNumEmpires; i ++)
    {
        GET_GAME_EMPIRE_DATA(pszEmpireData, iGameClass, iGameNumber, piEmpireKey[i]);
        iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::TotalAg, &vTotalAg);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = WriteNextStatistics(
            iGameClass, 
            iGameNumber, 
            piEmpireKey[i],
            vTotalAg.GetInteger(), 
            0, // No trades or allies at beginning
            pvGameClassData[SystemGameClassData::iMaxAgRatio].GetFloat()
            );

        RETURN_ON_ERROR(iErrCode);
    }

#ifdef _DEBUG
    // Verify map
    iErrCode = VerifyMap (iGameClass, iGameNumber);
    RETURN_ON_ERROR(iErrCode);
#endif

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
                                              unsigned int iNumExistingPlanets)
{
    int iErrCode;

    unsigned int i, j;

    int iGameClassOptions, * piNewPlanetKey = NULL, 
        iHomeWorldIndex, iHomeWorldKey, iMinX, iMinY, iMaxX, iMaxY,
        iGameMinX, iGameMaxX, iGameMinY, iGameMaxY;

    Variant* pvEmpireKey = NULL;
    AutoFreeData free(pvEmpireKey);

    ICachedTable* pWrite = NULL;
    AutoRelease<ICachedTable> rel(pWrite);

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    // Transfer new game data to the GameData table
    if (pvGameData != NULL)
    {
        iErrCode = t_pCache->GetTable(strGameData, &pWrite);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pWrite->WriteData(GameData::NumPlanetsPerEmpire, pvGameData[GameData::iNumPlanetsPerEmpire]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pWrite->WriteData(GameData::HWAg, pvGameData[GameData::iHWAg]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pWrite->WriteData(GameData::HWMin, pvGameData[GameData::iHWMin]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pWrite->WriteData(GameData::HWFuel, pvGameData[GameData::iHWFuel]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pWrite->WriteData(GameData::AvgAg, pvGameData[GameData::iAvgAg]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pWrite->WriteData(GameData::AvgMin, pvGameData[GameData::iAvgMin]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pWrite->WriteData(GameData::AvgFuel, pvGameData[GameData::iAvgFuel]);
        RETURN_ON_ERROR(iErrCode);
    }

    // Initialize some vars
    iGameClassOptions = pvGameClassData[SystemGameClassData::iOptions].GetInteger();

    iMinX = iMinY = MAX_COORDINATE;
    iMaxX = iMaxY = MIN_COORDINATE;

    iHomeWorldKey = iHomeWorldIndex = NO_KEY;

    piNewPlanetKey = (int*)StackAlloc(iNumNewPlanets * sizeof(int));
    memset (piNewPlanetKey, NO_KEY, iNumNewPlanets * sizeof(int));

    // Perform new planet insertions
    for (i = 0; i < iNumNewPlanets; i ++)
    {
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

        RETURN_ON_ERROR(iErrCode);

        if (iHomeWorldKey != NO_KEY && iHomeWorldIndex == NO_KEY)
        {
            iHomeWorldIndex = i;
        }
    }

    //
    // Fix up min, max for gamedata table
    //

    SafeRelease(pWrite);
    iErrCode = t_pCache->GetTable(strGameData, &pWrite);
    RETURN_ON_ERROR(iErrCode);

    // MinX
    iErrCode = pWrite->ReadData(GameData::MinX, &iGameMinX);
    RETURN_ON_ERROR(iErrCode);
    
    if (iMinX < iGameMinX) {
        
        iErrCode = pWrite->WriteData(GameData::MinX, iMinX);
        RETURN_ON_ERROR(iErrCode);
    }

    // MaxX
    iErrCode = pWrite->ReadData(GameData::MaxX, &iGameMaxX);
    RETURN_ON_ERROR(iErrCode);
    
    if (iMaxX > iGameMaxX) {
        
        iErrCode = pWrite->WriteData(GameData::MaxX, iMaxX);
        RETURN_ON_ERROR(iErrCode);
    }

    // MinY
    iErrCode = pWrite->ReadData(GameData::MinY, &iGameMinY);
    RETURN_ON_ERROR(iErrCode);
    
    if (iMinY < iGameMinY) {
        
        iErrCode = pWrite->WriteData(GameData::MinY, iMinY);
        RETURN_ON_ERROR(iErrCode);
    }

    // MaxY
    iErrCode = pWrite->ReadData(GameData::MaxY, &iGameMaxY);
    RETURN_ON_ERROR(iErrCode);
    
    if (iMaxY > iGameMaxY) {
        
        iErrCode = pWrite->WriteData(GameData::MaxY, iMaxY);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iGameClassOptions & EXPOSED_MAP)
    {
        // Insert all new planets into every empires' map
        unsigned int iTotalNumEmpires;
        iErrCode = t_pCache->ReadColumn(
            strGameEmpires,
            GameEmpires::EmpireKey,
            NULL,
            &pvEmpireKey,
            &iTotalNumEmpires
            );
        
        RETURN_ON_ERROR(iErrCode);

        for (i = 0; i < iNumNewPlanets; i ++)
        {
            for (j = 0; j < iTotalNumEmpires; j ++)
            {
                iErrCode = InsertPlanetIntoGameEmpireData(
                    iGameClass,
                    iGameNumber,
                    pvEmpireKey[j].GetInteger(),
                    piNewPlanetKey[i],
                    ppvNewPlanetData[i],
                    iGameClassOptions
                    );

                RETURN_ON_ERROR(iErrCode);
            }
        }

        // Insert all old planets into each new empires' map
        for (i = 0; i < iNumExistingPlanets; i ++)
        {
            for (j = 0; j < iNumNewEmpires; j ++)
            {
                iErrCode = InsertPlanetIntoGameEmpireData(
                    iGameClass,
                    iGameNumber,
                    piNewEmpireKey[j],
                    piExistingPlanetKey[i],
                    ppvExistingPlanetData[i],
                    iGameClassOptions
                    );

                RETURN_ON_ERROR(iErrCode);
            }
        }
    }
    else
    {
        //
        // Not exposed maps
        //

        // Add new planets to their owner's map if they're homeworlds or if the map is fully colonized
        for (i = 0; i < iNumNewPlanets; i ++)
        {
            int iOwner = ppvNewPlanetData[i][GameMap::iOwner].GetInteger();
            if (iOwner != SYSTEM)
            {
                bool bHW = ppvNewPlanetData[i][GameMap::iHomeWorld].GetInteger() == HOMEWORLD;
                if (bHW || (iGameClassOptions & FULLY_COLONIZED_MAP))
                {
                    Assert(iOwner != SYSTEM);

                    iErrCode = InsertPlanetIntoGameEmpireData(
                        iGameClass,
                        iGameNumber,
                        iOwner,
                        piNewPlanetKey[i],
                        ppvNewPlanetData[i],
                        iGameClassOptions
                        );

                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }
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
                                                  )
{
    int iErrCode = OK, iX, iY, iNewX, iNewY, iLink, cpDir;

    unsigned int iKey;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    char pszCoord [MAX_COORDINATE_LENGTH + 1];

    unsigned int piNeighbourKey [NUM_CARDINAL_POINTS];
    int piNeighbourDirection [NUM_CARDINAL_POINTS];
    bool pbLink [NUM_CARDINAL_POINTS];

    unsigned int i, iNumNeighbours;

    ICachedTable* pWrite = NULL;
    AutoRelease<ICachedTable> rel(pWrite);

    Assert(iEmpireKey != NO_KEY);

    bool bHomeWorld = pvPlanetData[GameMap::iHomeWorld].GetInteger() == HOMEWORLD;

    // Name
    if (bHomeWorld)
    {
        GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Name, pvPlanetData + GameMap::iName);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        char pszPlanetName [MAX_PLANET_NAME_LENGTH + 1];
        sprintf(pszPlanetName, "Planet %s", pvPlanetData[GameMap::iCoordinates].GetCharPtr());
        pvPlanetData[GameMap::iName] = pszPlanetName;
        Assert(pvPlanetData[GameMap::iName].GetCharPtr());
    }

    //
    // Ag, Minerals, Fuel, Coordinates, Link, HomeWorld are set by generator
    //

    // Pop, MaxPop, Owner
    int iMin = pvPlanetData[GameMap::iMinerals].GetInteger();
    int iFuel = pvPlanetData[GameMap::iFuel].GetInteger();

    int iMaxPop = max(iMin, iFuel);
    if (iMaxPop == 0)
    {
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

        iErrCode = t_pCache->GetFirstKey(
            strGameMap,
            GameMap::Coordinates,
            pszCoord,
            &iKey
            );
        
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            // Can't assert here because linked planets may not be in map yet
            // Assert(!(iLink & LINK_X[cpDir]));
            pvPlanetData[GameMap::iNorthPlanetKey + cpDir] = NO_KEY;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);
            
            Assert(iKey != NO_KEY);
            
            pvPlanetData[GameMap::iNorthPlanetKey + cpDir] = iKey;

            piNeighbourKey[iNumNeighbours] = iKey;
            piNeighbourDirection[iNumNeighbours] = cpDir;       
            pbLink[iNumNeighbours] = (iLink & LINK_X[cpDir]) != 0;
            
            iNumNeighbours ++;
        }
    }

    // Insert the planet, finally!
    iErrCode = t_pCache->InsertRow(strGameMap, GameMap::Template, pvPlanetData, &iKey);
    RETURN_ON_ERROR(iErrCode);

    *piNewPlanetKey = iKey;

    // Fix up neighboring links
    for (i = 0; i < iNumNeighbours; i ++)
    {
        iErrCode = t_pCache->WriteData(
            strGameMap,
            piNeighbourKey[i],
            GameMap::ColumnNames[GameMap::iNorthPlanetKey + OPPOSITE_CARDINAL_POINT [piNeighbourDirection[i]]],
            (int) iKey
            );

        RETURN_ON_ERROR(iErrCode);

        if (pbLink[i])
        {
            iErrCode = t_pCache->WriteOr(strGameMap, piNeighbourKey[i], GameMap::Link,OPPOSITE_LINK_X [piNeighbourDirection[i]]);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (iEmpireKey != SYSTEM && (bHomeWorld || (iGameClassOptions & FULLY_COLONIZED_MAP)))
    {
        // Increment some empire statistics
        GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

        Assert(pWrite == NULL);
        iErrCode = t_pCache->GetTable(strGameEmpireData, &pWrite);
        RETURN_ON_ERROR(iErrCode);

        // NumPlanets
        iErrCode = pWrite->Increment(GameEmpireData::NumPlanets, 1);
        RETURN_ON_ERROR(iErrCode);

        // TotalAg
        if (iAg > 0)
        {
            iErrCode = pWrite->Increment(GameEmpireData::TotalAg, iAg);
            RETURN_ON_ERROR(iErrCode);
        }

        // TotalMin
        iErrCode = pWrite->Increment(GameEmpireData::TotalMin, min (iMin, iPop));
        RETURN_ON_ERROR(iErrCode);

        // TotalFuel
        iErrCode = pWrite->Increment(GameEmpireData::TotalFuel, min (iFuel, iPop));
        RETURN_ON_ERROR(iErrCode);

        // TotalPop
        if (iPop > 0)
        {
            iErrCode = pWrite->Increment(GameEmpireData::TotalPop, iPop);
            RETURN_ON_ERROR(iErrCode);
        }

        // TargetPop
        iErrCode = pWrite->Increment(GameEmpireData::TargetPop, iMaxPop);
        RETURN_ON_ERROR(iErrCode);

        // HomeWorld
        if (bHomeWorld)
        {
            iErrCode = pWrite->WriteData(GameEmpireData::HomeWorld, (int)iKey);
            RETURN_ON_ERROR(iErrCode);
        }

        // TotalAg
        iErrCode = pWrite->ReadData(GameEmpireData::TotalAg, &iAg);
        RETURN_ON_ERROR(iErrCode);

        // TotalMin
        iErrCode = pWrite->ReadData(GameEmpireData::TotalMin, &iMin);
        RETURN_ON_ERROR(iErrCode);

        // TotalFuel
        iErrCode = pWrite->ReadData(GameEmpireData::TotalFuel, &iFuel);
        RETURN_ON_ERROR(iErrCode);

        // Update Econ
        iErrCode = pWrite->WriteData(GameEmpireData::Econ, GetEcon(iFuel, iMin, iAg));
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::InsertPlanetIntoGameEmpireData(int iGameClass, int iGameNumber, int iEmpireKey, 
                                               int iPlanetKey, const Variant* pvPlanetData, int iGameClassOptions)
{
    int iErrCode;

    GET_GAME_MAP(pszGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_MAP(pszGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DATA(pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    Variant pvOldPlanet[GameEmpireMap::NumColumns];

    ICachedTable* pEmpireData = NULL;
    AutoRelease<ICachedTable> rel(pEmpireData);

    Assert(iEmpireKey != SYSTEM && iEmpireKey != NO_KEY);
    Assert(iPlanetKey != NO_KEY);

    pvOldPlanet[GameEmpireMap::iGameClass] = iGameClass;
    pvOldPlanet[GameEmpireMap::iGameNumber] = iGameNumber;
    pvOldPlanet[GameEmpireMap::iEmpireKey] = iEmpireKey;
    pvOldPlanet[GameEmpireMap::iPlanetKey] = iPlanetKey;
    pvOldPlanet[GameEmpireMap::iNumUncloakedShips] = 0;
    pvOldPlanet[GameEmpireMap::iNumCloakedBuildShips] = 0;
    pvOldPlanet[GameEmpireMap::iNumUncloakedBuildShips] = 0;
    pvOldPlanet[GameEmpireMap::iNumCloakedShips] = 0;
    
    Variant vKey;
    int iKey, iExplored = 0, cpDir;
    unsigned int iProxyKey;

    if ((iGameClassOptions & EXPOSED_MAP) || (iGameClassOptions & FULLY_COLONIZED_MAP))
    {
        ENUMERATE_CARDINAL_POINTS (cpDir)
        {
            iErrCode = t_pCache->ReadData(pszGameMap, iPlanetKey, GameMap::ColumnNames[GameMap::iNorthPlanetKey + cpDir], &vKey);
            RETURN_ON_ERROR(iErrCode);

            iKey = vKey.GetInteger();
            if (iKey != NO_KEY)
            {
                iErrCode = t_pCache->GetFirstKey(
                    pszGameEmpireMap,
                    GameEmpireMap::PlanetKey,
                    iKey,
                    &iProxyKey
                    );
                
                if (iErrCode == ERROR_DATA_NOT_FOUND)
                {
                    iErrCode = OK;
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->WriteOr(pszGameEmpireMap, iProxyKey, GameEmpireMap::Explored, OPPOSITE_EXPLORED_X[cpDir]);
                    RETURN_ON_ERROR(iErrCode);

                    iExplored |= EXPLORED_X[cpDir];
                }
            }
        }
    }
    
    pvOldPlanet [GameEmpireMap::iExplored] = iExplored;
    
    iErrCode = t_pCache->InsertRow(pszGameEmpireMap, GameEmpireMap::Template, pvOldPlanet, NULL);
    RETURN_ON_ERROR(iErrCode);

    //
    // Handle max/min settings
    //
    int iMinX, iMaxX, iMinY, iMaxY;

    Assert(pEmpireData == NULL);
    iErrCode = t_pCache->GetTable(pszGameEmpireData, &pEmpireData);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = pEmpireData->ReadData(GameEmpireData::MinX, &iMinX);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpireData->ReadData(GameEmpireData::MaxX, &iMaxX);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpireData->ReadData(GameEmpireData::MinY, &iMinY);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpireData->ReadData(GameEmpireData::MaxY, &iMaxY);
    RETURN_ON_ERROR(iErrCode);

    int iX, iY;
    GetCoordinates(pvPlanetData[GameMap::iCoordinates].GetCharPtr(), &iX, &iY);

    if (iX < iMinX) {
        iErrCode = pEmpireData->WriteData(GameEmpireData::MinX, iX);
        RETURN_ON_ERROR(iErrCode);
    }
    
    if (iX > iMaxX) {
        iErrCode = pEmpireData->WriteData(GameEmpireData::MaxX, iX);
        RETURN_ON_ERROR(iErrCode);
    }
    
    if (iY < iMinY) {
        iErrCode = pEmpireData->WriteData(GameEmpireData::MinY, iY);
        RETURN_ON_ERROR(iErrCode);
    }
    
    if (iY > iMaxY) {
        iErrCode = pEmpireData->WriteData(GameEmpireData::MaxY, iY);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


#ifdef _DEBUG

int GameEngine::VerifyMap (int iGameClass, int iGameNumber) {

    int iErrCode;

    ICachedTable* pGameMap = NULL;
    AutoRelease<ICachedTable> rel(pGameMap);

    unsigned int* piPlanetKey = NULL, iNumPlanets, iKey, iNumMatches, iNumPlanetsPerEmpire, * piKey = NULL, iNumEmpires, i;
    AutoFreeKeys freeKeys1(piPlanetKey);
    AutoFreeKeys freeKeys2(piKey);

    int cpDir, iPlanet, iX, iY, iNewX, iNewY, iOwner, iHomeWorld, iNeighbourHW;
    int iGameClassOptions, iGameOptions;

    char pszNewCoord [MAX_COORDINATE_LENGTH + 1];
    Variant vCoord, * pvCoord = NULL, * pvLink = NULL;
    AutoFreeData free1(pvLink);
    AutoFreeData free2(pvCoord);

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    
    // Get num empires
    iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    // Get gameclass options
    Variant vTemp;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameClassOptions = vTemp.GetInteger();

    // Get some game settings
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumPlanetsPerEmpire, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iNumPlanetsPerEmpire = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strGameData, GameData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameOptions = vTemp.GetInteger();

    iErrCode = t_pCache->GetTable(strGameMap, &pGameMap);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = pGameMap->ReadColumn(GameMap::Coordinates, &piPlanetKey, &pvCoord, &iNumPlanets);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameMap->ReadColumn(GameMap::Link, NULL, &pvLink, &iNumPlanets);
    RETURN_ON_ERROR(iErrCode);

    if (!(iGameOptions & (GAME_MIRRORED_MAP | GAME_TWISTED_MAP)) && (iNumPlanets % iNumPlanetsPerEmpire != 0))
    {
        Assert(!"Map did not assign the same number of planets per empire");
    }

    // Make sure:
    // all planets have different coordinates
    // each planet has a link
    // each planet has a neighbour
    // each link has a planet
    // empty neighbour slots are really empty

    for (i = 0; i < iNumPlanets; i ++)
    {
        // Make sure coordinates are unique
        iErrCode = pGameMap->GetEqualKeys(
            GameMap::Coordinates,
            pvCoord[i].GetCharPtr(),
            NULL,
            &iNumMatches
            );

        RETURN_ON_ERROR(iErrCode);

        if (iNumMatches != 1)
        {
            Assert(!"A planet's coordinates were re-used");
        }

        if (pvLink[i].GetInteger() == 0 && iNumPlanetsPerEmpire != 1)
        {
            Assert(!"Planet has no links");
        }

        GetCoordinates(pvCoord[i].GetCharPtr(), &iX, &iY);

        iNumMatches = 0;
        
        iErrCode = pGameMap->ReadData(piPlanetKey[i], GameMap::HomeWorld, &iHomeWorld);
        RETURN_ON_ERROR(iErrCode);

        ENUMERATE_CARDINAL_POINTS(cpDir)
        {
            iErrCode = pGameMap->ReadData(piPlanetKey[i], GameMap::ColumnNames[GameMap::iNorthPlanetKey + cpDir], &iPlanet);
            RETURN_ON_ERROR(iErrCode);

            AdvanceCoordinates (iX, iY, &iNewX, &iNewY, cpDir);
            GetCoordinates (iNewX, iNewY, pszNewCoord);
            
            if (iPlanet == NO_KEY)
            {
                // Make sure there's no planet
                iErrCode = pGameMap->GetFirstKey(GameMap::Coordinates, pszNewCoord, &iKey);
                if (iErrCode == ERROR_DATA_NOT_FOUND)
                {
                    iErrCode = OK;
                }
                else
                {
                    Assert(iKey != NO_KEY);
                    Assert(!"Existing planet not listed among a planet's neighbours");
                }
                RETURN_ON_ERROR(iErrCode);

                // Make sure we're not linked to nowhere
                if (pvLink[i].GetInteger() & LINK_X[cpDir])
                {
                    Assert(!"Linked planet does not exist");
                }
            }
            else 
            {
                iNumMatches ++;

                // Make sure that planet has the right coordinates
                iErrCode = pGameMap->ReadData(iPlanet, GameMap::Coordinates, &vCoord);
                RETURN_ON_ERROR(iErrCode);

                if (strcmp (vCoord.GetCharPtr(), pszNewCoord) != 0)
                {
                    Assert(!"Planet has wrong coordinates");
                }

                if (iHomeWorld == HOMEWORLD && iNumPlanetsPerEmpire > 1)
                {
                    iErrCode = pGameMap->ReadData(iPlanet, GameMap::HomeWorld, &iNeighbourHW);
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (iNeighbourHW == HOMEWORLD)
                    {
                        // Make sure there's no link between two homeworlds
                        if (pvLink[i].GetInteger() & LINK_X[cpDir])
                        {
                            Assert(!"Homeworlds are adjacent");
                        }
                    }
                }
            }

        }   // End enum cardinal pts

        if (iNumMatches == 0 && iNumPlanets != 1)
        {
            Assert(!"A planet has no neighbours");
        }

    }   // End all planets loop

    // Verify all planets are connected
    if (!(iGameClassOptions & DISCONNECTED_MAP))
    {
        PlanetHashTable htVisited(NULL, NULL);
        bool init = htVisited.Initialize(iNumPlanets);
        Assert(init);

        // Start somewhere...
        iErrCode = DfsTraversePlanets(pGameMap, piPlanetKey[0], htVisited, iNumPlanets);
        RETURN_ON_ERROR(iErrCode);
        
        for (i = 0; i < iNumPlanets; i ++)
        {
            if (!htVisited.Contains(piPlanetKey[i]))
            {
                Assert(!"The map is not a connected graph");
            }
        }
    }

    // Verify each empire has exactly one homeworld
    iErrCode = pGameMap->GetEqualKeys(
        GameMap::HomeWorld,
        HOMEWORLD,
        &piKey,
        &iNumMatches
        );

    RETURN_ON_ERROR(iErrCode);
    
    if (iNumMatches != iNumEmpires)
    {
        Assert(!"The map does not contain the right number of homeworlds");
    }

    for (i = 0; i < iNumMatches; i ++)
    {
        iErrCode = pGameMap->ReadData(piKey[i], GameMap::Owner, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iOwner = vTemp.GetInteger();

        GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iOwner);
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::HomeWorld, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if ((int) piKey[i] != vTemp.GetInteger())
        {
            Assert(!"A homeworld does not match its owner's data");
        }
    }

    return iErrCode;
}


int GameEngine::DfsTraversePlanets(ICachedTable* pGameMap, unsigned int iPlanetKey, PlanetHashTable& htVisited, unsigned int iNumPlanets)
{
    int iErrCode, cpDir, iNextPlanetKey;

    // Don't revisit
    if (htVisited.Contains(iPlanetKey))
    {
        return OK;
    }
    htVisited.Insert(iPlanetKey, iPlanetKey);

    Variant vLink;
    iErrCode = pGameMap->ReadData(iPlanetKey, GameMap::Link, &vLink);
    RETURN_ON_ERROR(iErrCode);

    ENUMERATE_CARDINAL_POINTS(cpDir)
    {
        if (vLink.GetInteger() & LINK_X[cpDir])
        {
            // Find key
            iErrCode = pGameMap->ReadData(iPlanetKey, GameMap::ColumnNames[GameMap::iNorthPlanetKey + cpDir], &iNextPlanetKey);
            RETURN_ON_ERROR(iErrCode);

            if (iNextPlanetKey == NO_KEY) {
                Assert(!"Linked planet does not exist");
                return ERROR_FAILURE;
            }

            iErrCode = DfsTraversePlanets(pGameMap, iNextPlanetKey, htVisited, iNumPlanets);
            RETURN_ON_ERROR(iErrCode);
        }
    }   

    // All done
    return iErrCode;
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

int GameEngine::GetPlanetNameWithSecurity(int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, Variant* pvPlanetName)
{
    int iErrCode;

    if (iEmpireKey != NO_KEY)
    {
        GET_GAME_EMPIRE_MAP(strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

        unsigned int iKey;
        iErrCode = t_pCache->GetFirstKey(strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            *pvPlanetName = "Unknown";
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    return GetPlanetName(iGameClass, iGameNumber, iEmpireKey, pvPlanetName);
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

int GameEngine::GetPlanetName (int iGameClass, int iGameNumber, int iPlanetKey, Variant* pvPlanetName)
{
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    int iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, pvPlanetName);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
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

int GameEngine::DoesPlanetExist(int iGameClass, int iGameNumber, int iPlanetKey, bool* pbExists)
{
    *pbExists = false;

    GET_GAME_MAP(strGameMap, iGameClass, iGameNumber);
    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Ag, &vTemp);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    *pbExists = true;
    return iErrCode;
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

int GameEngine::GetNeighbourPlanetKey (int iGameClass, int iGameNumber, int iPlanetKey, int iDirection, int* piPlanetKey)
{
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    Variant vKey;
    int iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::ColumnNames[GameMap::iNorthPlanetKey + iDirection], &vKey);
    RETURN_ON_ERROR(iErrCode);

    *piPlanetKey = vKey.GetInteger();
    
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

    ICachedTable* pGameEmpireData = NULL;
    AutoRelease<ICachedTable> rel(pGameEmpireData);

    GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->GetTable(pszEmpireData, &pGameEmpireData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::MinX, piMinX);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::MaxX, piMaxX);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::MinY, piMinY);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::MaxY, piMaxY);
    RETURN_ON_ERROR(iErrCode);

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

int GameEngine::GetMapLimits (int iGameClass, int iGameNumber, int* piMinX, int* piMaxX, int* piMinY, int* piMaxY)
{
    ICachedTable* pGameData = NULL;
    AutoRelease<ICachedTable> rel(pGameData);

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    int iErrCode = t_pCache->GetTable(strGameData, &pGameData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameData->ReadData(GameData::MinX, piMinX);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameData->ReadData(GameData::MaxX, piMaxX);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameData->ReadData(GameData::MinY, piMinY);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameData->ReadData(GameData::MaxY, piMaxY);
    RETURN_ON_ERROR(iErrCode);

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

    GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->ReadColumn(
        pszEmpireMap,
        GameEmpireMap::PlanetKey, 
        ppiEmpireMapKey, 
        ppvPlanetKey, 
        piNumKeys
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

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

int GameEngine::GetNumVisitedPlanets (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumVisitedPlanets)
{
    GET_GAME_EMPIRE_MAP(pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    return t_pCache->GetNumCachedRows(pszEmpireMap, piNumVisitedPlanets);
}

int GameEngine::HasEmpireVisitedPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, bool* pbVisited)
{
    *pbVisited = false;

    GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    unsigned int iKey;
    int iErrCode = t_pCache->GetFirstKey(pszEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    *pbVisited = true;
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
                                        bool bIndependence, unsigned int** ppiShipOwnerData)
{
    int iErrCode;
    Assert(iTotalNumShips > 0);

    *ppiShipOwnerData = NULL;

    // Get number of empires
    GET_GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    unsigned int iNumEmpires, iNumShips, iTotalShips, * piShipKey = NULL, i, j;
    AutoFreeKeys freeKeys(piShipKey);

    int iType, iTemp;

    Variant* pvKey = NULL;
    AutoFreeData free(pvKey);
    iErrCode = t_pCache->ReadColumn(strEmpires, GameEmpires::EmpireKey, NULL, &pvKey, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    char strTheirShips [256], strTheirEmpireMap [256];

    ICachedTable* pShips = NULL, * pMap = NULL;
    AutoRelease<ICachedTable> rel1(pShips);
    AutoRelease<ICachedTable> rel2(pMap);

    unsigned int iSlotsAllocated = 1 + min(iNumEmpires + (bIndependence ? 1 : 0), iTotalNumShips) * (3 + 2 * min(NUM_SHIP_TYPES, iTotalNumShips));

    unsigned int* piShipOwnerData = new unsigned int[iSlotsAllocated];
    Assert(piShipOwnerData);
    Algorithm::AutoDelete<unsigned int> free_piShipOwnerData(piShipOwnerData, true);

    piShipOwnerData[0] = 0;
    unsigned int iCounter = 1;

    // Scan through all players' ship lists
    for (i = 0; i <= iNumEmpires; i ++)
    {
        iTotalShips = 0;

        if (i != iNumEmpires)
        {
            unsigned int iKey;
            COPY_GAME_EMPIRE_MAP(strTheirEmpireMap, iGameClass, iGameNumber, pvKey[i].GetInteger());

            SafeRelease(pMap);
            iErrCode = t_pCache->GetTable(strTheirEmpireMap, &pMap);
            RETURN_ON_ERROR(iErrCode);

            if (pvKey[i].GetInteger() == iEmpireKey && iPlanetProxyKey != NO_KEY)
            {
                iKey = iPlanetProxyKey;
            }
            else
            {
                iErrCode = pMap->GetFirstKey(GameEmpireMap::PlanetKey, iPlanetKey, &iKey);
                if (iErrCode == ERROR_DATA_NOT_FOUND)
                {
                    iErrCode = OK; // Not an error
                }
                RETURN_ON_ERROR(iErrCode);
            }

            if (iKey != NO_KEY)
            {
                iErrCode = pMap->ReadData(iKey, GameEmpireMap::NumUncloakedShips, &iTemp);
                RETURN_ON_ERROR(iErrCode);
                iTotalShips = iTemp;

                if (iEmpireKey == pvKey[i].GetInteger() || iEmpireKey == SYSTEM) {

                    iErrCode = pMap->ReadData(iKey, GameEmpireMap::NumCloakedShips, &iTemp);
                    RETURN_ON_ERROR(iErrCode);
                    iTotalShips += iTemp;

                    iErrCode = pMap->ReadData(iKey, GameEmpireMap::NumCloakedBuildShips, &iTemp);
                    RETURN_ON_ERROR(iErrCode);
                    iTotalShips += iTemp;
                }

                if (bVisibleBuilds || iEmpireKey == pvKey[i]) {

                    iErrCode = pMap->ReadData(iKey, GameEmpireMap::NumUncloakedBuildShips, &iTemp);
                    RETURN_ON_ERROR(iErrCode);

                    iTotalShips += iTemp;
                }

                if (iTotalShips > 0)
                {
                    COPY_GAME_EMPIRE_SHIPS(strTheirShips, iGameClass, iGameNumber, pvKey[i].GetInteger());
                }
            }
        }
        else if (bIndependence)
        {
            GET_GAME_EMPIRE_SHIPS(strTheirShips, iGameClass, iGameNumber, INDEPENDENT);
            iErrCode = t_pCache->GetNumCachedRows(strTheirShips, &iTotalShips);
            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);
        }

        if (iTotalShips > 0) {

            unsigned int iNumDisplayShips = 0;
            
            SafeRelease(pShips);
            iErrCode = t_pCache->GetTable(strTheirShips, &pShips);
            RETURN_ON_ERROR(iErrCode);

            // Scan through ship list
            if (piShipKey)
            {
                t_pCache->FreeKeys(piShipKey);
                piShipKey = NULL;
            }

            iErrCode = pShips->GetEqualKeys(GameEmpireShips::CurrentPlanet, iPlanetKey, &piShipKey, &iNumShips);
            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            else
            {
                RETURN_ON_ERROR(iErrCode);

                unsigned int piIndex [NUM_SHIP_TYPES];
                memset (piIndex, 0, NUM_SHIP_TYPES * sizeof (unsigned int));

                for (j = 0; j < iNumShips; j ++)
                {           
                    bool bDisplay = true;

                    // Read ship type
                    iErrCode = pShips->ReadData(piShipKey[j], GameEmpireShips::Type, &iType);
                    RETURN_ON_ERROR(iErrCode);

                    // Invisible builds?
                    if (i != iNumEmpires && !bVisibleBuilds && pvKey[i].GetInteger() != iEmpireKey) {
                        
                        iErrCode = pShips->ReadData(piShipKey[j], GameEmpireShips::BuiltThisUpdate, &iTemp);
                        RETURN_ON_ERROR(iErrCode);
                        if (iTemp != 0)
                        {
                            bDisplay = false;
                        }
                    }

                    // If ship is cloaked and doesn't belong to empire, don't show it
                    if (i != iNumEmpires && bDisplay && pvKey[i].GetInteger() != iEmpireKey) {

                        iErrCode = pShips->ReadData(piShipKey[j], GameEmpireShips::State, &iTemp);
                        RETURN_ON_ERROR(iErrCode);
                        
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
                    Assert(iCounter < iSlotsAllocated);
                    piShipOwnerData[iCounter] = (i == iNumEmpires) ? INDEPENDENT : pvKey[i].GetInteger();
                    iCounter ++;

                    // Write number of ships owner has
                    Assert(iCounter < iSlotsAllocated);
                    piShipOwnerData[iCounter] = iNumDisplayShips;
                    iCounter ++;

                    // Start off type count at zero
                    unsigned int iTypeCountIndex = iCounter;

                    Assert(iCounter < iSlotsAllocated);
                    piShipOwnerData[iCounter] = 0;
                    iCounter ++;

                    ENUMERATE_SHIP_TYPES (iType)
                    {
                        if (piIndex[iType] > 0)
                        {
                            // Increment type count
                            piShipOwnerData[iTypeCountIndex] ++;

                            // Add type and count
                            Assert(iCounter < iSlotsAllocated);
                            piShipOwnerData[iCounter] = iType;
                            iCounter ++;

                            Assert(iCounter < iSlotsAllocated);
                            piShipOwnerData[iCounter] = piIndex[iType];
                            iCounter ++;
                        }
                    }
                }
            }   // End if ships on planet
        }   // End if empire has ships
    }   // End for each empire

    *ppiShipOwnerData = piShipOwnerData;
    piShipOwnerData = NULL;

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

int GameEngine::RenamePlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, const char* pszNewName)
{
    int iErrCode = OK;

    GET_GAME_MAP (strMap, iGameClass, iGameNumber);

    // Make sure empire still owns planet
    Variant vEmpireKey;
    iErrCode = t_pCache->ReadData(strMap, iPlanetKey, GameMap::Owner, &vEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    if (vEmpireKey.GetInteger() == iEmpireKey)
    {
        iErrCode = t_pCache->WriteData(strMap, iPlanetKey, GameMap::Name, pszNewName);
        RETURN_ON_ERROR(iErrCode);
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

int GameEngine::SetPlanetMaxPop (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, int iNewMaxPop)
{
    int iErrCode;
    Variant vTemp;

    GET_GAME_MAP (strMap, iGameClass, iGameNumber);

    // Check for negative input
    if (iNewMaxPop < 0)
    {
        iNewMaxPop = 0;
    }

    // Make sure empire owns planet
    Variant vEmpireKey;
    iErrCode = t_pCache->ReadData(strMap, iPlanetKey, GameMap::Owner, &vEmpireKey);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    if (vEmpireKey.GetInteger() != iEmpireKey)
    {
        return ERROR_PLANET_DOES_NOT_BELONG_TO_EMPIRE;
    }

    // Get old maxpop
    int iOldMaxPop;
    iErrCode = t_pCache->ReadData(strMap, iPlanetKey, GameMap::MaxPop, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iOldMaxPop = vTemp.GetInteger();
    
    // Write new maxpop
    iErrCode = t_pCache->WriteData(strMap, iPlanetKey, GameMap::MaxPop, iNewMaxPop);
    RETURN_ON_ERROR(iErrCode);
    
    // Update empire's target pop
    int iPopDiff = iNewMaxPop - iOldMaxPop;
    
    // Update empire targetpop
    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::TargetPop, iPopDiff);
    RETURN_ON_ERROR(iErrCode);
    
    // Update NextTotalPop
    int iPlanetPop, iPop, iAg, iBonusAg, iPopLostToColonies;
    float fMaxAgRatio;

    iErrCode = t_pCache->ReadData(strMap, iPlanetKey, GameMap::Pop, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iPlanetPop = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalAg, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iAg = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::BonusAg, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iBonusAg = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalPop, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iPop = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    fMaxAgRatio = vTemp.GetFloat();

    // Adjust for colony pop loss
    iErrCode = t_pCache->ReadData(strMap, iPlanetKey, GameMap::PopLostToColonies, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iPopLostToColonies = vTemp.GetInteger();
    Assert(iPopLostToColonies >= 0);
    
    // Calculate next pop
    int iNewPlanetPop = GetNextPopulation(
        iPlanetPop - iPopLostToColonies, 
        GetAgRatio(iAg + iBonusAg, iPop, fMaxAgRatio)
        );

    int iOldNewPlanetPop = iNewPlanetPop;
    if (iNewPlanetPop > iNewMaxPop)
    {
        iNewPlanetPop = iNewMaxPop;
    }
    if (iOldNewPlanetPop > iOldMaxPop)
    {
        iOldNewPlanetPop = iOldMaxPop;
    }

    iPopDiff = iNewPlanetPop - iOldNewPlanetPop;
    if (iPopDiff != 0)
    {
        iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextTotalPop, iPopDiff);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // Update next min, fuel changes
    int iPlanetMin, iPlanetFuel;

    iErrCode = t_pCache->ReadData(strMap, iPlanetKey, GameMap::Minerals, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iPlanetMin = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strMap, iPlanetKey, GameMap::Fuel, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iPlanetFuel = vTemp.GetInteger();

    int iResDiff = 
        min (iPlanetMin, iOldNewPlanetPop + iPopDiff) - 
        min (iPlanetMin, iOldNewPlanetPop);

    iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextMin, iResDiff);
    RETURN_ON_ERROR(iErrCode);
        
    iResDiff = 
        min (iPlanetFuel, iOldNewPlanetPop + iPopDiff) - 
        min (iPlanetFuel, iOldNewPlanetPop);

    iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NextFuel, iResDiff);
    RETURN_ON_ERROR(iErrCode);
    
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

    GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->GetFirstKey(
        pszEmpireMap,
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        &iKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    *pbExplored = iKey != NO_KEY;

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

int GameEngine::GetGameAveragePlanetResources (int iGameClass, int iGameNumber, int* piAg, int* piMin, int* piFuel)
{
    int iErrCode;
    Variant vTemp;

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(strGameData, GameData::AvgAg, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piAg = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strGameData, GameData::AvgMin, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMin = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strGameData, GameData::AvgFuel, &vTemp);
    RETURN_ON_ERROR(iErrCode);

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

    GET_GAME_MAP (pszGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_MAP (pszGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    ICachedTable* pRead = NULL;
    AutoRelease<ICachedTable> rel(pRead);

    iErrCode = t_pCache->GetTable(pszGameMap, &pRead);
    RETURN_ON_ERROR(iErrCode);

    // Get planets around center
    iErrCode = pRead->ReadData(iCenterKey, GameMap::NorthPlanetKey, piPlanetKey + NORTH);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pRead->ReadData(iCenterKey, GameMap::EastPlanetKey, piPlanetKey + EAST);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pRead->ReadData(iCenterKey, GameMap::SouthPlanetKey, piPlanetKey + SOUTH);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pRead->ReadData(iCenterKey, GameMap::WestPlanetKey, piPlanetKey + WEST);
    RETURN_ON_ERROR(iErrCode);

    // First planet is center
    SafeRelease (pRead);
    iErrCode = t_pCache->GetTable(pszGameEmpireMap, &pRead);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pRead->GetFirstKey(GameEmpireMap::PlanetKey, iCenterKey, &iKey);
    RETURN_ON_ERROR(iErrCode);

    pvPlanetKey[0] = iCenterKey;
    piProxyKey[0] = iKey;
    iNumPlanets = 1;

    // Get planets one jump away
    ENUMERATE_CARDINAL_POINTS(i)
    {
        pbPlanetInLine[i] = false;

        if (piPlanetKey[i] != NO_KEY)
        {
            iErrCode = pRead->GetFirstKey(GameEmpireMap::PlanetKey, piPlanetKey[i], &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            else
            {
                RETURN_ON_ERROR(iErrCode);

                pvPlanetKey[iNumPlanets] = piPlanetKey[i];
                piProxyKey[iNumPlanets ++] = iKey;
                pbPlanetInLine[i] = true;
            }
        }
    }

    // Get planet NE of center
    SafeRelease (pRead);
    iErrCode = t_pCache->GetTable(pszGameMap, &pRead);
    RETURN_ON_ERROR(iErrCode);

    if (piPlanetKey[NORTH] != NO_KEY) {
        
        iErrCode = pRead->ReadData(piPlanetKey[NORTH], GameMap::EastPlanetKey, &iGameMapKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else if (piPlanetKey[EAST] != NO_KEY) {
        
        iErrCode = pRead->ReadData(piPlanetKey[EAST], GameMap::NorthPlanetKey, &iGameMapKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else {
        
        if (iX == MIN_COORDINATE) {
            
            iErrCode = pRead->ReadData(iCenterKey, GameMap::Coordinates, &vCoordinates);
            RETURN_ON_ERROR(iErrCode);
            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
        }

        GetCoordinates (iX + 1, iY + 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey(
            GameMap::Coordinates, 
            pszSearchCoord, 
            (unsigned int*)&iGameMapKey
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    if (iGameMapKey != NO_KEY)
    {
        iErrCode = t_pCache->GetFirstKey(pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, &iKey);
        RETURN_ON_ERROR(iErrCode);

        pbPlanetInLine[NORTH] = pbPlanetInLine[EAST] = true;
        pvPlanetKey[iNumPlanets] = iGameMapKey;
        piProxyKey[iNumPlanets ++] = iKey;
    }
    
    // Get planet SE of center
    SafeRelease (pRead);
    iErrCode = t_pCache->GetTable(pszGameMap, &pRead);
    RETURN_ON_ERROR(iErrCode);

    if (piPlanetKey[SOUTH] != NO_KEY) {
        
        iErrCode = pRead->ReadData(piPlanetKey[SOUTH], GameMap::EastPlanetKey, &iGameMapKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else if (piPlanetKey[EAST] != NO_KEY) {
        
        iErrCode = pRead->ReadData(piPlanetKey[EAST], GameMap::SouthPlanetKey, &iGameMapKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        if (iX == MIN_COORDINATE)
        {
            iErrCode = pRead->ReadData(iCenterKey, GameMap::Coordinates, &vCoordinates);
            RETURN_ON_ERROR(iErrCode);
            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
        }
        
        GetCoordinates(iX + 1, iY - 1, pszSearchCoord);
        
        iErrCode = pRead->GetFirstKey(GameMap::Coordinates, pszSearchCoord, (unsigned int*)&iGameMapKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }
    
    if (iGameMapKey != NO_KEY)
    {
        iErrCode = t_pCache->GetFirstKey(pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);

            pbPlanetInLine[SOUTH] = pbPlanetInLine[EAST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }

    // Get planet SW of center
    SafeRelease (pRead);
    iErrCode = t_pCache->GetTable(pszGameMap, &pRead);
    RETURN_ON_ERROR(iErrCode);

    if (piPlanetKey[SOUTH] != NO_KEY) {

        iErrCode = pRead->ReadData(piPlanetKey[SOUTH], GameMap::WestPlanetKey, &iGameMapKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else if (piPlanetKey[WEST] != NO_KEY) {

        iErrCode = pRead->ReadData(piPlanetKey[WEST], GameMap::SouthPlanetKey, &iGameMapKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        if (iX == MIN_COORDINATE)
        {
            iErrCode = pRead->ReadData(iCenterKey, GameMap::Coordinates, &vCoordinates);
            RETURN_ON_ERROR(iErrCode);
            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
        }

        GetCoordinates (iX - 1, iY - 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey(
            GameMap::Coordinates, 
            pszSearchCoord, 
            (unsigned int*) &iGameMapKey
            );
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY)
    {
        iErrCode = t_pCache->GetFirstKey(pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);

            pbPlanetInLine[SOUTH] = pbPlanetInLine[WEST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }
    
    // Get planet NW of center
    iErrCode = t_pCache->GetTable(pszGameMap, &pRead);
    RETURN_ON_ERROR(iErrCode);

    if (piPlanetKey[NORTH] != NO_KEY) {
        
        iErrCode = pRead->ReadData(piPlanetKey[NORTH], GameMap::WestPlanetKey, &iGameMapKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else if (piPlanetKey[WEST] != NO_KEY) {
        
        iErrCode = pRead->ReadData(piPlanetKey[WEST], GameMap::NorthPlanetKey, &iGameMapKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else {
        
        if (iX == MIN_COORDINATE) {
            
            iErrCode = pRead->ReadData(iCenterKey, GameMap::Coordinates, &vCoordinates);
            RETURN_ON_ERROR(iErrCode);
            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
        }
        
        GetCoordinates (iX - 1, iY + 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey(GameMap::Coordinates, pszSearchCoord, (unsigned int*) &iGameMapKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    if (iGameMapKey != NO_KEY)
    {
        iErrCode = t_pCache->GetFirstKey(pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);

            pbPlanetInLine[NORTH] = pbPlanetInLine[WEST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }

    // Retvals
    *piNumPlanets = iNumPlanets;

    if (iX == MIN_COORDINATE)
    {
        SafeRelease (pRead);
        iErrCode = t_pCache->GetTable(pszGameMap, &pRead);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = pRead->ReadData(iCenterKey, GameMap::Coordinates, &vCoordinates);
        RETURN_ON_ERROR(iErrCode);

        GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
    }
    
    *piMinX = pbPlanetInLine[WEST] ? iX - 1 : iX;
    *piMaxX = pbPlanetInLine[EAST] ? iX + 1 : iX;
    *piMinY = pbPlanetInLine[SOUTH] ? iY - 1 : iY;
    *piMaxY = pbPlanetInLine[NORTH] ? iY + 1 : iY;

    *piCenterX = iX;
    *piCenterY = iY;

    return iErrCode;
}


int GameEngine::GetLowestDiplomacyLevelForShipsOnPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
                                                         int iPlanetKey, bool bVisibleBuilds, 
                                                         Variant* pvEmpireKey, unsigned int& iNumEmpires,
                                                         int* piNumForeignShipsOnPlanet, int* piDiplomacyLevel,
                                                         Variant** ppvEmpireKey)
{
    int iErrCode;

    unsigned int iProxyPlanetKey;

    Variant vTotalNumShips, vNumOurShips, vValue;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    // Get total num ships
    iErrCode = t_pCache->ReadData(
        strGameMap,
        iPlanetKey,
        GameMap::NumUncloakedShips,
        &vTotalNumShips
        );

    RETURN_ON_ERROR(iErrCode);

    // Get our ships
    iErrCode = t_pCache->GetFirstKey(
        strGameEmpireMap,
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        &iProxyPlanetKey
        );

    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(
        strGameEmpireMap,
        iProxyPlanetKey,
        GameEmpireMap::NumUncloakedShips,
        &vNumOurShips
        );

    RETURN_ON_ERROR(iErrCode);
    
    if (bVisibleBuilds) {
        
        iErrCode = t_pCache->ReadData(
            strGameMap,
            iPlanetKey,
            GameMap::NumUncloakedBuildShips,
            &vValue
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        if (vValue.GetInteger() > 0) {
            
            vTotalNumShips += vValue.GetInteger();
            
            iErrCode = t_pCache->ReadData(
                strGameEmpireMap,
                iProxyPlanetKey,
                GameEmpireMap::NumUncloakedBuildShips,
                &vValue
                );
            
            RETURN_ON_ERROR(iErrCode);

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
    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    char strTheirMap [256];
    
    unsigned int i, iProxyDipKey;
    Variant vDipLevel;

    bool bLowestDipSet = false;
    int iLowestDip = ALLIANCE;
    
    if (pvEmpireKey == NULL) {

        iErrCode = t_pCache->ReadColumn(strGameEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
        RETURN_ON_ERROR(iErrCode);

        *ppvEmpireKey = pvEmpireKey;
    }

    for (i = 0; i < iNumEmpires; i ++) {
        
        if (pvEmpireKey[i].GetInteger() == iEmpireKey) {
            continue;
        }
        
        COPY_GAME_EMPIRE_MAP (strTheirMap, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        iErrCode = t_pCache->GetFirstKey(
            strTheirMap,
            GameEmpireMap::PlanetKey,
            iPlanetKey,
            &iProxyPlanetKey
            );
        
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
            continue;
        }
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(
            strTheirMap,
            iProxyPlanetKey,
            GameEmpireMap::NumUncloakedShips,
            &vNumOurShips
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        if (bVisibleBuilds) {

            iErrCode = t_pCache->ReadData(
                strTheirMap,
                iProxyPlanetKey,
                GameEmpireMap::NumUncloakedBuildShips,
                &vValue
                );
            
            RETURN_ON_ERROR(iErrCode);
            
            vNumOurShips += vValue.GetInteger();
        }
        
        if (vNumOurShips.GetInteger() > 0) {

            // Get dip
            iErrCode = t_pCache->GetFirstKey(
                strGameEmpireDiplomacy,
                GameEmpireDiplomacy::ReferenceEmpireKey,
                pvEmpireKey[i].GetInteger(),
                &iProxyDipKey
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                vDipLevel = WAR;
            }
            else
            {
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(
                    strGameEmpireDiplomacy,
                    iProxyDipKey,
                    GameEmpireDiplomacy::CurrentStatus,
                    &vDipLevel
                    );
                
                RETURN_ON_ERROR(iErrCode);
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

    return iErrCode;
}


int GameEngine::SetNewMinMaxIfNecessary (int iGameClass, int iGameNumber, int iEmpireKey, int iX, int iY) {

    int iErrCode;
    char pszTable [256];

    Variant vMinX, vMaxX, vMinY, vMaxY;
    const char* iMinXCol, * iMaxXCol, * iMinYCol, * iMaxYCol;

    if (iEmpireKey == NO_KEY) {

        COPY_GAME_DATA (pszTable, iGameClass, iGameNumber);

        iMinXCol = GameData::MinX;
        iMaxXCol = GameData::MaxX;
        iMinYCol = GameData::MinY;
        iMaxYCol = GameData::MaxY;

    } else {

        COPY_GAME_EMPIRE_DATA (pszTable, iGameClass, iGameNumber, iEmpireKey);

        iMinXCol = GameEmpireData::MinX;
        iMaxXCol = GameEmpireData::MaxX;
        iMinYCol = GameEmpireData::MinY;
        iMaxYCol = GameEmpireData::MaxY;
    }

    iErrCode = t_pCache->ReadData(pszTable, iMinXCol, &vMinX);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadData(pszTable, iMaxXCol, &vMaxX);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadData(pszTable, iMinYCol, &vMinY);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadData(pszTable, iMaxYCol, &vMaxY);
    RETURN_ON_ERROR(iErrCode);

    if (iX < vMinX.GetInteger()) {
        iErrCode = t_pCache->WriteData(pszTable, iMinXCol, iX);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iX > vMaxX.GetInteger()) {
        iErrCode = t_pCache->WriteData(pszTable, iMaxXCol, iX);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iY < vMinY.GetInteger()) {
        iErrCode = t_pCache->WriteData(pszTable, iMinYCol, iY);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iY > vMaxY.GetInteger()) {
        iErrCode = t_pCache->WriteData(pszTable, iMaxYCol, iY);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::GetPlanetPopulationWithColonyBuilds (unsigned int iGameClass, unsigned int iGameNumber,
                                                     unsigned int iEmpireKey, unsigned int iPlanetKey,
                                                     unsigned int* piPop) {

    int iErrCode, iPop, iCost;
    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> rel(pTable);

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    iErrCode = t_pCache->GetTable(strGameMap, &pTable);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(iPlanetKey, GameMap::Pop, &iPop);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pTable->ReadData(iPlanetKey, GameMap::PopLostToColonies, &iCost);
    RETURN_ON_ERROR(iErrCode);

    SafeRelease (pTable);

    Assert(iPop >= 0);
    Assert(iCost >= 0);
    iPop -= iCost;
    Assert(iPop >= 0);

    *piPop = iPop;

    return iErrCode;
}
