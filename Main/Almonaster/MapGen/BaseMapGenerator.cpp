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

#include "BaseMapGenerator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BaseMapGenerator::BaseMapGenerator(GameEngine* pGameEngine) 
    : 
    m_htCoordinates (NULL, NULL)
{
    Assert(pGameEngine != NULL);
    m_pGameEngine = pGameEngine;

    m_iGameClass = NO_KEY;
    m_iGameNumber = 0;

    m_piNewEmpireKey = NULL;
    m_iNumNewEmpires = 0;
    
    m_ppvExistingPlanetData = NULL;
    m_iNumExistingPlanets = 0;

    m_pvGameClassData = NULL;
    m_pvGameData = NULL;

    memset(&m_mcConfig, 0, sizeof(m_mcConfig));
    m_iGameClassOptions = 0;
    m_iNumPlanetsPerEmpire = 0;
    
    m_ppvNewPlanetData = NULL;
}

BaseMapGenerator::~BaseMapGenerator()
{
    if (m_ppvNewPlanetData != NULL)
    {
        FreePlanetData(m_ppvNewPlanetData);
    }
}

// IMapGenerator
void BaseMapGenerator::FreePlanetData(Variant** ppvNewPlanetData) {

    delete [] *ppvNewPlanetData;
    delete [] ppvNewPlanetData;
}

int BaseMapGenerator::CreatePlanets (

    int iGameClass,
    int iGameNumber,

    int* piNewEmpireKey,
    unsigned int iNumNewEmpires,

    Variant** ppvExistingPlanetData,
    unsigned int iNumExistingPlanets,

    Variant* pvGameClassData,
    Variant* pvGameData,

    Variant*** pppvNewPlanetData,
    unsigned int* piNumNewPlanets
    ) {

    int iErrCode;

    // Copy data
    Assert(iGameClass != NO_KEY);
    m_iGameClass = iGameClass;

    Assert(iGameNumber != 0);
    m_iGameNumber = iGameNumber;

    Assert(piNewEmpireKey != NULL);
    m_piNewEmpireKey = piNewEmpireKey;

    Assert(iNumNewEmpires > 0);
    m_iNumNewEmpires = iNumNewEmpires;
    
    Assert(ppvExistingPlanetData == NULL || iNumExistingPlanets > 0);
    Assert(iNumExistingPlanets == 0 || ppvExistingPlanetData != NULL);
    m_ppvExistingPlanetData = ppvExistingPlanetData;
    m_iNumExistingPlanets = iNumExistingPlanets;

    Assert(pvGameClassData != NULL);
    m_pvGameClassData = pvGameClassData;

    Assert(pvGameData != NULL);
    m_pvGameData = pvGameData;

    // Initialize data
    m_iTotalNumNewPlanets = 0;
    m_iNumNewPlanetsCreated = 0;

    ResetNewPlanetChain();

    // Cleanup if needed
    if (m_ppvNewPlanetData != NULL) {
        FreePlanetData(m_ppvNewPlanetData);
        m_ppvNewPlanetData = NULL;
    }

    // Init hashtable
    if (m_htCoordinates.IsInitialized())
    {
        m_htCoordinates.Clear();
    }
    else
    {
        int iMaxNumNewPlanets = m_pvGameClassData[SystemGameClassData::iMaxNumPlanets].GetInteger();
        bool ret = m_htCoordinates.Initialize(m_iNumExistingPlanets + iNumNewEmpires * iMaxNumNewPlanets);
        Assert(ret);
    }

    // Read map configuration
    iErrCode = m_pGameEngine->GetMapConfiguration(&m_mcConfig);
    RETURN_ON_ERROR(iErrCode);

    // Cache gameclass options
    m_iGameClassOptions = pvGameClassData[SystemGameClassData::iOptions].GetInteger();

    // If necessary, compute the game's random resource values
    ComputeGameResources();

    // Insert any existing planets into the hash table cache
    InsertMapCoordinates();

    // Call the derived class to create all planet chains
    CreatePlanetChains();
    Assert(m_iTotalNumNewPlanets == m_iNumNewPlanetsCreated);

    // Set out parameters
    *pppvNewPlanetData = m_ppvNewPlanetData;
    m_ppvNewPlanetData = NULL;

    *piNumNewPlanets = m_iNumNewPlanetsCreated;

    return iErrCode;
}

void BaseMapGenerator::ResetNewPlanetChain() {

    m_iNumChainPlanetsCreated = 0;
    m_iChainHomeWorldIndex = NO_KEY;

    m_iLinkedPlanetInPreviousChainIndex = NO_KEY;
	m_iExistingPlanetLinkedToChain = NO_KEY;
    m_cpLinkedPlanetInPreviousChainDirection = NO_DIRECTION;
}

