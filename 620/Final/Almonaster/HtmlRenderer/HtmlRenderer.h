//
// GameEngine.dll:  a component of Almonaster 2.0
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

#ifndef _HtmlRenderer_H_
#define _HtmlRenderer_H_

#include "Alajar.h"

#include "../Almonaster.h"

#include "../GameEngine/GameEngine.h"
#include "../Chatroom/CChatroom.h"

#include "Osal/String.h"
#include "Osal/Time.h"

#define OutputText(string) m_pHttpResponse->WriteText (string, sizeof(string) - 1);

struct PartialMapInfo {
    unsigned int iCenterKey;
    unsigned int iXRadius;
    unsigned int iYRadius;
    bool bDontShowPartialOptions;
};

struct ShipsInMapScreen {
    unsigned int iPlanetKey;
    unsigned int iCurrentShip;
    unsigned int iCurrentFleet;
};

struct AlmonasterStatistics {

    // Empires and games
    unsigned int Logins;
    unsigned int EmpiresCreated;
    unsigned int EmpiresDeleted;
    unsigned int GamesStarted;
    unsigned int GamesEnded;

    // Server data
    unsigned int NumPageScriptRenders;
    unsigned int TotalScriptTime;
};

// Mini-maps
enum IconIndicator {
    ICON_NONE,
    ICON_INDEPENDENT,
    ICON_LIVEPLANET,
    ICON_DEADPLANET,
    ICON_EMPIREPLANET,
};

struct MiniMapEntry {
    IconIndicator iiIcon;
    unsigned int iOwnerKey;
    unsigned int iAlienKey;
    unsigned int iPlanetKey;
    unsigned int iPlanetProxyKey;
};

// Page id's
enum PageId {
    MIN_PAGE_ID = 0,
    ACTIVE_GAME_LIST = 1,
    LOGIN = 2,
    NEW_EMPIRE = 3,
    OPEN_GAME_LIST = 4,
    SYSTEM_GAME_LIST = 5,
    PROFILE_EDITOR = 6,
    TOP_LISTS = 7,
    PROFILE_VIEWER = 8,
    SERVER_ADMINISTRATOR = 9,
    EMPIRE_ADMINISTRATOR = 10,
    GAME_ADMINISTRATOR = 11,
    THEME_ADMINISTRATOR = 12,
    PERSONAL_GAME_CLASSES = 13,
    CHATROOM = 14,
    SYSTEM_SERVER_INFORMATION = 15,
    SYSTEM_DOCUMENTATION = 16,
    SYSTEM_NEWS = 17,
    INFO = 18,
    TECH = 19,
    DIPLOMACY = 20,
    MAP = 21,
    PLANETS = 22,
    OPTIONS = 23,
    BUILD = 24,
    SHIPS = 25,
    GAME_SERVER_INFORMATION = 26,
    GAME_DOCUMENTATION = 27,
    GAME_NEWS = 28,
    GAME_PROFILE_VIEWER = 29,
    GAME_CONTRIBUTIONS = 30,
    GAME_CREDITS = 31,
    QUIT = 32,
    LATEST_NUKES = 33,
    SPECTATOR_GAMES = 34,
    SYSTEM_CONTRIBUTIONS = 35,
    SYSTEM_CREDITS = 36,
    LATEST_GAMES = 37,
    TOURNAMENT_ADMINISTRATOR = 38,
    PERSONAL_TOURNAMENTS = 39,
    TOURNAMENTS = 40,
    MAX_PAGE_ID = 41,
};

#define FIRST_GAME_PAGE                     INFO

#define EMPIRE_NAME_HASH_LIMIT              (0x00ffffff)

#define MAX_HTML_TIME_LENGTH                512

#define FIRST_VALID_PASSWORD_CHAR           ('#')
#define LAST_VALID_PASSWORD_CHAR            ('~')

#define FIRST_VALID_EMPIRE_NAME_CHAR        (' ')
#define LAST_VALID_EMPIRE_NAME_CHAR         ('~')

#define FIRST_VALID_SHIP_OR_FLEET_NAME_CHAR (' ')
#define LAST_VALID_SHIP_OR_FLEET_CHAR       ('~')

#define FIRST_VALID_CATEGORY_NAME_CHAR      (' ')
#define LAST_VALID_CATEGORY_NAME_CHAR       ('~')

#define FIRST_VALID_SERVER_NAME_CHAR        (' ')
#define LAST_VALID_SERVER_NAME_CHAR         ('~')

#define INVALID_PASSWORD_STRING             "!!!!!!!!!!"

#define UNSTARTED_GAMEPAGE_REFRESH_SEC      (120)

