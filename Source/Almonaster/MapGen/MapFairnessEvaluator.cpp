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

#include <math.h>
#include "MapFairnessEvaluator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MapFairnessEvaluator::MapFairnessEvaluator(const Variant** ppvPlanetData, unsigned int iNumPlanets, 
                                           const unsigned int* piEmpireKey, unsigned int iNumEmpires)
    : 
    m_ppvPlanetData(ppvPlanetData),
    m_iNumPlanets(iNumPlanets),
    m_piEmpireKey(piEmpireKey),
    m_iNumEmpires(iNumEmpires),
    m_dijkstra(ppvPlanetData, iNumPlanets),
    m_pllClaims(NULL),
    m_piEmpireResources(NULL)
{
}

MapFairnessEvaluator::~MapFairnessEvaluator() {

    delete [] m_pllClaims;
    delete [] m_piEmpireResources;
}

void MapFairnessEvaluator::Run() {

    Initialize();
    
    for (unsigned int i = 0; i < m_iNumEmpires; i ++)
    {
        EvaluateEmpireClaims(i);
    }

    ProcessClaims();
}

unsigned int MapFairnessEvaluator::GetResourceClaim(unsigned int iEmpireKey) {

    for (unsigned int i = 0; i < m_iNumEmpires; i ++) {

        if (m_piEmpireKey[i] == iEmpireKey)
        {
            return m_piEmpireResources[i];
        }
    }

    Assert(!"Should never get here");
    return 0;
}

void MapFairnessEvaluator::Initialize() {

    m_pllClaims = new LinkedList<PlanetClaim>[m_iNumPlanets];
    m_piEmpireResources = new unsigned int[m_iNumEmpires];
    Assert(m_pllClaims && m_piEmpireResources);
    memset(m_piEmpireResources, 0, m_iNumEmpires * sizeof(unsigned int));
}

void MapFairnessEvaluator::EvaluateEmpireClaims(unsigned int iEmpireIndex) {

    unsigned int iHWIndex = GetHomeWorldIndex(m_piEmpireKey[iEmpireIndex]);
    Assert(iHWIndex != NO_KEY);

    // Analyze map for this empire
    m_dijkstra.Run(iHWIndex, false);

    for (unsigned int i = 0; i < m_iNumPlanets; i ++) {

        if (i != iHWIndex) {

            unsigned int iDistance = m_dijkstra.GetShortestPathLength(i);
            Assert(iDistance != 0);

            if (iDistance != NO_PATH) {
         
                // Add claim to map
                PlanetClaim cClaim = { iEmpireIndex, iDistance, (double) 1 / iDistance, true };
                m_pllClaims[i].PushFirst(cClaim);
            }
        }
    }
}

// Rules of thumb:
//
// Ag is worth 25% more than other resources
// Resources per planet are given out as an inverse factor of slightly supra-linear distance
// Claims that have no realistic relevance to the empire (e.g. distance of 10 to one empire, and distance of 2 to another) are discarded

void MapFairnessEvaluator::ProcessClaims()
{
    for (unsigned int i = 0; i < m_iNumPlanets; i ++) {

        const LinkedList<PlanetClaim>& list = m_pllClaims[i];
        if (list.GetNumElements() > 0) {

            ListIterator<PlanetClaim> iter;

            const Variant* pvPlanetData = m_ppvPlanetData[i];
            int iAg = pvPlanetData[GameMap::iAg].GetInteger();
            int iMin = pvPlanetData[GameMap::iMinerals].GetInteger();
            int iFuel = pvPlanetData[GameMap::iFuel].GetInteger();

            int iTotalResources = (int) ((double)iAg * 1.25 + iMin + iFuel);

            unsigned int iMinDistance = UINT_MAX;
            while(list.GetNextIterator(&iter))
            {
                PlanetClaim claim = iter.GetData();
                if (claim.iDistance < iMinDistance)
                    iMinDistance = claim.iDistance;
            }

            double dInverseMinDistance = (double)1 / iMinDistance;
            double dTotalInverseDistance = 0;
            iter.Reset();
            while(list.GetNextIterator(&iter)) {

                PlanetClaim claim = iter.GetData();
                if (claim.dDistanceInverse * 3 < dInverseMinDistance)
                    claim.bValid = false;
                else
                    dTotalInverseDistance += claim.dDistanceInverse;
            }

            iter.Reset();
            while(list.GetNextIterator(&iter)) {

                PlanetClaim claim = iter.GetData();
                if (claim.bValid) {

                    double dDistanceFactor = claim.dDistanceInverse / dTotalInverseDistance;
                    dDistanceFactor *= pow(claim.dDistanceInverse, 1.25);
                    int iClaimedResources = (int)(dDistanceFactor * iTotalResources);

                    m_piEmpireResources[claim.iEmpireIndex] += iClaimedResources;
                }
            }
        }            
    }
}

unsigned int MapFairnessEvaluator::GetHomeWorldIndex(unsigned int iEmpireKey) {

    for (unsigned int i = 0; i < m_iNumPlanets; i ++)
    {
        const Variant* pvPlanetData = m_ppvPlanetData[i];

        if (pvPlanetData[GameMap::iHomeWorld].GetInteger() == HOMEWORLD &&
            (unsigned int)pvPlanetData[GameMap::iOwner].GetInteger() == iEmpireKey)
        {
            return i;
        }
    }

    return NO_KEY;
}