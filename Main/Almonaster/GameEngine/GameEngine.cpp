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

#include "GameEngine.h"
#include "Global.h"

int GameEngine::GetGameConfiguration(GameConfiguration* pgcConfig) {

    int iErrCode;

    ICachedTable* pSystem = NULL;
    AutoRelease<ICachedTable> release(pSystem);

    Assert(pgcConfig != NULL);

    iErrCode = t_pCache->GetTable(SYSTEM_DATA, &pSystem);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ShipBehavior, &pgcConfig->iShipBehavior);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ColonySimpleBuildFactor, &pgcConfig->iColonySimpleBuildFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ColonyMultipliedBuildFactor, &pgcConfig->fColonyMultipliedBuildFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ColonyMultipliedDepositFactor, &pgcConfig->fColonyMultipliedDepositFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ColonyExponentialDepositFactor, &pgcConfig->fColonyExponentialDepositFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::EngineerLinkCost, &pgcConfig->fEngineerLinkCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::StargateGateCost, &pgcConfig->fStargateGateCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::TerraformerPlowFactor, &pgcConfig->fTerraformerPlowFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::TroopshipInvasionFactor, &pgcConfig->fTroopshipInvasionFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::TroopshipFailureFactor, &pgcConfig->fTroopshipFailureFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::TroopshipSuccessFactor, &pgcConfig->fTroopshipSuccessFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::DoomsdayAnnihilationFactor, &pgcConfig->fDoomsdayAnnihilationFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::CarrierCost, &pgcConfig->fCarrierCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::BuilderMinBR, &pgcConfig->fBuilderMinBR);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::BuilderBRDampener, &pgcConfig->fBuilderBRDampener);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::BuilderMultiplier, &pgcConfig->fBuilderMultiplier);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::MorpherCost, &pgcConfig->fMorpherCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::JumpgateGateCost, &pgcConfig->fJumpgateGateCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::JumpgateRangeFactor, &pgcConfig->fJumpgateRangeFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::StargateRangeFactor, &pgcConfig->fStargateRangeFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::PercentFirstTradeIncrease, &pgcConfig->iPercentFirstTradeIncrease);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::PercentNextTradeIncrease, &pgcConfig->iPercentNextTradeIncrease);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::NukesForQuarantine, &pgcConfig->iNukesForQuarantine);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::UpdatesInQuarantine, &pgcConfig->iUpdatesInQuarantine);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::PercentTechIncreaseForLatecomers, &pgcConfig->iPercentTechIncreaseForLatecomers);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::PercentDamageUsedToDestroy, &pgcConfig->iPercentDamageUsedToDestroy);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::GetMapConfiguration (MapConfiguration* pmcConfig)
{
    Assert(pmcConfig != NULL);

    int iErrCode;
    ICachedTable* pSystem = NULL;
    AutoRelease<ICachedTable> release(pSystem);

    iErrCode = t_pCache->GetTable(SYSTEM_DATA, &pSystem);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ChanceNewLinkForms, &pmcConfig->iChanceNewLinkForms);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::MapDeviation, &pmcConfig->iMapDeviation);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ChanceNewPlanetLinkedToLastCreatedPlanetLargeMap, &pmcConfig->iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ChanceNewPlanetLinkedToLastCreatedPlanetSmallMap, &pmcConfig->iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::LargeMapThreshold, &pmcConfig->iLargeMapThreshold);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->ReadData(SystemData::ResourceAllocationRandomizationFactor, &pmcConfig->fResourceAllocationRandomizationFactor);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::SetGameConfiguration (const GameConfiguration& gcConfig) {

    int iErrCode;
    ICachedTable* pSystem = NULL;
    AutoRelease<ICachedTable> release(pSystem);

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

    iErrCode = t_pCache->GetTable(SYSTEM_DATA, &pSystem);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ShipBehavior, gcConfig.iShipBehavior);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ColonySimpleBuildFactor, gcConfig.iColonySimpleBuildFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ColonyMultipliedBuildFactor, gcConfig.fColonyMultipliedBuildFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ColonyMultipliedDepositFactor, gcConfig.fColonyMultipliedDepositFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ColonyExponentialDepositFactor, gcConfig.fColonyExponentialDepositFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::EngineerLinkCost, gcConfig.fEngineerLinkCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::StargateGateCost, gcConfig.fStargateGateCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::TerraformerPlowFactor, gcConfig.fTerraformerPlowFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::TroopshipInvasionFactor, gcConfig.fTroopshipInvasionFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::TroopshipFailureFactor, gcConfig.fTroopshipFailureFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::TroopshipSuccessFactor, gcConfig.fTroopshipSuccessFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::DoomsdayAnnihilationFactor, gcConfig.fDoomsdayAnnihilationFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::CarrierCost, gcConfig.fCarrierCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::BuilderMinBR, gcConfig.fBuilderMinBR);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::BuilderBRDampener, gcConfig.fBuilderBRDampener);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::BuilderMultiplier, gcConfig.fBuilderMultiplier);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::MorpherCost, gcConfig.fMorpherCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::JumpgateGateCost, gcConfig.fJumpgateGateCost);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::JumpgateRangeFactor, gcConfig.fJumpgateRangeFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::StargateRangeFactor, gcConfig.fStargateRangeFactor);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::PercentFirstTradeIncrease, gcConfig.iPercentFirstTradeIncrease);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::PercentNextTradeIncrease, gcConfig.iPercentNextTradeIncrease);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::NukesForQuarantine, gcConfig.iNukesForQuarantine);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::UpdatesInQuarantine, gcConfig.iUpdatesInQuarantine);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::PercentTechIncreaseForLatecomers, gcConfig.iPercentTechIncreaseForLatecomers);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::PercentDamageUsedToDestroy, gcConfig.iPercentDamageUsedToDestroy);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::SetMapConfiguration (const MapConfiguration& mcConfig) {

    int iErrCode;
    ICachedTable* pSystem = NULL;
    AutoRelease<ICachedTable> release(pSystem);

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

    iErrCode = t_pCache->GetTable(SYSTEM_DATA, &pSystem);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ChanceNewLinkForms, mcConfig.iChanceNewLinkForms);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::MapDeviation, mcConfig.iMapDeviation);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ChanceNewPlanetLinkedToLastCreatedPlanetLargeMap, mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ChanceNewPlanetLinkedToLastCreatedPlanetSmallMap, mcConfig.iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::LargeMapThreshold, mcConfig.iLargeMapThreshold);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pSystem->WriteData(SystemData::ResourceAllocationRandomizationFactor, mcConfig.fResourceAllocationRandomizationFactor);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Return the system version string
const char* GameEngine::GetSystemVersion()
{
    return "Almonaster Build 700 beta 1";
}

int GameEngine::GetNewSessionId (int64* pi64SessionId)
{
    Variant vSessionId;
    int iErrCode = t_pCache->Increment(SYSTEM_DATA, SystemData::SessionId, (int64) 1, &vSessionId);
    RETURN_ON_ERROR(iErrCode);

    *pi64SessionId = vSessionId.GetInteger64();
    return iErrCode;
}

void InitGameOptions(GameOptions* pgoOptions)
{
    memset(pgoOptions, 0, sizeof(GameOptions));
    pgoOptions->iTournamentKey = NO_KEY;
}

void ClearGameOptions(GameOptions* pgoOptions)
{
    if (pgoOptions->pSecurity)
    {
        delete [] pgoOptions->pSecurity;
    }
}