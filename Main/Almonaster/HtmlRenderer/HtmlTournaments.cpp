//
// Almonaster.dll:  a component of Almonaster
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

#include "HtmlRenderer.h"


void HtmlRenderer::WriteTournamentIcon (int iIconKey, int iTournamentKey, const char* pszAlt, bool bVerifyUpload) {

    WriteIcon (iIconKey, iTournamentKey, NO_KEY, pszAlt, BASE_UPLOADED_TOURNAMENT_ICON_DIR, bVerifyUpload);
}

void HtmlRenderer::WriteTournamentTeamIcon (int iIconKey, int iTournamentKey, int iTournamentTeamKey, 
                                            const char* pszAlt, bool bVerifyUpload) {

    WriteIcon (iIconKey, iTournamentKey, iTournamentTeamKey, pszAlt, BASE_UPLOADED_TOURNAMENT_TEAM_ICON_DIR, bVerifyUpload);
}

void HtmlRenderer::WriteAdministerTournament (unsigned int iTournamentKey) {

    int iErrCode, * piOptions = NULL, * piGameClass = NULL, * piGameNumber = NULL;
    Variant* pvData = NULL, * pvGameClassName = NULL, * pvEmpireName = NULL, * pvTeamName = NULL;

    String strDesc, strUrl, strNews;

    unsigned int i, * piGameClassKey = NULL, iNumGameClasses, iNumEmpires, * piTeamKey = NULL, iNumTeams;
    unsigned int iNumHalted = 0, iNumNotHalted = 0, iNumMarked = 0, iNumUnmarked = 0, iNumStartable = 0;
    unsigned int iGames, * piEmpireKey = NULL;

    iErrCode = g_pGameEngine->GetTournamentData (iTournamentKey, &pvData);
    if (iErrCode != OK) {
        OutputText ("<p><strong>The tournament does not exist</strong>");
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentGameClasses (iTournamentKey, &piGameClassKey, &pvGameClassName, &iNumGameClasses);
    if (iErrCode != OK) {
        OutputText ("<p><strong>The tournament does not exist</strong>");
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentEmpires (iTournamentKey, &piEmpireKey, NULL, &pvEmpireName, &iNumEmpires);
    if (iErrCode != OK) {
        OutputText ("<p><strong>The tournament does not exist</strong>");
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentTeams (iTournamentKey, &piTeamKey, &pvTeamName, &iNumTeams);
    if (iErrCode != OK) {
        OutputText ("<p><strong>The tournament does not exist</strong>");
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentGames (iTournamentKey, &piGameClass, &piGameNumber, &iGames);
    if (iErrCode != OK) {
        OutputText ("<p><strong>The tournament does not exist</strong>");
        goto Cleanup;
    }

    if (iNumGameClasses > 0) {

        piOptions = (int*) StackAlloc (iNumGameClasses * sizeof (int));

        for (i = 0; i < iNumGameClasses; i ++) {

            iErrCode = g_pGameEngine->GetGameClassOptions (piGameClassKey[i], piOptions + i);
            if (iErrCode != OK) {
                OutputText ("<p><strong>A gameclass does not exist</strong>");
                goto Cleanup;
            }

            if (piOptions[i] & GAMECLASS_HALTED) {
                iNumHalted ++;
            } else {
                iNumNotHalted ++;
            }

            if (piOptions[i] & GAMECLASS_MARKED_FOR_DELETION) {
                iNumMarked ++;
            } else {
                iNumUnmarked ++;
            }

            if (!(piOptions[i] & GAMECLASS_HALTED) && !(piOptions[i] & GAMECLASS_MARKED_FOR_DELETION)) {
                iNumStartable ++;
            }
        }
    }

    if (HTMLFilter (pvData[SystemTournaments::iDescription].GetCharPtr(), &strDesc, 0, false) != OK ||
        HTMLFilter (pvData[SystemTournaments::iWebPage].GetCharPtr(), &strUrl, 0, false) != OK ||
        HTMLFilter (pvData[SystemTournaments::iNews].GetCharPtr(), &strNews, 0, false) != OK) {
        OutputText ("<p><strong>The server is out of memory</strong>");
        goto Cleanup;
    }

    OutputText ("<p>");
    WriteTournamentIcon (pvData[SystemTournaments::iIcon].GetInteger(), iTournamentKey, NULL, false);

    OutputText (" <font size=\"+1\"><strong>Administer the ");
    m_pHttpResponse->WriteText (pvData[SystemTournaments::iName].GetCharPtr());
    OutputText (
        " tournament:</strong></font>"\
        "<p><table width=\"90%\">"\
        "<tr>"\
        "<td>Description:</td><td>"\
        "<textarea rows=\"4\" cols=\"40\" wrap=\"virtual\" name=\"TournamentDescription\">"
        );

    m_pHttpResponse->WriteText (strDesc.GetCharPtr(), strDesc.GetLength());

    OutputText (
        "</textarea>"\
        "</td>"\
        "</tr>"\

        "<tr>"\
        "<td>Webpage:</td><td>"\
        "<input type=\"text\" size=\"40\" maxlength=\""
        );
    m_pHttpResponse->WriteText (MAX_WEB_PAGE_LENGTH);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strUrl.GetCharPtr(), strUrl.GetLength());
    OutputText (
        "\" name=\"WebPageURL\">"\
        "</td>"\
        "</tr>"\

        "<tr>"\
        "<td>"\
        "Choose an icon:"\
        "</td>"\
        "<td>"

        "<select name=\"IconSelect\">"
        "<option "
        );

    if (pvData[SystemTournaments::iIcon].GetInteger() != NO_KEY) {
        OutputText ("selected ");
    }

    OutputText (
        "value=\"0\">An icon from the system set</option>"\
        "<option "
        );

    if (pvData[SystemTournaments::iIcon].GetInteger() == NO_KEY) {
        OutputText ("selected ");
    }
        
    OutputText (
        "value=\"1\">An uploaded icon</option>"\
        "</select> "
        );

    WriteButton (BID_CHOOSE);

    OutputText (
        "</td>"\
        "</tr>"

        "<tr>"\
        "<td>Create a tournament GameClass:</td><td>"
        );

    WriteButton (BID_CREATENEWGAMECLASS);

    OutputText (
        "</td>"\
        "</tr>"\
        );

    if (iNumUnmarked > 0) {
        
        OutputText (
            
            "<tr>"\
            "<td>Delete a tournament GameClass:</td><td>"\
            "<select name=\"DeleteGC\">"\
            );
        
        for (i = 0; i < iNumGameClasses; i ++) {
            
            if (!(piOptions[i] & GAMECLASS_MARKED_FOR_DELETION)) {
                
                OutputText ("<option value=\"");
                m_pHttpResponse->WriteText (piGameClassKey[i]);
                OutputText ("\">");
                m_pHttpResponse->WriteText (pvGameClassName[i].GetCharPtr());
                OutputText ("</option>");
            }
        }
        
        OutputText ("</select> ");
        WriteButton (BID_DELETEGAMECLASS);
        
        OutputText (
            "</td>"\
            "</tr>"\
            );
    }
    
    if (iNumMarked > 0) {
        
        OutputText (
            
            "<tr>"\
            "<td>Undelete a tournament GameClass:</td><td>"\
            "<select name=\"UndeleteGC\">"\
            );
        
        for (i = 0; i < iNumGameClasses; i ++) {
            
            if (piOptions[i] & GAMECLASS_MARKED_FOR_DELETION) {
                
                OutputText ("<option value=\"");
                m_pHttpResponse->WriteText (piGameClassKey[i]);
                OutputText ("\">");
                m_pHttpResponse->WriteText (pvGameClassName[i].GetCharPtr());
                OutputText ("</option>");
            }
        }
        
        OutputText ("</select> ");
        WriteButton (BID_UNDELETEGAMECLASS);
        
        OutputText (
            "</td>"\
            "</tr>"\
            );
    }
    
    if (iNumNotHalted > 0) {
        
        OutputText (
            
            "<tr>"\
            "<td>Halt a tournament GameClass:</td><td>"\
            "<select name=\"HaltGC\">"\
            );
        
        for (i = 0; i < iNumGameClasses; i ++) {
            
            if (!(piOptions[i] & GAMECLASS_HALTED)) {
                
                OutputText ("<option value=\"");
                m_pHttpResponse->WriteText (piGameClassKey[i]);
                OutputText ("\">");
                m_pHttpResponse->WriteText (pvGameClassName[i].GetCharPtr());
                OutputText ("</option>");
            }
        }
        
        OutputText ("</select> ");
        WriteButton (BID_HALTGAMECLASS);
        
        OutputText (
            "</td>"\
            "</tr>"\
            );
    }
    
    if (iNumHalted > 0) {
        
        OutputText (
            
            "<tr>"\
            "<td>Unhalt a tournament GameClass:</td><td>"\
            "<select name=\"UnhaltGC\">"\
            );
        
        for (i = 0; i < iNumGameClasses; i ++) {
            
            if (piOptions[i] & GAMECLASS_HALTED) {
                
                OutputText ("<option value=\"");
                m_pHttpResponse->WriteText (piGameClassKey[i]);
                OutputText ("\">");
                m_pHttpResponse->WriteText (pvGameClassName[i].GetCharPtr());
                OutputText ("</option>");
            }
        }
        
        OutputText ("</select> ");
        WriteButton (BID_UNHALTGAMECLASS);
        
        OutputText (
            "</td>"\
            "</tr>"\
            );
    }

    if (iNumStartable > 0 && iNumEmpires > 1) {

        OutputText (    
            "<tr>"\
            "<td>Start a new tournament game:</td><td>"
            );

        WriteButton (BID_START);
        
        OutputText (
            "</td>"\
            "</tr>"\
            );
    }

    // If there are games in progress, offer BID_ADMINISTERGAME
    if (iGames > 0) {

        OutputText (
            "<tr>"\
            "<td>View active tournament games:</td><td>"\
            );

        WriteButton (BID_VIEWGAMEINFORMATION);
        
        OutputText (
            "</td>"\
            "</tr>"\
            );
    }

    // Invite
    OutputText (
        "<tr>"\
        "<td>Invite an empire to join the tournament:</td>"\
        "<td><input name=\"InviteEmpireName\" size=\"20\" maxlength=\""
        );
    
    m_pHttpResponse->WriteText (MAX_EMPIRE_NAME_LENGTH);
    OutputText ("\"> ");
    WriteButton (BID_INVITEEMPIRE);

    OutputText (
        "</td>"\
        "</tr>"\
        );

    if (iNumEmpires > 0) {

        OutputText (
            "<tr>"\
            "<td>Delete an empire from the tournament:</td><td>"\
            "<select name=\"DelEmp\">"\
            );
        
        for (i = 0; i < iNumEmpires; i ++) {
            
            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (piEmpireKey[i]);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pvEmpireName[i].GetCharPtr());
            OutputText ("</option>");
        }
        
        OutputText ("</select> ");

        WriteButton (BID_DELETEEMPIRE);

        OutputText (
            "</td>"\
            "</tr>"\
            );
    }

    OutputText (
        "<tr>"\
        "<td>Create a team:</td><td>"\
        );

    WriteButton (BID_CREATETEAM);

    OutputText (
        "</td>"\
        "</tr>"
        );

    if (iNumTeams > 0) {

        OutputText (
            "<tr>"\
            "<td>Administer a team:</td><td>"\
            "<select name=\"AdminTeam\">"\
            );
        
        for (i = 0; i < iNumTeams; i ++) {
            
            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (piTeamKey[i]);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pvTeamName[i].GetCharPtr());
            OutputText ("</option>");
        }
        
        OutputText ("</select> ");

        WriteButton (BID_ADMINISTERTEAM);

        OutputText (
            "</td>"\
            "</tr>"\

            "<tr>"\
            "<td>Delete a team from the tournament:</td><td>"\
            "<select name=\"DelTeam\">"\
            );
        
        for (i = 0; i < iNumTeams; i ++) {
            
            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (piTeamKey[i]);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pvTeamName[i].GetCharPtr());
            OutputText ("</option>");
        }
        
        OutputText ("</select> ");

        WriteButton (BID_DELETETEAM);

        OutputText (
            "</td>"\
            "</tr>"\
            );
    }

    OutputText (
        "<tr>"\
        "<td>News:</td><td>"\
        "<textarea rows=\"6\" cols=\"60\" wrap=\"virtual\" name=\"TournamentNews\">"
        );

    m_pHttpResponse->WriteText (strNews.GetCharPtr(), strNews.GetLength());

    OutputText (
        "</textarea>"\
        "</td>"\
        "</tr>"
        );

    OutputText ("</table><p>");

    WriteButton (BID_CANCEL);
    WriteButton (BID_UPDATE);

Cleanup:

    if (pvData != NULL) {
        g_pGameEngine->FreeData (pvData);
    }

    if (piGameClassKey != NULL) {
        g_pGameEngine->FreeKeys (piGameClassKey);
    }

    if (piEmpireKey != NULL) {
        g_pGameEngine->FreeData (piEmpireKey);  // Not a bug
    }

    if (pvEmpireName != NULL) {
        delete [] pvEmpireName;
    }

    if (pvGameClassName != NULL) {
        delete [] pvGameClassName;
    }

    if (piTeamKey != NULL) {
        g_pGameEngine->FreeKeys (piTeamKey);
    }

    if (pvTeamName != NULL) {
        g_pGameEngine->FreeData (pvTeamName);
    }

    if (piGameClass != NULL) {
        delete [] piGameClass;
    }

    if (piGameNumber != NULL) {
        delete [] piGameNumber;
    }
}

