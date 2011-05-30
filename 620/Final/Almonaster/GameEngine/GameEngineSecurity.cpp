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
    Variant vGame, vEmpire, vTemp, * pvSec = NULL;

    char pszGameData [256] = "";
    GAME_SECURITY (pszGameSec, iGameClass, iGameNumber);

    IReadTable* pGameSec = NULL;

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

        bool fIdle = false;
        iErrCode = IsEmpireIdleInSomeGame (iEmpireKey, &fIdle);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (fIdle) {
            *pbAccess = false;
            goto Cleanup;
        }
    }

    // Check for block on specific empire
    if (iOptions & GAME_ENFORCE_SECURITY) {

        LinkedList<unsigned int> llBrokenList;

        unsigned int iKey = NO_KEY, iNumSecEntries = 0;

        int iSecKey, iSecOptions;
        int64 i64SessionId = NO_SESSION_ID, i64EmpireSessionId = NO_SESSION_ID;
        const char* pszIPAddress = NULL, * pszEmpireName = NULL;

        bool bFlag;

        Variant vEmpireIPAddress, vNewIPAddress, vNewSessionId;

        while (true) {
            
            if (pgoGameOptions == NULL) {

                bool bBroken = false;

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
                if (iSecKey != NO_KEY) {

                    pszEmpireName = pvSec [GameSecurity::EmpireName].GetCharPtr();

                    iErrCode = DoesEmpireKeyMatchName (iSecKey, pszEmpireName, &bFlag);
                    if (iErrCode != OK || !bFlag) {
                        iSecKey = NO_KEY;
                        bBroken = true;                     
                    }
                }
                
                iSecOptions = pvSec [GameSecurity::Options].GetInteger();
                if (iSecOptions & GAME_SECURITY_CHECK_SESSIONID) {

                    i64SessionId = pvSec [GameSecurity::SessionId].GetInteger64();
                    if (iSecKey != NO_KEY) {

                        // See if empire's session id is still valid
                        iErrCode = m_pGameData->ReadData (
                            SYSTEM_EMPIRE_DATA,
                            iSecKey,
                            SystemEmpireData::SessionId,
                            &vNewSessionId
                            );
                        
                        if (iErrCode != OK) {
                            iSecKey = NO_KEY;
                            iErrCode = OK;
                            bBroken = true;
                        }
                        
                        else if (i64SessionId != vNewSessionId.GetInteger64()) {
                            bBroken = true;
                            i64SessionId = vNewSessionId.GetInteger64();
                        }
                    }
                }
                
                if (iSecOptions & GAME_SECURITY_CHECK_IPADDRESS) {

                    pszIPAddress = pvSec [GameSecurity::IPAddress].GetCharPtr();

                    if (iSecKey != NO_KEY) {

                        // See if empire's ip address is still valid
                        iErrCode = m_pGameData->ReadData (
                            SYSTEM_EMPIRE_DATA,
                            iSecKey,
                            SystemEmpireData::IPAddress,
                            &vNewIPAddress
                            );

                        if (iErrCode != OK) {
                            iSecKey = NO_KEY;
                            iErrCode = OK;
                            bBroken = true;                     
                        }
                        
                        else if (strcmp (pszIPAddress, vNewIPAddress.GetCharPtr()) != 0) {
                            bBroken = true;
                            pszIPAddress = vNewIPAddress.GetCharPtr();
                        }
                    }
                }
                
                // If the row is broken, take note
                if (bBroken) {

                    if (!llBrokenList.PushLast (iKey)) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                }

                if (pvSec != NULL) {
                    m_pGameData->FreeData (pvSec);
                    pvSec = NULL;
                }

            } else {

                // Fetch a row from the options menu
                if (iNumSecEntries == pgoGameOptions->iNumSecurityEntries) {
                    break;
                }

                iSecKey = pgoGameOptions->pSecurity[iNumSecEntries].iEmpireKey;
                iSecOptions = pgoGameOptions->pSecurity[iNumSecEntries].iOptions;

                // Don't check session ids and ip addresses here
                iSecOptions &= ~(GAME_SECURITY_CHECK_SESSIONID | GAME_SECURITY_CHECK_IPADDRESS);

                iNumSecEntries ++;
            }
            
            // Check empire key
            if (iEmpireKey == iSecKey) {
                
                // Access denied
                *pbAccess = false;
                break;
            }
            
            // Check session id
            if (iSecOptions & GAME_SECURITY_CHECK_SESSIONID) {

                Assert (i64SessionId != NO_SESSION_ID);
                
                // Fault in session id
                if (i64EmpireSessionId == NO_SESSION_ID) {
                    
                    iErrCode = GetEmpireSessionId (iEmpireKey, &i64EmpireSessionId);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                }
                
                if (i64EmpireSessionId == i64SessionId) {
                    
                    // Access denied
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
                
                if (strcmp (vEmpireIPAddress.GetCharPtr(), pszIPAddress) == 0) {
                    
                    // Access denied
                    *pbAccess = false;
                    break;
                }
            }

        }   // End while loop

        if (llBrokenList.GetNumElements() > 0) {

            ListIterator<unsigned int> li;

            GAME_SECURITY (pszGameSec, iGameClass, iGameNumber);

            while (llBrokenList.PopFirst (&li)) {

                iKey = li.GetData();

                iErrCode = m_pGameData->ReadData (pszGameSec, iKey, GameSecurity::EmpireKey, &vTemp);
                if (iErrCode != OK) {
                    goto Cleanup;
                }
                iSecKey = vTemp.GetInteger();

                if (iSecKey != NO_KEY) {

                    iErrCode = m_pGameData->ReadData (pszGameSec, iKey, GameSecurity::EmpireName, &vTemp);
                    if (iErrCode != OK) {
                        goto Cleanup;
                    }
                    
                    pszEmpireName = vTemp.GetCharPtr();
                    
                    // See if empire's ip address is still valid
                    iErrCode = DoesEmpireKeyMatchName (iSecKey, pszEmpireName, &bFlag);
                    if (iErrCode != OK || !bFlag) {
                        
                        // No more empire key
                        iErrCode = m_pGameData->WriteData (pszGameSec, iKey, GameSecurity::EmpireKey, (int) NO_KEY);
                        if (iErrCode != OK) {
                            goto Cleanup;
                        }
                    }
                    
                    else if (bFlag) {
                        
                        iErrCode = m_pGameData->ReadData (
                            SYSTEM_EMPIRE_DATA,
                            iSecKey,
                            SystemEmpireData::SessionId,
                            &vNewSessionId
                            );
                        
                        if (iErrCode != OK) {
                            
                            // No more empire key
                            iErrCode = m_pGameData->WriteData (pszGameSec, iKey, GameSecurity::EmpireKey, (int) NO_KEY);
                            if (iErrCode != OK) {
                                goto Cleanup;
                            }
                            
                        } else {
                            
                            iErrCode = m_pGameData->ReadData (pszGameSec, iKey, GameSecurity::SessionId, &vTemp);
                            if (iErrCode != OK) {
                                goto Cleanup;
                            }
                            i64SessionId = vTemp.GetInteger64();
                            
                            if (i64SessionId != vNewSessionId.GetInteger64()) {
                                
                                // New Session Id
                                iErrCode = m_pGameData->WriteData (pszGameSec, iKey, GameSecurity::SessionId, vNewSessionId);
                                if (iErrCode != OK) {
                                    goto Cleanup;
                                }
                            }
                            
                            iErrCode = m_pGameData->ReadData (
                                SYSTEM_EMPIRE_DATA,
                                iSecKey,
                                SystemEmpireData::IPAddress,
                                &vNewIPAddress
                                );
                            
                            if (iErrCode != OK) {
                                
                                // No more empire key
                                iErrCode = m_pGameData->WriteData (pszGameSec, iKey, GameSecurity::EmpireKey, (int) NO_KEY);
                                if (iErrCode != OK) {
                                    goto Cleanup;
                                }
                                
                            } else {
                                
                                iErrCode = m_pGameData->ReadData (pszGameSec, iKey, GameSecurity::IPAddress, &vTemp);
                                if (iErrCode != OK) {
                                    goto Cleanup;
                                }
                                pszIPAddress = vTemp.GetCharPtr();
                                
                                if (strcmp (pszIPAddress, vNewIPAddress.GetCharPtr()) != 0) {
                                    
                                    // New IP address
                                    iErrCode = m_pGameData->WriteData (pszGameSec, iKey, GameSecurity::IPAddress, vNewIPAddress.GetCharPtr());
                                    if (iErrCode != OK) {
                                        goto Cleanup;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            llBrokenList.Clear();
        
        }   // End if any rows were broken

    }   // End if enforce security

Cleanup:

    SafeRelease (pGameSec);

    if (pvSec != NULL) {
        m_pGameData->FreeData (pvSec);
    }

    return iErrCode;
}