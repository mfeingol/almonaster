<%
#include <stdio.h>

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

if (InitializeEmpireInGame(false) != OK)
{
    return Redirect(LOGIN);
}

PageId pageRedirect;
if (InitializeGame(&pageRedirect) != OK)
{
    return Redirect(pageRedirect);
}

const char* pszColumns[] = {
    GameEmpireDiplomacy::EmpireKey,
    GameEmpireDiplomacy::DipOffer,
    GameEmpireDiplomacy::CurrentStatus,
};

bool bGameStarted = (m_iGameState & STARTED) != 0;

IHttpForm* pHttpForm;

int i, j, iErrCode;
bool bIgnore;
const char* pszRedrawMessage = NULL;

// Read default message target
int iDefaultMessageTarget;
iErrCode = GetEmpireDefaultMessageTarget (
    m_iGameClass,
    m_iGameNumber,
    m_iEmpireKey,
    &iDefaultMessageTarget
    );

if (iErrCode != OK) {
    Assert (false);
    AddMessage ("Could not read default message target; error was ");
    AppendMessage (iErrCode);
    iDefaultMessageTarget = MESSAGE_TARGET_BROADCAST;
}
Assert (iDefaultMessageTarget >= MESSAGE_TARGET_NONE && iDefaultMessageTarget <= MESSAGE_TARGET_LAST_USED);

