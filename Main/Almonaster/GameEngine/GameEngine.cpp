//
// GameEngine.dll:  a component of Almonaster
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

#include "GameEngine.h"

#include "../Scoring/AlmonasterScore.h"
#include "../Scoring/ClassicScore.h"
#include "../Scoring/BridierScore.h"
#include "../Scoring/TournamentScoring.h"


GameEngine::GameEngine(const char* pszDatabaseFile,
                       const Uuid& uuidDatabaseClsid,
                       const char* pszDatabaseConnectionString,

                       const char* pszHookLibrary,
                        
                       IAlmonasterUIEventSink* pUIEventSink, 
                        
                       IReport* pReport, 
                       IPageSourceControl* pPageSourceControl,
                        
                       const SystemConfiguration& scConfig,
                       const ChatroomConfig& ccConfig
                        
                       ) :

                       m_htGameObjectTable (NULL, NULL),
                        
                       m_pUIEventSink (pUIEventSink),  

                       m_pReport (pReport),    // Weak ref
                       m_pPageSourceControl (pPageSourceControl)   // Weak ref

                       {   
    
    m_iNumRefs = 1;
    m_bGoodDatabase = false;

    m_iRootKey = NO_KEY;
    m_iGuestKey = NO_KEY;

    m_scConfig = scConfig;
    m_ccConfig = ccConfig;

    m_pszDatabaseFile = String::StrDup(pszDatabaseFile);
    m_uuidDatabaseClsid = uuidDatabaseClsid;
    m_pszDatabaseConnectionString = String::StrDup(pszDatabaseConnectionString);

    m_pAlmonasterHook = NULL;

    typedef IAlmonasterHook* (*Fxn_AlmonasterHookCreateInstance)();
    Fxn_AlmonasterHookCreateInstance pCreateInstance;

    // Best effort
    if (pszHookLibrary != NULL && 
        m_libHook.Open (pszHookLibrary) == OK &&
        (pCreateInstance = (Fxn_AlmonasterHookCreateInstance) 
            m_libHook.GetExport ("AlmonasterHookCreateInstance")) != NULL &&
        (m_pAlmonasterHook = pCreateInstance()) != NULL
        ) {

        char* pszString = (char*) StackAlloc (String::StrLen (pszHookLibrary) + 256);
        sprintf (pszString, "GameEngine found hook library %s", pszHookLibrary);

        m_pReport->WriteReport (pszString);
    }

    m_pConn = NULL;
    m_pChatroom = NULL;

    m_dbsStage = DATABASE_BACKUP_NONE;
    m_iMaxNumTemplates = 0;
    m_iMaxNumTables = 0;
    m_bActiveBackup = false;
    m_tBackupStartTime = NULL_TIME;

    m_pUIEventSink->AddRef();

    unsigned int i;
    ENUMERATE_SCORING_SYSTEMS(i) {
        m_ppScoringSystem[i] = NULL;
    }
}


