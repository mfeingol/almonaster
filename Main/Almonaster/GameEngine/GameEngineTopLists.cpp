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

// Input:
// iListType -> Type of top list
//
// Output:
// ***pppvData
// [0] -> Keys
// [1] -> Data
//
// Returns the empire keys on a given top list

int GameEngine::GetTopList (ScoringSystem ssListType, Variant*** pppvData, unsigned int* piNumEmpires)
{
    Assert(ssListType >= FIRST_SCORING_SYSTEM && ssListType < NUM_SCORING_SYSTEMS);

    const TemplateDescription* ptTemplate = TOPLIST_TEMPLATE[ssListType];
    const char* pszTableName = TOPLIST_TABLE_NAME[ssListType];

    int iErrCode = t_pCache->ReadColumns(pszTableName, ptTemplate->NumColumns, ptTemplate->ColumnNames, NULL, pppvData, piNumEmpires);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    return iErrCode;
}

int GameEngine::UpdateTopListOnIncrease (ScoringSystem ssTopList, int iEmpireKey) {

    // Prepare the query
    TopListQuery* pQuery = new TopListQuery;
    if (pQuery == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    pQuery->TopList = ssTopList;
    pQuery->EmpireKey = iEmpireKey;

    return global.GetAsyncManager()->QueueTask(UpdateTopListOnIncreaseMsg, pQuery);
}

int GameEngine::UpdateTopListOnDecrease (ScoringSystem ssTopList, int iEmpireKey) {

    // Prepare the query
    TopListQuery* pQuery = new TopListQuery;
    if (pQuery == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    pQuery->TopList = ssTopList;
    pQuery->EmpireKey = iEmpireKey;

    return global.GetAsyncManager()->QueueTask(UpdateTopListOnDecreaseMsg, pQuery);
}

int GameEngine::UpdateTopListOnDeletion (ScoringSystem ssTopList, int iEmpireKey) {

    // Prepare the query
    TopListQuery* pQuery = new TopListQuery;
    if (pQuery == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    pQuery->TopList = ssTopList;
    pQuery->EmpireKey = iEmpireKey;

    return global.GetAsyncManager()->QueueTask(UpdateTopListOnDeletionMsg, pQuery);
}

int GameEngine::UpdateTopListOnIncreaseMsg (AsyncTask* pMessage) {

    TopListQuery* pQuery = (TopListQuery*) pMessage->pArguments;

    GameEngine gameEngine;
    int iErrCode = gameEngine.UpdateTopListOnIncrease (pQuery);

    delete pQuery;
    return iErrCode;
}

int GameEngine::UpdateTopListOnDecreaseMsg (AsyncTask* pMessage) {

    TopListQuery* pQuery = (TopListQuery*) pMessage->pArguments;

    GameEngine gameEngine;
    int iErrCode = gameEngine.UpdateTopListOnDecrease (pQuery);

    delete pQuery;
    return iErrCode;
}

int GameEngine::UpdateTopListOnDeletionMsg (AsyncTask* pMessage) {

    TopListQuery* pQuery = (TopListQuery*) pMessage->pArguments;

    GameEngine gameEngine;
    int iErrCode = gameEngine.UpdateTopListOnDeletion (pQuery);

    delete pQuery;
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

int GameEngine::CacheTopLists(int iEmpireKey)
{
    const TableCacheEntry entries[] =
    {
        { SYSTEM_ALMONASTER_SCORE_TOPLIST, NO_KEY, 0, NULL },
        { SYSTEM_CLASSIC_SCORE_TOPLIST, NO_KEY, 0, NULL },
        { SYSTEM_BRIDIER_SCORE_TOPLIST, NO_KEY, 0, NULL },
        { SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST, NO_KEY, 0, NULL },
        { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL },
    };

    return t_pCache->Cache(entries, countof(entries));
}

int GameEngine::UpdateTopListOnIncrease(TopListQuery* pQuery)
{
    IScoringSystem* pScoringSystem = NULL;
    Variant pvData[MAX_SCORING_SYSTEM_COLUMNS], pvOurData[MAX_SCORING_SYSTEM_COLUMNS];

    int iEmpireKey = pQuery->EmpireKey, iErrCode;
    unsigned int iKey;

    ScoringSystem ssTopList = pQuery->TopList;
    const char* pszTableName = TOPLIST_TABLE_NAME[ssTopList];

    iErrCode = CacheTopLists(iEmpireKey);
    if (iErrCode != OK)
    {
        goto Cleanup;
    }

    pScoringSystem = CreateScoringSystem(ssTopList);
    Assert (pScoringSystem != NULL);

    // Read empire's current score
    iErrCode = pScoringSystem->GetEmpireScore(iEmpireKey, pvOurData);
    if (iErrCode != OK) {
        Assert (false); // Spurious - race with deletion
        goto Cleanup;
    }

    // If empire is on the list, see if we need to shuffle him up
    iErrCode = t_pConn->GetFirstKey(pszTableName, TopList::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == OK) {

        // He's on the list, so move him up if he deserves it
        iErrCode = MoveEmpireUpInTopList (ssTopList, iEmpireKey, iKey, pvOurData);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    else if (iErrCode == ERROR_DATA_NOT_FOUND) {

        // Empire is not on the list, so we need to check if we need to add him to the list
        if (pScoringSystem->IsValidScore (pvOurData)) {

            unsigned int iNumRows;
            
            iErrCode = t_pConn->GetNumRows(pszTableName, &iNumRows);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (iNumRows < TOPLIST_SIZE) {

                iErrCode = AddEmpireToTopList (ssTopList, iEmpireKey, pvOurData);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            
            } else {

                unsigned int i, iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];

                // Read min score to be on list
                for (i = 0; i < iNumColumns; i ++) {
                
                    iErrCode = t_pCache->ReadData(pszTableName, iNumRows - 1, TopList::Data + i, pvData + i);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
                
                if (pScoringSystem->CompareScores (pvOurData, pvData) > 0) {
                    
                    // Woo-hoo, we qualify for the list!
                    iErrCode = AddEmpireToTopList (ssTopList, iEmpireKey, pvOurData);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
            }
        
        } else {

            iErrCode = OK;
        }
    }

    else Assert (false);

Cleanup:

    delete pScoringSystem;

    return iErrCode;
}


int GameEngine::UpdateTopListOnDecrease(TopListQuery* pQuery)
{
    IScoringSystem* pScoringSystem = NULL;
    int iEmpireKey = pQuery->EmpireKey, iErrCode;
    ScoringSystem ssTopList = pQuery->TopList;

    iErrCode = CacheTopLists(iEmpireKey);
    if (iErrCode != OK)
    {
        Assert (false);
        goto Cleanup;
    }

    pScoringSystem = CreateScoringSystem(ssTopList);
    Assert (pScoringSystem != NULL);

    unsigned int iKey;
    const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];

    // If empire is on the list, see if we need to shuffle him down
    iErrCode = t_pConn->GetFirstKey (pszTableName, TopList::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == OK) {

        Variant pvOurData [MAX_SCORING_SYSTEM_COLUMNS];
        
        iErrCode = pScoringSystem->GetEmpireScore (iEmpireKey, pvOurData);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = MoveEmpireDownInTopList (ssTopList, iEmpireKey, iKey, pvOurData);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    else if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    else Assert (false);

Cleanup:

    delete pScoringSystem;

    return iErrCode;
}

int GameEngine::UpdateTopListOnDeletion (TopListQuery* pQuery) {

    Variant** ppvData = NULL;

    unsigned int iKey;
    int iEmpireKey = pQuery->EmpireKey;
    const char* pszTableName = TOPLIST_TABLE_NAME [pQuery->TopList];
    ScoringSystem ssTopList = pQuery->TopList;

    int iErrCode = CacheTopLists(iEmpireKey);
    if (iErrCode != OK)
    {
        Assert (false);
        goto Cleanup;
    }

    // If empire is on the list, we need to shuffle everyone else up
    iErrCode = t_pConn->GetFirstKey (pszTableName, TopList::EmpireKey, iEmpireKey, &iKey);
    if (iErrCode == OK) {

        Variant** ppvStackData;
        unsigned int iNumRows, i, iNumNewRows;
        
        iErrCode = GetTopList(ssTopList, &ppvData, &iNumRows);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        ppvStackData = (Variant**) StackAlloc (max (TOPLIST_SIZE, iNumRows) * sizeof (Variant*));
        memcpy (ppvStackData, ppvData, iNumRows * sizeof (Variant*));

        // Don't need extra padding because we never add new empires in this code path

        // Move everyone else up
        for (i = iKey + 1; i < iNumRows; i ++) {
            ppvStackData[i - 1] = ppvStackData[i];
        }
        ppvStackData[i - 1] = NULL;

        // See if we need to add an empire to fill the new opening
        if (iNumRows == TOPLIST_SIZE) {

            iErrCode = PrivateFindNewEmpireForTopList (
                ssTopList,
                ppvStackData,
                iNumRows - 1,
                false,
                &iNumNewRows
                );

            if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
                Assert (false);
                goto Cleanup;
            }
        
        } else {

            iNumNewRows = iNumRows - 1;
        }

        // Flush changes
        iErrCode = PrivateFlushTopListData (ssTopList, ppvStackData, iNumNewRows);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }
    
    else if (iErrCode == ERROR_DATA_NOT_FOUND) {

        // If empire is not on the list, we're home free
        iErrCode = OK;
    }

Cleanup:

    if (ppvData != NULL)
    {
        t_pConn->FreeData(ppvData);
    }

    return iErrCode;
}

int GameEngine::MoveEmpireUpInTopList(ScoringSystem ssTopList, int iEmpireKey, unsigned int iKey, const Variant* pvOurData) {

    int iErrCode;
    
    unsigned int iNumRows;
    bool bMoved;

    Variant** ppvData = NULL, ** ppvStackData;

    iErrCode = GetTopList(ssTopList, &ppvData, &iNumRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ppvStackData = (Variant**)StackAlloc(max(iNumRows, TOPLIST_SIZE) * sizeof (Variant*));
    if (iNumRows > 0)
    {
        memcpy(ppvStackData, ppvData, iNumRows * sizeof (Variant*));
    }

    // Don't need extra padding because we never add empires in this code path

    iErrCode = PrivateMoveEmpireUpInTopList (
        ssTopList, 
        ppvStackData, 
        iNumRows, 
        iEmpireKey, 
        iKey, 
        pvOurData, 
        &bMoved
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if (bMoved) {
        
        iErrCode = PrivateFlushTopListData (ssTopList, ppvStackData, iNumRows);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    
    } else {

        unsigned int i, iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];
        const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];

        for (i = 0; i < iNumColumns; i ++) {

            iErrCode = t_pConn->WriteData (pszTableName, iKey, TopList::Data + i, pvOurData[i]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:
        
    if (ppvData != NULL) {
        t_pConn->FreeData(ppvData);
    }

    return iErrCode;
}

int GameEngine::PrivateMoveEmpireUpInTopList (ScoringSystem ssTopList, Variant** ppvData, 
                                              unsigned int iNumRows, int iEmpireKey, unsigned int iKey, 
                                              const Variant* pvOurData, bool* pbMoved) {

    int i, iErrCode = OK;

    unsigned int iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];

    Variant* pvTempData;

    IScoringSystem* pScoringSystem = CreateScoringSystem(ssTopList);
    Assert (pScoringSystem != NULL);
    Assert (ppvData[iKey][TopList::iEmpireKey].GetInteger() == iEmpireKey);

    *pbMoved = false;

    // Assign new data
    for (i = 0; i < (int) iNumColumns; i ++) {
        ppvData[iKey][TopList::iData + i] = pvOurData[i];
    }

    // Look for people to replace
    for (i = iKey - 1; i >= 0; i --) {

        if (pScoringSystem->CompareScores (pvOurData, ppvData[i] + TopList::iData) <= 0) {
            break;
        }

        // Swap rows
        pvTempData = ppvData[i];
        ppvData[i] = ppvData[i + 1];
        ppvData[i + 1] = pvTempData;
    }

    if (i != (int) iKey - 1) {
        *pbMoved = true;
    }

    delete pScoringSystem;

    return iErrCode;
}


int GameEngine::MoveEmpireDownInTopList(ScoringSystem ssTopList, int iEmpireKey, unsigned int iKey, const Variant* pvOurData)
{
    int iErrCode;
    unsigned int iNumRows, iNewNumRows;
    bool bChanged;

    Variant** ppvData = NULL, ** ppvStackData;

    iErrCode = GetTopList (ssTopList, &ppvData, &iNumRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ppvStackData = (Variant**) StackAlloc (max (iNumRows, TOPLIST_SIZE) * sizeof (Variant*));
    memcpy (ppvStackData, ppvData, iNumRows * sizeof (Variant*));

    // Don't need extra padding here because we don't add new rows

    iErrCode = PrivateMoveEmpireDownInTopList (
        ssTopList, 
        ppvStackData, 
        iNumRows, 
        iEmpireKey, 
        iKey, 
        pvOurData,
        &iNewNumRows,
        &bChanged
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (bChanged) {

        // Flush the table
        iErrCode = PrivateFlushTopListData (ssTopList, ppvStackData, iNewNumRows);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    
    } else {

        // Update the empire's cached score
        unsigned int i, iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];
        const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];

        for (i = 0; i < iNumColumns; i ++) {

            iErrCode = t_pConn->WriteData (pszTableName, iKey, TopList::Data + i, pvOurData[i]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:
        
    if (ppvData != NULL) {
        t_pConn->FreeData(ppvData);
    }

    return iErrCode;
}


int GameEngine::PrivateMoveEmpireDownInTopList (ScoringSystem ssTopList, Variant** ppvData, 
                                                unsigned int iNumRows, int iEmpireKey, unsigned int iKey, 
                                                const Variant* pvOurData, unsigned int* piNewNumRows,
                                                bool* pbChanged) {

    int iErrCode = OK;
    unsigned int i;

    Variant* pvTempData;

    IScoringSystem* pScoringSystem = CreateScoringSystem (ssTopList);
    Assert (pScoringSystem != NULL);
    Assert (ppvData[iKey][TopList::iEmpireKey].GetInteger() == iEmpireKey);

    *pbChanged = false;
    *piNewNumRows = iNumRows;

    // Maybe empire no longer belongs?
    bool bValid = pScoringSystem->IsValidScore (pvOurData);

    // Assign new data
    for (i = 0; i < TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList]; i ++) {

        if (ppvData[iKey][TopList::iData + i] != pvOurData[i]) {
            ppvData[iKey][TopList::iData + i] = pvOurData[i];
            *pbChanged = true;
        }
    }

    // Move empire down the list
    for (i = iKey + 1; i < iNumRows; i ++) {

        if (bValid && pScoringSystem->CompareScores (pvOurData, ppvData[i] + TopList::iData) >= 0) {
            break;
        }

        Assert (ppvData[i - 1][TopList::iEmpireKey].GetInteger() == iEmpireKey);

        // Swap rows
        pvTempData = ppvData[i];
        ppvData[i] = ppvData[i - 1];
        ppvData[i - 1] = pvTempData;

        *pbChanged = true;
    }

    // If we're not valid, we should have been pushed down to the end of the list
    if (!bValid) {
        Assert (i == iNumRows);
    }

    if (!bValid || i == TOPLIST_SIZE) {
        
        iErrCode = PrivateFindNewEmpireForTopList (ssTopList, ppvData, iNumRows, bValid, piNewNumRows);
        if (iErrCode != OK) {

            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            }
            
            else Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    delete pScoringSystem;

    return iErrCode;
}


int GameEngine::PrivateFindNewEmpireForTopList (ScoringSystem ssTopList, Variant** ppvData, 
                                                unsigned int iNumRows, bool bKeep, unsigned int* piNewNumRows) {

    int iErrCode;

    unsigned int* piKey = NULL, iNumEmpires, i, j, iListKey, iReplacementKey = NO_KEY, iLastRow = NO_KEY;

    const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];
    const unsigned int iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];
    
    Variant pvReplacementData [MAX_SCORING_SYSTEM_COLUMNS], pvData [MAX_SCORING_SYSTEM_COLUMNS];

    IScoringSystem* pScoringSystem = CreateScoringSystem (ssTopList);
    Assert (pScoringSystem != NULL);

    iLastRow = iNumRows - 1;

    if (bKeep) {

        for (i = 0; i < iNumColumns; i ++) {
            pvReplacementData[i] = ppvData [iLastRow][TopList::iData + i];
        }
    }

    // Get potential replacements
    iErrCode = pScoringSystem->GetReplacementKeys (bKeep ? pvReplacementData : NULL, &piKey, &iNumEmpires);
    if (iErrCode != OK) {
        
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        
        else Assert (false);
        goto Cleanup;
    }

    Assert (iNumEmpires > 0);

    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = pScoringSystem->GetEmpireScore (piKey[i], pvData);
        if (iErrCode != OK) {

            if (iErrCode == ERROR_UNKNOWN_ROW_KEY) {
                iErrCode = OK;
                continue;
            }

            Assert (false);
            goto Cleanup;
        }

        if (!pScoringSystem->IsValidScore (pvData)) {
            continue;
        }

        // See if they have a score...
        if (pScoringSystem->CompareScores (pvData, pvReplacementData) > 0) {
            
            // If they're not on the list already...
            iErrCode = t_pConn->GetFirstKey (pszTableName, TopList::EmpireKey, (int) piKey[i], &iListKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {

                iErrCode = OK;
                iReplacementKey = piKey[i];

                for (j = 0; j < iNumColumns; j ++) {
                    pvReplacementData[j] = pvData[j];
                }
            }

            else if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:

    // Okay, we're done with the long part
    delete pScoringSystem;

    if (piKey != NULL) {
        t_pConn->FreeKeys(piKey);
    }

    if (iErrCode != OK) {
        return iErrCode;
    }

    // Now, either we have a replacement, or there is none
    if (bKeep) {

        *piNewNumRows = iNumRows;

        if (iReplacementKey == NO_KEY) {

            // We remain on the list!
            return OK;

        } else {

            bool bMoved;
            
            // Write data to the list
            ppvData[iLastRow][TopList::iEmpireKey] = iReplacementKey;

            //for (j = 0; j < iNumColumns; j ++) {
            //  pvReplacementData[j] = pvReplacementData[j];
            //}

            // Move him up, if he deserves it
            return PrivateMoveEmpireUpInTopList (
                ssTopList,
                ppvData, 
                iLastRow + 1, 
                iReplacementKey, 
                iLastRow, 
                pvReplacementData,
                &bMoved
                );
        }

    } else {

        if (iReplacementKey == NO_KEY) {

            *piNewNumRows = iNumRows - 1;

            ppvData[iLastRow] = NULL;   // Not a leak
            return ERROR_DATA_NOT_FOUND;

        } else {

            bool bMoved;

            *piNewNumRows = iNumRows;

            ppvData[iLastRow][TopList::iEmpireKey] = iReplacementKey;
            
            //for (j = 0; j < iNumColumns; j ++) {
            //  pvReplacementData[j] = pvReplacementData[j];
            //}

            // Move him up, if he deserves it
            return PrivateMoveEmpireUpInTopList (
                ssTopList,
                ppvData, 
                iLastRow + 1, 
                iReplacementKey, 
                iLastRow, 
                pvReplacementData,
                &bMoved
                );
        }
    }
}


int GameEngine::InitializeEmptyTopList (ScoringSystem ssTopList) {

    int iErrCode;
    unsigned int* piKey = NULL, iNumEmpires, iNumRowsFull = 0, i, j;

    Variant pvVariantData [TOPLIST_SIZE * TopList::MaxNumColumns], pvData [MAX_SCORING_SYSTEM_COLUMNS];
    const unsigned int iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];
    
    IScoringSystem* pScoringSystem = CreateScoringSystem (ssTopList);
    Assert (pScoringSystem != NULL);

#ifdef _DEBUG
    const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];
    iErrCode = t_pConn->GetNumRows (pszTableName, &iNumRowsFull);
    Assert (iErrCode == OK && iNumRowsFull == 0);

    iNumRowsFull = 0;

#endif

    Variant* ppvData [TOPLIST_SIZE];
    for (i = 0; i < TOPLIST_SIZE; i ++) {
        ppvData[i] = pvVariantData + i * TopList::MaxNumColumns;
    }

    // Ask scoring system for keys
    iErrCode = pScoringSystem->GetReplacementKeys (NULL, &piKey, &iNumEmpires);
    if (iErrCode != OK) {

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }

        else Assert (false);
        goto Cleanup;
    }

    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = pScoringSystem->GetEmpireScore (piKey[i], pvData);

        if (iNumRowsFull < TOPLIST_SIZE) {

            // Add new entry
            ppvData[iNumRowsFull][TopList::iEmpireKey] = piKey[i];

            for (j = 0; j < iNumColumns; j ++) {
                ppvData[iNumRowsFull][TopList::iData + j] = pvData[j];
            }

            iNumRowsFull ++;
        
        } else {
        
            if (pScoringSystem->CompareScores (pvData, ppvData[TOPLIST_SIZE - 1] + TopList::iData) > 0) {
                
                // Replace last entry
                ppvData[TOPLIST_SIZE - 1][TopList::iEmpireKey] = piKey[i];
                
                for (j = 0; j < iNumColumns; j ++) {
                    ppvData[TOPLIST_SIZE - 1][TopList::iData + j] = pvData[j];
                }
            }

        }
        
        // Move up as far as possible
        j = iNumRowsFull - 1;
        while (j > 0) {

            if (pScoringSystem->CompareScores (pvData, ppvData[j - 1] + TopList::iData) > 0) {
                
                Variant* pvTempData = ppvData[j - 1];
                ppvData[j - 1] = ppvData[j];
                ppvData[j] = pvTempData;

                j --;
            }

            else break;
        }
    }

    if (iNumRowsFull > 0) {

        iErrCode = PrivateFlushTopListData (ssTopList, ppvData, iNumRowsFull);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    // Clean up!
    if (piKey != NULL) {
        t_pConn->FreeKeys(piKey);
    }

    delete pScoringSystem;

    return iErrCode;
}


int GameEngine::AddEmpireToTopList (ScoringSystem ssTopList, int iEmpireKey, const Variant* pvOurData) {

    int iErrCode;
    unsigned int i, iNumEmpires, iNumRealRows;
    bool bChanged;

    Variant** ppvData = NULL, ** ppvStackData, ppvStaticVariant [TOPLIST_SIZE * TopList::MaxNumColumns];

    // Get data from table
    iErrCode = GetTopList (ssTopList, &ppvData, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    ppvStackData = (Variant**) StackAlloc (max (TOPLIST_SIZE, iNumEmpires) * sizeof (Variant*));

    if (iNumEmpires > 0) {
        memcpy (ppvStackData, ppvData, iNumEmpires * sizeof (Variant*));
    }

    if (iNumEmpires == TOPLIST_SIZE) {

        // Replace the last guy
        ppvStackData[TOPLIST_SIZE - 1][TopList::iEmpireKey] = iEmpireKey;
        iNumRealRows = TOPLIST_SIZE;

    } else {

        // Add extra padding
        for (i = iNumEmpires; i < TOPLIST_SIZE; i ++) {
            ppvStackData[i] = ppvStaticVariant + (i - iNumEmpires) * TopList::MaxNumColumns;
        }

        // Add to list
        ppvStackData[iNumEmpires][TopList::iEmpireKey] = iEmpireKey;
        iNumRealRows = iNumEmpires + 1;
    }

    iErrCode = PrivateMoveEmpireUpInTopList (
        ssTopList,
        ppvStackData,
        iNumRealRows,
        iEmpireKey,
        iNumRealRows - 1,
        pvOurData,
        &bChanged
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = PrivateFlushTopListData (ssTopList, ppvStackData, iNumRealRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    if (ppvData != NULL) {
        t_pConn->FreeData(ppvData);
    }

    return iErrCode;
}


int GameEngine::PrivateFlushTopListData (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows) {

    int iErrCode;
    unsigned int i, iNumActualRows;

    const char* pszTable = TOPLIST_TABLE_NAME[ssTopList];
    const TemplateDescription& ttTemplate = *TOPLIST_TEMPLATE[ssTopList];

    IWriteTable* pWriteTable = NULL;

    iErrCode = t_pConn->GetTableForWriting (pszTable, &pWriteTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pWriteTable->GetNumRows (&iNumActualRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iNumRows != iNumActualRows) {

        iErrCode = pWriteTable->DeleteAllRows();
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        for (i = 0; i < iNumRows; i ++) {
            
            iErrCode = pWriteTable->InsertRow(ttTemplate, ppvData[i], NULL);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

    } else {

        for (i = 0; i < iNumRows; i ++) {
            
            iErrCode = pWriteTable->WriteData (i, TopList::EmpireKey, ppvData[i][TopList::iEmpireKey].GetInteger());
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iErrCode = pWriteTable->WriteData (i, TopList::Data, ppvData[i][TopList::iData]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:

    SafeRelease (pWriteTable);

    return iErrCode;
}

bool GameEngine::HasTopList(ScoringSystem ssTopList)
{
    IScoringSystem* pScoringSystem = CreateScoringSystem(ssTopList);
    Assert (pScoringSystem != NULL);
    bool has = pScoringSystem->HasTopList();
    delete pScoringSystem;
    return has;
}

int GameEngine::VerifyTopList (ScoringSystem ssTopList) {

    unsigned int i, j, iTestKey, iNumRows;
    int iErrCode, iEmpireKey;
    Variant vData, vLowerData, pvThisData [MAX_SCORING_SYSTEM_COLUMNS], pvNextData [MAX_SCORING_SYSTEM_COLUMNS];
    bool bExist;

    const char* pszTable = TOPLIST_TABLE_NAME [ssTopList];
    unsigned int iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];

    IScoringSystem* pScoringSystem = CreateScoringSystem (ssTopList);
    Assert (pScoringSystem != NULL);

    iErrCode = t_pConn->GetNumRows (pszTable, &iNumRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iNumRows > TOPLIST_SIZE) {
        iErrCode = ERROR_TOPLIST_TOO_LARGE;
        goto Cleanup;
    }

    // Verify data in list
    for (i = 0; i < iNumRows; i ++) {
        
        // If row doesn't exist, the table is corrupt
        iErrCode = t_pConn->DoesRowExist (pszTable, i, &bExist);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (!bExist) {
            iErrCode = ERROR_TOPLIST_CORRUPT;
            goto Cleanup;
        }

        // If empire doesn't exist, the table is corrupt
        iErrCode = t_pCache->ReadData(pszTable, i, TopList::EmpireKey, &vData);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iEmpireKey = vData.GetInteger();

        iErrCode = t_pConn->DoesRowExist (SYSTEM_EMPIRE_DATA, iEmpireKey, &bExist);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (!bExist) {
            iErrCode = ERROR_TOPLIST_CORRUPT;
            goto Cleanup;
        }

        // If key is in table twice, the table is corrupt
        iErrCode = t_pConn->GetFirstKey (pszTable, TopList::EmpireKey, iEmpireKey, &iTestKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        if (iTestKey != i) {
            iErrCode = ERROR_TOPLIST_CORRUPT;
            goto Cleanup;
        }

        // If our score doesn't match our empire, the table is corrupt
        for (j = 0; j < iNumColumns; j ++) {

            iErrCode = t_pCache->ReadData(pszTable, i, TopList::Data + j, pvThisData + j);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        iErrCode = pScoringSystem->GetEmpireScore (iEmpireKey, pvNextData);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (pScoringSystem->CompareScores (pvThisData, pvNextData) != 0) {
            iErrCode = ERROR_TOPLIST_CORRUPT;
            goto Cleanup;
        }

        if (i < iNumRows - 1) {

            // If we have a lower score than the next guy, the table is corrupt
            unsigned int iNextRow = i + 1;

            iErrCode = t_pConn->DoesRowExist (pszTable, iNextRow, &bExist);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            if (!bExist) {
                iErrCode = ERROR_TOPLIST_CORRUPT;
                goto Cleanup;
            }
            
            for (j = 0; j < iNumColumns; j ++) {
                
                iErrCode = t_pCache->ReadData(pszTable, iNextRow, TopList::Data + j, pvNextData + j);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }

            if (pScoringSystem->CompareScores (pvThisData, pvNextData) < 0) {
                iErrCode = ERROR_TOPLIST_CORRUPT;
                goto Cleanup;
            }
        }
    }

Cleanup:

    delete pScoringSystem;

    return iErrCode;
}