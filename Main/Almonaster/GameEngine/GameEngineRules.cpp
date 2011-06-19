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

#include <math.h>

#include "GameEngine.h"

// Return the BR corresponding to a given tech development level
int GameEngine::GetBattleRank (float fTech) {

    if (fTech > (float) 0.0) {
        return (int) sqrt (fTech);
    }
    
    return 0;
}


// Return the miltary value of a given attack level
int GameEngine::GetMilitaryValue (float fAttack) {
    return (int) (fAttack / 50);
}


// Return an empire's maintenance ratio, given its minerals, maintenance and build
float GameEngine::GetMaintenanceRatio (int iMin, int iMaint, int iBuild) {

    Assert (iMin >= 0 && iMaint >= 0 && iBuild >= 0);

    if (iMin == 0) {
        return 0.0;
    }

    int iMinUse = iMaint + iBuild;
    if (iMinUse != 0) {
        return (float) iMin / iMinUse;
    }
    
    return MAX_RATIO;
}


// Return an empire's fuel ratio, given its fuel and fuel use
float GameEngine::GetFuelRatio (int iFuel, int iFuelUse) {

    Assert (iFuel >= 0 && iFuelUse >= 0);

    if (iFuel == 0) {
        return 0.0;
    }

    if (iFuelUse != 0) {
        return (float) iFuel / iFuelUse;
    }

    return MAX_RATIO;
}


// Return an empire's ag ratio, given its ag and pop
float GameEngine::GetAgRatio (int iAg, int iPop, float fMaxAgRatio) {

    Assert (iAg >= 0 && iPop >= 0);

    if (iAg == 0) {
        return 0.0;
    }

    if (iPop != 0) {

        float fTry = (float) iAg / iPop;
        return fMaxAgRatio > fTry ? fTry : fMaxAgRatio;
    }
    
    return fMaxAgRatio;
}

float GameEngine::GetShipNextBR (float fBR, float fMaint) {

    Assert (fBR > 0);
    return fBR * fMaint;
}

int GameEngine::GetNextPopulation (int iPop, float fAgRatio) {

    Assert (iPop >= 0 && fAgRatio >= (float) 0.0);

    return (int) (fAgRatio * (double) iPop) + 1;
}

// Return en empire's econ, given its fuel, min and ag
int GameEngine::GetEcon (int iFuel, int iMin, int iAg) {

    Assert (iFuel >= 0 && iMin >= 0 && iAg >= 0);

    int iResources = iFuel + iMin + iAg;
    if (iResources == 0) {
        return 0;
    }
    
    int iEcon = (int) floor ((double) iResources / 100);
    if (iEcon == 0) {
        return 1;
    }
    return iEcon;
}


// Return an empire's tech development, given its fuel, min, maintenance, fuel use and maxtechdev
float GameEngine::GetTechDevelopment (int iFuel, int iMin, int iMaint, int iBuild, int iFuelUse, 
                                      float fMaxTechDev) {

    Assert (iFuel >= 0 && iMin >= 0 && iMaint >= 0 && iBuild >= 0 && iFuelUse >= 0);

    int iResTotal = iFuel + iMin;

    if (iResTotal == 0) {
        iResTotal = 1;
    }

    Assert (iResTotal > 0);

    int iResUsed = iMaint + iBuild + iFuelUse;

    if (iResUsed == 0) {
        return fMaxTechDev;
    }
    
    return (float) (fMaxTechDev * (double) (iResTotal - iResUsed) / iResTotal);
}


// Return the build cost of a given ship
int GameEngine::GetBuildCost (int iType, float fBR) {

    Assert (iType >= FIRST_SHIP && iType <= LAST_SHIP && fBR > 0.0);

    int iFactor = (int) fBR + 4;
    int iCost = iFactor * iFactor;

    // Add special
    switch (iType) {

    case ATTACK:
        break;

    case SATELLITE:
        iCost -= 10;
        break;

    case MINEFIELD:
    case DOOMSDAY:
        iCost += 10;
        break;

    case MORPHER:
        iCost += 35;

    case BUILDER:
        iCost += 50;
        break;

    case CARRIER:
        iCost += 75;
        break;

    case STARGATE:
    case ENGINEER:
        iCost += 100;
        break;

    case JUMPGATE:
        iCost += 150;
        break;

    default:
        iCost += 25;
        break;
    }

    return iCost;
}


