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

#include "Osal/Algorithm.h"

#include "GameEngine.h"

//
// Generic work
//

int GameEngine::GetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, 
                                     bool* pbOption) {

    GET_GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    Variant vOptions;
    int iErrCode = t_pCache->ReadData(strEmpireOptions, GameEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    *pbOption = (vOptions.GetInteger() & iFlag) != 0;
    
    return iErrCode;
}

int GameEngine::SetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, 
                                     bool bOption) {

    GET_GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    if (bOption) {
        return t_pCache->WriteOr(strEmpireOptions, GameEmpireData::Options, iFlag);
    } else {
        return t_pCache->WriteAnd(strEmpireOptions, GameEmpireData::Options, ~iFlag);
    }
}

int GameEngine::GetEmpireOptions (int iGameClass, int iGameNumber, int iEmpireKey, int* piOptions) {

    GET_GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    Variant vOptions;
    int iErrCode = t_pCache->ReadData(strEmpireOptions, GameEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    *piOptions = vOptions.GetInteger();
    
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

    GET_GAME_EMPIRE_DATA (strEmpireOptions, iGameClass, iGameNumber, iEmpireKey);

    Variant vTemp;
    iErrCode = t_pCache->ReadData(strEmpireOptions, GameEmpireData::MaxNumGameMessages, &vTemp);
    RETURN_ON_ERROR(iErrCode);

    *piMaxNumSavedMessages = vTemp.GetInteger();
    
    return iErrCode;
}


// Input:
// iGameClass -> GameClass key
// iGameNumber -> GameNumber
// iEmpireKey -> Empire Key
// iMaxNumSavedMessages -> Number of messages to be saved
//
// Set the empire's MaxNumSavedMessages parameter in the given game

int GameEngine::SetEmpireMaxNumSavedGameMessages(int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iMaxNumSavedMessages)
{
    int iErrCode;
   
    GET_GAME_EMPIRE_MESSAGES (strGameEmpireMessages, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_EMPIRE_DATA (strOptions, iGameClass, iGameNumber, iEmpireKey);

    Variant vTemp;
    iErrCode = t_pCache->ReadData(strOptions, GameEmpireData::MaxNumGameMessages, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    unsigned int iOldMaxNum = vTemp.GetInteger();

    if (iOldMaxNum != iMaxNumSavedMessages)
    {
        // Set the new max number of messages
        iErrCode = t_pCache->WriteData(strOptions, GameEmpireData::MaxNumGameMessages, (int)iMaxNumSavedMessages);
        RETURN_ON_ERROR(iErrCode);

        if (iMaxNumSavedMessages < iOldMaxNum)
        {
            ICachedTable* pMessages = NULL;
            AutoRelease<ICachedTable> rel(pMessages);

            iErrCode = t_pCache->GetTable(strGameEmpireMessages, &pMessages);
            RETURN_ON_ERROR(iErrCode);

            unsigned int iNumMessages;
            iErrCode = pMessages->GetNumCachedRows(&iNumMessages);
            RETURN_ON_ERROR(iErrCode);

            unsigned int iNumUnreadMessages;
            iErrCode = GetNumUnreadSystemMessagesPrivate(pMessages, &iNumUnreadMessages);
            RETURN_ON_ERROR(iErrCode);
            Assert(iNumMessages >= iNumUnreadMessages);

            iErrCode = DeleteOverflowMessages(pMessages, GameEmpireMessages::TimeStamp, GameEmpireMessages::Unread, iNumMessages, iNumUnreadMessages, iMaxNumSavedMessages, true);
            RETURN_ON_ERROR(iErrCode);
        }
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

int GameEngine::GetEmpireIgnoreMessages(int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, bool* pbIgnore)
{
    *pbIgnore = false;

    // Trivial case
    if (iEmpireKey == iIgnoredEmpire)
    {
        return OK;
    }

    unsigned int iKey;
    GET_GAME_EMPIRE_DIPLOMACY (pszDip, iGameClass, iGameNumber, iEmpireKey);
    int iErrCode = t_pCache->GetFirstKey(
        pszDip, 
        GameEmpireDiplomacy::ReferenceEmpireKey, 
        iIgnoredEmpire,
        &iKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return ERROR_EMPIRE_IS_NOT_IN_DIPLOMACY;
    }
    RETURN_ON_ERROR(iErrCode);

    Variant vIgnore;
    iErrCode = t_pCache->ReadData(pszDip, iKey, GameEmpireDiplomacy::State, &vIgnore);
    RETURN_ON_ERROR(iErrCode);

    *pbIgnore = (vIgnore.GetInteger() & IGNORE_EMPIRE) != 0;
    
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

int GameEngine::SetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, bool bIgnore)
{
    unsigned int iKey;
    GET_GAME_EMPIRE_DIPLOMACY (pszDip, iGameClass, iGameNumber, iEmpireKey);

    int iErrCode = t_pCache->GetFirstKey(
        pszDip, 
        GameEmpireDiplomacy::ReferenceEmpireKey, 
        iIgnoredEmpire,
        &iKey
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return ERROR_EMPIRE_IS_NOT_IN_DIPLOMACY;
    }
    RETURN_ON_ERROR(iErrCode);

    if (bIgnore)
    {
        iErrCode = t_pCache->WriteOr(pszDip, iKey, GameEmpireDiplomacy::State, IGNORE_EMPIRE);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = t_pCache->WriteAnd(pszDip, iKey, GameEmpireDiplomacy::State, ~IGNORE_EMPIRE);
        RETURN_ON_ERROR(iErrCode);
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

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    return t_pCache->ReadData(strGameEmpireData, GameEmpireData::Notepad, pvNotepad);
}


int GameEngine::GetEmpireDefaultMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int* piMessageTarget)
{
    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);

    Variant vData;
    int iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::DefaultMessageTarget, &vData);
    RETURN_ON_ERROR(iErrCode);

    *piMessageTarget = vData.GetInteger();

    return iErrCode;
}


int GameEngine::SetEmpireDefaultMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int iMessageTarget)
{
    if (iMessageTarget < MESSAGE_TARGET_NONE || iMessageTarget > MESSAGE_TARGET_LAST_USED)
    {
        return ERROR_INVALID_ARGUMENT;
    }

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    return t_pCache->WriteData(strGameEmpireData, GameEmpireData::DefaultMessageTarget, iMessageTarget);
}

int GameEngine::RequestPauseDuringUpdate (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

#ifdef _DEBUG
    Variant vTemp;
    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    Assert(!(vTemp.GetInteger() & REQUEST_PAUSE));
#endif

    iErrCode = t_pCache->WriteOr(strGameEmpireData, GameEmpireData::Options, REQUEST_PAUSE);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(strGameData, GameData::NumRequestingPause, 1);
    RETURN_ON_ERROR(iErrCode);

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
// Request game pause

int GameEngine::RequestPause (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState)
{
    int iErrCode, iGameState = 0, iOptions;
    unsigned int iNumEmpires;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    Variant vTemp, vOldNum;
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameState = vTemp.GetInteger();

    if (!(iGameState & STARTED))
    {
        return OK;
    }

    iErrCode = t_pCache->GetNumCachedRows(strEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iOptions = vTemp.GetInteger();

    if (iOptions & REQUEST_PAUSE)
    {
        return OK;
    }

    iErrCode = t_pCache->WriteOr(strGameEmpireData, GameEmpireData::Options, REQUEST_PAUSE);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(strGameData, GameData::NumRequestingPause, 1, &vOldNum);
    RETURN_ON_ERROR(iErrCode);
    
    if ((unsigned int)vOldNum.GetInteger() + 1 == iNumEmpires)
    {
        if (!(iGameState & ADMIN_PAUSED))
        {
            // Only pause the game if not all empires are idle
            bool bIdle;
            iErrCode = AreAllEmpiresIdle (iGameClass, iGameNumber, &bIdle);
            RETURN_ON_ERROR(iErrCode);

            if (!bIdle)
            {
                iErrCode = PauseGame(iGameClass, iGameNumber, false, true);
                RETURN_ON_ERROR(iErrCode);

                iGameState |= PAUSED;
            }
        }
    }

    *piGameState = iGameState;

    return iErrCode;
}

int GameEngine::CheckForDelayedPause (int iGameClass, int iGameNumber, const UTCTime& tNow, bool* pbNewlyPaused)
{
    int iErrCode;

    *pbNewlyPaused = false;

    unsigned int iNumEmpires;
    iErrCode = GetNumEmpiresInGame (iGameClass, iGameNumber, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    unsigned int iNumPaused;
    iErrCode = GetNumEmpiresRequestingPause (iGameClass, iGameNumber, &iNumPaused);
    RETURN_ON_ERROR(iErrCode);

    if (iNumPaused == iNumEmpires) {

        int iGameState;
        iErrCode = GetGameState (iGameClass, iGameNumber, &iGameState);
        RETURN_ON_ERROR(iErrCode);
        
        if (!(iGameState & PAUSED)) {

            bool bIdle;
            iErrCode = AreAllEmpiresIdle (iGameClass, iGameNumber, &bIdle);
            RETURN_ON_ERROR(iErrCode);

            if (!bIdle) {

                iErrCode = PauseGameAt (iGameClass, iGameNumber, tNow);
                RETURN_ON_ERROR(iErrCode);

                *pbNewlyPaused = true;
            }
        }
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
// Request end of game pause

int GameEngine::RequestNoPause (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState) {

    int iErrCode, iGameState = 0, iOptions;
    Variant vTemp;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameState = vTemp.GetInteger();

    if (!(iGameState & STARTED))
    {
        return OK;
    }

    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iOptions = vTemp.GetInteger();

    if (!(iOptions & REQUEST_PAUSE))
    {
        return OK;
    }

    iErrCode = t_pCache->WriteAnd(strGameEmpireData, GameEmpireData::Options, ~REQUEST_PAUSE);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(strGameData, GameData::NumRequestingPause, -1);
    RETURN_ON_ERROR(iErrCode);

    if (!(iGameState & ADMIN_PAUSED) && (iGameState & PAUSED))
    {
        iErrCode = UnpauseGame (iGameClass, iGameNumber, false, true);
        RETURN_ON_ERROR(iErrCode);

        iGameState &= ~PAUSED;
    }

    *piGameState = iGameState;
    
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

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    
    Variant vPaused;
    int iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vPaused);
    RETURN_ON_ERROR(iErrCode);

    *pbPause = (vPaused.GetInteger() & REQUEST_PAUSE) != 0;
    
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

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vNumPaused;
    int iErrCode = t_pCache->ReadData(strGameData, GameData::NumRequestingPause, &vNumPaused);
    RETURN_ON_ERROR(iErrCode);

    *piNumEmpires = vNumPaused.GetInteger();
    
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

int GameEngine::RequestDraw(int iGameClass, int iGameNumber, int iEmpireKey, bool* pbDrawnGame)
{
    int iErrCode;
    unsigned int iNumEmpires;

    *pbDrawnGame = false;

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);
    GET_GAME_EMPIRES (strEmpires, iGameClass, iGameNumber);

    Variant vTemp, vOldNum;
    int iOptions, iGameState = 0;

    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iGameState = vTemp.GetInteger();

    if (!(iGameState & STARTED))
    {
        return OK;
    }

    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iOptions = vTemp.GetInteger();

    if (iOptions & REQUEST_DRAW)
    {
        return OK;
    }

    iErrCode = t_pCache->WriteOr(strGameEmpireData, GameEmpireData::Options, REQUEST_DRAW);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(strGameData, GameData::NumRequestingDraw, 1, &vOldNum);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->GetNumCachedRows(strEmpires, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);
    
    if ((unsigned int)vOldNum.GetInteger() + 1 >= iNumEmpires)
    {
        *pbDrawnGame = true;
    }

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

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vState, vOptions;
    iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vState);
    RETURN_ON_ERROR(iErrCode);

    if (!(vState.GetInteger() & STARTED))
    {
        return OK;
    }

    iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    if (!(vOptions.GetInteger() & REQUEST_DRAW))
    {
        return OK;
    }

    iErrCode = t_pCache->WriteAnd(strGameEmpireData, GameEmpireData::Options, ~REQUEST_DRAW);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(strGameData, GameData::NumRequestingDraw, -1);
    RETURN_ON_ERROR(iErrCode);
   
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

    GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
    
    Variant vPaused;
    int iErrCode = t_pCache->ReadData(strGameEmpireData, GameEmpireData::Options, &vPaused);
    RETURN_ON_ERROR(iErrCode);

    *pbDraw = (vPaused.GetInteger() & REQUEST_DRAW) != 0;
    
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

    GET_GAME_DATA (strGameData, iGameClass, iGameNumber);

    Variant vNum;
    int iErrCode = t_pCache->ReadData(strGameData, GameData::NumRequestingDraw, &vNum);
    RETURN_ON_ERROR(iErrCode);

    *piNumEmpires = vNum.GetInteger();
    
    return iErrCode;
}