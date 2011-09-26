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

#include "AlmonasterScore.h"
#include "GameEngine.h"
#include "ClassicScore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AlmonasterScore::AlmonasterScore(GameEngine* pGameEngine)
{
    m_pGameEngine = pGameEngine;
}

bool AlmonasterScore::HasTopList() {
    return true;
}

int AlmonasterScore::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges)
{
    float fNukerScore, fNukedScore, fIncrease, fDecrease;
    
    int iErrCode, iNukerSignificance, iNukedSignificance, iNukerAllies, iNukedAllies;

    GET_GAME_EMPIRE_DIPLOMACY (strNukerDip, iGameClass, iGameNumber, iEmpireNuker);
    GET_GAME_EMPIRE_DIPLOMACY (strNukedDip, iGameClass, iGameNumber, iEmpireNuked);
    
    // Get empire statistics
    iErrCode = GetRelevantStatistics (
        iGameClass,
        iGameNumber,
        iEmpireNuker,
        &fNukerScore,
        &iNukerSignificance,
        &iNukerAllies
        );

    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetRelevantStatistics (
        iGameClass,
        iGameNumber,
        iEmpireNuked,
        &fNukedScore,
        &iNukedSignificance,
        &iNukedAllies
        );

    RETURN_ON_ERROR(iErrCode);

    // Run algorithm
    GetAlmonasterScoreChanges (
        fNukerScore, 
        fNukedScore, 
        iNukerSignificance,
        iNukedSignificance,
        iNukerAllies, 
        iNukedAllies, 
        &fIncrease, 
        &fDecrease
        );

    GET_SYSTEM_EMPIRE_DATA(strNuked, iEmpireNuked);
    GET_SYSTEM_EMPIRE_DATA(strNuker, iEmpireNuker);

    iErrCode = t_pCache->Increment(strNuked, iEmpireNuked, SystemEmpireData::AlmonasterScore, -fDecrease);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(strNuker, iEmpireNuker, SystemEmpireData::AlmonasterScore, fIncrease);
    RETURN_ON_ERROR(iErrCode);

    // Increment significance
    iErrCode = t_pCache->Increment(strNuker, iEmpireNuker, SystemEmpireData::AlmonasterScoreSignificance, 1);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(strNuked, iEmpireNuked, SystemEmpireData::AlmonasterScoreSignificance, 1);
    RETURN_ON_ERROR(iErrCode);

    // Adjust privileges after score changes
    iErrCode = m_pGameEngine->CalculatePrivilegeLevel (iEmpireNuker);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = m_pGameEngine->CalculatePrivilegeLevel (iEmpireNuked);
    RETURN_ON_ERROR(iErrCode);

    // Do accounting
    pscChanges->iFlags |= ALMONASTER_NUKER_SCORE_CHANGE | ALMONASTER_NUKED_SCORE_CHANGE;
    pscChanges->fAlmonasterNukerScore = fNukerScore;
    pscChanges->fAlmonasterNukerChange = fIncrease;
    pscChanges->fAlmonasterNukedScore = fNukedScore;
    pscChanges->fAlmonasterNukedChange = - fDecrease;

    return iErrCode;
}

int AlmonasterScore::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges)
{
    // Same as nuke
    int iErrCode = OnNuke (iGameClass, iGameNumber, iWinner, iLoser, pscChanges);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int AlmonasterScore::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) {

    // We need to wait for a colonizer or the game's end to give out credit and punishment,
    // so we don't do anything right now

    return OK;
}

