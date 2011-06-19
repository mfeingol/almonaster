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


const char* ppszActiveGameListHeaders[] = {
    "Name",
    "Ready",
    "Updates",
    "Next update",
    "Closes",
    "Empires",
    "Features",
    "Dip",
    "Resources",
    "Bridier",
    "Score",
    "Security",
    "Options",
};

const char* ppszOpenGameListHeaders[] = {
    "Name",
    "Missed updates",
    "Next update",
    "Update period",
    "Empires",
    "Planets",
    "Tech",
    "Dip",
    "Resources",
    "Initial techs",
    "Possible techs",
    "Bridier",
    "Score",
    "Security",
    "Options",
};

const char* ppszSpectatorGameListHeaders[] = {
    "Name",
    "Updates",
    "Next update",
    "Update period",
    "Empires",
    "Planets",
    "Tech",
    "Dip",
    "Resources",
    "Initial techs",
    "Possible techs",
    "Bridier",
    "Score",
    "Security",
    "Options",
};

const char* ppszSystemGameListHeaders[] = {
    "Name",
    "Update period",
    "Empires",
    "Planets",
    "Tech",
    "Dip",
    "Resources",
    "Initial techs",
    "Possible techs",
    "Options",
};

const char* ppszGameAdministratorListHeaders[] = {
    "Name",
    "Updates",
    "Next update",
    "Update period",
    "Empires",
    "Planets",
    "Tech",
    "Dip",
    "Resources",
    "Initial techs",
    "Possible techs",
    "Bridier",
    "Score",
    "Security",
    "Options",
};

void HtmlRenderer::WriteActiveGameListHeader (const char* pszTableColor) {
    WriteGameListHeader (ppszActiveGameListHeaders, sizeof (ppszActiveGameListHeaders) / sizeof (char*), pszTableColor);
}

void HtmlRenderer::WriteOpenGameListHeader (const char* pszTableColor) {
    WriteGameListHeader (ppszOpenGameListHeaders, sizeof (ppszOpenGameListHeaders) / sizeof (char*), pszTableColor);
}

void HtmlRenderer::WriteSpectatorGameListHeader (const char* pszTableColor) {
    WriteGameListHeader (ppszSpectatorGameListHeaders, sizeof (ppszSpectatorGameListHeaders) / sizeof (char*), pszTableColor);
}

void HtmlRenderer::WriteSystemGameListHeader (const char* pszTableColor) {
    WriteGameListHeader (ppszSystemGameListHeaders, sizeof (ppszSystemGameListHeaders) / sizeof (char*), pszTableColor);
}

void HtmlRenderer::WriteGameAdministratorListHeader (const char* pszTableColor) {
    WriteGameListHeader (ppszGameAdministratorListHeaders, sizeof (ppszGameAdministratorListHeaders) / sizeof (char*), pszTableColor);
}

void HtmlRenderer::WriteGameListHeader (const char** ppszHeaders, size_t stNumHeaders, const char* pszTableColor) {
    
    size_t i;
    
    // Setup string
    OutputText ("<p><center><table width=\"90%\" cellspacing=\"1\" cellpadding=\"2\"><tr>");
    
    for (i = 0; i < stNumHeaders; i ++) {
        OutputText ("<th bgcolor=\"");
        m_pHttpResponse->WriteText (pszTableColor);
        OutputText ("\">");
        m_pHttpResponse->WriteText (ppszHeaders[i]);
        OutputText ("</th>");
    }
    
    OutputText ("</tr>");
}


