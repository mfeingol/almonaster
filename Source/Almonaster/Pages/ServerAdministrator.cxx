<%
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

// Make sure that the unprivileged don't abuse this:
if (m_iPrivilege < ADMINISTRATOR)
{
    AddMessage ("You are not authorized to view this page");
    return Redirect (LOGIN);
}

int i, iServerAdminPage = 0, iInfoThemeKey = NO_KEY;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (WasButtonPressed (BID_CANCEL)) {
        m_bRedirectTest = false;
    } else {

        if ((pHttpForm = m_pHttpRequest->GetForm ("ServerAdminPage")) == NULL) {
            goto Redirection;
        }
        int iServerAdminPageSubmit = pHttpForm->GetIntValue();

        switch (iServerAdminPageSubmit) {

        case 0:
            {
            int iNewValue, iOldValue;
            float fNewValue, fOldValue;
            bool bNewValue, bOldValue;
            const char* pszNewValue, * pszOldValue;

            int iSystemOptions;
            iErrCode = GetSystemOptions (&iSystemOptions);
            RETURN_ON_ERROR(iErrCode);

            // Create alien
            if (WasButtonPressed (BID_CREATEALIENICON))
            {
                if ((pHttpForm = m_pHttpRequest->GetForm ("NewAuthorName")) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();

                if (pszNewValue == NULL || *pszNewValue == '\0' || strlen (pszNewValue) > MAX_ALIEN_AUTHOR_NAME_LENGTH) {
                    AddMessage ("You must submit an valid alien author name");
                } else {

                    // Icon upload hack
                    if ((pHttpForm = m_pHttpRequest->GetForm ("NewAlienFile")) == NULL) {
                        goto Redirection;
                    }

                    const char* pszFileName = pHttpForm->GetValue();
                    if (pszFileName == NULL)
                    {
                        AddMessage ("Please upload an alien icon GIF");
                    }
                    else
                    {
                        bool bGoodGIF;
                        iErrCode = VerifyGIF(pszFileName, &bGoodGIF);
                        RETURN_ON_ERROR(iErrCode);
                        if (bGoodGIF)
                        {
                            // The gif was OK, so insert the key and copy it to its destination
                            int iNewAddress;
                            unsigned int iNewKey;
                            iErrCode = CreateAlienIcon(pszNewValue, &iNewAddress, &iNewKey);
                            RETURN_ON_ERROR(iErrCode);

                            if (!CopyNewAlien(pszFileName, iNewAddress))
                            {
                                // Compensate
                                iErrCode = DeleteAlienIcon(iNewKey);
                                RETURN_ON_ERROR(iErrCode);
                                AddMessage("The alien icon file could not be copied to its destination");
                            }
                            else
                            {
                                AddMessage("Alien icon ");
                                AppendMessage(iNewAddress);
                                AppendMessage(" was successfully created");
                            }
                            break;
                        }
                    }
                }
            }

            // Delete alien
            if (WasButtonPressed (BID_DELETEALIENICON)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("OldAlienIcon")) == NULL)
                {
                    goto Redirection;
                }
                iNewValue = pHttpForm->GetIntValue();
                if (iNewValue < FIRST_ADDRESS || pHttpForm->GetValue() == NULL)
                {
                    AddMessage("Please enter an alien icon to delete");
                }
                else switch (DeleteAlienIconByAddress(iNewValue))
                {
                case OK:
                    if (DeleteAlienFile(iNewValue))
                    {
                        AddMessage("Alien icon ");
                        AppendMessage(iNewValue);
                        AppendMessage(" was deleted successfully");
                    }
                    else
                    {
                        AddMessage("Alien icon ");
                        AppendMessage(iNewValue);
                        AppendMessage(" was deleted successfully, but the file could not be removed. Please remove it manually.");
                    }
                    break;

                case ERROR_LAST_ALIEN_ICON:
                    AddMessage("You cannot delete the last alien icon");
                    break;

                case ERROR_ALIEN_ICON_DOES_NOT_EXIST:
                    AddMessage("The alien icon does not exist");
                    break;

                case ERROR_DEFAULT_ALIEN_ICON:
                    AddMessage("The system default alien icon cannot be deleted");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
            }

            // Server name
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewServerName")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldServerName")) == NULL) {
                goto Redirection;
            }
            pszOldValue = pHttpForm->GetValue();
            if (String::StrCmp (pszNewValue, pszOldValue) != 0)
            {
                if (VerifyCategoryName ("Server", pszNewValue, MAX_SERVER_NAME_LENGTH, true))
                {
                    iErrCode = SetSystemProperty (SystemData::ServerName, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The server name was updated");
                }
            }

            // Server name
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewAdminEmail")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldAdminEmail")) == NULL) {
                goto Redirection;
            }
            pszOldValue = pHttpForm->GetValue();
            if (String::StrCmp (pszNewValue, pszOldValue) != 0) {

                if (strlen (pszNewValue) > MAX_EMAIL_LENGTH) {
                    AddMessage ("The administrator e-mail address was too long");
                } else {
                    iErrCode = SetSystemProperty(SystemData::AdminEmail, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("The administrator e-mail address was updated");
                }
            }

            // Score-based privilege changes
            if ((pHttpForm = m_pHttpRequest->GetForm ("PrivElev")) == NULL) {
                goto Redirection;
            }

            bNewValue = pHttpForm->GetIntValue() == 0;
            bOldValue = (iSystemOptions & DISABLE_PRIVILEGE_SCORE_ELEVATION) != 0;

            if (bNewValue != bOldValue) {
                iErrCode = SetSystemOption(DISABLE_PRIVILEGE_SCORE_ELEVATION, bNewValue);
                RETURN_ON_ERROR(iErrCode);
                if (bNewValue) {
                    AddMessage ("The Almonaster scoring system will no longer cause empire privilege changes");
                } else {
                    AddMessage ("The Almonaster scoring system will now cause empire privilege changes");
                }
            }

            // Adept level
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewAdept")) == NULL) {
                goto Redirection;
            }
            fNewValue = pHttpForm->GetFloatValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldAdept")) == NULL) {
                goto Redirection;
            }
            fOldValue = pHttpForm->GetFloatValue();
            if (fNewValue != fOldValue) {
                iErrCode = SetScoreForPrivilege(ADEPT, fNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The score needed for adepthood was updated");
            }

            // MaxNumSystemMessages
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewMaxNumSystemMessages")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxNumSystemMessages")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iNewValue = (iNewValue / 10) * 10;
                iErrCode = SetSystemProperty(SystemData::MaxNumSystemMessages, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The max number of saved system messages was updated");
            }

            // MaxNumGameMessages
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewMaxNumGameMessages")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxNumGameMessages")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iNewValue = (iNewValue / 10) * 10;
                iErrCode = SetSystemProperty(SystemData::MaxNumGameMessages, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The max number of saved game messages was updated");
            }

            // DefaultMaxNumSystemMessages
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewDefaultMaxNumSystemMessages")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldDefaultMaxNumSystemMessages")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iNewValue = (iNewValue / 10) * 10;
                iErrCode = SetSystemProperty(SystemData::DefaultMaxNumSystemMessages, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default max number of saved system messages was updated");
            }

            // DefaultMaxNumGameMessages
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewDefaultMaxNumGameMessages")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldDefaultMaxNumGameMessages")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iNewValue = (iNewValue / 10) * 10;
                iErrCode = SetSystemProperty(SystemData::DefaultMaxNumGameMessages, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default max number of saved game messages was updated");
            }

            // MaxSizeIcons
            if ((pHttpForm = m_pHttpRequest->GetForm ("MaxSizeIcons")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldMaxSizeIcons")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iErrCode = SetSystemProperty(SystemData::MaxIconSize, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The max icon size was updated");
            }

            // Logins enabled
            const char* pszValue;
            Variant vReason;

            bool bFlag;

            if ((pHttpForm = m_pHttpRequest->GetForm ("Logins")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (iSystemOptions & LOGINS_ENABLED) != 0;

            if (bFlag != (iNewValue != 0))
            {
                bFlag = !bFlag;

                iErrCode = SetSystemOption (LOGINS_ENABLED, bFlag);
                RETURN_ON_ERROR(iErrCode);
                if (bFlag) {
                    AddMessage ("Empire logins are now enabled");
                } else {
                    AddMessage ("Empire logins are now disabled");
                }
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("LoginsReason")) == NULL) {
                goto Redirection;
            }
            pszValue = pHttpForm->GetValue();

            iErrCode = GetSystemProperty(SystemData::LoginsDisabledReason, &vReason);
            RETURN_ON_ERROR(iErrCode);
            
            if (String::StrCmp(pszValue, vReason.GetCharPtr()) != 0)
            {
                if (strlen(pszValue) > MAX_REASON_LENGTH)
                {
                    AddMessage ("The logins disabled reason was too long");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::LoginsDisabledReason, pszValue);
                    RETURN_ON_ERROR(iErrCode);
                }
            }

            // New empire creation
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewEmps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (iSystemOptions & NEW_EMPIRES_ENABLED) != 0;

            if (bFlag != (iNewValue != 0)) {
                bFlag = !bFlag;
                iErrCode = SetSystemOption (NEW_EMPIRES_ENABLED, bFlag);
                RETURN_ON_ERROR(iErrCode);
                if (bFlag) {
                    AddMessage ("New empire creation is now enabled");
                } else {
                    AddMessage ("New empire creation is now disabled");
                }
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewEmpsReason")) == NULL) {
                goto Redirection;
            }
            pszValue = pHttpForm->GetValue();

            iErrCode = GetSystemProperty(SystemData::NewEmpiresDisabledReason, &vReason);
            RETURN_ON_ERROR(iErrCode);
            
            if (String::StrCmp(pszValue, vReason.GetCharPtr()) != 0)
            {
                if (strlen(pszValue) > MAX_REASON_LENGTH)
                {
                    AddMessage ("The new empire creation disabled reason was too long");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::NewEmpiresDisabledReason, pszValue);
                    RETURN_ON_ERROR(iErrCode);
                }
            }

            // New empire creation
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewGames")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (iSystemOptions & NEW_GAMES_ENABLED) != 0;

            if (bFlag != (iNewValue != 0)) {
                bFlag = !bFlag;
                iErrCode = SetSystemOption (NEW_GAMES_ENABLED, bFlag);
                RETURN_ON_ERROR(iErrCode);

                if (bFlag) {
                    AddMessage ("New game creation is now enabled");
                } else {
                    AddMessage ("New game creation is now disabled");
                }
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewGamesReason")) == NULL) {
                goto Redirection;
            }
            pszValue = pHttpForm->GetValue();
            
            iErrCode = GetSystemProperty(SystemData::NewGamesDisabledReason, &vReason);
            RETURN_ON_ERROR(iErrCode);
            
            if (String::StrCmp(pszValue, vReason.GetCharPtr()) != 0)
            {
                if (strlen(pszValue) > MAX_REASON_LENGTH)
                {
                    AddMessage ("The new game creation disabled reason was too long");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::NewGamesDisabledReason, pszValue);
                    RETURN_ON_ERROR(iErrCode);
                }
            }

            // AccessEnabled
            if ((pHttpForm = m_pHttpRequest->GetForm ("Access")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (iSystemOptions & ACCESS_ENABLED) != 0;

            if (bFlag != (iNewValue != 0)) {
                bFlag = !bFlag;
                iErrCode = SetSystemOption (ACCESS_ENABLED, bFlag);
                RETURN_ON_ERROR(iErrCode);
                if (bFlag) {
                    AddMessage ("Server access for non-administrators is now enabled");
                } else {
                    AddMessage ("Server access for non-administrators is now disabled");
                }
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("AccessReason")) == NULL) {
                goto Redirection;
            }
            pszValue = pHttpForm->GetValue();
            
            iErrCode = GetSystemProperty(SystemData::AccessDisabledReason, &vReason);
            RETURN_ON_ERROR(iErrCode);
            
            if (String::StrCmp(pszValue, vReason.GetCharPtr()) != 0)
            {
                if (strlen(pszValue) > MAX_REASON_LENGTH)
                {
                    AddMessage ("The access disabled reason was too long");
                }
                else
                {
                    iErrCode = SetSystemProperty(SystemData::AccessDisabledReason, pszValue);
                    RETURN_ON_ERROR(iErrCode);
                }
            }

            // Number of nukes listed
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewNumNukes")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldNumNukes")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iErrCode = SetSystemProperty(SystemData::NumNukesListedInNukeHistories, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The number of nukes listed in nuke histories was updated");
            }

            // Number of system nukes listed
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewSystemNukes")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldSystemNukes")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iErrCode = SetSystemProperty(SystemData::NumNukesListedInSystemNukeList, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The number of nukes listed in the latest nukes screen was updated");
            }

            // Number of games listed
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewGamesListed")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldGamesListed")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iErrCode = SetSystemProperty(SystemData::NumGamesInLatestGameList, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The number of games listed in the latest games screen was updated");
            }

            // Updates down
            if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesDown")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            if ((pHttpForm = m_pHttpRequest->GetForm ("OldUpdatesDown")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();
            if (iNewValue != iOldValue) {
                iErrCode = SetSystemProperty(SystemData::NumUpdatesDownBeforeGameIsKilled, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The number of updates down before a game will be killed was updated");
            }

            // SecondsForLongtermStatus
            if ((pHttpForm = m_pHttpRequest->GetForm ("SecondsForLongtermStatusHrs")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue() * 3600;
            if ((pHttpForm = m_pHttpRequest->GetForm ("SecondsForLongtermStatusMin")) == NULL) {
                goto Redirection;
            }
            iNewValue += pHttpForm->GetIntValue() * 60;
            if ((pHttpForm = m_pHttpRequest->GetForm ("SecondsForLongtermStatusSec")) == NULL) {
                goto Redirection;
            }
            iNewValue += pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldSecondsForLongtermStatus")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iNewValue != iOldValue) {
                iErrCode = SetSystemProperty(SystemData::SecondsForLongtermStatus, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The update period for a game to be considered a longterm was updated");
            }

            // BridierTimeBombScanFrequency
            if ((pHttpForm = m_pHttpRequest->GetForm ("BridierScanHrs")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue() * 3600;
            if ((pHttpForm = m_pHttpRequest->GetForm ("BridierScanMin")) == NULL) {
                goto Redirection;
            }
            iNewValue += pHttpForm->GetIntValue() * 60;
            if ((pHttpForm = m_pHttpRequest->GetForm ("BridierScanSec")) == NULL) {
                goto Redirection;
            }
            iNewValue += pHttpForm->GetIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("OldBridierScan")) == NULL) {
                goto Redirection;
            }
            iOldValue = pHttpForm->GetIntValue();

            if (iNewValue != iOldValue) {
                iErrCode = SetBridierTimeBombScanFrequency (iNewValue);
                AddMessage ("The Bridier idle index decrease scan frequency was updated");
                RETURN_ON_ERROR(iErrCode);
            }

            // Default ship names
            Variant vOldName;
            int iNumNames = 0;

            char pszForm[128];

            ENUMERATE_SHIP_TYPES(i) {

                sprintf(pszForm, "ShipName%i", i);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();
                if (pszNewValue == NULL) {
                    pszNewValue = "";
                }

                if (!ShipOrFleetNameFilter (pszNewValue))
                {
                    AddMessage ("The new ship name for ");
                    AppendMessage (SHIP_TYPE_STRING[i]);
                    AppendMessage (" is illegal");
                }
                else
                {
                    iErrCode = GetDefaultShipName (i, &vOldName);
                    RETURN_ON_ERROR(iErrCode);
                    if (strcmp (pszNewValue, vOldName.GetCharPtr()) != 0)
                    {
                        iErrCode = SetDefaultShipName(i, pszNewValue);
                        RETURN_ON_ERROR(iErrCode);
                        iNumNames ++;
                    }
                }
            }

            switch (iNumNames)
            {
            case 0:
                break;
            case 1:
                AddMessage ("The default ship name was updated");
                break;
            default:
                AddMessage ("The default ship names were updated");
                break;
            }

            // Update text files
            iErrCode = TryUpdateIntroUpper();
            if (iErrCode == OK ) {
                AddMessage ("The text above the login form was updated");
            } else if (iErrCode != WARNING) {
                RETURN_ON_ERROR(iErrCode);
            }

            iErrCode = TryUpdateIntroLower();
            if (iErrCode == OK) {
                AddMessage ("The text below the login form was updated");
            } else if (iErrCode != WARNING) {
                RETURN_ON_ERROR(iErrCode);
            }

            iErrCode = TryUpdateServerNews();
            if (iErrCode == OK) {
                AddMessage ("The server news text was updated");
            }
            else if (iErrCode != WARNING) {
                RETURN_ON_ERROR(iErrCode);
            }

            iErrCode = TryUpdateContributors();
            if (iErrCode == OK) {
                AddMessage ("The contributors text was updated");
            } else if (iErrCode != WARNING) {
                RETURN_ON_ERROR(iErrCode);
            }

            // Create empire?
            if (WasButtonPressed (BID_CREATEEMPIRE)) {
                m_bRedirectTest = false;
                iServerAdminPage = 7;
                break;
            }

            // Purge DB?
            if (WasButtonPressed (BID_PURGE)) {
                m_bRedirectTest = false;
                iServerAdminPage = 5;
                break;
            }

            // Shutdown Server?
            if (WasButtonPressed (BID_SHUTDOWNSERVER)) {

                m_bRedirectTest = false;
                ShutdownServer();
                break;
            }

            // Restart Server?
            if (WasButtonPressed (BID_RESTARTSERVER)) {

                m_bRedirectTest = false;
                RestartServer();
                break;
            }

            // Restart PageSource?
            if (WasButtonPressed (BID_RESTARTALMONASTER)) {

                m_bRedirectTest = false;
                RestartAlmonaster();
                break;
            }

            // Redirect to default alien choice?
            if (m_pHttpRequest->GetFormBeginsWith ("DefaultAlien") != NULL) {
                m_bRedirectTest = false;
                iServerAdminPage = 1;
                break;
            }

            // Redirect to default alien choice?
            if (m_pHttpRequest->GetFormBeginsWith ("SysMsgAlien") != NULL) {
                m_bRedirectTest = false;
                iServerAdminPage = 6;
                break;
            }

            // Redirect to UI choice?
            if (m_pHttpRequest->GetForm ("ChooseUI.x") != NULL ||
                m_pHttpRequest->GetForm ("ChooseUI") != NULL ||
                m_pHttpRequest->GetForm ("Planet1.0.x") != NULL ||
                m_pHttpRequest->GetForm ("Planet2.0.x") != NULL) {
                iServerAdminPage = 2;
                m_bRedirectTest = false;
                break;
            }

            }
            break;

        case 1:
            {

            const char* pszStart;
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Alien")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "Alien%d.x", &i) == 1) {

                Variant vDefAlien;
                iErrCode = GetSystemProperty(SystemData::DefaultAlienKey, &vDefAlien);
                RETURN_ON_ERROR(iErrCode);

                if (i != vDefAlien.GetInteger())
                {
                    iErrCode = SetSystemProperty(SystemData::DefaultAlienKey, i);
                    RETURN_ON_ERROR(iErrCode);

                    int iNewAddress;
                    iErrCode = GetAlienIconAddress(i, &iNewAddress);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = SetSystemProperty(SystemData::DefaultAlienAddress, iNewAddress);
                    RETURN_ON_ERROR(iErrCode);

                    AddMessage ("The default alien icon was updated");

                } else {
                    AddMessage ("The default alien icon was not updated");
                }
                m_bRedirectTest = false;
                break;
            }

            }
            break;

        case 2:
            {

            unsigned int iBackground, iLivePlanet, iDeadPlanet, iButtons, iSeparator, iHorz, iVert, iColor;
            int iButtonAddress, iBackgroundAddress;

            iErrCode = GetDefaultUIKeys (
                &iBackground,
                &iBackgroundAddress,
                &iLivePlanet,
                &iDeadPlanet,
                &iButtons,
                &iButtonAddress,
                &iSeparator,
                &iHorz,
                &iVert,
                &iColor
                );
            RETURN_ON_ERROR(iErrCode);

            // Background
            unsigned int iKey;
            if ((pHttpForm = m_pHttpRequest->GetForm ("Background")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iBackground) {
                iErrCode = SetSystemProperty (SystemData::DefaultUIBackground, iKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default background key was updated");
            }

            // Live planet
            if ((pHttpForm = m_pHttpRequest->GetForm ("LivePlanet")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iLivePlanet) {
                iErrCode = SetSystemProperty (SystemData::DefaultUILivePlanet, iKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default live planet key was updated");
            }

            // Dead planet
            if ((pHttpForm = m_pHttpRequest->GetForm ("DeadPlanet")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iDeadPlanet) {
                iErrCode = SetSystemProperty (SystemData::DefaultUIDeadPlanet, iKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default dead planet key was updated");
            }

            // Buttons
            if ((pHttpForm = m_pHttpRequest->GetForm ("Button")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iButtons) {
                iErrCode = SetSystemProperty (SystemData::DefaultUIButtons, iKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default button key was updated");
            }

            // Separator
            if ((pHttpForm = m_pHttpRequest->GetForm ("Separator")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iSeparator) {
                iErrCode = SetSystemProperty (SystemData::DefaultUISeparator, iKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default separator key was updated");
            }

            // Horz
            if ((pHttpForm = m_pHttpRequest->GetForm ("Horz")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iHorz) {
                iErrCode = SetSystemProperty (SystemData::DefaultUIHorz, iKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default horizontal link bar was updated");
            }

            // Vert
            if ((pHttpForm = m_pHttpRequest->GetForm ("Vert")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iVert) {
                iErrCode = SetSystemProperty (SystemData::DefaultUIVert, iKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default vertical link bar was updated");
            }

            // Color key
            if ((pHttpForm = m_pHttpRequest->GetForm ("Color")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iColor) {
                iErrCode = SetSystemProperty (SystemData::DefaultUIColor, iKey);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default color scheme was updated");
            }

            const char* pszStart;
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ThemeInfo")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "ThemeInfo%d", &iInfoThemeKey) == 1) {

                iServerAdminPage = 3;
                m_bRedirectTest = false;
                break;
            }

            }

            break;

        case 3:
            break;

        case 4:
            break;

        case 5:
            {

            if (WasButtonPressed (BID_PURGE)) {

                int iCriteria = 0;

                if (m_pHttpRequest->GetForm ("XPlayed") != NULL) {
                    iCriteria |= NEVER_PLAYED_A_GAME;
                }

                if (m_pHttpRequest->GetForm ("XWins") != NULL) {
                    iCriteria |= NEVER_WON_A_GAME;
                }

                if (m_pHttpRequest->GetForm ("XLogins") != NULL) {
                    iCriteria |= ONLY_ONE_LOGIN;
                }

                if (m_pHttpRequest->GetForm ("XScore") != NULL) {
                    iCriteria |= CLASSIC_SCORE_IS_ZERO_OR_LESS;
                }

                if (m_pHttpRequest->GetForm ("XLastLogin1Month") != NULL) {
                    iCriteria |= LAST_LOGGED_IN_1_MONTH_AGO;
                }

                if (m_pHttpRequest->GetForm ("XLastLogin3Months") != NULL) {
                    iCriteria |= LAST_LOGGED_IN_3_MONTHS_AGO;
                }

                if (m_pHttpRequest->GetForm ("XNotOnTopLists") != NULL) {
                    iCriteria |= NOT_ON_TOP_LISTS;
                }

                if (iCriteria == 0) {
                    AddMessage ("You submitted no purging criteria");
                    iServerAdminPage = 5;
                    m_bRedirectTest = false;
                } else {

                    if (m_pHttpRequest->GetForm ("TestPurge") != NULL) {
                        iCriteria |= TEST_PURGE_ONLY;
                    }

                    iErrCode = PurgeDatabase (m_iEmpireKey, iCriteria);
                    RETURN_ON_ERROR(iErrCode);

                    if (iCriteria & TEST_PURGE_ONLY) {
                        AddMessage ("The database is being test purged");
                    } else {
                        AddMessage ("The database is being purged");
                    }
                }
            }

            }
            break;

        case 6:
            {

            const char* pszStart;
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Alien")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "Alien%d.x", &i) == 1) {

                Variant vSysMsgAlien;
                iErrCode = GetSystemProperty (SystemData::SystemMessagesAlienKey, &vSysMsgAlien);
                RETURN_ON_ERROR(iErrCode);

                if (i != vSysMsgAlien.GetInteger()) {

                    switch (SetSystemProperty (SystemData::SystemMessagesAlienKey, i)) {
                    case OK:
                        AddMessage ("The alien icon for system messages was updated");
                        break;
                    default:
                        RETURN_ON_ERROR(iErrCode);
                        break;
                    }

                } else {
                    AddMessage ("The alien icon for system messages was not updated");
                }
                m_bRedirectTest = false;
                break;
            }

            }
            break;

        case 7:

            if (WasButtonPressed (BID_CREATEEMPIRE)) {

                m_bRedirectTest = false;

                int iPrivilege;
                unsigned int iNewEmpireKey;
                const char* pszNewName, * pszNewPass;

                char pszStandardizedName [MAX_EMPIRE_NAME_LENGTH + 1];

                pHttpForm = m_pHttpRequest->GetForm ("NEmpireName");
                if (pHttpForm == NULL) {
                    goto Redirection;
                }

                pszNewName = pHttpForm->GetValue();
                if (pszNewName == NULL) {
                    AddMessage ("You must provide an empire name");
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                pHttpForm = m_pHttpRequest->GetForm ("NPassword");
                if (pHttpForm == NULL) {
                    goto Redirection;
                }

                pszNewPass = pHttpForm->GetValue();
                if (pszNewPass == NULL) {
                    AddMessage ("You must provide a password");
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                pHttpForm = m_pHttpRequest->GetForm ("NPassword2");
                if (pHttpForm == NULL) {
                    goto Redirection;
                }

                if (String::StrCmp (pHttpForm->GetValue(), pszNewPass) != 0) {
                    AddMessage ("You must confirm the password correctly");
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                pHttpForm = m_pHttpRequest->GetForm ("NPrivilege");
                if (pHttpForm == NULL) {
                    goto Redirection;
                }
                iPrivilege = pHttpForm->GetIntValue();

                if (!IS_VALID_PRIVILEGE (iPrivilege)) {
                    AddMessage ("Invalid privilege");
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                if (!VerifyEmpireName(pszNewName) || !StandardizeEmpireName(pszNewName, pszStandardizedName))
                {
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                if (!VerifyPassword (pszNewPass)) {
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                // Test empire existence
                iErrCode = LookupEmpireByName(pszStandardizedName, &iNewEmpireKey, NULL, NULL);
                RETURN_ON_ERROR(iErrCode);
                if (iNewEmpireKey != NO_KEY)
                {
                    AddMessage ("That empire already exists");
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                iErrCode = CreateEmpire (
                    pszStandardizedName,
                    pszNewPass,
                    iPrivilege,
                    NO_KEY,
                    true,
                    &iNewEmpireKey
                    );

                switch (iErrCode) {

                case OK:
                    AddMessage ("The new empire was created");
                    break;

                case ERROR_RESERVED_EMPIRE_NAME:
                    AddMessage ("The given empire name is reserved");
                    break;

                case ERROR_EMPIRE_ALREADY_EXISTS:
                    AddMessage ("The given empire name is already in use");
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                    break;
                }
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

iErrCode = OpenSystemPage(iServerAdminPage == 0);
RETURN_ON_ERROR(iErrCode);

// Individual page stuff starts here
switch (iServerAdminPage) {

case 0:
    {

    Seconds sSecondsForLongtermStatus, sBridierScan;
    int iNumHrs, iNumMin, j, iSystemOptions;

    String strFilter;

    Variant* pvServerData = NULL;
    AutoFreeData free_pvServerData(pvServerData);

    iErrCode = t_pCache->ReadRow(SYSTEM_DATA, NO_KEY, &pvServerData);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="ServerAdminPage" value="0"><%

    %><h3>Server Management</h3><%

    %><p><table width="90%"><%

    %><tr><td>Server name:</td><%
    %><td><input type="text" size="25" maxlength="<% Write (MAX_SERVER_NAME_LENGTH); %>" name="NewServerName"<% 
    %> value="<% Write (pvServerData[SystemData::iServerName].GetCharPtr()); 
    %>"><input type="hidden" name="OldServerName" value="<% 

    Write (pvServerData[SystemData::iServerName].GetCharPtr()); %>"></td></tr><%


    %><tr><td>Administrator e-mail address:</td><%
    %><td><input type="text" size="40" maxlength="<% Write (MAX_EMAIL_LENGTH); %>" name="NewAdminEmail"<% 
    %> value="<% Write (pvServerData[SystemData::iAdminEmail].GetCharPtr()); 
    %>"><input type="hidden" name="OldAdminEmail" value="<% 

    Write (pvServerData[SystemData::iAdminEmail].GetCharPtr()); %>"></td></tr><%

    %><tr><td>Default alien icon:</td><td><%

    Variant* pvAlienData = NULL;
    AutoFreeData free_pvAlienData(pvAlienData);
    iErrCode = GetAlienData(pvServerData[SystemData::iDefaultAlienKey], &pvAlienData);
    RETURN_ON_ERROR(iErrCode);

    WriteAlienButtonString(pvServerData[SystemData::iDefaultAlienKey], pvAlienData[SystemAlienIcons::iAddress], true, "DefaultAlien", pvAlienData[SystemAlienIcons::iAuthorName]);
    
    %></td></tr><%

    %><tr><td>Default UI elements:</td><td><table><tr><%
    if (pvServerData[SystemData::iDefaultUIBackground].GetInteger() == NULL_THEME) { 
        %><td width="75" height="75" bgcolor="#000000">&nbsp;</td><% 
    } else { 

        int iBackgroundAddress;
        iErrCode = GetThemeAddress(pvServerData[SystemData::iDefaultUIBackground], &iBackgroundAddress);
        RETURN_ON_ERROR(iErrCode);

        %><td><input type="image" border="0" width="75" height="75" src="<%
        WriteBackgroundImageSrc(pvServerData[SystemData::iDefaultUIBackground], iBackgroundAddress);
        %>" name="ChooseUI"></td><% 
    }
    %><td><%

    int iLivePlanetAddress;
    iErrCode = GetThemeAddress(pvServerData[SystemData::iDefaultUILivePlanet], &iLivePlanetAddress);
    RETURN_ON_ERROR(iErrCode);

    GetLivePlanetButtonString(pvServerData[SystemData::iDefaultUILivePlanet], iLivePlanetAddress, 1, 0, NULL, NULL, &strFilter);
    Write (strFilter);

    %></td><td><%

    int iDeadPlanetAddress;
    iErrCode = GetThemeAddress(pvServerData[SystemData::iDefaultUIDeadPlanet], &iDeadPlanetAddress);
    RETURN_ON_ERROR(iErrCode);

    GetDeadPlanetButtonString(pvServerData[SystemData::iDefaultUIDeadPlanet], iDeadPlanetAddress, 2, 0, NULL, NULL, &strFilter);
    Write (strFilter);

    %></td><td><% 

    int iButtonAddress;
    iErrCode = GetThemeAddress(pvServerData[SystemData::iDefaultUIButtons], &iButtonAddress);
    RETURN_ON_ERROR(iErrCode);

    WriteButtonString(pvServerData[SystemData::iDefaultUIButtons], iButtonAddress, "Login", "Login", "ChooseUI");
    %></td><%

    if (pvServerData[SystemData::iDefaultUISeparator].GetInteger() == NULL_THEME) {
        %><td width="150"><%
        Write (DEFAULT_SEPARATOR_STRING);
    } else {

        int iSeparatorAddress;
        iErrCode = GetThemeAddress(pvServerData[SystemData::iDefaultUISeparator], &iSeparatorAddress);
        RETURN_ON_ERROR(iErrCode);

        %><td><input type="image" name="ChooseUI" border="0" width="150" src="<% 
        WriteSeparatorSrc (pvServerData[SystemData::iDefaultUISeparator], iSeparatorAddress);
        %>"><% 
    }

    int iHorzAddress, iVertAddress;
    iErrCode = GetThemeAddress(pvServerData[SystemData::iDefaultUIHorz], &iHorzAddress);
    RETURN_ON_ERROR(iErrCode);
    iErrCode = GetThemeAddress(pvServerData[SystemData::iDefaultUIVert], &iVertAddress);
    RETURN_ON_ERROR(iErrCode);

    %></td><td><input type="image" name="ChooseUI" width="21" height="3" border="0" src="<%
    WriteHorzSrc(pvServerData[SystemData::iDefaultUIHorz], iHorzAddress);
    %>"></td><td><input type="image" name="ChooseUI" width="3" height="21" border="0" src="<%
    WriteVertSrc(pvServerData[SystemData::iDefaultUIVert], iVertAddress);
    %>"></td></tr></table></td></tr><%


    %><tr><td>Create a new empire:</td><td><%
    WriteButton (BID_CREATEEMPIRE);
    %></td></tr><%


    %><tr><td>The Almonaster scoring system:</td><td><select name="PrivElev"><%

    iSystemOptions = pvServerData[SystemData::iOptions].GetInteger();

    %><option <%
    if (!(iSystemOptions & DISABLE_PRIVILEGE_SCORE_ELEVATION)) {
        %>selected <%
    }
    %>value="1">Causes empire privilege levels to change</option><%

    %><option <%
    if (iSystemOptions & DISABLE_PRIVILEGE_SCORE_ELEVATION) {
        %>selected <%
    }
    %>value="0">Does not cause empire privilege levels to change</option><%

    %></select><%
    %></td></tr><%


    %><tr><td>Almonaster score needed to be an Apprentice:</td><%
    %><td><input type="text" size="7" maxlength="7" name="NewApprentice" value="<% 
    Write (pvServerData[SystemData::iApprenticeScore]); 
    %>"><input type="hidden" name="OldApprentice" value="<% Write (pvServerData[SystemData::iApprenticeScore]); 
    %>"></td></tr><%

    %><tr><td>Almonaster score needed to be an Adept:</td><%
    %><td><input type="text" size="7" maxlength="7" name="NewAdept" value="<% 
    Write (pvServerData[SystemData::iAdeptScore]); 
    %>"><input type="hidden" name="OldAdept" value="<% Write (pvServerData[SystemData::iAdeptScore]); 
    %>"></td></tr><%

    %><tr><td>Alien icon for system messages:</td><td><%

    Variant* pvMessageAlienData = NULL;
    AutoFreeData free_pvMessageAlienData(pvMessageAlienData);
    iErrCode = GetAlienData(pvServerData[SystemData::iSystemMessagesAlienKey], &pvMessageAlienData);
    RETURN_ON_ERROR(iErrCode);

    WriteAlienButtonString(pvServerData[SystemData::iSystemMessagesAlienKey], pvMessageAlienData[SystemAlienIcons::iAddress], true, 
                           "SysMsgAlien",pvMessageAlienData[SystemAlienIcons::iAuthorName]);

    %></td></tr><%

    %><tr><td>Maximum saved system messages:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewMaxNumSystemMessages" value="<% 
    Write (pvServerData[SystemData::iMaxNumSystemMessages]); %>"><input type="hidden" name="OldMaxNumSystemMessages" value="<% 
    Write (pvServerData[SystemData::iMaxNumSystemMessages]); %>"></td></tr><tr><td>Maximum saved game messages:</td><%

    %><td><input type="text" size="6" maxlength="6" name="NewMaxNumGameMessages" value="<% 
    Write (pvServerData[SystemData::iMaxNumGameMessages]); %>"><input type="hidden" name="OldMaxNumGameMessages" value="<% 
    Write (pvServerData[SystemData::iMaxNumGameMessages]); %>"></td></tr><tr><td>Default maximum saved system messages:</td><%

    %><td><select name="NewDefaultMaxNumSystemMessages"><% 

    for (i = 0; i <= pvServerData[SystemData::iMaxNumSystemMessages].GetInteger(); i += 10) {
        %><option<%
        if (pvServerData[SystemData::iDefaultMaxNumSystemMessages] == i) {
            %> selected<% 
        }
        %> value="<% Write (i); %>"><% Write (i); %></option><% 
    }

    %></select><input type="hidden" name="OldDefaultMaxNumSystemMessages" value="<% 
    Write (pvServerData[SystemData::iDefaultMaxNumSystemMessages].GetInteger()); %>"><%
    %></td></tr><%
    
    %><tr><td>Default maximum saved game messages:</td><%

    %><td><select name="NewDefaultMaxNumGameMessages"><% 
    for (i = 0; i <= pvServerData[SystemData::iMaxNumGameMessages].GetInteger(); i += 10) {
        %> <option<% if (pvServerData[SystemData::iDefaultMaxNumGameMessages].GetInteger() == i) { %> selected<% }
        %> value="<% Write (i); %>"><% Write (i); %></option><% 
    }

    %></select><input type="hidden" name="OldDefaultMaxNumGameMessages" value="<% 
    Write (pvServerData[SystemData::iDefaultMaxNumGameMessages]); %>"></td></tr><%

    %><tr><td>Maximum size of uploaded icons:</td><%
    %><td><input type="text" size="8" maxlength="8" name="MaxSizeIcons" value="<% 
    Write (pvServerData[SystemData::iMaxIconSize].GetInteger()); %>"> bytes<input type="hidden" name="OldMaxSizeIcons" value="<% 
    Write (pvServerData[SystemData::iMaxIconSize].GetInteger()); %>"><%
    %></td></tr><%

    %><tr><td>Number of nukes listed in empire nuke histories:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewNumNukes" value="<% 
    Write (pvServerData[SystemData::iNumNukesListedInNukeHistories].GetInteger()); %>"><%
    %><input type="hidden" name="OldNumNukes" value="<%
    Write (pvServerData[SystemData::iNumNukesListedInNukeHistories].GetInteger()); %>"></td></tr><%


    %><tr><td>Number of nukes listed in the latest nukes screen:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewSystemNukes" value="<% 
    Write (pvServerData[SystemData::iNumNukesListedInSystemNukeList].GetInteger()); %>"><%
    %><input type="hidden" name="OldSystemNukes" value="<%
    Write (pvServerData[SystemData::iNumNukesListedInSystemNukeList].GetInteger()); %>"></td></tr><%


    %><tr><td>Number of games listed in the latest games screen:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewGamesListed" value="<% 
    Write (pvServerData[SystemData::iNumGamesInLatestGameList].GetInteger()); %>"><%
    %><input type="hidden" name="OldGamesListed" value="<%
    Write (pvServerData[SystemData::iNumGamesInLatestGameList].GetInteger()); %>"></td></tr><%


    %><tr><td>Number of updates the server can be down before games are killed (except longterms):</td><td><%
    %><input type="text" size="4" maxlength="10" name="UpdatesDown"<%
    %> value="<% Write (pvServerData[SystemData::iNumUpdatesDownBeforeGameIsKilled].GetInteger()); %>"><%
    %><input type="hidden" name="OldUpdatesDown" value="<%
    Write (pvServerData[SystemData::iNumUpdatesDownBeforeGameIsKilled].GetInteger()); %>"></td></tr><%

    sSecondsForLongtermStatus = pvServerData[SystemData::iSecondsForLongtermStatus].GetInteger();

    %><tr><td>Update period required to be considered a longterm:</td><td><%
    %><input type="hidden" name="OldSecondsForLongtermStatus" value="<% Write (sSecondsForLongtermStatus); %>"><%
    %><input type="text" size="4" maxlength="20" name="SecondsForLongtermStatusHrs" value="<%
    iNumHrs = sSecondsForLongtermStatus / 3600;
    sSecondsForLongtermStatus -= iNumHrs * 3600;
    Write (iNumHrs);
    %>"> hrs, <%
    %><input type="text" size="4" maxlength="20" name="SecondsForLongtermStatusMin" value="<%
    iNumMin = sSecondsForLongtermStatus / 60;
    sSecondsForLongtermStatus -= iNumMin * 60;
    Write (iNumMin);
    %>"> min, <%
    %><input type="text" size="4" maxlength="20" name="SecondsForLongtermStatusSec" value="<%
    Write (sSecondsForLongtermStatus);
    %>"> secs<%
    %></td></tr><%

    sBridierScan = pvServerData[SystemData::iBridierTimeBombScanFrequency].GetInteger();

    %><tr><td>Bridier idle index decrease scan frequency:</td><td><%
    %><input type="hidden" name="OldBridierScan" value="<% Write (sBridierScan); %>"><%
    %><input type="text" size="4" maxlength="20" name="BridierScanHrs" value="<%
    iNumHrs = sBridierScan / 3600;
    sBridierScan -= iNumHrs * 3600;
    Write (iNumHrs);
    %>"> hrs, <%
    %><input type="text" size="4" maxlength="20" name="BridierScanMin" value="<%
    iNumMin = sBridierScan / 60;
    sBridierScan -= iNumMin * 60;
    Write (iNumMin);
    %>"> min, <%
    %><input type="text" size="4" maxlength="20" name="BridierScanSec" value="<%
    Write (sBridierScan);
    %>"> secs<%
    %></td></tr><%

    %><tr><td>Empire logins are:</td><td><select name="Logins"><%

    if (iSystemOptions & LOGINS_ENABLED) {
        %><option selected value="1">Enabled</option><%
        %><option value="0">Disabled</option><%
    } else {
        %><option value="1">Enabled</option><%
        %><option selected value="0">Disabled</option><%
    }

    %></select> Disabled reason: <input type="text" size="40" <%
    %>maxlength="<% Write (MAX_REASON_LENGTH); %>" name="LoginsReason" <% 
    %>value="<% Write (pvServerData[SystemData::iLoginsDisabledReason].GetCharPtr()); %>"></td></tr><%

    %><tr><td>New empire creation is:</td><td><select name="NewEmps"><%

    if (iSystemOptions & NEW_EMPIRES_ENABLED) {
        %><option selected value="1">Enabled</option><%
        %><option value="0">Disabled</option><%
    } else {
        %><option value="1">Enabled</option><%
        %><option selected value="0">Disabled</option><%
    }

    %></select> Disabled reason: <input type="text" size="40" <%
    %>maxlength="<% Write (MAX_REASON_LENGTH); %>" name="NewEmpsReason" <% 
    %>value="<% Write (pvServerData[SystemData::iNewEmpiresDisabledReason].GetCharPtr()); %>"></td></tr><%

    %><tr><td>New game creation is:</td><td><select name="NewGames"><%

    if (iSystemOptions & NEW_GAMES_ENABLED) {
        %><option selected value="1">Enabled</option><%
        %><option value="0">Disabled</option><%
    } else {
        %><option value="1">Enabled</option><%
        %><option selected value="0">Disabled</option><%
    }

    %></select> Disabled reason: <input type="text" size="40" <%
    %>maxlength="<% Write (MAX_REASON_LENGTH); %>" name="NewGamesReason" <% 
    %>value="<% Write (pvServerData[SystemData::iNewGamesDisabledReason].GetCharPtr()); %>"></td></tr><%

    %><tr><td>Access to the server is:</td><td><select name="Access"><%

    if (iSystemOptions & ACCESS_ENABLED) {
        %><option selected value="1">Enabled</option><%
        %><option value="0">Disabled</option><%
    } else {
        %><option value="1">Enabled</option><%
        %><option selected value="0">Disabled</option><%
    }

    %></select> Disabled reason: <input type="text" size="40" <%
    %>maxlength="<% Write (MAX_REASON_LENGTH); %>" name="AccessReason" <% 
    %>value="<% Write (pvServerData[SystemData::iAccessDisabledReason].GetCharPtr()); %>"></td></tr><%

    %><tr><td>Shut down the server:</td><td><% 
    WriteButton (BID_SHUTDOWNSERVER);
    %></td></tr><%

    %><tr><td>Restart the server:</td><td><% 
    WriteButton (BID_RESTARTSERVER);
    %></td></tr><%

    %><tr><td>Restart the Almonaster PageSource:</td><td><% 
    WriteButton (BID_RESTARTALMONASTER);
    %></td></tr><%

    %><tr><td>Run a custom purge of the empire database:</td><%
    %><td><% WriteButton (BID_PURGE);
    %></td></tr><%

    %><tr><td>Create a new alien icon:</td><%
    %><td><table><tr><td><%
    %>Author's name:</td><td><input type="text" size="20" <%
    %>maxlength="<% Write (MAX_ALIEN_AUTHOR_NAME_LENGTH); %>" name="NewAuthorName"></td></tr><%

    %><tr><td>File:</td><td><input type="file" name="NewAlienFile" size="20"></td></tr><%
    %><tr><td><% 
    WriteButton (BID_CREATEALIENICON);
    %></table></tr><tr><td>Delete an alien icon:</td><td><table><%

    %><tr><td>Icon id:</td><td><input type="text" size="6" maxlength="6" name="OldAlienIcon"></td><td><% 
    WriteButton (BID_DELETEALIENICON);
    %></table></tr><%

    %><tr><td>Default ship names:</td><td><table width="60%"><%

    for (i = FIRST_SHIP; i < NUM_SHIP_TYPES / 2; i ++) {

        %><tr><%

        HTMLFilter(pvServerData[SYSTEM_DATA_SHIP_NAME_COLUMN_INDEX[i]].GetCharPtr(), &strFilter, 0, false);

        %><td><% Write (SHIP_TYPE_STRING[i]); %>:</td><%
        %><td><%
        %><input type="text" size="12" maxlength="<% Write (MAX_SHIP_NAME_LENGTH); %>" <%
        %>name="ShipName<% Write (i); %>" <%
        %>value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength()); %>"><%
        %></td><%

        j = i + NUM_SHIP_TYPES / 2;

        HTMLFilter(pvServerData[SYSTEM_DATA_SHIP_NAME_COLUMN_INDEX[j]].GetCharPtr(), &strFilter, 0, false);

        %><td><% Write (SHIP_TYPE_STRING[j]); %>:</td><%
        %><td><input type="text" size="12" maxlength="<% Write (MAX_SHIP_NAME_LENGTH); %>" <%
        %>name="ShipName<% Write (j); %>" <%
        %>value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength()); %>"><%
        %></td><%

        %></tr><%
    }

    %></table></td></tr><%

    %><tr><td>Edit text above login form:</td><td><%
    WriteIntroUpper (true);
    %></td></tr><%

    %><tr><td>Edit text below login form:</td><td><%
    WriteIntroLower (true);
    %></td></tr><%

    %><tr><td>Edit server news:</td><td><%
    WriteServerNewsFile (true);
    %></td></tr><%

    %><tr><td>Edit contributor list:</td><td><%
    WriteContributorsFile (true);
    %></td></tr><%

    %></table><p><%

    WriteButton(BID_CANCEL);

    }

    break;

case 1:
    {

    Variant vAlienKey;
    iErrCode = GetSystemProperty(SystemData::DefaultAlienKey, &vAlienKey);
    RETURN_ON_ERROR(iErrCode);

    Variant vAlienAddress;
    iErrCode = GetSystemProperty(SystemData::DefaultAlienAddress, &vAlienAddress);
    RETURN_ON_ERROR(iErrCode);

    unsigned int iNumAliens, * piAlienKey = NULL;
    AutoFreeKeys free_piAlienKey(piAlienKey);

    Variant** ppvAlienData = NULL;
    AutoFreeData free_ppvAlienData(ppvAlienData);

    iErrCode = GetAliens(&ppvAlienData, &piAlienKey, &iNumAliens);
    RETURN_ON_ERROR(iErrCode);
    %><input type="hidden" name="ServerAdminPage" value="1"><%

    %><p><%

    iErrCode = WriteIcon(vAlienKey, vAlienAddress, NO_KEY, NO_KEY, "The current default alien icon", NULL, false);
    RETURN_ON_ERROR(iErrCode);

    %><p>Choose a new default alien icon:<p><table width="75%"><tr><td><%

    for (i = 0; i < (int)iNumAliens; i ++)
    {
        WriteAlienButtonString(
            piAlienKey[i],
            ppvAlienData[i][SystemAlienIcons::iAddress],
            vAlienKey.GetInteger() == (int)piAlienKey[i],
            "Alien",
            ppvAlienData[i][SystemAlienIcons::iAuthorName].GetCharPtr()
            );
            %> <%
    }

    %></td></tr></table><%
    %><p><% WriteButton (BID_CANCEL);
    }

    break;

case 2:
    {

    int iB, iL, iD, iS, iT, iH, iV, iC;

    Variant* pvData = NULL;
    AutoFreeData free_pvData(pvData);

    iErrCode = t_pCache->ReadRow(SYSTEM_DATA, &pvData);
    RETURN_ON_ERROR(iErrCode);

    iB = pvData[SystemData::iDefaultUIBackground].GetInteger();
    iL = pvData[SystemData::iDefaultUILivePlanet].GetInteger();
    iD = pvData[SystemData::iDefaultUIDeadPlanet].GetInteger();
    iS = pvData[SystemData::iDefaultUISeparator].GetInteger();
    iT = pvData[SystemData::iDefaultUIButtons].GetInteger();
    iH = pvData[SystemData::iDefaultUIHorz].GetInteger();
    iV = pvData[SystemData::iDefaultUIVert].GetInteger();
    iC = pvData[SystemData::iDefaultUIColor].GetInteger();

    %><input type="hidden" name="ServerAdminPage" value="2"><p>Choose the server's default UI elements:<p><%

    iErrCode = RenderThemeInfo(iB, iL, iD, iS, iT, iH, iV, iC);
    RETURN_ON_ERROR(iErrCode);

    %></table><p><%

    WriteButton (BID_CANCEL);

    }
    break;

case 3:
    {

    %><input type="hidden" name="ServerAdminPage" value="3"><%

    iErrCode = DisplayThemeData (iInfoThemeKey);
    RETURN_ON_ERROR(iErrCode);

    }
    break;

case 4:
    break;

case 5:

    {

    unsigned int iNumEmpires;
    iErrCode = GetNumEmpiresOnServer (&iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="ServerAdminPage" value="5"><%

    %><p>There <%
    if (iNumEmpires == 1) {
        %>is <strong>1</strong> registered empire<%
    } else {
        %>are <strong><% Write (iNumEmpires); %></strong> registered empires<%
    }
    %> on the server<%

    %><p>Select the characteristics of the empires you wish to purge:<%

    %><p><table width="35%"><%
    %><tr><td><input type="checkbox" name="XPlayed"><%
    %> Never played a game</td></tr><%

    %><tr><td><dd>and</td></tr><%

    %><tr><td><input type="checkbox" name="XWins"><%
    %> Never won a game</td></tr><%

    %><tr><td><dd>and</td></tr><%

    %><tr><td><input type="checkbox" name="XLogins"><%
    %> Have logged in at most once</td></tr><%

    %><tr><td><dd>and</td></tr><%

    %><tr><td><input type="checkbox" name="XScore"><%
    %> Have classic scores less than or equal to 0</td></tr><%

    %><tr><td><dd>and</td></tr><%

    %><tr><td><input type="checkbox" name="XLastLogin1Month"><%
    %> Last logged in more than 1 month ago</td></tr><%

     %><tr><td><dd>and</td></tr><%

    %><tr><td><input type="checkbox" name="XLastLogin3Months"><%
    %> Last logged in more than 3 months ago</td></tr><%

    %><tr><td><dd>and</td></tr><%

    %><tr><td><input type="checkbox" name="XNotOnTopLists"><%
    %> Not on a Top List</td></tr><%

    %></table><p><%

    %><input type="checkbox" name="TestPurge" checked="on"><%
    %> Only a test<p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_PURGE);

    }
    break;

case 6:
    {

    Variant vAlienKey;
    iErrCode = GetSystemProperty(SystemData::SystemMessagesAlienKey, &vAlienKey);
    RETURN_ON_ERROR(iErrCode);

    Variant vAlienAddress;
    iErrCode = GetSystemProperty(SystemData::SystemMessagesAlienAddress, &vAlienAddress);
    RETURN_ON_ERROR(iErrCode);

    unsigned int iNumAliens;
    Variant** ppvAlienData = NULL;
    AutoFreeData free_ppvAlienData(ppvAlienData);

    unsigned int* piAlienKey;
    AutoFreeKeys free_piAlienKey(piAlienKey);

    iErrCode = GetAliens(&ppvAlienData, &piAlienKey, &iNumAliens);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="ServerAdminPage" value="6"><%

    %><p><%

    iErrCode = WriteIcon(vAlienKey, vAlienAddress, NO_KEY, NO_KEY, "The current alien icon for system messages", NULL, false);
    RETURN_ON_ERROR(iErrCode);

    %><p>Choose a new alien icon for system messages:<p><table width="75%"><tr><td><%

    for (i = 0; i < (int)iNumAliens; i ++)
    {
        WriteAlienButtonString (
            piAlienKey[i],
            ppvAlienData[i][SystemAlienIcons::iAddress],
            vAlienKey.GetInteger() == (int)piAlienKey[i],
            "Alien",
            ppvAlienData[i][SystemAlienIcons::iAuthorName].GetCharPtr()
            );
            %> <%
    }

    %></td></tr></table><%
    %><p><% WriteButton (BID_CANCEL);
    }

    break;

case 7:
    {

    %><input type="hidden" name="ServerAdminPage" value="7"><%

    %><p>Create a new empire:<%
    %><p><table width="60%"><%

    %><tr><%
    %><td><strong>Empire name</strong>:</td><%
    %><td><input type="text" size="20" maxlength="<% Write (MAX_EMPIRE_NAME_LENGTH); %>" name="NEmpireName"></td><%
    %></tr><%

    %><tr><%
    %><td><strong>Password</strong>:</td><%
    %><td><input type="password" name="NPassword" size="20" maxlength="<% Write(MAX_EMPIRE_PASSWORD_LENGTH); %>"></td><%
    %></tr><%

    %><tr><%
    %><td><strong>Verify password</strong>:</td><%
    %><td><input type="password" name="NPassword2" size="20" maxlength="<% Write(MAX_EMPIRE_PASSWORD_LENGTH); %>"></td><%
    %></tr><%

    %><tr><%
    %><td><strong>Privilege level</strong>:</td><%
    %><td><select name="NPrivilege"><%

        %><option selected value="<% m_pHttpResponse->WriteText (NOVICE); %>"><%
        m_pHttpResponse->WriteText (PRIVILEGE_STRING [NOVICE]);
        %></option><%

        %><option value="<% m_pHttpResponse->WriteText (APPRENTICE); %>"><%
        m_pHttpResponse->WriteText (PRIVILEGE_STRING [APPRENTICE]);
        %></option><%

        %><option value="<% m_pHttpResponse->WriteText (ADEPT); %>"><%
        m_pHttpResponse->WriteText (PRIVILEGE_STRING [ADEPT]);
        %></option><%

        %><option value="<% m_pHttpResponse->WriteText (ADMINISTRATOR); %>"><%
        m_pHttpResponse->WriteText (PRIVILEGE_STRING [ADMINISTRATOR]);
        %></option><%
    
    %></select><%
    %></td><%
    %></tr><%

    %></table><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_CREATEEMPIRE);

    }
    break;

default:
    Assert(false);
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>