// DefaultMapGenerator.cpp: implementation of the DefaultMapGenerator class.
//
//////////////////////////////////////////////////////////////////////

#include "DefaultMapGenerator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DefaultMapGenerator::DefaultMapGenerator (IGameEngine* pGameEngine) : m_htCoordinates (NULL, NULL) {

    m_iNumRefs = 1;

    Assert (pGameEngine != NULL);

    m_pGameEngine = pGameEngine;
    m_pGameEngine->AddRef();

    m_pDatabase = m_pGameEngine->GetDatabase(); // AddRef()

    Assert (m_pDatabase != NULL);

    m_iGameClass = NO_KEY;
    m_iGameNumber = 0;
    m_iEmpireKey = NO_KEY;
    
    m_ppvPlanetData = NULL;
    m_iNumPlanets = 0;

    m_pvGameClassData = NULL;
    
    m_ppvNewPlanetData = NULL;
    m_iNumNewPlanets = 0;

    m_iNumPlanetsCreated = 0;

    m_iHomeWorldIndex = NO_KEY;
}

DefaultMapGenerator::~DefaultMapGenerator() {

    if (m_pGameEngine != NULL) {
        m_pGameEngine->Release();
    }

    if (m_pDatabase != NULL) {
        m_pDatabase->Release();
    }
}

IMapGenerator* DefaultMapGenerator::CreateInstance (IGameEngine* pGameEngine) {

    return new DefaultMapGenerator (pGameEngine);
}

// IMapGenerator
int DefaultMapGenerator::CreatePlanets (

        int iGameClass,
        int iGameNumber,
        int iEmpireKey,

        Variant** ppvPlanetData,
        unsigned int iNumPlanets,

        Variant* pvGameClassData,
        Variant* pvGameData,

        Variant** ppvNewPlanetData,
        unsigned int iNumNewPlanets
        ) {

    int iErrCode;

    // Assign values to members
    m_iGameClass = iGameClass;
    m_iGameNumber = iGameNumber;
    m_iEmpireKey = iEmpireKey;
    
    m_ppvPlanetData = ppvPlanetData;
    m_iNumPlanets = iNumPlanets;

    m_pvGameClassData = pvGameClassData;
    m_pvGameData = pvGameData;

    m_ppvNewPlanetData = ppvNewPlanetData;
    m_iNumNewPlanets = iNumNewPlanets;

    m_iGameClassOptions = pvGameClassData[SystemGameClassData::Options].GetInteger();

    // Get configuration atomically
    iErrCode = m_pGameEngine->GetMapConfiguration (&m_mcConfig);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Init hashtable
    if (!m_htCoordinates.Initialize (
        m_iNumPlanets + m_iNumNewPlanets != 0 ? 
        m_iNumNewPlanets : m_pvGameClassData[SystemGameClassData::MaxNumPlanets].GetInteger())) {

        return ERROR_OUT_OF_MEMORY;
    }

    // First, handle case where no planets exist
    if (m_iNumPlanets == 0) {

        // Decide random numbers
        RollDiceForGameResources();

        // Create first planet
        if (!CreateFirstPlanet()) {
            return ERROR_OUT_OF_MEMORY;
        }

    } else {

        // Populate hash table with existing coordinates
        if (!InsertMapCoordinates()) {
            return ERROR_OUT_OF_MEMORY;
        }
    }

    // Go off and create the rest of the planets
    bool bLoop = true;

    while (bLoop) {

        iErrCode = CreatePlanets();
        switch (iErrCode) {

        case ERROR_NO_NEW_PLANETS_AVAILABLE:

            // Wipe slate clean
            m_iNumPlanetsCreated = 0;

            m_htCoordinates.Clear();

            if (m_iNumPlanets != 0) {

                if (!InsertMapCoordinates()) {
                    return ERROR_OUT_OF_MEMORY;
                }

            } else {

                // Recreate first planet on map
                if (!CreateFirstPlanet()) {
                    return ERROR_OUT_OF_MEMORY;
                }
            }
            break;

        default:

            // Anything else, we exit the loop
            bLoop = false;
            break;
        }
    }

    // Filter any errors we might have from creating planets
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Finish off the map
    ChooseHomeworld();
    AssignResources();
    AddNonDefaultLinks();

    return OK;
}

