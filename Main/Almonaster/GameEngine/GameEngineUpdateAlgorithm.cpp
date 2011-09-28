//
// Almonaster.dll:  a component of Almonaster 2.x
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite nu330, Boston, MA  02111-1307, USA.

#include "GameEngine.h"

#include <math.h>

#define GetEmpireIndex(j, i) for (j = 0; j < (int) iNumEmpires; j ++) { if (piEmpireKey[j] == i) { break; }} Assert(j < iNumEmpires);

#define AddPlanetNameAndCoordinates(strString, pszPlanetName, iX, iY)   \
    (strString).AppendHtml (pszPlanetName, 0, false);                   \
    (strString) += " (";                                                \
    (strString) += iX;                                                  \
    (strString) += ",";                                                 \
    (strString) += iY;                                                  \
    (strString) += ")";

// Input:
// iGameClass -> Game class key
// iGameNumber -> Game number
// tUpdateTime -> Time update occurred
//
// Output:
// *pbGameOver -> true if the game ended, false otherwise
//
// Execute an update

int GameEngine::RunUpdate(int iGameClass, int iGameNumber, const UTCTime& tUpdateTime, bool* pbGameOver)
{
    int iErrCode;
    
    *pbGameOver = false;

    // Strings
    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_SHIPS(strIndependentShips, iGameClass, iGameNumber, INDEPENDENT);

    String strMessage;
    Variant vTemp, vBR, vShipName;

    GameConfiguration gcConfig;

    iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetGameConfiguration(&gcConfig);
    RETURN_ON_ERROR(iErrCode);
    
    // Get gameclass name
    char pszGameClassName[MAX_FULL_GAME_CLASS_NAME_LENGTH];
    iErrCode = GetGameClassName(iGameClass, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iGameClassOptions = vTemp.GetInteger();

    bool bIndependence = (iGameClassOptions & INDEPENDENCE) != 0;

    unsigned int* piNukedPlanetKey, ** ppiEmpireNukeKey, * piNumNukingShips, i, j, * piPlanetKey = NULL, iNumPlanets;
    AutoFreeKeys free_piPlanetKey(piPlanetKey);

    unsigned int** ppiShipNukeKey = NULL, iNumNukedPlanets = 0;
    AutoFreeArrayOfArrays<unsigned int> free_ppiShipNukeKey(ppiShipNukeKey, iNumNukedPlanets);

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iUpdatePeriod = vTemp.GetInteger();

    ////////////////////////////////
    // Initialize data for update //
    ////////////////////////////////

    // Get list of empire keys
    Variant* pvEmpireKey = NULL;
    AutoFreeData free_pvEmpireKey(pvEmpireKey);

    unsigned int iNumEmpires;
    iErrCode = t_pCache->ReadColumn(strGameEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);
    Assert(iNumEmpires > 0);
        
    // Initialize boolean alive array
    bool* pbAlive = (bool*) StackAlloc(iNumEmpires * sizeof (bool) * 2);
    bool* pbSendFatalMessage = pbAlive + iNumEmpires;
    unsigned int* piEmpireKey = (unsigned int*)StackAlloc (iNumEmpires * sizeof (unsigned int));
        
    for (i = 0; i < iNumEmpires; i ++)
    {
        piEmpireKey[i] = pvEmpireKey[i].GetInteger();
        pbAlive[i] = true;

        iErrCode = GetEmpireOption (piEmpireKey[i], DISPLAY_FATAL_UPDATE_MESSAGES, pbSendFatalMessage + i);
        RETURN_ON_ERROR(iErrCode);
    }

    // Randomize empire list so all order-dependent events are unpredictable
    Algorithm::Randomize<unsigned int> (piEmpireKey, iNumEmpires);

    // Read number of updates transpired so far
    Variant vNumUpdates;
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vNumUpdates);
    RETURN_ON_ERROR(iErrCode);

    int iNewUpdateCount = vNumUpdates.GetInteger() + 1;
    
    // Read max tech increase for game
    Variant vGameMaxTechIncrease;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxTechDev, &vGameMaxTechIncrease);
    RETURN_ON_ERROR(iErrCode);

    Variant vMaxAgRatio;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vMaxAgRatio);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iGameState = vTemp.GetInteger();

    ///////////////////////////////
    // Generate map if necessary //
    ///////////////////////////////

    bool bMapGenerated = false;

    if (iNewUpdateCount == 1 && 
        (iGameClassOptions & GENERATE_MAP_FIRST_UPDATE) &&
        !(iGameState & GAME_MAP_GENERATED))
    {
        iErrCode = t_pCache->ReadData(strGameData, GameData::MapFairness, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        GameFairnessOption gfoFairness = (GameFairnessOption)vTemp.GetInteger();
        iErrCode = AddEmpiresToMap (iGameClass, iGameNumber, (int*) piEmpireKey, iNumEmpires, gfoFairness);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteOr(strGameData, GameData::State, GAME_MAP_GENERATED);
        RETURN_ON_ERROR(iErrCode);

        iGameState |= GAME_MAP_GENERATED;
        bMapGenerated = true;
    }

    Assert(iGameState & GAME_MAP_GENERATED);

    //
    // Initialize empire's table names - big stack alloc!
    //

#define TABLE_LENGTH 192
#define NUM_TABLES 5

    unsigned int iNumTables = iNumEmpires * NUM_TABLES;

    char* pszBuffer = (char*) StackAlloc (iNumTables * TABLE_LENGTH * sizeof (char));
    char** ppszPointers = (char**) StackAlloc (iNumTables * sizeof (char**));

    for (i = 0; i < iNumTables; i ++)
    {
        ppszPointers[i] = pszBuffer + i * TABLE_LENGTH;
    }

    const char** pstrEmpireData = (const char**) ppszPointers;
    const char** pstrEmpireDip = pstrEmpireData + iNumEmpires;
    const char** pstrEmpireShips = pstrEmpireDip + iNumEmpires;
    const char** pstrEmpireFleets = pstrEmpireShips + iNumEmpires;
    const char** pstrEmpireMap = pstrEmpireFleets + iNumEmpires;

    String* pstrUpdateMessage = new String[iNumEmpires];
    Assert(pstrUpdateMessage);
    Algorithm::AutoDelete<String> auto_pstrUpdateMessage(pstrUpdateMessage, true);

    Variant* pvGoodColor = new Variant[iNumEmpires * 3];
    Assert(pvGoodColor);
    Algorithm::AutoDelete<Variant> auto_pvGoodColor(pvGoodColor, true);

    Variant* pvBadColor = pvGoodColor + iNumEmpires;
    Variant* pvEmpireName = pvBadColor + iNumEmpires;

    int* piTotalMin = (int*)StackAlloc(iNumEmpires * 10 * sizeof(int));
    int* piTotalFuel = piTotalMin + iNumEmpires;
    int* piTotalAg = piTotalFuel + iNumEmpires;

    int* piBonusMin = piTotalAg + iNumEmpires;
    int* piBonusFuel = piBonusMin + iNumEmpires;
    int* piBonusAg = piBonusFuel + iNumEmpires;

    int* piObliterator = piBonusAg + iNumEmpires;
    int* piObliterated = piObliterator + iNumEmpires;

    int* piWinner = piObliterated + iNumEmpires;
    int* piLoser = piWinner + iNumEmpires;

    float* pfMaintRatio = (float*)StackAlloc(iNumEmpires * 3 * sizeof(float));
    float* pfAgRatio = pfMaintRatio + iNumEmpires;
    float* pfFuelRatio = pfAgRatio + iNumEmpires;

    float fTechDelta;

    unsigned int iNumObliterations = 0, iNumSurrenders = 0, iNumRuins = 0, iNumNewDraws = 0, * piOriginalPlanetOwner = NULL, * piOriginalNumObliterations;

    Variant vMaxBR, vTechLevel, vTechDevs, vNumOwnShips, vMaint, vBuild, vFuel, vTheme;
    int iBR1, iBR2;

    char pszUpdateTime[OS::MaxDateLength];

    for (i = 0; i < iNumEmpires; i ++)
    {
        // Name
        iErrCode = GetEmpireProperty(piEmpireKey[i], SystemEmpireData::Name, pvEmpireName + i);
        RETURN_ON_ERROR(iErrCode);
        
        // Good color
        iErrCode = GetEmpireProperty(piEmpireKey[i], SystemEmpireData::AlmonasterTheme, &vTheme);
        RETURN_ON_ERROR(iErrCode);

        switch (vTheme.GetInteger()) {
            
        case INDIVIDUAL_ELEMENTS:
        case ALTERNATIVE_PATH:

            iErrCode = GetEmpireProperty(piEmpireKey[i], SystemEmpireData::UIColor, &vFuel);
            RETURN_ON_ERROR(iErrCode);
            
            switch (vFuel.GetInteger()) {
                
            case CUSTOM_COLORS:
                
                iErrCode = GetEmpireProperty (piEmpireKey[i], SystemEmpireData::CustomGoodColor, pvGoodColor + i);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = GetEmpireProperty (piEmpireKey[i], SystemEmpireData::CustomBadColor, pvBadColor + i);
                RETURN_ON_ERROR(iErrCode);

                break;
                
            case NULL_THEME:
                
                pvGoodColor[i] = DEFAULT_GOOD_COLOR;
                pvBadColor[i] = DEFAULT_BAD_COLOR;
                break;
                
            default:
                
                iErrCode = GetThemeGoodColor (vFuel.GetInteger(), pvGoodColor + i);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = GetThemeBadColor (vFuel.GetInteger(), pvBadColor + i);
                RETURN_ON_ERROR(iErrCode);
                break;
            }
            break;
        
        case NULL_THEME:

            pvGoodColor[i] = DEFAULT_GOOD_COLOR;
            pvBadColor[i] = DEFAULT_BAD_COLOR;
            break;

        default:
            
            iErrCode = t_pCache->ReadData(SYSTEM_THEMES, vTheme.GetInteger(), SystemThemes::GoodColor, pvGoodColor + i);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(SYSTEM_THEMES, vTheme.GetInteger(), SystemThemes::BadColor, pvBadColor + i);
            RETURN_ON_ERROR(iErrCode);
            
            break;
        }

        Assert(pvGoodColor[i].GetCharPtr() &&  pvBadColor[i].GetCharPtr());

        COPY_GAME_EMPIRE_DATA ((char*) pstrEmpireData[i], iGameClass, iGameNumber, piEmpireKey[i]);
        COPY_GAME_EMPIRE_DIPLOMACY ((char*) pstrEmpireDip[i], iGameClass, iGameNumber, piEmpireKey[i])
        COPY_GAME_EMPIRE_SHIPS ((char*) pstrEmpireShips[i], iGameClass, iGameNumber, piEmpireKey[i])
        COPY_GAME_EMPIRE_FLEETS ((char*) pstrEmpireFleets[i], iGameClass, iGameNumber, piEmpireKey[i])
        COPY_GAME_EMPIRE_MAP ((char*) pstrEmpireMap[i], iGameClass, iGameNumber, piEmpireKey[i]);

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TotalMin, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        piTotalMin[i] = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TotalBuild, &vBuild);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TotalMaintenance, &vMaint);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TotalFuelUse, &vFuel);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TotalFuel, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        piTotalFuel[i] = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TotalAg, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        piTotalAg[i] = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::BonusFuel, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        piBonusFuel[i] = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::BonusAg, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        piBonusAg[i] = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::BonusMin, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        piBonusMin[i] = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TotalPop, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        pfMaintRatio[i] = GetMaintenanceRatio (piTotalMin[i] + piBonusMin[i], vMaint, vBuild);
        Assert(pfMaintRatio[i] >= 0.0);

        pfAgRatio[i] = GetAgRatio (piTotalAg[i] + piBonusAg[i], vTemp.GetInteger(), vMaxAgRatio.GetFloat());
        Assert(pfAgRatio[i] >= 0.0);

        pfFuelRatio[i] = GetFuelRatio (piTotalFuel[i] + piBonusFuel[i], vFuel);
        Assert(pfFuelRatio[i] >= 0.0);

        // Initial update messages
        iErrCode = Time::GetDateString (tUpdateTime, pszUpdateTime);
        RETURN_ON_ERROR(iErrCode);

        pstrUpdateMessage[i] = BEGIN_LARGE_FONT "Update ";
        pstrUpdateMessage[i] += iNewUpdateCount;
        pstrUpdateMessage[i] += " occurred on ";
        pstrUpdateMessage[i] += pszUpdateTime;
        pstrUpdateMessage[i] += END_FONT "\n";

        // Increase empires' tech level
        fTechDelta = GetTechDevelopment (
            piTotalFuel[i] + piBonusFuel[i], 
            piTotalMin[i] + piBonusMin[i], 
            vMaint.GetInteger(), 
            vBuild.GetInteger(), 
            vFuel.GetInteger(), 
            vGameMaxTechIncrease.GetFloat()
            );

        iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::TechLevel, fTechDelta, &vTechLevel);
        RETURN_ON_ERROR(iErrCode);

        // Handle new tech developments
        iBR1 = GetBattleRank (vTechLevel.GetFloat());
        iBR2 = GetBattleRank (vTechLevel.GetFloat() + fTechDelta);

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::MaxBR, &vMaxBR);
        RETURN_ON_ERROR(iErrCode);

        // Did we develop a new tech?
        if (iBR1 != iBR2 && vMaxBR.GetInteger() < iBR2) {
            
            /// Increment maxBR
            iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::MaxBR, iBR2);
            RETURN_ON_ERROR(iErrCode);

            // Update message
            pstrUpdateMessage[i] += BEGIN_GOOD_FONT(i) "You have reached " BEGIN_STRONG "BR ";
            pstrUpdateMessage[i] += iBR2;
            pstrUpdateMessage[i] += END_STRONG END_FONT "\n";
            
            // If a tech remains to be developed, increment the number available
            iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TechUndevs, &vTechDevs);
            RETURN_ON_ERROR(iErrCode);
            
            if (vTechDevs.GetInteger() != 0)
            {
                iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::NumAvailableTechUndevs, iBR2 - iBR1);
                RETURN_ON_ERROR(iErrCode);
            }
        }

        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::NumAvailableTechUndevs, &vTechDevs);
        RETURN_ON_ERROR(iErrCode);

        if (vTechDevs.GetInteger() > 0) {

            // Add to update message if there are techs remaining
            pstrUpdateMessage[i] += BEGIN_GOOD_FONT(i) "You can develop " BEGIN_STRONG;
            pstrUpdateMessage[i] += vTechDevs.GetInteger();

            if (vTechDevs.GetInteger() == 1) {
                pstrUpdateMessage[i] += END_STRONG " new technology" END_FONT "\n";
            } else {
                pstrUpdateMessage[i] += END_STRONG " new technologies" END_FONT" \n";
            }
        }
    }   // End empire initialization loop


    // Add map generation to update messages
    if (bMapGenerated)
    {
        for (i = 0; i < iNumEmpires; i ++)
        {
            pstrUpdateMessage[i] += "The game map was generated\n";
        }
    }

    ////////////////////////////////////////////
    // Close game if numupdates is sufficient //
    ////////////////////////////////////////////
    
    if (iGameState & STILL_OPEN)
    {
        iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdatesBeforeGameCloses, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        
        if (iNewUpdateCount >= vTemp.GetInteger()) {
            
            // Close game
            iErrCode = t_pCache->WriteAnd(strGameData, GameData::State, ~STILL_OPEN);
            RETURN_ON_ERROR(iErrCode);
            
            char pszData[MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
            GetGameClassGameNumber(iGameClass, iGameNumber, pszData);
            
            // Mark closed in game list
            unsigned int iActiveGameKey;
            iErrCode = t_pCache->GetFirstKey(SYSTEM_ACTIVE_GAMES, SystemActiveGames::GameClassGameNumber, pszData, &iActiveGameKey);
            RETURN_ON_ERROR(iErrCode);
            
            Assert(iActiveGameKey != NO_KEY);
            
            iErrCode = t_pCache->WriteData(SYSTEM_ACTIVE_GAMES, iActiveGameKey, SystemActiveGames::State, 0);
            RETURN_ON_ERROR(iErrCode);

            // Add to update messages
            for (i = 0; i < iNumEmpires; i ++)
            {
                pstrUpdateMessage[i] += "The game has closed\n";
            }
        }
    }

    //////////////////////////////////////////////////
    // Update diplomatic status between all empires //
    //////////////////////////////////////////////////

    iErrCode = UpdateDiplomaticStatus (iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, 
        pbSendFatalMessage, pvEmpireName, 
        pstrUpdateMessage, pstrEmpireDip, pstrEmpireMap, strGameMap, pstrEmpireData, piWinner, piLoser, 
        &iNumSurrenders,  pszGameClassName, iNewUpdateCount, pvGoodColor, pvBadColor);

    RETURN_ON_ERROR(iErrCode);

    ////////////////////////////////////////////////////
    // Check for ally out, draw out if game is closed //
    ////////////////////////////////////////////////////

    if (!(iGameState & STILL_OPEN))
    {
        iErrCode = CheckGameForEndConditions (iGameClass, iGameNumber, NULL, pbGameOver);
        RETURN_ON_ERROR(iErrCode);

        if (*pbGameOver)
        {
            return OK;
        }   
    }

    ///////////////////////////////
    // Update planet populations //
    ///////////////////////////////
    
    iErrCode = t_pCache->GetAllKeys(strGameMap, &piPlanetKey, &iNumPlanets);
    RETURN_ON_ERROR(iErrCode);

    // Hit the heap for stack safety
    piOriginalPlanetOwner = new unsigned int[iNumPlanets * 2];
    Assert(piOriginalPlanetOwner);
    Algorithm::AutoDelete<unsigned int> auto_piOriginalPlanetOwner(piOriginalPlanetOwner, true);

    piOriginalNumObliterations = piOriginalPlanetOwner + iNumPlanets;

    memset(piOriginalPlanetOwner, NO_KEY, iNumPlanets * sizeof (unsigned int));
    memset(piOriginalNumObliterations, ANNIHILATED_UNKNOWN, iNumPlanets * sizeof (unsigned int));

    iErrCode = UpdatePlanetPopulations(iNumEmpires, piEmpireKey, pbAlive, pfAgRatio, strGameMap, 
        pstrEmpireData, pstrEmpireMap, pstrUpdateMessage, piPlanetKey, iNumPlanets, piTotalMin, piTotalFuel,
        pvGoodColor, pvBadColor, vMaxAgRatio.GetFloat());

    RETURN_ON_ERROR(iErrCode);

    ////////////////////////////////////////////////////////////////////////////
    // Make ships move and update their current BR's as per maintenance ratio //
    ////////////////////////////////////////////////////////////////////////////

    iErrCode = MoveShips (iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, pvEmpireName, 
        pfMaintRatio, strGameMap, pstrEmpireShips, pstrEmpireDip, pstrEmpireMap, pstrEmpireFleets, 
        pstrEmpireData, pstrUpdateMessage, pvGoodColor, pvBadColor, gcConfig, iGameClassOptions);

    RETURN_ON_ERROR(iErrCode);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Make ships fight.  Also check for Minefields and first contact with opponent's ships //
    //////////////////////////////////////////////////////////////////////////////////////////

    // Randomize planet list
    Algorithm::Randomize <unsigned int>(piPlanetKey, iNumPlanets);

    // Loop through all planets
    for (i = 0; i < iNumPlanets; i ++)
    {
        iErrCode = MakeShipsFight (iGameClass, iGameNumber, strGameMap, iNumEmpires, piEmpireKey, pbAlive, 
            pvEmpireName, pstrEmpireShips, pstrEmpireDip, pstrEmpireMap, pstrUpdateMessage, pstrEmpireData, 
            pfFuelRatio, piPlanetKey[i], piTotalMin, piTotalFuel, bIndependence, strIndependentShips,
            pvGoodColor, pvBadColor, gcConfig);

        RETURN_ON_ERROR(iErrCode);
    }

    ///////////////////////////////////////////
    // Make minefields set to detonate do so //
    ///////////////////////////////////////////

    iErrCode = MakeMinefieldsDetonate (iGameClass, iGameNumber, strGameMap, iNumEmpires, piEmpireKey, pbAlive, 
        pvEmpireName, pstrEmpireShips, pstrEmpireMap, pstrUpdateMessage, pstrEmpireData, 
        piTotalMin, piTotalFuel, bIndependence, strIndependentShips, pvGoodColor, pvBadColor, gcConfig);

    RETURN_ON_ERROR(iErrCode);

    ////////////////////////////////////////////////
    // Make ships/fleets perform special actions  //
    ////////////////////////////////////////////////

    // Array of keys of nuking ships
    ppiShipNukeKey = (unsigned int**)StackAlloc(iNumPlanets * 2 * sizeof (unsigned int*));
    memset(ppiShipNukeKey, 0, iNumPlanets * 2 * sizeof (unsigned int*));

    // Array of empires that own the nuking ships
    ppiEmpireNukeKey = ppiShipNukeKey + iNumPlanets;
    
    // Number of nuking ships per planet
    piNumNukingShips = (unsigned int*)StackAlloc(iNumPlanets * 2 * sizeof (unsigned int));

    // Keys of nuked planets
    piNukedPlanetKey = piNumNukingShips + iNumPlanets;

    /////////////////////////////
    // Perform special actions //
    /////////////////////////////

    iErrCode = PerformSpecialActions (iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pvGoodColor,
        pvBadColor, pvEmpireName, 
        pbAlive, iNumPlanets, piPlanetKey, piOriginalPlanetOwner, piOriginalNumObliterations,
        pstrEmpireShips, pstrEmpireFleets, pstrEmpireData, pstrEmpireMap, 
        pstrUpdateMessage, strGameMap, strGameData, piTotalAg, piTotalMin, piTotalFuel, pstrEmpireDip, 
        piObliterator, 
        piObliterated, &iNumObliterations, pszGameClassName, iNewUpdateCount, iGameClassOptions,
        ppiShipNukeKey, ppiEmpireNukeKey, piNukedPlanetKey, piNumNukingShips, &iNumNukedPlanets, gcConfig);

    RETURN_ON_ERROR(iErrCode);

    ///////////////////////////
    // Handle fleet movement //
    ///////////////////////////

    iErrCode = UpdateFleetOrders (iNumEmpires, piEmpireKey, pbAlive, strGameMap, pstrEmpireShips, 
        pstrEmpireFleets, pstrEmpireMap, pstrUpdateMessage);
    RETURN_ON_ERROR(iErrCode);

    /////////////////////
    // Make gates work //
    /////////////////////

    iErrCode = ProcessGates (iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, 
        pstrUpdateMessage, pvGoodColor, pvBadColor, pvEmpireName, piOriginalPlanetOwner,
        piOriginalNumObliterations, pstrEmpireShips, pstrEmpireFleets, 
        pstrEmpireMap, pstrEmpireData, pstrEmpireDip, strGameMap, gcConfig, iGameClassOptions);
    
    RETURN_ON_ERROR(iErrCode);

    ///////////////////
    // Process nukes //
    ///////////////////

    // Deletes ppiShipNukeKey and ppiEmpireNukeKey
    iErrCode = ProcessNukes (iNumEmpires, piEmpireKey, pbAlive, pbSendFatalMessage, pszGameClassName, 
        iGameClass, iGameNumber, 
        piTotalAg, piTotalMin, piTotalFuel, iNumNukedPlanets, piNumNukingShips, piNukedPlanetKey, 
        ppiEmpireNukeKey, ppiShipNukeKey, piObliterator, piObliterated, &iNumObliterations, pvEmpireName, 
        pstrEmpireDip, pstrEmpireShips, pstrEmpireMap, pstrUpdateMessage, pstrEmpireData, strGameMap, 
        iNewUpdateCount, gcConfig);

    RETURN_ON_ERROR(iErrCode);

    ////////////////////////
    // Add ship sightings //
    ////////////////////////

    iErrCode = AddShipSightings (iNumEmpires, piEmpireKey, pbAlive, pstrUpdateMessage, pvEmpireName, 
        bIndependence, iNumPlanets, piPlanetKey, strGameMap, pstrEmpireMap, pstrEmpireShips, 
        strIndependentShips);

    RETURN_ON_ERROR(iErrCode);

    /////////////////////////////////////////////////////////////////////////////
    // Update empires' econ, mil, maxecon, maxmil, totalbuild, end turn status //
    /////////////////////////////////////////////////////////////////////////////

    iErrCode = UpdateEmpiresEcon (iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, piTotalMin, 
        piTotalFuel, piTotalAg, iUpdatePeriod, tUpdateTime, strGameData, pstrEmpireDip, pstrEmpireData, 
        pstrEmpireShips, iNewUpdateCount, strGameMap, vMaxAgRatio.GetFloat(),
        gcConfig);

    RETURN_ON_ERROR(iErrCode);

    //////////////////////////////
    // Process subjective views //
    //////////////////////////////

    if (iGameClassOptions & SUBJECTIVE_VIEWS) {
        
        iErrCode = ProcessSubjectiveViews (
            iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, strGameMap, pstrEmpireMap, 
            pstrEmpireDip, pstrEmpireShips
            );
        RETURN_ON_ERROR(iErrCode);
    }

    ////////////////////////////////////////////////
    // Check for one player left in a closed game //
    ////////////////////////////////////////////////

    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameState = vTemp.GetInteger();
    
    if (!(iGameState & STILL_OPEN))
    {
        unsigned int iNumGameEmpires;        
        iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumGameEmpires);
        RETURN_ON_ERROR(iErrCode);

        if (iNumGameEmpires < 2)
        {
            //////////////////////////////////
            // Game over, we have a winner! //
            //////////////////////////////////

            // Determine which empire is still alive
            for (j = 0; j < iNumEmpires; j ++) {
                if (pbAlive[j]) {
                    break;
                }
            }

            Assert(j < iNumEmpires);
                
            // Send victory message to the remaining empire
            char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];
            sprintf (
                pszMessage, 
                "Congratulations! You have won %s %i", 
                pszGameClassName,
                iGameNumber
                );
                
            iErrCode = SendSystemMessage (piEmpireKey[j], pszMessage, SYSTEM, MESSAGE_SYSTEM);
            RETURN_ON_ERROR(iErrCode);
                
            // Add win to empires' statistics
            iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, piEmpireKey[j]);
            RETURN_ON_ERROR(iErrCode);

            // Game over
            *pbGameOver = true;

            iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_WIN);
            RETURN_ON_ERROR(iErrCode);

            return iErrCode;
        }
    }

    //////////////
    // Idleness //
    //////////////
    {
        unsigned int iNumUpdatedEmpires = 0, iSurvivorIndex = NO_KEY;

        // Number of empires who are idle enough to make the game ruin 
        unsigned int iGameRuinIdlers = 0;
        
        // Number of empires blocking the game from ruining under complex rules
        unsigned int iComplexAwake = 0;

        // Number of idle and resigned empires in the game
        unsigned int iNumIdleEmpires = 0, iNumResignedEmpires = 0;

        bool* pbNewlyIdle = (bool*) StackAlloc (3 * iNumEmpires * sizeof (bool));
        bool* pbResigned = pbNewlyIdle + iNumEmpires;
        bool* pbRuinEmpireUpdated = pbResigned + iNumEmpires;

        int* piRuinEmpire = (int*) StackAlloc (iNumEmpires * sizeof (int));

        int iNumIdleUpdatesForRuin, iNumUpdatesForIdle, iGameState;

        memset (pbRuinEmpireUpdated, false, iNumEmpires * sizeof (bool));
        memset (pbNewlyIdle, false, iNumEmpires * sizeof (bool));

        // Get idle updates for idle
        iErrCode = t_pCache->ReadData(
            SYSTEM_GAMECLASS_DATA,
            iGameClass,
            SystemGameClassData::NumUpdatesForIdle,
            &vTemp
            );

        RETURN_ON_ERROR(iErrCode);

        iNumUpdatesForIdle = vTemp.GetInteger();

        // Get idle updates for ruin
        iErrCode = t_pCache->ReadData(
            SYSTEM_GAMECLASS_DATA,
            iGameClass,
            SystemGameClassData::NumUpdatesForRuin,
            &vTemp
            );

        RETURN_ON_ERROR(iErrCode);
        iNumIdleUpdatesForRuin = vTemp.GetInteger();

        // Get ruin behavior
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::RuinFlags, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        int iRuinFlags = vTemp.GetInteger();

        bool bSimpleRuins = iRuinFlags == RUIN_CLASSIC_SC;
        bool bComplexRuins = iRuinFlags == RUIN_ALMONASTER;
        Assert(bSimpleRuins || bComplexRuins || iRuinFlags == 0);

        char pszDead [128 + MAX_EMPIRE_NAME_LENGTH];
        char pszMessage [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH] = "";
        
        for (i = 0; i < iNumEmpires; i ++) {

            if (!pbAlive[i]) {
                continue;
            }

            // Get empire options
            iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::Options, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            int iEmpireOptions = vTemp.GetInteger();

            pbResigned[i] = (iEmpireOptions & RESIGNED) != 0;
            if (pbResigned[i]) {

                iErrCode = t_pCache->WriteOr(pstrEmpireData[i], GameEmpireData::Options, UPDATED);
                RETURN_ON_ERROR(iErrCode);

                // Resigned empires disallow complex ruins, except when everyone is either resigned or idle
                iNumResignedEmpires ++;
                iComplexAwake ++;
                iGameRuinIdlers ++;
                iNumUpdatedEmpires ++;
            }

            else if (iNewUpdateCount > 1 && !(iEmpireOptions & LOGGED_IN_THIS_UPDATE)) {

                iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::NumUpdatesIdle, 1, &vTemp);
                RETURN_ON_ERROR(iErrCode);

                int iNumUpdatesIdle = vTemp.GetInteger() + 1;
                if (iNumUpdatesIdle >= NUM_UPDATES_FOR_GAME_RUIN) {
                    iGameRuinIdlers ++;
                }

                // Check for idle empire
                if (iNumUpdatesIdle >= iNumUpdatesForIdle) {

                    iErrCode = t_pCache->WriteOr(pstrEmpireData[i], GameEmpireData::Options, UPDATED);
                    RETURN_ON_ERROR(iErrCode);

                    iNumUpdatedEmpires ++;
                    iNumIdleEmpires ++;

                    if (iNumUpdatesIdle == iNumUpdatesForIdle) {
                        pbNewlyIdle[i] = true;
                    }

                } else {

                    // The empire is only mildly idle
                    if (iEmpireOptions & UPDATED) {

                        iErrCode = t_pCache->WriteAnd(pstrEmpireData[i], GameEmpireData::Options, ~UPDATED);
                        RETURN_ON_ERROR(iErrCode);
                    }
                }

                // Check for empire that would ruin
                if (iNumUpdatesIdle >= iNumIdleUpdatesForRuin) {

                    if (bSimpleRuins) {
                        
                        piRuinEmpire[iNumRuins ++] = i;

                        // Decrement num updated empires if necessary
                        if (iNumUpdatesIdle >= iNumUpdatesForIdle) {
                            iNumUpdatedEmpires --;
                        }
                    }

                } else {

                    iSurvivorIndex = i;
                    iComplexAwake ++;
                }

            } else {

                // Set num updates idle to zero
                iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::NumUpdatesIdle, 0);
                RETURN_ON_ERROR(iErrCode);

                // Turn off the logged-in-this update flag
                iErrCode = t_pCache->WriteAnd(pstrEmpireData[i], GameEmpireData::Options, ~LOGGED_IN_THIS_UPDATE);
                RETURN_ON_ERROR(iErrCode);

                iSurvivorIndex = i;
                iComplexAwake ++;

                if (iEmpireOptions & UPDATED) {

                    iErrCode = t_pCache->WriteAnd(pstrEmpireData[i], GameEmpireData::Options, ~UPDATED);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }   // End empire loop

        // Complex ruins:
        // Only ruin game with one empire alive if at one point in the game there were more than two
        bool bRuinGame = false;
        if (bComplexRuins && iComplexAwake < 2) {

            iErrCode = t_pCache->ReadData(strGameData, GameData::MaxNumEmpires, &vTemp);
            RETURN_ON_ERROR(iErrCode);

            if (vTemp.GetInteger() > 2) {
                bRuinGame = true;
            }
        }

        // Check for end game because of ruins
        if (
            (bComplexRuins && bRuinGame) ||                     // Complex game ruin 
            (iGameRuinIdlers == iNumEmpires) ||                 // All empires idle for game ruin
            (bSimpleRuins && iNumRuins >= iNumEmpires - 1)      // Simple game ruin
            ) {

            // If there's a survivor, award him a win
            if (iSurvivorIndex != NO_KEY) {
                
                // Send victory message to the remaining empire
                sprintf (
                    pszMessage, 
                    "Congratulations! You have won %s %i. The game ruined and you were the last remaining empire awake",
                    pszGameClassName,
                    iGameNumber
                    );

                iErrCode = SendSystemMessage (piEmpireKey[iSurvivorIndex], pszMessage, SYSTEM, MESSAGE_SYSTEM);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, piEmpireKey[iSurvivorIndex]);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[iSurvivorIndex], EMPIRE_GAME_ENDED, NULL);
                RETURN_ON_ERROR(iErrCode);
            }

            // Remove all remaining resigned empires - they shouldn't ruin
            for (i = 0; i < iNumEmpires; i ++) {

                if (pbAlive[i] && pbResigned[i]) {

                    iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[i], EMPIRE_RESIGNED, NULL);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
            
            // Ruin all remaining empires
            const char* pszWinnerName = NULL;
            if (iSurvivorIndex != NO_KEY) {
                pszWinnerName = pvEmpireName[iSurvivorIndex].GetCharPtr();
            }

            iErrCode = RuinGame (iGameClass, iGameNumber, pszWinnerName);
            RETURN_ON_ERROR(iErrCode);
            
            // Game over
            *pbGameOver = true;
            return iErrCode;
        }

        // The game lives on, so process ruined empires
        if (iNumRuins > 0)
        {
            GameUpdateInformation guInfo = 
            {
                iNewUpdateCount,
                iNumEmpires,
                (int*) piEmpireKey,
                pbAlive,
                pstrUpdateMessage
            };

            sprintf(pszMessage, "You ruined out of %s %i", pszGameClassName, iGameNumber);
            for (i = 0; i < iNumRuins; i ++) {

                int iIndex = piRuinEmpire[i];
                
                iErrCode = RuinEmpire (iGameClass, iGameNumber, piEmpireKey[iIndex], pszMessage);
                RETURN_ON_ERROR(iErrCode);
                
                pbAlive[iIndex] = false;
                pbSendFatalMessage[iIndex] = false; // Don't send on ruin
            }

            // Delete ruined empires from game
            for (i = 0; i < iNumRuins; i ++) {

                int iIndex = piRuinEmpire[i];
                
                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[iIndex], EMPIRE_RUINED, &guInfo);
                RETURN_ON_ERROR(iErrCode);

                sprintf (
                    pszDead,
                    BEGIN_STRONG "%s has fallen into ruin" END_STRONG "\n",
                    pvEmpireName[iIndex].GetCharPtr()
                    );
                
                for (j = 0; j < iNumEmpires; j ++) {
                    
                    if (!pbAlive[j]) {
                        continue;
                    }
                    
                    pstrUpdateMessage[j] += pszDead;
                }
            }

            iNumIdleEmpires -= iNumRuins;
            Assert(iNumIdleEmpires >= 0);

        }   // End if ruins occurred

        // Newly idle empires request pause and draw
        for (i = 0; i < iNumEmpires; i ++) {

            if (!pbNewlyIdle[i] || !pbAlive[i]) {
                continue;
            }

            bool bFlag;
            if (iGameClassOptions & ALLOW_DRAW) {

                iErrCode = IsEmpireRequestingDraw (iGameClass, iGameNumber, piEmpireKey[i], &bFlag);
                RETURN_ON_ERROR(iErrCode);

                if (!bFlag) {

                    iErrCode = RequestDraw (iGameClass, iGameNumber, piEmpireKey[i], &iGameState);
                    RETURN_ON_ERROR(iErrCode);
                    iNumNewDraws ++;
                }
            }

            iErrCode = IsEmpireRequestingPause (iGameClass, iGameNumber, piEmpireKey[i], &bFlag);
            RETURN_ON_ERROR(iErrCode);

            if (!bFlag) {

                iErrCode = RequestPauseDuringUpdate (iGameClass, iGameNumber, piEmpireKey[i]);
                RETURN_ON_ERROR(iErrCode);
            }
        }   // End for each empire

        // Set updated empires
        iErrCode = t_pCache->WriteData(strGameData, GameData::NumEmpiresUpdated, (int)iNumUpdatedEmpires);
        RETURN_ON_ERROR(iErrCode);

        // Notify world of idle empires
        if (iNumIdleEmpires > 0) {

            if (iNumIdleEmpires == 1) {
                strMessage = "There is " BEGIN_STRONG "1" END_STRONG " idle empire in the game\n";
            } else {
                strMessage = "There are " BEGIN_STRONG;
                strMessage += iNumIdleEmpires;
                strMessage += END_STRONG " idle empires in the game\n";
            }

            for (j = 0; j < iNumEmpires; j ++) {

                if (pbAlive[j]) {
                    pstrUpdateMessage[j] += strMessage;
                }
            }
        }

        // Notify world of resigned empires
        if (iNumResignedEmpires > 0) {

            if (iNumResignedEmpires == 1) {
                strMessage = "There is " BEGIN_STRONG "1" END_STRONG " resigned empire in the game\n";
            } else {
                strMessage = "There are " BEGIN_STRONG;
                strMessage += iNumResignedEmpires;
                strMessage += END_STRONG " resigned empires in the game\n";
            }

            for (j = 0; j < iNumEmpires; j ++) {

                if (pbAlive[j]) {
                    pstrUpdateMessage[j] += strMessage;
                }
            }
        }
    }


    /////////////////////////////////////////////////////////////////////////////
    // Check for ally out and draw out if someone died and the game has closed //
    /////////////////////////////////////////////////////////////////////////////

    if (!(iGameState & STILL_OPEN) && iNumObliterations + iNumRuins + iNumNewDraws > 0) {

        iErrCode = CheckGameForEndConditions (iGameClass, iGameNumber, NULL, pbGameOver);
        RETURN_ON_ERROR(iErrCode);

        if (*pbGameOver)
        {
            return iErrCode;
        }
    }

    ////////////////////////
    // List obliterations //
    ////////////////////////

    if (iNumObliterations > 0) {

        strMessage.Clear();
        for (i = 0; i < iNumObliterations; i ++) {
            
            strMessage += BEGIN_STRONG;
            strMessage += pvEmpireName [piObliterator[i]].GetCharPtr();
            strMessage += " has obliterated ";
            strMessage += pvEmpireName [piObliterated[i]].GetCharPtr();
            strMessage += END_STRONG "\n";
        }

        for (i = 0; i < iNumEmpires; i ++) {
            pstrUpdateMessage[i] += strMessage;
        }
    }
    
    /////////////////////
    // List surrenders //
    /////////////////////
    
    if (iNumSurrenders > 0) {
        
        strMessage.Clear();

        for (i = 0; i < iNumSurrenders; i ++) {

            for (j = 0; j < i; j ++) {

                if (piLoser[i] == piLoser[j] ||
                    piWinner[i] == piLoser[j]) {
                    break;
                }
            }

            if (j == i) {
                strMessage += BEGIN_STRONG;
                strMessage += pvEmpireName[piLoser[i]].GetCharPtr();
                strMessage += " surrendered to ";
                strMessage += pvEmpireName[piWinner[i]].GetCharPtr();
                strMessage += END_STRONG "\n";
            }
        }

        for (i = 0; i < iNumEmpires; i ++) {
            pstrUpdateMessage[i] += strMessage;
        }
    }

    if (bIndependence && (iNumObliterations > 0 || iNumSurrenders > 0)) {

        if (iNumObliterations + iNumSurrenders > 1) {
            strMessage = "The planets and ships belonging to the dead empires are now independent\n";
        } else {
            strMessage = "The planets and ships belonging to the dead empire are now independent\n";
        }

        for (i = 0; i < iNumEmpires; i ++) {
            pstrUpdateMessage[i] += strMessage;
        }
    }

