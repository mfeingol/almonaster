//
// Almonaster.dll:  a component of Almonaster 2.0
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

const char* ButtonText[] = {
    NULL,
    "Active Game List",
    "Administer Game",
    "Advanced Search",
    "All",
    "Almonaster Score",
    "Attack",
    "Backup",
    "Blank Empire Statistics",
    "Build",
    "Cancel",
    "Cancel All Builds",
    "Change Empire's Password",
    "Change Password",
    "Chatroom",
    "Choose",
    "Classic Score",
    "Cloaker",
    "Colony",
    "Create Alien Icon",
    "Create Empire",
    "Create New GameClass",
    "Create New SuperClass",
    "Create New Theme",
    "Delete Alien Icon",
    "Delete Backup",
    "Delete Empire",
    "Delete GameClass",
    "Delete SuperClass",
    "Delete Theme",
    "Diplomacy",
    "Doomsday",
    "Empire",
    "Empire Administrator",
    "End Turn",
    "Engineer",
    "Enter",
    "Exit",
    "Documentation",
    "Flush",
    "Force an Update",
    "Game Administrator",
    "Halt GameClass",
    "Info",
    "Kill Game",
    "Leave the Chatroom",
    "Login",
    "Map",
    "Minefield",
    "Minesweeper",
    "Obliterate Empire",
    "Open Game List",
    "Options",
    "Pause All Games",
    "Pause Game",
    "Personal GameClasses",
    "Planets",
    "Profile Editor",
    "Profile Viewer",
    "Purge",
    "Quit",
    "Refresh Messages",
    "Restart Almonaster",
    "Restart Server",
    "Restore Backup",
    "Restore Empire",
    "Satellite",
    "Science",
    "Search",
    "Selection",
    "Send Message",
    "Server Administrator",
    "Server News",
    "Server Information",
    "Ships",
    "Shutdown Server",
    "Speak",
    "Stargate",
    "Start",
    "System",
    "System Game List",
    "Tech",
    "Terraformer",
    "Theme Administrator",
    "Top Lists",
    "Troopship",
    "Undelete Empire",
    "Undelete GameClass",
    "Unend Turn",
    "Unhalt GameClass",
    "Unpause All Games",
    "Unpause Game",
    "View Empire's GameClasses",
    "View Empire's Nuke History",
    "View Game Information",
    "View Map",
    "View Messages",
    "View Profile",
    "Search IP Addresses",
    "Search Session IDs",
    "View Empire Information",
    "Resign",
    "Start",
    "Rename SuperClass",
    "Clear Messages",
    "Surrender",
    "Carrier",
    "Builder",
    "Morpher",
    "Jumpgate",
    "Latest Nukes",
    "Bridier Score",
    "Established Bridier Score",
    "Spectator Games",
    "Block",
    "Reset",
    "Contributions",
    "Credits",
    "Latest Games",
    "Tournament Administrator",
    "Personal Tournaments",
    "Create New Tournament",
    "Delete Tournament",
    "Administer Tournament",
    "Invite Empire",
    "Create Team",
    "View Tournament Information",
    "Decline",
    "Accept",
    "Administer Team",
    "Delete Team",
    "Add Empire",
    "Update",
    "Choose",
    "Choose",
    "Tournaments",
    "Join",
    "View Empire's Tournaments",
    "Rebuild",
    "Lookup",
    "View Mini-Map",
    "Build",
    "+",
    "-",
    "TOS",
    "Accept",
    "Decline",
    NULL
};