int HtmlRenderer::WriteActiveGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo) {
    
    int iErrCode, iButtonKey = m_iButtonKey, iEmpireKey =  m_iEmpireKey, i, iNumUpdates, iNumActiveEmpires,
        iSecondsSince, iSecondsUntil, iState, iGameOptions,
        iNumUpdatesBeforeGameCloses, iEmpireGameOptions, iNumUpdatedEmpires = 0;

    GameFairnessOption gfoFairness;
    unsigned int iNumUnreadMessages;

    Seconds sFirstUpdateDelay = 0;
    bool bReadyForUpdate, bOpen;

    Variant* pvEmpireKey = NULL, pvMin [NUM_ENTRY_SCORE_RESTRICTIONS], pvMax [NUM_ENTRY_SCORE_RESTRICTIONS], vTemp;

    UTCTime tCreationTime;

    String strList;

    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];
    char pszDateString [OS::MaxDateLength], pszLogin[64];

    //
    // Read data
    //

    iErrCode = g_pGameEngine->GetEmpireOptions (iGameClass, iGameNumber, m_iEmpireKey, &iEmpireGameOptions);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetGameProperty(iGameClass, iGameNumber, GameData::MapFairness, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    gfoFairness = (GameFairnessOption)vTemp.GetInteger();

    bReadyForUpdate = (iEmpireGameOptions & UPDATED) != 0;

    iErrCode = g_pGameEngine->GetGameUpdateData (
        iGameClass, 
        iGameNumber, 
        &iSecondsSince,
        &iSecondsUntil, 
        &iNumUpdates,
        &iState
        );

    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    iErrCode = g_pGameEngine->GetNumUnreadGameMessages (iGameClass, iGameNumber, iEmpireKey, &iNumUnreadMessages);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->IsGameOpen (iGameClass, iGameNumber, &bOpen);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    iErrCode = g_pGameEngine->GetGameCreationTime (iGameClass, iGameNumber, &tCreationTime);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    iErrCode = g_pGameEngine->GetNumUpdatesBeforeGameCloses (iGameClass, iGameNumber, &iNumUpdatesBeforeGameCloses);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (iNumUpdates == 0) {
        iErrCode = g_pGameEngine->GetFirstUpdateDelay (iGameClass, iGameNumber, &sFirstUpdateDelay);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }
    
    iErrCode = Time::GetDateString (tCreationTime, pszDateString);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    iErrCode = g_pGameEngine->GetGameEntryRestrictions (iGameClass, iGameNumber, &iGameOptions, pvMin, pvMax);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (iGameOptions & GAME_NAMES_LISTED) {

        iErrCode = g_pGameEngine->GetEmpiresInGame (iGameClass, iGameNumber, &pvEmpireKey, &iNumActiveEmpires);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
    } else {
        
        iErrCode = g_pGameEngine->GetNumEmpiresInGame (iGameClass, iGameNumber, &iNumActiveEmpires);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }
    
    if (iState & STARTED) {
        
        iErrCode = g_pGameEngine->GetNumUpdatedEmpires (iGameClass, iGameNumber, &iNumUpdatedEmpires);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }
    
    // Description
    OutputText (
        "<tr><td colspan=\"13\" align=\"center\">"\
        "<font size=\"-1\" face=\"" DEFAULT_GAMECLASS_DESCRIPTION_FONT "\"><br>"
        );
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iDescription].GetCharPtr());
    
    // Name
    OutputText ("</font></td></tr><tr><td width=\"20%\"><font size=\"3\">");
    
    iErrCode = g_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    m_pHttpResponse->WriteText (pszGameClassName);
    
    OutputText ("<strong> ");
    m_pHttpResponse->WriteText (iGameNumber);
    OutputText ("</strong></font>");
    
    // Time
    OutputText ("<p><font size=\"-1\">(Started <strong>");
    m_pHttpResponse->WriteText (pszDateString);
    OutputText ("</strong>)</font>");
    
    // Login
    if (m_iPrivilege > GUEST) {
    
        OutputText ("<p>");

        sprintf (pszLogin, "Login%i.%i", iGameClass, iGameNumber);
        WriteButtonString (iButtonKey, "Login", "Login", pszLogin);
    }

    // Ready
    OutputText ("</td><td align=\"center\">");
    
    if (!(iState & STARTED)) {

        OutputText ("<font size=\"2\">N/A");

    } else {

        Variant vUpdatesIdle;

        OutputText ("<font size=\"4\" color=\"#");
        
        iErrCode = g_pGameEngine->GetEmpireGameProperty (
            iGameClass,
            iGameNumber,
            m_iEmpireKey,
            GameEmpireData::NumUpdatesIdle,
            &vUpdatesIdle
            );
        
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        if ((iEmpireGameOptions & LOGGED_IN_THIS_UPDATE) || vUpdatesIdle.GetInteger() == 0) {
            
            if (bReadyForUpdate) {
                
                m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
                OutputText ("\">Yes");
                
            } else {
                
                m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                OutputText ("\">No");
            }
            
        } else {
            
            m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
            OutputText ("\">");
            m_pHttpResponse->WriteText (vUpdatesIdle.GetInteger());
            OutputText (" update");
            if (vUpdatesIdle.GetInteger() != 1) {
                OutputText ("s");
            }
            OutputText (" idle");
        }

        OutputText ("</font><p><font size=\"2\">(<strong>");
        m_pHttpResponse->WriteText (iNumUpdatedEmpires);
        OutputText ("</strong> of <strong>");
        m_pHttpResponse->WriteText (iNumActiveEmpires);
        OutputText ("</strong> ready)");
    }
    OutputText ("</font>");

    // Unread messages
    if (iNumUnreadMessages > 0) {
        OutputText ("<p><strong><font color=\"#");
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">"); 
        m_pHttpResponse->WriteText (iNumUnreadMessages);
        
        OutputText ("</font></strong> <font size=\"2\">unread message");
        if (iNumUnreadMessages != 1) {
            OutputText ("s");
        }
        OutputText ("</font>");
    }

    OutputText ("</td>");
    
    // Updates
    if (iNumUpdates == 0) {
        OutputText ("<td align=\"center\"><font size=\"2\">None");
    } else {
        OutputText ("<td align=\"center\"><font size=\"2\"><strong>");
        m_pHttpResponse->WriteText (iNumUpdates);
        OutputText ("</strong> update");
        if (iNumUpdates != 1) {
            OutputText ("s");
        }
    }
    OutputText ("</font></td>");
    
    // Next update
    if (!(iState & STARTED)) {

        int iNumNeeded = pvGameClassInfo[SystemGameClassData::iMinNumEmpires].GetInteger() - iNumActiveEmpires;
        
        OutputText ("<td align=\"center\"><font size=\"2\">When <strong>");
        m_pHttpResponse->WriteText (iNumNeeded);
        
        if (iNumNeeded == 1) {
            OutputText ("</strong> more empire joins");
        } else {
            OutputText ("</strong> more empires join");
        }
        
    } else {
        
        OutputText ("<td align=\"center\"><font size=\"2\">");
        
        WriteTime (iSecondsUntil);
        
        if (iState & ADMIN_PAUSED) {
            OutputText ("<br>(Paused by an administrator)");
        }
        
        else if (iState & PAUSED) {
            OutputText (" (paused)");
        }
    }
    OutputText ("</font></td>");
    
    // Closes
    if (bOpen) {
        
        int iUpdatesNeeded = iNumUpdatesBeforeGameCloses;
        if (iState & STARTED) {
            iUpdatesNeeded -= iNumUpdates;
        }
        
        OutputText ("<td align=\"center\"><font size=\"2\">");

        if (!(iState & STARTED)) {
            OutputText ("N/A");
        }
        
        else if (iUpdatesNeeded < 1) {
            OutputText ("Closed");
        }
        
        else {

            OutputText ("After <strong>");
            m_pHttpResponse->WriteText (iUpdatesNeeded);
            
            if (iUpdatesNeeded == 1) { 
                OutputText ("</strong> update");
            } else {
                OutputText ("</strong> updates");
            }
        }
        
    } else {
        OutputText ("<td align=\"center\"><font size=\"2\">Closed");
    }
    OutputText ("</font></td>");
    
    // Empires in game
    if (iGameOptions & GAME_NAMES_LISTED) {

        Variant vName, vOptions;

        // Empire names may fail; halted empire quits
        int iLoopGuard = iNumActiveEmpires - 1;
        for (i = 0; i <= iLoopGuard; i ++) {
            
            unsigned int iEmpireKey = pvEmpireKey[i].GetInteger();

            if (g_pGameEngine->GetEmpireName (iEmpireKey, &vName) == OK &&
                g_pGameEngine->GetEmpireGameProperty (iGameClass, iGameNumber, iEmpireKey, GameEmpireData::Options, &vOptions) == OK) {

                bool bUpdated = (vOptions.GetInteger() & UPDATED) != 0;
                if (bUpdated) {
                    strList += "<font color=\"#";
                    strList += m_vGoodColor.GetCharPtr();
                    strList += "\">";
                } else {
                    strList += "<font color=\"#";
                    strList += m_vBadColor.GetCharPtr();
                    strList += "\">";
                }

                strList += vName.GetCharPtr();
                if (i < iLoopGuard) {
                    strList += ", ";
                }

                if (bUpdated) {
                    strList += "</font>";
                } else {
                    strList += "</font>";
                }
            }
        }
    }

    AddEmpiresInGame (
        iState,
        iNumActiveEmpires, 
        strList.GetCharPtr(), 
        pvGameClassInfo[SystemGameClassData::iMinNumEmpires].GetInteger(),
        pvGameClassInfo[SystemGameClassData::iMaxNumEmpires].GetInteger()
        );
    
    // Features
    OutputText (
        "<td align=\"center\" width=\"10%\">"\
        "<font size=\"2\">"\
        "Updates every "
        );
    WriteTime (pvGameClassInfo[SystemGameClassData::iNumSecPerUpdate].GetInteger());
    
    if (iNumUpdates == 0 && sFirstUpdateDelay > 0) {
        OutputText ("<br>(");
        WriteTime (sFirstUpdateDelay);
        OutputText (" delay)");
    }

    OutputText ("<br><strong>");
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMinNumPlanets].GetInteger());
    OutputText ("</strong> ");
    
    if (pvGameClassInfo[SystemGameClassData::iMaxNumPlanets].GetInteger() == 
        pvGameClassInfo[SystemGameClassData::iMinNumPlanets].GetInteger()) {
        
        OutputText ("planet");
        if (pvGameClassInfo[SystemGameClassData::iMaxNumPlanets].GetInteger() != 1) {
            OutputText ("s");
        }
        
    } else {
        
        OutputText ("to <strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxNumPlanets].GetInteger());
        OutputText ("</strong> planets");
    }

    if (iGameOptions & GAME_MIRRORED_MAP) {
        OutputText ("<br>Mirrored map");
    }
    else if (iGameOptions & GAME_TWISTED_MAP) {
        OutputText ("<br>Twisted map");
    }

    switch (gfoFairness) {
    case GAME_FAIRNESS_RANDOM:
        OutputText("<br>Random map");
        break;
    case GAME_FAIRNESS_VERY_FAIR:
        OutputText("<br>Very fair map");
        break;
    case GAME_FAIRNESS_SOMEWHAT_FAIR:
        OutputText("<br>Somewhat fair map");
        break;
    case GAME_FAIRNESS_SOMEWHAT_UNFAIR:
        OutputText("<br>Somewhat unfair map");
        break;
    case GAME_FAIRNESS_VERY_UNFAIR:
        OutputText("<br>Very unfair map");
        break;
    }

    OutputText ("<br><strong>");
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iInitialTechLevel].GetFloat());
    OutputText ("</strong> initial tech<br><strong>");
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxTechDev].GetFloat());
    OutputText ("</strong> delta tech");

    if (pvGameClassInfo[SystemGameClassData::iInitialTechDevs].GetInteger() != ALL_TECHS) {    

        OutputText ("<br><strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iNumInitialTechDevs].GetInteger());
        OutputText ("</strong> tech dev");

        if (pvGameClassInfo[SystemGameClassData::iNumInitialTechDevs].GetInteger() != 1) {
            OutputText ("s");
        }
    }
    OutputText ("</font></td>");
    
    // Dip
    AddDiplomacy (pvGameClassInfo);

    // Resources
    AddResources (pvGameClassInfo);

    // Bridier, Score
    AddBridier (iGameClass, iGameNumber, pvGameClassInfo, iGameOptions, pvMin, pvMax, (iState & STARTED) != 0);
    AddScore (iGameOptions, pvMin, pvMax);
    AddSecurity (iGameOptions);

    // Options
    AddOptions (ACTIVE_GAME_LIST, pvGameClassInfo, iGameOptions);
    
Cleanup:

    if (pvEmpireKey != NULL) {
        g_pGameEngine->FreeData (pvEmpireKey);
    }   

    return iErrCode;
}