void BaseMapGenerator::RestartPlanetChain() {

    // Remove old link to existing chain if necessary
    if (m_iLinkedPlanetInPreviousChainIndex != NO_KEY && 
        !(m_iGameClassOptions & DISCONNECTED_MAP)) {

        Assert(m_cpLinkedPlanetInPreviousChainDirection != NO_DIRECTION);

        int iLink = m_ppvNewPlanetData[m_iLinkedPlanetInPreviousChainIndex][GameMap::iLink].GetInteger();
        int iNewLink = iLink & (~LINK_X[m_cpLinkedPlanetInPreviousChainDirection]);

        Assert(iNewLink != iLink);
        m_ppvNewPlanetData[m_iLinkedPlanetInPreviousChainIndex][GameMap::iLink] = iNewLink;
    }

    // Remove planets from lookup table
    unsigned int i, iNumNewPlanetsCreatedAfterRestart = m_iNumNewPlanetsCreated - m_iNumChainPlanetsCreated;
    for (i = iNumNewPlanetsCreatedAfterRestart; i < m_iNumNewPlanetsCreated; i ++) {

        const char* pszCoord = m_ppvNewPlanetData[i][GameMap::iCoordinates].GetCharPtr();
        bool bRet = m_htCoordinates.DeleteFirst(pszCoord, NULL, NULL);
        Assert(bRet);
    }

    // Backtrack counters
    m_iNumNewPlanetsCreated -= m_iNumChainPlanetsCreated;
    Assert(m_iNumNewPlanetsCreated % m_iNumPlanetsPerEmpire == 0);

    ResetNewPlanetChain();
}

int BaseMapGenerator::CreatePlanetChain(unsigned int iEmpireKey) {

    ResetNewPlanetChain();

    // Create some new planets
    while (m_iNumChainPlanetsCreated < m_iNumPlanetsPerEmpire) {

        PlanetLocation lLocation;

        // This can fail
        int iErrCode = GetNewPlanetLocation(&lLocation);
        if (iErrCode == ERROR_NO_NEW_PLANETS_AVAILABLE)
        {
            RestartPlanetChain();
            continue;
        }
        RETURN_ON_ERROR(iErrCode);

        // Create new planet
        CreatePlanet(iEmpireKey, &lLocation);
    }
    Assert(m_iNumChainPlanetsCreated == m_iNumPlanetsPerEmpire);

    // Finish off the chain
    ChooseHomeworldForChain();
    AssignResourcesForChain();
    CreateNonDefaultLinksForChain();

    return OK;
}

void BaseMapGenerator::ChooseHomeworldForChain() {

    unsigned int iFirstPlanetinChainIndex = m_iNumNewPlanetsCreated - m_iNumChainPlanetsCreated;

    unsigned int iHWIndex;
    while (true) {

        iHWIndex = Algorithm::GetRandomInteger(m_iNumChainPlanetsCreated) + iFirstPlanetinChainIndex;

        // If we chose the first planet in the chain, it's going to be linked to the map somewhere unless
		// the map is disconnected.
        // So for connected maps with > 1 planet per empire, the two planets in question must not be HWs
		if (!(m_iGameClassOptions & DISCONNECTED_MAP) &&
			iHWIndex == iFirstPlanetinChainIndex && 
			m_iNumPlanetsPerEmpire > 1) {

			// Check the linked planet from the previous chain
			if (m_iLinkedPlanetInPreviousChainIndex != NO_KEY &&
				m_ppvNewPlanetData[m_iLinkedPlanetInPreviousChainIndex][GameMap::iHomeWorld].GetInteger() == HOMEWORLD) {
				// Try again
				continue;
			}

			// Check the linked planet from the map
			if (m_iExistingPlanetLinkedToChain != NO_KEY &&
				m_ppvExistingPlanetData[m_iExistingPlanetLinkedToChain][GameMap::iHomeWorld].GetInteger() == HOMEWORLD) {
				// Try again
				continue;
			}
		}

		break;
    }

    // Mark the chosen planet as a homeworld
    m_ppvNewPlanetData[iHWIndex][GameMap::iHomeWorld] = HOMEWORLD;
    m_iChainHomeWorldIndex = iHWIndex;
}