const char* ButtonName[] = {
    NULL,
    "ActiveGameList",
    "AdministerGame",
    "AdvancedSearch",
    "All",
    "AlmonasterScore",
    "Attack",
    "Backup",
    "BlankEmpireStatistics",
    "Build",
    "Cancel",
    "CancelAllBuilds",
    "ChangeEmpiresPassword",
    "ChangePassword",
    "Chatroom",
    "Choose",
    "ClassicScore",
    "Cloaker",
    "Colony",
    "CreateAlienIcon",
    "CreateEmpire",
    "CreateNewGameClass",
    "CreateNewSuperClass",
    "CreateNewTheme",
    "DeleteAlienIcon",
    "DeleteBackup",
    "DeleteEmpire",
    "DeleteGameClass",
    "DeleteSuperClass",
    "DeleteTheme",
    "Diplomacy",
    "Doomsday",
    "Empire",
    "EmpireAdministrator",
    "EndTurn",
    "Engineer",
    "Enter",
    "Exit",
    "Documentation",
    "Flush",
    "ForceUpdate",
    "GameAdministrator",
    "HaltGameClass",
    "Info",
    "KillGame",
    "LeaveTheChatroom",
    "BLogin",
    "Map",
    "Minefield",
    "Minesweeper",
    "ObliterateEmpire",
    "OpenGameList",
    "Options",
    "PauseAllGames",
    "PauseGame",
    "PersonalGameClasses",
    "Planets",
    "ProfileEditor",
    "ProfileViewer",
    "Purge",
    "Quit",
    "RefreshMessages",
    "RestartAlmonaster",
    "RestartServer",
    "RestoreBackup",
    "RestoreEmpire",
    "Satellite",
    "Science",
    "Search",
    "Selection",
    "SendMessage",
    "ServerAdministrator",
    "ServerNews",
    "ServerInformation",
    "Ships",
    "ShutdownServer",
    "Speak",
    "Stargate",
    "Start",
    "System",
    "SystemGameList",
    "Tech",
    "Terraformer",
    "ThemeAdministrator",
    "TopLists",
    "Troopship",
    "UndeleteEmpire",
    "UndeleteGameClass",
    "UnendTurn",
    "UnhaltGameClass",
    "UnpauseAllGames",
    "UnpauseGame",
    "ViewEmpiresGameClasses",
    "ViewEmpiresNukeHistory",
    "ViewGameInformation",
    "ViewMap",
    "ViewMessages",
    "ViewProfile",
    "SearchIPs",
    "SearchIDs",
    "ViewInfo",
    "Resign",
    "StartBYOG",
    "RenameSuperClass",
    "ClearMessages",
    "Surrender",
    "Carrier",
    "Builder",
    "Morpher",
    "Jumpgate",
    "LatestNukes",
    "BridierScore",
    "EstablishedBridierScore",
    "SpectatorGames",
    "Block",
    "Reset",
    "Contributions",
    "Credits",
    "LatestGames",
    "TournamentAdministrator",
    "PersonalTournaments",
    "CreateNewTournament",
    "DeleteTournament",
    "AdministerTournament",
    "InviteEmpire",
    "CreateTeam",
    "ViewTournamentInformation",
    "Decline",
    "Accept",
    "AdministerTeam",
    "DeleteTeam",
    "AddEmpire",
    "Update",
    "ChooseIcon",
    "ChooseTheme",
    "Tournaments",
    "Join",
    "ViewEmpiresTournaments",
    "Rebuild",
    "Lookup",
    "ViewMiniMap",
    "MiniBuild",
    "Plus",
    "Minus",
    "TOS",
    "TOSAccept",
    "TOSDecline",
    NULL
};