int HtmlRenderer::WriteOpenGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo) {

    return WriteInPlayGameListData (iGameClass, iGameNumber, pvGameClassInfo, false, false);
}

int HtmlRenderer::WriteSpectatorGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo) {

    return WriteInPlayGameListData (iGameClass, iGameNumber, pvGameClassInfo, false, true);
}

int HtmlRenderer::WriteGameAdministratorListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo) {

    return WriteInPlayGameListData (iGameClass, iGameNumber, pvGameClassInfo, true, false);
}

int HtmlRenderer::WriteInPlayGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, 
                                           bool bAdmin, bool bSpectators) {

    int iErrCode, iNumEmpiresInGame, iGameOptions;

    String strList;

    char pszEnter [64];
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];

    iErrCode = g_pGameEngine->GetGameOptions (iGameClass, iGameNumber, &iGameOptions);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    if (bAdmin || bSpectators || (iGameOptions & GAME_NAMES_LISTED)) {

        // Make empire list
        Variant* pvEmpireKey, vTemp;
        iErrCode = g_pGameEngine->GetEmpiresInGame (iGameClass, iGameNumber, &pvEmpireKey, &iNumEmpiresInGame);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        Assert (iNumEmpiresInGame > 0);

        int i, iLoopGuard = iNumEmpiresInGame - 1;
        for (i = 0; i < iLoopGuard; i ++) {
            iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[i], &vTemp);
            if (iErrCode == OK) {
                strList += vTemp.GetCharPtr();
                strList += ", ";
            }
        }
        iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[i], &vTemp);
        if (iErrCode == OK) {
            strList += vTemp.GetCharPtr();
        }
        
        g_pGameEngine->FreeData (pvEmpireKey);
        
    } else {
        
        iErrCode = g_pGameEngine->GetNumEmpiresInGame (iGameClass, iGameNumber, &iNumEmpiresInGame);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        
        Assert (iNumEmpiresInGame > 0);
    }
    
    // Password
    bool bFlag;
    iErrCode = g_pGameEngine->IsGamePasswordProtected (iGameClass, iGameNumber, &bFlag);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    int iNumUpdates, iSecondsSince, iSecondsUntil, iState;
    iErrCode = g_pGameEngine->GetGameUpdateData (iGameClass, iGameNumber, &iSecondsSince, &iSecondsUntil, &iNumUpdates, &iState);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    UTCTime tCreationTime;
    iErrCode = g_pGameEngine->GetGameCreationTime (iGameClass, iGameNumber, &tCreationTime);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    char pszDateString [OS::MaxDateLength];
    Time::GetDateString (tCreationTime, pszDateString);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    int iNumUpdatesBeforeGameCloses;
    iErrCode = g_pGameEngine->GetNumUpdatesBeforeGameCloses (iGameClass, iGameNumber, &iNumUpdatesBeforeGameCloses);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    
    // Description
    OutputText (
        "<tr><td colspan=\"13\" align=\"center\">"\
        "<font size=\"-1\" face=\"" DEFAULT_GAMECLASS_DESCRIPTION_FONT "\"><br>"
        );
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iDescription].GetCharPtr());
    
    // Name
    OutputText ("</font></td></tr><tr><td><font size=\"3\">");
    
    iErrCode = g_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    m_pHttpResponse->WriteText (pszGameClassName);
    
    OutputText ("<strong> ");
    m_pHttpResponse->WriteText (iGameNumber);
    OutputText ("</strong></font>");
    
    // Time
    OutputText ("<p><font size=\"-1\">(Started <strong>");
    m_pHttpResponse->WriteText (pszDateString);
    OutputText ("</strong>)</font>");
    
    // Enter Game
    if (bSpectators) {

        OutputText ("<p>");

        sprintf (pszEnter, "Spectate%i.%i", iGameClass, iGameNumber);
        WriteButtonString (m_iButtonKey, ButtonName[BID_VIEWMAP], ButtonText[BID_VIEWMAP], pszEnter);

    } else {

        if (!bAdmin && m_iPrivilege >= NOVICE) {
            
            OutputText ("<p>");

            sprintf (pszEnter, "Enter%i.%i", iGameClass, iGameNumber);          
            WriteButtonString (m_iButtonKey, ButtonName[BID_ENTER], ButtonText[BID_ENTER], pszEnter);

            if (bFlag) {
                OutputText ("<br>Password: <input type=\"Password\" size=\"8\" maxlength=\"25\" name=\"Pass");
                m_pHttpResponse->WriteText (iGameClass);
                OutputText (".");
                m_pHttpResponse->WriteText (iGameNumber);
                OutputText ("\">");
            }
        }
    }
    
    OutputText ("</td>");
    
    // Missed Updates
    OutputText ("<td align=\"center\"><font size=\"2\"><strong>");
    
    if (iNumUpdates >= iNumUpdatesBeforeGameCloses || bSpectators) {

        m_pHttpResponse->WriteText (iNumUpdates);
        OutputText ("</strong> update");
        if (iNumUpdates != 1) {
            OutputText ("s");
        }

    } else {

        OutputText ("<font color=\"");

        if (iNumUpdates == 0) {
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        } else {
            m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        }
        
        OutputText ("\">");
        m_pHttpResponse->WriteText (iNumUpdates);
        OutputText ("</font></strong> of <strong><font color=\"");
        
        if (iNumUpdates == 0) {
            m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        } else {
            m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        }
        
        OutputText ("\">");
        m_pHttpResponse->WriteText (iNumUpdatesBeforeGameCloses);
        OutputText ("</font></strong> update");
        
        if (iNumUpdatesBeforeGameCloses != 1) {
            OutputText ("s");
        }

        OutputText (" to close</font>");
    }
    
    OutputText ("</td><td align=\"center\"><font size=\"2\"><font color=\"");
    
    // Next Update
    if (!(iState & STARTED)) {
        
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">Game has not started</font>");
        
    } else {
        
        m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
        OutputText ("\">");
        WriteTime (iSecondsUntil);
        
        if (iState & ADMIN_PAUSED) { 
            OutputText ("</font><br>(Paused by an administrator)");
        } else if (iState & PAUSED) { 
            OutputText ("</font> (paused)");
        } else {
            OutputText ("</font>");
        }
    }
    OutputText ("</font></td>");

    iErrCode = AddGameClassDescription (
        OPEN_GAME_LIST, 
        pvGameClassInfo, 
        iGameClass, 
        iGameNumber,
        iState,
        strList.GetCharPtr(), 
        iNumEmpiresInGame,
        bAdmin,
        bSpectators
        );

Cleanup:
    
    return iErrCode;
}