if (m_bOwnPost && !m_bRedirection) {

    // Handle submissions
    int iOldValue, iNewValue;

    // Handle messages:  messages will only be sent if the two empires in question are still in the game,
    // or, in the case of a broadcast, if the sender is.  This is handled in the gameengine

    // Make sure the sendmessage button was pressed
    if (WasButtonPressed (BID_SENDMESSAGE)) {

        m_bRedirectTest = false;

        const char* pszSentMessage;
        if (
            (pHttpForm = m_pHttpRequest->GetForm ("Message")) == NULL ||
            String::IsBlank ((pszSentMessage = pHttpForm->GetValue()))
            ) {
            AddMessage ("Your message was blank");
        } else {

            // See who was in the target list
            if ((pHttpForm = m_pHttpRequest->GetForm ("MessageTarget")) == NULL) {
                AddMessage ("Your message had no destination");
                pszRedrawMessage = pszSentMessage;
            } else {

                // Parse target list
                bool bBroadcast = false, bWar = false, bTruce = false, bTrade = false, bAlliance = false;
                int iNumTargets = pHttpForm->GetNumForms();
                unsigned int iNumEmpires;

                Assert (iNumTargets > 0);

                iErrCode = GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires);
                if (iErrCode != OK) {
                    AddMessage ("An error occurred reading data from the database");
                    goto Redirection;
                }

                unsigned int* piTargetKey = (unsigned int*) StackAlloc (iNumTargets * sizeof (unsigned int));
                int* piDelayEmpireKey = (int*) StackAlloc (iNumEmpires * sizeof (int));
                int iDelayEmpireKeys = 0;

                int iLastMessageTargetMask = 0;
                int* piLastMessageTargetKeyArray = (int*) StackAlloc (iNumEmpires * sizeof (int));
                int iNumLastUsed = 0;

                int iCounter = 0;

                for (i = 0; i < iNumTargets && !bBroadcast; i ++) {

                    piTargetKey[iCounter] = pHttpForm->GetForm(i)->GetUIntValue();

                    switch (piTargetKey[iCounter]) {

                    case NO_KEY:
                        bBroadcast = true;
                        iLastMessageTargetMask |= MESSAGE_TARGET_BROADCAST_MASK;
                        break;

                    case ALL_WAR:
                        bWar = true;
                        iLastMessageTargetMask |= MESSAGE_TARGET_WAR_MASK;
                        break;

                    case ALL_TRUCE:
                        bTruce = true;
                        iLastMessageTargetMask |= MESSAGE_TARGET_TRUCE_MASK;
                        break;

                    case ALL_TRADE:
                        bTrade = true;
                        iLastMessageTargetMask |= MESSAGE_TARGET_TRADE_MASK;
                        break;

                    case ALL_ALLIANCE:
                        bAlliance = true;
                        iLastMessageTargetMask |= MESSAGE_TARGET_ALLIANCE_MASK;
                        break;

                    default:
                        iLastMessageTargetMask |= MESSAGE_TARGET_INDIVIDUALS;
                        piDelayEmpireKey[iDelayEmpireKeys ++] = piTargetKey[iCounter ++];
                        break;
                    }
                }

                if (bBroadcast) {

                    iErrCode = BroadcastGameMessage (
                        m_iGameClass, 
                        m_iGameNumber, 
                        pszSentMessage,
                        m_iEmpireKey, 
                        MESSAGE_BROADCAST
                        );

                    switch (iErrCode) {

                    case OK:
                        AddMessage ("Your message was broadcast to everyone");
                        break;
                    case ERROR_CANNOT_SEND_MESSAGE:
                        AddMessage ("Your empire is not allowed to broadcast messages");
                        break;
                    case ERROR_STRING_IS_TOO_LONG:
                        AddMessage ("Your message was too long");
                        break;
                    case ERROR_EMPIRE_IS_NOT_IN_GAME:
                        AddMessage ("Your empire is not in that game");
                        break;
                    default:
                        {
                        char pszMessage [256];
                        sprintf (pszMessage, "Error %i occurred sending your message", iErrCode);
                        AddMessage (pszMessage);
                        }
                        break;
                    }

                } else {

                    bool bDipFilter = bWar || bTruce || bTrade || bAlliance;

                    // Filter out empires on war, truce, trade, alliance lists
                    if (bDipFilter) {

                        int iCurrent, iNumWar, iNumTruce, iNumTrade, iNumAlliance, 
                            * piWar, * piTruce, * piTrade, * piAlliance;
                        bool bDelete;

                        for (i = 0; i < iDelayEmpireKeys; i ++) {

                            iErrCode = GetDiplomaticStatus (
                                m_iGameClass,
                                m_iGameNumber,
                                m_iEmpireKey,
                                piDelayEmpireKey[i],
                                NULL,
                                NULL,
                                &iCurrent,
                                NULL
                                );

                            if (iErrCode != OK) {
                                // Remove from list
                                piDelayEmpireKey[i] = piDelayEmpireKey[-- iDelayEmpireKeys];
                                continue;
                            }

                            switch (iCurrent) {
                            case WAR:
                                bDelete = bWar;
                                break;
                            case TRUCE:
                                bDelete = bTruce;
                                break;
                            case TRADE:
                                bDelete = bTrade;
                                break;
                            case ALLIANCE:
                                bDelete = bAlliance;
                                break;
                            default:
                                bDelete = true;
                                Assert (false);
                            }

                            if (bDelete) {
                                // Remove from list
                                piDelayEmpireKey[i] = piDelayEmpireKey[-- iDelayEmpireKeys];
                            } else {
                                // Save last used
                                piLastMessageTargetKeyArray[iNumLastUsed ++] = piDelayEmpireKey[i];
                            }
                        }

                        piWar = (int*) StackAlloc (iNumEmpires * sizeof (int) * 4);
                        piTruce = piWar + iNumEmpires;
                        piTrade = piTruce + iNumEmpires;
                        piAlliance = piTrade + iNumEmpires;

                        // Add all lists
                        iErrCode = GetEmpiresAtDiplomaticStatus (
                            m_iGameClass,
                            m_iGameNumber,
                            m_iEmpireKey,
                            piWar,
                            &iNumWar, 
                            piTruce,
                            &iNumTruce,
                            piTrade,
                            &iNumTrade,
                            piAlliance,
                            &iNumAlliance
                            );

                        if (iErrCode == OK) {

                            if (bWar) {
                                for (i = 0; i < iNumWar; i ++) {
                                    piDelayEmpireKey[iDelayEmpireKeys ++] = piWar[i];
                                }
                            }

                            if (bTruce) {
                                for (i = 0; i < iNumTruce; i ++) {
                                    piDelayEmpireKey[iDelayEmpireKeys ++] = piTruce[i];
                                }
                            }

                            if (bTrade) {
                                for (i = 0; i < iNumTrade; i ++) {
                                    piDelayEmpireKey[iDelayEmpireKeys ++] = piTrade[i];
                                }
                            }

                            if (bAlliance) {
                                for (i = 0; i < iNumAlliance; i ++) {
                                    piDelayEmpireKey[iDelayEmpireKeys ++] = piAlliance[i];
                                }
                            }
                        }
                    }

                    String strYes, strIgnore, strNot;
                    Variant vTheEmpireName;

                    for (i = 0; i < iDelayEmpireKeys; i ++) {

                        iErrCode = GetEmpireName (piDelayEmpireKey[i], &vTheEmpireName);
                        if (iErrCode != OK) {
                            continue;
                        }

                        iErrCode = SendGameMessage (
                            m_iGameClass,
                            m_iGameNumber,
                            piDelayEmpireKey[i],
                            pszSentMessage,
                            m_iEmpireKey,
                            0,
                            NULL_TIME
                            );

                        if (iErrCode == ERROR_STRING_IS_TOO_LONG) {
                            AddMessage ("Your message was too long");
                            break;
                        }

                        switch (iErrCode) {
                        case OK:
                            if (strYes.IsBlank()) {
                                strYes += vTheEmpireName.GetCharPtr();
                            } else {
                                strYes += ", ";
                                strYes += vTheEmpireName.GetCharPtr();
                            }

                            if (!bDipFilter) {
                                // Save last used
                                piLastMessageTargetKeyArray[iNumLastUsed ++] = piDelayEmpireKey[i];
                            }

                            break;

                        case ERROR_EMPIRE_IS_IGNORING_SENDER:

                            if (strIgnore.IsBlank()) {
                                strIgnore = vTheEmpireName.GetCharPtr();
                            } else {
                                strIgnore += ", ";
                                strIgnore += vTheEmpireName.GetCharPtr();
                            }
                            break;

                        default:

                            if (strNot.IsBlank()) {
                                strNot = vTheEmpireName.GetCharPtr();
                            } else {
                                strNot += ", ";
                                strNot += vTheEmpireName.GetCharPtr();
                            }
                            break;
                        }
                    }

                    if (i == iDelayEmpireKeys) {

                        if (!strYes.IsBlank()) {
                            AddMessage ("Your message was sent to ");
                            AppendMessage (strYes);
                        }

                        if (!strIgnore.IsBlank()) {
                            AddMessage ("You are being ignored by ");
                            AppendMessage (strIgnore);
                        }

                        if (!strNot.IsBlank()) {
                            AddMessage ("Your message could not be sent to ");
                            AppendMessage (strNot);
                        }

                        if (i == 0) {
                            AddMessage ("Your message was sent to no one");
                        }
                    }
                }

                // Update last used, if necessary
                if (iDefaultMessageTarget == MESSAGE_TARGET_LAST_USED) {

                    iErrCode = SetLastUsedMessageTarget (
                        m_iGameClass,
                        m_iGameNumber,
                        m_iEmpireKey,
                        iLastMessageTargetMask,
                        iNumLastUsed == 0 ? NULL : piLastMessageTargetKeyArray,
                        iNumLastUsed
                        );

                    if (iErrCode != OK) {
                        AddMessage ("Could not set last used message target; error was ");
                        AppendMessage (iErrCode);
                    }
                }
            }
        }
    }

    // The gameengine accepts changes to diplomatic status only if they are legal at submission time
    if (bGameStarted) {

        if ((pHttpForm = m_pHttpRequest->GetForm ("NumKnownEmpires")) == NULL) {
            goto Redirection;
        }
        int iSelectedDip, iDipOffer, iFoeKey, iNumDiplomaticEmpires = pHttpForm->GetIntValue();

        char pszForm [256];

        for (i = 0; i < iNumDiplomaticEmpires; i ++) {

            // Get foe key
            sprintf (pszForm, "FoeKey%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iFoeKey = pHttpForm->GetIntValue();

            // Ignore
            sprintf (pszForm, "Ignore%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iNewValue = pHttpForm->GetIntValue();

            iErrCode = GetEmpireIgnoreMessages (m_iGameClass, m_iGameNumber, m_iEmpireKey, iFoeKey, &bIgnore);

            if (iErrCode == OK) {

                iOldValue = bIgnore ? 1:0;

                if (iOldValue != iNewValue) {

                    // Best effort
                    iErrCode = SetEmpireIgnoreMessages (
                        m_iGameClass,
                        m_iGameNumber,
                        m_iEmpireKey,
                        iFoeKey,
                        iNewValue != 0
                        );
                }
            }

            // Get previously selected dip option
            sprintf (pszForm, "SelectedDip%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iSelectedDip = pHttpForm->GetIntValue();

            // Get selected dip option
            sprintf (pszForm, "DipOffer%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iDipOffer = pHttpForm->GetIntValue();

            // Only update if we changed the selected option
            if (iSelectedDip != iDipOffer) {

                // Best effort
                iErrCode = UpdateDiplomaticOffer (
                    m_iGameClass,
                    m_iGameNumber,
                    m_iEmpireKey,
                    iFoeKey,
                    iDipOffer
                    );
            }
        }
    }
}

Redirection:
if (m_bRedirectTest)
{
    PageId pageRedirect;
    if (RedirectOnSubmitGame (&pageRedirect))
    {
        return Redirect (pageRedirect);
    }
}

OpenGamePage();

// Individual page stuff starts here

if (m_iGameRatios >= RATIOS_DISPLAY_ALWAYS) {
    GameCheck (WriteRatiosString (NULL));
}

%><p><%

UTCTime tLastLogin, tCurrentTime;
int iGameOptions, iNumUpdatesIdle, iEcon, iMil, iWins, iNukes, iNuked, iDraws, iMaxEcon, 
    iMaxMil, iNumTruces, iNumTrades, iNumAlliances;
float fMil;
Variant vIPAddress;

Time::GetTime (&tCurrentTime);

Variant** ppvEmpireData = NULL;
unsigned int* piProxyEmpireKey = NULL;

bool bSubjective = false;

int piDipKey [NUM_DIP_LEVELS], iSelected = 0, iNumOptions = 0, iSelectedIndex, iWeOffer, 
    iTheyOffer, iCurrentStatus, iKnownEmpireKey, iNumKnownEmpires = 0, iAlienKey,
    iRuins, iSec, iMin, iHour, iDay, iMonth, iYear, * piStatus = NULL, * piIndex = NULL, iIndex;
unsigned int iActiveEmpires;

DayOfWeek day;
UTCTime tCreated;

char pszCreated [OS::MaxDateLength];

bool bPrivateMessages;

Variant vKnownEmpireName, vTemp;

GAME_EMPIRE_DIPLOMACY (strGameEmpireDiplomacy, m_iGameClass, m_iGameNumber, m_iEmpireKey);

const char* pszTableColor = m_vTableColor.GetCharPtr();
const char* pszGood = m_vGoodColor.GetCharPtr();
const char* pszBad = m_vBadColor.GetCharPtr();

char pszProfile [MAX_EMPIRE_NAME_LENGTH + 128];

ICachedTable* pGameEmpireTable = NULL, * pSystemEmpireDataTable = NULL;

GAME_EMPIRE_DATA (pszEmpireData, m_iGameClass, m_iGameNumber, m_iEmpireKey);

iErrCode = t_pCache->GetTable(
    pszEmpireData, 
    &pGameEmpireTable
    );
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pGameEmpireTable->ReadData(GameEmpireData::LastLogin, &tLastLogin);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pGameEmpireTable->ReadData(GameEmpireData::Options, &iGameOptions);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pGameEmpireTable->ReadData(GameEmpireData::NumUpdatesIdle, &iNumUpdatesIdle);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pGameEmpireTable->ReadData(GameEmpireData::Econ, &iEcon);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pGameEmpireTable->ReadData(GameEmpireData::Mil, &fMil);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pGameEmpireTable->ReadData(GameEmpireData::NumTruces, &iNumTruces);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pGameEmpireTable->ReadData(GameEmpireData::NumTrades, &iNumTrades);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pGameEmpireTable->ReadData(GameEmpireData::NumAlliances, &iNumAlliances);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

SafeRelease (pGameEmpireTable);

iErrCode = t_pCache->GetTable(
    SYSTEM_EMPIRE_DATA, 
    &pSystemEmpireDataTable
    );
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::Wins, &iWins);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::Nukes, &iNukes);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::Nuked, &iNuked);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::Draws, &iDraws);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::Ruins, &iRuins);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::MaxEcon, &iMaxEcon);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::MaxMil, &iMaxMil);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::CreationTime, &tCreated);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = pSystemEmpireDataTable->ReadData(m_iEmpireKey, SystemEmpireData::IPAddress, &vIPAddress);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

