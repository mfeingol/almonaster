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
#include "Global.h"

struct DatabasePurge {
    int Criteria; 
    int EmpireKey;
};

// Output:
// *piNumEmpires -> Number of empires deleted
//
// Remove all non-Admin empires with one of the following two characteristics:
// 1) MaxEcon == 0 (means that they haven't even played a single update)
// 2) LastLogin is more than thirty days ago
//
// Run this with caution  :-)

int GameEngine::PurgeDatabase (int iEmpireKey, int iCriteria) {

    DatabasePurge* pPurgeInfo = new DatabasePurge;
    if (pPurgeInfo == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    pPurgeInfo->Criteria = iCriteria; 
    pPurgeInfo->EmpireKey = iEmpireKey;

    return global.GetAsyncManager()->QueueTask(PurgeDatabaseMsg, pPurgeInfo);
}


int GameEngine::PurgeDatabaseMsg(AsyncTask* pMessage) {

    DatabasePurge* pPurgeInfo = (DatabasePurge*)pMessage->pArguments;

    GameEngine gameEngine;
    int iErrCode = gameEngine.PurgeDatabasePrivate(pPurgeInfo->EmpireKey, pPurgeInfo->Criteria);

    delete pPurgeInfo;
    return iErrCode;
}

int GameEngine::PurgeDatabasePrivate (int iEmpireKey, int iCriteria) {

    int iErrCode = OK;
    unsigned int iNumEmpiresDeleted = 0;

    char pszText [256];

    if ((iCriteria & (~TEST_PURGE_ONLY)) != 0) {

        const Seconds sThirtyDays = 30 * 24 * 60 * 60;
        unsigned int iEmpireKey = NO_KEY;

        ICachedTable* pEmpires = NULL;

        int iValue;
        float fValue;
        UTCTime tValue, tNow;

        Time::GetTime (&tNow);

        while (true) {

            int64 i64SecretKey;

            SafeRelease (pEmpires);

            iErrCode = t_pCache->GetTable(SYSTEM_EMPIRE_DATA, &pEmpires);
            if (iErrCode != OK) {
                Assert (false);
                continue;
            }

            iErrCode = pEmpires->GetNextKey (iEmpireKey, &iEmpireKey);
            if (iErrCode != OK) {
                if (iErrCode == ERROR_DATA_NOT_FOUND) {
                    iErrCode = OK;
                } else Assert (false);
                break;
            }

            // Never purge administrators
            iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::Privilege, &iValue);
            if (iErrCode != OK || iValue >= ADMINISTRATOR) {
                continue;
            }
            
            // Never played a game
            if (iCriteria & NEVER_PLAYED_A_GAME) {
                
                // If max econ = 0, then this empire never played a game
                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::MaxEcon, &iValue);
                if (iErrCode != OK || iValue > 0) {
                    continue;
                }
            }
            
            // Never won a game
            if (iCriteria & NEVER_WON_A_GAME) {

                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::Wins, &iValue);
                if (iErrCode != OK || iValue > 0) {
                    continue;
                }
            }
            
            // Only logged in once
            if (iCriteria & ONLY_ONE_LOGIN) {

                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::NumLogins, &iValue);
                if (iErrCode != OK || iValue > 1) {
                    continue;
                }
            }
            
            // Bad score
            if (iCriteria & CLASSIC_SCORE_IS_ZERO_OR_LESS) {
                
                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::ClassicScore, &fValue);
                if (iErrCode != OK || fValue <= 0) {
                    continue;
                }
            }
            
            // Last logged in a while ago
            if (iCriteria & LAST_LOGGED_IN_1_MONTH_AGO) {

                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::LastLoginTime, &tValue);
                if (iErrCode != OK || Time::GetSecondDifference (tNow, tValue) < sThirtyDays) {
                    continue;
                }
            }
            if (iCriteria & LAST_LOGGED_IN_3_MONTHS_AGO) {

                iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::LastLoginTime, &tValue);
                if (iErrCode != OK || Time::GetSecondDifference (tNow, tValue) < 3 * sThirtyDays) {
                    continue;
                }
            }

            // Read secret key
            iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::SecretKey, &i64SecretKey);
            if (iErrCode != OK) {
                continue;
            }

            SafeRelease (pEmpires);

            // Not on top lists
            if (iCriteria & NOT_ON_TOP_LISTS) {

                bool bOnTopList = false;
                int i;
                ENUMERATE_SCORING_SYSTEMS (i) {

                    ScoringSystem ssTopList = (ScoringSystem) i;
                    if (HasTopList (ssTopList)) {

                        const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];

                        unsigned int iKey;
                        iErrCode = t_pCache->GetFirstKey(
                            pszTableName,
                            TopList::EmpireKey,
                            iEmpireKey,
                            &iKey
                            );

                        if (iErrCode == ERROR_DATA_NOT_FOUND) {
                            iErrCode = OK;
                        } else {
                            Assert (iErrCode == OK);
                            bOnTopList = true;
                            break;
                        }
                    }
                }

                if (bOnTopList) {
                    continue;
                }
            }

            int iNumGames, * piGameClass, * piGameNumber;
            iErrCode = GetEmpireActiveGames(iEmpireKey, &piGameClass, &piGameNumber, &iNumGames);
            if (iErrCode == OK)
            {
                delete [] piGameClass;
                delete [] piGameNumber;
            }

            if (iErrCode != OK || iNumGames > 0)
            {
                continue;
            }
            
            // Best effort delete the empire
            if (!(iCriteria & TEST_PURGE_ONLY)) {
                iErrCode = DeleteEmpire (iEmpireKey, &i64SecretKey, true, false);
            }
            
            if (iErrCode == OK) {
                iNumEmpiresDeleted ++;
            }
        }

        SafeRelease (pEmpires);
        iErrCode = OK;
    }

    // Send message to originator of purge       
    if (iCriteria & TEST_PURGE_ONLY) {

        if (iNumEmpiresDeleted == 1) {
            strcpy (pszText, "1 empire would have been purged from the database");
        } else {
            sprintf (pszText, "%i empires would have been purged from the database", iNumEmpiresDeleted);
        }

    } else {

        if (iNumEmpiresDeleted == 1) {
            strcpy (pszText, "1 empire was purged from the database");
        } else {
            sprintf (pszText, "%i empires were purged from the database", iNumEmpiresDeleted);
        }
    }

    // Best effort send message
    SendSystemMessage (iEmpireKey, pszText, SYSTEM, MESSAGE_SYSTEM);

    return iErrCode;
}