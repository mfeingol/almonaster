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

#include "TwistedMapGenerator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TwistedMapGenerator::TwistedMapGenerator(IGameEngine* pGameEngine) 
    :
    MirroredMapGenerator(pGameEngine) {

    memset(m_ed, 0, sizeof(m_ed));
}

TwistedMapGenerator::~TwistedMapGenerator() {

    int cp;
    ENUMERATE_CARDINAL_POINTS(cp) {
        delete [] m_ed[cp].pcEdgePlanet;
    }
}

IMapGenerator* TwistedMapGenerator::CreateInstance(IGameEngine* pGameEngine) {

    return new TwistedMapGenerator(pGameEngine);
}

int TwistedMapGenerator::CreatePlanetChains() {

    // Initialize
    memset(m_ed, 0, sizeof(m_ed));

    m_iBestJoinNumConnections = 0;
    m_cpChosenEdge = NO_DIRECTION;

    // The twisted generator algorithm works as follows:
    // 1) Create planet chains for half the empires
    // 2) Create a copy of the half-map and rotate it 180 degrees
    // 3) Find the location for the second half-map that maximizes connectivity, without
    // joining a planet with itself

    int iErrCode = OK;

    AssertGameClassSettings();

    // Allocate memory for the entire map
    if (!AllocatePlanetData(m_iNumPlanetsPerEmpire * m_iNumNewEmpires))
        return ERROR_OUT_OF_MEMORY;

    // Create half the map, one side of the mirror
    CreateHalfMap();

    // Build data structures that represent the surface of each edge
    // Planets inside those surfaces don't matter for the purpose of matching
    iErrCode = BuildEdgeDescriptions();
    if (iErrCode != OK)
        return iErrCode;

    // Run the numbers and find a good join
    ComputeBestTwistedJoin();

    // Complete the twist
    CompleteTwistedMap();

    return iErrCode;
}

int TwistedMapGenerator::CompleteTwistedMap() {

    int iErrCode = OK;

    const unsigned int iHalfNumPlanets = m_iNumNewPlanetsCreated;
    Assert(iHalfNumPlanets > 0);
    Assert(m_cpChosenEdge != NO_DIRECTION);

    m_iNumNewPlanetsCreated *= 2;

    for (unsigned int iMapIndex = 0; iMapIndex < iHalfNumPlanets; iMapIndex ++) {

        const unsigned int iTwistIndex = iHalfNumPlanets + iMapIndex;

        iErrCode = CopyPlanetData(m_ppvNewPlanetData[iMapIndex], m_ppvNewPlanetData[iTwistIndex]);
        if (iErrCode != OK)
            return iErrCode;

        // Execute the transform
        int iMapX, iMapY;
        GetCoordinates(m_ppvNewPlanetData[iMapIndex], &iMapX, &iMapY);
        int iMapLink = m_ppvNewPlanetData[iMapIndex][GameMap::iLink].GetInteger();

        int iTwistX, iTwistY, iTwistLink;
        TwistCoordinates(iMapX, iMapY, m_cpChosenEdge, m_iChosenXOffset, m_iChosenYOffset, &iTwistX, &iTwistY);
        TwistLinks(iMapLink, m_cpChosenEdge, &iTwistLink);

        // Set link flags
        m_ppvNewPlanetData[iTwistIndex][GameMap::iLink] = iTwistLink;

        // Set mirrored coordinates
        iErrCode = SetCoordinates(iTwistIndex, iTwistX, iTwistY);
        if (iErrCode != OK)
            return iErrCode;

        // Add to lookup table
        iErrCode = InsertIntoCoordinatesTable(iTwistIndex);
        if (iErrCode != OK)
            return iErrCode;
    }

    AssignMirroredPlanetOwners();
    CreateLinksBetweenTwists();

    return iErrCode;
}

