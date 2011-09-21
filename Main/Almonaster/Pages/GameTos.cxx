<% #include "Almonaster.h"
#include "GameEngine.h"

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
iErrCode = InitializeEmpireInGame(false, &bInitialized);
RETURN_ON_ERROR(iErrCode);
if (!bInitialized)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
bool bRedirected;
iErrCode = InitializeGame(&pageRedirect, &bRedirected);
RETURN_ON_ERROR(iErrCode);
if (bRedirected)
{
    return Redirect(pageRedirect);
}

int iTosPage = 0;
IHttpForm* pHttpForm;

if (m_bOwnPost && !m_bRedirection) {

    if ((pHttpForm = m_pHttpRequest->GetForm ("TosPage")) == NULL) {
        goto Redirection;
    }
    int iTosPageSubmit = pHttpForm->GetIntValue();

    switch (iTosPageSubmit) {
    case 0:

        if (WasButtonPressed (BID_TOS_ACCEPT)) {

            GameCheck (SetEmpireOption2 (m_iEmpireKey, EMPIRE_ACCEPTED_TOS, true));
            m_iSystemOptions2 |= EMPIRE_ACCEPTED_TOS;

            AddMessage ("You accepted the Terms of Service");

            return Redirect (INFO);
        }

        if (WasButtonPressed (BID_TOS_DECLINE)) {
            iTosPage = 1;
            m_bRedirectTest = false;
        }
        break;

    case 1:

        if (WasButtonPressed (BID_TOS_DECLINE)) {
            
            // Best effort
            DeleteEmpire (m_iEmpireKey, NULL, true, false);

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
    GameCheck(RedirectOnSubmitGame(&pageRedirect, &bRedirected));
    if (bRedirected)
    {
        return Redirect (pageRedirect);
    }
}
GameCheck(OpenGamePage());

%><input type="hidden" name="TosPage" value="<% Write (iTosPage); %>"><%

// Individual page stuff starts here
bool bGameStarted = (m_iGameState & STARTED) != 0;

if (bGameStarted && m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
    GameCheck (WriteRatiosString (NULL));
}

// Individual page stuff starts here
switch (iTosPage) {

case 0:

    WriteTOS();
    break;

case 1:

    WriteConfirmTOSDecline();
    break;
}

iErrCode = CloseGamePage();
RETURN_ON_ERROR(iErrCode);

%>