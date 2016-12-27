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

#include "BridierScore.h"
#include <math.h>

//
// BridierObject
//

BridierObject::BridierObject(GameEngine* pGameEngine)
{
    m_pGameEngine = pGameEngine;
}

int BridierObject::IsBridierGame (int iGameClass, int iGameNumber, bool* pbBridier) {

    int iErrCode;
    Variant vOptions;

    GET_GAME_DATA(pszGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(pszGameData, GameData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    *pbBridier = (vOptions.GetInteger() & GAME_COUNT_FOR_BRIDIER) != 0;

    return iErrCode;
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
    Time::GetTime (&tNow);
    
    // Update activity time
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->WriteData(
        strEmpire, 
        iEmpireKey, 
        SystemEmpireData::LastBridierActivity, 
        tNow
        );
    
    RETURN_ON_ERROR(iErrCode);
    
    // Get score
    iErrCode = GetEmpireScore (iEmpireKey, pvScore);
    RETURN_ON_ERROR(iErrCode);

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

        GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

        if (iNewRank != iOldRank) {

            iErrCode = t_pCache->WriteData(
                strEmpire, 
                iEmpireKey, 
                SystemEmpireData::BridierRank, 
                iNewRank
                );
            
            RETURN_ON_ERROR(iErrCode);
        }

        if (iNewIndex != iOldIndex) {

            iErrCode = t_pCache->WriteData(
                strEmpire, 
                iEmpireKey, 
                SystemEmpireData::BridierIndex, 
                iNewIndex
                );

            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

int BridierObject::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore)
{
    int iErrCode;
     
    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);

    iErrCode = t_pCache->ReadData(strEmpires, iEmpireKey, SystemEmpireData::BridierRank, pvScore + BRIDIER_RANK);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strEmpires, iEmpireKey, SystemEmpireData::BridierIndex, pvScore + BRIDIER_INDEX);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

//
// BridierScore
//

BridierScore::BridierScore(GameEngine* pGameEngine) : BridierObject(pGameEngine)
{
}

bool BridierScore::HasTopList() {
    return true;
}

int BridierScore::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) {

    int iErrCode;
    bool bBridier;

    iErrCode = IsBridierGame (iGameClass, iGameNumber, &bBridier);
    RETURN_ON_ERROR(iErrCode);
    if (!bBridier)
    {
        return OK;
    }

    iErrCode = OnNukeInternal (iGameClass, iGameNumber, iEmpireNuker, iEmpireNuked, pscChanges);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int BridierScore::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges)
{
    int iErrCode = OnNuke (iGameClass, iGameNumber, iWinner, iLoser, pscChanges);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int BridierScore::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) {

    int iErrCode;
    bool bBridier;

    Variant* pvEmpireKey = NULL;
    AutoFreeData free_pvEmpireKey(pvEmpireKey);
    unsigned int iNumEmpires;

    iErrCode = IsBridierGame (iGameClass, iGameNumber, &bBridier);
    RETURN_ON_ERROR(iErrCode);
    if (!bBridier)
    {
        return OK;
    }

    GET_GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadColumn(pszEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    Assert(iNumEmpires == 2);

    iErrCode = OnNukeInternal (
        iGameClass, 
        iGameNumber, 
        (pvEmpireKey[0].GetInteger() == iLoser) ? pvEmpireKey[1].GetInteger() : pvEmpireKey[0].GetInteger(),
        iLoser,
        pscChanges
        );
    RETURN_ON_ERROR(iErrCode);

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

    int iNukerRank, iNukerIndex, iNukedRank, iNukedIndex, iErrCode, iNukerRankChange, iNukerIndexChange, iNukedRankChange, iNukedIndexChange;

    // Get scores for players at start of game
    GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireNuker);

    iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::InitialBridierRank, pvNukerScore + BRIDIER_RANK);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::InitialBridierIndex, pvNukerScore + BRIDIER_INDEX);
    RETURN_ON_ERROR(iErrCode);

    COPY_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireNuked);

    iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::InitialBridierRank, pvNukedScore + BRIDIER_RANK);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::InitialBridierIndex, pvNukedScore + BRIDIER_INDEX);
    RETURN_ON_ERROR(iErrCode);

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
    RETURN_ON_ERROR(iErrCode);

    iErrCode = UpdateBridierScore (iEmpireNuked, iNukedRankChange, iNukedIndexChange);
    RETURN_ON_ERROR(iErrCode);

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

    return iErrCode;
}

int BridierScore::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) {

    return BridierObject::GetEmpireScore (iEmpireKey, pvScore);
}

//
// BridierScoreEstablished
//

BridierScoreEstablished::BridierScoreEstablished(GameEngine* pGameEngine) : BridierObject (pGameEngine)
{
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