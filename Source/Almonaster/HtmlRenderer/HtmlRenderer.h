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

#pragma once

#include "Alajar.h"

#include "Almonaster.h"
#include "GameEngine.h"
#include "CChatroom.h"

#include "Osal/LinkedList.h"
#include "Osal/String.h"
#include "Osal/Time.h"

enum PageId
{
    NO_PAGE = 0,
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
    GAME_TERMS_OF_SERVICE = 32,
    QUIT = 33,
    LATEST_NUKES = 34,
    SPECTATOR_GAMES = 35,
    SYSTEM_CONTRIBUTIONS = 36,
    SYSTEM_CREDITS = 37,
    LATEST_GAMES = 38,
    TOURNAMENT_ADMINISTRATOR = 39,
    PERSONAL_TOURNAMENTS = 40,
    TOURNAMENTS = 41,
    SYSTEM_TERMS_OF_SERVICE = 42,
    MAX_PAGE_ID = 43,
};

static GameRatioSetting g_pageGameRatioSetting[] = 
{
    RATIOS_DISPLAY_NEVER, //MIN_PAGE_ID = 0,
    RATIOS_DISPLAY_NEVER, //ACTIVE_GAME_LIST = 1,
    RATIOS_DISPLAY_NEVER, //LOGIN = 2,
    RATIOS_DISPLAY_NEVER, //NEW_EMPIRE = 3,
    RATIOS_DISPLAY_NEVER, //OPEN_GAME_LIST = 4,
    RATIOS_DISPLAY_NEVER, //SYSTEM_GAME_LIST = 5,
    RATIOS_DISPLAY_NEVER, //PROFILE_EDITOR = 6,
    RATIOS_DISPLAY_NEVER, //TOP_LISTS = 7,
    RATIOS_DISPLAY_NEVER, //PROFILE_VIEWER = 8,
    RATIOS_DISPLAY_NEVER, //SERVER_ADMINISTRATOR = 9,
    RATIOS_DISPLAY_NEVER, //EMPIRE_ADMINISTRATOR = 10,
    RATIOS_DISPLAY_NEVER, //GAME_ADMINISTRATOR = 11,
    RATIOS_DISPLAY_NEVER, //THEME_ADMINISTRATOR = 12,
    RATIOS_DISPLAY_NEVER, //PERSONAL_GAME_CLASSES = 13,
    RATIOS_DISPLAY_NEVER, //CHATROOM = 14,
    RATIOS_DISPLAY_NEVER, //SYSTEM_SERVER_INFORMATION = 15,
    RATIOS_DISPLAY_NEVER, //SYSTEM_DOCUMENTATION = 16,
    RATIOS_DISPLAY_NEVER, //SYSTEM_NEWS = 17,
    RATIOS_DISPLAY_ALWAYS, //INFO = 18,
    RATIOS_DISPLAY_ON_RELEVANT_SCREENS, //TECH = 19,
    RATIOS_DISPLAY_ALWAYS, //DIPLOMACY = 20,
    RATIOS_DISPLAY_ALWAYS, //MAP = 21,
    RATIOS_DISPLAY_ALWAYS, //PLANETS = 22,
    RATIOS_DISPLAY_ALWAYS, //OPTIONS = 23,
    RATIOS_DISPLAY_ON_RELEVANT_SCREENS, //BUILD = 24,
    RATIOS_DISPLAY_ON_RELEVANT_SCREENS, //SHIPS = 25,
    RATIOS_DISPLAY_ALWAYS, //GAME_SERVER_INFORMATION = 26,
    RATIOS_DISPLAY_ALWAYS, //GAME_DOCUMENTATION = 27,
    RATIOS_DISPLAY_ALWAYS, //GAME_NEWS = 28,
    RATIOS_DISPLAY_ALWAYS, //GAME_PROFILE_VIEWER = 29,
    RATIOS_DISPLAY_ALWAYS, //GAME_CONTRIBUTIONS = 30,
    RATIOS_DISPLAY_ALWAYS, //GAME_CREDITS = 31,
    RATIOS_DISPLAY_ALWAYS, //GAME_TERMS_OF_SERVICE = 32,
    RATIOS_DISPLAY_ALWAYS, //QUIT = 33,
    RATIOS_DISPLAY_NEVER, //LATEST_NUKES = 34,
    RATIOS_DISPLAY_NEVER, //SPECTATOR_GAMES = 35,
    RATIOS_DISPLAY_NEVER, //SYSTEM_CONTRIBUTIONS = 36,
    RATIOS_DISPLAY_NEVER, //SYSTEM_CREDITS = 37,
    RATIOS_DISPLAY_NEVER, //LATEST_GAMES = 38,
    RATIOS_DISPLAY_NEVER, //TOURNAMENT_ADMINISTRATOR = 39,
    RATIOS_DISPLAY_NEVER, //PERSONAL_TOURNAMENTS = 40,
    RATIOS_DISPLAY_NEVER, //TOURNAMENTS = 41,
    RATIOS_DISPLAY_NEVER, //SYSTEM_TERMS_OF_SERVICE = 42,
    RATIOS_DISPLAY_NEVER, //MAX_PAGE_ID = 43,
};

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
    unsigned int iAlienAddress;
    unsigned int iPlanetKey;
    unsigned int iPlanetProxyKey;
    int iX;
    int iY;
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

