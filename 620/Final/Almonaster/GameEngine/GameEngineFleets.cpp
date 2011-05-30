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


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiFleetKeys -> Integer keys of fleets
// *piNumFleets -> Number of fleets
//
// Return the keys of the fleets belonging a given empire

int GameEngine::GetEmpireFleetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiFleetKeys, 
                                    int* piNumFleets) {

    GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetAllKeys (
        strGameEmpireFleets,
        (unsigned int**) ppiFleetKeys, 
        (unsigned int*) piNumFleets
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
// iFleetKey -> Integer key of fleet
//
// Output:
// *piPlanetKey -> Integer key of planet
//
// Return the location of a given fleet

int GameEngine::GetFleetLocation (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
                                  int* piPlanetKey) {

    int iErrCode;

    GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    Variant vPlanetKey;
    iErrCode = m_pGameData->ReadData (strGameEmpireFleets, iFleetKey, GameEmpireFleets::CurrentPlanet, &vPlanetKey);
    
    if (iErrCode == OK) {
        *piPlanetKey = vPlanetKey.GetInteger();
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iFleetKey -> Integer key of fleet
//
// Output:
// *pstrFleetName -> Name of fleet
//
// Return the name of a given fleet

int GameEngine::GetFleetName (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
                              Variant* pvFleetName) {

    GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->ReadData (strGameEmpireFleets, iFleetKey, GameEmpireFleets::Name, pvFleetName);
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiLocationKey -> Integer keys of planet locations
// *piNumLocations -> Number of locations
//
// Return the names and locations of the planets upon which new fleets can 
// be created (those that have the empire's ships on them and those that are builders and belong to the empire)

int GameEngine::GetNewFleetLocations (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiLocationKey, 
                                      int* piNumLocations) {

    bool bAdd;
    int iErrCode;
    Variant vTemp;

    *ppiLocationKey = NULL;
    *piNumLocations = 0;

    GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    // Get the keys of all visited planets
    IReadTable* pEmpireMap = NULL;

    unsigned int* piProxyKey = NULL, i, iNumPlanets;
    int* piPlanetKey = NULL;

    iErrCode = m_pGameData->GetTableForReading (strEmpireMap, &pEmpireMap);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = pEmpireMap->ReadColumn (
        GameEmpireMap::PlanetKey, 
        &piProxyKey, 
        &piPlanetKey, 
        &iNumPlanets
        );

    SafeRelease (pEmpireMap);

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    else if (iErrCode != OK) {
        goto Cleanup;
    }

    // Of these, get the ones that have the empire's ships on them or that are the empire's builders
    for (i = 0; i < iNumPlanets; i ++) {
    
        bAdd = false;

        // Has uncloaked ships there?
        iErrCode = m_pGameData->ReadData (strEmpireMap, piProxyKey[i], GameEmpireMap::NumUncloakedShips, &vTemp);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (vTemp.GetInteger() > 0) {
            bAdd = true;
        } else {
            
            // Has cloaked ships there?
            iErrCode = m_pGameData->ReadData (strEmpireMap, piProxyKey[i], GameEmpireMap::NumCloakedShips, &vTemp);
            if (iErrCode != OK) {
                goto Cleanup;
            }

            if (vTemp.GetInteger() > 0) {
                bAdd = true;
            } else {
        
                // Belongs to the empire and has builder pop?
                iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Owner, &vTemp);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

                if (vTemp.GetInteger() == iEmpireKey) {
                    
                    Variant vBuilderPopLevel;

                    iErrCode = m_pGameData->ReadData (
                        SYSTEM_GAMECLASS_DATA, 
                        iGameClass,
                        SystemGameClassData::BuilderPopLevel, 
                        &vBuilderPopLevel
                        );

                    if (iErrCode != OK) {
                        goto Cleanup;
                    }

                    iErrCode = m_pGameData->ReadData (strGameMap, piPlanetKey[i], GameMap::Pop, &vTemp);
                    if (vTemp.GetInteger() >= vBuilderPopLevel.GetInteger()) {
                        bAdd = true;
                    }
                }
            }
        }

        if (bAdd) {

            if (*ppiLocationKey == NULL) {
                *ppiLocationKey = new int [iNumPlanets];
                if (*ppiLocationKey == NULL) {
                    iErrCode = ERROR_OUT_OF_MEMORY;
                    goto Cleanup;
                }
            }

            // Get planet key
            (*ppiLocationKey) [*piNumLocations] = piPlanetKey[i];

            // Increment counter
            (*piNumLocations) ++;
        }
    }

Cleanup:

    if (piPlanetKey != NULL) {
        m_pGameData->FreeData (piPlanetKey);
    }

    if (piProxyKey != NULL) {
        m_pGameData->FreeKeys (piProxyKey);
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// pszFleetName -> Name of new fleet
// iPlanetKey -> Integer key of fleet location
//
// Output:
//  ERROR_NAME_IS_IN_USE -> Fleet name is already being used
//  ERROR_EMPTY_NAME -> Name has no content
//  ERROR_ORPHANED_FLEET -> No ships on planet and planet does not belong to empire
//
// Creates a new fleet at the given location

int GameEngine::CreateNewFleet (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszFleetName, 
                                int iPlanetKey) {

    bool bAccept = true;
    Variant vOwner;

    // Make sure that the name isn't null
    if (String::IsBlank (pszFleetName)) {
        return ERROR_EMPTY_NAME;
    }

    GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    // Lock
    NamedMutex nmFleetLock;
    LockEmpireFleets (iGameClass, iGameNumber, iEmpireKey, &nmFleetLock);

    // Make sure the name isn't in use
    unsigned int iKey;
    int iErrCode = m_pGameData->GetFirstKey (strEmpireFleets, GameEmpireFleets::Name, pszFleetName, false, &iKey);

    if (iErrCode != ERROR_DATA_NOT_FOUND) {

        if (iErrCode == OK) {
            iErrCode = ERROR_NAME_IS_IN_USE;
        }

        goto Cleanup;
    }

    // Make sure the fleet can be built at the planet
    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (vOwner.GetInteger() != iEmpireKey) {
        
        // Make sure we have ships there at least
        GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

        Variant vNumUncloakedShips, vNumCloakedShips;

        iErrCode = m_pGameData->GetFirstKey (strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, false, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            bAccept = false;
        } else {

            if (iErrCode != OK) {
                goto Cleanup;
            }

            iErrCode = m_pGameData->ReadData (
                strEmpireMap, 
                iKey, 
                GameEmpireMap::NumUncloakedShips, 
                &vNumUncloakedShips
                );
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            if (vNumUncloakedShips.GetInteger() == 0) {

                iErrCode = m_pGameData->ReadData (
                    strEmpireMap, 
                    iKey, 
                    GameEmpireMap::NumCloakedShips, 
                    &vNumCloakedShips
                    );
                if (iErrCode != OK) {
                    goto Cleanup;
                }
                
                if (vNumCloakedShips.GetInteger() == 0) {
                    bAccept = false;
                }
            }
        }
    }

    if (!bAccept) {
        return ERROR_ORPHANED_FLEET;
    }

    // Insert new row
    {
        Variant pvColVal [GameEmpireFleets::NumColumns] = {
            pszFleetName,
            0,
            (float) 0.0,
            (float) 0.0,
            iPlanetKey,
            STAND_BY,
            0
        };

        if (pvColVal[GameEmpireFleets::Name].GetCharPtr() == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }
        
        // Insert row into fleets table
        iErrCode = m_pGameData->InsertRow (strEmpireFleets, pvColVal);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

Cleanup:

    UnlockEmpireFleets (nmFleetLock);

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iFleetKey -> Integer key of fleet
//
// Output:
// **ppiOrderKey -> Fleet order keys
// **ppstrOrderText -> Text of orders
// *piSelected -> Order selected
// *piNumOrders -> Number of orders
//
// Return the orders for a fleet

int GameEngine::GetFleetOrders (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
                                int** ppiOrderKey, String** ppstrOrderText, int* piSelected, 
                                int* piNumOrders) {

    int iErrCode, iX, iY, iNumShips;
    Variant vTemp, vPlanetKey, vCoordinates, vPlanetName, vBuildShips;

    char pszOrder [128 + MAX_PLANET_NAME_LENGTH];

    const size_t stMaxNumOrders = NUM_CARDINAL_POINTS + NUM_SHIP_TYPES;

    *ppiOrderKey = NULL;
    *ppstrOrderText = NULL;
    *piSelected = NO_KEY;
    *piNumOrders = 0;

    GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    // Get currently selected order
    iErrCode = m_pGameData->ReadData (strEmpireFleets, iFleetKey, GameEmpireFleets::Action, &vTemp);
    if (iErrCode != OK) {
        return iErrCode;
    }

    *piSelected = vTemp.GetInteger();

    // Get fleet location
    iErrCode = m_pGameData->ReadData (strEmpireFleets, iFleetKey, GameEmpireFleets::CurrentPlanet, &vPlanetKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Get planet coordinates and name
    iErrCode = m_pGameData->ReadData (strGameMap, vPlanetKey, GameMap::Coordinates, &vCoordinates);
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    GetCoordinates (vCoordinates.GetCharPtr(), &iX, &iY);
    
    iErrCode = m_pGameData->ReadData (strGameMap, vPlanetKey, GameMap::Name, &vPlanetName);
    if (iErrCode != OK) {
        return iErrCode;
    }

    ////////////////////////////////
    // Allocate memory for orders //
    ////////////////////////////////
    
    *ppiOrderKey = new int [stMaxNumOrders];
    if (*ppiOrderKey == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    *ppstrOrderText = new String [stMaxNumOrders];
    if (*ppstrOrderText == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }
    
    ///////////////////
    // Add "Standby" //
    ///////////////////
    
    sprintf (pszOrder, "Standby at %s (%i,%i)", vPlanetName.GetCharPtr(), iX, iY);

    (*ppiOrderKey)[0] = STAND_BY;
    (*ppstrOrderText)[0] = pszOrder;
    *piNumOrders = 1;

    ///////////////////////////////////////////
    // Add moves if fleet has no build ships //
    ///////////////////////////////////////////

    iErrCode = m_pGameData->ReadData (strEmpireFleets, iFleetKey, GameEmpireFleets::BuildShips, &vBuildShips);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = GetNumShipsInFleet (iGameClass, iGameNumber, iEmpireKey, iFleetKey, &iNumShips);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (vBuildShips.GetInteger() == 0) {
        
        GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
        GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

        Variant vOwner;

        // Get planet proxy key from empire map
        unsigned int iPlanetProxyKey;
        iErrCode = m_pGameData->GetFirstKey (
            strEmpireMap, 
            GameEmpireMap::PlanetKey, 
            vPlanetKey, 
            false, 
            &iPlanetProxyKey
            );

        if (iErrCode != OK) {

            Assert (!"Fleet located on unexplored planet");
            iErrCode = ERROR_DATA_CORRUPTION;
            goto Cleanup;
        }

        ///////////////
        // Add moves //
        ///////////////

        int i, iNewX, iNewY, iExplored, iLink;
        Variant vExplored, vNewSystemName, vNeighbourPlanetKey;
        
        iErrCode = m_pGameData->ReadData (
            strEmpireMap, 
            iPlanetProxyKey, 
            GameEmpireMap::Explored, 
            &vExplored
            );
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iExplored = vExplored.GetInteger();

        iErrCode = m_pGameData->ReadData (strGameMap, vPlanetKey, GameMap::Link, &vExplored);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iLink = vExplored.GetInteger();

        ENUMERATE_CARDINAL_POINTS(i) {
        
            if ((iLink & LINK_X[i]) && (iExplored & EXPLORED_X[i])) {
                
                // Get neighbouring planet's key
                iErrCode = m_pGameData->ReadData (
                    strGameMap, 
                    vPlanetKey.GetInteger(), 
                    GameMap::NorthPlanetKey + i,
                    &vNeighbourPlanetKey
                    );

                if (iErrCode != OK) {
                    goto Cleanup;
                }
                
                // Get neighbouring planet's name
                iErrCode = m_pGameData->ReadData (
                    strGameMap, 
                    vNeighbourPlanetKey, 
                    GameMap::Name, 
                    &vNewSystemName
                    );
                if (iErrCode != OK) {
                    goto Cleanup;
                }
                
                AdvanceCoordinates (iX, iY, &iNewX, &iNewY, i);

                sprintf (
                    pszOrder, 
                    "Move %s to %s (%i,%i)", 
                    CARDINAL_STRING[i], 
                    vNewSystemName.GetCharPtr(), 
                    iNewX, 
                    iNewY
                    );

                (*ppiOrderKey)[*piNumOrders] = MOVE_NORTH - i;
                (*ppstrOrderText)[*piNumOrders] = pszOrder;
                (*piNumOrders) ++;
            }
        }

        //////////
        // Nuke //
        //////////
        
        iErrCode = m_pGameData->ReadData (strGameMap, vPlanetKey, GameMap::Owner, &vOwner);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        if (vOwner.GetInteger() != SYSTEM && 
            vOwner.GetInteger() != INDEPENDENT && 
            vOwner.GetInteger() != iEmpireKey) {
            
            unsigned int iKey;

            iErrCode = m_pGameData->GetFirstKey (
                strEmpireDip, 
                GameEmpireDiplomacy::EmpireKey, 
                vOwner, 
                false, 
                &iKey
                );

            if (iErrCode == OK) {

                Variant vDipStatus;
                iErrCode = m_pGameData->ReadData (
                    strEmpireDip, 
                    iKey, 
                    GameEmpireDiplomacy::CurrentStatus, 
                    &vDipStatus
                    );
                if (iErrCode != OK) {
                    goto Cleanup;
                }
                
                if (vDipStatus.GetInteger() == WAR) {
                    
                    sprintf (pszOrder, "Nuke %s (%i,%i)", vPlanetName.GetCharPtr(), iX, iY);
                    
                    (*ppstrOrderText)[*piNumOrders] = pszOrder;
                    (*ppiOrderKey)[*piNumOrders] = NUKE;
                    (*piNumOrders) ++;
                }
            }

            else if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            }
        }
    }

    // Add disband
    (*ppiOrderKey)[*piNumOrders] = DISBAND;
    (*ppstrOrderText)[*piNumOrders] = "Disband fleet";
    (*piNumOrders) ++;

    // Add disband and dismantle if there are ships in the fleet
    if (iNumShips > 0) {
        (*ppiOrderKey)[*piNumOrders] = DISBAND_AND_DISMANTLE;
        (*ppstrOrderText)[*piNumOrders] = "Disband fleet and dismantle ships";
        (*piNumOrders) ++;
    }

Cleanup:

    if (iErrCode != OK) {

        if (*ppiOrderKey != NULL) {
            delete [] *ppiOrderKey;
            ppiOrderKey = NULL;
        }

        if (*ppstrOrderText != NULL) {
            delete [] *ppstrOrderText;
            ppstrOrderText = NULL;
        }

        *piNumOrders = 0;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iFleetKey -> Integer key associated with particular fleet
// pszNewName -> New name for fleet
//
// Updates the name of a fleet

int GameEngine::UpdateFleetName (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
                                 const char* pszNewName) {

    GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    // Make sure fleet exists
    bool bFleetExists;
    if (m_pGameData->DoesRowExist (strEmpireFleets, iFleetKey, &bFleetExists) != OK || !bFleetExists) {
        return ERROR_FLEET_DOES_NOT_EXIST;
    }

    // Write new name
    return m_pGameData->WriteData (strEmpireFleets, iFleetKey, GameEmpireFleets::Name, pszNewName);
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iFleetKey -> Integer key associated with particular fleet
// iOrderKey -> Integer key associated with particular order
//
// returns
//  OK -> Action was updated
//  ERROR_SAME_FLEET_ORDER -> Same order as before
//  ERROR_FLEET_DOES_NOT_EXIST -> Fleet doesn't exist
//  ERROR_CANNOT_NUKE -> Fleet cannot nuke
//  ERROR_CANNOT_MOVE -> Fleet cannot move
//  ERROR_UNKNOWN_ORDER -> Cannot interpret order
//
// Updates the fleet's orders

int GameEngine::UpdateFleetOrders (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
                                   int iOrderKey) {
    
    GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    NamedMutex nmFleetMutex;
    LockEmpireFleets (iGameClass, iGameNumber, iEmpireKey, &nmFleetMutex);

    bool bFleetExists;

    int iErrCode;
    unsigned int i, iPlanetProxyKey;

    Variant vOldAction, vFleetPlanet, vBuildShips, vNumShips, vLink, vExplored;

    // Does the fleet exist?
    iErrCode = m_pGameData->DoesRowExist (strEmpireFleets, iFleetKey, &bFleetExists);
    if (iErrCode != OK || !bFleetExists) {
        iErrCode = ERROR_FLEET_DOES_NOT_EXIST;
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strEmpireFleets, iFleetKey, GameEmpireFleets::Action, &vOldAction);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iOrderKey == vOldAction.GetInteger()) {
        iErrCode = ERROR_SAME_FLEET_ORDER;
        goto Cleanup;
    }

    // Check for standby
    if (iOrderKey == STAND_BY) {
        iErrCode = m_pGameData->WriteData (strEmpireFleets, iFleetKey, GameEmpireFleets::Action, STAND_BY);
        goto Cleanup;
    }

    // Check for fleet disband
    if (iOrderKey == DISBAND || iOrderKey == DISBAND_AND_DISMANTLE) {

        // Mark all member ships fleetkeys as NO_KEY
        GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);

        unsigned int* piShipKey, iNumShips;
        iErrCode = m_pGameData->GetEqualKeys (
            strEmpireShips, 
            GameEmpireShips::FleetKey, 
            iFleetKey, 
            false, 
            &piShipKey, 
            &iNumShips
            );

        if (iErrCode != OK) {

            if (iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                goto Cleanup;
            }

        } else {

            Variant vBuiltThisUpdate, vMaxBR, vType, vAction;
            GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

            NamedMutex nmShipMutex;
            LockEmpireShips (iGameClass, iGameNumber, iEmpireKey, &nmShipMutex);

            // Best effort loop through all ships
            for (i = 0; i < iNumShips; i ++) {

                // Set no fleetkey
                iErrCode = m_pGameData->WriteData (
                    strEmpireShips, 
                    piShipKey[i], 
                    GameEmpireShips::FleetKey, 
                    NO_KEY
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    continue;
                }

                // Being build this update?
                iErrCode = m_pGameData->ReadData (
                    strEmpireShips, 
                    piShipKey[i], 
                    GameEmpireShips::BuiltThisUpdate, 
                    &vBuiltThisUpdate
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    continue;
                }

                if (iOrderKey == DISBAND_AND_DISMANTLE) {

                    if (vBuiltThisUpdate.GetInteger() != 0) {

                        // Dismantle now
                        iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, piShipKey[i], true);
                        if (iErrCode != OK) {
                            Assert (false);
                            continue;
                        }

                    } else {

                        iErrCode = m_pGameData->ReadData (
                            strEmpireShips, 
                            piShipKey[i], 
                            GameEmpireShips::Action, 
                            &vAction
                            );

                        if (iErrCode != OK) {
                            Assert (false);
                            continue;
                        }

                        if (vAction.GetInteger() == FLEET) {

                            iErrCode = UpdateShipOrders (
                                iGameClass, 
                                iGameNumber, 
                                iEmpireKey, 
                                piShipKey[i], 
                                DISMANTLE
                                );

                            if (iErrCode != OK) {

                                // If that didn't work out, try standby
                                iErrCode = UpdateShipOrders (
                                    iGameClass, 
                                    iGameNumber, 
                                    iEmpireKey, 
                                    piShipKey[i], 
                                    STAND_BY
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    continue;
                                }
                            }
                        }
                    }

                } else {
                    
                    if (vBuiltThisUpdate.GetInteger() != 0) {
                        
                        // Build at
                        iErrCode = m_pGameData->WriteData (
                            strEmpireShips, 
                            piShipKey[i], 
                            GameEmpireShips::Action, 
                            BUILD_AT
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            continue;
                        }
                        
                    } else {
                        
                        // Set ships to the old fleet action
                        // if they were just sitting in the fleet
                        iErrCode = m_pGameData->ReadData (
                            strEmpireShips, 
                            piShipKey[i], 
                            GameEmpireShips::Action, 
                            &vAction
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            continue;
                        }

                        if (vAction.GetInteger() == FLEET) {

                            iErrCode = UpdateShipOrders (
                                iGameClass, 
                                iGameNumber, 
                                iEmpireKey, 
                                piShipKey[i], 
                                vOldAction.GetInteger()
                                );
                            
                            if (iErrCode != OK) {

                                // If that didn't work out, try standby
                                iErrCode = UpdateShipOrders (
                                    iGameClass, 
                                    iGameNumber, 
                                    iEmpireKey, 
                                    piShipKey[i], 
                                    STAND_BY
                                    );
                                if (iErrCode != OK) {
                                    Assert (false);
                                    continue;
                                }
                            }
                        }
                    }
                }
            }

            UnlockEmpireShips (nmShipMutex);

            m_pGameData->FreeKeys (piShipKey);
        }

        // Destroy the fleet and we're done
        iErrCode = m_pGameData->DeleteRow (strEmpireFleets, iFleetKey);
        goto Cleanup;
    }

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    
    iErrCode = m_pGameData->ReadData (strEmpireFleets, iFleetKey, GameEmpireFleets::CurrentPlanet, &vFleetPlanet);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Check for nukes
    if (iOrderKey == NUKE) {
        
        // Get owner
        Variant vOwner;
        iErrCode = m_pGameData->ReadData (strGameMap, vFleetPlanet, GameMap::Owner, &vOwner);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        if (vOwner.GetInteger() == SYSTEM || vOwner.GetInteger() == iEmpireKey) {
            iErrCode = ERROR_CANNOT_NUKE;
            goto Cleanup;
        }
        
        // Check diplomatic status with owner
        GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
        
        // Make sure the diplomatic status is war           
        unsigned int iKey;
        iErrCode = m_pGameData->GetFirstKey (strDiplomacy, GameEmpireDiplomacy::EmpireKey, vOwner, false, &iKey);
        if (iErrCode != OK) {
            iErrCode = ERROR_CANNOT_NUKE;
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->ReadData (strDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vOwner);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        if (vOwner.GetInteger() != WAR) {
            iErrCode = ERROR_CANNOT_NUKE;
            goto Cleanup;
        }
        
        iErrCode = m_pGameData->WriteData (strEmpireFleets, iFleetKey, GameEmpireFleets::Action, NUKE);
        goto Cleanup;
    }
    
    // Get proxy key for empire map
    iErrCode = m_pGameData->GetFirstKey (
        strEmpireMap, 
        GameEmpireMap::PlanetKey, 
        vFleetPlanet, 
        false, 
        &iPlanetProxyKey
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ENUMERATE_CARDINAL_POINTS(i) {

        if (iOrderKey == MOVE_NORTH - (int) i) {
            
            // Make sure we have no build ships
            iErrCode = m_pGameData->ReadData (strEmpireFleets, iFleetKey, GameEmpireFleets::BuildShips, &vBuildShips);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (vBuildShips.GetInteger() > 0) {
                iErrCode = ERROR_CANNOT_MOVE;
                goto Cleanup;
            }
            
            // Now, make sure we have a link to chase
            iErrCode = m_pGameData->ReadData (strGameMap, vFleetPlanet, GameMap::Link, &vLink);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;;
            }

            if (!(vLink.GetInteger() & LINK_X[i])) {
                iErrCode = ERROR_CANNOT_MOVE;
                goto Cleanup;
            }
            
            // Make sure we've explored the link
            iErrCode = m_pGameData->ReadData (
                strEmpireMap, 
                iPlanetProxyKey, 
                GameEmpireMap::Explored,
                &vExplored
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (vExplored.GetInteger() & EXPLORED_X[i]) {

#ifdef _DEBUG
                // Make sure there's a planet
                Variant vNewPlanetKey;
                iErrCode = m_pGameData->ReadData (
                    strGameMap, 
                    vFleetPlanet, 
                    GameMap::NorthPlanetKey + i, 
                    &vNewPlanetKey
                    );
                Assert (iErrCode == OK && vNewPlanetKey != NO_KEY);
                
                iErrCode = m_pGameData->GetFirstKey (
                    strEmpireMap, 
                    GameEmpireMap::PlanetKey, 
                    vNewPlanetKey, 
                    false, 
                    &iPlanetProxyKey
                    );
                
                Assert (iErrCode == OK && iPlanetProxyKey != NO_KEY);
#endif
                
                // Accept the order
                iErrCode = m_pGameData->WriteData (
                    strEmpireFleets, 
                    iFleetKey, 
                    GameEmpireFleets::Action, 
                    iOrderKey
                    );

                goto Cleanup;
            }

            iErrCode = ERROR_CANNOT_MOVE;
            goto Cleanup;
        }
    }

Cleanup:

    UnlockEmpireFleets (nmFleetMutex);

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iFleetKey -> Integer key associated with particular fleet
//
// Output:
// *piNumShips -> Number of ships in fleet
//
// Return the number of ships in a fleet

int GameEngine::GetNumShipsInFleet (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
                                    int* piNumShips) {

    GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetEqualKeys (
        pszShips,
        GameEmpireShips::FleetKey,
        iFleetKey,
        false,
        NULL,
        (unsigned int*) piNumShips
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return OK;
    }

    return iErrCode;
}