#ifdef _DEBUG
    iErrCode = VerifyUpdatedEmpireCount (iGameClass, iGameNumber);
    Assert(iErrCode == OK);
#endif

    /////////////////////////////
    // Update last update time //
    /////////////////////////////
    
    iErrCode = t_pCache->WriteData(strGameData, GameData::LastUpdateTime, tUpdateTime);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strGameData, GameData::RealLastUpdateTime, tUpdateTime);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->WriteData(strGameData, GameData::LastUpdateCheck, tUpdateTime);
    RETURN_ON_ERROR(iErrCode);

    /////////////////////////////////
    // Increment number of updates //
    /////////////////////////////////
    
    iErrCode = t_pCache->Increment (strGameData, GameData::NumUpdates, 1);
    RETURN_ON_ERROR(iErrCode);

    char pszUpdateReport [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH];
    sprintf (
        pszUpdateReport,
        "%s %i ran update %i",
        pszGameClassName,
        iGameNumber,
        iNewUpdateCount
        );

    global.GetReport()->WriteReport (pszUpdateReport);

    if (iGameState & PAUSED) {

        // The game updated because everyone was ready for the update
        iErrCode = t_pCache->WriteData(strGameData, GameData::SecondsUntilNextUpdateWhilePaused, iUpdatePeriod);
        RETURN_ON_ERROR(iErrCode);

    } else {

        // Check if we need to pause the game in the aftermath of the update
        bool bNewlyPaused;
        iErrCode = CheckForDelayedPause (iGameClass, iGameNumber, tUpdateTime, &bNewlyPaused);
        RETURN_ON_ERROR(iErrCode);

        if (bNewlyPaused) {

            for (i = 0; i < iNumEmpires; i ++) {
                pstrUpdateMessage[i] += "The game is now paused\n";
            }
        }
    }
    
    //////////////////////////
    // Send update messages //
    //////////////////////////
    
    for (i = 0; i < iNumEmpires; i ++) {

        if (pbAlive[i]) {
            
            iErrCode = SendGameMessage (
                iGameClass, 
                iGameNumber, 
                piEmpireKey[i], 
                pstrUpdateMessage[i], 
                SYSTEM,
                MESSAGE_UPDATE | MESSAGE_SYSTEM,
                tUpdateTime
                );

            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Send fatal messages, even if the game ended
    for (i = 0; i < iNumEmpires; i ++)
    {
        if (!pbAlive[i] && pbSendFatalMessage[i])
        {
            // TODOTODO - could this fail because an empire marked for deletion no longer exists?
            iErrCode = SendFatalUpdateMessage(iGameClass, iGameNumber, piEmpireKey[i], pszGameClassName, pstrUpdateMessage[i]);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

int GameEngine::UpdateDiplomaticStatus (int iGameClass, int iGameNumber, unsigned int iNumEmpires, unsigned int* piEmpireKey, 
                                        bool* pbAlive, bool* pbSendFatalMessage, Variant* pvEmpireName, 
                                        String* pstrUpdateMessage, 
                                        const char** pstrEmpireDip, const char** pstrEmpireMap, 
                                        const char* strGameMap, 
                                        const char** pstrEmpireData, int* piWinner, int* piLoser, 
                                        unsigned int* piNumSurrenders, const char* pszGameClassName,
                                        int iNewUpdateCount, Variant* pvGoodColor, Variant* pvBadColor) {
    
    Variant vOffer, vOldStatus, vReceive, vTemp, vMinX, vMaxX, vMinY, vMaxY, vNeighbourPlanetKey, vCoord;

    unsigned int iKey1, iKey2, iKey;
    String strMessage;

    int i, j, k, iErrCode, iFinal, iNumKeys, iDipLevel, iOldOffer, iOldReceive, iMapSharedDip;

    bool bInvisibleDiplomacy, bPermanentAlliances;

    *piNumSurrenders = 0;

    // Options
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vTemp
        );

    RETURN_ON_ERROR(iErrCode);

    bInvisibleDiplomacy = (vTemp.GetInteger() & VISIBLE_DIPLOMACY) == 0;
    bPermanentAlliances = (vTemp.GetInteger() & PERMANENT_ALLIANCES) != 0;

    // Maps shared
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MapsShared, 
        &vTemp
        );

    RETURN_ON_ERROR(iErrCode);
    
    iMapSharedDip = vTemp.GetInteger();

    // Get dip level
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vTemp
        );
    RETURN_ON_ERROR(iErrCode);
                
    iDipLevel = vTemp.GetInteger();

    // Loop through all empires
    for (i = 0; i < (int) iNumEmpires; i ++)
    {
        if (!pbAlive[i])
            continue;

        unsigned int* piProxyKey = NULL;
        AutoFreeKeys free_piProxyKey(piProxyKey);

        Variant* pvTemp = NULL;
        AutoFreeData free_pvTemp(pvTemp);
        
        // Get the empire's acquaintances
        iErrCode = t_pCache->ReadColumn(
            pstrEmpireDip[i], 
            GameEmpireDiplomacy::ReferenceEmpireKey, 
            &piProxyKey, 
            &pvTemp,
            (unsigned int*) &iNumKeys
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);

        iErrCode = OK;
        
        // Evaluate all matches
        for (j = 0; j < iNumKeys; j ++) {
            
            // If empire has already been processed, don't process again
            for (k = 0; k < i; k ++) {
                if ((int) piEmpireKey[k] == pvTemp[j].GetInteger()) {
                    break;
                }
            }

            if (k != i) {
                continue;
            }

            // Otherwise, find empire index
            for (k = i; k < (int) iNumEmpires; k ++) {
                if ((int) piEmpireKey[k] == pvTemp[j].GetInteger()) {
                    break;
                }
            }

            Assert(k < (int) iNumEmpires);
            
            if (!pbAlive[k]) {
                continue;
            }
            
            iKey1 = piProxyKey[j];

            iErrCode = t_pCache->ReadData(pstrEmpireDip[i], iKey1, GameEmpireDiplomacy::DipOffer, &vOffer);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->GetFirstKey(pstrEmpireDip[k], GameEmpireDiplomacy::ReferenceEmpireKey, piEmpireKey[i], &iKey2);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(pstrEmpireDip[k], iKey2, GameEmpireDiplomacy::DipOffer, &vReceive);
            RETURN_ON_ERROR(iErrCode);

            // Cache surrenders for later processing
            if (vReceive.GetInteger() == SURRENDER && vOffer.GetInteger() == ACCEPT_SURRENDER) {
                piWinner [*piNumSurrenders] = i;
                piLoser [*piNumSurrenders] = k;
                (*piNumSurrenders) ++;
                continue;
            }

            if (vReceive.GetInteger() == ACCEPT_SURRENDER && vOffer.GetInteger() == SURRENDER) {
                piWinner [*piNumSurrenders] = k;
                piLoser [*piNumSurrenders] = i;
                (*piNumSurrenders) ++;
                continue;
            }

            //
            // Decide status
            //

            iErrCode = t_pCache->ReadData(
                pstrEmpireDip[i], 
                iKey1, 
                GameEmpireDiplomacy::CurrentStatus, 
                &vOldStatus
                );

            RETURN_ON_ERROR(iErrCode);

            iFinal = GetNextDiplomaticStatus(vOffer.GetInteger(), vReceive.GetInteger(), vOldStatus.GetInteger());

            // Write final status
            if (iFinal != vOldStatus.GetInteger())
            {
                // Update current status
                iErrCode = t_pCache->WriteData(pstrEmpireDip[i], iKey1, GameEmpireDiplomacy::CurrentStatus, iFinal);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->WriteData(pstrEmpireDip[k], iKey2, GameEmpireDiplomacy::CurrentStatus, iFinal);
                RETURN_ON_ERROR(iErrCode);

                // Handle downgrades
                if (iFinal < vOldStatus.GetInteger())
                {
                    Assert(iFinal != ALLIANCE);
                    iOldOffer = vOffer.GetInteger();

                    if (iOldOffer != iFinal) {

                        switch (iOldOffer) {

                        case TRADE:

                            if (iFinal == WAR && GameAllowsDiplomacy (iDipLevel, TRUCE))
                            {                          
                                vOffer = TRUCE;
                            }
                            break;
                            
                        case ALLIANCE:
                            
                            switch (iFinal) {

                            case WAR:

                                if (GameAllowsDiplomacy (iDipLevel, TRUCE) && !GameAllowsDiplomacy (iDipLevel, TRADE))
                                {
                                    Assert(!bPermanentAlliances || vOldStatus.GetInteger() != ALLIANCE);
                                    vOffer = TRUCE;
                                }
                                else if (GameAllowsDiplomacy (iDipLevel, TRADE) && !GameAllowsDiplomacy (iDipLevel, TRUCE))
                                {
                                    vOffer = TRADE;
                                }
                                break;
                                
                            case TRUCE:
                                
                                if (GameAllowsDiplomacy (iDipLevel, TRADE))
                                {
                                    vOffer = TRADE;
                                }
                                break;
                            }
                            break;
                        }

                        if (iOldOffer != vOffer.GetInteger())
                        {
                            iErrCode = t_pCache->WriteData(pstrEmpireDip[i], iKey1, GameEmpireDiplomacy::DipOffer, vOffer);
                            RETURN_ON_ERROR(iErrCode);
                        }
                    }

                    iOldReceive = vReceive.GetInteger();

                    if (iOldReceive != iFinal) {

                        switch (iOldReceive) {

                        case TRADE:

                            if (iFinal == WAR && GameAllowsDiplomacy (iDipLevel, TRUCE))
                            {
                                vReceive = TRUCE;
                            }
                            break;
                            
                        case ALLIANCE:
                            
                            switch (iFinal) {
                                
                            case WAR:
                                
                                if (GameAllowsDiplomacy (iDipLevel, TRUCE) && !GameAllowsDiplomacy (iDipLevel, TRADE))
                                {
                                    Assert(!bPermanentAlliances || vOldStatus.GetInteger() != ALLIANCE);
                                    vReceive = TRUCE;
                                }
                                else if (GameAllowsDiplomacy (iDipLevel, TRADE) && !GameAllowsDiplomacy (iDipLevel, TRUCE))
                                {
                                    vReceive = TRADE;
                                }
                                break;
                                
                            case TRUCE:
                                
                                if (GameAllowsDiplomacy (iDipLevel, TRADE))
                                {
                                    vReceive = TRADE;
                                }
                                break;
                            }
                            break;
                        }

                        if (iOldReceive != vReceive.GetInteger())
                        {
                            iErrCode = t_pCache->WriteData(pstrEmpireDip[k], iKey2, GameEmpireDiplomacy::DipOffer, vReceive);
                            RETURN_ON_ERROR(iErrCode);
                        }
                    }
                    
                    if (vOldStatus.GetInteger() == ALLIANCE)
                    {
                        // Set once allied with bits
                        iErrCode = t_pCache->WriteOr(pstrEmpireDip[i], iKey1, GameEmpireDiplomacy::State, ONCE_ALLIED_WITH);
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->WriteOr(pstrEmpireDip[k], iKey2, GameEmpireDiplomacy::State, ONCE_ALLIED_WITH);
                        RETURN_ON_ERROR(iErrCode);
                    }

                }   // End if status is less than previous update

                // Shared maps?
                if (iMapSharedDip != NO_DIPLOMACY) {
                    
                    // Add to update messages if we're no longer sharing planets
                    if (vOldStatus.GetInteger() >= iMapSharedDip && iFinal < iMapSharedDip) {
                        
                        pstrUpdateMessage[i] += "You are no longer sharing your map with " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[k].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG "\n";
                        
                        pstrUpdateMessage[k] += "You are no longer sharing your map with " BEGIN_STRONG;
                        pstrUpdateMessage[k] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[k] += END_STRONG "\n";
                    }
                    
                    // Share planets if we've reached the current level from a lower dip level
                    else if (iFinal >= iMapSharedDip && vOldStatus.GetInteger() < iMapSharedDip) {
                        
                        // Add to update messages
                        pstrUpdateMessage[i] += "You are now sharing your map with " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[k].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG "\n";
                        
                        pstrUpdateMessage[k] += "You are now sharing your map with " BEGIN_STRONG;
                        pstrUpdateMessage[k] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[k] += END_STRONG "\n";
                        
                        iErrCode = SharePlanetsBetweenFriends (
                            iGameClass,
                            iGameNumber,
                            i,
                            k,
                            pstrEmpireMap, 
                            pstrEmpireDip, 
                            pstrEmpireData, 
                            strGameMap, 
                            iNumEmpires, 
                            piEmpireKey, 
                            iMapSharedDip,
                            true
                            );
                        
                        RETURN_ON_ERROR(iErrCode);
                    }
                }

            }   // End if status changed from previous update
            
            // Update messages
            if (iFinal == ALLIANCE) {
                pstrUpdateMessage[i] += BEGIN_GOOD_FONT(i);
                pstrUpdateMessage[k] += BEGIN_GOOD_FONT(k);
            }
            else if (iFinal == WAR) {
                pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                pstrUpdateMessage[k] += BEGIN_BAD_FONT(k);
            }

            pstrUpdateMessage[i] += BEGIN_STRONG;
            pstrUpdateMessage[k] += BEGIN_STRONG;

            pstrUpdateMessage[i] += DIP_STRING (iFinal);
            pstrUpdateMessage[k] += DIP_STRING (iFinal);

            pstrUpdateMessage[i] += " with ";
            pstrUpdateMessage[k] += " with ";

            pstrUpdateMessage[i] += pvEmpireName[k].GetCharPtr();
            pstrUpdateMessage[k] += pvEmpireName[i].GetCharPtr();
            
            if (iFinal == ALLIANCE || iFinal == WAR) {
                pstrUpdateMessage[i] += END_FONT;
                pstrUpdateMessage[k] += END_FONT;
            }
            pstrUpdateMessage[i] += END_STRONG "\n";
            pstrUpdateMessage[k] += END_STRONG "\n";
            
            iErrCode = t_pCache->WriteData(pstrEmpireDip[i], iKey1, GameEmpireDiplomacy::DipOfferLastUpdate, vOffer);
            RETURN_ON_ERROR(iErrCode);
                
            iErrCode = t_pCache->WriteData(pstrEmpireDip[k], iKey2, GameEmpireDiplomacy::DipOfferLastUpdate, vReceive);
            RETURN_ON_ERROR(iErrCode);
            
        }   // End all acquaintances loop

    }   // End all empires loop
    

    if (*piNumSurrenders > 0) {

        /////////////////////////////////////////
        // We treat surrenders just like nukes //
        /////////////////////////////////////////

        GameUpdateInformation guInfo = { iNewUpdateCount, iNumEmpires, (int*) piEmpireKey, pbAlive, pstrUpdateMessage };
        
        // Randomize surrenders
        if (*piNumSurrenders > 1)
        {
            Algorithm::RandomizeTwo<int, int> (piLoser, piWinner, *piNumSurrenders);
        }
        
        int iWinner, iLoser;
        for (i = 0; i < (int) *piNumSurrenders; i ++) {
            
            // Get empire indices
            iLoser = piLoser[i];
            iWinner = piWinner[i];

            if (!pbAlive[iLoser] || !pbAlive[iWinner]) {

                // Remove surrender from arrays of winners and losers
                piLoser[i] = piLoser [*piNumSurrenders - 1];
                piWinner[i] = piWinner [*piNumSurrenders - 1];
                (*piNumSurrenders) --;

            } else {
                
                // Make sure they're still at war
                iErrCode = t_pCache->GetFirstKey(
                    pstrEmpireDip [iWinner],
                    GameEmpireDiplomacy::ReferenceEmpireKey,
                    piEmpireKey [iLoser],
                    &iKey
                    );
                
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(
                    pstrEmpireDip [iWinner],
                    iKey,
                    GameEmpireDiplomacy::CurrentStatus,
                    &vTemp
                    );

                RETURN_ON_ERROR(iErrCode);

                if (vTemp.GetInteger() != WAR) {

                    // Remove surrender from arrays of winners and losers
                    piLoser[i] = piLoser [*piNumSurrenders - 1];
                    piWinner[i] = piWinner [*piNumSurrenders - 1];
                    (*piNumSurrenders) --;

                } else {
                    
                    // Mark loser as dead
                    pbAlive[iLoser] = false;
                    pbSendFatalMessage[i] = false;  // Don't send on surrender
                    
                    // Update statistics
                    iErrCode = UpdateScoresOnSurrender (piEmpireKey[iWinner], piEmpireKey[iLoser], 
                        pvEmpireName[iWinner].GetCharPtr(), pvEmpireName[iLoser].GetCharPtr(), iGameClass, 
                        iGameNumber, iNewUpdateCount, pszGameClassName);
                    
                    RETURN_ON_ERROR(iErrCode);

                    // Delete empire's tables
                    iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[iLoser], EMPIRE_SURRENDERED, &guInfo);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }
    }

    return iErrCode;
}


int GameEngine::UpdatePlanetPopulations (int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, 
                                         float* pfAgRatio, const char* strGameMap, const char** pstrEmpireData, 
                                         const char** pstrEmpireMap, String* pstrUpdateMessage, 
                                         unsigned int* piPlanetKey, int iNumPlanets, int* piTotalMin, 
                                         int* piTotalFuel, Variant* pvGoodColor, Variant* pvBadColor,
                                         float fMaxAgRatio) {

    int i, j, iErrCode = OK, iNewPop, iX, iY, iColBuildCost;
    unsigned int iKey;

    Variant vOwner, vPlanetName, vPop, vMaxPop, vTemp, vCoord, vPopLostToColonies;

    for (i = 0; i < iNumPlanets; i ++) {
        
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Owner, &vOwner);

        switch (vOwner.GetInteger()) {
            
        case SYSTEM:
            
            //////////////////////////////////////////////
            // Check for end of annihilation quarantine //
            //////////////////////////////////////////////
            
            iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Annihilated, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            
            if (vTemp.GetInteger() != NOT_ANNIHILATED && vTemp.GetInteger() != ANNIHILATED_FOREVER) {
                
                iErrCode = t_pCache->Increment (strGameMap, piPlanetKey[i], GameMap::Annihilated, -1);
                RETURN_ON_ERROR(iErrCode);
                
                if (vTemp.GetInteger() == 1) {
                    
                    iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Name, &vPlanetName);
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Coordinates, &vCoord);
                    RETURN_ON_ERROR(iErrCode);
                    
                    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
                    
                    for (j = 0; j < iNumEmpires; j ++) {
                        
                        if (pbAlive[j]) {
                            
                            iErrCode = t_pCache->GetFirstKey(
                                pstrEmpireMap[j], 
                                GameEmpireMap::PlanetKey, 
                                piPlanetKey[i], 
                                &iKey
                                );

                            if (iErrCode == ERROR_DATA_NOT_FOUND)
                            {
                                iErrCode = OK;
                            }
                            RETURN_ON_ERROR(iErrCode);

                            pstrUpdateMessage[j] += BEGIN_GOOD_FONT(j);

                            AddPlanetNameAndCoordinates (
                                pstrUpdateMessage[j], 
                                vPlanetName.GetCharPtr(),
                                iX,
                                iY
                                );

                            pstrUpdateMessage[j] += " is now safe for colonization" END_FONT "\n";
                        }
                    }
                }
            }

            break;

        case INDEPENDENT:

            //////////////////////////////
            // Calculate new planet pop //
            //////////////////////////////

            iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Ag, &vTemp);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::MaxPop, &vMaxPop);
            RETURN_ON_ERROR(iErrCode);

            // Calculate new pop
            iNewPop = GetNextPopulation (vPop.GetInteger(), GetAgRatio (vTemp, vPop, fMaxAgRatio));

            if (iNewPop > vMaxPop.GetInteger()) {
                iNewPop = vMaxPop.GetInteger();
            }

            if (iNewPop < 0) {
                iNewPop = 0;
            }
            
            // Only update on population change
            if (iNewPop != vPop.GetInteger()) {

                if (iNewPop > MAX_POPULATION) {
                    iNewPop = MAX_POPULATION;
                }

                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::Pop, iNewPop);
                RETURN_ON_ERROR(iErrCode);
            }
            
            break;
            
        default:

            /////////////////////////
            // Apply colony builds //
            /////////////////////////

            // Find empire index
            GetEmpireIndex (j, vOwner);
            
            iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::PopLostToColonies, &vPopLostToColonies);
            RETURN_ON_ERROR(iErrCode);
            
            if (vPopLostToColonies.GetInteger() == 0) {
            
                iColBuildCost = 0;

            } else {

                iColBuildCost = vPopLostToColonies.GetInteger();
                Assert(iColBuildCost > 0);

                // Set pop lost back to zero
                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::PopLostToColonies, 0);
                RETURN_ON_ERROR(iErrCode);
            }
                
            //////////////////////////////
            // Calculate new planet pop //
            //////////////////////////////

            iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::MaxPop, &vMaxPop);
            RETURN_ON_ERROR(iErrCode);

            iNewPop = vPop.GetInteger() - iColBuildCost;

            // This is to address the possibility that somehow colony building might make a planet's
            // population go negative, although it really shouldn't happen because the build algorithm
            // prevents colonies from being built when that happens
            if (iNewPop < 0) {
                Assert(false);
                iNewPop = 0;
            }

            // Calculate new pop
            iNewPop = GetNextPopulation (iNewPop, pfAgRatio[j]);

            if (iNewPop > vMaxPop.GetInteger()) {
                iNewPop = vMaxPop.GetInteger();
            }

            // Only update on population change
            if (iNewPop != vPop.GetInteger()) {

                if (iNewPop > MAX_POPULATION) {
                    iNewPop = MAX_POPULATION;
                }
                
                // Write the new population
                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::Pop, iNewPop);
                RETURN_ON_ERROR(iErrCode);
                
                // Add pop to empire's total pop
                iErrCode = t_pCache->Increment (
                    pstrEmpireData[j], 
                    GameEmpireData::TotalPop, 
                    iNewPop - vPop.GetInteger()
                    );

                RETURN_ON_ERROR(iErrCode);
                
                /////////////////////////////
                // Handle resource changes //
                /////////////////////////////
                
                // Min
                iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Minerals, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                
                piTotalMin[j] += min (iNewPop, vTemp.GetInteger()) - min (vPop.GetInteger(), vTemp.GetInteger());
                
                // Fuel
                iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Fuel, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                
                piTotalFuel[j] += min (iNewPop, vTemp.GetInteger()) - min (vPop.GetInteger(), vTemp.GetInteger());
            }

            break;
        }
    }

    return iErrCode;
}

int GameEngine::MoveShips (int iGameClass, int iGameNumber, int iNumEmpires, unsigned int* piEmpireKey, 
                           bool* pbAlive, 
                           Variant* pvEmpireName, float* pfMaintRatio, const char* strGameMap, 
                           const char** pstrEmpireShips, const char** pstrEmpireDip, 
                           const char** pstrEmpireMap, 
                           const char** pstrEmpireFleets, const char** pstrEmpireData, 
                           String* pstrUpdateMessage, const Variant* pvGoodColor, const Variant* pvBadColor,
                           const GameConfiguration& gcConfig, int iGameClassOptions) {

    int i, k, l, m, iTemp, piExploredKey [NUM_CARDINAL_POINTS], iNumAcquaintances, iFleetKey = NO_KEY, 
        iErrCode = OK, iMaintCost, iFuelCost, iNewX, iNewY, iX, iY;

    unsigned int iProxyKey, iKey, iDestProxyKey;
    float fNewBR;

    Variant vAction, vCurBR, vMaxBR, vType, vShipName, vCoord, vPlanetName, vDestPlanetKey, 
        pvColData[GameEmpireMap::NumColumns],
        vNeighbourPlanetKey, vTemp, vDipStatus, vDipLevel, vDestPlanetName, vMaxX, vMinX, 
        vMaxY, vMinY, vFleetAction;

    String strTemp;
    bool bUpdated;

    unsigned int piNumBuilds [NUM_SHIP_TYPES];
    unsigned int piNumForceDismantled [NUM_SHIP_TYPES];
    unsigned int piNumVoluntaryDismantled [NUM_SHIP_TYPES];

    // Loop through all empires
    for (i = 0; i < (int) iNumEmpires; i ++) {
        
        if (!pbAlive[i])
            continue;

        unsigned int* piShipKey = NULL, iNumShips, j;
        AutoFreeKeys free_piShipKey(piShipKey);
            
        // Get keys for all empire's ships
        iErrCode = t_pCache->GetAllKeys (pstrEmpireShips[i], &piShipKey, &iNumShips);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
            continue;
        }
        RETURN_ON_ERROR(iErrCode);
            
        // Randomize ship list
        if (iNumShips > 1)
        {
            Algorithm::Randomize (piShipKey, iNumShips);
        }
        
        // Loop through all ships
        memset (piNumBuilds, 0, sizeof (piNumBuilds));
        memset (piNumForceDismantled, 0, sizeof (piNumBuilds));
        memset (piNumVoluntaryDismantled, 0, sizeof (piNumBuilds));
        
        for (j = 0; j < iNumShips; j ++) {
            
            bUpdated = false;
            
            ////////////////////
            // Update ship BR //
            ////////////////////

            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vType);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::CurrentBR, 
                &vCurBR
                );
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::MaxBR, 
                &vMaxBR
                );
            RETURN_ON_ERROR(iErrCode);
            
            fNewBR = vCurBR.GetFloat() * pfMaintRatio[i];
            
            // Check for ships with BR's too low to survive
            if (fNewBR < FLOAT_PROXIMITY_TOLERANCE) {
                
                // Destroy ship
                iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                RETURN_ON_ERROR(iErrCode);
                
                // Add to force dismantled count
                piNumForceDismantled [vType.GetInteger()] ++;
                continue;
            }

            // Don't increase beyond max br
            if (fNewBR > vMaxBR.GetFloat()) {
                fNewBR = vMaxBR.GetFloat();
            }
            
            // Write if changed
            if (fNewBR != vCurBR.GetFloat()) {
                
                Assert(fNewBR >= 0.0);
                
                iErrCode = t_pCache->WriteData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::CurrentBR, 
                    fNewBR
                    );

                RETURN_ON_ERROR(iErrCode);
            }
            
            ///////////////////////
            // Handle ship moves //
            ///////////////////////
            
            // Get ship's current action
            iErrCode = t_pCache->ReadData(
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::Action, 
                &vAction
                );

            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::FleetKey, 
                &vTemp
                );
            RETURN_ON_ERROR(iErrCode);
            iFleetKey = vTemp.GetInteger();
            
            if (vAction.GetInteger() >= 0) {
                vAction = BUILD_INTO_FLEET;
            }
            
            // If the ship is following a fleet action, get that fleet's action
            else if (vAction.GetInteger() == FLEET) {
                    
                Assert(iFleetKey != NO_KEY);
#ifdef _DEBUG

                Variant vTemp2;
                iErrCode = t_pCache->ReadData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::CurrentPlanet, 
                    &vTemp2
                    );
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->ReadData(
                    pstrEmpireFleets[i], 
                    iFleetKey, 
                    GameEmpireFleets::CurrentPlanet, 
                    &vTemp
                    );
                RETURN_ON_ERROR(iErrCode);
                
                Assert(vTemp2.GetInteger() == vTemp.GetInteger());
#endif
                iErrCode = t_pCache->ReadData(
                    pstrEmpireFleets[i], 
                    iFleetKey, 
                    GameEmpireFleets::Action, 
                    &vAction
                    );

                RETURN_ON_ERROR(iErrCode);

                // Special case for standby and ...
                if (IS_FLEET_STANDBY_ACTION (vAction.GetInteger())) {

                    if (vType.GetInteger() == FLEET_STANDBY_TECH_FROM_ORDER (vAction.GetInteger())) {

                        vAction = FLEET_ACTION_FOR_TECH [vType.GetInteger()];
                        Assert(vAction.GetInteger() != NO_KEY);

                        // Fix up terraformers
                        if (vType.GetInteger() == TERRAFORMER) {

                            if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {
                                vAction = TERRAFORM_AND_DISMANTLE;
                            }
                        }

                        // Fix up troopships
                        else if (vType.GetInteger() == TROOPSHIP) {

                            if (gcConfig.iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL) {
                                vAction = INVADE_AND_DISMANTLE;
                            }
                        }

                    } else {

                        vAction = STAND_BY;
                    }

                    iErrCode = t_pCache->WriteData(
                        pstrEmpireShips[i], 
                        piShipKey[j],
                        GameEmpireShips::Action,
                        vAction
                        );

                    RETURN_ON_ERROR(iErrCode);

                    continue;
                }
                
                // Special case for nuke
                else if (vAction.GetInteger() == NUKE) {

                    // Make sure not cloaked
                    iErrCode = t_pCache->ReadData(
                        pstrEmpireShips[i], 
                        piShipKey[j], 
                        GameEmpireShips::State, 
                        &vTemp
                        );

                    RETURN_ON_ERROR(iErrCode);

                    if (vTemp.GetInteger() & CLOAKED) {

                        // Cloaked, so we can't nuke
                        vAction = STAND_BY;

                    } else {

                        // Make sure not a illegal nuking sci
                        if (vType.GetInteger() == SCIENCE && (iGameClassOptions & DISABLE_SCIENCE_SHIPS_NUKING)) {

                            vAction = STAND_BY;

                        } else {
                    
                            iErrCode = t_pCache->WriteData(
                                pstrEmpireShips[i], 
                                piShipKey[j],
                                GameEmpireShips::Action,
                                NUKE
                                );

                            RETURN_ON_ERROR(iErrCode);

                            continue;
                        }
                    }
                }

            } // End if action is fleet
            
            // Handle action
            unsigned int iPlanetKey = NO_KEY;
            switch (vAction.GetInteger()) {
                
            case STAND_BY:
                
                bUpdated = true;
                break;
                
                // Handle "Dismantle"
            case DISMANTLE:
                
                // Add voluntarily dismantled
                piNumVoluntaryDismantled [vType.GetInteger()] ++;
                
                // Delete ship
                iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                RETURN_ON_ERROR(iErrCode);
                break;

                // Handle builds
            case BUILD_AT:
            case BUILD_INTO_FLEET:
                
                // Get planet key
                iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                iPlanetKey = vTemp.GetInteger();
                
                iErrCode = t_pCache->ReadData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Type, 
                    &vType
                    );
                RETURN_ON_ERROR(iErrCode);
                
#ifdef _DEBUG
                if (vAction.GetInteger() == BUILD_INTO_FLEET)
                {
                    Variant vMakeSure;
                    iErrCode = t_pCache->ReadData(
                        pstrEmpireFleets[i], 
                        iFleetKey, 
                        GameEmpireFleets::CurrentPlanet, 
                        &vMakeSure
                        );
                    Assert(iErrCode == OK && iPlanetKey == (unsigned int) vMakeSure.GetInteger());
                }
#endif
                iErrCode = t_pCache->WriteData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::BuiltThisUpdate, 
                    0
                    );
                RETURN_ON_ERROR(iErrCode);
                
                // Increase empire's resource use
                iMaintCost = GetMaintenanceCost (vType.GetInteger(), vMaxBR.GetFloat());
                iFuelCost = GetFuelCost (vType.GetInteger(), vMaxBR.GetFloat());

                iErrCode = t_pCache->Increment (
                    pstrEmpireData[i], 
                    GameEmpireData::TotalMaintenance,
                    iMaintCost
                    );
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->Increment (
                    pstrEmpireData[i], 
                    GameEmpireData::TotalFuelUse, 
                    iFuelCost
                    );
                RETURN_ON_ERROR(iErrCode);
#ifdef _DEBUG
                {
                    Variant vTotalFuelUse;
                    iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TotalFuelUse, &vTotalFuelUse);
                    Assert(iErrCode == OK && vTotalFuelUse.GetInteger() >= 0);
                }