int HtmlRenderer::WriteSystemGameListData (int iGameClass, const Variant* pvGameClassInfo) {
    
    int iErrCode = OK;
    char pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH + 1];

    // Description
    OutputText (
        "<tr><td colspan=\"13\" align=\"center\">"\
        "<font size=\"-1\" face=\"" DEFAULT_GAMECLASS_DESCRIPTION_FONT "\"><br>"
        );
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iDescription].GetCharPtr());
    
    // Name
    OutputText ("</font></td></tr><tr><td><font size=\"3\">");
    
    iErrCode = g_pGameEngine->GetGameClassName (iGameClass, pszGameClassName);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    m_pHttpResponse->WriteText (pszGameClassName);
    
    // Game number, start button / halted
    char pszForm[80];
    
    OutputText (" <strong>");
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iOpenGameNum].GetInteger());
    OutputText ("</strong></font>");
    
    if (m_iPrivilege >= NOVICE) {
        
        if (!(pvGameClassInfo[SystemGameClassData::iOptions].GetInteger() & GAMECLASS_HALTED) &&
            !(pvGameClassInfo[SystemGameClassData::iOptions].GetInteger() & GAMECLASS_MARKED_FOR_DELETION) &&
            (pvGameClassInfo[SystemGameClassData::iMaxNumActiveGames].GetInteger() == INFINITE_ACTIVE_GAMES ||
            pvGameClassInfo[SystemGameClassData::iNumActiveGames].GetInteger() < 
            pvGameClassInfo[SystemGameClassData::iMaxNumActiveGames].GetInteger())) {
            
            OutputText ("<p>");
            
            sprintf (pszForm, "Start%i", iGameClass);
            WriteButtonString (m_iButtonKey, ButtonName[BID_START], ButtonText[BID_START], pszForm);
            
            // Password protection
            OutputText ("<br>Advanced:<input type=\"checkbox\" name=\"Advanced");
            m_pHttpResponse->WriteText (iGameClass);
            OutputText ("\">");
            
        } else {
            
            if ((pvGameClassInfo[SystemGameClassData::iOptions].GetInteger() & GAMECLASS_HALTED) != 0 &&
                (pvGameClassInfo[SystemGameClassData::iOptions].GetInteger() & GAMECLASS_MARKED_FOR_DELETION) != 0) {
                OutputText (" (<strong>Halted and marked for deletion</strong>)");
            }
            
            else if (pvGameClassInfo[SystemGameClassData::iOptions].GetInteger() & GAMECLASS_HALTED) {
                OutputText (" (<strong>Halted</strong>)");
            }
            
            else if (pvGameClassInfo[SystemGameClassData::iOptions].GetInteger() & GAMECLASS_MARKED_FOR_DELETION) {
                OutputText (" (<strong>Marked for deletion</strong>)");
            }
        }
        
        if (pvGameClassInfo[SystemGameClassData::iMaxNumActiveGames].GetInteger() != INFINITE_ACTIVE_GAMES) {
            
            if (pvGameClassInfo[SystemGameClassData::iNumActiveGames].GetInteger() >= 
                pvGameClassInfo[SystemGameClassData::iMaxNumActiveGames].GetInteger()) {
                
                OutputText ("<p>(Active game limit of <strong>");
                m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxNumActiveGames].GetInteger());
                OutputText ("</strong> reached)");
                
            } else {
                
                OutputText ("<p>(<strong>");
                m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iNumActiveGames].GetInteger());
                OutputText ("</strong> of <strong>");
                m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxNumActiveGames].GetInteger());
                OutputText ("</strong> active games)");
            }
        }
    }
    
    OutputText ("</td>");

    iErrCode = AddGameClassDescription (
        SYSTEM_GAME_LIST, pvGameClassInfo, NO_KEY, 0, 0, NULL, 0, false, false);

Cleanup:

    return iErrCode;
}


void HtmlRenderer::AddEmpiresInGame (int iGameState, int iNumActiveEmpires, const char* pszEmpires, 
                                     int iMinEmpires, int iMaxEmpires) {

    // Num empires
    OutputText ("<td align=\"center\"><font size=\"2\">");
    
    if (iNumActiveEmpires > 0) {

        OutputText ("<strong>");       
        m_pHttpResponse->WriteText (iNumActiveEmpires);
        OutputText ("</strong> empire");
        if (iNumActiveEmpires != 1) {
            OutputText ("s");
        }

        if (pszEmpires != NULL) {
            OutputText ("<br>(");
            m_pHttpResponse->WriteText (pszEmpires);
            OutputText (")");
        }

        OutputText ("<p>");
    }

    if (iGameState == 0) {

        if (iMinEmpires == iMaxEmpires) {

            OutputText ("<strong>");
            m_pHttpResponse->WriteText (iMinEmpires);
            OutputText ("</strong> empires");
        }

        else if (iMaxEmpires == UNLIMITED_EMPIRES) {

            OutputText ("<strong>");
            m_pHttpResponse->WriteText (iMinEmpires);
            OutputText ("</strong> to <strong>unlimited</strong> empires");
        }

        else {

            OutputText ("<strong>");
            m_pHttpResponse->WriteText (iMinEmpires);
            OutputText ("</strong> to <strong>");
            m_pHttpResponse->WriteText (iMaxEmpires);
            OutputText ("</strong> empires");
        }
    }
    else if ((iGameState & STILL_OPEN) && iNumActiveEmpires != iMaxEmpires) {

        if (iMinEmpires == iMaxEmpires) {

            OutputText ("<strong>");
            m_pHttpResponse->WriteText (iMinEmpires);
            OutputText ("</strong> required");
        }

        else if (iMaxEmpires == UNLIMITED_EMPIRES) {

            if (iNumActiveEmpires < iMinEmpires) {

                OutputText ("<strong>");
                m_pHttpResponse->WriteText (iMinEmpires);
                OutputText ("</strong> to start<br>");
            }

            OutputText ("Unlimited");
        }

        else {

            if (iNumActiveEmpires < iMinEmpires) {

                OutputText ("<strong>");
                m_pHttpResponse->WriteText (iMinEmpires);
                OutputText ("</strong> to start<br>");
            }

            OutputText ("<strong>");
            m_pHttpResponse->WriteText (iMaxEmpires);
            OutputText ("</strong> to close");
        }
    }

    OutputText ("</font></td>");
}