#define MAX_SPECIFIC_EMPIRE_BLOCKS          25
#define UNSTARTED_GAMEPAGE_REFRESH_SEC      (120)

enum ButtonId {
    BID_FIRST,
    BID_ACTIVEGAMELIST,
    BID_ADMINISTERGAME,
    BID_ADVANCEDSEARCH,
    BID_ALL,
    BID_ALMONASTERSCORE,
    BID_ATTACK,
//    BID_BACKUP,
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
//    BID_DELETEBACKUP,
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
    BID_ENTER,
    BID_EXIT,
    BID_DOCUMENTATION,
//    BID_FLUSH,
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
//    BID_RESTOREBACKUP,
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
    BID_START,
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
//    BID_REBUILD,
    BID_LOOKUP,
    BID_VIEWMINIMAP,
    BID_MINIBUILD,
    BID_PLUS,
    BID_MINUS,
    BID_TOS,
    BID_TOS_ACCEPT,
    BID_TOS_DECLINE,
    BID_ADD_ASSOCIATION,
    BID_REMOVE_ASSOCIATION,
    BID_LAST,
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
    SEARCHFIELD_AGE,
    SEARCHFIELD_GENDER,
};

struct SearchField {
    bool bCheckedByDefault;
    const char* pszName;
    const char* pszInputCheckBox;
    const char* pszInput1;
    const char* pszInput2;
    const char* pszSystemEmpireDataColumn;
    SearchFieldType sftType;
    Privilege prvMinPriv;
};

extern const SearchField g_AdvancedSearchFields[];

#define MAX_NUM_SEARCH_COLUMNS 31

//
// HtmlRenderer
//

class IPage;

class HtmlRenderer : public GameEngine
{
protected:

    bool m_bRedirectTest;

    // Data
    PageId m_pgPageId;

    IHttpRequest* m_pHttpRequest;
    IHttpResponse* m_pHttpResponse;

    Variant m_vEmpireName;
    Variant m_vPreviousIPAddress;

    String m_strMessage;

    unsigned int m_iEmpireKey;
    unsigned int m_iGameClass;
    int m_iGameNumber;
    unsigned int m_iTournamentKey;
    unsigned int m_iButtonKey;
    int m_iButtonAddress;
    unsigned int m_iBackgroundKey;
    int m_iBackgroundAddress;
    unsigned int m_iSeparatorKey;
    int m_iPrivilege;
    unsigned int m_iAlienKey;
    int m_iAlienAddress;
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
    bool m_bLoggedIntoGame;
    bool m_bAuthenticated;
    bool m_bAutoLogon;

    Timer m_tmTimer;
    Seconds m_sSecondsUntil;
    Seconds m_sSecondsSince;

    int64 m_i64SessionId;

    // Submission control
    int m_iNumOldUpdates;
    int m_iNumNewUpdates;

    // Methods
    void WriteGameListHeader (const char** ppszHeaders, size_t stNumHeaders, const char* pszTableColor);
    void AddEmpiresInGame (int iGameState, int iNumActiveEmpires, const char* pszEmpires, int iMinEmpires, int iMaxEmpires);

    int AddGameClassDescription (int iWhichList, const Variant* pvGameClassInfo, 
        int iGameClass, int iGameNumber, int iGameState, const char* pszEmpiresInGame, 
        int iNumEmpiresInGame, bool bAdmin, bool bSpectators);

    int WriteInPlayGameListData (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, 
        bool bAdmin, bool bSpectator);

    void AddOptions (int iWhichList, const Variant* pvGameClassInfo, int iGameOptions);
    void AddResources (const Variant* pvGameClassInfo);
    void AddDiplomacy (const Variant* pvGameClassInfo);

    void AddTechList (int iTechs, int iInitial);

    int AddBridier(int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, 
                   int iGameOptions, const Variant* pvMin, const Variant* pvMax, bool bDisplayGainLoss);

    int AddBridierGame (int iGameClass, int iGameNumber, const Variant* pvGameClassInfo, int iGameOptions, bool bDisplayGainLoss);

    void AddScore (int iGameOptions, const Variant* pvMin, const Variant* pvMax);
    void AddSecurity (int iGameOptions);

    int PostGamePageInformation();

    void WriteGameButtons();
    int WriteGameNextUpdateString();

