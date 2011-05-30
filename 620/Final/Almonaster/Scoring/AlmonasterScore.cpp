// AlmonasterScore.cpp: implementation of the AlmonasterScore class.
//
//////////////////////////////////////////////////////////////////////

#include "AlmonasterScore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

AlmonasterScore::AlmonasterScore (IGameEngine* pGameEngine) {

    m_iNumRefs = 1;

    Assert (pGameEngine != NULL);

    m_pGameEngine = pGameEngine; // Weak ref
    m_pDatabase = m_pGameEngine->GetDatabase(); // AddRef()

    Assert (m_pDatabase != NULL);
}

AlmonasterScore::~AlmonasterScore() {

    if (m_pDatabase != NULL) {
        m_pDatabase->Release();
    }
}

IScoringSystem* AlmonasterScore::CreateInstance (IGameEngine* pGameEngine) {

    return new AlmonasterScore (pGameEngine);
}

bool AlmonasterScore::HasTopList() {
    return true;
}

int AlmonasterScore::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked) {

    float fNukerScore, fNukedScore, fIncrease, fDecrease;
    
    int iErrCode, iNukerSignificance, iNukedSignificance, iNukerAllies, iNukedAllies, iOptions;

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

    iErrCode = m_pDatabase->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireNuked, 
        SystemEmpireData::AlmonasterScore, 
        - fDecrease
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pDatabase->Increment (
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
    iErrCode = m_pDatabase->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireNuker, 
        SystemEmpireData::AlmonasterScoreSignificance, 
        1
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pDatabase->Increment (
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
    iErrCode = m_pGameEngine->UpdateTopListOnDecrease (
        ALMONASTER_SCORE, 
        iEmpireNuked
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameEngine->UpdateTopListOnIncrease (
        ALMONASTER_SCORE, 
        iEmpireNuker
        );

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

    // Send messages?
    iErrCode = m_pGameEngine->GetEmpireOptions (iEmpireNuker, &iOptions);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (iOptions & SEND_SCORE_MESSAGE_ON_NUKE) {

        iErrCode = SendScoreChangeMessage (
            NUKER_LIST,
            iGameClass,
            iGameNumber,
            iEmpireNuker,
            iEmpireNuked,
            fNukerScore,
            fNukedScore,
            fIncrease,
            fDecrease
            );

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    iErrCode = m_pGameEngine->GetEmpireOptions (iEmpireNuked, &iOptions);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (iOptions & SEND_SCORE_MESSAGE_ON_NUKE) {

        iErrCode = SendScoreChangeMessage (
            NUKED_LIST,
            iGameClass,
            iGameNumber,
            iEmpireNuker,
            iEmpireNuked,
            fNukerScore,
            fNukedScore,
            fIncrease,
            fDecrease
            );

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    return iErrCode;
}

int AlmonasterScore::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser) {

    // Same as nuke
    return OnNuke (iGameClass, iGameNumber, iWinner, iLoser);
}

int AlmonasterScore::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser) {

    // We need to wait for a colonizer or the game's end to give out credit and punishment,
    // so we don't do anything right now

    return OK;
}

int AlmonasterScore::On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, 
                                                     int iPlanetKey) {
    NamedMutex nmEmpireLock;

    bool bLocked = false;

    float fWinnerScore, fLoserScore, fWinnerIncrease, fLoserDecrease;
    int iErrCode, iLoserSignificance, iLoserNumAllies, iWinnerNumAllies, iWinnerSignificance, iLoserKey;

    unsigned int iHashEmpireName;

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
        &iHashEmpireName,
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
    iErrCode = m_pDatabase->Increment (
        SYSTEM_EMPIRE_DATA, 
        iWinnerKey, 
        SystemEmpireData::AlmonasterScore, 
        fWinnerIncrease
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pDatabase->Increment (
        SYSTEM_EMPIRE_DATA, 
        iWinnerKey, 
        SystemEmpireData::AlmonasterScoreSignificance, 
        1
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameEngine->UpdateTopListOnIncrease (
        ALMONASTER_SCORE, 
        iWinnerKey
        );
    
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Try to find the loser
    // This doesn't really belong here, but it's a convenient place to do it for perf
    m_pGameEngine->LockEmpire (iLoserKey, &nmEmpireLock);
    bLocked = true;

    if (m_pGameEngine->ValidateEmpireKey (iLoserKey, iHashEmpireName)) {
        
        // Close enough!  Let's tag him
        iErrCode = m_pDatabase->Increment (
            SYSTEM_EMPIRE_DATA, 
            iLoserKey, 
            SystemEmpireData::AlmonasterScore, 
            - fLoserDecrease
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pDatabase->Increment (
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
        iErrCode = m_pGameEngine->UpdateTopListOnDecrease (
            ALMONASTER_SCORE, 
            iLoserKey
            );
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        // Send 'sorry' message
        Variant vEmpireName;
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
        
        if (m_pGameEngine->GetGameClassName (iGameClass, pszGameClassName) == OK &&
            m_pGameEngine->GetEmpireName (iWinnerKey, &vEmpireName) == OK) {
            
            char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH + 256];
            
            sprintf (
                pszMessage,
                "The ruins of your homeworld were colonized by %s in %s %i. Your scores "\
                "have been adjusted accordingly",
                vEmpireName.GetCharPtr(),
                pszGameClassName,
                iGameNumber
                );
            
            m_pGameEngine->SendSystemMessage (iLoserKey, pszMessage, SYSTEM);
        }
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
    iErrCode = m_pDatabase->ReadData (
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
    iErrCode = m_pDatabase->ReadColumn (
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

        if (pvHomeWorld[i].GetInteger() >= ROOT_KEY) {

            // Fault in survivors and their stats
            if (pvEmpireKey == NULL) {              

                iErrCode = m_pDatabase->ReadColumn (
                    strGameEmpires,
                    GameEmpires::EmpireKey,
                    NULL,
                    &pvEmpireKey,
                    &iNumEmpires
                    );

                // Must be at least one empire left...
                if (iErrCode != OK) {
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

            iErrCode = m_pGameEngine->UpdateTopListOnIncrease (
                ALMONASTER_SCORE, 
                pvEmpireKey[i].GetInteger()
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:

    if (piPlanetKey != NULL) {
        m_pDatabase->FreeKeys (piPlanetKey);
    }

    if (pvHomeWorld != NULL) {
        m_pDatabase->FreeData (pvHomeWorld);
    }

    if (pvEmpireKey != NULL) {
        m_pDatabase->FreeData (pvEmpireKey);
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

    iErrCode = m_pDatabase->ReadData (
        SYSTEM_EMPIRE_DATA,
        iEmpireKey,
        SystemEmpireData::AlmonasterScore, 
        &vScore
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pDatabase->ReadData (
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

        iErrCode = m_pDatabase->GetNextKey (pszEmpires, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pDatabase->ReadData (pszEmpires, iKey, GameEmpires::EmpireKey, &vKey);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pDatabase->ReadData (
            SYSTEM_EMPIRE_DATA,
            vKey.GetInteger(),
            SystemEmpireData::AlmonasterScore, 
            &vEmpScore
            );
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        iErrCode = m_pDatabase->ReadData (
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

        iErrCode = m_pDatabase->Increment (
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
    iErrCode = m_pDatabase->ReadData (
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

    iErrCode = m_pDatabase->ReadData (
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

    iErrCode = m_pDatabase->GetEqualKeys (
        strDip,
        GameEmpireDiplomacy::CurrentStatus,
        ALLIANCE,
        false,
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

    iErrCode = m_pDatabase->GetEqualKeys (
        strDip,
        GameEmpireDiplomacy::CurrentStatus,
        TRADE,
        false,
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
                                                      int* piLoserKey, unsigned int* piHashEmpireName,
                                                      Variant* pvPlanetName) {

    int iErrCode;

    IReadTable* pGameMap = NULL;

    // Get loser's score, significance and allies
    iErrCode = m_pDatabase->GetTableForReading (pszGameMap, &pGameMap);
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

    iErrCode = pGameMap->ReadData (iPlanetKey, GameMap::SurrenderEmpireNameHash, (int*) piHashEmpireName);
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

    int iErrCode;

    unsigned int iHashEmpireName, i;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    float* pfIncrease = (float*) StackAlloc (iNumEmpires * sizeof (float));

    float fLoserScore, fPartialDecrease, fDecrease = 0.0, fNumEmpires = (float) iNumEmpires;
    int iLoserSignificance, iLoserNumAllies, iLoserKey;

    bool bEmpireLocked = false;

    Variant vPlanetName;

    IScoringSystem* pClassicScore = NULL;

    NamedMutex nmEmpireLock;

    // Get loser's stats
    iErrCode = GetRelevantStatisticsFromPlanet (
        strGameMap,
        iPlanetKey,
        &fLoserScore,
        &iLoserSignificance,
        &iLoserNumAllies,
        &iLoserKey,
        &iHashEmpireName,
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
        iErrCode = m_pDatabase->Increment (
            SYSTEM_EMPIRE_DATA, 
            pvEmpireKey[i].GetInteger(), 
            SystemEmpireData::AlmonasterScore, 
            pfIncrease[i] / fNumEmpires
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = m_pDatabase->Increment (
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

    m_pGameEngine->LockEmpire (iLoserKey, &nmEmpireLock);
    bEmpireLocked = true;
    
    // If empire is valid, decrement its score
    if (!m_pGameEngine->ValidateEmpireKey (iLoserKey, iHashEmpireName)) {

        iLoserKey = NO_KEY;

    } else {
        
        // Give loser proportional loss also
        fDecrease /= fNumEmpires;
        
        iErrCode = m_pDatabase->Increment (
            SYSTEM_EMPIRE_DATA, 
            iLoserKey, 
            SystemEmpireData::AlmonasterScore, 
            - fDecrease
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Increment significance
        iErrCode = m_pDatabase->Increment (
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
        iErrCode = m_pGameEngine->UpdateTopListOnDecrease (
            ALMONASTER_SCORE, 
            iLoserKey
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Send 'sorry' message
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
        
        if (m_pGameEngine->GetGameClassName (iGameClass, pszGameClassName) == OK) {
            
            char pszMessage [MAX_FULL_GAME_CLASS_NAME_LENGTH + 256];
            
            sprintf (
                pszMessage,
                "The ruins of your homeworld were not colonized in %s %i, which just ended. "\
                "Your scores have been adjusted accordingly",
                pszGameClassName,
                iGameNumber
                );

            m_pGameEngine->SendSystemMessage (iLoserKey, pszMessage, SYSTEM);
        }
    }

    if (iNumEmpires == 1) {

        // Give surviving empire credit for nuke
        // This should be done elsewhere, but for perf we do it here

        char pszNukedName [MAX_EMPIRE_NAME_LENGTH + 1];
        char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];

        int iNukerEmpireKey = pvEmpireKey[0].GetInteger();

        Variant vNukerName;

        iErrCode = m_pDatabase->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iNukerEmpireKey, 
            SystemEmpireData::Name, 
            &vNukerName
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = m_pDatabase->Increment (
            SYSTEM_EMPIRE_DATA, 
            iNukerEmpireKey, 
            SystemEmpireData::Nukes, 
            1
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Parse out empire's name
        if (sscanf (vPlanetName.GetCharPtr(), RUINS_OF, pszNukedName) == 1 &&
            m_pGameEngine->GetGameClassName (iGameClass, pszGameClassName) == OK) {
            
            Variant vValue;
            int iNukerAlienKey, iNukedAlienKey;
            
            // Get nuker alien key
            iErrCode = m_pDatabase->ReadData (
                SYSTEM_EMPIRE_DATA, 
                iNukerEmpireKey, 
                SystemEmpireData::AlienKey, 
                &vValue
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iNukerAlienKey = vValue.GetInteger();
            
            // Get nuked alien key
            if (iLoserKey == NO_KEY) {

                iNukedAlienKey = NO_KEY;
            
            } else {

                iErrCode = m_pDatabase->ReadData (
                    SYSTEM_EMPIRE_DATA, 
                    iLoserKey, 
                    SystemEmpireData::AlienKey, 
                    &vValue
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                iNukedAlienKey = vValue.GetInteger();
                
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
                iErrCode = m_pGameEngine->SendVictorySneer (
                    iNukerEmpireKey, 
                    vNukerName.GetCharPtr(), 
                    iLoserKey
                    );

                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }

            // Notify classic score
            pClassicScore = m_pGameEngine->GetScoringSystem (CLASSIC_SCORE);
            Assert (pClassicScore != NULL);

            iErrCode = pClassicScore->OnNuke (iGameClass, iGameNumber, iNukerEmpireKey, NO_KEY);
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

    return m_pDatabase->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, pvScore);
}

int AlmonasterScore::GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires) {

    if (pvScore == NULL) {
        return m_pDatabase->GetAllKeys (SYSTEM_EMPIRE_DATA, ppiKey, piNumEmpires);
    }

    Variant vMaxScore = ALMONASTER_MAX_SCORE;
    unsigned int iColumn = SystemEmpireData::AlmonasterScore, iFlag = 0;

    return m_pDatabase->GetSearchKeys (
        SYSTEM_EMPIRE_DATA,
        1,
        &iColumn,
        &iFlag,
        pvScore, 
        &vMaxScore, 
        NO_KEY, 
        0,
        0,
        ppiKey, 
        piNumEmpires, 
        NULL
        );
}

int AlmonasterScore::SendScoreChangeMessage (NukeList nukeList, int iGameClass, int iGameNumber, 
                                             int iEmpireNuker, int iEmpireNuked, float fNukerScore,
                                             float fNukedScore, float fIncrease, float fDecrease) {

    int iErrCode, iEmpireKey;

    char pszMessage [768 + MAX_FULL_GAME_CLASS_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH];

    char pszGameClass [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];
    char pszEmpireName [MAX_EMPIRE_NAME_LENGTH + 1];

    char pszIncrease [128];
    char pszDecrease [128];

    // Get gameclass names
    iErrCode = m_pGameEngine->GetGameClassName (iGameClass, pszGameClass);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    switch (nukeList) {

    case NUKER_LIST:

        iEmpireKey = iEmpireNuker;

        iErrCode = m_pGameEngine->GetEmpireName (iEmpireNuked, pszEmpireName);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (fDecrease == (float) 0.0) {
            
            sprintf (
                pszDecrease,
                "Your opponent's Almonaster Score did not decrease"
                );

        } else {
            
            sprintf (
                pszDecrease,
                "Your opponent's Almonaster Score decreased from "
                BEGIN_STRONG "%f" END_STRONG " to " BEGIN_STRONG "%f" END_STRONG,
                fNukedScore,
                fNukedScore - fDecrease
                );
        }

        if (fIncrease == (float) 0.0) {
            
            sprintf (
                pszIncrease,
                "Your Almonaster Score did not increase"
                );

        } else {
            
            sprintf (
                pszIncrease,
                "Your Almonaster Score increased from "\
                BEGIN_STRONG "%f" END_STRONG " to " BEGIN_STRONG "%f" END_STRONG,
                fNukerScore,
                fNukerScore + fIncrease
                );
        }


        sprintf (
            pszMessage,
            "You nuked " BEGIN_STRONG "%s" END_STRONG " in %s %i\n"\
            "%s\n"\
            "%s",
            pszEmpireName,
            pszGameClass,
            iGameNumber,
            pszIncrease,
            pszDecrease
            );

        break;

    case NUKED_LIST:

        iEmpireKey = iEmpireNuked;

        iErrCode = m_pGameEngine->GetEmpireName (iEmpireNuker, pszEmpireName);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (fIncrease == (float) 0.0) {
            
            sprintf (
                pszIncrease,
                "Your opponent's Almonaster Score did not increase"
                );

        } else {
            
            sprintf (
                pszIncrease,
                "Your opponent's Almonaster Score increased from "
                BEGIN_STRONG "%f" END_STRONG " to " BEGIN_STRONG "%f" END_STRONG,
                fNukerScore,
                fNukerScore + fIncrease
                );
        }

        if (fDecrease == (float) 0.0) {
            
            sprintf (
                pszDecrease,
                "Your Almonaster Score did not decrease"
                );

        } else {
            
            sprintf (
                pszDecrease,
                "Your Almonaster Score decreased from "\
                BEGIN_STRONG "%f" END_STRONG " to " BEGIN_STRONG "%f" END_STRONG,
                fNukedScore,
                fNukedScore - fDecrease
                );
        }

        sprintf (
            pszMessage,
            "You were nuked by " BEGIN_STRONG "%s" END_STRONG " in %s %i\n"\
            "%s\n"\
            "%s",
            pszEmpireName,
            pszGameClass,
            iGameNumber,
            pszDecrease,
            pszIncrease
            );

        break;

    default:

        Assert (false);
        iErrCode = ERROR_INVALID_ARGUMENT;
        goto Cleanup;
    }

    iErrCode = m_pGameEngine->SendSystemMessage (iEmpireKey, pszMessage, SYSTEM);
    if (iErrCode != OK) {
        goto Cleanup;
    }

Cleanup:

    return iErrCode;
}