enum ButtonId {
    BID_FIRST,
    BID_ACTIVEGAMELIST,
    BID_ADMINISTERGAME,
    BID_ADVANCEDSEARCH,
    BID_ALL,
    BID_ALMONASTERSCORE,
    BID_ATTACK,
    BID_BACKUP,
    BID_BLANKEMPIRESTATISTICS,
    BID_BUILD,
    BID_CANCEL,
    BID_CANCELALLBUILDS,
    BID_CHANGEEMPIRESPASSWORD,
    BID_CHANGEPASSWORD,
    BID_CHATROOM,
    BID_CHOOSE,
    BID_CLASSICSCORE,
    BID_CLOAKER,
    BID_COLONY,
    BID_CREATEALIENICON,
    BID_CREATEEMPIRE,
    BID_CREATENEWGAMECLASS,
    BID_CREATENEWSUPERCLASS,
    BID_CREATENEWTHEME,
    BID_DELETEALIENICON,
    BID_DELETEBACKUP,
    BID_DELETEEMPIRE,
    BID_DELETEGAMECLASS,
    BID_DELETESUPERCLASS,
    BID_DELETETHEME,
    BID_DIPLOMACY,
    BID_DOOMSDAY,
    BID_EMPIRE,
    BID_EMPIREADMINISTRATOR,
    BID_ENDTURN,
    BID_ENGINEER,
    BID_ENTERGAME,
    BID_EXIT,
    BID_DOCUMENTATION,
    BID_FLUSH,
    BID_FORCEUPDATE,
    BID_GAMEADMINISTRATOR,
    BID_HALTGAMECLASS,
    BID_INFO,
    BID_KILLGAME,
    BID_LEAVETHECHATROOM,
    BID_LOGIN,
    BID_MAP,
    BID_MINEFIELD,
    BID_MINESWEEPER,
    BID_OBLITERATEEMPIRE,
    BID_OPENGAMELIST,
    BID_OPTIONS,
    BID_PAUSEALLGAMES,
    BID_PAUSEGAME,
    BID_PERSONALGAMECLASSES,
    BID_PLANETS,
    BID_PROFILEEDITOR,
    BID_PROFILEVIEWER,
    BID_PURGE,
    BID_QUIT,
    BID_REFRESHMESSAGES,
    BID_RESTARTALMONASTER,
    BID_RESTARTSERVER,
    BID_RESTOREBACKUP,
    BID_RESTOREEMPIRE,
    BID_SATELLITE,
    BID_SCIENCE,
    BID_SEARCH,
    BID_SELECTION,
    BID_SENDMESSAGE,
    BID_SERVERADMINISTRATOR,
    BID_SERVERNEWS,
    BID_SERVERINFORMATION,
    BID_SHIPS,
    BID_SHUTDOWNSERVER,
    BID_SPEAK,
    BID_STARGATE,
    BID_STARTGAME,
    BID_SYSTEM,
    BID_SYSTEMGAMELIST,
    BID_TECH,
    BID_TERRAFORMER,
    BID_THEMEADMINISTRATOR,
    BID_TOPLISTS,
    BID_TROOPSHIP,
    BID_UNDELETEEMPIRE,
    BID_UNDELETEGAMECLASS,
    BID_UNENDTURN,
    BID_UNHALTGAMECLASS,
    BID_UNPAUSEALLGAMES,
    BID_UNPAUSEGAME,
    BID_VIEWEMPIRESGAMECLASSES,
    BID_VIEWEMPIRESNUKEHISTORY,
    BID_VIEWGAMEINFORMATION,
    BID_VIEWMAP,
    BID_VIEWMESSAGES,
    BID_VIEWPROFILE,
    BID_SEARCHIPADDRESSES,
    BID_SEARCHSESSIONIDS,
    BID_VIEWEMPIREINFORMATION,
    BID_RESIGN,
    BID_STARTCUSTOMGAME,
    BID_RENAMESUPERCLASS,
    BID_CLEARMESSAGES,
    BID_SURRENDER,
    BID_CARRIER,
    BID_BUILDER,
    BID_MORPHER,
    BID_JUMPGATE,
    BID_LATESTNUKES,
    BID_BRIDIERSCORE,
    BID_BRIDIERSCOREESTABLISHED,
    BID_SPECTATORGAMES,
    BID_BLOCK,
    BID_RESET,
    BID_CONTRIBUTIONS,
    BID_CREDITS,
    BID_LATESTGAMES,
    BID_TOURNAMENTADMINISTRATOR,
    BID_PERSONALTOURNAMENTS,
    BID_CREATENEWTOURNAMENT,
    BID_DELETETOURNAMENT,
    BID_ADMINISTERTOURNAMENT,
    BID_INVITEEMPIRE,
    BID_CREATETEAM,
    BID_VIEWTOURNAMENTINFORMATION,
    BID_DECLINE,
    BID_ACCEPT,
    BID_ADMINISTERTEAM,
    BID_DELETETEAM,
    BID_ADDEMPIRE,
    BID_UPDATE,
    BID_CHOOSEICON,
    BID_CHOOSETHEME,
    BID_TOURNAMENTS,
    BID_JOIN,
    BID_VIEWEMPIRESTOURNAMENTS,
    BID_REBUILD,
    BID_LOOKUP,
    BID_VIEWMINIMAP,
    BID_LAST
};

extern const char* ButtonText[];
extern const char* ButtonName[];
extern const char* ButtonImageName[];
extern const char* ButtonFileName[];
extern const ButtonId PageButtonId[];
extern const char* PageName[];

enum SearchFieldType {
    SEARCHFIELD_INTEGER,
    SEARCHFIELD_INTEGER64,
    SEARCHFIELD_FLOAT,
    SEARCHFIELD_DATE,
    SEARCHFIELD_STRING,
    SEARCHFIELD_PRIVILEGE,
};

struct SearchField {
    bool bCheckedByDefault;
    const char* pszName;
    const char* pszInputCheckBox;
    const char* pszInput1;
    const char* pszInput2;
    unsigned int iSystemEmpireDataColumn;
    SearchFieldType sftType;
    Privilege prvMinPriv;
};

extern const SearchField g_AdvancedSearchFields[];

#define MAX_NUM_SEARCH_COLUMNS 29

//
// Globals
//

class HtmlRenderer {
protected:

    // Data
    PageId m_pgPageId;

    IHttpRequest* m_pHttpRequest;
    IHttpResponse* m_pHttpResponse;

    Variant m_vEmpireName;
    Variant m_vPassword;
    Variant m_vPreviousIPAddress;

    int64 m_i64SecretKey;
    UTCTime m_tOldSalt;
    UTCTime m_tNewSalt;

    String m_strMessage;

    unsigned int m_iEmpireKey;
    unsigned int m_iGameClass;
    int m_iGameNumber;
    unsigned int m_iButtonKey;
    unsigned int m_iBackgroundKey;
    unsigned int m_iSeparatorKey;
    int m_iPrivilege;
    unsigned int m_iAlienKey;
    unsigned int m_iThemeKey;
    int m_iGameState;
    int m_iGameRatios;
    int m_iReserved;

    int m_iSystemOptions;
    int m_iSystemOptions2;
    int m_iGameOptions;

    Variant m_vTableColor;
    Variant m_vTextColor;
    Variant m_vGoodColor;
    Variant m_vBadColor;
    Variant m_vPrivateMessageColor;
    Variant m_vBroadcastMessageColor;
    Variant m_vLocalPath;

    char m_pszGameClassName [MAX_FULL_GAME_CLASS_NAME_LENGTH];

    bool m_bRedirection;
    bool m_bRepeatedButtons;
    bool m_bTimeDisplay;
    bool m_bNotifiedProfileLink;
    bool m_bNotifiedTournamentInvitation;
    bool m_bNotifiedTournamentJoinRequest;
    bool m_bOwnPost;
    bool m_bLoggedIn;
    bool m_bAuthenticated;

    Timer m_tmTimer;
    Seconds m_sSecondsUntil;
    Seconds m_sSecondsSince;

    int64 m_i64SessionId;

    // Submission control
    int m_iNumOldUpdates;
    int m_iNumNewUpdates;

    // Methods