const char* ButtonImageName[] = {
    NULL,
    "ActiveGameList.x",
    "AdministerGame.x",
    "AdvancedSearch.x",
    "All.x",
    "AlmonasterScore.x",
    "Attack.x",
    "Backup.x",
    "BlankEmpireStatistics.x",
    "Build.x",
    "Cancel.x",
    "CancelAllBuilds.x",
    "ChangeEmpiresPassword.x",
    "ChangePassword.x",
    "Chatroom.x",
    "Choose.x",
    "ClassicScore.x",
    "Cloaker.x",
    "Colony.x",
    "CreateAlienIcon.x",
    "CreateEmpire.x",
    "CreateNewGameClass.x",
    "CreateNewSuperClass.x",
    "CreateNewTheme.x",
    "DeleteAlienIcon.x",
    "DeleteBackup.x",
    "DeleteEmpire.x",
    "DeleteGameClass.x",
    "DeleteSuperClass.x",
    "DeleteTheme.x",
    "Diplomacy.x",
    "Doomsday.x",
    "Empire.x",
    "EmpireAdministrator.x",
    "EndTurn.x",
    "Engineer.x",
    "Enter.x",
    "Exit.x",
    "Documentation.x",
    "Flush.x",
    "ForceUpdate.x",
    "GameAdministrator.x",
    "HaltGameClass.x",
    "Info.x",
    "KillGame.x",
    "LeaveTheChatroom.x",
    "BLogin.x",
    "Map.x",
    "Minefield.x",
    "Minesweeper.x",
    "ObliterateEmpire.x",
    "OpenGameList.x",
    "Options.x",
    "PauseAllGames.x",
    "PauseGame.x",
    "PersonalGameClasses.x",
    "Planets.x",
    "ProfileEditor.x",
    "ProfileViewer.x",
    "Purge.x",
    "Quit.x",
    "RefreshMessages.x",
    "RestartAlmonaster.x",
    "RestartServer.x",
    "RestoreBackup.x",
    "RestoreEmpire.x",
    "Satellite.x",
    "Science.x",
    "Search.x",
    "Selection.x",
    "SendMessage.x",
    "ServerAdministrator.x",
    "ServerNews.x",
    "ServerInformation.x",
    "Ships.x",
    "ShutdownServer.x",
    "Speak.x",
    "Stargate.x",
    "Start.x",
    "System.x",
    "SystemGameList.x",
    "Tech.x",
    "Terraformer.x",
    "ThemeAdministrator.x",
    "TopLists.x",
    "Troopship.x",
    "UndeleteEmpire.x",
    "UndeleteGameClass.x",
    "UnendTurn.x",
    "UnhaltGameClass.x",
    "UnpauseAllGames.x",
    "UnpauseGame.x",
    "ViewEmpiresGameClasses.x",
    "ViewEmpiresNukeHistory.x",
    "ViewGameInformation.x",
    "ViewMap.x",
    "ViewMessages.x",
    "ViewProfile.x",
    "SearchIPs.x",
    "SearchIDs.x",
    "ViewInfo.x",
    "Resign.x",
    "StartBYOG.x",
    "RenameSuperClass.x",
    "ClearMessages.x",
    "Surrender.x",
    "Carrier.x",
    "Builder.x",
    "Morpher.x",
    "Jumpgate.x",
    "LatestNukes.x",
    "BridierScore.x",
    "EstablishedBridierScore.x",
    "SpectatorGames.x",
    "Block.x",
    "Reset.x",
    "Contributions.x",
    "Credits.x",
    "LatestGames.x",
    "TournamentAdministrator.x",
    "PersonalTournaments.x",
    "CreateNewTournament.x",
    "DeleteTournament.x",
    "AdministerTournament.x",
    "InviteEmpire.x",
    "CreateTeam.x",
    "ViewTournamentInformation.x",
    "Decline.x",
    "Accept.x",
    "AdministerTeam.x",
    "DeleteTeam.x",
    "AddEmpire.x",
    "Update.x",
    "ChooseIcon.x",
    "ChooseTheme.x",
    "Tournaments.x",
    "Join.x",
    "ViewEmpiresTournaments.x",
    "Rebuild.x",
    "Lookup.x",
    "ViewMiniMap.x",
    "MiniBuild.x",
    "Plus.x",
    "Minus.x",
    "TOS.x",
    "TOSAccept.x",
    "TOSDecline.x",
    NULL
};

