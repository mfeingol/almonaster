// ClassicScore.cpp: implementation of the ClassicScore class.
//
//////////////////////////////////////////////////////////////////////

#include "ClassicScore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ClassicScore::ClassicScore (IGameEngine* pGameEngine) {

    m_iNumRefs = 1;

    Assert (pGameEngine != NULL);

    m_pGameEngine = pGameEngine; // Weak ref
    m_pDatabase = m_pGameEngine->GetDatabase(); // AddRef()

    Assert (m_pDatabase != NULL);

    m_iColumn = SystemEmpireData::ClassicScore;
}

ClassicScore::~ClassicScore() {

    if (m_pDatabase != NULL) {
        m_pDatabase->Release();
    }
}

IScoringSystem* ClassicScore::CreateInstance (IGameEngine* pGameEngine) {

    return new ClassicScore (pGameEngine);
}

bool ClassicScore::HasTopList() {
    return true;
}

int ClassicScore::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) {

    int iErrCode = OK;

    Assert (iEmpireNuker != NO_KEY || iEmpireNuked != NO_KEY);

    if (iEmpireNuker != NO_KEY) {

        // Reward nuker
        iErrCode = m_pDatabase->Increment (
            SYSTEM_EMPIRE_DATA, 
            iEmpireNuker,
            SystemEmpireData::ClassicScore, 
            CLASSIC_POINTS_FOR_NUKE
            );
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        // Update top list
        iErrCode = m_pGameEngine->UpdateTopListOnIncrease (
            CLASSIC_SCORE, 
            iEmpireNuker
            );
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    if (iEmpireNuked != NO_KEY) {

        // Punish nuked
        iErrCode = m_pDatabase->Increment (
            SYSTEM_EMPIRE_DATA, 
            iEmpireNuked,
            SystemEmpireData::ClassicScore, 
            CLASSIC_POINTS_FOR_NUKED
            );
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        iErrCode = m_pGameEngine->UpdateTopListOnDecrease (
            CLASSIC_SCORE, 
            iEmpireNuked
            );
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    return iErrCode;
}

int ClassicScore::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges) {

    // Same as nuked
    return OnNuke (iGameClass, iGameNumber, iWinner, iLoser, pscChanges);
}

int ClassicScore::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) {

    // Same as nuked, but no nuker yet
    return OnNuke (iGameClass, iGameNumber, NO_KEY, iLoser, pscChanges);
}

int ClassicScore::On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, 
                                                  int iPlanetKey, ScoringChanges* pscChanges) {

    // Same as nuked, but no nukee
    return OnNuke (iGameClass, iGameNumber, iWinnerKey, NO_KEY, pscChanges);
}

int ClassicScore::OnGameEnd (int iGameClass, int iGameNumber) {

    // We don't really care.
    // No sense in collecting uncolonized 3.0 style surrenders and giving partial credit...

    return OK;
}

int ClassicScore::OnWin (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    // Reward winner
    iErrCode = m_pDatabase->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey,
        SystemEmpireData::ClassicScore, 
        CLASSIC_POINTS_FOR_WIN
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Update top list
    iErrCode = m_pGameEngine->UpdateTopListOnIncrease (
        CLASSIC_SCORE, 
        iEmpireKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}

int ClassicScore::OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    // Reward for draw
    iErrCode = m_pDatabase->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey,
        SystemEmpireData::ClassicScore, 
        CLASSIC_POINTS_FOR_DRAW
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Update top list
    iErrCode = m_pGameEngine->UpdateTopListOnIncrease (
        CLASSIC_SCORE, 
        iEmpireKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}


int ClassicScore::OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    // 'Reward' for ruin
    iErrCode = m_pDatabase->Increment (
        SYSTEM_EMPIRE_DATA, 
        iEmpireKey,
        SystemEmpireData::ClassicScore, 
        CLASSIC_POINTS_FOR_RUIN
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Update top list
    iErrCode = m_pGameEngine->UpdateTopListOnDecrease (
        CLASSIC_SCORE, 
        iEmpireKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}

bool ClassicScore::IsValidScore (const Variant* pvScore) {

    float fScore = pvScore->GetFloat();
    return fScore >= CLASSIC_MIN_SCORE && fScore <= CLASSIC_MAX_SCORE;
}

int ClassicScore::CompareScores (const Variant* pvLeft, const Variant* pvRight) {

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

int ClassicScore::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) {

    return m_pDatabase->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::ClassicScore, pvScore);
}

int ClassicScore::GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires) {

    if (pvScore == NULL) {
        return m_pDatabase->GetAllKeys (SYSTEM_EMPIRE_DATA, ppiKey, piNumEmpires);
    }

    SearchColumn sc;    
    sc.iColumn = SystemEmpireData::ClassicScore;
    sc.iFlags = 0;
    sc.vData = *pvScore;
    sc.vData2 = CLASSIC_MAX_SCORE;

    SearchDefinition sd;
    sd.iMaxNumHits = 0;
    sd.iSkipHits = 0;
    sd.iStartKey = NO_KEY;
    sd.iNumColumns = 1;
    sd.pscColumns = &sc;

    return m_pDatabase->GetSearchKeys (
        SYSTEM_EMPIRE_DATA,
        sd,
        ppiKey, 
        piNumEmpires, 
        NULL
        );
}