//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998-2004-2001 Max Attar Feingold (maf6@cornell.edu)
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

#if !defined(AFX_TWISTEDMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
#define AFX_TWISTEDMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_

#include "MirroredMapGenerator.h"

struct PlanetCoordinates {
    int iX;
    int iY;
    bool bHW;
};

struct EdgeDescription {
    int iLength;
    PlanetCoordinates* pcEdgePlanet;
};

struct BestJoin {
    CardinalPoint cpJoin;
    int xOffset;
    int yOffset;
};

class TwistedMapGenerator : public MirroredMapGenerator {
private:

    EdgeDescription m_ed[NUM_CARDINAL_POINTS];
    int m_piExtreme[NUM_CARDINAL_POINTS];

    unsigned int m_iBestJoinNumConnections;
    LinkedList<BestJoin> m_llBestJoins;

    CardinalPoint m_cpChosenEdge;
    int m_iChosenXOffset;
    int m_iChosenYOffset;

    TwistedMapGenerator(IGameEngine* pGameEngine);
    ~TwistedMapGenerator();

    virtual int CreatePlanetChains();

    int CompleteTwistedMap();
    void CreateLinksBetweenTwists();

    int BuildEdgeDescriptions();

    void ComputeBestTwistedJoin();
    void ComputeBestTwistedJoinNorthSouth(CardinalPoint cpEdge);
    void ComputeBestTwistedJoinEastWest(CardinalPoint cpEdge);

    void TwistCoordinates(int iX, int iY,
                          CardinalPoint cpEdge,
                          int xOffset, int yOffset,
                          int* piTwistedX, int* piTwistedY);
    void TwistLinks(int iLink, CardinalPoint cpEdge, int* piTwistedLink);

public:

    static IMapGenerator* CreateInstance(IGameEngine* pGameEngine);
};

#endif // !defined(AFX_DEFAULTMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
