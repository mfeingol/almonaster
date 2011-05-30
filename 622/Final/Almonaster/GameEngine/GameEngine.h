// GameEngine.h: Definition of the GameEngine class
//
//////////////////////////////////////////////////////////////////////
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

#if !defined(AFX_GAMEENGINE_H__58727607_5549_11D1_9D09_0060083E8062__INCLUDED_)
#define AFX_GAMEENGINE_H__58727607_5549_11D1_9D09_0060083E8062__INCLUDED_

#include "Alajar.h"

#include <stdio.h>
#include <limits.h>

#define ALMONASTER_BUILD

#include "GameEngineSchema.h"
#include "GameEngineUI.h"
#include "GameEngineGameObject.h"
#include "GameEngineLocks.h"
#include "IGameEngine.h"
#include "../Chatroom/CChatroom.h"

#include "Osal/Thread.h"
#include "Osal/Event.h"
#include "Osal/FifoQueue.h"
#include "Osal/Library.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/HashTable.h"
#include "Osal/Vector.h"

#undef ALMONASTER_BUILD

class GameEngine;

// Remove annoying warning
#ifdef _WIN32
#pragma warning (disable : 4245)
#endif

//
// Types
//

struct LongRunningQueryMessage;
typedef int (THREAD_CALL *Fxn_QueryCallBack) (LongRunningQueryMessage*);

struct LongRunningQueryMessage {
    GameEngine* pGameEngine;
    Fxn_QueryCallBack pQueryCall;
    void* pArguments;
};

struct EmpireIdentity {
    int iEmpireKey;
    int64 i64SecretKey;
};

/////////////////////////////////////////////////////////////////////////////
// GameEngine

class GameEngine : public IDatabaseBackupNotificationSink, public IGameEngine {

    friend class AlmonasterHook;

private:

    IDatabase* m_pGameData;
    Chatroom* m_pChatroom;

    // Game objects
    class GameObjectHashValue {
    public:
        static unsigned int GetHashValue (const char* pData, unsigned int iNumBuckets, const void* pHashHint);
    };

    class GameObjectEquals {
    public:
        static bool Equals (const char* pLeft, const char* pRight, const void* pEqualsHint);
    };

    HashTable<const char*, GameObject*, GameObjectHashValue, GameObjectEquals> m_htGameObjectTable;
    ReadWriteLock m_rwGameObjectTableLock;

    // UI Event sink
    IAlmonasterUIEventSink* m_pUIEventSink;

    // Report, page source control
    IReport* m_pReport;
    IPageSourceControl* m_pPageSourceControl;

    // Almonaster hook
    Library m_libHook;
    IAlmonasterHook* m_pAlmonasterHook;

    // Scoring systems
    IScoringSystem* m_ppScoringSystem[NUM_SCORING_SYSTEMS];

    // Autobackup
    bool m_bGoodDatabase;

    Thread m_tAutoBackupThread;
    Event m_eAutoBackupEvent;

    static int THREAD_CALL AutomaticBackup (void* pvGameEngine);
    int AutomaticBackup();

    // Backup status
    DatabaseBackupStage m_dbsStage;

    unsigned int m_iMaxNumTemplates;
    unsigned int m_iMaxNumTables;

    bool m_bActiveBackup;
    UTCTime m_tBackupStartTime;

    // Configuration
    SystemConfiguration m_scConfig;
    ChatroomConfig m_ccConfig;

    ReadWriteLock m_rwGameConfigLock;
    ReadWriteLock m_rwMapConfigLock;

    // Database name
    char* m_pszDatabaseName;

    // Synchronization
    ReadWriteLock m_mConfigLock;

    Mutex m_mGameClasses;
    Mutex m_mSuperClasses;
    Mutex m_mAlienIcons;

    // GameEmpireLock management
    GameEmpireLockManager m_lockMgr;

    // Long running queries
    Event m_eQueryEvent;

    Thread m_tLongRunningQueries;
    ThreadSafeFifoQueue<LongRunningQueryMessage*> m_tsfqQueryQueue;

    static int THREAD_CALL LongRunningQueryProcessor (void* pVoid);
    int LongRunningQueryProcessorLoop();

    int SendLongRunningQueryMessage (Fxn_QueryCallBack pfxFunction, void* pVoid);

    // Setup
    int Setup();
    
    int CreateNewDatabase();
    int ReloadDatabase();

    int CreateDefaultSystemTemplates();
    int CreateDefaultSystemTables();
    int SetupDefaultSystemTables();
    int SetupDefaultSystemGameClasses();

    int VerifySystem();
    int VerifyEmpires();
    int VerifyGameClasses();
    int VerifyMarkedGameClasses();
    int VerifyActiveGames();

    int VerifyTournaments();
    int VerifyTopLists();
    int RebuildTopList (ScoringSystem ssTopList);

    void VerifySystemTables (bool* pbNewDatabase, bool* pbGoodDatabase, const char** ppszBadTable);
    void VerifyGameTables (int iGameClass, int iGameNumber, bool* pbGoodDatabase);

    // Games
    int CleanupGame (int iGameClass, int iGameNumber, GameResult grResult, const char* pszWinnerName = NULL);
    int QuitEmpireFromGameInternal (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire);

    int CheckGameForAllyOut (int iGameClass, int iGameNumber, bool* pbAlly);
    int CheckGameForDrawOut (int iGameClass, int iGameNumber, bool* pbDraw);

    int DeleteShipFromDeadEmpire (const char* pszEmpireShips, const char* pszGameMap, 
        unsigned int iShipKey, unsigned int iPlanetKey);

    int GetGames (bool bOpen, int** ppiGameClass, int** ppiGameNumber, int* piNumGames);

    int AddToGameTable (int iGameClass, int iGameNumber);
    int RemoveFromGameTable (int iGameClass, int iGameNumber);

    int RuinEmpire (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage);

    GameObject* GetGameObject (int iGameClass, int iGameNumber);

    int PauseGameAt (int iGameClass, int iGameNumber, const UTCTime& tNow);
    int PauseGameInternal (int iGameClass, int iGameNumber, const UTCTime& tNow, bool bAdmin, bool bBroadcast);

    // Empires
    int DeleteEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, ReasonForRemoval rReason,
        const GameUpdateInformation* pUpdateInfo);

    int RemoveEmpire (int iEmpireKey);
    int UpdateEmpireString (int iEmpireKey, int iColumn, const char* pszString, size_t stMaxLen, bool* pbTruncated);

    int QueueDeleteEmpire (int iEmpireKey, int64 i64SecretKey);
    static int THREAD_CALL DeleteEmpireMsg (LongRunningQueryMessage* pMessage);

    // Planets
    int AddEmpiresToMap (int iGameClass, int iGameNumber, int* piEmpireKey, int iNumEmpires, 
        GameFairnessOption gfoFairness, bool* pbCommit);

    int CreateMapFromMapGeneratorData(int iGameClass, int iGameNumber, int* piNewEmpireKey, 
        unsigned int iNumNewEmpires, Variant* pvGameClassData, Variant* pvGameData, 
        Variant** ppvNewPlanetData, unsigned int iNumNewPlanets,
        unsigned int* piExistingPlanetKey, Variant** ppvExistingPlanetData, unsigned int iNumExistingPlanets,
        bool* pbCommit);

    int CreatePlanetFromMapGeneratorData (Variant* pvPlanetData,
        int iGameClass, int iGameNumber, int iEmpireKey, int iGameClassOptions,
        int* piMinX, int* piMaxX, int* piMinY, int* piMaxY, int* piNewPlanetKey);

    int InsertPlanetIntoGameEmpireData (int iGameClass, int iGameNumber, int iEmpireKey, 
        int iPlanetKey, const Variant* pvPlanetData, int iGameClassOptions);

    void AdvanceCoordinates (int iX, int iY, int* piX, int* piY, int cpDirection);

#ifdef _DEBUG

    int VerifyMap (int iGameClass, int iGameNumber);
    int DfsTraversePlanets (IReadTable* pGameMap, unsigned int iPlanetKey, int* piLink, bool* pbVisited, 
        unsigned int iNumPlanets);

    int VerifyUpdatedEmpireCount (int iGameClass, int iGameNumber);