int HtmlRenderer::AddGameClassDescription (int iWhichList, const Variant* pvGameClassInfo, 
                                           int iGameClass, int iGameNumber, int iGameState, 
                                           const char* pszEmpiresInGame, 
                                           int iNumEmpiresInGame, bool bAdmin, bool bSpectators) {
    
    int iErrCode, iGameOptions = 0;
    GameFairnessOption gfoFairness = GAME_FAIRNESS_RANDOM;

    if (iWhichList == OPEN_GAME_LIST || iWhichList == ACTIVE_GAME_LIST) {

        Assert (iGameClass != NO_KEY);

        iErrCode = g_pGameEngine->GetGameOptions (iGameClass, iGameNumber, &iGameOptions);
        if (iErrCode != OK) {
            return iErrCode;
        }

        Variant vTemp;
        iErrCode = g_pGameEngine->GetGameProperty(iGameClass, iGameNumber, GameData::MapFairness, &vTemp);
        if (iErrCode != OK) {
            return iErrCode;
        }
        gfoFairness = (GameFairnessOption)vTemp.GetInteger();
    }

    // Time 
    OutputText ("<td align=\"center\"><font size=\"2\">");
    WriteTime (pvGameClassInfo[SystemGameClassData::iNumSecPerUpdate].GetInteger());
    
    if (iWhichList == OPEN_GAME_LIST) {
        
        Seconds sDelay;
        int iErrCode = g_pGameEngine->GetFirstUpdateDelay (iGameClass, iGameNumber, &sDelay);
        if (iErrCode == OK && sDelay > 0) {
            
            OutputText ("<p>(");
            WriteTime (sDelay);
            OutputText (" delay)");
        }
    }
    
    OutputText ("</font></td>");
    
    // Empires
    AddEmpiresInGame (
        iGameState,
        iNumEmpiresInGame, 
        pszEmpiresInGame, 
        pvGameClassInfo[SystemGameClassData::iMinNumEmpires].GetInteger(),
        pvGameClassInfo[SystemGameClassData::iMaxNumEmpires].GetInteger()
        );

    // Planets per empire
    OutputText ("<td align=\"center\"><font size=\"2\">");
    
    if (bAdmin && (iGameState & GAME_MAP_GENERATED)) {

        Variant vPlanets;
        iErrCode = g_pGameEngine->GetGameProperty(iGameClass, iGameNumber, GameData::NumPlanetsPerEmpire, &vPlanets);
        if (iErrCode == OK) {

            OutputText("<strong>");
            m_pHttpResponse->WriteText(vPlanets.GetInteger());
            OutputText("</strong> planet");
            if (pvGameClassInfo[SystemGameClassData::iMaxNumPlanets].GetInteger() != 1)
                OutputText ("s");
            OutputText(" per empire");
        }
    
    } else {
    
        OutputText("<strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMinNumPlanets].GetInteger());
        
        if (pvGameClassInfo[SystemGameClassData::iMaxNumPlanets].GetInteger() == 
            pvGameClassInfo[SystemGameClassData::iMinNumPlanets].GetInteger()) {
            
            OutputText ("</strong> planet");
            
            if (pvGameClassInfo[SystemGameClassData::iMaxNumPlanets].GetInteger() == 1) {
                OutputText (" per empire");
            } else {
                OutputText ("s per empire");
            }
            
        } else {
            
            OutputText("</strong> to <strong>");
            m_pHttpResponse->WriteText(pvGameClassInfo[SystemGameClassData::iMaxNumPlanets].GetInteger());
            OutputText("</strong> planets per empire");
        }
    }

    if (iWhichList == OPEN_GAME_LIST || iWhichList == ACTIVE_GAME_LIST) {

        if (iGameOptions & GAME_MIRRORED_MAP) {
            OutputText ("<p>Mirrored map");
        }
        else if (iGameOptions & GAME_TWISTED_MAP) {
            OutputText ("<p>Twisted map");
        }

        switch (gfoFairness) {
        case GAME_FAIRNESS_RANDOM:
            OutputText("<p>Random map");
            break;
        case GAME_FAIRNESS_VERY_FAIR:
            OutputText("<p>Very fair map");
            break;
        case GAME_FAIRNESS_SOMEWHAT_FAIR:
            OutputText("<p>Somewhat fair map");
            break;
        case GAME_FAIRNESS_SOMEWHAT_UNFAIR:
            OutputText("<p>Somewhat unfair map");
            break;
        case GAME_FAIRNESS_VERY_UNFAIR:
            OutputText("<p>Very unfair map");
            break;
        }

        if (bAdmin && (iGameState & GAME_MAP_GENERATED) && gfoFairness != GAME_FAIRNESS_RANDOM) {

            Variant vDev;
            iErrCode = g_pGameEngine->GetGameProperty(iGameClass, iGameNumber,
                                                      GameData::MapFairnessStandardDeviationPercentageOfMean,
                                                      &vDev);
            if (iErrCode == OK) {

                OutputText("<br>(<strong>");
                m_pHttpResponse->WriteText(vDev.GetInteger());
                OutputText("</strong>% deviation from fairness)");
            }
        }
    }

    if (pvGameClassInfo[SystemGameClassData::iOptions].GetInteger() & GENERATE_MAP_FIRST_UPDATE) {
        OutputText ("<p>Map on first update");
    }

    // Tech
    OutputText ("</font></td><td align=\"center\"><font size=\"2\"><strong>");
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iInitialTechLevel].GetFloat());
    OutputText ("</strong> initial<br><strong>");
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxTechDev].GetFloat());
    OutputText ("</strong> delta");

    if (pvGameClassInfo[SystemGameClassData::iInitialTechDevs].GetInteger() != ALL_TECHS) {    

        OutputText ("<br><strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iNumInitialTechDevs].GetInteger());
        OutputText ("</strong> tech dev");

        if (pvGameClassInfo[SystemGameClassData::iNumInitialTechDevs].GetInteger() != 1) {
            OutputText ("s");
        }
    }
    OutputText ("</font></td>");
    
    // Dip
    AddDiplomacy (pvGameClassInfo);

    // Resources
    AddResources (pvGameClassInfo);

    // Init techs
    AddTechList (
        pvGameClassInfo[SystemGameClassData::iInitialTechDevs].GetInteger(),
        pvGameClassInfo[SystemGameClassData::iNumInitialTechDevs].GetInteger()
        );

    // Dev techs
    AddTechList (pvGameClassInfo[SystemGameClassData::iDevelopableTechDevs].GetInteger(), 0);

    // Bridier, Score
    if (iWhichList == OPEN_GAME_LIST) {

        int iOptions;
        
        Variant pvMin [NUM_ENTRY_SCORE_RESTRICTIONS], pvMax [NUM_ENTRY_SCORE_RESTRICTIONS];
        
        int iErrCode = g_pGameEngine->GetGameEntryRestrictions (
            iGameClass, 
            iGameNumber, 
            &iOptions, 
            pvMin, 
            pvMax
            );
        
        if (iErrCode != OK) {

            OutputText ("<td>Error</td><td>Error</td><td>Error</td>");
        
        } else {
            
            AddBridier (iGameClass, iGameNumber, pvGameClassInfo, iOptions, pvMin, pvMax, true);
            AddScore (iOptions, pvMin, pvMax);
            AddSecurity (iOptions);
        }
    }
    
    // Options
    AddOptions (iWhichList, pvGameClassInfo, iGameOptions);
    
    return OK;
}

void HtmlRenderer::AddBridierGame (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, 
                                   int iGameOptions, bool bDisplayGainLoss) {

    if (pvGameClassInfo[SystemGameClassData::iMaxNumEmpires].GetInteger() == 2 &&
        iGameOptions & GAME_COUNT_FOR_BRIDIER) {
        
        OutputText ("<p><font color=\"#");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\"><strong>Bridier Game</strong>");
        
        if (bDisplayGainLoss) {
            
            int iErrCode, iGain, iLoss;
            
            iErrCode = g_pGameEngine->GetBridierRankPotentialGainLoss (
                iGameClass, 
                iGameNumber, 
                m_iEmpireKey, 
                &iGain, 
                &iLoss
                );
            
            if (iErrCode == OK && iGain >= 0 && iLoss >= 0) {

                if (iLoss > iGain) {

                    OutputText ("</font><font color=\"#");
                    m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
                    OutputText ("\">");
                }

                OutputText ("<br>Gain: <strong>");
                m_pHttpResponse->WriteText (iGain);
                OutputText ("</strong><br>Loss: <strong>");
                m_pHttpResponse->WriteText (iLoss);
                OutputText ("</strong>");
            }
        }

        OutputText ("</font>");
    }
}