#endif
                // Update empire's NumOwnShips and BuildShips
                iErrCode = t_pCache->GetFirstKey(
                    pstrEmpireMap[i], 
                    GameEmpireMap::PlanetKey, 
                    iPlanetKey,
                    &iKey
                    );
                RETURN_ON_ERROR(iErrCode);
                
                if (vType.GetInteger() == CLOAKER && (gcConfig.iShipBehavior & CLOAKER_CLOAK_ON_BUILD)) {

                    iErrCode = t_pCache->Increment (pstrEmpireMap[i], iKey, GameEmpireMap::NumCloakedBuildShips, -1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (pstrEmpireMap[i], iKey, GameEmpireMap::NumCloakedShips, 1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::NumCloakedBuildShips, -1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::NumCloakedShips, 1);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    iErrCode = t_pCache->Increment (pstrEmpireMap[i], iKey, GameEmpireMap::NumUncloakedBuildShips, -1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (pstrEmpireMap[i], iKey, GameEmpireMap::NumUncloakedShips, 1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::NumUncloakedBuildShips, -1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::NumUncloakedShips, 1);
                    RETURN_ON_ERROR(iErrCode);
                }
                
                // Add build to update message
                Assert(vType.GetInteger() >= 0 && vType.GetInteger() < NUM_SHIP_TYPES);
                piNumBuilds [vType.GetInteger()] ++;

                bUpdated = true;
                break;
                
                // Handle Move To's
                // Also check for first contact with an opponent's planet
            case MOVE_NORTH:
            case MOVE_EAST:
            case MOVE_SOUTH:
            case MOVE_WEST:
            case EXPLORE_NORTH:
            case EXPLORE_EAST:
            case EXPLORE_SOUTH:
            case EXPLORE_WEST:
                
                // Get planet key
                iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                iPlanetKey = vTemp.GetInteger();
                
                // Initialize the proxy key variable
                iDestProxyKey = NO_KEY;
                
                // Get the direction
                iTemp = (vAction.GetInteger() > EXPLORE_NORTH) ? MOVE_NORTH - vAction.GetInteger() : EXPLORE_NORTH - vAction.GetInteger();
                Assert(iTemp >= NORTH && iTemp <= WEST);
                
                // Get key of destination planet
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    iPlanetKey, 
                    GameMap::ColumnNames[GameMap::iNorthPlanetKey + iTemp],
                    &vDestPlanetKey
                    );
                RETURN_ON_ERROR(iErrCode);
                
                Assert(vDestPlanetKey != NO_KEY);
                vDestPlanetName = (const char*) NULL;
                
                // Get coordinates
                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
                RETURN_ON_ERROR(iErrCode);
                
                GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
                AdvanceCoordinates (iX, iY, &iNewX, &iNewY, iTemp);
                
                // Exploring?
                if (vAction.GetInteger() <= EXPLORE_NORTH) {
                    
                    // If planet hasn't been explored, add to empire's map, update all 
                    // ExploredX parameters and add to update message
                    iErrCode = t_pCache->GetFirstKey(
                        pstrEmpireMap[i], 
                        GameEmpireMap::PlanetKey, 
                        vDestPlanetKey.GetInteger(), 
                        &iDestProxyKey
                        );
                    
                    if (iErrCode != ERROR_DATA_NOT_FOUND)
                    {
                        RETURN_ON_ERROR(iErrCode);
                    }
                    else
                    {
                        Assert(iDestProxyKey == NO_KEY);
                        
                        ///////////////////////////
                        // An unexplored planet! //
                        ///////////////////////////
                        
                        // Add to map
                        pvColData[GameEmpireMap::iGameClass] = iGameClass;
                        pvColData[GameEmpireMap::iGameNumber] = iGameNumber;
                        pvColData[GameEmpireMap::iEmpireKey] = piEmpireKey[i];
                        pvColData[GameEmpireMap::iPlanetKey] = vDestPlanetKey.GetInteger();
                        pvColData[GameEmpireMap::iExplored] = 0;
                        pvColData[GameEmpireMap::iNumUncloakedShips] = 0;
                        pvColData[GameEmpireMap::iNumCloakedBuildShips] = 0;
                        pvColData[GameEmpireMap::iNumUncloakedBuildShips] = 0;
                        pvColData[GameEmpireMap::iNumCloakedShips] = 0;
                        
                        iErrCode = t_pCache->InsertRow(pstrEmpireMap[i], GameEmpireMap::Template, pvColData, &iDestProxyKey);
                        RETURN_ON_ERROR(iErrCode);
                        Assert(iDestProxyKey != NO_KEY);
                        
                        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::MinX, &vMinX);
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::MaxX, &vMaxX);
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::MinY, &vMinY);
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::MaxY, &vMaxY);
                        RETURN_ON_ERROR(iErrCode);
                        
                        Assert(
                            vMinX.GetInteger() > 0 && vMinY.GetInteger() > 0 && 
                            vMaxX.GetInteger() > 0 && vMaxY.GetInteger() > 0);
                        Assert(iNewX > 0 && iNewY > 0);
                        
                        if (iNewX > vMaxX.GetInteger()) {
                            iErrCode = t_pCache->WriteData(pstrEmpireData[i], 
                                GameEmpireData::MaxX, iNewX);
                            RETURN_ON_ERROR(iErrCode);
                        }
                        if (iNewX < vMinX.GetInteger()) {
                            iErrCode = t_pCache->WriteData(pstrEmpireData[i], 
                                GameEmpireData::MinX, iNewX);
                            RETURN_ON_ERROR(iErrCode);
                        }
                        if (iNewY > vMaxY.GetInteger()) {
                            iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::MaxY, iNewY);
                            RETURN_ON_ERROR(iErrCode);
                        }
                        if (iNewY < vMinY.GetInteger()) {
                            iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::MinY, iNewY);
                            RETURN_ON_ERROR(iErrCode);
                        }
                        
                        ////////////////////////////////////////////////////////
                        // Update all ExploredX parameters for the new planet //
                        ////////////////////////////////////////////////////////
                        
                        ENUMERATE_CARDINAL_POINTS(k) {
                            
                            // Get neighbour key
                            iErrCode = t_pCache->ReadData(
                                strGameMap,
                                vDestPlanetKey.GetInteger(), 
                                GameMap::ColumnNames[GameMap::iNorthPlanetKey + k],
                                &vNeighbourPlanetKey
                                );
                            RETURN_ON_ERROR(iErrCode);
                            
                            piExploredKey[k] = vNeighbourPlanetKey;
                            
                            if (piExploredKey[k] != NO_KEY) {
                                
                                // Has planet been explored already?
                                iErrCode = t_pCache->GetFirstKey(
                                    pstrEmpireMap[i], 
                                    GameEmpireMap::PlanetKey, 
                                    piExploredKey[k], 
                                    &iKey
                                    );
                                
                                if (iErrCode == ERROR_DATA_NOT_FOUND)
                                {
                                    iErrCode = OK;
                                }
                                else
                                {
                                    RETURN_ON_ERROR(iErrCode);
                                    
                                    // Set explored to true for old planet
                                    iErrCode = t_pCache->WriteOr(
                                        pstrEmpireMap[i], 
                                        iKey, 
                                        GameEmpireMap::Explored,
                                        OPPOSITE_EXPLORED_X[k]
                                        );
                                    RETURN_ON_ERROR(iErrCode);
                                    
                                    // Set explored to true for explored planet
                                    iErrCode = t_pCache->WriteOr(
                                        pstrEmpireMap[i], 
                                        iDestProxyKey,
                                        GameEmpireMap::Explored,
                                        EXPLORED_X[k]
                                        );
                                    RETURN_ON_ERROR(iErrCode);
                                }
                            }
                        }
                            
                        // Add exploration message to update message
                        iErrCode = t_pCache->ReadData(
                            pstrEmpireShips[i], 
                            piShipKey[j], 
                            GameEmpireShips::Name, 
                            &vShipName
                            );
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->ReadData(
                            strGameMap, 
                            vDestPlanetKey, 
                            GameMap::Name, 
                            &vDestPlanetName
                            );

                        RETURN_ON_ERROR(iErrCode);
                        
                        pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG " explored ";

                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i],
                            vDestPlanetName.GetCharPtr(),
                            iNewX,
                            iNewY
                            );

                        pstrUpdateMessage[i] += "\n";

                        // If mapshare, add to fellow sharer's maps
                        iErrCode = t_pCache->ReadData(
                            SYSTEM_GAMECLASS_DATA, 
                            iGameClass, 
                            SystemGameClassData::MapsShared, 
                            &vDipLevel
                            );
                        RETURN_ON_ERROR(iErrCode);
                        
                        if (vDipLevel != NO_DIPLOMACY)
                        {
                            Variant* pvAcquaintanceKey = NULL;
                            AutoFreeData free_pvAcquaintanceKey(pvAcquaintanceKey);
                            
                            unsigned int* piProxyKey = NULL;
                            AutoFreeKeys free_piProxyKey(piProxyKey);

                            iErrCode = t_pCache->ReadColumn(
                                pstrEmpireDip[i], 
                                GameEmpireDiplomacy::ReferenceEmpireKey, 
                                &piProxyKey, 
                                &pvAcquaintanceKey, 
                                (unsigned int*) &iNumAcquaintances
                                );

                            if (iErrCode == ERROR_DATA_NOT_FOUND)
                            {
                                iErrCode = OK;
                            }
                            else
                            {
                                RETURN_ON_ERROR(iErrCode);
                                
                                for (l = 0; l < iNumAcquaintances; l ++) {
                                    
                                    iErrCode = t_pCache->ReadData(
                                        pstrEmpireDip[i], 
                                        piProxyKey[l], 
                                        GameEmpireDiplomacy::CurrentStatus, 
                                        &vDipStatus
                                        );
                                    RETURN_ON_ERROR(iErrCode);
                                    
                                    if (vDipStatus.GetInteger() >= vDipLevel.GetInteger()) {
                                        
                                        GetEmpireIndex (m, pvAcquaintanceKey[l]);
                                        
                                        iErrCode = SharePlanetBetweenFriends (
                                            iGameClass,
                                            iGameNumber,
                                            vDestPlanetKey.GetInteger(),
                                            m,
                                            pstrEmpireMap,
                                            pstrEmpireDip,
                                            pstrEmpireData,
                                            strGameMap,
                                            iNumEmpires,
                                            piEmpireKey, 
                                            vDipLevel.GetInteger(),
                                            NULL,
                                            NULL,
                                            -1,
                                            true
                                            );
                                        
                                        RETURN_ON_ERROR(iErrCode);
                                    }
                                }   // End acquaintance loop

                            }   // End if acquaintances > 0

                        }   // End if mapshare

                    }   // End if exploring a new planet
                    
                }   // End if _explore_ and not _move_
                
                // Update action to standby and location to new planet
                iErrCode = t_pCache->WriteData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::CurrentPlanet, 
                    vDestPlanetKey
                    );

                RETURN_ON_ERROR(iErrCode);
                
                // Get proxy key in empire map for new planet
                if (iDestProxyKey == NO_KEY)
                {
                    iErrCode = t_pCache->GetFirstKey(
                        pstrEmpireMap[i], 
                        GameEmpireMap::PlanetKey, 
                        vDestPlanetKey, 
                        (unsigned int*) &iDestProxyKey
                        );
                    RETURN_ON_ERROR(iErrCode);
                }
                
                // Get proxy key in empire map for old planet
                iErrCode = t_pCache->GetFirstKey(
                    pstrEmpireMap[i], 
                    GameEmpireMap::PlanetKey, 
                    iPlanetKey, 
                    &iProxyKey
                    );
                RETURN_ON_ERROR(iErrCode);
                
                // Update ship count parameters
                iErrCode = t_pCache->ReadData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::State, 
                    &vTemp
                    );

                RETURN_ON_ERROR(iErrCode);
                
                if (vTemp.GetInteger() & CLOAKED) {
                    iErrCode = t_pCache->Increment (pstrEmpireMap[i], iProxyKey, GameEmpireMap::NumCloakedShips, -1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (pstrEmpireMap[i], iDestProxyKey, GameEmpireMap::NumCloakedShips, 1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::NumCloakedShips, -1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (strGameMap, vDestPlanetKey, GameMap::NumCloakedShips, 1);
                    RETURN_ON_ERROR(iErrCode);
                
                } else {

                    iErrCode = t_pCache->Increment (pstrEmpireMap[i], iProxyKey, GameEmpireMap::NumUncloakedShips, -1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (pstrEmpireMap[i], iDestProxyKey, GameEmpireMap::NumUncloakedShips, 1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::NumUncloakedShips, -1);
                    RETURN_ON_ERROR(iErrCode);
                    iErrCode = t_pCache->Increment (strGameMap, vDestPlanetKey, GameMap::NumUncloakedShips, 1);
                    RETURN_ON_ERROR(iErrCode);
#ifdef _DEBUG
                    Variant vFooBar;
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::NumUncloakedShips, &vFooBar);
                    RETURN_ON_ERROR(iErrCode);
                    Assert(vFooBar.GetInteger() >= 0);
#endif
                    if (String::IsBlank (vDestPlanetName.GetCharPtr())) {

                        iErrCode = t_pCache->ReadData(
                            strGameMap,
                            vDestPlanetKey.GetInteger(),
                            GameMap::Name,
                            &vDestPlanetName
                            );

                        RETURN_ON_ERROR(iErrCode);
                    }

                    iErrCode = CheckForFirstContact(
                        iGameClass, iGameNumber, piEmpireKey[i], i, 
                        vDestPlanetKey.GetInteger(), 
                        vDestPlanetName.GetCharPtr(),
                        iNewX, 
                        iNewY,
                        iNumEmpires,
                        piEmpireKey,
                        pvEmpireName,
                        pstrEmpireDip[i],
                        strGameMap,
                        pstrEmpireDip,
                        pstrUpdateMessage
                        );

                    RETURN_ON_ERROR(iErrCode);
                }
                
                bUpdated = true;
                break;
                
            case CLOAK:
                
                // Get planet key
                iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                iPlanetKey = vTemp.GetInteger();
                
                iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Name, &vShipName);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
                RETURN_ON_ERROR(iErrCode);

                GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
                
                // Set ship as cloaked
                iErrCode = ChangeShipCloakingState (
                    piShipKey[j],
                    iPlanetKey,
                    true,
                    pstrEmpireShips[i], 
                    pstrEmpireMap[i], 
                    strGameMap
                    );

                RETURN_ON_ERROR(iErrCode);

                // Add to update message
                pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                pstrUpdateMessage[i] += " cloaked at ";
                
                AddPlanetNameAndCoordinates (
                    pstrUpdateMessage[i],
                    vPlanetName.GetCharPtr(), 
                    iX, 
                    iY
                    );
                
                pstrUpdateMessage[i] += "\n";
                
                bUpdated = true;
                break;
                
            case UNCLOAK:
                
                // Get planet key
                iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                iPlanetKey = vTemp.GetInteger();
                
                iErrCode = t_pCache->ReadData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Name, 
                    &vShipName
                    );
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
                RETURN_ON_ERROR(iErrCode);
                
                GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
                
                // Set ship as uncloaked
                iErrCode = ChangeShipCloakingState (
                    piShipKey[j],
                    iPlanetKey,
                    false,
                    pstrEmpireShips[i], 
                    pstrEmpireMap[i], 
                    strGameMap
                    );

                RETURN_ON_ERROR(iErrCode);
                
                // Add to update message
                pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                pstrUpdateMessage[i] += " uncloaked at ";

                AddPlanetNameAndCoordinates (
                    pstrUpdateMessage[i],
                    vPlanetName.GetCharPtr(), 
                    iX, 
                    iY
                    );
                
                pstrUpdateMessage[i] += "\n";
                
                iErrCode = CheckForFirstContact(
                    iGameClass, iGameNumber, piEmpireKey[i], i, 
                    iPlanetKey, 
                    vPlanetName.GetCharPtr(),
                    iX, 
                    iY,
                    iNumEmpires,
                    piEmpireKey,
                    pvEmpireName,
                    pstrEmpireDip[i],
                    strGameMap,
                    pstrEmpireDip,
                    pstrUpdateMessage
                    );

                RETURN_ON_ERROR(iErrCode);
                
                bUpdated = true;
                break;

            default:
                
                // Special case for morphers
                if (IS_MORPH_ACTION (vAction.GetInteger())) {

                    bool bDied = false;

                    Variant vTechs, vBR, vShipType, vShipState;
                    
                    iTemp = MORPH_BASETECH - vAction.GetInteger();
                    Assert(iTemp >= FIRST_SHIP && iTemp <= LAST_SHIP);
                    
                    // Get planet key
                    iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                    RETURN_ON_ERROR(iErrCode);
                    iPlanetKey = vTemp.GetInteger();
                    
                    // Get developed techs
                    iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TechDevs, &vTechs);
                    RETURN_ON_ERROR(iErrCode);
                    
                    // Get BR
                    iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentBR, &vBR);
                    RETURN_ON_ERROR(iErrCode);

                    // Get current type
                    iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vShipType);
                    RETURN_ON_ERROR(iErrCode);

                    // Get ship name
                    iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Name, &vShipName);
                    RETURN_ON_ERROR(iErrCode);

                    // Get ship state
                    iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::State, &vShipState);
                    RETURN_ON_ERROR(iErrCode);
                    
                    // Get planet name
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
                    RETURN_ON_ERROR(iErrCode);
                    
                    // Get coordinates
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
                    RETURN_ON_ERROR(iErrCode);
                    
                    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
                    
                    if (vTechs.GetInteger() & TECH_BITS[iTemp] && !(vShipState.GetInteger() & CLOAKED)) {

                        // Make sure ship will survive
                        if (vBR.GetFloat() < gcConfig.fMorpherCost) {
                            
                            // Tell owner that the ship couldn't morph
                            pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                            pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                            pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                            pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                            pstrUpdateMessage[i] += END_STRONG " could not morph at ";
                            
                            AddPlanetNameAndCoordinates (
                                pstrUpdateMessage[i],
                                vPlanetName.GetCharPtr(), 
                                iX, 
                                iY
                                );
                            
                            pstrUpdateMessage[i] += "\n" END_FONT;
                        }
                        
                        else if (vBR.GetFloat() - gcConfig.fMorpherCost <= FLOAT_PROXIMITY_TOLERANCE) {
                            
                            // Just destroy the ship
                            iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                            RETURN_ON_ERROR(iErrCode);
                            
                            bDied = true;
                            
                            // Tell owner that the ship was destroyed morphing
                            pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                            pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                            pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                            pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                            pstrUpdateMessage[i] += END_STRONG " destroyed itself morphing at ";
                            
                            AddPlanetNameAndCoordinates (
                                pstrUpdateMessage[i],
                                vPlanetName.GetCharPtr(), 
                                iX, 
                                iY
                                );
                            
                            pstrUpdateMessage[i] += "\n" END_FONT;
                            
                        } else {
                            
                            // Recalculate resource usage
                            iErrCode = ChangeShipTypeOrMaxBR (
                                pstrEmpireShips[i],
                                pstrEmpireData[i],
                                piEmpireKey[i],
                                piShipKey[j],
                                vShipType.GetInteger(),
                                iTemp,
                                - gcConfig.fMorpherCost
                                );
                            
                            RETURN_ON_ERROR(iErrCode);
                            
                            // Handle cloaked state
                            if (iTemp == CLOAKER) {

                                Assert(!(vShipState.GetInteger() & CLOAKED));
                                
                                // Set ship as cloaked
                                iErrCode = ChangeShipCloakingState (
                                    piShipKey[j],
                                    iPlanetKey,
                                    true,
                                    pstrEmpireShips[i], 
                                    pstrEmpireMap[i], 
                                    strGameMap
                                    );
                                RETURN_ON_ERROR(iErrCode);
                            } 
                            
                            else if (vShipState.GetInteger() & CLOAKED) {
                                
                                // Set ship as uncloaked
                                iErrCode = ChangeShipCloakingState (
                                    piShipKey[j],
                                    iPlanetKey,
                                    false,
                                    pstrEmpireShips[i], 
                                    pstrEmpireMap[i], 
                                    strGameMap
                                    );
                                RETURN_ON_ERROR(iErrCode);
                            }
                            
                            // Tell owner about success
                            pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                            pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                            pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                            pstrUpdateMessage[i] += END_STRONG " morphed into ";
                            pstrUpdateMessage[i] += SHIP_TYPE_STRING [iTemp];
                            pstrUpdateMessage[i] += " form at ";
                            
                            AddPlanetNameAndCoordinates (
                                pstrUpdateMessage[i],
                                vPlanetName.GetCharPtr(), 
                                iX, 
                                iY
                                );
                            
                            pstrUpdateMessage[i] += "\n";
                        }
                    }
                
                    if (!bDied) {
                        
                        // Set to standby
                        iErrCode = t_pCache->WriteData(
                            pstrEmpireShips[i], 
                            piShipKey[j], 
                            GameEmpireShips::Action, 
                            STAND_BY
                            );
                        
                        RETURN_ON_ERROR(iErrCode);
                    }

                }   // End if morphing
                
                break;

            }   // End ship action switch                   
            
            // If the ship's action was processed, set its new action to standby.
            if (bUpdated) {
                
                iErrCode = t_pCache->WriteData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Action, 
                    STAND_BY
                    );

                RETURN_ON_ERROR(iErrCode);
            }
        }   // End ships loop

        // Ships built
        ENUMERATE_TECHS(j) {
        
            if (piNumBuilds[j] > 0) {
                
                if (piNumBuilds[j] == 1) {
                    
                    pstrUpdateMessage[i] += "You built " BEGIN_STRONG "1" END_STRONG " ";
                    pstrUpdateMessage[i] += SHIP_TYPE_STRING_LOWERCASE [j];
                    pstrUpdateMessage[i] += "\n";

                } else {
                    
                    pstrUpdateMessage[i] += "You built " BEGIN_STRONG;
                    pstrUpdateMessage[i] += piNumBuilds[j];
                    pstrUpdateMessage[i] += END_STRONG " ";
                    pstrUpdateMessage[i] += SHIP_TYPE_STRING_LOWERCASE_PLURAL [j];
                    pstrUpdateMessage[i] += "\n";
                }
            }
        }
        
        // Ships voluntarily dismantled
        ENUMERATE_TECHS(j) {
        
            if (piNumVoluntaryDismantled[j] > 0) {
                if (piNumVoluntaryDismantled[j] == 1) {
                    pstrUpdateMessage[i] += "You dismantled " BEGIN_STRONG "1" END_STRONG " ";
                    pstrUpdateMessage[i] += SHIP_TYPE_STRING_LOWERCASE [j];
                    pstrUpdateMessage[i] += "\n";
                } else {
                    pstrUpdateMessage[i] += "You dismantled " BEGIN_STRONG;
                    pstrUpdateMessage[i] += piNumVoluntaryDismantled[j];
                    pstrUpdateMessage[i] += END_STRONG " ";
                    pstrUpdateMessage[i] += SHIP_TYPE_STRING_LOWERCASE_PLURAL [j];
                    pstrUpdateMessage[i] += "\n";
                }
            }
        }

        // Ships forcibly dismantled
        ENUMERATE_TECHS(j) {
        
            if (piNumForceDismantled[j] > 0) {
                if (piNumForceDismantled[j] == 1) {
                    pstrUpdateMessage[i] += BEGIN_STRONG "1" END_STRONG " ";
                    pstrUpdateMessage[i] += SHIP_TYPE_STRING_LOWERCASE [j];
                    pstrUpdateMessage[i] += " was dismantled because it was beyond repair\n";
                } else {
                    pstrUpdateMessage[i] += BEGIN_STRONG;
                    pstrUpdateMessage[i] += piNumForceDismantled[j];
                    pstrUpdateMessage[i] += END_STRONG " ";
                    pstrUpdateMessage[i] += SHIP_TYPE_STRING_LOWERCASE_PLURAL [j];
                    pstrUpdateMessage[i] += " were dismantled because they were beyond repair\n";
                }
            }
        }
    }   // End empires loop

    return iErrCode;
}

