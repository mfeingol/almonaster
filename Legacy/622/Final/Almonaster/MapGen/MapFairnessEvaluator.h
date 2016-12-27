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

#if !defined(AFX_MAPFAIRNESSEVALUATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
#define AFX_MAPFAIRNESSEVALUATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_

#include "Osal/LinkedList.h"
#include "Osal/Variant.h"

#include "Dijkstra.h"

struct PlanetClaim {
    unsigned int iEmpireIndex;
    unsigned int iDistance;
    double dDistanceInverse;
    bool bValid;
};

class MapFairnessEvaluator {
private:

    unsigned int m_iNumPlanets, m_iNumEmpires;

    const unsigned int* m_piEmpireKey;
    const Variant** m_ppvPlanetData;

    Dijkstra m_dijkstra;

    LinkedList<PlanetClaim>* m_pllClaims;
    unsigned int* m_piEmpireResources;

    int Initialize();
    int EvaluateEmpireClaims(unsigned int iEmpireIndex);
    void ProcessClaims();
    unsigned int GetHomeWorldIndex(unsigned int iEmpireKey);

public:

    MapFairnessEvaluator(const Variant** ppvPlanetData, unsigned int iNumPlanets, 
                         const unsigned int* piEmpireKey, unsigned int iNumEmpires);
    ~MapFairnessEvaluator();

    int Run();
    unsigned int GetResourceClaim(unsigned int iEmpireKey);
};

#endif // !defined(AFX_MAPFAIRNESSEVALUATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
