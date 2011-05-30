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

#include "Osal/Algorithm.h"

#include "GameEngine.h"

//
// Generic work
//

int GameEngine::GetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, 
                                     bool* pbOption) {

    GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    Variant vOptions;
    int iErrCode = m_pGameData->ReadData (strEmpireOptions, GameEmpireData::Options, &vOptions);

    if (iErrCode == OK) {
        *pbOption = (vOptions.GetInteger() & iFlag) != 0;
    }

    return iErrCode;
}

int GameEngine::SetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, 
                                     bool bOption) {

    GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    if (bOption) {
        return m_pGameData->WriteOr (strEmpireOptions, GameEmpireData::Options, iFlag);
    } else {
        return m_pGameData->WriteAnd (strEmpireOptions, GameEmpireData::Options, ~iFlag);
    }
}

int GameEngine::GetEmpireOptions (int iGameClass, int iGameNumber, int iEmpireKey, int* piOptions) {

    GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    Variant vOptions;
    int iErrCode = m_pGameData->ReadData (strEmpireOptions, GameEmpireData::Options, &vOptions);

    if (iErrCode == OK) {
        *piOptions = vOptions.GetInteger();
    }

    return iErrCode;
}


// Input:
// iGameClass -> GameClass key
// iGameNumber -> GameNumber
// iEmpireKey -> Empire Key
//
// Output:
// *piMaxNumSavedMessages -> Maximum number of saved messages for the empire in the game

int GameEngine::GetEmpireMaxNumSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, 
                                                  int* piMaxNumSavedMessages) {

    int iErrCode = OK;

    GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    Variant vMaxNumSavedMessages = 0;
    iErrCode = m_pGameData->ReadData (strEmpireOptions, GameEmpireData::MaxNumGameMessages, &vMaxNumSavedMessages);

    if (iErrCode == OK) {
        *piMaxNumSavedMessages = vMaxNumSavedMessages;
    }

    return iErrCode;
}


// Input:
// iGameClass -> GameClass key
// iGameNumber -> GameNumber
// iEmpireKey -> Empire Key
// bAutoUpdate -> true / false
//
// Set the empire's AutoUpdate parameter in the given game

int GameEngine::SetEmpireAutoUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool bAutoUpdate) {

    int iErrCode = OK;
    Variant vOptions, vTemp;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    // Return if same setting
    iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        iErrCode = ERROR_EMPIRE_IS_NOT_IN_GAME;
        goto Cleanup;
    }

    if (((vOptions.GetInteger() & AUTO_UPDATE) != 0) == bAutoUpdate) {
        goto Cleanup;
    }

    // Write the new setting
    if (bAutoUpdate) {

        iErrCode = m_pGameData->WriteOr (strGameEmpireData, GameEmpireData::Options, AUTO_UPDATE);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // See if we're ready for an update right now       
        // If not, write that we are and increase the number of updated empires
        if (!(vOptions.GetInteger() & UPDATED)) {
            
            iErrCode = m_pGameData->WriteOr (strGameEmpireData, GameEmpireData::Options, UPDATED);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            // Increment number of updated empires
            iErrCode = m_pGameData->Increment (strGameData, GameData::NumEmpiresUpdated, 1, &vTemp);
            if (iErrCode != OK) {

                Assert (false);
                int iErrCode2 = m_pGameData->WriteAnd (strGameEmpireData, GameEmpireData::Options, ~UPDATED);
                Assert (iErrCode2 == OK);
                goto Cleanup;
            }

#ifdef _DEBUG
            iErrCode = VerifyUpdatedEmpireCount (iGameClass, iGameNumber);
            Assert (iErrCode == OK);
#endif
        }

    } else {

        iErrCode = m_pGameData->WriteAnd (strGameEmpireData, GameEmpireData::Options, ~AUTO_UPDATE);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // See if we're ready for an update right now
        // If we are, write that we aren't and decrease the number of updated empires
        if (vOptions.GetInteger() & UPDATED) {

            iErrCode = m_pGameData->WriteAnd (strGameEmpireData, GameEmpireData::Options, ~UPDATED);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            // Decrement number of updated empires
            iErrCode = m_pGameData->Increment (strGameData, GameData::NumEmpiresUpdated, -1, &vTemp);
            if (iErrCode != OK) {

                Assert (false);
                int iErrCode2 = m_pGameData->WriteOr (strGameEmpireData, GameEmpireData::Options, UPDATED);
                Assert (iErrCode2 == OK);
                goto Cleanup;
            }

#ifdef _DEBUG
            iErrCode = VerifyUpdatedEmpireCount (iGameClass, iGameNumber);
            Assert (iErrCode == OK);
            iErrCode = OK;
#endif
        }
    }