void BaseMapGenerator::CreateNonDefaultLinksForChain() {

    // Needless to say, this function should be called _after_ we've chosen a homeworld

    for (unsigned int i = 0; i < m_iNumChainPlanetsCreated; i ++) {

        unsigned int iIndex = m_iNumNewPlanetsCreated - m_iNumChainPlanetsCreated + i;

        int iX, iY;
        GetCoordinates(m_ppvNewPlanetData[iIndex], &iX, &iY);
        int iLink = m_ppvNewPlanetData[iIndex][GameMap::iLink].GetInteger();

        int cpDir;
        ENUMERATE_CARDINAL_POINTS (cpDir) {

            // Stop if a link already exists
            if (iLink & LINK_X[cpDir])
                continue;
            
            int iNewX, iNewY;
            AdvanceCoordinates(iX, iY, &iNewX, &iNewY, (CardinalPoint) cpDir);

            char pszCoord[MAX_COORDINATE_LENGTH + 1];
            GameEngine::GetCoordinates(iNewX, iNewY, pszCoord);
            
            Variant* pvNeighborPlanetData;
            if (!m_htCoordinates.FindFirst (pszCoord, &pvNeighborPlanetData))
                continue;

            bool bInCurrentChain = IsPlanetInCurrentChain(pvNeighborPlanetData);

            // If disconnected maps, proceed only if neighbor planet is in this chain
            if ((m_iGameClassOptions & DISCONNECTED_MAP) && !bInCurrentChain)
                continue;

            // If we're in the same chain, halve the probability of a new link forming,
            // because this link will be tested again by the other planet
            int iDieSides = 100 * (bInCurrentChain ? 2 : 1);
            int iRand = Algorithm::GetRandomInteger(iDieSides);
            if (iRand >= m_mcConfig.iChanceNewLinkForms)
                continue;

            // Create a new link if both planets aren't homeworlds or the map allows 1 planet per empire
            if (m_iNumPlanetsPerEmpire == 1 || 
                m_ppvNewPlanetData[iIndex][GameMap::iHomeWorld].GetInteger() != HOMEWORLD || 
                pvNeighborPlanetData[GameMap::iHomeWorld].GetInteger() != HOMEWORLD) {
                
                // Link 'em up
                iLink |= LINK_X[cpDir];
                m_ppvNewPlanetData[iIndex][GameMap::iLink] = iLink;
                
                // If this is a planet in the map, no one will care about this change
                // If it isn't, this is needed
                int iNeighborLink = pvNeighborPlanetData[GameMap::iLink].GetInteger();
                pvNeighborPlanetData[GameMap::iLink] = iNeighborLink | OPPOSITE_LINK_X[cpDir];
            }
        }
    }
}

void BaseMapGenerator::AssignResourcesForChain() {

    AssignResources(m_iChainHomeWorldIndex,
                    m_iNumNewPlanetsCreated - m_iNumChainPlanetsCreated,
                    m_iNumChainPlanetsCreated);
}