GameEngine::~GameEngine() {

    // Check all games for updates
    int iErrCode;
    
    if (m_bGoodDatabase) {
        iErrCode = CheckAllGamesForUpdates (true);
        Assert (iErrCode == OK);
    }

    // Clean up chatroom
    if (m_pChatroom != NULL) {
        delete m_pChatroom;
        m_pChatroom = NULL;
    }

    // Finalize
    if (m_pAlmonasterHook != NULL) {
        m_pAlmonasterHook->Finalize();
        m_pAlmonasterHook->Release();
    }

    if (m_libHook.IsOpen()) {
        m_libHook.Close();
    }

    // Stop AutoBackup
    if (m_scConfig.bAutoBackup && m_bGoodDatabase) {
        m_pReport->WriteReport ("GameEngine shutting down auto-backup thread");
        m_eAutoBackupEvent.Signal();
        m_tAutoBackupThread.WaitForTermination();
        m_pReport->WriteReport ("GameEngine shut down auto-backup thread");
    }

    if (m_pszDatabaseFile != NULL) {
        OS::HeapFree (m_pszDatabaseFile);
    }

    if (m_pszDatabaseConnectionString != NULL) {
        OS::HeapFree (m_pszDatabaseConnectionString);
    }

    // Stop long running query processor
    if (m_tsfqQueryQueue.IsInitialized() && m_tsfqQueryQueue.Push (NULL)) {

        m_eQueryEvent.Signal();
        m_tLongRunningQueries.WaitForTermination();
    }

    // Stop lock manager
    m_lockMgr.Shutdown();

    // Clean up game object table
    GameObject* pGameObject;
    HashTableIterator<const char*, GameObject*> htiGameObject;

    m_rwGameObjectTableLock.WaitWriter();
    if (m_htGameObjectTable.GetNextIterator (&htiGameObject)) {
        while (m_htGameObjectTable.Delete (&htiGameObject, NULL, &pGameObject)) {
            pGameObject->Release();
        }
    }
    m_rwGameObjectTableLock.SignalWriter();


    // Close down database
    if (m_bGoodDatabase) {

        Assert (m_pGameData != NULL);

        m_pReport->WriteReport ("GameEngine writing last shutdown time to database");
        
        UTCTime tTime;
        Time::GetTime (&tTime);
        iErrCode = m_pConn->WriteData (SYSTEM_DATA, SystemData::LastShutdownTime, tTime);
        Assert (iErrCode == OK);

        m_pReport->WriteReport ("GameEngine wrote last shutdown time to database");
    }

    SafeRelease(m_pConn)

    if (m_pGameData != NULL) {

        if (m_scConfig.bCheckDatabase) {

            iErrCode = m_pGameData->Check();
            if (iErrCode == OK) {
                m_pReport->WriteReport ("GameEngine checked database successfully");
            } else {
                m_pReport->WriteReport ("GameEngine checked the database and data corruption was detected");
                Assert (false);
            }
        }

        m_pReport->WriteReport ("GameEngine shutting down database");
        m_pGameData->Release();
        m_pReport->WriteReport ("GameEngine shut down database");
    }

    // Release UI event sink
    m_pUIEventSink->Release();

    // Release scoring systems
    unsigned int i;
    ENUMERATE_SCORING_SYSTEMS(i) {
        if (m_ppScoringSystem[i] != NULL) {
            m_ppScoringSystem[i]->Release();
            m_ppScoringSystem[i] = NULL;
        }
    }

    m_pReport->WriteReport ("GameEngine terminated");
}