void HtmlRenderer::WriteAdministerTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) {

    int iErrCode;

    Variant* pvData = NULL, * pvEmpireName = NULL;
    unsigned int* piEmpireKey = NULL, i, iNumEmpires, * piTeamKey = NULL;

    String strDesc, strUrl;

    iErrCode = g_pGameEngine->GetTournamentTeamData (iTournamentKey, iTeamKey, &pvData);
    if (iErrCode != OK) {
        OutputText ("<p><strong>The team does not exist</strong>");
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentEmpires (iTournamentKey, &piEmpireKey, &piTeamKey, &pvEmpireName, &iNumEmpires);
    if (iErrCode != OK) {
        OutputText ("The tournament does not exist");
        goto Cleanup;
    }

    if (HTMLFilter (pvData[SystemTournamentTeams::iDescription].GetCharPtr(), &strDesc, 0, false) != OK ||
        HTMLFilter (pvData[SystemTournamentTeams::iWebPage].GetCharPtr(), &strUrl, 0, false) != OK) {
        OutputText ("<p><strong>The server is out of memory</strong>");
        goto Cleanup;
    }

    OutputText ("<p>");
    WriteTournamentTeamIcon (pvData[SystemTournamentTeams::iIcon].GetInteger(), iTournamentKey, iTeamKey, NULL, false);

    OutputText (" <font size=\"+1\"><strong>Administer the ");
    m_pHttpResponse->WriteText (pvData[SystemTournamentTeams::iName].GetCharPtr());
    OutputText (
        " team:</strong></font>"\
        "<p><table width=\"65%\">"\
        "<tr>"\
        "<td>Description:</td><td>"\
        "<textarea rows=\"4\" cols=\"40\" wrap=\"virtual\" name=\"TeamDescription\">"
        );

    m_pHttpResponse->WriteText (strDesc.GetCharPtr(), strDesc.GetLength());

    OutputText (
        "</textarea>"\
        "</td>"\
        "</tr>"\

        "<tr>"\
        "<td>Webpage:</td><td>"\
        "<input type=\"text\" size=\"40\" maxlength=\""
        );
    m_pHttpResponse->WriteText (MAX_WEB_PAGE_LENGTH);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strUrl.GetCharPtr(), strUrl.GetLength());
    OutputText (
        "\" name=\"TeamWebPageURL\">"\
        "</td>"\
        "</tr>"\
        );

    OutputText (
        "<tr>"\
        "<td>"\
        "Choose an icon:"\
        "</td>"\
        "<td>"

        "<select name=\"IconSelect\">"
        "<option "
        );

    if (pvData[SystemTournamentTeams::iIcon].GetInteger() != NO_KEY) {
        OutputText ("selected ");
    }

    OutputText (
        "value=\"0\">An icon from the system set</option>"\
        "<option "
        );

    if (pvData[SystemTournamentTeams::iIcon].GetInteger() == NO_KEY) {
        OutputText ("selected ");
    }
        
    OutputText (
        "value=\"1\">An uploaded icon</option>"\
        "</select> "
        );

    WriteButton (BID_CHOOSE);

    OutputText (
        "</td>"\
        "</tr>"
        );

    for (i = 0; i < iNumEmpires; i ++) {
        
        if (piTeamKey[i] != iTeamKey) {
            
            OutputText (
                "<tr>"\
                "<td>Add an empire to this team:</td><td>"\
                "<select name=\"JoinTeam\">"\
                );
            
            for (; i < iNumEmpires; i ++) {
                
                if (piTeamKey[i] != iTeamKey) {

                    OutputText ("<option value=\"");
                    m_pHttpResponse->WriteText (piEmpireKey[i]);
                    OutputText ("\">");
                    m_pHttpResponse->WriteText (pvEmpireName[i].GetCharPtr());
                    OutputText ("</option>");
                }
            }
            
            OutputText ("</select> ");
            
            WriteButton (BID_ADDEMPIRE);
            
            OutputText (
                "</td>"\
                "</tr>"
                );

            break;
        }
    }

    for (i = 0; i < iNumEmpires; i ++) {
        
        if (piTeamKey[i] == iTeamKey) {
            
            OutputText (
                "<tr>"\
                "<td>Delete an empire from this team:</td><td>"\
                "<select name=\"DelFromTeam\">"\
                );
            
            for (; i < iNumEmpires; i ++) {

                if (piTeamKey[i] == iTeamKey) {
                
                    OutputText ("<option value=\"");
                    m_pHttpResponse->WriteText (piEmpireKey[i]);
                    OutputText ("\">");
                    m_pHttpResponse->WriteText (pvEmpireName[i].GetCharPtr());
                    OutputText ("</option>");
                }
            }
            
            OutputText ("</select> ");
            
            WriteButton (BID_DELETEEMPIRE);
            
            OutputText (
                "</td>"\
                "</tr>"
                );

            break;
        }
    }

    OutputText ("</table><p>");

    WriteButton (BID_CANCEL);
    WriteButton (BID_UPDATE);

Cleanup:

    if (pvData != NULL) {
        g_pGameEngine->FreeData (pvData);
    }

    if (piEmpireKey != NULL) {
        g_pGameEngine->FreeData (piEmpireKey);  // Not a bug
    }

    if (piTeamKey != NULL) {
        g_pGameEngine->FreeData (piTeamKey); // Not a bug
    }

    if (pvEmpireName != NULL) {
        delete [] pvEmpireName;
    }
}

