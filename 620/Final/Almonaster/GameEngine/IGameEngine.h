// IGameEngine.h: Definition of various interfaces
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

#if !defined(AFX_IGAMEENGINE_H__58727607_5549_11D1_9D09_0060083E8062__INCLUDED_)
#define AFX_IGAMEENGINE_H__58727607_5549_11D1_9D09_0060083E8062__INCLUDED_

#include "Database.h"

#include "GameEngineGameObject.h"
#include "GameEngineConstants.h"

#ifndef ALMONASTER_EXPORT
#ifdef ALMONASTER_BUILD
#define ALMONASTER_EXPORT EXPORT
#else
#define ALMONASTER_EXPORT IMPORT
#endif
#endif

ALMONASTER_EXPORT extern const Uuid IID_IGameEngine;
ALMONASTER_EXPORT extern const Uuid IID_IAlmonasterHook;
ALMONASTER_EXPORT extern const Uuid IID_IAlmonasterUIEventSink;
ALMONASTER_EXPORT extern const Uuid IID_IMapGenerator;
ALMONASTER_EXPORT extern const Uuid IID_IScoringSystem;

// Configuration structures
struct GameConfiguration {

    int iShipBehavior;

    int iColonySimpleBuildFactor;
    float fColonyMultipliedBuildFactor;
    float fColonyMultipliedDepositFactor;
    float fColonyExponentialDepositFactor;

    float fEngineerLinkCost;
    float fStargateGateCost;
    float fTerraformerPlowFactor;
    float fTroopshipInvasionFactor;
    float fTroopshipFailureFactor;
    float fTroopshipSuccessFactor;
    float fDoomsdayAnnihilationFactor;

    float fCarrierCost;
    float fBuilderMinBR;
    float fBuilderMultiplier;
    float fMorpherCost;
    float fJumpgateGateCost;
    float fJumpgateRangeFactor;
    float fStargateRangeFactor;

    int iPercentFirstTradeIncrease;
    int iPercentNextTradeIncrease;

    int iPercentTechIncreaseForLatecomers;
    int iPercentDamageUsedToDestroy;

    int iNukesForQuarantine;
    int iUpdatesInQuarantine;
};

struct MapConfiguration {

    int iChanceNewLinkForms;
    int iMapDeviation;
    int iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap;
    int iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap;
    int iLargeMapThreshold;
    float fResourceAllocationRandomizationFactor;
};

struct SystemConfiguration {

    bool bRebuildTopListsOnStartup;

    bool bReport;
    bool bCheckDatabase;
    bool bDatabaseWriteThrough;

    bool bAutoBackup;
    bool bBackupOnStartup;

    Seconds iSecondsBetweenBackups;
    Seconds iBackupLifeTimeInSeconds;
};

struct GameSecurityEntry {
    int iEmpireKey;
    int iOptions;
    const char* pszEmpireName;
};

struct PrearrangedTeam {
    unsigned int iNumEmpires;
    unsigned int* piEmpireKey;
};

struct GameOptions {

    unsigned int iNumEmpires;
    const unsigned int* piEmpireKey;
    unsigned int iTournamentKey;

    int iTeamOptions;
    unsigned int iNumPrearrangedTeams;
    PrearrangedTeam* paPrearrangedTeam;

    const char* pszPassword;
    const char* pszEnterGameMessage;
    int iOptions;
    int iNumUpdatesBeforeGameCloses;
    Seconds sFirstUpdateDelay;
    float fMinAlmonasterScore;
    float fMaxAlmonasterScore;
    float fMinClassicScore;
    float fMaxClassicScore;
    int iMinBridierRank;
    int iMaxBridierRank;
    int iMinBridierIndex;
    int iMaxBridierIndex;
    int iMinBridierRankGain;
    int iMaxBridierRankGain;
    int iMinBridierRankLoss;
    int iMaxBridierRankLoss;
    int iMinWins;
    int iMaxWins;

    unsigned int iNumSecurityEntries;
    GameSecurityEntry* pSecurity;
};

struct TopListQuery {
    ScoringSystem TopList;
    int EmpireKey;
};

enum GameAction {
    VIEW_GAME,
    ENTER_GAME,
};

enum GameResult {
    GAME_RESULT_NONE,
    GAME_RESULT_RUIN,
    GAME_RESULT_WIN,
    GAME_RESULT_DRAW
};

struct GameUpdateInformation {

    unsigned int iUpdate;
    unsigned int iNumEmpires;
    int* piEmpireKey;
    bool* pbAlive;
    String* pstrUpdateMessage;
};

struct RatioInformation {

    float fAgRatio;
    float fNextAgRatio;

    float fMaintRatio;
    float fNextMaintRatio;

    float fFuelRatio;
    float fNextFuelRatio;

    float fTechLevel;
    float fNextTechLevel;

    float fTechDev;
    float fNextTechDev;

    int iBR;
    int iNextBR;
};

class IAlmonasterUIEventSink : virtual public IObject {
public:

    virtual int OnCreateEmpire (int iEmpireKey) = 0;
    virtual int OnDeleteEmpire (int iEmpireKey) = 0;

    virtual int OnLoginEmpire (int iEmpireKey) = 0;
    
    virtual int OnCreateGame (int iGameClass, int iGameNumber) = 0;
    virtual int OnCleanupGame (int iGameClass, int iGameNumber) = 0;

    virtual int OnDeleteTournament (unsigned int iTournamentKey) = 0;
    virtual int OnDeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) = 0;
};

class IMapGenerator : virtual public IObject {
public:

    virtual int CreatePlanets (
        
        int iGameClass, 
        int iGameNumber, 
        int iEmpireKey, 
        
        Variant** ppvPlanetData, 
        unsigned int iNumPlanets,

        Variant* pvGameClassData,
        Variant* pvGameData,

        Variant** ppvNewPlanetData,
        unsigned int iNumNewPlanets
        ) = 0;
};

class IScoringSystem : virtual public IObject {
public:

    virtual bool HasTopList() = 0;

    virtual int OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked) = 0;
    virtual int OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser) = 0;

    virtual int On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser) = 0;
    virtual int On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey) = 0;
    virtual int OnGameEnd (int iGameClass, int iGameNumber) = 0;

    virtual int OnWin (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) = 0;

    virtual bool IsValidScore (const Variant* pvScore) = 0;
    virtual int CompareScores (const Variant* pvLeft, const Variant* pvRight) = 0;

    virtual int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) = 0;
    virtual int GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires) = 0;
};

class IGameEngine : virtual public IObject {
public:

    virtual int GetGameConfiguration (GameConfiguration* pgcConfig) = 0;
    virtual int GetMapConfiguration (MapConfiguration* pmcConfig) = 0;

    virtual int SetGameConfiguration (const GameConfiguration& gcConfig) = 0;
    virtual int SetMapConfiguration (const MapConfiguration& mcConfig) = 0;

    virtual int GetSystemConfiguration (SystemConfiguration* pscConfig)= 0;

        // Setup
    virtual int Setup() = 0;
    
    virtual int CreateDefaultSystemTemplates() = 0;
    virtual int CreateDefaultSystemTables() = 0;
    virtual int SetupDefaultSystemTables() = 0;
    virtual int SetupDefaultSystemGameClasses() = 0;

