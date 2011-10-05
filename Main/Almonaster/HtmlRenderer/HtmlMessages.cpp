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


int HtmlRenderer::WriteGameMessages()
{
    if (!m_strMessage.IsBlank())
    {
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (m_strMessage);
        OutputText ("</strong><p>");
    }
    
    // Check for messages
    Variant** ppvMessage = NULL;
    unsigned int iNumMessages;

    Algorithm::AutoDelete<Variant*> free_ppvMessage(ppvMessage, true);
    AutoFreeArrayOfData free_ppvMessage2(ppvMessage, iNumMessages);
   
    int iErrCode = GetUnreadGameMessages(m_iGameClass, m_iGameNumber, m_iEmpireKey, &ppvMessage, &iNumMessages);
    RETURN_ON_ERROR(iErrCode);
    
    if (iNumMessages > 0)
    {
        String strHTMLMessage, strFiltered;
        
        unsigned int iNumMessagesFromPeople = 0;
        char pszDate [OS::MaxDateLength], pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];

        // Cache empire tables of senders
        unsigned int* piEmpires = (unsigned int*)StackAlloc(iNumMessages * sizeof(unsigned int));
        for (unsigned int i = 0; i < iNumMessages; i ++)
        {
            piEmpires[i] = ppvMessage[i][GameEmpireMessages::iSourceKey].GetInteger();
        }
        
        iErrCode = CacheEmpires(piEmpires, iNumMessages);
        RETURN_ON_ERROR(iErrCode);

        OutputText ("<table width=\"55%\">");
        
        for (unsigned int i = 0; i < iNumMessages; i ++)
        {
            if (i > 0)
            {
                OutputText ("<tr><td>&nbsp;</td></tr>");
            }

            int iFlags = ppvMessage[i][GameEmpireMessages::iFlags].GetInteger();
            const char* pszSource = ppvMessage[i][GameEmpireMessages::iSourceName].GetCharPtr();
            
            const char* pszFontColor;
            if (iFlags & MESSAGE_BROADCAST)
            {
                pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
            }
            else
            {
                pszFontColor = m_vPrivateMessageColor.GetCharPtr();
            }

            // Format message
            const char* pszMessage;
            if (iFlags & MESSAGE_SYSTEM)
            {
                pszMessage = ppvMessage[i][GameEmpireMessages::iText].GetCharPtr();
            }
            else
            {
                HTMLFilter(ppvMessage[i][GameEmpireMessages::iText].GetCharPtr(), &strFiltered, MAX_NUM_SPACELESS_CHARS, true);
                pszMessage = strFiltered.GetCharPtr();
            }
            
            iErrCode = Time::GetDateString(ppvMessage[i][GameEmpireMessages::iTimeStamp].GetInteger64(), pszDate);
            Assert(iErrCode == OK);
            
            OutputText ("<tr><td align=\"left\">");
            
            Variant vAlienKey, vAlienAddress;
            if (iFlags & MESSAGE_SYSTEM)
            {
                iErrCode = GetSystemProperty(SystemData::SystemMessagesAlienKey, &vAlienKey);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = GetSystemProperty(SystemData::SystemMessagesAlienAddress, &vAlienAddress);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = WriteIcon(vAlienKey, vAlienAddress, NO_KEY, NO_KEY, SYSTEM_MESSAGE_SENDER, NULL, false);
                RETURN_ON_ERROR(iErrCode);
            }
            else
            {
                unsigned int iSrcEmpireKey = ppvMessage[i][GameEmpireMessages::iSourceKey].GetInteger();
                
                iErrCode = GetEmpireProperty(iSrcEmpireKey, SystemEmpireData::AlienKey, &vAlienKey);
                RETURN_ON_ERROR(iErrCode);

                iErrCode = GetEmpireProperty(iSrcEmpireKey, SystemEmpireData::AlienAddress, &vAlienAddress);
                RETURN_ON_ERROR(iErrCode);

                sprintf(pszProfile, "View the profile of %s", pszSource);

                iErrCode = WriteProfileAlienString(vAlienKey, vAlienAddress, iSrcEmpireKey, pszSource, 0, "ProfileLink", pszProfile, false, true);
                RETURN_ON_ERROR(iErrCode);
                OutputText (" ");
                        
                iNumMessagesFromPeople ++;
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
                m_pHttpResponse->WriteText(ppvMessage[i][GameEmpireMessages::iSourceName].GetCharPtr());
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
        }

        OutputText ("</table><p>");
        
        if (iNumMessagesFromPeople > 0) {
            NotifyProfileLink();
        }
    }

    return iErrCode;
}