Cleanup:

    return iErrCode;
}



// Input:
// iGameClass -> GameClass key
// iGameNumber -> GameNumber
// iEmpireKey -> Empire Key
// iMaxNumSavedMessages -> Number of messages to be saved
//
// Set the empire's MaxNumSavedMessages parameter in the given game

int GameEngine::SetEmpireMaxNumSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, 
                                                  int iMaxNumSavedMessages) {
    
    Variant vMaxNum;
    int iErrCode, iNumMessages;

    unsigned int* piKey = NULL;
    Variant* pvTimeStamp = NULL;
    
    GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA (strOptions, iGameClass, iGameNumber, iEmpireKey);

    // Lock message table
    NamedMutex nmMutex;
    LockEmpireGameMessages (iGameClass, iGameNumber, iEmpireKey, &nmMutex);
    
    // Get current and max number of messages
    iErrCode = m_pGameData->GetNumRows (strGameEmpireMessages, (unsigned int*) &iNumMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strOptions, GameEmpireData::MaxNumGameMessages, &vMaxNum);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // If we're going to be over the limit, trim the list of unread messages
    if (vMaxNum.GetInteger() > iMaxNumSavedMessages && 
        iNumMessages > iMaxNumSavedMessages) {
        
        // Get the oldest messages' keys
        int iErrCode2;
        
        iErrCode2 = m_pGameData->ReadColumn (
            strGameEmpireMessages,
            GameEmpireMessages::TimeStamp,
            &piKey,
            &pvTimeStamp,
            (unsigned int*) &iNumMessages
            );

        if (iErrCode2 == OK) {

            // Sort the messages by age
            Algorithm::QSortTwoAscending<Variant, unsigned int> (pvTimeStamp, piKey, iNumMessages);

            // Delete read messages until we're below the limit
            int i = 0;
            Variant vUnread;

            while (iNumMessages > iMaxNumSavedMessages && i < iNumMessages) {
                
                // Has message been read
                iErrCode2 = m_pGameData->ReadData (
                    strGameEmpireMessages, 
                    piKey[i], 
                    GameEmpireMessages::Unread, 
                    &vUnread
                    );

                if (iErrCode2 != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                
                if (vUnread.GetInteger() == MESSAGE_READ) {

                    iErrCode = m_pGameData->DeleteRow (strGameEmpireMessages, piKey[i]);
                    if (iErrCode2 != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                    iNumMessages --;
                } else {
                    break;
                }

                i ++;
            }
        }

        else Assert (iErrCode2 == ERROR_DATA_NOT_FOUND);
    }
    
    // Set the max number of messages
    iErrCode = m_pGameData->WriteData (strOptions, GameEmpireData::MaxNumGameMessages, iMaxNumSavedMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    // Unlock the messages table
    UnlockEmpireGameMessages (nmMutex);

    if (piKey != NULL) {
        m_pGameData->FreeKeys (piKey);
    }

    if (pvTimeStamp != NULL) {
        m_pGameData->FreeData (pvTimeStamp);
    }
    
    return iErrCode;
}


// Input:
// iGameClass -> GameClass
// iGameNumber -> GameNumber
// iEmpireKey -> Empire key
// iIgnoredEmpire -> Ignored empire key
//
// Output:
// *pbIgnore -> True if empire wants to ignore the empire, false if not
//
// Get if the empire wants an ignore messages from an empire in a game

int GameEngine::GetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, 
                                         bool* pbIgnore) {

    // Trivial case
    if (iEmpireKey == iIgnoredEmpire) {
        *pbIgnore = false;
        return OK;
    }

    unsigned int iKey;
    GAME_EMPIRE_DIPLOMACY (pszDip, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetFirstKey (
        pszDip, 
        GameEmpireDiplomacy::EmpireKey, 
        iIgnoredEmpire,
        false,
        &iKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        *pbIgnore = false;
        return ERROR_EMPIRE_IS_NOT_IN_DIPLOMACY;
    }

    if (iErrCode != OK) {
        return iErrCode;
    }

    Variant vIgnore;
    iErrCode = m_pGameData->ReadData (
        pszDip, 
        iKey,
        GameEmpireDiplomacy::State, 
        &vIgnore
        );

    if (iErrCode == OK) {
        *pbIgnore = (vIgnore.GetInteger() & IGNORE_EMPIRE) != 0;
    }

    return iErrCode;
}


// Input:
// iGameClass -> GameClass
// iGameNumber -> GameNumber
// iEmpireKey -> Empire key
// iIgnoredEmpire -> Ignored empire key
// bIgnore -> True if empire wants to ignore the empire, false if not
//
// Set if the empire wants an ignore messages from an empire in a game

int GameEngine::SetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, 
                                         bool bIgnore) {

    unsigned int iKey;
    GAME_EMPIRE_DIPLOMACY (pszDip, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = m_pGameData->GetFirstKey (
        pszDip, 
        GameEmpireDiplomacy::EmpireKey, 
        iIgnoredEmpire,
        false,
        &iKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return ERROR_EMPIRE_IS_NOT_IN_DIPLOMACY;
    }

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (bIgnore) {

        iErrCode = m_pGameData->WriteOr (
            pszDip, 
            iKey,
            GameEmpireDiplomacy::State, 
            IGNORE_EMPIRE
            );

    } else {

        iErrCode = m_pGameData->WriteAnd (
            pszDip, 
            iKey,
            GameEmpireDiplomacy::State, 
            ~IGNORE_EMPIRE
            );
    }

    return iErrCode;
}


// Input:
// iGameClass -> GameClass
// iGameNumber -> GameNumber
// iEmpireKey -> Empire key
//
// Output:
// *pvNotepad -> Empire's notes in a particular game
//
// Return an empire's notes in a particular game

int GameEngine::GetEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, Variant* pvNotepad) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Notepad, pvNotepad);
}