int TwistedMapGenerator::BuildEdgeDescriptions() {

    GetCoordinates(m_ppvNewPlanetData[0], m_piExtreme + EAST, m_piExtreme + NORTH);
    m_piExtreme[WEST] = m_piExtreme[EAST];
    m_piExtreme[SOUTH] = m_piExtreme[NORTH];

    // Find max dimensions of map
    unsigned int iHalfNumPlanets = m_iNumNewPlanetsCreated;
    for (unsigned int i = 1; i < iHalfNumPlanets; i ++) {

        int iX, iY;
        GetCoordinates(m_ppvNewPlanetData[i], &iX, &iY);

        if (iX < m_piExtreme[WEST])
            m_piExtreme[WEST] = iX;
        else if (iX > m_piExtreme[EAST])
            m_piExtreme[EAST] = iX;

        if (iY < m_piExtreme[SOUTH])
            m_piExtreme[SOUTH] = iY;
        else if (iY > m_piExtreme[NORTH])
            m_piExtreme[NORTH] = iY;
    }

    // Allocate memory for edge descriptions
    m_ed[NORTH].iLength = m_ed[SOUTH].iLength = m_piExtreme[EAST] - m_piExtreme[WEST] + 1;
    m_ed[EAST].iLength = m_ed[WEST].iLength = m_piExtreme[NORTH] - m_piExtreme[SOUTH] + 1;

    int cp;
    ENUMERATE_CARDINAL_POINTS(cp) {
        m_ed[cp].pcEdgePlanet = new PlanetCoordinates[m_ed[cp].iLength];
        if (m_ed[cp].pcEdgePlanet == NULL)
            return ERROR_OUT_OF_MEMORY;
    }

    // Initialize descriptions
    for (int i = 0; i < m_ed[NORTH].iLength; i ++) {
        m_ed[NORTH].pcEdgePlanet[i].iX = m_piExtreme[WEST] + i;
        m_ed[NORTH].pcEdgePlanet[i].iY = m_piExtreme[SOUTH] - 1;
        m_ed[NORTH].pcEdgePlanet[i].bHW = false;

        m_ed[SOUTH].pcEdgePlanet[i].iX = m_piExtreme[WEST] + i;
        m_ed[SOUTH].pcEdgePlanet[i].iY = m_piExtreme[NORTH] + 1;
        m_ed[SOUTH].pcEdgePlanet[i].bHW = false;
    }

    for (int i = 0; i < m_ed[EAST].iLength; i ++) {
        m_ed[EAST].pcEdgePlanet[i].iX = m_piExtreme[WEST] - 1;
        m_ed[EAST].pcEdgePlanet[i].iY = m_piExtreme[SOUTH] + i;
        m_ed[EAST].pcEdgePlanet[i].bHW = false;

        m_ed[WEST].pcEdgePlanet[i].iX = m_piExtreme[EAST] + 1;
        m_ed[WEST].pcEdgePlanet[i].iY = m_piExtreme[SOUTH] + i;
        m_ed[WEST].pcEdgePlanet[i].bHW = false;
    }

    // Calculate the surface for each edge
    for (unsigned int i = 0; i < iHalfNumPlanets; i ++) {

        int iX, iY;
        GetCoordinates(m_ppvNewPlanetData[i], &iX, &iY);

        bool bHW = m_ppvNewPlanetData[i][GameMap::iHomeWorld].GetInteger() == HOMEWORLD;

        // North, south
        int iNSIndex = iX - m_piExtreme[WEST];
        Assert(iNSIndex >= 0 && iNSIndex <= m_piExtreme[EAST] - m_piExtreme[WEST]);
        Assert(m_ed[NORTH].pcEdgePlanet[iNSIndex].iX == m_ed[SOUTH].pcEdgePlanet[iNSIndex].iX);
        Assert(m_ed[NORTH].pcEdgePlanet[iNSIndex].iX == iX);

        if (iY > m_ed[NORTH].pcEdgePlanet[iNSIndex].iY) {
            m_ed[NORTH].pcEdgePlanet[iNSIndex].iY = iY;
            m_ed[NORTH].pcEdgePlanet[iNSIndex].bHW = bHW;
        }

        if (iY < m_ed[SOUTH].pcEdgePlanet[iNSIndex].iY) {
            m_ed[SOUTH].pcEdgePlanet[iNSIndex].iY = iY;
            m_ed[SOUTH].pcEdgePlanet[iNSIndex].bHW = bHW;
        }

        // East, west
        int iEWIndex = iY - m_piExtreme[SOUTH];
        Assert(iEWIndex >= 0 && iEWIndex <= m_piExtreme[NORTH] - m_piExtreme[SOUTH]);
        Assert(m_ed[EAST].pcEdgePlanet[iEWIndex].iY == m_ed[WEST].pcEdgePlanet[iEWIndex].iY);
        Assert(m_ed[EAST].pcEdgePlanet[iEWIndex].iY == iY);

        if (iX > m_ed[EAST].pcEdgePlanet[iEWIndex].iX) {
            m_ed[EAST].pcEdgePlanet[iEWIndex].iX = iX;
            m_ed[EAST].pcEdgePlanet[iEWIndex].bHW = bHW;
        }

        if (iX < m_ed[WEST].pcEdgePlanet[iEWIndex].iX) {
            m_ed[WEST].pcEdgePlanet[iEWIndex].iX = iX;
            m_ed[WEST].pcEdgePlanet[iEWIndex].bHW = bHW;
        }
    }

    return OK;
}

