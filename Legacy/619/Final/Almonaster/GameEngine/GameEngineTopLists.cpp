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

#include <math.h>

#include "GameEngine.h"


// Input:
// iListType -> Type of top list
//
// Output:
// ***pppvData
// [0] -> Keys
// [1] -> Data
//
// Returns the empire keys on a given top list

int GameEngine::GetTopList (ScoringSystem ssListType, Variant*** pppvData, unsigned int* piNumEmpires) {

	int iErrCode;

	Assert (ssListType >= FIRST_SCORING_SYSTEM && ssListType < NUM_SCORING_SYSTEMS);

	const unsigned int iNumColumns = TOPLIST_NUM_COLUMNS [ssListType];
	unsigned int i, piColumn [TopList::MaxNumColumns];

	for (i = 0; i < iNumColumns; i ++) {
		piColumn[i] = TopList::EmpireKey + i;
	}

	const char* pszTableName = TOPLIST_TABLE_NAME [ssListType];

	iErrCode = m_pGameData->ReadColumns (
		pszTableName,
		iNumColumns,
		piColumn,
		pppvData,
		piNumEmpires
		);

	if (iErrCode == ERROR_DATA_NOT_FOUND) {
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

	return SendLongRunningQueryMessage (UpdateTopListOnIncreaseMsg, pQuery);
}


int GameEngine::UpdateTopListOnDecrease (ScoringSystem ssTopList, int iEmpireKey) {

	// Prepare the query
	TopListQuery* pQuery = new TopListQuery;
	if (pQuery == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	pQuery->TopList = ssTopList;
	pQuery->EmpireKey = iEmpireKey;

	return SendLongRunningQueryMessage (UpdateTopListOnDecreaseMsg, pQuery);
}

int GameEngine::UpdateTopListOnDeletion (ScoringSystem ssTopList, int iEmpireKey) {

	// Prepare the query
	TopListQuery* pQuery = new TopListQuery;
	if (pQuery == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	pQuery->TopList = ssTopList;
	pQuery->EmpireKey = iEmpireKey;

	return SendLongRunningQueryMessage (UpdateTopListOnDeletionMsg, pQuery);
}


int GameEngine::UpdateTopListOnIncreaseMsg (LongRunningQueryMessage* pMessage) {

	TopListQuery* pQuery = (TopListQuery*) pMessage->Arguments;

	int iErrCode = pMessage->GameEngine->UpdateTopListOnIncrease (pQuery);

	delete pQuery;
	return iErrCode;
}

int GameEngine::UpdateTopListOnDecreaseMsg (LongRunningQueryMessage* pMessage) {

	TopListQuery* pQuery = (TopListQuery*) pMessage->Arguments;

	int iErrCode = pMessage->GameEngine->UpdateTopListOnDecrease (pQuery);

	delete pQuery;
	return iErrCode;
}

int GameEngine::UpdateTopListOnDeletionMsg (LongRunningQueryMessage* pMessage) {

	TopListQuery* pQuery = (TopListQuery*) pMessage->Arguments;

	int iErrCode = pMessage->GameEngine->UpdateTopListOnDeletion (pQuery);

	delete pQuery;
	return iErrCode;
}


//
//
//

int GameEngine::UpdateTopListOnIncrease (TopListQuery* pQuery) {

	Variant pvData [MAX_SCORING_SYSTEM_COLUMNS], pvOurData [MAX_SCORING_SYSTEM_COLUMNS];

	int iEmpireKey = pQuery->EmpireKey, iErrCode;
	unsigned int iKey;

	ScoringSystem ssTopList = pQuery->TopList;

	IScoringSystem* pScoringSystem = GetScoringSystem (ssTopList);
	Assert (pScoringSystem != NULL);

	const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];

	// Read empire's current score
	iErrCode = pScoringSystem->GetEmpireScore (iEmpireKey, pvOurData);
	if (iErrCode != OK) {
		Assert (false);	// Spurious - race with deletion
		goto Cleanup;
	}

	// If empire is on the list, see if we need to shuffle him up
	iErrCode = m_pGameData->GetFirstKey (pszTableName, TopList::EmpireKey, iEmpireKey, false, &iKey);
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
			
			iErrCode = m_pGameData->GetNumRows (pszTableName, &iNumRows);
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
				
					iErrCode = m_pGameData->ReadData (pszTableName, iNumRows - 1, TopList::Data + i, pvData + i);
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

	pScoringSystem->Release();

	return iErrCode;
}


int GameEngine::UpdateTopListOnDecrease (TopListQuery* pQuery) {

	int iEmpireKey = pQuery->EmpireKey, iErrCode;
	ScoringSystem ssTopList = pQuery->TopList;

	IScoringSystem* pScoringSystem = GetScoringSystem (ssTopList);
	Assert (pScoringSystem != NULL);

	unsigned int iKey;
	const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];

	// If empire is on the list, see if we need to shuffle him down
	iErrCode = m_pGameData->GetFirstKey (pszTableName, TopList::EmpireKey, iEmpireKey, false, &iKey);
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

	pScoringSystem->Release();

	return iErrCode;
}

int GameEngine::UpdateTopListOnDeletion (TopListQuery* pQuery) {

	unsigned int iKey;

	int iEmpireKey = pQuery->EmpireKey, iErrCode;
	const char* pszTableName = TOPLIST_TABLE_NAME [pQuery->TopList];

	ScoringSystem ssTopList = pQuery->TopList;

	// If empire is on the list, we need to shuffle everyone else up
	iErrCode = m_pGameData->GetFirstKey (pszTableName, TopList::EmpireKey, iEmpireKey, false, &iKey);
	if (iErrCode == OK) {

		Variant** ppvData = NULL, ** ppvStackData;
		unsigned int iNumRows, i, iNumNewRows;
		
		iErrCode = GetTopList (ssTopList, &ppvData, &iNumRows);
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

Cleanup:

		if (ppvData != NULL) {
			m_pGameData->FreeData (ppvData);
		}
	}
	
	else if (iErrCode == ERROR_DATA_NOT_FOUND) {

		// If empire is not on the list, we're home free
		iErrCode = OK;
	}

	else Assert (false);

	return iErrCode;
}

int GameEngine::MoveEmpireUpInTopList (ScoringSystem ssTopList, int iEmpireKey, unsigned int iKey, 
									   const Variant* pvOurData) {

	int iErrCode;
	
	unsigned int iNumRows;
	bool bMoved;

	Variant** ppvData = NULL, ** ppvStackData;

	iErrCode = GetTopList (ssTopList, &ppvData, &iNumRows);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	ppvStackData = (Variant**) StackAlloc (max (iNumRows, TOPLIST_SIZE) * sizeof (Variant*));

	if (iNumRows > 0) {
		memcpy (ppvStackData, ppvData, iNumRows * sizeof (Variant*));
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

			iErrCode = m_pGameData->WriteData (pszTableName, iKey, TopList::Data + i, pvOurData[i]);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
	}

Cleanup:
		
	if (ppvData != NULL) {
		m_pGameData->FreeData (ppvData);
	}

	return iErrCode;
}

int GameEngine::PrivateMoveEmpireUpInTopList (ScoringSystem ssTopList, Variant** ppvData, 
											  unsigned int iNumRows, int iEmpireKey, unsigned int iKey, 
											  const Variant* pvOurData, bool* pbMoved) {

	int i, iErrCode = OK;

	unsigned int iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];

	Variant* pvTempData;

	IScoringSystem* pScoringSystem = GetScoringSystem (ssTopList);
	Assert (pScoringSystem != NULL);
	Assert (ppvData[iKey][TopList::EmpireKey].GetInteger() == iEmpireKey);

	*pbMoved = false;

	// Assign new data
	for (i = 0; i < (int) iNumColumns; i ++) {
		ppvData[iKey][TopList::Data + i] = pvOurData[i];
	}

	// Look for people to replace
	for (i = iKey - 1; i >= 0; i --) {

		if (pScoringSystem->CompareScores (pvOurData, ppvData[i] + TopList::Data) <= 0) {
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

	pScoringSystem->Release();

	return iErrCode;
}


int GameEngine::MoveEmpireDownInTopList (ScoringSystem ssTopList, int iEmpireKey, unsigned int iKey, 
										 const Variant* pvOurData) {

	int iErrCode;
	unsigned int iNumRows, iNumNewRows;
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
		&iNumNewRows,
		&bChanged
		);

	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	if (bChanged) {
		
		iErrCode = PrivateFlushTopListData (ssTopList, ppvStackData, iNumNewRows);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	
	} else {

		unsigned int i, iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];
		const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];

		for (i = 0; i < iNumColumns; i ++) {

			iErrCode = m_pGameData->WriteData (pszTableName, iKey, TopList::Data + i, pvOurData[i]);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
	}

Cleanup:
		
	if (ppvData != NULL) {
		m_pGameData->FreeData (ppvData);
	}

	return iErrCode;
}


