// ClassicScore.cpp: implementation of the ClassicScore class.
//
//////////////////////////////////////////////////////////////////////

#include "ClassicScore.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ClassicScore::ClassicScore(GameEngine* pGameEngine)
{
    m_pGameEngine = pGameEngine;
    m_pszColumn = SystemEmpireData::ClassicScore;
}

bool ClassicScore::HasTopList() {
    return true;
}

int ClassicScore::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) {

    int iErrCode = OK;

    Assert(iEmpireNuker != NO_KEY || iEmpireNuked != NO_KEY);

    GET_SYSTEM_EMPIRE_DATA(strNuker, iEmpireNuker);
    GET_SYSTEM_EMPIRE_DATA(strNuked, iEmpireNuked);

    if (iEmpireNuker != NO_KEY) {

        // Reward nuker
        iErrCode = t_pCache->Increment(
            strNuker, 
            iEmpireNuker,
            SystemEmpireData::ClassicScore, 
            CLASSIC_POINTS_FOR_NUKE
            );
        
        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    if (iEmpireNuked != NO_KEY) {

        // Punish nuked
        iErrCode = t_pCache->Increment(
            strNuked, 
            iEmpireNuked,
            SystemEmpireData::ClassicScore, 
            CLASSIC_POINTS_FOR_NUKED
            );
        
        if (iErrCode != OK) {
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
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->Increment(
        strEmpire, 
        iEmpireKey,
        SystemEmpireData::ClassicScore, 
        CLASSIC_POINTS_FOR_WIN
        );

    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    return iErrCode;
}

int ClassicScore::OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    // Reward for draw
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->Increment(
        strEmpire, 
        iEmpireKey,
        SystemEmpireData::ClassicScore, 
        CLASSIC_POINTS_FOR_DRAW
        );

    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    return iErrCode;
}


int ClassicScore::OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    // 'Reward' for ruin
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->Increment(
        strEmpire, 
        iEmpireKey,
        SystemEmpireData::ClassicScore, 
        CLASSIC_POINTS_FOR_RUIN
        );

    if (iErrCode != OK) {
        Assert(false);
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

int ClassicScore::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore)
{
    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);
    return t_pCache->ReadData(strEmpires, iEmpireKey, SystemEmpireData::ClassicScore, pvScore);
}