int HtmlRenderer::WriteSystemMessages()
{
    if (!m_strMessage.IsBlank()) {
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (m_strMessage);
        OutputText ("</strong><p>");
    }
    
    // Check for messages
    Variant** ppvMessage = NULL;
    unsigned int* piMessageKey = NULL, iNumMessages, i;
    
    Algorithm::AutoDelete<Variant*> free_ppvMessage(ppvMessage, true);
    AutoFreeArrayOfData free_ppvMessage2(ppvMessage, iNumMessages);
    AutoFreeKeys free_piMessageKey(piMessageKey);

    int iErrCode = GetUnreadSystemMessages(m_iEmpireKey, &ppvMessage, &piMessageKey, &iNumMessages);
    RETURN_ON_ERROR(iErrCode);
    
    if (iNumMessages > 0)
    {
        unsigned int iNumMessagesFromPeople = 0;

        // Cache empire tables of senders
        unsigned int* piEmpires = (unsigned int*)StackAlloc(iNumMessages * sizeof(unsigned int));
        for (i = 0; i < iNumMessages; i ++)
        {
            piEmpires[i] = ppvMessage[i][SystemEmpireMessages::iSourceKey].GetInteger();
        }
        
        iErrCode = CacheEmpires(piEmpires, iNumMessages);
        RETURN_ON_ERROR(iErrCode);

        OutputText ("<table width=\"55%\">");
        
        for (i = 0; i < iNumMessages; i ++)
        {
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

            bool bMessageFromEmpire;
            iErrCode = RenderSystemMessage(piMessageKey[i], ppvMessage[i], &bMessageFromEmpire);
            if (iErrCode == WARNING)
            {
                iErrCode = OK;
            }
            RETURN_ON_ERROR(iErrCode);

            if (bMessageFromEmpire)
            {
                iNumMessagesFromPeople ++;
            }
        }

        OutputText ("</table><p>");
        
        if (iNumMessagesFromPeople > 0) {
            NotifyProfileLink();
        }
    }

    return iErrCode;
}

int HtmlRenderer::RenderSystemMessage(int iMessageKey, const Variant* pvMessage, bool* pbMessageFromEmpire)
{
    int iErrCode = OK;
    unsigned int iSrcEmpireKey = NO_KEY;
    
    *pbMessageFromEmpire = false;

    char pszDate [OS::MaxDateLength];
    
    // Get message source
    int iFlags = pvMessage[SystemEmpireMessages::iFlags].GetInteger();
    const char* pszSource = pvMessage[SystemEmpireMessages::iSourceName].GetCharPtr();

    // Get system / personal
    bool bSystem = (iFlags & MESSAGE_SYSTEM) != 0;

    Variant vTemp;

    // Get date
    iErrCode = Time::GetDateString (pvMessage[SystemEmpireMessages::iTimeStamp].GetInteger64(), pszDate);
    Assert(iErrCode == OK);

    // Get source empire and icon
    int iAlienKey, iAlienAddress;
    if (bSystem)
    {
        iErrCode = GetSystemProperty(SystemData::SystemMessagesAlienKey, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iAlienKey = vTemp.GetInteger();

        iErrCode = GetSystemProperty(SystemData::SystemMessagesAlienAddress, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iAlienAddress = vTemp.GetInteger();
    }
    else
    {
        iSrcEmpireKey = pvMessage[SystemEmpireMessages::iSourceKey].GetInteger();

        iErrCode = GetEmpireProperty(iSrcEmpireKey, SystemEmpireData::AlienKey, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iAlienKey = vTemp.GetInteger();

        iErrCode = GetEmpireProperty(iSrcEmpireKey, SystemEmpireData::AlienAddress, &vTemp);
        RETURN_ON_ERROR(iErrCode);
        iAlienAddress = vTemp.GetInteger();
    }

    //
    // Print header
    //

    OutputText ("<tr><td align=\"left\">");

    if (bSystem)
    {
        iErrCode = WriteIcon(iAlienKey, iAlienAddress, NO_KEY, NO_KEY, SYSTEM_MESSAGE_SENDER, NULL, false);
        RETURN_ON_ERROR(iErrCode);
    }
    else if (iAlienKey != NO_KEY)
    {
        char pszProfile [MAX_EMPIRE_NAME_LENGTH + 64];
        sprintf(pszProfile, "View the profile of %s", pszSource);
            
        Assert(iSrcEmpireKey != NO_KEY);
        iErrCode = WriteProfileAlienString(iAlienKey, iAlienAddress, iSrcEmpireKey, pszSource, 0, "ProfileLink", pszProfile, false, true);
        RETURN_ON_ERROR(iErrCode);
        *pbMessageFromEmpire = true;
    }

    OutputText (" On ");
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
            
            HTMLFilter(pvMessage[SystemEmpireMessages::iText].GetCharPtr(), &strFiltered, MAX_NUM_SPACELESS_CHARS, true);
            pszMessage = strFiltered.GetCharPtr();
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

            sscanf(pvMessage[SystemEmpireMessages::iData].GetCharPtr(), "%i.%i", &iTournamentKey, &iSenderKey);

            OutputText ("invited you to join ");

            iErrCode = GetTournamentName(iTournamentKey, &vName);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetTournamentOwner (iTournamentKey, &iOwnerKey);
            RETURN_ON_ERROR(iErrCode);

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

            sscanf(pvMessage[SystemEmpireMessages::iData].GetCharPtr(), "%i.%i", &iTournamentKey, &iSenderKey);

            OutputText ("requested permission to join ");

            iErrCode = GetTournamentName (iTournamentKey, &vName);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetTournamentOwner (iTournamentKey, &iOwnerKey);
            RETURN_ON_ERROR(iErrCode);

            if (iOwnerKey != m_iEmpireKey && !(iOwnerKey == SYSTEM && m_iEmpireKey == global.GetRootKey()))
            {
                OutputText ("a tournament that you no longer own");
                return WARNING;
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
        Assert(false);
        break;
    }

    OutputText ("</td></tr>");

    return iErrCode;
}