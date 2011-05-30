//
// GameEngine.dll:  a component of Almonaster 2.x
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "Osal/Algorithm.h"

#include <math.h>

#define GetEmpireIndex(j, i) for (j = 0; j < (int) iNumEmpires; j ++) { if (piEmpireKey[j] == i) { break; }} Assert (j < iNumEmpires);

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

int GameEngine::RunUpdate (int iGameClass, int iGameNumber, const UTCTime& tUpdateTime, bool* pbGameOver) {

    int iResult = OK, iErrCode;
    
    *pbGameOver = false;

    // Strings
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    char strIndependentShips [256];

    String strMessage;
    Variant vTemp, vBR, vShipName, vGameState;
    int iGameClassOptions;

    GameConfiguration gcConfig;
    
    iErrCode = GetGameConfiguration (&gcConfig);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Get gameclass name
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    if (GetGameClassName (iGameClass, pszGameClassName) != OK) {
        StrNCpy (pszGameClassName, "Unknown gameclass");
    }

    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    iGameClassOptions = vTemp.GetInteger();

    bool bIndependence;

    if (iGameClassOptions & INDEPENDENCE) {
        bIndependence = true;
        GET_GAME_INDEPENDENT_SHIPS (strIndependentShips, iGameClass, iGameNumber);
    } else {
        bIndependence = false;
    }

    unsigned int* piNukedPlanetKey, ** ppiShipNukeKey = NULL, ** ppiEmpireNukeKey, * piNumNukingShips, 
        iNumNukedPlanets = 0, i, j, * piPlanetKey = NULL, iNumPlanets;

    Variant vUpdatePeriod, vLastUpdateTime;

    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::NumSecPerUpdate, 
        &vUpdatePeriod
        );
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (strGameData, GameData::LastUpdateTime, &vLastUpdateTime);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    ////////////////////////////////
    // Initialize data for update //
    ////////////////////////////////

    // Get list of empire keys
    bool* pbAlive, * pbSendFatalMessage;
    unsigned int* piEmpireKey, iNumEmpires, iTemp;
    
    {
        Variant* pvEmpireKey;
        iErrCode = m_pGameData->ReadColumn (strGameEmpires, GameEmpires::EmpireKey, &pvEmpireKey, &iNumEmpires);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        if (iNumEmpires == 0) {

            *pbGameOver = true;
            iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
            Assert (iErrCode == OK);

            return iResult;
        }
        
        // Initialize boolean alive array
        pbAlive = (bool*) StackAlloc (iNumEmpires * sizeof (bool) * 2);
        pbSendFatalMessage = pbAlive + iNumEmpires;
        piEmpireKey = (unsigned int*) StackAlloc (iNumEmpires * sizeof (unsigned int));
        
        for (i = 0; i < iNumEmpires; i ++) {

            piEmpireKey[i] = pvEmpireKey[i].GetInteger();
            pbAlive[i] = true;

            iErrCode = GetEmpireOption (piEmpireKey[i], DISPLAY_FATAL_UPDATE_MESSAGES, pbSendFatalMessage + i);
            if (iErrCode != OK) {
                Assert (false);
                m_pGameData->FreeData (pvEmpireKey);
                return iErrCode;
            }
        }

        m_pGameData->FreeData (pvEmpireKey);
    }

    // Randomize empire list so all order-dependent events are unpredictable
    Algorithm::Randomize<unsigned int> (piEmpireKey, iNumEmpires);

    // Read number of updates transpired so far
    Variant vNumUpdates;
    iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    int iNewUpdateCount = vNumUpdates.GetInteger() + 1;
    
    // Read max tech increase for game
    Variant vGameMaxTechIncrease;
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MaxTechDev, 
        &vGameMaxTechIncrease
        );

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

    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vGameState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    ///////////////////////////////
    // Generate map if necessary //
    ///////////////////////////////

    bool bMapGenerated = false;

    if (iNewUpdateCount == 1 && 
        (iGameClassOptions & GENERATE_MAP_FIRST_UPDATE) &&
        !(vGameState.GetInteger() & GAME_MAP_GENERATED)) {

        bool bCommit;

        iErrCode = AddEmpiresToMap (iGameClass, iGameNumber, (int*) piEmpireKey, iNumEmpires, &bCommit);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_MAP_GENERATED);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        vGameState = vGameState.GetInteger() | GAME_MAP_GENERATED;
        bMapGenerated = true;
    }

    Assert (vGameState.GetInteger() & GAME_MAP_GENERATED);

    //
    // Initialize empire's table names - big stack alloc!
    //

