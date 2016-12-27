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

#pragma once

#include "GameEngine.h"

//
// BridierObject
//

class BridierObject
{
protected:
    GameEngine* m_pGameEngine;

public:

    BridierObject(GameEngine* pGameEngine);

    int IsBridierGame (int iGameClass, int iGameNumber, bool* pbBridier);
    int CompareScores (const Variant* pvLeft, const Variant* pvRight);

    static void GetScoreChanges(int iNukerRank, int iNukerIndex, int iNukedRank, int iNukedIndex,
                                int* piNukerRankChange, int* piNukerIndexChange, int* piNukedRankChange, int* piNukedIndexChange);

    int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore);

    int UpdateBridierScore (int iEmpireKey, int iRankChange, int iIndexChange);
};


//
// BridierScore
//

class BridierScore : public IScoringSystem, protected BridierObject
{
public:

    BridierScore(GameEngine* pGameEngine);
    
    // IScoringSystem
    bool HasTopList();

    int OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges);
    int OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges);

    int On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges);
    int On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges);

    int OnGameEnd (int iGameClass, int iGameNumber);
    int OnWin (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnDraw (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnRuin (int iGameClass, int iGameNumber, int iEmpireKey);

    bool IsValidScore (const Variant* pvScore);
    int CompareScores (const Variant* pvLeft, const Variant* pvRight);

    int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore);

protected:

    int OnNukeInternal (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges);
};


//
// BridierScoreEstablished
//

class BridierScoreEstablished : public IScoringSystem, protected BridierObject {
public:

    BridierScoreEstablished(GameEngine* pGameEngine);

    // IScoringSystem
    bool HasTopList();

    int OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges);
    int OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges);

    int On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges);
    int On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges);

    int OnGameEnd (int iGameClass, int iGameNumber);
    int OnWin (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnDraw (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnRuin (int iGameClass, int iGameNumber, int iEmpireKey);

    bool IsValidScore (const Variant* pvScore);
    int CompareScores (const Variant* pvLeft, const Variant* pvRight);

    int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore);
};