void TwistedMapGenerator::ComputeBestTwistedJoin() {

    ComputeBestTwistedJoinNorthSouth(NORTH);
    ComputeBestTwistedJoinNorthSouth(SOUTH);
    ComputeBestTwistedJoinEastWest(EAST);
    ComputeBestTwistedJoinEastWest(WEST);

    // Pick one of the best joins at random
    int iNumJoins = m_llBestJoins.GetNumElements();
    Assert(iNumJoins > 0);

    int i = 0, iRand = Algorithm::GetRandomInteger(iNumJoins);
    ListIterator<BestJoin> li;
    while(true)
    {
        bool bRet = m_llBestJoins.PopFirst(&li);
        Assert(bRet);
        if (i ++ == iRand)
            break;
    }

    const BestJoin& bestJoin = li.GetData();
    m_cpChosenEdge = bestJoin.cpJoin;
    m_iChosenXOffset = bestJoin.xOffset;
    m_iChosenYOffset = bestJoin.yOffset;
}

void TwistedMapGenerator::ComputeBestTwistedJoinNorthSouth(CardinalPoint cpEdge) {

    Assert(cpEdge == NORTH || cpEdge == SOUTH);
    const int iCoordinateIncrement = cpEdge == NORTH ? 1 : -1;

    // For each possible xOffset...
    for (int xOffset = -m_ed[cpEdge].iLength; xOffset < m_ed[cpEdge].iLength; xOffset ++) {

        // An x and y offset combination represent a join attempt
        // For each x offset, there will be just one matching y offset that makes a valid join
        for (int yOffset = iCoordinateIncrement - (iCoordinateIncrement * m_ed[WEST].iLength); 
             ; 
             yOffset += iCoordinateIncrement) {

            // For each planet on the edge...
            unsigned int iConnections = 0;
            bool bCollision = false, bUnwantedJoin = false;
            for (int i = 0; i < m_ed[cpEdge].iLength; i ++) {

                // Obtain the planet's twisted coordinates according to this join attempt
                const PlanetCoordinates& cPlanet = m_ed[cpEdge].pcEdgePlanet[i];
                int iTwistedX, iTwistedY;
                TwistCoordinates(cPlanet.iX,
                                 cPlanet.iY,
                                 cpEdge,
                                 xOffset,
                                 yOffset,
                                 &iTwistedX,
                                 &iTwistedY);

                const int iEdgeIndex = iTwistedX - m_piExtreme[WEST];
                if (iEdgeIndex >= 0 && iEdgeIndex < m_ed[cpEdge].iLength) {

                    const PlanetCoordinates& pcConnection = m_ed[cpEdge].pcEdgePlanet[iEdgeIndex];

                    // The twisted planet falls within the boundary of interest.  Three possible outcomes:
                    // 1) A collision with an existing planet
                    // 2) A connection with a neighboring planet on the map
                    // 3) No connection

                    int iDiff = (iTwistedY - pcConnection.iY) * iCoordinateIncrement;
                    if (iDiff <= 0) {
                        // A collision
                        // Stop looking for other planets with connections
                        bCollision = true;
                        break;
                    }

                    if (iDiff == 1) {

                        // Planets MUST not be connected with their twisted equivalents
                        if (iEdgeIndex == i) {
                            bUnwantedJoin = true;
                            break;
                        }

                        // Homeworlds MUST not be connected together
                        if (cPlanet.bHW && pcConnection.bHW) {
                            bUnwantedJoin = true;
                            break;
                        }

                        // A valid connection
                        iConnections ++;
                    }

                    // No connection at all. Keep looking for other planets with connections
                }
            }   // End for each edge planet in the given join attempt

            if (bCollision)
                // Doom the join attempt and move on to the next one in the sequence
                continue;

            // If the join attempt isn't okay, ignore the connections we found
            if (!bUnwantedJoin && iConnections > 0) {

                // The join attempt has at least one connection.
                // But is it one of the best joins we've seen so far?
                if (iConnections >= m_iBestJoinNumConnections) {

                    // It might be a new winner
                    if (iConnections > m_iBestJoinNumConnections) {
                        m_llBestJoins.Clear();
                        m_iBestJoinNumConnections = iConnections;
                    }

                    // Regardless, add it to the list
                    BestJoin bj = { cpEdge, xOffset, yOffset };
                    m_llBestJoins.PushFirst(bj);
                }
            }

            // Once we've found a non-colliding join attempt, we move on to the next one.
            // We've either found the one y offset that works, or there is none to begin with.
            // Either way, we are now too far away for a valid join to occur as we continue to 
            // move away from the map
            break;
        
        }   // End y offset loop

    }   // End x offset loop
}