void DefaultMapGenerator::ChooseHomeworld() {

    ///////////////////////////////////////
    // Choose a homeworld for the empire //
    ///////////////////////////////////////

    int iRand = Algorithm::GetRandomInteger (m_iNumNewPlanets);

    // If chosen planet is first added, then make sure there isn't another HW next to it
    if (iRand == 0 && m_iNumPlanets > 0 && m_iNumNewPlanets > 1) {

        int iX, iY, iNewX, iNewY, cpDir;

        char pszCoord[MAX_COORDINATE_LENGTH + 1];
        Variant* pvPlanetData;

        m_pGameEngine->GetCoordinates (m_ppvNewPlanetData[0][GameMap::Coordinates].GetCharPtr(), &iX, &iY);

        // Check all directions for a planet
        ENUMERATE_CARDINAL_POINTS (cpDir) {

            AdvanceCoordinates (iX, iY, &iNewX, &iNewY, (CardinalPoint) cpDir);
            m_pGameEngine->GetCoordinates (iNewX, iNewY, pszCoord);

            if (m_htCoordinates.FindFirst (pszCoord, &pvPlanetData)) {

                Assert (pvPlanetData != NULL);

                // Make sure planet isn't a homeworld
                if (pvPlanetData[GameMap::HomeWorld].GetInteger() == HOMEWORLD) {

                    // We need to choose another planet that's not the first planet created
                    iRand = Algorithm::GetRandomInteger (m_iNumNewPlanets - 1) + 1;
                    break;
                }
            }
        }
    }

    m_ppvNewPlanetData[iRand][GameMap::HomeWorld] = HOMEWORLD;

    m_iHomeWorldIndex = iRand;
}

void DefaultMapGenerator::AddNonDefaultLinks() {

    //////////////////////////////////////////////
    // Add more links to newly inserted planets //
    //////////////////////////////////////////////

    int iLink, iX, iY, iNewX, iNewY, cpDir;

    char pszCoord[MAX_COORDINATE_LENGTH + 1];
    Variant* pvPlanetData;

    unsigned int i;
    for (i = 0; i < m_iNumNewPlanets; i ++) {

        iLink = m_ppvNewPlanetData[i][GameMap::Link].GetInteger();

        m_pGameEngine->GetCoordinates (m_ppvNewPlanetData[i][GameMap::Coordinates].GetCharPtr(), &iX, &iY);

        ENUMERATE_CARDINAL_POINTS (cpDir) {

            // Proceed if no link already exists
            if (!(iLink & LINK_X[cpDir])) {
            
                AdvanceCoordinates (iX, iY, &iNewX, &iNewY, (CardinalPoint) cpDir);
                m_pGameEngine->GetCoordinates (iNewX, iNewY, pszCoord);
                
                if (m_htCoordinates.FindFirst (pszCoord, &pvPlanetData)) {
                    
                    Assert (pvPlanetData != NULL);

                    // If disconnected maps, proceed if planet wasn't already on map
                    // We can find this out by seeing if the planet's name isn't blank
                    if (!(m_iGameClassOptions & DISCONNECTED_MAP) || 
                        pvPlanetData[GameMap::Name].GetType() != V_STRING ||
                        String::IsBlank (pvPlanetData[GameMap::Name].GetCharPtr())
                        ) {

                        // Throw 100 sided die
                        if (Algorithm::GetRandomInteger (100) < m_mcConfig.iChanceNewLinkForms) {
                            
                            // Create new link if at least one of the two planets isn't a HW's 
                            // or there's 1 planet per empire
                            
                            if (m_iNumNewPlanets == 1 || 
                                m_ppvNewPlanetData[i][GameMap::HomeWorld].GetInteger() != HOMEWORLD || 
                                pvPlanetData[GameMap::HomeWorld].GetInteger() != HOMEWORLD) {
                                
                                // Link 'em up!
                                iLink |= LINK_X[cpDir];
                                m_ppvNewPlanetData[i][GameMap::Link] = iLink;
                                
                                // If this is a planet in the map, no one will care about this change
                                // If it isn't, this has to be done
                                pvPlanetData[GameMap::Link] = pvPlanetData[GameMap::Link].GetInteger() | 
                                    OPPOSITE_LINK_X[cpDir];
                            }
                        }
                    }
                }
            }
        }
    }
}

