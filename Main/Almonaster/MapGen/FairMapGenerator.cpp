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

#include "Osal/Algorithm.h"

#include "Global.h"
#include "FairMapGenerator.h"
#include "MapFairnessEvaluator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FairMapGenerator::FairMapGenerator(IMapGenerator* pInner, GameFairnessOption gfoFairness) 
    :
    m_pInner(pInner),
    m_gfoFairness(gfoFairness)
{
}

int FairMapGenerator::CreatePlanets(
        
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
        ) {

    bool bEnforce;
    int iErrCode = CheckEnforceFairness(iGameClass, iGameNumber, &bEnforce);
    if (iErrCode != OK)
        return iErrCode;

    if (!bEnforce) {
        return m_pInner->CreatePlanets(iGameClass, iGameNumber, piNewEmpireKey, iNumNewEmpires,
                                       ppvExistingPlanetData, iNumExistingPlanets,
                                       pvGameClassData, pvGameData,
                                       pppvNewPlanetData,
                                       piNumNewPlanets);
    }

    unsigned int* piTotalEmpireKeys = NULL, iTotalNumEmpires = iNumNewEmpires;
    Algorithm::AutoDelete<unsigned int> autoDelKeys(piTotalEmpireKeys, true);
    if (ppvExistingPlanetData != NULL) {
        iErrCode = GetAllEmpireKeys(iGameClass, iGameNumber, &piTotalEmpireKeys, &iTotalNumEmpires);
        if (iErrCode != OK)
            return iErrCode;
    }

    m_iStdDevPercentageOfMean = GetStandardDeviationPercentageOfMean(m_gfoFairness, iTotalNumEmpires);

    const unsigned int MAX_ATTEMPTS = 1000;
    unsigned int iAttempts = 0;

    bool bAcceptable = false;
    while (!bAcceptable && iAttempts < MAX_ATTEMPTS) {

        iErrCode = m_pInner->CreatePlanets(iGameClass, iGameNumber, piNewEmpireKey, iNumNewEmpires,
                                           ppvExistingPlanetData, iNumExistingPlanets,
                                           pvGameClassData, pvGameData,
                                           pppvNewPlanetData,
                                           piNumNewPlanets);
        if (iErrCode != OK)
            return iErrCode;

        if (ppvExistingPlanetData == NULL) {

            iErrCode = EvaluateMap(iGameClass, iGameNumber,
                                   const_cast<const Variant**>(*pppvNewPlanetData), *piNumNewPlanets,
                                   (unsigned int*)piNewEmpireKey, iNumNewEmpires, &bAcceptable);        
        } else {

            Assert(iNumNewEmpires == 1 && iTotalNumEmpires > iNumNewEmpires);

            unsigned int iTotalNumPlanets = iNumExistingPlanets + *piNumNewPlanets;
            Variant** ppvTotalMap = new Variant*[iTotalNumPlanets];
            if (ppvTotalMap == NULL)
                return ERROR_OUT_OF_MEMORY;

            memcpy(ppvTotalMap, ppvExistingPlanetData, iNumExistingPlanets * sizeof(Variant*));
            memcpy(ppvTotalMap + iNumExistingPlanets, *pppvNewPlanetData, *piNumNewPlanets * sizeof(Variant*));

            iErrCode = EvaluateMap(iGameClass, iGameNumber,
                                   const_cast<const Variant**>(ppvTotalMap), iTotalNumPlanets,
                                   piTotalEmpireKeys, iTotalNumEmpires, &bAcceptable);
            delete [] ppvTotalMap;
        }

        if (iErrCode != OK)
            return iErrCode;

        iAttempts ++;
    }

    WriteReport(bAcceptable, iAttempts);

    return OK;
}

void FairMapGenerator::WriteReport(bool bAcceptable, unsigned int iAttempts)
{
    char pszReport[256];

    if (bAcceptable) {
        sprintf(pszReport, "Fair map generated after %d attempt", iAttempts);
        if (iAttempts != 1)
            strcat(pszReport, "s");
    } else {
        sprintf(pszReport, "Failed to generate fair map after %d attempts", iAttempts);
    }

    global.GetReport()->WriteReport(pszReport);
}

