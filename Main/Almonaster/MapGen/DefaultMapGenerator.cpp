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

#include "DefaultMapGenerator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DefaultMapGenerator::DefaultMapGenerator(GameEngine* pGameEngine) 
    :
    BaseMapGenerator(pGameEngine) {
}

int DefaultMapGenerator::CreatePlanetChains() {

    AllocateDefaultPlanetData();

    // The default map generator algorithm is as follows:
    // 1) Create a planet chain for each empire
    // 2) There is no step 2...
    int iErrCode = OK;
    for (unsigned int i = 0; i < m_iNumNewEmpires; i ++)
    {
        iErrCode = CreatePlanetChain(m_piNewEmpireKey[i]);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

void DefaultMapGenerator::AllocateDefaultPlanetData() {

    // Allocate data for all new planets
    Assert(m_iNumPlanetsPerEmpire > 0);
    Assert(m_iNumNewEmpires > 0);

    AllocatePlanetData(m_iNumPlanetsPerEmpire * m_iNumNewEmpires);
}