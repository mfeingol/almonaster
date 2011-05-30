<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "Osal/Algorithm.h"

#include <stdio.h>

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
String strRedirect;
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
				iPersonalMaxNumEmpires, iPersonalMaxNumPlanets, iNewValue, iOldValue, 
				iMaxNumUpdatesBeforeClose, iDefaultNumUpdatesBeforeClose;

			const char* pszNewValue;
			Seconds sDelay;

			Check (g_pGameEngine->GetMinNumSecsPerUpdateForSystemGameClass (&iSystemMinNumSecsPerUpdate));
			Check (g_pGameEngine->GetMaxNumSecsPerUpdateForSystemGameClass (&iSystemMaxNumSecsPerUpdate));
			Check (g_pGameEngine->GetMaxNumEmpiresForSystemGameClass (&iSystemMaxNumEmpires));
			Check (g_pGameEngine->GetMaxNumPlanetsForSystemGameClass (&iSystemMaxNumPlanets));

			Check (g_pGameEngine->GetMaxNumUpdatesBeforeClose (&iMaxNumUpdatesBeforeClose));
			Check (g_pGameEngine->GetDefaultNumUpdatesBeforeClose (&iDefaultNumUpdatesBeforeClose));

			Check (g_pGameEngine->GetMinNumSecsPerUpdateForPersonalGameClass (&iPersonalMinNumSecsPerUpdate));
			Check (g_pGameEngine->GetMaxNumSecsPerUpdateForPersonalGameClass (&iPersonalMaxNumSecsPerUpdate));
			Check (g_pGameEngine->GetMaxNumEmpiresForPersonalGameClass (&iPersonalMaxNumEmpires));
			Check (g_pGameEngine->GetMaxNumPlanetsForPersonalGameClass (&iPersonalMaxNumPlanets));

			int iMaxResourcesPerPlanet, iMaxResourcesPerPlanetPersonal, iSystemOptions;
			float fMaxInitialTechLevel, fMaxInitialTechLevelPersonal, fMaxTechDev, fMaxTechDevPersonal;

			Check (g_pGameEngine->GetMaxResourcesPerPlanet (&iMaxResourcesPerPlanet));
			Check (g_pGameEngine->GetMaxResourcesPerPlanetPersonal (&iMaxResourcesPerPlanetPersonal));

			Check (g_pGameEngine->GetMaxInitialTechLevel (&fMaxInitialTechLevel));
			Check (g_pGameEngine->GetMaxInitialTechLevelPersonal (&fMaxInitialTechLevelPersonal));

			Check (g_pGameEngine->GetMaxTechDev (&fMaxTechDev));
			Check (g_pGameEngine->GetMaxTechDevPersonal (&fMaxTechDevPersonal));

			Check (g_pGameEngine->GetAfterWeekendDelay (&sDelay));

			Check (g_pGameEngine->GetSystemOptions (&iSystemOptions));

			GameConfiguration gcOldConfig = { 0 }, gcNewConfig = { 0 };
			MapConfiguration mcOldConfig = { 0 }, mcNewConfig = { 0 };

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

			if (iNewValue != iMaxNumUpdatesBeforeClose) {
				if (g_pGameEngine->SetMaxNumUpdatesBeforeClose (iNewValue) == OK) {
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

			if (iNewValue != iDefaultNumUpdatesBeforeClose) {
				if (g_pGameEngine->SetDefaultNumUpdatesBeforeClose (iNewValue) == OK) {
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

			// MaxResourcesPerPlanet
			if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxResourcesPerPlanet")) == NULL) {
				goto Redirection;
			}
			iNewValue = pHttpForm->GetIntValue();

			if (iNewValue != iMaxResourcesPerPlanet) {
				if (g_pGameEngine->SetMaxResourcesPerPlanet (iNewValue) == OK) {
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

			if (fNewValue != fMaxInitialTechLevel) {
				if (g_pGameEngine->SetMaxInitialTechLevel (fNewValue) == OK) {
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

			if (fNewValue != fMaxTechDev) {
				if (g_pGameEngine->SetMaxTechDev (fNewValue) == OK) {
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
			if (iNewValue != iOldValue) {
				if (g_pGameEngine->SetMaxNumPersonalGameClasses (iNewValue) == OK) {
					AddMessage ("The maximum number of Personal GameClasses was updated");
				} else {
					AddMessage ("The maximum number of Personal GameClasses could not be updated");
				}
			}

			// Empires
			if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxEmps")) == NULL) {
				goto Redirection;
			}
			iNewValue = pHttpForm->GetIntValue();

			if (iNewValue != iPersonalMaxNumEmpires) {
				if (g_pGameEngine->SetMaxNumEmpiresForPersonalGameClass (iNewValue) == OK) {
					AddMessage ("The maximum number of empires for a Personal GameClass was updated");
				} else {
					AddMessage ("Invalid maximum number of empires for a Personal GameClass");
				}
			}

			// Planets
			if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxPlanets")) == NULL) {
				goto Redirection;
			}
			iNewValue = pHttpForm->GetIntValue();

			if (iNewValue != iPersonalMaxNumPlanets) {
				if (g_pGameEngine->SetMaxNumPlanetsForPersonalGameClass (iNewValue) == OK) {
					AddMessage ("The maximum number of planets for a Personal GameClass was updated");
				} else {
					AddMessage ("Invalid maximum number of planets for a Personal GameClass");
				}
			}

			// MaxResourcesPerPlanet
			if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxResourcesPerPlanet")) == NULL) {
				goto Redirection;
			}
			iNewValue = pHttpForm->GetIntValue();

			if (iNewValue != iMaxResourcesPerPlanetPersonal) {
				if (g_pGameEngine->SetMaxResourcesPerPlanetPersonal (iNewValue) == OK) {
					AddMessage ("The maximum average resources per attribute per planet for a Personal GameClass was updated");
				} else {
					AddMessage ("Invalid maximum average resources per attribute per planet for a Personal GameClass");
				}
			}

			// MaxInitialTechLevel
			if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxInitialTechLevel")) == NULL) {
				goto Redirection;
			}
			fNewValue = pHttpForm->GetFloatValue();

			if (fNewValue != fMaxInitialTechLevelPersonal) {
				if (g_pGameEngine->SetMaxInitialTechLevelPersonal (fNewValue) == OK) {
					AddMessage ("The maximum initial tech level for a Personal GameClass was updated");
				} else {
					AddMessage ("Invalid maximum initial tech level for a Personal GameClass");
				}
			}

			// MaxTechDev
			if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxTechDev")) == NULL) {
				goto Redirection;
			}
			fNewValue = pHttpForm->GetFloatValue();

			if (fNewValue != fMaxTechDevPersonal) {
				if (g_pGameEngine->SetMaxTechDevPersonal (fNewValue) == OK) {
					AddMessage ("The maximum tech development for a Personal GameClass was updated");
				} else {
					AddMessage ("Invalid maximum tech development for a Personal GameClass");
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
					AddMessage ("The minimum update period for a Personal GameClass was updated");
				} else {
					AddMessage ("Invalid minimum update period for a Personal GameClass");
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
					AddMessage ("The maximum update period for a Personal GameClass was updated");
				} else {
					AddMessage ("Invalid maximum update period for a Personal GameClass");
				}
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

			if (iNewValue != sDelay) {
				if (g_pGameEngine->SetAfterWeekendDelay (iNewValue) == OK) {
					AddMessage ("The after weekend delay was updated");
				} else {
					AddMessage ("The after weekend delay was updated");
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

			// DisableColonySettles
			if (m_pHttpRequest->GetForm ("DisableColonySettles") != NULL) {
				gcNewConfig.iShipBehavior |= COLONY_DISABLE_SETTLES;
			}

			// DisableFriendlyTerraformers
			if (m_pHttpRequest->GetForm ("DisableFriendlyTerraformers") != NULL) {
				gcNewConfig.iShipBehavior |= TERRAFORMER_DISABLE_FRIENDLY;
			}

			if (m_pHttpRequest->GetForm ("DisableMultipleTerraformers") != NULL) {
				gcNewConfig.iShipBehavior |= TERRAFORMER_DISABLE_MULTIPLE;
			}

			// DisableColonySurvival
			if (m_pHttpRequest->GetForm ("DisableColonySurvival") != NULL) {
				gcNewConfig.iShipBehavior |= COLONY_DISABLE_SURVIVAL;
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

				if (pszNewValue == NULL || *pszNewValue == '\0' || strlen (pszNewValue) > MAX_SUPER_CLASS_NAME_LENGTH) {
					AddMessage ("You must submit a valid SuperClass name");
				} else {

					int iKey;
					char pszMessage [1024];

					iErrCode = g_pGameEngine->CreateSuperClass (pszNewValue, &iKey);

					switch (iErrCode) {

					case OK:
						sprintf (pszMessage, "The SuperClass %s was created", pszNewValue );
						AddMessage (pszMessage);
						break;

					case ERROR_SUPERCLASS_ALREADY_EXISTS:
						sprintf (pszMessage, "The SuperClass %s already exists", pszNewValue );
						AddMessage (pszMessage);
						break;

					default:
						sprintf (pszMessage, "Error %i occurred", iErrCode);
						AddMessage (pszMessage);
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

			// Force update
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
				ProcessCreateGameClassForms (SYSTEM) != OK) {
				iGameAdminPage = 2;
			}

			}
			break;

		case 3:

			{

			bool bExist;
			const char* pszMessage;

			// Get game class, game number
			if ((pHttpForm  = m_pHttpRequest->GetFormBeginsWith ("GameClass")) == NULL ||
				(pHttpForm2 = m_pHttpRequest->GetFormBeginsWith ("GameNumber")) == NULL) {
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

				iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber);
				if (iErrCode != OK) {
					iGameAdminPage = 1;
					AddMessage ("The game no longer exists");
				} else {

					iErrCode = g_pGameEngine->SetGamePassword (iGameClass, iGameNumber, pszMessage);
					if (iErrCode == OK) {
						AddMessage ("The game password was updated");
						iGameAdminPage = 3;
					} else {
						AddMessage ("The game no longer exists");
						iGameAdminPage = 1;
					}
				}

				iErrCode = g_pGameEngine->SignalGameReader (iGameClass, iGameNumber);
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

				if ((iErrCode = g_pGameEngine->AdminPauseGame (iGameClass, iGameNumber, true)) == OK) {
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

				if ((iErrCode = g_pGameEngine->AdminUnpauseGame (iGameClass, iGameNumber, true)) == OK) {
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

				if (g_pGameEngine->WaitGameReader (iGameClass, iGameNumber) != OK) {
					iGameAdminPage = 1;
					AddMessage ("That game no longer exists");
				} else {

					if ((iErrCode = g_pGameEngine->BroadcastGameMessage (
						iGameClass,
						iGameNumber,
						pszMessage,
						m_iEmpireKey,
						true)
						) == OK) {
						AddMessage ("Your message was broadcast to all empires in the game");
					} else {
						AddMessage ("The game no longer exists");
					}

					if (g_pGameEngine->SignalGameReader (iGameClass, iGameNumber) == OK) {
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
				break;
			}

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
	bool* pbGameClassHalted, * pbGameClassDeleted;
	Check (g_pGameEngine->GetSystemGameClassKeys (&piGameClassKey, &pbGameClassHalted, &pbGameClassDeleted, &iNumGameClasses));

	Algorithm::AutoDelete<int> auto1 (piGameClassKey);
	Algorithm::AutoDelete<bool> auto2 (pbGameClassHalted);
	Algorithm::AutoDelete<bool> auto3 (pbGameClassDeleted);

	GameConfiguration gcConfig;
	MapConfiguration mcConfig;

	int iNumActiveGames, iNumOpenGames, iNumClosedGames, iMaxNumUpdatesBeforeClose, iDefaultNumUpdatesBeforeClose;

	Check (g_pGameEngine->GetNumActiveGames (&iNumActiveGames));
	Check (g_pGameEngine->GetNumOpenGames (&iNumOpenGames));
	Check (g_pGameEngine->GetNumClosedGames (&iNumClosedGames));

	int iSystemMinNumSecsPerUpdate, iSystemMaxNumSecsPerUpdate, iSystemMaxNumEmpires, iSystemMaxNumPlanets,
		iPersonalMinNumSecsPerUpdate, iPersonalMaxNumSecsPerUpdate, iPersonalMaxNumEmpires, 
		iPersonalMaxNumPlanets, iMaxNumPersonalGameClasses, iValue, iSystemOptions;

	Seconds sDelay;

	Check (g_pGameEngine->GetMinNumSecsPerUpdateForSystemGameClass (&iSystemMinNumSecsPerUpdate));
	Check (g_pGameEngine->GetMaxNumSecsPerUpdateForSystemGameClass (&iSystemMaxNumSecsPerUpdate));
	Check (g_pGameEngine->GetMaxNumEmpiresForSystemGameClass (&iSystemMaxNumEmpires));
	Check (g_pGameEngine->GetMaxNumPlanetsForSystemGameClass (&iSystemMaxNumPlanets));

	Check (g_pGameEngine->GetMaxNumUpdatesBeforeClose (&iMaxNumUpdatesBeforeClose));
	Check (g_pGameEngine->GetDefaultNumUpdatesBeforeClose (&iDefaultNumUpdatesBeforeClose));

	Check (g_pGameEngine->GetMinNumSecsPerUpdateForPersonalGameClass (&iPersonalMinNumSecsPerUpdate));
	Check (g_pGameEngine->GetMaxNumSecsPerUpdateForPersonalGameClass (&iPersonalMaxNumSecsPerUpdate));
	Check (g_pGameEngine->GetMaxNumEmpiresForPersonalGameClass (&iPersonalMaxNumEmpires));
	Check (g_pGameEngine->GetMaxNumPlanetsForPersonalGameClass (&iPersonalMaxNumPlanets));

	int iMaxResourcesPerPlanet, iMaxResourcesPerPlanetPersonal;
	float fMaxInitialTechLevel, fMaxInitialTechLevelPersonal, fMaxTechDev, fMaxTechDevPersonal;

	Check (g_pGameEngine->GetMaxResourcesPerPlanet (&iMaxResourcesPerPlanet));
	Check (g_pGameEngine->GetMaxResourcesPerPlanetPersonal (&iMaxResourcesPerPlanetPersonal));

	Check (g_pGameEngine->GetMaxInitialTechLevel (&fMaxInitialTechLevel));
	Check (g_pGameEngine->GetMaxInitialTechLevelPersonal (&fMaxInitialTechLevelPersonal));

	Check (g_pGameEngine->GetMaxTechDev (&fMaxTechDev));
	Check (g_pGameEngine->GetMaxTechDevPersonal (&fMaxTechDevPersonal));

	Check (g_pGameEngine->GetGameConfiguration (&gcConfig));
	Check (g_pGameEngine->GetMapConfiguration (&mcConfig));

	Check (g_pGameEngine->GetAfterWeekendDelay (&sDelay));

	Check (g_pGameEngine->GetMaxNumPersonalGameClasses (&iMaxNumPersonalGameClasses));

	Check (g_pGameEngine->GetSystemOptions (&iSystemOptions));

	%><input type="hidden" name="GameAdminPage" value="0"><%

	%><h3>Game Management</h3><%

	%><p><table width="75%"><%

	// Active games
	%><tr><td>There <% 
	if (iNumActiveGames == 1) { 
		%>is <strong>1</strong> active game<% 
	} else { 
		%>are <strong><% Write (iNumActiveGames); %></strong> active games<%
	}
	%> on the server<%

	if (iNumActiveGames > 0) {

		%> (<strong><% Write (iNumOpenGames); %></strong> open, <strong><% Write (iNumClosedGames); 
		%></strong> closed)</td><td><%

		WriteButton (BID_VIEWGAMEINFORMATION);
	}

	%></td></tr><%

	if (iNumActiveGames > 0) {

		%><tr><td>&nbsp;</td><td>&nbsp;</td><%
		%><tr><td>Pause all games:</td><td><%

		WriteButton (BID_PAUSEALLGAMES);

		%></td></tr><tr><td>Unpause all games:</td><td><%

		WriteButton (BID_UNPAUSEALLGAMES);

		%></td></tr><%
	}

	%></table><%

	%><p><h3>SuperClasses and GameClasses</h3><%

	%><p><table width="75%"><%

	// SuperClasses
	if (iNumSuperClasses > 0) { 

		%><tr><td>Delete a SuperClass:</td><td><select name="DelSuperClass"><%
		for (i = 0; i < iNumSuperClasses; i ++) { 
			%><option value="<% Write (piSuperClassKey [i]); %>"><% Write (pvSuperClassName[i].GetCharPtr()); %></option><%
		}
		%></select></td><td><%

		WriteButton (BID_DELETESUPERCLASS);
		%></td></tr><%
	}

	%><tr><td>Create a new SuperClass:</td><td><%
	%><input type="text" size="23" maxlength="<% Write (MAX_SUPER_CLASS_NAME_LENGTH); %>" name="NewSuperClassName"></td><td><%
	WriteButton (BID_CREATENEWSUPERCLASS);
	%></td></tr><%

	if (iNumSuperClasses > 0) {

		%><tr><td>Rename a SuperClass:</td><td><select name="RenSuperClass"><%
		for (i = 0; i < iNumSuperClasses; i ++) { 
			%><option value="<% Write ( piSuperClassKey [i]); %>"><% Write (pvSuperClassName[i].GetCharPtr()); %></option><%
		}
		%></select><%

		%><br> to<br><input type="text" size="23" maxlength="<% Write (MAX_SUPER_CLASS_NAME_LENGTH); 
		%>" name="RenSuperClassName"><%

		%></td><td><%

		WriteButton (BID_RENAMESUPERCLASS);
		%></td></tr><%
	}

	%><tr><td>&nbsp;</td></td><%

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

			%><tr><td>Delete a GameClass:</td><td><select name="DeleteGC"><%

			char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

			for (i = 0; i < iNumGameClasses; i ++) { 
				if (!pbGameClassDeleted[i] &&
					g_pGameEngine->GetGameClassName (piGameClassKey[i], pszGameClassName) == OK) {
					%><option value="<% Write (piGameClassKey[i]); %>"><% 
					Write (pszGameClassName); %></option><%
				}
			}
			%></select></td><td><%
			WriteButton (BID_DELETEGAMECLASS);
			%></td></tr><%
		}

		if (iNumDeleted > 0) {

			%><tr><td>Undelete a GameClass:</td><td><select name="UndeleteGC"><%

			char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

			for (i = 0; i < iNumGameClasses; i ++) { 
				if (pbGameClassDeleted[i] &&
					g_pGameEngine->GetGameClassName (piGameClassKey[i], pszGameClassName) == OK) {
					%><option value="<% Write (piGameClassKey[i]); %>"><% 
					Write (pszGameClassName); %></option><%
				}
			}
			%></select></td><td><%
			WriteButton (BID_UNDELETEGAMECLASS);
			%></td></tr><%
		}

		if (iNumNotHalted > 0) {

			%><tr><td>Halt a GameClass:</td><td><select name="HaltGC"><%

			char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

			for (i = 0; i < iNumGameClasses; i ++) { 
				if (!pbGameClassHalted[i] &&
					g_pGameEngine->GetGameClassName (piGameClassKey[i], pszGameClassName) == OK) {
					%><option value="<% Write (piGameClassKey[i]); %>"><% 
					Write (pszGameClassName); %></option><%
				}
			}
			%></select></td><td><%
			WriteButton (BID_HALTGAMECLASS);
			%></td></tr><%
		}

		if (iNumHalted > 0) {

			%><tr><td>Unhalt a GameClass:</td><td><select name="UnhaltGC"><%

			char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

			for (i = 0; i < iNumGameClasses; i ++) { 
				if (pbGameClassHalted[i] &&
					g_pGameEngine->GetGameClassName (piGameClassKey[i], pszGameClassName) == OK) {
					%><option value="<% Write (piGameClassKey[i]); %>"><% 
					Write (pszGameClassName); %></option><%
				}
			}
			%></select></td><td><%
			WriteButton (BID_UNHALTGAMECLASS);
			%></td></tr><%
		}
	}

	%><tr><td>Create a new GameClass:</td><td></td><td><%
	WriteButton (BID_CREATENEWGAMECLASS);
	%></td></tr></table><%

	// Parameters
	int iNumHrs, iNumMin;

	%><p><h3>GameClass Parameters</h3><%

	%><p><table width="75%"><%

	%><tr><td>Maximum number of empires in a System GameClass:</td><%
	%><td><input type="text" name="SMaxEmps" size="5" maxlength="20" value="<% 
		Write (iSystemMaxNumEmpires); %>"></td></tr><%

	%><tr><td>Maximum number of planets in a System GameClass:</td><%
	%><td><input type="text" name="SMaxPlanets" size="5" maxlength="20" value="<% 
		Write (iSystemMaxNumPlanets); %>"></td></tr><%

	%><tr><td>Maximum average number of resources per attribute per planet in a System GameClass:</td><%
	%><td><input type="text" name="SMaxResourcesPerPlanet" size="5" maxlength="20" value="<% 
		Write (iMaxResourcesPerPlanet); %>"></td></tr><%

	%><tr><td>Maximum initial tech level in a System GameClass:</td><%
	%><td><input type="text" name="SMaxInitialTechLevel" size="10" maxlength="20" value="<% 
		Write (fMaxInitialTechLevel); %>"></td></tr><%

	%><tr><td>Maximum tech development in a System GameClass:</td><%
	%><td><input type="text" name="SMaxTechDev" size="10" maxlength="20" value="<% 
		Write (fMaxTechDev); %>"></td></tr><%

	%><tr><td>Minimum update time in a System GameClass:</td><%
	%><td><%
	%><input type="text" size="4" maxlength="20" name="SMinHrs" value="<%
	iNumHrs = iSystemMinNumSecsPerUpdate / 3600;
	iSystemMinNumSecsPerUpdate -= iNumHrs * 3600;
	Write (iNumHrs);
	%>"> hrs, <%
	%><input type="text" size="4" maxlength="20" name="SMinMin" value="<%
	iNumMin = iSystemMinNumSecsPerUpdate / 60;
	iSystemMinNumSecsPerUpdate -= iNumMin * 60;
	Write (iNumMin);
	%>"> min, <%
	%><input type="text" size="4" maxlength="20" name="SMinSec" value="<%
	Write (iSystemMinNumSecsPerUpdate);
	%>"> secs<%
	%></td></tr><%

	%><tr><td>Maximum update time in a System GameClass:</td><%
	%><td><%
	%><input type="text" size="4" maxlength="20" name="SMaxHrs" value="<%
	iNumHrs = iSystemMaxNumSecsPerUpdate / 3600;
	iSystemMaxNumSecsPerUpdate -= iNumHrs * 3600;
	Write (iNumHrs);
	%>"> hrs, <%
	%><input type="text" size="4" maxlength="20" name="SMaxMin" value="<%
	iNumMin = iSystemMaxNumSecsPerUpdate / 60;
	iSystemMaxNumSecsPerUpdate -= iNumMin * 60;
	Write (iNumMin);
	%>"> min, <%
	%><input type="text" size="4" maxlength="20" name="SMaxSec" value="<%
	Write (iSystemMaxNumSecsPerUpdate);
	%>"> secs<%
	%></td></tr><%

	%><tr><td>&nbsp;</td></td><%

	%><tr><td>Maximum number of Personal GameClasses per empire:</td><%
	%><td><input type="text" size="6" maxlength="6" name="NewMaxNumPGC" value="<% 
	Write (iMaxNumPersonalGameClasses); %>"><%
	%><input type="hidden" name="OldMaxNumPGC" value="<%
	Write (iMaxNumPersonalGameClasses); %>"></td></tr><%

	%><tr><td>&nbsp;</td></td><%

	%><tr><td>Maximum number of empires in a Personal GameClass:</td><%
	%><td><input type="text" name="PMaxEmps" size="5" maxlength="20" value="<% 
		Write (iPersonalMaxNumEmpires); %>"></td></tr><%

	%><tr><td>Maximum number of planets in a Personal GameClass:</td><%
	%><td><input type="text" name="PMaxPlanets" size="5" maxlength="20" value="<% 
		Write (iPersonalMaxNumPlanets); %>"></td></tr><%

	%><tr><td>Maximum average number of resources per attribute per planet in a Personal GameClass:</td><%
	%><td><input type="text" name="PMaxResourcesPerPlanet" size="5" maxlength="20" value="<% 
		Write (iMaxResourcesPerPlanetPersonal); %>"></td></tr><%

	%><tr><td>Maximum initial tech level in a Personal GameClass:</td><%
	%><td><input type="text" name="PMaxInitialTechLevel" size="10" maxlength="20" value="<% 
		Write (fMaxInitialTechLevelPersonal); %>"></td></tr><%

	%><tr><td>Maximum tech development in a Personal GameClass:</td><%
	%><td><input type="text" name="PMaxTechDev" size="10" maxlength="20" value="<% 
		Write (fMaxTechDevPersonal); %>"></td></tr><%

	%><tr><td>Minimum update time in a Personal GameClass:</td><%
	%><td><%
	%><input type="text" size="4" maxlength="20" name="PMinHrs" value="<%
	iNumHrs = iPersonalMinNumSecsPerUpdate / 3600;
	iPersonalMinNumSecsPerUpdate -= iNumHrs * 3600;
	Write (iNumHrs);
	%>"> hrs, <%
	%><input type="text" size="4" maxlength="20" name="PMinMin" value="<%
	iNumMin = iPersonalMinNumSecsPerUpdate / 60;
	iPersonalMinNumSecsPerUpdate -= iNumMin * 60;
	Write (iNumMin);
	%>"> min, <%
	%><input type="text" size="4" maxlength="20" name="PMinSec" value="<%
	Write (iPersonalMinNumSecsPerUpdate);
	%>"> secs<%
	%></td></tr><%

	%><tr><td>Maximum update time in a System GameClass:</td><%
	%><td><%
	%><input type="text" size="4" maxlength="20" name="PMaxHrs" value="<%
	iNumHrs = iPersonalMaxNumSecsPerUpdate / 3600;
	iPersonalMaxNumSecsPerUpdate -= iNumHrs * 3600;
	Write (iNumHrs);
	%>"> hrs, <%
	%><input type="text" size="4" maxlength="20" name="PMaxMin" value="<%
	iNumMin = iPersonalMaxNumSecsPerUpdate / 60;
	iPersonalMaxNumSecsPerUpdate -= iNumMin * 60;
	Write (iNumMin);
	%>"> min, <%
	%><input type="text" size="4" maxlength="20" name="PMaxSec" value="<%
	Write (iPersonalMaxNumSecsPerUpdate);
	%>"> secs<%
	%></td></tr><%

	%><tr><td>&nbsp;</td><td>&nbsp;</td></tr><%

	%></table><%

	%><h3>Game Parameters</h3><%

	%><table width="75%"><%

	%><tr><td>Percent of damage used to destroy (DEST):</td><td><%
	%><input type="text" size="4" maxlength="10" name="DamageDest"<%
	%> value="<% Write (gcConfig.iPercentDamageUsedToDestroy); %>"></td></tr><%

	%><tr><td>Percent econ increase on first trade:</td><td><%
	%><input type="text" size="4" maxlength="10" name="FirstTradeIncrease"<%
	%> value="<% Write (gcConfig.iPercentFirstTradeIncrease); %>"></td></tr><%

	%><tr><td>Percent of previous econ increase on next trade:</td><td><%
	%><input type="text" size="4" maxlength="10" name="NextTradeOnIncrease"<%
	%> value="<% Write (gcConfig.iPercentNextTradeIncrease); %>"></td></tr><%

	%><tr><td>Percent of full tech increase for late entrances:</td><td><%
	%><input type="text" size="4" maxlength="10" name="TechForLatecomers"<%
	%> value="<% Write (gcConfig.iPercentTechIncreaseForLatecomers); %>"></td></tr><%

	%><tr><td>Number of nukes before planet is quarantined:</td><td><%
	%><input type="text" size="4" maxlength="10" name="NukesForQuarantine"<%
	%> value="<% Write (gcConfig.iNukesForQuarantine); %>"></td></tr><%

	%><tr><td>Number of updates in quarantine after planet is nuked:</td><td><%
	%><input type="text" size="4" maxlength="10" name="UpdatesInQuarantine"<%
	%> value="<% Write (gcConfig.iUpdatesInQuarantine); %>"></td></tr><%

	%><tr><td>Update delay after a weekend for non-weekend-update games:</td><td><%
	%><input type="text" size="4" maxlength="20" name="AfterWeekendDelayHrs" value="<%
	iNumHrs = sDelay / 3600;
	sDelay -= iNumHrs * 3600;
	Write (iNumHrs);
	%>"> hrs, <%
	%><input type="text" size="4" maxlength="20" name="AfterWeekendDelayMin" value="<%
	iNumMin = sDelay / 60;
	sDelay -= iNumMin * 60;
	Write (iNumMin);
	%>"> min, <%
	%><input type="text" size="4" maxlength="20" name="AfterWeekendDelaySec" value="<%
	Write (sDelay);
	%>"> secs<%
	%></td></tr><%

	%><tr><td>Maximum number of updates before games close:</td><%
	%><td><input type="text" name="MaxNumUpdatesBeforeClose" size="5" maxlength="20" value="<% 
		Write (iMaxNumUpdatesBeforeClose); %>"></td></tr><%

	%><tr><td>Default number of updates before games close:</td><%
	%><td><input type="text" name="DefaultNumUpdatesBeforeClose" size="5" maxlength="20" value="<% 
		Write (iDefaultNumUpdatesBeforeClose); %>"></td></tr><%

	%><tr><td>Default Bridier configuration:</td><%
	%><td><select name="BridierDefault"><%
	%><option<%
	if (iSystemOptions & DEFAULT_BRIDIER_GAMES) {
		%> selected<%
	}
	%> value="<% Write (DEFAULT_BRIDIER_GAMES); %>">Grudge games count for Bridier Scoring by default</option><%

	%><option<%
	if (!(iSystemOptions & DEFAULT_BRIDIER_GAMES)) {
		%> selected<%
	}
	%> value="0">Grudge games don't count for Bridier Scoring by default</option><%

	%></select></td></tr><%

	%><tr><%
	%><td>Empire names exposed:</td><%
	%><td><select name="DefaultNamesListed"><%
	%><option<%
	if (iSystemOptions & DEFAULT_NAMES_LISTED) {
		%> selected<%
	}
	%> value="<% Write (DEFAULT_NAMES_LISTED); %>">Empire names exposed by default on game lists</option><%
	%><option<%
	if (!(iSystemOptions & DEFAULT_NAMES_LISTED)) {
		%> selected<%
	}
	%> value="0">Empire names not exposed by default</option><%
	%></select></td><%
	%></tr><%

	%><tr><%
	%><td>Games open to spectators:</td><%
	%><td><select name="DefaultSpectator"><%
	%><option<%
	if (iSystemOptions & DEFAULT_ALLOW_SPECTATORS) {
		%> selected<%
	}
	%> value="<% Write (DEFAULT_ALLOW_SPECTATORS); %>">Games open to spectators by default</option><%
	%><option<%
	if (!(iSystemOptions & DEFAULT_ALLOW_SPECTATORS)) {
		%> selected<%
	}
	%> value="0">Games closed to spectators by default</option><%
	%></select></td><%
	%></tr><%

	int iFilterIP = iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS);
	int iFilterId = iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID | DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID);

	%><tr><%
	%><td>Default empire filtering by IP Address:</td><%
	%><td><select name="FilterIP"><%
	%><option<%

	if (iFilterIP == 0) {
		%> selected<%
	}

	%> value="0">Ignore entry with duplicate IP address</option><option<%

	if (iFilterIP == DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
		%> selected<%
	}

	%> value="<% Write (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS); %>"><%
	%>Warn on entry with duplicate IP address</option><option<%

	if (iFilterIP == DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
		%> selected<%
	}

	%> value="<% Write (DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS); %>"><%
	%>Reject on entry with duplicate IP address</option><option<%

	if (iFilterIP == (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) {
		%> selected<%
	}

	%> value="<% Write (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS); %>"><%
	%>Warn and reject on entry with duplicate IP address</option></select></td><%

	%></tr><%

	%><tr><%
	%><td>Default empire filtering by Session Id:</td><%
	%><td><select name="FilterId"><%
	%><option<%

	if (iFilterId == 0) {
		%> selected<%
	}

	%> value="0"><%

	%>Ignore entry with duplicate Session Id</option><option<%

	if (iFilterId == DEFAULT_WARN_ON_DUPLICATE_SESSION_ID) {
		%> selected<%
	}

	%> value="<% Write (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID); %>"><%
	%>Warn on entry with duplicate Session Id</option><option<%

	if (iFilterId == DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID) {
		%> selected<%
	}

	%> value="<% Write (DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID); %>"><%
	%>Reject on entry with duplicate Session Id</option><option<%

	if (iFilterId == (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID | DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID)) {
		%> selected<%
	}

	%> value="<% Write (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID | DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID); %>"><%
	%>Warn and reject on entry with duplicate Session Id</option></select></td><%

	%></tr><%

	%></table><p><%


	%><h3>Ship Parameters</h3><%

	%><table width="75%"><%

	iValue = gcConfig.iShipBehavior & (COLONY_USE_MULTIPLIED_BUILD_COST | COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT);

	%><tr><td>Colony behavior:</td><td><select name="ColonyBehavior"><%
	%><option<%
	if (iValue == 0) {
		%> selected<%
	} %> value="<% Write (0); %>">Use simple build cost and exponential population deposit</option><%
	
	%><option<%
	if (iValue == COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT) {
		%> selected<%
	} %> value="<% Write (COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT);
	%>">Use simple build cost and multiplied population deposit</option><%
	
	%><option<%
	if (iValue == COLONY_USE_MULTIPLIED_BUILD_COST) {
		%> selected<%
	} %> value="<% Write (COLONY_USE_MULTIPLIED_BUILD_COST);
	%>">Use multiplied build cost and exponential population deposit</option><%
	
	%><option<%
	if (iValue == (COLONY_USE_MULTIPLIED_BUILD_COST | COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT)) {
		%> selected<%
	} %> value="<% Write (COLONY_USE_MULTIPLIED_BUILD_COST | COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT);
	%>">Use multiplied build cost and multiplied population deposit</option><%

	%></select></td></tr><%


	%><tr><td>Colony simple build cost:</td><td><%
	%><input type="text" size="4" maxlength="10" name="ColonySimpleBuildCost"<%
	%> value="<% Write (gcConfig.iColonySimpleBuildFactor); %>"></td></tr><%

	%><tr><td>Colony multiplied build cost factor:</td><td><%
	%><input type="text" size="6" maxlength="10" name="ColonyMultipliedBuildCost"<%
	%> value="<% Write (gcConfig.fColonyMultipliedBuildFactor); %>"></td></tr><%

	%><tr><td>Colony multiplied deposit factor:</td><td><%
	%><input type="text" size="6" maxlength="10" name="ColonyMultipliedDepositFactor"<%
	%> value="<% Write (gcConfig.fColonyMultipliedDepositFactor); %>"></td></tr><%

	%><tr><td>Colony exponential deposit factor:</td><td><%
	%><input type="text" size="6" maxlength="10" name="ColonyExponentialDepositFactor"<%
	%> value="<% Write (gcConfig.fColonyExponentialDepositFactor); %>"></td></tr><%

	%><tr><td>Engineer cost of opening or closing links:</td><td><%
	%><input type="text" size="6" maxlength="10" name="EngineerCost"<%
	%> value="<% Write (gcConfig.fEngineerLinkCost); %>"></td></tr><%

	iValue = gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE;

	%><tr><td>Stargate behavior:</td><td><select name="StargateBehavior"><%
	%><option<%
	if (iValue == 0) {
		%> selected<%
	} %> value="<% Write (0);
	%>">Stargates are not limited by their range factor</option><%

	%><option<%
	if (iValue == STARGATE_LIMIT_RANGE) {
		%> selected<%
	} %> value="<% Write (STARGATE_LIMIT_RANGE);
	%>">Stargates are limited by their range factor</option><%
	%></select></td></tr><%

	%><tr><td>Stargate cost of stargating ships:</td><td><%
	%><input type="text" size="6" maxlength="10" name="StargateCost"<%
	%> value="<% Write (gcConfig.fStargateGateCost); %>"></td></tr><%

	%><tr><td>Stargate range factor:</td><td><%
	%><input type="text" size="6" maxlength="10" name="StargateRange"<%
	%> value="<% Write (gcConfig.fStargateRangeFactor); %>"></td></tr><%

	iValue = gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE;

	%><tr><td>Jumpgate behavior:</td><td><select name="JumpgateBehavior"><%
	%><option<%
	if (iValue == 0) {
		%> selected<%
	} %> value="<% Write (0);
	%>">Jumpgates are not limited by their range factor</option><%

	%><option<%
	if (iValue == JUMPGATE_LIMIT_RANGE) {
		%> selected<%
	} %> value="<% Write (JUMPGATE_LIMIT_RANGE);
	%>">Jumpgates are limited by their range factor</option><%
	%></select></td></tr><%

	%><tr><td>Jumpgate cost of jumpgating ships:</td><td><%
	%><input type="text" size="6" maxlength="10" name="JumpgateCost"<%
	%> value="<% Write (gcConfig.fJumpgateGateCost); %>"></td></tr><%

	%><tr><td>Jumpgate range factor:</td><td><%
	%><input type="text" size="6" maxlength="10" name="JumpgateRange"<%
	%> value="<% Write (gcConfig.fJumpgateRangeFactor); %>"></td></tr><%

	%><tr><td>Terraformer agriculture multiplier:</td><td><%
	%><input type="text" size="6" maxlength="10" name="TerraformerFactor"<%
	%> value="<% Write (gcConfig.fTerraformerPlowFactor); %>"></td></tr><%

	%><tr><td>Troopship invasion multiplier:</td><td><%
	%><input type="text" size="6" maxlength="10" name="TroopshipInvasion"<%
	%> value="<% Write (gcConfig.fTroopshipInvasionFactor); %>"></td></tr><%

	%><tr><td>Troopship failure factor:</td><td><%
	%><input type="text" size="6" maxlength="10" name="TroopshipFailure"<%
	%> value="<% Write (gcConfig.fTroopshipFailureFactor); %>"></td></tr><%

	%><tr><td>Troopship success factor:</td><td><%
	%><input type="text" size="6" maxlength="10" name="TroopshipSuccess"<%
	%> value="<% Write (gcConfig.fTroopshipSuccessFactor); %>"></td></tr><%

	%><tr><td>Doomsday annihilation multiplier:</td><td><%
	%><input type="text" size="6" maxlength="10" name="DoomsdayMultiplier"<%
	%> value="<% Write (gcConfig.fDoomsdayAnnihilationFactor); %>"></td></tr><%

	%><tr><td>Carrier battle cost:</td><td><%
	%><input type="text" size="6" maxlength="10" name="CarrierCost"<%
	%> value="<% Write (gcConfig.fCarrierCost); %>"></td></tr><%

	%><tr><td>Builder minimum BR to create planet:</td><td><%
	%><input type="text" size="6" maxlength="10" name="MinBuilderBR"<%
	%> value="<% Write (gcConfig.fBuilderMinBR); %>"></td></tr><%


	iValue = gcConfig.iShipBehavior & CLOAKER_CLOAK_ON_BUILD;

	%><tr><td>Cloaker behavior:</td><td><select name="CloakerBehavior"><%

	%><option<%
	if (iValue == CLOAKER_CLOAK_ON_BUILD) {
		%> selected<%
	} %> value="<% Write (CLOAKER_CLOAK_ON_BUILD); %>">Cloakers are cloaked when built</option><%

	%><option<%
	if (iValue == 0) {
		%> selected<%
	} %> value="0">Cloakers are uncloaked when built</option><%

	%></select></td></tr><%


	iValue = gcConfig.iShipBehavior & MORPHER_CLOAK_ON_CLOAKER_MORPH;

	%><tr><td>Morpher behavior:</td><td><select name="MorpherBehavior"><%

	%><option <%
	if (iValue == MORPHER_CLOAK_ON_CLOAKER_MORPH) {
		%>selected <%
	} %>value="<% Write (MORPHER_CLOAK_ON_CLOAKER_MORPH);
	%>">Morphers are cloaked when they morph into cloakers</option><%

	%><option <%
	if (iValue == 0) {
		%>selected <%
	} %>value="0">Morphers are uncloaked when they morph into cloakers</option><%

	%></select></td></tr><%


	%><tr><td>Morpher morphing cost:</td><td><%
	%><input type="text" size="6" maxlength="10" name="MorpherCost"<%
	%> value="<% Write (gcConfig.fMorpherCost); %>"></td></tr><%

	%><tr><td>Compatibility flags:</td><td><%

	%><br><input type="checkbox"<%
	if (gcConfig.iShipBehavior & COLONY_DISABLE_SETTLES) {
		%> checked<%
	}
	%> name="DisableColonySettles"> Disable colony population deposit on owned planets<%

	%><br><input type="checkbox"<%
	if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {
		%> checked<%
	}
	%> name="DisableFriendlyTerraformers"> Disable terraforming non-owned planets<%

	%><br><input type="checkbox"<%
	if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_MULTIPLE) {
		%> checked<%
	}
	%> name="DisableMultipleTerraformers"> Disable multiple terraformers acting on one planet during an update<%

	%><br><input type="checkbox"<%
	if (gcConfig.iShipBehavior & COLONY_DISABLE_SURVIVAL) {
		%> checked<%
	}
	%> name="DisableColonySurvival"> Disable colony survival after colonizing or settling<%

	%><br><input type="checkbox"<%
	if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_SURVIVAL) {
		%> checked<%
	}
	%> name="DisableTerraformerSurvival"> Disable terraformer survival after terraforming<%

	%><br><input type="checkbox"<%
	if (gcConfig.iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL) {
		%> checked<%
	}
	%> name="DisableTroopshipSurvival"> Disable troopship survival after invading<%

	%><br><input type="checkbox"<%
	if (gcConfig.iShipBehavior & MINEFIELD_DISABLE_DETONATE) {
		%> checked<%
	}
	%> name="DisableMinefieldDetonate"> Disable minefield detonation<%

	%></td></tr></table><%


	%><h3>Map Parameters</h3><%

	%><table width="75%"><%

	%><tr><td>Percent chance a planet will be a linked to an adjacent planet:</td><td><%
	%><input type="text" size="4" maxlength="10" name="ChanceNewLinkForms"<%
	%> value="<% Write (mcConfig.iChanceNewLinkForms); %>"></td></tr><%

	%><tr><td>Resource allocation deviation multiplier:</td><td><%
	%><input type="text" size="6" maxlength="10" name="ResourceRand"<%
	%> value="<% Write (mcConfig.fResourceAllocationRandomizationFactor); %>"></td></tr><%

	%><tr><td>Maximum map deviation:</td><td><%
	%><input type="text" size="4" maxlength="10" name="MapDeviation"<%
	%> value="<% Write (mcConfig.iMapDeviation); %>"></td></tr><%

	%><tr><td>Percent chance a new planet will be linked to the last created planet (large map):</td><td><%
	%><input type="text" size="4" maxlength="10" name="ChanceLinkedLarge"<%
	%> value="<% Write (mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap); %>"></td></tr><%

	%><tr><td>Percent chance a new planet will be linked to the last created planet (small map):</td><td><%
	%><input type="text" size="4" maxlength="10" name="ChanceLinkedSmall"<%
	%> value="<% Write (mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap); %>"></td></tr><%

	%><tr><td>Large map threshold:</td><td><%
	%><input type="text" size="4" maxlength="10" name="LargeMapThreshold"<%
	%> value="<% Write (mcConfig.iLargeMapThreshold); %>"></td></tr><%

	%></table><%

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

	%><input type="hidden" name="GameAdminPage" value="1"><%

	if (iNumActiveGames == 0) {
		%><p>There are no active games on the server<%
	} else { 

		%><p>There <%
		if (iNumActiveGames == 1) { 
			%>is <strong>1</strong> active game<%
		} else { 
			%>are <strong><% Write (iNumActiveGames); %></strong> active games<%
		}

		%> (<strong><% Write (iNumOpenGames); %></strong> open, <strong><% Write (iNumClosedGames); 

		const char* pszTableColor = m_vTableColor.GetCharPtr();

		%></strong> closed)<p><table width="90%"><tr><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Game</strong></th><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Update period</strong></th><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Updates</strong></th><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Started</strong></th><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Time remaining</strong></th><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>State</strong></th><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Password</strong></th><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Empires</strong></th><%
		%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Administer Game</strong></th></tr><%

		Variant* pvEmpireKey, vGamePassword, vName;
		int iNumActiveEmpires, j, iNumUpdates;
		bool bPaused, bOpen, bStarted;

		char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

		// Sort games by gameclass
		Algorithm::QSortTwoAscending<int, int> (piGameClass, piGameNumber, iNumActiveGames);

		// Sort games by gamenumber
		int iBegin = 0, iNumToSort;
		int iCurrentGameClass = piGameClass[0];

		for (i = 1; i < iNumActiveGames; i ++) {

			if (piGameClass[i] != iCurrentGameClass) {

				iNumToSort = i - iBegin;
				if (iNumToSort > 1) {
					Algorithm::QSortTwoAscending<int, int> (piGameNumber + iBegin, piGameClass + iBegin, iNumToSort);
				}

				iBegin = i;
				iCurrentGameClass = piGameClass[i];
			}
		}

		iNumToSort = i - iBegin;
		if (iNumToSort > 1) {
			Algorithm::QSortTwoAscending<int, int> (piGameNumber + iBegin, piGameClass + iBegin, iNumToSort);
		}
		iCurrentGameClass = piGameClass[0];

		bool bExists, bAdminPaused;
		Seconds iSeconds, iSecondsSince, iSecondsUntil;
		int iGameState;
		UTCTime tCreationTime;

		Variant* pvEmpireName;
		char pszAdmin [512];

		for (i = 0; i < iNumActiveGames; i ++) {

			// Check game for updates
			if (g_pGameEngine->CheckGameForUpdates (
				piGameClass[i],
				piGameNumber[i],
				&bExists
				) != OK ||

				g_pGameEngine->DoesGameExist (piGameClass[i], piGameNumber[i], &bExists) != OK || !bExists ||
				g_pGameEngine->GetGameClassName (piGameClass[i], pszGameClassName) != OK ||
				g_pGameEngine->GetGameClassUpdatePeriod (piGameClass[i], &iSeconds) != OK ||
				g_pGameEngine->IsGamePaused (piGameClass[i], piGameNumber[i], &bPaused) != OK ||
				g_pGameEngine->IsGameAdminPaused (piGameClass[i], piGameNumber[i], &bAdminPaused) != OK ||
				g_pGameEngine->IsGameOpen (piGameClass[i], piGameNumber[i], &bOpen) != OK ||
				g_pGameEngine->HasGameStarted (piGameClass[i], piGameNumber[i], &bStarted) != OK ||
				g_pGameEngine->GetGamePassword (piGameClass[i], piGameNumber[i], &vGamePassword) != OK ||
				g_pGameEngine->GetGameCreationTime (piGameClass[i], piGameNumber[i], &tCreationTime) != OK ||
				g_pGameEngine->GetEmpiresInGame (piGameClass[i], piGameNumber[i], &pvEmpireKey, 
					&iNumActiveEmpires) != OK ||
				g_pGameEngine->GetGameUpdateData (piGameClass[i], piGameNumber[i], &iSecondsSince, 
					&iSecondsUntil, &iNumUpdates, &iGameState) != OK
				) {
				continue;
			}

			if (i > 0 && piGameClass[i] != iCurrentGameClass) {
				iCurrentGameClass = piGameClass[i];
				%><tr><td align="center" colspan="9"><% 
				WriteSeparatorString (m_iSeparatorKey);
				%></td></tr><%
			}

			pvEmpireName = new Variant [iNumActiveEmpires];

			for (j = 0; j < iNumActiveEmpires; j ++) {

				iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[j], pvEmpireName + j);
				if (iErrCode != OK) {
					pvEmpireName[j] = "";
				}
			}

			WriteGameAdministratorGameData (pszGameClassName, piGameNumber[i], iSeconds, iSecondsUntil, 
				iNumUpdates, bOpen, bPaused, bAdminPaused, bStarted, 
				vGamePassword.GetCharPtr(), pvEmpireName, iNumActiveEmpires, tCreationTime);

			%><td align="center"><% 

			sprintf (pszAdmin, "AdministerGame%i.%i", piGameClass[i], piGameNumber[i]);

			WriteButtonString (
				m_iButtonKey,
				"AdministerGame",
				"Administer Game", 
				pszAdmin
				);

			g_pGameEngine->FreeData (pvEmpireKey);
			delete [] pvEmpireName;

			%></td></tr><%
		}

		%></table><%

		delete [] piGameClass;
		delete [] piGameNumber;
	}

	}
	break;

case 2: 

	%><input type="hidden" name="GameAdminPage" value="2"><p><h3>Create a new GameClass:</h3><% 
	WriteCreateGameClassString (SYSTEM, false);
	%><p><%

	WriteButton (BID_CANCEL);
	WriteButton (BID_CREATENEWGAMECLASS);

	break;

case 3:

	{

	bool bStarted, bExists, bPaused, bOpen, bAdminPaused;
	Variant vGamePassword, * pvEmpireKey;
	int iNumUpdates, iNumActiveEmpires, iGameState;
	Seconds iSeconds, iSecondsUntil, iSecondsSince;
	const char* pszTableColor = m_vTableColor.GetCharPtr();
	UTCTime tCreationTime;

	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	if (g_pGameEngine->CheckGameForUpdates (
		iGameClass,
		iGameNumber,
		&bExists
		) != OK ||

		g_pGameEngine->DoesGameExist (iGameClass, iGameNumber, &bExists) != OK || !bExists ||
		g_pGameEngine->GetGameClassName (iGameClass, pszGameClassName) != OK ||
		g_pGameEngine->GetGameClassUpdatePeriod (iGameClass, &iSeconds) != OK ||
		g_pGameEngine->IsGamePaused (iGameClass, iGameNumber, &bPaused) != OK ||
		g_pGameEngine->IsGameAdminPaused (iGameClass, iGameNumber, &bAdminPaused) != OK ||
		g_pGameEngine->IsGameOpen (iGameClass, iGameNumber, &bOpen) != OK ||
		g_pGameEngine->HasGameStarted (iGameClass, iGameNumber, &bStarted) != OK ||
		g_pGameEngine->GetGamePassword (iGameClass, iGameNumber, &vGamePassword) != OK ||
		g_pGameEngine->GetEmpiresInGame (iGameClass, iGameNumber, &pvEmpireKey, &iNumActiveEmpires) != OK ||
		g_pGameEngine->GetGameCreationTime (iGameClass, iGameNumber, &tCreationTime) != OK ||
		g_pGameEngine->GetGameUpdateData (iGameClass, iGameNumber, &iSecondsSince, &iSecondsUntil, 
			&iNumUpdates, &iGameState) != OK
		) {

		AddMessage ("The game could not be administered. It may no longer exist");
		goto AllGames;
	}

	%><input type="hidden" name="GameAdminPage" value="3"><% 
	%><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
	%><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

	%><p><h3>Administer <% Write (pszGameClassName); %> <% Write (iGameNumber); %>:</h3><%

	%><p><table width="90%"><tr><%
	%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Game</strong></th><%
	%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Update period</strong></th><%
	%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Updates</strong></th><%
	%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Started</strong></th><%
	%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Time remaining</strong></th><%
	%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>State</strong></th><%
	%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Password</strong></th><%
	%><th align="center" bgcolor="#<% Write (pszTableColor); %>"><strong>Empires</strong></th><%

	Variant* pvEmpireName = new Variant [iNumActiveEmpires];

	for (i = 0; i < iNumActiveEmpires; i ++) {

		iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[i], pvEmpireName + i);
		if (iErrCode != OK) {
			pvEmpireName[i] = "";
		}
	}

	WriteGameAdministratorGameData (pszGameClassName, iGameNumber, 
		iSeconds, iSecondsUntil, iNumUpdates, bOpen, bPaused, bAdminPaused, bStarted, 
		vGamePassword.GetCharPtr(), pvEmpireName, iNumActiveEmpires, tCreationTime);

	%></table><p><% 
	WriteSeparatorString (m_iSeparatorKey);
	%><p><%

	// Read game class data
	Variant* pvGameClassInfo = NULL;
	if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK) {

		%><p><%
		WriteOpenGameListHeader (m_vTableColor.GetCharPtr());

		// Best effort
		iErrCode = WriteOpenGameListData (iGameClass, iGameNumber, pvGameClassInfo, true);

		%></table><%
	}

	if (pvGameClassInfo != NULL) {
		g_pGameEngine->FreeData (pvGameClassInfo);
		pvGameClassInfo = NULL;
	}

	%><p><%
	WriteSeparatorString (m_iSeparatorKey);
	%><p><table width="90%"><%

	// View Map
	if (bStarted) {
		%><tr><td>View the game's map:</td><td><%
		WriteButton (BID_VIEWMAP);
		%></td></tr><%
	}

	// Change game password
	const char* pszPassword = vGamePassword.GetCharPtr();
	if (pszPassword != NULL && *pszPassword != '\0') {

		%><tr><td>Change the game's password:</td><td><%
		%><input type="text" name="NewPassword" size="<% Write (MAX_PASSWORD_LENGTH); %>" <%
		%>maxlength="<% Write (MAX_PASSWORD_LENGTH); %>" value="<% Write (pszPassword); %>"> <%
		WriteButton (BID_CHANGEPASSWORD);
		%></td></tr><%

	} else {

		%><tr><td>Password protect the game:</td><td><%
		%><input type="text" name="NewPassword" size="<% Write (MAX_PASSWORD_LENGTH); %>" <%
		%>maxlength="<% Write (MAX_PASSWORD_LENGTH); %>"> <%
		WriteButton (BID_CHANGEPASSWORD);
		%></td></tr><%
	}

	// Force game update or pause game
	if (bStarted) {

		%><tr><td>Force the game to update:</td><td><%
		WriteButton (BID_FORCEUPDATE);
		%></td></tr><%

		if (bAdminPaused) {
			%><tr><td>Unpause the game:</td><td><%
			WriteButton (BID_UNPAUSEGAME);
			%></td></tr><%
		} else {
			%><tr><td>Pause the game:</td><td><%
			WriteButton (BID_PAUSEGAME);
			%></td></tr><%
		}
	}

	// View empire info
	%><tr><td>View empire information:</td><td><%
	WriteButton (BID_VIEWEMPIREINFORMATION);
	%></td></tr><%

	%><tr><td>Search for empires with the same IP address:</td><td><%
	WriteButton (BID_SEARCHIPADDRESSES);
	%></td></tr><%

	%><tr><td>Search for empires with the same Session Id:</td><td><%
	WriteButton (BID_SEARCHSESSIONIDS);
	%></td></tr><%

	int iNumResigned, * piResignedKey;
	iErrCode = g_pGameEngine->GetResignedEmpiresInGame (iGameClass, iGameNumber, &piResignedKey, &iNumResigned);

	if (iErrCode == OK && iNumResigned > 0) {

		Variant vName;

		%><tr><td>Restore a resigned empire to the game:</td><td><select name="RestoreEmpireKey"><%

		for (i = 0; i < iNumResigned; i ++) {

			iErrCode = g_pGameEngine->GetEmpireName (piResignedKey[i], &vName);
			if (iErrCode == OK) {

				%><option value="<% Write (piResignedKey[i]); %>"><%
				Write (vName.GetCharPtr()); %></option><%
			}
		}

		delete [] piResignedKey;

		%> <%
		WriteButton (BID_RESTOREEMPIRE);
		%></td></tr><%
	}

	%><tr><td>Delete an empire from the game:</td><td><select name="DeleteEmpireKey"><%

	for (i = 0; i < iNumActiveEmpires; i ++) {
		%><option value="<% Write (pvEmpireKey[i].GetInteger()); %>"><%
		Write (pvEmpireName[i].GetCharPtr()); %></option><%
	}
	%> <%
	WriteButton (BID_DELETEEMPIRE);
	%></td></tr><%

	// Broadcast message to all empires
	%><tr><td>Broadcast a message to all empires in the game:<td><%
	%><textarea name="Message" rows="5" cols="45" wrap="physical"></textarea></td><td><%
	WriteButton (BID_SENDMESSAGE);
	%></td></tr><%

	// Kill game
	%><tr><td>Kill the game:</td><td><%
	WriteButton (BID_KILLGAME);
	%></td></tr><%

	%></table><%

	%><p><% WriteButton (BID_CANCEL);

	g_pGameEngine->FreeData (pvEmpireKey);
	delete [] pvEmpireName;

	}

	break;

case 4:
	{

	bool bStarted;

	iErrCode = g_pGameEngine->HasGameStarted (iGameClass, iGameNumber, &bStarted);
	if (iErrCode != OK) {
		goto AllGames;
	}

	if (!bStarted) {
		goto AllGames;
	}

	iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber);
	if (iErrCode != OK) {
		goto AllGames;
	}

	%><input type="hidden" name="GameAdminPage" value="4"><%
	%><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
	%><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

	RenderMap (iGameClass, iGameNumber, m_iEmpireKey, true, NULL, false);

	g_pGameEngine->SignalGameReader (iGameClass, iGameNumber);

	%><p><% WriteButton (BID_CANCEL);

	}
	break;

case 6:
	{

	Variant vOptions;

	int iLivePlanetKey, iDeadPlanetKey, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;

	bool bStarted, bFalse;

	IDatabase* pDatabase = g_pGameEngine->GetDatabase();
	IReadTable* pGameMap = NULL;
	void** ppPlanetData = NULL;

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

	iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber);
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

	iErrCode = pDatabase->GetTableForReading (pszGameMap, &pGameMap);
	if (iErrCode != OK) {
		AddMessage ("That game no longer exists");
		goto Cleanup;
	}

	iErrCode = pGameMap->ReadRow (iClickedPlanetKey, &ppPlanetData);
	if (iErrCode != OK) {
		AddMessage ("That game no longer exists");
		goto Cleanup;
	}

	m_iGameState |= STARTED;
	m_iGameClass = iGameClass;
	m_iGameNumber = iGameNumber;

	%><input type="hidden" name="GameAdminPage" value="6"><%
	%><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
	%><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

	%><p><table width="90%"><%

	// Best effort
	WriteUpClosePlanetString (NO_KEY, iClickedPlanetKey, 
		0, iLivePlanetKey, iDeadPlanetKey, 0, true, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel,
		1.0, (vOptions.GetInteger() & INDEPENDENCE) != 0, true, false, ppPlanetData, &bFalse);

Cleanup:

	if (pGameMap != NULL) {
		pGameMap->Release();
	}

	// Best effort
	g_pGameEngine->SignalGameReader (iGameClass, iGameNumber);

	if (ppPlanetData != NULL) {
		pDatabase->FreeData (ppPlanetData);
	}

	pDatabase->Release();

	if (iErrCode != OK) {
		goto AllGames;
	}

	%></table><p><%

	WriteButton (BID_VIEWMAP);

	}
	break;

case 7:

	{

	iErrCode = g_pGameEngine->WaitGameReader (iGameClass, iGameNumber);
	if (iErrCode != OK) {
		AddMessage ("That game no longer exists");
		goto AllGames;
	}

	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	iErrCode = g_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
	if (iErrCode != OK) {
		pszGameClassName[0] = '\0';
	}

	%><input type="hidden" name="GameAdminPage" value="7"><%
	%><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
	%><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

	%><p>Empire information for <%

	Write (pszGameClassName); %> <% Write (iGameNumber); %>:<p><%

	RenderEmpireInformation (iGameClass, iGameNumber, true);

	g_pGameEngine->SignalGameReader (iGameClass, iGameNumber);

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

	%><input type="hidden" name="GameAdminPage" value="8"><%
	%><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
	%><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

	%><p><table width="65%"><%
	%><tr><td align="center"><%
	%>Are you sure you want to kill <strong><% Write (pszGameClassName); %> <% Write (iGameNumber); 
	%></strong>?<p>If so, please send a message to its participants:<%
	%></td></tr></table><%
	%><p><textarea name="DoomMessage" rows="5" cols="45" wrap="physical"></textarea><p><%

	WriteButton (BID_CANCEL);
	WriteButton (BID_KILLGAME);

	}
	break;

default:

	Assert (false);
	break;
}

SYSTEM_CLOSE

%>