int FairMapGenerator::EvaluateMap(int iGameClass, int iGameNumber,
                                  const Variant** pvNewPlanetData, unsigned int iNumPlanets,
                                  unsigned int* piEmpireKey, unsigned int iNumEmpires,
                                  bool* pbAcceptable) {

    MapFairnessEvaluator mfeEval(pvNewPlanetData, iNumPlanets, piEmpireKey, iNumEmpires);

    int iErrCode = mfeEval.Run();
    if (iErrCode != OK)
        return iErrCode;

    unsigned int* piResourceClaim = (unsigned int*)StackAlloc(iNumEmpires * sizeof(unsigned int));
    for (unsigned int i = 0; i < iNumEmpires; i ++) {
        piResourceClaim[i] = mfeEval.GetResourceClaim(piEmpireKey[i]);
    }

    unsigned int iDev = Algorithm::GetStandardDeviation(piResourceClaim, iNumEmpires);
    unsigned int iMean = Algorithm::GetArithmeticMean(piResourceClaim, iNumEmpires);

    int iPercentage = 100 * iDev / iMean;

    if (m_iStdDevPercentageOfMean < 0) {
        // Result should be greater than abs(target)
        *pbAcceptable = iPercentage > -m_iStdDevPercentageOfMean;
    } else {
        // Result should be less than target
        *pbAcceptable = iPercentage < m_iStdDevPercentageOfMean;
    }

    if (*pbAcceptable) {

        // Record results for posterity
        iErrCode = m_gameEngine.SetGameProperty(iGameClass, iGameNumber, 
                                                GameData::MapFairnessStandardDeviationPercentageOfMean,
                                                iPercentage);

        for (unsigned int i = 0; i < iNumEmpires && iErrCode == OK; i ++) {

            iErrCode = m_gameEngine.SetEmpireGameProperty(iGameClass, iGameNumber, piEmpireKey[i],
                                                          GameEmpireData::MapFairnessResourcesClaimed,
                                                          mfeEval.GetResourceClaim(piEmpireKey[i]));
        }
    }

    return iErrCode;
}

void FairMapGenerator::FreePlanetData(Variant** ppvNewPlanetData) {
    m_pInner->FreePlanetData(ppvNewPlanetData);
}

int FairMapGenerator::CheckEnforceFairness(int iGameClass, int iGameNumber, bool* pbEnforce) {

    // If more than 1 update has transpired already, don't try to keep things fair
    int iErrCode = OK;
    bool bEnforce = false;

    if (m_gfoFairness != GAME_FAIRNESS_RANDOM) {

        int iNumUpdates;
        int iErrCode = m_gameEngine.GetNumUpdates(iGameClass, iGameNumber, &iNumUpdates);
        if (iErrCode == OK)
            bEnforce = iNumUpdates < 2;
    }

    *pbEnforce = bEnforce;
    return iErrCode;
}

int FairMapGenerator::GetStandardDeviationPercentageOfMean(GameFairnessOption gfoFairness, unsigned int iNumEmpires) {

    Assert(iNumEmpires > 1);

    if (iNumEmpires < 3) {

        switch (gfoFairness) {
        case GAME_FAIRNESS_VERY_FAIR:
            return 5;
        case GAME_FAIRNESS_SOMEWHAT_FAIR:
            return 15;
        case GAME_FAIRNESS_SOMEWHAT_UNFAIR:
            return -5;
        case GAME_FAIRNESS_VERY_UNFAIR:
            return -15;
        }

    } else if (iNumEmpires < 9) {

        switch (gfoFairness) {
        case GAME_FAIRNESS_VERY_FAIR:
            return 10;
        case GAME_FAIRNESS_SOMEWHAT_FAIR:
            return 20;
        case GAME_FAIRNESS_SOMEWHAT_UNFAIR:
            return -10;
        case GAME_FAIRNESS_VERY_UNFAIR:
            return -20;
        }

    } else {

        switch (gfoFairness) {
        case GAME_FAIRNESS_VERY_FAIR:
            return 20;
        case GAME_FAIRNESS_SOMEWHAT_FAIR:
            return 35;
        case GAME_FAIRNESS_SOMEWHAT_UNFAIR:
            return -20;
        case GAME_FAIRNESS_VERY_UNFAIR:
            return -35;
        }
    }

    Assert(false);
    return 100;
}

int FairMapGenerator::GetAllEmpireKeys(int iGameClass, int iGameNumber,
                                       unsigned int** ppiEmpireKey, unsigned int* piNumEmpires) {

    Variant* pvEmpireKey = NULL;
    unsigned int* piTotalEmpireKey = NULL, iNumTotalEmpires;

    int iErrCode = m_gameEngine.GetEmpiresInGame(iGameClass, iGameNumber, &pvEmpireKey, &iNumTotalEmpires);
    if (iErrCode != OK)
        return iErrCode;

    piTotalEmpireKey = new unsigned int[iNumTotalEmpires];
    if (piTotalEmpireKey == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
    } else {
        for (unsigned int i = 0; i < iNumTotalEmpires; i ++) {
            piTotalEmpireKey[i] = pvEmpireKey[i].GetInteger();
        }
        t_pCache->FreeData(pvEmpireKey);
    }

    *piNumEmpires = iNumTotalEmpires;
    *ppiEmpireKey = piTotalEmpireKey;
    
    return iErrCode;
}