int GameEngine::PrivateMoveEmpireDownInTopList (ScoringSystem ssTopList, Variant** ppvData, 
												unsigned int iNumRows, int iEmpireKey, unsigned int iKey, 
												const Variant* pvOurData, unsigned int* piNumNewRows,
												bool* pbChanged) {

	int iErrCode = OK;
	unsigned int i;

	Variant* pvTempData;

	IScoringSystem* pScoringSystem = GetScoringSystem (ssTopList);
	Assert (pScoringSystem != NULL);
	Assert (ppvData[iKey][TopList::EmpireKey].GetInteger() == iEmpireKey);

	// Maybe empire no longer belongs?
	bool bValid = pScoringSystem->IsValidScore (pvOurData);

	// Assign new data
	for (i = 0; i < TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList]; i ++) {
		ppvData[iKey][TopList::Data + i] = pvOurData[i];
	}

	// Look for people to replace
	for (i = iKey + 1; i < iNumRows; i ++) {

		if (bValid && pScoringSystem->CompareScores (pvOurData, ppvData[i] + TopList::Data) >= 0) {
			break;
		}

		Assert (ppvData[i - 1][TopList::EmpireKey].GetInteger() == iEmpireKey);

		// Swap rows
		pvTempData = ppvData[i];
		ppvData[i] = ppvData[i - 1];
		ppvData[i - 1] = pvTempData;
	}

	if (!bValid) {
		
		iErrCode = PrivateFindNewEmpireForTopList (ssTopList, ppvData, iNumRows, false, piNumNewRows);
		if (iErrCode != OK) {

			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				iErrCode = OK;
			}
			
			else Assert (false);
			goto Cleanup;
		}
		
	} else {

		if (i == iKey + 1) {

			*pbChanged = false;
			*piNumNewRows = iNumRows;
		
		} else {

			*pbChanged = true;

			if (i == TOPLIST_SIZE) {

				Assert (iNumRows == TOPLIST_SIZE);

				// Look for a replacement if we're at the end of the list
				iErrCode = PrivateFindNewEmpireForTopList (ssTopList, ppvData, iNumRows, true, piNumNewRows);
				if (iErrCode != OK) {

					if (iErrCode == ERROR_DATA_NOT_FOUND) {
						iErrCode = OK;
					}

					else Assert (false);
					goto Cleanup;
				}
			
			} else {

				*piNumNewRows = iNumRows;
			}
		}
	}

