//
// Almonaster.dll:  a component of Almonaster
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

#include "HtmlRenderer.h"

#include "Osal/Socket.h"


void HtmlRenderer::WriteProfile (unsigned int iTargetEmpireKey, bool bEmpireAdmin, bool bSendMessage, bool bShowButtons) {

    bool bCanBroadcast;

    Seconds sBridierSecondsLeft = -1;
    
    String strHtml, strLogin, strCreation;

    char pszHashedIPAddress [20];
    const char* pszIPAddress = NULL;
    
    Variant* pvEmpireData = NULL;
    int iErrCode, iNumActiveGames, iNumPersonalGameClasses, iOptions, iOptions2;
    unsigned int iNumActiveTournaments, iNumPersonalTournaments, iNumUnreadMessages;

    OutputText ("<input type=\"hidden\" name=\"TargetEmpireKey\" value=\"");
    m_pHttpResponse->WriteText (iTargetEmpireKey);
    OutputText ("\">");
    
    iErrCode = g_pGameEngine->GetEmpireData (iTargetEmpireKey, &pvEmpireData, &iNumActiveGames);    
    if (iErrCode != OK) {
        goto OnError;
    }

    iOptions = pvEmpireData [SystemEmpireData::Options].GetInteger();
    iOptions2 = pvEmpireData [SystemEmpireData::Options2].GetInteger();

    iErrCode = g_pGameEngine->GetJoinedTournaments (iTargetEmpireKey, NULL, NULL, &iNumActiveTournaments);
    if (iErrCode != OK) {
        goto OnError;
    }
    
    iErrCode = g_pGameEngine->GetEmpirePersonalGameClasses (iTargetEmpireKey, NULL, NULL, &iNumPersonalGameClasses);
    if (iErrCode != OK) {
        goto OnError;
    }

    iErrCode = g_pGameEngine->GetOwnedTournaments (iTargetEmpireKey, NULL, NULL, &iNumPersonalTournaments);
    if (iErrCode != OK) {
        goto OnError;
    }

    char pszLoginTime [OS::MaxDateLength], pszCreationTime [OS::MaxDateLength];
    
    iErrCode = Time::GetDateString (pvEmpireData[SystemEmpireData::LastLoginTime].GetUTCTime(), pszLoginTime);
    if (iErrCode != OK) {
        goto OnError;
    }
    
    iErrCode = Time::GetDateString (pvEmpireData[SystemEmpireData::CreationTime].GetUTCTime(), pszCreationTime);
    if (iErrCode != OK) {
        goto OnError;
    }
    
    iErrCode = g_pGameEngine->GetNumUnreadSystemMessages (iTargetEmpireKey, &iNumUnreadMessages);
    if (iErrCode != OK) {
        goto OnError;
    }
    
    // Name and alien icon
    OutputText ("<p>");
    
    WriteEmpireIcon (
        pvEmpireData[SystemEmpireData::AlienKey].GetInteger(),
        iTargetEmpireKey,
        NULL,
        true
        );
    
    OutputText (" <font size=\"+2\">");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Name].GetCharPtr());
    OutputText ("</font><p>");
    
    if (bShowButtons) {

        int iNumNukes, iNumNuked;

        iErrCode = g_pGameEngine->GetNumEmpiresInNukeHistory (iTargetEmpireKey, &iNumNukes, &iNumNuked);
        if (iErrCode != OK) {
            goto OnError;
        }

        // PGC button
        if (iNumPersonalGameClasses > 0) {
            WriteButton (BID_VIEWEMPIRESGAMECLASSES);
        }

        // PT button
        if (iNumPersonalTournaments > 0) {
            WriteButton (BID_VIEWEMPIRESTOURNAMENTS);
        }
    
        // Nuke history button      
        if (iNumNukes > 0 || iNumNuked > 0) {
            WriteButton (BID_VIEWEMPIRESNUKEHISTORY);
        }

        if (pvEmpireData[SystemEmpireData::BridierIndex].GetInteger() != BRIDIER_MAX_INDEX) {

            UTCTime tNow;
            Time::GetTime (&tNow);

            Seconds sTimeSpent = Time::GetSecondDifference (
                tNow, pvEmpireData[SystemEmpireData::LastBridierActivity].GetUTCTime()
                );

            sBridierSecondsLeft = 3 * 30 * DAY_LENGTH_IN_SECONDS - sTimeSpent;
            if (sTimeSpent > 3 * 30 * DAY_LENGTH_IN_SECONDS) {

                sBridierSecondsLeft = sBridierSecondsLeft + 30 * DAY_LENGTH_IN_SECONDS;
                if (sTimeSpent > 4 * 30 * DAY_LENGTH_IN_SECONDS) {
                    
                    sBridierSecondsLeft = sBridierSecondsLeft + 30 * DAY_LENGTH_IN_SECONDS;
                    if (sTimeSpent > 5 * 30 * DAY_LENGTH_IN_SECONDS) {
                        
                        sBridierSecondsLeft = sBridierSecondsLeft + 30 * DAY_LENGTH_IN_SECONDS;
                        if (sTimeSpent > 6 * 30 * DAY_LENGTH_IN_SECONDS) {
                            sBridierSecondsLeft = sBridierSecondsLeft + 30 * DAY_LENGTH_IN_SECONDS;
                        }
                    }
                }
            }
        }
    }
    
    OutputText (
        "<p><table border=\"0\" width=\"80%\">"\
        "<tr>"\
        "<td><strong>Empire Key:</strong></td>"\
        "<td>"
        );
    
    m_pHttpResponse->WriteText (iTargetEmpireKey);
    
    OutputText (
        "</td>"\
        "<td>&nbsp;</td>"\
        "<td><strong>Wins:</strong></td>"\
        "<td>"
        );
    
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Wins].GetInteger());
    
    OutputText (
        "</td>"\
        "</tr>"\
        "<tr>"\
        "<td><strong>Privilege:</strong></td>"\
        "<td>"
        );
    
    if (!bEmpireAdmin || 
        m_iPrivilege < ADMINISTRATOR ||
        m_iEmpireKey == iTargetEmpireKey || 
        iTargetEmpireKey == ROOT_KEY || 
        iTargetEmpireKey == GUEST_KEY) {
        
        m_pHttpResponse->WriteText (PRIVILEGE_STRING [pvEmpireData[SystemEmpireData::Privilege].GetInteger()]);
        
        if (iOptions2 & ADMINISTRATOR_FIXED_PRIVILEGE) {
            OutputText (" (fixed)");
        }

    } else {
        
        int i, j;

        bool bFixed = (iOptions2 & ADMINISTRATOR_FIXED_PRIVILEGE) != 0;
        
        OutputText ("<input type=\"hidden\" name=\"OldPriv\" value=\"");
        m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Privilege].GetInteger());
        OutputText (
            "\">"\
            
            "<input type=\"hidden\" name=\"OldFixedPriv\" value=\""
            );
        m_pHttpResponse->WriteText (bFixed ? "1" : "0");
        OutputText (
            "\">"\

            "<select name=\"NewPriv\">"
            );
        
        ENUMERATE_PRIVILEGE_LEVELS(i) {
            
            OutputText ("<option");
            
            if (pvEmpireData[SystemEmpireData::Privilege].GetInteger() == i) {
                OutputText (" selected");
            }
            
            OutputText (" value=\"");
            m_pHttpResponse->WriteText (i);
            OutputText ("\">");
            m_pHttpResponse->WriteText (PRIVILEGE_STRING [i]);
            OutputText ("</option>");
        }

        OutputText (
            "</select> "\
            "<select name=\"FixedPriv\">"\
            "<option"
            );

        if (!bFixed) {
            OutputText (" selected");
        }

        OutputText (
            " value=\"0\">Score-based privilege</option>"\
            "<option"
            );

        if (bFixed) {
            OutputText (" selected");
        }

        OutputText (
            " value=\"1\">Fixed privilege</option>"\
            "</select>"
            );
    }
    
    OutputText (
        "</td>"\
        "<td>&nbsp;</td>"\
        "<td><strong>Nukes:</strong></td>"\
        "<td>"
        );
    
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Nukes].GetInteger());
    
    OutputText (
        "</td>"\
        "</tr>"\
        "<tr>"\
        "<td><strong>Broadcast:</strong></td>"\
        "<td>"
        );
    
    bCanBroadcast = (pvEmpireData[SystemEmpireData::Options].GetInteger() & CAN_BROADCAST) != 0;
    
    if (!bEmpireAdmin || 
        m_iPrivilege < ADMINISTRATOR ||
        m_iEmpireKey == iTargetEmpireKey || 
        iTargetEmpireKey == ROOT_KEY ||
        iTargetEmpireKey == GUEST_KEY) {
        
        if (bCanBroadcast) {
            OutputText ("Yes");
        } else {
            OutputText ("No");
        }
        
    } else {
        
        OutputText ("<input type=\"hidden\" name=\"OldBroadcast\" value=\"");
        if (bCanBroadcast) {
            OutputText ("1");
        } else {
            OutputText ("0");
        }
        OutputText ("\"><select name=\"Broadcast\">");
        
        if (bCanBroadcast) {
            OutputText (
                "<option selected value=\"1\">Yes</option>"\
                "<option value=\"0\">No</option>"
                );
        } else {
            OutputText (
                "<option value=\"1\">Yes</option>"\
                "<option selected value=\"0\">No</option>"
                );
        }
        OutputText ("</select>");
    }
    
    OutputText (
        "</td>"\
        "<td>&nbsp;</td>"\
        "<td><strong>Nuked:</strong></td>"\
        "<td>"
        );
    
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Nuked].GetInteger());

    // Creation Time
    OutputText ("</td></tr><tr><td><strong>Creation Time:</strong></td><td>");
    m_pHttpResponse->WriteText (pszCreationTime);
    
    // Draws
    OutputText ("</td><td>&nbsp;</td><td><strong>Draws:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Draws].GetInteger());
    
    // Real Name
    OutputText ("</td></tr><tr><td><strong>Real Name:</strong></td><td>");
    iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::RealName].GetCharPtr(), &strHtml, 0, false);
    if (iErrCode == OK && !strHtml.IsBlank()) {
        m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
    }
    
    // Ruins
    OutputText ("</td><td>&nbsp;</td><td><strong>Ruins:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Ruins].GetInteger());

    // Age
    OutputText ("</td></tr><tr><td><strong>Age:</strong></td><td>");
    if (pvEmpireData[SystemEmpireData::Age].GetInteger() == EMPIRE_AGE_UNKNOWN) {
        OutputText ("N/A");
    } else {
        m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Age].GetInteger());
    }
    
    // Almonaster Score
    OutputText ("</td><td>&nbsp;</td><td><strong>Almonaster Score: </strong></td><td>");
    
    if (bEmpireAdmin &&
        m_iPrivilege >= ADMINISTRATOR &&
        m_iEmpireKey != iTargetEmpireKey && 
        iTargetEmpireKey != ROOT_KEY &&
        iTargetEmpireKey != GUEST_KEY) {
        
        OutputText ("<input type=\"hidden\" name=\"OldAScore\" value=\"");
        m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::AlmonasterScore].GetFloat());
        OutputText ("\">");
        
        OutputText ("<input type=\"text\" size=\"12\" maxlength=\"12\" name=\"NewAScore\" value=\"");
        m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::AlmonasterScore].GetFloat());
        OutputText ("\">");
        
    } else {
        
        m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::AlmonasterScore].GetFloat());
    }

    // Gender
    OutputText ("</td></tr><tr><td><strong>Gender:</strong></td><td>");
    if (pvEmpireData[SystemEmpireData::Gender].GetInteger() >= EMPIRE_GENDER_UNKNOWN &&
        pvEmpireData[SystemEmpireData::Gender].GetInteger() <= EMPIRE_GENDER_FEMALE) {

        m_pHttpResponse->WriteText (EMPIRE_GENDER_STRING [pvEmpireData[SystemEmpireData::Gender].GetInteger()]);
    
    } else {
        OutputText ("N/A");
    }
    
    // Significance
    OutputText ("</td><td>&nbsp;</td><td><strong>Significance:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::AlmonasterScoreSignificance].GetInteger());
    
    // Location
    OutputText ("</td></tr><tr><td><strong>Location:</strong></td><td>");
    iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::Location].GetCharPtr(), &strHtml, 0, false);
    if (iErrCode == OK && !strHtml.IsBlank()) {
        m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
    }
    
    // Classic Score
    OutputText ("</td><td>&nbsp;</td><td><strong>Classic Score:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::ClassicScore]);

    // Email address
    OutputText ("</td></tr><tr><td><strong>Email Address:</strong></td><td>");
    
    iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::Email].GetCharPtr(), &strHtml, 0, false);
    if (iErrCode == OK && !strHtml.IsBlank()) {
        OutputText ("<a href=\"mailto:");
        m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
        OutputText ("\">");
        m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
        OutputText ("</a>");
    }
    
    if (m_iPrivilege >= ADMINISTRATOR || m_iEmpireKey == iTargetEmpireKey) {
        
        iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::PrivateEmail].GetCharPtr(), &strHtml, 0, false);
        if (iErrCode == OK && !strHtml.IsBlank()) {
            OutputText (" [<a href=\"mailto:");
            m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
            OutputText ("\">");
            m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
            OutputText ("</a>]");
        }
    }
    
    // Bridier Rank
    OutputText ("</td><td>&nbsp;</td><td><strong>Bridier Rank:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::BridierRank].GetInteger());
    
    // Instant Messenger
    OutputText ("</td></tr><tr><td><strong>Instant Messenger:</strong></td><td>");

    iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::IMId].GetCharPtr(), &strHtml, 0, false);
    if (iErrCode == OK && !strHtml.IsBlank()) {
        m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
    }
    
    // Max Mil
    OutputText ("</td><td>&nbsp;</td><td><strong>Bridier Index:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::BridierIndex].GetInteger());

    if (sBridierSecondsLeft != -1) {

        int iDays = sBridierSecondsLeft / DAY_LENGTH_IN_SECONDS;

        OutputText (" (");
        m_pHttpResponse->WriteText (iDays);
        OutputText (" day");
        if (iDays != 1) {
            OutputText ("s");
        }
        OutputText (" left)");
    }
    
    // Web page
    OutputText ("</td></tr><tr><td><strong>Webpage:</strong></td><td>");
    
    RenderUnsafeHyperText (
        pvEmpireData[SystemEmpireData::WebPage].GetCharPtr(),
        pvEmpireData[SystemEmpireData::WebPage].GetCharPtr()
        );
    
    // Max Econ
    OutputText ("</td><td>&nbsp;</td><td><strong>Max Econ:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::MaxEcon].GetInteger());

    // Last Login
    OutputText ("</td></tr><tr><td><strong>Last Login:</strong></td><td>");
    m_pHttpResponse->WriteText (pszLoginTime);
    
    // Max Mil
    OutputText ("</td><td>&nbsp;</td><td><strong>Max Mil:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::MaxMil].GetInteger());
    
    // Login count
    OutputText ("</td></tr><tr><td><strong>Login Count:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::NumLogins].GetInteger());
    
    // Unread messages
    OutputText ("</td><td>&nbsp;</td><td><strong>Unread Messages:</strong></td><td>");
    m_pHttpResponse->WriteText (iNumUnreadMessages);

    // Browser
    OutputText ("</td></tr><tr><td><strong>Browser:</strong></td><td>");
    m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Browser].GetCharPtr());

    // Active Games
    OutputText ("</td><td>&nbsp;</td><td><strong>Games:</strong></td><td>");
    m_pHttpResponse->WriteText (iNumActiveGames);

    // IP Address  
    OutputText ("</td></tr><tr><td><strong>");

    pszIPAddress = pvEmpireData[SystemEmpireData::IPAddress].GetCharPtr();
    if (String::IsBlank (pszIPAddress)) {

        OutputText ("IP Address:</strong></td><td>");

    } else {
    
        if (m_iEmpireKey == iTargetEmpireKey || bEmpireAdmin || m_iPrivilege >= ADMINISTRATOR) {

            OutputText ("IP Address:</strong></td><td>");
            m_pHttpResponse->WriteText (pszIPAddress);

            if (m_iPrivilege >= ADMINISTRATOR) {

                if (WasButtonPressed (BID_LOOKUP)) {
                    
                    char pszDNS [256];
                    if (Socket::GetHostNameFromIPAddress (pszIPAddress, pszDNS, countof (pszDNS)) == OK) {
                        
                        OutputText (" (<em>");
                        m_pHttpResponse->WriteText (pszDNS);
                        OutputText ("</em>)");
                    }
                    
                } else {
                    
                    OutputText ("&nbsp;&nbsp;");
                    WriteButton (BID_LOOKUP);
                }
            }

        } else {
            
            OutputText ("Hashed IP Address:</strong></td><td>");

            HashIPAddress (pvEmpireData[SystemEmpireData::IPAddress].GetCharPtr(), pszHashedIPAddress);
            m_pHttpResponse->WriteText (pszHashedIPAddress);
        }
    }

    // Active Tournaments
    OutputText ("</td><td>&nbsp;</td><td><strong>Tournaments:</strong></td><td>");
    m_pHttpResponse->WriteText (iNumActiveTournaments);

    if (iOptions2 & UNAVAILABLE_FOR_TOURNAMENTS) {
        OutputText (" (unavailable)");
    }

    // Session Id
    OutputText ("</td></tr><tr><td>");

    if (m_iPrivilege < ADMINISTRATOR) {
        OutputText ("</td><td>");
    } else {

        OutputText ("<strong>Session Id:</strong></td><td>");

        m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::SessionId].GetInteger64());
        
        if (bEmpireAdmin && m_iPrivilege >= ADMINISTRATOR) {
            OutputText ("&nbsp;&nbsp;");
            WriteButton (BID_RESET);
        }
    }

    OutputText ("</td></tr></table>");
    
    bCanBroadcast = (m_iSystemOptions & CAN_BROADCAST) != 0;
    
    if (!String::IsBlank (pvEmpireData[SystemEmpireData::Quote].GetCharPtr())) {

        OutputText ("<p><table width=\"60%\"><tr><td><font face=\"" DEFAULT_QUOTE_FONT "\">");

        String strQuote;
        iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::Quote].GetCharPtr(), &strQuote, 0, true);
        if (iErrCode == OK) {
            m_pHttpResponse->WriteText (strQuote.GetCharPtr());
        } else {
            OutputText ("The server is out of memory");
        }
        
        OutputText ("</td></tr></table>");
    }
    
    if (!bEmpireAdmin && bSendMessage && bCanBroadcast && m_iEmpireKey != iTargetEmpireKey && m_iPrivilege > GUEST) {
        OutputText ("<p><textarea name=\"Message\" rows=\"7\" cols=\"60\" wrap=\"hard\"></textarea><p>");
        WriteButton (BID_SENDMESSAGE);
    }
    
    g_pGameEngine->FreeData (pvEmpireData);
    
    return;

OnError:

    if (pvEmpireData != NULL) {
        g_pGameEngine->FreeData (pvEmpireData);
    }
    
    OutputText ("<p><strong>Error ");
    m_pHttpResponse->WriteText (iErrCode);
    OutputText (" occurred reading data for this empire</strong>");
}