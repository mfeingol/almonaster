
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "Osal/Algorithm.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Options page
int HtmlRenderer::Render_Options() {

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

	INITIALIZE_EMPIRE

	INITIALIZE_GAME

	IHttpForm* pHttpForm;

	// Handle a submission
	int iErrCode, iOptionPage = 0, iGameClassOptions, iNumEmpires, iDiplomacy;
	unsigned int i;

	Variant vTemp;

	bool bGameStarted = (m_iGameState & STARTED) != 0;

	GameCheck (g_pGameEngine->GetGameClassOptions (m_iGameClass, &iGameClassOptions));
	GameCheck (g_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires));
	GameCheck (g_pGameEngine->GetGameClassDiplomacyLevel (m_iGameClass, &iDiplomacy));

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
	            bRedirectTest = false;
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

	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, AUTO_REFRESH, bUpdate) == OK) {

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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, GAME_REPEATED_BUTTONS, bUpdate) == OK) {

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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, GAME_DISPLAY_TIME, bUpdate) == OK) {

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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, DISPLACE_ENDTURN_BUTTON, bUpdate) == OK) {

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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, COUNTDOWN, bUpdate) == OK) {

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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, MAP_COLORING, bUpdate) == OK) {

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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIP_MAP_COLORING, bUpdate) == OK) {

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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIP_MAP_HIGHLIGHTING, bUpdate) == OK) {

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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SENSITIVE_MAPS, bUpdate) == OK) {

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

	                if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, PARTIAL_MAPS, !bFlag)) == OK) {
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

	                if ((iErrCode = g_pGameEngine->SetEmpireGameProperty(
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

	                if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIPS_ON_MAP_SCREEN, !bFlag)) == OK) {
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

	                if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, SHIPS_ON_PLANETS_SCREEN, !bFlag)) == OK) {
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

	                if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, BUILD_ON_MAP_SCREEN, !bFlag)) == OK) {
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

	                if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, BUILD_ON_PLANETS_SCREEN, !bFlag)) == OK) {
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

	                if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, LOCAL_MAPS_IN_UPCLOSE_VIEWS, !bFlag)) == OK) {
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

	                GameCheck (g_pGameEngine->SetEmpireGameProperty (
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
	            GameCheck (g_pGameEngine->GetEmpireDefaultBuilderPlanet (
	                m_iGameClass,
	                m_iGameNumber,
	                m_iEmpireKey,
	                &iOldValue,
	                &iRealPlanet
	                ));

	            if (iNewValue != iOldValue) {

	                iErrCode = g_pGameEngine->SetEmpireDefaultBuilderPlanet (m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue);
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

	                    if ((iErrCode = g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, REJECT_INDEPENDENT_SHIP_GIFTS, !bFlag)) == OK) {
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

	            GameCheck (g_pGameEngine->GetEmpireDefaultMessageTarget (
	                m_iGameClass,
	                m_iGameNumber,
	                m_iEmpireKey,
	                &iOldValue
	                ));

	            if (iNewValue != iOldValue) {

	                iErrCode = g_pGameEngine->SetEmpireDefaultMessageTarget (m_iGameClass, m_iGameNumber, m_iEmpireKey, iNewValue);
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
	                if (g_pGameEngine->SetEmpireMaxNumSavedGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, 
	                    iNewValue) == OK) {

	                    char pszMessage [256];
	                    sprintf (pszMessage, "Up to %i game messages will be saved", iNewValue);
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
	                if (g_pGameEngine->SetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, IGNORE_BROADCASTS, iNewValue != 0) == OK) {
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
	                    GameCheck (g_pGameEngine->IsEmpireRequestingPause (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bOldPause));

	                    // Only update if we changed the status
	                    if ((iPause != 0) != bOldPause) {

	                        iErrCode = g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
	                        m_pgeLock = NULL;

	                        if (iErrCode != OK || g_pGameEngine->WaitGameWriter (m_iGameClass, m_iGameNumber) != OK) {
	                            AddMessage ("That game no longer exists");
	                            return Redirect (ACTIVE_GAME_LIST);
	                        }

	                        if (iPause != 0) {

	                            GameCheck (g_pGameEngine->RequestPause (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_iGameState));

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

	                            GameCheck (g_pGameEngine->RequestNoPause (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_iGameState));

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

	                        if (g_pGameEngine->SignalGameWriter (m_iGameClass, m_iGameNumber) != OK || 
	                            g_pGameEngine->WaitGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_pgeLock) != OK) {

	                            AddMessage ("That game no longer exists");
	                            return Redirect (ACTIVE_GAME_LIST);
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
	                        GameCheck (g_pGameEngine->IsEmpireRequestingDraw (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bOldDraw));

	                        // Only update if we changed the status
	                        if ((iDraw != 0) != bOldDraw) {

	                            if (iDraw != 0) {

	                                GameCheck (g_pGameEngine->RequestDraw (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_iGameState));

	                                m_iGameOptions |= REQUEST_DRAW;

	                                if (!(m_iGameState & GAME_ENDED)) {
	                                    AddMessage ("You are now requesting a draw");
	                                } else {

	                                    // Release the read lock
	                                    g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
	                                    m_pgeLock = NULL;

	                                    // Take a write lock
	                                    if (g_pGameEngine->WaitGameWriter (m_iGameClass, m_iGameNumber) != OK) {

	                                        // The game ended
	                                        AddMessage ("That game no longer exists");
	                                        return Redirect (ACTIVE_GAME_LIST);

	                                    } else {

	                                        bool bEndGame = false;

	                                        // Try to end the game
	                                        iErrCode = g_pGameEngine->CheckGameForEndConditions (
	                                            m_iGameClass,
	                                            m_iGameNumber,
	                                            NULL,
	                                            &bEndGame
	                                            );

	                                        // Release the write lock
	                                        g_pGameEngine->SignalGameWriter (m_iGameClass, m_iGameNumber);

	                                        // Did the game end because of us?
	                                        if (iErrCode == OK && bEndGame) {
	                                            return Redirect (ACTIVE_GAME_LIST);
	                                        }

	                                        // Get the reader lock again
	                                        if (g_pGameEngine->WaitGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, &m_pgeLock) != OK) {

	                                            // The game ended after all
	                                            AddMessage ("That game no longer exists");
	                                            return Redirect (ACTIVE_GAME_LIST);
	                                        }

	                                        // Refresh game state
	                                        GameCheck (g_pGameEngine->GetGameState (m_iGameClass, m_iGameNumber, &m_iGameState));

	                                        // Proceed...
	                                        AddMessage ("You are now requesting a draw");
	                                    }
	                                }

	                            } else {

	                                GameCheck (g_pGameEngine->RequestNoDraw (m_iGameClass, m_iGameNumber, m_iEmpireKey));
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

	            iErrCode = g_pGameEngine->UpdateGameEmpireNotepad (
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
	                bRedirectTest = false;
	                break;
	            }

	            // Check for search for empires with duplicate IP's
	            if (WasButtonPressed (BID_SEARCHIPADDRESSES)) {

	                bRedirectTest = false;
	                SearchForDuplicateIPAddresses (m_iGameClass, m_iGameNumber);
	                break;
	            }

	            // Check for search for empires with duplicate session ids
	            if (WasButtonPressed (BID_SEARCHSESSIONIDS)) {

	                bRedirectTest = false;
	                SearchForDuplicateSessionIds (m_iGameClass, m_iGameNumber);
	                break;
	            }

	            // Resign, surrender
	            if (m_iGameState & STARTED) {

	                if (WasButtonPressed (BID_RESIGN)) {
	                    g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
	                    m_pgeLock = NULL;
	                    m_iReserved = BID_RESIGN;
	                    return Redirect (QUIT);
	                }
	            }

	            if (!(m_iGameState & STILL_OPEN)) {

	                if (WasButtonPressed (BID_SURRENDER)) {

	                    // Make sure this is allowed
	                    if ((iGameClassOptions & USE_SC30_SURRENDERS) ||

	                        ((g_pGameEngine->GameAllowsDiplomacy (iDiplomacy, SURRENDER) ||
	                         (iGameClassOptions & ONLY_SURRENDER_WITH_TWO_EMPIRES)) &&
	                         iNumEmpires == 2)) {

	                        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
	                        m_pgeLock = NULL;
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

	                bRedirectTest = false;

	                for (i = 0; i < iNumTestMessages; i ++) {

	                    // Get message key
	                    sprintf (pszForm, "MsgKey%i", i);
	                    if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                        goto Redirection;
	                    }
	                    iMessageKey = pHttpForm->GetIntValue();

	                    // Delete message
	                    if (g_pGameEngine->DeleteGameMessage (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey) == OK) {
	                        iDeletedMessages ++;
	                    }
	                }

	            } else {

	                // Check for delete selection
	                if (WasButtonPressed (BID_SELECTION)) {

	                    bRedirectTest = false;

	                    for (i = 0; i < iNumTestMessages; i ++) {

	                        // Get selected status of message's delete checkbox
	                        sprintf (pszForm, "DelChBx%i", i);
	                        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) != NULL) {

	                            // Get message key
	                            sprintf (pszForm, "MsgKey%i", i);
	                            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                                goto Redirection;
	                            }
	                            iMessageKey = pHttpForm->GetIntValue();

	                            // Delete message
	                            if (g_pGameEngine->DeleteGameMessage (m_iGameClass, m_iGameNumber, m_iEmpireKey, iMessageKey) == OK) {
	                                iDeletedMessages ++;
	                            }
	                        }
	                    }

	                } else {

	                    // Check for delete system messages
	                    if (WasButtonPressed (BID_SYSTEM)) {

	                        Variant vFlags;
	                        bRedirectTest = false;

	                        for (i = 0; i < iNumTestMessages; i ++) {

	                            // Get message key
	                            sprintf (pszForm, "MsgKey%i", i);
	                            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                                goto Redirection;
	                            }
	                            iMessageKey = pHttpForm->GetIntValue();

	                            if (g_pGameEngine->GetGameMessageProperty(
	                                m_iGameClass,
	                                m_iGameNumber,
	                                m_iEmpireKey,
	                                iMessageKey,
	                                GameEmpireMessages::Flags,
	                                &vFlags
	                                ) == OK &&

	                                (vFlags.GetInteger() & MESSAGE_SYSTEM) &&

	                                g_pGameEngine->DeleteGameMessage (
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
	                            bRedirectTest = false;

	                            // Get target empire
	                            if ((pHttpForm = m_pHttpRequest->GetForm ("SelectedEmpire")) == NULL) {
	                                goto Redirection;
	                            }
	                            const char* pszSrcEmpire = pHttpForm->GetValue();

	                            for (i = 0; i < iNumTestMessages; i ++) {

	                                sprintf (pszForm, "MsgKey%i", i);
	                                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                                    goto Redirection;
	                                }
	                                iMessageKey = pHttpForm->GetIntValue();

	                                if (g_pGameEngine->GetGameMessageProperty(
	                                    m_iGameClass,
	                                    m_iGameNumber,
	                                    m_iEmpireKey,
	                                    iMessageKey,
	                                    GameEmpireMessages::Source,
	                                    &vSource
	                                    ) == OK &&

	                                    String::StrCmp (vSource.GetCharPtr(), pszSrcEmpire) == 0 &&

	                                    g_pGameEngine->DeleteGameMessage (
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

	GAME_REDIRECT_ON_SUBMIT

	GAME_OPEN

	// Individual page stuff starts here

	bool bFlag;
	int j, iNumNames = 0, iValue;

	if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
	    GameCheck (WriteRatiosString (NULL));
	}

	switch (iOptionPage) {

	case 0:
	    {

	    
	Write ("<input type=\"hidden\" name=\"OptionPage\" value=\"0\"><p><p><table width=\"80%\"><tr><td>Placement of command buttons:</td><td><select name=\"RepeatButtons\">", sizeof ("<input type=\"hidden\" name=\"OptionPage\" value=\"0\"><p><p><table width=\"80%\"><tr><td>Placement of command buttons:</td><td><select name=\"RepeatButtons\">") - 1);
	if (m_bRepeatedButtons) { 
	        
	Write ("<option value=\"0\">At top of screen only</option><option selected value=\"1\">At top and bottom of screen</option>", sizeof ("<option value=\"0\">At top of screen only</option><option selected value=\"1\">At top and bottom of screen</option>") - 1);
	} else { 
	        
	Write ("<option selected value=\"0\">At top of screen only</option><option value=\"1\">At top and bottom of screen</option>", sizeof ("<option selected value=\"0\">At top of screen only</option><option value=\"1\">At top and bottom of screen</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldRepeatButtons\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldRepeatButtons\" value=\"") - 1);
	Write (m_bRepeatedButtons ? 1:0);
	    
	Write ("\"></td></tr><tr><td>Server time display:</td><td><select name=\"TimeDisplay\">", sizeof ("\"></td></tr><tr><td>Server time display:</td><td><select name=\"TimeDisplay\">") - 1);
	if (m_bTimeDisplay) { 
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldTimeDisplay\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldTimeDisplay\" value=\"") - 1);
	Write (m_bTimeDisplay ? 1:0);
	    
	Write ("\"></td></tr><tr><td>End Turn button displacement:</td><td><select name=\"DisplaceEndTurn\">", sizeof ("\"></td></tr><tr><td>End Turn button displacement:</td><td><select name=\"DisplaceEndTurn\">") - 1);
	if (m_iGameOptions & DISPLACE_ENDTURN_BUTTON) { 
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldDisplaceEndTurn\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldDisplaceEndTurn\" value=\"") - 1);
	Write ((m_iGameOptions & DISPLACE_ENDTURN_BUTTON) ? 1:0); 
	Write ("\"></td></tr><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"AutoRefresh\">", sizeof ("\"></td></tr><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"AutoRefresh\">") - 1);
	if (m_iGameOptions & AUTO_REFRESH) {
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	}

	    
	Write ("</select><input type=\"hidden\" name=\"OldAutoRefresh\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldAutoRefresh\" value=\"") - 1);
	Write ((m_iGameOptions & AUTO_REFRESH) ? 1:0); 
	Write ("\"></td></tr><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"Countdown\">", sizeof ("\"></td></tr><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"Countdown\">") - 1);
	if (m_iGameOptions & COUNTDOWN) { 
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldCountdown\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldCountdown\" value=\"") - 1);
	Write ((m_iGameOptions & COUNTDOWN) ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (m_iGameOptions & MAP_COLORING) != 0;

	    
	Write ("<tr><td>Map coloring by diplomatic status:</td><td><select name=\"MapColoring\">", sizeof ("<tr><td>Map coloring by diplomatic status:</td><td><select name=\"MapColoring\">") - 1);
	if (bFlag) {
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldMapColoring\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldMapColoring\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (m_iGameOptions & SHIP_MAP_COLORING) != 0;

	    
	Write ("<tr><td>Ship coloring by diplomatic status:</td><td><select name=\"ShipMapColoring\">", sizeof ("<tr><td>Ship coloring by diplomatic status:</td><td><select name=\"ShipMapColoring\">") - 1);
	if (bFlag) {
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldShipMapColoring\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldShipMapColoring\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (m_iGameOptions & SHIP_MAP_HIGHLIGHTING) != 0;

	    
	Write ("<tr><td>Ship highlighting on map screen:</td><td><select name=\"ShipHighlighting\">", sizeof ("<tr><td>Ship highlighting on map screen:</td><td><select name=\"ShipHighlighting\">") - 1);
	if (bFlag) {
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldShipHighlighting\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldShipHighlighting\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (m_iGameOptions & SENSITIVE_MAPS) != 0;

	    
	Write ("<tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name=\"SensitiveMaps\">", sizeof ("<tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name=\"SensitiveMaps\">") - 1);
	if (bFlag) {
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldSensitiveMaps\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldSensitiveMaps\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (m_iGameOptions & PARTIAL_MAPS) != 0;

	    
	Write ("<tr><td>Partial maps:</td><td><select name=\"PartialMaps\">", sizeof ("<tr><td>Partial maps:</td><td><select name=\"PartialMaps\">") - 1);
	if (bFlag) {
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else { 
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select></td></tr>", sizeof ("</select></td></tr>") - 1);
	GameCheck (g_pGameEngine->GetEmpireGameProperty (
	        m_iGameClass,
	        m_iGameNumber,
	        m_iEmpireKey,
	        GameEmpireData::MiniMaps,
	        &vTemp
	        ));

	    
	Write ("<input type=\"hidden\" name=\"OldMiniMaps\" value=\"", sizeof ("<input type=\"hidden\" name=\"OldMiniMaps\" value=\"") - 1);
	Write (vTemp.GetInteger()); 
	Write ("\"><tr><td>Mini-maps:</td><td><select name=\"MiniMaps\"><option ", sizeof ("\"><tr><td>Mini-maps:</td><td><select name=\"MiniMaps\"><option ") - 1);
	if (vTemp.GetInteger() == MINIMAPS_NEVER) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (MINIMAPS_NEVER); 
	Write ("\">Never show mini-maps</option><option ", sizeof ("\">Never show mini-maps</option><option ") - 1);
	if (vTemp.GetInteger() == MINIMAPS_OPTION) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (MINIMAPS_OPTION); 
	Write ("\">Show mini-maps as an option</option><option ", sizeof ("\">Show mini-maps as an option</option><option ") - 1);
	if (vTemp.GetInteger() == MINIMAPS_PRIMARY) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (MINIMAPS_PRIMARY); 
	Write ("\">Show mini-maps by default</option></select></td></tr>", sizeof ("\">Show mini-maps by default</option></select></td></tr>") - 1);
	iValue = m_iGameOptions & (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN);

	    
	Write ("<tr><td>Display ship menus in planet views:</td><td><select name=\"UpCloseShips\"><option ", sizeof ("<tr><td>Display ship menus in planet views:</td><td><select name=\"UpCloseShips\"><option ") - 1);
	if (iValue == (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN)) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN); 
	Write ("\">Ship menus on both map and planets screens</option><option ", sizeof ("\">Ship menus on both map and planets screens</option><option ") - 1);
	if (iValue == SHIPS_ON_MAP_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_MAP_SCREEN); 
	Write ("\">Ship menus on map screen</option><option ", sizeof ("\">Ship menus on map screen</option><option ") - 1);
	if (iValue == SHIPS_ON_PLANETS_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_PLANETS_SCREEN); 
	Write ("\">Ship menus on planets screen</option><option ", sizeof ("\">Ship menus on planets screen</option><option ") - 1);
	if (iValue == 0) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"0\">No ship menus in planet views</option></select></td></tr>", sizeof ("value=\"0\">No ship menus in planet views</option></select></td></tr>") - 1);
	iValue = m_iGameOptions & (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN);

	    
	Write ("<tr><td>Display build menus in planet views:</td><td><select name=\"UpCloseBuilds\"><option ", sizeof ("<tr><td>Display build menus in planet views:</td><td><select name=\"UpCloseBuilds\"><option ") - 1);
	if (iValue == (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN)) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN); 
	Write ("\">Build menus on both map and planets screens</option><option ", sizeof ("\">Build menus on both map and planets screens</option><option ") - 1);
	if (iValue == BUILD_ON_MAP_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (BUILD_ON_MAP_SCREEN); 
	Write ("\">Build menus on map screen</option><option ", sizeof ("\">Build menus on map screen</option><option ") - 1);
	if (iValue == BUILD_ON_PLANETS_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (BUILD_ON_PLANETS_SCREEN); 
	Write ("\">Build menus on planets screen</option><option ", sizeof ("\">Build menus on planets screen</option><option ") - 1);
	if (iValue == 0) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"0\">No build menus in planet views</option></select></td></tr>", sizeof ("value=\"0\">No build menus in planet views</option></select></td></tr>") - 1);
	bFlag = (m_iGameOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;

	    
	Write ("<tr><td>Display local maps in up-close map views:</td><td><select name=\"LocalMaps\">", sizeof ("<tr><td>Display local maps in up-close map views:</td><td><select name=\"LocalMaps\">") - 1);
	if (bFlag) {
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select></td></tr>", sizeof ("</select></td></tr>") - 1);
	GameCheck (g_pGameEngine->GetEmpireGameProperty (
	        m_iGameClass,
	        m_iGameNumber,
	        m_iEmpireKey,
	        GameEmpireData::GameRatios,
	        &vTemp
	        ));

	    int iGameRatios = vTemp.GetInteger();

	    
	Write ("<tr><td>Display ratios line:</td><td><select name=\"Ratios\"><option", sizeof ("<tr><td>Display ratios line:</td><td><select name=\"Ratios\"><option") - 1);
	if (iGameRatios == RATIOS_DISPLAY_NEVER) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (RATIOS_DISPLAY_NEVER); 
	Write ("\">Never</option><option", sizeof ("\">Never</option><option") - 1);
	if (iGameRatios == RATIOS_DISPLAY_ON_RELEVANT_SCREENS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (RATIOS_DISPLAY_ON_RELEVANT_SCREENS); 
	Write ("\">On relevant game screens</option><option", sizeof ("\">On relevant game screens</option><option") - 1);
	if (iGameRatios == RATIOS_DISPLAY_ALWAYS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (RATIOS_DISPLAY_ALWAYS); 
	Write ("\">On all game screens</option></select></td></tr><tr><td>Default builder planet:</td><td><select name=\"DefaultBuilderPlanet\">", sizeof ("\">On all game screens</option></select></td></tr><tr><td>Default builder planet:</td><td><select name=\"DefaultBuilderPlanet\">") - 1);
	int iRealPlanet;
	    GameCheck (g_pGameEngine->GetEmpireDefaultBuilderPlanet (
	        m_iGameClass,
	        m_iGameNumber,
	        m_iEmpireKey,
	        &iValue,
	        &iRealPlanet
	        ));

	    
	Write ("<option ", sizeof ("<option ") - 1);
	if (iValue == HOMEWORLD_DEFAULT_BUILDER_PLANET) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (HOMEWORLD_DEFAULT_BUILDER_PLANET); 
	Write ("\">Homeworld</option><option ", sizeof ("\">Homeworld</option><option ") - 1);
	if (iValue == LAST_BUILDER_DEFAULT_BUILDER_PLANET) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (LAST_BUILDER_DEFAULT_BUILDER_PLANET); 
	Write ("\">Last builder planet used</option><option ", sizeof ("\">Last builder planet used</option><option ") - 1);
	if (iValue == NO_DEFAULT_BUILDER_PLANET) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (NO_DEFAULT_BUILDER_PLANET); 
	Write ("\">No default builder planet</option>", sizeof ("\">No default builder planet</option>") - 1);
	unsigned int* piBuilderKey = NULL, iNumBuilderKeys;
	    GameCheck (g_pGameEngine->GetBuilderPlanetKeys (
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

	            iErrCode = g_pGameEngine->GetPlanetNameWithCoordinates (
	                m_iGameClass,
	                m_iGameNumber,
	                piBuilderKey[i],
	                pszPlanetName
	                );

	            if (iErrCode == OK) {

	                if (HTMLFilter (pszPlanetName, &strFilter, 0, false) == OK) {

	                    
	Write ("<option ", sizeof ("<option ") - 1);
	if ((unsigned int) iValue == piBuilderKey[i]) {
	                        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	                    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (piBuilderKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength()); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	        }

	        g_pGameEngine->FreeKeys (piBuilderKey);
	        piBuilderKey = NULL;
	    }

	    GameCheck (g_pGameEngine->GetEmpireDefaultMessageTarget (
	        m_iGameClass,
	        m_iGameNumber,
	        m_iEmpireKey,
	        &iValue
	        ));

	    
	Write ("</select></td></tr>", sizeof ("</select></td></tr>") - 1);
	if (iGameClassOptions & INDEPENDENCE) {

	        
	Write ("<tr><td>Independent ship gifts:</td><td><select name=\"IndependentGifts\"><option", sizeof ("<tr><td>Independent ship gifts:</td><td><select name=\"IndependentGifts\"><option") - 1);
	bFlag = (m_iGameOptions & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

	        if (!bFlag) {
	            
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Accept independent ship gifts</option><option", sizeof ("\">Accept independent ship gifts</option><option") - 1);
	if (bFlag) {
	            
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (1); 
	Write ("\">Reject independent ship gifts</option></select></tr>", sizeof ("\">Reject independent ship gifts</option></select></tr>") - 1);
	}

	    
	Write ("<tr><td>Default message target:</td><td><select name=\"MessageTarget\"><option", sizeof ("<tr><td>Default message target:</td><td><select name=\"MessageTarget\"><option") - 1);
	if (iValue == MESSAGE_TARGET_NONE) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_NONE); 
	Write ("\">None</option><option", sizeof ("\">None</option><option") - 1);
	if (iValue == MESSAGE_TARGET_BROADCAST) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_BROADCAST); 
	Write ("\">Broadcast</option><option", sizeof ("\">Broadcast</option><option") - 1);
	if (iValue == MESSAGE_TARGET_WAR) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_WAR); 
	Write ("\">All at War</option>", sizeof ("\">All at War</option>") - 1);
	if (iDiplomacy & TRUCE) {

	        
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == MESSAGE_TARGET_TRUCE) {
	            
	Write (" selected", sizeof (" selected") - 1);
	}
	        
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_TRUCE); 
	Write ("\">All at Truce</option>", sizeof ("\">All at Truce</option>") - 1);
	}

	    if (iDiplomacy & TRADE) {

	        
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == MESSAGE_TARGET_TRADE) {
	            
	Write (" selected", sizeof (" selected") - 1);
	}
	        
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_TRADE); 
	Write ("\">All at Trade</option>", sizeof ("\">All at Trade</option>") - 1);
	}

	    if (iDiplomacy & ALLIANCE) {

	        
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == MESSAGE_TARGET_ALLIANCE) {
	            
	Write (" selected", sizeof (" selected") - 1);
	}
	        
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_ALLIANCE); 
	Write ("\">All at Alliance</option>", sizeof ("\">All at Alliance</option>") - 1);
	}

	    
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == MESSAGE_TARGET_LAST_USED) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_LAST_USED); 
	Write ("\">Last target used</option></select></td></tr><tr><td>Game messages saved:<td valign=\"middle\">", sizeof ("\">Last target used</option></select></td></tr><tr><td>Game messages saved:<td valign=\"middle\">") - 1);
	unsigned int iNumMessages;

	    GameCheck (g_pGameEngine->GetNumGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumMessages)); 
	    if (iNumMessages > 0) {
	        Write (iNumMessages);
	        
	Write ("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp", sizeof ("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp") - 1);
	WriteButton (BID_VIEWMESSAGES);
	    } else {
	        
	Write ("None", sizeof ("None") - 1);
	}
	    
	Write ("</td></tr><tr><td>Maximum saved game messages</td><td><select name=\"MaxSavedMessages\">", sizeof ("</td></tr><tr><td>Maximum saved game messages</td><td><select name=\"MaxSavedMessages\">") - 1);
	Variant vMaxNumMessages;

	    GameCheck (g_pGameEngine->GetEmpireMaxNumSavedGameMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumMessages));
	    GameCheck (g_pGameEngine->GetSystemProperty (SystemData::MaxNumGameMessages, &vMaxNumMessages));
	    
	    unsigned int iMaxNumMessages = vMaxNumMessages.GetInteger();

	    for (i = 0; i < iMaxNumMessages; i += 10) {
	        
	Write ("<option ", sizeof ("<option ") - 1);
	if (i == iNumMessages) {
	            
	Write ("selected ", sizeof ("selected ") - 1);
	}
	        
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	    
	Write ("</select><input type=\"hidden\" name=\"OldMaxSavedMessages\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldMaxSavedMessages\" value=\"") - 1);
	Write (iNumMessages); 
	    
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	bFlag = (m_iGameOptions & IGNORE_BROADCASTS) != 0;

	    
	Write ("<tr><td>Ignore broadcasts:</td><td><select name=\"Ignore\">", sizeof ("<tr><td>Ignore broadcasts:</td><td><select name=\"Ignore\">") - 1);
	if (bFlag) {
	        
	Write ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>", sizeof ("<option selected value=\"1\">Yes</option><option value=\"0\">No</option>") - 1);
	} else {
	        
	Write ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>", sizeof ("<option value=\"1\">Yes</option><option selected value=\"0\">No</option>") - 1);
	} 
	Write ("</select><input type=\"hidden\" name=\"OldIgnore\" value=\"", sizeof ("</select><input type=\"hidden\" name=\"OldIgnore\" value=\"") - 1);
	Write (bFlag ? 1:0); 
	Write ("\"></td></tr><tr><td>Search for empires with duplicate IP addresses:</td><td>", sizeof ("\"></td></tr><tr><td>Search for empires with duplicate IP addresses:</td><td>") - 1);
	WriteButton (BID_SEARCHIPADDRESSES);
	    
	Write ("</td></tr><tr><td>Search for empires with duplicate Session Ids:</td><td>", sizeof ("</td></tr><tr><td>Search for empires with duplicate Session Ids:</td><td>") - 1);
	WriteButton (BID_SEARCHSESSIONIDS);
	    
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	if (bGameStarted) {


	        
	Write ("<tr><td>Request pause:</td><td><select name=\"Pause\"><option ", sizeof ("<tr><td>Request pause:</td><td><select name=\"Pause\"><option ") - 1);
	if (m_iGameOptions & REQUEST_PAUSE) {
	            
	Write ("selected", sizeof ("selected") - 1);
	} 
	Write (" value=\"1\">Yes</option><option ", sizeof (" value=\"1\">Yes</option><option ") - 1);
	if (!(m_iGameOptions & REQUEST_PAUSE)) {
	            
	Write ("selected", sizeof ("selected") - 1);
	} 
	Write (" value=\"0\">No</option></select>", sizeof (" value=\"0\">No</option></select>") - 1);
	GameCheck (g_pGameEngine->GetNumEmpiresRequestingPause (m_iGameClass, m_iGameNumber, &i));

	        
	Write (" <strong>", sizeof (" <strong>") - 1);
	Write (i); 
	Write ("</strong> of <strong>", sizeof ("</strong> of <strong>") - 1);
	Write (iNumEmpires); 
	Write ("</strong> empire", sizeof ("</strong> empire") - 1);
	if (iNumEmpires != 1) {
	            
	Write ("s", sizeof ("s") - 1);
	}

	        if (i == 1 || iNumEmpires == 1) {
	            
	Write (" is", sizeof (" is") - 1);
	} else {
	            
	Write (" are", sizeof (" are") - 1);
	}

	        
	Write (" requesting a pause</td></tr>", sizeof (" requesting a pause</td></tr>") - 1);
	}

	    if (bGameStarted && (iGameClassOptions & ALLOW_DRAW)) {

	        
	Write ("<tr><td>Request draw:</td><td><select name=\"Draw\"><option ", sizeof ("<tr><td>Request draw:</td><td><select name=\"Draw\"><option ") - 1);
	if (m_iGameOptions & REQUEST_DRAW) {
	            
	Write ("selected", sizeof ("selected") - 1);
	} 
	Write (" value=\"1\">Yes</option><option ", sizeof (" value=\"1\">Yes</option><option ") - 1);
	if (!(m_iGameOptions & REQUEST_DRAW)) {
	            
	Write ("selected", sizeof ("selected") - 1);
	} 
	Write (" value=\"0\">No</option></select>", sizeof (" value=\"0\">No</option></select>") - 1);
	GameCheck (g_pGameEngine->GetNumEmpiresRequestingDraw (m_iGameClass, m_iGameNumber, &i));

	        
	Write (" <strong>", sizeof (" <strong>") - 1);
	Write (i); 
	Write ("</strong> of <strong>", sizeof ("</strong> of <strong>") - 1);
	Write (iNumEmpires); 
	Write ("</strong> empire", sizeof ("</strong> empire") - 1);
	if (iNumEmpires != 1) {
	        
	Write ("s", sizeof ("s") - 1);
	}

	        if (i == 1 || iNumEmpires == 1) {
	            
	Write (" is", sizeof (" is") - 1);
	} else {
	            
	Write (" are", sizeof (" are") - 1);
	}
	        
	Write (" requesting a draw</td></tr>", sizeof (" requesting a draw</td></tr>") - 1);
	}


	    
	Write ("<tr><td align=\"top\">Keep game notes here:</td><td><textarea name=\"Notepad\" rows=\"8\" cols=\"50\" wrap=\"virtual\">", sizeof ("<tr><td align=\"top\">Keep game notes here:</td><td><textarea name=\"Notepad\" rows=\"8\" cols=\"50\" wrap=\"virtual\">") - 1);
	Variant vNotepad;
	    GameCheck (g_pGameEngine->GetEmpireNotepad (m_iGameClass, m_iGameNumber, m_iEmpireKey, &vNotepad));
	    Write (vNotepad.GetCharPtr());
	    
	Write ("</textarea></td>", sizeof ("</textarea></td>") - 1);
	if (m_iGameState & STARTED) {

	        
	Write ("<tr><td>Resign from the game:<br><em>(This option will dismantle all your ships and leave your empire automatically ready for updates. You will not be able to log back in, and your empire will either be nuked or fall into ruin)</em></td><td>", sizeof ("<tr><td>Resign from the game:<br><em>(This option will dismantle all your ships and leave your empire automatically ready for updates. You will not be able to log back in, and your empire will either be nuked or fall into ruin)</em></td><td>") - 1);
	WriteButton (BID_RESIGN);
	        
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	    if (!(m_iGameState & STILL_OPEN)) {

	        if ((iGameClassOptions & USE_SC30_SURRENDERS) ||

	            ((g_pGameEngine->GameAllowsDiplomacy (iDiplomacy, SURRENDER) ||
	             (iGameClassOptions & ONLY_SURRENDER_WITH_TWO_EMPIRES)) &&
	             iNumEmpires == 2)) {

	            
	Write ("<tr><td>Surrender from the game:", sizeof ("<tr><td>Surrender from the game:") - 1);
	if (iGameClassOptions & USE_SC30_SURRENDERS) {

	                
	Write ("<br><em>(This option will immediately remove your empire from the game. If an empire colonizes your old homeworld, or if the game ends with a single empire as the winner, then that empire will be given the nuke.)</em>", sizeof ("<br><em>(This option will immediately remove your empire from the game. If an empire colonizes your old homeworld, or if the game ends with a single empire as the winner, then that empire will be given the nuke.)</em>") - 1);
	} else {

	                
	Write ("<br><em>(This option will immediately end the game. Your empire will be considered  to be have been nuked by your foe.)</em>", sizeof ("<br><em>(This option will immediately end the game. Your empire will be considered  to be have been nuked by your foe.)</em>") - 1);
	}

	            
	Write ("</td><td>", sizeof ("</td><td>") - 1);
	WriteButton (BID_SURRENDER);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}
	    }

	    
	Write ("</tr></table><p>", sizeof ("</tr></table><p>") - 1);
	WriteButton (BID_CANCEL);

	    }
	    break;

	case 1:

	    {
	    unsigned int* piMessageKey = NULL, iNumMessages;
	    Variant** ppvMessage = NULL;

	    GameCheck (g_pGameEngine->GetSavedGameMessages (
	        m_iGameClass,
	        m_iGameNumber,
	        m_iEmpireKey,
	        &piMessageKey,
	        &ppvMessage, 
	        &iNumMessages
	        ));

	    
	Write ("<input type=\"hidden\" name=\"OptionPage\" value=\"1\">", sizeof ("<input type=\"hidden\" name=\"OptionPage\" value=\"1\">") - 1);
	if (iNumMessages == 0) {
	        
	Write ("<p>You have no saved game messages", sizeof ("<p>You have no saved game messages") - 1);
	} else {

	        // Sort
	        UTCTime* ptTime = (UTCTime*) StackAlloc (iNumMessages * sizeof (UTCTime));
	        int* piIndex = (int*) StackAlloc (iNumMessages * sizeof (int));

	        for (i = 0; i < iNumMessages; i ++) {
	            piIndex[i] = i;
	            ptTime[i] = ppvMessage[i][GameEmpireMessages::TimeStamp].GetInteger64();
	        }

	        Algorithm::QSortTwoDescending<UTCTime, int> (ptTime, piIndex, iNumMessages);

	        // Display
	        String* pstrNameList = new String [iNumMessages];
	        if (pstrNameList == NULL) {
	            
	Write ("<p>The server is out of memory", sizeof ("<p>The server is out of memory") - 1);
	} else {

	            Algorithm::AutoDelete<String> autopstrNameList (pstrNameList, true);

	            
	Write ("<p>You have <strong>", sizeof ("<p>You have <strong>") - 1);
	Write (iNumMessages); 
	Write ("</strong> saved game message", sizeof ("</strong> saved game message") - 1);
	if (iNumMessages != 1) { 
	                
	Write ("s", sizeof ("s") - 1);
	}
	            
	Write (":<p><table width=\"45%\"><input type=\"hidden\" name=\"NumSavedGameMessages\" value=\"", sizeof (":<p><table width=\"45%\"><input type=\"hidden\" name=\"NumSavedGameMessages\" value=\"") - 1);
	Write (iNumMessages); 
	Write ("\">", sizeof ("\">") - 1);
	const char* pszFontColor = NULL;
	            char pszDate [OS::MaxDateLength];

	            bool bSystemSent = false;

	            for (i = 0; i < iNumMessages; i ++) {

	                int iFlags = ppvMessage[piIndex[i]][GameEmpireMessages::Flags].GetInteger();

	                const char* pszSender = ppvMessage[piIndex[i]][GameEmpireMessages::Source].GetCharPtr();

	                
	Write ("<input type=\"hidden\" name=\"MsgKey", sizeof ("<input type=\"hidden\" name=\"MsgKey") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (piMessageKey[piIndex[i]]); 
	                
	Write ("\"><input type=\"hidden\" name=\"MsgSrc", sizeof ("\"><input type=\"hidden\" name=\"MsgSrc") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (pszSender);
	                
	Write ("\"><tr><td><strong>Time: </strong> ", sizeof ("\"><tr><td><strong>Time: </strong> ") - 1);
	iErrCode = Time::GetDateString (ppvMessage[piIndex[i]][GameEmpireMessages::TimeStamp].GetInteger64(), pszDate);
	                if (iErrCode != OK) {
	                    
	Write ("Could not read date", sizeof ("Could not read date") - 1);
	} else {
	                    Write (pszDate);
	                }

	                
	Write ("<br><strong>Sender: </strong>", sizeof ("<br><strong>Sender: </strong>") - 1);
	if (iFlags & MESSAGE_SYSTEM) { 

	                    bSystemSent = true;
	                    
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (SYSTEM_MESSAGE_SENDER); 
	Write ("</strong>", sizeof ("</strong>") - 1);
	} else {

	                    
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (pszSender); 
	Write ("</strong>", sizeof ("</strong>") - 1);
	// Find name in lists
	                    bFlag = false;
	                    for (j = 0; j < iNumNames; j ++) {
	                        if (pstrNameList[j].Equals (pszSender)) {
	                            bFlag = true;
	                            break;
	                        }
	                    }

	                    // Add name to list if not found
	                    if (!bFlag) {
	                        pstrNameList[iNumNames] = pszSender;
	                        iNumNames ++;
	                    }
	                }

	                if (iFlags & MESSAGE_BROADCAST) {
	                    
	Write (" (broadcast)", sizeof (" (broadcast)") - 1);
	pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
	                } else {
	                    pszFontColor = m_vPrivateMessageColor.GetCharPtr();
	                }

	                
	Write ("<br><strong>Delete: </strong><input type=\"checkbox\" name=\"DelChBx", sizeof ("<br><strong>Delete: </strong><input type=\"checkbox\" name=\"DelChBx") - 1);
	Write (i); 
	                
	Write ("\"></td></tr><tr><td><font size=\"", sizeof ("\"></td></tr><tr><td><font size=\"") - 1);
	Write (DEFAULT_MESSAGE_FONT_SIZE);
	                
	Write ("\" face=\"", sizeof ("\" face=\"") - 1);
	Write (DEFAULT_MESSAGE_FONT); 
	Write ("\"", sizeof ("\"") - 1);
	// Game messages from the system suffer from no special coloring
	                if (!(iFlags & MESSAGE_SYSTEM)) {
	                    
	Write (" color=\"#", sizeof (" color=\"#") - 1);
	Write (pszFontColor); 
	Write ("\"", sizeof ("\"") - 1);
	}
	                
	Write (">", sizeof (">") - 1);
	WriteFormattedMessage (ppvMessage[piIndex[i]][0].GetCharPtr());

	                
	Write ("</font></td></tr><tr><td>&nbsp;</td></tr>", sizeof ("</font></td></tr><tr><td>&nbsp;</td></tr>") - 1);
	} 
	Write ("</table><p>Delete messages:<p>", sizeof ("</table><p>Delete messages:<p>") - 1);
	WriteButton (BID_ALL);
	            WriteButton (BID_SELECTION);

	            if (bSystemSent) {
	                WriteButton (BID_SYSTEM);
	            }

	            if (iNumNames > 0) {

	                WriteButton (BID_EMPIRE);
	                
	Write ("<select name=\"SelectedEmpire\">", sizeof ("<select name=\"SelectedEmpire\">") - 1);
	for (j = 0; j < iNumNames; j ++) {
	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (pstrNameList[j]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pstrNameList[j]); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

	                
	Write ("</select>", sizeof ("</select>") - 1);
	}

	            g_pGameEngine->FreeData (ppvMessage);
	            g_pGameEngine->FreeKeys (piMessageKey);
	        }
	    }

	    break;
	    }

	default:

	    Assert (false);
	    break;
	}

	GAME_CLOSE


}