#endif

    // Ships
    int DeleteShip (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey);

    int ChangeShipTypeOrMaxBR (const char* pszShips, const char* pszEmpireData, 
        int iEmpireKey, int iShipKey, int iOldShipType, int iNewShipType, float fBRChange);

    int ChangeShipCloakingState (int iShipKey, int iPlanetKey, bool bCloaked, 
        const char* strEmpireShips, const char* strEmpireMap, const char* strGameMap);

    int GetColonyOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
        unsigned int iPlanetKey, bool* pbColonize, bool* pbSettle);
        
    int GetTerraformerOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
        unsigned int iPlanetKey, const GameConfiguration& gcConfig, bool* pbTerraform, 
        bool* pbTerraformAndDismantle);

    int GetTroopshipOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
        unsigned int iPlanetKey, const GameConfiguration& gcConfig, bool* pbInvade, 
        bool* pbInvadeAndDismantle);

    int GetDoomsdayOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
        unsigned int iPlanetKey, const GameConfiguration& gcConfig, int iGameClassOptions, bool* pbAnnihilate);

    // Fleets
    int GetFleetSpecialActionMask (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
        unsigned int iFleetKey, const GameConfiguration& gcConfig, int* piMask);

    // Score
    int UpdateScoresOnNuke (int iNukerKey, int iNukedKey, const char* pszNukerName, 
        const char* pszNukedName, int iGameClass, int iGameNumber, int iUpdate, ReasonForRemoval reason,
        const char* pszGameClassName);

    int UpdateScoresOnSurrender (int iWinnerKey, int iLoserKey, const char* pszWinnerName, 
        const char* pszLoserName, int iGameClass, int iGameNumber, int iUpdate, const char* pszGameClassName);

    int UpdateScoresOn30StyleSurrender (int iLoserKey, const char* pszLoserName, int iGameClass, 
        int iGameNumber, int iUpdate, const char* pszGameClassName);

    int UpdateScoresOn30StyleSurrenderColonization (int iWinnerKey, int iPlanetKey, const char* pszWinnerName,
        int iGameClass, int iGameNumber, int iUpdate, const char* pszGameClassName);

    int UpdateScoresOnNuke (int iGameClass, int iGameNumber, int iNukerKey, int iNukedKey, ScoringChanges* pscChanges);
    int UpdateScoresOnSurrender (int iGameClass, int iGameNumber, int iWinnerKey, int iLoserKey, ScoringChanges* pscChanges);
    int UpdateScoresOn30StyleSurrender (int iGameClass, int iGameNumber, int iLoserKey, ScoringChanges* pscChanges);
    int UpdateScoresOn30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges);

    int UpdateScoresOnGameEnd (int iGameClass, int iGameNumber);
    int UpdateScoresOnWin (int iGameClass, int iGameNumber, int iEmpireKey);
    int UpdateScoresOnDraw (int iGameClass, int iGameNumber, int iEmpireKey);
    int UpdateScoresOnRuin (int iGameClass, int iGameNumber, int iEmpireKey);

    int CalculatePrivilegeLevel (int iEmpireKey);

    int AddNukeToHistory (NukeList nlNukeList, const char* pszGameClassName, int iGameNumber, 
        int iEmpireKey, const char* pszEmpireName, int iAlienKey,
        int iOtherEmpireKey, const char* pszOtherEmpireName, int iOtherAlienKey);

    int GetBridierScore (int iEmpireKey, int* piRank, int* piIndex);
    
    int TriggerBridierTimeBombIfNecessaryCallback();
    static int THREAD_CALL TriggerBridierTimeBombIfNecessaryMsg (LongRunningQueryMessage* pMessage);

    int ScanEmpiresOnScoreChanges();

    // Options
    int CheckForDelayedPause (int iGameClass, int iGameNumber, const UTCTime& tNow, bool* pbNewlyPaused);

    // Top Lists
    int UpdateTopListOnIncrease (ScoringSystem ssTopList, int iEmpireKey);
    int UpdateTopListOnDecrease (ScoringSystem ssTopList, int iEmpireKey);
    int UpdateTopListOnDeletion (ScoringSystem ssTopList, int iEmpireKey);

    static int THREAD_CALL UpdateTopListOnIncreaseMsg (LongRunningQueryMessage* pMessage);
    static int THREAD_CALL UpdateTopListOnDecreaseMsg (LongRunningQueryMessage* pMessage);
    static int THREAD_CALL UpdateTopListOnDeletionMsg (LongRunningQueryMessage* pMessage);

    int UpdateTopListOnIncrease (TopListQuery* pQuery);
    int UpdateTopListOnDecrease (TopListQuery* pQuery);
    int UpdateTopListOnDeletion (TopListQuery* pQuery);

    bool HasTopList (ScoringSystem ssTopList);
    int VerifyTopList (ScoringSystem ssTopList);

    // Updates
    int RunUpdate (int iGameClass, int iGameNumber, const UTCTime& tUpdateTime, bool* pbGameOver);

    int UpdateDiplomaticStatus (int iGameClass, int iGameNumber, unsigned int iNumEmpires, unsigned int* piEmpireKey, 
        bool* pbAlive, bool* pbSendFatalMessage, Variant* pvEmpireName, String* pstrUpdateMessage, 
        const char** pstrEmpireDip, 
        const char** pstrEmpireMap, const char* strGameMap, const char** pstrEmpireData, 
        int* piWinner, int* piLoser, unsigned int* piNumSurrenders, const char* pszGameClassName, 
        int iNewUpdateCount, Variant* pvGoodColor, Variant* pvBadColor);

    int UpdatePlanetPopulations (int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, float* pfAgRatio, 
        const char* strGameMap, const char** pstrEmpireData, const char** pstrEmpireMap, String* pstrUpdateMessage, 
        unsigned int* piPlanetKey, int iNumPlanets, int* piTotalMin, int* piTotalFuel, Variant* pvGoodColor, 
        Variant* pvBadColor, float fMaxAgRatio);

    int MoveShips (int iGameClass, int iGameNumber, int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, 
        Variant* pvEmpireName, float* pfMaintRatio, const char* strGameMap, const char** pstrEmpireShips, 
        const char** pstrEmpireDip, const char** pstrEmpireMap, const char** pstrEmpireFleets, const char** pstrEmpireData, 
        String* pstrUpdateMessage, const Variant* pvGoodColor, const Variant* pvBadColor, 
        const GameConfiguration& gcConfig, int iGameClassOptions);

    int MakeShipsFight (int iGameClass, int iGameNumber, const char* strGameMap, int iNumEmpires, 
        unsigned int* piEmpireKey, bool* pbAlive, Variant* pvEmpireName, const char** pstrEmpireShips, 
        const char** pstrEmpireDip, const char** pstrEmpireMap, String* pstrUpdateMessage, 
        const char** pstrEmpireData, float* pfFuelRatio, unsigned int iPlanetKey, 
        int* piTotalMin, int* piTotalFuel, bool bIndependence, const char* strIndependentShips, 
        Variant* pvGoodColor, Variant* pvBadColor, const GameConfiguration& gcConfig);

    int MinefieldExplosion (const char* strGameMap, const char** pstrEmpireData, 
        unsigned int iPlanetKey, unsigned int iNumEmpires, unsigned int* piEmpireKey, 
        int* piTotalMin, int* piTotalFuel);

    int MakeMinefieldsDetonate (int iGameClass, int iGameNumber, const char* strGameMap, 
        unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, Variant* pvEmpireName, 
        const char** pstrEmpireShips, const char** pstrEmpireMap, String* pstrUpdateMessage, 
        const char** pstrEmpireData, int* piTotalMin, int* piTotalFuel, bool bIndependence, 
        const char* strIndependentShips, Variant* pvGoodColor, Variant* pvBadColor, 
        const GameConfiguration& gcConfig);

    int UpdateFleetOrders (unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, 
        const char* strGameMap, const char** pstrEmpireShips, const char** pstrEmpireFleets, 
        const char** pstrEmpireMap, String* pstrUpdateMessage);

    int UpdateEmpiresEcon (int iGameClass, int iGameNumber, int iNumEmpires, unsigned int* piEmpireKey, 
        bool* pbAlive, int* piTotalMin, 
        int* piTotalFuel, int* piTotalAg, const Seconds& iUpdatePeriod, const UTCTime& tUpdateTime, 
        const char* strGameData, const char** pstrEmpireDip, const char** pstrEmpireData, const char** pstrEmpireShips, 
        int iNewUpdateCount, const char* strGameMap, float fMaxAgRatio,
        const GameConfiguration& gcConfig);

    int PerformSpecialActions (int iGameClass, int iGameNumber, int iNumEmpires, unsigned int* piEmpireKey, 
        const Variant* pvGoodColor, const Variant* pvBadColor, const Variant* pvEmpireName, bool* pbAlive, 
        unsigned int iNumPlanets, unsigned int* piPlanetKey, unsigned int* piOriginalPlanetOwner, 
        unsigned int* piOriginalNumObliterations, const char** pstrEmpireShips, const char** pstrEmpireFleets,
        const char** pstrEmpireData, const char** pstrEmpireMap, String* pstrUpdateMessage, 
        const char* strGameMap, const char* strGameData, int* piTotalAg, int* piTotalMin, int* piTotalFuel,
        const char** pstrEmpireDip, int* piObliterator, int* piObliterated, unsigned int* piNumObliterations, 
        const char* pszGameClassName, int iNewUpdateCount, int iGameClassOptions, unsigned int** ppiShipNukeKey, 
        unsigned int** ppiEmpireNukeKey, unsigned int* piNukedPlanetKey, unsigned int* piNumNukingShips, 
        unsigned int* piNumNukedPlanets, const GameConfiguration& gcConfig);

    int ProcessNukes (int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, bool* pbSendFatalMessage,
        const char* pszGameClassName,
        int iGameClass, int iGameNumber, int* piTotalAg, int* piTotalMin, int* piTotalFuel,
        int iNumNukedPlanets, unsigned int* piNumNukingShips, unsigned int* piNukedPlanetKey, 
        unsigned int** ppiEmpireNukeKey, unsigned int** ppiShipNukeKey, int* piObliterator, int* piObliterated, 
        unsigned int* piNumObliterations,
        Variant* pvEmpireName, const char** pstrEmpireDip, const char** pstrEmpireShips, const char** pstrEmpireMap, 
        String* pstrUpdateMessage, const char** pstrEmpireData, const char* strGameMap, 
        int iNewUpdateCount, const GameConfiguration& gcConfig);

    int AddShipSightings (unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive,
        String* pstrUpdateMessage, const Variant* pvEmpireName, bool bIndependence, unsigned int iNumPlanets, 
        unsigned int* piPlanetKey, const char* strGameMap, const char** pstrEmpireMap, 
        const char** pstrEmpireShips, const char* strIndependentShips);

    int SharePlanetsBetweenFriends (int iGameClass, int iGameNumber, 
        unsigned int iEmpireIndex1, unsigned int iEmpireIndex2,
        const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireData, 
        const char* pszGameMap, unsigned int iNumEmpires, unsigned int* piEmpireKey, int iDipLevel,
        bool bShareWithFriendsClosure);

    int SharePlanetBetweenFriends (int iGameClass, int iGameNumber, unsigned int iPlanetKey, 
        unsigned int iEmpireIndex,
        const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireData, 
        const char* pszGameMap, 
        unsigned int iNumEmpires, unsigned int* piEmpireKey, int iDipLevel, 
        Variant* pvAcquaintanceKey, unsigned int* piProxyKey, unsigned int iNumAcquaintances,
        bool bShareWithFriendsClosure);

    int ProcessGates (int iGameClass, int iGameNumber, unsigned int iNumEmpires, unsigned int* piEmpireKey, 
        bool* pbAlive, String* pstrUpdateMessage, const Variant* pvGoodColor, const Variant* pvBadColor,
        const Variant* pvEmpireName, unsigned int* piOriginalPlanetOwner, unsigned int* piOriginalNumObliterations,
        const char** pstrEmpireShips, const char** pstrEmpireFleets, const char** pstrEmpireMap, 
        const char** pstrEmpireData, const char** pstrEmpireDip,
        const char* strGameMap, const GameConfiguration& gcConfig, int iGameClassOptions);

    int GateShips (unsigned int iGaterEmpireIndex, const char* pszGaterEmpireName,
        unsigned int iGatedEmpireIndex, const char* pszGatedEmpireName, int iGatedEmpireKey,
        int iShipType, const char* pszGateName,
        unsigned int iOldPlanetKey, unsigned int iNewPlanetKey,
        const char* pszOldPlanetName, const char* pszNewPlanetName,
        int iSrcX, int iSrcY, int iDestX, int iDestY,
        const char* pszGameMap,
        const char* pszEmpireMap, const char* pszEmpireShips,
        const char* pszEmpireFleets, const char* pszEmpireDip,
        const char** pstrEmpireMap, const char** pstrEmpireDip,
        unsigned int iNumEmpires, bool* pbAlive,
        String* pstrUpdateMessage, const unsigned int* piEmpireKey, const Variant* pvEmpireName);

    int CreateNewPlanetFromBuilder (const GameConfiguration& gcConfig,
        int iGameClass, int iGameNumber, int iEmpireKey, float fBR,
        int iPlanetKey, int iX, int iY, int iDirection, const char* strGameMap, const char* strGameData, 
        const char* strGameEmpireMap, const char* strEmpireDip, unsigned int* piEmpireKey, 
        unsigned int iNumEmpires, const char** pstrEmpireMap, const char** pstrEmpireDip,
        const char** pstrEmpireData);

    int CheckForFirstContact (int iEmpireKey, int i, int iPlanetKey, const char* pszPlanetName,
        int iNewX, int iNewY, unsigned int iNumEmpires, const unsigned int* piEmpireKey, 
        const Variant* pvEmpireName, 
        const char* strEmpireDip, const char* strGameMap, const char** pstrEmpireDip, 
        String* pstrUpdateMessage);

    // Updates
    void GetNextUpdateTime (const UTCTime& tLastUpdate, Seconds sUpdatePeriod, int iNumUpdates,
        Seconds sFirstUpdateDelay, Seconds sAfterWeekendDelay, bool bWeekends, UTCTime* ptNextUpdateTime);

    void AdvanceWeekendTime (const UTCTime& tNextUpdateTime, Seconds sAfterWeekendDelay, UTCTime* ptNextUpdateTime);

    void GetLastUpdateTimeForPausedGame (const UTCTime& tNow, Seconds sSecondsUntilNextUpdate,
        Seconds sUpdatePeriod, int iNumUpdates, Seconds sFirstUpdateDelay, UTCTime* ptLastUpdateTime);

    // Top Lists
    int MoveEmpireUpInTopList (ScoringSystem ssTopList, int iEmpireKey, unsigned int iKey, 
        const Variant* pvOurData);

    int MoveEmpireDownInTopList (ScoringSystem ssTopList, int iEmpireKey, unsigned int iKey, 
        const Variant* pvOurData);

    int PrivateMoveEmpireUpInTopList (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows, 
        int iEmpireKey, unsigned int iKey, const Variant* pvOurData, bool* pbChanged);

    int PrivateMoveEmpireDownInTopList (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows, 
        int iEmpireKey, unsigned int iKey, const Variant* pvOurData, unsigned int* piNewNumRows, bool* pbChanged);

    int PrivateFindNewEmpireForTopList (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows, 
        bool bKeep, unsigned int* piNewNumRows);

    int PrivateFlushTopListData (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows);

    int InitializeEmptyTopList (ScoringSystem ssTopList);

    int AddEmpireToTopList (ScoringSystem ssTopList, int iEmpireKey, const Variant* pvOurData);

    // Planets
    int SetNewMinMaxIfNecessary (int iGameClass, int iGameNumber, int iEmpireKey, int iX, int iY);

    // GameClasses
    bool DoesGameClassHaveActiveGames (int iGameClass);

    // Database ops
    int FlushDatabasePrivate (int iEmpireKey);
    static int THREAD_CALL FlushDatabaseMsg (LongRunningQueryMessage* pMessage);

    int BackupDatabasePrivate (int iEmpireKey);
    static int THREAD_CALL BackupDatabaseMsg (LongRunningQueryMessage* pMessage);

    int RestoreDatabaseBackupPrivate (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion);
    static int THREAD_CALL RestoreDatabaseBackupMsg (LongRunningQueryMessage* pMessage);

    int DeleteDatabaseBackupPrivate (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion);
    static int THREAD_CALL DeleteDatabaseBackupMsg (LongRunningQueryMessage* pMessage);

    int PurgeDatabasePrivate (int iEmpireKey, int iCriteria);
    static int THREAD_CALL PurgeDatabaseMsg (LongRunningQueryMessage* pMessage);

    int DeleteOldDatabaseBackups();

    // Diplomacy
    int AddDiplomaticOption (int iGameClass, int iGameNumber, int iTargetEmpireKey,
        const char* pszEmpireData, const char* pszEmpireDiplomacy, unsigned int iEmpireDataColumn, 
        int iDiplomacyLevel, int iCurrentDiplomacyLevel, int piDipOptKey[], int* piNumOptions);

    int BuildDuplicateList (int* piDuplicateKeys, unsigned int iNumDuplicates, String* pstrDuplicateList);

    int GetCorrectTruceTradeAllianceCounts (int iGameClass, int iGameNumber, int iEmpireKey, 
        int* piNumTruces, int* piNumTrades, int* piNumAlliances);

    int CheckTruceTradeAllianceCounts (int iGameClass, int iGameNumber, int iEmpireKey);

    int ProcessSubjectiveViews (int iGameClass, int iGameNumber,
        unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, 
        const char* strGameMap, const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireShips);

    int ProcessEmpireSubjectiveView (int iGameClass, int iGameNumber,
        unsigned int iEmpireKey, const char* pszGameMap, 
        unsigned int iFullNumEmpires, unsigned int* piFullEmpireKey, const char** pstrEmpireDip, 
        const char** pstrEmpireShips, const char* pszEmpireMap, const char* pszWriteEmpireDip);

    // Messages
    int SendFatalUpdateMessage (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszGameClassName,
        const String& strUpdateMessage);

    int GetNumUnreadSystemMessagesPrivate (IReadTable* pMessages, unsigned int* piNumber);
    int GetNumUnreadGameMessagesPrivate (IReadTable* pMessages, unsigned int* piNumber);

    int GetMessageProperty (const char* strMessages, unsigned int iMessageKey, unsigned int iColumn, 
        Variant* pvProperty);

    int DeleteOverflowMessages (IWriteTable* pMessages, unsigned int iTimeStampColumn, 
        unsigned int iUnreadColumn, unsigned int iNumMessages, unsigned int iNumUnreadMessages, 
        unsigned int iMaxNumMessages, bool bCheckUnread);

    // Empires
    int WriteNextStatistics (int iGameClass, int iGameNumber, int iEmpireKey, int iTotalAg, int iBonusAg, 
        float fMaxAgRatio);

    int UpdateGameEmpireString (int iGameClass, int iGameNumber, int iEmpireKey, int iColumn, 
        const char* pszString, size_t stMaxLen, bool* pbTruncated);

    // Tournaments
    int SetTournamentString (unsigned int iTournamentKey, unsigned int iColumn, const char* pszString);

    int HandleEmpireTournamentAddition (int iEmpireKey, int iMessageKey, int iMessageType, bool bAccept);
    int AddEmpireToTournament (unsigned int iTournamentKey, int iInviteKey);

    // Associations
    int GetAssociations (char* pszAssoc, unsigned int** ppiEmpires, unsigned int* piNumAssoc);
    int DeleteAssociation (IWriteTable* pEmpires, unsigned int iEmpireKey, unsigned int iSecondEmpireKey);
    int RemoveDeadEmpireAssociations (IWriteTable* pEmpires, unsigned int iEmpireKey);

