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

#include <stdio.h>
#include <limits.h>

#define ALMONASTER_BUILD

#include "GameEngineSchema.h"
#include "GameEngineConstants.h"
#include "GameEngineUI.h"
#include "AsyncManager.h"

#include "Osal/Thread.h"
#include "Osal/Event.h"
#include "Osal/FifoQueue.h"
#include "Osal/Library.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/HashTable.h"
#include "Osal/Vector.h"

// Remove annoying warning
#ifdef _WIN32
#pragma warning (disable : 4245)
#endif

//
// Types
//

struct EmpireIdentity
{
    int iEmpireKey;
    int64 i64SecretKey;
};

struct GameConfiguration
{
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
    float fBuilderBRDampener;
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

struct MapConfiguration
{
    int iChanceNewLinkForms;
    int iMapDeviation;
    int iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap;
    int iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap;
    int iLargeMapThreshold;
    float fResourceAllocationRandomizationFactor;
};

struct GameSecurityEntry
{
    int iEmpireKey;
    int iOptions;
    const char* pszEmpireName;
    int64 iSecretKey;
};

struct PrearrangedTeam
{
    unsigned int iNumEmpires;
    unsigned int* piEmpireKey;
};

struct GameOptions
{
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

    GameFairnessOption gfoFairness;

    unsigned int iNumSecurityEntries;
    GameSecurityEntry* pSecurity;
};

void InitGameOptions (GameOptions* pgoOptions);
void ClearGameOptions(GameOptions* pgoOptions);

class AutoClearGameOptions
{
private:
    GameOptions& m_goOptions;
public:

    AutoClearGameOptions(GameOptions& goOptions) : m_goOptions(goOptions)
    {
    }

    ~AutoClearGameOptions()
    {
        ClearGameOptions(&m_goOptions);
    }
};

enum GameAction
{
    VIEW_GAME,
    ENTER_GAME,
};

enum GameResult
{
    GAME_RESULT_NONE,
    GAME_RESULT_RUIN,
    GAME_RESULT_WIN,
    GAME_RESULT_DRAW
};

struct GameUpdateInformation
{
    unsigned int iUpdate;
    unsigned int iNumEmpires;
    int* piEmpireKey;
    bool* pbAlive;
    String* pstrUpdateMessage;
};

struct RatioInformation
{
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

struct BuildLocation
{
    unsigned int iPlanetKey;
    unsigned int iFleetKey;
};

enum FleetOrderType
{
    FLEET_ORDER_NORMAL      = 0,
    FLEET_ORDER_MERGE       = 1,
    FLEET_ORDER_MOVE_PLANET = 2,
    FLEET_ORDER_MOVE_FLEET  = 3,

    FLEET_ORDER_TYPE_FIRST  = FLEET_ORDER_NORMAL,
    FLEET_ORDER_TYPE_LAST   = FLEET_ORDER_MOVE_FLEET,
};

struct FleetOrder
{
    int iKey;
    char* pszText;
    FleetOrderType fotType;
};

enum ShipOrderType
{
    SHIP_ORDER_NORMAL       = 0,
    SHIP_ORDER_MOVE_PLANET  = 2,
    SHIP_ORDER_MOVE_FLEET   = 3,

    SHIP_ORDER_TYPE_FIRST  = SHIP_ORDER_NORMAL,
    SHIP_ORDER_TYPE_LAST   = SHIP_ORDER_MOVE_FLEET,
};

struct ShipOrder
{
    int iKey;
    char* pszText;
    ShipOrderType sotType;
};

struct ShipOrderPlanetInfo
{
    const char* pszName;
    unsigned int iPlanetKey;
    unsigned int iOwner;
    int iX;
    int iY;
};

struct ShipOrderShipInfo
{
    int iShipType;
    int iSelectedAction;
    int iState;
    float fBR;
    float fMaxBR;
    bool bBuilding;
};

struct ShipOrderGameInfo
{
    int iGameClassOptions;
    float fMaintRatio;
    float fNextMaintRatio;
};

struct ScoringChanges
{
    ScoringChanges()
    {
        iFlags = 0;
        pszNukedName = NULL;
    }

    int iFlags;
    const char* pszNukedName;

    float fAlmonasterNukerScore;
    float fAlmonasterNukerChange;
    float fAlmonasterNukedScore;
    float fAlmonasterNukedChange;

    int iBridierNukerRank;
    int iBridierNukerRankChange;
    int iBridierNukerIndex;
    int iBridierNukerIndexChange;

    int iBridierNukedRank;
    int iBridierNukedRankChange;
    int iBridierNukedIndex;
    int iBridierNukedIndexChange;
};

#define ALMONASTER_NUKER_SCORE_CHANGE 0x00000001
#define ALMONASTER_NUKED_SCORE_CHANGE 0x00000002
#define BRIDIER_SCORE_CHANGE          0x00000004


class IScoringSystem
{
public:

