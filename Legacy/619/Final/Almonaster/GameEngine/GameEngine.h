// GameEngine.h: Definition of the GameEngine class
//
//////////////////////////////////////////////////////////////////////
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

#if !defined(AFX_GAMEENGINE_H__58727607_5549_11D1_9D09_0060083E8062__INCLUDED_)
#define AFX_GAMEENGINE_H__58727607_5549_11D1_9D09_0060083E8062__INCLUDED_

#include "Alajar.h"

#include <stdio.h>
#include <limits.h>

#define ALMONASTER_BUILD

#include "GameEngineSchema.h"
#include "GameEngineUI.h"
#include "GameEngineGameObject.h"
#include "IGameEngine.h"

#include "Osal/Thread.h"
#include "Osal/Event.h"
#include "Osal/FifoQueue.h"
#include "Osal/Library.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/HashTable.h"

#undef ALMONASTER_BUILD

class GameEngine;
struct LongRunningQueryMessage;

// Remove annoying warning
#ifdef _WIN32
#pragma warning (disable : 4245)
#endif

//
// Types
//

typedef int (THREAD_CALL *Fxn_QueryCallBack) (LongRunningQueryMessage*);

struct LongRunningQueryMessage {
	GameEngine* GameEngine;
	Fxn_QueryCallBack QueryCall;
	void* Arguments;
};

/////////////////////////////////////////////////////////////////////////////
// GameEngine

class GameEngine : public IDatabaseBackupNotificationSink, public IGameEngine {

	friend class AlmonasterHook;

private:

	IDatabase* m_pGameData;

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
	ReadWriteLock rwGameObjectTableLock;

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
	unsigned int m_iNumTemplates;
	unsigned int m_iMaxNumTemplates;
	unsigned int m_iNumTables;
	unsigned int m_iMaxNumTables;
	bool m_bActiveBackup;
	UTCTime m_tBackupStartTime;

	// Configuration
	SystemConfiguration m_scConfig;

	ReadWriteLock m_rwGameConfigLock;
	ReadWriteLock m_rwMapConfigLock;

	// Database name
	char* m_pszDatabaseName;

	// Synchronization
	ReadWriteLock m_mConfigLock;
	Mutex m_mEmpires;
	Mutex m_mGameClasses;
	Mutex m_mSuperClasses;
	Mutex m_mAlienIcons;
	Mutex m_mThemes;

	// Long running queries
	Event m_eQueryEvent;

	Thread m_tLongRunningQueries;
	ThreadSafeFifoQueue<LongRunningQueryMessage*> m_tsfqQueryQueue;

	static int THREAD_CALL LongRunningQueryProcessor (void* pVoid);
	int LongRunningQueryProcessorLoop();

	int SendLongRunningQueryMessage (Fxn_QueryCallBack pfxFunction, void* pVoid);

	// Setup
	int Setup();
	
	int CreateDefaultSystemTemplates();
	int CreateDefaultSystemTables();
	int SetupDefaultSystemTables();
	int SetupDefaultSystemGameClasses();

	// Games
	int CleanupGame (int iGameClass, int iGameNumber);
	int QuitEmpireFromGameInternal (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire);
	int CheckGameForEndConditions (int iGameClass, int iGameNumber, const char* pszAdminMessage, bool* pbEndGame);
	int PauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast);
	int UnpauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast);

	int DeleteShipFromDeadEmpire (const char* pszEmpireShips, const char* pszGameMap, 
		unsigned int iShipKey, unsigned int iPlanetKey);

	int GetGames (bool bOpen, int** ppiGameClass, int** ppiGameNumber, int* piNumGames);

	int AddToGameTable (int iGameClass, int iGameNumber);
	int RemoveFromGameTable (int iGameClass, int iGameNumber);

	int RuinEmpire (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage);

	GameObject* GetGameObject (int iGameClass, int iGameNumber);

	// Empires
	int DeleteEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey);
	int RemoveEmpire (int iEmpireKey);
	int RemoveEmpireFromGameInternal (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire);

	int UpdateEmpireString (int iEmpireKey, int iColumn, const char* pszString);

	// Planets
	int AddEmpiresToMap (int iGameClass, int iGameNumber, int* piEmpireKey, int iNumEmpires, bool* pbCommit);

	int CreateMapFromMapGeneratorData (int iGameClass, int iGameNumber, int iEmpireKey,
		Variant* pvGameClassData, Variant* pvGameData, Variant** ppvPlanetData, unsigned int iNumNewPlanets,
		bool* pbCommit);

	int CreatePlanetFromMapGeneratorData (IReadTable* pRead, IWriteTable* pWrite, Variant* pvPlanetData,
		int iEmpireKey, int iGameClassOptions, int* piAg, int* piMin, int* piFuel, int* piPop, int* piMaxPop,
		int* piHomeWorldKey, int* piNumPlanets, int* piMinX, int* piMaxX, int* piMinY, int* piMaxY, 
		int* piNewPlanetKey);

	int InsertPlanetIntoGameEmpireData (int iPlanetKey, int iGameClassOptions, IReadTable* pGameMap, 
		IWriteTable* pGameEmpireMap, IReadTable* pGameEmpireMapRead);

	void AdvanceCoordinates (int iX, int iY, int* piX, int* piY, int cpDirection);

#ifdef _DEBUG

	int VerifyMap (int iGameClass, int iGameNumber);
	int DfsTraversePlanets (IReadTable* pGameMap, unsigned int iPlanetKey, int* piLink, bool* pbVisited, 
		unsigned int iNumPlanets);

