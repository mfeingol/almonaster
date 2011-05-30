//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998-2004 Max Attar Feingold (maf6@cornell.edu)
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

// Delete the player's tables from the given game:
//
// GameEmpireData(I.I.I)
// GameEmpireMessages(I.I.I)
// GameEmpireMap(I.I.I)
// GameEmpireShips(I.II)
// GameEmpireFleets(I.I.I)
// GameEmpireDiplomacy(I.I.I)

// TODO: use transaction

int GameEngine::DeleteEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, ReasonForRemoval rReason,
                                      const GameUpdateInformation* pguInfo) {

    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    Variant vOptions, vGameState, vTemp;

    char strGameEmpireData [256];

    // Independence?
    Variant vGameClassOptions;
    int iErrCode = m_pGameData->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::Options, 
        &vGameClassOptions
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    bool bIndependence = (vGameClassOptions.GetInteger() & INDEPENDENCE) != 0;
    bool bPermanentAlliances = (vGameClassOptions.GetInteger() & PERMANENT_ALLIANCES) != 0;

    // Get game state
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vGameState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Get empire's game options
    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Handle num updated
    if (vOptions.GetInteger() & UPDATED) {

        iErrCode = m_pGameData->Increment (strGameData, GameData::NumEmpiresUpdated, -1, &vTemp);
        Assert (iErrCode == OK);
    }

    // Handle num paused
    if (vOptions.GetInteger() & REQUEST_PAUSE) {
        
        // Decrement number of paused empires
        iErrCode = m_pGameData->Increment (strGameData, GameData::NumRequestingPause, -1);
        Assert (iErrCode == OK);
    }

    // Handle num drawing
    if (vOptions.GetInteger() & REQUEST_DRAW) {
        iErrCode = m_pGameData->Increment (strGameData, GameData::NumRequestingDraw, -1);
        Assert (iErrCode == OK);
    }
    
    // Handle num resigned
    if (vOptions.GetInteger() & RESIGNED) {
        iErrCode = m_pGameData->Increment (strGameData, GameData::NumEmpiresResigned, -1);
        Assert (iErrCode == OK);
    }

    // Delete empire from empirelist
    unsigned int i, j, iNumEmpires, iKey;

    iErrCode = m_pGameData->GetFirstKey (strEmpires, GameEmpires::EmpireKey, iEmpireKey, false, &iKey);
    if (iKey == NO_KEY) {
        Assert (false);
        return ERROR_FAILURE;
    }

    iErrCode = m_pGameData->DeleteRow (strEmpires, iKey);
    if (iKey == NO_KEY) {
        Assert (false);
        return ERROR_FAILURE;
    }

    // If game started, add empire to dead empire list
    if (rReason != EMPIRE_GAME_ENDED && 
        rReason != EMPIRE_QUIT && 
        rReason != EMPIRE_GAME_ENTRY_ERROR &&
        (vGameState.GetInteger() & STARTED)) {

        Variant pvTemp [GameDeadEmpires::NumColumns];

        if (GetEmpireName (iEmpireKey, pvTemp + GameDeadEmpires::Name) == OK) {
            
            Variant vIcon;
            if (GetEmpireProperty (iEmpireKey, SystemEmpireData::AlienKey, &vIcon) == OK) {

                pvTemp [GameDeadEmpires::Key] = iEmpireKey;
                pvTemp [GameDeadEmpires::Icon] = vIcon.GetInteger();

                pvTemp [GameDeadEmpires::Update] = pguInfo == NULL ? 0 : pguInfo->iUpdate;
                pvTemp [GameDeadEmpires::Reason] = rReason;

                if (GetEmpireProperty (
                    iEmpireKey, SystemEmpireData::SecretKey, pvTemp + GameDeadEmpires::SecretKey
                    ) == OK) {

                    GET_GAME_DEAD_EMPIRES (strGameEmpireData, iGameClass, iGameNumber);
                    iErrCode = m_pGameData->InsertRow (strGameEmpireData, pvTemp);
                    Assert (iErrCode == OK);
                }
            }
        }
    }

    // Read active empire keys
    Variant* pvEmpireKey;
    iErrCode = m_pGameData->ReadColumn (strEmpires, GameEmpires::EmpireKey, &pvEmpireKey, &iNumEmpires);

    // Remove from other empires' dip tables
    if (iErrCode == OK) {
        
        char strOtherEmpireDip [256];
        Variant vDipOffer, vDipStatus;
        int iDipCountStatus;

        for (i = 0; i < iNumEmpires; i ++) {    
            
            GET_GAME_EMPIRE_DIPLOMACY (strOtherEmpireDip, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
            
            iErrCode = m_pGameData->GetFirstKey (
                strOtherEmpireDip, 
                GameEmpireDiplomacy::EmpireKey, 
                iEmpireKey, 
                false, 
                &iKey
                );

            Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);
            
            if (iErrCode == OK) {

                Variant vState;

                GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
                
                // Collect info for dip counts
                iErrCode = m_pGameData->ReadData (strOtherEmpireDip, iKey, GameEmpireDiplomacy::DipOffer, &vDipOffer);
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->ReadData (strOtherEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->ReadData (strOtherEmpireDip, iKey, GameEmpireDiplomacy::State, &vState);
                Assert (iErrCode == OK);

                // Nuke the row
                iErrCode = m_pGameData->DeleteRow (strOtherEmpireDip, iKey);
                Assert (iErrCode == OK);

                if (vDipOffer.GetInteger() > vDipStatus.GetInteger() && vDipOffer.GetInteger() <= ALLIANCE) {
                    iDipCountStatus = vDipOffer.GetInteger();
                } else {
                    iDipCountStatus = vDipStatus.GetInteger();
                }

                // Reduce diplomatic counts
                switch (iDipCountStatus) {
                
                case WAR:
                    break;

                case ALLIANCE:

                    if (!bPermanentAlliances) {

                        // Decrement, since alliances aren't permanent
                        iErrCode = m_pGameData->Increment (strGameEmpireData, GameEmpireData::NumAlliances, -1);
                        Assert (iErrCode == OK);
                    }
                    
                    else if (vDipStatus.GetInteger() == ALLIANCE) {

                        // Increment leaked count
                        iErrCode = m_pGameData->Increment (strGameEmpireData, GameEmpireData::NumAlliancesLeaked, 1);
                        Assert (iErrCode == OK);
                    }

                    else {

                        // If we weren't once allied with this empire, decrement
                        if (iErrCode == OK && !(vState.GetInteger() & ONCE_ALLIED_WITH)) {

                            iErrCode = m_pGameData->Increment (strGameEmpireData, GameEmpireData::NumAlliances, -1);
                            Assert (iErrCode == OK);
                        }
                    }

                    // Always fall through...

                case TRADE:

                    iErrCode = m_pGameData->Increment (
                        strGameEmpireData,
                        GameEmpireData::NumTrades,
                        -1
                        );
                    Assert (iErrCode == OK);

                    // Fall through...

                case TRUCE:

                    iErrCode = m_pGameData->Increment (
                        strGameEmpireData,
                        GameEmpireData::NumTruces,
                        -1
                        );
#ifdef _DEBUG
                    iErrCode = CheckTruceTradeAllianceCounts (iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
                    Assert (iErrCode == OK);
#endif
                    break;

                default:

                    Assert (false);
                    break;
                }
            }
        }

        m_pGameData->FreeData (pvEmpireKey);
    }
    
    // Remove empire's ownership of planets from map
    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    
    unsigned int* piPlanetKey, iNumKeys = 0;
    int iMaxPop;
    iErrCode = m_pGameData->GetEqualKeys (strGameMap, GameMap::Owner, iEmpireKey, false, &piPlanetKey, &iNumKeys);

    // Set planets to the right ownership, 
    Variant vAg;
    if (iErrCode == OK) {

        Variant vMin, vFuel, vPop, vMaxAgRatio;

        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxAgRatio, 
            &vMaxAgRatio
            );
        Assert (iErrCode == OK);

        for (i = 0; i < iNumKeys; i ++) {

            // Resolve ownership and population
            iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Ag, &vAg);
            Assert (iErrCode == OK);

            if (bIndependence && vAg.GetInteger() > 0) {

                // Make planet independent
                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::Owner, INDEPENDENT);
                Assert (iErrCode == OK);

                // Set max pop to ag
                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::MaxPop, vAg);
                Assert (iErrCode == OK);

                // Calculate next population
                iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
                Assert (iErrCode == OK);

                vPop = GetNextPopulation (
                    vPop.GetInteger(), 
                    GetAgRatio (vAg.GetInteger(), vPop.GetInteger(), vMaxAgRatio.GetFloat())
                    );

                // Normalize to max pop
                if (vPop.GetInteger() > vAg.GetInteger()) {
                    vPop = vAg;
                }

                // Set max pop
                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::Pop, vPop);
                Assert (iErrCode == OK);

            } else {

                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::Owner, SYSTEM);
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::Pop, 0);
                Assert (iErrCode == OK);

                // Set max pop to the right thing
                iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Minerals, &vMin);
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Fuel, &vFuel);
                Assert (iErrCode == OK);

                iMaxPop = max (vMin, vFuel);
                if (iMaxPop == 0) {
                    iMaxPop = 1;
                }
                iErrCode = m_pGameData->WriteData (strGameMap, piPlanetKey[i], GameMap::MaxPop, iMaxPop);
                Assert (iErrCode == OK);
            }
        }

        m_pGameData->FreeKeys (piPlanetKey);
    }

    // Set the empire's HW to non-HW status if the map already was generated
    unsigned int iNumRows;
    if (m_pGameData->GetNumRows (strGameMap, &iNumRows) == OK && iNumRows > 0) {

        Variant vHWKey, vStillHW;
        iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::HomeWorld, &vHWKey);
        if (iErrCode == OK && vHWKey.GetInteger() != NO_KEY) {

            unsigned int iHWKey = vHWKey.GetInteger();

            // Is the homeworld still a homeworld?
            iErrCode = m_pGameData->ReadData (strGameMap, iHWKey, GameMap::HomeWorld, &vStillHW);
            Assert (iErrCode == OK);

            if (vStillHW.GetInteger() == HOMEWORLD) {

                Variant vAg, vMin, vFuel;

                // Mark as not homeworld
                iErrCode = m_pGameData->WriteData (strGameMap, iHWKey, GameMap::HomeWorld, NOT_HOMEWORLD);
                Assert (iErrCode == OK);

                // Halve resources
                iErrCode = m_pGameData->ReadData (strGameMap, iHWKey, GameMap::Ag, &vAg);
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->ReadData (strGameMap, iHWKey, GameMap::Minerals, &vMin);
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->ReadData (strGameMap, iHWKey, GameMap::Fuel, &vFuel);
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->WriteData (strGameMap, iHWKey, GameMap::Ag, (int) (vAg.GetInteger() / 2));
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->WriteData (strGameMap, iHWKey, GameMap::MaxPop, (int) (vAg.GetInteger() / 2));
                Assert (iErrCode == OK);

                iErrCode = m_pGameData->WriteData (strGameMap, iHWKey, GameMap::Fuel, (int) (vFuel.GetInteger() / 2));
                Assert (iErrCode == OK);
                
                iErrCode = m_pGameData->WriteData (strGameMap, iHWKey, GameMap::Minerals, (int) (vMin.GetInteger() / 2));
                Assert (iErrCode == OK);
            }
        }
    }

    // Handle ships
    unsigned int* piShipKey = NULL, iNumShips = 0;
    Variant* pvPlanetKey = NULL, * pvData = NULL;

    char strGameEmpireMap [256], strGameEmpireShips [256];
    
    iErrCode = m_pGameData->ReadColumn (strEmpireShips, GameEmpireShips::CurrentPlanet, &piShipKey, &pvPlanetKey, &iNumShips);
    Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);

    if (iNumShips > 0) {
        
        if (!bIndependence) {
            
            // Destroy ships: subtract them from planetary ship counts
            for (i = 0; i < iNumShips; i ++) {

                iErrCode = DeleteShipFromDeadEmpire (strEmpireShips, strGameMap, piShipKey[i], pvPlanetKey[i]);
                Assert (iErrCode == OK);
            }

        } else {
            
            Variant vOwner, vType, vBR, vMaxPop, vPop, vShipBehavior, vColonyMultipliedDepositFactor,
                vColonyExponentialDepositFactor, vTemp;

            unsigned int iInitPop, iOwnerKey;
            float fDecrease;
            bool bBuilding;

            unsigned int* piNumShipsAcquired = NULL;

            if (pguInfo != NULL) {
                piNumShipsAcquired = (unsigned int*) StackAlloc (pguInfo->iNumEmpires * sizeof (unsigned int));
                memset (piNumShipsAcquired, 0, pguInfo->iNumEmpires * sizeof (unsigned int));
            }

            // Get ship behavior
            iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ShipBehavior, &vShipBehavior);
            Assert (iErrCode == OK);
            
            iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ColonyMultipliedDepositFactor, &vColonyMultipliedDepositFactor);
            Assert (iErrCode == OK);
            
            iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ColonyExponentialDepositFactor, &vColonyExponentialDepositFactor);
            Assert (iErrCode == OK);

            GAME_INDEPENDENT_SHIPS (strIndependentShips, iGameClass, iGameNumber);

            for (i = 0; i < iNumShips; i ++) {

                // Default to death of ship
                iOwnerKey = NO_KEY;

                // Get owner of planet
                iErrCode = m_pGameData->ReadData (strGameMap, pvPlanetKey[i], GameMap::Owner, &vOwner);
                Assert (iErrCode == OK);

                switch (vOwner.GetInteger()) {
                    
                case INDEPENDENT:

                    // The ship becomes independent
                    iOwnerKey = INDEPENDENT;
                    break;

                case SYSTEM:

                    // The ship is on a system planet: it dies unless its a colony, in which case
                    // it colonizes the planet
                    iErrCode = m_pGameData->ReadData (strEmpireShips, piShipKey[i], GameEmpireShips::Type, &vType);
                    if (iErrCode == OK) {

                        if (vType.GetInteger() == COLONY) {

                            /////////////////////
                            // Colonize planet //
                            /////////////////////

                            // Read planet's ag
                            iErrCode = m_pGameData->ReadData (strGameMap, pvPlanetKey[i], GameMap::Ag, &vAg);

                            // Only colonize if ag > 0
                            if (vAg.GetInteger() > 0) {

                                // Set independent planet, MaxPop = Ag
                                if (vOwner.GetInteger() != INDEPENDENT) {

                                    Assert (vOwner.GetInteger() == SYSTEM);
                                    
                                    iErrCode = m_pGameData->WriteData (strGameMap, pvPlanetKey[i], GameMap::Owner, INDEPENDENT);
                                    Assert (iErrCode == OK);

                                    iErrCode = m_pGameData->WriteData (strGameMap, pvPlanetKey[i], GameMap::MaxPop, vAg);
                                    Assert (iErrCode == OK);
                                }
                                
                                // Get colony's capacity
                                iErrCode = m_pGameData->ReadData (strEmpireShips, piShipKey[i], GameEmpireShips::CurrentBR, &vBR);
                                if (iErrCode == OK) {

                                    iInitPop = GetColonizePopulation (
                                        vShipBehavior.GetInteger(), 
                                        vColonyMultipliedDepositFactor.GetFloat(), 
                                        vColonyExponentialDepositFactor.GetFloat(),
                                        vBR.GetFloat()
                                        );
                                    
                                    // Does ship exhaust all pop?
                                    if ((int) iInitPop <= vAg.GetInteger()) {
                                        
                                        // Ship will die
                                        iOwnerKey = NO_KEY;
                                        
                                    } else {
                                        
                                        // Calculate decrease
                                        int iInc = iInitPop - vAg.GetInteger();
                                        fDecrease = vBR.GetFloat() * (float) iInc / iInitPop;
                                        iInitPop = iInc;
                                        
                                        // Reduce ship BR
                                        iErrCode = m_pGameData->Increment (strEmpireShips, piShipKey[i], GameEmpireShips::CurrentBR, - fDecrease);
                                        Assert (iErrCode == OK);
                                        
                                        // Ship survives
                                        iOwnerKey = INDEPENDENT;
                                    }
                                    
                                    // Deposit population
                                    iErrCode = m_pGameData->Increment (strGameMap, pvPlanetKey[i], GameMap::Pop, iInitPop);
                                    Assert (iErrCode == OK);

                                    char pszMessage [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH + 96] = "";

                                    // Add to update message for observer empires
                                    if (pguInfo != NULL && pguInfo->pstrUpdateMessage != NULL) {

                                        for (j = 0; j < pguInfo->iNumEmpires; j ++) {
                                            
                                            if (!pguInfo->pbAlive[j]) {
                                                continue;
                                            }
                                            
                                            GAME_EMPIRE_MAP (pszEmpMap, iGameClass, iGameNumber, pguInfo->piEmpireKey[j]);
                                            
                                            iErrCode = m_pGameData->GetFirstKey (
                                                pszEmpMap,
                                                GameEmpireMap::PlanetKey,
                                                pvPlanetKey[i],
                                                false,
                                                &iKey
                                                );
                                            
                                            if (iErrCode == OK) {
                                                
                                                if (pszMessage[0] == '\0') {
                                                    
                                                    char pszPlanetName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH];
                                                    
                                                    iErrCode = GetPlanetNameWithCoordinates (
                                                        strGameMap,
                                                        pvPlanetKey[i].GetInteger(),
                                                        pszPlanetName
                                                        );
                                                    
                                                    if (iErrCode == OK) {
                                                        
                                                        sprintf (
                                                            pszMessage,
                                                            BEGIN_STRONG "%s" END_STRONG \
                                                            " was colonized by an independent ship\n",
                                                            pszPlanetName
                                                            );
                                                    }
                                                    
                                                    else Assert (false);
                                                }
                                                
                                                pguInfo->pstrUpdateMessage[j] += pszMessage;
                                            }
                                            
                                            else Assert (iErrCode == ERROR_DATA_NOT_FOUND);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    else Assert (false);

                    break;

                default:

                    // Someone owns the planet, so give the ship to the planet's owner,
                    // but only if he wants it
                    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, vOwner.GetInteger());

                    iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vTemp);
                    if (iErrCode == OK) {

                        if (!(vTemp.GetInteger() & REJECT_INDEPENDENT_SHIP_GIFTS)) {
                            iOwnerKey = vOwner.GetInteger();
                        }
                    }

                    else Assert (false);

                    break;
                }

                // Decide what to do with ship
                if (iOwnerKey == NO_KEY) {

                    // Remove ship
                    iErrCode = DeleteShipFromDeadEmpire (strEmpireShips, strGameMap, piShipKey[i], pvPlanetKey[i]);
                    Assert (iErrCode == OK);

                } else {

                    // Read ship data
                    iErrCode = m_pGameData->ReadRow (strEmpireShips, piShipKey[i], &pvData);
                    if (iErrCode == OK) {

                        // Set default parameters
                        pvData[GameEmpireShips::Action] = STAND_BY;
                        pvData[GameEmpireShips::FleetKey] = NO_KEY;

                        bBuilding = pvData[GameEmpireShips::BuiltThisUpdate].GetInteger() != 0;
                        if (bBuilding) {
                            pvData[GameEmpireShips::BuiltThisUpdate] = 0;
                        }

                        // Uncloak and build
                        if (pvData[GameEmpireShips::State].GetInteger() & CLOAKED) {

                            if (bBuilding) {

                                iErrCode = m_pGameData->Increment (strGameMap, pvPlanetKey[i], GameMap::NumCloakedBuildShips, -1);
                                Assert (iErrCode == OK);

                            } else {

                                iErrCode = m_pGameData->Increment (strGameMap, pvPlanetKey[i], GameMap::NumCloakedShips, -1);
                                Assert (iErrCode == OK);
                            }

                            iErrCode = m_pGameData->Increment (strGameMap, pvPlanetKey[i], GameMap::NumUncloakedShips, 1);
                            Assert (iErrCode == OK);

                            pvData[GameEmpireShips::State] = pvData[GameEmpireShips::State].GetInteger() & ~CLOAKED;

                        } else {

                            if (bBuilding) {

                                iErrCode = m_pGameData->Increment (strGameMap, pvPlanetKey[i], GameMap::NumUncloakedBuildShips, -1);
                                Assert (iErrCode == OK);

                                iErrCode = m_pGameData->Increment (strGameMap, pvPlanetKey[i], GameMap::NumUncloakedShips, 1);
                                Assert (iErrCode == OK);
                            }
                        }

                        if (iOwnerKey == INDEPENDENT) {

                            // The ship is independent
                            iErrCode = m_pGameData->InsertRow (strIndependentShips, pvData);
                            Assert (iErrCode == OK);

                        } else {

                            Assert (iOwnerKey != SYSTEM);

                            GET_GAME_EMPIRE_SHIPS (strGameEmpireShips, iGameClass, iGameNumber, iOwnerKey);

                            // Some empire now owns the ship
                            iErrCode = m_pGameData->InsertRow (strGameEmpireShips, pvData);
                            Assert (iErrCode == OK);

                            // Increment fuel and maintenance costs
                            iErrCode = m_pGameData->Increment (
                                strGameEmpireData,
                                GameEmpireData::TotalMaintenance,
                                GetMaintenanceCost (
                                    pvData[GameEmpireShips::Type].GetInteger(), 
                                    pvData[GameEmpireShips::MaxBR].GetFloat()
                                    )
                                );
                            Assert (iErrCode == OK);

                            iErrCode = m_pGameData->Increment (
                                strGameEmpireData,
                                GameEmpireData::TotalFuelUse,
                                GetFuelCost (
                                    pvData[GameEmpireShips::Type].GetInteger(), 
                                    pvData[GameEmpireShips::MaxBR].GetFloat()
                                    )
                                );
                            Assert (iErrCode == OK);

                            GET_GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iOwnerKey);

                            iErrCode = m_pGameData->GetFirstKey (
                                strGameEmpireMap,
                                GameEmpireMap::PlanetKey,
                                pvPlanetKey[i],
                                false,
                                &iKey
                                );

                            if (iErrCode == OK) {

                                iErrCode = m_pGameData->Increment (
                                    strGameEmpireMap, 
                                    iKey, 
                                    GameEmpireMap::NumUncloakedShips, 
                                    1
                                    );
                                Assert (iErrCode == OK);
                            }

                            else Assert (false);

                            if (pguInfo != NULL && pguInfo->piEmpireKey != NULL) {

                                for (j = 0; j < pguInfo->iNumEmpires; j ++) {

                                    if (pguInfo->piEmpireKey[j] == (int) iOwnerKey) {
                                        piNumShipsAcquired [j] ++;
                                    }
                                }
                            }
                        }

                        m_pGameData->FreeData (pvData);
                    }

                    else Assert (false);
                }
            }

            // Add to update message for recipient empire
            if (pguInfo != NULL && pguInfo->pstrUpdateMessage != NULL) {
                
                for (j = 0; j < pguInfo->iNumEmpires; j ++) {
                    
                    if (piNumShipsAcquired[j] > 0) {
                        
                        char pszMessage [128];                  
                        sprintf (
                            pszMessage,
                            "You acquired " BEGIN_STRONG "%i" END_STRONG " independent %s\n",
                            piNumShipsAcquired[j],
                            piNumShipsAcquired[j] == 1 ? "ship" : "ships"
                            );
                        
                        pguInfo->pstrUpdateMessage[j] += pszMessage;
                    }
                }
            }
        }

        m_pGameData->FreeKeys (piShipKey);
        m_pGameData->FreeData (pvPlanetKey);
    }

    ///////////////////
    // Delete tables //
    ///////////////////

    // Adjust maxecon and maxmil
    Variant vMax, vOld;
    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::MaxEcon, &vMax);
    Assert (iErrCode == OK);

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxEcon, &vOld);
    Assert (iErrCode == OK);
    
    if (vMax.GetInteger() > vOld.GetInteger()) {
        iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxEcon, vMax);
        Assert (iErrCode == OK);
    }
    
    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::MaxMil, &vMax);
    Assert (iErrCode == OK);

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxMil, &vOld);
    Assert (iErrCode == OK);
    
    if (vMax.GetInteger() > vOld.GetInteger()) {
        iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::MaxMil, vMax);
        Assert (iErrCode == OK);
    }

    char pszTable [512];

    // GameEmpireData(I.I.I)
    iErrCode = m_pGameData->DeleteTable (strEmpireData);
    Assert (iErrCode == OK);

    // GameEmpireMessages(I.I.I)
    GET_GAME_EMPIRE_MESSAGES (pszTable, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = m_pGameData->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    // GameEmpireMap(I.I.I)
    GET_GAME_EMPIRE_MAP (pszTable, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = m_pGameData->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    // GameEmpireShips(I.I.I)
    iErrCode = m_pGameData->DeleteTable (strEmpireShips);
    Assert (iErrCode == OK);
    
    // GameEmpireFleets(I.I.I)
    GET_GAME_EMPIRE_FLEETS (pszTable, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = m_pGameData->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    // GameEmpireDiplomacy(I.I.I)
    GET_GAME_EMPIRE_DIPLOMACY (pszTable, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = m_pGameData->DeleteTable (pszTable);
    Assert (iErrCode == OK);

    // Delete game from personal list
    SYSTEM_EMPIRE_ACTIVE_GAMES (strActiveGames, iEmpireKey);

    unsigned int iGames = 1;
    IWriteTable* pWrite = NULL;

    iErrCode = m_pGameData->GetTableForWriting (strActiveGames, &pWrite);
    Assert (iErrCode == OK);

    if (iErrCode == OK) {

        char pszData [MAX_GAMECLASS_GAMENUMBER_LENGTH + 1];
        GetGameClassGameNumber (iGameClass, iGameNumber, pszData);

        iErrCode = pWrite->GetFirstKey (SystemEmpireActiveGames::GameClassGameNumber, pszData, true, &iKey);
        Assert (iErrCode == OK);

        if (iErrCode == OK) {
            iErrCode = pWrite->DeleteRow (iKey);
            Assert (iErrCode == OK);
        }

        iErrCode = pWrite->GetNumRows (&iGames);
        Assert (iErrCode == OK);

        SafeRelease (pWrite);
    }

    // If last game, empire should be tested for deletion
    if (iGames == 0) {

        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Options, &vOld);
        Assert (iErrCode == OK);

        if (iErrCode == OK && (vOld.GetInteger() & EMPIRE_MARKED_FOR_DELETION)) {

            iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::SecretKey, &vOld);
            Assert (iErrCode == OK);

            if (iErrCode == OK) {

                // Fire at will
                iErrCode = QueueDeleteEmpire (iEmpireKey, vOld.GetInteger64());
                Assert (iErrCode == OK);
            }
        }
    }

    return OK;
}

int GameEngine::QueueDeleteEmpire (int iEmpireKey, int64 i64SecretKey) {

    EmpireIdentity* pid = new EmpireIdentity;
    pid->iEmpireKey = iEmpireKey;
    pid->i64SecretKey = i64SecretKey;

    return SendLongRunningQueryMessage (DeleteEmpireMsg, pid);
}

int GameEngine::DeleteEmpireMsg (LongRunningQueryMessage* pMessage) {

    EmpireIdentity* pid = (EmpireIdentity*) pMessage->pArguments;
    int iErrCode = pMessage->pGameEngine->DeleteEmpire (pid->iEmpireKey, &pid->i64SecretKey, false, false);

    delete pid;
    return iErrCode;
}

int GameEngine::RemoveEmpireFromGame (int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
                                      unsigned int iKillerEmpire) {

    int iErrCode;
    bool bFlag;

    Variant vEmpireName;
    char pszMessage [2 * MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + 192];

    // Make sure game still exists
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
    if (iErrCode != OK || !bFlag) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    // Is empire in the game
    iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
    if (iErrCode != OK || !bFlag) {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vState;
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
    if (iErrCode != OK) {
        return iErrCode;
    }

    Variant vAdmin;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    if (iKillerEmpire == NO_KEY) {
        
        // Make sure game is still open     
        if (!(vState.GetInteger() & STILL_OPEN)) {
            return ERROR_GAME_CLOSED;
        }

        // Get empire name
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        if (iErrCode != OK) {
            return iErrCode;
        }

        sprintf (pszMessage, "%s quit the game", vEmpireName.GetCharPtr());

    } else {
        
        // Get empire name
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        if (iErrCode != OK) {
            Assert (false);
            vEmpireName = "";
        }
        
        // Get admin name
        iErrCode = GetEmpireName (iKillerEmpire, &vAdmin);
        if (iErrCode != OK) {
            vAdmin = "";
        }

        sprintf (
            pszMessage, 
            "%s was removed from the game by the administrator %s", 
            vEmpireName.GetCharPtr(),
            vAdmin.GetCharPtr()
            );

        // Get gameclass name
        iErrCode = GetGameClassName (iGameClass, pszGameClassName);
        if (iErrCode != OK) {
            Assert (false);
            *pszGameClassName = '\0';
        }
    }
    
    // Actually remove empire from game
    iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_DELETING_EMPIRE);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    Variant vNumUpdates;
    iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode != OK) {
        return iErrCode;
    }

    GameUpdateInformation guInfo = { vNumUpdates.GetInteger(), 0, NULL, NULL, NULL};

    iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, 
        iKillerEmpire == NO_KEY ? EMPIRE_QUIT : EMPIRE_ADMIN_REMOVED, &guInfo);
    
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_DELETING_EMPIRE);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // Get number of empires remaining
    unsigned int iNumEmpires;
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    iErrCode = m_pGameData->GetNumRows (strGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    bool bGameAlive = true;
    switch (iNumEmpires) {
        
    case 1:

        // If the game is still open, let it continue
        if (vState.GetInteger() & STILL_OPEN) {
            break;
        }

        // Send a system message to the empire in question
        {
            Variant* pvEmpireKey;
            unsigned int iNumEmpires, i;

            iErrCode = m_pGameData->ReadColumn (
                strGameEmpires, 
                GameEmpires::EmpireKey, 
                &pvEmpireKey, 
                &iNumEmpires
                );

            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            int iErrCode2;
            for (i = 0; i < iNumEmpires; i ++) {

                if ((unsigned int) pvEmpireKey[i].GetInteger() != iEmpireKey) {

                    sprintf (
                        pszMessage, 
                        "%s was removed from %s %i by the administrator %s and the game was deleted by the server", 
                        vEmpireName.GetCharPtr(),
                        pszGameClassName,
                        iGameNumber,
                        vAdmin.GetCharPtr()
                        );
                    
                    // Best effort
                    iErrCode2 = SendSystemMessage (pvEmpireKey[i].GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
                    Assert (iErrCode2 == OK);
                    break;
                }
            }

            FreeData (pvEmpireKey);
        }

    case 0:
        
        // Game is over: clean up the game
        bGameAlive = false;
        iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        break;
        
    default:
        
        // Check diplomacy
        iErrCode = CheckGameForEndConditions (iGameClass, iGameNumber, pszMessage, &bFlag);
        Assert (iErrCode == OK);
        
        bGameAlive = !bFlag;

        // Check for pause
        if (bGameAlive) {

            iErrCode = CheckForDelayedPause (iGameClass, iGameNumber, &bFlag);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            if (bFlag) {
                BroadcastGameMessage (iGameClass, iGameNumber, "The game is now paused", SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
            }
        }

        break;                  
    }

    if (bGameAlive) {

        // The game didn't end, so broadcast event to remaining empires in game
        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vState);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    // If empire still exists, tell him what happened
    if (iKillerEmpire != NO_KEY) {

        iErrCode = DoesEmpireExist (iEmpireKey, &bFlag, NULL);
        if (iErrCode == OK && bFlag) {

            sprintf (
                pszMessage,
                "You were removed from %s %i by the administrator %s",
                pszGameClassName,
                iGameNumber,
                vAdmin.GetCharPtr()
                );

            SendSystemMessage (iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
        }
    }

    return iErrCode;
}


// Input:
// iGameClass -> GameClass
// iGameNumber -> GameNumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *piSuccesful-> OK if quit game, ERROR_FAILURE if couldn't
//
// Makes an empire quit a game
// The player can quit the game if the game hasn't started yet.
// If the player was the last player the game then the game is destroyed

int GameEngine::QuitEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire) {

    bool bExists;
    int iErrCode;

    iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bExists);
    if (iErrCode == OK) {

        if (!bExists) {
            iErrCode = ERROR_EMPIRE_IS_NOT_IN_GAME;
        } else {
            iErrCode = QuitEmpireFromGameInternal (iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
        }
    }

    return iErrCode;
}

int GameEngine::QuitEmpireFromGameInternal (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire) {

    // Has the game started yet?
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vStarted;
    int iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vStarted);
    if (iErrCode != OK) {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    if (vStarted.GetInteger() & STARTED) {
    
        // We failed
        return ERROR_GAME_HAS_STARTED;

    }

    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);
    Variant vAdminName;
    
    char pszMessage [1024];
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    
    // Get gameclass name
    if (iKillerEmpire != NO_KEY) {
        
        iErrCode = GetGameClassName (iGameClass, pszGameClassName);
        if (iErrCode != OK) {
            Assert (false);
            *pszGameClassName = '\0';
        }
        
        iErrCode = GetEmpireName (iKillerEmpire, &vAdminName);
        if (iErrCode != OK) {
            vAdminName = "";
        }
    }
    
    // How many empires left?
    unsigned int iNumEmpires;
    iErrCode = m_pGameData->GetNumRows (strEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // If one, then we're done with this game
    if (iNumEmpires == 1) {
        
        // Destroy the game
        iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
    } else {
        
        // Remove empire from game
        iErrCode = m_pGameData->WriteOr (strGameData, GameData::State, GAME_DELETING_EMPIRE);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_QUIT, NULL);

        int iErrCode2 = m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_DELETING_EMPIRE);
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        if (iErrCode2 != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // If empires remain, so broadcast to other players
        Variant vEmpireName;
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        if (iErrCode != OK) {
            Assert (false);
            vEmpireName = "";
        }
        
        if (iKillerEmpire == NO_KEY) {
            sprintf (pszMessage, "%s quit the game", vEmpireName.GetCharPtr());
        } else {
            sprintf (
                pszMessage, 
                "%s as removed from the game by the administrator %s", 
                vEmpireName.GetCharPtr(),
                vAdminName.GetCharPtr()
                );
        }
        
        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    // If empire still exists, tell him what happened
    if (iKillerEmpire != NO_KEY) {
        
        bool bFlag;
        iErrCode = DoesEmpireExist (iEmpireKey, &bFlag, NULL);
        if (iErrCode == OK && bFlag) {
            
            sprintf (
                pszMessage, 
                "You were removed from %s %i by the administrator %s",
                pszGameClassName,
                iGameNumber,
                vAdminName.GetCharPtr()
                );
            
            SendSystemMessage (iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
        }
    }

    return iErrCode;
}


int GameEngine::ResignEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;
    bool bFlag;

    // Has the game started yet?
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vState;
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (!(vState.GetInteger() & STARTED)) {
        // Game has to start for this option to work
        return ERROR_GAME_HAS_NOT_STARTED;
    }

    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);

    unsigned int iShipKey = NO_KEY;

    int iGameState;
    char pszMessage [256];
    Variant vOldResigned, vEmpireName;

    bool bResigned = false;

    // Set empire to paused
    iErrCode = RequestPause (iGameClass, iGameNumber, iEmpireKey, &iGameState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    int iGameClassOptions;
    iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iGameClassOptions & ALLOW_DRAW) {

        // Set empire to request draw
        iErrCode = RequestDraw (iGameClass, iGameNumber, iEmpireKey, &iGameState);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Set empire to resigned
    iErrCode = m_pGameData->WriteOr (strEmpireData, GameEmpireData::Options, RESIGNED);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Increment num resigned
    iErrCode = m_pGameData->Increment (strGameData, GameData::NumEmpiresResigned, 1, &vOldResigned);
    if (iErrCode != OK) {
        
        Assert (false);
        
        iErrCode = m_pGameData->WriteAnd (strEmpireData, GameEmpireData::Options, ~RESIGNED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        goto Cleanup;
    }

    bResigned = true;

    // Dismantle all ships
    while (true) {

        iErrCode = m_pGameData->GetNextKey (strEmpireShips, iShipKey, &iShipKey);
        if (iErrCode != OK) {
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            } else Assert (false);
            break;
        }

        iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, iShipKey);
        Assert (iErrCode == OK);
    }

    // Broadcast to other players
    iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
    if (iErrCode != OK) {
        Assert (false);
        vEmpireName = "";
    }
    
    sprintf (pszMessage, "%s resigned from the game", vEmpireName.GetCharPtr());

    iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    bool bUpdate;
    iErrCode = SetEmpireReadyForUpdate (iGameClass, iGameNumber, iEmpireKey, &bUpdate);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    // Kill game if all empires have resigned
    if (bResigned && !(vState.GetInteger() & STILL_OPEN)) {

        if (DoesGameExist (iGameClass, iGameNumber, &bFlag) == OK && bFlag) {

            int iNumEmpires;
            iErrCode = GetNumEmpiresInGame (iGameClass, iGameNumber, &iNumEmpires);
            if (iErrCode == OK && vOldResigned.GetInteger() + 1 == iNumEmpires) {

                iErrCode = RuinGame (iGameClass, iGameNumber, NULL);
                Assert (iErrCode == OK);
            }
        }
    }

    return iErrCode;
}


int GameEngine::UnresignEmpire (int iGameClass, int iGameNumber, int iEmpireKey, int iAdminKey) {

    int iErrCode;
    Variant vOptions;

    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    // Is empire resigned?
    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vOptions.GetInteger() & RESIGNED) {

        int iGameState;
        Variant vEmpireName, vAdminName;
        const char* pszAdminName;

        // Tolerate errors here
        iErrCode = GetEmpireName (iAdminKey, &vAdminName);
        if (iErrCode != OK) {
            pszAdminName = vAdminName.GetCharPtr();
        } else {
            iErrCode = OK;
            pszAdminName = NULL;
        }

        GAME_DATA (strGameData, iGameClass, iGameNumber);

        char pszMessage [2 * MAX_EMPIRE_NAME_LENGTH + 128];

        // No more pause
        iErrCode = RequestNoPause (iGameClass, iGameNumber, iEmpireKey, &iGameState);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Set empire to un-resigned
        iErrCode = m_pGameData->WriteAnd (strEmpireData, GameEmpireData::Options, ~RESIGNED);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Decrement num resigned
        iErrCode = m_pGameData->Increment (strGameData, GameData::NumEmpiresResigned, -1);
        if (iErrCode != OK) {

            Assert (false);
            
            int iErrCode2 = m_pGameData->WriteOr (strEmpireData, GameEmpireData::Options, RESIGNED);
            Assert (iErrCode2 == OK);

            goto Cleanup;
        }

        // Broadcast to other players
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        if (iErrCode != OK) {
            Assert (false);
            vEmpireName = "";
        }
        
        if (pszAdminName != NULL) {

            sprintf (
                pszMessage,
                "%s was restored to the game by the administrator %s",
                vEmpireName.GetCharPtr(),
                pszAdminName
                );

        } else {
            
            sprintf (
                pszMessage,
                "%s was restored to the game by an administrator",
                vEmpireName.GetCharPtr()
                );
        }
        
        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    return iErrCode;
}


int GameEngine::SurrenderEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, SurrenderType sType) {

    int iErrCode;
    unsigned int iEmpires, iNumAllies = 0, iNumEmpires;
    bool bFlag;

    Variant vTemp, vEmpireName, vHomeWorld, vNumUpdates, vSecretKey;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    char pszString [MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];

    // Game has to have started for this option to work
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (vTemp.GetInteger() & STILL_OPEN) {
        return ERROR_GAME_HAS_NOT_CLOSED;
    }

    // Is empire in the game?
    iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
    if (iErrCode != OK || !bFlag) {
        iErrCode = ERROR_EMPIRE_IS_NOT_IN_GAME;
        goto Cleanup;
    }

    // How many empires?
    iErrCode = m_pGameData->GetNumRows (strGameEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iNumEmpires == 1) {
        iErrCode = ERROR_NOT_ENOUGH_EMPIRES;
        goto Cleanup;
    }

    if (sType == NORMAL_SURRENDER && iNumEmpires != 2) {
        iErrCode = ERROR_NOT_ENOUGH_EMPIRES;
        goto Cleanup;
    }

    // Get empire name
    iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::SecretKey, &vSecretKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get gameclass name
    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get update count
    iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    int iUpdates = vTemp.GetInteger();

    if (sType == SC30_SURRENDER) {

        // Get empire's homeworld
        iErrCode = m_pGameData->ReadData (
            strGameEmpireData,
            GameEmpireData::HomeWorld,
            &vHomeWorld
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Mark homeworld with Almonaster score, significance, name hash value
        iErrCode = m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey, 
            SystemEmpireData::AlmonasterScore, 
            &vTemp
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameData->WriteData (
            strGameMap, 
            vHomeWorld.GetInteger(), 
            GameMap::SurrenderAlmonasterScore, 
            vTemp
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameData->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey, 
            SystemEmpireData::AlmonasterScoreSignificance, 
            &vTemp
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameData->WriteData (
            strGameMap, 
            vHomeWorld.GetInteger(), 
            GameMap::SurrenderAlmonasterSignificance, 
            vTemp
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Write hash
        iErrCode = m_pGameData->WriteData (
            strGameMap,
            vHomeWorld.GetInteger(),
            GameMap::SurrenderEmpireSecretKey,
            vSecretKey
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Get number of allies
        iErrCode = m_pGameData->GetEqualKeys (
            strDiplomacy,
            GameEmpireDiplomacy::CurrentStatus,
            ALLIANCE,
            false,
            NULL,
            &iEmpires
            );

        if (iErrCode == OK) {
            iNumAllies += iEmpires;
        }

        else if (iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }

        // Count trades as alliances
        iErrCode = m_pGameData->GetEqualKeys (
            strDiplomacy,
            GameEmpireDiplomacy::CurrentStatus,
            TRADE,
            false,
            NULL,
            &iEmpires
            );

        if (iErrCode == OK) {
            iNumAllies += iEmpires;
        }

        else if (iErrCode != ERROR_DATA_NOT_FOUND) {
            Assert (false);
            goto Cleanup;
        }

        // Write num allies
        iErrCode = m_pGameData->WriteData (
            strGameMap, 
            vHomeWorld.GetInteger(), 
            GameMap::SurrenderNumAllies, 
            iNumAllies
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Handle scoring
        iErrCode = UpdateScoresOn30StyleSurrender (
            iEmpireKey, 
            vEmpireName.GetCharPtr(), 
            iGameClass, 
            iGameNumber,
            iUpdates,
            pszGameClassName
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Set homeworld value to key
        iErrCode = m_pGameData->WriteData (strGameMap, vHomeWorld.GetInteger(), GameMap::HomeWorld, iEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // New name
        sprintf (pszString, RUINS_OF, vEmpireName.GetCharPtr());

        iErrCode = m_pGameData->WriteData (strGameMap, vHomeWorld.GetInteger(), GameMap::Name, pszString);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Prepare a game end message for remaining empires, just in case the game ends
    sprintf (
        pszString,
        "Congratulations! You have won %s %i. %s surrendered and the game ended", 
        pszGameClassName,
        iGameNumber,
        vEmpireName.GetCharPtr()
        );

    iErrCode = m_pGameData->ReadData (strGameData, GameData::NumUpdates, &vNumUpdates);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }   

    if (iNumEmpires == 2) {

        unsigned int iKey = NO_KEY, iWinnerKey = NO_KEY;

        // Find winner's key
        while (true) {

            iErrCode = m_pGameData->GetNextKey (strGameEmpires, iKey, &iKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = m_pGameData->ReadData (strGameEmpires, iKey, GameEmpires::EmpireKey, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iWinnerKey = vTemp.GetInteger();

            if (iWinnerKey != (unsigned int) iEmpireKey) {
                break;
            }
        }

        Assert (iWinnerKey != NO_KEY);

        if (sType == NORMAL_SURRENDER) {

            Variant vWinnerName;
            iErrCode = GetEmpireName (iWinnerKey, &vWinnerName);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Handle scoring
            iErrCode = UpdateScoresOnSurrender (
                iWinnerKey,
                iEmpireKey, 
                vWinnerName.GetCharPtr(), 
                vEmpireName.GetCharPtr(), 
                iGameClass, 
                iGameNumber,
                iUpdates,
                pszGameClassName
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        SendSystemMessage (iWinnerKey, pszString, SYSTEM, MESSAGE_SYSTEM);

        iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, iWinnerKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Delete surrendering empire from game
        {
            GameUpdateInformation guInfo = { vNumUpdates.GetInteger(), 0, NULL, NULL, NULL};

            iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_SURRENDERED, &guInfo);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_WIN);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

    } else {

        GameUpdateInformation guInfo = { vNumUpdates.GetInteger(), 0, NULL, NULL, NULL};
        
        Assert (sType != NORMAL_SURRENDER);

        // Delete surrendering empire from game
        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_SURRENDERED, &guInfo);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Check game for exit
        iErrCode = CheckGameForEndConditions (iGameClass, iGameNumber, pszString, &bFlag);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        if (!bFlag) {

            // Game is still alive - tell empires what happened
            sprintf (
                pszString,
                "%s surrendered from the game",
                vEmpireName.GetCharPtr()
                );
            
            BroadcastGameMessage (iGameClass, iGameNumber, pszString, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);

            // Check game for pause
            iErrCode = CheckForDelayedPause (iGameClass, iGameNumber, &bFlag);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (bFlag) {
                BroadcastGameMessage (iGameClass, iGameNumber, "The game is now paused", SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
            }
        }
    }

Cleanup:

    return iErrCode;
}


int GameEngine::DeleteShipFromDeadEmpire (const char* pszEmpireShips, const char* pszGameMap, 
                                          unsigned int iShipKey, unsigned int iPlanetKey) {
    
    int iErrCode;
    Variant vState, vBuilt;

    iErrCode = m_pGameData->ReadData (pszEmpireShips, iShipKey, GameEmpireShips::State, &vState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (pszEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vBuilt);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (vBuilt.GetInteger() != 0) {
        
        if (vState.GetInteger() & CLOAKED) {
            iErrCode = m_pGameData->Increment (pszGameMap, iPlanetKey, GameMap::NumCloakedBuildShips, -1);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
        } else {
            iErrCode = m_pGameData->Increment (pszGameMap, iPlanetKey, GameMap::NumUncloakedBuildShips, -1);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
        }
        
    } else {
        
        if (vState.GetInteger() & CLOAKED) {
            iErrCode = m_pGameData->Increment (pszGameMap, iPlanetKey, GameMap::NumCloakedShips, -1);
            Assert (iErrCode == OK);
        } else {
            iErrCode = m_pGameData->Increment (pszGameMap, iPlanetKey, GameMap::NumUncloakedShips, -1);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
#ifdef _DEBUG
            Variant vFooBar;
            iErrCode = m_pGameData->ReadData (pszGameMap, iPlanetKey, GameMap::NumUncloakedShips, &vFooBar);
            Assert (iErrCode == OK && vFooBar.GetInteger() >= 0);
#endif
        }
    }

    return iErrCode;
}


int GameEngine::GetEmpirePartialMapData (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbPartialMaps, 
                                         unsigned int* piCenterKey, unsigned int* piXRadius, 
                                         unsigned int* piRadiusY) {

    IReadTable* pGameEmpireData;

    GAME_EMPIRE_DATA (pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetTableForReading (pszGameEmpireData, &pGameEmpireData);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    int iOptions;
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::Options, &iOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(iOptions & PARTIAL_MAPS)) {

        SafeRelease (pGameEmpireData);

        *pbPartialMaps = false;
        *piCenterKey = NO_KEY;
        *piXRadius = 0;
        *piRadiusY = 0;

    } else {

        *pbPartialMaps = true;

        iErrCode = pGameEmpireData->ReadData (GameEmpireData::PartialMapCenter, (int*) piCenterKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pGameEmpireData->ReadData (GameEmpireData::PartialMapXRadius, (int*) piXRadius);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pGameEmpireData->ReadData (GameEmpireData::PartialMapYRadius, (int*) piRadiusY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    SafeRelease (pGameEmpireData);

    return iErrCode;
}

int GameEngine::SetEmpirePartialMapCenter (int iGameClass, int iGameNumber, int iEmpireKey, int iCenterKey) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->WriteData (
        strGameEmpireData,
        GameEmpireData::PartialMapCenter,
        iCenterKey
        );
}

int GameEngine::SetEmpirePartialMapXRadius (int iGameClass, int iGameNumber, int iEmpireKey, int iXRadius) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->WriteData (
        strGameEmpireData,
        GameEmpireData::PartialMapXRadius,
        iXRadius
        );
}

int GameEngine::SetEmpirePartialMapYRadius (int iGameClass, int iGameNumber, int iEmpireKey, int iYRadius) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->WriteData (
        strGameEmpireData,
        GameEmpireData::PartialMapYRadius,
        iYRadius
        );
}

int GameEngine::HasEmpireResignedFromGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbResigned) {

    Variant vOptions;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->ReadData (
        strGameEmpireData,
        GameEmpireData::Options,
        &vOptions
        );

    if (iErrCode != OK) {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }

    *pbResigned = (vOptions.GetInteger() & RESIGNED) != 0;

    return iErrCode;
}

int GameEngine::GetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
                                               int* piDefaultBuilderPlanet, int* piResolvedPlanetKey) {

    int iErrCode;
    Variant vDefault;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = m_pGameData->ReadData (
        strGameEmpireData,
        GameEmpireData::DefaultBuilderPlanet,
        &vDefault
        );
    
    if (iErrCode != OK) {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }

    int iResolvedPlanetKey, iDefaultBuilderPlanet = vDefault.GetInteger();

    if (iErrCode == OK) {
        
        switch (iDefaultBuilderPlanet) {
            
        case NO_DEFAULT_BUILDER_PLANET:

            iResolvedPlanetKey = NO_KEY;
            break;

        case HOMEWORLD_DEFAULT_BUILDER_PLANET:

            iErrCode = m_pGameData->ReadData (
                strGameEmpireData,
                GameEmpireData::HomeWorld,
                &vDefault
                );
            if (iErrCode != OK) {
                return iErrCode;
            }

            iResolvedPlanetKey = vDefault.GetInteger();
            break;

        case LAST_BUILDER_DEFAULT_BUILDER_PLANET:
            
            iErrCode = m_pGameData->ReadData (
                strGameEmpireData,
                GameEmpireData::LastBuilderPlanet,
                &vDefault
                );
            if (iErrCode != OK) {
                return iErrCode;
            }

            iResolvedPlanetKey = vDefault.GetInteger();
            break;

        default:

            iResolvedPlanetKey = iDefaultBuilderPlanet;
            break;
        }

        *piDefaultBuilderPlanet = iDefaultBuilderPlanet;
        *piResolvedPlanetKey = iResolvedPlanetKey;
    }

    return iErrCode;
}

int GameEngine::SetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
                                               int iDefaultBuildPlanet) {

    if (iDefaultBuildPlanet > NO_DEFAULT_BUILDER_PLANET ||
        iDefaultBuildPlanet < LAST_BUILDER_DEFAULT_BUILDER_PLANET) {

        bool bExists;   
        int iErrCode = DoesPlanetExist (iGameClass, iGameNumber, iDefaultBuildPlanet, &bExists);
        if (iErrCode != OK) {
            return iErrCode;
        }

        if (!bExists) {
            return ERROR_INVALID_ARGUMENT;
        }
    }

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->WriteData (
        strGameEmpireData,
        GameEmpireData::DefaultBuilderPlanet,
        iDefaultBuildPlanet
        );
}


int GameEngine::GetEmpireBR (int iGameClass, int iGameNumber, int iEmpireKey, int* piBR) {

    Variant vTech;
    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->ReadData (
        strGameEmpireData,
        GameEmpireData::TechLevel,
        &vTech
        );

    if (iErrCode == OK) {
        *piBR = GetBattleRank (vTech.GetFloat());
    }

    return iErrCode;
}

int GameEngine::GetEmpireMaintenanceRatio (int iGameClass, int iGameNumber, int iEmpireKey, 
                                           float* pfMaintenanceRatio) {

    int iMin, iBonusMin, iMaint, iBuild;
    IReadTable* pGameEmpireData;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetTableForReading (strGameEmpireData, &pGameEmpireData);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pGameEmpireData->ReadData (GameEmpireData::TotalMin, &iMin);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::BonusMin, &iBonusMin);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::TotalMaintenance, &iMaint);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::TotalBuild, &iBuild);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pGameEmpireData);
    
    *pfMaintenanceRatio = GetMaintenanceRatio (iMin + iBonusMin, iMaint, iBuild);

Cleanup:

    SafeRelease (pGameEmpireData);

    return iErrCode;
}

int GameEngine::GetEmpireNextMaintenanceRatio (int iGameClass, int iGameNumber, int iEmpireKey, 
                                               float* pfNextMaintenanceRatio) {

    int iMin, iBonusMin, iNextMin, iMaint, iNextMaint;

    IReadTable* pGameEmpireData = NULL;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetTableForReading (strGameEmpireData, &pGameEmpireData);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pGameEmpireData->ReadData (GameEmpireData::TotalMin, &iMin);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::BonusMin, &iBonusMin);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameEmpireData->ReadData (GameEmpireData::NextMin, &iNextMin);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameEmpireData->ReadData (GameEmpireData::TotalMaintenance, &iMaint);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::NextMaintenance, &iNextMaint);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    int iCalcMin = iMin + iBonusMin + iNextMin;
    Assert (iCalcMin >= 0);

    int iCalcMaint = iMaint + iNextMaint;
    Assert (iCalcMaint >= 0);

    *pfNextMaintenanceRatio = GetMaintenanceRatio (iCalcMin, iCalcMaint, 0);

Cleanup:

    SafeRelease (pGameEmpireData);

    return iErrCode;
}


int GameEngine::RuinEmpire (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage) {

    int iErrCode;

    //
    // Best effort everything
    //
    
    iErrCode = SendSystemMessage (iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
    Assert (iErrCode == OK);

    iErrCode = UpdateScoresOnRuin (iGameClass, iGameNumber, iEmpireKey);
    Assert (iErrCode == OK);

    return iErrCode;
}

//
//
int GameEngine::WriteNextStatistics (int iGameClass, int iGameNumber, int iEmpireKey, int iTotalAg, 
                                     int iBonusAg, float fMaxAgRatio) {

    int iErrCode;

    unsigned int* piPlanetKey = NULL, iNumPlanets, i;

    int iNextPop, iNextMin, iNextFuel, iNewPop;
    float fAgRatio;

    Variant vTotalPop, vPop, vMaxPop, vMin, vFuel;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

#ifdef _DEBUG

    Variant vTargetPop;
    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TargetPop, &vTargetPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    int iTotalMaxPop = 0;

#endif

    // Calculate NextTotalPop
    iErrCode = m_pGameData->GetEqualKeys (
        strGameMap, 
        GameMap::Owner, 
        iEmpireKey, 
        false, 
        &piPlanetKey, 
        &iNumPlanets
        );
    
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalPop, &vTotalPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    fAgRatio = GetAgRatio (iTotalAg + iBonusAg, vTotalPop.GetInteger(), fMaxAgRatio);
    
    iNextPop = iNextMin = iNextFuel = 0;

    for (i = 0; i < iNumPlanets; i ++) {
        
        // Calculate new planet pop
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::MaxPop, &vMaxPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

#ifdef _DEBUG
        iTotalMaxPop += vMaxPop.GetInteger();
#endif
        
        // Calculate new pop
        iNewPop = GetNextPopulation (vPop.GetInteger(), fAgRatio);
        if (iNewPop > vMaxPop.GetInteger()) {
            iNewPop = vMaxPop.GetInteger();
        }
        iNextPop += iNewPop;
        
        // Calculate min increase
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Minerals, &vMin);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Fuel, &vFuel);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iNextMin += min (vMin.GetInteger(), iNewPop) - min (vMin.GetInteger(), vPop.GetInteger());
        iNextFuel += min (vFuel.GetInteger(), iNewPop) - min (vFuel.GetInteger(), vPop.GetInteger());
    }

    Assert (iTotalMaxPop == vTargetPop.GetInteger());

    // Write nextpop, nextmin, next fuel
    iErrCode = m_pGameData->WriteData (strEmpireData, GameEmpireData::NextTotalPop, iNextPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = m_pGameData->WriteData (strEmpireData, GameEmpireData::NextMin, iNextMin);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = m_pGameData->WriteData (strEmpireData, GameEmpireData::NextFuel, iNextFuel);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    if (piPlanetKey != NULL) {
        m_pGameData->FreeKeys (piPlanetKey);
    }

    return iErrCode;
}

#ifdef _DEBUG

void GameEngine::CheckTargetPop (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    unsigned int* piPlanetKey = NULL, iNumPlanets, i;

    int iNextPop, iNextMin, iNextFuel, iNewPop;
    float fAgRatio;

    Variant vTotalPop, vPop, vMaxPop, vMin, vFuel, vTemp;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    Variant vTargetPop;
    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TargetPop, &vTargetPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    int iTotalMaxPop = 0;

    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    float fMaxAgRatio = vTemp.GetFloat();

    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalAg, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    int iTotalAg = vTemp.GetInteger();

    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::BonusAg, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    int iBonusAg = vTemp.GetInteger();

    // Calculate NextTotalPop
    iErrCode = m_pGameData->GetEqualKeys (
        strGameMap, 
        GameMap::Owner, 
        iEmpireKey, 
        false, 
        &piPlanetKey, 
        &iNumPlanets
        );
    
    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::TotalPop, &vTotalPop);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    fAgRatio = GetAgRatio (iTotalAg + iBonusAg, vTotalPop.GetInteger(), fMaxAgRatio);
    
    iNextPop = iNextMin = iNextFuel = 0;

    for (i = 0; i < iNumPlanets; i ++) {
        
        // Calculate new planet pop
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::MaxPop, &vMaxPop);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iTotalMaxPop += vMaxPop.GetInteger();
        
        // Calculate new pop
        iNewPop = GetNextPopulation (vPop.GetInteger(), fAgRatio);
        if (iNewPop > vMaxPop.GetInteger()) {
            iNewPop = vMaxPop.GetInteger();
        }
        iNextPop += iNewPop;
        
        // Calculate min increase
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Minerals, &vMin);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Fuel, &vFuel);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iNextMin += min (vMin.GetInteger(), iNewPop) - min (vMin.GetInteger(), vPop.GetInteger());
        iNextFuel += min (vFuel.GetInteger(), iNewPop) - min (vFuel.GetInteger(), vPop.GetInteger());
    }

    Assert (iTotalMaxPop == vTargetPop.GetInteger());