void HtmlRenderer::WriteCreateTournamentTeam (unsigned int iTournamentKey) {

    IHttpForm* pHttpForm;

    String strName, strDesc, strUrl;

    if ((pHttpForm = m_pHttpRequest->GetForm ("TeamName")) != NULL) {
        if (HTMLFilter (pHttpForm->GetValue(), &strName, 0, false) != OK) {
            strName = "";
        }
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TeamDescription")) != NULL) {
        if (HTMLFilter (pHttpForm->GetValue(), &strDesc, 0, false) != OK) {
            strDesc = "";
        }
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TeamWebPageURL")) != NULL) {
        if (HTMLFilter (pHttpForm->GetValue(), &strUrl, 0, false) != OK) {
            strUrl = "";
        }
    }

    // Name
    OutputText (
        "<p>"\
        "<table width=\"75%\">"\
        "<tr>"\
        "<td>Name:</td><td><input type=\"text\" size=\""
        );
    m_pHttpResponse->WriteText (MAX_TOURNAMENT_NAME_LENGTH);
    OutputText ("\" maxlength=\"");
    m_pHttpResponse->WriteText (MAX_TOURNAMENT_NAME_LENGTH);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strName.GetCharPtr(), strName.GetLength());
    OutputText ("\" name=\"TeamName\"></td></tr>");
    
    // Description
    OutputText (
        "<tr>"\
        "<td>Description:</td><td>"\
        "<textarea rows=\"4\" cols=\"40\" wrap=\"virtual\" name=\"TeamDescription\">"
        );

    m_pHttpResponse->WriteText (strDesc.GetCharPtr(), strDesc.GetLength());
        
    OutputText (
        "</textarea></td>"\
        "</tr>"\

        "<tr>"\
        "<td>Webpage:</td><td><input type=\"text\" size=\"40\" maxlength=\"");
    m_pHttpResponse->WriteText (MAX_WEB_PAGE_LENGTH);
    OutputText ("\" value=\"");
    m_pHttpResponse->WriteText (strUrl.GetCharPtr(), strUrl.GetLength());
    OutputText (
        "\" name=\"TeamWebPageURL\"></td></tr>"\

        "</table>"
        );
}

