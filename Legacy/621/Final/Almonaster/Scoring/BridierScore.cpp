// BridierScore.cpp: implementation of the BridierScore class.
//
//////////////////////////////////////////////////////////////////////

#include "BridierScore.h"

#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//
// BridierObject
//

BridierObject::BridierObject (IGameEngine* pGameEngine) {

    Assert (pGameEngine != NULL);

    m_pGameEngine = pGameEngine; // Weak ref
    m_pDatabase = m_pGameEngine->GetDatabase(); // AddRef()

    Assert (m_pDatabase != NULL);
}

BridierObject::~BridierObject() {

    if (m_pDatabase != NULL) {
        m_pDatabase->Release();
    }
}

int BridierObject::IsBridierGame (int iGameClass, int iGameNumber, bool* pbBridier) {

    int iErrCode;
    Variant vOptions;

    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    iErrCode = m_pDatabase->ReadData (pszGameData, GameData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *pbBridier = (vOptions.GetInteger() & GAME_COUNT_FOR_BRIDIER) != 0;

    return OK;
}

int BridierObject::CompareScores (const Variant* pvLeft, const Variant* pvRight) {

    int iLeftRank, iRightRank, iLeftIndex, iRightIndex;

    iLeftRank = pvLeft[BRIDIER_RANK].GetInteger();
    iLeftIndex = pvLeft[BRIDIER_INDEX].GetInteger();

    iRightRank = pvRight[BRIDIER_RANK].GetInteger();
    iRightIndex = pvRight[BRIDIER_INDEX].GetInteger();

    if (iLeftRank < iRightRank) {
        return -1;
    }

    if (iLeftRank == iRightRank) {

        if (iLeftIndex > iRightIndex) {
            return -1;
        }

        if (iLeftIndex < iRightIndex) {
            return 1;
        }

        return 0;
    }

    return 1;
}

void BridierObject::GetScoreChanges (int iNukerRank, int iNukerIndex, int iNukedRank, int iNukedIndex,
                                     int* piNukerRankChange, int* piNukerIndexChange, 
                                     int* piNukedRankChange, int* piNukedIndexChange) {

    int iStake, iRankIncrease, iRankDecrease;

    float A = (float) iNukerIndex / 100;
    float B = (float) iNukedIndex / 100;

    if (iNukerRank <= iNukedRank) {

        iStake = 10;
    
    } else {

        int iDiff = iNukerRank - iNukedRank;

        if (iDiff < 11) {
            iStake = 9;
        }
    
        else if (iDiff < 21) {
            iStake = 8;
        }

        else if (iDiff < 31) {
            iStake = 7;
        }

        else if (iDiff < 41) {
            iStake = 6;
        }

        else if (iDiff < 61) {
            iStake = 5;
        }

        else if (iDiff < 81) {
            iStake = 4;
        }

        else if (iDiff < 101) {
            iStake = 3;
        }

        else if (iDiff < 141) {
            iStake = 2;
        }

        else if (iDiff < 201) {
            iStake = 1;
        }

        else {
            iStake = 0;
        }
    }

    if (iNukerIndex == iNukedIndex && (iNukerIndex == BRIDIER_MIN_INDEX || iNukerIndex == BRIDIER_MAX_INDEX)) {

        iRankIncrease = iRankDecrease = iStake;

    } else {

        iRankIncrease = (int) floor ((float) iStake * (1 + (float) 19 * A - B - (float) 3 * A * B) / (float) 16);
        iRankDecrease = (int) floor ((float) iStake * (1 + (float) 19 * B - A - (float) 3 * A * B) / (float) 16);
    }

    *piNukerRankChange = iRankIncrease;
    *piNukerIndexChange = - iRankIncrease;

    *piNukedRankChange = - iRankDecrease;
    *piNukedIndexChange = - iRankDecrease;
}

int BridierObject::UpdateBridierScore (int iEmpireKey, int iRankChange, int iIndexChange) {

    int iErrCode, iOldRank, iOldIndex, iNewRank, iNewIndex;

    Variant pvScore [NUM_BRIDIER_COLUMNS];

    UTCTime tNow;

    NamedMutex nmMutex;
    iErrCode = m_pGameEngine->LockEmpireBridier (iEmpireKey, &nmMutex);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    Time::GetTime (&tNow);
    
    // Update activity time
    iErrCode = m_pDatabase->WriteData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::LastBridierActivity, 
        tNow
        );
    
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Get score
    iErrCode = GetEmpireScore (iEmpireKey, pvScore);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iOldRank = pvScore[BRIDIER_RANK].GetInteger();
    iOldIndex = pvScore[BRIDIER_INDEX].GetInteger();

    iNewRank = iOldRank + iRankChange;
    iNewIndex = iOldIndex + iIndexChange;

    if (iNewIndex < BRIDIER_MIN_INDEX) {
        iNewIndex = BRIDIER_MIN_INDEX;
    }

    if (iNewIndex > BRIDIER_MAX_INDEX) {
        iNewIndex = BRIDIER_MAX_INDEX;
    }

    if (iNewRank < BRIDIER_MIN_RANK) {
        iNewRank = BRIDIER_MIN_RANK;
    }

    if (iNewRank != iOldRank || iNewIndex != iOldIndex) {

        if (iNewRank != iOldRank) {

            iErrCode = m_pDatabase->WriteData (
                SYSTEM_EMPIRE_DATA, 
                iEmpireKey, 
                SystemEmpireData::BridierRank, 
                iNewRank
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        if (iNewIndex != iOldIndex) {

            iErrCode = m_pDatabase->WriteData (
                SYSTEM_EMPIRE_DATA, 
                iEmpireKey, 
                SystemEmpireData::BridierIndex, 
                iNewIndex
                );

            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        if (iRankChange > 0) {

            if (iNewIndex <= BRIDIER_TOPLIST_INDEX) {

                iErrCode = m_pGameEngine->UpdateTopListOnIncrease (BRIDIER_SCORE, iEmpireKey);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }

            if (iNewIndex <= BRIDIER_ESTABLISHED_TOPLIST_INDEX) {
                
                iErrCode = m_pGameEngine->UpdateTopListOnIncrease (BRIDIER_SCORE_ESTABLISHED, iEmpireKey);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }

        else if (iRankChange < 0) {

            iErrCode = m_pGameEngine->UpdateTopListOnDecrease (BRIDIER_SCORE, iEmpireKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (iNewIndex <= BRIDIER_TOPLIST_INDEX) {
                
                //
                // This looks weird, but this empire just got his index lowered to
                // where he qualifies for the top list - let's see if he deserves it
                //
                iErrCode = m_pGameEngine->UpdateTopListOnIncrease (BRIDIER_SCORE, iEmpireKey);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
            
            iErrCode = m_pGameEngine->UpdateTopListOnDecrease (BRIDIER_SCORE_ESTABLISHED, iEmpireKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (iNewIndex <= BRIDIER_ESTABLISHED_TOPLIST_INDEX) {
                
                //
                // This looks weird, but this empire just got his index lowered to
                // where he qualifies for the top list - let's see if he deserves it
                //
                iErrCode = m_pGameEngine->UpdateTopListOnIncrease (BRIDIER_SCORE_ESTABLISHED, iEmpireKey);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }
    }

Cleanup:

    m_pGameEngine->UnlockEmpireBridier (nmMutex);

    return iErrCode;
}

int BridierObject::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) {

    int iErrCode;

    iErrCode = m_pDatabase->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::BridierRank, 
        pvScore + BRIDIER_RANK
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pDatabase->ReadData (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey, 
        SystemEmpireData::BridierIndex, 
        pvScore + BRIDIER_INDEX
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    return iErrCode;
}

int BridierObject::GetReplacementKeys (bool bEstablished, const Variant* pvScore, unsigned int** ppiKey, 
                                       unsigned int* piNumEmpires) {

#ifdef _DEBUG
    const int iMaxIndex = bEstablished ? BRIDIER_ESTABLISHED_TOPLIST_INDEX : BRIDIER_TOPLIST_INDEX;
    Assert (pvScore == NULL || pvScore [BRIDIER_INDEX].GetInteger() <= iMaxIndex);
#endif

    SearchColumn sc [NUM_BRIDIER_COLUMNS];

    sc[BRIDIER_RANK].iColumn = SystemEmpireData::BridierRank;
    sc[BRIDIER_RANK].iFlags = 0;
    sc[BRIDIER_RANK].vData = pvScore != NULL ? pvScore [BRIDIER_RANK].GetInteger() : BRIDIER_MIN_RANK;
    sc[BRIDIER_RANK].vData2 = BRIDIER_MAX_RANK;

    sc[BRIDIER_INDEX].iColumn = SystemEmpireData::BridierIndex;
    sc[BRIDIER_INDEX].iFlags = 0;

    if (bEstablished) {
        sc[BRIDIER_INDEX].vData = BRIDIER_ESTABLISHED_TOPLIST_INDEX;
        sc[BRIDIER_INDEX].vData2 = BRIDIER_ESTABLISHED_TOPLIST_INDEX;
    } else {
        sc[BRIDIER_INDEX].vData = BRIDIER_MIN_INDEX;
        sc[BRIDIER_INDEX].vData2 = BRIDIER_TOPLIST_INDEX;
    }

    SearchDefinition sd;
    sd.iMaxNumHits = 0;
    sd.iSkipHits = 0;
    sd.iStartKey = NO_KEY;
    sd.iNumColumns = NUM_BRIDIER_COLUMNS;
    sd.pscColumns = sc;

    Assert (countof (sc) == NUM_BRIDIER_COLUMNS);

    return m_pDatabase->GetSearchKeys (
        SYSTEM_EMPIRE_DATA,
        sd,
        ppiKey,
        piNumEmpires,
        NULL
        );
}

//
// BridierScore
//

BridierScore::BridierScore (IGameEngine* pGameEngine)  : BridierObject (pGameEngine) {

    m_iNumRefs = 1;
}

IScoringSystem* BridierScore::CreateInstance (IGameEngine* pGameEngine) {

    return new BridierScore (pGameEngine);
}

bool BridierScore::HasTopList() {
    return true;
}

int BridierScore::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) {

    int iErrCode;
    bool bBridier;

    iErrCode = IsBridierGame (iGameClass, iGameNumber, &bBridier);
    if (iErrCode != OK || !bBridier) {
        return iErrCode;
    }

    return OnNukeInternal (iGameClass, iGameNumber, iEmpireNuker, iEmpireNuked, pscChanges);
}

int BridierScore::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges) {

    return OnNuke (iGameClass, iGameNumber, iWinner, iLoser, pscChanges);
}

int BridierScore::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) {

    int iErrCode;
    bool bBridier;

    Variant* pvEmpireKey;
    unsigned int iNumEmpires;

    iErrCode = IsBridierGame (iGameClass, iGameNumber, &bBridier);
    if (iErrCode != OK || !bBridier) {
        return iErrCode;
    }

    GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = m_pDatabase->ReadColumn (pszEmpires, GameEmpires::EmpireKey, &pvEmpireKey, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    Assert (iNumEmpires == 2);

    iErrCode = OnNukeInternal (
        iGameClass, 
        iGameNumber, 
        (pvEmpireKey[0].GetInteger() == iLoser) ? pvEmpireKey[1].GetInteger() : pvEmpireKey[0].GetInteger(),
        iLoser,
        pscChanges
        );

    m_pDatabase->FreeData (pvEmpireKey);

    return iErrCode;
}

int BridierScore::On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges) {
    return OK;
}

int BridierScore::OnGameEnd (int iGameClass, int iGameNumber) {
    return OK;
}

int BridierScore::OnWin (int iGameClass, int iGameNumber, int iEmpireKey) {
    return OK;
}

int BridierScore::OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) {
    return OK;
}

int BridierScore::OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) {
    return OK;
}

bool BridierScore::IsValidScore (const Variant* pvScore) {

    int iRank, iIndex;

    iRank = pvScore[BRIDIER_RANK].GetInteger();
    iIndex = pvScore[BRIDIER_INDEX].GetInteger();

    if (iRank < BRIDIER_MIN_RANK || iRank > BRIDIER_MAX_RANK) {
        return false;
    }

    if (iIndex < BRIDIER_MIN_INDEX || iIndex > BRIDIER_TOPLIST_INDEX) {
        return false;
    }

    return true;
}

int BridierScore::CompareScores (const Variant* pvLeft, const Variant* pvRight) {

    return BridierObject::CompareScores (pvLeft, pvRight);
}

int BridierScore::OnNukeInternal (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) {

    Variant pvNukerScore [NUM_BRIDIER_COLUMNS], pvNukedScore [NUM_BRIDIER_COLUMNS];

    int iNukerRank, iNukerIndex, iNukedRank, iNukedIndex, iErrCode, iNukerRankChange, iNukerIndexChange,
        iNukedRankChange, iNukedIndexChange;

    // Get scores for players at start of game
    GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireNuker);

    iErrCode = m_pDatabase->ReadData (pszEmpireData, GameEmpireData::InitialBridierRank, pvNukerScore + BRIDIER_RANK);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pDatabase->ReadData (pszEmpireData, GameEmpireData::InitialBridierIndex, pvNukerScore + BRIDIER_INDEX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireNuked);

    iErrCode = m_pDatabase->ReadData (pszEmpireData, GameEmpireData::InitialBridierRank, pvNukedScore + BRIDIER_RANK);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pDatabase->ReadData (pszEmpireData, GameEmpireData::InitialBridierIndex, pvNukedScore + BRIDIER_INDEX);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iNukerRank = pvNukerScore[BRIDIER_RANK].GetInteger();
    iNukerIndex = pvNukerScore[BRIDIER_INDEX].GetInteger();

    iNukedRank = pvNukedScore[BRIDIER_RANK].GetInteger();
    iNukedIndex = pvNukedScore[BRIDIER_INDEX].GetInteger();

    // Do the math!
    GetScoreChanges (
        iNukerRank,
        iNukerIndex,
        iNukedRank,
        iNukedIndex,
        &iNukerRankChange,
        &iNukerIndexChange,
        &iNukedRankChange,
        &iNukedIndexChange
        );

    // Update empires
    iErrCode = UpdateBridierScore (iEmpireNuker, iNukerRankChange, iNukerIndexChange);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = UpdateBridierScore (iEmpireNuked, iNukedRankChange, iNukedIndexChange);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Do accounting
    pscChanges->iFlags |= BRIDIER_SCORE_CHANGE;
    pscChanges->iBridierNukerRank = iNukerRank;
    pscChanges->iBridierNukerRankChange = iNukerRankChange;
    pscChanges->iBridierNukerIndex = iNukerIndex;
    pscChanges->iBridierNukerIndexChange = iNukerIndexChange;
    pscChanges->iBridierNukedRank = iNukedRank;
    pscChanges->iBridierNukedRankChange = iNukedRankChange;
    pscChanges->iBridierNukedIndex = iNukedIndex;
    pscChanges->iBridierNukedIndexChange = iNukedIndexChange;

Cleanup:

    return iErrCode;
}

int BridierScore::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) {

    return BridierObject::GetEmpireScore (iEmpireKey, pvScore);
}

int BridierScore::GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires) {

    return BridierObject::GetReplacementKeys (false, pvScore, ppiKey, piNumEmpires);
}