void DefaultMapGenerator::AssignResources() {

    //////////////////////////////
    // Allocate resources to HW //
    //////////////////////////////

    Assert (m_iHomeWorldIndex != NO_KEY);

    m_ppvNewPlanetData[m_iHomeWorldIndex][GameMap::Ag] = m_pvGameData[GameData::HWAg].GetInteger();
    m_ppvNewPlanetData[m_iHomeWorldIndex][GameMap::Minerals] = m_pvGameData[GameData::HWMin].GetInteger();
    m_ppvNewPlanetData[m_iHomeWorldIndex][GameMap::Fuel] = m_pvGameData[GameData::HWFuel].GetInteger();


    ///////////////////////////////////////////
    // Allocate resources to rest of planets //
    ///////////////////////////////////////////

    unsigned int iAvgPlanetAg = m_pvGameData[GameData::AvgAg].GetInteger();
    unsigned int iAvgPlanetMin = m_pvGameData[GameData::AvgMin].GetInteger();
    unsigned int iAvgPlanetFuel = m_pvGameData[GameData::AvgFuel].GetInteger();

    unsigned int iNumNonHWPlanets = m_iNumNewPlanets - 1;

    unsigned int* piAg = (unsigned int*) StackAlloc (iNumNonHWPlanets * 3 * sizeof (unsigned int));
    unsigned int* piMin = piAg + iNumNonHWPlanets;
    unsigned int* piFuel = piMin + iNumNonHWPlanets;

    unsigned int iAgCap = (unsigned int) (m_mcConfig.fResourceAllocationRandomizationFactor * (float) iAvgPlanetAg);
    unsigned int iMinCap = (unsigned int) (m_mcConfig.fResourceAllocationRandomizationFactor * (float) iAvgPlanetMin);
    unsigned int iFuelCap = (unsigned int) (m_mcConfig.fResourceAllocationRandomizationFactor * (float) iAvgPlanetFuel);

    unsigned int iTotalAg = 0;
    unsigned int iTotalMin = 0;
    unsigned int iTotalFuel = 0;

    unsigned int i, iTemp;

    // Allocate resources
    for (i = 0; i < iNumNonHWPlanets; i ++) {

        iTemp = Algorithm::GetRandomInteger (iAgCap);
        piAg[i] = iTemp;
        iTotalAg += iTemp;

        iTemp = Algorithm::GetRandomInteger (iMinCap);
        piMin[i] = iTemp;
        iTotalMin += iTemp;

        iTemp = Algorithm::GetRandomInteger (iFuelCap);
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

        iTemp = (unsigned int) ((float) piAg[i] * fAgFactor);
        piAg[i] = iTemp;
        iTotalAg += iTemp;

        iTemp = (unsigned int) ((float) piMin[i] * fMinFactor);
        piMin[i] = iTemp;
        iTotalMin += iTemp;

        iTemp = (unsigned int) ((float) piFuel[i] * fFuelFactor);
        piFuel[i] = iTemp;
        iTotalFuel += iTemp;
    }

    // Compensate for floating point errors
    int iResDiff;
    
    iResDiff = iGlobalAg - iTotalAg;
    if (iResDiff != 0) {
        iTemp = Algorithm::GetRandomInteger (iNumNonHWPlanets);
        piAg[iTemp] += iResDiff;
    }
    
    iResDiff = iGlobalMin - iTotalMin;
    if (iResDiff != 0) {
        iTemp = Algorithm::GetRandomInteger (iNumNonHWPlanets);
        piMin[iTemp] += iResDiff;
    }
    
    iResDiff = iGlobalFuel - iTotalFuel;
    if (iResDiff != 0) {
        iTemp = Algorithm::GetRandomInteger (iNumNonHWPlanets);
        piFuel[iTemp] += iResDiff;
    }

    unsigned iCounter;
    
    // Save resources to planet data
    for (i = 0, iCounter = 0; i < m_iNumNewPlanets; i ++) {
        
        if (i != m_iHomeWorldIndex) {
            
            m_ppvNewPlanetData[i][GameMap::Ag] = piAg[iCounter];
            m_ppvNewPlanetData[i][GameMap::Minerals] = piMin[iCounter];
            m_ppvNewPlanetData[i][GameMap::Fuel] = piFuel[iCounter];

            iCounter ++;
        }
    }
}

