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
#include "Global.h"

int GameEngine::GetGameConfiguration(GameConfiguration* pgcConfig) {

    int iErrCode;
    IReadTable* pSystem = NULL;

    Assert (pgcConfig != NULL);

    iErrCode = t_pConn->GetTableForReading (SYSTEM_DATA, &pSystem);
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

    return iErrCode;
}

int GameEngine::GetMapConfiguration (MapConfiguration* pmcConfig) {

    int iErrCode;
    IReadTable* pSystem = NULL;

    Assert (pmcConfig != NULL);

    iErrCode = t_pConn->GetTableForReading (SYSTEM_DATA, &pSystem);
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

    iErrCode = t_pConn->GetTableForWriting (SYSTEM_DATA, &pSystem);
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

    iErrCode = t_pConn->GetTableForWriting (SYSTEM_DATA, &pSystem);
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

    return iErrCode;
}


// Return the system version string
const char* GameEngine::GetSystemVersion() {
    return "Almonaster Build 623 Beta 1";
}

int GameEngine::GetNewSessionId (int64* pi64SessionId)
{
    Variant vSessionId;

    int iErrCode = t_pConn->Increment (SYSTEM_DATA, SystemData::SessionId, (int64) 1, &vSessionId);
    if (iErrCode == OK) {
        *pi64SessionId = vSessionId.GetInteger64();
        return OK;
    }

    return iErrCode;
}

void GameEngine::FreeData (void** ppData) {
    t_pConn->FreeData (ppData);
}

void GameEngine::FreeData (Variant* pvData) {
    t_pConn->FreeData (pvData);
}

void GameEngine::FreeData (Variant** ppvData) {
    t_pConn->FreeData (ppvData);
}

void GameEngine::FreeData (int* piData) {
    t_pConn->FreeData (piData);
}

void GameEngine::FreeData (unsigned int* piData) {
    t_pConn->FreeData (piData);
}

void GameEngine::FreeData (float* ppfData) {
    t_pConn->FreeData (ppfData);
}

void GameEngine::FreeData (char** ppszData) {
    t_pConn->FreeData (ppszData);
}

void GameEngine::FreeData (int64* pi64Data) {
    t_pConn->FreeData (pi64Data);
}

void GameEngine::FreeKeys (unsigned int* piKeys) {
    t_pConn->FreeKeys (piKeys);
}

void GameEngine::FreeKeys (int* piKeys) {
    t_pConn->FreeKeys ((unsigned int*) piKeys);
}