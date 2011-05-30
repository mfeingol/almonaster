//
// GameEngine.dll:  a component of Almonaster 2.0
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

    GAME_DATA (pszGameData, iGameClass, iGameNumber);

    // Read game options
    iErrCode = m_pGameData->ReadData (pszGameData, GameData::Options, &vOptions);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iOptions = *piOptions = vOptions.GetInteger();

    // Almonaster
    if (iOptions & GAME_RESTRICT_MIN_ALMONASTER_SCORE) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinAlmonasterScore, pvRestrictionMin + RESTRICT_ALMONASTER_SCORE);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    if (iOptions & GAME_RESTRICT_MAX_ALMONASTER_SCORE) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxAlmonasterScore, pvRestrictionMax + RESTRICT_ALMONASTER_SCORE);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    // Classic
    if (iOptions & GAME_RESTRICT_MIN_CLASSIC_SCORE) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinClassicScore, pvRestrictionMin + RESTRICT_CLASSIC_SCORE);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    if (iOptions & GAME_RESTRICT_MAX_CLASSIC_SCORE) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxClassicScore, pvRestrictionMax + RESTRICT_CLASSIC_SCORE);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    // Bridier Rank
    if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRank, pvRestrictionMin + RESTRICT_BRIDIER_RANK);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRank, pvRestrictionMax + RESTRICT_BRIDIER_RANK);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    // Bridier Rank
    if (iOptions & GAME_RESTRICT_MIN_BRIDIER_INDEX) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierIndex, pvRestrictionMin + RESTRICT_BRIDIER_INDEX);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    if (iOptions & GAME_RESTRICT_MAX_BRIDIER_INDEX) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierIndex, pvRestrictionMax + RESTRICT_BRIDIER_INDEX);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    // Bridier Rank Gain
    if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRankGain, pvRestrictionMin + RESTRICT_BRIDIER_RANK_GAIN);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRankGain, pvRestrictionMax + RESTRICT_BRIDIER_RANK_GAIN);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    // Bridier Rank Loss
    if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRankLoss, pvRestrictionMin + RESTRICT_BRIDIER_RANK_LOSS);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRankLoss, pvRestrictionMax + RESTRICT_BRIDIER_RANK_LOSS);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    // Wins
    if (iOptions & GAME_RESTRICT_MIN_WINS) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinWins, pvRestrictionMin + RESTRICT_WINS);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    if (iOptions & GAME_RESTRICT_MAX_WINS) {

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxWins, pvRestrictionMax + RESTRICT_WINS);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

Cleanup:

    return iErrCode;
}