void TwistedMapGenerator::ComputeBestTwistedJoinEastWest(CardinalPoint cpEdge) {

    Assert(cpEdge == EAST || cpEdge == WEST);
    const int iCoordinateIncrement = cpEdge == EAST ? 1 : -1;

    // For each possible yOffset...
    for (int yOffset = -m_ed[cpEdge].iLength; yOffset < m_ed[cpEdge].iLength; yOffset ++) {

        // An x and y offset combination represent a join attempt
        // For each y offset, there will be just one matching x offset that makes a valid join
        for (int xOffset = iCoordinateIncrement - (iCoordinateIncrement * m_ed[SOUTH].iLength); 
             ; 
             xOffset += iCoordinateIncrement) {

            // For each planet on the edge...
            unsigned int iConnections = 0;
            bool bCollision = false, bUnwantedJoin = false;
            for (int i = 0; i < m_ed[cpEdge].iLength; i ++) {

                // Obtain the planet's twisted coordinates according to this join attempt
                const PlanetCoordinates& cPlanet = m_ed[cpEdge].pcEdgePlanet[i];
                int iTwistedX, iTwistedY;
                TwistCoordinates(cPlanet.iX,
                                 cPlanet.iY,
                                 cpEdge,
                                 xOffset,
                                 yOffset,
                                 &iTwistedX,
                                 &iTwistedY);

                const int iEdgeIndex = iTwistedY - m_piExtreme[SOUTH];
                if (iEdgeIndex >= 0 && iEdgeIndex < m_ed[cpEdge].iLength) {

                    const PlanetCoordinates& pcConnection = m_ed[cpEdge].pcEdgePlanet[iEdgeIndex];

                    // The twisted planet falls within the boundary of interest.  Three possible outcomes:
                    // 1) A collision with an existing planet
                    // 2) A connection with a neighboring planet on the map
                    // 3) No connection

                    int iDiff = (iTwistedX - pcConnection.iX) * iCoordinateIncrement;
                    if (iDiff <= 0) {
                        // A collision - stop looking for other planets with connections
                        bCollision = true;
                        break;
                    }

                    if (iDiff == 1) {

                        // Planets MUST not be connected with their twisted equivalents
                        if (iEdgeIndex == i) {
                            bUnwantedJoin = true;
                            break;
                        }

                        // Homeworlds MUST not be connected together
                        if (cPlanet.bHW && pcConnection.bHW) {
                            bUnwantedJoin = true;
                            break;
                        }

                        // A valid connection
                        iConnections ++;
                    }

                    // No connection at all. Keep looking for other planets with connections
                }
            }   // End for each edge planet in the given join attempt

            if (bCollision)
                // Doom the join attempt and move on to the next one in the sequence
                continue;

            // If the join attempt isn't okay, ignore the connections we found
            if (!bUnwantedJoin && iConnections > 0) {

                // The join attempt has at least one connection.
                // But is it one of the best joins we've seen so far?
                if (iConnections >= m_iBestJoinNumConnections) {

                    // It might be a new winner
                    if (iConnections > m_iBestJoinNumConnections) {
                        m_llBestJoins.Clear();
                        m_iBestJoinNumConnections = iConnections;
                    }

                    // Regardless, add it to the list
                    BestJoin bj = { cpEdge, xOffset, yOffset };
                    m_llBestJoins.PushFirst(bj);
                }
            }

            // Once we've found a non-colliding join attempt, we move on to the next one.
            // We've either found the one y offset that works, or there is none to begin with.
            // Either way, we are now too far away for a valid join to occur as we continue to 
            // move away from the map
            break;
        
        }   // End x offset loop

    }   // End y offset loop
}