Cleanup:

	pScoringSystem->Release();

	return iErrCode;
}


int GameEngine::PrivateFindNewEmpireForTopList (ScoringSystem ssTopList, Variant** ppvData, 
												unsigned int iNumRows, bool bKeep, unsigned int* piNewNumRows) {

	int iErrCode;

	unsigned int* piKey = NULL, iNumEmpires, i, j, iListKey, iReplacementKey = NO_KEY, iLastRow = NO_KEY;

	const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];
	const unsigned int iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];
	
	Variant pvReplacementData [MAX_SCORING_SYSTEM_COLUMNS], pvData [MAX_SCORING_SYSTEM_COLUMNS];

	IScoringSystem* pScoringSystem = GetScoringSystem (ssTopList);
	Assert (pScoringSystem != NULL);

	IReadTable* pReadTable = NULL;

	iLastRow = iNumRows - 1;

	if (bKeep) {

		for (i = 0; i < iNumColumns; i ++) {
			pvReplacementData[i] = ppvData [iLastRow][TopList::Data + i];
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

	// This is fine because the only thread that can write to these tables is us
	iErrCode = m_pGameData->GetTableForReading (pszTableName, &pReadTable);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

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
		
		// See if they have a score...
		if (pScoringSystem->CompareScores (pvData, pvReplacementData) > 0) {
			
			// If they're not on the list already...
			iErrCode = pReadTable->GetFirstKey (TopList::EmpireKey, (int) piKey[i], &iListKey);
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
	pScoringSystem->Release();

	if (pReadTable != NULL) {
		pReadTable->Release();
	}

	if (piKey != NULL) {
		m_pGameData->FreeKeys (piKey);
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
			ppvData[iLastRow][TopList::EmpireKey] = iReplacementKey;

			//for (j = 0; j < iNumColumns; j ++) {
			//	pvReplacementData[j] = pvReplacementData[j];
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

			ppvData[iLastRow] = NULL;	// Not a leak
			return ERROR_DATA_NOT_FOUND;

		} else {

			bool bMoved;

			*piNewNumRows = iNumRows;

			ppvData[iLastRow][TopList::EmpireKey] = iReplacementKey;
			
			//for (j = 0; j < iNumColumns; j ++) {
			//	pvReplacementData[j] = pvReplacementData[j];
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

	const char* pszTableName = TOPLIST_TABLE_NAME [ssTopList];
	unsigned int iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];
	
	IScoringSystem* pScoringSystem = GetScoringSystem (ssTopList);
	Assert (pScoringSystem != NULL);

#ifdef _DEBUG

	iErrCode = m_pGameData->GetNumRows (pszTableName, &iNumRowsFull);
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
			ppvData[iNumRowsFull][TopList::EmpireKey] = piKey[i];

			for (j = 0; j < iNumColumns; j ++) {
				ppvData[iNumRowsFull][TopList::Data + j] = pvData[j];
			}

			iNumRowsFull ++;
		
		} else {
		
			if (pScoringSystem->CompareScores (pvData, ppvData[TOPLIST_SIZE - 1] + TopList::Data) > 0) {
				
				// Replace last entry
				ppvData[TOPLIST_SIZE - 1][TopList::EmpireKey] = piKey[i];
				
				for (j = 0; j < iNumColumns; j ++) {
					ppvData[TOPLIST_SIZE - 1][TopList::Data + j] = pvData[j];
				}
			}

		}
		
		// Move up as far as possible
		j = iNumRowsFull - 1;
		while (j > 0) {

			if (pScoringSystem->CompareScores (pvData, ppvData[j - 1] + TopList::Data) > 0) {
				
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
		m_pGameData->FreeKeys (piKey);
	}

	pScoringSystem->Release();

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
		ppvStackData[TOPLIST_SIZE - 1][TopList::EmpireKey] = iEmpireKey;
		iNumRealRows = TOPLIST_SIZE;

	} else {

		// Add extra padding
		for (i = iNumEmpires; i < TOPLIST_SIZE; i ++) {
			ppvStackData[i] = ppvStaticVariant + (i - iNumEmpires) * TopList::MaxNumColumns;
		}

		// Add to list
		ppvStackData[iNumEmpires][TopList::EmpireKey] = iEmpireKey;
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
		m_pGameData->FreeData (ppvData);
	}

	return iErrCode;
}


int GameEngine::PrivateFlushTopListData (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows) {

	int iErrCode;
	unsigned int i, iNumActualRows;

	const char* pszTable = TOPLIST_TABLE_NAME [ssTopList];

	IWriteTable* pWriteTable = NULL;
	IReadTable* pReadTable = NULL;

	iErrCode = m_pGameData->GetTableForWriting (pszTable, &pWriteTable);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pWriteTable->QueryInterface (IID_IReadTable, (void**) &pReadTable);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pReadTable->GetNumRows (&iNumActualRows);
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
			
			iErrCode = pWriteTable->InsertRow (ppvData[i]);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}

	} else {

		for (i = 0; i < iNumRows; i ++) {
			
			iErrCode = pWriteTable->WriteData (i, TopList::EmpireKey, ppvData[i][TopList::EmpireKey].GetInteger());
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = pWriteTable->WriteData (i, TopList::Data, ppvData[i][TopList::Data]);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
	}

Cleanup:

	if (pReadTable != NULL) {
		pReadTable->Release();
	}

	if (pWriteTable != NULL) {
		pWriteTable->Release();
	}

	return iErrCode;
}

