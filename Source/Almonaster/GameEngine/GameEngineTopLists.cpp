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

#include <math.h>

#include "GameEngine.h"
#include "Global.h"

#include "AlmonasterScore.h"
#include "BridierScore.h"
#include "ClassicScore.h"
#include "TournamentScoring.h"

int GameEngine::GetTopList(ScoringSystem ssListType, unsigned int** ppiEmpireKey, unsigned int* piNumEmpires)
{
    Assert(ssListType >= FIRST_SCORING_SYSTEM && ssListType < NUM_SCORING_SYSTEMS);

    const OrderByColumnDefinition scAlmonasterScoreCols[] = 
    { 
        { SystemEmpireData::AlmonasterScore, false },
        { SystemEmpireData::AlmonasterScoreSignificance, false },
        { SystemEmpireData::Nukes, false }
    };

    const OrderByColumnDefinition scClassicScoreCols[] = 
    { 
        { SystemEmpireData::ClassicScore, false },
        { SystemEmpireData::Nukes, false },
    };

    const OrderByColumnDefinition scBridierCols[] = 
    { 
        { SystemEmpireData::BridierRank, false },
        { SystemEmpireData::BridierIndex, true },
        { SystemEmpireData::Nukes, false },
    };

    const SearchColumnDefinition bridierScoreCol = { SystemEmpireData::BridierIndex, SEARCH_RANGE_INCLUSIVE, 0, BRIDIER_TOPLIST_INDEX };
    const SearchColumnDefinition bridierEstablishedScoreCol = { SystemEmpireData::BridierIndex, SEARCH_RANGE_INCLUSIVE, 0, BRIDIER_ESTABLISHED_TOPLIST_INDEX };

    SearchDefinition rangeSd = { 0, TOPLIST_SIZE, 0, NULL};
    OrderByDefinition orderByDef = { 0, NULL };

    switch (ssListType)
    {
    case ALMONASTER_SCORE:
        orderByDef.iNumColumns = countof(scAlmonasterScoreCols);
        orderByDef.pscColumns = (OrderByColumnDefinition*)scAlmonasterScoreCols;
        break;

    case CLASSIC_SCORE:
        orderByDef.iNumColumns = countof(scClassicScoreCols);
        orderByDef.pscColumns = (OrderByColumnDefinition*)scClassicScoreCols;
        break;

    case BRIDIER_SCORE:
        orderByDef.iNumColumns = countof(scBridierCols);
        orderByDef.pscColumns = (OrderByColumnDefinition*)scBridierCols;
        rangeSd.iNumColumns = 1;
        rangeSd.pscColumns = (SearchColumnDefinition*)&bridierScoreCol;
        break;

    case BRIDIER_SCORE_ESTABLISHED:
        orderByDef.iNumColumns = countof(scBridierCols);
        orderByDef.pscColumns = (OrderByColumnDefinition*)scBridierCols;
        rangeSd.iNumColumns = 1;
        rangeSd.pscColumns = (SearchColumnDefinition*)&bridierEstablishedScoreCol;
        break;

    default:
        Assert(false);
        break;
    }

    bool bMore;
    int iErrCode = t_pConn->GetSearchKeys(SYSTEM_EMPIRE_DATA, rangeSd, &orderByDef, ppiEmpireKey, piNumEmpires, &bMore);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

IScoringSystem* GameEngine::CreateScoringSystem(ScoringSystem ssTopList)
{
    switch (ssTopList)
    {
    case ALMONASTER_SCORE:
        return new AlmonasterScore(this);
    case CLASSIC_SCORE:
        return new ClassicScore(this);
    case BRIDIER_SCORE:
        return new BridierScore(this);
    case BRIDIER_SCORE_ESTABLISHED:
        return new BridierScoreEstablished(this);
    case TOURNAMENT_SCORING:
        return new TournamentScoring(this);
    default:
        Assert(false);
        return NULL;
    }
}
