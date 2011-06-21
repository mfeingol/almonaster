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

#include "Interface.h"
#include "GameEngine.h"

enum TournamentEvent {
    TOURNAMENT_NUKE,
    TOURNAMENT_NUKED,
    TOURNAMENT_WIN,
    TOURNAMENT_DRAW,
    TOURNAMENT_RUIN,
    TOURNAMENT_NUM_EVENTS
};


class TournamentScoring : public IScoringSystem
{
protected:
    GameEngine* m_pGameEngine;

    int IsTournamentGame (int iGameClass, int iGameNumber, unsigned int* piTournamentKey);
    int OnEvent (int iGameClass, int iGameNumber, int iEmpireKey, TournamentEvent event);
    int OnEvent (unsigned int iTournamentKey, unsigned int iEmpireKey, TournamentEvent event);

public:

    TournamentScoring(GameEngine* pGameEngine);
    ~TournamentScoring();

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
    int GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires);
};