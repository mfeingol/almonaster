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

#include "Osal/Algorithm.h"

#include "GameEngine.h"

#include "../MapGen/DefaultMapGenerator.h"


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

    int iErrCode = m_pGameData->ReadData (
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
                                   unsigned int iProperty, Variant* pvProperty) {

    GAME_MAP (pszMap, iGameClass, iGameNumber);
    return m_pGameData->ReadData (pszMap, iPlanetKey, iProperty, pvProperty);
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

    iErrCode = m_pGameData->GetFirstKey (
        pszMap,
        GameMap::Coordinates,
        pszCoord,
        false,
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

    iErrCode = m_pGameData->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::AlmonasterTheme, 
        &vTemp
        );
    
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    if (vTemp.GetInteger() == INDIVIDUAL_ELEMENTS) {
        
        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UILivePlanet, &vTemp);
        if (iErrCode != OK) {
            return iErrCode;
        }
        *piLivePlanetKey = vTemp.GetInteger();
        
        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::UIDeadPlanet, &vTemp);
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
                                 bool* pbCommit) {

    int iErrCode, iMinNumPlanets, iMaxNumPlanets, i, iNumNewPlanets, iGameClassOptions;

    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    char pszEmpireData [256];

    Variant** ppvPlanetData = NULL, * pvGameClassData = NULL, * pvGameData = NULL, ** ppvNewPlanetData = NULL, 
        * pvNewPlanetData = NULL, vTotalAg;

    unsigned int piColumn [GameMap::NumColumns], * piPlanetKey = NULL, iNumPlanets, iNumOrigRows;

    IMapGenerator* pMapGen = NULL;

    // Nothing committed yet
    *pbCommit = false;

    // Initialize columns
    for (i = 0; i < GameMap::NumColumns; i ++) {
        piColumn[i] = (unsigned int) i;
    }

    // Read gameclass data
    iErrCode = m_pGameData->ReadRow (SYSTEM_GAMECLASS_DATA, iGameClass, &pvGameClassData);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iGameClassOptions = pvGameClassData[SystemGameClassData::Options].GetInteger();

    // Does map have planets already?
    iErrCode = m_pGameData->GetNumRows (strGameMap, &iNumOrigRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Read game data
    iErrCode = m_pGameData->ReadRow (strGameData, &pvGameData);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Allocate new planet data
    iMinNumPlanets = pvGameClassData[SystemGameClassData::MinNumPlanets].GetInteger();
    iMaxNumPlanets = pvGameClassData[SystemGameClassData::MaxNumPlanets].GetInteger();

    pvNewPlanetData = new Variant [iMaxNumPlanets * GameMap::NumColumns];
    if (pvNewPlanetData == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    ppvNewPlanetData = (Variant**) StackAlloc (iMaxNumPlanets * sizeof (Variant*));
    for (i = 0; i < iMaxNumPlanets; i ++) {
        ppvNewPlanetData[i] = pvNewPlanetData + i * GameMap::NumColumns;
    }

    // Loop through every empire
    for (i = 0; i < iNumEmpires; i ++) {

        // Get existing map
        iErrCode = m_pGameData->ReadColumns (
            strGameMap,
            GameMap::NumColumns,
            piColumn,
            &piPlanetKey,
            &ppvPlanetData,
            &iNumPlanets
            );
        
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }

        // Get num planets
        if (iNumPlanets == 0) {

            if (iMinNumPlanets == iMaxNumPlanets) {
                iNumNewPlanets = iMinNumPlanets;
            } else {
                iNumNewPlanets = 0;
            }

        } else {

            iNumNewPlanets = pvGameData[GameData::NumPlanetsPerEmpire].GetInteger();
        }

        // Create new generator
        // For now, every gameclass uses the default map generator
        pMapGen = DefaultMapGenerator::CreateInstance (this);
        if (pMapGen == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }
        
        iErrCode = pMapGen->CreatePlanets (
            iGameClass,
            iGameNumber,
            piEmpireKey[i],
            ppvPlanetData,
            iNumPlanets,
            pvGameClassData,
            pvGameData,
            ppvNewPlanetData,
            iNumNewPlanets
            );

        SafeRelease (pMapGen);

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iNumNewPlanets == 0) {
            iNumNewPlanets = pvGameData[GameData::NumPlanetsPerEmpire].GetInteger();
        }

        iErrCode = CreateMapFromMapGeneratorData (
            iGameClass,
            iGameNumber,
            piEmpireKey[i],
            pvGameClassData,
            pvGameData,
            ppvNewPlanetData,
            iNumNewPlanets,
            pbCommit        // Assumed initialized, set to true if data is committed
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (ppvPlanetData != NULL) {
            m_pGameData->FreeData (ppvPlanetData);
            ppvPlanetData = NULL;
        }

        GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, piEmpireKey[i]);

        iErrCode = m_pGameData->ReadData (pszEmpireData, GameEmpireData::TotalAg, &vTotalAg);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Handle 'next statistics'
        iErrCode = WriteNextStatistics (
            iGameClass, 
            iGameNumber, 
            piEmpireKey[i], 
            
            vTotalAg.GetInteger(), 
            0, // No trades or allies at beginning
            pvGameClassData[SystemGameClassData::MaxAgRatio].GetFloat()
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Process subjective views if necessary
/*  if ((iGameClassOptions & EXPOSED_MAP) && (iGameClassOptions & SUBJECTIVE_VIEWS)) {
        
        Variant* pvEmpireKey;
        unsigned int iNumEmpires;

        unsigned int* piEmpireKey;
        bool* pbAlive;

        String* pstrEmpireMap = NULL, * pstrEmpireDip, * pstrEmpireShips;

        String strGameEmpires = GAME_EMPIRES (iGameClass, iGameNumber);

        iErrCode = m_pGameData->ReadColumn (
            strGameEmpires, 
            GameEmpires::EmpireKey, 
            NULL, 
            &pvEmpireKey, 
            &iNumEmpires
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        Assert (pvEmpireKey != NULL && iNumEmpires > 0);

        piEmpireKey = (unsigned int*) StackAlloc (iNumEmpires * sizeof (unsigned int));
        pbAlive = (bool*) StackAlloc (iNumEmpires * sizeof (bool));

        pstrEmpireMap = new String [iNumEmpires * 3];
        pstrEmpireDip = pstrEmpireMap + iNumEmpires;
        pstrEmpireShips = pstrEmpireDip + iNumEmpires;

        if (pstrEmpireMap == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            m_pGameData->FreeData (pvEmpireKey);
            goto Cleanup;
        }

        for (i = 0; i < (int) iNumEmpires; i ++) {

            piEmpireKey[i] = pvEmpireKey[i].GetInteger();
            pbAlive[i] = true;

            pstrEmpireMap[i] = GAME_EMPIRE_MAP (iGameClass, iGameNumber, piEmpireKey[i]);
            pstrEmpireDip[i] = GAME_EMPIRE_DIPLOMACY (iGameClass, iGameNumber, piEmpireKey[i]);
            pstrEmpireShips[i] = GAME_EMPIRE_SHIPS (iGameClass, iGameNumber, piEmpireKey[i]);
        }

        iErrCode = ProcessSubjectiveViews (
            iNumEmpires, 
            piEmpireKey, 
            pbAlive, 
            strGameMap, 
            pstrEmpireMap, 
            pstrEmpireDip, 
            pstrEmpireShips
            );

        m_pGameData->FreeData (pvEmpireKey);
        delete [] pstrEmpireMap;

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }*/

#ifdef _DEBUG

    // Verify map if we created a new one from scratch
    if (iNumOrigRows == 0) {
        iErrCode = VerifyMap (iGameClass, iGameNumber);
        Assert (iErrCode == OK);
        iErrCode = OK;
    }

#endif

Cleanup:

    if (pvGameClassData != NULL) {
        m_pGameData->FreeData (pvGameClassData);
    }

    if (pvGameData != NULL) {
        m_pGameData->FreeData (pvGameData);
    }

    if (piPlanetKey != NULL) {
        m_pGameData->FreeKeys (piPlanetKey);
    }

    if (pvNewPlanetData != NULL) {
        delete [] pvNewPlanetData;
    }

    if (ppvPlanetData != NULL) {
        m_pGameData->FreeData (ppvPlanetData);
    }

    return iErrCode;    
}



//
// Handle output from a map generator
//
int GameEngine::CreateMapFromMapGeneratorData (int iGameClass, int iGameNumber, int iEmpireKey,
                                               Variant* pvGameClassData, Variant* pvGameData, 
                                               Variant** ppvPlanetData, unsigned int iNumNewPlanets,
                                               bool* pbCommit) {

    int iErrCode = OK;

    IWriteTable* pWrite = NULL;

    unsigned int i, j, iKey, iNextKey;

    float fAgRatio;
    int iGameClassOptions, iAg, iMin, iFuel, iPop, iMaxPop, iNumPlanets, iNextTotalPop, * piNewPlanetKey, 
        iMaxPlanetPop, iNewPop, iHomeWorldIndex, iHomeWorldKey, iMinX, iMinY, iMaxX, iMaxY,
        iGameMinX, iGameMaxX, iGameMinY, iGameMaxY;

    Variant* pvKey = NULL;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    char strAnEmpireMap[256];

    if (pvGameData != NULL) {

        // Write newly generated random game data
        iErrCode = m_pGameData->GetTableForWriting (strGameData, &pWrite);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (
            GameData::NumPlanetsPerEmpire, 
            pvGameData[GameData::NumPlanetsPerEmpire].GetInteger()
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (
            GameData::HWAg, 
            pvGameData[GameData::HWAg].GetInteger()
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (
            GameData::HWMin, 
            pvGameData[GameData::HWMin].GetInteger()
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (
            GameData::HWFuel, 
            pvGameData[GameData::HWFuel].GetInteger()
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (
            GameData::AvgAg, 
            pvGameData[GameData::AvgAg].GetInteger()
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (
            GameData::AvgMin, 
            pvGameData[GameData::AvgMin].GetInteger()
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pWrite->WriteData (
            GameData::AvgFuel, 
            pvGameData[GameData::AvgFuel].GetInteger()
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pWrite);
    }

    // Initialize vars
    iGameClassOptions = pvGameClassData[SystemGameClassData::Options].GetInteger();

    iAg = iMin = iFuel = iPop = iMaxPop = iNextTotalPop = iNumPlanets = 0;

    iMinX = iMinY = MAX_COORDINATE;
    iMaxX = iMaxY = MIN_COORDINATE;

    iHomeWorldKey = iHomeWorldIndex = NO_KEY;

    piNewPlanetKey = (int*) StackAlloc (iNumNewPlanets * sizeof (int));

#ifdef _DEBUG
    memset (piNewPlanetKey, NO_KEY, iNumNewPlanets * sizeof (int));
#endif

    // If map exposed and empire doesn't have them, put all old planets into empire's map
    if (iGameClassOptions & EXPOSED_MAP) {

        iNextKey = NO_KEY;
        while (true) {
            
            iErrCode = m_pGameData->GetNextKey (strGameMap, iNextKey, &iNextKey);
            if (iErrCode != OK) {
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    iErrCode = OK;
                } else Assert (false);
                break;
            }
            
            iErrCode = m_pGameData->GetFirstKey (
                strGameEmpireMap,
                GameEmpireMap::PlanetKey,
                iNextKey,
                false,
                &iKey
                );
            
            if (iErrCode != OK) {
                
                if (iErrCode != ERROR_DATA_NOT_FOUND) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = InsertPlanetIntoGameEmpireData (
                    iNextKey, 
                    iGameClassOptions,
                    strGameMap, 
                    strGameEmpireMap
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }
    }

    // Point of no return for map
    *pbCommit = true;

    // Perform new planet insertions
    for (i = 0; i < iNumNewPlanets; i ++) {

        iErrCode = CreatePlanetFromMapGeneratorData (
            strGameMap,
            ppvPlanetData[i],
            iEmpireKey,
            iGameClassOptions,
            &iAg,
            &iMin,
            &iFuel,
            &iPop,
            &iMaxPop,
            &iHomeWorldKey,
            &iNumPlanets,
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
    // Fix up min, max for gamedata
    //

    iErrCode = m_pGameData->GetTableForWriting (strGameData, &pWrite);
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

    // NextTotalPop
    fAgRatio = GetAgRatio (iAg, iPop, pvGameClassData[SystemGameClassData::MaxAgRatio].GetFloat());

    for (i = 0; i < iNumNewPlanets; i ++) {

        iNewPop = GetNextPopulation (ppvPlanetData[i][GameMap::Pop].GetInteger(), fAgRatio);

        iMaxPlanetPop = ppvPlanetData[i][GameMap::MaxPop].GetInteger();
        if (iNewPop > iMaxPlanetPop) {
            iNewPop = iMaxPlanetPop;
        }

        iNextTotalPop += iNewPop;
    }

    // Write out empire parameters
    Assert (pWrite == NULL);
    iErrCode = m_pGameData->GetTableForWriting (strGameEmpireData, &pWrite);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // NumPlanets
    iErrCode = pWrite->WriteData (GameEmpireData::NumPlanets, iNumPlanets);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // TotalAg
    iErrCode = pWrite->WriteData (GameEmpireData::TotalAg, iAg);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // TotalMin
    iErrCode = pWrite->WriteData (GameEmpireData::TotalMin, iMin);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // TotalFuel
    iErrCode = pWrite->WriteData (GameEmpireData::TotalFuel, iFuel);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // TotalPop
    iErrCode = pWrite->WriteData (GameEmpireData::TotalPop, iPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Econ
    iErrCode = pWrite->WriteData (GameEmpireData::Econ, GetEcon (iFuel, iMin, iAg));
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // TargetPop
    iErrCode = pWrite->WriteData (GameEmpireData::TargetPop, iMaxPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // HomeWorld
    iErrCode = pWrite->WriteData (GameEmpireData::HomeWorld, iHomeWorldKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // NextTotalPop
    iErrCode = pWrite->WriteData (GameEmpireData::NextTotalPop, iNextTotalPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pWrite);

    // If FullyColonized or Map Exposed, put all new planets into empire's map
    if ((iGameClassOptions & EXPOSED_MAP) || (iGameClassOptions & FULLY_COLONIZED_MAP)) {

        for (i = 0; i < iNumNewPlanets; i ++) {

            iErrCode = InsertPlanetIntoGameEmpireData (
                piNewPlanetKey[i], 
                iGameClassOptions,
                strGameMap, 
                strGameEmpireMap
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    
    } else {
        
        // Else just add homeworld
        iErrCode = InsertPlanetIntoGameEmpireData (
            iHomeWorldKey,
            iGameClassOptions,
            strGameMap, 
            strGameEmpireMap
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (iGameClassOptions & EXPOSED_MAP) {

        char strAnEmpireData [256];

        IReadTable* pAnEmpireDataRead = NULL;

        int piMaxMinCol[4], piMaxMinVal[4];
        unsigned int iNumMaxMinChanged = 0, iNumKeys;

        // Insert new planets into other empires' maps
        iErrCode = m_pGameData->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, NULL, &pvKey, &iNumKeys);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Max, min
        if (iNumKeys > 1) {

            int iMyMinX, iMyMaxX, iMyMinY, iMyMaxY, iSampleKey = NO_KEY;

            for (i = 0; i < iNumKeys; i ++) {
                if (pvKey[i].GetInteger() != iEmpireKey) {
                    iSampleKey = pvKey[i].GetInteger();
                    break;
                }
            }

            Assert (iSampleKey != NO_KEY);

            GET_GAME_EMPIRE_DATA (strAnEmpireData, iGameClass, iGameNumber, iSampleKey);

            Assert (pAnEmpireDataRead == NULL);
            iErrCode = m_pGameData->GetTableForReading (strAnEmpireData, &pAnEmpireDataRead);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = pAnEmpireDataRead->ReadData (GameEmpireData::MinX, &iMyMinX);
            if (iErrCode != OK) {
                Assert (false);
                pAnEmpireDataRead->Release();
                goto Cleanup;
            }

            iErrCode = pAnEmpireDataRead->ReadData (GameEmpireData::MaxX, &iMyMaxX);
            if (iErrCode != OK) {
                Assert (false);
                pAnEmpireDataRead->Release();
                goto Cleanup;
            }

            iErrCode = pAnEmpireDataRead->ReadData (GameEmpireData::MinY, &iMyMinY);
            if (iErrCode != OK) {
                Assert (false);
                pAnEmpireDataRead->Release();
                goto Cleanup;
            }

            iErrCode = pAnEmpireDataRead->ReadData (GameEmpireData::MaxY, &iMyMaxY);
            if (iErrCode != OK) {
                Assert (false);
                pAnEmpireDataRead->Release();
                goto Cleanup;
            }

            SafeRelease (pAnEmpireDataRead);

            if (iMyMinX < iMinX) {
                iMinX = iMyMinX;
            } else {
                piMaxMinCol[iNumMaxMinChanged] = GameEmpireData::MinX;
                piMaxMinVal[iNumMaxMinChanged] = iMinX;
                iNumMaxMinChanged ++;
            }

            if (iMyMaxX > iMaxX) {
                iMaxX = iMyMaxX;
            } else {
                piMaxMinCol[iNumMaxMinChanged] = GameEmpireData::MaxX;
                piMaxMinVal[iNumMaxMinChanged] = iMaxX;
                iNumMaxMinChanged ++;
            }

            if (iMyMinY < iMinY) {
                iMinY = iMyMinY;
            } else {
                piMaxMinCol[iNumMaxMinChanged] = GameEmpireData::MinY;
                piMaxMinVal[iNumMaxMinChanged] = iMinY;
                iNumMaxMinChanged ++;
            }

            if (iMyMaxY > iMaxY) {
                iMaxY = iMyMaxY;
            } else {
                piMaxMinCol[iNumMaxMinChanged] = GameEmpireData::MaxY;
                piMaxMinVal[iNumMaxMinChanged] = iMaxY;
                iNumMaxMinChanged ++;
            }
        }
        
        for (i = 0; i < iNumKeys; i ++) {
            
            if (pvKey[i].GetInteger() != iEmpireKey) {

                // Insert planets
                GET_GAME_EMPIRE_MAP (strAnEmpireMap, iGameClass, iGameNumber, pvKey[i].GetInteger());

                for (j = 0; j < iNumNewPlanets; j ++) {
                
                    iErrCode = InsertPlanetIntoGameEmpireData (
                        piNewPlanetKey[j],
                        iGameClassOptions,
                        strGameMap, 
                        strAnEmpireMap
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }

                // Set min / max
                if (iNumMaxMinChanged > 0) {
                    
                    GET_GAME_EMPIRE_DATA (strAnEmpireData, iGameClass, iGameNumber, pvKey[i].GetInteger());
                    
                    Assert (pWrite == NULL);
                    iErrCode = m_pGameData->GetTableForWriting (strAnEmpireData, &pWrite);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    for (j = 0; j < iNumMaxMinChanged; j ++) {
                        
                        iErrCode = pWrite->WriteData (piMaxMinCol[j], piMaxMinVal[j]);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                    }

                    SafeRelease (pWrite);
                }
            }
        }

        m_pGameData->FreeData (pvKey);
        pvKey = NULL;
    
    }   // End if exposed map

    else if (!(iGameClassOptions & FULLY_COLONIZED_MAP)) {

        // Max, min for empire is homeworld
        GetCoordinates (ppvPlanetData[iHomeWorldIndex][GameMap::Coordinates].GetCharPtr(), &iMinX, &iMinY);

        iMaxX = iMinX;
        iMaxY = iMinY;
    }

    // Write empire min, max
    Assert (pWrite == NULL);
    iErrCode = m_pGameData->GetTableForWriting (strGameEmpireData, &pWrite);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pWrite->WriteData (GameEmpireData::MinX, iMinX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pWrite->WriteData (GameEmpireData::MaxX, iMaxX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pWrite->WriteData (GameEmpireData::MinY, iMinY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pWrite->WriteData (GameEmpireData::MaxY, iMaxY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pWrite);

    if (pvKey != NULL) {
        m_pGameData->FreeData (pvKey);
    }

    return iErrCode;
}


int GameEngine::CreatePlanetFromMapGeneratorData (const char* strGameMap, 
                                                  Variant* pvPlanetData,
                                                  int iEmpireKey,
                                                  int iGameClassOptions,
                                                  int* piAg,
                                                  int* piMin,
                                                  int* piFuel,
                                                  int* piPop,
                                                  int* piMaxPop,
                                                  int* piHomeWorldKey,
                                                  int* piNumPlanets,
                                                  int* piMinX,
                                                  int* piMaxX,
                                                  int* piMinY,
                                                  int* piMaxY,
                                                  int* piNewPlanetKey
                                                  ) {

    int iErrCode = OK, iMin, iFuel, iMaxPop, iX, iY, iNewX, iNewY, iLink, cpDir;

    unsigned int iKey;

    char pszCoord [MAX_COORDINATE_LENGTH + 1];

    unsigned int piNeighbourKey [NUM_CARDINAL_POINTS];
    int piNeighbourDirection [NUM_CARDINAL_POINTS];
    bool pbLink [NUM_CARDINAL_POINTS];

    unsigned int i, iNumNeighbours;

    bool bHomeWorld = pvPlanetData[GameMap::HomeWorld].GetInteger() == HOMEWORLD;

    // Name
    if (bHomeWorld) {

        iErrCode = m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA,
            iEmpireKey,
            SystemEmpireData::Name,
            pvPlanetData + GameMap::Name
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    
    } else {

        char pszPlanetName [MAX_PLANET_NAME_LENGTH + 1];
        sprintf (pszPlanetName, "Planet %s", pvPlanetData[GameMap::Coordinates].GetCharPtr());

        pvPlanetData[GameMap::Name] = pszPlanetName;
    }

    //
    // Ag, Minerals, Fuel, Coordinates, Link, HomeWorld are set by generator
    //

    // Pop, MaxPop, Owner
    iMin = pvPlanetData[GameMap::Minerals].GetInteger();
    iFuel = pvPlanetData[GameMap::Fuel].GetInteger();

    iMaxPop = max (iMin, iFuel);
    if (iMaxPop == 0) {
        iMaxPop = 1;
    }

    pvPlanetData[GameMap::MaxPop] = iMaxPop;

    if (bHomeWorld || (iGameClassOptions & FULLY_COLONIZED_MAP)) {

        int iAg = pvPlanetData[GameMap::Ag].GetInteger();
        int iPop = iAg > iMaxPop ? iMaxPop : iAg;

        *piAg += iAg;
        *piMin += min (iMin, iPop);
        *piFuel += min (iFuel, iPop);

        *piPop += iPop;
        *piMaxPop += iMaxPop;

        (*piNumPlanets) ++;

        pvPlanetData[GameMap::Pop] = iPop;
        pvPlanetData[GameMap::Owner] = iEmpireKey;

    } else {

        pvPlanetData[GameMap::Pop] = 0;
        pvPlanetData[GameMap::Owner] = SYSTEM;
    }

    // Nuked
    pvPlanetData[GameMap::Nuked] = 0;

    // PopLostToColonies
    pvPlanetData[GameMap::PopLostToColonies] = 0;

    // NumUncloakedShips, NumCloakedShips,  NumUncloakedBuildShips, NumCloakedBuildShips
    pvPlanetData[GameMap::NumUncloakedShips] = 0;
    pvPlanetData[GameMap::NumCloakedShips] = 0;
    pvPlanetData[GameMap::NumUncloakedBuildShips] = 0;
    pvPlanetData[GameMap::NumCloakedBuildShips] = 0;

    // Annihilated
    pvPlanetData[GameMap::Annihilated] = NOT_ANNIHILATED;

    iLink = pvPlanetData[GameMap::Link].GetInteger();

    // 3.0 style Surrenders
    pvPlanetData[GameMap::SurrenderNumAllies] = 0;
    pvPlanetData[GameMap::SurrenderAlmonasterSignificance] = 0;
    pvPlanetData[GameMap::SurrenderEmpireSecretKey] = (int64) 0;
    pvPlanetData[GameMap::SurrenderAlmonasterScore] = (float) 0.0;

    // NorthPlanetKey, EastPlanetKey, SouthPlanetKey, WestPlanetKey,
    GetCoordinates (pvPlanetData[GameMap::Coordinates].GetCharPtr(), &iX, &iY);

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

        iErrCode = m_pGameData->GetFirstKey (
            strGameMap,
            GameMap::Coordinates,
            pszCoord,
            false,
            &iKey
            );
        
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            
            // Can't assert here because linked planets may not be in map yet
            // Assert (!(iLink & LINK_X[cpDir]));
            pvPlanetData[GameMap::NorthPlanetKey + cpDir] = NO_KEY;
            
        } else {
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            Assert (iKey != NO_KEY);
            
            pvPlanetData[GameMap::NorthPlanetKey + cpDir] = iKey;

            piNeighbourKey[iNumNeighbours] = iKey;
            piNeighbourDirection[iNumNeighbours] = cpDir;       
            pbLink[iNumNeighbours] = (iLink & LINK_X[cpDir]) != 0;
            
            iNumNeighbours ++;
        }
    }

    // Insert the planet, finally!
    iErrCode = m_pGameData->InsertRow (strGameMap, pvPlanetData, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    *piNewPlanetKey = iKey;

    if (bHomeWorld) {
        *piHomeWorldKey = iKey;
    }
    
    // Fix up neighbours' data
    for (i = 0; i < iNumNeighbours; i ++) {
        
        iErrCode = m_pGameData->WriteData (
            strGameMap,
            piNeighbourKey[i],
            GameMap::NorthPlanetKey + OPPOSITE_CARDINAL_POINT [piNeighbourDirection[i]],
            (int) iKey
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        if (pbLink[i]) {
            
            iErrCode = m_pGameData->WriteOr (
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

Cleanup:

    return iErrCode;
}

int GameEngine::InsertPlanetIntoGameEmpireData (int iPlanetKey, 
                                                int iGameClassOptions, 
                                                const char* pszGameMap, 
                                                const char* pszGameEmpireMap) {

    int iErrCode;

    Variant pvOldPlanet[GameEmpireMap::NumColumns];

    pvOldPlanet[GameEmpireMap::PlanetKey] = iPlanetKey;
    pvOldPlanet[GameEmpireMap::RESERVED0] = 0;
    pvOldPlanet[GameEmpireMap::RESERVED1] = 0;
    pvOldPlanet[GameEmpireMap::RESERVED2] = 0;
    pvOldPlanet[GameEmpireMap::NumUncloakedShips] = 0;
    pvOldPlanet[GameEmpireMap::NumCloakedBuildShips] = 0;
    pvOldPlanet[GameEmpireMap::NumUncloakedBuildShips] = 0;
    pvOldPlanet[GameEmpireMap::NumCloakedShips] = 0;
    
    Variant vKey;
    int iKey, iExplored = 0, cpDir;
    unsigned int iProxyKey;

    if ((iGameClassOptions & EXPOSED_MAP) || (iGameClassOptions & FULLY_COLONIZED_MAP)) {

        ENUMERATE_CARDINAL_POINTS (cpDir) {
            
            iErrCode = m_pGameData->ReadData (
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
                
                iErrCode = m_pGameData->GetFirstKey (
                    pszGameEmpireMap,
                    GameEmpireMap::PlanetKey,
                    iKey,
                    false,
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
                    
                    iErrCode = m_pGameData->WriteOr (
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
    
    pvOldPlanet [GameEmpireMap::Explored] = iExplored;
    
    iErrCode = m_pGameData->InsertRow (pszGameEmpireMap, pvOldPlanet);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    return iErrCode;
}


#ifdef _DEBUG

int GameEngine::VerifyMap (int iGameClass, int iGameNumber) {

    int iErrCode;

    IReadTable* pGameMap = NULL;

    unsigned int* piPlanetKey = NULL, iNumPlanets, iKey, iNumMatches, iNumPlanetsPerEmpire;
    unsigned int* piKey = NULL, iNumEmpires, i;

    int cpDir, iPlanet, iX, iY, iNewX, iNewY, iOptions, iOwner, * piLink = NULL, iHomeWorld, iNeighbourHW;

    char** ppszCoord = NULL, pszNewCoord [MAX_COORDINATE_LENGTH + 1];
    const char* pszCoord;

    bool* pbVisited, bExists;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    
    char strGameEmpireData [256];

    Variant vTemp;

    // Get num empires
    iErrCode = m_pGameData->GetNumRows (strGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get gameclass options
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iOptions = vTemp.GetInteger();

    // Get num planets
    iErrCode = m_pGameData->ReadData (
        strGameData,  
        GameData::NumPlanetsPerEmpire, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iNumPlanetsPerEmpire = vTemp.GetInteger();

    iErrCode = m_pGameData->GetTableForReading (strGameMap, &pGameMap);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameMap->ReadColumn (GameMap::Coordinates, &piPlanetKey, &ppszCoord, &iNumPlanets);
    
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

    if (iNumPlanets % iNumPlanetsPerEmpire != 0) {
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
            ppszCoord[i],
            false,
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

        GetCoordinates (ppszCoord[i], &iX, &iY);

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
                    false,
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
                iErrCode = pGameMap->ReadData (iPlanet, GameMap::Coordinates, &pszCoord);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (strcmp (pszCoord, pszNewCoord) != 0) {
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
                        
                        // Make sure there's no link!
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
    if (!(iOptions & DISCONNECTED_MAP)) {

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
        false,
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

        iErrCode = m_pGameData->ReadData (strGameMap, piKey[i], GameMap::Owner, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iOwner = vTemp.GetInteger();

        GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iOwner);

        iErrCode = m_pGameData->ReadData (
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
        m_pGameData->FreeKeys (piPlanetKey);
    }

    if (piKey != NULL) {
        m_pGameData->FreeKeys (piKey);
    }

    if (piLink != NULL) {
        m_pGameData->FreeData (piLink);
    }

    if (ppszCoord != NULL) {
        m_pGameData->FreeData (ppszCoord);
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

    if (iEmpireKey == NO_KEY) {

        // Just return coordinates
        *pvPlanetName = "";
        return OK;
    }

    GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    unsigned int iKey;
    int iErrCode = m_pGameData->GetFirstKey (strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, false, &iKey);
    if (iErrCode == OK) {

        GAME_MAP (strGameMap, iGameClass, iGameNumber);
        
        return m_pGameData->ReadData (
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

    return m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Name, pvPlanetName);
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

    int iErrCode = m_pGameData->DoesRowExist (strGameMap, iPlanetKey, pbExists);
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
    int iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::NorthPlanetKey + iDirection, &vKey);

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

    int iErrCode = m_pGameData->GetTableForReading (pszEmpireData, &pGameEmpireData);
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

    int iErrCode = m_pGameData->GetTableForReading (strGameData, &pGameData);
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

    int iErrCode = m_pGameData->ReadColumn (
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

    return m_pGameData->GetNumRows (
        pszEmpireMap, 
        piNumVisitedPlanets
        );
}


int GameEngine::HasEmpireVisitedPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                                        bool* pbVisited) {

    unsigned int iKey;

    GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetFirstKey (
        pszEmpireMap,
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        false,
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
//  [2] -> Number of ship types owner has
//  [3] -> First type key
//  [4] -> Number of ships of first type
//  repeat 3-4
//  repeat 1-4
//
// Returns data about a given visited planet

int GameEngine::GetPlanetShipOwnerData (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                                        int iPlanetProxyKey, int iTotalNumShips, bool bVisibleBuilds, 
                                        bool bIndependence, int** ppiShipOwnerData) {

    // Get number of empires
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);
    GAME_EMPIRE_DIPLOMACY (strDip, iGameClass, iGameNumber, iEmpireKey);

    unsigned int iNumEmpires;
    Variant vDip, * pvKey;
    int iErrCode = m_pGameData->ReadColumn (strEmpires, GameEmpires::EmpireKey, &pvKey, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    char strTheirShips [256], strTheirEmpireMap [256];
    Variant vType, vState, vBuilt, vNuked;
    
    bool bDisplay, bAdded;
    unsigned int iNumKeys, iTotalShips, piIndex [NUM_SHIP_TYPES], * piShipKey = NULL, i, iCounter = 1, 
        iOwnersFound = 0, j, iTemp, iTypes, iKey;

    *ppiShipOwnerData = new int [(min (NUM_SHIP_TYPES, iTotalNumShips) + 1) * (iNumEmpires + 1) + 5];
    if (*ppiShipOwnerData == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    // Scan through all players' ship lists
    for (i = 0; i < iNumEmpires; i ++) {

        GET_GAME_EMPIRE_MAP (strTheirEmpireMap, iGameClass, iGameNumber, pvKey[i].GetInteger());

        if (pvKey[i].GetInteger() == iEmpireKey && iPlanetProxyKey != NO_KEY) {
            iKey = iPlanetProxyKey;
        } else {

            iErrCode = m_pGameData->GetFirstKey (
                strTheirEmpireMap, 
                GameEmpireMap::PlanetKey, 
                iPlanetKey, 
                false, 
                &iKey
                );

            if (iErrCode != ERROR_DATA_NOT_FOUND && iErrCode != OK) {
                goto Cleanup;
            }
        }
        
        if (iKey == NO_KEY) {
            
            iTotalShips = 0;
            iErrCode = OK;  // Not an error

        } else {
            
            iErrCode = m_pGameData->ReadData (
                strTheirEmpireMap, 
                iKey, 
                GameEmpireMap::NumUncloakedShips, 
                &vBuilt
                );
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            iTotalShips = vBuilt.GetInteger();

            if (iEmpireKey == pvKey[i].GetInteger() || iEmpireKey == SYSTEM) {

                iErrCode = m_pGameData->ReadData (
                    strTheirEmpireMap, 
                    iKey, 
                    GameEmpireMap::NumCloakedShips, 
                    &vBuilt
                    );
                if (iErrCode != OK) {
                    goto Cleanup;
                }

                iTotalShips += vBuilt.GetInteger();
                
                iErrCode = m_pGameData->ReadData (
                    strTheirEmpireMap, 
                    iKey, 
                    GameEmpireMap::NumCloakedBuildShips, 
                    &vBuilt
                    );
                if (iErrCode != OK) {
                    goto Cleanup;
                }

                iTotalShips += vBuilt.GetInteger();
            }

            if (bVisibleBuilds || iEmpireKey == pvKey[i]) {

                iErrCode = m_pGameData->ReadData (
                    strTheirEmpireMap, 
                    iKey, 
                    GameEmpireMap::NumUncloakedBuildShips, 
                    &vBuilt
                    );
                if (iErrCode != OK) {
                    goto Cleanup;
                }

                iTotalShips += vBuilt.GetInteger();
            }
        }

        if (iTotalShips > 0) {

            GET_GAME_EMPIRE_SHIPS (strTheirShips, iGameClass, iGameNumber, pvKey[i].GetInteger());
            
            bAdded = false;
            
            // Scan through ship list
            iErrCode = m_pGameData->GetEqualKeys (
                strTheirShips, 
                GameEmpireShips::CurrentPlanet, 
                iPlanetKey, 
                false, 
                &piShipKey, 
                &iNumKeys
                );
            
            if (iErrCode != OK) {

                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    iErrCode = OK;
                } else {
                    Assert (false);
                    goto Cleanup;
                }
            
            } else {
            
                memset (piIndex, 0, NUM_SHIP_TYPES * sizeof (unsigned int));
                
                for (j = 0; j < iNumKeys; j ++) {           
                    
                    bDisplay = true;
                    
                    // Read ship type
                    iErrCode = m_pGameData->ReadData (
                        strTheirShips, 
                        piShipKey[j], 
                        GameEmpireShips::Type, 
                        &vType
                        );
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }

                    // Invisible builds?
                    if (!bVisibleBuilds && pvKey[i].GetInteger() != iEmpireKey) {
                        
                        iErrCode = m_pGameData->ReadData (
                            strTheirShips,
                            piShipKey[j], 
                            GameEmpireShips::BuiltThisUpdate,
                            &vBuilt
                            );
                        if (iErrCode != OK) {
                            goto Cleanup;
                        }
                        
                        if (vBuilt.GetInteger() == 1) {
                            bDisplay = false;
                        }
                    }
                    
                    // If ship is cloaked and doesn't belong to empire, don't show it
                    if (bDisplay && pvKey[i] != iEmpireKey) {
                        
                        iErrCode = m_pGameData->ReadData (
                            strTheirShips, 
                            piShipKey[j], 
                            GameEmpireShips::State, 
                            &vState
                            );
                        if (iErrCode != OK) {
                            goto Cleanup;
                        }
                        
                        if (vState.GetInteger() & CLOAKED) {
                            bDisplay = false;
                        }
                    }
                    
                    if (bDisplay) {
                        
                        if (!bAdded) {
                            
                            bAdded = true;
                            
                            // Increment number of owners
                            iOwnersFound ++;
                            
                            // Get owner's key
                            (*ppiShipOwnerData)[iCounter] = pvKey[i];
                            iCounter ++;
                        }
                        
                        piIndex[vType.GetInteger()] ++;
                    }
                    
                }
                
                if (bAdded) {
                    
                    iTemp = iCounter;
                    iCounter ++;
                    iTypes = 0;
                    
                    ENUMERATE_SHIP_TYPES (j) {

                        if (piIndex[j] > 0) {
                            
                            iTypes ++;
                            
                            (*ppiShipOwnerData)[iCounter] = j;
                            iCounter ++;
                            
                            (*ppiShipOwnerData)[iCounter] = piIndex[j];
                            iCounter ++;
                        }
                    }
                    
                    (*ppiShipOwnerData)[iTemp] = iTypes;
                }
                
                m_pGameData->FreeKeys (piShipKey);
                piShipKey = NULL;
            }

        }   // End if ships on planet

    }   // End empire loop

    // Independent ships
    if (bIndependence) {

        GET_GAME_INDEPENDENT_SHIPS (strTheirShips, iGameClass, iGameNumber);
        
        bAdded = false;
        
        // Scan through ship list
        iErrCode = m_pGameData->GetEqualKeys (
            strTheirShips, 
            GameEmpireShips::CurrentPlanet, 
            iPlanetKey, 
            false, 
            &piShipKey, 
            &iNumKeys
            );
        
        if (iErrCode != OK) {

            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            } else {
                Assert (false);
                goto Cleanup;
            }

        } else {
            
            memset (piIndex, 0, NUM_SHIP_TYPES * sizeof (unsigned int));
            
            for (j = 0; j < iNumKeys; j ++) {           
                
                bDisplay = true;
                
                // Read ship type
                iErrCode = m_pGameData->ReadData (strTheirShips, piShipKey[j], GameEmpireShips::Type, &vType);
                
                if (!bAdded) {
                    
                    bAdded = true;
                    
                    // Increment number of owners
                    iOwnersFound ++;
                    
                    // Get owner's key
                    (*ppiShipOwnerData)[iCounter] = INDEPENDENT;
                    iCounter ++;
                }
                
                piIndex[vType.GetInteger()] ++;
            }
            
            if (bAdded) {
                
                iTemp = iCounter;
                iCounter ++;
                iTypes = 0;
                
                ENUMERATE_SHIP_TYPES (j) {
                    
                    if (piIndex[j] > 0) {
                        
                        iTypes ++;
                        
                        (*ppiShipOwnerData)[iCounter] = j;
                        iCounter ++;
                        
                        (*ppiShipOwnerData)[iCounter] = piIndex[j];
                        iCounter ++;
                    }
                }
                
                (*ppiShipOwnerData)[iTemp] = iTypes;
            }

            m_pGameData->FreeKeys (piShipKey);
            piShipKey = NULL;
        }

    }   // End if independence

    (*ppiShipOwnerData)[0] = iOwnersFound;

Cleanup:

    if (iErrCode != OK) {
        delete [] (*ppiShipOwnerData);
        *ppiShipOwnerData = NULL;
    }

    if (pvKey != NULL) {
        m_pGameData->FreeData (pvKey);
    }

    if (piShipKey != NULL) {
        m_pGameData->FreeKeys (piShipKey);
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
    iErrCode = m_pGameData->ReadData (strMap, iPlanetKey, GameMap::Owner, &vEmpireKey);

    if (iErrCode == OK && vEmpireKey.GetInteger() == iEmpireKey) {
        return m_pGameData->WriteData (strMap, iPlanetKey, GameMap::Name, pszNewName);
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

// TODO: transactions?

int GameEngine::SetPlanetMaxPop (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
                                 int iNewMaxPop) {
    
    int iErrCode;

    GAME_MAP (strMap, iGameClass, iGameNumber);

    // Check for negative input
    if (iNewMaxPop < 0) {
        iNewMaxPop = 0;
    }

    // Make sure empire owns planet
    Variant vEmpireKey;
    iErrCode = m_pGameData->ReadData (strMap, iPlanetKey, GameMap::Owner, &vEmpireKey);
    if (iErrCode != OK) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    if (vEmpireKey.GetInteger() != iEmpireKey) {
        return ERROR_PLANET_DOES_NOT_BELONG_TO_EMPIRE;
    }

    // Get old maxpop
    Variant vOldMaxPop;
    iErrCode = m_pGameData->ReadData (strMap, iPlanetKey, GameMap::MaxPop, &vOldMaxPop);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Write new maxpop
    iErrCode = m_pGameData->WriteData (strMap, iPlanetKey, GameMap::MaxPop, iNewMaxPop);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Update empire's target pop
    int iPopDiff = iNewMaxPop - vOldMaxPop.GetInteger();
    
    // Update empire targetpop
    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::TargetPop, iPopDiff);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Update NextTotalPop
    Variant vPlanetPop, vPop, vAg;

    iErrCode = m_pGameData->ReadData (strMap, iPlanetKey, GameMap::Pop, &vPlanetPop);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalAg, &vAg);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalPop, &vPop);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    Variant vMaxAgRatio;
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MaxAgRatio, 
        &vMaxAgRatio
        );
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Adjust for colony pop loss
    Variant vPopLostToColonies;
    iErrCode = m_pGameData->ReadData (strMap, iPlanetKey, GameMap::PopLostToColonies, &vPopLostToColonies);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    Assert (vPopLostToColonies.GetInteger() >= 0);
    
    // Calculate next pop
    int iNewPlanetPop = GetNextPopulation (
        vPlanetPop.GetInteger() - vPopLostToColonies.GetInteger(),
        GetAgRatio (vAg.GetInteger(), vPop.GetInteger(), vMaxAgRatio.GetFloat())
        );

    int iOldNewPlanetPop = iNewPlanetPop;
    
    if (iNewPlanetPop > iNewMaxPop) {
        iNewPlanetPop = iNewMaxPop;
    }
    if (iOldNewPlanetPop > vOldMaxPop.GetInteger()) {
        iOldNewPlanetPop = vOldMaxPop.GetInteger();
    }
    
    iPopDiff = iNewPlanetPop - iOldNewPlanetPop;
    if (iPopDiff != 0) {
        iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextTotalPop, iPopDiff);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // Update next min, fuel changes
    Variant vPlanetMin, vPlanetFuel;

    iErrCode = m_pGameData->ReadData (strMap, iPlanetKey, GameMap::Minerals, &vPlanetMin);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (strMap, iPlanetKey, GameMap::Fuel, &vPlanetFuel);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    int iResDiff = 
        min (vPlanetMin.GetInteger(), iOldNewPlanetPop + iPopDiff) - 
        min (vPlanetMin.GetInteger(), iOldNewPlanetPop);

    iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextMin, iResDiff);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
        
    iResDiff = 
        min (vPlanetFuel.GetInteger(), iOldNewPlanetPop + iPopDiff) - 
        min (vPlanetFuel.GetInteger(), iOldNewPlanetPop);

    iErrCode = m_pGameData->Increment (strEmpireData, GameEmpireData::NextFuel, iResDiff);
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

    int iErrCode = m_pGameData->GetFirstKey (
        pszEmpireMap,
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        false,
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

    iErrCode = m_pGameData->ReadData (strGameData, GameData::AvgAg, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *piAg = vTemp.GetInteger();

    iErrCode = m_pGameData->ReadData (strGameData, GameData::AvgMin, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *piMin = vTemp.GetInteger();

    iErrCode = m_pGameData->ReadData (strGameData, GameData::AvgFuel, &vTemp);
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
    const char* pszCoordinates;
    char pszSearchCoord [128];

    int piPlanetKey[NUM_CARDINAL_POINTS];
    bool pbPlanetInLine[NUM_CARDINAL_POINTS];

    GAME_MAP (pszGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_MAP (pszGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    IReadTable* pRead = NULL;

    iErrCode = m_pGameData->GetTableForReading (pszGameMap, &pRead);
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
    iErrCode = m_pGameData->GetTableForReading (pszGameEmpireMap, &pRead);
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
    iErrCode = m_pGameData->GetTableForReading (pszGameMap, &pRead);
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
            
            iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &pszCoordinates);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            GetCoordinates (pszCoordinates, &iX, &iY);
        }

        GetCoordinates (iX + 1, iY + 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey (
            GameMap::Coordinates, 
            pszSearchCoord, 
            false, 
            (unsigned int*) &iGameMapKey
            );

        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }

    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY) {
        
        iErrCode = m_pGameData->GetFirstKey (pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, false, &iKey);
        if (iErrCode == OK) {
            pbPlanetInLine[NORTH] = pbPlanetInLine[EAST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }
    
    // Get planet SE of center
    iErrCode = m_pGameData->GetTableForReading (pszGameMap, &pRead);
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
            
            iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &pszCoordinates);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            GetCoordinates (pszCoordinates, &iX, &iY);
        }
        
        GetCoordinates (iX + 1, iY - 1, pszSearchCoord);
        
        iErrCode = pRead->GetFirstKey (
            GameMap::Coordinates, 
            pszSearchCoord, 
            false, 
            (unsigned int*) &iGameMapKey
            );
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }
    
    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY) {
        
        iErrCode = m_pGameData->GetFirstKey (pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, false, &iKey);
        if (iErrCode == OK) {
            pbPlanetInLine[SOUTH] = pbPlanetInLine[EAST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }

    // Get planet SW of center
    iErrCode = m_pGameData->GetTableForReading (pszGameMap, &pRead);
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

            iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &pszCoordinates);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            GetCoordinates (pszCoordinates, &iX, &iY);
        }

        GetCoordinates (iX - 1, iY - 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey (
            GameMap::Coordinates, 
            pszSearchCoord, 
            false, 
            (unsigned int*) &iGameMapKey
            );
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }

    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY) {
        
        iErrCode = m_pGameData->GetFirstKey (pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, false, &iKey);
        if (iErrCode == OK) {
            pbPlanetInLine[SOUTH] = pbPlanetInLine[WEST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }
    
    // Get planet NW of center
    iErrCode = m_pGameData->GetTableForReading (pszGameMap, &pRead);
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
            
            iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &pszCoordinates);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            GetCoordinates (pszCoordinates, &iX, &iY);
        }
        
        GetCoordinates (iX - 1, iY + 1, pszSearchCoord);

        iErrCode = pRead->GetFirstKey (
            GameMap::Coordinates, 
            pszSearchCoord, 
            false, 
            (unsigned int*) &iGameMapKey
            );
        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }
    }

    SafeRelease (pRead);

    if (iGameMapKey != NO_KEY) {
        
        iErrCode = m_pGameData->GetFirstKey (pszGameEmpireMap, GameEmpireMap::PlanetKey, iGameMapKey, false, &iKey);
        if (iErrCode == OK && iKey != NO_KEY) {
            pbPlanetInLine[NORTH] = pbPlanetInLine[WEST] = true;
            pvPlanetKey[iNumPlanets] = iGameMapKey;
            piProxyKey[iNumPlanets ++] = iKey;
        }
    }

    // Retvals
    *piNumPlanets = iNumPlanets;

    if (iX == MIN_COORDINATE) {

        iErrCode = m_pGameData->GetTableForReading (pszGameMap, &pRead);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        iErrCode = pRead->ReadData (iCenterKey, GameMap::Coordinates, &pszCoordinates);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pRead);
        
        GetCoordinates (pszCoordinates, &iX, &iY);
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
    iErrCode = m_pGameData->ReadData (
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
    iErrCode = m_pGameData->GetFirstKey (
        strGameEmpireMap,
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        false,
        &iProxyPlanetKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (
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
        
        iErrCode = m_pGameData->ReadData (
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
            
            iErrCode = m_pGameData->ReadData (
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

    int iLowestDip = ALLIANCE;
    
    if (pvEmpireKey == NULL) {

        iErrCode = m_pGameData->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, &pvEmpireKey, &iNumEmpires);
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
        
        iErrCode = m_pGameData->GetFirstKey (
            strTheirMap,
            GameEmpireMap::PlanetKey,
            iPlanetKey,
            true,
            &iProxyPlanetKey
            );
        
        if (iErrCode != OK) {

            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                continue;
            }

            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->ReadData (
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

            iErrCode = m_pGameData->ReadData (
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
            iErrCode = m_pGameData->GetFirstKey (
                strGameEmpireDiplomacy,
                GameEmpireDiplomacy::EmpireKey,
                pvEmpireKey[i].GetInteger(),
                false,
                &iProxyDipKey
                );

            if (iErrCode == OK) {

                iErrCode = m_pGameData->ReadData (
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

            if (vDipLevel.GetInteger() < iLowestDip) {
                iLowestDip = vDipLevel.GetInteger();
            }
        }
    }

    *piDiplomacyLevel = iLowestDip;

Cleanup:

    return iErrCode;
}


int GameEngine::SetNewMinMaxIfNecessary (int iGameClass, int iGameNumber, int iEmpireKey, int iX, int iY) {

    int iErrCode;
    char pszTable [256];

    Variant vMinX, vMaxX, vMinY, vMaxY;
    unsigned int iMinXCol, iMaxXCol, iMinYCol, iMaxYCol;

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

    iErrCode = m_pGameData->ReadData (pszTable, iMinXCol, &vMinX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = m_pGameData->ReadData (pszTable, iMaxXCol, &vMaxX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = m_pGameData->ReadData (pszTable, iMinYCol, &vMinY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = m_pGameData->ReadData (pszTable, iMaxYCol, &vMaxY);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iX < vMinX.GetInteger()) {
        iErrCode = m_pGameData->WriteData (pszTable, iMinXCol, iX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (iX > vMaxX.GetInteger()) {
        iErrCode = m_pGameData->WriteData (pszTable, iMaxXCol, iX);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (iY < vMinY.GetInteger()) {
        iErrCode = m_pGameData->WriteData (pszTable, iMinYCol, iY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    if (iY > vMaxY.GetInteger()) {
        iErrCode = m_pGameData->WriteData (pszTable, iMaxYCol, iY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

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
    iErrCode = m_pGameData->ReadData (pszGameData, GameData::Options, &vGameOptions);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!(vGameOptions.GetInteger() & GAME_ALLOW_SPECTATORS)) {
        Assert (false);
        return ERROR_NOT_SPECTATOR_GAME;
    }

    iErrCode = m_pGameData->ReadData (pszGameData, GameData::State, &vGameState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (vGameState.GetInteger() & STILL_OPEN) {
        Assert (false);
        return ERROR_GAME_HAS_NOT_CLOSED;
    }

    // Get gameclass options
    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vGameClassOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vGameClassOptions.GetInteger() & EXPOSED_MAP) {

        // Easy way out
        iErrCode = m_pGameData->GetAllKeys (pszGameMap, ppiPlanetKey, piNumPlanets);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameData->GetTableForReading (pszGameData, &pReadTable);
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

        iErrCode = m_pGameData->ReadColumn (
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
            
            iErrCode = m_pGameData->GetNumRows (pszEmpireMap, &iNumPlanets);
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

        iErrCode = m_pGameData->ReadColumn (pszEmpireMap, GameEmpireMap::PlanetKey, &pvPlanetKey, &iNumPlanets);
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
            
            iErrCode = m_pGameData->GetTableForReading (pszEmpireMap, &pEmpMap);
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

        iErrCode = m_pGameData->GetTableForReading (pszGameMap, &pReadTable);
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
        m_pGameData->FreeData (pvEmpireKey);
    }

    if (pvPlanetKey != NULL) {
        m_pGameData->FreeData (pvPlanetKey);
    }

    if ((iErrCode != OK || *piNumPlanets == 0) && (*ppiPlanetKey) != NULL) {
        m_pGameData->FreeKeys (*ppiPlanetKey);
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

    iErrCode = m_pGameData->DoesRowExist (pszGameMap, iPlanetKey, pbVisible);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (!(*pbVisible)) {
        return OK;
    }
    
    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    while (true) {

        iErrCode = m_pGameData->GetNextKey (pszEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            return OK;
        }

        iErrCode = m_pGameData->ReadData (pszEmpires, iKey, GameEmpires::EmpireKey, &vEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, vEmpireKey.GetInteger());

        iErrCode = m_pGameData->GetFirstKey (
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