void DefaultMapGenerator::RollDiceForGameResources() {

    int iMin, iMax;

    //
    // Number of planets per empire
    //

    iMin = m_pvGameClassData[SystemGameClassData::MinNumPlanets].GetInteger();
    iMax = m_pvGameClassData[SystemGameClassData::MaxNumPlanets].GetInteger();

    Assert (iMin <= iMax);

    // Save off result
    m_iNumNewPlanets = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);
    m_pvGameData[GameData::NumPlanetsPerEmpire] = m_iNumNewPlanets;

    //
    // Homeworld resources
    //
    
    // HWAg
    iMin = m_pvGameClassData[SystemGameClassData::MinAgHW].GetInteger();
    iMax = m_pvGameClassData[SystemGameClassData::MaxAgHW].GetInteger();

    Assert (iMin <= iMax);
    
    m_pvGameData[GameData::HWAg] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

    // HWMin
    iMin = m_pvGameClassData[SystemGameClassData::MinMinHW].GetInteger();
    iMax = m_pvGameClassData[SystemGameClassData::MaxMinHW].GetInteger();

    Assert (iMin <= iMax);

    m_pvGameData[GameData::HWMin] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

    // HWFuel
    iMin = m_pvGameClassData[SystemGameClassData::MinFuelHW].GetInteger();
    iMax = m_pvGameClassData[SystemGameClassData::MaxFuelHW].GetInteger();

    Assert (iMin <= iMax);

    m_pvGameData[GameData::HWFuel] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

    //
    // Planet resources
    //
    
    // Planet ag
    iMin = m_pvGameClassData[SystemGameClassData::MinAvgAg].GetInteger();
    iMax = m_pvGameClassData[SystemGameClassData::MaxAvgAg].GetInteger();

    Assert (iMin <= iMax);
    
    m_pvGameData[GameData::AvgAg] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

    // Planet min
    iMin = m_pvGameClassData[SystemGameClassData::MinAvgMin].GetInteger();
    iMax = m_pvGameClassData[SystemGameClassData::MaxAvgMin].GetInteger();

    Assert (iMin <= iMax);
    
    m_pvGameData[GameData::AvgMin] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);

    // Planet fuel
    iMin = m_pvGameClassData[SystemGameClassData::MinAvgFuel].GetInteger();
    iMax = m_pvGameClassData[SystemGameClassData::MaxAvgFuel].GetInteger();

    Assert (iMin <= iMax);

    m_pvGameData[GameData::AvgFuel] = iMin + Algorithm::GetRandomInteger (iMax - iMin + 1);
}

bool DefaultMapGenerator::CreateFirstPlanet() {

    //
    // Find a location for the first planet
    //

    int iMaxNumEmpires = m_pvGameClassData[SystemGameClassData::MaxNumEmpires].GetInteger();
    if (iMaxNumEmpires == UNLIMITED_EMPIRES) {

        int iErrCode, iUpdatesBeforeClose;

        iErrCode = m_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iMaxNumEmpires);
        if (iErrCode != OK) {
            Assert (false);
            return false;
        }

        iErrCode = m_pGameEngine->GetNumUpdatesBeforeGameCloses (m_iGameClass, m_iGameNumber, &iUpdatesBeforeClose);
        if (iErrCode != OK) {
            Assert (false);
            return false;
        }

        // Guess - we can live with negative coordinates anyway (I think!)
        if (iUpdatesBeforeClose > 1) {
            iMaxNumEmpires *= 2;
        }
    }

    int iMaxCoordinate = iMaxNumEmpires * m_iNumNewPlanets * 2;
    int iMapDeviation = m_mcConfig.iMapDeviation;

    Assert (iMaxCoordinate > 0);

    int iXRand = Algorithm::GetRandomInteger (iMaxCoordinate) + 
                 iMapDeviation + Algorithm::GetRandomInteger (iMapDeviation);

    int iYRand = Algorithm::GetRandomInteger (iMaxCoordinate) + 
                 iMapDeviation + Algorithm::GetRandomInteger (iMapDeviation);

    return CreatePlanet (NO_KEY, iXRand, iYRand, NO_DIRECTION);
}


