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

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppvEmpData -> Empire data
//
// *piNumShips -> Num ships
// *piBattleRank -> BR
// *piMilVal -> Mil
// *pfTechDev -> Tech development
// *pfMaintRatio -> Maintenance ratio
// *pfFuelRatio -> Fuel ratio
// *pfAgRatio -> Ag ratio
// *pfHypMaintRatio -> Next update's maintenance ratio
// *pfHypFuelRatio -> Next update's fuel ratio
// *pfHypAgRatio -> Next update's ag ratio
// *pfNextTechIncrease -> Next update's tech increase
// *piShipLimit -> Ship limit
//
// Returns "Info" data about an empire

// TODO: reconsider use of this function - call GetRatioInformation instead
int GameEngine::GetEmpireGameInfo(int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvEmpData,
                                  int* piNumShips, int* piBattleRank, int* piMilVal, float* pfTechDev, 
                                  float* pfMaintRatio, float* pfFuelRatio, float* pfAgRatio, 
                                  float* pfHypMaintRatio, float* pfHypFuelRatio, float* pfHypAgRatio,
                                  float* pfNextTechIncrease, int* piShipLimit)
{
    int iErrCode;

    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_SHIPS (strGameEmpireShips, iGameClass, iGameNumber, iEmpireKey);

    // Read the GameEmpireData
    Variant* pvEmpData = NULL;
    AutoFreeData free(pvEmpData);

    iErrCode = t_pCache->ReadRow (strEmpireData, NO_KEY, &pvEmpData);
    RETURN_ON_ERROR(iErrCode);

    // Get the extras
    RatioInformation ratInfo;
    iErrCode = GetRatioInformation (iGameClass, iGameNumber, iEmpireKey, &ratInfo);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->GetNumCachedRows(strGameEmpireShips, (unsigned int*) piNumShips);
    RETURN_ON_ERROR(iErrCode);

    *piBattleRank = GetBattleRank (ratInfo.fTechLevel);
    *piMilVal = GetMilitaryValue (pvEmpData[GameEmpireData::iMil].GetFloat());

    *pfTechDev = ratInfo.fTechDev;
    *pfMaintRatio = ratInfo.fMaintRatio;
    *pfFuelRatio = ratInfo.fFuelRatio;
    *pfAgRatio = ratInfo.fAgRatio;
    *pfHypMaintRatio = ratInfo.fNextMaintRatio;
    *pfHypFuelRatio = ratInfo.fNextFuelRatio;
    *pfHypAgRatio = ratInfo.fNextAgRatio;
    *pfNextTechIncrease = ratInfo.fNextTechDev;

    Variant vTemp;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxNumShips, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    *piShipLimit = vTemp.GetInteger();

    *ppvEmpData = pvEmpData;
    pvEmpData = NULL;

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Empire key
//
// Output:
// *pfAgRatio -> Empire's ag ratio in a particular game
//
// Return an empire's ag ratio in a particular game

int GameEngine::GetEmpireAgRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfAgRatio) {

    int iTotalAg, iBonusAg, iTotalPop, iErrCode;
    Variant vMaxAgRatio;

    ICachedTable* pGameEmpireData = NULL;
    AutoRelease<ICachedTable> rel(pGameEmpireData);
    
    GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->GetTable(pszEmpireData, &pGameEmpireData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::TotalAg, &iTotalAg);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::BonusAg, &iBonusAg);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pGameEmpireData->ReadData(GameEmpireData::TotalPop, &iTotalPop);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::MaxAgRatio, &vMaxAgRatio);
    RETURN_ON_ERROR(iErrCode);
    *pfAgRatio = GetAgRatio (iTotalAg + iBonusAg, iTotalPop, vMaxAgRatio.GetFloat());

    return iErrCode;
}