    void WriteGameListHeader (const char** ppszHeaders, size_t stNumHeaders, const char* pszTableColor);

    void AddEmpiresInGame (int iNumActiveEmpires, const char* pszEmpires, int iMinEmpires, int iMaxEmpires);

    int AddGameClassDescription (int iWhichList, const Variant* pvGameClassInfo, 
        int iGameClass, int iGameNumber, const char* pszEmpiresInGame, int iNumEmpiresInGame, bool bAdmin,
        bool bSpectators);

    int WriteInPlayGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, 
        bool bAdmin, bool bSpectator);

    void AddOptions (int iWhichList, const Variant* pvGameClassInfo, int iGameOptions);
    void AddResources (const Variant* pvGameClassInfo);
    void AddDiplomacy (const Variant* pvGameClassInfo);

    void AddTechList (int iTechs);

    void AddBridier (int iGameOptions, const Variant* pvMin, const Variant* pvMax);
    void AddScore (int iGameOptions, const Variant* pvMin, const Variant* pvMax);
    void AddSecurity (int iGameOptions);

    void AddBridierGame (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, int iGameOptions, bool bDisplayGainLoss);

    int PostGamePageInformation();

    void WriteGameButtons();
    void WriteGameNextUpdateString();

    bool UpdateCachedFile (const char* pszFileName, const char* pszText);

    void RenderSearchField (const SearchField& sfField, bool fAdvanced);
    void RenderDateField (const char* pszField);
    bool ParseDateField (const char* pszField, UTCTime* ptTime);

    static ReadWriteLock ms_mNewsFileLock;
    static ReadWriteLock ms_mIntroUpperFileLock;
    static ReadWriteLock ms_mIntroLowerFileLock;