int GameEngine::GetEmpireDefaultMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, 
                                               int* piMessageTarget) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    Variant vData;
    int iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::DefaultMessageTarget, &vData);

    if (iErrCode == OK) {
        *piMessageTarget = vData.GetInteger();
    }

    return iErrCode;
}


int GameEngine::SetEmpireDefaultMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, 
                                               int iMessageTarget) {

    if (iMessageTarget < MESSAGE_TARGET_NONE || iMessageTarget > MESSAGE_TARGET_LAST_USED) {
        return ERROR_INVALID_ARGUMENT;
    }

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    return m_pGameData->WriteData (strGameEmpireData, GameEmpireData::DefaultMessageTarget, iMessageTarget);
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *piGameState -> State of game after request
//
// Request game pause

int GameEngine::RequestPause (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState) {
    return RequestPauseInternal (iGameClass, iGameNumber, iEmpireKey, piGameState, true);
}

int GameEngine::RequestPauseQuietly (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState) {
    return RequestPauseInternal (iGameClass, iGameNumber, iEmpireKey, piGameState, false);
}

int GameEngine::RequestPauseInternal (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState,
                                      bool bBroadcast) {

    int iErrCode;
    unsigned int iNumRows;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    Variant vOptions, vState, vOldNum;

    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    *piGameState = vState.GetInteger();

    if (!(vState.GetInteger() & STARTED)) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vOptions.GetInteger() & REQUEST_PAUSE) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteOr (strGameEmpireData, GameEmpireData::Options, REQUEST_PAUSE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->Increment (strGameData, GameData::NumRequestingPause, 1, &vOldNum);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->GetNumRows (strEmpires, &iNumRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if ((unsigned int) vOldNum.GetInteger() + 1 == iNumRows) {

        if (!(vState.GetInteger() & ADMIN_PAUSED)) {

            iErrCode = PauseGame (iGameClass, iGameNumber, false, bBroadcast);
            Assert (iErrCode == OK);

            if (iErrCode == OK) {
                *piGameState |= PAUSED;
            }
        }
    }

Cleanup:

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *piGameState -> State of game after request
//
// Request end of game pause

int GameEngine::RequestNoPause (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState) {

    int iErrCode, iState = 0, iOptions;
    Variant vTemp;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iState = vTemp.GetInteger();

    if (!(iState & STARTED)) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iOptions = vTemp.GetInteger();

    if (!(iOptions & REQUEST_PAUSE)) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteAnd (strGameEmpireData, GameEmpireData::Options, ~REQUEST_PAUSE);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->Increment (strGameData, GameData::NumRequestingPause, -1);
    if (iErrCode != OK) {
        // TODO - need compensation
        Assert (false);
        goto Cleanup;
    }

    if (!(iState & ADMIN_PAUSED) && (iState & PAUSED)) {

        iErrCode = UnpauseGame (iGameClass, iGameNumber, false, true);
        Assert (iErrCode == OK);

        if (iErrCode == OK) {
            iState &= ~PAUSED;
        }
    }

Cleanup:

    *piGameState = iState;
    
    return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *pbPause -> True if empire is requesting pause, false otherwise
//
// Return if the empire is requesting a pause in a given game

int GameEngine::IsEmpireRequestingPause (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbPause) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    
    Variant vPaused;
    int iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vPaused);
    
    if (iErrCode == OK) {
        *pbPause = (vPaused.GetInteger() & REQUEST_PAUSE) != 0;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *piNumEmpires -> Number of empires requesting pause in a given game
//
// Return the number of empires requesting pause in a given game

int GameEngine::GetNumEmpiresRequestingPause (int iGameClass, int iGameNumber, int* piNumEmpires) {

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vNumPaused;
    int iErrCode = m_pGameData->ReadData (strGameData, GameData::NumRequestingPause, &vNumPaused);

    if (iErrCode == OK) {
        *piNumEmpires = vNumPaused.GetInteger();
    }

    return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *piGameState -> State of game after request
//
// Request game draw

int GameEngine::RequestDraw (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState) {

    int iErrCode;
    unsigned int iNumRows;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    Variant vOptions, vState, vOldNum;

    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    *piGameState = vState.GetInteger();

    if (!(vState.GetInteger() & STARTED)) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (vOptions.GetInteger() & REQUEST_DRAW) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteOr (strGameEmpireData, GameEmpireData::Options, REQUEST_DRAW);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->Increment (strGameData, GameData::NumRequestingDraw, 1, &vOldNum);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->GetNumRows (strEmpires, &iNumRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if ((unsigned int) vOldNum.GetInteger() + 1 >= iNumRows) {
        *piGameState |= GAME_ENDED;
    }

Cleanup:

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Request no game draw

int GameEngine::RequestNoDraw (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vState, vOptions;
    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vState);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(vState.GetInteger() & STARTED)) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(vOptions.GetInteger() & REQUEST_DRAW)) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->WriteAnd (strGameEmpireData, GameEmpireData::Options, ~REQUEST_DRAW);
    Assert (iErrCode == OK);

    if (iErrCode == OK) {
        iErrCode = m_pGameData->Increment (strGameData, GameData::NumRequestingDraw, -1);
        Assert (iErrCode == OK);
    }

Cleanup:
    
    return iErrCode;
}

// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
// iEmpireKey -> Integer key of empire
//
// Output:
// *pbDraw -> True if empire is requesting pause, false otherwise
//
// Return if the empire is requesting a draw in a given game

int GameEngine::IsEmpireRequestingDraw (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbDraw) {

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    
    Variant vPaused;
    int iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vPaused);
    
    if (iErrCode == OK) {
        *pbDraw = (vPaused.GetInteger() & REQUEST_DRAW) != 0;
    }

    return iErrCode;
}


// Input:
// iGameClass -> Gameclass
// iGameNumber -> Gamenumber
//
// Output:
// *piNumEmpires -> Number of empires requesting draw in a given game
//
// Return the number of empires requesting draw in a given game

int GameEngine::GetNumEmpiresRequestingDraw (int iGameClass, int iGameNumber, int* piNumEmpires) {

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vNum;
    int iErrCode = m_pGameData->ReadData (strGameData, GameData::NumRequestingDraw, &vNum);

    if (iErrCode == OK) {
        *piNumEmpires = vNum.GetInteger();
    }

    return iErrCode;
}