    virtual bool HasTopList() = 0;

    virtual int OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges) = 0;
    virtual int OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges) = 0;

    virtual int On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges) = 0;
    virtual int On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges) = 0;

    virtual int OnGameEnd (int iGameClass, int iGameNumber) = 0;
    virtual int OnWin (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int OnDraw (int iGameClass, int iGameNumber, int iEmpireKey) = 0;
    virtual int OnRuin (int iGameClass, int iGameNumber, int iEmpireKey) = 0;

    virtual bool IsValidScore (const Variant* pvScore) = 0;
    virtual int CompareScores (const Variant* pvLeft, const Variant* pvRight) = 0;

    virtual int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore) = 0;
};

class IMapGenerator
{
public:

    // Implementor must do the following:
    //
    // 1) If iNumExistingPlanets == 0, fill in the following columns in the pvGameData row:
    //
    // GameData::NumPlanetsPerEmpire,
    // GameData::HWAg
    // GameData::AvgAg
    // GameData::HWMin
    // GameData::AvgMin
    // GameData::HWFuel
    // GameData::AvgFuel
    //
    // 2) Allocate *piNumNewPlanets new rows into *pppvNewPlanetData, each with GameMap::NumColumns
    // 3) Fill in the following columns for each new planet row:
    //
    // GameMap::Ag,
    // GameMap::Minerals
    // GameMap::Fuel
    // GameMap::Coordinates
    // GameMap::Link
    // GameMap::HomeWorld
    //
    // GameMap::Owner -> Set to empire's chain, even if not fully-colonized map
    //
    // Everything else is taken care of by the caller
    //
    // Sanity rules apply:
    // - Coordinates already in use on the map must not be used
    // - Links must actually have a planet behind them
    // - Exactly one homeworld per empire must be selected
    // - etc.

    virtual int CreatePlanets (
        
        int iGameClass,
        int iGameNumber,

        int* piNewEmpireKey,
        unsigned int iNumNewEmpires,

        Variant** ppvExistingPlanetData,
        unsigned int iNumExistingPlanets,

        Variant* pvGameClassData,
        Variant* pvGameData,
        
        Variant*** pppvNewPlanetData,
        unsigned int* piNumNewPlanets
        ) = 0;

    virtual void FreePlanetData(Variant** ppvNewPlanetData) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// GameEngine

extern __declspec(thread) IDatabaseConnection* t_pConn;
extern __declspec(thread) ICachedTableCollection* t_pCache;
extern __declspec(thread) Uuid t_uuidReq;

class GameEngine
{
private:
   
    int InitializeNewDatabase();
    int ReloadDatabase();

    int CreateDefaultSystemTables();
    int SetupDefaultSystemTables();
    int SetupDefaultThemes(unsigned int* piDefaultThemeKey);
    int SetupDefaultSystemGameClasses();

    int VerifySystem();
    int VerifyMarkedGameClasses();
    int VerifyActiveGames();

    int VerifyTournaments();

    bool VerifyTableExistence(const char* pszTable, bool bNewDatabase);
    bool VerifyTableExistenceWithRows(const char* pszTable, bool bNewDatabase);
    void VerifySystemTables(bool* pbNewDatabase, bool* pbGoodDatabase, const char** ppszBadTable);

    // Games
    int CleanupGame (int iGameClass, int iGameNumber, GameResult grResult, const char* pszWinnerName = NULL);
    int QuitEmpireFromGameInternal (int iGameClass, int iGameNumber, int iEmpireKey, int iKillerEmpire);

    int CheckGameForAllyOut (int iGameClass, int iGameNumber, bool* pbAlly);
    int CheckGameForDrawOut (int iGameClass, int iGameNumber, bool* pbDraw);

    int DeleteShipFromDeadEmpire (const char* pszEmpireShips, const char* pszGameMap, 
        unsigned int iShipKey, unsigned int iPlanetKey);

    int GetGames (bool bOpen, int** ppiGameClass, int** ppiGameNumber, unsigned int* piNumGames);

    int RuinEmpire (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage);

    int PauseGameAt (int iGameClass, int iGameNumber, const UTCTime& tNow);
    int PauseGameInternal (int iGameClass, int iGameNumber, const UTCTime& tNow, bool bAdmin, bool bBroadcast);

    // Empires
    int DeleteEmpireFromGame (int iGameClass, int iGameNumber, int iEmpireKey, ReasonForRemoval rReason,
        const GameUpdateInformation* pUpdateInfo);

    int RemoveEmpire (int iEmpireKey);
    int UpdateEmpireString (int iEmpireKey, const char* pszColumn, const char* pszString, size_t stMaxLen, bool* pbTruncated);

    int QueueDeleteEmpire (int iEmpireKey, int64 i64SecretKey);
    static int THREAD_CALL DeleteEmpireMsg (AsyncTask* pMessage);

