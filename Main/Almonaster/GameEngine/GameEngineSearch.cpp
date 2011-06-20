//
// GameEngine.dll:  a component of Almonaster
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

// Input:
// sdSearch -> Search definition
//
// Output:
// **ppiKey -> Keys of empires found
// *piNumHits -> Number of hits found
// *piStopKey -> Key at which the search stopped
//
// Perform case insensitive searches on multiple categories of empire data

int GameEngine::PerformMultipleSearch (const SearchDefinition& sdSearch, unsigned int** ppiKey, 
                                       unsigned int* piNumHits, unsigned int* piStopKey) {

    int iErrCode = t_pConn->GetSearchKeys (
        SYSTEM_EMPIRE_DATA, 
        sdSearch,
        ppiKey, 
        piNumHits, 
        piStopKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}