void TwistedMapGenerator::TwistCoordinates(int iX, int iY,
                                           CardinalPoint cpEdge,
                                           int xOffset, int yOffset,
                                           int* piTwistedX, int* piTwistedY) {

    switch (cpEdge) {

        case NORTH:
            // tx = xmin - (x - xmax) = xmin + xmax - x
            *piTwistedX = m_piExtreme[WEST] + m_piExtreme[EAST] - iX + xOffset;
            // ty = ymax - (y - ymax) = 2 * ymax - y
            *piTwistedY = 2 * m_piExtreme[NORTH] - iY + yOffset;
            break;

        case EAST:
            // tx = xmax - (x - xmax) = 2 * xmax - x
            *piTwistedX = 2 * m_piExtreme[EAST]  - iX + xOffset;
            // ty = ymin - (y - ymax) = ymin + ymax - y
            *piTwistedY = m_piExtreme[SOUTH] + m_piExtreme[NORTH] - iY + yOffset;
            break;

        case SOUTH:
            // tx = xmin - (x - xmax) = xmin + xmax - x
            *piTwistedX = m_piExtreme[WEST] + m_piExtreme[EAST] - iX + xOffset;
            // ty = ymin - (y - ymin) = 2 * ymin - y
            *piTwistedY = 2 * m_piExtreme[SOUTH] - iY + yOffset;
            break;

        case WEST:
            // tx = xmin - (x - xmin) = 2 * xmin - x
            *piTwistedX = 2 * m_piExtreme[WEST]  - iX + xOffset;
            // ty = ymin - (y - ymax) = ymin + ymax - y
            *piTwistedY = m_piExtreme[SOUTH] + m_piExtreme[NORTH] - iY + yOffset;
            break;

        default:
            Assert(false);
            break;
    }
}

void TwistedMapGenerator::TwistLinks(int iLink, CardinalPoint cpEdge, int* piTwistedLink) {

    int iNewLink = 0;

    if (iLink & LINK_NORTH) iNewLink |= LINK_SOUTH;
    if (iLink & LINK_EAST) iNewLink |= LINK_WEST;
    if (iLink & LINK_SOUTH) iNewLink |= LINK_NORTH;
    if (iLink & LINK_WEST) iNewLink |= LINK_EAST;

    *piTwistedLink = iNewLink;
}

void TwistedMapGenerator::CreateLinksBetweenTwists() {

    if (!(m_iGameClassOptions & DISCONNECTED_MAP)) {

        Assert(m_cpChosenEdge != NO_DIRECTION);

        const EdgeDescription& ed = m_ed[m_cpChosenEdge];

        for (int i = 0; i < ed.iLength; i ++) {

            const PlanetCoordinates& pc = ed.pcEdgePlanet[i];

            int iNeighborX, iNeighborY;
            AdvanceCoordinates(pc.iX, pc.iY, &iNeighborX, &iNeighborY, m_cpChosenEdge);

            // Find the twisted planet
            Variant* pvTwistedPlanetData = FindPlanet(iNeighborX, iNeighborY);
            if (pvTwistedPlanetData != NULL) {

                Variant* pvMapPlanetData = FindPlanet(pc.iX, pc.iY);
                Assert(pvMapPlanetData != NULL);

                AddLink(pvMapPlanetData, m_cpChosenEdge);
                AddLink(pvTwistedPlanetData, OPPOSITE_CARDINAL_POINT[m_cpChosenEdge]);
            }
        }
    }
}