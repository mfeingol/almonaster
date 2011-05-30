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

#if !defined(AFX_BASEMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
#define AFX_BASEMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_

#include "../GameEngine/GameEngine.h"

#include "Osal/HashTable.h"

class CoordHashValue {
public:
    static unsigned int GetHashValue (const char* pszData, unsigned int iNumBuckets, const void* pHashHint) {
        return Algorithm::GetStringHashValue (pszData, iNumBuckets, false);
    }
};

class CoordEquals {
public:
    static bool Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint) {
        return strcmp (pszLeft, pszRight) == 0;
    }
};

struct PlanetLocation {

    PlanetLocation() {
        iNumLinkedPlanets = 0;
    }

    int iX;
    int iY;
    CardinalPoint cpDirectionFromCoordinates;
    unsigned int iNumLinkedPlanets;
    unsigned int piLinkPlanetIndex[NUM_CARDINAL_POINTS];
    CardinalPoint pcpLinkDirection[NUM_CARDINAL_POINTS];

    void AddLinkedPlanet(unsigned int iIndex, CardinalPoint cp) {
        Assert(iNumLinkedPlanets < NUM_CARDINAL_POINTS);
        piLinkPlanetIndex[iNumLinkedPlanets] = iIndex;
        pcpLinkDirection[iNumLinkedPlanets ++] = cp;
    }
};

class BaseMapGenerator : public IMapGenerator {

protected:

    IGameEngine* m_pGameEngine;

    //
    // Data provided by caller
    //

    int m_iGameClass;
    int m_iGameNumber;

    int* m_piNewEmpireKey;
    unsigned int m_iNumNewEmpires;

    Variant** m_ppvExistingPlanetData;  // GameMap rows
    unsigned int m_iNumExistingPlanets;

    Variant* m_pvGameClassData; // SystemGameClassData row
    Variant* m_pvGameData;  // GameData row

    //
    // Data read from GameEngine
    //
    MapConfiguration m_mcConfig;

    int m_iGameClassOptions;
    unsigned int m_iNumPlanetsPerEmpire;

    //
    // Data we allocate ourselves
    //

    // Array of new planet data that we allocate - these are uninitialized GameMap rows
    Variant** m_ppvNewPlanetData;
    unsigned int m_iTotalNumNewPlanets;

    // Hashtable for fast coordinate lookups
    HashTable<const char*, Variant*, CoordHashValue, CoordEquals> m_htCoordinates;

    //
    // Volatile data
    //

    // Number of planets created while processing the entire batch of new empires
    unsigned int m_iNumNewPlanetsCreated;

    //
    // Per-chain volatile data
    //

    // Number of planets created while processing the chain
    unsigned int m_iNumChainPlanetsCreated;

    // Index of planet chosen to be empire's homeworld
    unsigned int m_iChainHomeWorldIndex;

    // Index of planet from previous chain chosen to start off new chain (if any)
    unsigned int m_iLinkedPlanetInPreviousChainIndex;
    CardinalPoint m_cpLinkedPlanetInPreviousChainDirection;

    //
    // Utility Methods
    //

    void ComputeGameResources();
    void AdvanceCoordinates (int iX, int iY, int* piX, int* piY, CardinalPoint cpDirection);
    bool DoesPlanetExist (int iX, int iY);
    bool IsPlanetInCurrentChain(const Variant* pvPlanetData);

    bool InsertMapCoordinates();
    Variant* FindPlanet(int iX, int iY);

    bool AllocatePlanetData(unsigned int iNumPlanets);

    void ResetNewPlanetChain();
    void RestartPlanetChain();

    int CreatePlanetChain(unsigned int iEmpireKey);
    int CreatePlanet(unsigned int iEmpireKey, PlanetLocation* plLocation);

    int GetNewPlanetLocation(PlanetLocation* plLocation);
    int GetFirstPlanetLocation(PlanetLocation* plLocation);
    int GetNewPlanetLocationFromMap(PlanetLocation* plLocation);
    int GetNewPlanetLocationFromPreviouslyCreatedChain(PlanetLocation* plLocation);
    int GetNewPlanetLocationFromCurrentChain(PlanetLocation* plLocation);

    void ChooseHomeworldForChain();
    void AssignResourcesForChain();
    void CreateNonDefaultLinksForChain();

    void AssignResources(unsigned int iHWIndex, unsigned int iStartIndex, unsigned int iNumPlanets);

    int InsertIntoCoordinatesTable(unsigned int iPlanetIndex);
    int SetCoordinates(unsigned int iPlanetIndex, int iX, int iY);
    void AddLink(unsigned int iPlanetIndex, CardinalPoint cp);
    void AddLink(Variant* pvPlanetData, CardinalPoint cp);

    void GetCoordinates(const Variant* pvPlanetData, int* piX, int* piY);
    int CopyPlanetData(const Variant* pvSrcPlanetData, Variant* pvDestPlanetData);

    BaseMapGenerator(IGameEngine* pGameEngine);
    ~BaseMapGenerator();

    virtual int CreatePlanetChains() = 0;

public:

    IMPLEMENT_INTERFACE (IMapGenerator);

    int CreatePlanets(

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
        );

    void FreePlanetData(Variant** ppvNewPlanetData);
};

#endif // !defined(AFX_BASEMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