const char* ButtonFileName[] = {
    NULL,
    "ActiveGameList.gif",
    "AdministerGame.gif",
    "AdvancedSearch.gif",
    "All.gif",
    "AlmonasterScore.gif",
    "Attack.gif",
    "Backup.gif",
    "BlankEmpireStatistics.gif",
    "Build.gif",
    "Cancel.gif",
    "CancelAllBuilds.gif",
    "ChangeEmpiresPassword.gif",
    "ChangePassword.gif",
    "Chatroom.gif",
    "Choose.gif",
    "ClassicScore.gif",
    "Cloaker.gif",
    "Colony.gif",
    "CreateAlienIcon.gif",
    "CreateEmpire.gif",
    "CreateNewGameClass.gif",
    "CreateNewSuperClass.gif",
    "CreateNewTheme.gif",
    "DeleteAlienIcon.gif",
    "DeleteBackup.gif",
    "DeleteEmpire.gif",
    "DeleteGameClass.gif",
    "DeleteSuperClass.gif",
    "DeleteTheme.gif",
    "Diplomacy.gif",
    "Doomsday.gif",
    "Empire.gif",
    "EmpireAdministrator.gif",
    "EndTurn.gif",
    "Engineer.gif",
    "Enter.gif",
    "Exit.gif",
    "Documentation.gif",
    "Flush.gif",
    "ForceUpdate.gif",
    "GameAdministrator.gif",
    "HaltGameClass.gif",
    "Info.gif",
    "KillGame.gif",
    "LeaveTheChatroom.gif",
    "Login.gif",
    "Map.gif",
    "Minefield.gif",
    "Minesweeper.gif",
    "ObliterateEmpire.gif",
    "OpenGameList.gif",
    "Options.gif",
    "PauseAllGames.gif",
    "PauseGame.gif",
    "PersonalGameClasses.gif",
    "Planets.gif",
    "ProfileEditor.gif",
    "ProfileViewer.gif",
    "Purge.gif",
    "Quit.gif",
    "RefreshMessages.gif",
    "RestartAlmonaster.gif",
    "RestartServer.gif",
    "RestoreBackup.gif",
    "RestoreEmpire.gif",
    "Satellite.gif",
    "Science.gif",
    "Search.gif",
    "Selection.gif",
    "SendMessage.gif",
    "ServerAdministrator.gif",
    "ServerNews.gif",
    "ServerInformation.gif",
    "Ships.gif",
    "ShutdownServer.gif",
    "Speak.gif",
    "Stargate.gif",
    "Start.gif",
    "System.gif",
    "SystemGameList.gif",
    "Tech.gif",
    "Terraformer.gif",
    "ThemeAdministrator.gif",
    "TopLists.gif",
    "Troopship.gif",
    "UndeleteEmpire.gif",
    "UndeleteGameClass.gif",
    "UnendTurn.gif",
    "UnhaltGameClass.gif",
    "UnpauseAllGames.gif",
    "UnpauseGame.gif",
    "ViewEmpiresGameClasses.gif",
    "ViewEmpiresNukeHistory.gif",
    "ViewGameInformation.gif",
    "ViewMap.gif",
    "ViewMessages.gif",
    "ViewProfile.gif",
    "Search.gif",
    "Search.gif",
    "ViewEmpireInformation.gif",
    "Resign.gif",
    "Start.gif",
    "RenameSuperClass.gif",
    "ClearMessages.gif",
    "Surrender.gif",
    "Carrier.gif",
    "Builder.gif",
    "Morpher.gif",
    "Jumpgate.gif",
    "LatestNukes.gif",
    "BridierScore.gif",
    "EstablishedBridierScore.gif",
    "SpectatorGames.gif",
    "Block.gif",
    "Reset.gif",
    "Contributions.gif",
    "Credits.gif",
    "LatestGames.gif",
    "TournamentAdministrator.gif",
    "PersonalTournaments.gif",
    "CreateNewTournament.gif",
    "DeleteTournament.gif",
    "AdministerTournament.gif",
    "InviteEmpire.gif",
    "CreateTeam.gif",
    "ViewTournamentInformation.gif",
    "Decline.gif",
    "Accept.gif",
    "AdministerTeam.gif",
    "DeleteTeam.gif",
    "AddEmpire.gif",
    "Update.gif",
    "Choose.gif",
    "Choose.gif",
    "Tournaments.gif",
    "Join.gif",
    "ViewEmpiresTournaments.gif",
    "Rebuild.gif",
    "Lookup.gif",
    "ViewMiniMap.gif",
    "Build.gif",
    "Plus.gif",
    "Minus.gif",
    "TOS.gif",
    "Accept.gif",
    "Decline.gif",
    NULL
};

const ButtonId PageButtonId[] = {
    BID_FIRST,
    BID_ACTIVEGAMELIST,
    BID_EXIT,
    BID_FIRST,  // Don't use
    BID_OPENGAMELIST,
    BID_SYSTEMGAMELIST,
    BID_PROFILEEDITOR,
    BID_TOPLISTS,
    BID_PROFILEVIEWER,
    BID_SERVERADMINISTRATOR,
    BID_EMPIREADMINISTRATOR,
    BID_GAMEADMINISTRATOR,
    BID_THEMEADMINISTRATOR,
    BID_PERSONALGAMECLASSES,
    BID_CHATROOM,
    BID_SERVERINFORMATION,
    BID_DOCUMENTATION,
    BID_SERVERNEWS,
    BID_INFO,
    BID_TECH,
    BID_DIPLOMACY,
    BID_MAP,
    BID_PLANETS,
    BID_OPTIONS,
    BID_BUILD,
    BID_SHIPS,
    BID_SERVERINFORMATION,
    BID_DOCUMENTATION,
    BID_SERVERNEWS,
    BID_PROFILEVIEWER,
    BID_CONTRIBUTIONS,
    BID_CREDITS,
    BID_TOS,
    BID_QUIT,
    BID_LATESTNUKES,
    BID_SPECTATORGAMES,
    BID_CONTRIBUTIONS,
    BID_CREDITS,
    BID_LATESTGAMES,
    BID_TOURNAMENTADMINISTRATOR,
    BID_PERSONALTOURNAMENTS,
    BID_TOURNAMENTS,
    BID_TOS,
    BID_LAST
};