// Return the mantenance cost of a given ship
int GameEngine::GetMaintenanceCost (int iType, float fBR) {

    Assert (iType >= FIRST_SHIP && iType <= LAST_SHIP && fBR > 0.0);

    int iCost = (int) (fBR * 2);

    // Add special
    switch (iType) {

    case ATTACK:
        break;

    case SATELLITE:
        iCost -= 2;
        if (iCost <= 2) {
            iCost = 2; // BR1 sats default to 2
        }
        break;

    case MINEFIELD:
    case DOOMSDAY:
        iCost += 2;
        break;

    case MORPHER:
        iCost += 6;
        break;

    case BUILDER:
        iCost += 8;
        break;

    case CARRIER:
        iCost += 12;
        break;

    case STARGATE:
    case ENGINEER:
        iCost += 16;
        break;

    case JUMPGATE:
        iCost += 24;
        break;

    default:
        iCost += 4;
        break;
    }

    return iCost;
}


// Return the fuel use cost of a given ship
int GameEngine::GetFuelCost (int iType, float fBR) {

    Assert (iType >= FIRST_SHIP && iType <= LAST_SHIP && fBR > 0.0);

    int iCost = (int) (fBR * 4);

    // Add special
    switch (iType){
    
    case ATTACK:
        break;

    case SATELLITE:
    case MINEFIELD:
        iCost = 0;
        break;

    case DOOMSDAY:
        iCost += 4;
        break;

    case MORPHER:
        iCost += 12;
        break;

    case BUILDER:
        iCost += 16;
        break;

    case CARRIER:
        iCost += 24;
        break;

    case STARGATE:
    case ENGINEER:
        iCost += 32;
        break;

    case JUMPGATE:
        iCost += 48;
        break;

    default:
        iCost += 8;
        break;
    }

    return iCost;
}

// Return the initial pop of a planet, given the colony ship's BR
int GameEngine::GetColonizePopulation (int iShipBehavior, float fColonyMultipliedDepositFactor, 
                                       float fColonyExponentialDepositFactor, float fBR) {

    Assert (fBR > 0.0);

    int iPop;

    if (iShipBehavior & COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT) {
        iPop = (int) (fColonyMultipliedDepositFactor * fBR);
    } else {
        iPop = (int) (pow (fBR, fColonyExponentialDepositFactor));
    }

    if (iPop == 0) {
        return 1;
    }

    return iPop;
}


// Return the population cost of a colony, given the colony ship's BR
unsigned int GameEngine::GetColonyPopulationBuildCost(int iShipBehavior, float fColonyMultipliedBuildFactor, 
                                                      int iColonySimpleBuildFactor, float fBR) {

    Assert (fBR > 0.0);
    Assert (fColonyMultipliedBuildFactor >= 0.0);
    Assert (iColonySimpleBuildFactor >= 0);

    if (iShipBehavior & COLONY_USE_MULTIPLIED_BUILD_COST) {
        return (int) (fColonyMultipliedBuildFactor * fBR);
    }
    
    return iColonySimpleBuildFactor;
}


// Return the ag increase produced by a terraformer of given BR
int GameEngine::GetTerraformerAg (float fTerraformerPlowFactor, float fBR) {

    Assert (fBR > 0.0);

    return (int) (fTerraformerPlowFactor * fBR);
}


// Return the pop that a troopship of given BR can handle
int GameEngine::GetTroopshipPop (float fTroopshipInvasionFactor, float fBR) {

    Assert (fBR > 0.0);

    return (int) (fTroopshipInvasionFactor * fBR);
}

int GameEngine::GetTroopshipFailurePopDecrement (float fTroopshipFailureFactor, float fBR) {

    Assert (fBR > 0.0);

    return (int) (- fTroopshipFailureFactor * fBR);
}

int GameEngine::GetTroopshipSuccessPopDecrement (float fTroopshipSuccessFactor, int iPop) {

    Assert (iPop >= 0);

    return (int) ceil ((double) iPop * (- fTroopshipSuccessFactor));
}

// Return the number of updates that a planet remains uninhabitable after
// a doomsday of given BR annihilates it
int GameEngine::GetDoomsdayUpdates (float fDoomsdayAnnihilationFactor, float fBR) {

    Assert (fBR > 0.0);
    return (int) (fDoomsdayAnnihilationFactor * fBR);
}

void GameEngine::GetBuilderNewPlanetResources (float fBR, float fBRDampening, float fMultiplier,
                                               int iAvgAg, int iAvgMin, int iAvgFuel,
                                               int* piNewAvgAg, int* piNewAvgMin, int* piNewAvgFuel) {

    Assert (fBRDampening <= fBR);

    double fMultiple = (fBR - fBRDampening + (double) 1.0) / fBR;
    double fBase = (double) fMultiplier * fMultiple * fMultiple;

    *piNewAvgAg = (int) ((double) iAvgAg * fBase);
    *piNewAvgMin = (int) ((double) iAvgMin * fBase);
    *piNewAvgFuel = (int) ((double) iAvgFuel * fBase);
}

