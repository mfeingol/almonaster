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

enum AutoLogonVars {
    NO_AUTOLOGON = -1,
    MAYBE_AUTOLOGON = -2
};

int iErrCode;

bool bInitialized;
iErrCode = InitializeEmpire(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

IHttpForm* pHttpForm;

int i, iProfileEditorPage = 0, iInfoThemeKey = NO_KEY, iAlienSelect = 0, iNewButtonKey = m_iButtonKey, iAutoLogonSelected = MAYBE_AUTOLOGON;
const char* pszGraphicsPath = NULL;

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if ((pHttpForm = m_pHttpRequest->GetForm ("ProfileEditorPage")) == NULL) {
        goto Redirection;
    }
    int iProfileEditorPageSubmit = pHttpForm->GetIntValue();

    if (WasButtonPressed (BID_CANCEL)) {

        if (iProfileEditorPageSubmit == 7) {
            m_bRedirectTest = false;
            iProfileEditorPage = 3;
        }

    } else {
    
        Variant vValue;

        switch (iProfileEditorPageSubmit) {
        case 0:
            {

            Variant vVerify;
            const char* pszNewValue, * pszVerify;
            int iNewValue, iVerify, iValue;

            bool bIPAddress, bSessionId, bValue, bValue2, bNewValue, bNewValue2;

            // Handle empire name recasing
            if ((pHttpForm = m_pHttpRequest->GetForm ("RecasedEmpireName")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pszNewValue, m_vEmpireName.GetCharPtr()) != 0)
            {
                if (String::StriCmp (pszNewValue, m_vEmpireName.GetCharPtr()) == 0)
                {
                    iErrCode = SetEmpireName (m_iEmpireKey, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your empire name was recased");
                    m_vEmpireName = pszNewValue;

                } else {
                    AddMessage ("You can only recase your empire's name, not rename it");
                }
            }

            // Handle password change
            if (m_iEmpireKey != global.GetGuestKey())
            {
                if ((pHttpForm = m_pHttpRequest->GetForm ("NewPassword")) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();

                if ((pHttpForm = m_pHttpRequest->GetForm ("VerifyPassword")) == NULL) {
                    goto Redirection;
                }
                pszVerify = pHttpForm->GetValue();
                
                if (String::StrCmp (pszNewValue, pszVerify) != 0)
                {
                    AddMessage ("Your password confirmation did not match");
                }
                else if (String::StrCmp(pszNewValue, INVALID_PASSWORD_STRING) != 0)
                {
                    if (VerifyPassword (pszNewValue))
                    {
                        iErrCode = ChangeEmpirePassword(m_iEmpireKey, pszNewValue);
                        if (iErrCode == ERROR_CANNOT_MODIFY_GUEST)
                        {
                            AddMessage (GUEST_NAME "'s password can only be changed by an administrator");
                        }
                        else
                        {
                            RETURN_ON_ERROR(iErrCode);
                            AddMessage ("Your password was changed");

                            ICookie* pCookie = m_pHttpRequest->GetCookie(AUTOLOGON_EMPIREKEY_COOKIE);
                            if (pCookie && pCookie->GetValue() && pCookie->GetUIntValue() == m_iEmpireKey)
                            {
                                String strHash;
                                iErrCode = GetAutologonPasswordHash(m_iEmpireKey, &strHash);
                                RETURN_ON_ERROR(iErrCode);
                                
                                iErrCode = m_pHttpResponse->CreateCookie(AUTOLOGON_PASSWORD_COOKIE, strHash, ONE_YEAR_IN_SECONDS, NULL);
                                RETURN_ON_ERROR(iErrCode);
                            }
                        }
                    }
                }
            }

            // Handle RealName change
            if ((pHttpForm = m_pHttpRequest->GetForm ("RealName")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (pszNewValue == NULL) {
                pszNewValue = "";
            }

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::RealName, &vVerify);
            RETURN_ON_ERROR(iErrCode);

            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0)
            {
                if (strlen (pszNewValue) > MAX_REAL_NAME_LENGTH)
                {
                    AddMessage ("Your real name was too long");
                }
                else
                {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::RealName, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your real name was changed");
                }
            }

            // Handle EmpAge change
            if ((pHttpForm = m_pHttpRequest->GetForm ("EmpAge")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Age, &vVerify);
            RETURN_ON_ERROR(iErrCode);
            if (iNewValue != vVerify.GetInteger()) {

                if (iNewValue < EMPIRE_AGE_MINIMUM && iNewValue != EMPIRE_AGE_UNKNOWN) {
                    AddMessage ("Your age is invalid");
                } else {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::Age, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your age was changed");
                }
            }

            // Handle EmpGender change
            if ((pHttpForm = m_pHttpRequest->GetForm ("EmpGender")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Gender, &vVerify);
            RETURN_ON_ERROR(iErrCode);
            if (iNewValue != vVerify.GetInteger()) {

                if (iNewValue < EMPIRE_GENDER_UNKNOWN || iNewValue > EMPIRE_GENDER_FEMALE) {
                    AddMessage ("Your gender is invalid");
                } else {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::Gender, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your gender was changed");
                }
            }

            // Handle Location change
            if ((pHttpForm = m_pHttpRequest->GetForm ("Location")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (pszNewValue == NULL) {
                pszNewValue = "";
            }

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Location, &vVerify);
            RETURN_ON_ERROR(iErrCode);

            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

                if (strlen (pszNewValue) > MAX_LOCATION_LENGTH) {
                    AddMessage ("Your location was too long");
                }

                else
                {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::Location, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your location was changed");
                }
            }

            // Handle Email change
            if ((pHttpForm = m_pHttpRequest->GetForm ("Email")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (pszNewValue == NULL) {
                pszNewValue = "";
            }

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Email, &vVerify);
            RETURN_ON_ERROR(iErrCode);

            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

                if (strlen (pszNewValue) > MAX_EMAIL_LENGTH) {
                    AddMessage ("Your e-mail address was too long");
                }
                else
                {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::Email, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your e-mail address was changed");
                }
            }

            // Handle Private email change
            if ((pHttpForm = m_pHttpRequest->GetForm ("PrivEmail")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (pszNewValue == NULL) {
                pszNewValue = "";
            }

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::PrivateEmail, &vVerify);
            RETURN_ON_ERROR(iErrCode);

            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

                if (strlen (pszNewValue) > MAX_EMAIL_LENGTH) {
                    AddMessage ("Your private e-mail address was too long");
                }
                else
                {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::PrivateEmail, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your private e-mail address was changed");
                }
            }

            // Handle IMId change
            if ((pHttpForm = m_pHttpRequest->GetForm ("IMId")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (pszNewValue == NULL) {
                pszNewValue = "";
            }

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::IMId, &vVerify);
            RETURN_ON_ERROR(iErrCode);

            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

                if (strlen (pszNewValue) > MAX_IMID_LENGTH) {
                    AddMessage ("Your instant messenger id was too long");
                }
                else
                {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::IMId, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your instant messenger id was changed");
                }
            }

            // Handle WebPage change
            if ((pHttpForm = m_pHttpRequest->GetForm ("WebPage")) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (pszNewValue == NULL) {
                pszNewValue = "";
            }

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::WebPage, &vVerify);
            RETURN_ON_ERROR(iErrCode);

            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

                if (strlen (pszNewValue) > MAX_WEB_PAGE_LENGTH) {
                    AddMessage ("Your real name was too long");
                }
                else
                {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::WebPage, pszNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your webpage was changed");
                }
            }

            // TourneyAvail
            if ((pHttpForm = m_pHttpRequest->GetForm ("TourneyAvail")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = !(m_iSystemOptions2 & UNAVAILABLE_FOR_TOURNAMENTS);
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption2 (m_iEmpireKey, UNAVAILABLE_FOR_TOURNAMENTS, bValue);
                RETURN_ON_ERROR(iErrCode);

                if (bValue) {
                    m_iSystemOptions2 |= UNAVAILABLE_FOR_TOURNAMENTS;
                    AddMessage ("You are no longer available to be entered into tournament games");
                } else {
                    m_iSystemOptions2 &= ~UNAVAILABLE_FOR_TOURNAMENTS;
                    AddMessage ("You are now available to be entered into tournament games");
                }
            }


            // Handle MaxNumSavedMessages change
            if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumSavedMessages")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            iErrCode = GetEmpireMaxNumSavedSystemMessages (m_iEmpireKey, &iVerify);
            RETURN_ON_ERROR(iErrCode);

            if (iNewValue != iVerify) {

                iErrCode = GetSystemProperty (SystemData::MaxNumSystemMessages, &vValue);
                RETURN_ON_ERROR(iErrCode);

                if (iNewValue > vValue.GetInteger()) {
                    AddMessage ("Illegal maximum number of saved system messages");
                } else {
                    iErrCode = SetEmpireMaxNumSavedSystemMessages (m_iEmpireKey, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your maximum number of saved messages was changed");
                }
            }

            // Handle MaxNumShipsBuiltAtOnce change
            if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumShipsBuiltAtOnce")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::MaxNumShipsBuiltAtOnce, &vValue);
            RETURN_ON_ERROR(iErrCode);

            if (iNewValue != vValue.GetInteger()) {

                if (iNewValue > 100) {
                    AddMessage ("Illegal maximum number of ships built at once");
                } else {
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::MaxNumShipsBuiltAtOnce, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Your maximum number of ships built at once was updated");
                }
            }

            // Handle DefaultBuilderPlanet
            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultBuilderPlanet")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            iErrCode = GetEmpireDefaultBuilderPlanet (m_iEmpireKey, &iVerify);
            RETURN_ON_ERROR(iErrCode);

            if (iNewValue != iVerify)
            {
                iErrCode = SetEmpireDefaultBuilderPlanet (m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Your default builder planet was updated");
            }

            // Handle IndependentGifts
            if ((pHttpForm = m_pHttpRequest->GetForm ("IndependentGifts")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

            if ((iNewValue != 0) != bValue) {

                iErrCode = SetEmpireOption (m_iEmpireKey, REJECT_INDEPENDENT_SHIP_GIFTS, !bValue);
                RETURN_ON_ERROR(iErrCode);

                if (!bValue) {
                    m_iSystemOptions |= REJECT_INDEPENDENT_SHIP_GIFTS;
                } else {
                    m_iSystemOptions &= ~REJECT_INDEPENDENT_SHIP_GIFTS;
                }
                AddMessage ("Your independent gift option was updated");
            }
            
            // Handle DeleteEmptyFleets
            if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteEmptyFleets")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions2 & DISBAND_EMPTY_FLEETS_ON_UPDATE) != 0;

            if ((iNewValue != 0) != bValue) {

                iErrCode = SetEmpireOption2 (m_iEmpireKey, DISBAND_EMPTY_FLEETS_ON_UPDATE, !bValue);
                RETURN_ON_ERROR(iErrCode);

                if (!bValue) {
                    m_iSystemOptions2 |= DISBAND_EMPTY_FLEETS_ON_UPDATE;
                } else {
                    m_iSystemOptions2 &= ~DISBAND_EMPTY_FLEETS_ON_UPDATE;
                }
                AddMessage ("Your empty fleet disband option was updated");
            }

            // Handle CollapseFleets
            if ((pHttpForm = m_pHttpRequest->GetForm ("CollapseFleets")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            bValue = (m_iSystemOptions2 & FLEETS_COLLAPSED_BY_DEFAULT) != 0;

            if ((iNewValue != 0) != bValue) {

                iErrCode = SetEmpireOption2 (m_iEmpireKey, FLEETS_COLLAPSED_BY_DEFAULT, !bValue);
                RETURN_ON_ERROR(iErrCode);

                if (!bValue) {
                    m_iSystemOptions2 |= FLEETS_COLLAPSED_BY_DEFAULT;
                } else {
                    m_iSystemOptions2 &= ~FLEETS_COLLAPSED_BY_DEFAULT;
                }
                AddMessage ("Your collapsed fleets option was updated");
            }

            // Handle BlockUploadedIcons
            if ((pHttpForm = m_pHttpRequest->GetForm ("BlockUploadedIcons")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();
            bValue = (m_iSystemOptions2 & BLOCK_UPLOADED_ICONS) != 0;

            if ((iNewValue != 0) != bValue) {

                iErrCode = SetEmpireOption2 (m_iEmpireKey, BLOCK_UPLOADED_ICONS, !bValue);
                RETURN_ON_ERROR(iErrCode);

                if (!bValue) {
                    m_iSystemOptions2 |= BLOCK_UPLOADED_ICONS;
                } else {
                    m_iSystemOptions2 &= ~BLOCK_UPLOADED_ICONS;
                }
                AddMessage ("Your block uploaded icons option was updated");
            }

            // Handle Confirm
            if ((pHttpForm = m_pHttpRequest->GetForm ("Confirm")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bSessionId = (m_iSystemOptions & CONFIRM_IMPORTANT_CHOICES) != 0;
            if (bSessionId != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, CONFIRM_IMPORTANT_CHOICES, !bSessionId);
                RETURN_ON_ERROR(iErrCode);

                if (bSessionId) {
                    m_iSystemOptions |= CONFIRM_IMPORTANT_CHOICES;
                    AddMessage ("You will no longer be prompted for confirmation on important decisions");
                } else {
                    m_iSystemOptions &= ~CONFIRM_IMPORTANT_CHOICES;
                    AddMessage ("You will now be prompted for confirmation on important decisions");
                }
            }

            // Handle autologon
            if ((pHttpForm = m_pHttpRequest->GetForm ("AutoLogonSel")) == NULL) {
                goto Redirection;
            }
            unsigned int uiVerify = pHttpForm->GetUIntValue();

            if ((pHttpForm = m_pHttpRequest->GetForm ("AutoLogon")) == NULL) {
                goto Redirection;
            }
            unsigned int uiNewValue = pHttpForm->GetUIntValue();

            if (uiVerify != uiNewValue) {

                if (uiNewValue == NO_AUTOLOGON) {

                    iAutoLogonSelected = NO_AUTOLOGON;
                    AddMessage ("Autologon is now off");

                    iErrCode = m_pHttpResponse->DeleteCookie(AUTOLOGON_EMPIREKEY_COOKIE, NULL);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = m_pHttpResponse->DeleteCookie(AUTOLOGON_PASSWORD_COOKIE, NULL);
                    RETURN_ON_ERROR(iErrCode);
                }
                else
                {
                    if (uiNewValue != m_iEmpireKey)
                    {
                        AddMessage ("Invalid autologon submission");
                    }
                    else
                    {
                        // Set cookies (expire in a year)
                        char pszText[128];
                        String::UItoA(m_iEmpireKey, pszText, 10);
                        iErrCode = m_pHttpResponse->CreateCookie(AUTOLOGON_EMPIREKEY_COOKIE, pszText, ONE_MONTH_IN_SECONDS, NULL);
                        RETURN_ON_ERROR(iErrCode);
                        
                        String strHash;
                        iErrCode = GetAutologonPasswordHash(m_iEmpireKey, &strHash);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = m_pHttpResponse->CreateCookie(AUTOLOGON_PASSWORD_COOKIE, strHash, ONE_MONTH_IN_SECONDS, NULL);
                        RETURN_ON_ERROR(iErrCode);

                        AddMessage ("Autologon is now on for ");
                        AppendMessage (m_vEmpireName.GetCharPtr());

                        iAutoLogonSelected = m_iEmpireKey;
                    }
                }
            }

            // Handle quote change
            const char* pszString;
            bool bTruncate;

            if ((pHttpForm = m_pHttpRequest->GetForm ("Quote")) == NULL) {
                goto Redirection;
            }
            pszString = pHttpForm->GetValue();

            if (pszString == NULL) {
                pszString = "";
            }

            iErrCode = UpdateEmpireQuote (m_iEmpireKey, pszString, &bTruncate);
            switch (iErrCode) {
            case OK:

                if (!bTruncate) {
                    AddMessage ("Your quote was updated");
                } else {
                    AddMessage ("Your quote will be truncated to ");
                    AppendMessage (MAX_QUOTE_LENGTH);
                    AppendMessage (" characters");
                }
                break;

            case WARNING:
                break;

            default:
                RETURN_ON_ERROR(iErrCode);
                break;
            }

            // Handle victory sneer change
            if ((pHttpForm = m_pHttpRequest->GetForm ("VictorySneer")) == NULL) {
                goto Redirection;
            }
            pszString = pHttpForm->GetValue();

            if (pszString == NULL) {
                pszString = "";
            }

            iErrCode = UpdateEmpireVictorySneer (m_iEmpireKey, pszString, &bTruncate);
            switch (iErrCode) {
            case OK:

                if (!bTruncate) {
                    AddMessage ("Your victory sneer was updated");
                } else {
                    AddMessage ("Your victory sneer will be truncated to ");
                    AppendMessage (MAX_QUOTE_LENGTH);
                    AppendMessage (" characters");
                }
                break;

            case WARNING:
                break;

            default:
                RETURN_ON_ERROR(iErrCode);
                break;
            }

            // Handle repeated buttons
            if ((pHttpForm = m_pHttpRequest->GetForm ("RepeatedButtons")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bNewValue = (iNewValue & SYSTEM_REPEATED_BUTTONS) != 0;
            bNewValue2 = (iNewValue & GAME_REPEATED_BUTTONS) != 0;

            bValue = (m_iSystemOptions & SYSTEM_REPEATED_BUTTONS) != 0;
            bValue2 = (m_iSystemOptions & GAME_REPEATED_BUTTONS) != 0;

            if ((bNewValue != bValue) || (bNewValue2 != bValue2)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, SYSTEM_REPEATED_BUTTONS, bNewValue);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = SetEmpireOption (m_iEmpireKey, GAME_REPEATED_BUTTONS, bNewValue2);
                RETURN_ON_ERROR(iErrCode);

                if (bNewValue) {
                    m_iSystemOptions |= SYSTEM_REPEATED_BUTTONS;
                } else {
                    m_iSystemOptions &= ~SYSTEM_REPEATED_BUTTONS;
                }

                if (bNewValue2) {
                    m_iSystemOptions |= GAME_REPEATED_BUTTONS;
                } else {
                    m_iSystemOptions &= ~GAME_REPEATED_BUTTONS;
                }

                if (bNewValue || bNewValue2) {
                    if (bNewValue) {
                        AddMessage ("System");
                    }
                    if (bNewValue2) {
                        if (bNewValue) {
                            AppendMessage (" and game");
                        } else {
                            AddMessage ("Game");
                        }
                    }
                    AppendMessage (" command buttons are now repeated");

                } else {

                    AppendMessage ("Command buttons are no longer repeated");
                }

                m_bRepeatedButtons = bNewValue; // Profile Editor is a system page
            }

            // Handle server time display
            if ((pHttpForm = m_pHttpRequest->GetForm ("TimeDisplay")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bNewValue = (iNewValue & SYSTEM_DISPLAY_TIME) != 0;
            bNewValue2 = (iNewValue & GAME_DISPLAY_TIME) != 0;

            bValue = (m_iSystemOptions & SYSTEM_DISPLAY_TIME) != 0;
            bValue2 = (m_iSystemOptions & GAME_DISPLAY_TIME) != 0;

            if ((bNewValue != bValue) || (bNewValue2 != bValue2)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, SYSTEM_DISPLAY_TIME, bNewValue);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = SetEmpireOption (m_iEmpireKey, GAME_DISPLAY_TIME, bNewValue2);
                RETURN_ON_ERROR(iErrCode);

                if (bNewValue) {
                    m_iSystemOptions |= SYSTEM_DISPLAY_TIME;
                } else {
                    m_iSystemOptions &= ~SYSTEM_DISPLAY_TIME;
                }

                if (bNewValue2) {
                    m_iSystemOptions |= GAME_DISPLAY_TIME;
                } else {
                    m_iSystemOptions &= ~GAME_DISPLAY_TIME;
                }

                if (bNewValue || bNewValue2) {
                    AddMessage ("Server time display is now enabled for");
                    if (bNewValue) {
                        AppendMessage (" system");
                    }
                    if (bNewValue2) {
                        if (bNewValue) {
                            AppendMessage (" and");
                        }
                        AppendMessage (" game");
                    }
                    AppendMessage (" screens");

                } else {

                    AppendMessage ("Server time display is disabled for all screens");
                }

                m_bTimeDisplay = bNewValue; // Profile Editor is a system page
            }

            // Handle AutoRefresh
            if ((pHttpForm = m_pHttpRequest->GetForm ("AutoRefresh")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & AUTO_REFRESH) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, AUTO_REFRESH, !bValue);
                RETURN_ON_ERROR(iErrCode);

                AddMessage ("Refresh on update countdown is ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~AUTO_REFRESH;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= AUTO_REFRESH;
                    AppendMessage ("now");
                }
                AppendMessage (" on by default");
            }

            // Handle Countdown
            if ((pHttpForm = m_pHttpRequest->GetForm ("Countdown")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & COUNTDOWN) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, COUNTDOWN, !bValue);
                AddMessage ("Visual update countdown ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~COUNTDOWN;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= COUNTDOWN;
                    AppendMessage ("now");
                }
                AppendMessage (" on by default");
            }

            // Handle map coloring
            if ((pHttpForm = m_pHttpRequest->GetForm ("MapColoring")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & MAP_COLORING) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, MAP_COLORING, !bValue);
                RETURN_ON_ERROR(iErrCode);

                AddMessage ("Map coloring by diplomatic status is ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~MAP_COLORING;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= MAP_COLORING;
                    AppendMessage ("now");
                }
                AppendMessage (" on by default");
            }

            // Handle ship map coloring
            if ((pHttpForm = m_pHttpRequest->GetForm ("ShipMapColoring")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & SHIP_MAP_COLORING) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, SHIP_MAP_COLORING, !bValue);
                RETURN_ON_ERROR(iErrCode);

                AddMessage ("Ship coloring by diplomatic status is ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~SHIP_MAP_COLORING;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= SHIP_MAP_COLORING;
                    AppendMessage ("now");
                }
                AppendMessage (" on by default");
            }

            // Handle ship map highlighting
            if ((pHttpForm = m_pHttpRequest->GetForm ("ShipHighlighting")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & SHIP_MAP_HIGHLIGHTING) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, SHIP_MAP_HIGHLIGHTING, !bValue);
                RETURN_ON_ERROR(iErrCode);

                AddMessage ("Ship highlighting on the map screen is ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~SHIP_MAP_HIGHLIGHTING;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= SHIP_MAP_HIGHLIGHTING;
                    AppendMessage ("now");
                }
                AppendMessage (" on by default");
            }

            // Handle sensitive maps
            if ((pHttpForm = m_pHttpRequest->GetForm ("SensitiveMaps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & SENSITIVE_MAPS) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, SENSITIVE_MAPS, !bValue);
                RETURN_ON_ERROR(iErrCode);

                AddMessage ("Sensitive maps are ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~SENSITIVE_MAPS;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= SENSITIVE_MAPS;
                    AppendMessage ("now");
                }
                AppendMessage (" on by default");
            }

            // Handle partial maps
            if ((pHttpForm = m_pHttpRequest->GetForm ("PartialMaps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & PARTIAL_MAPS) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, PARTIAL_MAPS, !bValue);
                RETURN_ON_ERROR(iErrCode);

                AddMessage ("Partial maps are ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~PARTIAL_MAPS;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= PARTIAL_MAPS;
                    AppendMessage ("now");
                }
                AppendMessage (" on by default");
            }

            // LocalMaps
            if ((pHttpForm = m_pHttpRequest->GetForm ("LocalMaps")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;
            if (bValue != (iNewValue != 0)) {
                iErrCode = SetEmpireOption (m_iEmpireKey, LOCAL_MAPS_IN_UPCLOSE_VIEWS, !bValue);
                RETURN_ON_ERROR(iErrCode);

                AddMessage ("Local maps will ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~LOCAL_MAPS_IN_UPCLOSE_VIEWS;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= LOCAL_MAPS_IN_UPCLOSE_VIEWS;
                    AppendMessage ("now");
                }
                AppendMessage (" be displayed by default in up-close map views");
            }

            // Ratios
            Variant vTemp;
            iErrCode = GetEmpireProperty(m_iEmpireKey, SystemEmpireData::GameRatios, &vTemp);
            RETURN_ON_ERROR(iErrCode);

            iValue = vTemp.GetInteger();

            if ((pHttpForm = m_pHttpRequest->GetForm ("Ratios")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            if (iNewValue != iValue && iNewValue >= RATIOS_DISPLAY_NEVER && iNewValue <= RATIOS_DISPLAY_ALWAYS) {

                iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::GameRatios, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AppendMessage ("Your game ratios line setting was updated");
            }

            // MessageTarget
            if ((pHttpForm = m_pHttpRequest->GetForm ("MessageTarget")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            iErrCode = GetEmpireDefaultMessageTarget (m_iEmpireKey, &iValue);
            RETURN_ON_ERROR(iErrCode);
            if (iValue != iNewValue) {
                iErrCode = SetEmpireDefaultMessageTarget (m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The default message target was updated");
            }

            // TechDesc
            if ((pHttpForm = m_pHttpRequest->GetForm ("TechDesc")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & SHOW_TECH_DESCRIPTIONS) != 0;
            if (bValue != (iNewValue != 0)) {
                iErrCode = SetEmpireOption (m_iEmpireKey, SHOW_TECH_DESCRIPTIONS, !bValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Ship type descriptions maps will ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~SHOW_TECH_DESCRIPTIONS;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= SHOW_TECH_DESCRIPTIONS;
                    AppendMessage ("now");
                }
                AppendMessage (" be displayed on the tech screen");
            }


            // UpCloseShips
            if ((pHttpForm = m_pHttpRequest->GetForm ("UpCloseShips")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & SHIPS_ON_MAP_SCREEN) != 0;
            if (bValue != ((iNewValue & SHIPS_ON_MAP_SCREEN) != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, SHIPS_ON_MAP_SCREEN, !bValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Ship menus will ");
                if (!(iNewValue & SHIPS_ON_MAP_SCREEN)) {
                    m_iSystemOptions &= ~SHIPS_ON_MAP_SCREEN;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= SHIPS_ON_MAP_SCREEN;
                    AppendMessage ("now");
                }
                AppendMessage (" be displayed by default in map page planet views");
            }

            bValue = (m_iSystemOptions & SHIPS_ON_PLANETS_SCREEN) != 0;
            if (bValue != ((iNewValue & SHIPS_ON_PLANETS_SCREEN)!= 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, SHIPS_ON_PLANETS_SCREEN, !bValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Ship menus will ");
                if (!(iNewValue & SHIPS_ON_PLANETS_SCREEN)) {
                    m_iSystemOptions &= ~SHIPS_ON_PLANETS_SCREEN;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= SHIPS_ON_PLANETS_SCREEN;
                    AppendMessage ("now");
                }
                AppendMessage (" be displayed by default on the planets page");
            }

            // UpCloseBuilds
            if ((pHttpForm = m_pHttpRequest->GetForm ("UpCloseBuilds")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & BUILD_ON_MAP_SCREEN) != 0;
            if (bValue != ((iNewValue & BUILD_ON_MAP_SCREEN)!= 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, BUILD_ON_MAP_SCREEN, !bValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Build menus will ");
                if (!(iNewValue & BUILD_ON_MAP_SCREEN)) {
                    m_iSystemOptions &= ~BUILD_ON_MAP_SCREEN;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= BUILD_ON_MAP_SCREEN;
                    AppendMessage ("now");
                }
                AppendMessage (" be displayed by default in map page planet views");
            }

            bValue = (m_iSystemOptions & BUILD_ON_PLANETS_SCREEN) != 0;
            if (bValue != ((iNewValue & BUILD_ON_PLANETS_SCREEN)!= 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, BUILD_ON_PLANETS_SCREEN, !bValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Build menus will ");
                if (!(iNewValue & BUILD_ON_PLANETS_SCREEN)) {
                    m_iSystemOptions &= ~BUILD_ON_PLANETS_SCREEN;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= BUILD_ON_PLANETS_SCREEN;
                    AppendMessage ("now");
                }
                AppendMessage (" be displayed by default on the planets page");
            }
            
            // Handle RefreshUnstarted
            if ((pHttpForm = m_pHttpRequest->GetForm ("RefreshUnstarted")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions2 & REFRESH_UNSTARTED_GAME_PAGES) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption2 (m_iEmpireKey, REFRESH_UNSTARTED_GAME_PAGES, !bValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Unstarted game screens will ");
                if (iNewValue == 0) {
                    m_iSystemOptions2 &= ~REFRESH_UNSTARTED_GAME_PAGES;
                    AppendMessage ("no longer be");
                } else {
                    m_iSystemOptions2 |= REFRESH_UNSTARTED_GAME_PAGES;
                    AppendMessage ("now");
                }
                AppendMessage (" refreshed");
            }

            // Handle displaced End Turn button
            if ((pHttpForm = m_pHttpRequest->GetForm ("DisplaceEndTurn")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & DISPLACE_ENDTURN_BUTTON) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, DISPLACE_ENDTURN_BUTTON, !bValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("The End Turn button will ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~DISPLACE_ENDTURN_BUTTON;
                    AppendMessage ("no longer be");
                } else {
                    m_iSystemOptions |= DISPLACE_ENDTURN_BUTTON;
                    AppendMessage ("now");
                }
                AppendMessage (" displaced by default");
            }

            // Handle fixed backgrounds
            if ((pHttpForm = m_pHttpRequest->GetForm ("FixedBg")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bValue = (m_iSystemOptions & FIXED_BACKGROUNDS) != 0;
            if (bValue != (iNewValue != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, FIXED_BACKGROUNDS, !bValue);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Fixed backgrounds are ");
                if (iNewValue == 0) {
                    m_iSystemOptions &= ~FIXED_BACKGROUNDS;
                    AppendMessage ("no longer");
                } else {
                    m_iSystemOptions |= FIXED_BACKGROUNDS;
                    AppendMessage ("now");
                }
                AppendMessage (" on");
            }

            // Password hashing
            if ((pHttpForm = m_pHttpRequest->GetForm ("Hashing")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            bIPAddress = (m_iSystemOptions & IP_ADDRESS_PASSWORD_HASHING) != 0;
            bSessionId = (m_iSystemOptions & SESSION_ID_PASSWORD_HASHING) != 0;

            if (bIPAddress != ((iNewValue & IP_ADDRESS_PASSWORD_HASHING) != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, IP_ADDRESS_PASSWORD_HASHING, !bIPAddress);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Game screen password hashing with IP address is now ");
                AppendMessage (bIPAddress ? "off" : "on");

                if (!bIPAddress) {
                    m_iSystemOptions |= IP_ADDRESS_PASSWORD_HASHING;
                } else {
                    m_iSystemOptions &= ~IP_ADDRESS_PASSWORD_HASHING;
                }
            }

            if (bSessionId != ((iNewValue & SESSION_ID_PASSWORD_HASHING) != 0)) {

                iErrCode = SetEmpireOption (m_iEmpireKey, SESSION_ID_PASSWORD_HASHING, !bSessionId);
                RETURN_ON_ERROR(iErrCode);
                AddMessage ("Game screen password hashing with Session Id is now ");
                AppendMessage (bIPAddress ? "off" : "on");

                if (!bSessionId) {
                    m_iSystemOptions |= SESSION_ID_PASSWORD_HASHING;
                } else {
                    m_iSystemOptions &= ~SESSION_ID_PASSWORD_HASHING;
                }
            }

            // AdvancedSearch
            if (m_iPrivilege >= PRIVILEGE_FOR_ADVANCED_SEARCH) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("AdvancedSearch")) != NULL) {

                    iNewValue = pHttpForm->GetIntValue();

                    bValue = (m_iSystemOptions & SHOW_ADVANCED_SEARCH_INTERFACE) != 0;
                    if (bValue != (iNewValue != 0)) {

                        iErrCode = SetEmpireOption (m_iEmpireKey, SHOW_ADVANCED_SEARCH_INTERFACE, !bValue);
                        RETURN_ON_ERROR(iErrCode);
                        AddMessage ("The advanced search interface will ");
                        if (iNewValue == 0) {
                            m_iSystemOptions &= ~SHOW_ADVANCED_SEARCH_INTERFACE;
                            AppendMessage ("no longer");
                        } else {
                            m_iSystemOptions |= SHOW_ADVANCED_SEARCH_INTERFACE;
                            AppendMessage ("now");
                        }
                        AppendMessage (" be displayed");
                    }
                }
            }

            // DisplayFatalUpdates
            if ((pHttpForm = m_pHttpRequest->GetForm ("DisplayFatalUpdates")) != NULL) {

                iNewValue = pHttpForm->GetIntValue();

                bValue = (m_iSystemOptions & DISPLAY_FATAL_UPDATE_MESSAGES) != 0;
                if (bValue != (iNewValue != 0)) {

                    iErrCode = SetEmpireOption (m_iEmpireKey, DISPLAY_FATAL_UPDATE_MESSAGES, !bValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Update messages when empire is nuked will ");
                    if (iNewValue == 0) {
                        m_iSystemOptions &= ~DISPLAY_FATAL_UPDATE_MESSAGES;
                        AppendMessage ("no longer");
                    } else {
                        m_iSystemOptions |= DISPLAY_FATAL_UPDATE_MESSAGES;
                        AppendMessage ("now");
                    }
                    AppendMessage (" be sent");
                }
            }

            // SendScore
            if ((pHttpForm = m_pHttpRequest->GetForm ("SendScore")) != NULL) {

                iNewValue = pHttpForm->GetIntValue();

                bValue = (m_iSystemOptions & SEND_SCORE_MESSAGE_ON_NUKE) != 0;
                if (bValue != (iNewValue != 0)) {

                    iErrCode = SetEmpireOption (m_iEmpireKey, SEND_SCORE_MESSAGE_ON_NUKE, !bValue);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("Score change information will ");
                    if (iNewValue == 0) {
                        m_iSystemOptions &= ~SEND_SCORE_MESSAGE_ON_NUKE;
                        AppendMessage ("no longer");
                    } else {
                        m_iSystemOptions |= SEND_SCORE_MESSAGE_ON_NUKE;
                        AppendMessage ("now");
                    }
                    AppendMessage (" be sent when your empire nukes or is nuked");
                }
            }

            // Handle ship name changes
            Variant vOldShipName;
            int iUpdate = 0;

            ENUMERATE_SHIP_TYPES(i) {

                char pszForm[128];
                sprintf(pszForm, "ShipName%i", i);
                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();
                if (pszNewValue == NULL) {
                    pszNewValue = "";
                }

                if (!ShipOrFleetNameFilter (pszNewValue)) {

                    AddMessage ("The new default name for ");
                    AppendMessage (SHIP_TYPE_STRING[i]);
                    AppendMessage (" is illegal");

                } else {

                    if (strlen (pszNewValue) > MAX_SHIP_NAME_LENGTH) {

                        AddMessage ("The new default name for ");
                        AppendMessage (SHIP_TYPE_STRING[i]);
                        AppendMessage (" is too long");

                    } else {

                        iErrCode = GetDefaultEmpireShipName (m_iEmpireKey, i, &vOldShipName);
                        RETURN_ON_ERROR(iErrCode);

                        if (strcmp (vOldShipName.GetCharPtr(), pszNewValue) != 0)
                        {
                            iErrCode = SetDefaultEmpireShipName (m_iEmpireKey, i, pszNewValue);
                            RETURN_ON_ERROR(iErrCode);
                            iUpdate ++;
                        }
                    }
                }
            }

            if (iUpdate > 0) {
                if (iUpdate == 1) {
                    AddMessage ("A default ship name was changed");
                } else {
                    AddMessage ("The default ship names were changed");
                }
            }

            // Handle alien selection request
            if (WasButtonPressed (BID_CHOOSEICON)) {
                m_bRedirectTest = false;

                if ((pHttpForm = m_pHttpRequest->GetForm ("IconSelect")) == NULL) {
                    goto Redirection;
                } else {
                    iProfileEditorPage = 1;
                    iAlienSelect = pHttpForm->GetIntValue();
                }
                break;
            }

            // Handle alien selection request
            if (WasButtonPressed (BID_VIEWMESSAGES)) {
                m_bRedirectTest = false;
                iProfileEditorPage = 2;
                break;
            }

            // Handle theme selection
            if (WasButtonPressed (BID_CHOOSETHEME)) {

                m_bRedirectTest = false;
                if ((pHttpForm = m_pHttpRequest->GetForm ("GraphicalTheme")) == NULL) {
                    goto Redirection;
                }
                iNewValue = pHttpForm->GetIntValue();

                Variant vOldThemeKey;
                switch (iNewValue) {

                case NULL_THEME:

                    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vOldThemeKey);
                    RETURN_ON_ERROR(iErrCode);

                    if (vOldThemeKey.GetInteger() != NULL_THEME)
                    {
                        iErrCode = SetEmpireThemeKey (m_iEmpireKey, NULL_THEME);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = GetUIData (NULL_THEME);
                        RETURN_ON_ERROR(iErrCode);

                        AddMessage ("You have selected the Null Theme");
                    }

                    break;

                case INDIVIDUAL_ELEMENTS:

                    iProfileEditorPage = 3;
                    break;

                case ALTERNATIVE_PATH:
                    iProfileEditorPage = 4;
                    break;

                default:
                    {

                    bool bExist;
                    iErrCode = DoesThemeExist (iNewValue, &bExist);
                    RETURN_ON_ERROR(iErrCode);
                    if (!bExist) {
                        AddMessage ("That theme doesn't exist");
                    } else {

                        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vOldThemeKey);
                        RETURN_ON_ERROR(iErrCode);
                        if (vOldThemeKey.GetInteger() != iNewValue) {

                            iErrCode = SetEmpireThemeKey (m_iEmpireKey, iNewValue);
                            RETURN_ON_ERROR(iErrCode);

                            iErrCode = GetUIData (iNewValue);
                            RETURN_ON_ERROR(iErrCode);

                            AddMessage ("You have selected a new theme");
                        }
                    }

                    }
                    break;
                }
            }

            // Handle delete empire request
            if (WasButtonPressed (BID_DELETEEMPIRE)) {

                if (m_iEmpireKey == global.GetRootKey()) {
                    AddMessage (ROOT_NAME " cannot commit suicide");
                    break;
                }

                else if (m_iEmpireKey == global.GetGuestKey()) {
                    AddMessage (GUEST_NAME " cannot commit suicide");
                    break;
                }

                else {
                    m_bRedirectTest = false;
                    iProfileEditorPage = 5;
                    break;
                }
            }

            // Handle undelete empire request
            if (WasButtonPressed (BID_UNDELETEEMPIRE)) {

                switch (UndeleteEmpire (m_iEmpireKey))
                {
                case ERROR_EMPIRE_DOES_NOT_EXIST:
                    AddMessage ("Your empire no longer exists");
                    break;

                case ERROR_CANNOT_UNDELETE_EMPIRE:
                    AddMessage ("Your empire cannot be undeleted");
                    break;

                case OK:
                    AddMessage ("Your empire is no longer marked for deletion"); 
                    m_iSystemOptions &= ~EMPIRE_MARKED_FOR_DELETION;
                    break;

                default:
                    RETURN_ON_ERROR(iErrCode);
                }
                break;
            }

            // Handle blank empire stats request
            if (WasButtonPressed (BID_BLANKEMPIRESTATISTICS)) {

                if (m_iEmpireKey == global.GetGuestKey()) {
                    AddMessage (GUEST_NAME "'s statistics cannot be blanked");
                } else {
                    m_bRedirectTest = false;
                    iProfileEditorPage = 6;
                }
                break;
            }

            // Handle view tournaments option
            if (WasButtonPressed (BID_VIEWTOURNAMENTINFORMATION)) {

                m_bRedirectTest = false;
                iProfileEditorPage = 9;
                break;
            }

            // Handle association creation
            if (WasButtonPressed (BID_ADD_ASSOCIATION)) {

                m_bRedirectTest = false;
                iProfileEditorPage = 10;
                break;
            }

            // Handle association deletion
            if (WasButtonPressed (BID_REMOVE_ASSOCIATION)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("Association")) == NULL)
                {
                    goto Redirection;
                }

                unsigned int iAssocKey = pHttpForm->GetIntValue();
                if (IS_KEY(iAssocKey))
                {
                    iErrCode = CacheEmpire(iAssocKey);
                    RETURN_ON_ERROR(iErrCode);

                    iErrCode = DeleteAssociation(m_iEmpireKey, iAssocKey);
                    switch (iErrCode)
                    {
                    case OK:
                        AddMessage ("The association was removed");
                        break;

                    case ERROR_ASSOCIATION_NOT_FOUND:
                        AddMessage ("The association no longer exists");
                        break;

                    default:
                        RETURN_ON_ERROR(iErrCode);
                        break;
                    }
                }

                m_bRedirectTest = false;
                break;
            }

            }
            break;

        case 1:
            {

            // An extra I/O, but it almost never happens
            iErrCode = CacheSystemAlienIcons();
            RETURN_ON_ERROR(iErrCode);

            unsigned int iAlienKey;
            bool bHandled;
            iErrCode = HandleIconSelection(&iAlienKey, BASE_UPLOADED_ALIEN_DIR, m_iEmpireKey, NO_KEY, &bHandled);
            RETURN_ON_ERROR(iErrCode);

            if (bHandled)
            {
                if (iAlienKey == UPLOADED_ICON)
                {
                    if (m_iAlienKey != UPLOADED_ICON)
                    {
                        int iAddress;
                        iErrCode = SetEmpireAlienIcon(m_iEmpireKey, UPLOADED_ICON, &iAddress);
                        RETURN_ON_ERROR(iErrCode);

                        m_iAlienKey = UPLOADED_ICON;
                        m_iAlienAddress = iAddress;
                    }
                }
                else if (m_iAlienKey != iAlienKey)
                {
                    int iAddress;
                    iErrCode = SetEmpireAlienIcon(m_iEmpireKey, iAlienKey, &iAddress);
                    RETURN_ON_ERROR(iErrCode);

                    m_iAlienKey = iAlienKey;
                    m_iAlienAddress = iAddress;

                    AddMessage ("Your icon was updated");
                }
                else
                {
                    AddMessage ("You chose the same icon");
                }
            }

            }
            break;

        case 2:
            {

            int iNumTestMessages, iMessageKey, iDeletedMessages = 0;

            // Get number of messages
            if ((pHttpForm = m_pHttpRequest->GetForm ("NumSavedSystemMessages")) == NULL) {
                goto Redirection;
            }
            iNumTestMessages = pHttpForm->GetIntValue();

            // Check for delete all
            if (WasButtonPressed (BID_ALL)) {

                m_bRedirectTest = false;

                for (i = 0; i < iNumTestMessages; i ++) {

                    // Get message key
                    char pszForm [128];
                    sprintf(pszForm, "MsgKey%i", i);
                    if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                        goto Redirection;
                    }
                    iMessageKey = pHttpForm->GetIntValue();

                    // Delete message
                    iErrCode = DeleteSystemMessage(m_iEmpireKey, iMessageKey);
                    RETURN_ON_ERROR(iErrCode);
                    iDeletedMessages ++;
                }

            } else {

                // Check for delete selection
                if (WasButtonPressed (BID_SELECTION)) {

                    m_bRedirectTest = false;

                    for (i = 0; i < iNumTestMessages; i ++) {

                        // Get selected status of message's delete checkbox
                        char pszForm[128];
                        sprintf(pszForm, "DelChBx%i", i);
                        if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) != NULL) {

                            // Get message key
                            sprintf(pszForm, "MsgKey%i", i);
                            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                                goto Redirection;
                            }
                            iMessageKey = pHttpForm->GetIntValue();

                            // Delete message
                            iErrCode = DeleteSystemMessage(m_iEmpireKey, iMessageKey);
                            RETURN_ON_ERROR(iErrCode);
                            iDeletedMessages ++;
                        }
                    }

                } else {

                    // Check for delete system messages
                    char pszForm [128];
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

                            iErrCode = GetSystemMessageProperty(m_iEmpireKey, iMessageKey, SystemEmpireMessages::Flags, &vFlags);
                            RETURN_ON_ERROR(iErrCode);
                            
                            if ((vFlags.GetInteger() & MESSAGE_SYSTEM))
                            {
                                iErrCode = DeleteSystemMessage(m_iEmpireKey, iMessageKey);
                                RETURN_ON_ERROR(iErrCode);
                                iDeletedMessages ++;
                            }
                        }

                    } else {

                        // Check for delete empire message
                        if (WasButtonPressed (BID_DELETEEMPIRE)) {

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

                                iErrCode = GetSystemMessageProperty(m_iEmpireKey, iMessageKey, SystemEmpireMessages::SourceName, &vSource);
                                RETURN_ON_ERROR(iErrCode);
                                
                                if (String::StrCmp (vSource.GetCharPtr(), pszSrcEmpire) == 0)
                                {
                                    iErrCode = DeleteSystemMessage(m_iEmpireKey, iMessageKey);
                                    RETURN_ON_ERROR(iErrCode);
                                    iDeletedMessages ++;
                                }
                            }
                        }
                    }
                }
            }

            if (iDeletedMessages > 0)
            {
                AddMessage (iDeletedMessages);
                AppendMessage (" system message");
                AppendMessage (iDeletedMessages == 1 ? " was deleted" : "s were deleted");
            }

            }
            break;

        case 3:
            {

            unsigned int iNewValue, iLivePlanetKey, iDeadPlanetKey, iColorKey, iThemeKey;
            int iLivePlanetAddress, iDeadPlanetAddress;

            // Handle graphical theme updates
            bool bUpdate = false, bColorError = false;

            iErrCode = GetEmpirePlanetIcons(m_iEmpireKey, &iLivePlanetKey, &iLivePlanetAddress, &iDeadPlanetKey,&iDeadPlanetAddress);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue);
            RETURN_ON_ERROR(iErrCode);

            iColorKey = vValue.GetInteger();

            // Background
            if ((pHttpForm = m_pHttpRequest->GetForm ("Background")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetUIntValue();

            if (iNewValue != m_iBackgroundKey)
            {
                iErrCode = SetEmpireBackgroundKey (m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                m_iBackgroundKey = iNewValue;

                iErrCode = GetThemeAddress(m_iBackgroundKey, &m_iBackgroundAddress);
                RETURN_ON_ERROR(iErrCode);

                bUpdate = true;
            }

            // LivePlanet
            if ((pHttpForm = m_pHttpRequest->GetForm ("LivePlanet")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetUIntValue();

            if (iNewValue != iLivePlanetKey)
            {
                iErrCode = SetEmpireLivePlanetKey(m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                bUpdate = true;
            }

            // DeadPlanet
            if ((pHttpForm = m_pHttpRequest->GetForm ("DeadPlanet")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetUIntValue();

            if (iNewValue != iDeadPlanetKey) {
                iErrCode = SetEmpireDeadPlanetKey (m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                bUpdate = true;
            }

            // Button
            if ((pHttpForm = m_pHttpRequest->GetForm ("Button")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetUIntValue();

            if (iNewValue != m_iButtonKey) {
                iErrCode = SetEmpireButtonKey (m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                iNewButtonKey = iNewValue;
                bUpdate = true;
            }

            // Separator
            if ((pHttpForm = m_pHttpRequest->GetForm ("Separator")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetUIntValue();

            if (iNewValue != m_iSeparatorKey) {
                iErrCode = SetEmpireSeparatorKey (m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                bUpdate = true;
            }

            // Get horz, vert keys
            unsigned int iHorzKey, iVertKey;
            
            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIHorz, &vValue);
            RETURN_ON_ERROR(iErrCode);
            iHorzKey = vValue.GetInteger();

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIVert, &vValue);
            RETURN_ON_ERROR(iErrCode);
            iVertKey = vValue.GetInteger();

            // Horz
            if ((pHttpForm = m_pHttpRequest->GetForm ("Horz")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetUIntValue();

            if (iNewValue != iHorzKey) {
                iErrCode = SetEmpireHorzKey (m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                bUpdate = true;
            }

            // Vert
            if ((pHttpForm = m_pHttpRequest->GetForm ("Vert")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetUIntValue();

            if (iNewValue != iVertKey) {
                iErrCode = SetEmpireVertKey (m_iEmpireKey, iNewValue);
                RETURN_ON_ERROR(iErrCode);
                bUpdate = true;
            }

            // Color
            if ((pHttpForm = m_pHttpRequest->GetForm ("Color")) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetUIntValue();

            if (iNewValue == CUSTOM_COLORS) {

                const char* pszNewValue;

                // Update text color
                if ((pHttpForm = m_pHttpRequest->GetForm ("CustomTextColor")) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();

                if (IsColor (pszNewValue)) {
                    m_vTableColor = pszNewValue;
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTextColor, m_vTableColor);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    AddMessage ("You must submit a valid text color");
                    bColorError = true;
                }

                // Update good color
                if ((pHttpForm = m_pHttpRequest->GetForm ("CustomGoodColor")) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();

                if (IsColor (pszNewValue)) {
                    m_vGoodColor = pszNewValue;
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomGoodColor, m_vGoodColor);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    AddMessage ("You must submit a valid good color");
                    bColorError = true;
                }

                // Update bad color
                if ((pHttpForm = m_pHttpRequest->GetForm ("CustomBadColor")) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();

                if (IsColor (pszNewValue)) {
                    m_vBadColor = pszNewValue;
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBadColor, m_vBadColor);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    AddMessage ("You must submit a valid bad color");
                    bColorError = true;
                }

                // Update private message color
                if ((pHttpForm = m_pHttpRequest->GetForm ("CustomMessageColor")) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();

                if (IsColor (pszNewValue)) {
                    m_vPrivateMessageColor = pszNewValue;
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomPrivateMessageColor, m_vPrivateMessageColor);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    AddMessage ("You must submit a valid message color");
                    bColorError = true;
                }

                // Update broadcast message color
                if ((pHttpForm = m_pHttpRequest->GetForm ("CustomBroadcastColor")) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();

                if (IsColor (pszNewValue)) {
                    m_vBroadcastMessageColor = pszNewValue;
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBroadcastMessageColor, m_vBroadcastMessageColor);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    AddMessage ("You must submit a valid broadcast color");
                    bColorError = true;
                }

                // Update table color
                if ((pHttpForm = m_pHttpRequest->GetForm ("CustomTableColor")) == NULL) {
                    goto Redirection;
                }
                pszNewValue = pHttpForm->GetValue();

                if (IsColor (pszNewValue)) {
                    m_vTableColor = pszNewValue;
                    iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTableColor, m_vTableColor);
                    RETURN_ON_ERROR(iErrCode);
                } else {
                    AddMessage ("You must submit a valid table color");
                    bColorError = true;
                }
            }

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vValue);
            RETURN_ON_ERROR(iErrCode);
            iThemeKey = vValue.GetInteger();

            if (iNewValue != iColorKey) {

                if (!bColorError) {
                    iErrCode = SetEmpireColorKey (m_iEmpireKey, iNewValue);
                    RETURN_ON_ERROR(iErrCode);
                    bUpdate = true;
                } else {

                    // We need a color key from somewhere - use the previous theme
                    if (iThemeKey != INDIVIDUAL_ELEMENTS && iThemeKey != ALTERNATIVE_PATH) {
                        iErrCode = SetEmpireColorKey (m_iEmpireKey, iThemeKey);
                        RETURN_ON_ERROR(iErrCode);
                    }

                    // No need for an else;  we'll keep using what we had before
                }
            }

            if (bUpdate) {
                if (iThemeKey != INDIVIDUAL_ELEMENTS) {
                    iErrCode = SetEmpireThemeKey (m_iEmpireKey, INDIVIDUAL_ELEMENTS);
                    RETURN_ON_ERROR(iErrCode);
                    AddMessage ("You are now using individual UI elements");
                } else {
                    AddMessage ("Your individual UI elements have been updated");
                }
                
                iErrCode = GetUIData(INDIVIDUAL_ELEMENTS);
                RETURN_ON_ERROR(iErrCode);
            }

            const char* pszStart;
            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ThemeInfo")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "ThemeInfo%d", &iInfoThemeKey) == 1) {

                iProfileEditorPage = 7;
                m_bRedirectTest = false;
                break;
            }

            }
            break;

        case 4:

            if (WasButtonPressed (BID_CHOOSE)) {

                if ((pHttpForm = m_pHttpRequest->GetForm ("GraphicsPath")) == NULL) {
                    goto Redirection;
                }

                pszGraphicsPath = pHttpForm->GetValue();
                if (pszGraphicsPath == NULL) {
                    AddMessage ("You must submit a valid alternative graphics path");
                    iProfileEditorPage = 0;
                } else {
                    iProfileEditorPage = 8;
                }

                break;
            }
            break;

        case 8:
            {

            if (WasButtonPressed (BID_CHOOSE)) {

                // Read the path
                if ((pHttpForm = m_pHttpRequest->GetForm ("GraphicsPath")) == NULL) {
                    goto Redirection;
                }

                const char* pszPath = pHttpForm->GetValue();

                if (pszPath == NULL) {
                    AddMessage ("You must submit a valid alternative graphics path");
                    iProfileEditorPage = 0;
                } else {

                    size_t stLength = strlen (pszPath);

                    if (stLength > MAX_GRAPHICS_ALTERNATIVE_PATH_LENGTH) {
                        AddMessage ("Your alternative graphics path is too long");
                    } else {
                        iErrCode = SetEmpireThemeKey (m_iEmpireKey, ALTERNATIVE_PATH);
                        RETURN_ON_ERROR(iErrCode);
                        
                        iErrCode = SetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlternativeGraphicsPath, pszPath);
                        RETURN_ON_ERROR(iErrCode);

                        iErrCode = GetUIData (ALTERNATIVE_PATH);
                        RETURN_ON_ERROR(iErrCode);
                    }
                }
            }

            }

            break;

        case 5:
            {

            char pszText [MAX_EMPIRE_NAME_LENGTH + 256];
            sprintf(pszText, "%s requested to be deleted", m_vEmpireName.GetCharPtr());
            global.WriteReport(TRACE_INFO, pszText);

            iErrCode = CacheEmpireForDeletion(m_iEmpireKey);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = DeleteEmpire(m_iEmpireKey, NULL, true, false);
            switch (iErrCode)
            {
            case OK:
                AddMessage ("The empire ");
                AppendMessage (m_vEmpireName.GetCharPtr());
                AppendMessage (" was deleted");
                return Redirect (LOGIN);

            case ERROR_EMPIRE_IS_IN_GAMES:
                AddMessage ("Your empire is still in at least one game. It will be deleted when it is no longer in any games");
                AddMessage ("Your personal information has been cleared");
                m_iSystemOptions |= EMPIRE_MARKED_FOR_DELETION;

                sprintf(pszText, "%s was marked for deletion", m_vEmpireName.GetCharPtr());
                global.WriteReport(TRACE_INFO, pszText);

                break;

            case ERROR_EMPIRE_DOES_NOT_EXIST:
                AddMessage ("The empire ");
                AppendMessage (m_vEmpireName.GetCharPtr());
                AppendMessage (" no longer exists");
                return Redirect (LOGIN);

            default:
                RETURN_ON_ERROR(iErrCode);
                break;
            }

            }
            break;

        case 6:

            iErrCode = BlankEmpireStatistics (m_iEmpireKey);
            RETURN_ON_ERROR(iErrCode);
            AddMessage ("Your empire's statistics have been blanked");

            char pszText [MAX_EMPIRE_NAME_LENGTH + 256];
            sprintf(pszText, "%s statistics were blanked", m_vEmpireName.GetCharPtr());
            global.WriteReport(TRACE_INFO, pszText);

            break;

        case 7:

            // Nothing to do
            break;

        case 9:

            {
            const char* pszStart = NULL;

            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewTourneyInfo")) != NULL && 
                (pszStart = pHttpForm->GetName()) != NULL &&
                sscanf (pszStart, "ViewTourneyInfo%d", &m_iReserved) == 1) {
                return Redirect (TOURNAMENTS);
            }

            }
            break;

        case 10:

            if (WasButtonPressed (BID_ADD_ASSOCIATION)) {

                const char* pszName = NULL, * pszPass = NULL;

                pHttpForm = m_pHttpRequest->GetForm ("AssocName");
                if (pHttpForm != NULL) {
                    pszName = pHttpForm->GetValue();
                }

                if (pszName != NULL) {

                    pHttpForm = m_pHttpRequest->GetForm ("AssocPass");
                    if (pHttpForm != NULL) {
                        pszPass = pHttpForm->GetValue();
                    }

                    if (pszPass == NULL) {
                        iErrCode = ERROR_PASSWORD;
                    } else {
                        iErrCode = CreateAssociation (m_iEmpireKey, pszName, pszPass);
                    }

                    switch (iErrCode)
                    {
                    case OK:
                        AddMessage ("A new empire association was created");
                        iProfileEditorPage = 0;
                        break;

                    case ERROR_PASSWORD:
                        AddMessage ("That was not the right password");
                        iProfileEditorPage = 10;
                        break;

                    case ERROR_ASSOCIATION_ALREADY_EXISTS:
                        AddMessage ("That association already exists");
                        iProfileEditorPage = 10;
                        break;

                    case ERROR_EMPIRE_DOES_NOT_EXIST:
                        AddMessage ("That empire does not exist");
                        iProfileEditorPage = 10;
                        break;

                    case ERROR_DUPLICATE_EMPIRE:
                        AddMessage ("Cannot create an association with oneself");
                        iProfileEditorPage = 10;
                        break;

                    default:
                        RETURN_ON_ERROR(iErrCode);
                        break;
                    }
                }
            }

            break;

        default:
            Assert(false);
        }
    }   // End if not cancel
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
        m_iButtonKey = iNewButtonKey;
        iErrCode = GetThemeAddress(m_iButtonKey, &m_iButtonAddress);
        RETURN_ON_ERROR(iErrCode);
        return Redirect(pageRedirect);
    }
}

iNewButtonKey = m_iButtonKey;

iErrCode = OpenSystemPage(iProfileEditorPage == 1 && iAlienSelect == 1);
RETURN_ON_ERROR(iErrCode);

// Individual page stuff starts here
switch (iProfileEditorPage) {

case 0:
    {

    Variant* pvEmpireData = NULL, vMaxNumSystemMessages;
    AutoFreeData free_pvEmpireData(pvEmpireData);

    int iOptions, iValue, j, iMaxNumSystemMessages;
    bool bIP, bID, bFlag;
    size_t stLen;

    unsigned int iNumTournamentsJoined, iNumSystemMessages;

    String strFilter;

    iErrCode = GetEmpireData (m_iEmpireKey, &pvEmpireData, NULL);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetJoinedTournaments (m_iEmpireKey, NULL, NULL, &iNumTournamentsJoined);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="ProfileEditorPage" value="0"><p><%

    iErrCode = WriteProfileAlienString (
        pvEmpireData[SystemEmpireData::iAlienKey].GetInteger(),
        pvEmpireData[SystemEmpireData::iAlienAddress].GetInteger(),
        m_iEmpireKey,
        m_vEmpireName.GetCharPtr(),
        0,
        "ProfileLink",
        "View your profile",
        false,
        false
        );
    RETURN_ON_ERROR(iErrCode);

    %> <font size="+2"><%
    Write (m_vEmpireName.GetCharPtr());
    %></font><p><%

    %><table width="90%"><%

    %><tr><td align="center" colspan="2"><h3>Empire Information:</h3></td></tr><%

    %><tr><td align="left">Recase empire name:</td><%
    %><td align="left"><input type="text" name="RecasedEmpireName" size="<%
    stLen = strlen (m_vEmpireName.GetCharPtr());
    Write ((int64)stLen); %>" maxlength="<% Write ((int64)stLen); 
    %>" value="<% Write (m_vEmpireName.GetCharPtr()); %>"></td></tr><%

    if (m_iEmpireKey != global.GetGuestKey()) {

        %><tr><td align="left">Password:</td><%
        %><td align="left"><input type="password" name="NewPassword" size="20" maxlength="<%
            Write(MAX_EMPIRE_PASSWORD_LENGTH); %>" value="<% Write(INVALID_PASSWORD_STRING); %>"></td></tr><%

        %><tr><td align="left">Verify password:</td><%
        %><td align="left"><input type="password" name="VerifyPassword" size="20" maxlength="<% 
            Write(MAX_EMPIRE_PASSWORD_LENGTH); %>" value="<% Write(INVALID_PASSWORD_STRING); %>"></td></tr><%
    }

    HTMLFilter (pvEmpireData[SystemEmpireData::iRealName].GetCharPtr(), &strFilter, 0, false);

    %><tr><td align="left">Real name:</td><%
    %><td><input type="text" name="RealName" size="40" maxlength="<% 
        Write (MAX_REAL_NAME_LENGTH); %>" value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength());
    %>"></td></tr><%


    // Age
    iValue = pvEmpireData[SystemEmpireData::iAge].GetInteger();

    %><tr><td align="left">Age:</td><%
    %><td><select name="EmpAge"><%

    %><option<%
    if (iValue == EMPIRE_AGE_UNKNOWN) {
        %> selected<%
    }
    %> value="<% Write (EMPIRE_AGE_UNKNOWN); %>">N/A</option><%

    for (i = EMPIRE_AGE_MINIMUM; i <= EMPIRE_AGE_MAXIMUM; i ++) {
        %><option<%
        if (iValue == i) {
            %> selected<%
        }
        %> value="<% Write (i); %>"><% Write (i); %></option><%
    }

    %></select></td></tr><%


    // Gender
    iValue = pvEmpireData[SystemEmpireData::iGender].GetInteger();

    %><tr><td align="left">Gender:</td><%
    %><td><select name="EmpGender"><%

    for (i = 0; i < EMPIRE_NUM_GENDERS; i ++) {

        %><option<%
        if (iValue == EMPIRE_GENDER[i]) {
            %> selected<%
        }
        %> value="<% Write (EMPIRE_GENDER[i]); %>"><% Write (EMPIRE_GENDER_STRING[EMPIRE_GENDER[i]]); %></option><%
    }
    %></select></td></tr><%

    // Location
    HTMLFilter (pvEmpireData[SystemEmpireData::iLocation].GetCharPtr(), &strFilter, 0, false);
    %><tr><td align="left">Location:</td><%
    %><td><input type="text" name="Location" size="40" maxlength="<% 
        Write (MAX_LOCATION_LENGTH); %>" value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength());
    %>"></td></tr><%

    HTMLFilter (pvEmpireData[SystemEmpireData::iEmail].GetCharPtr(), &strFilter, 0, false);
    %><tr><td align="left">E-mail address:</td><%
    %><td><input type="text" name="Email" size="40" maxlength="<%
        Write (MAX_EMAIL_LENGTH); %>" value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength());
    %>"></td></tr><% 

    HTMLFilter (pvEmpireData[SystemEmpireData::iPrivateEmail].GetCharPtr(), &strFilter, 0, false);
    %><tr><td align="left">Private e-mail address (<i>only visible to administrators</em>):</td><%
    %><td><input type="text" name="PrivEmail" size="40" maxlength="<%
        Write (MAX_EMAIL_LENGTH); %>" value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength());
    %>"></td></tr><% 

    HTMLFilter (pvEmpireData[SystemEmpireData::iIMId].GetCharPtr(), &strFilter, 0, false);
    %><tr><td align="left">Instant Messenger:</td><%
    %><td><input type="text" name="IMId" size="40" maxlength="<% 
        Write (MAX_IMID_LENGTH); %>" value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength());
    %>"></td></tr><%

    HTMLFilter (pvEmpireData[SystemEmpireData::iWebPage].GetCharPtr(), &strFilter, 0, false);
    %><tr><td align="left">Webpage:</td><%
    %><td><input type="text" name="WebPage" size="60" maxlength="<% 
        Write (MAX_WEB_PAGE_LENGTH); %>" value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength());
    %>"></td></tr><%

    %><tr><td>Tournaments joined:</td><%
    %><td><%

    if (iNumTournamentsJoined == 0) {
        %>You have not joined any tournaments<%
    } else {
        %>You have joined <strong><% Write (iNumTournamentsJoined); %></strong> tournament<%
        if (iNumTournamentsJoined != 1) {
            %>s<%
        }

        %>&nbsp&nbsp<% WriteButton (BID_VIEWTOURNAMENTINFORMATION);
    }
    %></td></tr><%


    %><tr><td>Tournament game availability:</td><td><select name="TourneyAvail"><%

    %><option<%
    if (!(m_iSystemOptions2 & UNAVAILABLE_FOR_TOURNAMENTS)) {
        %> selected<%
    }
    %> value="1">Available to be entered into tournament games</option><%

    %><option<%
    if (m_iSystemOptions2 & UNAVAILABLE_FOR_TOURNAMENTS) {
        %> selected<%
    }
    %> value="0">Not available to be entered into tournament games</option><%

    %></select></td></tr><%


    %><tr><td>Empire autologon (<em>uses cookies; only for private machines)</em>:</td><td><select name="AutoLogon"><%

    %><option value="<% Write (m_iEmpireKey); %>"><%
    %>Use the current empire (<% Write (m_vEmpireName.GetCharPtr()); %>) to autologon</option><%

    if (iAutoLogonSelected == MAYBE_AUTOLOGON)
    {
        unsigned int iAutoLogonKey = NO_KEY;
        ICookie* pCookie = m_pHttpRequest->GetCookie(AUTOLOGON_EMPIREKEY_COOKIE);
        if (pCookie && pCookie->GetValue())
        {
            iAutoLogonKey = pCookie->GetUIntValue();
            if (iAutoLogonKey != m_iEmpireKey)
            {
                iErrCode = CacheEmpire(iAutoLogonKey);
                RETURN_ON_ERROR(iErrCode);

                Variant vName;
                iErrCode = GetEmpireName(iAutoLogonKey, &vName);
                if (iErrCode == ERROR_EMPIRE_DOES_NOT_EXIST)
                {
                    iAutoLogonKey = NO_KEY;
                }
                else
                {
                    RETURN_ON_ERROR(iErrCode);

                    %><option selected value="<% Write (iAutoLogonKey); %>">Use another empire (<% Write(vName.GetCharPtr()); %>) to autologon</option><%
                }
            }

            iAutoLogonSelected = iAutoLogonKey;
        }
    }

    %><option <%
    if (iAutoLogonSelected == NO_AUTOLOGON || iAutoLogonSelected == MAYBE_AUTOLOGON) {
        iAutoLogonSelected = NO_AUTOLOGON;
        %>selected <%
    }
    %>value="<% Write (NO_AUTOLOGON); %>">Do not autologon</option></select><%
    %><input type="hidden" name="AutoLogonSel" value="<% Write (iAutoLogonSelected); %>"></td></tr><%

    %><tr><td>Empire associations:</td><td><%
    
    WriteButton (BID_ADD_ASSOCIATION);

    Variant* pvAssoc = NULL;
    AutoFreeData free_pvAssoc(pvAssoc);

    unsigned int iAssoc;
    iErrCode = GetAssociations(m_iEmpireKey, &pvAssoc, &iAssoc);
    RETURN_ON_ERROR(iErrCode);

    if (iAssoc > 0)
    {
        iErrCode = CacheEmpires(pvAssoc, iAssoc);
        RETURN_ON_ERROR(iErrCode);

        char pszName[MAX_EMPIRE_NAME_LENGTH + 1];

        %> <%

        if (iAssoc == 1)
        {
            iErrCode = GetEmpireName(pvAssoc[0].GetInteger(), pszName);
            RETURN_ON_ERROR(iErrCode);

            Write (pszName);
            %><input type="hidden" name="Association" value="<% Write (pvAssoc[0].GetInteger()); %>"><%
        }
        else
        {
            %><select name="Association"><%
            for (unsigned int a = 0; a < iAssoc && iErrCode == OK; a ++)
            {
                iErrCode = GetEmpireName(pvAssoc[a].GetInteger(), pszName);
                RETURN_ON_ERROR(iErrCode);
                
                %><option value="<% Write (pvAssoc[a].GetInteger()); %>"><% Write (pszName); %></option><%
            }
            %></select><%
        }

        %> <%

        WriteButton(BID_REMOVE_ASSOCIATION);
    }

    %></td></tr><%

    %><tr><td align="center" colspan="2">&nbsp;</td></tr><%
    %><tr><td align="center" colspan="2"><h3>System User Interface:</h3></td></tr><%

    %><tr><td>Choose an icon:</td><td><%

    %><select name="IconSelect"><%

    if (pvEmpireData[SystemEmpireData::iAlienKey].GetInteger() == UPLOADED_ICON) {
        %><option value="0">An icon from the system set</option><%
        %><option selected value="1">An uploaded icon</option><% 
    } else {
        %><option selected value="0">An icon from the system set</option><%
        %><option value="1">An uploaded icon</option><%
    }

    %></select> <%

    WriteButton (BID_CHOOSEICON); %></td></tr><%
        

    %><tr><td>Almonaster graphical theme:</td><td><select name="GraphicalTheme"><option<%

    if (pvEmpireData[SystemEmpireData::iAlmonasterTheme].GetInteger() == INDIVIDUAL_ELEMENTS) { 
        %> selected<%
    } %> value="<% Write (INDIVIDUAL_ELEMENTS); %>">Individual Graphical Elements</option><%

    %><option<% if (pvEmpireData[SystemEmpireData::iAlmonasterTheme].GetInteger() == ALTERNATIVE_PATH) {
        %> selected<%
    } %> value="<% Write (ALTERNATIVE_PATH); %>">Graphics from an alternative path</option><%

    %><option<% if (pvEmpireData[SystemEmpireData::iAlmonasterTheme].GetInteger() == NULL_THEME) {
        %> selected<%
    } %> value="<% Write (NULL_THEME); %>">Null Theme</option><%

    unsigned int* piThemeKey = NULL, iNumThemes;
    AutoFreeKeys free_piThemeKey(piThemeKey);

    iErrCode = GetFullThemeKeys (&piThemeKey, &iNumThemes);
    RETURN_ON_ERROR(iErrCode);

    if (iNumThemes > 0) {

        Variant vThemeName;
        for (i = 0; i < (int)iNumThemes; i ++)
        {
            iErrCode = GetThemeName (piThemeKey[i], &vThemeName);
            RETURN_ON_ERROR(iErrCode);
            
            %><option <%
            if (pvEmpireData[SystemEmpireData::iAlmonasterTheme].GetInteger() == (int)piThemeKey[i]) {
                %> selected<%
            }
            %> value="<% Write (piThemeKey[i]); %>"><% Write (vThemeName.GetCharPtr()); %></option><%
        }
    }
    %></select> <%

    WriteButton (BID_CHOOSETHEME); 
    %></td></tr><%


    %><tr><td>Placement of command buttons:</td><td><select name="RepeatedButtons"><%

    iValue = pvEmpireData[SystemEmpireData::iOptions].GetInteger() & (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS);

    %><option<%
    if (iValue == 0) {
        %> selected<%
    } %> value="0"><%
    %>At top of screen only</option><%

    %><option<%
    if (iValue == (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS)) {
        %> selected<%
    } %> value="<% Write (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS); %>"><%
    %>At top and bottom of screen on both system and game screens by default</option><%

    %><option<%
    if (iValue == SYSTEM_REPEATED_BUTTONS) {
        %> selected<%
    } %> value="<% Write (SYSTEM_REPEATED_BUTTONS); %>"><%
    %>At top and bottom of screen only on system screens</option><%

    %><option<%
    if (iValue == GAME_REPEATED_BUTTONS) {
        %> selected<%
    } %> value="<% Write (GAME_REPEATED_BUTTONS); %>"><%
    %>At top and bottom of screen only on game screens by default</option><%

    %></select></td></tr><%


    %><tr><td>Display server time:</td><td><select name="TimeDisplay"><%

    iValue = pvEmpireData[SystemEmpireData::iOptions].GetInteger() & (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME);

    %><option<%
    if (iValue == (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME)) {
        %> selected<%
    } %> value="<% Write (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME); %>"><%
    %>On both system screens and on game screens by default</option><%

    %><option<%
    if (iValue == SYSTEM_DISPLAY_TIME) {
        %> selected<%
    } %> value="<% Write (SYSTEM_DISPLAY_TIME); %>"><%
    %>Only on system screens</option><%

    %><option<%
    if (iValue == GAME_DISPLAY_TIME) {
        %> selected<%
    } %> value="<% Write (GAME_DISPLAY_TIME); %>"><%
    %>Only on game screens by default</option><%

    %><option<%
    if (iValue == 0) {
        %> selected<%
    } %> value="0"><%
    %>On neither system screens nor game screens by default</option><%

    %></select></td></tr><%
    
    
    %><tr><td>System messages saved:<td><%

    iErrCode = GetNumSystemMessages (m_iEmpireKey, &iNumSystemMessages);
    RETURN_ON_ERROR(iErrCode);

    if (iNumSystemMessages > 0) {
        Write (iNumSystemMessages);
        %>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp<%
        WriteButton (BID_VIEWMESSAGES);
    } else {
        %>None<%
    }

    %></tr><tr><td>Maximum saved system messages:</td><td><select name="MaxNumSavedMessages"><%

    iErrCode = GetSystemProperty (SystemData::MaxNumSystemMessages, &vMaxNumSystemMessages);
    RETURN_ON_ERROR(iErrCode);
    iMaxNumSystemMessages = vMaxNumSystemMessages.GetInteger();

    for (i = 0; i <= iMaxNumSystemMessages; i += 10) {
        %><option<%
        if (pvEmpireData[SystemEmpireData::iMaxNumSystemMessages].GetInteger() == i) {
            %> selected <%
        } %> value="<% Write (i); %>"><% Write (i); %></option><%
    }
    %></select></td></tr><%


    %><tr><td>Fixed backgrounds <em>(requires Internet Explorer)</em>:</td><%
    %><td><select name="FixedBg"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & FIXED_BACKGROUNDS) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off</option></select></td></tr><%


    %><tr><td>Block uploaded icons from other empires:</td><td><select name="BlockUploadedIcons"><%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions2].GetInteger() & BLOCK_UPLOADED_ICONS) != 0;

    %><option<%
    if (!bFlag) {
        %> selected<%
    } %> value="0">Display uploaded icons</option><%

    %><option<%
    if (bFlag) {
        %> selected<%
    } %> value="1">Display default icon instead of uploaded icons</option><%
   
    %></select></td></tr><%


    if (m_iPrivilege >= PRIVILEGE_FOR_ADVANCED_SEARCH) {

        %><tr><td>Profile Viewer search interface:</td><td><select name="AdvancedSearch"><option<%

        bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & SHOW_ADVANCED_SEARCH_INTERFACE) != 0;

        if (bFlag) {
            %> selected<%
        } %> value="1">Display advanced search interface</option><option<%

        if (!bFlag) {
            %> selected<%
        } %> value="0">Display simple search interface</option></select></td></tr><%
    }
    
    
    %><tr><td>Prompt for confirmation on important decisions:</td><td><select name="Confirm"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & CONFIRM_IMPORTANT_CHOICES) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">Always confirm on important decisions</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Never confirm on important decisions</option></select></td></tr><%


    %><tr><td align="center" colspan="2">&nbsp;</td></tr><%
    %><tr><td align="center" colspan="2"><h3>Game User Interface:</h3></td></tr><%

    %><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><%
    %><td><select name="AutoRefresh"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & AUTO_REFRESH) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><%
    %><td><select name="Countdown"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & COUNTDOWN) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Refresh unstarted game screens every 2 min <em>(requires JavaScript)</em>:</td><%
    %><td><select name="RefreshUnstarted"><%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions2].GetInteger() & REFRESH_UNSTARTED_GAME_PAGES) != 0;

    %><option<%
    if (bFlag) {
        %> selected<%
    } %> value="1">On in all unstarted games</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off</option></select></td></tr><%
    
    
    %><tr><td>Displace End Turn button to corner:</td><td><select name="DisplaceEndTurn"><%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & DISPLACE_ENDTURN_BUTTON) != 0;

    %><option<%
    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Map coloring by diplomatic status:</td><td><select name="MapColoring"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & MAP_COLORING) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Ship coloring by diplomatic status on map screen:</td><td><select name="ShipMapColoring"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & SHIP_MAP_COLORING) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Ship highlighting on map screen:</td><td><select name="ShipHighlighting"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & SHIP_MAP_HIGHLIGHTING) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><%
    %><td><select name="SensitiveMaps"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & SENSITIVE_MAPS) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Partial maps:</td><td><select name="PartialMaps"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & PARTIAL_MAPS) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Display local maps in up-close map views:</td><td><select name="LocalMaps"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">On by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Off by default</option></select></td></tr><%


    %><tr><td>Display ship menus in planet views:</td><td><select name="UpCloseShips"><%

    iOptions = pvEmpireData[SystemEmpireData::iOptions].GetInteger() & (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN);

    %><option <% if (iOptions == (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN)) { %>selected <% }
    %>value="<% Write (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN); %>"><%
    %>Ship menus on both map and planets screens by default</option><%

    %><option <% if (iOptions == SHIPS_ON_MAP_SCREEN) { %>selected <% }
    %>value="<% Write (SHIPS_ON_MAP_SCREEN); %>"><%
    %>Ship menus on map screens by default</option><%

    %><option <% if (iOptions == SHIPS_ON_PLANETS_SCREEN) { %>selected <% }
    %>value="<% Write (SHIPS_ON_PLANETS_SCREEN); %>"><%
    %>Ship menus on planets screen by default</option><%

    %><option <% if (iOptions == 0) { %>selected <% }
    %>value="0"><%
    %>No ship menus in planet views by default</option><%

    %></select></td></tr><%


    %><tr><td>Display build menus in planet views:</td><td><select name="UpCloseBuilds"><%

    iOptions = pvEmpireData[SystemEmpireData::iOptions].GetInteger() & (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN);

    %><option <% if (iOptions == (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN)) { %>selected <% }
    %>value="<% Write (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN); %>"><%
    %>Build menus on both map and planets screens by default</option><%

    %><option <% if (iOptions == BUILD_ON_MAP_SCREEN) { %>selected <% }
    %>value="<% Write (BUILD_ON_MAP_SCREEN); %>"><%
    %>Build menus on map screens by default</option><%

    %><option <% if (iOptions == BUILD_ON_PLANETS_SCREEN) { %>selected <% }
    %>value="<% Write (BUILD_ON_PLANETS_SCREEN); %>"><%
    %>Build menus on planets screen by default</option><%

    %><option <% if (iOptions == 0) { %>selected <% }
    %>value="0"><%
    %>No build menus in planet views by default</option><%

    %></select></td></tr><%


    %><tr><td>Show ship type descriptions in tech screen:</td><td><select name="TechDesc"><%

    %><option<%
    if (m_iSystemOptions & SHOW_TECH_DESCRIPTIONS) {
        %> selected<%
    }
    %> value="1">Show ship type descriptions</option><%

    %><option<%
    if (!(m_iSystemOptions & SHOW_TECH_DESCRIPTIONS)) {
        %> selected<%
    }
    %> value="0">Don't show ship type descriptions</option><%

    %></select></td></tr><%


    %><tr><td>Display ratios line:</td><td><select name="Ratios"><%

    %><option<%
    if (pvEmpireData[SystemEmpireData::iGameRatios].GetInteger() == RATIOS_DISPLAY_NEVER) {
        %> selected<%
    } %> value="<% Write (RATIOS_DISPLAY_NEVER); %>">Never by default</option><%

    %><option<%
    if (pvEmpireData[SystemEmpireData::iGameRatios].GetInteger() == RATIOS_DISPLAY_ON_RELEVANT_SCREENS) {
        %> selected<%
    } %> value="<% Write (RATIOS_DISPLAY_ON_RELEVANT_SCREENS); %>">On relevant game screens by default</option><%

    %><option<%
    if (pvEmpireData[SystemEmpireData::iGameRatios].GetInteger() == RATIOS_DISPLAY_ALWAYS) {
        %> selected<%
    } %> value="<% Write (RATIOS_DISPLAY_ALWAYS); %>">On all game screens by default</option><%

    %></select></td></tr><%


    %><tr><td>Game screen password hashing:<br><%
    %>(<em>IP address hashing can conflict with firewalls</em>)</td><%
    %><td><select name="Hashing"><option<%

    bIP = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & IP_ADDRESS_PASSWORD_HASHING) != 0;
    bID = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & SESSION_ID_PASSWORD_HASHING) != 0;

    if (bIP && bID) {
        %> selected<%
    } %> value="<% Write (IP_ADDRESS_PASSWORD_HASHING | SESSION_ID_PASSWORD_HASHING); %>"><%
    %>Use both IP address and Session Id</option><option<%

    if (bIP && !bID) {
        %> selected<%
    } %> value="<% Write (IP_ADDRESS_PASSWORD_HASHING); %>">Use only IP address</option><option<%

    if (bID && !bIP) {
        %> selected<%
    } %> value="<% Write (SESSION_ID_PASSWORD_HASHING); %>">Use only Session Id</option><option<%

    if (!bIP && !bID) {
        %> selected<%
    } %> value="0">Use neither (insecure)</option></select></td></tr><%


    %><tr><td align="center" colspan="2">&nbsp;</td></tr><%
    %><tr><td align="center" colspan="2"><h3>Gameplay:</h3></td></tr><%

    iValue = pvEmpireData[SystemEmpireData::iDefaultMessageTarget].GetInteger();

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

    %><option<%
    if (iValue == MESSAGE_TARGET_TRUCE) {
        %> selected<%
    }
    %> value="<% Write (MESSAGE_TARGET_TRUCE); %>">All at Truce</option><%

    %><option<%
    if (iValue == MESSAGE_TARGET_TRADE) {
        %> selected<%
    }
    %> value="<% Write (MESSAGE_TARGET_TRADE); %>">All at Trade</option><%

    %><option<%
    if (iValue == MESSAGE_TARGET_ALLIANCE) {
        %> selected<%
    }
    %> value="<% Write (MESSAGE_TARGET_ALLIANCE); %>">All at Alliance</option><%

    %><option<%
    if (iValue == MESSAGE_TARGET_LAST_USED) {
        %> selected<%
    }
    %> value="<% Write (MESSAGE_TARGET_LAST_USED); %>">Last target used</option><%

    %></select></td></tr><%
    
    
    %><tr><td>Default builder planet:</td><td><select name="DefaultBuilderPlanet"><%

    iValue = pvEmpireData[SystemEmpireData::iDefaultBuilderPlanet].GetInteger();

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

    %></select></td></tr><%


    %><tr><td>Maximum number of ships built at once:</td><td><select name="MaxNumShipsBuiltAtOnce"><%

    iValue = pvEmpireData[SystemEmpireData::iMaxNumShipsBuiltAtOnce].GetInteger();

    for (i = 5; i < 16; i ++) {
        %><option <%
        if (iValue == i) {
            %>selected <%
        }
        %>value="<% Write (i); %>"><% Write (i); %></option><%
    }

    for (i = 20; i < 101; i += 10) {
        %><option <%
        if (iValue == i) {
            %>selected <%
        }
        %>value="<% Write (i); %>"><% Write (i); %></option><%
    }
    %></select></td></tr><%


    %><tr><td>Independent ship gifts:</td><td><select name="IndependentGifts"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

    if (!bFlag) {
        %> selected<%
    } %> value="0">Accept by default</option><option<%

    if (bFlag) {
        %> selected<%
    } %> value="1">Reject by default</option></select></td></tr><%
    
    
    %><tr><td>Disband empty fleets on update:</td><td><select name="DeleteEmptyFleets"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions2].GetInteger() & DISBAND_EMPTY_FLEETS_ON_UPDATE) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">Always disband empty fleets</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Never disband empty fleets</option></select></td></tr><%


    %><tr><td>Collapse or expand fleets by default:</td><td><select name="CollapseFleets"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions2].GetInteger() & FLEETS_COLLAPSED_BY_DEFAULT) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">Fleets are collapsed by default</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Fleets are expanded by default</option></select></td></tr><%


    %><tr><td>Messages sent when nuke events occur:</td><td><select name="SendScore"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & SEND_SCORE_MESSAGE_ON_NUKE) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">Send score change information when my empire nukes or is nuked</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Only send a notification when my empire is nuked</option></select></td></tr><%


    %><tr><td>Update messages for update when nuked:</td><td><select name="DisplayFatalUpdates"><option<%

    bFlag = (pvEmpireData[SystemEmpireData::iOptions].GetInteger() & DISPLAY_FATAL_UPDATE_MESSAGES) != 0;

    if (bFlag) {
        %> selected<%
    } %> value="1">Send update messages when my empire is nuked</option><option<%

    if (!bFlag) {
        %> selected<%
    } %> value="0">Don't send update messages when my empire is nuked</option></select></td></tr><%


    %><tr><td align="center" colspan="2">&nbsp;</td></tr><%
    %><tr><td align="center" colspan="2"><h3>Text:</h3></td></tr><%

    HTMLFilter (pvEmpireData[SystemEmpireData::iQuote].GetCharPtr(), &strFilter, 0, false);
    %><tr><td align="left">Quote:<br>(<em>Visible to everyone from your profile</em>)</td><%
    %><td><textarea name="Quote" cols="50" rows="6" wrap="virtual"><%
    Write (strFilter.GetCharPtr(), strFilter.GetLength());
    %></textarea></td></tr><%

    HTMLFilter (pvEmpireData[SystemEmpireData::iVictorySneer].GetCharPtr(), &strFilter, 0, false);
    %><tr><td align="left">Victory Sneer:<br>(<em>Sent to your opponents when you nuke them</em>)</td><%
    %><td><textarea name="VictorySneer" cols="50" rows="4" wrap="virtual"><%
    Write (strFilter.GetCharPtr(), strFilter.GetLength());
    %></textarea></td></tr><%

    %></table><p><%

    %><h3>Default Ship Names:</h3><p><table width="60%"><%

    for (i = FIRST_SHIP; i < NUM_SHIP_TYPES / 2; i ++)
    {
        %><tr><%

        HTMLFilter(pvEmpireData[SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN_INDEX[i]].GetCharPtr(), &strFilter, 0, false);

        %><td><% Write (SHIP_TYPE_STRING[i]); %>:</td><%
        %><td><%
        %><input type="text" size="12" maxlength="<% Write (MAX_SHIP_NAME_LENGTH); %>" <%
        %>name="ShipName<% Write (i); %>" <%
        %>value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength()); %>"><%
        %></td><%

        j = i + NUM_SHIP_TYPES / 2;

        HTMLFilter(pvEmpireData[SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN_INDEX[j]].GetCharPtr(), &strFilter, 0, false);

        %><td><% Write (SHIP_TYPE_STRING[j]); %>:</td><%
        %><td><input type="text" size="12" maxlength="<% Write (MAX_SHIP_NAME_LENGTH); %>" <%
        %>name="ShipName<% Write (j); %>" <%
        %>value="<% Write (strFilter.GetCharPtr(), strFilter.GetLength()); %>"><%
        %></td><%

        %></tr><%
    }

    %></table><p><%

    if (m_iEmpireKey != global.GetGuestKey()) {
        WriteButton (BID_BLANKEMPIRESTATISTICS);
    }

    if (!(pvEmpireData[SystemEmpireData::iOptions].GetInteger() & EMPIRE_MARKED_FOR_DELETION)) {

        if (m_iEmpireKey != global.GetRootKey() && m_iEmpireKey != global.GetGuestKey()) {
            WriteButton (BID_DELETEEMPIRE);
        }

    } else {

        WriteButton (BID_UNDELETEEMPIRE);
    }

    %><p><% WriteButton (BID_CANCEL);

    }

    break;

case 1:
    {

    // An extra I/O, but it almost never happens
    iErrCode = CacheSystemAlienIcons();
    RETURN_ON_ERROR(iErrCode);

    Variant vAlienKey;
    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlienKey, &vAlienKey);
    RETURN_ON_ERROR(iErrCode);

    Variant vAlienAddress;
    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlienAddress, &vAlienAddress);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="ProfileEditorPage" value="1"><p><%
    iErrCode = WriteEmpireIcon(vAlienKey, vAlienAddress, m_iEmpireKey, "Your current icon", false);
    RETURN_ON_ERROR(iErrCode);
    %><p><%

    iErrCode = WriteIconSelection(iAlienSelect, vAlienKey.GetInteger(), "empire");
    RETURN_ON_ERROR(iErrCode);

    }
    break;

case 2:
    {

    Variant** ppvMessage = NULL;
    AutoFreeData free_ppvMessage(ppvMessage);
    unsigned int* piMessageKey = NULL, iNumMessages, j, iNumNames = 0;
    AutoFreeKeys free_piMessageKey(piMessageKey);
    bool bSystem = false, bFound;
    const char* pszFontColor = NULL;

    iErrCode = GetSavedSystemMessages(m_iEmpireKey, &piMessageKey, &ppvMessage, &iNumMessages);
    RETURN_ON_ERROR(iErrCode);

    %><input type="hidden" name="ProfileEditorPage" value="2"><%
    if (iNumMessages == 0) {
        %><p>You have no saved system messages.<%
    } else {

        // Sort
        UTCTime* ptTime = (UTCTime*) StackAlloc (iNumMessages * sizeof (UTCTime));
        int* piIndex = (int*) StackAlloc (iNumMessages * sizeof (int));

        for (i = 0; i < (int) iNumMessages; i ++) {
            piIndex[i] = i;
            ptTime[i] = ppvMessage[i][SystemEmpireMessages::iTimeStamp].GetInteger64();
        }

        Algorithm::QSortTwoDescending<UTCTime, int> (ptTime, piIndex, iNumMessages);

        // Display
        String* pstrNameList = new String[iNumMessages];
        Assert(pstrNameList);
        Algorithm::AutoDelete<String> autopstrNameList(pstrNameList, true);

        %><p>You have <strong><% Write (iNumMessages); %></strong> saved system message<%

        if (iNumMessages != 1) {
            %>s<%
        }

        %>:<p><input type="hidden" name="NumSavedSystemMessages" value="<% Write (iNumMessages); %>"><%
        %><table width="45%"><%

        char pszDate [OS::MaxDateLength];

        for (i = 0; i < (int) iNumMessages; i ++) {

            int iFlags = ppvMessage[piIndex[i]][SystemEmpireMessages::iFlags].GetInteger();

            const char* pszSender = ppvMessage[piIndex[i]][SystemEmpireMessages::iSourceName].GetCharPtr();

            %><input type="hidden" name="MsgKey<% Write (i); %>" value ="<% Write (piMessageKey[piIndex[i]]); %>"><%
            %><input type="hidden" name="MsgSrc<% Write (i); %>" value ="<% Write (pszSender); %>"><%

            %><tr><td>Time: <% 

            iErrCode = Time::GetDateString (ppvMessage[piIndex[i]][SystemEmpireMessages::iTimeStamp].GetInteger64(), pszDate);
            RETURN_ON_ERROR(iErrCode);
            Write (pszDate);

            %><br>Sender: <% 

            if (iFlags & MESSAGE_SYSTEM) {

                bSystem = true;
                %><strong><% Write (SYSTEM_MESSAGE_SENDER); %></strong><%

            } else {

                %><strong><% Write (pszSender); %></strong><%

                // Find name in lists
                bFound = false;
                for (j = 0; j < iNumNames; j ++) {
                    if (pstrNameList[j].Equals (pszSender)) {
                        bFound = true;
                        break;
                    }
                }
                // Add name to list if not found
                if (!bFound) {
                    pstrNameList[iNumNames] = pszSender;
                    iNumNames ++;
                }
            }

            if (iFlags & MESSAGE_BROADCAST) {
                %> (broadcast)<%
                pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
            } else {
                pszFontColor = m_vPrivateMessageColor.GetCharPtr();
            }

            %><br>Delete: <input type="checkbox" name="DelChBx<% Write (i); 
            %>"></td></tr><tr><td><font size="<% Write (DEFAULT_MESSAGE_FONT_SIZE); 
            %>" face="<% Write (DEFAULT_MESSAGE_FONT); %>" color="#<% Write (pszFontColor); %>"<%
            %>><% 

            WriteFormattedMessage (ppvMessage[piIndex[i]][SystemEmpireMessages::iText].GetCharPtr());
            %></font></td></tr><tr><td>&nbsp;</td></tr><%
        }

        %></table><p>Delete messages:<p><% 

        WriteButton (BID_ALL);
        WriteButton (BID_SELECTION);

        if (bSystem) {
            WriteButton (BID_SYSTEM);
        }
        if (iNumNames > 0) {
            WriteButton (BID_EMPIRE);
            %><select name="SelectedEmpire"><%
            for (j = 0; j < iNumNames; j ++) {
                %><option value="<% Write (pstrNameList[j]); %>"><% Write (pstrNameList[j]); %></option><%
            } %></select><%
        }
    }

    }
    break;

case 3:
    {

    Variant vValue;

    unsigned int iThemeKey, iLivePlanetKey, iDeadPlanetKey, iColorKey, iHorzKey, iVertKey, iBackgroundKey, iSeparatorKey, iButtonKey;
    int iLivePlanetAddress, iDeadPlanetAddress;

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vValue);
    RETURN_ON_ERROR(iErrCode);
    iThemeKey = vValue.GetInteger();

    switch (iThemeKey)
    {
    case INDIVIDUAL_ELEMENTS:
        iErrCode = GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iLivePlanetAddress, &iDeadPlanetKey, &iDeadPlanetAddress);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIHorz, &vValue);
        RETURN_ON_ERROR(iErrCode);
        iHorzKey = vValue.GetInteger();

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIVert, &vValue);
        RETURN_ON_ERROR(iErrCode);
        iVertKey = vValue.GetInteger();

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue);
        RETURN_ON_ERROR(iErrCode);
        iColorKey = vValue.GetInteger();

        iBackgroundKey = m_iBackgroundKey;
        iSeparatorKey = m_iSeparatorKey;
        iButtonKey = m_iButtonKey;

        break;

    case ALTERNATIVE_PATH:
        int iButtonAddress, iBackgroundAddress;
        iErrCode = GetDefaultUIKeys (
            &iBackgroundKey,
            &iBackgroundAddress,
            &iLivePlanetKey,
            &iDeadPlanetKey,
            &iButtonKey,
            &iButtonAddress,
            &iSeparatorKey,
            &iHorzKey,
            &iVertKey,
            &iColorKey
            );
        RETURN_ON_ERROR(iErrCode);
        break;

    default:
        iBackgroundKey = iSeparatorKey = iButtonKey = iLivePlanetKey = iDeadPlanetKey = iHorzKey = iVertKey = iColorKey = iThemeKey;
        break;
    }

    %><input type="hidden" name="ProfileEditorPage" value="3"><%

    %><p>Choose your individual UI elements:<p><%

    iErrCode = RenderThemeInfo (iBackgroundKey, iLivePlanetKey, iDeadPlanetKey, iSeparatorKey, iButtonKey, iHorzKey, iVertKey, iColorKey);
    RETURN_ON_ERROR(iErrCode);

    Variant vTextColor, vGoodColor, vBadColor, vPrivateColor, vBroadcastColor, vCustomTableColor;
        
    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTextColor, &vTextColor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomGoodColor, &vGoodColor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBadColor, &vBadColor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomPrivateMessageColor, &vPrivateColor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBroadcastMessageColor, &vBroadcastColor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTableColor, &vCustomTableColor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue);
    RETURN_ON_ERROR(iErrCode);
    iColorKey = vValue.GetInteger();
        
        
    %><tr><td>Custom text colors</td><td>&nbsp;</td><%
    %><td><%

    %>Text color:<br><input type="text" name="CustomTextColor" <%
    %>size="<% Write (MAX_COLOR_LENGTH); %>" maxlength="<% Write (MAX_COLOR_LENGTH); %>" <%
    %>value="<% Write (vTextColor.GetCharPtr()); %>"><%

    %><br>Good color:<br><input type="text" name="CustomGoodColor" <%
    %>size="<% Write (MAX_COLOR_LENGTH); %>" maxlength="<% Write (MAX_COLOR_LENGTH); %>" <%
    %>value="<% Write (vGoodColor.GetCharPtr()); %>"><%

    %><br>Bad color:<br><input type="text" name="CustomBadColor" <%
    %>size="<% Write (MAX_COLOR_LENGTH); %>" maxlength="<% Write (MAX_COLOR_LENGTH); %>" <%
    %>value="<% Write (vBadColor.GetCharPtr()); %>"><%

    %><br>Message color:<br><input type="text" name="CustomMessageColor" <%
    %>size="<% Write (MAX_COLOR_LENGTH); %>" maxlength="<% Write (MAX_COLOR_LENGTH); %>" <%
    %>value="<% Write (vPrivateColor.GetCharPtr()); %>"><%

    %><br>Broadcast color:<br><input type="text" name="CustomBroadcastColor" <%
    %>size="<% Write (MAX_COLOR_LENGTH); %>" maxlength="<% Write (MAX_COLOR_LENGTH); %>" <%
    %>value="<% Write (vBroadcastColor.GetCharPtr()); %>"><%

    %><br>Table color:<br><input type="text" name="CustomTableColor" <%
    %>size="<% Write (MAX_COLOR_LENGTH); %>" maxlength="<% Write (MAX_COLOR_LENGTH); %>" <%
    %>value="<% Write (vCustomTableColor.GetCharPtr()); %>"><%

    %></td></tr><%

    %><tr><td>&nbsp;</td><td>&nbsp;</td><td bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" <%
    %>align="center"><input<%

    if (iColorKey == CUSTOM_COLORS) {
        %> checked<%
    }
    %> type="radio" name="Color" value="<% Write (CUSTOM_COLORS); %>"></td><%
    %></tr><%

    %></table><p><% WriteButton (BID_CANCEL);

    }
    break;

case 4: 
    {

    %><input type="hidden" name="ProfileEditorPage" value="4"><%

    %><p>Enter a directory on your local disk or on a network where a full Almonaster theme can be found:<%

    Variant vPath;
    iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlternativeGraphicsPath, &vPath);
    RETURN_ON_ERROR(iErrCode);

    %><p><input type="text" name="GraphicsPath" size="50" <%
    %>maxlength="<% Write (MAX_GRAPHICS_ALTERNATIVE_PATH_LENGTH); %>" value="<% Write (vPath.GetCharPtr()); %>"><%

    %><p>E.g:</strong> <strong>file://C:/Almonaster/MyCoolTheme</strong> or <%
    %><strong>http://www.myisp.net/~myusername/MyTheme</strong><%

    %><p><table width="60%"><tr><td><%
    %>Make sure the path is valid and the theme is complete, or else your browser <%
    %>will have problems displaying images. If you provide a local file path and <%
    %>you log into the server from a computer that doesn't have the same <%
    %>same theme directory, then the images won't be displayed either.<%
    %><p>If you haven't done so already, you should go back and <%
    %>choose text colors from the Individual Graphical Elements page that match the style of your theme.<%
    %><p>In short, hit cancel unless you know what you are doing.</td></tr></table><%

    %><p><% 

    WriteButton (BID_CANCEL);
    WriteButton (BID_CHOOSE);

    unsigned int* piThemeKey = NULL, iNumThemes;
    AutoFreeKeys free_piThemeKey(piThemeKey);
    iErrCode = GetThemeKeys (&piThemeKey, &iNumThemes);
    RETURN_ON_ERROR(iErrCode);

    if (iNumThemes == 0)
    {
        %><p>There are no themes available for download<%
    }
    else
    {
        %><p>You can download the following themes:<p><table><tr><td><ul><%

        for (i = 0; i < (int)iNumThemes; i ++)
        {
            Variant* pvThemeData = NULL;
            AutoFreeData free_pvThemeData(pvThemeData);

            iErrCode = GetThemeData (piThemeKey[i], &pvThemeData);
            RETURN_ON_ERROR(iErrCode);

            %><li><a href="<%

            WriteThemeDownloadSrc(piThemeKey[i], pvThemeData[SystemThemes::iAddress], pvThemeData[SystemThemes::iFileName]);

            %>"><% Write (pvThemeData[SystemThemes::iName].GetCharPtr()); %></a> (<%

            int iOptions = pvThemeData[SystemThemes::iOptions].GetInteger();

            bool bElement = false;
            if (iOptions & THEME_BACKGROUND) { bElement = true;
                %>Background<%
            }

            if (iOptions & THEME_LIVE_PLANET) {
                if (bElement) { %>, <% } else {
                    bElement = true;
                }
                %>Live Planet<%
            }

            if (iOptions & THEME_DEAD_PLANET) {
                if (bElement) { %>, <% } else {
                    bElement = true;
                } 
                %>Dead Planet<%
            }

            if (iOptions & THEME_SEPARATOR) {
                if (bElement) { %>, <% } else {
                    bElement = true;
                }
                %>Separator<%
            }

            if (iOptions & THEME_BUTTONS) {
                if (bElement) { %>, <% } else {
                    bElement = true;
                }
                %>Buttons<%
            }

            if (iOptions & THEME_HORZ) {
                if (bElement) { %>, <% } else {
                    bElement = true;
                }
                %>Horizontal Bar<%
            }

            if (iOptions & THEME_VERT) {
                if (bElement) { %>, <% } else {
                    bElement = true;
                }
                %>Vertical Bar<%
            }

            %>)</li><%
        }

        %></ul></td></tr></table><%

        %><p>A complete theme has a Background, a Live Planet, a Dead Planet, a Separator, Buttons, <%
        %>a Horizontal Bar and a Vertical Bar</strong><p><%
    }

    }
    break;