#define TABLE_LENGTH 192
#define NUM_TABLES 5

    unsigned int iNumTables = iNumEmpires * NUM_TABLES;

    char* pszBuffer = (char*) StackAlloc (iNumTables * TABLE_LENGTH * sizeof (char));
    char** ppszPointers = (char**) StackAlloc (iNumTables * sizeof (char**));

    for (i = 0; i < iNumTables; i ++) {
        ppszPointers[i] = pszBuffer + i * TABLE_LENGTH;
    }

    const char** pstrEmpireData = (const char**) ppszPointers;
    const char** pstrEmpireDip = pstrEmpireData + iNumEmpires;
    const char** pstrEmpireShips = pstrEmpireDip + iNumEmpires;
    const char** pstrEmpireFleets = pstrEmpireShips + iNumEmpires;
    const char** pstrEmpireMap = pstrEmpireFleets + iNumEmpires;

    String* pstrUpdateMessage = new String [iNumEmpires];

    Variant* pvGoodColor = new Variant [iNumEmpires * 3];
    Variant* pvBadColor = pvGoodColor + iNumEmpires;
    Variant* pvEmpireName = pvBadColor + iNumEmpires;

    int* piTotalMin = (int*) StackAlloc (iNumEmpires * 10 * sizeof (int));
    int* piTotalFuel = piTotalMin + iNumEmpires;
    int* piTotalAg = piTotalFuel + iNumEmpires;

    int* piBonusMin = piTotalAg + iNumEmpires;
    int* piBonusFuel = piBonusMin + iNumEmpires;
    int* piBonusAg = piBonusFuel + iNumEmpires;

    int* piObliterator = piBonusAg + iNumEmpires;
    int* piObliterated = piObliterator + iNumEmpires;

    int* piWinner = piObliterated + iNumEmpires;
    int* piLoser = piWinner + iNumEmpires;

    float* pfMaintRatio = (float*) StackAlloc (iNumEmpires * 3 * sizeof (float));
    float* pfAgRatio = pfMaintRatio + iNumEmpires;
    float* pfFuelRatio = pfAgRatio + iNumEmpires;

    float fTechDelta;

    unsigned int iNumObliterations = 0, iNumSurrenders = 0, iNumRuins = 0, iNumNewDraws = 0,
        * piOriginalPlanetOwner = NULL, * piOriginalNumObliterations;

    Variant vMaxBR, vTechLevel, vTechDevs, vNumOwnShips, vMaint, vBuild, vFuel, vTheme;
    int iBR1, iBR2;

    char pszUpdateTime [OS::MaxDateLength];

    if (pstrUpdateMessage == NULL || pvGoodColor == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    for (i = 0; i < iNumEmpires; i ++) {
        
        // Name
        iErrCode = m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            piEmpireKey[i], 
            SystemEmpireData::Name, 
            &pvEmpireName[i]
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Good color
        iErrCode = m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            piEmpireKey[i], 
            SystemEmpireData::AlmonasterTheme, 
            &vTheme
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        switch (vTheme.GetInteger()) {
            
        case INDIVIDUAL_ELEMENTS:
        case ALTERNATIVE_PATH:

            iErrCode = m_pGameData->ReadData (
                SYSTEM_EMPIRE_DATA, 
                piEmpireKey[i], 
                SystemEmpireData::UIColor, 
                &vFuel
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            switch (vFuel.GetInteger()) {
                
            case CUSTOM_COLORS:
                
                iErrCode = GetEmpireProperty (piEmpireKey[i], SystemEmpireData::CustomGoodColor, pvGoodColor + i);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iErrCode = GetEmpireProperty (piEmpireKey[i], SystemEmpireData::CustomBadColor, pvBadColor + i);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                break;
                
            case NULL_THEME:
                
                pvGoodColor[i] = DEFAULT_GOOD_COLOR;
                pvBadColor[i] = DEFAULT_BAD_COLOR;
                break;
                
            default:
                
                iErrCode = GetThemeGoodColor (vFuel.GetInteger(), pvGoodColor + i);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = GetThemeBadColor (vFuel.GetInteger(), pvBadColor + i);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                break;
            }

            break;
        
        case NULL_THEME:

            pvGoodColor[i] = DEFAULT_GOOD_COLOR;
            pvBadColor[i] = DEFAULT_BAD_COLOR;
            break;

        default:
            
            iErrCode = m_pGameData->ReadData (
                SYSTEM_THEMES, 
                vTheme.GetInteger(),
                SystemThemes::GoodColor, 
                pvGoodColor + i
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (
                SYSTEM_THEMES, 
                vTheme.GetInteger(),
                SystemThemes::BadColor, 
                pvBadColor + i
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            break;
        }
        
        iErrCode = m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            piEmpireKey[i], 
            SystemEmpireData::Name, 
            pvEmpireName + i
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        GET_GAME_EMPIRE_DATA ((char*) pstrEmpireData[i], iGameClass, iGameNumber, piEmpireKey[i]);
        GET_GAME_EMPIRE_DIPLOMACY ((char*) pstrEmpireDip[i], iGameClass, iGameNumber, piEmpireKey[i])
        GET_GAME_EMPIRE_SHIPS ((char*) pstrEmpireShips[i], iGameClass, iGameNumber, piEmpireKey[i])
        GET_GAME_EMPIRE_FLEETS ((char*) pstrEmpireFleets[i], iGameClass, iGameNumber, piEmpireKey[i])
        GET_GAME_EMPIRE_MAP ((char*) pstrEmpireMap[i], iGameClass, iGameNumber, piEmpireKey[i]);

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalMin, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        piTotalMin[i] = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalBuild, &vBuild);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalMaintenance, &vMaint);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalFuelUse, &vFuel);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalFuel, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        piTotalFuel[i] = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalAg, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        piTotalAg[i] = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::BonusFuel, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        piBonusFuel[i] = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::BonusAg, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        piBonusAg[i] = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::BonusMin, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        piBonusMin[i] = vTemp.GetInteger();

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalPop, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        pfMaintRatio[i] = GetMaintenanceRatio (piTotalMin[i] + piBonusMin[i], vMaint, vBuild);
        Assert (pfMaintRatio[i] >= 0.0);

        pfAgRatio[i] = GetAgRatio (piTotalAg[i] + piBonusAg[i], vTemp.GetInteger(), vMaxAgRatio.GetFloat());
        Assert (pfAgRatio[i] >= 0.0);

        pfFuelRatio[i] = GetFuelRatio (piTotalFuel[i] + piBonusFuel[i], vFuel);
        Assert (pfFuelRatio[i] >= 0.0);

        // Initial update messages
        iErrCode = Time::GetDateString (tUpdateTime, pszUpdateTime);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

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

        iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::TechLevel, fTechDelta, &vTechLevel);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Handle new tech developments
        iBR1 = GetBattleRank (vTechLevel.GetFloat());
        iBR2 = GetBattleRank (vTechLevel.GetFloat() + fTechDelta);

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::MaxBR, &vMaxBR);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Did we develop a new tech?
        if (iBR1 != iBR2 && vMaxBR.GetInteger() < iBR2) {
            
            /// Increment maxBR
            iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::MaxBR, iBR2);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Update message
            pstrUpdateMessage[i] += BEGIN_GOOD_FONT(i) "You have reached " BEGIN_STRONG "BR ";
            pstrUpdateMessage[i] += iBR2;
            pstrUpdateMessage[i] += END_STRONG END_FONT "\n";
            
            // If a tech remains to be developed, increment the number available
            iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TechUndevs, &vTechDevs);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (vTechDevs.GetInteger() != 0) {
                iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumAvailableTechUndevs, 
                    iBR2 - iBR1);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }

        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::NumAvailableTechUndevs, &vTechDevs);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

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


    // Add mep generation to update messages
    if (bMapGenerated) {

        for (i = 0; i < iNumEmpires; i ++) {
            pstrUpdateMessage[i] += "The map was generated\n";
        }
    }


    ////////////////////////////////////////////
    // Close game if numupdates is sufficient //
    ////////////////////////////////////////////
    
    if (vGameState.GetInteger() & STILL_OPEN) {
        
        iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdatesBeforeGameCloses, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        if (iNewUpdateCount >= vTemp.GetInteger()) {
            
            // Close game
            iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~STILL_OPEN);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
            GetGameClassGameNumber (iGameClass, iGameNumber, pszData);
            
            // Mark closed in game list
            iErrCode = m_pGameData->GetFirstKey (
                SYSTEM_ACTIVE_GAMES,
                SystemActiveGames::GameClassGameNumber,
                pszData,
                true,
                (unsigned int*) &iTemp
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            Assert (iTemp != NO_KEY);
            
            iErrCode = m_pGameData->WriteData (SYSTEM_ACTIVE_GAMES, iTemp, SystemActiveGames::State, 0);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Add to update messages
            for (i = 0; i < iNumEmpires; i ++) {
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

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ////////////////////////////////////////////////////
    // Check for ally out, draw out if game is closed //
    ////////////////////////////////////////////////////

    if (!(vGameState.GetInteger() & STILL_OPEN)) {

        iErrCode = CheckGameForEndConditions (iGameClass, iGameNumber, NULL, pbGameOver);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (*pbGameOver) {
            goto Cleanup;
        }   
    }


    ///////////////////////////////
    // Update planet populations //
    ///////////////////////////////
    
    iErrCode = m_pGameData->GetAllKeys (strGameMap, &piPlanetKey, &iNumPlanets);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    Assert (piPlanetKey [iNumPlanets - 1] == iNumPlanets - 1);

    // Hit the heap for stack safety
    piOriginalPlanetOwner = new unsigned int [iNumPlanets * 2];
    if (piOriginalPlanetOwner == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    piOriginalNumObliterations = piOriginalPlanetOwner + iNumPlanets;

    memset (piOriginalPlanetOwner, NO_KEY, iNumPlanets * sizeof (unsigned int));
    memset (piOriginalNumObliterations, ANNIHILATED_UNKNOWN, iNumPlanets * sizeof (unsigned int));

    iErrCode = UpdatePlanetPopulations (iNumEmpires, piEmpireKey, pbAlive, pfAgRatio, strGameMap, 
        pstrEmpireData, pstrEmpireMap, pstrUpdateMessage, piPlanetKey, iNumPlanets, piTotalMin, piTotalFuel,
        pvGoodColor, pvBadColor, vMaxAgRatio.GetFloat());

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Make ships move and update their current BR's as per maintenance ratio //
    ////////////////////////////////////////////////////////////////////////////

    iErrCode = MoveShips (iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, pvEmpireName, 
        pfMaintRatio, strGameMap, pstrEmpireShips, pstrEmpireDip, pstrEmpireMap, pstrEmpireFleets, 
        pstrEmpireData, pstrUpdateMessage, pvGoodColor, pvBadColor, gcConfig, iGameClassOptions);

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // Make ships fight.  Check for Minefields and first contact with opponent's ships //
    /////////////////////////////////////////////////////////////////////////////////////

    iErrCode = MakeShipsFight (iGameClass, iGameNumber, strGameMap, iNumEmpires, piEmpireKey, pbAlive, 
        pvEmpireName, pstrEmpireShips, pstrEmpireDip, pstrEmpireMap, pstrUpdateMessage, pstrEmpireData, 
        pfFuelRatio, iNumPlanets, piPlanetKey, piTotalMin, piTotalFuel, bIndependence, strIndependentShips,
        pvGoodColor, pvBadColor, gcConfig);

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ///////////////////////////////////////////
    // Make minefields set to detonate do so //
    ///////////////////////////////////////////

    iErrCode = MakeMinefieldsDetonate (iGameClass, iGameNumber, strGameMap, iNumEmpires, piEmpireKey, pbAlive, 
        pvEmpireName, pstrEmpireShips, pstrEmpireMap, pstrUpdateMessage, pstrEmpireData, 
        piTotalMin, piTotalFuel, bIndependence, strIndependentShips, pvGoodColor, pvBadColor, gcConfig);

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ////////////////////////////////////////////////
    // Make ships/fleets perform special actions  //
    ////////////////////////////////////////////////

    // Array of keys of nuking ships
    ppiShipNukeKey = (unsigned int**) StackAlloc (iNumPlanets * 2 * sizeof (unsigned int*));

    // Array of empires that own the nuking ships
    ppiEmpireNukeKey = ppiShipNukeKey + iNumPlanets;
    
    // Number of nuking ships per planet
    piNumNukingShips = (unsigned int*) StackAlloc (iNumPlanets * 2 * sizeof (unsigned int));

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

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ///////////////////////////
    // Handle fleet movement //
    ///////////////////////////

    iErrCode = UpdateFleetOrders (iNumEmpires, piEmpireKey, pbAlive, strGameMap, pstrEmpireShips, 
        pstrEmpireFleets, pstrEmpireMap, pstrUpdateMessage);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    /////////////////////
    // Make gates work //
    /////////////////////

    iErrCode = ProcessGates (iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, 
        pstrUpdateMessage, pvGoodColor, pvBadColor, pvEmpireName, piOriginalPlanetOwner,
        piOriginalNumObliterations, pstrEmpireShips, pstrEmpireFleets, 
        pstrEmpireMap, pstrEmpireData, pstrEmpireDip, strGameMap, gcConfig, iGameClassOptions);
    
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

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

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ////////////////////////
    // Add ship sightings //
    ////////////////////////

    iErrCode = AddShipSightings (iNumEmpires, piEmpireKey, pbAlive, pstrUpdateMessage, pvEmpireName, 
        bIndependence, iNumPlanets, piPlanetKey, strGameMap, pstrEmpireMap, pstrEmpireShips, 
        strIndependentShips);

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    /////////////////////////////////////////////////////////////////////////////
    // Update empires' econ, mil, maxecon, maxmil, totalbuild, end turn status //
    /////////////////////////////////////////////////////////////////////////////

    iErrCode = UpdateEmpiresEcon (iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, piTotalMin, 
        piTotalFuel, piTotalAg, vUpdatePeriod, tUpdateTime, strGameData, pstrEmpireDip, pstrEmpireData, 
        pstrEmpireShips, iNewUpdateCount, strGameMap, vMaxAgRatio.GetFloat(),
        gcConfig);

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    //////////////////////////////
    // Process subjective views //
    //////////////////////////////

    if (iGameClassOptions & SUBJECTIVE_VIEWS) {
        
        iErrCode = ProcessSubjectiveViews (
            iGameClass, iGameNumber, iNumEmpires, piEmpireKey, pbAlive, strGameMap, pstrEmpireMap, 
            pstrEmpireDip, pstrEmpireShips
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    ////////////////////////////////////////////////
    // Check for one player left in a closed game //
    ////////////////////////////////////////////////

    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vGameState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (!(vGameState.GetInteger() & STILL_OPEN)) {
        
        iErrCode = m_pGameData->GetNumRows (strGameEmpires, (unsigned int*) &iTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        if (iTemp < 2) {
            
            //////////////////////////////////
            // Game over, we have a winner! //
            //////////////////////////////////

            // Determine which empire is still alive
            for (j = 0; j < iNumEmpires; j ++) {
                if (pbAlive[j]) {
                    break;
                }
            }

            if (j < iNumEmpires) {
                
                // Send victory message to the remaining empire
                char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];
                sprintf (
                    pszMessage, 
                    "Congratulations! You have won %s %i", 
                    pszGameClassName,
                    iGameNumber
                    );
                
                iErrCode = SendSystemMessage (piEmpireKey[j], pszMessage, SYSTEM, MESSAGE_SYSTEM);
                if (iErrCode != OK) {
                    Assert (false);
                    //goto Cleanup;
                }
                
                // Add win to empires' statistics
                iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, piEmpireKey[j]);
                if (iErrCode != OK) {
                    Assert (false);
                    //goto Cleanup;
                }
            }

            else Assert (false);

            *pbGameOver = true;
            
            // Game over
            iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_WIN);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            goto Cleanup;
        }
    }

    //////////////
    // Idleness //
    //////////////
    {
        unsigned int iNumUpdatedEmpires = 0, iSurvivorIndex = NO_KEY, iGameRuinIdlers = 0, 
            iAwake = 0, iNumIdleEmpires = 0;
        Variant vRuinFlags;

        bool* pbNewlyIdle = (bool*) StackAlloc (iNumEmpires * sizeof (bool));

        int* piRuinEmpire = (int*) StackAlloc (iNumEmpires * sizeof (int));
        bool* pbRuinEmpireUpdated = (bool*) StackAlloc (iNumEmpires * sizeof (bool));

        int iNumUpdatesIdle, iNumIdleUpdatesForRuin, iNumUpdatesForIdle, iGameState;

        memset (pbRuinEmpireUpdated, false, iNumEmpires * sizeof (bool));
        memset (pbNewlyIdle, false, iNumEmpires * sizeof (bool));

        // Get idle updates for idle
        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA,
            iGameClass,
            SystemGameClassData::NumUpdatesForIdle,
            &vTemp
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iNumUpdatesForIdle = vTemp.GetInteger();

        // Get idle updates for ruin
        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA,
            iGameClass,
            SystemGameClassData::NumUpdatesForRuin,
            &vTemp
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iNumIdleUpdatesForRuin = vTemp.GetInteger();

        // Get ruin behavior
        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA,
            iGameClass,
            SystemGameClassData::RuinFlags,
            &vRuinFlags
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        bool bSimpleRuins = vRuinFlags.GetInteger() == RUIN_CLASSIC_SC;
        bool bComplexRuins = vRuinFlags.GetInteger() == RUIN_ALMONASTER;

        Assert (bSimpleRuins || bComplexRuins || vRuinFlags.GetInteger() == 0);

        bool bRuinGame = false;

        char pszDead [128 + MAX_EMPIRE_NAME_LENGTH];
        char pszMessage [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH] = "";
        
        for (i = 0; i < iNumEmpires; i ++) {

            if (!pbAlive[i]) {
                continue;
            }

            // Get empire options
            iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::Options, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            int iEmpireOptions = vTemp.GetInteger();

            if (iEmpireOptions & RESIGNED) {

                iErrCode = m_pGameData->WriteOr (pstrEmpireData[i], GameEmpireData::Options, UPDATED);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iGameRuinIdlers ++;
                iNumUpdatedEmpires ++;
                iNumIdleEmpires ++;
            }

            else if (iNewUpdateCount > 1 && !(iEmpireOptions & LOGGED_IN_THIS_UPDATE)) {

                iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumUpdatesIdle, 1, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                iNumUpdatesIdle = vTemp.GetInteger() + 1;

                if (iNumUpdatesIdle >= NUM_UPDATES_FOR_GAME_RUIN || (iEmpireOptions & RESIGNED)) {
                    iGameRuinIdlers ++;
                }

                // Check for idle empire
                if (iNumUpdatesIdle >= iNumUpdatesForIdle || (iEmpireOptions & RESIGNED)) {

                    iErrCode = m_pGameData->WriteOr (pstrEmpireData[i], GameEmpireData::Options, UPDATED);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    iNumUpdatedEmpires ++;
                    iNumIdleEmpires ++;

                    if (iNumUpdatesIdle == iNumUpdatesForIdle) {
                        pbNewlyIdle[i] = true;
                    }

                } else {

                    // The empire is only mildly idle
                    if (iEmpireOptions & UPDATED) {

                        iErrCode = m_pGameData->WriteAnd (pstrEmpireData[i], GameEmpireData::Options, ~UPDATED);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                    }

                    iAwake ++;
                }

                // Check for simple ruin
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
                }

            } else {

                // Set num updates idle to zero
                iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::NumUpdatesIdle, 0);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                // Turn off the logged-in-this update flag
                iErrCode = m_pGameData->WriteAnd (pstrEmpireData[i], GameEmpireData::Options, ~LOGGED_IN_THIS_UPDATE);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iSurvivorIndex = i;
                iAwake ++;

                if (iEmpireOptions & UPDATED) {

                    iErrCode = m_pGameData->WriteAnd (pstrEmpireData[i], GameEmpireData::Options, ~UPDATED);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
            }
        }   // End empire loop

        // Complex ruins:
        // Only ruin game with one empire alive if at one point in the game there were more than two
        if (bComplexRuins && iAwake < 2) {

            iErrCode = m_pGameData->ReadData (strGameData, GameData::MaxNumEmpires, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (vTemp.GetInteger() > 2) {
                bRuinGame = true;
            }
        }

        // Check for end game because of ruins
        if (
            (bComplexRuins && bRuinGame) ||                     // Complex game ruin 
            (iGameRuinIdlers == iNumEmpires) ||                 // All empires idle for ruin
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
                if (iErrCode != OK) {
                    Assert (false);
                    //goto Cleanup;
                }
                
                iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, piEmpireKey[iSurvivorIndex]);
                if (iErrCode != OK) {
                    Assert (false);
                    //goto Cleanup;
                }
                
                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[iSurvivorIndex], EMPIRE_GAME_ENDED, NULL);
                if (iErrCode != OK) {
                    Assert (false);
                    //goto Cleanup;
                }
            }
            
            // Ruin all remaining empires
            const char* pszWinnerName = NULL;
            if (iSurvivorIndex != NO_KEY) {
                pszWinnerName = pvEmpireName[iSurvivorIndex].GetCharPtr();
            }

            iErrCode = RuinGame (iGameClass, iGameNumber, pszWinnerName);
            if (iErrCode != OK) {
                Assert (false);
            }
            
            // Game over
            *pbGameOver = true;

            goto Cleanup;
        }

        // The game lives on, so process ruined empires
        if (iNumRuins > 0) {

            GameUpdateInformation guInfo = {
                iNewUpdateCount,
                iNumEmpires,
                (int*) piEmpireKey,
                pbAlive,
                pstrUpdateMessage
            };

            sprintf (pszMessage, "You ruined out of %s %i", pszGameClassName, iGameNumber);
            for (i = 0; i < iNumRuins; i ++) {

                int iIndex = piRuinEmpire[i];
                
                iErrCode = RuinEmpire (iGameClass, iGameNumber, piEmpireKey[iIndex], pszMessage);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                pbAlive[iIndex] = false;
                pbSendFatalMessage[iIndex] = false; // Don't send on ruin
            }

            // Delete ruined empires from game
            for (i = 0; i < iNumRuins; i ++) {

                int iIndex = piRuinEmpire[i];
                
                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[iIndex], EMPIRE_RUINED, &guInfo);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

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

        }   // End if ruins occurred

        // Newly idle empires request pause and draw
        for (i = 0; i < iNumEmpires; i ++) {

            if (!pbNewlyIdle[i] || !pbAlive[i]) {
                continue;
            }

            if (iGameClassOptions & ALLOW_DRAW) {

                bool bDraw;
                iErrCode = IsEmpireRequestingDraw (iGameClass, iGameNumber, piEmpireKey[i], &bDraw);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (!bDraw) {

                    iErrCode = RequestDraw (iGameClass, iGameNumber, piEmpireKey[i], &iGameState);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iNumNewDraws ++;
                }
            }

            bool bNewPause;
            iErrCode = RequestPauseQuietly (iGameClass, iGameNumber, piEmpireKey[i], &bNewPause, &iGameState);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (bNewPause && (iGameState & PAUSED)) {
                
                for (j = 0; j < iNumEmpires; j ++) {
                    if (pbAlive[j]) {
                        pstrUpdateMessage[j] += "The game is now paused\n";
                    }
                }

                // No reason to continue scanning empires
                break;
            }
        }

        // Set updated empires
        iErrCode = m_pGameData->WriteData (strGameData, GameData::NumEmpiresUpdated, iNumUpdatedEmpires);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

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
    }


    /////////////////////////////////////////////////////////////////////////////
    // Check for ally out and draw out if someone died and the game has closed //
    /////////////////////////////////////////////////////////////////////////////

    if (!(vGameState.GetInteger() & STILL_OPEN) && iNumObliterations + iNumRuins + iNumNewDraws > 0) {

        iErrCode = CheckGameForEndConditions (iGameClass, iGameNumber, NULL, pbGameOver);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (*pbGameOver) {
            goto Cleanup;
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
    Assert (iErrCode == OK);
    iErrCode = OK;
#endif

    /////////////////////////////////
    // Increment number of updates //
    /////////////////////////////////
    
    iErrCode = m_pGameData->Increment (strGameData, GameData::NumUpdates, 1);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    /////////////////////////////
    // Update last update time //
    /////////////////////////////
    
    iErrCode = m_pGameData->WriteData (strGameData, GameData::LastUpdateTime, tUpdateTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteData (strGameData, GameData::LastUpdateCheck, tUpdateTime);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // If paused, reset SecondsSinceLastUpdateWhilePaused to zero
    if (vGameState.GetInteger() & (PAUSED | ADMIN_PAUSED)) {

        iErrCode = m_pGameData->WriteData (
            strGameData, 
            GameData::SecondsSinceLastUpdateWhilePaused,
            0
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    
    } else {

        bool bPaused;
        iErrCode = CheckForDelayedPause (iGameClass, iGameNumber, &bPaused);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (bPaused) {

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

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    //////////////
    // Clean up //
    //////////////

Cleanup:

    // Send fatal messages even if the game ended
    for (i = 0; i < iNumEmpires; i ++) {

        if (!pbAlive[i] && pbSendFatalMessage[i]) {

            SendFatalUpdateMessage (
                iGameClass, 
                iGameNumber, 
                piEmpireKey[i], 
                pszGameClassName, 
                pstrUpdateMessage[i]
                );
        }
    }

    if (pstrUpdateMessage != NULL) {
        delete [] pstrUpdateMessage;
    }

    if (pvGoodColor != NULL) {
        delete [] pvGoodColor;
    }

    if (piOriginalPlanetOwner != NULL) {
        delete [] piOriginalPlanetOwner;
    }

    if (piPlanetKey != NULL) {
        m_pGameData->FreeKeys (piPlanetKey);
    }

    for (i = 0; i < iNumNukedPlanets; i ++) {

        if (ppiShipNukeKey[i] != NULL) {
            delete [] ppiShipNukeKey[i];
        }
    }

    return iResult;
}



int GameEngine::UpdateDiplomaticStatus (int iGameClass, int iGameNumber, unsigned int iNumEmpires, unsigned int* piEmpireKey, 
                                        bool* pbAlive, bool* pbSendFatalMessage, Variant* pvEmpireName, 
                                        String* pstrUpdateMessage, 
                                        const char** pstrEmpireDip, const char** pstrEmpireMap, 
                                        const char* strGameMap, 
                                        const char** pstrEmpireData, int* piWinner, int* piLoser, 
                                        unsigned int* piNumSurrenders, const char* pszGameClassName,
                                        int iNewUpdateCount, Variant* pvGoodColor, Variant* pvBadColor) {
    
    Variant vOffer, vOldStatus, vReceive, * pvTemp, vTemp, vMinX, vMaxX, vMinY, vMaxY, vNeighbourPlanetKey,
        vCoord;

    unsigned int iKey1, iKey2, iKey, * piProxyKey;
    String strMessage;

    int i, j, k, iErrCode, iFinal, iNumKeys, iDipLevel, iOldOffer, iOldReceive, iMapSharedDip;

    bool bInvisibleDiplomacy, bPermanentAlliances;

    *piNumSurrenders = 0;

    // Options
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    bInvisibleDiplomacy = (vTemp.GetInteger() & VISIBLE_DIPLOMACY) == 0;
    bPermanentAlliances = (vTemp.GetInteger() & PERMANENT_ALLIANCES) != 0;

    // Maps shared
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MapsShared, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    iMapSharedDip = vTemp.GetInteger();

    // Get dip level
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vTemp
        );
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
                
    iDipLevel = vTemp.GetInteger();

    // Loop through all empires
    for (i = 0; i < (int) iNumEmpires; i ++) {
        
        if (!pbAlive[i]) {
            continue;
        }

#ifdef _DEBUG
        iErrCode = CheckTruceTradeAllianceCounts (iGameClass, iGameNumber, piEmpireKey[i]);
        Assert (iErrCode == OK);
#endif
        
        // Get the empire's acquaintances
        iErrCode = m_pGameData->ReadColumn (
            pstrEmpireDip[i], 
            GameEmpireDiplomacy::EmpireKey, 
            &piProxyKey, 
            &pvTemp,
            (unsigned int*) &iNumKeys
            );

        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            return iErrCode;
        }

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

            Assert (k < (int) iNumEmpires);
            
            if (!pbAlive[k]) {
                continue;
            }
            
            iKey1 = piProxyKey[j];

            iErrCode = m_pGameData->ReadData (pstrEmpireDip[i], iKey1, GameEmpireDiplomacy::DipOffer, &vOffer);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup1;
            }
            
            iErrCode = m_pGameData->GetFirstKey (pstrEmpireDip[k], GameEmpireDiplomacy::EmpireKey, piEmpireKey[i], false, &iKey2);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup1;
            }

            iErrCode = m_pGameData->ReadData (pstrEmpireDip[k], iKey2, GameEmpireDiplomacy::DipOffer, &vReceive);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup1;
            }

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

            iErrCode = m_pGameData->ReadData (
                pstrEmpireDip[i], 
                iKey1, 
                GameEmpireDiplomacy::CurrentStatus, 
                &vOldStatus
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup1;
            }

            if (vReceive.GetInteger() == vOffer.GetInteger()) {
                
                if (vReceive.GetInteger() == SURRENDER || 
                    vReceive.GetInteger() == ACCEPT_SURRENDER) {

                    // Stay with prev status
                    iFinal = vOldStatus.GetInteger();
                
                } else {

                    // The empires agree
                    iFinal = vOffer.GetInteger();
                }

            } else {

                // The final diplomatic level is the least of the offered dips and the current status
                if (vOffer.GetInteger() < vReceive.GetInteger()) {

                    if (vOffer.GetInteger() < vOldStatus.GetInteger()) {
                        iFinal = vOffer.GetInteger();
                    } else {
                        iFinal = vOldStatus.GetInteger();
                    }
                    
                } else {
                    
                    if (vReceive.GetInteger() < vOldStatus.GetInteger()) {
                        iFinal = vReceive.GetInteger();
                    } else {
                        iFinal = vOldStatus.GetInteger();
                    }
                }
            }

            // Write final status
            if (iFinal != vOldStatus.GetInteger()) {
                
                // Update current status
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireDip[i], 
                    iKey1, 
                    GameEmpireDiplomacy::CurrentStatus, 
                    iFinal
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup1;
                }
                
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireDip[k], 
                    iKey2, 
                    GameEmpireDiplomacy::CurrentStatus, 
                    iFinal
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup1;
                }
                
                // Handle potential alliance leak decrement
                if (bPermanentAlliances && iFinal == ALLIANCE) {

                    iErrCode = m_pGameData->ReadData(
                        pstrEmpireDip[i],
                        iKey1,
                        GameEmpireDiplomacy::State,
                        &vTemp
                        );
                    
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup1;
                    }
                    
                    if (vTemp.GetInteger() & ONCE_ALLIED_WITH) {

#ifdef _DEBUG
                        iErrCode = m_pGameData->ReadData (
                            pstrEmpireData[i],
                            GameEmpireData::NumAlliancesLeaked,
                            &vTemp
                            );

                        Assert (iErrCode == OK && vTemp.GetInteger() > 0);

                        iErrCode = m_pGameData->ReadData (
                            pstrEmpireData[k],
                            GameEmpireData::NumAlliancesLeaked,
                            &vTemp
                            );

                        Assert (iErrCode == OK && vTemp.GetInteger() > 0);
#endif
                        
                        iErrCode = m_pGameData->Increment (
                            pstrEmpireData[i],
                            GameEmpireData::NumAlliancesLeaked,
                            -1
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup1;
                        }
                        
                        iErrCode = m_pGameData->Increment (
                            pstrEmpireData[k],
                            GameEmpireData::NumAlliancesLeaked,
                            -1
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup1;
                        }
                    }
                }

                // Handle downgrades
                else if (iFinal < vOldStatus.GetInteger()) {

                    iOldOffer = vOffer.GetInteger();

                    if (iOldOffer != iFinal) {

                        switch (iOldOffer) {

                        case TRADE:

                            if (iFinal == WAR && GameAllowsDiplomacy (iDipLevel, TRUCE)) {                          
                                
                                vOffer = TRUCE;
                                
                                iErrCode = m_pGameData->Increment (
                                    pstrEmpireData[i], 
                                    GameEmpireData::NumTrades, 
                                    -1
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup1;
                                }
                            }
                            break;
                            
                        case ALLIANCE:
                            
                            switch (iFinal) {

                            case WAR:

                                if (GameAllowsDiplomacy (iDipLevel, TRUCE) && 
                                    !GameAllowsDiplomacy (iDipLevel, TRADE)) {

                                    vOffer = TRUCE;

                                    if (!bPermanentAlliances) {

                                        iErrCode = m_pGameData->Increment (
                                            pstrEmpireData[i], 
                                            GameEmpireData::NumAlliances, 
                                            -1
                                            );
                                        if (iErrCode != OK) {
                                            Assert (false);
                                            goto Cleanup1;
                                        }
                                    }
                                    
                                    iErrCode = m_pGameData->Increment (
                                        pstrEmpireData[i], 
                                        GameEmpireData::NumTrades, 
                                        -1
                                        );
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto Cleanup1;
                                    }
                                }
                                else if (GameAllowsDiplomacy (iDipLevel, TRADE) && 
                                    !GameAllowsDiplomacy (iDipLevel, TRUCE)) {
                                    
                                    vOffer = TRADE;

                                    if (!bPermanentAlliances) {
                                        
                                        iErrCode = m_pGameData->Increment (
                                            pstrEmpireData[i], 
                                            GameEmpireData::NumAlliances, 
                                            -1
                                            );
                                        if (iErrCode != OK) {
                                            Assert (false);
                                            goto Cleanup1;
                                        }
                                    }
                                }
                                break;
                                
                            case TRUCE:
                                
                                if (GameAllowsDiplomacy (iDipLevel, TRADE)) {
                                    
                                    vOffer = TRADE;

                                    if (!bPermanentAlliances) {
                                    
                                        iErrCode = m_pGameData->Increment (
                                            pstrEmpireData[i], 
                                            GameEmpireData::NumAlliances, 
                                            -1
                                            );
                                        if (iErrCode != OK) {
                                            Assert (false);
                                            goto Cleanup1;
                                        }
                                    }
                                }
                                break;
                            }
                            break;
                        }

                        if (iOldOffer != vOffer.GetInteger()) {
                            
                            iErrCode = m_pGameData->WriteData (
                                pstrEmpireDip[i], 
                                iKey1, 
                                GameEmpireDiplomacy::DipOffer, 
                                vOffer
                                );
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup1;
                            }
                        }
                    }

                    iOldReceive = vReceive.GetInteger();

                    if (iOldReceive != iFinal) {

                        switch (iOldReceive) {

                        case TRADE:

                            if (iFinal == WAR && GameAllowsDiplomacy (iDipLevel, TRUCE)) {                          
                                
                                vReceive = TRUCE;

                                iErrCode = m_pGameData->Increment (
                                    pstrEmpireData[k], 
                                    GameEmpireData::NumTrades, 
                                    -1
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup1;
                                }
                            }
                            break;
                            
                        case ALLIANCE:
                            
                            switch (iFinal) {
                                
                            case WAR:
                                
                                if (GameAllowsDiplomacy (iDipLevel, TRUCE) && 
                                    !GameAllowsDiplomacy (iDipLevel, TRADE)) {
                                    
                                    vReceive = TRUCE;

                                    if (!bPermanentAlliances) {
                                    
                                        iErrCode = m_pGameData->Increment (
                                            pstrEmpireData[k], 
                                            GameEmpireData::NumAlliances, 
                                            -1
                                            );
                                        if (iErrCode != OK) {
                                            Assert (false);
                                            goto Cleanup1;
                                        }
                                    }
                                    
                                    iErrCode = m_pGameData->Increment (
                                        pstrEmpireData[k], 
                                        GameEmpireData::NumTrades, 
                                        -1
                                        );
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto Cleanup1;
                                    }
                                }
                                else if (GameAllowsDiplomacy (iDipLevel, TRADE) && 
                                    !GameAllowsDiplomacy (iDipLevel, TRUCE)) {
                                    
                                    vReceive = TRADE;
                                    
                                    if (!bPermanentAlliances) {

                                        iErrCode = m_pGameData->Increment (
                                            pstrEmpireData[k], 
                                            GameEmpireData::NumAlliances, 
                                            -1
                                            );
                                        if (iErrCode != OK) {
                                            Assert (false);
                                            goto Cleanup1;
                                        }
                                    }
                                }
                                break;
                                
                            case TRUCE:
                                
                                if (GameAllowsDiplomacy (iDipLevel, TRADE)) {
                                    
                                    vReceive = TRADE;

                                    if (!bPermanentAlliances) {
                                    
                                        iErrCode = m_pGameData->Increment (
                                            pstrEmpireData[k], 
                                            GameEmpireData::NumAlliances, 
                                            -1
                                            );
                                        if (iErrCode != OK) {
                                            Assert (false);
                                            goto Cleanup1;
                                        }
                                    }
                                }
                                break;
                            }
                            break;
                        }

                        if (iOldReceive != vReceive.GetInteger()) {

                            iErrCode = m_pGameData->WriteData (
                                pstrEmpireDip[k], 
                                iKey2, 
                                GameEmpireDiplomacy::DipOffer, 
                                vReceive
                                );
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup1;
                            }
                        }
                    }
                    
                    // Adjust counts
                    Variant vOfferi, vOfferk;
                    
                    iErrCode = m_pGameData->ReadData (
                        pstrEmpireDip[i], 
                        iKey1, 
                        GameEmpireDiplomacy::DipOffer, 
                        &vOfferi
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup1;
                    }
                    
                    iErrCode = m_pGameData->ReadData (
                        pstrEmpireDip[k], 
                        iKey2, 
                        GameEmpireDiplomacy::DipOffer, 
                        &vOfferk
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup1;
                    }
                    
                    bool bTruce, bTrade;
                    switch (vOldStatus.GetInteger()) {
                        
                    case TRUCE:
                        
                        if (iFinal == vOfferi.GetInteger()) {
                            iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumTruces, -1);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup1;
                            }
                        }
                        if (iFinal == vOfferk.GetInteger()) {
                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumTruces, -1);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup1;
                            }
                        }
                        break;
                        
                    case TRADE:
                        
                        bTruce = GameAllowsDiplomacy (iDipLevel, TRUCE);
                        
                        if (iFinal == vOfferi.GetInteger()) {
                            
                            iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumTrades, -1);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup1;
                            }
                            
                            if (!bTruce) {
                                iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumTruces, -1);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup1;
                                }
                            }
                        }
                        if (iFinal == vOfferk.GetInteger()) {
                            
                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumTrades, -1);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup1;
                            }
                            
                            if (!bTruce) {
                                iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumTruces, -1);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup1;
                                }
                            }
                        }
                        break;
                        
                    case ALLIANCE:
                        
                        bTruce = GameAllowsDiplomacy (iDipLevel, TRUCE);
                        bTrade = GameAllowsDiplomacy (iDipLevel, TRADE);
                        
                        if (bPermanentAlliances) {
                            
                            // Increment leaked count
                            iErrCode = m_pGameData->Increment (
                                pstrEmpireData[i],
                                GameEmpireData::NumAlliancesLeaked,
                                1
                                );
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup1;
                            }
                            
                            // Increment leaked count
                            iErrCode = m_pGameData->Increment (
                                pstrEmpireData[k],
                                GameEmpireData::NumAlliancesLeaked,
                                1
                                );
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup1;
                            }
                        }
                        
                        // Set once allied with bit
                        iErrCode = m_pGameData->WriteOr (
                            pstrEmpireDip[i],
                            iKey1,
                            GameEmpireDiplomacy::State,
                            ONCE_ALLIED_WITH
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup1;
                        }
                        
                        // Set once allied with bit
                        iErrCode = m_pGameData->WriteOr (
                            pstrEmpireDip[k],
                            iKey2,
                            GameEmpireDiplomacy::State,
                            ONCE_ALLIED_WITH
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup1;
                        }
                        
                        if (iFinal == vOfferi.GetInteger()) {
                            
                            if (!bPermanentAlliances) {
                                
                                // Decrement effective count
                                iErrCode = m_pGameData->Increment (
                                    pstrEmpireData[i], 
                                    GameEmpireData::NumAlliances, 
                                    -1
                                    );
                                
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup1;
                                }
                            }
                            
                            if (!bTrade) {
                                
                                iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumTrades, -1);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup1;
                                }
                                
                                if (!bTruce) {
                                    iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumTruces, -1);
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto Cleanup1;
                                    }
                                }
                            }
                        }
                        
                        if (iFinal == vOfferk.GetInteger()) {

                            if (!bPermanentAlliances) {
                                
                                iErrCode = m_pGameData->Increment (
                                    pstrEmpireData[k], 
                                    GameEmpireData::NumAlliances, 
                                    -1
                                    );
                                
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup1;
                                }
                            }
                            
                            if (!bTrade) {
                                
                                iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumTrades, -1);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup1;
                                }
                                
                                if (!bTruce) {
                                    iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumTruces, -1);
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto Cleanup1;
                                    }
                                }
                            }
                        }
                        break;

                    default:

                        Assert (false);
                        break;

                    }   // End switch on old status

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
                            iMapSharedDip
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup1;
                        }
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
            
            // Update virtual status for invisible diplomacy
            if (bInvisibleDiplomacy) {
                
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireDip[i], 
                    iKey1, 
                    GameEmpireDiplomacy::VirtualStatus, 
                    vOffer
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup1;
                }
                
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireDip[k], 
                    iKey2, 
                    GameEmpireDiplomacy::VirtualStatus, 
                    vReceive
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup1;
                }
            }
            
        }   // End all acquaintances loop

Cleanup1:

        if (iNumKeys > 0) {
            m_pGameData->FreeData (pvTemp);
            m_pGameData->FreeKeys (piProxyKey);
        }

        if (iErrCode != OK) {
            return iErrCode;
        }

#ifdef _DEBUG

        iErrCode = CheckTruceTradeAllianceCounts (iGameClass, iGameNumber, piEmpireKey[i]);
        Assert (iErrCode == OK);
        iErrCode = OK;