    int WriteTextFile (bool bTextArea, const char* pszFile, const char* pszFileForm, const char* pszFileHashForm);
    int TryUpdateFile (const char* pszFile, const char* pszFileForm, const char* pszFileHashForm);
    int UpdateCachedFile (const char* pszFileName, const char* pszText);

    void RenderSearchField (const SearchField& sfField, bool fAdvanced);
    void RenderDateField (const char* pszField);
    bool ParseDateField (const char* pszField, UTCTime* ptTime);
    void RenderHiddenSearchVariant (const char* pszColumn, const char* pszColName, const Variant& vData);

    static bool ms_bLocksInitialized;
    static ReadWriteLock ms_mTextFileLock;

    // Caching
    ColumnEntry m_systemEmpireCol;
    ColumnEntry m_systemTournamentCol;

    ColumnEntry m_gameCols[2];
    ColumnEntry m_gameEmpireCols[3];

    CrossJoinEntry m_crossJoinEntry;

public:

    // Statics
    static AlmonasterStatistics m_sStats;

    static UTCTime m_stEmpiresInGamesCheck;
    static UTCTime m_stServerNewsLastUpdate;

    static Mutex m_slockEmpiresInGames;
    static unsigned int m_siNumGamingEmpires;

    static int StaticInitialize();

    // Constructor
    HtmlRenderer(PageId pgPageId, IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse);

    // Wiring
    int ReadStandardForms();

    void GatherCacheTables(PageId pgPageId, Vector<TableCacheEntry>& cache);
    void GatherCacheTablesForSystemPage(Vector<TableCacheEntry>& cache);
    void GatherCacheTablesForGamePage(Vector<TableCacheEntry>& cache);
    int AfterCacheTablesForGamePage();

    int Render();
    int Redirect(PageId pageId);
    int CacheTables(PageId pgPageId, Vector<TableCacheEntry>& cache);

    void ShutdownServer();
    void RestartServer();
    void RestartAlmonaster();

    bool IsGamePage(PageId pageId);
    bool ShouldDisplayGameRatios();

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
    
    // Graphics
    int GetUIData (int iThemeKey);
    int GetTextColorData (int iEmpireColorKey);

    bool IsColor (const char* pszNewValue);

    void WriteBackgroundImageSrc(unsigned int iThemeKey, int iAddress);
    void WriteLivePlanetImageSrc(unsigned int iThemeKey, int iAddress);
    void WriteDeadPlanetImageSrc(unsigned int iThemeKey, int iAddress);
    void WriteSeparatorSrc(unsigned int iThemeKey, int iAddress);

    void WriteHorzSrc(unsigned int iThemeKey, int iAddress);
    void WriteVertSrc(unsigned int iThemeKey, int iAddress);

    void GetHorzString(unsigned int iThemeKey, int iAddress, String* pstrString, bool bBlowup = true);
    void GetVertString(unsigned int iThemeKey, int iAddress, String* pstrString, bool bBlowup = true);

    void WriteButtonImageSrc(unsigned int iRealThemeKey, int iAddress, const char* pszButtonName);
    void WriteThemeDownloadSrc(unsigned int iRealThemeKey, int iAddress, const char* pszFileName);

    void WriteSeparatorString(unsigned int iSeparatorKey, int iSeparatorAddress);
    void WriteButtonString(unsigned int iButtonKey, int iButtonAddress, const char* pszButtonFileName, const char* pszButtonText, const char* pszButtonName);

    // Strings
    bool StandardizeEmpireName (const char* pszName, char pszFinalName[MAX_EMPIRE_NAME_LENGTH + 1]);
    
    bool VerifyPassword (const char* pszPassword, bool bPrintErrors = true);
    bool VerifyEmpireName (const char* pszEmpireName, bool bPrintErrors = true);
    bool VerifyCategoryName (const char* pszCategory, const char* pszName, size_t stMaxLen, bool bPrintErrors);

    bool ShipOrFleetNameFilter (const char* pszName);

    int WriteVersionString();
    void WriteAlmonasterBanner();
    int WriteContactLine();

    void WriteBodyString (Seconds iSecondsUntil);

    void GetButtonName (const char* pszFormName, int iButtonKey, String* pstrButtonName);

    void OpenForm();

    void SendWelcomeMessage (const char* pszEmpireName);

    // Password hashing
    int GetAutologonPasswordHash(int iEmpireKey, String* pstrHash);
    int GetPagePasswordHash(PageId page, int iEmpireKey, const UTCTime& tSalt, String* pstrHash);

    void HashIPAddress (const char* pszIPAddress, char* pszHashedIPAddress);

    int RedirectOnSubmit (PageId* ppageRedirect, bool* pbRedirected);

    void WriteTime (Seconds sNumSeconds);

