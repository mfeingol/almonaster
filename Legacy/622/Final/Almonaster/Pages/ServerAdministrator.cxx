<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
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

INITIALIZE_EMPIRE

IHttpForm* pHttpForm;

// Make sure that the unprivileged don't abuse this:
if (m_iPrivilege < ADMINISTRATOR) {
    AddMessage ("You are not authorized to view this page");
    return Redirect (LOGIN);
}

int i, iErrCode, iServerAdminPage = 0, iInfoThemeKey = NO_KEY;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (WasButtonPressed (BID_CANCEL)) {
        bRedirectTest = false;
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
            Check (g_pGameEngine->GetSystemOptions (&iSystemOptions));

            // Create alien
            if (WasButtonPressed (BID_CREATEALIENICON)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("NewAlienKey")) == NULL) {
                    goto Redirection;
                }
                iNewValue = pHttpForm->GetIntValue();
                if (iNewValue < 1) {
                    AddMessage ("The key must be an integer greater than zero");
                } else {

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
                        if (pszFileName == NULL) {
                            AddMessage ("You didn't upload a file");
                        } else {

                            if (VerifyGIF (pszFileName)) {

                                // The gif was OK, so insert the key and copy it to its destination
                                switch (g_pGameEngine->CreateAlienIcon (iNewValue, pszNewValue)) {

                                case OK:
                                    if (CopyNewAlien (pszFileName, iNewValue) != OK) {
                                        AddMessage ("The file was uploaded, but could not be copied");
                                    } else {
                                        AddMessage ("The alien icon was created successfully");
                                    }
                                    break;

                                case ERROR_ALIEN_ICON_ALREADY_EXISTS:
                                    AddMessage ("The alien icon key already exists");
                                    break;

                                default:
                                    AddMessage ("An unknown error occurred creating the icon");
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            // Delete alien
            if (WasButtonPressed (BID_DELETEALIENICON)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("OldAlienKey")) == NULL) {
                    goto Redirection;
                }
                iNewValue = pHttpForm->GetIntValue();
                if (iNewValue < 1) {
                    AddMessage ("The key must be an integer greater than zero");
                } else {

                    switch (g_pGameEngine->DeleteAlienIcon (iNewValue)) {
                    case OK:
                        DeleteAlien (iNewValue);
                        AddMessage ("The alien icon was deleted successfully");
                        break;

                    case ERROR_LAST_ALIEN_ICON:
                        AddMessage ("You cannot delete the last alien icon");
                        break;

                    case ERROR_ALIEN_ICON_DOES_NOT_EXIST:
                        AddMessage ("The given alien icon key does not exist");
                        break;

                    case ERROR_DEFAULT_ALIEN_ICON:
                        AddMessage ("The default alien icon cannot be deleted");
                        break;

                    default:
                        AddMessage ("An unknown error occurred deleting the icon");
                        break;
                    }
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
            if (String::StrCmp (pszNewValue, pszOldValue) != 0) {

                if (VerifyCategoryName ("Server", pszNewValue, MAX_SERVER_NAME_LENGTH, true) == OK) {
                    if (g_pGameEngine->SetSystemProperty (SystemData::ServerName, pszNewValue) == OK) {
                        AddMessage ("The server name was updated");
                    } else {
                        AddMessage ("The server name could not be updated");
                    }
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

                    if (g_pGameEngine->SetSystemProperty (SystemData::AdminEmail, pszNewValue) == OK) {
                        AddMessage ("The administrator e-mail address was updated");
                    } else {
                        AddMessage ("The administrator e-mail address could not be updated");
                    }
                }
            }

            // Score-based privilege changes
            if ((pHttpForm = m_pHttpRequest->GetForm ("PrivElev")) == NULL) {
                goto Redirection;
            }

            bNewValue = pHttpForm->GetIntValue() == 0;
            bOldValue = (iSystemOptions & DISABLE_PRIVILEGE_SCORE_ELEVATION) != 0;

            if (bNewValue != bOldValue) {
                if (g_pGameEngine->SetSystemOption (DISABLE_PRIVILEGE_SCORE_ELEVATION, bNewValue) == OK) {

                    if (bNewValue) {
                        AddMessage ("The Almonaster scoring system will no longer cause empire privilege changes");
                    } else {
                        AddMessage ("The Almonaster scoring system will now cause empire privilege changes");
                    }

                } else {

                    AddMessage ("An error occurred setting the privilege change option: ");
                    AppendMessage (iErrCode);
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
                if (g_pGameEngine->SetScoreForPrivilege (ADEPT, fNewValue) == OK) {
                    AddMessage ("The score needed for adepthood was updated");
                } else {
                    AddMessage ("The score needed for adepthood could not be updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::MaxNumSystemMessages, iNewValue) == OK) {
                    AddMessage ("The max number of saved system messages was updated");
                } else {
                    AddMessage ("The max number of saved system messages could not be updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::MaxNumGameMessages, iNewValue) == OK) {
                    AddMessage ("The max number of saved game messages was updated");
                } else {
                    AddMessage ("The max number of saved game messages could not be updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::DefaultMaxNumSystemMessages, iNewValue) == OK) {
                    AddMessage ("The default max number of saved system messages was updated");
                } else {
                    AddMessage ("The default max number of saved system messages could not be updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::DefaultMaxNumGameMessages, iNewValue) == OK) {
                    AddMessage ("The default max number of saved game messages was updated");
                } else {
                    AddMessage ("The default max number of saved game messages could not be updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::MaxIconSize, iNewValue) == OK) {
                    AddMessage ("The max icon size was updated");
                } else {
                    AddMessage ("The max icon size could not be updated");
                }
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

            if (bFlag != (iNewValue != 0)) {
                bFlag = !bFlag;

                Check (g_pGameEngine->SetSystemOption (LOGINS_ENABLED, bFlag));

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

            if (g_pGameEngine->GetSystemProperty (SystemData::LoginsDisabledReason, &vReason) == OK &&
                String::StrCmp (pszValue, vReason.GetCharPtr()) != 0 &&
                g_pGameEngine->SetSystemProperty (SystemData::LoginsDisabledReason, pszValue) != OK) {
                AddMessage ("The logins disabled reason was too long");
            }

            // New empire creation
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewEmps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (iSystemOptions & NEW_EMPIRES_ENABLED) != 0;

            if (bFlag != (iNewValue != 0)) {
                bFlag = !bFlag;
                Check (g_pGameEngine->SetSystemOption (NEW_EMPIRES_ENABLED, bFlag));

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
            if (g_pGameEngine->GetSystemProperty (SystemData::NewEmpiresDisabledReason, &vReason) == OK &&
                String::StrCmp (pszValue, vReason.GetCharPtr()) != 0 &&
                g_pGameEngine->SetSystemProperty (SystemData::NewEmpiresDisabledReason, pszValue) != OK) {
                AddMessage ("The new empire creation disabled reason was too long");
            }

            // New empire creation
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewGames")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (iSystemOptions & NEW_GAMES_ENABLED) != 0;

            if (bFlag != (iNewValue != 0)) {
                bFlag = !bFlag;
                Check (g_pGameEngine->SetSystemOption (NEW_GAMES_ENABLED, bFlag));

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
            if (g_pGameEngine->GetSystemProperty (SystemData::NewGamesDisabledReason, &vReason) == OK &&
                String::StrCmp (pszValue, vReason.GetCharPtr()) != 0 &&
                g_pGameEngine->SetSystemProperty (SystemData::NewGamesDisabledReason, pszValue) != OK) {
                AddMessage ("The new game creation disabled reason was too long");
            }

            // AccessEnabled
            if ((pHttpForm = m_pHttpRequest->GetForm ("Access")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bFlag = (iSystemOptions & ACCESS_ENABLED) != 0;

            if (bFlag != (iNewValue != 0)) {
                bFlag = !bFlag;
                Check (g_pGameEngine->SetSystemOption (ACCESS_ENABLED, bFlag));

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
            if (g_pGameEngine->GetSystemProperty (SystemData::AccessDisabledReason, &vReason) == OK &&
                String::StrCmp (pszValue, vReason.GetCharPtr()) != 0 &&
                g_pGameEngine->SetSystemProperty (SystemData::AccessDisabledReason, pszValue) != OK) {
                AddMessage ("The access disabled reason was too long");
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
                if (g_pGameEngine->SetSystemProperty (SystemData::NumNukesListedInNukeHistories, iNewValue) == OK) {
                    AddMessage ("The number of nukes listed in nuke histories was updated");
                } else {
                    AddMessage ("The number of nukes listed in nuke histories could not be updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::NumNukesListedInSystemNukeList, iNewValue) == OK) {
                    AddMessage ("The number of nukes listed in the latest nukes screen was updated");
                } else {
                    AddMessage ("The number of nukes listed in the latest nukes screen could not be updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::NumGamesInLatestGameList, iNewValue) == OK) {
                    AddMessage ("The number of games listed in the latest games screen was updated");
                } else {
                    AddMessage ("The number of games listed in the latest games screen could not be updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::NumUpdatesDownBeforeGameIsKilled, iNewValue) == OK) {
                    AddMessage ("The number of updates down before a game will be killed was updated");
                } else {
                    AddMessage ("The number of updates down before a game will be killed was updated");
                }
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
                if (g_pGameEngine->SetSystemProperty (SystemData::SecondsForLongtermStatus, iNewValue) == OK) {
                    AddMessage ("The update period for a game to be considered a longterm was updated");
                } else {
                    AddMessage ("The update period for a game to be considered a longterm could not be updated");
                }
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
                if (g_pGameEngine->SetBridierTimeBombScanFrequency (iNewValue) == OK) {
                    AddMessage ("The Bridier idle index decrease scan frequency was updated");
                } else {
                    AddMessage ("The Bridier idle index decrease scan frequency could not be updated");
                }
            }

            // Default ship names
            Variant vOldName;
            int iNumNames = 0;

            char pszForm[128];

            ENUMERATE_SHIP_TYPES(i) {

                sprintf (pszForm, "ShipName%i", i);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();
                if (pszNewValue == NULL) {
                    pszNewValue = "";
                }

                if (!ShipOrFleetNameFilter (pszNewValue)) {
                    AddMessage ("The new ship name for ");
                    AppendMessage (SHIP_TYPE_STRING[i]);
                    AppendMessage (" is illegal");
                } else {

                    Check (g_pGameEngine->GetDefaultShipName (i, &vOldName));

                    if (strcmp (pszNewValue, vOldName.GetCharPtr()) != 0) {
                        if (g_pGameEngine->SetDefaultShipName (i, pszNewValue) == OK) {
                            iNumNames ++;
                        }
                    }
                }
            }

            switch (iNumNames) {
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
                AddMessage ("The text above the login form could not be updated");
            }

            iErrCode = TryUpdateIntroLower();
            if (iErrCode == OK) {
                AddMessage ("The text below the login form was updated");
            } else if (iErrCode != WARNING) {
                AddMessage ("The text below the login form could not be updated");
            }

            iErrCode = TryUpdateServerNews();
            if (iErrCode == OK) {
                AddMessage ("The server news text was updated");
            }
            else if (iErrCode != WARNING) {
                AddMessage ("The server news text could not be updated: ");
                AppendMessage (iErrCode);
            }

            iErrCode = TryUpdateContributors();
            if (iErrCode == OK) {
                AddMessage ("The contributors text was updated");
            } else if (iErrCode != WARNING) {
                AddMessage ("The contributors text could not be updated");
            }

            // Create empire?
            if (WasButtonPressed (BID_CREATEEMPIRE)) {
                bRedirectTest = false;
                iServerAdminPage = 7;
                break;
            }

            // Flush tables?
            if (WasButtonPressed (BID_FLUSH)) {

                bRedirectTest = false;

                iErrCode = g_pGameEngine->FlushDatabase (m_iEmpireKey);

                if (iErrCode == OK) {
                    AddMessage ("All database tables will be flushed to disk");
                } else {
                    AddMessage ("An error occurred preparing to flush the database: ");
                    AppendMessage (iErrCode);
                }

                break;
            }

            // Purge DB?
            if (WasButtonPressed (BID_PURGE)) {
                bRedirectTest = false;
                iServerAdminPage = 5;
                break;
            }

            // Backup DB?
            if (WasButtonPressed (BID_BACKUP)) {

                bRedirectTest = false;

                iErrCode = g_pGameEngine->BackupDatabase (m_iEmpireKey);

                if (iErrCode == OK) {
                    AddMessage ("The database will be backed up");
                } else {
                    AddMessage ("An unexpected error occurred: ");
                    AppendMessage (iErrCode);
                }
                break;
            }

            if (WasButtonPressed (BID_REBUILD)) {
                iErrCode = g_pGameEngine->RebuildTopLists();

                if (iErrCode == OK) {
                    AddMessage ("The Top Lists were rebuilt");
                } else {
                    AddMessage ("An unexpected error occurred: ");
                    AppendMessage (iErrCode);
                }
            }

            // Shutdown Server?
            if (WasButtonPressed (BID_SHUTDOWNSERVER)) {

                bRedirectTest = false;
                ShutdownServer();

                break;
            }

            // Restart Server?
            if (WasButtonPressed (BID_RESTARTSERVER)) {

                bRedirectTest = false;
                RestartServer();

                break;
            }

            // Restart PageSource?
            if (WasButtonPressed (BID_RESTARTALMONASTER)) {

                bRedirectTest = false;
                RestartAlmonaster();

                break;
            }

            // Restore backup?
            if (WasButtonPressed (BID_RESTOREBACKUP)) {

                bRedirectTest = false;
                iServerAdminPage = 4;
                break;
            }

            // Delete backup?
            if (WasButtonPressed (BID_DELETEBACKUP)) {

                bRedirectTest = false;

                pHttpForm = m_pHttpRequest->GetForm ("DBRestore");
                if (pHttpForm == NULL || (pszValue = pHttpForm->GetValue()) == NULL) {

                    AddMessage ("No backup to restore");

                } else {

                    int iDay, iMonth, iYear, iVersion;
                    if (sscanf (pszValue, "%i.%i.%i.%i", &iDay, &iMonth, &iYear, &iVersion) != 4) {
                        AddMessage ("No backup to restore");
                    } else {

                        if (g_pGameEngine->DeleteDatabaseBackup (m_iEmpireKey, iDay, iMonth, iYear, iVersion) != OK) {
                            AddMessage ("The backup could not be deleted");
                        } else {

                            char pszVer[128];
                            if (iVersion == 0) {
                                memcpy (pszVer, " ", sizeof (" "));
                            } else {
                                sprintf (pszVer, "(%i) ", iVersion);
                            }

                            char pszMessage [512];
                            sprintf (
                                pszMessage,
                                "The %i_%i_%i%sbackup will be deleted",
                                iYear,
                                iMonth,
                                iDay,
                                pszVer
                                );

                            AddMessage (pszMessage);
                        }
                    }
                }

                break;
            }

            // Redirect to default alien choice?
            if (m_pHttpRequest->GetFormBeginsWith ("DefaultAlien") != NULL) {
                bRedirectTest = false;
                iServerAdminPage = 1;
                break;
            }

            // Redirect to default alien choice?
            if (m_pHttpRequest->GetFormBeginsWith ("SysMsgAlien") != NULL) {
                bRedirectTest = false;
                iServerAdminPage = 6;
                break;
            }

            // Redirect to UI choice?
            if (m_pHttpRequest->GetForm ("ChooseUI.x") != NULL ||
                m_pHttpRequest->GetForm ("ChooseUI") != NULL ||
                m_pHttpRequest->GetForm ("Planet1.0.x") != NULL ||
                m_pHttpRequest->GetForm ("Planet2.0.x") != NULL) {
                iServerAdminPage = 2;
                bRedirectTest = false;
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
                iErrCode = g_pGameEngine->GetSystemProperty (SystemData::DefaultAlien, &vDefAlien);

                if (i != vDefAlien.GetInteger()) {

                    switch (g_pGameEngine->SetSystemProperty (SystemData::DefaultAlien, i)) {
                    case OK:
                        AddMessage ("The default alien icon was updated");
                        break;
                    default:
                        AddMessage ("Error ");
                        AppendMessage (iErrCode);
                        break;
                    }

                } else {
                    AddMessage ("The default alien icon was not updated");
                }
                bRedirectTest = false;
                break;
            }

            }
            break;

        case 2:
            {

            unsigned int iBackground, iLivePlanet, iDeadPlanet, iButtons, iSeparator, iHorz, iVert, iColor;

            Check (g_pGameEngine->GetDefaultUIKeys (
                &iBackground,
                &iLivePlanet,
                &iDeadPlanet,
                &iButtons,
                &iSeparator,
                &iHorz,
                &iVert,
                &iColor
                ));

            // Background
            unsigned int iKey;
            if ((pHttpForm = m_pHttpRequest->GetForm ("Background")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iBackground) {
                Check (g_pGameEngine->SetSystemProperty (SystemData::DefaultUIBackground, iKey));
                AddMessage ("The default background key was updated");
            }

            // Live planet
            if ((pHttpForm = m_pHttpRequest->GetForm ("LivePlanet")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iLivePlanet) {
                Check (g_pGameEngine->SetSystemProperty (SystemData::DefaultUILivePlanet, iKey));
                AddMessage ("The default live planet key was updated");
            }

            // Dead planet
            if ((pHttpForm = m_pHttpRequest->GetForm ("DeadPlanet")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iDeadPlanet) {
                Check (g_pGameEngine->SetSystemProperty (SystemData::DefaultUIDeadPlanet, iKey));
                AddMessage ("The default dead planet key was updated");
            }

            // Buttons
            if ((pHttpForm = m_pHttpRequest->GetForm ("Button")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iButtons) {
                Check (g_pGameEngine->SetSystemProperty (SystemData::DefaultUIButtons, iKey));
                AddMessage ("The default button key was updated");
            }

            // Separator
            if ((pHttpForm = m_pHttpRequest->GetForm ("Separator")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iSeparator) {
                Check (g_pGameEngine->SetSystemProperty (SystemData::DefaultUISeparator, iKey));
                AddMessage ("The default separator key was updated");
            }

            // Horz
            if ((pHttpForm = m_pHttpRequest->GetForm ("Horz")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iHorz) {
                Check (g_pGameEngine->SetSystemProperty (SystemData::DefaultUIHorz, iKey));
                AddMessage ("The default horizontal link bar was updated");
            }

            // Vert
            if ((pHttpForm = m_pHttpRequest->GetForm ("Vert")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iVert) {
                Check (g_pGameEngine->SetSystemProperty (SystemData::DefaultUIVert, iKey));
                AddMessage ("The default vertical link bar was updated");
            }

            // Color key
            if ((pHttpForm = m_pHttpRequest->GetForm ("Color")) == NULL) {
                goto Redirection;
            }
            iKey = pHttpForm->GetUIntValue();
            if (iKey != iColor) {
                Check (g_pGameEngine->SetSystemProperty (SystemData::DefaultUIColor, iKey));
                AddMessage ("The default color scheme was updated");
            }

            const char* pszStart;
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ThemeInfo")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "ThemeInfo%d", &iInfoThemeKey) == 1) {

                iServerAdminPage = 3;
                bRedirectTest = false;
                break;
            }

            }

            break;

        case 3:

            break;

        case 4:
            {

            const char* pszValue;

            if (WasButtonPressed (BID_RESTOREBACKUP)) {

                bRedirectTest = false;

                pHttpForm = m_pHttpRequest->GetForm ("DBRestore");
                if (pHttpForm == NULL || (pszValue = pHttpForm->GetValue()) == NULL) {

                    AddMessage ("There are no backups to restore");

                } else {

                    int iDay, iMonth, iYear, iVersion;
                    if (sscanf (pszValue, "%i.%i.%i.%i", &iDay, &iMonth, &iYear, &iVersion) != 4) {
                        AddMessage ("No backup to restore");
                    } else {

                        if (g_pGameEngine->RestoreDatabaseBackup (m_iEmpireKey, iDay, iMonth, iYear, iVersion) != OK) {
                            AddMessage ("The backup could not be restored");
                        } else {

                            char pszVer[128];
                            if (iVersion == 0) {
                                pszVer[0] = '\0';
                            } else {
                                sprintf (pszVer, "(%i)", iVersion);
                            }

                            char pszMessage [512];
                            sprintf (
                                pszMessage,
                                "The %i_%i_%i%s backup will be restored",
                                iYear,
                                iMonth,
                                iDay,
                                pszVer
                                );

                            AddMessage (pszMessage);
                        }
                    }
                }
            }

            }
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
                    bRedirectTest = false;
                } else {

                    if (m_pHttpRequest->GetForm ("TestPurge") != NULL) {
                        iCriteria |= TEST_PURGE_ONLY;
                    }

                    iErrCode = g_pGameEngine->PurgeDatabase (m_iEmpireKey, iCriteria);

                    if (iErrCode == OK) {

                        if (iCriteria & TEST_PURGE_ONLY) {
                            AddMessage ("The database is being test purged");
                        } else {
                            AddMessage ("The database is being purged");
                        }

                    } else {
                        AddMessage ("An error occurred while requesting a purge: ");
                        AppendMessage (iErrCode);
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
                iErrCode = g_pGameEngine->GetSystemProperty (SystemData::SystemMessagesAlienKey, &vSysMsgAlien);

                if (i != vSysMsgAlien.GetInteger()) {

                    switch (g_pGameEngine->SetSystemProperty (SystemData::SystemMessagesAlienKey, i)) {
                    case OK:
                        AddMessage ("The alien icon for system messages was updated");
                        break;
                    default:
                        AddMessage ("Error ");
                        AppendMessage (iErrCode);
                        break;
                    }

                } else {
                    AddMessage ("The alien icon for system messages was not updated");
                }
                bRedirectTest = false;
                break;
            }

            }
            break;

        case 7:

            if (WasButtonPressed (BID_CREATEEMPIRE)) {

                bRedirectTest = false;

                int iPrivilege;
                unsigned int iNewEmpireKey;
                bool bFlag;
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

                if (VerifyEmpireName (pszNewName) != OK || 
                    StandardizeEmpireName (pszNewName, pszStandardizedName) != OK) {
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                if (VerifyPassword (pszNewPass) != OK) {
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                // Test empire existence
                iErrCode = g_pGameEngine->DoesEmpireExist (pszStandardizedName, &bFlag, &iNewEmpireKey, NULL, NULL);
                if (iErrCode != OK || bFlag) {
                    AddMessage ("That empire already exists");
                    iServerAdminPage = 7;
                    goto Redirection;
                }

                iErrCode = g_pGameEngine->CreateEmpire (
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

                    AddMessage ("An error occurred creating the empire: ");
                    AppendMessage (iErrCode);
                    break;
                }
            }
            break;

        default:

            Assert (false);
            break;
        }
    }
}

SYSTEM_REDIRECT_ON_SUBMIT

SYSTEM_OPEN (iServerAdminPage == 0)

// Individual page stuff starts here
switch (iServerAdminPage) {

Main:
case 0:
    {

    Seconds sSecondsForLongtermStatus, sBridierScan;
    int iNumHrs, iNumMin, j, iSystemOptions;

    String strFilter;
    Variant vAuthorName;

    IDatabaseBackupEnumerator* pBackupEnumerator = NULL;

    Variant* pvServerData = NULL;
    IDatabase* pDatabase = g_pGameEngine->GetDatabase();
    if (pDatabase == NULL) {
        goto Cancel;
    }

    iErrCode = pDatabase->ReadRow (SYSTEM_DATA, &pvServerData);
    if (iErrCode != OK) {
        goto Cancel;
    }

    %><input type="hidden" name="ServerAdminPage" value="0"><%

    %><h3>Server Management</h3><%

    %><p><table width="90%"><%

    %><tr><td>Server name:</td><%
    %><td><input type="text" size="25" maxlength="<% Write (MAX_SERVER_NAME_LENGTH); %>" name="NewServerName"<% 
    %> value="<% Write (pvServerData[SystemData::ServerName].GetCharPtr()); 
    %>"><input type="hidden" name="OldServerName" value="<% 

    Write (pvServerData[SystemData::ServerName].GetCharPtr()); %>"></td></tr><%


    %><tr><td>Administrator e-mail address:</td><%
    %><td><input type="text" size="40" maxlength="<% Write (MAX_EMAIL_LENGTH); %>" name="NewAdminEmail"<% 
    %> value="<% Write (pvServerData[SystemData::AdminEmail].GetCharPtr()); 
    %>"><input type="hidden" name="OldAdminEmail" value="<% 

    Write (pvServerData[SystemData::AdminEmail].GetCharPtr()); %>"></td></tr><%

    %><tr><td>Default alien icon:</td><td><%

    Check (g_pGameEngine->GetAlienAuthorName (pvServerData[SystemData::DefaultAlien], &vAuthorName));

    WriteAlienButtonString (
        pvServerData[SystemData::DefaultAlien], 
        true,
        "DefaultAlien",
        vAuthorName.GetCharPtr()
        );

    %></td></tr><%

    %><tr><td>Default UI elements:</td><td><table><tr><%
    if (pvServerData[SystemData::DefaultUIBackground].GetInteger() == NULL_THEME) { 
        %><td width="75" height="75" bgcolor="#000000">&nbsp;</td><% 
    } else { 
        %><td><input type="image" border="0" width="75" height="75" src="<%
        WriteBackgroundImageSrc (pvServerData[SystemData::DefaultUIBackground].GetInteger());
        %>" name="ChooseUI"></td><% 
    }
    %><td><% 

    GetLivePlanetButtonString (pvServerData[SystemData::DefaultUILivePlanet], 1, 0, NULL, NULL, &strFilter);
    Write (strFilter);

    %></td><td><%

    GetDeadPlanetButtonString (pvServerData[SystemData::DefaultUIDeadPlanet], 2, 0, NULL, NULL, &strFilter);
    Write (strFilter);

    %></td><td><% 
    WriteButtonString (pvServerData[SystemData::DefaultUIButtons], "Login", "Login", "ChooseUI");
    %></td><%

    if (pvServerData[SystemData::DefaultUISeparator].GetInteger() == NULL_THEME) {
        %><td width="150"><%
        Write (DEFAULT_SEPARATOR_STRING);
    } else { 
        %><td><input type="image" name="ChooseUI" border="0" width="150" src="<% 
        WriteSeparatorSrc (pvServerData[SystemData::DefaultUISeparator].GetInteger());
        %>"><% 
    }

    %></td><td><input type="image" name="ChooseUI" width="21" height="3" border="0" src="<%
    WriteHorzSrc (pvServerData[SystemData::DefaultUIHorz].GetInteger());
    %>"></td><td><input type="image" name="ChooseUI" width="3" height="21" border="0" src="<%
    WriteVertSrc (pvServerData[SystemData::DefaultUIVert].GetInteger());
    %>"></td></tr></table></td></tr><%


    %><tr><td>Create a new empire:</td><td><%
    WriteButton (BID_CREATEEMPIRE);
    %></td></tr><%


    %><tr><td>The Almonaster scoring system:</td><td><select name="PrivElev"><%

    iSystemOptions = pvServerData[SystemData::Options].GetInteger();

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
    Write (pvServerData[SystemData::ApprenticeScore]); 
    %>"><input type="hidden" name="OldApprentice" value="<% Write (pvServerData[SystemData::ApprenticeScore]); 
    %>"></td></tr><%

    %><tr><td>Almonaster score needed to be an Adept:</td><%
    %><td><input type="text" size="7" maxlength="7" name="NewAdept" value="<% 
    Write (pvServerData[SystemData::AdeptScore]); 
    %>"><input type="hidden" name="OldAdept" value="<% Write (pvServerData[SystemData::AdeptScore]); 
    %>"></td></tr><%

    %><tr><td>Alien icon for system messages:</td><td><%

    Check (g_pGameEngine->GetAlienAuthorName (
        pvServerData[SystemData::SystemMessagesAlienKey].GetInteger(), 
        &vAuthorName
        ));

    WriteAlienButtonString (
        pvServerData[SystemData::SystemMessagesAlienKey].GetInteger(),
        true,
        "SysMsgAlien",
        vAuthorName.GetCharPtr()
        );

    %></td></tr><%

    %><tr><td>Maximum saved system messages:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewMaxNumSystemMessages" value="<% 
    Write (pvServerData[SystemData::MaxNumSystemMessages]); %>"><input type="hidden" name="OldMaxNumSystemMessages" value="<% 
    Write (pvServerData[SystemData::MaxNumSystemMessages]); %>"></td></tr><tr><td>Maximum saved game messages:</td><%

    %><td><input type="text" size="6" maxlength="6" name="NewMaxNumGameMessages" value="<% 
    Write (pvServerData[SystemData::MaxNumGameMessages]); %>"><input type="hidden" name="OldMaxNumGameMessages" value="<% 
    Write (pvServerData[SystemData::MaxNumGameMessages]); %>"></td></tr><tr><td>Default maximum saved system messages:</td><%

    %><td><select name="NewDefaultMaxNumSystemMessages"><% 

    for (i = 0; i <= pvServerData[SystemData::MaxNumSystemMessages].GetInteger(); i += 10) {
        %><option<%
        if (pvServerData[SystemData::DefaultMaxNumSystemMessages] == i) {
            %> selected<% 
        }
        %> value="<% Write (i); %>"><% Write (i); %></option><% 
    }

    %></select><input type="hidden" name="OldDefaultMaxNumSystemMessages" value="<% 
    Write (pvServerData[SystemData::DefaultMaxNumSystemMessages].GetInteger()); %>"><%
    %></td></tr><%
    
    %><tr><td>Default maximum saved game messages:</td><%

    %><td><select name="NewDefaultMaxNumGameMessages"><% 
    for (i = 0; i <= pvServerData[SystemData::MaxNumGameMessages].GetInteger(); i += 10) {
        %> <option<% if (pvServerData[SystemData::DefaultMaxNumGameMessages].GetInteger() == i) { %> selected<% }
        %> value="<% Write (i); %>"><% Write (i); %></option><% 
    }

    %></select><input type="hidden" name="OldDefaultMaxNumGameMessages" value="<% 
    Write (pvServerData[SystemData::DefaultMaxNumGameMessages]); %>"></td></tr><%

    %><tr><td>Maximum size of uploaded icons:</td><%
    %><td><input type="text" size="8" maxlength="8" name="MaxSizeIcons" value="<% 
    Write (pvServerData[SystemData::MaxIconSize].GetInteger()); %>"> bytes<input type="hidden" name="OldMaxSizeIcons" value="<% 
    Write (pvServerData[SystemData::MaxIconSize].GetInteger()); %>"><%
    %></td></tr><%

    %><tr><td>Number of nukes listed in empire nuke histories:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewNumNukes" value="<% 
    Write (pvServerData[SystemData::NumNukesListedInNukeHistories].GetInteger()); %>"><%
    %><input type="hidden" name="OldNumNukes" value="<%
    Write (pvServerData[SystemData::NumNukesListedInNukeHistories].GetInteger()); %>"></td></tr><%


    %><tr><td>Number of nukes listed in the latest nukes screen:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewSystemNukes" value="<% 
    Write (pvServerData[SystemData::NumNukesListedInSystemNukeList].GetInteger()); %>"><%
    %><input type="hidden" name="OldSystemNukes" value="<%
    Write (pvServerData[SystemData::NumNukesListedInSystemNukeList].GetInteger()); %>"></td></tr><%


    %><tr><td>Number of games listed in the latest games screen:</td><%
    %><td><input type="text" size="6" maxlength="6" name="NewGamesListed" value="<% 
    Write (pvServerData[SystemData::NumGamesInLatestGameList].GetInteger()); %>"><%
    %><input type="hidden" name="OldGamesListed" value="<%
    Write (pvServerData[SystemData::NumGamesInLatestGameList].GetInteger()); %>"></td></tr><%


    %><tr><td>Number of updates the server can be down before games are killed (except longterms):</td><td><%
    %><input type="text" size="4" maxlength="10" name="UpdatesDown"<%
    %> value="<% Write (pvServerData[SystemData::NumUpdatesDownBeforeGameIsKilled].GetInteger()); %>"><%
    %><input type="hidden" name="OldUpdatesDown" value="<%
    Write (pvServerData[SystemData::NumUpdatesDownBeforeGameIsKilled].GetInteger()); %>"></td></tr><%

    sSecondsForLongtermStatus = pvServerData[SystemData::SecondsForLongtermStatus].GetInteger();

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

    sBridierScan = pvServerData[SystemData::BridierTimeBombScanFrequency].GetInteger();

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

    %><tr><td>Rebuild the system Top Lists:</td><td><%
    WriteButton (BID_REBUILD);
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
    %>value="<% Write (pvServerData[SystemData::LoginsDisabledReason].GetCharPtr()); %>"></td></tr><%

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
    %>value="<% Write (pvServerData[SystemData::NewEmpiresDisabledReason].GetCharPtr()); %>"></td></tr><%

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
    %>value="<% Write (pvServerData[SystemData::NewGamesDisabledReason].GetCharPtr()); %>"></td></tr><%

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
    %>value="<% Write (pvServerData[SystemData::AccessDisabledReason].GetCharPtr()); %>"></td></tr><%

    %><tr><td>Shut down the server:</td><td><% 
    WriteButton (BID_SHUTDOWNSERVER);
    %></td></tr><%

    %><tr><td>Restart the server:</td><td><% 
    WriteButton (BID_RESTARTSERVER);
    %></td></tr><%

    %><tr><td>Restart the Almonaster PageSource:</td><td><% 
    WriteButton (BID_RESTARTALMONASTER);
    %></td></tr><%

    %><tr><td>Flush all database tables:</td><%
    %><td><% WriteButton (BID_FLUSH);
    %></td></tr><%

    %><tr><td>Run a custom purge of the empire database:</td><%
    %><td><% WriteButton (BID_PURGE);
    %></td></tr><%

    %><tr><td>Administer database backups:</td><td><%

    pBackupEnumerator = pDatabase->GetBackupEnumerator();
    if (pBackupEnumerator == NULL) {
        %>Could not enumerate backups<%
    } else {

        int iDay, iMonth, iYear, iVersion;

        IDatabaseBackup** ppBackups = pBackupEnumerator->GetBackups();
        unsigned int iNumBackups = pBackupEnumerator->GetNumBackups();
        if (ppBackups == NULL || iNumBackups == 0) {
            %>There are no backups to restore<%
        } else {

            %><select name="DBRestore"><%

            for (i = 0; i < (int) iNumBackups; i ++) {

                ppBackups[i]->GetDate (&iDay, &iMonth, &iYear, &iVersion);

                %><option value="<% Write (iDay); %>.<% Write (iMonth); %>.<% Write (iYear); %>.<% Write (iVersion); %>"><%

                Write (Time::GetMonthName (iMonth)); %> <%
                Write (iDay); %>, <%
                Write (iYear);

                if (iVersion != 0) {
                    %> (<% Write (iVersion); %>)<%
                }
                %></option><%
            }
            %></select> <%

            WriteButton (BID_RESTOREBACKUP);
            WriteButton (BID_DELETEBACKUP);
        }

        pBackupEnumerator->Release();
    }

    %></td></tr><%

    %></tr><tr><td>Backup the database:</td><td><p><% 
    WriteButton (BID_BACKUP); 
    %></tr><%

    %><tr><td>Create a new alien icon:</td><%
    %><td><table><tr><td>Key:</td><td><%

    %><input type="text" size="6" maxlength="6" name="NewAlienKey"></td></tr><tr><td><%
    %>Author's name:</td><td><input type="text" size="20" <%
    %>maxlength="<% Write (MAX_ALIEN_AUTHOR_NAME_LENGTH); %>" name="NewAuthorName"></td></tr><%

    %><tr><td>File:</td><td><input type="file" name="NewAlienFile" size="20"></td></tr><%
    %><tr><td><% 
    WriteButton (BID_CREATEALIENICON);
    %></table></tr><tr><td>Delete an alien icon:</td><td><table><%

    %><tr><td>Key:</td><td><input type="text" size="6" maxlength="6" name="OldAlienKey"></td></tr><tr><td><% 
    WriteButton (BID_DELETEALIENICON);
    %></table></tr><%

    %><tr><td>Default ship names:</td><td><table width="60%"><%

    for (i = FIRST_SHIP; i < NUM_SHIP_TYPES / 2; i ++) {

        %><tr><%

        if (HTMLFilter (pvServerData[SYSTEM_DATA_SHIP_NAME_COLUMN[i]].GetCharPtr(), &strFilter, 0, false) == OK) {

            %><td><% Write (SHIP_TYPE_STRING[i]); %>:</td><%
            %><td><%
            %><input type="text" size="12" maxlength="<% Write (MAX_SHIP_NAME_LENGTH); %>" <%
            %>name="ShipName<% Write (i); %>" <%
            %>value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength()); %>"><%
            %></td><%
        }

        j = i + NUM_SHIP_TYPES / 2;

        if (HTMLFilter (pvServerData[SYSTEM_DATA_SHIP_NAME_COLUMN[j]].GetCharPtr(), &strFilter, 0, false) == OK) {

            %><td><% Write (SHIP_TYPE_STRING[j]); %>:</td><%
            %><td><input type="text" size="12" maxlength="<% Write (MAX_SHIP_NAME_LENGTH); %>" <%
            %>name="ShipName<% Write (j); %>" <%
            %>value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength()); %>"><%
            %></td><%
        }

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

Cancel:

    WriteButton(BID_CANCEL);

    if (pvServerData != NULL) {
        pDatabase->FreeData (pvServerData);
    }

    if (pDatabase != NULL) {
        pDatabase->Release();
    }

    }

    break;

case 1:
    {

    int iNumAliens;
    Variant vAlien;

    Check (g_pGameEngine->GetSystemProperty (SystemData::DefaultAlien, &vAlien));
    int iAlien = vAlien.GetInteger();

    Variant** ppvAlienData;
    Check (g_pGameEngine->GetAlienKeys (&ppvAlienData, &iNumAliens));
    %><input type="hidden" name="ServerAdminPage" value="1"><%

    %><p><%

    WriteIcon (iAlien, NO_KEY, NO_KEY, "The current default alien icon", NULL, false);

    %><p>Choose a new default alien icon:<p><table width="75%"><tr><td><%

    for (i = 0; i < iNumAliens; i ++) {

        WriteAlienButtonString (
            ppvAlienData[i][SystemAlienIcons::AlienKey],
            iAlien == ppvAlienData[i][SystemAlienIcons::AlienKey],
            "Alien",
            ppvAlienData[i][SystemAlienIcons::AuthorName].GetCharPtr()
            );
            %> <%
    }

    if (iNumAliens > 0) {
        g_pGameEngine->FreeData (ppvAlienData);
    }

    %></td></tr></table><%
    %><p><% WriteButton (BID_CANCEL);
    }

    break;

case 2:
    {

    int iB, iL, iD, iS, iT, iH, iV, iC;

    IDatabase* pDatabase = g_pGameEngine->GetDatabase();

    IReadTable* pSystemData;
    void** ppData;

    iErrCode = pDatabase->GetTableForReading (SYSTEM_DATA, &pSystemData);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = pSystemData->ReadRow (0, &ppData);
    if (iErrCode != OK) {
        pSystemData->Release();
        goto Cleanup;
    }

    iB = *((int*) ppData[SystemData::DefaultUIBackground]);
    iL = *((int*) ppData[SystemData::DefaultUILivePlanet]);
    iD = *((int*) ppData[SystemData::DefaultUIDeadPlanet]);
    iS = *((int*) ppData[SystemData::DefaultUISeparator]);
    iT = *((int*) ppData[SystemData::DefaultUIButtons]);
    iH = *((int*) ppData[SystemData::DefaultUIHorz]);
    iV = *((int*) ppData[SystemData::DefaultUIVert]);
    iC = *((int*) ppData[SystemData::DefaultUIColor]);

    pDatabase->FreeData (ppData);

    SafeRelease (pSystemData);

    %><input type="hidden" name="ServerAdminPage" value="2"><p>Choose the server's default UI elements:<p><%

    iErrCode = RenderThemeInfo (
        iB,
        iL,
        iD,
        iS,
        iT,
        iH,
        iV,
        iC
        );

    %></table><p><%

    if (iErrCode != OK) {
        %>Error <% Write (iErrCode); %> occurred rendering theme info<p><%
    }

    WriteButton (BID_CANCEL);

Cleanup:

    pDatabase->Release();

    }
    break;

case 3:
    {

    %><input type="hidden" name="ServerAdminPage" value="3"><%

    DisplayThemeData (iInfoThemeKey);

    }
    break;

case 4:
    {

    const char* pszValue;

    pHttpForm = m_pHttpRequest->GetForm ("DBRestore");
    if (pHttpForm == NULL || (pszValue = pHttpForm->GetValue()) == NULL) {
        goto Main;
    }

    %><input type="hidden" name="ServerAdminPage" value="4"><%
    %><input type="hidden" name="DBRestore" value="<% Write (pszValue); %>"><%

    %><p>You have chosen to restore a database backup.  Are you sure you want to do this?<p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_RESTOREBACKUP);

    }
    break;

case 5:

    {

    int iNumEmpires;
    Check (g_pGameEngine->GetNumEmpiresOnServer (&iNumEmpires));
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

    int iNumAliens;
    Variant vAlien;

    Check (g_pGameEngine->GetSystemProperty (SystemData::SystemMessagesAlienKey, &vAlien));
    int iAlien = vAlien.GetInteger();

    Variant** ppvAlienData;
    Check (g_pGameEngine->GetAlienKeys (&ppvAlienData, &iNumAliens));
    %><input type="hidden" name="ServerAdminPage" value="6"><%

    %><p><%

    WriteIcon (iAlien, NO_KEY, NO_KEY, "The current alien icon for system messages", NULL, false);

    %><p>Choose a new alien icon for system messages:<p><table width="75%"><tr><td><%

    for (i = 0; i < iNumAliens; i ++) {

        WriteAlienButtonString (
            ppvAlienData[i][SystemAlienIcons::AlienKey],
            iAlien == ppvAlienData[i][SystemAlienIcons::AlienKey],
            "Alien",
            ppvAlienData[i][SystemAlienIcons::AuthorName].GetCharPtr()
            );
            %> <%
    }

    if (iNumAliens > 0) {
        g_pGameEngine->FreeData (ppvAlienData);
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
    %><td><input type="password" name="NPassword" size="20" maxlength="<% Write (MAX_PASSWORD_LENGTH); %>"></td><%
    %></tr><%

    %><tr><%
    %><td><strong>Verify password</strong>:</td><%
    %><td><input type="password" name="NPassword2" size="20" maxlength="<% Write (MAX_PASSWORD_LENGTH); %>"></td><%
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

    Assert (false);
}

SYSTEM_CLOSE

%>