void HtmlRenderer::AddOptions (int iWhichList, const Variant* pvGameClassInfo, int iGameOptions) {
    
    int iGameClassOptions = pvGameClassInfo[SystemGameClassData::iOptions].GetInteger();
    
    // Weekend
    m_pHttpResponse->WriteText (
        "</td><td align=\"center\" width=\"160\"><table><tr><td align=\"center\"><font size=\"2\">"
        );

    // Spectators
    if (iWhichList != SYSTEM_GAME_LIST) {
        
        if (iGameOptions & GAME_ALLOW_SPECTATORS) {
            OutputText ("Spectators<br>");
        } else {
            OutputText ("<strike>Spectators</strike><br>");
        }
    }       
    
    // Visible builds
    if (iGameClassOptions & VISIBLE_BUILDS) {
        OutputText ("VisibleBuilds<br>");
    } else {
        OutputText ("<strike>VisibleBuilds</strike><br>");
    }
    
    // Visible diplomacy
    if (iGameClassOptions & VISIBLE_DIPLOMACY) {
        OutputText ("VisibleDip");
    } else {
        OutputText ("<strike>VisibleDip</strike>");
    }
    
    // Pop to build
    OutputText ("<br><strong>");
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iBuilderPopLevel].GetInteger());
    OutputText ("</strong> BuildPop");

    // MaxShips
    if (pvGameClassInfo[SystemGameClassData::iMaxNumShips].GetInteger() == INFINITE_SHIPS) {
        OutputText ("<br><strike>MaxShips</strike>");
    } else {
        OutputText ("<br><strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxNumShips].GetInteger());
        OutputText ("</strong> MaxShips");
    }
    
    // Max ag ratio
    OutputText ("<br><strong>");
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxAgRatio].GetFloat());
    OutputText ("</strong> MaxAgRatio");

    // Ship flags
    if (iGameClassOptions & USE_FRIENDLY_GATES) {
        OutputText ("<br>FriendlyGates");
    } else {
        OutputText ("<br><strike>FriendlyGates</strike>");
    }

    if (iGameClassOptions & DISABLE_SCIENCE_SHIPS_NUKING) {
        OutputText ("<br>FriendlyScis");
    } else {
        OutputText ("<br><strike>FriendlyScis</strike>");
    }

    if (!(iGameClassOptions & DISABLE_SUICIDAL_DOOMSDAYS)) {
        OutputText ("<br>SuicidalDooms");
    } else {
        OutputText ("<br><strike>SuicidalDooms</strike>");
    }

    if (!(iGameClassOptions & USE_UNFRIENDLY_DOOMSDAYS)) {
        OutputText ("<br>FriendlyDooms");
    } else {
        OutputText ("<br><strike>FriendlyDooms</strike>");
    }
    
    if (iGameClassOptions & USE_CLASSIC_DOOMSDAYS) {
        OutputText ("<br>PermanentDooms");
    } else {
        OutputText ("<br><strike>PermanentDooms</strike>");
    }

    OutputText ("</td><td align=\"center\"><font size=\"2\">");

    // Weekend
    if (iGameClassOptions & WEEKEND_UPDATES) {
        OutputText ("Weekend<br>");
    } else {
        OutputText ("<strike>Weekend</strike></br>");
    }

    // Private messages
    if (iGameClassOptions & PRIVATE_MESSAGES) {
        OutputText ("PrivateMsg<br>");
    } else {
        OutputText ("<strike>PrivateMsg</strike><br>");
    }

    // Dip exposed
    if (iGameClassOptions & EXPOSED_DIPLOMACY) {
        OutputText ("DipExposed<br>");
    } else {
        OutputText ("<strike>DipExposed</strike><br>");
    }
    
    // Map exposed
    if (iGameClassOptions & EXPOSED_MAP) {
        OutputText ("MapExposed<br>");
    } else {
        OutputText ("<strike>MapExposed</strike><br>");
    }
    
    // Map shared
    if (pvGameClassInfo[SystemGameClassData::iMapsShared] != NO_DIPLOMACY) {
        OutputText ("MapShared (at <strong>");
        m_pHttpResponse->WriteText (DIP_STRING (pvGameClassInfo[SystemGameClassData::iMapsShared].GetInteger()));
        OutputText ("</strong>)");
    } else {
        OutputText ("<strike>MapShared</strike>");
    }
    
    // Fully colonized
    if (iGameClassOptions & FULLY_COLONIZED_MAP) {
        OutputText ("<br>FullCol");
    } else {
        OutputText ("<br><strike>FullCol</strike>");
    }
    
    // Disconnected maps
    if (iGameClassOptions & DISCONNECTED_MAP) {
        OutputText ("<br>Disconnected");
    } else {
        OutputText ("<br><strike>Disconnected</strike>");
    }

    // Independence
    if (iGameClassOptions & INDEPENDENCE) {
        OutputText ("<br>Independence");
    } else {
        OutputText ("<br><strike>Independence</strike>");
    }
    
    // Subjective views
    if (iGameClassOptions & SUBJECTIVE_VIEWS) {
        OutputText ("<br>Subjective");
    } else {
        OutputText ("<br><strike>Subjective</strike>");
    }

    // Updates for idle
    OutputText ("<br><strong>");
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iNumUpdatesForIdle].GetInteger());
    OutputText ("</strong> IdleUpdate");
    
    if (pvGameClassInfo[SystemGameClassData::iNumUpdatesForIdle].GetInteger() > 1) {
        OutputText ("s");
    }
    
    // Ruins
    switch (pvGameClassInfo[SystemGameClassData::iRuinFlags].GetInteger()) {
    case 0:
        
        OutputText ("<br><strike>Ruins</strike>");
        break;
        
    case RUIN_CLASSIC_SC:
        
        OutputText ("<br>SimpleRuins<br>(<strong>");
        break;
        
    case RUIN_ALMONASTER:
        
        OutputText ("<br>ComplexRuins<br>(<strong>");
        break;
        
    default:
        
        Assert (false);
        break;
    }
    
    if (pvGameClassInfo[SystemGameClassData::iRuinFlags].GetInteger() != 0) {
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iNumUpdatesForRuin].GetInteger());
        OutputText ("</strong> updates)");
    }
    
    OutputText ("</td></tr></table></td>");
}

void HtmlRenderer::AddResources (const Variant* pvGameClassInfo) {
    
    OutputText ("<td align=\"center\" width=\"14%\"><font size=\"2\"><strong>");
    
    //
    // Planet Res
    //
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMinAvgAg].GetInteger());
    if (pvGameClassInfo[SystemGameClassData::iMaxAvgAg].GetInteger() != 
        pvGameClassInfo[SystemGameClassData::iMinAvgAg].GetInteger()) {
        
        OutputText ("</strong>-<strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxAvgAg].GetInteger());
    }
    OutputText ("</strong> Ag<br><strong>");
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMinAvgMin].GetInteger());
    if (pvGameClassInfo[SystemGameClassData::iMaxAvgMin].GetInteger() != 
        pvGameClassInfo[SystemGameClassData::iMinAvgMin].GetInteger()) {
        
        OutputText ("</strong>-<strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxAvgMin].GetInteger());
    }
    OutputText ("</strong> Min<br><strong>");
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMinAvgFuel].GetInteger());
    if (pvGameClassInfo[SystemGameClassData::iMaxAvgFuel].GetInteger() != 
        pvGameClassInfo[SystemGameClassData::iMinAvgFuel].GetInteger()) {
        
        OutputText ("</strong>-<strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxAvgFuel].GetInteger());
    }
    OutputText ("</strong> Fuel<br><strong>");
    
    //
    // HW
    //
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMinAgHW].GetInteger());
    if (pvGameClassInfo[SystemGameClassData::iMaxAgHW].GetInteger() != 
        pvGameClassInfo[SystemGameClassData::iMinAgHW].GetInteger()) {
        
        OutputText ("</strong>-<strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxAgHW].GetInteger());
    }
    OutputText ("</strong> HWAg<br><strong>");
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMinMinHW].GetInteger());
    if (pvGameClassInfo[SystemGameClassData::iMaxMinHW].GetInteger() != 
        pvGameClassInfo[SystemGameClassData::iMinMinHW].GetInteger()) {
        
        OutputText ("</strong>-<strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxMinHW].GetInteger());
    }
    OutputText ("</strong> HWMin<br><strong>");
    
    m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMinFuelHW].GetInteger());
    if (pvGameClassInfo[SystemGameClassData::iMaxFuelHW].GetInteger() != 
        pvGameClassInfo[SystemGameClassData::iMinFuelHW].GetInteger()) {
        
        OutputText ("</strong>-<strong>");
        m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxFuelHW].GetInteger());
    }
    OutputText ("</strong> HWFuel");

    OutputText ("</td>");
}