case 5:
    {

    %><input type="hidden" name="ProfileEditorPage" value="5"><%

    %><p>Are you sure you want to delete your empire? <%
    %>The data cannot be recovered if the empire is deleted.<p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_DELETEEMPIRE);

    }
    break;

case 6:
    {

    %><input type="hidden" name="ProfileEditorPage" value="6"><%

    %><p>Are you sure you want to blank your empire's statistics?<br>The data cannot be recovered.<p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_BLANKEMPIRESTATISTICS);

    }
    break;

case 7:
    {

    %><input type="hidden" name="ProfileEditorPage" value="7"><%

    iErrCode = DisplayThemeData(iInfoThemeKey);
    RETURN_ON_ERROR(iErrCode);

    }
    break;

case 8:

    {
    // Alternative graphics path test
    %><input type="hidden" name="ProfileEditorPage" value="8"><%
    %><input type="hidden" name="GraphicsPath" value="<% Write (pszGraphicsPath); %>"><%

    %><p>If you see broken images below, press the Cancel button. <%
    %>Otherwise press Choose to select the alternative path:<p><%

    %><img src="<% Write (pszGraphicsPath); %>/<% Write (LIVE_PLANET_NAME); %>"><%
    %><img src="<% Write (pszGraphicsPath); %>/<% Write (DEAD_PLANET_NAME); %>"><%

    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_CHOOSE);

    }
    break;

