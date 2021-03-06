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
// iGameNumber - > Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiEmpireKey -> Integer keys of empires
// *piNumEmpires -> Number of empires
//
// Return the keys of the empires in the given empire's diplomacy list

int GameEngine::GetKnownEmpireKeys(int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvEmpireKey, int* piNumEmpires)
{
    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->ReadColumn(
        strGameEmpireDiplomacy,
        GameEmpireDiplomacy::ReferenceEmpireKey,
        NULL,
        ppvEmpireKey, 
        (unsigned int*)piNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of caller empire
// iFoeKey -> Integer key of other empire
//
// Output:
// *piWeOffer -> What we are offering
// *piTheyOffer -> What we're receiving
// *piCurrent -> What we're at now
//
// Return diplomatic status between two empires

int GameEngine::GetVisibleDiplomaticStatus(int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, int* piWeOffer, int* piTheyOffer, int* piCurrent, bool* pbMet) {

    int iErrCode = OK;

    ICachedTable* pTable = NULL;
    AutoRelease<ICachedTable> release(pTable);

    if (piWeOffer)
    {
        *piWeOffer = WAR;
    }

    if (piTheyOffer)
    {
        *piTheyOffer = WAR;
    }

    if (piCurrent)
    {
        *piCurrent = WAR;
    }

    if (pbMet)
    {
        *pbMet = true;
    }

    if (pbMet || piWeOffer || piCurrent)
    {
        GET_GAME_EMPIRE_DIPLOMACY (strEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);
        iErrCode = t_pCache->GetTable(strEmpireDiplomacy, &pTable);
        RETURN_ON_ERROR(iErrCode);

        unsigned int iKey;
        iErrCode = pTable->GetFirstKey(GameEmpireDiplomacy::ReferenceEmpireKey, iFoeKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            if (pbMet)
            {
                *pbMet = false;
            }
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);

        if (piWeOffer)
        {
            iErrCode = pTable->ReadData(iKey, GameEmpireDiplomacy::DipOffer, piWeOffer);
            RETURN_ON_ERROR(iErrCode);
        }

        if (piCurrent)
        {
            iErrCode = pTable->ReadData(iKey, GameEmpireDiplomacy::CurrentStatus, piCurrent);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    // They offer
    if (piTheyOffer)
    {
        Variant vOptions;
        iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vOptions);
        RETURN_ON_ERROR(iErrCode);

        GET_GAME_EMPIRE_DIPLOMACY (strEmpireDiplomacy, iGameClass, iGameNumber, iFoeKey);
        iErrCode = t_pCache->GetTable(strEmpireDiplomacy, &pTable);
        RETURN_ON_ERROR(iErrCode);
        
        unsigned int iKey;
        iErrCode = pTable->GetFirstKey(GameEmpireDiplomacy::ReferenceEmpireKey, iEmpireKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            Assert(pbMet == NULL && piWeOffer == NULL && piCurrent == NULL);
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);

        if (!(vOptions.GetInteger() & VISIBLE_DIPLOMACY))
        {
            iErrCode = pTable->ReadData(iKey, GameEmpireDiplomacy::DipOfferLastUpdate, piTheyOffer);   
            RETURN_ON_ERROR(iErrCode);
        }
        else
        {
            iErrCode = pTable->ReadData(iKey, GameEmpireDiplomacy::DipOffer, piTheyOffer);    
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
// iFoeKey -> Integer key of other empire
//
// Output:
// **ppiDipOption -> Text of diplomacy options
// piDipOptKey -> Integer keys of diplomacy options
// *piSelected -> Key of selected option
// *piNumOptions -> Number of options
//
// Returns the diplomatic options of an empire with regards to another

int GameEngine::GetDiplomaticOptions(int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
                                     int piDipOptKey[], int* piSelected, int* piNumOptions) {

    int iErrCode = OK;

    *piNumOptions = 0;

    GET_GAME_EMPIRE_DIPLOMACY (strDip, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    // Read selected option
    unsigned int iKey;
    Variant vWeOffer, vStatus, vLevel, vPrevDipKey, vOptions;

    iErrCode = t_pCache->GetFirstKey(strDip, GameEmpireDiplomacy::ReferenceEmpireKey, iFoeKey, &iKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strDip, iKey, GameEmpireDiplomacy::DipOffer, &vWeOffer);
    RETURN_ON_ERROR(iErrCode);

    *piSelected = vWeOffer.GetInteger();

    // Read current status
    iErrCode = t_pCache->GetFirstKey(strDip, GameEmpireDiplomacy::ReferenceEmpireKey, iFoeKey, &iKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vStatus);
    RETURN_ON_ERROR(iErrCode);

    // Read gameclass conditions
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vLevel
        );

    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = t_pCache->ReadData(
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );

    RETURN_ON_ERROR(iErrCode);

    bool bUnbreakableAlliances = (vOptions.GetInteger() & UNBREAKABLE_ALLIANCES) != 0;

    // Get game state
    int iGameState;
    iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
    RETURN_ON_ERROR(iErrCode);

    // Switch on current level
    int iLevel = vLevel.GetInteger();
    switch (vStatus.GetInteger()) {

    case WAR:

        // Add War
        piDipOptKey[(*piNumOptions) ++] = WAR;
        
        // Add truce?
        if (GameAllowsDiplomacy (iLevel, TRUCE))
        {
            iErrCode = AddDiplomaticOption(iGameClass, iGameNumber, iEmpireKey, iKey, TRUCE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
            RETURN_ON_ERROR(iErrCode);

        }
        else
        {
            // Add trade?
            if (GameAllowsDiplomacy (iLevel, TRADE))
            {
                iErrCode = AddDiplomaticOption(iGameClass, iGameNumber, iEmpireKey, iKey, TRADE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
                RETURN_ON_ERROR(iErrCode);
            }
            else
            {
                // Alliance
                if (GameAllowsDiplomacy (iLevel, ALLIANCE))
                {
                    iErrCode = AddDiplomaticOption(iGameClass, iGameNumber, iEmpireKey, iKey, ALLIANCE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        }
        
        break;

    case TRUCE:

        // Add war and truce
        piDipOptKey[(*piNumOptions) ++] = WAR;
        piDipOptKey[(*piNumOptions) ++] = TRUCE;

        // Trade
        if (GameAllowsDiplomacy (iLevel, TRADE))
        {
            iErrCode = AddDiplomaticOption(iGameClass, iGameNumber, iEmpireKey, iKey, TRADE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
            RETURN_ON_ERROR(iErrCode);

        } else {
            
            // Alliance         
            if (GameAllowsDiplomacy (iLevel, ALLIANCE))
            {
                iErrCode = AddDiplomaticOption(iGameClass, iGameNumber, iEmpireKey, iKey, ALLIANCE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
                RETURN_ON_ERROR(iErrCode);
            }
        }

        break;

    case TRADE:

        // Note:  no need to check num truces because if you can offer someone trade, 
        // you can also offer him truce.  Same comment applies to alliances below

        // Add previous options
        if (!GameAllowsDiplomacy (iLevel, TRUCE)) {
            piDipOptKey[(*piNumOptions) ++] = WAR;
        } else {
            piDipOptKey[(*piNumOptions) ++] = TRUCE;
        }
        
        // Add trade
        piDipOptKey[(*piNumOptions) ++] = TRADE;
        
        // Alliance?
        if (GameAllowsDiplomacy (iLevel, ALLIANCE))
        {
            iErrCode = AddDiplomaticOption(iGameClass, iGameNumber, iEmpireKey, iKey, ALLIANCE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
            RETURN_ON_ERROR(iErrCode);
        }

        break;

    case ALLIANCE:

        if (!bUnbreakableAlliances) {

            bool bGameAllowsTrade = GameAllowsDiplomacy (iLevel, TRADE);

            // War?
            if (!GameAllowsDiplomacy (iLevel, TRUCE) && !bGameAllowsTrade) {
                piDipOptKey[(*piNumOptions) ++] = WAR;
            } else {
                // Truce?
                if (!bGameAllowsTrade) {
                    piDipOptKey[(*piNumOptions) ++] = TRUCE;
                } else {
                    piDipOptKey[(*piNumOptions) ++] = TRADE;
                }
            }
        }

        // Add alliance
        piDipOptKey[(*piNumOptions) ++] = ALLIANCE;
        
        break;

    default:
        
        Assert(false);
        break;
    }

    // Add surrender options
    if (vStatus.GetInteger() == WAR && 
        GameAllowsDiplomacy (iLevel, SURRENDER) && 
        !(vOptions.GetInteger() & ONLY_SURRENDER_WITH_TWO_EMPIRES) &&
        !(iGameState & STILL_OPEN)
        ) {

        unsigned int iNumEmps;
        GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
        
        iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumEmps);
        RETURN_ON_ERROR(iErrCode);
        
        if (iNumEmps > 2) {

            // Make sure not allied before
            Variant vState;

            iErrCode = t_pCache->ReadData(strDip, iKey, GameEmpireDiplomacy::State, &vState);
            RETURN_ON_ERROR(iErrCode);

            if (!(vState.GetInteger() & ONCE_ALLIED_WITH)) {

                // Add surrender option if offered to this person or not being offered to anyone else
                if (vWeOffer.GetInteger() == SURRENDER) {
                    piDipOptKey[(*piNumOptions) ++] = SURRENDER;
                } else {
                    
                    unsigned int iNumSurrendering;
                    iErrCode = t_pCache->GetEqualKeys(
                        strDip,
                        GameEmpireDiplomacy::DipOffer,
                        SURRENDER,
                        NULL,
                        &iNumSurrendering
                        );
                    
                    if (iErrCode == ERROR_DATA_NOT_FOUND)
                    {
                        piDipOptKey[(*piNumOptions) ++] = SURRENDER;
                        iErrCode = OK;
                    }
                    RETURN_ON_ERROR(iErrCode);
                }
                
                piDipOptKey[(*piNumOptions) ++] = ACCEPT_SURRENDER;
            }
        }
    }

    return iErrCode;
}

int GameEngine::AddDiplomaticOption(int iGameClass, int iGameNumber, int iEmpireKey, int iDiplomacyProxyKey,
                                    int iDiplomacyLevel, int iCurrentDiplomacyLevel, int piDipOptKey[], int* piNumOptions)
{
    int iErrCode = OK;
    bool bAdd = true;

    if (iDiplomacyLevel != iCurrentDiplomacyLevel)
    {
        // Either upgrading or downgrading diplomacy levels...
        unsigned int iMaxCount;
        iErrCode = GetMaxNumDiplomacyPartners(iGameClass, iGameNumber, iDiplomacyLevel, &iMaxCount);
        RETURN_ON_ERROR(iErrCode);
        
        if (iMaxCount != UNRESTRICTED_DIPLOMACY)
        {
            // Diplomacy is restricted...
            unsigned int iCount;
            iErrCode = GetCumulativeDiplomacyCountForLimits(iGameClass, iGameNumber, iEmpireKey, iDiplomacyLevel, &iCount);
            RETURN_ON_ERROR(iErrCode);

            Assert(iCount >= 0 && iCount <= iMaxCount);
            if (iCount == iMaxCount)
            {
                bAdd = false;

                // However, if permanent alliances are set and we have the once allied bit set, it's okay
                if (iDiplomacyLevel == ALLIANCE)
                {                    
                    Variant vValue;
                    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vValue);
                    RETURN_ON_ERROR(iErrCode);
                    
                    if (vValue.GetInteger() & PERMANENT_ALLIANCES)
                    {
                        GET_GAME_EMPIRE_DIPLOMACY(strDiplomacy, iGameClass, iGameNumber, iEmpireKey);
                        iErrCode = t_pCache->ReadData(strDiplomacy, iDiplomacyProxyKey, GameEmpireDiplomacy::State, &vValue);
                        RETURN_ON_ERROR(iErrCode);
                        
                        if (vValue.GetInteger() & ONCE_ALLIED_WITH)
                        {
                            bAdd = true;
                        }
                    }
                }
            }
        }
    }

    if (bAdd)
    {
        piDipOptKey[*piNumOptions] = iDiplomacyLevel;
        (*piNumOptions) ++;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of source empire
// iFoeKey -> Integer key of target empire
// iDipOffer -> Key of dip offered
//
// Updates the diplomatic status of one player with respect to another

int GameEngine::UpdateDiplomaticOffer(int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, int iDipOffer)
{
    int iErrCode;
    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);
    
    // Make sure empires are still in game and have met
    unsigned int iKey;
    iErrCode = t_pCache->GetFirstKey(strGameEmpireDiplomacy, GameEmpireDiplomacy::ReferenceEmpireKey, iFoeKey, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Get current status and current offering
    Variant vStatus;
    iErrCode = t_pCache->ReadData(strGameEmpireDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vStatus);
    RETURN_ON_ERROR(iErrCode);

    Variant vOffering;
    iErrCode = t_pCache->ReadData(strGameEmpireDiplomacy, iKey, GameEmpireDiplomacy::DipOffer, &vOffering);
    RETURN_ON_ERROR(iErrCode);

    // If current offer is new offer, just say yes
    if (vOffering.GetInteger() == iDipOffer)
    {
        return OK;
    }

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    //////////////////
    // Verify offer //
    //////////////////

    Variant vDipLevel;
    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::DiplomacyLevel, &vDipLevel);
    RETURN_ON_ERROR(iErrCode);

    int iOptions;
    iErrCode = GetGameClassOptions(iGameClass, &iOptions);
    RETURN_ON_ERROR(iErrCode);

    // Get game state
    int iGameState;
    iErrCode = GetGameState(iGameClass, iGameNumber, &iGameState);
    RETURN_ON_ERROR(iErrCode);

    // If alliances are unbreakable, don't allow any changes
    if ((iOptions & UNBREAKABLE_ALLIANCES) && vStatus.GetInteger() == ALLIANCE && iDipOffer != ALLIANCE)
    {
        return ERROR_UNBREAKABLE_ALLIANCE;
    }

    bool bUpdate = false;
    int iDipLevel = vDipLevel.GetInteger();

    if (iDipOffer == WAR) {
        
        switch (vStatus.GetInteger()) {
            
        case WAR:
            bUpdate = true;
            break;
            
        case TRUCE:
            bUpdate = GameAllowsDiplomacy (iDipLevel, TRUCE);
            break;
            
        case TRADE:
            bUpdate = GameAllowsDiplomacy (iDipLevel, TRADE) && !GameAllowsDiplomacy (iDipLevel, TRUCE);
            break;
            
        case ALLIANCE:
            bUpdate = GameAllowsDiplomacy (iDipLevel, ALLIANCE) && 
                      !GameAllowsDiplomacy (iDipLevel, TRADE) && 
                      !GameAllowsDiplomacy (iDipLevel, TRUCE);
            break;
        }

    }
    else if (iDipOffer == SURRENDER || iDipOffer == ACCEPT_SURRENDER)
    {
        if (GameAllowsDiplomacy (iDipLevel, SURRENDER) && vStatus.GetInteger() == WAR &&
            !(iOptions & ONLY_SURRENDER_WITH_TWO_EMPIRES) && !(iGameState & STILL_OPEN))
        {
            // Allow surrenders?
            unsigned int iNumEmps;
            GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
            
            iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumEmps);
            RETURN_ON_ERROR(iErrCode);
            
            if (iNumEmps > 2)
            {
                // Make sure not allied before
                Variant vState;
                iErrCode = t_pCache->ReadData(strGameEmpireDiplomacy, iKey, GameEmpireDiplomacy::State, &vState);
                RETURN_ON_ERROR(iErrCode);
                
                if (!(vState.GetInteger() & ONCE_ALLIED_WITH))
                {
                    if (iDipOffer == SURRENDER)
                    {
                        unsigned int iNumSurrendering;
                        iErrCode = t_pCache->GetEqualKeys(
                            strGameEmpireDiplomacy,
                            GameEmpireDiplomacy::DipOffer,
                            SURRENDER,
                            NULL,
                            &iNumSurrendering
                            );

                        if (iErrCode == ERROR_DATA_NOT_FOUND)
                        {
                            bUpdate = true;
                            iErrCode = OK;
                        }
                        RETURN_ON_ERROR(iErrCode);
                        
                    } else {
                        
                        Assert(iDipOffer == ACCEPT_SURRENDER);
                        bUpdate = true;
                    }
                }
            }
        }
    }
    else
    {
        bUpdate = true;

        // Make sure that we are under the limit if we're trying to upgrade
        if (iDipOffer > vStatus.GetInteger())
        {
            unsigned int iMax;
            iErrCode = GetMaxNumDiplomacyPartners(iGameClass, iGameNumber, iDipOffer, &iMax);
            RETURN_ON_ERROR(iErrCode);

            if (iMax != UNRESTRICTED_DIPLOMACY)
            {
                unsigned int iCount;
                iErrCode = GetCumulativeDiplomacyCountForLimits(iGameClass, iGameNumber, iEmpireKey, iDipOffer, &iCount);
                RETURN_ON_ERROR(iErrCode);
                Assert(iCount <= iMax);

                if (iCount == iMax)
                {
                    bUpdate = false;
                    
                     // However, if permanent alliances are set and we have the once allied bit set, it's okay
                    if (iDipOffer == ALLIANCE)
                    {
                        if (iOptions & PERMANENT_ALLIANCES)
                        {
                            Variant vState;
                            iErrCode = t_pCache->ReadData(strGameEmpireDiplomacy, iKey, GameEmpireDiplomacy::State, &vState);
                            RETURN_ON_ERROR(iErrCode);
                        
                            if (vState.GetInteger() & ONCE_ALLIED_WITH)
                            {
                                bUpdate = true;
                            }
                        }
                    }
                }
            }
        }

        if (bUpdate)
        {
            // Validate with gameclass diplomacy type
            switch (iDipOffer) {
                
            case TRUCE:
                
                Assert(GameAllowsDiplomacy(iDipLevel, TRUCE));

                switch (vStatus.GetInteger()) {
                    
                case WAR:
                    bUpdate = GameAllowsDiplomacy(iDipLevel, TRUCE);
                    break;
                    
                case TRUCE:
                    bUpdate = true;
                    break;
                    
                case TRADE:
                    bUpdate = GameAllowsDiplomacy(iDipLevel, TRADE);
                    break;
                    
                case ALLIANCE:
                    bUpdate = GameAllowsDiplomacy(iDipLevel, ALLIANCE) && !GameAllowsDiplomacy (iDipLevel, TRADE);
                    break;
                }
                break;
                
            case TRADE:

                Assert(GameAllowsDiplomacy (iDipLevel, TRADE));
                
                switch (vStatus.GetInteger()) {
                    
                case WAR:
                    bUpdate  = !GameAllowsDiplomacy (iDipLevel, TRUCE);
                    break;
                    
                case TRUCE:
                    bUpdate = GameAllowsDiplomacy (iDipLevel, TRUCE);
                    break;
                    
                case TRADE:
                    bUpdate = true;
                    break;
                    
                case ALLIANCE:
                    bUpdate = GameAllowsDiplomacy (iDipLevel, ALLIANCE);
                    break;
                }
                break;
                    
            case ALLIANCE:
                    
                switch (vStatus.GetInteger()) {
                    
                case WAR:
                    bUpdate = GameAllowsDiplomacy (iDipLevel, ALLIANCE) && 
                              !GameAllowsDiplomacy (iDipLevel, TRADE) && 
                              !GameAllowsDiplomacy (iDipLevel, TRUCE);
                    break;
                    
                case TRUCE:
                    bUpdate = GameAllowsDiplomacy (iDipLevel, TRUCE) && 
                              !GameAllowsDiplomacy (iDipLevel, TRADE);
                    break;
                    
                case TRADE:
                    bUpdate = GameAllowsDiplomacy (iDipLevel, TRADE);
                    break;
                    
                case ALLIANCE:
                    bUpdate = true;
                    break;
                }

                break;
                
            default:
                Assert(false);
                break;
            }
        }
    }

    // After all this, update the setting
    if (bUpdate)
    {
        // Write the new offer
        iErrCode = t_pCache->WriteData(strGameEmpireDiplomacy, iKey, GameEmpireDiplomacy::DipOffer, iDipOffer);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// **ppiDuplicateKeys -> Flattened array of duplicate keys
// **ppiNumDuplicatesInList -> Number of keys in each set of duplicates
// *piNumDuplicates -> Number of duplicates lists found
//
// Searches for empires in a game with the same data column

int GameEngine::SearchForDuplicates (int iGameClass, int iGameNumber, const char* pszSystemEmpireDataColumn,
                                     const char* pszGameEmpireDataColumn, int** ppiDuplicateKeys, 
                                     unsigned int** ppiNumDuplicatesInList, unsigned int* piNumDuplicates)
{
    *ppiDuplicateKeys = NULL;
    *ppiNumDuplicatesInList = NULL;
    *piNumDuplicates = 0;

    unsigned int i, j, iNumEmpires, iIndex = 0;
    Variant* pvEmpireKey = NULL, * pvData = NULL, * pvGameData = NULL;
    bool* pbDup, bListStarted;

    AutoFreeData free_pvEmpireKey(pvEmpireKey);
    Algorithm::AutoDelete<Variant> del_pvData(pvData, true);

    GET_GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    // Get empires
    int iErrCode = t_pCache->ReadColumn(strGameEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    if (iNumEmpires == 0)
        return OK;

    iErrCode = CacheEmpires((const Variant*)pvEmpireKey, iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    // Allocate memory
    if (pszGameEmpireDataColumn != NULL)
    {
        pvData = new Variant[iNumEmpires * 2];
        Assert(pvData);
        pvGameData = pvData + iNumEmpires;
        
        for (i = 0; i < iNumEmpires; i ++)
        {
            GET_GAME_EMPIRE_DATA(strThisGameEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
            iErrCode = t_pCache->ReadData(strThisGameEmpireData, pszGameEmpireDataColumn, pvGameData + i);
            RETURN_ON_ERROR(iErrCode);
        }
    }
    else
    {
        pvData = new Variant[iNumEmpires];
        Assert(pvData);
    }

    pbDup = (bool*)StackAlloc(iNumEmpires * sizeof(bool));

    // Get all empire values
    for (i = 0; i < iNumEmpires; i ++)
    {
        pbDup[i] = false;

        GET_SYSTEM_EMPIRE_DATA(strThisEmpire, pvEmpireKey[i].GetInteger());
        iErrCode = t_pCache->ReadData(strThisEmpire, pvEmpireKey[i].GetInteger(), pszSystemEmpireDataColumn, pvData + i);
        RETURN_ON_ERROR(iErrCode);
    }

    for (i = 0; i < iNumEmpires; i ++)
    {
        if (pbDup[i])
        {
            continue;
        }

        bListStarted = false;

        for (j = i + 1; j < iNumEmpires; j ++)
        {
            if (!pbDup[j] && 
                
                (
                 pvData[i] == pvData[j] || 
                (pszGameEmpireDataColumn != NULL && (pvGameData[i] == pvData[j] || pvGameData[j] == pvData[i]))
                )
                
                ) {

                // A dup!!
                pbDup[j] = true;

                if (*ppiDuplicateKeys == NULL) {

                    *ppiDuplicateKeys = new int [iNumEmpires];
                    Assert(*ppiDuplicateKeys);

                    *ppiNumDuplicatesInList = new unsigned int [iNumEmpires / 2 + 1];
                    Assert(*ppiNumDuplicatesInList);
                    memset (*ppiNumDuplicatesInList, 0, (iNumEmpires / 2 + 1) * sizeof (int));
                }

                if ((*ppiNumDuplicatesInList)[*piNumDuplicates] == 0)
                {
                    bListStarted = true;
                    (*ppiDuplicateKeys)[iIndex] = pvEmpireKey[i].GetInteger();
                    (*ppiDuplicateKeys)[iIndex + 1] = pvEmpireKey[j].GetInteger();
                    (*ppiNumDuplicatesInList)[*piNumDuplicates] = 2;
                    iIndex += 2;
                }
                else
                {
                    (*ppiDuplicateKeys)[iIndex] = pvEmpireKey[j].GetInteger();
                    (*ppiNumDuplicatesInList)[*piNumDuplicates] ++;
                    iIndex ++;
                }
            }
        }

        if (bListStarted)
        {
            (*piNumDuplicates) ++;
        }
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// **ppiDuplicateKeys -> Flattened array of duplicate keys
// **ppiNumDuplicatesInList -> Number of keys in each set of duplicates
// *piNumDuplicates -> Number of duplicates lists found
//
// Return the number of empires requesting pause in a given game

int GameEngine::DoesEmpireHaveDuplicates (int iGameClass, int iGameNumber, int iEmpireKey, 
                                          const char* pszSystemEmpireDataColumn, int** ppiDuplicateKeys, 
                                          unsigned int* piNumDuplicates) {

    *ppiDuplicateKeys = NULL;
    *piNumDuplicates = 0;

    unsigned int i, iNumEmpires;
    Variant* pvEmpireKey = NULL, vOurData, vTheirData;
    AutoFreeData free(pvEmpireKey);

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    GET_GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

    // Get empires
    int iErrCode = t_pCache->ReadColumn(
        pszGameEmpires,
        GameEmpires::EmpireKey,
        NULL,
        &pvEmpireKey,
        &iNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
        return OK;

    RETURN_ON_ERROR(iErrCode);

    // Get empire's data
    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, pszSystemEmpireDataColumn, &vOurData);
    RETURN_ON_ERROR(iErrCode);

    // Scan for equals
    for (i = 0; i < iNumEmpires; i ++)
    {
        if (pvEmpireKey[i].GetInteger() != iEmpireKey)
        {
            GET_SYSTEM_EMPIRE_DATA(strThisEmpire, pvEmpireKey[i].GetInteger());
            iErrCode = t_pCache->ReadData(strThisEmpire, pvEmpireKey[i].GetInteger(), pszSystemEmpireDataColumn, &vTheirData);
            RETURN_ON_ERROR(iErrCode);
            
            if (vOurData == vTheirData)
            {
                // A dup!!
                if (*ppiDuplicateKeys == NULL)
                {
                    *ppiDuplicateKeys = new int [iNumEmpires];
                    Assert(*ppiDuplicateKeys);
                }
                
                (*ppiDuplicateKeys)[(*piNumDuplicates) ++] = pvEmpireKey[i].GetInteger();
            }
        }
    }

    return iErrCode;
}


int GameEngine::BuildDuplicateList(int* piDuplicateKeys, unsigned int iNumDuplicates, String* pstrDuplicateList)
{
    Assert(iNumDuplicates > 0);

    // Build list
    int iErrCode;
    unsigned int i, iLoopGuard = iNumDuplicates - 1;
    Variant vName;

    pstrDuplicateList->Clear();
    pstrDuplicateList->PreAllocate (iNumDuplicates * 25);

    for (i = 0; i < iLoopGuard;  i ++)
    {
        GET_SYSTEM_EMPIRE_DATA(strEmpire, piDuplicateKeys[i]);
        iErrCode = t_pCache->ReadData(strEmpire, piDuplicateKeys[i], SystemEmpireData::Name, &vName);
        RETURN_ON_ERROR(iErrCode);
        
        *pstrDuplicateList += vName.GetCharPtr();
        
        if (i == iLoopGuard - 1) {
            *pstrDuplicateList += " and ";
        } else {
            *pstrDuplicateList += ", ";
        }
    }
    
    // Get last guy
    GET_SYSTEM_EMPIRE_DATA(strEmpire, piDuplicateKeys[i]);
    iErrCode = t_pCache->ReadData(strEmpire, piDuplicateKeys[i], SystemEmpireData::Name, &vName);
    RETURN_ON_ERROR(iErrCode);
    
    *pstrDuplicateList += vName.GetCharPtr();

    return iErrCode;
}

int GameEngine::GetCumulativeDiplomacyCountForLimits(int iGameClass, int iGameNumber, int iEmpireKey, int iDiplomacyLevel, unsigned int* piCount)
{
     unsigned int iNumTruces, iNumTrades, iNumAlliances, iNumFormerAlliances;

     *piCount = 0;

     int iErrCode = GetDiplomacyCountsForLimits(iGameClass, iGameNumber, iEmpireKey, &iNumTruces, &iNumTrades, &iNumAlliances, &iNumFormerAlliances);
     RETURN_ON_ERROR(iErrCode);

     // We only count former alliances towards limits when you're actually trying to ally.
     // So you can have your only ally nuked in a permanent alliance game, and then go up to trade with someone else...
     switch (iDiplomacyLevel)
     {
     case TRUCE:
         *piCount = iNumTruces + iNumTrades + iNumAlliances;
         break;
     case TRADE:
         *piCount = iNumTrades + iNumAlliances;
         break;
     case ALLIANCE:
         *piCount = iNumAlliances + iNumFormerAlliances;
         break;
     }

     return iErrCode;
}

int GameEngine::GetDiplomacyCountsForLimits(int iGameClass, int iGameNumber, int iEmpireKey,
                                            unsigned int* piNumTruces, unsigned int* piNumTrades, unsigned int* piNumAlliances, unsigned int* piNumFormerAlliances)
{
    int iErrCode;
    GET_GAME_EMPIRE_DIPLOMACY(strGameEmpireDip, iGameClass, iGameNumber, iEmpireKey);

    *piNumTruces = *piNumTrades = *piNumAlliances = *piNumFormerAlliances = 0;

    const char* pszColumns[] = { GameEmpireDiplomacy::CurrentStatus, GameEmpireDiplomacy::DipOffer, GameEmpireDiplomacy::State };

    unsigned int iNumRows;
    Variant** ppvData = NULL;
    AutoFreeData free_ppvData(ppvData);

    iErrCode = t_pCache->ReadColumns(strGameEmpireDip, countof(pszColumns), pszColumns, NULL, &ppvData, &iNumRows);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    int iOptions;
    iErrCode = GetGameClassOptions(iGameClass, &iOptions);
    RETURN_ON_ERROR(iErrCode);

    if (iOptions & PERMANENT_ALLIANCES)
    {
        // Count allies of ours who were nuked as former alliances
        GET_GAME_EMPIRE_DATA(strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
        Variant vNukedAllies;
        iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::NumNukedAllies, &vNukedAllies);
        RETURN_ON_ERROR(iErrCode);

        (*piNumFormerAlliances) += vNukedAllies.GetInteger();
    }

    for (unsigned int i = 0; i < iNumRows; i ++)
    {
        int iCurrentStatus = ppvData[i][0].GetInteger();
        int iDipOffer = ppvData[i][1].GetInteger();
        int iState = ppvData[i][2].GetInteger();

        if ((iOptions & PERMANENT_ALLIANCES) && (iState & ONCE_ALLIED_WITH) && iCurrentStatus != ALLIANCE && iDipOffer != ALLIANCE)
        {
            // Count alliances we broke off as former alliances
            (*piNumFormerAlliances) ++;
        }

        // Take the greater of the status and the offer
        switch (iCurrentStatus)
        {
        case WAR:
            switch (iDipOffer)
            {
            case TRUCE:
                (*piNumTruces) ++;
                break;
            case TRADE:
                (*piNumTrades) ++;
                break;
            case ALLIANCE:
                (*piNumAlliances) ++;
                break;
            }
            break;

        case TRUCE:
            switch (iDipOffer)
            {
            case WAR:
            case TRUCE:
                (*piNumTruces) ++;
                break;
            case TRADE:
                (*piNumTrades) ++;
                break;
            case ALLIANCE:
                (*piNumAlliances) ++;
                break;
            }
            break;

        case TRADE:
            switch (iDipOffer)
            {
            case WAR:
            case TRUCE:
            case TRADE:
                (*piNumTrades) ++;
                break;
            case ALLIANCE:
                (*piNumAlliances) ++;
                break;
            }
            break;

        case ALLIANCE:
            (*piNumAlliances) ++;
            break;

        default:
            Assert(false);
            break;
        }
    }

    return iErrCode;
}

int GetCorrectTruceTradeAllianceCounts (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumTruces, unsigned int* piNumTrades, unsigned int* piNumAlliances)
{
    int iErrCode;

    unsigned int iNumTruces, iNumTrades, iNumAlliances, i, * piOfferingKey, iNumOffering;
    AutoFreeKeys free(piOfferingKey);
    Variant vStatus, vState, vOptions;

    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDip, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    // Truces
    iErrCode = t_pCache->GetEqualKeys(
        strGameEmpireDip,
        GameEmpireDiplomacy::CurrentStatus,
        TRUCE,
        NULL,
        &iNumTruces
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    // Trades
    iErrCode = t_pCache->GetEqualKeys(
        strGameEmpireDip,
        GameEmpireDiplomacy::CurrentStatus,
        TRADE,
        NULL,
        &iNumTrades
        );
    
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    
    // Alliances
    iErrCode = t_pCache->GetEqualKeys(
        strGameEmpireDip,
        GameEmpireDiplomacy::CurrentStatus,
        ALLIANCE,
        NULL,
        &iNumAlliances
        );
    
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    // Add lower offerings
    iNumTruces += iNumTrades + iNumAlliances;
    iNumTrades += iNumAlliances;
    
    // Offering truce from below
    iErrCode = t_pCache->GetEqualKeys(
        strGameEmpireDip,
        GameEmpireDiplomacy::DipOffer,
        TRUCE,
        &piOfferingKey,
        &iNumOffering
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    
    for (i = 0; i < iNumOffering; i ++)
    {
        iErrCode = t_pCache->ReadData(strGameEmpireDip, piOfferingKey[i], GameEmpireDiplomacy::CurrentStatus, &vStatus);
        RETURN_ON_ERROR(iErrCode);

        if (vStatus.GetInteger() < TRUCE)
        {
            iNumTruces ++;
        }
    }
    
    // Offering trade from below
    if (piOfferingKey)
    {
        t_pCache->FreeKeys(piOfferingKey);
        piOfferingKey = NULL;
    }

    iErrCode = t_pCache->GetEqualKeys(
        strGameEmpireDip,
        GameEmpireDiplomacy::DipOffer,
        TRADE,
        &piOfferingKey,
        &iNumOffering
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    
    for (i = 0; i < iNumOffering; i ++)
    {
        iErrCode = t_pCache->ReadData(strGameEmpireDip, piOfferingKey[i], GameEmpireDiplomacy::CurrentStatus, &vStatus);
        RETURN_ON_ERROR(iErrCode);
        
        if (vStatus.GetInteger() < TRADE)
        {
            iNumTrades ++;
        }

        if (vStatus.GetInteger() < TRUCE)
        {
            iNumTruces ++;
        }
    }

    // Offering alliance from below
    if (piOfferingKey)
    {
        t_pCache->FreeKeys(piOfferingKey);
        piOfferingKey = NULL;
    }

    iErrCode = t_pCache->GetEqualKeys(
        strGameEmpireDip,
        GameEmpireDiplomacy::DipOffer,
        ALLIANCE,
        &piOfferingKey,
        &iNumOffering
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);
    
    for (i = 0; i < iNumOffering; i ++) {

        iErrCode = t_pCache->ReadData(
            strGameEmpireDip,
            piOfferingKey[i],
            GameEmpireDiplomacy::State,
            &vState
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = t_pCache->ReadData(
            strGameEmpireDip,
            piOfferingKey[i],
            GameEmpireDiplomacy::CurrentStatus,
            &vStatus
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        // In permanent alliance games, don't count an alliance if it was leaked
        if (!(vOptions.GetInteger() & PERMANENT_ALLIANCES) || !(vState.GetInteger() & ONCE_ALLIED_WITH)) {
            
            if (vStatus.GetInteger() < ALLIANCE) {
                iNumAlliances ++;
            }
        }
        
        if (vStatus.GetInteger() < TRADE) {
            iNumTrades ++;
        }
        
        if (vStatus.GetInteger() < TRUCE) {
            iNumTruces ++;
        }
    }
    
    // Set retvals
    *piNumTruces = iNumTruces;
    *piNumTrades = iNumTrades;
    *piNumAlliances = iNumAlliances;

    return iErrCode;
}

int GameEngine::GetNumEmpiresAtDiplomaticStatusNextUpdate (int iGameClass, int iGameNumber, int iEmpireKey, 
                                                           int* piWar, int* piTruce, int* piTrade, int* piAlliance)
{
    const char* ppszColumns[] = {
        GameEmpireDiplomacy::ReferenceEmpireKey, 
        GameEmpireDiplomacy::DipOffer,
        GameEmpireDiplomacy::CurrentStatus
    };

    int iErrCode;

    GET_GAME_EMPIRE_DIPLOMACY(strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    int iWar = 0, iTruce = 0, iTrade = 0, iAlliance = 0;
    unsigned int iNumEmpires, i;

    Variant** ppvData = NULL;
    AutoFreeData free(ppvData);

    iErrCode = t_pCache->ReadColumns (
        strGameEmpireDiplomacy, 
        countof(ppszColumns),
        ppszColumns,
        NULL,
        &ppvData, 
        &iNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
        goto Cleanup;   // Set return values to zero
    }

    GOTO_CLEANUP_ON_ERROR(iErrCode);

    for (i = 0; i < iNumEmpires; i ++) {

        unsigned int iFoeKey = ppvData[i][0].GetInteger();
        int iWeOffer = ppvData[i][1].GetInteger();
        int iStatus = ppvData[i][2].GetInteger();

        int iTheyOffer;
        iErrCode = GetVisibleDiplomaticStatus(iGameClass, iGameNumber, iEmpireKey, iFoeKey, NULL, &iTheyOffer, NULL, NULL);
        GOTO_CLEANUP_ON_ERROR(iErrCode);

        switch (GetNextDiplomaticStatus(iWeOffer, iTheyOffer, iStatus))
        {
        case WAR:
            iWar ++;
            break;

        case TRUCE:
            iTruce ++;
            break;

        case TRADE:
            iTrade ++;
            break;

        case ALLIANCE:
            iAlliance ++;
            break;

        default:
            Assert(false);
            break;
        }
    }

Cleanup:

    if (piWar != NULL) {
        *piWar = iWar;
    }

    if (piTruce != NULL) {
        *piTruce = iTruce;
    }

    if (piTrade != NULL) {
        *piTrade = iTrade;
    }

    if (piAlliance != NULL) {
        *piAlliance = iAlliance;
    }

    return iErrCode;
}

int GameEngine::GetNumEmpiresAtDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey, int* piWar, 
                                                 int* piTruce, int* piTrade, int* piAlliance) {

    int iErrCode;

    Variant* pvData = NULL;
    AutoFreeData free(pvData);

    int iWar = 0, iTruce = 0, iTrade = 0, iAlliance = 0;
    unsigned int iNumRows, i;
    
    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);
    iErrCode = t_pCache->ReadColumn(strGameEmpireDiplomacy, GameEmpireDiplomacy::CurrentStatus, NULL, &pvData, &iNumRows);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    for (i = 0; i < iNumRows; i ++)
    {
        switch (pvData[i].GetInteger())
        {
        case WAR:
            iWar ++;
            break;

        case TRUCE:
            iTruce ++;
            break;

        case TRADE:
            iTrade ++;
            break;

        case ALLIANCE:
            iAlliance ++;
            break;

        default:
            Assert(false);
            break;
        }
    }

    if (piWar != NULL) {
        *piWar = iWar;
    }

    if (piTruce != NULL) {
        *piTruce = iTruce;
    }

    if (piTrade != NULL) {
        *piTrade = iTrade;
    }

    if (piAlliance != NULL) {
        *piAlliance = iAlliance;
    }

    return iErrCode;
}

int GameEngine::GetEmpiresAtDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey, 
                                              int piWar[], int* piNumWar, 
                                              int piTruce[], int* piNumTruce, 
                                              int piTrade[], int* piNumTrade, 
                                              int piAlliance[], int* piNumAlliance
                                              ) {

    int iErrCode;

    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    ICachedTable* pGameEmpireDiplomacy = NULL;
    AutoRelease<ICachedTable> release(pGameEmpireDiplomacy);

    iErrCode = t_pCache->GetTable(strGameEmpireDiplomacy, &pGameEmpireDiplomacy);
    RETURN_ON_ERROR(iErrCode);

    Variant* pvData = NULL;
    AutoFreeData free(pvData);

    int iWar = 0, iTruce = 0, iTrade = 0, iAlliance = 0, iKey;
    unsigned int iNumRows, * piKey = NULL;
    AutoFreeKeys freeKeys(piKey);

    iErrCode = pGameEmpireDiplomacy->ReadColumn(GameEmpireDiplomacy::CurrentStatus, &piKey, &pvData, &iNumRows);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    for (unsigned int i = 0; i < iNumRows; i ++)
    {
        iErrCode = pGameEmpireDiplomacy->ReadData(piKey[i], GameEmpireDiplomacy::ReferenceEmpireKey, &iKey);
        RETURN_ON_ERROR(iErrCode);

        switch (pvData[i].GetInteger())
        {
        case WAR:
            piWar [iWar ++] = iKey;
            break;

        case TRUCE:
            piTruce [iTruce ++] = iKey;
            break;

        case TRADE:
            piTrade [iTrade ++] = iKey;
            break;

        case ALLIANCE:
            piAlliance [iAlliance ++] = iKey;
            break;

        default:
            Assert(false);
            break;
        }
    }

    *piNumWar = iWar;
    *piNumTruce = iTruce;
    *piNumTrade = iTrade;
    *piNumAlliance = iAlliance;

    return iErrCode;
}


int GameEngine::GetLastUsedMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int* piLastUsedMask,
                                          unsigned int** ppiLastUsedProxyKeyArray, unsigned int* piNumLastUsed) {

    int iErrCode, iLastUsedMask;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    *piLastUsedMask = MESSAGE_TARGET_NONE;
    *ppiLastUsedProxyKeyArray = NULL;
    *piNumLastUsed = 0;

    // Read mask
    Variant vTemp;
    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::LastMessageTargetMask, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    *piLastUsedMask = iLastUsedMask = vTemp.GetInteger();

    // Leave early if we didn't cc anyone
    if (!(iLastUsedMask & MESSAGE_TARGET_INDIVIDUALS))
    {
        return OK;
    }

    // Read last used array
    iErrCode = t_pCache->GetEqualKeys(
        strGameEmpireDiplomacy,
        GameEmpireDiplomacy::LastMessageTargetFlag,
        1,
        ppiLastUsedProxyKeyArray,
        piNumLastUsed
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


int GameEngine::SetLastUsedMessageTarget(int iGameClass, int iGameNumber, int iEmpireKey, int iLastUsedMask,
                                         int* piLastUsedKeyArray, int iNumLastUsed) {

    int iErrCode, i;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    // Mask
    iErrCode = t_pCache->WriteData(strGameEmpireData, GameEmpireData::LastMessageTargetMask, iLastUsedMask);
    RETURN_ON_ERROR(iErrCode);

    // Leave early if we didn't cc anyone
    if (!(iLastUsedMask & MESSAGE_TARGET_INDIVIDUALS))
    {
        Assert(piLastUsedKeyArray == NULL && iNumLastUsed == 0);
        return OK;
    }

    // First set all values to 0
    unsigned int iKey = NO_KEY;
    while (true)
    {
        unsigned int iNextKey;
        iErrCode = t_pCache->GetNextKey(strGameEmpireDiplomacy, iKey, &iNextKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
            break;
        }
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->WriteData(strGameEmpireDiplomacy, iNextKey, GameEmpireDiplomacy::LastMessageTargetFlag, 0);
        RETURN_ON_ERROR(iErrCode);
        
        iKey = iNextKey;
    }

    // Now, set the provided keys
    for (i = 0; i < iNumLastUsed; i ++)
    {
        iErrCode = t_pCache->GetFirstKey(strGameEmpireDiplomacy, GameEmpireDiplomacy::ReferenceEmpireKey, piLastUsedKeyArray[i], &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            RETURN_ON_ERROR(iErrCode);
            iErrCode = t_pCache->WriteData(strGameEmpireDiplomacy, iKey, GameEmpireDiplomacy::LastMessageTargetFlag, 1);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}