public:

    // Stats
    static AlmonasterStatistics m_sStats;

    HtmlRenderer (PageId pageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);
    static int Initialize();

    int Render();
    int Redirect (PageId pageId);

    void ShutdownServer();
    void RestartServer();
    void RestartAlmonaster();

    bool IsGamePage (PageId pageId);

    static int OnCreateEmpire (int iEmpireKey);
    static int OnDeleteEmpire (int iEmpireKey);
    static int OnLoginEmpire (int iEmpireKey);
    static int OnCreateGame (int iGameClass, int iGameNumber);
    static int OnCleanupGame (int iGameClass, int iGameNumber);

    static int OnDeleteTournament (unsigned int iTournamentKey);
    static int OnDeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey);

    static int OnPageRender (MilliSeconds msTime);

    static void ZeroStatistics();

    void WriteFormattedMessage (const char* pszText);
    
    int GetUIData (int iThemeKey);
    int GetTextColorData (int iEmpireColorKey);

    bool IsColor (const char* pszNewValue);

    void WriteBackgroundImageSrc (int iThemeKey);
    void WriteLivePlanetImageSrc (int iThemeKey);
    void WriteDeadPlanetImageSrc (int iThemeKey);
    void WriteSeparatorSrc (int iThemeKey);

    void WriteHorzSrc (int iThemeKey);
    void WriteVertSrc (int iThemeKey);

    int GetHorzString (int iThemeKey, String* pstrString, bool bBlowup = true);
    int GetVertString (int iThemeKey, String* pstrString, bool bBlowup = true);

    void WriteButtonImageSrc (int iRealThemeKey, const char* pszButtonName);
    void WriteThemeDownloadSrc (int iRealThemeKey, const char* pszFileName);

    int StandardizeEmpireName (const char* pszName, char pszFinalName[MAX_EMPIRE_NAME_LENGTH + 1]);
    
    int VerifyPassword (const char* pszPassword, bool bPrintErrors = true);
    int VerifyEmpireName (const char* pszEmpireName, bool bPrintErrors = true);
    int VerifyCategoryName (const char* pszCategory, const char* pszName, size_t stMaxLen, bool bPrintErrors);

    bool ShipOrFleetNameFilter (const char* pszName);

    void WriteVersionString();
    void WriteAlmonasterBanner();
    void WriteContactLine();

    void WriteBodyString (Seconds iSecondsUntil);
    void WriteSeparatorString (int iSeparatorKey);
    void WriteButtonString (int iButtonKey, const char* pszButtonFileName, const char* pszButtonText, 
        const char* pszButtonName);

    int GetButtonName (const char* pszFormName, int iButtonKey, String* pstrButtonName);

    void OpenForm();

    void SendWelcomeMessage (const char* pszEmpireName);

    // Password hashing
    int GetPasswordHashForAutologon (int64* pi64Hash);
    int GetPasswordHashForGamePage (const UTCTime& tSalt, int64* pi64Hash);
    int GetPasswordHashForSystemPage (const UTCTime& tSalt, int64* pi64Hash);

    void HashIPAddress (const char* pszIPAddress, char* pszHashedIPAddress);

    bool RedirectOnSubmit (PageId* ppageRedirect);

    void WriteTime (Seconds sNumSeconds);
    int ConvertTime (Seconds sNumSeconds, char pszTime[MAX_HTML_TIME_LENGTH]);

    void WriteEmpireIcon (int iIconKey, int iEmpireKey, const char* pszAlt, bool bVerifyUpload);
    void WriteTournamentIcon (int iIconKey, int iTournamentKey, const char* pszAlt, bool bVerifyUpload);
    void WriteTournamentTeamIcon (int iIconKey, int iTournamentKey, int iTournamentTeamKey, const char* pszAlt, bool bVerifyUpload);
    void WriteIcon (int iIconKey, int iEntityKey, int iEntityKey2, const char* pszAlt, const char* pszUploadDir, bool bVerifyUpload);

    void WriteProfileAlienString (int iAlienKey, int iEmpireKey, const char* pszEmpireName, int iBorder, 
        const char* pszFormName, const char* pszAlt, bool bVerifyUpload, bool bKeyAndHash);

    void NotifyProfileLink();
    bool NotifiedProfileLink();

    void NotifyTournamentInvitation (int iMessageKey, int iTournamentKey);
    bool NotifiedTournamentInvitation();

    void NotifyTournamentJoinRequest (int iMessageKey, int iTournamentKey);
    bool NotifiedTournamentJoinRequest();

    unsigned int GetEmpireNameHashValue (const char* pszEmpireName);

    int HTMLFilter (const char* pszSource, String* pstrFiltered, size_t stNumChars, bool bAddMarkups);

    bool VerifyGIF (const char* pszFileName);
    int CopyUploadedIcon (const char* pszFileName, const char* pszUploadDir, int iKey1, int iKey2);

    int CopyNewAlien (const char* pszFileName, int iAlienKey);
    int DeleteAlien (int iAlienKey);

    void ReportLoginFailure (IReport* pReport, const char* pszEmpireName);
    void ReportLoginSuccess (IReport* pReport, const char* pszEmpireName);
    void ReportEmpireCreation (IReport* pReport, const char* pszEmpireName);

    void WriteSystemTitleString();
    void WriteSystemHeaders (bool bFileUpload);

    void WriteSystemButtons(int iButtonKey, int iPrivilege);
    void WriteSystemMessages();
    bool RenderSystemMessage (int iMessageKey, const Variant* pvMessage);

    void WriteBackupMessage();

    int InitializeEmpire (bool bAutoLogon);
    int InitializeSessionId (bool* pbUpdateSessionId, bool* pbUpdateCookie);

    void CloseSystemPage();

    int PostSystemPageInformation();

    void WriteActiveGameListHeader (const char* pszTableColor);
    void WriteOpenGameListHeader (const char* pszTableColor);
    void WriteSpectatorGameListHeader (const char* pszTableColor);
    void WriteSystemGameListHeader (const char* pszTableColor);
    void WriteGameAdministratorListHeader (const char* pszTableColor);

    int WriteActiveGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo);
    int WriteOpenGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo);
    int WriteSpectatorGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo);
    int WriteGameAdministratorListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo);
    int WriteSystemGameListData (int iGameClass, const Variant* pvGameClassInfo);

    void WriteCreateGameClassString (int iEmpireKey, unsigned int iTournamentKey, bool bPersonalGame);
    
    int ProcessCreateGameClassForms (unsigned int iOwnerKey, unsigned int iTournamentKey);
    
    int ProcessCreateDynamicGameClassForms (unsigned int iOwnerKey, int* piGameClass, int* piGameNumber, 
        bool* pbGameCreated);

    int ParseCreateGameClassForms (Variant* pvSubmitArray, int iOwnerKey, unsigned int iTournamentKey, bool bDynamic);

    void WriteCreateTournament (int iEmpireKey);
    int ProcessCreateTournament (int iEmpireKey);
    int ParseCreateTournamentForms (Variant* pvSubmitArray, int iEmpireKey);

    void WriteCreateTournamentTeam (unsigned int iTournamentKey);
    int ProcessCreateTournamentTeam (unsigned int iTournamentKey);
    int ParseCreateTournamentTeamForms (Variant* pvSubmitArray, unsigned int iTournamentKey);

    void WriteTournamentAdministrator (int iEmpireKey);
    void WriteAdministerTournament (unsigned int iTournamentKey);

    void WriteAdministerTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey);

    int StartTournamentGame (unsigned int iTournamentKey, int iTeamOptions);

    void WriteProfile (unsigned int iTargetEmpireKey, bool bEmpireAdmin, bool bSendMessage, bool bShowButtons);

    void WriteGameTitleString();
    int WriteGameHeaderString();
    
    bool RedirectOnSubmitGame (PageId* ppageRedirect);

    void CloseGamePage();

    int InitializeGame (PageId* ppageRedirect);

    void WriteGameMessages();

    void GetAlienButtonString (int iAlienKey, int iEmpireKey, bool bBorder, int iPlanetKey, int iProxyKey,
        const char* pszAlt, const char* pszExtraTag, String* pstrAlienButtonString);

    void WriteAlienButtonString (int iAlienKey, bool bBorder, const char* pszNamePrefix, 
        const char* pszAuthorName);

    void GetLivePlanetButtonString (int iLivePlanetKey, int iPlanetKey, int iProxyKey, const char* pszAlt,
        const char* pszExtraTag, String* pstrLivePlanet);
    void GetDeadPlanetButtonString (int iDeadPlanetKey, int iPlanetKey, int iProxyKey, const char* pszAlt,
        const char* pszExtraTag, String* pstrDeadPlanet);
    void GetIndependentPlanetButtonString (int iPlanetKey, int iProxyKey, const char* pszAlt, 
        const char* pszExtraTag, String* pstrPlanetString);

    void WriteLivePlanetString (int iLivePlanetKey);
    void WriteDeadPlanetString (int iDeadPlanetKey);
    void WriteIndependentPlanetString();

    int WriteUpClosePlanetString (int iEmpireKey, int iPlanetKey, int iProxyPlanetKey, int iLivePlanetKey, 
        int iDeadPlanetKey, int iPlanetCounter, bool bVisibleBuilds, int iGoodAg, int iBadAg, int iGoodMin, 
        int iBadMin, int iGoodFuel, int iBadFuel, float fEmpireAgRatio, bool bIndependence, bool bAdmin, 
        bool bSpectator, const Variant* pvPlanetData, bool* pbOurPlanet);

    int WriteRatiosString (RatioInformation* pratInfo);

    int RenderUnsafeHyperText (const char* pszText, const char* pszUrl);
    int RenderHyperText (const char* pszText, const char* pszUrl);

    void WriteServerRules();
    void WriteFaq();
    void WriteServerNews();

    void WriteContributions();
    void WriteCredits();

    void WriteIntro();
    void WriteIntroUpper();
    void WriteIntroLower();
    void WriteServerNewsFile();

    bool UpdateIntroUpper (const char* pszText);
    bool UpdateIntroLower (const char* pszText);
    bool UpdateServerNews (const char* pszText);

    int WriteShip (const GameConfiguration& gcConfig, const Variant* pvData, int i, int iShipKey, 
        int iBR, float fMaintRatio, int iShipLoc, int& iLastLocation, int& iLastX, int& iLastY, 
        Variant& vPlanetName, bool bFleet = false);

    int GetGoodBadResourceLimits (int iGameClass, int iGameNumber, int* piGoodAg, int* piBadAg, int* piGoodMin,
        int* piBadMin, int* piGoodFuel, int* piBadFuel);

    void WriteNukeHistory (int iTargetEmpireKey);
    void WritePersonalGameClasses (int iTargetEmpireKey);
    void WritePersonalTournaments (int iTargetEmpireKey);

    void WriteGameAdministratorGameData (const char* pszGameClassName, 
        int iGameNumber, Seconds iSeconds, Seconds iSecondsUntil, int iNumUpdates, bool bOpen, bool bPaused, 
        bool bAdminPaused, bool bStarted, const char* pszGamePassword, Variant* pvEmpireName, 
        int iNumActiveEmpires, const UTCTime& tCreationTime, bool bAdmin);

    int RenderMap (int iGameClass, int iGameNumber, int iEmpireKey, bool bAdmin,
        const PartialMapInfo* pPartialMapInfo, bool bSpectators);

    void RenderMiniMap (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey);
    void RenderMiniPlanet (const MiniMapEntry& mmEntry, unsigned int iEmpireKey, unsigned int iLivePlanetKey, 
        unsigned int iDeadPlanetKey);

    int GetSensitiveMapText (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey,
        int iProxyPlanetKey, bool bVisibleBuilds, bool bIndependence, const Variant* pvPlanetData, String* pstrAltTag);

    int RenderThemeInfo (int iBackgroundKey, int iLivePlanetKey, int iDeadPlanetKey, int iSeparatorKey,
        int iButtonKey, int iHorzKey, int iVertKey, int iColorKey);

    int DisplayThemeData (int iThemeKey);

    void WriteStringByDiplomacy (const char* pszString,
        int iDiplomacy);

    void SearchForDuplicateIPAddresses (int iGameClass, int iGameNumber);
    void SearchForDuplicateSessionIds (int iGameClass, int iGameNumber);

    int LoginEmpire();

    void RenderShips (const char* pszShips, const char* pszFleets,
        int iBR, float fMaintRatio, ShipsInMapScreen* pShipsInMap);

    int HandleShipMenuSubmissions();

    bool VerifyEmpireNameHash (int iEmpireKey, unsigned int iHash);

    // New button system
    bool IsLegalButtonId (ButtonId bidButton);
    bool WasButtonPressed (ButtonId bidButton);
    void WriteButton (ButtonId bidButton);

    // Search interface
    int HandleSearchSubmission (
        unsigned int* piSearchColName, 
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
        );

    void RenderSearchForms (bool fAdvanced);

    void RenderSearchResults (
        unsigned int* piSearchColName, 
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
        );

    void RenderEmpireInformation (int iGameClass, int iGameNumber, bool bAdmin);

    // Game entry confirmation
    void RenderGameConfiguration (int iGameClass, unsigned int iTournamentKey);

    int ParseGameConfigurationForms (int iGameClass, unsigned int iTournamentKey, 
        const Variant* pvGameClassInfo, int iEmpireKey, GameOptions* pgoOptions);

    // GameOptions
    void InitGameOptions (GameOptions* pgoOptions);
    void ClearGameOptions (GameOptions* pgoOptions);

    // Icons
    void WriteIconSelection (int iIconSelect, int iIcon, const char* pszCategory);
    int HandleIconSelection (unsigned int* piIcon, const char* pszUploadDir, unsigned int iKey1, unsigned int iKey2);

    // Admin
    void WriteActiveGameAdministration (int* piGameClass, int* piGameNumber, unsigned int iNumActiveGames, 
        unsigned int iNumOpenGames, unsigned int iNumClosedGames, bool bAdmin);

    void WriteAdministerGame (int iGameClass, int iGameNumber, bool bAdmin);

    // Tournaments
    void RenderTournaments (const unsigned int* piTournamentKey, unsigned int iTournaments, bool bSingleOwner);
    void RenderTournamentSimple (unsigned int iTournamentKey, bool bSingleOwner);
    void RenderTournamentDetailed (unsigned int iTournamentKey);

    void RenderEmpire (unsigned int iTournamentKey, int iEmpireKey);

    // Context stuff
    MilliSeconds GetTimerCount() {
        return Time::GetTimerCount (m_tmTimer);
    }
    
    void AddMessage (const char* pszMessage) {
        if (!m_strMessage.IsBlank()) {
            m_strMessage += "<br>";
        }
        m_strMessage += pszMessage;
    }

    void AddMessage (int iInt) {
        if (!m_strMessage.IsBlank()) {
            m_strMessage += "<br>";
        }
        m_strMessage += iInt;
    }

    void AddMessage (float fFloat) {
        if (!m_strMessage.IsBlank()) {
            m_strMessage += "<br>";
        }
        m_strMessage += fFloat;
    }

    void AppendMessage (const char* pszMessage) {
        m_strMessage += pszMessage;
    }

    void AppendMessage (int iInt) {
        m_strMessage += iInt;
    }

    void AppendMessage (unsigned int uiInt) {
        m_strMessage += uiInt;
    }

    void AppendMessage (float fFloat) {
        m_strMessage += fFloat;
    }

    int Render_TournamentManager (unsigned int iOwnerKey);

    // Rendering functions
    int Render_ActiveGameList();
    int Render_Login();
    int Render_NewEmpire();
    int Render_OpenGameList();
    int Render_SystemGameList();
    int Render_ProfileEditor();
    int Render_TopLists();
    int Render_ProfileViewer();
    int Render_ServerAdministrator();
    int Render_EmpireAdministrator();
    int Render_GameAdministrator();
    int Render_ThemeAdministrator();
    int Render_PersonalGameClasses();
    int Render_Chatroom();
    int Render_SystemServerRules();
    int Render_SystemFAQ();
    int Render_SystemNews();
    int Render_Info();
    int Render_Tech();
    int Render_Diplomacy();
    int Render_Map();
    int Render_Planets();
    int Render_Options();
    int Render_Build();
    int Render_Ships();
    int Render_GameServerRules();
    int Render_GameFAQ();
    int Render_GameNews();
    int Render_GameProfileViewer();
    int Render_Quit();
    int Render_LatestNukes();
    int Render_SpectatorGames();
    int Render_GameContributions();
    int Render_GameCredits();
    int Render_SystemContributions();
    int Render_SystemCredits();
    int Render_LatestGames();
    int Render_TournamentAdministrator();
    int Render_PersonalTournaments();
    int Render_Tournaments();
};

