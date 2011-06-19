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

#include "Dijkstra.h"
#include "../GameEngine/GameEngine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Dijkstra::Dijkstra(const Variant** ppvPlanetData, unsigned int iNumPlanets)
    : 
    m_ppvPlanetData(ppvPlanetData),
    m_iNumPlanets(iNumPlanets),
    m_iInfinity(iNumPlanets * iNumPlanets), 
    m_piPrevious(NULL),
    m_piDistance(NULL),
    m_pbVisited(NULL),
    m_htCoordinates(NULL, NULL)
{
}

Dijkstra::~Dijkstra() {

    delete [] m_piDistance;
    m_piPrevious = NULL;

    delete [] m_pbVisited;
}

int Dijkstra::Run(unsigned int iSource, bool bTraverseHomeworlds) {

    int iErrCode = Initialize(iSource);
    if (iErrCode != OK)
        return iErrCode;

    unsigned int iCurrent = iSource;
    while(iCurrent != NO_KEY) {

        m_pbVisited[iCurrent] = true;

        unsigned int iCurrentDistance = m_piDistance[iCurrent] + 1;

        int cp;
        ENUMERATE_CARDINAL_POINTS(cp) {
        
            unsigned int iPlanet = GetPlanet(iCurrent, (CardinalPoint)cp);
            if (iPlanet != NO_KEY) {

                if (bTraverseHomeworlds || IsHomeWorld(iPlanet)) {

                    if (m_piDistance[iPlanet] > iCurrentDistance) {
                        m_piDistance[iPlanet] = iCurrentDistance;
                        m_piPrevious[iPlanet] = iCurrent;
                    }
                }
            }
        }

        iCurrent = GetClosestUnvisitedPlanet();
    }

    return OK;
}

unsigned int Dijkstra::GetShortestPathLength(unsigned int iDestination) {

    unsigned int iLength = m_piDistance[iDestination];
    if (iLength == m_iInfinity)
        return NO_PATH;

    return iLength;
}

unsigned int Dijkstra::GetClosestUnvisitedPlanet() {

    // TODO: optimize somehow?
    unsigned int iNext = NO_KEY;
    unsigned int iShortestDistance = m_iInfinity;

    for (unsigned int i = 0; i < m_iNumPlanets; i ++) {

        if (!m_pbVisited[i] && m_piDistance[i] < iShortestDistance) {
            iShortestDistance = m_piDistance[i];
            iNext = i;
        }
    }

    return iNext;
}

int Dijkstra::Initialize(unsigned int iSource) {

    if (m_piDistance == NULL) {

        m_piDistance = new unsigned int[m_iNumPlanets * 2];
        m_piPrevious = m_piDistance + m_iNumPlanets;

        m_pbVisited = new bool[m_iNumPlanets];

        bool bHt = m_htCoordinates.Initialize(m_iNumPlanets * 2); 

        if (m_piDistance == NULL || m_piPrevious == NULL || m_pbVisited == NULL || !bHt)
            return ERROR_OUT_OF_MEMORY;
    }

    for (unsigned int i = 0; i < m_iNumPlanets; i ++) {

        m_piDistance[i] = m_iInfinity;
        m_piPrevious[i] = NO_KEY;
        m_pbVisited[i] = false;

        const char* pszCoord = m_ppvPlanetData[i][GameMap::iCoordinates].GetCharPtr();
        if (!m_htCoordinates.Insert(pszCoord, i))
            return ERROR_OUT_OF_MEMORY;
    }

    m_piDistance[iSource] = 0;

    return OK;
}

inline unsigned int Dijkstra::GetPlanet(unsigned int iIndex, CardinalPoint cp) {

    // NB - GameMap::NorthPlanetKey may not be set yet, so can't use that

    const Variant* pvPlanetData = m_ppvPlanetData[iIndex];
    int iLink = pvPlanetData[GameMap::iLink].GetInteger();

    // Make sure there's a link
    if (!(iLink & LINK_X[cp]))
        return NO_KEY;

    int iX, iY;
    const char* pszCoord = pvPlanetData[GameMap::iCoordinates].GetCharPtr();
    GameEngine::GetCoordinates(pszCoord, &iX, &iY);
    BaseMapGenerator::AdvanceCoordinates(iX, iY, &iX, &iY, cp);

    char pszNextCoord [MAX_COORDINATE_LENGTH + 1];
    GameEngine::GetCoordinates(iX, iY, pszNextCoord);

    unsigned int iNextIndex;
    if (!m_htCoordinates.FindFirst(pszNextCoord, &iNextIndex))
        return NO_KEY;

    return iNextIndex;
}

inline bool Dijkstra::IsHomeWorld(unsigned int iIndex) {
    return m_ppvPlanetData[iIndex][GameMap::iHomeWorld].GetInteger() != HOMEWORLD;
}