int GameEngine::GameAccessCheck (int iGameClass, int iGameNumber, int iEmpireKey, 
                                 const GameOptions* pgoGameOptions, GameAction gaAction, 
                                 bool* pbAccess) {

    int iErrCode, iOptions, iPrivilege;
    unsigned int iKey;

    Variant vGame, vEmpire, vTemp, * pvSec = NULL, vEmpireSecretKey;

    char pszGameData [256] = "";

    GAME_SECURITY (pszGameSec, iGameClass, iGameNumber);
    GAME_DEAD_EMPIRES (pszDeadEmpires, iGameClass, iGameNumber);

    bool bFlag;

    IReadTable* pGameSec = NULL;

    // Make sure empire wasn't nuked out of this game
    iErrCode = GetEmpireProperty (iEmpireKey, SystemEmpireData::SecretKey, &vEmpireSecretKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->GetFirstKey (pszDeadEmpires, GameDeadEmpires::SecretKey, vEmpireSecretKey, false, &iKey);
    if (iErrCode != ERROR_DATA_NOT_FOUND && iErrCode != ERROR_UNKNOWN_TABLE_NAME) {
        *pbAccess = false;
        goto Cleanup;
    }

    // Fast resolution for admins, guests
    iErrCode = GetEmpirePrivilege (iEmpireKey, &iPrivilege);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (iPrivilege == ADMINISTRATOR) {
        *pbAccess = true;
        goto Cleanup;
    }

    if (iPrivilege == GUEST) {
        *pbAccess = gaAction == VIEW_GAME;
        goto Cleanup;
    }

    // Read game options
    if (pgoGameOptions == NULL) {

        // The gameclass already exists, so we can allow owners through quickly
        Variant vGameClassOwner;

        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::Owner, 
            &vGameClassOwner
            );

        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (vGameClassOwner.GetInteger() == iEmpireKey) {
            *pbAccess = true;
            goto Cleanup;
        }

        GET_GAME_DATA (pszGameData, iGameClass, iGameNumber);

        iErrCode = m_pGameData->ReadData (pszGameData, GameData::Options, &vGame);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        iOptions = vGame.GetInteger();
    
    } else {
        
        iOptions = pgoGameOptions->iOptions;
    }

    // Default to allow access
    *pbAccess = true;

    // Almonaster
    if (iOptions & (GAME_RESTRICT_MIN_ALMONASTER_SCORE | GAME_RESTRICT_MAX_ALMONASTER_SCORE)) {

        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vEmpire);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iOptions & GAME_RESTRICT_MIN_ALMONASTER_SCORE) {

            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinAlmonasterScore, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->fMinAlmonasterScore;
            }

            if (vEmpire.GetFloat() < vGame.GetFloat()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }
        
        if (iOptions & GAME_RESTRICT_MAX_ALMONASTER_SCORE) {
            
            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxAlmonasterScore, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }
            
            } else {

                vGame = pgoGameOptions->fMaxAlmonasterScore;
            }

            if (vEmpire.GetFloat() > vGame.GetFloat()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }
    }

    // Classic
    if (iOptions & (GAME_RESTRICT_MIN_CLASSIC_SCORE | GAME_RESTRICT_MAX_CLASSIC_SCORE)) {

        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::ClassicScore, &vEmpire);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iOptions & GAME_RESTRICT_MIN_CLASSIC_SCORE) {

            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinClassicScore, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->fMinClassicScore;
            }

            if (vEmpire.GetFloat() < vGame.GetFloat()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }

        if (iOptions & GAME_RESTRICT_MAX_CLASSIC_SCORE) {
            
            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxClassicScore, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->fMaxClassicScore;
            }

            if (vEmpire.GetFloat() > vGame.GetFloat()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }
    }

    // Bridier Rank
    if (iOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK | GAME_RESTRICT_MAX_BRIDIER_RANK)) {

        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierRank, &vEmpire);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK) {

            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRank, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->iMinBridierRank;
            }

            if (vEmpire.GetInteger() < vGame.GetInteger()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }

        if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK) {
            
            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRank, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->iMaxBridierRank;
            }

            if (vEmpire.GetInteger() > vGame.GetInteger()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }
    }

    // Bridier Index
    if (iOptions & (GAME_RESTRICT_MIN_BRIDIER_INDEX | GAME_RESTRICT_MAX_BRIDIER_INDEX)) {

        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::BridierIndex, &vEmpire);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iOptions & GAME_RESTRICT_MIN_BRIDIER_INDEX) {

            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierIndex, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->iMinBridierIndex;
            }

            if (vEmpire.GetInteger() < vGame.GetInteger()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }

        if (iOptions & GAME_RESTRICT_MAX_BRIDIER_INDEX) {

            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierIndex, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->iMaxBridierIndex;
            }

            if (vEmpire.GetInteger() > vGame.GetInteger()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }
    }

    // Bridier Rank Gain
    if (pgoGameOptions == NULL &&
        iOptions & 
        (GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN | GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN |
         GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS | GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS)) {

        Variant vGameRank, vGameIndex, vOwnerKey;
        int iNukerRankChange, iNukerIndexChange, iNukedRankChange, iNukedIndexChange, iEmpireRank, iEmpireIndex;

        GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

        unsigned int iNumRows;

        iErrCode = m_pGameData->GetNumRows (pszGameEmpires, &iNumRows);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iNumRows > 0) {

            Assert (iNumRows == 1);

            iErrCode = GetBridierScore (iEmpireKey, &iEmpireRank, &iEmpireIndex);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (pszGameEmpires, 0, GameEmpires::EmpireKey, &vOwnerKey);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            
            GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, vOwnerKey.GetInteger());
            
            iErrCode = m_pGameData->ReadData (pszEmpireData, GameEmpireData::InitialBridierRank, &vGameRank);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (pszEmpireData, GameEmpireData::InitialBridierIndex, &vGameIndex);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

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
                    
                    iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRankGain, &vGame);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                    
                    if (iNukerRankChange < vGame.GetInteger()) {
                        *pbAccess = false;
                        goto Cleanup;
                    }
                }
                
                if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN) {
                    
                    iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRankGain, &vGame);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                    
                    if (iNukerRankChange > vGame.GetInteger()) {
                        *pbAccess = false;
                        goto Cleanup;
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
                    
                    iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinBridierRankLoss, &vGame);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                    
                    if (-iNukedRankChange < vGame.GetInteger()) {
                        *pbAccess = false;
                        goto Cleanup;
                    }
                }
                
                if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS) {
                    
                    iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxBridierRankLoss, &vGame);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                    
                    if (-iNukedRankChange > vGame.GetInteger()) {
                        *pbAccess = false;
                        goto Cleanup;
                    }
                }
            }
        }
    }

    // Wins
    if (iOptions & (GAME_RESTRICT_MIN_WINS | GAME_RESTRICT_MAX_WINS)) {

        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Wins, &vEmpire);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iOptions & GAME_RESTRICT_MIN_WINS) {

            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MinWins, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->iMinWins;
            }

            if (vEmpire.GetInteger() < vGame.GetInteger()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }

        if (iOptions & GAME_RESTRICT_MAX_WINS) {

            if (pgoGameOptions == NULL) {

                iErrCode = m_pGameData->ReadData (pszGameData, GameData::MaxWins, &vGame);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

            } else {

                vGame = pgoGameOptions->iMaxWins;
            }

            if (vEmpire.GetInteger() > vGame.GetInteger()) {
                *pbAccess = false;
                goto Cleanup;
            }
        }
    }

    // Check for idle filtering
    if (iOptions & GAME_RESTRICT_IDLE_EMPIRES) {

        iErrCode = IsEmpireIdleInSomeGame (iEmpireKey, &bFlag);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (bFlag) {
            *pbAccess = false;
            goto Cleanup;
        }
    }

    // Check for block on specific empire
    if (iOptions & GAME_ENFORCE_SECURITY) {

        Variant vEmpireIPAddress;

        unsigned int iNumSecEntries = 0;

        iKey = NO_KEY;
        while (true) {

            int iSecKey = NO_KEY, iSecOptions = 0;

            const char* pszIPAddress = NULL, * pszIPAddress2 = NULL;
            Variant vIPAddress2, * pvIPAddress2 = NULL;

            int64 i64SessionId = NO_SESSION_ID, i64SessionId2 = NO_SESSION_ID, * pi64SessionId2 = NULL;
            int64 i64EmpireSessionId = NO_SESSION_ID, i64SecretKey;
            
            if (pgoGameOptions == NULL) {

                bool bValidKey = false;

                iErrCode = m_pGameData->GetTableForReading (pszGameSec, &pGameSec);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

                // Fetch a row from the table
                iErrCode = pGameSec->GetNextKey (iKey, &iKey);
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    SafeRelease (pGameSec);
                    iErrCode = OK;
                    break;
                }

                if (iErrCode != OK) {
                    goto Cleanup;
                }

                iErrCode = pGameSec->ReadRow (iKey, &pvSec);
                if (iErrCode != OK) {
                    goto Cleanup;
                }

                SafeRelease (pGameSec);

                iSecKey = pvSec [GameSecurity::EmpireKey].GetInteger();                
                i64SecretKey = pvSec [GameSecurity::SecretKey].GetInteger64();
                iSecOptions = pvSec [GameSecurity::Options].GetInteger();

                if (iSecOptions & GAME_SECURITY_CHECK_SESSIONID) {
                    i64SessionId = pvSec [GameSecurity::SessionId].GetInteger64();
                    pi64SessionId2 = &i64SessionId2;
                }

                if (iSecOptions & GAME_SECURITY_CHECK_IPADDRESS) {
                    pszIPAddress = pvSec [GameSecurity::IPAddress].GetCharPtr();
                    pvIPAddress2 = &vIPAddress2;
                }

                if (iSecKey != NO_KEY) {

                    iErrCode = CheckSecretKey (iSecKey, i64SecretKey, &bValidKey, pi64SessionId2, pvIPAddress2);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }

                    if (bValidKey) {

                        // i64SessionId2 is already set one way or another

                        if (pvIPAddress2 != NULL) {
                            pszIPAddress2 = vIPAddress2.GetCharPtr();
                        }

                    } else {

                        // Nuke the key in the row
                        iErrCode = m_pGameData->WriteData (pszGameSec, iKey, GameSecurity::EmpireKey, NO_KEY);
                        if (iErrCode != OK) {
                            goto Cleanup;
                        }
                    }
                }

            } else {

                // Fetch a row from the options menu
                if (iNumSecEntries == pgoGameOptions->iNumSecurityEntries) {
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
                *pbAccess = false;
                break;
            }
            
            // Check session id match
            if (iSecOptions & GAME_SECURITY_CHECK_SESSIONID) {

                Assert (i64SessionId != NO_SESSION_ID);
                
                // Fault in session id
                if (i64EmpireSessionId == NO_SESSION_ID) {
                    iErrCode = GetEmpireSessionId (iEmpireKey, &i64EmpireSessionId);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                }
                
                // Check
                if (i64EmpireSessionId == i64SessionId || i64EmpireSessionId == i64SessionId2) {
                    *pbAccess = false;
                    break;
                }
            }

            // Check ip address
            if (iSecOptions & GAME_SECURITY_CHECK_IPADDRESS) {

                Assert (pszIPAddress != NULL);
                
                // Fault in ip address
                if (vEmpireIPAddress.GetType() != V_STRING) {
                    iErrCode = GetEmpireIPAddress (iEmpireKey, &vEmpireIPAddress);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                }
                
                // Check
                if (String::StrCmp (vEmpireIPAddress.GetCharPtr(), pszIPAddress) == 0 ||
                    String::StrCmp (vEmpireIPAddress.GetCharPtr(), pszIPAddress2) == 0) {
                    *pbAccess = false;
                    break;
                }
            }

            if (pvSec != NULL) {
                    m_pGameData->FreeData (pvSec);
                    pvSec = NULL;
                }

        }   // End while loop

    }   // End if enforce security

Cleanup:

    SafeRelease (pGameSec);

    if (pvSec != NULL) {
        m_pGameData->FreeData (pvSec);
    }

    return iErrCode;
}