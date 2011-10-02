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

int HtmlRenderer::WriteAdministerTournament(unsigned int iTournamentKey)
{
    int iErrCode, * piOptions = NULL;

    Variant* pvData = NULL, * pvGameClassName = NULL, * pvEmpireName = NULL, * pvTeamName = NULL, * pvEmpireKey = NULL;
    AutoFreeData free_pvData(pvData);
    AutoFreeData free_pvEmpireKey(pvEmpireKey);
    AutoFreeData free_pvTeamName(pvTeamName);
    Algorithm::AutoDelete<Variant> free_pvEmpireName(pvEmpireName, true);
    Algorithm::AutoDelete<Variant> free_pvGameClassName(pvGameClassName, true);

    String strDesc, strUrl, strNews;

    unsigned int i, * piGameClassKey = NULL, iNumGameClasses, iNumEmpires, * piTeamKey = NULL, iNumTeams;
    AutoFreeKeys free_piGameClassKey(piGameClassKey);
    AutoFreeKeys free_piTeamKey(piTeamKey);

    unsigned int iNumHalted = 0, iNumNotHalted = 0, iNumMarked = 0, iNumUnmarked = 0, iNumStartable = 0, iGames;
    
    iErrCode = CacheTournamentEmpireTables(iTournamentKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentData(iTournamentKey, &pvData);
    if (iErrCode == ERROR_TOURNAMENT_DOES_NOT_EXIST)
    {
        OutputText ("<p><strong>The tournament does not exist</strong>");
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentGameClasses (iTournamentKey, &piGameClassKey, &pvGameClassName, &iNumGameClasses);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentEmpires (iTournamentKey, &pvEmpireKey, NULL, &pvEmpireName, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentTeams (iTournamentKey, &piTeamKey, &pvTeamName, &iNumTeams);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentGames(iTournamentKey, NULL, &iGames);
    RETURN_ON_ERROR(iErrCode);

    if (iNumGameClasses > 0)
    {
        piOptions = (int*) StackAlloc (iNumGameClasses * sizeof (int));

        for (i = 0; i < iNumGameClasses; i ++)
        {
            iErrCode = GetGameClassOptions (piGameClassKey[i], piOptions + i);
            RETURN_ON_ERROR(iErrCode);

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

    HTMLFilter (pvData[SystemTournaments::iDescription].GetCharPtr(), &strDesc, 0, false);
    HTMLFilter (pvData[SystemTournaments::iWebPage].GetCharPtr(), &strUrl, 0, false);
    HTMLFilter (pvData[SystemTournaments::iNews].GetCharPtr(), &strNews, 0, false);

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
            m_pHttpResponse->WriteText (pvEmpireKey[i].GetInteger());
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

    return iErrCode;
}

int HtmlRenderer::WriteAdministerTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) {

    int iErrCode;

    Variant* pvData = NULL, * pvEmpireName = NULL, * pvEmpireKey = NULL, * pvTeamKey = NULL;
    AutoFreeData free_pvData(pvData);
    AutoFreeData free_pvEmpireKey(pvEmpireKey);
    AutoFreeData free_pvTeamKey(pvTeamKey);
    Algorithm::AutoDelete<Variant> free_pvEmpireName(pvEmpireName, true);

    unsigned int i, iNumEmpires;
    String strDesc, strUrl;

    iErrCode = CacheTournamentEmpireTables(iTournamentKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentTeamData(iTournamentKey, iTeamKey, &pvData);
    if (iErrCode == ERROR_TOURNAMENT_TEAM_DOES_NOT_EXIST)
    {
        OutputText ("<p><strong>The tournament or team does not exist</strong>");
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentEmpires (iTournamentKey, &pvEmpireKey, &pvTeamKey, &pvEmpireName, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    HTMLFilter(pvData[SystemTournamentTeams::iDescription].GetCharPtr(), &strDesc, 0, false);
    HTMLFilter(pvData[SystemTournamentTeams::iWebPage].GetCharPtr(), &strUrl, 0, false);

    OutputText ("<p>");
    WriteTournamentTeamIcon(pvData[SystemTournamentTeams::iIcon].GetInteger(), iTournamentKey, iTeamKey, NULL, false);

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

    for (i = 0; i < iNumEmpires; i ++)
    {
        if ((unsigned int)pvTeamKey[i].GetInteger() != iTeamKey) {
            
            OutputText (
                "<tr>"\
                "<td>Add an empire to this team:</td><td>"\
                "<select name=\"JoinTeam\">"\
                );
            
            for (; i < iNumEmpires; i ++) {
                
                if ((unsigned int)pvTeamKey[i].GetInteger() != iTeamKey) {

                    OutputText ("<option value=\"");
                    m_pHttpResponse->WriteText (pvEmpireKey[i].GetInteger());
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
        
        if ((unsigned int)pvTeamKey[i].GetInteger() == iTeamKey) {
            
            OutputText (
                "<tr>"\
                "<td>Delete an empire from this team:</td><td>"\
                "<select name=\"DelFromTeam\">"\
                );
            
            for (; i < iNumEmpires; i ++) {

                if ((unsigned int)pvTeamKey[i].GetInteger() == iTeamKey) {
                
                    OutputText ("<option value=\"");
                    m_pHttpResponse->WriteText (pvEmpireKey[i].GetInteger());
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

    return iErrCode;
}

void HtmlRenderer::WriteCreateTournamentTeam (unsigned int iTournamentKey) {

    IHttpForm* pHttpForm;

    String strName, strDesc, strUrl;

    if ((pHttpForm = m_pHttpRequest->GetForm ("TeamName")) != NULL)
    {
        HTMLFilter (pHttpForm->GetValue(), &strName, 0, false);
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TeamDescription")) != NULL)
    {
        HTMLFilter (pHttpForm->GetValue(), &strDesc, 0, false);
    }

    if ((pHttpForm = m_pHttpRequest->GetForm ("TeamWebPageURL")) != NULL)
    {
        HTMLFilter (pHttpForm->GetValue(), &strUrl, 0, false);
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

int HtmlRenderer::ProcessCreateTournamentTeam(unsigned int iTournamentKey, bool* pbCreated)
{
    int iErrCode;
    unsigned int iTournamentTeamKey;

    *pbCreated = false;
    
    // Parse the forms
    bool bParsed;
    Variant pvSubmitArray [SystemTournamentTeams::NumColumns];
    iErrCode = ParseCreateTournamentTeamForms(pvSubmitArray, iTournamentKey, &bParsed);
    RETURN_ON_ERROR(iErrCode);

    if (!bParsed)
    {
        return OK;
    }
    
    // Create the tournament
    iErrCode = CreateTournamentTeam(iTournamentKey, pvSubmitArray, &iTournamentTeamKey);
    switch (iErrCode)
    {
    case OK:
        *pbCreated = true;
        AddMessage("The team was created");
        break;
    case ERROR_TOURNAMENT_TEAM_ALREADY_EXISTS:
        iErrCode = OK;
        AddMessage("The new team name already exists");
        break;
    case ERROR_NAME_IS_TOO_LONG:
        iErrCode = OK;
        AddMessage("The new team name is too long");
        break;
    case ERROR_DESCRIPTION_IS_TOO_LONG:
        iErrCode = OK;
        AddMessage("The new team description is too long");
        break;
    default:
        break;
    }
    
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int HtmlRenderer::ParseCreateTournamentTeamForms (Variant* pvSubmitArray, unsigned int iTournamentKey, bool* pbParsed)
{
    *pbParsed = false;

    pvSubmitArray[SystemTournamentTeams::iTournamentKey] = iTournamentKey;

    // TeamName
    IHttpForm* pHttpForm = m_pHttpRequest->GetForm ("TeamName");
    if (pHttpForm == NULL)
    {
        return ERROR_MISSING_FORM;
    }

    if (!VerifyCategoryName("Team", pHttpForm->GetValue(), MAX_TOURNAMENT_TEAM_NAME_LENGTH, true))
    {
        return ERROR_MISSING_FORM;
    }

    pvSubmitArray [SystemTournamentTeams::iName] = pHttpForm->GetValue();

    // Description
    pHttpForm = m_pHttpRequest->GetForm ("TeamDescription");
    if (pHttpForm == NULL)
    {
        return ERROR_MISSING_FORM;
    }

    if (String::StrLen (pHttpForm->GetValue()) > MAX_TOURNAMENT_TEAM_DESCRIPTION_LENGTH)
    {
        AddMessage("The description is too long");
        return OK;
    }

    pvSubmitArray [SystemTournamentTeams::iDescription] = pHttpForm->GetValue();

    // URL
    pHttpForm = m_pHttpRequest->GetForm ("TeamWebPageURL");
    if (pHttpForm == NULL) {
        return ERROR_MISSING_FORM;
    }

    if (String::StrLen(pHttpForm->GetValue()) > MAX_WEB_PAGE_LENGTH)
    {
        AddMessage("The description is too long");
        return OK;
    }

    // Web page
    const char* pszWebPage = pHttpForm->GetValue();
    pvSubmitArray[SystemTournamentTeams::iWebPage] = pszWebPage;
    if (pszWebPage)
    {
        Assert(pvSubmitArray[SystemTournamentTeams::iWebPage].GetCharPtr());
    }

    // Icon
    EnsureDefaultSystemIcon();
    pvSubmitArray [SystemTournamentTeams::iIcon] = m_iDefaultSystemIcon;

    *pbParsed = true;

    return OK;
}

int HtmlRenderer::StartTournamentGame(unsigned int iTournamentKey, int iTeamOptions, bool bAdvanced)
{
    int iErrCode, iGameNumber;
    Variant* pvEmpireKey = NULL, * pvTeamEmpireKey = NULL;
    AutoFreeData free_pvEmpireKey(pvEmpireKey);
    AutoFreeData free_pvTeamEmpireKey(pvTeamEmpireKey);

    unsigned int i, j, iGameClass, iCheckKey, iTotalEmpires = 0, iMaxNumEmpires, iNumTeams, iNumEmpires;
    unsigned int* piTeamKey = NULL, * piJoinedTeamKey, * piJoinedKey = NULL;
    AutoFreeKeys free_piTeamKey(piTeamKey);

    IHttpForm* pHttpForm;

    GameOptions goOptions;
    InitGameOptions(&goOptions);
    AutoClearGameOptions clear_goOptions(goOptions);

    // Get teams
    iErrCode = GetTournamentTeams(iTournamentKey, &piTeamKey, NULL, &iNumTeams);
    RETURN_ON_ERROR(iErrCode);

    // Get empires
    iErrCode = GetAvailableTournamentEmpires(iTournamentKey, &pvEmpireKey, &pvTeamEmpireKey, NULL, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    // Get gameclass
    pHttpForm = m_pHttpRequest->GetForm ("GameClassKey");
    if (pHttpForm == NULL)
    {
        return ERROR_MISSING_FORM;
    }
    iGameClass = pHttpForm->GetIntValue();

    iErrCode = GetGameClassTournament (iGameClass, &iCheckKey);
    RETURN_ON_ERROR(iErrCode);

    if (iCheckKey != iTournamentKey)
    {
        AddMessage("Gameclass is not in tournament");
        return ERROR_GAMECLASS_IS_NOT_IN_TOURNAMENT;
    }

    iErrCode = GetMaxNumEmpires(iGameClass, (int*) &iMaxNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    piJoinedKey = (unsigned int*) StackAlloc (2 * iMaxNumEmpires * sizeof (unsigned int));
    piJoinedTeamKey = (unsigned int*) piJoinedKey + iMaxNumEmpires;

    // Get team list
    for (i = 0; i < iNumTeams; i ++)
    {
        char pszTeam [64];
        sprintf(pszTeam, "TeamSel%i", piTeamKey[i]);

        if (m_pHttpRequest->GetForm (pszTeam))
        {
            for (j = 0; j < iNumEmpires; j ++)
            {
                if ((unsigned int)pvTeamEmpireKey[j].GetInteger() == piTeamKey[i])
                {
                    if (iTotalEmpires >= iMaxNumEmpires)
                    {
                        AddMessage("Too many empires selected");
                        return ERROR_TOO_MANY_EMPIRES;
                    }

                    Assert(iTotalEmpires < iMaxNumEmpires);

                    piJoinedKey [iTotalEmpires] = pvEmpireKey[j].GetInteger();
                    piJoinedTeamKey [iTotalEmpires] = pvTeamEmpireKey[j].GetInteger();

                    iTotalEmpires ++;
                }
            }
        }
    }

    // Get empire list
    for (i = 0; i < iNumEmpires; i ++)
    {
        char pszEmpire[64];
        sprintf(pszEmpire, "EmpireSel%i", pvEmpireKey[i].GetInteger());

        if (m_pHttpRequest->GetForm (pszEmpire))
        {
            for (j = 0; j < iTotalEmpires; j ++)
            {
                if (piJoinedKey[j] == (unsigned int)pvEmpireKey[i].GetInteger())
                {
                    break;
                }
            }

            if (j == iTotalEmpires)
            {
                if (iTotalEmpires >= iMaxNumEmpires)
                {
                    AddMessage("Too many empires selected");
                    return ERROR_TOO_MANY_EMPIRES;
                }

                Assert(iTotalEmpires < iMaxNumEmpires);

                piJoinedKey[iTotalEmpires] = pvEmpireKey[i].GetInteger();
                piJoinedTeamKey[iTotalEmpires] = pvTeamEmpireKey[j].GetInteger();

                iTotalEmpires ++;
            }
        }
    }

    if (iTotalEmpires != iMaxNumEmpires)
    {
        AddMessage("Not enough empires could be found to start the game");
        return ERROR_NOT_ENOUGH_EMPIRES;
    }

    // Game options
    if (bAdvanced)
    {
        iErrCode = ParseGameConfigurationForms(iGameClass, iTournamentKey, NULL, &goOptions);
        if (iErrCode == WARNING)
        {
            AddMessage("Could not process game options");
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = GetDefaultGameOptions(iGameClass, &goOptions);
        RETURN_ON_ERROR(iErrCode);
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

        for (i = 0; i < iTotalEmpires; i ++)
        {
            if (piJoinedTeamKey[i] == NO_KEY)
            {
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
        iErrCode = GetMaxNumAllies (iGameClass, (int*) &iNumEmpiresInTeam);
        RETURN_ON_ERROR(iErrCode);

        switch (iNumEmpiresInTeam) {

        case UNRESTRICTED_DIPLOMACY:
            break;

        case FAIR_DIPLOMACY:

            iNumEmpiresInTeam = GetNumFairDiplomaticPartners (iMaxNumEmpiresInTeam);
            // Fall through

        default:

            if (iMaxNumEmpiresInTeam > iNumEmpiresInTeam)
            {
                AddMessage("One of the selected teams exceeds the alliance limit for this game");
                return ERROR_ALLIANCE_LIMIT_EXCEEDED;
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
                    Assert(paTeam [iCountedTeams].iNumEmpires == 0);
                    paTeam [iCountedTeams].piEmpireKey = piJoinedKey + i;
                }

                Assert(iCountedTeams < iNumTeams);
                paTeam [iCountedTeams].iNumEmpires ++;

                iCountedTeams ++;
                iCurrentTeam = piJoinedTeamKey[i];
            
            } else {

                Assert(iCountedTeams <= iNumTeams);
                Assert(paTeam [iCountedTeams - 1].piEmpireKey != NULL);
                paTeam [iCountedTeams - 1].iNumEmpires ++;
            }
        }

        Assert(iCountedTeams == iNumTeams);

        goOptions.iNumPrearrangedTeams = iNumTeams;
        goOptions.paPrearrangedTeam = paTeam;
    }

    goOptions.iTeamOptions = iTeamOptions;
    goOptions.iNumEmpires = iTotalEmpires;
    goOptions.piEmpireKey = piJoinedKey;
    goOptions.iTournamentKey = iTournamentKey;

    iErrCode = CreateGame(iGameClass, TOURNAMENT, goOptions, &iGameNumber);
    switch (iErrCode)
    {
    case OK:
        AddMessage("The game was started");
        break;

    case ERROR_GAMECLASS_HALTED:
        iErrCode = ERROR_COULD_NOT_START_GAME;
        AddMessage("The gameclass is halted");
        break;

    case ERROR_GAMECLASS_DELETED:
        iErrCode = ERROR_COULD_NOT_START_GAME;
        AddMessage("The gameclass is marked for deletion");
        break;
    
    case ERROR_EMPIRE_IS_HALTED:
        iErrCode = ERROR_COULD_NOT_START_GAME;
        AddMessage("An empire is halted and could not enter the game");
        break;

    case ERROR_ALLIANCE_LIMIT_EXCEEDED:
        iErrCode = ERROR_COULD_NOT_START_GAME;
        AddMessage("One of the selected teams exceeds the alliance limit for this game");
        break;

    case ERROR_EMPIRE_IS_UNAVAILABLE_FOR_TOURNAMENTS:
        iErrCode = ERROR_COULD_NOT_START_GAME;
        AddMessage("One of the selected empires is unavailable to join tournament games");
        break;

    case ERROR_ACCESS_DENIED:
        iErrCode = ERROR_COULD_NOT_START_GAME;
        AddMessage("An empire does not have permission to enter the game");
        break;

    case ERROR_DUPLICATE_IP_ADDRESS:
        iErrCode = ERROR_COULD_NOT_START_GAME;
        AddMessage("An empire has a duplicate IP address and may not enter the game");
        break;

    case ERROR_DUPLICATE_SESSION_ID:
        iErrCode = ERROR_COULD_NOT_START_GAME;
        AddMessage("An empire has a duplicate Session Id and may not enter the game");
        break;

    default:
        RETURN_ON_ERROR(iErrCode);
        break;
    }

    return iErrCode;
}

int HtmlRenderer::RenderTournaments(const Variant* pvTournamentKey, unsigned int iNumTournaments, bool bSingleOwner)
{
    unsigned int* piTournamentKey = (unsigned int*)StackAlloc(iNumTournaments * sizeof(unsigned int));
    for (unsigned int i = 0; i < iNumTournaments; i ++)
    {
        piTournamentKey[i] = pvTournamentKey[i].GetInteger();
    }
    return RenderTournaments(piTournamentKey, iNumTournaments, bSingleOwner);
}

int HtmlRenderer::RenderTournaments(const unsigned int* piTournamentKey, unsigned int iNumTournaments, bool bSingleOwner)
{
    int iErrCode = CacheTournamentTables(piTournamentKey, iNumTournaments);
    RETURN_ON_ERROR(iErrCode);

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

    for (unsigned int i = 0; i < iNumTournaments; i ++)
    {
        iErrCode = RenderTournamentSimple(piTournamentKey[i], bSingleOwner);
        RETURN_ON_ERROR(iErrCode);
    }

    OutputText ("</table>");

    return OK;
}

int HtmlRenderer::RenderTournamentSimple (unsigned int iTournamentKey, bool bSingleOwner)
{
    int iErrCode;

    Variant* pvData = NULL;
    AutoFreeData free(pvData);

    const char* pszUrl = NULL;
    unsigned int iNumTeams, iNumEmpires;

    iErrCode = GetTournamentData(iTournamentKey, &pvData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentTeams(iTournamentKey, NULL, NULL, &iNumTeams);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentEmpires(iTournamentKey, NULL, NULL, NULL, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

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
    sprintf(pszForm, "ViewTourneyInfo%i", iTournamentKey);
    WriteButtonString (m_iButtonKey, "ViewTournamentInformation", "View Tournament Information", pszForm);

    OutputText (
        "</td>"\
        "</tr>"
        );

    return iErrCode;
}

int HtmlRenderer::RenderTournamentDetailed(unsigned int iTournamentKey)
{
    int iErrCode;

    Variant* pvData = NULL, * pvTeamName = NULL, * pvEmpireKey = NULL, * pvEmpTeamKey = NULL;
    AutoFreeData free_pvData(pvData);
    AutoFreeData free_pvTeamName(pvTeamName);
    AutoFreeData free_pvEmpireKey(pvEmpireKey);
    AutoFreeData free_pvEmpTeamKey(pvEmpTeamKey);

    unsigned int i, j, iNumTeams, * piTeamKey = NULL, iNumEmpires, iEmpiresRendered = 0;
    AutoFreeKeys free_piTeamKey(piTeamKey);

    const char* pszString = NULL;

    // Cache tables for tournament
    iErrCode = CacheTournamentAndEmpireTables(iTournamentKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentData (iTournamentKey, &pvData);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentTeams (iTournamentKey, &piTeamKey, &pvTeamName, &iNumTeams);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = GetTournamentEmpires (iTournamentKey, &pvEmpireKey, &pvEmpTeamKey, NULL, &iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

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

        for (i = 0; i < iNumTeams; i ++)
        {
            Variant* pvTeamData = NULL;
            AutoFreeData free_pvTeamData(pvTeamData);

            iErrCode = GetTournamentTeamData (iTournamentKey, piTeamKey[i], &pvTeamData);
            RETURN_ON_ERROR(iErrCode);

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
            for (j = 0; j < iNumEmpires; j ++)
            {
                if ((unsigned int)pvEmpTeamKey[j].GetInteger() == piTeamKey[i])
                {
                    iErrCode = RenderEmpire (iTournamentKey, pvEmpireKey[j].GetInteger());
                    RETURN_ON_ERROR(iErrCode);

                    iEmpiresRendered ++;
                }
            }

            OutputText ("</table>");
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

        for (j = 0; j < iNumEmpires; j ++)
        {
            if (pvEmpTeamKey[j].GetInteger() == NO_KEY)
            {
                iErrCode = RenderEmpire (iTournamentKey, pvEmpireKey[j].GetInteger());
                RETURN_ON_ERROR(iErrCode);
            }
        }

        OutputText ("</table>");
    }

    if (iNumTeams == 0 && iNumEmpires == 0) {

        OutputText ("<p>This tournament has no empires or teams");
    }

    // News
    if (!String::IsBlank (pvData[SystemTournaments::iNews].GetCharPtr()))
    {
        String strFilter;
        HTMLFilter (pvData[SystemTournaments::iNews].GetCharPtr(), &strFilter, 0, true);
        OutputText ("<p><h3>News:</h3><p><table width=\"75%\"><tr><td>");
        m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
        OutputText ("</td></tr></table>");
    }

    // Actions
    if (!m_bNotifiedTournamentInvitation && !NotifiedTournamentInvitation()) {
        
        for (i = 0; i < iNumEmpires; i ++)
        {
            if ((unsigned int)pvEmpireKey[i].GetInteger() == m_iEmpireKey)
            {
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

    return iErrCode;
}