void BaseMapGenerator::AssignResources(unsigned int iHWIndex,
                                       unsigned int iStartIndex,
                                       unsigned int iNumPlanets) {

    ////////////////////////////
    // Assign resources to HW //
    ////////////////////////////

    unsigned int iNumNonHWPlanets = iNumPlanets;
    if (iHWIndex != NO_KEY)
    {
        m_ppvNewPlanetData[iHWIndex][GameMap::iAg] = m_pvGameData[GameData::iHWAg].GetInteger();
        m_ppvNewPlanetData[iHWIndex][GameMap::iMinerals] = m_pvGameData[GameData::iHWMin].GetInteger();
        m_ppvNewPlanetData[iHWIndex][GameMap::iFuel] = m_pvGameData[GameData::iHWFuel].GetInteger();
        iNumNonHWPlanets --;
    }

    /////////////////////////////////////////
    // Assign resources to rest of planets //
    /////////////////////////////////////////

    unsigned int iAvgPlanetAg = m_pvGameData[GameData::iAvgAg].GetInteger();
    unsigned int iAvgPlanetMin = m_pvGameData[GameData::iAvgMin].GetInteger();
    unsigned int iAvgPlanetFuel = m_pvGameData[GameData::iAvgFuel].GetInteger();

    unsigned int* piAg = (unsigned int*)StackAlloc(iNumNonHWPlanets * 3 * sizeof (unsigned int));
    unsigned int* piMin = piAg + iNumNonHWPlanets;
    unsigned int* piFuel = piMin + iNumNonHWPlanets;

    unsigned int iAgCap = (unsigned int)(m_mcConfig.fResourceAllocationRandomizationFactor * (float)iAvgPlanetAg);
    unsigned int iMinCap = (unsigned int)(m_mcConfig.fResourceAllocationRandomizationFactor * (float)iAvgPlanetMin);
    unsigned int iFuelCap = (unsigned int)(m_mcConfig.fResourceAllocationRandomizationFactor * (float)iAvgPlanetFuel);

    unsigned int iTotalAg = 0;
    unsigned int iTotalMin = 0;
    unsigned int iTotalFuel = 0;

    unsigned int i, iTemp;

    // Allocate resources
    for (i = 0; i < iNumNonHWPlanets; i ++) {

        iTemp = Algorithm::GetRandomInteger(iAgCap);
        piAg[i] = iTemp;
        iTotalAg += iTemp;

        iTemp = Algorithm::GetRandomInteger(iMinCap);
        piMin[i] = iTemp;
        iTotalMin += iTemp;

        iTemp = Algorithm::GetRandomInteger(iFuelCap);
        piFuel[i] = iTemp;
        iTotalFuel += iTemp;
    }

    // Normalize resources to constraints
    float fAgFactor, fMinFactor, fFuelFactor;

    unsigned int iGlobalAg = iAvgPlanetAg * iNumNonHWPlanets;
    unsigned int iGlobalMin = iAvgPlanetMin * iNumNonHWPlanets;
    unsigned int iGlobalFuel = iAvgPlanetFuel * iNumNonHWPlanets;

    fAgFactor = (float) iGlobalAg / iTotalAg;
    fMinFactor = (float) iGlobalMin / iTotalMin;
    fFuelFactor = (float) iGlobalFuel / iTotalFuel;

    iTotalAg = iTotalMin = iTotalFuel = 0;

    for (i = 0; i < iNumNonHWPlanets; i ++) {

        iTemp = (unsigned int)((float)piAg[i] * fAgFactor);
        piAg[i] = iTemp;
        iTotalAg += iTemp;

        iTemp = (unsigned int)((float)piMin[i] * fMinFactor);
        piMin[i] = iTemp;
        iTotalMin += iTemp;

        iTemp = (unsigned int)((float)piFuel[i] * fFuelFactor);
        piFuel[i] = iTemp;
        iTotalFuel += iTemp;
    }

    // Compensate for floating point errors
    int iResDiff;
    
    iResDiff = iGlobalAg - iTotalAg;
    if (iResDiff != 0) {
        iTemp = Algorithm::GetRandomInteger(iNumNonHWPlanets);
        piAg[iTemp] += iResDiff;
    }
    
    iResDiff = iGlobalMin - iTotalMin;
    if (iResDiff != 0) {
        iTemp = Algorithm::GetRandomInteger(iNumNonHWPlanets);
        piMin[iTemp] += iResDiff;
    }
    
    iResDiff = iGlobalFuel - iTotalFuel;
    if (iResDiff != 0) {
        iTemp = Algorithm::GetRandomInteger(iNumNonHWPlanets);
        piFuel[iTemp] += iResDiff;
    }

    // Save resources to planet data
    unsigned int iCounter = 0;
    unsigned int iEndIndex = iStartIndex + iNumPlanets;
    for (unsigned int i = iStartIndex; i < iEndIndex; i ++) {

        if (i == iHWIndex)
            continue;

        m_ppvNewPlanetData[i][GameMap::iAg] = piAg[iCounter];
        m_ppvNewPlanetData[i][GameMap::iMinerals] = piMin[iCounter];
        m_ppvNewPlanetData[i][GameMap::iFuel] = piFuel[iCounter];

        iCounter ++;
    }
}

void BaseMapGenerator::ComputeGameResources() {

    int iMin, iMax;

    if (m_iNumExistingPlanets > 0) {

        m_iNumPlanetsPerEmpire = m_pvGameData[GameData::iNumPlanetsPerEmpire].GetInteger();
    
    } else {

        // It's our job to compute all these things now...

        //
        // Number of planets per empire
        //

        iMin = m_pvGameClassData[SystemGameClassData::iMinNumPlanets].GetInteger();
        iMax = m_pvGameClassData[SystemGameClassData::iMaxNumPlanets].GetInteger();
        Assert(iMin <= iMax);

        m_iNumPlanetsPerEmpire = iMin;
        if (iMin != iMax) {
            m_iNumPlanetsPerEmpire = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);
        }

        m_pvGameData[GameData::iNumPlanetsPerEmpire] = m_iNumPlanetsPerEmpire;

        //
        // Homeworld resources
        //
        
        // HWAg
        iMin = m_pvGameClassData[SystemGameClassData::iMinAgHW].GetInteger();
        iMax = m_pvGameClassData[SystemGameClassData::iMaxAgHW].GetInteger();

        Assert(iMin <= iMax);
        
        m_pvGameData[GameData::iHWAg] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

        // HWMin
        iMin = m_pvGameClassData[SystemGameClassData::iMinMinHW].GetInteger();
        iMax = m_pvGameClassData[SystemGameClassData::iMaxMinHW].GetInteger();

        Assert(iMin <= iMax);

        m_pvGameData[GameData::iHWMin] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

        // HWFuel
        iMin = m_pvGameClassData[SystemGameClassData::iMinFuelHW].GetInteger();
        iMax = m_pvGameClassData[SystemGameClassData::iMaxFuelHW].GetInteger();

        Assert(iMin <= iMax);

        m_pvGameData[GameData::iHWFuel] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

        //
        // Planet resources
        //
        
        // Planet ag
        iMin = m_pvGameClassData[SystemGameClassData::iMinAvgAg].GetInteger();
        iMax = m_pvGameClassData[SystemGameClassData::iMaxAvgAg].GetInteger();

        Assert(iMin <= iMax);
        
        m_pvGameData[GameData::iAvgAg] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

        // Planet min
        iMin = m_pvGameClassData[SystemGameClassData::iMinAvgMin].GetInteger();
        iMax = m_pvGameClassData[SystemGameClassData::iMaxAvgMin].GetInteger();

        Assert(iMin <= iMax);
        
        m_pvGameData[GameData::iAvgMin] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

        // Planet fuel
        iMin = m_pvGameClassData[SystemGameClassData::iMinAvgFuel].GetInteger();
        iMax = m_pvGameClassData[SystemGameClassData::iMaxAvgFuel].GetInteger();

        Assert(iMin <= iMax);

        m_pvGameData[GameData::iAvgFuel] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);
    }
}

