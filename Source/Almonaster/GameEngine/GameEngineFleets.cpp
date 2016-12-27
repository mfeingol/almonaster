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
//
// Output:
// **ppiFleetKeys -> Integer keys of fleets
// *piNumFleets -> Number of fleets
//
// Return the keys of the fleets belonging a given empire

int GameEngine::GetEmpireFleetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiFleetKeys, 
                                    int* piNumFleets) {

    GET_GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->GetAllKeys (
        strGameEmpireFleets,
        (unsigned int**) ppiFleetKeys, 
        (unsigned int*) piNumFleets
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iFleetKey -> Integer key of fleet
//
// Output:
// *pvFleetName -> Fleet property
//
// Return a property of a given fleet

int GameEngine::GetFleetProperty(int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, const char* pszProperty, Variant* pvFleetName)
{
    int iErrCode;
    ICachedTable* pFleets = NULL;
    AutoRelease<ICachedTable> release(pFleets);

    GET_GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->GetTable(strGameEmpireFleets, &pFleets);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pFleets->ReadData(iFleetKey, pszProperty, pvFleetName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        return ERROR_FLEET_DOES_NOT_EXIST;
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


int GameEngine::GetFleetFlag (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int iFlag, 
                              bool* pbFlag) {

    int iErrCode;
    Variant vTemp;

    GET_GAME_EMPIRE_FLEETS (strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadData(strGameEmpireFleets, iFleetKey, GameEmpireFleets::Flags, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *pbFlag = (vTemp.GetInteger() & iFlag) != 0;

    return iErrCode;
}

int GameEngine::SetFleetFlag (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int iFlag, bool bFlag)
{
    int iErrCode;

    GET_GAME_EMPIRE_FLEETS(strGameEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    if (bFlag)
    {
        iErrCode = t_pCache->WriteOr(strGameEmpireFleets, iFleetKey, GameEmpireFleets::Flags, iFlag);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_FLEET_DOES_NOT_EXIST;
        }
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = t_pCache->WriteAnd(strGameEmpireFleets, iFleetKey, GameEmpireFleets::Flags, ~iFlag);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_FLEET_DOES_NOT_EXIST;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
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

    GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    // Get the keys of all visited planets
    unsigned int* piProxyKey = NULL, i, iNumPlanets;
    Variant* pvPlanetKey = NULL;
    
    AutoFreeKeys freeKeys(piProxyKey);
    AutoFreeData freeData(pvPlanetKey);

    iErrCode = t_pCache->ReadColumn(strEmpireMap, GameEmpireMap::PlanetKey, &piProxyKey, &pvPlanetKey, &iNumPlanets);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    // Of these, get the ones that have the empire's ships on them or that are the empire's builders
    for (i = 0; i < iNumPlanets; i ++)
    {
        bAdd = false;

        // Has uncloaked ships there?
        iErrCode = t_pCache->ReadData(strEmpireMap, piProxyKey[i], GameEmpireMap::NumUncloakedShips, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if (vTemp.GetInteger() > 0)
        {
            bAdd = true;
        }
        else
        {
            // Has cloaked ships there?
            iErrCode = t_pCache->ReadData(strEmpireMap, piProxyKey[i], GameEmpireMap::NumCloakedShips, &vTemp);
            RETURN_ON_ERROR(iErrCode);

            if (vTemp.GetInteger() > 0)
            {
                bAdd = true;
            }
            else
            {
                // Belongs to the empire and has builder pop?
                iErrCode = t_pCache->ReadData(strGameMap, pvPlanetKey[i].GetInteger(), GameMap::Owner, &vTemp);
                RETURN_ON_ERROR(iErrCode);

                if (vTemp.GetInteger() == iEmpireKey)
                {
                    Variant vBuilderPopLevel;
                    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::BuilderPopLevel, &vBuilderPopLevel);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = t_pCache->ReadData(strGameMap, pvPlanetKey[i].GetInteger(), GameMap::Pop, &vTemp);
                    RETURN_ON_ERROR(iErrCode);

                    if (vTemp.GetInteger() >= vBuilderPopLevel.GetInteger())
                    {
                        bAdd = true;
                    }
                }
            }
        }

        if (bAdd) {

            if (*ppiLocationKey == NULL)
            {
                *ppiLocationKey = new int [iNumPlanets];
                Assert(*ppiLocationKey);
            }

            // Get planet key
            (*ppiLocationKey) [*piNumLocations] = pvPlanetKey[i].GetInteger();

            // Increment counter
            (*piNumLocations) ++;
        }
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

int GameEngine::CreateNewFleet (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszFleetName, int iPlanetKey, unsigned int* piFleetKey)
{
    int iErrCode, iEmpireOptions2, iFleetOptions = 0;
    bool bAccept = true;
    Variant vOwner;

    *piFleetKey = NO_KEY;

    // Make sure that the name isn't null
    if (String::IsBlank (pszFleetName))
    {
        return ERROR_EMPTY_NAME;
    }

    GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    // Make sure the name isn't in use
    unsigned int iKey;
    iErrCode = t_pCache->GetFirstKey(strEmpireFleets, GameEmpireFleets::Name, pszFleetName, &iKey);
    if (iErrCode == OK)
    {
        return ERROR_NAME_IS_IN_USE;
    }
    if (iErrCode != ERROR_DATA_NOT_FOUND)
    {
        RETURN_ON_ERROR(iErrCode);
    }

    // Make sure the fleet can be built at the planet
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vOwner);
    RETURN_ON_ERROR(iErrCode);

    if (vOwner.GetInteger() != iEmpireKey) {
        
        // Make sure we have ships there at least
        GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

        Variant vNumUncloakedShips, vNumCloakedShips;

        iErrCode = t_pCache->GetFirstKey(strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            bAccept = false;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strEmpireMap, iKey, GameEmpireMap::NumUncloakedShips, &vNumUncloakedShips);
            RETURN_ON_ERROR(iErrCode);
            
            if (vNumUncloakedShips.GetInteger() == 0)
            {
                iErrCode = t_pCache->ReadData(strEmpireMap, iKey, GameEmpireMap::NumCloakedShips, &vNumCloakedShips);
                RETURN_ON_ERROR(iErrCode);
                
                if (vNumCloakedShips.GetInteger() == 0)
                {
                    bAccept = false;
                }
            }
        }
    }

    if (!bAccept)
    {
        return ERROR_ORPHANED_FLEET;
    }

    // Get empire options
    iErrCode = GetEmpireOptions2 (iEmpireKey, &iEmpireOptions2);
    RETURN_ON_ERROR(iErrCode);

    if (iEmpireOptions2 & FLEETS_COLLAPSED_BY_DEFAULT)
    {
        iFleetOptions |= FLEET_COLLAPSED_DISPLAY;
    }

    // Insert new row
    Variant pvColVal[GameEmpireFleets::NumColumns] = 
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        pszFleetName,
        (float) 0.0,
        (float) 0.0,
        iPlanetKey,
        STAND_BY,
        iFleetOptions,
    };
    Assert(pvColVal[GameEmpireFleets::iName].GetCharPtr());
        
    // Insert row into fleets table
    iErrCode = t_pCache->InsertRow(strEmpireFleets, GameEmpireFleets::Template, pvColVal, piFleetKey);
    RETURN_ON_ERROR(iErrCode);

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

int GameEngine::GetFleetOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
                                unsigned int iFleetKey, const GameConfiguration& gcConfig,
                                FleetOrder** ppfoOrders, unsigned int* piNumOrders, unsigned int* piSelected) {

    int iErrCode, iX, iY, iSelected;

    unsigned int i, iPlanetKey, iNumShips, iNumBuildShips, * piFleetKey = NULL, iNumOrders;
    AutoFreeKeys free(piFleetKey);

    unsigned int iMaxNumOrders = NUM_CARDINAL_POINTS + NUM_SHIP_TYPES + FLEET_STANDBY_NUM_ACTIONS + 15;

    ICachedTable* pFleets = NULL;
    AutoRelease<ICachedTable> release(pFleets);

    Variant vTemp;
    String strPlanetName;
    char pszOrder [128 + MAX_PLANET_NAME_LENGTH + MAX_FLEET_NAME_LENGTH];

    FleetOrder* pfoOrders = NULL;
    AutoFreeFleetOrders freeOrders(pfoOrders, iMaxNumOrders);

    *ppfoOrders = NULL;
    *piNumOrders = 0;
    *piSelected = 0;

    GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    // Get currently selected order
    iErrCode = t_pCache->ReadData(strEmpireFleets, iFleetKey, GameEmpireFleets::Action, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iSelected = vTemp.GetInteger();

    // Get fleet location
    iErrCode = t_pCache->ReadData(strEmpireFleets, iFleetKey, GameEmpireFleets::CurrentPlanet, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iPlanetKey = vTemp.GetInteger();

    // Get planet coordinates and name
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Coordinates, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    GetCoordinates (vTemp.GetCharPtr(), &iX, &iY);
    
    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    // Filter planet name
    if (String::AtoHtml(vTemp.GetCharPtr(), &strPlanetName, 0, false) == NULL)
    {
        Assert(false);
    }

    ////////////////////////////////
    // Allocate memory for orders //
    ////////////////////////////////
    
    pfoOrders = new FleetOrder[iMaxNumOrders];
    Assert(pfoOrders);
    memset(pfoOrders, 0, iMaxNumOrders * sizeof (FleetOrder));
    Assert(FLEET_ORDER_NORMAL == 0);

    ///////////////////////////////////////////
    // Add moves if fleet has no build ships //
    ///////////////////////////////////////////

    iErrCode = GetNumShipsInFleet(iGameClass, iGameNumber, iEmpireKey, iFleetKey, &iNumShips, &iNumBuildShips);
    RETURN_ON_ERROR(iErrCode);
    Assert(iNumBuildShips >= 0);

    if (iNumBuildShips > 0)
    {
        ///////////////////
        // Add "Standby" //
        ///////////////////

        sprintf(pszOrder, "Build at %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

        pfoOrders[0].iKey = STAND_BY;
        pfoOrders[0].pszText = String::StrDup (pszOrder);
        iNumOrders = 1;
    }
    else
    {
        GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);
        GET_GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

        // Get planet proxy key from empire map
        unsigned int iPlanetProxyKey;
        iErrCode = t_pCache->GetFirstKey(strEmpireMap, GameEmpireMap::PlanetKey, iPlanetKey, &iPlanetProxyKey);
        RETURN_ON_ERROR(iErrCode);

        ///////////////////
        // Add "Standby" //
        ///////////////////

        sprintf(pszOrder, "Standby at %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

        pfoOrders[0].iKey = STAND_BY;
        pfoOrders[0].pszText = String::StrDup (pszOrder);
        iNumOrders = 1;

        ///////////////
        // Add moves //
        ///////////////

        int iNewX, iNewY, iExplored, iLink;
        Variant vNewSystemName, vNeighbourPlanetKey;
        
        iErrCode = t_pCache->ReadData(strEmpireMap, iPlanetProxyKey, GameEmpireMap::Explored, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iExplored = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Link, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iLink = vTemp.GetInteger();

        ENUMERATE_CARDINAL_POINTS(i) {
        
            if ((iLink & LINK_X[i]) && (iExplored & EXPLORED_X[i])) {
                
                // Get neighbouring planet's key
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    iPlanetKey, 
                    GameMap::ColumnNames[GameMap::iNorthPlanetKey + i],
                    &vNeighbourPlanetKey
                    );

                RETURN_ON_ERROR(iErrCode);
                
                // Get neighbouring planet's name
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    vNeighbourPlanetKey.GetInteger(), 
                    GameMap::Name, 
                    &vNewSystemName
                    );
                RETURN_ON_ERROR(iErrCode);
                
                AdvanceCoordinates (iX, iY, &iNewX, &iNewY, i);

                sprintf (
                    pszOrder, 
                    "Move %s to %s (%i,%i)", 
                    CARDINAL_STRING[i], 
                    vNewSystemName.GetCharPtr(), 
                    iNewX, 
                    iNewY
                    );

                Assert(iNumOrders < iMaxNumOrders);
                pfoOrders[iNumOrders].iKey = MOVE_NORTH - i;
                pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                iNumOrders ++;
            }
        }
    }

    if (iNumShips > 0 && iNumBuildShips == 0) {

        //////////////////////////
        // Special ships orders //
        //////////////////////////

        int iMask = 0;
        iErrCode = GetFleetSpecialActionMask (iGameClass, iGameNumber, iEmpireKey, iFleetKey, gcConfig, &iMask);
        RETURN_ON_ERROR(iErrCode);

        if (iMask & TECH_NUKE) {

            sprintf(pszOrder, "Nuke %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

            Assert(iNumOrders < iMaxNumOrders);
            pfoOrders[iNumOrders].iKey = NUKE;
            pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            iNumOrders ++;
        }

        if (iMask & TECH_COLONY) {

            sprintf(pszOrder, "Standby and colonize %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

            Assert(iNumOrders < iMaxNumOrders);
            pfoOrders[iNumOrders].iKey = FLEET_STANDBY_AND_COLONIZE;
            pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            iNumOrders ++;
        }

        if (iMask & TECH_TERRAFORMER) {

            sprintf(pszOrder, "Standby and terraform %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

            Assert(iNumOrders < iMaxNumOrders);
            pfoOrders[iNumOrders].iKey = FLEET_STANDBY_AND_TERRAFORM;
            pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            iNumOrders ++;
        }

        if (iMask & TECH_TROOPSHIP) {

            sprintf(pszOrder, "Standby and invade %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

            Assert(iNumOrders < iMaxNumOrders);
            pfoOrders[iNumOrders].iKey = FLEET_STANDBY_AND_INVADE;
            pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            iNumOrders ++;
        }

        if (iMask & TECH_DOOMSDAY) {

            sprintf(pszOrder, "Standby and annihilate %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

            Assert(iNumOrders < iMaxNumOrders);
            pfoOrders[iNumOrders].iKey = FLEET_STANDBY_AND_ANNIHILATE;
            pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            iNumOrders ++;
        }
    }

    ////////////////////////////////
    // Pick up unaffiliated ships //
    ////////////////////////////////

    unsigned int iNumSuchShips;
    iErrCode = GetUnaffiliatedMobileShipsAtPlanet (iGameClass, iGameNumber, iEmpireKey, iPlanetKey, NULL, &iNumSuchShips);
    RETURN_ON_ERROR(iErrCode);

    if (iNumSuchShips > 0)
    {
        sprintf(pszOrder, "Pick up unaffiliated ships at %s (%i,%i)", strPlanetName.GetCharPtr(), iX, iY);

        Assert(iNumOrders < iMaxNumOrders);
        pfoOrders[iNumOrders].iKey = PICK_UP_UNAFFILIATED_SHIPS;
        pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
        iNumOrders ++;
    }

    if (iNumShips > 0)
    {
        ///////////
        // Merge //
        ///////////

        // Find fleets on same planet
        iErrCode = t_pCache->GetTable(strEmpireFleets, &pFleets);
        RETURN_ON_ERROR(iErrCode);

        unsigned int iNumFleets;
        iErrCode = pFleets->GetEqualKeys(GameEmpireFleets::CurrentPlanet, iPlanetKey, &piFleetKey, &iNumFleets);
        // We should always find at least ourselves
        RETURN_ON_ERROR(iErrCode);
        Assert(iNumFleets >= 1);

        if (iNumFleets > 1)
        {
            // '2' refers to Disband and Disband & Dismantle
            // + 10 is for good luck and buffering for the next section
            const unsigned int iNewMaxNumOrders = iNumOrders + 2 + iNumFleets + 10;

            if (iNewMaxNumOrders > iMaxNumOrders) {

                // + 10 is for good luck and buffering for the next section
                iMaxNumOrders = iNewMaxNumOrders;

                // Damn, we have to realloc; luckily, we'll hardly ever hit this case
                FleetOrder* pfoOrdersTemp = new FleetOrder [iMaxNumOrders];
                Assert(pfoOrdersTemp);
                Assert(FLEET_ORDER_NORMAL == 0);
                memset (pfoOrdersTemp, 0, iMaxNumOrders * sizeof (FleetOrder));
                memcpy (pfoOrdersTemp, pfoOrders, iNumOrders * sizeof (FleetOrder));

                delete [] pfoOrders;
                pfoOrders = pfoOrdersTemp;
            }

            for (i = 0; i < iNumFleets; i ++)
            {
                if (piFleetKey[i] == iFleetKey)
                {
                    continue;
                }

                String strFleetName;

                iErrCode = pFleets->ReadData(piFleetKey[i], GameEmpireFleets::Name, &vTemp);
                RETURN_ON_ERROR(iErrCode);

                if (String::AtoHtml(vTemp.GetCharPtr(), &strFleetName, 0, false) == NULL)
                {
                    Assert(false);
                }

                sprintf(pszOrder, "Merge into fleet %s", strFleetName.GetCharPtr());

                Assert(iNumOrders < iMaxNumOrders);
                pfoOrders[iNumOrders].iKey = piFleetKey[i];
                pfoOrders[iNumOrders].fotType = FLEET_ORDER_MERGE;
                pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
                iNumOrders ++;
            }
        }
    }

    // Add fleet moves to different planets
    if (iNumBuildShips > 0 && iNumShips == iNumBuildShips)
    {
        unsigned int iNumLocations, iNumRealLocations;
        BuildLocation* pblBuildLoc = NULL;
        Algorithm::AutoDelete<BuildLocation> autoDel (pblBuildLoc, true);

        iErrCode = GetBuildLocations(iGameClass, iGameNumber, iEmpireKey, NO_KEY, &pblBuildLoc, &iNumLocations);
        RETURN_ON_ERROR(iErrCode);

        // Count the locations we'll be adding
        iNumRealLocations = 0;
        for (i = 0; i < iNumLocations; i ++)
        {
            // Ignore our planet and 'new fleet' locations
            if (pblBuildLoc[i].iPlanetKey == iPlanetKey || pblBuildLoc[i].iFleetKey == FLEET_NEWFLEETKEY)
            {
                continue;
            }
            iNumRealLocations ++;
        }

        // '2' refers to Disband and Disband & Dismantle
        const unsigned int iNewMaxNumOrders = iNumOrders + 2 + iNumRealLocations;

        if (iNewMaxNumOrders > iMaxNumOrders) {

            iMaxNumOrders = iNewMaxNumOrders;

            // We have to realloc
            FleetOrder* pfoOrdersTemp = new FleetOrder [iMaxNumOrders];
            Assert(pfoOrdersTemp);
            Assert(FLEET_ORDER_NORMAL == 0);
            memset(pfoOrdersTemp, 0, iMaxNumOrders * sizeof (FleetOrder));
            memcpy(pfoOrdersTemp, pfoOrders, iNumOrders * sizeof (FleetOrder));

            delete [] pfoOrders;
            pfoOrders = pfoOrdersTemp;
        }

        String strFleetName;
        int iPlanetX = 0, iPlanetY = 0;
        unsigned int iCachedPlanetKey = NO_KEY;
        
        for (i = 0; i < iNumLocations; i ++) {

            // Ignore our planet and 'new fleet' locations
            if (pblBuildLoc[i].iPlanetKey == iPlanetKey || pblBuildLoc[i].iFleetKey == FLEET_NEWFLEETKEY) {
                continue;
            }

            if (iCachedPlanetKey != pblBuildLoc[i].iPlanetKey) {

                iErrCode = GetPlanetName (iGameClass, iGameNumber, pblBuildLoc[i].iPlanetKey, &vTemp);
                RETURN_ON_ERROR(iErrCode);

                if (String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false) == NULL)
                {
                    Assert(false);
                }

                iErrCode = GetPlanetCoordinates (iGameClass, iGameNumber, pblBuildLoc[i].iPlanetKey, &iPlanetX, &iPlanetY);
                RETURN_ON_ERROR(iErrCode);

                iCachedPlanetKey = pblBuildLoc[i].iPlanetKey;
            }

            sprintf(pszOrder, "Build at %s (%i,%i)", strPlanetName.GetCharPtr(), iPlanetX, iPlanetY);

            if (pblBuildLoc[i].iFleetKey == NO_KEY) {

                pfoOrders[iNumOrders].iKey = pblBuildLoc[i].iPlanetKey;
                pfoOrders[iNumOrders].fotType = FLEET_ORDER_MOVE_PLANET;
            
            } else {

                pfoOrders[iNumOrders].iKey = pblBuildLoc[i].iFleetKey;
                pfoOrders[iNumOrders].fotType = FLEET_ORDER_MOVE_FLEET;

                iErrCode = GetFleetProperty(iGameClass, iGameNumber, iEmpireKey, pblBuildLoc[i].iFleetKey, GameEmpireFleets::Name, &vTemp);
                RETURN_ON_ERROR(iErrCode);

                if (String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false) == NULL)
                {
                    Assert(false);
                }

                strcat (pszOrder, " in fleet ");
                strcat (pszOrder, strFleetName.GetCharPtr());
            }

            Assert(iNumOrders < iMaxNumOrders);
            pfoOrders[iNumOrders].pszText = String::StrDup (pszOrder);
            iNumOrders ++;
        }
    }

    // Add disband
    Assert(iNumOrders < iMaxNumOrders);
    pfoOrders[iNumOrders].iKey = DISBAND;
    pfoOrders[iNumOrders].pszText = String::StrDup ("Disband fleet");
    iNumOrders ++;

    // Add disband and dismantle if there are ships in the fleet
    if (iNumShips > 0) {

        Assert(iNumOrders < iMaxNumOrders);
        pfoOrders[iNumOrders].iKey = DISBAND_AND_DISMANTLE;
        pfoOrders[iNumOrders].pszText = String::StrDup ("Disband fleet and dismantle ships");
        iNumOrders ++;
    }

    // Find selected
    for (i = 0; i < iNumOrders; i ++)
    {
        if (pfoOrders[i].iKey == iSelected)
        {
            *piSelected = i;
            break;
        }
    }

    // Ensure no allocation errors
    for (i = 0; i < iNumOrders; i ++)
    {
        Assert(pfoOrders[i].pszText);
    }

    *ppfoOrders = pfoOrders;
    pfoOrders = NULL;

    *piNumOrders = iNumOrders;

    return iErrCode;
}

void GameEngine::FreeFleetOrders(FleetOrder* pfoOrders, unsigned int iMaxNumOrders)
{
    for (unsigned int i = 0; i < iMaxNumOrders; i ++)
    {
        if (pfoOrders[i].pszText != NULL)
        {
            OS::HeapFree (pfoOrders[i].pszText);
        }
    }
    delete [] pfoOrders;
}

int GameEngine::GetFleetSpecialActionMask(unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
                                          unsigned int iFleetKey, const GameConfiguration& gcConfig, int* piMask) {

    int iErrCode;
    unsigned int iNumShips, i, * piShipKey = NULL, iPlanetKey;
    AutoFreeKeys free(piShipKey);

    ICachedTable* pShips = NULL;
    AutoRelease<ICachedTable> release(pShips);

    *piMask = 0;

    GET_GAME_EMPIRE_SHIPS(pszShips, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_FLEETS(pszFleets, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->GetTable(pszShips, &pShips);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pShips->GetEqualKeys(GameEmpireShips::FleetKey, iFleetKey, &piShipKey, &iNumShips);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    // Get planet key
    Variant vTemp;
    iErrCode = t_pCache->ReadData(pszFleets, iFleetKey, GameEmpireFleets::CurrentPlanet, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iPlanetKey = vTemp.GetInteger();

    // Run tests
    unsigned int iNumCloaked = 0;
    int iTestMask = 0, iMask = 0;

    for (i = 0; i < iNumShips; i ++)
    {
        iErrCode = pShips->ReadData(piShipKey[i], GameEmpireShips::BuiltThisUpdate, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        // Ignore ships built this update
        if (vTemp.GetInteger() != 0)
            continue;

        iErrCode = pShips->ReadData(piShipKey[i], GameEmpireShips::Type, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        int iType = vTemp.GetInteger();

        iErrCode = pShips->ReadData(piShipKey[i], GameEmpireShips::State, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        if (vTemp.GetInteger() & CLOAKED)
        {
            iNumCloaked ++;
        }

        switch (iType) {

        case COLONY:

            if (!(iTestMask & TECH_COLONY)) {

                bool bColonize, bSettle;
                
                iErrCode = GetColonyOrders(iGameClass, iGameNumber, iEmpireKey, iPlanetKey, &bColonize, &bSettle);
                RETURN_ON_ERROR(iErrCode);

                if (bColonize)
                {
                    iMask |= TECH_COLONY;
                }

                iTestMask |= TECH_COLONY;
            }
            break;

        case TERRAFORMER:

            if (!(iTestMask & TECH_TERRAFORMER)) {

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

                if (bTerraform || bTerraformAndDismantle) {
                    iMask |= TECH_TERRAFORMER;
                }

                iTestMask |= TECH_TERRAFORMER;
            }
            break;

        case TROOPSHIP:
            
            if (!(iTestMask & TECH_TROOPSHIP)) {

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

                if (bInvade || bInvadeAndDismantle) {
                    iMask |= TECH_TROOPSHIP;
                }

                iTestMask |= TECH_TROOPSHIP;
            }
            break;

        case DOOMSDAY:
            
            if (!(iTestMask & TECH_DOOMSDAY)) {

                bool bAnnihilate;
                int iGameClassOptions;

                iErrCode = GetGameClassOptions (iGameClass, &iGameClassOptions);
                RETURN_ON_ERROR(iErrCode);

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
                    iMask |= TECH_DOOMSDAY;
                }

                iTestMask |= TECH_DOOMSDAY;
            }
            break;

        default:
            continue;
        
        }   // End switch on ship type

        // If we've already tested the entire possible mask, stop checking new ships
        if (iTestMask == FLEET_STANDBY_TECHMASK) {
            break;
        }
    }

    //////////
    // Nuke //
    //////////

    Assert(iNumCloaked <= iNumShips);

    // Add nuked only if not all active ships are cloaked
    if (iNumCloaked < iNumShips)
    {
        GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

        iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Owner, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        unsigned int iOwner = vTemp.GetInteger();

        if (iOwner == INDEPENDENT) {
            iMask |= TECH_NUKE;
        }
        else if (iOwner != SYSTEM && iOwner != iEmpireKey) {

            GET_GAME_EMPIRE_DIPLOMACY (strEmpireDip, iGameClass, iGameNumber, iEmpireKey);

            unsigned int iKey;

            iErrCode = t_pCache->GetFirstKey(
                strEmpireDip, 
                GameEmpireDiplomacy::ReferenceEmpireKey, 
                iOwner, 
                &iKey
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);

            iErrCode = t_pCache->ReadData(strEmpireDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vTemp);
            RETURN_ON_ERROR(iErrCode);

            if (vTemp.GetInteger() == WAR) {
                iMask |= TECH_NUKE;
            }
        }
    }

    *piMask |= iMask;

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

int GameEngine::UpdateFleetName(int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, const char* pszNewName)
{
    GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);

    // Make sure that the name isn't null
    if (String::IsBlank (pszNewName))
    {
        return ERROR_EMPTY_NAME;
    }

    // Write new name
    int iErrCode = t_pCache->WriteData(strEmpireFleets, iFleetKey, GameEmpireFleets::Name, pszNewName);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        iErrCode = ERROR_FLEET_DOES_NOT_EXIST;

    return iErrCode;
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

int GameEngine::UpdateFleetOrders(unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
                                  unsigned int iFleetKey, const FleetOrder& foOrder)
{
    int iErrCode;
    unsigned int i, iPlanetProxyKey, * piShipKey = NULL, iNumShips;
    AutoFreeKeys free(piShipKey);

    Variant vOldAction, vFleetPlanet = NO_KEY, vNumShips, vLink, vExplored, vTemp;

    GET_GAME_EMPIRE_FLEETS (strEmpireFleets, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_MAP (strEmpireMap, iGameClass, iGameNumber, iEmpireKey);

    int iOrderKey = foOrder.iKey;
    FleetOrderType fotType = foOrder.fotType;

    Assert(foOrder.pszText == NULL);

    iErrCode = t_pCache->ReadData(strEmpireFleets, iFleetKey, GameEmpireFleets::Action, &vOldAction);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_FLEET_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    if (iOrderKey == vOldAction.GetInteger())
    {
        return OK;
    }

    switch (fotType) {
        
    case FLEET_ORDER_MERGE:
    case FLEET_ORDER_MOVE_FLEET:

        // Order key is the target fleet key
        iErrCode = MergeFleets (iGameClass, iGameNumber, iEmpireKey, iFleetKey, iOrderKey, NO_KEY);
        RETURN_ON_ERROR(iErrCode);
        return iErrCode;

    case FLEET_ORDER_MOVE_PLANET:

        // Order key is the target planet key
        iErrCode = MergeFleets (iGameClass, iGameNumber, iEmpireKey, iFleetKey, NO_KEY, iOrderKey);
        RETURN_ON_ERROR(iErrCode);
        return iErrCode;
    }

    //
    // Only normal orders reach here
    //
    if (fotType != FLEET_ORDER_NORMAL)
    {
        return ERROR_UNKNOWN_ORDER;
    }

    // Check for standby
    if (iOrderKey == STAND_BY)
    {
        iErrCode = t_pCache->WriteData(strEmpireFleets, iFleetKey, GameEmpireFleets::Action, STAND_BY);
        RETURN_ON_ERROR(iErrCode);
        return iErrCode;
    }

    // Check for pickup
    if (iOrderKey == PICK_UP_UNAFFILIATED_SHIPS)
    {
        unsigned int iFleetPlanet;
        iErrCode = t_pCache->ReadData(strEmpireFleets, iFleetKey, GameEmpireFleets::CurrentPlanet, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iFleetPlanet = vTemp.GetInteger();

        iErrCode = GetUnaffiliatedMobileShipsAtPlanet(iGameClass, iGameNumber, iEmpireKey, iFleetPlanet, &piShipKey, &iNumShips);
        RETURN_ON_ERROR(iErrCode);

        if (iNumShips == 0)
        {
            return ERROR_WRONG_NUMBER_OF_SHIPS;
        }

        ShipOrder soOrder;
        soOrder.iKey = iFleetKey;
        soOrder.pszText = NULL;
        soOrder.sotType = SHIP_ORDER_NORMAL;

        for (i = 0; i < iNumShips; i ++)
        {
            iErrCode = UpdateShipOrders (iGameClass, iGameNumber, iEmpireKey, piShipKey[i], soOrder);
            RETURN_ON_ERROR(iErrCode);
        }

        return iErrCode;
    }

    // Check for fleet disband
    if (iOrderKey == DISBAND || iOrderKey == DISBAND_AND_DISMANTLE)
    {
        // Mark all member ships fleetkeys as NO_KEY
        GET_GAME_EMPIRE_SHIPS (strEmpireShips, iGameClass, iGameNumber, iEmpireKey);

        iErrCode = t_pCache->GetEqualKeys(
            strEmpireShips, 
            GameEmpireShips::FleetKey, 
            iFleetKey, 
            &piShipKey, 
            &iNumShips
            );

        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);

            Variant vBuiltThisUpdate, vMaxBR, vType, vAction;
            GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

            ShipOrder soOrder;
            soOrder.pszText = NULL;
            soOrder.sotType = SHIP_ORDER_NORMAL;

            // Best effort loop through all ships
            for (i = 0; i < iNumShips; i ++)
            {
                // Set no fleetkey
                iErrCode = t_pCache->WriteData(
                    strEmpireShips, 
                    piShipKey[i], 
                    GameEmpireShips::FleetKey, 
                    (int)NO_KEY
                    );

                RETURN_ON_ERROR(iErrCode);

                // Being build this update?
                iErrCode = t_pCache->ReadData(
                    strEmpireShips, 
                    piShipKey[i], 
                    GameEmpireShips::BuiltThisUpdate, 
                    &vBuiltThisUpdate
                    );
                
                RETURN_ON_ERROR(iErrCode);

                if (iOrderKey == DISBAND_AND_DISMANTLE) {

                    if (vBuiltThisUpdate.GetInteger() != 0) {

                        // Dismantle now
                        iErrCode = DeleteShip (iGameClass, iGameNumber, iEmpireKey, piShipKey[i]);
                        RETURN_ON_ERROR(iErrCode);

                    } else {

                        iErrCode = t_pCache->ReadData(
                            strEmpireShips, 
                            piShipKey[i], 
                            GameEmpireShips::Action, 
                            &vAction
                            );

                        RETURN_ON_ERROR(iErrCode);

                        if (vAction.GetInteger() == FLEET)
                        {
                            soOrder.iKey = DISMANTLE;
                            iErrCode = UpdateShipOrders (
                                iGameClass, 
                                iGameNumber, 
                                iEmpireKey, 
                                piShipKey[i], 
                                soOrder
                                );

                            RETURN_ON_ERROR(iErrCode);
                        }
                    }

                } else {
                    
                    if (vBuiltThisUpdate.GetInteger() != 0)
                    {
                        // Build at
                        iErrCode = t_pCache->WriteData(
                            strEmpireShips, 
                            piShipKey[i], 
                            GameEmpireShips::Action, 
                            BUILD_AT
                            );
                        
                        RETURN_ON_ERROR(iErrCode);
                        
                    } else {
                        
                        // Set ships to the old fleet action
                        // if they were just sitting in the fleet
                        iErrCode = t_pCache->ReadData(
                            strEmpireShips, 
                            piShipKey[i], 
                            GameEmpireShips::Action, 
                            &vAction
                            );
                        
                        RETURN_ON_ERROR(iErrCode);

                        if (vAction.GetInteger() == FLEET) {

                            soOrder.iKey = vOldAction.GetInteger();
                            iErrCode = UpdateShipOrders (
                                iGameClass, 
                                iGameNumber, 
                                iEmpireKey, 
                                piShipKey[i], 
                                soOrder
                                );

                            RETURN_ON_ERROR(iErrCode);
                        }
                    }
                }
            }
        }

        // Destroy the fleet and we're done
        iErrCode = t_pCache->DeleteRow(strEmpireFleets, iFleetKey);
        RETURN_ON_ERROR(iErrCode);

        return iErrCode;
    }

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    
    iErrCode = t_pCache->ReadData(strEmpireFleets, iFleetKey, GameEmpireFleets::CurrentPlanet, &vFleetPlanet);
    RETURN_ON_ERROR(iErrCode);

    // Check for nukes
    if (iOrderKey == NUKE)
    {
        // Get owner
        iErrCode = t_pCache->ReadData(strGameMap, vFleetPlanet, GameMap::Owner, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        unsigned int iOwner = vTemp.GetInteger();
        
        if (iOwner == SYSTEM || iOwner == iEmpireKey)
        {
            return ERROR_CANNOT_NUKE;
        }

        if (iOwner != INDEPENDENT)
        {
            // Check diplomatic status with owner
            GET_GAME_EMPIRE_DIPLOMACY (strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
            
            // Make sure the diplomatic status is war           
            unsigned int iKey;
            iErrCode = t_pCache->GetFirstKey(strDiplomacy, GameEmpireDiplomacy::ReferenceEmpireKey, iOwner, &iKey);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(strDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            int iDip = vTemp.GetInteger();

            if (iDip != WAR)
            {
                return ERROR_CANNOT_NUKE;
            }
        }
        
        iErrCode = t_pCache->WriteData(strEmpireFleets, iFleetKey, GameEmpireFleets::Action, NUKE);
        RETURN_ON_ERROR(iErrCode);

        return iErrCode;
    }

    // Check standby and ... orders
    if (IS_FLEET_STANDBY_ACTION (iOrderKey))
    {
        GameConfiguration gcConfig;
        iErrCode = GetGameConfiguration (&gcConfig);
        RETURN_ON_ERROR(iErrCode);

        int iMask;
        iErrCode = GetFleetSpecialActionMask (iGameClass, iGameNumber, iEmpireKey, iFleetKey, gcConfig, &iMask);
        RETURN_ON_ERROR(iErrCode);

        ShipType iTech = (ShipType) FLEET_STANDBY_TECH_FROM_ORDER(iOrderKey);
        int iBit = TECH_BITS [iTech];

        if (!(iMask & iBit))
        {
            // We don't have that kind of ship, sorry.
            return ERROR_INVALID_FLEET_ORDER;
        }

        iErrCode = t_pCache->WriteData(strEmpireFleets, iFleetKey, GameEmpireFleets::Action, iOrderKey);
        RETURN_ON_ERROR(iErrCode);

        return iErrCode;
    }
    
    //
    // Move
    //

    // Get proxy key for empire map
    iErrCode = t_pCache->GetFirstKey(strEmpireMap, GameEmpireMap::PlanetKey, vFleetPlanet, &iPlanetProxyKey);
    RETURN_ON_ERROR(iErrCode);

    ENUMERATE_CARDINAL_POINTS(i)
    {
        if (iOrderKey == MOVE_NORTH - (int) i)
        {
            // Make sure we have no build ships
            unsigned int iBuildShips;
            iErrCode = GetNumShipsInFleet(iGameClass, iGameNumber, iEmpireKey, iFleetKey, NULL, &iBuildShips);
            RETURN_ON_ERROR(iErrCode);

            if (iBuildShips > 0)
            {
                return ERROR_CANNOT_MOVE;
            }
            
            // Now, make sure we have a link to chase
            iErrCode = t_pCache->ReadData(strGameMap, vFleetPlanet, GameMap::Link, &vLink);
            RETURN_ON_ERROR(iErrCode);

            if (!(vLink.GetInteger() & LINK_X[i]))
            {
                return ERROR_CANNOT_MOVE;
            }
            
            // Make sure we've explored the link
            iErrCode = t_pCache->ReadData(strEmpireMap, iPlanetProxyKey, GameEmpireMap::Explored, &vExplored);
            RETURN_ON_ERROR(iErrCode);

            if (vExplored.GetInteger() & EXPLORED_X[i])
            {
#ifdef _DEBUG
                // Make sure there's a planet
                Variant vNewPlanetKey;
                iErrCode = t_pCache->ReadData(
                    strGameMap, 
                    vFleetPlanet, 
                    GameMap::ColumnNames[GameMap::iNorthPlanetKey + i],
                    &vNewPlanetKey
                    );
                Assert(iErrCode == OK && vNewPlanetKey != NO_KEY);
                
                iErrCode = t_pCache->GetFirstKey(
                    strEmpireMap, 
                    GameEmpireMap::PlanetKey, 
                    vNewPlanetKey, 
                    &iPlanetProxyKey
                    );
                
                Assert(iErrCode == OK && iPlanetProxyKey != NO_KEY);
#endif
                // Accept the order
                iErrCode = t_pCache->WriteData(strEmpireFleets, iFleetKey, GameEmpireFleets::Action, iOrderKey);
                RETURN_ON_ERROR(iErrCode);

                return iErrCode;
            }

            return ERROR_CANNOT_MOVE;
        }
    }

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
                                    unsigned int* piNumShips, unsigned int* piNumBuildShips)
{
    Assert(piNumShips != NULL || piNumBuildShips != NULL);

    int iErrCode = OK;
    GET_GAME_EMPIRE_SHIPS(strShips, iGameClass, iGameNumber, iEmpireKey);

    if (piNumShips)
    {
        iErrCode = t_pCache->GetEqualKeys(strShips, GameEmpireShips::FleetKey, iFleetKey, NULL, piNumShips);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    if (piNumBuildShips)
    {
        const char* ppszColumns[] = { GameEmpireShips::FleetKey, GameEmpireShips::BuiltThisUpdate };
        const Variant pvEquals[] = { iFleetKey, 1 };

        iErrCode = t_pCache->GetEqualKeys(strShips, ppszColumns, pvEquals, countof(pvEquals), NULL, piNumBuildShips);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::MergeFleets(unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
                            unsigned int iSrcKey, unsigned int iDestKey, unsigned int iPlanetKey)
{
    int iErrCode;

    unsigned int* piShipKey = NULL, iNumSrcShips, i, iSrcPlanet, iDestPlanet;
    AutoFreeKeys free(piShipKey);

    Variant vTemp, vSrcName;
    int iSrcFlags = 0;

    ICachedTable* pFleets = NULL;
    AutoRelease<ICachedTable> release(pFleets);

    GET_GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, iEmpireKey);

    if (iSrcKey == iDestKey)
    {
        return ERROR_SAME_FLEET_ORDER;
    }

    iErrCode = t_pCache->GetTable(pszFleets, &pFleets);
    RETURN_ON_ERROR(iErrCode);
    
    // See where the fleets live
    iErrCode = pFleets->ReadData(iSrcKey, GameEmpireFleets::CurrentPlanet, (int*)&iSrcPlanet);
    if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
    {
        return ERROR_FLEET_DOES_NOT_EXIST;
    }
    RETURN_ON_ERROR(iErrCode);

    if (iDestKey != NO_KEY)
    {
        iErrCode = pFleets->ReadData(iDestKey, GameEmpireFleets::CurrentPlanet, (int*) &iDestPlanet);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_FLEET_DOES_NOT_EXIST;
        }
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        if (iPlanetKey == NO_KEY)
        {
            return ERROR_INVALID_ARGUMENT;
        }
        iDestPlanet = iPlanetKey;
    }

    // Prepare the proper arguments to MoveShip
    if (iSrcPlanet == iDestPlanet)
    {
        iDestPlanet = NO_KEY;
    }
    else if (iDestKey == NO_KEY)
    {
        iErrCode = pFleets->ReadData(iSrcKey, GameEmpireFleets::Name, &vSrcName);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = pFleets->ReadData(iSrcKey, GameEmpireFleets::Flags, &iSrcFlags);
        RETURN_ON_ERROR(iErrCode);

        // We need to create a fleet for the destination planet
        iErrCode = CreateRandomFleet (iGameClass, iGameNumber, iEmpireKey, iDestPlanet, &iDestKey);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        // Careful - we can only move a fleet between planets if the source fleet is empty or only contains ships being built this update
        unsigned int iSrcShips, iSrcBuildShips;
        iErrCode = GetNumShipsInFleet(iGameClass, iGameNumber, iEmpireKey, iSrcKey, &iSrcShips, &iSrcBuildShips);
        RETURN_ON_ERROR(iErrCode);

        Assert(iSrcShips >= iSrcBuildShips);
        if (iSrcShips > 0 && iSrcShips > iSrcBuildShips)
        {
            return ERROR_INVALID_FLEET_ORDER;
        }
    }

    // Make sure the target planet exists
    if (iDestPlanet != NO_KEY)
    {
        iErrCode = GetPlanetProperty(iGameClass, iGameNumber, iDestPlanet, GameMap::Owner, &vTemp);
        if (iErrCode == ERROR_UNKNOWN_ROW_KEY)
        {
            return ERROR_WRONG_PLANET;
        }
        RETURN_ON_ERROR(iErrCode);

        if ((unsigned int) vTemp.GetInteger() != iEmpireKey)
        {
            return ERROR_WRONG_PLANET;
        }
    }

    // Okay, the fleets are compatible.  Merge them
    iErrCode = t_pCache->GetEqualKeys(pszShips, GameEmpireShips::FleetKey, iSrcKey, &piShipKey, &iNumSrcShips);
    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    // Move each ship to the new planet/fleet combination
    for (i = 0; i < iNumSrcShips; i ++)
    {
        iErrCode = MoveShip(iGameClass, iGameNumber, iEmpireKey, piShipKey[i], iDestPlanet, iDestKey);
        RETURN_ON_ERROR(iErrCode);
    }

    // Delete the source fleet
    iErrCode = t_pCache->DeleteRow(pszFleets, iSrcKey);
    RETURN_ON_ERROR(iErrCode);

    // Reset the dest fleet if necessary
    if (vSrcName.GetType() == V_STRING)
    {
        iErrCode = t_pCache->WriteData(pszFleets, iDestKey, GameEmpireFleets::Name, vSrcName);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(pszFleets, iDestKey, GameEmpireFleets::Flags, iSrcFlags);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::CreateRandomFleet(unsigned int iGameClass, unsigned int iGameNumber, unsigned int iEmpireKey, unsigned int iPlanetKey, unsigned int* piFleetKey)
{
    int iErrCode;
    *piFleetKey = NO_KEY;
    do
    {
        // Generate a random fleet name
        char pszFleetName [64];
        snprintf (pszFleetName, sizeof (pszFleetName), "%d", Algorithm::GetRandomInteger(0x7fffffff));
        
        iErrCode = CreateNewFleet(iGameClass, iGameNumber, iEmpireKey, pszFleetName, iPlanetKey, piFleetKey);
    }
    while (iErrCode == ERROR_NAME_IS_IN_USE);

    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}