int GameEngine::VerifyTopList (ScoringSystem ssTopList) {

	IReadTable* pReadTable = NULL;

	unsigned int i, j, iTestKey, iNumRows;
	int iErrCode, iEmpireKey;
	Variant vData, vLowerData, pvThisData [MAX_SCORING_SYSTEM_COLUMNS], pvNextData [MAX_SCORING_SYSTEM_COLUMNS];
	bool bExist;

	const char* pszTable = TOPLIST_TABLE_NAME [ssTopList];
	unsigned int iNumColumns = TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [ssTopList];

	IScoringSystem* pScoringSystem = GetScoringSystem (ssTopList);
	Assert (pScoringSystem != NULL);

	iErrCode = m_pGameData->GetTableForReading (pszTable, &pReadTable);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}

	iErrCode = pReadTable->GetNumRows (&iNumRows);
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
		iErrCode = pReadTable->DoesRowExist (i, &bExist);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (!bExist) {
			iErrCode = ERROR_TOPLIST_CORRUPT;
			goto Cleanup;
		}

		// If empire doesn't exist, the table is corrupt
		iErrCode = pReadTable->ReadData (i, TopList::EmpireKey, &iEmpireKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pGameData->DoesRowExist (SYSTEM_EMPIRE_DATA, iEmpireKey, &bExist);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (!bExist) {
			iErrCode = ERROR_TOPLIST_CORRUPT;
			goto Cleanup;
		}

		// If key is in table twice, the table is corrupt
		iErrCode = pReadTable->GetFirstKey (TopList::EmpireKey, iEmpireKey, &iTestKey);
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

			iErrCode = pReadTable->ReadData (i, TopList::Data + j, pvThisData + j);
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

			iErrCode = pReadTable->DoesRowExist (iNextRow, &bExist);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}

			if (!bExist) {
				iErrCode = ERROR_TOPLIST_CORRUPT;
				goto Cleanup;
			}
			
			for (j = 0; j < iNumColumns; j ++) {
				
				iErrCode = pReadTable->ReadData (iNextRow, TopList::Data + j, pvNextData + j);
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

	if (pReadTable != NULL) {
		pReadTable->Release();
	}

	pScoringSystem->Release();

	return iErrCode;
}