int GameEngine::MakeShipsFight (int iGameClass, int iGameNumber, const char* strGameMap, int iNumEmpires, 
                                unsigned int* piEmpireKey, bool* pbAlive, Variant* pvEmpireName, const char** pstrEmpireShips, 
                                const char** pstrEmpireDip, const char** pstrEmpireMap, String* pstrUpdateMessage, 
                                const char** pstrEmpireData, float* pfFuelRatio, unsigned int iPlanetKey, 
                                int* piTotalMin, int* piTotalFuel,
                                bool bIndependence, const char* strIndependentShips,
                                Variant* pvGoodColor, Variant* pvBadColor, 
                                const GameConfiguration& gcConfig) {

    int i, j, k, l, iErrCode = OK;
    
    unsigned int iPlanetProxyKey, iNumShips, iCounter;

    Variant vNumShips, vNumCloaked, pvColData[GameEmpireDiplomacy::NumColumns], vPlanetName, vTemp, vPop, vMin, vFuel, vCoord, vOwner, vMineName;
    String strList;
    
    char pszPlanetName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH + 1];

    float fDmgRatio, fDest, fDV, fDamageRatio, fFactor;

    unsigned int iNumAdjustedEmpires = iNumEmpires + 1;

    bool* pbBattleEmpire = (bool*) StackAlloc (2 * iNumAdjustedEmpires * sizeof (bool));
    bool* pbHasShipsEmpire = pbBattleEmpire + iNumAdjustedEmpires;

    int* piIntBlock = (int*) StackAlloc (5 * iNumAdjustedEmpires * sizeof (int));

    int* piBattleEmpireIndex = piIntBlock;
    int* piNumBattleShips = piBattleEmpireIndex + iNumAdjustedEmpires;
    int* piNumCloaked = piNumBattleShips + iNumAdjustedEmpires;
    int* piWatcherIndex = piNumCloaked + iNumAdjustedEmpires;
    int* piBattleShipsDestroyed = piWatcherIndex + iNumAdjustedEmpires;

    void** ppvPtrBlock = (void**) StackAlloc (9 * iNumAdjustedEmpires * sizeof (void*));
    memset(ppvPtrBlock, 0, iNumAdjustedEmpires * sizeof (void*));

    int** ppiBattleShipKey = (int**) ppvPtrBlock;
    int** ppiBattleShipType = (int**) ppiBattleShipKey + iNumAdjustedEmpires;
    float** ppfBattleShipBP = (float**) ppiBattleShipType + iNumAdjustedEmpires;
    float** ppfRealBR = (float**) ppfBattleShipBP + iNumAdjustedEmpires;
    int** ppiCloakerKey = (int**)  ppfRealBR + iNumAdjustedEmpires;
    bool** ppbAlive = (bool**) ppiCloakerKey + iNumAdjustedEmpires;
    bool** ppbSweep = (bool**) ppbAlive + iNumAdjustedEmpires;
    int** ppiDipRef = (int**)  ppbSweep + iNumAdjustedEmpires;
    const char** ppszEmpireShips = (const char**) ppiDipRef + iNumAdjustedEmpires;

    // Assign 2D dipref arrays
    int* piBlob = (int*) StackAlloc (iNumAdjustedEmpires * iNumAdjustedEmpires * sizeof (int));
    memset (piBlob, 0, iNumAdjustedEmpires * iNumAdjustedEmpires * sizeof (int));

    for (i = 0; i <= iNumEmpires; i ++) {
        ppiDipRef[i] = piBlob + i * (iNumAdjustedEmpires);
    }

    float* pfBP = (float*) StackAlloc ((iNumAdjustedEmpires) * 3 * sizeof (float));
    float* pfEnemyBP = pfBP + iNumAdjustedEmpires;
    float* pfTotDmg = pfEnemyBP + iNumAdjustedEmpires;

    ICachedTable* pShipsTable = NULL;
    AutoRelease<ICachedTable> release_pShipsTable(pShipsTable);
    
    ICachedTable* pShipsTableW = NULL;
    AutoRelease<ICachedTable> release_pShipsTableW(pShipsTableW);

    unsigned int* piShipKey = NULL,  * piIndependentShipKey = NULL;
    AutoFreeKeys free_piShipKey(piShipKey);
    AutoFreeKeys free_piIndependentShipKey(piIndependentShipKey);

    // First, find out if a battle can take place at this planet
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::NumUncloakedShips, &vNumShips);
    RETURN_ON_ERROR(iErrCode);

    if (vNumShips.GetInteger() == 0)
    {
        return OK;
    }

    unsigned int iNumBattleEmpires = 0;  // Zero empires fighting
    int iNumWatchers = 0;       // Zero empires watching
    int iNumHasShipsEmpires = 0;
    unsigned int iNumIndependentShips = 0;
    unsigned int iMineIndex = NO_KEY, iTempMineIndex;
    unsigned int iMineOwner = NO_KEY;
    bool bMines = false, bDetonated = false;

    AutoFreeArrayOfArrays<int> free_ppiBattleShipKey(ppiBattleShipKey, iNumBattleEmpires);
    AutoFreeArrayOfArrays<float> free_ppfBattleShipBP(ppfBattleShipBP, iNumBattleEmpires);
    AutoFreeArrayOfArrays<bool> free_ppbAlive(ppbAlive, iNumBattleEmpires);

    // Check the owner
    bool bIndependentOwner = bIndependence;
    if (bIndependence)
    {
        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
        RETURN_ON_ERROR(iErrCode);
        bIndependentOwner = vOwner.GetInteger() == INDEPENDENT;
    }

    // Get the planet name
    iErrCode = GetPlanetNameWithCoordinates (strGameMap, iPlanetKey, pszPlanetName);
    RETURN_ON_ERROR(iErrCode);

    // There are ships here:  loop through all empires
    for (j = 0; j < iNumEmpires; j ++) {

        pbBattleEmpire[j] = false;
        pbHasShipsEmpire[j] = false;

        if (!pbAlive[j]) {
            continue;
        }

        iErrCode = t_pCache->GetFirstKey(
            pstrEmpireMap[j], 
            GameEmpireMap::PlanetKey, 
            iPlanetKey, 
            &iPlanetProxyKey
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            continue;
        }
        RETURN_ON_ERROR(iErrCode);

        // Add to watchers list
        piWatcherIndex[iNumWatchers] = j;
        iNumWatchers ++;

        // Does empire have visible ships at the planet?
        iErrCode = t_pCache->ReadData(
            pstrEmpireMap[j], 
            iPlanetProxyKey, 
            GameEmpireMap::NumUncloakedShips, 
            &vNumShips
            );

        RETURN_ON_ERROR(iErrCode);

        if (vNumShips.GetInteger() == 0) {
            continue;
        }

        pbHasShipsEmpire[j] = true;
        iNumHasShipsEmpires ++;

        // Scan through empires already found at this planet
        for (k = 0; k < j; k ++)
        {
            // Only care if they have ships
            if (!pbHasShipsEmpire[k]) {
                continue;
            }

            int iStatus;
            bool bMet;

            // Check diplomatic status
            iErrCode = GetVisibleDiplomaticStatus(
                iGameClass,
                iGameNumber,
                piEmpireKey[j],
                piEmpireKey[k],
                NULL, NULL, &iStatus, &bMet
                );

            RETURN_ON_ERROR(iErrCode);

            // Save the diplomatic status between the empires
            ppiDipRef[k][j] = ppiDipRef[j][k] = iStatus;

            // If it's war, they will fight
            if (iStatus == WAR) {

                // Add empires to list of battling empires

                if (!pbBattleEmpire[j]) {

                    // j
                    pbBattleEmpire[j] = true;
                    piBattleEmpireIndex [iNumBattleEmpires] = j;

                    piNumBattleShips[iNumBattleEmpires] = 0;
                    piNumCloaked[iNumBattleEmpires] = 0;
                    piBattleShipsDestroyed[iNumBattleEmpires] = 0;

                    ppiBattleShipKey[iNumBattleEmpires] = NULL;
                    ppiBattleShipType[iNumBattleEmpires] = NULL;
                    ppfBattleShipBP[iNumBattleEmpires] = NULL;
                    ppbAlive[iNumBattleEmpires] = NULL;
                    ppbSweep[iNumBattleEmpires] = NULL;
                    ppiCloakerKey[iNumBattleEmpires] = NULL;

                    iNumBattleEmpires ++;
                }

                if (!pbBattleEmpire[k]) {

                    // k
                    pbBattleEmpire[k] = true;
                    piBattleEmpireIndex [iNumBattleEmpires] = k;

                    piNumBattleShips[iNumBattleEmpires] = 0;
                    piNumCloaked[iNumBattleEmpires] = 0;
                    piBattleShipsDestroyed[iNumBattleEmpires] = 0;

                    ppiBattleShipKey[iNumBattleEmpires] = NULL;
                    ppiBattleShipType[iNumBattleEmpires] = NULL;
                    ppfBattleShipBP[iNumBattleEmpires] = NULL;
                    ppbAlive[iNumBattleEmpires] = NULL;
                    ppbSweep[iNumBattleEmpires] = NULL;
                    ppiCloakerKey[iNumBattleEmpires] = NULL;

                    iNumBattleEmpires ++;
                }

                if (!bMet) {

                    //////////////////////////////////
                    // First contact (ship to ship) //
                    //////////////////////////////////
                    // TODO - merge this code with the other first contact code

                    // Add second empire to first's dip screen
                    pvColData[GameEmpireDiplomacy::iGameClass] = iGameClass;
                    pvColData[GameEmpireDiplomacy::iGameNumber] = iGameNumber;
                    pvColData[GameEmpireDiplomacy::iEmpireKey] = piEmpireKey[j];
                    pvColData[GameEmpireDiplomacy::iReferenceEmpireKey] = piEmpireKey[k];
                    pvColData[GameEmpireDiplomacy::iDipOffer] = WAR;
                    pvColData[GameEmpireDiplomacy::iCurrentStatus] = WAR;
                    pvColData[GameEmpireDiplomacy::iDipOfferLastUpdate] = WAR;
                    pvColData[GameEmpireDiplomacy::iState] = 0;
                    pvColData[GameEmpireDiplomacy::iSubjectiveEcon] = 0;
                    pvColData[GameEmpireDiplomacy::iSubjectiveMil] = 0;
                    pvColData[GameEmpireDiplomacy::iLastMessageTargetFlag] = 0;

                    iErrCode = t_pCache->InsertRow(pstrEmpireDip[j], GameEmpireDiplomacy::Template, pvColData, NULL);
                    RETURN_ON_ERROR(iErrCode);

                    // Add first empire to second's dip screen
                    pvColData[GameEmpireDiplomacy::iEmpireKey] = piEmpireKey[k]; // EmpireKey
                    pvColData[GameEmpireDiplomacy::iReferenceEmpireKey] = piEmpireKey[j]; // iReferenceEmpireKey

                    iErrCode = t_pCache->InsertRow(pstrEmpireDip[k], GameEmpireDiplomacy::Template, pvColData, NULL);
                    RETURN_ON_ERROR(iErrCode);

                    // Add to first empire's update message
                    pstrUpdateMessage[j] += "You have had " BEGIN_STRONG " first contact " END_STRONG " with " BEGIN_STRONG;
                    pstrUpdateMessage[j] += pvEmpireName[k].GetCharPtr();
                    pstrUpdateMessage[j] += END_STRONG " (ship to ship) at ";
                    pstrUpdateMessage[j].AppendHtml (pszPlanetName, 0, false);
                    pstrUpdateMessage[j] += "\n";

                    // Add to second empire's update message
                    pstrUpdateMessage[k] += "You have had " BEGIN_STRONG " first contact " END_STRONG " with " BEGIN_STRONG;
                    pstrUpdateMessage[k] += pvEmpireName[j].GetCharPtr();
                    pstrUpdateMessage[k] += END_STRONG " (ship to ship) at ";
                    pstrUpdateMessage[k].AppendHtml (pszPlanetName, 0, false);
                    pstrUpdateMessage[k] += "\n";

                    // End first contact
                }
            }
        }   // End previous empires scanned loop
    }   // End empire loop

    // Independent ships fight everyone
    if (bIndependentOwner) {

        iErrCode = t_pCache->GetEqualKeys (
            strIndependentShips,
            GameEmpireShips::CurrentPlanet,
            iPlanetKey,
            &piIndependentShipKey,
            &iNumIndependentShips
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);

            // Add everyone else who had a ship and isn't on the list already
            for (j = 0; j < iNumEmpires; j ++)
            {
                if (pbAlive[j] && pbHasShipsEmpire[j] && !pbBattleEmpire[j])
                {
                    pbBattleEmpire[j] = true;
                    piBattleEmpireIndex [iNumBattleEmpires] = j;

                    ppiBattleShipKey[iNumBattleEmpires] = NULL;
                    ppiBattleShipType[iNumBattleEmpires] = NULL;
                    ppfBattleShipBP[iNumBattleEmpires] = NULL;
                    ppbAlive[iNumBattleEmpires] = NULL;
                    ppbSweep[iNumBattleEmpires] = NULL;
                    ppiCloakerKey[iNumBattleEmpires] = NULL;

                    piNumBattleShips[iNumBattleEmpires] = 0;
                    piNumCloaked[iNumBattleEmpires] = 0;
                    piBattleShipsDestroyed[iNumBattleEmpires] = 0;

                    iNumBattleEmpires ++;
                }
            }

            // Add the Independent empire too
            piBattleEmpireIndex [iNumBattleEmpires] = INDEPENDENT;

            ppiBattleShipKey[iNumBattleEmpires] = NULL;
            ppiBattleShipType[iNumBattleEmpires] = NULL;
            ppfBattleShipBP[iNumBattleEmpires] = NULL;
            ppbAlive[iNumBattleEmpires] = NULL;
            ppbSweep[iNumBattleEmpires] = NULL;
            ppiCloakerKey[iNumBattleEmpires] = NULL;

            piNumBattleShips[iNumBattleEmpires] = 0;
            piNumCloaked[iNumBattleEmpires] = 0;
            piBattleShipsDestroyed[iNumBattleEmpires] = 0;

            iNumBattleEmpires ++;
        }
    }

    //////////////////////
    // Fight the battle //
    //////////////////////

    if (iNumBattleEmpires > 0) {

#ifdef _DEBUG

        // Make sure there are no index duplicates
        for (j = 0; j < (int)iNumBattleEmpires; j ++)
        {
            Assert(
                piBattleEmpireIndex[j] == INDEPENDENT ||
                (piBattleEmpireIndex[j] >= 0 && 
                piBattleEmpireIndex[j] < iNumEmpires)
                );

            for (k = j + 1; k < (int)iNumBattleEmpires; k ++)
            {
                Assert(piBattleEmpireIndex[j] != piBattleEmpireIndex[k]);
            }
        }
#endif
        for (j = 0; j < (int)iNumBattleEmpires; j ++) {

            const char* cStateColumn, * pszTypeColumn, * cCurrentBRColumn;

            if (piBattleEmpireIndex[j] == INDEPENDENT) {

                cStateColumn = GameEmpireShips::State;
                pszTypeColumn = GameEmpireShips::Type;
                cCurrentBRColumn = GameEmpireShips::CurrentBR;

                ppszEmpireShips[j] = strIndependentShips;
                piShipKey = piIndependentShipKey;
                piIndependentShipKey = NULL;
                iNumShips = iNumIndependentShips;

            } else {

                cStateColumn = GameEmpireShips::State;
                pszTypeColumn = GameEmpireShips::Type;
                cCurrentBRColumn = GameEmpireShips::CurrentBR;

                ppszEmpireShips[j] = pstrEmpireShips[piBattleEmpireIndex[j]];

                // Make a list of all empire's ships at planet
                if (piShipKey)
                {
                    t_pCache->FreeKeys(piShipKey);
                    piShipKey = NULL;
                }

                iErrCode = t_pCache->GetEqualKeys (
                    ppszEmpireShips[j], 
                    GameEmpireShips::CurrentPlanet, 
                    iPlanetKey, 
                    &piShipKey, 
                    &iNumShips
                    );

                RETURN_ON_ERROR(iErrCode);
            }

            Assert(piShipKey != NULL && iNumShips > 0);

            // Randomize ship array
            Algorithm::Randomize (piShipKey, iNumShips);

            // Set up tables
            pfBP[j] = (float) 0.0;
            pfEnemyBP[j] = (float) 0.0;
            pfTotDmg[j] = (float) 0.0;

            ppiBattleShipKey[j] = new int [iNumShips * 3];
            Assert(ppiBattleShipKey[j]);
            ppiBattleShipType[j] = ppiBattleShipKey[j] + iNumShips;
            ppiCloakerKey[j] = ppiBattleShipType[j] + iNumShips;

            ppfBattleShipBP[j] = new float [iNumShips * 2];
            Assert(ppfBattleShipBP[j]);
            ppfRealBR[j] = ppfBattleShipBP[j] + iNumShips;

            ppbAlive[j] = new bool [iNumShips * 2];
            Assert(ppbAlive[j]);
            ppbSweep[j] = ppbAlive[j] + iNumShips;

            iErrCode = t_pCache->GetTable(ppszEmpireShips[j], &pShipsTable);
            RETURN_ON_ERROR(iErrCode);

            // Collect keys while filtering cloaked ships
            for (l = 0; l < (int) iNumShips; l ++) {

                int iState;

                iErrCode = pShipsTable->ReadData (piShipKey[l], cStateColumn, &iState);
                RETURN_ON_ERROR(iErrCode);

                if (!(iState & CLOAKED)) {

                    // Add ship to tables
                    ppiBattleShipKey[j][piNumBattleShips[j]] = piShipKey[l];
                    ppbAlive[j][piNumBattleShips[j]] = true;
                    ppbSweep[j][piNumBattleShips[j]] = false;

                    int iType;
                    iErrCode = pShipsTable->ReadData (piShipKey[l], pszTypeColumn, &iType);
                    RETURN_ON_ERROR(iErrCode);
                    ppiBattleShipType[j][piNumBattleShips[j]] = iType;

                    // Add ship's BR ^ 2 to BP
                    float fBR;
                    iErrCode = pShipsTable->ReadData (piShipKey[l], cCurrentBRColumn, &fBR);
                    RETURN_ON_ERROR(iErrCode);
                    ppfRealBR[j][piNumBattleShips[j]] = fBR;

                    fFactor = fBR * fBR;
                    ppfBattleShipBP[j][piNumBattleShips[j]] = fFactor;
                    pfBP[j] += fFactor;
                    piNumBattleShips[j] ++;

                } else {

                    Assert(piBattleEmpireIndex[j] != INDEPENDENT);

                    // Add cloaked ships to a list (useful if a mine goes off)
                    ppiCloakerKey[j][piNumCloaked[j]] = piShipKey[l];
                    piNumCloaked[j] ++;
                }

            }   // End for all ships

            //////////////////////
            // Apply fuel ratio //
            //////////////////////

            if (piBattleEmpireIndex[j] != INDEPENDENT && pfFuelRatio[piBattleEmpireIndex[j]] < (float) 1.0)
            {
                // Apply to BP
                pfBP[j] *= pfFuelRatio[piBattleEmpireIndex[j]];

                // Apply to individual ships
                for (l = 0; l < piNumBattleShips[j]; l ++) {
                    ppfBattleShipBP[j][l] *= pfFuelRatio[piBattleEmpireIndex[j]];
                }
            }

        } // End empire BP collection loop  


        // Calculate enemy_bp_tot for each empire
        for (j = 0; j < (int)iNumBattleEmpires; j ++)
        {
            for (k = j + 1; k < (int)iNumBattleEmpires; k ++)
            {
                if (piBattleEmpireIndex[j] == INDEPENDENT || 
                    piBattleEmpireIndex[k] == INDEPENDENT || 
                    ppiDipRef[piBattleEmpireIndex[j]][piBattleEmpireIndex[k]] == WAR)
                {
                    pfEnemyBP[j] += pfBP[k];
                    pfEnemyBP[k] += pfBP[j];
                }
            }
        }

        // Calculate tot_dmg for each empire
        // This can only be done after all the enemy_bp_tot's are known, 
        // so that's why we have redundant loops
        for (j = 0; j < (int)iNumBattleEmpires; j ++)
        {
            for (k = j + 1; k < (int)iNumBattleEmpires; k ++)
            {
                if (piBattleEmpireIndex[j] == INDEPENDENT || 
                    piBattleEmpireIndex[k] == INDEPENDENT || 
                    ppiDipRef[piBattleEmpireIndex[j]][piBattleEmpireIndex[k]] == WAR)
                {
                    fDmgRatio = pfBP[j] / pfEnemyBP[k];
                    pfTotDmg[j] += fDmgRatio * pfBP[k];

                    fDmgRatio = pfBP[k] / pfEnemyBP[j];
                    pfTotDmg[k] += fDmgRatio * pfBP[j];
                }
            }
        }

        ////////////////////
        // Resolve combat //
        ////////////////////

        for (j = 0; j < (int)iNumBattleEmpires; j ++) {

            const char* cActionColumn, * pszTypeColumn, * cCurrentBRColumn, * cMaxBRColumn;

            fDest = pfTotDmg[j] * (float) gcConfig.iPercentDamageUsedToDestroy / 100;
            fDV = pfTotDmg[j] - fDest;

            iTempMineIndex = NO_KEY;

            if (piBattleEmpireIndex[j] == INDEPENDENT) {

                cActionColumn = GameEmpireShips::Action;
                pszTypeColumn = GameEmpireShips::Type;
                cCurrentBRColumn = GameEmpireShips::CurrentBR;
                cMaxBRColumn = GameEmpireShips::MaxBR;

            } else {

                cActionColumn = GameEmpireShips::Action;
                pszTypeColumn = GameEmpireShips::Type;
                cCurrentBRColumn = GameEmpireShips::CurrentBR;
                cMaxBRColumn = GameEmpireShips::MaxBR;
            }

            // Apply DEST
            for (int iCurrentShip = 0; iCurrentShip < piNumBattleShips[j]; iCurrentShip ++) {

                switch (ppiBattleShipType[j][iCurrentShip]) {

                    case MINEFIELD:

                        //
                        // Mines are immune to DEST
                        //

                        // Detonate?
                        iErrCode = t_pCache->ReadData(
                            ppszEmpireShips[j],
                            ppiBattleShipKey[j][iCurrentShip], 
                            cActionColumn, 
                            &vTemp
                            );

                        RETURN_ON_ERROR(iErrCode);

                        if (vTemp.GetInteger() == DETONATE) {

                            if (!bMines) {
                                bMines = true;
                                bDetonated = true;
                                iMineOwner = j;
                                iMineIndex = iCurrentShip;
                            }

                            // Kill ship
                            ppbAlive[j][iCurrentShip] = false;
                            piBattleShipsDestroyed[j] ++;

                            // Reduce empire's battle points
                            pfBP[j] -= ppfBattleShipBP[j][iCurrentShip];

                        } else {

                            if (iTempMineIndex == NO_KEY) {
                                iTempMineIndex = iCurrentShip;
                            }
                        }

                        break;

                    case CARRIER:

                        // A capable carrier?
                        if (fDest >= ppfBattleShipBP[j][iCurrentShip] &&
                            ppfRealBR[j][iCurrentShip] >= gcConfig.fCarrierCost) {

                                float fAbsorbed = GetCarrierDESTAbsorption (ppfRealBR[j][iCurrentShip]);

                                if (fDest < fAbsorbed) {
                                    fDest = 0;
                                } else {
                                    fDest -= fAbsorbed;
                                }

                                float fNewBR = ppfRealBR[j][iCurrentShip] - gcConfig.fCarrierCost;

                                // Reduce ship BR
                                if (fNewBR <= FLOAT_PROXIMITY_TOLERANCE) {

                                    // Kill ship
                                    ppbAlive[j][iCurrentShip] = false;
                                    piBattleShipsDestroyed[j] ++;

                                    // Reduce empire's battle points
                                    pfBP[j] -= ppfBattleShipBP[j][iCurrentShip];

                                } else {

                                    const char* pszEmpireData;
                                    int iEmpireKey;

                                    if (piBattleEmpireIndex[j] == INDEPENDENT) {
                                        pszEmpireData = NULL;
                                        iEmpireKey = INDEPENDENT;
                                    } else {
                                        pszEmpireData = pstrEmpireData[piBattleEmpireIndex[j]];
                                        iEmpireKey = piEmpireKey[piBattleEmpireIndex[j]];
                                    }

                                    // Reduce ship's BR
                                    iErrCode = ChangeShipTypeOrMaxBR (
                                        ppszEmpireShips[j],
                                        pszEmpireData,
                                        iEmpireKey,
                                        ppiBattleShipKey[j][iCurrentShip],
                                        CARRIER,
                                        CARRIER,
                                        - gcConfig.fCarrierCost
                                        );

                                    RETURN_ON_ERROR(iErrCode);

									float fNewBP = fNewBR * fNewBR;

									// Reduce empire's battle points
                                    pfBP[j] -= ppfBattleShipBP[j][iCurrentShip];
                                    pfBP[j] += fNewBP;

                                    // Adjust ship's BR, battle points
                                    ppfRealBR[j][iCurrentShip] = fNewBR;
                                    ppfBattleShipBP[j][iCurrentShip] = fNewBP;
                                }
                            }
                            break;

                    case MINESWEEPER:

                        ppbSweep[j][iCurrentShip] = true;
                        // Fall through

                    default:

                        if (fDest >= ppfBattleShipBP[j][iCurrentShip]) {

                            // Kill ship
                            ppbAlive[j][iCurrentShip] = false;
                            piBattleShipsDestroyed[j] ++;

                            // Reduce empire's battle points
                            pfBP[j] -= ppfBattleShipBP[j][iCurrentShip];

                            // Reduce DEST
                            fDest -= ppfBattleShipBP[j][iCurrentShip];
                        }
                        break;

                }   // End type switch

            }   // End ships loop

            Assert(fDest >= 0.0);

            // Apply damage ratio
            fDamageRatio = (fDV + fDest) / pfBP[j];

            if (fDamageRatio >= (float) 1.0 - FLOAT_PROXIMITY_TOLERANCE) {

                for (k = 0; k < piNumBattleShips[j]; k ++) {
                    ppbAlive[j][k] = false;
                }

                if (iTempMineIndex != NO_KEY && !bMines) {

                    Assert(iMineOwner == NO_KEY);
                    Assert(iMineIndex == NO_KEY);

                    bMines = true;
                    iMineOwner = j;
                    iMineIndex = iTempMineIndex;
                }

                piBattleShipsDestroyed[j] = piNumBattleShips[j];

            } else {

                fFactor = (float) 1.0 - fDamageRatio;

                SafeRelease(pShipsTableW);
                iErrCode = t_pCache->GetTable(ppszEmpireShips[j], &pShipsTableW);
                RETURN_ON_ERROR(iErrCode);

                // Reduce BR of remaining ships
                for (k = 0; k < piNumBattleShips[j]; k ++) {

                    if (!ppbAlive[j][k]) {
                        continue;
                    }

                    unsigned int iShipKey = ppiBattleShipKey[j][k];

                    float fMaxBR, fNextBR = ppfRealBR[j][k] * fFactor;
                    iErrCode = pShipsTableW->ReadData (iShipKey, cMaxBRColumn, &fMaxBR);
                    RETURN_ON_ERROR(iErrCode);

                    if (fNextBR > fMaxBR) {
                        fNextBR = fMaxBR;
                    }
                    Assert(fNextBR >= 0.0);

                    // Write new BR of ship
                    iErrCode = pShipsTableW->WriteData(iShipKey, cCurrentBRColumn, fNextBR);
                    RETURN_ON_ERROR(iErrCode);

                }   // End for each ship

            }  // End if any ships survived

        } // End empires loop

        // Check for surviving sweepers if a mine went off
        for (j = 0; bMines && j < (int)iNumBattleEmpires; j ++) {
            for (k = 0; bMines && k < (int)piNumBattleShips[j]; k ++) {

                if (ppbAlive[j][k] && ppbSweep[j][k]) {
                    bMines = false;
                }
            }
        }

        if (bMines) {

            ///////////////////
            // Mine goes off //
            ///////////////////

            iErrCode = MinefieldExplosion (
                strGameMap,
                pstrEmpireData, 
                iPlanetKey,
                iNumEmpires,
                piEmpireKey,
                piTotalMin,
                piTotalFuel
                );

            RETURN_ON_ERROR(iErrCode);

            // Kill all ships on planet
            for (j = 0; j < (int)iNumBattleEmpires; j ++) {
                for (k = 0; k < (int)piNumBattleShips[j]; k ++) {
                    ppbAlive[j][k] = false;
                }

                piBattleShipsDestroyed[j] = piNumBattleShips[j];
            }

            // Get mine name
            if (piBattleEmpireIndex[iMineOwner] != INDEPENDENT) {

                iErrCode = t_pCache->ReadData(
                    pstrEmpireShips[piBattleEmpireIndex[iMineOwner]],
                    ppiBattleShipKey[iMineOwner][iMineIndex],
                    GameEmpireShips::Name,
                    &vMineName
                    );

                RETURN_ON_ERROR(iErrCode);
            }

        }   // End if mines

        //////////////////////////////
        // Conflict update messages //
        //////////////////////////////          

        // Was a ship destroyed?
        for (j = 0; j < (int)iNumBattleEmpires; j ++) {
            if (piBattleShipsDestroyed[j] > 0) {
                break;
            }
        }

        // Process destroyed ships
        if (j < (int)iNumBattleEmpires) {

            String strBattleEmpires;

            // Build battle empire list
            for (k = 0; k < (int)iNumBattleEmpires; k ++) {

                int iShipsToDeclare = piNumBattleShips[k];

                if (piBattleEmpireIndex[k] == INDEPENDENT) {

                    strBattleEmpires += " * " BEGIN_STRONG;
                    strBattleEmpires += iShipsToDeclare;
                    strBattleEmpires += " " INDEPENDENT_NAME END_STRONG " ship";
                    if (iShipsToDeclare != 1) {
                        strBattleEmpires += "s";
                    }

                } else {

                    strBattleEmpires += " * " BEGIN_STRONG;
                    strBattleEmpires += pvEmpireName[piBattleEmpireIndex[k]].GetCharPtr();
                    strBattleEmpires += END_STRONG ": " BEGIN_STRONG;
                    strBattleEmpires += iShipsToDeclare;
                    strBattleEmpires += END_STRONG " ship";
                    if (iShipsToDeclare != 1) {
                        strBattleEmpires += "s";
                    }
                }

                strBattleEmpires += " (";

                // Count ships by type
                unsigned int piShipsByType [NUM_SHIP_TYPES];
                memset (piShipsByType, 0, sizeof (piShipsByType));

                int iType;
                for (l = 0; l < iShipsToDeclare; l ++) {

                    iType = ppiBattleShipType[k][l];
                    Assert(iType >= FIRST_SHIP && iType <= LAST_SHIP);
                    piShipsByType [iType] ++;
                }

                int iCounted = 0;
                ENUMERATE_SHIP_TYPES (iType) {

                    int iNumShips = piShipsByType [iType];
                    if (iNumShips > 0) {

                        if (iCounted > 0) {
                            strBattleEmpires += ", ";
                        }

                        //strBattleEmpires += BEGIN_STRONG;
                        strBattleEmpires += iNumShips;
                        //strBattleEmpires += END_STRONG;
                        strBattleEmpires += " ";
                        if (iNumShips == 1) {
                            strBattleEmpires += SHIP_TYPE_STRING_LOWERCASE [iType];
                        } else {
                            strBattleEmpires += SHIP_TYPE_STRING_LOWERCASE_PLURAL [iType];
                        }

                        iCounted += iNumShips;
                    }
                }

                strBattleEmpires += ")\n";
            }

            // Add battle notification to update messages
            for (j = 0; j < iNumWatchers; j ++) {

                pstrUpdateMessage[piWatcherIndex[j]] += BEGIN_STRONG "There was a fleet battle at ";
                pstrUpdateMessage[piWatcherIndex[j]].AppendHtml (pszPlanetName, 0, false);
                pstrUpdateMessage[piWatcherIndex[j]] += " between:" END_STRONG "\n";

                // Add empires participating in battle
                pstrUpdateMessage[piWatcherIndex[j]] += strBattleEmpires;

                // Add minefield detonation if necessary
                if (bMines) {

                    pstrUpdateMessage[piWatcherIndex[j]] += BEGIN_BAD_FONT(piWatcherIndex[j]);

                    if (piBattleEmpireIndex[iMineOwner] == INDEPENDENT) {

                        pstrUpdateMessage[piWatcherIndex[j]] += "An " BEGIN_STRONG "Independent" END_STRONG " ";
                        pstrUpdateMessage[piWatcherIndex[j]] += SHIP_TYPE_STRING [MINEFIELD];

                    } else {

                        pstrUpdateMessage[piWatcherIndex[j]] += SHIP_TYPE_STRING [MINEFIELD];
                        pstrUpdateMessage[piWatcherIndex[j]] += " ";
                        pstrUpdateMessage[piWatcherIndex[j]].AppendHtml (vMineName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[piWatcherIndex[j]] += " of " BEGIN_STRONG;
                        pstrUpdateMessage[piWatcherIndex[j]] += pvEmpireName[piBattleEmpireIndex[iMineOwner]].GetCharPtr();
                        pstrUpdateMessage[piWatcherIndex[j]] += END_STRONG;
                    }

                    if (bDetonated) {
                        pstrUpdateMessage[piWatcherIndex[j]] += " was " BEGIN_STRONG "detonated" END_STRONG " at ";
                    } else {
                        pstrUpdateMessage[piWatcherIndex[j]] += " " BEGIN_STRONG "exploded" END_STRONG " at ";
                    }

                    pstrUpdateMessage[piWatcherIndex[j]].AppendHtml (pszPlanetName, 0, false);
                    pstrUpdateMessage[piWatcherIndex[j]] += END_FONT "\n";
                }
            }

            // Destroy each battling empire's ships
            for (j = 0; j < (int)iNumBattleEmpires; j ++) {

                strList.Clear();

                if (piBattleEmpireIndex[j] == INDEPENDENT) {

                    for (k = 0; k < piNumBattleShips[j]; k ++) {

                        if (!ppbAlive[j][k]) {

                            // Destroy ship
                            iErrCode = DeleteShip (iGameClass, iGameNumber, INDEPENDENT, ppiBattleShipKey[j][k]);
                            RETURN_ON_ERROR(iErrCode);
                        }
                    }
                    iCounter = piBattleShipsDestroyed[j];

                } else {

                    iCounter = 0;

                    if (piBattleShipsDestroyed[j] > 0) {

                        // Make a list of the empire's destroyed ships
                        for (k = 0; k < piNumBattleShips[j]; k ++) {

                            if (ppbAlive[j][k]) {
                                continue;
                            }

                            // Get ship name
                            iErrCode = t_pCache->ReadData(
                                ppszEmpireShips[j], 
                                ppiBattleShipKey[j][k], 
                                GameEmpireShips::Name, 
                                &vTemp
                                );
                            RETURN_ON_ERROR(iErrCode);

                            // Add ship name to list
                            if (iCounter > 0) {
                                strList += ", ";
                            }
                            strList.AppendHtml (vTemp.GetCharPtr(), 0, false);
                            iCounter ++;

                            // Destroy ship
                            iErrCode = DeleteShip (
                                iGameClass, 
                                iGameNumber, 
                                piEmpireKey[piBattleEmpireIndex[j]], 
                                ppiBattleShipKey[j][k]
                                );

                            RETURN_ON_ERROR(iErrCode);
                        }
                    }

                    // Add battling empires' cloaked mine victims
                    if (bMines) {

                        for (k = 0; k < piNumCloaked[j]; k ++) {

                            // Get cloaked ship name
                            iErrCode = t_pCache->ReadData(
                                ppszEmpireShips[j], 
                                ppiCloakerKey[j][k], 
                                GameEmpireShips::Name, 
                                &vTemp
                                );

                            RETURN_ON_ERROR(iErrCode);

                            // Add ship name to list
                            if (iCounter > 0) {
                                strList += ", ";
                            }
                            strList.AppendHtml (vTemp.GetCharPtr(), 0, false);
                            iCounter ++;

                            // Destroy the ship
                            iErrCode = DeleteShip (
                                iGameClass, 
                                iGameNumber, 
                                piEmpireKey[piBattleEmpireIndex[j]], 
                                ppiCloakerKey[j][k]
                                );

                            RETURN_ON_ERROR(iErrCode);
                        }

                    }   // End if mines

                }   // End test not independent

                // Add to update messages
                if (iCounter > 0) {

                    String strBattleMessage;

                    if (piBattleEmpireIndex[j] == INDEPENDENT) {

                        strBattleMessage = BEGIN_STRONG;
                        strBattleMessage += iCounter;
                        strBattleMessage += " Independent" END_STRONG " ship";

                        if (iCounter == 1) {
                            strBattleMessage += " was " BEGIN_STRONG "destroyed" END_STRONG " at ";
                        } else {
                            strBattleMessage += "s were " BEGIN_STRONG "destroyed" END_STRONG " at ";
                        }

                        strBattleMessage.AppendHtml (pszPlanetName, 0, false);
                        strBattleMessage += "\n";

                    } else {

                        strBattleMessage = BEGIN_STRONG;
                        strBattleMessage += iCounter;
                        strBattleMessage += END_STRONG " ship";
                        if (iCounter != 1) {
                            strBattleMessage += END_STRONG "s";
                        }

                        strBattleMessage += " of " BEGIN_STRONG;
                        strBattleMessage += pvEmpireName[piBattleEmpireIndex[j]].GetCharPtr();
                        strBattleMessage += END_STRONG;

                        if (iCounter == 1) {
                            strBattleMessage += " was " BEGIN_STRONG "destroyed" END_STRONG " at ";
                        } else {
                            strBattleMessage += " were " BEGIN_STRONG "destroyed" END_STRONG " at ";
                        }

                        strBattleMessage.AppendHtml (pszPlanetName, 0, false);

                        strBattleMessage += ": ";
                        strBattleMessage += strList;

                        strBattleMessage += "\n";
                    }

                    for (k = 0; k < iNumWatchers; k ++) {
                        pstrUpdateMessage[piWatcherIndex[k]] += strBattleMessage;
                    }
                }
            }   // End battle empire loop

            // If a minefield exploded, add all innocent passers-by to the list of the dead
            if (bMines) {

                for (j = 0; j < iNumEmpires; j ++) {

                    if (!pbAlive[j] || pbBattleEmpire[j]) {
                        continue;
                    }

                    if (piShipKey)
                    {
                        t_pCache->FreeKeys(piShipKey);
                        piShipKey = NULL;
                    }

                    // Get the ship keys
                    iErrCode = t_pCache->GetEqualKeys (
                        pstrEmpireShips[j], 
                        GameEmpireShips::CurrentPlanet, 
                        iPlanetKey, 
                        &piShipKey, 
                        &iNumShips
                        );

                    if (iErrCode == ERROR_DATA_NOT_FOUND)
                    {
                        iErrCode = OK;
                        continue;
                    }
                    RETURN_ON_ERROR(iErrCode);

                    iCounter = 0;
                    strList.Clear();

                    for (k = 0; k < (int) iNumShips; k ++)
                    {
                        iErrCode = t_pCache->ReadData(
                            pstrEmpireShips[j], 
                            piShipKey[k], 
                            GameEmpireShips::Name, 
                            &vTemp
                            );

                        RETURN_ON_ERROR(iErrCode);

                        // Add ship name to list
                        if (iCounter > 0) {
                            strList += ", ";
                        }
                        strList.AppendHtml (vTemp.GetCharPtr(), 0, false);
                        iCounter ++;

                        // Destroy the ship
                        iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[j], piShipKey[k]);
                        RETURN_ON_ERROR(iErrCode);
                    }

                    String strBattleMessage = BEGIN_STRONG;
                    strBattleMessage += iCounter;
                    strBattleMessage += END_STRONG " ship";
                    if (iCounter != 1) {
                        strBattleMessage += END_STRONG "s";
                    }

                    strBattleMessage += " of " BEGIN_STRONG;
                    strBattleMessage += pvEmpireName[j].GetCharPtr();
                    strBattleMessage += END_STRONG;

                    if (iCounter == 1) {
                        strBattleMessage += " was " BEGIN_STRONG "destroyed" END_STRONG " at ";
                    } else {
                        strBattleMessage += " were " BEGIN_STRONG "destroyed" END_STRONG " at ";
                    }

                    strBattleMessage.AppendHtml (pszPlanetName, 0, false);

                    strBattleMessage += ": ";
                    strBattleMessage += strList;

                    strBattleMessage += "\n";

                    for (k = 0; k < iNumWatchers; k ++)
                    {
                        pstrUpdateMessage[piWatcherIndex[k]] += strBattleMessage;
                    }

                }   // End all empires loop

            }   // End if mines

        }   // End if some ship was destroyed

    }  // End of battle

    return iErrCode;
}


int GameEngine::MinefieldExplosion (const char* strGameMap, const char** pstrEmpireData, 
                                    unsigned int iPlanetKey, unsigned int iNumEmpires, 
                                    unsigned int* piEmpireKey, int* piTotalMin, int* piTotalFuel) {

    int iErrCode, iFinalPop;
    Variant vOwner, vPop;

    // Get planet owner
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    RETURN_ON_ERROR(iErrCode);

    if (vOwner.GetInteger() != SYSTEM) {

        // Get pop
        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Pop, &vPop);
        RETURN_ON_ERROR(iErrCode);

        iFinalPop = (int) ceil ((float) vPop.GetInteger() / 2);
        Assert(iFinalPop >= 0);

        if (vOwner.GetInteger() != INDEPENDENT) {

            unsigned int iOwner;
            int iDiff;
            Variant vMin, vFuel;

            GetEmpireIndex (iOwner, vOwner);

            // Reduce owner's resources and econ
            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
            RETURN_ON_ERROR(iErrCode);

            iDiff = min (vMin.GetInteger(), iFinalPop) - min (vMin.GetInteger(), vPop.GetInteger());
            piTotalMin[iOwner] += iDiff;
            
            iDiff = min (vFuel.GetInteger(), iFinalPop) - min (vFuel.GetInteger(), vPop.GetInteger());
            piTotalFuel[iOwner] += iDiff;
            
            iErrCode = t_pCache->Increment(pstrEmpireData[iOwner], GameEmpireData::TotalPop, iFinalPop - vPop.GetInteger());
            RETURN_ON_ERROR(iErrCode);
        }

        // Reduce pop
        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Pop, iFinalPop);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::MakeMinefieldsDetonate (int iGameClass, int iGameNumber, const char* strGameMap, 
                                        unsigned int iNumEmpires, unsigned int* piEmpireKey, 
                                        bool* pbAlive, Variant* pvEmpireName, const char** pstrEmpireShips, 
                                        const char** pstrEmpireMap, String* pstrUpdateMessage, 
                                        const char** pstrEmpireData, int* piTotalMin, int* piTotalFuel, 
                                        bool bIndependence, const char* strIndependentShips, 
                                        Variant* pvGoodColor, Variant* pvBadColor, 
                                        const GameConfiguration& gcConfig) {

    int iErrCode = OK, iX, iY;
    unsigned int i, j, k, l, iNumMines;

    String strPrefix, strMessage;
    Variant vPlanetName, vCoordinates, vNumShips, vOwner, vShipName, vType, vTemp;
    bool bSweep;

    const char* pszShipTable;

    unsigned int iNumAdjustedEmpires = iNumEmpires;
    const char* pszCurrentPlanetColumn, * pszTypeColumn;

    if (bIndependence) {
        iNumAdjustedEmpires ++;
    }

    unsigned int* piProxyPlanetKey = (unsigned int*) StackAlloc (iNumAdjustedEmpires * sizeof (unsigned int));
    memset (piProxyPlanetKey, NO_KEY, iNumAdjustedEmpires * sizeof (unsigned int));

    unsigned int** ppiShipKey = (unsigned int**) StackAlloc (iNumAdjustedEmpires * sizeof (unsigned int*));
    memset (ppiShipKey, NULL, iNumAdjustedEmpires * sizeof (unsigned int*));

    AutoFreeArrayOfKeys free_ppiShipKey(ppiShipKey, iNumAdjustedEmpires);

    unsigned int* piNumShips = (unsigned int*) StackAlloc (iNumAdjustedEmpires * sizeof (unsigned int));
    memset (piNumShips, 0, iNumAdjustedEmpires * sizeof (unsigned int));

    // Loop through all empires
    for (i = 0; i < iNumEmpires; i ++) {
        
        if (!pbAlive[i]) {
            continue;
        }

        unsigned int* piMineKey = NULL;
        AutoFreeKeys free_piMineKey(piMineKey);

        iErrCode = t_pCache->GetEqualKeys (
            pstrEmpireShips[i], 
            GameEmpireShips::Action,
            DETONATE,
            &piMineKey, 
            &iNumMines
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            continue;
        }

        RETURN_ON_ERROR(iErrCode);

        for (j = 0; j < iNumMines; j ++)
        {
            //
            // Mine may no longer exist, so check first...
            //

            // Get its planet
            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piMineKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
            if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
                continue;
            RETURN_ON_ERROR(iErrCode);
            unsigned int iPlanetKey = vTemp.GetInteger();

            // Maybe we don't need to detonate after all?
            if (gcConfig.iShipBehavior & MINEFIELD_DISABLE_DETONATE)
            {
                iErrCode = t_pCache->WriteData(pstrEmpireShips[i], piMineKey[j], GameEmpireShips::Action, STAND_BY);
                RETURN_ON_ERROR(iErrCode);
            }

            // Is there a minesweeper on the planet?
            bSweep = false;
            for (k = 0; !bSweep && k < iNumAdjustedEmpires; k ++) {

                if (k == iNumEmpires) {

                    pszShipTable = strIndependentShips;
                    pszCurrentPlanetColumn = GameEmpireShips::CurrentPlanet;
                    pszTypeColumn = GameEmpireShips::Type;
    
                } else {

                    if (!pbAlive[k]) {
                        continue;
                    }
                
                    // Is planet on empire's map?
                    iErrCode = t_pCache->GetFirstKey(
                        pstrEmpireMap[k], 
                        GameEmpireMap::PlanetKey,
                        iPlanetKey,
                        piProxyPlanetKey + k
                        );
                    
                    if (iErrCode == ERROR_DATA_NOT_FOUND) {
                        iErrCode = OK;
                        continue;
                    }
                    RETURN_ON_ERROR(iErrCode);

                    pszShipTable = pstrEmpireShips[k];
                    pszCurrentPlanetColumn = GameEmpireShips::CurrentPlanet;
                    pszTypeColumn = GameEmpireShips::Type;
                }
                
                // Does empire have ships on planet?
                iErrCode = t_pCache->GetEqualKeys(
                    pszShipTable,
                    pszCurrentPlanetColumn,
                    iPlanetKey,
                    ppiShipKey + k, 
                    piNumShips + k
                    );
                
                if (iErrCode == ERROR_DATA_NOT_FOUND)
                {
                    iErrCode = OK;
                    continue;
                }
                RETURN_ON_ERROR(iErrCode);

                for (l = 0; l < piNumShips[k]; l ++) {

                    iErrCode = t_pCache->ReadData(
                        pszShipTable,
                        ppiShipKey[k][l],
                        pszTypeColumn,
                        &vType
                        );
                    
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (vType.GetInteger() == MINESWEEPER) {
                        bSweep = true;
                        break;
                    }
                }
            }

            // Get ship name
            iErrCode = t_pCache->ReadData(
                pstrEmpireShips[i],
                piMineKey[j],
                GameEmpireShips::Name,
                &vShipName
                );

            RETURN_ON_ERROR(iErrCode);

            // Get planet name, coordinates, owner
            iErrCode = t_pCache->ReadData(
                strGameMap, 
                iPlanetKey, 
                GameMap::Name, 
                &vPlanetName
                );

            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(
                strGameMap, 
                iPlanetKey, 
                GameMap::Coordinates, 
                &vCoordinates
                );

            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
            RETURN_ON_ERROR(iErrCode);

            GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);

            // Prefix to update messages
            strPrefix = SHIP_TYPE_STRING [MINEFIELD];
            strPrefix += " ";
            strPrefix.AppendHtml (vShipName.GetCharPtr(), 0, false);
            strPrefix += " of " BEGIN_STRONG;
            strPrefix += pvEmpireName[i].GetCharPtr();
            
            if (bSweep) {
                
                strPrefix += END_STRONG " was " BEGIN_STRONG "defused" END_STRONG " and destroyed by a ";
                strPrefix += SHIP_TYPE_STRING_LOWERCASE [MINESWEEPER];
                strPrefix += " at ";

            } else {
                
                strPrefix += END_STRONG " was " BEGIN_STRONG "detonated" END_STRONG " at ";
            }

            AddPlanetNameAndCoordinates (strPrefix, vPlanetName.GetCharPtr(), iX, iY);
            strPrefix += END_FONT "\n";

            // End strPrefix

            if (bSweep) {

                // Destroy mine
                iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piMineKey[j]);
                RETURN_ON_ERROR(iErrCode);

            } else {
        
                // Reduce planet population
                iErrCode = MinefieldExplosion (
                    strGameMap,
                    pstrEmpireData, 
                    iPlanetKey,
                    iNumEmpires,
                    piEmpireKey,
                    piTotalMin,
                    piTotalFuel
                    );

                RETURN_ON_ERROR(iErrCode);

                strMessage = (const char*) NULL;
                
                // Destroy ships from all empires
                for (k = 0; k < iNumAdjustedEmpires; k ++) {

                    if (piNumShips[k] == 0) {
                        continue;
                    }

                    if (k == iNumEmpires) {
                        
                        strMessage += BEGIN_STRONG;
                        strMessage += piNumShips[k];
                        strMessage += " Independent" END_STRONG " ship";
                        
                        if (piNumShips[k] == 1) {
                            strMessage += " was " BEGIN_STRONG "destroyed" END_STRONG " at ";
                        } else {
                            strMessage += "s were " BEGIN_STRONG "destroyed" END_STRONG " at ";
                        }

                        AddPlanetNameAndCoordinates (strMessage, vPlanetName.GetCharPtr(), iX, iY);
                        strMessage += "\n";
                        
                        for (l = 0; l < piNumShips[k]; l ++) {
                            
                            iErrCode = DeleteShip (
                                iGameClass, 
                                iGameNumber, 
                                INDEPENDENT, 
                                ppiShipKey[k][l]
                                );

                            RETURN_ON_ERROR(iErrCode);
                        }
                        
                    } else {

                        for (l = 0; l < piNumShips[k]; l ++) {

                            // Get ship name
                            iErrCode = t_pCache->ReadData(
                                pstrEmpireShips[k], 
                                ppiShipKey[k][l],
                                GameEmpireShips::Name, 
                                &vShipName
                                );
                            
                            RETURN_ON_ERROR(iErrCode);
                            
                            iErrCode = DeleteShip (
                                iGameClass, 
                                iGameNumber, 
                                piEmpireKey[k], 
                                ppiShipKey[k][l]
                                );
                            
                            RETURN_ON_ERROR(iErrCode);
                            
                            if (l > 0) {
                                strMessage += ", ";
                            }
                            
                            strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        }
                                                
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[k].GetCharPtr();
                        strMessage += END_STRONG;
                        
                        if (piNumShips[k] == 1) {
                            strMessage += " was " BEGIN_STRONG "destroyed" END_STRONG " at ";
                        } else {
                            strMessage += " were " BEGIN_STRONG "destroyed" END_STRONG " at ";
                        }
                        
                        AddPlanetNameAndCoordinates (strMessage, vPlanetName.GetCharPtr(), iX, iY);
                        strMessage += "\n";
                    
                    }   // End if independent
                    
                }   // End empire ship destruction loop
            
            }   // End if no sweeper was found

            // Flush update messages
            for (k = 0; k < iNumEmpires; k ++) {
                
                if (piProxyPlanetKey[k] != NO_KEY) {

                    if (piEmpireKey[k] == piEmpireKey[i]) {
                        pstrUpdateMessage[k] += BEGIN_GOOD_FONT(k);
                    } else {
                        pstrUpdateMessage[k] += BEGIN_BAD_FONT(k);
                    }
                    pstrUpdateMessage[k] += strPrefix;

                    pstrUpdateMessage[k] += strMessage;
                }
            }
            
        }   // End mine loop

    }   // End empire loop

    return iErrCode;
}