//
// BridierScoreEstablished
//

BridierScoreEstablished::BridierScoreEstablished (IGameEngine* pGameEngine) : BridierObject (pGameEngine) {

    m_iNumRefs = 1;
}

IScoringSystem* BridierScoreEstablished::CreateInstance (IGameEngine* pGameEngine) {

    return new BridierScoreEstablished (pGameEngine);
}

bool BridierScoreEstablished::HasTopList() {
    return true;
}

int BridierScoreEstablished::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) {
    return OK;
}

int BridierScoreEstablished::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges) {
    return OK;
}

int BridierScoreEstablished::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) {
    return OK;
}

int BridierScoreEstablished::On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges) {
    return OK;
}

int BridierScoreEstablished::OnGameEnd (int iGameClass, int iGameNumber) {
    return OK;
}

int BridierScoreEstablished::OnWin (int iGameClass, int iGameNumber, int iEmpireKey) {
    return OK;
}

int BridierScoreEstablished::OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) {
    return OK;
}

int BridierScoreEstablished::OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) {
    return OK;
}

bool BridierScoreEstablished::IsValidScore (const Variant* pvScore) {

    int iRank, iIndex;

    iRank = pvScore [BRIDIER_RANK].GetInteger();
    iIndex = pvScore [BRIDIER_INDEX].GetInteger();

    if (iRank < BRIDIER_MIN_RANK || iRank > BRIDIER_MAX_RANK) {
        return false;
    }

    if (iIndex != BRIDIER_ESTABLISHED_TOPLIST_INDEX) {
        return false;
    }

    return true;
}

int BridierScoreEstablished::CompareScores (const Variant* pvLeft, const Variant* pvRight) {

    return BridierObject::CompareScores (pvLeft, pvRight);
}

int BridierScoreEstablished::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) {

    return BridierObject::GetEmpireScore (iEmpireKey, pvScore);
}

int BridierScoreEstablished::GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires) {

    return BridierObject::GetReplacementKeys (true, pvScore, ppiKey, piNumEmpires);
}