    int WriteEmpireIcon(unsigned int iIconKey, int iAddress, unsigned int iEmpireKey, const char* pszAlt, bool bVerifyUpload);
    int WriteTournamentIcon(unsigned int iIconKey, int iAddress, int iTournamentKey, const char* pszAlt, bool bVerifyUpload);
    int WriteTournamentTeamIcon(unsigned int iIconKey, int iAddress, int iTournamentKey, int iTournamentTeamKey, const char* pszAlt, bool bVerifyUpload);
    int WriteIcon(unsigned int iIconKey, int iAddress, unsigned int iEntityKey, unsigned int iEntityKey2, const char* pszAlt, const char* pszUploadDir, bool bVerifyUpload);

    int WriteProfileAlienString(unsigned int iAlienKey, int iAddress, unsigned int iEmpireKey, const char* pszEmpireName, int iBorder, 
        const char* pszFormName, const char* pszAlt, bool bVerifyUpload, bool bKeyAndHash);

    void NotifyProfileLink();
    bool NotifiedProfileLink();

    void NotifyTournamentInvitation (int iMessageKey, int iTournamentKey);
    bool NotifiedTournamentInvitation();

    void NotifyTournamentJoinRequest (int iMessageKey, int iTournamentKey);
    bool NotifiedTournamentJoinRequest();

    unsigned int GetEmpireNameHashValue (const char* pszEmpireName);

    void HTMLFilter (const char* pszSource, String* pstrFiltered, size_t stNumChars, bool bAddMarkups);

    int VerifyGIF(const char* pszFileName, bool* pbGoodGIF);
    bool CopyUploadedIcon (const char* pszFileName, const char* pszUploadDir, int iKey1, int iKey2);

    bool CopyNewAlien(const char* pszFileName, int iNewAddress);
    bool DeleteAlienFile(int iAddress);

    void ReportLoginFailure(const char* pszEmpireName);
    void ReportLoginSuccess(const char* pszEmpireName, bool bAutoLogon);
    void ReportEmpireCreation(const char* pszEmpireName);

    int OpenSystemPage(bool bFileUpload);
    int WriteSystemTitleString();
    int WriteSystemHeaders(bool bFileUpload);

    int WriteSystemButtons(int iPrivilege);
    int WriteSystemMessages();
    int RenderSystemMessage (int iMessageKey, const Variant* pvMessage, bool* pbMessageFromEmpire);

    void WriteBackupMessage();

    int InitializeEmpireInGame(bool bAutoLogon, bool* pbInitialized);
    int InitializeEmpire(bool bAutoLogon, bool* pbInitialized);
    int InitializeSessionId (bool* pbUpdateSessionId, bool* pbUpdateCookie);

    int CloseSystemPage();
    void PostSystemPageInformation();

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

    int WriteCreateGameClassString (int iEmpireKey, unsigned int iTournamentKey, bool bPersonalGame);
    
    int ProcessCreateGameClassForms (unsigned int iOwnerKey, unsigned int iTournamentKey, bool* pbProcessed);
    
    int ProcessCreateDynamicGameClassForms (unsigned int iOwnerKey, int* piGameClass, int* piGameNumber, bool* pbGameCreated);

    int ParseCreateGameClassForms(Variant* pvSubmitArray, int iOwnerKey, unsigned int iTournamentKey, bool bDynamic, bool* pbParsed);

    void WriteCreateTournament (int iEmpireKey);
    int ProcessCreateTournament (int iEmpireKey, bool* pbCreated);
    int ParseCreateTournamentForms (Variant* pvSubmitArray, int iEmpireKey, bool* pbParsed);

    void WriteCreateTournamentTeam (unsigned int iTournamentKey);
    int ProcessCreateTournamentTeam (unsigned int iTournamentKey, bool* pbCreated);
    int ParseCreateTournamentTeamForms (Variant* pvSubmitArray, unsigned int iTournamentKey, bool* pbParsed);

    int WriteTournamentAdministrator (int iEmpireKey);
    int WriteAdministerTournament(unsigned int iTournamentKey);