    // Planets
    int AddEmpiresToMap (int iGameClass, int iGameNumber, int* piEmpireKey, int iNumEmpires, GameFairnessOption gfoFairness);

    int CreateMapFromMapGeneratorData(int iGameClass, int iGameNumber, int* piNewEmpireKey, 
        unsigned int iNumNewEmpires, Variant* pvGameClassData, Variant* pvGameData, 
        Variant** ppvNewPlanetData, unsigned int iNumNewPlanets,
        unsigned int* piExistingPlanetKey, Variant** ppvExistingPlanetData, unsigned int iNumExistingPlanets);

    int CreatePlanetFromMapGeneratorData (Variant* pvPlanetData,
        int iGameClass, int iGameNumber, int iEmpireKey, int iGameClassOptions,
        int* piMinX, int* piMaxX, int* piMinY, int* piMaxY, int* piNewPlanetKey);

    int InsertPlanetIntoGameEmpireData (int iGameClass, int iGameNumber, int iEmpireKey, 
        int iPlanetKey, const Variant* pvPlanetData, int iGameClassOptions);

    void AdvanceCoordinates (int iX, int iY, int* piX, int* piY, int cpDirection);

#ifdef _DEBUG

    typedef HashTable<unsigned int, unsigned int, GenericHashValue<unsigned int>, GenericEquals<unsigned int>> PlanetHashTable;

    int VerifyMap (int iGameClass, int iGameNumber);
    int DfsTraversePlanets (ICachedTable* pGameMap, unsigned int iPlanetKey, PlanetHashTable& htVisited, unsigned int iNumPlanets);

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

    int GetBridierScore (int iEmpireKey, int* piRank, int* piIndex);
    
    int TriggerBridierTimeBombIfNecessaryCallback();
    static int THREAD_CALL TriggerBridierTimeBombIfNecessaryMsg (AsyncTask* pMessage);

    int ScanEmpiresOnScoreChanges();

    // Options
    int CheckForDelayedPause (int iGameClass, int iGameNumber, const UTCTime& tNow, bool* pbNewlyPaused);

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

    int GateShips (int iGameClass, int iGameNumber, unsigned int iGaterEmpireIndex, const char* pszGaterEmpireName,
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

    int CheckForFirstContact(int iGameClass, int iGameNumber, int iEmpireKey, int iEmpireIndex,
        int iPlanetKey, const char* pszPlanetName,
        int iNewX, int iNewY, unsigned int iNumEmpires, const unsigned int* piEmpireKey, const Variant* pvEmpireName, 
        const char* strEmpireDip, const char* strGameMap, const char** pstrEmpireDip, String* pstrUpdateMessage);

    // Updates
    void GetNextUpdateTime (const UTCTime& tLastUpdate, Seconds sUpdatePeriod, int iNumUpdates,
        Seconds sFirstUpdateDelay, Seconds sAfterWeekendDelay, bool bWeekends, UTCTime* ptNextUpdateTime);

    void AdvanceWeekendTime (const UTCTime& tNextUpdateTime, Seconds sAfterWeekendDelay, UTCTime* ptNextUpdateTime);

    void GetLastUpdateTimeForPausedGame (const UTCTime& tNow, Seconds sSecondsUntilNextUpdate,
        Seconds sUpdatePeriod, int iNumUpdates, Seconds sFirstUpdateDelay, UTCTime* ptLastUpdateTime);

    // Planets
    int SetNewMinMaxIfNecessary (int iGameClass, int iGameNumber, int iEmpireKey, int iX, int iY);

    // Database ops
    int PurgeDatabasePrivate (int iEmpireKey, int iCriteria);
    static int THREAD_CALL PurgeDatabaseMsg (AsyncTask* pMessage);

    // Diplomacy
    int AddDiplomaticOption(int iGameClass, int iGameNumber, int iEmpireKey, int iDiplomacyProxyKey,
                            int iDiplomacyLevel, int iCurrentDiplomacyLevel, int piDipOptKey[], int* piNumOptions);

    int BuildDuplicateList (int* piDuplicateKeys, unsigned int iNumDuplicates, String* pstrDuplicateList);

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

    int GetNumUnreadSystemMessagesPrivate (ICachedTable* pMessages, unsigned int* piNumber);
    int GetNumUnreadGameMessagesPrivate (ICachedTable* pMessages, unsigned int* piNumber);

    int GetMessageProperty (const char* strMessages, unsigned int iMessageKey, const char* pszColumn, Variant* pvProperty);

    int DeleteOverflowMessages (ICachedTable* pMessages, const char* pszTimeStampColumn, const char* pszUnreadColumn, 
        unsigned int iNumMessages, unsigned int iNumUnreadMessages, unsigned int iMaxNumMessages, bool bCheckUnread);

    // Empires
    int WriteNextStatistics (int iGameClass, int iGameNumber, int iEmpireKey, int iTotalAg, int iBonusAg, 
        float fMaxAgRatio);

    int UpdateGameEmpireString (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszColumn, 
        const char* pszString, size_t stMaxLen, bool* pbTruncated);

    // Tournaments
    int SetTournamentString (unsigned int iTournamentKey, const char* pszColumn, const char* pszString);

    int HandleEmpireTournamentAddition (int iEmpireKey, int iMessageKey, int iMessageType, bool bAccept);
    int AddEmpireToTournament (unsigned int iTournamentKey, int iInviteKey);

    // Associations
    int RemoveDeadEmpireAssociations(unsigned int iEmpireKey);

public:

    // Setup
    int Setup();
    int WriteAvailability();

    int FlushDatabase (int iEmpireKey);
    int BackupDatabase (int iEmpireKey);
    int PurgeDatabase (int iEmpireKey, int iCriteria);

    int RestoreDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion);
    int DeleteDatabaseBackup (int iEmpireKey, int iDay, int iMonth, int iYear, int iVersion);

