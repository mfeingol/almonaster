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

#if !defined(AFX_FAIRMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
#define AFX_FAIRMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_

#include "BaseMapGenerator.h"

class FairMapGenerator : public IMapGenerator {
private:

    IGameEngine* m_pGameEngine;

    IMapGenerator* m_pInner;
    GameFairnessOption m_gfoFairness;

    int m_iStdDevPercentageOfMean;

    // Methods
    FairMapGenerator(IGameEngine* pGameEngine, IMapGenerator* pInner, GameFairnessOption gfoFairness);
    ~FairMapGenerator();

    int CheckEnforceFairness(int iGameClass, int iGameNumber, bool* pbEnforce);

    int EvaluateMap(int iGameClass, int iGameNumber,
                    const Variant** pvNewPlanetData, unsigned int iNumPlanets,
                    unsigned int* piEmpireKey, unsigned int iNumEmpires,
                    bool* pbAcceptable);

    void WriteReport(bool bAcceptable, unsigned int iAttempts);
    static int GetStandardDeviationPercentageOfMean(GameFairnessOption gfoFairness, unsigned int iNumEmpires);

    int GetAllEmpireKeys(int iGameClass, int iGameNumber,
                         unsigned int** ppiEmpireKey, unsigned int* piNumEmpires);

public:

    IMPLEMENT_INTERFACE (IMapGenerator);

    static IMapGenerator* CreateInstance(IGameEngine* pGameEngine, IMapGenerator* pInner, GameFairnessOption gfoFairness);

    virtual int CreatePlanets (
        
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

    virtual void FreePlanetData(Variant** ppvNewPlanetData);
};

#endif // !defined(AFX_FAIRMAPGENERATOR_H__DB071C4C_B823_4571_B2C9_56FE5DC4D31A__INCLUDED_)
