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


void HtmlRenderer::WriteGameMessages() {
    
    if (!m_strMessage.IsBlank()) {
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (m_strMessage);
        OutputText ("</strong><p>");
    }
    
    // Check for messages
    Variant** ppvMessage;
    unsigned int iNumMessages, i;
    
    int iErrCode = GetUnreadGameMessages (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        &ppvMessage,
        &iNumMessages
        );
    
    if (iErrCode == OK && iNumMessages > 0) {
        
        const char* pszSource, * pszMessage, * pszFontColor;
        String strHTMLMessage, strFiltered;
        bool bExists;
        
        unsigned int iSrcEmpireKey = NO_KEY, iNumMessagesFromPeople = 0;
        char pszDate [OS::MaxDateLength], pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];

        Variant vAlienKey;

        OutputText ("<table width=\"55%\">");
        
        for (i = 0; i < iNumMessages; i ++) {

            int iFlags = ppvMessage[i][GameEmpireMessages::iFlags].GetInteger();

            if (i > 0) {
                OutputText ("<tr><td>&nbsp;</td></tr>");
            }
            
            pszSource = ppvMessage[i][GameEmpireMessages::iSource].GetCharPtr();
            
            if (iFlags & MESSAGE_BROADCAST) {
                pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
            } else {
                pszFontColor = m_vPrivateMessageColor.GetCharPtr();
            }

            // Format message
            if (iFlags & MESSAGE_SYSTEM) {
                pszMessage = ppvMessage[i][GameEmpireMessages::iText].GetCharPtr();
            } else {
                
                if (HTMLFilter (
                    ppvMessage[i][GameEmpireMessages::iText].GetCharPtr(), 
                    &strFiltered, 
                    MAX_NUM_SPACELESS_CHARS,
                    true
                    ) == OK) {
                    
                    pszMessage = strFiltered.GetCharPtr();
                    
                } else {
                    
                    pszMessage = "The server is out of memory";
                }
            }
            
            iErrCode = Time::GetDateString (ppvMessage[i][GameEmpireMessages::iTimeStamp].GetInteger64(), pszDate);
            if (iErrCode != OK) {
                StrNCpy (pszDate, "The server is out of memory");
            }
            
            OutputText ("<tr><td align=\"left\">");
            
            if (iFlags & MESSAGE_SYSTEM) {

                iErrCode = GetSystemProperty (SystemData::SystemMessagesAlienKey, &vAlienKey);
                if (iErrCode == OK) {
                    WriteIcon (vAlienKey.GetInteger(), NO_KEY, NO_KEY, SYSTEM_MESSAGE_SENDER, NULL, false);
                }

            } else {
                
                if (DoesEmpireExist (pszSource, &bExists, &iSrcEmpireKey, NULL, NULL) == OK && bExists) {

                    if (GetEmpireProperty (
                        iSrcEmpireKey, 
                        SystemEmpireData::AlienKey, 
                        &vAlienKey
                        ) == OK) {
                    
                        sprintf (pszProfile, "View the profile of %s", pszSource);
                        
                        WriteProfileAlienString (
                            vAlienKey.GetInteger(),
                            iSrcEmpireKey,
                            pszSource,
                            0, 
                            "ProfileLink",
                            pszProfile,
                            false,
                            true
                            );

                        OutputText (" ");
                    }
                        
                    iNumMessagesFromPeople ++;
                }
            }
            
            OutputText ("On ");
            m_pHttpResponse->WriteText (pszDate);
            OutputText (", ");
            
            if (!(iFlags & MESSAGE_SYSTEM)) {

                if (iFlags & MESSAGE_ADMINISTRATOR) {
                    OutputText (" the administrator ");
                }
                else if (iFlags & MESSAGE_TOURNAMENT_ADMINISTRATOR) {
                    OutputText (" the tournament administrator ");
                }

                OutputText ("<strong>");
                m_pHttpResponse->WriteText (ppvMessage[i][GameEmpireMessages::iSource].GetCharPtr());
                OutputText ("</strong>");
                
                if (iFlags & MESSAGE_BROADCAST) {
                    OutputText (" broadcast");
                } else {
                    OutputText (" sent");
                }

            } else {

                if (iFlags & MESSAGE_UPDATE) {
                    OutputText ("an update occurred");
                } else {
                    OutputText ("the system ");

                    if (iFlags & MESSAGE_BROADCAST) {
                        OutputText ("broadcast");
                    } else {
                        OutputText ("sent");
                    }

                    OutputText (" a message");
                }
            }
            
            OutputText (":<p><font face=\"");
            OutputText (DEFAULT_MESSAGE_FONT);
            OutputText ("\" size=\"");
            OutputText (DEFAULT_MESSAGE_FONT_SIZE);

            if (!(iFlags & MESSAGE_SYSTEM)) {
                OutputText ("\" color=\"#");
                m_pHttpResponse->WriteText (pszFontColor);
            }
            OutputText ("\">");
            WriteFormattedMessage (pszMessage);
            OutputText ("</font></td></tr>");
            
            FreeData (ppvMessage[i]);
            
        }   // End empire loop

        OutputText ("</table><p>");
        
        if (iNumMessagesFromPeople > 0) {
            NotifyProfileLink();
        }
        
        delete [] ppvMessage;
    }
}