const char* PageName[] = {
    NULL,
    "Active Game List",
    "Login",
    "New Empire",
    "Open Game List",
    "System Game List",
    "Profile Editor",
    "Top Lists",
    "Profile Viewer",
    "Server Administrator",
    "Empire Administrator",
    "Game Administrator",
    "Theme Administrator",
    "Personal GameClasses",
    "Chatroom",
    "Server Information",
    "Documentation",
    "Server News",
    "Info",
    "Tech",
    "Diplomacy",
    "Map",
    "Planets",
    "Options",
    "Build",
    "Ships",
    "Server Information",
    "Documentation",
    "Server News",
    "Profile Viewer",
    "Contributions",
    "Credits",
    "Terms of Service",
    "Info",
    "Latest Nukes",
    "Spectator Games",
    "Contributions",
    "Credits",
    "Latest Games",
    "Tournament Administrator",
    "Personal Tournaments",
    "Tournaments",
    "Terms of Service",
    NULL
};

void HtmlRenderer::WriteButtonString (int iButtonKey, const char* pszButtonFileName, 
                                      const char* pszButtonText, const char* pszButtonName) {
    
    if (iButtonKey == NULL_THEME) {
        OutputText ("<input type=\"submit\" name=\"");
        m_pHttpResponse->WriteText (pszButtonName);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (pszButtonText);
        OutputText ("\">");
        return;
    }
    
    else if (iButtonKey == ALTERNATIVE_PATH) {
        
        OutputText ("<input type=\"image\" border=\"0\" alt=\"");
        m_pHttpResponse->WriteText (pszButtonText);
        OutputText ("\" src=\"");
        m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
        OutputText ("/");
        m_pHttpResponse->WriteText (pszButtonFileName);
        OutputText (DEFAULT_IMAGE_EXTENSION "\" name=\"");
        m_pHttpResponse->WriteText (pszButtonName);
        OutputText ("\">");
        return;
    }
    
    OutputText ("<input type=\"image\" border=\"0\" alt=\"");
    m_pHttpResponse->WriteText (pszButtonText);
    OutputText ("\" src=\"" BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iButtonKey);
    OutputText ("/");
    m_pHttpResponse->WriteText (pszButtonFileName);
    OutputText (DEFAULT_IMAGE_EXTENSION "\" name=\"");
    m_pHttpResponse->WriteText (pszButtonName);
    OutputText ("\">");
}

int HtmlRenderer::GetButtonName (const char* pszFormName, int iButtonKey, String* pstrButtonName) {
    
    if (iButtonKey == NO_KEY || iButtonKey == NULL_THEME) {
        *pstrButtonName = pszFormName;
    } else {
        *pstrButtonName = pszFormName;
        *pstrButtonName += ".x";
    }
    
    return pstrButtonName->GetCharPtr() != NULL ? OK : ERROR_OUT_OF_MEMORY;
}

bool HtmlRenderer::IsLegalButtonId (ButtonId bidButton) {
    
    return bidButton > BID_FIRST && bidButton < BID_LAST;
}


bool HtmlRenderer::WasButtonPressed (ButtonId bidButton) {
    
    Assert (IsLegalButtonId (bidButton));
    
    if (m_iButtonKey == NULL_THEME) {   
        return m_pHttpRequest->GetForm (ButtonName[bidButton]) != NULL;
    }
    
    return m_pHttpRequest->GetForm (ButtonImageName[bidButton]) != NULL;
}

void HtmlRenderer::WriteButton (ButtonId bidButton) {
    
    Assert (IsLegalButtonId (bidButton));
    
    switch (m_iButtonKey) {
        
    case NULL_THEME:
        
        OutputText ("<input type=\"submit\" name=\"");
        m_pHttpResponse->WriteText (ButtonName[bidButton]);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (ButtonText[bidButton]);
        OutputText ("\">");
        
        break;

    case ALTERNATIVE_PATH:
        
        OutputText ("<input type=\"image\" border=\"0\" alt=\"");
        m_pHttpResponse->WriteText (ButtonText[bidButton]);
        OutputText ("\" src=\"");
        m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
        OutputText ("/");
        m_pHttpResponse->WriteText (ButtonFileName[bidButton]);
        OutputText ("\" name=\"");
        m_pHttpResponse->WriteText (ButtonName[bidButton]);
        OutputText ("\">");
        
        break;
        
    default:
        
        OutputText ("<input type=\"image\" alt=\"");
        m_pHttpResponse->WriteText (ButtonText[bidButton]);
        OutputText ("\" src=\"" BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (m_iButtonKey);
        OutputText ("/");
        m_pHttpResponse->WriteText (ButtonFileName[bidButton]);
        OutputText ("\" name=\"");
        m_pHttpResponse->WriteText (ButtonName[bidButton]);
        OutputText ("\">");
        
        break;
    }
}