int HtmlRenderer::ProcessCreateTournamentTeam (unsigned int iTournamentKey) {

    int iErrCode;
    unsigned int iTournamentTeamKey;

    Variant pvSubmitArray [SystemTournamentTeams::NumColumns];
    
    // Parse the forms
    iErrCode = ParseCreateTournamentTeamForms (pvSubmitArray, iTournamentKey);
    if (iErrCode != OK) {
        return iErrCode;
    }
    
    // Create the tournament, finally
    iErrCode = g_pGameEngine->CreateTournamentTeam (iTournamentKey, pvSubmitArray, &iTournamentTeamKey);
    switch (iErrCode) {

    case OK:
        AddMessage ("The team was created");
        break;
    case ERROR_TOURNAMENT_TEAM_ALREADY_EXISTS:
        AddMessage ("The new team name already exists");
        break;
    case ERROR_NAME_IS_TOO_LONG:
        AddMessage ("The new team name is too long");
        break;
    case ERROR_DESCRIPTION_IS_TOO_LONG:
        AddMessage ("The new team description is too long");
        break;
    default:
        AddMessage ("The team could not be created; the error was ");
        AppendMessage (iErrCode);
        break;
    }
    
    return iErrCode;
}

int HtmlRenderer::ParseCreateTournamentTeamForms (Variant* pvSubmitArray, unsigned int iTournamentKey) {

    IHttpForm* pHttpForm;

    // Name
    pHttpForm = m_pHttpRequest->GetForm ("TeamName");
    if (pHttpForm == NULL) {
        AddMessage ("Missing TeamName form");
        return ERROR_FAILURE;
    }

    if (VerifyCategoryName ("Team", pHttpForm->GetValue(), MAX_TOURNAMENT_TEAM_NAME_LENGTH, true) != OK) {
        return ERROR_FAILURE;
    }

    pvSubmitArray [SystemTournamentTeams::iName] = pHttpForm->GetValue();

    // Description
    pHttpForm = m_pHttpRequest->GetForm ("TeamDescription");
    if (pHttpForm == NULL) {
        AddMessage ("Missing TeamDescription form");
        return ERROR_FAILURE;
    }

    if (String::StrLen (pHttpForm->GetValue()) > MAX_TOURNAMENT_TEAM_DESCRIPTION_LENGTH) {
        AddMessage ("The description is too long");
        return ERROR_FAILURE;
    }

    pvSubmitArray [SystemTournamentTeams::iDescription] = pHttpForm->GetValue();

    // URL
    pHttpForm = m_pHttpRequest->GetForm ("TeamWebPageURL");
    if (pHttpForm == NULL) {
        AddMessage ("Missing TeamWebPageURL form");
        return ERROR_FAILURE;
    }

    if (String::StrLen (pHttpForm->GetValue()) > MAX_WEB_PAGE_LENGTH) {
        AddMessage ("The description is too long");
        return ERROR_FAILURE;
    }

    // Web page
    pvSubmitArray [SystemTournamentTeams::iWebPage] = pHttpForm->GetValue();

    // Icon
    pvSubmitArray [SystemTournamentTeams::iIcon] = GetDefaultSystemIcon();

    return OK;
}

