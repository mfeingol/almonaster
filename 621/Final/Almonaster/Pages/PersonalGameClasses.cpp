
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the PersonalGameClasses page
int HtmlRenderer::Render_PersonalGameClasses() {

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

	IHttpForm* pHttpForm;

	int i, iErrCode, iPersonalGameClassesPage = 0;
	unsigned int iGameClassKey = NO_KEY;

	char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

	    if (WasButtonPressed (BID_CANCEL)) {
	        bRedirectTest = false;
	    } else {

	        int iPersonalGameClassesPageSubmit;

	        if ((pHttpForm = m_pHttpRequest->GetForm ("PersonalGameClassesPage")) == NULL) {
	            goto Redirection;
	        }
	        iPersonalGameClassesPageSubmit = pHttpForm->GetIntValue();

	        int iGameNumber;
	        bool bFlag = false;

	        const char* pszStart;

	        switch (iPersonalGameClassesPageSubmit) {

	        case 0:

	            if (WasButtonPressed (BID_DELETEGAMECLASS) &&
	                (pHttpForm = m_pHttpRequest->GetForm ("DelGC")) != NULL) {

	                iGameClassKey = pHttpForm->GetIntValue();
	                bRedirectTest = false;

	                if (m_iSystemOptions & CONFIRM_IMPORTANT_CHOICES) {
	                    iPersonalGameClassesPage = 3;
	                    goto Redirection;
	                }

	                unsigned int iOwnerKey;
	                iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
	                if (iErrCode == OK) {

	                    if (m_iEmpireKey == iOwnerKey ||
	                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
	                        ) {

	                        iErrCode = g_pGameEngine->DeleteGameClass (iGameClassKey, &bFlag);

	                        if (iErrCode == OK) {
	                            if (bFlag) {
	                                AddMessage ("The GameClass was deleted");
	                            } else {
	                                AddMessage ("The GameClass has been marked for deletion");
	                            }
	                        }
	                        else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
	                            AddMessage ("The GameClass no longer exists");
	                        }
	                        else {
	                            AddMessage ("The gameclass could not be deleted; the error was ");
	                            AppendMessage (iErrCode);
	                        }
	                    }
	                }

	                goto Redirection;
	            }

	            if (WasButtonPressed (BID_UNDELETEGAMECLASS) &&
	                (pHttpForm = m_pHttpRequest->GetForm ("UndelGC")) != NULL) {

	                iGameClassKey = pHttpForm->GetIntValue();

	                unsigned int iOwnerKey;
	                iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
	                if (iErrCode == OK) {

	                    if (m_iEmpireKey == iOwnerKey ||
	                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
	                        ) {

	                        iErrCode = g_pGameEngine->UndeleteGameClass (iGameClassKey);
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

	                            AddMessage ("The gameclass could not be undeleted; the error was ");
	                            AppendMessage (iErrCode);
	                            break;
	                        }
	                    }
	                }

	                bRedirectTest = false;
	                goto Redirection;
	            }

	            if (WasButtonPressed (BID_HALTGAMECLASS) &&
	                (pHttpForm = m_pHttpRequest->GetForm ("HaltGC")) != NULL) {

	                iGameClassKey = pHttpForm->GetIntValue();

	                unsigned int iOwnerKey;
	                iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
	                if (iErrCode == OK) {

	                    if (m_iEmpireKey == iOwnerKey ||
	                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
	                        ) {

	                        iErrCode = g_pGameEngine->HaltGameClass (iGameClassKey);

	                        if (iErrCode == OK) {
	                            AddMessage ("The GameClass was halted");
	                        }
	                        else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
	                            AddMessage ("The GameClass no longer exists");
	                        }
	                        else {
	                            AddMessage ("The gameclass could not be halted; the error was ");
	                            AppendMessage (iErrCode);
	                        }
	                    }
	                }

	                bRedirectTest = false;
	                goto Redirection;
	            }

	            if (WasButtonPressed (BID_UNHALTGAMECLASS) &&
	                (pHttpForm = m_pHttpRequest->GetForm ("UnhaltGC")) != NULL) {

	                iGameClassKey = pHttpForm->GetIntValue();

	                unsigned int iOwnerKey;
	                iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
	                if (iErrCode == OK) {

	                    if (m_iEmpireKey == iOwnerKey ||
	                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
	                        ) {

	                        iErrCode = g_pGameEngine->UnhaltGameClass (iGameClassKey);
	                        switch (iErrCode) {

	                        case OK:

	                            AddMessage ("The GameClass was unhalted");
	                            break;

	                        case ERROR_GAMECLASS_DOES_NOT_EXIST:

	                            AddMessage ("The GameClass no longer exists");
	                            break;

	                        case ERROR_GAMECLASS_NOT_HALTED:

	                            AddMessage ("The GameClass was not halted");
	                            break;

	                        default:

	                            AddMessage ("The gameclass could not be unhalted; the error was ");
	                            AppendMessage (iErrCode);
	                            break;
	                        }
	                    }
	                }

	                bRedirectTest = false;
	                goto Redirection;
	            }

	            // Handle game start
	            if ((pHttpForm = m_pHttpRequest->GetFormBeginsWith ("Start")) != NULL && 
	                (pszStart = pHttpForm->GetName()) != NULL &&
	                sscanf (pszStart, "Start%d", &iGameClassKey) == 1) {

	                bRedirectTest = false;

	                // Check for advanced
	                char pszAdvanced [64];
	                sprintf (pszAdvanced, "Advanced%i", iGameClassKey);

	                if ((pHttpForm = m_pHttpRequest->GetForm (pszAdvanced)) != NULL) {
	                    iPersonalGameClassesPage = 1;
	                    break;
	                }

	                GameOptions goOptions;
	                iErrCode = g_pGameEngine->GetDefaultGameOptions (iGameClassKey, &goOptions);
	                if (iErrCode != OK) {

	                    if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
	                        AddMessage ("That gameclass no longer exists");
	                    } else {
	                        AddMessage ("Could not read default game options; the error was ");
	                        AppendMessage (iErrCode);
	                    }
	                    goto Redirection;
	                }

	                goOptions.iNumEmpires = 1;
	                goOptions.piEmpireKey = &m_iEmpireKey;

	                // Create the game
	                iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

	                HANDLE_CREATE_GAME_OUTPUT (iErrCode);
	            }

	            // Handle new gameclass creation
	            if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && WasButtonPressed (BID_CREATENEWGAMECLASS)) {
	                iPersonalGameClassesPage = 2;
	                bRedirectTest = false;
	                goto Redirection;
	            }

	        case 2:

	            // Handle new gameclass creation
	            if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES &&
	                WasButtonPressed (BID_CREATENEWGAMECLASS)) {

	                bRedirectTest = false;
	                if (ProcessCreateGameClassForms (m_iEmpireKey, NO_KEY) != OK) {
	                    iPersonalGameClassesPage = 2;
	                }
	                goto Redirection;
	            }

	            break;

	        case 3:

	            unsigned int iOwnerKey;

	            pHttpForm = m_pHttpRequest->GetForm ("GameClassKey");
	            if (pHttpForm != NULL && WasButtonPressed (BID_DELETEGAMECLASS)) {

	                iGameClassKey = pHttpForm->GetUIntValue();

	                iErrCode = g_pGameEngine->GetGameClassOwner (iGameClassKey, &iOwnerKey);
	                if (iErrCode == OK) {

	                    if (m_iEmpireKey == iOwnerKey ||
	                        (m_iPrivilege == ADMINISTRATOR && (iOwnerKey != ROOT_KEY || m_iEmpireKey == ROOT_KEY))
	                        ) {

	                        iErrCode = g_pGameEngine->DeleteGameClass (iGameClassKey, &bFlag);

	                        if (iErrCode == OK) {
	                            if (bFlag) {
	                                AddMessage ("The GameClass was deleted");
	                            } else {
	                                AddMessage ("The GameClass has been marked for deletion");
	                            }
	                        }
	                        else if (iErrCode == ERROR_GAMECLASS_DOES_NOT_EXIST) {
	                            AddMessage ("The GameClass no longer exists");
	                        }
	                        else {
	                            AddMessage ("The gameclass could not be deleted; the error was ");
	                            AppendMessage (iErrCode);
	                        }
	                    }
	                }
	            }

	        case 1:

	            // Check for choose
	            if (WasButtonPressed (BID_START) || WasButtonPressed (BID_BLOCK)) {

	                bRedirectTest = false;

	                if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassKey")) == NULL) {
	                    iPersonalGameClassesPage = 0;
	                    break;
	                }
	                iGameClassKey = pHttpForm->GetIntValue();

	                GameOptions goOptions;
	                InitGameOptions (&goOptions);

	                iErrCode = ParseGameConfigurationForms (iGameClassKey, NO_KEY, NULL, &goOptions);
	                if (iErrCode != OK) {
	                    iPersonalGameClassesPage = 1;
	                    break;
	                }

	                goOptions.iNumEmpires = 1;
	                goOptions.piEmpireKey = &m_iEmpireKey;

	                // Create the game
	                iErrCode = g_pGameEngine->CreateGame (iGameClassKey, m_iEmpireKey, goOptions, &iGameNumber);

	                ClearGameOptions (&goOptions);

	                HANDLE_CREATE_GAME_OUTPUT (iErrCode);
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

	switch (iPersonalGameClassesPage) {

	case 0:

	    {

	    int iNumGameClasses, * piGameClassKey = NULL, * piOptions;
	    Variant* pvName = NULL, vMaxNumPGC;

	    unsigned int iNumHalted = 0, iNumUnhalted = 0, iNumMarkedForDeletion = 0;

	    
	Write ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"0\">") - 1);
	if (m_iPrivilege < PRIVILEGE_FOR_PERSONAL_GAMECLASSES) {

	        
	Write ("<p><h3>You must be an ", sizeof ("<p><h3>You must be an ") - 1);
	Write (PRIVILEGE_STRING [PRIVILEGE_FOR_PERSONAL_GAMECLASSES]);
	        
	Write (" to create personal GameClasses</h3>", sizeof (" to create personal GameClasses</h3>") - 1);
	float fScore;
	        if (g_pGameEngine->GetScoreForPrivilege (PRIVILEGE_FOR_PERSONAL_GAMECLASSES, &fScore) == OK) {

	            
	Write ("<p>(To reach <strong>", sizeof ("<p>(To reach <strong>") - 1);
	Write (PRIVILEGE_STRING [PRIVILEGE_FOR_PERSONAL_GAMECLASSES]);
	            
	Write ("</strong> status, you need an <em>Almonaster</em> score of <strong>", sizeof ("</strong> status, you need an <em>Almonaster</em> score of <strong>") - 1);
	Write (fScore);
	            
	Write ("</strong>)", sizeof ("</strong>)") - 1);
	}
	    }

	    Check (g_pGameEngine->GetSystemProperty (SystemData::MaxNumPersonalGameClasses, &vMaxNumPGC));
	    Check (g_pGameEngine->GetEmpirePersonalGameClasses (m_iEmpireKey, &piGameClassKey, &pvName, &iNumGameClasses));

	    if (iNumGameClasses == 0) {

	        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES) {

	            
	Write ("<p>You have <strong>0</strong> personal GameClasses", sizeof ("<p>You have <strong>0</strong> personal GameClasses") - 1);
	if (vMaxNumPGC.GetInteger() > 0) {
	                
	Write ("<p><table width=\"75%\"><tr><td>Create a new GameClass:</td><td>", sizeof ("<p><table width=\"75%\"><tr><td>Create a new GameClass:</td><td>") - 1);
	WriteButton (BID_CREATENEWGAMECLASS); 
	Write ("</td></tr></table>", sizeof ("</td></tr></table>") - 1);
	}
	        }

	    } else {

	        
	Write ("<p>You have <strong>", sizeof ("<p>You have <strong>") - 1);
	Write (iNumGameClasses); 
	Write ("</strong> personal GameClass", sizeof ("</strong> personal GameClass") - 1);
	if (iNumGameClasses != 1) {
	            
	Write ("es", sizeof ("es") - 1);
	}

	        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && iNumGameClasses >= vMaxNumPGC.GetInteger()) {
	            
	Write (" and can create no more", sizeof (" and can create no more") - 1);
	}

	        piOptions = (int*) StackAlloc (iNumGameClasses * sizeof (int));

	        for (i = 0; i < iNumGameClasses; i ++) {

	            iErrCode = g_pGameEngine->GetGameClassOptions (piGameClassKey[i], piOptions + i);
	            if (iErrCode != OK) {
	                piOptions[i] = 0;
	            } else {

	                if (piOptions[i] & GAMECLASS_HALTED) {
	                    iNumHalted ++;
	                } else {
	                    iNumUnhalted ++;
	                }

	                if (piOptions[i] & GAMECLASS_MARKED_FOR_DELETION) {
	                    iNumMarkedForDeletion ++;
	                }
	            }
	        }

	        
	Write ("<p><table>", sizeof ("<p><table>") - 1);
	// Delete
	        if (iNumGameClasses - iNumMarkedForDeletion > 0) {

	            
	Write ("<tr><td>Delete a personal GameClass</td><td><select name=\"DelGC\">", sizeof ("<tr><td>Delete a personal GameClass</td><td><select name=\"DelGC\">") - 1);
	for (i = 0; i < iNumGameClasses; i ++) {

	                if (!(piOptions[i] & GAMECLASS_MARKED_FOR_DELETION)) {

	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piGameClassKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pvName[i].GetCharPtr());
	                    
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_DELETEGAMECLASS);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	        // Undelete
	        if (iNumMarkedForDeletion > 0) {

	            
	Write ("<tr><td>Undelete a personal GameClass</td><td><select name=\"UndelGC\">", sizeof ("<tr><td>Undelete a personal GameClass</td><td><select name=\"UndelGC\">") - 1);
	for (i = 0; i < iNumGameClasses; i ++) {

	                if (piOptions[i] & GAMECLASS_MARKED_FOR_DELETION) {

	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piGameClassKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pvName[i].GetCharPtr());
	                    
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_UNDELETEGAMECLASS);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	        // Halt
	        if (iNumUnhalted > 0) {

	            
	Write ("<tr><td>Halt a personal GameClass</td><td><select name=\"HaltGC\">", sizeof ("<tr><td>Halt a personal GameClass</td><td><select name=\"HaltGC\">") - 1);
	for (i = 0; i < iNumGameClasses; i ++) {

	                if (!(piOptions[i] & GAMECLASS_HALTED)) {

	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piGameClassKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pvName[i].GetCharPtr());
	                    
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_HALTGAMECLASS);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	        // Halt
	        if (iNumHalted > 0) {

	            
	Write ("<tr><td>Unhalt a personal GameClass</td><td><select name=\"UnhaltGC\">", sizeof ("<tr><td>Unhalt a personal GameClass</td><td><select name=\"UnhaltGC\">") - 1);
	for (i = 0; i < iNumGameClasses; i ++) {

	                if (piOptions[i] & GAMECLASS_HALTED) {

	                    
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piGameClassKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pvName[i].GetCharPtr());
	                    
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td>", sizeof ("</select></td><td>") - 1);
	WriteButton (BID_UNHALTGAMECLASS);
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	        if (m_iPrivilege >= PRIVILEGE_FOR_PERSONAL_GAMECLASSES && iNumGameClasses < vMaxNumPGC.GetInteger()) {
	            
	Write ("<tr><td>Create a new GameClass</td><td></td><td>", sizeof ("<tr><td>Create a new GameClass</td><td></td><td>") - 1);
	WriteButton (BID_CREATENEWGAMECLASS); 
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	        
	Write ("</table>", sizeof ("</table>") - 1);
	//
	        // Start games
	        //

	        
	Write ("<p><h3>Start a new game:</h3>", sizeof ("<p><h3>Start a new game:</h3>") - 1);
	Variant* pvGameClassInfo = NULL;

	        WriteSystemGameListHeader (m_vTableColor.GetCharPtr());
	        for (i = 0; i < iNumGameClasses; i ++) {

	            // Read game class data
	            if (g_pGameEngine->GetGameClassData (piGameClassKey[i], &pvGameClassInfo) == OK) {

	                // Best effort
	                iErrCode = WriteSystemGameListData (
	                    piGameClassKey[i], 
	                    pvGameClassInfo
	                    );
	            }

	            if (pvGameClassInfo != NULL) {
	                g_pGameEngine->FreeData (pvGameClassInfo);
	                pvGameClassInfo = NULL;
	            }
	        }

	        
	Write ("</table>", sizeof ("</table>") - 1);
	g_pGameEngine->FreeKeys (piGameClassKey);
	        g_pGameEngine->FreeData (pvName);
	    }

	    }
	    break;

	case 1:

	    int iGameNumber;

	    Check (g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName));
	    Check (g_pGameEngine->GetNextGameNumber (iGameClassKey, &iGameNumber));

	    
	Write ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"1\"><input type=\"hidden\" name=\"GameClassKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"1\"><input type=\"hidden\" name=\"GameClassKey\" value=\"") - 1);
	Write (iGameClassKey); 
	Write ("\"><h3>Advanced game creation options:<p>", sizeof ("\"><h3>Advanced game creation options:<p>") - 1);
	Write (pszGameClassName); 
	Write (" ", sizeof (" ") - 1);
	Write (iGameNumber);

	    
	Write ("</h3><p>", sizeof ("</h3><p>") - 1);
	RenderGameConfiguration (iGameClassKey, NO_KEY);

	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_START);

	    break;

	case 2:

	    
	Write ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"2\"><p><h3>Create a new GameClass:</h3>", sizeof ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"2\"><p><h3>Create a new GameClass:</h3>") - 1);
	WriteCreateGameClassString (m_iEmpireKey, NO_KEY, false);
	    
	Write ("<p>", sizeof ("<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_CREATENEWGAMECLASS);

	    break;

	case 3:

	    
	Write ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"3\"><input type=\"hidden\" name=\"GameClassKey\" value=\"", sizeof ("<input type=\"hidden\" name=\"PersonalGameClassesPage\" value=\"3\"><input type=\"hidden\" name=\"GameClassKey\" value=\"") - 1);
	Write (iGameClassKey); 
	Write ("\">", sizeof ("\">") - 1);
	Check (g_pGameEngine->GetGameClassName (iGameClassKey, pszGameClassName));

	    
	Write ("<p>Are you sure you want to delete <strong>", sizeof ("<p>Are you sure you want to delete <strong>") - 1);
	Write (pszGameClassName); 
	Write ("</strong>?<p>", sizeof ("</strong>?<p>") - 1);
	WriteButton (BID_CANCEL);
	    WriteButton (BID_DELETEGAMECLASS);

	    break;

	default:

	    Assert (false);
	    break;
	}

	SYSTEM_CLOSE


}