void HtmlRenderer::AddDiplomacy (const Variant* pvGameClassInfo) {
    
    int iOptions = pvGameClassInfo[SystemGameClassData::iOptions].GetInteger();
    
    OutputText ("<td align=\"center\" width=\"75\"><font size=\"2\"><strong>" WAR_STRING "</strong>");
    
    int iDip = pvGameClassInfo[SystemGameClassData::iDiplomacyLevel].GetInteger();
    
    if (g_pGameEngine->GameAllowsDiplomacy (iDip, TRUCE)) {
        OutputText ("<br><strong>" TRUCE_STRING "</strong>");
        
        switch (pvGameClassInfo[SystemGameClassData::iMaxNumTruces].GetInteger()) {
            
        case UNRESTRICTED_DIPLOMACY:
            break;      
            
        case FAIR_DIPLOMACY:
            
            OutputText (" (Fair)");
            break;
            
        default:
            
            OutputText (" (<strong>");
            m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxNumTruces].GetInteger());
            OutputText ("</strong>)");
            break;
        }
    }
    
    if (g_pGameEngine->GameAllowsDiplomacy (iDip, TRADE)) {
        OutputText ("<br><strong>" TRADE_STRING "</strong>");
        
        switch (pvGameClassInfo[SystemGameClassData::iMaxNumTrades].GetInteger()) {
            
        case UNRESTRICTED_DIPLOMACY:
            break;      
            
        case FAIR_DIPLOMACY:
            
            OutputText (" (Fair)");
            break;
            
        default:
            
            OutputText (" (<strong>");
            m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::iMaxNumTrades].GetInteger());
            OutputText ("</strong>)");
            break;
        }
    }
    
    if (g_pGameEngine->GameAllowsDiplomacy (iDip, ALLIANCE)) {
        OutputText ("<br><strong>" ALLIANCE_STRING "</strong>");
        
        bool bOpened = true;
        int iMax = pvGameClassInfo[SystemGameClassData::iMaxNumAlliances].GetInteger();
        
        switch (iMax) {
            
        case UNRESTRICTED_DIPLOMACY:
            bOpened = false;
            break;      
            
        case FAIR_DIPLOMACY:
            
            OutputText (" (Fair");
            break;
            
        default:
            
            OutputText (" (<strong>");
            m_pHttpResponse->WriteText (iMax);
            OutputText ("</strong>");
            break;
        }
        
        if (iOptions & UNBREAKABLE_ALLIANCES) {
            
            if (iMax == UNRESTRICTED_DIPLOMACY) {
                bOpened = true;
                OutputText (" (");
            } else {
                OutputText (", ");
            }
            
            OutputText ("unbreakable");
        }
        
        if (iOptions & PERMANENT_ALLIANCES) {
            
            if (!bOpened) {
                bOpened = true;
                OutputText (" (");
            } else {
                OutputText (", ");
            }
            
            OutputText ("entire game");
        }
        
        if (bOpened) {
            OutputText (")");
        }
    }
    
    if (g_pGameEngine->GameAllowsDiplomacy (iDip, SURRENDER) || (iOptions & USE_SC30_SURRENDERS)) {
        
        OutputText ("<br><strong>" SURRENDER_STRING "</strong>");
        
        if (iOptions & ONLY_SURRENDER_WITH_TWO_EMPIRES) {
            OutputText (" (<strong>2</strong> empires)");
        }
        
        else if (iOptions & USE_SC30_SURRENDERS) {
            OutputText (" (Classic)");
        }
    }

    if (iOptions & ALLOW_DRAW) {
        OutputText ("<br><strong>Draw</strong>");
    }
    
    OutputText ("</font></td>");
}

void HtmlRenderer::AddTechList (int iTechs, int iInitial) {

    OutputText ("<td align=\"center\"><font size=\"2\">");
    
    int i, iNumTechs = 0;
    String strTechList;
    
    if (iTechs == 0) {
        OutputText ("None");
    }
    
    else if (iTechs == ALL_TECHS) {
        OutputText ("All Techs");
    }
    
    else {
        
        bool bNewTechs = false;
        
        if ((iTechs & ALL_CLASSIC_TECHS) == ALL_CLASSIC_TECHS) {
            strTechList = "All Classic Techs";
            iTechs &= ~ALL_CLASSIC_TECHS;
            iNumTechs += NUM_CLASSIC_TECHS;
        }
        
        else if ((iTechs & ALL_NEW_TECHS) == ALL_NEW_TECHS) {
            iTechs &= ~ALL_NEW_TECHS;
            bNewTechs = true;
        }
        
        ENUMERATE_TECHS (i) {
            
            if (iTechs & TECH_BITS[i]) {
                
                if (iNumTechs > 0) {
                    strTechList += ", ";
                }
                strTechList += SHIP_TYPE_STRING[i];
                iNumTechs ++;
            }
        }
        
        if (bNewTechs) {
            if (iNumTechs > 0) {
                strTechList += ", ";
            }
            strTechList += "All New Techs";
        }
        
        m_pHttpResponse->WriteText (strTechList.GetCharPtr(), strTechList.GetLength());
    }

    OutputText ("</font></td>");
}

#define OUTPUT_TEXT_SEPARATOR OutputText ("<br>")

void HtmlRenderer::AddBridier (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, 
                               int iGameOptions, const Variant* pvMin, const Variant* pvMax, 
                               bool bDisplayGainLoss) {
        
    bool bText = false;
    
    // Bridier
    OutputText ("<td align=\"center\"><font size=\"2\">");
    
    if (iGameOptions & GAME_COUNT_FOR_BRIDIER) {
        
        // Bridier Rank Gain
        if ((iGameOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN | GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN)) == 
            (GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN | GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN)) {
            
            if (bText) {
                OUTPUT_TEXT_SEPARATOR;
            } else {
                bText = true;
            }
            
            OutputText ("<strong>");
            m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_RANK_GAIN].GetInteger());
            OutputText ("</strong> to <strong>");
            m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_RANK_GAIN].GetInteger());
            OutputText ("</strong> Rank Gain");
        }
        
        else if (iGameOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN) {
            
            if (bText) {
                OUTPUT_TEXT_SEPARATOR;
            } else {
                bText = true;
            }
            
            OutputText ("At least <strong>");
            m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_RANK_GAIN].GetInteger());
            OutputText ("</strong> Rank Gain");
        }
        
        else if (iGameOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN) {
            
            if (bText) {
                OUTPUT_TEXT_SEPARATOR;
            } else {
                bText = true;
            }
            
            OutputText ("At most <strong>");
            m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_RANK_GAIN].GetInteger());
            OutputText ("</strong> Rank Gain");
        }

        // Bridier Rank Loss
        if ((iGameOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS | GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS)) == 
            (GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS | GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS)) {
            
            if (bText) {
                OUTPUT_TEXT_SEPARATOR;
            } else {
                bText = true;
            }
            
            OutputText ("<strong>");
            m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_RANK_LOSS].GetInteger());
            OutputText ("</strong> to <strong>");
            m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_RANK_LOSS].GetInteger());
            OutputText ("</strong> Rank Loss");
        }
        
        else if (iGameOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS) {
            
            if (bText) {
                OUTPUT_TEXT_SEPARATOR;
            } else {
                bText = true;
            }
            
            OutputText ("At least <strong>");
            m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_RANK_LOSS].GetInteger());
            OutputText ("</strong> Rank Loss");
        }
        
        else if (iGameOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS) {
            
            if (bText) {
                OUTPUT_TEXT_SEPARATOR;
            } else {
                bText = true;
            }
            
            OutputText ("At most <strong>");
            m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_RANK_LOSS].GetInteger());
            OutputText ("</strong> Rank Loss");
        }
        
        if (!bText) {
            OutputText ("Any empire");
        }
        
    } else {
        
        OutputText ("N/A");
    }

    AddBridierGame (iGameClass, iGameNumber, pvGameClassInfo, iGameOptions, bDisplayGainLoss);
        
    OutputText ("</font></td>");
}

