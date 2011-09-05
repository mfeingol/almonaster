<%
#include <stdio.h>
#include "Osal/Algorithm.h"

// Almonaster
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

if (InitializeEmpireInGame(false) != OK)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
if (InitializeGame(&pageRedirect) != OK)
{
    return Redirect(pageRedirect);
}

IHttpForm* pHttpForm;

// Handle a submission
int iErrCode, iOptionPage = 0, iGameClassOptions, iDiplomacy;
unsigned int i, iNumEmpires;

Variant vTemp;

bool bGameStarted = (m_iGameState & STARTED) != 0;

GameCheck (GetGameClassOptions (m_iGameClass, &iGameClassOptions));
GameCheck (GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires));
GameCheck (GetGameClassDiplomacyLevel (m_iGameClass, &iDiplomacy));

if (m_bOwnPost && !m_bRedirection) {

    // Handle submissions
    if ((pHttpForm = m_pHttpRequest->GetForm ("OptionPage")) == NULL) {
        goto Redirection;
    }
    int iOldValue, iNewValue, iOptionPageSubmit = pHttpForm->GetIntValue();

    switch (iOptionPageSubmit) {

    case 0:

        // Make sure cancel wasn't pressed
        if (WasButtonPressed (BID_CANCEL)) {
            m_bRedirectTest = false;
        } else {

            bool bFlag, bUpdate;

            // Autorefesh
            if ((pHttpForm = m_pHttpRequest->GetForm ("AutoRefresh")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldAutoRefresh")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {

                bUpdate = iNewValue != 0;

                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, AUTO_REFRESH, bUpdate) == OK) {

                    if (bUpdate) {
                        m_iGameOptions |= AUTO_REFRESH;
                        AddMessage ("Refresh on update countdown is now enabled");
                    } else {
                        m_iGameOptions &= ~AUTO_REFRESH;
                        AddMessage ("Refresh on update countdown is now disabled");
                    }
                } else {
                    AddMessage ("Your autorefresh setting could not be updated");
                }
            }

            // RepeatedButtons
            if ((pHttpForm = m_pHttpRequest->GetForm ("RepeatButtons")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldRepeatButtons")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {

                bUpdate = iNewValue != 0;
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, GAME_REPEATED_BUTTONS, bUpdate) == OK) {

                    m_bRepeatedButtons = bUpdate;

                    if (bUpdate) {
                        m_iGameOptions |= GAME_REPEATED_BUTTONS;
                        AddMessage ("Your command buttons are now repeated");
                    } else {
                        m_iGameOptions &= ~GAME_REPEATED_BUTTONS;
                        AddMessage ("Your command buttons are no longer repeated");
                    }
                } else {
                    AddMessage ("Your command buttons setting could not be updated");
                }
            }

            // Handle server time display
            if ((pHttpForm = m_pHttpRequest->GetForm ("TimeDisplay")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldTimeDisplay")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {

                bUpdate = iNewValue != 0;
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, GAME_DISPLAY_TIME, bUpdate) == OK) {

                    m_bTimeDisplay = bUpdate;

                    if (bUpdate) {
                        m_iGameOptions |= GAME_DISPLAY_TIME;
                        AddMessage ("Server time display is now enabled");
                    } else {
                        m_iGameOptions &= ~GAME_DISPLAY_TIME;
                        AddMessage ("Server time display is no longer enabled");
                    }
                } else {
                    AddMessage ("Your server time display setting could not be updated");
                }
            }

            // Handle end turn button displacement
            if ((pHttpForm = m_pHttpRequest->GetForm ("DisplaceEndTurn")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldDisplaceEndTurn")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {

                bUpdate = iNewValue != 0;
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, DISPLACE_ENDTURN_BUTTON, bUpdate) == OK) {

                    if (bUpdate) {
                        m_iGameOptions |= DISPLACE_ENDTURN_BUTTON;
                        AddMessage ("The End Turn button is now displaced");
                    } else {
                        m_iGameOptions &= ~DISPLACE_ENDTURN_BUTTON;
                        AddMessage ("The End Turn button is no longer displaced");
                    }
                } else {
                    AddMessage ("The End Turn button displacement setting could not be updated");
                }
            }

            // Countdown
            if ((pHttpForm = m_pHttpRequest->GetForm ("Countdown")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldCountdown")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {

                bUpdate = iNewValue != 0;
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, COUNTDOWN, bUpdate) == OK) {

                    if (bUpdate) {
                        m_iGameOptions |= COUNTDOWN;
                        AddMessage ("Update countdown is now enabled");
                    } else {
                        m_iGameOptions &= ~COUNTDOWN;
                        AddMessage ("Update countdown is now disabled");
                    }
                } else {
                    AddMessage ("Your update countdown setting could not be updated");
                }
            }

            // Map coloring
            if ((pHttpForm = m_pHttpRequest->GetForm ("MapColoring")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMapColoring")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {
                bUpdate = iNewValue != 0;
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, MAP_COLORING, bUpdate) == OK) {

                    if (bUpdate) {
                        m_iGameOptions |= MAP_COLORING;
                        AddMessage ("Map coloring by diplomatic status is now enabled");
                    } else {
                        m_iGameOptions &= ~MAP_COLORING;
                        AddMessage ("Map coloring by diplomatic status is now disabled");
                    }
                } else {
                    AddMessage ("Your map coloring setting could not be updated");
                }
            }

            // Ship map coloring
            if ((pHttpForm = m_pHttpRequest->GetForm ("ShipMapColoring")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldShipMapColoring")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {

                bUpdate = iNewValue != 0;
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIP_MAP_COLORING, bUpdate) == OK) {

                    if (bUpdate) {
                        m_iGameOptions |= SHIP_MAP_COLORING;
                        AddMessage ("Ship coloring by diplomatic status is now enabled");
                    } else {
                        m_iGameOptions &= ~SHIP_MAP_COLORING;
                        AddMessage ("Ship coloring by diplomatic status is now disabled");
                    }
                } else {
                    AddMessage ("Your ship coloring setting could not be updated");
                }
            }

            // Ship highlighting
            if ((pHttpForm = m_pHttpRequest->GetForm ("ShipHighlighting")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldShipHighlighting")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {

                bUpdate = iNewValue != 0;
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIP_MAP_HIGHLIGHTING, bUpdate) == OK) {

                    if (bUpdate) {
                        m_iGameOptions |= SHIP_MAP_HIGHLIGHTING;
                        AddMessage ("Ship highlighting on the map screen is now enabled");
                    } else {
                        m_iGameOptions &= ~SHIP_MAP_HIGHLIGHTING;
                        AddMessage ("Ship highlighting on the map screen is now disabled");
                    }
                } else {
                    AddMessage ("Your ship highlighting setting could not be updated");
                }
            }

            // Sensitive Maps
            if ((pHttpForm = m_pHttpRequest->GetForm ("SensitiveMaps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldSensitiveMaps")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {
                bUpdate = iNewValue != 0;
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SENSITIVE_MAPS, bUpdate) == OK) {

                    if (bUpdate) {
                        m_iGameOptions |= SENSITIVE_MAPS;
                        AddMessage ("Sensitive maps are now enabled");
                    } else {
                        m_iGameOptions &= ~SENSITIVE_MAPS;
                        AddMessage ("Sensitive maps are now disabled");
                    }
                } else {
                    AddMessage ("Your sensitive maps setting could not be updated");
                }
            }

            // Partial maps
            if ((pHttpForm = m_pHttpRequest->GetForm ("PartialMaps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (m_iGameOptions & PARTIAL_MAPS) != 0;
            if (bFlag != (iNewValue != 0)) {

                if ((iErrCode = SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, PARTIAL_MAPS, !bFlag)) == OK) {
                    if (!bFlag) {
                        m_iGameOptions |= PARTIAL_MAPS;
                        AddMessage ("Partial maps are now enabled");
                    } else {
                        m_iGameOptions &= ~PARTIAL_MAPS;
                        AddMessage ("Partial maps are now disabled");
                    }
                } else {
                    AddMessage ("Your partial maps setting could not be updated: the error code was ");
                    AppendMessage (iErrCode);
                }
            }
            
            // MiniMaps
            if ((pHttpForm = m_pHttpRequest->GetForm ("MiniMaps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMiniMaps")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iNewValue != iOldValue && iNewValue >= MINIMAPS_NEVER && iNewValue <= MINIMAPS_PRIMARY) {

                if ((iErrCode = SetEmpireGameProperty(
                    m_iGameClass, 
                    m_iGameNumber, 
                    m_iEmpireKey, 
                    GameEmpireData::MiniMaps, 
                    iNewValue)
                    ) == OK) {
                    
                    AddMessage ("Your mini-maps setting was updated");
                    
                } else {
                    AddMessage ("Your mini-maps setting could not be updated: the error code was ");
                    AppendMessage (iErrCode);
                }
            }

            // UpCloseShips
            if ((pHttpForm = m_pHttpRequest->GetForm ("UpCloseShips")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (m_iGameOptions & SHIPS_ON_MAP_SCREEN) != 0;
            if (bFlag != ((iNewValue & SHIPS_ON_MAP_SCREEN) != 0)) {

                if ((iErrCode = SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIPS_ON_MAP_SCREEN, !bFlag)) == OK) {
                    if (!bFlag) {
                        m_iGameOptions |= SHIPS_ON_MAP_SCREEN;
                        AddMessage ("Ship menus will now be displayed on map screen planet views");
                    } else {
                        m_iGameOptions &= ~SHIPS_ON_MAP_SCREEN;
                        AddMessage ("Ship menus will no longer be displayed on map screen planet views");
                    }
                } else {
                    AddMessage ("Your map screen ship menu setting could not be changed: the error code was ");
                    AppendMessage (iErrCode);
                }
            }

            bFlag = (m_iGameOptions & SHIPS_ON_PLANETS_SCREEN) != 0;
            if (bFlag != ((iNewValue & SHIPS_ON_PLANETS_SCREEN) != 0)) {

                if ((iErrCode = SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIPS_ON_PLANETS_SCREEN, !bFlag)) == OK) {
                    if (!bFlag) {
                        m_iGameOptions |= SHIPS_ON_PLANETS_SCREEN;
                        AddMessage ("Ship menus will now be displayed on the planets screen");
                    } else {
                        m_iGameOptions &= ~SHIPS_ON_PLANETS_SCREEN;
                        AddMessage ("Ship menus will no longer be displayed on the planets screen");
                    }
                } else {
                    AddMessage ("Your planet screen ship menu setting could not be changed: the error code was ");
                    AppendMessage (iErrCode);
                }
            }

            // UpCloseShips
            if ((pHttpForm = m_pHttpRequest->GetForm ("UpCloseBuilds")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (m_iGameOptions & BUILD_ON_MAP_SCREEN) != 0;
            if (bFlag != ((iNewValue & BUILD_ON_MAP_SCREEN) != 0)) {

                if ((iErrCode = SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, BUILD_ON_MAP_SCREEN, !bFlag)) == OK) {
                    if (!bFlag) {
                        m_iGameOptions |= BUILD_ON_MAP_SCREEN;
                        AddMessage ("Build menus will now be displayed on map screen planet views");
                    } else {
                        m_iGameOptions &= ~BUILD_ON_MAP_SCREEN;
                        AddMessage ("Build menus will no longer be displayed on map screen planet views");
                    }
                } else {
                    AddMessage ("Your map screen build menu setting could not be changed: the error code was ");
                    AppendMessage (iErrCode);
                }
            }

            bFlag = (m_iGameOptions & BUILD_ON_PLANETS_SCREEN) != 0;
            if (bFlag != ((iNewValue & BUILD_ON_PLANETS_SCREEN) != 0)) {

                if ((iErrCode = SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, BUILD_ON_PLANETS_SCREEN, !bFlag)) == OK) {
                    if (!bFlag) {
                        m_iGameOptions |= BUILD_ON_PLANETS_SCREEN;
                        AddMessage ("Build menus will now be displayed on the planets screen");
                    } else {
                        m_iGameOptions &= ~BUILD_ON_PLANETS_SCREEN;
                        AddMessage ("Build menus will no longer be displayed on the planets screen");
                    }
                } else {
                    AddMessage ("Your planet screen ship menu setting could not be changed: the error code was ");
                    AppendMessage (iErrCode);
                }
            }

            // LocalMaps
            if ((pHttpForm = m_pHttpRequest->GetForm ("LocalMaps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (m_iGameOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;
            if (bFlag != (iNewValue != 0)) {

                if ((iErrCode = SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, LOCAL_MAPS_IN_UPCLOSE_VIEWS, !bFlag)) == OK) {
                    if (!bFlag) {
                        m_iGameOptions |= LOCAL_MAPS_IN_UPCLOSE_VIEWS;
                        AddMessage ("Local maps will now be displayed in up-close map views");
                    } else {
                        m_iGameOptions &= ~LOCAL_MAPS_IN_UPCLOSE_VIEWS;
                        AddMessage ("Local maps will no longer be displayed in up-close map views");
                    }
                } else {
                    AddMessage ("Your local map setting could not be changed: the error code was ");
                    AppendMessage (iErrCode);
                }
            }

            // Ratios
            if ((pHttpForm = m_pHttpRequest->GetForm ("Ratios")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != m_iGameRatios && 
                iNewValue >= RATIOS_DISPLAY_NEVER && iNewValue <= RATIOS_DISPLAY_ALWAYS) {

                GameCheck (SetEmpireGameProperty (
                    m_iGameClass,
                    m_iGameNumber,
                    m_iEmpireKey,
                    GameEmpireData::GameRatios,
                    iNewValue
                    ));

                m_iGameRatios = iNewValue;

                AddMessage ("Your game ratios line setting was updated");
            }


            // Handle DefaultBuilderPlanet
            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultBuilderPlanet")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            int iRealPlanet;
            GameCheck (GetEmpireDefaultBuilderPlanet (
                m_iGameClass,
                m_iGameNumber,
                m_iEmpireKey,
                &iOldValue,
                &iRealPlanet
                ));

            if (iNewValue != iOldValue) {

                iErrCode = SetEmpireDefaultBuilderPlanet (m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue);
                if (iErrCode == OK) {
                    AddMessage ("The default builder planet was updated");
                } else {
                    AddMessage ("The default builder planet could not be updated; the error was ");
                    AppendMessage (iErrCode);
                }
            }

            // Handle IndependentGifts
            if (iGameClassOptions & INDEPENDENCE) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("IndependentGifts")) == NULL) {
                    goto Redirection;
                }
                iNewValue = pHttpForm->GetIntValue();

                bFlag = (m_iGameOptions & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;
                if (bFlag != (iNewValue != 0)) {

                    if ((iErrCode = SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, REJECT_INDEPENDENT_SHIP_GIFTS, !bFlag)) == OK) {
                        if (!bFlag) {
                            m_iGameOptions |= REJECT_INDEPENDENT_SHIP_GIFTS;
                            AddMessage ("Independent ship gifts will now be rejected");
                        } else {
                            m_iGameOptions &= ~REJECT_INDEPENDENT_SHIP_GIFTS;
                            AddMessage ("Independent ship gifts will now be accepted");
                        }
                    } else {
                        AddMessage ("Your independent ship setting could not be changed: the error code was ");
                        AppendMessage (iErrCode);
                    }
                }
            }

            // Handle MessageTarget
            if ((pHttpForm = m_pHttpRequest->GetForm ("MessageTarget")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            GameCheck (GetEmpireDefaultMessageTarget (
                m_iGameClass,
                m_iGameNumber,
                m_iEmpireKey,
                &iOldValue
                ));

            if (iNewValue != iOldValue) {

                iErrCode = SetEmpireDefaultMessageTarget (m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue);
                if (iErrCode == OK) {
                    AddMessage ("The default message target was updated");
                } else {
                    AddMessage ("The default message target could not be updated; the error was ");
                    AppendMessage (iErrCode);
                }
            }

            // MaxSavedMessages
            if ((pHttpForm = m_pHttpRequest->GetForm ("MaxSavedMessages")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxSavedMessages")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {
                if (SetEmpireMaxNumSavedGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, 
                    iNewValue) == OK) {

                    char pszMessage [256];
                    sprintf(pszMessage, "Up to %i game messages will be saved", iNewValue);
                    AddMessage (pszMessage);
                } else {
                    AddMessage ("Your max saved game messages setting could not be updated");
                }
            }

            // Ignore
            if ((pHttpForm = m_pHttpRequest->GetForm ("Ignore")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldIgnore")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iOldValue != iNewValue) {
                if (SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, IGNORE_BROADCASTS, iNewValue != 0) == OK) {
                    if (iNewValue != 0) {
                        m_iGameOptions |= IGNORE_BROADCASTS;
                        AddMessage ("You will now ignore all broadcasts");
                    } else {
                        m_iGameOptions &= ~IGNORE_BROADCASTS;
                        AddMessage ("You will no longer ignore all broadcasts");
                    }
                } else {
                    AddMessage ("Your ignore broadcast setting be updated");
                }
            }

            // Changes to pause status
            if (bGameStarted && m_iNumNewUpdates == m_iNumOldUpdates) {

                int iPause;
                bool bOldPause;
                if ((pHttpForm = m_pHttpRequest->GetForm ("Pause")) != NULL) {

                    iPause = pHttpForm->GetIntValue();

                    // Get selected dip option
                    GameCheck (IsEmpireRequestingPause (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bOldPause));

                    // Only update if we changed the status
                    if ((iPause != 0) != bOldPause) {

                        if (iPause != 0) {

                            GameCheck (RequestPause (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_iGameState));

                            if (m_iGameState & ADMIN_PAUSED) {
                                AddMessage ("The game was already paused by an admin");
                            }

                            else if (m_iGameState & PAUSED) {
                                AddMessage ("You paused the game");
                            }

                            else {
                                AddMessage ("You are now requesting a pause");
                            }

                            m_iGameOptions |= REQUEST_PAUSE;

                        } else {

                            GameCheck (RequestNoPause (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_iGameState));

                            if (m_iGameState & ADMIN_PAUSED) {
                                AddMessage ("The game has been paused by an administrator and will remain paused");
                            }

                            else if (!(m_iGameState & PAUSED) && (m_iGameState & PAUSED)) {
                                AddMessage ("You unpaused the game");
                            }

                            else {
                                AddMessage ("You are no longer requesting a pause");
                            }

                            m_iGameOptions &= ~REQUEST_PAUSE;
                        }
                    }
                }

                // Changes to draw status
                if (iGameClassOptions & ALLOW_DRAW) {

                    int iDraw;
                    bool bOldDraw;
                    if ((pHttpForm = m_pHttpRequest->GetForm ("Draw")) != NULL) {

                        iDraw = pHttpForm->GetIntValue();

                        // Get selected dip option
                        GameCheck (IsEmpireRequestingDraw (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bOldDraw));

                        // Only update if we changed the status
                        if ((iDraw != 0) != bOldDraw) {

                            if (iDraw != 0) {

                                GameCheck (RequestDraw (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_iGameState));

                                m_iGameOptions |= REQUEST_DRAW;

                                if (!(m_iGameState & GAME_ENDED))
                                {
                                    AddMessage ("You are now requesting a draw");
                                }
                                else
                                {
                                    bool bEndGame = false;

                                    // Try to end the game
                                    iErrCode = CheckGameForEndConditions (
                                        m_iGameClass,
                                        m_iGameNumber,
                                        NULL,
                                        &bEndGame
                                        );

                                    // Did the game end because of us?
                                    if (iErrCode == OK && bEndGame) {
                                        return Redirect (ACTIVE_GAME_LIST);
                                    }

                                    // Refresh game state
                                    GameCheck(GetGameState (m_iGameClass, m_iGameNumber, &m_iGameState));

                                    // Proceed...
                                    AddMessage ("You are now requesting a draw");
                                }

                            } else {

                                GameCheck (RequestNoDraw (m_iGameClass, m_iGameNumber, m_iEmpireKey));
                                AddMessage ("You are no longer requesting a draw");

                                m_iGameOptions &= ~REQUEST_DRAW;
                            }
                        }
                    }
                }
            }


            // Notepad
            if ((pHttpForm = m_pHttpRequest->GetForm ("Notepad")) == NULL) {
                goto Redirection;
            }

            iErrCode = UpdateGameEmpireNotepad (
                m_iGameClass,
                m_iGameNumber,
                m_iEmpireKey,
                pHttpForm->GetValue(),
                &bFlag
                );

            switch (iErrCode) {

            case WARNING:
                break;

            case OK:

                if (bFlag) {
                    AddMessage ("Your notepad was updated, but the contents were truncated");
                } else {
                    AddMessage ("Your notepad was updated");
                }
                break;

            default:

                AddMessage ("Your notepad could not be updated. The error was ");
                AppendMessage (iErrCode);
                break;
            }

            // Check for ViewMessages button press
            if (WasButtonPressed (BID_VIEWMESSAGES)) {
                iOptionPage = 1;
                m_bRedirectTest = false;
                break;
            }

            // Check for search for empires with duplicate IP's
            if (WasButtonPressed (BID_SEARCHIPADDRESSES)) {

                m_bRedirectTest = false;
                SearchForDuplicateIPAddresses (m_iGameClass, m_iGameNumber);
                break;
            }

            // Check for search for empires with duplicate session ids
            if (WasButtonPressed (BID_SEARCHSESSIONIDS)) {

                m_bRedirectTest = false;
                SearchForDuplicateSessionIds (m_iGameClass, m_iGameNumber);
                break;
            }

            // Resign, surrender
            if (m_iGameState & STARTED) {

                if (WasButtonPressed (BID_RESIGN)) {
                    m_iReserved = BID_RESIGN;
                    return Redirect (QUIT);
                }
            }

            if (!(m_iGameState & STILL_OPEN)) {

                if (WasButtonPressed (BID_SURRENDER)) {

                    // Make sure this is allowed
                    if ((iGameClassOptions & USE_SC30_SURRENDERS) ||

                        ((GameAllowsDiplomacy (iDiplomacy, SURRENDER) ||
                         (iGameClassOptions & ONLY_SURRENDER_WITH_TWO_EMPIRES)) &&
                         iNumEmpires == 2)) {

                        m_iReserved = BID_SURRENDER;
                        return Redirect (QUIT);
                    }
                }
            }
        }

        break;

    case 1:

        {
            int iMessageKey, iDeletedMessages = 0;
            unsigned int iNumTestMessages;

            // Get number of messages
            if ((pHttpForm = m_pHttpRequest->GetForm ("NumSavedGameMessages")) == NULL) {
                goto Redirection;
            }
            iNumTestMessages = pHttpForm->GetUIntValue();

            // Check for delete all
            char pszForm [128];

            if (WasButtonPressed (BID_ALL)) {

                m_bRedirectTest = false;

                for (i = 0; i < iNumTestMessages; i ++) {

                    // Get message key
                    sprintf(pszForm, "MsgKey%i", i);
                    if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                        goto Redirection;
                    }
                    iMessageKey = pHttpForm->GetIntValue();

                    // Delete message
                    if (DeleteGameMessage (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey) == OK) {
                        iDeletedMessages ++;
                    }
                }

            } else {

                // Check for delete selection
                if (WasButtonPressed (BID_SELECTION)) {

                    m_bRedirectTest = false;

                    for (i = 0; i < iNumTestMessages; i ++) {

                        // Get selected status of message's delete checkbox
                        sprintf(pszForm, "DelChBx%i", i);
                        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) != NULL) {

                            // Get message key
                            sprintf(pszForm, "MsgKey%i", i);
                            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                                goto Redirection;
                            }
                            iMessageKey = pHttpForm->GetIntValue();

                            // Delete message
                            if (DeleteGameMessage (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey) == OK) {
                                iDeletedMessages ++;
                            }
                        }
                    }

                } else {

                    // Check for delete system messages
                    if (WasButtonPressed (BID_SYSTEM)) {

                        Variant vFlags;
                        m_bRedirectTest = false;

                        for (i = 0; i < iNumTestMessages; i ++) {

                            // Get message key
                            sprintf(pszForm, "MsgKey%i", i);
                            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                                goto Redirection;
                            }
                            iMessageKey = pHttpForm->GetIntValue();

                            if (GetGameMessageProperty(
                                m_iGameClass,
                                m_iGameNumber,
                                m_iEmpireKey,
                                iMessageKey,
                                GameEmpireMessages::Flags,
                                &vFlags
                                ) == OK &&

                                (vFlags.GetInteger() & MESSAGE_SYSTEM) &&

                                DeleteGameMessage (
                                m_iGameClass,
                                m_iGameNumber,
                                m_iEmpireKey,
                                iMessageKey
                                ) == OK) {

                                iDeletedMessages ++;
                            }
                        }

                    } else {

                        // Check for delete empire message
                        if (WasButtonPressed (BID_EMPIRE)) {

                            Variant vSource;
                            m_bRedirectTest = false;

                            // Get target empire
                            if ((pHttpForm = m_pHttpRequest->GetForm ("SelectedEmpire")) == NULL) {
                                goto Redirection;
                            }
                            const char* pszSrcEmpire = pHttpForm->GetValue();

                            for (i = 0; i < iNumTestMessages; i ++) {

                                sprintf(pszForm, "MsgKey%i", i);
                                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                                    goto Redirection;
                                }
                                iMessageKey = pHttpForm->GetIntValue();

                                if (GetGameMessageProperty(
                                    m_iGameClass,
                                    m_iGameNumber,
                                    m_iEmpireKey,
                                    iMessageKey,
                                    GameEmpireMessages::SourceName,
                                    &vSource
                                    ) == OK &&

                                    String::StrCmp (vSource.GetCharPtr(), pszSrcEmpire) == 0 &&

                                    DeleteGameMessage (
                                    m_iGameClass,
                                    m_iGameNumber,
                                    m_iEmpireKey,
                                    iMessageKey
                                    ) == OK) {

                                    iDeletedMessages ++;
                                }
                            }
                        }
                    }
                }
            }

            if (iDeletedMessages > 0) {

                char pszMessage [256];
                sprintf (
                    pszMessage,
                    "%i game message%s deleted", 
                    iDeletedMessages,
                    iDeletedMessages == 1 ? " was" : "s were"
                    );

                AddMessage (pszMessage);
            }

        }
        break;

    case 2:

        // Nothing submitted
        break;

    default:

        Assert (false);
    }
}