void HtmlRenderer::WriteSystemMessages() {
    
    if (!m_strMessage.IsBlank()) {
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (m_strMessage);
        OutputText ("</strong><p>");
    }
    
    // Check for messages
    Variant** ppvMessage = NULL;
    unsigned int* piMessageKey = NULL, iNumMessages, i;
    
    int iErrCode = GetUnreadSystemMessages (
        m_iEmpireKey,
        &ppvMessage,
        &piMessageKey,
        &iNumMessages
        );
    
    if (iErrCode == OK && iNumMessages > 0) {

        unsigned int iNumMessagesFromPeople = 0;

        OutputText ("<table width=\"55%\">");
        
        for (i = 0; i < iNumMessages; i ++) {

            int iType = ppvMessage[i][SystemEmpireMessages::iType].GetInteger();

            // Slight hack to restrict invitations to one per screen
            if (m_bNotifiedTournamentInvitation && iType == MESSAGE_TOURNAMENT_INVITATION) {
                continue;
            }

            // Slight hack to restrict join requests to one per screen
            if (m_bNotifiedTournamentJoinRequest && iType == MESSAGE_TOURNAMENT_JOIN_REQUEST) {
                continue;
            }

            if (i > 0) {
                OutputText ("<tr><td>&nbsp</td></tr>");
            }

            if (RenderSystemMessage (piMessageKey[i], ppvMessage[i])) {
                iNumMessagesFromPeople ++;
            }

            FreeData (ppvMessage[i]);    
        }

        OutputText ("</table><p>");
        
        if (iNumMessagesFromPeople > 0) {
            NotifyProfileLink();
        }
        
        delete [] ppvMessage;
        FreeKeys (piMessageKey);
    }
}