SafeRelease (pSystemEmpireDataTable);

Time::GetDate (tCreated, &iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);
sprintf (pszCreated, "%s %i %i", Time::GetAbbreviatedMonthName (iMonth), iDay, iYear);


iMil = GetMilitaryValue (fMil);

if (bGameStarted) {

    bool bVisible;
    int iMaxNumTruces, iMaxNumTrades, iMaxNumAlliances, m_iGameClassOptions, m_iGameClassDip;

    iErrCode = GetGameClassVisibleDiplomacy(m_iGameClass, &bVisible);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = DoesGameClassHaveSubjectiveViews (m_iGameClass, &bSubjective);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetMaxNumDiplomacyPartners (m_iGameClass, m_iGameNumber, TRUCE, &iMaxNumTruces);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetMaxNumDiplomacyPartners (m_iGameClass, m_iGameNumber, TRADE, &iMaxNumTrades);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetMaxNumDiplomacyPartners (m_iGameClass, m_iGameNumber, ALLIANCE, &iMaxNumAlliances);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetGameClassOptions (m_iGameClass, &m_iGameClassOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetGameClassDiplomacyLevel (m_iGameClass, &m_iGameClassDip);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    %><p><table width="80%"><tr><%

    %><th bgcolor="<% Write (pszTableColor); %>">Diplomatic Settings</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Econ and Mil views</th><%
    if (m_iGameClassDip & TRUCE) {
        %><th bgcolor="<% Write (pszTableColor); %>">Truces</th><%
    }
    if (m_iGameClassDip & TRADE) {
        %><th bgcolor="<% Write (pszTableColor); %>">Trades</th><%
    }
    if (m_iGameClassDip & ALLIANCE) {
        %><th bgcolor="<% Write (pszTableColor); %>">Alliances</th><%
    }
    if (m_iGameClassDip & SURRENDER) {
        %><th bgcolor="<% Write (pszTableColor); %>">Surrenders</th><%
    }

    %></tr><tr><%

    %><td align="center"><%
    if (bVisible) {
        %>Visible<%
    } else {
        %>Not visible<%
    }
    %></td><%

    %><td align="center"><%
    if (bSubjective) {
        %>Subjective<%
    } else {
        %>Objective<%
    }
    %></td><%

    if (m_iGameClassDip & TRUCE) {

        %><td align="center"><%
        switch (iMaxNumTruces) {

        case UNRESTRICTED_DIPLOMACY:

            %>Unrestricted<%
            break;

        case FAIR_DIPLOMACY:

            %>Fair truces<%
            break;

        default:

            %>Limit of <strong><% Write (iMaxNumTruces); %></strong> truce<%
            if (iMaxNumTruces != 1) { %>s<% }
            break;
        }
        %> (<% Write (iNumTruces); %>)</td><%
    }

    if (m_iGameClassDip & TRADE) {

        %><td align="center"><%
        switch (iMaxNumTrades) {

        case UNRESTRICTED_DIPLOMACY:

            %>Unrestricted<%
            break;

        case FAIR_DIPLOMACY:

            %>Fair trades<%
            break;

        default:

            %>Limit of <strong><% Write (iMaxNumTrades); %></strong> trade<%
            if (iMaxNumTrades != 1) { %>s<% }
            break;
        }
        %> (<% Write (iNumTrades); %>)</td><%
    }

    if (m_iGameClassDip & ALLIANCE) {

        %><td align="center"><%
        switch (iMaxNumAlliances) {

        case UNRESTRICTED_DIPLOMACY:

            %>Unrestricted<%
            break;

        case FAIR_DIPLOMACY:

            %>Fair alliances<%
            break;

        default:

            %>Limit of <strong><% Write (iMaxNumAlliances); %></strong> alliance<%
            if (iMaxNumAlliances != 1) { %>s<% }
            break;
        }
        %> (<% Write (iNumAlliances); %>)<%

        if (m_iGameClassOptions & UNBREAKABLE_ALLIANCES) {
            %><br>Unbreakable<%
        }

        if (m_iGameClassOptions & PERMANENT_ALLIANCES) {
            %><br>Count for entire game<%
        }

        %></td><%
    }

    if (m_iGameClassDip & SURRENDER) {

        %><td align="center"><%

        if (m_iGameClassOptions & ONLY_SURRENDER_WITH_TWO_EMPIRES) {
            %>When 2 empires remain<%
        } else {
            %>Allowed<%
        }

        %></td><%
    }

    %></tr></table><%
}

// Self
%><p><table width="95%"><%
%><tr><th></th><%
%><th bgcolor="<% Write (pszTableColor); %>">Alien</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Econ</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Mil</th><%
%><th colspan="2" bgcolor="<% Write (pszTableColor); %>">They Offer</th><%
%><th colspan="2" bgcolor="<% Write (pszTableColor); %>">You Offer</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Status</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Messages</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Last Access</th></tr><%

%><tr><%
%><td align="center"><strong><font size="+1">You</font></strong></td><%
%><td align="center"><%

WriteProfileAlienString (
    m_iAlienKey,
    m_iEmpireKey,
    m_vEmpireName.GetCharPtr(),
    0,
    "ProfileLink",
    "View your profile",
    false,
    false
    );

%></td><%
%><td align="center"><% Write (iEcon); %></td><%
%><td align="center"><% Write (iMil); %></td><%
%><td colspan="2" align="center">-</td><%
%><td colspan="2" align="center">-</td><%
%><td align="center"><strong>-</strong></td><%
%><td align="center">-</td><%

%><td align="center"><%

WriteTime (Time::GetSecondDifference (tCurrentTime, tLastLogin));

%> ago<%

if (iGameOptions & RESIGNED) {
    %><br>(<strong><em>Resigned</em></strong>)<%
}

else if (iNumUpdatesIdle > 0) {
    %><br>(<strong><% Write (iNumUpdatesIdle); %></strong> update<%
    
    if (iNumUpdatesIdle != 1) {
        %>s<%
    }
    %> idle)<%
} %></td></tr><%

%><tr><th></th><%
%><th bgcolor="<% Write (pszTableColor); %>">Wins</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Nukes</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Nuked</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Draws</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Ruins</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Created</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Max Econ</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Max Mil</th><%
%><th bgcolor="<% Write (pszTableColor); %>">IP Address</th><%
%><th bgcolor="<% Write (pszTableColor); %>">Ready for Update</th></tr><%

%><tr><%
%><td></td><%
%><td align="center"><% Write (iWins); %></td><%
%><td align="center"><% Write (iNukes); %></td><%
%><td align="center"><% Write (iNuked); %></td><%
%><td align="center"><% Write (iDraws); %></td><%
%><td align="center"><% Write (iRuins); %></td><%
%><td align="center"><% Write (pszCreated); %></td><%
%><td align="center"><% Write (iMaxEcon); %></td><%
%><td align="center"><% Write (iMaxMil); %></td><%
%><td align="center"><% Write (vIPAddress.GetCharPtr()); %></td><%

%><td align="center"><%

if (iGameOptions & UPDATED) {
    %><strong><font color="#<% Write (pszGood); %>">Yes</font></strong><%
} else {
    %><strong><font color="#<% Write (pszBad); %>">No</font></strong><%
} %></td></tr><%

// Other empires
iErrCode = t_pCache->ReadColumns (
    strGameEmpireDiplomacy,
    countof (pszColumns),
    pszColumns,
    &piProxyEmpireKey,
    &ppvEmpireData,
    (unsigned int*) &iNumKnownEmpires
    );

if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
    Assert (false);
    goto Cleanup;
}