void HtmlRenderer::AddScore (int iOptions, const Variant* pvMin, const Variant* pvMax) {
    
    bool bText = false;

    OutputText ("<td align=\"center\"><font size=\"2\">");
    
    // Almonaster
    if ((iOptions & (GAME_RESTRICT_MIN_ALMONASTER_SCORE | GAME_RESTRICT_MAX_ALMONASTER_SCORE)) == 
        (GAME_RESTRICT_MIN_ALMONASTER_SCORE | GAME_RESTRICT_MAX_ALMONASTER_SCORE)) {
        
        bText = true;
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_ALMONASTER_SCORE].GetFloat());
        OutputText ("</strong> to <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_ALMONASTER_SCORE].GetFloat());
        OutputText ("</strong> Almonaster");
    }
    
    else if (iOptions & GAME_RESTRICT_MIN_ALMONASTER_SCORE) {
        
        bText = true;
        
        OutputText ("At least <strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_ALMONASTER_SCORE].GetFloat());
        OutputText ("</strong> Almonaster");
    }
    
    else if (iOptions & GAME_RESTRICT_MAX_ALMONASTER_SCORE) {
        
        bText = true;
        
        OutputText ("At most <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_ALMONASTER_SCORE].GetFloat());
        OutputText ("</strong> Almonaster");
    }
    
    // Classic
    if ((iOptions & (GAME_RESTRICT_MIN_CLASSIC_SCORE | GAME_RESTRICT_MAX_CLASSIC_SCORE)) == 
        (GAME_RESTRICT_MIN_CLASSIC_SCORE | GAME_RESTRICT_MAX_CLASSIC_SCORE)) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_CLASSIC_SCORE].GetFloat());
        OutputText ("</strong> to <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_CLASSIC_SCORE].GetFloat());
        OutputText ("</strong> Classic");
    }
    
    else if (iOptions & GAME_RESTRICT_MIN_CLASSIC_SCORE) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("At least <strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_CLASSIC_SCORE].GetFloat());
        OutputText ("</strong> Classic");
    }
    
    else if (iOptions & GAME_RESTRICT_MAX_CLASSIC_SCORE) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("At most <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_CLASSIC_SCORE].GetFloat());
        OutputText ("</strong> Classic");
    }

    // Bridier Rank
    if ((iOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK | GAME_RESTRICT_MAX_BRIDIER_RANK)) == 
        (GAME_RESTRICT_MIN_BRIDIER_RANK | GAME_RESTRICT_MAX_BRIDIER_RANK)) {
        
        bText = true;
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_RANK].GetInteger());
        OutputText ("</strong> to <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_RANK].GetInteger());
        OutputText ("</strong> Rank");
    }
    
    else if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK) {
        
        bText = true;
        
        OutputText ("At least <strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_RANK].GetInteger());
        OutputText ("</strong> Rank");
    }
    
    else if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK) {
        
        bText = true;
        
        OutputText ("At most <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_RANK].GetInteger());
        OutputText ("</strong> Rank");
    }
    
    // Bridier Index
    if ((iOptions & (GAME_RESTRICT_MIN_BRIDIER_INDEX | GAME_RESTRICT_MAX_BRIDIER_INDEX)) == 
        (GAME_RESTRICT_MIN_BRIDIER_INDEX | GAME_RESTRICT_MAX_BRIDIER_INDEX)) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_INDEX].GetInteger());
        OutputText ("</strong> to <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_INDEX].GetInteger());
        OutputText ("</strong> Index");
    }
    
    else if (iOptions & GAME_RESTRICT_MIN_BRIDIER_INDEX) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("At least <strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_INDEX].GetInteger());
        OutputText ("</strong> Index");
    }
    
    else if (iOptions & GAME_RESTRICT_MAX_BRIDIER_INDEX) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("At most <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_INDEX].GetInteger());
        OutputText ("</strong> Index");
    }
    
    // Wins
    if ((iOptions & (GAME_RESTRICT_MIN_WINS | GAME_RESTRICT_MAX_WINS)) == 
        (GAME_RESTRICT_MIN_WINS | GAME_RESTRICT_MAX_WINS)) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("<strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_WINS].GetInteger());
        OutputText ("</strong> to <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_WINS].GetInteger());
        OutputText ("</strong> Wins");
    }
    
    else if (iOptions & GAME_RESTRICT_MIN_WINS) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("At least <strong>");
        m_pHttpResponse->WriteText (pvMin [RESTRICT_WINS].GetInteger());
        OutputText ("</strong> Wins");
    }
    
    else if (iOptions & GAME_RESTRICT_MAX_WINS) {
        
        if (bText) {
            OUTPUT_TEXT_SEPARATOR;
        } else {
            bText = true;
        }
        
        OutputText ("At most <strong>");
        m_pHttpResponse->WriteText (pvMax [RESTRICT_WINS].GetInteger());
        OutputText ("</strong> Wins");
    }
    
    if (!bText) {
        OutputText ("Any empire");
    }

    OutputText ("</font></td>");
}

#undef OUTPUT_TEXT_SEPARATOR

void HtmlRenderer::AddSecurity (int iOptions) {
    
    bool bWarn, bBlock, bDisplayed = false;

    OutputText ("<td align=\"center\"><font size=\"2\">");
    
    bBlock = (iOptions & GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS) != 0;
    bWarn = (iOptions & GAME_WARN_ON_DUPLICATE_IP_ADDRESS) != 0;
    
    if (bWarn || bBlock) {
        
        bDisplayed = true;
        
        if (bWarn && !bBlock) {
            OutputText ("<strong>Warn</strong> on duplicate IP address");
        }
        else if (!bWarn && bBlock) {
            OutputText ("<strong>Reject</strong> on duplicate IP address");
        }
        else if (bWarn && bBlock) {
            OutputText ("<strong>Warn</strong> and <strong>reject</strong> on duplicate IP address");
        }
    }
    
    bBlock = (iOptions & GAME_BLOCK_ON_DUPLICATE_SESSION_ID) != 0;
    bWarn = (iOptions & GAME_WARN_ON_DUPLICATE_SESSION_ID) != 0;
    
    if (bWarn || bBlock) {
        
        if (bDisplayed) {
            OutputText ("<br>");
        } else {
            bDisplayed = true;
        }
        
        if (bWarn && !bBlock) {
            OutputText ("<strong>Warn</strong> on duplicate Session Id");
        }
        else if (!bWarn && bBlock) {
            OutputText ("<strong>Reject</strong> on duplicate SessionId");
        }
        else if (bWarn && bBlock) {
            OutputText ("<strong>Warn</strong> and <strong>reject</strong> on duplicate Session Id");
        }
    }

    if (iOptions & GAME_RESTRICT_IDLE_EMPIRES) {

        if (bDisplayed) {
            OutputText ("<br>");
        } else {
            bDisplayed = true;
        }

        OutputText ("<strong>No idlers</strong>");
    }
    
    if (!bDisplayed) {
        OutputText ("None");
    }
    
    OutputText ("</font></td>");
}