#endif
    }   // End all empires loop
    

    if (*piNumSurrenders > 0) {

        /////////////////////////////////////////
        // We treat surrenders just like nukes //
        /////////////////////////////////////////

        GameUpdateInformation guInfo = { iNewUpdateCount, iNumEmpires, (int*) piEmpireKey, pbAlive, pstrUpdateMessage };
        
        // Randomize surrenders
        if (*piNumSurrenders > 1) {
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
                iErrCode = m_pGameData->GetFirstKey (
                    pstrEmpireDip [iWinner],
                    GameEmpireDiplomacy::EmpireKey,
                    piEmpireKey [iLoser],
                    false,
                    &iKey
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }

                iErrCode = m_pGameData->ReadData (
                    pstrEmpireDip [iWinner],
                    iKey,
                    GameEmpireDiplomacy::CurrentStatus,
                    &vTemp
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }

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
                        iGameNumber, pszGameClassName);
                    
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    // Send system message to dead player
                    char pszMessage [256 + MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH];

                    sprintf (
                        pszMessage,
                        "You surrendered out of %s %i to %s",
                        pszGameClassName,
                        iGameNumber,
                        pvEmpireName[iWinner].GetCharPtr()
                        );
                    
                    iErrCode = SendSystemMessage (piEmpireKey[iLoser], pszMessage, SYSTEM, MESSAGE_SYSTEM);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    // Delete empire's tables
                    iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[iLoser], EMPIRE_SURRENDERED, &guInfo);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
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
        
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Owner, &vOwner);

        switch (vOwner.GetInteger()) {
            
        case SYSTEM:
            
            //////////////////////////////////////////////
            // Check for end of annihilation quarantine //
            //////////////////////////////////////////////
            
            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Annihilated, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            if (vTemp.GetInteger() != NOT_ANNIHILATED && vTemp.GetInteger() != ANNIHILATED_FOREVER) {
                
                iErrCode = m_pGameData->Increment (strGameMap, piPlanetKey[i], GameMap::Annihilated, -1);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                if (vTemp.GetInteger() == 1) {
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Name, &vPlanetName);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Coordinates, &vCoord);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
                    
                    for (j = 0; j < iNumEmpires; j ++) {
                        
                        if (pbAlive[j]) {
                            
                            iErrCode = m_pGameData->GetFirstKey (
                                pstrEmpireMap[j], 
                                GameEmpireMap::PlanetKey, 
                                piPlanetKey[i], 
                                false, 
                                &iKey
                                );
                            
                            if (iErrCode == OK) {

                                pstrUpdateMessage[j] += BEGIN_GOOD_FONT(j);

                                AddPlanetNameAndCoordinates (
                                    pstrUpdateMessage[j], 
                                    vPlanetName.GetCharPtr(),
                                    iX,
                                    iY
                                    );

                                pstrUpdateMessage[j] += " is now safe for colonization" END_FONT "\n";
                            }
                            
                            else if (iErrCode == ERROR_DATA_NOT_FOUND) {

                                iErrCode = OK;
                            }

                            else {
                                Assert (false);
                                return iErrCode;
                            }
                        }
                    }
                }
            }

            break;

        case INDEPENDENT:

            //////////////////////////////
            // Calculate new planet pop //
            //////////////////////////////

            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Ag, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::MaxPop, &vMaxPop);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

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

                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::Pop, iNewPop);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
            }
            
            break;
            
        default:

            /////////////////////////
            // Apply colony builds //
            /////////////////////////

            // Find empire index
            GetEmpireIndex (j, vOwner);
            
            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::PopLostToColonies, &vPopLostToColonies);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            if (vPopLostToColonies.GetInteger() == 0) {
            
                iColBuildCost = 0;

            } else {

                iColBuildCost = vPopLostToColonies.GetInteger();
                Assert (iColBuildCost > 0);

                // Set pop lost back to zero
                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::PopLostToColonies, 0);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
            }
                
            //////////////////////////////
            // Calculate new planet pop //
            //////////////////////////////

            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::MaxPop, &vMaxPop);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            iNewPop = vPop.GetInteger() - iColBuildCost;

            // This is to address the possibility that somehow colony building might make a planet's
            // population go negative, although it really shouldn't happen because the build algorithm
            // prevents colonies from being built when that happens
            if (iNewPop < 0) {
                Assert (false);
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
                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::Pop, iNewPop);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                // Add pop to empire's total pop
                iErrCode = m_pGameData->Increment (
                    pstrEmpireData[j], 
                    GameEmpireData::TotalPop, 
                    iNewPop - vPop.GetInteger()
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                /////////////////////////////
                // Handle resource changes //
                /////////////////////////////
                
                // Min
                iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Minerals, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                piTotalMin[j] += min (iNewPop, vTemp.GetInteger()) - min (vPop.GetInteger(), vTemp.GetInteger());
                
                // Fuel
                iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Fuel, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
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

    int i, j, k, l, m, iTemp, piExploredKey [NUM_CARDINAL_POINTS], iNumAcquaintances, iFleetKey = NO_KEY, 
        iErrCode = OK, iMaintCost, iFuelCost, iNewX, iNewY, iNumShips, iX, iY;

    unsigned int* piProxyKey, iProxyKey, * piShipKey = NULL, iKey, iDestProxyKey;
    float fNewBR;

    Variant vAction, vCurBR, vMaxBR, vType, vShipName, vCoord, vPlanetName, vDestPlanetKey, 
        pvColData[GameEmpireMap::NumColumns],
        vNeighbourPlanetKey, vTemp, * pvAcquaintanceKey, vDipStatus, vDipLevel, vDestPlanetName, vMaxX, vMinX, 
        vMaxY, vMinY, vFleetAction;

    String strTemp;
    bool bUpdated;

    unsigned int piNumBuilds [NUM_SHIP_TYPES];
    unsigned int piNumForceDismantled [NUM_SHIP_TYPES];
    unsigned int piNumVoluntaryDismantled [NUM_SHIP_TYPES];

    // Loop through all empires
    for (i = 0; i < (int) iNumEmpires; i ++) {
        
        if (!pbAlive[i]) {
            continue;
        }
            
        // Get keys for all empire's ships
        iErrCode = m_pGameData->GetAllKeys (pstrEmpireShips[i], &piShipKey, (unsigned int*) &iNumShips);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            continue;
        }

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
            
        // Randomize ship list
        if (iNumShips > 1) {
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

            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vType);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::CurrentBR, 
                &vCurBR
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::MaxBR, 
                &vMaxBR
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            fNewBR = vCurBR.GetFloat() * pfMaintRatio[i];
            
            // Check for ships with BR's too low to survive
            if (fNewBR < FLOAT_PROXIMITY_TOLERANCE) {
                
                // Destroy ship
                iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
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
                
                Assert (fNewBR >= 0.0);
                
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::CurrentBR, 
                    fNewBR
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
            
            ///////////////////////
            // Handle ship moves //
            ///////////////////////
            
            // Get ship's current action
            iErrCode = m_pGameData->ReadData (
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::Action, 
                &vAction
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::FleetKey, 
                &vTemp
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            iFleetKey = vTemp.GetInteger();
            
            if (vAction.GetInteger() >= 0) {
                vAction = BUILD_INTO_FLEET;
            }
            
            // If the ship is following a fleet action, get that fleet's action
            else if (vAction.GetInteger() == FLEET) {
                    
                Assert (iFleetKey != NO_KEY);
#ifdef _DEBUG

                Variant vTemp2;
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::CurrentPlanet, 
                    &vTemp2
                    );
                Assert (iErrCode == OK);
                
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireFleets[i], 
                    iFleetKey, 
                    GameEmpireFleets::CurrentPlanet, 
                    &vTemp
                    );
                Assert (iErrCode == OK);
                
                Assert (vTemp2.GetInteger() == vTemp.GetInteger());
#endif
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireFleets[i], 
                    iFleetKey, 
                    GameEmpireFleets::Action, 
                    &vAction
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                // Special case for standby and ...
                if (IS_FLEET_STANDBY_ACTION (vAction.GetInteger())) {

                    if (vType.GetInteger() == FLEET_STANDBY_TECH_FROM_ORDER (vAction.GetInteger())) {

                        vAction = FLEET_ACTION_FOR_TECH [vType.GetInteger()];
                        Assert (vAction.GetInteger() != NO_KEY);

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

                    iErrCode = m_pGameData->WriteData (
                        pstrEmpireShips[i], 
                        piShipKey[j],
                        GameEmpireShips::Action,
                        vAction
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    continue;
                }
                
                // Special case for nuke
                else if (vAction.GetInteger() == NUKE) {

                    // Make sure not cloaked
                    iErrCode = m_pGameData->ReadData (
                        pstrEmpireShips[i], 
                        piShipKey[j], 
                        GameEmpireShips::State, 
                        &vTemp
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    if (vTemp.GetInteger() & CLOAKED) {

                        // Cloaked, so we can't nuke
                        vAction = STAND_BY;

                    } else {

                        // Make sure not a illegal nuking sci
                        if (vType.GetInteger() == SCIENCE && (iGameClassOptions & DISABLE_SCIENCE_SHIPS_NUKING)) {

                            vAction = STAND_BY;

                        } else {
                    
                            iErrCode = m_pGameData->WriteData (
                                pstrEmpireShips[i], 
                                piShipKey[j],
                                GameEmpireShips::Action,
                                NUKE
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

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
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                break;

                // Handle builds
            case BUILD_AT:
            case BUILD_INTO_FLEET:
                
                // Get planet key
                iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                iPlanetKey = vTemp.GetInteger();
                
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Type, 
                    &vType
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                // Set action to the right thing and built this update to 0
                if (vAction.GetInteger() == BUILD_INTO_FLEET) {
                    
                    iErrCode = m_pGameData->Increment (
                        pstrEmpireFleets[i], 
                        iFleetKey, 
                        GameEmpireFleets::BuildShips, 
                        -1
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
#ifdef _DEBUG
                    Variant vMakeSure;
                    iErrCode = m_pGameData->ReadData (
                        pstrEmpireFleets[i], 
                        iFleetKey, 
                        GameEmpireFleets::CurrentPlanet, 
                        &vMakeSure
                        );
                    Assert (iErrCode == OK && iPlanetKey == (unsigned int) vMakeSure.GetInteger());
#endif
                }
                
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::BuiltThisUpdate, 
                    0
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                // Increase empire's resource use
                iMaintCost = GetMaintenanceCost (vType.GetInteger(), vMaxBR.GetFloat());
                iFuelCost = GetFuelCost (vType.GetInteger(), vMaxBR.GetFloat());

                iErrCode = m_pGameData->Increment (
                    pstrEmpireData[i], 
                    GameEmpireData::TotalMaintenance,
                    iMaintCost
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iErrCode = m_pGameData->Increment (
                    pstrEmpireData[i], 
                    GameEmpireData::TotalFuelUse, 
                    iFuelCost
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
#ifdef _DEBUG
                {
                    Variant vTotalFuelUse;
                    iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalFuelUse, &vTotalFuelUse);
                    Assert (iErrCode == OK && vTotalFuelUse.GetInteger() >= 0);
                }
#endif
                // Update empire's NumOwnShips and BuildShips
                iErrCode = m_pGameData->GetFirstKey (
                    pstrEmpireMap[i], 
                    GameEmpireMap::PlanetKey, 
                    iPlanetKey,
                    false, 
                    &iKey
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (vType.GetInteger() == CLOAKER && (gcConfig.iShipBehavior & CLOAKER_CLOAK_ON_BUILD)) {

                    iErrCode = m_pGameData->Increment (pstrEmpireMap[i], iKey, GameEmpireMap::NumCloakedBuildShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (pstrEmpireMap[i], iKey, GameEmpireMap::NumCloakedShips, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumCloakedBuildShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumCloakedShips, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                } else {
                    iErrCode = m_pGameData->Increment (pstrEmpireMap[i], iKey, GameEmpireMap::NumUncloakedBuildShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (pstrEmpireMap[i], iKey, GameEmpireMap::NumUncloakedShips, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumUncloakedBuildShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumUncloakedShips, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
                
                // Add build to update message
                Assert (vType.GetInteger() >= 0 && vType.GetInteger() < NUM_SHIP_TYPES);
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
                iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                iPlanetKey = vTemp.GetInteger();
                
                // Initialize the proxy key variable
                iDestProxyKey = NO_KEY;
                
                // Get the direction
                iTemp = (vAction.GetInteger() > EXPLORE_NORTH) ? 
                    MOVE_NORTH - vAction.GetInteger() : EXPLORE_NORTH - vAction.GetInteger();
                
                Assert (iTemp >= NORTH && iTemp <= WEST);
                
                // Get key of destination planet
                iErrCode = m_pGameData->ReadData (
                    strGameMap, 
                    iPlanetKey, 
                    GameMap::NorthPlanetKey + iTemp, 
                    &vDestPlanetKey
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                Assert (vDestPlanetKey != NO_KEY);
                vDestPlanetName = (const char*) NULL;
                
                // Get coordinates
                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
                AdvanceCoordinates (iX, iY, &iNewX, &iNewY, iTemp);
                
                // Exploring?
                if (vAction.GetInteger() <= EXPLORE_NORTH) {
                    
                    // If planet hasn't been explored, add to empire's map, update all 
                    // ExploredX parameters and add to update message
                    iErrCode = m_pGameData->GetFirstKey (
                        pstrEmpireMap[i], 
                        GameEmpireMap::PlanetKey, 
                        vDestPlanetKey.GetInteger(), 
                        false, 
                        &iDestProxyKey
                        );
                    
                    if (iErrCode == ERROR_DATA_NOT_FOUND) {

                        Assert (iDestProxyKey == NO_KEY);
                        
                        ///////////////////////////
                        // An unexplored planet! //
                        ///////////////////////////
                        
                        // Add to map
                        pvColData[GameEmpireMap::PlanetKey] = vDestPlanetKey.GetInteger();
                        pvColData[GameEmpireMap::Explored] = 0;
                        pvColData[GameEmpireMap::RESERVED0] = 0;
                        pvColData[GameEmpireMap::RESERVED1] = 0;
                        pvColData[GameEmpireMap::RESERVED2] = 0;
                        pvColData[GameEmpireMap::NumUncloakedShips] = 0;
                        pvColData[GameEmpireMap::NumCloakedBuildShips] = 0;
                        pvColData[GameEmpireMap::NumUncloakedBuildShips] = 0;
                        pvColData[GameEmpireMap::NumCloakedShips] = 0;
                        
                        iErrCode = m_pGameData->InsertRow (pstrEmpireMap[i], pvColData, &iDestProxyKey);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        Assert (iDestProxyKey != NO_KEY);
                        
                        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::MinX, &vMinX);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::MaxX, &vMaxX);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::MinY, &vMinY);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::MaxY, &vMaxY);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        Assert (
                            vMinX.GetInteger() > 0 && vMinY.GetInteger() > 0 && 
                            vMaxX.GetInteger() > 0 && vMaxY.GetInteger() > 0);
                        Assert (iNewX > 0 && iNewY > 0);
                        
                        if (iNewX > vMaxX.GetInteger()) {
                            iErrCode = m_pGameData->WriteData (pstrEmpireData[i], 
                                GameEmpireData::MaxX, iNewX);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                        }
                        if (iNewX < vMinX.GetInteger()) {
                            iErrCode = m_pGameData->WriteData (pstrEmpireData[i], 
                                GameEmpireData::MinX, iNewX);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                        }
                        if (iNewY > vMaxY.GetInteger()) {
                            iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::MaxY, iNewY);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                        }
                        if (iNewY < vMinY.GetInteger()) {
                            iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::MinY, iNewY);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                        }
                        
                        ////////////////////////////////////////////////////////
                        // Update all ExploredX parameters for the new planet //
                        ////////////////////////////////////////////////////////
                        
                        ENUMERATE_CARDINAL_POINTS(k) {
                            
                            // Get neighbour key
                            iErrCode = m_pGameData->ReadData (
                                strGameMap,
                                vDestPlanetKey.GetInteger(), 
                                GameMap::NorthPlanetKey + k, 
                                &vNeighbourPlanetKey
                                );
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            piExploredKey[k] = vNeighbourPlanetKey;
                            
                            if (piExploredKey[k] != NO_KEY) {
                                
                                // Has planet been explored already?
                                iErrCode = m_pGameData->GetFirstKey (
                                    pstrEmpireMap[i], 
                                    GameEmpireMap::PlanetKey, 
                                    piExploredKey[k], 
                                    false, 
                                    &iKey
                                    );
                                
                                if (iErrCode == OK) {
                                    
                                    // Set explored to true for old planet
                                    iErrCode = m_pGameData->WriteOr (
                                        pstrEmpireMap[i], 
                                        iKey, 
                                        GameEmpireMap::Explored,
                                        OPPOSITE_EXPLORED_X[k]
                                        );
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto Cleanup;
                                    }
                                    
                                    // Set explored to true for explored planet
                                    iErrCode = m_pGameData->WriteOr (
                                        pstrEmpireMap[i], 
                                        iDestProxyKey,
                                        GameEmpireMap::Explored,
                                        EXPLORED_X[k]
                                        );
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto Cleanup;
                                    }
                                    
                                } else {
                                    
                                    if (iErrCode != ERROR_DATA_NOT_FOUND) {
                                        Assert (false);
                                        goto Cleanup;
                                    }
                                }
                            }
                        }
                            
                        // Add exploration message to update message
                        iErrCode = m_pGameData->ReadData (
                            pstrEmpireShips[i], 
                            piShipKey[j], 
                            GameEmpireShips::Name, 
                            &vShipName
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->ReadData (
                            strGameMap, 
                            vDestPlanetKey, 
                            GameMap::Name, 
                            &vDestPlanetName
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
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
                        iErrCode = m_pGameData->ReadData (
                            SYSTEM_GAMECLASS_DATA, 
                            iGameClass, 
                            SystemGameClassData::MapsShared, 
                            &vDipLevel
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        if (vDipLevel != NO_DIPLOMACY) {
                            
                            iErrCode = m_pGameData->ReadColumn (
                                pstrEmpireDip[i], 
                                GameEmpireDiplomacy::EmpireKey, 
                                &piProxyKey, 
                                &pvAcquaintanceKey, 
                                (unsigned int*) &iNumAcquaintances
                                );
                            
                            if (iErrCode == OK) {
                                
                                for (l = 0; l < iNumAcquaintances; l ++) {
                                    
                                    iErrCode = m_pGameData->ReadData (
                                        pstrEmpireDip[i], 
                                        piProxyKey[l], 
                                        GameEmpireDiplomacy::CurrentStatus, 
                                        &vDipStatus
                                        );
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        m_pGameData->FreeData (pvAcquaintanceKey);
                                        m_pGameData->FreeKeys ((unsigned int*) piProxyKey);
                                        goto Cleanup;
                                    }
                                    
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
                                            -1
                                            );
                                        
                                        if (iErrCode != OK) {
                                            Assert (false);
                                            m_pGameData->FreeData (pvAcquaintanceKey);
                                            m_pGameData->FreeKeys ((unsigned int*) piProxyKey);
                                            goto Cleanup;
                                        }
                                    }
                                }   // End acquaintance loop
                                
                                m_pGameData->FreeData (pvAcquaintanceKey);
                                m_pGameData->FreeKeys ((unsigned int*) piProxyKey);
                                
                            }   // End if acquaintances > 0

                            else if (iErrCode == ERROR_DATA_NOT_FOUND) {
                                iErrCode = OK;
                            }

                            else {
                                Assert (false);
                                goto Cleanup;
                            }
                        }   // End if mapshare
                    }   // End if exploring a new planet
                    
                    else if (iErrCode != OK) {

                        Assert (false);
                        goto Cleanup;
                    }
                }   // End if _explore_ and not _move_
                
                // Update action to standby and location to new planet
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::CurrentPlanet, 
                    vDestPlanetKey
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                // Get proxy key in empire map for new planet
                if (iDestProxyKey == NO_KEY) {
                    
                    iErrCode = m_pGameData->GetFirstKey (
                        pstrEmpireMap[i], 
                        GameEmpireMap::PlanetKey, 
                        vDestPlanetKey, 
                        false, 
                        (unsigned int*) &iDestProxyKey
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
                
                // Get proxy key in empire map for old planet
                iErrCode = m_pGameData->GetFirstKey (
                    pstrEmpireMap[i], 
                    GameEmpireMap::PlanetKey, 
                    iPlanetKey, 
                    false, 
                    &iProxyKey
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                // Update ship count parameters
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::State, 
                    &vTemp
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (vTemp.GetInteger() & CLOAKED) {
                    iErrCode = m_pGameData->Increment (pstrEmpireMap[i], iProxyKey, GameEmpireMap::NumCloakedShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (pstrEmpireMap[i], iDestProxyKey, GameEmpireMap::NumCloakedShips, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumCloakedShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (strGameMap, vDestPlanetKey, GameMap::NumCloakedShips, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                
                } else {

                    iErrCode = m_pGameData->Increment (pstrEmpireMap[i], iProxyKey, GameEmpireMap::NumUncloakedShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (pstrEmpireMap[i], iDestProxyKey, GameEmpireMap::NumUncloakedShips, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::NumUncloakedShips, -1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iErrCode = m_pGameData->Increment (strGameMap, vDestPlanetKey, GameMap::NumUncloakedShips, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
#ifdef _DEBUG
                    Variant vFooBar;
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::NumUncloakedShips, &vFooBar);
                    Assert (iErrCode == OK && vFooBar.GetInteger() >= 0);
#endif
                    if (String::IsBlank (vDestPlanetName.GetCharPtr())) {

                        iErrCode = m_pGameData->ReadData (
                            strGameMap,
                            vDestPlanetKey.GetInteger(),
                            GameMap::Name,
                            &vDestPlanetName
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                    }

                    iErrCode = CheckForFirstContact (
                        piEmpireKey[i], 
                        i, 
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

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
                
                bUpdated = true;
                break;
                
            case CLOAK:
                
                // Get planet key
                iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                iPlanetKey = vTemp.GetInteger();
                
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Name, 
                    &vShipName
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

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

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

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
                iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                iPlanetKey = vTemp.GetInteger();
                
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Name, 
                    &vShipName
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
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

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
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
                
                iErrCode = CheckForFirstContact (
                    piEmpireKey[i], 
                    i, 
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

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                bUpdated = true;
                break;

            default:
                
                // Special case for morphers
                if (IS_MORPH_ACTION (vAction.GetInteger())) {

                    bool bDied = false;

                    Variant vTechs, vBR, vShipType, vShipState;
                    
                    iTemp = MORPH_BASETECH - vAction.GetInteger();
                    Assert (iTemp >= FIRST_SHIP && iTemp <= LAST_SHIP);
                    
                    // Get planet key
                    iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iPlanetKey = vTemp.GetInteger();
                    
                    // Get developed techs
                    iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TechDevs, &vTechs);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    // Get BR
                    iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentBR, &vBR);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    // Get current type
                    iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vShipType);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    // Get ship name
                    iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Name, &vShipName);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    // Get ship state
                    iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::State, &vShipState);
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
                    
                    // Get coordinates
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
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
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
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
                            
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            // Handle cloaked state
                            if (iTemp == CLOAKER) {

                                Assert (!(vShipState.GetInteger() & CLOAKED));
                                
                                // Set ship as cloaked
                                iErrCode = ChangeShipCloakingState (
                                    piShipKey[j],
                                    iPlanetKey,
                                    true,
                                    pstrEmpireShips[i], 
                                    pstrEmpireMap[i], 
                                    strGameMap
                                    );
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
                            }
                            
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
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
                        iErrCode = m_pGameData->WriteData (
                            pstrEmpireShips[i], 
                            piShipKey[j], 
                            GameEmpireShips::Action, 
                            STAND_BY
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                    }

                }   // End if morphing
                
                break;

            }   // End ship action switch                   
            
            // If the ship's action was processed, set its new action to standby.
            if (bUpdated) {
                
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Action, 
                    STAND_BY
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }   // End ships loop
        
Cleanup:

        m_pGameData->FreeKeys (piShipKey);

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

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
                                const char** pstrEmpireData, float* pfFuelRatio, int iNumPlanets, 
                                unsigned int* piPlanetKey, int* piTotalMin, int* piTotalFuel,
                                bool bIndependence, const char* strIndependentShips,
                                Variant* pvGoodColor, Variant* pvBadColor, 
                                const GameConfiguration& gcConfig) {

    int i, j, k, l, iErrCode = OK, iCurrentShip, iCounter;
    
    unsigned int iPlanetProxyKey, iKey, * piShipKey = NULL, iNumShips, iNumIndependentShips = 0,
        * piIndependentShipKey = NULL;

    Variant vNumShips, vNumCloaked, vDipStatus, pvColData[GameEmpireDiplomacy::NumColumns], vPlanetName, 
        vTemp, vPop, vMin, vFuel, vCoord, vOwner, vMineName;
    String strList;
    
    char pszPlanetName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH];

    float fDmgRatio, fDest, fDV, fDamageRatio, fFactor;
    bool bMines, bDetonated;

    unsigned int iNumAdjustedEmpires = iNumEmpires + 1;

    bool* pbBattleEmpire = (bool*) StackAlloc (iNumAdjustedEmpires * sizeof (bool));

    int* piIntBlock = (int*) StackAlloc (5 * iNumAdjustedEmpires * sizeof (int));

    int* piBattleEmpireIndex = piIntBlock;
    int* piNumBattleShips = piBattleEmpireIndex + iNumAdjustedEmpires;
    int* piNumCloaked = piNumBattleShips + iNumAdjustedEmpires;
    int* piWatcherIndex = piNumCloaked + iNumAdjustedEmpires;
    int* piBattleShipsDestroyed = piWatcherIndex + iNumAdjustedEmpires;

    void** ppvPtrBlock = (void**) StackAlloc (8 * iNumAdjustedEmpires * sizeof (void*));

    int** ppiBattleShipKey = (int**) ppvPtrBlock;
    float** ppfBattleShipBR = (float**) ppiBattleShipKey + iNumAdjustedEmpires;
    float** ppfRealBR = (float**) ppfBattleShipBR + iNumAdjustedEmpires;
    int** ppiCloakerKey = (int**)  ppfRealBR + iNumAdjustedEmpires;
    bool** ppbAlive = (bool**) ppiCloakerKey + iNumAdjustedEmpires;
    bool** ppbSweep = (bool**) ppbAlive + iNumAdjustedEmpires;
    int** ppiDipRef = (int**)  ppbSweep + iNumAdjustedEmpires;
    const char** ppszEmpireShips = (const char**) ppiDipRef + iNumAdjustedEmpires;

    // Assign 2D dipref arrays
    int* piBlob = (int*) StackAlloc (iNumAdjustedEmpires * iNumAdjustedEmpires * sizeof (int));
    for (i = 0; i <= iNumEmpires; i ++) {
        ppiDipRef[i] = piBlob + i * (iNumAdjustedEmpires);
    }

    float* pfBP = (float*) StackAlloc ((iNumAdjustedEmpires) * 3 * sizeof (float));
    float* pfEnemyBP = pfBP + iNumAdjustedEmpires;
    float* pfTotDmg = pfEnemyBP + iNumAdjustedEmpires;

    unsigned int iMineIndex, iTempMineIndex, iMineOwner;

    // Randomize planet list
    Algorithm::Randomize <unsigned int> (piPlanetKey, iNumPlanets);

    // Loop through all planets
    int iNumBattleEmpires, iNumWatchers;
    bool bFight, bTestedIndependence;

    for (i = 0; i < iNumPlanets; i ++) {
    
        // Initialize variables
        iNumBattleEmpires = 0;  // Zero empires fighting
        iNumWatchers = 0;       // Zero empires watching
        bFight = false;         // No fight
        
        iMineOwner = NO_KEY;
        iMineIndex = NO_KEY;

        // No owners yet
        bTestedIndependence = true;
        
        // First, find out if a battle will take place at this planet
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::NumUncloakedShips, &vNumShips);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        if (vNumShips.GetInteger() == 0) {
            continue;
        }

        pszPlanetName[0] = '\0';

        bTestedIndependence = false;
        bMines = false;
        bDetonated = false;

        // There are ships here:  loop through all empires
        for (j = 0; j < iNumEmpires; j ++) {

            if (!pbAlive[j]) {
                continue;
            }
            
            iErrCode = m_pGameData->GetFirstKey (
                pstrEmpireMap[j], 
                GameEmpireMap::PlanetKey, 
                piPlanetKey[i], 
                false, 
                &iPlanetProxyKey
                );
            
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            }
            
            else if (iErrCode != OK) {
                
                Assert (false);
                return iErrCode;
            }
            
            else {
                
                // Add to watchers list
                piWatcherIndex[iNumWatchers] = j;
                iNumWatchers ++;
                
                // Does empire have visible ships at the planet?
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireMap[j], 
                    iPlanetProxyKey, 
                    GameEmpireMap::NumUncloakedShips, 
                    &vNumShips
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                if (vNumShips.GetInteger() > 0) {
                    
                    // Add empire to list of possible battling empires
                    piBattleEmpireIndex [iNumBattleEmpires] = j;
                    pbBattleEmpire [j] = true;
                    
                    piNumBattleShips[iNumBattleEmpires] = 0;
                    piNumCloaked[iNumBattleEmpires] = 0;
                    piBattleShipsDestroyed[iNumBattleEmpires] = 0;

                    ppiBattleShipKey[iNumBattleEmpires] = NULL;
                    ppfBattleShipBR[iNumBattleEmpires] = NULL;
                    ppbAlive[iNumBattleEmpires] = NULL;
                    ppbSweep[iNumBattleEmpires] = NULL;
                    ppiCloakerKey[iNumBattleEmpires] = NULL;
                    
                    // Scan through empires already found at this planet
                    for (k = 0; k < iNumBattleEmpires; k ++) {
                        
                        // Check diplomatic setting
                        iErrCode = m_pGameData->GetFirstKey (
                            pstrEmpireDip[j], 
                            GameEmpireDiplomacy::EmpireKey, 
                            piEmpireKey[piBattleEmpireIndex[k]], 
                            false, 
                            &iKey
                            );
                        
                        if (iErrCode == OK) {
                            
                            iErrCode = m_pGameData->ReadData (pstrEmpireDip[j], iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
                            if (iErrCode != OK) {
                                Assert (false);
                                return iErrCode;
                            }

                            if (vDipStatus.GetInteger() == WAR) {
                                bFight = true;
                            }
                        }
                        
                        else if (iErrCode != ERROR_DATA_NOT_FOUND) {
                        
                            Assert (false);
                            return iErrCode;
                        }
                        
                        else {

                            
                            //////////////////////////////////
                            // First contact (ship to ship) //
                            //////////////////////////////////
                            
                            vDipStatus = WAR;
                            bFight = true;

                            // Add second empire to first's dip screen
                            pvColData[GameEmpireDiplomacy::EmpireKey] = piEmpireKey[piBattleEmpireIndex[k]];
                            pvColData[GameEmpireDiplomacy::DipOffer] = WAR;
                            pvColData[GameEmpireDiplomacy::CurrentStatus] = WAR;
                            pvColData[GameEmpireDiplomacy::VirtualStatus] = WAR;
                            pvColData[GameEmpireDiplomacy::State] = 0;
                            pvColData[GameEmpireDiplomacy::SubjectiveEcon] = 0;
                            pvColData[GameEmpireDiplomacy::SubjectiveMil] = 0;
                            pvColData[GameEmpireDiplomacy::LastMessageTargetFlag] = 0;
                            
                            iErrCode = m_pGameData->InsertRow (pstrEmpireDip[j], pvColData);
                            if (iErrCode != OK) {
                                Assert (false);
                                return iErrCode;
                            }
                            
                            // Add first empire to second's dip screen
                            pvColData[GameEmpireDiplomacy::EmpireKey] = piEmpireKey[j]; // EmpireKey
                            
                            iErrCode = m_pGameData->InsertRow (
                                pstrEmpireDip[piBattleEmpireIndex[k]], 
                                pvColData
                                );
                            if (iErrCode != OK) {
                                Assert (false);
                                return iErrCode;
                            }
                            
                            // Add to first empire's update message
                            if (pszPlanetName[0] == '\0') {

                                iErrCode = GetPlanetNameWithCoordinates (strGameMap, piPlanetKey[i], pszPlanetName);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    strcpy (pszPlanetName, "Error");
                                }
                            }

                            pstrUpdateMessage[j] += "You have had " BEGIN_STRONG " first contact " END_STRONG " with " BEGIN_STRONG;
                            pstrUpdateMessage[j] += pvEmpireName[piBattleEmpireIndex[k]].GetCharPtr();
                            pstrUpdateMessage[j] += END_STRONG " (ship to ship) at ";
                            pstrUpdateMessage[j].AppendHtml (pszPlanetName, 0, false);
                            pstrUpdateMessage[j] += "\n";
                            
                            // Add to second empire's update message
                            pstrUpdateMessage[piBattleEmpireIndex[k]] += "You have had " BEGIN_STRONG " first contact " END_STRONG " with " BEGIN_STRONG;
                            pstrUpdateMessage[piBattleEmpireIndex[k]] += pvEmpireName[j].GetCharPtr();
                            pstrUpdateMessage[piBattleEmpireIndex[k]] += END_STRONG " (ship to ship) at ";
                            pstrUpdateMessage[piBattleEmpireIndex[k]].AppendHtml (pszPlanetName, 0, false);
                            pstrUpdateMessage[piBattleEmpireIndex[k]] += "\n";

                            // End first contact
                        }
                        
                        // Save the diplomatic status between the empires
                        ppiDipRef[iNumBattleEmpires][k] = vDipStatus.GetInteger();
                        ppiDipRef[k][iNumBattleEmpires] = vDipStatus.GetInteger();
                    }
                    
                    // Increment the number of empires with ships on the planet
                    iNumBattleEmpires ++;
                
                } else {
                    
                    // Sir Not-involved-in-this-battle
                    pbBattleEmpire[j] = false;

                } // End if empire has uncloaked ships on planet
            } // End if empire has explored planet
        }   // End empire loop

        // Independent ships?
        if (bIndependence && iNumBattleEmpires > 0) {

            bFight = true;
            
            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Owner, &vOwner);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            if (vOwner.GetInteger() == INDEPENDENT) {

                bTestedIndependence = true;

                iErrCode = m_pGameData->GetEqualKeys (
                    strIndependentShips,
                    GameIndependentShips::CurrentPlanet,
                    piPlanetKey[i],
                    false,
                    &piIndependentShipKey,
                    &iNumIndependentShips
                    );

                if (iErrCode == OK) {

                    // Add to owners list
                    piBattleEmpireIndex [iNumBattleEmpires] = INDEPENDENT;

                    for (j = 0; j < iNumBattleEmpires; j ++) {
                        ppiDipRef[iNumBattleEmpires][j] = WAR;
                        ppiDipRef[j][iNumBattleEmpires] = WAR;
                    }

                    ppiBattleShipKey[iNumBattleEmpires] = NULL;
                    ppfBattleShipBR[iNumBattleEmpires] = NULL;
                    ppbAlive[iNumBattleEmpires] = NULL;
                    ppbSweep[iNumBattleEmpires] = NULL;
                    ppiCloakerKey[iNumBattleEmpires] = NULL;

                    piNumBattleShips[iNumBattleEmpires] = 0;
                    piNumCloaked[iNumBattleEmpires] = 0;
                    piBattleShipsDestroyed[iNumBattleEmpires] = 0;

/*                  piNumBattleShips[iNumBattleEmpires] = iNumShips;
                    piNumCloaked[iNumBattleEmpires] = 0;
                    piBattleShipsDestroyed[iNumBattleEmpires] = 0;

                    ppiBattleShipKey[iNumBattleEmpires] = new int [iNumShips];
                    memcpy (ppiBattleShipKey[iNumBattleEmpires], piShipKey, sizeof (int) * iNumShips);

                    ppiCloakerKey[iNumBattleEmpires] = NULL;

                    ppfBattleShipBR[iNumBattleEmpires] = new float [iNumShips];
                    ppbAlive[iNumBattleEmpires] = new bool [iNumShips];

                    for (j = 0; j < iNumShips; j ++) {

                        ppbAlive[iNumBattleEmpires][j] = true;



                        ppfBattleShipBR[iNumBattleEmpires][j];
                    }

                    FreeKeys (piShipKey);
*/
                    iNumBattleEmpires ++;
                }

                else if (iErrCode != ERROR_DATA_NOT_FOUND) {

                    Assert (false);
                    return iErrCode;
                }

                else iErrCode = OK;
            }
        }

        //////////////////////
        // Fight the battle //
        //////////////////////

        if (bFight) {

            for (j = 0; j < iNumBattleEmpires; j ++) {

                if (piBattleEmpireIndex[j] == INDEPENDENT) {

                    ppszEmpireShips[j] = strIndependentShips;
                    piShipKey = piIndependentShipKey;
                    piIndependentShipKey = NULL;
                    iNumShips = iNumIndependentShips;

                } else {

                    ppszEmpireShips[j] = pstrEmpireShips[piBattleEmpireIndex[j]];
                    
                    // Make a list of all ships at planet
                    iErrCode = m_pGameData->GetEqualKeys (
                        ppszEmpireShips[j], 
                        GameEmpireShips::CurrentPlanet, 
                        piPlanetKey[i], 
                        false, 
                        &piShipKey, 
                        &iNumShips
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }
                }

                Assert (piShipKey != NULL && iNumShips > 0);

                // Randomize ship array
                Algorithm::Randomize (piShipKey, iNumShips);

                // Set up tables
                pfBP[j] = (float) 0.0;
                pfEnemyBP[j] = (float) 0.0;
                pfTotDmg[j] = (float) 0.0;

                ppiBattleShipKey[j] = new int [iNumShips * 2];
                ppiCloakerKey[j] = ppiBattleShipKey[j] + iNumShips;

                ppfBattleShipBR[j] = new float [iNumShips * 2];
                ppfRealBR[j] = ppfBattleShipBR[j] + iNumShips;

                ppbAlive[j] = new bool [iNumShips * 2];
                ppbSweep[j] = ppbAlive[j] + iNumShips;

                // Collect keys while filtering cloaked ships
                for (l = 0; l < (int) iNumShips; l ++) {

                    iErrCode = m_pGameData->ReadData (
                        ppszEmpireShips[j], 
                        piShipKey[l], 
                        GameEmpireShips::State, 
                        &vTemp
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }

                    if (!(vTemp.GetInteger() & CLOAKED)) {

                        // Add ship to tables
                        ppiBattleShipKey[j][piNumBattleShips[j]] = piShipKey[l];
                        ppbAlive[j][piNumBattleShips[j]] = true;
                        ppbSweep[j][piNumBattleShips[j]] = false;
                                            
                        // Add ship's BR ^ 2 to BP
                        iErrCode = m_pGameData->ReadData (
                            ppszEmpireShips[j], 
                            piShipKey[l], 
                            GameEmpireShips::CurrentBR, 
                            &vTemp
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }

                        ppfRealBR[j][piNumBattleShips[j]] = vTemp.GetFloat();

                        fFactor = vTemp.GetFloat() * vTemp.GetFloat();
                        ppfBattleShipBR[j][piNumBattleShips[j]] = fFactor;
                        pfBP[j] += fFactor;
                        piNumBattleShips[j] ++;
                    
                    } else {

                        Assert (piBattleEmpireIndex[j] != INDEPENDENT);

                        // Add cloaked ships to a list (useful if a mine goes off)
                        ppiCloakerKey[j][piNumCloaked[j]] = piShipKey[l];
                        piNumCloaked[j] ++;
                    }

                }   // End cloaker filter loop

                //////////////////////
                // Apply fuel ratio //
                //////////////////////

                if (piBattleEmpireIndex[j] != INDEPENDENT && 
                    pfFuelRatio[piBattleEmpireIndex[j]] < (float) 1.0) {
                    
                    // Apply to BP
                    pfBP[j] *= pfFuelRatio[piBattleEmpireIndex[j]];

                    // Apply to individual ships
                    for (l = 0; l < piNumBattleShips[j]; l ++) {
                        ppfBattleShipBR[j][l] *= pfFuelRatio[piBattleEmpireIndex[j]];
                    }
                }

                m_pGameData->FreeKeys (piShipKey);
                piShipKey = NULL;

            } // End empire BP collection loop  


            // Calculate enemy_bp_tot for each empire
            for (j = 0; j < iNumBattleEmpires; j ++) {
                for (k = j + 1; k < iNumBattleEmpires; k ++) {
                    if (ppiDipRef[j][k] == WAR) {
                        pfEnemyBP[j] += pfBP[k];
                        pfEnemyBP[k] += pfBP[j];
                    }
                }
            }

            // Calculate tot_dmg for each empire
            // This can only be done after all the enemy_bp_tot's are known, 
            // so that's why we have redundant loops
            for (j = 0; j < iNumBattleEmpires; j ++) {
                for (k = j + 1; k < iNumBattleEmpires; k ++) {
                    if (ppiDipRef[j][k] == WAR) {
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

            for (j = 0; j < iNumBattleEmpires; j ++) {

                unsigned int iActionColumn, iTypeColumn;

                fDest = pfTotDmg[j] * (float) gcConfig.iPercentDamageUsedToDestroy / 100;
                fDV = pfTotDmg[j] - fDest;

                iTempMineIndex = NO_KEY;

                if (piBattleEmpireIndex[j] == INDEPENDENT) {

                    iActionColumn = GameIndependentShips::Action;
                    iTypeColumn = GameIndependentShips::Type;

                } else {

                    iActionColumn = GameEmpireShips::Action;
                    iTypeColumn = GameEmpireShips::Type;
                }

                // Apply DEST
                for (iCurrentShip = 0; iCurrentShip < piNumBattleShips[j]; iCurrentShip ++) {

                    // Get ship type
                    iErrCode = m_pGameData->ReadData (
                        ppszEmpireShips[j],
                        ppiBattleShipKey[j][iCurrentShip], 
                        iTypeColumn, 
                        &vTemp
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }

                    switch (vTemp.GetInteger()) {

                    case MINEFIELD:

                        //
                        // Mines are immune to DEST
                        //

                        // Detonate?
                        iErrCode = m_pGameData->ReadData (
                            ppszEmpireShips[j],
                            ppiBattleShipKey[j][iCurrentShip], 
                            iActionColumn, 
                            &vTemp
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }
                        
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
                            pfBP[j] -= ppfBattleShipBR[j][iCurrentShip];
                        
                        } else {

                            if (iTempMineIndex == NO_KEY) {
                                iTempMineIndex = iCurrentShip;
                            }
                        }

                        break;
                            
                    case CARRIER:
                        
                        // A capable carrier?
                        if (fDest >= ppfBattleShipBR[j][iCurrentShip] &&
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
                                pfBP[j] -= ppfBattleShipBR[j][iCurrentShip];

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
                                
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto OnError;
                                }
                                
                                // Adjust BR, battle points
                                ppfRealBR[j][iCurrentShip] -= gcConfig.fCarrierCost;
                                ppfBattleShipBR[j][iCurrentShip] = 
                                    ppfRealBR[j][iCurrentShip] * ppfRealBR[j][iCurrentShip];
                                
                                // Reduce empire's battle points
                                pfBP[j] -= ppfBattleShipBR[j][iCurrentShip];
                                pfBP[j] += fNewBR * fNewBR;
                            }
                        }
                        break;

                    case MINESWEEPER:
                        
                        ppbSweep[j][iCurrentShip] = true;
                        // Fall through
                        
                    default:
                        
                        if (fDest >= ppfBattleShipBR[j][iCurrentShip]) {

                            // Kill ship
                            ppbAlive[j][iCurrentShip] = false;
                            piBattleShipsDestroyed[j] ++;
                            
                            // Reduce empire's battle points
                            pfBP[j] -= ppfBattleShipBR[j][iCurrentShip];
                            
                            // Reduce DEST
                            fDest -= ppfBattleShipBR[j][iCurrentShip];
                        }
                        break;
                    
                    }   // End type switch

                }   // End ships loop

                Assert (fDest >= 0.0);

                // Apply damage ratio
                fDamageRatio = (fDV + fDest) / pfBP[j];
                
                if (fDamageRatio >= (float) 1.0 - FLOAT_PROXIMITY_TOLERANCE) {

                    for (k = 0; k < piNumBattleShips[j]; k ++) {
                        ppbAlive[j][k] = false;
                    }

                    if (iTempMineIndex != NO_KEY && !bMines) {
                        
                        Assert (iMineOwner == NO_KEY);
                        Assert (iMineIndex == NO_KEY);

                        bMines = true;
                        iMineOwner = j;
                        iMineIndex = iTempMineIndex;
                    }
                    
                    piBattleShipsDestroyed[j] = piNumBattleShips[j];

                } else {
                    
                    fFactor = (float) 1.0 - fDamageRatio;
                    
                    // Reduce BR of remaining ships
                    for (k = 0; k < piNumBattleShips[j]; k ++) {
                        
                        if (ppbAlive[j][k]) {

                            iErrCode = m_pGameData->ReadData (
                                ppszEmpireShips[j], 
                                ppiBattleShipKey[j][k], 
                                GameEmpireShips::CurrentBR, 
                                &vTemp
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }
                            
                            vTemp *= fFactor;

                            iErrCode = m_pGameData->ReadData (
                                ppszEmpireShips[j], 
                                ppiBattleShipKey[j][k], 
                                GameEmpireShips::MaxBR, 
                                &vDipStatus
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }

                            if (vDipStatus.GetFloat() < vTemp.GetFloat()) {
                                vTemp = vDipStatus.GetFloat();
                            }
                            
                            // Write new BR of ship
                            iErrCode = m_pGameData->WriteData (
                                ppszEmpireShips[j], 
                                ppiBattleShipKey[j][k], 
                                GameEmpireShips::CurrentBR, 
                                vTemp
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }
                            
                            Assert (vTemp.GetFloat() >= 0.0);
                        }
                    }
                }  // End if ships survived
            } // End empires loop

            // Check for surviving sweepers if a mine went off
            for (j = 0; bMines && j < iNumBattleEmpires; j ++) {

                for (k = 0; bMines && k < piNumBattleShips[j]; k ++) {
                
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
                    piPlanetKey[i],
                    iNumEmpires,
                    piEmpireKey,
                    piTotalMin,
                    piTotalFuel
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto OnError;
                }
                
                // Kill all ships on planet
                for (j = 0; j < iNumBattleEmpires; j ++) {
                    for (k = 0; k < piNumBattleShips[j]; k ++) {
                        ppbAlive[j][k] = false;
                    }
                    
                    piBattleShipsDestroyed[j] = piNumBattleShips[j];
                }

                // Get mine name
                if (piBattleEmpireIndex[iMineOwner] != INDEPENDENT) {

                    iErrCode = m_pGameData->ReadData (
                        pstrEmpireShips[piBattleEmpireIndex[iMineOwner]],
                        ppiBattleShipKey[iMineOwner][iMineIndex],
                        GameEmpireShips::Name,
                        &vMineName
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }
                }

            }   // End if mines

            //////////////////////////////
            // Conflict update messages //
            //////////////////////////////          
            
            // Was a ship destroyed?
            for (j = 0; j < iNumBattleEmpires; j ++) {
                if (piBattleShipsDestroyed[j] > 0) {
                    break;
                }
            }

            // Make ship lists
            if (j < iNumBattleEmpires) {

                if (pszPlanetName[0] == '\0') {

                    iErrCode = GetPlanetNameWithCoordinates (strGameMap, piPlanetKey[i], pszPlanetName);
                    if (iErrCode != OK) {
                        Assert (false);
                        strcpy (pszPlanetName, "Error");
                    }
                }

                // Add battle notification to update messages
                for (j = 0; j < iNumWatchers; j ++) {

                    pstrUpdateMessage[piWatcherIndex[j]] += BEGIN_STRONG "There was a fleet battle at ";
                    pstrUpdateMessage[piWatcherIndex[j]].AppendHtml (pszPlanetName, 0, false);
                    pstrUpdateMessage[piWatcherIndex[j]] += END_STRONG ":\n";

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

                // Destroy each empire's ships
                for (j = 0; j < iNumBattleEmpires; j ++) {

                    strList.Clear();

                    if (piBattleEmpireIndex[j] == INDEPENDENT) {

                        iCounter = piBattleShipsDestroyed[j];
                        
                        for (k = 0; k < piNumBattleShips[j]; k ++) {
                            
                            if (!ppbAlive[j][k]) {
                                
                                // Destroy ship
                                iErrCode = DeleteShip (iGameClass, iGameNumber, INDEPENDENT, ppiBattleShipKey[j][k]);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto OnError;
                                }
                            }
                        }

                    } else {

                        iCounter = 0;
                        
                        if (piBattleShipsDestroyed[j] > 0) {
                            
                            // Make a list of the empire's destroyed ships
                            for (k = 0; k < piNumBattleShips[j]; k ++) {

                                if (!ppbAlive[j][k]) {
                                    
                                    // Get ship name
                                    iErrCode = m_pGameData->ReadData (
                                        ppszEmpireShips[j], 
                                        ppiBattleShipKey[j][k], 
                                        GameEmpireShips::Name, 
                                        &vTemp
                                        );
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto OnError;
                                    }
                                    
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
                                    
                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto OnError;
                                    }
                                }
                            }
                        }
                        
                        // Add cloaked mine victims
                        if (bMines) {
                            
                            for (k = 0; k < piNumCloaked[j]; k ++) {
                                
                                // Get cloaked ship name
                                iErrCode = m_pGameData->ReadData (
                                    ppszEmpireShips[j], 
                                    ppiCloakerKey[j][k], 
                                    GameEmpireShips::Name, 
                                    &vTemp
                                    );

                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto OnError;
                                }
                                
                                // Add ship name to list
                                if (iCounter == 0) {
                                    strList.AppendHtml (vTemp.GetCharPtr(), 0, false);
                                } else {
                                    strList += ", ";
                                    strList.AppendHtml (vTemp.GetCharPtr(), 0, false);
                                }
                                iCounter ++;
                                
                                // Destroy the ship
                                iErrCode = DeleteShip (
                                    iGameClass, 
                                    iGameNumber, 
                                    piEmpireKey[piBattleEmpireIndex[j]], 
                                    ppiCloakerKey[j][k]
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto OnError;
                                }
                            }
                        }
                    }
                    
                    // Add to update messages
                    if (iCounter > 0) {

                        String strBattleMessage;

                        int iBattleShips = max (piNumBattleShips[j], iCounter);

                        if (piBattleEmpireIndex[j] == INDEPENDENT) {

                            strBattleMessage = BEGIN_STRONG;
                            strBattleMessage += iCounter;
                            strBattleMessage += END_STRONG " (of " BEGIN_STRONG;
                            strBattleMessage += iBattleShips;

                            strBattleMessage += END_STRONG ") " BEGIN_STRONG "Independent" END_STRONG " ship";

                            if (iBattleShips == 1) {
                                strBattleMessage += " was " BEGIN_STRONG "destroyed" END_STRONG " at ";
                            } else {
                                strBattleMessage += "s were " BEGIN_STRONG "destroyed" END_STRONG " at ";
                            }

                            strBattleMessage.AppendHtml (pszPlanetName, 0, false);
                            strBattleMessage += "\n";

                        } else {

                            strBattleMessage = BEGIN_STRONG;
                            strBattleMessage += iCounter;
                            strBattleMessage += END_STRONG " (of " BEGIN_STRONG;
                            strBattleMessage += iBattleShips;

                            if (iBattleShips == 1) {
                                strBattleMessage += END_STRONG ") ship";
                            } else {
                                strBattleMessage += END_STRONG ") ships";
                            }

                            strBattleMessage += " of " BEGIN_STRONG;
                            strBattleMessage += pvEmpireName[piBattleEmpireIndex[j]].GetCharPtr();
                            strBattleMessage += END_STRONG;

                            if (iBattleShips == 1) {
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

                // If a minefield exploded, add the innocent passer-by's cloakers to the list of the dead
                if (bMines) {

                    for (j = 0; j < iNumEmpires; j ++) {
                        
                        if (pbBattleEmpire[j] || piNumCloaked[j] == 0) {
                            continue;
                        }

                        // Get the cloaker keys
                        iErrCode = m_pGameData->GetEqualKeys (
                            pstrEmpireShips[j], 
                            GameEmpireShips::CurrentPlanet, 
                            piPlanetKey[i], 
                            false, 
                            &piShipKey, 
                            &iNumShips
                            );

                        if (iErrCode == ERROR_DATA_NOT_FOUND) {
                            iErrCode = OK;
                            continue;
                        }

                        else if (iErrCode != OK) {

                            Assert (false);
                            goto OnError;
                        }

                        iCounter = 0;

                        for (k = 0; k < (int) iNumShips; k ++) {
                            
                            iErrCode = m_pGameData->ReadData (
                                pstrEmpireShips[j], 
                                piShipKey[k], 
                                GameEmpireShips::State, 
                                &vTemp
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto OnError;
                            }
                            
                            if (vTemp.GetInteger() & CLOAKED) {

                                // Kill ship
                                iErrCode = m_pGameData->ReadData (
                                    pstrEmpireShips[j], 
                                    piShipKey[k], 
                                    GameEmpireShips::Name,
                                    &vTemp
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto OnError;
                                }
                                
                                if (iCounter == 0) {
                                    strList = vTemp.GetCharPtr();
                                } else {
                                    strList += ", ";
                                    strList += vTemp.GetCharPtr();
                                }
                                iCounter ++;
                                
                                // Destroy the ship
                                iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[j], piShipKey[k]);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto OnError;
                                }
                            }
                        }

                        FreeKeys (piShipKey);
                        piShipKey = NULL;

                        for (k = 0; k < iNumWatchers; k ++) {
                            
                            pstrUpdateMessage[piWatcherIndex[k]] += strList;
                            pstrUpdateMessage[piWatcherIndex[k]] += " of " BEGIN_STRONG; 
                            pstrUpdateMessage[piWatcherIndex[k]] += pvEmpireName[j].GetCharPtr();
                            pstrUpdateMessage[piWatcherIndex[k]] += END_STRONG;

                            if (iCounter == 1) {
                                pstrUpdateMessage[piWatcherIndex[k]] += " was "\
                                    BEGIN_STRONG "destroyed" END_STRONG " at ";
                            } else {
                                pstrUpdateMessage[piWatcherIndex[k]] += " were "\
                                    BEGIN_STRONG "destroyed" END_STRONG " at ";
                            }

                            pstrUpdateMessage[piWatcherIndex[k]].AppendHtml (pszPlanetName, 0, false);
                            pstrUpdateMessage[piWatcherIndex[k]] += "\n";
                        }

                    }   // End all empires loop
                }   // End if mines
            }   // End if some ship was destroyed
        
            // Clean up
            for (j = 0; j < iNumBattleEmpires; j ++) {

                delete [] ppiBattleShipKey[j];
                delete [] ppfBattleShipBR[j];
                delete [] ppbAlive[j];

                ppiBattleShipKey[j] = NULL;
                ppfBattleShipBR[j] = NULL;
                ppbAlive[j] = NULL;
            }

        }  // End of battle

    } // End planet loop

    return iErrCode;

OnError:

    if (piShipKey != NULL) {
        m_pGameData->FreeKeys (piShipKey);
    }

    if (piIndependentShipKey != NULL) {
        m_pGameData->FreeKeys (piIndependentShipKey);
    }

    for (i = 0; i < iNumBattleEmpires; i ++) {

        if (ppiBattleShipKey[i] != NULL) {
            delete [] ppiBattleShipKey[i];
        }

        if (ppfBattleShipBR[i] != NULL) {
            delete [] ppfBattleShipBR[i];
        }

        if (ppbAlive[i] != NULL) {
            delete [] ppbAlive[i];
        }
    }

    return iErrCode;
}


int GameEngine::MinefieldExplosion (const char* strGameMap, const char** pstrEmpireData, 
                                    unsigned int iPlanetKey, unsigned int iNumEmpires, 
                                    unsigned int* piEmpireKey, int* piTotalMin, int* piTotalFuel) {

    int iErrCode, iFinalPop;
    Variant vOwner, vPop;

    // Get planet owner
    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vOwner.GetInteger() != SYSTEM) {

        // Get pop
        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Pop, &vPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iFinalPop = (int) ceil ((float) vPop.GetInteger() / 2);
        Assert (iFinalPop >= 0);

        if (vOwner.GetInteger() != INDEPENDENT) {

            unsigned int iOwner;
            int iDiff;
            Variant vMin, vFuel;

            GetEmpireIndex (iOwner, vOwner);

            // Reduce owner's resources and econ
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

            iDiff = min (vMin.GetInteger(), iFinalPop) - min (vMin.GetInteger(), vPop.GetInteger());
            piTotalMin[iOwner] += iDiff;
            
            iDiff = min (vFuel.GetInteger(), iFinalPop) - min (vFuel.GetInteger(), vPop.GetInteger());
            piTotalFuel[iOwner] += iDiff;
            
            iErrCode = m_pGameData->Increment (
                pstrEmpireData[iOwner], 
                GameEmpireData::TotalPop, 
                iFinalPop - vPop.GetInteger()
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        // Reduce pop
        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Pop, iFinalPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

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
    unsigned int i, j, k, l, * piMineKey = NULL, iNumMines;

    String strPrefix, strMessage;
    Variant vPlanetName, vCoordinates, vNumShips, vOwner, vShipName, vType, vTemp;
    bool bSweep, bExist;

    const char* pszShipTable;

    unsigned int iNumAdjustedEmpires = iNumEmpires, iCurrentPlanetColumn, iTypeColumn;

    if (bIndependence) {
        iNumAdjustedEmpires ++;
    }

    unsigned int* piProxyPlanetKey = (unsigned int*) StackAlloc (iNumEmpires * sizeof (unsigned int));
    memset (piProxyPlanetKey, NO_KEY, iNumAdjustedEmpires * sizeof (unsigned int));

    unsigned int** ppiShipKey = (unsigned int**) StackAlloc (iNumEmpires * sizeof (unsigned int*));
    memset (ppiShipKey, NULL, iNumAdjustedEmpires * sizeof (unsigned int*));

    unsigned int* piNumShips = (unsigned int*) StackAlloc (iNumEmpires * sizeof (unsigned int));
    memset (piNumShips, 0, iNumAdjustedEmpires * sizeof (unsigned int));

    // Loop through all empires
    for (i = 0; i < iNumEmpires; i ++) {
        
        if (!pbAlive[i]) {
            continue;
        }

        iErrCode = m_pGameData->GetEqualKeys (
            pstrEmpireShips[i], 
            GameEmpireShips::Action,
            DETONATE,
            false,
            &piMineKey, 
            &iNumMines
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            continue;
        }

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        for (j = 0; j < iNumMines; j ++) {

            // Does mine still exist?
            iErrCode = m_pGameData->DoesRowExist (pstrEmpireShips[i], piMineKey[j], &bExist);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (!bExist) {
                continue;
            }

            if (gcConfig.iShipBehavior & MINEFIELD_DISABLE_DETONATE) {
                
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireShips[i], 
                    piMineKey[j], 
                    GameEmpireShips::Action, 
                    STAND_BY
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                continue;
            }

            bSweep = false;

            // Get its planet
            iErrCode = m_pGameData->ReadData (
                pstrEmpireShips[i], 
                piMineKey[j], 
                GameEmpireShips::CurrentPlanet, 
                &vTemp
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            unsigned int iPlanetKey = vTemp.GetInteger();

            // Is there a minesweeper on the planet?
            for (k = 0; !bSweep && k < iNumAdjustedEmpires; k ++) {

                if (k == iNumEmpires) {

                    pszShipTable = strIndependentShips;
                    iCurrentPlanetColumn = GameIndependentShips::CurrentPlanet;
                    iTypeColumn = GameIndependentShips::Type;
    
                } else {

                    if (!pbAlive[k]) {
                        continue;
                    }
                
                    // Is planet on empire's map?
                    iErrCode = m_pGameData->GetFirstKey (
                        pstrEmpireMap[k], 
                        GameEmpireMap::PlanetKey,
                        iPlanetKey,
                        false, 
                        piProxyPlanetKey + k
                        );
                    
                    if (iErrCode == ERROR_DATA_NOT_FOUND) {
                        iErrCode = OK;
                        continue;
                    }
                    
                    else if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    pszShipTable = pstrEmpireShips[k];
                    iCurrentPlanetColumn = GameEmpireShips::CurrentPlanet;
                    iTypeColumn = GameEmpireShips::Type;
                }
                
                // Does empire have ships on planet?
                iErrCode = m_pGameData->GetEqualKeys (
                    pszShipTable,
                    iCurrentPlanetColumn,
                    iPlanetKey,
                    false,
                    ppiShipKey + k, 
                    piNumShips + k
                    );
                
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    iErrCode = OK;
                    continue;
                }
                
                else if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                for (l = 0; l < piNumShips[k]; l ++) {

                    iErrCode = m_pGameData->ReadData (
                        pszShipTable,
                        ppiShipKey[k][l],
                        iTypeColumn,
                        &vType
                        );
                    
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    if (vType.GetInteger() == MINESWEEPER) {
                        bSweep = true;
                        break;
                    }
                }
            }

            // Get ship name
            iErrCode = m_pGameData->ReadData (
                pstrEmpireShips[i],
                piMineKey[j],
                GameEmpireShips::Name,
                &vShipName
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Get planet name, coordinates, owner
            iErrCode = m_pGameData->ReadData (
                strGameMap, 
                iPlanetKey, 
                GameMap::Name, 
                &vPlanetName
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = m_pGameData->ReadData (
                strGameMap, 
                iPlanetKey, 
                GameMap::Coordinates, 
                &vCoordinates
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

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
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

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

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

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
                        }
                        
                    } else {

                        for (l = 0; l < piNumShips[k]; l ++) {

                            // Get ship name
                            iErrCode = m_pGameData->ReadData (
                                pstrEmpireShips[k], 
                                ppiShipKey[k][l],
                                GameEmpireShips::Name, 
                                &vShipName
                                );
                            
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            iErrCode = DeleteShip (
                                iGameClass, 
                                iGameNumber, 
                                piEmpireKey[k], 
                                ppiShipKey[k][l]
                                );
                            
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
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

            // Clean up
            for (k = 0; k < iNumAdjustedEmpires; k ++) {

                if (ppiShipKey[k] != NULL) {
                    m_pGameData->FreeKeys (ppiShipKey[k]);
                    ppiShipKey[k] = NULL;
                }
            }
            
        }   // End mine loop

        m_pGameData->FreeKeys (piMineKey);
        piMineKey = NULL;
    
    }   // End empire loop

Cleanup:

    if (piMineKey != NULL) {
        m_pGameData->FreeKeys (piMineKey);
    }

    // Clean up
    for (k = 0; k < iNumAdjustedEmpires; k ++) {
        
        if (ppiShipKey[k] != NULL) {
            m_pGameData->FreeKeys (ppiShipKey[k]);
        }
    }

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

    unsigned int i, j, k, iNumWatchers, iProxyKey, iNumAdjustedEmpires, * piShipKey = NULL, iNumShips, 
        iStateColumn, iNumVisibleShips;

    unsigned int* piWatcherIndex = (unsigned int*) StackAlloc (iNumEmpires * sizeof (unsigned int));
    memset (piWatcherIndex, NO_KEY, iNumEmpires * sizeof (unsigned int));

    String strList;
    
    char pszPlanetName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH] = "";
    Variant vState, vName;

    const char* pszShips;

    iNumAdjustedEmpires = iNumEmpires;
    if (bIndependence) {
        iNumAdjustedEmpires ++;
    }

    for (i = 0; i < iNumPlanets; i ++) {

        // Calculate watchers
        iNumWatchers = 0;

        for (j = 0; j < iNumEmpires; j ++) {

            if (!pbAlive[j]) {
                continue;
            }

            // Is planet on empire's map?
            iErrCode = m_pGameData->GetFirstKey (
                pstrEmpireMap[j],
                GameEmpireMap::PlanetKey,
                piPlanetKey[i],
                false, 
                &iProxyKey
                );

            if (iErrCode == OK) {
                piWatcherIndex[iNumWatchers] = j;
                iNumWatchers ++;
            }

            else if (iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                goto Cleanup;
            }

            else {
                iErrCode = OK;
            }
        }

        if (iNumWatchers > 0) {

            for (j = 0; j < iNumAdjustedEmpires; j ++) {

                if (j == iNumEmpires) {

                    // Get independent ships
                    Assert (bIndependence);

                    iErrCode = m_pGameData->GetEqualKeys (
                        strIndependentShips,
                        GameIndependentShips::CurrentPlanet,
                        piPlanetKey[i],
                        false,
                        &piShipKey,
                        &iNumShips
                        );
                    
                    if (iErrCode != OK) {
                        
                        if (iErrCode == ERROR_DATA_NOT_FOUND) {
                            iErrCode = OK;
                        } else {
                            Assert (false);
                            goto Cleanup;
                        }
                    }

                    pszShips = strIndependentShips;
                    iStateColumn = GameIndependentShips::State;
                
                } else {

                    if (!pbAlive[j]) {
                        continue;
                    }

                    iErrCode = m_pGameData->GetEqualKeys (
                        pstrEmpireShips[j],
                        GameEmpireShips::CurrentPlanet,
                        piPlanetKey[i],
                        false,
                        &piShipKey,
                        &iNumShips
                        );

                    if (iErrCode != OK) {
                        
                        if (iErrCode == ERROR_DATA_NOT_FOUND) {
                            iErrCode = OK;
                        } else {
                            Assert (false);
                            goto Cleanup;
                        }
                    }
                    
                    pszShips = pstrEmpireShips[j];
                    iStateColumn = GameEmpireShips::State;
                }

                if (iNumShips > 0) {

                    Assert (iErrCode == OK);
                    
                    strList.Clear();
                    iNumVisibleShips = 0;

                    for (k = 0; k < iNumShips; k ++) {
                        
                        // Add to list if visible
                        iErrCode = m_pGameData->ReadData (
                            pszShips,
                            piShipKey[k], 
                            iStateColumn,
                            &vState
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        if (!(vState.GetInteger() & CLOAKED)) {

                            if (j < iNumEmpires) {
                            
                                iErrCode = m_pGameData->ReadData (
                                    pstrEmpireShips[j], 
                                    piShipKey[k], 
                                    GameEmpireShips::Name, 
                                    &vName
                                    );

                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
                                if (iNumVisibleShips > 0) {
                                    strList += ", ";
                                }
                                strList.AppendHtml (vName.GetCharPtr(), 0, false);
                            }

                            iNumVisibleShips ++;
                        }

                    }   // End ship loop

                    m_pGameData->FreeKeys (piShipKey);
                    piShipKey = NULL;


                    if (iNumVisibleShips > 0) {

                        String shipSighting;
                        
                        iErrCode = GetPlanetNameWithCoordinates (strGameMap, piPlanetKey[i], pszPlanetName);
                        if (iErrCode != OK) {
                            Assert (false);
                            strcpy (pszPlanetName, "Error");
                            iErrCode = OK;
                        }

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
                            Assert (j == iNumEmpires);

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

Cleanup:

    if (piShipKey != NULL) {
        m_pGameData->FreeKeys (piShipKey);
    }

    return iErrCode;
}


int GameEngine::UpdateFleetOrders (unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, 
                                   const char* strGameMap, const char** pstrEmpireShips, 
                                   const char** pstrEmpireFleets, const char** pstrEmpireMap,
                                     String* pstrUpdateMessage) {

    int iDirection, iErrCode = OK;
    unsigned int i, iFleetKey, * piShipKey = NULL, k, iNumShips;
    bool bFlag;

    float fStrength, fMaxStrength;
    Variant vTemp, vFleetPlanet, vAction;
    
    for (i = 0; i < iNumEmpires; i ++) {
        
        if (!pbAlive[i]) {
            continue;
        }

        iFleetKey = NO_KEY;
        while (true) {

            unsigned int iNumDefectedShips = 0;

            iErrCode = m_pGameData->GetNextKey (pstrEmpireFleets[i], iFleetKey, &iFleetKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Get ships in fleet
            iErrCode = m_pGameData->GetEqualKeys (
                pstrEmpireShips[i],
                GameEmpireShips::FleetKey,
                iFleetKey,
                false,
                &piShipKey,
                &iNumShips
                );

            if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                goto Cleanup;
            }

            if (iNumShips == 0) {

                // Check for auto-disband
                iErrCode = GetEmpireOption2 (piEmpireKey[i], DISBAND_EMPTY_FLEETS_ON_UPDATE, &bFlag);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (bFlag) {

                    Variant vFleetName;

                    iErrCode = m_pGameData->ReadData (
                        pstrEmpireFleets[i], 
                        iFleetKey, 
                        GameEmpireFleets::Name, 
                        &vFleetName
                        );

                    pstrUpdateMessage[i] += "Fleet ";
                    pstrUpdateMessage[i].AppendHtml (vFleetName.GetCharPtr(), 0, false);
                    pstrUpdateMessage[i] += " was disbanded\n";

                    iErrCode = m_pGameData->DeleteRow (pstrEmpireFleets[i], iFleetKey);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    continue;
                }
            }

            // Get fleet location
            iErrCode = m_pGameData->ReadData (
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::CurrentPlanet, 
                &vFleetPlanet
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            // Get fleet action
            iErrCode = m_pGameData->ReadData (
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::Action, 
                &vAction
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (vAction.GetInteger() <= MOVE_NORTH && 
                vAction.GetInteger() >= MOVE_WEST) { 

#ifdef _DEBUG
                Variant vBuildShips;
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireFleets[i], 
                    iFleetKey, 
                    GameEmpireFleets::BuildShips, 
                    &vBuildShips
                    );
                Assert (iErrCode == OK);
                Assert (vBuildShips.GetInteger() == 0);
#endif  
                iDirection = MOVE_NORTH - vAction.GetInteger();
                Assert (iDirection >= NORTH && iDirection <= WEST);
                
                iErrCode = m_pGameData->ReadData (
                    strGameMap, 
                    vFleetPlanet.GetInteger(), 
                    GameMap::NorthPlanetKey + iDirection, 
                    &vFleetPlanet
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                Assert (vFleetPlanet.GetInteger() != NO_KEY);

#ifdef _DEBUG
                unsigned int iPlanetKey;
                iErrCode = m_pGameData->GetFirstKey (
                    pstrEmpireMap[i],
                    GameEmpireMap::PlanetKey,
                    vFleetPlanet.GetInteger(),
                    false,
                    &iPlanetKey
                    );
                Assert (iErrCode == OK && iPlanetKey != NO_KEY);
#endif

                iErrCode = m_pGameData->WriteData (
                    pstrEmpireFleets[i], 
                    iFleetKey, 
                    GameEmpireFleets::CurrentPlanet, 
                    vFleetPlanet
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
            
            // Calculate fleet's current strength, max strength
            fStrength = (float) 0.0;
            fMaxStrength = (float) 0.0;
            
            for (k = 0; k < iNumShips; k ++) {

                iErrCode = m_pGameData->ReadData (
                    pstrEmpireShips[i], 
                    piShipKey[k], 
                    GameEmpireShips::CurrentPlanet, 
                    &vTemp
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (vTemp.GetInteger() != vFleetPlanet.GetInteger()) {
                    
                    // Defect from fleet
                    iErrCode = m_pGameData->WriteData (
                        pstrEmpireShips[i], 
                        piShipKey[k], 
                        GameEmpireShips::FleetKey, 
                        NO_KEY
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    iNumDefectedShips ++;

                } else {

                    // Keep in fleet
                    iErrCode = m_pGameData->WriteData (
                        pstrEmpireShips[i], 
                        piShipKey[k], 
                        GameEmpireShips::Action, 
                        FLEET
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    // Strength counts
                    iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[k], GameEmpireShips::MaxBR, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    fMaxStrength += (float) vTemp.GetFloat() * vTemp.GetFloat();
                    
                    iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[k], GameEmpireShips::CurrentBR, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    fStrength += (float) vTemp.GetFloat() * vTemp.GetFloat();
                }
            }

            // Write num ships
            iErrCode = m_pGameData->WriteData (
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::NumShips, 
                iNumShips - iNumDefectedShips
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            // Write new fleet strengths
            iErrCode = m_pGameData->WriteData (
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::MaxStrength, 
                fMaxStrength
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->WriteData (
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::CurrentStrength, 
                fStrength
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (iNumShips > 0) {
                m_pGameData->FreeKeys (piShipKey);
                piShipKey = NULL;
            }
            
            // Write standby as the new default action for the fleet
            iErrCode = m_pGameData->WriteData (
                pstrEmpireFleets[i], 
                iFleetKey, 
                GameEmpireFleets::Action, 
                STAND_BY
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

Cleanup:

        if (piShipKey != NULL) {
            m_pGameData->FreeKeys (piShipKey);
            piShipKey = NULL;
        }

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
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

    int iNumTraders, iNumAllies = 0, i, j, iNumBusinesses, iErrCode = OK, iEcon, iNumShips, iMil,
        iBonusMin, iBonusFuel, iBonusAg;
    
    unsigned int* piProxyKey;
    
    float fIncrease, fMil;
    
    Variant vTemp, * pvShipMil, vPop, vMaxPop, vTotalPop, vTotalAg, vMin, vFuel;

    for (i = 0; i < iNumEmpires; i ++) {

        if (!pbAlive[i]) {
            continue;
        }

        // Count number of empires at trade
        iErrCode = m_pGameData->GetEqualKeys (
            pstrEmpireDip[i], 
            GameEmpireDiplomacy::CurrentStatus, 
            TRADE, 
            false, 
            NULL, 
            (unsigned int*) &iNumTraders
            );
        
        // Count number of empires at alliance
        iErrCode = m_pGameData->GetEqualKeys (
            pstrEmpireDip[i], 
            GameEmpireDiplomacy::CurrentStatus, 
            ALLIANCE, 
            false, 
            NULL, 
            (unsigned int*) &iNumAllies
            );

#ifdef _DEBUG

        iErrCode = GetMaxNumDiplomacyPartners (iGameClass, iGameNumber, ALLIANCE, &j);
        Assert (iErrCode == OK && (j == UNRESTRICTED_DIPLOMACY || iNumAllies <= j));

        iErrCode = GetMaxNumDiplomacyPartners (iGameClass, iGameNumber, TRADE, &j);
        Assert (iErrCode == OK && (j == UNRESTRICTED_DIPLOMACY || iNumTraders <= j));
#endif

        // Add trade and alliance advantages to econ
        fIncrease = (float) gcConfig.iPercentFirstTradeIncrease / 100;
        iNumBusinesses = iNumTraders + iNumAllies;
        
        iBonusFuel = 0;
        iBonusMin = 0;
        iBonusAg = 0;
        
        for (j = 0; j < iNumBusinesses; j ++) {
            
            iBonusFuel += (int) ((float) (piTotalFuel[i] + iBonusFuel) * fIncrease);
            iBonusMin += (int) ((float) (piTotalMin[i] + iBonusMin) * fIncrease);
            iBonusAg += (int) ((float) (piTotalAg[i] + iBonusAg) * fIncrease);

            fIncrease *= (float) gcConfig.iPercentNextTradeIncrease / 100;
        }
        
        // Write new resource totals to database
        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::TotalFuel, piTotalFuel[i]);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::TotalMin, piTotalMin[i]);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::TotalAg, piTotalAg[i]);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::BonusFuel, iBonusFuel);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::BonusMin, iBonusMin);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::BonusAg, iBonusAg);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Write new econ to GameEmpireData
        iEcon = GetEcon (piTotalFuel[i] + iBonusFuel, piTotalMin[i] + iBonusMin, piTotalAg[i] + iBonusAg);

        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::Econ, iEcon);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        // Check MaxEcon
        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::MaxEcon, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        if (iEcon > vTemp.GetInteger()) {
            
            iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::MaxEcon, iEcon);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
        }
        
        // Calculate Mil
        fMil = (float) 0.0;
        iErrCode = m_pGameData->ReadColumn (
            pstrEmpireShips[i], 
            GameEmpireShips::CurrentBR, 
            &piProxyKey, 
            &pvShipMil, 
            (unsigned int*) &iNumShips
            );

#ifdef _DEBUG

        if (iErrCode == OK) {

            Variant vMaxBR, vType, vUse;
            int iFuelUse = 0, iMaintUse = 0;

            for (j = 0; j < iNumShips; j ++) {

                Assert (pvShipMil[j].GetFloat() >= 0.0);

                fMil += (float) pvShipMil[j] * pvShipMil[j];

                iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piProxyKey[j], GameEmpireShips::MaxBR, &vMaxBR);
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piProxyKey[j], GameEmpireShips::Type, &vType);
                Assert (iErrCode == OK);

                iFuelUse += GetFuelCost (vType, vMaxBR);
                iMaintUse += GetMaintenanceCost (vType, vMaxBR);
            }
            
            iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalFuelUse, &vUse);
            Assert (iErrCode == OK);

            Assert (vUse.GetInteger() == iFuelUse);

            iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TotalMaintenance, &vUse);
            Assert (iErrCode == OK);

            Assert (vUse.GetInteger() == iMaintUse);
            
            m_pGameData->FreeKeys (piProxyKey);
            m_pGameData->FreeData (pvShipMil);
        }
#else
        if (iErrCode == OK) {

            for (j = 0; j < iNumShips; j ++) {
                fMil += (float) pvShipMil[j] * pvShipMil[j];
            }
            
            m_pGameData->FreeKeys (piProxyKey);
            m_pGameData->FreeData (pvShipMil);
        }

        else if (iErrCode != ERROR_DATA_NOT_FOUND) {

            Assert (false);
            return iErrCode;
        }
#endif
        
        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::Mil, fMil);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        iMil = GetMilitaryValue (fMil);
        
        // MaxMil
        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::MaxMil, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        if (iMil > vTemp.GetInteger()) {
            
            iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::MaxMil, iMil);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
        }
        
        // Set TotalBuild to 0
        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::TotalBuild, 0);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Set NumBuilds to 0
        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::NumBuilds, 0);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Set NextMaintenance and NextFuelUse to 0
        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::NextMaintenance, 0);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pGameData->WriteData (pstrEmpireData[i], GameEmpireData::NextFuelUse, 0);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = WriteNextStatistics (
            iGameClass, 
            iGameNumber, 
            piEmpireKey[i],
            piTotalAg[i], 
            iBonusAg,
            fMaxAgRatio
            );

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

#ifdef _DEBUG
        iErrCode = CheckTruceTradeAllianceCounts (iGameClass, iGameNumber, piEmpireKey[i]);
        Assert (iErrCode == OK);
        iErrCode = OK;
#endif
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

    int iNumShips, i, j, k, l, iTemp, iErrCode = OK, iNumWatcherEmpires, iTerraform, iDiff, iInvadePop, iLinkX, 
        iLinkY, iX, iY;
    
    unsigned int* piShipKey, * piTempShipKey, * piTempEmpireKey, iKey;
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
            
        iErrCode = m_pGameData->GetAllKeys (pstrEmpireShips[i], &piShipKey, (unsigned int*) &iNumShips);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            continue;
        }
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Get developed techs
        iErrCode = m_pGameData->ReadData (pstrEmpireData[i], GameEmpireData::TechDevs, &vTechs);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Randomize ship keys
        Algorithm::Randomize (piShipKey, iNumShips);
        
        for (j = 0; j < iNumShips; j ++) {
            
            // Get ship action
            iErrCode = m_pGameData->ReadData (
                pstrEmpireShips[i], 
                piShipKey[j], 
                GameEmpireShips::Action, 
                &vAction
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            bDied = bDismantle = false;
            
            if (vAction.GetInteger() == NUKE) {

#ifdef _DEBUG
                // Make sure not cloaked
                iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::State, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (vTemp.GetInteger() & CLOAKED) {
                    Assert (false);
                    continue;
                }

                // Make sure mobile
                iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vTemp);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (!IsMobileShip (vTemp.GetInteger())) {
                    Assert (false);
                    continue;
                }

                // Make sure not illegally nuking science ship
                if (vTemp.GetInteger() == SCIENCE && (iGameClassOptions & DISABLE_SCIENCE_SHIPS_NUKING)) {
                    Assert (false);
                    continue;
                }
#endif
                // Add to nuke lookup tables
                iErrCode = m_pGameData->ReadData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::CurrentPlanet, 
                    &vTemp
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
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
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Action, 
                    STAND_BY
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

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
            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            unsigned int iPlanetKey = vTemp.GetInteger();

            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);
            
            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vShipType);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Name, &vShipName);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentBR, &vBR);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

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

                    iErrCode = m_pGameData->GetFirstKey (
                        pstrEmpireMap[k], 
                        GameEmpireMap::PlanetKey, 
                        iPlanetKey, 
                        false, 
                        &iKey
                        );
                    
                    if (iErrCode == OK) {
                        piWatcherEmpire [iNumWatcherEmpires] = k;
                        iNumWatcherEmpires ++;
                    }
                    
                    else if (iErrCode == ERROR_DATA_NOT_FOUND) {
                        iErrCode = OK;
                    }
                    
                    else {
                        Assert (false);
                        goto Cleanup;
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
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Pop, &vPop);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Annihilated, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
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
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Ag, &vAg);
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

                        iMaxPop = GetMaxPop (vMin.GetInteger(), vFuel.GetInteger());

                        if (iGameClassOptions & USE_SC30_SURRENDERS) {

                            iErrCode = m_pGameData->ReadData (
                                strGameMap, 
                                iPlanetKey, 
                                GameMap::HomeWorld, 
                                &vTemp
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

                            if (vTemp.GetInteger() >= ROOT_KEY) {
                                                                
                                // Update scores
                                iErrCode = UpdateScoresOn30StyleSurrenderColonization (
                                    piEmpireKey[i],
                                    iPlanetKey,
                                    pvEmpireName[i].GetCharPtr(),
                                    iGameClass,
                                    iGameNumber,
                                    pszGameClassName
                                    );

                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }

                                // Restore planet name
                                if (sscanf (vPlanetName.GetCharPtr(), RUINS_OF, pszEmpireName) == 1) {

                                    iErrCode = m_pGameData->WriteData (
                                        strGameMap, 
                                        iPlanetKey,
                                        GameMap::Name,
                                        pszEmpireName
                                        );

                                    if (iErrCode != OK) {
                                        Assert (false);
                                        goto Cleanup;
                                    }
                                }

                                // Set not homeworld
                                iErrCode = m_pGameData->WriteData (
                                    strGameMap, 
                                    iPlanetKey,
                                    GameMap::HomeWorld,
                                    NOT_HOMEWORLD
                                    );
                                
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }

                                bColonizeRuins = true;
                            }
                        }

                        // If there was a previous owner, subtract the ag and a planet from him
                        if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {

                            // Find owner
                            GetEmpireIndex (k, vOwner);
                            
                            // Was the planet a homeworld?
                            iErrCode = m_pGameData->ReadData (
                                strGameMap, 
                                iPlanetKey, 
                                GameMap::HomeWorld, 
                                &vTemp
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            if (vTemp.GetInteger() == HOMEWORLD) {
                                
                                // Obliterated by a colony!
                                // No longer a HW - do this so DeleteEmpireFromGame doesn't divide the resources
                                iErrCode = m_pGameData->WriteData (
                                    strGameMap, 
                                    iPlanetKey, 
                                    GameMap::HomeWorld, 
                                    NOT_HOMEWORLD
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
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
                                    iGameNumber, pszGameClassName);
                                
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
                                // Send system message to dead player
                                strMessage = "You were colonized out of ";
                                strMessage += pszGameClassName;
                                strMessage += " ";
                                strMessage += iGameNumber;
                                strMessage += " by ";
                                strMessage += pvEmpireName[i].GetCharPtr();
                                
                                iErrCode = SendSystemMessage (piEmpireKey[k], strMessage, SYSTEM, MESSAGE_SYSTEM);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
                                // Delete empire
                                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[k], EMPIRE_COLONIZED, &guInfo);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
                            } else {

                                Variant vMaxPop;

                                // Remove the planet from his planet count
                                iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumPlanets, -1);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }

                                // Remove the ag from total ag
                                piTotalAg[k] -= vAg.GetInteger();

                                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }

                                // Remove max pop from his target pop total
                                iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::TargetPop, - vMaxPop.GetInteger());
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                            }
                        }

                        // Set the maxpop to something appropriate
                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::MaxPop, iMaxPop);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        // Make us the owner
                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Owner, piEmpireKey[i]);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        if (piOriginalPlanetOwner [iPlanetKey] == NO_KEY) {
                            piOriginalPlanetOwner [iPlanetKey] = vOwner.GetInteger();
                        }

                        // Increase number of empire's planets
                        iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumPlanets, 1);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

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

                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Pop, iTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        // Increase empire's pop
                        iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::TotalPop, iTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        // Increase empire's targetpop
                        iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::TargetPop, iMaxPop);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

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
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

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
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Pop, &vPop);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
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
                        
                        iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::Pop, iTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                            
                        // Increase empire's TotalPop
                        iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::TotalPop, iTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        // Increase min
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Minerals, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                            
                        if (vTemp.GetInteger() > vPop.GetInteger()) {

                            if (vTemp.GetInteger() < vPop.GetInteger() + iTemp) {
                                piTotalMin[i] += (vTemp.GetInteger() - vPop.GetInteger());
                            } else {
                                piTotalMin[i] += iTemp;
                            }
                        }
                            
                        // Increase fuel
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Fuel, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                            
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
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        bDied = true;

                    } else {
                    
                        ////////////////////////
                        // Deposit pop failed //
                        ////////////////////////

                        Assert (vAction.GetInteger() != COLONIZE);

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

                    Assert (false);
                }

                bUpdated = true;
                
                break;
                                
            case TERRAFORMER:
                    
                if (vAction.GetInteger() != TERRAFORM) {
                    
                    if (vAction.GetInteger() == TERRAFORM_AND_DISMANTLE) {
                        bDismantle = true;
                    } else {
                        Assert (false);
                        continue;
                    }
                }
                
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

                if (vFuel.GetInteger() > vMin.GetInteger()) {
                    iTemp = vFuel.GetInteger() - vAg.GetInteger();
                } else {
                    iTemp = vMin.GetInteger() - vAg.GetInteger();
                }

                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
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
                        iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::Ag, iTerraform);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
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
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

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
                        iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::Ag, iTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
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

                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                    }

                    // Set already terraformed if necessary
                    if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_MULTIPLE) {
                        Assert (iPlanetKey < iNumPlanets); // Should never fire
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
                        Assert (false);
                        continue;
                    }
                }

                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (vOwner.GetInteger() == (int) piEmpireKey[i] || vOwner.GetInteger() == SYSTEM) {
                
                    bActionFlag = false;
                
                } else {

                    bActionFlag = true;
                    
                    if (vOwner.GetInteger() != INDEPENDENT) {
                        
                        iErrCode = m_pGameData->GetFirstKey (
                            pstrEmpireDip[i], 
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
                            pstrEmpireDip[i], 
                            iKey, 
                            GameEmpireDiplomacy::CurrentStatus, 
                            &vTemp
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
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
                    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Pop, &vPop);
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
                        
                        iErrCode = m_pGameData->Increment (strGameMap, iPlanetKey, GameMap::Pop, iTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        // Reduce owner's total pop
                        if (vOwner.GetInteger() != INDEPENDENT) {
                            
                            GetEmpireIndex (k, vOwner);
                            
                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::TotalPop, iTemp);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            // Reduce owner's resources and econ
                            iDiff = min (vPop.GetInteger() + iTemp, vMin.GetInteger()) - 
                                min (vPop.GetInteger(), vMin.GetInteger());
                            
                            piTotalMin[k] += iDiff;
                            
                            iDiff = min (vPop.GetInteger() + iTemp, vFuel.GetInteger()) - 
                                min (vPop.GetInteger(), vFuel.GetInteger());
                            
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
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        bDied = true;
                        
                    } else {
                       
                        ////////////////////////
                        // Invasion succeeded //
                        ////////////////////////
                        
                        // Change owner
                        iErrCode = m_pGameData->WriteData (
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::Owner, 
                            piEmpireKey[i]
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        if (piOriginalPlanetOwner [iPlanetKey] == NO_KEY) {
                            piOriginalPlanetOwner [iPlanetKey] = vOwner.GetInteger();
                        }

                        // Reduce planet pop
                        int iNewPop = - GetTroopshipSuccessPopDecrement (
                            gcConfig.fTroopshipSuccessFactor,
                            vPop.GetInteger()
                            );

                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Pop, iNewPop);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        // Save old max pop
                        Variant vOldMaxPop;
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::MaxPop, &vOldMaxPop);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        // Write new max pop
                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::MaxPop, iMaxPop);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        // Increase empire's maxpop
                        iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::TargetPop, iMaxPop);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        // Get planet resources
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Ag, &vAg);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        // Increase new owner's total pop and change number of planets
                        iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::TotalPop, iNewPop);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        iErrCode = m_pGameData->Increment (pstrEmpireData[i], GameEmpireData::NumPlanets, 1);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

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
                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::TotalPop, - vPop.GetInteger());
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumPlanets, -1);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::TargetPop, - vOldMaxPop.GetInteger());
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
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
                            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::HomeWorld, &vTemp);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            if (vTemp.GetInteger() == HOMEWORLD) {
                                
                                // No longer a HW - do this so DeleteEmpireFromGame doesn't divide the resources
                                iErrCode = m_pGameData->WriteData (
                                    strGameMap, 
                                    iPlanetKey, 
                                    GameMap::HomeWorld, 
                                    NOT_HOMEWORLD
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
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
                                    iGameNumber, pszGameClassName);

                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
                                // Send system message to dead player
                                strMessage = "You were invaded out of ";
                                strMessage += pszGameClassName;
                                strMessage += " ";
                                strMessage += iGameNumber;
                                strMessage += " by ";
                                strMessage += pvEmpireName[i].GetCharPtr();

                                iErrCode = SendSystemMessage (piEmpireKey[k], strMessage, SYSTEM, MESSAGE_SYSTEM);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }

                                // Delete empire's tables
                                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[k], EMPIRE_INVADED, &guInfo);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                            }
                        }
                        
                        // Handle ship's BR
                        if (iInvadePop == vPop.GetInteger()) {
                            
                            // Delete ship
                            iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
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

                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
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
                    Assert (false);
                    continue;
                }

                bActionFlag = true;

                // Make sure planet can be annihilated:
                // either an enemy's planet or an uncolonized planet or your own planet and not a HW
                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (vOwner.GetInteger() != SYSTEM && vOwner.GetInteger() != INDEPENDENT) {

                    if (vOwner.GetInteger() == (int) piEmpireKey[i]) {
                        
                        if (iGameClassOptions & DISABLE_SUICIDAL_DOOMSDAYS) {
                            
                            bActionFlag = false;
                        
                        } else {
                        
                            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::HomeWorld, &vTemp);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            if (vTemp.GetInteger() == HOMEWORLD) {
                                bActionFlag = false;
                            }
                        }

                    } else {

                        iErrCode = m_pGameData->GetFirstKey (pstrEmpireDip[i], GameEmpireDiplomacy::EmpireKey, vOwner, false, &iKey);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->ReadData (pstrEmpireDip[i], iKey, GameEmpireDiplomacy::CurrentStatus, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        if (vTemp.GetInteger() != WAR) {
                            
                            if (!(iGameClassOptions & USE_UNFRIENDLY_DOOMSDAYS)) {
                                
                                bActionFlag = false;
                                
                            } else {
                                
                                iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::HomeWorld, &vTemp);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
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
                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Owner, SYSTEM);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        if (piOriginalPlanetOwner [iPlanetKey] == NO_KEY) {
                            piOriginalPlanetOwner [iPlanetKey] = vOwner.GetInteger();
                        }
                        
                        // Reduce planet pop
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Pop, &vPop);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Pop, 0);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        if (vOwner.GetInteger() != INDEPENDENT) {

                            Variant vMaxPop;
                            
                            // Reduce owner's max pop
                            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::MaxPop, &vMaxPop);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

                            // Find owner
                            GetEmpireIndex (k, vOwner);
                            
                            // Read planet resources
                            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Ag, &vAg);
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

                            // Reduce owner's total pop, targetpop and change number of planets
                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::TotalPop, -vPop);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumPlanets, -1);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::TargetPop, - vMaxPop.GetInteger());
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

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
                            iErrCode = m_pGameData->ReadData (
                                strGameMap, 
                                iPlanetKey, 
                                GameMap::HomeWorld, 
                                &vTemp
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                            if (vTemp.GetInteger() == HOMEWORLD) {

                                // No longer a homeworld
                                iErrCode = m_pGameData->WriteData (
                                    strGameMap, 
                                    iPlanetKey, 
                                    GameMap::HomeWorld, 
                                    NOT_HOMEWORLD
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
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
                                    iGameNumber, pszGameClassName);

                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
                                // Send system message to dead player
                                strMessage = "You were annihilated out of ";
                                strMessage += pszGameClassName;
                                strMessage += " ";
                                strMessage += iGameNumber;
                                strMessage += " by ";
                                strMessage += pvEmpireName[i].GetCharPtr();
                                
                                iErrCode = SendSystemMessage (piEmpireKey[k], strMessage, SYSTEM, MESSAGE_SYSTEM);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                                
                                // Delete empire's tables
                                iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[k], EMPIRE_ANNIHILATED, &guInfo);
                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }
                            }
                        }
                    }

                    // Take note for jumpgate calculations
                    if (piOriginalNumObliterations [iPlanetKey] == ANNIHILATED_UNKNOWN) {
                        
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Annihilated, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        piOriginalNumObliterations [iPlanetKey] = vTemp.GetInteger();
                    }

                    // Reduce ag to zero
                    iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Ag, 0);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    // Add to update messages
                    strMessage.Clear();
                    strMessage.AppendHtml (vShipName.GetCharPtr(), 0, false);
                    strMessage += " of " BEGIN_STRONG;
                    strMessage += pvEmpireName[i].GetCharPtr();
                    
                    if (iGameClassOptions & USE_CLASSIC_DOOMSDAYS) {

                        strMessage += END_STRONG " permanently annihilated ";
                        
                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Minerals, 0);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Fuel, 0);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        iErrCode = m_pGameData->WriteData (strGameMap, iPlanetKey, GameMap::Annihilated, ANNIHILATED_FOREVER);

                    } else {

                        strMessage += END_STRONG " annihilated ";
                    
                        iErrCode = m_pGameData->Increment (
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::Annihilated, 
                            GetDoomsdayUpdates (gcConfig.fDoomsdayAnnihilationFactor, vBR.GetFloat())
                            );
                    }

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
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
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

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
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Link, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        if (vTemp.GetInteger() & LINK_X[iTemp]) {
                            bActionFlag = false;
                        }
                    }
                    
                    // Get other planet's data
                    iErrCode = m_pGameData->ReadData (
                        strGameMap, 
                        iPlanetKey, 
                        GameMap::NorthPlanetKey + iTemp, 
                        &vLinkPlanetKey
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, vLinkPlanetKey.GetInteger(), GameMap::Name, &vLinkPlanetName);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    AdvanceCoordinates (iX, iY, &iLinkX, &iLinkY, iTemp);

                    if (!bActionFlag) {

                        bTarget = false;

                        // Can the empire see the target planet?
                        iErrCode = m_pGameData->GetFirstKey (
                            pstrEmpireMap[i], 
                            GameEmpireMap::PlanetKey, 
                            vLinkPlanetKey.GetInteger(), 
                            false, 
                            &iKey
                            );

                        if (iErrCode == OK) {
                            bTarget = true;
                        }
                        
                        else if (iErrCode != ERROR_DATA_NOT_FOUND) {
                            Assert (false);
                            goto Cleanup;
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
                        iErrCode = m_pGameData->WriteOr (
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::Link, 
                            LINK_X[iTemp]
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
                        iErrCode = m_pGameData->WriteOr (
                            strGameMap,
                            vLinkPlanetKey.GetInteger(), 
                            GameMap::Link,
                            OPPOSITE_LINK_X[iTemp]
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
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
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                        } else {
                            
                            // Delete ship
                            iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

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
                            iErrCode = m_pGameData->GetFirstKey (
                                pstrEmpireMap[k], 
                                GameEmpireMap::PlanetKey, 
                                vLinkPlanetKey.GetInteger(), 
                                false, 
                                &iKey
                                );
                            
                            if (iErrCode == OK) {
                                bTarget = true;
                            }
                            
                            else if (iErrCode == ERROR_DATA_NOT_FOUND) {
                                bTarget = false;
                                iErrCode = OK;
                            }
                            
                            else {
                                Assert (false);
                                goto Cleanup;
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
                        iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Link, &vTemp);
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        if (!(vTemp.GetInteger() & LINK_X[iTemp])) {
                            bActionFlag = false;
                        }
                    }
                    
                    // Get other planet's data
                    iErrCode = m_pGameData->ReadData (
                        strGameMap, 
                        iPlanetKey, 
                        GameMap::NorthPlanetKey + iTemp, 
                        &vLinkPlanetKey
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    iErrCode = m_pGameData->ReadData (
                        strGameMap, 
                        vLinkPlanetKey.GetInteger(), 
                        GameMap::Name, 
                        &vLinkPlanetName
                        );
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    
                    AdvanceCoordinates (iX, iY, &iLinkX, &iLinkY, iTemp);

                    if (!bActionFlag) {

                        bTarget = false;

                        // Can the empire see the target planet?
                        iErrCode = m_pGameData->GetFirstKey (
                            pstrEmpireMap[i], 
                            GameEmpireMap::PlanetKey, 
                            vLinkPlanetKey.GetInteger(), 
                            false, 
                            &iKey
                            );

                        if (iErrCode == OK) {
                            bTarget = true;
                        }
                        
                        else if (iErrCode != ERROR_DATA_NOT_FOUND) {
                            Assert (false);
                            goto Cleanup;
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
                        iErrCode = m_pGameData->WriteAnd (
                            strGameMap, 
                            iPlanetKey, 
                            GameMap::Link,
                            ~LINK_X[iTemp]
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        iErrCode = m_pGameData->WriteAnd (
                            strGameMap, 
                            vLinkPlanetKey.GetInteger(), 
                            GameMap::Link,
                            ~OPPOSITE_LINK_X[iTemp]
                            );
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }
                        
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

                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
                        } else {
                            
                            // Delete ship
                            iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }
                            
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
                            iErrCode = m_pGameData->GetFirstKey (
                                pstrEmpireMap[k], 
                                GameEmpireMap::PlanetKey, 
                                vLinkPlanetKey.GetInteger(), 
                                false, 
                                &iKey
                                );
                            
                            if (iErrCode == OK) {
                                bTarget = true;
                            }
                            
                            else if (iErrCode == ERROR_DATA_NOT_FOUND) {
                                bTarget = false;
                                iErrCode = OK;
                            }
                            
                            else {
                                Assert (false);
                                goto Cleanup;
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
                        
                    Assert (false);
                        
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
                    iErrCode = m_pGameData->ReadData (
                        strGameMap, 
                        iPlanetKey, 
                        GameMap::NorthPlanetKey + iDirection, 
                        &vEmptyKey
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

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

                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

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
                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        bDied = true;
                    }

                    break;
                }

                bUpdated = true;
                break;

            default:

                // If not a stargate or a jumpgate, something bad happened
                Assert (vShipType.GetInteger() == STARGATE || vShipType.GetInteger() == JUMPGATE);

            } // End type switch

            ///////////////////////////////////////////////////
            // If the ship needs to be dismantled, do it now //
            ///////////////////////////////////////////////////

            if (bDismantle && !bDied) {
                
                // Delete the ship
                iErrCode = DeleteShip (iGameClass, iGameNumber, piEmpireKey[i], piShipKey[j]);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
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
                
                iErrCode = m_pGameData->WriteData (
                    pstrEmpireShips[i], 
                    piShipKey[j], 
                    GameEmpireShips::Action, 
                    STAND_BY
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
            
        } // End ship loop

Cleanup:

        m_pGameData->FreeKeys (piShipKey);

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

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
        
        iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::Owner, &vOwner);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
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
                    
                    iErrCode = m_pGameData->GetFirstKey (
                        pstrEmpireDip[j], 
                        GameEmpireDiplomacy::EmpireKey, 
                        vOwner, 
                        false, 
                        &iKey
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    iErrCode = m_pGameData->ReadData (
                        pstrEmpireDip[j], 
                        iKey, 
                        GameEmpireDiplomacy::CurrentStatus, 
                        &vTemp
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                }
                
                if (vTemp.GetInteger() == WAR) {
                    
                    // Get planet data
                    iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::Ag, &vAg);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::Minerals, &vMin);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::Fuel, &vFuel);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::Pop, &vPop);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    // Nuke planet
                    iErrCode = m_pGameData->WriteData (strGameMap, piNukedPlanetKey[i], GameMap::Owner, SYSTEM);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    iErrCode = m_pGameData->WriteData (strGameMap, piNukedPlanetKey[i], GameMap::Pop, 0);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    iErrCode = m_pGameData->Increment (strGameMap, piNukedPlanetKey[i], GameMap::Nuked, 1);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    // Reduce planet resources by half
                    iErrCode = m_pGameData->WriteData (strGameMap, piNukedPlanetKey[i], GameMap::Ag, (int) (vAg.GetInteger() / 2));
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    iErrCode = m_pGameData->WriteData (strGameMap, piNukedPlanetKey[i], GameMap::Fuel, (int) (vFuel.GetInteger() / 2));
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    iErrCode = m_pGameData->WriteData (strGameMap, piNukedPlanetKey[i], GameMap::Minerals, (int) (vMin.GetInteger() / 2));
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    // Save old max pop
                    iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::MaxPop, &vMaxPop);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    // Set new max pop
                    vTemp = GetMaxPop (vFuel.GetInteger() / 2, vMin.GetInteger() / 2);

                    iErrCode = m_pGameData->WriteData (strGameMap, piNukedPlanetKey[i], GameMap::MaxPop, vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    // Was the nuked planet a home world?
                    iErrCode = m_pGameData->ReadData (
                        strGameMap, 
                        piNukedPlanetKey[i], 
                        GameMap::HomeWorld, 
                        &vTemp
                        );

                    if (vTemp.GetInteger() == HOMEWORLD) {

                        // No longer a homeworld
                        iErrCode = m_pGameData->WriteData (
                            strGameMap, 
                            piNukedPlanetKey[i], 
                            GameMap::HomeWorld, 
                            NOT_HOMEWORLD
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }
                        
                        Assert (vOwner.GetInteger() != INDEPENDENT);

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
                            pszGameClassName);

                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }

                        // Send system message to dead player
                        strMessage = "You were nuked out of ";
                        strMessage += pszGameClassName;
                        strMessage += " ";
                        strMessage += iGameNumber;
                        strMessage += " by ";
                        strMessage += pvEmpireName[j].GetCharPtr();

                        // Best effort
                        iErrCode = SendSystemMessage (piEmpireKey[k], strMessage, SYSTEM, MESSAGE_SYSTEM);
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }
                        
                        // Delete empire's tables
                        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, piEmpireKey[k], EMPIRE_NUKED, &guInfo);
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }

                        iNukedIndex = k;
                        
                    } else {
                        
                        if (vOwner.GetInteger() != INDEPENDENT) {

                            // Find owner's index
                            GetEmpireIndex (k, vOwner);

                            // Subtract resources from owner                        
                            // Reduce owner's total pop, targetpop and change number of planets                         
                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::TotalPop, -vPop);
                            if (iErrCode != OK) {
                                Assert (false);
                                return iErrCode;
                            }

                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::NumPlanets, -1);
                            if (iErrCode != OK) {
                                Assert (false);
                                return iErrCode;
                            }

                            iErrCode = m_pGameData->Increment (pstrEmpireData[k], GameEmpireData::TargetPop, -vMaxPop);
                            if (iErrCode != OK) {
                                Assert (false);
                                return iErrCode;
                            }

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
                    
                    iErrCode = m_pGameData->ReadData (pstrEmpireShips[j], ppiShipNukeKey[i][m], 
                        GameEmpireShips::Name, &vShipName);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::Name, 
                        &vPlanetName);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::Coordinates, 
                        &vCoord);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);

                    //////////////////////////////////////////////////////
                    // Handle the NUM_NUKES_BEFORE_ANNIHILATION'th nuke //
                    //////////////////////////////////////////////////////

                    iErrCode = m_pGameData->ReadData (strGameMap, piNukedPlanetKey[i], GameMap::Nuked, &vTemp);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }

                    if (vTemp.GetInteger() >= gcConfig.iNukesForQuarantine) {
                        
                        // Set ag to zero - destroyed ecosystem
                        iErrCode = m_pGameData->WriteData (strGameMap, piNukedPlanetKey[i], GameMap::Ag, 0);
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }
                        
                        // Increment annihilated updates
                        iErrCode = m_pGameData->Increment (
                            strGameMap, 
                            piNukedPlanetKey[i], 
                            GameMap::Annihilated, 
                            gcConfig.iUpdatesInQuarantine
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }
                    
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
                        if (!pbAlive[k]) {

                            if (k != iNukedIndex) {
                                continue;
                            }

                            iNukedIndex = iNumEmpires;
                            iErrCode = OK;

                        } else {

                            iErrCode = m_pGameData->GetFirstKey (
                                pstrEmpireMap[k], 
                                GameEmpireMap::PlanetKey, 
                                piNukedPlanetKey[i], 
                                false, 
                                &iKey
                                );
                        }

                        if (iErrCode == OK) {
                            pstrUpdateMessage[k] += strMessage;
                        }
                        
                        else if (iErrCode == ERROR_DATA_NOT_FOUND) {
                            iErrCode = OK;
                        }

                        else {
                            Assert (false);
                            return iErrCode;
                        }
                    }
                    
                    // Exit ship loop:  planet has been nuked
                    break;
                }
            }   // End if ship can nuke 
        }   // End nuking ship loop

        delete [] ppiShipNukeKey[i];
        ppiShipNukeKey[i] = NULL;
    }
    
    return iErrCode;
}

