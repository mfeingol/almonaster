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
                                                  unsigned int* piMaxNumSavedMessages) {

    int iErrCode = OK;

    GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    Variant vTemp;
    iErrCode = m_pGameData->ReadData (strEmpireOptions, GameEmpireData::MaxNumGameMessages, &vTemp);

    if (iErrCode == OK) {
        *piMaxNumSavedMessages = vTemp.GetInteger();
    }

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
                                                  unsigned int iMaxNumSavedMessages) {
    
    int iErrCode;

    unsigned int* piKey = NULL, iNumMessages, iMaxNum;
    UTCTime* ptTimeStamp = NULL;
   
    Variant vTemp;

    GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    GAME_EMPIRE_DATA (strOptions, iGameClass, iGameNumber, iEmpireKey);

    IWriteTable* pMessages = NULL;

    iErrCode = m_pGameData->ReadData (strOptions, GameEmpireData::MaxNumGameMessages, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    iMaxNum = vTemp.GetInteger();

    // Set the max number of messages
    iErrCode = m_pGameData->WriteData (strOptions, GameEmpireData::MaxNumGameMessages, iMaxNumSavedMessages);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iMaxNumSavedMessages >= iMaxNum) {
        goto Cleanup;
    }

    // Lock message table
    iErrCode = m_pGameData->GetTableForWriting (strGameEmpireMessages, &pMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Get current and max number of messages
    iErrCode = pMessages->GetNumRows (&iNumMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // If we're going to be over the limit, trim the list of unread messages
    if (iMaxNum > iMaxNumSavedMessages && iNumMessages > iMaxNumSavedMessages) {
        
        int iUnread;
        unsigned int iReadNumMessages, i, iCurrentNumMessages;

        // Get the oldest messages' keys
        iErrCode = pMessages->ReadColumn (GameEmpireMessages::TimeStamp, &piKey, &ptTimeStamp, &iReadNumMessages);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        Assert (iReadNumMessages == iNumMessages);

        // Sort the messages by age
        Algorithm::QSortTwoAscending<UTCTime, unsigned int> (ptTimeStamp, piKey, iReadNumMessages);

        // Delete read messages until we're below the limit
        iCurrentNumMessages = iReadNumMessages;

        for (i = 0; i < iReadNumMessages && iCurrentNumMessages > iMaxNumSavedMessages; i ++) {

            // Has message been read?
            iErrCode = pMessages->ReadData (piKey[i], GameEmpireMessages::Unread, &iUnread);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (iUnread == MESSAGE_UNREAD) {
                continue;
            }

            iErrCode = pMessages->DeleteRow (piKey[i]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iCurrentNumMessages --;
        }
    }

Cleanup:

    // Unlock the messages table
    SafeRelease (pMessages);

    if (piKey != NULL) {
        m_pGameData->FreeKeys (piKey);
    }

    if (ptTimeStamp != NULL) {
        m_pGameData->FreeData (ptTimeStamp);
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

    int iErrCode, iGameState = 0, iOptions;
    unsigned int iNumEmpires;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    Variant vTemp, vOldNum;

#ifdef _DEBUG
    // Only call this function holding an exclusive lock
    GameObject* pGameObj = GetGameObject (iGameClass, iGameNumber);
    Assert (pGameObj != NULL && pGameObj->HeldExclusive (NULL));
    SafeRelease (pGameObj);
#endif

    iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iGameState = vTemp.GetInteger();

    if (!(iGameState & STARTED)) {
        goto Cleanup;
    }

    iErrCode = m_pGameData->GetNumRows (strEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strGameEmpireData, GameEmpireData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iOptions = vTemp.GetInteger();

    if (iOptions & REQUEST_PAUSE) {
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
    
    if ((unsigned int) vOldNum.GetInteger() + 1 == iNumEmpires) {

        if (!(iGameState & ADMIN_PAUSED)) {

            iErrCode = PauseGame (iGameClass, iGameNumber, false, bBroadcast);
            Assert (iErrCode == OK);

            if (iErrCode == OK) {
                *piGameState |= PAUSED;
            }
        }
    }

Cleanup:

    *piGameState = iGameState;

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

#ifdef _DEBUG
    // Only call this function holding an exclusive lock
    GameObject* pGameObj = GetGameObject (iGameClass, iGameNumber);
    Assert (pGameObj != NULL && pGameObj->HeldExclusive (NULL));
    SafeRelease (pGameObj);
#endif

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

int GameEngine::GetNumEmpiresRequestingPause (int iGameClass, int iGameNumber, unsigned int* piNumEmpires) {

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
    unsigned int iNumEmpires;

    GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GAME_DATA (strGameData, iGameClass, iGameNumber);
    GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    Variant vTemp, vOldNum;
    int iOptions, iState = 0;

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

    if (iOptions & REQUEST_DRAW) {
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

    iErrCode = m_pGameData->GetNumRows (strEmpires, &iNumEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    
    if ((unsigned int) vOldNum.GetInteger() + 1 >= iNumEmpires) {
        *piGameState |= GAME_ENDED;
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
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->Increment (strGameData, GameData::NumRequestingDraw, -1);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
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

int GameEngine::GetNumEmpiresRequestingDraw (int iGameClass, int iGameNumber, unsigned int* piNumEmpires) {

    GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vNum;
    int iErrCode = m_pGameData->ReadData (strGameData, GameData::NumRequestingDraw, &vNum);

    if (iErrCode == OK) {
        *piNumEmpires = vNum.GetInteger();
    }

    return iErrCode;
}