int GameEngine::Initialize()
{
    int iOptions = 0, i, iErrCode;

    // Lock Manager
    LockManagerConfig lmConf;
    lmConf.iNumEmpiresHint = 1000;          // Guess
    lmConf.iMaxAgedOutPerScan = 25;         // Heuristic
    lmConf.msMaxScanTime = 1000;            // One second
    lmConf.msScanPeriod = 1000 * 60 * 15;   // Fifteen minutes
    lmConf.sAgeOutPeriod = 60 * 10;         // Ten minutes

    iErrCode = m_lockMgr.Initialize (lmConf);
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine could not initialize the game empire lock manager");
        Assert (false);
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine initialized the game empire lock manager");

    iErrCode = m_mGameClasses.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }
    iErrCode = m_mSuperClasses.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }
    iErrCode = m_mAlienIcons.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }
    iErrCode = m_rwGameObjectTableLock.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }
    iErrCode = m_rwGameConfigLock.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }
    iErrCode = m_rwMapConfigLock.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }
    iErrCode = m_mConfigLock.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }
    iErrCode = m_eQueryEvent.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }
    iErrCode = m_eAutoBackupEvent.Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return iErrCode;
    }

    if (!m_tsfqQueryQueue.Initialize()) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return ERROR_OUT_OF_MEMORY;
    }

    if (m_pAlmonasterHook != NULL) {
        m_pAlmonasterHook->Initialize (this);
    }

    // Create the database
    m_pReport->WriteReport ("GameEngine is creating the database");

    iErrCode = m_libDatabase.Open(m_pszDatabaseFile);
    if (iErrCode == OK)
    {
        typedef int (*Fxn_CreateInstance) (const Uuid&, const Uuid&, void**);
        Fxn_CreateInstance pCreateInstance = (Fxn_CreateInstance)m_libDatabase.GetExport("CreateInstance");
        if (pCreateInstance == NULL)
        {
            m_pReport->WriteReport("Unable to obtain CreateInstance export from database library");
            return ERROR_FAILURE;
        }
        iErrCode = pCreateInstance(m_uuidDatabaseClsid, IID_IDatabase, (void**)&m_pGameData);
    }

    if (iErrCode == OK) {
        m_pReport->WriteReport ("GameEngine created the database");
    } else {

        Assert (false);

        char pszString [256];
        sprintf (pszString, "Error: GameEngine could not create the database;  the error was %i", iErrCode);

        m_pReport->WriteReport (pszString);

        return iErrCode;
    }

    // Initialize the database
    m_pReport->WriteReport ("GameEngine initializing the database");

    if (m_scConfig.bCheckDatabase) {
        iOptions |= DATABASE_CHECK;
    }

    if (m_scConfig.bDatabaseWriteThrough) {
        iOptions |= DATABASE_WRITETHROUGH;
    }

    iErrCode = m_pGameData->Initialize(m_pszDatabaseConnectionString, iOptions);
    switch (iErrCode) {

    case OK:
        m_pReport->WriteReport ("GameEngine initialized the database sucessfully");
        break;

    case WARNING:
        m_pReport->WriteReport ("GameEngine created a new database");
        break;

    default:

        char pszString [256];
        sprintf (
            pszString, 
            "Error %i occurred while initializing the database: make sure the database is being "\
            "set to use a valid directory and that there is enough space on the disk",
            iErrCode
            );

        m_pReport->WriteReport (pszString);
        return iErrCode;
    }

    m_pConn = m_pGameData->CreateConnection();
    if (m_pConn == NULL)
    {
        m_pReport->WriteReport("Unable to create database connection");
        return ERROR_FAILURE;
    }

    if (!m_htGameObjectTable.Initialize(250)) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return ERROR_OUT_OF_MEMORY;
    }

    // Init scoring systems
    m_ppScoringSystem[ALMONASTER_SCORE] = AlmonasterScore::CreateInstance (this);
    m_ppScoringSystem[CLASSIC_SCORE] = ClassicScore::CreateInstance (this);
    m_ppScoringSystem[BRIDIER_SCORE] = BridierScore::CreateInstance (this);
    m_ppScoringSystem[BRIDIER_SCORE_ESTABLISHED] = BridierScoreEstablished::CreateInstance (this);
    m_ppScoringSystem[TOURNAMENT_SCORING] = TournamentScoring::CreateInstance (this);

    ENUMERATE_SCORING_SYSTEMS (i) {

        if (m_ppScoringSystem[i] == NULL) {
            m_pReport->WriteReport ("GameEngine is out of memory");
            return ERROR_OUT_OF_MEMORY;
        }
    }

    // Hook library
    if (iErrCode == OK && m_pAlmonasterHook != NULL) {
        m_pAlmonasterHook->Setup();
    }

    // Setup the system
    m_pReport->WriteReport ("GameEngine is setting up the system");

    iErrCode = Setup();
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pConn->GetFirstKey(SYSTEM_EMPIRE_DATA, SystemEmpireData::Name, ROOT_NAME, &m_iRootKey);
    if (iErrCode != OK)
    {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pConn->GetFirstKey(SYSTEM_EMPIRE_DATA, SystemEmpireData::Name, GUEST_NAME, &m_iGuestKey);
    if (iErrCode != OK)
    {
        Assert (false);
        return iErrCode;
    }

    // Setup the system
    m_pReport->WriteReport ("GameEngine is initializing the chatroom");

    // Chatroom
    m_pChatroom = new Chatroom(m_ccConfig, m_pGameData);
    if (m_pChatroom == NULL) {
        m_pReport->WriteReport ("GameEngine is out of memory");
        return ERROR_OUT_OF_MEMORY;
    }

    iErrCode = m_pChatroom->Initialize();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("Chatroom init failed");
        return iErrCode;
    }

    // Long running queries
    iErrCode = m_tLongRunningQueries.Start (LongRunningQueryProcessor, this, Thread::LowerPriority);
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine could not start the long running query thread");
        Assert (false);
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine started the long running query thread");

    // AutoBackup
    if (m_scConfig.bAutoBackup) {
        iErrCode = m_tAutoBackupThread.Start (AutomaticBackup, this);
        if (iErrCode != OK) {
            m_pReport->WriteReport ("GameEngine could not start the auto-backup thread");
            Assert (false);
            return iErrCode;
        }
        m_pReport->WriteReport ("GameEngine started the auto-backup thread");
    }

    // Hook library
    if (m_pAlmonasterHook != NULL) {
        m_pAlmonasterHook->Running();
    }

    m_pReport->WriteReport ("GameEngine set up the system successfully");

    return iErrCode;
}