int GameEngine::AddShipSightings (unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive,
                                  String* pstrUpdateMessage, const Variant* pvEmpireName, 
                                  bool bIndependence,
                                  unsigned int iNumPlanets, unsigned int* piPlanetKey,
                                  const char* strGameMap, const char** pstrEmpireMap, 
                                  const char** pstrEmpireShips, const char* strIndependentShips
                                  ) {

    int iErrCode = OK;

    unsigned int i, j, k, iNumWatchers, iProxyKey, iNumAdjustedEmpires, * piShipKey = NULL, iNumShips, iNumVisibleShips;
    AutoFreeKeys free_piShipKey(piShipKey);

    unsigned int* piWatcherIndex = (unsigned int*) StackAlloc (iNumEmpires * sizeof (unsigned int));
    memset (piWatcherIndex, NO_KEY, iNumEmpires * sizeof (unsigned int));

    const char* pszStateColumn;
    String strList;
    
    char pszPlanetName[MAX_PLANET_NAME_WITH_COORDINATES_LENGTH] = "";
    Variant vState, vName;

    const char* pszShips;

    iNumAdjustedEmpires = iNumEmpires;
    if (bIndependence)
    {
        iNumAdjustedEmpires ++;
    }

    for (i = 0; i < iNumPlanets; i ++)
    {
        // Calculate watchers
        iNumWatchers = 0;

        for (j = 0; j < iNumEmpires; j ++) {

            if (!pbAlive[j]) {
                continue;
            }

            // Is planet on empire's map?
            iErrCode = t_pCache->GetFirstKey(
                pstrEmpireMap[j],
                GameEmpireMap::PlanetKey,
                piPlanetKey[i],
                &iProxyKey
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            else
            {
                RETURN_ON_ERROR(iErrCode);

                piWatcherIndex[iNumWatchers] = j;
                iNumWatchers ++;
            }
        }

        if (iNumWatchers > 0)
        {
            for (j = 0; j < iNumAdjustedEmpires; j ++) {

                if (piShipKey)
                {
                    t_pCache->FreeKeys(piShipKey);
                    piShipKey = NULL;
                }

                if (j == iNumEmpires) {

                    // Get independent ships
                    Assert(bIndependence);

                    iErrCode = t_pCache->GetEqualKeys (
                        strIndependentShips,
                        GameEmpireShips::CurrentPlanet,
                        piPlanetKey[i],
                        &piShipKey,
                        &iNumShips
                        );

                    if (iErrCode == ERROR_DATA_NOT_FOUND)
                    {
                        iErrCode = OK;
                    }
                    RETURN_ON_ERROR(iErrCode);

                    pszShips = strIndependentShips;
                    pszStateColumn = GameEmpireShips::State;
                
                } else {

                    if (!pbAlive[j]) {
                        continue;
                    }

                    iErrCode = t_pCache->GetEqualKeys (
                        pstrEmpireShips[j],
                        GameEmpireShips::CurrentPlanet,
                        piPlanetKey[i],
                        &piShipKey,
                        &iNumShips
                        );

                    if (iErrCode == ERROR_DATA_NOT_FOUND)
                    {
                        iErrCode = OK;
                    }
                    RETURN_ON_ERROR(iErrCode);
                    
                    pszShips = pstrEmpireShips[j];
                    pszStateColumn = GameEmpireShips::State;
                }

                if (iNumShips > 0) {

                    Assert(iErrCode == OK);
                    
                    strList.Clear();
                    iNumVisibleShips = 0;

                    for (k = 0; k < iNumShips; k ++) {
                        
                        // Add to list if visible
                        iErrCode = t_pCache->ReadData(
                            pszShips,
                            piShipKey[k], 
                            pszStateColumn,
                            &vState
                            );

                        RETURN_ON_ERROR(iErrCode);

                        if (!(vState.GetInteger() & CLOAKED)) {

                            if (j < iNumEmpires) {
                            
                                iErrCode = t_pCache->ReadData(
                                    pstrEmpireShips[j], 
                                    piShipKey[k], 
                                    GameEmpireShips::Name, 
                                    &vName
                                    );

                                RETURN_ON_ERROR(iErrCode);
                                
                                if (iNumVisibleShips > 0) {
                                    strList += ", ";
                                }
                                strList.AppendHtml (vName.GetCharPtr(), 0, false);
                            }

                            iNumVisibleShips ++;
                        }

                    }   // End ship loop

                    if (iNumVisibleShips > 0)
                    {
                        String shipSighting;
                        
                        iErrCode = GetPlanetNameWithCoordinates(strGameMap, piPlanetKey[i], pszPlanetName);
                        RETURN_ON_ERROR(iErrCode);

                        shipSighting = BEGIN_STRONG;
                        shipSighting += iNumVisibleShips;

                        if (j < iNumEmpires) {

                            shipSighting += END_STRONG " ship";
                            if (iNumVisibleShips != 1) {
                                shipSighting += "s of " BEGIN_STRONG;
                            } else {
                                shipSighting += " of " BEGIN_STRONG;
                            }

                            shipSighting += pvEmpireName[j].GetCharPtr();
                        
                            if (iNumVisibleShips == 1) {
                                shipSighting += END_STRONG " was";
                            } else {
                                shipSighting += END_STRONG " were";
                            }

                            shipSighting += " sighted at ";
                            shipSighting.AppendHtml (pszPlanetName, 0, false);
                            shipSighting += ": ";
                            shipSighting += strList;
                            shipSighting += "\n";

                        } else {

                            // Independent
                            Assert(j == iNumEmpires);

                            if (iNumVisibleShips == 1) {
                                shipSighting += " Independent" END_STRONG " ship was ";
                            } else {
                                shipSighting += " Independent" END_STRONG " ships were ";
                            }

                            shipSighting += " sighted at ";
                            shipSighting.AppendHtml (pszPlanetName, 0, false);
                            shipSighting += "\n";
                        }
                        
                        for (k = 0; k < iNumWatchers; k ++) {

                            if (piWatcherIndex[k] != j) {
                                pstrUpdateMessage[piWatcherIndex[k]] += shipSighting;
                            }
                        }

                    }   // End if visible ships
                
                }   // End if ships on planet

            }   // End empire loop

        }   // If watchers

    }   // End planet loop

    return iErrCode;
}


int GameEngine::UpdateFleetOrders (unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, 
                                   const char* strGameMap, const char** pstrEmpireShips, 
                                   const char** pstrEmpireFleets, const char** pstrEmpireMap,
                                     String* pstrUpdateMessage) {

    int iDirection, iErrCode = OK;
    unsigned int i, iFleetKey, * piShipKey = NULL, k, iNumShips;
    AutoFreeKeys free_piShipKey(piShipKey);

    bool bFlag;
    float fStrength, fMaxStrength;
    Variant vTemp, vFleetPlanet, vAction;
    
    for (i = 0; i < iNumEmpires; i ++) {
        
        if (!pbAlive[i]) {
            continue;
        }

        iFleetKey = NO_KEY;
        while (true)
        {
            iErrCode = t_pCache->GetNextKey (pstrEmpireFleets[i], iFleetKey, &iFleetKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            RETURN_ON_ERROR(iErrCode);

            if (piShipKey)
            {
                t_pCache->FreeKeys(piShipKey);
                piShipKey = NULL;
            }

            // Get ships in fleet
            iErrCode = t_pCache->GetEqualKeys (
                pstrEmpireShips[i],
                GameEmpireShips::FleetKey,
                iFleetKey,
                &piShipKey,
                &iNumShips
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);

            if (iNumShips == 0)
            {
                // Check for auto-disband
                iErrCode = GetEmpireOption2 (piEmpireKey[i], DISBAND_EMPTY_FLEETS_ON_UPDATE, &bFlag);
                RETURN_ON_ERROR(iErrCode);

                if (bFlag)
                {
                    Variant vFleetName;
                    iErrCode = t_pCache->ReadData(pstrEmpireFleets[i], iFleetKey, GameEmpireFleets::Name, &vFleetName);
                    RETURN_ON_ERROR(iErrCode);

                    pstrUpdateMessage[i] += "Fleet ";
                    pstrUpdateMessage[i].AppendHtml (vFleetName.GetCharPtr(), 0, false);
                    pstrUpdateMessage[i] += " was disbanded\n";

                    iErrCode = t_pCache->DeleteRow (pstrEmpireFleets[i], iFleetKey);
                    RETURN_ON_ERROR(iErrCode);
                    continue;
                }
            }

            // Get fleet location
            iErrCode = t_pCache->ReadData(
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::CurrentPlanet, 
                &vFleetPlanet
                );

            RETURN_ON_ERROR(iErrCode);
            
            // Get fleet action
            iErrCode = t_pCache->ReadData(
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::Action, 
                &vAction
                );
            RETURN_ON_ERROR(iErrCode);

            if (vAction.GetInteger() <= MOVE_NORTH && vAction.GetInteger() >= MOVE_WEST)
            {
                iDirection = MOVE_NORTH - vAction.GetInteger();
                Assert(iDirection >= NORTH && iDirection <= WEST);
                
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    vFleetPlanet.GetInteger(), 
                    GameMap::ColumnNames[GameMap::iNorthPlanetKey + iDirection],
                    &vFleetPlanet
                    );
                RETURN_ON_ERROR(iErrCode);
                Assert(vFleetPlanet.GetInteger() != NO_KEY);

#ifdef _DEBUG
                unsigned int iPlanetKey;
                iErrCode = t_pCache->GetFirstKey(
                    pstrEmpireMap[i],
                    GameEmpireMap::PlanetKey,
                    vFleetPlanet.GetInteger(),
                    &iPlanetKey
                    );
                RETURN_ON_ERROR(iErrCode);
                Assert(iPlanetKey != NO_KEY);
#endif

                iErrCode = t_pCache->WriteData(
                    pstrEmpireFleets[i], 
                    iFleetKey, 
                    GameEmpireFleets::CurrentPlanet, 
                    vFleetPlanet
                    );
                RETURN_ON_ERROR(iErrCode);
            }
            
            // Calculate fleet's current strength, max strength
            fStrength = (float) 0.0;
            fMaxStrength = (float) 0.0;
            
            for (k = 0; k < iNumShips; k ++)
            {
                iErrCode = t_pCache->ReadData(
                    pstrEmpireShips[i], 
                    piShipKey[k], 
                    GameEmpireShips::CurrentPlanet, 
                    &vTemp
                    );
                RETURN_ON_ERROR(iErrCode);

                if (vTemp.GetInteger() != vFleetPlanet.GetInteger()) {
                    
                    // Defect from fleet
                    iErrCode = t_pCache->WriteData(
                        pstrEmpireShips[i], 
                        piShipKey[k], 
                        GameEmpireShips::FleetKey, 
                        (int)NO_KEY
                        );
                    RETURN_ON_ERROR(iErrCode);

                } else {

                    // Keep in fleet
                    iErrCode = t_pCache->WriteData(
                        pstrEmpireShips[i], 
                        piShipKey[k], 
                        GameEmpireShips::Action, 
                        FLEET
                        );
                    RETURN_ON_ERROR(iErrCode);
                    
                    // Strength counts
                    iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[k], GameEmpireShips::MaxBR, &vTemp);
                    RETURN_ON_ERROR(iErrCode);
                    
                    fMaxStrength += (float) vTemp.GetFloat() * vTemp.GetFloat();
                    
                    iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[k], GameEmpireShips::CurrentBR, &vTemp);
                    RETURN_ON_ERROR(iErrCode);
                    
                    fStrength += (float) vTemp.GetFloat() * vTemp.GetFloat();
                }
            }

            // Write new fleet strengths
            iErrCode = t_pCache->WriteData(
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::MaxStrength, 
                fMaxStrength
                );
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->WriteData(
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::CurrentStrength, 
                fStrength
                );
            RETURN_ON_ERROR(iErrCode);
            
            // Write standby as the new default action for the fleet
            iErrCode = t_pCache->WriteData(
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::Action, 
                STAND_BY
                );
            RETURN_ON_ERROR(iErrCode);
        }

    }   // End empire loop

    return iErrCode;
}


int GameEngine::UpdateEmpiresEcon (int iGameClass, int iGameNumber, int iNumEmpires, unsigned int* piEmpireKey, 
                                   bool* pbAlive, int* piTotalMin, int* piTotalFuel, int* piTotalAg, 
                                   const Seconds& iUpdatePeriod, const UTCTime& tUpdateTime, 
                                   const char* strGameData, 
                                   const char** pstrEmpireDip, const char** pstrEmpireData, const char** pstrEmpireShips, 
                                   int iNewUpdateCount, const char* strGameMap,
                                   float fMaxAgRatio, const GameConfiguration& gcConfig) {

    int i, j, iErrCode = OK, iEcon, iNumShips, iMil, iBonusMin, iBonusFuel, iBonusAg;
    float fMil;
    
    unsigned int iNumTraders, iNumAllies;

    Variant vTemp, * pvShipMil = NULL, vPop, vMaxPop, vTotalPop, vTotalAg, vMin, vFuel;
    AutoFreeData free_pvShipMil(pvShipMil);

    for (i = 0; i < iNumEmpires; i ++) {

        if (!pbAlive[i]) {
            continue;
        }

        // Count number of empires at trade
        iErrCode = t_pCache->GetEqualKeys (
            pstrEmpireDip[i], 
            GameEmpireDiplomacy::CurrentStatus, 
            TRADE, 
            NULL, 
            (unsigned int*) &iNumTraders
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
        
        // Count number of empires at alliance
        iErrCode = t_pCache->GetEqualKeys (
            pstrEmpireDip[i], 
            GameEmpireDiplomacy::CurrentStatus, 
            ALLIANCE, 
            NULL, 
            (unsigned int*) &iNumAllies
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);

#ifdef _DEBUG

        unsigned int iMaxAllies;
        iErrCode = GetMaxNumDiplomacyPartners (iGameClass, iGameNumber, ALLIANCE, &iMaxAllies);
        RETURN_ON_ERROR(iErrCode);
        Assert(iMaxAllies == UNRESTRICTED_DIPLOMACY || iNumAllies <= iMaxAllies);

        unsigned int iMaxTrade;
        iErrCode = GetMaxNumDiplomacyPartners (iGameClass, iGameNumber, TRADE, &iMaxTrade);
        RETURN_ON_ERROR(iErrCode);
        Assert(iMaxTrade == UNRESTRICTED_DIPLOMACY || iNumTraders <= iMaxTrade);
#endif

        unsigned int iNumTradeRelationships = iNumTraders + iNumAllies;

        // Add trade and alliance advantages to econ
        CalculateTradeBonuses (
            iNumTradeRelationships, piTotalAg[i], piTotalMin[i], piTotalFuel[i],
            gcConfig.iPercentFirstTradeIncrease, gcConfig.iPercentNextTradeIncrease,
            &iBonusAg, &iBonusMin, &iBonusFuel
            );

        // Write new resource totals to database
        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::TotalFuel, piTotalFuel[i]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::TotalMin, piTotalMin[i]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::TotalAg, piTotalAg[i]);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::BonusFuel, iBonusFuel);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::BonusMin, iBonusMin);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::BonusAg, iBonusAg);
        RETURN_ON_ERROR(iErrCode);

        // Write new econ to GameEmpireData
        iEcon = GetEcon (piTotalFuel[i] + iBonusFuel, piTotalMin[i] + iBonusMin, piTotalAg[i] + iBonusAg);

        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::Econ, iEcon);
        RETURN_ON_ERROR(iErrCode);
        
        // Check MaxEcon
        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::MaxEcon, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        
        if (iEcon > vTemp.GetInteger())
        {
            iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::MaxEcon, iEcon);
            RETURN_ON_ERROR(iErrCode);
        }

        if (pvShipMil)
        {
            t_pCache->FreeData(pvShipMil);
            pvShipMil = NULL;
        }
        
        // Calculate Mil
        fMil = (float) 0.0;
        iErrCode = t_pCache->ReadColumn(
            pstrEmpireShips[i], 
            GameEmpireShips::CurrentBR, 
            NULL, 
            &pvShipMil, 
            (unsigned int*) &iNumShips
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);

            for (j = 0; j < iNumShips; j ++)
            {
                fMil += (float) pvShipMil[j] * pvShipMil[j];
            }
        }
        
        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::Mil, fMil);
        RETURN_ON_ERROR(iErrCode);
        
        iMil = GetMilitaryValue (fMil);
        
        // MaxMil
        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::MaxMil, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if (iMil > vTemp.GetInteger())
        {
            iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::MaxMil, iMil);
            RETURN_ON_ERROR(iErrCode);
        }
        
        // Set TotalBuild to 0
        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::TotalBuild, 0);
        RETURN_ON_ERROR(iErrCode);

        // Set NumBuilds to 0
        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::NumBuilds, 0);
        RETURN_ON_ERROR(iErrCode);

        // Set NextMaintenance and NextFuelUse to 0
        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::NextMaintenance, 0);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(pstrEmpireData[i], GameEmpireData::NextFuelUse, 0);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = WriteNextStatistics (
            iGameClass, 
            iGameNumber, 
            piEmpireKey[i],
            piTotalAg[i], 
            iBonusAg,
            fMaxAgRatio
            );

        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