%><input type="hidden" name="NumKnownEmpires" value="<% Write (iNumKnownEmpires); %>"><%

// Sort by diplomatic status
piStatus = (int*) StackAlloc (iNumKnownEmpires * 2 * sizeof (int));
piIndex = piStatus + iNumKnownEmpires;

for (i = 0; i < iNumKnownEmpires; i ++) {
    piStatus[i] = ppvEmpireData[i][2].GetInteger();
    piIndex[i] = i;
}
Algorithm::QSortTwoDescending<int, int> (piStatus, piIndex, iNumKnownEmpires);

for (iIndex = 0; iIndex < iNumKnownEmpires; iIndex ++) {

    i = piIndex [iIndex];

    // Get diplomacy data
    iKnownEmpireKey = ppvEmpireData[i][0].GetInteger();
    iWeOffer = ppvEmpireData[i][1].GetInteger();
    iCurrentStatus = ppvEmpireData[i][2].GetInteger();

    iErrCode = GetDiplomaticStatus (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        iKnownEmpireKey,
        NULL,
        &iTheyOffer,
        NULL,
        NULL
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (bGameStarted) {

        iErrCode = GetDiplomaticOptions (
            m_iGameClass,
            m_iGameNumber,
            m_iEmpireKey,
            iKnownEmpireKey,
            piDipKey,
            &iSelected,
            &iNumOptions
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    GET_GAME_EMPIRE_DATA (pszEmpireData, m_iGameClass, m_iGameNumber, iKnownEmpireKey)

    // Get empire data
    iErrCode = t_pCache->GetTable(pszEmpireData, &pGameEmpireTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameEmpireTable->ReadData(GameEmpireData::LastLogin, &tLastLogin);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameEmpireTable->ReadData(GameEmpireData::Options, &iGameOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pGameEmpireTable->ReadData(GameEmpireData::NumUpdatesIdle, &iNumUpdatesIdle);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (bSubjective) {

        SafeRelease (pGameEmpireTable);

        iErrCode = t_pCache->ReadData(
            strGameEmpireDiplomacy,
            piProxyEmpireKey[i],
            GameEmpireDiplomacy::SubjectiveEcon,
            &vTemp
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iEcon = vTemp.GetInteger();

        iErrCode = t_pCache->ReadData(
            strGameEmpireDiplomacy,
            piProxyEmpireKey[i],
            GameEmpireDiplomacy::SubjectiveMil,
            &vTemp
            );
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iMil = vTemp.GetInteger();

    } else {

        iErrCode = pGameEmpireTable->ReadData(GameEmpireData::Econ, &iEcon);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pGameEmpireTable->ReadData(GameEmpireData::Mil, &fMil);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        SafeRelease (pGameEmpireTable);

        iMil = GetMilitaryValue (fMil);
    }

    // Do this every time to improve concurrency with logins, etc.
    iErrCode = t_pCache->GetTable(SYSTEM_EMPIRE_DATA, &pSystemEmpireDataTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::Name, &vKnownEmpireName);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::AlienKey, &iAlienKey);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::Wins, &iWins);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::Nukes, &iNukes);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::Nuked, &iNuked);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::Draws, &iDraws);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::Ruins, &iRuins);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::MaxEcon, &iMaxEcon);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::MaxMil, &iMaxMil);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::CreationTime, &tCreated);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystemEmpireDataTable->ReadData(iKnownEmpireKey, SystemEmpireData::IPAddress, &vIPAddress);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    SafeRelease (pSystemEmpireDataTable);

    Time::GetDate (tCreated, &iSec, &iMin, &iHour, &day, &iDay, &iMonth, &iYear);
    sprintf (pszCreated, "%s %i %i", Time::GetAbbreviatedMonthName (iMonth), iDay, iYear);

    %><tr><td colspan="11">&nbsp;</td></tr><%

    if ((iIndex == 0 && piStatus[0] != ALLIANCE) || (iIndex > 0 && piStatus[iIndex] != piStatus[iIndex - 1])) {
        %><tr><td align="center" colspan="11"><% WriteSeparatorString (m_iSeparatorKey); %></td></tr><%
        %><tr><td colspan="11">&nbsp;</td></tr><%
    }

    %><tr><%
    %><th><%
    %></th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Alien</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Econ</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Mil</th><%
    %><th colspan="2" bgcolor="<% Write (pszTableColor); %>">They Offer</th><%
    %><th colspan="2" bgcolor="<% Write (pszTableColor); %>">You Offer</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Status</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Messages</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Last Access</th></tr><tr><%

    %><td width="10%" align="center"><strong><%

    if (iGameOptions & RESIGNED) {
        %><strike><%
    }

    if (iCurrentStatus == WAR) {
        %><font size="+1" color="#<% Write (pszBad); %>"><% 
        Write (vKnownEmpireName.GetCharPtr()); %></font><%
    }
    else if (iCurrentStatus == ALLIANCE) {
        %><font size="+1" color="#<% Write (pszGood); %>"><% 
        Write (vKnownEmpireName.GetCharPtr()); %></font><%
    }
    else {
        %><font size="+1"><% Write (vKnownEmpireName.GetCharPtr()); %></font><%
    }

    if (iGameOptions & RESIGNED) {
        %></strike><%
    }
    %></strong></td><%

    %><td align="center"><%

    sprintf (pszProfile, "View the profile of %s", vKnownEmpireName.GetCharPtr());
    WriteProfileAlienString (
        iAlienKey,
        iKnownEmpireKey,
        vKnownEmpireName.GetCharPtr(),
        0,
        "ProfileLink",
        pszProfile,
        false,
        true
        );

    NotifyProfileLink();

    %></td><%

    %><td align="center"><% Write (iEcon); %></td><%
    %><td align="center"><% Write (iMil); %></td><%

    if (!bGameStarted) {
        %><td colspan="2" align="center">-</td><%
        %><td colspan="2" align="center">-</td><%
        %><td align="center">-</td><%
    } else {

        %><td colspan="2" align="center"><%

        if (iTheyOffer == WAR) {
            %><font color="#<% Write (pszBad); %>"><% Write (WAR_STRING);
        }

        else if (iTheyOffer == ALLIANCE) {
            %><font color="#<% Write (pszGood); %>"><% 
            Write (ALLIANCE_STRING); %></font><%
        } else {
            Write (DIP_STRING (iTheyOffer)); 
        }

        %></td><td colspan="2" align="center"><%

        if (iNumOptions > 1) {
            %><select name="DipOffer<% Write (i); %>" size="1"><%
            iSelectedIndex = -1;
            for (j = 0; j < iNumOptions; j ++) {
                if (piDipKey[j] == iSelected) { 
                    iSelectedIndex = j;
                    %><option selected value="<% Write (piDipKey[j]); %>"><% Write (DIP_STRING (piDipKey[j]));
                    %></option><%
                } else {
                    %><option value="<% Write (piDipKey[j]); %>"><% Write (DIP_STRING (piDipKey[j])); %></option><%
                }
            } %></select><%

            if (iSelectedIndex == -1) {
                iSelectedIndex = piDipKey[0];
            }

            %><input type="hidden" name="SelectedDip<% Write (i); %>" value ="<% 

            Write (piDipKey[iSelectedIndex]); %>"><%

        } else {

            if (piDipKey[0] == WAR) {
                %><font color="#<% Write (pszBad); %>"><% 
                Write (WAR_STRING);
            } else if (piDipKey[0] == ALLIANCE) {
                %><font color="#<% Write (pszGood); %>"><% 
                Write (ALLIANCE_STRING); %></font><%
            } else {
                Write (DIP_STRING (piDipKey[0])); 
            }

            %><input type="hidden" name="DipOffer<% Write (i); %>" value ="<% Write (piDipKey[0]); %>"><%
            %><input type="hidden" name="SelectedDip<% Write (i); %>" value ="<% Write (piDipKey[0]); %>"><%
        }

        %><input type="hidden" name="FoeKey<% Write (i); %>" value="<% Write (iKnownEmpireKey); %>"><%

        %></td><td align="center"><strong><%

        if (iCurrentStatus == WAR) {
            %><font color="#<% Write (pszBad); %>"><% 
            Write (WAR_STRING);
        }

        else if (iCurrentStatus == ALLIANCE) {
            %><font color="#<% Write (pszGood); %>"><% 
            Write (ALLIANCE_STRING); %></font><%
        } else {
            Write (DIP_STRING (iCurrentStatus)); 
        } %></strong></td><%
    
    }   // End if game started

    %><td align="center"><% 
    %><select name="Ignore<% Write (i); %>"><%

    iErrCode = GetEmpireIgnoreMessages (
        m_iGameClass, 
        m_iGameNumber, 
        m_iEmpireKey, 
        iKnownEmpireKey, 
        &bIgnore
        );
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (bIgnore) {
        %><option value="0">Hear</option><%
        %><option selected value="1">Ignore</option><%
    } else {
        %><option selected value="0">Hear</option><%
        %><option value="1">Ignore</option><%
    }
    %></select><%
    %></td><%

    %><td align="center"><%

    WriteTime (Time::GetSecondDifference (tCurrentTime, tLastLogin));

    %> ago <%

    if (iGameOptions & RESIGNED) {
        %><br>(<strong><em>Resigned</em></strong>)<%
    }

    else if (iNumUpdatesIdle > 0) {
        %><br>(<strong><% Write (iNumUpdatesIdle); %></strong> update<%
        
        if (iNumUpdatesIdle != 1) {
            %>s<%
        }
        %> idle)<%
    } %></td></tr><%

    %><tr><%

    %><th></th><%

    %><th bgcolor="<% Write (pszTableColor); %>">Wins</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Nukes</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Nuked</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Draws</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Ruins</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Created</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Max Econ</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Max Mil</th><%
    %><th bgcolor="<% Write (pszTableColor); %>"><%

    if (m_iPrivilege < ADMINISTRATOR) {
        %>Hashed <%
    }

    %>IP Address</th><%
    %><th bgcolor="<% Write (pszTableColor); %>">Ready for Update</th></tr><%

    %><tr><td></td><%
    %><td align="center"><% Write (iWins); %></td><%
    %><td align="center"><% Write (iNukes); %></td><%
    %><td align="center"><% Write (iNuked); %></td><%
    %><td align="center"><% Write (iDraws); %></td><%
    %><td align="center"><% Write (iRuins); %></td><%
    %><td align="center"><% Write (pszCreated); %></td><%
    %><td align="center"><% Write (iMaxEcon); %></td><%
    %><td align="center"><% Write (iMaxMil); %></td><%

    %><td align="center"><% 

    if (m_iPrivilege < ADMINISTRATOR) {
        char pszHashedIPAddress [128];
        HashIPAddress (vIPAddress.GetCharPtr(), pszHashedIPAddress);
        Write (pszHashedIPAddress);
    } else {
        Write (vIPAddress.GetCharPtr());
    }

    %></td><%

    %><td align="center"><% 
    if (iGameOptions & UPDATED) {
        %><strong><font color="#<% Write (pszGood); %>">Yes</font></strong><%
    } else {
        %><strong><font color="#<% Write (pszBad); %>">No</font></strong><%
    } %></td></tr><%
}   // End known empire loop

