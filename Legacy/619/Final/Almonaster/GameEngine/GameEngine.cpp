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

#include "GameEngine.h"
#include "Database.h"

#include "../Scoring/AlmonasterScore.h"
#include "../Scoring/ClassicScore.h"
#include "../Scoring/BridierScore.h"

const Uuid IID_IGameEngine = { 0xb7365051, 0xbf4f, 0x11d3, { 0xa2, 0xb6, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IAlmonasterHook = { 0xb7365052, 0xbf4f, 0x11d3, { 0xa2, 0xb6, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IAlmonasterUIEventSink = { 0xb7365053, 0xbf4f, 0x11d3, { 0xa2, 0xb6, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IMapGenerator = { 0x2751ca22, 0x493a, 0x4bdb, { 0x85, 0xcd, 0x7b, 0x59, 0xa2, 0xf8, 0x88, 0x79 } };
const Uuid IID_IScoringSystem = { 0xde61fe7e, 0xb8e8, 0x4bcd, { 0x96, 0xeb, 0xdc, 0xf7, 0x96, 0x52, 0xe, 0xb } };

GameEngine::GameEngine (const char* pszDatabaseName, 
						const char* pszHookLibrary,
						
						IAlmonasterUIEventSink* pUIEventSink, 
						
						IReport* pReport, 
						IPageSourceControl* pPageSourceControl,
						
						SystemConfiguration* pscConfig
						
						) :

						m_htGameObjectTable (NULL, NULL),
						
						m_pUIEventSink (pUIEventSink),  

						m_pReport (pReport),	// Weak ref
						m_pPageSourceControl (pPageSourceControl)	// Weak ref

						{	
	
	m_iNumRefs = 1;
	m_bGoodDatabase = false;

	memcpy (&m_scConfig, pscConfig, sizeof (m_scConfig));

	m_pszDatabaseName = String::StrDup (pszDatabaseName);

	typedef IAlmonasterHook* (*Fxn_AlmonasterHookCreateInstance)();
	Fxn_AlmonasterHookCreateInstance pCreateInstance;
	m_pAlmonasterHook = NULL;

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

	m_pGameData = NULL;

	m_iNumTemplates = 0;
	m_iMaxNumTemplates = 0;
	m_iNumTables = 0;
	m_iMaxNumTables = 0;
	m_bActiveBackup = false;
	m_tBackupStartTime = 0;

	m_pUIEventSink->AddRef();

	int i;

	ENUMERATE_SCORING_SYSTEMS(i) {
		m_ppScoringSystem[i] = NULL;
	}
}


GameEngine::~GameEngine() {

	// Check all games for updates
	int iErrCode;
	
	if (m_bGoodDatabase) {
		iErrCode = CheckAllGamesForUpdates();
		Assert (iErrCode == OK);
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

	if (m_pszDatabaseName != NULL) {
		OS::HeapFree (m_pszDatabaseName);
	}

	// Stop long running query processor
	if (m_tsfqQueryQueue.IsInitialized() && m_tsfqQueryQueue.Push (NULL)) {

		m_eQueryEvent.Signal();
		m_tLongRunningQueries.WaitForTermination();
	}

	// Clean up game object table
	GameObject* pGameObject;
	HashTableIterator<const char*, GameObject*> htiGameObject;

	rwGameObjectTableLock.WaitWriter();

	if (m_htGameObjectTable.GetNextIterator (&htiGameObject)) {
		while (m_htGameObjectTable.Delete (&htiGameObject, NULL, &pGameObject)) {
			pGameObject->Release();
		}
	}

	rwGameObjectTableLock.SignalWriter();


	// Close down database
	if (m_bGoodDatabase) {

		Assert (m_pGameData != NULL);

		m_pReport->WriteReport ("GameEngine writing last shutdown time to database");
		
		UTCTime tTime;
		Time::GetTime (&tTime);
		
		m_pGameData->WriteData (SYSTEM_DATA, SystemData::LastShutdownTime, tTime);
		
		m_pReport->WriteReport ("GameEngine wrote last shutdown time to database");
	}

	if (m_pGameData != NULL) {
		m_pReport->WriteReport ("GameEngine shutting down database");
		m_pGameData->Release();
		m_pReport->WriteReport ("GameEngine shut down database");
	}

	m_pUIEventSink->Release();

	m_pReport->WriteReport ("GameEngine terminated");

	// Release scoring systems
	int i;

	ENUMERATE_SCORING_SYSTEMS(i) {
		if (m_ppScoringSystem[i] != NULL) {
			m_ppScoringSystem[i]->Release();
		}
	}
}

// TODO - hack
#ifndef DELAY_TABLE_LOADS
#define DELAY_TABLE_LOADS (0x00000001)
#endif

int GameEngine::Initialize() {

	int iOptions = 0, i;

	if (!m_htGameObjectTable.Initialize (250) || !m_tsfqQueryQueue.Initialize()) {
		m_pReport->WriteReport ("GameEngine is out of memory");
		return ERROR_OUT_OF_MEMORY;
	}

	if (m_pAlmonasterHook != NULL) {
		m_pAlmonasterHook->Initialize (this);
	}

	// Create the database
	m_pReport->WriteReport ("GameEngine is creating the database");

	int iErrCode = DatabaseCreateInstance (CLSID_Database, IID_IDatabase, (void**) &m_pGameData);
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

	if (m_scConfig.bDelayTableLoads) {
		iOptions |= DELAY_TABLE_LOADS;
	}

	if (m_scConfig.bCheckOnStartup) {
		iOptions |= CHECK_ON_STARTUP;
	}

	iErrCode = m_pGameData->Initialize (m_pszDatabaseName, iOptions);
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

	// Init scoring systems
	m_ppScoringSystem[ALMONASTER_SCORE] = AlmonasterScore::CreateInstance (this);
	m_ppScoringSystem[CLASSIC_SCORE] = ClassicScore::CreateInstance (this);
	m_ppScoringSystem[BRIDIER_SCORE] = BridierScore::CreateInstance (this);
	m_ppScoringSystem[BRIDIER_SCORE_ESTABLISHED] = BridierScoreEstablished::CreateInstance (this);

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

	// Long running queries
	m_tLongRunningQueries.Start (LongRunningQueryProcessor, this, Thread::LowerPriority);
	m_pReport->WriteReport ("GameEngine started the long running query processor");

	// Hook library
	if (m_pAlmonasterHook != NULL) {
		m_pAlmonasterHook->Running();
	}

	// AutoBackup
	if (iErrCode == OK) {

		m_pReport->WriteReport ("GameEngine set up the system successfully");
		
		if (m_scConfig.bAutoBackup) {
			m_pReport->WriteReport ("GameEngine starting auto-backup thread");
			m_tAutoBackupThread.Start (AutomaticBackup, this);
			m_pReport->WriteReport ("GameEngine started auto-backup thread");
		}

	} else {

		m_pReport->WriteReport ("GameEngine could not set up the system successfully");
		m_scConfig.bAutoBackup = false;
	}

	return iErrCode;
}


IDatabase* GameEngine::GetDatabase() {

	Assert (m_pGameData != NULL);

	m_pGameData->AddRef(); 
	return m_pGameData;
}

IScoringSystem* GameEngine::GetScoringSystem (ScoringSystem ssScoringSystem) {

	Assert (ssScoringSystem >= FIRST_SCORING_SYSTEM && ssScoringSystem < NUM_SCORING_SYSTEMS);

	m_ppScoringSystem[ssScoringSystem]->AddRef();
	return m_ppScoringSystem[ssScoringSystem];
}

int GameEngine::GetGameConfiguration (GameConfiguration* pgcConfig) {

	int iErrCode;
	IReadTable* pSystem = NULL;

	Assert (pgcConfig != NULL);

	LockGameConfigurationForReading();

	iErrCode = m_pGameData->GetTableForReading (SYSTEM_DATA, &pSystem);
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

	UnlockGameConfigurationForReading();

	if (pSystem != NULL) {
		pSystem->Release();
	}

	return iErrCode;
}

int GameEngine::GetMapConfiguration (MapConfiguration* pmcConfig) {

	int iErrCode;
	IReadTable* pSystem = NULL;

	Assert (pmcConfig != NULL);

	LockMapConfigurationForReading();

	iErrCode = m_pGameData->GetTableForReading (SYSTEM_DATA, &pSystem);
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

	UnlockMapConfigurationForReading();

	if (pSystem != NULL) {
		pSystem->Release();
	}

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

	iErrCode = m_pGameData->GetTableForWriting (SYSTEM_DATA, &pSystem);
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

	UnlockGameConfigurationForWriting();

	if (pSystem != NULL) {
		pSystem->Release();
	}

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

	iErrCode = m_pGameData->GetTableForWriting (SYSTEM_DATA, &pSystem);
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

	UnlockMapConfigurationForWriting();

	if (pSystem != NULL) {
		pSystem->Release();
	}

	return iErrCode;
}


bool GameEngine::ReportLoginEvents() {	
	return m_scConfig.bReportLoginEvents;
}


// Output:
// *pstrEmail -> root's email address
//
// Return the empire root's email address

int GameEngine::GetAdministratorEmailAddress (Variant* pvEmail) {

	return m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, ROOT_KEY, SystemEmpireData::Email, pvEmail);
}

// Return the system's version string

const char* GameEngine::GetSystemVersion() {
	return "Almonaster Build 619.3";
}

int GameEngine::GetNewSessionId (int64* pi64SessionId) {

	Variant vSessionId;

	int iErrCode = m_pGameData->Increment (SYSTEM_DATA, SystemData::SessionId, (int64) 1, &vSessionId);
	if (iErrCode == OK) {
		*pi64SessionId = vSessionId.GetInteger64();
		return OK;
	}

	return iErrCode;
}

void GameEngine::FreeData (void** ppData) {
	m_pGameData->FreeData (ppData);
}

void GameEngine::FreeData (Variant* pvData) {
	m_pGameData->FreeData (pvData);
}

void GameEngine::FreeData (Variant** ppvData) {
	m_pGameData->FreeData (ppvData);
}

void GameEngine::FreeData (int* piData) {
	m_pGameData->FreeData (piData);
}

void GameEngine::FreeData (float* ppfData) {
	m_pGameData->FreeData (ppfData);
}

void GameEngine::FreeData (char** ppszData) {
	m_pGameData->FreeData (ppszData);
}

void GameEngine::FreeData (UTCTime* ptData) {
	m_pGameData->FreeData (ptData);
}

void GameEngine::FreeKeys (unsigned int* piKeys) {
	m_pGameData->FreeKeys (piKeys);
}

void GameEngine::FreeKeys (int* piKeys) {
	m_pGameData->FreeKeys ((unsigned int*) piKeys);
}

unsigned int GameEngine::GameObjectHashValue::GetHashValue (const char* pszData, unsigned int iNumBuckets, 
															const void* pHashHint) {

	return Algorithm::GetStringHashValue (pszData, iNumBuckets, false);
}

bool GameEngine::GameObjectEquals::Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint) {

	return String::StrCmp (pszLeft, pszRight) == 0;
}