int GameEngine::PerformSpecialActions (int iGameClass, int iGameNumber, int iNumEmpires, 
                                       unsigned int* piEmpireKey, 
                                       const Variant* pvGoodColor, const Variant* pvBadColor,
                                       const Variant* pvEmpireName, bool* pbAlive, unsigned int iNumPlanets, 
                                       unsigned int* piPlanetKey, unsigned int* piOriginalPlanetOwner,
                                       unsigned int* piOriginalNumObliterations,
                                       const char** pstrEmpireShips, const char** pstrEmpireFleets,
                                       const char** pstrEmpireData, const char** pstrEmpireMap, 
                                       String* pstrUpdateMessage, 
                                       const char* strGameMap, const char* strGameData,
                                       int* piTotalAg, int* piTotalMin, int* piTotalFuel, 
                                       const char** pstrEmpireDip, 
                                       int* piObliterator, int* piObliterated, 
                                       unsigned int* piNumObliterations, 
                                       const char* pszGameClassName, int iNewUpdateCount, 
                                       int iGameClassOptions,
                                       unsigned int** ppiShipNukeKey, 
                                       unsigned int** ppiEmpireNukeKey, unsigned int* piNukedPlanetKey, 
                                       unsigned int* piNumNukingShips, unsigned int* piNumNukedPlanets,
                                       const GameConfiguration& gcConfig) {

    Variant vAction, vPlanetName, vShipName, vPop, vOwner, vTemp, vShipType, vAg, 
        vMin, vFuel, vBR, vMaxBR, vLinkPlanetKey, vLinkPlanetName, vCoord, vTechs;

    int iNumShips, i, j, k, l, iTemp, iErrCode = OK, iNumWatcherEmpires, iTerraform, iDiff, iInvadePop, iLinkX, iLinkY, iX, iY;
    
    unsigned int* piShipKey = NULL, * piTempShipKey, * piTempEmpireKey, iKey;
    AutoFreeKeys free_piShipKey(piShipKey);

    bool bActionFlag, bDied, bOriginal, bTarget, bUpdated = false, bDismantle;

    String strMessage, strOriginal;

    // Array of empires that watch the action
    unsigned int* piSpaceForNukingShips = (unsigned int*) StackAlloc (iNumPlanets * sizeof (unsigned int));
    int* piWatcherEmpire = (int*) StackAlloc (iNumEmpires * sizeof (int));
    bool* pbTerraformed = (bool*) StackAlloc (iNumPlanets * sizeof (bool));

    GameUpdateInformation guInfo = { iNewUpdateCount, iNumEmpires, (int*) piEmpireKey, pbAlive, pstrUpdateMessage };

    *piNumNukedPlanets = 0;

    memset (piNumNukingShips, 0, iNumPlanets * sizeof (unsigned int));
    memset (piSpaceForNukingShips, 0, iNumPlanets * sizeof (int));
    memset (pbTerraformed, 0, iNumPlanets * sizeof (bool));

    // Loop through all empires
    for (i = 0; i < iNumEmpires; i ++) {

        if (!pbAlive[i]) {
            continue;
        }

        if (piShipKey)
        {
            t_pCache->FreeKeys(piShipKey);
            piShipKey = NULL;
        }
            
        iErrCode = t_pCache->GetAllKeys (pstrEmpireShips[i], &piShipKey, (unsigned int*) &iNumShips);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            continue;
        }
        RETURN_ON_ERROR(iErrCode);

        // Get developed techs
        iErrCode = t_pCache->ReadData(pstrEmpireData[i], GameEmpireData::TechDevs, &vTechs);
        RETURN_ON_ERROR(iErrCode);
        
        // Randomize ship keys
        Algorithm::Randomize(piShipKey, iNumShips);
        
        for (j = 0; j < iNumShips; j ++) {
            
            // Get ship action
            iErrCode = t_pCache->ReadData(
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::Action, 
                &vAction
                );

            RETURN_ON_ERROR(iErrCode);

            bDied = bDismantle = false;
            
            if (vAction.GetInteger() == NUKE) {

#ifdef _DEBUG
                // Make sure not cloaked
                iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::State, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                
                if (vTemp.GetInteger() & CLOAKED) {
                    Assert(false);
                    continue;
                }

                // Make sure mobile
                iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vTemp);
                RETURN_ON_ERROR(iErrCode);

                if (!IsMobileShip (vTemp.GetInteger())) {
                    Assert(false);
                    continue;
                }

                // Make sure not illegally nuking science ship
                if (vTemp.GetInteger() == SCIENCE && (iGameClassOptions & DISABLE_SCIENCE_SHIPS_NUKING)) {
                    Assert(false);
                    continue;
                }
#endif
                // Add to nuke lookup tables
                iErrCode = t_pCache->ReadData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::CurrentPlanet, 
                    &vTemp
                    );
                RETURN_ON_ERROR(iErrCode);
                
                // Is planet on nuked list already?
                for (k = 0; k < (int) *piNumNukedPlanets; k ++) {
                    if (piNukedPlanetKey[k] == (unsigned int) vTemp.GetInteger()) {
                        break;
                    }
                }
                
                // If not, add the new planet
                if (k == (int) *piNumNukedPlanets) {    
                    piNukedPlanetKey[k] = vTemp.GetInteger();
                    (*piNumNukedPlanets) ++;
                }
                
                if (piSpaceForNukingShips[k] == 0) {
                    
                    ppiShipNukeKey[k] = new unsigned int [iNumShips * 2];
                    ppiEmpireNukeKey[k] = ppiShipNukeKey[k] + iNumShips;
                    piSpaceForNukingShips[k] = iNumShips;
                    
                } else {
                    
                    // Resize array?
                    if (piNumNukingShips[k] == piSpaceForNukingShips[k]) {
                        
                        iTemp = piSpaceForNukingShips[k] * 2;
                        
                        piTempShipKey = new unsigned int [iTemp * 2];
                        Assert(piTempShipKey);
                        piTempEmpireKey = piTempShipKey + iTemp;
                        
                        // Copy arrays
                        memcpy (piTempShipKey, ppiShipNukeKey[k], piSpaceForNukingShips[k] * sizeof (unsigned int));
                        memcpy (piTempEmpireKey, ppiEmpireNukeKey[k], piSpaceForNukingShips[k] * sizeof (unsigned int));
                        
                        delete [] (ppiShipNukeKey[k]);
                        
                        ppiShipNukeKey[k] = piTempShipKey;
                        ppiEmpireNukeKey[k] = piTempEmpireKey;
                        
                        piSpaceForNukingShips[k] = iTemp;
                    }
                }
                
                // Add new ship and empire key to the arrays
                ppiShipNukeKey [k][piNumNukingShips[k]] = piShipKey[j];
                ppiEmpireNukeKey [k][piNumNukingShips[k]] = piEmpireKey[i];
                
                piNumNukingShips[k] ++;

                // Set to standby
                iErrCode = t_pCache->WriteData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Action, 
                    STAND_BY
                    );
                
                RETURN_ON_ERROR(iErrCode);

                continue;

            }   // End if action is nuke

            if (vAction.GetInteger() >= EXPLORE_WEST) {
                // Explore, move, non "special" stuff
                continue;
            }


            /////////////////////
            // Special actions //
            /////////////////////

            // Read some ship and planet data
            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            unsigned int iPlanetKey = vTemp.GetInteger();

            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
            RETURN_ON_ERROR(iErrCode);

            GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
            
            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vShipType);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Name, &vShipName);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentBR, &vBR);
            RETURN_ON_ERROR(iErrCode);

            // Make list of watching empires
            iNumWatcherEmpires = 0;
            for (k = 0; k < iNumEmpires; k ++) {

                if (!pbAlive[k]) {
                    continue;
                }

                if (k == i) {
                    piWatcherEmpire [iNumWatcherEmpires] = k;
                    iNumWatcherEmpires ++;
                } else {

                    iErrCode = t_pCache->GetFirstKey(
                        pstrEmpireMap[k], 
                        GameEmpireMap::PlanetKey, 
                        iPlanetKey, 
                        &iKey
                        );
                    
                    if (iErrCode == ERROR_DATA_NOT_FOUND)
                    {
                        iErrCode = OK;
                    }
                    else
                    {
                        RETURN_ON_ERROR(iErrCode);

                        piWatcherEmpire [iNumWatcherEmpires] = k;
                        iNumWatcherEmpires ++;
                    }
                }
            }

            // Select by ship type
            bUpdated = false;

            switch (vShipType.GetInteger()) {
                
            case COLONY:
                
                switch (vAction.GetInteger()) {

                case COLONIZE:

                    {
                    bool bColonizeRuins = false;
                    char pszEmpireName [MAX_EMPIRE_NAME_LENGTH + 1] = "";
                    
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Pop, &vPop);
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Annihilated, &vTemp);
                    RETURN_ON_ERROR(iErrCode);
                    
                    // Make sure what we're doing is legal
                    // A planet cannot be colonized if it was annihilated or if the
                    // owner isn't the colonizer and the pop is > 0
                    if (vTemp.GetInteger() != NOT_ANNIHILATED || 
                        (vPop.GetInteger() > 0 && (unsigned int) vOwner.GetInteger() != piEmpireKey[i])
                        ) {
                        
                        // Tell owner that colonization failed in update message
                        pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                        pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG " could not colonize ";                      

                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i],
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );

                        pstrUpdateMessage[i] += "\n" END_FONT;
                        break;
                    }
                
                    // Check for a normal colonization situation
                    if (vPop.GetInteger() == 0 && vOwner.GetInteger() != (int) piEmpireKey[i]) {

                        /////////////////////
                        // Colonize planet //
                        /////////////////////

                        Variant vMin, vFuel;

                        int iMaxPop;

                        // Get planet ag, min, fuel
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Ag, &vAg);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
                        RETURN_ON_ERROR(iErrCode);

                        iMaxPop = GetMaxPop (vMin.GetInteger(), vFuel.GetInteger());

                        if (iGameClassOptions & USE_SC30_SURRENDERS) {

                            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::HomeWorld, &vTemp);
                            RETURN_ON_ERROR(iErrCode);

                            if (vTemp.GetInteger() != HOMEWORLD && vTemp.GetInteger() != NOT_HOMEWORLD)
                            {
                                // Update scores
                                iErrCode = UpdateScoresOn30StyleSurrenderColonization (
                                    piEmpireKey[i],
                                    iPlanetKey,
                                    pvEmpireName[i].GetCharPtr(),
                                    iGameClass,
                                    iGameNumber,
                                    iNewUpdateCount,
                                    pszGameClassName
                                    );

                                RETURN_ON_ERROR(iErrCode);

                                // Restore planet name
                                if (sscanf (vPlanetName.GetCharPtr(), RUINS_OF, pszEmpireName) == 1) {

                                    iErrCode = t_pCache->WriteData(
                                        strGameMap, 
                                        iPlanetKey,
                                        GameMap::Name,
                                        pszEmpireName
                                        );

                                    RETURN_ON_ERROR(iErrCode);
                                }

                                // Set not homeworld
                                iErrCode = t_pCache->WriteData(
                                    strGameMap, 
                                    iPlanetKey,
                                    GameMap::HomeWorld,
                                    NOT_HOMEWORLD
                                    );
                                
                                RETURN_ON_ERROR(iErrCode);

                                bColonizeRuins = true;
                            }
                        }

                        // If there was a previous owner, subtract the ag and a planet from him
                        if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {

                            // Find owner
                            GetEmpireIndex (k, vOwner);
                            
                            // Was the planet a homeworld?
                            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::HomeWorld, &vTemp);
                            RETURN_ON_ERROR(iErrCode);
                            
                            if (vTemp.GetInteger() == HOMEWORLD) {
                                
                                // Obliterated by a colony!
                                // No longer a HW - do this so DeleteEmpireFromGame doesn't divide the resources
                                iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::HomeWorld, NOT_HOMEWORLD);
                                RETURN_ON_ERROR(iErrCode);
                                
                                piObliterator[*piNumObliterations] = i;
                                piObliterated[*piNumObliterations] = k;

                                (*piNumObliterations) ++;
                                
                                //////////////////
                                // Obliteration //
                                //////////////////
                                
                                // Mark owner as dead
                                pbAlive[k] = false;
                                
                                // Update statistics
                                iErrCode = UpdateScoresOnNuke (piEmpireKey[i], piEmpireKey[k], 
                                    pvEmpireName[i].GetCharPtr(), pvEmpireName[k].GetCharPtr(), iGameClass, 
                                    iGameNumber, iNewUpdateCount, EMPIRE_COLONIZED, pszGameClassName);
                                
                                RETURN_ON_ERROR(iErrCode);

                                // Delete empire
                                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[k], EMPIRE_COLONIZED, &guInfo);
                                RETURN_ON_ERROR(iErrCode);
                                
                            } else {

                                Variant vMaxPop;

                                // Remove the planet from his planet count
                                iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::NumPlanets, -1);
                                RETURN_ON_ERROR(iErrCode);

                                // Remove the ag from total ag
                                piTotalAg[k] -= vAg.GetInteger();

                                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
                                RETURN_ON_ERROR(iErrCode);

                                // Remove max pop from his target pop total
                                iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::TargetPop, - vMaxPop.GetInteger());
                                RETURN_ON_ERROR(iErrCode);
                            }
                        }

                        // Set the maxpop to something appropriate
                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::MaxPop, iMaxPop);
                        RETURN_ON_ERROR(iErrCode);

                        // Make us the owner
                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Owner, (int)piEmpireKey[i]);
                        RETURN_ON_ERROR(iErrCode);

                        if (piOriginalPlanetOwner [iPlanetKey] == NO_KEY) {
                            piOriginalPlanetOwner [iPlanetKey] = vOwner.GetInteger();
                        }

                        // Increase number of empire's planets
                        iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::NumPlanets, 1);
                        RETURN_ON_ERROR(iErrCode);

                        // Get colony's capacity
                        iTemp = GetColonizePopulation (
                            gcConfig.iShipBehavior,
                            gcConfig.fColonyMultipliedDepositFactor,
                            gcConfig.fColonyExponentialDepositFactor,
                            vBR.GetFloat());

                        // Deposit population
                        if (iTemp > MAX_POPULATION) {
                            iTemp = MAX_POPULATION;
                        }

                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Pop, iTemp);
                        RETURN_ON_ERROR(iErrCode);
                        
                        // Increase empire's pop
                        iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::TotalPop, iTemp);
                        RETURN_ON_ERROR(iErrCode);

                        // Increase empire's targetpop
                        iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::TargetPop, iMaxPop);
                        RETURN_ON_ERROR(iErrCode);

                        // Increase empire's ag
                        piTotalAg[i] += vAg.GetInteger();
                        
                        // Increase empire's min
                        if (vMin.GetInteger() > iTemp) {
                            piTotalMin[i] += iTemp;
                        } else {
                            piTotalMin[i] += vMin.GetInteger();
                        }
                        
                        // Increase empire's fuel
                        if (vFuel.GetInteger() > iTemp) {
                            piTotalFuel[i] += iTemp;
                        } else {
                            piTotalFuel[i] += vFuel.GetInteger();
                        }

                        // Add to update message
                        strMessage.Clear();
                        strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[i].GetCharPtr();
                        strMessage += END_STRONG " colonized ";
                        
                        AddPlanetNameAndCoordinates (
                            strMessage,
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );
                        
                        strMessage += "\n";
                        
                        // Delete the colony
                        iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                        RETURN_ON_ERROR(iErrCode);

                        bDied = true;

                        for (k = 0; k < iNumWatcherEmpires; k ++) {
                            pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                        }

                        if (bColonizeRuins) {

                            pstrUpdateMessage[i] += "You colonized the ruins of ";
                            pstrUpdateMessage[i] += pszEmpireName;
                            pstrUpdateMessage[i] += "\n";
                        }

                    } else {

                        // Could not colonize
                        pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                        pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG " could not colonize ";                              
                        
                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i],
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );
                        
                        pstrUpdateMessage[i] += "\n" END_FONT;
                    }

                    }
                    
                    break;
                
                case DEPOSIT_POP:

                    {
                    
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Pop, &vPop);
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                    RETURN_ON_ERROR(iErrCode);
                    
                    // Make sure that we can deposit
                    if (vOwner.GetInteger() == (int) piEmpireKey[i]) {

                        iTemp = GetColonizePopulation (
                            gcConfig.iShipBehavior,
                            gcConfig.fColonyMultipliedDepositFactor,
                            gcConfig.fColonyExponentialDepositFactor,
                            vBR.GetFloat()
                            );
                        
                        //////////////////////////////////////////////////
                        // Deposit all population and delete the colony //
                        //////////////////////////////////////////////////

                        if (vPop.GetInteger() + iTemp > MAX_POPULATION) {
                            iTemp = MAX_POPULATION - vPop.GetInteger();
                        }
                        
                        iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::Pop, iTemp);
                        RETURN_ON_ERROR(iErrCode);
                            
                        // Increase empire's TotalPop
                        iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::TotalPop, iTemp);
                        RETURN_ON_ERROR(iErrCode);
                        
                        // Increase min
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                            
                        if (vTemp.GetInteger() > vPop.GetInteger()) {

                            if (vTemp.GetInteger() < vPop.GetInteger() + iTemp) {
                                piTotalMin[i] += (vTemp.GetInteger() - vPop.GetInteger());
                            } else {
                                piTotalMin[i] += iTemp;
                            }
                        }
                            
                        // Increase fuel
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                            
                        if (vTemp.GetInteger() > vPop.GetInteger()) {
                            if (vTemp.GetInteger() < vPop.GetInteger() + iTemp) {
                                piTotalFuel[i] += (vTemp.GetInteger() - vPop.GetInteger());
                            } else {
                                piTotalFuel[i] += iTemp;
                            }
                        }
                            
                        // Add to update message
                        strMessage.Clear();
                        strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[i].GetCharPtr();
                        strMessage += END_STRONG " settled ";
                        
                        AddPlanetNameAndCoordinates (
                            strMessage,
                            vPlanetName.GetCharPtr(),
                            iX,
                            iY
                            );
                        
                        strMessage += "\n";
                        
                        for (k = 0; k < iNumWatcherEmpires; k ++) {
                            pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                        }
                        
                        // Melt the ship down
                        iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                        RETURN_ON_ERROR(iErrCode);
                        
                        bDied = true;

                    } else {
                    
                        ////////////////////////
                        // Deposit pop failed //
                        ////////////////////////

                        Assert(vAction.GetInteger() != COLONIZE);

                        // Add to update message
                        pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                        pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG " could not settle ";
                        
                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i],
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );
                        
                        pstrUpdateMessage[i] += "\n" END_FONT;
                    }
                        
                    }   // Scope
                    break;
                    
                default:
                    Assert(false);
                }

                bUpdated = true;
                
                break;
                                
            case TERRAFORMER:
                    
                if (vAction.GetInteger() != TERRAFORM) {
                    
                    if (vAction.GetInteger() == TERRAFORM_AND_DISMANTLE) {
                        bDismantle = true;
                    } else {
                        Assert(false);
                        continue;
                    }
                }
                
                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Ag, &vAg);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
                RETURN_ON_ERROR(iErrCode);

                if (vFuel.GetInteger() > vMin.GetInteger()) {
                    iTemp = vFuel.GetInteger() - vAg.GetInteger();
                } else {
                    iTemp = vMin.GetInteger() - vAg.GetInteger();
                }

                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                RETURN_ON_ERROR(iErrCode);
                
                if (iTemp > 0 &&
                    (
                    !(gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) ||
                    vOwner.GetInteger() == (int) piEmpireKey[i]
                    )
                    &&
                    (
                    !(gcConfig.iShipBehavior & TERRAFORMER_DISABLE_MULTIPLE) ||
                    !pbTerraformed[iPlanetKey]
                    )
                    ) {
                    
                    iTerraform = GetTerraformerAg (gcConfig.fTerraformerPlowFactor, vBR.GetFloat());
                    
                    if (iTerraform <= iTemp) {
                        
                        ////////////////////
                        // Full terraform //
                        ////////////////////

                        if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {
                            
                            // Increase empire's ag
                            GetEmpireIndex (k, vOwner);
                            piTotalAg[k] += iTerraform;
                        }
                        
                        // Increase planet ag
                        iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::Ag, iTerraform);
                        RETURN_ON_ERROR(iErrCode);
                        
                        // Add to update message
                        strMessage.Clear();
                        strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[i].GetCharPtr();
                        strMessage += END_STRONG " destroyed itself terraforming ";
                        
                        AddPlanetNameAndCoordinates (
                            strMessage,
                            vPlanetName.GetCharPtr(),
                            iX,
                            iY
                            );
                        
                        strMessage += "\n";

                        for (k = 0; k < iNumWatcherEmpires; k ++) {
                            pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                        }
                        
                        // Delete ship
                        iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                        RETURN_ON_ERROR(iErrCode);

                        bDied = true;

                    } else {

                        ///////////////////////
                        // Partial terraform //
                        ///////////////////////

                        if (vOwner != SYSTEM && vOwner != INDEPENDENT) {
                            
                            // Increase empire's ag
                            GetEmpireIndex (k, vOwner);
                            piTotalAg[k] += iTemp;
                        }
                        
                        // Increase planet ag
                        iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::Ag, iTemp);
                        RETURN_ON_ERROR(iErrCode);
                        
                        // Add to update message
                        strMessage.Clear();
                        strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[i].GetCharPtr();
                        strMessage += END_STRONG " terraformed ";
                        
                        AddPlanetNameAndCoordinates (
                            strMessage,
                            vPlanetName.GetCharPtr(),
                            iX,
                            iY
                            );
                        
                        strMessage += "\n";
                        
                        for (k = 0; k < iNumWatcherEmpires; k ++) {
                            pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                        }
                        
                        // Update ship's Max BR
                        iErrCode = ChangeShipTypeOrMaxBR (
                            pstrEmpireShips[i],
                            pstrEmpireData[i],
                            piEmpireKey[i],
                            piShipKey[j], 
                            TERRAFORMER, 
                            TERRAFORMER,
                            - vBR.GetFloat() * (float) iTemp / iTerraform
                            );

                        RETURN_ON_ERROR(iErrCode);
                    }

                    // Set already terraformed if necessary
                    if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_MULTIPLE) {
                        Assert(iPlanetKey < iNumPlanets); // Should never fire
                        pbTerraformed[iPlanetKey] = true;
                    }

                } else {
                
                    //////////////////////
                    // Failed terraform //
                    //////////////////////

                    // Add to update message
                    pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                    pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                    pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                    pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                    pstrUpdateMessage[i] += END_STRONG " failed to terraform ";
                    
                    AddPlanetNameAndCoordinates (
                        pstrUpdateMessage[i],
                        vPlanetName.GetCharPtr(), 
                        iX, 
                        iY
                        );
                    
                    pstrUpdateMessage[i] += "\n" END_FONT;
                }

                bUpdated = true;
                break;

            case TROOPSHIP:

                if (vAction.GetInteger() != INVADE) {
                    
                    if (vAction.GetInteger() == INVADE_AND_DISMANTLE) {
                        bDismantle = true;
                    } else {
                        Assert(false);
                        continue;
                    }
                }

                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                RETURN_ON_ERROR(iErrCode);

                if (vOwner.GetInteger() == (int) piEmpireKey[i] || vOwner.GetInteger() == SYSTEM) {
                
                    bActionFlag = false;
                
                } else {

                    bActionFlag = true;
                    
                    if (vOwner.GetInteger() != INDEPENDENT) {
                        
                        iErrCode = t_pCache->GetFirstKey(
                            pstrEmpireDip[i], 
                            GameEmpireDiplomacy::ReferenceEmpireKey, 
                            vOwner, 
                            &iKey
                            );
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->ReadData(
                            pstrEmpireDip[i], 
                            iKey, 
                            GameEmpireDiplomacy::CurrentStatus, 
                            &vTemp
                            );
                        RETURN_ON_ERROR(iErrCode);
                        
                        if (vTemp.GetInteger() != WAR) {
                            bActionFlag = false;
                        }
                    }
                }

                if (!bActionFlag) {

                    // Couldn't invade
                    pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                    pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                    pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                    pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                    pstrUpdateMessage[i] += END_STRONG " could not invade ";
                    
                    AddPlanetNameAndCoordinates (
                        pstrUpdateMessage[i],
                        vPlanetName.GetCharPtr(), 
                        iX, 
                        iY
                        );

                    pstrUpdateMessage[i] += "\n" END_FONT;

                } else {

                    int iMaxPop;
                    
                    // Attempt invasion
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Pop, &vPop);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
                    RETURN_ON_ERROR(iErrCode);

                    iMaxPop = GetMaxPop (vMin.GetInteger(), vFuel.GetInteger());
                    
                    iInvadePop = GetTroopshipPop (
                        gcConfig.fTroopshipInvasionFactor,
                        vBR.GetFloat()
                        );
                    
                    if (iInvadePop < vPop.GetInteger()) {
                        
                        /////////////////////
                        // Invasion failed //
                        /////////////////////

                        // Reduce pop
                        iTemp = GetTroopshipFailurePopDecrement (
                            gcConfig.fTroopshipFailureFactor,
                            vBR.GetFloat()
                            );
                        
                        iErrCode = t_pCache->Increment (strGameMap, iPlanetKey, GameMap::Pop, iTemp);
                        RETURN_ON_ERROR(iErrCode);
                        
                        // Reduce owner's total pop
                        if (vOwner.GetInteger() != INDEPENDENT) {
                            
                            GetEmpireIndex (k, vOwner);
                            
                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::TotalPop, iTemp);
                            RETURN_ON_ERROR(iErrCode);
                            
                            // Reduce owner's resources and econ
                            iDiff = min (vPop.GetInteger() + iTemp, vMin.GetInteger()) - min (vPop.GetInteger(), vMin.GetInteger());
                            piTotalMin[k] += iDiff;
                            
                            iDiff = min (vPop.GetInteger() + iTemp, vFuel.GetInteger()) - min (vPop.GetInteger(), vFuel.GetInteger());
                            piTotalFuel[k] += iDiff;
                        }
                        
                        // Update message
                        strMessage.Clear();
                        strMessage += BEGIN_BAD_FONT(i);
                        strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[i].GetCharPtr();
                        strMessage += END_STRONG " was destroyed attempting to invade ";
                        
                        AddPlanetNameAndCoordinates (
                            strMessage,
                            vPlanetName.GetCharPtr(),
                            iX,
                            iY
                            );
                        
                        strMessage += "\n" END_FONT;
                        
                        for (k = 0; k < iNumWatcherEmpires; k ++) {
                            pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                        }
                        
                        // Delete ship
                        iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                        RETURN_ON_ERROR(iErrCode);
                        
                        bDied = true;
                        
                    } else {
                       
                        ////////////////////////
                        // Invasion succeeded //
                        ////////////////////////
                        
                        // Change owner
                        iErrCode = t_pCache->WriteData(
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::Owner, 
                            (int)piEmpireKey[i]
                            );

                        RETURN_ON_ERROR(iErrCode);

                        if (piOriginalPlanetOwner [iPlanetKey] == NO_KEY) {
                            piOriginalPlanetOwner [iPlanetKey] = vOwner.GetInteger();
                        }

                        // Reduce planet pop
                        int iNewPop = - GetTroopshipSuccessPopDecrement (
                            gcConfig.fTroopshipSuccessFactor,
                            vPop.GetInteger()
                            );

                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Pop, iNewPop);
                        RETURN_ON_ERROR(iErrCode);

                        // Save old max pop
                        Variant vOldMaxPop;
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::MaxPop, &vOldMaxPop);
                        RETURN_ON_ERROR(iErrCode);

                        // Write new max pop
                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::MaxPop, iMaxPop);
                        RETURN_ON_ERROR(iErrCode);

                        // Increase empire's maxpop
                        iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::TargetPop, iMaxPop);
                        RETURN_ON_ERROR(iErrCode);

                        // Get planet resources
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Ag, &vAg);
                        RETURN_ON_ERROR(iErrCode);

                        // Increase new owner's total pop and change number of planets
                        iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::TotalPop, iNewPop);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = t_pCache->Increment (pstrEmpireData[i], GameEmpireData::NumPlanets, 1);
                        RETURN_ON_ERROR(iErrCode);

                        // Increase new owner's resources and econ                      
                        piTotalAg[i] += vAg;
                        
                        if (vMin < iNewPop) {
                            piTotalMin[i] += vMin.GetInteger();
                        } else {
                            piTotalMin[i] += iNewPop;
                        }
                        
                        if (vFuel < iNewPop) {
                            piTotalFuel[i] += vFuel.GetInteger();
                        } else {
                            piTotalFuel[i] += iNewPop;
                        }
                        
                        if (vOwner.GetInteger() != INDEPENDENT) {
                            
                            GetEmpireIndex (k, vOwner);
                            
                            // Reduce former owner's total pop, targetpop and change number of planets
                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::TotalPop, - vPop.GetInteger());
                            RETURN_ON_ERROR(iErrCode);
                            
                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::NumPlanets, -1);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::TargetPop, - vOldMaxPop.GetInteger());
                            RETURN_ON_ERROR(iErrCode);
                            
                            // Reduce owner's resources and econ
                            piTotalAg[k] -= vAg.GetInteger();
                            
                            if (vMin.GetInteger() < vPop.GetInteger()) {
                                piTotalMin[k] -= vMin.GetInteger();
                            } else {
                                piTotalMin[k] -= vPop.GetInteger();
                            }
                            
                            if (vFuel.GetInteger() < vPop.GetInteger()) {
                                piTotalFuel[k] -= vFuel.GetInteger();
                            } else {
                                piTotalFuel[k] -= vPop.GetInteger();
                            }
                            
                            // Is the invaded planet a HW?
                            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::HomeWorld, &vTemp);
                            RETURN_ON_ERROR(iErrCode);
                            
                            if (vTemp.GetInteger() == HOMEWORLD) {
                                
                                // No longer a HW - do this so DeleteEmpireFromGame doesn't divide the resources
                                iErrCode = t_pCache->WriteData(
                                    strGameMap, 
                                    iPlanetKey, 
                                    GameMap::HomeWorld, 
                                    NOT_HOMEWORLD
                                    );
                                RETURN_ON_ERROR(iErrCode);
                                
                                piObliterator[*piNumObliterations] = i;
                                piObliterated[*piNumObliterations] = k;
                                (*piNumObliterations) ++;
                                
                                //////////////////
                                // Obliteration //
                                //////////////////
                                
                                // Mark owner as dead
                                pbAlive[k] = false;
                                
                                // Update statistics
                                iErrCode = UpdateScoresOnNuke (piEmpireKey[i], piEmpireKey[k], 
                                    pvEmpireName[i].GetCharPtr(), pvEmpireName[k].GetCharPtr(), iGameClass, 
                                    iGameNumber, iNewUpdateCount, EMPIRE_INVADED, pszGameClassName);

                                RETURN_ON_ERROR(iErrCode);

                                // Delete empire's tables
                                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[k], EMPIRE_INVADED, &guInfo);
                                RETURN_ON_ERROR(iErrCode);
                            }
                        }
                        
                        // Handle ship's BR
                        if (iInvadePop == vPop.GetInteger()) {
                            
                            // Delete ship
                            iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                            RETURN_ON_ERROR(iErrCode);
                            
                            bDied = true;
                            
                            strMessage.Clear();
                            strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                            strMessage += " of " BEGIN_STRONG;
                            strMessage += pvEmpireName[i].GetCharPtr();
                            strMessage += END_STRONG " destroyed itself successfully invading ";
                            
                            AddPlanetNameAndCoordinates (
                                strMessage,
                                vPlanetName.GetCharPtr(),
                                iX,
                                iY
                                );
                            
                            strMessage += "\n";

                            for (k = 0; k < iNumWatcherEmpires; k ++) {
                                pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                            }
                            
                        } else {
                            
                            // Reduce BR and MaxBR
                            iErrCode = ChangeShipTypeOrMaxBR (
                                pstrEmpireShips[i],
                                pstrEmpireData[i],
                                piEmpireKey[i],
                                piShipKey[j], 
                                TROOPSHIP, 
                                TROOPSHIP,
                                - vBR.GetFloat() * (float) vPop.GetInteger() / iInvadePop
                                );

                            RETURN_ON_ERROR(iErrCode);
                            
                            strMessage.Clear();
                            strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                            strMessage += " of " BEGIN_STRONG;
                            strMessage += pvEmpireName[i].GetCharPtr();
                            strMessage += END_STRONG " successfully invaded ";
                            
                            AddPlanetNameAndCoordinates (
                                strMessage,
                                vPlanetName.GetCharPtr(), 
                                iX, 
                                iY
                                );
                            
                            strMessage += "\n";

                            for (k = 0; k < iNumWatcherEmpires; k ++) {
                                pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                            }
                        }
                    }
                }

                bUpdated = true;
                break;
        
            case DOOMSDAY:
                    
                if (vAction.GetInteger() != ANNIHILATE) {
                    Assert(false);
                    continue;
                }

                bActionFlag = true;

                // Make sure planet can be annihilated:
                // either an enemy's planet or an uncolonized planet or your own planet and not a HW
                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                RETURN_ON_ERROR(iErrCode);
                
                if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {

                    if (vOwner.GetInteger() == (int) piEmpireKey[i]) {
                        
                        if (iGameClassOptions & DISABLE_SUICIDAL_DOOMSDAYS) {
                            
                            bActionFlag = false;
                        
                        } else {
                        
                            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::HomeWorld, &vTemp);
                            RETURN_ON_ERROR(iErrCode);
                            
                            if (vTemp.GetInteger() == HOMEWORLD) {
                                bActionFlag = false;
                            }
                        }

                    } else {

                        iErrCode = t_pCache->GetFirstKey(pstrEmpireDip[i], GameEmpireDiplomacy::ReferenceEmpireKey, vOwner, &iKey);
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->ReadData(pstrEmpireDip[i], iKey, GameEmpireDiplomacy::CurrentStatus, &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                        
                        if (vTemp.GetInteger() != WAR) {
                            
                            if (!(iGameClassOptions & USE_UNFRIENDLY_DOOMSDAYS)) {
                                
                                bActionFlag = false;
                                
                            } else {
                                
                                iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::HomeWorld, &vTemp);
                                RETURN_ON_ERROR(iErrCode);
                                
                                if (vTemp.GetInteger() == HOMEWORLD) {
                                    bActionFlag = false;
                                }
                            }
                        }
                    }
                }
                
                if (bActionFlag) {

                    if (vOwner.GetInteger() != SYSTEM) {
                        
                        // Change owner to SYSTEM
                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Owner, (int)SYSTEM);
                        RETURN_ON_ERROR(iErrCode);

                        if (piOriginalPlanetOwner [iPlanetKey] == NO_KEY) {
                            piOriginalPlanetOwner [iPlanetKey] = vOwner.GetInteger();
                        }
                        
                        // Reduce planet pop
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Pop, &vPop);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Pop, 0);
                        RETURN_ON_ERROR(iErrCode);

                        if (vOwner.GetInteger() != INDEPENDENT) {

                            Variant vMaxPop;
                            
                            // Reduce owner's max pop
                            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
                            RETURN_ON_ERROR(iErrCode);

                            // Find owner
                            GetEmpireIndex (k, vOwner);
                            
                            // Read planet resources
                            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Ag, &vAg);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Minerals, &vMin);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Fuel, &vFuel);
                            RETURN_ON_ERROR(iErrCode);

                            // Reduce owner's total pop, targetpop and change number of planets
                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::TotalPop, -vPop);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::NumPlanets, -1);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::TargetPop, - vMaxPop.GetInteger());
                            RETURN_ON_ERROR(iErrCode);

                            // Reduce owner's resources and econ
                            piTotalAg[k] -= vAg.GetInteger();
                            
                            if (vMin.GetInteger() < vPop.GetInteger()) {
                                piTotalMin[k] -= vMin.GetInteger();
                            } else {
                                piTotalMin[k] -= vPop.GetInteger();
                            }
                            
                            if (vFuel.GetInteger() < vPop.GetInteger()) {
                                piTotalFuel[k] -= vFuel.GetInteger();
                            } else {
                                piTotalFuel[k] -= vPop.GetInteger();
                            }
                            
                            // Did we obliterate the poor guy's HW?
                            iErrCode = t_pCache->ReadData(
                                strGameMap, 
                                iPlanetKey, 
                                GameMap::HomeWorld, 
                                &vTemp
                                );

                            RETURN_ON_ERROR(iErrCode);
                            
                            if (vTemp.GetInteger() == HOMEWORLD) {

                                // No longer a homeworld
                                iErrCode = t_pCache->WriteData(
                                    strGameMap, 
                                    iPlanetKey, 
                                    GameMap::HomeWorld, 
                                    NOT_HOMEWORLD
                                    );
                                RETURN_ON_ERROR(iErrCode);
                                
                                piObliterator[*piNumObliterations] = i;
                                piObliterated[*piNumObliterations] = k;
                                
                                (*piNumObliterations) ++;
                                
                                //////////////////
                                // Obliteration //
                                //////////////////
                                
                                // Mark empire as dead
                                pbAlive[k] = false;
                                
                                // Update empire's statistics
                                iErrCode = UpdateScoresOnNuke (piEmpireKey[i], piEmpireKey[k], 
                                    pvEmpireName[i].GetCharPtr(), pvEmpireName[k].GetCharPtr(), iGameClass, 
                                    iGameNumber, iNewUpdateCount, EMPIRE_ANNIHILATED, pszGameClassName);

                                RETURN_ON_ERROR(iErrCode);

                                // Delete empire's tables
                                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[k], EMPIRE_ANNIHILATED, &guInfo);
                                RETURN_ON_ERROR(iErrCode);
                            }
                        }
                    }

                    // Take note for jumpgate calculations
                    if (piOriginalNumObliterations [iPlanetKey] == ANNIHILATED_UNKNOWN) {
                        
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Annihilated, &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                        
                        piOriginalNumObliterations [iPlanetKey] = vTemp.GetInteger();
                    }

                    // Reduce ag to zero
                    iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Ag, 0);
                    RETURN_ON_ERROR(iErrCode);

                    // Add to update messages
                    strMessage.Clear();
                    strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                    strMessage += " of " BEGIN_STRONG;
                    strMessage += pvEmpireName[i].GetCharPtr();
                    
                    if (iGameClassOptions & USE_CLASSIC_DOOMSDAYS) {

                        strMessage += END_STRONG " permanently annihilated ";
                        
                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Minerals, 0);
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Fuel, 0);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = t_pCache->WriteData(strGameMap, iPlanetKey, GameMap::Annihilated, (int)ANNIHILATED_FOREVER);

                    } else {

                        strMessage += END_STRONG " annihilated ";
                    
                        iErrCode = t_pCache->Increment (
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::Annihilated, 
                            GetDoomsdayUpdates (gcConfig.fDoomsdayAnnihilationFactor, vBR.GetFloat())
                            );
                    }

                    RETURN_ON_ERROR(iErrCode);
                    
                    AddPlanetNameAndCoordinates (
                        strMessage,
                        vPlanetName.GetCharPtr(), 
                        iX, 
                        iY
                        );
                    
                    strMessage += "\n";

                    for (k = 0; k < iNumWatcherEmpires; k ++) {
                        pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                    }
                    
                    // Delete ship
                    iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                    RETURN_ON_ERROR(iErrCode);

                    bDied = true;
                    
                } else {

                    //////////////////////////
                    // Failed to annihilate //
                    //////////////////////////

                    // Add to update message
                    pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                    pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                    pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                    pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                    pstrUpdateMessage[i] += END_STRONG " failed to annihilate ";
                    
                    AddPlanetNameAndCoordinates (
                        pstrUpdateMessage[i],
                        vPlanetName.GetCharPtr(),
                        iX,
                        iY
                        );
                    
                    pstrUpdateMessage[i] += "\n" END_FONT;
                }
                
                bUpdated = true;
                break;
                    
            case ENGINEER:
                    
                bActionFlag = true;

                switch (vAction.GetInteger()) {
                    
                case OPEN_LINK_NORTH:
                case OPEN_LINK_EAST:
                case OPEN_LINK_SOUTH:
                case OPEN_LINK_WEST:
                    
                    iTemp = OPEN_LINK_NORTH - vAction.GetInteger();
                    
                    // Make sure ship still has enough BR               
                    if (vBR.GetFloat() < gcConfig.fEngineerLinkCost) {
                        bActionFlag = false;
                    } else {
                        
                        // Make sure link is still closed
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Link, &vTemp);
                        RETURN_ON_ERROR(iErrCode);
                        
                        if (vTemp.GetInteger() & LINK_X[iTemp]) {
                            bActionFlag = false;
                        }
                    }
                    
                    // Get other planet's data
                    iErrCode = t_pCache->ReadData(
                        strGameMap, 
                        iPlanetKey, 
                        GameMap::ColumnNames[GameMap::iNorthPlanetKey + iTemp],
                        &vLinkPlanetKey
                        );
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(strGameMap, vLinkPlanetKey.GetInteger(), GameMap::Name, &vLinkPlanetName);
                    RETURN_ON_ERROR(iErrCode);

                    AdvanceCoordinates (iX, iY, &iLinkX, &iLinkY, iTemp);

                    if (!bActionFlag) {

                        bTarget = false;

                        // Can the empire see the target planet?
                        iErrCode = t_pCache->GetFirstKey(
                            pstrEmpireMap[i], 
                            GameEmpireMap::PlanetKey, 
                            vLinkPlanetKey.GetInteger(), 
                            &iKey
                            );

                        if (iErrCode == ERROR_DATA_NOT_FOUND)
                        {
                            iErrCode = OK;
                        }
                        else
                        {
                            RETURN_ON_ERROR(iErrCode);
                            bTarget = true;
                        }
                        
                        // Notify owner of failure
                        pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                        pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG " failed to open the link from ";
                        
                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i], 
                            vPlanetName.GetCharPtr(),
                            iX, 
                            iY
                            );
                        
                        pstrUpdateMessage[i] += " to ";
                        
                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i],
                            (bTarget ? vLinkPlanetName.GetCharPtr() : "Unknown"),
                            iLinkX, 
                            iLinkY
                            );
                        
                        pstrUpdateMessage[i] += "\n" END_FONT;
                        
                    } else {
                        
                        // Open link
                        iErrCode = t_pCache->WriteOr(
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::Link, 
                            LINK_X[iTemp]
                            );
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = t_pCache->WriteOr(
                            strGameMap,
                            vLinkPlanetKey.GetInteger(), 
                            GameMap::Link,
                            OPPOSITE_LINK_X[iTemp]
                            );
                        RETURN_ON_ERROR(iErrCode);
                        
                        // Calculate damage to engineer
                        if (vBR.GetFloat() - gcConfig.fEngineerLinkCost > FLOAT_PROXIMITY_TOLERANCE) {
                            
                            // Reduce ship BR
                            iErrCode = ChangeShipTypeOrMaxBR (
                                pstrEmpireShips[i], 
                                pstrEmpireData[i],
                                piEmpireKey[i],
                                piShipKey[j], 
                                ENGINEER, 
                                ENGINEER,
                                - gcConfig.fEngineerLinkCost
                                );
                            RETURN_ON_ERROR(iErrCode);

                        } else {
                            
                            // Delete ship
                            iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                            RETURN_ON_ERROR(iErrCode);

                            bDied = true;
                        }
                    
                        // Loop through all empires and choose an update message in the following way:
                        // 1) If the empire can see both planets, name the ship and both planets
                        // 2) If the empire can see only the original planet, name the ship and the original 
                        //    planet, but not the target planet
                        // 3) If the empire can see only the target planet, name the target planet only
                        
                        strOriginal.Clear();
                        AddPlanetNameAndCoordinates (
                            strOriginal, 
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );
                        
                        strMessage.Clear();
                        AddPlanetNameAndCoordinates (
                            strMessage, 
                            vLinkPlanetName.GetCharPtr(), 
                            iLinkX, 
                            iLinkY
                            );
                        
                        for (k = 0; k < iNumEmpires; k ++) {
                            
                            if (!pbAlive[k]) {
                                continue;
                            }
                            
                            // Can the empire see the original planet?
                            if (k == i) {
                                bOriginal = true;
                            } else {
                                
                                bOriginal = false;
                                for (l = 0; l < iNumWatcherEmpires; l ++) {
                                    if (piWatcherEmpire[l] == k) {
                                        bOriginal = true;
                                        break;
                                    }
                                }
                            }
                            
                            // Can the empire see the target planet?
                            iErrCode = t_pCache->GetFirstKey(
                                pstrEmpireMap[k], 
                                GameEmpireMap::PlanetKey, 
                                vLinkPlanetKey.GetInteger(), 
                                &iKey
                                );

                            if (iErrCode == ERROR_DATA_NOT_FOUND)
                            {
                                iErrCode = OK;
                                bTarget = false;
                            }
                            else
                            {
                                RETURN_ON_ERROR(iErrCode);
                                bTarget = true;
                            }

                            if (bOriginal) {
                                
                                pstrUpdateMessage[k].AppendHtml (vShipName.GetCharPtr(), 0, false);
                                pstrUpdateMessage[k] += " of " BEGIN_STRONG;
                                pstrUpdateMessage[k] += pvEmpireName[i].GetCharPtr();
                                pstrUpdateMessage[k] += 
                                    (bDied ? 
                                    END_STRONG " destroyed itself opening the link from " : 
                                END_STRONG " opened the link from ");
                                
                                pstrUpdateMessage[k] += strOriginal;
                                pstrUpdateMessage[k] += " to ";
                                
                                if (bTarget) {
                                    pstrUpdateMessage[k] += strMessage + "\n";
                                } else {
                                    
                                    AddPlanetNameAndCoordinates (
                                        pstrUpdateMessage[k], 
                                        "Unknown", 
                                        iLinkX, 
                                        iLinkY
                                        );
                                    pstrUpdateMessage[k] += "\n";
                                }
                                
                            } else {
                                
                                if (bTarget) {
                                    
                                    pstrUpdateMessage[k] += "The link from ";
                                    pstrUpdateMessage[k] += strMessage;
                                    pstrUpdateMessage[k] += " to ";
                                    
                                    AddPlanetNameAndCoordinates (
                                        pstrUpdateMessage[k], 
                                        "Unknown", 
                                        iX, 
                                        iY
                                        );
                                    
                                    pstrUpdateMessage[k] += " was opened\n";
                                }
                            }
                        }   // End empire update message loop
                    }
                    
                    break;
                    
                case CLOSE_LINK_NORTH:
                case CLOSE_LINK_EAST:
                case CLOSE_LINK_SOUTH:
                case CLOSE_LINK_WEST:
                    
                    iTemp = CLOSE_LINK_NORTH - vAction.GetInteger();
                    
                    // Make sure ship still has enough BR               
                    if (vBR.GetFloat() < gcConfig.fEngineerLinkCost) {
                        bActionFlag = false;
                    } else {
                        
                        // Make sure link is still open
                        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Link, &vTemp);
                        RETURN_ON_ERROR(iErrCode);

                        if (!(vTemp.GetInteger() & LINK_X[iTemp])) {
                            bActionFlag = false;
                        }
                    }
                    
                    // Get other planet's data
                    iErrCode = t_pCache->ReadData(
                        strGameMap, 
                        iPlanetKey, 
                        GameMap::ColumnNames[GameMap::iNorthPlanetKey + iTemp],
                        &vLinkPlanetKey
                        );
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(
                        strGameMap, 
                        vLinkPlanetKey.GetInteger(), 
                        GameMap::Name, 
                        &vLinkPlanetName
                        );
                    RETURN_ON_ERROR(iErrCode);
                    
                    AdvanceCoordinates (iX, iY, &iLinkX, &iLinkY, iTemp);

                    if (!bActionFlag) {

                        bTarget = false;

                        // Can the empire see the target planet?
                        iErrCode = t_pCache->GetFirstKey(
                            pstrEmpireMap[i], 
                            GameEmpireMap::PlanetKey, 
                            vLinkPlanetKey.GetInteger(), 
                            &iKey
                            );

                        if (iErrCode == ERROR_DATA_NOT_FOUND)
                        {
                            iErrCode = OK;
                        }
                        else
                        {
                            RETURN_ON_ERROR(iErrCode);
                            bTarget = true;
                        }

                        // Notify owner of failure
                        pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                        pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG " failed to close the link from ";
                        
                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i],
                            vPlanetName.GetCharPtr(),
                            iX,
                            iY
                            );
                        
                        pstrUpdateMessage[i] += " to ";

                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i],
                            (bTarget ? vLinkPlanetName.GetCharPtr() : "Unknown"),
                            iLinkX, 
                            iLinkY
                            );
                        
                        pstrUpdateMessage[i] += "\n" END_FONT;
                        
                    } else {
                        
                        // Close link
                        iErrCode = t_pCache->WriteAnd(
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::Link,
                            ~LINK_X[iTemp]
                            );
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = t_pCache->WriteAnd(
                            strGameMap, 
                            vLinkPlanetKey.GetInteger(), 
                            GameMap::Link,
                            ~OPPOSITE_LINK_X[iTemp]
                            );
                        RETURN_ON_ERROR(iErrCode);
                        
                        // Calculate damage to engineer
                        if (vBR.GetFloat() - gcConfig.fEngineerLinkCost > FLOAT_PROXIMITY_TOLERANCE) {
                            
                            // Reduce ship BR
                            iErrCode = ChangeShipTypeOrMaxBR (
                                pstrEmpireShips[i],
                                pstrEmpireData[i],
                                piEmpireKey[i],
                                piShipKey[j],
                                ENGINEER, 
                                ENGINEER,
                                - gcConfig.fEngineerLinkCost
                                );

                            RETURN_ON_ERROR(iErrCode);
                            
                        } else {
                            
                            // Delete ship
                            iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                            RETURN_ON_ERROR(iErrCode);
                            
                            bDied = true;                           
                        }
                        
                        // Loop through all empires and choose an update message in the following way:
                        // 1) If the empire can see both planets, name the ship and both planets
                        // 2) If the empire can see only the original planet, name the ship and the original 
                        //    planet, but not the target planet
                        // 3) If the empire can see only the target planet, name the target planet only
                        
                        strOriginal.Clear();
                        AddPlanetNameAndCoordinates (
                            strOriginal, 
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );
                        
                        strMessage.Clear();
                        AddPlanetNameAndCoordinates (
                            strMessage, 
                            vLinkPlanetName.GetCharPtr(), 
                            iLinkX, 
                            iLinkY
                            );
                        
                        for (k = 0; k < iNumEmpires; k ++) {
                            if (!pbAlive[k]) {
                                continue;
                            }
                            
                            // Can the empire see the original planet?
                            bOriginal = false;
                            if (k == i) {
                                bOriginal = true;
                            } else {
                                
                                for (l = 0; l < iNumWatcherEmpires; l ++) {
                                    if (piWatcherEmpire[l] == k) {
                                        bOriginal = true;
                                        break;
                                    }
                                }
                            }
                            
                            // Can the empire see the target planet?
                            iErrCode = t_pCache->GetFirstKey(
                                pstrEmpireMap[k], 
                                GameEmpireMap::PlanetKey, 
                                vLinkPlanetKey.GetInteger(), 
                                &iKey
                                );

                            if (iErrCode == ERROR_DATA_NOT_FOUND)
                            {
                                bTarget = false;
                                iErrCode = OK;
                            }
                            else
                            {
                                RETURN_ON_ERROR(iErrCode);
                                bTarget = true;
                            }

                            if (bOriginal) {
                                
                                pstrUpdateMessage[k].AppendHtml (vShipName.GetCharPtr(), 0, false);
                                pstrUpdateMessage[k] += " of " BEGIN_STRONG;
                                pstrUpdateMessage[k] += pvEmpireName[i].GetCharPtr();
                                pstrUpdateMessage[k] += 
                                    (bDied ? 
                                    END_STRONG " destroyed itself closing the link from " : 
                                END_STRONG " closed the link from ");
                                
                                pstrUpdateMessage[k] += strOriginal;
                                pstrUpdateMessage[k] += " to ";
                                
                                if (bTarget) {
                                    pstrUpdateMessage[k] += strMessage + "\n";
                                } else {
                                    
                                    AddPlanetNameAndCoordinates (
                                        pstrUpdateMessage[k], 
                                        "Unknown",
                                        iLinkX, 
                                        iLinkY
                                        );
                                    
                                    pstrUpdateMessage[k] += "\n";
                                }
                                
                            } else {
                                
                                if (bTarget) {
                                    pstrUpdateMessage[k] += "The link from ";
                                    pstrUpdateMessage[k] += strMessage;
                                    pstrUpdateMessage[k] += " to ";
                                    
                                    AddPlanetNameAndCoordinates (
                                        pstrUpdateMessage[k], 
                                        "Unknown",
                                        iX, 
                                        iY
                                        );
                                    
                                    pstrUpdateMessage[k] += " was closed\n";
                                }
                            }
                        }   // End empire update message loop
                    }

                    break;
                    
                default:
                    Assert(false);
                        
                }   // End engineer action switch

                bUpdated = true;
                break;

            case BUILDER:

                switch (vAction.GetInteger()) {

                case CREATE_PLANET_NORTH:
                case CREATE_PLANET_EAST:
                case CREATE_PLANET_SOUTH:
                case CREATE_PLANET_WEST:

                    Variant vEmptyKey;
                    int iDirection = CREATE_PLANET_NORTH - vAction.GetInteger();

                    // Make sure no planet exists
                    iErrCode = t_pCache->ReadData(
                        strGameMap, 
                        iPlanetKey, 
                        GameMap::ColumnNames[GameMap::iNorthPlanetKey + iDirection],
                        &vEmptyKey
                        );

                    RETURN_ON_ERROR(iErrCode);

                    if (vEmptyKey.GetInteger() != NO_KEY || vBR.GetFloat() < gcConfig.fBuilderMinBR) {

                        // Notify owner of failure
                        pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                        pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                        pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[i] += END_STRONG " failed to create a new planet ";
                        pstrUpdateMessage[i] += CARDINAL_STRING [iDirection];
                        pstrUpdateMessage[i] += " of ";
                        
                        AddPlanetNameAndCoordinates (
                            pstrUpdateMessage[i],
                            vPlanetName.GetCharPtr(),
                            iX,
                            iY
                            );

                        pstrUpdateMessage[i] += "\n" END_FONT;

                    } else {

                        // Create the planet
                        iErrCode = CreateNewPlanetFromBuilder (
                            gcConfig,
                            iGameClass, 
                            iGameNumber, 
                            piEmpireKey[i],
                            vBR.GetFloat(),
                            iPlanetKey,
                            iX,
                            iY,
                            iDirection, 
                            strGameMap, 
                            strGameData, 
                            pstrEmpireMap[i],
                            pstrEmpireDip[i],
                            piEmpireKey, 
                            iNumEmpires, 
                            pstrEmpireMap, 
                            pstrEmpireDip,
                            pstrEmpireData
                            );

                        RETURN_ON_ERROR(iErrCode);

                        // Add to update messages
                        strMessage.Clear();
                        strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[i].GetCharPtr();
                        strMessage += END_STRONG " destroyed itself creating a new planet ";
                        strMessage += CARDINAL_STRING [iDirection];
                        strMessage += " of ";
                        
                        AddPlanetNameAndCoordinates (
                            strMessage,
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );
                        
                        strMessage += "\n";

                        for (k = 0; k < iNumWatcherEmpires; k ++) {
                            pstrUpdateMessage [piWatcherEmpire[k]] += strMessage;
                        }
                        
                        // Delete ship
                        iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                        RETURN_ON_ERROR(iErrCode);

                        bDied = true;
                    }

                    break;
                }

                bUpdated = true;
                break;

            default:

                // If not a stargate or a jumpgate, something bad happened
                Assert(vShipType.GetInteger() == STARGATE || vShipType.GetInteger() == JUMPGATE);
                break;

            } // End type switch

            ///////////////////////////////////////////////////
            // If the ship needs to be dismantled, do it now //
            ///////////////////////////////////////////////////

            if (bDismantle && !bDied) {
                
                // Delete the ship
                iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                RETURN_ON_ERROR(iErrCode);
                
                // Update message
                pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                pstrUpdateMessage[i] += " of " BEGIN_STRONG;
                pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                pstrUpdateMessage[i] += END_STRONG " was dismantled at ";

                AddPlanetNameAndCoordinates (
                    pstrUpdateMessage[i],
                    vPlanetName.GetCharPtr(),
                    iX,
                    iY
                    );

                pstrUpdateMessage[i] += "\n";

                bDied = true;
            }

            ////////////////////////////////////////////////////////////////////////////////
            // If the ship's action was processed, set its new action to standby or fleet //
            ////////////////////////////////////////////////////////////////////////////////

            // If the ship's action was processed, set its new action to standby.
            // UpdateFleetOrders() will take care of setting ships back to FLEET if necessary
            if (bUpdated && !bDied) {
                
                iErrCode = t_pCache->WriteData(
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Action, 
                    STAND_BY
                    );

                RETURN_ON_ERROR(iErrCode);
            }
            
        } // End ship loop

    } // End empire loop

    return iErrCode;
}