bool DefaultMapGenerator::CreatePlanet (unsigned int iIndex, int iX, int iY, CardinalPoint cpDirection) {

    if (cpDirection != NO_DIRECTION) {

        AdvanceCoordinates (iX, iY, &iX, &iY, cpDirection);
        
        // Set links
        if (iIndex != NO_KEY) {
            
            int iLink = m_ppvNewPlanetData[iIndex][GameMap::Link].GetInteger();
            Assert ((iLink & LINK_X[cpDirection]) == 0);
            m_ppvNewPlanetData[iIndex][GameMap::Link] = iLink | LINK_X[cpDirection];
            m_ppvNewPlanetData[m_iNumPlanetsCreated][GameMap::Link] = OPPOSITE_LINK_X[cpDirection];

        } else {

            // iIndex is NO_KEY, so source planet was already on map
        
            // Only add this link if the map isn't disconnected
            if (!(m_iGameClassOptions & DISCONNECTED_MAP)) {
                m_ppvNewPlanetData[m_iNumPlanetsCreated][GameMap::Link] = OPPOSITE_LINK_X[cpDirection];
            } else {
                m_ppvNewPlanetData[m_iNumPlanetsCreated][GameMap::Link] = 0;
            }
        }

    } else {

        // No links
        m_ppvNewPlanetData[m_iNumPlanetsCreated][GameMap::Link] = 0;
    }

    char pszCoord[MAX_COORDINATE_LENGTH + 1];
    m_pGameEngine->GetCoordinates (iX, iY, pszCoord);

    // Sanity check - not in table already
    Assert (!m_htCoordinates.FindFirst (pszCoord, (Variant**) NULL));

    // Set default values
    m_ppvNewPlanetData[m_iNumPlanetsCreated][GameMap::Coordinates] = pszCoord;
    m_ppvNewPlanetData[m_iNumPlanetsCreated][GameMap::HomeWorld] = NOT_HOMEWORLD;

    // Insert into hash table
    bool bRetVal = m_htCoordinates.Insert (
        m_ppvNewPlanetData[m_iNumPlanetsCreated][GameMap::Coordinates].GetCharPtr(),
        m_ppvNewPlanetData[m_iNumPlanetsCreated]
        );

    m_iNumPlanetsCreated ++;

    return bRetVal;
}

int DefaultMapGenerator::CreatePlanets() {

    int iX, iY;
    unsigned int iIndex;

    CardinalPoint cpDirection;

    // If necessary, create a planet to start the chain
    if (m_iNumPlanets != 0) {

        // Get a random one - should never fail
        if (!ChoosePlanetAndDirectionFromMap (&iIndex, &cpDirection)) {
            Assert (false);
            return ERROR_NO_PLANETS_AVAILABLE;
        }

        m_pGameEngine->GetCoordinates (
            m_ppvPlanetData[iIndex][GameMap::Coordinates].GetCharPtr(),
            &iX,
            &iY
            );

        // Create new planet
        if (!CreatePlanet (NO_KEY, iX, iY, cpDirection)) {
            return ERROR_OUT_OF_MEMORY;
        }
    }

    // Create the rest of the planets
    while (m_iNumPlanetsCreated < m_iNumNewPlanets) {

        // Could easily fail
        if (!ChoosePlanetAndDirectionFromNewPlanets (&iIndex, &iX, &iY, &cpDirection)) {
            return ERROR_NO_NEW_PLANETS_AVAILABLE;
        }

        // Create new planet
        if (!CreatePlanet (iIndex, iX, iY, cpDirection)) {
            return ERROR_OUT_OF_MEMORY;
        }
    }

    return OK;
}