public:

    // Constructor/destructor
    GameEngine (
        
        const char* pszDatabaseName, 
        const char* pszHookLibrary,

        IAlmonasterUIEventSink* pUIEventSink, 

        IReport* pReport, 
        IPageSourceControl* pPageSourceControl,

        const SystemConfiguration& scConfig,
        const ChatroomConfig& ccConfig
        );
    
    ~GameEngine();

    void FreeData (void** ppData);
    void FreeData (Variant* pvData);
    void FreeData (Variant** ppvData);
    void FreeData (int* piData);
    void FreeData (unsigned int* piData);
    void FreeData (float* ppfData);
    void FreeData (char** ppszData);
    void FreeData (int64* pi64Data);
    void FreeKeys (unsigned int* piKeys);
    void FreeKeys (int* piKeys);

    int Initialize();

    int FlushDatabase (int iEmpireKey);
    int BackupDatabase (int iEmpireKey);
    int PurgeDatabase (int iEmpireKey, int iCriteria);

    int RestoreDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion);
    int DeleteDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion);

    IDatabase* GetDatabase();
    Chatroom* GetChatroom();
    IScoringSystem* GetScoringSystem (ScoringSystem ssScoringSystem);

    IReport* GetReport();

    const char* GetSystemVersion();

    int GetNewSessionId (int64* pi64SessionId);

    int GetGameConfiguration (GameConfiguration* pgcConfig);
    int GetMapConfiguration (MapConfiguration* pmcConfig);

    int SetGameConfiguration (const GameConfiguration& gcConfig);
    int SetMapConfiguration (const MapConfiguration& mcConfig);

    int GetSystemConfiguration (SystemConfiguration* pscConfig);

    // Rules
    int GetBattleRank (float fTech);
    int GetMilitaryValue (float fAttack);
    float GetMaintenanceRatio (int iMin, int iMaint, int iBuild);
    float GetFuelRatio (int iFuel, int iFuelUse);
    float GetAgRatio (int iAg, int iPop, float fMaxAgRatio);
    float GetShipNextBR (float fBR, float fMaint);
    int GetNextPopulation (int iPop, float fAgRatio);
    int GetEcon (int iFuel, int iMin, int iAg);
    float GetTechDevelopment (int iFuel, int iMin, int iMaint, int iBuild, int iFuelUse, float fMaxTechDev);
    int GetBuildCost (int iType, float fBR);
    int GetMaintenanceCost (int iType, float fBR);
    int GetFuelCost (int iType, float fBR);
    int GetColonizePopulation (int iShipBehavior, float fColonyMultipliedDepositFactor, 
        float fColonyExponentialDepositFactor, float fBR);
    int GetColonyPopulationBuildCost (int iShipBehavior, float fColonyMultipliedBuildFactor, 
        int iColonySimpleBuildFactor, float fBR);
    
    int GetTerraformerAg (float fTerraformerPlowFactor, float fBR);
    int GetTroopshipPop (float fTroopshipInvasionFactor, float fBR);
    int GetTroopshipFailurePopDecrement (float fTroopshipFailureFactor, float fBR);
    int GetTroopshipSuccessPopDecrement (float fTroopshipSuccessFactor, int iPop);
    int GetDoomsdayUpdates (float fDoomsdayAnnihilationFactor, float fBR);
    
    void GetBuilderNewPlanetResources (float fBR, float fBRDampening, float fMultiplier,
        int iAvgAg, int iAvgMin, int iAvgFuel, int* piNewAvgAg, int* piNewAvgMin, int* piNewAvgFuel);
    
    float GetGateBRForRange (float fRangeFactor, int iSrcX, int iSrcY, int iDestX, int iDestY);
    float GetCarrierDESTAbsorption (float fBR);
    bool IsMobileShip (int iShipType);

    float GetLateComerTechIncrease (int iPercentTechIncreaseForLatecomers, int iNumUpdates, float fMaxTechDev);
    int GetMaxPop (int iMin, int iFuel);