    int WriteAdministerTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey);

    int StartTournamentGame(unsigned int iTournamentKey, int iTeamOptions, bool bAdvanced);

    int WriteProfile (unsigned int iEmpireKey, unsigned int iTargetEmpireKey, bool bEmpireAdmin, bool bSendMessage, bool bShowButtons);

    int OpenGamePage();
    int WriteGameTitleString();
    int WriteGameHeaderString();
    
    int RedirectOnSubmitGame(PageId* ppageRedirect, bool* pbRedirected);

    int CloseGamePage();

    int InitializeGame(PageId* ppageRedirect, bool* pbRedirected);

    int WriteGameMessages();

    int GetAlienPlanetButtonString(unsigned int iAlienKey, int iAlienAddress, unsigned int iEmpireKey, bool bBorder, int iPlanetKey, int iProxyKey,
        const char* pszAlt, const char* pszExtraTag, String* pstrAlienButtonString);

    void WriteAlienButtonString(unsigned int iAlienKey, int iAddress, bool bBorder, const char* pszNamePrefix, const char* pszAuthorName);

    void GetLivePlanetButtonString(unsigned int iLivePlanetThemeKey, int iLivePlanetAddress, int iPlanetKey, int iProxyKey, const char* pszAlt, const char* pszExtraTag, String* pstrLivePlanet);
    void GetDeadPlanetButtonString(unsigned int iDeadPlanetThemeKey, int iDeadPlanetAddress, int iPlanetKey, int iProxyKey, const char* pszAlt, const char* pszExtraTag, String* pstrDeadPlanet);
    void GetIndependentPlanetButtonString(int iPlanetKey, int iProxyKey, const char* pszAlt, const char* pszExtraTag, String* pstrPlanetString);

    void WriteLivePlanetString(unsigned int iLivePlanetKey, int iLivePlanetAddress);
    void WriteDeadPlanetString(unsigned int iDeadPlanetKey, int iDeadPlanetAddress);
    void WriteIndependentPlanetString();

    int WriteUpClosePlanetString (unsigned int iEmpireKey, int iPlanetKey, int iProxyPlanetKey, unsigned int iLivePlanetKey, int iLivePlanetAddress,
        unsigned int iDeadPlanetKey, int iDeadPlanetAddress, int iPlanetCounter, bool bVisibleBuilds, int iGoodAg, int iBadAg, int iGoodMin, 
        int iBadMin, int iGoodFuel, int iBadFuel, float fEmpireAgRatio, bool bIndependence, bool bAdmin, 
        bool bSpectator, const Variant* pvPlanetData, bool* pbOurPlanet);

    int WriteRatiosString (RatioInformation* pratInfo);

    void RenderUnsafeHyperText (const char* pszText, const char* pszUrl);
    void RenderHyperText (const char* pszText, const char* pszUrl);

    int WriteServerRules();
    void WriteFaq();
    void WriteServerNews();

    void WriteContributions();
    void WriteCredits();

    void WriteIntro();

    void WriteIntroUpper (bool bTextArea);
    void WriteIntroLower (bool bTextArea);
    void WriteServerNewsFile (bool bTextArea);
    void WriteContributorsFile (bool bTextArea);

    void WriteTOS();
    void WriteTOSFile();
    void WriteConfirmTOSDecline();

    int TryUpdateIntroUpper();
    int TryUpdateIntroLower();
    int TryUpdateServerNews();
    int TryUpdateContributors();

    int WriteShip (unsigned int iShipKey, const Variant* pvData, unsigned int iIndex, bool bFleet,
        const GameConfiguration& gcConfig, const ShipOrderPlanetInfo& planetInfo, 
        const ShipOrderShipInfo& shipInfo, const ShipOrderGameInfo& gameInfo,
        const BuildLocation* pblLocations, unsigned int iNumLocations);

    int GetGoodBadResourceLimits (int iGameClass, int iGameNumber, int* piGoodAg, int* piBadAg, int* piGoodMin,
        int* piBadMin, int* piGoodFuel, int* piBadFuel);

    int WriteNukeHistory(int iTargetEmpireKey);
    int WritePersonalGameClasses(int iTargetEmpireKey);
    int WritePersonalTournaments(int iTargetEmpireKey);
    int WritePersonalTournaments();

    void WriteGameAdministratorGameData (const char* pszGameClassName, 
        int iGameNumber, Seconds iSeconds, Seconds iSecondsUntil, int iNumUpdates, bool bOpen, bool bPaused, 
        bool bAdminPaused, bool bStarted, const char* pszGamePasswordHash, Variant** ppvEmpiresInGame, 
        int iNumActiveEmpires, const UTCTime& tCreationTime, bool bAdmin);

    int PopulatePlanetInfo (unsigned int iGameClass, unsigned int iGameNumber, unsigned int iShipPlanet,
        ShipOrderPlanetInfo& planetInfo, String& strPlanetName);

    int RenderMap (int iGameClass, int iGameNumber, int iEmpireKey, bool bAdmin,
        const PartialMapInfo* pPartialMapInfo, bool bSpectators);

    int RenderMiniMap (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey);
    int RenderMiniPlanet(const MiniMapEntry& mmEntry, unsigned int iEmpireKey, unsigned int iLivePlanetKey, int iLivePlanetAddress,
                         unsigned int iDeadPlanetKey, int iDeadPlanetAddress);

    int GetSensitiveMapText (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey,
        int iProxyPlanetKey, bool bVisibleBuilds, bool bIndependence, const Variant* pvPlanetData, String* pstrAltTag);

    int RenderThemeInfo (int iBackgroundKey, int iLivePlanetKey, int iDeadPlanetKey, int iSeparatorKey,
        int iButtonKey, int iHorzKey, int iVertKey, int iColorKey);

    int DisplayThemeData (int iThemeKey);

    void WriteStringByDiplomacy (const char* pszString,
        int iDiplomacy);

    enum DuplicateType
    {
        IP_ADDRESS,
        SESSION_ID,
    };
    int SearchForDuplicateEmpires(int iGameClass, int iGameNumber, DuplicateType type);

    int HtmlLoginEmpire(bool* pbLoggedIn);

    int RenderShips (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
        int iBR, float fMaintRatio, float fNextMaintRatio, ShipsInMapScreen* pShipsInMap, bool bShipString,
        unsigned int* piShips, unsigned int* piFleets);

    int HandleShipMenuSubmissions();

    int VerifyEmpireNameHash (int iEmpireKey, unsigned int iHash, bool* pbVerified);
    
    // Build
    int RenderMiniBuild (unsigned int iPlanetKey, bool bSingleBar);
    int HandleMiniBuild (unsigned int iPlanetKey);

    int HtmlCreateRandomFleet (unsigned int iPlanetKey, unsigned int* piFleetKey);

    int HandleBuildNewShipsResult(int iErrCode, int iNumShipsBuilt, int iBR, int iTechKey, const char* pszPlanetName, 
                                  int iX, int iY, const char* pszFleetName, bool bBuildReduced);

    int HandleCreateNewFleetResult(int iErrCode, const char* pszFleetName);

    // New button system
    bool IsLegalButtonId (ButtonId bidButton);
    bool WasButtonPressed (ButtonId bidButton);
    void WriteButton (ButtonId bidButton);

    // Search interface
    int HandleSearchSubmission(
        SearchDefinition& sd,
        const char** pszFormName,
        const char** pszColName1,
        const char** pszColName2,
        unsigned int** ppiSearchEmpireKey,
        unsigned int* piNumSearchEmpires,
        bool* pbMore
        );

    int RenderSearchForms (bool fAdvanced);

    int RenderSearchResults(
        SearchDefinition& sd,
        const char** pszFormName,
        const char** pszColName1,
        const char** pszColName2,
        unsigned int* piSearchEmpireKey,
        unsigned int iNumSearchEmpires,
        bool bMore
        );

    int RenderEmpireInformation (int iGameClass, int iGameNumber, bool bAdmin);

    // Game entry confirmation
    int RenderGameConfiguration (int iGameClass, unsigned int iTournamentKey);
    int ParseGameConfigurationForms (int iGameClass, unsigned int iTournamentKey, const Variant* pvGameClassInfo, GameOptions* pgoOptions);

    // Icons
    int WriteIconSelection(int iIconSelect, unsigned int iAlienKey, const char* pszCategory);
    int HandleIconSelection(unsigned int* piAlienKey, const char* pszUploadDir, unsigned int iKey1, unsigned int iKey2, bool* pbHandled);

    // Admin
    int WriteActiveGameAdministration (const Variant** ppvGames, unsigned int iNumActiveGames, unsigned int iNumOpenGames, unsigned int iNumClosedGames, bool bAdmin);
    int WriteAdministerGame(int iGameClass, int iGameNumber, bool bAdmin);

    // Tournaments
    int RenderTournaments(const Variant* pvTournamentKey, unsigned int iTournaments, bool bSingleOwner);
    int RenderTournaments(const unsigned int* piTournamentKey, unsigned int iTournaments, bool bSingleOwner);
    int RenderTournamentSimple(unsigned int iTournamentKey, bool bSingleOwner);
    int RenderTournamentDetailed(unsigned int iTournamentKey);

    int RenderEmpire (unsigned int iTournamentKey, int iEmpireKey);

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
    int Render_GameTos();
    int Render_SystemTos();

    // Cache contribution functions
    void RegisterCache_ActiveGameList(Vector<TableCacheEntry>& cache);
    void RegisterCache_Login(Vector<TableCacheEntry>& cache);
    void RegisterCache_NewEmpire(Vector<TableCacheEntry>& cache);
    void RegisterCache_OpenGameList(Vector<TableCacheEntry>& cache);
    void RegisterCache_SystemGameList(Vector<TableCacheEntry>& cache);
    void RegisterCache_ProfileEditor(Vector<TableCacheEntry>& cache);
    void RegisterCache_TopLists(Vector<TableCacheEntry>& cache);
    void RegisterCache_ProfileViewer(Vector<TableCacheEntry>& cache);
    void RegisterCache_ServerAdministrator(Vector<TableCacheEntry>& cache);
    void RegisterCache_EmpireAdministrator(Vector<TableCacheEntry>& cache);
    void RegisterCache_GameAdministrator(Vector<TableCacheEntry>& cache);
    void RegisterCache_ThemeAdministrator(Vector<TableCacheEntry>& cache);
    void RegisterCache_PersonalGameClasses(Vector<TableCacheEntry>& cache);
    void RegisterCache_Chatroom(Vector<TableCacheEntry>& cache);
    void RegisterCache_SystemServerRules(Vector<TableCacheEntry>& cache);
    void RegisterCache_SystemFAQ(Vector<TableCacheEntry>& cache);
    void RegisterCache_SystemNews(Vector<TableCacheEntry>& cache);
    void RegisterCache_Info(Vector<TableCacheEntry>& cache);
    void RegisterCache_Tech(Vector<TableCacheEntry>& cache);
    void RegisterCache_Diplomacy(Vector<TableCacheEntry>& cache);
    void RegisterCache_Map(Vector<TableCacheEntry>& cache);
    void RegisterCache_Planets(Vector<TableCacheEntry>& cache);
    void RegisterCache_Options(Vector<TableCacheEntry>& cache);
    void RegisterCache_Build(Vector<TableCacheEntry>& cache);
    void RegisterCache_Ships(Vector<TableCacheEntry>& cache);
    void RegisterCache_GameServerRules(Vector<TableCacheEntry>& cache);
    void RegisterCache_GameFAQ(Vector<TableCacheEntry>& cache);
    void RegisterCache_GameNews(Vector<TableCacheEntry>& cache);
    void RegisterCache_GameProfileViewer(Vector<TableCacheEntry>& cache);
    void RegisterCache_Quit(Vector<TableCacheEntry>& cache);
    void RegisterCache_LatestNukes(Vector<TableCacheEntry>& cache);
    void RegisterCache_SpectatorGames(Vector<TableCacheEntry>& cache);
    void RegisterCache_GameContributions(Vector<TableCacheEntry>& cache);
    void RegisterCache_GameCredits(Vector<TableCacheEntry>& cache);
    void RegisterCache_SystemContributions(Vector<TableCacheEntry>& cache);
    void RegisterCache_SystemCredits(Vector<TableCacheEntry>& cache);
    void RegisterCache_LatestGames(Vector<TableCacheEntry>& cache);
    void RegisterCache_TournamentAdministrator(Vector<TableCacheEntry>& cache);
    void RegisterCache_PersonalTournaments(Vector<TableCacheEntry>& cache);
    void RegisterCache_Tournaments(Vector<TableCacheEntry>& cache);
    void RegisterCache_GameTos(Vector<TableCacheEntry>& cache);
    void RegisterCache_SystemTos(Vector<TableCacheEntry>& cache);

    int AfterCache_ActiveGameList();
    int AfterCache_Login();
    int AfterCache_NewEmpire();
    int AfterCache_OpenGameList();
    int AfterCache_SystemGameList();
    int AfterCache_ProfileEditor();
    int AfterCache_TopLists();
    int AfterCache_ProfileViewer();
    int AfterCache_ServerAdministrator();
    int AfterCache_EmpireAdministrator();
    int AfterCache_GameAdministrator();
    int AfterCache_ThemeAdministrator();
    int AfterCache_PersonalGameClasses();
    int AfterCache_Chatroom();
    int AfterCache_SystemServerRules();
    int AfterCache_SystemFAQ();
    int AfterCache_SystemNews();
    int AfterCache_Info();
    int AfterCache_Tech();
    int AfterCache_Diplomacy();
    int AfterCache_Map();
    int AfterCache_Planets();
    int AfterCache_Options();
    int AfterCache_Build();
    int AfterCache_Ships();
    int AfterCache_GameServerRules();
    int AfterCache_GameFAQ();
    int AfterCache_GameNews();
    int AfterCache_GameProfileViewer();
    int AfterCache_Quit();
    int AfterCache_LatestNukes();
    int AfterCache_SpectatorGames();
    int AfterCache_GameContributions();
    int AfterCache_GameCredits();
    int AfterCache_SystemContributions();
    int AfterCache_SystemCredits();
    int AfterCache_LatestGames();
    int AfterCache_TournamentAdministrator();
    int AfterCache_PersonalTournaments();
    int AfterCache_Tournaments();
    int AfterCache_GameTos();
    int AfterCache_SystemTos();

    void RegisterCacheRatioTablesIfNecessary(Vector<TableCacheEntry>& cache);
    int AfterCacheRatioTablesIfNecessary();
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