bool DefaultMapGenerator::ChoosePlanetAndDirectionFromMap (unsigned int* piIndex, 
                                                           CardinalPoint* pcpDirection) {

    unsigned int iNumTried = 0, iRand = Algorithm::GetRandomInteger (m_iNumPlanets), iNumOpenDirections = 0;

    CardinalPoint pcpOpenDirections [NUM_CARDINAL_POINTS];

    while (iNumTried < m_iNumPlanets) {

        if (m_ppvPlanetData[iRand][GameMap::NorthPlanetKey] == NO_KEY) {
            pcpOpenDirections [iNumOpenDirections ++] = NORTH;
        }

        if (m_ppvPlanetData[iRand][GameMap::EastPlanetKey] == NO_KEY) {
            pcpOpenDirections [iNumOpenDirections ++] = EAST;
        }

        if (m_ppvPlanetData[iRand][GameMap::SouthPlanetKey] == NO_KEY) {
            pcpOpenDirections [iNumOpenDirections ++] = SOUTH;
        }

        if (m_ppvPlanetData[iRand][GameMap::WestPlanetKey] == NO_KEY) {
            pcpOpenDirections [iNumOpenDirections ++] = WEST;
        }

        if (iNumOpenDirections > 0) {

            // This planet will do
            *piIndex = iRand;

            // Choose a random open direction
            *pcpDirection = pcpOpenDirections[Algorithm::GetRandomInteger (iNumOpenDirections)];

            return true;
        }

        iNumTried ++;
        iRand = (iRand + 1) % m_iNumPlanets;
    }

    // No open directions anywhere. This should never really happen
    Assert (false);
    return false;
}


bool DefaultMapGenerator::ChoosePlanetAndDirectionFromNewPlanets (unsigned int* piIndex, 
                                                                  int* piX, int* piY,
                                                                  CardinalPoint* pcpDirection) {

    const char* pszCoord;
    unsigned int iNumTried = 0, iNumOpenDirections = 0;
    
    int iX, iY, iRand;

    CardinalPoint pcpOpenDirections [NUM_CARDINAL_POINTS];

    // Use last-created heuristic, to string planets out a bit
    iRand = Algorithm::GetRandomInteger (100);

    if (iRand < ((int) m_iNumNewPlanets < m_mcConfig.iLargeMapThreshold ? 
            m_mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap : 
            m_mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap)) {

        Assert (m_iNumPlanetsCreated > 0);

        // Pick the last planet created
        iRand = m_iNumPlanetsCreated - 1;

    } else {

        // Just pick a random planet
        iRand = Algorithm::GetRandomInteger (m_iNumPlanetsCreated);
    }

    while (iNumTried < m_iNumPlanetsCreated) {

        pszCoord = m_ppvNewPlanetData[iRand][GameMap::Coordinates].GetCharPtr();

        m_pGameEngine->GetCoordinates (pszCoord, &iX, &iY);

        if (!DoesPlanetExist (iX, iY + 1)) {
            pcpOpenDirections [iNumOpenDirections ++] = NORTH;
        }

        if (!DoesPlanetExist (iX + 1, iY)) {
            pcpOpenDirections [iNumOpenDirections ++] = EAST;
        }

        if (!DoesPlanetExist (iX, iY - 1)) {
            pcpOpenDirections [iNumOpenDirections ++] = SOUTH;
        }

        if (!DoesPlanetExist (iX - 1, iY)) {
            pcpOpenDirections [iNumOpenDirections ++] = WEST;
        }

        if (iNumOpenDirections > 0) {

            // This planet will do
            *piIndex = iRand;

            // Choose a random open direction
            *pcpDirection = pcpOpenDirections[Algorithm::GetRandomInteger (iNumOpenDirections)];

            *piX = iX;
            *piY = iY;

            return true;
        }

        iNumTried ++;
        iRand = (iRand + 1) % m_iNumPlanetsCreated;
    }

    // No open directions anywhere. This can happen
    return false;
}

void DefaultMapGenerator::AdvanceCoordinates (int iX, int iY, int* piX, int* piY, CardinalPoint cpDirection) {

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

bool DefaultMapGenerator::DoesPlanetExist (int iX, int iY) {

    char pszCoord [MAX_COORDINATE_LENGTH + 1];
    m_pGameEngine->GetCoordinates (iX, iY, pszCoord);

    return m_htCoordinates.FindFirst (pszCoord, (Variant**) NULL);
}

bool DefaultMapGenerator::InsertMapCoordinates() {

    unsigned int i;
    
    for (i = 0; i < m_iNumPlanets; i ++) {
        
        if (!m_htCoordinates.Insert (m_ppvPlanetData[i][GameMap::Coordinates].GetCharPtr(), m_ppvPlanetData[i])) {
            return false;
        }
    }

    return true;
}
