// DefaultMapGenerator.h: interface for the DefaultMapGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEFAULTMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
#define AFX_DEFAULTMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_

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


class DefaultMapGenerator : public IMapGenerator {

protected:

    // Objects - may be needed
    IGameEngine* m_pGameEngine;
    IDatabase* m_pDatabase;

    // Map configuration data
    MapConfiguration m_mcConfig;

    //
    // Data
    //
    // Note - all heap memory is allocated by caller of CreatePlanets

    int m_iGameClass;
    int m_iGameNumber;
    int m_iEmpireKey;

    int m_iGameClassOptions;

    // Data of already created planets - GameMap rows
    Variant** m_ppvPlanetData;
    unsigned int m_iNumPlanets;
    
    // GameClass data - SystemGameClassData row
    Variant* m_pvGameClassData;

    // Game data - GameData row
    Variant* m_pvGameData;

    // Array of new planet data - uninitialized GameMap rows
    Variant** m_ppvNewPlanetData;
    unsigned int m_iNumNewPlanets;

    // Number of planets created so far
    unsigned int m_iNumPlanetsCreated;

    // Index of planet chosen to be empire's homeworld
    unsigned int m_iHomeWorldIndex;

    // Hashtable for fast coordinate lookups
    HashTable<const char*, Variant*, CoordHashValue, CoordEquals> m_htCoordinates;

    // Methods
    void RollDiceForGameResources();
    void AdvanceCoordinates (int iX, int iY, int* piX, int* piY, CardinalPoint cpDirection);
    bool DoesPlanetExist (int iX, int iY);
    bool InsertMapCoordinates();

    void ChooseHomeworld();
    void AssignResources();
    void AddNonDefaultLinks();

    bool CreateFirstPlanet();
    bool CreatePlanet (unsigned int iIndex, int iX, int iY, CardinalPoint cpDirection);

    int CreatePlanets();

    bool ChoosePlanetAndDirectionFromMap (unsigned int* piIndex, CardinalPoint* pcpDirection);
    bool ChoosePlanetAndDirectionFromNewPlanets (unsigned int* piIndex, int* piX, int* piY, 
        CardinalPoint* pcpDirection);

    DefaultMapGenerator (IGameEngine* pGameEngine);
    ~DefaultMapGenerator();

public:

    IMPLEMENT_INTERFACE (IMapGenerator);

    static IMapGenerator* CreateInstance (IGameEngine* pGameEngine);

    //
    // Main IMapGenerator method
    //
    //
    // Implementor must do the following:
    //
    // 1) If iNumPlanets == 0, fill in the following columns in the pvGameData row:
    //
    // GameData::NumPlanetsPerEmpire,
    // GameData::HWAg
    // GameData::AvgAg
    // GameData::HWMin
    // GameData::AvgMin
    // GameData::HWFuel
    // GameData::AvgFuel
    //
    // 2) Fill in iNumNewPlanets rows in ppvNewPlanetData with the following columns:
    //
    // GameMap::Ag,
    // GameMap::Minerals
    // GameMap::Fuel
    // GameMap::Coordinates
    // GameMap::Link
    // GameMap::HomeWorld
    //
    // Everything else will be taken care of by the caller
    //
    // Sanity rules apply:
    // - Coordinates already in use on the map must not be used
    // - Links must actually have a planet behind them
    // - Exactly one homeworld per empire must be selected
    // - etc.

    int CreatePlanets (

        int iGameClass, 
        int iGameNumber, 
        int iEmpireKey, 

        Variant** ppvPlanetData, 
        unsigned int iNumPlanets,

        Variant* pvGameClassData,
        Variant* pvGameData,

        Variant** ppvNewPlanetData,
        unsigned int iNumNewPlanets
        );
};

#endif // !defined(AFX_DEFAULTMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