//
// Messages
//

iErrCode = DoesGameClassAllowPrivateMessages (m_iGameClass, &bPrivateMessages);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

iErrCode = GetNumEmpiresInGame(m_iGameClass, m_iGameNumber, &iActiveEmpires);
if (iErrCode != OK) {
    Assert (false);
    goto Cleanup;
}

if (iNumKnownEmpires > 0 || iActiveEmpires > 1) {
    %><tr><td colspan="11">&nbsp;</td></tr><%
}

%></table><%

if (iActiveEmpires > 1) {

    WriteSeparatorString (m_iSeparatorKey);

    bool bBroadcast = bBroadcast = (m_iSystemOptions & CAN_BROADCAST) != 0;
    bool bAllTargets = bPrivateMessages && bGameStarted;

    if (bBroadcast || (bPrivateMessages && iNumKnownEmpires > 0)) {

        int iExtra = 0, iWar = 0, iTruce = 0, iTrade = 0, iAlliance = 0, iDipTargets = 0;

        if (bAllTargets) {

            iErrCode = GetNumEmpiresAtDiplomaticStatus (
                m_iGameClass,
                m_iGameNumber,
                m_iEmpireKey,
                &iWar,
                &iTruce,
                &iTrade,
                &iAlliance
                );

            if (iErrCode != OK) {
                Assert (false);
                iWar = iTruce = iTrade = iAlliance = iDipTargets = 0;
            } else {

                if (iWar > 0) {
                    iExtra ++;
                }

                if (iTruce > 0) {
                    iExtra ++;
                }

                if (iTrade > 0) {
                    iExtra ++;
                }

                if (iAlliance > 0) {
                    iExtra ++;
                }

                iDipTargets = iWar + iTruce + iTrade + iAlliance;
            }
        }

        // Array of bits for selected
        size_t stAlloc = (NUM_MESSAGE_TARGETS + iNumKnownEmpires) * sizeof (bool);
        bool* pbSelected = (bool*) StackAlloc (stAlloc);
        memset (pbSelected, 0, stAlloc);

        switch (iDefaultMessageTarget) {

        case MESSAGE_TARGET_NONE:

            if (iNumKnownEmpires == 0) {
                pbSelected[MESSAGE_TARGET_BROADCAST] = true;
            }
            break;

        case MESSAGE_TARGET_LAST_USED:

            {

            // Handle last used
            int iLastUsedMask;
            unsigned int* piLastUsedProxyKeyArray = NULL, iNumLastUsed;

            iErrCode = GetLastUsedMessageTarget (
                m_iGameClass,
                m_iGameNumber,
                m_iEmpireKey,
                &iLastUsedMask,
                &piLastUsedProxyKeyArray,
                &iNumLastUsed
                );

            // Best effort
            Assert (iErrCode == OK);

            if (iErrCode == OK) {

                // Process mask
                if ((iLastUsedMask & (~MESSAGE_TARGET_INDIVIDUALS)) == 0) {

                    // Default to broadcast if no last target
                    if (iNumLastUsed == 0) {
                        pbSelected[MESSAGE_TARGET_BROADCAST] = true;
                    }

                } else {

                    if (iLastUsedMask & MESSAGE_TARGET_BROADCAST_MASK) {
                        pbSelected[MESSAGE_TARGET_BROADCAST] = true;
                    }

                    if (iLastUsedMask & MESSAGE_TARGET_WAR_MASK) {
                        pbSelected[MESSAGE_TARGET_WAR] = true;
                    }

                    if (iLastUsedMask & MESSAGE_TARGET_TRUCE_MASK) {
                        pbSelected[MESSAGE_TARGET_TRUCE] = true;
                    }

                    if (iLastUsedMask & MESSAGE_TARGET_TRADE_MASK) {
                        pbSelected[MESSAGE_TARGET_TRADE] = true;
                    }

                    if (iLastUsedMask & MESSAGE_TARGET_ALLIANCE_MASK) {
                        pbSelected[MESSAGE_TARGET_ALLIANCE] = true;
                    }
                }

                // Process array
                for (i = 0; i < (int)iNumLastUsed; i ++) {

                    // Find order of empire
                    for (j = 0; j < iNumKnownEmpires; j ++) {

                        if (piLastUsedProxyKeyArray[i] == (int) piProxyEmpireKey[j]) {
                            break;
                        }
                    }

                    if (j < iNumKnownEmpires) {
                        pbSelected[NUM_MESSAGE_TARGETS + j] = true;
                    }
                }

                t_pCache->FreeKeys (piLastUsedProxyKeyArray);
            }

            }
            break;

        default:

            pbSelected[iDefaultMessageTarget] = true;
            break;
        }

        %><p><table width="90%"><tr><td><strong>Send a message to:</strong></td><%

        %><td align="center" rowspan="3"><textarea name="Message" rows="7" cols="60" wrap="virtual"><% 
        if (pszRedrawMessage != NULL) {
            Write (pszRedrawMessage);
        } %></textarea></td><%

        %><tr><td><select name="MessageTarget" multiple size="<%

        if (bPrivateMessages) {

            if (iNumKnownEmpires < 5) {
                Write (iNumKnownEmpires + iExtra + (bBroadcast ? 1 : 0));
            } else {
                %>5<%
            }
        } else {
            %>1<%
        } %>"><%

        bool bNoPrivates = !bPrivateMessages || iNumKnownEmpires == 0;
        bool bNoDipTargets = !bAllTargets || iDipTargets == 0;

        if (bBroadcast) {

            %><option<%
            if (pbSelected[MESSAGE_TARGET_BROADCAST] || (bNoPrivates && bNoDipTargets)) {
                %> selected<%
            }
            %> value="<% Write (NO_KEY); %>">Broadcast</option><%
        }

        if (bAllTargets) {

            if (iWar > 0) {
                %><option<%
                if (pbSelected[MESSAGE_TARGET_WAR] || (!bBroadcast && bNoPrivates && iWar == iDipTargets)) {
                    %> selected<%
                }
                %> value="<% Write (ALL_WAR); %>">All at War</option><%
            }

            if (iTruce > 0) {
                %><option<%
                if (pbSelected[MESSAGE_TARGET_TRUCE] || (!bBroadcast && bNoPrivates && iTruce == iDipTargets)) {
                    %> selected<%
                }
                %> value="<% Write (ALL_TRUCE); %>">All at Truce</option><%
            }

            if (iTrade > 0) {
                %><option<%
                if (pbSelected[MESSAGE_TARGET_TRADE] || (!bBroadcast && bNoPrivates && iTrade == iDipTargets)) {
                    %> selected<%
                }
                %> value="<% Write (ALL_TRADE); %>">All at Trade</option><%
            }

            if (iAlliance > 0) {
                %><option<%
                if (pbSelected[MESSAGE_TARGET_ALLIANCE] || (!bBroadcast && bNoPrivates && iAlliance == iDipTargets)) {
                    %> selected<%
                }
                %> value="<% Write (ALL_ALLIANCE); %>">All at Alliance</option><%
            }
        }

        if (bPrivateMessages) {

            Variant vSendEmpireName;
            for (i = 0; i < iNumKnownEmpires; i ++) {

                iKnownEmpireKey = ppvEmpireData[i][0].GetInteger();

                iErrCode = GetEmpireIgnoreMessages (
                    m_iGameClass,
                    m_iGameNumber, 
                    iKnownEmpireKey,
                    m_iEmpireKey,
                    &bIgnore
                    );
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (!bIgnore) {

                    iErrCode = GetEmpireName (iKnownEmpireKey, &vSendEmpireName);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }

                    %><option<%
                    if (pbSelected[NUM_MESSAGE_TARGETS + i] || (iNumKnownEmpires == 1 && !bBroadcast && bNoDipTargets)) {
                        %> selected<%
                    }
                    %> value="<% Write (iKnownEmpireKey); %>"><% 
                    Write (vSendEmpireName.GetCharPtr()); %></option><%
                }
            }
        }
        %></select></td><tr><td><%

        WriteButton (BID_SENDMESSAGE);

        %></table><%
    }
}

Cleanup:

if (piProxyEmpireKey != NULL) {
    t_pCache->FreeKeys (piProxyEmpireKey);
}

if (ppvEmpireData != NULL) {
    t_pCache->FreeData (ppvEmpireData);
}

if (pGameEmpireTable != NULL) {
    pGameEmpireTable->Release();
}

if (pSystemEmpireDataTable != NULL) {
    pSystemEmpireDataTable->Release();
}

if (iErrCode != OK) {
    %><p>Error <% Write (iErrCode); %> occurred processing the Diplomacy page<%
}

CloseGamePage();

%>