#define TRANSPARENT_DOT         "dot.gif"
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

#define ICON_WIDTH 40
#define ICON_HEIGHT 40

#define FAQ_FILE                "faq/faq.html"
#define NEWS_FILE               "text/news.html"
#define INTRO_FILE              "text/intro.html"
#define INTRO_UPPER_FILE        "text/intro-upper.html"
#define INTRO_LOWER_FILE        "text/intro-lower.html"
#define CONTRIBUTORS_FILE       "text/contributors.html"

#define CREDITS_FILE            "text/credits.html"
#define CONTRIBUTIONS_FILE      "text/contributions.html"

#define TOS_FILE                "text/tos.html"

#define MAX_NUM_SPACELESS_CHARS 72

#define LOWEST_STRING           "Lowest"
#define HIGHEST_STRING          "Highest"
#define UNLIMITED_STRING        "Unlimited"

// Cookies
#define SESSION_ID_COOKIE           "SessionId"
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
        return iErrCode;                                                            \
    }

#define GameCheck(FxnCall) iErrCode = ##FxnCall;                                    \
    if (iErrCode != OK) {                                                           \
        AddMessage ("Error ");                                                      \
        AppendMessage (iErrCode);                                                   \
        AppendMessage (" occurred during a game in " __FILE__);                     \
        AppendMessage (" on line ");                                                \
        AppendMessage (__LINE__);                                                   \
        AppendMessage ("; please contact the administrator");                       \
        return iErrCode;                                                            \
    }