// Function Pointers
static int THREAD_CALL Fxn_ActiveGameList (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ActiveGameList();
}

static int THREAD_CALL Fxn_Login (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Login();
}

static int THREAD_CALL Fxn_NewEmpire (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_NewEmpire();
}

static int THREAD_CALL Fxn_OpenGameList (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_OpenGameList();
}

static int THREAD_CALL Fxn_SystemGameList (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemGameList();
}

static int THREAD_CALL Fxn_ProfileEditor (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ProfileEditor();
}

static int THREAD_CALL Fxn_TopLists (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_TopLists();
}

static int THREAD_CALL Fxn_ProfileViewer (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ProfileViewer();
}

static int THREAD_CALL Fxn_ServerAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ServerAdministrator();
}

static int THREAD_CALL Fxn_EmpireAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_EmpireAdministrator();
}

static int THREAD_CALL Fxn_GameAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameAdministrator();
}

static int THREAD_CALL Fxn_ThemeAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_ThemeAdministrator();
}

static int THREAD_CALL Fxn_PersonalGameClasses (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_PersonalGameClasses();
}

static int THREAD_CALL Fxn_Chatroom (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Chatroom();
}

static int THREAD_CALL Fxn_SystemServerRules (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemServerRules();
}

static int THREAD_CALL Fxn_SystemFAQ (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemFAQ();
}

