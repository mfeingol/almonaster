// AlmonasterScore.cpp: implementation of the AlmonasterScore class.
//
//////////////////////////////////////////////////////////////////////

#include "AlmonasterScore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AlmonasterScore::AlmonasterScore (IGameEngine* pGameEngine)
{
    m_iNumRefs = 1;

    Assert (pGameEngine != NULL);

    m_pGameEngine = pGameEngine; // Weak ref
    
    IDatabase* pDatabase = m_pGameEngine->GetDatabase(); // AddRef()
    Assert (pDatabase != NULL);

    t_pConn = pDatabase->CreateConnection();
    Assert (t_pConn != NULL);

    SafeRelease(pDatabase);
}

AlmonasterScore::~AlmonasterScore()
{
    SafeRelease(t_pConn);
}

IScoringSystem* AlmonasterScore::CreateInstance (IGameEngine* pGameEngine) {

    return new AlmonasterScore (pGameEngine);
}

bool AlmonasterScore::HasTopList() {
    return true;
}

int AlmonasterScore::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) {

    float fNukerScore, fNukedScore, fIncrease, fDecrease;
    
    int iErrCode, iNukerSignificance, iNukedSignificance, iNukerAllies, iNukedAllies;

    GAME_EMPIRE_DIPLOMACY (strNukerDip, iGameClass, iGameNumber, iEmpireNuker);
    GAME_EMPIRE_DIPLOMACY (strNukedDip, iGameClass, iGameNumber, iEmpireNuked);
    
    // Get empire statistics
    iErrCode = GetRelevantStatistics (
        iGameClass,
        iGameNumber,
        iEmpireNuker,
        &fNukerScore,
        &iNukerSignificance,
        &iNukerAllies
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = GetRelevantStatistics (
        iGameClass,
        iGameNumber,
        iEmpireNuked,
        &fNukedScore,
        &iNukedSignificance,
        &iNukedAllies
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

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

    iErrCode = t_pConn->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireNuked, 
        SystemEmpireData::AlmonasterScore, 
        - fDecrease
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = t_pConn->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireNuker, 
        SystemEmpireData::AlmonasterScore, 
        fIncrease
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Increment significance
    iErrCode = t_pConn->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireNuker, 
        SystemEmpireData::AlmonasterScoreSignificance, 
        1
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = t_pConn->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireNuked, 
        SystemEmpireData::AlmonasterScoreSignificance, 
        1
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Update top lists after score changes
    iErrCode = m_pGameEngine->UpdateTopListOnDecrease (ALMONASTER_SCORE, iEmpireNuked);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameEngine->UpdateTopListOnIncrease (ALMONASTER_SCORE, iEmpireNuker);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Adjust privileges after score changes
    iErrCode = m_pGameEngine->CalculatePrivilegeLevel (iEmpireNuker);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameEngine->CalculatePrivilegeLevel (iEmpireNuked);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Do accounting
    pscChanges->iFlags |= ALMONASTER_NUKER_SCORE_CHANGE | ALMONASTER_NUKED_SCORE_CHANGE;
    pscChanges->fAlmonasterNukerScore = fNukerScore;
    pscChanges->fAlmonasterNukerChange = fIncrease;
    pscChanges->fAlmonasterNukedScore = fNukedScore;
    pscChanges->fAlmonasterNukedChange = - fDecrease;

    return iErrCode;
}

int AlmonasterScore::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges) {

    // Same as nuke
    return OnNuke (iGameClass, iGameNumber, iWinner, iLoser, pscChanges);
}

int AlmonasterScore::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) {

    // We need to wait for a colonizer or the game's end to give out credit and punishment,
    // so we don't do anything right now

    return OK;
}

