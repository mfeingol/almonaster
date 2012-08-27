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

#include "MirroredMapGenerator.h"

int MirroredMapGenerator::CreatePlanetChains() {

    // Initialize
    m_cpMirroredEdge = NO_DIRECTION;
    m_iEdgeCoordinate = 0;
    m_iNumEdgePlanets = 0;

    // The mirrored map generator algorithm works as follows:
    // 1) Create planet chains for half the empires
    // 2) Pick the edge with the most connectivity of the four
    // 3) Mirror the map on that edge, adding a layer of buffer planets

    AssertGameClassSettings();

    // Allocate memory for two halves of the map, not counting buffer planets
    AllocatePlanetData(m_iNumPlanetsPerEmpire * m_iNumNewEmpires);

    // Create half the map, one side of the mirror
    CreateHalfMap();

    // Pick an appropriate edge for the mirror
    ComputeBestMirroredEdge();

    // Set buffer planet depth correctly
    m_iBufferPlanetDepth = (m_iGameClassOptions & DISCONNECTED_MAP) ? 0 : 1;

    // Now that we know what we're getting, reallocate the map
    ReallocateMirroredMapIfNecessary();

    // Mirror the planets we just created
    CompleteMirror();

    return OK;
}

void MirroredMapGenerator::AssertGameClassSettings() {

    // Should have a fresh map
    Assert(m_iNumExistingPlanets == 0);

    // Should have an even number of empires
    Assert(m_iNumNewEmpires != 0 && m_iNumNewEmpires % 2 == 0);

    // Should have a fixed number of empires
    Assert(m_pvGameClassData[SystemGameClassData::iMinNumEmpires] == 
           m_pvGameClassData[SystemGameClassData::iMaxNumEmpires]);
}

int MirroredMapGenerator::CreateHalfMap() {

    int iErrCode = OK;

    const unsigned int iHalfNumEmpires = m_iNumNewEmpires / 2;
    for (unsigned int i = 0; i < iHalfNumEmpires; i ++)
    {
        iErrCode = CreatePlanetChain(m_piNewEmpireKey[i]);
        RETURN_ON_ERROR(iErrCode);
    }

    Assert(m_iNumNewPlanetsCreated * 2 == m_iTotalNumNewPlanets);

    return iErrCode;
}

void MirroredMapGenerator::ComputeBestMirroredEdge() {
 
    // The best edge is the one that has the most planets...

    int iX, iY;
    GetCoordinates(m_ppvNewPlanetData[0], &iX, &iY);

    int piEdgeCoord[NUM_CARDINAL_POINTS];
    piEdgeCoord[EAST] = piEdgeCoord[WEST] = iX;
    piEdgeCoord[NORTH] = piEdgeCoord[SOUTH] = iY;

    unsigned int piNumEdgePlanets[NUM_CARDINAL_POINTS];
    piNumEdgePlanets[NORTH] = piNumEdgePlanets[EAST] = piNumEdgePlanets[SOUTH] = piNumEdgePlanets[WEST] = 1;

    CardinalPoint pcpEdge[NUM_CARDINAL_POINTS];
    int cpDir;
    ENUMERATE_CARDINAL_POINTS (cpDir) {
        pcpEdge[cpDir] = (CardinalPoint)cpDir;
    }

    for (unsigned int i = 1; i < m_iNumNewPlanetsCreated; i ++) {

        GetCoordinates(m_ppvNewPlanetData[i], &iX, &iY);

        if (iX < piEdgeCoord[WEST]) {
            piEdgeCoord[WEST] = iX;
            piNumEdgePlanets[WEST] = 1;
        }
        else if (iX == piEdgeCoord[WEST]) {
            piNumEdgePlanets[WEST] ++;
        }

        if (iX > piEdgeCoord[EAST]) {
            piEdgeCoord[EAST] = iX;
            piNumEdgePlanets[EAST] = 1;
        }
        else if (iX == piEdgeCoord[EAST]) {
            piNumEdgePlanets[EAST] ++;
        }

        if (iY < piEdgeCoord[SOUTH]) {
            piEdgeCoord[SOUTH] = iY;
            piNumEdgePlanets[SOUTH] = 1;
        }
        else if (iY == piEdgeCoord[SOUTH]) {
            piNumEdgePlanets[SOUTH] ++;
        }

        if (iY > piEdgeCoord[NORTH]) {
            piEdgeCoord[NORTH] = iY;
            piNumEdgePlanets[NORTH] = 1;
        }
        else if (iY == piEdgeCoord[NORTH]) {
            piNumEdgePlanets[NORTH] ++;
        }
    }

    // By randomizing and sorting, we ensure a random edge pick if there's a draw
    Algorithm::RandomizeTwo<unsigned int, CardinalPoint>(piNumEdgePlanets, pcpEdge, NUM_CARDINAL_POINTS);
    Algorithm::QSortTwoDescending<unsigned int, CardinalPoint>(piNumEdgePlanets, pcpEdge, NUM_CARDINAL_POINTS);

    m_cpMirroredEdge = pcpEdge[0];
    m_iEdgeCoordinate = piEdgeCoord[m_cpMirroredEdge];
    m_iNumEdgePlanets = piNumEdgePlanets[0];

    // Note - because we have buffer planets, we don't truly care that two homeworlds might be facing
    // each other from across the mirror. Such is life.
}

