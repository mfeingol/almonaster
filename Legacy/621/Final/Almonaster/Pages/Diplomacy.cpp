
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Diplomacy page
int HtmlRenderer::Render_Diplomacy() {

	// Almonaster
	// Copyright (c) 1998-2004 Max Attar Feingold (maf6@cornell.edu)
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

	const unsigned int piColumns[] = {
	    GameEmpireDiplomacy::EmpireKey,
	    GameEmpireDiplomacy::DipOffer,
	    GameEmpireDiplomacy::CurrentStatus,
	};

	bool bGameStarted = (m_iGameState & STARTED) != 0;

	IHttpForm* pHttpForm;

	int i, j, iErrCode;
	bool bIgnore;
	const char* pszRedrawMessage = NULL;

	// Read default message target
	int iDefaultMessageTarget;
	iErrCode = g_pGameEngine->GetEmpireDefaultMessageTarget (
	    m_iGameClass,
	    m_iGameNumber,
	    m_iEmpireKey,
	    &iDefaultMessageTarget
	    );

	if (iErrCode != OK) {
	    Assert (false);
	    AddMessage ("Could not read default message target; error was ");
	    AppendMessage (iErrCode);
	    iDefaultMessageTarget = MESSAGE_TARGET_BROADCAST;
	}
	Assert (iDefaultMessageTarget >= MESSAGE_TARGET_NONE && iDefaultMessageTarget <= MESSAGE_TARGET_LAST_USED);

	if (m_bOwnPost && !m_bRedirection) {

	    // Handle submissions
	    int iOldValue, iNewValue;

	    // Handle messages:  messages will only be sent if the two empires in question are still in the game,
	    // or, in the case of a broadcast, if the sender is.  This is handled in the gameengine

	    // Make sure the sendmessage button was pressed
	    if (WasButtonPressed (BID_SENDMESSAGE)) {

	        bRedirectTest = false;

	        const char* pszSentMessage;
	        if (
	            (pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL ||
	            String::IsBlank ((pszSentMessage = pHttpForm->GetValue()))
	            ) {
	            AddMessage ("Your message was blank");
	        } else {

	            // See who was in the target list
	            if ((pHttpForm = m_pHttpRequest->GetForm ("MessageTarget")) == NULL) {
	                AddMessage ("Your message had no destination");
	                pszRedrawMessage = pszSentMessage;
	            } else {

	                // Parse target list
	                bool bBroadcast = false, bWar = false, bTruce = false, bTrade = false, bAlliance = false;
	                int iNumTargets = pHttpForm->GetNumForms(), iNumEmpires;

	                Assert (iNumTargets > 0);

	                iErrCode = g_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires);
	                if (iErrCode != OK) {
	                    AddMessage ("An error occurred reading data from the database");
	                    goto Redirection;
	                }

	                unsigned int* piTargetKey = (unsigned int*) StackAlloc (iNumTargets * sizeof (unsigned int));
	                int* piDelayEmpireKey = (int*) StackAlloc (iNumEmpires * sizeof (int));
	                int iDelayEmpireKeys = 0;

	                int iLastMessageTargetMask = 0;
	                int* piLastMessageTargetKeyArray = (int*) StackAlloc (iNumEmpires * sizeof (int));
	                int iNumLastUsed = 0;

	                int iCounter = 0;

	                for (i = 0; i < iNumTargets && !bBroadcast; i ++) {

	                    piTargetKey[iCounter] = pHttpForm->GetForm(i)->GetUIntValue();

	                    switch (piTargetKey[iCounter]) {

	                    case NO_KEY:
	                        bBroadcast = true;
	                        iLastMessageTargetMask |= MESSAGE_TARGET_BROADCAST_MASK;
	                        break;

	                    case ALL_WAR:
	                        bWar = true;
	                        iLastMessageTargetMask |= MESSAGE_TARGET_WAR_MASK;
	                        break;

	                    case ALL_TRUCE:
	                        bTruce = true;
	                        iLastMessageTargetMask |= MESSAGE_TARGET_TRUCE_MASK;
	                        break;

	                    case ALL_TRADE:
	                        bTrade = true;
	                        iLastMessageTargetMask |= MESSAGE_TARGET_TRADE_MASK;
	                        break;

	                    case ALL_ALLIANCE:
	                        bAlliance = true;
	                        iLastMessageTargetMask |= MESSAGE_TARGET_ALLIANCE_MASK;
	                        break;

	                    default:
	                        iLastMessageTargetMask |= MESSAGE_TARGET_INDIVIDUALS;
	                        piDelayEmpireKey[iDelayEmpireKeys ++] = piTargetKey[iCounter ++];
	                        break;
	                    }
	                }

	                if (bBroadcast) {

	                    iErrCode = g_pGameEngine->BroadcastGameMessage (
	                        m_iGameClass, 
	                        m_iGameNumber, 
	                        pszSentMessage,
	                        m_iEmpireKey, 
	                        MESSAGE_BROADCAST
	                        );

	                    switch (iErrCode) {

	                    case OK:
	                        AddMessage ("Your message was broadcast to everyone");
	                        break;

	                    case ERROR_CANNOT_SEND_MESSAGE:

	                        AddMessage ("Your empire is not allowed to broadcast messages");
	                        break;

	                    case ERROR_STRING_IS_TOO_LONG:

	                        AddMessage ("Your message was too long");
	                        break;

	                    case ERROR_EMPIRE_IS_NOT_IN_GAME:

	                        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber, m_iEmpireKey, m_pgeLock);
	                        m_pgeLock = NULL;
	                        AddMessage ("Your empire is not in that game");
	                        break;

	                    default:

	                        {
	                        char pszMessage [256];
	                        sprintf (pszMessage, "Error %i occurred sending your message", iErrCode);
	                        AddMessage (pszMessage);
	                        }

	                        break;
	                    }

	                } else {

	                    bool bDipFilter = bWar || bTruce || bTrade || bAlliance;

	                    // Filter out empires on war, truce, trade, alliance lists
	                    if (bDipFilter) {

	                        int iCurrent, iNumWar, iNumTruce, iNumTrade, iNumAlliance, 
	                            * piWar, * piTruce, * piTrade, * piAlliance;
	                        bool bDelete;

	                        for (i = 0; i < iDelayEmpireKeys; i ++) {

	                            iErrCode = g_pGameEngine->GetDiplomaticStatus (
	                                m_iGameClass,
	                                m_iGameNumber,
	                                m_iEmpireKey,
	                                piDelayEmpireKey[i],
	                                NULL,
	                                NULL,
	                                &iCurrent,
	                                NULL
	                                );

	                            if (iErrCode != OK) {
	                                // Remove from list
	                                piDelayEmpireKey[i] = piDelayEmpireKey[-- iDelayEmpireKeys];
	                                continue;
	                            }

	                            switch (iCurrent) {
	                            case WAR:
	                                bDelete = bWar;
	                                break;
	                            case TRUCE:
	                                bDelete = bTruce;
	                                break;
	                            case TRADE:
	                                bDelete = bTrade;
	                                break;
	                            case ALLIANCE:
	                                bDelete = bAlliance;
	                                break;
	                            default:
	                                bDelete = true;
	                                Assert (false);
	                            }

	                            if (bDelete) {
	                                // Remove from list
	                                piDelayEmpireKey[i] = piDelayEmpireKey[-- iDelayEmpireKeys];
	                            } else {
	                                // Save last used
	                                piLastMessageTargetKeyArray[iNumLastUsed ++] = piDelayEmpireKey[i];
	                            }
	                        }

	                        piWar = (int*) StackAlloc (iNumEmpires * sizeof (int) * 4);
	                        piTruce = piWar + iNumEmpires;
	                        piTrade = piTruce + iNumEmpires;
	                        piAlliance = piTrade + iNumEmpires;

	                        // Add all lists
	                        iErrCode = g_pGameEngine->GetEmpiresAtDiplomaticStatus (
	                            m_iGameClass,
	                            m_iGameNumber,
	                            m_iEmpireKey,
	                            piWar,
	                            &iNumWar, 
	                            piTruce,
	                            &iNumTruce,
	                            piTrade,
	                            &iNumTrade,
	                            piAlliance,
	                            &iNumAlliance
	                            );

	                        if (iErrCode == OK) {

	                            if (bWar) {
	                                for (i = 0; i < iNumWar; i ++) {
	                                    piDelayEmpireKey[iDelayEmpireKeys ++] = piWar[i];
	                                }
	                            }

	                            if (bTruce) {
	                                for (i = 0; i < iNumTruce; i ++) {
	                                    piDelayEmpireKey[iDelayEmpireKeys ++] = piTruce[i];
	                                }
	                            }

	                            if (bTrade) {
	                                for (i = 0; i < iNumTrade; i ++) {
	                                    piDelayEmpireKey[iDelayEmpireKeys ++] = piTrade[i];
	                                }
	                            }

	                            if (bAlliance) {
	                                for (i = 0; i < iNumAlliance; i ++) {
	                                    piDelayEmpireKey[iDelayEmpireKeys ++] = piAlliance[i];
	                                }
	                            }
	                        }
	                    }

	                    String strYes, strIgnore, strNot;
	                    Variant vTheEmpireName;

	                    for (i = 0; i < iDelayEmpireKeys; i ++) {

	                        iErrCode = g_pGameEngine->GetEmpireName (piDelayEmpireKey[i], &vTheEmpireName);
	                        if (iErrCode != OK) {
	                            continue;
	                        }

	                        iErrCode = g_pGameEngine->SendGameMessage (
	                            m_iGameClass,
	                            m_iGameNumber,
	                            piDelayEmpireKey[i],
	                            pszSentMessage,
	                            m_iEmpireKey,
	                            0,
	                            NULL_TIME
	                            );

	                        if (iErrCode == ERROR_STRING_IS_TOO_LONG) {
	                            AddMessage ("Your message was too long");
	                            break;
	                        }

	                        switch (iErrCode) {
	                        case OK:
	                            if (strYes.IsBlank()) {
	                                strYes += vTheEmpireName.GetCharPtr();
	                            } else {
	                                strYes += ", ";
	                                strYes += vTheEmpireName.GetCharPtr();
	                            }

	                            if (!bDipFilter) {
	                                // Save last used
	                                piLastMessageTargetKeyArray[iNumLastUsed ++] = piDelayEmpireKey[i];
	                            }

	                            break;

	                        case ERROR_EMPIRE_IS_IGNORING_SENDER:

	                            if (strIgnore.IsBlank()) {
	                                strIgnore = vTheEmpireName.GetCharPtr();
	                            } else {
	                                strIgnore += ", ";
	                                strIgnore += vTheEmpireName.GetCharPtr();
	                            }
	                            break;

	                        default:

	                            if (strNot.IsBlank()) {
	                                strNot = vTheEmpireName.GetCharPtr();
	                            } else {
	                                strNot += ", ";
	                                strNot += vTheEmpireName.GetCharPtr();
	                            }
	                            break;
	                        }
	                    }

	                    if (i == iDelayEmpireKeys) {

	                        if (!strYes.IsBlank()) {
	                            AddMessage ("Your message was sent to ");
	                            AppendMessage (strYes);
	                        }

	                        if (!strIgnore.IsBlank()) {
	                            AddMessage ("You are being ignored by ");
	                            AppendMessage (strIgnore);
	                        }

	                        if (!strNot.IsBlank()) {
	                            AddMessage ("Your message could not be sent to ");
	                            AppendMessage (strNot);
	                        }

	                        if (i == 0) {
	                            AddMessage ("Your message was sent to no one");
	                        }
	                    }
	                }

	                // Update last used, if necessary
	                if (iDefaultMessageTarget == MESSAGE_TARGET_LAST_USED) {

	                    iErrCode = g_pGameEngine->SetLastUsedMessageTarget (
	                        m_iGameClass,
	                        m_iGameNumber,
	                        m_iEmpireKey,
	                        iLastMessageTargetMask,
	                        iNumLastUsed == 0 ? NULL : piLastMessageTargetKeyArray,
	                        iNumLastUsed
	                        );

	                    if (iErrCode != OK) {
	                        AddMessage ("Could not set last used message target; error was ");
	                        AppendMessage (iErrCode);
	                    }
	                }
	            }
	        }
	    }

	    // The gameengine accepts changes to diplomatic status only if they are legal at submission time
	    if (bGameStarted) {

	        if ((pHttpForm = m_pHttpRequest->GetForm ("NumKnownEmpires")) == NULL) {
	            goto Redirection;
	        }
	        int iSelectedDip, iDipOffer, iFoeKey, iNumDiplomaticEmpires = pHttpForm->GetIntValue();

	        char pszForm [256];

	        for (i = 0; i < iNumDiplomaticEmpires; i ++) {

	            // Get foe key
	            sprintf (pszForm, "FoeKey%i", i);
	            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                goto Redirection;
	            }
	            iFoeKey = pHttpForm->GetIntValue();

	            // Ignore
	            sprintf (pszForm, "Ignore%i", i);
	            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            iErrCode = g_pGameEngine->GetEmpireIgnoreMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, iFoeKey, &bIgnore);

	            if (iErrCode == OK) {

	                iOldValue = bIgnore ? 1:0;

	                if (iOldValue != iNewValue) {

	                    // Best effort
	                    iErrCode = g_pGameEngine->SetEmpireIgnoreMessages (
	                        m_iGameClass,
	                        m_iGameNumber,
	                        m_iEmpireKey,
	                        iFoeKey,
	                        iNewValue != 0
	                        );
	                }
	            }

	            // Get previously selected dip option
	            sprintf (pszForm, "SelectedDip%i", i);
	            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                goto Redirection;
	            }
	            iSelectedDip = pHttpForm->GetIntValue();

	            // Get selected dip option
	            sprintf (pszForm, "DipOffer%i", i);
	            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                goto Redirection;
	            }
	            iDipOffer = pHttpForm->GetIntValue();

	            // Only update if we changed the selected option
	            if (iSelectedDip != iDipOffer) {

	                // Best effort
	                iErrCode = g_pGameEngine->UpdateDiplomaticOffer (
	                    m_iGameClass,
	                    m_iGameNumber,
	                    m_iEmpireKey,
	                    iFoeKey,
	                    iDipOffer
	                    );
	            }
	        }
	    }
	}

	GAME_REDIRECT_ON_SUBMIT

	GAME_OPEN

	// Individual page stuff starts here

	if (m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
	    GameCheck (WriteRatiosString (NULL));
	}


	Write ("<p>", sizeof ("<p>") - 1);
	UTCTime tLastLogin, tCurrentTime;
	int iGameOptions, iNumUpdatesIdle, iEcon, iMil, iWins, iNukes, iNuked, iDraws, iMaxEcon, 
	    iMaxMil, iNumTruces, iNumTrades, iNumAlliances;
	float fMil;
	Variant vIPAddress;

	Time::GetTime (&tCurrentTime);

	Variant** ppvEmpireData = NULL;
	unsigned int* piProxyEmpireKey = NULL;

	bool bSubjective = false;

	int piDipKey [NUM_DIP_LEVELS], iSelected = 0, iNumOptions = 0, iSelectedIndex, iWeOffer, 
	    iTheyOffer, iCurrentStatus, iKnownEmpireKey, iNumKnownEmpires = 0, iAlienKey, iActiveEmpires,
	    iRuins, iSec, iMin, iHour, iDay, iMonth, iYear, * piStatus = NULL, * piIndex = NULL, iIndex;

	DayOfWeek day;
	UTCTime tCreated;

	char pszCreated [OS::MaxDateLength];

	bool bPrivateMessages;

	Variant vKnownEmpireName, vTemp;

	GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, m_iGameClass, m_iGameNumber, m_iEmpireKey);

	const char* pszTableColor = m_vTableColor.GetCharPtr();
	const char* pszGood = m_vGoodColor.GetCharPtr();
	const char* pszBad = m_vBadColor.GetCharPtr();

	char pszProfile [MAX_EMPIRE_NAME_LENGTH + 128];

	IDatabase* pDatabase = g_pGameEngine->GetDatabase();
	IReadTable* pGameEmpireTable = NULL, * pSystemEmpireDataTable = NULL;

	GAME_EMPIRE_DATA (pszEmpireData, m_iGameClass, m_iGameNumber, m_iEmpireKey);

	iErrCode = pDatabase->GetTableForReading (
	    pszEmpireData, 
	    &pGameEmpireTable
	    );
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pGameEmpireTable->ReadData (GameEmpireData::LastLogin, &tLastLogin);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pGameEmpireTable->ReadData (GameEmpireData::Options, &iGameOptions);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pGameEmpireTable->ReadData (GameEmpireData::NumUpdatesIdle, &iNumUpdatesIdle);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pGameEmpireTable->ReadData (GameEmpireData::Econ, &iEcon);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pGameEmpireTable->ReadData (GameEmpireData::Mil, &fMil);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pGameEmpireTable->ReadData (GameEmpireData::NumTruces, &iNumTruces);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pGameEmpireTable->ReadData (GameEmpireData::NumTrades, &iNumTrades);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pGameEmpireTable->ReadData (GameEmpireData::NumAlliances, &iNumAlliances);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	SafeRelease (pGameEmpireTable);

	iErrCode = pDatabase->GetTableForReading (
	    SYSTEM_EMPIRE_DATA, 
	    &pSystemEmpireDataTable
	    );
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::Wins, &iWins);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::Nukes, &iNukes);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::Nuked, &iNuked);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::Draws, &iDraws);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::Ruins, &iRuins);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::MaxEcon, &iMaxEcon);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::MaxMil, &iMaxMil);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::CreationTime, &tCreated);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = pSystemEmpireDataTable->ReadData (m_iEmpireKey, SystemEmpireData::IPAddress, &vIPAddress);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	SafeRelease (pSystemEmpireDataTable);

	Time::GetDate (tCreated, &iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);
	sprintf (pszCreated, "%s %i %i", Time::GetAbbreviatedMonthName (iMonth), iDay, iYear);


	iMil = g_pGameEngine->GetMilitaryValue (fMil);

	if (bGameStarted) {

	    bool bVisible;
	    int iMaxNumTruces, iMaxNumTrades, iMaxNumAlliances, m_iGameClassOptions, m_iGameClassDip;

	    iErrCode = g_pGameEngine->GetGameClassVisibleDiplomacy(m_iGameClass, &bVisible);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = g_pGameEngine->DoesGameClassHaveSubjectiveViews (m_iGameClass, &bSubjective);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = g_pGameEngine->GetMaxNumDiplomacyPartners (m_iGameClass, m_iGameNumber, TRUCE, &iMaxNumTruces);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = g_pGameEngine->GetMaxNumDiplomacyPartners (m_iGameClass, m_iGameNumber, TRADE, &iMaxNumTrades);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = g_pGameEngine->GetMaxNumDiplomacyPartners (m_iGameClass, m_iGameNumber, ALLIANCE, &iMaxNumAlliances);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = g_pGameEngine->GetGameClassOptions (m_iGameClass, &m_iGameClassOptions);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = g_pGameEngine->GetGameClassDiplomacyLevel (m_iGameClass, &m_iGameClassDip);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    
	Write ("<p><table width=\"80%\"><tr><th bgcolor=\"", sizeof ("<p><table width=\"80%\"><tr><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Diplomatic Settings</th><th bgcolor=\"", sizeof ("\">Diplomatic Settings</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Econ and Mil views</th>", sizeof ("\">Econ and Mil views</th>") - 1);
	if (m_iGameClassDip & TRUCE) {
	        
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Truces</th>", sizeof ("\">Truces</th>") - 1);
	}
	    if (m_iGameClassDip & TRADE) {
	        
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Trades</th>", sizeof ("\">Trades</th>") - 1);
	}
	    if (m_iGameClassDip & ALLIANCE) {
	        
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Alliances</th>", sizeof ("\">Alliances</th>") - 1);
	}
	    if (m_iGameClassDip & SURRENDER) {
	        
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Surrenders</th>", sizeof ("\">Surrenders</th>") - 1);
	}

	    
	Write ("</tr><tr><td align=\"center\">", sizeof ("</tr><tr><td align=\"center\">") - 1);
	if (bVisible) {
	        
	Write ("Visible", sizeof ("Visible") - 1);
	} else {
	        
	Write ("Not visible", sizeof ("Not visible") - 1);
	}
	    
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	if (bSubjective) {
	        
	Write ("Subjective", sizeof ("Subjective") - 1);
	} else {
	        
	Write ("Objective", sizeof ("Objective") - 1);
	}
	    
	Write ("</td>", sizeof ("</td>") - 1);
	if (m_iGameClassDip & TRUCE) {

	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	switch (iMaxNumTruces) {

	        case UNRESTRICTED_DIPLOMACY:

	            
	Write ("Unrestricted", sizeof ("Unrestricted") - 1);
	break;

	        case FAIR_DIPLOMACY:

	            
	Write ("Fair truces", sizeof ("Fair truces") - 1);
	break;

	        default:

	            
	Write ("Limit of <strong>", sizeof ("Limit of <strong>") - 1);
	Write (iMaxNumTruces); 
	Write ("</strong> truce", sizeof ("</strong> truce") - 1);
	if (iMaxNumTruces != 1) { 
	Write ("s", sizeof ("s") - 1);
	}
	            break;
	        }
	        
	Write (" (", sizeof (" (") - 1);
	Write (iNumTruces); 
	Write (")</td>", sizeof (")</td>") - 1);
	}

	    if (m_iGameClassDip & TRADE) {

	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	switch (iMaxNumTrades) {

	        case UNRESTRICTED_DIPLOMACY:

	            
	Write ("Unrestricted", sizeof ("Unrestricted") - 1);
	break;

	        case FAIR_DIPLOMACY:

	            
	Write ("Fair trades", sizeof ("Fair trades") - 1);
	break;

	        default:

	            
	Write ("Limit of <strong>", sizeof ("Limit of <strong>") - 1);
	Write (iMaxNumTrades); 
	Write ("</strong> trade", sizeof ("</strong> trade") - 1);
	if (iMaxNumTrades != 1) { 
	Write ("s", sizeof ("s") - 1);
	}
	            break;
	        }
	        
	Write (" (", sizeof (" (") - 1);
	Write (iNumTrades); 
	Write (")</td>", sizeof (")</td>") - 1);
	}

	    if (m_iGameClassDip & ALLIANCE) {

	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	switch (iMaxNumAlliances) {

	        case UNRESTRICTED_DIPLOMACY:

	            
	Write ("Unrestricted", sizeof ("Unrestricted") - 1);
	break;

	        case FAIR_DIPLOMACY:

	            
	Write ("Fair alliances", sizeof ("Fair alliances") - 1);
	break;

	        default:

	            
	Write ("Limit of <strong>", sizeof ("Limit of <strong>") - 1);
	Write (iMaxNumAlliances); 
	Write ("</strong> alliance", sizeof ("</strong> alliance") - 1);
	if (iMaxNumAlliances != 1) { 
	Write ("s", sizeof ("s") - 1);
	}
	            break;
	        }
	        
	Write (" (", sizeof (" (") - 1);
	Write (iNumAlliances); 
	Write (")", sizeof (")") - 1);
	if (m_iGameClassOptions & UNBREAKABLE_ALLIANCES) {
	            
	Write ("<br>Unbreakable", sizeof ("<br>Unbreakable") - 1);
	}

	        if (m_iGameClassOptions & PERMANENT_ALLIANCES) {
	            
	Write ("<br>Count for entire game", sizeof ("<br>Count for entire game") - 1);
	}

	        
	Write ("</td>", sizeof ("</td>") - 1);
	}

	    if (m_iGameClassDip & SURRENDER) {

	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	if (m_iGameClassOptions & ONLY_SURRENDER_WITH_TWO_EMPIRES) {
	            
	Write ("When 2 empires remain", sizeof ("When 2 empires remain") - 1);
	} else {
	            
	Write ("Allowed", sizeof ("Allowed") - 1);
	}

	        
	Write ("</td>", sizeof ("</td>") - 1);
	}

	    
	Write ("</tr></table>", sizeof ("</tr></table>") - 1);
	}

	// Self

	Write ("<p><table width=\"95%\"><tr><th></th><th bgcolor=\"", sizeof ("<p><table width=\"95%\"><tr><th></th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Alien</th><th bgcolor=\"", sizeof ("\">Alien</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Econ</th><th bgcolor=\"", sizeof ("\">Econ</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Mil</th><th colspan=\"2\" bgcolor=\"", sizeof ("\">Mil</th><th colspan=\"2\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">They Offer</th><th colspan=\"2\" bgcolor=\"", sizeof ("\">They Offer</th><th colspan=\"2\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">You Offer</th><th bgcolor=\"", sizeof ("\">You Offer</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Status</th><th bgcolor=\"", sizeof ("\">Status</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Messages</th><th bgcolor=\"", sizeof ("\">Messages</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Last Access</th></tr><tr><td align=\"center\"><strong><font size=\"+1\">You</font></strong></td><td align=\"center\">", sizeof ("\">Last Access</th></tr><tr><td align=\"center\"><strong><font size=\"+1\">You</font></strong></td><td align=\"center\">") - 1);
	WriteProfileAlienString (
	    m_iAlienKey,
	    m_iEmpireKey,
	    m_vEmpireName.GetCharPtr(),
	    0,
	    "ProfileLink",
	    "View your profile",
	    false,
	    false
	    );


	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iEcon); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iMil); 
	Write ("</td><td colspan=\"2\" align=\"center\">-</td><td colspan=\"2\" align=\"center\">-</td><td align=\"center\"><strong>-</strong></td><td align=\"center\">-</td><td align=\"center\">", sizeof ("</td><td colspan=\"2\" align=\"center\">-</td><td colspan=\"2\" align=\"center\">-</td><td align=\"center\"><strong>-</strong></td><td align=\"center\">-</td><td align=\"center\">") - 1);
	WriteTime (Time::GetSecondDifference (tCurrentTime, tLastLogin));


	Write (" ago", sizeof (" ago") - 1);
	if (iGameOptions & RESIGNED) {
	    
	Write ("<br>(<strong><em>Resigned</em></strong>)", sizeof ("<br>(<strong><em>Resigned</em></strong>)") - 1);
	}

	else if (iNumUpdatesIdle > 0) {
	    
	Write ("<br>(<strong>", sizeof ("<br>(<strong>") - 1);
	Write (iNumUpdatesIdle); 
	Write ("</strong> update", sizeof ("</strong> update") - 1);
	if (iNumUpdatesIdle != 1) {
	        
	Write ("s", sizeof ("s") - 1);
	}
	    
	Write (" idle)", sizeof (" idle)") - 1);
	} 
	Write ("</td></tr><tr><th></th><th bgcolor=\"", sizeof ("</td></tr><tr><th></th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Wins</th><th bgcolor=\"", sizeof ("\">Wins</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Nukes</th><th bgcolor=\"", sizeof ("\">Nukes</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Nuked</th><th bgcolor=\"", sizeof ("\">Nuked</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Draws</th><th bgcolor=\"", sizeof ("\">Draws</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Ruins</th><th bgcolor=\"", sizeof ("\">Ruins</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Created</th><th bgcolor=\"", sizeof ("\">Created</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Max Econ</th><th bgcolor=\"", sizeof ("\">Max Econ</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Max Mil</th><th bgcolor=\"", sizeof ("\">Max Mil</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">IP Address</th><th bgcolor=\"", sizeof ("\">IP Address</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Ready for Update</th></tr><tr><td></td><td align=\"center\">", sizeof ("\">Ready for Update</th></tr><tr><td></td><td align=\"center\">") - 1);
	Write (iWins); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iNukes); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iNuked); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iDraws); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iRuins); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (pszCreated); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iMaxEcon); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iMaxMil); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (vIPAddress.GetCharPtr()); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	if (iGameOptions & UPDATED) {
	    
	Write ("<strong><font color=\"#", sizeof ("<strong><font color=\"#") - 1);
	Write (pszGood); 
	Write ("\">Yes</font></strong>", sizeof ("\">Yes</font></strong>") - 1);
	} else {
	    
	Write ("<strong><font color=\"#", sizeof ("<strong><font color=\"#") - 1);
	Write (pszBad); 
	Write ("\">No</font></strong>", sizeof ("\">No</font></strong>") - 1);
	} 
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	// Other empires
	iErrCode = pDatabase->ReadColumns (
	    strGameEmpireDiplomacy,
	    countof (piColumns),
	    piColumns,
	    &piProxyEmpireKey,
	    &ppvEmpireData,
	    (unsigned int*) &iNumKnownEmpires
	    );

	if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
	    Assert (false);
	    goto Cleanup;
	}


	Write ("<input type=\"hidden\" name=\"NumKnownEmpires\" value=\"", sizeof ("<input type=\"hidden\" name=\"NumKnownEmpires\" value=\"") - 1);
	Write (iNumKnownEmpires); 
	Write ("\">", sizeof ("\">") - 1);
	// Sort by diplomatic status
	piStatus = (int*) StackAlloc (iNumKnownEmpires * 2 * sizeof (int));
	piIndex = piStatus + iNumKnownEmpires;

	for (i = 0; i < iNumKnownEmpires; i ++) {
	    piStatus[i] = ppvEmpireData[i][2].GetInteger();
	    piIndex[i] = i;
	}
	Algorithm::QSortTwoDescending<int, int> (piStatus, piIndex, iNumKnownEmpires);

	for (iIndex = 0; iIndex < iNumKnownEmpires; iIndex ++) {

	    i = piIndex [iIndex];

	    // Get diplomacy data
	    iKnownEmpireKey = ppvEmpireData[i][0].GetInteger();
	    iWeOffer = ppvEmpireData[i][1].GetInteger();
	    iCurrentStatus = ppvEmpireData[i][2].GetInteger();

	    iErrCode = g_pGameEngine->GetDiplomaticStatus (
	        m_iGameClass,
	        m_iGameNumber,
	        m_iEmpireKey,
	        iKnownEmpireKey,
	        NULL,
	        &iTheyOffer,
	        NULL,
	        NULL
	        );

	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    if (bGameStarted) {

	        iErrCode = g_pGameEngine->GetDiplomaticOptions (
	            m_iGameClass,
	            m_iGameNumber,
	            m_iEmpireKey,
	            iKnownEmpireKey,
	            piDipKey,
	            &iSelected,
	            &iNumOptions
	            );

	        if (iErrCode != OK) {
	            Assert (false);
	            goto Cleanup;
	        }
	    }

	    GET_GAME_EMPIRE_DATA (pszEmpireData, m_iGameClass, m_iGameNumber, iKnownEmpireKey)

	    // Get empire data
	    iErrCode = pDatabase->GetTableForReading (pszEmpireData, &pGameEmpireTable);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pGameEmpireTable->ReadData (GameEmpireData::LastLogin, &tLastLogin);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pGameEmpireTable->ReadData (GameEmpireData::Options, &iGameOptions);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pGameEmpireTable->ReadData (GameEmpireData::NumUpdatesIdle, &iNumUpdatesIdle);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    if (bSubjective) {

	        SafeRelease (pGameEmpireTable);

	        iErrCode = pDatabase->ReadData (
	            strGameEmpireDiplomacy,
	            piProxyEmpireKey[i],
	            GameEmpireDiplomacy::SubjectiveEcon,
	            &vTemp
	            );

	        if (iErrCode != OK) {
	            Assert (false);
	            goto Cleanup;
	        }
	        iEcon = vTemp.GetInteger();

	        iErrCode = pDatabase->ReadData (
	            strGameEmpireDiplomacy,
	            piProxyEmpireKey[i],
	            GameEmpireDiplomacy::SubjectiveMil,
	            &vTemp
	            );
	        if (iErrCode != OK) {
	            Assert (false);
	            goto Cleanup;
	        }
	        iMil = vTemp.GetInteger();

	    } else {

	        iErrCode = pGameEmpireTable->ReadData (GameEmpireData::Econ, &iEcon);
	        if (iErrCode != OK) {
	            Assert (false);
	            goto Cleanup;
	        }

	        iErrCode = pGameEmpireTable->ReadData (GameEmpireData::Mil, &fMil);
	        if (iErrCode != OK) {
	            Assert (false);
	            goto Cleanup;
	        }

	        SafeRelease (pGameEmpireTable);

	        iMil = g_pGameEngine->GetMilitaryValue (fMil);
	    }

	    // Do this every time to improve concurrency with logins, etc.
	    iErrCode = pDatabase->GetTableForReading (SYSTEM_EMPIRE_DATA, &pSystemEmpireDataTable);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::Name, &vKnownEmpireName);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::AlienKey, &iAlienKey);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::Wins, &iWins);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::Nukes, &iNukes);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::Nuked, &iNuked);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::Draws, &iDraws);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::Ruins, &iRuins);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::MaxEcon, &iMaxEcon);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::MaxMil, &iMaxMil);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::CreationTime, &tCreated);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    iErrCode = pSystemEmpireDataTable->ReadData (iKnownEmpireKey, SystemEmpireData::IPAddress, &vIPAddress);
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    SafeRelease (pSystemEmpireDataTable);

	    Time::GetDate (tCreated, &iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);
	    sprintf (pszCreated, "%s %i %i", Time::GetAbbreviatedMonthName (iMonth), iDay, iYear);

	    
	Write ("<tr><td colspan=\"11\">&nbsp;</td></tr>", sizeof ("<tr><td colspan=\"11\">&nbsp;</td></tr>") - 1);
	if ((iIndex == 0 && piStatus[0] != ALLIANCE) || (iIndex > 0 && piStatus[iIndex] != piStatus[iIndex - 1])) {
	        
	Write ("<tr><td align=\"center\" colspan=\"11\">", sizeof ("<tr><td align=\"center\" colspan=\"11\">") - 1);
	WriteSeparatorString (m_iSeparatorKey); 
	Write ("</td></tr><tr><td colspan=\"11\">&nbsp;</td></tr>", sizeof ("</td></tr><tr><td colspan=\"11\">&nbsp;</td></tr>") - 1);
	}

	    
	Write ("<tr><th></th><th bgcolor=\"", sizeof ("<tr><th></th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Alien</th><th bgcolor=\"", sizeof ("\">Alien</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Econ</th><th bgcolor=\"", sizeof ("\">Econ</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Mil</th><th colspan=\"2\" bgcolor=\"", sizeof ("\">Mil</th><th colspan=\"2\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">They Offer</th><th colspan=\"2\" bgcolor=\"", sizeof ("\">They Offer</th><th colspan=\"2\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">You Offer</th><th bgcolor=\"", sizeof ("\">You Offer</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Status</th><th bgcolor=\"", sizeof ("\">Status</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Messages</th><th bgcolor=\"", sizeof ("\">Messages</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Last Access</th></tr><tr><td width=\"10%\" align=\"center\"><strong>", sizeof ("\">Last Access</th></tr><tr><td width=\"10%\" align=\"center\"><strong>") - 1);
	if (iGameOptions & RESIGNED) {
	        
	Write ("<strike>", sizeof ("<strike>") - 1);
	}

	    if (iCurrentStatus == WAR) {
	        
	Write ("<font size=\"+1\" color=\"#", sizeof ("<font size=\"+1\" color=\"#") - 1);
	Write (pszBad); 
	Write ("\">", sizeof ("\">") - 1);
	Write (vKnownEmpireName.GetCharPtr()); 
	Write ("</font>", sizeof ("</font>") - 1);
	}
	    else if (iCurrentStatus == ALLIANCE) {
	        
	Write ("<font size=\"+1\" color=\"#", sizeof ("<font size=\"+1\" color=\"#") - 1);
	Write (pszGood); 
	Write ("\">", sizeof ("\">") - 1);
	Write (vKnownEmpireName.GetCharPtr()); 
	Write ("</font>", sizeof ("</font>") - 1);
	}
	    else {
	        
	Write ("<font size=\"+1\">", sizeof ("<font size=\"+1\">") - 1);
	Write (vKnownEmpireName.GetCharPtr()); 
	Write ("</font>", sizeof ("</font>") - 1);
	}

	    if (iGameOptions & RESIGNED) {
	        
	Write ("</strike>", sizeof ("</strike>") - 1);
	}
	    
	Write ("</strong></td><td align=\"center\">", sizeof ("</strong></td><td align=\"center\">") - 1);
	sprintf (pszProfile, "View the profile of %s", vKnownEmpireName.GetCharPtr());
	    WriteProfileAlienString (
	        iAlienKey,
	        iKnownEmpireKey,
	        vKnownEmpireName.GetCharPtr(),
	        0,
	        "ProfileLink",
	        pszProfile,
	        false,
	        true
	        );

	    NotifyProfileLink();

	    
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iEcon); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iMil); 
	Write ("</td>", sizeof ("</td>") - 1);
	if (!bGameStarted) {
	        
	Write ("<td colspan=\"2\" align=\"center\">-</td><td colspan=\"2\" align=\"center\">-</td><td align=\"center\">-</td>", sizeof ("<td colspan=\"2\" align=\"center\">-</td><td colspan=\"2\" align=\"center\">-</td><td align=\"center\">-</td>") - 1);
	} else {

	        
	Write ("<td colspan=\"2\" align=\"center\">", sizeof ("<td colspan=\"2\" align=\"center\">") - 1);
	if (iTheyOffer == WAR) {
	            
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (pszBad); 
	Write ("\">", sizeof ("\">") - 1);
	Write (WAR_STRING);
	        }

	        else if (iTheyOffer == ALLIANCE) {
	            
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (pszGood); 
	Write ("\">", sizeof ("\">") - 1);
	Write (ALLIANCE_STRING); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	            Write (DIP_STRING (iTheyOffer)); 
	        }

	        
	Write ("</td><td colspan=\"2\" align=\"center\">", sizeof ("</td><td colspan=\"2\" align=\"center\">") - 1);
	if (iNumOptions > 1) {
	            
	Write ("<select name=\"DipOffer", sizeof ("<select name=\"DipOffer") - 1);
	Write (i); 
	Write ("\" size=\"1\">", sizeof ("\" size=\"1\">") - 1);
	iSelectedIndex = -1;
	            for (j = 0; j < iNumOptions; j ++) {
	                if (piDipKey[j] == iSelected) { 
	                    iSelectedIndex = j;
	                    
	Write ("<option selected value=\"", sizeof ("<option selected value=\"") - 1);
	Write (piDipKey[j]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (DIP_STRING (piDipKey[j]));
	                    
	Write ("</option>", sizeof ("</option>") - 1);
	} else {
	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piDipKey[j]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (DIP_STRING (piDipKey[j])); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            } 
	Write ("</select>", sizeof ("</select>") - 1);
	if (iSelectedIndex == -1) {
	                iSelectedIndex = piDipKey[0];
	            }

	            
	Write ("<input type=\"hidden\" name=\"SelectedDip", sizeof ("<input type=\"hidden\" name=\"SelectedDip") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (piDipKey[iSelectedIndex]); 
	Write ("\">", sizeof ("\">") - 1);
	} else {

	            if (piDipKey[0] == WAR) {
	                
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (pszBad); 
	Write ("\">", sizeof ("\">") - 1);
	Write (WAR_STRING);
	            } else if (piDipKey[0] == ALLIANCE) {
	                
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (pszGood); 
	Write ("\">", sizeof ("\">") - 1);
	Write (ALLIANCE_STRING); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	                Write (DIP_STRING (piDipKey[0])); 
	            }

	            
	Write ("<input type=\"hidden\" name=\"DipOffer", sizeof ("<input type=\"hidden\" name=\"DipOffer") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (piDipKey[0]); 
	Write ("\"><input type=\"hidden\" name=\"SelectedDip", sizeof ("\"><input type=\"hidden\" name=\"SelectedDip") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (piDipKey[0]); 
	Write ("\">", sizeof ("\">") - 1);
	}

	        
	Write ("<input type=\"hidden\" name=\"FoeKey", sizeof ("<input type=\"hidden\" name=\"FoeKey") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (iKnownEmpireKey); 
	Write ("\"></td><td align=\"center\"><strong>", sizeof ("\"></td><td align=\"center\"><strong>") - 1);
	if (iCurrentStatus == WAR) {
	            
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (pszBad); 
	Write ("\">", sizeof ("\">") - 1);
	Write (WAR_STRING);
	        }

	        else if (iCurrentStatus == ALLIANCE) {
	            
	Write ("<font color=\"#", sizeof ("<font color=\"#") - 1);
	Write (pszGood); 
	Write ("\">", sizeof ("\">") - 1);
	Write (ALLIANCE_STRING); 
	Write ("</font>", sizeof ("</font>") - 1);
	} else {
	            Write (DIP_STRING (iCurrentStatus)); 
	        } 
	Write ("</strong></td>", sizeof ("</strong></td>") - 1);
	}   // End if game started

	    
	Write ("<td align=\"center\"><select name=\"Ignore", sizeof ("<td align=\"center\"><select name=\"Ignore") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	iErrCode = g_pGameEngine->GetEmpireIgnoreMessages (
	        m_iGameClass, 
	        m_iGameNumber, 
	        m_iEmpireKey, 
	        iKnownEmpireKey, 
	        &bIgnore
	        );
	    if (iErrCode != OK) {
	        Assert (false);
	        goto Cleanup;
	    }

	    if (bIgnore) {
	        
	Write ("<option value=\"0\">Hear</option><option selected value=\"1\">Ignore</option>", sizeof ("<option value=\"0\">Hear</option><option selected value=\"1\">Ignore</option>") - 1);
	} else {
	        
	Write ("<option selected value=\"0\">Hear</option><option value=\"1\">Ignore</option>", sizeof ("<option selected value=\"0\">Hear</option><option value=\"1\">Ignore</option>") - 1);
	}
	    
	Write ("</select></td><td align=\"center\">", sizeof ("</select></td><td align=\"center\">") - 1);
	WriteTime (Time::GetSecondDifference (tCurrentTime, tLastLogin));

	    
	Write (" ago ", sizeof (" ago ") - 1);
	if (iGameOptions & RESIGNED) {
	        
	Write ("<br>(<strong><em>Resigned</em></strong>)", sizeof ("<br>(<strong><em>Resigned</em></strong>)") - 1);
	}

	    else if (iNumUpdatesIdle > 0) {
	        
	Write ("<br>(<strong>", sizeof ("<br>(<strong>") - 1);
	Write (iNumUpdatesIdle); 
	Write ("</strong> update", sizeof ("</strong> update") - 1);
	if (iNumUpdatesIdle != 1) {
	            
	Write ("s", sizeof ("s") - 1);
	}
	        
	Write (" idle)", sizeof (" idle)") - 1);
	} 
	Write ("</td></tr><tr><th></th><th bgcolor=\"", sizeof ("</td></tr><tr><th></th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Wins</th><th bgcolor=\"", sizeof ("\">Wins</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Nukes</th><th bgcolor=\"", sizeof ("\">Nukes</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Nuked</th><th bgcolor=\"", sizeof ("\">Nuked</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Draws</th><th bgcolor=\"", sizeof ("\">Draws</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Ruins</th><th bgcolor=\"", sizeof ("\">Ruins</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Created</th><th bgcolor=\"", sizeof ("\">Created</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Max Econ</th><th bgcolor=\"", sizeof ("\">Max Econ</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Max Mil</th><th bgcolor=\"", sizeof ("\">Max Mil</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">", sizeof ("\">") - 1);
	if (m_iPrivilege < ADMINISTRATOR) {
	        
	Write ("Hashed ", sizeof ("Hashed ") - 1);
	}

	    
	Write ("IP Address</th><th bgcolor=\"", sizeof ("IP Address</th><th bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Ready for Update</th></tr><tr><td></td><td align=\"center\">", sizeof ("\">Ready for Update</th></tr><tr><td></td><td align=\"center\">") - 1);
	Write (iWins); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iNukes); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iNuked); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iDraws); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iRuins); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (pszCreated); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iMaxEcon); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (iMaxMil); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	if (m_iPrivilege < ADMINISTRATOR) {
	        char pszHashedIPAddress [128];
	        HashIPAddress (vIPAddress.GetCharPtr(), pszHashedIPAddress);
	        Write (pszHashedIPAddress);
	    } else {
	        Write (vIPAddress.GetCharPtr());
	    }

	    
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	if (iGameOptions & UPDATED) {
	        
	Write ("<strong><font color=\"#", sizeof ("<strong><font color=\"#") - 1);
	Write (pszGood); 
	Write ("\">Yes</font></strong>", sizeof ("\">Yes</font></strong>") - 1);
	} else {
	        
	Write ("<strong><font color=\"#", sizeof ("<strong><font color=\"#") - 1);
	Write (pszBad); 
	Write ("\">No</font></strong>", sizeof ("\">No</font></strong>") - 1);
	} 
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}   // End known empire loop

	//
	// Messages
	//

	iErrCode = g_pGameEngine->DoesGameClassAllowPrivateMessages (m_iGameClass, &bPrivateMessages);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	iErrCode = g_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iActiveEmpires);
	if (iErrCode != OK) {
	    Assert (false);
	    goto Cleanup;
	}

	if (iNumKnownEmpires > 0 || iActiveEmpires > 1) {
	    
	Write ("<tr><td colspan=\"11\">&nbsp;</td></tr>", sizeof ("<tr><td colspan=\"11\">&nbsp;</td></tr>") - 1);
	}


	Write ("</table>", sizeof ("</table>") - 1);
	if (iActiveEmpires > 1) {

	    WriteSeparatorString (m_iSeparatorKey);

	    bool bBroadcast = bBroadcast = (m_iSystemOptions & CAN_BROADCAST) != 0;
	    bool bAllTargets = bPrivateMessages && bGameStarted;

	    if (bBroadcast || (bPrivateMessages && iNumKnownEmpires > 0)) {

	        int iExtra = 0, iWar = 0, iTruce = 0, iTrade = 0, iAlliance = 0, iDipTargets = 0;

	        if (bAllTargets) {

	            iErrCode = g_pGameEngine->GetNumEmpiresAtDiplomaticStatus (
	                m_iGameClass,
	                m_iGameNumber,
	                m_iEmpireKey,
	                &iWar,
	                &iTruce,
	                &iTrade,
	                &iAlliance
	                );

	            if (iErrCode != OK) {
	                Assert (false);
	                iWar = iTruce = iTrade = iAlliance = iDipTargets = 0;
	            } else {

	                if (iWar > 0) {
	                    iExtra ++;
	                }

	                if (iTruce > 0) {
	                    iExtra ++;
	                }

	                if (iTrade > 0) {
	                    iExtra ++;
	                }

	                if (iAlliance > 0) {
	                    iExtra ++;
	                }

	                iDipTargets = iWar + iTruce + iTrade + iAlliance;
	            }
	        }

	        // Array of bits for selected
	        size_t stAlloc = (NUM_MESSAGE_TARGETS + iNumKnownEmpires) * sizeof (bool);
	        bool* pbSelected = (bool*) StackAlloc (stAlloc);
	        memset (pbSelected, 0, stAlloc);

	        switch (iDefaultMessageTarget) {

	        case MESSAGE_TARGET_NONE:

	            if (iNumKnownEmpires == 0) {
	                pbSelected[MESSAGE_TARGET_BROADCAST] = true;
	            }
	            break;

	        case MESSAGE_TARGET_LAST_USED:

	            {

	            // Handle last used
	            int iLastUsedMask, iNumLastUsed;
	            int* piLastUsedProxyKeyArray = NULL;

	            iErrCode = g_pGameEngine->GetLastUsedMessageTarget (
	                m_iGameClass,
	                m_iGameNumber,
	                m_iEmpireKey,
	                &iLastUsedMask,
	                &piLastUsedProxyKeyArray,
	                &iNumLastUsed
	                );

	            // Best effort
	            Assert (iErrCode == OK);

	            if (iErrCode == OK) {

	                // Process mask
	                if ((iLastUsedMask & (~MESSAGE_TARGET_INDIVIDUALS)) == 0) {

	                    // Default to broadcast if no last target
	                    if (iNumLastUsed == 0) {
	                        pbSelected[MESSAGE_TARGET_BROADCAST] = true;
	                    }

	                } else {

	                    if (iLastUsedMask & MESSAGE_TARGET_BROADCAST_MASK) {
	                        pbSelected[MESSAGE_TARGET_BROADCAST] = true;
	                    }

	                    if (iLastUsedMask & MESSAGE_TARGET_WAR_MASK) {
	                        pbSelected[MESSAGE_TARGET_WAR] = true;
	                    }

	                    if (iLastUsedMask & MESSAGE_TARGET_TRUCE_MASK) {
	                        pbSelected[MESSAGE_TARGET_TRUCE] = true;
	                    }

	                    if (iLastUsedMask & MESSAGE_TARGET_TRADE_MASK) {
	                        pbSelected[MESSAGE_TARGET_TRADE] = true;
	                    }

	                    if (iLastUsedMask & MESSAGE_TARGET_ALLIANCE_MASK) {
	                        pbSelected[MESSAGE_TARGET_ALLIANCE] = true;
	                    }
	                }

	                // Process array
	                for (i = 0; i < iNumLastUsed; i ++) {

	                    // Find order of empire
	                    for (j = 0; j < iNumKnownEmpires; j ++) {

	                        if (piLastUsedProxyKeyArray[i] == (int) piProxyEmpireKey[j]) {
	                            break;
	                        }
	                    }

	                    if (j < iNumKnownEmpires) {
	                        pbSelected[NUM_MESSAGE_TARGETS + j] = true;
	                    }
	                }

	                g_pGameEngine->FreeKeys (piLastUsedProxyKeyArray);
	            }

	            }
	            break;

	        default:

	            pbSelected[iDefaultMessageTarget] = true;
	            break;
	        }

	        
	Write ("<p><table width=\"90%\"><tr><td><strong>Send a message to:</strong></td><td align=\"center\" rowspan=\"3\"><textarea name=\"Message\" rows=\"7\" cols=\"60\" wrap=\"virtual\">", sizeof ("<p><table width=\"90%\"><tr><td><strong>Send a message to:</strong></td><td align=\"center\" rowspan=\"3\"><textarea name=\"Message\" rows=\"7\" cols=\"60\" wrap=\"virtual\">") - 1);
	if (pszRedrawMessage != NULL) {
	            Write (pszRedrawMessage);
	        } 
	Write ("</textarea></td><tr><td><select name=\"MessageTarget\" multiple size=\"", sizeof ("</textarea></td><tr><td><select name=\"MessageTarget\" multiple size=\"") - 1);
	if (bPrivateMessages) {

	            if (iNumKnownEmpires < 5) {
	                Write (iNumKnownEmpires + iExtra + (bBroadcast ? 1 : 0));
	            } else {
	                
	Write ("5", sizeof ("5") - 1);
	}
	        } else {
	            
	Write ("1", sizeof ("1") - 1);
	} 
	Write ("\">", sizeof ("\">") - 1);
	bool bNoPrivates = !bPrivateMessages || iNumKnownEmpires == 0;
	        bool bNoDipTargets = !bAllTargets || iDipTargets == 0;

	        if (bBroadcast) {

	            
	Write ("<option", sizeof ("<option") - 1);
	if (pbSelected[MESSAGE_TARGET_BROADCAST] || (bNoPrivates && bNoDipTargets)) {
	                
	Write (" selected", sizeof (" selected") - 1);
	}
	            
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (NO_KEY); 
	Write ("\">Broadcast</option>", sizeof ("\">Broadcast</option>") - 1);
	}

	        if (bAllTargets) {

	            if (iWar > 0) {
	                
	Write ("<option", sizeof ("<option") - 1);
	if (pbSelected[MESSAGE_TARGET_WAR] || (!bBroadcast && bNoPrivates && iWar == iDipTargets)) {
	                    
	Write (" selected", sizeof (" selected") - 1);
	}
	                
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (ALL_WAR); 
	Write ("\">All at War</option>", sizeof ("\">All at War</option>") - 1);
	}

	            if (iTruce > 0) {
	                
	Write ("<option", sizeof ("<option") - 1);
	if (pbSelected[MESSAGE_TARGET_TRUCE] || (!bBroadcast && bNoPrivates && iTruce == iDipTargets)) {
	                    
	Write (" selected", sizeof (" selected") - 1);
	}
	                
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (ALL_TRUCE); 
	Write ("\">All at Truce</option>", sizeof ("\">All at Truce</option>") - 1);
	}

	            if (iTrade > 0) {
	                
	Write ("<option", sizeof ("<option") - 1);
	if (pbSelected[MESSAGE_TARGET_TRADE] || (!bBroadcast && bNoPrivates && iTrade == iDipTargets)) {
	                    
	Write (" selected", sizeof (" selected") - 1);
	}
	                
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (ALL_TRADE); 
	Write ("\">All at Trade</option>", sizeof ("\">All at Trade</option>") - 1);
	}

	            if (iAlliance > 0) {
	                
	Write ("<option", sizeof ("<option") - 1);
	if (pbSelected[MESSAGE_TARGET_ALLIANCE] || (!bBroadcast && bNoPrivates && iAlliance == iDipTargets)) {
	                    
	Write (" selected", sizeof (" selected") - 1);
	}
	                
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (ALL_ALLIANCE); 
	Write ("\">All at Alliance</option>", sizeof ("\">All at Alliance</option>") - 1);
	}
	        }

	        if (bPrivateMessages) {

	            Variant vSendEmpireName;
	            for (i = 0; i < iNumKnownEmpires; i ++) {

	                iKnownEmpireKey = ppvEmpireData[i][0].GetInteger();

	                iErrCode = g_pGameEngine->GetEmpireIgnoreMessages (
	                    m_iGameClass,
	                    m_iGameNumber, 
	                    iKnownEmpireKey,
	                    m_iEmpireKey,
	                    &bIgnore
	                    );
	                if (iErrCode != OK) {
	                    Assert (false);
	                    goto Cleanup;
	                }

	                if (!bIgnore) {

	                    iErrCode = g_pGameEngine->GetEmpireName (iKnownEmpireKey, &vSendEmpireName);
	                    if (iErrCode != OK) {
	                        Assert (false);
	                        goto Cleanup;
	                    }

	                    
	Write ("<option", sizeof ("<option") - 1);
	if (pbSelected[NUM_MESSAGE_TARGETS + i] || (iNumKnownEmpires == 1 && !bBroadcast && bNoDipTargets)) {
	                        
	Write (" selected", sizeof (" selected") - 1);
	}
	                    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (iKnownEmpireKey); 
	Write ("\">", sizeof ("\">") - 1);
	Write (vSendEmpireName.GetCharPtr()); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	        }
	        
	Write ("</select></td><tr><td>", sizeof ("</select></td><tr><td>") - 1);
	WriteButton (BID_SENDMESSAGE);

	        
	Write ("</table>", sizeof ("</table>") - 1);
	}
	}

	Cleanup:

	if (piProxyEmpireKey != NULL) {
	    g_pGameEngine->FreeKeys (piProxyEmpireKey);
	}

	if (ppvEmpireData != NULL) {
	    g_pGameEngine->FreeData (ppvEmpireData);
	}

	if (pGameEmpireTable != NULL) {
	    pGameEmpireTable->Release();
	}

	if (pSystemEmpireDataTable != NULL) {
	    pSystemEmpireDataTable->Release();
	}

	pDatabase->Release();

	if (iErrCode != OK) {
	    
	Write ("<p>Error ", sizeof ("<p>Error ") - 1);
	Write (iErrCode); 
	Write (" occurred processing the Diplomacy page", sizeof (" occurred processing the Diplomacy page") - 1);
	}

	GAME_CLOSE


}