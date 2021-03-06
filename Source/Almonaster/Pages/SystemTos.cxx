<%

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
int iTosPage = 0;

if (m_bOwnPost && !m_bRedirection) {

    if ((pHttpForm = m_pHttpRequest->GetForm ("TosPage")) == NULL) {
        goto Redirection;
    }
    int iTosPageSubmit = pHttpForm->GetIntValue();

    switch (iTosPageSubmit) {
    case 0:

        if (WasButtonPressed (BID_TOS_ACCEPT)) {

            iErrCode = SetEmpireOption2 (m_iEmpireKey, EMPIRE_ACCEPTED_TOS, true);
            RETURN_ON_ERROR(iErrCode);
            m_iSystemOptions2 |= EMPIRE_ACCEPTED_TOS;

            AddMessage ("You accepted the Terms of Service");

	          char pszText [MAX_EMPIRE_NAME_LENGTH + 128];
	          sprintf(pszText, "%s accepted the Terms of Service", m_vEmpireName.GetCharPtr());
            global.WriteReport(TRACE_INFO, pszText);

            return Redirect (ACTIVE_GAME_LIST);
        }

        if (WasButtonPressed (BID_TOS_DECLINE))
        {
            iTosPage = 1;
            m_bRedirectTest = false;
        }
        break;

    case 1:

        if (WasButtonPressed (BID_TOS_DECLINE)) {

              iErrCode = CacheEmpireForDeletion(m_iEmpireKey);
	            RETURN_ON_ERROR(iErrCode);
	            
	            iErrCode = DeleteEmpire(m_iEmpireKey, NULL, true, false);
	            RETURN_ON_ERROR(iErrCode);

              char pszText [MAX_EMPIRE_NAME_LENGTH + 128];
	            sprintf(pszText, "%s declined the Terms of Service and was deleted", m_vEmpireName.GetCharPtr());
              global.WriteReport(TRACE_WARNING, pszText);

              AddMessage("The ");
              AppendMessage(m_vEmpireName.GetCharPtr());
              AppendMessage(" empire was deleted from the server");

	            return Redirect (LOGIN);
	        }

        break;

    default:
        break;
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

%><input type="hidden" name="TosPage" value="<% Write (iTosPage); %>"><%

// Individual page stuff starts here
switch (iTosPage) {

case 0:
    WriteTOS();
    break;

case 1:
    WriteConfirmTOSDecline();
    break;
}

iErrCode = CloseSystemPage();
RETURN_ON_ERROR(iErrCode);

%>