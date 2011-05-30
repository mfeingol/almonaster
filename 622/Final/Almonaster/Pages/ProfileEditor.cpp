
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"
#include <stdio.h>

#include "Osal/Algorithm.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the ProfileEditor page
int HtmlRenderer::Render_ProfileEditor() {

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

	INITIALIZE_EMPIRE

	IHttpForm* pHttpForm;

	int i, iErrCode, iProfileEditorPage = 0, iInfoThemeKey = NO_KEY, iAlienSelect = 0, 
	    iNewButtonKey = m_iButtonKey;
	const char* pszGraphicsPath = NULL;

	int iAutoLogonSelected = MAYBE_AUTOLOGON;

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

	    if ((pHttpForm = m_pHttpRequest->GetForm ("ProfileEditorPage")) == NULL) {
	        goto Redirection;
	    }
	    int iProfileEditorPageSubmit = pHttpForm->GetIntValue();

	    if (WasButtonPressed (BID_CANCEL)) {

	        if (iProfileEditorPageSubmit == 7) {
	            bRedirectTest = false;
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

	            if (String::StrCmp (pszNewValue, m_vEmpireName.GetCharPtr()) != 0) {
	                if (String::StriCmp (pszNewValue, m_vEmpireName.GetCharPtr()) == 0) {

	                    EmpireCheck (g_pGameEngine->SetEmpireName (m_iEmpireKey, pszNewValue));
	                    AddMessage ("Your empire name was recased");
	                    m_vEmpireName = pszNewValue;

	                } else {
	                    AddMessage ("You can only recase your empire's name, not rename it");
	                }
	            }

	            // Handle password change
	            if (m_iEmpireKey != GUEST_KEY) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("NewPassword")) == NULL) {
	                    goto Redirection;
	                }
	                pszNewValue = pHttpForm->GetValue();

	                if ((pHttpForm = m_pHttpRequest->GetForm ("VerifyPassword")) == NULL) {
	                    goto Redirection;
	                }
	                pszVerify = pHttpForm->GetValue();
	                
	                if (String::StrCmp (pszNewValue, pszVerify) != 0) {
	                    AddMessage ("Your password confirmation did not match");
	                }

	                else if (String::StrCmp (pszNewValue, INVALID_PASSWORD_STRING) != 0) {
	                
	                    if (String::StrCmp (pszVerify, m_vPassword.GetCharPtr()) == 0) {
	                        AddMessage ("You submitted the same password");
	                    }
	                    
	                    else if (VerifyPassword (pszNewValue) == OK) {

	                        iErrCode = g_pGameEngine->ChangeEmpirePassword (m_iEmpireKey, pszNewValue);
	                        if (iErrCode == ERROR_CANNOT_MODIFY_GUEST) {
	                            AddMessage (GUEST_NAME "'s password can only be changed by an administrator");
	                        } else {

	                            AddMessage ("Your password was changed");
	                            m_vPassword = pszNewValue;

	                            ICookie* pCookie = m_pHttpRequest->GetCookie (AUTOLOGON_EMPIREKEY_COOKIE);
	                            if (pCookie != NULL && pCookie->GetValue() != NULL) {

	                                if (pCookie->GetUIntValue() == m_iEmpireKey) {

	                                    int64 i64Hash = 0;
	                                    char pszText [128] = "";
	                                    
	                                    iErrCode = GetPasswordHashForAutologon (&i64Hash);
	                                    if (iErrCode != OK) {
	                                        AddMessage ("Autologon failed and will be disabled");
	                                    } else {
	                                    
	                                        m_pHttpResponse->CreateCookie (
	                                            AUTOLOGON_PASSWORD_COOKIE,
	                                            String::I64toA (i64Hash, pszText, 10),
	                                            ONE_YEAR_IN_SECONDS,
	                                            NULL
	                                            );
	                                    }
	                                }
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

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::RealName, &vVerify));

	            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

	                if (strlen (pszNewValue) > MAX_REAL_NAME_LENGTH) {
	                    AddMessage ("Your real name was too long");
	                }

	                else if (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::RealName, pszNewValue) == OK) {
	                    AddMessage ("Your real name was changed");
	                } else {
	                    AddMessage ("Your real name could not be changed");
	                }
	            }

	            // Handle EmpAge change
	            if ((pHttpForm = m_pHttpRequest->GetForm ("EmpAge")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Age, &vVerify));
	            if (iNewValue != vVerify.GetInteger()) {

	                if (iNewValue < EMPIRE_AGE_MINIMUM && iNewValue != EMPIRE_AGE_UNKNOWN) {
	                    AddMessage ("Your age is invalid");
	                } else {
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::Age, iNewValue));
	                    AddMessage ("Your age was changed");
	                }
	            }

	            // Handle EmpGender change
	            if ((pHttpForm = m_pHttpRequest->GetForm ("EmpGender")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Gender, &vVerify));
	            if (iNewValue != vVerify.GetInteger()) {

	                if (iNewValue < EMPIRE_GENDER_UNKNOWN || iNewValue > EMPIRE_GENDER_FEMALE) {
	                    AddMessage ("Your gender is invalid");
	                } else {
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::Gender, iNewValue));
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

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Location, &vVerify));

	            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

	                if (strlen (pszNewValue) > MAX_LOCATION_LENGTH) {
	                    AddMessage ("Your location was too long");
	                }

	                else if (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::Location, pszNewValue) == OK) {
	                    AddMessage ("Your location was changed");
	                } else {
	                    AddMessage ("Your location could not be changed");
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

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::Email, &vVerify));

	            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

	                if (strlen (pszNewValue) > MAX_EMAIL_LENGTH) {
	                    AddMessage ("Your e-mail address was too long");
	                }

	                else if (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::Email, pszNewValue) == OK) {
	                    AddMessage ("Your e-mail address was changed");
	                } else {
	                    AddMessage ("Your e-mail address could not be changed");
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

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::PrivateEmail, &vVerify));

	            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

	                if (strlen (pszNewValue) > MAX_EMAIL_LENGTH) {
	                    AddMessage ("Your private e-mail address was too long");
	                }

	                else if (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::PrivateEmail, pszNewValue) == OK) {
	                    AddMessage ("Your private e-mail address was changed");
	                } else {
	                    AddMessage ("Your private e-mail address could not be changed");
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

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::IMId, &vVerify));

	            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

	                if (strlen (pszNewValue) > MAX_IMID_LENGTH) {
	                    AddMessage ("Your instant messenger id was too long");
	                }

	                else if (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::IMId, pszNewValue) == OK) {
	                    AddMessage ("Your instant messenger id was changed");
	                } else {
	                    AddMessage ("Your instant messenger id could not be changed");
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

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::WebPage, &vVerify));

	            if (String::StrCmp (pszNewValue, vVerify.GetCharPtr()) != 0) {

	                if (strlen (pszNewValue) > MAX_WEB_PAGE_LENGTH) {
	                    AddMessage ("Your real name was too long");
	                }

	                else if (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::WebPage, pszNewValue) == OK) {
	                    AddMessage ("Your webpage was changed");
	                } else {
	                    AddMessage ("Your webpage could not be changed");
	                }
	            }


	            // TourneyAvail
	            if ((pHttpForm = m_pHttpRequest->GetForm ("TourneyAvail")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            bValue = !(m_iSystemOptions2 & UNAVAILABLE_FOR_TOURNAMENTS);
	            if (bValue != (iNewValue != 0)) {

	                EmpireCheck (g_pGameEngine->SetEmpireOption2 (m_iEmpireKey, UNAVAILABLE_FOR_TOURNAMENTS, bValue));

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

	            EmpireCheck (g_pGameEngine->GetEmpireMaxNumSavedSystemMessages (m_iEmpireKey, &iVerify));

	            if (iNewValue != iVerify) {

	                EmpireCheck (g_pGameEngine->GetSystemProperty (SystemData::MaxNumSystemMessages, &vValue));

	                if (iNewValue > vValue.GetInteger()) {
	                    AddMessage ("Illegal maximum number of saved system messages");
	                } else {
	                    EmpireCheck (g_pGameEngine->SetEmpireMaxNumSavedSystemMessages (m_iEmpireKey, iNewValue));
	                    AddMessage ("Your maximum number of saved messages was changed");
	                }
	            }

	            // Handle MaxNumShipsBuiltAtOnce change
	            if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumShipsBuiltAtOnce")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::MaxNumShipsBuiltAtOnce, &vValue));

	            if (iNewValue != vValue.GetInteger()) {

	                if (iNewValue > 100) {
	                    AddMessage ("Illegal maximum number of ships built at once");
	                } else {
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::MaxNumShipsBuiltAtOnce, iNewValue));
	                    AddMessage ("Your maximum number of ships built at once was updated");
	                }
	            }

	            // Handle DefaultBuilderPlanet
	            if ((pHttpForm = m_pHttpRequest->GetForm ("DefaultBuilderPlanet")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            EmpireCheck (g_pGameEngine->GetEmpireDefaultBuilderPlanet (m_iEmpireKey, &iVerify));

	            if (iNewValue != iVerify) {

	                iErrCode = g_pGameEngine->SetEmpireDefaultBuilderPlanet (m_iEmpireKey, iNewValue);
	                if (iErrCode == OK) {
	                    AddMessage ("Your default builder planet was updated");
	                } else {
	                    AddMessage ("Your default builder planet could not be updated; the error was ");
	                    AppendMessage (iErrCode);
	                }
	            }

	            // Handle IndependentGifts
	            if ((pHttpForm = m_pHttpRequest->GetForm ("IndependentGifts")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            bValue = (m_iSystemOptions & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

	            if ((iNewValue != 0) != bValue) {

	                iErrCode = g_pGameEngine->SetEmpireOption (m_iEmpireKey, REJECT_INDEPENDENT_SHIP_GIFTS, !bValue);
	                if (iErrCode == OK) {
	                    if (!bValue) {
	                        m_iSystemOptions |= REJECT_INDEPENDENT_SHIP_GIFTS;
	                    } else {
	                        m_iSystemOptions &= ~REJECT_INDEPENDENT_SHIP_GIFTS;
	                    }
	                    AddMessage ("Your independent gift option was updated");
	                } else {
	                    AddMessage ("Your independent gift option was not updated; the error was ");
	                    AppendMessage (iErrCode);
	                }
	            }
	            
	            // Handle DeleteEmptyFleets
	            if ((pHttpForm = m_pHttpRequest->GetForm ("DeleteEmptyFleets")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            bValue = (m_iSystemOptions2 & DISBAND_EMPTY_FLEETS_ON_UPDATE) != 0;

	            if ((iNewValue != 0) != bValue) {

	                iErrCode = g_pGameEngine->SetEmpireOption2 (m_iEmpireKey, DISBAND_EMPTY_FLEETS_ON_UPDATE, !bValue);
	                if (iErrCode == OK) {
	                    if (!bValue) {
	                        m_iSystemOptions2 |= DISBAND_EMPTY_FLEETS_ON_UPDATE;
	                    } else {
	                        m_iSystemOptions2 &= ~DISBAND_EMPTY_FLEETS_ON_UPDATE;
	                    }
	                    AddMessage ("Your empty fleet disband option was updated");
	                } else {
	                    AddMessage ("Your empty fleet disband option was not updated; the error was ");
	                    AppendMessage (iErrCode);
	                }
	            }

	            // Handle CollapseFleets
	            if ((pHttpForm = m_pHttpRequest->GetForm ("CollapseFleets")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();
	            bValue = (m_iSystemOptions2 & FLEETS_COLLAPSED_BY_DEFAULT) != 0;

	            if ((iNewValue != 0) != bValue) {

	                EmpireCheck (g_pGameEngine->SetEmpireOption2 (m_iEmpireKey, FLEETS_COLLAPSED_BY_DEFAULT, !bValue));

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

	                EmpireCheck (g_pGameEngine->SetEmpireOption2 (m_iEmpireKey, BLOCK_UPLOADED_ICONS, !bValue));

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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, CONFIRM_IMPORTANT_CHOICES, !bSessionId));

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

	                    m_pHttpResponse->DeleteCookie (AUTOLOGON_EMPIREKEY_COOKIE, NULL);
	                    m_pHttpResponse->DeleteCookie (AUTOLOGON_PASSWORD_COOKIE, NULL);

	                } else {

	                    if (uiNewValue != m_iEmpireKey) {
	                        AddMessage ("Invalid autologon submission");
	                        goto Quote;
	                    }

	                    // Set cookies (expire in a year)
	                    char pszText [128];
	                    iErrCode = m_pHttpResponse->CreateCookie (
	                        AUTOLOGON_EMPIREKEY_COOKIE,
	                        String::UItoA (uiNewValue, pszText, 10),
	                        ONE_YEAR_IN_SECONDS,
	                        NULL
	                        );
	                        
	                    if (iErrCode != OK) {
	                        AddMessage ("An autologon cookie could not be created");
	                    } else {
	                        
	                        int64 i64Hash = 0;                        
	                        iErrCode = GetPasswordHashForAutologon (&i64Hash);
	                        if (iErrCode != OK) {
	                            AddMessage ("An autologon hash could not be created");
	                        } else {

	                            iErrCode = m_pHttpResponse->CreateCookie (
	                                AUTOLOGON_PASSWORD_COOKIE,
	                                String::I64toA (i64Hash, pszText, 10),
	                                ONE_YEAR_IN_SECONDS,
	                                NULL
	                                );
	                                
	                            if (iErrCode != OK) {
	                                AddMessage ("An autologon cookie could not be created");
	                            } else {
	                                AddMessage ("Autologon is now on for ");
	                                AppendMessage (m_vEmpireName.GetCharPtr());

	                                iAutoLogonSelected = m_iEmpireKey;
	                            }
	                        }
	                    }
	                }
	            }

	Quote:
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

	            iErrCode = g_pGameEngine->UpdateEmpireQuote (m_iEmpireKey, pszString, &bTruncate);

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

	            case ERROR_STRING_IS_TOO_LONG:
	                AddMessage ("Your quote is too long");
	                break;

	            default:
	                AddMessage ("Your quote could not be updated. The error was ");
	                AppendMessage (iErrCode);
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

	            iErrCode = g_pGameEngine->UpdateEmpireVictorySneer (m_iEmpireKey, pszString, &bTruncate);

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

	            case ERROR_STRING_IS_TOO_LONG:
	                AddMessage ("Your victory sneer is too long");
	                break;

	            default:
	                AddMessage ("Your victory sneer could not be updated - the error was ");
	                AppendMessage (iErrCode);
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SYSTEM_REPEATED_BUTTONS, bNewValue));
	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, GAME_REPEATED_BUTTONS, bNewValue2));

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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SYSTEM_DISPLAY_TIME, bNewValue));
	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, GAME_DISPLAY_TIME, bNewValue2));

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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, AUTO_REFRESH, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, COUNTDOWN, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, MAP_COLORING, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHIP_MAP_COLORING, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHIP_MAP_HIGHLIGHTING, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SENSITIVE_MAPS, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, PARTIAL_MAPS, !bValue));
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
	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, LOCAL_MAPS_IN_UPCLOSE_VIEWS, !bValue));
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
	            EmpireCheck (g_pGameEngine->GetEmpireProperty (
	                m_iEmpireKey,
	                SystemEmpireData::GameRatios,
	                &vTemp
	                ));

	            iValue = vTemp.GetInteger();

	            if ((pHttpForm = m_pHttpRequest->GetForm ("Ratios")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            if (iNewValue != iValue && iNewValue >= RATIOS_DISPLAY_NEVER && iNewValue <= RATIOS_DISPLAY_ALWAYS) {

	                EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::GameRatios, iNewValue));
	                AppendMessage ("Your game ratios line setting was updated");
	            }

	            // MessageTarget
	            if ((pHttpForm = m_pHttpRequest->GetForm ("MessageTarget")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            EmpireCheck (g_pGameEngine->GetEmpireDefaultMessageTarget (m_iEmpireKey, &iValue));
	            if (iValue != iNewValue) {
	                iErrCode = g_pGameEngine->SetEmpireDefaultMessageTarget (m_iEmpireKey, iNewValue);
	                if (iErrCode == OK) {
	                    AddMessage ("The default message target was updated");
	                } else {
	                    AddMessage ("The default message target could not be updated; the error was ");
	                    AppendMessage (iErrCode);
	                }
	            }

	            // TechDesc
	            if ((pHttpForm = m_pHttpRequest->GetForm ("TechDesc")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetIntValue();

	            bValue = (m_iSystemOptions & SHOW_TECH_DESCRIPTIONS) != 0;
	            if (bValue != (iNewValue != 0)) {
	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHOW_TECH_DESCRIPTIONS, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHIPS_ON_MAP_SCREEN, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHIPS_ON_PLANETS_SCREEN, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, BUILD_ON_MAP_SCREEN, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, BUILD_ON_PLANETS_SCREEN, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption2 (m_iEmpireKey, REFRESH_UNSTARTED_GAME_PAGES, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, DISPLACE_ENDTURN_BUTTON, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, FIXED_BACKGROUNDS, !bValue));
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

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, IP_ADDRESS_PASSWORD_HASHING, !bIPAddress));
	                AddMessage ("Game screen password hashing with IP address is now ");
	                AppendMessage (bIPAddress ? "off" : "on");

	                if (!bIPAddress) {
	                    m_iSystemOptions |= IP_ADDRESS_PASSWORD_HASHING;
	                } else {
	                    m_iSystemOptions &= ~IP_ADDRESS_PASSWORD_HASHING;
	                }
	            }

	            if (bSessionId != ((iNewValue & SESSION_ID_PASSWORD_HASHING) != 0)) {

	                EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SESSION_ID_PASSWORD_HASHING, !bSessionId));
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

	                        EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SHOW_ADVANCED_SEARCH_INTERFACE, !bValue));
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

	                    EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, DISPLAY_FATAL_UPDATE_MESSAGES, !bValue));
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

	                    EmpireCheck (g_pGameEngine->SetEmpireOption (m_iEmpireKey, SEND_SCORE_MESSAGE_ON_NUKE, !bValue));
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

	                    AddMessage ("The new default name for ");
	                    AppendMessage (SHIP_TYPE_STRING[i]);
	                    AppendMessage (" is illegal");

	                } else {

	                    if (strlen (pszNewValue) > MAX_SHIP_NAME_LENGTH) {

	                        AddMessage ("The new default name for ");
	                        AppendMessage (SHIP_TYPE_STRING[i]);
	                        AppendMessage (" is too long");

	                    } else {

	                        EmpireCheck (g_pGameEngine->GetDefaultEmpireShipName (m_iEmpireKey, i, &vOldShipName));

	                        if (strcmp (vOldShipName.GetCharPtr(), pszNewValue) != 0 &&
	                            g_pGameEngine->SetDefaultEmpireShipName (m_iEmpireKey, i, pszNewValue) == OK) {
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
	                bRedirectTest = false;

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
	                bRedirectTest = false;
	                iProfileEditorPage = 2;
	                break;
	            }

	            // Handle theme selection
	            if (WasButtonPressed (BID_CHOOSETHEME)) {

	                bRedirectTest = false;
	                if ((pHttpForm = m_pHttpRequest->GetForm ("GraphicalTheme")) == NULL) {
	                    goto Redirection;
	                }
	                iNewValue = pHttpForm->GetIntValue();

	                Variant vOldThemeKey;
	                switch (iNewValue) {

	                case NULL_THEME:

	                    EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vOldThemeKey));
	                    if (vOldThemeKey.GetInteger() != NULL_THEME) {

	                        EmpireCheck (g_pGameEngine->SetEmpireThemeKey (m_iEmpireKey, NULL_THEME));
	                        EmpireCheck (GetUIData (NULL_THEME));

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
	                    iErrCode = g_pGameEngine->DoesThemeExist (iNewValue, &bExist);
	                    if (iErrCode != OK || !bExist) {
	                        AddMessage ("That theme doesn't exist");
	                    } else {

	                        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vOldThemeKey));
	                        if (vOldThemeKey.GetInteger() != iNewValue) {

	                            EmpireCheck (g_pGameEngine->SetEmpireThemeKey (m_iEmpireKey, iNewValue));
	                            EmpireCheck (GetUIData (iNewValue));

	                            AddMessage ("You have selected a new theme");
	                        }
	                    }

	                    }
	                    break;
	                }
	            }

	            // Handle delete empire request
	            if (WasButtonPressed (BID_DELETEEMPIRE)) {

	                if (m_iEmpireKey == ROOT_KEY) {
	                    AddMessage (ROOT_NAME " cannot commit suicide");
	                    break;
	                }

	                else if (m_iEmpireKey == GUEST_KEY) {
	                    AddMessage (GUEST_NAME " cannot commit suicide");
	                    break;
	                }

	                else {
	                    bRedirectTest = false;
	                    iProfileEditorPage = 5;
	                    break;
	                }
	            }

	            // Handle undelete empire request
	            if (WasButtonPressed (BID_UNDELETEEMPIRE)) {

	                const char* pszReport = NULL;

	                switch (g_pGameEngine->UndeleteEmpire (m_iEmpireKey)) {

	                case ERROR_CANNOT_UNDELETE_EMPIRE:

	                    pszReport = "could not be undeleted";
	                    AddMessage ("Your empire cannot be undeleted");
	                    break;

	                case OK:

	                    pszReport = "was successfully undeleted";
	                    AddMessage ("Your empire is no longer marked for deletion"); 
	                    m_iSystemOptions &= ~EMPIRE_MARKED_FOR_DELETION;

	                    break;

	                default:

	                    AddMessage ("An unexpected error occurred");
	                }

	                SystemConfiguration scConfig;
	                if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {

	                    char pszText [MAX_EMPIRE_NAME_LENGTH + 256];

	                    if (pszReport == NULL) {
	                        sprintf (pszText, "UndeleteEmpire failed for %s: error %d", m_vEmpireName.GetCharPtr(), iErrCode);
	                    } else {
	                        sprintf (pszText, "%s %s", m_vEmpireName.GetCharPtr(), pszReport);
	                    }
	                    g_pReport->WriteReport (pszText);
	                }

	                break;
	            }

	            // Handle blank empire stats request
	            if (WasButtonPressed (BID_BLANKEMPIRESTATISTICS)) {

	                if (m_iEmpireKey == GUEST_KEY) {
	                    AddMessage (GUEST_NAME "'s statistics cannot be blanked");
	                } else {
	                    bRedirectTest = false;
	                    iProfileEditorPage = 6;
	                }
	                break;
	            }

	            // Handle view tournaments option
	            if (WasButtonPressed (BID_VIEWTOURNAMENTINFORMATION)) {

	                bRedirectTest = false;
	                iProfileEditorPage = 9;
	                break;
	            }

	            // Handle association creation
	            if (WasButtonPressed (BID_ADD_ASSOCIATION)) {

	                bRedirectTest = false;
	                iProfileEditorPage = 10;
	                break;
	            }

	            // Handle association deletion
	            if (WasButtonPressed (BID_REMOVE_ASSOCIATION)) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("Association")) == NULL) {
	                    goto Redirection;
	                }
	                
	                iErrCode = g_pGameEngine->DeleteAssociation (m_iEmpireKey, pHttpForm->GetIntValue());
	                switch (iErrCode) {
	                case OK:
	                    AddMessage ("The association was removed");
	                    break;

	                case ERROR_ASSOCIATION_NOT_FOUND:
	                    AddMessage ("The association no longer exists");
	                    break;

	                default:
	                    AddMessage ("The association could not be removed. The error was ");
	                    AppendMessage (iErrCode);
	                    break;
	                }

	                bRedirectTest = false;
	                break;
	            }

	            }
	            break;

	        case 1:
	            {

	            unsigned int iIcon;

	            iErrCode = HandleIconSelection (&iIcon, BASE_UPLOADED_ALIEN_DIR, m_iEmpireKey, NO_KEY);
	            if (iErrCode == OK) {

	                if (iIcon == UPLOADED_ICON) {

	                    if (m_iAlienKey != UPLOADED_ICON) {
	                        EmpireCheck (g_pGameEngine->SetEmpireAlienKey (m_iEmpireKey, UPLOADED_ICON));
	                        m_iAlienKey = UPLOADED_ICON;
	                    }
	                }

	                else if (m_iAlienKey != iIcon) {

	                    if (g_pGameEngine->SetEmpireAlienKey (m_iEmpireKey, iIcon) == OK) {
	                        m_iAlienKey = iIcon;
	                        AddMessage ("Your icon was updated");
	                    } else {
	                        AddMessage ("That icon no longer exists");
	                    }
	                }

	                else {
	                    AddMessage ("That was the same icon");
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
	                    if (g_pGameEngine->DeleteSystemMessage (m_iEmpireKey, iMessageKey) == OK) {
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
	                            if (g_pGameEngine->DeleteSystemMessage (m_iEmpireKey, iMessageKey) == OK) {
	                                iDeletedMessages ++;
	                            }
	                        }
	                    }

	                } else {

	                    // Check for delete system messages
	                    char pszForm [128];
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

	                            if (g_pGameEngine->GetSystemMessageProperty(
	                                m_iEmpireKey,
	                                iMessageKey,
	                                SystemEmpireMessages::Flags,
	                                &vFlags
	                                ) == OK &&

	                                (vFlags.GetInteger() & MESSAGE_SYSTEM) &&

	                                g_pGameEngine->DeleteSystemMessage (
	                                m_iEmpireKey,
	                                iMessageKey
	                                ) == OK) {

	                                iDeletedMessages ++;
	                            }
	                        }

	                    } else {

	                        // Check for delete empire message
	                        if (WasButtonPressed (BID_DELETEEMPIRE)) {

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

	                                if (g_pGameEngine->GetSystemMessageProperty(
	                                    m_iEmpireKey,
	                                    iMessageKey,
	                                    SystemEmpireMessages::Source,
	                                    &vSource
	                                    ) == OK &&

	                                    String::StrCmp (vSource.GetCharPtr(), pszSrcEmpire) == 0 &&

	                                    g_pGameEngine->DeleteSystemMessage (m_iEmpireKey, iMessageKey) == OK) {

	                                    iDeletedMessages ++;
	                                }
	                            }
	                        }
	                    }
	                }
	            }

	            if (iDeletedMessages > 0) {
	                AddMessage (iDeletedMessages);
	                AppendMessage (" system message");
	                AppendMessage (iDeletedMessages == 1 ? " was deleted" : "s were deleted");
	            }

	            }
	            break;

	        case 3:
	            {

	            unsigned int iNewValue, iLivePlanetKey, iDeadPlanetKey, iColorKey, iThemeKey;

	            // Handle graphical theme updates
	            bool bUpdate = false, bColorError = false;

	            EmpireCheck (g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey));
	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue));
	            iColorKey = vValue.GetInteger();

	            // Background
	            if ((pHttpForm = m_pHttpRequest->GetForm ("Background")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetUIntValue();

	            if (iNewValue != m_iBackgroundKey) {
	                EmpireCheck (g_pGameEngine->SetEmpireBackgroundKey (m_iEmpireKey, iNewValue));
	                m_iBackgroundKey = iNewValue;
	                bUpdate = true;
	            }

	            // LivePlanet
	            if ((pHttpForm = m_pHttpRequest->GetForm ("LivePlanet")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetUIntValue();

	            if (iNewValue != iLivePlanetKey) {
	                EmpireCheck (g_pGameEngine->SetEmpireLivePlanetKey (m_iEmpireKey, iNewValue));
	                bUpdate = true;
	            }

	            // DeadPlanet
	            if ((pHttpForm = m_pHttpRequest->GetForm ("DeadPlanet")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetUIntValue();

	            if (iNewValue != iDeadPlanetKey) {
	                EmpireCheck (g_pGameEngine->SetEmpireDeadPlanetKey (m_iEmpireKey, iNewValue));
	                bUpdate = true;
	            }

	            // Button
	            if ((pHttpForm = m_pHttpRequest->GetForm ("Button")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetUIntValue();

	            if (iNewValue != m_iButtonKey) {
	                EmpireCheck (g_pGameEngine->SetEmpireButtonKey (m_iEmpireKey, iNewValue));
	                iNewButtonKey = iNewValue;
	                bUpdate = true;
	            }

	            // Separator
	            if ((pHttpForm = m_pHttpRequest->GetForm ("Separator")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetUIntValue();

	            if (iNewValue != m_iSeparatorKey) {
	                EmpireCheck (g_pGameEngine->SetEmpireSeparatorKey (m_iEmpireKey, iNewValue));
	                bUpdate = true;
	            }

	            // Get horz, vert keys
	            unsigned int iHorzKey, iVertKey;
	            
	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIHorz, &vValue));
	            iHorzKey = vValue.GetInteger();
	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIVert, &vValue));
	            iVertKey = vValue.GetInteger();

	            // Horz
	            if ((pHttpForm = m_pHttpRequest->GetForm ("Horz")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetUIntValue();

	            if (iNewValue != iHorzKey) {
	                EmpireCheck (g_pGameEngine->SetEmpireHorzKey (m_iEmpireKey, iNewValue));
	                bUpdate = true;
	            }

	            // Vert
	            if ((pHttpForm = m_pHttpRequest->GetForm ("Vert")) == NULL) {
	                goto Redirection;
	            }
	            iNewValue = pHttpForm->GetUIntValue();

	            if (iNewValue != iVertKey) {
	                EmpireCheck (g_pGameEngine->SetEmpireVertKey (m_iEmpireKey, iNewValue));
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
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTextColor, m_vTableColor));
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
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomGoodColor, m_vGoodColor));
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
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBadColor, m_vBadColor));
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
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomPrivateMessageColor, m_vPrivateMessageColor));
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
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBroadcastMessageColor, m_vBroadcastMessageColor));
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
	                    EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTableColor, m_vTableColor));
	                } else {
	                    AddMessage ("You must submit a valid table color");
	                    bColorError = true;
	                }
	            }

	            EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vValue));
	            iThemeKey = vValue.GetInteger();

	            if (iNewValue != iColorKey) {

	                if (!bColorError) {
	                    EmpireCheck (g_pGameEngine->SetEmpireColorKey (m_iEmpireKey, iNewValue));
	                    bUpdate = true;
	                } else {

	                    // We need a color key from somewhere - use the previous theme
	                    if (iThemeKey != INDIVIDUAL_ELEMENTS && iThemeKey != ALTERNATIVE_PATH) {
	                        EmpireCheck (g_pGameEngine->SetEmpireColorKey (m_iEmpireKey, iThemeKey));
	                    }

	                    // No need for an else;  we'll keep using what we had before
	                }
	            }

	            if (bUpdate) {
	                if (iThemeKey != INDIVIDUAL_ELEMENTS) {
	                    EmpireCheck (g_pGameEngine->SetEmpireThemeKey (m_iEmpireKey, INDIVIDUAL_ELEMENTS));
	                    AddMessage ("You are now using individual UI elements");
	                } else {
	                    AddMessage ("Your individual UI elements have been updated");
	                }
	                
	                EmpireCheck (GetUIData (INDIVIDUAL_ELEMENTS));
	            }

	            const char* pszStart;
	            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ThemeInfo")) != NULL && 
	                (pszStart = pHttpForm->GetName()) != NULL &&
	                sscanf (pszStart, "ThemeInfo%d", &iInfoThemeKey) == 1) {

	                iProfileEditorPage = 7;
	                bRedirectTest = false;
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
	                        EmpireCheck (g_pGameEngine->SetEmpireThemeKey (m_iEmpireKey, ALTERNATIVE_PATH));
	                        EmpireCheck (g_pGameEngine->SetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlternativeGraphicsPath, pszPath));
	                        EmpireCheck (GetUIData (ALTERNATIVE_PATH));
	                    }
	                }
	            }

	            }

	            break;

	        case 5:
	            {

	            SystemConfiguration scConfig;
	            bool bReport = g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport;
	            if (bReport) {
	                char pszText [MAX_EMPIRE_NAME_LENGTH + 256];
	                sprintf (pszText, "%s asked to be deleted", m_vEmpireName.GetCharPtr());
	                g_pReport->WriteReport (pszText);
	            }

	            iErrCode = g_pGameEngine->DeleteEmpire (m_iEmpireKey, NULL, true, false);
	            switch (iErrCode) {

	            case OK:

	                AddMessage ("The empire ");
	                AppendMessage (m_vEmpireName.GetCharPtr());
	                AppendMessage (" was deleted");
	                return Redirect (LOGIN);

	                // The code in DeleteEmpire will report

	            case ERROR_EMPIRE_IS_IN_GAMES:

	                AddMessage ("Your empire is still in at least one game. It will be deleted when it is no longer in any games");
	                AddMessage ("Your personal information has been cleared");
	                m_iSystemOptions |= EMPIRE_MARKED_FOR_DELETION;

	                if (bReport) {
	                    char pszText [MAX_EMPIRE_NAME_LENGTH + 256];
	                    sprintf (pszText, "%s was marked for deletion", m_vEmpireName.GetCharPtr());
	                    g_pReport->WriteReport (pszText);
	                }

	                break;

	            case ERROR_EMPIRE_DOES_NOT_EXIST:

	                AddMessage ("The empire ");
	                AppendMessage (m_vEmpireName.GetCharPtr());
	                AppendMessage (" no longer exists");
	                return Redirect (LOGIN);

	            default:

	                Assert (false);
	                AddMessage ("An unexpected error occurred: ");
	                AppendMessage (iErrCode);

	                if (bReport) {
	                    char pszText [MAX_EMPIRE_NAME_LENGTH + 256];
	                    sprintf (pszText, "%s was not deleted: error %d", m_vEmpireName.GetCharPtr(), iErrCode);
	                    g_pReport->WriteReport (pszText);
	                }

	                return Redirect (m_pgPageId);
	            }

	            }
	            break;

	        case 6:

	            EmpireCheck (g_pGameEngine->BlankEmpireStatistics (m_iEmpireKey));
	            AddMessage ("Your empire's statistics have been blanked");

	            SystemConfiguration scConfig;
		        if (g_pGameEngine->GetSystemConfiguration (&scConfig) == OK && scConfig.bReport) {
		            char pszText [MAX_EMPIRE_NAME_LENGTH + 256];
		            sprintf (pszText, "%s statistics were blanked", m_vEmpireName.GetCharPtr());
	                g_pReport->WriteReport (pszText);
		        }

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
	                        iErrCode = g_pGameEngine->CreateAssociation (m_iEmpireKey, pszName, pszPass);
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
	                        AddMessage ("The association could not be created. The error was ");
	                        AppendMessage (iErrCode);
	                        iProfileEditorPage = 0;
	                        break;
	                    }
	                }
	            }

	            break;

	        default:
	            Assert (false);
	        }
	    }   // End if not cancel
	} 

	Redirection:
	if (bRedirectTest && !m_bRedirection) {
	    PageId pageRedirect;
	    if (RedirectOnSubmit (&pageRedirect)) {
	        m_iButtonKey = iNewButtonKey;
	        return Redirect (pageRedirect);
	    }
	}
	iNewButtonKey = m_iButtonKey;

	SYSTEM_OPEN (iProfileEditorPage == 1 && iAlienSelect == 1)

	// Individual page stuff starts here
	switch (iProfileEditorPage) {

	case 0:
	    {

	    Variant* pvEmpireData, vMaxNumSystemMessages;
	    int iNumActiveGames, iOptions, * piThemeKey, iNumThemes, iValue, j, iMaxNumSystemMessages;
	    bool bIP, bID, bFlag;
	    size_t stLen;

	    unsigned int iNumTournamentsJoined, iNumSystemMessages;

	    String strFilter;

	    EmpireCheck (g_pGameEngine->GetEmpireData (m_iEmpireKey, &pvEmpireData, &iNumActiveGames));
	    EmpireCheck (g_pGameEngine->GetJoinedTournaments (m_iEmpireKey, NULL, NULL, &iNumTournamentsJoined));

	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"0\"><p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"0\"><p>") - 1);
	WriteProfileAlienString (
	        pvEmpireData[SystemEmpireData::AlienKey].GetInteger(),
	        m_iEmpireKey,
	        m_vEmpireName.GetCharPtr(),
	        0,
	        "ProfileLink",
	        "View your profile",
	        false,
	        false
	        );

	    
	Write (" <font size=\"+2\">", sizeof (" <font size=\"+2\">") - 1);
	Write (m_vEmpireName.GetCharPtr());
	    
	Write ("</font><p><table width=\"90%\"><tr><td align=\"center\" colspan=\"2\"><h3>Empire Information:</h3></td></tr><tr><td align=\"left\">Recase empire name:</td><td align=\"left\"><input type=\"text\" name=\"RecasedEmpireName\" size=\"", sizeof ("</font><p><table width=\"90%\"><tr><td align=\"center\" colspan=\"2\"><h3>Empire Information:</h3></td></tr><tr><td align=\"left\">Recase empire name:</td><td align=\"left\"><input type=\"text\" name=\"RecasedEmpireName\" size=\"") - 1);
	stLen = strlen (m_vEmpireName.GetCharPtr());
	    Write ((uint64) stLen); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write ((uint64) stLen); 
	    
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (m_vEmpireName.GetCharPtr()); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	if (m_iEmpireKey != GUEST_KEY) {

	        
	Write ("<tr><td align=\"left\">Password:</td><td align=\"left\"><input type=\"password\" name=\"NewPassword\" size=\"20\" maxlength=\"", sizeof ("<tr><td align=\"left\">Password:</td><td align=\"left\"><input type=\"password\" name=\"NewPassword\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_PASSWORD_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (INVALID_PASSWORD_STRING); 
	Write ("\"></td></tr><tr><td align=\"left\">Verify password:</td><td align=\"left\"><input type=\"password\" name=\"VerifyPassword\" size=\"20\" maxlength=\"", sizeof ("\"></td></tr><tr><td align=\"left\">Verify password:</td><td align=\"left\"><input type=\"password\" name=\"VerifyPassword\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_PASSWORD_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (INVALID_PASSWORD_STRING); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


	    if (HTMLFilter (pvEmpireData[SystemEmpireData::RealName].GetCharPtr(), &strFilter, 0, false) == OK) {

	        
	Write ("<tr><td align=\"left\">Real name:</td><td><input type=\"text\" name=\"RealName\" size=\"40\" maxlength=\"", sizeof ("<tr><td align=\"left\">Real name:</td><td><input type=\"text\" name=\"RealName\" size=\"40\" maxlength=\"") - 1);
	Write (MAX_REAL_NAME_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
	        
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


	    // Age
	    iValue = pvEmpireData[SystemEmpireData::Age].GetInteger();

	    
	Write ("<tr><td align=\"left\">Age:</td><td><select name=\"EmpAge\"><option", sizeof ("<tr><td align=\"left\">Age:</td><td><select name=\"EmpAge\"><option") - 1);
	if (iValue == EMPIRE_AGE_UNKNOWN) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (EMPIRE_AGE_UNKNOWN); 
	Write ("\">N/A</option>", sizeof ("\">N/A</option>") - 1);
	for (i = EMPIRE_AGE_MINIMUM; i <= EMPIRE_AGE_MAXIMUM; i ++) {
	        
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == i) {
	            
	Write (" selected", sizeof (" selected") - 1);
	}
	        
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

	    
	Write ("</select></td></tr>", sizeof ("</select></td></tr>") - 1);
	// Gender
	    iValue = pvEmpireData[SystemEmpireData::Gender].GetInteger();

	    
	Write ("<tr><td align=\"left\">Gender:</td><td><select name=\"EmpGender\">", sizeof ("<tr><td align=\"left\">Gender:</td><td><select name=\"EmpGender\">") - 1);
	for (i = 0; i < EMPIRE_NUM_GENDERS; i ++) {

	        
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == EMPIRE_GENDER[i]) {
	            
	Write (" selected", sizeof (" selected") - 1);
	}
	        
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (EMPIRE_GENDER[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (EMPIRE_GENDER_STRING[EMPIRE_GENDER[i]]); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	    
	Write ("</select></td></tr>", sizeof ("</select></td></tr>") - 1);
	// Location
	    if (HTMLFilter (pvEmpireData[SystemEmpireData::Location].GetCharPtr(), &strFilter, 0, false) == OK) {

	        
	Write ("<tr><td align=\"left\">Location:</td><td><input type=\"text\" name=\"Location\" size=\"40\" maxlength=\"", sizeof ("<tr><td align=\"left\">Location:</td><td><input type=\"text\" name=\"Location\" size=\"40\" maxlength=\"") - 1);
	Write (MAX_LOCATION_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
	        
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


	    if (HTMLFilter (pvEmpireData[SystemEmpireData::Email].GetCharPtr(), &strFilter, 0, false) == OK) {

	        
	Write ("<tr><td align=\"left\">E-mail address:</td><td><input type=\"text\" name=\"Email\" size=\"40\" maxlength=\"", sizeof ("<tr><td align=\"left\">E-mail address:</td><td><input type=\"text\" name=\"Email\" size=\"40\" maxlength=\"") - 1);
	Write (MAX_EMAIL_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
	        
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


	    if (HTMLFilter (pvEmpireData[SystemEmpireData::PrivateEmail].GetCharPtr(), &strFilter, 0, false) == OK) {

	        
	Write ("<tr><td align=\"left\">Private e-mail address (<i>only visible to administrators</em>):</td><td><input type=\"text\" name=\"PrivEmail\" size=\"40\" maxlength=\"", sizeof ("<tr><td align=\"left\">Private e-mail address (<i>only visible to administrators</em>):</td><td><input type=\"text\" name=\"PrivEmail\" size=\"40\" maxlength=\"") - 1);
	Write (MAX_EMAIL_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
	        
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


	    if (HTMLFilter (pvEmpireData[SystemEmpireData::IMId].GetCharPtr(), &strFilter, 0, false) == OK) {

	        
	Write ("<tr><td align=\"left\">Instant Messenger:</td><td><input type=\"text\" name=\"IMId\" size=\"40\" maxlength=\"", sizeof ("<tr><td align=\"left\">Instant Messenger:</td><td><input type=\"text\" name=\"IMId\" size=\"40\" maxlength=\"") - 1);
	Write (MAX_IMID_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
	        
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}


	    if (HTMLFilter (pvEmpireData[SystemEmpireData::WebPage].GetCharPtr(), &strFilter, 0, false) == OK) {

	        
	Write ("<tr><td align=\"left\">Webpage:</td><td><input type=\"text\" name=\"WebPage\" size=\"60\" maxlength=\"", sizeof ("<tr><td align=\"left\">Webpage:</td><td><input type=\"text\" name=\"WebPage\" size=\"60\" maxlength=\"") - 1);
	Write (MAX_WEB_PAGE_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
	        
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}

	    
	Write ("<tr><td>Tournaments joined:</td><td>", sizeof ("<tr><td>Tournaments joined:</td><td>") - 1);
	if (iNumTournamentsJoined == 0) {
	        
	Write ("You have not joined any tournaments", sizeof ("You have not joined any tournaments") - 1);
	} else {
	        
	Write ("You have joined <strong>", sizeof ("You have joined <strong>") - 1);
	Write (iNumTournamentsJoined); 
	Write ("</strong> tournament", sizeof ("</strong> tournament") - 1);
	if (iNumTournamentsJoined != 1) {
	            
	Write ("s", sizeof ("s") - 1);
	}

	        
	Write ("&nbsp&nbsp", sizeof ("&nbsp&nbsp") - 1);
	WriteButton (BID_VIEWTOURNAMENTINFORMATION);
	    }
	    
	Write ("</td></tr><tr><td>Tournament game availability:</td><td><select name=\"TourneyAvail\"><option", sizeof ("</td></tr><tr><td>Tournament game availability:</td><td><select name=\"TourneyAvail\"><option") - 1);
	if (!(m_iSystemOptions2 & UNAVAILABLE_FOR_TOURNAMENTS)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"1\">Available to be entered into tournament games</option><option", sizeof (" value=\"1\">Available to be entered into tournament games</option><option") - 1);
	if (m_iSystemOptions2 & UNAVAILABLE_FOR_TOURNAMENTS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"0\">Not available to be entered into tournament games</option></select></td></tr><tr><td>Empire autologon (<em>uses cookies; only for private machines)</em>:</td><td><select name=\"AutoLogon\"><option value=\"", sizeof (" value=\"0\">Not available to be entered into tournament games</option></select></td></tr><tr><td>Empire autologon (<em>uses cookies; only for private machines)</em>:</td><td><select name=\"AutoLogon\"><option value=\"") - 1);
	Write (m_iEmpireKey); 
	Write ("\">Use the current empire (", sizeof ("\">Use the current empire (") - 1);
	Write (m_vEmpireName.GetCharPtr()); 
	Write (") to autologon</option>", sizeof (") to autologon</option>") - 1);
	if (iAutoLogonSelected == MAYBE_AUTOLOGON) {

	        unsigned int iAutoLogonKey = NO_KEY;
	        ICookie* pCookie = m_pHttpRequest->GetCookie (AUTOLOGON_EMPIREKEY_COOKIE);

	        if (pCookie != NULL && pCookie->GetValue() != NULL) {

	            iAutoLogonKey = pCookie->GetUIntValue();
	            if (iAutoLogonKey != m_iEmpireKey) {

	                Variant vName;
	                iErrCode = g_pGameEngine->GetEmpireName (iAutoLogonKey, &vName);
	                if (iErrCode != OK) {
	                    iAutoLogonKey = m_iEmpireKey;
	                } else {

	                    
	Write ("<option selected value=\"", sizeof ("<option selected value=\"") - 1);
	Write (iAutoLogonKey); 
	Write ("\">Use the ", sizeof ("\">Use the ") - 1);
	Write (vName.GetCharPtr()); 
	Write (" empire to autologon</option>", sizeof (" empire to autologon</option>") - 1);
	}
	            }

	            iAutoLogonSelected = iAutoLogonKey;
	        }
	    }

	    
	Write ("<option ", sizeof ("<option ") - 1);
	if (iAutoLogonSelected == NO_AUTOLOGON || iAutoLogonSelected == MAYBE_AUTOLOGON) {
	        iAutoLogonSelected = NO_AUTOLOGON;
	        
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (NO_AUTOLOGON); 
	Write ("\">Do not autologon</option></select><input type=\"hidden\" name=\"AutoLogonSel\" value=\"", sizeof ("\">Do not autologon</option></select><input type=\"hidden\" name=\"AutoLogonSel\" value=\"") - 1);
	Write (iAutoLogonSelected); 
	Write ("\"></td></tr><tr><td>Empire associations:</td><td>", sizeof ("\"></td></tr><tr><td>Empire associations:</td><td>") - 1);
	WriteButton (BID_ADD_ASSOCIATION);

	    unsigned int* piAssoc, iAssoc;
	    if (g_pGameEngine->GetAssociations (m_iEmpireKey, &piAssoc, &iAssoc) == OK && iAssoc > 0) {

	        char pszName [MAX_EMPIRE_NAME_LENGTH + 1];

	        
	Write (" ", sizeof (" ") - 1);
	if (iAssoc == 1) {
	            
	            if (g_pGameEngine->GetEmpireName (piAssoc[0], pszName) == OK) {
	                Write (pszName);
	                
	Write ("<input type=\"hidden\" name=\"Association\" value=\"", sizeof ("<input type=\"hidden\" name=\"Association\" value=\"") - 1);
	Write (piAssoc[0]); 
	Write ("\">", sizeof ("\">") - 1);
	}

	        } else {

	            
	Write ("<select name=\"Association\">", sizeof ("<select name=\"Association\">") - 1);
	for (unsigned int a = 0; a < iAssoc; a ++) {
	            
	                if (g_pGameEngine->GetEmpireName (piAssoc[a], pszName) == OK) {
	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piAssoc[a]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pszName); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select>", sizeof ("</select>") - 1);
	}

	        
	Write (" ", sizeof (" ") - 1);
	WriteButton (BID_REMOVE_ASSOCIATION);
	        OS::HeapFree (piAssoc);
	    }

	    
	Write ("</td></tr><tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr><tr><td align=\"center\" colspan=\"2\"><h3>System User Interface:</h3></td></tr><tr><td>Choose an icon:</td><td><select name=\"IconSelect\">", sizeof ("</td></tr><tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr><tr><td align=\"center\" colspan=\"2\"><h3>System User Interface:</h3></td></tr><tr><td>Choose an icon:</td><td><select name=\"IconSelect\">") - 1);
	if (pvEmpireData[SystemEmpireData::AlienKey].GetInteger() == UPLOADED_ICON) {
	        
	Write ("<option value=\"0\">An icon from the system set</option><option selected value=\"1\">An uploaded icon</option>", sizeof ("<option value=\"0\">An icon from the system set</option><option selected value=\"1\">An uploaded icon</option>") - 1);
	} else {
	        
	Write ("<option selected value=\"0\">An icon from the system set</option><option value=\"1\">An uploaded icon</option>", sizeof ("<option selected value=\"0\">An icon from the system set</option><option value=\"1\">An uploaded icon</option>") - 1);
	}

	    
	Write ("</select> ", sizeof ("</select> ") - 1);
	WriteButton (BID_CHOOSEICON); 
	Write ("</td></tr><tr><td>Almonaster graphical theme:</td><td><select name=\"GraphicalTheme\"><option", sizeof ("</td></tr><tr><td>Almonaster graphical theme:</td><td><select name=\"GraphicalTheme\"><option") - 1);
	if (pvEmpireData[SystemEmpireData::AlmonasterTheme].GetInteger() == INDIVIDUAL_ELEMENTS) { 
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (INDIVIDUAL_ELEMENTS); 
	Write ("\">Individual Graphical Elements</option><option", sizeof ("\">Individual Graphical Elements</option><option") - 1);
	if (pvEmpireData[SystemEmpireData::AlmonasterTheme].GetInteger() == ALTERNATIVE_PATH) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (ALTERNATIVE_PATH); 
	Write ("\">Graphics from an alternative path</option><option", sizeof ("\">Graphics from an alternative path</option><option") - 1);
	if (pvEmpireData[SystemEmpireData::AlmonasterTheme].GetInteger() == NULL_THEME) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (NULL_THEME); 
	Write ("\">Null Theme</option>", sizeof ("\">Null Theme</option>") - 1);
	iErrCode = g_pGameEngine->GetFullThemeKeys (&piThemeKey, &iNumThemes);
	    if (iErrCode != OK) {
	        goto Cleanup;
	    }

	    if (iNumThemes > 0) {

	        Variant vThemeName;
	        for (i = 0; i < iNumThemes; i ++) {
	            if (g_pGameEngine->GetThemeName (piThemeKey[i], &vThemeName) == OK) {
	                
	Write ("<option ", sizeof ("<option ") - 1);
	if (pvEmpireData[SystemEmpireData::AlmonasterTheme].GetInteger() == piThemeKey[i]) {
	                    
	Write (" selected", sizeof (" selected") - 1);
	}
	                
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (piThemeKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (vThemeName.GetCharPtr()); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	        }

	        g_pGameEngine->FreeKeys (piThemeKey);
	    }
	    
	Write ("</select> ", sizeof ("</select> ") - 1);
	WriteButton (BID_CHOOSETHEME); 
	    
	Write ("</td></tr><tr><td>Placement of command buttons:</td><td><select name=\"RepeatedButtons\">", sizeof ("</td></tr><tr><td>Placement of command buttons:</td><td><select name=\"RepeatedButtons\">") - 1);
	iValue = pvEmpireData[SystemEmpireData::Options].GetInteger() & (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS);

	    
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == 0) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">At top of screen only</option><option", sizeof (" value=\"0\">At top of screen only</option><option") - 1);
	if (iValue == (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (GAME_REPEATED_BUTTONS | SYSTEM_REPEATED_BUTTONS); 
	Write ("\">At top and bottom of screen on both system and game screens by default</option><option", sizeof ("\">At top and bottom of screen on both system and game screens by default</option><option") - 1);
	if (iValue == SYSTEM_REPEATED_BUTTONS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (SYSTEM_REPEATED_BUTTONS); 
	Write ("\">At top and bottom of screen only on system screens</option><option", sizeof ("\">At top and bottom of screen only on system screens</option><option") - 1);
	if (iValue == GAME_REPEATED_BUTTONS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (GAME_REPEATED_BUTTONS); 
	Write ("\">At top and bottom of screen only on game screens by default</option></select></td></tr><tr><td>Display server time:</td><td><select name=\"TimeDisplay\">", sizeof ("\">At top and bottom of screen only on game screens by default</option></select></td></tr><tr><td>Display server time:</td><td><select name=\"TimeDisplay\">") - 1);
	iValue = pvEmpireData[SystemEmpireData::Options].GetInteger() & (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME);

	    
	Write ("<option", sizeof ("<option") - 1);
	if (iValue == (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (GAME_DISPLAY_TIME | SYSTEM_DISPLAY_TIME); 
	Write ("\">On both system screens and on game screens by default</option><option", sizeof ("\">On both system screens and on game screens by default</option><option") - 1);
	if (iValue == SYSTEM_DISPLAY_TIME) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (SYSTEM_DISPLAY_TIME); 
	Write ("\">Only on system screens</option><option", sizeof ("\">Only on system screens</option><option") - 1);
	if (iValue == GAME_DISPLAY_TIME) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (GAME_DISPLAY_TIME); 
	Write ("\">Only on game screens by default</option><option", sizeof ("\">Only on game screens by default</option><option") - 1);
	if (iValue == 0) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">On neither system screens nor game screens by default</option></select></td></tr><tr><td>System messages saved:<td>", sizeof (" value=\"0\">On neither system screens nor game screens by default</option></select></td></tr><tr><td>System messages saved:<td>") - 1);
	iErrCode = g_pGameEngine->GetNumSystemMessages (m_iEmpireKey, &iNumSystemMessages);
	    if (iErrCode != OK) {
	        goto Cleanup;
	    }

	    if (iNumSystemMessages > 0) {
	        Write (iNumSystemMessages);
	        
	Write ("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp", sizeof ("&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp") - 1);
	WriteButton (BID_VIEWMESSAGES);
	    } else {
	        
	Write ("None", sizeof ("None") - 1);
	}

	    
	Write ("</tr><tr><td>Maximum saved system messages:</td><td><select name=\"MaxNumSavedMessages\">", sizeof ("</tr><tr><td>Maximum saved system messages:</td><td><select name=\"MaxNumSavedMessages\">") - 1);
	iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxNumSystemMessages, &vMaxNumSystemMessages);
	    if (iErrCode != OK) {
	        goto Cleanup;
	    }
	    iMaxNumSystemMessages = vMaxNumSystemMessages.GetInteger();

	    for (i = 0; i <= iMaxNumSystemMessages; i += 10) {
	        
	Write ("<option", sizeof ("<option") - 1);
	if (pvEmpireData[SystemEmpireData::MaxNumSystemMessages].GetInteger() == i) {
	            
	Write (" selected ", sizeof (" selected ") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	    
	Write ("</select></td></tr><tr><td>Fixed backgrounds <em>(requires Internet Explorer)</em>:</td><td><select name=\"FixedBg\"><option", sizeof ("</select></td></tr><tr><td>Fixed backgrounds <em>(requires Internet Explorer)</em>:</td><td><select name=\"FixedBg\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & FIXED_BACKGROUNDS) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On</option><option", sizeof (" value=\"1\">On</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off</option></select></td></tr><tr><td>Block uploaded icons from other empires:</td><td><select name=\"BlockUploadedIcons\">", sizeof (" value=\"0\">Off</option></select></td></tr><tr><td>Block uploaded icons from other empires:</td><td><select name=\"BlockUploadedIcons\">") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options2].GetInteger() & BLOCK_UPLOADED_ICONS) != 0;

	    
	Write ("<option", sizeof ("<option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Display uploaded icons</option><option", sizeof (" value=\"0\">Display uploaded icons</option><option") - 1);
	if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Display default icon instead of uploaded icons</option></select></td></tr>", sizeof (" value=\"1\">Display default icon instead of uploaded icons</option></select></td></tr>") - 1);
	if (m_iPrivilege >= PRIVILEGE_FOR_ADVANCED_SEARCH) {

	        
	Write ("<tr><td>Profile Viewer search interface:</td><td><select name=\"AdvancedSearch\"><option", sizeof ("<tr><td>Profile Viewer search interface:</td><td><select name=\"AdvancedSearch\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SHOW_ADVANCED_SEARCH_INTERFACE) != 0;

	        if (bFlag) {
	            
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Display advanced search interface</option><option", sizeof (" value=\"1\">Display advanced search interface</option><option") - 1);
	if (!bFlag) {
	            
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Display simple search interface</option></select></td></tr>", sizeof (" value=\"0\">Display simple search interface</option></select></td></tr>") - 1);
	}
	    
	    
	    
	Write ("<tr><td>Prompt for confirmation on important decisions:</td><td><select name=\"Confirm\"><option", sizeof ("<tr><td>Prompt for confirmation on important decisions:</td><td><select name=\"Confirm\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & CONFIRM_IMPORTANT_CHOICES) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Always confirm on important decisions</option><option", sizeof (" value=\"1\">Always confirm on important decisions</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Never confirm on important decisions</option></select></td></tr><tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr><tr><td align=\"center\" colspan=\"2\"><h3>Game User Interface:</h3></td></tr><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"AutoRefresh\"><option", sizeof (" value=\"0\">Never confirm on important decisions</option></select></td></tr><tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr><tr><td align=\"center\" colspan=\"2\"><h3>Game User Interface:</h3></td></tr><tr><td>Refresh on update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"AutoRefresh\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & AUTO_REFRESH) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"Countdown\"><option", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Visual update countdown <em>(requires JavaScript)</em>:</td><td><select name=\"Countdown\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & COUNTDOWN) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Refresh unstarted game screens every 2 min <em>(requires JavaScript)</em>:</td><td><select name=\"RefreshUnstarted\">", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Refresh unstarted game screens every 2 min <em>(requires JavaScript)</em>:</td><td><select name=\"RefreshUnstarted\">") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options2].GetInteger() & REFRESH_UNSTARTED_GAME_PAGES) != 0;

	    
	Write ("<option", sizeof ("<option") - 1);
	if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On in all unstarted games</option><option", sizeof (" value=\"1\">On in all unstarted games</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off</option></select></td></tr><tr><td>Displace End Turn button to corner:</td><td><select name=\"DisplaceEndTurn\">", sizeof (" value=\"0\">Off</option></select></td></tr><tr><td>Displace End Turn button to corner:</td><td><select name=\"DisplaceEndTurn\">") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & DISPLACE_ENDTURN_BUTTON) != 0;

	    
	Write ("<option", sizeof ("<option") - 1);
	if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Map coloring by diplomatic status:</td><td><select name=\"MapColoring\"><option", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Map coloring by diplomatic status:</td><td><select name=\"MapColoring\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & MAP_COLORING) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Ship coloring by diplomatic status on map screen:</td><td><select name=\"ShipMapColoring\"><option", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Ship coloring by diplomatic status on map screen:</td><td><select name=\"ShipMapColoring\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SHIP_MAP_COLORING) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Ship highlighting on map screen:</td><td><select name=\"ShipHighlighting\"><option", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Ship highlighting on map screen:</td><td><select name=\"ShipHighlighting\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SHIP_MAP_HIGHLIGHTING) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name=\"SensitiveMaps\"><option", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Sensitive maps <em>(requires Internet Explorer)</em>:</td><td><select name=\"SensitiveMaps\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SENSITIVE_MAPS) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Partial maps:</td><td><select name=\"PartialMaps\"><option", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Partial maps:</td><td><select name=\"PartialMaps\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & PARTIAL_MAPS) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Display local maps in up-close map views:</td><td><select name=\"LocalMaps\"><option", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Display local maps in up-close map views:</td><td><select name=\"LocalMaps\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & LOCAL_MAPS_IN_UPCLOSE_VIEWS) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">On by default</option><option", sizeof (" value=\"1\">On by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Off by default</option></select></td></tr><tr><td>Display ship menus in planet views:</td><td><select name=\"UpCloseShips\">", sizeof (" value=\"0\">Off by default</option></select></td></tr><tr><td>Display ship menus in planet views:</td><td><select name=\"UpCloseShips\">") - 1);
	iOptions = pvEmpireData[SystemEmpireData::Options].GetInteger() & 
	        (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN);

	    
	Write ("<option ", sizeof ("<option ") - 1);
	if (iOptions == (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN)) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_MAP_SCREEN | SHIPS_ON_PLANETS_SCREEN); 
	Write ("\">Ship menus on both map and planets screens by default</option><option ", sizeof ("\">Ship menus on both map and planets screens by default</option><option ") - 1);
	if (iOptions == SHIPS_ON_MAP_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_MAP_SCREEN); 
	Write ("\">Ship menus on map screens by default</option><option ", sizeof ("\">Ship menus on map screens by default</option><option ") - 1);
	if (iOptions == SHIPS_ON_PLANETS_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (SHIPS_ON_PLANETS_SCREEN); 
	Write ("\">Ship menus on planets screen by default</option><option ", sizeof ("\">Ship menus on planets screen by default</option><option ") - 1);
	if (iOptions == 0) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"0\">No ship menus in planet views by default</option></select></td></tr><tr><td>Display build menus in planet views:</td><td><select name=\"UpCloseBuilds\">", sizeof ("value=\"0\">No ship menus in planet views by default</option></select></td></tr><tr><td>Display build menus in planet views:</td><td><select name=\"UpCloseBuilds\">") - 1);
	iOptions = pvEmpireData[SystemEmpireData::Options].GetInteger() & 
	        (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN);

	    
	Write ("<option ", sizeof ("<option ") - 1);
	if (iOptions == (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN)) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (BUILD_ON_MAP_SCREEN | BUILD_ON_PLANETS_SCREEN); 
	Write ("\">Build menus on both map and planets screens by default</option><option ", sizeof ("\">Build menus on both map and planets screens by default</option><option ") - 1);
	if (iOptions == BUILD_ON_MAP_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (BUILD_ON_MAP_SCREEN); 
	Write ("\">Build menus on map screens by default</option><option ", sizeof ("\">Build menus on map screens by default</option><option ") - 1);
	if (iOptions == BUILD_ON_PLANETS_SCREEN) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (BUILD_ON_PLANETS_SCREEN); 
	Write ("\">Build menus on planets screen by default</option><option ", sizeof ("\">Build menus on planets screen by default</option><option ") - 1);
	if (iOptions == 0) { 
	Write ("selected ", sizeof ("selected ") - 1);
	}
	    
	Write ("value=\"0\">No build menus in planet views by default</option></select></td></tr><tr><td>Show ship type descriptions in tech screen:</td><td><select name=\"TechDesc\"><option", sizeof ("value=\"0\">No build menus in planet views by default</option></select></td></tr><tr><td>Show ship type descriptions in tech screen:</td><td><select name=\"TechDesc\"><option") - 1);
	if (m_iSystemOptions & SHOW_TECH_DESCRIPTIONS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"1\">Show ship type descriptions</option><option", sizeof (" value=\"1\">Show ship type descriptions</option><option") - 1);
	if (!(m_iSystemOptions & SHOW_TECH_DESCRIPTIONS)) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"0\">Don't show ship type descriptions</option></select></td></tr><tr><td>Display ratios line:</td><td><select name=\"Ratios\"><option", sizeof (" value=\"0\">Don't show ship type descriptions</option></select></td></tr><tr><td>Display ratios line:</td><td><select name=\"Ratios\"><option") - 1);
	if (pvEmpireData[SystemEmpireData::GameRatios].GetInteger() == RATIOS_DISPLAY_NEVER) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (RATIOS_DISPLAY_NEVER); 
	Write ("\">Never by default</option><option", sizeof ("\">Never by default</option><option") - 1);
	if (pvEmpireData[SystemEmpireData::GameRatios].GetInteger() == RATIOS_DISPLAY_ON_RELEVANT_SCREENS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (RATIOS_DISPLAY_ON_RELEVANT_SCREENS); 
	Write ("\">On relevant game screens by default</option><option", sizeof ("\">On relevant game screens by default</option><option") - 1);
	if (pvEmpireData[SystemEmpireData::GameRatios].GetInteger() == RATIOS_DISPLAY_ALWAYS) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (RATIOS_DISPLAY_ALWAYS); 
	Write ("\">On all game screens by default</option></select></td></tr><tr><td>Game screen password hashing:<br>(<em>IP address hashing can conflict with firewalls</em>)</td><td><select name=\"Hashing\"><option", sizeof ("\">On all game screens by default</option></select></td></tr><tr><td>Game screen password hashing:<br>(<em>IP address hashing can conflict with firewalls</em>)</td><td><select name=\"Hashing\"><option") - 1);
	bIP = (pvEmpireData[SystemEmpireData::Options].GetInteger() & IP_ADDRESS_PASSWORD_HASHING) != 0;
	    bID = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SESSION_ID_PASSWORD_HASHING) != 0;

	    if (bIP && bID) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (IP_ADDRESS_PASSWORD_HASHING | SESSION_ID_PASSWORD_HASHING); 
	Write ("\">Use both IP address and Session Id</option><option", sizeof ("\">Use both IP address and Session Id</option><option") - 1);
	if (bIP && !bID) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (IP_ADDRESS_PASSWORD_HASHING); 
	Write ("\">Use only IP address</option><option", sizeof ("\">Use only IP address</option><option") - 1);
	if (bID && !bIP) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (SESSION_ID_PASSWORD_HASHING); 
	Write ("\">Use only Session Id</option><option", sizeof ("\">Use only Session Id</option><option") - 1);
	if (!bIP && !bID) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Use neither (insecure)</option></select></td></tr><tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr><tr><td align=\"center\" colspan=\"2\"><h3>Gameplay:</h3></td></tr>", sizeof (" value=\"0\">Use neither (insecure)</option></select></td></tr><tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr><tr><td align=\"center\" colspan=\"2\"><h3>Gameplay:</h3></td></tr>") - 1);
	iValue = pvEmpireData[SystemEmpireData::DefaultMessageTarget].GetInteger();

	    
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
	Write ("\">All at War</option><option", sizeof ("\">All at War</option><option") - 1);
	if (iValue == MESSAGE_TARGET_TRUCE) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_TRUCE); 
	Write ("\">All at Truce</option><option", sizeof ("\">All at Truce</option><option") - 1);
	if (iValue == MESSAGE_TARGET_TRADE) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_TRADE); 
	Write ("\">All at Trade</option><option", sizeof ("\">All at Trade</option><option") - 1);
	if (iValue == MESSAGE_TARGET_ALLIANCE) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_ALLIANCE); 
	Write ("\">All at Alliance</option><option", sizeof ("\">All at Alliance</option><option") - 1);
	if (iValue == MESSAGE_TARGET_LAST_USED) {
	        
	Write (" selected", sizeof (" selected") - 1);
	}
	    
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (MESSAGE_TARGET_LAST_USED); 
	Write ("\">Last target used</option></select></td></tr><tr><td>Default builder planet:</td><td><select name=\"DefaultBuilderPlanet\">", sizeof ("\">Last target used</option></select></td></tr><tr><td>Default builder planet:</td><td><select name=\"DefaultBuilderPlanet\">") - 1);
	iValue = pvEmpireData[SystemEmpireData::DefaultBuilderPlanet].GetInteger();

	    
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
	Write ("\">No default builder planet</option></select></td></tr><tr><td>Maximum number of ships built at once:</td><td><select name=\"MaxNumShipsBuiltAtOnce\">", sizeof ("\">No default builder planet</option></select></td></tr><tr><td>Maximum number of ships built at once:</td><td><select name=\"MaxNumShipsBuiltAtOnce\">") - 1);
	iValue = pvEmpireData[SystemEmpireData::MaxNumShipsBuiltAtOnce].GetInteger();

	    for (i = 5; i < 16; i ++) {
	        
	Write ("<option ", sizeof ("<option ") - 1);
	if (iValue == i) {
	            
	Write ("selected ", sizeof ("selected ") - 1);
	}
	        
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

	    for (i = 20; i < 101; i += 10) {
	        
	Write ("<option ", sizeof ("<option ") - 1);
	if (iValue == i) {
	            
	Write ("selected ", sizeof ("selected ") - 1);
	}
	        
	Write ("value=\"", sizeof ("value=\"") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (i); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	    
	Write ("</select></td></tr><tr><td>Independent ship gifts:</td><td><select name=\"IndependentGifts\"><option", sizeof ("</select></td></tr><tr><td>Independent ship gifts:</td><td><select name=\"IndependentGifts\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & REJECT_INDEPENDENT_SHIP_GIFTS) != 0;

	    if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Accept by default</option><option", sizeof (" value=\"0\">Accept by default</option><option") - 1);
	if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Reject by default</option></select></td></tr><tr><td>Disband empty fleets on update:</td><td><select name=\"DeleteEmptyFleets\"><option", sizeof (" value=\"1\">Reject by default</option></select></td></tr><tr><td>Disband empty fleets on update:</td><td><select name=\"DeleteEmptyFleets\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options2].GetInteger() & DISBAND_EMPTY_FLEETS_ON_UPDATE) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Always disband empty fleets</option><option", sizeof (" value=\"1\">Always disband empty fleets</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Never disband empty fleets</option></select></td></tr><tr><td>Collapse or expand fleets by default:</td><td><select name=\"CollapseFleets\"><option", sizeof (" value=\"0\">Never disband empty fleets</option></select></td></tr><tr><td>Collapse or expand fleets by default:</td><td><select name=\"CollapseFleets\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options2].GetInteger() & FLEETS_COLLAPSED_BY_DEFAULT) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Fleets are collapsed by default</option><option", sizeof (" value=\"1\">Fleets are collapsed by default</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Fleets are expanded by default</option></select></td></tr><tr><td>Messages sent when nuke events occur:</td><td><select name=\"SendScore\"><option", sizeof (" value=\"0\">Fleets are expanded by default</option></select></td></tr><tr><td>Messages sent when nuke events occur:</td><td><select name=\"SendScore\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & SEND_SCORE_MESSAGE_ON_NUKE) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Send score change information when my empire nukes or is nuked</option><option", sizeof (" value=\"1\">Send score change information when my empire nukes or is nuked</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Only send a notification when my empire is nuked</option></select></td></tr><tr><td>Update messages for update when nuked:</td><td><select name=\"DisplayFatalUpdates\"><option", sizeof (" value=\"0\">Only send a notification when my empire is nuked</option></select></td></tr><tr><td>Update messages for update when nuked:</td><td><select name=\"DisplayFatalUpdates\"><option") - 1);
	bFlag = (pvEmpireData[SystemEmpireData::Options].GetInteger() & DISPLAY_FATAL_UPDATE_MESSAGES) != 0;

	    if (bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"1\">Send update messages when my empire is nuked</option><option", sizeof (" value=\"1\">Send update messages when my empire is nuked</option><option") - 1);
	if (!bFlag) {
	        
	Write (" selected", sizeof (" selected") - 1);
	} 
	Write (" value=\"0\">Don't send update messages when my empire is nuked</option></select></td></tr><tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr><tr><td align=\"center\" colspan=\"2\"><h3>Text:</h3></td></tr>", sizeof (" value=\"0\">Don't send update messages when my empire is nuked</option></select></td></tr><tr><td align=\"center\" colspan=\"2\">&nbsp;</td></tr><tr><td align=\"center\" colspan=\"2\"><h3>Text:</h3></td></tr>") - 1);
	if (HTMLFilter (pvEmpireData[SystemEmpireData::Quote].GetCharPtr(), &strFilter, 0, false) == OK) {

	        
	Write ("<tr><td align=\"left\">Quote:<br>(<em>Visible to everyone from your profile</em>)</td><td><textarea name=\"Quote\" cols=\"50\" rows=\"6\" wrap=\"virtual\">", sizeof ("<tr><td align=\"left\">Quote:<br>(<em>Visible to everyone from your profile</em>)</td><td><textarea name=\"Quote\" cols=\"50\" rows=\"6\" wrap=\"virtual\">") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
	        
	Write ("</textarea></td></tr>", sizeof ("</textarea></td></tr>") - 1);
	}

	    if (HTMLFilter (pvEmpireData[SystemEmpireData::VictorySneer].GetCharPtr(), &strFilter, 0, false) == OK) {

	        
	Write ("<tr><td align=\"left\">Victory Sneer:<br>(<em>Sent to your opponents when you nuke them</em>)</td><td><textarea name=\"VictorySneer\" cols=\"50\" rows=\"4\" wrap=\"virtual\">", sizeof ("<tr><td align=\"left\">Victory Sneer:<br>(<em>Sent to your opponents when you nuke them</em>)</td><td><textarea name=\"VictorySneer\" cols=\"50\" rows=\"4\" wrap=\"virtual\">") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength());
	        
	Write ("</textarea></td></tr>", sizeof ("</textarea></td></tr>") - 1);
	}

	    
	Write ("</table><p><h3>Default Ship Names:</h3><p><table width=\"60%\">", sizeof ("</table><p><h3>Default Ship Names:</h3><p><table width=\"60%\">") - 1);
	for (i = FIRST_SHIP; i < NUM_SHIP_TYPES / 2; i ++) {

	        
	Write ("<tr>", sizeof ("<tr>") - 1);
	if (HTMLFilter (pvEmpireData[SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[i]].GetCharPtr(), &strFilter, 0, false) == OK) {

	            
	Write ("<td>", sizeof ("<td>") - 1);
	Write (SHIP_TYPE_STRING[i]); 
	Write (":</td><td><input type=\"text\" size=\"12\" maxlength=\"", sizeof (":</td><td><input type=\"text\" size=\"12\" maxlength=\"") - 1);
	Write (MAX_SHIP_NAME_LENGTH); 
	Write ("\" name=\"ShipName", sizeof ("\" name=\"ShipName") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength()); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	}

	        j = i + NUM_SHIP_TYPES / 2;

	        if (HTMLFilter (pvEmpireData[SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[j]].GetCharPtr(), &strFilter, 0, false) == OK) {

	            
	Write ("<td>", sizeof ("<td>") - 1);
	Write (SHIP_TYPE_STRING[j]); 
	Write (":</td><td><input type=\"text\" size=\"12\" maxlength=\"", sizeof (":</td><td><input type=\"text\" size=\"12\" maxlength=\"") - 1);
	Write (MAX_SHIP_NAME_LENGTH); 
	Write ("\" name=\"ShipName", sizeof ("\" name=\"ShipName") - 1);
	Write (j); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength()); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	}

	        
	Write ("</tr>", sizeof ("</tr>") - 1);
	}

	    
	Write ("</table><p>", sizeof ("</table><p>") - 1);
	if (m_iEmpireKey != GUEST_KEY) {
	        WriteButton (BID_BLANKEMPIRESTATISTICS);
	    }

	    if (!(pvEmpireData[SystemEmpireData::Options].GetInteger() & EMPIRE_MARKED_FOR_DELETION)) {

	        if (m_iEmpireKey != ROOT_KEY && m_iEmpireKey != GUEST_KEY) {
	            WriteButton (BID_DELETEEMPIRE);
	        }

	    } else {

	        WriteButton (BID_UNDELETEEMPIRE);
	    }

	Cleanup:

	    if (iErrCode != OK) {
	        
	Write ("<p>Error ", sizeof ("<p>Error ") - 1);
	Write (iErrCode); 
	Write (" occurred rendering your profile<p>", sizeof (" occurred rendering your profile<p>") - 1);
	}

	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);

	    g_pGameEngine->FreeData (pvEmpireData);
	    }

	    break;

	case 1:
	    {

	    Variant vAlienKey;
	    EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlienKey, &vAlienKey));

	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"1\"><p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"1\"><p>") - 1);
	WriteEmpireIcon (vAlienKey.GetInteger(), m_iEmpireKey, "Your current icon", false);
	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteIconSelection (iAlienSelect, vAlienKey.GetInteger(), "empire");

	    }
	    break;

	case 2:
	    {

	    Variant** ppvMessage;
	    unsigned int* piMessageKey, iNumMessages, j, iNumNames = 0;
	    bool bSystem = false, bFound;
	    const char* pszFontColor = NULL;

	    EmpireCheck (g_pGameEngine->GetSavedSystemMessages (
	        m_iEmpireKey,
	        &piMessageKey,
	        &ppvMessage,
	        &iNumMessages)
	        );

	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"2\">", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"2\">") - 1);
	if (iNumMessages == 0) {
	        
	Write ("<p>You have no saved system messages.", sizeof ("<p>You have no saved system messages.") - 1);
	} else {

	        // Sort
	        UTCTime* ptTime = (UTCTime*) StackAlloc (iNumMessages * sizeof (UTCTime));
	        int* piIndex = (int*) StackAlloc (iNumMessages * sizeof (int));

	        for (i = 0; i < (int) iNumMessages; i ++) {
	            piIndex[i] = i;
	            ptTime[i] = ppvMessage[i][SystemEmpireMessages::TimeStamp].GetUTCTime();
	        }

	        Algorithm::QSortTwoDescending<UTCTime, int> (ptTime, piIndex, iNumMessages);

	        // Display
	        String* pstrNameList = new String [iNumMessages];
	        if (pstrNameList == NULL) {
	            
	Write ("<p>Server is out of memory", sizeof ("<p>Server is out of memory") - 1);
	} else {

	            Algorithm::AutoDelete<String> autopstrNameList (pstrNameList, true);

	            
	Write ("<p>You have <strong>", sizeof ("<p>You have <strong>") - 1);
	Write (iNumMessages); 
	Write ("</strong> saved system message", sizeof ("</strong> saved system message") - 1);
	if (iNumMessages != 1) {
	                
	Write ("s", sizeof ("s") - 1);
	}

	            
	Write (":<p><input type=\"hidden\" name=\"NumSavedSystemMessages\" value=\"", sizeof (":<p><input type=\"hidden\" name=\"NumSavedSystemMessages\" value=\"") - 1);
	Write (iNumMessages); 
	Write ("\"><table width=\"45%\">", sizeof ("\"><table width=\"45%\">") - 1);
	char pszDate [OS::MaxDateLength];

	            for (i = 0; i < (int) iNumMessages; i ++) {

	                int iFlags = ppvMessage[piIndex[i]][SystemEmpireMessages::Flags].GetInteger();

	                const char* pszSender = ppvMessage[piIndex[i]][SystemEmpireMessages::Source].GetCharPtr();

	                
	Write ("<input type=\"hidden\" name=\"MsgKey", sizeof ("<input type=\"hidden\" name=\"MsgKey") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (piMessageKey[piIndex[i]]); 
	Write ("\"><input type=\"hidden\" name=\"MsgSrc", sizeof ("\"><input type=\"hidden\" name=\"MsgSrc") - 1);
	Write (i); 
	Write ("\" value =\"", sizeof ("\" value =\"") - 1);
	Write (pszSender); 
	Write ("\"><tr><td>Time: ", sizeof ("\"><tr><td>Time: ") - 1);
	iErrCode = Time::GetDateString (ppvMessage[piIndex[i]][SystemEmpireMessages::TimeStamp], pszDate);
	                if (iErrCode != OK) {
	                    
	Write ("Unknown date", sizeof ("Unknown date") - 1);
	} else {
	                    Write (pszDate);
	                }

	                 
	Write ("<br>Sender: ", sizeof ("<br>Sender: ") - 1);
	if (iFlags & MESSAGE_SYSTEM) {

	                    bSystem = true;
	                    
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (SYSTEM_MESSAGE_SENDER); 
	Write ("</strong>", sizeof ("</strong>") - 1);
	} else {

	                    
	Write ("<strong>", sizeof ("<strong>") - 1);
	Write (pszSender); 
	Write ("</strong>", sizeof ("</strong>") - 1);
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
	                    
	Write (" (broadcast)", sizeof (" (broadcast)") - 1);
	pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
	                } else {
	                    pszFontColor = m_vPrivateMessageColor.GetCharPtr();
	                }

	                
	Write ("<br>Delete: <input type=\"checkbox\" name=\"DelChBx", sizeof ("<br>Delete: <input type=\"checkbox\" name=\"DelChBx") - 1);
	Write (i); 
	                
	Write ("\"></td></tr><tr><td><font size=\"", sizeof ("\"></td></tr><tr><td><font size=\"") - 1);
	Write (DEFAULT_MESSAGE_FONT_SIZE); 
	                
	Write ("\" face=\"", sizeof ("\" face=\"") - 1);
	Write (DEFAULT_MESSAGE_FONT); 
	Write ("\" color=\"#", sizeof ("\" color=\"#") - 1);
	Write (pszFontColor); 
	Write ("\">", sizeof ("\">") - 1);
	WriteFormattedMessage (ppvMessage[piIndex[i]][SystemEmpireMessages::Text].GetCharPtr());
	                
	Write ("</font></td></tr><tr><td>&nbsp;</td></tr>", sizeof ("</font></td></tr><tr><td>&nbsp;</td></tr>") - 1);
	}

	            
	Write ("</table><p>Delete messages:<p>", sizeof ("</table><p>Delete messages:<p>") - 1);
	WriteButton (BID_ALL);
	            WriteButton (BID_SELECTION);

	            if (bSystem) {
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
	        }

	        g_pGameEngine->FreeData (ppvMessage);
	        g_pGameEngine->FreeKeys (piMessageKey);
	    }

	    }
	    break;

	case 3:
	    {

	    Variant vValue;

	    unsigned int iThemeKey, iLivePlanetKey, iDeadPlanetKey, iColorKey, iHorzKey, iVertKey, iBackgroundKey, iSeparatorKey,
	        iButtonKey;

	    EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlmonasterTheme, &vValue));
	    iThemeKey = vValue.GetInteger();

	    switch (iThemeKey) {

	    case INDIVIDUAL_ELEMENTS:

	        EmpireCheck (g_pGameEngine->GetEmpirePlanetIcons (m_iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey));
	        
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIHorz, &vValue));
	        iHorzKey = vValue.GetInteger();
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIVert, &vValue));
	        iVertKey = vValue.GetInteger();
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue));
	        iColorKey = vValue.GetInteger();

	        iBackgroundKey = m_iBackgroundKey;
	        iSeparatorKey = m_iSeparatorKey;
	        iButtonKey = m_iButtonKey;

	        break;

	    case ALTERNATIVE_PATH:

	        EmpireCheck (g_pGameEngine->GetDefaultUIKeys (
	            &iBackgroundKey,
	            &iLivePlanetKey,
	            &iDeadPlanetKey,
	            &iButtonKey,
	            &iSeparatorKey,
	            &iHorzKey,
	            &iVertKey,
	            &iColorKey
	            ));

	        break;

	    default:

	        iBackgroundKey = iSeparatorKey = iButtonKey = iLivePlanetKey = iDeadPlanetKey = iHorzKey = iVertKey = 
	            iColorKey = iThemeKey;
	        break;
	    }

	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"3\"><p>Choose your individual UI elements:<p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"3\"><p>Choose your individual UI elements:<p>") - 1);
	iErrCode = RenderThemeInfo (iBackgroundKey, iLivePlanetKey, 
	        iDeadPlanetKey, iSeparatorKey, iButtonKey, iHorzKey, iVertKey, iColorKey);

	    if (iErrCode != OK) {
	        
	Write ("Theme information could not be rendered. The error was ", sizeof ("Theme information could not be rendered. The error was ") - 1);
	Write (iErrCode); 
	Write ("<p>", sizeof ("<p>") - 1);
	} else {

	        int iColorKey;
	        Variant vTextColor, vGoodColor, vBadColor, vPrivateColor, vBroadcastColor, vCustomTableColor;
	        
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTextColor, &vTextColor));
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomGoodColor, &vGoodColor));
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBadColor, &vBadColor));
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomPrivateMessageColor, &vPrivateColor));
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBroadcastMessageColor, &vBroadcastColor));
	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTableColor, &vCustomTableColor));

	        EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue));
	        iColorKey = vValue.GetInteger();
	        
	        
	        
	Write ("<tr><td>Custom text colors</td><td>&nbsp;</td><td>Text color:<br><input type=\"text\" name=\"CustomTextColor\" size=\"", sizeof ("<tr><td>Custom text colors</td><td>&nbsp;</td><td>Text color:<br><input type=\"text\" name=\"CustomTextColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vTextColor.GetCharPtr()); 
	Write ("\"><br>Good color:<br><input type=\"text\" name=\"CustomGoodColor\" size=\"", sizeof ("\"><br>Good color:<br><input type=\"text\" name=\"CustomGoodColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vGoodColor.GetCharPtr()); 
	Write ("\"><br>Bad color:<br><input type=\"text\" name=\"CustomBadColor\" size=\"", sizeof ("\"><br>Bad color:<br><input type=\"text\" name=\"CustomBadColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vBadColor.GetCharPtr()); 
	Write ("\"><br>Message color:<br><input type=\"text\" name=\"CustomMessageColor\" size=\"", sizeof ("\"><br>Message color:<br><input type=\"text\" name=\"CustomMessageColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vPrivateColor.GetCharPtr()); 
	Write ("\"><br>Broadcast color:<br><input type=\"text\" name=\"CustomBroadcastColor\" size=\"", sizeof ("\"><br>Broadcast color:<br><input type=\"text\" name=\"CustomBroadcastColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vBroadcastColor.GetCharPtr()); 
	Write ("\"><br>Table color:<br><input type=\"text\" name=\"CustomTableColor\" size=\"", sizeof ("\"><br>Table color:<br><input type=\"text\" name=\"CustomTableColor\" size=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_COLOR_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vCustomTableColor.GetCharPtr()); 
	Write ("\"></td></tr><tr><td>&nbsp;</td><td>&nbsp;</td><td bgcolor=\"", sizeof ("\"></td></tr><tr><td>&nbsp;</td><td>&nbsp;</td><td bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\"><input", sizeof ("\" align=\"center\"><input") - 1);
	if (iColorKey == CUSTOM_COLORS) {
	            
	Write (" checked", sizeof (" checked") - 1);
	}
	        
	Write (" type=\"radio\" name=\"Color\" value=\"", sizeof (" type=\"radio\" name=\"Color\" value=\"") - 1);
	Write (CUSTOM_COLORS); 
	Write ("\"></td></tr>", sizeof ("\"></td></tr>") - 1);
	}

	    
	Write ("</table><p>", sizeof ("</table><p>") - 1);
	WriteButton (BID_CANCEL);

	    }
	    break;

	case 4: 
	    {

	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"4\"><p>Enter a directory on your local disk or on a network where a full Almonaster theme can be found:", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"4\"><p>Enter a directory on your local disk or on a network where a full Almonaster theme can be found:") - 1);
	Variant vPath;
	    
	    EmpireCheck (g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlternativeGraphicsPath, &vPath));

	    
	Write ("<p><input type=\"text\" name=\"GraphicsPath\" size=\"50\" maxlength=\"", sizeof ("<p><input type=\"text\" name=\"GraphicsPath\" size=\"50\" maxlength=\"") - 1);
	Write (MAX_GRAPHICS_ALTERNATIVE_PATH_LENGTH); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vPath.GetCharPtr()); 
	Write ("\"><p>E.g:</strong> <strong>file://C:/Almonaster/MyCoolTheme</strong> or <strong>http://www.myisp.net/~myusername/MyTheme</strong><p><table width=\"60%\"><tr><td>Make sure the path is valid and the theme is complete, or else your browser will have problems displaying images. If you provide a local file path and you log into the server from a computer that doesn't have the same same theme directory, then the images won't be displayed either.<p>If you haven't done so already, you should go back and choose text colors from the Individual Graphical Elements page that match the style of your theme.<p>In short, hit cancel unless you know what you are doing.</td></tr></table><p>", sizeof ("\"><p>E.g:</strong> <strong>file://C:/Almonaster/MyCoolTheme</strong> or <strong>http://www.myisp.net/~myusername/MyTheme</strong><p><table width=\"60%\"><tr><td>Make sure the path is valid and the theme is complete, or else your browser will have problems displaying images. If you provide a local file path and you log into the server from a computer that doesn't have the same same theme directory, then the images won't be displayed either.<p>If you haven't done so already, you should go back and choose text colors from the Individual Graphical Elements page that match the style of your theme.<p>In short, hit cancel unless you know what you are doing.</td></tr></table><p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_CHOOSE);

	    int* piThemeKey, iNumThemes;
	    Check (g_pGameEngine->GetThemeKeys (&piThemeKey, &iNumThemes));

	    if (iNumThemes == 0) {
	        
	Write ("<p>There are no themes available for download", sizeof ("<p>There are no themes available for download") - 1);
	} else {

	        
	Write ("<p>You can download the following themes:<p><table><tr><td><ul>", sizeof ("<p>You can download the following themes:<p><table><tr><td><ul>") - 1);
	Variant* pvThemeData;
	        bool bElement;

	        int iOptions;

	        for (i = 0; i < iNumThemes; i ++) {

	            if (g_pGameEngine->GetThemeData (piThemeKey[i], &pvThemeData) != OK) {
	                continue;
	            }

	            
	Write ("<li><a href=\"", sizeof ("<li><a href=\"") - 1);
	WriteThemeDownloadSrc (piThemeKey[i], pvThemeData[SystemThemes::FileName].GetCharPtr());

	            
	Write ("\">", sizeof ("\">") - 1);
	Write (pvThemeData[SystemThemes::Name].GetCharPtr()); 
	Write ("</a> (", sizeof ("</a> (") - 1);
	iOptions = pvThemeData[SystemThemes::Options].GetInteger();

	            bElement = false;
	            if (iOptions & THEME_BACKGROUND) { bElement = true;
	                
	Write ("Background", sizeof ("Background") - 1);
	}

	            if (iOptions & THEME_LIVE_PLANET) {
	                if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
	                    bElement = true;
	                }
	                
	Write ("Live Planet", sizeof ("Live Planet") - 1);
	}

	            if (iOptions & THEME_DEAD_PLANET) {
	                if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
	                    bElement = true;
	                } 
	                
	Write ("Dead Planet", sizeof ("Dead Planet") - 1);
	}

	            if (iOptions & THEME_SEPARATOR) {
	                if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
	                    bElement = true;
	                }
	                
	Write ("Separator", sizeof ("Separator") - 1);
	}

	            if (iOptions & THEME_BUTTONS) {
	                if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
	                    bElement = true;
	                }
	                
	Write ("Buttons", sizeof ("Buttons") - 1);
	}

	            if (iOptions & THEME_HORZ) {
	                if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
	                    bElement = true;
	                }
	                
	Write ("Horizontal Bar", sizeof ("Horizontal Bar") - 1);
	}

	            if (iOptions & THEME_VERT) {
	                if (bElement) { 
	Write (", ", sizeof (", ") - 1);
	} else {
	                    bElement = true;
	                }
	                
	Write ("Vertical Bar", sizeof ("Vertical Bar") - 1);
	}

	            
	Write (")</li>", sizeof (")</li>") - 1);
	g_pGameEngine->FreeData (pvThemeData);
	        }

	        
	Write ("</ul></td></tr></table>", sizeof ("</ul></td></tr></table>") - 1);
	g_pGameEngine->FreeKeys (piThemeKey);

	        
	Write ("<p>A complete theme has a Background, a Live Planet, a Dead Planet, a Separator, Buttons, a Horizontal Bar and a Vertical Bar</strong><p>", sizeof ("<p>A complete theme has a Background, a Live Planet, a Dead Planet, a Separator, Buttons, a Horizontal Bar and a Vertical Bar</strong><p>") - 1);
	}

	    }
	    break;

	case 5:
	    {

	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"5\"><p>Are you sure you want to delete your empire? The data cannot be recovered if the empire is deleted.<p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"5\"><p>Are you sure you want to delete your empire? The data cannot be recovered if the empire is deleted.<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_DELETEEMPIRE);

	    }
	    break;

	case 6:
	    {

	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"6\"><p>Are you sure you want to blank your empire's statistics?<br>The data cannot be recovered.<p>", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"6\"><p>Are you sure you want to blank your empire's statistics?<br>The data cannot be recovered.<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_BLANKEMPIRESTATISTICS);

	    }
	    break;

	case 7:
	    {

	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"7\">", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"7\">") - 1);
	DisplayThemeData (iInfoThemeKey);

	    }
	    break;

	case 8:

	    {
	    // Alternative graphics path test
	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"8\"><input type=\"hidden\" name=\"GraphicsPath\" value=\"", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"8\"><input type=\"hidden\" name=\"GraphicsPath\" value=\"") - 1);
	Write (pszGraphicsPath); 
	Write ("\"><p>If you see broken images below, press the Cancel button. Otherwise press Choose to select the alternative path:<p><img src=\"", sizeof ("\"><p>If you see broken images below, press the Cancel button. Otherwise press Choose to select the alternative path:<p><img src=\"") - 1);
	Write (pszGraphicsPath); 
	Write ("/", sizeof ("/") - 1);
	Write (LIVE_PLANET_NAME); 
	Write ("\"><img src=\"", sizeof ("\"><img src=\"") - 1);
	Write (pszGraphicsPath); 
	Write ("/", sizeof ("/") - 1);
	Write (DEAD_PLANET_NAME); 
	Write ("\"><p>", sizeof ("\"><p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_CHOOSE);

	    }
	    break;

	case 9:
	    {
	    // View tournaments
	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"9\">", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"9\">") - 1);
	unsigned int* piTournamentKey = NULL, iTournaments;

	    // List all joined tournaments
	    iErrCode = g_pGameEngine->GetJoinedTournaments (m_iEmpireKey, &piTournamentKey, NULL, &iTournaments);
	    if (iErrCode != OK) {
	        
	Write ("<p>Error ", sizeof ("<p>Error ") - 1);
	Write (iErrCode); 
	Write (" occurred", sizeof (" occurred") - 1);
	}

	    if (iTournaments == 0) {
	        
	Write ("<p><h3>You are not in any tournaments</h3>", sizeof ("<p><h3>You are not in any tournaments</h3>") - 1);
	}

	    else {

	        
	Write ("<p>You are in <strong>", sizeof ("<p>You are in <strong>") - 1);
	Write (iTournaments); 
	Write ("</strong> tournament", sizeof ("</strong> tournament") - 1);
	if (iTournaments != 1) {
	            
	Write ("s", sizeof ("s") - 1);
	}
	        
	Write (":</h3>", sizeof (":</h3>") - 1);
	RenderTournaments (piTournamentKey, iTournaments, false);
	    }

	    if (piTournamentKey != NULL) {
	        g_pGameEngine->FreeData (piTournamentKey);   // Not a bug
	    }

	    }
	    break;

	case 10:

	    // Add association
	    
	Write ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"10\"><p><table align=\"center\" width=\"50%\"><tr><td colspan=\"2\">Empire associations provide a means of quickly switching between empires. This can be done by viewing the current empire's profile and selecting another empire from the dropdown list.<p>Enter the name and password of another empire to create a new association:</td></tr><tr><td colspan=\"2\">&nbsp;</td></tr><tr><td align=\"right\">Empire Name:</td><td><input type=\"text\" size=\"20\" maxlength=\"", sizeof ("<input type=\"hidden\" name=\"ProfileEditorPage\" value=\"10\"><p><table align=\"center\" width=\"50%\"><tr><td colspan=\"2\">Empire associations provide a means of quickly switching between empires. This can be done by viewing the current empire's profile and selecting another empire from the dropdown list.<p>Enter the name and password of another empire to create a new association:</td></tr><tr><td colspan=\"2\">&nbsp;</td></tr><tr><td align=\"right\">Empire Name:</td><td><input type=\"text\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_EMPIRE_NAME_LENGTH); 
	Write ("\" name=\"AssocName\"></td></tr><tr><td align=\"right\">Password:</td><td><input type=\"password\" size=\"20\" maxlength=\"", sizeof ("\" name=\"AssocName\"></td></tr><tr><td align=\"right\">Password:</td><td><input type=\"password\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_PASSWORD_LENGTH); 
	Write ("\" name=\"AssocPass\"></td></tr></table><p>", sizeof ("\" name=\"AssocPass\"></td></tr></table><p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_ADD_ASSOCIATION);

	    break;

	default:

	    Assert (false);
	}

	SYSTEM_CLOSE


}