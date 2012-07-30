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
#include "Global.h"

// Delete the player's tables from the given game:
//
// GameEmpireData(I.I.I)
// GameEmpireMessages(I.I.I)
// GameEmpireMap(I.I.I)
// GameEmpireShips(I.II)
// GameEmpireFleets(I.I.I)
// GameEmpireDiplomacy(I.I.I)
//
int GameEngine::DeleteEmpireFromGame(int iGameClass, int iGameNumber, int iEmpireKey, ReasonForRemoval rReason, const GameUpdateInformation* pguInfo)
{
    GET_GAME_EMPIRE_DATA(strEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_DATA(strGameData, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_SHIPS(strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRES(strEmpires, iGameClass, iGameNumber);
    GET_SYSTEM_EMPIRE_DATA(strSystemEmpireData, iEmpireKey);

    ICachedTable* pEmpireActiveGames = NULL;
    AutoRelease<ICachedTable> release(pEmpireActiveGames);
    Variant vOptions, vGameState, vTemp;

    Variant* pvPlanetKey = NULL, * pvData = NULL, * pvEmpireKey = NULL;
    AutoFreeData freeData1(pvPlanetKey);
    AutoFreeData freeData2(pvData);
    AutoFreeData freeData3(pvEmpireKey);

    unsigned int* piShipKey = NULL, * piPlanetKey = NULL;
    AutoFreeKeys freeKeys1(piShipKey);
    AutoFreeKeys freeKeys2(piPlanetKey);

    // Independence?
    Variant vGameClassOptions;
    int iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vGameClassOptions);
    RETURN_ON_ERROR(iErrCode);

    bool bIndependence = (vGameClassOptions.GetInteger() & INDEPENDENCE) != 0;

    // Get game state
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vGameState);
    RETURN_ON_ERROR(iErrCode);

    // Get empire's game options
    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    // Handle num updated
    if (vOptions.GetInteger() & UPDATED)
    {
        iErrCode = t_pCache->Increment(strGameData, GameData::NumEmpiresUpdated, -1, &vTemp);
        RETURN_ON_ERROR(iErrCode);
    }

    // Handle num paused
    if (vOptions.GetInteger() & REQUEST_PAUSE)
    {
        // Decrement number of paused empires
        iErrCode = t_pCache->Increment(strGameData, GameData::NumRequestingPause, -1);
        RETURN_ON_ERROR(iErrCode);
    }

    // Handle num drawing
    if (vOptions.GetInteger() & REQUEST_DRAW)
    {
        iErrCode = t_pCache->Increment(strGameData, GameData::NumRequestingDraw, -1);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // Handle num resigned
    if (vOptions.GetInteger() & RESIGNED)
    {
        iErrCode = t_pCache->Increment(strGameData, GameData::NumEmpiresResigned, -1);
        RETURN_ON_ERROR(iErrCode);
    }

    // Delete empire from empirelist
    unsigned int i, j, iNumEmpires, iGameEmpireKey;
    iErrCode = t_pCache->GetFirstKey(strEmpires, GameEmpires::EmpireKey, iEmpireKey, &iGameEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    // If game started, add empire to dead empire list
    if (rReason != EMPIRE_GAME_ENDED && 
        rReason != EMPIRE_QUIT && 
        rReason != EMPIRE_GAME_ENTRY_ERROR &&
        (vGameState.GetInteger() & STARTED))
    {
        Variant pvGameEmpireData[GameNukedEmpires::NumColumns];

        pvGameEmpireData[GameNukedEmpires::iGameClass] = iGameClass;
        pvGameEmpireData[GameNukedEmpires::iGameNumber] = iGameNumber;

        iErrCode = GetEmpireName(iEmpireKey, pvGameEmpireData + GameNukedEmpires::iName);
        RETURN_ON_ERROR(iErrCode);
            
        Variant vAlienKey, vAlienAddress;
        iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::AlienKey, &vAlienKey);
        RETURN_ON_ERROR(iErrCode);
        iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::AlienAddress, &vAlienAddress);
        RETURN_ON_ERROR(iErrCode);

        pvGameEmpireData[GameNukedEmpires::iNukedEmpireKey] = iEmpireKey;
        pvGameEmpireData[GameNukedEmpires::iAlienKey] = vAlienKey;
        pvGameEmpireData[GameNukedEmpires::iAlienAddress] = vAlienAddress;
        pvGameEmpireData[GameNukedEmpires::iUpdate] = pguInfo == NULL ? 0 : pguInfo->iUpdate;
        pvGameEmpireData[GameNukedEmpires::iReason] = rReason;

        iErrCode = GetEmpireProperty(iEmpireKey, SystemEmpireData::SecretKey, pvGameEmpireData + GameNukedEmpires::iSecretKey);
        RETURN_ON_ERROR(iErrCode);

        GET_GAME_NUKED_EMPIRES(strGameNukedData, iGameClass, iGameNumber);
        iErrCode = t_pCache->InsertRow(strGameNukedData, GameNukedEmpires::Template, pvGameEmpireData, NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    // Read active empire keys
    iErrCode = t_pCache->ReadColumn(strEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    // Remove from other empires' dip tables
    for (i = 0; i < iNumEmpires; i ++)
    {
        if (iEmpireKey == pvEmpireKey[i].GetInteger())
            continue;

        GET_GAME_EMPIRE_DIPLOMACY(strOtherEmpireDip, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
            
        unsigned int iDiplomacyKey;
        iErrCode = t_pCache->GetFirstKey(strOtherEmpireDip, GameEmpireDiplomacy::ReferenceEmpireKey, iEmpireKey, &iDiplomacyKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
            continue;
        }

        GET_GAME_EMPIRE_DATA(strGameEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
                
        Variant vDipStatus;
        iErrCode = t_pCache->ReadData(strOtherEmpireDip, iDiplomacyKey, GameEmpireDiplomacy::CurrentStatus, &vDipStatus);
        RETURN_ON_ERROR(iErrCode);

        // Add to the nuked ally count
        if (vDipStatus.GetInteger() == ALLIANCE)
        {
            iErrCode = t_pCache->Increment(strGameEmpireData, GameEmpireData::NumNukedAllies, 1);
            RETURN_ON_ERROR(iErrCode);
        }

        // Nuke the row
        iErrCode = t_pCache->DeleteRow(strOtherEmpireDip, iDiplomacyKey);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // Delete row from GameEmpires
    iErrCode = t_pCache->DeleteRow(strEmpires, iGameEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    // Remove empire's ownership of planets from map
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    unsigned int iNumKeys;
    iErrCode = t_pCache->GetEqualKeys(strGameMap, GameMap::Owner, iEmpireKey, &piPlanetKey, &iNumKeys);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        //
        // Set planets to the right ownership
        //
        Variant vMaxAgRatio;
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vMaxAgRatio);
        RETURN_ON_ERROR(iErrCode);

        for (i = 0; i < iNumKeys; i ++)
        {
            // Resolve ownership and population
            Variant vAg;
            iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Ag, &vAg);
            RETURN_ON_ERROR(iErrCode);

            Variant vMin, vFuel, vPop; 
            if (bIndependence && vAg.GetInteger() > 0)
            {
                // Make planet independent
                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::Owner, (int)INDEPENDENT);
                RETURN_ON_ERROR(iErrCode);

                // Set max pop to ag
                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::MaxPop, vAg);
                RETURN_ON_ERROR(iErrCode);

                // Calculate next population
                iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
                RETURN_ON_ERROR(iErrCode);

                // Normalize next pop to max pop
                vPop = GetNextPopulation(vPop.GetInteger(), GetAgRatio (vAg.GetInteger(), vPop.GetInteger(), vMaxAgRatio.GetFloat()));
                if (vPop.GetInteger() > vAg.GetInteger())
                {
                    vPop = vAg;
                }

                // Set max pop
                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::Pop, vPop);
                RETURN_ON_ERROR(iErrCode);
            }
            else
            {
                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::Owner, (int)SYSTEM);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::Pop, (int)0);
                RETURN_ON_ERROR(iErrCode);

                // Set max pop to the right thing
                iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Minerals, &vMin);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Fuel, &vFuel);
                RETURN_ON_ERROR(iErrCode);

                int iMaxPop = max(vMin, vFuel);
                if (iMaxPop == 0)
                {
                    iMaxPop = 1;
                }

                iErrCode = t_pCache->WriteData(strGameMap, piPlanetKey[i], GameMap::MaxPop, iMaxPop);
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    // Set the empire's HW to non-HW status if the map already was generated
    unsigned int iNumRows;
    iErrCode = t_pCache->GetNumCachedRows(strGameMap, &iNumRows);
    RETURN_ON_ERROR(iErrCode);

    if (iNumRows > 0)
    {
        Variant vHWKey, vStillHW;
        iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::HomeWorld, &vHWKey);
        RETURN_ON_ERROR(iErrCode);

        if (vHWKey.GetInteger() != NO_KEY)
        {
            unsigned int iHWKey = vHWKey.GetInteger();

            // Is the homeworld still a homeworld?
            iErrCode = t_pCache->ReadData(strGameMap, iHWKey, GameMap::HomeWorld, &vStillHW);
            RETURN_ON_ERROR(iErrCode);

            if (vStillHW.GetInteger() == HOMEWORLD)
            {
                Variant vAg, vMin, vFuel;

                // Mark as not homeworld
                iErrCode = t_pCache->WriteData(strGameMap, iHWKey, GameMap::HomeWorld, NOT_HOMEWORLD);
                RETURN_ON_ERROR(iErrCode);

                // Halve resources
                iErrCode = t_pCache->ReadData(strGameMap, iHWKey, GameMap::Ag, &vAg);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(strGameMap, iHWKey, GameMap::Minerals, &vMin);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->ReadData(strGameMap, iHWKey, GameMap::Fuel, &vFuel);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->WriteData(strGameMap, iHWKey, GameMap::Ag, (int) (vAg.GetInteger() / 2));
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->WriteData(strGameMap, iHWKey, GameMap::MaxPop, (int) (vAg.GetInteger() / 2));
                RETURN_ON_ERROR(iErrCode);

                iErrCode = t_pCache->WriteData(strGameMap, iHWKey, GameMap::Fuel, (int) (vFuel.GetInteger() / 2));
                RETURN_ON_ERROR(iErrCode);
                
                iErrCode = t_pCache->WriteData(strGameMap, iHWKey, GameMap::Minerals, (int) (vMin.GetInteger() / 2));
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    // Handle ships
    unsigned int iNumShips;
    iErrCode = t_pCache->ReadColumn(strEmpireShips, GameEmpireShips::CurrentPlanet, &piShipKey, &pvPlanetKey, &iNumShips);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        Assert(iNumShips > 0);
        if (!bIndependence)
        {
            // Destroy ships: subtract them from planetary ship counts
            for (i = 0; i < iNumShips; i ++)
            {
                iErrCode = DeleteShipFromDeadEmpire (strEmpireShips, strGameMap, piShipKey[i], pvPlanetKey[i]);
                RETURN_ON_ERROR(iErrCode);
            }
        }
        else
        {
            Variant vOwner, vType, vBR, vMaxPop, vPop, vShipBehavior, vColonyMultipliedDepositFactor, vColonyExponentialDepositFactor, vTemp;

            unsigned int iInitPop, iOwnerKey;
            float fDecrease;
            bool bBuilding;

            unsigned int* piNumShipsAcquired = NULL;

            if (pguInfo != NULL)
            {
                piNumShipsAcquired = (unsigned int*) StackAlloc (pguInfo->iNumEmpires * sizeof (unsigned int));
                memset (piNumShipsAcquired, 0, pguInfo->iNumEmpires * sizeof (unsigned int));
            }

            // Get ship behavior
            iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::ShipBehavior, &vShipBehavior);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::ColonyMultipliedDepositFactor, &vColonyMultipliedDepositFactor);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::ColonyExponentialDepositFactor, &vColonyExponentialDepositFactor);
            RETURN_ON_ERROR(iErrCode);

            for (i = 0; i < iNumShips; i ++)
            {
                // Default to death of ship
                iOwnerKey = NO_KEY;

                // Get owner of planet
                iErrCode = t_pCache->ReadData(strGameMap, pvPlanetKey[i], GameMap::Owner, &vOwner);
                RETURN_ON_ERROR(iErrCode);

                switch (vOwner.GetInteger()) {
                    
                case INDEPENDENT:
                    // The ship becomes independent
                    iOwnerKey = INDEPENDENT;
                    break;

                case SYSTEM:
                    // The ship is on a system planet: it dies unless its a colony, in which case it colonizes the planet
                    iErrCode = t_pCache->ReadData(strEmpireShips, piShipKey[i], GameEmpireShips::Type, &vType);
                    RETURN_ON_ERROR(iErrCode);

                    if (vType.GetInteger() == COLONY) {

                        /////////////////////
                        // Colonize planet //
                        /////////////////////

                        // Read planet's ag
                        Variant vAg;
                        iErrCode = t_pCache->ReadData(strGameMap, pvPlanetKey[i], GameMap::Ag, &vAg);
                        RETURN_ON_ERROR(iErrCode);

                        // Only colonize if ag > 0
                        if (vAg.GetInteger() > 0)
                        {
                            // Set independent planet, MaxPop = Ag
                            if (vOwner.GetInteger() != INDEPENDENT)
                            {
                                Assert(vOwner.GetInteger() == SYSTEM);
                                    
                                iErrCode = t_pCache->WriteData(strGameMap, pvPlanetKey[i], GameMap::Owner, (int)INDEPENDENT);
                                RETURN_ON_ERROR(iErrCode);

                                iErrCode = t_pCache->WriteData(strGameMap, pvPlanetKey[i], GameMap::MaxPop, vAg);
                                RETURN_ON_ERROR(iErrCode);
                            }
                                
                            // Get colony's capacity
                            iErrCode = t_pCache->ReadData(strEmpireShips, piShipKey[i], GameEmpireShips::CurrentBR, &vBR);
                            RETURN_ON_ERROR(iErrCode);

                            iInitPop = GetColonizePopulation (
                                vShipBehavior.GetInteger(), 
                                vColonyMultipliedDepositFactor.GetFloat(), 
                                vColonyExponentialDepositFactor.GetFloat(),
                                vBR.GetFloat()
                                );
                                    
                            // Does ship exhaust all pop?
                            if ((int) iInitPop <= vAg.GetInteger())
                            {
                                // Ship will die
                                iOwnerKey = NO_KEY;
                            }
                            else
                            {
                                // Calculate decrease
                                int iInc = iInitPop - vAg.GetInteger();
                                fDecrease = vBR.GetFloat() * (float) iInc / iInitPop;
                                iInitPop = iInc;
                                        
                                // Reduce ship BR
                                iErrCode = t_pCache->Increment(strEmpireShips, piShipKey[i], GameEmpireShips::CurrentBR, - fDecrease);
                                RETURN_ON_ERROR(iErrCode);
                                        
                                // Ship survives
                                iOwnerKey = INDEPENDENT;
                            }
                                    
                            // Deposit population
                            iErrCode = t_pCache->Increment(strGameMap, pvPlanetKey[i], GameMap::Pop, iInitPop);
                            RETURN_ON_ERROR(iErrCode);

                            // Add to update message for observer empires
                            if (pguInfo != NULL && pguInfo->pstrUpdateMessage != NULL)
                            {
                                char pszMessage[MAX_PLANET_NAME_WITH_COORDINATES_LENGTH + 96] = "";
                                for (j = 0; j < pguInfo->iNumEmpires; j ++)
                                {
                                    if (!pguInfo->pbAlive[j])
                                        continue;

                                    unsigned int iMapKey;
                                    GET_GAME_EMPIRE_MAP (pszEmpMap, iGameClass, iGameNumber, pguInfo->piEmpireKey[j]);
                                    iErrCode = t_pCache->GetFirstKey(pszEmpMap, GameEmpireMap::PlanetKey, pvPlanetKey[i], &iMapKey);
                                    if (iErrCode == ERROR_DATA_NOT_FOUND)
                                    {
                                        iErrCode = OK;
                                        continue;
                                    }
                                    RETURN_ON_ERROR(iErrCode);

                                    if (pszMessage[0] == '\0')
                                    {
                                        char pszPlanetName[MAX_PLANET_NAME_WITH_COORDINATES_LENGTH];
                                        iErrCode = GetPlanetNameWithCoordinates(strGameMap, pvPlanetKey[i].GetInteger(), pszPlanetName);
                                        RETURN_ON_ERROR(iErrCode);
                                                        
                                        sprintf(pszMessage, BEGIN_STRONG "%s" END_STRONG " was colonized by an independent ship\n", pszPlanetName);
                                    }

                                    pguInfo->pstrUpdateMessage[j] += pszMessage;
                                }
                            }
                        }
                    }
                    break;

                default:
                    // Someone owns the planet, so give the ship to the planet's owner, but only if he wants it
                    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, vOwner.GetInteger());
                    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vTemp);
                    RETURN_ON_ERROR(iErrCode);

                    if (!(vTemp.GetInteger() & REJECT_INDEPENDENT_SHIP_GIFTS))
                    {
                        iOwnerKey = vOwner.GetInteger();
                    }
                    break;
                }

                // Decide what to do with ship
                if (iOwnerKey == NO_KEY)
                {
                    // Remove ship
                    iErrCode = DeleteShipFromDeadEmpire (strEmpireShips, strGameMap, piShipKey[i], pvPlanetKey[i]);
                    RETURN_ON_ERROR(iErrCode);
                }
                else
                {
                    GET_GAME_EMPIRE_SHIPS(strIndependentShips, iGameClass, iGameNumber, INDEPENDENT);

                    // Read ship data
                    iErrCode = t_pCache->ReadRow (strEmpireShips, piShipKey[i], &pvData);
                    RETURN_ON_ERROR(iErrCode);

                    // Set default parameters
                    pvData[GameEmpireShips::iGameClass] = iGameClass;
                    pvData[GameEmpireShips::iGameNumber] = iGameNumber;
                    pvData[GameEmpireShips::iEmpireKey] = INDEPENDENT;
                    pvData[GameEmpireShips::iAction] = STAND_BY;
                    pvData[GameEmpireShips::iFleetKey] = NO_KEY;
                    pvData[GameEmpireShips::iGateDestination] = NO_KEY;
                    pvData[GameEmpireShips::iColonyBuildCost] = 0;

                    bBuilding = pvData[GameEmpireShips::iBuiltThisUpdate].GetInteger() != 0;
                    if (bBuilding)
                    {
                        pvData[GameEmpireShips::iBuiltThisUpdate] = 0;
                    }

                    // Uncloak and build
                    if (pvData[GameEmpireShips::iState].GetInteger() & CLOAKED)
                    {
                        if (bBuilding)
                        {
                            iErrCode = t_pCache->Increment(strGameMap, pvPlanetKey[i], GameMap::NumCloakedBuildShips, -1);
                            RETURN_ON_ERROR(iErrCode);
                        }
                        else
                        {
                            iErrCode = t_pCache->Increment(strGameMap, pvPlanetKey[i], GameMap::NumCloakedShips, -1);
                            RETURN_ON_ERROR(iErrCode);
                        }

                        iErrCode = t_pCache->Increment(strGameMap, pvPlanetKey[i], GameMap::NumUncloakedShips, 1);
                        RETURN_ON_ERROR(iErrCode);

                        pvData[GameEmpireShips::iState] = pvData[GameEmpireShips::iState].GetInteger() & ~CLOAKED;
                    }
                    else
                    {
                        if (bBuilding)
                        {
                            iErrCode = t_pCache->Increment(strGameMap, pvPlanetKey[i], GameMap::NumUncloakedBuildShips, -1);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = t_pCache->Increment(strGameMap, pvPlanetKey[i], GameMap::NumUncloakedShips, 1);
                            RETURN_ON_ERROR(iErrCode);
                        }
                    }

                    if (iOwnerKey == INDEPENDENT)
                    {
                        // The ship is independent
                        iErrCode = t_pCache->InsertRow(strIndependentShips, GameEmpireShips::Template, pvData, NULL);
                        RETURN_ON_ERROR(iErrCode);
                    }
                    else
                    {
                        Assert(iOwnerKey != SYSTEM);

                        GET_GAME_EMPIRE_DATA(strGameEmpireData, iGameClass, iGameNumber, iOwnerKey);
                        GET_GAME_EMPIRE_SHIPS(strGameEmpireShips, iGameClass, iGameNumber, iOwnerKey);

                        // Some empire now owns the ship
                        iErrCode = t_pCache->InsertRow(strGameEmpireShips, GameEmpireShips::Template, pvData, NULL);
                        RETURN_ON_ERROR(iErrCode);

                        // Increment fuel and maintenance costs
                        int iDeltaMaintenance = GetMaintenanceCost(pvData[GameEmpireShips::iType].GetInteger(), pvData[GameEmpireShips::iMaxBR].GetFloat());
                        iErrCode = t_pCache->Increment(strGameEmpireData, GameEmpireData::TotalMaintenance, iDeltaMaintenance);
                        RETURN_ON_ERROR(iErrCode);

                        int iDeltaFuel = GetFuelCost(pvData[GameEmpireShips::iType].GetInteger(), pvData[GameEmpireShips::iMaxBR].GetFloat());
                        iErrCode = t_pCache->Increment(strGameEmpireData, GameEmpireData::TotalFuelUse, iDeltaFuel);
                        RETURN_ON_ERROR(iErrCode);

                        unsigned int iEmpireMapKey;
                        GET_GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iOwnerKey);
                        iErrCode = t_pCache->GetFirstKey(strGameEmpireMap, GameEmpireMap::PlanetKey, pvPlanetKey[i], &iEmpireMapKey);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = t_pCache->Increment(strGameEmpireMap, iEmpireMapKey, GameEmpireMap::NumUncloakedShips, 1);
                        RETURN_ON_ERROR(iErrCode);

                        if (pguInfo != NULL && pguInfo->piEmpireKey != NULL)
                        {
                            for (j = 0; j < pguInfo->iNumEmpires; j ++)
                            {
                                if (pguInfo->piEmpireKey[j] == (int) iOwnerKey)
                                {
                                    piNumShipsAcquired[j] ++;
                                }
                            }
                        }
                    }
                }
            }

            // Add to update message for recipient empire
            if (pguInfo != NULL && pguInfo->pstrUpdateMessage != NULL)
            {
                for (j = 0; j < pguInfo->iNumEmpires; j ++)
                {
                    if (piNumShipsAcquired[j] > 0)
                    {
                        char pszMessage[128];                  
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
    }

    // Update empire's MaxEcon and MaxMil
    Variant vMax, vOld;
    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::MaxEcon, &vMax);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strSystemEmpireData, iEmpireKey, SystemEmpireData::MaxEcon, &vOld);
    RETURN_ON_ERROR(iErrCode);

    if (vMax.GetInteger() > vOld.GetInteger())
    {
        iErrCode = t_pCache->WriteData(strSystemEmpireData, iEmpireKey, SystemEmpireData::MaxEcon, vMax);
        RETURN_ON_ERROR(iErrCode);
    }

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::MaxMil, &vMax);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strSystemEmpireData, iEmpireKey, SystemEmpireData::MaxMil, &vOld);
    RETURN_ON_ERROR(iErrCode);

    if (vMax.GetInteger() > vOld.GetInteger())
    {
        iErrCode = t_pCache->WriteData(strSystemEmpireData, iEmpireKey, SystemEmpireData::MaxMil, vMax);
        RETURN_ON_ERROR(iErrCode);
    }

    /////////////////
    // Delete rows //
    /////////////////

    // GameEmpireData(I.I.I)
    iErrCode = t_pCache->DeleteAllRows(strEmpireData);
    RETURN_ON_ERROR(iErrCode);

    // GameEmpireMessages(I.I.I)
    GET_GAME_EMPIRE_MESSAGES(strEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strEmpireMessages);
    RETURN_ON_ERROR(iErrCode);

    // GameEmpireMap(I.I.I)
    GET_GAME_EMPIRE_MAP(strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strEmpireMap);
    RETURN_ON_ERROR(iErrCode);

    // GameEmpireShips(I.I.I)
    iErrCode = t_pCache->DeleteAllRows(strEmpireShips);
    RETURN_ON_ERROR(iErrCode);

    // GameEmpireFleets(I.I.I)
    GET_GAME_EMPIRE_FLEETS(strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strEmpireFleets);
    RETURN_ON_ERROR(iErrCode);

    // GameEmpireDiplomacy(I.I.I)
    GET_GAME_EMPIRE_DIPLOMACY(strEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->DeleteAllRows(strEmpireDiplomacy);
    RETURN_ON_ERROR(iErrCode);

    // Delete game from personal list
    GET_SYSTEM_EMPIRE_ACTIVE_GAMES(strEmpireActiveGames, iEmpireKey);
    iErrCode = t_pCache->GetTable(strEmpireActiveGames, &pEmpireActiveGames);
    RETURN_ON_ERROR(iErrCode);

    unsigned int iGames;
    iErrCode = pEmpireActiveGames->GetNumCachedRows(&iGames);
    RETURN_ON_ERROR(iErrCode);

    const char* ppszColumns[] = { SystemEmpireActiveGames::GameClass, SystemEmpireActiveGames::GameNumber };
    const Variant pvGameData[] = { iGameClass, iGameNumber };

    unsigned int* piGameKey = NULL, iNumEqualKeys;
    AutoFreeKeys free_piGameKey(piGameKey);
    iErrCode = pEmpireActiveGames->GetEqualKeys(ppszColumns, pvGameData, countof(ppszColumns), &piGameKey, &iNumEqualKeys);
    RETURN_ON_ERROR(iErrCode);
    Assert(iNumEqualKeys == 1);

    iErrCode = pEmpireActiveGames->DeleteRow(piGameKey[0]);
    RETURN_ON_ERROR(iErrCode);

    // If last game, empire should be checked for deletion
    if (iGames == 1)
    {
        Variant vOptions;
        iErrCode = t_pCache->ReadData(strSystemEmpireData, iEmpireKey, SystemEmpireData::Options, &vOptions);
        RETURN_ON_ERROR(iErrCode);
        
        if (vOptions.GetInteger() & EMPIRE_MARKED_FOR_DELETION)
        {
            Variant vSecretKey;
            iErrCode = t_pCache->ReadData(strSystemEmpireData, iEmpireKey, SystemEmpireData::SecretKey, &vSecretKey);
            RETURN_ON_ERROR(iErrCode);

            // Fire at will
            iErrCode = QueueDeleteEmpire (iEmpireKey, vSecretKey.GetInteger64());
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

int GameEngine::QueueDeleteEmpire (int iEmpireKey, int64 i64SecretKey) {

    EmpireIdentity* pid = new EmpireIdentity;
    Assert(pid);
    pid->iEmpireKey = iEmpireKey;
    pid->i64SecretKey = i64SecretKey;

    return global.GetAsyncManager()->QueueTask(DeleteEmpireMsg, pid);
}

int GameEngine::DeleteEmpireMsg(AsyncTask* pMessage) {

    EmpireIdentity* pid = (EmpireIdentity*)pMessage->pArguments;
    Algorithm::AutoDelete<EmpireIdentity> del(pid, false);

    GameEngine gameEngine;

    int iErrCode = gameEngine.CacheEmpireForDeletion(pid->iEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = gameEngine.DeleteEmpire(pid->iEmpireKey, &pid->i64SecretKey, false, false);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::RemoveEmpireFromGame(int iGameClass, int iGameNumber, unsigned int iEmpireKey, unsigned int iKillerEmpire) {

    int iErrCode;
    bool bFlag;

    Variant vEmpireName;
    char pszMessage [2 * MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + 192];

    // Make sure game still exists
    iErrCode = DoesGameExist (iGameClass, iGameNumber, &bFlag);
    RETURN_ON_ERROR(iErrCode);
    if (!bFlag)
    {
        return ERROR_GAME_DOES_NOT_EXIST;
    }

    // Is empire in the game
    iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
    RETURN_ON_ERROR(iErrCode);
    if (!bFlag)
    {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vState;
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vState);
    RETURN_ON_ERROR(iErrCode);

    Variant vAdmin;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    if (iKillerEmpire == NO_KEY)
    {
        // Make sure game is still open     
        if (!(vState.GetInteger() & STILL_OPEN))
        {
            return ERROR_GAME_CLOSED;
        }

        // Get empire name
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        RETURN_ON_ERROR(iErrCode);

        sprintf(pszMessage, "%s quit the game", vEmpireName.GetCharPtr());

    } else {
        
        // Get empire name
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        RETURN_ON_ERROR(iErrCode);
        
        // Get admin name
        iErrCode = GetEmpireName (iKillerEmpire, &vAdmin);
        RETURN_ON_ERROR(iErrCode);

        sprintf (
            pszMessage, 
            "%s was removed from the game by the administrator %s", 
            vEmpireName.GetCharPtr(),
            vAdmin.GetCharPtr()
            );

        // Get gameclass name
        iErrCode = GetGameClassName(iGameClass, pszGameClassName);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // Actually remove empire from game
    Variant vNumUpdates;
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vNumUpdates);
    RETURN_ON_ERROR(iErrCode);

    GameUpdateInformation guInfo = { vNumUpdates.GetInteger(), 0, NULL, NULL, NULL};
    iErrCode = DeleteEmpireFromGame(iGameClass, iGameNumber, iEmpireKey, iKillerEmpire == NO_KEY ? EMPIRE_QUIT : EMPIRE_ADMIN_REMOVED, &guInfo);
    RETURN_ON_ERROR(iErrCode);
    
    // Get number of empires remaining
    unsigned int iNumEmpires;
    GET_GAME_EMPIRES(strGameEmpires, iGameClass, iGameNumber);

    iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);
    
    bool bGameAlive = true;
    switch (iNumEmpires) {
        
    case 1:

        // If the game is still open, let it continue
        if (vState.GetInteger() & STILL_OPEN) {
            break;
        }

        // Send a system message to the empire in question
        {
            Variant* pvEmpireKey = NULL;
            AutoFreeData free(pvEmpireKey);
            unsigned int iNumEmpires, i;

            iErrCode = t_pCache->ReadColumn(
                strGameEmpires, 
                GameEmpires::EmpireKey,
                NULL,
                &pvEmpireKey, 
                &iNumEmpires
                );

            RETURN_ON_ERROR(iErrCode);

            for (i = 0; i < iNumEmpires; i ++)
            {
                if ((unsigned int) pvEmpireKey[i].GetInteger() != iEmpireKey) {

                    sprintf (
                        pszMessage, 
                        "%s was removed from %s %i by the administrator %s and the game was deleted by the server", 
                        vEmpireName.GetCharPtr(),
                        pszGameClassName,
                        iGameNumber,
                        vAdmin.GetCharPtr()
                        );
                    
                    iErrCode = SendSystemMessage (pvEmpireKey[i].GetInteger(), pszMessage, SYSTEM, MESSAGE_SYSTEM);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }

    case 0:
        
        // Game is over: clean up the game
        bGameAlive = false;
        iErrCode = CleanupGame(iGameClass, iGameNumber, GAME_RESULT_NONE);
        RETURN_ON_ERROR(iErrCode);
        break;
        
    default:
        
        // Check diplomacy
        iErrCode = CheckGameForEndConditions(iGameClass, iGameNumber, pszMessage, &bFlag);
        RETURN_ON_ERROR(iErrCode);
        
        bGameAlive = !bFlag;

        // Check for pause
        if (bGameAlive)
        {
            UTCTime tNow;
            Time::GetTime (&tNow);

            iErrCode = CheckForDelayedPause(iGameClass, iGameNumber, tNow, &bFlag);
            RETURN_ON_ERROR(iErrCode);

            if (bFlag)
            {
                iErrCode = BroadcastGameMessage(iGameClass, iGameNumber, "The game is now paused", SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
                RETURN_ON_ERROR(iErrCode);
            }
        }

        break;                  
    }

    if (bGameAlive)
    {
        // The game didn't end, so broadcast event to remaining empires in game
        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vState);
        RETURN_ON_ERROR(iErrCode);
    }

    // If empire still exists, tell him what happened
    if (iKillerEmpire != NO_KEY)
    {
        iErrCode = DoesEmpireExist (iEmpireKey, &bFlag, NULL);
        RETURN_ON_ERROR(iErrCode);
        if (bFlag)
        {
            sprintf (
                pszMessage,
                "You were removed from %s %i by the administrator %s",
                pszGameClassName,
                iGameNumber,
                vAdmin.GetCharPtr()
                );

            iErrCode = SendSystemMessage (iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
            RETURN_ON_ERROR(iErrCode);
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
    RETURN_ON_ERROR(iErrCode);

    if (!bExists)
    {
        iErrCode = ERROR_EMPIRE_IS_NOT_IN_GAME;
    }
    else
    {
        iErrCode = QuitEmpireFromGameInternal(iGameClass, iGameNumber, iEmpireKey, iKillerEmpire);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::QuitEmpireFromGameInternal (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire)
{
    // Has the game started yet?
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vStarted;
    int iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vStarted);
    RETURN_ON_ERROR(iErrCode);

    if (vStarted.GetInteger() & STARTED)
    {
        // Can't quit anymore
        return ERROR_GAME_HAS_STARTED;
    }

    GET_GAME_EMPIRES(strEmpires, iGameClass, iGameNumber);
    Variant vAdminName;
    
    char pszMessage [1024];
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    
    // Get gameclass name
    if (iKillerEmpire != NO_KEY)
    {
        iErrCode = GetGameClassName(iGameClass, pszGameClassName);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetEmpireName(iKillerEmpire, &vAdminName);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // How many empires left?
    unsigned int iNumEmpires;
    iErrCode = t_pCache->GetNumCachedRows(strEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);
    
    // If one, then we're done with this game
    if (iNumEmpires == 1)
    {
        // Destroy the game
        iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_NONE);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        // Remove empire from game
        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_QUIT, NULL);
        RETURN_ON_ERROR(iErrCode);

        // If empires remain, so broadcast to other players
        Variant vEmpireName;
        iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
        RETURN_ON_ERROR(iErrCode);
        
        if (iKillerEmpire == NO_KEY)
        {
            sprintf(pszMessage, "%s quit the game", vEmpireName.GetCharPtr());
        }
        else
        {
            sprintf (
                pszMessage, 
                "%s as removed from the game by the administrator %s", 
                vEmpireName.GetCharPtr(),
                vAdminName.GetCharPtr()
                );
        }
        
        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
    }

    // If empire still exists, tell him what happened
    if (iKillerEmpire != NO_KEY)
    {
        sprintf (
            pszMessage, 
            "You were removed from %s %i by the administrator %s",
            pszGameClassName,
            iGameNumber,
            vAdminName.GetCharPtr()
            );
            
        iErrCode = SendSystemMessage(iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::ResignEmpireFromGame(int iGameClass, int iGameNumber, int iEmpireKey)
{
    int iErrCode;
    bool bFlag;

    // Has the game started yet?
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vState;
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vState);
    RETURN_ON_ERROR(iErrCode);

    if (!(vState.GetInteger() & STARTED))
    {
        // Game has to start for this option to work
        return ERROR_GAME_HAS_NOT_STARTED;
    }

    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    char pszMessage [256];
    Variant vOldResigned, vEmpireName;

    unsigned int* piShipKey = NULL, iNumShips;
    AutoFreeKeys auto_piShipKey(piShipKey);

    // Set empire to paused
    int iGameState;
    iErrCode = RequestPause (iGameClass, iGameNumber, iEmpireKey, &iGameState);
    RETURN_ON_ERROR(iErrCode);

    int iGameClassOptions;
    iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
    RETURN_ON_ERROR(iErrCode);

    if (iGameClassOptions & ALLOW_DRAW)
    {
        // Set empire to request draw
        bool bDrawnGame;
        iErrCode = RequestDraw (iGameClass, iGameNumber, iEmpireKey, &bDrawnGame);
        RETURN_ON_ERROR(iErrCode);
    }

    // Set empire to resigned
    iErrCode = t_pCache->WriteOr(strEmpireData, GameEmpireData::Options, RESIGNED);
    RETURN_ON_ERROR(iErrCode);

    // Increment num resigned
    iErrCode = t_pCache->Increment(strGameData, GameData::NumEmpiresResigned, 1, &vOldResigned);
    RETURN_ON_ERROR(iErrCode);

    // Dismantle all ships
    GET_GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->GetAllKeys(strEmpireShips, &piShipKey, &iNumShips);
    if (iErrCode != ERROR_DATA_NOT_FOUND)
    {
        RETURN_ON_ERROR(iErrCode);

        for (unsigned int i = 0; i < iNumShips; i ++)
        {
            iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, piShipKey[i]);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // Broadcast to other players
    iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
    RETURN_ON_ERROR(iErrCode);
    
    sprintf(pszMessage, "%s resigned from the game", vEmpireName.GetCharPtr());

    iErrCode = BroadcastGameMessage(iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
    RETURN_ON_ERROR(iErrCode);

    bool bUpdate;
    iErrCode = SetEmpireReadyForUpdate(iGameClass, iGameNumber, iEmpireKey, &bUpdate);
    RETURN_ON_ERROR(iErrCode);

    // Kill game if all empires have resigned
    if (!(vState.GetInteger() & STILL_OPEN))
    {
        iErrCode = DoesGameExist(iGameClass, iGameNumber, &bFlag);
        RETURN_ON_ERROR(iErrCode);
        
        if (bFlag)
        {
            unsigned int iNumEmpires;
            iErrCode = GetNumEmpiresInGame(iGameClass, iGameNumber, &iNumEmpires);
            RETURN_ON_ERROR(iErrCode);

            if (vOldResigned.GetInteger() + 1 == (int)iNumEmpires)
            {
                iErrCode = ResignGame(iGameClass, iGameNumber);
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    return iErrCode;
}


int GameEngine::UnresignEmpire (int iGameClass, int iGameNumber, int iEmpireKey, int iAdminKey) {

    int iErrCode;
    Variant vOptions;

    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    // Is empire resigned?
    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    if (vOptions.GetInteger() & RESIGNED)
    {
        int iGameState;
        Variant vEmpireName, vAdminName;

        iErrCode = GetEmpireName(iAdminKey, &vAdminName);
        RETURN_ON_ERROR(iErrCode);

        GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
        char pszMessage[2 * MAX_EMPIRE_NAME_LENGTH + 128];

        // No more pause
        iErrCode = RequestNoPause (iGameClass, iGameNumber, iEmpireKey, &iGameState);
        RETURN_ON_ERROR(iErrCode);

        // Set empire to un-resigned
        iErrCode = t_pCache->WriteAnd(strEmpireData, GameEmpireData::Options, ~RESIGNED);
        RETURN_ON_ERROR(iErrCode);

        // Decrement num resigned
        iErrCode = t_pCache->Increment(strGameData, GameData::NumEmpiresResigned, -1);
        RETURN_ON_ERROR(iErrCode);

        // Broadcast to other players
        iErrCode = GetEmpireName(iEmpireKey, &vEmpireName);
        RETURN_ON_ERROR(iErrCode);

        sprintf (
            pszMessage,
            "%s was restored to the game by the administrator %s",
            vEmpireName.GetCharPtr(),
            vAdminName.GetCharPtr()
            );

        iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszMessage, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::SurrenderEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, SurrenderType sType)
{
    int iErrCode;
    unsigned int iEmpires, iNumAllies = 0, iNumEmpires;
    bool bFlag;

    Variant vTemp, vEmpireName, vHomeWorld, vNumUpdates, vSecretKey;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    char pszString [MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];

    // Game has to have started for this option to work
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    if (vTemp.GetInteger() & STILL_OPEN)
    {
        return ERROR_GAME_HAS_NOT_CLOSED;
    }

    // Is empire in the game?
    iErrCode = IsEmpireInGame (iGameClass, iGameNumber, iEmpireKey, &bFlag);
    RETURN_ON_ERROR(iErrCode);
    if (!bFlag)
    {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }

    // How many empires?
    iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    if (iNumEmpires == 1)
    {
        return ERROR_NOT_ENOUGH_EMPIRES;
    }

    if (sType == NORMAL_SURRENDER && iNumEmpires != 2)
    {
        return ERROR_NOT_ENOUGH_EMPIRES;
    }

    // Get empire name
    iErrCode = GetEmpireName (iEmpireKey, &vEmpireName);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::SecretKey, &vSecretKey);
    RETURN_ON_ERROR(iErrCode);

    // Get gameclass name
    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    // Get update count
    iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iUpdates = vTemp.GetInteger();

    if (sType == SC30_SURRENDER) {

        // Get empire's homeworld
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::HomeWorld, &vHomeWorld);
        RETURN_ON_ERROR(iErrCode);
        
        // Mark homeworld with Almonaster score, significance, name hash value
        GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScore, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(strGameMap, vHomeWorld.GetInteger(), GameMap::SurrenderAlmonasterScore, vTemp);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScoreSignificance, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(strGameMap, vHomeWorld.GetInteger(), GameMap::SurrenderAlmonasterSignificance, vTemp);
        RETURN_ON_ERROR(iErrCode);

        // Write hash
        iErrCode = t_pCache->WriteData(
            strGameMap,
            vHomeWorld.GetInteger(),
            GameMap::SurrenderEmpireSecretKey,
            vSecretKey
            );

        RETURN_ON_ERROR(iErrCode);

        // Get number of allies
        iErrCode = t_pCache->GetEqualKeys(
            strDiplomacy,
            GameEmpireDiplomacy::CurrentStatus,
            ALLIANCE,
            NULL,
            &iEmpires
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);
            iNumAllies += iEmpires;
        }

        // Count trades as alliances
        iErrCode = t_pCache->GetEqualKeys(
            strDiplomacy,
            GameEmpireDiplomacy::CurrentStatus,
            TRADE,
            NULL,
            &iEmpires
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);
            iNumAllies += iEmpires;
        }

        // Write num allies
        iErrCode = t_pCache->WriteData(
            strGameMap, 
            vHomeWorld.GetInteger(), 
            GameMap::SurrenderNumAllies, 
            (int)iNumAllies
            );

        RETURN_ON_ERROR(iErrCode);

        // Handle scoring
        iErrCode = UpdateScoresOn30StyleSurrender (
            iEmpireKey, 
            vEmpireName.GetCharPtr(), 
            iGameClass, 
            iGameNumber,
            iUpdates,
            pszGameClassName
            );

        RETURN_ON_ERROR(iErrCode);

        // Set homeworld value to key
        iErrCode = t_pCache->WriteData(strGameMap, vHomeWorld.GetInteger(), GameMap::HomeWorld, iEmpireKey);
        RETURN_ON_ERROR(iErrCode);

        // New name
        sprintf(pszString, RUINS_OF, vEmpireName.GetCharPtr());

        iErrCode = t_pCache->WriteData(strGameMap, vHomeWorld.GetInteger(), GameMap::Name, pszString);
        RETURN_ON_ERROR(iErrCode);
    }

    // Prepare a game end message for remaining empires, just in case the game ends
    sprintf (
        pszString,
        "Congratulations! You have won %s %i. %s surrendered and the game ended", 
        pszGameClassName,
        iGameNumber,
        vEmpireName.GetCharPtr()
        );

    iErrCode = t_pCache->ReadData(strGameData, GameData::NumUpdates, &vNumUpdates);
    RETURN_ON_ERROR(iErrCode);

    if (iNumEmpires == 2) {

        unsigned int iKey = NO_KEY, iWinnerKey = NO_KEY;

        // Find winner's key
        while (true)
        {
            iErrCode = t_pCache->GetNextKey(strGameEmpires, iKey, &iKey);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strGameEmpires, iKey, GameEmpires::EmpireKey, &vTemp);
            RETURN_ON_ERROR(iErrCode);

            iWinnerKey = vTemp.GetInteger();
            if (iWinnerKey != (unsigned int) iEmpireKey)
            {
                break;
            }
        }

        Assert(iWinnerKey != NO_KEY);

        if (sType == NORMAL_SURRENDER) {

            Variant vWinnerName;
            iErrCode = GetEmpireName (iWinnerKey, &vWinnerName);
            RETURN_ON_ERROR(iErrCode);

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
            
            RETURN_ON_ERROR(iErrCode);
        }

        iErrCode = SendSystemMessage (iWinnerKey, pszString, SYSTEM, MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = UpdateScoresOnWin (iGameClass, iGameNumber, iWinnerKey);
        RETURN_ON_ERROR(iErrCode);

        // Delete surrendering empire from game
        GameUpdateInformation guInfo = { vNumUpdates.GetInteger(), 0, NULL, NULL, NULL};

        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_SURRENDERED, &guInfo);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = CleanupGame (iGameClass, iGameNumber, GAME_RESULT_WIN);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        GameUpdateInformation guInfo = { vNumUpdates.GetInteger(), 0, NULL, NULL, NULL};
        Assert(sType != NORMAL_SURRENDER);

        // Delete surrendering empire from game
        iErrCode = DeleteEmpireFromGame (iGameClass, iGameNumber, iEmpireKey, EMPIRE_SURRENDERED, &guInfo);
        RETURN_ON_ERROR(iErrCode);

        // Check game for exit
        iErrCode = CheckGameForEndConditions (iGameClass, iGameNumber, pszString, &bFlag);
        RETURN_ON_ERROR(iErrCode);
        
        if (!bFlag)
        {
            // Game is still alive - tell empires what happened
            sprintf (
                pszString,
                "%s surrendered from the game",
                vEmpireName.GetCharPtr()
                );
            
            iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, pszString, SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
            RETURN_ON_ERROR(iErrCode);

            // Check game for pause
            UTCTime tNow;
            Time::GetTime (&tNow);

            iErrCode = CheckForDelayedPause(iGameClass, iGameNumber, tNow, &bFlag);
            RETURN_ON_ERROR(iErrCode);

            if (bFlag)
            {
                iErrCode = BroadcastGameMessage (iGameClass, iGameNumber, "The game is now paused", SYSTEM, MESSAGE_BROADCAST | MESSAGE_SYSTEM);
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    return iErrCode;
}


int GameEngine::DeleteShipFromDeadEmpire(const char* pszEmpireShips, const char* pszGameMap, 
                                         unsigned int iShipKey, unsigned int iPlanetKey)
{
    int iErrCode;
    Variant vState, vBuilt;

    iErrCode = t_pCache->ReadData(pszEmpireShips, iShipKey, GameEmpireShips::State, &vState);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(pszEmpireShips, iShipKey, GameEmpireShips::BuiltThisUpdate, &vBuilt);
    RETURN_ON_ERROR(iErrCode);

    if (vBuilt.GetInteger() != 0) {
        
        if (vState.GetInteger() & CLOAKED) {
            iErrCode = t_pCache->Increment(pszGameMap, iPlanetKey, GameMap::NumCloakedBuildShips, -1);
            RETURN_ON_ERROR(iErrCode);
        } else {
            iErrCode = t_pCache->Increment(pszGameMap, iPlanetKey, GameMap::NumUncloakedBuildShips, -1);
            RETURN_ON_ERROR(iErrCode);
        }
        
    } else {
        
        if (vState.GetInteger() & CLOAKED) {
            iErrCode = t_pCache->Increment(pszGameMap, iPlanetKey, GameMap::NumCloakedShips, -1);
            Assert(iErrCode == OK);
        } else {
            iErrCode = t_pCache->Increment(pszGameMap, iPlanetKey, GameMap::NumUncloakedShips, -1);
            RETURN_ON_ERROR(iErrCode);
#ifdef _DEBUG
            Variant vFooBar;
            iErrCode = t_pCache->ReadData(pszGameMap, iPlanetKey, GameMap::NumUncloakedShips, &vFooBar);
            Assert(iErrCode == OK && vFooBar.GetInteger() >= 0);
#endif
        }
    }

    return iErrCode;
}


int GameEngine::GetEmpirePartialMapData (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbPartialMaps, 
                                         unsigned int* piCenterKey, unsigned int* piXRadius, 
                                         unsigned int* piRadiusY) {

    ICachedTable* pGameEmpireData = NULL;
    AutoRelease<ICachedTable> rel(pGameEmpireData);

    GET_GAME_EMPIRE_DATA (pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->GetTable(pszGameEmpireData, &pGameEmpireData);
    RETURN_ON_ERROR(iErrCode);

    int iOptions;
    iErrCode = pGameEmpireData->ReadData(GameEmpireData::Options, &iOptions);
    RETURN_ON_ERROR(iErrCode);

    if (!(iOptions & PARTIAL_MAPS))
    {
        *pbPartialMaps = false;
        *piCenterKey = NO_KEY;
        *piXRadius = 0;
        *piRadiusY = 0;
    }
    else
    {
        *pbPartialMaps = true;

        iErrCode = pGameEmpireData->ReadData(GameEmpireData::PartialMapCenter, (int*) piCenterKey);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = pGameEmpireData->ReadData(GameEmpireData::PartialMapXRadius, (int*) piXRadius);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pGameEmpireData->ReadData(GameEmpireData::PartialMapYRadius, (int*) piRadiusY);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::HasEmpireResignedFromGame(int iGameClass, int iGameNumber, int iEmpireKey, bool* pbResigned) {

    Variant vOptions;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->ReadData(
        strGameEmpireData,
        GameEmpireData::Options,
        &vOptions
        );

    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }
    RETURN_ON_ERROR(iErrCode);

    *pbResigned = (vOptions.GetInteger() & RESIGNED) != 0;

    return iErrCode;
}

int GameEngine::GetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
                                               int* piDefaultBuilderPlanet, int* piResolvedPlanetKey) {

    int iErrCode;
    Variant vDefault;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::DefaultBuilderPlanet, &vDefault);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_EMPIRE_IS_NOT_IN_GAME;
    }
    RETURN_ON_ERROR(iErrCode);

    int iResolvedPlanetKey, iDefaultBuilderPlanet = vDefault.GetInteger();

    if (iErrCode == OK) {
        
        switch (iDefaultBuilderPlanet) {
            
        case NO_DEFAULT_BUILDER_PLANET:

            iResolvedPlanetKey = NO_KEY;
            break;

        case HOMEWORLD_DEFAULT_BUILDER_PLANET:

            iErrCode = t_pCache->ReadData(
                strGameEmpireData,
                GameEmpireData::HomeWorld,
                &vDefault
                );
            RETURN_ON_ERROR(iErrCode);

            iResolvedPlanetKey = vDefault.GetInteger();
            break;

        case LAST_BUILDER_DEFAULT_BUILDER_PLANET:
            
            iErrCode = t_pCache->ReadData(
                strGameEmpireData,
                GameEmpireData::LastBuilderPlanet,
                &vDefault
                );
            RETURN_ON_ERROR(iErrCode);

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
        int iErrCode = DoesPlanetExist(iGameClass, iGameNumber, iDefaultBuildPlanet, &bExists);
        RETURN_ON_ERROR(iErrCode);

        if (!bExists) {
            return ERROR_INVALID_ARGUMENT;
        }
    }

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return t_pCache->WriteData(
        strGameEmpireData,
        GameEmpireData::DefaultBuilderPlanet,
        iDefaultBuildPlanet
        );
}


int GameEngine::GetEmpireBR (int iGameClass, int iGameNumber, int iEmpireKey, int* piBR) {

    Variant vTech;
    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->ReadData(
        strGameEmpireData,
        GameEmpireData::TechLevel,
        &vTech
        );

    RETURN_ON_ERROR(iErrCode);

    *piBR = GetBattleRank (vTech.GetFloat());
    return iErrCode;
}

int GameEngine::GetEmpireMaintenanceRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfMaintenanceRatio)
{
    int iMin, iBonusMin, iMaint, iBuild;
    ICachedTable* pGameEmpireData;
    AutoRelease<ICachedTable> rel(pGameEmpireData);

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->GetTable(strGameEmpireData, &pGameEmpireData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::TotalMin, &iMin);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = pGameEmpireData->ReadData(GameEmpireData::BonusMin, &iBonusMin);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = pGameEmpireData->ReadData(GameEmpireData::TotalMaintenance, &iMaint);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = pGameEmpireData->ReadData(GameEmpireData::TotalBuild, &iBuild);
    RETURN_ON_ERROR(iErrCode);

    *pfMaintenanceRatio = GetMaintenanceRatio (iMin + iBonusMin, iMaint, iBuild);

    return iErrCode;
}

int GameEngine::GetEmpireNextMaintenanceRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfNextMaintenanceRatio)
{
    int iMin, iBonusMin, iNextMin, iMaint, iNextMaint;

    ICachedTable* pGameEmpireData = NULL;
    AutoRelease<ICachedTable> rel(pGameEmpireData);

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->GetTable(strGameEmpireData, &pGameEmpireData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::TotalMin, &iMin);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = pGameEmpireData->ReadData(GameEmpireData::BonusMin, &iBonusMin);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::NextMin, &iNextMin);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::TotalMaintenance, &iMaint);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = pGameEmpireData->ReadData(GameEmpireData::NextMaintenance, &iNextMaint);
    RETURN_ON_ERROR(iErrCode);

    int iCalcMin = iMin + iBonusMin + iNextMin;
    Assert(iCalcMin >= 0);

    int iCalcMaint = iMaint + iNextMaint;
    Assert(iCalcMaint >= 0);

    *pfNextMaintenanceRatio = GetMaintenanceRatio (iCalcMin, iCalcMaint, 0);

    return iErrCode;
}

int GameEngine::RuinEmpire (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage)
{
    int iErrCode = SendSystemMessage(iEmpireKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = UpdateScoresOnRuin (iGameClass, iGameNumber, iEmpireKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::WriteNextStatistics (int iGameClass, int iGameNumber, int iEmpireKey, int iTotalAg, int iBonusAg, float fMaxAgRatio)
{
    int iErrCode;

    unsigned int* piPlanetKey = NULL, iNumPlanets, i;
    AutoFreeKeys free(piPlanetKey);

    int iNextPop, iNextMin, iNextFuel, iNewPop;
    float fAgRatio;

    Variant vTotalPop, vPop, vMaxPop, vMin, vFuel;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

#ifdef _DEBUG

    Variant vTargetPop;
    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TargetPop, &vTargetPop);
    RETURN_ON_ERROR(iErrCode);

    int iTotalMaxPop = 0;

#endif

    // Calculate NextTotalPop
    iErrCode = t_pCache->GetEqualKeys(
        strGameMap, 
        GameMap::Owner, 
        iEmpireKey, 
        &piPlanetKey, 
        &iNumPlanets
        );
    
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalPop, &vTotalPop);
    RETURN_ON_ERROR(iErrCode);

    fAgRatio = GetAgRatio (iTotalAg + iBonusAg, vTotalPop.GetInteger(), fMaxAgRatio);
    
    iNextPop = iNextMin = iNextFuel = 0;

    for (i = 0; i < iNumPlanets; i ++) {
        
        // Calculate new planet pop
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::MaxPop, &vMaxPop);
        RETURN_ON_ERROR(iErrCode);

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
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Minerals, &vMin);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Fuel, &vFuel);
        RETURN_ON_ERROR(iErrCode);
        
        iNextMin += min (vMin.GetInteger(), iNewPop) - min (vMin.GetInteger(), vPop.GetInteger());
        iNextFuel += min (vFuel.GetInteger(), iNewPop) - min (vFuel.GetInteger(), vPop.GetInteger());
    }

#ifdef _DEBUG
    Assert(iTotalMaxPop == vTargetPop.GetInteger());
#endif

    // Write nextpop, nextmin, next fuel
    iErrCode = t_pCache->WriteData(strEmpireData, GameEmpireData::NextTotalPop, iNextPop);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->WriteData(strEmpireData, GameEmpireData::NextMin, iNextMin);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->WriteData(strEmpireData, GameEmpireData::NextFuel, iNextFuel);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

#ifdef _DEBUG

int GameEngine::CheckTargetPop (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    unsigned int* piPlanetKey = NULL, iNumPlanets, i;
    AutoFreeKeys free(piPlanetKey);

    int iNextPop, iNextMin, iNextFuel, iNewPop;
    float fAgRatio;

    Variant vTotalPop, vPop, vMaxPop, vMin, vFuel, vTemp;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    Variant vTargetPop;
    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TargetPop, &vTargetPop);
    RETURN_ON_ERROR(iErrCode);
    int iTotalMaxPop = 0;

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    float fMaxAgRatio = vTemp.GetFloat();

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalAg, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iTotalAg = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::BonusAg, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iBonusAg = vTemp.GetInteger();

    // Calculate NextTotalPop
    iErrCode = t_pCache->GetEqualKeys(
        strGameMap, 
        GameMap::Owner, 
        iEmpireKey, 
        &piPlanetKey, 
        &iNumPlanets
        );
    
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TotalPop, &vTotalPop);
    RETURN_ON_ERROR(iErrCode);

    fAgRatio = GetAgRatio (iTotalAg + iBonusAg, vTotalPop.GetInteger(), fMaxAgRatio);
    
    iNextPop = iNextMin = iNextFuel = 0;

    for (i = 0; i < iNumPlanets; i ++) {
        
        // Calculate new planet pop
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Pop, &vPop);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::MaxPop, &vMaxPop);
        RETURN_ON_ERROR(iErrCode);

        iTotalMaxPop += vMaxPop.GetInteger();
        
        // Calculate new pop
        iNewPop = GetNextPopulation (vPop.GetInteger(), fAgRatio);
        if (iNewPop > vMaxPop.GetInteger()) {
            iNewPop = vMaxPop.GetInteger();
        }
        iNextPop += iNewPop;
        
        // Calculate min increase
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Minerals, &vMin);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(strGameMap, piPlanetKey[i], GameMap::Fuel, &vFuel);
        RETURN_ON_ERROR(iErrCode);
        
        iNextMin += min (vMin.GetInteger(), iNewPop) - min (vMin.GetInteger(), vPop.GetInteger());
        iNextFuel += min (vFuel.GetInteger(), iNewPop) - min (vFuel.GetInteger(), vPop.GetInteger());
    }

    Assert(iTotalMaxPop == vTargetPop.GetInteger());

    return iErrCode;
}

#endif

int GameEngine::UpdateGameEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, 
                                         const char* pszNotepad, bool* pbTruncated)
{
    int iErrCode = UpdateGameEmpireString (
        iGameClass, 
        iGameNumber, 
        iEmpireKey, 
        GameEmpireData::Notepad, 
        pszNotepad, 
        MAX_NOTEPAD_LENGTH,
        pbTruncated
        );

    if (iErrCode == WARNING)
    {
        return iErrCode;
    }

    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::UpdateGameEmpireString (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszColumn, 
                                        const char* pszString, size_t stMaxLen, bool* pbTruncated) {

    int iErrCode;

    ICachedTable* pWriteTable = NULL;
    AutoRelease<ICachedTable> rel(pWriteTable);
    Variant vOldString;

    GET_GAME_EMPIRE_DATA (pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    *pbTruncated = false;

    iErrCode = t_pCache->GetTable(pszGameEmpireData, &pWriteTable);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pWriteTable->ReadData(pszColumn, &vOldString);
    RETURN_ON_ERROR(iErrCode);

    if (String::StrCmp (pszString, vOldString.GetCharPtr()) == 0)
    {
        iErrCode = WARNING;
    }
    else
    {
        char* pszNew = NULL;
        Algorithm::AutoDelete<char> del(pszNew, true);

        if (String::StrLen (pszString) >= stMaxLen)
        {
            pszNew = new char [stMaxLen + 1];
            Assert(pszNew);
            memcpy (pszNew, pszString, stMaxLen);
            pszNew [stMaxLen] = '\0';

            *pbTruncated = true;
            pszString = pszNew;
        }

        iErrCode = pWriteTable->WriteData(pszColumn, pszString);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


int GameEngine::GetEmpireGameProperty (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszColumn, Variant* pvProperty) {

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->ReadData(strGameEmpireData, pszColumn, pvProperty);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::SetEmpireGameProperty (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszColumn, const Variant& vProperty) {

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->WriteData(strGameEmpireData, pszColumn, vProperty);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}