float GameEngine::GetGateBRForRange (float fRangeFactor, int iSrcX, int iSrcY, int iDestX, int iDestY) {

    int iDistX = abs (iDestX - iSrcX);
    int iDistY = abs (iDestY - iSrcY);

    int iDist = max (iDistX, iDistY);

    return fRangeFactor * iDist;
}

float GameEngine::GetCarrierDESTAbsorption (float fBR) {

    return fBR * fBR;
}

bool GameEngine::IsMobileShip (int iShipType) {

    return iShipType != SATELLITE &&
           iShipType != STARGATE &&
           iShipType != MINEFIELD &&
           iShipType != JUMPGATE;
}

float GameEngine::GetLateComerTechIncrease (int iPercentTechIncreaseForLatecomers, int iNumUpdates, 
                                            float fMaxTechDev) {

    Assert (fMaxTechDev >= 0.0);

    return (float) ((double) iNumUpdates * fMaxTechDev * ((double) iPercentTechIncreaseForLatecomers / 100));
}

int GameEngine::GetMaxPop (int iMin, int iFuel) {

    int iMaxPop = max (iMin, iFuel);

    if (iMaxPop == 0) {
        return 1;
    }

    return iMaxPop;
}

int GameEngine::GetMaxNumDiplomacyPartners (int iGameClass, int iGameNumber, int iDiplomacyLevel,
                                            int* piMaxNumPartners) {

    int iErrCode = OK;
    Variant vMax;

    Assert (iDiplomacyLevel != WAR);
    
    switch (iDiplomacyLevel) {

    case TRUCE:
        
        iErrCode = m_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxNumTruces, 
            &vMax
            );
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        break;

    case TRADE:
        
        iErrCode = m_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxNumTrades, 
            &vMax
            );
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        break;

    case ALLIANCE:
        
        iErrCode = m_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::MaxNumAlliances, 
            &vMax
            );
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        break;

    default:

        Assert (false);
        return ERROR_INVALID_ARGUMENT;
    }   

    if (vMax.GetInteger() == FAIR_DIPLOMACY) {

        GAME_DATA (pszGameData, iGameClass, iGameNumber);

        iErrCode = m_pConn->ReadData (
            pszGameData, 
            GameData::MaxNumEmpires, 
            &vMax
            );
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        *piMaxNumPartners = (int) ((double) (vMax.GetInteger() - 2) / 2);
        return OK;
    }

    *piMaxNumPartners = vMax.GetInteger();  // Number or UNRESTRICTED_DIPLOMACY

    return iErrCode;
}

void GameEngine::CalculateTradeBonuses (int iNumTrades, int iTotalAg, int iTotalMin, int iTotalFuel,
                                        int iPercentFirstTradeIncrease, int iPercentNextTradeIncrease,
                                        int* piBonusAg, int* piBonusMin, int* piBonusFuel) {

    int iBonusAg = 0, iBonusMin = 0, iBonusFuel = 0;

    double fIncrease = (double) iPercentFirstTradeIncrease / 100;
    double fNext = (double) iPercentNextTradeIncrease / 100;

    for (int i = 0; i < iNumTrades; i ++) {

        iBonusAg += (int) ((double) iTotalAg * fIncrease);
        iBonusMin += (int) ((double) iTotalMin * fIncrease);
        iBonusFuel += (int) ((double) iTotalFuel * fIncrease);

        fIncrease *= fNext;
    }

    *piBonusAg = iBonusAg;
    *piBonusMin = iBonusMin;
    *piBonusFuel = iBonusFuel;
}

unsigned int GameEngine::GetNumFairDiplomaticPartners (unsigned int iMaxNumEmpires) {

    return (int) ((double) (iMaxNumEmpires - 2) / 2);
}

bool GameEngine::GameAllowsDiplomacy (int iDiplomacyLevel, int iDip) {

    return (iDiplomacyLevel & iDip) != 0;
}

bool GameEngine::IsLegalDiplomacyLevel (int iDiplomacyLevel) {

    return (iDiplomacyLevel & ~(WAR | TRUCE | TRADE | ALLIANCE | SURRENDER)) == 0;
}

bool GameEngine::IsLegalPrivilege (int iPrivilege) {

    return (iPrivilege >= NOVICE && iPrivilege <= ADMINISTRATOR) || iPrivilege == GUEST;
}

int GameEngine::GetNextDiplomaticStatus (int iOffer1, int iOffer2, int iCurrentStatus) {

    if (iOffer1 > ALLIANCE) {
        iOffer1 = iCurrentStatus;
    }

    if (iOffer2 > ALLIANCE) {
        iOffer2 = iCurrentStatus;
    }

    int iStatus = min (iOffer1, iOffer2);
    Assert (iStatus <= ALLIANCE);
    return iStatus;
}