void BaseMapGenerator::CreatePlanet(unsigned int iEmpireKey, PlanetLocation* plLocation) {

    int iX, iY;
    unsigned int iNewPlanetIndex = m_iNumNewPlanetsCreated;

    // Set default values
    m_ppvNewPlanetData[iNewPlanetIndex][GameMap::iGameClass] = m_iGameClass;
    m_ppvNewPlanetData[iNewPlanetIndex][GameMap::iGameNumber] = m_iGameNumber;
    m_ppvNewPlanetData[iNewPlanetIndex][GameMap::iOwner] = iEmpireKey;
    m_ppvNewPlanetData[iNewPlanetIndex][GameMap::iHomeWorld] = NOT_HOMEWORLD;
    m_ppvNewPlanetData[iNewPlanetIndex][GameMap::iLink] = 0;
    m_ppvNewPlanetData[iNewPlanetIndex][GameMap::iCoordinates].Clear();

    if (plLocation->cpDirectionFromCoordinates == NO_DIRECTION) {

        // The very first planet on the map, not just in the chain
        Assert(iNewPlanetIndex == 0);
        Assert(plLocation->iNumLinkedPlanets == 0);

        iX = plLocation->iX;
        iY = plLocation->iY;

        // No links to anyone yet

    } else {

        AdvanceCoordinates(plLocation->iX, plLocation->iY, &iX, &iY, plLocation->cpDirectionFromCoordinates);

        for (unsigned int i = 0; i < plLocation->iNumLinkedPlanets; i ++) {

            const CardinalPoint cp = plLocation->pcpLinkDirection[i];

            // Add link from new to old planet
            AddLink(iNewPlanetIndex, cp);

            // Add link from old to new planet
            const unsigned int iPrevIndex = plLocation->piLinkPlanetIndex[i];
            if (iPrevIndex != NO_KEY)
            {
                AddLink(iPrevIndex, OPPOSITE_CARDINAL_POINT[cp]);
            }
        }
    }

    SetCoordinates(iNewPlanetIndex, iX, iY);

    InsertIntoCoordinatesTable(iNewPlanetIndex);

    m_iNumChainPlanetsCreated ++;
    m_iNumNewPlanetsCreated ++;
}

void BaseMapGenerator::InsertIntoCoordinatesTable(unsigned int iPlanetIndex) {

    const char* pszCoord = m_ppvNewPlanetData[iPlanetIndex][GameMap::iCoordinates].GetCharPtr();

    // Sanity check - shouldn't exist already
    Assert(!m_htCoordinates.FindFirst(pszCoord, (Variant**) NULL));

    // Insert into coordinates hash table
    bool ret = m_htCoordinates.Insert(pszCoord, m_ppvNewPlanetData[iPlanetIndex]);
    Assert(ret);
}

void BaseMapGenerator::SetCoordinates(unsigned int iPlanetIndex, int iX, int iY) {

    char pszCoord[MAX_COORDINATE_LENGTH + 1];
    GameEngine::GetCoordinates(iX, iY, pszCoord);

    m_ppvNewPlanetData[iPlanetIndex][GameMap::iCoordinates] = pszCoord;
    Assert(m_ppvNewPlanetData[iPlanetIndex][GameMap::iCoordinates].GetCharPtr());
}

void BaseMapGenerator::AddLink(unsigned int iPlanetIndex, CardinalPoint cp) {

    AddLink(m_ppvNewPlanetData[iPlanetIndex], cp);
}