bool HtmlRenderer::RenderSystemMessage (int iMessageKey, const Variant* pvMessage) {
    
    int iErrCode = OK;
    unsigned int iSrcEmpireKey = NO_KEY, iAlienKey = NO_KEY;
    bool bEmpireLink = false, bExists;

    char pszDate [OS::MaxDateLength];
    
    // Get message source
    int iFlags = pvMessage[SystemEmpireMessages::iFlags].GetInteger();
    const char* pszSource = pvMessage[SystemEmpireMessages::iSource].GetCharPtr();

    // Get system / personal
    bool bSystem = (iFlags & MESSAGE_SYSTEM) != 0;

    Variant vTemp;

    // Get date
    iErrCode = Time::GetDateString (pvMessage[SystemEmpireMessages::iTimeStamp].GetInteger64(), pszDate);
    if (iErrCode != OK) {
        StrNCpy (pszDate, "The server is out of memory");
        iMessageKey = NO_KEY;
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    // Get source empire and icon
    if (bSystem) {

        iErrCode = GetSystemProperty (SystemData::SystemMessagesAlienKey, &vTemp);
        if (iErrCode == OK) {
            iAlienKey = vTemp.GetInteger();
        }
        
    } else {

        iErrCode = DoesEmpireExist (pszSource, &bExists, &iSrcEmpireKey, NULL, NULL);
        if (iErrCode == OK && bExists) {

            iErrCode = GetEmpireProperty (
                iSrcEmpireKey, 
                SystemEmpireData::AlienKey,
                &vTemp
                );

            if (iErrCode == OK) {
                iAlienKey = vTemp.GetInteger();
            }
        }
    }

    // No errors so far...
    iErrCode = OK;

    //
    // Print header
    //

    OutputText ("<tr><td align=\"left\">");

    if (iAlienKey != NO_KEY) {
        
        if (bSystem) {

            WriteIcon (iAlienKey, NO_KEY, NO_KEY, SYSTEM_MESSAGE_SENDER, NULL, false);

        } else {

            char pszProfile [MAX_EMPIRE_NAME_LENGTH + 64];
            sprintf (pszProfile, "View the profile of %s", pszSource);
            
            WriteProfileAlienString (
                iAlienKey,
                iSrcEmpireKey,
                pszSource,
                0, 
                "ProfileLink",
                pszProfile,
                false,
                true
                );
            
            bEmpireLink  = true;
        }

        OutputText (" ");
    }

    OutputText ("On ");
    m_pHttpResponse->WriteText (pszDate);
    OutputText (", ");

    //
    // Select based on message type
    //

    switch (pvMessage[SystemEmpireMessages::iType].GetInteger()) {
        
    case MESSAGE_NORMAL:
        {

        // Select broadcast and color
        const char* pszFontColor, * pszMessage;
        String strFiltered;
        
        // Format message
        if (bSystem) {

            OutputText ("the system ");

            if (iFlags & MESSAGE_BROADCAST) {
                pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
                OutputText ("broadcast");
            } else {
                pszFontColor = m_vPrivateMessageColor.GetCharPtr();
                OutputText ("sent");
            }

            OutputText (" a message");

            pszMessage = pvMessage[SystemEmpireMessages::iText].GetCharPtr();
        
        } else {

            if (iFlags & MESSAGE_ADMINISTRATOR) {
                OutputText (" the administrator ");
            }
            else if (iFlags & MESSAGE_TOURNAMENT_ADMINISTRATOR) {
                OutputText (" the tournament administrator ");
            }

            OutputText ("<strong>");
            m_pHttpResponse->WriteText (pszSource);
            OutputText ("</strong> ");

            if (iFlags & MESSAGE_BROADCAST) {
                pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
                OutputText ("broadcast");
            } else {
                pszFontColor = m_vPrivateMessageColor.GetCharPtr();
                OutputText ("sent");
            }
            
            pszMessage = HTMLFilter (
                pvMessage[SystemEmpireMessages::iText].GetCharPtr(), 
                &strFiltered, 
                MAX_NUM_SPACELESS_CHARS,
                true
                ) == OK ? strFiltered.GetCharPtr() : "The server is out of memory";
        }
        
        // Normal message stuff
        OutputText (":<p><font face=\"" DEFAULT_MESSAGE_FONT "\" size=\"" DEFAULT_MESSAGE_FONT_SIZE);

        if (!bSystem) {
            OutputText ("\" color=\"#");
            m_pHttpResponse->WriteText (pszFontColor);
        }

        OutputText ("\">");
        WriteFormattedMessage (pszMessage);
        OutputText ("</font>");

        }
        break;

    case MESSAGE_TOURNAMENT_INVITATION:

        OutputText ("<strong>");
        m_pHttpResponse->WriteText (pszSource);
        OutputText ("</strong> ");
        
        // Only do one per screen
        if (m_bNotifiedTournamentInvitation) {

            OutputText ("invited you to join a tournament");

        } else {

            Variant vName = NULL;

            unsigned int iSenderKey, iOwnerKey, iTournamentKey;

            sscanf (pvMessage[SystemEmpireMessages::iData].GetCharPtr(), "%i.%i", &iTournamentKey, &iSenderKey);

            OutputText ("invited you to join ");

            iErrCode = GetTournamentName (iTournamentKey, &vName);
            if (iErrCode != OK) {
                OutputText ("a tournament that no longer exists");
                goto Cleanup;
            }

            iErrCode = GetTournamentOwner (iTournamentKey, &iOwnerKey);
            if (iErrCode != OK) {
                OutputText ("a tournament that no longer exists");
                goto Cleanup;
            }

            OutputText ("the <strong>");
            
            m_pHttpResponse->WriteText (vName.GetCharPtr());
            
            OutputText ("</strong> ");

            if (iOwnerKey == SYSTEM) {
                OutputText ("system");
            } else {
                OutputText ("personal");
            }
            
            OutputText (
                " tournament. If you accept, you are giving permission to the tournament "\
                " administrator to add your empire to new <strong>");

            m_pHttpResponse->WriteText (vName.GetCharPtr());
            
            OutputText (
                "</strong> games."\
                "</td></tr><tr><td align=\"center\">&nbsp;<br>"
                );

            WriteButton (BID_DECLINE);
            WriteButton (BID_VIEWTOURNAMENTINFORMATION);
            WriteButton (BID_ACCEPT);

            NotifyTournamentInvitation (iMessageKey, iTournamentKey);
        }
        break;

    case MESSAGE_TOURNAMENT_JOIN_REQUEST:

        OutputText ("<strong>");
        m_pHttpResponse->WriteText (pszSource);
        OutputText ("</strong> ");
        
        // Only do one per screen
        if (m_bNotifiedTournamentJoinRequest) {

            OutputText ("requested permission to join a tournament");

        } else {

            Variant vName = NULL;
            unsigned int iTournamentKey, iSenderKey, iOwnerKey;

            sscanf (pvMessage[SystemEmpireMessages::iData].GetCharPtr(), "%i.%i", &iTournamentKey, &iSenderKey);

            OutputText ("requested permission to join ");

            iErrCode = GetTournamentName (iTournamentKey, &vName);
            if (iErrCode != OK) {
                OutputText ("a tournament that no longer exists");
                goto Cleanup;
            }

            iErrCode = GetTournamentOwner (iTournamentKey, &iOwnerKey);
            if (iErrCode != OK) {
                OutputText ("a tournament that no longer exists");
                goto Cleanup;
            }

            if (iOwnerKey != m_iEmpireKey && !(iOwnerKey == SYSTEM && m_iEmpireKey == global.GetRootKey())) {
                iErrCode = ERROR_WRONG_TOURNAMENT_OWNER;
                OutputText ("a tournament that you don't own");
                goto Cleanup;
            }

            OutputText ("the <strong>");
            
            m_pHttpResponse->WriteText (vName.GetCharPtr());
            
            OutputText ("</strong> tournament.</td></tr><tr><td align=\"center\">&nbsp;<br>");

            WriteButton (BID_DECLINE);
            WriteButton (BID_VIEWTOURNAMENTINFORMATION);
            WriteButton (BID_ACCEPT);

            NotifyTournamentJoinRequest (iMessageKey, iTournamentKey);
        }
        break;

    default:

        Assert (false);
        break;
    }

Cleanup:

    OutputText ("</td></tr>");

    if (iErrCode != OK && iMessageKey != NO_KEY) {
        DeleteSystemMessage (m_iEmpireKey, iMessageKey);
    }

    return bEmpireLink;
}