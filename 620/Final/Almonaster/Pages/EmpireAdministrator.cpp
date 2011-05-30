
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the EmpireAdministrator page
int HtmlRenderer::Render_EmpireAdministrator() {

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

	IHttpForm* pSearchForm, * pHttpForm;

	int iErrCode, iEmpireAdminPage = 0;
	unsigned int iLastKey = NO_KEY, * piSearchEmpireKey = NULL, iNumSearchEmpires = 0, iTargetEmpireKey = NO_KEY;

	// Make sure that the unprivileged don't abuse this:
	if (m_iPrivilege < ADMINISTRATOR) {
	    AddMessage ("You are not authorized to view this page");
	    return Redirect (LOGIN);
	}

	SearchColumn sc [MAX_NUM_SEARCH_COLUMNS];
	SearchDefinition sd;
	sd.pscColumns = sc;

	const char* pszFormName [MAX_NUM_SEARCH_COLUMNS];
	const char* pszColName1 [MAX_NUM_SEARCH_COLUMNS];
	const char* pszColName2 [MAX_NUM_SEARCH_COLUMNS];

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

	    if (!WasButtonPressed (BID_CANCEL)) {

	        if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireAdminPage")) == NULL) {
	            goto Redirection;
	        }
	        int iEmpireAdminPageSubmit = pHttpForm->GetIntValue();
	        pHttpForm = NULL;

	        switch (iEmpireAdminPageSubmit) {

	        case 0:
	            {

	            // Broadcast
	            if (WasButtonPressed (BID_SENDMESSAGE)) {

	                const char* pszString;

	                if ((pHttpForm = m_pHttpRequest->GetForm ("Message")) != NULL && 
	                    (pszString = pHttpForm->GetValue()) != NULL &&
	                    *pszString != '\0') {

	                    iErrCode = g_pGameEngine->SendMessageToAll (m_iEmpireKey, pszString);

	                    if (iErrCode == OK) {
	                        AddMessage ("Your message was broadcast to all empires");
	                    } else {
	                        AddMessage ("Your message could not be broadcast to all empires");
	                    }

	                } else {
	                    AddMessage ("Your message was blank");
	                }

	                bRedirectTest = false;

	            } else {

	                if (WasButtonPressed (BID_SEARCH)) {
	SearchResults:
	                    iEmpireAdminPage = 0;

	                    iErrCode = HandleSearchSubmission (
	                        sd,
	                        pszFormName,
	                        pszColName1,
	                        pszColName2,
	                        &piSearchEmpireKey,
	                        &iNumSearchEmpires,
	                        &iLastKey
	                        );

	                    switch (iErrCode) {

	                    case OK:
	                    case ERROR_TOO_MANY_HITS:

	                        if (iNumSearchEmpires > 0) {

	                            if (iNumSearchEmpires == 1 && iLastKey == NO_KEY) {
	                                iTargetEmpireKey = piSearchEmpireKey[0];
	                                g_pGameEngine->FreeKeys (piSearchEmpireKey);
	                                piSearchEmpireKey = NULL;
	                                iEmpireAdminPage = 3;
	                            } else {
	                                iEmpireAdminPage = 1;
	                            }
	                            break;
	                        }

	                        // Fall through if 0 empires

	                    case ERROR_DATA_NOT_FOUND:

	                        AddMessage ("No empires matched your search criteria");
	                        break;

	                    case ERROR_INVALID_ARGUMENT:

	                        // A form was missing
	                        goto Redirection;

	                    case ERROR_INVALID_QUERY:

	                        AddMessage ("You submitted an invalid query");
	                        goto Redirection;

	                    default:

	                        {
	                        char pszMessage [64];
	                        sprintf (pszMessage, "Error %i occurred", iErrCode);
	                        AddMessage (pszMessage);
	                        }
	                        break;
	                    }
	                }
	            }

	            }

	            break;

	        case 1:
	            {

	            unsigned int iHash;
	            const char* pszStart;
	            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ViewProfile")) != NULL && 
	                (pszStart = pHttpForm->GetName()) != NULL &&
	                sscanf (pszStart, "ViewProfile.%d.%d", &iTargetEmpireKey, &iHash) == 2) {

	                if (!VerifyEmpireNameHash (iTargetEmpireKey, iHash)) {
	                    AddMessage ("That empire no longer exists");
	                } else {
	                    iEmpireAdminPage = 3;
	                }

	                bRedirectTest = false;
	            }

	            if (WasButtonPressed (BID_SEARCH)) {
	                goto SearchResults;
	            }

	            }
	            break;

	        case 2:
	            {

	            if (WasButtonPressed (BID_CHANGEEMPIRESPASSWORD)) {

	                // Change password
	                if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpire")) == NULL) {
	                    goto Redirection;
	                }
	                iTargetEmpireKey = pHttpForm->GetIntValue();

	                if ((pHttpForm = m_pHttpRequest->GetForm ("NewPass")) == NULL || 
	                    (pSearchForm = m_pHttpRequest->GetForm ("NewPass2")) == NULL) {
	                    goto Redirection;
	                }

	                if (iTargetEmpireKey != m_iEmpireKey) {

	                    if (iTargetEmpireKey == ROOT_KEY) {
	                        AddMessage ("You cannot update root's password");
	                        return Redirect (m_pgPageId);
	                    }

	                    Variant vOldPassword;
	                    int iPrivilege;
	                    const char* pszValue = pHttpForm->GetValue(), * pszValue2 = pSearchForm->GetValue();

	                    if (pszValue == NULL || *pszValue == '\0' ||
	                        pszValue2 == NULL || *pszValue2 == '\0') {
	                        AddMessage ("Blank passwords are not allowed");
	                        return Redirect (m_pgPageId);
	                    }

	                    if (g_pGameEngine->GetEmpirePassword (iTargetEmpireKey, &vOldPassword) != OK ||
	                        g_pGameEngine->GetEmpirePrivilege (iTargetEmpireKey, &iPrivilege) != OK) {
	                        AddMessage ("The empire no longer exists");
	                        return Redirect (m_pgPageId);
	                    }

	                    if (iPrivilege >= ADMINISTRATOR && m_iEmpireKey != ROOT_KEY) {
	                        AddMessage ("You cannot change an administrator's password");
	                        return Redirect (m_pgPageId);
	                    }

	                    if (String::StrCmp (vOldPassword.GetCharPtr(), pszValue) == 0) {
	                        AddMessage ("The new password was the same as the old one");
	                    } else {
	                        if (VerifyPassword (pszValue) != OK) {
	                            return Redirect (m_pgPageId);
	                        }

	                        // Confirm
	                        if (strcmp (pszValue, pszValue2) != 0) {
	                            AddMessage ("The new password was not confirmed");
	                            return Redirect (m_pgPageId);
	                        }

	                        iErrCode = g_pGameEngine->SetEmpirePassword (iTargetEmpireKey, pszValue);
	                        if (iErrCode == OK) {
	                            AddMessage ("The empire's password was successfully changed");
	                        } else {
	                            AddMessage ("The empire's password could not be changed");
	                            return Redirect (m_pgPageId);
	                        }
	                    }
	                }
	            }

	            }
	            break;

	        case 3:

	            {

	            if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpire")) == NULL) {
	                goto Redirection;
	            }

	            iTargetEmpireKey = pHttpForm->GetIntValue();

	            // Change privilege, scores
	            int iValue, iOldValue;
	            float fValue, fOldValue;
	            bool bValue, bOldValue;
	            
	            if ((pHttpForm = m_pHttpRequest->GetForm ("FixedPriv")) == NULL) {
	                goto Redirection;
	            }
	            bValue = pHttpForm->GetIntValue() != 0;

	            if ((pHttpForm = m_pHttpRequest->GetForm ("OldFixedPriv")) == NULL) {
	                goto Redirection;
	            }
	            bOldValue = pHttpForm->GetIntValue() != 0;
	            
	            if (bValue != bOldValue) {
	            
	                if (iTargetEmpireKey == m_iEmpireKey) {
	                    AddMessage ("You cannot change your own privilege settings");
	                }

	                else if (iTargetEmpireKey == ROOT_KEY) {
	                    AddMessage ("You cannot change " ROOT_NAME "'s privilege settings");
	                }

	                else if (iTargetEmpireKey == GUEST_KEY) {
	                    AddMessage ("You cannot change " GUEST_NAME "'s privilege settings");
	                }

	                else {

	                    if (g_pGameEngine->SetEmpireOption2 (iTargetEmpireKey, ADMINISTRATOR_FIXED_PRIVILEGE, bValue) == OK) {
	                        AddMessage ("The empire's privilege settings were successfully updated");
	                    } else {
	                        AddMessage ("The empire's privilege settings could not be updated");
	                    }
	                }
	            }

	            if ((pHttpForm = m_pHttpRequest->GetForm ("NewPriv")) == NULL) {
	                goto Redirection;
	            }
	            iValue = pHttpForm->GetIntValue();

	            if ((pHttpForm = m_pHttpRequest->GetForm ("OldPriv")) == NULL) {
	                goto Redirection;
	            }
	            iOldValue = pHttpForm->GetIntValue();

	            if (iValue != iOldValue) {

	                if (iTargetEmpireKey == m_iEmpireKey) {
	                    AddMessage ("You cannot change your own privilege level");
	                }

	                else if (iTargetEmpireKey == ROOT_KEY) {
	                    AddMessage ("You cannot change " ROOT_NAME "'s privilege level");
	                }

	                else if (iTargetEmpireKey == GUEST_KEY) {
	                    AddMessage ("You cannot change " GUEST_NAME "'s privilege level");
	                }

	                else if (!IS_VALID_PRIVILEGE (iValue)) {
	                    AddMessage ("Invalid privilege");
	                }

	                else {

	                    if (g_pGameEngine->SetEmpirePrivilege (iTargetEmpireKey, iValue) == OK) {
	                        AddMessage ("The empire's privilege level was successfully updated");
	                    } else {
	                        AddMessage ("The empire's privilege level could not be updated");
	                    }
	                }
	            }

	            if ((pHttpForm = m_pHttpRequest->GetForm ("NewAScore")) == NULL) {
	                goto Redirection;
	            }
	            fValue = pHttpForm->GetFloatValue();

	            if ((pHttpForm = m_pHttpRequest->GetForm ("OldAScore")) == NULL) {
	                goto Redirection;
	            }
	            fOldValue = pHttpForm->GetFloatValue();

	            if (fValue != fOldValue) {

	                if (iTargetEmpireKey == m_iEmpireKey) {
	                    AddMessage ("You cannot change your own Almonaster score");
	                }

	                else if (iTargetEmpireKey == ROOT_KEY) {
	                    AddMessage ("You cannot change " ROOT_NAME "'s Almonaster score");
	                }

	                else if (iTargetEmpireKey == GUEST_KEY) {
	                    AddMessage ("You cannot change " GUEST_NAME "'s Almonaster score");
	                }

	                else {

	                    if (g_pGameEngine->SetEmpireAlmonasterScore (iTargetEmpireKey, fValue) == OK) {
	                        AddMessage ("The empire's Almonaster score was successfully updated");
	                    } else {
	                        AddMessage ("The submitted Almonaster score was incorrect");
	                    }
	                }
	            }

	            if ((pHttpForm = m_pHttpRequest->GetForm ("Broadcast")) == NULL) {
	                goto Redirection;
	            }
	            bValue = (pHttpForm->GetIntValue() != 0);

	            if ((pHttpForm = m_pHttpRequest->GetForm ("OldBroadcast")) == NULL) {
	                goto Redirection;
	            }
	            bOldValue = (pHttpForm->GetIntValue() != 0);

	            if (bValue != bOldValue) {

	                if (iTargetEmpireKey == m_iEmpireKey) {
	                    AddMessage ("You cannot change your own broadcast setting");
	                }

	                else if (iTargetEmpireKey == ROOT_KEY) {
	                    AddMessage ("You cannot change " ROOT_NAME "'s broadcast setting");
	                }

	                else if (iTargetEmpireKey == GUEST_KEY) {
	                    AddMessage ("You cannot change " GUEST_NAME "'s broadcast setting");
	                }

	                else {

	                    if (g_pGameEngine->SetEmpireOption (iTargetEmpireKey, CAN_BROADCAST, bValue) == OK) {
	                        if (bValue) {
	                            AddMessage ("The empire can now broadcast messages");
	                        } else {
	                            AddMessage ("The empire can no longer broadcast messages");
	                        }
	                    } else {
	                        AddMessage ("The empire does not exist");
	                    }
	                }
	            }

	            if (WasButtonPressed (BID_RESET)) {

	                iErrCode = g_pGameEngine->ResetEmpireSessionId (iTargetEmpireKey);
	                if (iErrCode == OK) {
	                    AddMessage ("The empire's session id will be reset on its next login");
	                } else {
	                    AddMessage ("The empire's session id could not be reset");
	                }

	                iEmpireAdminPage = 3;
	                bRedirectTest = false;
	            }

	            // Lookup or Update
	            if (WasButtonPressed (BID_LOOKUP) || WasButtonPressed (BID_UPDATE)) {
	                bRedirectTest = false;
	                iEmpireAdminPage = 3;
	                break;
	            }

	            // Change password
	            if (WasButtonPressed (BID_CHANGEEMPIRESPASSWORD) && iTargetEmpireKey != m_iEmpireKey) {
	                iEmpireAdminPage = 2;
	                bRedirectTest = false;
	                break;
	            }

	            // Obliterate empire
	            if (WasButtonPressed (BID_OBLITERATEEMPIRE)) {

	                if (iTargetEmpireKey == m_iEmpireKey) {
	                    AddMessage ("You cannot obliterate yourself");
	                }

	                else if (iTargetEmpireKey == ROOT_KEY) {
	                    AddMessage ("You cannot obliterate " ROOT_NAME);
	                }

	                else if (iTargetEmpireKey == GUEST_KEY) {
	                    AddMessage ("You cannot obliterate " GUEST_NAME);
	                }

	                else {

	                    iEmpireAdminPage = 4;
	                }

	                bRedirectTest = false;
	                break;
	            }

	            }

	            break;

	        case 4:
	            {

	            // Obliterate empire
	            if (WasButtonPressed (BID_OBLITERATEEMPIRE)) {

	                if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpire")) == NULL) {
	                    goto Redirection;
	                }
	                iTargetEmpireKey = pHttpForm->GetIntValue();

	                if ((pHttpForm = m_pHttpRequest->GetForm ("TargetEmpireSecret")) == NULL) {
	                    goto Redirection;
	                }
	                int64 i64TargetEmpireSecret = pHttpForm->GetInt64Value();

	                if (iTargetEmpireKey == m_iEmpireKey) {
	                    AddMessage ("You cannot obliterate yourself");
	                }

	                else if (iTargetEmpireKey == ROOT_KEY) {
	                    AddMessage ("You cannot obliterate " ROOT_NAME);
	                }

	                else if (iTargetEmpireKey == GUEST_KEY) {
	                    AddMessage ("You cannot obliterate " GUEST_NAME);
	                }

	                else {

	                    iErrCode = g_pGameEngine->ObliterateEmpire (iTargetEmpireKey, i64TargetEmpireSecret, m_iEmpireKey);

	                    if (iErrCode == ERROR_EMPIRE_DOES_NOT_EXIST) {
	                        AddMessage ("The empire no longer exists");
	                    } else {
	                        if (iErrCode == OK) {
	                            AddMessage ("The empire was successfully obliterated from the server");
	                        } else {
	                            char pszMessage [256];
	                            sprintf (pszMessage, "Error %i occurred while obliterating the empire", iErrCode);
	                            AddMessage (pszMessage);
	                        }
	                    }
	                }

	                bRedirectTest = false;
	                break;
	            }

	            }
	            break;

	        default:

	            Assert (false);
	        }

	    } else {
	        bRedirectTest = false;
	    }
	} 

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)

	switch (iEmpireAdminPage) {

	case 0:
	    {

	    
	Write ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"0\">") - 1);
	RenderSearchForms (true);

	    
	Write ("<p>Broadcast a message to all empires:<p><textarea name=\"Message\" rows=\"7\" cols=\"60\" wrap=\"physical\"></textarea><p>", sizeof ("<p>Broadcast a message to all empires:<p><textarea name=\"Message\" rows=\"7\" cols=\"60\" wrap=\"physical\"></textarea><p>") - 1);
	WriteButton (BID_SENDMESSAGE);

	    }

	    break;

	case 1:
	    {

	    
	Write ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"1\">", sizeof ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"1\">") - 1);
	Assert (piSearchEmpireKey != NULL);

	    RenderSearchResults (
	        sd,
	        pszFormName,
	        pszColName1,
	        pszColName2,
	        piSearchEmpireKey,
	        iNumSearchEmpires,
	        iLastKey
	        );

	    g_pGameEngine->FreeKeys (piSearchEmpireKey);

	    }

	    break;

	case 2:

	    {
	    
	Write ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"2\"><input type=\"hidden\" name=\"TargetEmpire\" value=\"", sizeof ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"2\"><input type=\"hidden\" name=\"TargetEmpire\" value=\"") - 1);
	Write (iTargetEmpireKey); 
	Write ("\">", sizeof ("\">") - 1);
	Variant vTargetPassword, vTargetName;
	    if (g_pGameEngine->GetEmpirePassword (iTargetEmpireKey, &vTargetPassword) != OK ||
	        g_pGameEngine->GetEmpireName (iTargetEmpireKey, &vTargetName) != OK) {
	        
	Write ("<strong>The empire no longer exists</strong>", sizeof ("<strong>The empire no longer exists</strong>") - 1);
	} else {

	        
	Write ("<p><strong>Change ", sizeof ("<p><strong>Change ") - 1);
	Write (vTargetName.GetCharPtr()); 
	Write ("'s password:</strong><p><table><tr><td><strong>New password</strong></td><td><input type=\"password\" name=\"NewPass\" size=\"20\" maxlength=\"", sizeof ("'s password:</strong><p><table><tr><td><strong>New password</strong></td><td><input type=\"password\" name=\"NewPass\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_PASSWORD_LENGTH); 
	            
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vTargetPassword.GetCharPtr()); 
	Write ("\"></td></tr><tr><td><strong>Confirm password</strong></td><td><input type=\"password\" name=\"NewPass2\" size=\"20\" maxlength=\"", sizeof ("\"></td></tr><tr><td><strong>Confirm password</strong></td><td><input type=\"password\" name=\"NewPass2\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_PASSWORD_LENGTH); 
	            
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (vTargetPassword.GetCharPtr()); 
	Write ("\"></td></tr></table>", sizeof ("\"></td></tr></table>") - 1);
	}

	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_CHANGEEMPIRESPASSWORD);

	    }
	    break;

	case 3:
	    {

	    
	Write ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"3\"><input type=\"hidden\" name=\"TargetEmpire\" value=\"", sizeof ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"3\"><input type=\"hidden\" name=\"TargetEmpire\" value=\"") - 1);
	Write (iTargetEmpireKey); 
	Write ("\">", sizeof ("\">") - 1);
	WriteProfile (iTargetEmpireKey, true, false, false);

	    
	Write ("<p>", sizeof ("<p>") - 1);
	if (m_iEmpireKey != iTargetEmpireKey) {

	        int iTargetPrivilege;
	        iErrCode = g_pGameEngine->GetEmpirePrivilege (iTargetEmpireKey, &iTargetPrivilege);

	        if (iErrCode == OK && iTargetPrivilege < ADMINISTRATOR || m_iEmpireKey == ROOT_KEY) {

	            WriteButton (BID_CHANGEEMPIRESPASSWORD);

	            if (iTargetEmpireKey != GUEST_KEY && iTargetEmpireKey != ROOT_KEY) {
	                WriteButton (BID_OBLITERATEEMPIRE);
	            }
	        }
	    }

	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_UPDATE);

	    }

	    break;

	case 4:
	    {

	    Variant vEmpireName, vSecretKey;

	    
	Write ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"4\"><input type=\"hidden\" name=\"TargetEmpire\" value=\"", sizeof ("<input type=\"hidden\" name=\"EmpireAdminPage\" value=\"4\"><input type=\"hidden\" name=\"TargetEmpire\" value=\"") - 1);
	Write (iTargetEmpireKey); 
	Write ("\"><p>", sizeof ("\"><p>") - 1);
	if (g_pGameEngine->GetEmpireName (iTargetEmpireKey, &vEmpireName) != OK ||
	        g_pGameEngine->GetEmpireProperty (iTargetEmpireKey, SystemEmpireData::SecretKey, &vSecretKey) != OK) {
	        
	Write ("That empire no longer exists<p>", sizeof ("That empire no longer exists<p>") - 1);
	WriteButton (BID_CANCEL);
	    } else {
	        
	Write ("<input type=\"hidden\" name=\"TargetEmpireSecret\" value=\"", sizeof ("<input type=\"hidden\" name=\"TargetEmpireSecret\" value=\"") - 1);
	Write (vSecretKey.GetInteger64()); 
	Write ("\">Are you sure you want to obliterate the ", sizeof ("\">Are you sure you want to obliterate the ") - 1);
	Write (vEmpireName.GetCharPtr()); 
	Write (" empire?<p>", sizeof (" empire?<p>") - 1);
	WriteButton (BID_CANCEL);
	        WriteButton (BID_OBLITERATEEMPIRE);
	    }

	    }
	    break;

	default:

	    Assert (false);
	    break;
	}

	SYSTEM_CLOSE


}