    // Games
    virtual int CleanupGame (int iGameClass, int iGameNumber, GameResult grResult, const char* pszWinnerName = NULL) = 0;
    virtual int QuitEmpireFromGameInternal (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire) = 0;
    virtual int CheckGameForEndConditions (int iGameClass, int iGameNumber, const char* pszAdminMessage, bool* pbEndGame) = 0;
    virtual int PauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) = 0;
    virtual int UnpauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast) = 0;

    virtual int DeleteShipFromDeadEmpire (const char* pszEmpireShips, const char* pszGameMap, 
        unsigned int iShipKey, unsigned int iPlanetKey) = 0;

    virtual int AddToGameTable (int iGameClass, int iGameNumber) = 0;
    virtual int RemoveFromGameTable (int iGameClass, int iGameNumber) = 0;

    virtual GameObject* GetGameObject (int iGameClass, int iGameNumber) = 0;

    // Empires
    virtual int DeleteEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, ReasonForRemoval rReason, 
        const GameUpdateInformation* pUpdateInfo) = 0;
    
    virtual int RemoveEmpire (int iEmpireKey) = 0;
    virtual int RemoveEmpireFromGameInternal (int iGameClass, int iGameNumber, int iEmpireKey, 
        int iKillerEmpire) = 0;

    // Ships
    virtual int DeleteShip (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, bool bSafe) = 0;

    virtual int ChangeShipTypeOrMaxBR (const char* pszShips, const char* pszEmpireData, int iShipKey, int iEmpireKey,
        int iOldShipType, int iNewShipType, float fBRChange) = 0;

    // Score
    virtual int UpdateScoresOnNuke (int iNukerKey, int iNukedKey, const char* pszNukerName, 
        const char* pszNukedName, int iGameClass, int iGameNumber, const char* pszGameClassName,
        bool bSurrender = false) = 0;

    virtual int UpdateScoresOnSurrender (int iWinnerKey, int iLoserKey, const char* pszWinnerName, 
        const char* pszLoserName, int iGameClass, int iGameNumber, const char* pszGameClassName) = 0;

    virtual int UpdateScoresOn30StyleSurrender (int iLoserKey, const char* pszLoserName, int iGameClass, 
        int iGameNumber, const char* pszGameClassName) = 0;

    virtual int UpdateScoresOn30StyleSurrenderColonization (int iWinnerKey, int iPlanetKey, 
        const char* pszWinnerName, int iGameClass, int iGameNumber, const char* pszGameClassName) = 0;

    virtual int UpdateScoresOnGameEnd (int iGameClass, int iGameNumber) = 0;

    virtual int UpdateScoresOnNuke (int iGameClass, int iGameNumber, int iNukerKey, int iNukedKey) = 0;
    virtual int UpdateScoresOnSurrender (int iGameClass, int iGameNumber, int iWinnerKey, int iLoserKey) = 0;
    virtual int UpdateScoresOn30StyleSurrender (int iGameClass, int iGameNumber, int iLoserKey) = 0;
    virtual int UpdateScoresOn30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, 
        int iPlanetKey) = 0;
    virtual int UpdateScoresOnWin (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int UpdateScoresOnDraw (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int UpdateScoresOnRuin (int iGameClass, int iGameNumber, int iEmpireKey) = 0;

    virtual int CalculatePrivilegeLevel (int iEmpireKey) = 0;
    virtual int AddNukeToHistory (NukeList nlNukeList, const char* pszGameClassName, int iGameNumber, 
        int iEmpireKey, const char* pszEmpireName, int iAlienKey,
        int iOtherEmpireKey, const char* pszOtherEmpireName, int iOtherAlienKey) = 0;

    virtual bool ValidateEmpireKey (int iLoserKey, unsigned int iHashEmpireName) = 0;

    virtual int GetBridierScore (int iEmpireKey, int* piRank, int* piIndex) = 0;

    // Updates
    virtual int RunUpdate (int iGameClass, int iGameNumber, const UTCTime& tUpdateTime, bool* pbGameOver) = 0;
    
    virtual int VerifyGameTables (int iGameClass, int iGameNumber) = 0;

    // Score
    virtual int ScanEmpiresOnScoreChanges() = 0;

    // Top Lists
    virtual int UpdateTopListOnIncrease (ScoringSystem ssTopList, int iEmpireKey) = 0;
    virtual int UpdateTopListOnDecrease (ScoringSystem ssTopList, int iEmpireKey) = 0;
    virtual int UpdateTopListOnDeletion (ScoringSystem ssTopList, int iEmpireKey) = 0;

    virtual int UpdateTopListOnIncrease (TopListQuery* pQuery) = 0;
    virtual int UpdateTopListOnDecrease (TopListQuery* pQuery) = 0;
    virtual int UpdateTopListOnDeletion (TopListQuery* pQuery) = 0;

    virtual int RebuildTopLists() = 0;

    virtual int MoveEmpireUpInTopList (ScoringSystem ssTopList, int iEmpireKey, unsigned int iKey, 
        const Variant* pvOurData) = 0;

    virtual int MoveEmpireDownInTopList (ScoringSystem ssTopList, int iEmpireKey, unsigned int iKey, 
        const Variant* pvOurData) = 0;

    virtual int PrivateMoveEmpireUpInTopList (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows, 
        int iEmpireKey, unsigned int iKey, const Variant* pvOurData, bool* pbChanged) = 0;

    virtual int PrivateMoveEmpireDownInTopList (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows, 
        int iEmpireKey, unsigned int iKey, const Variant* pvOurData, unsigned int* piNewNumRows, bool* pbChanged) = 0;

    virtual int PrivateFindNewEmpireForTopList (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows, 
        bool bKeep, unsigned int* piNewNumRows) = 0;

    virtual int PrivateFlushTopListData (ScoringSystem ssTopList, Variant** ppvData, unsigned int iNumRows) = 0;

    virtual int InitializeEmptyTopList (ScoringSystem ssTopList) = 0;

    virtual int AddEmpireToTopList (ScoringSystem ssTopList, int iEmpireKey, const Variant* pvOurData) = 0;

    // Planets
    virtual int SetNewMinMaxIfNecessary (int iGameClass, int iGameNumber, int iEmpireKey, int iX, int iY) = 0;

    // GameClasses
    virtual bool DoesGameClassHaveActiveGames (int iGameClass) = 0;

    // Database ops
    virtual int FlushDatabasePrivate (int iEmpireKey) = 0;
    virtual int BackupDatabasePrivate (int iEmpireKey) = 0;
    virtual int RestoreDatabaseBackupPrivate (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion) = 0;
    virtual int DeleteDatabaseBackupPrivate (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion) = 0;
    virtual int PurgeDatabasePrivate (int iEmpireKey, int iCriteria) = 0;
    virtual int DeleteOldDatabaseBackups() = 0;

    // Public
    virtual void FreeData (void** ppData) = 0;
    virtual void FreeData (Variant* pvData) = 0;
    virtual void FreeData (Variant** ppvData) = 0;
    virtual void FreeData (int* piData) = 0;
    virtual void FreeData (float* ppfData) = 0;
    virtual void FreeData (char** ppszData) = 0;
    virtual void FreeData (UTCTime* ptData) = 0;
    virtual void FreeKeys (unsigned int* piKeys) = 0;
    virtual void FreeKeys (int* piKeys) = 0;

    virtual int Initialize() = 0;

    virtual int FlushDatabase (int iEmpireKey) = 0;
    virtual int BackupDatabase (int iEmpireKey) = 0;
    virtual int PurgeDatabase (int iEmpireKey, int iCriteria) = 0;

    virtual int RestoreDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion) = 0;
    virtual int DeleteDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion) = 0;

    virtual IDatabase* GetDatabase() = 0;
    virtual IScoringSystem* GetScoringSystem (ScoringSystem ssScoringSystem) = 0;

    virtual const char* GetSystemVersion() = 0;
    virtual int GetNewSessionId (int64* pi64SessionId) = 0;

    // Rules
    virtual int GetBattleRank (float fTech) = 0;
    virtual int GetMilitaryValue (float fAttack) = 0;
    virtual float GetMaintenanceRatio (int iMin, int iMaint, int iBuild) = 0;
    virtual float GetFuelRatio (int iFuel, int iFuelUse) = 0;
    virtual float GetAgRatio (int iAg, int iPop, float fMaxAgRatio) = 0;
    virtual float GetShipNextBR (float fBR, float fMaint) = 0;
    virtual int GetNextPopulation (int iPop, float fAgRatio) = 0;
    virtual int GetEcon (int iFuel, int iMin, int iAg) = 0;
    virtual float GetTechDevelopment (int iFuel, int iMin, int iMaint, int iBuild, int iFuelUse, float fMaxTechDev) = 0;
    virtual int GetBuildCost (int iType, float fBR) = 0;
    virtual int GetMaintenanceCost (int iType, float fBR) = 0;
    virtual int GetFuelCost (int iType, float fBR) = 0;
    virtual int GetColonizePopulation (int iShipBehavior, float fColonyMultipliedDepositFactor, 
        float fColonyExponentialDepositFactor, float fBR) = 0;
    virtual int GetColonyPopulationBuildCost (int iShipBehavior, float fColonyMultipliedBuildFactor, 
        int iColonySimpleBuildFactor, float fBR) = 0;
    
    virtual int GetTerraformerAg (float fTerraformerPlowFactor, float fBR) = 0;
    virtual int GetTroopshipPop (float fTroopshipInvasionFactor, float fBR) = 0;
    virtual int GetTroopshipFailurePopDecrement (float fTroopshipFailureFactor, float fBR) = 0;
    virtual int GetTroopshipSuccessPopDecrement (float fTroopshipSuccessFactor, int iPop) = 0;
    virtual int GetDoomsdayUpdates (float fDoomsdayAnnihilationFactor, float fBR) = 0;
    
    virtual void GetBuilderNewPlanetResources (float fBR, float fMinBR, float fMultiplier, 
        int iAvgAg, int iAvgMin, int iAvgFuel, 
        int* piNewAvgAg, int* piNewAvgMin, int* piNewAvgFuel) = 0;
    
    virtual float GetGateBRForRange (float fRangeFactor, int iSrcX, int iSrcY, int iDestX, int iDestY) = 0;
    virtual float GetCarrierDESTAbsorption (float fBR) = 0;

    virtual bool IsMobileShip (int iShipType) = 0;

    virtual float GetLateComerTechIncrease (int iPercentTechIncreaseForLatecomers, int iNumUpdates, float fMaxTechDev) = 0;
    virtual int GetMaxPop (int iMin, int iFuel) = 0;

    virtual int GetMaxNumDiplomacyPartners (int iGameClass, int iGameNumber, int iDiplomacyLevel, int* piMaxNumPartners) = 0;

    virtual unsigned int GetNumFairDiplomaticPartners (unsigned int iMaxNumEmpires) = 0;

    virtual bool GameAllowsDiplomacy (int iDiplomacyLevel, int iDip) = 0;
    virtual bool IsLegalDiplomacyLevel (int iDiplomacyLevel) = 0;
    virtual bool IsLegalPrivilege (int iPrivilege) = 0;

    // Locks
    virtual void LockGameClass (int iGameClass, NamedMutex* pnmMutex) = 0;
    virtual void UnlockGameClass (const NamedMutex& nmMutex) = 0;

    virtual void LockGameClasses() = 0;
    virtual void UnlockGameClasses() = 0;

    virtual void LockSuperClasses() = 0;
    virtual void UnlockSuperClasses() = 0;

    virtual void LockEmpireSystemMessages (int iEmpireKey, NamedMutex* pnmMutex) = 0;
    virtual void UnlockEmpireSystemMessages (const NamedMutex& nmMutex) = 0;

    virtual void LockEmpireBridier (int iEmpireKey, NamedMutex* pnmMutex) = 0;
    virtual void UnlockEmpireBridier (const NamedMutex& nmMutex) = 0;
    
    virtual void LockGame (int iGameClass, int iGameNumber, NamedMutex* pnmMutex) = 0;
    virtual void UnlockGame (const NamedMutex& nmMutex) = 0;

    virtual void LockEmpireGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex) = 0;
    virtual void UnlockEmpireGameMessages (const NamedMutex& nmMutex) = 0;

    virtual void LockEmpireShips (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex) = 0;
    virtual void UnlockEmpireShips (const NamedMutex& nmMutex) = 0;

    virtual void LockEmpires() = 0;
    virtual void UnlockEmpires() = 0;

    virtual void LockAlienIcons() = 0;
    virtual void UnlockAlienIcons() = 0;

    virtual void LockAutoUpdate (int iGameClass, int iGameNumber, NamedMutex* pnmMutex) = 0;
    virtual void UnlockAutoUpdate (const NamedMutex& nmMutex) = 0;

    virtual void LockPauseGame (int iGameClass, int iGameNumber, NamedMutex* pnmMutex) = 0;
    virtual void UnlockPauseGame (const NamedMutex& nmMutex) = 0;

    virtual void LockEmpireUpdated (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex) = 0;
    virtual void UnlockEmpireUpdated (const NamedMutex& nmMutex) = 0;

    virtual void LockEmpireTechs (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex) = 0;
    virtual void UnlockEmpireTechs (const NamedMutex& nmMutex) = 0;

    virtual void LockEmpireFleets (int iGameClass, int iGameNumber, int iEmpireKey, NamedMutex* pnmMutex) = 0;
    virtual void UnlockEmpireFleets (const NamedMutex& nmMutex) = 0;

    virtual void LockEmpire (int iEmpireKey, NamedMutex* pnmMutex) = 0;
    virtual void UnlockEmpire (const NamedMutex& nmMutex) = 0;

    virtual int WaitGameReader (int iGameClass, int iGameNumber) = 0;
    virtual int SignalGameReader (int iGameClass, int iGameNumber) = 0;

    virtual int WaitGameWriter (int iGameClass, int iGameNumber) = 0;
    virtual int SignalGameWriter (int iGameClass, int iGameNumber) = 0;

    virtual int WaitForUpdate (int iGameClass, int iGameNumber) = 0;
    virtual int SignalAfterUpdate (int iGameClass, int iGameNumber) = 0;

    virtual void LockGameConfigurationForReading() = 0;
    virtual void UnlockGameConfigurationForReading() = 0;

    virtual void LockGameConfigurationForWriting() = 0;
    virtual void UnlockGameConfigurationForWriting() = 0;

    virtual void LockMapConfigurationForReading() = 0;
    virtual void UnlockMapConfigurationForReading() = 0;

    virtual void LockMapConfigurationForWriting() = 0;
    virtual void UnlockMapConfigurationForWriting() = 0;

    // Update
    virtual int ForceUpdate (int iGameClass, int iGameNumber) = 0;

    virtual int SetEmpireReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet) = 0;
    virtual int SetEmpireNotReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet) = 0;

    // Themes
    virtual int DoesThemeExist (int iThemeKey, bool* pbExist) = 0;
    virtual int CreateTheme (Variant* pvData, unsigned int* piKey) = 0;
    virtual int GetNumThemes (int* piNumThemes) = 0;
    virtual int GetThemeKeys (int** ppiThemeKey, int* piNumKeys) = 0;
    virtual int GetFullThemeKeys (int** ppiThemeKey, int* piNumKeys) = 0;
    virtual int GetThemeData (int iThemeKey, Variant** ppvThemeData) = 0;
    virtual int GetThemeName (int iThemeKey, Variant* pvThemeName) = 0;

    virtual int GetThemeTextColor (int iThemeKey, Variant* pvColor) = 0;
    virtual int GetThemeGoodColor (int iThemeKey, Variant* pvColor) = 0;
    virtual int GetThemeBadColor (int iThemeKey, Variant* pvColor) = 0;
    virtual int GetThemePrivateMessageColor (int iThemeKey, Variant* pvColor) = 0;
    virtual int GetThemeBroadcastMessageColor (int iThemeKey, Variant* pvColor) = 0;
    virtual int GetThemeTableColor (int iThemeKey, Variant* pvTableColor) = 0;

    virtual int DeleteTheme (int iThemeKey) = 0;

    virtual int SetThemeName (int iThemeKey, const char* pszThemeName) = 0;
    virtual int SetThemeVersion (int iThemeName, const char* pszVersion) = 0;
    virtual int SetThemeFileName (int iThemeKey, const char* pszFileName) = 0;
    virtual int SetThemeAuthorName (int iThemeKey, const char* pszAuthorName) = 0;
    virtual int SetThemeAuthorEmail (int iThemeKey, const char* pszAuthorEmail) = 0;
    virtual int SetThemeBackground (int iThemeKey, bool bExists) = 0;
    virtual int SetThemeLivePlanet (int iThemeKey, bool bExists) = 0;
    virtual int SetThemeDeadPlanet (int iThemeKey, bool bExists) = 0;
    virtual int SetThemeSeparator (int iThemeKey, bool bExists) = 0;
    virtual int SetThemeButtons (int iThemeKey, bool bExists) = 0;
    virtual int SetThemeDescription (int iThemeKey, const char* pszDescription) = 0;

    virtual int SetThemeHorz (int iThemeKey, bool bExists) = 0;
    virtual int SetThemeVert (int iThemeKey, bool bExists) = 0;

    virtual int SetThemeTextColor (int iThemeKey, const char* pszColor) = 0;
    virtual int SetThemeGoodColor (int iThemeKey, const char* pszColor) = 0;
    virtual int SetThemeBadColor (int iThemeKey, const char* pszColor) = 0;
    virtual int SetThemePrivateMessageColor (int iThemeKey, const char* pszColor) = 0;
    virtual int SetThemeBroadcastMessageColor (int iThemeKey, const char* pszColor) = 0;
    virtual int SetThemeTableColor (int iThemeKey, const char* pszTableColor) = 0;

    // SuperClasses
    virtual int CreateSuperClass (const char* pszName, int* piKey) = 0;
    virtual int DeleteSuperClass (int iSuperClassKey, bool* pbResult) = 0;
    virtual int GetSuperClassKeys (int** ppiKey, int* piNumSuperClasses) = 0;
    virtual int GetSuperClassKeys (int** ppiKey, Variant** ppvName, int* piNumSuperClasses) = 0;
    virtual int GetSuperClassName (int iKey, Variant* pvName) = 0;
    virtual int RenameSuperClass (int iKey, const char* pszNewName) = 0;

    // GameClasses
    virtual int GetGameClassName (int iGameClass, char pszName [MAX_FULL_GAME_CLASS_NAME_LENGTH]) = 0;
    virtual int GetGameClassUpdatePeriod (int iGameClass, Seconds* piNumSeconds) = 0;
    virtual int GetNextGameNumber (int iGameClass, int* piGameNumber) = 0;

    virtual int GetGameClassOptions (int iGameClass, int* piOptions) = 0;

    virtual int GetMaxNumEmpires (int iGameClass, int* piMaxNumEmpires) = 0;
    virtual int GetMaxNumAllies (int iGameClass, int* piMaxNumAllies) = 0;

    virtual int DeleteGameClass (int iGameClass, bool* pbDeleted) = 0;
    virtual int UndeleteGameClass (int iGameClassKey) = 0;

    virtual int HaltGameClass (int iGameClass) = 0;
    virtual int UnhaltGameClass (int iGameClass) = 0;

    virtual int CreateGameClass (int iCreator, Variant* pvGameClassData, int* piGameClass) = 0;

    virtual int GetSystemGameClassKeys (int** ppiKey, bool** ppbHalted, bool** ppbDeleted, int* piNumKeys) = 0;
    virtual int GetStartableSystemGameClassKeys (int** ppiKey, int* piNumKeys) = 0;

    virtual int GetGameClassSuperClassKey (int iGameClass, int* piSuperClassKey) = 0;
    virtual int SetGameClassSuperClassKey (int iGameClass, int iSuperClassKey) = 0;

    virtual int GetGameClassData (int iGameClass, Variant** ppvData) = 0;
    virtual int GetGameClassOwner (int iGameClass, unsigned int* piOwner) = 0;
    virtual int GetDevelopableTechs (int iGameClass, int* piInitialTechs, int* piDevelopableTechs) = 0;
    
    virtual int GetNumEmpiresRequiredForGameToStart (int iGameClass, int* piNumEmpiresRequired) = 0;
    virtual int DoesGameClassAllowPrivateMessages (int iGameClass, bool* pbPrivateMessages) = 0;
    virtual int DoesGameClassHaveSubjectiveViews (int iGameClass, bool* pbSubjective) = 0;

    virtual int GetGameClassMaxTechIncrease (int iGameClass, float* pfMaxTechIncrease) = 0;
    virtual int GetGameClassVisibleBuilds (int iGameClass, bool* pbVisible) = 0;
    virtual int GetGameClassVisibleDiplomacy (int iGameClass, bool* pbVisible) = 0;
    virtual int GetGameClassDiplomacyLevel (int iGameClass, int* piDiplomacy) = 0;

    virtual int GetMinNumSecsPerUpdateForSystemGameClass (int* piMinNumSecsPerUpdate) = 0;
    virtual int GetMaxNumSecsPerUpdateForSystemGameClass (int* piMaxNumSecsPerUpdate) = 0;
    virtual int GetMaxNumEmpiresForSystemGameClass (int* piMaxNumEmpires) = 0;
    virtual int GetMaxNumPlanetsForSystemGameClass (int* piMaxNumPlanets) = 0;

    virtual int GetMinNumSecsPerUpdateForPersonalGameClass (int* piMinNumSecsPerUpdate) = 0;
    virtual int GetMaxNumSecsPerUpdateForPersonalGameClass (int* piMaxNumSecsPerUpdate) = 0;
    virtual int GetMaxNumEmpiresForPersonalGameClass (int* piMaxNumEmpires) = 0;
    virtual int GetMaxNumPlanetsForPersonalGameClass (int* piMaxNumPlanets) = 0;

    virtual int SetMinNumSecsPerUpdateForSystemGameClass (int iMinNumSecsPerUpdate) = 0;
    virtual int SetMaxNumSecsPerUpdateForSystemGameClass (int iMaxNumSecsPerUpdate) = 0;
    virtual int SetMaxNumEmpiresForSystemGameClass (int iMaxNumEmpires) = 0;
    virtual int SetMaxNumPlanetsForSystemGameClass (int iMaxNumPlanets) = 0;

    virtual int SetMinNumSecsPerUpdateForPersonalGameClass (int iMinNumSecsPerUpdate) = 0;
    virtual int SetMaxNumSecsPerUpdateForPersonalGameClass (int iMaxNumSecsPerUpdate) = 0;

    virtual int SetMaxNumEmpiresForPersonalGameClass (int iMaxNumEmpires) = 0;
    virtual int SetMaxNumPlanetsForPersonalGameClass (int iMaxNumPlanets) = 0;

    virtual int GetGameClassProperty (int iGameClass, unsigned int iProperty, Variant* pvProperty) = 0;

    // Games
    virtual int GetGameCreationTime (int iGameClass, int iGameNumber, UTCTime* ptCreationTime) = 0;
    virtual int GetGameState (int iGameClass, int iGameNumber, int* piGameState) = 0;

    virtual int DeleteGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, int iReason) = 0;
    
    virtual int GetNumActiveGames (int* piNumGames) = 0;
    virtual int GetNumOpenGames (int* piNumGames) = 0;
    virtual int GetNumClosedGames (int* piNumGames) = 0;

    virtual int GetActiveGames (int** ppiGameClasses, int** ppiGameNumbers, int* piNumGames) = 0;
    virtual int GetOpenGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames) = 0;
    virtual int GetClosedGames (int** ppiGameClass, int** ppiGameNumber, int* piNumGames) = 0;

    virtual int IsGameOpen (int iGameClass, int iGameNumber, bool* pbOpen) = 0;
    virtual int HasGameStarted (int iGameClass, int iGameNumber, bool* pbStarted) = 0;

    virtual int IsGamePasswordProtected (int iGameClass, int iGameNumber, bool* pbProtected) = 0;
    virtual int GetGamePassword (int iGameClass, int iGameNumber, Variant* pvPassword) = 0;
    virtual int SetGamePassword (int iGameClass, int iGameNumber, const char* pszNewPassword) = 0;

    virtual int CreateGame (int iGameClass, int iEmpireCreator, const GameOptions& goGameOptions, int* piGameNumber) = 0;
    virtual int EnterGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszPassword, int* piNumUpdates, bool bSendMessages, bool bGameAlreadyLocked) = 0;

    virtual int DoesGameExist (int iGameClass, int iGameNumber, bool* pbExists) = 0;
    virtual int GetNumUpdates (int iGameClass, int iGameNumber, int* piNumUpdates) = 0;
    
    virtual int GetNumUpdatesBeforeGameCloses (int iGameClass, int iGameNumber, int* piNumUpdates) = 0;
    virtual int GetGameOptions (int iGameClass, int iGameNumber, int* piOptions) = 0;
    virtual int GetFirstUpdateDelay (int iGameClass, int iGameNumber, Seconds* psDelay) = 0;

    virtual int GetNumEmpiresInGame (int iGameClass, int iGameNumber, int* piEmpires) = 0;
    virtual int GetNumEmpiresNeededForGame (int iGameClass, int* piNumEmpiresNeeded) = 0;
    virtual int IsEmpireInGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbInGame) = 0;
    virtual int GetNumUpdatedEmpires (int iGameClass, int iGameNumber, int* piUpdatedEmpires) = 0;
    virtual int GetEmpiresInGame (int iGameClass, int iGameNumber, Variant** ppiEmpireKey, int* piNumEmpires) = 0;

    virtual int IsGamePaused (int iGameClass, int iGameNumber, bool* pbPaused) = 0;
    virtual int IsGameAdminPaused (int iGameClass, int iGameNumber, bool* pbAdminPaused) = 0;

    virtual void GetGameClassGameNumber (const char* pszGameData, int* piGameClass, int* piGameNumber) = 0;
    virtual void GetGameClassGameNumber (int iGameClass, int iGameNumber, char* pszGameData) = 0;

    virtual int PauseAllGames() = 0;
    virtual int UnpauseAllGames() = 0;

    virtual int LogEmpireIntoGame (int iGameClass, int iGameNumber, int iEmpireKey) = 0;

    virtual int RuinGame (int iGameClass, int iGameNumber, const char* pszWinnerName) = 0;

    virtual int GetResignedEmpiresInGame (int iGameClass, int iGameNumber, int** ppiEmpireKey, int* piNumResigned) = 0;
    virtual int UnresignEmpire (int iGameClass, int iGameNumber, int iEmpireKey, int iAdminKey) = 0;

    virtual int GetGameEntryRestrictions (int iGameClass, int iGameNumber, int* piOptions, 
        Variant pvRestrictionMin [NUM_ENTRY_SCORE_RESTRICTIONS], 
        Variant pvRestrictionMax [NUM_ENTRY_SCORE_RESTRICTIONS]) = 0;

    virtual int GameAccessCheck (int iGameClass, int iGameNumber, int iEmpireKey, 
        const GameOptions* pgoGameOptions, GameAction gaAction, bool* pbAccess) = 0;

    virtual int GetBridierRankPotentialGainLoss (int iGameClass, int iGameNumber, int iEmpireKey, int* piGain, int* piLoss) = 0;

    // GameEmpireData
    virtual int QuitEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire = NO_KEY) = 0;
    virtual int ResignEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int SurrenderEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, SurrenderType sType) = 0;

    virtual int HasEmpireResignedFromGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbResigned) = 0;

    // Empires
    virtual int GetEmpireName (int iEmpireKey, Variant* pvName) = 0;
    virtual int GetEmpireName (int iEmpireKey, char pszName [MAX_EMPIRE_NAME_LENGTH + 1]) = 0;
    virtual int SetEmpireName (int iEmpireKey, const char* pszName) = 0;
    
    virtual int SetEmpirePassword (int iEmpireKey, const char* pszPassword) = 0;
    virtual int ChangeEmpirePassword (int iEmpireKey, const char* pszPassword) = 0;

    virtual int SetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int iMaxNumSavedMessages) = 0;
    
    virtual int UpdateEmpireQuote (int iEmpireKey, const char* pszQuote, bool* pbTruncated) = 0;
    virtual int UpdateEmpireVictorySneer (int iEmpireKey, const char* pszSneer, bool* pbTruncated) = 0;

    virtual int SendVictorySneer (int iWinnerKey, const char* pszWinnerName, int iLoserKey) = 0;

    virtual int SetEmpireBackgroundKey (int iEmpireKey, int iBackgroundKey) = 0;
    virtual int SetEmpireLivePlanetKey (int iEmpireKey, int iLivePlanetKey) = 0;
    virtual int SetEmpireDeadPlanetKey (int iEmpireKey, int iDeadPlanetKey) = 0;
    virtual int SetEmpireButtonKey (int iEmpireKey, int iButtonKey) = 0;
    virtual int SetEmpireSeparatorKey (int iEmpireKey, int iSeparatorKey) = 0;
    virtual int SetEmpireHorzKey (int iEmpireKey, int iHorzKey) = 0;
    virtual int SetEmpireVertKey (int iEmpireKey, int iVertKey) = 0;

    virtual int SetEmpireColorKey (int iEmpireKey, int iColorKey) = 0;

    virtual int GetEmpireLivePlanetKey (int iEmpireKey, int* piLivePlanetKey) = 0;
    virtual int GetEmpireDeadPlanetKey (int iEmpireKey, int* piDeadPlanetKey) = 0;

    virtual int SetEmpireThemeKey (int iEmpireKey, int iThemeKey) = 0;

    virtual int GetEmpireMaxNumSavedSystemMessages (int iEmpireKey, int* piMaxNumSavedMessages) = 0;

    virtual int DeleteEmpire (int iEmpireKey) = 0;
    virtual int ObliterateEmpire (int iEmpireKey, int iKillerEmpire) = 0;

    virtual int RemoveEmpireFromGame (int iGameClass, int iGameNumber, unsigned int iEmpireKey, unsigned int iKillerEmpire) = 0;

    virtual int DoesEmpireExist (const char* pszName, bool* pbExists, unsigned int* piEmpireKey, Variant* pvEmpireName) = 0;
    virtual int DoesEmpireExist (unsigned int iEmpireKey, bool* pbExists) = 0;

    virtual int DoesEmpireKeyMatchName (int iEmpireKey, const char* pszEmpireName, bool* pbMatch) = 0;
    virtual int IsPasswordCorrect (int iEmpireKey, const char* pszPassword) = 0;

    virtual int GetNumEmpiresOnServer (int* piNumEmpires) = 0;

    virtual int UndeleteEmpire (int iEmpireKey) = 0;
    virtual int BlankEmpireStatistics (int iEmpireKey) = 0;

    virtual int GetEmpirePersonalGameClasses (int iEmpireKey, int** ppiGameClassKey, Variant** ppvName, int* piNumKeys) = 0;
    virtual int GetEmpireData (int iEmpireKey, Variant** ppvEmpData, int* piNumActiveGames) = 0;
    virtual int GetEmpireActiveGames (int iEmpireKey, int** ppiGameClass, int** ppiGameNumber, int* piNumGames) = 0;

    virtual int GetEmpirePrivilege (int iEmpireKey, int* piPrivilege) = 0;
    virtual int SetEmpirePrivilege (int iEmpireKey, int iPrivilege) = 0;

    virtual int GetEmpireAlmonasterScore (int iEmpireKey, float* pfAlmonasterScore) = 0;
    virtual int SetEmpireAlmonasterScore (int iEmpireKey, float fAlmonasterScore) = 0;

    virtual int GetEmpirePassword (int iEmpireKey, Variant* pvPassword) = 0;

    virtual int GetEmpireDataColumn (int iEmpireKey, unsigned int iColumn, Variant* pvData) = 0;

    virtual int GetNumLogins (int iEmpireKey, int* piNumLogins) = 0;

    virtual int GetEmpireMaxNumShipsBuiltAtOnce (int iEmpireKey, int* piMaxNumShipsBuiltAtOnce) = 0;
    virtual int SetEmpireMaxNumShipsBuiltAtOnce (int iEmpireKey, int iMaxNumShipsBuiltAtOnce) = 0;

    virtual int GetEmpireIPAddress (int iEmpireKey, Variant* pvIPAddress) = 0;
    virtual int SetEmpireIPAddress (int iEmpireKey, const char* pszIPAddress) = 0;

    virtual int GetEmpireSessionId (int iEmpireKey, int64* pi64SessionId) = 0;
    virtual int SetEmpireSessionId (int iEmpireKey, int64 i64SessionId) = 0;

    virtual int GetEmpireDefaultBuilderPlanet (int iEmpireKey, int* piDefaultBuildPlanet) = 0;
    virtual int SetEmpireDefaultBuilderPlanet (int iEmpireKey, int iDefaultBuildPlanet) = 0;

    virtual int ResetEmpireSessionId (int iEmpireKey) = 0;
    virtual int EndResetEmpireSessionId (int iEmpireKey) = 0;

    virtual int GetEmpireOptions (int iEmpireKey, int* piOptions) = 0;
    virtual int GetEmpireLastBridierActivity (int iEmpireKey, UTCTime* ptTime) = 0;

    virtual int GetEmpireProperty (int iEmpireKey, unsigned int iProperty, Variant* pvProperty) = 0;
    virtual int SetEmpireProperty (int iEmpireKey, unsigned int iProperty, const Variant& vProperty) = 0;

    //
    // Game Empire Data
    //

    virtual int GetEmpireOptions (int iGameClass, int iGameNumber, int iEmpireKey, int* piOptions) = 0;

    virtual int GetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, bool* pbOption) = 0;
    virtual int SetEmpireOption (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iFlag, bool bOption) = 0;

    virtual int GetEmpirePartialMapData (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbPartialMaps, 
        unsigned int* piCenterKey, unsigned int* piXRadius, unsigned int* piRadiusY) = 0;

    virtual int SetEmpirePartialMapCenter (int iGameClass, int iGameNumber, int iEmpireKey, int iCenterKey) = 0;
    virtual int SetEmpirePartialMapXRadius (int iGameClass, int iGameNumber, int iEmpireKey, int iXRadius) = 0;
    virtual int SetEmpirePartialMapYRadius (int iGameClass, int iGameNumber, int iEmpireKey, int iYRadius) = 0;

    virtual int GetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
        int* piDefaultBuildPlanet, int* piResolvedPlanetKey) = 0;
    virtual int SetEmpireDefaultBuilderPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iDefaultBuildPlanet) = 0;

    virtual int GetEmpireBR (int iGameClass, int iGameNumber, int iEmpireKey, int* piBR) = 0;
    virtual int GetEmpireMaintenanceRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfMaintenanceRatio) = 0;

    virtual int GetEmpireDefaultMessageTarget (int iEmpireKey, int* piMessageTarget) = 0;
    virtual int SetEmpireDefaultMessageTarget (int iEmpireKey, int iMessageTarget) = 0;

    // System Messages
    virtual int SendSystemMessage (int iEmpireKey, const char* pszMessage, int iSource, bool bBroadcast = false) = 0;
    virtual int DeliverSystemMessage (int iEmpireKey, const Variant* pvData) = 0;

    virtual int GetNumSystemMessages (int iEmpireKey, int* piNumber) = 0;
    virtual int GetNumUnreadSystemMessages (int iEmpireKey, int* piNumber) = 0;
    virtual int SendMessageToAll (int iEmpireKey, const char* pszMessage) = 0;
    virtual int GetSavedSystemMessages (int iEmpireKey, int** ppiMessageKey, Variant*** pppvMessage, int* piNumMessages) = 0;

    virtual int GetUnreadSystemMessages (int iEmpireKey, Variant*** pppvMessage, unsigned int** ppiMessageKey, int* piNumMessages) = 0;
    
    virtual int DeleteSystemMessage (int iEmpireKey, int iKey) = 0;

    // Game Messages
    virtual int GetNumGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumber) = 0;
    virtual int GetNumUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumber) = 0;
    virtual int GetSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiMessageKey,
        Variant*** ppvData, int* piNumMessages) = 0;
    
    virtual int SendGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, 
        int iSource, bool bBroadcast, bool bUpdateMessage, const UTCTime& tSendTime) = 0;
    
    virtual int GetUnreadGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, Variant*** pppvMessage, 
        int* piNumMessages) = 0;
    
    virtual int DeleteGameMessage (int iGameClass, int iGameNumber, int iEmpireKey, int iMessageKey) = 0;
    virtual int BroadcastGameMessage (int iGameClass, int iGameNumber, const char* pszMessage, int iSourceKey, bool bAdmin) = 0;

    // Planets
    virtual int GetPlanetCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, int* piPlanetX, int* piPlanetY) = 0;
    virtual int GetPlanetKeyFromCoordinates (int iGameClass, int iGameNumber, int iX, int iY, int* piPlanetKey) = 0;

    virtual int GetNeighbourPlanetKey (int iGameClass, int iGameNumber, int iPlanetKey, int iDirection, 
        int* piPlanetKey) = 0;
    virtual int HasEmpireExploredPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
        bool* pbExplored) = 0;

    virtual void GetCoordinates (const char* pszCoord, int* piX, int* piY) = 0;
    virtual void GetCoordinates (int iX, int iY, char* pszCoord) = 0;

    virtual int GetLowestDiplomacyLevelForShipsOnPlanet (int iGameClass, int iGameNumber, int iEmpireKey, 
        int iPlanetKey, bool bVisibleBuilds, Variant* pvEmpireKey, unsigned int& iNumEmpires, 
        int* piNumForeignShipsOnPlanet, int* piDiplomacyLevel, Variant** ppvEmpireKey) = 0;

    virtual int SharePlanetsBetweenFriends (int iGameClass, int iGameNumber, 
        unsigned int iEmpireIndex1, unsigned int iEmpireIndex2,
        const char** pstrEmpireMap, const char** pstrEmpireDip, const char** pstrEmpireData, 
        const char* pszGameMap, unsigned int iNumEmpires, unsigned int* piEmpireKey, int iDipLevel) = 0;

    // Score
    virtual int GetNumEmpiresInNukeHistory (int iEmpireKey, int* piNumNukes, int* piNumNuked) = 0;
    virtual int GetNukeHistory (int iEmpireKey, int* piNumNuked, Variant*** pppvNukedData, int* piNumNukers, 
        Variant*** pppvNukerData) = 0;

    virtual int GetSystemNukeHistory (int* piNumNukes, Variant*** pppvNukedData) = 0;
    virtual int GetSystemLatestGames (int* piNumGames, Variant*** pppvGameData) = 0;

    virtual int TriggerBridierTimeBombIfNecessary() = 0;

    virtual int GetBridierTimeBombScanFrequency (Seconds* piFrequency) = 0;
    virtual int SetBridierTimeBombScanFrequency (Seconds iFrequency) = 0;

    // System Config
    virtual int GetDefaultUIKeys (unsigned int* piBackground, unsigned int* piLivePlanet, 
        unsigned int* piDeadPlanet, unsigned int* piButtons, unsigned int* piSeparator, 
        unsigned int* piHorz, unsigned int* piVert, unsigned int* piColor) = 0;

    virtual int SetScoreForPrivilege (Privilege privLevel, float fScore) = 0;
    virtual int GetScoreForPrivilege (Privilege privLevel, float* pfScore) = 0;

    virtual int SetSystemOption (int iOption, bool bFlag) = 0;
    virtual int SetSystemProperty (int iColumn, const Variant& vProperty) = 0;

    virtual int GetSystemOptions (int* piOptions) = 0;
    virtual int GetDefaultGameOptions (int iGameClass, GameOptions* pgoOptions) = 0;

    // Aliens
    virtual int GetNumAliens (int* piNumAliens) = 0;
    virtual int GetAlienKeys (Variant*** pppvData, int* piNumAliens) = 0;
    virtual int CreateAlienIcon (int iAlienKey, const char* pszAuthorName) = 0;
    virtual int DeleteAlienIcon (int iAlienKey) = 0;

    // Top Lists
    virtual int GetTopList (ScoringSystem ssListType, Variant*** pppvData, unsigned int* piNumEmpires) = 0;

    // Search
    virtual int PerformMultipleSearch (int iStartKey, int iSkipHits, int iMaxNumHits, int iNumCols, 
        const unsigned int* piColumn, const unsigned int* piFlags, const Variant* pvData, const Variant* pvData2, 
        int** ppiKey, int* piNumHits, int* piStopKey) = 0;

    // Updates
    virtual int CheckGameForUpdates (int iGameClass, int iGameNumber, bool* pbUpdate) = 0;
    virtual int CheckAllGamesForUpdates() = 0;

    //////////
    // Game //
    //////////

    // Tech
    virtual int GetNumAvailableTechs (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumAvailableTechs) = 0;
    virtual int GetDevelopedTechs (int iGameClass, int iGameNumber, int iEmpireKey, int* piTechDevs, int* piTechUndevs) = 0;
    virtual int RegisterNewTechDevelopment (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey) = 0;
    virtual int GetDefaultEmpireShipName (int iEmpireKey, int iTechKey, Variant* pvDefaultShipName) = 0;
    virtual int SetDefaultEmpireShipName (int iEmpireKey, int iTechKey, const char* pszDefaultShipName) = 0;
    virtual int GetNumTechs (int iTechBitmap) = 0;

    // Options
    virtual int SetEmpireAutoUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool bAutoUpdate) = 0;

    virtual int GetEmpireMaxNumSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumMessages) = 0;
    virtual int SetEmpireMaxNumSavedGameMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iNumMessages) = 0;

    virtual int GetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, bool* pbIgnore) = 0;
    virtual int SetEmpireIgnoreMessages (int iGameClass, int iGameNumber, int iEmpireKey, int iIgnoredEmpire, bool bIgnore) = 0;

    virtual int GetEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, Variant* pvNotepad) = 0;
    virtual int UpdateGameEmpireNotepad (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszNotepad, 
        bool* pbTruncated) = 0;

    virtual int SearchForDuplicates (int iGameClass, int iGameNumber, unsigned int iSystemEmpireDataColumn,
        unsigned int iGameEmpireDataColumn, int** ppiDuplicateKeys, unsigned int** ppiNumDuplicatesInList, 
        unsigned int* piNumDuplicates) = 0;

    virtual int DoesEmpireHaveDuplicates (int iGameClass, int iGameNumber, int iEmpireKey, int iSystemEmpireDataColumn,
        int** ppiDuplicateKeys, unsigned int* piNumDuplicates) = 0;

    virtual int GetEmpireDefaultMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int* piMessageTarget) = 0;
    virtual int SetEmpireDefaultMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int iMessageTarget) = 0;

    // Diplomacy
    virtual int GetKnownEmpireKeys (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvEmpireKey, 
        int* piNumEmpires) = 0;
    virtual int GetDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
        int* piWeOffer, int* piTheyOffer, int* piCurrent) = 0;
    virtual int GetDiplomaticOptions (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, 
        int piDipOptKey[], int* piSelected, int* piNumOptions) = 0;
    virtual int UpdateDiplomaticOffer (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, int iDipOffer) = 0;

    virtual int AdminPauseGame (int iGameClass, int iGameNumber, bool bBroadcast) = 0;
    virtual int AdminUnpauseGame (int iGameClass, int iGameNumber, bool bBroadcast) = 0;

    virtual int GetNumEmpiresAtDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey,
        int* piWar, int* piTruce, int* piTrade, int* piAlliance) = 0;

    virtual int GetEmpiresAtDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey,
        int piWar[], int* piNumWar, int piTruce[], int* piNumTruce, int piTrade[], int* piNumTrade, 
        int piAlliance[], int* piNumAlliance) = 0;

    virtual int GetLastUsedMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int* piLastUsedMask,
        int** ppiLastUsedProxyKeyArray, int* piNumLastUsed) = 0;

    virtual int SetLastUsedMessageTarget (int iGameClass, int iGameNumber, int iEmpireKey, int iLastUsedMask,
        int* piLastUsedKeyArray, int iNumLastUsed) = 0;

    // Map
    virtual int GetMapLimits (int iGameClass, int iGameNumber, int iEmpireKey, int* piMinX, int* piMaxX, 
        int* piMinY, int* piMaxY) = 0;
    virtual int GetMapLimits (int iGameClass, int iGameNumber, int* piMinX, int* piMaxX, int* piMinY, int* piMaxY) = 0;

    virtual int HasEmpireVisitedPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, bool* pbVisited) = 0;

    virtual int GetPlanetShipOwnerData (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, 
        int iPlanetProxyKey, int iTotalNumShips, bool bVisibleBuilds, bool bIndependence, 
        int** ppiShipOwnerData) = 0;

    virtual int GetPlanetName (int iGameClass, int iGameNumber, int iPlanetKey, Variant* pvPlanetName) = 0;
    virtual int GetPlanetNameWithSecurity (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey,  
        Variant* pvPlanetName) = 0;

    virtual int GetPlanetNameWithCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, char pszName [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH]) = 0;

    virtual int DoesPlanetExist (int iGameClass, int iGameNumber, int iPlanetKey, bool* pbExists) = 0;

    virtual int RenamePlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, const char* pszNewName) = 0;
    virtual int SetPlanetMaxPop (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, int iNewMaxPop) = 0;

    virtual int GetGameAveragePlanetResources (int iGameClass, int iGameNumber, int* piAg, int* piMin, int* piFuel) = 0;
    virtual int GetEmpireAgRatio (int iGameClass, int iGameNumber, int iEmpireKey, float* pfAgRatio) = 0;

    virtual int GetVisitedSurroundingPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int iCenterKey, 
        Variant* pvPlanetKey, int* piProxyKey, int* piNumPlanets, int* piCenterX, int* piCenterY, 
        int* piMinX, int* piMaxX, int* piMinY, int* piMaxY) = 0;

    // Build
    virtual int GetBuilderPlanetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiBuilderKey, 
        int* piNumBuilders) = 0;

    virtual int BuildNewShips (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey, int iNumShips, 
        const char* pszShipName, float fBR, int iPlanetKey, int iFleetKey, int* piNumShipsBuilt, 
        bool* pbBuildReduced) = 0;

    virtual int GetNumBuilds (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumBuilds) = 0;
    virtual int CancelAllBuilds (int iGameClass, int iGameNumber, int iEmpireKey) = 0;

    // Ships
    virtual int GetShipOrders (int iGameClass, int iGameNumber, int iEmpireKey, 
        const GameConfiguration& gcConfig, int iShipKey, int iShipType, float fBR, float fMaintRatio, 
        int iPlanetKey, int iLocationX, int iLocationY, int** ppiOrderKey, String** ppstrOrderText, 
        int* piNumOrders, int* piSelected) = 0;

    virtual int UpdateShipName (int iGameClass, int iGameNumber, int iEmpireKey, int iShipKey, const char* pszNewName) = 0;

    virtual int GetNumShips (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumShips) = 0;
    virtual int GetNumFleets (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumFleets) = 0;

    // Fleets
    virtual int GetEmpireFleetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiFleetKeys, 
        int* piNumFleets) = 0;

    virtual int GetFleetLocation (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int* piPlanetKey) = 0;
    virtual int GetFleetName (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, Variant* pvFleetName) = 0;
    virtual int GetNewFleetLocations (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiLocationKey, 
        int* piNumLocations) = 0;

    virtual int CreateNewFleet (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszFleetName, 
        int iPlanetKey) = 0;

    virtual int GetFleetOrders (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int** ppiOrderKey, 
        String** ppstrOrderText, int* piSelected, int* piNumOrders) = 0;

    virtual int UpdateFleetName (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, 
        const char* pszNewName) = 0;

    virtual int UpdateFleetOrders (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int iOrderKey) = 0;

    virtual int GetNumShipsInFleet (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int* piNumShips) = 0;

    // Auxiliary database functions
    virtual bool IsDatabaseBackingUp() = 0;

    virtual void GetDatabaseBackupProgress (DatabaseBackupStage* pdbsStage, Seconds* piElapsedTime, 
        unsigned int* piNumber) = 0;

    // Tournaments
    virtual int GetOwnedTournaments (int iEmpireKey, unsigned int** ppiTournamentKey, Variant** ppvName, unsigned int* piNumTournaments) = 0;
    virtual int GetJoinedTournaments (int iEmpireKey, unsigned int** ppiTournamentKey, Variant** ppvName, unsigned int* piNumTournaments) = 0;
    
    virtual int CreateTournament (Variant* pvTournamentData, unsigned int* piTournamentKey) = 0;
    virtual int DeleteTournament (int iEmpireKey, unsigned int iTournamentKey, bool bOwnerDeleted) = 0;

    virtual int GetTournamentData (unsigned int iTournamentKey, Variant** ppvTournamentData) = 0;

    virtual int GetTournamentName (unsigned int iTournamentKey, Variant* pvTournamentName) = 0;
    virtual int GetTournamentOwner (unsigned int iTournamentKey, unsigned int* piOwnerKey) = 0;

    virtual int GetTournamentDescription (unsigned int iTournamentKey, Variant* pvTournamentDesc) = 0;
    virtual int SetTournamentDescription (unsigned int iTournamentKey, const char* pszTournamentDesc) = 0;
    
    virtual int GetTournamentUrl (unsigned int iTournamentKey, Variant* pvTournamentUrl) = 0;
    virtual int SetTournamentUrl (unsigned int iTournamentKey, const char* pszTournamentUrl) = 0;

    virtual int GetTournamentNews (unsigned int iTournamentKey, Variant* pvTournamentNews) = 0;
    virtual int SetTournamentNews (unsigned int iTournamentKey, const char* pszTournamentNews) = 0;

    virtual int InviteEmpireIntoTournament (unsigned int iTournamentKey, int iOwnerKey, int iSourceKey, int iInviteKey) = 0;
    virtual int InviteSelfIntoTournament (unsigned int iTournamentKey, int iEmpireKey) = 0;

    virtual int RespondToTournamentInvitation (int iInviteKey, int iMessageKey, bool bAccept) = 0;
    virtual int RespondToTournamentJoinRequest (int iEmpireKey, int iMessageKey, bool bAccept) = 0;

    virtual int DeleteEmpireFromTournament (unsigned int iTournamentKey, int iDeleteKey) = 0;

    virtual int GetTournamentGameClasses (unsigned int iTournamentKey, unsigned int** ppiGameClassKey, 
        Variant** ppvName, unsigned int* piNumKeys) = 0;

    virtual int GetTournamentEmpires (unsigned int iTournamentKey, unsigned int** ppiEmpireKey, 
        unsigned int** ppiTeamKey, Variant** ppvName, unsigned int* piNumKeys) = 0;

    virtual int GetAvailableTournamentEmpires (unsigned int iTournamentKey, unsigned int** ppiEmpireKey, 
        unsigned int** ppiTeamKey, Variant** ppvName, unsigned int* piNumKeys) = 0;

    virtual int GetTournamentTeamEmpires (unsigned int iTournamentKey, unsigned int iTeamKey, 
        int** ppiEmpireKey, Variant** ppvName, unsigned int* piNumKeys) = 0;

    virtual int GetTournamentTeams (unsigned int iTournamentKey, unsigned int** ppiTeamKey, 
        Variant** ppvName, unsigned int* piNumKeys) = 0;

    virtual int GetTournamentGames (unsigned int iTournamentKey, int** ppiGameClass, int** ppiGameNumber, unsigned int* piNumGames) = 0;

    virtual int CreateTournamentTeam (unsigned int iTournamentKey, Variant* pvTeamData, unsigned int* piTeamKey) = 0;
    virtual int DeleteTournamentTeam (unsigned int iTournamentKey, unsigned int iTeamKey) = 0;

    virtual int SetEmpireTournamentTeam (unsigned int iTournamentKey, int iEmpireKey, unsigned int iTeamKey) = 0;

    virtual int GetTournamentTeamData (unsigned int iTournamentKey, unsigned int iTeamKey, Variant** ppvTournamentTeamData) = 0;
    
    virtual int GetTournamentEmpireData (unsigned int iTournamentKey, unsigned int iEmpireKey, Variant** ppvTournamentEmpireData) = 0;

    virtual int GetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, Variant* pvTournamentTeamDesc) = 0;
    virtual int SetTournamentTeamDescription (unsigned int iTournamentKey, unsigned int iTeamKey, const char* pszTournamentTeamDesc) = 0;
    
    virtual int GetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, Variant* pvTournamentTeamUrl) = 0;
    virtual int SetTournamentTeamUrl (unsigned int iTournamentKey, unsigned int iTeamKey, const char* pszTournamentTeamUrl) = 0;

    virtual int GetGameClassTournament (int iGameClass, unsigned int* piTournamentKey) = 0;

    virtual int GetTournamentIcon (unsigned int iTournamentKey, unsigned int* piIcon) = 0;
    virtual int SetTournamentIcon (unsigned int iTournamentKey, unsigned int iIcon) = 0;

    virtual int GetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int* piIcon) = 0;
    virtual int SetTournamentTeamIcon (unsigned int iTournamentKey, unsigned int iTeamKey, unsigned int iIcon) = 0;
};

class IAlmonasterHook : virtual public IObject {
public:

    virtual int Initialize (IGameEngine* pGameEngine) = 0;
    virtual int Setup() = 0;
    virtual int Running() = 0;
    virtual int Finalize() = 0;
};

#endif // !defined(AFX_IGAMEENGINE_H__58727607_5549_11D1_9D09_0060083E8062__INCLUDED_)