static int THREAD_CALL Fxn_SystemNews (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemNews();
}

static int THREAD_CALL Fxn_Info (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Info();
}

static int THREAD_CALL Fxn_Tech (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Tech();
}

static int THREAD_CALL Fxn_Diplomacy (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Diplomacy();
}

static int THREAD_CALL Fxn_Map (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Map();
}

static int THREAD_CALL Fxn_Planets (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Planets();
}

static int THREAD_CALL Fxn_Options (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Options();
}

static int THREAD_CALL Fxn_Build (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Build();
}

static int THREAD_CALL Fxn_Ships (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Ships();
}

static int THREAD_CALL Fxn_GameServerRules (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameServerRules();
}

static int THREAD_CALL Fxn_GameFAQ (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameFAQ();
}

static int THREAD_CALL Fxn_GameNews (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameNews();
}

static int THREAD_CALL Fxn_GameProfileViewer (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameProfileViewer();
}

static int THREAD_CALL Fxn_Quit (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Quit();
}

static int THREAD_CALL Fxn_LatestNukes (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_LatestNukes();
}

static int THREAD_CALL FXN_SpectatorGames (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SpectatorGames();
}

static int THREAD_CALL Fxn_GameContributions (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameContributions();
}

static int THREAD_CALL Fxn_GameCredits (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_GameCredits();
}

static int THREAD_CALL Fxn_SystemContributions (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemContributions();
}

static int THREAD_CALL Fxn_SystemCredits (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_SystemCredits();
}

static int THREAD_CALL Fxn_LatestGames (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_LatestGames();
}

static int THREAD_CALL Fxn_TournamentAdministrator (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_TournamentAdministrator();
}

static int THREAD_CALL Fxn_PersonalTournaments (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_PersonalTournaments();
}

static int THREAD_CALL Fxn_Tournaments (void* pThis) {
    return ((HtmlRenderer*) pThis)->Render_Tournaments();
}


const ThreadFunction g_pfxnRenderPage[] = {
    NULL,
    Fxn_ActiveGameList,
    Fxn_Login,
    Fxn_NewEmpire,
    Fxn_OpenGameList,
    Fxn_SystemGameList,
    Fxn_ProfileEditor,
    Fxn_TopLists,
    Fxn_ProfileViewer,
    Fxn_ServerAdministrator,
    Fxn_EmpireAdministrator,
    Fxn_GameAdministrator,
    Fxn_ThemeAdministrator,
    Fxn_PersonalGameClasses,
    Fxn_Chatroom,
    Fxn_SystemServerRules,
    Fxn_SystemFAQ,
    Fxn_SystemNews,
    Fxn_Info,
    Fxn_Tech,
    Fxn_Diplomacy,
    Fxn_Map,
    Fxn_Planets,
    Fxn_Options,
    Fxn_Build,
    Fxn_Ships,
    Fxn_GameServerRules,
    Fxn_GameFAQ,
    Fxn_GameNews,
    Fxn_GameProfileViewer,
    Fxn_GameContributions,
    Fxn_GameCredits,
    Fxn_Quit,
    Fxn_LatestNukes,
    FXN_SpectatorGames,
    Fxn_SystemContributions,
    Fxn_SystemCredits,
    Fxn_LatestGames,
    Fxn_TournamentAdministrator,
    Fxn_PersonalTournaments,
    Fxn_Tournaments,
    NULL
};


#define DEFAULT_MESSAGE_FONT_SIZE           "-1"
#define DEFAULT_MESSAGE_FONT                "sans-serif"
#define DEFAULT_PLANET_NAME_FONT            "sans-serif"
#define DEFAULT_GAMECLASS_DESCRIPTION_FONT  "sans-serif"
#define DEFAULT_QUOTE_FONT                  "courier"

#define DEFAULT_LINK_COLOR "9090FF"
#define DEFAULT_ALINK_COLOR "9090FF"
#define DEFAULT_VLINK_COLOR "9090FF"
#define DEFAULT_BG_COLOR "000000"

#define DEFAULT_SEPARATOR_STRING "<hr width=\"90%\" height=\"16\">"

#define SYSTEM_MESSAGE_SENDER "The System"

// Dirs
#define BASE_RESOURCE_DIR       "resource/"
#define BASE_ALIEN_DIR          "aliens/"
#define BASE_UPLOADED_ALIEN_DIR "alienuploads"
#define BASE_UPLOADED_TOURNAMENT_ICON_DIR "tournamentuploads"
#define BASE_UPLOADED_TOURNAMENT_TEAM_ICON_DIR "tournamentteamuploads"

#define INDEPENDENT_PLANET_NAME "independent.gif"
#define BACKGROUND_IMAGE        "background.jpg"
#define ALMONASTER_BANNER_IMAGE "almonaster.gif"
#define SEPARATOR_IMAGE         "separator.gif"
#define DEFAULT_IMAGE_EXTENSION ".gif"
#define LIVE_PLANET_NAME        "planet.gif"
#define DEAD_PLANET_NAME        "planet2.gif"
#define HORZ_LINE_NAME          "horz.gif"
#define VERT_LINE_NAME          "vert.gif"

#define ALIEN_NAME              "alien"
#define ICON_FORMAT             "GIF89a"

#define ICON_WIDTH 40
#define ICON_HEIGHT 40

#define FAQ_FILE                "faq/faq.html"
#define NEWS_FILE               "text/news.html"
#define INTRO_FILE              "text/intro.html"
#define INTRO_UPPER_FILE        "text/intro-upper.html"
#define INTRO_LOWER_FILE        "text/intro-lower.html"

#define CREDITS_FILE            "text/credits.html"
#define CONTRIBUTIONS_FILE      "text/contributions.html"