IDatabase* GameEngine::GetDatabase() {

    Assert (m_pGameData != NULL);
    m_pGameData->AddRef(); 
    return m_pGameData;
}

Chatroom* GameEngine::GetChatroom() {

    Assert (m_pChatroom != NULL);
    return m_pChatroom;
}

unsigned int GameEngine::GetRootKey()
{
    Assert(m_iRootKey != NO_KEY);
    return m_iRootKey;
}

unsigned int GameEngine::GetGuestKey()
{
    Assert(m_iGuestKey != NO_KEY);
    return m_iGuestKey;
}

IScoringSystem* GameEngine::GetScoringSystem (ScoringSystem ssScoringSystem) {

    Assert (ssScoringSystem >= FIRST_SCORING_SYSTEM && ssScoringSystem < NUM_SCORING_SYSTEMS);

    m_ppScoringSystem[ssScoringSystem]->AddRef();
    return m_ppScoringSystem[ssScoringSystem];
}

IReport* GameEngine::GetReport() {

    Assert (m_pReport != NULL);
    m_pReport->AddRef(); 
    return m_pReport;
}

int GameEngine::GetGameConfiguration (GameConfiguration* pgcConfig) {

    int iErrCode;
    IReadTable* pSystem = NULL;

    Assert (pgcConfig != NULL);

    LockGameConfigurationForReading();

    iErrCode = m_pConn->GetTableForReading (SYSTEM_DATA, &pSystem);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ShipBehavior, &pgcConfig->iShipBehavior);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ColonySimpleBuildFactor, &pgcConfig->iColonySimpleBuildFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ColonyMultipliedBuildFactor, &pgcConfig->fColonyMultipliedBuildFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ColonyMultipliedDepositFactor, &pgcConfig->fColonyMultipliedDepositFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ColonyExponentialDepositFactor, &pgcConfig->fColonyExponentialDepositFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::EngineerLinkCost, &pgcConfig->fEngineerLinkCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::StargateGateCost, &pgcConfig->fStargateGateCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::TerraformerPlowFactor, &pgcConfig->fTerraformerPlowFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::TroopshipInvasionFactor, &pgcConfig->fTroopshipInvasionFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::TroopshipFailureFactor, &pgcConfig->fTroopshipFailureFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::TroopshipSuccessFactor, &pgcConfig->fTroopshipSuccessFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::DoomsdayAnnihilationFactor, &pgcConfig->fDoomsdayAnnihilationFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::CarrierCost, &pgcConfig->fCarrierCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::BuilderMinBR, &pgcConfig->fBuilderMinBR);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::BuilderBRDampener, &pgcConfig->fBuilderBRDampener);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::BuilderMultiplier, &pgcConfig->fBuilderMultiplier);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::MorpherCost, &pgcConfig->fMorpherCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::JumpgateGateCost, &pgcConfig->fJumpgateGateCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::JumpgateRangeFactor, &pgcConfig->fJumpgateRangeFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::StargateRangeFactor, &pgcConfig->fStargateRangeFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::PercentFirstTradeIncrease, &pgcConfig->iPercentFirstTradeIncrease);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::PercentNextTradeIncrease, &pgcConfig->iPercentNextTradeIncrease);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::NukesForQuarantine, &pgcConfig->iNukesForQuarantine);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::UpdatesInQuarantine, &pgcConfig->iUpdatesInQuarantine);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::PercentTechIncreaseForLatecomers, &pgcConfig->iPercentTechIncreaseForLatecomers);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::PercentDamageUsedToDestroy, &pgcConfig->iPercentDamageUsedToDestroy);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pSystem);

    UnlockGameConfigurationForReading();

    return iErrCode;
}