int AlmonasterScore::On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, 
                                                     int iPlanetKey, ScoringChanges* pscChanges) {
    NamedMutex nmEmpireLock;

    bool bLocked = false, bValid;

    float fWinnerScore, fLoserScore, fWinnerIncrease, fLoserDecrease;
    int iErrCode, iLoserSignificance, iLoserNumAllies, iWinnerNumAllies, iWinnerSignificance, iLoserKey;

    int64 i64EmpireSecretKey;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRE_DIPLOMACY (strWinnerDip, iGameClass, iGameNumber, iWinnerKey);

    iErrCode = GetRelevantStatistics (
        iGameClass,
        iGameNumber,
        iWinnerKey,
        &fWinnerScore,
        &iWinnerSignificance,
        &iWinnerNumAllies
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

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

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

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
    iErrCode = t_pConn->Increment (
        SYSTEM_EMPIRE_DATA, 
        iWinnerKey, 
        SystemEmpireData::AlmonasterScore, 
        fWinnerIncrease
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->Increment (
        SYSTEM_EMPIRE_DATA, 
        iWinnerKey, 
        SystemEmpireData::AlmonasterScoreSignificance, 
        1
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameEngine->UpdateTopListOnIncrease (ALMONASTER_SCORE, iWinnerKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Do accounting for winner
    pscChanges->iFlags |= ALMONASTER_NUKER_SCORE_CHANGE;
    pscChanges->fAlmonasterNukerScore = fWinnerScore;
    pscChanges->fAlmonasterNukerChange = fWinnerIncrease;

    // Try to find the loser
    // This doesn't really belong here, but it's a convenient place to do it for perf
    iErrCode = m_pGameEngine->LockEmpire (iLoserKey, &nmEmpireLock);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    bLocked = true;

    iErrCode = m_pGameEngine->CheckSecretKey (iLoserKey, i64EmpireSecretKey, &bValid, NULL, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (bValid) {

        iErrCode = t_pConn->Increment (
            SYSTEM_EMPIRE_DATA, 
            iLoserKey, 
            SystemEmpireData::AlmonasterScore, 
            - fLoserDecrease
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = t_pConn->Increment (
            SYSTEM_EMPIRE_DATA, 
            iLoserKey, 
            SystemEmpireData::AlmonasterScoreSignificance, 
            1
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Update top lists after score change
        iErrCode = m_pGameEngine->UpdateTopListOnDecrease (ALMONASTER_SCORE, iLoserKey);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Do accounting for loser
        pscChanges->iFlags |= ALMONASTER_NUKED_SCORE_CHANGE;
        pscChanges->fAlmonasterNukedScore = fLoserScore;
        pscChanges->fAlmonasterNukedChange = - fLoserDecrease;
    }
    
Cleanup:

    if (bLocked) {
        m_pGameEngine->UnlockEmpire (nmEmpireLock);
    }

    return iErrCode;
}


int AlmonasterScore::OnGameEnd (int iGameClass, int iGameNumber) {

    int iErrCode;
    Variant vOptions, * pvHomeWorld = NULL, * pvEmpireKey = NULL;

    float* pfWinnerScore = NULL;
    int* piWinnerNumAllies = NULL, * piWinnerSignificance = NULL;

    unsigned int i, j, iNumPlanets, * piPlanetKey = NULL, iNumEmpires = 0;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);
    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    // See if game has 3.0 surrenders flag set
    iErrCode = t_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(vOptions.GetInteger() & USE_SC30_SURRENDERS)) {
        goto Cleanup;
    }

    // Need to scan map for uncolonized homeworlds
    iErrCode = t_pConn->ReadColumn (
        strGameMap,
        GameMap::HomeWorld,
        &piPlanetKey,
        &pvHomeWorld,
        &iNumPlanets
        );

    if (iErrCode != OK) {

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            goto Cleanup;
        }

        Assert (false);
        goto Cleanup;
    }

    for (i = 0; i < iNumPlanets; i ++) {

        if (pvHomeWorld[i].GetInteger() != HOMEWORLD &&
            pvHomeWorld[i].GetInteger() != NOT_HOMEWORLD) {

            // Fault in survivors and their stats
            if (pvEmpireKey == NULL) {              

                iErrCode = t_pConn->ReadColumn (
                    strGameEmpires,
                    GameEmpires::EmpireKey,
                    NULL,
                    &pvEmpireKey,
                    &iNumEmpires
                    );

                // There may or may not be empires left...
                if (iErrCode != OK) {

                    if (iErrCode == ERROR_DATA_NOT_FOUND) {
                        iErrCode = OK;
                        continue;
                    }

                    Assert (false);
                    goto Cleanup;
                }

                pfWinnerScore = (float*) StackAlloc (iNumEmpires * sizeof (float));
                piWinnerSignificance = (int*) StackAlloc (2 * iNumEmpires * sizeof (int));
                piWinnerNumAllies = piWinnerSignificance + iNumEmpires;
                
                for (j = 0; j < iNumEmpires; j ++) {
                    
                    iErrCode = GetRelevantStatistics (
                        iGameClass, 
                        iGameNumber, 
                        pvEmpireKey[j].GetInteger(),
                        pfWinnerScore + j,
                        piWinnerSignificance + j,
                        piWinnerNumAllies + j
                        );
                
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
            }

            // Handle all necessary processing
            Assert (pvEmpireKey != NULL);

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

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    if (pvEmpireKey != NULL) {

        // This means we incremented some scores.  Notify the top lists

        for (i = 0; i < iNumEmpires; i ++) {

            iErrCode = m_pGameEngine->UpdateTopListOnIncrease (ALMONASTER_SCORE, pvEmpireKey[i].GetInteger());
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:

    if (piPlanetKey != NULL) {
        t_pConn->FreeKeys (piPlanetKey);
    }

    if (pvHomeWorld != NULL) {
        t_pConn->FreeData (pvHomeWorld);
    }

    if (pvEmpireKey != NULL) {
        t_pConn->FreeData (pvEmpireKey);
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
    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = t_pConn->ReadData (
        SYSTEM_EMPIRE_DATA,
        iEmpireKey,
        SystemEmpireData::AlmonasterScore, 
        &vScore
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = t_pConn->ReadData (
        SYSTEM_EMPIRE_DATA,
        iEmpireKey,
        SystemEmpireData::AlmonasterScoreSignificance, 
        &vSignificance
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    while (true) {

        iErrCode = t_pConn->GetNextKey (pszEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = t_pConn->ReadData (pszEmpires, iKey, GameEmpires::EmpireKey, &vKey);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = t_pConn->ReadData (
            SYSTEM_EMPIRE_DATA,
            vKey.GetInteger(),
            SystemEmpireData::AlmonasterScore, 
            &vEmpScore
            );
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = t_pConn->ReadData (
            SYSTEM_EMPIRE_DATA,
            vKey.GetInteger(),
            SystemEmpireData::AlmonasterScoreSignificance, 
            &vEmpSignificance
            );
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

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
        Assert (fNukedDecrease >= (float) 0.0);

        if (fNukedDecrease > fMaxNukedDecrease) {
            fMaxNukedDecrease = fNukedDecrease;
        }
    }

    Assert (fMaxNukedDecrease >= 0);

    if (fMaxNukedDecrease > 0) {

        iErrCode = t_pConn->Increment (
            SYSTEM_EMPIRE_DATA,
            iEmpireKey,
            SystemEmpireData::AlmonasterScore, 
            - fMaxNukedDecrease
            );

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        iErrCode = m_pGameEngine->UpdateTopListOnDecrease (ALMONASTER_SCORE, iEmpireKey);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    return iErrCode;
}

void AlmonasterScore::GetAlmonasterScoreChanges (float fNukerScore, float fNukedScore, int iNukerSignificance,
                                                 int iNukedSignificance, int iNumNukerAllies, 
                                                 int iNumNukedAllies, float* pfNukerIncrease, 
                                                 float* pfNukedDecrease) {

    Assert (fNukerScore >= ALMONASTER_MIN_SCORE && fNukedScore <= ALMONASTER_MAX_SCORE);
    Assert (fNukedScore >= ALMONASTER_MIN_SCORE && fNukedScore <= ALMONASTER_MAX_SCORE);
    Assert (iNukerSignificance >= 0);
    Assert (iNukedSignificance >= 0);
    Assert (iNumNukerAllies >= 0);
    Assert (iNumNukedAllies >= 0);

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

    Assert (*pfNukedDecrease >= (float) 0.0 && *pfNukerIncrease > (float) 0.0);
}

int AlmonasterScore::GetRelevantStatistics (int iGameClass, int iGameNumber, int iEmpireKey,
                                            float* pfAlmonasterScore, int* piSignificance, 
                                            int* piNumAllies) {

    int iErrCode;

    Variant vTemp;

    unsigned int iEmpires;

    GAME_EMPIRE_DIPLOMACY (strDip, iGameClass, iGameNumber, iEmpireKey);

    // Get winner's score, significance and allies
    iErrCode = t_pConn->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::AlmonasterScore, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    *pfAlmonasterScore = vTemp.GetFloat();

    iErrCode = t_pConn->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::AlmonasterScoreSignificance, 
        &vTemp
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    *piSignificance = vTemp.GetInteger();

    *piNumAllies = 0;

    iErrCode = t_pConn->GetEqualKeys (
        strDip,
        GameEmpireDiplomacy::CurrentStatus,
        ALLIANCE,
        NULL,
        &iEmpires
        );

    if (iErrCode == OK) {
        (*piNumAllies) += iEmpires;
    }

    else if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    
    else {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = t_pConn->GetEqualKeys (
        strDip,
        GameEmpireDiplomacy::CurrentStatus,
        TRADE,
        NULL,
        &iEmpires
        );

    if (iErrCode == OK) {
        (*piNumAllies) += iEmpires;
    }

    else if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    else {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    return iErrCode;
}

int AlmonasterScore::GetRelevantStatisticsFromPlanet (const char* pszGameMap, int iPlanetKey, float* pfScore,
                                                      int* piSignificance, int* piNumAllies, 
                                                      int* piLoserKey, int64* pi64EmpireSecretKey,
                                                      Variant* pvPlanetName) {

    int iErrCode;

    IReadTable* pGameMap = NULL;

    // Get loser's score, significance and allies
    iErrCode = t_pConn->GetTableForReading (pszGameMap, &pGameMap);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameMap->ReadData (iPlanetKey, GameMap::SurrenderAlmonasterScore, pfScore);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameMap->ReadData (iPlanetKey, GameMap::SurrenderAlmonasterSignificance, piSignificance);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameMap->ReadData (iPlanetKey, GameMap::SurrenderNumAllies, piNumAllies);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameMap->ReadData (iPlanetKey, GameMap::HomeWorld, piLoserKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameMap->ReadData (iPlanetKey, GameMap::SurrenderEmpireSecretKey, pi64EmpireSecretKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (pvPlanetName != NULL) {
        
        iErrCode = pGameMap->ReadData (iPlanetKey, GameMap::Name, pvPlanetName);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    pGameMap->Release();

    return iErrCode;

Cleanup:

    if (pGameMap != NULL) {
        pGameMap->Release();
    }

    return iErrCode;
}

int AlmonasterScore::HandleUncolonizedHomeWorldOnEndGame (int iGameClass, int iGameNumber, int iPlanetKey, 
                                                          Variant* pvEmpireKey, float* pfWinnerScore, 
                                                          int* piWinnerSignificance, int* piWinnerNumAllies, 
                                                          unsigned int iNumEmpires) {

    int iErrCode, iLoserSignificance, iLoserNumAllies, iLoserKey;

    unsigned int i;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    float* pfIncrease = (float*) StackAlloc (iNumEmpires * sizeof (float));
    float fLoserScore, fPartialDecrease, fDecrease = 0.0, fNumEmpires = (float) iNumEmpires;

    int64 i64EmpireSecretKey;

    bool bEmpireLocked = false, bValid;

    Variant vPlanetName, vTemp, vNukerNewScore;
    IScoringSystem* pClassicScore = NULL;
    ScoringChanges scChanges;

    NamedMutex nmEmpireLock;

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

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

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
    for (i = 0; i < iNumEmpires; i ++) {

        // Increment winner's score and significance
        iErrCode = t_pConn->Increment (
            SYSTEM_EMPIRE_DATA, 
            pvEmpireKey[i].GetInteger(), 
            SystemEmpireData::AlmonasterScore, 
            pfIncrease[i] / fNumEmpires,
            &vNukerNewScore
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = t_pConn->Increment (
            SYSTEM_EMPIRE_DATA, 
            pvEmpireKey[i].GetInteger(), 
            SystemEmpireData::AlmonasterScoreSignificance, 
            1
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    iErrCode = m_pGameEngine->LockEmpire (iLoserKey, &nmEmpireLock);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    bEmpireLocked = true;
    
    // If empire is valid, decrement its score
    iErrCode = m_pGameEngine->CheckSecretKey (iLoserKey, i64EmpireSecretKey, &bValid, NULL, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!bValid) {

        iLoserKey = NO_KEY;

    } else {
        
        // Give loser proportional loss also
        fDecrease /= fNumEmpires;
        
        iErrCode = t_pConn->Increment (
            SYSTEM_EMPIRE_DATA, 
            iLoserKey, 
            SystemEmpireData::AlmonasterScore, 
            - fDecrease,
            &vTemp
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        float fNewScore = vTemp.GetFloat();

        scChanges.iFlags |= ALMONASTER_NUKED_SCORE_CHANGE;
        scChanges.fAlmonasterNukedScore = fNewScore - fDecrease;
        scChanges.fAlmonasterNukedChange = - fDecrease;
        
        // Increment significance
        iErrCode = t_pConn->Increment (
            SYSTEM_EMPIRE_DATA, 
            iLoserKey, 
            SystemEmpireData::AlmonasterScoreSignificance, 
            1
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Notify top list
        iErrCode = m_pGameEngine->UpdateTopListOnDecrease (ALMONASTER_SCORE, iLoserKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (iNumEmpires != 1) {

            // Send notification message
            char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

            if (m_pGameEngine->GetGameClassName (iGameClass, pszGameClassName) == OK) {

                char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 512];

                int iOptions;
                if (m_pGameEngine->GetEmpireOptions (iLoserKey, &iOptions) == OK) {

                    if (iOptions & SEND_SCORE_MESSAGE_ON_NUKE) {

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

                    m_pGameEngine->SendSystemMessage (iLoserKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
                }
            }
        }
    }

    if (iNumEmpires == 1) {

        // Give surviving empire credit for nuke
        // This should be done elsewhere, but for perf we do it here

        char pszNukedName [MAX_EMPIRE_NAME_LENGTH + 1];
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];

        int iNukerEmpireKey = pvEmpireKey[0].GetInteger();

        scChanges.iFlags |= ALMONASTER_NUKER_SCORE_CHANGE;
        scChanges.fAlmonasterNukerScore = vNukerNewScore.GetFloat() - pfIncrease[0];
        scChanges.fAlmonasterNukerChange = pfIncrease[0];

        Variant vNukerName;
        iErrCode = t_pConn->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iNukerEmpireKey, 
            SystemEmpireData::Name, 
            &vNukerName
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = t_pConn->Increment (SYSTEM_EMPIRE_DATA, iNukerEmpireKey, SystemEmpireData::Nukes, 1);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Parse out nuked empire's name
        int scanf = sscanf (vPlanetName.GetCharPtr(), RUINS_OF, pszNukedName);
        Assert (scanf == 1);
        scChanges.pszNukedName = pszNukedName;

        int iNukerAlienKey, iNukedAlienKey;

        // Get nuker alien key
        iErrCode = t_pConn->ReadData (SYSTEM_EMPIRE_DATA, iNukerEmpireKey, SystemEmpireData::AlienKey, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iNukerAlienKey = vTemp.GetInteger();

        // Get nuked alien key
        if (iLoserKey == NO_KEY) {

            iNukedAlienKey = NO_KEY;
        
        } else {

            iErrCode = t_pConn->ReadData (SYSTEM_EMPIRE_DATA, iLoserKey, SystemEmpireData::AlienKey, &vTemp);            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
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

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Send victory sneer
            iErrCode = m_pGameEngine->SendVictorySneer (iNukerEmpireKey, vNukerName.GetCharPtr(), iLoserKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        // Notify classic score
        pClassicScore = m_pGameEngine->GetScoringSystem (CLASSIC_SCORE);
        Assert (pClassicScore != NULL);

        // We ignore scoring changes when accounting this nuke
        iErrCode = pClassicScore->OnNuke (iGameClass, iGameNumber, iNukerEmpireKey, NO_KEY, &scChanges);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

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

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

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

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Send nuke messages
        int iNumUpdates;
        iErrCode = m_pGameEngine->GetNumUpdates (iGameClass, iGameNumber, &iNumUpdates);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pGameEngine->SendScoringChangeMessages (
            iGameClass, iGameNumber, iNukerEmpireKey, iLoserKey, iNumUpdates, EMPIRE_SURRENDERED, pszGameClassName, &scChanges);

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    if (bEmpireLocked) {
        m_pGameEngine->UnlockEmpire (nmEmpireLock);
    }

    if (pClassicScore != NULL) {
        pClassicScore->Release();
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

int AlmonasterScore::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) {

    return t_pConn->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, pvScore);
}

int AlmonasterScore::GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires) {

    if (pvScore == NULL) {
        return t_pConn->GetAllKeys (SYSTEM_EMPIRE_DATA, ppiKey, piNumEmpires);
    }

    SearchColumn sc;
    sc.pszColumn = SystemEmpireData::AlmonasterScore;
    sc.iFlags = 0;
    sc.vData = *pvScore;
    sc.vData2 = ALMONASTER_MAX_SCORE;

    SearchDefinition sd;
    sd.iMaxNumHits = 0;
    sd.iSkipHits = 0;
    sd.iStartKey = NO_KEY;
    sd.iNumColumns = 1;
    sd.pscColumns = &sc;

    return t_pConn->GetSearchKeys (
        SYSTEM_EMPIRE_DATA,
        sd,
        ppiKey, 
        piNumEmpires, 
        NULL
        );
}