int AlmonasterScore::On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, 
                                                     int iPlanetKey, ScoringChanges* pscChanges)
{
    bool bValid;

    float fWinnerScore, fLoserScore, fWinnerIncrease, fLoserDecrease;
    int iErrCode, iLoserSignificance, iLoserNumAllies, iWinnerNumAllies, iWinnerSignificance, iLoserKey;

    int64 i64EmpireSecretKey;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRE_DIPLOMACY (strWinnerDip, iGameClass, iGameNumber, iWinnerKey);

    iErrCode = GetRelevantStatistics (
        iGameClass,
        iGameNumber,
        iWinnerKey,
        &fWinnerScore,
        &iWinnerSignificance,
        &iWinnerNumAllies
        );

    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetRelevantStatisticsFromPlanet (
        strGameMap,
        iPlanetKey,
        &fLoserScore,
        &iLoserSignificance,
        &iLoserNumAllies,
        &iLoserKey,
        &i64EmpireSecretKey,
        NULL
        );

    RETURN_ON_ERROR(iErrCode);

    GetAlmonasterScoreChanges (
        fWinnerScore,
        fLoserScore, 
        iWinnerSignificance,
        iLoserSignificance,
        iWinnerNumAllies, 
        iLoserNumAllies, 
        &fWinnerIncrease, 
        &fLoserDecrease
        );

    // Increment winner's score and significance
    GET_SYSTEM_EMPIRE_DATA(strWinner, iWinnerKey);
    iErrCode = t_pCache->Increment(
        strWinner, 
        iWinnerKey, 
        SystemEmpireData::AlmonasterScore, 
        fWinnerIncrease
        );

    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(
        strWinner, 
        iWinnerKey, 
        SystemEmpireData::AlmonasterScoreSignificance, 
        1
        );

    RETURN_ON_ERROR(iErrCode);

    // Do accounting for winner
    pscChanges->iFlags |= ALMONASTER_NUKER_SCORE_CHANGE;
    pscChanges->fAlmonasterNukerScore = fWinnerScore;
    pscChanges->fAlmonasterNukerChange = fWinnerIncrease;

    // Try to find the loser
    // This doesn't really belong here, but it's a convenient place to do it for perf
    iErrCode = m_pGameEngine->CheckSecretKey (iLoserKey, i64EmpireSecretKey, &bValid, NULL, NULL);
    RETURN_ON_ERROR(iErrCode);

    if (bValid) {

        GET_SYSTEM_EMPIRE_DATA(strLoser, iLoserKey);

        iErrCode = t_pCache->Increment(
            strLoser, 
            iLoserKey, 
            SystemEmpireData::AlmonasterScore, 
            - fLoserDecrease
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->Increment(
            strLoser, 
            iLoserKey, 
            SystemEmpireData::AlmonasterScoreSignificance, 
            1
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        // Do accounting for loser
        pscChanges->iFlags |= ALMONASTER_NUKED_SCORE_CHANGE;
        pscChanges->fAlmonasterNukedScore = fLoserScore;
        pscChanges->fAlmonasterNukedChange = - fLoserDecrease;
    }

    return iErrCode;
}


int AlmonasterScore::OnGameEnd (int iGameClass, int iGameNumber) {

    int iErrCode;
    Variant vOptions, * pvHomeWorld = NULL, * pvEmpireKey = NULL;
    AutoFreeData free_pvHomeWorld(pvHomeWorld);
    AutoFreeData free_pvEmpireKey(pvEmpireKey);

    float* pfWinnerScore = NULL;
    int* piWinnerNumAllies = NULL, * piWinnerSignificance = NULL;

    unsigned int i, j, iNumPlanets, * piPlanetKey = NULL, iNumEmpires = 0;
    AutoFreeKeys free_piPlanetKey(piPlanetKey);

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    // See if game has 3.0 surrenders flag set
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );

    RETURN_ON_ERROR(iErrCode);

    if (!(vOptions.GetInteger() & USE_SC30_SURRENDERS))
    {
        return OK;
    }

    // Need to scan map for uncolonized homeworlds
    iErrCode = t_pCache->ReadColumn(
        strGameMap,
        GameMap::HomeWorld,
        &piPlanetKey,
        &pvHomeWorld,
        &iNumPlanets
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    bool bReadGameEmpires = false;
    for (i = 0; i < iNumPlanets; i ++)
    {
        if (pvHomeWorld[i].GetInteger() != HOMEWORLD &&
            pvHomeWorld[i].GetInteger() != NOT_HOMEWORLD) {

            // Fault in survivors and their stats
            if (!bReadGameEmpires)
            {
                bReadGameEmpires = true;

                iErrCode = t_pCache->ReadColumn(
                    strGameEmpires,
                    GameEmpires::EmpireKey,
                    NULL,
                    &pvEmpireKey,
                    &iNumEmpires
                    );

                // There may or may not be empires left...
                if (iErrCode == ERROR_DATA_NOT_FOUND)
                {
                    iErrCode = OK;
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);

                    pfWinnerScore = (float*) StackAlloc (iNumEmpires * sizeof (float));
                    piWinnerSignificance = (int*) StackAlloc (2 * iNumEmpires * sizeof (int));
                    piWinnerNumAllies = piWinnerSignificance + iNumEmpires;
                
                    for (j = 0; j < iNumEmpires; j ++)
                    {
                        iErrCode = GetRelevantStatistics (
                            iGameClass, 
                            iGameNumber, 
                            pvEmpireKey[j].GetInteger(),
                            pfWinnerScore + j,
                            piWinnerSignificance + j,
                            piWinnerNumAllies + j
                            );
                
                        RETURN_ON_ERROR(iErrCode);
                    }
                }
            }

            // Handle all necessary processing
            if (pvEmpireKey)
            {
                iErrCode = HandleUncolonizedHomeWorldOnEndGame (
                    iGameClass,
                    iGameNumber,
                    piPlanetKey[i],
                    pvEmpireKey,
                    pfWinnerScore,
                    piWinnerSignificance,
                    piWinnerNumAllies,
                    iNumEmpires
                    );

                RETURN_ON_ERROR(iErrCode);
            }
        }
    }

    return iErrCode;
}


int AlmonasterScore::OnWin (int iGameClass, int iGameNumber, int iEmpireKey) {

    return OK;
}

int AlmonasterScore::OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) {

    return OK;
}