case 9:
    {
    // View tournaments
    %><input type="hidden" name="ProfileEditorPage" value="9"><%

    unsigned int iTournaments;
    Variant* pvTournamentKey = NULL;
    AutoFreeData free_pvTournamentKey(pvTournamentKey);

    // List all joined tournaments
    iErrCode = GetJoinedTournaments (m_iEmpireKey, &pvTournamentKey, NULL, &iTournaments);
    RETURN_ON_ERROR(iErrCode);

    if (iTournaments == 0)
    {
        %><p><h3>You are not in any tournaments</h3><%
    }
    else
    {
        %><p>You are in <strong><% Write (iTournaments); %></strong> tournament<%
        if (iTournaments != 1)
        {
            %>s<%
        }
        %>:</h3><%

        iErrCode = RenderTournaments(pvTournamentKey, iTournaments, false);
        RETURN_ON_ERROR(iErrCode);
    }

    }
    break;

case 10:

    // Add association
    %><input type="hidden" name="ProfileEditorPage" value="10"><%

    %><p><table align="center" width="50%"><tr><%

    %><td colspan="2"><%
    %>Empire associations provide a means of quickly switching between empires. <%
    %>This can be done by viewing the current empire's profile and selecting <%
    %>another empire from the dropdown list.<%
    %><p>Enter the name and password of another empire to create a new association:<%
    %></td><%

    %></tr><tr><%
    %><td colspan="2">&nbsp;</td><%
    %></tr><tr><%

    %><td align="right">Empire Name:</td><%
    %><td><%
    %><input type="text" size="20" maxlength="<% Write (MAX_EMPIRE_NAME_LENGTH); %>" name="AssocName"<% 
    %>></td><%

    %></tr><tr><%

    %><td align="right">Password:</td><%
    %><td><%
    %><input type="password" size="20" maxlength="<% Write(MAX_EMPIRE_PASSWORD_LENGTH); %>" name="AssocPass"><%
    %></td><%

    %></tr><%
    %></table><%
    %><p><%

    WriteButton (BID_CANCEL);
    WriteButton (BID_ADD_ASSOCIATION);

    break;

default:

    Assert(false);
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>