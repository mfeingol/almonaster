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

#include "GameEngine.h"

#include "../Scoring/BridierScore.h"

int GameEngine::GetGameEntryRestrictions (int iGameClass, int iGameNumber, int* piOptions, 
                                          Variant pvRestrictionMin [NUM_ENTRY_SCORE_RESTRICTIONS], 
                                          Variant pvRestrictionMax [NUM_ENTRY_SCORE_RESTRICTIONS]) {

    int iErrCode, iOptions;

    Variant vOptions;

    GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

    // Read game options
    iErrCode = t_pCache->ReadData(pszGameData, GameData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    iOptions = *piOptions = vOptions.GetInteger();

    // Almonaster
    if (iOptions & GAME_RESTRICT_MIN_ALMONASTER_SCORE) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MinAlmonasterScore, pvRestrictionMin + RESTRICT_ALMONASTER_SCORE);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iOptions & GAME_RESTRICT_MAX_ALMONASTER_SCORE) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxAlmonasterScore, pvRestrictionMax + RESTRICT_ALMONASTER_SCORE);
        RETURN_ON_ERROR(iErrCode);
    }

    // Classic
    if (iOptions & GAME_RESTRICT_MIN_CLASSIC_SCORE) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MinClassicScore, pvRestrictionMin + RESTRICT_CLASSIC_SCORE);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iOptions & GAME_RESTRICT_MAX_CLASSIC_SCORE) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxClassicScore, pvRestrictionMax + RESTRICT_CLASSIC_SCORE);
        RETURN_ON_ERROR(iErrCode);
    }

    // Bridier Rank
    if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MinBridierRank, pvRestrictionMin + RESTRICT_BRIDIER_RANK);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxBridierRank, pvRestrictionMax + RESTRICT_BRIDIER_RANK);
        RETURN_ON_ERROR(iErrCode);
    }

    // Bridier Rank
    if (iOptions & GAME_RESTRICT_MIN_BRIDIER_INDEX) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MinBridierIndex, pvRestrictionMin + RESTRICT_BRIDIER_INDEX);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iOptions & GAME_RESTRICT_MAX_BRIDIER_INDEX) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxBridierIndex, pvRestrictionMax + RESTRICT_BRIDIER_INDEX);
        RETURN_ON_ERROR(iErrCode);
    }

    // Bridier Rank Gain
    if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MinBridierRankGain, pvRestrictionMin + RESTRICT_BRIDIER_RANK_GAIN);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxBridierRankGain, pvRestrictionMax + RESTRICT_BRIDIER_RANK_GAIN);
        RETURN_ON_ERROR(iErrCode);
    }

    // Bridier Rank Loss
    if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MinBridierRankLoss, pvRestrictionMin + RESTRICT_BRIDIER_RANK_LOSS);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxBridierRankLoss, pvRestrictionMax + RESTRICT_BRIDIER_RANK_LOSS);
        RETURN_ON_ERROR(iErrCode);
    }

    // Wins
    if (iOptions & GAME_RESTRICT_MIN_WINS) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MinWins, pvRestrictionMin + RESTRICT_WINS);
        RETURN_ON_ERROR(iErrCode);
    }

    if (iOptions & GAME_RESTRICT_MAX_WINS) {

        iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxWins, pvRestrictionMax + RESTRICT_WINS);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::GameAccessCheck (int iGameClass, int iGameNumber, int iEmpireKey, 
                                 const GameOptions* pgoGameOptions, GameAction gaAction, 
                                 bool* pbAccess, GameAccessDeniedReason* prAccessDeniedReason) {

    int iErrCode, iOptions, iPrivilege;
    unsigned int iKey;

    Variant vGame, vEmpire, vTemp, * pvSec = NULL, vEmpireSecretKey;
    AutoFreeData free(pvSec);

    char pszGameData [256] = "";

    GET_GAME_SECURITY (pszGameSec, iGameClass, iGameNumber);
    GET_GAME_NUKED_EMPIRES (pszDeadEmpires, iGameClass, iGameNumber);
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    bool bFlag;

    ICachedTable* pGameSec = NULL;
    AutoRelease<ICachedTable> rel(pGameSec);

    *pbAccess = false;
    *prAccessDeniedReason = ACCESS_DENIED_NO_REASON;

    // Make sure empire wasn't nuked out of this game
    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::SecretKey, &vEmpireSecretKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->GetFirstKey(pszDeadEmpires, GameNukedEmpires::SecretKey, vEmpireSecretKey, &iKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);
        *prAccessDeniedReason = ACCESS_DENIED_IN_DEAD_EMPIRES_TABLE;
        return OK;
    }

    // Fast resolution for admins, guests
    iErrCode = GetEmpirePrivilege (iEmpireKey, &iPrivilege);
    RETURN_ON_ERROR(iErrCode);

    if (iPrivilege == ADMINISTRATOR) {
        *pbAccess = true;
        return OK;
    }

    if (iPrivilege == GUEST)
    {
        if (gaAction == VIEW_GAME)
        {
            *pbAccess = true;
        }
        else
        {
            *prAccessDeniedReason = ACCESS_DENIED_GUEST_ACCOUNT;
        }
        return OK;
    }

    // Read game options
    if (pgoGameOptions == NULL) {

        // The gameclass already exists, so we can allow owners through quickly
        Variant vGameClassOwner;

        iErrCode = t_pCache->ReadData(
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::Owner, 
            &vGameClassOwner
            );

        RETURN_ON_ERROR(iErrCode);

        if (vGameClassOwner.GetInteger() == iEmpireKey)
        {
            *pbAccess = true;
            return OK;
        }

        COPY_GAME_DATA (pszGameData, iGameClass, iGameNumber);

        iErrCode = t_pCache->ReadData(pszGameData, GameData::Options, &vGame);
        RETURN_ON_ERROR(iErrCode);

        iOptions = vGame.GetInteger();
    
    } else {
        
        iOptions = pgoGameOptions->iOptions;
    }

    // Almonaster
    if (iOptions & (GAME_RESTRICT_MIN_ALMONASTER_SCORE | GAME_RESTRICT_MAX_ALMONASTER_SCORE)) {

        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScore, &vEmpire);
        RETURN_ON_ERROR(iErrCode);

        if (iOptions & GAME_RESTRICT_MIN_ALMONASTER_SCORE) {

            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MinAlmonasterScore, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->fMinAlmonasterScore;
            }

            if (vEmpire.GetFloat() < vGame.GetFloat())
            {
                *prAccessDeniedReason = ACCESS_DENIED_MIN_ALMONASTER_SCORE;
                return OK;
            }
        }
        
        if (iOptions & GAME_RESTRICT_MAX_ALMONASTER_SCORE) {
            
            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxAlmonasterScore, &vGame);
                RETURN_ON_ERROR(iErrCode);
            
            } else {

                vGame = pgoGameOptions->fMaxAlmonasterScore;
            }

            if (vEmpire.GetFloat() > vGame.GetFloat())
            {
                *prAccessDeniedReason = ACCESS_DENIED_MAX_ALMONASTER_SCORE;
                return OK;
            }
        }
    }

    // Classic
    if (iOptions & (GAME_RESTRICT_MIN_CLASSIC_SCORE | GAME_RESTRICT_MAX_CLASSIC_SCORE)) {

        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::ClassicScore, &vEmpire);
        RETURN_ON_ERROR(iErrCode);

        if (iOptions & GAME_RESTRICT_MIN_CLASSIC_SCORE) {

            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MinClassicScore, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->fMinClassicScore;
            }

            if (vEmpire.GetFloat() < vGame.GetFloat())
            {
                *prAccessDeniedReason = ACCESS_DENIED_MIN_CLASSIC_SCORE;
                return OK;
            }
        }

        if (iOptions & GAME_RESTRICT_MAX_CLASSIC_SCORE) {
            
            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxClassicScore, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->fMaxClassicScore;
            }

            if (vEmpire.GetFloat() > vGame.GetFloat())
            {
                *prAccessDeniedReason = ACCESS_DENIED_MAX_CLASSIC_SCORE;
                return OK;
            }
        }
    }

    // Bridier Rank
    if (iOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK | GAME_RESTRICT_MAX_BRIDIER_RANK)) {

        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::BridierRank, &vEmpire);
        RETURN_ON_ERROR(iErrCode);

        if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK) {

            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MinBridierRank, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->iMinBridierRank;
            }

            if (vEmpire.GetInteger() < vGame.GetInteger())
            {
                *prAccessDeniedReason = ACCESS_DENIED_MIN_BRIDIER_RANK;
                return OK;
            }
        }

        if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK) {
            
            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxBridierRank, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->iMaxBridierRank;
            }

            if (vEmpire.GetInteger() > vGame.GetInteger())
            {
                *prAccessDeniedReason = ACCESS_DENIED_MAX_BRIDIER_RANK;
                return OK;
            }
        }
    }

    // Bridier Index
    if (iOptions & (GAME_RESTRICT_MIN_BRIDIER_INDEX | GAME_RESTRICT_MAX_BRIDIER_INDEX)) {

        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::BridierIndex, &vEmpire);
        RETURN_ON_ERROR(iErrCode);

        if (iOptions & GAME_RESTRICT_MIN_BRIDIER_INDEX) {

            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MinBridierIndex, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->iMinBridierIndex;
            }

            if (vEmpire.GetInteger() < vGame.GetInteger())
            {
                *prAccessDeniedReason = ACCESS_DENIED_MIN_BRIDIER_INDEX;
                return OK;
            }
        }

        if (iOptions & GAME_RESTRICT_MAX_BRIDIER_INDEX) {

            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxBridierIndex, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->iMaxBridierIndex;
            }

            if (vEmpire.GetInteger() > vGame.GetInteger())
            {
                *prAccessDeniedReason = ACCESS_DENIED_MAX_BRIDIER_INDEX;
                return OK;
            }
        }
    }

    // Bridier Rank Gain
    if (pgoGameOptions == NULL && iOptions & 
        (GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN | GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN |
         GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS | GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS))
    {
        Variant vGameRank, vGameIndex, vOwnerKey;
        int iNukerRankChange, iNukerIndexChange, iNukedRankChange, iNukedIndexChange, iEmpireRank, iEmpireIndex;

        GET_GAME_EMPIRES(pszGameEmpires, iGameClass, iGameNumber);
        unsigned int iNumRows;
        iErrCode = t_pCache->GetNumCachedRows(pszGameEmpires, &iNumRows);
        RETURN_ON_ERROR(iErrCode);

        if (iNumRows > 0)
        {
            Assert (iNumRows == 1);

            iErrCode = GetBridierScore (iEmpireKey, &iEmpireRank, &iEmpireIndex);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(pszGameEmpires, 0, GameEmpires::EmpireKey, &vOwnerKey);
            RETURN_ON_ERROR(iErrCode);
            
            GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, vOwnerKey.GetInteger());
            
            iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::InitialBridierRank, &vGameRank);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = t_pCache->ReadData(pszEmpireData, GameEmpireData::InitialBridierIndex, &vGameIndex);
            RETURN_ON_ERROR(iErrCode);

            if (iOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN | GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN)) {
            
                // We nuke the empire
                BridierObject::GetScoreChanges (
                    vGameRank.GetInteger(), 
                    vGameIndex.GetInteger(), 
                    iEmpireRank, 
                    iEmpireIndex,
                    &iNukerRankChange, 
                    &iNukerIndexChange, 
                    &iNukedRankChange, 
                    &iNukedIndexChange
                    );
                
                if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN) {
                    
                    iErrCode = t_pCache->ReadData(pszGameData, GameData::MinBridierRankGain, &vGame);
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (iNukerRankChange < vGame.GetInteger()) {
                        *prAccessDeniedReason = ACCESS_DENIED_MIN_BRIDIER_RANK_GAIN;
                        return OK;
                    }
                }
                
                if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN) {
                    
                    iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxBridierRankGain, &vGame);
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (iNukerRankChange > vGame.GetInteger()) {
                        *prAccessDeniedReason = ACCESS_DENIED_MAX_BRIDIER_RANK_GAIN;
                        return OK;
                    }
                }
            }

            if (iOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS | GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS)) {

                // Empire nukes us
                BridierObject::GetScoreChanges (
                    iEmpireRank, 
                    iEmpireIndex,
                    vGameRank.GetInteger(),
                    vGameIndex.GetInteger(),
                    &iNukerRankChange, 
                    &iNukerIndexChange, 
                    &iNukedRankChange, 
                    &iNukedIndexChange
                    );

                if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS) {
                    
                    iErrCode = t_pCache->ReadData(pszGameData, GameData::MinBridierRankLoss, &vGame);
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (-iNukedRankChange < vGame.GetInteger()) {
                        *prAccessDeniedReason = ACCESS_DENIED_MIN_BRIDIER_RANK_LOSS;
                        return OK;
                    }
                }
                
                if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS) {
                    
                    iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxBridierRankLoss, &vGame);
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (-iNukedRankChange > vGame.GetInteger()) {
                        *prAccessDeniedReason = ACCESS_DENIED_MAX_BRIDIER_RANK_LOSS;
                        return OK;
                    }
                }
            }
        }
    }

    // Wins
    if (iOptions & (GAME_RESTRICT_MIN_WINS | GAME_RESTRICT_MAX_WINS)) {

        iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Wins, &vEmpire);
        RETURN_ON_ERROR(iErrCode);

        if (iOptions & GAME_RESTRICT_MIN_WINS) {

            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MinWins, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->iMinWins;
            }

            if (vEmpire.GetInteger() < vGame.GetInteger()) {
                *prAccessDeniedReason = ACCESS_DENIED_MIN_WINS;
                return OK;
            }
        }

        if (iOptions & GAME_RESTRICT_MAX_WINS) {

            if (pgoGameOptions == NULL) {

                iErrCode = t_pCache->ReadData(pszGameData, GameData::MaxWins, &vGame);
                RETURN_ON_ERROR(iErrCode);

            } else {

                vGame = pgoGameOptions->iMaxWins;
            }

            if (vEmpire.GetInteger() > vGame.GetInteger()) {
                *prAccessDeniedReason = ACCESS_DENIED_MAX_WINS;
                return OK;
            }
        }
    }

    // Check for block on specific empire
    if (iOptions & GAME_ENFORCE_SECURITY) {

        Variant vEmpireIPAddress;

        unsigned int iNumSecEntries = 0;

        iKey = NO_KEY;
        while (true)
        {
            int iSecKey = NO_KEY, iSecOptions = 0;

            const char* pszIPAddress = NULL, * pszIPAddress2 = NULL;
            Variant vIPAddress2, * pvIPAddress2 = NULL;

            int64 i64SessionId = NO_SESSION_ID, i64SessionId2 = NO_SESSION_ID, * pi64SessionId2 = NULL;
            int64 i64EmpireSessionId = NO_SESSION_ID, i64SecretKey;
            
            if (pgoGameOptions == NULL) {

                bool bValidKey = false;

                SafeRelease(pGameSec);
                iErrCode = t_pCache->GetTable(pszGameSec, &pGameSec);
                RETURN_ON_ERROR(iErrCode);

                // Fetch a row from the table
                iErrCode = pGameSec->GetNextKey (iKey, &iKey);
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    SafeRelease (pGameSec);
                    iErrCode = OK;
                    break;
                }

                RETURN_ON_ERROR(iErrCode);

                if (pvSec)
                {
                    t_pCache->FreeData(pvSec);
                    pvSec = NULL;
                }
                iErrCode = pGameSec->ReadRow (iKey, &pvSec);
                RETURN_ON_ERROR(iErrCode);

                SafeRelease (pGameSec);

                iSecKey = pvSec[GameSecurity::iEmpireKey].GetInteger();                
                i64SecretKey = pvSec[GameSecurity::iSecretKey].GetInteger64();
                iSecOptions = pvSec[GameSecurity::iOptions].GetInteger();

                if (iSecOptions & GAME_SECURITY_CHECK_SESSIONID) {
                    i64SessionId = pvSec[GameSecurity::iSessionId].GetInteger64();
                    pi64SessionId2 = &i64SessionId2;
                }

                if (iSecOptions & GAME_SECURITY_CHECK_IPADDRESS) {
                    pszIPAddress = pvSec[GameSecurity::iIPAddress].GetCharPtr();
                    pvIPAddress2 = &vIPAddress2;
                }

                if (iSecKey != NO_KEY) {

                    iErrCode = CheckSecretKey (iSecKey, i64SecretKey, &bValidKey, pi64SessionId2, pvIPAddress2);
                    RETURN_ON_ERROR(iErrCode);

                    if (bValidKey) {

                        // i64SessionId2 is already set one way or another

                        if (pvIPAddress2 != NULL) {
                            pszIPAddress2 = vIPAddress2.GetCharPtr();
                        }

                    } else {

                        // Nuke the key in the row
                        iErrCode = t_pCache->WriteData(pszGameSec, iKey, GameSecurity::EmpireKey, (int)NO_KEY);
                        RETURN_ON_ERROR(iErrCode);
                    }
                }
            }
            else
            {
                // Fetch a row from the options menu
                if (iNumSecEntries == pgoGameOptions->iNumSecurityEntries)
                {
                    break;
                }

                iSecKey = pgoGameOptions->pSecurity[iNumSecEntries].iEmpireKey;
                i64SecretKey = pgoGameOptions->pSecurity[iNumSecEntries].iSecretKey;                
                iSecOptions = pgoGameOptions->pSecurity[iNumSecEntries].iOptions;

                // Don't check session ids and ip addresses here
                // The provider of the options pointer is the game creator
                iSecOptions &= ~(GAME_SECURITY_CHECK_SESSIONID | GAME_SECURITY_CHECK_IPADDRESS);

                iNumSecEntries ++;
            }
            
            // Check empire secret key
            if (vEmpireSecretKey.GetInteger64() == i64SecretKey) {
                *prAccessDeniedReason = ACCESS_DENIED_BLOCKED_EMPIRE;
                return OK;
            }
            
            // Check session id match
            if (iSecOptions & GAME_SECURITY_CHECK_SESSIONID) {

                Assert (i64SessionId != NO_SESSION_ID);
                
                // Fault in session id
                if (i64EmpireSessionId == NO_SESSION_ID) {
                    iErrCode = GetEmpireSessionId (iEmpireKey, &i64EmpireSessionId);
                    RETURN_ON_ERROR(iErrCode);
                }
                
                // Check
                if (i64EmpireSessionId == i64SessionId || i64EmpireSessionId == i64SessionId2) {
                    *prAccessDeniedReason = ACCESS_DENIED_BLOCKED_EMPIRE;
                    return OK;
                }
            }

            // Check ip address
            if (iSecOptions & GAME_SECURITY_CHECK_IPADDRESS) {

                Assert (pszIPAddress != NULL);
                
                // Fault in ip address
                if (vEmpireIPAddress.GetType() != V_STRING) {
                    iErrCode = GetEmpireIPAddress (iEmpireKey, &vEmpireIPAddress);
                    RETURN_ON_ERROR(iErrCode);
                }
                
                // Check
                if (String::StrCmp (vEmpireIPAddress.GetCharPtr(), pszIPAddress) == 0 ||
                    String::StrCmp (vEmpireIPAddress.GetCharPtr(), pszIPAddress2) == 0) {
                    *prAccessDeniedReason = ACCESS_DENIED_BLOCKED_EMPIRE;
                    return OK;
                }
            }

        }   // End while loop

    }   // End if enforce per-empire security

    // Check for idle filtering
    if (iOptions & GAME_RESTRICT_IDLE_EMPIRES) {

        iErrCode = IsEmpireIdleInSomeGame (iEmpireKey, &bFlag);
        RETURN_ON_ERROR(iErrCode);

        if (bFlag)
        {
            *prAccessDeniedReason = ACCESS_DENIED_IDLE_EMPIRE;
            return OK;
        }
    }

    // Grant access
    *pbAccess = true;
    Assert(*prAccessDeniedReason == ACCESS_DENIED_NO_REASON);

    return iErrCode;
}