int HtmlRenderer::StartTournamentGame(unsigned int iTournamentKey, int iTeamOptions, bool bAdvanced) {

    int iErrCode, iGameNumber;
    unsigned int* piEmpireKey = NULL, * piJoinedKey = NULL;

    unsigned int i, j, iGameClass, * piTeamEmpireKey = NULL, iCheckKey, iTotalEmpires = 0, iMaxNumEmpires, 
        * piTeamKey = NULL, iNumTeams, iNumEmpires, * piJoinedTeamKey;

    IHttpForm* pHttpForm;

    GameOptions goOptions;
    InitGameOptions (&goOptions);

    // Get teams
    iErrCode = g_pGameEngine->GetTournamentTeams (
        iTournamentKey,
        &piTeamKey,
        NULL,
        &iNumTeams
        );

    if (iErrCode != OK) {
        AddMessage ("Could not read tournament teams. The error was ");
        AppendMessage (iErrCode);
        goto Cleanup;
    }

    // Get empires
    iErrCode = g_pGameEngine->GetAvailableTournamentEmpires (
        iTournamentKey,
        &piEmpireKey,
        &piTeamEmpireKey,
        NULL,
        &iNumEmpires
        );

    if (iErrCode != OK) {
        AddMessage ("Could not read tournament empires. The error was ");
        AppendMessage (iErrCode);
        goto Cleanup;
    }

    // Get gameclass
    pHttpForm = m_pHttpRequest->GetForm ("GameClassKey");
    if (pHttpForm == NULL) {
        AddMessage ("Missing GameClassKey form");
        iErrCode = ERROR_FAILURE;
        goto Cleanup;
    }
    iGameClass = pHttpForm->GetIntValue();

    iErrCode = g_pGameEngine->GetGameClassTournament (iGameClass, &iCheckKey);
    if (iErrCode != OK || iCheckKey != iTournamentKey) {
        AddMessage ("The gameclass does not belong to the tournament");
        iErrCode = ERROR_GAMECLASS_IS_NOT_IN_TOURNAMENT;
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetMaxNumEmpires (iGameClass, (int*) &iMaxNumEmpires);
    if (iErrCode != OK) {
        AddMessage ("Gameclass information could not be read");
        goto Cleanup;
    }

    piJoinedKey = (unsigned int*) StackAlloc (2 * iMaxNumEmpires * sizeof (unsigned int));
    piJoinedTeamKey = (unsigned int*) piJoinedKey + iMaxNumEmpires;

    // Get team list
    for (i = 0; i < iNumTeams; i ++) {

        char pszTeam [64];
        sprintf (pszTeam, "TeamSel%i", piTeamKey[i]);

        if (m_pHttpRequest->GetForm (pszTeam)) {

            for (j = 0; j < iNumEmpires; j ++) {

                if (piTeamEmpireKey[j] == piTeamKey[i]) {

                    if (iTotalEmpires >= iMaxNumEmpires) {
                        AddMessage ("You selected too many empires");
                        iErrCode = ERROR_TOO_MANY_EMPIRES;
                        goto Cleanup;
                    }

                    Assert (iTotalEmpires < iMaxNumEmpires);

                    piJoinedKey [iTotalEmpires] = piEmpireKey[j];
                    piJoinedTeamKey [iTotalEmpires] = piTeamEmpireKey[j];

                    iTotalEmpires ++;
                }
            }
        }
    }

    // Get empire list
    for (i = 0; i < iNumEmpires; i ++) {

        char pszEmpire[64];
        sprintf (pszEmpire, "EmpireSel%i", piEmpireKey[i]);

        if (m_pHttpRequest->GetForm (pszEmpire)) {

            for (j = 0; j < iTotalEmpires; j ++) {

                if (piJoinedKey[j] == piEmpireKey[i]) {
                    break;
                }
            }

            if (j == iTotalEmpires) {

                if (iTotalEmpires >= iMaxNumEmpires) {
                    AddMessage ("You selected too many empires");
                    iErrCode = ERROR_TOO_MANY_EMPIRES;
                    goto Cleanup;
                }

                Assert (iTotalEmpires < iMaxNumEmpires);

                piJoinedKey [iTotalEmpires] = piEmpireKey[i];
                piJoinedTeamKey [iTotalEmpires] = piTeamEmpireKey[j];

                iTotalEmpires ++;
            }
        }
    }

    if (iTotalEmpires != iMaxNumEmpires) {
        AddMessage ("Not enough empires could be found to start the game");
        iErrCode = ERROR_NOT_ENOUGH_EMPIRES;
        goto Cleanup;
    }

    // Game options
    if (bAdvanced) {
        iErrCode = ParseGameConfigurationForms(iGameClass, iTournamentKey, NULL, &goOptions);
    } else {
        iErrCode = g_pGameEngine->GetDefaultGameOptions (iGameClass, &goOptions);
    }

    if (iErrCode != OK) {
        AddMessage ("Could not process game options");
        goto Cleanup;
    }

    if (iTeamOptions != 0) {

        unsigned int iCurrentTeam, iNumTeams, iCountedTeams, iNumEmpiresInTeam, iMaxNumEmpiresInTeam;

        // Sort empires by teams
        Algorithm::QSortTwoAscending<unsigned int, unsigned int> (piJoinedTeamKey, piJoinedKey, iTotalEmpires);

        // Count number of teams
        iCurrentTeam = NO_KEY;
        iNumTeams = 0;
        iNumEmpiresInTeam = 0;
        iMaxNumEmpiresInTeam = 0;

        for (i = 0; i < iTotalEmpires; i ++) {

            if (piJoinedTeamKey[i] == NO_KEY) {
                continue;
            }

            if (piJoinedTeamKey[i] != iCurrentTeam) {
                iNumTeams ++;
                iCurrentTeam = piJoinedTeamKey[i];
                iNumEmpiresInTeam = 0;
            } else {
                iNumEmpiresInTeam ++;
                if (iNumEmpiresInTeam > iMaxNumEmpiresInTeam) {
                    iMaxNumEmpiresInTeam = iNumEmpiresInTeam;
                }
            }
        }

        // Check max num allies
        iErrCode = g_pGameEngine->GetMaxNumAllies (iGameClass, (int*) &iNumEmpiresInTeam);
        if (iErrCode != OK) {
            AddMessage ("Could not read gameclass data");
            goto Cleanup;
        }

        switch (iNumEmpiresInTeam) {

        case UNRESTRICTED_DIPLOMACY:
            break;

        case FAIR_DIPLOMACY:

            iNumEmpiresInTeam = g_pGameEngine->GetNumFairDiplomaticPartners (iMaxNumEmpiresInTeam);
            // Fall through

        default:

            if (iMaxNumEmpiresInTeam > iNumEmpiresInTeam) {
                AddMessage ("One of the selected teams exceeds the alliance limit for this game");
                iErrCode = ERROR_ALLIANCE_LIMIT_EXCEEDED;
                goto Cleanup;
            }
        }

        // Allocate team structures
        PrearrangedTeam* paTeam = (PrearrangedTeam*) StackAlloc (iNumTeams * sizeof (PrearrangedTeam));
        memset (paTeam, 0, iNumTeams * sizeof (PrearrangedTeam));

        // Build team structures
        iCurrentTeam = NO_KEY;
        iCountedTeams = 0;

        for (i = 0; i < iTotalEmpires; i ++) {

            if (piJoinedTeamKey[i] == NO_KEY) {
                continue;
            }

            if (piJoinedTeamKey[i] != iCurrentTeam) {

                if (paTeam [iCountedTeams].piEmpireKey == NULL) {
                    Assert (paTeam [iCountedTeams].iNumEmpires == 0);
                    paTeam [iCountedTeams].piEmpireKey = piJoinedKey + i;
                }

                Assert (iCountedTeams < iNumTeams);
                paTeam [iCountedTeams].iNumEmpires ++;

                iCountedTeams ++;
                iCurrentTeam = piJoinedTeamKey[i];
            
            } else {

                Assert (iCountedTeams <= iNumTeams);
                Assert (paTeam [iCountedTeams - 1].piEmpireKey != NULL);
                paTeam [iCountedTeams - 1].iNumEmpires ++;
            }
        }

        Assert (iCountedTeams == iNumTeams);

        goOptions.iNumPrearrangedTeams = iNumTeams;
        goOptions.paPrearrangedTeam = paTeam;
    }

    goOptions.iTeamOptions = iTeamOptions;
    goOptions.iNumEmpires = iTotalEmpires;
    goOptions.piEmpireKey = piJoinedKey;
    goOptions.iTournamentKey = iTournamentKey;

    iErrCode = g_pGameEngine->CreateGame (iGameClass, TOURNAMENT, goOptions, &iGameNumber);
    switch (iErrCode) {
        
    case OK:
        AddMessage ("The game was started");
        break;

    case ERROR_GAMECLASS_HALTED:
        AddMessage ("The gameclass is halted");
        break;

    case ERROR_GAMECLASS_DELETED:
        AddMessage ("The gameclass is marked for deletion");
        break;
    
    case ERROR_EMPIRE_IS_HALTED:
        AddMessage ("An empire is halted and could not enter the game");
        break;

    case ERROR_ALLIANCE_LIMIT_EXCEEDED:
        AddMessage ("One of the selected teams exceeds the alliance limit for this game");
        break;

    case ERROR_EMPIRE_IS_UNAVAILABLE_FOR_TOURNAMENTS:
        AddMessage ("One of the selected empires is unavailable to join tournament games");
        break;

    case ERROR_ACCESS_DENIED:
        AddMessage ("An empire does not have permission to enter the game");
        break;

    case ERROR_DUPLICATE_IP_ADDRESS:
        AddMessage ("An empire has a duplicate IP address and may not enter the game");
        break;

    case ERROR_DUPLICATE_SESSION_ID:
        AddMessage ("An empire has a duplicate Session Id and may not enter the game");
        break;

    default:
        AddMessage ("Error ");
        AppendMessage (iErrCode);
        AppendMessage (" occurred while creating the game");
        break;
    }

Cleanup:

    ClearGameOptions (&goOptions);

    if (piEmpireKey != NULL) {
        g_pGameEngine->FreeData (piEmpireKey);
    }

    if (piTeamEmpireKey != NULL) {
        g_pGameEngine->FreeData (piTeamEmpireKey);
    }

    if (piTeamKey != NULL) {
        g_pGameEngine->FreeKeys (piTeamKey);
    }

    return iErrCode;
}


void HtmlRenderer::RenderTournaments (const unsigned int* piTournamentKey, unsigned int iNumTournaments, bool bSingleOwner) {

    unsigned int i;

    OutputText ("<p><table width=\"75%\" bordercolor=\"#");
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    OutputText (

        "\">"\
    
        "<tr>"\
        "<th align=\"center\" bgcolor=\"");

    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    
    OutputText (
        "\">Icon</th>"\
        "<th align=\"center\" bgcolor=\"");
    
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    
    OutputText (
        "\">Name</th>"\
        "<th align=\"center\" bgcolor=\"");
    
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

    if (!bSingleOwner) {

        OutputText (
            "\">Owner</th>"\
            "<th align=\"center\" bgcolor=\"");
        
        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    }

    OutputText (
        "\">Webpage</th>"\
        "<th align=\"center\" bgcolor=\"");
    
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    
    OutputText (
        "\">Teams</th>"\
        "<th align=\"center\" bgcolor=\"");
    
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    
    OutputText (
        "\">Empires</th>"\
        "<th align=\"center\" bgcolor=\"");

    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    
    OutputText (
        "\">View Information</th>"\
        "</tr>"
        );

    for (i = 0; i < iNumTournaments; i ++) {

        RenderTournamentSimple (piTournamentKey[i], bSingleOwner);
    }

    OutputText ("</table>");
}


void HtmlRenderer::RenderTournamentSimple (unsigned int iTournamentKey, bool bSingleOwner) {

    int iErrCode;

    Variant* pvData = NULL;
    const char* pszUrl = NULL;
    unsigned int iNumTeams, iNumEmpires;

    iErrCode = g_pGameEngine->GetTournamentData (iTournamentKey, &pvData);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentTeams (iTournamentKey, NULL, NULL, &iNumTeams);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentEmpires (iTournamentKey, NULL, NULL, NULL, &iNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    OutputText (
        "<tr>"\
        
    // Icon
        "<td align=\"center\">"
        );
    
    WriteTournamentIcon (
        pvData[SystemTournaments::iIcon].GetInteger(),
        iTournamentKey,
        NULL,
        true
        );
    
    OutputText (
        "</td>"\
        
    // Name
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (pvData[SystemTournaments::iName].GetCharPtr());
    
    OutputText ("</td>");

    if (!bSingleOwner) {

        // Owner
        OutputText ("<td align=\"center\">");
        m_pHttpResponse->WriteText (pvData[SystemTournaments::iOwnerName].GetCharPtr());
        OutputText ("</td>");
    }

    // Webpage
    OutputText ("<td align=\"center\">");

    pszUrl = pvData[SystemTournaments::iWebPage].GetCharPtr();
    if (!String::IsBlank (pszUrl)) {

        OutputText ("<a href=\"");
        m_pHttpResponse->WriteText (pszUrl);
        OutputText ("\">");
        m_pHttpResponse->WriteText (pszUrl);
        OutputText ("</a>");
    }
    
    OutputText (
        "</td>"\
        
    // Teams
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (iNumTeams);
    
    OutputText (
        "</td>"\

    // Empires
        "<td align=\"center\">"
        );
    
    m_pHttpResponse->WriteText (iNumEmpires);
    
    OutputText (
        "</td>"\

    // View
        "<td align=\"center\">"
        );

    char pszForm [40];
    sprintf (pszForm, "ViewTourneyInfo%i", iTournamentKey);
    WriteButtonString (m_iButtonKey, "ViewTournamentInformation", "View Tournament Information", pszForm);

    OutputText (
        "</td>"\

        "</tr>"
        );

Cleanup:

    if (pvData != NULL) {
        g_pGameEngine->FreeData (pvData);
    }
}


void HtmlRenderer::RenderTournamentDetailed (unsigned int iTournamentKey) {

    int iErrCode;
    unsigned int* piEmpireKey = NULL;

    Variant* pvData = NULL, * pvTeamName = NULL, * pvTeamData = NULL;
    unsigned int i, j, iNumTeams, * piTeamKey = NULL, iNumEmpires, * piEmpTeamKey = NULL, 
        iEmpiresRendered = 0;

    const char* pszString = NULL;

    iErrCode = g_pGameEngine->GetTournamentData (iTournamentKey, &pvData);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentTeams (iTournamentKey, &piTeamKey, &pvTeamName, &iNumTeams);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetTournamentEmpires (iTournamentKey, &piEmpireKey, &piEmpTeamKey, NULL, &iNumEmpires);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    OutputText ("<p>");

    // Icon
    WriteTournamentIcon (
        pvData[SystemTournaments::iIcon].GetInteger(),
        iTournamentKey,
        NULL,
        true
        );

    // Name
    OutputText (" <h3>");
    m_pHttpResponse->WriteText (pvData[SystemTournaments::iName].GetCharPtr());

    if (pvData[SystemTournaments::iOwner].GetInteger() != SYSTEM) {
        OutputText (" (");
        m_pHttpResponse->WriteText (pvData[SystemTournaments::iOwnerName].GetCharPtr());
        OutputText (")");
    }
    OutputText ("</h3>");

    // Webpage
    pszString = pvData[SystemTournaments::iWebPage].GetCharPtr();
    if (!String::IsBlank (pszString)) {

        OutputText ("<p><a href=\"");
        m_pHttpResponse->WriteText (pszString);
        OutputText ("\">");
        m_pHttpResponse->WriteText (pszString);
        OutputText ("</a>");
    }

    pszString = pvData[SystemTournaments::iDescription].GetCharPtr();

    // Description
    if (!String::IsBlank (pszString)) {

        OutputText ("<p><table width=\"40%\"><tr><td>\"");
        m_pHttpResponse->WriteText (pvData[SystemTournaments::iDescription].GetCharPtr());
        OutputText ("\"</td></tr></table>");
    }

    // Teams
    if (iNumTeams > 0) {

        OutputText ("<p><h3>Teams:</h3>");

        for (i = 0; i < iNumTeams; i ++) {

            iErrCode = g_pGameEngine->GetTournamentTeamData (iTournamentKey, piTeamKey[i], &pvTeamData);
            if (iErrCode != OK) {
                goto Cleanup;
            }

            OutputText ("<p><table width=\"60%\" bordercolor=\"#");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText (

                "\">"\
            
                "<tr>"\
                "<th align=\"center\" bgcolor=\"");

            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

            OutputText (
                "\">Icon</th>"\
                "<th align=\"center\" bgcolor=\"");

            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

            OutputText (
                "\">Name</th>"\
                "<th align=\"center\" bgcolor=\"");
            
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

            OutputText (
                "\">Wins</th>"\
                "<th align=\"center\" bgcolor=\"");

            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

            OutputText (
                "\">Nukes</th>"\
                "<th align=\"center\" bgcolor=\"");

            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

            OutputText (
                "\">Nuked</th>"\
                "<th align=\"center\" bgcolor=\"");

            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

            OutputText (
                "\">Draws</th>"\
                "<th align=\"center\" bgcolor=\"");

            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

            OutputText (
                "\">Ruins</th>"\
                "<th align=\"center\" bgcolor=\"");

            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

            OutputText (
                "\">Description</th>"\

                "</tr>"

                "<tr>"\

            // Icon
                "<td align=\"center\">"
                );
            
            WriteTournamentIcon (
                pvTeamData[SystemTournamentTeams::iIcon].GetInteger(),
                iTournamentKey,
                NULL,
                true
                );
            
            OutputText (
                "</td>"\
                
            // Name
                "<td align=\"center\"><strong>"
                );

            pszString = pvTeamData[SystemTournamentTeams::iWebPage].GetCharPtr();
            if (!String::IsBlank (pszString)) {

                OutputText ("<a href=\"");
                m_pHttpResponse->WriteText (pszString);
                OutputText ("\">");
                m_pHttpResponse->WriteText (pvTeamData[SystemTournamentTeams::iName].GetCharPtr());
                OutputText ("</a>");
            
            } else {

                m_pHttpResponse->WriteText (pvTeamData[SystemTournamentTeams::iName].GetCharPtr());
            }

            OutputText (
                "</strong></td>"\

            // Wins
                "<td align=\"center\"><strong>"
                );
            
            m_pHttpResponse->WriteText (pvTeamData[SystemTournamentTeams::iWins].GetInteger());
            
            OutputText (
                "</strong></td>"\

            // Nukes
                "<td align=\"center\"><strong>"
                );
            
            m_pHttpResponse->WriteText (pvTeamData[SystemTournamentTeams::iNukes].GetInteger());
            
            OutputText (
                "</strong></td>"\

            // Nuked
                "<td align=\"center\"><strong>"
                );
            
            m_pHttpResponse->WriteText (pvTeamData[SystemTournamentTeams::iNuked].GetInteger());
            
            OutputText (
                "</strong></td>"\

            // Draws
                "<td align=\"center\"><strong>"
                );
            
            m_pHttpResponse->WriteText (pvTeamData[SystemTournamentTeams::iDraws].GetInteger());
            
            OutputText (
                "</strong></td>"\

            // Ruins
                "<td align=\"center\"><strong>"
                );

            m_pHttpResponse->WriteText (pvTeamData[SystemTournamentTeams::iRuins].GetInteger());
            
            OutputText (
                "</strong></td>"\

                "<td align=\"center\">"
                );

            // Description

            m_pHttpResponse->WriteText (pvTeamData[SystemTournamentTeams::iDescription].GetCharPtr());

            OutputText (
                "</td>"\

                "</tr>"

                );

            if (iNumEmpires > 0) {
                OutputText("<tr><th colspan=\"8\" bgcolor=\"");
                m_pHttpResponse->WriteText(m_vTableColor.GetCharPtr());
                OutputText("\">Empires:</th></tr>");
            }

            // Empires
            for (j = 0; j < iNumEmpires; j ++) {

                if (piEmpTeamKey[j] == piTeamKey[i]) {
                    RenderEmpire (iTournamentKey, piEmpireKey[j]);
                    iEmpiresRendered ++;
                }
            }

            OutputText ("</table>");

            if (pvTeamData != NULL) {
                g_pGameEngine->FreeData (pvTeamData);
                pvTeamData = NULL;
            }
        }
    }
        
    if (iEmpiresRendered < iNumEmpires) {

        OutputText (
            
            "<p><h3>Unaffiliated Empires:</h3>"\
            "<p><table width=\"60%\" bordercolor=\"#"
            );

        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
        OutputText (

            "\">"\
        
            "<tr>"\
            "<th align=\"center\" bgcolor=\"");

        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

        OutputText (
            "\">Icon</th>"\
            "<th align=\"center\" bgcolor=\"");

        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

        OutputText (
            "\">Name</th>"\
            "<th align=\"center\" bgcolor=\"");
        
        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

        OutputText (
            "\">Wins</th>"\
            "<th align=\"center\" bgcolor=\"");

        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

        OutputText (
            "\">Nukes</th>"\
            "<th align=\"center\" bgcolor=\"");

        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

        OutputText (
            "\">Nuked</th>"\
            "<th align=\"center\" bgcolor=\"");

        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

        OutputText (
            "\">Draws</th>"\
            "<th align=\"center\" bgcolor=\"");

        m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());

        OutputText ("\">Ruins</th></tr>");

        for (j = 0; j < iNumEmpires; j ++) {

            if (piEmpTeamKey[j] == NO_KEY) {
                RenderEmpire (iTournamentKey, piEmpireKey[j]);
            }
        }

        OutputText ("</table>");
    }

    if (iNumTeams == 0 && iNumEmpires == 0) {

        OutputText ("<p>This tournament has no empires or teams");
    }

    // News
    if (!String::IsBlank (pvData[SystemTournaments::iNews].GetCharPtr())) {

        String strFilter;
        iErrCode = HTMLFilter (pvData[SystemTournaments::iNews].GetCharPtr(), &strFilter, 0, true);
        if (iErrCode == OK) {

            OutputText ("<p><h3>News:</h3><p><table width=\"75%\"><tr><td>");
            m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
            OutputText ("</td></tr></table>");
        }
    }

    // Actions
    if (!m_bNotifiedTournamentInvitation && !NotifiedTournamentInvitation()) {
        
        for (i = 0; i < iNumEmpires; i ++) {
            
            if (piEmpireKey[i] == m_iEmpireKey) {

                OutputText ("<p>");
                WriteSeparatorString (m_iSeparatorKey);
                
                // Allow empire to invite himself
                OutputText (
                    "<p><table width=\"60%\"><tr><td>"\
                    "If you wish to quit from the tournament, press the following button. "\
                    "</td></tr><tr><td align=\"center\"><br>"
                    );
                
                WriteButton (BID_QUIT);
                
                OutputText ("</td></tr></table>");
                break;
            }
        }
        
        if (i == iNumEmpires) {

            OutputText ("<p>");
            WriteSeparatorString (m_iSeparatorKey);
            
            // Allow empire to invite himself
            OutputText (
                "<p><table width=\"60%\"><tr><td>If you wish to join the tournament, press the following button. "\
                "You will be giving permission to the tournament administrator to add your empire to new "
                "tournament games. The administrator also has the right to reject your join request."\
                "</td></tr><tr><td align=\"center\"><br>"
                );
            
            WriteButton (BID_JOIN);
            
            OutputText ("</td></tr></table>");
        }
    }

Cleanup:

    if (pvData != NULL) {
        g_pGameEngine->FreeData (pvData);
    }

    if (pvTeamName != NULL) {
        g_pGameEngine->FreeData (pvTeamName);
    }

    if (piTeamKey != NULL) {
        g_pGameEngine->FreeKeys (piTeamKey);
    }

    if (pvTeamData != NULL) {
        g_pGameEngine->FreeData (pvTeamData);
    }

    if (piEmpireKey != NULL) {
        g_pGameEngine->FreeKeys (piEmpireKey);
    }

    if (piEmpTeamKey != NULL) {
        g_pGameEngine->FreeKeys (piEmpTeamKey);
    }
}