#define MAX_NUM_SPACELESS_CHARS 72

#define LOWEST_STRING           "Lowest"
#define HIGHEST_STRING          "Highest"
#define UNLIMITED_STRING        "Unlimited"

// Cookies
#define LAST_EMPIRE_USED_COOKIE     "LastEmpireUsed"
#define AUTOLOGON_EMPIREKEY_COOKIE  "AutoLogonEmpireKey"
#define AUTOLOGON_PASSWORD_COOKIE   "AutoLogonPassword"

// Error macros
#define Check(FxnCall) iErrCode = ##FxnCall;                                        \
    if (iErrCode != OK) {                                                           \
        AddMessage ("Error ");                                                      \
        AppendMessage (iErrCode);                                                   \
        AppendMessage (" occurred in " __FILE__);                                   \
        AppendMessage (" on line ");                                                \
        AppendMessage (__LINE__);                                                   \
        AppendMessage ("; please contact the administrator");                       \
        return Redirect (LOGIN);                                                    \
    }

#define GameCheck(FxnCall) iErrCode = ##FxnCall;                                    \
    if (iErrCode != OK) {                                                           \
        AddMessage ("Error ");                                                      \
        AppendMessage (iErrCode);                                                   \
        AppendMessage (" occurred during a game in " __FILE__);                     \
        AppendMessage (" on line ");                                                \
        AppendMessage (__LINE__);                                                   \
        AppendMessage ("; please contact the administrator");                       \
        g_pGameEngine->SignalGameReader (m_iGameClass, m_iGameNumber);              \
        return Redirect (ACTIVE_GAME_LIST);                                         \
    }

#define EmpireCheck(FxnCall) iErrCode = ##FxnCall;                                  \
    if (iErrCode != OK) {                                                           \
        AddMessage ("That empire no longer exists");                                \
        return Redirect (LOGIN);                                                    \
    }

// Common macros
#define INITIALIZE_EMPIRE                                                           \
                                                                                    \
    bool bRedirectTest = true;                                                      \
    if (InitializeEmpire (false) != OK) {                                           \
        return Redirect (LOGIN);                                                    \
    }

#define SYSTEM_REDIRECT_ON_SUBMIT                                                   \
                                                                                    \
    goto Redirection;                                                               \
Redirection:                                                                        \
    if (bRedirectTest && !m_bRedirection) {                                         \
        PageId pageRedirect;                                                        \
        if (RedirectOnSubmit (&pageRedirect)) {                                     \
            return Redirect (pageRedirect);                                         \
        }                                                                           \
    }

#define SYSTEM_OPEN(bFlag)                                                          \
                                                                                    \
    Write ("<html><head><title>");                                                  \
                                                                                    \
    WriteSystemTitleString();                                                       \
    Write ("</title></head>");                                                      \
    WriteBodyString (-1);                                                           \
                                                                                    \
    Write ("<center>");                                                             \
                                                                                    \
    WriteSystemHeaders (bFlag);                                                     \
                                                                                    \
    Check (PostSystemPageInformation());                                            \
                                                                                    \
    WriteSystemButtons (m_iButtonKey, m_iPrivilege);                                \
                                                                                    \
    if (m_bTimeDisplay) {                                                           \
                                                                                    \
        char pszDateString [OS::MaxDateLength];                                     \
                                                                                    \
        int ec = Time::GetDateString (pszDateString);                               \
        if (ec == OK) {                                                             \
                                                                                    \
            OutputText ("Server time is <strong>");                                 \
            m_pHttpResponse->WriteText (pszDateString);                             \
            OutputText ("</strong><p>");                                            \
        }                                                                           \
    }                                                                               \
                                                                                    \
    WriteSystemMessages();                                                          \
                                                                                    \
    WriteSeparatorString (m_iSeparatorKey);                                         \

#define SYSTEM_CLOSE                                                                \
                                                                                    \
    CloseSystemPage();                              \
    return OK;

#define INITIALIZE_GAME                                                             \
                                                                                    \
    if (m_iPrivilege == GUEST) {                                                    \
        AddMessage ("Your empire does not have the privilege to access this game"); \
        return Redirect (ACTIVE_GAME_LIST);                                         \
    }                                                                               \
                                                                                    \
    PageId pageRedirect;                                                            \
    if (InitializeGame (&pageRedirect) != OK) {                                     \
        return Redirect (pageRedirect);                                             \
    }

#define GAME_REDIRECT_ON_SUBMIT                                                     \
                                                                                    \
    goto Redirection;                                                               \
Redirection:                                                                        \
    if (bRedirectTest && !m_bRedirection) {                                         \
        PageId pageRedirect;                                                        \
        if (RedirectOnSubmitGame (&pageRedirect)) {                                 \
            return Redirect (pageRedirect);                                         \
        }                                                                           \
    }


#define GAME_OPEN                                                                   \
                                                                                    \
    Write ("<html><head><title>");                                                  \
    WriteGameTitleString();                                 \
    Write ("</title></head>");                                                      \
                                                                                    \
    if (m_iGameState & PAUSED) {                                                        \
        WriteBodyString (-1);                               \
    } else {                                                                        \
        WriteBodyString (m_sSecondsUntil);                  \
    }                                                                               \
                                                                                    \
    Write ("<center>");                                                             \
    GameCheck (WriteGameHeaderString());

#define GAME_CLOSE                                                                  \
                                                                                    \
    CloseGamePage();                                                                \
    return OK;