#ifdef _DEBUG
    void CheckTargetPop (int iGameClass, int iGameNumber, int iEmpireKey);
#endif

    int GetMaxNumDiplomacyPartners (int iGameClass, int iGameNumber, int iDiplomacyLevel, int* piMaxNumPartners);

    void CalculateTradeBonuses (int iNumTrades, int iTotalAg, int iTotalMin, int iTotalFuel,
        int iPercentFirstTradeIncrease, int iPercentNextTradeIncrease, 
        int* piBonusAg, int* piBonusMin, int* iBonusFuel);

    unsigned int GetNumFairDiplomaticPartners (unsigned int iMaxNumEmpires);

    bool GameAllowsDiplomacy (int iDiplomacyLevel, int iDip);
    bool IsLegalDiplomacyLevel (int iDiplomacyLevel);
    bool IsLegalPrivilege (int iPrivilege);

    int GetNextDiplomaticStatus (int iOffer1, int iOffer2, int iCurrentStatus);

    // Locks: don't use these unless you know what you're doing!
    int LockGameClass (int iGameClass, NamedMutex* pnmMutex);
    void UnlockGameClass (const NamedMutex& nmMutex);

    void LockGameClasses();
    void UnlockGameClasses();

    void LockSuperClasses();
    void UnlockSuperClasses();

    int LockEmpireBridier (int iEmpireKey, NamedMutex* pnmMutex);
    void UnlockEmpireBridier (const NamedMutex& nmMutex);

    void LockAlienIcons();
    void UnlockAlienIcons();

    int LockTournament (unsigned int iTournamentKey, NamedMutex* pnmMutex);
    void UnlockTournament (const NamedMutex& nmMutex);

    int LockEmpire (int iEmpireKey, NamedMutex* pnmMutex);
    void UnlockEmpire (const NamedMutex& nmMutex);

    int WaitGameReader (int iGameClass, int iGameNumber, int iEmpireKey, GameEmpireLock** ppgeLock);
    int SignalGameReader (int iGameClass, int iGameNumber, int iEmpireKey, GameEmpireLock* pgeLock);

    int WaitGameWriter (int iGameClass, int iGameNumber);
    int SignalGameWriter (int iGameClass, int iGameNumber);

    int WaitForUpdate (int iGameClass, int iGameNumber);
    int SignalAfterUpdate (int iGameClass, int iGameNumber);

    void LockGameConfigurationForReading();
    void UnlockGameConfigurationForReading();

    void LockGameConfigurationForWriting();
    void UnlockGameConfigurationForWriting();

    void LockMapConfigurationForReading();
    void UnlockMapConfigurationForReading();

    void LockMapConfigurationForWriting();
    void UnlockMapConfigurationForWriting();

    // Update
    int ForceUpdate (int iGameClass, int iGameNumber);

    int SetEmpireReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet);
    int SetEmpireNotReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet);

    // Themes
    int DoesThemeExist (int iThemeKey, bool* pbExist);
    int CreateTheme (Variant* pvData, unsigned int* piKey);
    int GetNumThemes (int* piNumThemes);
    int GetThemeKeys (int** ppiThemeKey, int* piNumKeys);
    int GetFullThemeKeys (int** ppiThemeKey, int* piNumKeys);
    int GetThemeData (int iThemeKey, Variant** ppvThemeData);
    int GetThemeName (int iThemeKey, Variant* pvThemeName);

    int GetThemeTextColor (int iThemeKey, Variant* pvColor);
    int GetThemeGoodColor (int iThemeKey, Variant* pvColor);
    int GetThemeBadColor (int iThemeKey, Variant* pvColor);
    int GetThemePrivateMessageColor (int iThemeKey, Variant* pvColor);
    int GetThemeBroadcastMessageColor (int iThemeKey, Variant* pvColor);
    int GetThemeTableColor (int iThemeKey, Variant* pvTableColor);

    int DeleteTheme (int iThemeKey);

    int SetThemeName (int iThemeKey, const char* pszThemeName);
    int SetThemeVersion (int iThemeName, const char* pszVersion);
    int SetThemeFileName (int iThemeKey, const char* pszFileName);
    int SetThemeAuthorName (int iThemeKey, const char* pszAuthorName);
    int SetThemeAuthorEmail (int iThemeKey, const char* pszAuthorEmail);
    int SetThemeBackground (int iThemeKey, bool bExists);
    int SetThemeLivePlanet (int iThemeKey, bool bExists);
    int SetThemeDeadPlanet (int iThemeKey, bool bExists);
    int SetThemeSeparator (int iThemeKey, bool bExists);
    int SetThemeButtons (int iThemeKey, bool bExists);
    int SetThemeDescription (int iThemeKey, const char* pszDescription);

    int SetThemeHorz (int iThemeKey, bool bExists);
    int SetThemeVert (int iThemeKey, bool bExists);

    int SetThemeTextColor (int iThemeKey, const char* pszColor);
    int SetThemeGoodColor (int iThemeKey, const char* pszColor);
    int SetThemeBadColor (int iThemeKey, const char* pszColor);
    int SetThemePrivateMessageColor (int iThemeKey, const char* pszColor);
    int SetThemeBroadcastMessageColor (int iThemeKey, const char* pszColor);
    int SetThemeTableColor (int iThemeKey, const char* pszTableColor);

    // SuperClasses
    int CreateSuperClass (const char* pszName, int* piKey);
    int DeleteSuperClass (int iSuperClassKey, bool* pbResult);
    int GetSuperClassKeys (int** ppiKey, int* piNumSuperClasses);
    int GetSuperClassKeys (int** ppiKey, Variant** ppvName, int* piNumSuperClasses);
    int GetSuperClassName (int iKey, Variant* pvName);
    int RenameSuperClass (int iKey, const char* pszNewName);

    // GameClasses
    int GetGameClassName (int iGameClass, char pszName [MAX_FULL_GAME_CLASS_NAME_LENGTH]);

    int GetGameClassUpdatePeriod (int iGameClass, Seconds* piNumSeconds);
    int GetNextGameNumber (int iGameClass, int* piGameNumber);
    
    int GetGameClassOptions (int iGameClass, int* piOptions);
    int GetSupportedMapGenerationTypes(int iGameClass, MapGeneration* pmgMapGen);
    void GetSupportedMapGenerationTypes(int iMinEmps, int iMaxEmps, int iMinPlanets, 
                                        MapGeneration* pmgMapGen);

    int GetMaxNumEmpires (int iGameClass, int* piMaxNumEmpires);
    int GetMaxNumAllies (int iGameClass, int* piMaxNumAllies);

    int DeleteGameClass (int iGameClass, bool* pbDeleted);
    int UndeleteGameClass (int iGameClassKey);

    int HaltGameClass (int iGameClass);
    int UnhaltGameClass (int iGameClass);

    int CreateGameClass (int iCreator, Variant* pvGameClassData, int* piGameClass);
    int GetSystemGameClassKeys (int** ppiKey, bool** ppbHalted, bool** ppbDeleted, int* piNumKeys);
    int GetStartableSystemGameClassKeys (int** ppiKey, int* piNumKeys);

    int GetGameClassSuperClassKey (int iGameClass, int* piSuperClassKey);
    int SetGameClassSuperClassKey (int iGameClass, int iSuperClassKey);

    int GetGameClassData (int iGameClass, Variant** ppvData);
    int GetGameClassOwner (int iGameClass, unsigned int* piOwner);
    int GetDevelopableTechs (int iGameClass, int* piInitialTechs, int* piDevelopableTechs);

    int GetNumEmpiresRequiredForGameToStart (int iGameClass, int* piNumEmpiresRequired);
    int DoesGameClassAllowPrivateMessages (int iGameClass, bool* pbPrivateMessages);
    int DoesGameClassHaveSubjectiveViews (int iGameClass, bool* pbSubjective);

    int GetGameClassMaxTechIncrease (int iGameClass, float* pfMaxTechIncrease);
    int GetGameClassVisibleBuilds (int iGameClass, bool* pbVisible);
    int GetGameClassVisibleDiplomacy (int iGameClass, bool* pbVisible);
    int GetGameClassDiplomacyLevel (int iGameClass, int* piDiplomacy);

    int GetMinNumSecsPerUpdateForSystemGameClass (int* piMinNumSecsPerUpdate);
    int GetMaxNumSecsPerUpdateForSystemGameClass (int* piMaxNumSecsPerUpdate);
    int GetMaxNumEmpiresForSystemGameClass (int* piMaxNumEmpires);
    int GetMaxNumPlanetsForSystemGameClass (int* piMaxNumPlanets);

    int GetMinNumSecsPerUpdateForPersonalGameClass (int* piMinNumSecsPerUpdate);
    int GetMaxNumSecsPerUpdateForPersonalGameClass (int* piMaxNumSecsPerUpdate);
    int GetMaxNumEmpiresForPersonalGameClass (int* piMaxNumEmpires);
    int GetMaxNumPlanetsForPersonalGameClass (int* piMaxNumPlanets);

    int SetMinNumSecsPerUpdateForSystemGameClass (int iMinNumSecsPerUpdate);
    int SetMaxNumSecsPerUpdateForSystemGameClass (int iMaxNumSecsPerUpdate);
    int SetMaxNumEmpiresForSystemGameClass (int iMaxNumEmpires);
    int SetMaxNumPlanetsForSystemGameClass (int iMaxNumPlanets);

    int SetMinNumSecsPerUpdateForPersonalGameClass (int iMinNumSecsPerUpdate);
    int SetMaxNumSecsPerUpdateForPersonalGameClass (int iMaxNumSecsPerUpdate);
    int SetMaxNumEmpiresForPersonalGameClass (int iMaxNumEmpires);
    int SetMaxNumPlanetsForPersonalGameClass (int iMaxNumPlanets);

    int GetGameClassProperty (int iGameClass, unsigned int iProperty, Variant* pvProperty);

    // Games
    int GetGameUpdateData (int iGameClass, int iGameNumber, int* piSecondsSince, int* piSecondsUntil, 
        int* piNumUpdates, int* piGameState, Vector<UTCTime>* pvecUpdateTimes = NULL);

    int ResetAllGamesUpdateTime();
    int ResetGameUpdateTime (int iGameClass, int iGameNumber);

    int GetGameState (int iGameClass, int iGameNumber, int* piGameState);
    int GetGameCreationTime (int iGameClass, int iGameNumber, UTCTime* ptCreationTime);

    int CheckGameForEndConditions (int iGameClass, int iGameNumber, const char* pszAdminMessage, bool* pbEndGame);
    int DeleteGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, int iReason);
    
    int GetNumActiveGames (int* piNumGames);
    int GetNumOpenGames (int* piNumGames);
    int GetNumClosedGames (int* piNumGames);

    int AreAllEmpiresIdle (int iGameClass, int iGameNumber, bool* pbIdle);

    int GetActiveGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames);
    int GetOpenGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames);
    int GetClosedGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames);

    int IsGameOpen (int iGameClass, int iGameNumber, bool* pbOpen);
    int HasGameStarted (int iGameClass, int iGameNumber, bool* pbStarted);

    int IsGamePasswordProtected (int iGameClass, int iGameNumber, bool* pbProtected);
    int SetGamePassword (int iGameClass, int iGameNumber, const char* pszNewPassword);

    int GetGameProperty(int iGameClass, int iGameNumber, unsigned int iProp, Variant* pvProp);
    int SetGameProperty(int iGameClass, int iGameNumber, unsigned int iProp, const Variant& vProp);

    int CreateGame (int iGameClass, int iEmpireCreator, const GameOptions& goGameOptions, int* piGameNumber);
    int EnterGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszPassword,
        const GameOptions* pgoGameOptions, int* piNumUpdates, bool bSendMessages, bool bCreatingGame, 
        bool bCheckSecurity, NamedMutex* pempireMutex, bool* pbUnlocked);

    int SetEnterGameIPAddress (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszIPAddress);

    int DoesGameExist (int iGameClass, int iGameNumber, bool* pbExists);
    int GetNumUpdates (int iGameClass, int iGameNumber, int* piNumUpdates);
    
    int GetNumUpdatesBeforeGameCloses (int iGameClass, int iGameNumber, int* piNumUpdates);
    int GetGameOptions (int iGameClass, int iGameNumber, int* piOptions);
    int GetFirstUpdateDelay (int iGameClass, int iGameNumber, Seconds* psDelay);
    
    int GetNumEmpiresInGame (int iGameClass, int iGameNumber, int* piEmpires);
    int GetNumDeadEmpiresInGame (int iGameClass, int iGameNumber, unsigned int* piDeadEmpires);
    int GetNumEmpiresNeededForGame (int iGameClass, int* piNumEmpiresNeeded);
    int IsEmpireInGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbInGame);
    int GetNumUpdatedEmpires (int iGameClass, int iGameNumber, int* piUpdatedEmpires);
    int GetEmpiresInGame (int iGameClass, int iGameNumber, Variant** ppiEmpireKey, int* piNumEmpires);

    int IsGamePaused (int iGameClass, int iGameNumber, bool* pbPaused);
    int IsGameAdminPaused (int iGameClass, int iGameNumber, bool* pbAdminPaused);
    int IsSpectatorGame (int iGameClass, int iGameNumber, bool* pbSpectatorGame);

    int AddToLatestGames (const Variant* pvColumns, unsigned int iTournamentKey);
    int AddToLatestGames (const char* pszTable, const Variant* pvColumns);

    void GetGameClassGameNumber (const char* pszGameData, int* piGameClass, int* piGameNumber);
    void GetGameClassGameNumber (int iGameClass, int iGameNumber, char* pszGameData);

    int PauseAllGames();
    int UnpauseAllGames();

    int PauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast);
    int UnpauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast);

    int LogEmpireIntoGame (int iGameClass, int iGameNumber, int iEmpireKey, int* piIdleUpdates);

    int RuinGame (int iGameClass, int iGameNumber, const char* pszWinnerName);
    int ResignGame (int iGameClass, int iGameNumber);

    int GetResignedEmpiresInGame (int iGameClass, int iGameNumber, int** ppiEmpireKey, int* piNumResigned);
    int UnresignEmpire (int iGameClass, int iGameNumber, int iEmpireKey, int iAdminKey);

    int GetGameEntryRestrictions (int iGameClass, int iGameNumber, int* piOptions, 
        Variant pvRestrictionMin [NUM_ENTRY_SCORE_RESTRICTIONS], 
        Variant pvRestrictionMax [NUM_ENTRY_SCORE_RESTRICTIONS]);

    int GameAccessCheck (int iGameClass, int iGameNumber, int iEmpireKey, 
        const GameOptions* pgoGameOptions, GameAction gaAction, 
        bool* pbAccess, GameAccessDeniedReason* prAccessDeniedReason);

    int GetBridierRankPotentialGainLoss (int iGameClass, int iGameNumber, int iEmpireKey, int* piGain, int* piLoss);

    int GetNumEmpiresInGames (unsigned int* piNumEmpires);
    int GetNumRecentActiveEmpiresInGames (unsigned int* piNumEmpires);

    // GameEmpireData
    int QuitEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire = NO_KEY);
    int ResignEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey);
    int SurrenderEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, SurrenderType sType);

    int HasEmpireResignedFromGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbResigned);

    // Empires
    int CreateEmpire (const char* pszEmpireName, const char* pszPassword, int iPrivilege, unsigned int iParentKey, 
        bool bBypassDisabled, unsigned int* piEmpireKey);

    int GetEmpireName (int iEmpireKey, Variant* pvName);
    int GetEmpireName (int iEmpireKey, char pszName [MAX_EMPIRE_NAME_LENGTH + 1]);
    int SetEmpireName (int iEmpireKey, const char* pszName);
    
    int SetEmpirePassword (int iEmpireKey, const char* pszPassword);
    int ChangeEmpirePassword (int iEmpireKey, const char* pszPassword);

    int SetEmpireMaxNumSavedSystemMessages (int iEmpireKey, unsigned int iMaxNumSavedMessages);

    int UpdateEmpireQuote (int iEmpireKey, const char* pszQuote, bool* pbTruncated);
    int UpdateEmpireVictorySneer (int iEmpireKey, const char* pszSneer, bool* pbTruncated);

    int SendVictorySneer (int iWinnerKey, const char* pszWinnerName, int iLoserKey);

    int SetEmpireBackgroundKey (int iEmpireKey, int iBackgroundKey);
    int SetEmpireLivePlanetKey (int iEmpireKey, int iLivePlanetKey);
    int SetEmpireDeadPlanetKey (int iEmpireKey, int iDeadPlanetKey);
    int SetEmpireButtonKey (int iEmpireKey, int iButtonKey);
    int SetEmpireSeparatorKey (int iEmpireKey, int iSeparatorKey);
    int SetEmpireHorzKey (int iEmpireKey, int iHorzKey);
    int SetEmpireVertKey (int iEmpireKey, int iVertKey);

    int SetEmpireColorKey (int iEmpireKey, int iColorKey);

    int GetEmpireLivePlanetKey (int iEmpireKey, int* piLivePlanetKey);
    int GetEmpireDeadPlanetKey (int iEmpireKey, int* piDeadPlanetKey);

    int SetEmpireThemeKey (int iEmpireKey, int iThemeKey);

    int GetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int* piMaxNumSavedMessages);

    int DeleteEmpire (int iEmpireKey, int64* pi64SecretKey, bool bMarkOnFailure, bool bDeletePersonal);
    int ObliterateEmpire (unsigned int iEmpireKey, int64 i64SecretKey, unsigned int iKillerEmpire);

    int RemoveEmpireFromGame (int iGameClass, int iGameNumber, unsigned int iEmpireKey, unsigned int iKillerEmpire);

    int DoesEmpireExist (const char* pszName, bool* pbExists, unsigned int* piEmpireKey, Variant* pvEmpireName, int64* piSecretKey);
    int DoesEmpireExist (unsigned int iEmpireKey, bool* pbExists, Variant* pvEmpireName);

    int CheckSecretKey (unsigned int iEmpireKey, int64 i64SecretKey, bool* pbMatch, int64* pi64SessionId, Variant* pvIPAddress);

    int IsPasswordCorrect (int iEmpireKey, const char* pszPassword);

    int LoginEmpire (int iEmpireKey, const char* pszBrowser, const char* pszIPAddress);
    
    int GetNumEmpiresOnServer (int* piNumEmpires);
    int GetDefaultEmpireShipNames (int iEmpireKey, const char*** pppszShipName);

    int UndeleteEmpire (int iEmpireKey);
    int BlankEmpireStatistics (int iEmpireKey);

    int GetEmpirePersonalGameClasses (int iEmpireKey, int** ppiGameClassKey, Variant** ppvName, int* piNumKeys);

    int GetEmpireData (int iEmpireKey, Variant** ppvEmpData, int* piNumActiveGames);
    int GetEmpireActiveGames (int iEmpireKey, int** ppiGameClass, int** ppiGameNumber, int* piNumGames);

    int GetEmpirePrivilege (int iEmpireKey, int* piPrivilege);
    int SetEmpirePrivilege (int iEmpireKey, int iPrivilege);

    int GetEmpireAlmonasterScore (int iEmpireKey, float* pfAlmonasterScore);
    int SetEmpireAlmonasterScore (int iEmpireKey, float fAlmonasterScore);

    int GetEmpirePassword (int iEmpireKey, Variant* pvPassword);

    int GetEmpireDataColumn (int iEmpireKey, unsigned int iColumn, Variant* pvData);

    int GetNumLogins (int iEmpireKey, int* piNumLogins);

    int GetEmpireIPAddress (int iEmpireKey, Variant* pvIPAddress);
    int SetEmpireIPAddress (int iEmpireKey, const char* pszIPAddress);

    int GetEmpireSessionId (int iEmpireKey, int64* pi64SessionId);
    int SetEmpireSessionId (int iEmpireKey, int64 i64SessionId);

    int GetEmpireDefaultBuilderPlanet (int iEmpireKey, int* piDefaultBuildPlanet);
    int SetEmpireDefaultBuilderPlanet (int iEmpireKey, int iDefaultBuildPlanet);

    int ResetEmpireSessionId (int iEmpireKey);
    int EndResetEmpireSessionId (int iEmpireKey);

    int GetEmpireOptions (int iEmpireKey, int* piOptions);
    int GetEmpireOptions2 (int iEmpireKey, int* piOptions);

    int GetEmpireLastBridierActivity (int iEmpireKey, UTCTime* ptTime);

    int GetEmpireOption (int iEmpireKey, unsigned int iFlag, bool* pbOption);
    int SetEmpireOption (int iEmpireKey, unsigned int iFlag, bool bSet);

    int GetEmpireOption2 (int iEmpireKey, unsigned int iFlag, bool* pbOption);
    int SetEmpireOption2 (int iEmpireKey, unsigned int iFlag, bool bSet);

    int GetEmpireProperty (int iEmpireKey, unsigned int iProperty, Variant* pvProperty);
    int SetEmpireProperty (int iEmpireKey, unsigned int iProperty, const Variant& vProperty);

    int IsEmpireIdleInSomeGame (int iEmpireKey, bool* pfIdle);

    //
    // Game Empire Data
    //

    int GetEmpireOptions (int iGameClass, int iGameNumber, int iEmpireKey, int* piOptions);

    int GetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, bool* pbOption);
    int SetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, bool bOption);

    int GetEmpirePartialMapData (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbPartialMaps, 
        unsigned int* piCenterKey, unsigned int* piXRadius, unsigned int* piRadiusY);

    int GetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
        int* piDefaultBuildPlanet, int* piResolvedPlanetKey);
    int SetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iDefaultBuildPlanet);

    int GetEmpireBR (int iGameClass, int iGameNumber, int iEmpireKey, int* piBR);
    int GetEmpireMaintenanceRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfMaintenanceRatio);
    int GetEmpireNextMaintenanceRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfNextMaintenanceRatio);

    int GetEmpireDefaultMessageTarget (int iEmpireKey, int* piMessageTarget);
    int SetEmpireDefaultMessageTarget (int iEmpireKey, int iMessageTarget);

    int GetEmpireGameProperty (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iColumn,
        Variant* pvProperty);

    int SetEmpireGameProperty (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iColumn,
        const Variant& vProperty);

    // System Messages
    int SendSystemMessage (int iEmpireKey, const char* pszMessage, int iSource, int iFlags);
    int DeliverSystemMessage (int iEmpireKey, const Variant* pvData);

    int GetNumSystemMessages (int iEmpireKey, unsigned int* piNumber);
    int GetNumUnreadSystemMessages (int iEmpireKey, unsigned int* piNumber);
    int SendMessageToAll (int iEmpireKey, const char* pszMessage);
    int GetSavedSystemMessages (int iEmpireKey, unsigned int** ppiMessageKey, Variant*** pppvMessage, 
        unsigned int* piNumMessages);

    int GetUnreadSystemMessages (int iEmpireKey, Variant*** pppvMessage, unsigned int** ppiMessageKey, 
        unsigned int* piNumMessages);
    
    int DeleteSystemMessage (int iEmpireKey, unsigned int iKey);

    int GetSystemMessageProperty (int iEmpireKey, unsigned int iMessageKey, unsigned int iColumn, 
        Variant* pvProperty);

    // Game Messages
    int GetNumGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumber);
    int GetNumUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumber);

    int GetSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int** ppiMessageKey,
        Variant*** ppvData, unsigned int* piNumMessages);
    
    int SendGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, int iSource, 
        int iFlags, const UTCTime& tSendTime);
    
    int GetUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, Variant*** pppvMessage, 
        unsigned int* piNumMessages);
    
    int DeleteGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iMessageKey);
    int BroadcastGameMessage (int iGameClass, int iGameNumber, const char* pszMessage, int iSourceKey, 
        int iFlags);

    int GetGameMessageProperty (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iMessageKey, 
        unsigned int iColumn, Variant* pvProperty);

    // Planets
    int GetPlanetCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, int* piPlanetX, int* piPlanetY);

    int GetPlanetProperty (int iGameClass, int iGameNumber, unsigned int iPlanetKey, unsigned int iProperty, 
        Variant* pvProperty);

    int GetPlanetKeyFromCoordinates (int iGameClass, int iGameNumber, int iX, int iY, int* piPlanetKey);
    int GetEmpirePlanetIcons (int iEmpireKey, unsigned int* piLivePlanetKey, unsigned int* piLiveDeadPlanetKey);

    int GetNeighbourPlanetKey (int iGameClass, int iGameNumber, int iPlanetKey, int iDirection, 
        int* piPlanetKey);
    int HasEmpireExploredPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
        bool* pbExplored);

    static void GetCoordinates (const char* pszCoord, int* piX, int* piY);
    static void GetCoordinates (int iX, int iY, char* pszCoord);

    int GetLowestDiplomacyLevelForShipsOnPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
        int iPlanetKey, bool bVisibleBuilds, Variant* pvEmpireKey, unsigned int& iNumEmpires,
        int* piNumForeignShipsOnPlanet, int* piDiplomacyLevel, Variant** ppvEmpireKey);

    int GetPlanetPopulationWithColonyBuilds (unsigned int iGameClass, unsigned int iGameNumber,
        unsigned int iEmpireKey, unsigned int iPlanetKey, unsigned int* piPop);

    // Score
    int SendScoringChangeMessages (int iGameClass, int iGameNumber, int iNukerKey, int iNukedKey, int iUpdate, 
        ReasonForRemoval reason, const char* pszGameClassName, ScoringChanges* pscChanges);

    int GetNumEmpiresInNukeHistory (int iEmpireKey, int* piNumNukes, int* piNumNuked);
    int GetNukeHistory (int iEmpireKey, int* piNumNuked, Variant*** pppvNukedData, int* piNumNukers, 
        Variant*** pppvNukerData);

    int GetSystemNukeHistory (int* piNumNukes, Variant*** pppvNukedData);
    int GetSystemLatestGames (int* piNumGames, Variant*** pppvGameData);

    int TriggerBridierTimeBombIfNecessary();

    int GetBridierTimeBombScanFrequency (Seconds* piFrequency);
    int SetBridierTimeBombScanFrequency (Seconds iFrequency);

    // System Config
    int GetDefaultUIKeys (unsigned int* piBackground, unsigned int* piLivePlanet, 
        unsigned int* piDeadPlanet, unsigned int* piButtons, unsigned int* piSeparator, 
        unsigned int* piHorz, unsigned int* piVert, unsigned int* piColor);

    int SetScoreForPrivilege (Privilege privLevel, float fScore);
    int GetScoreForPrivilege (Privilege privLevel, float* pfScore);

    int GetDefaultGameOptions (int iGameClass, GameOptions* pgoOptions);

    int GetSystemOptions (int* piOptions);
    int SetSystemOption (int iOption, bool bFlag);

    int GetSystemProperty (int iColumn, Variant* pvProperty);
    int SetSystemProperty (int iColumn, const Variant& vProperty);

    int GetDefaultShipName (int iTech, Variant* pvShipName);
    int SetDefaultShipName (int iShipKey, const char* pszShipName);

    // Aliens
    int GetNumAliens (int* piNumAliens);
    int GetAlienKeys (Variant*** pppvData, int* piNumAliens);
    int CreateAlienIcon (int iAlienKey, const char* pszAuthorName);
    int DeleteAlienIcon (int iAlienKey);

    int GetAlienAuthorName (int iAlienKey, Variant* pvAuthorName);

    int SetEmpireAlienKey (int iEmpireKey, int iAlienKey);

    // Top Lists
    int GetTopList (ScoringSystem ssListType, Variant*** pppvData, unsigned int* piNumEmpires);
    int RebuildTopLists();

    // Search
    int PerformMultipleSearch (const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, 
        unsigned int* piStopKey);

    // Updates
    int CheckGameForUpdates (int iGameClass, int iGameNumber, bool fUpdateCheckTime, bool* pbUpdate);
    int CheckAllGamesForUpdates (bool fUpdateCheckTime);

    static int THREAD_CALL CheckAllGamesForUpdatesMsg (LongRunningQueryMessage* pMessage);

    //////////
    // Game //
    //////////

    // Tech
    int GetNumAvailableTechs (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumAvailableTechs);
    int GetDevelopedTechs (int iGameClass, int iGameNumber, int iEmpireKey, int* piTechDevs, int* piTechUndevs);
    int RegisterNewTechDevelopment (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey);
    int GetDefaultEmpireShipName (int iEmpireKey, int iTechKey, Variant* pvDefaultShipName);
    int SetDefaultEmpireShipName (int iEmpireKey, int iTechKey, const char* pszDefaultShipName);
    int GetNumTechs (int iTechBitmap);

    // Options
    int GetEmpireMaxNumSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumMessages);
    int SetEmpireMaxNumSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iNumMessages);

    int GetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, bool* pbIgnore);
    int SetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, bool bIgnore);

    int GetEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, Variant* pvNotepad);

    int UpdateGameEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszNotepad, 
        bool* pbTruncated);

    int SearchForDuplicates (int iGameClass, int iGameNumber, unsigned int iSystemEmpireDataColumn,
        unsigned int iGameEmpireDataColumn, int** ppiDuplicateKeys, unsigned int** ppiNumDuplicatesInList, 
        unsigned int* piNumDuplicates);

    int DoesEmpireHaveDuplicates (int iGameClass, int iGameNumber, int iEmpireKey, int iSystemEmpireDataColumn,
        int** ppiDuplicateKeys, unsigned int* piNumDuplicates);

    int GetEmpireDefaultMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int* piMessageTarget);
    int SetEmpireDefaultMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int iMessageTarget);

    // Diplomacy
    int GetKnownEmpireKeys (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvEmpireKey, 
        int* piNumEmpires);
    int GetDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
        int* piWeOffer, int* piTheyOffer, int* piCurrent, bool* pbMet);
    int GetDiplomaticOptions (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
        int piDipOptKey[], int* piSelected, int* piNumOptions);
    int UpdateDiplomaticOffer (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, int iDipOffer);

    int RequestPause (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState);
    int RequestPauseDuringUpdate (int iGameClass, int iGameNumber, int iEmpireKey);
    int RequestNoPause (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState);

    int IsEmpireRequestingPause (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbPause);
    int GetNumEmpiresRequestingPause (int iGameClass, int iGameNumber, unsigned int* piNumEmpires);

    int RequestDraw (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState);
    int RequestNoDraw (int iGameClass, int iGameNumber, int iEmpireKey);
    int IsEmpireRequestingDraw (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbDraw);
    int GetNumEmpiresRequestingDraw (int iGameClass, int iGameNumber, unsigned int* piNumEmpires);

    int GetNumEmpiresAtDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey,
        int* piWar, int* piTruce, int* piTrade, int* piAlliance);

    int GetNumEmpiresAtDiplomaticStatusNextUpdate (int iGameClass, int iGameNumber, int iEmpireKey, 
        int* piWar, int* piTruce, int* piTrade, int* piAlliance);

    int GetEmpiresAtDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey,
        int piWar[], int* piNumWar, int piTruce[], int* piNumTruce, int piTrade[], int* piNumTrade, 
        int piAlliance[], int* piNumAlliance);

    int GetLastUsedMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int* piLastUsedMask,
        int** ppiLastUsedProxyKeyArray, int* piNumLastUsed);

    int SetLastUsedMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int iLastUsedMask,
        int* piLastUsedKeyArray, int iNumLastUsed);

    // Info / End Turn
    int GetEmpireGameInfo (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvEmpData,
        int* piNumShips, int* piBattleRank, int* piMilVal, float* pfTechDev, float* pfMaintRatio, 
        float* pfFuelRatio, float* pfAgRatio, float* pfHypMaintRatio, float* pfHypFuelRatio, 
        float* pfHypAgRatio, float* pfNextTechDev, int* piShipLimit);

    // Map
    int GetMapLimits (int iGameClass, int iGameNumber, int iEmpireKey, int* piMinX, int* piMaxX, int* piMinY, int* piMaxY);
    int GetMapLimits (int iGameClass, int iGameNumber, int* piMinX, int* piMaxX, int* piMinY, int* piMaxY);

    int GetVisitedPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvPlanetKey, 
        unsigned int** ppiEmpireMapKey, unsigned int* piNumKeys);

    int GetNumVisitedPlanets (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int* piNumVisitedPlanets);
    int HasEmpireVisitedPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, bool* pbVisited);

    int GetPlanetShipOwnerData (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
        int iPlanetProxyKey, unsigned int iTotalNumShips, bool bVisibleBuilds, bool bIndependence, 
        unsigned int** ppiShipOwnerData);

    int GetPlanetName (int iGameClass, int iGameNumber, int iPlanetKey, Variant* pvPlanetName);
    int GetPlanetNameWithSecurity (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey,  
        Variant* pvPlanetName);

    int DoesPlanetExist (int iGameClass, int iGameNumber, int iPlanetKey, bool* pbExists);

    int GetPlanetNameWithCoordinates (const char* pszGameMap, unsigned int iPlanetKey, char pszName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH]);
    int GetPlanetNameWithCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, char pszName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH]);

    int RenamePlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, const char* pszNewName);
    int SetPlanetMaxPop (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, int iNewMaxPop);

    int GetGameAveragePlanetResources (int iGameClass, int iGameNumber, int* piAg, int* piMin, int* piFuel);
    int GetEmpireAgRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfAgRatio);

    int GetVisitedSurroundingPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int iCenterKey, 
        Variant* pvPlanetKey, int* piProxyKey, int* piNumPlanets, int* piCenterX, int* piCenterY, 
        int* piMinX, int* piMaxX, int* piMinY, int* piMaxY);

    // Build
    int GetRatioInformation (int iGameClass, int iGameNumber, int iEmpireKey, RatioInformation* pRatInfo);

    int GetBuilderPlanetKeys (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
        unsigned int** ppiBuilderKey, unsigned int* piNumBuilders);

    int BuildNewShips (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey, int iNumShips, 
        const char* pszShipName, float fBR, int iPlanetKey, int iFleetKey, int* piNumShipsBuilt, 
        bool* pbBuildReduced);

    int GetNumBuilds (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumBuilds);
    int CancelAllBuilds (int iGameClass, int iGameNumber, int iEmpireKey);

    int GetBuildLocations (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
        unsigned int iPlanetKey, BuildLocation** ppblBuildLocation, unsigned int* piNumLocations);

    int IsPlanetBuilder (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey,
        unsigned int iPlanetKey, bool* pbBuilder);

    // Ships
    int GetShipOrders (unsigned int iGameClass, unsigned int iGameNumber, unsigned int iEmpireKey, 
        unsigned int iShipKey, const ShipOrderShipInfo* pShipInfo, const ShipOrderGameInfo* pGameInfo, 
        const ShipOrderPlanetInfo* pPlanetInfo, const GameConfiguration& gcConfig, 
        const BuildLocation* pblLocations, unsigned int iNumLocations,
        ShipOrder** ppsoOrder, unsigned int* piNumOrders, int* piSelectedOrder);

    int UpdateShipName (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, const char* pszNewName);
    int UpdateShipOrders (unsigned int iGameClass, unsigned int iGameNumber, unsigned int iEmpireKey, 
        unsigned int iShipKey, const ShipOrder& soOrder);

    int GetNumShips (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumShips);

    int MoveShip (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, unsigned int iShipKey, 
        unsigned int iPlanetKey, unsigned int iFleetKey);

    int GetUnaffiliatedMobileShipsAtPlanet (unsigned int iGameClass, unsigned int iGameNumber,
        unsigned int iEmpireKey, unsigned int iPlanetKey, unsigned int** ppiShipKey, unsigned int* piNumShips);

    int HasUnaffiliatedMobileShipsAtPlanet (unsigned int iGameClass, unsigned int iGameNumber, 
        unsigned int iEmpireKey, unsigned int iPlanetKey, bool* pbFlag);

    void FreeShipOrders (ShipOrder* psoOrders, unsigned int iNumOrders);

    // Fleets
    int GetEmpireFleetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiFleetKeys, 
        int* piNumFleets);

    int GetNumFleets (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumFleets);

    int GetFleetProperty (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, unsigned int iProperty,
        Variant* pvProperty);

    int GetFleetFlag (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int iFlag, bool* pbFlag);
    int SetFleetFlag (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int iFlag, bool bFlag);

    int GetNewFleetLocations (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiLocationKey, 
        int* piNumLocations);

    int CreateNewFleet (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszFleetName, 
        int iPlanetKey, unsigned int* piFleetKey);

    int GetFleetOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, unsigned int iFleetKey, 
        const GameConfiguration& gcConfig, FleetOrder** ppfoOrders, unsigned int* piNumOrders, 
        unsigned int* piSelected);

    void FreeFleetOrders (FleetOrder* pfoOrders, unsigned int iNumOrders);

    int UpdateFleetName (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
        const char* pszNewName);

    int UpdateFleetOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, 
        unsigned int iFleetKey, const FleetOrder& foOrder);

    int GetNumShipsInFleet (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
        unsigned int* piNumShips, unsigned int* piNumBuildShips);

    int MergeFleets (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, unsigned int iSrcKey, 
        unsigned int iDestKey, unsigned int iPlanetKey);

    int CreateRandomFleet (unsigned int iGameClass, unsigned int iGameNumber, unsigned int iEmpireKey, 
        unsigned int iPlanetKey, unsigned int* piFleetKey);

    // Tournaments
    int GetOwnedTournaments (int iEmpireKey, unsigned int** ppiTournamentKey, Variant** ppvName, unsigned int* piNumTournaments);
    int GetJoinedTournaments (int iEmpireKey, unsigned int** ppiTournamentKey, Variant** ppvName, unsigned int* piNumTournaments);

    int CreateTournament (Variant* pvTournamentData, unsigned int* piTournamentKey);
    int DeleteTournament (int iEmpireKey, unsigned int iTournamentKey, bool bOwnerDeleted);

    int GetTournamentData (unsigned int iTournamentKey, Variant** ppvTournamentData);
    int GetTournamentName (unsigned int iTournamentKey, Variant* pvTournamentName);
    int GetTournamentOwner (unsigned int iTournamentKey, unsigned int* piOwnerKey);

    int GetTournamentDescription (unsigned int iTournamentKey, Variant* pvTournamentDesc);
    int SetTournamentDescription (unsigned int iTournamentKey, const char* pszTournamentDesc);

    int GetTournamentUrl (unsigned int iTournamentKey, Variant* pvTournamentUrl);
    int SetTournamentUrl (unsigned int iTournamentKey, const char* pszTournamentUrl);

    int GetTournamentNews (unsigned int iTournamentKey, Variant* pvTournamentNews);
    int SetTournamentNews (unsigned int iTournamentKey, const char* pszTournamentNews);

    int InviteEmpireIntoTournament (unsigned int iTournamentKey, int iOwnerKey, int iSourceKey, int iInviteKey);
    int InviteSelfIntoTournament (unsigned int iTournamentKey, int iEmpireKey);

    int RespondToTournamentInvitation (int iInviteKey, int iMessageKey, bool bAccept);
    int RespondToTournamentJoinRequest (int iEmpireKey, int iMessageKey, bool bAccept);

    int DeleteEmpireFromTournament (unsigned int iTournamentKey, int iDeleteKey);

    int GetTournamentGameClasses (unsigned int iTournamentKey, unsigned int** ppiGameClassKey, 
        Variant** ppvName, unsigned int* piNumKeys);

    int GetTournamentEmpires (unsigned int iTournamentKey, unsigned int** ppiEmpireKey, unsigned int** ppiTeamKey,
        Variant** ppvName, unsigned int* piNumKeys);

    int GetAvailableTournamentEmpires (unsigned int iTournamentKey, unsigned int** ppiEmpireKey, 
        unsigned int** ppiTeamKey, Variant** ppvName, unsigned int* piNumKeys);

    int GetTournamentTeamEmpires (unsigned int iTournamentKey, unsigned int iTeamKey, 
        int** ppiEmpireKey, Variant** ppvName, unsigned int* piNumKeys);

    int GetTournamentTeams (unsigned int iTournamentKey, unsigned int** ppiTeamKey, 
        Variant** ppvName, unsigned int* piNumKeys);

    int GetTournamentGames (unsigned int iTournamentKey, int** ppiGameClass, int** ppiGameNumber, unsigned int* piNumGames);

    int CreateTournamentTeam (unsigned int iTournamentKey, Variant* pvTeamData, unsigned int* piTeamKey);
    int DeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey);

    int SetEmpireTournamentTeam (unsigned int iTournamentKey, int iEmpireKey, unsigned int iTeamKey);

    int GetTournamentTeamData (unsigned int iTournamentKey, unsigned int iTeamKey, Variant** ppvTournamentTeamData);

    int GetTournamentEmpireData (unsigned int iTournamentKey, unsigned int iEmpireKey, Variant** ppvTournamentEmpireData);

    int GetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, Variant* pvTournamentTeamDesc);
    int SetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, const char* pszTournamentTeamDesc);
    
    int GetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, Variant* pvTournamentTeamUrl);
    int SetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, const char* pszTournamentTeamUrl);

    int GetGameClassTournament (int iGameClass, unsigned int* piTournamentKey);

    int GetTournamentIcon (unsigned int iTournamentKey, unsigned int* piIcon);
    int SetTournamentIcon (unsigned int iTournamentKey, unsigned int iIcon);

    int GetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int* piIcon);
    int SetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int iIcon);

    // Associations
    int GetAssociations (unsigned int iEmpireKey, unsigned int** ppiEmpires, unsigned int* piNumAssoc);
    int CheckAssociation (unsigned int iEmpireKey, unsigned int iSwitch, bool* pbAuth);
    int CreateAssociation (unsigned int iEmpireKey, const char* pszSecondEmpire, const char* pszPassword);
    int DeleteAssociation (unsigned int iEmpireKey, unsigned int iSecondEmpireKey);

    //
    // Interfaces
    //
    IMPLEMENT_TWO_INTERFACES (IDatabaseBackupNotificationSink, IGameEngine);

    // IDatabaseBackupNotificationSink
    void BeginBackup (const char* pszBackupDirectory);
    
    void BeginTemplateBackup (unsigned int iNumTemplates);
    void EndTemplateBackup();

    void BeginTableBackup (unsigned int iNumTables);
    void EndTableBackup();

    void BeginVariableLengthDataBackup();
    void EndVariableLengthDataBackup();

    void BeginMetaDataBackup();
    void EndMetaDataBackup();

    void EndBackup (IDatabaseBackup* pBackup);

    void AbortBackup (int iErrCode);

    // Auxiliary database functions
    bool IsDatabaseBackingUp();
    void GetDatabaseBackupProgress (DatabaseBackupStage* pdbsStage, Seconds* piElapsedTime, unsigned int* piNumber);
};

struct PlanetData {

    char Name [MAX_PLANET_NAME_LENGTH + 1];
    
    int Ag;
    int Min;
    int Fuel;

    int Owner;

    int XCoord;
    int YCoord;

    int NorthPlanetKey;
    int EastPlanetKey;
    int SouthPlanetKey;
    int WestPlanetKey;

    bool LinkNorth;
    bool LinkEast;
    bool LinkSouth;
    bool LinkWest;

    bool HomeWorld;
};


class IGenerateMaps {
public:

    virtual int AddEmpireToMap (

        GameEngine* pGameEngine,

        int iGameClass,
        int iGameNumber,
        int iEmpireKey,

        PlanetData* pExistingPlanetData,
        unsigned int iNumExistingPlanets,

        PlanetData* pNewPlanetData,
        unsigned int iNumNewPlanets
        ) = 0;
};


#endif // !defined(AFX_GAMEENGINE_H__58727607_5549_11D1_9D09_0060083E8062__INCLUDED_)