int GameEngine::ProcessNukes (int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, bool* pbSendFatalMessage,
                              const char* pszGameClassName, int iGameClass, int iGameNumber, int* piTotalAg, 
                              int* piTotalMin, int* piTotalFuel, int iNumNukedPlanets, 
                              unsigned int* piNumNukingShips, unsigned int* piNukedPlanetKey, 
                              unsigned int** ppiEmpireNukeKey, unsigned int** ppiShipNukeKey,
                              int* piObliterator, int* piObliterated, unsigned int* piNumObliterations,
                              Variant* pvEmpireName, const char** pstrEmpireDip, const char** pstrEmpireShips, 
                              const char** pstrEmpireMap, String* pstrUpdateMessage, const char** pstrEmpireData, 
                              const char* strGameMap, int iNewUpdateCount, 
                              const GameConfiguration& gcConfig) {
    
    int i, m, j, k, iErrCode = OK, iX, iY;
    unsigned int iKey;
    Variant vOwner, vTemp, vAg, vMin, vFuel, vPop, vMaxPop, vPlanetName, vShipName, vCoord;
    String strMessage;

    int iNukedIndex = iNumEmpires;

    GameUpdateInformation guInfo = { iNewUpdateCount, iNumEmpires, (int*) piEmpireKey, pbAlive, pstrUpdateMessage };

    for (i = 0; i < iNumNukedPlanets; i ++) {
        
        iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::Owner, &vOwner);
        RETURN_ON_ERROR(iErrCode);
        
        // Scan the nuke list
        for (m = 0; m < (int) piNumNukingShips[i]; m ++) {
            
            // Find the nuker's index
            GetEmpireIndex (j, ppiEmpireNukeKey[i][m]);
            
            // Make sure that the nuking empire is still alive
            if (!pbAlive[j]) {
                continue;
            }

            // Make sure empire is alive and owner is not system account (already nuked or annihilated) 
            // and not the nuking empire (trooped or colonized) and is still at war         
            if (vOwner != SYSTEM && vOwner != ppiEmpireNukeKey[i][m]) {
                
                // Get dip status
                if (vOwner.GetInteger() == INDEPENDENT) {
                    vTemp = WAR;
                } else {
                    
                    iErrCode = t_pCache->GetFirstKey(
                        pstrEmpireDip[j], 
                        GameEmpireDiplomacy::ReferenceEmpireKey, 
                        vOwner, 
                        &iKey
                        );

                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(
                        pstrEmpireDip[j], 
                        iKey, 
                        GameEmpireDiplomacy::CurrentStatus, 
                        &vTemp
                        );

                    RETURN_ON_ERROR(iErrCode);
                }
                
                if (vTemp.GetInteger() == WAR) {
                    
                    // Get planet data
                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::Ag, &vAg);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::Minerals, &vMin);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::Fuel, &vFuel);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::Pop, &vPop);
                    RETURN_ON_ERROR(iErrCode);

                    // Nuke planet
                    iErrCode = t_pCache->WriteData(strGameMap, piNukedPlanetKey[i], GameMap::Owner, (int)SYSTEM);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->WriteData(strGameMap, piNukedPlanetKey[i], GameMap::Pop, (int)0);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->Increment (strGameMap, piNukedPlanetKey[i], GameMap::Nuked, (int)1);
                    RETURN_ON_ERROR(iErrCode);

                    // Reduce planet resources by half
                    iErrCode = t_pCache->WriteData(strGameMap, piNukedPlanetKey[i], GameMap::Ag, (int) (vAg.GetInteger() / 2));
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->WriteData(strGameMap, piNukedPlanetKey[i], GameMap::Fuel, (int) (vFuel.GetInteger() / 2));
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->WriteData(strGameMap, piNukedPlanetKey[i], GameMap::Minerals, (int) (vMin.GetInteger() / 2));
                    RETURN_ON_ERROR(iErrCode);

                    // Save old max pop
                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::MaxPop, &vMaxPop);
                    RETURN_ON_ERROR(iErrCode);

                    // Set new max pop
                    vTemp = GetMaxPop (vFuel.GetInteger() / 2, vMin.GetInteger() / 2);

                    iErrCode = t_pCache->WriteData(strGameMap, piNukedPlanetKey[i], GameMap::MaxPop, vTemp);
                    RETURN_ON_ERROR(iErrCode);

                    // Was the nuked planet a home world?
                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::HomeWorld, &vTemp);
                    RETURN_ON_ERROR(iErrCode);

                    if (vTemp.GetInteger() == HOMEWORLD) {

                        // No longer a homeworld
                        iErrCode = t_pCache->WriteData(
                            strGameMap, 
                            piNukedPlanetKey[i], 
                            GameMap::HomeWorld, 
                            NOT_HOMEWORLD
                            );

                        RETURN_ON_ERROR(iErrCode);
                        
                        Assert(vOwner.GetInteger() != INDEPENDENT);

                        // Find owner's index
                        GetEmpireIndex (k, vOwner);

                        // Add one to the obliteration list
                        piObliterator[*piNumObliterations] = j;
                        piObliterated[*piNumObliterations] = k;
                        (*piNumObliterations) ++;
                        
                        //////////////////
                        // Obliteration //
                        //////////////////
                        
                        // Mark empire as dead
                        pbAlive[k] = false;
                        
                        // Update empire's statistics
                        iErrCode = UpdateScoresOnNuke (piEmpireKey[j], piEmpireKey[k], 
                            pvEmpireName[j].GetCharPtr(), pvEmpireName[k].GetCharPtr(), iGameClass, iGameNumber,
                            iNewUpdateCount, EMPIRE_NUKED, pszGameClassName);

                        RETURN_ON_ERROR(iErrCode);

                        // Delete empire's tables
                        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[k], EMPIRE_NUKED, &guInfo);
                        RETURN_ON_ERROR(iErrCode);

                        iNukedIndex = k;
                        
                    } else {
                        
                        if (vOwner.GetInteger() != INDEPENDENT) {

                            // Find owner's index
                            GetEmpireIndex (k, vOwner);

                            // Subtract resources from owner                        
                            // Reduce owner's total pop, targetpop and change number of planets                         
                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::TotalPop, -vPop);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::NumPlanets, -1);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->Increment (pstrEmpireData[k], GameEmpireData::TargetPop, -vMaxPop);
                            RETURN_ON_ERROR(iErrCode);

                            // Reduce owner's resources and econ
                            piTotalAg[k] -= vAg.GetInteger();
                            
                            if (vMin.GetInteger() < vPop.GetInteger()) {
                                piTotalMin[k] -= vMin.GetInteger();
                            } else {
                                piTotalMin[k] -= vPop.GetInteger();
                            }
                            
                            if (vFuel.GetInteger() < vPop.GetInteger()) {
                                piTotalFuel[k] -= vFuel.GetInteger();
                            } else {
                                piTotalFuel[k] -= vPop.GetInteger();
                            }
                        }
                    }
                    
                    iErrCode = t_pCache->ReadData(pstrEmpireShips[j], ppiShipNukeKey[i][m], GameEmpireShips::Name, &vShipName);
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::Name, &vPlanetName);
                    RETURN_ON_ERROR(iErrCode);
                    
                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::Coordinates, &vCoord);
                    RETURN_ON_ERROR(iErrCode);

                    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);

                    //////////////////////////////////////////////////////
                    // Handle the NUM_NUKES_BEFORE_ANNIHILATION'th nuke //
                    //////////////////////////////////////////////////////

                    iErrCode = t_pCache->ReadData(strGameMap, piNukedPlanetKey[i], GameMap::Nuked, &vTemp);
                    RETURN_ON_ERROR(iErrCode);

                    if (vTemp.GetInteger() >= gcConfig.iNukesForQuarantine) {
                        
                        // Set ag to zero - destroyed ecosystem
                        iErrCode = t_pCache->WriteData(strGameMap, piNukedPlanetKey[i], GameMap::Ag, 0);
                        RETURN_ON_ERROR(iErrCode);
                        
                        // Increment annihilated updates
                        iErrCode = t_pCache->Increment (
                            strGameMap, 
                            piNukedPlanetKey[i], 
                            GameMap::Annihilated, 
                            gcConfig.iUpdatesInQuarantine
                            );

                        RETURN_ON_ERROR(iErrCode);
                    
                        strMessage.Clear();
                        strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[j].GetCharPtr();
                        strMessage += END_STRONG " nuked and annihilated " BEGIN_STRONG;
                        
                        AddPlanetNameAndCoordinates (
                            strMessage,
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );
                        
                        strMessage += END_STRONG "\n";

                    } else {

                        // Nuke did not destroy ecosystem
                        strMessage.Clear();
                        strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                        strMessage += " of " BEGIN_STRONG;
                        strMessage += pvEmpireName[j].GetCharPtr();
                        strMessage += END_STRONG " nuked " BEGIN_STRONG;
                        
                        AddPlanetNameAndCoordinates (
                            strMessage,
                            vPlanetName.GetCharPtr(), 
                            iX, 
                            iY
                            );
                        
                        strMessage += END_STRONG "\n";
                    }

                    for (k = 0; k < iNumEmpires; k ++) {
                        
                        // Allow the nuked empire to see his own nuking
                        if (!pbAlive[k])
                        {
                            if (k != iNukedIndex)
                                continue;

                            iNukedIndex = iNumEmpires;
                            pstrUpdateMessage[k] += strMessage;
                        }
                        else
                        {
                            iErrCode = t_pCache->GetFirstKey(
                                pstrEmpireMap[k], 
                                GameEmpireMap::PlanetKey, 
                                piNukedPlanetKey[i], 
                                &iKey
                                );

                            if (iErrCode == ERROR_DATA_NOT_FOUND)
                            {
                                iErrCode = OK;
                            }
                            else
                            {
                                RETURN_ON_ERROR(iErrCode);
                                pstrUpdateMessage[k] += strMessage;
                            }
                        }
                    }
                    
                    // Exit ship loop:  planet has been nuked
                    break;
                }

            }   // End if ship can nuke 
    
        }   // End nuking ship loop
    }
    
    return iErrCode;
}

int GameEngine::SharePlanetsBetweenFriends (int iGameClass, int iGameNumber, 
                                            unsigned int iEmpireIndex1, unsigned int iEmpireIndex2,
                                            const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireData,
                                            const char* pszGameMap, unsigned int iNumEmpires, 
                                            unsigned int* piEmpireKey, int iDipLevel, 
                                            bool bShareWithFriendsClosure) {
    int iErrCode;

    unsigned int iNumPlanets1, iNumPlanets2, i, * piProxyKey1 = NULL, * piProxyKey2 = NULL, iNumAcquaintances1, iNumAcquaintances2;
    AutoFreeKeys free_piProxyKey1(piProxyKey1);
    AutoFreeKeys free_piProxyKey2(piProxyKey2);

    Variant* pvPlanetKey1 = NULL, * pvPlanetKey2 = NULL, * pvAcquaintanceKey1 = NULL, * pvAcquaintanceKey2 = NULL;
    AutoFreeData free_pvPlanetKey1(pvPlanetKey1);
    AutoFreeData free_pvPlanetKey2(pvPlanetKey2);
    AutoFreeData free_pvAcquaintanceKey1(pvAcquaintanceKey1);
    AutoFreeData free_pvAcquaintanceKey2(pvAcquaintanceKey2);

    // Read diplomacy tables
    iErrCode = t_pCache->ReadColumn(
        pstrEmpireDip[iEmpireIndex1], 
        GameEmpireDiplomacy::ReferenceEmpireKey, 
        &piProxyKey1, 
        &pvAcquaintanceKey1, 
        &iNumAcquaintances1
        );
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadColumn(
        pstrEmpireDip[iEmpireIndex2], 
        GameEmpireDiplomacy::ReferenceEmpireKey, 
        &piProxyKey2, 
        &pvAcquaintanceKey2, 
        &iNumAcquaintances2
        );
    RETURN_ON_ERROR(iErrCode);

    // Read planet keys
    iErrCode = t_pCache->ReadColumn(
        pstrEmpireMap[iEmpireIndex1], 
        GameEmpireMap::PlanetKey, 
        NULL,
        &pvPlanetKey1, 
        &iNumPlanets1
        );
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadColumn(
        pstrEmpireMap[iEmpireIndex2], 
        GameEmpireMap::PlanetKey, 
        NULL,
        &pvPlanetKey2, 
        &iNumPlanets2
        );
    RETURN_ON_ERROR(iErrCode);

    // Share 1's planets with 2
    for (i = 0; i < iNumPlanets1; i ++) {

        iErrCode = SharePlanetBetweenFriends (
            iGameClass,
            iGameNumber,
            pvPlanetKey1[i].GetInteger(), 
            iEmpireIndex2,
            pstrEmpireMap, 
            pstrEmpireDip, 
            pstrEmpireData,
            pszGameMap,
            iNumEmpires, 
            piEmpireKey, 
            iDipLevel,
            pvAcquaintanceKey2, 
            piProxyKey2,
            iNumAcquaintances2,
            bShareWithFriendsClosure
            );
        
        RETURN_ON_ERROR(iErrCode);
    }
    
    // Share 1's planets with 2
    for (i = 0; i < iNumPlanets2; i ++)
    {
        iErrCode = SharePlanetBetweenFriends (
            iGameClass,
            iGameNumber,
            pvPlanetKey2[i].GetInteger(), 
            iEmpireIndex1,
            pstrEmpireMap, 
            pstrEmpireDip, 
            pstrEmpireData,
            pszGameMap,
            iNumEmpires, 
            piEmpireKey, 
            iDipLevel,
            pvAcquaintanceKey1, 
            piProxyKey1,
            iNumAcquaintances1,
            bShareWithFriendsClosure
            );
        
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


int GameEngine::SharePlanetBetweenFriends (int iGameClass, int iGameNumber, unsigned int iPlanetKey, 
                                           unsigned int iEmpireIndex,
                                           const char** pstrEmpireMap, const char** pstrEmpireDip, 
                                           const char** pstrEmpireData,
                                           const char* pszGameMap, unsigned int iNumEmpires, 
                                           unsigned int* piEmpireKey, int iDipLevel,
                                           Variant* pvAcquaintanceKey, unsigned int* piProxyKey,
                                           unsigned int iNumAcquaintances, bool bShareWithFriendsClosure) {

    Assert((pvAcquaintanceKey == NULL && piProxyKey == NULL) ||
           (pvAcquaintanceKey != NULL && piProxyKey != NULL && iNumAcquaintances > 0));

    // Check for planet already in empire's map
    unsigned int iKey, i, j;

    int iErrCode = t_pCache->GetFirstKey(
        pstrEmpireMap[iEmpireIndex], 
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        &iKey
        );
    
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        return iErrCode;
    }

    Variant pvColData[GameEmpireMap::NumColumns], vNeighbourPlanetKey, * pvPassedPtr = pvAcquaintanceKey, * pvFreeAcquaintanceKey = NULL;
    unsigned int* piFreeProxyKey = NULL;

    AutoFreeData free_pvFreeAcquaintanceKey(pvFreeAcquaintanceKey);
    AutoFreeKeys free_piFreeProxyKey(piFreeProxyKey);

    // It's an unknown planet
    pvColData[GameEmpireMap::iGameClass] = iGameClass;
    pvColData[GameEmpireMap::iGameNumber] = iGameNumber;
    pvColData[GameEmpireMap::iEmpireKey] = piEmpireKey[iEmpireIndex];
    pvColData[GameEmpireMap::iPlanetKey] = iPlanetKey;
    pvColData[GameEmpireMap::iNumUncloakedShips] = 0;
    pvColData[GameEmpireMap::iNumCloakedBuildShips] = 0;
    pvColData[GameEmpireMap::iNumUncloakedBuildShips] = 0;
    pvColData[GameEmpireMap::iNumCloakedShips] = 0;

    // For each direction, check if the planet is in 2's map
    int iExplored = 0;

    ENUMERATE_CARDINAL_POINTS (i) {

        // Get neighbour planet key
        iErrCode = t_pCache->ReadData(
            pszGameMap, 
            iPlanetKey, 
            GameMap::ColumnNames[GameMap::iNorthPlanetKey + i],
            &vNeighbourPlanetKey
            );
        RETURN_ON_ERROR(iErrCode);
        
        // Search for neighbour planet key in empire's map
        if (vNeighbourPlanetKey.GetInteger() != NO_KEY) {
            
            iErrCode = t_pCache->GetFirstKey(
                pstrEmpireMap[iEmpireIndex], 
                GameEmpireMap::PlanetKey,
                vNeighbourPlanetKey,
                &iKey
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            else
            {
                RETURN_ON_ERROR(iErrCode);

                // The neighbour exists, so let's fix him up
                iErrCode = t_pCache->WriteOr(
                    pstrEmpireMap[iEmpireIndex], 
                    iKey,
                    GameEmpireMap::Explored,
                    OPPOSITE_EXPLORED_X[i]
                    );
                RETURN_ON_ERROR(iErrCode);
                
                // Make planet aware of neighbour
                iExplored |= EXPLORED_X[i];
            }
        }
    }

    pvColData[GameEmpireMap::iExplored] = iExplored;
    
    // Insert into empire's map
    iErrCode = t_pCache->InsertRow(pstrEmpireMap[iEmpireIndex], GameEmpireMap::Template, pvColData, NULL);
    RETURN_ON_ERROR(iErrCode);
    
    // Update max, min
    int iX, iY;
    Variant vMinX, vMaxX, vMinY, vMaxY, vCoord;
    
    iErrCode = t_pCache->ReadData(pszGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
    RETURN_ON_ERROR(iErrCode);
    
    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);

    // Fix empire's min / max
    iErrCode = SetNewMinMaxIfNecessary (iGameClass, iGameNumber, piEmpireKey[iEmpireIndex], iX, iY);
    RETURN_ON_ERROR(iErrCode);

    // Maybe empire has some friends to share with too?
    if (bShareWithFriendsClosure)
    {
        if (pvPassedPtr == NULL && iNumAcquaintances != 0)
        {
            iErrCode = t_pCache->ReadColumn(
                pstrEmpireDip[iEmpireIndex], 
                GameEmpireDiplomacy::ReferenceEmpireKey, 
                &piProxyKey, 
                &pvAcquaintanceKey, 
                &iNumAcquaintances
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);

            pvFreeAcquaintanceKey = pvAcquaintanceKey;
            piFreeProxyKey = piProxyKey;
        }
        
        if (iNumAcquaintances > 0)
        {
            Variant vDipStatus;

            for (i = 0; i < iNumAcquaintances; i ++) {
                
                iErrCode = t_pCache->ReadData(
                    pstrEmpireDip[iEmpireIndex], 
                    piProxyKey[i], 
                    GameEmpireDiplomacy::CurrentStatus, 
                    &vDipStatus
                    );
                RETURN_ON_ERROR(iErrCode);
                
                if (vDipStatus.GetInteger() >= iDipLevel) {
                    
                    GetEmpireIndex (j, pvAcquaintanceKey[i]);
                    
                    iErrCode = SharePlanetBetweenFriends (
                        iGameClass,
                        iGameNumber,
                        iPlanetKey, 
                        j,
                        pstrEmpireMap, 
                        pstrEmpireDip,
                        pstrEmpireData,
                        pszGameMap,
                        iNumEmpires, 
                        piEmpireKey,
                        iDipLevel,
                        NULL,
                        NULL,
                        -1,
                        bShareWithFriendsClosure
                        );
                    RETURN_ON_ERROR(iErrCode);
                }

            }   // End acquaintance loop
        
        }   // End if acquaintances > 0
    }

    return iErrCode;
}


int GameEngine::GetPlanetNameWithCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, 
                                              char pszName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH]) {

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    return GetPlanetNameWithCoordinates (strGameMap, iPlanetKey, pszName);
}

int GameEngine::GetPlanetNameWithCoordinates (const char* pszGameMap, unsigned int iPlanetKey, 
                                              char pszName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH]) {

    Variant vPlanetName, vCoord;
    int iX, iY, iErrCode;

    iErrCode = t_pCache->ReadData(pszGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadData(pszGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
    RETURN_ON_ERROR(iErrCode);
    
    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);

    sprintf(pszName, "%s (%i,%i)", vPlanetName.GetCharPtr(), iX, iY);
    return iErrCode;
}


int GameEngine::ProcessGates (int iGameClass, int iGameNumber,
                              unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive,
                              String* pstrUpdateMessage, const Variant* pvGoodColor, 
                              const Variant* pvBadColor, const Variant* pvEmpireName, 
                              unsigned int* piOriginalPlanetOwner,
                              unsigned int* piOriginalNumObliterations,
                              const char** pstrEmpireShips, const char** pstrEmpireFleets, 
                              const char** pstrEmpireMap, const char** pstrEmpireData, 
                              const char** pstrEmpireDip,
                              const char* strGameMap, 
                              const GameConfiguration& gcConfig, int iGameClassOptions) {

    unsigned int* piShipKey = NULL, iNumKeys, i, j, k;
    AutoFreeKeys free_piShipKey(piShipKey);

    int iDestX, iDestY, iSrcX, iSrcY, iErrCode = OK;
    float fGateCost = 0;

    bool bGateSurvived, bFailed;
    
    Variant vNewPlanetKey, vOldPlanetName, vCoord, vTemp, vBR, vShipName, vPlanetName, vShipType, vOwner, vAnnihilated;

    unsigned int* piGateEmpireIndex = (unsigned int*) StackAlloc (iNumEmpires * sizeof (unsigned int));
    unsigned int iNumGateEmpires;

    for (i = 0; i < iNumEmpires; i ++) {
        
        if (!pbAlive[i]) {
            continue;
        }

        if (piShipKey)
        {
            t_pCache->FreeKeys(piShipKey);
            piShipKey = NULL;
        }
        
        iErrCode = t_pCache->GetEqualKeys (
            pstrEmpireShips[i], 
            GameEmpireShips::Action, 
            GATE_SHIPS,
            &piShipKey, 
            &iNumKeys
            );

         if (iErrCode == ERROR_DATA_NOT_FOUND)
         {
             iErrCode = OK;
             continue;
         }
         RETURN_ON_ERROR(iErrCode);
        
        // Randomize gate list
        Algorithm::Randomize<unsigned int> (piShipKey, iNumKeys);
        
        // Loop through all gating ships            
        for (j = 0; j < iNumKeys; j ++) {

            bFailed = false;
            bGateSurvived = true;
            
            // Get the key of the destination planet
            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::GateDestination, &vNewPlanetKey);
            RETURN_ON_ERROR(iErrCode);

            // Get some data        
            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentBR, &vBR);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Name, &vShipName);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            unsigned int iPlanetKey = vTemp.GetInteger();

            iErrCode = t_pCache->ReadData(pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vShipType);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, &vOldPlanetName);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameMap, vNewPlanetKey.GetInteger(), GameMap::Name, &vPlanetName);
            RETURN_ON_ERROR(iErrCode);
            
            // Get src coords
            iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
            RETURN_ON_ERROR(iErrCode);
                
            GetCoordinates (vCoord.GetCharPtr(), &iSrcX, &iSrcY);

            // Get dest coords
            iErrCode = t_pCache->ReadData(strGameMap, vNewPlanetKey.GetInteger(), GameMap::Coordinates, &vCoord);
            RETURN_ON_ERROR(iErrCode);
            
            GetCoordinates (vCoord.GetCharPtr(), &iDestX, &iDestY);

            // Ship type specific information
            float fRangeFactor = 0;
            bool bRangeLimited = false;
            if (vShipType.GetInteger() == STARGATE) {

                // Get original owner
                if (piOriginalPlanetOwner [vNewPlanetKey.GetInteger()] == NO_KEY) {
                    
                    iErrCode = t_pCache->ReadData(strGameMap, vNewPlanetKey.GetInteger(), GameMap::Owner, &vOwner);
                    RETURN_ON_ERROR(iErrCode);

                } else {
                    
                    vOwner = piOriginalPlanetOwner [vNewPlanetKey.GetInteger()];
                }

                fGateCost = gcConfig.fStargateGateCost;
                fRangeFactor = gcConfig.fStargateRangeFactor;
                bRangeLimited = (gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE) != 0;
            }

            else if (vShipType.GetInteger() == JUMPGATE) {
                
                fGateCost = gcConfig.fJumpgateGateCost;
                fRangeFactor = gcConfig.fJumpgateRangeFactor;
                bRangeLimited = (gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE) != 0;

                // Enforce annihilation rules
                if (piOriginalNumObliterations [vNewPlanetKey.GetInteger()] == ANNIHILATED_UNKNOWN) {

                    iErrCode = t_pCache->ReadData(strGameMap, vNewPlanetKey.GetInteger(), GameMap::Annihilated, &vAnnihilated);
                    RETURN_ON_ERROR(iErrCode);

                } else {

                    vAnnihilated = piOriginalNumObliterations [vNewPlanetKey.GetInteger()];
                }
            }
            else Assert(!"Non-gate cannot gate");
   
            // Check for incapacity
            if (vBR.GetFloat() < fGateCost ||
                
                (vShipType.GetInteger() == STARGATE && 
                vOwner.GetInteger() != (int) piEmpireKey[i]) ||
                
                (bRangeLimited && vBR.GetFloat() < 
                GetGateBRForRange (fRangeFactor, iSrcX, iSrcY, iDestX, iDestY)) ||
                
                (vShipType.GetInteger() == JUMPGATE &&
                vAnnihilated.GetInteger() != NOT_ANNIHILATED)
                
                ) {
                
                pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                pstrUpdateMessage[i] += SHIP_TYPE_STRING[vShipType.GetInteger()];
                pstrUpdateMessage[i] += " ";
                pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                pstrUpdateMessage[i] += " of ";
                pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                pstrUpdateMessage[i] += " could not gate any ships to ";

                AddPlanetNameAndCoordinates (
                    pstrUpdateMessage[i],
                    vPlanetName.GetCharPtr(),
                    iDestX,
                    iDestY
                    );

                pstrUpdateMessage[i] += END_FONT "\n";
                
            } else {

                bool bGated = false;

                piGateEmpireIndex[0] = i;
                iNumGateEmpires = 1;

                if (iGameClassOptions & USE_FRIENDLY_GATES) {

                    int iStatus;

                    for (k = 0; k < iNumEmpires; k ++) {

                        if (k == i || !pbAlive[k]) {
                            continue;
                        }

                        iErrCode = GetVisibleDiplomaticStatus (
                            iGameClass,
                            iGameNumber,
                            piEmpireKey[i],
                            piEmpireKey[k],
                            NULL,
                            NULL,
                            &iStatus,
                            NULL
                            );

                        RETURN_ON_ERROR(iErrCode);

                        if (iStatus == ALLIANCE) {
                            piGateEmpireIndex[iNumGateEmpires ++] = k;
                        }
                    }
                }

                for (k = 0; k < iNumGateEmpires; k ++) {

                    unsigned int iGatedEmpireIndex = piGateEmpireIndex[k];

                    // Gate ships
                    iErrCode = GateShips (
                        iGameClass, iGameNumber, i, 
                        pvEmpireName[i].GetCharPtr(),
                        iGatedEmpireIndex,
                        pvEmpireName[iGatedEmpireIndex].GetCharPtr(),
                        piEmpireKey [iGatedEmpireIndex],

                        vShipType.GetInteger(),
                        vShipName.GetCharPtr(),

                        iPlanetKey,
                        vNewPlanetKey.GetInteger(),
                        vOldPlanetName.GetCharPtr(),
                        vPlanetName.GetCharPtr(),
                        iSrcX, iSrcY, iDestX, iDestY,

                        strGameMap, 
                        pstrEmpireMap[iGatedEmpireIndex],
                        pstrEmpireShips[iGatedEmpireIndex],
                        pstrEmpireFleets[iGatedEmpireIndex],
                        pstrEmpireDip[iGatedEmpireIndex],

                        pstrEmpireMap, pstrEmpireDip,
                               
                        iNumEmpires, pbAlive, pstrUpdateMessage, piEmpireKey, pvEmpireName
                        );

                    if (iErrCode == ERROR_EMPIRE_CANNOT_SEE_PLANET || iErrCode == ERROR_EMPIRE_HAS_NO_SHIPS)
                    {
                        iErrCode = OK;
                    }
                    else
                    {
                        RETURN_ON_ERROR(iErrCode);
                        bGated = true;
                    }
                }

                if (!bGated) {

                    pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                    pstrUpdateMessage[i] += SHIP_TYPE_STRING[vShipType.GetInteger()];
                    pstrUpdateMessage[i] += " ";
                    pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                    pstrUpdateMessage[i] += " of ";
                    pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                    pstrUpdateMessage[i] += " could not find any ships to gate\n" END_FONT;
                
                } else {

                    // Did gate survive?
                    if (vBR.GetFloat() - fGateCost > FLOAT_PROXIMITY_TOLERANCE) {
                        
                        // Subtract from the current and max BR of the gate
                        iErrCode = ChangeShipTypeOrMaxBR (
                            pstrEmpireShips[i],
                            pstrEmpireData[i],
                            piEmpireKey[i],
                            piShipKey[j],
                            vShipType.GetInteger(),
                            vShipType.GetInteger(),
                            - fGateCost
                            );
                        
                        RETURN_ON_ERROR(iErrCode);
                        
                    } else {
                        
                        bGateSurvived = false;

                        pstrUpdateMessage[i] += BEGIN_BAD_FONT(i);
                        pstrUpdateMessage[i] += SHIP_TYPE_STRING[vShipType.GetInteger()];
                        pstrUpdateMessage[i] += " ";
                        pstrUpdateMessage[i].AppendHtml (vShipName.GetCharPtr(), 0, false);
                        pstrUpdateMessage[i] += " of ";
                        pstrUpdateMessage[i] += pvEmpireName[i].GetCharPtr();
                        pstrUpdateMessage[i] += " destroyed itself gating ships\n" END_FONT;
                        
                        // Scrap heap
                        iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                        RETURN_ON_ERROR(iErrCode);
                    }
                }

            }   // End can ship gate?

            // Set gate to stand by
            if (bGateSurvived) {

                iErrCode = t_pCache->WriteData(
                    pstrEmpireShips[i],
                    piShipKey[j],
                    GameEmpireShips::Action, 
                    STAND_BY
                    );

                RETURN_ON_ERROR(iErrCode);
            }

        }   // End loop through all gates

        // If we reach this point, there's something to delete
        t_pCache->FreeKeys(piShipKey);
        piShipKey = NULL;

    }   // End empire loop

    return iErrCode;
}