int GameEngine::GetMapConfiguration (MapConfiguration* pmcConfig) {

    int iErrCode;
    IReadTable* pSystem = NULL;

    Assert (pmcConfig != NULL);

    LockMapConfigurationForReading();

    iErrCode = m_pConn->GetTableForReading (SYSTEM_DATA, &pSystem);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ChanceNewLinkForms, &pmcConfig->iChanceNewLinkForms);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::MapDeviation, &pmcConfig->iMapDeviation);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ChanceNewPlanetLinkedToLastCreatedPlanetLargeMap, &pmcConfig->iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ChanceNewPlanetLinkedToLastCreatedPlanetSmallMap, &pmcConfig->iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::LargeMapThreshold, &pmcConfig->iLargeMapThreshold);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->ReadData (SystemData::ResourceAllocationRandomizationFactor, &pmcConfig->fResourceAllocationRandomizationFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pSystem);

    UnlockMapConfigurationForReading();

    return iErrCode;
}

int GameEngine::SetGameConfiguration (const GameConfiguration& gcConfig) {

    int iErrCode;
    IWriteTable* pSystem = NULL;

    int iShipBehavior = gcConfig.iShipBehavior;

    // Sanity checks
    if ((iShipBehavior &= ALL_SHIP_BEHAVIOR_OPTIONS) != iShipBehavior ||

        gcConfig.iColonySimpleBuildFactor < 0 ||
        gcConfig.fColonyMultipliedBuildFactor < 0 ||
        gcConfig.fColonyMultipliedDepositFactor < 0 ||
        gcConfig.fColonyExponentialDepositFactor < 0 ||
        gcConfig.fEngineerLinkCost < 0 ||
        gcConfig.fStargateGateCost < 0 ||
        gcConfig.fTerraformerPlowFactor < 0 ||
        gcConfig.fTroopshipInvasionFactor < 0 ||
        gcConfig.fTroopshipFailureFactor < 0 ||
        gcConfig.fTroopshipSuccessFactor < 0 ||
        gcConfig.fTroopshipSuccessFactor < 0 ||
        gcConfig.fDoomsdayAnnihilationFactor < 0 ||
        gcConfig.fCarrierCost < 0 ||
        gcConfig.fBuilderMinBR < 0 ||
        gcConfig.fBuilderBRDampener < 0 ||
        gcConfig.fBuilderMultiplier < 0 ||
        gcConfig.fMorpherCost < 0 ||
        gcConfig.fJumpgateGateCost < 0 ||
        gcConfig.fJumpgateRangeFactor < 0 ||
        gcConfig.fStargateRangeFactor < 0 ||
        gcConfig.iPercentFirstTradeIncrease < 0 ||
        gcConfig.iPercentNextTradeIncrease < 0 ||
        gcConfig.iNukesForQuarantine < 0 ||
        gcConfig.iUpdatesInQuarantine < 0 ||
        gcConfig.iPercentTechIncreaseForLatecomers < 0 ||
        gcConfig.iPercentDamageUsedToDestroy < 0 ||
        gcConfig.iPercentDamageUsedToDestroy > 100
        ) {

        return ERROR_INVALID_ARGUMENT;
    }

    LockGameConfigurationForWriting();

    iErrCode = m_pConn->GetTableForWriting (SYSTEM_DATA, &pSystem);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ShipBehavior, gcConfig.iShipBehavior);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ColonySimpleBuildFactor, gcConfig.iColonySimpleBuildFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ColonyMultipliedBuildFactor, gcConfig.fColonyMultipliedBuildFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ColonyMultipliedDepositFactor, gcConfig.fColonyMultipliedDepositFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ColonyExponentialDepositFactor, gcConfig.fColonyExponentialDepositFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::EngineerLinkCost, gcConfig.fEngineerLinkCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::StargateGateCost, gcConfig.fStargateGateCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::TerraformerPlowFactor, gcConfig.fTerraformerPlowFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::TroopshipInvasionFactor, gcConfig.fTroopshipInvasionFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::TroopshipFailureFactor, gcConfig.fTroopshipFailureFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::TroopshipSuccessFactor, gcConfig.fTroopshipSuccessFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::DoomsdayAnnihilationFactor, gcConfig.fDoomsdayAnnihilationFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::CarrierCost, gcConfig.fCarrierCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::BuilderMinBR, gcConfig.fBuilderMinBR);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::BuilderBRDampener, gcConfig.fBuilderBRDampener);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::BuilderMultiplier, gcConfig.fBuilderMultiplier);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::MorpherCost, gcConfig.fMorpherCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::JumpgateGateCost, gcConfig.fJumpgateGateCost);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::JumpgateRangeFactor, gcConfig.fJumpgateRangeFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::StargateRangeFactor, gcConfig.fStargateRangeFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::PercentFirstTradeIncrease, gcConfig.iPercentFirstTradeIncrease);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::PercentNextTradeIncrease, gcConfig.iPercentNextTradeIncrease);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::NukesForQuarantine, gcConfig.iNukesForQuarantine);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::UpdatesInQuarantine, gcConfig.iUpdatesInQuarantine);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::PercentTechIncreaseForLatecomers, gcConfig.iPercentTechIncreaseForLatecomers);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::PercentDamageUsedToDestroy, gcConfig.iPercentDamageUsedToDestroy);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pSystem);

    UnlockGameConfigurationForWriting();

    return iErrCode;
}

