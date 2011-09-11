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

#pragma once

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

class TwistedMapGenerator : public MirroredMapGenerator
{
private:

    EdgeDescription m_ed[NUM_CARDINAL_POINTS];
    int m_piExtreme[NUM_CARDINAL_POINTS];

    unsigned int m_iBestJoinNumConnections;
    LinkedList<BestJoin> m_llBestJoins;

    CardinalPoint m_cpChosenEdge;
    int m_iChosenXOffset;
    int m_iChosenYOffset;

    virtual int CreatePlanetChains();

    void CompleteTwistedMap();
    void CreateLinksBetweenTwists();

    void BuildEdgeDescriptions();

    void ComputeBestTwistedJoin();
    void ComputeBestTwistedJoinNorthSouth(CardinalPoint cpEdge);
    void ComputeBestTwistedJoinEastWest(CardinalPoint cpEdge);

    void TwistCoordinates(int iX, int iY,
                          CardinalPoint cpEdge,
                          int xOffset, int yOffset,
                          int* piTwistedX, int* piTwistedY);
    void TwistLinks(int iLink, CardinalPoint cpEdge, int* piTwistedLink);

public:

    TwistedMapGenerator(GameEngine* pGameEngine);
    virtual ~TwistedMapGenerator();
};
