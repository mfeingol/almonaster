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
// iGameClass -> Gameclass
// iGameNumber - > Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// **ppiEmpireKey -> Integer keys of empires
// *piNumEmpires -> Number of empires
//
// Return the keys of the empires in the given empire's diplomacy list

int GameEngine::GetKnownEmpireKeys (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvEmpireKey, 
                                    int* piNumEmpires) {

    GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pConn->ReadColumn (
        strGameEmpireDiplomacy, 
        GameEmpireDiplomacy::EmpireKey, 
        ppvEmpireKey, 
        (unsigned int*) piNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

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

int GameEngine::GetDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
                                     int* piWeOffer, int* piTheyOffer, int* piCurrent, bool* pbMet) {

    int iErrCode = OK;
    IReadTable* pTable = NULL;
    unsigned int iKey;

    if (piWeOffer != NULL) {
        *piWeOffer = WAR;
    }

    if (piTheyOffer != NULL) {
        *piTheyOffer = WAR;
    }

    if (piCurrent != NULL) {
        *piCurrent = WAR;
    }

    if (pbMet != NULL) {
        *pbMet = true;
    }

    if (piWeOffer != NULL || piCurrent != NULL) {

        GAME_EMPIRE_DIPLOMACY (strEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

        iErrCode = m_pConn->GetTableForReading (strEmpireDiplomacy, &pTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Get proxy key
        iErrCode = pTable->GetFirstKey (GameEmpireDiplomacy::EmpireKey, iFoeKey, &iKey);
        if (iErrCode != OK) {
            if (iErrCode == ERROR_DATA_NOT_FOUND) {

                iErrCode = OK;
                if (pbMet != NULL) {
                    *pbMet = false;
                }
            }
            goto Cleanup;
        }

        if (piWeOffer != NULL) {

            iErrCode = pTable->ReadData (iKey, GameEmpireDiplomacy::DipOffer, piWeOffer);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        if (piCurrent != NULL) {

            // Current status
            iErrCode = pTable->ReadData (iKey, GameEmpireDiplomacy::CurrentStatus, piCurrent);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

        SafeRelease (pTable);
    }

    // They offer
    if (piTheyOffer != NULL) {

        Variant vOptions;

        iErrCode = m_pConn->ReadData (
            SYSTEM_GAMECLASS_DATA, 
            iGameClass, 
            SystemGameClassData::Options, 
            &vOptions
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        GAME_EMPIRE_DIPLOMACY (strEmpireDiplomacy, iGameClass, iGameNumber, iFoeKey);
        
        iErrCode = m_pConn->GetTableForReading (strEmpireDiplomacy, &pTable);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        iErrCode = pTable->GetFirstKey (GameEmpireDiplomacy::EmpireKey, iEmpireKey, &iKey);
        if (iErrCode != OK) {
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                
                Assert (piWeOffer == NULL && piCurrent == NULL);
                iErrCode = OK;
                if (pbMet != NULL) {
                    *pbMet = false;
                }
            }
            goto Cleanup;
        }

        if (!(vOptions.GetInteger() & VISIBLE_DIPLOMACY)) {
            iErrCode = pTable->ReadData (iKey, GameEmpireDiplomacy::VirtualStatus, piTheyOffer);   
        } else {
            iErrCode = pTable->ReadData (iKey, GameEmpireDiplomacy::DipOffer, piTheyOffer);    
        }

        SafeRelease (pTable);

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    SafeRelease (pTable);

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

int GameEngine::GetDiplomaticOptions (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
                                      int piDipOptKey[], int* piSelected, int* piNumOptions) {

    int iErrCode = OK;

    *piNumOptions = 0;

    GAME_EMPIRE_DIPLOMACY (strDip, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    // Read selected option
    unsigned int iKey;
    Variant vWeOffer, vStatus, vLevel, vPrevDipKey, vOptions;

    iErrCode = m_pConn->GetFirstKey (strDip, GameEmpireDiplomacy::EmpireKey, iFoeKey, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pConn->ReadData (strDip, iKey, GameEmpireDiplomacy::DipOffer, &vWeOffer);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *piSelected = vWeOffer.GetInteger();

    // Read current status
    iErrCode = m_pConn->GetFirstKey (strDip, GameEmpireDiplomacy::EmpireKey, iFoeKey, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pConn->ReadData (strDip, iKey, GameEmpireDiplomacy::CurrentStatus, &vStatus);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Read gameclass conditions
    iErrCode = m_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vLevel
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    iErrCode = m_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    bool bUnbreakableAlliances = (vOptions.GetInteger() & UNBREAKABLE_ALLIANCES) != 0;

    // Get game state
    int iGameState;
    iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Switch on current level
    int iLevel = vLevel.GetInteger();
    switch (vStatus.GetInteger()) {

    case WAR:

        // Add War
        piDipOptKey[(*piNumOptions) ++] = WAR;
        
        // Add truce?
        if (GameAllowsDiplomacy (iLevel, TRUCE)) {

            iErrCode = AddDiplomaticOption (iGameClass, iGameNumber, iKey, strGameEmpireData, strDip,
                GameEmpireData::NumTruces, TRUCE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

        } else {

            // Add trade?
            if (GameAllowsDiplomacy (iLevel, TRADE)) {
                
                iErrCode = AddDiplomaticOption (iGameClass, iGameNumber, iKey, strGameEmpireData, strDip, 
                    GameEmpireData::NumTrades, TRADE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
            } else {
                
                // Alliance
                if (GameAllowsDiplomacy (iLevel, ALLIANCE)) {
                    iErrCode = AddDiplomaticOption (iGameClass, iGameNumber, iKey, strGameEmpireData, strDip, 
                        GameEmpireData::NumAlliances, ALLIANCE, vWeOffer.GetInteger(), piDipOptKey, 
                        piNumOptions);
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                }
            }
        }
        
        break;

    case TRUCE:

        // Add war and truce
        piDipOptKey[(*piNumOptions) ++] = WAR;
        piDipOptKey[(*piNumOptions) ++] = TRUCE;

        // Trade
        if (GameAllowsDiplomacy (iLevel, TRADE)) {
            
            iErrCode = AddDiplomaticOption (iGameClass, iGameNumber, iKey, strGameEmpireData, strDip,
                GameEmpireData::NumTrades, TRADE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);

            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

        } else {
            
            // Alliance         
            if (GameAllowsDiplomacy (iLevel, ALLIANCE)) {
                
                iErrCode = AddDiplomaticOption (iGameClass, iGameNumber, iKey, strGameEmpireData, strDip,
                    GameEmpireData::NumAlliances, ALLIANCE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
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
        if (GameAllowsDiplomacy (iLevel, ALLIANCE)) {

            iErrCode = AddDiplomaticOption (iGameClass, iGameNumber, iKey, strGameEmpireData, strDip,
                GameEmpireData::NumAlliances, ALLIANCE, vWeOffer.GetInteger(), piDipOptKey, piNumOptions);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
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
        
        Assert (false);
        break;
    }

    // Add surrender options
    if (vStatus.GetInteger() == WAR && 
        GameAllowsDiplomacy (iLevel, SURRENDER) && 
        !(vOptions.GetInteger() & ONLY_SURRENDER_WITH_TWO_EMPIRES) &&
        !(iGameState & STILL_OPEN)
        ) {

        unsigned int iNumEmps;
        GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
        
        iErrCode = m_pConn->GetNumRows (strGameEmpires, &iNumEmps);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        if (iNumEmps > 2) {

            // Make sure not allied before
            Variant vState;

            iErrCode = m_pConn->ReadData (strDip, iKey, GameEmpireDiplomacy::State, &vState);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            if (!(vState.GetInteger() & ONCE_ALLIED_WITH)) {

                // Add surrender option if offered to this person or not being offered to anyone else
                if (vWeOffer.GetInteger() == SURRENDER) {
                    piDipOptKey[(*piNumOptions) ++] = SURRENDER;
                } else {
                    
                    unsigned int iNumSurrendering;
                    iErrCode = m_pConn->GetEqualKeys (
                        strDip,
                        GameEmpireDiplomacy::DipOffer,
                        SURRENDER,
                        NULL,
                        &iNumSurrendering
                        );
                    
                    if (iErrCode == ERROR_DATA_NOT_FOUND) {
                        piDipOptKey[(*piNumOptions) ++] = SURRENDER;
                    }
                    iErrCode = OK;
                }
                
                piDipOptKey[(*piNumOptions) ++] = ACCEPT_SURRENDER;
            }
        }
    }

    return iErrCode;
}

int GameEngine::AddDiplomaticOption (int iGameClass, int iGameNumber, int iTargetEmpireKey,
                                     const char* pszEmpireData, const char* pszEmpireDiplomacy, 
                                     const char* pszEmpireDataColumn, int iDiplomacyLevel, 
                                     int iCurrentDiplomacyLevel, int piDipOptKey[], int* piNumOptions) {

    int iErrCode;

    if (iDiplomacyLevel == iCurrentDiplomacyLevel) {
        piDipOptKey[(*piNumOptions) ++] = iDiplomacyLevel;
    } else {

        int iMaxCount;

        iErrCode = GetMaxNumDiplomacyPartners (iGameClass, iGameNumber, iDiplomacyLevel, &iMaxCount);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        if (iMaxCount == UNRESTRICTED_DIPLOMACY) {
            piDipOptKey[(*piNumOptions) ++] = iDiplomacyLevel;
        } else {
            
            Variant vValue;
            int iErrCode = m_pConn->ReadData (pszEmpireData, pszEmpireDataColumn, &vValue);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            Assert (vValue.GetInteger() >= 0);
            
            if (vValue.GetInteger() < iMaxCount) {
            
                piDipOptKey[(*piNumOptions) ++] = iDiplomacyLevel;
            
            } else {
                
                // If permanent alliances are set and we have the once allied bit set, add the option
                if (iDiplomacyLevel == ALLIANCE) {
                    
                    iErrCode = m_pConn->ReadData (
                        SYSTEM_GAMECLASS_DATA,
                        iGameClass,
                        SystemGameClassData::Options,
                        &vValue
                        );
                    
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    if (vValue.GetInteger() & PERMANENT_ALLIANCES) {
                        
                        iErrCode = m_pConn->ReadData (
                            pszEmpireDiplomacy,
                            iTargetEmpireKey,
                            GameEmpireDiplomacy::State,
                            &vValue
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }
                        
                        if (vValue.GetInteger() & ONCE_ALLIED_WITH) {               
#ifdef _DEBUG
                            // Make sure we have at least one leaked alliance
                            iErrCode = m_pConn->ReadData (
                                pszEmpireData, 
                                GameEmpireData::NumAlliancesLeaked,
                                &vValue
                                );

                            Assert (iErrCode == OK && vValue.GetInteger() > 0);
#endif
                            piDipOptKey[(*piNumOptions) ++] = ALLIANCE;
                        }
                    }
                }
            }
        }
    }

    return OK;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of source empire
// iFoeKey -> Integer key of target empire
// iDipOffer -> Key of dip offered
//
// Updates the diplomatic status of one player with respect to another

int GameEngine::UpdateDiplomaticOffer (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
                                       int iDipOffer) {

    int iErrCode;
    GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);
    
    // Make sure empires are still in game and have met
    unsigned int iKey;
    iErrCode = m_pConn->GetFirstKey (
        strGameEmpireDiplomacy, 
        GameEmpireDiplomacy::EmpireKey, 
        iFoeKey, 
        &iKey
        );

    if (iKey == NO_KEY) {
        return iErrCode;
    }

    // Get current status and current offering
    Variant vStatus, vOffering;

    iErrCode = m_pConn->ReadData (strGameEmpireDiplomacy, iKey, GameEmpireDiplomacy::CurrentStatus, &vStatus);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pConn->ReadData (strGameEmpireDiplomacy, iKey, GameEmpireDiplomacy::DipOffer, &vOffering);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // If current offer is new offer, return an error
    if (vOffering.GetInteger() == iDipOffer) {
        return ERROR_FAILURE;
    }

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    //////////////////
    // Verify offer //
    //////////////////

    Variant vDipLevel, vOptions;

    iErrCode = m_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA, 
        iGameClass, 
        SystemGameClassData::DiplomacyLevel, 
        &vDipLevel
        );
    
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );
    
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    bool bPermanentAlliances = (vOptions.GetInteger() & PERMANENT_ALLIANCES) != 0;

    // Get game state
    int iGameState;
    iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // If alliances are unbreakable, don't allow any changes
    if ((vOptions.GetInteger() & UNBREAKABLE_ALLIANCES) && 
        vStatus.GetInteger() == ALLIANCE &&
        iDipOffer != ALLIANCE
        ) {

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

    } else if (iDipOffer == SURRENDER || iDipOffer == ACCEPT_SURRENDER) {
        
        if (GameAllowsDiplomacy (iDipLevel, SURRENDER) && 
            vStatus.GetInteger() == WAR &&
            !(vOptions.GetInteger() & ONLY_SURRENDER_WITH_TWO_EMPIRES) &&
            !(iGameState & STILL_OPEN)) {

            // Allow surrenders?
            unsigned int iNumEmps;
            GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);
            
            iErrCode = m_pConn->GetNumRows (strGameEmpires, &iNumEmps);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
            
            if (iNumEmps > 2) {

                // Make sure not allied before
                Variant vState;
                
                iErrCode = m_pConn->ReadData (
                    strGameEmpireDiplomacy, 
                    iKey, 
                    GameEmpireDiplomacy::State, 
                    &vState
                    );
                
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
                
                if (!(vState.GetInteger() & ONCE_ALLIED_WITH)) {
                
                    if (iDipOffer == SURRENDER) {
                        
                        unsigned int iNumSurrendering;
                        iErrCode = m_pConn->GetEqualKeys (
                            strGameEmpireDiplomacy,
                            GameEmpireDiplomacy::DipOffer,
                            SURRENDER,
                            NULL,
                            &iNumSurrendering
                            );
                        
                        if (iErrCode != OK) {

                            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                                bUpdate = true;
                                iErrCode = OK;
                            } else {
                                Assert (false);
                                return iErrCode;
                            }
                        }
                        
                    } else {
                        
                        Assert (iDipOffer == ACCEPT_SURRENDER);
                        bUpdate = true;
                    }
                }
            }
        }
        
    } else {
        
        bool bLimited = false;

        // Make sure that we are under the limit if we're trying to upgrade
        if (iDipOffer > vStatus.GetInteger()) {

            int iMax;
            iErrCode = GetMaxNumDiplomacyPartners (iGameClass, iGameNumber, iDipOffer, &iMax);
            if (iErrCode != OK) {
                return iErrCode;
            }

            if (iMax != UNRESTRICTED_DIPLOMACY) {

                Variant vCount;
                const char* pszColumn;

                switch (iDipOffer) {    
                case TRUCE:
                    pszColumn = GameEmpireData::NumTruces;
                    break;
                case TRADE:
                    pszColumn = GameEmpireData::NumTrades;
                    break;
                case ALLIANCE:
                    pszColumn = GameEmpireData::NumAlliances;
                    break;
                default:
                    return ERROR_INVALID_ARGUMENT;
                }
                
                iErrCode = m_pConn->ReadData(strGameEmpireData, pszColumn, &vCount);
                if (iErrCode != OK) {
                    return iErrCode;
                }

                Assert (vCount.GetInteger() >= 0);
                
                bLimited = vCount.GetInteger() >= iMax;
                
                if (bLimited && iDipOffer == ALLIANCE) {
                    
                    // Allow if once allied bit is set
                    iErrCode = m_pConn->ReadData (
                        SYSTEM_GAMECLASS_DATA,
                        iGameClass,
                        SystemGameClassData::Options,
                        &vCount
                        );
                    
                    if (iErrCode != OK) {
                        Assert (false);
                        return iErrCode;
                    }
                    
                    if (vCount.GetInteger() & PERMANENT_ALLIANCES) {
                        
                        iErrCode = m_pConn->ReadData (
                            strGameEmpireDiplomacy,
                            iKey,
                            GameEmpireDiplomacy::State,
                            &vCount
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }
                        
                        if (vCount.GetInteger() & ONCE_ALLIED_WITH) {
#ifdef _DEBUG
                            // Make sure we have at least one leaked alliance
                            iErrCode = m_pConn->ReadData (
                                strGameEmpireData, 
                                GameEmpireData::NumAlliancesLeaked,
                                &vCount
                                );
                            
                            Assert (iErrCode == OK && vCount.GetInteger() > 0);
#endif
                            bLimited = false;
                        }
                    }

                }
            }
        }

        if (bLimited) {
            bUpdate = false;
        } else {

            // Validate with gameclass diplomacy type
            switch (iDipOffer) {
                
            case TRUCE:
                
                Assert (GameAllowsDiplomacy (iDipLevel, TRUCE));

                switch (vStatus.GetInteger()) {
                    
                case WAR:
                    bUpdate = GameAllowsDiplomacy (iDipLevel, TRUCE);
                    break;
                    
                case TRUCE:
                    bUpdate = true;
                    break;
                    
                case TRADE:
                    bUpdate = GameAllowsDiplomacy (iDipLevel, TRADE);
                    break;
                    
                case ALLIANCE:
                    bUpdate = GameAllowsDiplomacy (iDipLevel, ALLIANCE) && 
                              !GameAllowsDiplomacy (iDipLevel, TRADE);
                    break;
                }
                break;
                
            case TRADE:

                Assert (GameAllowsDiplomacy (iDipLevel, TRADE));
                
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
                Assert (false);
                break;
            }
        }
    }

    // After all this, update the setting
    if (bUpdate) {

        // Write the new offer
        iErrCode = m_pConn->WriteData (
            strGameEmpireDiplomacy, 
            iKey, 
            GameEmpireDiplomacy::DipOffer, 
            iDipOffer
            );

        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Adjust non-diplomatic settings for counts
        if (iDipOffer > ALLIANCE) {
            iDipOffer = WAR;
        }

        if (vOffering.GetInteger() > ALLIANCE) {
            vOffering = vStatus.GetInteger();
        }
        
        if (iDipOffer > vOffering.GetInteger()) {
            
            if (iDipOffer != vStatus.GetInteger()) {
                
                // Best effort
                // Increase new offering count
                switch (iDipOffer) {
                
                case TRUCE:

                    iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTruces, 1);
                    Assert (iErrCode == OK);
                    
                    break;
                
                case TRADE:
                    
                    iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTrades, 1);
                    Assert (iErrCode == OK);

                    if (iErrCode == OK && !GameAllowsDiplomacy (iDipLevel, TRUCE)) {
                        iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTruces, 1);
                        Assert (iErrCode == OK);
                    }

                    break;
                
                case ALLIANCE:
                    {

                    bool bIncrement = true;

                    // If permanent alliances and we once allied, don't increment
                    if (bPermanentAlliances) {
                        
                        Variant vState;
                        
                        iErrCode = m_pConn->ReadData (
                            strGameEmpireDiplomacy, 
                            iKey, 
                            GameEmpireDiplomacy::State, 
                            &vState
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }

                        if (vState.GetInteger() & ONCE_ALLIED_WITH) {
                            bIncrement = false;
                        }
                    }

                    if (bIncrement) {
                        iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumAlliances, 1);
                        Assert (iErrCode == OK);
                    }

                    if (iErrCode == OK && !GameAllowsDiplomacy (iDipLevel, TRADE)) {
                        iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTrades, 1);
                        Assert (iErrCode == OK);
                        
                        if (iErrCode == OK && !GameAllowsDiplomacy (iDipLevel, TRUCE)) {
                            iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTruces, 1);
                            Assert (iErrCode == OK);
                        }
                    }

                    }   // Scope
                    break;
                }
#ifdef _DEBUG
                int iErrCode2 = CheckTruceTradeAllianceCounts (iGameClass, iGameNumber, iEmpireKey);
                Assert (iErrCode2 == OK);
#endif
            }
            
        } else {
            
            if (vOffering.GetInteger() != vStatus.GetInteger()) {
                
                // Decrease old offering
                switch (vOffering.GetInteger()) {
                
                case TRUCE:
                    
                    iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTruces, -1);
                    Assert (iErrCode == OK);
                    break;

                case TRADE:
                    
                    iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTrades, -1);
                    Assert (iErrCode == OK);

                    if (iErrCode == OK && !GameAllowsDiplomacy (iDipLevel, TRUCE)) {
                        iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTruces, -1);
                        Assert (iErrCode == OK);
                    }

                    break;
                
                case ALLIANCE:

                    {

                    bool bIncrement = true;

                    // If permanent alliances and we once allied, don't increment
                    if (bPermanentAlliances) {
                        
                        Variant vState;
                        
                        iErrCode = m_pConn->ReadData (
                            strGameEmpireDiplomacy, 
                            iKey, 
                            GameEmpireDiplomacy::State, 
                            &vState
                            );
                        
                        if (iErrCode != OK) {
                            Assert (false);
                            return iErrCode;
                        }

                        if (vState.GetInteger() & ONCE_ALLIED_WITH) {
                            bIncrement = false;
                        }
                    }
                
                    if (bIncrement) {
                        iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumAlliances, -1);
                        Assert (iErrCode == OK);
                    }

                    if (iErrCode == OK && !GameAllowsDiplomacy (iDipLevel, TRADE)) {
                        iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTrades, -1);
                        Assert (iErrCode == OK);
                        
                        if (iErrCode == OK && !GameAllowsDiplomacy (iDipLevel, TRUCE)) {
                            iErrCode = m_pConn->Increment (strGameEmpireData, GameEmpireData::NumTruces, -1);
                            Assert (iErrCode == OK);
                        }
                    }

                    }   // Scope
                    break;
                }
#ifdef _DEBUG
                int iErrCode2 = CheckTruceTradeAllianceCounts (iGameClass, iGameNumber, iEmpireKey);
                Assert (iErrCode2 == OK);
#endif
            }
        }
    }

    if (iErrCode != OK) {

        // Try to reset previous value
        int iErrCode2 = 
            m_pConn->WriteData (
            strGameEmpireDiplomacy, 
            iKey, 
            GameEmpireDiplomacy::DipOffer, 
            vStatus
            );
        Assert (iErrCode2 == OK);
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
                                     unsigned int** ppiNumDuplicatesInList, unsigned int* piNumDuplicates) {

    *ppiDuplicateKeys = NULL;
    *ppiNumDuplicatesInList = NULL;
    *piNumDuplicates = 0;

    unsigned int i, j, iNumEmpires, iIndex = 0;
    Variant* pvEmpireKey = NULL, * pvData = NULL, * pvGameData = NULL;
    bool* pbDup, bListStarted;

    GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

    char strGameEmpireData [256];

    // Get empires
    int iErrCode = m_pConn->ReadColumn (
        strGameEmpires,
        GameEmpires::EmpireKey,
        &pvEmpireKey,
        &iNumEmpires
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (iNumEmpires == 0) return OK;

    // Allocate memory
    if (pszGameEmpireDataColumn != NULL) {

        pvData = new Variant [iNumEmpires * 2];
        if (pvData == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }

        pvGameData = pvData + iNumEmpires;
        
        for (i = 0; i < iNumEmpires; i ++) {
            
            GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
            
            iErrCode = m_pConn->ReadData (
                strGameEmpireData,
                pszGameEmpireDataColumn,
                pvGameData + i
                );
            
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }

    } else {

        pvData = new Variant [iNumEmpires];
        if (pvData == NULL) {
            iErrCode = ERROR_OUT_OF_MEMORY;
            goto Cleanup;
        }
    }

    pbDup = (bool*) StackAlloc (iNumEmpires * sizeof (bool));

    // Get all empire values
    for (i = 0; i < iNumEmpires; i ++) {

        iErrCode = m_pConn->ReadData (
            SYSTEM_EMPIRE_DATA,
            pvEmpireKey[i].GetInteger(),
            pszSystemEmpireDataColumn,
            pvData + i
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        pbDup[i] = false;
    }

    for (i = 0; i < iNumEmpires; i ++) {

        if (pbDup[i]) {
            continue;
        }

        bListStarted = false;

        for (j = i + 1; j < iNumEmpires; j ++) {
            
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
                    if (*ppiDuplicateKeys == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }

                    *ppiNumDuplicatesInList = new unsigned int [iNumEmpires / 2 + 1];
                    if (*ppiNumDuplicatesInList == NULL) {
                        delete [] *ppiDuplicateKeys;
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                    
                    memset (*ppiNumDuplicatesInList, 0, (iNumEmpires / 2 + 1) * sizeof (int));
                }

                if ((*ppiNumDuplicatesInList)[*piNumDuplicates] == 0) {
                    bListStarted = true;
                    (*ppiDuplicateKeys)[iIndex] = pvEmpireKey[i].GetInteger();
                    (*ppiDuplicateKeys)[iIndex + 1] = pvEmpireKey[j].GetInteger();
                    (*ppiNumDuplicatesInList)[*piNumDuplicates] = 2;
                    iIndex += 2;
                } else {
                    (*ppiDuplicateKeys)[iIndex] = pvEmpireKey[j].GetInteger();
                    (*ppiNumDuplicatesInList)[*piNumDuplicates] ++;
                    iIndex ++;
                }
            }
        }

        if (bListStarted) {
            (*piNumDuplicates) ++;
        }
    }

Cleanup:
    
    if (pvData != NULL) {
        delete [] pvData;
    }

    if (pvEmpireKey != NULL) {
        FreeData (pvEmpireKey);
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
    Variant* pvEmpireKey, vOurData, vTheirData;

    GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

    // Get empires
    int iErrCode = m_pConn->ReadColumn (
        pszGameEmpires,
        GameEmpires::EmpireKey,
        &pvEmpireKey,
        &iNumEmpires
        );

    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }
    
    if (iNumEmpires == 0) return OK;

    // Get empire's data
    iErrCode = m_pConn->ReadData (
        SYSTEM_EMPIRE_DATA,
        iEmpireKey,
        pszSystemEmpireDataColumn,
        &vOurData
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Scan for equals
    for (i = 0; i < iNumEmpires; i ++) {

        if (pvEmpireKey[i].GetInteger() != iEmpireKey) {
            
            iErrCode = m_pConn->ReadData (
                SYSTEM_EMPIRE_DATA,
                pvEmpireKey[i].GetInteger(),
                pszSystemEmpireDataColumn,
                &vTheirData
                );
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (vOurData == vTheirData) {

                // A dup!!
                if (*ppiDuplicateKeys == NULL) {
                    *ppiDuplicateKeys = new int [iNumEmpires];
                    if (*ppiDuplicateKeys == NULL) {
                        iErrCode = ERROR_OUT_OF_MEMORY;
                        goto Cleanup;
                    }
                }
                
                (*ppiDuplicateKeys)[(*piNumDuplicates) ++] = pvEmpireKey[i].GetInteger();
            }
        }
    }

Cleanup:

    if (pvEmpireKey != NULL) {
        FreeData (pvEmpireKey);
    }

    return iErrCode;
}


int GameEngine::BuildDuplicateList (int* piDuplicateKeys, unsigned int iNumDuplicates, 
                                    String* pstrDuplicateList) {

    Assert (iNumDuplicates > 0);

    // Build list
    int iErrCode;
    unsigned int i, iLoopGuard = iNumDuplicates - 1;
    Variant vName;

    pstrDuplicateList->Clear();
    pstrDuplicateList->PreAllocate (iNumDuplicates * 25);

    for (i = 0; i < iLoopGuard;  i ++) {
        
        iErrCode = m_pConn->ReadData (
            SYSTEM_EMPIRE_DATA,
            piDuplicateKeys[i],
            SystemEmpireData::Name,
            &vName
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        *pstrDuplicateList += vName.GetCharPtr();
        
        if (i == iLoopGuard - 1) {
            *pstrDuplicateList += " and ";
        } else {
            *pstrDuplicateList += ", ";
        }
    }
    
    // Get last guy
    iErrCode = m_pConn->ReadData (
        SYSTEM_EMPIRE_DATA,
        piDuplicateKeys[i],
        SystemEmpireData::Name,
        &vName
        );
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    *pstrDuplicateList += vName.GetCharPtr();

Cleanup:

    return iErrCode;
}

int GameEngine::GetCorrectTruceTradeAllianceCounts (int iGameClass, int iGameNumber, int iEmpireKey,
                                                    int* piNumTruces, int* piNumTrades, int* piNumAlliances) {
    
    int iErrCode;

    unsigned int iNumTruces, iNumTrades, iNumAlliances, i, * piOfferingKey, iNumOffering;
    Variant vStatus, vState, vOptions;

    GAME_EMPIRE_DIPLOMACY (strGameEmpireDip, iGameClass, iGameNumber, iEmpireKey);

    iErrCode = m_pConn->ReadData (
        SYSTEM_GAMECLASS_DATA,
        iGameClass,
        SystemGameClassData::Options,
        &vOptions
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Truces
    iErrCode = m_pConn->GetEqualKeys (
        strGameEmpireDip,
        GameEmpireDiplomacy::CurrentStatus,
        TRUCE,
        NULL,
        &iNumTruces
        );

    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }

    // Trades
    iErrCode = m_pConn->GetEqualKeys (
        strGameEmpireDip,
        GameEmpireDiplomacy::CurrentStatus,
        TRADE,
        NULL,
        &iNumTrades
        );
    
    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }
    
    // Alliances
    iErrCode = m_pConn->GetEqualKeys (
        strGameEmpireDip,
        GameEmpireDiplomacy::CurrentStatus,
        ALLIANCE,
        NULL,
        &iNumAlliances
        );
    
    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }

    // Add lower offerings
    iNumTruces += iNumTrades + iNumAlliances;
    iNumTrades += iNumAlliances;
    
    // Offering truce from below
    iErrCode = m_pConn->GetEqualKeys (
        strGameEmpireDip,
        GameEmpireDiplomacy::DipOffer,
        TRUCE,
        &piOfferingKey,
        &iNumOffering
        );

    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }
    
    for (i = 0; i < iNumOffering; i ++) {
        
        iErrCode = m_pConn->ReadData (
            strGameEmpireDip,
            piOfferingKey[i],
            GameEmpireDiplomacy::CurrentStatus,
            &vStatus
            );
        if (iErrCode != OK) {
            Assert (false);
            m_pConn->FreeKeys(piOfferingKey);
            return iErrCode;
        }
        
        if (vStatus.GetInteger() < TRUCE) {
            iNumTruces ++;
        }
    }
    
    if (iNumOffering > 0) {
        m_pConn->FreeKeys(piOfferingKey);
    }
    
    // Offering trade from below
    iErrCode = m_pConn->GetEqualKeys (
        strGameEmpireDip,
        GameEmpireDiplomacy::DipOffer,
        TRADE,
        &piOfferingKey,
        &iNumOffering
        );

    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }
    
    for (i = 0; i < iNumOffering; i ++) {
        
        iErrCode = m_pConn->ReadData (
            strGameEmpireDip,
            piOfferingKey[i],
            GameEmpireDiplomacy::CurrentStatus,
            &vStatus
            );
        if (iErrCode != OK) {
            Assert (false);
            m_pConn->FreeKeys(piOfferingKey);
            return iErrCode;
        }
        
        if (vStatus.GetInteger() < TRADE) {
            iNumTrades ++;
        }

        if (vStatus.GetInteger() < TRUCE) {
            iNumTruces ++;
        }
    }

    if (iNumOffering > 0) {
        m_pConn->FreeKeys(piOfferingKey);
    }
    
    // Offering alliance from below
    iErrCode = m_pConn->GetEqualKeys (
        strGameEmpireDip,
        GameEmpireDiplomacy::DipOffer,
        ALLIANCE,
        &piOfferingKey,
        &iNumOffering
        );

    if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
        Assert (false);
        return iErrCode;
    }
    
    for (i = 0; i < iNumOffering; i ++) {

        iErrCode = m_pConn->ReadData (
            strGameEmpireDip,
            piOfferingKey[i],
            GameEmpireDiplomacy::State,
            &vState
            );
        
        if (iErrCode != OK) {
            Assert (false);
            m_pConn->FreeKeys(piOfferingKey);
            return iErrCode;
        }
        
        iErrCode = m_pConn->ReadData (
            strGameEmpireDip,
            piOfferingKey[i],
            GameEmpireDiplomacy::CurrentStatus,
            &vStatus
            );
        
        if (iErrCode != OK) {
            Assert (false);
            m_pConn->FreeKeys(piOfferingKey);
            return iErrCode;
        }
        
        // In permanent alliance games, don't count an alliance if it wasleaked
        if (!(vOptions.GetInteger() & PERMANENT_ALLIANCES) || 
            !(vState.GetInteger() & ONCE_ALLIED_WITH)) {
            
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
    
    if (iNumOffering > 0) {
        m_pConn->FreeKeys(piOfferingKey);
    }

    // Set retvals
    *piNumTruces = iNumTruces;
    *piNumTrades = iNumTrades;
    *piNumAlliances = iNumAlliances;

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}

int GameEngine::CheckTruceTradeAllianceCounts (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iNumTruces, iNumTrades, iNumAlliances, iNumRealTruces, iNumRealTrades, iNumRealAlliances,
            iNumAlliancesLeaked, iErrCode;

    IReadTable* pGameEmpireData = NULL;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    // Look for dip count bugs;
    iErrCode = m_pConn->GetTableForReading (strGameEmpireData, &pGameEmpireData);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::NumTruces, &iNumTruces);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::NumTrades, &iNumTrades);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::NumAlliances, &iNumAlliances);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    iErrCode = pGameEmpireData->ReadData (GameEmpireData::NumAlliancesLeaked, &iNumAlliancesLeaked);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    // Release table
    SafeRelease (pGameEmpireData);
    
    // Adjust for leaked alliances
    iNumAlliances -= iNumAlliancesLeaked;

    iErrCode = GetCorrectTruceTradeAllianceCounts (
        iGameClass,
        iGameNumber,
        iEmpireKey,
        &iNumRealTruces,
        &iNumRealTrades,
        &iNumRealAlliances
        );
    
    if (iErrCode == OK) {
        
        Assert (iNumTruces == iNumRealTruces);
        Assert (iNumTrades == iNumRealTrades);
        Assert (iNumAlliances == iNumRealAlliances);
        
        if (iNumTruces != iNumRealTruces ||
            iNumTrades != iNumRealTrades ||
            iNumAlliances != iNumRealAlliances
            ) {
            
            iErrCode = ERROR_FAILURE;
        }
    }