void BaseMapGenerator::AddLink(Variant* pvPlanetData, CardinalPoint cp) {

    int iLink = pvPlanetData[GameMap::iLink].GetInteger();
    Assert((iLink & LINK_X[cp]) == 0);
    pvPlanetData[GameMap::iLink] = iLink | LINK_X[cp];
}

int BaseMapGenerator::GetNewPlanetLocation(PlanetLocation* plLocation) {

    // Are we starting off the chain?
    if (m_iNumChainPlanetsCreated == 0) {

        if (m_iNumExistingPlanets == 0 && m_iNumNewPlanetsCreated == 0)
            return GetFirstPlanetLocation(plLocation);

        // Decide between starting the chain from an existing planet and from a newly created planet
        unsigned int iRand = Algorithm::GetRandomInteger(m_iNumExistingPlanets + m_iNumNewPlanetsCreated);

        if (iRand < m_iNumExistingPlanets)
            return GetNewPlanetLocationFromMap(plLocation);

        return GetNewPlanetLocationFromPreviouslyCreatedChain(plLocation);
    }

    return GetNewPlanetLocationFromCurrentChain(plLocation);
}

int BaseMapGenerator::GetFirstPlanetLocation(PlanetLocation* plLocation) {

    int iMaxNumEmpires = m_pvGameClassData[SystemGameClassData::iMaxNumEmpires].GetInteger();
    if (iMaxNumEmpires == UNLIMITED_EMPIRES) {

        int iErrCode, iUpdatesBeforeClose;
        unsigned int iNumEmpiresInGame;
        iErrCode = m_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpiresInGame);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = m_pGameEngine->GetNumUpdatesBeforeGameCloses (m_iGameClass, m_iGameNumber, &iUpdatesBeforeClose);
        RETURN_ON_ERROR(iErrCode);

        // TODO - this is a guess - I think we can live with negative coordinates...
        if (iUpdatesBeforeClose > 1) {
            iMaxNumEmpires = iNumEmpiresInGame * 2;
        } else {
            iMaxNumEmpires = iNumEmpiresInGame;
        }
    }

    //
    // Compute coordinates for the first planet on the map
    //

    int iMaxCoordinate = iMaxNumEmpires * m_iNumPlanetsPerEmpire * 2;
    int iMapDeviation = m_mcConfig.iMapDeviation;

    Assert(iMaxCoordinate > 0);

    plLocation->iX = Algorithm::GetRandomInteger (iMaxCoordinate) + iMapDeviation + Algorithm::GetRandomInteger (iMapDeviation);
    plLocation->iY = Algorithm::GetRandomInteger (iMaxCoordinate) + iMapDeviation + Algorithm::GetRandomInteger (iMapDeviation);
    plLocation->cpDirectionFromCoordinates = NO_DIRECTION;

    return OK;
}

int BaseMapGenerator::GetNewPlanetLocationFromPreviouslyCreatedChain (PlanetLocation* plLocation) {

    CardinalPoint pcpOpenDirections [NUM_CARDINAL_POINTS];

    unsigned int iNumPreviouslyCreatedPlanets = m_iNumNewPlanetsCreated - m_iNumChainPlanetsCreated;
    unsigned int iRand = Algorithm::GetRandomInteger (iNumPreviouslyCreatedPlanets);

    unsigned int iNumTried = 0;
    while (iNumTried < iNumPreviouslyCreatedPlanets) {

        Assert(iRand < iNumPreviouslyCreatedPlanets);

        unsigned int iNumOpenDirections = 0;

        int iX, iY, iProbeX, iProbeY;
        GetCoordinates(m_ppvNewPlanetData[iRand], &iX, &iY);

        AdvanceCoordinates(iX, iY, &iProbeX, &iProbeY, NORTH);
        if (!DoesPlanetExist(iProbeX, iProbeY)) {
            pcpOpenDirections [iNumOpenDirections ++] = NORTH;
        }

        AdvanceCoordinates(iX, iY, &iProbeX, &iProbeY, EAST);
        if (!DoesPlanetExist(iProbeX, iProbeY)) {
            pcpOpenDirections [iNumOpenDirections ++] = EAST;
        }

        AdvanceCoordinates(iX, iY, &iProbeX, &iProbeY, SOUTH);
        if (!DoesPlanetExist(iProbeX, iProbeY)) {
            pcpOpenDirections [iNumOpenDirections ++] = SOUTH;
        }

        AdvanceCoordinates(iX, iY, &iProbeX, &iProbeY, WEST);
        if (!DoesPlanetExist(iProbeX, iProbeY)) {
            pcpOpenDirections [iNumOpenDirections ++] = WEST;
        }

        if (iNumOpenDirections > 0) {

            // This planet will do...

            // Get coordinates
            plLocation->iX = iX;
            plLocation->iY = iY;

            // Choose a random open direction
            CardinalPoint cp = pcpOpenDirections[Algorithm::GetRandomInteger(iNumOpenDirections)];
            plLocation->cpDirectionFromCoordinates = cp;

            // Linked to a new planet, but in previous chain
            if (!(m_iGameClassOptions & DISCONNECTED_MAP)) {
                plLocation->AddLinkedPlanet(iRand, OPPOSITE_CARDINAL_POINT[cp]);
            }

            m_iLinkedPlanetInPreviousChainIndex = iRand;
            m_cpLinkedPlanetInPreviousChainDirection = cp;

            return OK;
        }

        iNumTried ++;
        iRand = (iRand + 1) % iNumPreviouslyCreatedPlanets;
    }

    // No open directions anywhere from the newly created planets. This can happen
    return ERROR_NO_NEW_PLANETS_AVAILABLE;
}