#define HANDLE_CREATE_GAME_OUTPUT(iErrCode)                                         \
                                                                                    \
    switch (iErrCode) {                                                             \
    case OK:                                                                        \
        {                                                                           \
        char pszMessage [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH];       \
        /* Go to info screen! */                                                                \
        m_iGameClass = iGameClassKey;                                                           \
        m_iGameNumber = iGameNumber;                                                            \
        Check (g_pGameEngine->SetEnterGameIPAddress (iGameClassKey, iGameNumber, m_iEmpireKey, m_pHttpRequest->GetClientIP())); \
        Check (g_pGameEngine->GetGameClassName (iGameClassKey, m_pszGameClassName));                                \
        sprintf (pszMessage, "Welcome to %s %i, %s", m_pszGameClassName, iGameNumber, m_vEmpireName.GetCharPtr());  \
        AddMessage (pszMessage);                                                                        \
        return Redirect (INFO);                                                                         \
        }                                                                                               \
    case ERROR_DISABLED:                                                                                \
        {                                                                                               \
        Variant vReason;                                                                                \
        Check (g_pGameEngine->GetSystemProperty (SystemData::NewGamesDisabledReason, &vReason));        \
        AddMessage ("New game creation is disabled on the server at this time. ");                      \
        if (String::IsBlank (vReason.GetCharPtr())) {                                                   \
            AppendMessage ("Please try back later.");                                                   \
        } else {                                                                                        \
            AppendMessage (vReason.GetCharPtr());                                                       \
        }                                                                                               \
        return Redirect (m_pgPageId);                       \
        }                                                                                               \
    case ERROR_ALREADY_IN_GAME:                                                                         \
        AddMessage ("Your empire is already in that game");                                             \
        return Redirect (m_pgPageId);                                                                   \
    case ERROR_WAS_ALREADY_IN_GAME:                                                                     \
        AddMessage ("Your empire was already in that game");                                            \
        return Redirect (m_pgPageId);                                                                   \
    case ERROR_GAME_CLOSED:                                                                             \
        AddMessage ("The game already closed");                                                         \
        return Redirect (m_pgPageId);                       \
    case ERROR_WRONG_PASSWORD:                                                                          \
        AddMessage ("Your password was not accepted");                                      \
        return Redirect (m_pgPageId);                       \
    case ERROR_NULL_PASSWORD:                                                                           \
        AddMessage ("Game passwords cannot be blank");                                      \
        return Redirect (m_pgPageId);                       \
    case ERROR_ACCESS_DENIED:                                                                               \
        AddMessage ("Your scores were not within the accepted range");                          \
        return Redirect (m_pgPageId);                       \
    case ERROR_GAMECLASS_HALTED:                                                                        \
        AddMessage ("The gameclass is halted");                                             \
        return Redirect (m_pgPageId);                       \
    case ERROR_GAMECLASS_DELETED:                                                                       \
        AddMessage ("The gameclass is marked for deletion");                                    \
        return Redirect (m_pgPageId);                       \
    case ERROR_EMPIRE_IS_HALTED:                                                                        \
        AddMessage ("Your empire is halted and cannot enter games");                            \
        return Redirect (m_pgPageId);                       \
    case ERROR_GAME_DOES_NOT_EXIST:                                                                     \
        AddMessage ("That game no longer exists");                                          \
        return Redirect (m_pgPageId);                       \
    case ERROR_DUPLICATE_IP_ADDRESS:                                                                    \
        AddMessage ("You could not enter the game because your IP address was the same as an empire already playing the game");                                         \
        return Redirect (m_pgPageId);                       \
    case ERROR_DUPLICATE_SESSION_ID:                                                                    \
        AddMessage ("You could not enter the game because your Session Id was the same as an empire already playing the game");                                         \
        return Redirect (m_pgPageId);                       \
    default:                                                                                            \
        AddMessage ("Error ");                                                      \
        AppendMessage (iErrCode);                                                               \
        AppendMessage (" occurred while creating a game");                                  \
        return Redirect (m_pgPageId);                       \
    }

#define HANDLE_ENTER_GAME_OUTPUT(iErrCode)                                                      \
                                                                                                \
    switch (iErrCode) {                                                                         \
    case OK:                                                                                    \
        {                                                                                       \
        char pszMessage [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH];       \
        /* Go to info screen! */                                                                \
        m_iGameClass = iGameClassKey;                                                           \
        m_iGameNumber = iGameNumber;                                                            \
        Check (g_pGameEngine->SetEnterGameIPAddress (iGameClassKey, iGameNumber, m_iEmpireKey, m_pHttpRequest->GetClientIP())); \
        Check (g_pGameEngine->GetGameClassName (iGameClassKey, m_pszGameClassName));                                \
        sprintf (pszMessage, "Welcome to %s %i, %s", m_pszGameClassName, iGameNumber, m_vEmpireName.GetCharPtr());  \
        AddMessage (pszMessage);                                                        \
        return Redirect (INFO);                         \
        }                                                                                       \
    case ERROR_GAME_DOES_NOT_EXIST:                                                             \
        AddMessage ("That game no longer exists");                                  \
        return Redirect (m_pgPageId);               \
    case ERROR_ALREADY_IN_GAME:                                                                 \
        AddMessage ("Your empire is already in that game");                                     \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_WAS_ALREADY_IN_GAME:                                                                 \
        AddMessage ("Your empire was already in that game");                                        \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_GAME_CLOSED:                                                                     \
        AddMessage ("The game already closed");                                     \
        return Redirect (m_pgPageId);               \
    case ERROR_WRONG_PASSWORD:                                                                  \
        AddMessage ("Your password was not accepted");                              \
        return Redirect (m_pgPageId);               \
    case ERROR_NULL_PASSWORD:                                                                   \
        AddMessage ("Game passwords cannot be blank");                              \
        return Redirect (m_pgPageId);               \
    case ERROR_ACCESS_DENIED:                                                                       \
        AddMessage ("Your scores were not within the accepted range");                  \
        return Redirect (m_pgPageId);               \
    case ERROR_GAMECLASS_HALTED:                                                                \
        AddMessage ("The gameclass is halted");                                     \
        return Redirect (m_pgPageId);               \
    case ERROR_EMPIRE_IS_HALTED:                                                                \
        AddMessage ("Your empire is halted and cannot enter games");                    \
        return Redirect (m_pgPageId);               \
    case ERROR_DUPLICATE_IP_ADDRESS:                                                            \
        AddMessage ("You could not enter the game because your IP address was the same as an empire already playing the game");                                         \
        return Redirect (m_pgPageId);               \
    case ERROR_DUPLICATE_SESSION_ID:                                                            \
        AddMessage ("You could not enter the game because your Session Id was the same as an empire already playing the game");                                         \
        return Redirect (m_pgPageId);               \
    case ERROR_COULD_NOT_CREATE_PLANETS:                                                        \
        /* Fatal error:  we should never reach this case */                                     \
        Assert (false);                                                                         \
        iErrCode = g_pGameEngine->DeleteGame (iGameClassKey, iGameNumber, SYSTEM, NULL, MAP_CREATION_ERROR);                                                                \
        break;                                                                                  \
    default:                                                                                    \
        AddMessage ("Unknown error ");                                              \
        AppendMessage (iErrCode);                                                       \
        AppendMessage (" occurred while entering a game");                          \
        return Redirect (m_pgPageId);               \
    }

#endif