int GameEngine::SharePlanetsBetweenFriends (int iGameClass, int iGameNumber, 
                                            unsigned int iEmpireIndex1, unsigned int iEmpireIndex2,
                                            const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireData,
                                            const char* pszGameMap, unsigned int iNumEmpires, 
                                            unsigned int* piEmpireKey, int iDipLevel) {
    int iErrCode;

    unsigned int iNumPlanets1, iNumPlanets2, i, * piProxyKey1 = NULL, * piProxyKey2 = NULL, iNumAcquaintances1, 
        iNumAcquaintances2;
    
    Variant* pvPlanetKey1 = NULL, * pvPlanetKey2 = NULL, * pvAcquaintanceKey1 = NULL, 
        * pvAcquaintanceKey2 = NULL;

    // Read diplomacy tables
    iErrCode = m_pGameData->ReadColumn (
        pstrEmpireDip[iEmpireIndex1], 
        GameEmpireDiplomacy::EmpireKey, 
        &piProxyKey1, 
        &pvAcquaintanceKey1, 
        &iNumAcquaintances1
        );
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = m_pGameData->ReadColumn (
        pstrEmpireDip[iEmpireIndex2], 
        GameEmpireDiplomacy::EmpireKey, 
        &piProxyKey2, 
        &pvAcquaintanceKey2, 
        &iNumAcquaintances2
        );
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Read planet keys
    iErrCode = m_pGameData->ReadColumn (
        pstrEmpireMap[iEmpireIndex1], 
        GameEmpireMap::PlanetKey, 
        &pvPlanetKey1, 
        &iNumPlanets1
        );
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadColumn (
        pstrEmpireMap[iEmpireIndex2], 
        GameEmpireMap::PlanetKey, 
        &pvPlanetKey2, 
        &iNumPlanets2
        );
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

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
            iNumAcquaintances2
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    
    // Share 1's planets with 2
    for (i = 0; i < iNumPlanets2; i ++) {

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
            iNumAcquaintances1
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    if (pvPlanetKey1 != NULL) {
        m_pGameData->FreeData (pvPlanetKey1);
    }

    if (pvPlanetKey2 != NULL) {
        m_pGameData->FreeData (pvPlanetKey2);
    }

    if (pvAcquaintanceKey1 != NULL) {
        m_pGameData->FreeData (pvAcquaintanceKey1);
    }

    if (pvAcquaintanceKey2 != NULL) {
        m_pGameData->FreeData (pvAcquaintanceKey2);
    }

    if (piProxyKey1 != NULL) {
        m_pGameData->FreeKeys (piProxyKey1);
    }

    if (piProxyKey2 != NULL) {
        m_pGameData->FreeKeys (piProxyKey2);
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
                                           unsigned int iNumAcquaintances) {

    Assert ((pvAcquaintanceKey == NULL && piProxyKey == NULL) ||
            (pvAcquaintanceKey != NULL && piProxyKey != NULL && iNumAcquaintances > 0));

    // Check for planet already in empire's map
    unsigned int iKey, i, j;

    int iErrCode = m_pGameData->GetFirstKey (
        pstrEmpireMap[iEmpireIndex], 
        GameEmpireMap::PlanetKey,
        iPlanetKey,
        false,
        &iKey
        );
    
    if (iErrCode == OK) {
        return OK;
    }

    if (iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }

    Variant pvColData [GameEmpireMap::NumColumns], vNeighbourPlanetKey, * pvPassedPtr = pvAcquaintanceKey;

    // It's an unknown planet
    pvColData[GameEmpireMap::PlanetKey] = iPlanetKey;
    pvColData[GameEmpireMap::NumUncloakedShips] = 0;
    pvColData[GameEmpireMap::NumCloakedBuildShips] = 0;
    pvColData[GameEmpireMap::NumUncloakedBuildShips] = 0;
    pvColData[GameEmpireMap::NumCloakedShips] = 0;

    // For each direction, check if the planet is in 2's map
    int iExplored = 0;

    ENUMERATE_CARDINAL_POINTS (i) {

        // Get neighbour planet key
        iErrCode = m_pGameData->ReadData (
            pszGameMap, 
            iPlanetKey, 
            GameMap::NorthPlanetKey + i, 
            &vNeighbourPlanetKey
            );
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        // Search for neighbour planet key in empire's map
        if (vNeighbourPlanetKey.GetInteger() != NO_KEY) {
            
            iErrCode = m_pGameData->GetFirstKey (
                pstrEmpireMap[iEmpireIndex], 
                GameEmpireMap::PlanetKey,
                vNeighbourPlanetKey,
                false,
                &iKey
                );

            if (iErrCode == OK) {
                
                // The neighbour exists, so let's fix him up
                iErrCode = m_pGameData->WriteOr (
                    pstrEmpireMap[iEmpireIndex], 
                    iKey,
                    GameEmpireMap::Explored,
                    OPPOSITE_EXPLORED_X[i]
                    );
                Assert (iErrCode == OK);
                
                // Make planet aware of neighbour
                iExplored |= EXPLORED_X[i];
            }
            
            else if (iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                return iErrCode;
            }
        }
    }

    pvColData[GameEmpireMap::Explored] = iExplored;
    
    // Insert into empire's map
    iErrCode = m_pGameData->InsertRow (pstrEmpireMap[iEmpireIndex], pvColData);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Update max, min
    int iX, iY;
    Variant vMinX, vMaxX, vMinY, vMaxY, vCoord;
    
    iErrCode = m_pGameData->ReadData (pszGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);

    // Fix empire's min / max
    iErrCode = SetNewMinMaxIfNecessary (iGameClass, iGameNumber, piEmpireKey[iEmpireIndex], iX, iY);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Maybe empire has some friends to share with too?
    if (pvPassedPtr == NULL && iNumAcquaintances != 0) {
        
        iErrCode = m_pGameData->ReadColumn (
            pstrEmpireDip[iEmpireIndex], 
            GameEmpireDiplomacy::EmpireKey, 
            &piProxyKey, 
            &pvAcquaintanceKey, 
            &iNumAcquaintances
            );

        if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            return iErrCode;
        }
    }
    
    if (iNumAcquaintances > 0) {

        Variant vDipStatus;

        for (i = 0; i < iNumAcquaintances; i ++) {
            
            iErrCode = m_pGameData->ReadData (
                pstrEmpireDip[iEmpireIndex], 
                piProxyKey[i], 
                GameEmpireDiplomacy::CurrentStatus, 
                &vDipStatus
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
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
                    -1
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }   // End acquaintance loop
    
Cleanup:

        if (pvPassedPtr == NULL) {
            FreeData (pvAcquaintanceKey);
            FreeKeys (piProxyKey);
        }

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
    }   // End if acquaintances > 0

    return iErrCode;
}


int GameEngine::GetPlanetNameWithCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, 
                                              char pszName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH]) {

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    return GetPlanetNameWithCoordinates (strGameMap, iPlanetKey, pszName);
}

