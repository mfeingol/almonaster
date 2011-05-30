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

#include "Osal/File.h"
#include "Osal/Algorithm.h"
#include "Osal/TempFile.h"

#include <stdio.h>
#include <math.h>

/*

  <script language="JavaScript">
  <!--
  function Open(text){
  var newWindow;
  newWindow=open("", "SensitiveWindow","toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes,resizable=yes,copyhistory=no,width=600,height=180");
  newWindow.document.writeln (text);
  newWindow.document.close();
  }
  -->
  </script>
	  
		onMouseOver="Open ('Wombat: 3 Sciences, 2 Attacks<p>Booga: 1 Science, 3 Troopships');"
		
*/

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
	"Enter Game",
	"Exit",
	"FAQ",
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
	"Server Status",
	"Ships",
	"Shutdown Server",
	"Speak",
	"Stargate",
	"Start Game",
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
	"Choose Alien",
	"Choose Theme",
	"Start Personal Game",
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
	"EnterGame",
	"Exit",
	"FAQ",
	"Flush",
	"ForceUpdate",
	"GameAdministrator",
	"HaltGameClass",
	"Info",
	"KillGame",
	"LeaveTheChatroom",
	"Login",
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
	"ServerStatus",
	"Ships",
	"ShutdownServer",
	"Speak",
	"Stargate",
	"StartGame",
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
	"ChooseAlien",
	"ChooseTheme",
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
	"EnterGame.x",
	"Exit.x",
	"FAQ.x",
	"Flush.x",
	"ForceUpdate.x",
	"GameAdministrator.x",
	"HaltGameClass.x",
	"Info.x",
	"KillGame.x",
	"LeaveTheChatroom.x",
	"Login.x",
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
	"ServerStatus.x",
	"Ships.x",
	"ShutdownServer.x",
	"Speak.x",
	"Stargate.x",
	"StartGame.x",
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
	"ChooseAlien.x",
	"ChooseTheme.x",
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
	"EnterGame.gif",
	"Exit.gif",
	"FAQ.gif",
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
	"ServerStatus.gif",
	"Ships.gif",
	"ShutdownServer.gif",
	"Speak.gif",
	"Stargate.gif",
	"StartGame.gif",
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
	"Choose.gif",
	"Choose.gif",
	"StartGame.gif",
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
	NULL
};

const ButtonId PageButtonId[] = {
	BID_FIRST,
	BID_ACTIVEGAMELIST,
	BID_EXIT,
	BID_FIRST,	// Don't use
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
	BID_SERVERRULES,
	BID_FAQ,
	BID_SERVERNEWS,
	BID_INFO,
	BID_TECH,
	BID_DIPLOMACY,
	BID_MAP,
	BID_PLANETS,
	BID_OPTIONS,
	BID_BUILD,
	BID_SHIPS,
	BID_SERVERRULES,
	BID_FAQ,
	BID_SERVERNEWS,
	BID_PROFILEVIEWER,
	BID_QUIT,
	BID_LATESTNUKES,
	BID_SPECTATORGAMES,
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
	"Server Status",
	"FAQ",
	"Server News",
	"Info",
	"Tech",
	"Diplomacy",
	"Map",
	"Planets",
	"Options",
	"Build",
	"Ships",
	"Server Status",
	"Server News",
	"FAQ",
	"Profile Viewer",
	"Info",
	"Latest Nukes",
	"Spectator Games",
	NULL
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HtmlRenderer::HtmlRenderer (PageId pageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse) {
	
	m_pHttpRequest = pHttpRequest;
	m_pHttpResponse = pHttpResponse;
	
	m_pgPageId = pageId;
	
	m_vEmpireName = (const char*) NULL;
	m_vPreviousIPAddress = (const char*) NULL;
	
	*m_pszGameClassName = '\0';
	
	m_iEmpireKey = NO_KEY;
	m_iGameClass = NO_KEY;
	m_iGameNumber = NO_KEY;
	m_iButtonKey = NO_KEY;
	m_iBackgroundKey = NO_KEY;
	m_iSeparatorKey = NO_KEY;
	m_iPrivilege = NOVICE;
	m_iAlienKey = NO_KEY;
	m_iThemeKey = NO_KEY;
	m_iGameState = 0;
	m_iReserved = NO_KEY;
	
	m_bRedirection = false;
	m_bHalted = false;
	m_bRepeatedButtons = false;
	m_bCountdown = false;
	m_bAutoRefresh = false;
	m_bFixedBackgrounds = false;
	m_bTimeDisplay = false;
	m_bOwnPost = false;
	m_bReadyForUpdate = false;
	m_bLoggedIn = false;
	m_bAuthenticated = false;
	
	m_sSecondsUntil = 0;
	m_sSecondsSince = 0;
	Time::StartTimer (&m_tmTimer);
	
	m_bHashPasswordWithIPAddress = false;
	m_bHashPasswordWithSessionId = false;
	
	m_bNotifiedProfileLink = false;
	
	m_i64SessionId = NO_SESSION_ID;
}

int HtmlRenderer::Render() {
	
	Assert (m_pgPageId > MIN_PAGE_ID && m_pgPageId < MAX_PAGE_ID);
	
	return g_pfxnRenderPage[m_pgPageId] (this);
}

int HtmlRenderer::Redirect (PageId pageId) {
	
	Assert (pageId > MIN_PAGE_ID && pageId < MAX_PAGE_ID);
	
	m_bRedirection = true;
	m_pgPageId = pageId;
	
	// Best effort
	m_pHttpResponse->Clear();
	
	// Call the function
	return g_pfxnRenderPage[pageId] (this);
}

void HtmlRenderer::ShutdownServer () {
	
	g_pHttpServer->Shutdown();
	AddMessage ("The server will shut down now");
}

void HtmlRenderer::RestartServer () {
	
	g_pHttpServer->Restart (NULL);
	AddMessage ("The server will now restart");
}

void HtmlRenderer::RestartAlmonaster () {
	
	g_pPageSourceControl->Restart();
	AddMessage ("Almonaster will now restart");
}

bool HtmlRenderer::IsGamePage (PageId pageId) {
	
	return (pageId >= INFO && pageId <= QUIT);
}

void HtmlRenderer::WriteBackgroundImageSrc (int iThemeKey) {
	
	// No Null Theme background image
	
	OutputText (BASE_RESOURCE_DIR);
	m_pHttpResponse->WriteText (iThemeKey);
	OutputText ("/" BACKGROUND_IMAGE);
}

void HtmlRenderer::WriteLivePlanetImageSrc (int iThemeKey) {
	
	if (iThemeKey == NULL_THEME) {
		OutputText (BASE_RESOURCE_DIR LIVE_PLANET_NAME);
	} else {
		OutputText (BASE_RESOURCE_DIR);
		m_pHttpResponse->WriteText (iThemeKey);
		OutputText ("/" LIVE_PLANET_NAME);
	}
}

void HtmlRenderer::WriteDeadPlanetImageSrc (int iThemeKey) {
	
	if (iThemeKey == NULL_THEME) {
		OutputText (BASE_RESOURCE_DIR DEAD_PLANET_NAME);
	} else {
		OutputText (BASE_RESOURCE_DIR);
		m_pHttpResponse->WriteText (iThemeKey);
		OutputText ("/" DEAD_PLANET_NAME);
	}
}

void HtmlRenderer::WriteSeparatorSrc (int iThemeKey) {
	
	// No Null Theme separator
	
	OutputText (BASE_RESOURCE_DIR);
	m_pHttpResponse->WriteText (iThemeKey);
	OutputText ("/" SEPARATOR_IMAGE);
}

void HtmlRenderer::WriteHorzSrc (int iThemeKey) {
	
	if (iThemeKey == NULL_THEME) {
		OutputText (BASE_RESOURCE_DIR HORZ_LINE_NAME);
	} else {
		OutputText (BASE_RESOURCE_DIR);
		m_pHttpResponse->WriteText (iThemeKey);
		OutputText ("/" HORZ_LINE_NAME);
	}
}

void HtmlRenderer::WriteVertSrc (int iThemeKey) {
	
	if (iThemeKey == NULL_THEME) {
		OutputText (BASE_RESOURCE_DIR VERT_LINE_NAME);
	} else {
		OutputText (BASE_RESOURCE_DIR);
		m_pHttpResponse->WriteText (iThemeKey);
		OutputText ("/" VERT_LINE_NAME);
	}
}

int HtmlRenderer::GetHorzString (int iThemeKey, String* pstrString, bool bBlowup) {
	
	switch (iThemeKey) {
		
	case NULL_THEME:
		
		if (bBlowup) {
			*pstrString = "<img src=\"" BASE_RESOURCE_DIR HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
		} else {
			*pstrString = "<img src=\"" BASE_RESOURCE_DIR HORZ_LINE_NAME "\">";
		}
		break;
		
	case ALTERNATIVE_PATH:
		
		*pstrString = "<img src=\"";
		*pstrString += m_vLocalPath.GetCharPtr();
		
		if (bBlowup) {
			*pstrString += "/" HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
		} else {
			*pstrString += "/" HORZ_LINE_NAME "\">";
		}
		break;
		
	default:
		
		if (bBlowup) {
			*pstrString = "<img src=\"" BASE_RESOURCE_DIR;
			*pstrString += iThemeKey;
			*pstrString += "/" HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
		} else {
			*pstrString = "<img src=\"" BASE_RESOURCE_DIR;
			*pstrString += iThemeKey;
			*pstrString += "/" HORZ_LINE_NAME "\">";
		}
		break;
	}
	
	if (pstrString->GetCharPtr() == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}
	
	return OK;
}

int HtmlRenderer::GetVertString (int iThemeKey, String* pstrString, bool bBlowup) {
	
	switch (iThemeKey) {
		
	case NULL_THEME:
		
		if (bBlowup) {
			*pstrString = "<img src=\"" BASE_RESOURCE_DIR VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
		} else {
			*pstrString = "<img src=\"" BASE_RESOURCE_DIR VERT_LINE_NAME "\">";
		}
		break;
		
	case ALTERNATIVE_PATH:
		
		*pstrString = "<img src=\"";
		*pstrString += m_vLocalPath.GetCharPtr();
		
		if (bBlowup) {
			*pstrString += "/" VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
		} else {
			*pstrString += "/" VERT_LINE_NAME "\">";
		}
		break;
		
	default:
		
		*pstrString = "<img src=\"" BASE_RESOURCE_DIR;
		*pstrString += iThemeKey;
		
		if (bBlowup) {
			*pstrString += "/" VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
		} else {
			*pstrString += "/" VERT_LINE_NAME "\">";
		}
		break;
	}
	
	if (pstrString->GetCharPtr() == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}
	
	return OK;
}


int HtmlRenderer::GetUIData (int iThemeKey) {
	
	int iErrCode = OK;
	
	if (iThemeKey == INDIVIDUAL_ELEMENTS) {
		
		iErrCode = g_pGameEngine->GetEmpireButtonKey (m_iEmpireKey, &m_iButtonKey);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		iErrCode = g_pGameEngine->GetEmpireBackgroundKey (m_iEmpireKey, &m_iBackgroundKey);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		iErrCode = g_pGameEngine->GetEmpireSeparatorKey (m_iEmpireKey, &m_iSeparatorKey);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		int iColorKey;
		iErrCode = g_pGameEngine->GetEmpireColorKey (m_iEmpireKey, &iColorKey);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		iErrCode = GetTextColorData (iColorKey);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
	} else {
		
		m_iButtonKey = iThemeKey;
		m_iBackgroundKey = iThemeKey;
		m_iSeparatorKey = iThemeKey;
		
		if (iThemeKey == NULL_THEME) {
			
			iErrCode = GetTextColorData (NULL_THEME);
			if (iErrCode != OK) {
				return iErrCode;
			}
		}
		
		else if (iThemeKey == ALTERNATIVE_PATH) {
			
			int iEmpireColorKey;
			
			iErrCode = g_pGameEngine->GetEmpireColorKey (m_iEmpireKey, &iEmpireColorKey);
			if (iErrCode != OK) {
				return iErrCode;
			}
			
			iErrCode = GetTextColorData (iEmpireColorKey);
			if (iErrCode != OK) {
				return iErrCode;
			}
			
			iErrCode = g_pGameEngine->GetEmpireAlternativeGraphicsPath (m_iEmpireKey, &m_vLocalPath);
			if (iErrCode != OK) {
				return iErrCode;
			}
		}
		
		else {
			
			iErrCode = GetTextColorData (iThemeKey);
			if (iErrCode != OK) {
				return iErrCode;
			}
		}
	}
	
	return iErrCode;
}


int HtmlRenderer::GetTextColorData (int iEmpireColorKey) {
	
	int iErrCode;
	
	switch (iEmpireColorKey) {
		
	case NULL_THEME:
		
		m_vTableColor = DEFAULT_TABLE_COLOR;
		m_vTextColor = DEFAULT_TEXT_COLOR;
		m_vGoodColor = DEFAULT_GOOD_COLOR;
		m_vBadColor = DEFAULT_BAD_COLOR;
		m_vPrivateMessageColor = DEFAULT_PRIVATE_MESSAGE_COLOR;
		m_vBroadcastMessageColor = DEFAULT_BROADCAST_MESSAGE_COLOR;
		break;
		
	case CUSTOM_COLORS:
		
		{			
			iErrCode = g_pGameEngine->GetEmpireCustomTableColor (m_iEmpireKey, &m_vTableColor);
			if (iErrCode != OK) {
				return iErrCode;
			}
			
			iErrCode = g_pGameEngine->GetEmpireCustomTextColor (m_iEmpireKey, &m_vTextColor);
			if (iErrCode != OK) {
				return iErrCode;
			}
			
			iErrCode = g_pGameEngine->GetEmpireCustomGoodColor (m_iEmpireKey, &m_vGoodColor);
			if (iErrCode != OK) {
				return iErrCode;
			}
			
			iErrCode = g_pGameEngine->GetEmpireCustomBadColor (m_iEmpireKey, &m_vBadColor);
			if (iErrCode != OK) {
				return iErrCode;
			}
			
			iErrCode = g_pGameEngine->GetEmpireCustomPrivateMessageColor (m_iEmpireKey, &m_vPrivateMessageColor);
			if (iErrCode != OK) {
				return iErrCode;
			}
			
			iErrCode = g_pGameEngine->GetEmpireCustomBroadcastMessageColor (m_iEmpireKey, &m_vBroadcastMessageColor);
			if (iErrCode != OK) {
				return iErrCode;
			}
			break;
			
		}
		
	default:
		
		iErrCode = g_pGameEngine->GetThemeTextColor (iEmpireColorKey, &m_vTextColor);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		iErrCode = g_pGameEngine->GetThemeGoodColor (iEmpireColorKey, &m_vGoodColor);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		iErrCode = g_pGameEngine->GetThemeBadColor (iEmpireColorKey, &m_vBadColor);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		iErrCode = g_pGameEngine->GetThemePrivateMessageColor (iEmpireColorKey, &m_vPrivateMessageColor);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		iErrCode = g_pGameEngine->GetThemeBroadcastMessageColor (iEmpireColorKey, &m_vBroadcastMessageColor);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		iErrCode = g_pGameEngine->GetThemeTableColor (iEmpireColorKey, &m_vTableColor);
		if (iErrCode != OK) {
			return iErrCode;
		}
		break;
	}
	
	return OK;
}

bool HtmlRenderer::IsColor (const char* pszColor) {
	
	if (String::StrLen (pszColor) != MAX_COLOR_LENGTH) {
		return false;
	}
	
	int i;
	char szChar;
	
	for (i = 0; i < MAX_COLOR_LENGTH; i ++) {
		
		szChar = pszColor[i];
		
		if (!(szChar >= '0' && szChar <= '9') &&
			!(szChar >= 'a' && szChar <= 'f') && 
			!(szChar >= 'A' && szChar <= 'F')
			) {
			return false;
		}
	}
	
	return true;
}


void HtmlRenderer::WriteButtonImageSrc (int iRealThemeKey, const char* pszButtonName) {
	
	m_pHttpResponse->WriteText (BASE_RESOURCE_DIR);
	m_pHttpResponse->WriteText (iRealThemeKey);
	OutputText ("/");
	m_pHttpResponse->WriteText (pszButtonName);
	m_pHttpResponse->WriteText (DEFAULT_IMAGE_EXTENSION);
}

void HtmlRenderer::WriteThemeDownloadSrc (int iRealThemeKey, const char* pszFileName) {
	
	m_pHttpResponse->WriteText (BASE_RESOURCE_DIR);
	m_pHttpResponse->WriteText (iRealThemeKey);
	OutputText ("/");
	m_pHttpResponse->WriteText (pszFileName);
}

int HtmlRenderer::StandardizeEmpireName (const char* pszName, char pszFinalName[MAX_EMPIRE_NAME_LENGTH + 1]) {
	
	char pszCopy [MAX_EMPIRE_NAME_LENGTH + 1];
	strcpy (pszCopy, pszName);
	
	char* pszNameCopy = pszCopy;
	
	// Remove beginning spaces
	int i = 0, iLen = strlen (pszNameCopy);
	while (i < iLen && pszNameCopy[i] == ' ') {
		pszNameCopy ++;
		iLen --;
	}
	
	if (i == iLen) {
		
		AddMessage ("An empire name must contain more than just spaces");
		return ERROR_FAILURE;
		
	}
	
	// Remove trailing spaces
	i = iLen - 1;
	while (i > -1 && pszNameCopy[i] == ' ') {
		pszNameCopy[i] = '\0';
		i --;
		iLen --;
	}
	
	// Remove intermediate spaces
	i = iLen - 1;
	while (i > 0) {
		if (pszNameCopy[i] == ' ' && pszNameCopy[i - 1] == ' ') {
			strcpy (pszNameCopy + i - 1, pszNameCopy + i);
			iLen --;
		}
		i --;
	}
	
	// Make sure there's something left
	if (*pszNameCopy == '\0') {
		AddMessage ("An empire name must contain more than just spaces");
		return ERROR_FAILURE;
	}
	
	// Make sure the name isn't SYSTEM_MESSAGE_SENDER
	if (stricmp (pszNameCopy, SYSTEM_MESSAGE_SENDER) == 0) {
		AddMessage (SYSTEM_MESSAGE_SENDER " is a reserved name");
		return ERROR_FAILURE;
	}
	
	// Name is valid
	strcpy (pszFinalName, pszNameCopy);
	
	return OK;
}

int HtmlRenderer::VerifyPassword (const char* pszPassword, bool bPrintErrors) {
	
	if (pszPassword == NULL || *pszPassword == '\0') {
		if (bPrintErrors) {
			AddMessage ("Passwords cannot be blank");
		}
		return ERROR_FAILURE;
	}
	
	size_t i, stLength = strlen (pszPassword);
	if (stLength > MAX_PASSWORD_LENGTH) {
		
		if (bPrintErrors) {
			char pszText [256];
			sprintf (pszText, "Passwords cannot be longer than %i characters", MAX_PASSWORD_LENGTH);
			AddMessage (pszText);
		}
		
		return ERROR_FAILURE;
	}
	
	// Make sure the characters are permitted
	for (i = 0; i < stLength; i ++) {
		if (pszPassword[i] < FIRST_VALID_PASSWORD_CHAR || 
			pszPassword[i] > LAST_VALID_PASSWORD_CHAR) {
			if (bPrintErrors) {
				AddMessage ("The password contains an invalid character");
			}
			return ERROR_FAILURE;
		}
	}
	
	// We're ok
	return OK;
}

int HtmlRenderer::VerifyEmpireName (const char* pszEmpireName, bool bPrintErrors) {
	
	char c;
	
	if (pszEmpireName == NULL || *pszEmpireName == '\0') {
		if (bPrintErrors) {
			AddMessage ("Empire names cannot be blank");
		}
		return ERROR_FAILURE;
	}
	
	size_t i, stLength = strlen (pszEmpireName);
	if (stLength > MAX_EMPIRE_NAME_LENGTH) {
		
		if (bPrintErrors) {
			
			char pszText [256];
			sprintf (pszText, "Empire names cannot be longer than %i characters", MAX_EMPIRE_NAME_LENGTH);
			
			AddMessage (pszText);
		}
		return ERROR_FAILURE;
	}
	
	// Make sure the characters are permitted
	for (i = 0; i < stLength; i ++) {
		
		c = pszEmpireName[i];
		
		if (c < FIRST_VALID_EMPIRE_NAME_CHAR || 
			c > LAST_VALID_EMPIRE_NAME_CHAR ||
			c == '\"' ||
			c == ',' ||
			c == '>' ||
			c == '<' ||
			c == '(' ||
			c == ')' ||
			c == '&' ||
			c == '#'
			) {
			
			if (bPrintErrors) {
				AddMessage ("The empire name contains an invalid character");
			}
			return ERROR_FAILURE;
		}
	}
	
	// We're ok
	return OK;
}

int HtmlRenderer::VerifyGameClassName (const char* pszGameClassName, bool bPrintErrors) {
	
	char c;
	
	if (pszGameClassName == NULL || *pszGameClassName == '\0') {
		if (bPrintErrors) {
			AddMessage ("Gameclass names cannot be blank");
		}
		return ERROR_FAILURE;
	}
	
	size_t i, stLength = strlen (pszGameClassName);
	if (stLength > MAX_GAME_CLASS_NAME_LENGTH) {
		
		if (bPrintErrors) {
			char pszText [256];
			sprintf (pszText, "Gameclass names cannot be longer than %i characters", MAX_GAME_CLASS_NAME_LENGTH);
			AddMessage (pszText);
		}
		
		return ERROR_FAILURE;
	}
	
	// Make sure the characters are permitted
	for (i = 0; i < stLength; i ++) {
		
		c = pszGameClassName[i];
		
		if (c < FIRST_VALID_GAMECLASS_NAME_CHAR || 
			c > LAST_VALID_GAMECLASS_NAME_CHAR ||
			c == '\\' ||
			c == '/' ||
			c == ':' ||
			c == '*' ||
			c == '?' ||
			c == '\"' ||
			c == '<' ||
			c == '>' ||
			c == '|'
			) {
			
			if (bPrintErrors) {
				AddMessage ("The gameclass name contains an invalid character");
			}
			return ERROR_FAILURE;
		}
	}
	
	// We're ok
	return OK;
}

int HtmlRenderer::VerifyServerName (const char* pszServerName, bool bPrintErrors) {
	
	if (pszServerName == NULL || *pszServerName == '\0') {
		if (bPrintErrors) {
			AddMessage ("Server names cannot be blank");
		}
		return ERROR_FAILURE;
	}
	
	size_t i, stLength = strlen (pszServerName);
	if (stLength > MAX_SERVER_NAME_LENGTH) {
		
		if (bPrintErrors) {
			char pszText [256];
			sprintf (pszText, "Server names cannot be longer than %i characters", MAX_SERVER_NAME_LENGTH);
			AddMessage (pszText);
		}
		
		return ERROR_FAILURE;
	}
	
	// Make sure the characters are permitted
	for (i = 0; i < stLength; i ++) {
		
		if (pszServerName[i] < FIRST_VALID_SERVER_NAME_CHAR || 
			pszServerName[i] > LAST_VALID_SERVER_NAME_CHAR ||
			pszServerName[i] == '\"' ||
			pszServerName[i] == ',' ||
			pszServerName[i] == '>' ||
			pszServerName[i] == '<' ||
			pszServerName[i] == '&' ||
			pszServerName[i] == '#'
			) {
			if (bPrintErrors) {
				AddMessage ("The server name contains an invalid character");
			}
			return ERROR_FAILURE;
		}
	}
	
	// We're ok
	return OK;
}

void HtmlRenderer::WriteVersionString() {
	
	m_pHttpResponse->WriteText (g_pGameEngine->GetSystemVersion());
	OutputText (" @ ");
	
	Variant vServerName;
	int iErrCode = g_pGameEngine->GetServerName (&vServerName);
	if (iErrCode == OK) {	
		m_pHttpResponse->WriteText (vServerName.GetCharPtr());
	}
}

void HtmlRenderer::WriteBodyString (Seconds iSecondsUntil) {
	
	OutputText ("<body bgcolor=\"#" DEFAULT_BG_COLOR "\"");
	
	switch (m_iBackgroundKey) {
		
	case NULL_THEME:
		break;
		
	case ALTERNATIVE_PATH:
		
		OutputText (" background=\"");
		m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
		OutputText ("/" BACKGROUND_IMAGE "\"");
		break;
		
	default:
		
		OutputText (" background=\"" BASE_RESOURCE_DIR);
		m_pHttpResponse->WriteText (m_iBackgroundKey);
		OutputText ("/" BACKGROUND_IMAGE "\"");
		break;
	}
	
	if (m_bFixedBackgrounds) {
		OutputText (" bgproperties=\"fixed\"");
	}
	
	OutputText (" text=\"#");
	m_pHttpResponse->WriteText (m_vTextColor.GetCharPtr());
	
	OutputText ("\" link=\"#" DEFAULT_LINK_COLOR "\" alink=\"#"\
		DEFAULT_ALINK_COLOR "\" vlink=\"#" DEFAULT_VLINK_COLOR "\"");
	
	if (m_bAutoRefresh && iSecondsUntil >= 0) {
		
		OutputText (" onLoad=\"setTimeout ('m_bAutoRefresh()', ");
		m_pHttpResponse->WriteText (iSecondsUntil * 1000);
		OutputText (")\"><script><!--\n"\
			"function m_bAutoRefresh() {\n"\
			"document.forms[0].Auto.value=\"1\";\n"\
			"document.forms[0].submit();\n"\
			"} //--></script>"
			);
		
	} else {
		
		OutputText (">");
	}
}

void HtmlRenderer::WriteAlmonasterBanner() {
	
	OutputText ("<img alt=\"Almonaster\" src=\"" BASE_RESOURCE_DIR ALMONASTER_BANNER_IMAGE "\">");
}


void HtmlRenderer::WriteSeparatorString (int iSeparatorKey) {
	
	switch (iSeparatorKey) {
		
	case NULL_THEME:
		OutputText (DEFAULT_SEPARATOR_STRING);
		break;
		
	case ALTERNATIVE_PATH:
		
		OutputText ("<img src=\"");
		m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
		OutputText ("/" SEPARATOR_IMAGE "\" width=\"90%\" height=\"16\">");
		break;
		
	default:
		
		OutputText ("<img src=\"" BASE_RESOURCE_DIR);
		m_pHttpResponse->WriteText (iSeparatorKey);		
		OutputText ("/" SEPARATOR_IMAGE "\" width=\"90%\" height=\"16\">");
		break;
	}
}


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


void HtmlRenderer::OpenForm () {
	
	OutputText ("<form method=\"post\"><input type=\"hidden\" name=\"PageId\" value=\"");
	m_pHttpResponse->WriteText ((int) m_pgPageId);
	OutputText ("\">");
}


void HtmlRenderer::SendWelcomeMessage (int iEmpireKey, const char* pszEmpireName) {
	
	size_t stLen = strlen (pszEmpireName);
	size_t stLen2 = sizeof ("Welcome to Almonaster, ") - 1;
	size_t stLen3 = sizeof (". If you are new to the game then you should read the FAQ before you begin to play.");
	
	char* pszBuffer = (char*) StackAlloc (stLen + stLen2 + stLen3);
	char* pszBuffer2 = pszBuffer + stLen2;
	
	strncpy (pszBuffer, "Welcome to Almonaster, ", stLen2);
	strncpy (pszBuffer2, pszEmpireName, stLen);
	strncpy (pszBuffer2 + stLen, ". If you are new to the game then you should read the FAQ before you begin to play.", stLen3);
	
	g_pGameEngine->SendSystemMessage (
		iEmpireKey,
		pszBuffer,
		SYSTEM
		);
}

uint64 HtmlRenderer::GetPasswordHash () {
	
	// 2147483648 is (2^31)
	return (uint64)
		
		Algorithm::GetStringHashValue (m_vPassword.GetCharPtr(), 2147483648, false) * 
		Algorithm::GetStringHashValue (m_pHttpRequest->GetBrowserName(), 2147483648, false);
}

uint64 HtmlRenderer::GetGamePagePasswordHash () {
	
	// 2147483648 is (2^31)
	uint64 ui64Hash = (uint64) 
		Algorithm::GetStringHashValue (m_vPassword.GetCharPtr(), 2147483648, false);
	
	if (m_bHashPasswordWithIPAddress) {
		ui64Hash *= (uint64) Algorithm::GetStringHashValue (m_pHttpRequest->GetClientIP(), 2147483648, false);
	}
	
	if (m_bHashPasswordWithSessionId) {
		ui64Hash *= m_i64SessionId;
		ui64Hash += m_i64SessionId;
	}
	
	if (m_bHashPasswordWithIPAddress) {
		ui64Hash *= (uint64) Algorithm::GetStringHashValue (m_pHttpRequest->GetClientIP(), 2147483648, false);
	}
	
	ui64Hash *= (uint64) Algorithm::GetStringHashValue (m_pHttpRequest->GetBrowserName(), 2147483648, false);
	
	return ui64Hash;
}

void HtmlRenderer::HashIPAddress (const char* pszIPAddress, char* pszHashedIPAddress) {
	
	unsigned int iHashValue;
	unsigned char* pszIP = (unsigned char*) &iHashValue;
	
	// Hash the string
	iHashValue = Algorithm::GetStringHashValue (
		pszIPAddress, 
		0x78af383d,		// A random but large positive number
		false
		);
	
	// Usually the upper byte will be a very low number.  Add some deterministic "noise"
	pszIP[3] = (unsigned char) (pszIP[3] ^ pszIP[0] ^ pszIP[1] ^ pszIP[2]);
	
	sprintf (
		pszHashedIPAddress, 
		"%u.%u.%u.%u", 
		pszIP[0], 
		pszIP[1], 
		pszIP[2], 
		pszIP[3]
		);
}

void HtmlRenderer::WriteFormattedMessage (const char* pszText) {
	
	size_t stLength = String::StrLen (pszText);
	
	char* pszFinalText = (char*) StackAlloc (stLength * 4);
	*pszFinalText = '\0';
	
	// Replace all \n's with <br>'s
	char* pszBase = (char*) pszText + stLength, 
		* pszTemp = (char*) pszText, 
		* pszFind;
	
	while (true) {
		
		pszFind = strstr (pszTemp, "\n");
		
		if (pszFind == NULL) {
			strcat (pszFinalText, pszTemp);
			break;
		}
		
		*pszFind = '\0';
		strcat (pszFinalText, pszTemp);
		strcat (pszFinalText, "<br>");
		*pszFind = '\n';
		
		pszTemp = pszFind + sizeof (char);
		
		if (pszTemp >= pszBase) {
			break;
		}
	}
	
	// Replace all \t's with four &nbsp;'s
	stLength = strlen (pszFinalText);
	pszBase = pszFinalText + stLength * sizeof (char), pszTemp = pszFinalText;
	
	while (true) {
		
		pszFind = strstr (pszTemp, "\t");
		
		if (pszFind == NULL) {
			m_pHttpResponse->WriteText (pszTemp);
			break;
		}
		
		*pszFind = '\0';
		m_pHttpResponse->WriteText (pszTemp);
		OutputText ("&nbsp;&nbsp;&nbsp;&nbsp;");
		pszTemp = pszFind + sizeof (char);
		*pszFind = '\t';
		
		if (pszTemp >= pszBase) {
			break;
		}
	}
}

void HtmlRenderer::WriteTime (Seconds sNumSeconds) {
	
	int iHrs = 0, iMin = 0;
	
	if (sNumSeconds < 0) {
		OutputText ("<strong>Error: ");
		m_pHttpResponse->WriteText (sNumSeconds);
		OutputText ("</strong>");
		return;
	}
	
	if (sNumSeconds == 0) {
		OutputText ("<strong>0</strong> sec");
		return;
	}
	
	if (sNumSeconds >= 3600) {
		iHrs = sNumSeconds / 3600;
		sNumSeconds -= iHrs * 3600;
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (iHrs);
		OutputText ("</strong> hr");
		if (iHrs != 1) {
			OutputText ("s");
		}
	} else {
		OutputText ("");
	}
	
	if (sNumSeconds >= 60) {
		iMin = sNumSeconds / 60;
		sNumSeconds -= iMin * 60;
		
		if (iHrs > 0) {
			OutputText (", ");
		}
		
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (iMin);
		OutputText ("</strong> min");
	}
	
	if (sNumSeconds > 0) {
		if (iMin > 0 || iHrs > 0) {
			OutputText (", ");
		}
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (sNumSeconds);
		OutputText ("</strong> sec");
	}
}

int HtmlRenderer::ConvertTime (Seconds sNumSeconds, char pszTime[MAX_HTML_TIME_LENGTH]) {
	
	int iHrs = 0, iMin = 0;
	*pszTime = '\0';
	
	if (sNumSeconds < 0) {
		sprintf (pszTime, "<strong>Error: %i</strong>", sNumSeconds);
		return ERROR_INVALID_ARGUMENT;
	}
	
	if (sNumSeconds == 0) {
		StrNCpy (pszTime, "<strong>0</strong> sec");
		return OK;
	}
	
	if (sNumSeconds >= 3600) {
		
		iHrs = sNumSeconds / 3600;
		sNumSeconds -= iHrs * 3600;
		
		sprintf (pszTime, "<strong>%i</strong> hr", iHrs);
		
		if (iHrs != 1) {
			StrNCat (pszTime, "s");
		}
	}
	
	if (sNumSeconds >= 60) {
		
		iMin = sNumSeconds / 60;
		sNumSeconds -= iMin * 60;
		
		if (iHrs > 0) {
			StrNCat (pszTime, ", ");
		}
		
		sprintf (pszTime + strlen (pszTime), "<strong>%i</strong> min", iMin);
	}
	
	if (sNumSeconds > 0) {
		
		if (iMin > 0 || iHrs > 0) {
			StrNCat (pszTime, ", ");
		}
		
		sprintf (pszTime + strlen (pszTime), "<strong>%i</strong> sec", sNumSeconds);
	}
	
	return OK;
}


void HtmlRenderer::WriteAlienString (int iAlienKey, int iEmpireKey, const char* pszAlt, bool bVerifyUpload) {
	
	if (iAlienKey == UPLOADED_ICON) {
		
		bool bDisplay = true;
		
		if (bVerifyUpload) {
			
			// Make sure file exists
			char pszDestFileName[OS::MaxFileNameLength];
			
			sprintf (
				pszDestFileName, 
				"%s/" BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
				g_pszResourceDir,
				iEmpireKey
				);
			
			if (!File::DoesFileExist (pszDestFileName)) {
				bDisplay = false;
			}
		}
		
		if (bDisplay) {
			
			OutputText ("<img src=\"" BASE_RESOURCE_DIR BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME);
			iAlienKey = iEmpireKey;
			
		} else {
			
			// Get default icon
			int iErrCode = g_pGameEngine->GetDefaultAlien (&iAlienKey);
			if (iErrCode != OK) {
				iAlienKey = 1;
			}
			
			OutputText ("<img src=\"" BASE_RESOURCE_DIR BASE_ALIEN_DIR ALIEN_NAME);
		}
		
	} else {
		
		OutputText ("<img src=\"" BASE_RESOURCE_DIR BASE_ALIEN_DIR ALIEN_NAME);
	}
	
	m_pHttpResponse->WriteText (iAlienKey);
	OutputText (DEFAULT_IMAGE_EXTENSION "\"");
	
	if (!String::IsBlank (pszAlt)) {
		OutputText (" alt=\"");
		m_pHttpResponse->WriteText (pszAlt);
		OutputText ("\"");
	}
	
	OutputText (">");
}

void HtmlRenderer::WriteProfileAlienString (int iAlienKey, int iEmpireKey, 
											const char* pszEmpireName, int iBorder, const char* pszFormName, 
											const char* pszAlt, bool bVerifyUpload, bool bKeyAndHash) {
	
	OutputText ("<input type=\"image\" border=\"");
	m_pHttpResponse->WriteText (iBorder);
	OutputText ("\" src=\"" BASE_RESOURCE_DIR);
	
	if (iAlienKey == UPLOADED_ICON) {
		
		bool bDisplay = true;
		
		if (bVerifyUpload) {
			
			// Make sure file exists
			char pszDestFileName[OS::MaxFileNameLength];
			
			sprintf (
				pszDestFileName, 
				"%s/" BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
				g_pszResourceDir,
				iEmpireKey
				);
			
			if (!File::DoesFileExist (pszDestFileName)) {
				bDisplay = false;
			}
		}
		
		if (bDisplay) {
			
			OutputText (BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME);
			m_pHttpResponse->WriteText (iEmpireKey);
			OutputText (DEFAULT_IMAGE_EXTENSION);
			
		} else {
			
			// Get default icon
			int iErrCode = g_pGameEngine->GetDefaultAlien (&iAlienKey);
			if (iErrCode != OK) {
				iAlienKey = 1;
			}
			
			OutputText (BASE_ALIEN_DIR ALIEN_NAME);
			m_pHttpResponse->WriteText (iAlienKey);
			OutputText (DEFAULT_IMAGE_EXTENSION);
		}
		
	} else {
		
		OutputText (BASE_ALIEN_DIR ALIEN_NAME);
		m_pHttpResponse->WriteText (iAlienKey);
		OutputText (DEFAULT_IMAGE_EXTENSION);
	}
	
	OutputText ("\" name=\"");
	m_pHttpResponse->WriteText (pszFormName);
	
	if (bKeyAndHash) {
		OutputText (".");
		m_pHttpResponse->WriteText (iEmpireKey);
		OutputText (".");
		m_pHttpResponse->WriteText (Algorithm::GetStringHashValue (pszEmpireName, EMPIRE_NAME_HASH_LIMIT, true));
	}
	
	OutputText ("\" alt=\"");
	m_pHttpResponse->WriteText (pszAlt);
	OutputText ("\">");
}

void HtmlRenderer::NotifyProfileLink() {
	
	OutputText ("<input type=\"hidden\" name=\"HintProfileLink\" value=\"1\">");
}

bool HtmlRenderer::NotifiedProfileLink() {
	
	return m_pHttpRequest->GetForm ("HintProfileLink") != NULL;
}

int HtmlRenderer::HTMLFilter (const char* pszSource, String* pstrFiltered, size_t stNumChars, bool bAddMarkups) {
	
	*pstrFiltered = "";
	if (!String::IsBlank (pszSource)) {
		return String::AtoHtml (pszSource, pstrFiltered, stNumChars, bAddMarkups) != NULL ? OK : ERROR_OUT_OF_MEMORY;
	}
	
	return OK;
}

bool HtmlRenderer::VerifyGIF (const char* pszFileName) {
	
	File fGifFile;
	
	if (fGifFile.OpenRead (pszFileName) != OK) {
		AddMessage ("The uploaded file could not be opened");
		return false;
	}
	
	const size_t stHeaderSize = 10;
	char pszBuffer [stHeaderSize];
	size_t stNumBytes, stSize;
	
	// Read the gif header
	if (fGifFile.Read (pszBuffer, stHeaderSize, &stNumBytes) != OK) {
		AddMessage ("The uploaded file appears to be damaged");
		fGifFile.Close();
		return false;
	}
	
	// Close the gif file
	fGifFile.Close();
	
	if (stNumBytes != stHeaderSize) {
		AddMessage ("The uploaded file is too small");
		return false;
	}
	
	if (File::GetFileSize (pszFileName, &stSize) != OK) {
		AddMessage ("The uploaded file could not be opened");
		return false;
	}
	
	int iMaxIconSize;
	int iErrCode = g_pGameEngine->GetMaxIconSize (&iMaxIconSize);
	Assert (iErrCode == OK);
	
	if (stSize > (size_t) iMaxIconSize) {
		
		char pszError [512];
		sprintf (
			pszError, 
			"The uploaded file is larger than the upper limit (%i KB)", 
			(int) (iMaxIconSize / 1024)
			);
		AddMessage (pszError);
		return false;
	}
	
	// Ensure gif89a
	if (strncmp (ICON_FORMAT, pszBuffer, sizeof (ICON_FORMAT) - 1) != 0) {
		AddMessage ("The uploaded file is not in " ICON_FORMAT " format");
		return false;
	}
	
	// Get size of image
	short siX = *(short*) (pszBuffer + 6);
	short siY = *(short*) (pszBuffer + 8);
	
	if (siX != ICON_WIDTH || siY != ICON_HEIGHT) {
		
		char pszError [512];
		sprintf (
			pszError, 
			"The uploaded " ICON_FORMAT " file is not %ix%i",
			ICON_WIDTH,
			ICON_HEIGHT
			);
		
		AddMessage (pszError);
		return false;
	}
	
	return true;
}


int HtmlRenderer::CopyUploadedAlien (const char* pszFileName, int iEmpireKey) {
	
	int iErrCode;
	
	char pszDestFileName[OS::MaxFileNameLength];
	
	sprintf (
		pszDestFileName, 
		"%s/" BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION,
		g_pszResourceDir,
		iEmpireKey
		);
	
	// TODO:  need a better solution for this
	unsigned int iTries = 0;
	while (iTries < 20) {
		
		g_pFileCache->ReleaseFile (pszDestFileName);
		
		iErrCode = File::CopyFile (pszFileName, pszDestFileName);
		if (iErrCode == OK) {
			return OK;
		}
		
		iTries ++;
		OS::Sleep (250);
	}
	
	return OK;
}

int HtmlRenderer::CopyNewAlien (const char* pszFileName, int iAlienKey) {
	
	char pszDestFileName[OS::MaxFileNameLength];
	
	sprintf (
		pszDestFileName, 
		"%s/" BASE_ALIEN_DIR ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
		g_pszResourceDir,
		iAlienKey
		);
	
	g_pFileCache->ReleaseFile (pszDestFileName);
	
	return File::CopyFile (pszFileName, pszDestFileName);
}

int HtmlRenderer::DeleteAlien (int iAlienKey) {
	
	char pszDestFileName[OS::MaxFileNameLength];
	
	sprintf (
		pszDestFileName, 
		"%s/" BASE_ALIEN_DIR ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
		g_pszResourceDir,
		iAlienKey
		);
	
	g_pFileCache->ReleaseFile (pszDestFileName);
	
	return File::DeleteFile (pszDestFileName);
}

unsigned int HtmlRenderer::GetEmpireNameHashValue (const char* pszEmpireName) {
	
	return Algorithm::GetStringHashValue (pszEmpireName, 0xffffffff, true);
}

void HtmlRenderer::WriteStringByDiplomacy (const char* pszString, int iDiplomacy) {
	
	switch (iDiplomacy) {
		
	case WAR:
		
		OutputText ("<font color=\"");
		m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		OutputText ("\"><strong>");
		m_pHttpResponse->WriteText (pszString);
		OutputText ("</strong></font>");
		break;
		
	case ALLIANCE:
		
		OutputText ("<font color=\"");
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\"><strong>");
		m_pHttpResponse->WriteText (pszString);
		OutputText ("</strong></font>");
		break;
		
	default:
		
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (pszString);
		OutputText ("</strong>");
		break;
	}
}

int HtmlRenderer::LoginEmpire() {

	int iErrCode;
	
	if (m_vEmpireName.GetType() != V_STRING || 
		m_vEmpireName.GetCharPtr() == NULL ||
		*m_vEmpireName.GetCharPtr() == '\0') {
		
		iErrCode = g_pGameEngine->GetEmpireName (m_iEmpireKey, &m_vEmpireName);
		if (iErrCode != OK) {
			AddMessage ("Login failed: the empire's name could not be read");
			return iErrCode;
		}
	}
	
	if (m_vPassword.GetType() != V_STRING || 
		m_vPassword.GetCharPtr() == NULL ||
		*m_vPassword.GetCharPtr() == '\0') {
		
		iErrCode = g_pGameEngine->GetEmpirePassword (m_iEmpireKey, &m_vPassword);
		if (iErrCode != OK) {
			AddMessage ("Login failed: the empire's password could not be read");
			return iErrCode;
		}
	}
	
	// Add to report
	ReportLoginSuccess (g_pReport, m_vEmpireName.GetCharPtr());
	
	// We're authenticated, so register a login
	iErrCode = g_pGameEngine->LoginEmpire (m_iEmpireKey, m_pHttpRequest->GetBrowserName());
	
	if (iErrCode != OK) {
		
		if (iErrCode == ERROR_DISABLED) {
			
			String strMessage = "The server is denying all logins at this time. ";
			
			Variant vReason;
			iErrCode = g_pGameEngine->GetLoginsDisabledReason (&vReason);
			if (iErrCode != OK) {
				return iErrCode;
			}
			
			const char* pszReason = vReason.GetCharPtr();
			if (pszReason == NULL || *pszReason == '\0') {
				strMessage += "Please try back later.";
			} else {
				strMessage += pszReason;
			}
			AddMessage (strMessage);
			
		} else {
			
			char pszMessage [2048];
			
			sprintf (
				pszMessage,
				"<strong>The empire %s could not log in due to error %i. Please contact the administrator.",
				m_vEmpireName.GetCharPtr(),
				iErrCode
				);
			
			AddMessage (pszMessage);
		}
								
	} else {
		
		// Get theme key
		int iThemeKey, iOptions;

		iErrCode = g_pGameEngine->GetEmpireOptions (m_iEmpireKey, &iOptions);
		if (iErrCode != OK) {
			AddMessage ("Login failed: the empire's options could not be read");
			return iErrCode;
		}
		
		iErrCode = g_pGameEngine->GetEmpireThemeKey (m_iEmpireKey, &iThemeKey);
		if (iErrCode != OK) {
			AddMessage ("Login failed: the empire's theme key could not be read");
			return iErrCode;
		}
		
		iErrCode = GetUIData (iThemeKey);
		if (iErrCode != OK) {
			AddMessage ("Login failed: the empire's ui data could not be read");
			return iErrCode;
		}
								
		iErrCode = g_pGameEngine->GetEmpirePrivilege (m_iEmpireKey, &m_iPrivilege);
		if (iErrCode != OK) {
			AddMessage ("Login failed: the empire's privilege level could not be read");
			return iErrCode;
		}

		m_bHalted = (iOptions & EMPIRE_MARKED_FOR_DELETION) != 0;
		
		iErrCode = g_pGameEngine->GetEmpireAlienKey (m_iEmpireKey, &m_iAlienKey);
		if (iErrCode != OK) {
			AddMessage ("Login failed: the empire's alien key could not be read");
			return iErrCode;
		}

		m_bFixedBackgrounds = (iOptions & FIXED_BACKGROUNDS) != 0;
		
		iErrCode = g_pGameEngine->GetEmpireIPAddress (m_iEmpireKey, &m_vPreviousIPAddress);
		if (iErrCode != OK) {
			AddMessage ("Login failed: the empire's old IP address data could not be read");
			return iErrCode;
		}
		
		// Set a cookie for the last empire id (expires in a year)
		char pszEmpireKey [128];
		m_pHttpResponse->CreateCookie (
			LAST_EMPIRE_USED_COOKIE,
			itoa (m_iEmpireKey, pszEmpireKey, 10),
			ONE_YEAR_IN_SECONDS, 
			NULL
			);
	}
	
	return OK;
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
		
		OutputText ("<input type=\"image\" border=\"0\" alt=\"");
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

bool HtmlRenderer::VerifyEmpireNameHash (int iEmpireKey, unsigned int iHash) {
	
	int iErrCode;
	unsigned int iRealHash;
	Variant vName;
	
	iErrCode = g_pGameEngine->GetEmpireName (iEmpireKey, &vName);
	if (iErrCode != OK) {
		return false;
	}
	
	iRealHash = Algorithm::GetStringHashValue (vName.GetCharPtr(), EMPIRE_NAME_HASH_LIMIT, true);
	
	return iRealHash == iHash;
}

void HtmlRenderer::WriteGameTitleString () {
	
	m_pHttpResponse->WriteText (PageName [m_pgPageId]);
	OutputText (": ");
	WriteVersionString();
}

void HtmlRenderer::WriteGameHeaderString () {
	
	// Open form
	OutputText ("<form method=\"post\"><input type=\"hidden\" name=\"PageId\" value=\"");
	m_pHttpResponse->WriteText ((int) m_pgPageId);
	OutputText ("\">");
	
	const char* pszEmpireName = m_vEmpireName.GetCharPtr();
	
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
	
	OutputText (" <font size=\"+3\"><strong>");
	m_pHttpResponse->WriteText (pszEmpireName);
	
	if (pszEmpireName [strlen (pszEmpireName) - 1] == 's') {
		OutputText ("' ");
	} else {
		OutputText ("'s ");
	}
	
	m_pHttpResponse->WriteText (PageName[m_pgPageId]);
	OutputText (" : ");
	m_pHttpResponse->WriteText (m_pszGameClassName);
	OutputText (" ");
	m_pHttpResponse->WriteText (m_iGameNumber);
	OutputText ("</strong></font><p>");
	
	// Informational forms
	PostGamePageInformation();
	
	// Buttons
	WriteGameButtons();
	
	// Write local time
	if (m_bTimeDisplay) {
		
		char pszDateString [OS::MaxDateLength];
		
		int iErrCode = Time::GetDateString (pszDateString);
		if (iErrCode == OK) {
			
			OutputText ("Server time is <strong>");
			m_pHttpResponse->WriteText (pszDateString);
			OutputText ("</strong><p>");
		}
	}
	
	// Next update
	WriteGameNextUpdateString();
	
	// Messages
	WriteGameMessagesString();
	
	// Last separator
	WriteSeparatorString (m_iSeparatorKey);
}

bool HtmlRenderer::RedirectOnSubmitGame (PageId* ppageRedirect) {
	
	int iErrCode;
	bool bFlag;
	
	IHttpForm* pHttpForm;

	Assert (m_pgPageId > MIN_PAGE_ID && m_pgPageId < MAX_PAGE_ID);
	
	if (WasButtonPressed (PageButtonId[m_pgPageId])) {
		return false;
	}
	
	if (WasButtonPressed (BID_INFO)) {
		*ppageRedirect = INFO;
		goto True;
	}
	
	if (WasButtonPressed (BID_BUILD)) {
		*ppageRedirect = BUILD;
		goto True;
	}
	
	if (WasButtonPressed (BID_TECH)) {
		*ppageRedirect = TECH;
		goto True;
	}
	
	if (WasButtonPressed (BID_OPTIONS)) {
		*ppageRedirect = OPTIONS;
		goto True;
	}
	
	if (WasButtonPressed (BID_SHIPS)) {
		*ppageRedirect = SHIPS;
		goto True;
	}
	
	if (WasButtonPressed (BID_PLANETS)) {
		*ppageRedirect = PLANETS;
		goto True;
	}
	
	if (WasButtonPressed (BID_MAP)) {
		*ppageRedirect = MAP;
		goto True;
	}
	
	if (WasButtonPressed (BID_DIPLOMACY)) {
		*ppageRedirect = DIPLOMACY;
		goto True;
	}
	
	if (WasButtonPressed (BID_ENDTURN)) {
		
		// End turn
		iErrCode = g_pGameEngine->SetEmpireReadyForUpdate (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
		Assert (iErrCode == OK);
		
		// Redirect to same page
		if (bFlag) {
			AddMessage ("You are now ready for an update");
		}
		*ppageRedirect = m_pgPageId;
		goto True;
	}
	
	if (WasButtonPressed (BID_UNENDTURN)) {
		
		// Unend turn
		iErrCode = g_pGameEngine->SetEmpireNotReadyForUpdate (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
		
		// Redirect to same page
		if (bFlag) {
			AddMessage ("You are no longer ready for an update");
		}
		*ppageRedirect = m_pgPageId;
		goto True;
	}
	
	if (WasButtonPressed (BID_EXIT)) {
		*ppageRedirect = ACTIVE_GAME_LIST;
		goto True;
	}
	
	if (WasButtonPressed (BID_QUIT)) {
		m_iReserved = BID_QUIT;
		*ppageRedirect = QUIT;
		goto True;
	}
	
	if (WasButtonPressed (BID_SERVERRULES)) {
		*ppageRedirect = GAME_SERVER_RULES;
		goto True;
	}
	
	if (WasButtonPressed (BID_FAQ)) {
		*ppageRedirect = GAME_FAQ;
		goto True;
	}
	
	if (WasButtonPressed (BID_SERVERNEWS)) {
		*ppageRedirect = GAME_NEWS;
		goto True;
	}
	
	pHttpForm = m_pHttpRequest->GetForm ("ProfileLink.x");
	if (pHttpForm != NULL) {
		
		m_iReserved = m_iEmpireKey;
		*ppageRedirect = GAME_PROFILE_VIEWER;
		goto True;
	}
	
	if (NotifiedProfileLink()) {
		
		pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ProfileLink");
		if (pHttpForm != NULL) {
			
			const char* pszProfile = pHttpForm->GetName();
			
			Assert (pszProfile != NULL);
			
			int iViewProfileEmpireKey;
			unsigned int iHash;
			
			if (sscanf (pszProfile, "ProfileLink.%d.%d.x", &iViewProfileEmpireKey, &iHash) == 2 &&
				VerifyEmpireNameHash (iViewProfileEmpireKey, iHash)) {
				
				m_iReserved = iViewProfileEmpireKey;
				*ppageRedirect = GAME_PROFILE_VIEWER;
				goto True;
				
			} else {
				
				AddMessage ("That empire no longer exists");
			}
		}
	}
	
	return false;
	
True:
	
	g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
	return true;
}

int HtmlRenderer::InitializeGame (PageId* ppageRedirect) {
	
	int iErrCode;
	bool bFlag;
	
	PageId pgSrcPageId;
	IHttpForm* pHttpForm = m_pHttpRequest->GetForm ("PageId");
	
	pgSrcPageId = (pHttpForm != NULL) ? (PageId) pHttpForm->GetIntValue() : LOGIN;
	
	// If an auto-submission, get game class and number from forms
	if (pgSrcPageId == m_pgPageId) {
		
		// Get game class
		if ((pHttpForm = m_pHttpRequest->GetForm ("GameClass")) == NULL) {
			AddMessage ("Missing GameClass form");
			*ppageRedirect = ACTIVE_GAME_LIST;
			return ERROR_FAILURE;
		}
		m_iGameClass = pHttpForm->GetIntValue();
		
		// Get game number
		if ((pHttpForm = m_pHttpRequest->GetForm ("GameNumber")) == NULL) {
			AddMessage ("Missing GameNumber form");
			*ppageRedirect = ACTIVE_GAME_LIST;
			return ERROR_FAILURE;
		}
		m_iGameNumber = pHttpForm->GetIntValue();
		
		// Get old update count
		if ((pHttpForm = m_pHttpRequest->GetForm ("Updates")) == NULL) {
			AddMessage ("Missing Updates form");
			*ppageRedirect = ACTIVE_GAME_LIST;
			return ERROR_FAILURE;
		}
		m_iNumOldUpdates = pHttpForm->GetIntValue();	
	}
	
	// Verify existence of game
	iErrCode = g_pGameEngine->DoesGameExist (m_iGameClass, m_iGameNumber, &bFlag);
	if (iErrCode != OK || !bFlag) {
		AddMessage ("That game no longer exists");
		*ppageRedirect = ACTIVE_GAME_LIST;
		return ERROR_FAILURE;
	}
	
	// Verify empire's presence in game
	iErrCode = g_pGameEngine->IsEmpireInGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
	if (iErrCode != OK || !bFlag) {
		AddMessage ("You are no longer in that game");
		*ppageRedirect = ACTIVE_GAME_LIST;
		return ERROR_FAILURE;
	}
	
	// Get empire game variables if we're coming from a non-game page
	if (pgSrcPageId == m_pgPageId ||
		(pgSrcPageId != m_pgPageId && !IsGamePage (pgSrcPageId))
		) {

		int iOptions;

		iErrCode = g_pGameEngine->GetEmpireOptions (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iOptions);
		if (iErrCode != OK) {
			AddMessage ("That empire no longer exists");
			*ppageRedirect = LOGIN;
			return ERROR_FAILURE;
		}
		
		// Get gameclass name
		iErrCode = g_pGameEngine->GetGameClassName (m_iGameClass, m_pszGameClassName);
		if (iErrCode != OK) {
			AddMessage ("That game no longer exists");
			*ppageRedirect = ACTIVE_GAME_LIST;
			return ERROR_FAILURE;
		}

		m_bAutoRefresh = (iOptions & AUTO_REFRESH) != 0;
		m_bCountdown = (iOptions & COUNTDOWN) != 0;
		m_bRepeatedButtons = (iOptions & GAME_REPEATED_BUTTONS) != 0;
		m_bTimeDisplay = (iOptions & GAME_DISPLAY_TIME) != 0;
	}
	
	///////////////////////
	// Check for updates //
	///////////////////////
	
	bool bUpdate;
	if (g_pGameEngine->CheckGameForUpdates (m_iGameClass, m_iGameNumber, &bUpdate) != OK ||
		g_pGameEngine->WaitGameReader (m_iGameClass, m_iGameNumber) != OK
		) {
		
		// Remove update message after update
		if (bUpdate && m_strMessage.Equals ("You are now ready for an update")) {
			m_strMessage.Clear();
		}
		
		AddMessage ("The game ended");
		*ppageRedirect = ACTIVE_GAME_LIST;
		return ERROR_FAILURE;
	}
	
	// Re-verify empire's presence in game
	iErrCode = g_pGameEngine->IsEmpireInGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
	if (iErrCode != OK || !bFlag) {
		g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
		AddMessage ("You are no longer in that game");
		*ppageRedirect = ACTIVE_GAME_LIST;
		return ERROR_FAILURE;
	}
	
	// Verify not resigned
	iErrCode = g_pGameEngine->HasEmpireResignedFromGame (m_iGameClass, m_iGameNumber, m_iEmpireKey, &bFlag);
	if (iErrCode != OK || bFlag) {
		g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
		AddMessage ("Your empire has resigned from that game");
		*ppageRedirect = ACTIVE_GAME_LIST;
		return ERROR_FAILURE;
	}
	
	// Log empire into game if not an auto submission
	IHttpForm* pHttpAuto = m_pHttpRequest->GetForm ("Auto");
	if (pHttpAuto == NULL || pHttpAuto->GetIntValue() == 0 && !m_bLoggedIn) {
		
		iErrCode = g_pGameEngine->LogEmpireIntoGame (
			m_iGameClass, 
			m_iGameNumber, 
			m_iEmpireKey
			);
		
		if (iErrCode != OK) {
			AddMessage ("Your empire could not be logged into the game");
			*ppageRedirect = ACTIVE_GAME_LIST;
			return ERROR_FAILURE;
		}
		
		m_bLoggedIn = true;
	}
	
	// Remove update message after update
	if (bUpdate && m_strMessage.Equals ("You are now ready for an update")) {
		m_strMessage.Clear();
	}
	
	// Get game update information
	if (g_pGameEngine->GetGameUpdateData (
		m_iGameClass, 
		m_iGameNumber, 
		&m_sSecondsSince, 
		&m_sSecondsUntil, 
		&m_iNumNewUpdates, 
		&m_iGameState
		) != OK ||
		
		g_pGameEngine->GetEmpireOption (m_iGameClass, m_iGameNumber, m_iEmpireKey, UPDATED, &m_bReadyForUpdate) != OK

		) {
		
		Assert (false);
		g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
		
		AddMessage ("The game no longer exists");
		*ppageRedirect = ACTIVE_GAME_LIST;
		return ERROR_FAILURE;
	}
	
	return OK;
}

void HtmlRenderer::WriteGameButtons() {
	
	OutputText ("<table border=\"0\" width=\"90%\"><tr><td width=\"2%\" align=\"left\">");
	
	// Info
	WriteButton (BID_INFO);
	
	OutputText ("</td><td align=\"center\">");
	
	// Map
	WriteButton (BID_MAP);
	
	// Planets
	WriteButton (BID_PLANETS);
	
	// Diplomacy
	WriteButton (BID_DIPLOMACY);
	
	OutputText ("</td><td width=\"2%\" align=\"right\">");
	
	// Exit
	WriteButton (BID_EXIT);
	
	OutputText ("</td></tr><tr><td width=\"2%\" align=\"left\">");
	
	// Options
	WriteButton (BID_OPTIONS);
	
	OutputText ("</td><td align=\"center\">");
	
	// Ships
	WriteButton (BID_SHIPS);
	
	// Build
	WriteButton (BID_BUILD);
	
	// Tech
	WriteButton (BID_TECH);
	
	if (m_iGameState & STARTED) {
		
		if (m_bReadyForUpdate) {
			// Unend Turn
			WriteButton (BID_UNENDTURN);
		} else {
			// End Turn
			WriteButton (BID_ENDTURN);
		}
		
	} else {
		
		OutputText ("</td><td width=\"2%\" align=\"right\">");
		
		WriteButton (BID_QUIT);
	}
	
	OutputText ("</td></tr></table><p>");
}


void HtmlRenderer::WriteGameNextUpdateString() {

	//OutputText ("<p>Next update ");
	OutputText ("<p>");
	
	if (m_iNumNewUpdates > 0) {
		
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (m_iNumNewUpdates);
		OutputText ("</strong> update");
		if (m_iNumNewUpdates != 1) {
			OutputText ("s");
		}

		OutputText (", next ");

	} else {

		OutputText ("First update ");
	}
	
	if (!m_bCountdown || m_iGameState & PAUSED || !(m_iGameState & STARTED)) {
		
		if (!(m_iGameState & STARTED)) {
			
			int iNumEmpires, iNumNeeded, iTotal, 
				
				iErrCode = g_pGameEngine->GetNumEmpiresInGame (m_iGameClass, m_iGameNumber, &iNumEmpires);
			if (iErrCode != OK) {
				Assert (false);
				return;
			}
			
			iErrCode = g_pGameEngine->GetNumEmpiresNeededForGame (m_iGameClass, &iNumNeeded);
			if (iErrCode != OK) {
				Assert (false);
				return;
			}
			
			iTotal = iNumNeeded - iNumEmpires;
			
			OutputText ("when <strong>");
			m_pHttpResponse->WriteText (iTotal);
			OutputText ("</strong>");
			m_pHttpResponse->WriteText (iTotal == 1 ? " more empire joins" : " more empires join");
			
		} else {
			
			OutputText ("in ");
			WriteTime (m_sSecondsUntil);
			
			if (m_iGameState & PAUSED) {
				if (m_iGameState & ADMIN_PAUSED) {
					OutputText (" (<strong>paused by an administrator</strong>)");
				} else {
					OutputText (" (<strong>paused</strong>)");
				}
			}
		}
		
	} else {
		
		OutputText (
			
			"in <input name=\"jtimer\" size=\"22\"><script><!--\n"\
			"var t = (new Date()).getTime()/1000+");
		
		m_pHttpResponse->WriteText (m_sSecondsUntil - 1);	
		OutputText (
			
			";\n"\
			"function count() {\n"\
			"var next = '';\n"\
			"var pre = '';\n"\
			"var now = new Date();\n"\
			"var sec = Math.floor(t-now.getTime()/1000);\n"\
			"if (sec < 1) {\n"\
			"if (sec == 0) {\n"\
			"document.forms[0].jtimer.value='The update is occurring...';\n"\
			"setTimeout('count()',2000); return;\n"\
			"}\n"\
			"document.forms[0].jtimer.value='The update occurred';\n");
		
		if (m_bAutoRefresh) {
			OutputText ("setTimeout('document.forms[0].submit()', 1000);\n");
		}
		
		OutputText (
			
			"return;\n"\
			"}\n"\
			"var hrs = Math.floor(sec/3600);\n"\
			"sec -= hrs*3600;\n"\
			"var min = Math.floor(sec/60);\n"\
			"sec -= min*60;\n"\
			"if (hrs) { next += hrs+' hr'; if (hrs != 1) next += 's'; }\n"\
			"if (min) { if (hrs) next += ', '; next += min+' min'; }\n"\
			"if (sec) { if (hrs || min) next += ', '; next += sec+' sec'; }\n"\
			"document.forms[0].jtimer.value=pre+next;\n"\
			"setTimeout('count()',500);\n"\
			"}\n"\
			"count(); // --></script>");
	}
	
	OutputText ("<p>");
}


void HtmlRenderer::WriteGameMessagesString() {
	
	if (!m_strMessage.IsBlank()) {
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (m_strMessage);
		OutputText ("</strong><p>");
	}
	
	// Check for messages
	Variant** ppvMessage;
	int iNumMessages, i;
	
	int iErrCode = g_pGameEngine->GetUnreadGameMessages (
		m_iGameClass,
		m_iGameNumber,
		m_iEmpireKey,
		&ppvMessage,
		&iNumMessages
		);
	
	if (iErrCode == OK && iNumMessages > 0) {
		
		const char* pszMethod, * pszSource, * pszMessage, * pszFontColor;
		String strHTMLMessage, strFiltered;
		bool bBroadcast, bSystem, bExists;
		
		int iSrcEmpireKey, iAlienKey;
		
		unsigned int iNumMessagesFromPeople = 0;
		char pszDate [OS::MaxDateLength], pszProfile [1024];
		
		for (i = 0; i < iNumMessages; i ++) {
			
			pszSource = ppvMessage[i][GameEmpireMessages::Source].GetCharPtr();
			
			if (ppvMessage[i][GameEmpireMessages::Broadcast].GetInteger() != 0) {
				bBroadcast = true;
				pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
			} else {
				bBroadcast = false;
				pszFontColor = m_vPrivateMessageColor.GetCharPtr();
			}
			
			pszMethod = bBroadcast ? "</strong> broadcast:<p>" : "</strong> sent:";
			
			bSystem = strcmp (pszSource, SYSTEM_MESSAGE_SENDER) == 0;
			
			// Format message
			if (bSystem) {
				pszMessage = ppvMessage[i][GameEmpireMessages::Text].GetCharPtr();
			} else {
				
				if (HTMLFilter (
					ppvMessage[i][GameEmpireMessages::Text].GetCharPtr(), 
					&strFiltered, 
					MAX_NUM_SPACELESS_CHARS,
					true
					) == OK) {
					
					pszMessage = strFiltered.GetCharPtr();
					
				} else {
					
					pszMessage = "The server is out of memory";
				}
			}
			
			iErrCode = Time::GetDateString (ppvMessage[i][GameEmpireMessages::TimeStamp].GetUTCTime(), pszDate);
			if (iErrCode != OK) {
				StrNCpy (pszDate, "The server is out of memory");
			}
			
			OutputText ("<table width=\"55%\"><tr><td align=\"left\">");
			
			if (!bSystem) {
				
				iErrCode = g_pGameEngine->DoesEmpireExist (pszSource, &bExists, &iSrcEmpireKey, NULL);
				if (iErrCode == OK && bExists) {
					
					iErrCode = g_pGameEngine->GetEmpireAlienKey (iSrcEmpireKey, &iAlienKey);
					if (iErrCode == OK) {
						
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
						
						iNumMessagesFromPeople ++;
						
						OutputText (" ");
					}
				}
			}
			
			
			OutputText ("On ");
			m_pHttpResponse->WriteText (pszDate);
			
			OutputText (", <strong>");
			
			m_pHttpResponse->WriteText (ppvMessage[i][GameEmpireMessages::Source].GetCharPtr());
			
			m_pHttpResponse->WriteText (pszMethod);
			OutputText ("<p><font face=\"");
			m_pHttpResponse->WriteText (DEFAULT_MESSAGE_FONT);
			OutputText ("\" size=\"");
			m_pHttpResponse->WriteText (DEFAULT_MESSAGE_FONT_SIZE);
			if (!bSystem) {
				OutputText ("\" color=\"#");
				m_pHttpResponse->WriteText (pszFontColor);
			}
			OutputText ("\">");
			WriteFormattedMessage (pszMessage);
			OutputText ("</font></td></tr></table><p>");
			
			g_pGameEngine->FreeData (ppvMessage[i]);
			
		}	// End empire loop
		
		if (iNumMessagesFromPeople > 0) {
			NotifyProfileLink();
		}
		
		delete [] ppvMessage;
	}
}


void HtmlRenderer::PostGamePageInformation() {
	
	uint64 iPasswordHash = GetGamePagePasswordHash();
	
	OutputText ("<input type=\"hidden\" name=\"EmpireKey\" value=\"");
	m_pHttpResponse->WriteText (m_iEmpireKey);
	OutputText ("\"><input type=\"hidden\" name=\"Password\" value=\"");
	m_pHttpResponse->WriteText (iPasswordHash);
	OutputText ("\"><input type=\"hidden\" name=\"GameClass\" value=\"");
	m_pHttpResponse->WriteText (m_iGameClass);
	OutputText ("\"><input type=\"hidden\" name=\"GameNumber\" value=\"");
	m_pHttpResponse->WriteText (m_iGameNumber);
	OutputText ("\"><input type=\"hidden\" name=\"Updates\" value=\"");
	m_pHttpResponse->WriteText (m_iNumNewUpdates);
	OutputText ("\"><input type=\"hidden\" name=\"Auto\" value=\"0\">");
}

void HtmlRenderer::CloseGamePage() {
	
	// Unlock the game by decrementing the thread count
	g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);
	
	OutputText ("<p>");
	WriteSeparatorString (m_iSeparatorKey);
	OutputText ("<p><strong><font size=\"3\">");
	
	if (m_bRepeatedButtons) {
		WriteGameButtons();
		OutputText ("<p>");
	}
	
	Variant vAdminEmail;
	int iErrCode = g_pGameEngine->GetAdministratorEmailAddress (&vAdminEmail);
	if (iErrCode == OK) {
		
		OutputText ("<p><strong>Contact <a href=\"mailto:");
		m_pHttpResponse->WriteText (vAdminEmail.GetCharPtr());
		OutputText ("\">root</a> if you have problems or suggestions</strong><p>");
		
	} else {
		
		OutputText ("<p>Could not read the administrator's email address<p>");
	}
	
	WriteButton (BID_SERVERNEWS);
	WriteButton (BID_SERVERRULES);
	WriteButton (BID_FAQ);
	
	MilliSeconds msTime = GetTimerCount();
	
	OnPageRender (msTime);
	
	OutputText ("<p>");
	WriteVersionString();
	OutputText ("<br>Script time: ");
	m_pHttpResponse->WriteText ((int) msTime);
	OutputText (" ms</font></strong></center></form></body></html>");
}

void HtmlRenderer::GetAlienButtonString (int iAlienKey, int iEmpireKey, bool bBorder, int iPlanetKey, 
										 int iProxyKey, const char* pszAlt, String* pstrAlienButtonString) {
	
	*pstrAlienButtonString = "<input type=\"image\" border=\"";
	*pstrAlienButtonString += (bBorder ? 1:0);
	*pstrAlienButtonString += "\" src=\"" BASE_RESOURCE_DIR;
	
	if (iAlienKey == NO_KEY) {
		*pstrAlienButtonString += BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME;
		*pstrAlienButtonString += iEmpireKey;
	} else {
		*pstrAlienButtonString += BASE_ALIEN_DIR ALIEN_NAME;
		*pstrAlienButtonString += iAlienKey;
	}
	
	*pstrAlienButtonString += DEFAULT_IMAGE_EXTENSION "\" name=\"Planet";
	*pstrAlienButtonString += iPlanetKey;
	*pstrAlienButtonString += ".";
	*pstrAlienButtonString += iProxyKey;
	
	*pstrAlienButtonString += "\"";
	
	if (!String::IsBlank (pszAlt)) {
		
		*pstrAlienButtonString += " alt=\"";
		*pstrAlienButtonString += pszAlt;
		*pstrAlienButtonString += "\"";
	}
	
	*pstrAlienButtonString += ">";
}

void HtmlRenderer::WriteAlienButtonString (int iAlienKey, bool bBorder, 
										   const char* pszAuthorName) {
	
	OutputText ("<input type=\"image\" border=\"");
	m_pHttpResponse->WriteText (bBorder ? 1:0);
	OutputText ("\" src=\"" BASE_RESOURCE_DIR);
	OutputText (BASE_ALIEN_DIR ALIEN_NAME);
	m_pHttpResponse->WriteText (iAlienKey);
	OutputText (DEFAULT_IMAGE_EXTENSION "\" name=\"Alien");
	m_pHttpResponse->WriteText (iAlienKey);
	OutputText ("\" alt=\"Alien ");
	m_pHttpResponse->WriteText (iAlienKey);
	OutputText (" by ");
	m_pHttpResponse->WriteText (pszAuthorName);
	OutputText ("\">");
}

void HtmlRenderer::GetLivePlanetButtonString (int iLivePlanetKey, int iPlanetKey, int iProxyKey, 
											  const char* pszAlt, String* pstrLivePlanet) {
	
	switch (iLivePlanetKey) {
		
	case NULL_THEME:
		
		*pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"" \
			BASE_RESOURCE_DIR LIVE_PLANET_NAME "\" name=\"Planet";
		break;
		
	case ALTERNATIVE_PATH:
		
		*pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"";
		*pstrLivePlanet += m_vLocalPath.GetCharPtr();
		*pstrLivePlanet += "/" LIVE_PLANET_NAME "\" name=\"Planet";
		break;
		
	default:
		
		*pstrLivePlanet = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR;
		*pstrLivePlanet += iLivePlanetKey;
		*pstrLivePlanet += "/" LIVE_PLANET_NAME "\" name=\"Planet";
		break;
	}
	
	*pstrLivePlanet += iPlanetKey;
	*pstrLivePlanet += ".";
	*pstrLivePlanet += iProxyKey;
	*pstrLivePlanet += "\"";
	
	if (!String::IsBlank (pszAlt)) {
		
		*pstrLivePlanet += " alt=\"";
		*pstrLivePlanet += pszAlt;
		*pstrLivePlanet += "\"";
	}
	
	*pstrLivePlanet += ">";
}

void HtmlRenderer::GetDeadPlanetButtonString (int iDeadPlanetKey, int iPlanetKey, int iProxyKey, 
											  const char* pszAlt, String* pstrDeadPlanet) {
	
	switch (iDeadPlanetKey) {	
		
	case NULL_THEME:
		
		*pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"" \
			BASE_RESOURCE_DIR DEAD_PLANET_NAME "\" name=\"Planet";
		break;
		
	case ALTERNATIVE_PATH:
		
		*pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"";
		*pstrDeadPlanet += m_vLocalPath.GetCharPtr();
		*pstrDeadPlanet += "/" DEAD_PLANET_NAME "\" name=\"Planet";
		break;
		
	default:
		
		*pstrDeadPlanet = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR;
		*pstrDeadPlanet += iDeadPlanetKey;
		*pstrDeadPlanet += "/" DEAD_PLANET_NAME "\" name=\"Planet";
		break;
	}
	
	*pstrDeadPlanet += iPlanetKey;
	*pstrDeadPlanet += ".";
	*pstrDeadPlanet += iProxyKey;
	*pstrDeadPlanet += "\"";
	
	if (!String::IsBlank (pszAlt)) {
		
		*pstrDeadPlanet += " alt=\"";
		*pstrDeadPlanet += pszAlt;
		*pstrDeadPlanet += "\"";
	}
	
	*pstrDeadPlanet += ">";
}

void HtmlRenderer::GetIndependentPlanetButtonString (int iPlanetKey, int iProxyKey, const char* pszAlt, 
													 String* pstrPlanetString) {
	
	*pstrPlanetString = "<input type=\"image\" border=\"0\" src=\"" BASE_RESOURCE_DIR INDEPENDENT_PLANET_NAME "\" name=\"Planet";
	*pstrPlanetString += iPlanetKey;
	*pstrPlanetString += ".";
	*pstrPlanetString += iProxyKey;
	*pstrPlanetString += "\"";
	
	if (!String::IsBlank (pszAlt)) {
		
		*pstrPlanetString += " alt=\"";
		*pstrPlanetString += pszAlt;
		*pstrPlanetString += "\"";
	}
	
	*pstrPlanetString += ">";
}

void HtmlRenderer::WriteLivePlanetString (int iLivePlanetKey) {
	
	switch (iLivePlanetKey) {
		
	case NULL_THEME:
		
		OutputText ("<img src=\"" BASE_RESOURCE_DIR LIVE_PLANET_NAME "\">");
		break;
		
	case ALTERNATIVE_PATH:
		
		OutputText ("<img src=\"");
		m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
		OutputText ("/" LIVE_PLANET_NAME "\">");
		break;
		
	default:
		
		OutputText ("<img src=\"" BASE_RESOURCE_DIR);
		m_pHttpResponse->WriteText (iLivePlanetKey);
		OutputText ("/" LIVE_PLANET_NAME "\">");
		break;
	}
}

void HtmlRenderer::WriteDeadPlanetString (int iDeadPlanetKey) {
	
	switch (iDeadPlanetKey) {
		
	case NULL_THEME:
		
		OutputText ("<img src=\"" BASE_RESOURCE_DIR DEAD_PLANET_NAME "\">");
		break;
		
	case ALTERNATIVE_PATH:
		
		OutputText ("<img src=\"");
		m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
		OutputText ("/" DEAD_PLANET_NAME "\">");
		break;
		
	default:
		
		OutputText ("<img src=\"" BASE_RESOURCE_DIR);
		m_pHttpResponse->WriteText (iDeadPlanetKey);
		OutputText ("/" DEAD_PLANET_NAME "\">");
		break;
	}
}

void HtmlRenderer::WriteIndependentPlanetString() {
	
	OutputText ("<img src=\"" BASE_RESOURCE_DIR INDEPENDENT_PLANET_NAME "\">");
}

int HtmlRenderer::WriteUpClosePlanetString (int iEmpireKey, int iPlanetKey, int iProxyPlanetKey, 
											int iLivePlanetKey, int iDeadPlanetKey, 
											int iPlanetCounter, bool bVisibleBuilds, int iGoodAg, int iBadAg, 
											int iGoodMin, int iBadMin, int iGoodFuel, int iBadFuel, 
											float fEmpireAgRatio, bool bIndependence, 
											bool bAdmin, bool bSpectator, void** ppPlanetData, 
											bool* pbOurPlanet) {
	
	bool bMapColoring = !bAdmin && !bSpectator;
	
	int iErrCode = OK, i, j;
	
	const char* pszTableColor = m_vTableColor.GetCharPtr();
	
	String strFilter;
	
	if (!(m_iGameState & STARTED)) {
		
		// Lay down the default "planet lost in outer space" table
		OutputText ("<tr><th>&nbsp;</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Name</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Location</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Owner</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Min</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Fuel</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Ag</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Pop</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Max Pop</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">Next Pop</th><th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\" align=\"left\">Jumps</th></tr><tr><td align=\"center\">");
		
		WriteProfileAlienString (
			m_iAlienKey,
			iEmpireKey,
			m_vEmpireName.GetCharPtr(),
			0, 
			"ProfileLink",
			"View your profile",
			false,
			false
			);
		
		OutputText ("</td><td align=\"center\"><strong>");
		m_pHttpResponse->WriteText (m_vEmpireName.GetCharPtr());	// Name
		OutputText ("</strong></td><td align=\"center\">None yet</td><td align=\"center\"><strong>");
		m_pHttpResponse->WriteText (m_vEmpireName.GetCharPtr());
		OutputText ("</strong></td><td align=\"center\"><font color=\"");
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\">0</font></td><td align=\"center\"><font color=\"");
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\">0</font></td><td align=\"left\">None yet</td></tr>");
		
		return OK;
	}
	
	OutputText ("<tr><th>&nbsp;</th><th align=\"left\" bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Name</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Location</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Owner</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Min</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Fuel</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Ag</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Pop</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Max Pop</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Next Pop</th><th bgcolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\">Jumps</th></tr><tr><td align=\"center\">");
	
	int iData, iAlienKey, iOwner = *((int*) ppPlanetData[GameMap::Owner]);
	int iAnnihilated = *((int*) ppPlanetData[GameMap::Annihilated]);
	
	String strPlanet;
	int iWeOffer, iTheyOffer, iCurrent;
	
	Variant vEmpireName;
	
	if (iOwner == SYSTEM) {
		
		iCurrent = TRUCE;
		
		if (iAnnihilated == 0) {
			WriteLivePlanetString (iLivePlanetKey);
		} else {
			WriteDeadPlanetString (iDeadPlanetKey);
		}
		
	} else {
		
		if (iOwner == INDEPENDENT) {
			
			iCurrent = bMapColoring ? WAR : TRUCE;
			WriteIndependentPlanetString();
			
		} else {
			
			if (iOwner == iEmpireKey) {
				
				iCurrent = bMapColoring ? ALLIANCE : TRUCE;
				
				WriteProfileAlienString (
					m_iAlienKey,
					iEmpireKey,
					m_vEmpireName.GetCharPtr(),
					0, 
					"ProfileLink",
					"View your profile",
					false,
					false
					);
				
			} else {
				
				if (!bMapColoring) {
					iCurrent = TRUCE;
				} else {
					
					iErrCode = g_pGameEngine->GetDiplomaticStatus (
						m_iGameClass, 
						m_iGameNumber, 
						iEmpireKey, 
						iOwner, 
						&iWeOffer, 
						&iTheyOffer, 
						&iCurrent
						);
					
					if (iErrCode == ERROR_DATA_NOT_FOUND) {
						iCurrent = WAR;
					}
					
					else if (iErrCode != OK) {
						Assert (false);
						return iErrCode;
					}
				}
				
				iErrCode = g_pGameEngine->GetEmpireAlienKey (iOwner, &iAlienKey);
				if (iErrCode != OK) {
					Assert (false);
					return iErrCode;
				}
				
				iErrCode = g_pGameEngine->GetEmpireName (iOwner, &vEmpireName);
				if (iErrCode != OK) {
					Assert (false);
					return iErrCode;
				}
				
				char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
				sprintf (pszProfile, "View the profile of %s", vEmpireName.GetCharPtr());
				
				WriteProfileAlienString (
					iAlienKey,
					iOwner,
					vEmpireName.GetCharPtr(),
					0, 
					"ProfileLink",
					pszProfile,
					false,
					true
					);
				
				if (!m_bNotifiedProfileLink) {
					NotifyProfileLink();
					m_bNotifiedProfileLink = true;
				}
			}
		}
	}
	
	OutputText ("</td>");
	
	if (HTMLFilter ((char*) ppPlanetData[GameMap::Name], &strFilter, 0, false) != OK) {
		return ERROR_OUT_OF_MEMORY;
	}
	
	if (iOwner != SYSTEM && iOwner == iEmpireKey && !bAdmin) {
		
		OutputText ("<td><input type=\"text\" size=\"15\" maxlength=\"");
		m_pHttpResponse->WriteText (MAX_PLANET_NAME_LENGTH);
		OutputText ("\" name=\"NewPlanetName");
		m_pHttpResponse->WriteText (iPlanetCounter);
		OutputText ("\"value=\"");
		m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
		OutputText ("\"><input type=\"hidden\" name=\"KeyPlanet");
		m_pHttpResponse->WriteText (iPlanetCounter);
		OutputText ("\" value=\"");
		m_pHttpResponse->WriteText (iPlanetKey);
		OutputText ("\"><input type=\"hidden\" name=\"OldPlanetName");
		m_pHttpResponse->WriteText (iPlanetCounter);
		OutputText ("\" value=\"");
		m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
		OutputText ("\"><input type=\"hidden\" name=\"OldMaxPop");
		m_pHttpResponse->WriteText (iPlanetCounter);
		OutputText ("\" value=\"");
		m_pHttpResponse->WriteText (*((int*) ppPlanetData[GameMap::MaxPop]));
		OutputText ("\"></td>");
		
		*pbOurPlanet = true;
		
	} else {
		
		OutputText ("<td align=\"left\"><strong>");
		WriteStringByDiplomacy (strFilter.GetCharPtr(), iCurrent);
		OutputText ("</strong>");
		
		if (iAnnihilated != NOT_ANNIHILATED) {
			
			OutputText ("<br>(Quarantined <strong>");
			
			if (iAnnihilated == ANNIHILATED_FOREVER) {
				OutputText ("forever</strong>)");
			} else {
				
				m_pHttpResponse->WriteText (iAnnihilated);
				OutputText ("</strong> update");
				
				if (iAnnihilated == 1) {
					OutputText (")");
				} else {
					OutputText ("s)");
				}
			}
		}
		
		else if (*(int*) ppPlanetData[GameMap::HomeWorld] >= ROOT_KEY) {
			OutputText ("<br>(Surrendered)");
		}
		
		OutputText ("</td>");
		
		*pbOurPlanet = false;
	}
	
	// Coordinates
	OutputText ("<td align=\"center\">");
	WriteStringByDiplomacy ((char*) ppPlanetData[GameMap::Coordinates], iCurrent);
	OutputText ("</td><td align=\"center\">");
	
	// Owner name
	if (iOwner != SYSTEM) {
		
		Variant vEmpireName;
		const char* pszEmpireName;
		
		if (iOwner == iEmpireKey) {
			pszEmpireName = m_vEmpireName.GetCharPtr();
		} else {
			
			if (iOwner == INDEPENDENT) {
				pszEmpireName = INDEPENDENT_NAME;
			} else {
				
				iErrCode = g_pGameEngine->GetEmpireName (iOwner, &vEmpireName);
				if (iErrCode != OK) {
					Assert (false);
					return iErrCode;
				}
				
				pszEmpireName = vEmpireName.GetCharPtr();
			}
		}
		
		WriteStringByDiplomacy (pszEmpireName, iCurrent);
		
		OutputText ("</td>");
		
	} else {
		OutputText ("-</td>");
	}
	
	// Minerals
	OutputText ("<td align=\"center\">");
	
	iData = *((int*) ppPlanetData[GameMap::Minerals]);
	if (iData < iBadMin) {
		OutputText ("<font color=\"");
		m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		OutputText ("\">");
		m_pHttpResponse->WriteText (iData);
		OutputText ("</font>");
	} else {
		if (iData > iGoodMin) {
			OutputText ("<font color=\"");
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
			OutputText ("\">");
			m_pHttpResponse->WriteText (iData);
			OutputText ("</font>");
		} else {
			m_pHttpResponse->WriteText (iData);
		}
	}
	OutputText ("</td><td align=\"center\">");
	
	iData = *((int*) ppPlanetData[GameMap::Fuel]);
	if (iData < iBadFuel) {
		OutputText ("<font color=\"");
		m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		OutputText ("\">");
		m_pHttpResponse->WriteText (iData);
		OutputText ("</font>");
	} else {
		if (iData > iGoodFuel) {
			OutputText ("<font color=\"");
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
			OutputText ("\">");
			m_pHttpResponse->WriteText (iData);
			OutputText ("</font>");
		} else {
			m_pHttpResponse->WriteText (iData);
		}
	}
	OutputText ("</td><td align=\"center\">");
	
	int iAg = *((int*) ppPlanetData[GameMap::Ag]);
	if (iAg < iBadAg) {
		OutputText ("<font color=\"");
		m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		OutputText ("\">");
		m_pHttpResponse->WriteText (iAg);
		OutputText ("</font>");
	} else {
		if (iAg > iGoodAg) {
			OutputText ("<font color=\"");
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
			OutputText ("\">");
			m_pHttpResponse->WriteText (iAg);
			OutputText ("</font>");
		} else {
			m_pHttpResponse->WriteText (iAg); 
		}
	}
	OutputText ("</td><td align=\"center\">");
	
	iData = *((int*) ppPlanetData[GameMap::Pop]);
	if (iData == iAg) {
		OutputText ("<font color=\"");
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\">");
		m_pHttpResponse->WriteText (iData);
		OutputText ("</font>");
	} else {
		if (iData > iAg || iData == 0) {
			OutputText ("<font color=\"");
			m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
			OutputText ("\">");
			m_pHttpResponse->WriteText (iData);
			OutputText ("</font>");
		} else {
			m_pHttpResponse->WriteText (iData); 
		}
	}
	
	OutputText ("</td><td align=\"center\">");
	
	if (iOwner != SYSTEM && iOwner == iEmpireKey && !bAdmin) {
		
		OutputText ("<input type=\"text\" size=\"4\" maxlength=\"4\" name=\"NewMaxPop");
		m_pHttpResponse->WriteText (iPlanetCounter);
		OutputText ("\"value=\"");
		m_pHttpResponse->WriteText (*((int*) ppPlanetData[GameMap::MaxPop]));
		OutputText ("\">");
		
	} else {
		
		if (iOwner != SYSTEM && bAdmin) {
			m_pHttpResponse->WriteText (*((int*) ppPlanetData[GameMap::MaxPop]));
		} else {
			OutputText ("-");
		}
	}
	OutputText ("</td><td align=\"center\">");
	
	if (iOwner == SYSTEM) {
		OutputText ("0");
	}
	
	else if (iOwner == iEmpireKey || bAdmin) {
		
		int iCost = *((int*) ppPlanetData[GameMap::PopLostToColonies]);
		int iPop  = *((int*) ppPlanetData[GameMap::Pop]);
		
		Assert (iCost >= 0 && iCost <= iPop);
		
		int iNextPop = g_pGameEngine->GetNextPopulation (
			iPop - iCost,
			fEmpireAgRatio
			);
		
		if (iNextPop > *((int*) ppPlanetData[GameMap::MaxPop])) {
			iNextPop = *((int*) ppPlanetData[GameMap::MaxPop]);
		}
		
		iData = iNextPop;
		if (iData == iAg) {
			OutputText ("<font color=\"");
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
			OutputText ("\">");
			m_pHttpResponse->WriteText (iData);
			OutputText ("</font>");
		} else {
			if (iData > iAg || iData == 0) {
				OutputText ("<font color=\"");
				m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
				OutputText ("\">");
				m_pHttpResponse->WriteText (iData);
				OutputText ("</font>");
			} else {
				m_pHttpResponse->WriteText (iData); 
			}
		}
	}
	
	else {
		
		OutputText ("-");
	}
	
	OutputText ("</td><td align=\"center\">");
	
	int iX, iY;
	g_pGameEngine->GetCoordinates ((char*) ppPlanetData[GameMap::Coordinates], &iX, &iY);
	
	int iLink = *((int*) ppPlanetData[GameMap::Link]);
	bool bLinkNorth = (iLink & LINK_NORTH) != 0;
	bool bLinkEast  = (iLink & LINK_EAST) != 0;
	bool bLinkSouth = (iLink & LINK_SOUTH) != 0;
	bool bLinkWest  = (iLink & LINK_WEST) != 0;
	
	int iNumJumps = (bLinkNorth ? 1:0) + (bLinkEast ? 1:0) + (bLinkSouth ? 1:0) + (bLinkWest ? 1:0);
	
	if (iNumJumps > 0) {
		
		int piJumpX [NUM_CARDINAL_POINTS];
		int piJumpY [NUM_CARDINAL_POINTS];
		int piCardinalPoint [NUM_CARDINAL_POINTS];
		
		int iNeighbourKey, iCounter = 0;
		
		if (bLinkNorth) {
			piJumpX[iCounter] = iX;
			piJumpY[iCounter] = iY + 1;
			piCardinalPoint [iCounter] = NORTH;
			iCounter ++;
		}
		
		if (bLinkEast) {
			piJumpX[iCounter] = iX + 1;
			piJumpY[iCounter] = iY;
			piCardinalPoint [iCounter] = EAST;
			iCounter ++;
		}
		
		if (bLinkSouth) {
			piJumpX[iCounter] = iX;
			piJumpY[iCounter] = iY - 1;
			piCardinalPoint [iCounter] = SOUTH;
			iCounter ++;
		}
		
		if (bLinkWest) {
			piJumpX[iCounter] = iX - 1;
			piJumpY[iCounter] = iY;
			piCardinalPoint [iCounter] = WEST;
			iCounter ++;
		}
		
		Assert (iCounter == iNumJumps);
		
		Variant vPlanetName;
		for (i = 0; i < iNumJumps; i ++) {
			
			// Get neighbouring planet's key
			iErrCode = g_pGameEngine->GetNeighbourPlanetKey (
				m_iGameClass, 
				m_iGameNumber, 
				iPlanetKey, 
				piCardinalPoint[i], 
				&iNeighbourKey
				);
			
			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}
			
			Assert (iNeighbourKey != NO_KEY);
			
			if (!bAdmin) {
				
				iErrCode = g_pGameEngine->GetPlanetNameWithSecurity (
					m_iGameClass, 
					m_iGameNumber, 
					iEmpireKey, 
					iNeighbourKey, 
					&vPlanetName
					);

			} else {
				
				iErrCode = g_pGameEngine->GetPlanetName (
					m_iGameClass, 
					m_iGameNumber, 
					iNeighbourKey, 
					&vPlanetName
					);
			}
			
			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}
			
			OutputText ("<strong>");
			m_pHttpResponse->WriteText (CARDINAL_STRING[piCardinalPoint[i]]);
			OutputText ("</strong>: ");
			m_pHttpResponse->WriteText (vPlanetName.GetCharPtr());
			OutputText (" (");
			m_pHttpResponse->WriteText (piJumpX[i]);
			OutputText (",");
			m_pHttpResponse->WriteText (piJumpY[i]);
			OutputText (")<br>");
		}
		
	} else {
		OutputText ("<strong>None</strong>");
	}
	
	OutputText ("</td></tr>");
	
	int iTotalNumShips = 
		*((int*) ppPlanetData[GameMap::NumUncloakedShips]) + 
		*((int*) ppPlanetData[GameMap::NumCloakedShips]) + 
		*((int*) ppPlanetData[GameMap::NumUncloakedBuildShips]) + 
		*((int*) ppPlanetData[GameMap::NumCloakedBuildShips]);
	
	if (iTotalNumShips > 0) {
		
		int* piOwnerData;
		iErrCode = g_pGameEngine->GetPlanetShipOwnerData (
			m_iGameClass, 
			m_iGameNumber, 
			iEmpireKey, 
			iPlanetKey, 
			iProxyPlanetKey, 
			iTotalNumShips, 
			bVisibleBuilds, 
			bIndependence, 
			&piOwnerData
			);
		
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}
		
		int iNumOwners = piOwnerData[0];
		
		if (iNumOwners > 0) {
			
			m_pHttpResponse->WriteText (
				"<tr><td></td><td></td><td colspan=\"10\"><table><tr><td></td><td bgcolor=\""
				);
			
			m_pHttpResponse->WriteText (pszTableColor);
			OutputText ("\" align=\"center\"><strong>Empire</strong></td>");
			
			bool pbTech [NUM_SHIP_TYPES];
			ENUMERATE_SHIP_TYPES (i) {
				pbTech[i] = false;
			}
			int** ppiNumTechsTable = (int**) StackAlloc (iNumOwners * sizeof (int*));
			int* piTemp = (int*) StackAlloc (iNumOwners * NUM_SHIP_TYPES * sizeof (int));
			for (i = 0; i < iNumOwners; i ++) {
				ppiNumTechsTable[i] = &(piTemp [i * NUM_SHIP_TYPES]);
				ENUMERATE_SHIP_TYPES (j) {
					ppiNumTechsTable[i][j] = 0;
				}
			}
			int* piOwnerKey = (int*) StackAlloc (iNumOwners * sizeof (int));
			
			int iBase = 1, iNumOwnerTechs, iType;
			
			for (i = 0; i < iNumOwners; i ++) {
				
				piOwnerKey[i] = piOwnerData [iBase];
				iNumOwnerTechs = piOwnerData [iBase + 1];
				
				for (j = 0; j < iNumOwnerTechs; j ++) {
					iType = piOwnerData [iBase + 2 + j * 2];
					pbTech [iType] = true;
					ppiNumTechsTable [i][iType] = piOwnerData [iBase + 3 + j * 2];
				}
				
				iBase += 2 + 2 * iNumOwnerTechs;
			}
			
			ENUMERATE_SHIP_TYPES (i) {
				if (pbTech[i]) {
					OutputText ("<td bgcolor=\"");
					m_pHttpResponse->WriteText (pszTableColor);
					OutputText ("\" align=\"center\"><strong>");
					m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
					OutputText ("</strong></td>");
				}
			}
			OutputText ("</tr>");
			
			int iDip, iWeOffer, iTheyOffer;
			
			Variant vEmpireName;
			const char* pszEmpireName;
			
			for (i = 0; i < iNumOwners; i ++) {
				
				OutputText ("<tr><td align=\"right\">");
				
				if (piOwnerKey[i] != INDEPENDENT) {
					
					if (piOwnerKey[i] == iEmpireKey) {
						
						WriteProfileAlienString (
							m_iAlienKey,
							iEmpireKey,
							m_vEmpireName.GetCharPtr(),
							0, 
							"ProfileLink",
							"View your profile",
							false,
							false
							);
						
					} else {
						
						iErrCode = g_pGameEngine->GetEmpireAlienKey (piOwnerKey[i], &iAlienKey);
						if (iErrCode != OK) {
							Assert (false);
							delete [] piOwnerData;
							return iErrCode;
						}
						
						iErrCode = g_pGameEngine->GetEmpireName (piOwnerKey[i], &vEmpireName);
						if (iErrCode != OK) {
							Assert (false);
							delete [] piOwnerData;
							return iErrCode;
						}
						
						char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
						sprintf (pszProfile, "View the profile of %s", vEmpireName.GetCharPtr());
						
						WriteProfileAlienString (
							iAlienKey,
							piOwnerKey[i],
							vEmpireName.GetCharPtr(),
							0, 
							"ProfileLink",
							pszProfile,
							false,
							true
							);
						
						if (!m_bNotifiedProfileLink) {
							NotifyProfileLink();
							m_bNotifiedProfileLink = true;
						}
					}
					
				} else {
					
					WriteIndependentPlanetString();
				}
				
				OutputText ("</td>");
				
				if (piOwnerKey[i] == iEmpireKey) {
					
					iDip = ALLIANCE;
					pszEmpireName = m_vEmpireName.GetCharPtr();
					
				} else {
					
					if (piOwnerKey[i] == INDEPENDENT) {
						iDip = WAR;
						pszEmpireName = INDEPENDENT_NAME;
					} else {
						
						iErrCode = g_pGameEngine->GetEmpireName (piOwnerKey[i], &vEmpireName);
						if (iErrCode != OK) {
							Assert (false);
							delete [] piOwnerData;
							return iErrCode;
						}
						
						pszEmpireName = vEmpireName.GetCharPtr();
						
						if (!bMapColoring) {
							iDip = TRUCE;
						} else {
							
							iErrCode = g_pGameEngine->GetDiplomaticStatus (
								m_iGameClass, 
								m_iGameNumber, 
								iEmpireKey, 
								piOwnerKey[i], 
								&iWeOffer, 
								&iTheyOffer, 
								&iDip
								);
							
							// Error means they haven't met
							if (iErrCode == ERROR_DATA_NOT_FOUND) {
								iErrCode = OK;
								iDip = WAR;
							}
							
							else if (iErrCode != OK) {
								Assert (false);
								delete [] piOwnerData;
								return iErrCode;
							}
						}
					}
				}
				
				if (iDip == WAR) {
					
					OutputText ("<td align=\"center\"><font color=\"");
					m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
					OutputText ("\"><strong>");
					m_pHttpResponse->WriteText (pszEmpireName);
					OutputText ("</strong></font></td>");
					
					ENUMERATE_SHIP_TYPES (j) {
						if (pbTech[j]) {
							OutputText ("<td align=\"center\"><font color=\"");
							m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
							OutputText ("\">");
							m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
							OutputText ("</font></td>");
						}
					}
					
				} else {
					
					if (iDip == ALLIANCE) {
						OutputText ("<td align=\"center\"><font color=\"");
						m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
						OutputText ("\"><strong>");
						m_pHttpResponse->WriteText (pszEmpireName);
						OutputText ("</strong></font></td>");
						
						ENUMERATE_SHIP_TYPES (j) {
							if (pbTech[j]) {
								OutputText ("<td align=\"center\"><font color=\"");
								m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
								OutputText ("\">");
								m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
								OutputText ("</font></td>");
							}
						}
						
					} else {
						
						OutputText ("<td align=\"center\"><strong>");
						m_pHttpResponse->WriteText (pszEmpireName);
						OutputText ("</strong></td>");
						
						ENUMERATE_SHIP_TYPES (j) {
							if (pbTech[j]) {
								OutputText ("<td align=\"center\">");
								m_pHttpResponse->WriteText (ppiNumTechsTable[i][j]);
								OutputText ("</td>");
							}
						}
					}
				}
				OutputText ("</tr>");
			}
			OutputText ("</table></td></tr>");
		}
		
		delete [] piOwnerData;
	}
	
	return iErrCode;
}

void HtmlRenderer::WriteRatiosString (int* piBR, float* pfMaintRatio) {
	
	
	float fFuelRatio, fTechLevel, fTechDev, fNextMaintRatio;
	
	float fNextFuelRatio, fNextTechLevel, fNextTechDev;
	int iNextBR;
	
	float fMaxTechDev;
	
	if (g_pGameEngine->GetGameClassMaxTechIncrease (m_iGameClass, &fMaxTechDev) != OK ||
		
		
		g_pGameEngine->GetShipRatios (m_iGameClass, m_iGameNumber, m_iEmpireKey, pfMaintRatio, 
		&fFuelRatio, &fTechLevel, &fTechDev, piBR) != OK
		
		||
		
		g_pGameEngine->GetNextShipRatios (m_iGameClass, m_iGameNumber, m_iEmpireKey, &fNextMaintRatio, 
		&fNextFuelRatio, &fNextTechLevel, &fNextTechDev, &iNextBR) != OK
		
		) {
		
		OutputText ("GameEngine::GetShipRatios() failed. Please contact the administrator");
		
	} else {
		
		OutputText ("<p></center><table border=\"0\" width=\"85%\">"\
			"<tr><td align=\"right\">Maintenance Ratio: <strong><font color=\"");
		
		if (*pfMaintRatio >= (float) 1.0) {
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		} else {
			m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		}
		
		OutputText ("\">");
		m_pHttpResponse->WriteText (*pfMaintRatio);
		
		OutputText ("</font></strong></td><td align=\"right\">Fuel Ratio: <strong><font color=\"");
		
		if (fFuelRatio >= (float) 1.0) {
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		} else {
			m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		}
		
		OutputText ("\">");
		m_pHttpResponse->WriteText (fFuelRatio);
		OutputText ("</font></strong></td><td align=\"right\">Tech Level: <strong>");
		m_pHttpResponse->WriteText (fTechLevel);
		OutputText (" (BR ");
		m_pHttpResponse->WriteText (*piBR);
		OutputText (")</strong></td><td align=\"right\">Tech Increase: <strong>");
		
		if (fTechDev < (float) 0.0) {
			OutputText ("<font color=\"");
			m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
			OutputText ("\">");
			m_pHttpResponse->WriteText (fTechDev);
			OutputText ("</font>");
		} else {
			
			if (fTechDev == fMaxTechDev) {
				OutputText ("<font color=\"");
				m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
				OutputText ("\">");
				m_pHttpResponse->WriteText (fTechDev);
				OutputText ("</font>");
			} else {
				m_pHttpResponse->WriteText (fTechDev);
			}
		}		 
		
		OutputText ("</strong></td></tr>");
		
		// Next ratios
		OutputText ("<tr><td align=\"right\">Next: <strong><font color=\"");
		
		if (fNextMaintRatio >= (float) 1.0) {
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		} else {
			m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		}
		
		OutputText ("\">");
		m_pHttpResponse->WriteText (fNextMaintRatio);
		
		OutputText ("</font></strong></td><td align=\"right\">Next: <strong><font color=\"");
		
		if (fNextFuelRatio >= (float) 1.0) {
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		} else {
			m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		}
		
		OutputText ("\">");
		m_pHttpResponse->WriteText (fNextFuelRatio);
		
		OutputText ("</font></strong></td><td align=\"right\">Next: <strong>");
		
		if (iNextBR > *piBR) {
			
			OutputText ("<font color=\"");
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
			OutputText ("\">");
			
			m_pHttpResponse->WriteText (fNextTechLevel);
			OutputText (" (BR ");
			m_pHttpResponse->WriteText (iNextBR);
			OutputText (")");
			
			OutputText ("</font>");
			
		} else {
			
			m_pHttpResponse->WriteText (fNextTechLevel);
			OutputText (" (BR ");
			m_pHttpResponse->WriteText (iNextBR);
			OutputText (")");
		}
		
		OutputText ("</strong></td><td align=\"right\">Next: <strong>");
		
		if (fNextTechDev < (float) 0.0) {
			OutputText ("<font color=\"");
			m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
			OutputText ("\">");
			m_pHttpResponse->WriteText (fNextTechDev);
			OutputText ("</font>");
		} else {
			
			if (fNextTechDev == fMaxTechDev) {
				OutputText ("<font color=\"");
				m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
				OutputText ("\">");
				m_pHttpResponse->WriteText (fNextTechDev);
				OutputText ("</font>");
			} else {
				m_pHttpResponse->WriteText (fNextTechDev);
			}
		}		 
		
		OutputText ("</table><center>");
	}
}


int HtmlRenderer::WriteShip (const GameConfiguration& gcConfig, IReadTable* pShips, int i, int iShipKey, int iBR,
							 float fMaintRatio, int iShipLoc, int& iLastLocation, int& iLastX, int& iLastY, 
							 Variant& vPlanetName, bool bFleet) {
	
	int iErrCode, * piOrderKey, iNumOrders, iSelectedOrder, iType, j;
	
	String* pstrOrderText;
	
	void** ppData;
	
	float fCurrentBR, fMaxBR, fNextBR;
	
	iErrCode = pShips->ReadRow (iShipKey, &ppData);	
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}
	
	OutputText ("<tr><td align=\"center\"><input type=\"hidden\" name=\"ShipKey");
	m_pHttpResponse->WriteText (i);
	OutputText ("\" value=\"");
	m_pHttpResponse->WriteText (iShipKey);
	
	OutputText ("\"><input type=\"text\" name=\"ShipName");
	m_pHttpResponse->WriteText (i);
	
	String strHtml;
	iErrCode = HTMLFilter ((char*) ppData[GameEmpireShips::Name], &strHtml, 0, false);
	if (iErrCode != OK) {
		goto Cleanup;
	}
	
	OutputText ("\" value=\"");
	m_pHttpResponse->WriteText (strHtml);
	OutputText ("\" size=\"15\" maxlength=\"");
	m_pHttpResponse->WriteText (MAX_SHIP_NAME_LENGTH);
	OutputText ("\"><input type=\"hidden\" name=\"OldShipName");
	m_pHttpResponse->WriteText (i);
	OutputText ("\" value=\"");
	m_pHttpResponse->WriteText ((char*) ppData[GameEmpireShips::Name]);
	OutputText ("\"></td><td align=\"center\"><font color=\"");
	
	fCurrentBR = *((float*) ppData[GameEmpireShips::CurrentBR]);
	fMaxBR = *((float*) ppData[GameEmpireShips::MaxBR]);
	
	if (fCurrentBR < fMaxBR) {
		m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
	} else {
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
	}
	OutputText ("\">");
	m_pHttpResponse->WriteText (fCurrentBR);
	OutputText ("</font></td><td align=\"center\"><font color=\"");
	if (fMaxBR < (float) iBR) {
		m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
	} else {
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
	}
	OutputText ("\">");
	m_pHttpResponse->WriteText (fMaxBR);
	OutputText ("</font></td><td align=\"center\">");
	
	fNextBR = fMaintRatio * fCurrentBR; 
	
	OutputText ("<font color=\"");
	if (fNextBR < fMaxBR) {
		m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
		OutputText ("\">");
		m_pHttpResponse->WriteText (fNextBR);
	} else {
		m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
		OutputText ("\">");
		m_pHttpResponse->WriteText (fMaxBR);
	}
	OutputText ("</font></td>");
	
	if (!bFleet) {
		
		if (iShipLoc != iLastLocation) {
			
			iLastLocation = iShipLoc;
			
			iErrCode = g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, iLastLocation, &vPlanetName);
			if (iErrCode != OK) {
				goto Cleanup;
			}
			
			iErrCode = g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, iLastLocation, &iLastX, &iLastY);
			if (iErrCode != OK) {
				goto Cleanup;
			}
		}
		
		OutputText ("<td align=\"center\">");
		m_pHttpResponse->WriteText (vPlanetName.GetCharPtr());
		OutputText (" (");
		m_pHttpResponse->WriteText (iLastX);
		OutputText (",");
		m_pHttpResponse->WriteText (iLastY);
		OutputText (")</td>");
	}
	
	iType  = *((int*) ppData[GameEmpireShips::Type]);
	
	if (*((int*) ppData[GameEmpireShips::State]) & CLOAKED) {
		OutputText ("<td align=\"center\"><strike>");
		m_pHttpResponse->WriteText (SHIP_TYPE_STRING[iType]);
		OutputText ("</strike></td><td align=\"center\">");
	} else {
		OutputText ("<td align=\"center\">");
		m_pHttpResponse->WriteText (SHIP_TYPE_STRING[iType]);
		OutputText ("</td><td align=\"center\">");
	}
	
	// Get orders
	iErrCode = g_pGameEngine->GetShipOrders (
		m_iGameClass,
		m_iGameNumber,
		m_iEmpireKey,
		gcConfig,
		iShipKey,
		iType,
		fCurrentBR,
		fMaintRatio,
		iLastLocation,
		iLastX,
		iLastY,
		&piOrderKey,
		&pstrOrderText,
		&iNumOrders,
		&iSelectedOrder
		);
	
	if (iErrCode != OK) {
		goto Cleanup;
	}
	
	Assert (iNumOrders > 1);
	
	OutputText ("<select name=\"ShipOrder");
	m_pHttpResponse->WriteText (i);
	OutputText ("\">");
	
	for (j = 0; j < iNumOrders; j ++) {
		
		if (iSelectedOrder == piOrderKey[j] ||
			iSelectedOrder == GATE_SHIPS && 
			*((int*) ppData[GameEmpireShips::GateDestination]) == piOrderKey[j]) {
			OutputText ("<option selected value=\"");
			m_pHttpResponse->WriteText (piOrderKey[j]);
			OutputText ("\">");
			m_pHttpResponse->WriteText (pstrOrderText[j]);
		} else {
			OutputText ("<option value=\"");
			m_pHttpResponse->WriteText (piOrderKey[j]);
			OutputText ("\">");
			m_pHttpResponse->WriteText (pstrOrderText[j]);
		}
	}
	OutputText ("</select></td></tr><input type=\"hidden\" name=\"ShipSelectedOrder");
	m_pHttpResponse->WriteText (i);
	OutputText ("\" value=\"");
	m_pHttpResponse->WriteText (iSelectedOrder);
	OutputText ("\">");
	
	delete [] piOrderKey;
	delete [] pstrOrderText;
	
Cleanup:
	
	g_pGameEngine->FreeData (ppData);
	
	return iErrCode;
}

bool HtmlRenderer::ShipOrFleetNameFilter (const char* pszName) {
	
	if (pszName == NULL || *pszName == '\0') {
		return true;
	}
	
	// Check for commas
	if (strstr (pszName, ",") != NULL) {
		return false;
	}
	
	size_t i, stLen = strlen (pszName);
	
	for (i = 0; i < stLen; i ++) {
		
		if ((pszName[i] < FIRST_VALID_SHIP_OR_FLEET_NAME_CHAR ||
			pszName[i] > LAST_VALID_SHIP_OR_FLEET_CHAR)
			||
			pszName[i] == ','
			) {
			
			return false;
		}
	}
	
	return true;
}


int HtmlRenderer::GetGoodBadResourceLimits (int iGameClass, int iGameNumber, int* piGoodAg, int* piBadAg, int* piGoodMin,
											int* piBadMin, int* piGoodFuel, int* piBadFuel) {
	
	int iAg, iMin, iFuel;
	
	int iErrCode = g_pGameEngine->GetGameAveragePlanetResources (iGameClass, iGameNumber, &iAg, &iMin, &iFuel);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}
	
	*piGoodAg = (int) (iAg * 1.5);
	*piBadAg  = iAg / 2;
	
	*piGoodMin = (int) (iMin * 1.5);
	*piBadMin  = iAg / 2;
	
	*piGoodFuel = (int) (iFuel * 1.5);
	*piBadFuel  = iAg / 2;
	
	return OK;
}

int HtmlRenderer::RenderMap (int iGameClass, int iGameNumber, int iEmpireKey, bool bAdmin, 
							 const PartialMapInfo* pPartialMapInfo, bool bSpectators) {
	
	int iErrCode;
	
	Variant* pvPlanetKey = NULL, vOptions, * pvEmpireKey = NULL;
	
	int iMinX, iMaxX, iMinY, iMaxY, iNumJumps, iLivePlanetKey, iDeadPlanetKey,
		iMapMinX = 0, iMapMaxX = 0, iMapMinY = 0, iMapMaxY = 0;
	
	unsigned int iNumPlanets, * piPlanetKey = NULL, i, j, * piProxyKey = NULL;
	
	size_t stTemp;
	
	IDatabase* pDatabase = g_pGameEngine->GetDatabase();
	IReadTable* pGameMap = NULL, * pEmpireMap = NULL;
	
	GAME_MAP (strGameMap, iGameClass, iGameNumber);
	
	Variant pvEasyWayOut[9];
	unsigned int piEasyProxyKeys[9], iGridX, iGridY, iGridLocX, iGridLocY;

	int iHorzKey, iVertKey, iNumOwnShips, iNumOtherShips, iAlienKey, iAccountingNumOtherShips, iDiplomacyLevel,
		iCenterX = MAX_COORDINATE, iCenterY = MAX_COORDINATE;
	
	bool bPartialMapShortcut = false;
	
	String* pstrGrid = NULL, ** ppstrGrid, strPlanetString, strImage, strHorz, strVert, strFilter, strAltTag;
	
	char* pszHorz, * pszVert;
	
	void** ppPlanetData = NULL;
	int iNumUncloakedShips, iNumCloakedShips, iNumUncloakedBuildShips, iNumCloakedBuildShips;
	
	const char* pszColor;
	int iWeOffer, iTheyOffer, iCurrent, iX, iY, iOwner, iLink, iProxyKey, iPlanetKey, iOptions;
	bool bLinkNorth, bLinkEast, bLinkSouth, bLinkWest, bVisible, bIndependence, bSensitive, bMapColoring, 
		bShipColoring, bHighlightShips;
	
	unsigned int iNumEmpires;

	Assert (!(bSpectators && bAdmin));

	if (!bAdmin && !bSpectators) {

		iErrCode = g_pGameEngine->GetEmpireOptions (iGameClass, iGameNumber, iEmpireKey, &iOptions);
		if (iErrCode != OK) {
		
			bSensitive = bMapColoring = bShipColoring = bHighlightShips = false;
	
		} else {

			bSensitive = (iOptions & SENSITIVE_MAPS) != 0;
			bMapColoring = (iOptions & MAP_COLORING) != 0;
			bShipColoring = (iOptions & SHIP_MAP_COLORING) != 0;
			bHighlightShips = (iOptions & SHIP_MAP_HIGHLIGHTING) != 0;
		}
		
		// Get map geography information
		iErrCode = g_pGameEngine->GetMapLimits (
			iGameClass, 
			iGameNumber, 
			iEmpireKey, 
			&iMapMinX, 
			&iMapMaxX, 
			&iMapMinY, 
			&iMapMaxY
			);
		
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		if (pPartialMapInfo != NULL && 
			pPartialMapInfo->iCenterKey != PARTIAL_MAP_NATURAL_CENTER && 
			pPartialMapInfo->iXRadius == 1 && 
			pPartialMapInfo->iYRadius == 1
			) {
			
			bPartialMapShortcut = true;
			
			// Scan 8 surounding planets
			iErrCode = g_pGameEngine->GetVisitedSurroundingPlanetKeys (
				iGameClass, 
				iGameNumber, 
				iEmpireKey,
				pPartialMapInfo->iCenterKey,
				pvEasyWayOut,
				(int*) piEasyProxyKeys,
				(int*) &iNumPlanets,
				&iCenterX,
				&iCenterY,
				&iMinX, 
				&iMaxX, 
				&iMinY, 
				&iMaxY
				);
			
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			pvPlanetKey = pvEasyWayOut;
			piProxyKey = piEasyProxyKeys;
			
			Assert (iNumPlanets > 0);
			
		} else {
			
			iErrCode = g_pGameEngine->GetVisitedPlanetKeys (
				iGameClass, 
				iGameNumber, 
				iEmpireKey,
				&pvPlanetKey,
				(int**) &piProxyKey, 
				(int*) &iNumPlanets
				);
			
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			Assert (iNumPlanets > 0);
			
			iMinX = iMapMinX;
			iMaxX = iMapMaxX; 
			iMinY = iMapMinY;
			iMaxY = iMapMaxY;
		}
		
	} else {
		
		// Options
		iErrCode = g_pGameEngine->GetEmpireOptions (iEmpireKey, &iOptions);
		if (iErrCode != OK) {
		
			bSensitive = bMapColoring = bShipColoring = bHighlightShips = false;
	
		} else {

			bSensitive = (iOptions & SENSITIVE_MAPS) != 0;
			bMapColoring = false;
			bShipColoring = false;
			bHighlightShips = (iOptions & SHIP_MAP_HIGHLIGHTING) != 0;
		}

		// Get map geography information
		iErrCode = g_pGameEngine->GetMapLimits (
			iGameClass, 
			iGameNumber,
			&iMapMinX, 
			&iMapMaxX, 
			&iMapMinY, 
			&iMapMaxY
			);
		
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = pDatabase->GetAllKeys (strGameMap, &piPlanetKey, &iNumPlanets);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iMinX = iMapMinX;
		iMaxX = iMapMaxX; 
		iMinY = iMapMinY;
		iMaxY = iMapMaxY;
	}
	
	// Partial map menus
	if (pPartialMapInfo != NULL) {

		Assert (!bSpectators);
		
		if (!bPartialMapShortcut) {
			
			// Get radius data
			if (pPartialMapInfo->iCenterKey == PARTIAL_MAP_NATURAL_CENTER) {
				
				iCenterX = iMapMinX + (iMapMaxX - iMapMinX) / 2;
				iCenterY = iMapMinY + (iMapMaxY - iMapMinY) / 2;
				
			} else {
				
				iErrCode = g_pGameEngine->GetPlanetCoordinates (
					iGameClass,
					iGameNumber,
					pPartialMapInfo->iCenterKey,
					&iCenterX,
					&iCenterY
					);
				
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
			}
			
			if (pPartialMapInfo->iXRadius != PARTIAL_MAP_UNLIMITED_RADIUS) {
				iMinX = iCenterX - pPartialMapInfo->iXRadius;
				iMaxX = iCenterX + pPartialMapInfo->iXRadius;
				Assert (iMinX >= 0);
			}
			
			if (pPartialMapInfo->iYRadius != PARTIAL_MAP_UNLIMITED_RADIUS) {
				iMinY = iCenterY - pPartialMapInfo->iYRadius;
				iMaxY = iCenterY + pPartialMapInfo->iYRadius;
				Assert (iMinY >= 0);
			}
		}
		
		// Draw table
		if (!pPartialMapInfo->bDontShowPartialOptions) {
			
			String strName;
			unsigned int iValue, iMaxXRadius, iMaxYRadius;
			
			Assert (iCenterX != MAX_COORDINATE);
			Assert (iCenterY != MAX_COORDINATE);
			
			iMaxXRadius = max (iCenterX - iMapMinX, iMapMaxX - iCenterX);
			iMaxYRadius = max (iCenterY - iMapMinY, iMapMaxY - iCenterY);
			
			OutputText ("<p><table><tr><th bgcolor=\"#");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">Partial map center</th><th bgcolor=\"#");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">Partial map X radius</th><th bgcolor=\"#");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">Partial map Y radius</th></tr><tr><td><select name=\"Center\"><option");
			
			if (pPartialMapInfo->iCenterKey == PARTIAL_MAP_NATURAL_CENTER) {
				OutputText (" selected");
			}
			OutputText (" value=\"");
			m_pHttpResponse->WriteText (PARTIAL_MAP_NATURAL_CENTER);
			OutputText ("\">The map's natural center (");
			m_pHttpResponse->WriteText (iCenterX);
			OutputText (",");
			m_pHttpResponse->WriteText (iCenterY);
			OutputText (")</option>");
			
			for (i = 0; i < iNumPlanets; i ++) {
				
				OutputText ("<option");
				if (pPartialMapInfo->iCenterKey == (unsigned int) pvPlanetKey[i].GetInteger()) {
					OutputText (" selected");
				}
				OutputText (" value=\"");
				m_pHttpResponse->WriteText (pvPlanetKey[i].GetInteger());
				OutputText ("\">");
				
				iErrCode = g_pGameEngine->GetPlanetNameWithCoordinates (
					strGameMap, 
					pvPlanetKey[i].GetInteger(), 
					&strName
					);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				m_pHttpResponse->WriteText (strName);
				OutputText ("</option>");
			}
			OutputText ("</select></td><td><select name=\"iXRadius\"><option");
			
			if (pPartialMapInfo->iXRadius == PARTIAL_MAP_UNLIMITED_RADIUS) {
				OutputText (" selected");
			}
			
			OutputText (" value=\"");
			m_pHttpResponse->WriteText (PARTIAL_MAP_UNLIMITED_RADIUS);
			OutputText ("\">As large as possible</option>");
			
			for (iValue = 1; iValue <= iMaxXRadius; iValue ++) {
				
				OutputText ("<option");
				if (pPartialMapInfo->iXRadius == iValue) {
					OutputText (" selected");
				}
				OutputText (" value=\"");
				m_pHttpResponse->WriteText (iValue);
				OutputText ("\">");
				m_pHttpResponse->WriteText (iValue);
				OutputText ("</option>");
			}
			OutputText ("</select></td><td><select name=\"iYRadius\"><option");
			
			// Y radius
			if (pPartialMapInfo->iYRadius == PARTIAL_MAP_UNLIMITED_RADIUS) {
				OutputText (" selected");
			}
			
			OutputText (" value=\"");
			m_pHttpResponse->WriteText (PARTIAL_MAP_UNLIMITED_RADIUS);
			OutputText ("\">As large as possible</option>");
			
			for (iValue = 1; iValue <= iMaxYRadius; iValue ++) {
				
				OutputText ("<option");
				if (pPartialMapInfo->iYRadius == iValue) {
					OutputText (" selected");
				}
				OutputText (" value=\"");
				m_pHttpResponse->WriteText (iValue);
				OutputText ("\">");
				m_pHttpResponse->WriteText (iValue);
				OutputText ("</option>");
			}
			OutputText ("</select></td></tr></table>");
		}
	}
	
	OutputText ("<p>Click on a planet for a closer view:<p>");
	
	// We have the end points, so generate the grid
	iGridX = (iMaxX - iMinX + 1) * 3;
	iGridY = (iMaxY - iMinY + 1) * 3;
	
	// Allocate grid
	pstrGrid = new String [iGridX * iGridY];
	ppstrGrid = (String**) StackAlloc (iGridX * sizeof (String*));
	
	for (i = 0; i < iGridX; i ++) {
		ppstrGrid[i] = pstrGrid + i * iGridY;
	}
	
	if (m_iThemeKey == INDIVIDUAL_ELEMENTS) {
		
		iErrCode = g_pGameEngine->GetEmpireHorzKey (iEmpireKey, &iHorzKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetEmpireVertKey (iEmpireKey, &iVertKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetEmpirePlanetIcons (iEmpireKey, &iLivePlanetKey, &iDeadPlanetKey);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
		
	} else {
		
		iHorzKey = iVertKey = iLivePlanetKey = iDeadPlanetKey = m_iThemeKey;
	}
	
	iErrCode = GetHorzString (iHorzKey, &strHorz, false);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	iErrCode = GetVertString (iVertKey, &strVert, false);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	pszHorz = (char*) StackAlloc (strHorz.GetLength() + 100);
	pszVert = (char*) StackAlloc (strVert.GetLength() + 100);
	
	//sprintf (pszHorz, "<td align=\"center\">%s</td>", strHorz.GetCharPtr());
	//sprintf (pszVert, "<td align=\"center\">%s</td>", strVert.GetCharPtr());
	
	stTemp = sizeof ("<td align=\"center\">") - 1;
	
	strncpy (pszHorz, "<td align=\"center\">", stTemp);
	strncpy (pszVert, "<td align=\"center\">", stTemp);
	
	strncpy (pszHorz + stTemp, strHorz.GetCharPtr(), strHorz.GetLength());
	strncpy (pszVert + stTemp, strVert.GetCharPtr(), strVert.GetLength());
	
	strncpy (pszHorz + stTemp + strHorz.GetLength(), "</td>", sizeof ("</td>"));
	strncpy (pszVert + stTemp + strVert.GetLength(), "</td>", sizeof ("</td>"));
	
	iErrCode = pDatabase->ReadData (
		SYSTEM_GAMECLASS_DATA, 
		iGameClass, 
		SystemGameClassData::Options, 
		&vOptions
		);
	
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	bVisible = (vOptions.GetInteger() & VISIBLE_BUILDS) != 0;
	bIndependence = (vOptions.GetInteger() & INDEPENDENCE) != 0;
	
	// Open tables
	iErrCode = pDatabase->GetTableForReading (strGameMap, &pGameMap);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	if (!bAdmin && !bSpectators) {
		
		GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);
		
		iErrCode = pDatabase->GetTableForReading (strGameEmpireMap, &pEmpireMap);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}
	}
	
	// Loop through all planets, placing them on grid
	for (i = 0; i < iNumPlanets; i ++) {
		
		if (!bAdmin && !bSpectators) {
			
			iPlanetKey = pvPlanetKey[i].GetInteger();
			iProxyKey = piProxyKey[i];
			
			iErrCode = pGameMap->ReadRow (iPlanetKey, &ppPlanetData);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			g_pGameEngine->GetCoordinates ((char*) ppPlanetData[GameMap::Coordinates], &iX, &iY);
			
			// Partial map filtering
			if (pPartialMapInfo != NULL && (iX > iMaxX || iX < iMinX || iY > iMaxY || iY < iMinY)) {
				pDatabase->FreeData (ppPlanetData);
				ppPlanetData = NULL;
				continue;
			}
			
			iErrCode = pEmpireMap->ReadData (iProxyKey, GameEmpireMap::NumUncloakedShips, &iNumUncloakedShips);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = pEmpireMap->ReadData (iProxyKey, GameEmpireMap::NumCloakedShips, &iNumCloakedShips);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = pEmpireMap->ReadData (iProxyKey, GameEmpireMap::NumUncloakedBuildShips, &iNumUncloakedBuildShips);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = pEmpireMap->ReadData (iProxyKey, GameEmpireMap::NumCloakedBuildShips, &iNumCloakedBuildShips);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iNumOwnShips = iNumUncloakedShips + iNumCloakedShips + iNumUncloakedBuildShips + iNumCloakedBuildShips;
			
		} else {
			
			iPlanetKey = piPlanetKey[i];
			iProxyKey = 0;
			
			iErrCode = pGameMap->ReadRow (iPlanetKey, &ppPlanetData);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			g_pGameEngine->GetCoordinates ((char*) ppPlanetData[GameMap::Coordinates], &iX, &iY);
			
			iNumOwnShips = iNumUncloakedShips = iNumCloakedShips = iNumUncloakedBuildShips = 
				iNumCloakedBuildShips = 0;
		}
		
		// Main map loop
		iNumOtherShips = *((int*) ppPlanetData[GameMap::NumUncloakedShips]) - iNumUncloakedShips;
		if (bVisible || bAdmin) {
			iNumOtherShips += *((int*) ppPlanetData[GameMap::NumUncloakedBuildShips]) - iNumUncloakedBuildShips;
		}
		
		Assert (iNumOtherShips >= 0 && iNumOwnShips >= 0);
		
		iLink = *((int*) ppPlanetData[GameMap::Link]);
		bLinkNorth = (iLink & LINK_NORTH) != 0;
		bLinkEast  = (iLink & LINK_EAST) != 0;
		bLinkSouth = (iLink & LINK_SOUTH) != 0;
		bLinkWest  = (iLink & LINK_WEST) != 0;
		
		iNumJumps = (bLinkNorth ? 1:0) + (bLinkEast ? 1:0) + (bLinkSouth ? 1:0) + (bLinkWest ? 1:0);
		
		iOwner = *((int*) ppPlanetData[GameMap::Owner]);
		
		// Get grid coordinates		
		iGridLocX = (iX - iMinX) * 3 + 1;
		iGridLocY = (iMaxY - iY) * 3 + 1;
		
		Assert (iGridLocX >= 0 && iGridLocY >= 0 && iGridLocX < iGridX && iGridLocY < iGridY);
		
		if (bSensitive) {
			
			iErrCode = GetSensitiveMapText (
				iGameClass, 
				iGameNumber, 
				bAdmin ? SYSTEM : (bSpectators ? GUEST : iEmpireKey),
				iPlanetKey,
				iProxyKey,
				bVisible || bAdmin, 
				bIndependence,
				ppPlanetData,
				&strAltTag
				);
			
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}
		
		// Get planet string		
		if (*((int*) ppPlanetData[GameMap::Annihilated]) != NOT_ANNIHILATED) {
			
			GetDeadPlanetButtonString (iDeadPlanetKey, iPlanetKey, iProxyKey, strAltTag, &strPlanetString);
			pszColor = NULL;
			
		} else {
			
			switch (iOwner) {
				
			case SYSTEM:
				
				GetLivePlanetButtonString (iLivePlanetKey, iPlanetKey, iProxyKey, strAltTag, &strPlanetString);
				pszColor = NULL;
				
				break;
				
			case INDEPENDENT:
				
				GetIndependentPlanetButtonString (iPlanetKey, iProxyKey, strAltTag, &strPlanetString);
				pszColor = bMapColoring ? m_vBadColor.GetCharPtr() : NULL;
				
				break;
				
			default:
				
				iErrCode = g_pGameEngine->GetEmpireAlienKey (iOwner, &iAlienKey);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				GetAlienButtonString (
					iAlienKey, 
					iOwner, 
					iOwner == iEmpireKey, 
					iPlanetKey, 
					iProxyKey, 
					strAltTag,
					&strPlanetString
					);
				
				if (iOwner == iEmpireKey) {
					pszColor = bMapColoring ? m_vGoodColor.GetCharPtr() : NULL;
				} else {
					
					if (!bMapColoring) {
						pszColor = NULL;
					} else {
						
						iErrCode = g_pGameEngine->GetDiplomaticStatus (
							iGameClass,
							iGameNumber,
							iEmpireKey,
							iOwner,
							&iWeOffer,
							&iTheyOffer,
							&iCurrent
							);
						
						if (iErrCode == ERROR_DATA_NOT_FOUND) {
							iCurrent = WAR;
						}
						
						else if (iErrCode != OK) {
							Assert (false);
							goto Cleanup;
						}
						
						switch (iCurrent) {
							
						case WAR:
							pszColor = m_vBadColor.GetCharPtr();
							break;
						case ALLIANCE:
							pszColor = m_vGoodColor.GetCharPtr();
							break;
						default:
							pszColor = NULL;
							break;
						}
					}
				}
				
				break;				
			}
		}
		
		if (HTMLFilter ((char*) ppPlanetData[GameMap::Name], &strFilter, 0, false) != OK) {
			strFilter.Clear();
		}
		
		// Put planet on grid, record planet index
		ppstrGrid[iGridLocX][iGridLocY] =
			
			"<td><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\"><tr><td align=\"center\">"\
			"<font size=\"1\"";
		
		if (pszColor != NULL) {
			ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
			ppstrGrid[iGridLocX][iGridLocY] += pszColor;
			ppstrGrid[iGridLocX][iGridLocY] += "\"";
		}
		
		// Planet name
		ppstrGrid[iGridLocX][iGridLocY] += ">"; 
		ppstrGrid[iGridLocX][iGridLocY] += *((int*) ppPlanetData[GameMap::Minerals]);
		ppstrGrid[iGridLocX][iGridLocY] += "</font></td><td rowspan=\"3\">";
		
		ppstrGrid[iGridLocX][iGridLocY] += strPlanetString;
		
		
		// Write planet
		ppstrGrid[iGridLocX][iGridLocY] += "</td><td align=\"center\"><font size=\"1\"";
		
		if (pszColor != NULL) {
			ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
			ppstrGrid[iGridLocX][iGridLocY] += pszColor;
			ppstrGrid[iGridLocX][iGridLocY] += "\"";
		}
		
		ppstrGrid[iGridLocX][iGridLocY] += ">";
		ppstrGrid[iGridLocX][iGridLocY] += *((int*) ppPlanetData[GameMap::Fuel]);
		ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr><tr><td align=\"center\"><font size=\"1\"";
		
		if (pszColor != NULL) {
			ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
			ppstrGrid[iGridLocX][iGridLocY] += pszColor;
			ppstrGrid[iGridLocX][iGridLocY] += "\"";
		}
		
		ppstrGrid[iGridLocX][iGridLocY] += ">";
		ppstrGrid[iGridLocX][iGridLocY] += *((int*) ppPlanetData[GameMap::Ag]);
		ppstrGrid[iGridLocX][iGridLocY] += "</font></td><td align=\"center\"><font size=\"1\"";
		
		if (pszColor != NULL) {
			ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
			ppstrGrid[iGridLocX][iGridLocY] += pszColor;
			ppstrGrid[iGridLocX][iGridLocY] += "\"";
		}
		
		ppstrGrid[iGridLocX][iGridLocY] += ">";
		ppstrGrid[iGridLocX][iGridLocY] += *((int*) ppPlanetData[GameMap::Pop]);
		ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr><tr><td align=\"center\"><font size=\"1\"";
		
		if (bShipColoring) {
			
			if (iNumOwnShips > 0) {
				
				ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
				ppstrGrid[iGridLocX][iGridLocY] += m_vGoodColor.GetCharPtr();
				ppstrGrid[iGridLocX][iGridLocY] += "\"";
			}
		}
		
		else if (pszColor != NULL) {
			ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
			ppstrGrid[iGridLocX][iGridLocY] += pszColor;
			ppstrGrid[iGridLocX][iGridLocY] += "\"";
		}
		
		ppstrGrid[iGridLocX][iGridLocY] += ">";
		
		if (bHighlightShips) {
			
			if (iNumOwnShips > 0) {
				ppstrGrid[iGridLocX][iGridLocY] += "<font size=\"2\">(<strong>";
				ppstrGrid[iGridLocX][iGridLocY] += iNumOwnShips;
				ppstrGrid[iGridLocX][iGridLocY] += "</strong>)</font>";
			} else {
				ppstrGrid[iGridLocX][iGridLocY] += "(0)";
			}
			
		} else {
			
			if (iNumOwnShips > 0) {
				ppstrGrid[iGridLocX][iGridLocY] += "(";
				ppstrGrid[iGridLocX][iGridLocY] += iNumOwnShips;
				ppstrGrid[iGridLocX][iGridLocY] += ")";
			} else {
				ppstrGrid[iGridLocX][iGridLocY] += "(0)";
			}
		}
		
		ppstrGrid[iGridLocX][iGridLocY] += "</font></td><td align=\"center\"><font size=\"1\"";
		
		if (bShipColoring) {
			
			iErrCode = g_pGameEngine->GetLowestDiplomacyLevelForShipsOnPlanet (
				iGameClass,
				iGameNumber,
				iEmpireKey,
				iPlanetKey,
				bVisible,
				pvEmpireKey,
				iNumEmpires,
				&iAccountingNumOtherShips,
				&iDiplomacyLevel,
				&pvEmpireKey
				);
			
			Assert (iAccountingNumOtherShips == iNumOtherShips);
			
			if (iAccountingNumOtherShips > 0) {
				
				ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
				
				switch (iDiplomacyLevel) {
					
				case WAR:
					ppstrGrid[iGridLocX][iGridLocY] += m_vBadColor.GetCharPtr();
					break;
				case ALLIANCE:
					ppstrGrid[iGridLocX][iGridLocY] += m_vGoodColor.GetCharPtr();
					break;
				default:
					ppstrGrid[iGridLocX][iGridLocY] += m_vTextColor.GetCharPtr();
					break;
				}
				
				ppstrGrid[iGridLocX][iGridLocY] += "\"";
			}
		}
		
		else if (pszColor != NULL) {
			ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
			ppstrGrid[iGridLocX][iGridLocY] += pszColor;
			ppstrGrid[iGridLocX][iGridLocY] += "\"";
		}
		
		ppstrGrid[iGridLocX][iGridLocY] += ">";
		
		
		if (bHighlightShips) {
			
			if (iNumOtherShips > 0) {
				ppstrGrid[iGridLocX][iGridLocY] += "<font size=\"2\">(<strong>";
				ppstrGrid[iGridLocX][iGridLocY] += iNumOtherShips;
				ppstrGrid[iGridLocX][iGridLocY] += "</strong>)</font>";
			} else {
				ppstrGrid[iGridLocX][iGridLocY] += "(0)";
			}
			
		} else {
			
			if (iNumOtherShips > 0) {
				ppstrGrid[iGridLocX][iGridLocY] += "(";
				ppstrGrid[iGridLocX][iGridLocY] += iNumOtherShips;
				ppstrGrid[iGridLocX][iGridLocY] += ")";
			} else {
				ppstrGrid[iGridLocX][iGridLocY] += "(0)";
			}
		}
		
		ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr><tr><td colspan=\"3\" align=\"center\">"\
			"<font size=\"1\" face=\"" DEFAULT_PLANET_NAME_FONT "\"";
		
		if (pszColor != NULL) {
			ppstrGrid[iGridLocX][iGridLocY] += " color=\"#";
			ppstrGrid[iGridLocX][iGridLocY] += pszColor;
			ppstrGrid[iGridLocX][iGridLocY] += "\"";
		}
		
		ppstrGrid[iGridLocX][iGridLocY] += ">";
		ppstrGrid[iGridLocX][iGridLocY] += strFilter;
		ppstrGrid[iGridLocX][iGridLocY] += "</font></td></tr></table></td>";
		
		// Place links on grid
		if (iNumJumps > 0) {
			
			if (bLinkNorth) {
				ppstrGrid [iGridLocX][iGridLocY - 1] = pszVert;
				
				if (iGridLocY > 3) {
					if (!ppstrGrid [iGridLocX][iGridLocY - 3].IsBlank()) {
						ppstrGrid [iGridLocX][iGridLocY - 2] = pszVert;
					}
				}
				iNumJumps --;
			}
			
			if (bLinkEast) {
				ppstrGrid [iGridLocX + 1][iGridLocY] = pszHorz;
				if (iGridLocX < iGridX - 3) {
					if (!ppstrGrid [iGridLocX + 3][iGridLocY].IsBlank()) {
						ppstrGrid [iGridLocX + 2][iGridLocY] = pszHorz;
					}
				}
				iNumJumps --;
			}
			
			if (bLinkSouth) {
				ppstrGrid [iGridLocX][iGridLocY + 1] = pszVert;
				if (iGridLocY < iGridY - 3) {
					if (!ppstrGrid [iGridLocX][iGridLocY + 3].IsBlank()) {
						ppstrGrid [iGridLocX][iGridLocY + 2] = pszVert;
					}
				}
				iNumJumps --;
			}
			
			if (bLinkWest) {
				ppstrGrid [iGridLocX - 1][iGridLocY] = pszHorz;
				if (iGridLocX > 3) {
					if (!ppstrGrid [iGridLocX - 3][iGridLocY].IsBlank()) {
						ppstrGrid [iGridLocX - 2][iGridLocY] = pszHorz;
					}
				}
			}
		}
		
		pDatabase->FreeData (ppPlanetData);
		ppPlanetData = NULL;
	}
	
	// Write out map
	OutputText ("<table cellpadding=\"0\" cellspacing=\"0\">"); 
	
	// Parse grid
	for (j = 0; j < iGridY; j ++) {
		
		OutputText ("<tr>");
		
		for (i = 0; i < iGridX; i ++) {
			
			if (ppstrGrid[i][j].IsBlank()) {
				
				if (i % 3 == 1 && j % 3 == 1) { 
					OutputText ("<td width=\"50\" height=\"50\">&nbsp;</td>");
				} else {
					OutputText ("<td></td>");
				}
			} else {
				m_pHttpResponse->WriteText (ppstrGrid[i][j]);
			}
		}
		OutputText ("</tr>");
	}
	
	OutputText ("</table>");
	
	
Cleanup:
	
	if (!bAdmin && !bSpectators) {
		
		if (pvPlanetKey != pvEasyWayOut) {
			pDatabase->FreeData (pvPlanetKey);
		}
		if (piProxyKey != piEasyProxyKeys) {
			pDatabase->FreeKeys (piProxyKey);
		}
		
		if (pEmpireMap != NULL) {
			pEmpireMap->Release();
		}
		
	} else {
			
		if (piPlanetKey != NULL) {
			pDatabase->FreeKeys (piPlanetKey);
		}
	}
	
	if (pvEmpireKey != NULL) {
		pDatabase->FreeData (pvEmpireKey);
	}
	
	if (ppPlanetData != NULL) {
		pDatabase->FreeData (ppPlanetData);
	}
	
	if (pstrGrid != NULL) {
		delete [] pstrGrid;
	}
	
	if (pGameMap != NULL) {
		pGameMap->Release();
	}
	
	pDatabase->Release();
	
	return iErrCode;
}


void HtmlRenderer::SearchForDuplicateIPAddresses (int iGameClass, int iGameNumber) {
	
	int* piDuplicateKeys;
	unsigned int * piNumDuplicatesInList, iNumDuplicates;
	
	char pszBuffer [512];
	
	int iErrCode = g_pGameEngine->SearchForDuplicates (
		iGameClass, 
		iGameNumber,
		SystemEmpireData::IPAddress,
		GameEmpireData::EnterGameIPAddress,
		&piDuplicateKeys,
		&piNumDuplicatesInList,
		&iNumDuplicates
		);
	
	if (iErrCode != OK) {
		
		sprintf (pszBuffer, "Error %i occurred searching for empires with the same IP address", iErrCode);
		AddMessage (pszBuffer);
		
	} else {
		
		if (iNumDuplicates == 0) {
			AddMessage ("No </strong>empires with the same IP address were found<strong>");
		} else {
			
			if (iNumDuplicates == 1) {
				
				AddMessage ("1 </strong>duplicate IP address was found:<strong>");
				
			} else {
				
				sprintf (pszBuffer, "%i</strong> duplicate IP addresses were found:<strong>", iNumDuplicates);
				AddMessage (pszBuffer);
			}
			
			Variant vName;
			String strList;
			unsigned int iIndex = 0, iLimit, i, j;
			
			for (i = 0; i < iNumDuplicates; i ++) {
				
				strList.Clear();
				
				iLimit = iIndex + piNumDuplicatesInList[i] - 1;
				
				for (j = iIndex; j < iLimit; j ++) {
					
					iErrCode = g_pGameEngine->GetEmpireName (piDuplicateKeys[j], &vName);
					if (iErrCode == OK) {
						
						strList += vName.GetCharPtr();
						if (j == iLimit - 1) {
							strList += " </strong>and<strong> ";
						} else {
							strList += ", ";
						}
					}
				}
				iIndex = iLimit + 1;
				
				iErrCode = g_pGameEngine->GetEmpireName (piDuplicateKeys[j], &vName);
				if (iErrCode == OK) {
					strList += vName.GetCharPtr();
				}
				
				strList += " </strong>have the same IP address<strong>";
				
				AddMessage (strList);
			}
			
			// Free memory
			delete [] piNumDuplicatesInList;
			delete [] piDuplicateKeys;
		}
	}
}

void HtmlRenderer::SearchForDuplicateSessionIds (int iGameClass, int iGameNumber) {
	
	int* piDuplicateKeys;
	unsigned int * piNumDuplicatesInList, iNumDuplicates;
	
	char pszBuffer [512];
	
	int iErrCode = g_pGameEngine->SearchForDuplicates (
		iGameClass, 
		iGameNumber,
		SystemEmpireData::SessionId,
		NO_KEY,
		&piDuplicateKeys,
		&piNumDuplicatesInList,
		&iNumDuplicates
		);
	
	if (iErrCode != OK) {
		
		sprintf (pszBuffer, "Error %i occurred searching for empires with the same Session Id", iErrCode);
		AddMessage (pszBuffer);
		
	} else {
		
		if (iNumDuplicates == 0) {
			AddMessage ("No </strong>empires with the same Session Id were found<strong>");
		} else {
			
			if (iNumDuplicates == 1) {
				
				AddMessage ("1 </strong>duplicate Session Id was found:<strong>");
				
			} else {
				
				sprintf (pszBuffer, "%i</strong> duplicate Session Ids were found:<strong>", iNumDuplicates);
				AddMessage (pszBuffer);
			}
			
			Variant vName;
			String strList;
			unsigned int iIndex = 0, iLimit, i, j;
			
			for (i = 0; i < iNumDuplicates; i ++) {
				
				strList.Clear();
				
				iLimit = iIndex + piNumDuplicatesInList[i] - 1;
				
				for (j = iIndex; j < iLimit; j ++) {
					
					iErrCode = g_pGameEngine->GetEmpireName (piDuplicateKeys[j], &vName);
					if (iErrCode == OK) {
						
						strList += vName.GetCharPtr();
						if (j == iLimit - 1) {
							strList += " </strong>and<strong> ";
						} else {
							strList += ", ";
						}
					}
				}
				iIndex = iLimit + 1;
				
				iErrCode = g_pGameEngine->GetEmpireName (piDuplicateKeys[j], &vName);
				if (iErrCode == OK) {
					strList += vName.GetCharPtr();
				}
				
				strList += " </strong>have the same Session Id<strong>";
				
				AddMessage (strList);
			}
			
			// Free memory
			delete [] piNumDuplicatesInList;
			delete [] piDuplicateKeys;
		}
	}
}

void HtmlRenderer::RenderShips (IReadTable* pShips, IReadTable* pFleets, int iBR, float fMaintRatio, 
								ShipsInMapScreen* pShipsInMap) {
	
	// Read ship location column
	unsigned int* piShipKey = NULL, * piFleetKey = NULL, iNumShips = 0, iNumFleets = 0, iNumFleetShips = 0;
	int* piShipLoc = NULL, * piFleetLoc = NULL, iErrCode;
	
	unsigned int** ppiFleetShips = NULL, * piNumShipsInFleet = NULL, i, j, iNumOrders;
	void** ppData = NULL;
	
	int* piOrderKey = NULL, iSelectedOrder, iLastX = 0, iLastY = 0, iLastLocation = NO_KEY;
	String* pstrOrderText = NULL;
	
	Variant vPlanetName;
	
	GameConfiguration gcConfig;
	iErrCode = g_pGameEngine->GetGameConfiguration (&gcConfig);
	if (iErrCode != OK) {
		Assert (false);
		goto Cleanup;
	}
	
	if (pShipsInMap == NULL) {
		
		iErrCode = pShips->ReadColumn (
			GameEmpireShips::CurrentPlanet, 
			&piShipKey,
			&piShipLoc, 
			&iNumShips
			);
		
		if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = pFleets->ReadColumn (
			GameEmpireFleets::CurrentPlanet, 
			&piFleetKey, 
			&piFleetLoc, 
			&iNumFleets
			);
		
		if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
			Assert (false);
			goto Cleanup;
		}
		
		OutputText ("<input type=\"hidden\" name=\"NumShips\" value=\"");
		m_pHttpResponse->WriteText (iNumShips);
		OutputText ("\">");
		
		OutputText ("<input type=\"hidden\" name=\"NumFleets\" value=\"");
		m_pHttpResponse->WriteText (iNumFleets);
		OutputText ("\">");
		
	} else {
		
		// Single planet render
		iErrCode = pShips->GetEqualKeys (
			GameEmpireShips::CurrentPlanet,
			pShipsInMap->iPlanetKey,
			false,
			&piShipKey,
			&iNumShips
			);
		
		if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
			Assert (false);
			goto Cleanup;
		}
		
		iErrCode = pFleets->GetEqualKeys (
			GameEmpireFleets::CurrentPlanet,
			pShipsInMap->iPlanetKey,
			false,
			&piFleetKey,
			&iNumFleets
			);
		
		if (iErrCode != OK && iErrCode != ERROR_DATA_NOT_FOUND) {
			Assert (false);
			goto Cleanup;
		}
	}
	
	if (iNumShips == 0) {
		
		if (pShipsInMap == NULL) {
			OutputText ("<p>You have no ships in service");
		}
		
	} else {
		
		// Sort ships by location
		if (pShipsInMap == NULL) {
			Algorithm::QSortTwoAscending<int, unsigned int> (piShipLoc, piShipKey, iNumShips);
		}
		
		// Allocate space for fleet data
		if (iNumFleets > 0) {
			
			if (pShipsInMap == NULL) {
				Algorithm::QSortTwoAscending<int, unsigned int> (piFleetLoc, piFleetKey, iNumFleets);
			}
			
			ppiFleetShips = (unsigned int**) StackAlloc (iNumFleets * sizeof (unsigned int*));
			piNumShipsInFleet = (unsigned int*) StackAlloc (iNumFleets * sizeof (unsigned int));
			
			for (i = 0; i < iNumFleets; i ++) {
				
				iErrCode = g_pGameEngine->GetNumShipsInFleet (
					m_iGameClass,
					m_iGameNumber,
					m_iEmpireKey,
					piFleetKey[i],
					(int*) piNumShipsInFleet + i
					);
				
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (piNumShipsInFleet[i] == 0) {
					ppiFleetShips[i] = NULL;
				} else {
					ppiFleetShips[i] = (unsigned int*) StackAlloc ((piNumShipsInFleet[i] + 1) * sizeof (unsigned int));
					ppiFleetShips[i][0] = 1;
					
					iNumFleetShips += piNumShipsInFleet[i];
				}
			}
		}
		
		if (iNumFleetShips < iNumShips) {
			
			OutputText ("<p><table width=\"90%\"><tr><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">Ship Name</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">BR</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">Max BR</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">Next BR</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">Location</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">Type</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
			OutputText ("\">Orders</th></tr>");
		}
		
		// Process ships!
		unsigned int iFleetKey, iLastFleetKey = NO_KEY, iProxyFleetKey = 0, iLastProxyFleetKey = 0;
		
		for (i = 0; i < iNumShips; i ++) {
			
			iErrCode = pShips->ReadData (piShipKey[i], GameEmpireShips::FleetKey, (int*) &iFleetKey);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			if (iFleetKey != NO_KEY) {
				
				if (iFleetKey == iLastFleetKey) {
					iProxyFleetKey = iLastProxyFleetKey;
				} else {
					
					// Find fleet key
					for (j = 0; j < iNumFleets; j ++) {
						if (piFleetKey[j] == iFleetKey) {
							iProxyFleetKey = iLastProxyFleetKey = j;
							iLastFleetKey = iFleetKey;
							break;
						}
					}
					
					if (j == iNumFleets) {
						continue;
					}
				}
				
				// Add to fleet
				ppiFleetShips[iProxyFleetKey][ppiFleetShips[iProxyFleetKey][0]] = i;
				ppiFleetShips[iProxyFleetKey][0] ++;
				
			} else {
				
				iErrCode = WriteShip (
					gcConfig,
					pShips, pShipsInMap == NULL ? i : pShipsInMap->iCurrentShip, 
					piShipKey[i], 
					iBR, fMaintRatio, pShipsInMap == NULL ? piShipLoc[i] : pShipsInMap->iPlanetKey, 
					iLastLocation, iLastX, iLastY, vPlanetName
					);
				
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (pShipsInMap != NULL) {
					pShipsInMap->iCurrentShip ++;
				}
			}
		}	// End process ship loop
		
		if (iNumFleetShips < iNumShips) {
			OutputText ("</table>");
			
			if (iNumFleetShips > 0) {
				OutputText ("<p>");
				
				if (pShipsInMap == NULL) {
					WriteSeparatorString (m_iSeparatorKey);
				}
			}
		}
	}	// End if numships > 0
	
	
	// Process fleets!
	if (iNumFleets > 0) {
		
		unsigned int iNumShipsInFleet;
		int iPercentage, iIndex;
		float fCurrentStrength, fMaxStrength;
		
		String strHtml;
		
		for (i = 0; i < iNumFleets; i ++) {
			
			iErrCode = pFleets->ReadRow (piFleetKey[i], &ppData);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			iErrCode = g_pGameEngine->GetFleetOrders (m_iGameClass, m_iGameNumber, m_iEmpireKey, piFleetKey[i],
				&piOrderKey, &pstrOrderText, &iSelectedOrder, (int*) &iNumOrders);
			
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			Assert (iNumOrders > 1);
			
			OutputText ("<p><table align=\"center\" cellspacing=\"2\" width=\"90%\" cellspacing=\"0\" cellpadding=\"0\""\
				"<tr align=\"left\"><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
			OutputText ("\" align=\"center\">Fleet Name</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\" align=\"center\">Ships</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\" align=\"center\" colspan=\"2\">Strength</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\" align=\"center\">Location</th><th bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\" align=\"center\">Orders</th></tr>");
			
			iErrCode = HTMLFilter ((char*) ppData[GameEmpireFleets::Name], &strHtml, 0, false);
			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
			
			OutputText ("<tr align=\"left\"><td><input type=\"text\" name=\"FleetName");
			m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
			OutputText ("\" value=\"");
			m_pHttpResponse->WriteText (strHtml);
			OutputText ("\"size=\"15\" maxlength=\"");
			m_pHttpResponse->WriteText (MAX_FLEET_NAME_LENGTH);
			OutputText ("\"><input type=\"hidden\" name =\"OldFleetName");
			m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
			OutputText ("\" value=\""); 
			m_pHttpResponse->WriteText (strHtml); 
			OutputText ("\"><input type=\"hidden\" name=\"FleetKey");
			m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
			OutputText ("\" value=\"");
			m_pHttpResponse->WriteText (piFleetKey[i]);
			
#ifdef _DEBUG
			if (ppiFleetShips == NULL || ppiFleetShips[i] == NULL) {
				iNumShipsInFleet = 0;
			} else {
				iNumShipsInFleet = ppiFleetShips[i][0] - 1;
			}
			
			Assert ((piNumShipsInFleet == NULL && iNumShipsInFleet == 0) ||
				(piNumShipsInFleet[i] == iNumShipsInFleet));
#endif
			
			if (piNumShipsInFleet == NULL) {
				iNumShipsInFleet = 0;
			} else {
				iNumShipsInFleet = piNumShipsInFleet[i];
			}
			
			OutputText ("\"></td><td align=\"center\">");
			m_pHttpResponse->WriteText (iNumShipsInFleet);
			OutputText ("</td><td align=\"center\" colspan=\"2\">");
			
			fCurrentStrength = *((float*) ppData[GameEmpireFleets::CurrentStrength]);
			fMaxStrength = *((float*) ppData[GameEmpireFleets::MaxStrength]);
			
			if (fMaxStrength == (float) 0.0) {
				OutputText ("0 / 0 (0%)");
			} else {
				iPercentage = (int) (100 * fCurrentStrength / fMaxStrength);
				if (iPercentage == 100) {
					OutputText ("<font color=\"#");
					m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
					OutputText ("\">");
				} else {
					OutputText ("<font color=\"#");
					m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
					OutputText ("\">");
				}

				m_pHttpResponse->WriteText (fCurrentStrength);
				OutputText (" / ");
				m_pHttpResponse->WriteText (fMaxStrength);

				OutputText (" (");
				m_pHttpResponse->WriteText (iPercentage);
				OutputText ("%)</font>");
			}
			
			OutputText ("</td><input type=\"hidden\" name=\"FleetSelectedOrder");
			m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
			OutputText ("\" value=\"");
			m_pHttpResponse->WriteText (iSelectedOrder);
			OutputText ("\"><td align=\"center\">");
			
			if (pShipsInMap != NULL  || piFleetLoc[i] != iLastLocation) {
				
				iLastLocation = pShipsInMap == NULL ? piFleetLoc[i] : pShipsInMap->iPlanetKey;
				
				iErrCode = g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, iLastLocation, &vPlanetName);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				iErrCode = g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, iLastLocation, &iLastX, &iLastY);
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
			}
			
			m_pHttpResponse->WriteText (vPlanetName.GetCharPtr());
			OutputText (" (");
			m_pHttpResponse->WriteText (iLastX);
			OutputText (",");
			m_pHttpResponse->WriteText (iLastY); 
			OutputText (")</td><td align=\"center\">");
			
			if (iNumOrders == 1) {
				OutputText ("<strong>");
				m_pHttpResponse->WriteText (pstrOrderText[0]);
				OutputText ("</strong><input type=\"hidden\" name=\"FleetOrder");
				m_pHttpResponse->WriteText (i);
				OutputText ("\" value=\"");
				m_pHttpResponse->WriteText (piOrderKey[0]);
				OutputText ("\">");
				
			} else {
				
				OutputText ("<select name=\"FleetOrder");
				m_pHttpResponse->WriteText (pShipsInMap == NULL ? i : pShipsInMap->iCurrentFleet);
				OutputText ("\">");
				
				for (j = 0; j < iNumOrders; j ++) {
					if (iSelectedOrder == piOrderKey[j]) {
						OutputText ("<option selected value=\"");
						m_pHttpResponse->WriteText (piOrderKey[j]);
						OutputText ("\">");
						m_pHttpResponse->WriteText (pstrOrderText[j]);
					} else {
						OutputText ("<option value=\"");
						m_pHttpResponse->WriteText (piOrderKey[j]);
						OutputText ("\">");
						m_pHttpResponse->WriteText (pstrOrderText[j]);
					}
				}
				OutputText ("</select>");
			}
			OutputText ("</td></tr>");
			
			if (iNumShipsInFleet > 0) {
				
				OutputText ("<tr><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
				OutputText ("\" align=\"center\">Ship Name</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
				OutputText ("\" align=\"center\">BR</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
				OutputText ("\" align=\"center\">Max BR</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\">Next BR</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\">Type</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\" align=\"center\">Orders</th></tr>");
				
				for (j = 1; j < ppiFleetShips[i][0]; j ++) {
					
					iIndex = ppiFleetShips[i][j];
					
					iErrCode = WriteShip (
						gcConfig,
						pShips, 
						pShipsInMap == NULL ? iIndex : pShipsInMap->iCurrentShip, 
						piShipKey[iIndex], iBR, fMaintRatio, 
						pShipsInMap == NULL ? piShipLoc[iIndex] : pShipsInMap->iPlanetKey, 
						iLastLocation, iLastX, iLastY, vPlanetName, true
						);
					
					if (iErrCode != OK) {
						Assert (false);
						goto Cleanup;
					}
					
					if (pShipsInMap != NULL) {
						pShipsInMap->iCurrentShip ++;
					}
				}
			}
			
			OutputText ("</table>");
			
			if (pShipsInMap != NULL) {
				pShipsInMap->iCurrentFleet ++;
			}
			
			g_pGameEngine->FreeData (ppData);
			ppData = NULL;
		}
		
		delete [] pstrOrderText;
		pstrOrderText = NULL;
		
		delete [] piOrderKey;
		piOrderKey = NULL;
	}
	
Cleanup:
	
	if (ppData != NULL) {
		g_pGameEngine->FreeData (ppData);
	}
	
	if (pstrOrderText != NULL) {
		delete [] pstrOrderText;
	}
	
	if (piOrderKey != NULL) {
		delete [] piOrderKey;
	}
	
	if (iNumShips > 0) {
		
		if (pShipsInMap == NULL) {
			g_pGameEngine->FreeData (piShipLoc);
		}
		if (piShipKey != NULL) {
			g_pGameEngine->FreeKeys (piShipKey);
		}
	}
	
	if (iNumFleets > 0) {
		if (pShipsInMap == NULL) {
			g_pGameEngine->FreeData (piFleetLoc);
		}
		if (piFleetKey != NULL) {
			g_pGameEngine->FreeKeys (piFleetKey);
		}
	}
	
	if (iErrCode != OK) {
		AddMessage ("Error in RenderShips()");
	}
}

int HtmlRenderer::HandleShipMenuSubmissions () {
	
	int iErrCode, i, iKey, iOldOrderKey, iNewOrderKey, iNumShips, iNumFleets, iRealNumber;
	
	IHttpForm* pHttpForm;
	const char* pszOldName, * pszNewName;
	
	// Discard submission if update counts don't match
	if (m_iNumNewUpdates != m_iNumOldUpdates) {
		return OK;
	}
	
	// Get number of ships
	if ((pHttpForm = m_pHttpRequest->GetForm ("NumShips")) == NULL) {
		return OK;
	}
	iNumShips = pHttpForm->GetIntValue();
	
	// Danger!
	iErrCode = g_pGameEngine->GetNumShips (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iRealNumber);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}
	
	if (iNumShips <= iRealNumber) {
		
		char pszForm [128];
		
		for (i = 0; i < iNumShips; i ++) {
			
			iKey = NO_KEY;
			
			// Get old ship name
			sprintf (pszForm, "OldShipName%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				continue;
			}
			pszOldName = pHttpForm->GetValue();
			if (pszOldName == NULL) {
				pszOldName = "";
			}
			
			// Get new ship name
			sprintf (pszForm, "ShipName%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				continue;
			}
			pszNewName = pHttpForm->GetValue();
			if (pszNewName == NULL) {
				pszNewName = "";
			}
			
			if (strcmp (pszOldName, pszNewName) != 0) {
				
				if (!ShipOrFleetNameFilter (pszNewName)) {
					AddMessage ("The ship name contains an illegal character");
				} else {
					
					if (strlen (pszNewName) > MAX_SHIP_NAME_LENGTH) {
						AddMessage ("The ship name is too long");
					} else {
						
						// Get ship key
						sprintf (pszForm, "ShipKey%i", i);
						if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
							continue;
						}
						iKey = pHttpForm->GetIntValue();
						
						// Update ship name, best effort
						iErrCode = g_pGameEngine->UpdateShipName (
							m_iGameClass, 
							m_iGameNumber, 
							m_iEmpireKey, 
							iKey, 
							pszNewName
							);
					}
				}
			}
			
			// Get old ship order
			sprintf (pszForm, "ShipSelectedOrder%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				continue;
			}
			iOldOrderKey = pHttpForm->GetIntValue();
			
			// Get new ship order
			sprintf (pszForm, "ShipOrder%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				continue;
			}
			iNewOrderKey = pHttpForm->GetIntValue();
			
			if (iOldOrderKey != iNewOrderKey) {
				
				// Get ship key if necessary
				if (iKey == NO_KEY) {
					sprintf (pszForm, "ShipKey%i", i);
					if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
						continue;
					}
					iKey = pHttpForm->GetIntValue();
				}
				
				// Update ship order, best effort
				iErrCode = g_pGameEngine->UpdateShipOrders (
					m_iGameClass, 
					m_iGameNumber, 
					m_iEmpireKey, 
					iKey, 
					iNewOrderKey
					);
			}
		}	// End ships loop
	}
	
	// Get number of fleets
	if ((pHttpForm = m_pHttpRequest->GetForm ("NumFleets")) == NULL) {
		return OK;
	}
	iNumFleets = pHttpForm->GetIntValue();
	
	// Danger!
	iErrCode = g_pGameEngine->GetNumFleets (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iRealNumber);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}
	
	if (iNumFleets <= iRealNumber) {
		
		char pszForm [128];
		
		for (i = 0; i < iNumFleets; i ++) {
			
			iKey = NO_KEY;
			
			// Get old fleet name
			sprintf (pszForm, "OldFleetName%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				continue;
			}
			pszOldName = pHttpForm->GetValue();
			if (pszOldName == NULL) {
				pszOldName = "";
			}
			
			// Get new fleet name
			sprintf (pszForm, "FleetName%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				continue;
			}
			pszNewName = pHttpForm->GetValue();
			if (pszNewName == NULL) {
				pszNewName = "";
			}
			
			if (strcmp (pszOldName, pszNewName) != 0) {
				
				if (!ShipOrFleetNameFilter (pszNewName)) {
					AddMessage ("Illegal fleet name");
				} else {
					
					if (strlen (pszNewName) > MAX_FLEET_NAME_LENGTH) {
						AddMessage ("The fleet name is too long");
					} else {
						
						// Get fleet key
						sprintf (pszForm, "FleetKey%i", i);
						if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
							continue;
						}
						iKey = pHttpForm->GetIntValue();
						
						// Update fleet name, best effort
						iErrCode = g_pGameEngine->UpdateFleetName (
							m_iGameClass, 
							m_iGameNumber, 
							m_iEmpireKey, 
							iKey, 
							pszNewName
							);
					}
				}
			}
			
			// Get old fleet order
			sprintf (pszForm, "FleetSelectedOrder%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				continue;
			}
			iOldOrderKey = pHttpForm->GetIntValue();
			
			// Get new fleet order
			sprintf (pszForm, "FleetOrder%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
				continue;
			}
			iNewOrderKey = pHttpForm->GetIntValue();
			
			if (iOldOrderKey != iNewOrderKey) {
				
				// Get fleet key if necessary
				if (iKey == NO_KEY) {
					sprintf (pszForm, "FleetKey%i", i);
					if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
						continue;
					}
					iKey = pHttpForm->GetIntValue();
				}
				
				// Update fleet orders, best effort
				iErrCode = g_pGameEngine->UpdateFleetOrders (
					m_iGameClass, 
					m_iGameNumber, 
					m_iEmpireKey, 
					iKey, 
					iNewOrderKey
					);
			}
		}	// End fleets loop
	}
	
	return OK;
}

void HtmlRenderer::ReportLoginFailure (IReport* pReport, const char* pszEmpireName) {
	
	if (g_pGameEngine->ReportLoginEvents()) {
		
		char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);
		sprintf (pszMessage, "Logon failure for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
		
		pReport->WriteReport (pszMessage);
	}
}

void HtmlRenderer::ReportLoginSuccess (IReport* pReport, const char* pszEmpireName) {
	
	if (g_pGameEngine->ReportLoginEvents()) {
		
		char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);
		sprintf (pszMessage, "Logon success for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
		
		pReport->WriteReport (pszMessage);
	}
}


void HtmlRenderer::ReportEmpireCreation (IReport* pReport, const char* pszEmpireName) {
	
	if (g_pGameEngine->ReportLoginEvents()) {
		
		char* pszMessage = (char*) StackAlloc (MAX_EMPIRE_NAME_LENGTH + 256);
		sprintf (pszMessage, "Creation success for %s from %s", pszEmpireName, m_pHttpRequest->GetClientIP());
		
		pReport->WriteReport (pszMessage);
	}
}

void HtmlRenderer::WriteSystemTitleString () {
	
	m_pHttpResponse->WriteText (PageName [m_pgPageId]);
	OutputText (": ");
	WriteVersionString();
}

void HtmlRenderer::WriteSystemHeaders (bool bFileUpload) {
	
	if (bFileUpload) {
		OutputText ("<form method=\"post\" enctype=\"multipart/form-data\">");
	} else {
		OutputText ("<form method=\"post\">");
	}
	
	OutputText ("<input type=\"hidden\" name=\"PageId\" value=\"");
	m_pHttpResponse->WriteText ((int) m_pgPageId);
	OutputText ("\">");
	
	const char* pszName = m_vEmpireName.GetCharPtr();
	
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
	
	OutputText (" <font size=\"+3\"><strong>");
	m_pHttpResponse->WriteText (pszName);
	
	if (pszName [strlen (pszName) - 1] == 's') {
		OutputText ("' ");
	} else {
		OutputText ("'s ");
	}
	m_pHttpResponse->WriteText (PageName [m_pgPageId]);
	
	if (m_bHalted) {
		OutputText (" (Empire marked for deletion)");
	}
	
	OutputText ("</strong></font><p>");
}

void HtmlRenderer::WriteSystemButtons (int iButtonKey, int iPrivilege) {
	
	// ActiveGameList
	WriteButton (BID_ACTIVEGAMELIST);
	
	// OpenGameList
	WriteButton (BID_OPENGAMELIST);
	
	// SystemGameList
	WriteButton (BID_SYSTEMGAMELIST);
	
	// ProfileViewer
	OutputText ("<br>");
	WriteButton (BID_PROFILEVIEWER);
	
	// ProfileEditor
	WriteButton (BID_PROFILEEDITOR);
	
	// Top Lists
	WriteButton (BID_TOPLISTS);
	
	// Chatroom
	WriteButton (BID_CHATROOM);
	
	// Exit
	WriteButton (BID_EXIT);
	
	OutputText ("<br>");

	// Latest nukes
	WriteButton (BID_SPECTATORGAMES);
	WriteButton (BID_LATESTNUKES);

	OutputText ("<br>");
	
	// Personal Game Classes
	if (iPrivilege >= ADEPT) {
		WriteButton (BID_PERSONALGAMECLASSES);
	}
	
	if (iPrivilege >= ADMINISTRATOR) {
		
		// Server Administrator
		OutputText ("<br>");
		WriteButton (BID_SERVERADMINISTRATOR);
		
		// Game Administrator
		WriteButton (BID_GAMEADMINISTRATOR);
		
		// Empire Administrator
		OutputText ("<br>");
		WriteButton (BID_EMPIREADMINISTRATOR);
		
		// Theme Administrator
		WriteButton (BID_THEMEADMINISTRATOR);
	}
	
	OutputText ("<p>");
}

void HtmlRenderer::WriteSystemMessages() {
	
	if (!m_strMessage.IsBlank()) {
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (m_strMessage);
		OutputText ("</strong><p>");
	}
	
	// Check for messages
	Variant** ppvMessage;
	int iNumMessages, i;
	
	int iErrCode = g_pGameEngine->GetUnreadSystemMessages (
		m_iEmpireKey,
		&ppvMessage,
		&iNumMessages
		);
	
	if (iErrCode == OK && iNumMessages > 0) {
		
		const char* pszMethod, * pszSource, * pszMessage, * pszFontColor;
		String strHTMLMessage, strFiltered;
		bool bBroadcast, bSystem, bExists;
		
		int iSrcEmpireKey, iAlienKey;
		
		unsigned int iNumMessagesFromPeople = 0;
		char pszDate [OS::MaxDateLength], pszProfile [1024];
		
		for (i = 0; i < iNumMessages; i ++) {
			
			pszSource = ppvMessage[i][SystemEmpireMessages::Source].GetCharPtr();
			
			if (ppvMessage[i][SystemEmpireMessages::Broadcast].GetInteger() != 0) {
				bBroadcast = true;
				pszFontColor = m_vBroadcastMessageColor.GetCharPtr();
			} else {
				bBroadcast = false;
				pszFontColor = m_vPrivateMessageColor.GetCharPtr();
			}
			
			pszMethod = bBroadcast ? "</strong> broadcast:<p>" : "</strong> sent:";
			
			bSystem = strcmp (pszSource, SYSTEM_MESSAGE_SENDER) == 0;
			
			// Format message
			if (bSystem) {
				pszMessage = ppvMessage[i][SystemEmpireMessages::Text].GetCharPtr();
			} else {
				
				if (HTMLFilter (
					ppvMessage[i][SystemEmpireMessages::Text].GetCharPtr(), 
					&strFiltered, 
					MAX_NUM_SPACELESS_CHARS,
					true
					) == OK) {
					
					pszMessage = strFiltered.GetCharPtr();
					
				} else {
					
					pszMessage = "The server is out of memory";
				}
			}
			
			iErrCode = Time::GetDateString (ppvMessage[i][SystemEmpireMessages::TimeStamp].GetUTCTime(), pszDate);
			if (iErrCode != OK) {
				StrNCpy (pszDate, "The server is out of memory");
			}
			
			OutputText ("<table width=\"55%\"><tr><td align=\"left\">");
			
			if (!bSystem) {
				
				iErrCode = g_pGameEngine->DoesEmpireExist (pszSource, &bExists, &iSrcEmpireKey, NULL);
				if (iErrCode == OK && bExists) {
					
					iErrCode = g_pGameEngine->GetEmpireAlienKey (iSrcEmpireKey, &iAlienKey);
					if (iErrCode == OK) {
						
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
						
						iNumMessagesFromPeople ++;
						
						OutputText (" ");
					}
				}
			}
			
			
			OutputText ("On ");
			m_pHttpResponse->WriteText (pszDate);
			
			OutputText (", <strong>");
			
			m_pHttpResponse->WriteText (ppvMessage[i][SystemEmpireMessages::Source].GetCharPtr());
			
			m_pHttpResponse->WriteText (pszMethod);
			OutputText ("<p><font face=\"");
			m_pHttpResponse->WriteText (DEFAULT_MESSAGE_FONT);
			OutputText ("\" size=\"");
			m_pHttpResponse->WriteText (DEFAULT_MESSAGE_FONT_SIZE);
			if (!bSystem) {
				OutputText ("\" color=\"#");
				m_pHttpResponse->WriteText (pszFontColor);
			}
			OutputText ("\">");
			WriteFormattedMessage (pszMessage);
			OutputText ("</font></td></tr></table><p>");
			
			g_pGameEngine->FreeData (ppvMessage[i]);
			
		}	// End empire loop
		
		if (iNumMessagesFromPeople > 0) {
			NotifyProfileLink();
		}
		
		delete [] ppvMessage;
	}
}

void HtmlRenderer::WriteBackupMessage () {
	
	int iErrCode;
	String strTimeElapsed, strTimeRemaining;
	
	unsigned int iNumTemplates, iMaxNumTemplates, iNumTables, iMaxNumTables, iPercent;
	Seconds iNumSeconds, iNumSecsRemaining;
	
	g_pGameEngine->GetDatabaseBackupProgress (
		&iNumTemplates, 
		&iMaxNumTemplates, 
		&iNumTables, 
		&iMaxNumTables,
		&iNumSeconds
		);
	
	iPercent = iMaxNumTables == 0 ? 0 : 100 * iNumTables / iMaxNumTables;
	
	iNumSecsRemaining = iNumTables == 0 ? 600 : (Seconds) ((float) iMaxNumTables * iNumSeconds / iNumTables) - iNumSeconds;
	
	const char* pszTimeElapsed, * pszTimeRemaining;
	char pszElapsed [MAX_HTML_TIME_LENGTH], pszRemaining [MAX_HTML_TIME_LENGTH];
	
	iErrCode = ConvertTime (iNumSeconds, pszElapsed);
	if (iErrCode == OK) {
		pszTimeElapsed = pszElapsed;
	} else {
		pszTimeElapsed = "Unknown time";
	}
	
	iErrCode = ConvertTime (iNumSecsRemaining, pszRemaining);
	if (iErrCode == OK) {
		pszTimeRemaining = pszRemaining;
	} else {
		pszTimeRemaining = "Unknown time";
	}
	
	char pszBuffer [2048];
	
	sprintf (
		pszBuffer,
		"Access to the server is denied. The database is being backed up.<br> %i%% "\
		"of the tables have been written in %s (%i of %i)"\
		"<br>Approximately %s remain%c",
		iPercent,
		pszTimeElapsed,
		iNumTables,
		iMaxNumTables,
		pszTimeRemaining,
		(iNumSecsRemaining == 1 || iNumSecsRemaining == 60 || iNumSecsRemaining == 3600) ? 's' : '\0'
		);
	
	AddMessage (pszBuffer);
}

int HtmlRenderer::InitializeEmpire() {
	
	int iErrCode;
	
	IHttpForm* pHttpForm;
	bool bExists;
	
	// Make sure we're not backing up
	if (g_pGameEngine->IsDatabaseBackingUp()) {		
		WriteBackupMessage();
		return ERROR_FAILURE;
	}
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("PageId")) != NULL &&
		pHttpForm->GetIntValue() == m_pgPageId) {
		
		m_bOwnPost = true;

		if (m_bAuthenticated) {

			iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bExists);
			if (iErrCode != OK || !bExists) { 		
				AddMessage ("That empire no longer exists");
				return ERROR_FAILURE;
			}

		} else {
		
			// Get empire key
			if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireKey")) != NULL) {

				m_iEmpireKey = pHttpForm->GetIntValue();
				
				// Make sure empire key exists
				iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bExists);
				if (iErrCode != OK || 
					!bExists || 
					g_pGameEngine->GetEmpireName (m_iEmpireKey, &m_vEmpireName) != OK) {
					
					AddMessage ("That empire no longer exists");
					return ERROR_FAILURE;
				}
				
			} else {
				
				// Look for empire name form
				const char* pszName;
				if ((pHttpForm = m_pHttpRequest->GetForm ("EmpireName")) == NULL || 
					(pszName = pHttpForm->GetValue()) == NULL) {
					
					AddMessage ("Missing EmpireKey form");
					return ERROR_FAILURE;
					
				} else {
					
					// Look up name
					iErrCode = g_pGameEngine->DoesEmpireExist (
						pszName, 
						&bExists, 
						&m_iEmpireKey, 
						&m_vEmpireName
						);
					
					if (iErrCode != OK || !bExists) {
						
						AddMessage ("That empire doesn't exist");
						return ERROR_FAILURE;
					}
				}
			}
		}

		// Handle session id
		bool bUpdateSessionId, bUpdateCookie;
		iErrCode = InitializeSessionId (&bUpdateSessionId, &bUpdateCookie);
		if (iErrCode != OK) {
			AddMessage ("Session id negotiation failed");
			return ERROR_FAILURE;
		}

		if (!m_bAuthenticated) {

			int iEmpireOptions;
		
			// Get submitted password
			iErrCode = g_pGameEngine->GetEmpirePassword (m_iEmpireKey, &m_vPassword);
			if (iErrCode != OK) {
				AddMessage ("That empire no longer exists");
				return iErrCode;
			}
			
			iErrCode = g_pGameEngine->GetEmpireIPAddress (m_iEmpireKey, &m_vPreviousIPAddress);
			if (iErrCode != OK) {
				AddMessage ("That empire no longer exists");
				return iErrCode;
			}
			
			// Get rest of data
			iErrCode = g_pGameEngine->GetEmpireOptions (m_iEmpireKey, &iEmpireOptions);
			if (iErrCode != OK) {
				AddMessage ("That empire no longer exists");
				return iErrCode;
			}

			m_bHashPasswordWithIPAddress = (iEmpireOptions & IP_ADDRESS_PASSWORD_HASHING) != 0;
			m_bHashPasswordWithSessionId = (iEmpireOptions & SESSION_ID_PASSWORD_HASHING) != 0;

			if ((pHttpForm = m_pHttpRequest->GetForm ("Password")) != NULL) {
				
				uint64 iSubmittedPasswordHash = pHttpForm->GetInt64Value(), iRealPasswordHash;
				
				// Authenticate
				if (!IsGamePage (m_pgPageId)) {
					iRealPasswordHash = GetPasswordHash();
				} else {
					iRealPasswordHash = GetGamePagePasswordHash();
				}				
				
				if (iRealPasswordHash != iSubmittedPasswordHash) {
					
					char pszBuffer [256 + MAX_EMPIRE_NAME_LENGTH];
					sprintf (
						pszBuffer,
						"That was the wrong password for the %s empire",
						m_vEmpireName.GetCharPtr()
						);
					
					AddMessage (pszBuffer);
					return ERROR_FAILURE;
				}
				
			} else {
				
				// Authenticate cleartext
				const char* pszPassword;
				if ((pHttpForm = m_pHttpRequest->GetForm ("ClearTextPassword")) == NULL ||
					(pszPassword = pHttpForm->GetValue()) == NULL ||
					strcmp (pszPassword, m_vPassword.GetCharPtr()) != 0
					) {
					
					char pszBuffer [256 + MAX_EMPIRE_NAME_LENGTH];
					sprintf (
						pszBuffer,
						"That was the wrong password for the %s empire",
						m_vEmpireName.GetCharPtr()
						);
					
					AddMessage (pszBuffer);
					return ERROR_FAILURE;
				}
			}

			m_bAuthenticated = true;
		}
		
		// Update session id in database
		if (bUpdateSessionId) {
			
			// Write the empire's new session id
			iErrCode = g_pGameEngine->SetEmpireSessionId (m_iEmpireKey, m_i64SessionId);
			if (iErrCode != OK) {
				AddMessage ("That empire no longer exists");
				return iErrCode;
			}
		}
		
		// Update session id cookie
		if (bUpdateCookie) {
			
			// Best effort set a new cookie
			char pszSessionId [128];
			String::I64toA (m_i64SessionId, pszSessionId, 10);
			
			m_pHttpResponse->CreateCookie ("SessionId", pszSessionId, 31536000, NULL);
		}
		
		// Update IP address
		if (strcmp (m_pHttpRequest->GetClientIP(), m_vPreviousIPAddress.GetCharPtr()) != 0) {
			
			iErrCode = g_pGameEngine->SetEmpireIPAddress (
				m_iEmpireKey,
				m_pHttpRequest->GetClientIP()
				);
			if (iErrCode != OK) {
				AddMessage ("That empire no longer exists");
				return ERROR_FAILURE;
			}
		}
		
		iErrCode = g_pGameEngine->GetEmpireThemeKey (m_iEmpireKey, &m_iThemeKey);
		
		if (iErrCode == OK) {
			iErrCode = g_pGameEngine->GetEmpirePrivilege (m_iEmpireKey, &m_iPrivilege);
		
			if (iErrCode == OK) {
				iErrCode = g_pGameEngine->GetEmpireAlienKey (m_iEmpireKey, &m_iAlienKey);
			}
		}

		int iEmpireOptions;

		if (iErrCode == OK) {

			iErrCode = g_pGameEngine->GetEmpireOptions (m_iEmpireKey, &iEmpireOptions);
			if (iErrCode == OK) {
				m_bFixedBackgrounds = (iEmpireOptions & FIXED_BACKGROUNDS) != 0;
				m_bHalted = (iEmpireOptions & EMPIRE_MARKED_FOR_DELETION) != 0;
			}
		
			if (iErrCode == OK) {
				iErrCode = GetUIData (m_iThemeKey);
		
				if (iErrCode == OK && !IsGamePage (m_pgPageId)) {

					m_bRepeatedButtons = (iEmpireOptions & SYSTEM_REPEATED_BUTTONS) != 0;
					m_bTimeDisplay = (iEmpireOptions & SYSTEM_DISPLAY_TIME) != 0;
				}
			}
		}
		
		if (iErrCode != OK) {
			AddMessage ("That empire no longer exists");
			return ERROR_FAILURE;
		}
		
		// Add name to web server's log
		m_pHttpResponse->AddCustomLogMessage (m_vEmpireName.GetCharPtr());
		
	} else {
		
		bool bExists;
		iErrCode = g_pGameEngine->DoesEmpireExist (m_iEmpireKey, &bExists);
		if (iErrCode != OK || !bExists) {
			AddMessage ("That empire no longer exists");
			return ERROR_FAILURE;
		}
	}
	
	// Make sure access is allowed
	int iOptions;
	iErrCode = g_pGameEngine->GetSystemOptions (&iOptions);
	
	if ((iErrCode != OK || !(iOptions & ACCESS_ENABLED)) && m_iPrivilege < ADMINISTRATOR) {
		
		// Get reason
		char pszBuffer [128 + MAX_REASON_LENGTH] = "Access is denied to the server at this time. ";
		
		Variant vReason;
		if (g_pGameEngine->GetAccessDisabledReason (&vReason) == OK) {
			strcat (pszBuffer, vReason.GetCharPtr());
		}
		AddMessage (pszBuffer);
		
		return ERROR_FAILURE;
	}
	
	return OK;
}

int HtmlRenderer::InitializeSessionId (bool* pbUpdateSessionId, bool* pbUpdateCookie) {

	int iErrCode, iOptions;

	// Check for force reset
	iErrCode = g_pGameEngine->GetEmpireOptions (m_iEmpireKey, &iOptions);
	if (iErrCode != OK) {
		return iErrCode;
	}

	if (iOptions & RESET_SESSION_ID) {

		iErrCode = g_pGameEngine->GetNewSessionId (&m_i64SessionId);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		iErrCode = g_pGameEngine->EndResetEmpireSessionId (m_iEmpireKey);
		if (iErrCode != OK) {
			Assert (false);
			return iErrCode;
		}

		*pbUpdateSessionId = *pbUpdateCookie = true;
		return OK;
	}
	
	*pbUpdateSessionId = *pbUpdateCookie = false;
	
	// First, get the empire's session id
	iErrCode = g_pGameEngine->GetEmpireSessionId (m_iEmpireKey, &m_i64SessionId);
	if (iErrCode != OK) {
		return iErrCode;
	}

	// Now, see if the request brought a cookie with a session id
	ICookie* pCookie = m_pHttpRequest->GetCookie ("SessionId");
	int64 i64CookieSessionId = NO_SESSION_ID;
	
	if (pCookie != NULL) {
		i64CookieSessionId = pCookie->GetInt64Value();
	}
	
	if (i64CookieSessionId == NO_SESSION_ID) {
		
		if (m_i64SessionId == NO_SESSION_ID) {
			
			// Generate a new session id
			iErrCode = g_pGameEngine->GetNewSessionId (&m_i64SessionId);
			if (iErrCode != OK) {
				Assert (false);
				return iErrCode;
			}
			
			*pbUpdateSessionId = true;
		}
		
		*pbUpdateCookie = true;
		
	} else {
		
		// We have a cookie with a valid value
		if (i64CookieSessionId != m_i64SessionId) {
			
			// For some reason the cookie has a different value than the stored value
			// This could happen if a player is on a new machine and uses a different empire
			// to generate a new value, uses the original empire, then reverts to the old machine 
			// with the old empire.  We'll believe the cookie's value, in this case, to ensure that
			// all empires on the same machine have the same session id
			//
			// This decision will have to be revised if we absolutely require that session id's be unique
			// in the future for some feature or other, since people can easily edit their cookie files and 
			// change the values
			
			m_i64SessionId = i64CookieSessionId;
			
			// Write the empire's new session id
			*pbUpdateSessionId = true;
		}
	}
	
	return iErrCode;
}



bool HtmlRenderer::RedirectOnSubmit (PageId* ppageRedirect) {
	
	if (WasButtonPressed (PageButtonId[m_pgPageId])) {
		return false;
	}
	
	if (WasButtonPressed (BID_ACTIVEGAMELIST)) {
		*ppageRedirect = ACTIVE_GAME_LIST;
		return true;
	}
	
	if (WasButtonPressed (BID_OPENGAMELIST)) {
		*ppageRedirect = OPEN_GAME_LIST;
		return true;
	}
	
	if (WasButtonPressed (BID_SYSTEMGAMELIST)) {
		*ppageRedirect = SYSTEM_GAME_LIST;
		return true;
	}
	
	if (WasButtonPressed (BID_PROFILEVIEWER)) {
		*ppageRedirect = PROFILE_VIEWER;
		return true;
	}
	
	if (WasButtonPressed (BID_PROFILEEDITOR)) {
		*ppageRedirect = PROFILE_EDITOR;
		return true;
	}
	
	if (WasButtonPressed (BID_TOPLISTS)) {
		*ppageRedirect = TOP_LISTS;
		return true;
	}

	if (WasButtonPressed (BID_SPECTATORGAMES)) {
		*ppageRedirect = SPECTATOR_GAMES;
		return true;
	}
	
	if (WasButtonPressed (BID_LATESTNUKES)) {
		*ppageRedirect = LATEST_NUKES;
		return true;
	}
	
	if (WasButtonPressed (BID_CHATROOM)) {
		*ppageRedirect = CHATROOM;
		return true;
	}
	
	if (WasButtonPressed (BID_EXIT)) {
		*ppageRedirect = LOGIN;
		return true;
	}
	
	if (m_iPrivilege >= ADEPT) {
		
		if (WasButtonPressed (BID_PERSONALGAMECLASSES)) {
			*ppageRedirect = PERSONAL_GAME_CLASSES;
			return true;
		}
	}
	
	if (m_iPrivilege >= ADMINISTRATOR) {
		
		if (WasButtonPressed (BID_SERVERADMINISTRATOR)) {
			*ppageRedirect = SERVER_ADMINISTRATOR;
			return true;
		}
		
		if (WasButtonPressed (BID_GAMEADMINISTRATOR)) {
			*ppageRedirect = GAME_ADMINISTRATOR;
			return true;
		}
		
		if (WasButtonPressed (BID_EMPIREADMINISTRATOR)) {
			*ppageRedirect = EMPIRE_ADMINISTRATOR;
			return true;
		}
		
		if (WasButtonPressed (BID_THEMEADMINISTRATOR)) {
			*ppageRedirect = THEME_ADMINISTRATOR;
			return true;
		}
	}
	
	if (WasButtonPressed (BID_SERVERRULES)) {
		*ppageRedirect = SYSTEM_SERVER_RULES;
		return true;
	}
	
	if (WasButtonPressed (BID_FAQ)) {
		*ppageRedirect = SYSTEM_FAQ;
		return true;
	}
	
	if (WasButtonPressed (BID_SERVERNEWS)) {
		*ppageRedirect = SYSTEM_NEWS;
		return true;
	}
	
	IHttpForm* pHttpForm = m_pHttpRequest->GetForm ("ProfileLink.x");
	if (pHttpForm != NULL) {
		
		m_iReserved = m_iEmpireKey;
		*ppageRedirect = PROFILE_VIEWER;
		return true;
	}
	
	if (NotifiedProfileLink()) {
		
		pHttpForm = m_pHttpRequest->GetFormBeginsWith ("ProfileLink");
		if (pHttpForm != NULL) {
			
			const char* pszProfile = pHttpForm->GetName();
			
			Assert (pszProfile != NULL);
			
			int iViewProfileEmpireKey;
			unsigned int iHash;
			
			if (sscanf (pszProfile, "ProfileLink.%d.%d.x", &iViewProfileEmpireKey, &iHash) == 2 &&
				VerifyEmpireNameHash (iViewProfileEmpireKey, iHash)) {
				
				m_iReserved = iViewProfileEmpireKey;
				*ppageRedirect = PROFILE_VIEWER;
				return true;
				
			} else {
				
				AddMessage ("That empire no longer exists");
			}
		}
	}
	
	return false;
}


void HtmlRenderer::CloseSystemPage() {
	
	String strFilter;
	
	int iButtonKey = m_iButtonKey, iPrivilege = m_iPrivilege;
	
	OutputText ("<p>");
	WriteSeparatorString (m_iSeparatorKey);
	OutputText ("<p>");
	
	if (m_bRepeatedButtons) {
		WriteSystemButtons (iButtonKey, iPrivilege);
	}
	
	Variant vAdminEmail;
	int iErrCode = g_pGameEngine->GetAdministratorEmailAddress (&vAdminEmail);
	if (iErrCode == OK) {
		
		OutputText ("<p><strong>Contact <a href=\"mailto:");
		
		if (HTMLFilter (vAdminEmail.GetCharPtr(), &strFilter, 0, false) == OK) {
			m_pHttpResponse->WriteText (strFilter.GetCharPtr());
		}
		OutputText ("\">root</a> if you have problems or suggestions</strong>");
	}
	
	if (m_pgPageId != LOGIN && m_pgPageId != NEW_EMPIRE) {
		
		OutputText ("<p>");
		
		WriteButton (BID_SERVERNEWS);
		WriteButton (BID_SERVERRULES);
		WriteButton (BID_FAQ);
	}
	
	OutputText ("<p><strong><font size=\"3\">");
	WriteVersionString();
	OutputText ("<br>Script time: ");
	
	MilliSeconds msTime = GetTimerCount();
	
	OnPageRender (msTime);
	
	m_pHttpResponse->WriteText ((int) msTime);
	OutputText (" ms</font></strong></center></form></body></html>");
}

void HtmlRenderer::PostSystemPageInformation() {
	
	uint64 iPasswordHash = GetPasswordHash();
	
	OutputText ("<input type=\"hidden\" name=\"EmpireKey\" value=\"");
	m_pHttpResponse->WriteText (m_iEmpireKey);
	OutputText ("\"><input type=\"hidden\" name=\"Password\" value=\"");
	m_pHttpResponse->WriteText (iPasswordHash);
	OutputText ("\">");
}

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

void HtmlRenderer::WriteGameListHeader (const char** ppszHeaders, size_t stNumHeaders, const char* pszTableColor) {
	
	size_t i;
	
	// Setup string
	OutputText ("<p><center><table border=\"1\" bordercolor=\"");
	m_pHttpResponse->WriteText (pszTableColor);
	OutputText ("\" width=\"90%\" cellspacing=\"1\" cellpadding=\"2\"><tr>");
	
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
		iSecondsSince, iSecondsUntil, iState, iGameOptions, iNumUnreadMessages, iMaxNumEmpiresEver,
		iNumUpdatesBeforeGameCloses, iNumUpdatedEmpires = 0;
	
	bool bReadyForUpdate, bOpen;

	Variant* pvEmpireKey = NULL, pvMin [NUM_ENTRY_SCORE_RESTRICTIONS], pvMax [NUM_ENTRY_SCORE_RESTRICTIONS];

	UTCTime tCreationTime;

	char pszDateString [OS::MaxDateLength], pszLogin[64];
	const char* pszEmpires;

	//
	// Read data
	//

	iErrCode = g_pGameEngine->GetEmpireOption (iGameClass, iGameNumber, iEmpireKey, UPDATED, &bReadyForUpdate);
	if (iErrCode != OK) {
		goto Cleanup;
	}
	
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
	
	iErrCode = g_pGameEngine->GetMaxNumActiveEmpiresInGame (iGameClass, iGameNumber, &iMaxNumEmpiresEver);
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
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::Description].GetCharPtr());
	
	// Name
	OutputText ("</font></td></tr><tr><td width=\"20%\"><font size=\"3\">");
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::Name].GetCharPtr());
	
	if (pvGameClassInfo[SystemGameClassData::Owner].GetInteger() != SYSTEM) {
		OutputText (" (");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::OwnerName].GetCharPtr());
		OutputText (")");
	}
	
	OutputText ("<strong> ");
	m_pHttpResponse->WriteText (iGameNumber);
	OutputText ("</strong></font>");
	
	// Time
	OutputText ("<p><font size=\"-1\">(Started <strong>");
	m_pHttpResponse->WriteText (pszDateString);
	OutputText ("</strong>)</font>");
	
	// Login
	sprintf (pszLogin, "Login%i.%i", iGameClass, iGameNumber);
	
	OutputText ("<p>");
	WriteButtonString (iButtonKey, "Login", "Login", pszLogin);
	
	// Ready for update
	OutputText ("</td><td align=\"center\">");
	
	if (!(iState & STARTED)) {
		OutputText ("<font size=\"2\">N/A");
	} else {

		OutputText ("<font size=\"4\" color=\"#");
		if (bReadyForUpdate) {
			m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
			OutputText ("\">Yes");
		} else {
			m_pHttpResponse->WriteText (m_vBadColor.GetCharPtr());
			OutputText ("\">No");
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
		int iNumNeeded = pvGameClassInfo[SystemGameClassData::MinNumEmpires] - iNumActiveEmpires;
		
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
			OutputText (" (paused by an administrator)");
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
	if (iNumActiveEmpires == 1) {
		pszEmpires = "empire ";
	} else {
		pszEmpires = "empires ";
	}

	if (iGameOptions & GAME_NAMES_LISTED) {

		Variant vTemp;
		String strList;
		
		// Empire names may fail; halted empire quits
		int iLoopGuard = iNumActiveEmpires - 1;
		for (i = 0; i < iLoopGuard; i ++) {
			
			iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[i].GetInteger(), &vTemp);
			if (iErrCode == OK) {
				strList += vTemp.GetCharPtr();
				strList += ", ";
			}
		}
		iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[i].GetInteger(), &vTemp);
		if (iErrCode == OK) {
			strList += vTemp.GetCharPtr();
		}
		
		if (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger() != 
			pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger()) {
			
			OutputText ("<td align=\"center\"><font size=\"2\"><strong>");
			m_pHttpResponse->WriteText (iNumActiveEmpires);
			OutputText ("</strong> ");
			m_pHttpResponse->WriteText (pszEmpires);
			OutputText ("<br>(");
			m_pHttpResponse->WriteText (strList.GetCharPtr(), strList.GetLength());
			OutputText (")<br><strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger());
			OutputText ("</strong> to <strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger());
			OutputText ("</strong> required");
			
		} else {
			
			OutputText ("<td align=\"center\"><font size=\"2\"><strong>");
			m_pHttpResponse->WriteText (iNumActiveEmpires);
			OutputText ("</strong> ");
			m_pHttpResponse->WriteText (pszEmpires);
			OutputText ("<br>(");
			m_pHttpResponse->WriteText (strList.GetCharPtr(), strList.GetLength());
			OutputText (")<br><strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger());
			OutputText ("</strong> required");
		}
		
	} else {
		
		if (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger() != 
			pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger()) {
			
			OutputText ("<td align=\"center\"><font size=\"2\"><strong>");
			m_pHttpResponse->WriteText (iNumActiveEmpires);
			OutputText ("</strong> ");
			m_pHttpResponse->WriteText (pszEmpires);
			OutputText ("<br><strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger());
			OutputText ("</strong> to <strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger());
			OutputText ("</strong> required");
			
		} else {
			
			OutputText ("<td align=\"center\"><font size=\"2\"><strong>");
			m_pHttpResponse->WriteText (iNumActiveEmpires);
			OutputText ("</strong> ");
			m_pHttpResponse->WriteText (pszEmpires);
			OutputText ("<br><strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger());
			OutputText ("</strong> required");
		}
	}

	AddBridierGame (iGameClass, iGameNumber, pvGameClassInfo, iGameOptions, true);
	
	OutputText ("</font></td>");
	
	// Features
	OutputText ("<td align=\"center\" width=\"10%\"><font size=\"2\">");
	WriteTime (pvGameClassInfo[SystemGameClassData::NumSecPerUpdate].GetInteger());
	
	OutputText (" updates<br><strong>");
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinNumPlanets].GetInteger());
	OutputText ("</strong> ");
	
	if (pvGameClassInfo[SystemGameClassData::MaxNumPlanets].GetInteger() == 
		pvGameClassInfo[SystemGameClassData::MinNumPlanets].GetInteger()) {
		
		OutputText ("planet");
		if (pvGameClassInfo[SystemGameClassData::MaxNumPlanets].GetInteger() != 1) {
			OutputText ("s");
		}
		
	} else {
		
		OutputText ("to <strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumPlanets].GetInteger());
		OutputText ("</strong> planets");
	}
	OutputText ("<br><strong>");
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::InitialTechLevel].GetFloat());
	OutputText ("</strong> initial tech<br><strong>");
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxTechDev].GetFloat());
	OutputText ("</strong> delta tech</font></td>");
	
	// Dip
	AddDiplomacy (pvGameClassInfo);

	// Resources
	AddResources (pvGameClassInfo);

	// Bridier, Score
	AddBridier (iGameOptions, pvMin, pvMax);
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

int HtmlRenderer::WriteOpenGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, 
										 bool bAdmin) {

	return WriteInPlayGameListData (iGameClass, iGameNumber, pvGameClassInfo, bAdmin, false);
}

int HtmlRenderer::WriteSpectatorGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo) {

	return WriteInPlayGameListData (iGameClass, iGameNumber, pvGameClassInfo, false, true);
}

int HtmlRenderer::WriteInPlayGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, 
										   bool bAdmin, bool bSpectators) {

	int iErrCode, iNumEmpiresInGame, iGameOptions;

	String strList;
	char pszEnter [64];

	iErrCode = g_pGameEngine->GetGameOptions (iGameClass, iGameNumber, &iGameOptions);
	if (iErrCode != OK) {
		goto Cleanup;
	}
	
	if (iGameOptions & GAME_NAMES_LISTED) {

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
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::Description].GetCharPtr());
	
	// Name
	OutputText ("</font></td></tr><tr><td><font size=\"3\">");
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::Name].GetCharPtr());
	
	if (pvGameClassInfo[SystemGameClassData::Owner].GetInteger() != SYSTEM) {
		OutputText (" (");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::OwnerName].GetCharPtr());
		OutputText (")");
	}
	
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
			WriteButtonString (m_iButtonKey, ButtonName[BID_ENTERGAME], ButtonText[BID_ENTERGAME], pszEnter);

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
	
	if (bSpectators) {

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

		OutputText ("</font>");
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
			OutputText ("</font> (paused by an administrator)");
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
		strList.GetCharPtr(), 
		iNumEmpiresInGame,
		bAdmin,
		bSpectators
		);

Cleanup:
	
	return iErrCode;
}


int HtmlRenderer::WriteSystemGameListData (int iGameClass, const Variant* pvGameClassInfo) {
	
	int iOwner = pvGameClassInfo[SystemGameClassData::Owner].GetInteger();
	
	// Description
	OutputText (
		"<tr><td colspan=\"13\" align=\"center\">"\
		"<font size=\"-1\" face=\"" DEFAULT_GAMECLASS_DESCRIPTION_FONT "\"><br>"
		);
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::Description].GetCharPtr());
	
	// Name
	OutputText ("</font></td></tr><tr><td><font size=\"3\">");
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::Name].GetCharPtr());
	
	if (pvGameClassInfo[SystemGameClassData::Owner].GetInteger() != SYSTEM) {
		OutputText (" (");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::OwnerName].GetCharPtr());
		OutputText (")");
	}
	
	// Game number, start button / halted
	char pszForm[256];
	
	OutputText (" <strong>");
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::OpenGameNum].GetInteger());
	OutputText ("</strong></font>");
	
	if (m_iPrivilege >= NOVICE) {
		
		if (!(pvGameClassInfo[SystemGameClassData::Options].GetInteger() & GAMECLASS_HALTED) &&
			!(pvGameClassInfo[SystemGameClassData::Options].GetInteger() & GAMECLASS_MARKED_FOR_DELETION) &&
			(pvGameClassInfo[SystemGameClassData::MaxNumActiveGames].GetInteger() == INFINITE_ACTIVE_GAMES ||
			pvGameClassInfo[SystemGameClassData::NumActiveGames].GetInteger() < 
			pvGameClassInfo[SystemGameClassData::MaxNumActiveGames].GetInteger())) {
			
			OutputText ("<p>");
			
			sprintf (pszForm, "Start%i", iGameClass);
			WriteButtonString (m_iButtonKey, "StartGame", "Start", pszForm);
			
			if (iOwner != SYSTEM && (iOwner == m_iEmpireKey || m_iPrivilege >= ADMINISTRATOR)) {
				
				OutputText ("<br>");
				
				sprintf (pszForm, "HaltGameClass%i", iGameClass);
				WriteButtonString (m_iButtonKey, "HaltGameClass", "Halt GameClass", pszForm);
				
				OutputText ("<br>");
				
				sprintf (pszForm, "DeleteGameClass%i", iGameClass);
				WriteButtonString (m_iButtonKey, "DeleteGameClass", "Delete GameClass", pszForm);
			}
			
			// Password protection
			OutputText ("<br>Advanced:<input type=\"checkbox\" name=\"Advanced");
			m_pHttpResponse->WriteText (iGameClass);
			OutputText ("\">");
			
		} else {
			
			if ((pvGameClassInfo[SystemGameClassData::Options].GetInteger() & GAMECLASS_HALTED) != 0 &&
				(pvGameClassInfo[SystemGameClassData::Options].GetInteger() & GAMECLASS_MARKED_FOR_DELETION) != 0) {
				OutputText (" (<strong>Halted and marked for deletion</strong>)");
			}
			
			else if (pvGameClassInfo[SystemGameClassData::Options].GetInteger() & GAMECLASS_HALTED) {
				OutputText (" (<strong>Halted</strong>)");
			}
			
			else if (pvGameClassInfo[SystemGameClassData::Options].GetInteger() & GAMECLASS_MARKED_FOR_DELETION) {
				OutputText (" (<strong>Marked for deletion</strong>)");
			}
			
			if (iOwner != SYSTEM && (iOwner == m_iEmpireKey || m_iPrivilege >= ADMINISTRATOR)) {
				
				OutputText ("<p>");
				
				if (pvGameClassInfo[SystemGameClassData::Options].GetInteger() & GAMECLASS_HALTED) {
					
					sprintf (pszForm, "UnhaltGameClass%i", iGameClass);
					WriteButtonString (m_iButtonKey, "UnhaltGameClass", "Unhalt GameClass", pszForm);
					
					if (pvGameClassInfo[SystemGameClassData::Options].GetInteger() & GAMECLASS_MARKED_FOR_DELETION) {
						
						OutputText ("<br>");
						
						sprintf (pszForm, "UndeleteGameClass%i", iGameClass);
						WriteButtonString (m_iButtonKey, "UndeleteGameClass", "Undelete GameClass", pszForm);
						
					} else {
						
						OutputText ("<br>");
						
						sprintf (pszForm, "DeleteGameClass%i", iGameClass);
						WriteButtonString (m_iButtonKey, "DeleteGameClass", "Delete GameClass", pszForm);
					}
					
				} else {
					
					sprintf (pszForm, "HaltGameClass%i", iGameClass);
					WriteButtonString (m_iButtonKey, "HaltGameClass", "Halt GameClass", pszForm);
					
					OutputText ("<br>");
					
					sprintf (pszForm, "UndeleteGameClass%i", iGameClass);
					WriteButtonString (m_iButtonKey, "UndeleteGameClass", "Undelete GameClass", pszForm);
				}
			}
		}
		
		if (pvGameClassInfo[SystemGameClassData::MaxNumActiveGames].GetInteger() != INFINITE_ACTIVE_GAMES) {
			
			if (pvGameClassInfo[SystemGameClassData::NumActiveGames].GetInteger() >= 
				pvGameClassInfo[SystemGameClassData::MaxNumActiveGames].GetInteger()) {
				
				OutputText ("<p>(Active game limit of <strong>");
				m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumActiveGames].GetInteger());
				OutputText ("</strong> reached)");
				
			} else {
				
				OutputText ("<p>(<strong>");
				m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::NumActiveGames].GetInteger());
				OutputText ("</strong> of <strong>");
				m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumActiveGames].GetInteger());
				OutputText ("</strong> active games)");
			}
		}
	}
	
	OutputText ("</td>");
	
	return AddGameClassDescription (SYSTEM_GAME_LIST, pvGameClassInfo, NO_KEY, NO_KEY, NULL, 0, false, false);
}


int HtmlRenderer::AddGameClassDescription (int iWhichList, const Variant* pvGameClassInfo, 
										   int iGameClass, int iGameNumber, const char* pszEmpiresInGame, 
										   int iNumEmpiresInGame, bool bAdmin, bool bSpectators) {
	
	int iErrCode, iGameOptions = 0;

	if (iWhichList == OPEN_GAME_LIST) {

		Assert (iGameClass != NO_KEY);

		iErrCode = g_pGameEngine->GetGameOptions (iGameClass, iGameNumber, &iGameOptions);
		if (iErrCode != OK) {
			return iErrCode;
		}
	}

	// Time	
	OutputText ("<td align=\"center\"><font size=\"2\">");
	WriteTime (pvGameClassInfo[SystemGameClassData::NumSecPerUpdate].GetInteger());
	
	if (iWhichList == OPEN_GAME_LIST) {
		
		Seconds sDelay;
		int iErrCode = g_pGameEngine->GetFirstUpdateDelay (iGameClass, iGameNumber, &sDelay);
		if (iErrCode == OK && sDelay > 0) {
			
			OutputText ("<p>(");
			WriteTime (sDelay);
			OutputText (" delay)");
		}
	}
	
	OutputText ("</font></td><td align=\"center\"><font size=\"2\">");
	
	// Empires
	if (iWhichList == OPEN_GAME_LIST) {
		
		char* pszEmpires;
		if (iNumEmpiresInGame == 1) {
			pszEmpires = "empire";
		} else {
			pszEmpires = "empires";
		}
		
		if (iGameOptions & GAME_NAMES_LISTED) {
			
			Assert (pszEmpiresInGame != NULL);
			
			if (iNumEmpiresInGame != pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger()) {
				
				OutputText ("<strong>");
				m_pHttpResponse->WriteText (iNumEmpiresInGame);
				OutputText ("</strong> ");
				m_pHttpResponse->WriteText (pszEmpires);
				OutputText ("<br>(");
				m_pHttpResponse->WriteText (pszEmpiresInGame);
				OutputText (")");
				OutputText ("<br><strong>");
				m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger());
				
				if (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger() != 
					pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger()) {
					
					OutputText ("</strong> to <strong>");
					m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger());
				}
				OutputText ("</strong> required");
				
			} else {
				
				OutputText ("Full game: <strong>");
				m_pHttpResponse->WriteText (iNumEmpiresInGame);
				OutputText ("</strong> ");
				m_pHttpResponse->WriteText (pszEmpires);
				OutputText ("<br>(");
				m_pHttpResponse->WriteText (pszEmpiresInGame);
				OutputText (")");
			}
			
		} else {
			
			if (iNumEmpiresInGame != pvGameClassInfo[SystemGameClassData::MaxNumEmpires]) {
				
				OutputText ("<strong>");
				m_pHttpResponse->WriteText (iNumEmpiresInGame);
				OutputText ("</strong> ");
				m_pHttpResponse->WriteText (pszEmpires); 
				OutputText ("<br><strong>");
				m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger());
				
				if (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger() != 
					pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger()) {
					OutputText ("</strong> to <strong>");
					m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger());
				}
				OutputText ("</strong> required");
				
			} else {
				
				OutputText ("Full game: <strong>");
				m_pHttpResponse->WriteText (iNumEmpiresInGame);
				OutputText ("</strong> ");
				m_pHttpResponse->WriteText (pszEmpires);
			}
		}

		AddBridierGame (iGameClass, iGameNumber, pvGameClassInfo, iGameOptions, !bAdmin && !bSpectators);
		
	} else {
		
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger());
		
		if (pvGameClassInfo[SystemGameClassData::MinNumEmpires].GetInteger() != 
			pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger()) {
			
			OutputText ("</strong> to <strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger());		
		}
		OutputText ("</strong> empires");
	}
	
	OutputText ("</font></td><td align=\"center\"><font size=\"2\"><strong>");
	
	
	// Planets per empire
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinNumPlanets].GetInteger());
	
	if (pvGameClassInfo[SystemGameClassData::MaxNumPlanets].GetInteger() == 
		pvGameClassInfo[SystemGameClassData::MinNumPlanets].GetInteger()) {
		
		OutputText ("</strong> planet");
		
		if (pvGameClassInfo[SystemGameClassData::MaxNumPlanets].GetInteger() == 1) {
			OutputText (" per empire");
		} else {
			OutputText ("s per empire");
		}
		
	} else {
		
		OutputText ("</strong> to <strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumPlanets].GetInteger());
		OutputText ("</strong> planets per empire");
	}
    
	// Tech
	OutputText ("</font></td><td align=\"center\"><font size=\"2\"><strong>");
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::InitialTechLevel].GetFloat());
	OutputText ("</strong> initial<br><strong>");
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxTechDev].GetFloat());
	
	OutputText ("</strong> delta</font></td>");
	
	// Dip
	AddDiplomacy (pvGameClassInfo);

	// Resources
	AddResources (pvGameClassInfo);

	// Init techs
	AddTechList (pvGameClassInfo[SystemGameClassData::InitialTechDevs].GetInteger());
	
	// Dev techs
	AddTechList (pvGameClassInfo[SystemGameClassData::DevelopableTechDevs].GetInteger());

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
			
			AddBridier (iOptions, pvMin, pvMax);
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

	if (pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger() == 2 &&
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
				if (iLoss > 0) {
					OutputText ("-");
				}
				m_pHttpResponse->WriteText (iLoss);
				OutputText ("</strong>");
			}
		}

		OutputText ("</font>");
	}
}

void HtmlRenderer::AddOptions (int iWhichList, const Variant* pvGameClassInfo, int iGameOptions) {
	
	int iGameClassOptions = pvGameClassInfo[SystemGameClassData::Options].GetInteger();
	
	// Weekend
	m_pHttpResponse->WriteText (
		"</td><td align=\"center\" width=\"160\"><table><tr><td align=\"center\"><font size=\"2\">"
		);
	
	if (iGameClassOptions & WEEKEND_UPDATES) {
		OutputText ("Weekend");
	} else {
		OutputText ("<strike>Weekend</strike>");
	}

	// Spectators
	if (iWhichList != SYSTEM_GAME_LIST) {
		
		if (iGameOptions & GAME_ALLOW_SPECTATORS) {
			OutputText ("<br>Spectators");
		} else {
			OutputText ("<br><strike>Spectators</strike>");
		}
	}		
	
	// Visible builds
	if (iGameClassOptions & VISIBLE_BUILDS) {
		OutputText ("<br>VisibleBuilds");
	} else {
		OutputText ("<br><strike>VisibleBuilds</strike>");
	}
	
	// Visible diplomacy
	if (iGameClassOptions & VISIBLE_DIPLOMACY) {
		OutputText ("<br>VisibleDip");
	} else {
		OutputText ("<br><strike>VisibleDip</strike>");
	}
	
	// Pop to build
	OutputText ("<br><strong>");
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::BuilderPopLevel].GetInteger());
	OutputText ("</strong> BuildPop");
	
	// Max ag ratio
	OutputText ("<br><strong>");
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxAgRatio].GetFloat());
	OutputText ("</strong> MaxAgRatio");

	// Ship flags
	if (iGameClassOptions & USE_FRIENDLY_GATES) {
		OutputText ("<br>FriendlyGates");
	} else {
		OutputText ("<br><strike>FriendlyGates</strike>");
	}

	if (!(iGameClassOptions & DISABLE_SUICIDAL_DOOMSDAYS)) {
		OutputText ("<br>SuicidalDooms");
	} else {
		OutputText ("<br><strike>SuicidalDooms</strike>");
	}

	if (iGameClassOptions & USE_UNFRIENDLY_DOOMSDAYS) {
		OutputText ("<br>UnfriendlyDooms");
	} else {
		OutputText ("<br><strike>UnfriendlyDooms</strike>");
	}
	
	if (iGameClassOptions & USE_CLASSIC_DOOMSDAYS) {
		OutputText ("<br>PermanentDooms");
	} else {
		OutputText ("<br><strike>PermanentDooms</strike>");
	}

	OutputText ("</td><td align=\"center\"><font size=\"2\">");
	
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
	if (pvGameClassInfo[SystemGameClassData::MapsShared] != NO_DIPLOMACY) {
		OutputText ("MapShared (at <strong>");
		m_pHttpResponse->WriteText (DIP_STRING (pvGameClassInfo[SystemGameClassData::MapsShared].GetInteger()));
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
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::NumUpdatesForIdle].GetInteger());
	OutputText ("</strong> IdleUpdate");
	
	if (pvGameClassInfo[SystemGameClassData::NumUpdatesForIdle].GetInteger() > 1) {
		OutputText ("s");
	}
	
	// Ruins
	switch (pvGameClassInfo[SystemGameClassData::RuinFlags].GetInteger()) {
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
	
	if (pvGameClassInfo[SystemGameClassData::RuinFlags].GetInteger() != 0) {
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::NumUpdatesForRuin].GetInteger());
		OutputText ("</strong> updates)");
	}
	
	OutputText ("</td></tr></table></td>");
}

void HtmlRenderer::AddResources (const Variant* pvGameClassInfo) {
	
	OutputText ("<td align=\"center\" width=\"14%\"><font size=\"2\"><strong>");
	
	//
	// Planet Res
	//
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinAvgAg].GetInteger());
	if (pvGameClassInfo[SystemGameClassData::MaxAvgAg] != 
		pvGameClassInfo[SystemGameClassData::MinAvgAg]) {
		
		OutputText ("</strong>-<strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxAvgAg].GetInteger());
	}
	OutputText ("</strong> Ag<br><strong>");
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinAvgMin].GetInteger());
	if (pvGameClassInfo[SystemGameClassData::MaxAvgMin] != 
		pvGameClassInfo[SystemGameClassData::MinAvgMin]) {
		
		OutputText ("</strong>-<strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxAvgMin].GetInteger());
	}
	OutputText ("</strong> Min<br><strong>");
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinAvgFuel].GetInteger());
	if (pvGameClassInfo[SystemGameClassData::MaxAvgFuel] != 
		pvGameClassInfo[SystemGameClassData::MinAvgFuel]) {
		
		OutputText ("</strong>-<strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxAvgFuel].GetInteger());
	}
	OutputText ("</strong> Fuel<br><strong>");
	
	//
	// HW
	//
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinAgHW].GetInteger());
	if (pvGameClassInfo[SystemGameClassData::MaxAgHW] != 
		pvGameClassInfo[SystemGameClassData::MinAgHW]) {
		
		OutputText ("</strong>-<strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxAgHW].GetInteger());
	}
	OutputText ("</strong> HWAg<br><strong>");
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinMinHW].GetInteger());
	if (pvGameClassInfo[SystemGameClassData::MaxMinHW] != 
		pvGameClassInfo[SystemGameClassData::MinMinHW]) {
		
		OutputText ("</strong>-<strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxMinHW].GetInteger());
	}
	OutputText ("</strong> HWMin<br><strong>");
	
	m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MinFuelHW].GetInteger());
	if (pvGameClassInfo[SystemGameClassData::MaxFuelHW] != 
		pvGameClassInfo[SystemGameClassData::MinFuelHW]) {
		
		OutputText ("</strong>-<strong>");
		m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxFuelHW].GetInteger());
	}
	OutputText ("</strong> HWFuel</td>");
}

void HtmlRenderer::AddDiplomacy (const Variant* pvGameClassInfo) {
	
	int iOptions = pvGameClassInfo[SystemGameClassData::Options].GetInteger();
	
	OutputText ("<td align=\"center\" width=\"75\"><font size=\"2\"><strong>" WAR_STRING "</strong>");
	
	int iDip = pvGameClassInfo[SystemGameClassData::DiplomacyLevel].GetInteger();
	
	if (g_pGameEngine->GameAllowsDiplomacy (iDip, TRUCE)) {
		OutputText ("<br><strong>" TRUCE_STRING "</strong>");
		
		switch (pvGameClassInfo[SystemGameClassData::MaxNumTruces].GetInteger()) {
			
		case UNRESTRICTED_DIPLOMACY:
			break;		
			
		case FAIR_DIPLOMACY:
			
			OutputText (" (Fair)");
			break;
			
		default:
			
			OutputText (" (<strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumTruces].GetInteger());
			OutputText ("</strong>)");
			break;
		}
	}
	
	if (g_pGameEngine->GameAllowsDiplomacy (iDip, TRADE)) {
		OutputText ("<br><strong>" TRADE_STRING "</strong>");
		
		switch (pvGameClassInfo[SystemGameClassData::MaxNumTrades].GetInteger()) {
			
		case UNRESTRICTED_DIPLOMACY:
			break;		
			
		case FAIR_DIPLOMACY:
			
			OutputText (" (Fair)");
			break;
			
		default:
			
			OutputText (" (<strong>");
			m_pHttpResponse->WriteText (pvGameClassInfo[SystemGameClassData::MaxNumTrades].GetInteger());
			OutputText ("</strong>)");
			break;
		}
	}
	
	if (g_pGameEngine->GameAllowsDiplomacy (iDip, ALLIANCE)) {
		OutputText ("<br><strong>" ALLIANCE_STRING "</strong>");
		
		bool bOpened = true;
		int iMax = pvGameClassInfo[SystemGameClassData::MaxNumAlliances].GetInteger();
		
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
	
	if (g_pGameEngine->GameAllowsDiplomacy (iDip, DRAW)) {
		OutputText ("<br><strong>" DRAW_STRING "</strong>");
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
	
	OutputText ("</font></td>");
}

void HtmlRenderer::AddTechList (int iTechs) {

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

void HtmlRenderer::AddBridier (int iOptions, const Variant* pvMin, const Variant* pvMax) {
		
	bool bText = false;
	
	// Bridier
	OutputText ("<td align=\"center\"><font size=\"2\">");
	
	if (iOptions & GAME_COUNT_FOR_BRIDIER) {
		
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
		
		// Bridier Rank Gain
		if ((iOptions & (GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN | GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN)) == 
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
		
		else if (iOptions & GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN) {
			
			if (bText) {
				OUTPUT_TEXT_SEPARATOR;
			} else {
				bText = true;
			}
			
			OutputText ("At least <strong>");
			m_pHttpResponse->WriteText (pvMin [RESTRICT_BRIDIER_RANK_GAIN].GetInteger());
			OutputText ("</strong> Rank Gain");
		}
		
		else if (iOptions & GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN) {
			
			if (bText) {
				OUTPUT_TEXT_SEPARATOR;
			} else {
				bText = true;
			}
			
			OutputText ("At most <strong>");
			m_pHttpResponse->WriteText (pvMax [RESTRICT_BRIDIER_RANK_GAIN].GetInteger());
			OutputText ("</strong> Rank Gain");
		}
		
		if (!bText) {
			OutputText ("Any empire");
		}
		
	} else {
		
		OutputText ("N/A");
	}
		
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
	
	else if (!bDisplayed) {
		OutputText ("None");
	}
	
	OutputText ("</font></td>");
}

void HtmlRenderer::WriteCreateGameClassString (int iEmpireKey, bool bPersonalGame) {
	
	int* piSuperClassKey, iNumSuperClasses, i, iErrCode;
	Variant* pvSuperClassName;

	IHttpForm* pHttpForm;

	// Conserve last settings variables
	String strName, strDesc;

	int iSelSuperClass = NO_KEY, iSelNumActiveGames, iSelMinNumEmpires, iSelMaxNumEmpires, 
		iSelMinNumPlanets, iSelMaxNumPlanets, iSelHoursPU, iSelMinsPU, iSelSecsPU,
		iSelMinAg, iSelMaxAg, iSelMinMin, iSelMaxMin, iSelMinFuel, iSelMaxFuel,
		iSelMinHWAg, iSelMaxHWAg, iSelMinHWMin, iSelMaxHWMin, iSelMinHWFuel, iSelMaxHWFuel,
		iSelPopLevel, iSelDip, iSelSurrenders, iSelMaxNumTruces, iSelStaticTruces,
		iSelMaxNumTrades, iSelStaticTrades, iSelMaxNumAllies, iSelStaticAllies,
		iSelUpdatesIdle, iSelRuins, iSelUpdatesRuin, iSelInitShip, iSelDevShip,
		iSelMapExposed, iSelDipShareLevel;

	float fSelInitTechLevel, fSelTechIncrease, fSelMaxAgRatio;

	bool bSelActiveGames, bSelWeekend, bSelDraws, bSelPermanentAlliances, bSelBreakAlliances,
		bSelSubjective, bFriendlyGates, bSuicidalDoomsdays, bUnfriendlyDoomsdays, bClassicDoomsdays,
		bSelIndependence, bSelPrivate, bSelVisibleBuilds, bSelVisibleDiplomacy, bSelDipExposed, 
		bSelDisconnectedMaps;


	if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassName")) != NULL) {
		if (HTMLFilter (pHttpForm->GetValue(), &strName, 0, false) != OK) {
			strName = "";
		}
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassDescription")) != NULL) {
		if (HTMLFilter (pHttpForm->GetValue(), &strDesc, 0, false) != OK) {
			strDesc = "";
		}
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("SuperClassKey")) != NULL) {
		iSelSuperClass = pHttpForm->GetIntValue();
	}

	// Name
	OutputText ("<p><table width=\"90%\"><tr><td>Name:</td><td><input type=\"text\" size=\"");
	m_pHttpResponse->WriteText (MAX_GAME_CLASS_NAME_LENGTH);
	OutputText ("\" maxlength=\"");
	m_pHttpResponse->WriteText (MAX_GAME_CLASS_NAME_LENGTH);
	OutputText ("\" value=\"");
	m_pHttpResponse->WriteText (strName.GetCharPtr(), strName.GetLength());
	OutputText ("\" name=\"GameClassName\"></td></tr>");
	
	// Description
	OutputText (
		"<tr><td>Description:</td><td>"\
		"<textarea rows=\"3\" cols=\"50\" wrap=\"physical\" name=\"GameClassDescription\">"
		);

	m_pHttpResponse->WriteText (strDesc.GetCharPtr(), strDesc.GetLength());
		
	OutputText ("</textarea></td></tr>");
	
	int iMinNumSecsPerUpdate, iMaxNumSecsPerUpdate, iMaxNumEmpires, iMaxNumPlanets, iMaxResourcesPerPlanet;
	float fMaxInitialTechLevel, fMaxTechDev;
	
	if (iEmpireKey == SYSTEM) {
		
		iErrCode = g_pGameEngine->GetSuperClassKeys (&piSuperClassKey, &pvSuperClassName, &iNumSuperClasses);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		if (iNumSuperClasses > 0) {
			
			OutputText ("<tr><td>Super Class:</td><td><select name=\"SuperClassKey\">");
			
			for (i = 0; i < iNumSuperClasses; i ++) {		
				OutputText ("<option");

				if (iSelSuperClass == piSuperClassKey[i]) {
					OutputText (" selected");
				}

				OutputText (" value=\"");
				m_pHttpResponse->WriteText (piSuperClassKey[i]);
				OutputText ("\">");
				m_pHttpResponse->WriteText (pvSuperClassName[i].GetCharPtr());
				OutputText ("</option>");
			}
			
			g_pGameEngine->FreeKeys ((unsigned int*) piSuperClassKey);
			g_pGameEngine->FreeData (pvSuperClassName);
		}
		
		OutputText ("</select></td></tr>");
		
		iErrCode = g_pGameEngine->GetMinNumSecsPerUpdateForSystemGameClass (&iMinNumSecsPerUpdate);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumSecsPerUpdateForSystemGameClass (&iMaxNumSecsPerUpdate);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumEmpiresForSystemGameClass (&iMaxNumEmpires);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumPlanetsForSystemGameClass (&iMaxNumPlanets);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxResourcesPerPlanet (&iMaxResourcesPerPlanet);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxInitialTechLevel (&fMaxInitialTechLevel);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxTechDev (&fMaxTechDev);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
	} else {
		
		iErrCode = g_pGameEngine->GetMinNumSecsPerUpdateForPersonalGameClass (&iMinNumSecsPerUpdate);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumSecsPerUpdateForPersonalGameClass (&iMaxNumSecsPerUpdate);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumEmpiresForPersonalGameClass (&iMaxNumEmpires);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumPlanetsForPersonalGameClass (&iMaxNumPlanets);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxResourcesPerPlanetPersonal (&iMaxResourcesPerPlanet);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxInitialTechLevelPersonal (&fMaxInitialTechLevel);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxTechDevPersonal (&fMaxTechDev);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}

	// Conserve settings
	if ((pHttpForm = m_pHttpRequest->GetForm ("ActiveGames")) != NULL) {
		bSelActiveGames = pHttpForm->GetIntValue() != 0;
	} else {
		bSelActiveGames = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("NumActiveGames")) != NULL) {
		iSelNumActiveGames = pHttpForm->GetIntValue();
	} else {
		iSelNumActiveGames = 5;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumEmpires")) != NULL) {
		iSelMinNumEmpires = pHttpForm->GetIntValue();
	} else {
		iSelMinNumEmpires = 2;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumEmpires")) != NULL) {
		iSelMaxNumEmpires = pHttpForm->GetIntValue();
	} else {
		iSelMaxNumEmpires = min (8, iMaxNumEmpires);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumPlanets")) != NULL) {
		iSelMinNumPlanets = pHttpForm->GetIntValue();
	} else {
		iSelMinNumPlanets = min (4, iMaxNumPlanets);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumPlanets")) != NULL) {
		iSelMaxNumPlanets = pHttpForm->GetIntValue();
	} else {
		iSelMaxNumPlanets = min (4, iMaxNumPlanets);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("InitTechLevel")) != NULL) {
		fSelInitTechLevel = pHttpForm->GetFloatValue();
	} else {
		fSelInitTechLevel = (float) 1.0;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("TechIncrease")) != NULL) {
		fSelTechIncrease = pHttpForm->GetFloatValue();
	} else {
		fSelTechIncrease = (float) 1.5;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("HoursPU")) != NULL) {
		iSelHoursPU = pHttpForm->GetIntValue();
	} else {
		iSelHoursPU = 0;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinsPU")) != NULL) {
		iSelMinsPU = pHttpForm->GetIntValue();
	} else {
		iSelMinsPU = 3;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("SecsPU")) != NULL) {
		iSelSecsPU = pHttpForm->GetIntValue();
	} else {
		iSelSecsPU = 0;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("Weekend")) != NULL) {
		bSelWeekend = pHttpForm->GetIntValue() != 0;
	} else {
		bSelWeekend = true;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinAg")) != NULL) {
		iSelMinAg = pHttpForm->GetIntValue();
	} else {
		iSelMinAg = min (33, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAg")) != NULL) {
		iSelMaxAg = pHttpForm->GetIntValue();
	} else {
		iSelMaxAg = min (33, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinMin")) != NULL) {
		iSelMinMin = pHttpForm->GetIntValue();
	} else {
		iSelMinMin = min (33, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxMin")) != NULL) {
		iSelMaxMin = pHttpForm->GetIntValue();
	} else {
		iSelMaxMin = min (33, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinFuel")) != NULL) {
		iSelMinFuel = pHttpForm->GetIntValue();
	} else {
		iSelMinFuel = min (33, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxFuel")) != NULL) {
		iSelMaxFuel = pHttpForm->GetIntValue();
	} else {
		iSelMaxFuel = min (33, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWAg")) != NULL) {
		iSelMinHWAg = pHttpForm->GetIntValue();
	} else {
		iSelMinHWAg = min (100, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWAg")) != NULL) {
		iSelMaxHWAg = pHttpForm->GetIntValue();
	} else {
		iSelMaxHWAg = min (100, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWMin")) != NULL) {
		iSelMinHWMin = pHttpForm->GetIntValue();
	} else {
		iSelMinHWMin = min (100, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWMin")) != NULL) {
		iSelMaxHWMin = pHttpForm->GetIntValue();
	} else {
		iSelMaxHWMin = min (100, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWFuel")) != NULL) {
		iSelMinHWFuel = pHttpForm->GetIntValue();
	} else {
		iSelMinHWFuel = min (100, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWFuel")) != NULL) {
		iSelMaxHWFuel = pHttpForm->GetIntValue();
	} else {
		iSelMaxHWFuel = min (100, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("PopLevel")) != NULL) {
		iSelPopLevel = pHttpForm->GetIntValue();
	} else {
		iSelPopLevel = min (50, iMaxResourcesPerPlanet);
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAgRatio")) != NULL) {
		fSelMaxAgRatio = pHttpForm->GetFloatValue();
	} else {
		fSelMaxAgRatio = MAX_RATIO;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("Dip")) != NULL) {
		iSelDip = pHttpForm->GetIntValue();
	} else {
		iSelDip = WAR | TRUCE | TRADE | ALLIANCE;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("Draws")) != NULL) {
		bSelDraws = pHttpForm->GetIntValue() != 0;
	} else {
		bSelDraws = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("Surrenders")) != NULL) {
		iSelSurrenders = pHttpForm->GetIntValue();
	} else {
		iSelSurrenders = 2;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTruces")) != NULL) {
		iSelMaxNumTruces = pHttpForm->GetIntValue();
	} else {
		iSelMaxNumTruces = UNRESTRICTED_DIPLOMACY;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTruces")) != NULL) {
		iSelStaticTruces = pHttpForm->GetIntValue();
	} else {
		iSelStaticTruces = 2;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTrades")) != NULL) {
		iSelMaxNumTrades = pHttpForm->GetIntValue();
	} else {
		iSelMaxNumTrades = UNRESTRICTED_DIPLOMACY;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTrades")) != NULL) {
		iSelStaticTrades = pHttpForm->GetIntValue();
	} else {
		iSelStaticTrades = 2;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumAllies")) != NULL) {
		iSelMaxNumAllies = pHttpForm->GetIntValue();
	} else {
		iSelMaxNumAllies = UNRESTRICTED_DIPLOMACY;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("StaticAllies")) != NULL) {
		iSelStaticAllies = pHttpForm->GetIntValue();
	} else {
		iSelStaticAllies = 2;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("PermanentAlliances")) != NULL) {
		bSelPermanentAlliances = pHttpForm->GetIntValue() != 0;
	} else {
		bSelPermanentAlliances = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("BreakAlliances")) != NULL) {
		bSelBreakAlliances = pHttpForm->GetIntValue() != 0;
	} else {
		bSelBreakAlliances = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesIdle")) != NULL) {
		iSelUpdatesIdle = pHttpForm->GetIntValue();
	} else {
		iSelUpdatesIdle = 2;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("Ruins")) != NULL) {
		iSelRuins = pHttpForm->GetIntValue();
	} else {
		iSelRuins = RUIN_ALMONASTER;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesRuin")) != NULL) {
		iSelUpdatesRuin = pHttpForm->GetIntValue();
	} else {
		iSelUpdatesRuin = 12;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("Subjective")) != NULL) {
		
		bSelSubjective = pHttpForm->GetIntValue() != 0;

		// Little bit of a hack here.  We assume the presence of the Subjective
		// form tells us if this is a submission re-render or a new form
		iSelInitShip = iSelDevShip = 0;

		ENUMERATE_TECHS(i) {

			char pszShip [64];

			sprintf (pszShip, "InitShip%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszShip)) != NULL) {
				iSelInitShip |= TECH_BITS[i];
			}

			sprintf (pszShip, "DevShip%i", i);
			if ((pHttpForm = m_pHttpRequest->GetForm (pszShip)) != NULL) {
				iSelDevShip |= TECH_BITS[i];
			}
		}

	} else {

		bSelSubjective = false;
		iSelInitShip = TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
		iSelDevShip = ALL_CLASSIC_TECHS;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("FriendlyGates")) != NULL) {
		bFriendlyGates = pHttpForm->GetIntValue() != 0;
	} else {
		bFriendlyGates = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("SuicidalDoomsdays")) != NULL) {
		bSuicidalDoomsdays = pHttpForm->GetIntValue() != 0;
	} else {
		bSuicidalDoomsdays = true;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("UnfriendlyDoomsdays")) != NULL) {
		bUnfriendlyDoomsdays = pHttpForm->GetIntValue() != 0;
	} else {
		bUnfriendlyDoomsdays = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("ClassicDoomsdays")) != NULL) {
		bClassicDoomsdays = pHttpForm->GetIntValue() != 0;
	} else {
		bClassicDoomsdays = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("Independence")) != NULL) {
		bSelIndependence = pHttpForm->GetIntValue() != 0;
	} else {
		bSelIndependence = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("Private")) != NULL) {
		bSelPrivate = pHttpForm->GetIntValue() != 0;
	} else {
		bSelPrivate = true;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleBuilds")) != NULL) {
		bSelVisibleBuilds = pHttpForm->GetIntValue() != 0;
	} else {
		bSelVisibleBuilds = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleDiplomacy")) != NULL) {
		bSelVisibleDiplomacy = pHttpForm->GetIntValue() != 0;
	} else {
		bSelVisibleDiplomacy = true;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("DipExposed")) != NULL) {
		bSelDipExposed = pHttpForm->GetIntValue() != 0;
	} else {
		bSelDipExposed = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("MapExposed")) != NULL) {
		iSelMapExposed = pHttpForm->GetIntValue() != 0;
	} else {
		iSelMapExposed = 0;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("DisconnectedMaps")) != NULL) {
		bSelDisconnectedMaps = pHttpForm->GetIntValue() != 0;
	} else {
		bSelDisconnectedMaps = false;
	}

	if ((pHttpForm = m_pHttpRequest->GetForm ("DipShareLevel")) != NULL) {
		iSelDipShareLevel = pHttpForm->GetIntValue() != 0;
	} else {
		iSelDipShareLevel = NO_DIPLOMACY;
	}

	// Continue rendering
	if (bPersonalGame) {
		
		OutputText ("<input type=\"hidden\" name=\"ActiveGames\" value=\"0\">");
		
	} else {
		
		OutputText (
			"<tr>"\
			"<td>Number of simultaneously active games:</td><td><select name=\"ActiveGames\">"\
			"<option"
			);

		if (!bSelActiveGames) {
			OutputText (" selected");
		}
			
		OutputText (" value=\"0\">Unlimited number of active games</option><option");

		if (bSelActiveGames) {
			OutputText (" selected");
		}

		OutputText (
			" value=\"1\">Limited number of active games</option></select>"\
			" Limit: <input type=\"text\" size=\"4\" maxlength=\"10\" name=\"NumActiveGames\" value=\""
			);

		m_pHttpResponse->WriteText (iSelNumActiveGames);
			
		OutputText ("\"></td></tr>");
	}
	
	OutputText (
		"<tr>"\
		"<td>Number of empires <em>(<strong>2</strong> to <strong>"
		);
	
	m_pHttpResponse->WriteText (iMaxNumEmpires);
	
	OutputText (
		"</strong>)</em>:</td>"\
		"<td>From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinNumEmpires\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMinNumEmpires);

	OutputText (
		"\"> "\
		"to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxNumEmpires\" value=\""
		);
	
	m_pHttpResponse->WriteText (iSelMaxNumEmpires);

	OutputText (
		"\">"\
		"</td></tr>"\
		
		"<tr>"\
		
		"<td>Number of planets per empire <em>(<strong>1</strong> to <strong>"
		);
	
	m_pHttpResponse->WriteText (iMaxNumPlanets);
	
	OutputText (
		"</strong>)</em>:</td>"\
		"<td>From "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinNumPlanets\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMinNumPlanets);

	OutputText (
		"\">"\
		" to "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxNumPlanets\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMaxNumPlanets);

	OutputText (
		"\">"\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Initial tech level <em>(<strong>1.0</strong> to <strong>")
		
		m_pHttpResponse->WriteText (fMaxInitialTechLevel);
	
	OutputText (
		"</strong>)</em>:</td>"\
		"<td><input type=\"text\" size=\"6\" maxlength=\"20\" name=\"InitTechLevel\" value=\""
		);
	
	m_pHttpResponse->WriteText (fSelInitTechLevel);

	OutputText (
		"\"></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Tech increase per update <em>(<strong>0.0</strong> to <strong>");
	
	m_pHttpResponse->WriteText (fMaxTechDev);
	
	OutputText (
		"</strong>)</em>:</td>"\
		"<td><input type=\"text\" size=\"6\" maxlength=\"20\" name=\"TechIncrease\" value=\""
		);
		
	m_pHttpResponse->WriteText (fSelTechIncrease);

	OutputText (
		"\"></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Update period <em>(");
	
	WriteTime (iMinNumSecsPerUpdate);
	OutputText (" to ");
	WriteTime (iMaxNumSecsPerUpdate);
	
	OutputText (
		")</em>:</td>"\
		"<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"HoursPU\" value=\""
		);

		m_pHttpResponse->WriteText (iSelHoursPU);
		
	OutputText (
		"\"> hours, "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinsPU\" value=\""
		);

		m_pHttpResponse->WriteText (iSelMinsPU);
		
	OutputText (
		"\"> minutes, "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SecsPU\" value=\""
		);

		m_pHttpResponse->WriteText (iSelSecsPU);

	OutputText (
		"\"> seconds</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Weekend updates:</td>"\
		"<td><select name=\"Weekend\">"\
		"<option"
		);

	if (bSelWeekend) {
		OutputText (" selected");
	}
		
	OutputText (
		" value=\"1\">Update on weekends</option>"\
		"<option"
		);

	if (!bSelWeekend) {
		OutputText (" selected");
	}
		
	OutputText (
		" value=\"0\">Don't update on weekends</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Average planet resources <em>(<strong>1</strong> to <strong>"
		);
	
	m_pHttpResponse->WriteText (iMaxResourcesPerPlanet);
	
	OutputText (
		"</strong> each)</em>:</td>"\
		"<td>"\
		"From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinAg\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMinAg);
	
	OutputText (
		"\">"\
		" to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxAg\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMaxAg);
	
	OutputText (
		"\"> agriculture"\
		"</td></tr><tr><td></td><td>"\
		"From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinMin\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMinMin);
	
	OutputText (
		"\">"\
		" to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxMin\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMaxMin);
	
	OutputText (
		"\"> minerals"\
		"</td></tr><tr><td></td><td>"\
		"From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinFuel\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMinFuel);
	
	OutputText (
		"\">"\
		" to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxFuel\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMaxFuel);
	
	OutputText (
		"\"> fuel"\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Average homeworld resources<br><em>(from the population needed to build to "\
		"<strong>");
	
	m_pHttpResponse->WriteText (iMaxResourcesPerPlanet);
	
	OutputText (
		"</strong>)</em>:</td>"\
		"<td>"\
		"From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinHWAg\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMinHWAg);
	
	OutputText (
		"\">"\
		" to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxHWAg\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMaxHWAg);
	
	OutputText (
		"\"> agriculture"\
		"</td></tr><tr><td></td><td>"\
		"From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinHWMin\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMinHWMin);
	
	OutputText (
		"\">"\
		" to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxHWMin\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMaxHWMin);
	
	OutputText (
		"\"> minerals"\
		"</td></tr><tr><td></td><td>"\
		"From <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinHWFuel\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMinHWFuel);
	
	OutputText (
		"\">"\
		" to <input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MaxHWFuel\" value=\""
		);

	m_pHttpResponse->WriteText (iSelMaxHWFuel);
	
	OutputText (
		"\"> fuel"\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Population needed to build<br><em>(<strong>1</strong> "\
		"to the minimum homeworld agriculture)</em>:</td>"\
		"<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"PopLevel\" value=\""
		);

	m_pHttpResponse->WriteText (iSelPopLevel);
	
	OutputText (
		"\"></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Maximum Agriculture Ratio <em>(<strong>");
	
	m_pHttpResponse->WriteText (MIN_MAX_AG_RATIO);
	
	OutputText ("</strong> to <strong>");
	
	m_pHttpResponse->WriteText (MAX_RATIO);
	
	OutputText (
		"</strong>)</em>:</td>"\
		"<td><input type=\"text\" size=\"8\" maxlength=\"20\" name=\"MaxAgRatio\" value=\"");
	
	m_pHttpResponse->WriteText (fSelMaxAgRatio);
	
	OutputText (
		"\"></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Diplomacy:</td>"\
		"<td><select name=\"Dip\">"\
		"<option"
		);

	if (iSelDip == WAR) {
		OutputText (" selected");
	}
	
	OutputText (
		" value=\""
		);
	
	m_pHttpResponse->WriteText (WAR);
	OutputText (
		
		"\">War</option>"\
		"<option"
		);
	
	if (iSelDip == (WAR | TRUCE)) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (WAR | TRUCE);
	OutputText (
		
		"\">War, Truce</option>"\
		"<option"
		);
	
	if (iSelDip == (WAR | TRADE)) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (WAR | TRADE);
	OutputText (
		
		"\">War, Trade</option>"\
		"<option"
		);
	
	if (iSelDip == (WAR | ALLIANCE)) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (WAR | ALLIANCE);
	OutputText (
		
		"\">War, Alliance</option>"\
		"<option"
		);
	
	if (iSelDip == (WAR | TRUCE | TRADE)) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (WAR | TRUCE | TRADE);
	OutputText (
		
		"\">War, Truce, Trade</option>"\
		"<option"
		);
	
	if (iSelDip == (WAR | TRUCE | ALLIANCE)) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (WAR | TRUCE | ALLIANCE);
	OutputText (
		
		"\">War, Truce, Alliance</option>"\
		"<option"
		);
	
	if (iSelDip == (WAR | TRADE | ALLIANCE)) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (WAR | TRADE | ALLIANCE);
	OutputText (
		
		"\">War, Trade, Alliance</option>"\
		"<option"
		);
	
	if (iSelDip == (WAR | TRUCE | TRADE | ALLIANCE)) {
		OutputText (" selected");
	}

	OutputText (
		" value=\""
		);
	
	m_pHttpResponse->WriteText (WAR | TRUCE | TRADE | ALLIANCE);
	
	OutputText (
		"\">War, Truce, Trade, Alliance</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Draw settings:</td>"\
		"<td><select name=\"Draws\">"\
		"<option"
		);
	
	if (bSelDraws) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Allow draws</option>"\
		"<option"
		);
	
	if (!bSelDraws) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">No draws</option>"\
		"</select> "\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Surrender settings:</td>"\
		"<td><select name=\"Surrenders\">"\
		"<option"
		);
	
	if (iSelSurrenders == 1) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Allow surrenders always</option>"\
		"<option"
		);
	
	if (iSelSurrenders == 2) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"2\">Allow surrenders when two empires remain</option>"\
		"<option"
		);
	
	if (iSelSurrenders == 3) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"3\">Allow classic SC-style surrenders</option>"\
		"<option"
		);
	
	if (iSelSurrenders == 0) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Never allow surrenders</option>"\
		"</select>"\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Maximum number of truces allowed:</td>"\
		"<td><select name=\"MaxNumTruces\">"\
		"<option"
		);
	
	if (iSelMaxNumTruces == UNRESTRICTED_DIPLOMACY) {
		OutputText (" selected");
	}

	OutputText (
		" value=\""
		);
	
	m_pHttpResponse->WriteText (UNRESTRICTED_DIPLOMACY);
	
	OutputText (
		"\">No restrictions</option>"\
		"<option"
		);
	
	if (iSelMaxNumTruces == FAIR_DIPLOMACY) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (FAIR_DIPLOMACY);
	
	OutputText (
		"\">A fair number: (N-2)/2</option>"\
		"<option"
		);
	
	if (iSelMaxNumTruces == STATIC_RESTRICTION) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (STATIC_RESTRICTION);
	
	OutputText (
		"\">A static limit</option></select> "\
		"Static limit: "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"StaticTruces\" value=\""
		);

	m_pHttpResponse->WriteText (iSelStaticTruces);
	
	OutputText (
		"\"></td></tr>"\
		
		"<tr>"\
		"<td>Maximum number of trades allowed:</td>"\
		"<td><select name=\"MaxNumTrades\">"\
		"<option"
		);
	
	if (iSelMaxNumTrades == UNRESTRICTED_DIPLOMACY) {
		OutputText (" selected");
	}

	OutputText (
		" value=\""
		);
	
	m_pHttpResponse->WriteText (UNRESTRICTED_DIPLOMACY);
	
	OutputText (
		"\">No restrictions</option>"\
		"<option"
		);
	
	if (iSelMaxNumTrades == FAIR_DIPLOMACY) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (FAIR_DIPLOMACY);
	
	OutputText (
		"\">A fair number: (N-2)/2</option>"\
		"<option"
		);
	
	if (iSelMaxNumTrades == STATIC_RESTRICTION) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (STATIC_RESTRICTION);
	
	OutputText ("\">A static limit</option></select> "\
		"Static limit: "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"StaticTrades\" value=\""
		);

	m_pHttpResponse->WriteText (iSelStaticTrades);
	
	OutputText (
		"\"></td></tr>"\
		
		"<tr>"\
		"<td>Maximum number of alliances allowed:</td>"\
		"<td><select name=\"MaxNumAllies\">"\
		"<option"
		);
	
	if (iSelMaxNumAllies == UNRESTRICTED_DIPLOMACY) {
		OutputText (" selected");
	}

	OutputText (
		" value=\""
		);
	
	m_pHttpResponse->WriteText (UNRESTRICTED_DIPLOMACY);
	
	m_pHttpResponse->WriteText (
		"\">No restrictions</option>"\
		"<option"
		);
	
	if (iSelMaxNumAllies == FAIR_DIPLOMACY) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (FAIR_DIPLOMACY);
	
	m_pHttpResponse->WriteText (
		"\">A fair number: (N-2)/2</option>"\
		"<option"
		);
	
	if (iSelMaxNumAllies == STATIC_RESTRICTION) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	
	m_pHttpResponse->WriteText (STATIC_RESTRICTION);
	
	OutputText (
		
		"\">A static limit</option></select> "\
		"Static limit: "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"StaticAllies\" value=\""
		);
	
	m_pHttpResponse->WriteText (iSelStaticAllies);
	
	OutputText (
		"\"> "\
		
		"<select name=\"PermanentAlliances\">"\
		"<option"
		);
	
	if (!bSelPermanentAlliances) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Limit is for concurrent alliances</option>"\
		"<option"
		);
	
	if (bSelPermanentAlliances) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Limit is for the entire game</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Breakable alliances:</td>"\
		"<td>"\
		"<select name=\"BreakAlliances\">"\
		"<option"
		);
	
	if (!bSelBreakAlliances) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Alliances are breakable</option>"\
		"<option"
		);
	
	if (bSelBreakAlliances) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Alliances are unbreakable</option>"\
		"</select>"\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Updates before empires become idle "\
		"<em>(<strong>1</strong> to 10</em>:</td>"\
		"<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"UpdatesIdle\" value=\""
		);

	m_pHttpResponse->WriteText (iSelUpdatesIdle);
	
	OutputText (
		"\"></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Ruin settings:</td><td>"\
		"<select name=\"Ruins\">"\
		"<option"
		);
	
	if (iSelRuins == 0) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Empires never fall into ruin, except when they're all idle</option>"\
		"<option"
		);
	
	if (iSelRuins == RUIN_CLASSIC_SC) {
		OutputText (" selected");
	}

	OutputText (
		" value=\""
		);

	m_pHttpResponse->WriteText (RUIN_CLASSIC_SC);
	
	OutputText ("\">Simple ruins: ruin after a number of idle updates</option>"\
		"<option"
		);
	
	if (iSelRuins == RUIN_ALMONASTER) {
		OutputText (" selected");
	}

	OutputText (
		" value=\""
		);

	m_pHttpResponse->WriteText (RUIN_ALMONASTER);
	
	OutputText (
		"\">Complex ruins: ruin if only one empire is awake and more than two played the game</option>"\
		"</select>"\

		"<br>Number of idle updates before empires ruin: "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"UpdatesRuin\" value=\""
		);

	m_pHttpResponse->WriteText (iSelUpdatesRuin);
	
	OutputText (
		"\"></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Econ and mil views of other empires:</td>"\
		"<td><select name=\"Subjective\">"\
		"<option"
		);
	
	if (bSelSubjective) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Subjective views: econ and mil represent only what you can see</option>"\
		"<option"
		);
	
	if (!bSelSubjective) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Objective views: econ and mil represent an empire's real totals</option>"\
		"</select>"\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Ship availability:</td>"\
		"<td><table><tr><td>"\
		"Initially available:</td><td>"
		);
	
	ENUMERATE_TECHS(i) {
		
		if (i != FIRST_SHIP) {
			OutputText ("<br>");
		}
		
		OutputText ("<input name=\"InitShip");
		m_pHttpResponse->WriteText (i);
		OutputText ("\" type=\"checkbox\"");

		if (iSelInitShip & TECH_BITS[i]) {
			OutputText (" checked");
		}
		OutputText ("> ");
		m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
	}
	
	OutputText ("</td><td>Developable:</td><td>");
	
	ENUMERATE_TECHS(i) {
		
		if (i != FIRST_SHIP) {
			OutputText ("<br>");
		}
		
		OutputText ("<input name=\"DevShip");
		m_pHttpResponse->WriteText (i);
		OutputText ("\" type=\"checkbox\"");
		if (iSelDevShip & TECH_BITS[i]) {
			OutputText (" checked");
		}
		OutputText ("> ");
		m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
	}

	OutputText ("</td></tr></table></td>"\
		"</tr>"\

		"<tr>"\
		"<td>Gate behavior:</td>"\
		"<td><select name=\"FriendlyGates\">"\
		"<option"
		);

	if (!bFriendlyGates) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Jumpgates and stargates cannot gate allied ships</option>"\
		"<option"
		);

	if (bFriendlyGates) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Jumpgates and stargates can gate allied ships</option>"\
		"</select></td>"\
		"</tr>"\

		"<tr>"\
		"<td>Doomsday behavior:</td>"\
		"<td><select name=\"SuicidalDoomsdays\">"\
		"<option"
		);
	
	if (bSuicidalDoomsdays) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Doomsdays can annihilate their owner's own planets</option>"\
		"<option"
		);
	
	if (!bSuicidalDoomsdays) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Doomsdays cannot annihilate their owner's own planets</option>"\
		"</select><br>"\

		"<select name=\"UnfriendlyDoomsdays\">"\
		"<option"
		);
	
	if (bUnfriendlyDoomsdays) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Doomsdays can annihilate any planet belonging to other empires (except non-warring homeworlds)</option>"\
		"<option"
		);
	
	if (!bUnfriendlyDoomsdays) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Doomsdays can only annihilate planets belonging to warring empires</option>"\
		"</select><br>"\

		"<select name=\"ClassicDoomsdays\">"\
		"<option"
		);
	
	if (!bClassicDoomsdays) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Doomsdays annihilate planets for a limited quarantine time</option>"\
		"<option"
		);

	if (bClassicDoomsdays) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Doomsdays annihilate planets permanently, as in classic SC</option>"\
		"</select></td>"\
		"</tr>"\

		"<tr>"\
		"<td>Independence:</td>"\
		"<td><select name=\"Independence\">"\
		"<option"
		);
	
	if (bSelIndependence) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Independent planets and ships</option>"\
		"<option"
		);
	
	if (!bSelIndependence) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">No independent planets or ships</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Private messages:</td>"\
		"<td><select name=\"Private\">"\
		"<option"
		);
	
	if (bSelPrivate) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Allow private messages</option>"\
		"<option"
		);
	
	if (!bSelPrivate) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Don't allow private messages</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Build visibility:</td>"\
		"<td><select name=\"VisibleBuilds\">"\
		"<option"
		);
	
	if (bSelVisibleBuilds) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Visible builds</option>"\
		"<option"
		);
	
	if (!bSelVisibleBuilds) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Invisible builds</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Diplomacy visibility before updates:</td>"\
		"<td><select name=\"VisibleDiplomacy\">"\
		"<option"
		);
	
	if (bSelVisibleDiplomacy) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Visible diplomacy</option>"\
		"<option"
		);
	
	if (!bSelVisibleDiplomacy) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Invisible diplomacy</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Diplomacy initially exposed:</td>"\
		"<td><select name=\"DipExposed\">"\
		"<option"
		);
	
	if (bSelDipExposed) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Diplomacy exposed</option>"\
		"<option"
		);
	
	if (!bSelDipExposed) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Diplomacy not exposed</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Map initial state:</td>"\
		"<td><select name=\"MapExposed\">"\
		"<option"
		);
	
	if (iSelMapExposed == EXPOSED_MAP) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	m_pHttpResponse->WriteText (EXPOSED_MAP);
	OutputText (
		"\">Map exposed</option>"\
		"<option"
		);
	
	if (iSelMapExposed == FULLY_COLONIZED_MAP) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	m_pHttpResponse->WriteText (FULLY_COLONIZED_MAP);
	OutputText (
		"\">Map fully colonized</option>"\
		"<option"
		);
	
	if (iSelMapExposed == (EXPOSED_MAP | FULLY_COLONIZED_MAP)) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	m_pHttpResponse->WriteText (EXPOSED_MAP | FULLY_COLONIZED_MAP);
	OutputText ("\">Map exposed and fully colonized</option>"\
		"<option"
		);
	
	if (iSelMapExposed == 0) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Map not exposed or fully colonized</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Disconnected maps (requires engineers):</td>"\
		"<td><select name=\"DisconnectedMaps\">"\
		"<option"
		);
	
	if (!bSelDisconnectedMaps) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"0\">Connected maps</option>"\
		"<option"
		);
	
	if (bSelDisconnectedMaps) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Disconnected maps</option>"\
		"</select></td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Map sharing:</td>"\
		"<td><select name=\"DipShareLevel\"><option"
		);
	
	if (iSelDipShareLevel == TRUCE) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	m_pHttpResponse->WriteText (TRUCE);
	OutputText ("\">Maps shared at " TRUCE_STRING "</option><option"
		);
	
	if (iSelDipShareLevel == TRADE) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	m_pHttpResponse->WriteText (TRADE);
	OutputText ("\">Maps shared at " TRADE_STRING "</option><option"
		);
	
	if (iSelDipShareLevel == ALLIANCE) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	m_pHttpResponse->WriteText (ALLIANCE);
	OutputText ("\">Maps shared at " ALLIANCE_STRING "</option><option"
		);
	
	if (iSelDipShareLevel == NO_DIPLOMACY) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"");
	m_pHttpResponse->WriteText (NO_DIPLOMACY);
	OutputText ("\">Maps not shared</option></select></td>"\
		
		"</tr>"\
		
		"</table>");
	
	return;
	
Cleanup:
	
	OutputText ("<p>An error occurred reading data from the system database. Please contact the administrator<p>");
}


int HtmlRenderer::ProcessCreateGameClassForms (int iOwnerKey) {
	
	int iErrCode, iGameClass;

	Variant pvSubmitArray [SystemGameClassData::NumColumns];
	
	// Parse the forms
	iErrCode = ParseCreateGameClassForms (pvSubmitArray, iOwnerKey);
	if (iErrCode != OK) {
		return iErrCode;
	}
	
	// Create the gameclass, finally
	iErrCode = g_pGameEngine->CreateGameClass (pvSubmitArray, &iGameClass);
	switch (iErrCode) {

	case OK:
		AddMessage ("The GameClass was created");
		break;
	case ERROR_GAMECLASS_ALREADY_EXISTS:
		AddMessage ("The new GameClass name already exists");
		break;
	case ERROR_NAME_IS_TOO_LONG:
		AddMessage ("The new GameClass name is too long");
		break;
	case ERROR_DESCRIPTION_IS_TOO_LONG:
		AddMessage ("The new GameClass description is too long");
		break;
	default:
		AddMessage ("The GameClass could not be created; the error was ");
		AppendMessage (iErrCode);
		break;
	}
	
	return OK;
}


int HtmlRenderer::ProcessCreateDynamicGameClassForms (int iOwnerKey, int* piGameClass, int* piGameNumber, 
													  bool* pbGameCreated) {
	
	int iErrCode;
	
	Variant pvSubmitArray [SystemGameClassData::NumColumns];
	
	GameOptions goOptions;
	InitGameOptions (&goOptions);
	
	*pbGameCreated = false;
	
	// Parse the forms
	iErrCode = ParseCreateGameClassForms (pvSubmitArray, iOwnerKey);
	if (iErrCode != OK) {
		goto Cleanup;
	}
	
	iErrCode = ParseGameConfigurationForms (NO_KEY, pvSubmitArray, iOwnerKey, &goOptions);
	if (iErrCode != OK) {
		goto Cleanup;
	}
	
	if (goOptions.sFirstUpdateDelay > pvSubmitArray[SystemGameClassData::NumSecPerUpdate].GetInteger() * 10) {
		AddMessage ("The first update delay is too large");
		iErrCode = ERROR_FAILURE;
		goto Cleanup;
	}
	
	// Dynamic gameclass
	pvSubmitArray[SystemGameClassData::Options] = 
		pvSubmitArray[SystemGameClassData::Options].GetInteger() | DYNAMIC_GAMECLASS;
	
	// Create the gameclass
	iErrCode = g_pGameEngine->CreateGameClass (pvSubmitArray, piGameClass);
	if (iErrCode != OK) {
		
		switch (iErrCode) {
			
		case ERROR_GAMECLASS_ALREADY_EXISTS:
			AddMessage ("The new game's name already exists");
			break;
		case ERROR_NAME_IS_TOO_LONG:
			AddMessage ("The new game's name is too long");
			break;
		case ERROR_DESCRIPTION_IS_TOO_LONG:
			AddMessage ("The new GameClass description is too long");
			break;
		default:
			AddMessage ("The game could not be created; the error was ");
			AppendMessage (iErrCode);
			break;
		}
		
		goto Cleanup;
	}

	// Bridier sanity
	Assert (pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger() == 2 || 
		!(goOptions.iOptions & GAME_COUNT_FOR_BRIDIER));

	// Spectator sanity
	Assert (
		((pvSubmitArray[SystemGameClassData::Options].GetInteger() & EXPOSED_SPECTATORS) == 
		EXPOSED_SPECTATORS) || 
		!(goOptions.iOptions & GAME_ALLOW_SPECTATORS));

	// Try to create the game
	*pbGameCreated = true;
	
	iErrCode = g_pGameEngine->CreateGame (*piGameClass, iOwnerKey, goOptions, piGameNumber);
	if (iErrCode == OK) {
		
		// Halt the gameclass
		int iErrCode2 = g_pGameEngine->HaltGameClass (*piGameClass);
		Assert (iErrCode2 == OK);
		
	} else {
		
		// Delete the gameclass
		bool bDeleted;
		int iErrCode2 = g_pGameEngine->DeleteGameClass (*piGameClass, &bDeleted);
		Assert (iErrCode2 == OK);
	}

Cleanup:

	ClearGameOptions (&goOptions);

	return iErrCode;
}


int HtmlRenderer::ParseCreateGameClassForms (Variant* pvSubmitArray, int iOwnerKey) {
	
	IHttpForm* pHttpForm;
	
	int iOptions = 0, iTemp;
	
	int iMinNumSecsPerUpdate, iMaxNumSecsPerUpdate, iMaxNumEmpires, iMaxNumPlanets, iErrCode,
		iMaxResourcesPerPlanet;
	float fMaxInitialTechLevel, fMaxTechDev;
	
	int i, iInitTechDevs = 0, iDevTechDevs = 0, iDip;
	
	char pszShipString [MAX_SHIP_NAME_LENGTH + 64];
	
	const char* pszString;
	
	if (iOwnerKey == SYSTEM) {
		
		iErrCode = g_pGameEngine->GetMinNumSecsPerUpdateForSystemGameClass (&iMinNumSecsPerUpdate);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumSecsPerUpdateForSystemGameClass (&iMaxNumSecsPerUpdate);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumEmpiresForSystemGameClass (&iMaxNumEmpires);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumPlanetsForSystemGameClass (&iMaxNumPlanets);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxResourcesPerPlanet (&iMaxResourcesPerPlanet);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxInitialTechLevel (&fMaxInitialTechLevel);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxTechDev (&fMaxTechDev);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
	} else {
		
		iErrCode = g_pGameEngine->GetMinNumSecsPerUpdateForPersonalGameClass (&iMinNumSecsPerUpdate);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumSecsPerUpdateForPersonalGameClass (&iMaxNumSecsPerUpdate);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumEmpiresForPersonalGameClass (&iMaxNumEmpires);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxNumPlanetsForPersonalGameClass (&iMaxNumPlanets);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxResourcesPerPlanetPersonal (&iMaxResourcesPerPlanet);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxInitialTechLevelPersonal (&fMaxInitialTechLevel);
		if (iErrCode != OK) {
			goto Cleanup;
		}
		
		iErrCode = g_pGameEngine->GetMaxTechDevPersonal (&fMaxTechDev);
		if (iErrCode != OK) {
			goto Cleanup;
		}
	}
	
	// GameClassName
	if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassName")) == NULL) {
		AddMessage ("Missing GameClassName form");
		return ERROR_FAILURE;
	}
	
	if (pHttpForm->GetValue() == NULL) {
		AddMessage ("The name cannot be blank");
		return ERROR_FAILURE;
	}
	
	pszString = pHttpForm->GetValue();
	
	if (VerifyGameClassName (pszString) != OK) {
		return ERROR_FAILURE;
	}
	
	pvSubmitArray[SystemGameClassData::Name] = pszString;
	
	// Description
	if ((pHttpForm = m_pHttpRequest->GetForm ("GameClassDescription")) == NULL) {
		AddMessage ("Missing GameClassDescription form");
		return ERROR_FAILURE;
	}
	
	pszString = pHttpForm->GetValue();
	
	if (pszString == NULL) {
		pvSubmitArray[SystemGameClassData::Description] = "";
	} else {
		
		if (strlen (pszString) > MAX_GAMECLASS_DESCRIPTION_LENGTH) {
			AddMessage ("The description is too long");
			return ERROR_FAILURE;
		}
		
		pvSubmitArray[SystemGameClassData::Description] = pszString;
	}
	
	// MaxNumEmpires
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumEmpires")) == NULL) {
		AddMessage ("Missing MaxNumEmpires form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxNumEmpires] = pHttpForm->GetIntValue();
	
	// NumPlanetsPerEmpire
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumPlanets")) == NULL) {
		AddMessage ("Missing NumPlanetsPerEmpire form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxNumPlanets] = pHttpForm->GetIntValue();
	
	// TechIncrease
	if ((pHttpForm = m_pHttpRequest->GetForm ("TechIncrease")) == NULL) {
		AddMessage ("Missing TechIncrease form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxTechDev] = pHttpForm->GetFloatValue();
	
	// OpenGameNum
	pvSubmitArray[SystemGameClassData::OpenGameNum] = 1;
	
	// SecsPerUpdate //
	
	// Hours per update
	if ((pHttpForm = m_pHttpRequest->GetForm ("HoursPU")) == NULL) {
		AddMessage ("Missing HoursPU form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * pHttpForm->GetIntValue();
	
	// Minutes per update
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinsPU")) == NULL) {
		AddMessage ("Missing MinsPU form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::NumSecPerUpdate] += 60 * pHttpForm->GetIntValue();
	
	// Seconds per update
	if ((pHttpForm = m_pHttpRequest->GetForm ("SecsPU")) == NULL) {
		AddMessage ("Missing SecsPU form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::NumSecPerUpdate] += pHttpForm->GetIntValue();
	
	// Weekend updates
	if ((pHttpForm = m_pHttpRequest->GetForm ("Weekend")) == NULL) {
		AddMessage ("Missing Weekend form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= WEEKEND_UPDATES;
	}
	
	// InitTechLevel
	if ((pHttpForm = m_pHttpRequest->GetForm ("InitTechLevel")) == NULL) {
		AddMessage ("Missing InitTechLevel form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::InitialTechLevel] = pHttpForm->GetFloatValue();
	
	// MinNumEmpires
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumEmpires")) == NULL) {
		AddMessage ("Missing MinNumEmpires form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MinNumEmpires] = pHttpForm->GetIntValue();
	
	// BuilderPopLevel
	if ((pHttpForm = m_pHttpRequest->GetForm ("PopLevel")) == NULL) {
		AddMessage ("Missing PopLevel form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::BuilderPopLevel] = pHttpForm->GetIntValue();
	
	// MaxAgRatio
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAgRatio")) == NULL) {
		AddMessage ("Missing MaxAgRatio form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxAgRatio] = pHttpForm->GetFloatValue();
	
	// Draws
	if ((pHttpForm = m_pHttpRequest->GetForm ("Draws")) == NULL) {
		AddMessage ("Missing Draws form");
		return ERROR_FAILURE;
	}
	iTemp = (pHttpForm->GetIntValue() != 0) ? DRAW : 0;
	
	// Surrenders
	if ((pHttpForm = m_pHttpRequest->GetForm ("Surrenders")) == NULL) {
		AddMessage ("Missing Surrenders form");
		return ERROR_FAILURE;
	}
	
	iDip = pHttpForm->GetIntValue();
	
	switch (iDip) {
	case 3:
		iOptions |= USE_SC30_SURRENDERS;
		break;
		
	case 2:
		iOptions |= ONLY_SURRENDER_WITH_TWO_EMPIRES;
		
	case 1:
		iTemp |= SURRENDER;
		break;
	}
	
	// Dip
	if ((pHttpForm = m_pHttpRequest->GetForm ("Dip")) == NULL) {
		AddMessage ("Missing Dip form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::DiplomacyLevel] = pHttpForm->GetIntValue() | iTemp;
	
	iDip = pvSubmitArray[SystemGameClassData::DiplomacyLevel].GetInteger();
	
	// BreakAlliances
	if ((pHttpForm = m_pHttpRequest->GetForm ("BreakAlliances")) == NULL) {
		AddMessage ("Missing BreakAlliances form");
		return ERROR_FAILURE;
	}
	
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= UNBREAKABLE_ALLIANCES;
	}
	
	// PermanentAlliances
	if ((pHttpForm = m_pHttpRequest->GetForm ("PermanentAlliances")) == NULL) {
		AddMessage ("Missing PermanentAlliances form");
		return ERROR_FAILURE;
	}
	
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= PERMANENT_ALLIANCES;
	}
	
	// VisibleDiplomacy
	if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleDiplomacy")) == NULL) {
		AddMessage ("Missing VisibleDiplomacy form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= VISIBLE_DIPLOMACY;
	}

	// FriendlyGates
	if ((pHttpForm = m_pHttpRequest->GetForm ("FriendlyGates")) == NULL) {
		AddMessage ("Missing FriendlyGates form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= USE_FRIENDLY_GATES;
	}
	
	// SuicidalDoomsdays
	if ((pHttpForm = m_pHttpRequest->GetForm ("SuicidalDoomsdays")) == NULL) {
		AddMessage ("Missing SuicidalDoomsdays form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() == 0) {
		iOptions |= DISABLE_SUICIDAL_DOOMSDAYS;
	}

	// UnfriendlyDoomsdays
	if ((pHttpForm = m_pHttpRequest->GetForm ("UnfriendlyDoomsdays")) == NULL) {
		AddMessage ("Missing UnfriendlyDoomsdays form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= USE_UNFRIENDLY_DOOMSDAYS;
	}

	// ClassicDoomsdays
	if ((pHttpForm = m_pHttpRequest->GetForm ("ClassicDoomsdays")) == NULL) {
		AddMessage ("Missing ClassicDoomsdays form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= USE_CLASSIC_DOOMSDAYS;
	}
	
	// Independence
	if ((pHttpForm = m_pHttpRequest->GetForm ("Independence")) == NULL) {
		AddMessage ("Missing Independence form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= INDEPENDENCE;
	}
	
	// MapExposed and FullyCol
	if ((pHttpForm = m_pHttpRequest->GetForm ("MapExposed")) == NULL) {
		AddMessage ("Missing MapExposed form");
		return ERROR_FAILURE;
	}
	
	switch (pHttpForm->GetIntValue()) {
	case 0:
		break;
		
	case EXPOSED_MAP:
		iOptions |= EXPOSED_MAP;
		break;
		
	case FULLY_COLONIZED_MAP:
		iOptions |= FULLY_COLONIZED_MAP;
		break;
		
	case EXPOSED_MAP | FULLY_COLONIZED_MAP:
		iOptions |= EXPOSED_MAP | FULLY_COLONIZED_MAP;
		break;
		
	default:
		AddMessage ("Illegal MapExposed value");
		return ERROR_FAILURE;
	}
	
	// DisconnectedMaps
	if ((pHttpForm = m_pHttpRequest->GetForm ("DisconnectedMaps")) == NULL) {
		AddMessage ("Missing DisconnectedMaps form");
		return ERROR_FAILURE;
	}
	
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= DISCONNECTED_MAP;
	}
	
	// DipShareLevel
	if ((pHttpForm = m_pHttpRequest->GetForm ("DipShareLevel")) == NULL) {
		AddMessage ("Missing DipShareLevel form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MapsShared] = pHttpForm->GetIntValue();
	if (pvSubmitArray[SystemGameClassData::MapsShared] != NO_DIPLOMACY &&
		(pvSubmitArray[SystemGameClassData::MapsShared] < TRUCE ||
		pvSubmitArray[SystemGameClassData::MapsShared] > ALLIANCE)) {
		
		AddMessage ("Illegal value for DipShareLevel form");
		return ERROR_FAILURE;
	}
	
	// Owner
	pvSubmitArray[SystemGameClassData::Owner] = iOwnerKey;
	
	if (iOwnerKey == SYSTEM) {
		pvSubmitArray[SystemGameClassData::OwnerName] = "";
	} else {
		
		iErrCode = g_pGameEngine->GetEmpireName (iOwnerKey, pvSubmitArray + SystemGameClassData::OwnerName);
		if (iErrCode != OK) {
			AddMessage ("The creator's name could not be read");
			return iErrCode;
		}
	}
	
	// Private messages
	if ((pHttpForm = m_pHttpRequest->GetForm ("Private")) == NULL) {
		AddMessage ("Missing Private form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= PRIVATE_MESSAGES;
	}
	
	// DipExposed
	if ((pHttpForm = m_pHttpRequest->GetForm ("DipExposed")) == NULL) {
		AddMessage ("Missing DipExposed form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= EXPOSED_DIPLOMACY;
	}
	
	// Visible builds
	if ((pHttpForm = m_pHttpRequest->GetForm ("VisibleBuilds")) == NULL) {
		AddMessage ("Missing VisibleBuilds form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= VISIBLE_BUILDS;
	}
	
	// Subjective views
	if ((pHttpForm = m_pHttpRequest->GetForm ("Subjective")) == NULL) {
		AddMessage ("Missing Subjective form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		iOptions |= SUBJECTIVE_VIEWS;
	}
	
	// SuperClassKey
	if (iOwnerKey == SYSTEM) {
		if ((pHttpForm = m_pHttpRequest->GetForm ("SuperClassKey")) == NULL) {
			AddMessage ("Missing SuperClassKey form");
			return ERROR_FAILURE;
		}
		pvSubmitArray[SystemGameClassData::SuperClassKey] = pHttpForm->GetIntValue();
	} else {
		pvSubmitArray[SystemGameClassData::SuperClassKey] = NO_KEY;
	}
	
	// MaxNumTruces
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTruces")) == NULL) {
		AddMessage ("Missing MaxNumTruces form");
		return ERROR_FAILURE;
	}
	iMaxNumEmpires = pHttpForm->GetIntValue();
	
	switch (iMaxNumEmpires) {
		
	case UNRESTRICTED_DIPLOMACY:
		
		pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
		break;
		
	case FAIR_DIPLOMACY:
		
		pvSubmitArray[SystemGameClassData::MaxNumTruces] = FAIR_DIPLOMACY;
		break;
		
	case STATIC_RESTRICTION:
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTruces")) == NULL) {
			AddMessage ("Missing StaticTruces form");
			return ERROR_FAILURE;
		}
		pvSubmitArray[SystemGameClassData::MaxNumTruces] = pHttpForm->GetIntValue();
		break;
		
	default:
		
		AddMessage ("Illegal MaxNumTruces value");
		return ERROR_FAILURE;
	}
	
	// MaxNumTrades
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumTrades")) == NULL) {
		AddMessage ("Missing MaxNumTrades form");
		return ERROR_FAILURE;
	}
	iMaxNumEmpires = pHttpForm->GetIntValue();
	
	switch (iMaxNumEmpires) {
		
	case UNRESTRICTED_DIPLOMACY:
		
		pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
		break;
		
	case FAIR_DIPLOMACY:
		
		pvSubmitArray[SystemGameClassData::MaxNumTrades] = FAIR_DIPLOMACY;
		break;
		
	case STATIC_RESTRICTION:
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("StaticTrades")) == NULL) {
			AddMessage ("Missing StaticTrades form");
			return ERROR_FAILURE;
		}
		pvSubmitArray[SystemGameClassData::MaxNumTrades] = pHttpForm->GetIntValue();
		break;
		
	default:
		
		AddMessage ("Illegal MaxNumTrades value");
		return ERROR_FAILURE;
	}
	
	// MaxNumAllies
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxNumAllies")) == NULL) {
		AddMessage ("Missing MaxNumAllies form");
		return ERROR_FAILURE;
	}
	iMaxNumEmpires = pHttpForm->GetIntValue();
	
	switch (iMaxNumEmpires) {
		
	case UNRESTRICTED_DIPLOMACY:
		
		pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
		break;
		
	case FAIR_DIPLOMACY:
		
		pvSubmitArray[SystemGameClassData::MaxNumAlliances] = FAIR_DIPLOMACY;
		break;
		
	case STATIC_RESTRICTION:
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("StaticAllies")) == NULL) {
			AddMessage ("Missing StaticAllies form");
			return ERROR_FAILURE;
		}
		pvSubmitArray[SystemGameClassData::MaxNumAlliances] = pHttpForm->GetIntValue();
		break;
		
	default:
		
		AddMessage ("Illegal MaxNumAllies value");
		return ERROR_FAILURE;
	}
	
	// MinNumPlanets
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinNumPlanets")) == NULL) {
		AddMessage ("Missing MinNumPlanets form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MinNumPlanets] = pHttpForm->GetIntValue();

	//
	// MaxResources
	//

	// MaxAg
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxAg")) == NULL) {
		AddMessage ("Missing MaxAg form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxAvgAg] = pHttpForm->GetIntValue();
	
	// MaxMin
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxMin")) == NULL) {
		AddMessage ("Missing MaxMin form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxAvgMin] = pHttpForm->GetIntValue();
	
	// MaxFuel
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxFuel")) == NULL) {
		AddMessage ("Missing MaxFuel form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxAvgFuel] = pHttpForm->GetIntValue();
	
	//
	// MaxResourcesHW
	//
	
	// MaxAgHW
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWAg")) == NULL) {
		AddMessage ("Missing MaxHWAg form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxAgHW] = pHttpForm->GetIntValue();
	
	// MaxMinHW
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWMin")) == NULL) {
		AddMessage ("Missing MaxHWMin form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxMinHW] = pHttpForm->GetIntValue();
	
	// MaxFuelHW
	if ((pHttpForm = m_pHttpRequest->GetForm ("MaxHWFuel")) == NULL) {
		AddMessage ("Missing MaxHWFuel form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MaxFuelHW] = pHttpForm->GetIntValue();
	
	//
	// MinResources
	//
	
	// MinAg
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinAg")) == NULL) {
		AddMessage ("Missing MinAg form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MinAvgAg] = pHttpForm->GetIntValue();
	
	// MinMin
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinMin")) == NULL) {
		AddMessage ("Missing MinMin form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MinAvgMin] = pHttpForm->GetIntValue();
	
	// MinFuel
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinFuel")) == NULL) {
		AddMessage ("Missing MinFuel form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MinAvgFuel] = pHttpForm->GetIntValue();
	
	//
	// MinResourcesHW
	//
	
	// MinAgHW
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWAg")) == NULL) {
		AddMessage ("Missing MinHWAg form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MinAgHW] = pHttpForm->GetIntValue();
	
	// MinMinHW
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWMin")) == NULL) {
		AddMessage ("Missing MinHWMin form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MinMinHW] = pHttpForm->GetIntValue();
	
	// MinFuelHW
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinHWFuel")) == NULL) {
		AddMessage ("Missing MinHWFuel form");
		return ERROR_FAILURE;
	}
	pvSubmitArray[SystemGameClassData::MinFuelHW] = pHttpForm->GetIntValue();
	
	// NumUpdatesForIdle
	if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesIdle")) == NULL) {
		AddMessage ("Missing UpdatesIdle form");
		return ERROR_FAILURE;
	}
	
	iTemp = pHttpForm->GetIntValue();
	
	if (iTemp < 1 || iTemp > 10) {
		AddMessage ("Incorrect value for number of idle updates");
		return ERROR_FAILURE;
	}
	
	pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = iTemp;
	
	// RuinFlags, NumUpdatesForRuin
	if ((pHttpForm = m_pHttpRequest->GetForm ("Ruins")) == NULL) {
		AddMessage ("Missing Ruins form");
		return ERROR_FAILURE;
	}
	
	iTemp = pHttpForm->GetIntValue();
	
	switch (iTemp) {
		
	case 0:
	case RUIN_CLASSIC_SC:
	case RUIN_ALMONASTER:
		
		pvSubmitArray[SystemGameClassData::RuinFlags] = iTemp;
		break;
		
	default:
		
		AddMessage ("Incorrect Ruins value");
		return ERROR_FAILURE;
	}
	
	if (iTemp != 0) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("UpdatesRuin")) == NULL) {
			AddMessage ("Missing UpdatesRuin form");
			return ERROR_FAILURE;
		}
		
		iTemp = pHttpForm->GetIntValue();
		
		if (iTemp < 1 || iTemp < pvSubmitArray[SystemGameClassData::NumUpdatesForIdle].GetInteger()) {
			AddMessage ("Incorrect value for number of idle updates before ruin");
			return ERROR_FAILURE;
		}
		
		pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = iTemp;
	}
	
	// MaxNumActiveGames
	if ((pHttpForm = m_pHttpRequest->GetForm ("ActiveGames")) == NULL) {
		AddMessage ("Missing ActiveGames form");
		return ERROR_FAILURE;
	}
	
	if (pHttpForm->GetIntValue() == 0) {
		pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
	} else {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("NumActiveGames")) == NULL) {
			AddMessage ("Missing NumActiveGames form");
			return ERROR_FAILURE;
		}
		
		iTemp = pHttpForm->GetIntValue();
		
		if (iTemp < 1) {
			AddMessage ("Incorrect value for number of simultaneous active games");
			return ERROR_FAILURE;
		}
		
		pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = iTemp;
	}
	
	// Options
	pvSubmitArray[SystemGameClassData::Options] = iOptions;
	
	///////////////////
	// Sanity checks //
	///////////////////
	
	// Truce - trade - alliance limits
	if (pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger() != UNRESTRICTED_DIPLOMACY &&
		pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger() != FAIR_DIPLOMACY &&
		
		(pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger() < 1 || 
		
		pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger() >= 
		pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger()
		)
		) {
		
		AddMessage ("Incorrect maximum number of truces");
		return ERROR_FAILURE;
	}
	
	if (pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger() != UNRESTRICTED_DIPLOMACY &&
		pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger() != FAIR_DIPLOMACY &&
		
		(pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger() < 1 || 
		
		pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger() >= 
		pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger()
		)
		) {
		
		AddMessage ("Incorrect maximum number of trades");
		return ERROR_FAILURE;
	}
	
	if (pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() != UNRESTRICTED_DIPLOMACY &&
		pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() != FAIR_DIPLOMACY &&
		
		(pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() < 1 || 
		
		pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() >= 
		pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger()
		)
		) {
		
		AddMessage ("Incorrect maximum number of alliances");
		return ERROR_FAILURE;
	}
	
	// Compare limits
	iMaxNumEmpires = pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger();
	
	unsigned int iNumTruces, iNumTrades, iNumAlliances;
	
	switch (pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger()) {
		
	case UNRESTRICTED_DIPLOMACY:
		
		iNumTruces = iMaxNumEmpires;
		break;
		
	case FAIR_DIPLOMACY:
		
		iNumTruces = g_pGameEngine->GetNumFairDiplomaticPartners (iMaxNumEmpires);
		break;
		
	default:
		
		iNumTruces = pvSubmitArray[SystemGameClassData::MaxNumTruces].GetInteger();
		break;
	}
	
	switch (pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger()) {
		
	case UNRESTRICTED_DIPLOMACY:
		
		iNumTrades = iMaxNumEmpires;
		break;
		
	case FAIR_DIPLOMACY:
		
		iNumTrades = g_pGameEngine->GetNumFairDiplomaticPartners (iMaxNumEmpires);
		break;
		
	default:
		
		iNumTrades = pvSubmitArray[SystemGameClassData::MaxNumTrades].GetInteger();
		break;
	}
	
	switch (pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger()) {
		
	case UNRESTRICTED_DIPLOMACY:
		
		iNumAlliances = iMaxNumEmpires;
		break;
		
	case FAIR_DIPLOMACY:
		
		iNumAlliances = g_pGameEngine->GetNumFairDiplomaticPartners (iMaxNumEmpires);
		break;
		
	default:
		
		iNumAlliances = pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger();
		break;
	}
	
	if (g_pGameEngine->GameAllowsDiplomacy (iDip, TRUCE)) {
		
		if ((g_pGameEngine->GameAllowsDiplomacy (iDip, TRADE) && iNumTruces < iNumTrades) || 
			(g_pGameEngine->GameAllowsDiplomacy (iDip, ALLIANCE) && iNumTruces < iNumAlliances)) {
			AddMessage ("The max number of truces is too low");
			return ERROR_FAILURE;
		}
	}
	
	if (g_pGameEngine->GameAllowsDiplomacy (iDip, TRADE)) {
		
		if (g_pGameEngine->GameAllowsDiplomacy (iDip, ALLIANCE) && iNumTrades < iNumAlliances) {
			AddMessage ("The max number of trades is too low");
			return ERROR_FAILURE;
		}
	}
	
	// Number of empires
	if (pvSubmitArray[SystemGameClassData::MinNumEmpires].GetInteger() < 2 || 
		pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger() < pvSubmitArray[SystemGameClassData::MinNumEmpires].GetInteger() ||
		pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger() > iMaxNumEmpires) {
		
		AddMessage ("Incorrect number of empires");
		return ERROR_FAILURE;
	}
	
	// Planets per empire
	if (pvSubmitArray[SystemGameClassData::MaxNumPlanets].GetInteger() < 1 || 
		pvSubmitArray[SystemGameClassData::MaxNumPlanets].GetInteger() > iMaxNumPlanets ||
		
		pvSubmitArray[SystemGameClassData::MinNumPlanets].GetInteger() > 
		pvSubmitArray[SystemGameClassData::MaxNumPlanets].GetInteger()) {
		
		AddMessage ("Incorrect number of planets per empire");
		return ERROR_FAILURE;
	}
	
	// Name of gameclass
	if (pvSubmitArray[SystemGameClassData::Name].GetCharPtr() == NULL ||
		*pvSubmitArray[SystemGameClassData::Name].GetCharPtr() == '\0') {
		AddMessage ("GameClass names cannot be blank");
		return ERROR_FAILURE;
	}
	
	// Tech increase
	if (pvSubmitArray[SystemGameClassData::MaxTechDev].GetFloat() < (float) 0.0) {
		AddMessage ("Tech increase cannot be negative");
		return ERROR_FAILURE;
	}
	
	// Tech initial level
	if (pvSubmitArray[SystemGameClassData::InitialTechLevel].GetFloat() < (float) 1.0) {
		AddMessage ("Initial tech level cannot be less than 1.0");
		return ERROR_FAILURE;
	}
	
	// PopLevel
	if (pvSubmitArray[SystemGameClassData::BuilderPopLevel].GetInteger() < 1) {
		AddMessage ("Incorrect population needed to build");
		return ERROR_FAILURE;
	}
	
	// MaxAgRatio
	if (pvSubmitArray[SystemGameClassData::MaxAgRatio].GetFloat() < MIN_MAX_AG_RATIO ||
		pvSubmitArray[SystemGameClassData::MaxAgRatio].GetFloat() > MAX_RATIO) {
		AddMessage ("Incorrect Maximum Agriculture Ratio");
		return ERROR_FAILURE;
	}
	
	// Resources
	if (pvSubmitArray[SystemGameClassData::MinAvgAg].GetInteger() < 1 || 
		pvSubmitArray[SystemGameClassData::MaxAvgAg].GetInteger() > iMaxResourcesPerPlanet ||
		
		pvSubmitArray[SystemGameClassData::MinAvgAg].GetInteger() > 
		pvSubmitArray[SystemGameClassData::MaxAvgAg].GetInteger()) {
		
		AddMessage ("Incorrect average planet agriculture level");
		return ERROR_FAILURE;
	}
	
	if (pvSubmitArray[SystemGameClassData::MinAvgMin].GetInteger() < 1 ||
		pvSubmitArray[SystemGameClassData::MaxAvgMin].GetInteger() > iMaxResourcesPerPlanet ||
		
		pvSubmitArray[SystemGameClassData::MinAvgMin].GetInteger() > 
		pvSubmitArray[SystemGameClassData::MaxAvgMin].GetInteger()) {
		
		AddMessage ("Incorrect average planet mineral level");
		return ERROR_FAILURE;
	}
	
	if (pvSubmitArray[SystemGameClassData::MinAvgFuel].GetInteger() < 1 ||
		pvSubmitArray[SystemGameClassData::MaxAvgFuel].GetInteger() > iMaxResourcesPerPlanet ||
		
		pvSubmitArray[SystemGameClassData::MinAvgFuel].GetInteger() > 
		pvSubmitArray[SystemGameClassData::MaxAvgFuel].GetInteger()) {
		
		AddMessage ("Incorrect average planet fuel level");
		return ERROR_FAILURE;
	}
	
	
	if (pvSubmitArray[SystemGameClassData::MinAgHW].GetInteger() < 
		pvSubmitArray[SystemGameClassData::BuilderPopLevel].GetInteger() ||

		pvSubmitArray[SystemGameClassData::MaxAgHW].GetInteger() > iMaxResourcesPerPlanet ||
		
		pvSubmitArray[SystemGameClassData::MinAgHW].GetInteger() > 
		pvSubmitArray[SystemGameClassData::MaxAgHW].GetInteger()) {
		
		AddMessage ("Incorrect homeworld agriculture level");
		return ERROR_FAILURE;
	}
	
	if (pvSubmitArray[SystemGameClassData::MinMinHW].GetInteger() < 
		pvSubmitArray[SystemGameClassData::BuilderPopLevel].GetInteger() ||

		pvSubmitArray[SystemGameClassData::MaxMinHW].GetInteger() > iMaxResourcesPerPlanet ||
		
		pvSubmitArray[SystemGameClassData::MinMinHW].GetInteger() > 
		pvSubmitArray[SystemGameClassData::MaxMinHW].GetInteger()) {
		
		AddMessage ("Incorrect homeworld mineral level");
		return ERROR_FAILURE;
	}
	
	if (pvSubmitArray[SystemGameClassData::MinFuelHW].GetInteger() < 
		pvSubmitArray[SystemGameClassData::BuilderPopLevel].GetInteger() ||

		pvSubmitArray[SystemGameClassData::MaxFuelHW].GetInteger() > iMaxResourcesPerPlanet ||
		
		pvSubmitArray[SystemGameClassData::MinFuelHW].GetInteger() > 
		pvSubmitArray[SystemGameClassData::MaxFuelHW].GetInteger()) {
		
		AddMessage ("Incorrect homeworld fuel level");
		return ERROR_FAILURE;
	}
	
	// Num secs per update
	if (pvSubmitArray[SystemGameClassData::NumSecPerUpdate].GetInteger() < iMinNumSecsPerUpdate || 
		pvSubmitArray[SystemGameClassData::NumSecPerUpdate].GetInteger() > iMaxNumSecsPerUpdate) {
		
		AddMessage ("Invalid update period");
		return ERROR_FAILURE;
	}
	
	// Diplomacy
	if (!g_pGameEngine->IsLegalDiplomacyLevel (iDip)) {
		AddMessage ("Illegal Diplomacy level");
		return ERROR_FAILURE;
	}

	// Subjective views and exposed maps
	iTemp = pvSubmitArray[SystemGameClassData::Options].GetInteger();
	if ((iTemp & SUBJECTIVE_VIEWS) && (iTemp & EXPOSED_MAP)) {
		pvSubmitArray[SystemGameClassData::Options] = iTemp & ~SUBJECTIVE_VIEWS;
		AddMessage ("Games with exposed maps cannot have subjective views. "\
			"This option has been turned off");
	}
	
	// Diplomacy for map shared
	switch (pvSubmitArray[SystemGameClassData::MapsShared].GetInteger()) {
		
	case NO_DIPLOMACY:
		break;
		
	case TRUCE:
	case TRADE:
	case ALLIANCE:
		
		if (!g_pGameEngine->GameAllowsDiplomacy (
			iDip, 
			pvSubmitArray[SystemGameClassData::MapsShared].GetInteger())
			) {
			
			AddMessage ("The shared map diplomacy level must be selectable");
			return ERROR_FAILURE;
		}
		break;
		
	default:
		
		AddMessage ("Illegal shared map diplomacy level");
		return ERROR_FAILURE;
	}
	
	// Alliance limit options without limits
	if (pvSubmitArray[SystemGameClassData::MaxNumAlliances].GetInteger() == UNRESTRICTED_DIPLOMACY) {
		
		int iOptions = pvSubmitArray[SystemGameClassData::Options].GetInteger();
		
		if (iOptions & PERMANENT_ALLIANCES) {
			AddMessage ("Permanent alliances have been unselected");
			iOptions &= ~PERMANENT_ALLIANCES;
			pvSubmitArray[SystemGameClassData::Options] = iOptions;
		}
	}
	
	// InitDevShips
	ENUMERATE_TECHS(i) {
		
		sprintf (pszShipString, "InitShip%i", i);
		
		if ((pHttpForm = m_pHttpRequest->GetForm (pszShipString)) != NULL) {
			iInitTechDevs |= TECH_BITS[i];
		}
	}
	
	pvSubmitArray[SystemGameClassData::InitialTechDevs] = iInitTechDevs;
	
	
	// DevShips
	ENUMERATE_TECHS(i) {
		
		sprintf (pszShipString, "DevShip%i", i);
		
		if ((pHttpForm = m_pHttpRequest->GetForm (pszShipString)) != NULL) {
			iDevTechDevs |= TECH_BITS[i];
		}
	}
	
	pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = iDevTechDevs;
	
	if (iDevTechDevs == 0) {
		AddMessage ("At least one ship must be developable");
		return ERROR_FAILURE;
	}
	
	// Verify that the initdevships can be developed
	ENUMERATE_TECHS(i) {
		
		if ((iInitTechDevs & TECH_BITS[i]) && !(iDevTechDevs & TECH_BITS[i])) {
			
			// An initial ship couldn't be developed
			AddMessage ("An initial ship could not be developed");
			return ERROR_FAILURE;
		}
	}
	
	// Verify that engineers can be built if disconnected maps are selected
	if ((pvSubmitArray[SystemGameClassData::Options].GetInteger() & DISCONNECTED_MAP) &&
		!(iDevTechDevs & TECH_BITS[ENGINEER])
		) {
		AddMessage ("Engineer ships must be developable if maps are disconnected");
		return ERROR_FAILURE;
	}
	
	// Verify that if !mapexposed, sci's can be developed
	if (!(pvSubmitArray[SystemGameClassData::Options].GetInteger() & EXPOSED_MAP) && 
		!(iDevTechDevs & TECH_SCIENCE)
		) {
		AddMessage ("Science ships must be developable if maps are not exposed");
		return ERROR_FAILURE;
	}
	
	// Make sure that minefields aren't selected without minesweepers
	if ((iDevTechDevs & TECH_MINEFIELD) && 
		!(iDevTechDevs & TECH_MINESWEEPER) &&
		pvSubmitArray[SystemGameClassData::MaxAgRatio].GetFloat() > MAX_MAX_AG_RATIO_WITHOUT_SWEEPERS
		) {
		AddMessage ("Minefields cannot be developed if minesweepers cannot be developed and the ag ratio is less than ");
		AppendMessage (MAX_MAX_AG_RATIO_WITHOUT_SWEEPERS);
		return ERROR_FAILURE;
	}
	
	// Make sure that independence wasn't selected with maxnumempires = 2
	if (pvSubmitArray[SystemGameClassData::Options].GetInteger() & INDEPENDENCE &&
		pvSubmitArray[SystemGameClassData::MaxNumEmpires].GetInteger() == 2) {
		
		pvSubmitArray[SystemGameClassData::Options] = 
			pvSubmitArray[SystemGameClassData::Options].GetInteger() & ~INDEPENDENCE;
		
		AddMessage ("Independence requires more than two empires in a game.");
	}
	
	return OK;
	
Cleanup:
	
	AddMessage (
		"An error occurred reading data from the system database. Please contact the administrator"
		);
	return iErrCode;
}


void HtmlRenderer::WriteProfile (int iTargetEmpireKey, bool bEmpireAdmin) {

	bool bCanBroadcast;
	
	String strHtml, strLogin, strCreation;
	
	OutputText ("<input type=\"hidden\" name=\"TargetEmpireKey\" value=\"");
	m_pHttpResponse->WriteText (iTargetEmpireKey);
	OutputText ("\">");
	
	Variant* pvEmpireData = NULL;
	int iNumActiveGames, iNumPersonalGameClasses, iNumUnreadMessages;
	
	NamedMutex nmLock;
	g_pGameEngine->LockEmpire (iTargetEmpireKey, &nmLock);
	
	int iErrCode = g_pGameEngine->GetEmpireData (iTargetEmpireKey, &pvEmpireData, &iNumActiveGames);	
	if (iErrCode != OK) {
		goto OnError;
	}
	
	iErrCode = g_pGameEngine->GetEmpireGameClassKeys (iTargetEmpireKey, NULL, &iNumPersonalGameClasses);
	if (iErrCode != OK) {
		goto OnError;
	}
	
	// Name and alien icon
	OutputText ("<p>");
	
	WriteAlienString (
		pvEmpireData[SystemEmpireData::AlienKey].GetInteger(),
		iTargetEmpireKey,
		NULL,
		true
		);
	
	OutputText (" <font size=\"+2\">");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Name].GetCharPtr());
	OutputText ("</font><p>");
	
	// PGC button
	if (!bEmpireAdmin && iNumPersonalGameClasses > 0) {
		WriteButton (BID_VIEWEMPIRESGAMECLASSES);
	}
	
	// Nuke history button
	if (!bEmpireAdmin) {
		
		int iNumNukes, iNumNuked;
		iErrCode = g_pGameEngine->GetNumEmpiresInNukeHistory (iTargetEmpireKey, &iNumNukes, &iNumNuked);
		if (iErrCode != OK) {
			goto OnError;
		}
		
		if (iNumNukes > 0 || iNumNuked > 0) {
			WriteButton (BID_VIEWEMPIRESNUKEHISTORY);
		}
	}
	
	OutputText (
		"<p><table border=\"0\" width=\"80%\">"\
        "<tr>"\
		"<td><strong>Empire Name:</strong></td>"\
		"<td>"
		);
	
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Name].GetCharPtr());
	
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
		"<td><strong>Empire Key:</strong></td>"\
		"<td>"
		);
	
	m_pHttpResponse->WriteText (iTargetEmpireKey);
	
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
		"<td><strong>Privilege:</strong></td>"\
		"<td>"
		);
	
	if (!bEmpireAdmin || m_iEmpireKey == iTargetEmpireKey || iTargetEmpireKey == ROOT_KEY || 
		iTargetEmpireKey == GUEST_KEY) {
		
		m_pHttpResponse->WriteText (PRIVILEGE_STRING [pvEmpireData[SystemEmpireData::Privilege].GetInteger()]);
		
	} else {
		
		int i, j;
		
		OutputText ("<input type=\"hidden\" name=\"OldPriv\" value=\"");
		m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Privilege].GetInteger());
		OutputText ("\">");
		
		OutputText ("<select name=\"NewPriv\">");
		
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
		OutputText ("</select>");
	}
	
	OutputText (
		"</td>"\
		"<td>&nbsp;</td>"\
		"<td><strong>Nuked:</strong></td>"\
		"<td>"
		);
	
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Nuked].GetInteger());
	OutputText (
		"</td>"\
        "</tr>"\
        "<tr>"\
		"<td><strong>Broadcast:</strong></td>"\
		"<td>"
		);
	
	bCanBroadcast = (pvEmpireData[SystemEmpireData::Options].GetInteger() & CAN_BROADCAST) != 0;
	
	if (!bEmpireAdmin || m_iEmpireKey == iTargetEmpireKey || iTargetEmpireKey == ROOT_KEY
		|| iTargetEmpireKey == GUEST_KEY) {
		
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
		OutputText ("\">");
		
		OutputText ("<select name=\"Broadcast\">");
		
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
	
	// Draws
	OutputText ("</td><td>&nbsp;</td><td><strong>Draws:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Draws].GetInteger());
	
	// Real Name
	OutputText ("</td></tr><tr><td><strong>Real Name:</strong></td><td>");
	iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::RealName].GetCharPtr(), &strHtml, 0, false);
	if (iErrCode != OK) {
		OutputText ("The server is out of memory");
	} else {
		m_pHttpResponse->WriteText (strHtml.GetCharPtr(), strHtml.GetLength());
	}
	
	// Ruins
	OutputText ("</td><td>&nbsp;</td><td><strong>Ruins:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Ruins].GetInteger());
	
	// Web page
	OutputText ("</td></tr><tr><td><strong>Webpage URL:</strong></td><td>");
	
	iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::WebPage].GetCharPtr(), &strHtml, 0, false);
	if (iErrCode == OK && !strHtml.IsBlank()) {
		
		OutputText ("<a href=\"");
		
		if (strHtml.GetLength() >= 3 && strnicmp (strHtml.GetCharPtr(), "www", 3) == 0) {
			OutputText ("http://");
		}
		
		m_pHttpResponse->WriteText (strHtml);
		OutputText ("\">");
		m_pHttpResponse->WriteText (strHtml);
		OutputText ("</a>");
	} else {
		OutputText ("&nbsp;");
	}
	
	// Almonaster Score
	OutputText ("</td><td>&nbsp;</td><td><strong>Almonaster Score: </strong></td><td>");
	
	if (bEmpireAdmin && 
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
	
	//
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
	
	// Email address
	OutputText ("</td></tr><tr><td><strong>Email Address:</strong></td><td>");
	
	iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::Email].GetCharPtr(), &strHtml, 0, false);
	if (iErrCode == OK && !strHtml.IsBlank()) {
		OutputText ("<a href=\"mailto:");
		m_pHttpResponse->WriteText (strHtml.GetCharPtr());
		OutputText ("\">");
		m_pHttpResponse->WriteText (strHtml.GetCharPtr());
		OutputText ("</a>");
	} else {
		OutputText ("&nbsp;");
	}
	
	// Significance
	OutputText ("</td><td>&nbsp;</td><td><strong>Significance:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::AlmonasterScoreSignificance].GetInteger());
	
	// Creation Time	
	OutputText ("</td></tr><tr><td><strong>Creation Time:</strong></td><td>");
	m_pHttpResponse->WriteText (pszCreationTime);
	
	// Classic Score
	OutputText ("</td><td>&nbsp;</td><td><strong>Classic Score:</strong></td><td>");
	
	if (bEmpireAdmin && m_iEmpireKey != iTargetEmpireKey && iTargetEmpireKey != ROOT_KEY &&
		iTargetEmpireKey == GUEST_KEY) {
		
		OutputText ("<input type=\"text\" size=\"12\" maxlength=\"12\" name=\"NewCScore\" value=\"");
		m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::ClassicScore].GetFloat());
		OutputText ("\">");
		
	} else {
		
		m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::ClassicScore]);
	}
	
	// Last Login
	OutputText ("</td></tr><tr><td><strong>Last Login:</strong></td><td>");
	m_pHttpResponse->WriteText (pszLoginTime);
	
	// Bridier Rank
	OutputText ("</td><td>&nbsp;</td><td><strong>Bridier Rank:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::BridierRank].GetInteger());
	
	// IP Address	
	OutputText ("</td></tr><tr><td><strong>IP Address:</strong></td><td>");
	
	if (m_iEmpireKey == iTargetEmpireKey || bEmpireAdmin || m_iPrivilege >= ADMINISTRATOR) {
		m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::IPAddress].GetCharPtr());
	} else {
		
		char pszHashedIPAddress [128];
		HashIPAddress (pvEmpireData[SystemEmpireData::IPAddress].GetCharPtr(), pszHashedIPAddress);
		
		m_pHttpResponse->WriteText (pszHashedIPAddress);
	}
	
	// Max Mil
	OutputText ("</td><td>&nbsp;</td><td><strong>Bridier Index:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::BridierIndex].GetInteger());
	
	// Browser
	OutputText ("</td></tr><tr><td><strong>Browser:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::Browser].GetCharPtr());
	
	// Max Econ
	OutputText ("</td><td>&nbsp;</td><td><strong>Max Econ:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::MaxEcon].GetInteger());
	
	// Login count
	OutputText ("</td></tr><tr><td><strong>Login Count:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::NumLogins].GetInteger());
	
	// Max Mil
	OutputText ("</td><td>&nbsp;</td><td><strong>Max Mil:</strong></td><td>");
	m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::MaxMil].GetInteger());
	
	// Login count
	OutputText ("</td></tr><tr><td><strong>Active Games:</strong></td><td>");
	m_pHttpResponse->WriteText (iNumActiveGames);
	
	// Unread messages
	OutputText ("</td><td>&nbsp;</td><td><strong>Unread Messages:</strong></td><td>");
	m_pHttpResponse->WriteText (iNumUnreadMessages);
	
	// Session Id
	if (m_iPrivilege >= ADMINISTRATOR) {
		
		OutputText (
			"</td></tr>"\

			"<tr>"\
			"<td><strong>Session Id:</strong></td>"\
			"<td>"
			);
		
		m_pHttpResponse->WriteText (pvEmpireData[SystemEmpireData::SessionId].GetInteger64());
		
		if (bEmpireAdmin) {
			OutputText ("&nbsp;&nbsp;");
			WriteButton (BID_RESET);
		}

		OutputText (
			"</td>"\
			
			"<td>&nbsp;</td>"\
			
			"<td></td><td>"
			);
	}
	
	OutputText ("</td></tr></table>");
	
	iErrCode = g_pGameEngine->GetEmpireOption (m_iEmpireKey, CAN_BROADCAST, &bCanBroadcast);
	if (iErrCode != OK) {
		goto OnError;
	}
	
	if (!bEmpireAdmin) {
		
		if (pvEmpireData[SystemEmpireData::Quote].GetCharPtr() != NULL &&
			*(pvEmpireData[SystemEmpireData::Quote].GetCharPtr()) != '\0') {
			
			OutputText ("<p><table width=\"60%\"><tr><td><font face=\"" DEFAULT_QUOTE_FONT "\">");
			
			String strQuote;
			iErrCode = HTMLFilter (pvEmpireData[SystemEmpireData::Quote].GetCharPtr(), &strQuote, 0, false);
			if (iErrCode == OK) {
				m_pHttpResponse->WriteText (strQuote.GetCharPtr());
			} else {
				OutputText ("The server is out of memory");
			}
			
			OutputText ("</td></tr></table>");
		}
		
		if (bCanBroadcast && m_iEmpireKey != iTargetEmpireKey) {
			OutputText ("<p><textarea name=\"Message\" rows=\"7\" cols=\"60\" wrap=\"hard\"></textarea><p>");
			WriteButton (BID_SENDMESSAGE);
		}
	}
	
	g_pGameEngine->UnlockEmpire (nmLock);
	g_pGameEngine->FreeData (pvEmpireData);
	
	return;
	
OnError:
	
	g_pGameEngine->UnlockEmpire (nmLock);
	
	if (pvEmpireData != NULL) {
		g_pGameEngine->FreeData (pvEmpireData);
	}
	
	OutputText ("<p><strong>Error ");
	m_pHttpResponse->WriteText (iErrCode);
	OutputText (" occurred reading data for this empire</strong>");
}


void HtmlRenderer::WriteServerRules() {
	
	int iErrCode, iNumUpdatesDown, iNumNukesListed, iValue, iNumActiveGames, iNumOpenGames, iNumClosedGames,
		iSystemOptions, iDefaultNumUpdatesForClose;
	
	float fValue;
	
	IDatabase* pDatabase = NULL;
	
	Seconds sSecondsForLongtermStatus, sAfterWeekendDelay, iUptime, iCpuTime, stFileCacheSize,
		sBridierScanFrequency;
	
	size_t iTotalPhysicalMemory, iTotalFreePhysicalMemory, iTotalSwapMemory, iTotalFreeSwapMemory,
		iTotalVirtualMemory;
	
	char pszDateString [OS::MaxDateLength];
	
	unsigned int iNumProcessors, iMHz, iNumFiles, iNumPages, iTimeSpent;
	
	String strProcessorInformation;
	bool bMMX;
	
	GameConfiguration gcConfig;
	MapConfiguration mcConfig;
	
	HttpServerStatistics stats;
	AlmonasterStatistics aStats;
	
	iErrCode = g_pGameEngine->GetGameConfiguration (&gcConfig);
	if (iErrCode != OK) {
		goto ErrorExit;
	}
	
	iErrCode = g_pGameEngine->GetMapConfiguration (&mcConfig);
	if (iErrCode != OK) {
		goto ErrorExit;
	}
	
	iErrCode = g_pGameEngine->GetNumUpdatesDownBeforeGameIsKilled (&iNumUpdatesDown);
	if (iErrCode != OK) {
		goto ErrorExit;
	}
	
	iErrCode = g_pGameEngine->GetSecondsForLongtermStatus (&sSecondsForLongtermStatus);
	if (iErrCode != OK) {
		goto ErrorExit;
	}
	
	iErrCode = g_pGameEngine->GetAfterWeekendDelay (&sAfterWeekendDelay);
	if (iErrCode != OK) {
		goto ErrorExit;
	}
	
	iErrCode = g_pGameEngine->GetNumNukesListedInNukeHistories (&iNumNukesListed);
	if (iErrCode != OK) {
		goto ErrorExit;
	}
	
	iErrCode = g_pGameEngine->GetSystemOptions (&iSystemOptions);
	if (iErrCode != OK) {
		goto ErrorExit;
	}
	
	iErrCode = g_pGameEngine->GetDefaultNumUpdatesBeforeClose (&iDefaultNumUpdatesForClose);
	if (iErrCode != OK) {
		goto ErrorExit;
	}

	iErrCode = g_pGameEngine->GetBridierTimeBombScanFrequency (&sBridierScanFrequency);
	if (iErrCode != OK) {
		goto ErrorExit;
	}
	
	OutputText ("<p><h2>Server information</h2></center><ul><li>The web server is <strong>");
	m_pHttpResponse->WriteText (g_pHttpServer->GetServerName());
	OutputText ("</strong>, running on <strong>");
	m_pHttpResponse->WriteText (g_pHttpServer->GetIPAddress());
	OutputText ("</strong> port <strong>");
	m_pHttpResponse->WriteText (g_pHttpServer->GetPort());
	OutputText ("</strong></li><li>The web server is using <strong>");
	m_pHttpResponse->WriteText (g_pHttpServer->GetNumThreads());
	OutputText ("</strong> threads in its threadpool</li>");
	
	if (OS::GetProcessMemoryStatistics (&iTotalPhysicalMemory, &iTotalVirtualMemory) == OK) {
		OutputText ("<li>The server process' working set size is <strong>");
		m_pHttpResponse->WriteText (iTotalPhysicalMemory / 1024); 
		OutputText (" KB</strong> and its virtual memory size is <strong>");
		m_pHttpResponse->WriteText (iTotalVirtualMemory / 1024); 
		OutputText (" KB</strong></li>");
	} else {
		OutputText ("<li>Process memory usage information is not available</li>");
	}
	
	if (OS::GetProcessTimeStatistics (&iUptime, &iCpuTime) == OK) {
		
		OutputText ("<li>The server process has been running for ");
		WriteTime (iUptime);
		OutputText (" and has used ");
		WriteTime (iCpuTime);
		OutputText (" of CPU time</li>");
		
	} else {
		OutputText ("<li>Process time information is not available</li>");
	}
	
	iNumFiles = g_pFileCache->GetNumFiles();
	stFileCacheSize = g_pFileCache->GetSize();
	
	OutputText ("<li>The server file cache contains <strong>");
	m_pHttpResponse->WriteText (iNumFiles);
	OutputText ("</strong> file");
	if (iNumFiles != 1) {
		OutputText ("s");
	}
	if (iNumFiles != 0) {
		OutputText (", totalling <strong>");
		m_pHttpResponse->WriteText ((unsigned int) (stFileCacheSize / 1024));
		OutputText ("</strong> KB");
	}
	OutputText ("</li>");
	
	// Stats
	iErrCode = g_pHttpServer->GetStatistics (&stats);
	if (iErrCode == OK) {
		
		OutputText ("<li>The server has handled <strong>");
		m_pHttpResponse->WriteText (stats.NumRequests);
		OutputText ("</strong> requests today, totalling <strong>");
		m_pHttpResponse->WriteText (stats.NumBytesReceived / 1024);
		OutputText ("</strong> KB received and <strong>");
		m_pHttpResponse->WriteText (stats.NumBytesSent / 1024);
		OutputText ("</strong> KB sent</li>");
	}
	
	pDatabase = g_pGameEngine->GetDatabase();
	
	OutputText ("<li>The server database contains <strong>");
	m_pHttpResponse->WriteText (pDatabase->GetNumTables());
	OutputText ("</strong> tables, <strong>");
	m_pHttpResponse->WriteText (pDatabase->GetNumLoadedRows());
	OutputText ("</strong> loaded rows and <strong>");
	m_pHttpResponse->WriteText (pDatabase->GetNumTemplates());
	OutputText ("</strong> templates and occupies <strong>");
	m_pHttpResponse->WriteText ((int) pDatabase->GetSizeOnDisk() / 1024);
	OutputText ("</strong> KB on disk</li>");
	
	pDatabase->Release();
	
	aStats = m_sStats;
	iNumPages = aStats.NumPageScriptRenders;
	iTimeSpent = aStats.TotalScriptTime;
	
	OutputText (
		"<li>Since the server was started:<ul>"\
		"<li><strong>"
		);

	m_pHttpResponse->WriteText (iNumPages);	

	OutputText ("</strong> page");
	if (iNumPages != 1) {
		OutputText ("s have");
	} else {
		OutputText (" has");
	}
	
	OutputText (" been rendered");

	if (iNumPages > 0) {

		OutputText (", with an average script time of <strong>");
		m_pHttpResponse->WriteText (iTimeSpent / iNumPages);
		OutputText ("</strong> ms");
	}
		
	OutputText ("</li><li><strong>");
	m_pHttpResponse->WriteText (aStats.Logins);
	OutputText ("</strong> empire");
	if (aStats.Logins != 1) {
		OutputText ("s have");
	} else {
		OutputText (" has");
	}
	OutputText (" logged in</li>");
	
	OutputText ("<li><strong>");
	m_pHttpResponse->WriteText (aStats.EmpiresCreated);
	OutputText ("</strong> empire");
	if (aStats.EmpiresCreated != 1) {
		OutputText ("s have");
	} else {
		OutputText (" has");
	}
	OutputText (" been created</li>");
	
	OutputText ("<li><strong>");
	m_pHttpResponse->WriteText (aStats.EmpiresDeleted);
	OutputText ("</strong> empire");
	if (aStats.EmpiresDeleted != 1) {
		OutputText ("s have");
	} else {
		OutputText (" has");
	}
	OutputText (" been deleted</li>");
	
	OutputText ("<li><strong>");
	m_pHttpResponse->WriteText (aStats.GamesStarted);
	OutputText ("</strong> game");
	if (aStats.GamesStarted != 1) {
		OutputText ("s have");
	} else {
		OutputText (" has");
	}
	OutputText (" been started</li>");
	
	OutputText ("<li><strong>");
	m_pHttpResponse->WriteText (aStats.GamesEnded);
	OutputText ("</strong> game");
	if (aStats.GamesEnded != 1) {
		OutputText ("s have");
	} else {
		OutputText (" has");
	}
	OutputText (" ended</li></ul></li>");
	
	if (g_pGameEngine->GetNumOpenGames (&iNumOpenGames) == OK &&
		g_pGameEngine->GetNumClosedGames (&iNumClosedGames) == OK) {
		
		iNumActiveGames = iNumOpenGames + iNumClosedGames;
		
		if (iNumActiveGames == 0) {
			OutputText ("<li>There are no active games on the server</li>");
		} else { 			
			OutputText ("<li>There ");
			if (iNumActiveGames == 1) { 
				OutputText ("is <strong>1</strong> active game");
			} else {
				OutputText ("are <strong>");
				m_pHttpResponse->WriteText (iNumActiveGames);
				OutputText ("</strong> active games");
			}
			OutputText (" on the server (<strong>");
			m_pHttpResponse->WriteText (iNumOpenGames);
			OutputText ("</strong> open, <strong>");
			m_pHttpResponse->WriteText (iNumClosedGames); 
			OutputText ("</strong> closed)</li>");
		}
	}
	
	// Machine information
	OutputText ("</ul><p><center><h2>Machine information</h2></center><ul>");
	
	iErrCode = Time::GetDateString (pszDateString);
	if (iErrCode == OK) {
		
		OutputText ("<li>The server's local time is <strong>");
		m_pHttpResponse->WriteText (pszDateString);
		OutputText ("</strong>");
		
		int iBias;
		char pszTimeZone[OS::MaxTimeZoneLength];
		
		if (Time::GetTimeZone (pszTimeZone, &iBias) == OK) {
			OutputText (", <strong>");
			m_pHttpResponse->WriteText (pszTimeZone);
			OutputText ("</strong>");
			
			if (iBias != 0) {
				OutputText (" (<strong>");
				if (iBias < 0) {
					OutputText ("GMT");
					m_pHttpResponse->WriteText (iBias / 60);
				} else {
					OutputText ("GMT+");
					m_pHttpResponse->WriteText (iBias / 60);
				}
				OutputText ("</strong>)");
			}
		}
		
		OutputText ("</li>");
	}
	
	OutputText ("<li>The server machine is running <strong>");
	
	char pszOSVersion[OS::MaxOSVersionLength];
	
	iErrCode = OS::GetOSVersion (pszOSVersion);
	if (iErrCode == OK) {
		m_pHttpResponse->WriteText (pszOSVersion);
	} else {
		OutputText ("Could not obtain OS version");
	}
	
	char pszProcessorInformation[OS::MaxProcessorInfoLength];
	
	if (OS::GetProcessorInformation (pszProcessorInformation, &iNumProcessors, &bMMX, &iMHz) == OK) {
		
		OutputText ("</strong></li><li>The server machine has <strong>");
		m_pHttpResponse->WriteText (iNumProcessors);
		if (bMMX) {
			OutputText (" MMX-enabled ");
		} else {
			OutputText (" ");
		}
		if (iMHz != CPU_SPEED_UNAVAILABLE) {
			m_pHttpResponse->WriteText (iMHz);
			OutputText (" MHz ");
		}
		m_pHttpResponse->WriteText (pszProcessorInformation);
		OutputText ("</strong> processor");
		
		if (iNumProcessors != 1) {
			OutputText ("s");
		}
		OutputText ("</li>");
	}
	
	if (OS::GetMemoryStatistics (
		&iTotalPhysicalMemory, 
		&iTotalFreePhysicalMemory, 
		&iTotalSwapMemory, 
		&iTotalFreeSwapMemory
		) == OK) {
		
		OutputText ("<li>The server machine has <strong>");
		m_pHttpResponse->WriteText (iTotalPhysicalMemory / 1024);
		OutputText (" KB</strong> of physical memory, of which <strong>");
		m_pHttpResponse->WriteText ((iTotalPhysicalMemory - iTotalFreePhysicalMemory) / 1024);
		OutputText (" KB</strong> are in use</li><li>The server machine has <strong>");
		m_pHttpResponse->WriteText (iTotalSwapMemory / 1024);
		OutputText (" KB</strong> of swap memory, of which <strong>");
		m_pHttpResponse->WriteText ((iTotalSwapMemory - iTotalFreeSwapMemory) / 1024);
		OutputText (" KB</strong> are in use</li>");
	}
	
	OutputText ("</ul><p><center><h2>Ships</h2></center><ul><li>Ships are not destroyed "\
		"if they perform special actions at costs that are beneath their full capacities</li>"\
		"<li>Engineers lose <strong>");
	
	m_pHttpResponse->WriteText (gcConfig.fEngineerLinkCost); 
	OutputText ("</strong> BR and MaxBR when they open or close a link</li><li>Stargates lose <strong>");
	m_pHttpResponse->WriteText (gcConfig.fStargateGateCost); 
	OutputText ("</strong> BR and MaxBR when they stargate at least one ship</li>");
	
	if (gcConfig.iShipBehavior & STARGATE_LIMIT_RANGE) {
		
		OutputText ("<li>Stargates can stargate ships to planets up BR / <strong>");
		m_pHttpResponse->WriteText (gcConfig.fJumpgateRangeFactor);
		OutputText ("</strong> hops away </li>");
		
	} else {
		
		OutputText ("<li>Stargate range limitations are <strong>not</strong> enforced</li>");
	}
	
	OutputText ("<li>Colonies cost ");
	
	if (!(gcConfig.iShipBehavior & COLONY_USE_MULTIPLIED_BUILD_COST)) {
		
		OutputText ("<strong>");
		m_pHttpResponse->WriteText (gcConfig.iColonySimpleBuildFactor);
		OutputText ("</strong> population unit");
		
		if (gcConfig.iColonySimpleBuildFactor != 1) {
			OutputText ("s");
		}
		
	} else {
		
		OutputText ("BR * <strong>");
		m_pHttpResponse->WriteText (gcConfig.fColonyMultipliedBuildFactor);
		OutputText ("</strong>");
	}
	OutputText (" each to build</li>");
	
	OutputText ("<li>Colonies deposit up to their BR");
	if (gcConfig.iShipBehavior & COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT) {
		OutputText (" * <strong>");
		m_pHttpResponse->WriteText (gcConfig.fColonyMultipliedDepositFactor);
	} else {
		OutputText (" ^ <strong>");
		m_pHttpResponse->WriteText (gcConfig.fColonyExponentialDepositFactor);
	}
	OutputText ("</strong> population units when colonizing planets</li>"\
		"<li>Colonies can also settle (deposit population units upon) already colonized planets</li>"\
		"<li>Terraformers can increase a planet's agriculture by up to their BR * <strong>");
	
	m_pHttpResponse->WriteText (gcConfig.fTerraformerPlowFactor);
	OutputText ("</strong> agriculture units</li>"\
		"<li>Multiple terraformers can act upon a planet at the same time</li>"\
		"<li>Terraformers can act upon any planet, even those uncolonized or belonging to an enemy"\
		"<li>Troopships can invade a planet with up to BR * <strong>");
	
	m_pHttpResponse->WriteText (gcConfig.fTroopshipInvasionFactor); 
	OutputText ("</strong> population units</li>");
	
	OutputText ("<li>Troopships that fail to invade destroy BR * <strong>");
	m_pHttpResponse->WriteText (gcConfig.fTroopshipFailureFactor);
	OutputText ("</strong> of the planet's population units</li>"\
		"<li>Troopships that successfully invade destroy <strong>");
	
	m_pHttpResponse->WriteText (gcConfig.fTroopshipSuccessFactor * (float) 100);
	OutputText ("</strong>% of the planet's population units</li><li>Cloakers are automatically <strong>");
	
	if (!(gcConfig.iShipBehavior & CLOAKER_CLOAK_ON_BUILD)) {
		OutputText ("un");
	}
	OutputText ("cloaked</strong> when built</li><li>Morphers are automatically <strong>");
	
	if (!(gcConfig.iShipBehavior & MORPHER_CLOAK_ON_CLOAKER_MORPH)) {
		OutputText ("un");
	}
	OutputText ("cloaked</strong> when they morph into cloakers</li>"\
		"<li>Planets annihilated by a doomsday will remain quarantined for BR * <strong>");
	m_pHttpResponse->WriteText (gcConfig.fDoomsdayAnnihilationFactor);
	OutputText ("</strong> updates and will have their agriculture level reduced to zero</li>");
	
	OutputText ("<li>Carriers lose <strong>");
	m_pHttpResponse->WriteText (gcConfig.fCarrierCost);
	OutputText ("</strong> BR and MaxBR when involved in a fleet battle</li>"\
		
		"<li>Builders require <strong>");
	m_pHttpResponse->WriteText (gcConfig.fBuilderMinBR);
	OutputText ("</strong> BR to create a planet</li>"\
		
		"<li>Morphers lose <strong>");
	m_pHttpResponse->WriteText (gcConfig.fMorpherCost);
	OutputText ("</strong> BR and MaxBR when they morph to a different tech type</li>"\
		
		"<li>Jumpgates lose <strong>");
	m_pHttpResponse->WriteText (gcConfig.fJumpgateGateCost);
	OutputText ("</strong> BR and MaxBR when they when they jumpgate at least one ship</li>");
	
	if (gcConfig.iShipBehavior & JUMPGATE_LIMIT_RANGE) {
		
		OutputText ("<li>Jumpgates can jumpgate ships to planets up to BR / <strong>");
		m_pHttpResponse->WriteText (gcConfig.fJumpgateRangeFactor);
		OutputText ("</strong> hops away </li>");
		
	} else {
		
		OutputText ("<li>Jumpgate range limitations are <strong>not</strong> enforced</li>");
	}
	
	OutputText ("<li>Colony population deposit (settle) is <strong>");
	if (gcConfig.iShipBehavior & COLONY_DISABLE_SETTLES) {
		OutputText ("disabled");
	} else {
		OutputText ("enabled");
	}
	
	OutputText ("</strong></li><li>Terraforming non-owned planets is <strong>");
	if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_FRIENDLY) {
		OutputText ("disabled");
	} else {
		OutputText ("enabled");
	}

	OutputText ("</strong></li><li>Colony survival after colonizing or settling is <strong>");
	if (gcConfig.iShipBehavior & COLONY_DISABLE_SURVIVAL) {
		OutputText ("disabled");
	} else {
		OutputText ("enabled");
	}
	
	OutputText ("</strong></li><li>Terraformer survival after terraforming is <strong>");
	if (gcConfig.iShipBehavior & TERRAFORMER_DISABLE_SURVIVAL) {
		OutputText ("disabled");
	} else {
		OutputText ("enabled");
	}
	
	OutputText ("</strong></li><li>Troopship survival after invading is <strong>");
	if (gcConfig.iShipBehavior & TROOPSHIP_DISABLE_SURVIVAL) {
		OutputText ("disabled");
	} else {
		OutputText ("enabled");
	}
	
	OutputText ("</strong></li><li>Minefield detonation is <strong>");
	if (gcConfig.iShipBehavior & MINEFIELD_DISABLE_DETONATE) {
		OutputText ("disabled");
	} else {
		OutputText ("enabled");
	}
	
	// Planets and maps
	OutputText ("</strong></li></ul><p><center><h2>Planets and maps</h2></center>"\
		"<ul><li>Planets that have been nuked <strong>");
	m_pHttpResponse->WriteText (gcConfig.iNukesForQuarantine);
	OutputText ("</strong> or more times are automatically be quarantined for <strong>");
	
	m_pHttpResponse->WriteText (gcConfig.iUpdatesInQuarantine);
	OutputText ("</strong> updates after each nuke and have their agriculture reduced to zero</li>"\
		"<li>When generating a map, the chance that a planet will have a link to an adjacent "\
		"planet that it wasn't originally linked to is <strong>");
	
	m_pHttpResponse->WriteText (mcConfig.iChanceNewLinkForms);
	OutputText ("%</strong></li><li>Planets can have up to <strong>");
	m_pHttpResponse->WriteText (mcConfig.fResourceAllocationRandomizationFactor);
	OutputText ("</strong> times their average allocation in resources</li>"\
		
		"<li>The maximum map deviation is <strong>");
	m_pHttpResponse->WriteText (mcConfig.iMapDeviation);
	OutputText ("</strong>. This number determines the approximate coordinates of the first planet on the map</li>"
		
		"<li>The chance that a new planet in a chain will be linked to the last created planet "\
		"in that chain is <strong>");
	m_pHttpResponse->WriteText (mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap);
	OutputText ("</strong> for large maps and <strong>");
	m_pHttpResponse->WriteText (mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap);
	OutputText ("</strong> for small maps</li>"\
		
		"<li>Large maps are those with <strong>");
	m_pHttpResponse->WriteText (mcConfig.iLargeMapThreshold);
	OutputText ("</strong> or more planets per empire</li>"\
		
		"</ul>"\
		"<p><center><h2>Gameplay</h2></center>"\
		"<ul><li>Empires can quit from a game at any time before the game starts</li>"\
		"<li>When an empire enters or quits from a game, its name is broadcast to everyone else in the game</li>"\
		"<li>The IP addresses that appear in the diplomacy and profile pages are not real addresses.  However,"\
		" two empires played from the same IP address will have the same value.  Beware, however, of accusing"\
		" players of multi-emping when they're simply behind the same firewall</li>"\
		"<li>The tech level of empires who join a game late is determined by adding to the initial BR for "\
		"the game <strong>");
	
	m_pHttpResponse->WriteText (gcConfig.iPercentTechIncreaseForLatecomers);
	OutputText ("</strong>% of the product of the number of updates missed "\
		"times the max increase allowed by the gameclass</li>"\
		"<li>The first trade agreement will increase an empire's econ by <strong>");
	
	m_pHttpResponse->WriteText (gcConfig.iPercentFirstTradeIncrease);
	OutputText ("%</strong>. Subsequent trade agreements will give <strong>");
	m_pHttpResponse->WriteText (gcConfig.iPercentNextTradeIncrease);
	OutputText ("%</strong> of the previous increase</li>");
	
	OutputText ("<li><strong>");
	m_pHttpResponse->WriteText (gcConfig.iPercentDamageUsedToDestroy);
	OutputText (
		"</strong>% of TOT_DMG is converted to DEST during ship combat</li>"\
		"<li>If all empires in a game are idle, then the game will end automatically</li>"\
		"<li>If the server is down for <strong>"
		);
	
	m_pHttpResponse->WriteText (iNumUpdatesDown);
	OutputText ("</strong> update");
	if (iNumUpdatesDown != 1) {
		OutputText ("s");
	} 
	OutputText (" of a short term game, then the game automatically ends</li>"\
		"<li>Short term games are those with update periods of less than <strong>");
	
	WriteTime (sSecondsForLongtermStatus);
	
	OutputText (
		"</strong></li>"\
		"<li>Games with no weekend updates postpone updates scheduled to occur on a weekend until the following Monday"\
		" at 00:00:00 server time plus ");
	
	WriteTime (sAfterWeekendDelay);
	
	OutputText ("</li><li>Grudge games <strong>");
	
	if (!(iSystemOptions & DEFAULT_BRIDIER_GAMES)) {
		OutputText ("do not ");
	}
	
	OutputText (
		"count</strong> towards Bridier Scoring by default</li>"\
		
		"<li>The default IP Address filtering is "
		);
	
	if ((iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) ==
		(DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) {
		OutputText ("<strong>warn</strong> and <strong>reject</strong>");
	}
	
	else if (iSystemOptions & DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
		OutputText ("<strong>warn</strong>");
	}
	
	else if (iSystemOptions & DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
		OutputText ("<strong>reject</strong>");
	}
	
	else {
		OutputText ("<strong>none</strong>");
	}
	OutputText ("</li>"
		
		"<li>The default Session Id filtering is "
		);
	
	if ((iSystemOptions & (DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) ==
		(DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS)) {
		OutputText ("<strong>warn</strong> and <strong>reject</strong>");
	}
	
	else if (iSystemOptions & DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS) {
		OutputText ("<strong>warn</strong>");
	}
	
	else if (iSystemOptions & DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
		OutputText ("<strong>reject</strong>");
	}
	
	else {
		OutputText ("<strong>none</strong>");
	}
	
	OutputText ("</li><li>The default number of updates before a game closes is <strong>");
	m_pHttpResponse->WriteText (iDefaultNumUpdatesForClose);
	
	OutputText ("</strong>"\
		
		"</li></ul><p><center><h2>User Interface</h2></center>"\
		
		"<li>Empire names are case insensitive and all beginning and trailing spaces are automatically removed</li>"\
		"<li>Empire names and user input are fully filtered for HTML content</li>"\
		"<li>The chatroom will allow a maximum of <strong>");
	
	m_pHttpResponse->WriteText (g_pChatroom->GetMaxNumSpeakers());
	OutputText ("</strong> simultaneous empires</li>");
	
	OutputText ("<li>The chatroom will display the last <strong>");
	m_pHttpResponse->WriteText (g_pChatroom->GetMaxNumMessages());
	OutputText ("</strong> messages</li>");
	
	OutputText ("<li>Empires time out of the chatroom when they are idle for ");
	WriteTime (g_pChatroom->GetTimeOut());
	
	iErrCode = g_pGameEngine->GetMaxIconSize (&iValue);
	OutputText ("</li><li>The maximum size of an uploaded alien icon is <strong>");
	m_pHttpResponse->WriteText (iValue);
	OutputText ("</strong> bytes</li>");
	
	iErrCode = g_pGameEngine->GetDefaultAlien (&iValue);
	
	if (iErrCode == OK) {
		OutputText ("<li>The default alien icon for new empires is: ");
		WriteAlienString (iValue, NO_KEY, NULL, false);	
		OutputText ("</li>");
	}
	
	OutputText ("</ul><center><h2>Score</h2></center><ul><li>A win gives <strong>");
	
	m_pHttpResponse->WriteText (CLASSIC_POINTS_FOR_WIN);
	OutputText ("</strong> classic points</li><li>A draw gives <strong>");
	
	m_pHttpResponse->WriteText (CLASSIC_POINTS_FOR_DRAW);
	OutputText ("</strong> classic points</li><li>A nuke gives <strong>");
	
	m_pHttpResponse->WriteText (CLASSIC_POINTS_FOR_NUKE);
	OutputText ("</strong> classic points</li><li>Being nuked subtracts <strong>");

	m_pHttpResponse->WriteText (- CLASSIC_POINTS_FOR_NUKED);
	OutputText ("</strong> classic points</li><li>Ruining subtracts <strong>");
	
	m_pHttpResponse->WriteText (- CLASSIC_POINTS_FOR_RUIN);
	OutputText ("</strong> classic points</li><li>The Almonaster base unit is <strong>");
	
	m_pHttpResponse->WriteText (ALMONASTER_BASE_UNIT);
	OutputText ("</strong> points</li><li>The minimum Almonaster score is <strong>");
	
	m_pHttpResponse->WriteText (ALMONASTER_MIN_SCORE);

	OutputText ("</strong> points</li><li>The initial Almonaster score is <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_INITIAL_SCORE);
	
	OutputText ("</strong> points</li><li>The Almonaster score increase factor is between <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_MAX_INCREASE_FACTOR);
	OutputText ("</strong> and <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_MAX_DECREASE_FACTOR);
	
	OutputText ("</strong></li><li>The Almonaster score alliance ratio is between <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_MIN_ALLIANCE_RATIO);
	OutputText ("</strong> and <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_MAX_ALLIANCE_RATIO);
	
	OutputText ("</strong></li><li>The Almonaster score nuker significance ratio is between <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_MIN_NUKER_SIGNIFICANCE_RATIO);
	OutputText ("</strong> and <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_MAX_NUKER_SIGNIFICANCE_RATIO);
	
	OutputText ("</strong></li><li>The Almonaster score nuked significance ratio is between <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_MIN_NUKED_SIGNIFICANCE_RATIO);
	OutputText ("</strong> and <strong>");
	m_pHttpResponse->WriteText (ALMONASTER_MAX_NUKED_SIGNIFICANCE_RATIO);
	OutputText ("</strong></li>");
	
	if (g_pGameEngine->GetApprenticeScore (&fValue) == OK) {
		
		OutputText ("<li>Empires with Almonaster scores greater than <strong>");
		m_pHttpResponse->WriteText (fValue);	
		OutputText ("</strong> are considered Apprentices and have the right to create their own personal games</li>");
	}
	
	if (g_pGameEngine->GetAdeptScore (&fValue) == OK &&
		g_pGameEngine->GetMaxNumPersonalGameClasses (&iValue) == OK) {
		
		OutputText ("<li>Empires with Almonaster scores greater than <strong>");
		m_pHttpResponse->WriteText (fValue);
		
		OutputText ("</strong> are considered Adepts and have the right to create up to <strong>");
		m_pHttpResponse->WriteText (iValue);
		OutputText ("</strong> Personal GameClasses</li>");
	}

	OutputText (
		"</li>"\
		"<li>Empires with no Bridier activity for three months will see their Bridier Index "\
		"set to <strong>200</strong> if it is less.</li>"\
		"<li>Empires with no Bridier activity for four months will see their Bridier Index "\
		"set to <strong>300</strong> if it is less.</li>"\
		"<li>Empires with no Bridier activity for five months will see their Bridier Index "\
		"set to <strong>400</strong> if it is less.</li>"\
		"<li>Empires with no Bridier activity for six months will see their Bridier Index "\
		"set to <strong>500</strong> if it is less.</li>"\
		"<li>Empires will be checked for Bridier activity every "
		);

	WriteTime (sBridierScanFrequency);
	OutputText (
		"</li>"\
		"<li>Up to <strong>"
		);
	
	m_pHttpResponse->WriteText (iNumNukesListed);
	OutputText ("</strong> nuke");
	if (iNumNukesListed != 1) {
		OutputText ("s");
	}
	
	OutputText (
		
		" will be listed in each empire's nuke history</li>"\
		
		"</ul><center><h2>Nomenclature</h2></center>"
		
		"<center><table width=\"75%\" border=\"1\">"\
		"<tr>"\
		"<td><strong>Weekend Updates (Weekend)</strong></td>"\
		"<td>Does the game update on weekends?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Private Messages (PrivateMsg)</strong></td>"\
		"<td>Can private messages be sent between empires?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Entry List (EntryList)</strong></td>"\
		"<td>Are empire names exposed in game lists and broadcast when they enter games?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Exposed Diplomacy (DipExposed)</strong></td>"\
		"<td>Have all empires in the game met each other initially?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Exposed Maps (MapExposed)</strong></td>"\
		"<td>Are all planets visible to all empires initially?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Shared Maps (MapShared)</strong></td>"\
		"<td>Can empires with a given diplomatic agreement share their maps?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Fully Colonized Maps</strong></td>"\
		"<td>Do empires begin the game with their planets already colonized?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Disconnected Maps (Disconnected)</strong></td>"\
		"<td>Are empire's planets connected to other empire's planets when the game begins?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Visible Builds</strong></td>"\
		"<td>Are ships visible to other empires during the update in which they're being built?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Visible Diplomacy</strong></td>"\
		"<td>Will diplomacy offerings be visible until the next update?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Population needed to Build (BuildPop)</strong></td>"\
		"<td>What population is needed at a planet for it to be able to build ships?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Maximum Ag Ratio (MaxAgRatio)</strong></td>"\
		"<td>What is the maximum possible ag ratio for the game?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Updates for Idle Status (IdleUpdates)</strong></td>"\
		"<td>How many updates can an empire be absent from the game before the empire becomes idle "\
		"and is automatically ready for an update?</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Fair diplomacy</strong></td>"\
		"<td>Empires are limited to a maximum of (N - 2) / 2 empires at the specified diplomacy level,"\
		"where N is the greatest number of empires in the game at any one time"\
		"</td></tr>"\
		
		"<tr>"\
		"<td><strong>X truces, trades or alliances</strong></td>"\
		"<td>Empires are limited to a maximum of X empires at a certain level of diplomacy. "\
		"Alliances count as trades and truces, and trades count as truces. If alliance limits are set to "\
		"count for the entire game, then any alliance that ceases to exist (due to an annihilation, or at "\
		"least one empire dropping down to another level), then that alliance will count for the entire game "\
		"and will limit the ability of the affected empires to establish other alliances. "\
		"Empires who were previously allied can ally again with no additional counts added."\
		"</td></tr>"\
		
		"<tr>"\
		"<td><strong>Unbreakable alliances</strong></td>"\
		"<td>If alliances are unbreakable, then empires who are at alliance cannot drop down to any "\
		"other diplomatic level and will remain at alliance until one of them leaves the game."\
		"</td></tr>"\
		
		"<tr>"\
		"<td><strong>Surrenders</strong></td>"\
		"<td>Surrenders can only take place between two empires who are currently at war and who have "\
		"never been allied in the course of the game."\
		"</td></tr>"\
		
		"<tr>"\
		"<td><strong>Classic SC-style Surrenders (Classic)</strong></td>"\
		"<td>Surrendering empires leave the game immediately, but when their homeworlds are colonized, "\
		"the colonizers are considered to have nuked the empire"\
		"</td></tr>"\
		
		"<tr>"\
		"<td><strong>Permanent Doomsdays (PermanentDooms)</strong></td>"\
		"<td>Doomsdays behave as in classic Stellar Crisis: annihilated planets remain in quarantine forever,"\
		" and never revert to colonizable status."\
		"</td></tr>"\

		"<tr>"\
		"<td><strong>Unfriendly Doomsdays (UnfriendlyDooms)</strong></td>"\
		"<td>Doomsdays can annihilate planets belonging to non-warring empires, except for their"\
		" homeworlds."\
		"</td></tr>"\

		"<tr>"\
		"<td><strong>Suicidal Doomsdays (SuicidalDooms)</strong></td>"\
		"<td>Doomsdays can annihilate planets belonging to their owner, except for their own homeworld."\
		"</td></tr>"\

		"<tr>"\
		"<td><strong>Friendly Gates (FriendlyGates)</strong></td>"\
		"<td>Stargates and jumpgates can gate ships belonging to allied empires"\
		"</td></tr>"\
		
		"<tr>"\
		"<td><strong>Independence (Independence)</strong></td>"\
		"<td>When an empire is nuked, its territories remain alive and populated as independent planets "\
		"that cannot simply be re-colonized by other empires. Similarly, the empire's ships remain alive, and "\
		"either remain independent or revert to belonging to the owner of the planet at which they were located</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Subjective Views (Subjective)</strong></td>"\
		"<td>The empire econ and mil totals displayed in the diplomacy screen for each empire represent the "\
		"portion of the empire's resources that can be seen by the viewing empire on his or her map."\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Simple Ruins (SimpleRuins)</strong></td>"\
		"<td>Games with simple ruins enabled will behave just like classic Stellar Crisis: if an empire "\
		"has been idle for a gameclass-specific number of updates, then it will fall into ruin and be "\
		"removed from the game"\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td><strong>Complex Ruins (ComplexRuins)</strong></td>"\
		"<td>Games with complex ruins enabled function as follows: if all empires or all empires except one "\
		"are idle for a gameclass-specific number of updates, then the idle empires will fall into ruin, but "\
		"only if more than two empires were in the game at some point during its lifetime."\
		"</td>"\
		"</tr>"\
		
		"</table>"
		
		);
		
		return;
		
ErrorExit:
		
		OutputText ("<p><strong>Error ");
		m_pHttpResponse->WriteText (iErrCode);
		OutputText (" occurred reading server configuration data</strong>");
}

void HtmlRenderer::WriteFaq() {
	
	OutputText ("<p>An easy to print FAQ can be found <a href=\"" BASE_RESOURCE_DIR FAQ_FILE "\">here</a>.<p>");
	
	char pszFileName[OS::MaxFileNameLength];
	sprintf (pszFileName, "%s/" FAQ_FILE, g_pszResourceDir);
	
	ICachedFile* pcfFaq = g_pFileCache->GetFile (pszFileName);
	if (pcfFaq == NULL) {
		OutputText ("<p><strong>The FAQ file could not be found; please alert your system administrator</strong>");
	} else {
		OutputText ("<p></center>");
		m_pHttpResponse->WriteTextFile (pcfFaq);
		pcfFaq->Release();
		OutputText ("<center>");
	}
}

void HtmlRenderer::WriteServerNews () {
	
	UTCTime tTime;
	
	Time::GetTime (&tTime);
	
	int iDay  = Time::GetDay (tTime);
	int iYear = Time::GetYear (tTime);
	
	const char* pszMonth = Time::GetMonthName (Time::GetMonth (tTime));
	const char* pszDayW  = Time::GetDayOfWeek (tTime);
	
	OutputText ("<p><table><tr><th bgcolor=\"#");
	m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
	OutputText ("\">Today is ");
	m_pHttpResponse->WriteText (pszDayW);
	OutputText (", ");
	m_pHttpResponse->WriteText (pszMonth);
	OutputText (" ");
	m_pHttpResponse->WriteText (iDay);
	OutputText (", ");
	m_pHttpResponse->WriteText (iYear);
	OutputText ("</th></tr></table><p>");
	
	WriteServerNewsFile();
}

void HtmlRenderer::WriteIntro() {
	
	char pszFileName[OS::MaxFileNameLength];
	sprintf (pszFileName, "%s/" INTRO_FILE, g_pszResourceDir);
	
	ICachedFile* pcfIntro = g_pFileCache->GetFile (pszFileName);
	if (pcfIntro == NULL) {
		OutputText ("<p><strong>The intro file could not be found; please alert your system administrator</strong>");
	} else {
		m_pHttpResponse->WriteTextFile (pcfIntro);
		pcfIntro->Release();
	}
}

void HtmlRenderer::WriteIntroUpper() {
	
	char pszFileName[OS::MaxFileNameLength];
	sprintf (pszFileName, "%s/" INTRO_UPPER_FILE, g_pszResourceDir);
	
	m_mIntroUpperFileLock.WaitReader();
	
	ICachedFile* pcfIntro = g_pFileCache->GetFile (pszFileName);
	if (pcfIntro != NULL) {
		m_pHttpResponse->WriteTextFile (pcfIntro);
		pcfIntro->Release();
	}
	
	m_mIntroUpperFileLock.SignalReader();
}

void HtmlRenderer::WriteIntroLower() {
	
	char pszFileName[OS::MaxFileNameLength];
	sprintf (pszFileName, "%s/" INTRO_LOWER_FILE, g_pszResourceDir);
	
	m_mIntroLowerFileLock.WaitReader();
	
	ICachedFile* pcfIntro = g_pFileCache->GetFile (pszFileName);
	if (pcfIntro != NULL) {
		m_pHttpResponse->WriteTextFile (pcfIntro);
		pcfIntro->Release();
	}
	
	m_mIntroLowerFileLock.SignalReader();
}

void HtmlRenderer::WriteServerNewsFile() {
	
	char pszFileName[OS::MaxFileNameLength];
	sprintf (pszFileName, "%s/" NEWS_FILE, g_pszResourceDir);
	
	m_mNewsFileLock.WaitReader();
	
	ICachedFile* pcfCachedFile = g_pFileCache->GetFile (pszFileName);
	
	if (pcfCachedFile != NULL) {
		m_pHttpResponse->WriteTextFile (pcfCachedFile);
		pcfCachedFile->Release();
	}
	
	m_mNewsFileLock.SignalReader();
}


bool HtmlRenderer::UpdateIntroUpper (const char* pszText) {
	
	char pszFileName[OS::MaxFileNameLength];
	sprintf (pszFileName, "%s/" INTRO_UPPER_FILE, g_pszResourceDir);
	
	m_mIntroUpperFileLock.WaitWriter();
	
	bool bRetVal = UpdateCachedFile (pszFileName, pszText);
	
	m_mIntroUpperFileLock.SignalWriter();
	
	return bRetVal;
}

bool HtmlRenderer::UpdateIntroLower (const char* pszText) {
	
	char pszFileName[OS::MaxFileNameLength];
	sprintf (pszFileName, "%s/" INTRO_LOWER_FILE, g_pszResourceDir);
	
	m_mIntroLowerFileLock.WaitWriter();
	
	bool bRetVal = UpdateCachedFile (pszFileName, pszText);
	
	m_mIntroLowerFileLock.SignalWriter();
	
	return bRetVal;
}

bool HtmlRenderer::UpdateServerNews (const char* pszText) {
	
	char pszFileName[OS::MaxFileNameLength];
	sprintf (pszFileName, "%s/" NEWS_FILE, g_pszResourceDir);
	
	m_mNewsFileLock.WaitWriter();
	
	bool bRetVal = UpdateCachedFile (pszFileName, pszText);
	
	m_mNewsFileLock.SignalWriter();
	
	return bRetVal;
}

bool HtmlRenderer::UpdateCachedFile (const char* pszFileName, const char* pszText) {
	
	size_t stSize;
	
	ICachedFile* pcfCachedFile = g_pFileCache->GetFile (pszFileName);
	if (pcfCachedFile == NULL || (stSize = pcfCachedFile->GetSize()) == 0) {
		
		if (pszText == NULL || *pszText == '\0') {
			// No change
			return false;
		}
		
		// Null file: write away
		File fCachedFile;
		if (fCachedFile.OpenWrite (pszFileName) != OK) {
			Assert (false);
			return false;
		}
		
		fCachedFile.Write (pszText);
		fCachedFile.Close();
		
		return true;
		
	} else {
		
		// Are we changing?
		const char* pszData = (char*) pcfCachedFile->GetData();
		
		size_t stNewSize = String::StrLen (pszText);
		
		if (stSize != stNewSize || strncmp (pszText, pszData, stNewSize) != 0) {
			
			// File is changing, so get the filecache to release it
			g_pFileCache->ReleaseFile (pszFileName);
			pcfCachedFile->Release();
			
			// Now, replace the file
			File fCachedFile;
			if (fCachedFile.OpenWrite (pszFileName) != OK) {
				Assert (false);
				return false;
			}
			
			fCachedFile.Write (pszText == NULL ? "" : pszText);
			fCachedFile.Close();
			
			return true;
			
		} else {
			
			// Just release the file
			pcfCachedFile->Release();
			
			return false;
		}
	}
}

void HtmlRenderer::WriteNukeHistory (int iTargetEmpireKey) {
	
	Variant vNukeEmpireName;
	
	Variant** ppvNukedData, ** ppvNukerData;
	int iNumNuked, iNumNukers, i;	
	
	if (g_pGameEngine->GetEmpireName (iTargetEmpireKey, &vNukeEmpireName) != OK || 
		g_pGameEngine->GetNukeHistory (iTargetEmpireKey, &iNumNuked, &ppvNukedData, &iNumNukers, 
		&ppvNukerData) != OK) {
		
		OutputText ("<p><strong>The empire's nuke history could not be read</strong>");
		
	} else {
		
		OutputText ("<input type=\"hidden\" name=\"NumNukerEmps\" value=\"");
		m_pHttpResponse->WriteText (iNumNukers);
		OutputText ("\"><input type=\"hidden\" name=\"NumNukedEmps\" value=\"");
		m_pHttpResponse->WriteText (iNumNuked);
		OutputText ("\">");
		
		if (iNumNuked == 0 && iNumNukers == 0) { 
			OutputText ("<p><strong>This empire has no nuke history</strong>");
		} else {
			
			NotifyProfileLink();
			
			char pszDateString [OS::MaxDateLength];
			
			int iErrCode, iAlloc = max (iNumNukers, iNumNuked);
			
			UTCTime* ptTime = (UTCTime*) StackAlloc (iAlloc * sizeof (UTCTime));
			Variant** ppvData = (Variant**) StackAlloc (iAlloc * sizeof (Variant*));
			
			if (iNumNuked > 0) {
				
				OutputText ("<p><h3>Empires who were nuked by ");
				m_pHttpResponse->WriteText (vNukeEmpireName.GetCharPtr());
				OutputText (":</h3><p><table width=\"90%\"><tr><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
				OutputText ("\" align=\"center\">Empire</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr()); 
				OutputText ("\" align=\"center\">Icon</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\" align=\"center\">Game</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\" align=\"center\">Time</th></tr>");
				
				// Sort by timestamp
				for (i = 0; i < iNumNuked; i ++) {
					ptTime[i] = ppvNukedData[i][SystemEmpireNukeList::TimeStamp].GetUTCTime();
					ppvData[i] = ppvNukedData[i];
				}
				
				Algorithm::QSortTwoAscending<UTCTime, Variant*> (ptTime, ppvData, iNumNuked);
				
				char pszEmpire [256 + MAX_EMPIRE_NAME_LENGTH];
				
				for (i = 0; i < iNumNuked; i ++) {
					
					OutputText ("<tr><td align=\"center\"><strong>");
					
					m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr()); 
					OutputText ("</strong></td><td align=\"center\">");
					
					sprintf (
						pszEmpire, 
						"View the profile of %s",
						ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr()
						);
					
					WriteProfileAlienString (
						ppvData[i][SystemEmpireNukeList::AlienKey].GetInteger(), 
						ppvData[i][SystemEmpireNukeList::EmpireKey].GetInteger(),
						ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr(),
						0,
						"ProfileLink",
						pszEmpire,
						true,
						true
						);
					
					OutputText ("</td><td align=\"center\">");
					m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::GameClassName].GetCharPtr());
					OutputText (" ");
					m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::GameNumber].GetInteger());
					OutputText ("</td><td align=\"center\">");
					
					iErrCode = Time::GetDateString (
						ppvData[i][SystemEmpireNukeList::TimeStamp].GetUTCTime(), 
						pszDateString
						);
					
					if (iErrCode == OK) {
						m_pHttpResponse->WriteText (pszDateString);
					} else {
						OutputText ("Could not read date string");
					}
					
					OutputText ("</td></tr>");
				}
				
				g_pGameEngine->FreeData (ppvNukedData);
				
				OutputText ("</table>");
			}
			
			if (iNumNukers > 0) {
				
				OutputText ("<p><h3>Empires who nuked ");
				m_pHttpResponse->WriteText (vNukeEmpireName.GetCharPtr());
				
				OutputText (":</h3><p><table width=\"90%\"><tr><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\" align=\"center\">Empire</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\" align=\"center\">Icon</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\" align=\"center\">Game</th><th bgcolor=\"");
				m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
				OutputText ("\" align=\"center\">Time</th></tr>");
				
				// Sort by timestamp
				for (i = 0; i < iNumNukers; i ++) {
					ptTime[i] = ppvNukerData[i][SystemEmpireNukeList::TimeStamp].GetUTCTime();
					ppvData[i] = ppvNukerData[i];
				}
				Algorithm::QSortTwoAscending<UTCTime, Variant*> (ptTime, ppvData, iNumNukers);
				
				char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
				
				for (i = 0; i < iNumNukers; i ++) {
					
					OutputText ("<tr><td align=\"center\"><strong>");
					m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr()); 
					OutputText ("</strong></td><td align=\"center\">");
					
					sprintf (pszProfile, "View the profile of %s", ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr());
					
					WriteProfileAlienString (
						ppvData[i][SystemEmpireNukeList::AlienKey].GetInteger(), 
						ppvData[i][SystemEmpireNukeList::EmpireKey].GetInteger(),
						ppvData[i][SystemEmpireNukeList::EmpireName].GetCharPtr(),
						0,
						"ProfileLink",
						pszProfile,
						true,
						true
						);
					
					OutputText ("</td><td align=\"center\">");
					m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::GameClassName].GetCharPtr());
					OutputText (" ");
					m_pHttpResponse->WriteText (ppvData[i][SystemEmpireNukeList::GameNumber].GetInteger());
					OutputText ("</td><td align=\"center\">");
					
					iErrCode = Time::GetDateString (
						ppvData[i][SystemEmpireNukeList::TimeStamp].GetUTCTime(), 
						pszDateString
						);
					
					if (iErrCode == OK) {
						m_pHttpResponse->WriteText (pszDateString);
					} else {
						OutputText ("Could not read date string");
					}
					
					OutputText ("</td></tr>");
				}
				
				g_pGameEngine->FreeData (ppvNukerData);
				
				OutputText ("</table>");
			}
		}
	}
}

void HtmlRenderer::WritePersonalGameClasses (int iTargetEmpireKey) {
	
	int iMaxNumPGC, iNumGameClasses, i;
	int* piGameClassKey;
	
	int iErrCode = g_pGameEngine->GetMaxNumPersonalGameClasses (&iMaxNumPGC);
	int iErrCode1 = g_pGameEngine->GetEmpireGameClassKeys (iTargetEmpireKey, &piGameClassKey, &iNumGameClasses);
	
	if (iErrCode != OK || iErrCode1 != OK || iNumGameClasses == 0) {
		OutputText ("<p><strong>There are no Personal GameClasses available</strong>");
	} else {
		
		int iGameClass, iInitTechs, iDevTechs;
		Variant* pvGameClassInfo = NULL;
		
		bool bDraw = false;
		
		for (i = 0; i < iNumGameClasses; i ++) {
			
			iGameClass = piGameClassKey[i];
			
			if (!bDraw) {
				bDraw = true;
				OutputText ("<p><h3>Start a new game:</h3>");
				WriteSystemGameListHeader (m_vTableColor);
			}
			
			// Read game class data
			if (g_pGameEngine->GetGameClassData (iGameClass, &pvGameClassInfo) == OK &&
				g_pGameEngine->GetDevelopableTechs (iGameClass, &iInitTechs, &iDevTechs) == OK) {
				
				// Best effort
				iErrCode = WriteSystemGameListData (iGameClass, pvGameClassInfo);
			}
			
			if (pvGameClassInfo != NULL) {
				g_pGameEngine->FreeData (pvGameClassInfo);
				pvGameClassInfo = NULL;
			}
		}
		
		if (!bDraw) {
			OutputText ("<p><strong>There are no Personal GameClasses available to your empire</strong>");
		} else {
			OutputText ("</table>");
		}
		
		g_pGameEngine->FreeKeys (piGameClassKey);
	}
}

void HtmlRenderer::WriteGameAdministratorGameData (const char* pszGameClassName, 
												   int iGameNumber, Seconds iSeconds, Seconds iSecondsUntil, 
												   int iNumUpdates, bool bOpen, bool bPaused, bool bAdminPaused, 
												   bool bStarted, const char* pszGamePassword, 
												   Variant* pvEmpireName, int iNumActiveEmpires, 
												   const UTCTime& tCreationTime) {
	
	int i;
	Variant vName;
	
	int iErrCode;
	
	OutputText ("<tr><td align=\"left\">");
	m_pHttpResponse->WriteText (pszGameClassName);
	OutputText (" <strong>");
	m_pHttpResponse->WriteText (iGameNumber);
	
	OutputText ("</strong></td><td align=\"center\">");
	
	WriteTime (iSeconds);
	
	OutputText ("</td><td align=\"center\">");
	m_pHttpResponse->WriteText (iNumUpdates);
	
	OutputText ("</td><td align=\"center\">");
	
	char pszDateString [OS::MaxDateLength];
	iErrCode = Time::GetDateString (tCreationTime, pszDateString);
	
	if (iErrCode == OK) {
		m_pHttpResponse->WriteText (pszDateString);
	} else {
		OutputText ("Could not read date string");
	}
	
	OutputText ("</td><td align=\"center\">");
	
	if (bStarted) {
		WriteTime (iSecondsUntil);
	} else {
		OutputText ("-");
	}
	
	OutputText ("</td><td align=\"center\">");
	
	if (bOpen) {
		OutputText ("Open");
		if (bPaused) {
			if (!bAdminPaused) {
				OutputText (" (paused)");
			} else {
				OutputText (" (paused by an admin)");
			}
		} else {
			if (bStarted) {
				OutputText (" (started) ");
			} else {
				OutputText (" (not started) ");
			}
		}
	} else {
		OutputText ("Closed");
		if (bPaused) {
			if (!bAdminPaused) {
				OutputText (" (paused)");
			} else {
				OutputText (" (paused by an admin)");
			}
		}
	}
	
	OutputText ("</td><td align=\"center\">");
	if (!String::IsBlank (pszGamePassword)) {
		
		String strFilter;
		int iErrCode = HTMLFilter (pszGamePassword, &strFilter, 0, false);
		if (iErrCode != OK) {
			OutputText ("The server is out of memory");
		} else {
			m_pHttpResponse->WriteText (strFilter, strFilter.GetLength());
		}
	}
	OutputText ("</td><td align=\"center\"><strong>");
	m_pHttpResponse->WriteText (iNumActiveEmpires);
	OutputText ("</strong> (");
	
	Assert (iNumActiveEmpires > 0);
	
	for (i = 0; i < iNumActiveEmpires - 1; i ++) {
		m_pHttpResponse->WriteText (pvEmpireName[i].GetCharPtr());
		OutputText (", ");
	}
	m_pHttpResponse->WriteText (pvEmpireName[i].GetCharPtr());
	
	OutputText (")</td>");
}


int HtmlRenderer::RenderThemeInfo (int iBackgroundKey, int iLivePlanetKey, int iDeadPlanetKey, int iSeparatorKey,
								   int iButtonKey, int iHorzKey, int iVertKey, int iColorKey) {
	
	const int piUIKey[] = {
		iBackgroundKey,
			iColorKey,
			iLivePlanetKey, 
			iDeadPlanetKey,
			iButtonKey,
			iSeparatorKey, 
			iHorzKey, 
			iVertKey,
	};
	
	const char* ppszName[] = {
		"Name",
			"Background",
			"Text Colors",
			"Live Planet",
			"Dead Planet",
			"Buttons",
			"Separator",
			"Horizontal Link",
			"Vertical Link"
	};
	
	const char* ppszFormName[] = {
		"Background",
			"Color",
			"LivePlanet",
			"DeadPlanet",
			"Button",
			"Separator",
			"Horz",
			"Vert"
	};
	
	const int piThemeBitField[] = {
		THEME_BACKGROUND,
			ALL_THEME_OPTIONS,
			THEME_LIVE_PLANET,
			THEME_DEAD_PLANET,
			THEME_BUTTONS,
			THEME_SEPARATOR,
			THEME_HORZ,
			THEME_VERT
	};
	
	const char* pszTableColor = m_vTableColor.GetCharPtr();
	
	int* piThemeKey, iNumThemes, i, j;
	int iErrCode = g_pGameEngine->GetThemeKeys (&piThemeKey, &iNumThemes);
	if (iErrCode != OK || iNumThemes == 0) {
		return iErrCode;
	}
	
	String strHorz, strVert;
	
	iErrCode = GetHorzString (NULL_THEME, &strHorz);
	if (iErrCode != OK) {
		return iErrCode;
	}
	
	iErrCode = GetVertString (NULL_THEME, &strVert);
	if (iErrCode != OK) {
		return iErrCode;
	}
	
	OutputText ("<table><tr>");
	for (i = 0; i < sizeof (ppszName) / sizeof (const char*); i ++) {
		
		OutputText ("<th bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\" align=\"center\">");
		m_pHttpResponse->WriteText (ppszName[i]);
		OutputText (":</th>");
	}
	OutputText ("</tr>");
	
	// Null Theme
	OutputText ("<tr>"\
		"<td>Null Theme</td>"\
		"<td bgcolor=\""
		);
	m_pHttpResponse->WriteText (DEFAULT_BG_COLOR);
	OutputText ("\" width=\"120\" height=\"120\">&nbsp;</td>");
	
	// Text Color
	OutputText (
		
		"<td align=\"center\"><table width=\"100\" height=\"100\" cellspacing=\"0\" bgcolor=\"#" DEFAULT_BG_COLOR "\">"\
		
		// Text
		
		"<tr><td align=\"center\"><font color=\"#" DEFAULT_TEXT_COLOR "\">Text</font></td></tr>"\
		
		// Good Color
		
		"<tr><td align=\"center\"><font color=\"#" DEFAULT_GOOD_COLOR "\">Good</font></td></tr>"\
		
		// Bad Color
		
		"<tr><td align=\"center\"><font color=\"#" DEFAULT_BAD_COLOR "\">Bad</font></td></tr>"\
		
		// Private Color
		
		"<tr><td align=\"center\"><font color=\"#" DEFAULT_PRIVATE_MESSAGE_COLOR "\">Private</font></td></tr>"\
		
		// Broadcast Color
		
		"<tr><td align=\"center\"><font color=\"#" DEFAULT_BROADCAST_MESSAGE_COLOR "\">Broadcast</font></td></tr>"\
		
		// Table Color
		
		"<tr><th align=\"center\" bgcolor=\"" DEFAULT_TABLE_COLOR "\"><font color=\"#" DEFAULT_TEXT_COLOR "\">Table</font></th></tr>"\
		
		"</table></td>"
		);
	
	// Live Planet
	OutputText ("<td align=\"center\"><img src=\"");
	WriteLivePlanetImageSrc (NULL_THEME);
	
	OutputText ("\"></td><td align=\"center\"><img src=\"");
	WriteDeadPlanetImageSrc (NULL_THEME);
	
	OutputText ("\"></td><td align=\"center\">");
	WriteButtonString (NULL_THEME, "Login", "Login", "Login");
	
	OutputText ("</td><td align=\"center\">");
	
	WriteSeparatorString (NULL_THEME);
	
	OutputText ("</td><td align=\"center\">");
	m_pHttpResponse->WriteText (strHorz);
	
	OutputText ("</td><td align=\"center\">");
	m_pHttpResponse->WriteText (strVert);
	OutputText ("</td>");
	
	OutputText ("</tr><tr><td>&nbsp;</td>");
	
	for (i = 0; i < sizeof (ppszFormName) / sizeof (const char*); i ++) {
		
		OutputText ("<td bgcolor=\"");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\" align=\"center\"><input");
		
		if (piUIKey[i] == NULL_THEME) {
			OutputText (" checked");
		}
		
		OutputText (" type=\"radio\" name=\"");
		m_pHttpResponse->WriteText (ppszFormName[i]);
		OutputText ("\" value=\"");
		m_pHttpResponse->WriteText (NULL_THEME);
		OutputText ("\"></td>");
	}
	
	int iOptions;
	Variant* pvThemeData;
	
	char pszForm [128];
	
	for (i = 0; i < iNumThemes; i ++) {
		
		iErrCode = g_pGameEngine->GetThemeData (piThemeKey[i], &pvThemeData);
		if (iErrCode != OK) {
			return iErrCode;
		}
		
		OutputText ("<tr><td width=\"10%\" align=\"center\">");
		m_pHttpResponse->WriteText (pvThemeData[SystemThemes::Name].GetCharPtr());
		OutputText ("<p>");
		
		sprintf (pszForm, "ThemeInfo%i", piThemeKey[i]);
		WriteButtonString (m_iButtonKey, "Info", "Theme Info", pszForm); 
		
		OutputText ("</td>");
		
		iOptions = pvThemeData[SystemThemes::Options].GetInteger();
		
		// Background
		if (iOptions & THEME_BACKGROUND) {
			
			OutputText ("<td align=\"center\"><img width=\"120\" height=\"120\" src=\"");
			WriteBackgroundImageSrc (piThemeKey[i]);
			OutputText ("\"></td>");
		} else {
			OutputText ("<td>&nbsp;</td>");
		}
		
		// Text colors
		OutputText ("<td align=\"center\"><table width=\"100\" height=\"100\" cellspacing=\"0\"");
		
		if (iOptions & THEME_BACKGROUND) {
			OutputText (" background=\"");
			WriteBackgroundImageSrc (piThemeKey[i]);
			OutputText ("\"");
		}
		OutputText (">"\
			
			// Text
			"<tr><td align=\"center\"><font color=\"#");
		
		m_pHttpResponse->WriteText (pvThemeData[SystemThemes::TextColor].GetCharPtr());
		OutputText ("\">Text</font></td></tr>"\
			
			// Good Color
			"<tr><td align=\"center\"><font color=\"#");
		m_pHttpResponse->WriteText (pvThemeData[SystemThemes::GoodColor].GetCharPtr());
		OutputText ("\">Good</font></td></tr>"\
			
			// Bad Color
			
			"<tr><td align=\"center\"><font color=\"#");
		m_pHttpResponse->WriteText (pvThemeData[SystemThemes::BadColor].GetCharPtr());
		OutputText ("\">Bad</font></td></tr>"\
			
			// Private Color
			
			"<tr><td align=\"center\"><font color=\"#");
		m_pHttpResponse->WriteText (pvThemeData[SystemThemes::PrivateMessageColor].GetCharPtr());
		OutputText ("\">Private</font></td></tr>"\
			
			// Broadcast Color
			
			"<tr><td align=\"center\"><font color=\"#");
		m_pHttpResponse->WriteText (pvThemeData[SystemThemes::BroadcastMessageColor].GetCharPtr());
		OutputText ("\">Broadcast</font></td></tr>"\
			
			// Table Color
			
			"<tr><th align=\"center\" bgcolor=\"");
		m_pHttpResponse->WriteText (pvThemeData[SystemThemes::TableColor].GetCharPtr());
		OutputText ("\"><font color=\"#");
		m_pHttpResponse->WriteText (pvThemeData[SystemThemes::TextColor].GetCharPtr());
		OutputText ("\">Table</font></th></tr>"\
			
			"</table></td>"
			);
		
		// Live Planet
		if (iOptions & THEME_LIVE_PLANET) {
			OutputText ("<td align=\"center\"><img src=\"");
			WriteLivePlanetImageSrc (piThemeKey[i]);
			OutputText ("\"></td>");
		} else {
			OutputText ("<td>&nbsp;</td>");
		}
		
		if (iOptions & THEME_DEAD_PLANET) {
			OutputText ("<td align=\"center\"><img src=\"");
			WriteDeadPlanetImageSrc (piThemeKey[i]);
			OutputText ("\"></td>");
		} else {
			OutputText ("<td>&nbsp;</td>");
		}
		
		if (iOptions & THEME_BUTTONS) {
			OutputText ("<td align=\"center\"><img src=\"");
			WriteButtonImageSrc (piThemeKey[i], "Login");
			OutputText ("\"></td>");
		} else {
			OutputText ("<td>&nbsp;</td>");
		}
		
		if (iOptions & THEME_SEPARATOR) {
			OutputText ("<td align=\"center\"><img width=\"200\" src=\"");
			WriteSeparatorSrc (piThemeKey[i]);
			OutputText ("\"></td>");
		} else {
			OutputText ("<td>&nbsp;</td>");
		}
		
		if (iOptions & THEME_HORZ) {
			OutputText ("<td align=\"center\"><img src=\"");
			WriteHorzSrc (piThemeKey[i]);
			OutputText ("\"></td>");
		} else {
			OutputText ("<td>&nbsp;</td>");
		}
		
		if (iOptions & THEME_VERT) {
			OutputText ("<td align=\"center\"><img src=\"");
			WriteVertSrc (piThemeKey[i]);
			OutputText ("\"></td>");
		} else {
			OutputText ("<td>&nbsp;</td>");
		}
		
		OutputText ("</tr><tr><td>&nbsp;</td>");
		
		for (j = 0; j < sizeof (piThemeBitField) / sizeof (int); j ++) {
			
			if (iOptions & piThemeBitField[j]) {
				
				OutputText ("<td bgcolor=\"");
				m_pHttpResponse->WriteText (pszTableColor);
				OutputText ("\" align=\"center\"><input");
				
				if (piUIKey[j] == piThemeKey[i]) {
					OutputText (" checked");
				}
				
				OutputText (" type=\"radio\" name=\"");
				m_pHttpResponse->WriteText (ppszFormName[j]);
				OutputText ("\" value=\"");
				m_pHttpResponse->WriteText (piThemeKey[i]);
				OutputText ("\"></td>");
				
			} else { 
				OutputText ("<td>&nbsp;</td>");
			}
		}
		
		OutputText ("</tr>");
		
		g_pGameEngine->FreeData (pvThemeData);
	}
	
	g_pGameEngine->FreeKeys (piThemeKey);
	
	return OK;
}

int HtmlRenderer::DisplayThemeData (int iThemeKey) {
	
	Variant* pvThemeData;
	int iErrCode = g_pGameEngine->GetThemeData (iThemeKey, &pvThemeData);
	if (iErrCode != OK) {
		return iErrCode;
	}
	
	OutputText ("<p><table width=\"50%\"><tr><td>Theme:</td><td>");
	m_pHttpResponse->WriteText (pvThemeData[SystemThemes::Name].GetCharPtr());
	
	OutputText ("</td></tr><tr><td>Version:</td><td>");
	m_pHttpResponse->WriteText (pvThemeData[SystemThemes::Version].GetCharPtr());
	
	OutputText ("</tr><tr><td>Author:</td><td>");
	m_pHttpResponse->WriteText (pvThemeData[SystemThemes::AuthorName].GetCharPtr());
	
	OutputText ("</td></tr><tr><td>Email:</td><td>");
	m_pHttpResponse->WriteText (pvThemeData[SystemThemes::AuthorEmail].GetCharPtr());
	
	OutputText ("</td></tr><tr><td>Description:</td><td>");
	m_pHttpResponse->WriteText (pvThemeData[SystemThemes::Description].GetCharPtr());
	
	OutputText ("</td></tr><tr><td>Download:</td><td><a href=\"");
	
	WriteThemeDownloadSrc (iThemeKey, pvThemeData[SystemThemes::FileName].GetCharPtr());
	
	OutputText ("\">");
	
	m_pHttpResponse->WriteText (pvThemeData[SystemThemes::FileName].GetCharPtr());
	
	OutputText ("</a></td></tr></table><p>");
	
	WriteButton (BID_CANCEL);
	
	g_pGameEngine->FreeData (pvThemeData);
	
	return iErrCode;
}

int HtmlRenderer::HandleSearchSubmission (unsigned int* piSearchColName, 
										  Variant* pvSearchColData1,
										  Variant* pvSearchColData2,
										  const char** pszFormName,
										  const char** pszColName1,
										  const char** pszColName2,
										  
										  int* piNumSearchColumns,
										  
										  int** ppiSearchEmpireKey,
										  int* piNumSearchEmpires,
										  int* piLastKey,
										  int* piMaxNumHits
										  ) {
	const char* pszString = NULL;
	int iValue = -1, iValue2 = -1;
	float fValue, fValue2;
	int64 i64Value = -1, i64Value2 = -1;
	
	int iErrCode = OK;
	
	int iNumSearchColumns = 0, iSkip = 0, iStartKey = 0;
	
	IHttpForm* pHttpForm;
	
	int iMaxNumHits = 0;
	
	if (m_pHttpRequest->GetForm ("EmpireName") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TEmpireName")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		pszString = pHttpForm->GetValue();
		
		if (pszString != NULL && *pszString != '\0') {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("SEmpireName")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			iValue = pHttpForm->GetIntValue();
			
			piSearchColName [iNumSearchColumns] = SystemEmpireData::Name;
			pvSearchColData1 [iNumSearchColumns] = pszString;
			pvSearchColData2 [iNumSearchColumns] = iValue;
			pszFormName [iNumSearchColumns] = "EmpireName";
			pszColName1 [iNumSearchColumns] = "TEmpireName";
			pszColName2 [iNumSearchColumns] = "SEmpireName";
			iNumSearchColumns ++;
		}
	}
	
	if (m_pHttpRequest->GetForm ("EmpireKeyF") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TEmpireKey1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TEmpireKey2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = NO_KEY;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "EmpireKeyF";
		pszColName1 [iNumSearchColumns] = "TEmpireKey1";
		pszColName2 [iNumSearchColumns] = "TEmpireKey2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("RealName") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TRealName")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		pszString = pHttpForm->GetValue();
		
		if (pszString != NULL && *pszString != '\0') {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("SRealName")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			iValue = pHttpForm->GetIntValue();
			
			piSearchColName [iNumSearchColumns] = SystemEmpireData::RealName;
			pvSearchColData1 [iNumSearchColumns] = pszString;
			pvSearchColData2 [iNumSearchColumns] = iValue;
			pszFormName [iNumSearchColumns] = "RealName";
			pszColName1 [iNumSearchColumns] = "TRealName";
			pszColName2 [iNumSearchColumns] = "SRealName";
			iNumSearchColumns ++;
		}
	}
	
	if (m_pHttpRequest->GetForm ("Email") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TEmail")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		pszString = pHttpForm->GetValue();
		
		if (pszString != NULL && *pszString != '\0') {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("SEmail")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			iValue = pHttpForm->GetIntValue();
			
			piSearchColName [iNumSearchColumns] = SystemEmpireData::Email;
			pvSearchColData1 [iNumSearchColumns] = pszString;
			pvSearchColData2 [iNumSearchColumns] = iValue;
			pszFormName [iNumSearchColumns] = "Email";
			pszColName1 [iNumSearchColumns] = "TEmail";
			pszColName2 [iNumSearchColumns] = "SEmail";
			iNumSearchColumns ++;
		}
	}
	
	if (m_pHttpRequest->GetForm ("WebPage") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TWebPage")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		pszString = pHttpForm->GetValue();
		
		if (pszString != NULL && *pszString != '\0') {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("SWebPage")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			iValue = pHttpForm->GetIntValue();
			
			piSearchColName [iNumSearchColumns] = SystemEmpireData::WebPage;
			pvSearchColData1 [iNumSearchColumns] = pszString;
			pvSearchColData2 [iNumSearchColumns] = iValue;
			pszFormName [iNumSearchColumns] = "WebPage";
			pszColName1 [iNumSearchColumns] = "TWebPage";
			pszColName2 [iNumSearchColumns] = "SWebPage";
			iNumSearchColumns ++;
		}
	}
	
	if (m_pHttpRequest->GetForm ("Quote") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TQuote")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		pszString = pHttpForm->GetValue();
		
		if (pszString != NULL && *pszString != '\0') {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("SQuote")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			iValue = pHttpForm->GetIntValue();
			
			piSearchColName [iNumSearchColumns] = SystemEmpireData::Quote;
			pvSearchColData1 [iNumSearchColumns] = pszString;
			pvSearchColData2 [iNumSearchColumns] = iValue;
			pszFormName [iNumSearchColumns] = "Quote";
			pszColName1 [iNumSearchColumns] = "TQuote";
			pszColName2 [iNumSearchColumns] = "SQuote";
			iNumSearchColumns ++;
		}
	}
	
	if (m_pHttpRequest->GetForm ("Sneer") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TSneer")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		pszString = pHttpForm->GetValue();
		
		if (pszString != NULL && *pszString != '\0') {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("SSneer")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			iValue = pHttpForm->GetIntValue();
			
			piSearchColName [iNumSearchColumns] = SystemEmpireData::VictorySneer;
			pvSearchColData1 [iNumSearchColumns] = pszString;
			pvSearchColData2 [iNumSearchColumns] = iValue;
			pszFormName [iNumSearchColumns] = "Sneer";
			pszColName1 [iNumSearchColumns] = "TSneer";
			pszColName2 [iNumSearchColumns] = "SSneer";
			iNumSearchColumns ++;
		}
	}
	
	if (m_iPrivilege >= ADMINISTRATOR) {
		
		if (m_pHttpRequest->GetForm ("IPAddress") != NULL) {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("TIPAddress")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			pszString = pHttpForm->GetValue();
			
			if (pszString != NULL && *pszString != '\0') {
				
				if ((pHttpForm = m_pHttpRequest->GetForm ("SIPAddress")) == NULL) {
					return ERROR_INVALID_ARGUMENT;
				}
				iValue = pHttpForm->GetIntValue();
				
				piSearchColName [iNumSearchColumns] = SystemEmpireData::IPAddress;
				pvSearchColData1 [iNumSearchColumns] = pszString;
				pvSearchColData2 [iNumSearchColumns] = iValue;
				pszFormName [iNumSearchColumns] = "IPAddress";
				pszColName1 [iNumSearchColumns] = "TIPAddress";
				pszColName2 [iNumSearchColumns] = "SIPAddress";
				iNumSearchColumns ++;
			}
		}
		
		if (m_pHttpRequest->GetForm ("SessionId") != NULL) {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("TSessionId1")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			i64Value = pHttpForm->GetInt64Value();
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("TSessionId2")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			i64Value2 = pHttpForm->GetInt64Value();
			
			piSearchColName [iNumSearchColumns] = SystemEmpireData::SessionId;
			pvSearchColData1 [iNumSearchColumns] = i64Value;
			pvSearchColData2 [iNumSearchColumns] = i64Value2;
			pszFormName [iNumSearchColumns] = "SessionId";
			pszColName1 [iNumSearchColumns] = "TSessionId1";
			pszColName2 [iNumSearchColumns] = "TSessionId2";
			iNumSearchColumns ++;
		}
	}
	
	if (m_pHttpRequest->GetForm ("Browser") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TBrowser")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		pszString = pHttpForm->GetValue();
		
		if (pszString != NULL && *pszString != '\0') {
			
			if ((pHttpForm = m_pHttpRequest->GetForm ("SBrowser")) == NULL) {
				return ERROR_INVALID_ARGUMENT;
			}
			iValue = pHttpForm->GetIntValue();
			
			piSearchColName [iNumSearchColumns] = SystemEmpireData::Browser;
			pvSearchColData1 [iNumSearchColumns] = pszString;
			pvSearchColData2 [iNumSearchColumns] = iValue;
			pszFormName [iNumSearchColumns] = "Browser";
			pszColName1 [iNumSearchColumns] = "TBrowser";
			pszColName2 [iNumSearchColumns] = "SBrowser";
			iNumSearchColumns ++;
		}
	}
	
	if (m_pHttpRequest->GetForm ("Privilege") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("SPrivilege")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::Privilege;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue;
		pszFormName [iNumSearchColumns] = "Privilege";
		pszColName1 [iNumSearchColumns] = "SPrivilege";
		pszColName2 [iNumSearchColumns] = NULL;
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("NumLogins") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TNumLogins1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TNumLogins2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::NumLogins;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "NumLogins";
		pszColName1 [iNumSearchColumns] = "TNumLogins1";
		pszColName2 [iNumSearchColumns] = "TNumLogins2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("AlienKey") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TAlienKey1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TAlienKey2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::AlienKey;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "AlienKey";
		pszColName1 [iNumSearchColumns] = "TAlienKey1";
		pszColName2 [iNumSearchColumns] = "TAlienKey2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("Wins") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TWins1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TWins2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::Wins;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "Wins";
		pszColName1 [iNumSearchColumns] = "TWins1";
		pszColName2 [iNumSearchColumns] = "TWins2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("Nukes") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TNukes1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TNukes2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::Nukes;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "Nukes";
		pszColName1 [iNumSearchColumns] = "TNukes1";
		pszColName2 [iNumSearchColumns] = "TNukes2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("Nuked") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TNuked1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TNuked2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::Nuked;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "Nuked";
		pszColName1 [iNumSearchColumns] = "TNuked1";
		pszColName2 [iNumSearchColumns] = "TNuked2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("Draws") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TDraws1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TDraws2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::Draws;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "Draws";
		pszColName1 [iNumSearchColumns] = "TDraws1";
		pszColName2 [iNumSearchColumns] = "TDraws2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("AScore") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TAScore1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		fValue = pHttpForm->GetFloatValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TAScore2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		fValue2 = pHttpForm->GetFloatValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::AlmonasterScore;
		pvSearchColData1 [iNumSearchColumns] = fValue;
		pvSearchColData2 [iNumSearchColumns] = fValue2;
		pszFormName [iNumSearchColumns] = "AScore";
		pszColName1 [iNumSearchColumns] = "TAScore1";
		pszColName2 [iNumSearchColumns] = "TAScore2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("AScoreSig") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TAScoreSig1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TAScoreSig2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::AlmonasterScoreSignificance;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "AScoreSig";
		pszColName1 [iNumSearchColumns] = "TAScoreSig1";
		pszColName2 [iNumSearchColumns] = "TAScoreSig2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("CScore") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TCScore1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		fValue = pHttpForm->GetFloatValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TCScore2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		fValue2 = pHttpForm->GetFloatValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::ClassicScore;
		pvSearchColData1 [iNumSearchColumns] = fValue;
		pvSearchColData2 [iNumSearchColumns] = fValue2;
		pszFormName [iNumSearchColumns] = "CScore";
		pszColName1 [iNumSearchColumns] = "TCScore1";
		pszColName2 [iNumSearchColumns] = "TCScore2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("MaxEcon") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TMaxEcon1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TMaxEcon2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::MaxEcon;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "MaxEcon";
		pszColName1 [iNumSearchColumns] = "TMaxEcon1";
		pszColName2 [iNumSearchColumns] = "TMaxEcon2";
		iNumSearchColumns ++;
	}
	
	if (m_pHttpRequest->GetForm ("MaxMil") != NULL) {
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TMaxMil1")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue = pHttpForm->GetIntValue();
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("TMaxMil2")) == NULL) {
			return ERROR_INVALID_ARGUMENT;
		}
		iValue2 = pHttpForm->GetIntValue();
		
		piSearchColName [iNumSearchColumns] = SystemEmpireData::MaxMil;
		pvSearchColData1 [iNumSearchColumns] = iValue;
		pvSearchColData2 [iNumSearchColumns] = iValue2;
		pszFormName [iNumSearchColumns] = "MaxMil";
		pszColName1 [iNumSearchColumns] = "TMaxMil1";
		pszColName2 [iNumSearchColumns] = "TMaxMil2";
		iNumSearchColumns ++;
	}
	
	Assert (iNumSearchColumns <= MAX_NUM_SEARCH_COLUMNS);
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("Skip")) != NULL) {
		iSkip = pHttpForm->GetIntValue();
	} else {
		return ERROR_INVALID_ARGUMENT;
	}
	
	if (iNumSearchColumns == 0 ||
		(pHttpForm = m_pHttpRequest->GetForm ("MaxNumHits")) == NULL ||
		(iMaxNumHits = pHttpForm->GetIntValue()) < 1
		) {
		
		return ERROR_INVALID_QUERY;
		
	}
	
	// Get startkey
	pHttpForm = m_pHttpRequest->GetForm ("StartKey");
	if (pHttpForm != NULL) {
		iStartKey = pHttpForm->GetIntValue();
	}
	
	iErrCode = g_pGameEngine->PerformMultipleSearch (
		iStartKey,
		iSkip,
		iMaxNumHits,
		iNumSearchColumns,
		piSearchColName,
		pvSearchColData1,
		pvSearchColData2,
		ppiSearchEmpireKey,
		piNumSearchEmpires,
		piLastKey
		);
	
	// Out
	*piNumSearchColumns = iNumSearchColumns;
	*piMaxNumHits = iMaxNumHits;
	
	return iErrCode;
}


int HtmlRenderer::RenderSearchForms (bool bAdmin) {
	
	int iNumEmpires, iErrCode = g_pGameEngine->GetNumEmpiresOnServer (&iNumEmpires);
	if (iErrCode != OK) {
		return iErrCode;
	}
	
	OutputText ("<p>There ");
	
	if (iNumEmpires == 1) {
		OutputText (" is <strong>1</strong> registered empire ");
	} else {
		OutputText (" are <strong>");
		m_pHttpResponse->WriteText (iNumEmpires);
		OutputText ("</strong> registered empires");
	}
	OutputText (" on the server");
	
	if (!bAdmin) {
		
		OutputText (
			
			"<p>Search for the following empire:"\
			"<p><input type=\"hidden\" name=\"EmpireName\" value=\"on\">"\
			"Empire name: <input type=\"text\" size=\"20\" name=\"TEmpireName\"> "\
			"<select name=\"SEmpireName\" size=\"1\">"\
			
			"<option selected value=\""
			);
		
		m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Exact search</option><option value=\"");
		m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Substring search</option><option value=\"");
		m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Begins with search</option></select>");
		
	} else {
		
		OutputText (
			
			"<p>Choose the characteristics of the empires you wish to find:"\
			
			"<p><table width=\"65%\">"\
			"<tr>"\
			"<td><input type=\"checkbox\" checked name=\"EmpireName\"></td>"\
			"<td>Empire name:</td>"\
			"<td><input type=\"text\" size=\"20\" name=\"TEmpireName\"></td>"\
			"<td><select name=\"SEmpireName\" size=\"1\">"\
			
			"<option selected value=\""
			);
		
		m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Exact search</option><option value=\"");
		m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Substring search</option><option value=\"");
		m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
		OutputText (
			
			"\">Begins with search</option></select></td></tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"EmpireKeyF\"></td>"\
			"<td>Empire key:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TEmpireKey1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TEmpireKey2\" value=\"0\"></td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"RealName\"></td>"\
			"<td>Real name:</td>"\
			"<td><input type=\"text\" size=\"20\" name=\"TRealName\"></td>"\
			"<td><select name=\"SRealName\" size=\"1\">"\
			"<option selected value=\""
			);
		
		m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Exact search</option><option value=\"");
		m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Substring search</option><option value=\"");
		m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
		OutputText (
			"\">Begins with search</option></select></td></tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Email\"></td>"\
			"<td>Email address:</td>"\
			"<td><input type=\"text\" size=\"20\" name=\"TEmail\"></td>"\
			"<td><select name=\"SEmail\" size=\"1\">"\
			"<option selected value=\""
			);
		
		m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Exact search</option><option value=\"");
		m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Substring search</option><option value=\"");
		m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
		OutputText (
			
			"\">Begins with search</option></select></td></tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"WebPage\"></td>"\
			"<td>Webpage URL:</td>"\
			"<td><input type=\"text\" size=\"20\" name=\"TWebPage\"></td>"\
			"<td><select name=\"SWebPage\" size=\"1\">"\
			"<option selected value=\""
			);
		
		m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Exact search</option><option value=\"");
		m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Substring search</option><option value=\"");
		m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
		
		OutputText (
			
			"\">Begins with search</option></select></td></tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Quote\"></td>"\
			"<td>Quote:</td>"\
			"<td><input type=\"text\" size=\"20\" name=\"TQuote\"></td>"\
			"<td><select name=\"SQuote\" size=\"1\">"\
			"<option selected value=\""
			);
		
		m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Exact search</option><option value=\"");
		m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Substring search</option><option value=\"");
		m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
		
		OutputText (
			
			"\">Begins with search</option>"\
			"</select></td></tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Sneer\"></td>"\
			"<td>Victory Sneer:</td>"\
			"<td><input type=\"text\" size=\"20\" name=\"TSneer\"></td>"\
			"<td><select name=\"SSneer\" size=\"1\">"\
			"<option selected value=\""
			);
		
		m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Exact search</option><option value=\"");
		m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Substring search</option><option value=\"");
		m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
		
		OutputText ("\">Begins with search</option></select></td></tr>");
		
		if (m_iPrivilege >= ADMINISTRATOR) {
			
			OutputText (
				"<tr>"\
				"<td><input type=\"checkbox\" name=\"IPAddress\"></td>"\
				"<td>IP address:</td>"\
				"<td><input type=\"text\" size=\"20\" name=\"TIPAddress\"></td>"\
				"<td><select name=\"SIPAddress\" size=\"1\">"\
				"<option selected value=\""
				);
			
			m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
			OutputText ("\">Exact search</option><option value=\"");
			m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
			OutputText ("\">Substring search</option><option value=\"");
			m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
			
			OutputText ("\">Begins with search</option></select></td></tr>");
			
			OutputText (
				"<tr>"\
				"<td><input type=\"checkbox\" name=\"SessionId\"></td>"\
				"<td>Session Id:</td>"\
				"<td><input type=\"text\" size=\"6\" name=\"TSessionId1\" value=\"0\"> to <input"\
				" type=\"text\" size=\"6\" name=\"TSessionId2\" value=\"0\"></td>"\
				"</tr>"\
				);
		}
		
		OutputText (
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Browser\"></td>"\
			"<td>Browser:</td>"\
			"<td><input type=\"text\" size=\"20\" name=\"TBrowser\"></td>"\
			"<td><select name=\"SBrowser\" size=\"1\">"\
			
			"<option selected value=\""
			);
		
		m_pHttpResponse->WriteText (EXACT_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Exact search</option><option value=\"");
		m_pHttpResponse->WriteText (SUBSTRING_SEARCH_CASE_INSENSITIVE);
		OutputText ("\">Substring search</option><option value=\"");
		m_pHttpResponse->WriteText (BEGINS_WITH_SEARCH_CASE_INSENSITIVE);
		
		OutputText (
			
			"\">Begins with search</option>"\
			"</select></td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Privilege\"></td>"\
			"<td>Privilege level:</td>"\
			"<td><select name=\"SPrivilege\" size=\"1\">"
			);
		
		int i, j;
		ENUMERATE_PRIVILEGE_LEVELS(i) {
			
			if (i == NOVICE) {
				OutputText ("<option selected");
			} else {
				OutputText ("<option");
			}
			OutputText (" value=\"");
			m_pHttpResponse->WriteText (i);
			OutputText ("\">");
			m_pHttpResponse->WriteText (PRIVILEGE_STRING[i]);
			OutputText ("</option>");
		}
		
		OutputText (
			
			"</select></td>"\
			
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"NumLogins\"></td>"\
			"<td>Number of logins:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TNumLogins1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TNumLogins2\" value=\"0\"> logins</td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"AlienKey\"></td>"\
			"<td>Alien key:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TAlienKey1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TAlienKey2\" value=\"0\"></td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Wins\"></td>"\
			"<td>Wins:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TWins1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TWins2\" value=\"0\"> wins</td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Nukes\"></td>"\
			"<td>Nukes:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TNukes1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TNukes2\" value=\"0\"> nukes</td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Nuked\"></td>"\
			"<td>Nuked:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TNuked1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TNuked2\" value=\"0\"> nuked</td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"Draws\"></td>"\
			"<td>Draws:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TDraws1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TDraws2\" value=\"0\"> draws</td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"AScore\"></td>"\
			"<td>Almonaster Score:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TAScore1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TAScore2\" value=\"0\"> points</td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"AScoreSig\"></td>"\
			"<td>Almonaster score significance:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TAScoreSig1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TAScoreSig2\" value=\"0\"> points</td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"CScore\"></td>"\
			"<td>Classic score:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TCScore1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TCScore2\" value=\"0\"> points</td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"MaxEcon\"></td>"\
			"<td>Max econ:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TMaxEcon1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TMaxEcon2\" value=\"0\"></td>"\
			"</tr>"\
			
			"<tr>"\
			"<td><input type=\"checkbox\" name=\"MaxMil\"></td>"\
			"<td>Max mil:</td>"\
			"<td><input type=\"text\" size=\"6\" name=\"TMaxMil1\" value=\"0\"> to <input"\
			" type=\"text\" size=\"6\" name=\"TMaxMil2\" value=\"0\"></td>"\
			"</tr></table>"
			);
		
	}	// End if admin
	
    OutputText (
		"<p>Return up to <input type=\"text\" size=\"3\" name=\"MaxNumHits\" value=\"20\"> "\
		"hits per page, skip the first <input type=\"text\" size=\"3\" name=\"Skip\" value=\"0\"><p>"
		);
	
	WriteButton (BID_SEARCH);
	
	return OK;
}

void HtmlRenderer::RenderSearchResults (unsigned int* piSearchColName, 
										Variant* pvSearchColData1,
										Variant* pvSearchColData2,
										const char** pszFormName,
										const char** pszColName1,
										const char** pszColName2,
										
										int iNumSearchColumns,
										
										int* piSearchEmpireKey,
										int iNumSearchEmpires,
										int iLastKey,
										int iMaxNumHits
										) {
	int i, iErrCode;
	
	OutputText ("<p>");
	
	if (iLastKey != NO_KEY) {
		OutputText ("More than ");
	}
	OutputText ("<strong>");
	m_pHttpResponse->WriteText (iNumSearchEmpires);
	OutputText ("</strong> empire");
	
	if (iNumSearchEmpires == 1) {
		OutputText (" was");
	} else {
		OutputText ("s were");
	}
	
	OutputText (" found:<p><table><tr>");
	OutputText ("<th align=\"center\" bgcolor=\"");
	m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
	OutputText ("\"><strong>Empire name</strong></th>");
	OutputText ("<th align=\"center\" bgcolor=\"");
	m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
	OutputText ("\"><strong>Icon</strong></th>");
	
	for (i = 0; i < iNumSearchColumns; i ++) {
		
		switch (piSearchColName[i]) {
			
		case NO_KEY:
			
			OutputText ("<th align=\"center\" bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\">");
			OutputText ("<strong>Empire key</strong></td>");
			
			break;
			
		case SystemEmpireData::Name:
			
			break;
			
		default:
			
			OutputText ("<th align=\"center\" bgcolor=\"");
			m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
			OutputText ("\"><strong>"); 
			m_pHttpResponse->WriteText (SYSTEM_EMPIRE_DATA_COLUMN_NAMES [piSearchColName[i]]);
			OutputText ("</strong></td>");
			
			break;
		}
	}
	
	OutputText ("</tr>");
	
	int j, iAlien;
	Variant vName, vData;
	char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
	
	const char* pszString;
	
	NotifyProfileLink();
	
	for (i = 0; i < iNumSearchEmpires; i ++) {
		
		if (g_pGameEngine->GetEmpireName (piSearchEmpireKey[i], &vName) == OK &&
			g_pGameEngine->GetEmpireAlienKey (piSearchEmpireKey[i], &iAlien) == OK) {
			
			OutputText ("<tr><td align=\"center\"><strong>");
			m_pHttpResponse->WriteText (vName.GetCharPtr());
			OutputText ("</strong></td><td align=\"center\">");
			
			sprintf (pszProfile, "View the profile of %s", vName.GetCharPtr());
			
			WriteProfileAlienString (
				iAlien, 
				piSearchEmpireKey[i],
				vName.GetCharPtr(),
				0, 
				"ViewProfile",
				pszProfile,
				true,
				true
				);
			
			OutputText ("</td>");
			
			iErrCode = OK;
			
			for (j = 0; j < iNumSearchColumns && iErrCode == OK; j ++) {
				
				if (piSearchColName[j] != SystemEmpireData::Name) {
					
					OutputText ("<td align=\"center\">");
					
					if (piSearchColName[j] == NO_KEY) {
						m_pHttpResponse->WriteText (piSearchEmpireKey[i]);
					} else {
						
						iErrCode = g_pGameEngine->GetEmpireDataColumn (
							piSearchEmpireKey[i], 
							piSearchColName[j], 
							&vData
							);
						
						if (iErrCode == OK) {
							
							switch (piSearchColName[j]) {
								
							case SystemEmpireData::Privilege:
								
								m_pHttpResponse->WriteText (PRIVILEGE_STRING[vData.GetInteger()]);
								break;
								
							case SystemEmpireData::WebPage:
								
								pszString = vData.GetCharPtr();
								if (!String::IsBlank (pszString)) {
									OutputText ("<a href=\"");
									m_pHttpResponse->WriteText (pszString);
									OutputText ("\">");
									m_pHttpResponse->WriteText (pszString);
									OutputText ("</a>");
								}
								break;
								
							default:
								
								m_pHttpResponse->WriteText (vData);
								break;
							}
						}
					}
					OutputText ("</td>");
				}
			}
			
			OutputText ("</tr>");
		}
	}
	
	OutputText ("</table>");
	
	// Lay down query information for submission if more empires were found
	if (iLastKey != NO_KEY) {
		
		OutputText ("<p>Search for the next ");
		
		if (iMaxNumHits != 1) {
			m_pHttpResponse->WriteText (iMaxNumHits);
			OutputText (" empires");
		} else {
			OutputText ("empire");
		}
		
		OutputText (": "); WriteButton (BID_SEARCH);
		
		OutputText ("<input type=\"hidden\" name=\"Skip\" value=\"0\">");
		OutputText ("<input type=\"hidden\" name=\"MaxNumHits\" value=\"");
		m_pHttpResponse->WriteText (iMaxNumHits);
		OutputText ("\">");
		OutputText ("<input type=\"hidden\" name=\"StartKey\" value=\"");
		m_pHttpResponse->WriteText (iLastKey);
		OutputText ("\">");
		
		for (i = 0; i < iNumSearchColumns; i ++) {
			
			OutputText ("<input type=\"hidden\" name=\"");
			m_pHttpResponse->WriteText (pszFormName[i]);
			OutputText ("\" value=\"1\">");
			OutputText ("<input type=\"hidden\" name=\"");
			m_pHttpResponse->WriteText (pszColName1[i]);
			OutputText ("\" value=\"");
			m_pHttpResponse->WriteText (pvSearchColData1[i]);
			OutputText ("\">");
			
			if (pszColName2[i] != NULL) {
				OutputText ("<input type=\"hidden\" name=\"");
				m_pHttpResponse->WriteText (pszColName2[i]);
				OutputText ("\" value=\"");
				m_pHttpResponse->WriteText (pvSearchColData2[i]);
				OutputText ("\">");
			}
		}
	}
	
	OutputText ("<p>");
	WriteButton (BID_CANCEL);
}


int HtmlRenderer::GetSensitiveMapText (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey,
									   int iProxyPlanetKey, bool bVisibleBuilds, bool bIndependence,
									   void** ppPlanetData, String* pstrAltTag) {
	
	int iErrCode = OK, iNumOwners;
	
	int iTotalNumShips = 
		*((int*) ppPlanetData[GameMap::NumUncloakedShips]) + 
		*((int*) ppPlanetData[GameMap::NumCloakedShips]) + 
		*((int*) ppPlanetData[GameMap::NumUncloakedBuildShips]) + 
		*((int*) ppPlanetData[GameMap::NumCloakedBuildShips]);
	
	*pstrAltTag = "";
	
	if (iTotalNumShips == 0) {
		goto End;
	}
	
	int* piOwnerData;
	iErrCode = g_pGameEngine->GetPlanetShipOwnerData (
		iGameClass,
		iGameNumber,
		iEmpireKey,
		iPlanetKey,
		iProxyPlanetKey,
		iTotalNumShips,
		bVisibleBuilds,
		bIndependence,
		&piOwnerData
		);
	
	if (iErrCode != OK) {
		Assert (false);
		goto End;
	}
	
	iNumOwners = piOwnerData[0];
	
	if (iNumOwners > 0) {
		
		int i, j, iBase, iNumOwnerTechs, iType, iOwnerKey, iNumShips;
		Variant vEmpireName;
		
		iBase = 1;
		
		for (i = 0; i < iNumOwners; i ++) {
			
			if (i > 0) {
				*pstrAltTag += "\n";
			}
			
			iOwnerKey = piOwnerData [iBase];
			iNumOwnerTechs = piOwnerData [iBase + 1];
			
			Assert (iNumOwnerTechs > 0);
			
			if (iOwnerKey == m_iEmpireKey) {
				*pstrAltTag += m_vEmpireName.GetCharPtr();
			}
			
			else if (iOwnerKey == INDEPENDENT) {
				*pstrAltTag += "Independent";
			}
			
			else {
				
				iErrCode = g_pGameEngine->GetEmpireName (iOwnerKey, &vEmpireName);
				if (iErrCode != OK) {
					Assert (false);
					continue;
				}
				
				*pstrAltTag += vEmpireName.GetCharPtr();
			}
			
			*pstrAltTag += ": ";
			
			for (j = 0; j < iNumOwnerTechs; j ++) {
				
				iType = piOwnerData [iBase + 2 + j * 2];
				iNumShips = piOwnerData [iBase + 3 + j * 2];
				
				Assert (iType >= FIRST_SHIP && iType <= LAST_SHIP);
				Assert (iNumShips > 0);
				
				if (j > 0) {
					*pstrAltTag += ", ";
				}
				
				*pstrAltTag += iNumShips;
				*pstrAltTag += " ";
				
				if (iNumShips == 1) {
					*pstrAltTag += SHIP_TYPE_STRING[iType];
				} else {
					*pstrAltTag += SHIP_TYPE_STRING_PLURAL[iType];
				}
			}
			
			iBase += 2 + 2 * iNumOwnerTechs;
		}
	}
	
	delete [] piOwnerData;
	
End:
	
	return iErrCode;
}


void HtmlRenderer::RenderGameConfiguration (int iGameClass) {
	
	int iMaxUpdatesBeforeGameCloses, iNumUpdatesBeforeGameCloses, iHrsUD, iMinUD, iSecUD, iMaxNumEmpires, 
		iBridier, iErrCode, iFilterIP, iFilterId;
	
	bool bRestrictAlmonaster, bRestrictClassic, bRestrictBridierRank, bRestrictBridierIndex,
		bRestrictBridierRankGain, bRestrictWins, bNamesListed, bSpectators;
	
	const char* pszRestrictAlmonasterMin = NULL, * pszRestrictAlmonasterMax = NULL, * pszRestrictClassicMin = NULL,
		* pszRestrictClassicMax = NULL, * pszRestrictBridierRankMin = NULL, * pszRestrictBridierRankMax = NULL, 
		* pszRestrictBridierIndexMin = NULL, * pszRestrictBridierIndexMax = NULL, * pszRestrictBridierRankGainMin = NULL, 
		* pszRestrictBridierRankGainMax = NULL, * pszRestrictWinsMin = NULL, * pszRestrictWinsMax = NULL;
	
	float fRestrictAlmonasterMin = 0, fRestrictAlmonasterMax = 0, fRestrictClassicMin = 0,
		fRestrictClassicMax = 0;
	
	int iRestrictBridierRankMin = 0, iRestrictBridierRankMax = 0, iRestrictBridierIndexMin = 0, 
		iRestrictBridierIndexMax = 0, iRestrictBridierRankGainMin = 0, iRestrictBridierRankGainMax = 0, 
		iRestrictWinsMin = 0, iRestrictWinsMax = 0;
	
	IHttpForm* pHttpForm;

	const char* pszMessage, * pszGamePassword, * pszGamePassword2;

	int iSystemOptions, iGameClassOptions;

	char pszFormIP [64];
	char pszFormID [64];

	unsigned int iNumBlocks = 0;

	iErrCode = g_pGameEngine->GetSystemOptions (&iSystemOptions);
	if (iErrCode != OK) {
		goto OnError;
	}

	if (iGameClass != NO_KEY) {

		iErrCode = g_pGameEngine->GetGameClassOptions (iGameClass, &iGameClassOptions);
		if (iErrCode != OK) {
			goto OnError;
		}

	} else {

		iGameClassOptions = 0;
	}

	// NumUpdatesClose
	if ((pHttpForm = m_pHttpRequest->GetForm ("NumUpdatesForClose")) != NULL) {
		iNumUpdatesBeforeGameCloses = pHttpForm->GetIntValue();
	} else {
		iErrCode = g_pGameEngine->GetDefaultNumUpdatesBeforeClose (&iNumUpdatesBeforeGameCloses);
		if (iErrCode != OK) {
			goto OnError;
		}
	}
	
	// MaxUpdatesBeforeGameCloses
	iErrCode = g_pGameEngine->GetMaxNumUpdatesBeforeClose (&iMaxUpdatesBeforeGameCloses);
	if (iErrCode != OK) {
		goto OnError;
	}
	
	// MaxNumEmpires
	if (iGameClass == NO_KEY) {
		iMaxNumEmpires = 2;
	} else {
		iErrCode = g_pGameEngine->GetMaxNumEmpires (iGameClass, &iMaxNumEmpires);
		if (iErrCode != OK) {
			goto OnError;
		}
	}
	
	// Bridier
	if ((pHttpForm = m_pHttpRequest->GetForm ("Bridier")) != NULL) {
		iBridier = pHttpForm->GetIntValue();
	} else {
		iBridier = (iSystemOptions & DEFAULT_BRIDIER_GAMES) ? GAME_COUNT_FOR_BRIDIER : 0;
	}
	
	// HoursUD
	if ((pHttpForm = m_pHttpRequest->GetForm ("HoursUD")) != NULL) {
		iHrsUD = pHttpForm->GetIntValue();
	} else {
		iHrsUD = 0;
	}
	
	// MinsUD
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinsUD")) != NULL) {
		iMinUD = pHttpForm->GetIntValue();
	} else {
		iMinUD = 0;
	}
	
	// SecsUD
	if ((pHttpForm = m_pHttpRequest->GetForm ("SecsUD")) != NULL) {
		iSecUD = pHttpForm->GetIntValue();
	} else {
		iSecUD = 0;
	}

	// NamesListed
	if ((pHttpForm = m_pHttpRequest->GetForm ("NamesListed")) != NULL) {
		bNamesListed = pHttpForm->GetIntValue() != 0;
	} else {
		bNamesListed = (iSystemOptions & DEFAULT_NAMES_LISTED) != 0;
	}

	// Spectators
	if ((pHttpForm = m_pHttpRequest->GetForm ("Spectators")) != NULL) {
		bSpectators = pHttpForm->GetIntValue() != 0;
	} else {
		bSpectators = (iSystemOptions & DEFAULT_ALLOW_SPECTATORS) != 0;
	}

	// EnterGameMessage
	if ((pHttpForm = m_pHttpRequest->GetForm ("EnterGameMessage")) != NULL) {
		pszMessage = pHttpForm->GetValue();
	} else {
		pszMessage = NULL;
	}
	
	// RestrictAlmonaster
	bRestrictAlmonaster = m_pHttpRequest->GetForm ("RestrictAlmonaster") != NULL;
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictAlmonasterMin")) != NULL) {
		
		pszRestrictAlmonasterMin = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictAlmonasterMin, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictAlmonasterMin, LOWEST_STRING) != 0) {
			fRestrictAlmonasterMin = pHttpForm->GetFloatValue();
			pszRestrictAlmonasterMin = NULL;
		}
		
	} else {
		fRestrictAlmonasterMin = ALMONASTER_MIN_SCORE;
	}
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictAlmonasterMax")) != NULL) {
		
		pszRestrictAlmonasterMax = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictAlmonasterMax, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictAlmonasterMax, LOWEST_STRING) != 0) {
			fRestrictAlmonasterMax = pHttpForm->GetFloatValue();
			pszRestrictAlmonasterMax = NULL;
		}
		
	} else {
		pszRestrictAlmonasterMax = HIGHEST_STRING;
	}
	
	// RestrictClassic
	bRestrictClassic = m_pHttpRequest->GetForm ("RestrictClassic") != NULL;
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictClassicMin")) != NULL) {
		
		pszRestrictClassicMin = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictClassicMin, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictClassicMin, LOWEST_STRING) != 0) {
			fRestrictClassicMin = pHttpForm->GetFloatValue();
			pszRestrictClassicMin = NULL;
		}
		
	} else {
		pszRestrictClassicMin = LOWEST_STRING;
	}
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictClassicMax")) != NULL) {
		
		pszRestrictClassicMax = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictClassicMax, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictClassicMax, LOWEST_STRING) != 0) {
			fRestrictClassicMax = pHttpForm->GetFloatValue();
			pszRestrictClassicMax = NULL;
		}
		
	} else {
		pszRestrictClassicMax = HIGHEST_STRING;
	}
	
	// RestrictBridierRank
	bRestrictBridierRank = m_pHttpRequest->GetForm ("RestrictBridierRank") != NULL;
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankMin")) != NULL) {
		
		pszRestrictBridierRankMin = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictBridierRankMin, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictBridierRankMin, LOWEST_STRING) != 0) {
			iRestrictBridierRankMin = pHttpForm->GetIntValue();
			pszRestrictBridierRankMin = NULL;
		}
		
	} else {
		iRestrictBridierRankMin = BRIDIER_MIN_RANK;
	}
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankMax")) != NULL) {
		
		pszRestrictBridierRankMax = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictBridierRankMax, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictBridierRankMax, LOWEST_STRING) != 0) {
			iRestrictBridierRankMax = pHttpForm->GetIntValue();
			pszRestrictBridierRankMax = NULL;
		}
		
	} else {
		pszRestrictBridierRankMax = HIGHEST_STRING;
	}
	
	// RestrictBridierIndex
	bRestrictBridierIndex = m_pHttpRequest->GetForm ("RestrictBridierIndex") != NULL;
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierIndexMin")) != NULL) {
		
		pszRestrictBridierIndexMin = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictBridierIndexMin, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictBridierIndexMin, LOWEST_STRING) != 0) {
			iRestrictBridierIndexMin = pHttpForm->GetIntValue();
			pszRestrictBridierIndexMin = NULL;
		}
		
	} else {
		iRestrictBridierIndexMin = BRIDIER_MIN_INDEX;
	}
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierIndexMax")) != NULL) {
		
		pszRestrictBridierIndexMax = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictBridierIndexMax, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictBridierIndexMax, LOWEST_STRING) != 0) {
			iRestrictBridierIndexMax = pHttpForm->GetIntValue();
			pszRestrictBridierIndexMax = NULL;
		}
		
	} else {
		iRestrictBridierIndexMax = BRIDIER_MAX_INDEX;
	}
	
	// RestrictBridierRankGain
	bRestrictBridierRankGain = m_pHttpRequest->GetForm ("RestrictBridierRankGain") != NULL;
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankGainMin")) != NULL) {
		
		pszRestrictBridierRankGainMin = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictBridierRankGainMin, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictBridierRankGainMin, LOWEST_STRING) != 0) {
			iRestrictBridierRankGainMin = pHttpForm->GetIntValue();
			pszRestrictBridierRankGainMin = NULL;
		}
		
	} else {
		iRestrictBridierRankGainMin = BRIDIER_MIN_RANK_GAIN;
	}
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankGainMax")) != NULL) {
		
		pszRestrictBridierRankGainMax = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictBridierRankGainMax, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictBridierRankGainMax, LOWEST_STRING) != 0) {
			iRestrictBridierRankGainMax = pHttpForm->GetIntValue();
			pszRestrictBridierRankGainMax = NULL;
		}
		
	} else {
		pszRestrictBridierRankGainMax = HIGHEST_STRING;
	}
	
	// RestrictWins
	bRestrictWins = m_pHttpRequest->GetForm ("RestrictWins") != NULL;
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictWinsMin")) != NULL) {
		
		pszRestrictWinsMin = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictWinsMin, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictWinsMin, LOWEST_STRING) != 0) {
			iRestrictWinsMin = pHttpForm->GetIntValue();
			pszRestrictWinsMin = NULL;
		}
		
	} else {
		iRestrictWinsMin = 0;
	}
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictWinsMax")) != NULL) {
		
		pszRestrictWinsMax = pHttpForm->GetValue();
		
		if (String::StriCmp (pszRestrictWinsMax, HIGHEST_STRING) != 0 &&
			String::StriCmp (pszRestrictWinsMax, LOWEST_STRING) != 0) {
			iRestrictWinsMax = pHttpForm->GetIntValue();
			pszRestrictWinsMax = NULL;
		}
		
	} else {
		pszRestrictWinsMax = HIGHEST_STRING;
	}
	
	// FilterIP
	if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIP")) != NULL) {
		iFilterIP = pHttpForm->GetIntValue();
	} else {
		iFilterIP = GAME_WARN_ON_DUPLICATE_IP_ADDRESS;
	}
	
	// FilterId
	if ((pHttpForm = m_pHttpRequest->GetForm ("FilterId")) != NULL) {
		iFilterId = pHttpForm->GetIntValue();
	} else {
		iFilterId = GAME_WARN_ON_DUPLICATE_SESSION_ID;
	}
	
	// GamePassword
	if ((pHttpForm = m_pHttpRequest->GetForm ("GamePassword")) != NULL) {
		pszGamePassword = pHttpForm->GetValue();
	} else {
		pszGamePassword = NULL;
	}
	
	// GamePassword2
	if ((pHttpForm = m_pHttpRequest->GetForm ("GamePassword2")) != NULL) {
		pszGamePassword2 = pHttpForm->GetValue();
	} else {
		pszGamePassword2 = NULL;
	}
	
	if (iGameClass != NO_KEY) {
		OutputText ("<input type=\"hidden\" name=\"GameClassKey\" value=\"");
		m_pHttpResponse->WriteText (iGameClass);
		OutputText ("\">");
	}
	
	OutputText (
		
		"<table width=\"90%\">"\
		
		"<tr>"\
		"<td>Updates before game closes <em>(<strong>1</strong> to <strong>"
		);
	
	m_pHttpResponse->WriteText (iMaxUpdatesBeforeGameCloses);	
	
	OutputText (
		"</strong>)</em>:</td>"\
		"<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"NumUpdatesForClose\" value=\""
		);
	
	m_pHttpResponse->WriteText (iNumUpdatesBeforeGameCloses);
	
	// First update delay
	OutputText (
		
		"\"></td></tr>"\
		
		"<tr>"\
		"<td>First update delay (<em>at most <strong>10</strong> update periods</em>):</td>"\
		"<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"HoursUD\" value=\"");
	
	m_pHttpResponse->WriteText (iHrsUD);
	
	OutputText (
		
		"\"> hours, "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinsUD\" value=\"");
	
	m_pHttpResponse->WriteText (iMinUD);
	
	OutputText (
		
		"\"> minutes, "\
		"<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SecsUD\" value=\"");
	
	m_pHttpResponse->WriteText (iSecUD);
	
	OutputText (
		
		"\"> seconds</td>"\
		"</tr>"
		
		);
	
	// Enter message
	OutputText (
		
		"<tr><td>Message sent to empires entering the game:</td><td>"\
		"<textarea rows=\"3\" cols=\"50\" wrap=\"physical\" name=\"EnterGameMessage\">"
		);
	
	if (pszMessage != NULL) {
		
		String strFilter;
		
		if (HTMLFilter (pszMessage, &strFilter, 0, false) == OK) {
			m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
		}
	}
	
	OutputText ("</textarea></td></tr>");

	// Bridier
	if (iMaxNumEmpires == 2) {

		OutputText (
			
			"<tr>"\
			"<td>Bridier configuration (<em>only for grudge games</em>):</td><td><select name=\"Bridier\">"\
			"<option"
			);
		
		if (iBridier == GAME_COUNT_FOR_BRIDIER) {
			OutputText (" selected");
		}
		
		OutputText (" value=\"");
		m_pHttpResponse->WriteText (GAME_COUNT_FOR_BRIDIER);
		OutputText ("\">Game counts towards Bridier Scoring</option>"\
			
			"<option"
			);
		
		if (iBridier == 0) {
			OutputText (" selected");
		}
		
		OutputText (
			
			" value=\"0\">Game does not count towards Bridier Scoring</option></select>"\
			"</td>"\
			"</tr>"
			);
	}

	OutputText (

		"<tr>"\
		"<td>Empire names exposed:</td>"\
		"<td><select name=\"NamesListed\">"\
		"<option"
		);

	if (bNamesListed) {
		OutputText (" selected");
	}

	OutputText (
		" value=\"1\">Empire names exposed on game lists and broadcast on game entry</option>"\
		"<option"
		);

	if (!bNamesListed) {
		OutputText (" selected");
	}
		
	OutputText (
		" value=\"0\">Empire names not exposed</option>"\
		"</select></td>"\
		"</tr>"
		);

	if (iGameClass == NO_KEY || (iGameClassOptions & EXPOSED_SPECTATORS) == EXPOSED_SPECTATORS) {

		OutputText (
			"<tr>"\
			"<td>Game available to spectators:"
			);

		if (iGameClass == NO_KEY) {
			OutputText ("(<em>requires exposed maps</em>)");
		}
			
		OutputText (
			"</td>"\
			"<td><select name=\"Spectators\">"\
			"<option"
			);

		if (bSpectators) {
			OutputText (" selected");
		}

		OutputText (
			" value=\"1\">Game is available from the Spectator Games page</option>"\
			"<option"
			);

		if (!bSpectators) {
			OutputText (" selected");
		}
			
		OutputText (
			" value=\"0\">Game is not available from the Spectator Games page</option>"\
			"</select></td>"\
			"</tr>"
			);
	}

	OutputText (
		"<tr><td>Password protection: </td>"\
		"<td><table><tr><td>Password:</td><td><input type=\"password\" name=\"GamePassword\" size=\""
		);
	
	m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
	
	OutputText ("\" maxlength=\"");
	m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
	
	if (pszGamePassword != NULL) {
		OutputText ("\" value=\"");
		m_pHttpResponse->WriteText (pszGamePassword);
	}
	
	OutputText ("\"></td></tr><tr><td>Confirm:</td><td><input type=\"password\" name=\"GamePassword2\" size=\"");
	m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
	OutputText ("\" maxlength=\"");
	
	m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
	
	if (pszGamePassword2 != NULL) {
		OutputText ("\" value=\"");
		m_pHttpResponse->WriteText (pszGamePassword2);
	}
	
	OutputText (
		
		"\"></td></tr></table></td></tr>"\
		
		"<tr>"\
		"<td>Empire filtering by Almonaster Score:</td>"\
		"<td><input type=\"checkbox\""
		);
	
	if (bRestrictAlmonaster) {
		OutputText (" checked");
	}
	
	OutputText (
		" name=\"RestrictAlmonaster\">"\
		" From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictAlmonasterMin\" value=\""
		);
	
	if (pszRestrictAlmonasterMin != NULL) {
		m_pHttpResponse->WriteText (pszRestrictAlmonasterMin);
	} else {
		m_pHttpResponse->WriteText (fRestrictAlmonasterMin);
	}
	
	OutputText (
		"\"> to "\
		"<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictAlmonasterMax\" value=\""
		);
	
	if (pszRestrictAlmonasterMax != NULL) {
		m_pHttpResponse->WriteText (pszRestrictAlmonasterMax);
	} else {
		m_pHttpResponse->WriteText (fRestrictAlmonasterMax);
	}
	
	OutputText (
		"\">"\
		"</td>"\
		"</tr>"\
		
		"<tr>"\
		"<td>Empire filtering by Classic Score:</td>"\
		"<td><input type=\"checkbox\""
		);
	
	if (bRestrictClassic) {
		OutputText (" checked");
	}
	
	OutputText (
		" name=\"RestrictClassic\">"\
		" From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictClassicMin\" value=\""
		);
	
	if (pszRestrictClassicMin != NULL) {
		m_pHttpResponse->WriteText (pszRestrictClassicMin);
	} else {
		m_pHttpResponse->WriteText (fRestrictClassicMin);
	}
	
	OutputText (
		"\"> to "\
		"<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictClassicMax\" value=\""
		);
	
	if (pszRestrictClassicMax != NULL) {
		m_pHttpResponse->WriteText (pszRestrictClassicMax);
	} else {
		m_pHttpResponse->WriteText (fRestrictClassicMax);
	}
	
	OutputText (
		"\">"\
		"</td>"\
		"</tr>"
		);
	
	if (iMaxNumEmpires == 2) {
		
		OutputText (
			"<tr>"\
			"<td>Empire filtering by Bridier Rank:</td>"\
			"<td><input type=\"checkbox\""
			);
		
		if (bRestrictBridierRank) {
			OutputText (" checked");
		}
		
		OutputText (
			" name=\"RestrictBridierRank\">"\
			" From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankMin\" value=\""
			);
		
		if (pszRestrictBridierRankMin != NULL) {
			m_pHttpResponse->WriteText (pszRestrictBridierRankMin);
		} else {
			m_pHttpResponse->WriteText (iRestrictBridierRankMin);
		}
		
		OutputText (
			"\"> to "\
			"<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankMax\" value=\""
			);
		
		if (pszRestrictBridierRankMax != NULL) {
			m_pHttpResponse->WriteText (pszRestrictBridierRankMax);
		} else {
			m_pHttpResponse->WriteText (iRestrictBridierRankMax);
		}
		
		OutputText (
			"\">"\
			"</td>"\
			"</tr>"
			
			"<tr>"\
			"<td>Empire filtering by Bridier Index:</td>"\
			"<td><input type=\"checkbox\""
			);
		
		if (bRestrictBridierIndex) {
			OutputText (" checked");
		}
		
		OutputText (
			" name=\"RestrictBridierIndex\">"\
			" From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierIndexMin\" value=\""
			);
		
		if (pszRestrictBridierIndexMin != NULL) {
			m_pHttpResponse->WriteText (pszRestrictBridierIndexMin);
		} else {
			m_pHttpResponse->WriteText (iRestrictBridierIndexMin);
		}
		
		OutputText (
			"\"> to "\
			"<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierIndexMax\" value=\""
			);
		
		if (pszRestrictBridierIndexMax != NULL) {
			m_pHttpResponse->WriteText (pszRestrictBridierIndexMax);
		} else {
			m_pHttpResponse->WriteText (iRestrictBridierIndexMax);
		}
		
		OutputText (
			"\">"\
			"</td>"\
			"</tr>"
			
			"<tr>"\
			"<td>Empire filtering by Bridier Rank Gain:</td>"\
			"<td><input type=\"checkbox\""
			);
		
		if (bRestrictBridierRankGain) {
			OutputText (" checked");
		}
		
		OutputText (
			" name=\"RestrictBridierRankGain\">"\
			" From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankGainMin\" value=\""
			);
		
		if (pszRestrictBridierRankGainMin != NULL) {
			m_pHttpResponse->WriteText (pszRestrictBridierRankGainMin);
		} else {
			m_pHttpResponse->WriteText (iRestrictBridierRankGainMin);
		}
		
		OutputText (
			"\"> to "\
			"<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankGainMax\" value=\""
			);
		
		if (pszRestrictBridierRankGainMax != NULL) {
			m_pHttpResponse->WriteText (pszRestrictBridierRankGainMax);
		} else {
			m_pHttpResponse->WriteText (iRestrictBridierRankGainMax);
		}
		
		OutputText (
			"\">"\
			"</td>"\
			"</tr>"
			);
	}
	
	OutputText (
		"<tr>"\
		"<td>Empire filtering by number of Wins:</td>"\
		"<td><input type=\"checkbox\""
		);
	
	if (bRestrictWins) {
		OutputText (" checked");
	}
	
	OutputText (
		" name=\"RestrictWins\">"\
		" From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictWinsMin\" value=\""
		);
	
	if (pszRestrictWinsMin != NULL) {
		m_pHttpResponse->WriteText (pszRestrictWinsMin);
	} else {
		m_pHttpResponse->WriteText (iRestrictWinsMin);
	}
	
	OutputText (
		"\"> to "\
		"<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictWinsMax\" value=\""
		);
	
	if (pszRestrictWinsMax != NULL) {
		m_pHttpResponse->WriteText (pszRestrictWinsMax);
	} else {
		m_pHttpResponse->WriteText (iRestrictWinsMax);
	}
	
	OutputText (
		"\">"\
		"</td>"\
		"</tr>"
		
		"<tr>"\
		"<td>Empire filtering by IP address:</td>"\
		"<td><select name=\"FilterIP\">"\
		"<option"
		);
	
	if (iFilterIP == 0) {
		OutputText (" selected");
	}
	
	OutputText (
		" value=\"0\">Ignore entry with duplicate IP address</option><option");
	
	if (iFilterIP == GAME_WARN_ON_DUPLICATE_IP_ADDRESS) {
		OutputText (" selected");
	}
	
	OutputText (" value=\"");
	m_pHttpResponse->WriteText (GAME_WARN_ON_DUPLICATE_IP_ADDRESS);
	OutputText ("\">Warn on entry with duplicate IP address</option><option");
	
	if (iFilterIP == GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
		OutputText (" selected");
	}
	
	OutputText (" value=\"");
	m_pHttpResponse->WriteText (GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS);
	OutputText ("\">Reject on entry with duplicate IP address</option><option");
	
	if (iFilterIP == (GAME_WARN_ON_DUPLICATE_IP_ADDRESS | GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS)) {
		OutputText (" selected");
	}
	
	OutputText (" value=\"");
	m_pHttpResponse->WriteText (GAME_WARN_ON_DUPLICATE_IP_ADDRESS | GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS);
	OutputText ("\">Warn and reject on entry with duplicate IP address</option></select></td>"\
		
		"</tr>"\
		
		"<tr>"\
		"<td>Empire filtering by Session Id:</td>"\
		"<td><select name=\"FilterId\">"\
		"<option"
		);
	
	if (iFilterId == 0) {
		OutputText (" selected");
	}
	
	OutputText (" value=\"0\">");
	
	OutputText ("Ignore entry with duplicate Session Id</option><option");
	
	if (iFilterId == GAME_WARN_ON_DUPLICATE_SESSION_ID) {
		OutputText (" selected");
	}
	
	OutputText (" value=\"");
	m_pHttpResponse->WriteText (GAME_WARN_ON_DUPLICATE_SESSION_ID);
	OutputText ("\">Warn on entry with duplicate Session Id</option><option");
	
	if (iFilterId == GAME_BLOCK_ON_DUPLICATE_SESSION_ID) {
		OutputText (" selected");
	}
	
	OutputText (" value=\"");
	m_pHttpResponse->WriteText (GAME_BLOCK_ON_DUPLICATE_SESSION_ID);
	OutputText ("\">Reject on entry with duplicate Session Id</option><option");
	
	if (iFilterId == (GAME_WARN_ON_DUPLICATE_SESSION_ID | GAME_BLOCK_ON_DUPLICATE_SESSION_ID)) {
		OutputText (" selected");
	}
	
	OutputText (" value=\"");
	m_pHttpResponse->WriteText (GAME_WARN_ON_DUPLICATE_SESSION_ID | GAME_BLOCK_ON_DUPLICATE_SESSION_ID);
	OutputText (
		"\">Warn and reject on entry with duplicate Session Id</option></select></td>"\
		
		"</tr>"\

		"<tr>"\
		"<td>Block specific empires:</td>"\
		"<td>"
		);

	pHttpForm = m_pHttpRequest->GetForm ("FilterEmpireName");
	if (pHttpForm != NULL) {

		iNumBlocks = pHttpForm->GetNumForms();

		for (unsigned int i = 0;  i < iNumBlocks; i ++) {
			
			const char* pszName = pHttpForm->GetForm(i)->GetValue();
			if (pszName != NULL && stricmp (pszName, m_vEmpireName.GetCharPtr()) != 0) {

				bool bExists;
				int iEmpireKey;
				Variant vRealName;

				// Make sure empire exists
				iErrCode = g_pGameEngine->DoesEmpireExist (pszName, &bExists, &iEmpireKey, &vRealName);
				if (iErrCode == OK && bExists && iEmpireKey != m_iEmpireKey) {

					Assert (iEmpireKey != NO_KEY);

					sprintf (pszFormIP, "FilterEmpireIP%i", i);
					sprintf (pszFormID, "FilterEmpireID%i", i);

					IHttpForm* pFormIP = m_pHttpRequest->GetForm (pszFormIP);
					IHttpForm* pFormID = m_pHttpRequest->GetForm (pszFormID);
					
					OutputText ("Name: <input type=\"text\" name=\"FilterEmpireName\" size=\"20\" maxlength=\"");				
					m_pHttpResponse->WriteText (MAX_EMPIRE_NAME_LENGTH);
					OutputText ("\" value=\"");
					m_pHttpResponse->WriteText (vRealName.GetCharPtr());

					OutputText ("\"> <input type=\"checkbox\" ");

					if (pFormIP != NULL) {
						OutputText ("checked ");
					}

					OutputText ("name=\"");
					m_pHttpResponse->WriteText (pszFormIP);
					OutputText ("\">Same IP Address <input type=\"checkbox\" ");

					if (pFormID != NULL) {
						OutputText ("checked ");
					}
					
					OutputText ("name=\"");
					m_pHttpResponse->WriteText (pszFormID);
					OutputText ("\">Same Session Id<br>");
				}
			}
		}
	}

	sprintf (pszFormIP, "FilterEmpireIP%i", iNumBlocks);
	sprintf (pszFormID, "FilterEmpireID%i", iNumBlocks);

	OutputText ("Name: <input type=\"text\" name=\"FilterEmpireName\" size=\"20\" maxlength=\"");				
	m_pHttpResponse->WriteText (MAX_EMPIRE_NAME_LENGTH);
	OutputText ("\"> <input type=\"checkbox\" name=\"");
	m_pHttpResponse->WriteText (pszFormIP);
	OutputText ("\">Same IP Address <input type=\"checkbox\" name=\"");
	m_pHttpResponse->WriteText (pszFormID);
	OutputText ("\">Same Session Id&nbsp;&nbsp;");

	WriteButton (BID_BLOCK);

	OutputText (
		"</td></tr>"\
		
		"</table><p>"
		);
	
	return;
	
OnError:
	
	OutputText ("Error ");
	m_pHttpResponse->WriteText (iErrCode);
	OutputText (" occurred while processing this page");
}


int HtmlRenderer::ParseGameConfigurationForms (int iGameClass, const Variant* pvGameClassInfo, int iEmpireKey, 
											   GameOptions* pgoOptions) {
	
	int iErrCode, iTemp, iMaxUpdatesBeforeGameCloses, iGameClassOptions, iMaxNumEmpires;
	bool bFlag;

	const char* pszPassword;
	size_t stLen;
	
	Seconds sUpdatePeriod;
	
	IHttpForm* pHttpForm;
	
	pgoOptions->iOptions = 0;
	pgoOptions->pszPassword = NULL;

	// Check for refresh requests
	if (WasButtonPressed (BID_BLOCK)) {
		return WARNING;
	}
	
	// MaxUpdatesBeforeGameCloses
	iErrCode = g_pGameEngine->GetMaxNumUpdatesBeforeClose (&iMaxUpdatesBeforeGameCloses);
	if (iErrCode != OK) {
		goto OnError;
	}
	
	if (iGameClass == NO_KEY) {

		Assert (pvGameClassInfo != NULL);

		sUpdatePeriod = pvGameClassInfo[SystemGameClassData::NumSecPerUpdate].GetInteger();
		iGameClassOptions = pvGameClassInfo[SystemGameClassData::Options].GetInteger();
		iMaxNumEmpires = pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger();
	
	} else {

		Assert (pvGameClassInfo == NULL);

		iErrCode = g_pGameEngine->GetGameClassUpdatePeriod (iGameClass, &sUpdatePeriod);
		if (iErrCode != OK) {
			goto OnError;
		}

		iErrCode = g_pGameEngine->GetGameClassOptions (iGameClass, &iGameClassOptions);
		if (iErrCode != OK) {
			goto OnError;
		}

		iErrCode = g_pGameEngine->GetMaxNumEmpires (iGameClass, &iMaxNumEmpires);
		if (iErrCode != OK) {
			goto OnError;
		}
	}
	
	// NumUpdatesForClose
	if ((pHttpForm = m_pHttpRequest->GetForm ("NumUpdatesForClose")) == NULL) {
		AddMessage ("Missing NumUpdatesForClose form");
		return ERROR_FAILURE;
	}
	pgoOptions->iNumUpdatesBeforeGameCloses = pHttpForm->GetIntValue();
	
	if (pgoOptions->iNumUpdatesBeforeGameCloses > iMaxUpdatesBeforeGameCloses) {
		AddMessage ("The number of updates before the game closes is too high");
		return ERROR_FAILURE;
	}
	
	// FirstUpdateDelay
	if ((pHttpForm = m_pHttpRequest->GetForm ("HoursUD")) == NULL) {
		AddMessage ("Missing HoursUD form");
		return ERROR_FAILURE;
	}
	pgoOptions->sFirstUpdateDelay = 60 * 60 * pHttpForm->GetIntValue();
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("MinsUD")) == NULL) {
		AddMessage ("Missing MinsUD form");
		return ERROR_FAILURE;
	}
	pgoOptions->sFirstUpdateDelay += 60 * pHttpForm->GetIntValue();
	
	if ((pHttpForm = m_pHttpRequest->GetForm ("SecsUD")) == NULL) {
		AddMessage ("Missing SecsUD form");
		return ERROR_FAILURE;
	}
	pgoOptions->sFirstUpdateDelay += pHttpForm->GetIntValue();

	// NamesListed
	if ((pHttpForm = m_pHttpRequest->GetForm ("NamesListed")) == NULL) {
		AddMessage ("Missing NamesListed form");
		return ERROR_FAILURE;
	}
	if (pHttpForm->GetIntValue() != 0) {
		pgoOptions->iOptions |= GAME_NAMES_LISTED;
	}

	// Spectators
	if ((pHttpForm = m_pHttpRequest->GetForm ("Spectators")) != NULL && 
		pHttpForm->GetIntValue() != 0) {

		if ((iGameClassOptions & EXPOSED_SPECTATORS) != EXPOSED_SPECTATORS) {
			AddMessage ("The game cannot be available to spectators if it does not have exposed maps and diplomacy");
			return ERROR_FAILURE;
		} else {
			pgoOptions->iOptions |= GAME_ALLOW_SPECTATORS;
		}
	}

	// First update delay
	if (pgoOptions->sFirstUpdateDelay < 0) {
		AddMessage ("Invalid first update delay");
		return ERROR_FAILURE;
	}
	
	if (pgoOptions->sFirstUpdateDelay > sUpdatePeriod * 10) {
		AddMessage ("The first update delay is too large");
		return ERROR_FAILURE;
	}
	
	// Bridier
	if ((pHttpForm = m_pHttpRequest->GetForm ("Bridier")) != NULL) {
		
		if (pHttpForm->GetIntValue() == GAME_COUNT_FOR_BRIDIER) {
			
			if (iMaxNumEmpires != 2) {
				AddMessage ("The game cannot count towards Bridier Scoring");
				return ERROR_FAILURE;
			} else {
				pgoOptions->iOptions |= GAME_COUNT_FOR_BRIDIER;
			}
		}
	}
	
	// EnterGameMessage
	if ((pHttpForm = m_pHttpRequest->GetForm ("EnterGameMessage")) == NULL) {
		AddMessage ("Missing EnterGameMessage form");
		return ERROR_FAILURE;
	}
	
	pgoOptions->pszEnterGameMessage = pHttpForm->GetValue();
	if (pgoOptions->pszEnterGameMessage != NULL) {
		
		if (strlen (pgoOptions->pszEnterGameMessage) > MAX_ENTER_GAME_MESSAGE_LENGTH) {
			AddMessage ("The message sent to players entering the game is too long");
			return ERROR_FAILURE;
		}
	}
	
	// Password
	if ((pHttpForm = m_pHttpRequest->GetForm ("GamePassword")) == NULL) {
		AddMessage ("Missing GamePassword form");
		return ERROR_FAILURE;
	}
	
	pszPassword = pHttpForm->GetValue();
	stLen = String::StrLen (pszPassword);
	
	if (stLen > 0) {
		
		if (stLen > MAX_PASSWORD_LENGTH) {
			AddMessage ("The game password was too long");
			return ERROR_FAILURE;
		}
		
		if ((pHttpForm = m_pHttpRequest->GetForm ("GamePassword2")) == NULL) {
			AddMessage ("Missing GamePassword2 form");
			return ERROR_FAILURE;
		}
		
		if (String::StrCmp (pszPassword, pHttpForm->GetValue()) != 0) {
			AddMessage ("The game password was not properly confirmed");
			return ERROR_FAILURE;
		}
		
		// Check password
		if (VerifyPassword (pszPassword) != OK) {
			AddMessage ("The game password contained an invalid character");
			return ERROR_FAILURE;
		}
		
		pgoOptions->pszPassword = pszPassword;
	}
	
	// FilterIP
	if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIP")) == NULL) {
		AddMessage ("Missing FilterIP form");
		return ERROR_FAILURE;
	}
	iTemp = pHttpForm->GetIntValue();
	
	if (((iTemp & ~GAME_WARN_ON_DUPLICATE_IP_ADDRESS) & ~GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS) != 0) {
		AddMessage ("Incorrect FilterIP value");
		return ERROR_FAILURE;
	}
	
	pgoOptions->iOptions |= iTemp;
	
	// FilterId
	if ((pHttpForm = m_pHttpRequest->GetForm ("FilterId")) == NULL) {
		AddMessage ("Missing FilterId form");
		return ERROR_FAILURE;
	}
	iTemp = pHttpForm->GetIntValue();
	
	if (((iTemp & ~GAME_WARN_ON_DUPLICATE_SESSION_ID) & ~GAME_BLOCK_ON_DUPLICATE_SESSION_ID) != 0) {
		AddMessage ("Incorrect FilterId value");
		return ERROR_FAILURE;
	}
	
	pgoOptions->iOptions |= iTemp;
	
	//
	// Score filtering
	//
	
	// Almonaster Score
	if (m_pHttpRequest->GetForm ("RestrictAlmonaster") != NULL) {
		
		float fMin, fMax;
		
		// Min
		if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictAlmonasterMin")) == NULL) {
			AddMessage ("Missing RestrictAlmonasterMin form");
			return ERROR_FAILURE;
		}
		fMin = pHttpForm->GetFloatValue();
		
		if (fMin < ALMONASTER_MIN_SCORE || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
			AddMessage ("Invalid minimum Almonaster Score");
			return ERROR_FAILURE;
		}
		
		if (fMin != ALMONASTER_MIN_SCORE) {
			pgoOptions->iOptions |= GAME_RESTRICT_MIN_ALMONASTER_SCORE;
			pgoOptions->fMinAlmonasterScore = fMin;
		}
		
		// Max
		if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictAlmonasterMax")) == NULL) {
			AddMessage ("Missing RestrictAlmonasterMax form");
			return ERROR_FAILURE;
		}
		
		if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
			
			fMax = pHttpForm->GetFloatValue();
			
			if (fMin > fMax) {
				AddMessage ("Invalid Almonaster Score restrictions");
				return ERROR_FAILURE;
			}
			
			if (fMax > ALMONASTER_MAX_SCORE || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
				AddMessage ("Invalid maximum Almonaster Score");
				return ERROR_FAILURE;
			}
			
			pgoOptions->iOptions |= GAME_RESTRICT_MAX_ALMONASTER_SCORE;
			pgoOptions->fMaxAlmonasterScore = fMax;
		}
	}
	
	// Classic Score
	if (m_pHttpRequest->GetForm ("RestrictClassic") != NULL) {
		
		float fMin, fMax;
		
		// Min
		if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictClassicMin")) == NULL) {
			AddMessage ("Missing RestrictClassicMin form");
			return ERROR_FAILURE;
		}
		
		if (String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
			fMin = CLASSIC_MIN_SCORE;		
		} else {
			
			fMin = pHttpForm->GetFloatValue();
			
			if (fMin < CLASSIC_MIN_SCORE || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
				AddMessage ("Invalid minimum Classic Score");
				return ERROR_FAILURE;
			}
			
			if (fMin != CLASSIC_MIN_SCORE) {
				pgoOptions->iOptions |= GAME_RESTRICT_MIN_CLASSIC_SCORE;
				pgoOptions->fMinClassicScore = fMin;
			}
		}
		
		// Max
		if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictClassicMax")) == NULL) {
			AddMessage ("Missing RestrictClassicMax form");
			return ERROR_FAILURE;
		}
		
		if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
			
			fMax = pHttpForm->GetFloatValue();
			
			if (fMin > fMax) {
				AddMessage ("Invalid Classic Score restrictions");
				return ERROR_FAILURE;
			}
			
			if (fMax > CLASSIC_MAX_SCORE || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
				AddMessage ("Invalid maximum Classic Score");
				return ERROR_FAILURE;
			}
			
			pgoOptions->iOptions |= GAME_RESTRICT_MAX_CLASSIC_SCORE;
			pgoOptions->fMaxClassicScore = fMax;
		}
	}
	
	if (pgoOptions->iOptions & GAME_COUNT_FOR_BRIDIER) {
		
		// Bridier Rank
		if (m_pHttpRequest->GetForm ("RestrictBridierRank") != NULL) {
			
			int iMin, iMax;
			
			// Min
			if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankMin")) == NULL) {
				AddMessage ("Missing RestrictBridierRankMin form");
				return ERROR_FAILURE;
			}
			iMin = pHttpForm->GetIntValue();
			
			if (iMin < BRIDIER_MIN_RANK || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
				AddMessage ("Invalid minimum Bridier Rank");
				return ERROR_FAILURE;
			}
			
			if (iMin != BRIDIER_MIN_RANK) {
				pgoOptions->iOptions |= GAME_RESTRICT_MIN_BRIDIER_RANK;
				pgoOptions->iMinBridierRank = iMin;
			}

			// Max
			if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankMax")) == NULL) {
				AddMessage ("Missing RestrictBridierRankMax form");
				return ERROR_FAILURE;
			}
			
			if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
				
				iMax = pHttpForm->GetIntValue();
				
				if (iMin > iMax) {
					AddMessage ("Invalid Bridier Rank restrictions");
					return ERROR_FAILURE;
				}
				
				if (iMax > BRIDIER_MAX_RANK || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
					AddMessage ("Invalid maximum Bridier Rank");
					return ERROR_FAILURE;
				}
				
				pgoOptions->iOptions |= GAME_RESTRICT_MAX_BRIDIER_RANK;
				pgoOptions->iMaxBridierRank = iMax;
			}
		}
		
		// Bridier Index
		if (m_pHttpRequest->GetForm ("RestrictBridierIndex") != NULL) {
			
			int iMin, iMax;
			
			// Min
			if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierIndexMin")) == NULL) {
				AddMessage ("Missing RestrictBridierIndexMin form");
				return ERROR_FAILURE;
			}
			iMin = pHttpForm->GetIntValue();
			
			if (iMin < BRIDIER_MIN_INDEX || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
				AddMessage ("Invalid minimum Bridier Index");
				return ERROR_FAILURE;
			}
			
			if (iMin != BRIDIER_MIN_INDEX) {
				pgoOptions->iOptions |= GAME_RESTRICT_MIN_BRIDIER_INDEX;
				pgoOptions->iMinBridierIndex = iMin;
			}
			
			// Max
			if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierIndexMax")) == NULL) {
				AddMessage ("Missing RestrictBridierIndexMax form");
				return ERROR_FAILURE;
			}
			
			if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
				
				iMax = pHttpForm->GetIntValue();
				
				if (iMin > iMax) {
					AddMessage ("Invalid Bridier Index restrictions");
					return ERROR_FAILURE;
				}
				
				if (iMax > BRIDIER_MAX_INDEX || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
					AddMessage ("Invalid maximum Bridier Index");
					return ERROR_FAILURE;
				}
				
				pgoOptions->iOptions |= GAME_RESTRICT_MAX_BRIDIER_INDEX;
				pgoOptions->iMaxBridierIndex = iMax;
			}
		}
		
		// Bridier RankGain
		if (m_pHttpRequest->GetForm ("RestrictBridierRankGain") != NULL) {
			
			int iMin, iMax;
			
			// Min
			if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankGainMin")) == NULL) {
				AddMessage ("Missing RestrictBridierRankGainMin form");
				return ERROR_FAILURE;
			}
			iMin = pHttpForm->GetIntValue();
			
			if (iMin < BRIDIER_MIN_RANK_GAIN || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
				AddMessage ("Invalid minimum Bridier Rank Gain");
				return ERROR_FAILURE;
			}
			
			if (iMin != BRIDIER_MIN_RANK_GAIN) {
				pgoOptions->iOptions |= GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN;
				pgoOptions->iMinBridierRankGain = iMin;
			}
			
			// Max
			if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankGainMax")) == NULL) {
				AddMessage ("Missing RestrictBridierRankGainMax form");
				return ERROR_FAILURE;
			}
			
			if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
				
				iMax = pHttpForm->GetIntValue();
				
				if (iMin > iMax) {
					AddMessage ("Invalid Bridier Rank Gain restrictions");
					return ERROR_FAILURE;
				}
				
				if (iMax > BRIDIER_MAX_RANK_GAIN || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
					AddMessage ("Invalid maximum Bridier Rank Gain");
					return ERROR_FAILURE;
				}
				
				pgoOptions->iOptions |= GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN;
				pgoOptions->iMaxBridierRankGain = iMax;
			}
		}
	}
	
	// Wins
	if (m_pHttpRequest->GetForm ("RestrictWins") != NULL) {
		
		int iMin, iMax;
		
		// Min
		if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictWinsMin")) == NULL) {
			AddMessage ("Missing RestrictWinsMin form");
			return ERROR_FAILURE;
		}
		iMin = pHttpForm->GetIntValue();
		
		if (iMin < MIN_NUM_WINS || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
			AddMessage ("Invalid minimum Wins");
			return ERROR_FAILURE;
		}
		
		if (iMin != MIN_NUM_WINS) {
			pgoOptions->iOptions |= GAME_RESTRICT_MIN_WINS;
			pgoOptions->iMinWins = iMin;
		}
		
		// Max
		if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictWinsMax")) == NULL) {
			AddMessage ("Missing RestrictWinsMax form");
			return ERROR_FAILURE;
		}
		
		if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
			
			iMax = pHttpForm->GetIntValue();
			
			if (iMin > iMax) {
				AddMessage ("Invalid Wins restrictions");
				return ERROR_FAILURE;
			}
			
			if (iMax > MAX_NUM_WINS || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
				AddMessage ("Invalid maximum Wins");
				return ERROR_FAILURE;
			}
			
			pgoOptions->iOptions |= GAME_RESTRICT_MAX_WINS;
			pgoOptions->iMaxWins = iMax;
		}
	}

	// Handle specific empire filtering
	pHttpForm = m_pHttpRequest->GetForm ("FilterEmpireName");
	if (pHttpForm != NULL) {

		unsigned int i, j, iNumBlocks = pHttpForm->GetNumForms(), iNumRealBlocks = 0;
		if (iNumBlocks > 1 || iNumBlocks == 1 && pHttpForm->GetValue() != NULL) {

			pgoOptions->pSecurity = new GameSecurityEntry [iNumBlocks];
			if (pgoOptions->pSecurity == NULL) {
				AddMessage ("The server is out of memory");
				return ERROR_OUT_OF_MEMORY;
			}

#ifdef _DEBUG
			memset (pgoOptions->pSecurity, 0xde, iNumBlocks * sizeof (GameSecurityEntry));
#endif
			for (i = 0;  i < iNumBlocks; i ++) {
			
				const char* pszName = pHttpForm->GetForm(i)->GetValue();
				if (pszName != NULL && stricmp (pszName, m_vEmpireName.GetCharPtr()) != 0) {

					int iEmpireKey;

					// Make sure empire exists
					iErrCode = g_pGameEngine->DoesEmpireExist (pszName, &bFlag, &iEmpireKey, NULL);
					if (iErrCode == OK && bFlag && iEmpireKey != m_iEmpireKey) {

						Assert (iEmpireKey != NO_KEY);

						bool bAlready = false;

						char pszFormIP [64];
						char pszFormID [64];

						sprintf (pszFormIP, "FilterEmpireIP%i", i);
						sprintf (pszFormID, "FilterEmpireID%i", i);

						IHttpForm* pFormIP = m_pHttpRequest->GetForm (pszFormIP);
						IHttpForm* pFormID = m_pHttpRequest->GetForm (pszFormID);

						int iOptions = 0;

						if (pFormIP != NULL) {
							iOptions |= GAME_SECURITY_CHECK_IPADDRESS;
						}

						if (pFormID != NULL) {
							iOptions |= GAME_SECURITY_CHECK_SESSIONID;
						}

						// Make sure name hasn't been used already
						for (j = 0; j < iNumRealBlocks; j ++) {

							if (stricmp (pgoOptions->pSecurity[j].pszEmpireName, pszName) == 0) {
								bAlready = true;
								break;
							}
						}

						if (!bAlready) {
						
							pgoOptions->pSecurity[iNumRealBlocks].iEmpireKey = iEmpireKey;
							pgoOptions->pSecurity[iNumRealBlocks].iOptions = iOptions;
							pgoOptions->pSecurity[iNumRealBlocks].pszEmpireName = pszName;

							iNumRealBlocks ++;
						}
					}
				}
			}

			if (iNumRealBlocks > 0) {
				pgoOptions->iOptions |= GAME_ENFORCE_SECURITY;
				pgoOptions->iNumSecurityEntries = iNumRealBlocks;
			}
		}
	}

/*/ Finally, do a quick access check to make sure the game can be created by this empire
	// When a non-null options structure is passed in, the code won't blindly approve gameclass
	// owners, and will reject out of line parameters like score constraints that exclude the creator

	iErrCode = g_pGameEngine->GameAccessCheck (iGameClass, 0, iEmpireKey, pgoOptions, ENTER_GAME, &bFlag);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}
	
	if (!bFlag) {
		AddMessage ("You do not have permission to enter the game");
		return ERROR_ACCESS_DENIED;
	}
*/
	return OK;
	
OnError:
	
	AddMessage ("Error reading gameclass data");
	
	return iErrCode;
}




//
// Events and statistics
//

AlmonasterStatistics HtmlRenderer::m_sStats;

int HtmlRenderer::OnCreateEmpire (int iEmpireKey) {
	
	// Stats
	Algorithm::AtomicIncrement (&m_sStats.EmpiresCreated);
	
	return OK;
}

int HtmlRenderer::OnDeleteEmpire (int iEmpireKey) {
	
	char pszDestFileName[OS::MaxFileNameLength];
	
	sprintf (
		pszDestFileName, 
		"%s/" BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
		g_pszResourceDir,
		iEmpireKey
		);
	
	// Just in case...
	g_pFileCache->ReleaseFile (pszDestFileName);
	
	// Attempt to delete an uploaded file
	File::DeleteFile (pszDestFileName);
	
	// Stats
	Algorithm::AtomicIncrement (&m_sStats.EmpiresDeleted);
	
	return OK;
}

int HtmlRenderer::OnLoginEmpire (int iEmpireKey) {
	
	Algorithm::AtomicIncrement (&m_sStats.Logins);
	
	return OK;
}

int HtmlRenderer::OnCreateGame (int iGameClass, int iGameNumber) {
	
	Algorithm::AtomicIncrement (&m_sStats.GamesStarted);
	
	return OK;
}

int HtmlRenderer::OnCleanupGame (int iGameClass, int iGameNumber) {
	
	Algorithm::AtomicIncrement (&m_sStats.GamesEnded);
	
	return OK;
}

int HtmlRenderer::OnPageRender (MilliSeconds msTime) {
	
	Algorithm::AtomicIncrement (&m_sStats.NumPageScriptRenders);
	Algorithm::AtomicIncrement (&m_sStats.TotalScriptTime, msTime);
	
	return OK;
}

void HtmlRenderer::ZeroStatistics() {
	
	memset (&m_sStats, 0, sizeof (AlmonasterStatistics));
}

void HtmlRenderer::InitGameOptions (GameOptions* pgoOptions) {

	memset (pgoOptions, 0, sizeof (GameOptions));
}

void HtmlRenderer::ClearGameOptions (GameOptions* pgoOptions) {

	if (pgoOptions->pSecurity != NULL) {
		delete [] pgoOptions->pSecurity;
	}
}

static const char* const pszEmpireInfoHeadersAdmin[] = {
	"Name",
	"Alien",
	"Econ",
	"Mil",
	"Tech Level",
	"Planets",
	"Ships",
	"At war",
	"At truce",
	"At trade",
	"At alliance",
	"Pause",
	"Last Access",
	"Ready for Update",
};

static const char* const pszEmpireInfoHeadersSpectator[] = {
	"Name",
	"Alien",
	"Econ",
	"Mil",
	"Planets",
	"Pause",
	"Last Access",
	"Ready for Update",
};

void HtmlRenderer::RenderEmpireInformation (int iGameClass, int iGameNumber, bool bAdmin) {

	int i, iErrCode, iNumEmpires, iValue, iFoeKey, iWar, iTruce, iTrade, iAlliance, iUnmet;
	unsigned int iKey;
	float fValue;
	bool bUpdated;

	Variant* pvEmpireKey = NULL, vValue;
	UTCTime tCurrentTime, tValue;

	String strWar, strTruce, strTrade, strAlliance;

	IDatabase* pDatabase = g_pGameEngine->GetDatabase();
	IReadTable* pGameEmpireData = NULL, * pGameEmpireDip = NULL;

	const char* pszTableColor = m_vTableColor.GetCharPtr(), * const * pszHeaders;

	char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];
	char strGameEmpireData [256], pszGameEmpireShips [256], pszGameEmpireDip [256];

	Time::GetTime (&tCurrentTime);

	iErrCode = g_pGameEngine->GetEmpiresInGame (iGameClass, iGameNumber, &pvEmpireKey, &iNumEmpires);
	if (iErrCode != OK) {
		goto Cleanup;
	}

	OutputText ("<table width=\"90%\"><tr>");

	if (bAdmin) {

		pszHeaders = pszEmpireInfoHeadersAdmin;
		iValue = sizeof (pszEmpireInfoHeadersAdmin) / sizeof (char*);

	} else {

		pszHeaders = pszEmpireInfoHeadersSpectator;
		iValue = sizeof (pszEmpireInfoHeadersSpectator) / sizeof (char*);
	}

	for (i = 0; i < iValue; i ++) {
		
		OutputText ("<th bgcolor=\"#");
		m_pHttpResponse->WriteText (pszTableColor);
		OutputText ("\">");
		m_pHttpResponse->WriteText (pszHeaders[i]);
		OutputText ("</th>");
	}

	OutputText ("</tr>");

	NotifyProfileLink();

	for (i = 0; i < iNumEmpires; i ++) {

		GET_GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

		iErrCode = pDatabase->GetTableForReading (strGameEmpireData, &pGameEmpireData);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		OutputText ("<tr>");

		// Name
		iErrCode = g_pGameEngine->GetEmpireName (pvEmpireKey[i].GetInteger(), &vValue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		OutputText ("<td align=\"center\">");
		m_pHttpResponse->WriteText (vValue.GetCharPtr());
		OutputText ("</td>");

		// Alien
		iErrCode = g_pGameEngine->GetEmpireAlienKey (pvEmpireKey[i].GetInteger(), &iValue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		OutputText ("<td align=\"center\">");

		sprintf (pszProfile, "View the profile of %s", vValue.GetCharPtr());

		WriteProfileAlienString (
			iValue,
			pvEmpireKey[i].GetInteger(),
			vValue.GetCharPtr(),
			0,
			"ProfileLink",
			pszProfile,
			false,
			true
			);

		OutputText ("</td>");

		// Econ
		iErrCode = pGameEmpireData->ReadData (GameEmpireData::Econ, &iValue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		OutputText ("<td align=\"center\">");
		m_pHttpResponse->WriteText (iValue);
		OutputText ("</td>");

		// Mil
		iErrCode = pGameEmpireData->ReadData (GameEmpireData::Mil, &fValue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		OutputText ("<td align=\"center\">");
		m_pHttpResponse->WriteText (g_pGameEngine->GetMilitaryValue (fValue));
		OutputText ("</td>");

		if (bAdmin) {

			// TechLevel
			iErrCode = pGameEmpireData->ReadData (GameEmpireData::TechLevel, &fValue);
			if (iErrCode != OK) {
				goto Cleanup;
			}

			OutputText ("<td align=\"center\">");
			m_pHttpResponse->WriteText (fValue);
			OutputText ("</td>");
		}

		// Planets
		iErrCode = pGameEmpireData->ReadData (GameEmpireData::NumPlanets, &iValue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		OutputText ("<td align=\"center\">");
		m_pHttpResponse->WriteText (iValue);
		OutputText ("</td>");

		if (bAdmin) {

			// Ships
			GET_GAME_EMPIRE_SHIPS (pszGameEmpireShips, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			GET_GAME_EMPIRE_DIPLOMACY (pszGameEmpireDip, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			iErrCode = pDatabase->GetNumRows (pszGameEmpireShips, (unsigned int*) &iValue);
			if (iErrCode != OK) {
				goto Cleanup;
			}

			OutputText ("<td align=\"center\">");
			m_pHttpResponse->WriteText (iValue);
			OutputText ("</td>");

			iErrCode = pDatabase->GetTableForReading (pszGameEmpireDip, &pGameEmpireDip);
			if (iErrCode != OK) {
				goto Cleanup;
			}

			iKey = NO_KEY;
			iWar = iTruce = iTrade = iAlliance = iUnmet = 0;
			
			strWar.Clear();
			strTruce.Clear();
			strTrade.Clear();
			strAlliance.Clear();

			while (true) {

				String* pStr = &strWar;
				Variant vName;

				iErrCode = pGameEmpireDip->GetNextKey (iKey, &iKey);
				if (iErrCode != OK) {
					if (iErrCode == ERROR_DATA_NOT_FOUND) {
						break;
					}
					Assert (false);
					goto Cleanup;
				}

				iErrCode = pGameEmpireDip->ReadData (iKey, GameEmpireDiplomacy::CurrentStatus, &iValue);
				if (iErrCode != OK) {
					goto Cleanup;
				}

				iErrCode = pGameEmpireDip->ReadData (iKey, GameEmpireDiplomacy::EmpireKey, &iFoeKey);
				if (iErrCode != OK) {
					goto Cleanup;
				}

				iErrCode = g_pGameEngine->GetEmpireName (iFoeKey, &vName);
				if (iErrCode != OK) {
					goto Cleanup;
				}

				switch (iValue) {

				case WAR:
					pStr = &strWar;
					iWar ++;
					break;

				case TRUCE:
					pStr = &strTruce;
					iTruce ++;
					break;

				case TRADE:
					pStr = &strTrade;
					iTrade ++;
					break;

				case ALLIANCE:
					pStr = &strAlliance;
					iAlliance ++;
					break;

				default:
					Assert (false);
					break;
				}

				if (!pStr->IsBlank()) {
					*pStr += ", ";
				}
				*pStr += vName.GetCharPtr();
			}

			pGameEmpireDip->Release();
			pGameEmpireDip = NULL;

			iUnmet = iNumEmpires - iWar - iTruce - iTrade - iAlliance - 1;

			// War
			OutputText ("<td align=\"center\">");

			if (iWar > 0) {
				m_pHttpResponse->WriteText (iWar);
				m_pHttpResponse->WriteText (" (");
				m_pHttpResponse->WriteText (strWar, strWar.GetLength());
				m_pHttpResponse->WriteText (")");

				if (iUnmet > 0) {
					m_pHttpResponse->WriteText (", ");
				}
			}

			if (iUnmet > 0) {
				m_pHttpResponse->WriteText (iUnmet);
				m_pHttpResponse->WriteText (" unmet");
			}
			else if (iWar == 0) {
				m_pHttpResponse->WriteText ("0");
			}

			OutputText ("</td>");

			// Truce
			OutputText ("<td align=\"center\">");
			m_pHttpResponse->WriteText (iTruce);
			if (iTruce > 0) {
				m_pHttpResponse->WriteText (" (");
				m_pHttpResponse->WriteText (strTruce, strTruce.GetLength());
				m_pHttpResponse->WriteText (")");
			}
			OutputText ("</td>");

			// Trade
			OutputText ("<td align=\"center\">");
			m_pHttpResponse->WriteText (iTrade);
			if (iTrade > 0) {
				m_pHttpResponse->WriteText (" (");
				m_pHttpResponse->WriteText (strTrade, strTrade.GetLength());
				m_pHttpResponse->WriteText (")");
			}
			OutputText ("</td>");

			// Alliance
			OutputText ("<td align=\"center\">");
			m_pHttpResponse->WriteText (iAlliance);
			if (iAlliance > 0) {
				m_pHttpResponse->WriteText (" (");
				m_pHttpResponse->WriteText (strAlliance, strAlliance.GetLength());
				m_pHttpResponse->WriteText (")");
			}
			OutputText ("</td>");
		}

		// Paused
		iErrCode = pGameEmpireData->ReadData (GameEmpireData::Options, &iValue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		OutputText ("<td align=\"center\">");
		m_pHttpResponse->WriteText ((iValue & REQUEST_PAUSED) != 0 ? "Yes" : "No");
		OutputText ("</td>");

		bUpdated = iValue & UPDATED;

		// LastLogin, idle
		iErrCode = pGameEmpireData->ReadData (GameEmpireData::LastLogin, &tValue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		OutputText ("<td align=\"center\">");
		WriteTime (Time::GetSecondDifference (tCurrentTime, tValue));
		OutputText (" ago");

		iErrCode = pGameEmpireData->ReadData (GameEmpireData::NumUpdatesIdle, &iValue);
		if (iErrCode != OK) {
			goto Cleanup;
		}

		if (iValue > 0) {

			OutputText ("<br>(<strong>");
			m_pHttpResponse->WriteText (iValue);
			OutputText ("</strong> update");

			if (iValue != 1) {
				OutputText ("s");
			}
			OutputText (" idle)");
		}
		OutputText ("</td>");

		// Updated
		OutputText ("<td align=\"center\">");
		m_pHttpResponse->WriteText (bUpdated ? "Yes" : "No");
		OutputText ("</td>");

		pGameEmpireData->Release();
		pGameEmpireData = NULL;

		OutputText ("</tr>");
	}

	OutputText ("</table><p>");

Cleanup:

	if (pGameEmpireData != NULL) {
		pGameEmpireData->Release();
	}

	if (pGameEmpireDip != NULL) {
		pGameEmpireDip->Release();
	}

	if (pDatabase != NULL) {
		pDatabase->Release();
	}

	if (pvEmpireKey != NULL) {
		g_pGameEngine->FreeData (pvEmpireKey);
	}
}