Redirection:
if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
    GameCheck(RedirectOnSubmitGame(&pageRedirect, &bRedirected));
    if (bRedirected)
    {
        return Redirect (pageRedirect);
    }
}

GameCheck(OpenGamePage());

// Individual page stuff starts here

bool bFlag;
int j, iNumNames = 0, iValue;

if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
    GameCheck (WriteRatiosString (NULL));
}

switch (iOptionPage) {

case 0:
    {

    %><input type="hidden" name="OptionPage" value="0"><p><%
    %><p><table width="80%"><%

    %><tr><td>Placement of command buttons:</td><td><select name="RepeatButtons"><%
    if (m_bRepeatedButtons) { 
        %><option value="0">At top of screen only</option><%
        %><option selected value="1">At top and bottom of screen</option><%
    } else { 
        %><option selected value="0">At top of screen only</option><%
        %><option value="1">At top and bottom of screen</option><%
    } %></select><input type="hidden" name="OldRepeatButtons" value="<% Write (m_bRepeatedButtons ? 1:0);
    %>"></td></tr><%


    %><tr><td>Server time display:</td><td><select name="TimeDisplay"><%
    if (m_bTimeDisplay) { 
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else { 
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select><input type="hidden" name="OldTimeDisplay" value="<% Write (m_bTimeDisplay ? 1:0);
    %>"></td></tr><%


    %><tr><td>End Turn button displacement:</td><td><select name="DisplaceEndTurn"><%
    if (m_iGameOptions & DISPLACE_ENDTURN_BUTTON) { 
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else { 
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select><input type="hidden" name="OldDisplaceEndTurn" value="<% 
    Write ((m_iGameOptions & DISPLACE_ENDTURN_BUTTON) ? 1:0); %>"></td></tr><%


    %><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><%
    %><td><select name="AutoRefresh"><%

    if (m_iGameOptions & AUTO_REFRESH) {
        %><option selected value="1">Yes</option><option value="0">No</option><%
    } else {
        %><option value="1">Yes</option><option selected value="0">No</option><%
    }

    %></select><input type="hidden" name="OldAutoRefresh" value="<%
    Write ((m_iGameOptions & AUTO_REFRESH) ? 1:0); %>"></td></tr><%


    %><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name="Countdown"><%
    if (m_iGameOptions & COUNTDOWN) { 
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else {
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select><input type="hidden" name="OldCountdown" value="<% Write ((m_iGameOptions & COUNTDOWN) ? 1:0); %>"><%

    %></td></tr><%

    bFlag = (m_iGameOptions & MAP_COLORING) != 0;

    %><tr><td>Map coloring by diplomatic status:</td><td><select name="MapColoring"><%
    if (bFlag) {
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else { 
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select><input type="hidden" name="OldMapColoring" value="<% Write (bFlag ? 1:0); %>"><%
    %></td></tr><%


    bFlag = (m_iGameOptions & SHIP_MAP_COLORING) != 0;

    %><tr><td>Ship coloring by diplomatic status:</td><td><select name="ShipMapColoring"><%
    if (bFlag) {
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else { 
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select><input type="hidden" name="OldShipMapColoring" value="<% Write (bFlag ? 1:0); %>"><%
    %></td></tr><%


    bFlag = (m_iGameOptions & SHIP_MAP_HIGHLIGHTING) != 0;

    %><tr><td>Ship highlighting on map screen:</td><td><select name="ShipHighlighting"><%
    if (bFlag) {
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else { 
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select><input type="hidden" name="OldShipHighlighting" value="<% Write (bFlag ? 1:0); %>"><%
    %></td></tr><%


    bFlag = (m_iGameOptions & SENSITIVE_MAPS) != 0;

    %><tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name="SensitiveMaps"><%
    if (bFlag) {
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else { 
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select><input type="hidden" name="OldSensitiveMaps" value="<% Write (bFlag ? 1:0); %>"><%
    %></td></tr><%


    bFlag = (m_iGameOptions & PARTIAL_MAPS) != 0;

    %><tr><td>Partial maps:</td><td><select name="PartialMaps"><%
    if (bFlag) {
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else { 
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select></td></tr><%
    
    
    GameCheck (GetEmpireGameProperty (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        GameEmpireData::MiniMaps,
        &vTemp
        ));

    %><input type="hidden" name="OldMiniMaps" value="<% Write (vTemp.GetInteger()); %>"><%
    %><tr><td>Mini-maps:</td><td><select name="MiniMaps"><%
    
    %><option <%
    if (vTemp.GetInteger() == MINIMAPS_NEVER) {
        %>selected <%
    }
    %>value="<% Write (MINIMAPS_NEVER); %>">Never show mini-maps</option><%
    
    %><option <%
    if (vTemp.GetInteger() == MINIMAPS_OPTION) {
        %>selected <%
    }
    %>value="<% Write (MINIMAPS_OPTION); %>">Show mini-maps as an option</option><%
    
    %><option <%
    if (vTemp.GetInteger() == MINIMAPS_PRIMARY) {
        %>selected <%
    }
    %>value="<% Write (MINIMAPS_PRIMARY); %>">Show mini-maps by default</option><%
    
    %></select></td></tr><%


    iValue = m_iGameOptions & (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN);

    %><tr><td>Display ship menus in planet views:</td><td><select name="UpCloseShips"><%

    %><option <% if (iValue == (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN)) { %>selected <% }
    %>value="<% Write (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN); %>"><%
    %>Ship menus on both map and planets screens</option><%

    %><option <% if (iValue == SHIPS_ON_MAP_SCREEN) { %>selected <% }
    %>value="<% Write (SHIPS_ON_MAP_SCREEN); %>"><%
    %>Ship menus on map screen</option><%

    %><option <% if (iValue == SHIPS_ON_PLANETS_SCREEN) { %>selected <% }
    %>value="<% Write (SHIPS_ON_PLANETS_SCREEN); %>"><%
    %>Ship menus on planets screen</option><%

    %><option <% if (iValue == 0) { %>selected <% }
    %>value="0"><%
    %>No ship menus in planet views</option><%

    %></select></td></tr><%


    iValue = m_iGameOptions & (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN);

    %><tr><td>Display build menus in planet views:</td><td><select name="UpCloseBuilds"><%

    %><option <% if (iValue == (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN)) { %>selected <% }
    %>value="<% Write (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN); %>"><%
    %>Build menus on both map and planets screens</option><%

    %><option <% if (iValue == BUILD_ON_MAP_SCREEN) { %>selected <% }
    %>value="<% Write (BUILD_ON_MAP_SCREEN); %>"><%
    %>Build menus on map screen</option><%

    %><option <% if (iValue == BUILD_ON_PLANETS_SCREEN) { %>selected <% }
    %>value="<% Write (BUILD_ON_PLANETS_SCREEN); %>"><%
    %>Build menus on planets screen</option><%

    %><option <% if (iValue == 0) { %>selected <% }
    %>value="0"><%
    %>No build menus in planet views</option><%

    %></select></td></tr><%


    bFlag = (m_iGameOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;

    %><tr><td>Display local maps in up-close map views:</td><td><select name="LocalMaps"><%

    if (bFlag) {
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else {
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select></td></tr><%


    GameCheck (GetEmpireGameProperty (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        GameEmpireData::GameRatios,
        &vTemp
        ));

    int iGameRatios = vTemp.GetInteger();

    %><tr><td>Display ratios line:</td><td><select name="Ratios"><%

    %><option<%
    if (iGameRatios == RATIOS_DISPLAY_NEVER) {
        %> selected<%
    } %> value="<% Write (RATIOS_DISPLAY_NEVER); %>">Never</option><%

    %><option<%
    if (iGameRatios == RATIOS_DISPLAY_ON_RELEVANT_SCREENS) {
        %> selected<%
    } %> value="<% Write (RATIOS_DISPLAY_ON_RELEVANT_SCREENS); %>">On relevant game screens</option><%

    %><option<%
    if (iGameRatios == RATIOS_DISPLAY_ALWAYS) {
        %> selected<%
    } %> value="<% Write (RATIOS_DISPLAY_ALWAYS); %>">On all game screens</option><%

    %></select></td></tr><%


    %><tr><td>Default builder planet:</td><td><select name="DefaultBuilderPlanet"><%

    int iRealPlanet;
    GameCheck (GetEmpireDefaultBuilderPlanet (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        &iValue,
        &iRealPlanet
        ));

    %><option <%
    if (iValue == HOMEWORLD_DEFAULT_BUILDER_PLANET) {
        %>selected <%
    }
    %>value="<% Write (HOMEWORLD_DEFAULT_BUILDER_PLANET); %>">Homeworld</option><%

    %><option <%
    if (iValue == LAST_BUILDER_DEFAULT_BUILDER_PLANET) {
        %>selected <%
    }
    %>value="<% Write (LAST_BUILDER_DEFAULT_BUILDER_PLANET); %>">Last builder planet used</option><%

    %><option <%
    if (iValue == NO_DEFAULT_BUILDER_PLANET) {
        %>selected <%
    }
    %>value="<% Write (NO_DEFAULT_BUILDER_PLANET); %>">No default builder planet</option><%

    unsigned int* piBuilderKey = NULL, iNumBuilderKeys;
    GameCheck (GetBuilderPlanetKeys (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        &piBuilderKey,
        &iNumBuilderKeys
        ));

    if (iNumBuilderKeys > 0) {

        String strFilter;
        char pszPlanetName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH];

        for (i = 0; (unsigned int) i < iNumBuilderKeys; i ++) {

            iErrCode = GetPlanetNameWithCoordinates (
                m_iGameClass,
                m_iGameNumber,
                piBuilderKey[i],
                pszPlanetName
                );

            if (iErrCode == OK) {

                if (HTMLFilter (pszPlanetName, &strFilter, 0, false) == OK) {

                    %><option <%
                    if ((unsigned int) iValue == piBuilderKey[i]) {
                        %>selected <%
                    }
                    %>value="<% Write (piBuilderKey[i]); %>"><%

                    Write (strFilter.GetCharPtr(), strFilter.GetLength()); %></option><%
                }
            }
        }

        t_pCache->FreeKeys (piBuilderKey);
        piBuilderKey = NULL;
    }

    GameCheck (GetEmpireDefaultMessageTarget (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        &iValue
        ));

    %></select></td></tr><%


    if (iGameClassOptions & INDEPENDENCE) {

        %><tr><td>Independent ship gifts:</td><td><select name="IndependentGifts"><option<%

        bFlag = (m_iGameOptions & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

        if (!bFlag) {
            %> selected<%
        } %> value="<% Write (0); %>">Accept independent ship gifts</option><option<%

        if (bFlag) {
            %> selected<%
        } %> value="<% Write (1); %>">Reject independent ship gifts</option></select></tr><%
    }

    %><tr><td>Default message target:</td><td><select name="MessageTarget"><%

    %><option<%
    if (iValue == MESSAGE_TARGET_NONE) {
        %> selected<%
    }
    %> value="<% Write (MESSAGE_TARGET_NONE); %>">None</option><%

    %><option<%
    if (iValue == MESSAGE_TARGET_BROADCAST) {
        %> selected<%
    }
    %> value="<% Write (MESSAGE_TARGET_BROADCAST); %>">Broadcast</option><%

    %><option<%
    if (iValue == MESSAGE_TARGET_WAR) {
        %> selected<%
    }
    %> value="<% Write (MESSAGE_TARGET_WAR); %>">All at War</option><%

    if (iDiplomacy & TRUCE) {

        %><option<%
        if (iValue == MESSAGE_TARGET_TRUCE) {
            %> selected<%
        }
        %> value="<% Write (MESSAGE_TARGET_TRUCE); %>">All at Truce</option><%
    }

    if (iDiplomacy & TRADE) {

        %><option<%
        if (iValue == MESSAGE_TARGET_TRADE) {
            %> selected<%
        }
        %> value="<% Write (MESSAGE_TARGET_TRADE); %>">All at Trade</option><%
    }

    if (iDiplomacy & ALLIANCE) {

        %><option<%
        if (iValue == MESSAGE_TARGET_ALLIANCE) {
            %> selected<%
        }
        %> value="<% Write (MESSAGE_TARGET_ALLIANCE); %>">All at Alliance</option><%
    }

    %><option<%
    if (iValue == MESSAGE_TARGET_LAST_USED) {
        %> selected<%
    }
    %> value="<% Write (MESSAGE_TARGET_LAST_USED); %>">Last target used</option><%

    %></select></td></tr><%

    %><tr><td>Game messages saved:<td valign="middle"><%

    unsigned int iNumMessages;

    GameCheck (GetNumGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumMessages)); 
    if (iNumMessages > 0) {
        Write (iNumMessages);
        %>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<%
        WriteButton (BID_VIEWMESSAGES);
    } else {
        %>None<%
    }
    %></td></tr><%

    %><tr><td>Maximum saved game messages</td><td><select name="MaxSavedMessages"><%
    
    Variant vMaxNumMessages;

    GameCheck (GetEmpireMaxNumSavedGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumMessages));
    GameCheck (GetSystemProperty (SystemData::MaxNumGameMessages, &vMaxNumMessages));
    
    unsigned int iMaxNumMessages = vMaxNumMessages.GetInteger();

    for (i = 0; i < iMaxNumMessages; i += 10) {
        %><option <%

        if (i == iNumMessages) {
            %>selected <%
        }
        %>value="<% Write (i); %>"><% Write (i); %></option><%
    }
    %></select><input type="hidden" name="OldMaxSavedMessages" value="<% Write (iNumMessages); 
    %>"></td></tr><%


    bFlag = (m_iGameOptions & IGNORE_BROADCASTS) != 0;

    %><tr><td>Ignore broadcasts:</td><td><select name="Ignore"><%

    if (bFlag) {
        %><option selected value="1">Yes</option><%
        %><option value="0">No</option><%
    } else {
        %><option value="1">Yes</option><%
        %><option selected value="0">No</option><%
    } %></select><input type="hidden" name="OldIgnore" value="<% Write (bFlag ? 1:0); %>"></td></tr><%

    %><tr><td>Search for empires with duplicate IP addresses:</td><td><%
    WriteButton (BID_SEARCHIPADDRESSES);
    %></td></tr><%

    %><tr><td>Search for empires with duplicate Session Ids:</td><td><%
    WriteButton (BID_SEARCHSESSIONIDS);
    %></td></tr><%

    if (bGameStarted) {


        %><tr><td>Request pause:</td><td><%

        %><select name="Pause"><%

        %><option <%
        if (m_iGameOptions & REQUEST_PAUSE) {
            %>selected<%
        } %> value="1">Yes</option><%

        %><option <%
        if (!(m_iGameOptions & REQUEST_PAUSE)) {
            %>selected<%
        } %> value="0">No</option><%

        %></select><%

        GameCheck (GetNumEmpiresRequestingPause (m_iGameClass, m_iGameNumber, &i));

        %> <strong><% Write (i); %></strong> of <strong><% Write (iNumEmpires); %></strong> empire<%

        if (iNumEmpires != 1) {
            %>s<%
        }

        if (i == 1 || iNumEmpires == 1) {
            %> is<%
        } else {
            %> are<%
        }

        %> requesting a pause<%

        %></td></tr><%
    }

    if (bGameStarted && (iGameClassOptions & ALLOW_DRAW)) {

        %><tr><td>Request draw:</td><td><%

        %><select name="Draw"><%

        %><option <%
        if (m_iGameOptions & REQUEST_DRAW) {
            %>selected<%
        } %> value="1">Yes</option><%

        %><option <%
        if (!(m_iGameOptions & REQUEST_DRAW)) {
            %>selected<%
        } %> value="0">No</option><%

        %></select><%

        GameCheck (GetNumEmpiresRequestingDraw (m_iGameClass, m_iGameNumber, &i));

        %> <strong><% Write (i); %></strong> of <strong><% Write (iNumEmpires); %></strong> empire<%

        if (iNumEmpires != 1) {
        %>s<%
        }

        if (i == 1 || iNumEmpires == 1) {
            %> is<%
        } else {
            %> are<%
        }
        %> requesting a draw<%

        %></td></tr><%
    }


    %><tr><td align="top">Keep game notes here:<%
    %></td><td><textarea name="Notepad" rows="8" cols="50" wrap="virtual"><%


    Variant vNotepad;
    GameCheck (GetEmpireNotepad (m_iGameClass, m_iGameNumber, m_iEmpireKey, &vNotepad));
    Write (vNotepad.GetCharPtr());
    %></textarea></td><%

    if (m_iGameState & STARTED) {

        %><tr><td>Resign from the game:<%
        %><br><em>(This option will dismantle all your ships and leave your empire automatically ready for <%
        %>updates. You will not be able to log back in, and your empire will either be nuked or <%
        %>fall into ruin)</em><%

        %></td><td><%
        WriteButton (BID_RESIGN);
        %></td></tr><%
    }

    if (!(m_iGameState & STILL_OPEN)) {

        if ((iGameClassOptions & USE_SC30_SURRENDERS) ||

            ((GameAllowsDiplomacy (iDiplomacy, SURRENDER) ||
             (iGameClassOptions & ONLY_SURRENDER_WITH_TWO_EMPIRES)) &&
             iNumEmpires == 2)) {

            %><tr><td>Surrender from the game:<%

            if (iGameClassOptions & USE_SC30_SURRENDERS) {

                %><br><em>(This option will immediately remove your empire from the game. <%
                %>If an empire colonizes your old homeworld, or if the game ends with a single empire <%
                %>as the winner, then that empire will be given the nuke.)</em><%

            } else {

                %><br><em>(This option will immediately end the game. Your empire will be considered <%
                %> to be have been nuked by your foe.)</em><%
            }

            %></td><td><%
            WriteButton (BID_SURRENDER);
            %></td></tr><%
        }
    }

    %></tr></table><p><% 
    WriteButton (BID_CANCEL);

    }
    break;

case 1:

    {
    unsigned int* piMessageKey = NULL, iNumMessages;
    Variant** ppvMessage = NULL;

    GameCheck(GetSavedGameMessages(m_iGameClass, m_iGameNumber, m_iEmpireKey, &piMessageKey, &ppvMessage, &iNumMessages));

    %><input type="hidden" name="OptionPage" value="1"><%

    if (iNumMessages == 0)
    {
        %><p>You have no saved game messages<%
    }
    else
    {
        // Sort
        UTCTime* ptTime = (UTCTime*)StackAlloc(iNumMessages * sizeof(UTCTime));
        int* piIndex = (int*)StackAlloc(iNumMessages * sizeof(int));

        for (i = 0; i < iNumMessages; i ++)
        {
            piIndex[i] = i;
            ptTime[i] = ppvMessage[i][GameEmpireMessages::iTimeStamp].GetInteger64();
        }
        Algorithm::QSortTwoDescending<UTCTime, int> (ptTime, piIndex, iNumMessages);

        // Display
        String* pstrNameList = new String[iNumMessages];
        Assert(pstrNameList);
        Algorithm::AutoDelete<String> autopstrNameList(pstrNameList, true);

        %><p>You have <strong><% Write(iNumMessages); %></strong> saved game message<% 

        if (iNumMessages != 1)
        {
            %>s<%
        }
        %>:<p><table width="45%"><%

        %><input type="hidden" name="NumSavedGameMessages" value="<% Write (iNumMessages); %>"><%

        const char* pszFontColor = NULL;
        bool bSystemSent = false;

        for (i = 0; i < iNumMessages; i ++)
        {
            int iFlags = ppvMessage[piIndex[i]][GameEmpireMessages::iFlags].GetInteger();

            const char* pszSender = ppvMessage[piIndex[i]][GameEmpireMessages::iSourceName].GetCharPtr();

            %><input type="hidden" name="MsgKey<% Write (i); %>" value ="<% Write (piMessageKey[piIndex[i]]); 
            %>"><input type="hidden" name="MsgSrc<% Write (i); %>" value="<% Write (pszSender); %>"><%

            char pszDate [OS::MaxDateLength];
            iErrCode = Time::GetDateString (ppvMessage[piIndex[i]][GameEmpireMessages::iTimeStamp].GetInteger64(), pszDate);
            Assert(iErrCode == OK);

            %><tr><td><% 

            if (iFlags & MESSAGE_BROADCAST)
            {
                %>Broadcast by <%
            }
            else
            {
                %>Sent by <%
            }

            if (iFlags & MESSAGE_SYSTEM)
            { 
                bSystemSent = true;
                %><strong><% Write(SYSTEM_MESSAGE_SENDER); %></strong><%
            }
            else
            {
                %><strong><% Write(pszSender); %></strong><%

                // Find name in lists
                bFlag = false;
                for (j = 0; j < iNumNames; j ++)
                {
                    if (pstrNameList[j].Equals(pszSender))
                    {
                        bFlag = true;
                        break;
                    }
                }

                // Add name to list if not found
                if (!bFlag)
                {
                    pstrNameList[iNumNames] = pszSender;
                    iNumNames ++;
                }
            }

            %> on <% Write (pszDate);

            if (iFlags & MESSAGE_BROADCAST)
            {
                pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
            }
            else
            {
                pszFontColor = m_vPrivateMessageColor.GetCharPtr();
            }

            %><br><input type="checkbox" name="DelChBx<% Write(i); %>"> Delete</td></tr><%
            %><tr><td><font size="<% Write(DEFAULT_MESSAGE_FONT_SIZE); %>" face="<% Write (DEFAULT_MESSAGE_FONT); %>"<%

            // Game messages from the system suffer from no special coloring
            if (!(iFlags & MESSAGE_SYSTEM)) {
                %> color="#<% Write (pszFontColor); %>"<%
            }
            %>><%

            WriteFormattedMessage (ppvMessage[piIndex[i]][GameEmpireMessages::iText].GetCharPtr());

            %></font></td></tr><tr><td>&nbsp;</td></tr><%
        }
        
        %></table><p>Delete messages:<p><%

        WriteButton (BID_ALL);
        WriteButton (BID_SELECTION);

        if (bSystemSent) {
            WriteButton (BID_SYSTEM);
        }

        if (iNumNames > 0) {

            WriteButton (BID_EMPIRE);
            %><select name="SelectedEmpire"><%

            for (j = 0; j < iNumNames; j ++) {
                %><option value="<% Write (pstrNameList[j]); %>"><% Write (pstrNameList[j]); %></option><%
            }

            %></select><%
        }

        t_pCache->FreeData (ppvMessage);
        t_pCache->FreeKeys (piMessageKey);
    }

    break;
    }

default:

    Assert (false);
    break;
}

CloseGamePage();

%>