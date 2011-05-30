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

// Input:
// strTableName -> Name of table to be searched
// iNumCols -> Number of columns to be tested
// *pstrColName -> Array of names of columns to be tested
// *pvData -> Array of data to test with (min if numeric, data string if string)
// *pvData2 -> Another array of data (max if numeric, comparison type if string)
//
// Output:
// **ppiKey -> Keys of empires found
// *piNumHits -> Number of hits found
//
// Perform case insensitive searches on multiple categories of empire data

int GameEngine::PerformMultipleSearch (int iStartKey, int iSkipHits, int iMaxNumHits, int iNumCols, 
                                       const unsigned int* piColumn, const unsigned int* piFlags, 
                                       const Variant* pvData, const Variant* pvData2, 
                                       int** ppiKey, int* piNumHits, int* piStopKey) {

    int iErrCode = m_pGameData->GetSearchKeys (
        SYSTEM_EMPIRE_DATA, 
        iNumCols, 
        piColumn,
        piFlags,
        pvData,
        pvData2,
        iStartKey,
        iSkipHits, 
        iMaxNumHits, 
        (unsigned int**) ppiKey, 
        (unsigned int*) piNumHits, 
        (unsigned int*) piStopKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return OK;
    }

    return iErrCode;
}