Cleanup:

    SafeRelease (pGameEmpireData);

    return iErrCode;
}

int GameEngine::GetNumEmpiresAtDiplomaticStatusNextUpdate (int iGameClass, int iGameNumber, int iEmpireKey, 
                                                           int* piWar, int* piTruce, int* piTrade, 
                                                           int* piAlliance) {

    const char* ppszColumns[] = {
        GameEmpireDiplomacy::EmpireKey, 
        GameEmpireDiplomacy::DipOffer,
        GameEmpireDiplomacy::CurrentStatus
    };

    int iErrCode;

    GAME_EMPIRE_DIPLOMACY(strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    int iWar = 0, iTruce = 0, iTrade = 0, iAlliance = 0;
    unsigned int iNumEmpires, i;

    Variant** ppvData = NULL;

    iErrCode = m_pConn->ReadColumns (
        strGameEmpireDiplomacy, 
        countof(ppszColumns),
        ppszColumns, 
        &ppvData, 
        &iNumEmpires
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
        goto Cleanup;   // Set return values to zero
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    for (i = 0; i < iNumEmpires; i ++) {

        unsigned int iFoeKey = ppvData[i][0].GetInteger();
        int iWeOffer = ppvData[i][1].GetInteger();
        int iStatus = ppvData[i][2].GetInteger();

        int iTheyOffer;
        iErrCode = GetDiplomaticStatus (iGameClass, iGameNumber, iEmpireKey, iFoeKey, NULL, &iTheyOffer, NULL, NULL);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        switch (GetNextDiplomaticStatus (iWeOffer, iTheyOffer, iStatus)) {

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
            Assert (false);
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

    if (ppvData != NULL) {
        m_pConn->FreeData(ppvData);
    }

    return iErrCode;
}

int GameEngine::GetNumEmpiresAtDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey, int* piWar, 
                                                 int* piTruce, int* piTrade, int* piAlliance) {

    int iErrCode;

    GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    IReadTable* pGameEmpireDiplomacy = NULL;

    int* piData = NULL, iWar = 0, iTruce = 0, iTrade = 0, iAlliance = 0;
    unsigned int iNumRows, i;

    iErrCode = m_pConn->GetTableForReading (strGameEmpireDiplomacy, &pGameEmpireDiplomacy);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = pGameEmpireDiplomacy->ReadColumn (
        GameEmpireDiplomacy::CurrentStatus,
        NULL,
        &piData,
        &iNumRows
        );

    SafeRelease (pGameEmpireDiplomacy);

    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        } else {
            Assert (false);
            goto Cleanup;
        }
    }

    for (i = 0; i < iNumRows; i ++) {

        switch (piData[i]) {

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
            Assert (false);
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

Cleanup:

    SafeRelease (pGameEmpireDiplomacy);

    if (piData != NULL) {
        m_pConn->FreeData(piData);
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

    GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    IReadTable* pGameEmpireDiplomacy = NULL;

    iErrCode = m_pConn->GetTableForReading (strGameEmpireDiplomacy, &pGameEmpireDiplomacy);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    int* piData = NULL, iWar = 0, iTruce = 0, iTrade = 0, iAlliance = 0, iKey;
    unsigned int iNumRows, i, * piKey = NULL;

    iErrCode = pGameEmpireDiplomacy->ReadColumn (
        GameEmpireDiplomacy::CurrentStatus,
        &piKey,
        &piData,
        &iNumRows
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    for (i = 0; i < iNumRows; i ++) {

        iErrCode = pGameEmpireDiplomacy->ReadData (
            piKey[i],
            GameEmpireDiplomacy::EmpireKey,
            &iKey
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        switch (piData[i]) {

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
            Assert (false);
            break;
        }
    }

    *piNumWar = iWar;
    *piNumTruce = iTruce;
    *piNumTrade = iTrade;
    *piNumAlliance = iAlliance;

Cleanup:

    SafeRelease (pGameEmpireDiplomacy);

    if (piData != NULL) {
        m_pConn->FreeData(piData);
    }

    if (piKey != NULL) {
        m_pConn->FreeKeys(piKey);
    }

    return iErrCode;
}


int GameEngine::GetLastUsedMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int* piLastUsedMask,
                                          int** ppiLastUsedProxyKeyArray, int* piNumLastUsed) {

    int iErrCode, iLastUsedMask;
    Variant vTemp;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    *piLastUsedMask = MESSAGE_TARGET_NONE;
    *ppiLastUsedProxyKeyArray = NULL;
    *piNumLastUsed = 0;

    // Read mask
    iErrCode = m_pConn->ReadData (strGameEmpireData, GameEmpireData::LastMessageTargetMask, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    *piLastUsedMask = iLastUsedMask = vTemp.GetInteger();

    // Leave early if we didn't cc anyone
    if (!(iLastUsedMask & MESSAGE_TARGET_INDIVIDUALS)) {
        return OK;
    }

    // Read last used array
    iErrCode = m_pConn->GetEqualKeys (
        strGameEmpireDiplomacy,
        GameEmpireDiplomacy::LastMessageTargetFlag,
        1,
        (unsigned int**) ppiLastUsedProxyKeyArray,
        (unsigned int*) piNumLastUsed
        );

    if (iErrCode != OK) {

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        } else {
            Assert (false);
        }
    }

    return iErrCode;
}


int GameEngine::SetLastUsedMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int iLastUsedMask,
                                          int* piLastUsedKeyArray, int iNumLastUsed) {

    int iErrCode, i;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, iGameClass, iGameNumber, iEmpireKey);

    // Mask
    iErrCode = m_pConn->WriteData (
        strGameEmpireData,
        GameEmpireData::LastMessageTargetMask,
        iLastUsedMask
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Leave early if we didn't cc anyone
    if (!(iLastUsedMask & MESSAGE_TARGET_INDIVIDUALS)) {
        Assert (piLastUsedKeyArray == NULL && iNumLastUsed == 0);
        return OK;
    }

    // Array - first set all values to 0
    iErrCode = m_pConn->WriteColumn (
        strGameEmpireDiplomacy,
        GameEmpireDiplomacy::LastMessageTargetFlag,
        0
        );

    if (iErrCode != OK) {

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        } else {
            Assert (false);
            return iErrCode;
        }
    }

    // Now, set the provided keys
    unsigned int iKey;

    for (i = 0; i < iNumLastUsed; i ++) {

        iErrCode = m_pConn->GetFirstKey (
            strGameEmpireDiplomacy,
            GameEmpireDiplomacy::EmpireKey,
            piLastUsedKeyArray[i],
            &iKey
            );

        if (iErrCode == OK) {

            Assert (iKey != NO_KEY);

            iErrCode = m_pConn->WriteData (
                strGameEmpireDiplomacy,
                iKey,
                GameEmpireDiplomacy::LastMessageTargetFlag,
                1
                );

            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }
        }
        
        else if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        
        else {
            
            Assert (false);
            break;
        }
    }

    return iErrCode;
}