#define EmpireCheck(FxnCall) iErrCode = ##FxnCall;                                  \
    if (iErrCode != OK) {                                                           \
        AddMessage ("That empire no longer exists");                                \
        return iErrCode;                                                            \
    }

#define HANDLE_CREATE_GAME_OUTPUT(iErrCode)                                         \
                                                                                    \
    switch (iErrCode) {                                                             \
    case OK:                                                                        \
        {                                                                           \
        char pszMessage [128 + MAX_FULL_GAME_CLASS_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH];       \
        /* Go to info screen! */                                                                \
        m_iGameClass = iGameClassKey;                                                           \
        m_iGameNumber = iGameNumber;                                                            \
        iErrCode = SetEnterGameIPAddress (iGameClassKey, iGameNumber, m_iEmpireKey, m_pHttpRequest->GetClientIP()); \
        RETURN_ON_ERROR(iErrCode);                                                                      \
        iErrCode = GetGameClassName (iGameClassKey, m_pszGameClassName);                                \
        RETURN_ON_ERROR(iErrCode);                                                                      \
        sprintf(pszMessage, "Welcome to %s %i, %s", m_pszGameClassName, iGameNumber, m_vEmpireName.GetCharPtr());  \
        AddMessage (pszMessage);                                                                        \
        return Redirect (INFO);                                                                         \
        }                                                                                               \
    case ERROR_DISABLED:                                                                                \
        {                                                                                               \
        Variant vReason;                                                                                \
        iErrCode = GetSystemProperty (SystemData::NewGamesDisabledReason, &vReason);                    \
        RETURN_ON_ERROR(iErrCode);                                                                      \
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
    case ERROR_PASSWORD:                                                                                \
        AddMessage ("Your password was not accepted");                                                  \
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
        RETURN_ON_ERROR(iErrCode);                          \
        return iErrCode;                                    \
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
        iErrCode = SetEnterGameIPAddress (iGameClassKey, iGameNumber, m_iEmpireKey, m_pHttpRequest->GetClientIP()); \
        RETURN_ON_ERROR(iErrCode);                                                              \
        iErrCode = GetGameClassName (iGameClassKey, m_pszGameClassName);                        \
        RETURN_ON_ERROR(iErrCode);                                                              \
        sprintf(pszMessage, "Welcome to %s %i, %s", m_pszGameClassName, iGameNumber, m_vEmpireName.GetCharPtr());  \
        AddMessage (pszMessage);                                                                \
        return Redirect (INFO);                                                                 \
        }                                                                                       \
    case ERROR_GAME_DOES_NOT_EXIST:                                                             \
        AddMessage ("That game no longer exists");                                              \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_ALREADY_IN_GAME:                                                                 \
        AddMessage ("Your empire is already in that game");                                     \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_WAS_ALREADY_IN_GAME:                                                             \
        AddMessage ("Your empire was already in that game");                                    \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_GAME_CLOSED:                                                                     \
        AddMessage ("The game already closed");                                                 \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_PASSWORD:                                                                        \
        AddMessage ("Your password was not accepted");                                          \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_NULL_PASSWORD:                                                                   \
        AddMessage ("Game passwords cannot be blank");                                          \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_ACCESS_DENIED:                                                                   \
        AddMessage ("Your scores were not within the accepted range");                          \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_GAMECLASS_HALTED:                                                                \
        AddMessage ("The gameclass is halted");                                                 \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_EMPIRE_IS_HALTED:                                                                \
        AddMessage ("Your empire is halted and cannot enter games");                            \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_DUPLICATE_IP_ADDRESS:                                                            \
        AddMessage ("You could not enter the game because your IP address was the same as an empire already playing the game");                                         \
        return Redirect (m_pgPageId);                                                           \
    case ERROR_DUPLICATE_SESSION_ID:                                                            \
        AddMessage ("You could not enter the game because your Session Id was the same as an empire already playing the game");                                         \
        return Redirect (m_pgPageId);                                                           \
    default:                                                                                    \
        RETURN_ON_ERROR(iErrCode);                                                              \
        return iErrCode;                                                                        \
    }