    const char* GetSystemVersion();

    int GetNewSessionId (int64* pi64SessionId);

    int GetGameConfiguration (GameConfiguration* pgcConfig);
    int GetMapConfiguration (MapConfiguration* pmcConfig);

    int SetGameConfiguration (const GameConfiguration& gcConfig);
    int SetMapConfiguration (const MapConfiguration& mcConfig);

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
    unsigned int GetColonyPopulationBuildCost (int iShipBehavior, float fColonyMultipliedBuildFactor, int iColonySimpleBuildFactor, float fBR);
    
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
    int CheckTargetPop (int iGameClass, int iGameNumber, int iEmpireKey);
#endif

    int GetMaxNumDiplomacyPartners(int iGameClass, int iGameNumber, int iDiplomacyLevel, unsigned int* piMaxNumPartners);
    int GetCumulativeDiplomacyCountForLimits(int iGameClass, int iGameNumber, int iEmpireKey, int iDiplomacyLevel, unsigned int* piCount);
    int GetDiplomacyCountsForLimits(int iGameClass, int iGameNumber, int iEmpireKey,
                                    unsigned int* piNumTruces, unsigned int* piNumTrades, unsigned int* piNumAlliances, unsigned int* piNumFormerAlliances);

    void CalculateTradeBonuses (int iNumTrades, int iTotalAg, int iTotalMin, int iTotalFuel,
        int iPercentFirstTradeIncrease, int iPercentNextTradeIncrease, 
        int* piBonusAg, int* piBonusMin, int* iBonusFuel);

    unsigned int GetNumFairDiplomaticPartners (unsigned int iMaxNumEmpires);

    bool GameAllowsDiplomacy (int iDiplomacyLevel, int iDip);
    bool IsLegalDiplomacyLevel (int iDiplomacyLevel);
    bool IsLegalPrivilege (int iPrivilege);

    int GetNextDiplomaticStatus (int iOffer1, int iOffer2, int iCurrentStatus);

    // Update
    int ForceUpdate (int iGameClass, int iGameNumber);

    int SetEmpireReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet);
    int SetEmpireNotReadyForUpdate (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbSet);

    // Themes
    int DoesThemeExist (int iThemeKey, bool* pbExist);
    int CreateTheme (Variant* pvData, unsigned int* piKey);
    int GetThemeKeys(unsigned int** ppiThemeKey, unsigned int* piNumKeys);
    int GetFullThemeKeys (unsigned int** ppiThemeKey, unsigned int* piNumKeys);
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
    int GetSuperClassKeys(unsigned int** ppiKey, unsigned int* piNumSuperClasses);
    int GetSuperClassKeys(unsigned int** ppiKey, Variant** ppvName, unsigned int* piNumSuperClasses);
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
    int GetStartableSystemGameClassKeys(unsigned int** ppiKey, unsigned int* piNumKeys);

    int GetGameClassSuperClassKey(int iGameClass, unsigned int* piSuperClassKey);
    int SetGameClassSuperClassKey(int iGameClass, unsigned int iSuperClassKey);

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

    int GetGameClassProperty (int iGameClass, const char* pszColumn, Variant* pvProperty);

    // Games
    int GetGameUpdateData (int iGameClass, int iGameNumber, int* piSecondsSince, int* piSecondsUntil, 
        int* piNumUpdates, int* piGameState, Vector<UTCTime>* pvecUpdateTimes = NULL);

    int ResetAllGamesUpdateTime();
    int ResetGameUpdateTime (int iGameClass, int iGameNumber);

    int GetGameState (int iGameClass, int iGameNumber, int* piGameState);
    int GetGameCreationTime (int iGameClass, int iGameNumber, UTCTime* ptCreationTime);