void MirroredMapGenerator::CompleteMirror() {

    // Mirror the map
    const unsigned int iHalfNumPlanets = m_iNumNewPlanetsCreated;
    Assert(iHalfNumPlanets > 0);

    m_iNumNewPlanetsCreated *= 2;

    for (unsigned int iMapIndex = 0; iMapIndex < iHalfNumPlanets; iMapIndex ++) {

        const unsigned int iMirrorIndex = iHalfNumPlanets + iMapIndex;

        CopyPlanetData(m_ppvNewPlanetData[iMapIndex], m_ppvNewPlanetData[iMirrorIndex]);

        // Execute the transform
        int iMapX, iMapY;
        GetCoordinates(m_ppvNewPlanetData[iMapIndex], &iMapX, &iMapY);
        int iMapLink = m_ppvNewPlanetData[iMapIndex][GameMap::iLink].GetInteger();

        int iMirrorX, iMirrorY, iMirrorLink;
        MirrorPlanet(iMapX, iMapY, iMapLink, m_cpMirroredEdge, &iMirrorX, &iMirrorY, &iMirrorLink);

        // Set link flags
        m_ppvNewPlanetData[iMirrorIndex][GameMap::iLink] = iMirrorLink;

        // Set mirrored coordinates
        SetCoordinates(iMirrorIndex, iMirrorX, iMirrorY);

        // Add to lookup table
        InsertIntoCoordinatesTable(iMirrorIndex);

        // Make buffer planets if needed
        CreateBufferPlanetsIfNecessary(iMapIndex, iMirrorIndex, iMapX, iMapY, iMirrorX, iMirrorY);
    }

    AssignMirroredPlanetOwners();

    // Assign resources to buffer planets
    if (m_iBufferPlanetDepth > 0)
    {
        AssignResources(NO_KEY,
                        m_iNumPlanetsPerEmpire * m_iNumNewEmpires,
                        m_iNumEdgePlanets * m_iBufferPlanetDepth);
    }
}

void MirroredMapGenerator::CreateBufferPlanetsIfNecessary(int iOldIndex, int iMirrorIndex,
                                                          int iOldX, int iOldY, int iMirrorX, int iMirrorY) {

    if (m_iBufferPlanetDepth > 0) {

        Assert(!(m_iGameClassOptions & DISCONNECTED_MAP));

        if ((unsigned int)abs(iMirrorX - iOldX) == m_iBufferPlanetDepth + 1 || 
            (unsigned int)abs(iMirrorY - iOldY) == m_iBufferPlanetDepth + 1) {

            // Start at the old planet
            int iBufferX = iOldX, iBufferY = iOldY;
            for (unsigned int i = 0; i < m_iBufferPlanetDepth; i ++) {

                // Create planets from the first half towards the mirror
                PlanetLocation lLocation;
                lLocation.iX = iBufferX;
                lLocation.iY = iBufferY;
                lLocation.cpDirectionFromCoordinates = m_cpMirroredEdge;

                if (i == 0)
                    // Link to planet on original half of map
                    lLocation.AddLinkedPlanet(iOldIndex, OPPOSITE_CARDINAL_POINT[m_cpMirroredEdge]);
                else
                    // Link to previous buffer planet
                    lLocation.AddLinkedPlanet(m_iNumNewPlanetsCreated - 1, OPPOSITE_CARDINAL_POINT[m_cpMirroredEdge]);
                
                if (i == m_iBufferPlanetDepth - 1)
                    // Link to planet on mirrored half of map
                    lLocation.AddLinkedPlanet(iMirrorIndex, m_cpMirroredEdge);

                // Create a new buffer planet
                CreatePlanet(SYSTEM, &lLocation);
                AdvanceCoordinates(iBufferX, iBufferY, &iBufferX, &iBufferY, m_cpMirroredEdge);
            }
        }
    }
}