Cleanup:

    if (piPlanetKey != NULL) {
        m_pGameData->FreeKeys (piPlanetKey);
    }
}

#endif

int GameEngine::UpdateGameEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, 
                                         const char* pszNotepad, bool* pbTruncated) {

    return UpdateGameEmpireString (
        iGameClass, 
        iGameNumber, 
        iEmpireKey, 
        GameEmpireData::Notepad, 
        pszNotepad, 
        MAX_NOTEPAD_LENGTH,
        pbTruncated
        );
}

int GameEngine::UpdateGameEmpireString (int iGameClass, int iGameNumber, int iEmpireKey, int iColumn, 
                                        const char* pszString, size_t stMaxLen, bool* pbTruncated) {

    int iErrCode;

    IWriteTable* pWriteTable = NULL;
    const char* pszOldString;

    GAME_EMPIRE_DATA (pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    *pbTruncated = false;

    iErrCode = m_pGameData->GetTableForWriting (pszGameEmpireData, &pWriteTable);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pWriteTable->ReadData (iColumn, &pszOldString);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (String::StrCmp (pszString, pszOldString) != 0) {

        char* pszNew = NULL;

        if (String::StrLen (pszString) >= stMaxLen) {

            pszNew = new char [stMaxLen + 1];
            if (pszNew == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }

            memcpy (pszNew, pszString, stMaxLen);
            pszNew [stMaxLen] = '\0';

            *pbTruncated = true;
            pszString = pszNew;
        }

        iErrCode = pWriteTable->WriteData (iColumn, pszString);

        if (pszNew != NULL) {
            delete [] pszNew;
        }

    } else {
        iErrCode = WARNING;
    }

Cleanup:

    SafeRelease (pWriteTable);

    return iErrCode;
}


int GameEngine::GetEmpireGameProperty (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iColumn, 
                                       Variant* pvProperty) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->ReadData (strGameEmpireData, iColumn, pvProperty);
}

int GameEngine::SetEmpireGameProperty (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iColumn,
                                       const Variant& vProperty) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->WriteData (strGameEmpireData, iColumn, vProperty);
}
