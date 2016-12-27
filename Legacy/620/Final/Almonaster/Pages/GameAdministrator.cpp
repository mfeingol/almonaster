
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "Osal/Algorithm.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the GameAdministrator page
int HtmlRenderer::Render_GameAdministrator() {

	// Almonaster 2.0
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

	INITIALIZE_EMPIRE

	IHttpForm* pHttpForm;

	int i, iErrCode;

	// Make sure that the unprivileged don't abuse this:
	if (m_iPrivilege < ADMINISTRATOR) {
	    AddMessage ("You are not authorized to view this page");
	    return Redirect (LOGIN);
	}

	int iGameAdminPage = 0;
	int iGameClass = 0, iGameNumber = 0, iClickedPlanetKey = 0;

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

	    IHttpForm* pHttpForm2;

	    if ((pHttpForm = m_pHttpRequest->GetForm ("GameAdminPage")) == NULL) {
	        goto Redirection;
	    }
	    int iGameAdminPageSubmit = pHttpForm->GetIntValue();

	    if (WasButtonPressed (BID_CANCEL)) {

	        bRedirectTest = false;

	        if ((iGameAdminPageSubmit == 5 || iGameAdminPageSubmit == 7) &&
	            (pHttpForm = m_pHttpRequest->GetForm ("GameClass")) != NULL &&
	            (pHttpForm2 = m_pHttpRequest->GetForm ("GameNumber")) != NULL) {

	            iGameClass = pHttpForm->GetIntValue();
	            iGameNumber = pHttpForm2->GetIntValue();
	            iGameAdminPage = 3;
	        }

	        else if (iGameAdminPageSubmit == 4 &&
	            (pHttpForm = m_pHttpRequest->GetForm ("GameClass")) != NULL &&
	            (pHttpForm2 = m_pHttpRequest->GetForm ("GameNumber")) != NULL) {

	            iGameClass = pHttpForm->GetIntValue();
	            iGameNumber = pHttpForm2->GetIntValue();
	            iGameAdminPage = 3;
	        } else if (iGameAdminPageSubmit == 3) {
	            iGameAdminPage = 1;
	        }

	    } else {

	        switch (iGameAdminPageSubmit) {

	        case 0:
	            {

	            // Handle parameter changes
	            int iSystemMinNumSecsPerUpdate, iSystemMaxNumSecsPerUpdate, iSystemMaxNumEmpires, 
	                iSystemMaxNumPlanets, iPersonalMinNumSecsPerUpdate, iPersonalMaxNumSecsPerUpdate, 
	                iPersonalMaxNumEmpires, iPersonalMaxNumPlanets, iNewValue, iOldValue, iSystemOptions;
	                
	            bool bNewValue;

	            Variant vMaxNumUpdatesBeforeClose, vDefaultNumUpdatesBeforeClose, vUnlimitedEmpirePrivilege, vDelay;

	            const char* pszNewValue;

	            Check (g_pGameEngine->GetMinNumSecsPerUpdateForSystemGameClass (&iSystemMinNumSecsPerUpdate));
	            Check (g_pGameEngine->GetMaxNumSecsPerUpdateForSystemGameClass (&iSystemMaxNumSecsPerUpdate));
	            Check (g_pGameEngine->GetMaxNumEmpiresForSystemGameClass (&iSystemMaxNumEmpires));
	            Check (g_pGameEngine->GetMaxNumPlanetsForSystemGameClass (&iSystemMaxNumPlanets));

	            Check (g_pGameEngine->GetSystemProperty (SystemData::MaxNumUpdatesBeforeClose, &vMaxNumUpdatesBeforeClose));
	            Check (g_pGameEngine->GetSystemProperty (SystemData::DefaultNumUpdatesBeforeClose, &vDefaultNumUpdatesBeforeClose));

	            Check (g_pGameEngine->GetMinNumSecsPerUpdateForPersonalGameClass (&iPersonalMinNumSecsPerUpdate));
	            Check (g_pGameEngine->GetMaxNumSecsPerUpdateForPersonalGameClass (&iPersonalMaxNumSecsPerUpdate));
	            Check (g_pGameEngine->GetMaxNumEmpiresForPersonalGameClass (&iPersonalMaxNumEmpires));
	            Check (g_pGameEngine->GetMaxNumPlanetsForPersonalGameClass (&iPersonalMaxNumPlanets));

	            Variant vMaxResourcesPerPlanet, vMaxResourcesPerPlanetPersonal, vMaxInitialTechLevel, 
	                vMaxInitialTechLevelPersonal, vMaxTechDev, vMaxTechDevPersonal;

	            Check (g_pGameEngine->GetSystemProperty (SystemData::MaxResourcesPerPlanet, &vMaxResourcesPerPlanet));
	            Check (g_pGameEngine->GetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, &vMaxResourcesPerPlanetPersonal));

	            Check (g_pGameEngine->GetSystemProperty (SystemData::MaxInitialTechLevel, &vMaxInitialTechLevel));
	            Check (g_pGameEngine->GetSystemProperty (SystemData::MaxInitialTechLevelPersonal, &vMaxInitialTechLevelPersonal));

	            Check (g_pGameEngine->GetSystemProperty (SystemData::MaxTechDev, &vMaxTechDev));
	            Check (g_pGameEngine->GetSystemProperty (SystemData::MaxTechDevPersonal, &vMaxTechDevPersonal));

	            Check (g_pGameEngine->GetSystemProperty (SystemData::AfterWeekendDelay, &vDelay));

	            Check (g_pGameEngine->GetSystemOptions (&iSystemOptions));

	            Check (g_pGameEngine->GetSystemProperty (SystemData::PrivilegeForUnlimitedEmpires, &vUnlimitedEmpirePrivilege));

	            GameConfiguration gcOldConfig, gcNewConfig;
	            MapConfiguration mcOldConfig, mcNewConfig;

	            memset (&gcOldConfig, 0, sizeof (gcOldConfig));
	            memset (&gcNewConfig, 0, sizeof (gcNewConfig));
	            memset (&mcOldConfig, 0, sizeof (mcOldConfig));
	            memset (&mcNewConfig, 0, sizeof (mcNewConfig));

	            Check (g_pGameEngine->GetGameConfiguration (&gcOldConfig));
	            Check (g_pGameEngine->GetMapConfiguration (&mcOldConfig));

	            // Empires
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxEmps")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != iSystemMaxNumEmpires) {
	                if (g_pGameEngine->SetMaxNumEmpiresForSystemGameClass (iNewValue) == OK) {
	                    AddMessage ("The maximum number of empires for a System GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum number of empires for a System GameClass");
	                }
	            }

	            // Planets
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxPlanets")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != iSystemMaxNumPlanets) {
	                if (g_pGameEngine->SetMaxNumPlanetsForSystemGameClass (iNewValue) == OK) {
	                    AddMessage ("The maximum number of planets for a System GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum number of planets for a System GameClass");
	                }
	            }

	            // MaxNumUpdatesBeforeClose
	            if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumUpdatesBeforeClose")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != vMaxNumUpdatesBeforeClose.GetInteger()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxNumUpdatesBeforeClose, iNewValue) == OK) {
	                    AddMessage ("The maximum number of updates before closing was updated");
	                } else {
	                    AddMessage ("Invalid maximum number of updates before closing");
	                }
	            }

	            // DefaultNumUpdatesBeforeClose
	            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultNumUpdatesBeforeClose")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != vDefaultNumUpdatesBeforeClose.GetInteger()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::DefaultNumUpdatesBeforeClose, iNewValue) == OK) {
	                    AddMessage ("The default number of updates before closing was updated");
	                } else {
	                    AddMessage ("Invalid default number of updates before closing");
	                }
	            }

	            // BridierDefault
	            if ((pHttpForm = m_pHttpRequest->GetForm ("BridierDefault")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != (iSystemOptions & DEFAULT_BRIDIER_GAMES)) {
	                if (g_pGameEngine->SetSystemOption (DEFAULT_BRIDIER_GAMES, iNewValue != 0) == OK) {
	                    AddMessage ("The default Bridier configuration was updated");
	                } else {
	                    AddMessage ("The default Bridier configuration could not be updated");
	                }
	            }

	            // DefaultNamesListed
	            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultNamesListed")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != (iSystemOptions & DEFAULT_NAMES_LISTED)) {
	                if (g_pGameEngine->SetSystemOption (DEFAULT_NAMES_LISTED, iNewValue != 0) == OK) {
	                    AddMessage ("The default names listed configuration was updated");
	                } else {
	                    AddMessage ("The default names listed configuration could not be updated");
	                }
	            }

	            // DefaultSpectator
	            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultSpectator")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != (iSystemOptions & DEFAULT_ALLOW_SPECTATORS)) {
	                if (g_pGameEngine->SetSystemOption (DEFAULT_ALLOW_SPECTATORS, iNewValue != 0) == OK) {
	                    AddMessage ("The default spectator game setting was updated");
	                } else {
	                    AddMessage ("The default spectator game setting could not be updated");
	                }
	            }

	            // FilterIP
	            if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIP")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue() & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS);

	            if (iNewValue != (iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS))) {

	                if (iNewValue & DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
	                    Check (g_pGameEngine->SetSystemOption (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS, true));
	                } else {
	                    Check (g_pGameEngine->SetSystemOption (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS, false));
	                }

	                if (iNewValue & DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
	                    Check (g_pGameEngine->SetSystemOption (DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS, true));
	                } else {
	                    Check (g_pGameEngine->SetSystemOption (DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS, false));
	                }

	                AddMessage ("The default IP Address filtering configuration was updated");
	            }

	            // FilterId
	            if ((pHttpForm = m_pHttpRequest->GetForm ("FilterId")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue() & (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID | DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID);

	            if (iNewValue != (iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID | DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID))) {

	                if (iNewValue & DEFAULT_WARN_ON_DUPLICATE_SESSION_ID) {
	                    Check (g_pGameEngine->SetSystemOption (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID, true));
	                } else {
	                    Check (g_pGameEngine->SetSystemOption (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID, false));
	                }

	                if (iNewValue & DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID) {
	                    Check (g_pGameEngine->SetSystemOption (DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID, true));
	                } else {
	                    Check (g_pGameEngine->SetSystemOption (DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID, false));
	                }

	                AddMessage ("The default Session Id filtering configuration was updated");
	            }
	            
	            // FilterIdle
	            if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIdle")) == NULL) {
	                goto Redirection;
	            }
	            bNewValue = pHttpForm->GetIntValue() != 0;

	            if (bNewValue != ((iSystemOptions & DEFAULT_RESTRICT_IDLE_EMPIRES) != 0)) {
	            
	                Check (g_pGameEngine->SetSystemOption (DEFAULT_RESTRICT_IDLE_EMPIRES, bNewValue));
	                AddMessage ("The default idle empire blocking configuration was updated");
	            }

	            // MaxResourcesPerPlanet
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxResourcesPerPlanet")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != vMaxResourcesPerPlanet.GetInteger()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxResourcesPerPlanet, iNewValue) == OK) {
	                    AddMessage ("The maximum average resources per attribute per planet for a System GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum average resources per attribute per planet for a System GameClass");
	                }
	            }

	            // MaxInitialTechLevel
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxInitialTechLevel")) == NULL) {
	                goto Redirection;
	            }
	            float fNewValue = pHttpForm->GetFloatValue();

	            if (fNewValue != vMaxInitialTechLevel.GetFloat()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxInitialTechLevel, fNewValue) == OK) {
	                    AddMessage ("The maximum initial tech level for a System GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum initial tech level for a System GameClass");
	                }
	            }

	            // MaxTechDev
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxTechDev")) == NULL) {
	                goto Redirection;
	            }
	            fNewValue = pHttpForm->GetFloatValue();

	            if (fNewValue != vMaxTechDev.GetFloat()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxTechDev, fNewValue) == OK) {
	                    AddMessage ("The maximum tech development for a System GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum tech development for a System GameClass");
	                }
	            }

	            // Min update period
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMinHrs")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue() * 3600;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMinMin")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue() * 60;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMinSec")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue();

	            if (iNewValue != iSystemMinNumSecsPerUpdate) {
	                if (g_pGameEngine->SetMinNumSecsPerUpdateForSystemGameClass (iNewValue) == OK) {
	                    AddMessage ("The minimum update period for a System GameClass was updated");
	                } else {
	                    AddMessage ("Invalid minimum update period for a System GameClass");
	                }
	            }

	            // Max update period
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxHrs")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue() * 3600;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxMin")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue() * 60;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxSec")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue();

	            if (iNewValue != iSystemMaxNumSecsPerUpdate) {
	                if (g_pGameEngine->SetMaxNumSecsPerUpdateForSystemGameClass (iNewValue) == OK) {
	                    AddMessage ("The maximum update period for a System GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum update period for a System GameClass");
	                }
	            }

	            // Max number of personal game classes
	            if ((pHttpForm = m_pHttpRequest->GetForm ("NewMaxNumPGC")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();
	            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxNumPGC")) == NULL) {
	                goto Redirection;
	            }
	            iOldValue = pHttpForm->GetIntValue();
	            if (iNewValue != iOldValue && iNewValue >= 0) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxNumPersonalGameClasses, iNewValue) == OK) {
	                    AddMessage ("The maximum number of personal GameClasses was updated");
	                } else {
	                    AddMessage ("The maximum number of personal GameClasses could not be updated");
	                }
	            }

	            // Max number of personal tournaments
	            if ((pHttpForm = m_pHttpRequest->GetForm ("NewMaxNumT")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();
	            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxNumT")) == NULL) {
	                goto Redirection;
	            }
	            iOldValue = pHttpForm->GetIntValue();
	            if (iNewValue != iOldValue && iNewValue >= 0) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxNumPersonalTournaments, iNewValue) == OK) {
	                    AddMessage ("The maximum number of personal tournaments was updated");
	                } else {
	                    AddMessage ("The maximum number of personal tournaments could not be updated");
	                }
	            }

	            // Max number of personal game classes per tournament
	            if ((pHttpForm = m_pHttpRequest->GetForm ("NewMaxNumPGCT")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();
	            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxNumPGCT")) == NULL) {
	                goto Redirection;
	            }
	            iOldValue = pHttpForm->GetIntValue();
	            if (iNewValue != iOldValue && iNewValue >= 0) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxNumGameClassesPerPersonalTournament, iNewValue) == OK) {
	                    AddMessage ("The maximum number of GameClasses per personal tournament was updated");
	                } else {
	                    AddMessage ("The maximum number of GameClasses per personal tournament could not be updated");
	                }
	            }

	            // Empires
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxEmps")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != iPersonalMaxNumEmpires) {
	                if (g_pGameEngine->SetMaxNumEmpiresForPersonalGameClass (iNewValue) == OK) {
	                    AddMessage ("The maximum number of empires for a personal GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum number of empires for a personal GameClass");
	                }
	            }

	            // Planets
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxPlanets")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != iPersonalMaxNumPlanets) {
	                if (g_pGameEngine->SetMaxNumPlanetsForPersonalGameClass (iNewValue) == OK) {
	                    AddMessage ("The maximum number of planets for a personal GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum number of planets for a personal GameClass");
	                }
	            }

	            // MaxResourcesPerPlanet
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxResourcesPerPlanet")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != vMaxResourcesPerPlanetPersonal.GetInteger()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, iNewValue) == OK) {
	                    AddMessage ("The maximum average resources per attribute per planet for a personal GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum average resources per attribute per planet for a personal GameClass");
	                }
	            }

	            // MaxInitialTechLevel
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxInitialTechLevel")) == NULL) {
	                goto Redirection;
	            }
	            fNewValue = pHttpForm->GetFloatValue();

	            if (fNewValue != vMaxInitialTechLevelPersonal.GetFloat()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxInitialTechLevelPersonal, fNewValue) == OK) {
	                    AddMessage ("The maximum initial tech level for a personal GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum initial tech level for a personal GameClass");
	                }
	            }

	            // MaxTechDev
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxTechDev")) == NULL) {
	                goto Redirection;
	            }
	            fNewValue = pHttpForm->GetFloatValue();

	            if (fNewValue != vMaxTechDevPersonal.GetFloat()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::MaxTechDevPersonal, fNewValue) == OK) {
	                    AddMessage ("The maximum tech development for a personal GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum tech development for a personal GameClass");
	                }
	            }

	            // Min update period
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMinHrs")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue() * 3600;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMinMin")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue() * 60;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMinSec")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue();

	            if (iNewValue != iPersonalMinNumSecsPerUpdate) {
	                if (g_pGameEngine->SetMinNumSecsPerUpdateForPersonalGameClass (iNewValue) == OK) {
	                    AddMessage ("The minimum update period for a personal GameClass was updated");
	                } else {
	                    AddMessage ("Invalid minimum update period for a personal GameClass");
	                }
	            }

	            // Max update period
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxHrs")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue() * 3600;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxMin")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue() * 60;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxSec")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue();

	            if (iNewValue != iPersonalMaxNumSecsPerUpdate) {
	                if (g_pGameEngine->SetMaxNumSecsPerUpdateForPersonalGameClass (iNewValue) == OK) {
	                    AddMessage ("The maximum update period for a personal GameClass was updated");
	                } else {
	                    AddMessage ("Invalid maximum update period for a personal GameClass");
	                }
	            }

	            // UnlimitedPrivilege
	            if ((pHttpForm = m_pHttpRequest->GetForm ("UnlimitedPrivilege")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != vUnlimitedEmpirePrivilege.GetInteger()) {
	                if (g_pGameEngine->SetSystemProperty (SystemData::PrivilegeForUnlimitedEmpires, iNewValue) == OK) {
	                    AddMessage ("The privilege level required to create games with unlimited empires was updated");
	                } else {
	                    AddMessage ("The privilege level required to create games with unlimited empires could not be set");
	                }
	            }
	            
	            // PauseGames
	            if ((pHttpForm = m_pHttpRequest->GetForm ("PauseGames")) == NULL) {
	                goto Redirection;
	            }
	            bNewValue = pHttpForm->GetIntValue() != 0;

	            if (bNewValue != ((iSystemOptions & PAUSE_GAMES_BY_DEFAULT) != 0)) {
	            
	                Check (g_pGameEngine->SetSystemOption (PAUSE_GAMES_BY_DEFAULT, bNewValue));
	                AddMessage ("The default pause game setting was updated");
	            }

	            // AfterWeekendDelay
	            if ((pHttpForm = m_pHttpRequest->GetForm ("AfterWeekendDelayHrs")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue() * 3600;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("AfterWeekendDelayMin")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue() * 60;
	            if ((pHttpForm = m_pHttpRequest->GetForm ("AfterWeekendDelaySec")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue += pHttpForm->GetIntValue();

	            if (iNewValue != vDelay.GetInteger()) {

	                if (iNewValue > MAX_AFTER_WEEKEND_DELAY) {

	                    AddMessage ("The after weekend delay was greater than ");
	                    AppendMessage (MAX_AFTER_WEEKEND_DELAY / (60 * 60));
	                    AppendMessage (" hours");

	                } else {

	                    if (g_pGameEngine->SetSystemProperty (SystemData::AfterWeekendDelay, iNewValue) == OK) {
	                        AddMessage ("The after weekend delay was updated");
	                    } else {
	                        AddMessage ("The after weekend delay could not be updated");
	                    }
	                }
	            }

	            //
	            // Game configuration
	            //

	            gcNewConfig.iShipBehavior = 0;

	            // DamageDest
	            if ((pHttpForm = m_pHttpRequest->GetForm ("DamageDest")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iPercentDamageUsedToDestroy = pHttpForm->GetIntValue();

	            // FirstTradeIncrease
	            if ((pHttpForm = m_pHttpRequest->GetForm ("FirstTradeIncrease")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iPercentFirstTradeIncrease = pHttpForm->GetIntValue();

	            // NextTradeOnIncrease
	            if ((pHttpForm = m_pHttpRequest->GetForm ("NextTradeOnIncrease")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iPercentNextTradeIncrease = pHttpForm->GetIntValue();

	            // TechForLatecomers
	            if ((pHttpForm = m_pHttpRequest->GetForm ("TechForLatecomers")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iPercentTechIncreaseForLatecomers = pHttpForm->GetIntValue();

	            // NukesForQuarantine
	            if ((pHttpForm = m_pHttpRequest->GetForm ("NukesForQuarantine")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iNukesForQuarantine = pHttpForm->GetIntValue();

	            // UpdatesInQuarantine
	            if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesInQuarantine")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iUpdatesInQuarantine = pHttpForm->GetIntValue();

	            // ColonyBehavior
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ColonyBehavior")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iShipBehavior |= pHttpForm->GetIntValue();

	            // ColonySimpleBuildCost
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ColonySimpleBuildCost")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iColonySimpleBuildFactor = pHttpForm->GetIntValue();

	            // ColonyMultipliedBuildCost
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ColonyMultipliedBuildCost")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fColonyMultipliedBuildFactor = pHttpForm->GetFloatValue();

	            // ColonyMultipliedDepositFactor
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ColonyMultipliedDepositFactor")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fColonyMultipliedDepositFactor = pHttpForm->GetFloatValue();

	            // EngineerCost
	            if ((pHttpForm = m_pHttpRequest->GetForm ("EngineerCost")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fEngineerLinkCost = pHttpForm->GetFloatValue();

	            // ColonyExponentialDepositFactor
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ColonyExponentialDepositFactor")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fColonyExponentialDepositFactor = pHttpForm->GetFloatValue();

	            // StargateBehavior
	            if ((pHttpForm = m_pHttpRequest->GetForm ("StargateBehavior")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iShipBehavior |= pHttpForm->GetIntValue();

	            // StargateCost
	            if ((pHttpForm = m_pHttpRequest->GetForm ("StargateCost")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fStargateGateCost = pHttpForm->GetFloatValue();

	            // StargateRange
	            if ((pHttpForm = m_pHttpRequest->GetForm ("StargateRange")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fStargateRangeFactor = pHttpForm->GetFloatValue();

	            // JumpgateBehavior
	            if ((pHttpForm = m_pHttpRequest->GetForm ("JumpgateBehavior")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iShipBehavior |= pHttpForm->GetIntValue();

	            // JumpgateCost
	            if ((pHttpForm = m_pHttpRequest->GetForm ("JumpgateCost")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fJumpgateGateCost = pHttpForm->GetFloatValue();

	            // JumpgateRange
	            if ((pHttpForm = m_pHttpRequest->GetForm ("JumpgateRange")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fJumpgateRangeFactor = pHttpForm->GetFloatValue();

	            // TerraformerFactor
	            if ((pHttpForm = m_pHttpRequest->GetForm ("TerraformerFactor")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fTerraformerPlowFactor = pHttpForm->GetFloatValue();

	            // TroopshipInvasion
	            if ((pHttpForm = m_pHttpRequest->GetForm ("TroopshipInvasion")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fTroopshipInvasionFactor = pHttpForm->GetFloatValue();

	            // TroopshipFailure
	            if ((pHttpForm = m_pHttpRequest->GetForm ("TroopshipFailure")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fTroopshipFailureFactor = pHttpForm->GetFloatValue();

	            // TroopshipSuccess
	            if ((pHttpForm = m_pHttpRequest->GetForm ("TroopshipSuccess")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fTroopshipSuccessFactor = pHttpForm->GetFloatValue();

	            // DoomsdayMultiplier
	            if ((pHttpForm = m_pHttpRequest->GetForm ("DoomsdayMultiplier")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fDoomsdayAnnihilationFactor = pHttpForm->GetFloatValue();

	            // CarrierCost
	            if ((pHttpForm = m_pHttpRequest->GetForm ("CarrierCost")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fCarrierCost = pHttpForm->GetFloatValue();

	            // MinBuilderBR
	            if ((pHttpForm = m_pHttpRequest->GetForm ("MinBuilderBR")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fBuilderMinBR = pHttpForm->GetFloatValue();

	            // BuilderDampener
	            if ((pHttpForm = m_pHttpRequest->GetForm ("BuilderDampener")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fBuilderBRDampener = pHttpForm->GetFloatValue();

	            // BuilderMultiplier
	            if ((pHttpForm = m_pHttpRequest->GetForm ("BuilderMultiplier")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fBuilderMultiplier = pHttpForm->GetFloatValue();

	            // CloakerBehavior
	            if ((pHttpForm = m_pHttpRequest->GetForm ("CloakerBehavior")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iShipBehavior |= pHttpForm->GetIntValue();

	            // MorpherBehavior
	            if ((pHttpForm = m_pHttpRequest->GetForm ("MorpherBehavior")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.iShipBehavior |= pHttpForm->GetIntValue();

	            // MorpherCost
	            if ((pHttpForm = m_pHttpRequest->GetForm ("MorpherCost")) == NULL) {
	                goto Redirection;
	            }
	            gcNewConfig.fMorpherCost = pHttpForm->GetFloatValue();

	            // DisableFriendlyTerraformers
	            if (m_pHttpRequest->GetForm ("DisableFriendlyTerraformers") != NULL) {
	                gcNewConfig.iShipBehavior |= TERRAFORMER_DISABLE_FRIENDLY;
	            }

	            if (m_pHttpRequest->GetForm ("DisableMultipleTerraformers") != NULL) {
	                gcNewConfig.iShipBehavior |= TERRAFORMER_DISABLE_MULTIPLE;
	            }

	            // DisableTerraformerSurvival
	            if (m_pHttpRequest->GetForm ("DisableTerraformerSurvival") != NULL) {
	                gcNewConfig.iShipBehavior |= TERRAFORMER_DISABLE_SURVIVAL;
	            }

	            // DisableTroopshipSurvival
	            if (m_pHttpRequest->GetForm ("DisableTroopshipSurvival") != NULL) {
	                gcNewConfig.iShipBehavior |= TROOPSHIP_DISABLE_SURVIVAL;
	            }

	            // DisableMinefieldDetonate
	            if (m_pHttpRequest->GetForm ("DisableMinefieldDetonate") != NULL) {
	                gcNewConfig.iShipBehavior |= MINEFIELD_DISABLE_DETONATE;
	            }

	            // Update game configuration?
	            if (memcmp (&gcNewConfig, &gcOldConfig, sizeof (GameConfiguration)) != 0) {

	                if (g_pGameEngine->SetGameConfiguration (gcNewConfig) == OK) {
	                    AddMessage ("The server's game configuration was updated");
	                } else {
	                    AddMessage ("The server's game configuration could not be updated");
	                }
	            }

	            //
	            // Map configuration
	            //

	            // ChanceNewLinkForms
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ChanceNewLinkForms")) == NULL) {
	                goto Redirection;
	            }
	            mcNewConfig.iChanceNewLinkForms = pHttpForm->GetIntValue();

	            // ResourceRand
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ResourceRand")) == NULL) {
	                goto Redirection;
	            }
	            mcNewConfig.fResourceAllocationRandomizationFactor = pHttpForm->GetFloatValue();

	            // MapDeviation
	            if ((pHttpForm = m_pHttpRequest->GetForm ("MapDeviation")) == NULL) {
	                goto Redirection;
	            }
	            mcNewConfig.iMapDeviation = pHttpForm->GetIntValue();

	            // ChanceLinkedLarge
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ChanceLinkedLarge")) == NULL) {
	                goto Redirection;
	            }
	            mcNewConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap = pHttpForm->GetIntValue();

	            // ChanceLinkedSmall
	            if ((pHttpForm = m_pHttpRequest->GetForm ("ChanceLinkedSmall")) == NULL) {
	                goto Redirection;
	            }
	            mcNewConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap = pHttpForm->GetIntValue();

	            // LargeMapThreshold
	            if ((pHttpForm = m_pHttpRequest->GetForm ("LargeMapThreshold")) == NULL) {
	                goto Redirection;
	            }
	            mcNewConfig.iLargeMapThreshold = pHttpForm->GetIntValue();

	            // Update map configuration?
	            if (memcmp (&mcNewConfig, &mcOldConfig, sizeof (MapConfiguration)) != 0) {

	                if (g_pGameEngine->SetMapConfiguration (mcNewConfig) == OK) {
	                    AddMessage ("The server's map configuration was updated");
	                } else {
	                    AddMessage ("The server's map configuration could not be updated");
	                }
	            }

	            // Handle active game information
	            if (WasButtonPressed (BID_VIEWGAMEINFORMATION)) {
	                iGameAdminPage = 1;
	                break;
	            }

	            // Handle super class creation
	            if (WasButtonPressed (BID_CREATENEWSUPERCLASS)) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("NewSuperClassName")) == NULL) {
	                    goto Redirection;
	                }
	                pszNewValue = pHttpForm->GetValue();

	                if (VerifyCategoryName ("SuperClass", pszNewValue, MAX_SUPER_CLASS_NAME_LENGTH, true) == OK) {

	                    int iKey;
	                    iErrCode = g_pGameEngine->CreateSuperClass (pszNewValue, &iKey);

	                    switch (iErrCode) {

	                    case OK:
	                        AddMessage ("The SuperClass ");
	                        AppendMessage (pszNewValue);
	                        AppendMessage (" was created");
	                        break;

	                    case ERROR_SUPERCLASS_ALREADY_EXISTS:
	                        AddMessage ("The SuperClass ");
	                        AppendMessage (pszNewValue);
	                        AppendMessage (" already exists");
	                        break;

	                    default:
	                        AddMessage ("The SuperClass could not be created. The error was ");
	                        AppendMessage (iErrCode);
	                        break;
	                    }
	                }

	                break;
	            }

	            // Handle super class deletion and rename
	            bool bFlag;

	            if (WasButtonPressed (BID_DELETESUPERCLASS)) {
	                if ((pHttpForm = m_pHttpRequest->GetForm ("DelSuperClass")) == NULL) {
	                    goto Redirection;
	                }
	                iErrCode = g_pGameEngine->DeleteSuperClass (pHttpForm->GetIntValue(), &bFlag);

	                if (iErrCode == OK) {
	                    if (bFlag) {
	                        AddMessage ("The SuperClass was deleted");
	                    } else {
	                        AddMessage ("The SuperClass could not be deleted because it still has GameClasses");
	                    }
	                } else {
	                    char pszMessage [256];
	                    sprintf (pszMessage, "Error %i occurred deleting the SuperClass", iErrCode);
	                    AddMessage (pszMessage);
	                }

	                break;
	            }

	            if (WasButtonPressed (BID_RENAMESUPERCLASS)) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("RenSuperClass")) == NULL) {
	                    goto Redirection;
	                }

	                int iKey = pHttpForm->GetIntValue();

	                if ((pHttpForm = m_pHttpRequest->GetForm ("RenSuperClassName")) == NULL ||
	                    pHttpForm->GetValue() == NULL) {
	                    goto Redirection;
	                }

	                iErrCode = g_pGameEngine->RenameSuperClass (iKey, pHttpForm->GetValue());

	                if (iErrCode == OK) {
	                    AddMessage ("The SuperClass was renamed");
	                }

	                else if (iErrCode == ERROR_SUPERCLASS_DOES_NOT_EXIST) {
	                    AddMessage ("The SuperClass does not exist");
	                }

	                else if (iErrCode == ERROR_INVALID_ARGUMENT) {
	                    AddMessage ("The new SuperClass name was invalid");
	                }

	                else {
	                    char pszMessage [256];
	                    sprintf (pszMessage, "Error %i occurred renaming the SuperClass", iErrCode);
	                    AddMessage (pszMessage);
	                }

	                break;
	            }

	            // Handle game class creation
	            if (WasButtonPressed (BID_CREATENEWGAMECLASS)) {
	                iGameAdminPage = 2;
	                break;
	            }

	            // Handle game class deletion
	            if (WasButtonPressed (BID_DELETEGAMECLASS)) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteGC")) == NULL) {
	                    goto Redirection;
	                }

	                iErrCode = g_pGameEngine->DeleteGameClass (pHttpForm->GetIntValue(), &bFlag);

	                if (iErrCode == OK) {
	                    if (bFlag) {
	                        AddMessage ("The GameClass was deleted");
	                    } else {
	                        AddMessage ("The GameClass has been marked for deletion");
	                    }
	                } else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
	                    AddMessage ("The GameClass no longer exists");
	                }
	                else {
	                    char pszMessage [256];
	                    sprintf (pszMessage, "Error %i occurred deleting the gameclass", iErrCode);
	                    AddMessage (pszMessage);
	                }
	                break;
	            }

	            // Handle game class undeletion
	            if (WasButtonPressed (BID_UNDELETEGAMECLASS)) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("UndeleteGC")) == NULL) {
	                    goto Redirection;
	                }

	                iErrCode = g_pGameEngine->UndeleteGameClass (pHttpForm->GetIntValue());
	                switch (iErrCode) {

	                case OK:

	                    AddMessage ("The GameClass was undeleted");
	                    break;

	                case ERROR_GAMECLASS_DOES_NOT_EXIST:

	                    AddMessage ("The GameClass no longer exists");
	                    break;

	                case ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION:

	                    AddMessage ("The GameClass was not marked for deletion");
	                    break;

	                default:

	                    {
	                    char pszMessage [256];
	                    sprintf (pszMessage, "Error %i occurred undeleting the GameClass", iErrCode);
	                    AddMessage (pszMessage);
	                    }

	                    break;
	                }
	                break;
	            }

	            // Handle game class halt
	            if (WasButtonPressed (BID_HALTGAMECLASS)) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("HaltGC")) == NULL) {
	                    goto Redirection;
	                }
	                iErrCode = g_pGameEngine->HaltGameClass (pHttpForm->GetIntValue());
	                if (iErrCode == OK) {
	                    AddMessage ("The GameClass was halted");
	                }
	                else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
	                    AddMessage ("The GameClass no longer exists");
	                }
	                else {
	                    char pszMessage [256];
	                    sprintf (pszMessage, "Error %i occurred halting the GameClass", iErrCode);
	                    AddMessage (pszMessage);
	                }
	                break;
	            }

	            // Handle game class unhalting
	            if (WasButtonPressed (BID_UNHALTGAMECLASS)) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("UnhaltGC")) == NULL) {
	                    goto Redirection;
	                }

	                iErrCode = g_pGameEngine->UnhaltGameClass (pHttpForm->GetIntValue());
	                switch (iErrCode) {

	                case OK:

	                    AddMessage ("The GameClass is no longer halted");
	                    break;

	                case ERROR_GAMECLASS_DOES_NOT_EXIST:

	                    AddMessage ("The GameClass no longer exists");
	                    break;

	                case ERROR_GAMECLASS_NOT_HALTED:

	                    AddMessage ("The GameClass was not halted");
	                    break;

	                default:

	                    {
	                    char pszMessage [256];
	                    sprintf (pszMessage, "Error %i occurred unhalting the GameClass", iErrCode);
	                    AddMessage (pszMessage);
	                    }

	                    break;
	                }
	                break;
	            }

	            // Handle pause all games
	            if (WasButtonPressed (BID_PAUSEALLGAMES)) {

	                iErrCode = g_pGameEngine->PauseAllGames();

	                if (iErrCode == OK) {
	                    AddMessage ("All games were paused");
	                } else {
	                    char pszMessage [256];
	                    sprintf (pszMessage, "Error %i occurred pausing all games", iErrCode);
	                    AddMessage (pszMessage);
	                }
	            }

	            // Handle unpause all games
	            if (WasButtonPressed (BID_UNPAUSEALLGAMES)) {

	                iErrCode = g_pGameEngine->UnpauseAllGames();

	                if (iErrCode == OK) {
	                    AddMessage ("All games were unpaused");
	                } else {
	                    char pszMessage [256];
	                    sprintf (pszMessage, "Error %i occurred unpausing all games", iErrCode);
	                    AddMessage (pszMessage);
	                }
	            }


	            }
	            break;

	        case 1:
	            {

	            const char* pszStart;

	            // Administer game
	            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("AdministerGame")) != NULL && 
	                (pszStart = pHttpForm->GetName()) != NULL &&
	                sscanf (pszStart, "AdministerGame%d.%d", &iGameClass, &iGameNumber) == 2) {

	                iGameAdminPage = 3;
	            }

	            }
	            break;

	        case 2:
	            {

	            if (WasButtonPressed (BID_CREATENEWGAMECLASS) &&
	                ProcessCreateGameClassForms (SYSTEM, NO_KEY) != OK) {
	                iGameAdminPage = 2;
	            }

	            }
	            break;

	        case 3:

	            {

	            bool bExist;
	            const char* pszMessage;

	            // Get game class, game number
	            if ((pHttpForm  = m_pHttpRequest->GetForm ("GameClass")) == NULL ||
	                (pHttpForm2 = m_pHttpRequest->GetForm ("GameNumber")) == NULL) {
	                break;
	            }
	            iGameClass  = pHttpForm->GetIntValue();
	            iGameNumber = pHttpForm2->GetIntValue();

	            // View map
	            if (WasButtonPressed (BID_VIEWMAP)) {
	                bRedirectTest = false;
	                iGameAdminPage = 4;
	                break;
	            }

	            // Change password
	            if (WasButtonPressed (BID_CHANGEPASSWORD)) {

	                bRedirectTest = false;

	                pHttpForm = m_pHttpRequest->GetForm ("NewPassword");
	                if (pHttpForm == NULL) {
	                    break;
	                }
	                pszMessage = pHttpForm->GetValue();

	                if (!String::IsBlank (pszMessage) &&
	                    VerifyPassword (pszMessage) != OK) {
	                    break;
	                }

	                iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	                if (iErrCode != OK) {
	                    iGameAdminPage = 1;
	                    AddMessage ("The game no longer exists");
	                } else {

	                    iErrCode = g_pGameEngine->SetGamePassword (iGameClass, iGameNumber, pszMessage);
	                    if (iErrCode == OK) {

	                        if (String::IsBlank (pszMessage)) {
	                            AddMessage ("The game password was removed");
	                        } else {
	                            AddMessage ("The game password was updated");
	                        }
	                        iGameAdminPage = 3;
	                    } else {
	                        AddMessage ("The game no longer exists");
	                        iGameAdminPage = 1;
	                    }
	                }

	                iErrCode = g_pGameEngine->SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	                if (iErrCode != OK) {
	                    AddMessage ("The game no longer exists");
	                    iGameAdminPage = 1;
	                }

	                break;
	            }

	            // Force update
	            if (WasButtonPressed (BID_FORCEUPDATE)) {

	                if (g_pGameEngine->ForceUpdate (iGameClass, iGameNumber) == OK) {
	                    AddMessage ("The game was forcibly updated");
	                } else {
	                    AddMessage ("The game no longer exists");
	                }

	                if (g_pGameEngine->DoesGameExist (iGameClass, iGameNumber, &bExist) == OK && bExist) {
	                    iGameAdminPage = 3;
	                } else {
	                    iGameAdminPage = 1;
	                }

	                bRedirectTest = false;
	                break;
	            }

	            // Delete empire from game
	            if (WasButtonPressed (BID_DELETEEMPIRE)) {

	                pHttpForm = m_pHttpRequest->GetForm ("DeleteEmpireKey");
	                if (pHttpForm != NULL) {

	                    int iTargetEmpireKey = pHttpForm->GetIntValue();

	                    // Lock game as writer
	                    iErrCode = g_pGameEngine->WaitGameWriter (iGameClass, iGameNumber);

	                    if (iErrCode == OK) {

	                        if (!(m_iGameState & STARTED)) {

	                            iErrCode = g_pGameEngine->QuitEmpireFromGame (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
	                            if (iErrCode == ERROR_GAME_HAS_STARTED) {

	                                // Try remove
	                                iErrCode = g_pGameEngine->RemoveEmpireFromGame (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
	                            }

	                        } else {

	                            if (m_iGameState & STILL_OPEN) {
	                                iErrCode = g_pGameEngine->RemoveEmpireFromGame (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
	                            }
	                        }

	                        g_pGameEngine->SignalGameWriter (iGameClass, iGameNumber);
	                    }

	                    if (iErrCode == OK) {
	                        AddMessage ("The empire was deleted from the game");
	                    } else {
	                        AddMessage ("The empire could not be deleted from the game");
	                    }
	                }

	                if (g_pGameEngine->DoesGameExist (iGameClass, iGameNumber, &bExist) == OK && bExist) {
	                    iGameAdminPage = 3;
	                } else {
	                    iGameAdminPage = 1;
	                }

	                bRedirectTest = false;
	                break;
	            }

	            // Restore resigned empire to game
	            if (WasButtonPressed (BID_RESTOREEMPIRE)) {

	                pHttpForm = m_pHttpRequest->GetForm ("RestoreEmpireKey");
	                if (pHttpForm != NULL) {

	                    int iTargetEmpireKey = pHttpForm->GetIntValue();

	                    // Lock game as writer
	                    iErrCode = g_pGameEngine->WaitGameWriter (iGameClass, iGameNumber);

	                    if (iErrCode == OK) {

	                        if (!(m_iGameState & STARTED)) {

	                            iErrCode = g_pGameEngine->UnresignEmpire (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
	                            if (iErrCode == OK) {
	                                AddMessage ("The empire was restored");
	                            } else {
	                                char pszMessage [256];
	                                sprintf (pszMessage, "Error %i occurred restoring the empire", iErrCode);
	                                AddMessage (pszMessage);
	                            }
	                        }

	                        g_pGameEngine->SignalGameWriter (iGameClass, iGameNumber);
	                    }
	                }

	                if (g_pGameEngine->DoesGameExist (iGameClass, iGameNumber, &bExist) == OK && bExist) {
	                    iGameAdminPage = 3;
	                } else {
	                    iGameAdminPage = 1;
	                }

	                bRedirectTest = false;
	                break;
	            }

	            // Check for search for empires with duplicate IP's
	            if (WasButtonPressed (BID_SEARCHIPADDRESSES)) {

	                iGameAdminPage = 3;
	                bRedirectTest = false;
	                SearchForDuplicateIPAddresses (iGameClass, iGameNumber);
	                break;
	            }

	            // Check for search for empires with duplicate IP's
	            if (WasButtonPressed (BID_SEARCHSESSIONIDS)) {

	                iGameAdminPage = 3;
	                bRedirectTest = false;
	                SearchForDuplicateSessionIds (iGameClass, iGameNumber);
	                break;
	            }

	            // Check for view empire info
	            if (WasButtonPressed (BID_VIEWEMPIREINFORMATION)) {

	                iGameAdminPage = 7;
	                bRedirectTest = false;
	                break;
	            }

	            // Pause game
	            if (WasButtonPressed (BID_PAUSEGAME)) {

	                iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	                if (iErrCode == OK) {
	                    iErrCode = g_pGameEngine->PauseGame (iGameClass, iGameNumber, true, true);
	                    g_pGameEngine->SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	                }

	                if (iErrCode == OK) {
	                    AddMessage ("The game is now paused");
	                    iGameAdminPage = 3;
	                } else {
	                    AddMessage ("The game no longer exists");
	                    iGameAdminPage = 1;
	                }

	                bRedirectTest = false;
	                break;
	            }

	            // Unpause game
	            if (WasButtonPressed (BID_UNPAUSEGAME)) {

	                iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	                if (iErrCode == OK) {
	                    iErrCode = g_pGameEngine->UnpauseGame (iGameClass, iGameNumber, true, true);
	                    g_pGameEngine->SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	                }

	                if (iErrCode == OK) {
	                    AddMessage ("The game is no longer paused");
	                    iGameAdminPage = 3;
	                } else {
	                    AddMessage ("The game no longer exists");
	                    iGameAdminPage = 1;
	                }

	                bRedirectTest = false;
	                break;
	            }

	            // Broadcast message
	            if (WasButtonPressed (BID_SENDMESSAGE)) {

	                bRedirectTest = false;

	                if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL) {
	                    goto Redirection;
	                }
	                pszMessage = pHttpForm->GetValue();

	                if (g_pGameEngine->WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL) != OK) {
	                    iGameAdminPage = 1;
	                    AddMessage ("That game no longer exists");
	                } else {

	                    if ((iErrCode = g_pGameEngine->BroadcastGameMessage (
	                        iGameClass,
	                        iGameNumber,
	                        pszMessage,
	                        m_iEmpireKey,
	                        MESSAGE_BROADCAST | MESSAGE_ADMINISTRATOR)
	                        ) == OK) {
	                        AddMessage ("Your message was broadcast to all empires in the game");
	                    } else {
	                        AddMessage ("The game no longer exists");
	                    }

	                    if (g_pGameEngine->SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL) == OK) {
	                        iGameAdminPage = 3;
	                    } else {
	                        iGameAdminPage = 1;
	                    }

	                }
	                break;
	            }

	            // Kill game
	            if (WasButtonPressed (BID_KILLGAME)) {

	                bRedirectTest = false;
	                iGameAdminPage = 8;
	                break;
	            }

	            }
	            break;

	        case 4:

	            {

	            const char* pszStart;
	            int iClickedProxyPlanetKey;

	            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Planet")) != NULL && 
	                (pszStart = pHttpForm->GetName()) != NULL &&
	                sscanf (pszStart, "Planet%d.%d.x", &iClickedPlanetKey, &iClickedProxyPlanetKey) == 2) {

	                if ((pHttpForm  = m_pHttpRequest->GetFormBeginsWith ("GameClass")) == NULL ||
	                    (pHttpForm2 = m_pHttpRequest->GetFormBeginsWith ("GameNumber")) == NULL) {
	                    break;
	                }

	                iGameClass  = pHttpForm->GetIntValue();
	                iGameNumber = pHttpForm2->GetIntValue();

	                // We clicked on a planet
	                iGameAdminPage = 6;
	                bRedirectTest = false;
	            }

	            }
	            break;

	        case 6:

	            {

	            // View map
	            if (WasButtonPressed (BID_VIEWMAP)) {

	                // Get game class, game number
	                if ((pHttpForm  = m_pHttpRequest->GetFormBeginsWith ("GameClass")) == NULL ||
	                    (pHttpForm2 = m_pHttpRequest->GetFormBeginsWith ("GameNumber")) == NULL) {
	                    break;
	                }
	                iGameClass  = pHttpForm->GetIntValue();
	                iGameNumber = pHttpForm2->GetIntValue();

	                bRedirectTest = false;
	                iGameAdminPage = 4;
	                break;
	            }

	            }
	            break;

	        case 7:

	            break;

	        case 8:

	            // Kill game
	            if (WasButtonPressed (BID_KILLGAME)) {

	                // Get game class, game number
	                if ((pHttpForm  = m_pHttpRequest->GetFormBeginsWith ("GameClass")) == NULL ||
	                    (pHttpForm2 = m_pHttpRequest->GetFormBeginsWith ("GameNumber")) == NULL) {
	                    break;
	                }
	                iGameClass  = pHttpForm->GetIntValue();
	                iGameNumber = pHttpForm2->GetIntValue();

	                bRedirectTest = false;

	                if ((pHttpForm = m_pHttpRequest->GetForm ("DoomMessage")) == NULL) {
	                    break;
	                }
	                const char* pszMessage = pHttpForm->GetValue();

	                if (g_pGameEngine->DeleteGame (iGameClass, iGameNumber, m_iEmpireKey, pszMessage, 0) == OK) {
	                    AddMessage ("The game was deleted");
	                } else {
	                    AddMessage ("The game no longer exists");
	                }

	                iGameAdminPage = 1;
	            }
	            break;

	        default:
	            Assert (false);
	            break;
	        }
	    }
	} 

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)

	switch (iGameAdminPage) {

	case 0:
	    {

	    int* piSuperClassKey, iNumSuperClasses;
	    Variant* pvSuperClassName;
	    Check (g_pGameEngine->GetSuperClassKeys (&piSuperClassKey, &pvSuperClassName, &iNumSuperClasses));

	    int *piGameClassKey, iNumGameClasses;
	    bool* pbGameClassHalted, * pbGameClassDeleted, bFlag;
	    Check (g_pGameEngine->GetSystemGameClassKeys (&piGameClassKey, &pbGameClassHalted, &pbGameClassDeleted, &iNumGameClasses));

	    Algorithm::AutoDelete<int> auto1 (piGameClassKey);
	    Algorithm::AutoDelete<bool> auto2 (pbGameClassHalted);
	    Algorithm::AutoDelete<bool> auto3 (pbGameClassDeleted);

	    GameConfiguration gcConfig;
	    MapConfiguration mcConfig;

	    int iNumActiveGames, iNumOpenGames, iNumClosedGames;
	    Variant vMaxNumUpdatesBeforeClose, vDefaultNumUpdatesBeforeClose, vMaxNumPersonalGameClasses, vDelay,
	        vMaxNumT, vMaxNumPersonalGameClassesT;

	    Check (g_pGameEngine->GetNumActiveGames (&iNumActiveGames));
	    Check (g_pGameEngine->GetNumOpenGames (&iNumOpenGames));
	    Check (g_pGameEngine->GetNumClosedGames (&iNumClosedGames));

	    int iSystemMinNumSecsPerUpdate, iSystemMaxNumSecsPerUpdate, iSystemMaxNumEmpires, iSystemMaxNumPlanets,
	        iPersonalMinNumSecsPerUpdate, iPersonalMaxNumSecsPerUpdate, iPersonalMaxNumEmpires, 
	        iPersonalMaxNumPlanets, iValue, iSystemOptions;

	    Check (g_pGameEngine->GetMinNumSecsPerUpdateForSystemGameClass (&iSystemMinNumSecsPerUpdate));
	    Check (g_pGameEngine->GetMaxNumSecsPerUpdateForSystemGameClass (&iSystemMaxNumSecsPerUpdate));
	    Check (g_pGameEngine->GetMaxNumEmpiresForSystemGameClass (&iSystemMaxNumEmpires));
	    Check (g_pGameEngine->GetMaxNumPlanetsForSystemGameClass (&iSystemMaxNumPlanets));

	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxNumUpdatesBeforeClose, &vMaxNumUpdatesBeforeClose));
	    Check (g_pGameEngine->GetSystemProperty (SystemData::DefaultNumUpdatesBeforeClose, &vDefaultNumUpdatesBeforeClose));

	    Check (g_pGameEngine->GetMinNumSecsPerUpdateForPersonalGameClass (&iPersonalMinNumSecsPerUpdate));
	    Check (g_pGameEngine->GetMaxNumSecsPerUpdateForPersonalGameClass (&iPersonalMaxNumSecsPerUpdate));
	    Check (g_pGameEngine->GetMaxNumEmpiresForPersonalGameClass (&iPersonalMaxNumEmpires));
	    Check (g_pGameEngine->GetMaxNumPlanetsForPersonalGameClass (&iPersonalMaxNumPlanets));

	    Variant vMaxResourcesPerPlanet, vMaxResourcesPerPlanetPersonal, vMaxInitialTechLevel, 
	        vMaxInitialTechLevelPersonal, vMaxTechDev, vMaxTechDevPersonal, vUnlimitedEmpirePrivilege;

	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxResourcesPerPlanet, &vMaxResourcesPerPlanet));
	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, &vMaxResourcesPerPlanetPersonal));

	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxInitialTechLevel, &vMaxInitialTechLevel));
	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxInitialTechLevelPersonal, &vMaxInitialTechLevelPersonal));

	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxTechDev, &vMaxTechDev));
	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxTechDevPersonal, &vMaxTechDevPersonal));

	    Check (g_pGameEngine->GetGameConfiguration (&gcConfig));
	    Check (g_pGameEngine->GetMapConfiguration (&mcConfig));

	    Check (g_pGameEngine->GetSystemProperty (SystemData::AfterWeekendDelay, &vDelay));

	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxNumPersonalGameClasses, &vMaxNumPersonalGameClasses));
	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxNumPersonalTournaments, &vMaxNumT));
	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxNumGameClassesPerPersonalTournament, &vMaxNumPersonalGameClassesT));

	    Check (g_pGameEngine->GetSystemOptions (&iSystemOptions));

	    Check (g_pGameEngine->GetSystemProperty (SystemData::PrivilegeForUnlimitedEmpires, &vUnlimitedEmpirePrivilege));

	    
	Write ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"0\"><h3>Game Management</h3><p><table width=\"75%\">", sizeof ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"0\"><h3>Game Management</h3><p><table width=\"75%\">") - 1);
	// Active games
	    
	Write ("<tr><td>There ", sizeof ("<tr><td>There ") - 1);
	if (iNumActiveGames == 1) { 
	        
	Write ("is <strong>1</strong> active game", sizeof ("is <strong>1</strong> active game") - 1);
	} else { 
	        
	Write ("are ", sizeof ("are ") - 1);
	if (iNumActiveGames == 0) {
	            
	Write ("no", sizeof ("no") - 1);
	} else {
	            
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (iNumActiveGames); 
	Write ("</strong>", sizeof ("</strong>") - 1);
	}
	        
	Write (" active games", sizeof (" active games") - 1);
	}
	    
	Write (" on the server", sizeof (" on the server") - 1);
	if (iNumActiveGames > 0) {

	        
	Write (" (<strong>", sizeof (" (<strong>") - 1);
	Write (iNumOpenGames); 
	Write ("</strong> open, <strong>", sizeof ("</strong> open, <strong>") - 1);
	Write (iNumClosedGames); 
	        
	Write ("</strong> closed)</td><td>", sizeof ("</strong> closed)</td><td>") - 1);
	WriteButton (BID_VIEWGAMEINFORMATION);
	    }

	    
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	if (iNumActiveGames > 0) {

	        
	Write ("<tr><td>&nbsp;</td><td>&nbsp;</td><tr><td>Pause all games:</td><td>", sizeof ("<tr><td>&nbsp;</td><td>&nbsp;</td><tr><td>Pause all games:</td><td>") - 1);
	WriteButton (BID_PAUSEALLGAMES);

	        
	Write ("</td></tr><tr><td>Unpause all games:</td><td>", sizeof ("</td></tr><tr><td>Unpause all games:</td><td>") - 1);
	WriteButton (BID_UNPAUSEALLGAMES);

	        
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	    
	Write ("</table><p><h3>SuperClasses and GameClasses</h3><p><table width=\"75%\">", sizeof ("</table><p><h3>SuperClasses and GameClasses</h3><p><table width=\"75%\">") - 1);
	// SuperClasses
	    if (iNumSuperClasses > 0) { 

	        
	Write ("<tr><td>Delete a SuperClass:</td><td><select name=\"DelSuperClass\">", sizeof ("<tr><td>Delete a SuperClass:</td><td><select name=\"DelSuperClass\">") - 1);
	for (i = 0; i < iNumSuperClasses; i ++) { 
	            
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piSuperClassKey [i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pvSuperClassName[i].GetCharPtr()); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	        
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_DELETESUPERCLASS);
	        
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	    
	Write ("<tr><td>Create a new SuperClass:</td><td><input type=\"text\" size=\"23\" maxlength=\"", sizeof ("<tr><td>Create a new SuperClass:</td><td><input type=\"text\" size=\"23\" maxlength=\"") - 1);
	Write (MAX_SUPER_CLASS_NAME_LENGTH); 
	Write ("\" name=\"NewSuperClassName\"></td><td>", sizeof ("\" name=\"NewSuperClassName\"></td><td>") - 1);
	WriteButton (BID_CREATENEWSUPERCLASS);
	    
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	if (iNumSuperClasses > 0) {

	        
	Write ("<tr><td>Rename a SuperClass:</td><td><select name=\"RenSuperClass\">", sizeof ("<tr><td>Rename a SuperClass:</td><td><select name=\"RenSuperClass\">") - 1);
	for (i = 0; i < iNumSuperClasses; i ++) { 
	            
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piSuperClassKey [i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pvSuperClassName[i].GetCharPtr()); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	        
	Write ("</select><br> to<br><input type=\"text\" size=\"23\" maxlength=\"", sizeof ("</select><br> to<br><input type=\"text\" size=\"23\" maxlength=\"") - 1);
	Write (MAX_SUPER_CLASS_NAME_LENGTH); 
	        
	Write ("\" name=\"RenSuperClassName\"></td><td>", sizeof ("\" name=\"RenSuperClassName\"></td><td>") - 1);
	WriteButton (BID_RENAMESUPERCLASS);
	        
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	    
	Write ("<tr><td>&nbsp;</td></td>", sizeof ("<tr><td>&nbsp;</td></td>") - 1);
	// GameClasses
	    if (iNumGameClasses > 0) {

	        int iNumHalted = 0, iNumNotHalted = 0, iNumDeleted = 0, iNumNotDeleted = 0;
	        for (i = 0; i < iNumGameClasses; i ++) {

	            if (pbGameClassHalted[i]) {
	                iNumHalted ++;
	                iNumNotDeleted ++;
	            }
	            else if (pbGameClassDeleted[i]) {
	                iNumDeleted ++;
	                iNumNotHalted ++;
	            }
	            else {
	                iNumNotDeleted ++;
	                iNumNotHalted ++;
	            }
	        }

	        if (iNumNotDeleted > 0) {

	            
	Write ("<tr><td>Delete a GameClass:</td><td><select name=\"DeleteGC\">", sizeof ("<tr><td>Delete a GameClass:</td><td><select name=\"DeleteGC\">") - 1);
	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	            for (i = 0; i < iNumGameClasses; i ++) { 
	                if (!pbGameClassDeleted[i] &&
	                    g_pGameEngine->GetGameClassName (piGameClassKey[i], pszGameClassName) == OK) {
	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piGameClassKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pszGameClassName); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_DELETEGAMECLASS);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	        if (iNumDeleted > 0) {

	            
	Write ("<tr><td>Undelete a GameClass:</td><td><select name=\"UndeleteGC\">", sizeof ("<tr><td>Undelete a GameClass:</td><td><select name=\"UndeleteGC\">") - 1);
	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	            for (i = 0; i < iNumGameClasses; i ++) { 
	                if (pbGameClassDeleted[i] &&
	                    g_pGameEngine->GetGameClassName (piGameClassKey[i], pszGameClassName) == OK) {
	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piGameClassKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pszGameClassName); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_UNDELETEGAMECLASS);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	        if (iNumNotHalted > 0) {

	            
	Write ("<tr><td>Halt a GameClass:</td><td><select name=\"HaltGC\">", sizeof ("<tr><td>Halt a GameClass:</td><td><select name=\"HaltGC\">") - 1);
	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	            for (i = 0; i < iNumGameClasses; i ++) { 
	                if (!pbGameClassHalted[i] &&
	                    g_pGameEngine->GetGameClassName (piGameClassKey[i], pszGameClassName) == OK) {
	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piGameClassKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pszGameClassName); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_HALTGAMECLASS);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	        if (iNumHalted > 0) {

	            
	Write ("<tr><td>Unhalt a GameClass:</td><td><select name=\"UnhaltGC\">", sizeof ("<tr><td>Unhalt a GameClass:</td><td><select name=\"UnhaltGC\">") - 1);
	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	            for (i = 0; i < iNumGameClasses; i ++) { 
	                if (pbGameClassHalted[i] &&
	                    g_pGameEngine->GetGameClassName (piGameClassKey[i], pszGameClassName) == OK) {
	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piGameClassKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pszGameClassName); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_UNHALTGAMECLASS);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}
	    }

	    
	Write ("<tr><td>Create a new GameClass:</td><td></td><td>", sizeof ("<tr><td>Create a new GameClass:</td><td></td><td>") - 1);
	WriteButton (BID_CREATENEWGAMECLASS);
	    
	Write ("</td></tr></table>", sizeof ("</td></tr></table>") - 1);
	// Parameters
	    int iNumHrs, iNumMin;

	    
	Write ("<p><h3>GameClass Parameters</h3><p><table width=\"75%\"><tr><td>Maximum number of empires in a System GameClass:</td><td><input type=\"text\" name=\"SMaxEmps\" size=\"5\" maxlength=\"20\" value=\"", sizeof ("<p><h3>GameClass Parameters</h3><p><table width=\"75%\"><tr><td>Maximum number of empires in a System GameClass:</td><td><input type=\"text\" name=\"SMaxEmps\" size=\"5\" maxlength=\"20\" value=\"") - 1);
	Write (iSystemMaxNumEmpires); 
	Write ("\"></td></tr><tr><td>Maximum number of planets in a System GameClass:</td><td><input type=\"text\" name=\"SMaxPlanets\" size=\"5\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum number of planets in a System GameClass:</td><td><input type=\"text\" name=\"SMaxPlanets\" size=\"5\" maxlength=\"20\" value=\"") - 1);
	Write (iSystemMaxNumPlanets); 
	Write ("\"></td></tr><tr><td>Maximum average number of resources per attribute per planet in a System GameClass:</td><td><input type=\"text\" name=\"SMaxResourcesPerPlanet\" size=\"5\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum average number of resources per attribute per planet in a System GameClass:</td><td><input type=\"text\" name=\"SMaxResourcesPerPlanet\" size=\"5\" maxlength=\"20\" value=\"") - 1);
	Write (vMaxResourcesPerPlanet.GetInteger()); 
	Write ("\"></td></tr><tr><td>Maximum initial tech level in a System GameClass:</td><td><input type=\"text\" name=\"SMaxInitialTechLevel\" size=\"10\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum initial tech level in a System GameClass:</td><td><input type=\"text\" name=\"SMaxInitialTechLevel\" size=\"10\" maxlength=\"20\" value=\"") - 1);
	Write (vMaxInitialTechLevel.GetFloat()); 
	Write ("\"></td></tr><tr><td>Maximum tech development in a System GameClass:</td><td><input type=\"text\" name=\"SMaxTechDev\" size=\"10\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum tech development in a System GameClass:</td><td><input type=\"text\" name=\"SMaxTechDev\" size=\"10\" maxlength=\"20\" value=\"") - 1);
	Write (vMaxTechDev.GetFloat()); 
	Write ("\"></td></tr><tr><td>Minimum update time in a System GameClass:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMinHrs\" value=\"", sizeof ("\"></td></tr><tr><td>Minimum update time in a System GameClass:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMinHrs\" value=\"") - 1);
	iNumHrs = iSystemMinNumSecsPerUpdate / 3600;
	    iSystemMinNumSecsPerUpdate -= iNumHrs * 3600;
	    Write (iNumHrs);
	    
	Write ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMinMin\" value=\"", sizeof ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMinMin\" value=\"") - 1);
	iNumMin = iSystemMinNumSecsPerUpdate / 60;
	    iSystemMinNumSecsPerUpdate -= iNumMin * 60;
	    Write (iNumMin);
	    
	Write ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMinSec\" value=\"", sizeof ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMinSec\" value=\"") - 1);
	Write (iSystemMinNumSecsPerUpdate);
	    
	Write ("\"> secs</td></tr><tr><td>Maximum update time in a System GameClass:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMaxHrs\" value=\"", sizeof ("\"> secs</td></tr><tr><td>Maximum update time in a System GameClass:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMaxHrs\" value=\"") - 1);
	iNumHrs = iSystemMaxNumSecsPerUpdate / 3600;
	    iSystemMaxNumSecsPerUpdate -= iNumHrs * 3600;
	    Write (iNumHrs);
	    
	Write ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMaxMin\" value=\"", sizeof ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMaxMin\" value=\"") - 1);
	iNumMin = iSystemMaxNumSecsPerUpdate / 60;
	    iSystemMaxNumSecsPerUpdate -= iNumMin * 60;
	    Write (iNumMin);
	    
	Write ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMaxSec\" value=\"", sizeof ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SMaxSec\" value=\"") - 1);
	Write (iSystemMaxNumSecsPerUpdate);
	    
	Write ("\"> secs</td></tr><tr><td>&nbsp;</td></td><tr><td>Maximum number of personal GameClasses per empire:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewMaxNumPGC\" value=\"", sizeof ("\"> secs</td></tr><tr><td>&nbsp;</td></td><tr><td>Maximum number of personal GameClasses per empire:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewMaxNumPGC\" value=\"") - 1);
	Write (vMaxNumPersonalGameClasses.GetInteger()); 
	Write ("\"><input type=\"hidden\" name=\"OldMaxNumPGC\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"OldMaxNumPGC\" value=\"") - 1);
	Write (vMaxNumPersonalGameClasses.GetInteger()); 
	Write ("\"></td></tr><tr><td>Maximum number of personal tournaments per empire:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewMaxNumT\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum number of personal tournaments per empire:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewMaxNumT\" value=\"") - 1);
	Write (vMaxNumT.GetInteger()); 
	Write ("\"><input type=\"hidden\" name=\"OldMaxNumT\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"OldMaxNumT\" value=\"") - 1);
	Write (vMaxNumT.GetInteger()); 
	Write ("\"></td></tr><tr><td>Maximum number of GameClasses per personal tournament:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewMaxNumPGCT\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum number of GameClasses per personal tournament:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewMaxNumPGCT\" value=\"") - 1);
	Write (vMaxNumPersonalGameClassesT.GetInteger()); 
	Write ("\"><input type=\"hidden\" name=\"OldMaxNumPGCT\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"OldMaxNumPGCT\" value=\"") - 1);
	Write (vMaxNumPersonalGameClassesT.GetInteger()); 
	Write ("\"></td></tr><tr><td>&nbsp;</td></td><tr><td>Maximum number of empires in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxEmps\" size=\"5\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>&nbsp;</td></td><tr><td>Maximum number of empires in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxEmps\" size=\"5\" maxlength=\"20\" value=\"") - 1);
	Write (iPersonalMaxNumEmpires); 
	Write ("\"></td></tr><tr><td>Maximum number of planets in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxPlanets\" size=\"5\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum number of planets in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxPlanets\" size=\"5\" maxlength=\"20\" value=\"") - 1);
	Write (iPersonalMaxNumPlanets); 
	Write ("\"></td></tr><tr><td>Maximum average number of resources per attribute per planet in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxResourcesPerPlanet\" size=\"5\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum average number of resources per attribute per planet in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxResourcesPerPlanet\" size=\"5\" maxlength=\"20\" value=\"") - 1);
	Write (vMaxResourcesPerPlanetPersonal.GetInteger()); 
	Write ("\"></td></tr><tr><td>Maximum initial tech level in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxInitialTechLevel\" size=\"10\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum initial tech level in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxInitialTechLevel\" size=\"10\" maxlength=\"20\" value=\"") - 1);
	Write (vMaxInitialTechLevelPersonal.GetFloat()); 
	Write ("\"></td></tr><tr><td>Maximum tech development in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxTechDev\" size=\"10\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum tech development in a personal GameClass:</td><td><input type=\"text\" name=\"PMaxTechDev\" size=\"10\" maxlength=\"20\" value=\"") - 1);
	Write (vMaxTechDevPersonal.GetFloat()); 
	Write ("\"></td></tr><tr><td>Minimum update time in a personal GameClass:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMinHrs\" value=\"", sizeof ("\"></td></tr><tr><td>Minimum update time in a personal GameClass:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMinHrs\" value=\"") - 1);
	iNumHrs = iPersonalMinNumSecsPerUpdate / 3600;
	    iPersonalMinNumSecsPerUpdate -= iNumHrs * 3600;
	    Write (iNumHrs);
	    
	Write ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMinMin\" value=\"", sizeof ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMinMin\" value=\"") - 1);
	iNumMin = iPersonalMinNumSecsPerUpdate / 60;
	    iPersonalMinNumSecsPerUpdate -= iNumMin * 60;
	    Write (iNumMin);
	    
	Write ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMinSec\" value=\"", sizeof ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMinSec\" value=\"") - 1);
	Write (iPersonalMinNumSecsPerUpdate);
	    
	Write ("\"> secs</td></tr><tr><td>Maximum update time in a personal GameClass:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMaxHrs\" value=\"", sizeof ("\"> secs</td></tr><tr><td>Maximum update time in a personal GameClass:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMaxHrs\" value=\"") - 1);
	iNumHrs = iPersonalMaxNumSecsPerUpdate / 3600;
	    iPersonalMaxNumSecsPerUpdate -= iNumHrs * 3600;
	    Write (iNumHrs);
	    
	Write ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMaxMin\" value=\"", sizeof ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMaxMin\" value=\"") - 1);
	iNumMin = iPersonalMaxNumSecsPerUpdate / 60;
	    iPersonalMaxNumSecsPerUpdate -= iNumMin * 60;
	    Write (iNumMin);
	    
	Write ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMaxSec\" value=\"", sizeof ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PMaxSec\" value=\"") - 1);
	Write (iPersonalMaxNumSecsPerUpdate);
	    
	Write ("\"> secs</td></tr><tr><td>&nbsp;</td><td>&nbsp;</td></tr><tr><td>Privilege level required to create Personal Games and GameClasses with unlimited empires:</td><td><select name=\"UnlimitedPrivilege\">", sizeof ("\"> secs</td></tr><tr><td>&nbsp;</td><td>&nbsp;</td></tr><tr><td>Privilege level required to create Personal Games and GameClasses with unlimited empires:</td><td><select name=\"UnlimitedPrivilege\">") - 1);
	int iUnlimitedEmpirePrivilege = vUnlimitedEmpirePrivilege.GetInteger();

	    for (i = NOVICE; i <= ADMINISTRATOR; i ++) {

	        
	Write ("<option ", sizeof ("<option ") - 1);
	if (i == iUnlimitedEmpirePrivilege) {
	            
	Write ("selected ", sizeof ("selected ") - 1);
	}

	        
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (i);
	        
	Write ("\">", sizeof ("\">") - 1);
	Write (PRIVILEGE_STRING[i]);
	        
	Write ("</option>", sizeof ("</option>") - 1);
	}
	    
	Write ("</td></tr><tr><td>Initial state of games:</td><td><select name=\"PauseGames\">", sizeof ("</td></tr><tr><td>Initial state of games:</td><td><select name=\"PauseGames\">") - 1);
	bFlag = (iSystemOptions & PAUSE_GAMES_BY_DEFAULT) != 0;
	    
	    
	Write ("<option value=\"0\" ", sizeof ("<option value=\"0\" ") - 1);
	if (!bFlag) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write (">Don't pause games by default when they start</option><option value=\"1\" ", sizeof (">Don't pause games by default when they start</option><option value=\"1\" ") - 1);
	if (bFlag) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write (">Pause games by default when they start</option></td></tr><tr><td>&nbsp;</td><td>&nbsp;</td></tr></table><h3>Game Parameters</h3><table width=\"75%\"><tr><td>Percent of damage used to destroy (DEST):</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"DamageDest\" value=\"", sizeof (">Pause games by default when they start</option></td></tr><tr><td>&nbsp;</td><td>&nbsp;</td></tr></table><h3>Game Parameters</h3><table width=\"75%\"><tr><td>Percent of damage used to destroy (DEST):</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"DamageDest\" value=\"") - 1);
	Write (gcConfig.iPercentDamageUsedToDestroy); 
	Write ("\"></td></tr><tr><td>Percent econ increase on first trade:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"FirstTradeIncrease\" value=\"", sizeof ("\"></td></tr><tr><td>Percent econ increase on first trade:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"FirstTradeIncrease\" value=\"") - 1);
	Write (gcConfig.iPercentFirstTradeIncrease); 
	Write ("\"></td></tr><tr><td>Percent of previous econ increase on next trade:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"NextTradeOnIncrease\" value=\"", sizeof ("\"></td></tr><tr><td>Percent of previous econ increase on next trade:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"NextTradeOnIncrease\" value=\"") - 1);
	Write (gcConfig.iPercentNextTradeIncrease); 
	Write ("\"></td></tr><tr><td>Percent of full tech increase for late entrances:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"TechForLatecomers\" value=\"", sizeof ("\"></td></tr><tr><td>Percent of full tech increase for late entrances:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"TechForLatecomers\" value=\"") - 1);
	Write (gcConfig.iPercentTechIncreaseForLatecomers); 
	Write ("\"></td></tr><tr><td>Number of nukes before planet is quarantined:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"NukesForQuarantine\" value=\"", sizeof ("\"></td></tr><tr><td>Number of nukes before planet is quarantined:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"NukesForQuarantine\" value=\"") - 1);
	Write (gcConfig.iNukesForQuarantine); 
	Write ("\"></td></tr><tr><td>Number of updates in quarantine after planet is nuked:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"UpdatesInQuarantine\" value=\"", sizeof ("\"></td></tr><tr><td>Number of updates in quarantine after planet is nuked:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"UpdatesInQuarantine\" value=\"") - 1);
	Write (gcConfig.iUpdatesInQuarantine); 
	Write ("\"></td></tr><tr><td>Update delay after a weekend for non-weekend-update games:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"AfterWeekendDelayHrs\" value=\"", sizeof ("\"></td></tr><tr><td>Update delay after a weekend for non-weekend-update games:</td><td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"AfterWeekendDelayHrs\" value=\"") - 1);
	Seconds sDelay = vDelay.GetInteger();

	    iNumHrs = sDelay / 3600;
	    sDelay -= iNumHrs * 3600;
	    Write (iNumHrs);
	    
	Write ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"AfterWeekendDelayMin\" value=\"", sizeof ("\"> hrs, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"AfterWeekendDelayMin\" value=\"") - 1);
	iNumMin = sDelay / 60;
	    sDelay -= iNumMin * 60;
	    Write (iNumMin);
	    
	Write ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"AfterWeekendDelaySec\" value=\"", sizeof ("\"> min, <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"AfterWeekendDelaySec\" value=\"") - 1);
	Write (sDelay);
	    
	Write ("\"> secs</td></tr><tr><td>Maximum number of updates before games close:</td><td><input type=\"text\" name=\"MaxNumUpdatesBeforeClose\" size=\"5\" maxlength=\"20\" value=\"", sizeof ("\"> secs</td></tr><tr><td>Maximum number of updates before games close:</td><td><input type=\"text\" name=\"MaxNumUpdatesBeforeClose\" size=\"5\" maxlength=\"20\" value=\"") - 1);
	Write (vMaxNumUpdatesBeforeClose.GetInteger()); 
	Write ("\"></td></tr><tr><td>Default number of updates before games close:</td><td><input type=\"text\" name=\"DefaultNumUpdatesBeforeClose\" size=\"5\" maxlength=\"20\" value=\"", sizeof ("\"></td></tr><tr><td>Default number of updates before games close:</td><td><input type=\"text\" name=\"DefaultNumUpdatesBeforeClose\" size=\"5\" maxlength=\"20\" value=\"") - 1);
	Write (vDefaultNumUpdatesBeforeClose.GetInteger()); 
	Write ("\"></td></tr><tr><td>Default Bridier configuration:</td><td><select name=\"BridierDefault\"><option", sizeof ("\"></td></tr><tr><td>Default Bridier configuration:</td><td><select name=\"BridierDefault\"><option") - 1);
	if (iSystemOptions & DEFAULT_BRIDIER_GAMES) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_BRIDIER_GAMES); 
	Write ("\">Grudge games count for Bridier Scoring by default</option><option", sizeof ("\">Grudge games count for Bridier Scoring by default</option><option") - 1);
	if (!(iSystemOptions & DEFAULT_BRIDIER_GAMES)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"0\">Grudge games don't count for Bridier Scoring by default</option></select></td></tr><tr><td>Empire names exposed:</td><td><select name=\"DefaultNamesListed\"><option", sizeof (" value=\"0\">Grudge games don't count for Bridier Scoring by default</option></select></td></tr><tr><td>Empire names exposed:</td><td><select name=\"DefaultNamesListed\"><option") - 1);
	if (iSystemOptions & DEFAULT_NAMES_LISTED) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_NAMES_LISTED); 
	Write ("\">Empire names exposed by default on game lists</option><option", sizeof ("\">Empire names exposed by default on game lists</option><option") - 1);
	if (!(iSystemOptions & DEFAULT_NAMES_LISTED)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"0\">Empire names not exposed by default</option></select></td></tr><tr><td>Games open to spectators:</td><td><select name=\"DefaultSpectator\"><option", sizeof (" value=\"0\">Empire names not exposed by default</option></select></td></tr><tr><td>Games open to spectators:</td><td><select name=\"DefaultSpectator\"><option") - 1);
	if (iSystemOptions & DEFAULT_ALLOW_SPECTATORS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_ALLOW_SPECTATORS); 
	Write ("\">Games open to spectators by default</option><option", sizeof ("\">Games open to spectators by default</option><option") - 1);
	if (!(iSystemOptions & DEFAULT_ALLOW_SPECTATORS)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"0\">Games closed to spectators by default</option></select></td></tr>", sizeof (" value=\"0\">Games closed to spectators by default</option></select></td></tr>") - 1);
	int iFilterIP = iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS);
	    int iFilterId = iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID | DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID);
	    
	    bool fFilterIdle = (iSystemOptions & DEFAULT_RESTRICT_IDLE_EMPIRES) != 0;

	    
	Write ("<tr><td>Default empire filtering by IP Address:</td><td><select name=\"FilterIP\"><option", sizeof ("<tr><td>Default empire filtering by IP Address:</td><td><select name=\"FilterIP\"><option") - 1);
	if (iFilterIP == 0) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"0\">Ignore entry with duplicate IP address</option><option", sizeof (" value=\"0\">Ignore entry with duplicate IP address</option><option") - 1);
	if (iFilterIP == DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS); 
	Write ("\">Warn on entry with duplicate IP address</option><option", sizeof ("\">Warn on entry with duplicate IP address</option><option") - 1);
	if (iFilterIP == DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS); 
	Write ("\">Reject on entry with duplicate IP address</option><option", sizeof ("\">Reject on entry with duplicate IP address</option><option") - 1);
	if (iFilterIP == (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS); 
	Write ("\">Warn and reject on entry with duplicate IP address</option></select></td></tr><tr><td>Default empire filtering by Session Id:</td><td><select name=\"FilterId\"><option", sizeof ("\">Warn and reject on entry with duplicate IP address</option></select></td></tr><tr><td>Default empire filtering by Session Id:</td><td><select name=\"FilterId\"><option") - 1);
	if (iFilterId == 0) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"0\">Ignore entry with duplicate Session Id</option><option", sizeof (" value=\"0\">Ignore entry with duplicate Session Id</option><option") - 1);
	if (iFilterId == DEFAULT_WARN_ON_DUPLICATE_SESSION_ID) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID); 
	Write ("\">Warn on entry with duplicate Session Id</option><option", sizeof ("\">Warn on entry with duplicate Session Id</option><option") - 1);
	if (iFilterId == DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID); 
	Write ("\">Reject on entry with duplicate Session Id</option><option", sizeof ("\">Reject on entry with duplicate Session Id</option><option") - 1);
	if (iFilterId == (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID | DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID | DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID); 
	Write ("\">Warn and reject on entry with duplicate Session Id</option></select></td></tr><tr><td>Default idle empire blocking:</td><td><select name=\"FilterIdle\"><option", sizeof ("\">Warn and reject on entry with duplicate Session Id</option></select></td></tr><tr><td>Default idle empire blocking:</td><td><select name=\"FilterIdle\"><option") - 1);
	if (!fFilterIdle) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"0\">Don't block empires who are idle in other games</option><option", sizeof (" value=\"0\">Don't block empires who are idle in other games</option><option") - 1);
	if (fFilterIdle) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}

	    
	Write (" value=\"1\">Block empires who are idle in other games</option></select></td></tr></table><p><h3>Ship Parameters</h3><table width=\"75%\">", sizeof (" value=\"1\">Block empires who are idle in other games</option></select></td></tr></table><p><h3>Ship Parameters</h3><table width=\"75%\">") - 1);
	iValue = gcConfig.iShipBehavior & (COLONY_USE_MULTIPLIED_BUILD_COST | COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT);

	    
	Write ("<tr><td>Colony behavior:</td><td><select name=\"ColonyBehavior\"><option", sizeof ("<tr><td>Colony behavior:</td><td><select name=\"ColonyBehavior\"><option") - 1);
	if (iValue == 0) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0); 
	Write ("\">Use simple build cost and exponential population deposit</option><option", sizeof ("\">Use simple build cost and exponential population deposit</option><option") - 1);
	if (iValue == COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT);
	    
	Write ("\">Use simple build cost and multiplied population deposit</option><option", sizeof ("\">Use simple build cost and multiplied population deposit</option><option") - 1);
	if (iValue == COLONY_USE_MULTIPLIED_BUILD_COST) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (COLONY_USE_MULTIPLIED_BUILD_COST);
	    
	Write ("\">Use multiplied build cost and exponential population deposit</option><option", sizeof ("\">Use multiplied build cost and exponential population deposit</option><option") - 1);
	if (iValue == (COLONY_USE_MULTIPLIED_BUILD_COST | COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (COLONY_USE_MULTIPLIED_BUILD_COST | COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT);
	    
	Write ("\">Use multiplied build cost and multiplied population deposit</option></select></td></tr><tr><td>Colony simple build cost:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"ColonySimpleBuildCost\" value=\"", sizeof ("\">Use multiplied build cost and multiplied population deposit</option></select></td></tr><tr><td>Colony simple build cost:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"ColonySimpleBuildCost\" value=\"") - 1);
	Write (gcConfig.iColonySimpleBuildFactor); 
	Write ("\"></td></tr><tr><td>Colony multiplied build cost factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"ColonyMultipliedBuildCost\" value=\"", sizeof ("\"></td></tr><tr><td>Colony multiplied build cost factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"ColonyMultipliedBuildCost\" value=\"") - 1);
	Write (gcConfig.fColonyMultipliedBuildFactor); 
	Write ("\"></td></tr><tr><td>Colony multiplied deposit factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"ColonyMultipliedDepositFactor\" value=\"", sizeof ("\"></td></tr><tr><td>Colony multiplied deposit factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"ColonyMultipliedDepositFactor\" value=\"") - 1);
	Write (gcConfig.fColonyMultipliedDepositFactor); 
	Write ("\"></td></tr><tr><td>Colony exponential deposit factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"ColonyExponentialDepositFactor\" value=\"", sizeof ("\"></td></tr><tr><td>Colony exponential deposit factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"ColonyExponentialDepositFactor\" value=\"") - 1);
	Write (gcConfig.fColonyExponentialDepositFactor); 
	Write ("\"></td></tr><tr><td>Engineer cost of opening or closing links:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"EngineerCost\" value=\"", sizeof ("\"></td></tr><tr><td>Engineer cost of opening or closing links:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"EngineerCost\" value=\"") - 1);
	Write (gcConfig.fEngineerLinkCost); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	iValue = gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE;

	    
	Write ("<tr><td>Stargate behavior:</td><td><select name=\"StargateBehavior\"><option", sizeof ("<tr><td>Stargate behavior:</td><td><select name=\"StargateBehavior\"><option") - 1);
	if (iValue == 0) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0);
	    
	Write ("\">Stargates are not limited by their range factor</option><option", sizeof ("\">Stargates are not limited by their range factor</option><option") - 1);
	if (iValue == STARGATE_LIMIT_RANGE) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (STARGATE_LIMIT_RANGE);
	    
	Write ("\">Stargates are limited by their range factor</option></select></td></tr><tr><td>Stargate cost of stargating ships:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"StargateCost\" value=\"", sizeof ("\">Stargates are limited by their range factor</option></select></td></tr><tr><td>Stargate cost of stargating ships:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"StargateCost\" value=\"") - 1);
	Write (gcConfig.fStargateGateCost); 
	Write ("\"></td></tr><tr><td>Stargate range factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"StargateRange\" value=\"", sizeof ("\"></td></tr><tr><td>Stargate range factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"StargateRange\" value=\"") - 1);
	Write (gcConfig.fStargateRangeFactor); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	iValue = gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE;

	    
	Write ("<tr><td>Jumpgate behavior:</td><td><select name=\"JumpgateBehavior\"><option", sizeof ("<tr><td>Jumpgate behavior:</td><td><select name=\"JumpgateBehavior\"><option") - 1);
	if (iValue == 0) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (0);
	    
	Write ("\">Jumpgates are not limited by their range factor</option><option", sizeof ("\">Jumpgates are not limited by their range factor</option><option") - 1);
	if (iValue == JUMPGATE_LIMIT_RANGE) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (JUMPGATE_LIMIT_RANGE);
	    
	Write ("\">Jumpgates are limited by their range factor</option></select></td></tr><tr><td>Jumpgate cost of jumpgating ships:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"JumpgateCost\" value=\"", sizeof ("\">Jumpgates are limited by their range factor</option></select></td></tr><tr><td>Jumpgate cost of jumpgating ships:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"JumpgateCost\" value=\"") - 1);
	Write (gcConfig.fJumpgateGateCost); 
	Write ("\"></td></tr><tr><td>Jumpgate range factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"JumpgateRange\" value=\"", sizeof ("\"></td></tr><tr><td>Jumpgate range factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"JumpgateRange\" value=\"") - 1);
	Write (gcConfig.fJumpgateRangeFactor); 
	Write ("\"></td></tr><tr><td>Terraformer agriculture multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"TerraformerFactor\" value=\"", sizeof ("\"></td></tr><tr><td>Terraformer agriculture multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"TerraformerFactor\" value=\"") - 1);
	Write (gcConfig.fTerraformerPlowFactor); 
	Write ("\"></td></tr><tr><td>Troopship invasion multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"TroopshipInvasion\" value=\"", sizeof ("\"></td></tr><tr><td>Troopship invasion multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"TroopshipInvasion\" value=\"") - 1);
	Write (gcConfig.fTroopshipInvasionFactor); 
	Write ("\"></td></tr><tr><td>Troopship failure factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"TroopshipFailure\" value=\"", sizeof ("\"></td></tr><tr><td>Troopship failure factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"TroopshipFailure\" value=\"") - 1);
	Write (gcConfig.fTroopshipFailureFactor); 
	Write ("\"></td></tr><tr><td>Troopship success factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"TroopshipSuccess\" value=\"", sizeof ("\"></td></tr><tr><td>Troopship success factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"TroopshipSuccess\" value=\"") - 1);
	Write (gcConfig.fTroopshipSuccessFactor); 
	Write ("\"></td></tr><tr><td>Doomsday annihilation multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"DoomsdayMultiplier\" value=\"", sizeof ("\"></td></tr><tr><td>Doomsday annihilation multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"DoomsdayMultiplier\" value=\"") - 1);
	Write (gcConfig.fDoomsdayAnnihilationFactor); 
	Write ("\"></td></tr><tr><td>Carrier battle cost:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"CarrierCost\" value=\"", sizeof ("\"></td></tr><tr><td>Carrier battle cost:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"CarrierCost\" value=\"") - 1);
	Write (gcConfig.fCarrierCost); 
	Write ("\"></td></tr><tr><td>Builder minimum BR to create planet:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"MinBuilderBR\" value=\"", sizeof ("\"></td></tr><tr><td>Builder minimum BR to create planet:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"MinBuilderBR\" value=\"") - 1);
	Write (gcConfig.fBuilderMinBR); 
	Write ("\"></td></tr><tr><td>Builder resource multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"BuilderMultiplier\" value=\"", sizeof ("\"></td></tr><tr><td>Builder resource multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"BuilderMultiplier\" value=\"") - 1);
	Write (gcConfig.fBuilderMultiplier); 
	Write ("\"></td></tr><tr><td>Builder BR dampening factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"BuilderDampener\" value=\"", sizeof ("\"></td></tr><tr><td>Builder BR dampening factor:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"BuilderDampener\" value=\"") - 1);
	Write (gcConfig.fBuilderBRDampener); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	iValue = gcConfig.iShipBehavior & CLOAKER_CLOAK_ON_BUILD;

	    
	Write ("<tr><td>Cloaker behavior:</td><td><select name=\"CloakerBehavior\"><option", sizeof ("<tr><td>Cloaker behavior:</td><td><select name=\"CloakerBehavior\"><option") - 1);
	if (iValue == CLOAKER_CLOAK_ON_BUILD) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (CLOAKER_CLOAK_ON_BUILD); 
	Write ("\">Cloakers are cloaked when built</option><option", sizeof ("\">Cloakers are cloaked when built</option><option") - 1);
	if (iValue == 0) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Cloakers are uncloaked when built</option></select></td></tr>", sizeof (" value=\"0\">Cloakers are uncloaked when built</option></select></td></tr>") - 1);
	iValue = gcConfig.iShipBehavior & MORPHER_CLOAK_ON_CLOAKER_MORPH;

	    
	Write ("<tr><td>Morpher behavior:</td><td><select name=\"MorpherBehavior\"><option ", sizeof ("<tr><td>Morpher behavior:</td><td><select name=\"MorpherBehavior\"><option ") - 1);
	if (iValue == MORPHER_CLOAK_ON_CLOAKER_MORPH) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	} 
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (MORPHER_CLOAK_ON_CLOAKER_MORPH);
	    
	Write ("\">Morphers are cloaked when they morph into cloakers</option><option ", sizeof ("\">Morphers are cloaked when they morph into cloakers</option><option ") - 1);
	if (iValue == 0) {
	        
	Write ("selected ", sizeof ("selected ") - 1);
	} 
	Write ("value=\"0\">Morphers are uncloaked when they morph into cloakers</option></select></td></tr><tr><td>Morpher morphing cost:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"MorpherCost\" value=\"", sizeof ("value=\"0\">Morphers are uncloaked when they morph into cloakers</option></select></td></tr><tr><td>Morpher morphing cost:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"MorpherCost\" value=\"") - 1);
	Write (gcConfig.fMorpherCost); 
	Write ("\"></td></tr><tr><td>Compatibility flags:</td><td><br><input type=\"checkbox\"", sizeof ("\"></td></tr><tr><td>Compatibility flags:</td><td><br><input type=\"checkbox\"") - 1);
	if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {
	        
	Write (" checked", sizeof (" checked") - 1);
	}
	    
	Write (" name=\"DisableFriendlyTerraformers\"> Disable terraforming non-owned planets<br><input type=\"checkbox\"", sizeof (" name=\"DisableFriendlyTerraformers\"> Disable terraforming non-owned planets<br><input type=\"checkbox\"") - 1);
	if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_MULTIPLE) {
	        
	Write (" checked", sizeof (" checked") - 1);
	}
	    
	Write (" name=\"DisableMultipleTerraformers\"> Disable multiple terraformers acting on one planet during an update<br><input type=\"checkbox\"", sizeof (" name=\"DisableMultipleTerraformers\"> Disable multiple terraformers acting on one planet during an update<br><input type=\"checkbox\"") - 1);
	if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_SURVIVAL) {
	        
	Write (" checked", sizeof (" checked") - 1);
	}
	    
	Write (" name=\"DisableTerraformerSurvival\"> Disable terraformer survival after terraforming<br><input type=\"checkbox\"", sizeof (" name=\"DisableTerraformerSurvival\"> Disable terraformer survival after terraforming<br><input type=\"checkbox\"") - 1);
	if (gcConfig.iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL) {
	        
	Write (" checked", sizeof (" checked") - 1);
	}
	    
	Write (" name=\"DisableTroopshipSurvival\"> Disable troopship survival after invading<br><input type=\"checkbox\"", sizeof (" name=\"DisableTroopshipSurvival\"> Disable troopship survival after invading<br><input type=\"checkbox\"") - 1);
	if (gcConfig.iShipBehavior & MINEFIELD_DISABLE_DETONATE) {
	        
	Write (" checked", sizeof (" checked") - 1);
	}
	    
	Write (" name=\"DisableMinefieldDetonate\"> Disable minefield detonation</td></tr></table><h3>Map Parameters</h3><table width=\"75%\"><tr><td>Percent chance a planet will be a linked to an adjacent planet:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"ChanceNewLinkForms\" value=\"", sizeof (" name=\"DisableMinefieldDetonate\"> Disable minefield detonation</td></tr></table><h3>Map Parameters</h3><table width=\"75%\"><tr><td>Percent chance a planet will be a linked to an adjacent planet:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"ChanceNewLinkForms\" value=\"") - 1);
	Write (mcConfig.iChanceNewLinkForms); 
	Write ("\"></td></tr><tr><td>Resource allocation deviation multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"ResourceRand\" value=\"", sizeof ("\"></td></tr><tr><td>Resource allocation deviation multiplier:</td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"ResourceRand\" value=\"") - 1);
	Write (mcConfig.fResourceAllocationRandomizationFactor); 
	Write ("\"></td></tr><tr><td>Maximum map deviation:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"MapDeviation\" value=\"", sizeof ("\"></td></tr><tr><td>Maximum map deviation:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"MapDeviation\" value=\"") - 1);
	Write (mcConfig.iMapDeviation); 
	Write ("\"></td></tr><tr><td>Percent chance a new planet will be linked to the last created planet (large map):</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"ChanceLinkedLarge\" value=\"", sizeof ("\"></td></tr><tr><td>Percent chance a new planet will be linked to the last created planet (large map):</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"ChanceLinkedLarge\" value=\"") - 1);
	Write (mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap); 
	Write ("\"></td></tr><tr><td>Percent chance a new planet will be linked to the last created planet (small map):</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"ChanceLinkedSmall\" value=\"", sizeof ("\"></td></tr><tr><td>Percent chance a new planet will be linked to the last created planet (small map):</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"ChanceLinkedSmall\" value=\"") - 1);
	Write (mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap); 
	Write ("\"></td></tr><tr><td>Large map threshold:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"LargeMapThreshold\" value=\"", sizeof ("\"></td></tr><tr><td>Large map threshold:</td><td><input type=\"text\" size=\"4\" maxlength=\"10\" name=\"LargeMapThreshold\" value=\"") - 1);
	Write (mcConfig.iLargeMapThreshold); 
	Write ("\"></td></tr></table>", sizeof ("\"></td></tr></table>") - 1);
	WriteButton (BID_CANCEL);

	    if (iNumSuperClasses > 0) {
	        g_pGameEngine->FreeKeys (piSuperClassKey);
	        g_pGameEngine->FreeData (pvSuperClassName);
	    }

	    }

	    break;

	AllGames:
	case 1:
	    {

	    int* piGameClass, * piGameNumber, iNumActiveGames, iNumOpenGames, iNumClosedGames;
	    Check (g_pGameEngine->GetActiveGames (&piGameClass, &piGameNumber, &iNumActiveGames));
	    Check (g_pGameEngine->GetNumOpenGames (&iNumOpenGames));
	    Check (g_pGameEngine->GetNumClosedGames (&iNumClosedGames));

	    
	Write ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"1\">", sizeof ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"1\">") - 1);
	WriteActiveGameAdministration (piGameClass, piGameNumber, iNumActiveGames, iNumOpenGames, iNumClosedGames, true);

	    if (piGameClass != NULL) {
	        delete [] piGameClass;
	    }

	    if (piGameNumber != NULL) {
	        delete [] piGameNumber;
	    }

	    }
	    break;

	case 2: 

	    
	Write ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"2\"><p><h3>Create a new GameClass:</h3>", sizeof ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"2\"><p><h3>Create a new GameClass:</h3>") - 1);
	WriteCreateGameClassString (SYSTEM, NO_KEY, false);
	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_CREATENEWGAMECLASS);

	    break;

	case 3:

	    
	Write ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"3\">", sizeof ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"3\">") - 1);
	WriteAdministerGame (iGameClass, iGameNumber, true);

	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);

	    break;

	case 4:
	    {

	    int iState;

	    iErrCode = g_pGameEngine->GetGameState (iGameClass, iGameNumber, &iState);
	    if (iErrCode != OK) {
	        goto AllGames;
	    }

	    if (!(iState & GAME_MAP_GENERATED)) {
	        goto AllGames;
	    }

	    iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	    if (iErrCode != OK) {
	        goto AllGames;
	    }

	    
	Write ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"4\"><input type=\"hidden\" name=\"GameClass\" value=\"", sizeof ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"4\"><input type=\"hidden\" name=\"GameClass\" value=\"") - 1);
	Write (iGameClass); 
	Write ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"") - 1);
	Write (iGameNumber); 
	Write ("\">", sizeof ("\">") - 1);
	RenderMap (iGameClass, iGameNumber, m_iEmpireKey, true, NULL, false);

	    g_pGameEngine->SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);

	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);

	    }
	    break;

	case 6:
	    {

	    Variant vOptions;

	    unsigned int iLivePlanetKey, iDeadPlanetKey;
	    int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;

	    bool bStarted, bFalse;

	    IDatabase* pDatabase = g_pGameEngine->GetDatabase();
	    Variant* pvPlanetData = NULL;

	    GAME_MAP (pszGameMap, iGameClass, iGameNumber);

	    iErrCode = g_pGameEngine->HasGameStarted (iGameClass, iGameNumber, &bStarted);
	    if (iErrCode != OK) {
	        AddMessage ("That game no longer exists");
	        goto AllGames;
	    }

	    if (!bStarted) {
	        AddMessage ("The game hasn't started yet, so it has no map");
	        goto AllGames;
	    }

	    iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	    if (iErrCode != OK) {
	        AddMessage ("That game no longer exists");
	        goto AllGames;
	    }

	    iErrCode = g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);
	    if (iErrCode != OK) {
	        AddMessage ("That game no longer exists");
	        goto Cleanup;
	    }

	    iErrCode = GetGoodBadResourceLimits (
	        iGameClass,
	        iGameNumber,
	        &iGoodAg,
	        &iBadAg,
	        &iGoodMin,
	        &iBadMin,
	        &iGoodFuel,
	        &iBadFuel
	        );

	    if (iErrCode != OK) {
	        AddMessage ("That game no longer exists");
	        goto Cleanup;
	    }

	    iErrCode = pDatabase->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vOptions);
	    if (iErrCode != OK) {
	        AddMessage ("That game no longer exists");
	        goto Cleanup;
	    }

	    iErrCode = pDatabase->ReadRow (pszGameMap, iClickedPlanetKey, &pvPlanetData);
	    if (iErrCode != OK) {
	        AddMessage ("That game no longer exists");
	        goto Cleanup;
	    }

	    m_iGameState |= STARTED | GAME_MAP_GENERATED;
	    m_iGameClass = iGameClass;
	    m_iGameNumber = iGameNumber;

	    
	Write ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"6\"><input type=\"hidden\" name=\"GameClass\" value=\"", sizeof ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"6\"><input type=\"hidden\" name=\"GameClass\" value=\"") - 1);
	Write (iGameClass); 
	Write ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"") - 1);
	Write (iGameNumber); 
	Write ("\"><p><table width=\"90%\">", sizeof ("\"><p><table width=\"90%\">") - 1);
	// Best effort
	    WriteUpClosePlanetString (NO_KEY, iClickedPlanetKey, 
	        0, iLivePlanetKey, iDeadPlanetKey, 0, true, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel,
	        1.0, (vOptions.GetInteger() & INDEPENDENCE) != 0, true, false, pvPlanetData, &bFalse);

	Cleanup:

	    // Best effort
	    g_pGameEngine->SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);

	    if (pvPlanetData != NULL) {
	        pDatabase->FreeData (pvPlanetData);
	    }

	    pDatabase->Release();

	    if (iErrCode != OK) {
	        goto AllGames;
	    }

	    
	Write ("</table><p>", sizeof ("</table><p>") - 1);
	WriteButton (BID_VIEWMAP);

	    }
	    break;

	case 7:

	    {

	    iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber, NO_KEY, NULL);
	    if (iErrCode != OK) {
	        AddMessage ("That game no longer exists");
	        goto AllGames;
	    }

	    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	    iErrCode = g_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
	    if (iErrCode != OK) {
	        pszGameClassName[0] = '\0';
	    }

	    
	Write ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"7\"><input type=\"hidden\" name=\"GameClass\" value=\"", sizeof ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"7\"><input type=\"hidden\" name=\"GameClass\" value=\"") - 1);
	Write (iGameClass); 
	Write ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"") - 1);
	Write (iGameNumber); 
	Write ("\"><p>Empire information for ", sizeof ("\"><p>Empire information for ") - 1);
	Write (pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (iGameNumber); 
	Write (":<p>", sizeof (":<p>") - 1);
	RenderEmpireInformation (iGameClass, iGameNumber, true);

	    g_pGameEngine->SignalGameReader (iGameClass, iGameNumber, NO_KEY, NULL);

	    WriteButton (BID_CANCEL);

	    }
	    break;

	case 8:

	    {

	    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	    iErrCode = g_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
	    if (iErrCode != OK) {
	        goto AllGames;
	    }

	    
	Write ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"8\"><input type=\"hidden\" name=\"GameClass\" value=\"", sizeof ("<input type=\"hidden\" name=\"GameAdminPage\" value=\"8\"><input type=\"hidden\" name=\"GameClass\" value=\"") - 1);
	Write (iGameClass); 
	Write ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"", sizeof ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"") - 1);
	Write (iGameNumber); 
	Write ("\"><p><table width=\"65%\"><tr><td align=\"center\">Are you sure you want to kill <strong>", sizeof ("\"><p><table width=\"65%\"><tr><td align=\"center\">Are you sure you want to kill <strong>") - 1);
	Write (pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (iGameNumber); 
	    
	Write ("</strong>?<p>If so, please send a message to its participants:</td></tr></table><p><textarea name=\"DoomMessage\" rows=\"5\" cols=\"45\" wrap=\"physical\"></textarea><p>", sizeof ("</strong>?<p>If so, please send a message to its participants:</td></tr></table><p><textarea name=\"DoomMessage\" rows=\"5\" cols=\"45\" wrap=\"physical\"></textarea><p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_KILLGAME);

	    }
	    break;

	default:

	    Assert (false);
	    break;
	}

	SYSTEM_CLOSE


}