int GameEngine::SetMapConfiguration (const MapConfiguration& mcConfig) {

    int iErrCode;
    IWriteTable* pSystem = NULL;

    // Sanity checks
    if (
        mcConfig.iChanceNewLinkForms < 0 ||
        mcConfig.iChanceNewLinkForms > 100 ||
        mcConfig.iMapDeviation < 0 ||
        mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap < 0 ||
        mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap > 100 ||
        mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap < 0 ||
        mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap > 100 ||
        mcConfig.iLargeMapThreshold < 1 ||
        mcConfig.fResourceAllocationRandomizationFactor < 0
        ) {

        return ERROR_INVALID_ARGUMENT;
    }

    LockMapConfigurationForWriting();

    iErrCode = m_pConn->GetTableForWriting (SYSTEM_DATA, &pSystem);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ChanceNewLinkForms, mcConfig.iChanceNewLinkForms);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::MapDeviation, mcConfig.iMapDeviation);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ChanceNewPlanetLinkedToLastCreatedPlanetLargeMap, mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ChanceNewPlanetLinkedToLastCreatedPlanetSmallMap, mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::LargeMapThreshold, mcConfig.iLargeMapThreshold);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pSystem->WriteData (SystemData::ResourceAllocationRandomizationFactor, mcConfig.fResourceAllocationRandomizationFactor);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pSystem);

    UnlockMapConfigurationForWriting();

    return iErrCode;
}


int GameEngine::GetSystemConfiguration (SystemConfiguration* pscConfig) {   
    
    *pscConfig = m_scConfig;
    return OK;
}


// Return the system's version string

const char* GameEngine::GetSystemVersion() {
    return "Almonaster Build 623 Beta 1";
}

int GameEngine::GetNewSessionId (int64* pi64SessionId) {

    Variant vSessionId;

    int iErrCode = m_pConn->Increment (SYSTEM_DATA, SystemData::SessionId, (int64) 1, &vSessionId);
    if (iErrCode == OK) {
        *pi64SessionId = vSessionId.GetInteger64();
        return OK;
    }

    return iErrCode;
}

void GameEngine::FreeData (void** ppData) {
    m_pConn->FreeData (ppData);
}

void GameEngine::FreeData (Variant* pvData) {
    m_pConn->FreeData (pvData);
}

void GameEngine::FreeData (Variant** ppvData) {
    m_pConn->FreeData (ppvData);
}

void GameEngine::FreeData (int* piData) {
    m_pConn->FreeData (piData);
}

void GameEngine::FreeData (unsigned int* piData) {
    m_pConn->FreeData (piData);
}

void GameEngine::FreeData (float* ppfData) {
    m_pConn->FreeData (ppfData);
}

void GameEngine::FreeData (char** ppszData) {
    m_pConn->FreeData (ppszData);
}

void GameEngine::FreeData (int64* pi64Data) {
    m_pConn->FreeData (pi64Data);
}

void GameEngine::FreeKeys (unsigned int* piKeys) {
    m_pConn->FreeKeys (piKeys);
}

void GameEngine::FreeKeys (int* piKeys) {
    m_pConn->FreeKeys ((unsigned int*) piKeys);
}

unsigned int GameEngine::GameObjectHashValue::GetHashValue (const char* pszData, unsigned int iNumBuckets, 
                                                            const void* pHashHint) {

    return Algorithm::GetStringHashValue (pszData, iNumBuckets, false);
}

bool GameEngine::GameObjectEquals::Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint) {

    return String::StrCmp (pszLeft, pszRight) == 0;
}