#endif

	// Ships
	int DeleteShip (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, bool bSafe);

	int ChangeShipTypeOrMaxBR (const char* pszShips, const char* pszEmpireData, int iEmpireKey, int iShipKey, 
		int iOldShipType, int iNewShipType, float fBRChange);

	int ChangeShipCloakingState (int iShipKey, int iPlanetKey, bool bCloaked, 
		const char* strEmpireShips, const char* strEmpireMap, const char* strGameMap);

	// Score
	int UpdateScoresOnNuke (int iNukerKey, int iNukedKey, const char* pszNukerName, 
		const char* pszNukedName, int iGameClass, int iGameNumber, const char* pszGameClassName,
		bool bSurrender = false);

	int UpdateScoresOnSurrender (int iWinnerKey, int iLoserKey, const char* pszWinnerName, 
		const char* pszLoserName, int iGameClass, int iGameNumber, const char* pszGameClassName);

	int UpdateScoresOn30StyleSurrender (int iLoserKey, const char* pszLoserName, int iGameClass, 
		int iGameNumber, const char* pszGameClassName);

	int UpdateScoresOn30StyleSurrenderColonization (int iWinnerKey, int iPlanetKey, const char* pszWinnerName,
		int iGameClass, int iGameNumber, const char* pszGameClassName);

	int UpdateScoresOnNuke (int iGameClass, int iGameNumber, int iNukerKey, int iNukedKey);
	int UpdateScoresOnSurrender (int iGameClass, int iGameNumber, int iWinnerKey, int iLoserKey);
	int UpdateScoresOn30StyleSurrender (int iGameClass, int iGameNumber, int iLoserKey);
	int UpdateScoresOn30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey);
	int UpdateScoresOnGameEnd (int iGameClass, int iGameNumber);
	int UpdateScoresOnWin (int iGameClass, int iGameNumber, int iEmpireKey);
	int UpdateScoresOnDraw (int iGameClass, int iGameNumber, int iEmpireKey);
	int UpdateScoresOnRuin (int iGameClass, int iGameNumber, int iEmpireKey);

	int CalculatePrivilegeLevel (int iEmpireKey);

	int AddNukeToHistory (NukeList nlNukeList, const char* pszGameClassName, int iGameNumber, 
		int iEmpireKey, const char* pszEmpireName, int iAlienKey,
		int iOtherEmpireKey, const char* pszOtherEmpireName, int iOtherAlienKey);

	bool ValidateEmpireKey (int iLoserKey, unsigned int iHashEmpireName);

	int GetBridierScore (int iEmpireKey, int* piRank, int* piIndex);
	
	int TriggerBridierTimeBombIfNecessaryCallback();
	static int THREAD_CALL TriggerBridierTimeBombIfNecessaryMsg (LongRunningQueryMessage* pMessage);

	int ScanEmpiresOnScoreChanges();

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

	int VerifyTopList (ScoringSystem ssTopList);

	// Updates
	int RunUpdate (int iGameClass, int iGameNumber, const UTCTime& tUpdateTime, bool* pbGameOver);

	inline int UpdateDiplomaticStatus (int iGameClass, int iGameNumber, unsigned int iNumEmpires, unsigned int* piEmpireKey, 
		bool* pbAlive, bool* pbSendFatalMessage, Variant* pvEmpireName, String* pstrUpdateMessage, 
		const char** pstrEmpireDip, 
		const char** pstrEmpireMap, const char* strGameMap, const char** pstrEmpireData, 
		int* piWinner, int* piLoser, unsigned int* piNumSurrenders, const char* pszGameClassName, 
		int iNewUpdateCount, bool* pbAllyOut, bool* pbDrawOut, Variant* pvGoodColor, Variant* pvBadColor);

	inline int UpdatePlanetPopulations (int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, float* pfAgRatio, 
		const char* strGameMap, const char** pstrEmpireData, const char** pstrEmpireMap, String* pstrUpdateMessage, 
		unsigned int* piPlanetKey, int iNumPlanets, int* piTotalMin, int* piTotalFuel, Variant* pvGoodColor, 
		Variant* pvBadColor, float fMaxAgRatio);

	inline int MoveShips (int iGameClass, int iGameNumber, int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, 
		Variant* pvEmpireName, float* pfMaintRatio, const char* strGameMap, const char** pstrEmpireShips, 
		const char** pstrEmpireDip, const char** pstrEmpireMap, const char** pstrEmpireFleets, const char** pstrEmpireData, 
		String* pstrUpdateMessage, const Variant* pvGoodColor, const Variant* pvBadColor, 
		const GameConfiguration& gcConfig);

	inline int MakeShipsFight (int iGameClass, int iGameNumber, const char* strGameMap, int iNumEmpires, 
		unsigned int* piEmpireKey, bool* pbAlive, Variant* pvEmpireName, const char** pstrEmpireShips, 
		const char** pstrEmpireDip, const char** pstrEmpireMap, String* pstrUpdateMessage, 
		const char** pstrEmpireData, float* pfFuelRatio, 
		int iNumPlanets, unsigned int* piPlanetKey, int* piTotalMin, int* piTotalFuel,
		bool bIndependence, const char* strIndependentShips, Variant* pvGoodColor, Variant* pvBadColor,
		const GameConfiguration& gcConfig);

	int MinefieldExplosion (const char* strGameMap, const char** pstrEmpireData, 
		unsigned int iPlanetKey, unsigned int iNumEmpires, unsigned int* piEmpireKey, 
		int* piTotalMin, int* piTotalFuel);

	inline int MakeMinefieldsDetonate (int iGameClass, int iGameNumber, const char* strGameMap, 
		unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, Variant* pvEmpireName, 
		const char** pstrEmpireShips, const char** pstrEmpireMap, String* pstrUpdateMessage, 
		const char** pstrEmpireData, int* piTotalMin, int* piTotalFuel, bool bIndependence, 
		const char* strIndependentShips, Variant* pvGoodColor, Variant* pvBadColor, 
		const GameConfiguration& gcConfig);

	inline int UpdateFleetOrders (int iNumEmpires, bool* pbAlive, const char* strGameMap, 
		const char** pstrEmpireShips, const char** pstrEmpireFleets, const char** pstrEmpireMap);

	inline int UpdateEmpiresEcon (int iGameClass, int iGameNumber, int iNumEmpires, unsigned int* piEmpireKey, 
		bool* pbAlive, int* piTotalMin, 
		int* piTotalFuel, int* piTotalAg, const Seconds& iUpdatePeriod, const UTCTime& tUpdateTime, 
		const char* strGameData, const char** pstrEmpireDip, const char** pstrEmpireData, const char** pstrEmpireShips, 
		int iNewUpdateCount, const char* strGameMap, float fMaxAgRatio,
		const GameConfiguration& gcConfig);

	inline int PerformSpecialActions (int iGameClass, int iGameNumber, int iNumEmpires, unsigned int* piEmpireKey, 
		const Variant* pvGoodColor, const Variant* pvBadColor, const Variant* pvEmpireName, bool* pbAlive, 
		int iNumPlanets, unsigned int* piPlanetKey, unsigned int* piOriginalPlanetOwner, 
		unsigned int* piOriginalNumObliterations, const char** pstrEmpireShips, 
		const char** pstrEmpireData, const char** pstrEmpireMap, String* pstrUpdateMessage, 
		const char* strGameMap, const char* strGameData, int* piTotalAg, int* piTotalMin, int* piTotalFuel,
		const char** pstrEmpireDip, int* piObliterator, int* piObliterated, unsigned int* piNumObliterations, 
		const char* pszGameClassName, int iNewUpdateCount, int iGameClassOptions, unsigned int** ppiShipNukeKey, 
		unsigned int** ppiEmpireNukeKey, unsigned int* piNukedPlanetKey, unsigned int* piNumNukingShips, 
		unsigned int* piNumNukedPlanets, const GameConfiguration& gcConfig);

	inline int ProcessNukes (int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, const char* pszGameClassName,
		int iGameClass, int iGameNumber, int* piTotalAg, int* piTotalMin, int* piTotalFuel,
		int iNumNukedPlanets, unsigned int* piNumNukingShips, unsigned int* piNukedPlanetKey, 
		unsigned int** ppiEmpireNukeKey, unsigned int** ppiShipNukeKey, int* piObliterator, int* piObliterated, 
		unsigned int* piNumObliterations,
		Variant* pvEmpireName, const char** pstrEmpireDip, const char** pstrEmpireShips, const char** pstrEmpireMap, 
		String* pstrUpdateMessage, const char** pstrEmpireData, const char* strGameMap, 
		int iNewUpdateCount, const GameConfiguration& gcConfig);

	inline int AddShipSightings (unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive,
		String* pstrUpdateMessage, const Variant* pvEmpireName, bool bIndependence, unsigned int iNumPlanets, 
		unsigned int* piPlanetKey, const char* strGameMap, const char** pstrEmpireMap, 
		const char** pstrEmpireShips, const char* strIndependentShips);

	int SharePlanetsBetweenFriends (int iGameClass, int iGameNumber, 
		unsigned int iEmpireIndex1, unsigned int iEmpireIndex2,
		const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireData, 
		const char* pszGameMap, unsigned int iNumEmpires, unsigned int* piEmpireKey, int iDipLevel);

	inline int SharePlanetBetweenFriends (int iGameClass, int iGameNumber, unsigned int iPlanetKey, 
		unsigned int iEmpireIndex,
		const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireData, 
		const char* pszGameMap, 
		unsigned int iNumEmpires, unsigned int* piEmpireKey, int iDipLevel, 
		Variant* pvAcquaintanceKey, unsigned int* piProxyKey, unsigned int iNumAcquaintances);

	inline int ProcessGates (int iGameClass, int iGameNumber, unsigned int iNumEmpires, unsigned int* piEmpireKey, 
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

	inline int CreateNewPlanetFromBuilder (const GameConfiguration& gcConfig,
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

	int VerifyGameTables (int iGameClass, int iGameNumber);

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

	int ProcessSubjectiveViews (unsigned int iNumEmpires, unsigned int* piEmpireKey, bool* pbAlive, 
		const char* strGameMap, const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireShips);

	int ProcessEmpireSubjectiveView (unsigned int iEmpireKey, IReadTable* pGameMap, 
		unsigned int iFullNumEmpires, unsigned int* piFullEmpireKey, const char** pstrEmpireDip, 
		const char** pstrEmpireShips, IReadTable* pEmpireMap, IWriteTable* pWriteEmpireDip, IReadTable* pEmpireDip);

	// Messages
	int SendFatalUpdateMessage (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszGameClassName,
		const String& strUpdateMessage);

	// Empires
	int WriteNextStatistics (int iGameClass, int iGameNumber, int iEmpireKey, int iTotalAg, int iBonusAg, 
		float fMaxAgRatio);

public:

	// Constructor/destructor
	GameEngine (
		
		const char* pszDatabaseName, 
		const char* pszHookLibrary,

		IAlmonasterUIEventSink* pUIEventSink, 

		IReport* pReport, 
		IPageSourceControl* pPageSourceControl,

		SystemConfiguration* pscConfig
	
		);
	
	~GameEngine();

	void FreeData (void** ppData);
	void FreeData (Variant* pvData);
	void FreeData (Variant** ppvData);
	void FreeData (int* piData);
	void FreeData (float* ppfData);
	void FreeData (char** ppszData);
	void FreeData (UTCTime* ptData);
	void FreeKeys (unsigned int* piKeys);
	void FreeKeys (int* piKeys);

	int Initialize();

	int FlushDatabase (int iEmpireKey);
	int BackupDatabase (int iEmpireKey);
	int PurgeDatabase (int iEmpireKey, int iCriteria);

	int RestoreDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion);
	int DeleteDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion);

	IDatabase* GetDatabase();
	IScoringSystem* GetScoringSystem (ScoringSystem ssScoringSystem);

	int GetAdministratorEmailAddress (Variant* pvEmail);

	const char* GetSystemVersion();

	int GetNewSessionId (int64* pi64SessionId);

	int GetGameConfiguration (GameConfiguration* pgcConfig);
	int GetMapConfiguration (MapConfiguration* pmcConfig);

	int SetGameConfiguration (const GameConfiguration& gcConfig);
	int SetMapConfiguration (const MapConfiguration& mcConfig);

	bool ReportLoginEvents();

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
	
	void GetBuilderNewPlanetResources (float fBR, float fMinBR, int iAvgAg, int iAvgMin, int iAvgFuel, 
		int* piNewAvgAg, int* piNewAvgMin, int* piNewAvgFuel);
	
	float GetGateBRForRange (float fRangeFactor, int iSrcX, int iSrcY, int iDestX, int iDestY);
	float GetCarrierDESTAbsorption (float fBR);
	bool IsMobileShip (int iShipType);

	float GetLateComerTechIncrease (int iPercentTechIncreaseForLatecomers, int iNumUpdates, float fMaxTechDev);
	int GetMaxPop (int iMin, int iFuel);

	int GetMaxNumDiplomacyPartners (int iGameClass, int iGameNumber, int iDiplomacyLevel, int* piMaxNumPartners);
	int GetMaxNumDiplomacyPartners (int iGameClass, int iDiplomacyLevel, int* piMaxNumPartners);

	unsigned int GetNumFairDiplomaticPartners (unsigned int iMaxNumEmpires);

	bool GameAllowsDiplomacy (int iDiplomacyLevel, int iDip);
	bool IsLegalDiplomacyLevel (int iDiplomacyLevel);
	bool IsLegalPrivilege (int iPrivilege);

	// Locks: don't use these unless you know what you're doing!
	void LockGameClass (int iGameClass, NamedMutex* pnmMutex);
	void UnlockGameClass (const NamedMutex& nmMutex);

	void LockGameClasses();
	void UnlockGameClasses();

	void LockSuperClasses();
	void UnlockSuperClasses();

	void LockEmpireSystemMessages (int iEmpireKey, NamedMutex* pnmMutex);
	void UnlockEmpireSystemMessages (const NamedMutex& nmMutex);

	void LockEmpireBridier (int iEmpireKey, NamedMutex* pnmMutex);
	void UnlockEmpireBridier (const NamedMutex& nmMutex);
	
	void LockGame (int iGameClass, int iGameNumber, NamedMutex* pnmMutex);
	void UnlockGame (const NamedMutex& nmMutex);

	void LockEmpireGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex);
	void UnlockEmpireGameMessages (const NamedMutex& nmMutex);

	void LockEmpireShips (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex);
	void UnlockEmpireShips (const NamedMutex& nmMutex);

	void LockEmpires();
	void UnlockEmpires();

	void LockAlienIcons();
	void UnlockAlienIcons();
	
	void LockThemes();
	void UnlockThemes();

	void LockAutoUpdate (int iGameClass, int iGameNumber, NamedMutex* pnmMutex);
	void UnlockAutoUpdate (const NamedMutex& nmMutex);

	void LockPauseGame (int iGameClass, int iGameNumber, NamedMutex* pnmMutex);
	void UnlockPauseGame (const NamedMutex& nmMutex);

	void LockEmpireUpdated (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex);
	void UnlockEmpireUpdated (const NamedMutex& nmMutex);

	void LockEmpireTechs (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex);
	void UnlockEmpireTechs (const NamedMutex& nmMutex);

	void LockEmpireFleets (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex);
	void UnlockEmpireFleets (const NamedMutex& nmMutex);

	void LockEmpire (int iEmpireKey, NamedMutex* pnmMutex);
	void UnlockEmpire (const NamedMutex& nmMutex);

	int WaitGameReader (int iGameClass, int iGameNumber);
	int SignalGameReader (int iGameClass, int iGameNumber);

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
	int CreateTheme (Variant* pvData, int* piKey);
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
	int GetDefaultBackgroundKey (int* piBackgroundKey);
	int GetDefaultSeparatorKey (int* piSeparatorKey);
	int GetDefaultButtonKey (int* piButtonKey);

	int GetMaxIconSize (int* piSize);
	int SetMaxIconSize (int iSize);

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
	int GetMaxNumEmpires (int iGameClass, int* piMaxNumEmpires);

	int DeleteGameClass (int iGameClass, bool* pbDeleted);
	int UndeleteGameClass (int iGameClassKey);

	int HaltGameClass (int iGameClass);
	int UnhaltGameClass (int iGameClass);

	int CreateGameClass (Variant* pvGameClassData, int* piGameClass);
	int GetSystemGameClassKeys (int** ppiKey, bool** ppbHalted, bool** ppbDeleted, int* piNumKeys);
	int GetStartableSystemGameClassKeys (int** ppiKey, int* piNumKeys);

	int GetGameClassSuperClassKey (int iGameClass, int* piSuperClassKey);
	int SetGameClassSuperClassKey (int iGameClass, int iSuperClassKey);

	int GetGameClassData (int iGameClass, Variant** ppvData);
	int GetGameClassOwner (int iGameClass, int* piOwner);
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

	// Games
	int GetGameUpdateData (int iGameClass, int iGameNumber, int* piSecondsSince, int* piSecondsUntil, 
		int* piNumUpdates, int* piGameState);

	int GetGameCreationTime (int iGameClass, int iGameNumber, UTCTime* ptCreationTime);

	int DeleteGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, int iReason);
	
	int GetNumActiveGames (int* piNumGames);
	int GetNumOpenGames (int* piNumGames);
	int GetNumClosedGames (int* piNumGames);

	int GetActiveGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames);
	int GetOpenGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames);
	int GetClosedGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames);

	int IsGameOpen (int iGameClass, int iGameNumber, bool* pbOpen);
	int HasGameStarted (int iGameClass, int iGameNumber, bool* pbStarted);

	int IsGamePasswordProtected (int iGameClass, int iGameNumber, bool* pbProtected);
	int GetGamePassword (int iGameClass, int iGameNumber, Variant* pvPassword);
	int SetGamePassword (int iGameClass, int iGameNumber, const char* pszNewPassword);

	int CreateGame (int iGameClass, int iEmpireKey, const GameOptions& goGameOptions, int* piGameNumber);
	int EnterGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszPassword, int* piNumUpdates);

	int SetEnterGameIPAddress (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszIPAddress);

	int DoesGameExist (int iGameClass, int iGameNumber, bool* pbExists);
	int GetNumUpdates (int iGameClass, int iGameNumber, int* piNumUpdates);
	
	int GetNumUpdatesBeforeGameCloses (int iGameClass, int iGameNumber, int* piNumUpdates);
	int GetGameOptions (int iGameClass, int iGameNumber, int* piOptions);
	int GetFirstUpdateDelay (int iGameClass, int iGameNumber, Seconds* psDelay);
	
	int GetNumEmpiresInGame (int iGameClass, int iGameNumber, int* piEmpires);
	int GetNumEmpiresNeededForGame (int iGameClass, int* piNumEmpiresNeeded);
	int IsEmpireInGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbInGame);
	int GetMaxNumActiveEmpiresInGame (int iGameClass, int iGameNumber, int* piEmpires);
	int GetNumUpdatedEmpires (int iGameClass, int iGameNumber, int* piUpdatedEmpires);
	int GetEmpiresInGame (int iGameClass, int iGameNumber, Variant** ppiEmpireKey, int* piNumEmpires);

	int IsGamePaused (int iGameClass, int iGameNumber, bool* pbPaused);
	int IsGameAdminPaused (int iGameClass, int iGameNumber, bool* pbAdminPaused);
	int IsSpectatorGame (int iGameClass, int iGameNumber, bool* pbSpectatorGame);

	void GetGameClassGameNumber (const char* pszGameData, int* piGameClass, int* piGameNumber);
	void GetGameClassGameNumber (int iGameClass, int iGameNumber, char* pszGameData);

	int PauseAllGames();
	int UnpauseAllGames();

	int LogEmpireIntoGame (int iGameClass, int iGameNumber, int iEmpireKey);

	int RuinGame (int iGameClass, int iGameNumber);

	int GetResignedEmpiresInGame (int iGameClass, int iGameNumber, int** ppiEmpireKey, int* piNumResigned);
	int UnresignEmpire (int iGameClass, int iGameNumber, int iEmpireKey, int iAdminKey);

	int GetGameEntryRestrictions (int iGameClass, int iGameNumber, int* piOptions, 
		Variant pvRestrictionMin [NUM_ENTRY_SCORE_RESTRICTIONS], 
		Variant pvRestrictionMax [NUM_ENTRY_SCORE_RESTRICTIONS]);

	int GameAccessCheck (int iGameClass, int iGameNumber, int iEmpireKey, 
		const GameOptions* pgoGameOptions, GameAction gaAction, bool* pbAccess);

	int GetBridierRankPotentialGainLoss (int iGameClass, int iGameNumber, int iEmpireKey, int* piGain, int* piLoss);

	// GameEmpireData
	int QuitEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire = NO_KEY);
	int ResignEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey);
	int SurrenderEmpireFromGame30Style (int iGameClass, int iGameNumber, int iEmpireKey);

	int HasEmpireResignedFromGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbResigned);

	// Empires
	int CreateEmpire (const char* pszEmpireName, const char* pszPassword, int iPrivilege, int iParentKey, int* piEmpireKey);

	int GetEmpireName (int iEmpireKey, Variant* pvName);
	int SetEmpireName (int iEmpireKey, const char* pszName);
	
	int SetEmpirePassword (int iEmpireKey, const char* pszPassword);
	int ChangeEmpirePassword (int iEmpireKey, const char* pszPassword);

	int SetEmpireRealName (int iEmpireKey, const char* pszRealName);
	int SetEmpireEmail (int iEmpireKey, const char* pszEmail);
	int SetEmpireWebPage (int iEmpireKey, const char* pszWebPage);
	int SetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int iMaxNumSavedMessages);

	int UpdateEmpireQuote (int iEmpireKey, const char* pszQuote);
	int UpdateEmpireVictorySneer (int iEmpireKey, const char* pszSneer);
	int SendVictorySneer (int iWinnerKey, const char* pszWinnerName, int iLoserKey);

	int GetEmpireAlternativeGraphicsPath (int iEmpireKey, Variant* pvPath);
	int SetEmpireAlternativeGraphicsPath (int iEmpireKey, const char* pszPath);

	int SetEmpireBackgroundKey (int iEmpireKey, int iBackgroundKey);
	int SetEmpireLivePlanetKey (int iEmpireKey, int iLivePlanetKey);
	int SetEmpireDeadPlanetKey (int iEmpireKey, int iDeadPlanetKey);
	int SetEmpireButtonKey (int iEmpireKey, int iButtonKey);
	int SetEmpireSeparatorKey (int iEmpireKey, int iSeparatorKey);
	int SetEmpireHorzKey (int iEmpireKey, int iHorzKey);
	int SetEmpireVertKey (int iEmpireKey, int iVertKey);

	int SetEmpireColorKey (int iEmpireKey, int iColorKey);

	int SetEmpireCustomTableColor (int iEmpireKey, const char* pszColor);
	int SetEmpireCustomTextColor (int iEmpireKey, const char* pszColor);
	int SetEmpireCustomGoodColor (int iEmpireKey, const char* pszColor);
	int SetEmpireCustomBadColor (int iEmpireKey, const char* pszColor);
	int SetEmpireCustomPrivateMessageColor (int iEmpireKey, const char* pszColor);
	int SetEmpireCustomBroadcastMessageColor (int iEmpireKey, const char* pszColor);
	
	int GetEmpireCustomTableColor (int iEmpireKey, Variant* pvColor);
	int GetEmpireCustomTextColor (int iEmpireKey, Variant* pvColor);
	int GetEmpireCustomGoodColor (int iEmpireKey, Variant* pvColor);
	int GetEmpireCustomBadColor (int iEmpireKey, Variant* pvColor);
	int GetEmpireCustomPrivateMessageColor (int iEmpireKey, Variant* pvColor);
	int GetEmpireCustomBroadcastMessageColor (int iEmpireKey, Variant* pvColor);

	int GetEmpireLivePlanetKey (int iEmpireKey, int* piLivePlanetKey);
	int GetEmpireDeadPlanetKey (int iEmpireKey, int* piDeadPlanetKey);

	int SetEmpireThemeKey (int iEmpireKey, int iThemeKey);

	int GetEmpireRealName (int iEmpireKey, Variant* pvRealName);
	int GetEmpireEmail (int iEmpireKey, Variant* pvEmail);
	int GetEmpireWebPage (int iEmpireKey, Variant* pvWebPage);
	int GetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int* piMaxNumSavedMessages);

	int DeleteEmpire (int iEmpireKey);
	int ObliterateEmpire (int iEmpireKey, int iKillerEmpire);

	int RemoveEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire);

	int DoesEmpireExist (const char* pszName, bool* pbExists, int* piEmpireKey, Variant* pvEmpireName);
	int DoesEmpireExist (int iEmpireKey, bool* pbExists, Variant* pvEmpireName);
	int DoesEmpireExist (int iEmpireKey, bool* pbExists);

	int DoesEmpireKeyMatchName (int iEmpireKey, const char* pszEmpireName, bool* pbMatch);
	
	int IsPasswordCorrect (int iEmpireKey, const char* pszPassword);
	int LoginEmpire (int iEmpireKey, const char* pszBrowser);
	int GetNumEmpiresOnServer (int* piNumEmpires);
	int GetDefaultEmpireShipNames (int iEmpireKey, const char*** pppszShipName);

	int UndeleteEmpire (int iEmpireKey);
	int BlankEmpireStatistics (int iEmpireKey);
	int GetEmpireGameClassKeys (int iEmpireKey, int** ppiGameClassKey, int* piNumKeys);
	int GetEmpireData (int iEmpireKey, Variant** ppvEmpData, int* piNumActiveGames);
	int GetEmpireActiveGames (int iEmpireKey, int** ppiGameClass, int** ppiGameNumber, int* piNumGames);
	
	int GetEmpireThemeKey (int iEmpireKey, int* piThemeKey);
	int GetEmpireBackgroundKey (int iEmpireKey, int* piBackgroundKey);
	int GetEmpireSeparatorKey (int iEmpireKey, int* piSeparatorKey);
	int GetEmpireButtonKey (int iEmpireKey, int* piButtonKey);
	int GetEmpireHorzKey (int iEmpireKey, int* piHorzKey);
	int GetEmpireVertKey (int iEmpireKey, int* piVertKey);

	int GetEmpireColorKey (int iEmpireKey, int* piColorKey);

	int GetEmpirePrivilege (int iEmpireKey, int* piPrivilege);
	int SetEmpirePrivilege (int iEmpireKey, int iPrivilege);

	int GetEmpireAlmonasterScore (int iEmpireKey, float* pfAlmonasterScore);
	int SetEmpireAlmonasterScore (int iEmpireKey, float fAlmonasterScore);

	int GetEmpirePassword (int iEmpireKey, Variant* pvPassword);

	int GetEmpireDataColumn (int iEmpireKey, unsigned int iColumn, Variant* pvData);

	int GetNumLogins (int iEmpireKey, int* piNumLogins);

	int GetEmpireMaxNumShipsBuiltAtOnce (int iEmpireKey, int* piMaxNumShipsBuiltAtOnce);
	int SetEmpireMaxNumShipsBuiltAtOnce (int iEmpireKey, int iMaxNumShipsBuiltAtOnce);

	int GetEmpireIPAddress (int iEmpireKey, Variant* pvIPAddress);
	int SetEmpireIPAddress (int iEmpireKey, const char* pszIPAddress);

	int GetEmpireSessionId (int iEmpireKey, int64* pi64SessionId);
	int SetEmpireSessionId (int iEmpireKey, int64 i64SessionId);

	int GetEmpireDefaultBuilderPlanet (int iEmpireKey, int* piDefaultBuildPlanet);
	int SetEmpireDefaultBuilderPlanet (int iEmpireKey, int iDefaultBuildPlanet);

	int ResetEmpireSessionId (int iEmpireKey);
	int EndResetEmpireSessionId (int iEmpireKey);

	int GetEmpireOptions (int iEmpireKey, int* piOptions);

	int GetEmpireOption (int iEmpireKey, unsigned int iFlag, bool* pbOption);
	int SetEmpireOption (int iEmpireKey, unsigned int iFlag, bool bSet);

	// Game Empire Data
	int GetEmpireOptions (int iGameClass, int iGameNumber, int iEmpireKey, int* piOptions);

	int GetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, bool* pbOption);
	int SetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, bool bOption);

	int GetEmpirePartialMapData (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbPartialMaps, 
		unsigned int* piCenterKey, unsigned int* piXRadius, unsigned int* piRadiusY);

	int SetEmpirePartialMapCenter (int iGameClass, int iGameNumber, int iEmpireKey, int iCenterKey);
	int SetEmpirePartialMapXRadius (int iGameClass, int iGameNumber, int iEmpireKey, int iXRadius);
	int SetEmpirePartialMapYRadius (int iGameClass, int iGameNumber, int iEmpireKey, int iYRadius);

	int GetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
		int* piDefaultBuildPlanet, int* piResolvedPlanetKey);
	int SetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iDefaultBuildPlanet);

	int GetEmpireBR (int iGameClass, int iGameNumber, int iEmpireKey, int* piBR);
	int GetEmpireMaintenanceRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfMaintenanceRatio);

	int GetEmpireDefaultMessageTarget (int iEmpireKey, int* piMessageTarget);
	int SetEmpireDefaultMessageTarget (int iEmpireKey, int iMessageTarget);

	// System Messages
	int SendSystemMessage (int iEmpireKey, const char* pszMessage, int iSource, bool bBroadcast = false);
	int GetNumSystemMessages (int iEmpireKey, int* piNumber);
	int GetNumUnreadSystemMessages (int iEmpireKey, int* piNumber);
	int SendMessageToAll (int iEmpireKey, const char* pszMessage);
	int GetSavedSystemMessages (int iEmpireKey, int** ppiMessageKey, Variant*** pppvMessage, int* piNumMessages);

	int GetUnreadSystemMessages (int iEmpireKey, Variant*** pppvMessage, int* piNumMessages);
	
	int DeleteSystemMessage (int iEmpireKey, int iKey);

	int GetSystemLimitOnSavedSystemMessages (int* piSystemMaxNumSavedMessages);
	int GetSystemMessageSender (int iEmpireKey, int iMessageKey, Variant* pvSender);

	// Game Messages
	int GetNumGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumber);
	int GetNumUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumber);
	int GetSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiMessageKey,
		Variant*** ppvData, int* piNumMessages);
	
	int SendGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, int iSource, 
		bool bBroadcast = false, const UTCTime& tSendTime = NULL_TIME);
	
	int GetUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, Variant*** pppvMessage, 
		int* piNumMessages);
	
	int DeleteGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, int iMessageKey);
	int BroadcastGameMessage (int iGameClass, int iGameNumber, const char* pszMessage, int iSourceKey, bool bAdmin);

	int GetGameMessageSender (int iGameClass, int iGameNumber, int iEmpireKey, int iMessageKey, Variant* pvSender);

	// Planets
	int GetPlanetCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, int* piPlanetX, int* piPlanetY);
	int GetPlanetKeyFromCoordinates (int iGameClass, int iGameNumber, int iX, int iY, int* piPlanetKey);
	int GetEmpirePlanetIcons (int iEmpireKey, int* piLivePlanetKey, int* piLiveDeadPlanetKey);
	int GetNeighbourPlanetKey (int iGameClass, int iGameNumber, int iPlanetKey, int iDirection, 
		int* piPlanetKey);
	int HasEmpireExploredPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
		bool* pbExplored);

	void GetCoordinates (const char* pszCoord, int* piX, int* piY);
	void GetCoordinates (int iX, int iY, char* pszCoord);

	int GetLowestDiplomacyLevelForShipsOnPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
		int iPlanetKey, bool bVisibleBuilds, Variant* pvEmpireKey, unsigned int& iNumEmpires,
		int* piNumForeignShipsOnPlanet, int* piDiplomacyLevel, Variant** ppvEmpireKey);

	// Score
	int GetNumEmpiresInNukeHistory (int iEmpireKey, int* piNumNukes, int* piNumNuked);
	int GetNukeHistory (int iEmpireKey, int* piNumNuked, Variant*** pppvNukedData, int* piNumNukers, 
		Variant*** pppvNukerData);

	int GetSystemNukeHistory (int* piNumNukes, Variant*** pppvNukedData);

	int TriggerBridierTimeBombIfNecessary();

	int GetBridierTimeBombScanFrequency (Seconds* piFrequency);
	int SetBridierTimeBombScanFrequency (Seconds iFrequency);

	// System Config	
	int GetServerName (Variant* pvServerName);
	int SetServerName (const char* pszServerName);
	
	int GetDefaultUIKeys (int* piBackground, int* piLivePlanet, int* piDeadPlanet, int* piButtons,
		int* piSeparator, int* piHorz, int* piVert, int* piColor);

	int SetAdeptScore (float fScore);
	int GetAdeptScore (float* pfScore);

	int SetApprenticeScore (float fScore);
	int GetApprenticeScore (float* pfScore);

	int SetMaxNumSystemMessages (int iMaxNumSystemMessages);
	int SetMaxNumGameMessages (int iMaxNumGameMessages);
	int SetDefaultMaxNumSystemMessages (int iDefaultMaxNumSystemMessages);
	int SetDefaultMaxNumGameMessages (int iDefaultMaxNumGameMessages);
	
	int GetMaxNumPersonalGameClasses (int* piMaxNumPersonalGameClasses);
	int SetMaxNumPersonalGameClasses (int iMaxNumPersonalGameClasses);
	
	int GetDefaultShipName (int iTech, Variant* pvShipName);
	int SetDefaultShipName (int iShipKey, const char* pszShipName);

	int SetDefaultBackgroundKey (int iKey);
	int SetDefaultLivePlanetKey (int iKey);
	int SetDefaultDeadPlanetKey (int iKey);
	int SetDefaultButtonKey (int iKey);
	int SetDefaultSeparatorKey (int iKey);
	int SetDefaultHorzKey (int iKey);
	int SetDefaultVertKey (int iKey);
	int SetDefaultColorKey (int iKey);

	int SetSystemOption (int iOption, bool bFlag);

	int GetLoginsDisabledReason (Variant* pvReason);
	int GetEmpireCreationDisabledReason (Variant* pvReason);
	int GetGameCreationDisabledReason (Variant* pvReason);
	int GetAccessDisabledReason (Variant* pvReason);

	int SetLoginsDisabledReason (const char* pszReason);
	int SetEmpireCreationDisabledReason (const char* pszReason);
	int SetGameCreationDisabledReason (const char* pszReason);
	int SetAccessDisabledReason (const char* pszReason);

	int GetMaxResourcesPerPlanet (int* piMaxResourcesPerPlanet);
	int SetMaxResourcesPerPlanet (int iMaxResourcesPerPlanet);

	int GetMaxResourcesPerPlanetPersonal (int* piMaxResourcesPerPlanet);
	int SetMaxResourcesPerPlanetPersonal (int iMaxResourcesPerPlanet);
	
	int GetMaxInitialTechLevel (float* pfMaxInitialTechLevel);
	int SetMaxInitialTechLevel (float fMaxInitialTechLevel);

	int GetMaxInitialTechLevelPersonal (float* pfMaxInitialTechLevel);
	int SetMaxInitialTechLevelPersonal (float fMaxInitialTechLevel);

	int GetMaxTechDev (float* pfMaxInitialTechLevel);
	int SetMaxTechDev (float fMaxInitialTechLevel);

	int GetMaxTechDevPersonal (float* pfMaxTechDev);
	int SetMaxTechDevPersonal (float fMaxTechDev);

	int GetNumUpdatesDownBeforeGameIsKilled (int* piNumUpdatesDown);
	int SetNumUpdatesDownBeforeGameIsKilled (int iNumUpdatesDown);

	int GetSecondsForLongtermStatus (Seconds* psSeconds);
	int SetSecondsForLongtermStatus (Seconds sSeconds);

	int GetAfterWeekendDelay (Seconds* psDelay);
	int SetAfterWeekendDelay (Seconds sDelay);

	int GetNumNukesListedInNukeHistories (int* piNumNukes);
	int SetNumNukesListedInNukeHistories (int iNumNukes);

	int GetMaxNumUpdatesBeforeClose (int* piMaxNumUpdates);
	int SetMaxNumUpdatesBeforeClose (int iMaxNumUpdates);

	int GetDefaultNumUpdatesBeforeClose (int* piMaxNumUpdates);
	int SetDefaultNumUpdatesBeforeClose (int iMaxNumUpdates);

	int GetSystemOptions (int* piOptions);
	int GetDefaultGameOptions (int iGameClass, GameOptions* pgoOptions);

	// Aliens
	int GetNumAliens (int* piNumAliens);
	int GetAlienKeys (Variant*** pppvData, int* piNumAliens);
	int CreateAlienIcon (int iAlienKey, const char* pszAuthorName);
	int DeleteAlienIcon (int iAlienKey);
	int GetDefaultAlien (int* piDefaultAlien);
	int SetDefaultAlien (int iDefaultAlien);
	int GetEmpireAlienKey (int iEmpireKey, int* piAlienKey);
	int SetEmpireAlienKey (int iEmpireKey, int iAlienKey);
	int GetAlienAuthorName (int iAlienKey, Variant* pvAuthorName);

	// Top Lists
	int GetTopList (ScoringSystem ssListType, Variant*** pppvData, unsigned int* piNumEmpires);

	// Search
	int PerformMultipleSearch (int iStartKey, int iSkipHits, int iMaxNumHits, int iNumCols, unsigned int* piColumn, 
		Variant* pvData, Variant* pvData2, int** ppiKey, int* piNumHits, int* piStopKey);

	// Updates
	int CheckGameForUpdates (int iGameClass, int iGameNumber, bool* pbUpdate);
	int CheckAllGamesForUpdates();

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
	int SetEmpireAutoUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool bAutoUpdate);

	int GetEmpireMaxNumSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumMessages);
	int SetEmpireMaxNumSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iNumMessages);

	int GetSystemLimitOnSavedGameMessages (int* piMaxNumSavedGamesMessages);

	int GetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, bool* pbIgnore);
	int SetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, bool bIgnore);

	int GetEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, Variant* pvNotepad);
	int SetEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszNotepad);

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
		int* piWeOffer, int* piTheyOffer, int* piCurrent);
	int GetDiplomaticOptions (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
		int piDipOptKey[], int* piSelected, int* piNumOptions);
	int UpdateDiplomaticOffer (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, int iDipOffer);

	int RequestPause (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState);
	int RequestNoPause (int iGameClass, int iGameNumber, int iEmpireKey, int* piGameState);
	int AdminPauseGame (int iGameClass, int iGameNumber, bool bBroadcast);
	int AdminUnpauseGame (int iGameClass, int iGameNumber, bool bBroadcast);

	int IsEmpireRequestingPause (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbPause);
	int GetNumEmpiresRequestingPause (int iGameClass, int iGameNumber, int* piNumEmpires);

	int GetNumEmpiresAtDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey,
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
		float* pfHypAgRatio, float* pfNextTechDev);

	// Map
	int GetMapLimits (int iGameClass, int iGameNumber, int iEmpireKey, int* piMinX, int* piMaxX, int* piMinY, int* piMaxY);
	int GetMapLimits (int iGameClass, int iGameNumber, int* piMinX, int* piMaxX, int* piMinY, int* piMaxY);

	int GetVisitedPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvPlanetKey, 
		int** ppiEmpireMapKey, int* piNumKeys);
	int GetNumVisitedPlanets (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumVisitedPlanets);
	int HasEmpireVisitedPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, bool* pbVisited);

	int GetPlanetShipOwnerData (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
		int iPlanetProxyKey, int iTotalNumShips, bool bVisibleBuilds, bool bIndependence, 
		int** ppiShipOwnerData);

	int GetPlanetName (int iGameClass, int iGameNumber, int iPlanetKey, Variant* pvPlanetName);
	int GetPlanetNameWithSecurity (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey,  
		Variant* pvPlanetName);

	int GetPlanetNameWithCoordinates (const char* pszGameMap, unsigned int iPlanetKey, String* pstrName);
	int GetPlanetNameWithCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, String* pstrName);

	int RenamePlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, const char* pszNewName);
	int SetPlanetMaxPop (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, int iNewMaxPop);

	int GetGameAveragePlanetResources (int iGameClass, int iGameNumber, int* piAg, int* piMin, int* piFuel);
	int GetEmpireAgRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfAgRatio);

	int GetVisitedSurroundingPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int iCenterKey, 
		Variant* pvPlanetKey, int* piProxyKey, int* piNumPlanets, int* piCenterX, int* piCenterY, 
		int* piMinX, int* piMaxX, int* piMinY, int* piMaxY);

	// Build
	int GetShipRatios (int iGameClass, int iGameNumber, int iEmpireKey, float* pfMaintRatio, 
		float* pfFuelRatio, float* pfTechLevel, float* pfTechDev, int* piBR);

	int GetNextShipRatios (int iGameClass, int iGameNumber, int iEmpireKey, float* pfNextMaintRatio, 
		float* pfNextFuelRatio, float* pfNextTechLevel, float* pfNextTechDev, int* piNextBR);

	int GetBuilderPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiBuilderKey, 
		int* piNumBuilders);

	int BuildNewShips (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey, int iNumShips, 
		const char* pszShipName, float fBR, int iPlanetKey, int iFleetKey);

	int GetNumBuilds (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumBuilds);
	int CancelAllBuilds (int iGameClass, int iGameNumber, int iEmpireKey);

	// Ships
	int GetShipOrders (int iGameClass, int iGameNumber, int iEmpireKey, const GameConfiguration& gcConfig,
		int iShipKey, int iShipType, float fBR, float fMaintRatio, int iPlanetKey, int iLocationX, 
		int iLocationY, int** ppiOrderKey, String** ppstrOrderText, int* piNumOrders, int* piSelected);

	int UpdateShipName (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, const char* pszNewName);

	int UpdateShipOrders (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, int iNewShipOrder);

	int GetNumShips (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumShips);
	int GetNumFleets (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumFleets);

	// Fleets
	int GetEmpireFleetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiFleetKeys, 
		int* piNumFleets);

	int GetFleetLocation (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int* piPlanetKey);
	int GetFleetName (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, Variant* pvFleetName);
	int GetNewFleetLocations (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiLocationKey, 
		int* piNumLocations);

	int CreateNewFleet (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszFleetName, 
		int iPlanetKey);

	int GetFleetOrders (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int** ppiOrderKey, 
		String** ppstrOrderText, int* piSelected, int* piNumOrders);

	int UpdateFleetName (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
		const char* pszNewName);

	int UpdateFleetOrders (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int iOrderKey);

	int GetNumShipsInFleet (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int* piNumShips);

	IMPLEMENT_TWO_INTERFACES (IDatabaseBackupNotificationSink, IGameEngine);

	// IDatabaseBackupNotificationSink
	void BeginBackup (const char* pszBackupDirectory);
	
	void BeginTemplateBackup (unsigned int iNumTemplates);
	void BackupTemplate (const char* pszTemplateName);
	void EndTemplateBackup();

	void BeginTableBackup (unsigned int iNumTables);
	void BackupTable (const char* pszTableName);
	void EndTableBackup();

	void EndBackup (IDatabaseBackup* pBackup);

	void AbortBackup (int iErrCode);

	// Auxiliary database functions
	bool IsDatabaseBackingUp();
	void GetDatabaseBackupProgress (unsigned int* piNumTemplates, unsigned int* piMaxNumTemplates, 
		unsigned int* piNumTables, unsigned int* piMaxNumTables, Seconds* piElapsedTime);
};


struct PlanetData {

	char Name [MAX_PLANET_NAME_LENGTH];
	
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