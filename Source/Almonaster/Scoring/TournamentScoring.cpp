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

#include "TournamentScoring.h"

static const char* s_pszEmpireColumn[TOURNAMENT_NUM_EVENTS] =
{
    SystemTournamentEmpires::Nukes,
    SystemTournamentEmpires::Nuked,
    SystemTournamentEmpires::Wins,
    SystemTournamentEmpires::Draws,
    SystemTournamentEmpires::Ruins,
};

static const char* s_pszTeamColumn[TOURNAMENT_NUM_EVENTS] =
{
    SystemTournamentTeams::Nukes,
    SystemTournamentTeams::Nuked,
    SystemTournamentTeams::Wins,
    SystemTournamentTeams::Draws,
    SystemTournamentTeams::Ruins,
};

TournamentScoring::TournamentScoring(GameEngine* pGameEngine)
{
    m_pGameEngine = pGameEngine;
}

TournamentScoring::~TournamentScoring()
{
}

int TournamentScoring::IsTournamentGame (int iGameClass, int iGameNumber, unsigned int* piTournamentKey) {

    int iErrCode;
    Variant vKey;

    *piTournamentKey = NO_KEY;

    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::TournamentKey,
        &vKey
        );
    RETURN_ON_ERROR(iErrCode);

    *piTournamentKey = vKey.GetInteger();
    return iErrCode;
}

int TournamentScoring::OnEvent (unsigned int iTournamentKey, unsigned int iEmpireKey, TournamentEvent event) {

    int iErrCode;
    unsigned int iKey;
    Variant vTeamKey;

    GET_SYSTEM_TOURNAMENT_EMPIRES(pszEmpires, iTournamentKey);

    iErrCode = t_pCache->GetFirstKey(pszEmpires, SystemTournamentEmpires::EmpireKey, iEmpireKey, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Update event count
    iErrCode = t_pCache->Increment(pszEmpires, iKey, s_pszEmpireColumn [event], 1);
    RETURN_ON_ERROR(iErrCode);

    // Get team
    iErrCode = t_pCache->ReadData(pszEmpires, iKey, SystemTournamentEmpires::TeamKey, &vTeamKey);
    RETURN_ON_ERROR(iErrCode);

    if (vTeamKey.GetInteger() != NO_KEY) {

        GET_SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

        iErrCode = t_pCache->Increment(pszTeams, vTeamKey.GetInteger(), s_pszTeamColumn [event], 1);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

bool TournamentScoring::HasTopList() {
    return false;
}

int TournamentScoring::OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) {

    int iErrCode;
    unsigned int iTournamentKey;
    
    iErrCode = IsTournamentGame (iGameClass, iGameNumber, &iTournamentKey);
    RETURN_ON_ERROR(iErrCode);
    if (iTournamentKey == NO_KEY) {
        return OK;
    }

    GameEngine engine;
    iErrCode = engine.CacheTournamentTables(&iTournamentKey, 1);
    RETURN_ON_ERROR(iErrCode);

    Assert(iEmpireNuker != NO_KEY || iEmpireNuked != NO_KEY);

    if (iEmpireNuker != NO_KEY) {

        iErrCode = OnEvent (iTournamentKey, iEmpireNuker, TOURNAMENT_NUKE);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iEmpireNuked != NO_KEY) {

        iErrCode = OnEvent (iTournamentKey, iEmpireNuked, TOURNAMENT_NUKED);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int TournamentScoring::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges) {

    // Same as nuked
    int iErrCode = OnNuke (iGameClass, iGameNumber, iWinner, iLoser, pscChanges);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int TournamentScoring::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) {

    // Same as nuke, but no nuker yet
    int iErrCode = OnNuke (iGameClass, iGameNumber, NO_KEY, iLoser, pscChanges);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int TournamentScoring::On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges)
{
    // Same as nuke, but no nukee
    int iErrCode = OnNuke (iGameClass, iGameNumber, iWinnerKey, NO_KEY, pscChanges);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int TournamentScoring::OnGameEnd (int iGameClass, int iGameNumber) {

    // Don't care
    return OK;
}

int TournamentScoring::OnEvent (int iGameClass, int iGameNumber, int iEmpireKey, TournamentEvent event) {

    int iErrCode;
    unsigned int iTournamentKey;
    
    iErrCode = IsTournamentGame (iGameClass, iGameNumber, &iTournamentKey);
    RETURN_ON_ERROR(iErrCode);
    if (iTournamentKey == NO_KEY) {
        return OK;
    }

    GameEngine engine;
    iErrCode = engine.CacheTournamentTables(&iTournamentKey, 1);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = OnEvent (iTournamentKey, iEmpireKey, event);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int TournamentScoring::OnWin (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode = OnEvent (iGameClass, iGameNumber, iEmpireKey, TOURNAMENT_WIN);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int TournamentScoring::OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode = OnEvent (iGameClass, iGameNumber, iEmpireKey, TOURNAMENT_DRAW);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}


int TournamentScoring::OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode = OnEvent (iGameClass, iGameNumber, iEmpireKey, TOURNAMENT_RUIN);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

bool TournamentScoring::IsValidScore (const Variant* pvScore) {

    Assert(false);
    return false;
}

int TournamentScoring::CompareScores (const Variant* pvLeft, const Variant* pvRight) {

    Assert(false);
    return 0;
}

int TournamentScoring::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) {

    Assert(false);
    return ERROR_NOT_IMPLEMENTED;
}