int GameEngine::GateShips (int iGameClass, int iGameNumber,
                           unsigned int iGaterEmpireIndex, const char* pszGaterEmpireName,
                           unsigned int iGatedEmpireIndex, const char* pszGatedEmpireName,
                           int iGatedEmpireKey,
                           
                           int iShipType, const char* pszGateName,
                           
                           unsigned int iOldPlanetKey, unsigned int iNewPlanetKey,
                           const char* pszOldPlanetName, const char* pszNewPlanetName,
                           int iSrcX, int iSrcY, int iDestX, int iDestY,
                           
                           const char* pszGameMap,
                           const char* pszEmpireMap, const char* pszEmpireShips,
                           const char* pszEmpireFleets, const char* pszEmpireDip,

                           const char** pstrEmpireMap, const char** pstrEmpireDip,
                           
                           unsigned int iNumEmpires, bool* pbAlive,
                           String* pstrUpdateMessage, const unsigned int* piEmpireKey, 
                           const Variant* pvEmpireName) {

    int iErrCode;

    unsigned int k, iOldProxyKey, iNewProxyKey, * piGateShipKey = NULL, iNumShips, iNumPrivateGatedShips, iNumPublicGatedShips;
    AutoFreeKeys free_piGateShipKey(piGateShipKey);

    // Make sure we can see the planet
    iErrCode = t_pCache->GetFirstKey(
        pszEmpireMap,
        GameEmpireMap::PlanetKey,
        iNewPlanetKey, 
        &iNewProxyKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return ERROR_EMPIRE_CANNOT_SEE_PLANET;
    }
    RETURN_ON_ERROR(iErrCode);

    Assert(iNewProxyKey != NO_KEY);

    // Get potential ships to gate
    iErrCode = t_pCache->GetEqualKeys (
        pszEmpireShips,
        GameEmpireShips::CurrentPlanet,
        iOldPlanetKey,
        &piGateShipKey,
        &iNumShips
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        Assert(iGaterEmpireIndex != iGatedEmpireIndex);
        return ERROR_EMPIRE_HAS_NO_SHIPS;
    }
    RETURN_ON_ERROR(iErrCode);

    // Maybe we're alone?
    if (iNumShips == 1 && iGaterEmpireIndex == iGatedEmpireIndex)
    {
        return ERROR_EMPIRE_HAS_NO_SHIPS;
    }
    
    ////////////////
    // Gate away! //
    ////////////////
                
    // Get proxy key of old planet
    iErrCode = t_pCache->GetFirstKey(
        pszEmpireMap, 
        GameEmpireMap::PlanetKey, 
        iOldPlanetKey, 
        &iOldProxyKey
        );
    
    RETURN_ON_ERROR(iErrCode);

#ifdef _DEBUG
    // Verify ship count on old planet
    Variant vU, vC;
    iErrCode = t_pCache->ReadData(pszEmpireMap, iOldProxyKey, GameEmpireMap::NumCloakedShips, &vC);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadData(pszEmpireMap, iOldProxyKey, GameEmpireMap::NumUncloakedShips, &vU);
    RETURN_ON_ERROR(iErrCode);
    Assert(vU + vC == iNumShips);
#endif
                
    // Gate all mobile ships on planet
    iNumPrivateGatedShips = iNumPublicGatedShips = 0;
    
    for (k = 0; k < iNumShips; k ++)
    {
        Variant vTemp;
        iErrCode = t_pCache->ReadData(pszEmpireShips, piGateShipKey[k], GameEmpireShips::Type, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        Assert(vTemp.GetInteger() >= FIRST_SHIP && vTemp.GetInteger() <= LAST_SHIP);
        
        if (IsMobileShip (vTemp.GetInteger())) {
                        
            ///////////////
            // Gate ship //
            ///////////////
            
            // Set ship location to new planet
            iErrCode = t_pCache->WriteData(
                pszEmpireShips, 
                piGateShipKey[k], 
                GameEmpireShips::CurrentPlanet, 
                (int)iNewPlanetKey
                );
            
            RETURN_ON_ERROR(iErrCode);
            
            // Decrease number of ships at source planet, increment at new planet
            iErrCode = t_pCache->ReadData(
                pszEmpireShips, 
                piGateShipKey[k], 
                GameEmpireShips::State, 
                &vTemp
                );
            
            RETURN_ON_ERROR(iErrCode);
                        
            if (vTemp.GetInteger() & CLOAKED) {
                
                iErrCode = t_pCache->Increment (
                    pszEmpireMap, 
                    iOldProxyKey, 
                    GameEmpireMap::NumCloakedShips, 
                    -1
                    );

                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->Increment (
                    pszEmpireMap,
                    iNewProxyKey, 
                    GameEmpireMap::NumCloakedShips,
                    1
                    );
                
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->Increment (
                    pszGameMap,
                    iOldPlanetKey, 
                    GameMap::NumCloakedShips, 
                    -1
                    );
                
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->Increment (
                    pszGameMap,
                    iNewPlanetKey, 
                    GameMap::NumCloakedShips, 
                    1
                    );
                
                RETURN_ON_ERROR(iErrCode);
                            
            } else {
                
                iErrCode = t_pCache->Increment (
                    pszEmpireMap,
                    iOldProxyKey, 
                    GameEmpireMap::NumUncloakedShips,
                    -1
                    );
                
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->Increment (
                    pszEmpireMap,
                    iNewProxyKey, 
                    GameEmpireMap::NumUncloakedShips,
                    1
                    );
                
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->Increment (
                    pszGameMap,
                    iOldPlanetKey,
                    GameMap::NumUncloakedShips,
                    -1
                    );
                
                RETURN_ON_ERROR(iErrCode);
#ifdef _DEBUG
                Variant vFooBar;
                iErrCode = t_pCache->ReadData(pszGameMap, iOldPlanetKey, GameMap::NumUncloakedShips, &vFooBar);
                RETURN_ON_ERROR(iErrCode);
                Assert(vFooBar >= 0);
#endif
                iErrCode = t_pCache->Increment (pszGameMap, iNewPlanetKey, GameMap::NumUncloakedShips, 1);
                RETURN_ON_ERROR(iErrCode);
                
                iNumPublicGatedShips ++;
            }
            
            iNumPrivateGatedShips ++;
        }
    }

    if (iNumPrivateGatedShips == 0)
    {
        Assert(iNumPublicGatedShips == 0);
        return ERROR_EMPIRE_HAS_NO_SHIPS;
    }

    if (piGateShipKey)
    {
        t_pCache->FreeKeys(piGateShipKey);
        piGateShipKey = NULL;
    }

    // Gate fleets with ships also
    iErrCode = t_pCache->GetEqualKeys (
        pszEmpireFleets,
        GameEmpireFleets::CurrentPlanet,
        iOldPlanetKey,
        &piGateShipKey, 
        &iNumShips
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    for (k = 0; k < iNumShips; k ++) {
            
        // Set fleet location to new planet
        iErrCode = t_pCache->WriteData(
            pszEmpireFleets, 
            piGateShipKey[k], 
            GameEmpireFleets::CurrentPlanet, 
            (int)iNewPlanetKey
            );
            
        RETURN_ON_ERROR(iErrCode);
    }

    pstrUpdateMessage[iGatedEmpireIndex] += SHIP_TYPE_STRING[iShipType];
    pstrUpdateMessage[iGatedEmpireIndex] += " ";
    pstrUpdateMessage[iGatedEmpireIndex].AppendHtml (pszGateName, 0, false);
    pstrUpdateMessage[iGatedEmpireIndex] += " of " BEGIN_STRONG;
    pstrUpdateMessage[iGatedEmpireIndex] += pszGaterEmpireName;
    pstrUpdateMessage[iGatedEmpireIndex] += END_STRONG " gated " BEGIN_STRONG;
        
    pstrUpdateMessage[iGatedEmpireIndex] += iNumPrivateGatedShips;
    pstrUpdateMessage[iGatedEmpireIndex] += (iNumPrivateGatedShips == 1 ? 
        END_STRONG " ship of " BEGIN_STRONG : 
        END_STRONG " ships of " BEGIN_STRONG);
        
    pstrUpdateMessage[iGatedEmpireIndex] += pszGatedEmpireName;
    pstrUpdateMessage[iGatedEmpireIndex] += END_STRONG " from ";
        
    AddPlanetNameAndCoordinates (pstrUpdateMessage[iGatedEmpireIndex], pszOldPlanetName, iSrcX, iSrcY);
    pstrUpdateMessage[iGatedEmpireIndex] += " to ";
    AddPlanetNameAndCoordinates (pstrUpdateMessage[iGatedEmpireIndex], pszNewPlanetName, iDestX, iDestY);
    pstrUpdateMessage[iGatedEmpireIndex] += "\n";

    if (iNumPublicGatedShips > 0) {
        
        // Scan empires for watchers
        String strNew, strOld, strBoth;
        for (k = 0; k < iNumEmpires; k ++) {

            unsigned int iGateOldPlanetKey, iGateNewPlanetKey;
            
            if (!pbAlive[k] || k == iGatedEmpireIndex) {
                continue;
            }

            if (k == iGaterEmpireIndex && iGaterEmpireIndex == iGatedEmpireIndex) {
                continue;
            }

            // TODO - precalculate and pass in as array?
            iErrCode = t_pCache->GetFirstKey(
                pstrEmpireMap[k], 
                GameEmpireMap::PlanetKey, 
                iOldPlanetKey,
                &iGateOldPlanetKey
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->GetFirstKey(
                pstrEmpireMap[k], 
                GameEmpireMap::PlanetKey, 
                iNewPlanetKey,
                &iGateNewPlanetKey
                );
            
            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);

            if (iGateOldPlanetKey == NO_KEY && iGateNewPlanetKey != NO_KEY)
            {
                if (strNew.IsBlank()) {
                    
                    strNew = BEGIN_STRONG;
                    strNew += iNumPublicGatedShips;
                    strNew += (iNumPublicGatedShips == 1 ? 
                        END_STRONG " ship of " BEGIN_STRONG : 
                        END_STRONG " ships of " BEGIN_STRONG);
                    strNew += pszGatedEmpireName;
                    strNew += (iNumPublicGatedShips == 1 ?
                        END_STRONG " was gated from an unknown planet to " : 
                        END_STRONG " were gated from an unknown planet to ");
                    AddPlanetNameAndCoordinates (strNew, pszNewPlanetName, iDestX, iDestY);
                    strNew += "\n";
                }
                pstrUpdateMessage[k] += strNew;
            }
            else if (iGateOldPlanetKey != NO_KEY && iGateNewPlanetKey == NO_KEY) {
                
                if (strOld.IsBlank()) {
                    
                    strOld += SHIP_TYPE_STRING[iShipType];
                    strOld += " ";
                    strOld.AppendHtml (pszGateName, 0, false);
                    strOld += " of " BEGIN_STRONG;
                    strOld += pszGaterEmpireName;
                    strOld += END_STRONG " gated " BEGIN_STRONG;
                    strOld += iNumPublicGatedShips;
                    strOld += (iNumPublicGatedShips == 1 ? 
                        END_STRONG " ship of " BEGIN_STRONG : 
                        END_STRONG " ships of " BEGIN_STRONG);
                    strOld += pszGatedEmpireName;
                    strOld += END_STRONG " from ";
                    AddPlanetNameAndCoordinates (strOld, pszOldPlanetName, iSrcX, iSrcY);                                                     
                    strOld += " to an unknown planet\n";
                }
                pstrUpdateMessage[k] += strOld;
            }

            else if (iGateOldPlanetKey != NO_KEY && iGateNewPlanetKey != NO_KEY) {
                
                // Add to update message
                if (strBoth.IsBlank()) {
                    
                    strBoth += SHIP_TYPE_STRING[iShipType];
                    strBoth += " ";
                    strBoth.AppendHtml (pszGateName, 0, false);
                    strBoth += " of " BEGIN_STRONG;
                    strBoth += pszGaterEmpireName;
                    strBoth += END_STRONG " gated " BEGIN_STRONG;
                    strBoth += iNumPublicGatedShips;
                    strBoth += (iNumPublicGatedShips == 1 ? 
                        END_STRONG " ship of " BEGIN_STRONG :
                        END_STRONG " ships of " BEGIN_STRONG);
                    strBoth += pszGatedEmpireName;
                    strBoth += END_STRONG " from ";

                    AddPlanetNameAndCoordinates (strBoth, pszOldPlanetName, iSrcX, iSrcY);
                    strBoth += " to ";
                    AddPlanetNameAndCoordinates (strBoth, pszNewPlanetName, iDestX, iDestY);
                    strBoth += "\n";
                }
                pstrUpdateMessage[k] += strBoth;
            }
        
        }   // End empire loop

        // Check for first contact if non-cloaked ships were jumpgated
        if (iShipType == JUMPGATE) {
            
            iErrCode = CheckForFirstContact(
                iGameClass, iGameNumber, iGatedEmpireKey, iGatedEmpireIndex,
                iNewPlanetKey,
                pszNewPlanetName,
                iDestX,
                iDestY,
                iNumEmpires,
                piEmpireKey,
                pvEmpireName,
                pszEmpireDip,
                pszGameMap,
                pstrEmpireDip,
                pstrUpdateMessage
                );
            
            RETURN_ON_ERROR(iErrCode);
        }

     } else if (iNumPrivateGatedShips > 0 && iGaterEmpireIndex != iGatedEmpireIndex) {

        // Handle case where empire gates ally's cloaker(s)
        pstrUpdateMessage[iGaterEmpireIndex] += SHIP_TYPE_STRING[iShipType];
        pstrUpdateMessage[iGaterEmpireIndex] += " ";
        pstrUpdateMessage[iGaterEmpireIndex].AppendHtml (pszGateName, 0, false);
        pstrUpdateMessage[iGaterEmpireIndex] += " of " BEGIN_STRONG;
        pstrUpdateMessage[iGaterEmpireIndex] += pszGaterEmpireName;
        pstrUpdateMessage[iGaterEmpireIndex] += END_STRONG " gated an unknown number of ships from ";

        AddPlanetNameAndCoordinates (pstrUpdateMessage[iGaterEmpireIndex], pszOldPlanetName, iSrcX, iSrcY);
        pstrUpdateMessage[iGaterEmpireIndex] += " to ";
        AddPlanetNameAndCoordinates (pstrUpdateMessage[iGaterEmpireIndex], pszNewPlanetName, iDestX, iDestY);
        pstrUpdateMessage[iGaterEmpireIndex] += "\n";
    }

    return iErrCode;
}

int GameEngine::CheckForFirstContact(int iGameClass, int iGameNumber, int iEmpireKey,
                                     int iEmpireIndex, int iPlanetKey, const char* pszPlanetName, int iNewX, int iNewY,
                                     unsigned int iNumEmpires, const unsigned int* piEmpireKey, const Variant* pvEmpireName,
                                     const char* strEmpireDip, const char* strGameMap, const char** pstrEmpireDip, String* pstrUpdateMessage
                                     ) {
    
    int iErrCode;
    unsigned int k, iKey;
    Variant vOwner;
    
    // Check for first contact with another empire
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    RETURN_ON_ERROR(iErrCode);
    
    if (vOwner.GetInteger() != SYSTEM && 
        vOwner.GetInteger() != INDEPENDENT && 
        vOwner.GetInteger() != iEmpireKey) {
        
        // Check for owner in diplomacy table
        iErrCode = t_pCache->GetFirstKey(
            strEmpireDip, 
            GameEmpireDiplomacy::ReferenceEmpireKey, 
            vOwner, 
            &iKey
            );
        
        if (iErrCode != ERROR_DATA_NOT_FOUND)
        {
            RETURN_ON_ERROR(iErrCode);
        }
        else
        {
            ////////////////////////////////////
            // First contact (ship to planet) //
            ////////////////////////////////////

            String strTemp;
            Variant pvColData [GameEmpireDiplomacy::NumColumns];
            
            // Add owner empire to current empire's dip screen
            pvColData[GameEmpireDiplomacy::iGameClass] = iGameClass;
            pvColData[GameEmpireDiplomacy::iGameNumber] = iGameNumber;
            pvColData[GameEmpireDiplomacy::iEmpireKey] = iEmpireKey;
            pvColData[GameEmpireDiplomacy::iReferenceEmpireKey] = vOwner.GetInteger();
            pvColData[GameEmpireDiplomacy::iDipOffer] = WAR;
            pvColData[GameEmpireDiplomacy::iCurrentStatus] = WAR;
            pvColData[GameEmpireDiplomacy::iDipOfferLastUpdate] = WAR;
            pvColData[GameEmpireDiplomacy::iState] = 0;
            pvColData[GameEmpireDiplomacy::iSubjectiveEcon] = 0;
            pvColData[GameEmpireDiplomacy::iSubjectiveMil] = 0;
            pvColData[GameEmpireDiplomacy::iLastMessageTargetFlag] = 0;
            
            iErrCode = t_pCache->InsertRow(strEmpireDip, GameEmpireDiplomacy::Template, pvColData, NULL);
            RETURN_ON_ERROR(iErrCode);
            
            // Add current empire to owner empire's dip screen
            GetEmpireIndex(k, (unsigned int)vOwner.GetInteger());
            
            pvColData[GameEmpireDiplomacy::iEmpireKey] = vOwner.GetInteger();
            pvColData[GameEmpireDiplomacy::iReferenceEmpireKey] = iEmpireKey;   // iReferenceEmpireKey
            
            iErrCode = t_pCache->InsertRow(pstrEmpireDip[k], GameEmpireDiplomacy::Template, pvColData, NULL);
            RETURN_ON_ERROR(iErrCode);
            
            // Add to empires' update messages          
            strTemp.Clear();

            AddPlanetNameAndCoordinates (strTemp, pszPlanetName, iNewX, iNewY);
            
            pstrUpdateMessage[iEmpireIndex] += "You have had " BEGIN_STRONG " first contact " END_STRONG " with " BEGIN_STRONG;
            pstrUpdateMessage[iEmpireIndex] += pvEmpireName[k].GetCharPtr();
            pstrUpdateMessage[iEmpireIndex] += END_STRONG " (ship to planet) at ";
            pstrUpdateMessage[iEmpireIndex] += strTemp;
            pstrUpdateMessage[iEmpireIndex] += "\n";
            
            pstrUpdateMessage[k] += "You have had " BEGIN_STRONG " first contact " END_STRONG " with " BEGIN_STRONG;
            pstrUpdateMessage[k] += pvEmpireName[iEmpireIndex].GetCharPtr();
            pstrUpdateMessage[k] += END_STRONG " (ship to planet) at ";
            pstrUpdateMessage[k] += strTemp;
            pstrUpdateMessage[k] += "\n";
        }

    }   // End if planet belongs to someone else

    return iErrCode;
}


int GameEngine::ProcessSubjectiveViews (int iGameClass, int iGameNumber,
                                        unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive,
                                        const char* strGameMap, const char** pstrEmpireMap, const char** pstrEmpireDip, 
                                        const char** pstrEmpireShips) {

    int iErrCode = OK;
    unsigned int i;

    for (i = 0; i < iNumEmpires; i ++)
    {
        if (!pbAlive[i]) 
            continue;

        iErrCode = ProcessEmpireSubjectiveView (iGameClass, iGameNumber,
            piEmpireKey[i], strGameMap, iNumEmpires, piEmpireKey,
            pstrEmpireMap, pstrEmpireShips, pstrEmpireMap[i], pstrEmpireDip[i]);

        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


int GameEngine::ProcessEmpireSubjectiveView (int iGameClass, int iGameNumber,
                                             unsigned int iEmpireKey, const char* pszGameMap,
                                             unsigned int iFullNumEmpires, unsigned int* piFullEmpireKey,
                                             const char** pstrEmpireMap, const char** pstrEmpireShips,
                                             const char* pszEmpireMap, const char* pszWriteEmpireDip
                                             ) {

    int iErrCode, iOwner, iPop, iMin, iFuel, iAg, iNumVisibleShips, iMyShips, iUnaccountedShips;
    unsigned int i, j, k, iNumEmpires = 0, iNumPlanets, iKey, iNumShips, iOwnerIndex = 0;

    Variant* pvPlanetKey = NULL, * pvEmpireKey = NULL;
    AutoFreeData free_pvPlanetKey(pvPlanetKey);
    AutoFreeData free_pvEmpireKey(pvEmpireKey);

    unsigned int * piProxyPlanetKey = NULL, * piProxyEmpireKey = NULL;
    AutoFreeKeys free_piProxyPlanetKey(piProxyPlanetKey);
    AutoFreeKeys free_piProxyEmpireKey(piProxyEmpireKey);

    // Get diplomacy rows
    iErrCode = t_pCache->ReadColumn(
        pszWriteEmpireDip,
        GameEmpireDiplomacy::ReferenceEmpireKey, 
        &piProxyEmpireKey, 
        &pvEmpireKey, 
        &iNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    // Allocate temp space for empire counts
    int* piAg = (int*) StackAlloc (iNumEmpires * sizeof(int) * 3);
    int* piMin = piAg + iNumEmpires;
    int* piFuel = piMin + iNumEmpires;

    float* pfMil = (float*) StackAlloc (iNumEmpires * sizeof (float));

    memset (piAg, 0, iNumEmpires * sizeof(int) * 3);
    memset (pfMil, 0, iNumEmpires * sizeof(float));

    // Loop through all visible planets
    iErrCode = t_pCache->ReadColumn(
        pszEmpireMap,
        GameEmpireMap::PlanetKey, 
        &piProxyPlanetKey, 
        &pvPlanetKey, 
        &iNumPlanets
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumPlanets; i ++)
    {
        ICachedTable* pReadTable = NULL;
        AutoRelease<ICachedTable> release_pReadTable(pReadTable);

        iErrCode = t_pCache->GetTable(pszGameMap, &pReadTable);
        RETURN_ON_ERROR(iErrCode);

        // Get planet's owner
        iErrCode = pReadTable->ReadData(pvPlanetKey[i].GetInteger(), GameMap::Owner, &iOwner);
        RETURN_ON_ERROR(iErrCode);

        switch (iOwner) {

        case SYSTEM:
        case INDEPENDENT:
            break;

        default:

            if (iOwner != (int) iEmpireKey) {

                // Find empire index
                for (j = 0; j < iNumEmpires; j ++)
                {
                    if (pvEmpireKey[j].GetInteger() == iOwner) {
                        iOwnerIndex = j;
                        break;
                    }
                }

                if (j == iNumEmpires)
                {
                    // We haven't met this empire yet
                    continue;
                }

                //
                // Add planet's resources being used to running total
                //

                // Get pop
                iErrCode = pReadTable->ReadData(pvPlanetKey[i].GetInteger(), GameMap::Pop, &iPop);
                RETURN_ON_ERROR(iErrCode);

                // Get ag
                iErrCode = pReadTable->ReadData(pvPlanetKey[i].GetInteger(), GameMap::Ag, &iAg);
                RETURN_ON_ERROR(iErrCode);

                // Add min and fuel?
                if (iPop > 0) {

                    iErrCode = pReadTable->ReadData(pvPlanetKey[i].GetInteger(), GameMap::Minerals, &iMin);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = pReadTable->ReadData(pvPlanetKey[i].GetInteger(), GameMap::Fuel, &iFuel);
                    RETURN_ON_ERROR(iErrCode);

                    piMin[iOwnerIndex] += min (iPop, iMin);
                    piFuel[iOwnerIndex] += min (iPop, iFuel);
                }

                piAg[iOwnerIndex] += iAg;
            }
        }   // End planet owner switch

        // Are there ships on the planet?
        iErrCode = pReadTable->ReadData(pvPlanetKey[i].GetInteger(), GameMap::NumUncloakedShips, &iNumVisibleShips);
        RETURN_ON_ERROR(iErrCode);

        // Don't need to worry about builds, because when this is run there are no build ships left
        Variant vTemp;
        iErrCode = t_pCache->ReadData(pszEmpireMap, piProxyPlanetKey[i], GameEmpireMap::NumUncloakedShips, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iMyShips = vTemp.GetInteger();

        if (iMyShips < iNumVisibleShips)
        {
            iUnaccountedShips = iNumVisibleShips - iMyShips;

            // Scan for unaccounted ships
            for (j = 0; j < iNumEmpires; j ++) {

                GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, pvEmpireKey[j].GetInteger());

                iErrCode = t_pCache->GetFirstKey(
                    pszEmpireMap,
                    GameEmpireMap::PlanetKey,
                    pvPlanetKey[i],
                    &iKey
                    );
                
                if (iErrCode == ERROR_DATA_NOT_FOUND)
                {
                    return OK;
                }
                RETURN_ON_ERROR(iErrCode);

                if (iKey != NO_KEY) {

                    iErrCode = t_pCache->ReadData(
                        pszEmpireMap,
                        iKey, 
                        GameEmpireMap::NumUncloakedShips, 
                        &vTemp
                        );

                    RETURN_ON_ERROR(iErrCode);

                    iNumVisibleShips = vTemp.GetInteger();

                    if (iNumVisibleShips > 0)
                    {
                        unsigned int* piShipKey = NULL;
                        AutoFreeKeys free_piShipKey(piShipKey);

                        GET_GAME_EMPIRE_SHIPS(pszEmpireShips, iGameClass, iGameNumber, pvEmpireKey[j].GetInteger());

                        // Get strength of ships at planet
                        iErrCode = t_pCache->GetEqualKeys (
                            pszEmpireShips,
                            GameEmpireShips::CurrentPlanet,
                            pvPlanetKey[i],
                            &piShipKey,
                            &iNumShips
                            );

                        RETURN_ON_ERROR(iErrCode);

                        Assert((int) iNumShips >= iNumVisibleShips);

                        // Add mil of non-cloakers
                        for (k = 0; k < iNumShips; k ++) {

                            Variant vState;

                            // Cloaked?
                            iErrCode = t_pCache->ReadData(
                                pszEmpireShips,
                                piShipKey[k],
                                GameEmpireShips::State,
                                &vState
                                );

                            RETURN_ON_ERROR(iErrCode);

                            if (!(vState.GetInteger() & CLOAKED)) {

                                Variant vBR;

                                iErrCode = t_pCache->ReadData(pszEmpireShips, piShipKey[k], GameEmpireShips::CurrentBR, &vBR);
                                RETURN_ON_ERROR(iErrCode);

                                // Add mil at last
                                pfMil[j] += vBR.GetFloat() * vBR.GetFloat();
                            }
                        }

                        iUnaccountedShips -= iNumVisibleShips;

                        if (iUnaccountedShips == 0)
                        {
                            break;
                        }
                    }
                }
            }   // End empire loop

            // At this point, there may still be unaccounted ships that belong to empires
            // who we have't met yet;  that's fine

        }
    }   // End planet loop

    // Flush results
    for (i = 0; i < iNumEmpires; i ++)
    {
        ICachedTable* pWriteEmpireDip = NULL;
        AutoRelease<ICachedTable> release_pWriteEmpireDip(pWriteEmpireDip);

        iErrCode = t_pCache->GetTable(pszWriteEmpireDip, &pWriteEmpireDip);
        RETURN_ON_ERROR(iErrCode);

        // Econ
        iErrCode = pWriteEmpireDip->WriteData(
            piProxyEmpireKey[i],
            GameEmpireDiplomacy::SubjectiveEcon,
            GetEcon (piFuel[i], piMin[i], piAg[i])
            );

        RETURN_ON_ERROR(iErrCode);

        // Mil
        iErrCode = pWriteEmpireDip->WriteData(
            piProxyEmpireKey[i],
            GameEmpireDiplomacy::SubjectiveMil,
            GetMilitaryValue (pfMil[i])
            );

        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::CreateNewPlanetFromBuilder (const GameConfiguration& gcConfig,
                                            int iGameClass, int iGameNumber, int iEmpireKey, float fBR,
                                            int iPlanetKey, int iX, int iY, int iDirection,

                                            const char* strGameMap,
                                            const char* strGameData,
                                            const char* strGameEmpireMap,
                                            const char* strEmpireDip,

                                            unsigned int* piEmpireKey,
                                            unsigned int iNumEmpires,

                                            const char** pstrEmpireMap,
                                            const char** pstrEmpireDip,
                                            const char** pstrEmpireData

                                            ) {

    Variant vDipStatus, vDipLevel;
    unsigned int iNumAcquaintances, i, j, iNewPlanetKey, iProxyKey;

    char pszCoord [MAX_COORDINATE_LENGTH + 1];
    char pszName [MAX_PLANET_NAME_LENGTH + 1];

    int iErrCode, iNewX, iNewY, iNeighbourX, iNeighbourY, iExplored = 0, piNeighbourKey [NUM_CARDINAL_POINTS], iNewAg, iNewMin, iNewFuel;

    // Name
    AdvanceCoordinates(iX, iY, &iNewX, &iNewY, iDirection);
    sprintf(pszName, "Planet %i,%i", iNewX, iNewY);

    // Coordinates
    GetCoordinates (iNewX, iNewY, pszCoord);

    // Create new planet
    Variant pvGameMap[GameMap::NumColumns] =
    {
        iGameClass,
        iGameNumber,
        pszName, //Name,
        0, //Ag(*),
        0, //Minerals(*),
        0, //Fuel(*),
        0, //Pop,
        0, //MaxPop(*),
        SYSTEM, //Owner,
        0, //Nuked,
        pszCoord, //Coordinates,
        NO_KEY, //NorthPlanetKey(*),
        NO_KEY, //EastPlanetKey(*),
        NO_KEY, //SouthPlanetKey(*),
        NO_KEY, //WestPlanetKey(*),
        OPPOSITE_LINK_X [iDirection], //Link,
        0, //PopLostToColonies,
        0, //SurrenderNumAllies,
        0, //SurrenderAlmonasterSignificance,
        NOT_HOMEWORLD, //HomeWorld,
        0, //Annihilated,
        0, //NumUncloakedShips,
        0, //NumCloakedShips,
        0, //NumUncloakedBuildShips,
        0, //NumCloakedBuildShips,
        (int64) 0, //SurrenderEmpireSecretKey,
        (float) 0.0, //SurrenderAlmonasterScore,
    };
    Assert(pvGameMap[GameMap::iName].GetCharPtr());

    // Read avg resources
    iErrCode = t_pCache->ReadData(strGameData, GameData::AvgAg, pvGameMap + GameMap::iAg);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strGameData, GameData::AvgMin, pvGameMap + GameMap::iMinerals);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strGameData, GameData::AvgFuel, pvGameMap + GameMap::iFuel);
    RETURN_ON_ERROR(iErrCode);

    // Get real resources
    GetBuilderNewPlanetResources (
        fBR, 
        gcConfig.fBuilderBRDampener,
        gcConfig.fBuilderMultiplier,
        pvGameMap[GameMap::iAg].GetInteger(),
        pvGameMap[GameMap::iMinerals].GetInteger(),
        pvGameMap[GameMap::iFuel].GetInteger(),
        &iNewAg,
        &iNewMin,
        &iNewFuel
        );

    pvGameMap[GameMap::iAg] = iNewAg;
    pvGameMap[GameMap::iMinerals] = iNewMin;
    pvGameMap[GameMap::iFuel] = iNewFuel;

    // MaxPop
    pvGameMap[GameMap::iMaxPop] = GetMaxPop(pvGameMap[GameMap::iMinerals].GetInteger(), pvGameMap[GameMap::iFuel].GetInteger());

    // Surrounding keys
    ENUMERATE_CARDINAL_POINTS(i) {

        if (i == (unsigned int) OPPOSITE_CARDINAL_POINT [iDirection]) {

            pvGameMap[GameMap::iNorthPlanetKey + i] = piNeighbourKey[i] = iPlanetKey;
        
        } else {

            AdvanceCoordinates (iNewX, iNewY, &iNeighbourX, &iNeighbourY, i);

            iErrCode = GetPlanetKeyFromCoordinates (
                iGameClass, 
                iGameNumber, 
                iNeighbourX, 
                iNeighbourY, 
                piNeighbourKey + i
                );

            RETURN_ON_ERROR(iErrCode);
        }

        pvGameMap[GameMap::iNorthPlanetKey + i] = piNeighbourKey[i];

        if (piNeighbourKey[i] != NO_KEY) {
            
            // Have we explored our neighbour?
            iErrCode = t_pCache->GetFirstKey(
                strGameEmpireMap,
                GameEmpireMap::PlanetKey,
                piNeighbourKey[i],
                &iProxyKey
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                return OK;
            }
            else
            {
                RETURN_ON_ERROR(iErrCode);
            
                iExplored |= EXPLORED_X[i];
                
                // Set neighbour's explored bit
                iErrCode = t_pCache->WriteOr(strGameEmpireMap, iProxyKey, GameEmpireMap::Explored, OPPOSITE_EXPLORED_X[i]);
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    // Insert row into game map
    iErrCode = t_pCache->InsertRow(strGameMap, GameMap::Template, pvGameMap, &iNewPlanetKey);
    RETURN_ON_ERROR(iErrCode);
    
    // Set link planet's link bit
    iErrCode = t_pCache->WriteOr(
        strGameMap,
        iPlanetKey,
        GameMap::Link,
        LINK_X[iDirection]
        );
    
    RETURN_ON_ERROR(iErrCode);
    
    // Set neighbours' cardinal point keys
    ENUMERATE_CARDINAL_POINTS(i) {

        if (piNeighbourKey[i] != NO_KEY)
        {
            iErrCode = t_pCache->WriteData(
                strGameMap,
                piNeighbourKey[i],
                GameMap::ColumnNames[GameMap::iNorthPlanetKey + OPPOSITE_CARDINAL_POINT[i]],
                (int)iNewPlanetKey
                );
            
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Fix empire's min / max
    iErrCode = SetNewMinMaxIfNecessary (iGameClass, iGameNumber, iEmpireKey, iNewX, iNewY);
    RETURN_ON_ERROR(iErrCode);

    // Fix game's min / max
    iErrCode = SetNewMinMaxIfNecessary (iGameClass, iGameNumber, NO_KEY, iNewX, iNewY);
    RETURN_ON_ERROR(iErrCode);

    Variant pvGameEmpireMap[GameEmpireMap::NumColumns] =
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iNewPlanetKey, //PlanetKey,
        iExplored, //Explored,
        0, //NumUncloakedShips,
        0, //NumCloakedBuildShips,
        0, //NumUncloakedBuildShips,
        0, //NumCloakedShips
    };
        
    iErrCode = t_pCache->InsertRow(strGameEmpireMap, GameEmpireMap::Template, pvGameEmpireMap, NULL);
    RETURN_ON_ERROR(iErrCode);

    // If mapshare, add new planet to fellow sharer's maps
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MapsShared, 
        &vDipLevel
        );

    RETURN_ON_ERROR(iErrCode);
    
    if (vDipLevel != NO_DIPLOMACY)
    {
        Variant* pvAcquaintanceKey = NULL;
        AutoFreeData free_pvAcquaintanceKey(pvAcquaintanceKey);

        unsigned int* piProxyKey = NULL;
        AutoFreeKeys free_piProxyKey(piProxyKey);

        iErrCode = t_pCache->ReadColumn(
            strEmpireDip, 
            GameEmpireDiplomacy::ReferenceEmpireKey, 
            &piProxyKey, 
            &pvAcquaintanceKey, 
            &iNumAcquaintances
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);
            
            for (i = 0; i < iNumAcquaintances; i ++) {
                
                iErrCode = t_pCache->ReadData(
                    strEmpireDip, 
                    piProxyKey[i], 
                    GameEmpireDiplomacy::CurrentStatus, 
                    &vDipStatus
                    );

                RETURN_ON_ERROR(iErrCode);
                
                if (vDipStatus.GetInteger() >= vDipLevel.GetInteger()) {
                    
                    GetEmpireIndex (j, pvAcquaintanceKey[i]);
                    
                    iErrCode = SharePlanetBetweenFriends (
                        iGameClass,
                        iGameNumber,
                        iNewPlanetKey,
                        j,
                        pstrEmpireMap,
                        pstrEmpireDip,
                        pstrEmpireData,
                        strGameMap,
                        iNumEmpires,
                        piEmpireKey, 
                        vDipLevel.GetInteger(),
                        NULL,
                        NULL,
                        -1,
                        true
                        );
                    
                    RETURN_ON_ERROR(iErrCode);
                }
            }   // End acquaintance loop

        }   // End if acquaintances > 0

    }   // End if mapshare

    return iErrCode;
}

int GameEngine::ChangeShipTypeOrMaxBR (const char* pszShips, const char* pszEmpireData, 
                                       int iEmpireKey, int iShipKey, int iOldShipType, int iNewShipType, 
                                       float fBRChange) {

    int iErrCode, iOld, iNew;
    float fNewMaxBR, fOldMaxBR;
    Variant vTemp;

    const char* pszCurrentBRColumn, * pszMaxBRColumn, * pszTypeColumn;

    if (iEmpireKey == INDEPENDENT) {

        pszCurrentBRColumn = GameEmpireShips::CurrentBR;
        pszMaxBRColumn = GameEmpireShips::MaxBR;
        pszTypeColumn = GameEmpireShips::Type;

    } else {

        pszCurrentBRColumn = GameEmpireShips::CurrentBR;
        pszMaxBRColumn = GameEmpireShips::MaxBR;
        pszTypeColumn = GameEmpireShips::Type;
    }

    if (iOldShipType != iNewShipType) {

        // Set new type
        iErrCode = t_pCache->WriteData(pszShips, iShipKey, pszTypeColumn, iNewShipType);
        RETURN_ON_ERROR(iErrCode);

        // Handle removal from fleet if not mobile ship
        if (iEmpireKey != INDEPENDENT && IsMobileShip (iOldShipType) && !IsMobileShip (iNewShipType)) {

            iErrCode = t_pCache->WriteData(pszShips, iShipKey, GameEmpireShips::FleetKey, (int)NO_KEY);
            RETURN_ON_ERROR(iErrCode);

            // UpdateFleetOrders will handle the fleet's accounting
        }
    }

    if (fBRChange != 0.0) {
    
        // Change ship stats
        iErrCode = t_pCache->Increment (pszShips, iShipKey, pszCurrentBRColumn, fBRChange);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->Increment (pszShips, iShipKey, pszMaxBRColumn, fBRChange, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        fOldMaxBR = vTemp.GetFloat();
    
    } else {

        iErrCode = t_pCache->ReadData(pszShips, iShipKey, pszMaxBRColumn, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        fOldMaxBR = vTemp.GetFloat();
    }

    if (iEmpireKey != INDEPENDENT) {

        fNewMaxBR = fOldMaxBR + fBRChange;

        // Change empire maintenance
        iOld = GetMaintenanceCost (iOldShipType, fOldMaxBR);
        iNew = GetMaintenanceCost (iNewShipType, fNewMaxBR);

        iErrCode = t_pCache->Increment (pszEmpireData, GameEmpireData::TotalMaintenance, iNew - iOld);
        RETURN_ON_ERROR(iErrCode);

        // Change empire fuel use
        iOld = GetFuelCost (iOldShipType, fOldMaxBR);
        iNew = GetFuelCost (iNewShipType, fNewMaxBR);

        iErrCode = t_pCache->Increment (pszEmpireData, GameEmpireData::TotalFuelUse, iNew - iOld);
        RETURN_ON_ERROR(iErrCode);

#ifdef _DEBUG
        Variant vTotalFuelUse;
        iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::TotalFuelUse, &vTotalFuelUse);
        RETURN_ON_ERROR(iErrCode);
        Assert(vTotalFuelUse.GetInteger() >= 0);

        Variant vCurrentBR;
        iErrCode = t_pCache->ReadData(pszShips, iShipKey, GameEmpireShips::CurrentBR, &vCurrentBR);
        RETURN_ON_ERROR(iErrCode);
        Assert(vCurrentBR.GetFloat() > 0.0);
#endif

    }

    return iErrCode;
}