//
// GameEngine.dll:  a component of Almonaster
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

#include "Osal/IObject.h"

class IAlmonasterEventSink
{
public:

    virtual int OnCreateEmpire(int iEmpireKey) = 0;
    virtual int OnDeleteEmpire(int iEmpireKey) = 0;

    virtual int OnLoginEmpire(int iEmpireKey) = 0;
    
    virtual int OnCreateGame(int iGameClass, int iGameNumber) = 0;
    virtual int OnCleanupGame(int iGameClass, int iGameNumber) = 0;

    virtual int OnDeleteTournament(unsigned int iTournamentKey) = 0;
    virtual int OnDeleteTournamentTeam(unsigned int iTournamentKey, unsigned int iTeamKey) = 0;
};

struct ScoringChanges
{
    ScoringChanges()
    {
        iFlags = 0;
        pszNukedName = NULL;
    }

    int iFlags;
    const char* pszNukedName;

    float fAlmonasterNukerScore;
    float fAlmonasterNukerChange;
    float fAlmonasterNukedScore;
    float fAlmonasterNukedChange;

    int iBridierNukerRank;
    int iBridierNukerRankChange;
    int iBridierNukerIndex;
    int iBridierNukerIndexChange;

    int iBridierNukedRank;
    int iBridierNukedRankChange;
    int iBridierNukedIndex;
    int iBridierNukedIndexChange;
};

#define ALMONASTER_NUKER_SCORE_CHANGE 0x00000001
#define ALMONASTER_NUKED_SCORE_CHANGE 0x00000002
#define BRIDIER_SCORE_CHANGE          0x00000004

class IScoringSystem
{
public:

    virtual bool HasTopList() = 0;

    virtual int OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) = 0;
    virtual int OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges) = 0;

    virtual int On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) = 0;
    virtual int On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges) = 0;

    virtual int OnGameEnd (int iGameClass, int iGameNumber) = 0;
    virtual int OnWin (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) = 0;

    virtual bool IsValidScore (const Variant* pvScore) = 0;
    virtual int CompareScores (const Variant* pvLeft, const Variant* pvRight) = 0;

    virtual int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) = 0;
    virtual int GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires) = 0;
};

class IMapGenerator
{
public:

    // Implementor must do the following:
    //
    // 1) If iNumExistingPlanets == 0, fill in the following columns in the pvGameData row:
    //
    // GameData::NumPlanetsPerEmpire,
    // GameData::HWAg
    // GameData::AvgAg
    // GameData::HWMin
    // GameData::AvgMin
    // GameData::HWFuel
    // GameData::AvgFuel
    //
    // 2) Allocate *piNumNewPlanets new rows into *pppvNewPlanetData, each with GameMap::NumColumns
    // 3) Fill in the following columns for each new planet row:
    //
    // GameMap::Ag,
    // GameMap::Minerals
    // GameMap::Fuel
    // GameMap::Coordinates
    // GameMap::Link
    // GameMap::HomeWorld
    //
    // GameMap::Owner -> Set to empire's chain, even if not fully-colonized map
    //
    // Everything else is taken care of by the caller
    //
    // Sanity rules apply:
    // - Coordinates already in use on the map must not be used
    // - Links must actually have a planet behind them
    // - Exactly one homeworld per empire must be selected
    // - etc.

    virtual int CreatePlanets (
        
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
        ) = 0;

    virtual void FreePlanetData(Variant** ppvNewPlanetData) = 0;
};