int BaseMapGenerator::GetNewPlanetLocationFromCurrentChain(PlanetLocation* plLocation) {

    CardinalPoint pcpOpenDirections [NUM_CARDINAL_POINTS];

    unsigned int iNumPreviouslyCreatedPlanets = m_iNumNewPlanetsCreated - m_iNumChainPlanetsCreated;
    unsigned int iRand;

    // Use last-created heuristic, to string planets out a bit
    int iLastCreatedChance = (int)m_iNumPlanetsPerEmpire < m_mcConfig.iLargeMapThreshold ? 
        m_mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap : 
        m_mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap;

    if (Algorithm::GetRandomInteger(100) < iLastCreatedChance) {
        iRand = m_iNumNewPlanetsCreated - 1;
    } else {
        iRand = iNumPreviouslyCreatedPlanets + Algorithm::GetRandomInteger(m_iNumChainPlanetsCreated);
    }

    unsigned int iNumTried = 0;
    while (iNumTried < m_iNumChainPlanetsCreated) {

        Assert(iRand < m_iNumNewPlanetsCreated);
        Assert(iRand >= m_iNumNewPlanetsCreated - m_iNumChainPlanetsCreated);

        unsigned int iNumOpenDirections = 0;

        int iX, iY, iProbeX, iProbeY;
        GetCoordinates(m_ppvNewPlanetData[iRand], &iX, &iY);

        AdvanceCoordinates(iX, iY, &iProbeX, &iProbeY, NORTH);
        if (!DoesPlanetExist(iProbeX, iProbeY)) {
            pcpOpenDirections [iNumOpenDirections ++] = NORTH;
        }

        AdvanceCoordinates(iX, iY, &iProbeX, &iProbeY, EAST);
        if (!DoesPlanetExist(iProbeX, iProbeY)) {
            pcpOpenDirections [iNumOpenDirections ++] = EAST;
        }

        AdvanceCoordinates(iX, iY, &iProbeX, &iProbeY, SOUTH);
        if (!DoesPlanetExist(iProbeX, iProbeY)) {
            pcpOpenDirections [iNumOpenDirections ++] = SOUTH;
        }

        AdvanceCoordinates(iX, iY, &iProbeX, &iProbeY, WEST);
        if (!DoesPlanetExist(iProbeX, iProbeY)) {
            pcpOpenDirections [iNumOpenDirections ++] = WEST;
        }

        if (iNumOpenDirections > 0) {

            // This planet will do...

            // Get coordinates
            plLocation->iX = iX;
            plLocation->iY = iY;

            // Choose a random open direction
            CardinalPoint cp = pcpOpenDirections[Algorithm::GetRandomInteger(iNumOpenDirections)];
            plLocation->cpDirectionFromCoordinates = cp;

            // Linked to a planet in the current chain
            plLocation->AddLinkedPlanet(iRand, OPPOSITE_CARDINAL_POINT[cp]);

            return OK;
        }

        iNumTried ++;
        iRand = iNumPreviouslyCreatedPlanets + ((iRand + 1) % m_iNumChainPlanetsCreated);
    }

    // No open directions anywhere from our current chain. This can happen
    return ERROR_NO_NEW_PLANETS_AVAILABLE;
}