int AlmonasterScore::OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) {

    //
    // Ruin == nuked by the worst empire in the game
    //

    int iErrCode;
    Variant vScore, vSignificance, vEmpScore, vEmpSignificance, vKey;

    float fNukerIncrease, fNukedDecrease, fMaxNukedDecrease = -1.0;

    unsigned int iKey = NO_KEY;
    GET_GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = m_pGameEngine->GetEmpireProperty(iEmpireKey, SystemEmpireData::AlmonasterScore, &vScore);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = m_pGameEngine->GetEmpireProperty(
        iEmpireKey,
        SystemEmpireData::AlmonasterScoreSignificance, 
        &vSignificance
        );

    RETURN_ON_ERROR(iErrCode);

    while (true) {

        iErrCode = t_pCache->GetNextKey (pszEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadData(pszEmpires, iKey, GameEmpires::EmpireKey, &vKey);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = m_pGameEngine->GetEmpireProperty(
            vKey.GetInteger(),
            SystemEmpireData::AlmonasterScore, 
            &vEmpScore
            );
        RETURN_ON_ERROR(iErrCode);

        iErrCode = m_pGameEngine->GetEmpireProperty(
            vKey.GetInteger(),
            SystemEmpireData::AlmonasterScoreSignificance, 
            &vEmpSignificance
            );
        RETURN_ON_ERROR(iErrCode);

        GetAlmonasterScoreChanges (
            vEmpScore.GetFloat(), 
            vScore.GetFloat(), 
            vEmpSignificance.GetInteger(), 
            vSignificance.GetInteger(), 
            0,
            0,
            &fNukerIncrease,
            &fNukedDecrease
            );
        Assert(fNukedDecrease >= (float) 0.0);

        if (fNukedDecrease > fMaxNukedDecrease) {
            fMaxNukedDecrease = fNukedDecrease;
        }
    }

    Assert(fMaxNukedDecrease >= 0);

    if (fMaxNukedDecrease > 0)
    {
        GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

        iErrCode = t_pCache->Increment(
            strEmpire,
            iEmpireKey,
            SystemEmpireData::AlmonasterScore, 
            - fMaxNukedDecrease
            );

        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

void AlmonasterScore::GetAlmonasterScoreChanges (float fNukerScore, float fNukedScore, int iNukerSignificance,
                                                 int iNukedSignificance, int iNumNukerAllies, 
                                                 int iNumNukedAllies, float* pfNukerIncrease, 
                                                 float* pfNukedDecrease) {

    Assert(fNukerScore >= ALMONASTER_MIN_SCORE && fNukedScore <= ALMONASTER_MAX_SCORE);
    Assert(fNukedScore >= ALMONASTER_MIN_SCORE && fNukedScore <= ALMONASTER_MAX_SCORE);
    Assert(iNukerSignificance >= 0);
    Assert(iNukedSignificance >= 0);
    Assert(iNumNukerAllies >= 0);
    Assert(iNumNukedAllies >= 0);

    // Prevent division by zero
    iNumNukedAllies += 2;
    iNumNukerAllies += 2;

    iNukerSignificance += 2;
    iNukedSignificance += 2;

    // Calculate initial increase / decrease factors
    float fIncreaseFactor = fNukedScore / fNukerScore;
    float fDecreaseFactor = fIncreaseFactor;

    // Normalize increase factor
    if (fIncreaseFactor > ALMONASTER_MAX_INCREASE_FACTOR) {
        fIncreaseFactor = ALMONASTER_MAX_INCREASE_FACTOR;
    }

    // Normalize decrease factor
    if (fDecreaseFactor > ALMONASTER_MAX_DECREASE_FACTOR) {
        fDecreaseFactor = ALMONASTER_MAX_DECREASE_FACTOR;
    }

    // Calculate alliance ratio
    float fAllianceRatio = (float) iNumNukedAllies / iNumNukerAllies;
    if (fAllianceRatio > ALMONASTER_MAX_ALLIANCE_RATIO) {
        fAllianceRatio = ALMONASTER_MAX_ALLIANCE_RATIO;
    }
    else if (fAllianceRatio < ALMONASTER_MIN_ALLIANCE_RATIO) {
        fAllianceRatio = ALMONASTER_MIN_ALLIANCE_RATIO;
    }

    // Calculate significance ratios    
    float fNukerSignificanceRatio = (float) iNukerSignificance / iNukedSignificance;
    float fNukedSignificanceRatio = fNukerSignificanceRatio;

    if (fNukerSignificanceRatio > ALMONASTER_MAX_NUKER_SIGNIFICANCE_RATIO) {
        fNukerSignificanceRatio = ALMONASTER_MAX_NUKER_SIGNIFICANCE_RATIO;
    }
    else if (fNukerSignificanceRatio < ALMONASTER_MIN_NUKER_SIGNIFICANCE_RATIO) {
        fNukerSignificanceRatio = ALMONASTER_MIN_NUKER_SIGNIFICANCE_RATIO;
    }

    if (fNukedSignificanceRatio > ALMONASTER_MAX_NUKED_SIGNIFICANCE_RATIO) {
        fNukedSignificanceRatio = ALMONASTER_MAX_NUKED_SIGNIFICANCE_RATIO;
    }
    else if (fNukedSignificanceRatio < ALMONASTER_MIN_NUKED_SIGNIFICANCE_RATIO) {
        fNukedSignificanceRatio = ALMONASTER_MIN_NUKED_SIGNIFICANCE_RATIO;
    }

    // Calculate nuker empire's increase
    *pfNukerIncrease = ALMONASTER_BASE_UNIT * fIncreaseFactor * fAllianceRatio * fNukerSignificanceRatio;

    // Calculate nuked empire's decrease
    *pfNukedDecrease = ALMONASTER_BASE_UNIT * fDecreaseFactor * fAllianceRatio * fNukedSignificanceRatio;

    // Make sure decrease doesn't drop nuked empire below min score
    if (fNukedScore - *pfNukedDecrease < ALMONASTER_MIN_SCORE) {
         *pfNukedDecrease = fNukedScore - ALMONASTER_MIN_SCORE;
    }

    if (fNukerScore + *pfNukerIncrease > ALMONASTER_MAX_SCORE) {
        *pfNukerIncrease = fNukerScore - ALMONASTER_MAX_SCORE;
    }

    Assert(*pfNukedDecrease >= (float) 0.0 && *pfNukerIncrease > (float) 0.0);
}

int AlmonasterScore::GetRelevantStatistics (int iGameClass, int iGameNumber, int iEmpireKey,
                                            float* pfAlmonasterScore, int* piSignificance, 
                                            int* piNumAllies) {

    int iErrCode;

    Variant vTemp;

    unsigned int iEmpires;

    GET_GAME_EMPIRE_DIPLOMACY (strDip, iGameClass, iGameNumber, iEmpireKey);

    // Get winner's score, significance and allies
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->ReadData(
        strEmpire, 
        iEmpireKey, 
        SystemEmpireData::AlmonasterScore, 
        &vTemp
        );

    RETURN_ON_ERROR(iErrCode);

    *pfAlmonasterScore = vTemp.GetFloat();

    iErrCode = t_pCache->ReadData(
        strEmpire, 
        iEmpireKey, 
        SystemEmpireData::AlmonasterScoreSignificance, 
        &vTemp
        );

    RETURN_ON_ERROR(iErrCode);

    *piSignificance = vTemp.GetInteger();

    *piNumAllies = 0;

    iErrCode = t_pCache->GetEqualKeys(
        strDip,
        GameEmpireDiplomacy::CurrentStatus,
        ALLIANCE,
        NULL,
        &iEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        (*piNumAllies) += iEmpires;
    }

    iErrCode = t_pCache->GetEqualKeys(
        strDip,
        GameEmpireDiplomacy::CurrentStatus,
        TRADE,
        NULL,
        &iEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        (*piNumAllies) += iEmpires;
    }

    return iErrCode;
}

int AlmonasterScore::GetRelevantStatisticsFromPlanet (const char* pszGameMap, int iPlanetKey, float* pfScore,
                                                      int* piSignificance, int* piNumAllies, 
                                                      int* piLoserKey, int64* pi64EmpireSecretKey,
                                                      Variant* pvPlanetName) {

    int iErrCode;

    ICachedTable* pGameMap = NULL;
    AutoRelease<ICachedTable> release_pGameMap(pGameMap);

    // Get loser's score, significance and allies
    iErrCode = t_pCache->GetTable(pszGameMap, &pGameMap);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameMap->ReadData(iPlanetKey, GameMap::SurrenderAlmonasterScore, pfScore);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameMap->ReadData(iPlanetKey, GameMap::SurrenderAlmonasterSignificance, piSignificance);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameMap->ReadData(iPlanetKey, GameMap::SurrenderNumAllies, piNumAllies);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameMap->ReadData(iPlanetKey, GameMap::HomeWorld, piLoserKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameMap->ReadData(iPlanetKey, GameMap::SurrenderEmpireSecretKey, pi64EmpireSecretKey);
    RETURN_ON_ERROR(iErrCode);
    
    if (pvPlanetName)
    {
        iErrCode = pGameMap->ReadData(iPlanetKey, GameMap::Name, pvPlanetName);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int AlmonasterScore::HandleUncolonizedHomeWorldOnEndGame(int iGameClass, int iGameNumber, int iPlanetKey, 
                                                         Variant* pvEmpireKey, float* pfWinnerScore, 
                                                         int* piWinnerSignificance, int* piWinnerNumAllies, 
                                                         unsigned int iNumEmpires) {

    int iErrCode, iLoserSignificance, iLoserNumAllies, iLoserKey;

    unsigned int i;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    float* pfIncrease = (float*) StackAlloc (iNumEmpires * sizeof (float));
    float fLoserScore, fPartialDecrease, fDecrease = 0.0, fNumEmpires = (float) iNumEmpires;

    int64 i64EmpireSecretKey;

    bool bValid;

    Variant vPlanetName, vTemp, vNukerNewScore;
    ScoringChanges scChanges;

    // Get loser's stats
    iErrCode = GetRelevantStatisticsFromPlanet (
        strGameMap,
        iPlanetKey,
        &fLoserScore,
        &iLoserSignificance,
        &iLoserNumAllies,
        &iLoserKey,
        &i64EmpireSecretKey,
        &vPlanetName
        );

    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumEmpires; i ++) {

        GetAlmonasterScoreChanges (
            pfWinnerScore[i], 
            fLoserScore, 
            piWinnerSignificance[i],
            iLoserSignificance,
            piWinnerNumAllies[i], 
            iLoserNumAllies,
            pfIncrease + i, 
            &fPartialDecrease
            );

        fDecrease += fPartialDecrease;
    }

    // Give winners their proportional share of the loot
    for (i = 0; i < iNumEmpires; i ++)
    {
        GET_SYSTEM_EMPIRE_DATA(strThisEmpire, pvEmpireKey[i].GetInteger());

        // Increment winner's score and significance
        iErrCode = t_pCache->Increment(
            strThisEmpire, 
            pvEmpireKey[i].GetInteger(), 
            SystemEmpireData::AlmonasterScore, 
            pfIncrease[i] / fNumEmpires,
            &vNukerNewScore
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->Increment(
            strThisEmpire, 
            pvEmpireKey[i].GetInteger(), 
            SystemEmpireData::AlmonasterScoreSignificance, 
            1
            );
        
        RETURN_ON_ERROR(iErrCode);
    }

    // If empire is valid, decrement its score
    iErrCode = m_pGameEngine->CheckSecretKey (iLoserKey, i64EmpireSecretKey, &bValid, NULL, NULL);
    RETURN_ON_ERROR(iErrCode);

    if (!bValid) {

        iLoserKey = NO_KEY;

    } else {
        
        GET_SYSTEM_EMPIRE_DATA(strLoser, iLoserKey);

        // Give loser proportional loss also
        fDecrease /= fNumEmpires;
        
        iErrCode = t_pCache->Increment(strLoser, iLoserKey, SystemEmpireData::AlmonasterScore, - fDecrease, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        
        float fNewScore = vTemp.GetFloat();

        scChanges.iFlags |= ALMONASTER_NUKED_SCORE_CHANGE;
        scChanges.fAlmonasterNukedScore = fNewScore - fDecrease;
        scChanges.fAlmonasterNukedChange = - fDecrease;
        
        // Increment significance
        iErrCode = t_pCache->Increment(strLoser, iLoserKey, SystemEmpireData::AlmonasterScoreSignificance, 1);
        RETURN_ON_ERROR(iErrCode);

        if (iNumEmpires != 1)
        {
            // Send notification message
            char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
            iErrCode = m_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
            RETURN_ON_ERROR(iErrCode);

            char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 512];

            int iOptions;
            iErrCode = m_pGameEngine->GetEmpireOptions (iLoserKey, &iOptions);
            RETURN_ON_ERROR(iErrCode);

            if (iOptions & SEND_SCORE_MESSAGE_ON_NUKE)
            {
                if (fDecrease == 0) {

                    sprintf (
                        pszMessage,
                        "The ruins of your homeworld were not colonized in %s %i, which just ended. "\
                        "Your Almonaster score remained the same at %.3f",
                        pszGameClassName, iGameNumber, fNewScore
                        );

                } else {

                    sprintf (
                        pszMessage,
                        "The ruins of your homeworld were not colonized in %s %i, which just ended. "\
                        "Your Almonaster score decreased from "\
                        BEGIN_STRONG "%.3f" END_STRONG " to " BEGIN_STRONG "%f" END_STRONG,
                        pszGameClassName, iGameNumber, fNewScore - fDecrease, fNewScore
                        );
                }

            } else {

                sprintf (
                    pszMessage,
                    "The ruins of your homeworld were not colonized in %s %i, which just ended.",
                    pszGameClassName, iGameNumber
                    );
            }

            iErrCode = m_pGameEngine->SendSystemMessage (iLoserKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (iNumEmpires == 1)
    {
        // Give surviving empire credit for nuke
        // This should be done elsewhere, but for perf we do it here

        char pszNukedName [MAX_EMPIRE_NAME_LENGTH + 1];
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];

        int iNukerEmpireKey = pvEmpireKey[0].GetInteger();
        GET_SYSTEM_EMPIRE_DATA(strNukerEmpire, iNukerEmpireKey);

        scChanges.iFlags |= ALMONASTER_NUKER_SCORE_CHANGE;
        scChanges.fAlmonasterNukerScore = vNukerNewScore.GetFloat() - pfIncrease[0];
        scChanges.fAlmonasterNukerChange = pfIncrease[0];

        Variant vNukerName;
        iErrCode = t_pCache->ReadData(strNukerEmpire, iNukerEmpireKey, SystemEmpireData::Name, &vNukerName);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->Increment(strNukerEmpire, iNukerEmpireKey, SystemEmpireData::Nukes, 1);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = m_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
        RETURN_ON_ERROR(iErrCode);

        // Parse out nuked empire's name
        int scanf = sscanf (vPlanetName.GetCharPtr(), RUINS_OF, pszNukedName);
        Assert(scanf == 1);
        scChanges.pszNukedName = pszNukedName;

        int iNukerAlienKey, iNukedAlienKey;

        // Get nuker alien key
        iErrCode = t_pCache->ReadData(strNukerEmpire, iNukerEmpireKey, SystemEmpireData::AlienKey, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iNukerAlienKey = vTemp.GetInteger();

        // Get nuked alien key
        if (iLoserKey == NO_KEY) {

            iNukedAlienKey = NO_KEY;
        
        } else {

            GET_SYSTEM_EMPIRE_DATA(strLoserEmpire, iLoserKey);

            iErrCode = t_pCache->ReadData(strLoserEmpire, iLoserKey, SystemEmpireData::AlienKey, &vTemp);            
            RETURN_ON_ERROR(iErrCode);
            iNukedAlienKey = vTemp.GetInteger();

            // Add to nuked's nuke history
            iErrCode = m_pGameEngine->AddNukeToHistory (
                NUKER_LIST,
                pszGameClassName,
                iGameNumber,
                iLoserKey,
                NULL,
                NO_KEY,
                iNukerEmpireKey,
                vNukerName.GetCharPtr(),
                iNukerAlienKey
                );

            RETURN_ON_ERROR(iErrCode);

            // Send victory sneer
            iErrCode = m_pGameEngine->SendVictorySneer (iNukerEmpireKey, vNukerName.GetCharPtr(), iLoserKey);
            RETURN_ON_ERROR(iErrCode);
        }

        // Notify classic score
        ClassicScore classicScore(m_pGameEngine);

        // We ignore scoring changes when accounting this nuke
        iErrCode = classicScore.OnNuke (iGameClass, iGameNumber, iNukerEmpireKey, NO_KEY, &scChanges);
        RETURN_ON_ERROR(iErrCode);

        // Add to nuker's nuke history
        iErrCode = m_pGameEngine->AddNukeToHistory (
            NUKED_LIST,
            pszGameClassName,
            iGameNumber,
            iNukerEmpireKey,
            NULL,
            NO_KEY,
            iLoserKey,
            pszNukedName,
            iNukedAlienKey
            );

        RETURN_ON_ERROR(iErrCode);

        // Update system nuke list
        iErrCode = m_pGameEngine->AddNukeToHistory (
            SYSTEM_LIST,
            pszGameClassName,
            iGameNumber,
            iNukerEmpireKey,
            vNukerName.GetCharPtr(),
            iNukerAlienKey,
            iLoserKey,
            pszNukedName,
            iNukedAlienKey
            );

        RETURN_ON_ERROR(iErrCode);

        // Send nuke messages
        int iNumUpdates;
        iErrCode = m_pGameEngine->GetNumUpdates (iGameClass, iGameNumber, &iNumUpdates);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = m_pGameEngine->SendScoringChangeMessages(iGameClass, iGameNumber, iNukerEmpireKey, iLoserKey, iNumUpdates, EMPIRE_SURRENDERED, pszGameClassName, &scChanges);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

bool AlmonasterScore::IsValidScore (const Variant* pvScore) {

    float fScore = pvScore->GetFloat();
    return fScore >= CLASSIC_MIN_SCORE && fScore <= CLASSIC_MAX_SCORE;
}

int AlmonasterScore::CompareScores (const Variant* pvLeft, const Variant* pvRight) {

    float fLeft = pvLeft->GetFloat();
    float fRight = pvRight->GetFloat();

    if (fLeft < fRight) {
        return -1;
    }

    if (fLeft == fRight) {
        return 0;
    }

    return 1;
}

int AlmonasterScore::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);
    return t_pCache->ReadData(strEmpires, iEmpireKey, SystemEmpireData::AlmonasterScore, pvScore);
}