int GameEngine::GetPlanetNameWithCoordinates (const char* pszGameMap, unsigned int iPlanetKey, 
                                              char pszName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH]) {

    Variant vPlanetName, vCoord;
    int iX, iY, iErrCode;

    iErrCode = m_pGameData->ReadData (pszGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    iErrCode = m_pGameData->ReadData (pszGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    GetCoordinates (vCoord.GetCharPtr(), &iX, &iY);

    sprintf (pszName, "%s (%i,%i)", vPlanetName.GetCharPtr(), iX, iY);
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

    int iDestX, iDestY, iSrcX, iSrcY, iErrCode = OK;
    float fGateCost = 0, fRangeFactor;

    bool bGateSurvived, bRangeLimited, bFailed;
    
    Variant vNewPlanetKey, vOldPlanetName, vCoord, vTemp, vBR, vShipName, vPlanetName, 
        vShipType, vOwner, vAnnihilated;

    unsigned int* piGateEmpireIndex = (unsigned int*) StackAlloc (iNumEmpires * sizeof (unsigned int));
    unsigned int iNumGateEmpires;

    for (i = 0; i < iNumEmpires; i ++) {
        
        if (!pbAlive[i]) {
            continue;
        }
        
        iErrCode = m_pGameData->GetEqualKeys (
            pstrEmpireShips[i], 
            GameEmpireShips::Action, 
            GATE_SHIPS,
            false, 
            &piShipKey, 
            &iNumKeys
            );
        
        if (iErrCode != OK) {
            
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                continue;
            }

            Assert (false);
            return iErrCode;
        }
        
        // Randomize gate list
        Algorithm::Randomize<unsigned int> (piShipKey, iNumKeys);
        
        // Loop through all gating ships            
        for (j = 0; j < iNumKeys; j ++) {

            bFailed = false;
            bGateSurvived = true;
            
            // Get the key of the destination planet
            iErrCode = m_pGameData->ReadData (
                pstrEmpireShips[i], 
                piShipKey[j],
                GameEmpireShips::GateDestination,
                &vNewPlanetKey
                );

            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            // Get some data        
            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentBR, &vBR);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            
            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Name, &vShipName);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::CurrentPlanet, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            unsigned int iPlanetKey = vTemp.GetInteger();

            iErrCode = m_pGameData->ReadData (pstrEmpireShips[i], piShipKey[j], GameEmpireShips::Type, &vShipType);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Name, &vOldPlanetName);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }

            iErrCode = m_pGameData->ReadData (strGameMap, vNewPlanetKey.GetInteger(), GameMap::Name, &vPlanetName);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            
            // Get src coords
            iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Coordinates, &vCoord);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
                
            GetCoordinates (vCoord.GetCharPtr(), &iSrcX, &iSrcY);

            // Get dest coords
            iErrCode = m_pGameData->ReadData (strGameMap, vNewPlanetKey.GetInteger(), GameMap::Coordinates, &vCoord);
            if (iErrCode != OK) {
                Assert (false);
                goto OnError;
            }
            
            GetCoordinates (vCoord.GetCharPtr(), &iDestX, &iDestY);

            // Ship type specific information
            if (vShipType.GetInteger() == STARGATE) {

                // Get original owner
                if (piOriginalPlanetOwner [vNewPlanetKey.GetInteger()] == NO_KEY) {
                    
                    iErrCode = m_pGameData->ReadData (strGameMap, vNewPlanetKey.GetInteger(), GameMap::Owner, &vOwner);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }

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

                    iErrCode = m_pGameData->ReadData (strGameMap, vNewPlanetKey.GetInteger(), GameMap::Annihilated, &vAnnihilated);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto OnError;
                    }

                } else {

                    vAnnihilated = piOriginalNumObliterations [vNewPlanetKey.GetInteger()];
                }
            }

            else {
                    
                Assert (false);
                iErrCode = ERROR_CANNOT_GATE;
                goto OnError;
            }
                
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

                        iErrCode = GetDiplomaticStatus (
                            iGameClass,
                            iGameNumber,
                            piEmpireKey[i],
                            piEmpireKey[k],
                            NULL,
                            NULL,
                            &iStatus
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }

                        if (iStatus == ALLIANCE) {
                            piGateEmpireIndex[iNumGateEmpires ++] = k;
                        }
                    }
                }

                for (k = 0; k < iNumGateEmpires; k ++) {

                    unsigned int iGatedEmpireIndex = piGateEmpireIndex[k];

                    // Gate ships
                    iErrCode = GateShips (
                        i, 
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

                    if (iErrCode == OK) {
                        bGated = true;
                    }

                    else if (iErrCode != ERROR_EMPIRE_CANNOT_SEE_PLANET && iErrCode != ERROR_EMPIRE_HAS_NO_SHIPS) {
                        Assert (false);
                        goto OnError;
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
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }
                        
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
                        if (iErrCode != OK) {
                            Assert (false);
                            goto OnError;
                        }
                    }
                }

            }   // End can ship gate?

            // Set gate to stand by
            if (bGateSurvived) {

                iErrCode = m_pGameData->WriteData (
                    pstrEmpireShips[i],
                    piShipKey[j],
                    GameEmpireShips::Action, 
                    STAND_BY
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto OnError;
                }
            }

        }   // End loop through all gates

        // If we reach this point, there's something to delete
        m_pGameData->FreeKeys (piShipKey);
        piShipKey = NULL;

    }   // End empire loop

    return iErrCode;

