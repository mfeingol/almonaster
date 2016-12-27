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
// *piNumAvailableTechs -> Number of available techs for development
//
// Return the number of available techs for development

int GameEngine::GetNumAvailableTechs (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumAvailableTechs) {

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    Variant vNumAvailableTechs;
    
    int iErrCode = t_pCache->ReadData(
        strGameEmpireData, 
        GameEmpireData::NumAvailableTechUndevs, 
        &vNumAvailableTechs
        );

    RETURN_ON_ERROR(iErrCode);

    Variant vTechUndevs;
    
    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::TechUndevs, &vTechUndevs);
    RETURN_ON_ERROR(iErrCode);

    *piNumAvailableTechs = min (vNumAvailableTechs.GetInteger(), GetNumTechs (vTechUndevs.GetInteger()));

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *piTechDevs -> Developed techs bitmap
// *piTechUndevs -> Undeveloped techs bitmap
//
// Returns the empire's developed and undeveloped techs

int GameEngine::GetDevelopedTechs (int iGameClass, int iGameNumber, int iEmpireKey, int* piTechDevs, 
                                   int* piTechUndevs) {

    GET_GAME_EMPIRE_DATA (strGameData, iGameClass, iGameNumber, iEmpireKey);

    Variant vTemp;
    int iErrCode = t_pCache->ReadData(strGameData, GameEmpireData::TechDevs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piTechDevs = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(strGameData, GameEmpireData::TechUndevs, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piTechUndevs = vTemp.GetInteger();

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iTechKey -> Integer key for new technology
//
// Register a new tech dev for an empire

int GameEngine::RegisterNewTechDevelopment (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey) {

    int iErrCode;

    // Make sure a tech dev is available to the empire
    Variant vTech;
    GET_GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::NumAvailableTechUndevs, &vTech);
    RETURN_ON_ERROR(iErrCode);
    if (vTech.GetInteger() == 0) {
        return ERROR_NO_TECHNOLOGY_AVAILABLE;
    }

    // Make sure the tech dev can be selected
    iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::TechUndevs, &vTech);
    RETURN_ON_ERROR(iErrCode);
    if (!(vTech.GetInteger() & TECH_BITS[iTechKey])) {
        return ERROR_WRONG_TECHNOLOGY;
    }

    // Add tech to developed field
    iErrCode = t_pCache->WriteOr(strEmpireData, GameEmpireData::TechDevs, TECH_BITS[iTechKey]);
    RETURN_ON_ERROR(iErrCode);

    // Delete tech from undeveloped field
    iErrCode = t_pCache->WriteAnd(strEmpireData, GameEmpireData::TechUndevs, ~TECH_BITS[iTechKey]);
    RETURN_ON_ERROR(iErrCode);

    // Subtract one from number of available techs to develop
    iErrCode = t_pCache->Increment(strEmpireData, GameEmpireData::NumAvailableTechUndevs, -1);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire key
// iTechKey -> Tech key
//
// Output:
// *pstrDefaultShipName -> Empire's default name for tech
//
// Return the empire's default name for the given tech

int GameEngine::GetDefaultEmpireShipName (int iEmpireKey, int iTechKey, Variant* pvDefaultShipName) {

    if (iTechKey < 0 || iTechKey >= NUM_SHIP_TYPES)
    {
        return ERROR_WRONG_TECHNOLOGY;
    }

    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);
    return t_pCache->ReadData(strEmpires, iEmpireKey, SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[iTechKey], pvDefaultShipName);
}


// Input:
// iEmpireKey -> Empire key
// iTechKey -> Tech key
// strDefaultShipName -> Empire's default name for tech
//
// Set the empire's default name for the given tech

int GameEngine::SetDefaultEmpireShipName (int iEmpireKey, int iTechKey, const char* pszDefaultShipName) {

    if (iTechKey < 0 || iTechKey >= NUM_SHIP_TYPES) {
        Assert(false);
        return ERROR_WRONG_TECHNOLOGY;
    }

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    return t_pCache->WriteData(strEmpire, iEmpireKey, SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN [iTechKey], pszDefaultShipName);
}


// Input:
// iTechBitmap -> A tech bitmap (developed or undeveloped)
//
// Return the number of techs in the given bitmap

int GameEngine::GetNumTechs (int iTechBitmap) {
    
    int i, iNumTechs = 0;

    ENUMERATE_TECHS (i) {
        if (iTechBitmap & TECH_BITS[i]) {
            iNumTechs ++;
        }
    }

    return iNumTechs;
}