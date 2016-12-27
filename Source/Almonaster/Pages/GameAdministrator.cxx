<%
#include "Osal/Algorithm.h"
#include <stdio.h>

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

int iErrCode;

bool bInitialized;
iErrCode = InitializeEmpire(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

int i;

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

        m_bRedirectTest = false;

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

            iErrCode = GetMinNumSecsPerUpdateForSystemGameClass (&iSystemMinNumSecsPerUpdate);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetMaxNumSecsPerUpdateForSystemGameClass (&iSystemMaxNumSecsPerUpdate);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetMaxNumEmpiresForSystemGameClass (&iSystemMaxNumEmpires);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetMaxNumPlanetsForSystemGameClass (&iSystemMaxNumPlanets);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::MaxNumUpdatesBeforeClose, &vMaxNumUpdatesBeforeClose);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::DefaultNumUpdatesBeforeClose, &vDefaultNumUpdatesBeforeClose);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetMinNumSecsPerUpdateForPersonalGameClass (&iPersonalMinNumSecsPerUpdate);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetMaxNumSecsPerUpdateForPersonalGameClass (&iPersonalMaxNumSecsPerUpdate);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetMaxNumEmpiresForPersonalGameClass (&iPersonalMaxNumEmpires);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetMaxNumPlanetsForPersonalGameClass (&iPersonalMaxNumPlanets);
            RETURN_ON_ERROR(iErrCode);

            Variant vMaxResourcesPerPlanet, vMaxResourcesPerPlanetPersonal, vMaxInitialTechLevel, vMaxInitialTechLevelPersonal, vMaxTechDev, vMaxTechDevPersonal;

            iErrCode = GetSystemProperty (SystemData::MaxResourcesPerPlanet, &vMaxResourcesPerPlanet);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, &vMaxResourcesPerPlanetPersonal);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::MaxInitialTechLevel, &vMaxInitialTechLevel);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::MaxInitialTechLevelPersonal, &vMaxInitialTechLevelPersonal);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::MaxTechDev, &vMaxTechDev);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::MaxTechDevPersonal, &vMaxTechDevPersonal);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::AfterWeekendDelay, &vDelay);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemOptions (&iSystemOptions);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetSystemProperty (SystemData::PrivilegeForUnlimitedEmpires, &vUnlimitedEmpirePrivilege);
            RETURN_ON_ERROR(iErrCode);

            GameConfiguration gcOldConfig, gcNewConfig;
            MapConfiguration mcOldConfig, mcNewConfig;

            memset(&gcOldConfig, 0, sizeof (gcOldConfig));
            memset(&gcNewConfig, 0, sizeof (gcNewConfig));
            memset(&mcOldConfig, 0, sizeof (mcOldConfig));
            memset(&mcNewConfig, 0, sizeof (mcNewConfig));

            iErrCode = GetGameConfiguration (&gcOldConfig);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetMapConfiguration (&mcOldConfig);
            RETURN_ON_ERROR(iErrCode);

            // Empires
            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxEmps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != iSystemMaxNumEmpires)
            {
                if (iNewValue < 2)
                {
                   AddMessage ("Invalid maximum number of empires for a System GameClass");
                }
                else
                {
                    iErrCode = SetMaxNumEmpiresForSystemGameClass(iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum number of empires for a System GameClass was updated");
                }
            }

            // Planets
            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxPlanets")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != iSystemMaxNumPlanets)
            {
                if (iNewValue < 2)
                {
                   AddMessage ("Invalid maximum number of planets for a System GameClass");
                }
                else
                {
                    iErrCode = SetMaxNumPlanetsForSystemGameClass(iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum number of planets for a System GameClass was updated");
                }
            }

            // MaxNumUpdatesBeforeClose
            if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumUpdatesBeforeClose")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != vMaxNumUpdatesBeforeClose.GetInteger())
            {
                if (iNewValue < 2)
                {
                   AddMessage ("Invalid maximum number of updates before closing");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::MaxNumUpdatesBeforeClose, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum number of updates before closing was updated");
                }
            }

            // DefaultNumUpdatesBeforeClose
            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultNumUpdatesBeforeClose")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != vDefaultNumUpdatesBeforeClose.GetInteger())
            {
                if (iNewValue < 1)
                {
                   AddMessage ("Invalid default number of updates before closing");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::DefaultNumUpdatesBeforeClose, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The default number of updates before closing was updated");
                }
            }

            // BridierDefault
            if ((pHttpForm = m_pHttpRequest->GetForm ("BridierDefault")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != (iSystemOptions & DEFAULT_BRIDIER_GAMES))
            {
                iErrCode = SetSystemOption(DEFAULT_BRIDIER_GAMES, iNewValue != 0);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default Bridier configuration was updated");
            }

            // DefaultNamesListed
            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultNamesListed")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != (iSystemOptions & DEFAULT_NAMES_LISTED))
            {
                iErrCode = SetSystemOption (DEFAULT_NAMES_LISTED, iNewValue != 0);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default names listed configuration was updated");
            }

            // DefaultSpectator
            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultSpectator")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != (iSystemOptions & DEFAULT_ALLOW_SPECTATORS))
            {
                iErrCode = SetSystemOption (DEFAULT_ALLOW_SPECTATORS, iNewValue != 0);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default spectator game setting was updated");
            }

            // FilterIP
            if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIP")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue() & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS);

            if (iNewValue != (iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS))) {

                if (iNewValue & DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
                    iErrCode = SetSystemOption (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS, true);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    iErrCode = SetSystemOption (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS, false);
                    RETURN_ON_ERROR(iErrCode);
                }

                if (iNewValue & DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
                    iErrCode = SetSystemOption (DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS, true);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    iErrCode = SetSystemOption (DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS, false);
                    RETURN_ON_ERROR(iErrCode);
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
                    iErrCode = SetSystemOption (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID, true);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    iErrCode = SetSystemOption (DEFAULT_WARN_ON_DUPLICATE_SESSION_ID, false);
                    RETURN_ON_ERROR(iErrCode);
                }

                if (iNewValue & DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID) {
                    iErrCode = SetSystemOption (DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID, true);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    iErrCode = SetSystemOption (DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID, false);
                    RETURN_ON_ERROR(iErrCode);
                }

                AddMessage ("The default Session Id filtering configuration was updated");
            }
            
            // FilterIdle
            if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIdle")) == NULL) {
                goto Redirection;
            }
            bNewValue = pHttpForm->GetIntValue() != 0;

            if (bNewValue != ((iSystemOptions & DEFAULT_RESTRICT_IDLE_EMPIRES) != 0)) {
            
                iErrCode = SetSystemOption (DEFAULT_RESTRICT_IDLE_EMPIRES, bNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default idle empire blocking configuration was updated");
            }

            // MaxResourcesPerPlanet
            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxResourcesPerPlanet")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != vMaxResourcesPerPlanet.GetInteger())
            {
                if (iNewValue < 1)
                {
                    AddMessage ("Invalid maximum average resources per attribute per planet for a System GameClass");
                }
                else
                {
                    iErrCode = SetSystemProperty (SystemData::MaxResourcesPerPlanet, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum average resources per attribute per planet for a System GameClass was updated");
                }
            }

            // MaxInitialTechLevel
            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxInitialTechLevel")) == NULL) {
                goto Redirection;
            }
            float fNewValue = pHttpForm->GetFloatValue();

            if (fNewValue != vMaxInitialTechLevel.GetFloat())
            {
                if (fNewValue < 1)
                {
                    AddMessage ("Invalid maximum initial tech level for a System GameClass");
                }
                else
                {
                    iErrCode = SetSystemProperty (SystemData::MaxInitialTechLevel, fNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum initial tech level for a System GameClass was updated");
                }
            }

            // MaxTechDev
            if ((pHttpForm = m_pHttpRequest->GetForm ("SMaxTechDev")) == NULL) {
                goto Redirection;
            }
            fNewValue = pHttpForm->GetFloatValue();

            if (fNewValue != vMaxTechDev.GetFloat())
            {
                if (fNewValue < 0)
                {
                    AddMessage ("Invalid maximum tech development for a System GameClass");
                }
                else
                {
                    iErrCode = SetSystemProperty (SystemData::MaxTechDev, fNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum tech development for a System GameClass was updated");
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

            if (iNewValue != iSystemMinNumSecsPerUpdate)
            {
                if (iNewValue < 1)
                {
                    AddMessage ("Invalid minimum update period for a System GameClass");
                }
                else
                {
                    iErrCode = SetMinNumSecsPerUpdateForSystemGameClass (iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The minimum update period for a System GameClass was updated");
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

            if (iNewValue != iSystemMaxNumSecsPerUpdate)
            {
                if (iNewValue < 1)
                {
                    AddMessage ("Invalid maximum update period for a System GameClass");
                }
                else
                {
                    iErrCode = SetMaxNumSecsPerUpdateForSystemGameClass (iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum update period for a System GameClass was updated");
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
            if (iNewValue != iOldValue && iNewValue >= 0)
            {
                if (iNewValue < 0)
                {
                    AddMessage ("Invalid maximum number of personal GameClasses");
                }
                else
                {
                    iErrCode = SetSystemProperty (SystemData::MaxNumPersonalGameClasses, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum number of personal GameClasses was updated");
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
            if (iNewValue != iOldValue && iNewValue >= 0)
            {
                if (iNewValue < 0)
                {
                    AddMessage ("Invalid maximum number of personal tournaments");
                }
                else
                {
                    iErrCode = SetSystemProperty (SystemData::MaxNumPersonalTournaments, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum number of personal tournaments was updated");
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
            if (iNewValue != iOldValue && iNewValue >= 0)
            {
                if (iNewValue < 1)
                {
                    AddMessage ("Invalid maximum number of GameClasses per personal tournament");
                }
                else
                {
                    iErrCode = SetSystemProperty (SystemData::MaxNumGameClassesPerPersonalTournament, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum number of GameClasses per personal tournament was updated");
                }
            }

            // Empires
            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxEmps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != iPersonalMaxNumEmpires)
            {
                if (iNewValue < 2)
                {
                    AddMessage ("Invalid maximum number of empires for a personal GameClass");
                }
                else
                {
                    iErrCode = SetMaxNumEmpiresForPersonalGameClass(iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum number of empires for a personal GameClass was updated");
                }
            }

            // Planets
            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxPlanets")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != iPersonalMaxNumPlanets)
            {
                if (iNewValue < 1)
                {
                    AddMessage ("Invalid maximum number of planets for a personal GameClass");
                }
                else
                {
                    iErrCode = SetMaxNumPlanetsForPersonalGameClass(iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum number of planets for a personal GameClass was updated");
                }
            }

            // MaxResourcesPerPlanet
            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxResourcesPerPlanet")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != vMaxResourcesPerPlanetPersonal.GetInteger())
            {
                if (iNewValue < 1)
                {
                    AddMessage ("Invalid maximum average resources per attribute per planet for a personal GameClass");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::MaxResourcesPerPlanetPersonal, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum average resources per attribute per planet for a personal GameClass was updated");
                }
            }

            // MaxInitialTechLevel
            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxInitialTechLevel")) == NULL) {
                goto Redirection;
            }
            fNewValue = pHttpForm->GetFloatValue();

            if (fNewValue != vMaxInitialTechLevelPersonal.GetFloat())
            {
                if (fNewValue < 1)
                {
                    AddMessage ("Invalid maximum initial tech level for a personal GameClass");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::MaxInitialTechLevelPersonal, fNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum initial tech level for a personal GameClass was updated");
                }
            }

            // MaxTechDev
            if ((pHttpForm = m_pHttpRequest->GetForm ("PMaxTechDev")) == NULL) {
                goto Redirection;
            }
            fNewValue = pHttpForm->GetFloatValue();

            if (fNewValue != vMaxTechDevPersonal.GetFloat())
            {
                if (fNewValue < 1)
                {
                    AddMessage ("Invalid maximum tech development for a personal GameClass");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::MaxTechDevPersonal, fNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum tech development for a personal GameClass was updated");
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

            if (iNewValue != iPersonalMinNumSecsPerUpdate)
            {
                if (iNewValue < 1)
                {
                    AddMessage ("Invalid minimum update period for a personal GameClass");
                }
                else
                {
                    iErrCode = SetMinNumSecsPerUpdateForPersonalGameClass(iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The minimum update period for a personal GameClass was updated");
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

            if (iNewValue != iPersonalMaxNumSecsPerUpdate)
            {
                if (iNewValue < 1)
                {
                    AddMessage ("Invalid maximum update period for a personal GameClass");
                }
                else
                {
                    iErrCode = SetMaxNumSecsPerUpdateForPersonalGameClass(iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The maximum update period for a personal GameClass was updated");
                }
            }

            // UnlimitedPrivilege
            if ((pHttpForm = m_pHttpRequest->GetForm ("UnlimitedPrivilege")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != vUnlimitedEmpirePrivilege.GetInteger())
            {
                if (iNewValue < NOVICE || iNewValue > ADMINISTRATOR)
                {
                    AddMessage ("The privilege level required to create games with unlimited empires could not be set");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::PrivilegeForUnlimitedEmpires, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The privilege level required to create games with unlimited empires was updated");
                }
            }
            
            // PauseGames
            if ((pHttpForm = m_pHttpRequest->GetForm ("PauseGames")) == NULL) {
                goto Redirection;
            }
            bNewValue = pHttpForm->GetIntValue() != 0;

            if (bNewValue != ((iSystemOptions & PAUSE_GAMES_BY_DEFAULT) != 0))
            {
                iErrCode = SetSystemOption (PAUSE_GAMES_BY_DEFAULT, bNewValue);
                RETURN_ON_ERROR(iErrCode);
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

                if (iNewValue < 0)
                {
                    AddMessage("Invalid after weekend delay");
                }
                else if (iNewValue > MAX_AFTER_WEEKEND_DELAY)
                {
                    AddMessage ("Invalid after weekend delay - must be less than ");
                    AppendMessage (MAX_AFTER_WEEKEND_DELAY / (60 * 60));
                    AppendMessage (" hours");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::AfterWeekendDelay, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
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

                iErrCode = SetGameConfiguration(gcNewConfig);
                if (iErrCode == ERROR_INVALID_ARGUMENT)
                {
                    AddMessage("Invalid argument detected when updating the server's game configuration");
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The server's game configuration was updated");
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

                iErrCode = SetMapConfiguration(mcNewConfig);
                if (iErrCode == ERROR_INVALID_ARGUMENT)
                {
                    AddMessage("Invalid argument detected when updating the server's map configuration");
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The server's map configuration was updated");
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

                if (VerifyCategoryName("SuperClass", pszNewValue, MAX_SUPER_CLASS_NAME_LENGTH, true))
                {
                    int iKey;
                    iErrCode = CreateSuperClass (pszNewValue, &iKey);
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
                        RETURN_ON_ERROR(iErrCode);
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
                iErrCode = DeleteSuperClass (pHttpForm->GetIntValue(), &bFlag);
                RETURN_ON_ERROR(iErrCode);
                if (bFlag) {
                    AddMessage ("The SuperClass was deleted");
                } else {
                    AddMessage ("The SuperClass could not be deleted because it still has GameClasses");
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

                iErrCode = RenameSuperClass (iKey, pHttpForm->GetValue());
                switch (iErrCode)
                {
                case OK:
                    AddMessage ("The SuperClass was renamed");
                    break;

                case ERROR_SUPERCLASS_DOES_NOT_EXIST:
                    iErrCode = OK;
                    AddMessage ("The SuperClass does not exist");
                    break;

                case ERROR_INVALID_ARGUMENT:
                    iErrCode = OK;
                    AddMessage ("The new SuperClass name was invalid");
                    break;
                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
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

                iErrCode = DeleteGameClass (pHttpForm->GetIntValue(), &bFlag);
                if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST)
                {
                    AddMessage ("The GameClass no longer exists");
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                    if (bFlag) {
                        AddMessage ("The GameClass was deleted");
                    } else {
                        AddMessage ("The GameClass has been marked for deletion");
                    }
                }
                break;
            }

            // Handle game class undeletion
            if (WasButtonPressed (BID_UNDELETEGAMECLASS)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("UndeleteGC")) == NULL) {
                    goto Redirection;
                }

                iErrCode = UndeleteGameClass(pHttpForm->GetIntValue());
                switch (iErrCode) {

                case OK:
                    AddMessage ("The GameClass was undeleted");
                    break;

                case ERROR_GAMECLASS_DOES_NOT_EXIST:
                    iErrCode = OK;
                    AddMessage ("The GameClass no longer exists");
                    break;

                case ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION:
                    iErrCode = OK;
                    AddMessage ("The GameClass was not marked for deletion");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
                break;
            }

            // Handle game class halt
            if (WasButtonPressed (BID_HALTGAMECLASS)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("HaltGC")) == NULL) {
                    goto Redirection;
                }
                iErrCode = HaltGameClass(pHttpForm->GetIntValue());
                switch (iErrCode)
                {
                case OK:
                    AddMessage ("The GameClass was halted");
                    break;

                case ERROR_GAMECLASS_DOES_NOT_EXIST:
                    iErrCode = OK;
                    AddMessage ("The GameClass no longer exists");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
                break;
            }

            // Handle game class unhalting
            if (WasButtonPressed (BID_UNHALTGAMECLASS)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("UnhaltGC")) == NULL) {
                    goto Redirection;
                }

                iErrCode = UnhaltGameClass(pHttpForm->GetIntValue());
                switch (iErrCode)
                {
                case OK:
                    AddMessage ("The GameClass is no longer halted");
                    break;

                case ERROR_GAMECLASS_DOES_NOT_EXIST:
                    iErrCode = OK;
                    AddMessage ("The GameClass no longer exists");
                    break;

                case ERROR_GAMECLASS_NOT_HALTED:
                    iErrCode = OK;
                    AddMessage ("The GameClass was not halted");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
                break;
            }

            // Handle pause all games
            if (WasButtonPressed (BID_PAUSEALLGAMES))
            {
                iErrCode = PauseAllGames();
                RETURN_ON_ERROR(iErrCode);
                AddMessage("All games were paused");
            }

            // Handle unpause all games
            if (WasButtonPressed (BID_UNPAUSEALLGAMES))
            {
                iErrCode = UnpauseAllGames();
                RETURN_ON_ERROR(iErrCode);
                AddMessage("All games were unpaused");
            }

            // Handle reset update times on all games
            if (WasButtonPressed (BID_RESET))
            {
                iErrCode = ResetAllGamesUpdateTime();
                RETURN_ON_ERROR(iErrCode);
                AddMessage("All game update times were reset");
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

            if (WasButtonPressed (BID_CREATENEWGAMECLASS))
            {
                bool bProcessed;
                iErrCode = ProcessCreateGameClassForms(SYSTEM, NO_KEY, &bProcessed);
                RETURN_ON_ERROR(iErrCode);
                if (!bProcessed)
                {
                    iGameAdminPage = 2;
                }
            }

            }
            break;

        case 3:
            {
            // Get game class, game number
            if ((pHttpForm  = m_pHttpRequest->GetForm ("GameClass")) == NULL ||
                (pHttpForm2 = m_pHttpRequest->GetForm ("GameNumber")) == NULL) {
                break;
            }
            iGameClass  = pHttpForm->GetIntValue();
            iGameNumber = pHttpForm2->GetIntValue();

            // View map
            if (WasButtonPressed (BID_VIEWMAP)) {
                m_bRedirectTest = false;
                iGameAdminPage = 4;
                break;
            }

            // Change password
            if (WasButtonPressed (BID_CHANGEPASSWORD)) {

                m_bRedirectTest = false;

                pHttpForm = m_pHttpRequest->GetForm ("NewPassword");
                if (pHttpForm == NULL) {
                    break;
                }
                const char* pszMessage = pHttpForm->GetValue();

                if (!String::IsBlank (pszMessage) && !VerifyPassword(pszMessage))
                {
                    break;
                }

                iErrCode = SetGamePassword(iGameClass, iGameNumber, pszMessage);
                RETURN_ON_ERROR(iErrCode);

                if (String::IsBlank (pszMessage)) {
                    AddMessage ("The game password was removed");
                } else {
                    AddMessage ("The game password was updated");
                }
                iGameAdminPage = 3;
                break;
            }

            // Force update
            if (WasButtonPressed (BID_FORCEUPDATE)) {

                bool bExist;
                iErrCode = DoesGameExist(iGameClass, iGameNumber, &bExist);
                RETURN_ON_ERROR(iErrCode);

                if (bExist)
                {
                    iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	                RETURN_ON_ERROR(iErrCode);

                    iErrCode = ForceUpdate (iGameClass, iGameNumber);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The game was forcibly updated");
                
                    iErrCode = DoesGameExist(iGameClass, iGameNumber, &bExist);
                    RETURN_ON_ERROR(iErrCode);
                }
                
                if (bExist)
                {
                    iGameAdminPage = 3;
                }
                else
                {
                    iGameAdminPage = 1;
                }

                m_bRedirectTest = false;
                break;
            }

            if (WasButtonPressed (BID_RESET))
            {
                iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	            RETURN_ON_ERROR(iErrCode);

                iErrCode = ResetGameUpdateTime (iGameClass, iGameNumber);
                if (iErrCode == ERROR_GAME_PAUSED)
                {
                    iErrCode = OK;
                    AddMessage ("The game was paused and its update time could not be reset");
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The game's update time was reset");
                }

                bool bExist;
                iErrCode = DoesGameExist(iGameClass, iGameNumber, &bExist);
                RETURN_ON_ERROR(iErrCode);
                if (bExist) {
                    iGameAdminPage = 3;
                } else {
                    iGameAdminPage = 1;
                }

                m_bRedirectTest = false;
                break;
            }

            // Delete empire from game
            if (WasButtonPressed (BID_DELETEEMPIRE))
            {
                pHttpForm = m_pHttpRequest->GetForm ("DeleteEmpireKey");
                if (pHttpForm != NULL)
                {
                    int iTargetEmpireKey = pHttpForm->GetIntValue();
                    iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
                    RETURN_ON_ERROR(iErrCode);

                    if (m_iGameState & STARTED)
                    {
                        iErrCode = RemoveEmpireFromGame(iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                        switch (iErrCode)
                        {
                        case OK:
                            AddMessage("The empire was deleted from the game");
                            break;

                        case ERROR_EMPIRE_IS_NOT_IN_GAME:
                            AddMessage("The empire is no longer in this game");
                            iErrCode = OK;
                            break;

                        default:
                            RETURN_ON_ERROR(iErrCode);
                        }
                    }
                    else
                    {
                        iErrCode = QuitEmpireFromGame(iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                        switch (iErrCode)
                        {
                        case OK:
                            AddMessage("The empire was deleted from the game");
                            break;
                        case ERROR_EMPIRE_IS_NOT_IN_GAME:
                            AddMessage("The empire is no longer in this game");
                            iErrCode = OK;
                            break;
            
                        case ERROR_GAME_HAS_STARTED:
                            iErrCode = RemoveEmpireFromGame(iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                            switch (iErrCode)
                            {
                            case OK:
                                AddMessage("The empire was deleted from the game");
                                break;

                            case ERROR_EMPIRE_IS_NOT_IN_GAME:
                                AddMessage("The empire is no longer in this game");
                                iErrCode = OK;
                                break;

                            default:
                               RETURN_ON_ERROR(iErrCode);
                               break;
                            }
                            break;
                        default:
                            RETURN_ON_ERROR(iErrCode);
                            break;
                        }
                    }
                }

                bool bExist;
                iErrCode = DoesGameExist(iGameClass, iGameNumber, &bExist);
                RETURN_ON_ERROR(iErrCode);
                if (bExist)
                {
                    iGameAdminPage = 3;
                }
                else
                {
                    iGameAdminPage = 1;
                }

                m_bRedirectTest = false;
                break;
            }

            // Restore resigned empire to game
            if (WasButtonPressed (BID_RESTOREEMPIRE)) {

                pHttpForm = m_pHttpRequest->GetForm ("RestoreEmpireKey");
                if (pHttpForm != NULL) {

                    int iTargetEmpireKey = pHttpForm->GetIntValue();

                    if (!(m_iGameState & STARTED)) {

                        iErrCode = UnresignEmpire (iGameClass, iGameNumber, iTargetEmpireKey, m_iEmpireKey);
                        RETURN_ON_ERROR(iErrCode);
                        AddMessage ("The empire was restored");
                    }
                }

                bool bExist;
                iErrCode = DoesGameExist (iGameClass, iGameNumber, &bExist);
                RETURN_ON_ERROR(iErrCode);
                if (bExist) {
                    iGameAdminPage = 3;
                } else {
                    iGameAdminPage = 1;
                }

                m_bRedirectTest = false;
                break;
            }

            // Check for search for empires with duplicate IP's
            if (WasButtonPressed (BID_SEARCHIPADDRESSES))
            {
                iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	            RETURN_ON_ERROR(iErrCode);

                iGameAdminPage = 3;
                m_bRedirectTest = false;
                iErrCode = SearchForDuplicateEmpires(iGameClass, iGameNumber, IP_ADDRESS);
                RETURN_ON_ERROR(iErrCode);
                break;
            }

            // Check for search for empires with duplicate IP's
            if (WasButtonPressed (BID_SEARCHSESSIONIDS))
            {
                iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	            RETURN_ON_ERROR(iErrCode);

                iGameAdminPage = 3;
                m_bRedirectTest = false;
                iErrCode = SearchForDuplicateEmpires(iGameClass, iGameNumber, SESSION_ID);
                RETURN_ON_ERROR(iErrCode);
                break;
            }

            // Check for view empire info
            if (WasButtonPressed (BID_VIEWEMPIREINFORMATION))
            {
                iGameAdminPage = 7;
                m_bRedirectTest = false;
                break;
            }

            // Pause game
            if (WasButtonPressed(BID_PAUSEGAME))
            {
                m_bRedirectTest = false;

                // Flush remaining updates
                iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
                RETURN_ON_ERROR(iErrCode);
                
                bool bUpdated;
                iErrCode = CheckGameForUpdates(iGameClass, iGameNumber, &bUpdated);
                RETURN_ON_ERROR(iErrCode);

                // Best effort pause the game
                if (bUpdated)
                {
                    bool bExists;
                    iErrCode = DoesGameExist(iGameClass, iGameNumber, &bExists);
                    RETURN_ON_ERROR(iErrCode);
                    if (!bExists)
                    {
                        AddMessage ("The game no longer exists");
                        iGameAdminPage = 1;
                        break;
                    }
                }

                iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	            RETURN_ON_ERROR(iErrCode);

                iErrCode = PauseGame (iGameClass, iGameNumber, true, true);
                RETURN_ON_ERROR(iErrCode);
                
                AddMessage ("The game is now paused");
                iGameAdminPage = 3;
                break;
            }

            // Unpause game
            if (WasButtonPressed (BID_UNPAUSEGAME))
            {
                iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	            RETURN_ON_ERROR(iErrCode);

                iErrCode = UnpauseGame (iGameClass, iGameNumber, true, true);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The game is no longer paused");
                iGameAdminPage = 3;
                
                m_bRedirectTest = false;
                break;
            }

            // Broadcast message
            if (WasButtonPressed (BID_SENDMESSAGE)) {

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL) {
                    goto Redirection;
                }
                const char* pszMessage = pHttpForm->GetValue();

                iErrCode = CacheGameTablesForBroadcast(iGameClass, iGameNumber);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = BroadcastGameMessage(iGameClass, iGameNumber, pszMessage, m_iEmpireKey, MESSAGE_BROADCAST | MESSAGE_ADMINISTRATOR);
                RETURN_ON_ERROR(iErrCode);
                
                AddMessage ("Your message was broadcast to all empires in the game");

                iGameAdminPage = 3;
                break;
            }

            // Kill game
            if (WasButtonPressed (BID_KILLGAME)) {

                m_bRedirectTest = false;
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
                m_bRedirectTest = false;
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

                m_bRedirectTest = false;
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

                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("DoomMessage")) == NULL) {
                    break;
                }
                const char* pszMessage = pHttpForm->GetValue();

                iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = DeleteGame(iGameClass, iGameNumber, m_iEmpireKey, pszMessage, REASON_NONE);
                RETURN_ON_ERROR(iErrCode);

                AddMessage ("The game was deleted");
                iGameAdminPage = 1;
            }
            break;

        default:
            Assert(false);
            break;
        }
    }
} 

Redirection:
if (m_bRedirectTest)
{
    bool bRedirected;
    PageId pageRedirect;
    iErrCode = RedirectOnSubmit(&pageRedirect, &bRedirected);
    RETURN_ON_ERROR(iErrCode);
    if (bRedirected)
    {
        return Redirect(pageRedirect);
    }
}

iErrCode = OpenSystemPage(false);
RETURN_ON_ERROR(iErrCode);

switch (iGameAdminPage) {

case 0:
    {

    unsigned int* piSuperClassKey = NULL, iNumSuperClasses;
    AutoFreeKeys free_piSuperClassKey(piSuperClassKey);

    Variant* pvSuperClassName = NULL;
    AutoFreeData free_pvSuperClassName(pvSuperClassName);

    iErrCode = GetSuperClassKeys (&piSuperClassKey, &pvSuperClassName, &iNumSuperClasses);
    RETURN_ON_ERROR(iErrCode);

    int *piGameClassKey = NULL, iNumGameClasses;
    Algorithm::AutoDelete<int> auto1 (piGameClassKey, true);

    bool* pbGameClassHalted = NULL, * pbGameClassDeleted = NULL, bFlag;
    Algorithm::AutoDelete<bool> auto2 (pbGameClassHalted, true);
    Algorithm::AutoDelete<bool> auto3 (pbGameClassDeleted, true);

    iErrCode = GetSystemGameClassKeys (&piGameClassKey, &pbGameClassHalted, &pbGameClassDeleted, &iNumGameClasses);
    RETURN_ON_ERROR(iErrCode);
    
    GameConfiguration gcConfig;
    MapConfiguration mcConfig;

    unsigned int iNumActiveGames, iNumOpenGames, iNumClosedGames;
    Variant vMaxNumUpdatesBeforeClose, vDefaultNumUpdatesBeforeClose, vMaxNumPersonalGameClasses, vDelay, vMaxNumT, vMaxNumPersonalGameClassesT;

    iErrCode = GetNumActiveGames (&iNumActiveGames);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNumOpenGames (&iNumOpenGames);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetNumClosedGames (&iNumClosedGames);
    RETURN_ON_ERROR(iErrCode);

    int iSystemMinNumSecsPerUpdate, iSystemMaxNumSecsPerUpdate, iSystemMaxNumEmpires, iSystemMaxNumPlanets,
        iPersonalMinNumSecsPerUpdate, iPersonalMaxNumSecsPerUpdate, iPersonalMaxNumEmpires, 
        iPersonalMaxNumPlanets, iValue, iSystemOptions;

    iErrCode = GetMinNumSecsPerUpdateForSystemGameClass (&iSystemMinNumSecsPerUpdate);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetMaxNumSecsPerUpdateForSystemGameClass (&iSystemMaxNumSecsPerUpdate);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetMaxNumEmpiresForSystemGameClass (&iSystemMaxNumEmpires);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetMaxNumPlanetsForSystemGameClass (&iSystemMaxNumPlanets);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemProperty (SystemData::MaxNumUpdatesBeforeClose, &vMaxNumUpdatesBeforeClose);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::DefaultNumUpdatesBeforeClose, &vDefaultNumUpdatesBeforeClose);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetMinNumSecsPerUpdateForPersonalGameClass (&iPersonalMinNumSecsPerUpdate);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetMaxNumSecsPerUpdateForPersonalGameClass (&iPersonalMaxNumSecsPerUpdate);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetMaxNumEmpiresForPersonalGameClass (&iPersonalMaxNumEmpires);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetMaxNumPlanetsForPersonalGameClass (&iPersonalMaxNumPlanets);
    RETURN_ON_ERROR(iErrCode);

    Variant vMaxResourcesPerPlanet, vMaxResourcesPerPlanetPersonal, vMaxInitialTechLevel, vMaxInitialTechLevelPersonal, vMaxTechDev, vMaxTechDevPersonal, vUnlimitedEmpirePrivilege;

    iErrCode = GetSystemProperty (SystemData::MaxResourcesPerPlanet, &vMaxResourcesPerPlanet);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemProperty (SystemData::MaxResourcesPerPlanetPersonal, &vMaxResourcesPerPlanetPersonal);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemProperty (SystemData::MaxInitialTechLevel, &vMaxInitialTechLevel);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::MaxInitialTechLevelPersonal, &vMaxInitialTechLevelPersonal);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemProperty (SystemData::MaxTechDev, &vMaxTechDev);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::MaxTechDevPersonal, &vMaxTechDevPersonal);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetGameConfiguration (&gcConfig);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetMapConfiguration (&mcConfig);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemProperty (SystemData::AfterWeekendDelay, &vDelay);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemProperty (SystemData::MaxNumPersonalGameClasses, &vMaxNumPersonalGameClasses);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::MaxNumPersonalTournaments, &vMaxNumT);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetSystemProperty (SystemData::MaxNumGameClassesPerPersonalTournament, &vMaxNumPersonalGameClassesT);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemOptions (&iSystemOptions);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetSystemProperty (SystemData::PrivilegeForUnlimitedEmpires, &vUnlimitedEmpirePrivilege);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="GameAdminPage" value="0"><%

    %><h3>Game Management</h3><%

    %><p><table width="75%"><%

    // Active games
    %><tr><td>There <% 
    if (iNumActiveGames == 1) { 
        %>is <strong>1</strong> active game<% 
    } else { 
        %>are <%
        if (iNumActiveGames == 0) {
            %>no<%
        } else {
            %><strong><% Write (iNumActiveGames); %></strong><%
        }
        %> active games<%
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

        %></td></tr><tr><td>Reset update times on all games:</td><td><%

        WriteButton (BID_RESET);

        %></td></tr><%
    }

    %></table><%

    %><p><h3>SuperClasses and GameClasses</h3><%

    %><p><table width="75%"><%

    // SuperClasses
    if (iNumSuperClasses > 0) { 

        %><tr><td>Delete a SuperClass:</td><td><select name="DelSuperClass"><%
        for (i = 0; i < (int)iNumSuperClasses; i ++) { 
            %><option value="<% Write (piSuperClassKey [i]); %>"><% 
            Write (pvSuperClassName[i].GetCharPtr()); %></option><%
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
        for (i = 0; i < (int)iNumSuperClasses; i ++)
        { 
            %><option value="<% Write (piSuperClassKey [i]); %>"><% 
            Write (pvSuperClassName[i].GetCharPtr()); %></option><%
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
    if (iNumGameClasses > 0)
    {
        int iNumHalted = 0, iNumNotHalted = 0, iNumDeleted = 0, iNumNotDeleted = 0;
        for (i = 0; i < iNumGameClasses; i ++)
        {
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

            for (i = 0; i < iNumGameClasses; i ++)
            { 
                if (pbGameClassDeleted[i])
                    continue;
                
                iErrCode = GetGameClassName (piGameClassKey[i], pszGameClassName);
                RETURN_ON_ERROR(iErrCode);
                
                %><option value="<% Write (piGameClassKey[i]); %>"><% 
                Write (pszGameClassName); %></option><%
            }
            %></select></td><td><%
            WriteButton (BID_DELETEGAMECLASS);
            %></td></tr><%
        }

        if (iNumDeleted > 0) {

            %><tr><td>Undelete a GameClass:</td><td><select name="UndeleteGC"><%

            char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

            for (i = 0; i < iNumGameClasses; i ++)
            { 
                if (!pbGameClassDeleted[i])
                    continue;
                
                iErrCode = GetGameClassName(piGameClassKey[i], pszGameClassName);
                RETURN_ON_ERROR(iErrCode);

                %><option value="<% Write (piGameClassKey[i]); %>"><% 
                Write (pszGameClassName); %></option><%
            }
            %></select></td><td><%
            WriteButton (BID_UNDELETEGAMECLASS);
            %></td></tr><%
        }

        if (iNumNotHalted > 0) {

            %><tr><td>Halt a GameClass:</td><td><select name="HaltGC"><%

            char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

            for (i = 0; i < iNumGameClasses; i ++)
            { 
                if (pbGameClassHalted[i])
                    continue;
                
                iErrCode = GetGameClassName(piGameClassKey[i], pszGameClassName);
                RETURN_ON_ERROR(iErrCode);
                    
                %><option value="<% Write (piGameClassKey[i]); %>"><% 
                Write (pszGameClassName); %></option><%
            }
            %></select></td><td><%
            WriteButton (BID_HALTGAMECLASS);
            %></td></tr><%
        }

        if (iNumHalted > 0) {

            %><tr><td>Unhalt a GameClass:</td><td><select name="UnhaltGC"><%

            char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

            for (i = 0; i < iNumGameClasses; i ++)
            {
                if (!pbGameClassHalted[i])
                    continue;
                
                iErrCode = GetGameClassName (piGameClassKey[i], pszGameClassName);
                RETURN_ON_ERROR(iErrCode);
                
                %><option value="<% Write (piGameClassKey[i]); %>"><% 
                Write (pszGameClassName); %></option><%
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
        Write (vMaxResourcesPerPlanet.GetInteger()); %>"></td></tr><%

    %><tr><td>Maximum initial tech level in a System GameClass:</td><%
    %><td><input type="text" name="SMaxInitialTechLevel" size="10" maxlength="20" value="<% 
        Write (vMaxInitialTechLevel.GetFloat()); %>"></td></tr><%

    %><tr><td>Maximum tech development in a System GameClass:</td><%
    %><td><input type="text" name="SMaxTechDev" size="10" maxlength="20" value="<% 
        Write (vMaxTechDev.GetFloat()); %>"></td></tr><%

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

    %><tr><td>Maximum number of personal GameClasses per empire:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewMaxNumPGC" value="<% 
    Write (vMaxNumPersonalGameClasses.GetInteger()); %>"><%
    %><input type="hidden" name="OldMaxNumPGC" value="<%
    Write (vMaxNumPersonalGameClasses.GetInteger()); %>"></td></tr><%

    %><tr><td>Maximum number of personal tournaments per empire:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewMaxNumT" value="<% 
    Write (vMaxNumT.GetInteger()); %>"><%
    %><input type="hidden" name="OldMaxNumT" value="<%
    Write (vMaxNumT.GetInteger()); %>"></td></tr><%

    %><tr><td>Maximum number of GameClasses per personal tournament:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewMaxNumPGCT" value="<% 
    Write (vMaxNumPersonalGameClassesT.GetInteger()); %>"><%
    %><input type="hidden" name="OldMaxNumPGCT" value="<%
    Write (vMaxNumPersonalGameClassesT.GetInteger()); %>"></td></tr><%

    %><tr><td>&nbsp;</td></td><%

    %><tr><td>Maximum number of empires in a personal GameClass:</td><%
    %><td><input type="text" name="PMaxEmps" size="5" maxlength="20" value="<% 
        Write (iPersonalMaxNumEmpires); %>"></td></tr><%

    %><tr><td>Maximum number of planets in a personal GameClass:</td><%
    %><td><input type="text" name="PMaxPlanets" size="5" maxlength="20" value="<% 
        Write (iPersonalMaxNumPlanets); %>"></td></tr><%

    %><tr><td>Maximum average number of resources per attribute per planet in a personal GameClass:</td><%
    %><td><input type="text" name="PMaxResourcesPerPlanet" size="5" maxlength="20" value="<% 
        Write (vMaxResourcesPerPlanetPersonal.GetInteger()); %>"></td></tr><%

    %><tr><td>Maximum initial tech level in a personal GameClass:</td><%
    %><td><input type="text" name="PMaxInitialTechLevel" size="10" maxlength="20" value="<% 
        Write (vMaxInitialTechLevelPersonal.GetFloat()); %>"></td></tr><%

    %><tr><td>Maximum tech development in a personal GameClass:</td><%
    %><td><input type="text" name="PMaxTechDev" size="10" maxlength="20" value="<% 
        Write (vMaxTechDevPersonal.GetFloat()); %>"></td></tr><%

    %><tr><td>Minimum update time in a personal GameClass:</td><%
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

    %><tr><td>Maximum update time in a personal GameClass:</td><%
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

    %><tr><td>Privilege level required to create Personal Games and GameClasses with unlimited empires:</td><%
    %><td><select name="UnlimitedPrivilege"><%

    int iUnlimitedEmpirePrivilege = vUnlimitedEmpirePrivilege.GetInteger();

    for (i = NOVICE; i <= ADMINISTRATOR; i ++) {

        %><option <%

        if (i == iUnlimitedEmpirePrivilege) {
            %>selected <%
        }

        %>value="<%
        Write (i);
        %>"><%
        Write (PRIVILEGE_STRING[i]);
        %></option><%
    }
    %></td></tr><%
    
    
    %><tr><td>Initial state of games:</td><%
    %><td><select name="PauseGames"><%
    
    bFlag = (iSystemOptions & PAUSE_GAMES_BY_DEFAULT) != 0;
    
    %><option value="0" <%
    if (!bFlag) {
        %>selected <%
    }
    %>>Don't pause games by default when they start</option><%
    
    %><option value="1" <%
    if (bFlag) {
        %>selected <%
    }
    %>>Pause games by default when they start</option><%

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

    Seconds sDelay = vDelay.GetInteger();

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
        Write (vMaxNumUpdatesBeforeClose.GetInteger()); %>"></td></tr><%

    %><tr><td>Default number of updates before games close:</td><%
    %><td><input type="text" name="DefaultNumUpdatesBeforeClose" size="5" maxlength="20" value="<% 
        Write (vDefaultNumUpdatesBeforeClose.GetInteger()); %>"></td></tr><%

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
    
    bool fFilterIdle = (iSystemOptions & DEFAULT_RESTRICT_IDLE_EMPIRES) != 0;

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

    %><tr><%
    %><td>Default idle empire blocking:</td><%
    %><td><select name="FilterIdle"><%
    %><option<%

    if (!fFilterIdle) {
        %> selected<%
    }

    %> value="0">Don't block empires who are idle in other games</option><option<%
    
    if (fFilterIdle) {
        %> selected<%
    }

    %> value="1">Block empires who are idle in other games</option><%
    %></select></td><%
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

    %><tr><td>Builder resource multiplier:</td><td><%
    %><input type="text" size="6" maxlength="10" name="BuilderMultiplier"<%
    %> value="<% Write (gcConfig.fBuilderMultiplier); %>"></td></tr><%

    %><tr><td>Builder BR dampening factor:</td><td><%
    %><input type="text" size="6" maxlength="10" name="BuilderDampener"<%
    %> value="<% Write (gcConfig.fBuilderBRDampener); %>"></td></tr><%


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

    }

    break;

AllGames:
case 1:
    {

    Variant** ppvActiveGames = NULL;
    AutoFreeData free_ppvActiveGames(ppvActiveGames);

    unsigned int iNumActiveGames, iNumOpenGames, iNumClosedGames;

    iErrCode = GetActiveGames(&ppvActiveGames, &iNumActiveGames);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetNumOpenGames(&iNumOpenGames);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetNumClosedGames(&iNumClosedGames);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="GameAdminPage" value="1"><%

    iErrCode = WriteActiveGameAdministration((const Variant**)ppvActiveGames, iNumActiveGames, iNumOpenGames, iNumClosedGames, true);
    RETURN_ON_ERROR(iErrCode);

    }
    break;

case 2: 

    %><input type="hidden" name="GameAdminPage" value="2"><p><h3>Create a new GameClass:</h3><%

    iErrCode = WriteCreateGameClassString(SYSTEM, NO_KEY, false);
    RETURN_ON_ERROR(iErrCode);
    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_CREATENEWGAMECLASS);

    break;

case 3:

    %><input type="hidden" name="GameAdminPage" value="3"><% 
    iErrCode = WriteAdministerGame (iGameClass, iGameNumber, true);
    RETURN_ON_ERROR(iErrCode);

    %><p><% WriteButton (BID_CANCEL);

    break;

case 4:
    {

    int iState;

    iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	RETURN_ON_ERROR(iErrCode);

    iErrCode = GetGameState (iGameClass, iGameNumber, &iState);
    RETURN_ON_ERROR(iErrCode);

    if (!(iState & GAME_MAP_GENERATED))
    {
        goto AllGames;
    }

    %><input type="hidden" name="GameAdminPage" value="4"><%
    %><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    iErrCode = RenderMap(iGameClass, iGameNumber, m_iEmpireKey, true, NULL, false);
    RETURN_ON_ERROR(iErrCode);

    %><p><% WriteButton (BID_CANCEL);

    }
    break;

case 6:
    {

    iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	RETURN_ON_ERROR(iErrCode);

    Variant vOptions;

    unsigned int iLivePlanetKey, iDeadPlanetKey;
    int iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel;

    bool bStarted, bFalse;

    Variant* pvPlanetData = NULL;
    AutoFreeData free_pvPlanetData(pvPlanetData);

    GET_GAME_MAP (pszGameMap, iGameClass, iGameNumber);

    iErrCode = HasGameStarted (iGameClass, iGameNumber, &bStarted);
    RETURN_ON_ERROR(iErrCode);

    if (!bStarted)
    {
        AddMessage ("The game hasn't started yet, so it has no map");
        goto AllGames;
    }

    int iLivePlanetAddress, iDeadPlanetAddress;
    iErrCode = GetEmpirePlanetIcons(m_iEmpireKey, &iLivePlanetKey, &iLivePlanetAddress, &iDeadPlanetKey, &iDeadPlanetAddress);
    RETURN_ON_ERROR(iErrCode);

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

    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vOptions);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadRow (pszGameMap, iClickedPlanetKey, &pvPlanetData);
    RETURN_ON_ERROR(iErrCode);

    m_iGameState |= STARTED | GAME_MAP_GENERATED;
    m_iGameClass = iGameClass;
    m_iGameNumber = iGameNumber;

    %><input type="hidden" name="GameAdminPage" value="6"><%
    %><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    %><p><table width="90%"><%

    iErrCode = WriteUpClosePlanetString (NO_KEY, iClickedPlanetKey, 
        0, iLivePlanetKey, iLivePlanetAddress, iDeadPlanetKey, iDeadPlanetAddress, 0, true, iGoodAg, iBadAg, iGoodMin, iBadMin, iGoodFuel, iBadFuel,
        1.0, (vOptions.GetInteger() & INDEPENDENCE) != 0, true, false, pvPlanetData, &bFalse);
    RETURN_ON_ERROR(iErrCode);

    %></table><p><%

    WriteButton (BID_VIEWMAP);

    }
    break;

case 7:
    {
    
    iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
	RETURN_ON_ERROR(iErrCode);

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];
    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="GameAdminPage" value="7"><%
    %><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    %><p>Empire information for <%

    Write (pszGameClassName); %> <% Write (iGameNumber); %>:<p><%

    iErrCode = RenderEmpireInformation (iGameClass, iGameNumber, true);
    RETURN_ON_ERROR(iErrCode);

    WriteButton (BID_CANCEL);

    }
    break;

case 8:

    {

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    iErrCode = GetGameClassName (iGameClass, pszGameClassName);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="GameAdminPage" value="8"><%
    %><input type="hidden" name="GameClass" value="<% Write (iGameClass); %>"><%
    %><input type="hidden" name="GameNumber" value="<% Write (iGameNumber); %>"><%

    %><p><table width="65%"><%
    %><tr><td align="center"><%
    %>Are you sure you want to kill <strong><% Write (pszGameClassName); %> <% Write (iGameNumber); 
    %></strong>?<p>If so, please send a message to its participants:<%
    %></td></tr></table><%
    %><p><textarea name="DoomMessage" rows="5" cols="45" wrap="virtual"></textarea><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_KILLGAME);

    }
    break;

default:

    Assert(false);
    break;
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>