OnError:

    if (piShipKey != NULL) {
        m_pGameData->FreeKeys (piShipKey);
    }

    return iErrCode;
}



int GameEngine::GateShips (unsigned int iGaterEmpireIndex, const char* pszGaterEmpireName,
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

    unsigned int k, iOldProxyKey, iNewProxyKey, *piGateShipKey = NULL, iNumShips,
        iNumPrivateGatedShips, iNumPublicGatedShips;

    Variant vTemp;

    // Make sure we can see the planet
    iErrCode = m_pGameData->GetFirstKey (
        pszEmpireMap,
        GameEmpireMap::PlanetKey,
        iNewPlanetKey, 
        false,
        &iNewProxyKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return ERROR_EMPIRE_CANNOT_SEE_PLANET;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    Assert (iNewProxyKey != NO_KEY);

    // Get potential ships to gate
    iErrCode = m_pGameData->GetEqualKeys (
        pszEmpireShips,
        GameEmpireShips::CurrentPlanet,
        iOldPlanetKey,
        false,
        &piGateShipKey,
        &iNumShips
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        Assert (iGaterEmpireIndex != iGatedEmpireIndex);
        iErrCode = ERROR_EMPIRE_HAS_NO_SHIPS;
        goto Cleanup;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Maybe we're alone?
    if (iNumShips == 1 && iGaterEmpireIndex == iGatedEmpireIndex) {
        return ERROR_EMPIRE_HAS_NO_SHIPS;
    }
    
    ////////////////
    // Gate away! //
    ////////////////
                
    // Get proxy key of old planet
    iErrCode = m_pGameData->GetFirstKey (
        pszEmpireMap, 
        GameEmpireMap::PlanetKey, 
        iOldPlanetKey, 
        false, 
        &iOldProxyKey
        );
    
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

#ifdef _DEBUG
    {
    // Verify ship count on old planet
    Variant vU, vC;
    iErrCode = m_pGameData->ReadData (pszEmpireMap, iOldProxyKey, GameEmpireMap::NumCloakedShips, &vC);
    Assert (iErrCode == OK);
    
    iErrCode = m_pGameData->ReadData (pszEmpireMap, iOldProxyKey, GameEmpireMap::NumUncloakedShips, &vU);
    Assert (iErrCode == OK && vU + vC == iNumShips);
    }
#endif
                
    // Gate all mobile ships on planet
    iNumPrivateGatedShips = iNumPublicGatedShips = 0;
    
    for (k = 0; k < iNumShips; k ++) {
                    
        iErrCode = m_pGameData->ReadData (
            pszEmpireShips, 
            piGateShipKey[k], 
            GameEmpireShips::Type, 
            &vTemp
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        Assert (vTemp.GetInteger() >= FIRST_SHIP && vTemp.GetInteger() <= LAST_SHIP);
        
        if (IsMobileShip (vTemp.GetInteger())) {
                        
            ///////////////
            // Gate ship //
            ///////////////
            
            // Set ship location to new planet
            iErrCode = m_pGameData->WriteData (
                pszEmpireShips, 
                piGateShipKey[k], 
                GameEmpireShips::CurrentPlanet, 
                iNewPlanetKey
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            // Decrease number of ships at source planet, increment at new planet
            iErrCode = m_pGameData->ReadData (
                pszEmpireShips, 
                piGateShipKey[k], 
                GameEmpireShips::State, 
                &vTemp
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
                        
            if (vTemp.GetInteger() & CLOAKED) {
                
                iErrCode = m_pGameData->Increment (
                    pszEmpireMap, 
                    iOldProxyKey, 
                    GameEmpireMap::NumCloakedShips, 
                    -1
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = m_pGameData->Increment (
                    pszEmpireMap,
                    iNewProxyKey, 
                    GameEmpireMap::NumCloakedShips,
                    1
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = m_pGameData->Increment (
                    pszGameMap,
                    iOldPlanetKey, 
                    GameMap::NumCloakedShips, 
                    -1
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = m_pGameData->Increment (
                    pszGameMap,
                    iNewPlanetKey, 
                    GameMap::NumCloakedShips, 
                    1
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                            
            } else {
                
                iErrCode = m_pGameData->Increment (
                    pszEmpireMap,
                    iOldProxyKey, 
                    GameEmpireMap::NumUncloakedShips,
                    -1
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = m_pGameData->Increment (
                    pszEmpireMap,
                    iNewProxyKey, 
                    GameEmpireMap::NumUncloakedShips,
                    1
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iErrCode = m_pGameData->Increment (
                    pszGameMap,
                    iOldPlanetKey,
                    GameMap::NumUncloakedShips,
                    -1
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
#ifdef _DEBUG
                Variant vFooBar;
                iErrCode = m_pGameData->ReadData (pszGameMap, iOldPlanetKey, GameMap::NumUncloakedShips, &vFooBar);
                Assert (iErrCode == OK && vFooBar >= 0);
#endif
                iErrCode = m_pGameData->Increment (pszGameMap, iNewPlanetKey, GameMap::NumUncloakedShips, 1);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iNumPublicGatedShips ++;
            }
            
            iNumPrivateGatedShips ++;
        }
    }

    if (piGateShipKey != NULL) {
        m_pGameData->FreeKeys (piGateShipKey);
        piGateShipKey = NULL;
    }

    if (iNumPrivateGatedShips == 0) {
        iErrCode = ERROR_EMPIRE_HAS_NO_SHIPS;
    } else {

        // Gate fleets with ships also
        iErrCode = m_pGameData->GetEqualKeys (
            pszEmpireFleets,
            GameEmpireFleets::CurrentPlanet,
            iOldPlanetKey,
            false,
            &piGateShipKey, 
            &iNumShips
            );
                        
        if (iErrCode != OK) {

            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            } else {
                Assert (false);
                goto Cleanup;
            }
        }

        for (k = 0; k < iNumShips; k ++) {
            
            // Set fleet location to new planet
            iErrCode = m_pGameData->WriteData (
                pszEmpireFleets, 
                piGateShipKey[k], 
                GameEmpireFleets::CurrentPlanet, 
                iNewPlanetKey
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
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
    }

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
            iErrCode = m_pGameData->GetFirstKey (
                pstrEmpireMap[k], 
                GameEmpireMap::PlanetKey, 
                iOldPlanetKey,
                false,
                &iGateOldPlanetKey
                );

            if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->GetFirstKey (
                pstrEmpireMap[k], 
                GameEmpireMap::PlanetKey, 
                iNewPlanetKey,
                false,
                &iGateNewPlanetKey
                );
            
            if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = OK;
            
            if (iGateOldPlanetKey == NO_KEY && iGateNewPlanetKey != NO_KEY) {
                
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
            
            iErrCode = CheckForFirstContact (
                iGatedEmpireKey,
                iGatedEmpireIndex,
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
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
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

Cleanup:

    if (piGateShipKey != NULL) {
        m_pGameData->FreeKeys (piGateShipKey);
    }

    return iErrCode;
}

int GameEngine::CheckForFirstContact (int iEmpireKey, int i, int iPlanetKey, const char* pszPlanetName,
                                      int iNewX, int iNewY,
                                      unsigned int iNumEmpires,
                                      const unsigned int* piEmpireKey,
                                      const Variant* pvEmpireName,
                                      const char* strEmpireDip,
                                      const char* strGameMap,
                                      const char** pstrEmpireDip,
                                      String* pstrUpdateMessage
                                      ) {
    
    int iErrCode;
    unsigned int k, iKey;
    Variant vOwner;
    
    // Check for first contact with another empire
    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    if (vOwner.GetInteger() != SYSTEM && 
        vOwner.GetInteger() != INDEPENDENT && 
        vOwner.GetInteger() != iEmpireKey) {
        
        // Check for owner in diplomacy table
        iErrCode = m_pGameData->GetFirstKey (
            strEmpireDip, 
            GameEmpireDiplomacy::EmpireKey, 
            vOwner, 
            false, 
            &iKey
            );
        
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            
            ////////////////////////////////////
            // First contact (ship to planet) //
            ////////////////////////////////////

            String strTemp;
            Variant pvColData [GameEmpireDiplomacy::NumColumns];
            
            // Add owner empire to current empire's dip screen
            pvColData[GameEmpireDiplomacy::EmpireKey] = vOwner.GetInteger();
            pvColData[GameEmpireDiplomacy::DipOffer] = WAR;
            pvColData[GameEmpireDiplomacy::CurrentStatus] = WAR;
            pvColData[GameEmpireDiplomacy::VirtualStatus] = WAR;
            pvColData[GameEmpireDiplomacy::State] = 0;
            pvColData[GameEmpireDiplomacy::SubjectiveEcon] = 0;
            pvColData[GameEmpireDiplomacy::SubjectiveMil] = 0;
            pvColData[GameEmpireDiplomacy::LastMessageTargetFlag] = 0;
            
            iErrCode = m_pGameData->InsertRow (strEmpireDip, pvColData);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            // Add current empire to owner empire's dip screen
            GetEmpireIndex (k, (unsigned int) vOwner.GetInteger());
            
            pvColData[GameEmpireDiplomacy::EmpireKey] = iEmpireKey; // EmpireKey
            
            iErrCode = m_pGameData->InsertRow (pstrEmpireDip[k], pvColData);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            // Add to empires' update messages          
            strTemp.Clear();

            AddPlanetNameAndCoordinates (strTemp, pszPlanetName, iNewX, iNewY);
            
            pstrUpdateMessage[i] += "You have had " BEGIN_STRONG " first contact " END_STRONG " with " BEGIN_STRONG;
            pstrUpdateMessage[i] += pvEmpireName[k].GetCharPtr();
            pstrUpdateMessage[i] += END_STRONG " (ship to planet) at ";
            pstrUpdateMessage[i] += strTemp;
            pstrUpdateMessage[i] += "\n";
            
            pstrUpdateMessage[k] += "You have had " BEGIN_STRONG " first contact " END_STRONG " with " BEGIN_STRONG;
            pstrUpdateMessage[k] += pvEmpireName[i].GetCharPtr();
            pstrUpdateMessage[k] += END_STRONG " (ship to planet) at ";
            pstrUpdateMessage[k] += strTemp;
            pstrUpdateMessage[k] += "\n";
        }

    }   // End if planet belongs to someone else

    else if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}


int GameEngine::ProcessSubjectiveViews (int iGameClass, int iGameNumber,
                                        unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive,
                                        const char* strGameMap, const char** pstrEmpireMap, const char** pstrEmpireDip, 
                                        const char** pstrEmpireShips) {

    int iErrCode = OK;
    unsigned int i;

    for (i = 0; i < iNumEmpires; i ++) {

        if (!pbAlive[i]) continue;

        iErrCode = ProcessEmpireSubjectiveView (iGameClass, iGameNumber,
            piEmpireKey[i], strGameMap, iNumEmpires, piEmpireKey,
            pstrEmpireMap, pstrEmpireShips, pstrEmpireMap[i], pstrEmpireDip[i]);

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

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

    IReadTable* pReadTable = NULL;
    IWriteTable* pWriteEmpireDip = NULL;

    int* piPlanetKey = NULL, * piEmpireKey = NULL;
    unsigned int * piProxyPlanetKey = NULL, * piProxyEmpireKey = NULL, * piShipKey = NULL;

    Variant vTemp;

    // Get diplomacy rows
    iErrCode = m_pGameData->GetTableForReading (pszWriteEmpireDip, &pReadTable);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = pReadTable->ReadColumn (
        GameEmpireDiplomacy::EmpireKey, 
        &piProxyEmpireKey, 
        &piEmpireKey, 
        &iNumEmpires
        );

    SafeRelease (pReadTable);

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return OK;
    }

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    Assert (iNumEmpires > 0);

    // Allocate temp space for empire counts
    int* piAg = (int*) StackAlloc (iNumEmpires * sizeof(int) * 3);
    int* piMin = piAg + iNumEmpires;
    int* piFuel = piMin + iNumEmpires;

    float* pfMil = (float*) StackAlloc (iNumEmpires * sizeof (float));

    memset (piAg, 0, iNumEmpires * sizeof(int) * 3);
    memset (pfMil, 0, iNumEmpires * sizeof(float));

    // Loop through all visible planets
    iErrCode = m_pGameData->GetTableForReading (pszEmpireMap, &pReadTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pReadTable->ReadColumn (
        GameEmpireMap::PlanetKey, 
        &piProxyPlanetKey, 
        &piPlanetKey, 
        &iNumPlanets
        );

    SafeRelease (pReadTable);

    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        goto Cleanup;
    }

    for (i = 0; i < iNumPlanets; i ++) {

        iErrCode = m_pGameData->GetTableForReading (pszGameMap, &pReadTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Get planet's owner
        iErrCode = pReadTable->ReadData (piPlanetKey[i], GameMap::Owner, &iOwner);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        switch (iOwner) {

        case SYSTEM:
        case INDEPENDENT:
            break;

        default:

            if (iOwner != (int) iEmpireKey) {

                // Find empire
                for (j = 0; j < iNumEmpires; j ++) {

                    if (piEmpireKey[j] == iOwner) {
                        iOwnerIndex = j;
                        break;
                    }
                }

                if (j == iNumEmpires) {

                    // We haven't met this guy yet
                    SafeRelease (pReadTable);
                    continue;
                }

                //
                // Add planet's resources being used to running total
                //

                // Get pop
                iErrCode = pReadTable->ReadData (piPlanetKey[i], GameMap::Pop, &iPop);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                // Get ag
                iErrCode = pReadTable->ReadData (piPlanetKey[i], GameMap::Ag, &iAg);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                // Add min and fuel?
                if (iPop > 0) {

                    iErrCode = pReadTable->ReadData (piPlanetKey[i], GameMap::Minerals, &iMin);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    iErrCode = pReadTable->ReadData (piPlanetKey[i], GameMap::Fuel, &iFuel);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    piMin[iOwnerIndex] += min (iPop, iMin);
                    piFuel[iOwnerIndex] += min (iPop, iFuel);
                }

                piAg[iOwnerIndex] += iAg;
            }
        }   // End planet owner switch

        // Are there ships on the planet?
        iErrCode = pReadTable->ReadData (piPlanetKey[i], GameMap::NumUncloakedShips, &iNumVisibleShips);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pReadTable);

        // Don't need to worry about builds, because when this is run there are no build ships left
        iErrCode = m_pGameData->ReadData (pszEmpireMap, piProxyPlanetKey[i], GameEmpireMap::NumUncloakedShips, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iMyShips = vTemp.GetInteger();

        if (iMyShips < iNumVisibleShips) {

            iUnaccountedShips = iNumVisibleShips - iMyShips;

            // Scan for unaccounted ships
            for (j = 0; j < iNumEmpires; j ++) {

                GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, piEmpireKey[j]);

                iErrCode = m_pGameData->GetFirstKey (
                    pszEmpireMap,
                    GameEmpireMap::PlanetKey,
                    piPlanetKey[i],
                    false,
                    &iKey
                    );
                
                if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
                    Assert (false);
                    goto Cleanup;
                }

                if (iKey != NO_KEY) {

                    iErrCode = m_pGameData->ReadData (
                        pszEmpireMap,
                        iKey, 
                        GameEmpireMap::NumUncloakedShips, 
                        &vTemp
                        );

                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    iNumVisibleShips = vTemp.GetInteger();

                    if (iNumVisibleShips > 0) {

                        GAME_EMPIRE_SHIPS (pszEmpireShips, iGameClass, iGameNumber, piEmpireKey[j]);

                        // Get strength of ships at planet
                        iErrCode = m_pGameData->GetEqualKeys (
                            pszEmpireShips,
                            GameEmpireShips::CurrentPlanet,
                            piPlanetKey[i],
                            false,
                            &piShipKey,
                            &iNumShips
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            goto Cleanup;
                        }

                        Assert ((int) iNumShips >= iNumVisibleShips);

                        // Add mil of non-cloakers
                        for (k = 0; k < iNumShips; k ++) {

                            Variant vState;

                            // Cloaked?
                            iErrCode = m_pGameData->ReadData (
                                pszEmpireShips,
                                piShipKey[k],
                                GameEmpireShips::State,
                                &vState
                                );

                            if (iErrCode != OK) {
                                Assert (false);
                                goto Cleanup;
                            }

                            if (!(vState.GetInteger() & CLOAKED)) {

                                Variant vBR;

                                iErrCode = m_pGameData->ReadData (
                                    pszEmpireShips,
                                    piShipKey[k],
                                    GameEmpireShips::CurrentBR,
                                    &vBR
                                    );

                                if (iErrCode != OK) {
                                    Assert (false);
                                    goto Cleanup;
                                }

                                // Add mil at last
                                pfMil[j] += vBR.GetFloat() * vBR.GetFloat();
                            }
                        }

                        m_pGameData->FreeKeys (piShipKey);
                        piShipKey = NULL;

                        iUnaccountedShips -= iNumVisibleShips;

                        if (iUnaccountedShips == 0) {
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
    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = m_pGameData->GetTableForWriting (pszWriteEmpireDip, &pWriteEmpireDip);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Econ
        iErrCode = pWriteEmpireDip->WriteData (
            piProxyEmpireKey[i],
            GameEmpireDiplomacy::SubjectiveEcon,
            GetEcon (piFuel[i], piMin[i], piAg[i])
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Mil
        iErrCode = pWriteEmpireDip->WriteData (
            piProxyEmpireKey[i],
            GameEmpireDiplomacy::SubjectiveMil,
            GetMilitaryValue (pfMil[i])
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pWriteEmpireDip);
    }

Cleanup:

    SafeRelease (pReadTable);
    SafeRelease (pWriteEmpireDip);

    if (piPlanetKey != NULL) m_pGameData->FreeData (piPlanetKey);
    if (piProxyPlanetKey != NULL) m_pGameData->FreeKeys (piProxyPlanetKey);

    if (piEmpireKey != NULL) m_pGameData->FreeData (piEmpireKey);
    if (piProxyEmpireKey != NULL) m_pGameData->FreeKeys (piProxyEmpireKey);

    if (piShipKey != NULL) m_pGameData->FreeKeys (piShipKey);

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
    unsigned int* piProxyKey = NULL, iNumAcquaintances, i, j, iNewPlanetKey, iProxyKey;

    char pszCoord [MAX_COORDINATE_LENGTH + 1];
    char pszName [MAX_PLANET_NAME_LENGTH + 1];

    Variant* pvAcquaintanceKey = NULL;

    int iErrCode, iNewX, iNewY, iNeighbourX, iNeighbourY, iExplored = 0, piNeighbourKey [NUM_CARDINAL_POINTS],
        iNewAg, iNewMin, iNewFuel;

    AdvanceCoordinates (iX, iY, &iNewX, &iNewY, iDirection);

    // Name
    sprintf (pszName, "Planet %i,%i", iNewX, iNewY);

    // Coordinates
    GetCoordinates (iNewX, iNewY, pszCoord);

    // Create new planet
    Variant pvGameMap [GameMap::NumColumns] = {
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

    // Read avg resources
    iErrCode = m_pGameData->ReadData (strGameData, GameData::AvgAg, pvGameMap + GameMap::Ag);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strGameData, GameData::AvgMin, pvGameMap + GameMap::Minerals);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strGameData, GameData::AvgFuel, pvGameMap + GameMap::Fuel);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get real resources
    GetBuilderNewPlanetResources (
        fBR, 
        gcConfig.fBuilderBRDampener,
        gcConfig.fBuilderMultiplier,
        pvGameMap[GameMap::Ag].GetInteger(),
        pvGameMap[GameMap::Minerals].GetInteger(),
        pvGameMap[GameMap::Fuel].GetInteger(),
        &iNewAg,
        &iNewMin,
        &iNewFuel
        );

    pvGameMap[GameMap::Ag] = iNewAg;
    pvGameMap[GameMap::Minerals] = iNewMin;
    pvGameMap[GameMap::Fuel] = iNewFuel;

    // MaxPop
    pvGameMap[GameMap::MaxPop] = GetMaxPop (
        pvGameMap[GameMap::Minerals].GetInteger(), 
        pvGameMap[GameMap::Fuel].GetInteger()
        );

    // Surrounding keys
    ENUMERATE_CARDINAL_POINTS(i) {

        if (i == (unsigned int) OPPOSITE_CARDINAL_POINT [iDirection]) {

            pvGameMap[GameMap::NorthPlanetKey + i] = piNeighbourKey[i] = iPlanetKey;
        
        } else {

            AdvanceCoordinates (iNewX, iNewY, &iNeighbourX, &iNeighbourY, i);

            iErrCode = GetPlanetKeyFromCoordinates (
                iGameClass, 
                iGameNumber, 
                iNeighbourX, 
                iNeighbourY, 
                piNeighbourKey + i
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        pvGameMap[GameMap::NorthPlanetKey + i] = piNeighbourKey[i];

        if (piNeighbourKey[i] != NO_KEY) {
            
            // Have we explored our neighbour?
            iErrCode = m_pGameData->GetFirstKey (
                strGameEmpireMap,
                GameEmpireMap::PlanetKey,
                piNeighbourKey[i],
                false,
                &iProxyKey
                );
            
            if (iErrCode == OK) {

                Assert (iProxyKey != NO_KEY);
                
                iExplored |= EXPLORED_X[i];
                
                // Set neighbour's explored bit
                iErrCode = m_pGameData->WriteOr (
                    strGameEmpireMap,
                    iProxyKey,
                    GameEmpireMap::Explored,
                    OPPOSITE_EXPLORED_X[i]
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
            
            else if (iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    // Insert row into game map
    iErrCode = m_pGameData->InsertRow (strGameMap, pvGameMap, &iNewPlanetKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Set link planet's link bit
    iErrCode = m_pGameData->WriteOr (
        strGameMap,
        iPlanetKey,
        GameMap::Link,
        LINK_X[iDirection]
        );
    
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Set neighbours' cardinal point keys
    ENUMERATE_CARDINAL_POINTS(i) {

        if (piNeighbourKey[i] != NO_KEY) {

            iErrCode = m_pGameData->WriteData (
                strGameMap,
                piNeighbourKey[i],
                GameMap::NorthPlanetKey + OPPOSITE_CARDINAL_POINT[i],
                iNewPlanetKey
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    // Fix empire's min / max
    iErrCode = SetNewMinMaxIfNecessary (iGameClass, iGameNumber, iEmpireKey, iNewX, iNewY);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Fix game's min / max
    iErrCode = SetNewMinMaxIfNecessary (iGameClass, iGameNumber, NO_KEY, iNewX, iNewY);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    { // Scope

        Variant pvGameEmpireMap [GameEmpireMap::NumColumns] = {

            iNewPlanetKey, //PlanetKey,
            iExplored, //Explored,
            0, //RESERVED0,
            0, //RESERVED1,
            0, //RESERVED2,
            0, //NumUncloakedShips,
            0, //NumCloakedBuildShips,
            0, //NumUncloakedBuildShips,
            0, //NumCloakedShips
        };
        
        iErrCode = m_pGameData->InsertRow (strGameEmpireMap, pvGameEmpireMap);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // If mapshare, add new planet to fellow sharer's maps
    iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::MapsShared, 
        &vDipLevel
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (vDipLevel != NO_DIPLOMACY) {
        
        iErrCode = m_pGameData->ReadColumn (
            strEmpireDip, 
            GameEmpireDiplomacy::EmpireKey, 
            &piProxyKey, 
            &pvAcquaintanceKey, 
            &iNumAcquaintances
            );
        
        if (iErrCode == OK) {
            
            for (i = 0; i < iNumAcquaintances; i ++) {
                
                iErrCode = m_pGameData->ReadData (
                    strEmpireDip, 
                    piProxyKey[i], 
                    GameEmpireDiplomacy::CurrentStatus, 
                    &vDipStatus
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
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
                        -1
                        );
                    
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
            }   // End acquaintance loop
        }   // End if acquaintances > 0
        
        else if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        
        else {
            Assert (false);
            goto Cleanup;
        }
    }   // End if mapshare

Cleanup:

    if (piProxyKey != NULL) {
        m_pGameData->FreeKeys (piProxyKey);
    }

    if (pvAcquaintanceKey != NULL) {
        m_pGameData->FreeData (pvAcquaintanceKey);
    }

    return iErrCode;
}

int GameEngine::ChangeShipTypeOrMaxBR (const char* pszShips, const char* pszEmpireData, 
                                       int iEmpireKey, int iShipKey, int iOldShipType, int iNewShipType, 
                                       float fBRChange) {

    int iErrCode, iOld, iNew;
    float fNewMaxBR, fOldMaxBR;
    Variant vTemp;

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

        // Handle removal from fleet if not mobile ship
        if (iEmpireKey != INDEPENDENT && IsMobileShip (iOldShipType) && !IsMobileShip (iNewShipType)) {

            iErrCode = m_pGameData->WriteData (pszShips, iShipKey, GameEmpireShips::FleetKey, NO_KEY);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            // UpdateFleetOrders will handle the fleet's accounting
        }
    }

    if (fBRChange != 0.0) {
    
        // Change ship stats
        iErrCode = m_pGameData->Increment (pszShips, iShipKey, iCurrentBRColumn, fBRChange);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        iErrCode = m_pGameData->Increment (pszShips, iShipKey, iMaxBRColumn, fBRChange, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        fOldMaxBR = vTemp.GetFloat();
    
    } else {

        iErrCode = m_pGameData->ReadData (pszShips, iShipKey, iMaxBRColumn, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        fOldMaxBR = vTemp.GetFloat();
    }

    if (iEmpireKey != INDEPENDENT) {

        fNewMaxBR = fOldMaxBR + fBRChange;

        // Change empire maintenance
        iOld = GetMaintenanceCost (iOldShipType, fOldMaxBR);
        iNew = GetMaintenanceCost (iNewShipType, fNewMaxBR);

        iErrCode = m_pGameData->Increment (pszEmpireData, GameEmpireData::TotalMaintenance, iNew - iOld);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Change empire fuel use
        iOld = GetFuelCost (iOldShipType, fOldMaxBR);
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