    int CheckGameForEndConditions (int iGameClass, int iGameNumber, const char* pszAdminMessage, bool* pbEndGame);
    int DeleteGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszMessage, int iReason);
    
    int GetNumActiveGames(unsigned int* piNumGames);
    int GetNumOpenGames(unsigned int* piNumGames);
    int GetNumClosedGames(unsigned int* piNumGames);

    int AreAllEmpiresIdle (int iGameClass, int iGameNumber, bool* pbIdle);

    int GetActiveGames(int** ppiGameClass, int** ppiGameNumber, unsigned int* piNumGames);
    int GetOpenGames(int** ppiGameClass, int** ppiGameNumber, unsigned int* piNumGames);
    int GetClosedGames(int** ppiGameClass, int** ppiGameNumber, unsigned int* piNumGames);

    int IsGameOpen (int iGameClass, int iGameNumber, bool* pbOpen);
    int HasGameStarted (int iGameClass, int iGameNumber, bool* pbStarted);

    int IsGamePasswordProtected (int iGameClass, int iGameNumber, bool* pbProtected);
    int SetGamePassword (int iGameClass, int iGameNumber, const char* pszNewPassword);

    int GetGameProperty(int iGameClass, int iGameNumber, const char* pszColumn, Variant* pvProp);
    int SetGameProperty(int iGameClass, int iGameNumber, const char* pszColumn, const Variant& vProp);

    int CreateGame (int iGameClass, int iEmpireCreator, const GameOptions& goGameOptions, int* piGameNumber);
    int EnterGame (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszPassword,
        const GameOptions* pgoGameOptions, int* piNumUpdates, bool bSendMessages, bool bCreatingGame, bool bCheckSecurity);

    int SetEnterGameIPAddress (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszIPAddress);

    int DoesGameExist (int iGameClass, int iGameNumber, bool* pbExists);
    int GetNumUpdates (int iGameClass, int iGameNumber, int* piNumUpdates);
    
    int GetNumUpdatesBeforeGameCloses (int iGameClass, int iGameNumber, int* piNumUpdates);
    int GetGameOptions (int iGameClass, int iGameNumber, int* piOptions);
    int GetFirstUpdateDelay (int iGameClass, int iGameNumber, Seconds* psDelay);
    
    int GetNumEmpiresInGame (int iGameClass, int iGameNumber, unsigned int* piEmpires);
    int GetNumDeadEmpiresInGame (int iGameClass, int iGameNumber, unsigned int* piDeadEmpires);
    int GetNumEmpiresNeededForGame (int iGameClass, int* piNumEmpiresNeeded);
    int IsEmpireInGame (int iGameClass, int iGameNumber, int iEmpireKey, bool* pbInGame);
    int GetNumUpdatedEmpires (int iGameClass, int iGameNumber, int* piUpdatedEmpires);
    
    int GetEmpiresInGame (int iGameClass, int iGameNumber, Variant** ppiEmpireKey, unsigned int* piNumEmpires);
    int GetEmpiresInGame (int iGameClass, int iGameNumber, Variant*** pppvEmpiresInGame, unsigned int* piNumEmpires);

    int IsGamePaused (int iGameClass, int iGameNumber, bool* pbPaused);
    int IsGameAdminPaused (int iGameClass, int iGameNumber, bool* pbAdminPaused);
    int IsSpectatorGame (int iGameClass, int iGameNumber, bool* pbSpectatorGame);

    int AddToLatestGames(const Variant* pvColumns);

    void GetGameClassGameNumber (const char* pszGameData, int* piGameClass, int* piGameNumber);
    void GetGameClassGameNumber (int iGameClass, int iGameNumber, char* pszGameData);

    int PauseAllGames();
    int UnpauseAllGames();

    int PauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast);
    int UnpauseGame (int iGameClass, int iGameNumber, bool bAdmin, bool bBroadcast);

    int LogEmpireIntoGame (int iGameClass, int iGameNumber, int iEmpireKey, int* piIdleUpdates);

    int RuinGame (int iGameClass, int iGameNumber, const char* pszWinnerName);
    int ResignGame (int iGameClass, int iGameNumber);

    int GetResignedEmpiresInGame (int iGameClass, int iGameNumber, unsigned int** ppiEmpireKey, unsigned int* piNumResigned);
    int UnresignEmpire (int iGameClass, int iGameNumber, int iEmpireKey, int iAdminKey);

    int GetGameEntryRestrictions (int iGameClass, int iGameNumber, int* piOptions, 
        Variant pvRestrictionMin [NUM_ENTRY_SCORE_RESTRICTIONS], 
        Variant pvRestrictionMax [NUM_ENTRY_SCORE_RESTRICTIONS]);

    int GameAccessCheck(int iGameClass, int iGameNumber, int iEmpireKey, 
        const GameOptions* pgoGameOptions, GameAction gaAction, 
        bool* pbAccess, GameAccessDeniedReason* prAccessDeniedReason);

    int GetBridierRankPotentialGainLoss (int iGameClass, int iGameNumber, int iEmpireKey, int* piGain, int* piLoss);

    int GetNumUniqueEmpiresInGames (unsigned int* piNumEmpires);
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
    
    int SetEmpirePassword(unsigned int iEmpireKey, const char* pszPassword);
    int ChangeEmpirePassword(unsigned int iEmpireKey, const char* pszPassword);

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

    int CacheEmpire(unsigned int iEmpireKey);
    int CacheEmpire(unsigned int iEmpireKey, unsigned int* piResults);
    int CacheEmpires(const Variant* pvEmpireKey, unsigned int iNumEmpires);
    int CacheEmpires(const unsigned int* piEmpireKey, unsigned int iNumEmpires);
    int CacheEmpires(const unsigned int* piEmpireKey, unsigned int iNumEmpires, unsigned int* piResults);
    int CacheEmpireAndMessages(unsigned int iEmpireKey);
    int CacheEmpireMessagesAndTournaments(unsigned int iEmpireKey);
    int CacheEmpireForDeletion(unsigned int iEmpireKey);
    int CacheTournamentTables(const unsigned int* piTournamentKey, unsigned int iNumTournaments);
    int CacheTournamentAndEmpireTables(unsigned int iTournamentKey);
    int CacheTournamentEmpireTables(unsigned int iTournamentKey);
    int CacheTournamentEmpiresForGame(unsigned int iTournamentKey);
    int CacheGameData(int* piGameClass, int* piGameNumber, int iEmpireKey, unsigned int iNumGames);
    int CacheGameEmpireData(unsigned int iEmpireKey, const Variant* pvGame, unsigned int iNumGames);
    int CacheEmpireAndActiveGames(const unsigned int* piEmpireKey, unsigned int iNumEmpires);
    int CacheEmpiresAndGameMessages(int iGameClass, int iGameNumber, const unsigned int* piEmpireKey, unsigned int iNumEmpires);
    int CacheEmpireActiveGamesMessagesNukeLists(const unsigned int* piEmpireKey, unsigned int iNumEmpires);
    int CacheAllGameTables(int iGameClass, int iGameNumber);
    int CacheGameTablesForBroadcast(int iGameClass, int iGameNumber);
    int CacheProfileData(unsigned int iEmpireKey);
    int CacheNukeHistory(unsigned int iEmpireKey);
    int CacheMutualAssociations(unsigned int iEmpireKey, unsigned int iSecondEmpireKey);
    int CacheForReload();
    int CacheForCheckAllGamesForUpdates();
    int CacheSystemAvailability();
    int CacheSystemAlienIcons();
    int CacheGameEmpireTables(const Variant* pvGameClassGameNumber, unsigned int iNumGames);

    enum GameCacheEntryFlags
    {
        EMPTY_GAME_EMPIRE_DIPLOMACY      = 0x00000001,
        EMPTY_GAME_EMPIRE_MAP            = 0x00000002,
        EMPTY_GAME_EMPIRE_SHIPS          = 0x00000004,
        EMPTY_GAME_EMPIRE_MESSAGES       = 0x00000008,
        EMPTY_GAME_EMPIRE_FLEETS         = 0x00000010,
        EMPTY_SYSTEM_EMPIRE_MESSAGES     = 0x00000020,
        EMPTY_SYSTEM_EMPIRE_ACTIVE_GAMES = 0x00000040,
    };

    int CreateEmptyGameCacheEntries(int iGameClass, int iGameNumber, int iEmpireKey, int iTournamentKey, int iDiplomacyKey, int eFlags);

    int LookupEmpireByName(const char* pszName, unsigned int* piEmpireKey, Variant* pvName, int64* pi64SecretKey);
    int LookupEmpireByName(const char* pszName, unsigned int* piEmpireKey, Variant* pvName, int64* pi64SecretKey, ICachedTable** ppTable);

    int DoesEmpireExist (unsigned int iEmpireKey, bool* pbExists, Variant* pvEmpireName);

    int CheckSecretKey (unsigned int iEmpireKey, int64 i64SecretKey, bool* pbMatch, int64* pi64SessionId, Variant* pvIPAddress);

    int IsPasswordCorrect (int iEmpireKey, const char* pszPassword);

    int LoginEmpire (int iEmpireKey, const char* pszBrowser, const char* pszIPAddress);
    
    int GetNumEmpiresOnServer(unsigned int* piNumEmpires);
    int GetDefaultEmpireShipNames (int iEmpireKey, const char*** pppszShipName);

    int UndeleteEmpire (int iEmpireKey);
    int BlankEmpireStatistics(unsigned int iEmpireKey);

    int GetEmpirePersonalGameClasses (int iEmpireKey, unsigned int** ppiGameClassKey, Variant** ppvName, unsigned int* piNumKeys);

    int GetEmpireData (int iEmpireKey, Variant** ppvEmpData, unsigned int* piNumActiveGames);
    int GetEmpireActiveGames(int iEmpireKey, int** ppiGameClass, int** ppiGameNumber, unsigned int* piNumGames);

    int GetEmpirePrivilege(unsigned int iEmpireKey, int* piPrivilege);
    int SetEmpirePrivilege(unsigned int iEmpireKey, int iPrivilege);

    int GetEmpireAlmonasterScore(unsigned int iEmpireKey, float* pfAlmonasterScore);
    int SetEmpireAlmonasterScore(unsigned int iEmpireKey, float fAlmonasterScore);

    int GetEmpirePassword(unsigned int iEmpireKey, Variant* pvPassword);

    int GetEmpireDataColumn (int iEmpireKey, const char* pszColumn, Variant* pvData);

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

    int GetEmpireProperty (int iEmpireKey, const char* pszColumn, Variant* pvProperty);
    int SetEmpireProperty (int iEmpireKey, const char* pszColumn, const Variant& vProperty);

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

    int GetEmpireGameProperty (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszColumn, Variant* pvProperty);
    int SetEmpireGameProperty (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszColumn, const Variant& vProperty);

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

    int GetSystemMessageProperty (int iEmpireKey, unsigned int iMessageKey, const char* pszColumn, Variant* pvProperty);

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
    int BroadcastGameMessage (int iGameClass, int iGameNumber, const char* pszMessage, int iSourceKey, int iFlags);

    int GetGameMessageProperty (int iGameClass, int iGameNumber, int iEmpireKey, unsigned int iMessageKey, const char* pszColumn, Variant* pvProperty);

    // Planets
    int GetPlanetCoordinates (int iGameClass, int iGameNumber, int iPlanetKey, int* piPlanetX, int* piPlanetY);
    int GetPlanetProperty (int iGameClass, int iGameNumber, unsigned int iPlanetKey, const char* pszProperty, Variant* pvProperty);
    int GetPlanetKeyFromCoordinates (int iGameClass, int iGameNumber, int iX, int iY, int* piPlanetKey);
    int GetEmpirePlanetIcons (int iEmpireKey, unsigned int* piLivePlanetKey, unsigned int* piLiveDeadPlanetKey);

    int GetNeighbourPlanetKey (int iGameClass, int iGameNumber, int iPlanetKey, int iDirection, int* piPlanetKey);
    int HasEmpireExploredPlanet (int iGameClass, int iGameNumber, int iEmpireKey, int iPlanetKey, bool* pbExplored);

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

    IScoringSystem* CreateScoringSystem(ScoringSystem ssTopList);

    int CalculatePrivilegeLevel (int iEmpireKey);

    int AddNukeToHistory (NukeList nlNukeList, const char* pszGameClassName, int iGameNumber, 
        int iEmpireKey, const char* pszEmpireName, int iAlienKey,
        int iOtherEmpireKey, const char* pszOtherEmpireName, int iOtherAlienKey);

    // System Config
    int GetDefaultUIKeys (unsigned int* piBackground, unsigned int* piLivePlanet, 
        unsigned int* piDeadPlanet, unsigned int* piButtons, unsigned int* piSeparator, 
        unsigned int* piHorz, unsigned int* piVert, unsigned int* piColor);

    int SetScoreForPrivilege (Privilege privLevel, float fScore);
    int GetScoreForPrivilege (Privilege privLevel, float* pfScore);

    int GetDefaultGameOptions (int iGameClass, GameOptions* pgoOptions);

    int GetSystemOptions (int* piOptions);
    int SetSystemOption (int iOption, bool bFlag);

    int GetSystemProperty(const char* pszColumn, Variant* pvProperty);
    int SetSystemProperty(const char* pszColumn, const Variant& vProperty);

    int GetDefaultShipName (int iTech, Variant* pvShipName);
    int SetDefaultShipName (int iShipKey, const char* pszShipName);

    // Aliens
    int GetNumAliens (unsigned int* piNumAliens);
    int GetAlienKeys(Variant*** pppvData, unsigned int* piNumAliens);
    int CreateAlienIcon (int iAlienKey, const char* pszAuthorName);
    int DeleteAlienIcon (int iAlienKey);

    int GetAlienAuthorName (int iAlienKey, Variant* pvAuthorName);

    int SetEmpireAlienKey (int iEmpireKey, int iAlienKey);

    // Top Lists
    int GetTopList(ScoringSystem ssListType, unsigned int** ppiEmpireKey, unsigned int* piNumEmpires);

    // Search
    int PerformMultipleSearch(const RangeSearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);

    // Updates
    int CheckGameForUpdates(int iGameClass, int iGameNumber, bool* pbUpdate);
    int CheckAllGamesForUpdates();

    static int THREAD_CALL CheckAllGamesForUpdatesMsg (AsyncTask* pMessage);

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

    int SearchForDuplicates (int iGameClass, int iGameNumber, const char* pszSystemEmpireDataColumn,
        const char* pszGameEmpireDataColumn, int** ppiDuplicateKeys, unsigned int** ppiNumDuplicatesInList, 
        unsigned int* piNumDuplicates);

    int DoesEmpireHaveDuplicates (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszSystemEmpireDataColumn,
        int** ppiDuplicateKeys, unsigned int* piNumDuplicates);

    int GetEmpireDefaultMessageTarget(int iGameClass, int iGameNumber, int iEmpireKey, int* piMessageTarget);
    int SetEmpireDefaultMessageTarget(int iGameClass, int iGameNumber, int iEmpireKey, int iMessageTarget);

    // Diplomacy
    int GetKnownEmpireKeys (int iGameClass, int iGameNumber, int iEmpireKey, Variant** ppvEmpireKey, int* piNumEmpires);
    int GetVisibleDiplomaticStatus (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, int* piWeOffer, int* piTheyOffer, int* piCurrent, bool* pbMet);
    int GetDiplomaticOptions (int iGameClass, int iGameNumber, int iEmpireKey, int iFoeKey, int piDipOptKey[], int* piSelected, int* piNumOptions);
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
        unsigned int** ppiLastUsedProxyKeyArray, unsigned int* piNumLastUsed);

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

    int BuildNewShips (int iGameClass, int iGameNumber, int iEmpireKey, int iTechKey, unsigned int iNumShips, 
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

    static void FreeShipOrders (ShipOrder* psoOrders, unsigned int iNumOrders);

    // Fleets
    int GetEmpireFleetKeys (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiFleetKeys, 
        int* piNumFleets);

    int GetNumFleets (int iGameClass, int iGameNumber, int iEmpireKey, int* piNumFleets);
    int GetFleetProperty (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, const char* pszProperty, Variant* pvProperty);
    int GetFleetFlag (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int iFlag, bool* pbFlag);
    int SetFleetFlag (int iGameClass, int iGameNumber, int iEmpireKey, int iFleetKey, int iFlag, bool bFlag);

    int GetNewFleetLocations (int iGameClass, int iGameNumber, int iEmpireKey, int** ppiLocationKey, 
        int* piNumLocations);

    int CreateNewFleet (int iGameClass, int iGameNumber, int iEmpireKey, const char* pszFleetName, 
        int iPlanetKey, unsigned int* piFleetKey);

    int GetFleetOrders (unsigned int iGameClass, int iGameNumber, unsigned int iEmpireKey, unsigned int iFleetKey, 
        const GameConfiguration& gcConfig, FleetOrder** ppfoOrders, unsigned int* piNumOrders, 
        unsigned int* piSelected);

    static void FreeFleetOrders (FleetOrder* pfoOrders, unsigned int iNumOrders);

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
    int GetJoinedTournaments (int iEmpireKey, Variant** ppvTournamentKey, Variant** ppvName, unsigned int* piNumTournaments);

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

    int GetTournamentEmpires(unsigned int iTournamentKey, Variant** ppvEmpireKey, Variant** ppvTeamKey, Variant** ppvName, unsigned int* piNumKeys);
    int GetAvailableTournamentEmpires (unsigned int iTournamentKey, Variant** ppvEmpireKey, Variant** ppvTeamKey, Variant** ppvName, unsigned int* piNumKeys);

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
    int GetAssociations (unsigned int iEmpireKey, Variant** ppvAssoc, unsigned int* piNumAssoc);
    int CheckAssociation (unsigned int iEmpireKey, unsigned int iSwitch, bool* pbAuth);
    int CreateAssociation (unsigned int iEmpireKey, const char* pszSecondEmpire, const char* pszPassword);
    int DeleteAssociation (unsigned int iEmpireKey, unsigned int iSecondEmpireKey);
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

class AutoFreeFleetOrders
{
private:
    FleetOrder*& m_pfoOrders;
    unsigned int& m_iMaxNumOrders;

public:
    AutoFreeFleetOrders(FleetOrder*& pfoOrders, unsigned int& iMaxNumOrders) : m_pfoOrders(pfoOrders), m_iMaxNumOrders(iMaxNumOrders)
    {
    }

    ~AutoFreeFleetOrders()
    {
        if (m_pfoOrders)
        {
            GameEngine::FreeFleetOrders(m_pfoOrders, m_iMaxNumOrders);
        }
    }
};

class AutoFreeShipOrders
{
private:
    ShipOrder*& m_pfoOrders;
    unsigned int& m_iMaxNumOrders;

public:
    AutoFreeShipOrders(ShipOrder*& pfoOrders, unsigned int& iMaxNumOrders) : m_pfoOrders(pfoOrders), m_iMaxNumOrders(iMaxNumOrders)
    {
    }

    ~AutoFreeShipOrders()
    {
        if (m_pfoOrders)
        {
            GameEngine::FreeShipOrders(m_pfoOrders, m_iMaxNumOrders);
        }
    }
};