int BaseMapGenerator::GetNewPlanetLocationFromMap(PlanetLocation* plLocation) {

    CardinalPoint pcpOpenDirections [NUM_CARDINAL_POINTS];
    unsigned int iRand = Algorithm::GetRandomInteger (m_iNumExistingPlanets);

    unsigned int iNumTried = 0;
    while (iNumTried < m_iNumExistingPlanets) {

        unsigned int iNumOpenDirections = 0;

        if (m_ppvExistingPlanetData[iRand][GameMap::iNorthPlanetKey] == NO_KEY) {
            pcpOpenDirections [iNumOpenDirections ++] = NORTH;
        }

        if (m_ppvExistingPlanetData[iRand][GameMap::iEastPlanetKey] == NO_KEY) {
            pcpOpenDirections [iNumOpenDirections ++] = EAST;
        }

        if (m_ppvExistingPlanetData[iRand][GameMap::iSouthPlanetKey] == NO_KEY) {
            pcpOpenDirections [iNumOpenDirections ++] = SOUTH;
        }

        if (m_ppvExistingPlanetData[iRand][GameMap::iWestPlanetKey] == NO_KEY) {
            pcpOpenDirections [iNumOpenDirections ++] = WEST;
        }

        if (iNumOpenDirections > 0) {

            // This planet will do...
            GetCoordinates(m_ppvExistingPlanetData[iRand], &plLocation->iX, &plLocation->iY);

            // Choose a random open direction
            CardinalPoint cp = pcpOpenDirections[Algorithm::GetRandomInteger(iNumOpenDirections)];
            plLocation->cpDirectionFromCoordinates = cp;

            // Linked to a planet on the existing map...
            if (!(m_iGameClassOptions & DISCONNECTED_MAP)) {
                plLocation->AddLinkedPlanet(NO_KEY, OPPOSITE_CARDINAL_POINT[cp]);
            }

			m_iExistingPlanetLinkedToChain = iRand;

            return OK;
        }

        iNumTried ++;
        iRand = (iRand + 1) % m_iNumExistingPlanets;
    }

    // No open directions anywhere on the existing map. This should never happen
    Assert(false);
    return ERROR_NO_NEW_PLANETS_AVAILABLE;
}

void BaseMapGenerator::AdvanceCoordinates (int iX, int iY, int* piX, int* piY, CardinalPoint cpDirection) {

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

bool BaseMapGenerator::DoesPlanetExist (int iX, int iY) {

    char pszCoord [MAX_COORDINATE_LENGTH + 1];
    GameEngine::GetCoordinates(iX, iY, pszCoord);

    return m_htCoordinates.FindFirst (pszCoord, (Variant**) NULL);
}

void BaseMapGenerator::InsertMapCoordinates()
{
    for (unsigned int i = 0; i < m_iNumExistingPlanets; i ++)
    {
        bool ret = m_htCoordinates.Insert (m_ppvExistingPlanetData[i][GameMap::iCoordinates].GetCharPtr(), m_ppvExistingPlanetData[i]);
        Assert(ret);
    }
}

Variant* BaseMapGenerator::FindPlanet(int iX, int iY) {

    char pszCoord [MAX_COORDINATE_LENGTH + 1];
    GameEngine::GetCoordinates(iX, iY, pszCoord);

    Variant* pvPlanetData;
    if (!m_htCoordinates.FindFirst(pszCoord, &pvPlanetData))
        return NULL;

    return pvPlanetData;
}

bool BaseMapGenerator::IsPlanetInCurrentChain(const Variant* pvPlanetData) {

    // This is a bit of a hack.  We could optimize it by adding some more metadata somewhere...
    for (unsigned int i = m_iNumNewPlanetsCreated - m_iNumChainPlanetsCreated; i < m_iNumNewPlanetsCreated; i ++)
    {
        if (m_ppvNewPlanetData[i] == pvPlanetData)
        {
            return true;
        }
    }

    return false;
}

void BaseMapGenerator::AllocatePlanetData(unsigned int iNumPlanets) {

    Assert(iNumPlanets > 0);

    m_iTotalNumNewPlanets = iNumPlanets;
    Variant* pvNewPlanetData = new Variant[iNumPlanets * GameData::NumColumns];
    Assert(pvNewPlanetData);

    Assert(m_ppvNewPlanetData == NULL);
    m_ppvNewPlanetData = new Variant*[iNumPlanets];
    Assert(m_ppvNewPlanetData);

    for (unsigned int i = 0; i < iNumPlanets; i ++)
    {
        m_ppvNewPlanetData[i] = pvNewPlanetData + i * GameData::NumColumns;
    }
}

void BaseMapGenerator::GetCoordinates(const Variant* pvPlanetData, int* piX, int* piY) {

    GameEngine::GetCoordinates(pvPlanetData[GameMap::iCoordinates].GetCharPtr(), piX, piY);
}

void BaseMapGenerator::CopyPlanetData(const Variant* pvSrcPlanetData, Variant* pvDestPlanetData) {

    for (unsigned int i = 0; i < GameMap::NumColumns; i ++)
    {
        pvDestPlanetData[i] = pvSrcPlanetData[i];
        if (pvSrcPlanetData[i].GetType() == V_STRING && pvSrcPlanetData[i].GetCharPtr())
        {
            Assert(pvDestPlanetData[i].GetCharPtr());
        }
    }
}