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

int TournamentScoring::IsTournamentGame (int iGameClass, int iGameNumber, unsigned int* piTournamentKey) {

    int iErrCode;
    Variant vKey;

    *piTournamentKey = NO_KEY;

    iErrCode = t_pConn->ReadData(
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::TournamentKey,
        &vKey
        );

    if (iErrCode == OK) {
        *piTournamentKey = vKey.GetInteger();
    }

    return iErrCode;
}

int TournamentScoring::OnEvent (unsigned int iTournamentKey, unsigned int iEmpireKey, TournamentEvent event) {

    int iErrCode;
    unsigned int iKey;
    Variant vTeamKey;

    SYSTEM_TOURNAMENT_EMPIRES (pszEmpires, iTournamentKey);

    iErrCode = t_pConn->GetFirstKey (pszEmpires, SystemTournamentEmpires::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Update event count
    iErrCode = t_pConn->Increment (pszEmpires, iKey, s_pszEmpireColumn [event], 1);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Get team
    iErrCode = t_pConn->ReadData(pszEmpires, iKey, SystemTournamentEmpires::TeamKey, &vTeamKey);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (vTeamKey.GetInteger() != NO_KEY) {

        SYSTEM_TOURNAMENT_TEAMS (pszTeams, iTournamentKey);

        iErrCode = t_pConn->Increment (pszTeams, vTeamKey.GetInteger(), s_pszTeamColumn [event], 1);
        if (iErrCode != OK) {
            return iErrCode;
        }
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
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    if (iTournamentKey == NO_KEY) {
        return OK;
    }

    Assert (iEmpireNuker != NO_KEY || iEmpireNuked != NO_KEY);

    if (iEmpireNuker != NO_KEY) {

        iErrCode = OnEvent (iTournamentKey, iEmpireNuker, TOURNAMENT_NUKE);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    if (iEmpireNuked != NO_KEY) {

        iErrCode = OnEvent (iTournamentKey, iEmpireNuked, TOURNAMENT_NUKED);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    return iErrCode;
}

int TournamentScoring::OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges) {

    // Same as nuked
    return OnNuke (iGameClass, iGameNumber, iWinner, iLoser, pscChanges);
}

int TournamentScoring::On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) {

    // Same as nuke, but no nuker yet
    return OnNuke (iGameClass, iGameNumber, NO_KEY, iLoser, pscChanges);
}

int TournamentScoring::On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, 
                                                       int iPlanetKey, ScoringChanges* pscChanges) {

    // Same as nuke, but no nukee
    return OnNuke (iGameClass, iGameNumber, iWinnerKey, NO_KEY, pscChanges);
}

int TournamentScoring::OnGameEnd (int iGameClass, int iGameNumber) {

    // Don't care
    return OK;
}

int TournamentScoring::OnEvent (int iGameClass, int iGameNumber, int iEmpireKey, TournamentEvent event) {

    int iErrCode;
    unsigned int iTournamentKey;
    
    iErrCode = IsTournamentGame (iGameClass, iGameNumber, &iTournamentKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    if (iTournamentKey == NO_KEY) {
        return OK;
    }

    iErrCode = OnEvent (iTournamentKey, iEmpireKey, event);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}

int TournamentScoring::OnWin (int iGameClass, int iGameNumber, int iEmpireKey) {

    return OnEvent (iGameClass, iGameNumber, iEmpireKey, TOURNAMENT_WIN);
}

int TournamentScoring::OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) {

    return OnEvent (iGameClass, iGameNumber, iEmpireKey, TOURNAMENT_DRAW);
}


int TournamentScoring::OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) {

    return OnEvent (iGameClass, iGameNumber, iEmpireKey, TOURNAMENT_RUIN);
}

bool TournamentScoring::IsValidScore (const Variant* pvScore) {

    Assert (false);
    return false;
}

int TournamentScoring::CompareScores (const Variant* pvLeft, const Variant* pvRight) {

    Assert (false);
    return 0;
}

int TournamentScoring::GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) {

    Assert (false);
    return ERROR_NOT_IMPLEMENTED;
}

int TournamentScoring::GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires) {

    Assert (false);
    return ERROR_NOT_IMPLEMENTED;
}