void MirroredMapGenerator::AssignMirroredPlanetOwners() {

    const unsigned int iHalfNumEmpires = m_iNumNewEmpires / 2;
    const unsigned int iHalfNumPlanets = iHalfNumEmpires * m_iNumPlanetsPerEmpire;

    // Assign remaining empires as owners of the second set of planet chains
    unsigned int iPlanetCount = 0;
    for (unsigned i = 0; i < iHalfNumEmpires; i ++)
    {
        for (unsigned j = 0; j < m_iNumPlanetsPerEmpire; j ++)
        {
            unsigned int iEmpireKey = m_piNewEmpireKey[iHalfNumEmpires + i];
            m_ppvNewPlanetData[iHalfNumPlanets + iPlanetCount][GameMap::iOwner] = iEmpireKey;
            iPlanetCount ++;
        }
    }
}

void MirroredMapGenerator::MirrorPlanet(int iOldX, int iOldY, int iOldLink, CardinalPoint cpEdge,
                                        int* piX, int* piY, int* piLink) {

    switch(cpEdge) {

    case NORTH:
        *piX = iOldX;
        *piY = 2 * m_iEdgeCoordinate - iOldY + m_iBufferPlanetDepth + 1;
        break;

    case EAST:
        *piX = 2 * m_iEdgeCoordinate - iOldX + m_iBufferPlanetDepth + 1;
        *piY = iOldY;
        break;

    case SOUTH:
        *piX = iOldX;
        *piY = 2 * m_iEdgeCoordinate - iOldY - m_iBufferPlanetDepth - 1;
        break;

    case WEST:
        *piX = 2 * m_iEdgeCoordinate - iOldX - m_iBufferPlanetDepth - 1;
        *piY = iOldY;
        break;

    default:
        Assert(false);
        break;
    }

    *piLink = MirrorLinks(iOldLink, cpEdge);
}

int MirroredMapGenerator::MirrorLinks(int iOldLink, CardinalPoint cpEdge) {

    int iLink;
    switch(cpEdge) {

    case NORTH:
    case SOUTH:

        iLink = iOldLink & (LINK_EAST | LINK_WEST);
        if (iOldLink & LINK_SOUTH) iLink |= LINK_NORTH;
        if (iOldLink & LINK_NORTH) iLink |= LINK_SOUTH;
        break;

    case WEST:
    case EAST:
        iLink = iOldLink & (LINK_NORTH | LINK_SOUTH);
        if (iOldLink & LINK_EAST) iLink |= LINK_WEST;
        if (iOldLink & LINK_WEST) iLink |= LINK_EAST;
        break;

    default:
        iLink = 0;
        Assert(false);
        break;
    }

    return iLink;
}

void MirroredMapGenerator::ReallocateMirroredMapIfNecessary()
{
    if (m_iBufferPlanetDepth > 0)
    {
        Assert(m_iNumEdgePlanets > 0);
        Assert(m_iNumNewPlanetsCreated == m_iNumPlanetsPerEmpire * m_iNumNewEmpires / 2);

        Variant** ppvPreviousNewPlanetData = m_ppvNewPlanetData;
        m_ppvNewPlanetData = NULL;

        unsigned int iNumBufferPlanets = m_iNumEdgePlanets * m_iBufferPlanetDepth;
        AllocatePlanetData(m_iNumPlanetsPerEmpire * m_iNumNewEmpires + iNumBufferPlanets);
        
        for (unsigned int i = 0; i < m_iNumNewPlanetsCreated; i ++)
        {
            CopyPlanetData(ppvPreviousNewPlanetData[i], m_ppvNewPlanetData[i]);
        }

        FreePlanetData(ppvPreviousNewPlanetData);
    }
}