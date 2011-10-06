//
// Almonaster
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

#include "Osal/Variant.h"
#include "SqlDatabase.h"
#include "GameEngineStrings.h"

////////////////
// SystemData //
////////////////

#define SYSTEM_DATA "SystemData"

namespace SystemData
{
    static const char* const DefaultAlienKey = "DefaultAlienKey";
    static const char* const DefaultAlienAddress = "DefaultAlienAddress";
    static const char* const ServerName = "ServerName";
    static const char* const DefaultMaxNumSystemMessages = "DefaultMaxNumSystemMessages";
    static const char* const DefaultMaxNumGameMessages = "DefaultMaxNumGameMessages";
    static const char* const DefaultUIButtons = "DefaultUIButtons";
    static const char* const DefaultUIBackground = "DefaultUIBackground";
    static const char* const DefaultUILivePlanet = "DefaultUILivePlanet";
    static const char* const DefaultUIDeadPlanet = "DefaultUIDeadPlanet";
    static const char* const DefaultUISeparator = "DefaultUISeparator";
    static const char* const DefaultPrivilegeLevel = "DefaultPrivilegeLevel";
    static const char* const AdeptScore = "AdeptScore";
    static const char* const MaxNumSystemMessages = "MaxNumSystemMessages";
    static const char* const MaxNumGameMessages = "MaxNumGameMessages";
    static const char* const DefaultAttackName = "DefaultAttackName";
    static const char* const DefaultScienceName = "DefaultScienceName";
    static const char* const DefaultColonyName = "DefaultColonyName";
    static const char* const DefaultStargateName = "DefaultStargateName";
    static const char* const DefaultCloakerName = "DefaultCloakerName";
    static const char* const DefaultSatelliteName = "DefaultSatelliteName";
    static const char* const DefaultTerraformerName = "DefaultTerraformerName";
    static const char* const DefaultTroopshipName = "DefaultTroopshipName";
    static const char* const DefaultDoomsdayName = "DefaultDoomsdayName";
    static const char* const DefaultMinefieldName = "DefaultMinefieldName";
    static const char* const DefaultMinesweeperName = "DefaultMinesweeperName";
    static const char* const DefaultEngineerName = "DefaultEngineerName";
    static const char* const MaxNumPersonalGameClasses = "MaxNumPersonalGameClasses";
    static const char* const DefaultUIHorz = "DefaultUIHorz";
    static const char* const DefaultUIVert = "DefaultUIVert";
    static const char* const DefaultUIColor = "DefaultUIColor";
    static const char* const MaxIconSize = "MaxIconSize";
    static const char* const SystemMaxNumEmpires = "SystemMaxNumEmpires";
    static const char* const SystemMaxNumPlanets = "SystemMaxNumPlanets";
    static const char* const SystemMinSecs = "SystemMinSecs";
    static const char* const SystemMaxSecs = "SystemMaxSecs";
    static const char* const PersonalMaxNumEmpires = "PersonalMaxNumEmpires";
    static const char* const PersonalMaxNumPlanets = "PersonalMaxNumPlanets";
    static const char* const PersonalMinSecs = "PersonalMinSecs";
    static const char* const PersonalMaxSecs = "PersonalMaxSecs";
    static const char* const Options = "Options";
    static const char* const LastBridierTimeBombScan = "LastBridierTimeBombScan";
    static const char* const BridierTimeBombScanFrequency = "BridierTimeBombScanFrequency";
    static const char* const PrivilegeForUnlimitedEmpires = "PrivilegeForUnlimitedEmpires";
    static const char* const LoginsDisabledReason = "LoginsDisabledReason";
    static const char* const NewEmpiresDisabledReason = "NewEmpiresDisabledReason";
    static const char* const NewGamesDisabledReason = "NewGamesDisabledReason";
    static const char* const AccessDisabledReason = "AccessDisabledReason";
    static const char* const ApprenticeScore = "ApprenticeScore";
    static const char* const DefaultUIIndependentPlanet = "DefaultUIIndependentPlanet";
    static const char* const MaxNumUpdatesBeforeClose = "MaxNumUpdatesBeforeClose";
    static const char* const DefaultNumUpdatesBeforeClose = "DefaultNumUpdatesBeforeClose";
    static const char* const SessionId = "SessionId";
    static const char* const MaxResourcesPerPlanet = "MaxResourcesPerPlanet";
    static const char* const MaxResourcesPerPlanetPersonal = "MaxResourcesPerPlanetPersonal";
    static const char* const MaxInitialTechLevel = "MaxInitialTechLevel";
    static const char* const MaxInitialTechLevelPersonal = "MaxInitialTechLevelPersonal";
    static const char* const MaxTechDev = "MaxTechDev";
    static const char* const MaxTechDevPersonal = "MaxTechDevPersonal";
    static const char* const PercentFirstTradeIncrease = "PercentFirstTradeIncrease";
    static const char* const PercentNextTradeIncrease = "PercentNextTradeIncrease";
    static const char* const SecondsForLongtermStatus = "SecondsForLongtermStatus";
    static const char* const NumUpdatesDownBeforeGameIsKilled = "NumUpdatesDownBeforeGameIsKilled";
    static const char* const NumNukesListedInNukeHistories = "NumNukesListedInNukeHistories";
    static const char* const NukesForQuarantine = "NukesForQuarantine";
    static const char* const UpdatesInQuarantine = "UpdatesInQuarantine";
    static const char* const PercentTechIncreaseForLatecomers = "PercentTechIncreaseForLatecomers";
    static const char* const PercentDamageUsedToDestroy = "PercentDamageUsedToDestroy";
    static const char* const AfterWeekendDelay = "AfterWeekendDelay";
    static const char* const ChanceNewLinkForms = "ChanceNewLinkForms";
    static const char* const ResourceAllocationRandomizationFactor = "ResourceAllocationRandomizationFactor";
    static const char* const MapDeviation = "MapDeviation";
    static const char* const ChanceNewPlanetLinkedToLastCreatedPlanetLargeMap = "ChanceNewPlanetLinkedToLastCreatedPlanetLargeMap";
    static const char* const ChanceNewPlanetLinkedToLastCreatedPlanetSmallMap = "ChanceNewPlanetLinkedToLastCreatedPlanetSmallMap";
    static const char* const LargeMapThreshold = "LargeMapThreshold";
    static const char* const ShipBehavior = "ShipBehavior";
    static const char* const ColonySimpleBuildFactor = "ColonySimpleBuildFactor";
    static const char* const ColonyMultipliedBuildFactor = "ColonyMultipliedBuildFactor";
    static const char* const ColonyMultipliedDepositFactor = "ColonyMultipliedDepositFactor";
    static const char* const ColonyExponentialDepositFactor = "ColonyExponentialDepositFactor";
    static const char* const EngineerLinkCost = "EngineerLinkCost";
    static const char* const StargateGateCost = "StargateGateCost";
    static const char* const TerraformerPlowFactor = "TerraformerPlowFactor";
    static const char* const TroopshipInvasionFactor = "TroopshipInvasionFactor";
    static const char* const TroopshipFailureFactor = "TroopshipFailureFactor";
    static const char* const TroopshipSuccessFactor = "TroopshipSuccessFactor";
    static const char* const DoomsdayAnnihilationFactor = "DoomsdayAnnihilationFactor";
    static const char* const CarrierCost = "CarrierCost";
    static const char* const BuilderMinBR = "BuilderMinBR";
    static const char* const MorpherCost = "MorpherCost";
    static const char* const JumpgateGateCost = "JumpgateGateCost";
    static const char* const JumpgateRangeFactor = "JumpgateRangeFactor";
    static const char* const StargateRangeFactor = "StargateRangeFactor";
    static const char* const DefaultCarrierName = "DefaultCarrierName";
    static const char* const DefaultBuilderName = "DefaultBuilderName";
    static const char* const DefaultMorpherName = "DefaultMorpherName";
    static const char* const DefaultJumpgateName = "DefaultJumpgateName";
    static const char* const BuilderMultiplier = "BuilderMultiplier";
    static const char* const NumNukesListedInSystemNukeList = "NumNukesListedInSystemNukeList";
    static const char* const NumGamesInLatestGameList = "NumGamesInLatestGameList";
    static const char* const MaxNumPersonalTournaments = "MaxNumPersonalTournaments";
    static const char* const MaxNumGameClassesPerPersonalTournament = "MaxNumGameClassesPerPersonalTournament";
    static const char* const SystemMessagesAlienKey = "SystemMessagesAlienKey";
    static const char* const SystemMessagesAlienAddress = "SystemMessagesAlienAddress";
    static const char* const AdminEmail = "AdminEmail";
    static const char* const BuilderBRDampener = "BuilderBRDampener";

    enum Columns
    {
        iDefaultAlienKey,
        iDefaultAlienAddress,
        iServerName,
        iDefaultMaxNumSystemMessages,
        iDefaultMaxNumGameMessages,
        iDefaultUIButtons,
        iDefaultUIBackground,
        iDefaultUILivePlanet,
        iDefaultUIDeadPlanet,
        iDefaultUISeparator,
        iDefaultPrivilegeLevel,
        iAdeptScore,
        iMaxNumSystemMessages,
        iMaxNumGameMessages,
        iDefaultAttackName,
        iDefaultScienceName,
        iDefaultColonyName,
        iDefaultStargateName,
        iDefaultCloakerName,
        iDefaultSatelliteName,
        iDefaultTerraformerName,
        iDefaultTroopshipName,
        iDefaultDoomsdayName,
        iDefaultMinefieldName,
        iDefaultMinesweeperName,
        iDefaultEngineerName,
        iMaxNumPersonalGameClasses,
        iDefaultUIHorz,
        iDefaultUIVert,
        iDefaultUIColor,
        iMaxIconSize,
        iSystemMaxNumEmpires,
        iSystemMaxNumPlanets,
        iSystemMinSecs,
        iSystemMaxSecs,
        iPersonalMaxNumEmpires,
        iPersonalMaxNumPlanets,
        iPersonalMinSecs,
        iPersonalMaxSecs,
        iOptions,
        iLastBridierTimeBombScan,
        iBridierTimeBombScanFrequency,
        iPrivilegeForUnlimitedEmpires,
        iLoginsDisabledReason,
        iNewEmpiresDisabledReason,
        iNewGamesDisabledReason,
        iAccessDisabledReason,
        iApprenticeScore,
        iDefaultUIIndependentPlanet,
        iMaxNumUpdatesBeforeClose,
        iDefaultNumUpdatesBeforeClose,
        iSessionId,
        iMaxResourcesPerPlanet,
        iMaxResourcesPerPlanetPersonal,
        iMaxInitialTechLevel,
        iMaxInitialTechLevelPersonal,
        iMaxTechDev,
        iMaxTechDevPersonal,
        iPercentFirstTradeIncrease,
        iPercentNextTradeIncrease,
        iSecondsForLongtermStatus,
        iNumUpdatesDownBeforeGameIsKilled,
        iNumNukesListedInNukeHistories,
        iNukesForQuarantine,
        iUpdatesInQuarantine,
        iPercentTechIncreaseForLatecomers,
        iPercentDamageUsedToDestroy,
        iAfterWeekendDelay,
        iChanceNewLinkForms,
        iResourceAllocationRandomizationFactor,
        iMapDeviation,
        iChanceNewPlanetLinkedToLastCreatedPlanetLargeMap,
        iChanceNewPlanetLinkedToLastCreatedPlanetSmallMap,
        iLargeMapThreshold,
        iShipBehavior,
        iColonySimpleBuildFactor,
        iColonyMultipliedBuildFactor,
        iColonyMultipliedDepositFactor,
        iColonyExponentialDepositFactor,
        iEngineerLinkCost,
        iStargateGateCost,
        iTerraformerPlowFactor,
        iTroopshipInvasionFactor,
        iTroopshipFailureFactor,
        iTroopshipSuccessFactor,
        iDoomsdayAnnihilationFactor,
        iCarrierCost,
        iBuilderMinBR,
        iMorpherCost,
        iJumpgateGateCost,
        iJumpgateRangeFactor,
        iStargateRangeFactor,
        iDefaultCarrierName,
        iDefaultBuilderName,
        iDefaultMorpherName,
        iDefaultJumpgateName,
        iBuilderMultiplier,
        iNumNukesListedInSystemNukeList,
        iNumGamesInLatestGameList,
        iMaxNumPersonalTournaments,
        iMaxNumGameClassesPerPersonalTournament,
        iSystemMessagesAlienKey,
        iSystemMessagesAlienAddress,
        iAdminEmail,
        iBuilderBRDampener,
    };

    static const char* const ColumnNames[] =
    {
        DefaultAlienKey,
        DefaultAlienAddress,
        ServerName,
        DefaultMaxNumSystemMessages,
        DefaultMaxNumGameMessages,
        DefaultUIButtons,
        DefaultUIBackground,
        DefaultUILivePlanet,
        DefaultUIDeadPlanet,
        DefaultUISeparator,
        DefaultPrivilegeLevel,
        AdeptScore,
        MaxNumSystemMessages,
        MaxNumGameMessages,
        DefaultAttackName,
        DefaultScienceName,
        DefaultColonyName,
        DefaultStargateName,
        DefaultCloakerName,
        DefaultSatelliteName,
        DefaultTerraformerName,
        DefaultTroopshipName,
        DefaultDoomsdayName,
        DefaultMinefieldName,
        DefaultMinesweeperName,
        DefaultEngineerName,
        MaxNumPersonalGameClasses,
        DefaultUIHorz,
        DefaultUIVert,
        DefaultUIColor,
        MaxIconSize,
        SystemMaxNumEmpires,
        SystemMaxNumPlanets,
        SystemMinSecs,
        SystemMaxSecs,
        PersonalMaxNumEmpires,
        PersonalMaxNumPlanets,
        PersonalMinSecs,
        PersonalMaxSecs,
        Options,
        LastBridierTimeBombScan,
        BridierTimeBombScanFrequency,
        PrivilegeForUnlimitedEmpires,
        LoginsDisabledReason,
        NewEmpiresDisabledReason,
        NewGamesDisabledReason,
        AccessDisabledReason,
        ApprenticeScore,
        DefaultUIIndependentPlanet,
        MaxNumUpdatesBeforeClose,
        DefaultNumUpdatesBeforeClose,
        SessionId,
        MaxResourcesPerPlanet,
        MaxResourcesPerPlanetPersonal,
        MaxInitialTechLevel,
        MaxInitialTechLevelPersonal,
        MaxTechDev,
        MaxTechDevPersonal,
        PercentFirstTradeIncrease,
        PercentNextTradeIncrease,
        SecondsForLongtermStatus,
        NumUpdatesDownBeforeGameIsKilled,
        NumNukesListedInNukeHistories,
        NukesForQuarantine,
        UpdatesInQuarantine,
        PercentTechIncreaseForLatecomers,
        PercentDamageUsedToDestroy,
        AfterWeekendDelay,
        ChanceNewLinkForms,
        ResourceAllocationRandomizationFactor,
        MapDeviation,
        ChanceNewPlanetLinkedToLastCreatedPlanetLargeMap,
        ChanceNewPlanetLinkedToLastCreatedPlanetSmallMap,
        LargeMapThreshold,
        ShipBehavior,
        ColonySimpleBuildFactor,
        ColonyMultipliedBuildFactor,
        ColonyMultipliedDepositFactor,
        ColonyExponentialDepositFactor,
        EngineerLinkCost,
        StargateGateCost,
        TerraformerPlowFactor,
        TroopshipInvasionFactor,
        TroopshipFailureFactor,
        TroopshipSuccessFactor,
        DoomsdayAnnihilationFactor,
        CarrierCost,
        BuilderMinBR,
        MorpherCost,
        JumpgateGateCost,
        JumpgateRangeFactor,
        StargateRangeFactor,
        DefaultCarrierName,
        DefaultBuilderName,
        DefaultMorpherName,
        DefaultJumpgateName,
        BuilderMultiplier,
        NumNukesListedInSystemNukeList,
        NumGamesInLatestGameList,
        MaxNumPersonalTournaments,
        MaxNumGameClassesPerPersonalTournament,
        SystemMessagesAlienKey,
        SystemMessagesAlienAddress,
        AdminEmail,
        BuilderBRDampener,
    };

    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
        V_INT,
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
        V_INT,
        V_INT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_FLOAT,
    };
    
    static const unsigned int Sizes[] = {
        0,
        0,
        MAX_SERVER_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


//////////////////////
// SystemEmpireData //
//////////////////////

#define SYSTEM_EMPIRE_DATA "SystemEmpireData"

#define GET_SYSTEM_EMPIRE_DATA(pszBuffer, i)       \
    char pszBuffer [sizeof("SystemEmpireData") + 32]; \
    sprintf(pszBuffer, "SystemEmpireData_%s_%i", ID_COLUMN_NAME, i);

namespace SystemEmpireData
{
    static const char* const Name = "Name";
    static const char* const Password = "Password";
    static const char* const Privilege = "Privilege";
    static const char* const RealName = "RealName";
    static const char* const Email = "Email";
    static const char* const WebPage = "WebPage";
    static const char* const Quote = "Quote";
    static const char* const AlienKey = "AlienKey";
    static const char* const AlienAddress = "AlienAddress";
    static const char* const UIIndependentPlanet = "UIIndependentPlanet";
    static const char* const Wins = "Wins";
    static const char* const Nukes = "Nukes";
    static const char* const Nuked = "Nuked";
    static const char* const LastLoginTime = "LastLoginTime";
    static const char* const Draws = "Draws";
    static const char* const MaxEcon = "MaxEcon";
    static const char* const MaxMil = "MaxMil";
    static const char* const IPAddress = "IPAddress";
    static const char* const Ruins = "Ruins";
    static const char* const MaxNumSystemMessages = "MaxNumSystemMessages";
    static const char* const ClassicScore = "ClassicScore";
    static const char* const AlmonasterScore = "AlmonasterScore";
    static const char* const UIButtons = "UIButtons";
    static const char* const UIBackground = "UIBackground";
    static const char* const UILivePlanet = "UILivePlanet";
    static const char* const UIDeadPlanet = "UIDeadPlanet";
    static const char* const UISeparator = "UISeparator";
    static const char* const AlmonasterTheme = "AlmonasterTheme";
    static const char* const AlternativeGraphicsPath = "AlternativeGraphicsPath";
    static const char* const DefaultAttackName = "DefaultAttackName";
    static const char* const DefaultScienceName = "DefaultScienceName";
    static const char* const DefaultColonyName = "DefaultColonyName";
    static const char* const DefaultStargateName = "DefaultStargateName";
    static const char* const DefaultCloakerName = "DefaultCloakerName";
    static const char* const DefaultSatelliteName = "DefaultSatelliteName";
    static const char* const DefaultTerraformerName = "DefaultTerraformerName";
    static const char* const DefaultTroopshipName = "DefaultTroopshipName";
    static const char* const DefaultDoomsdayName = "DefaultDoomsdayName";
    static const char* const DefaultMinefieldName = "DefaultMinefieldName";
    static const char* const DefaultMinesweeperName = "DefaultMinesweeperName";
    static const char* const DefaultEngineerName = "DefaultEngineerName";
    static const char* const UIHorz = "UIHorz";
    static const char* const UIVert = "UIVert";
    static const char* const UIColor = "UIColor";
    static const char* const CustomTableColor = "CustomTableColor";
    static const char* const Options = "Options";
    static const char* const MaxNumShipsBuiltAtOnce = "MaxNumShipsBuiltAtOnce";
    static const char* const CreationTime = "CreationTime";
    static const char* const NumLogins = "NumLogins";
    static const char* const Browser = "Browser";
    static const char* const CustomTextColor = "CustomTextColor";
    static const char* const CustomGoodColor = "CustomGoodColor";
    static const char* const CustomBadColor = "CustomBadColor";
    static const char* const CustomPrivateMessageColor = "CustomPrivateMessageColor";
    static const char* const CustomBroadcastMessageColor = "CustomBroadcastMessageColor";
    static const char* const SessionId = "SessionId";
    static const char* const DefaultBuilderPlanet = "DefaultBuilderPlanet";
    static const char* const DefaultMessageTarget = "DefaultMessageTarget";
    static const char* const AlmonasterScoreSignificance = "AlmonasterScoreSignificance";
    static const char* const VictorySneer = "VictorySneer";
    static const char* const DefaultCarrierName = "DefaultCarrierName";
    static const char* const DefaultBuilderName = "DefaultBuilderName";
    static const char* const DefaultMorpherName = "DefaultMorpherName";
    static const char* const DefaultJumpgateName = "DefaultJumpgateName";
    static const char* const BridierRank = "BridierRank";
    static const char* const BridierIndex = "BridierIndex";
    static const char* const LastBridierActivity = "LastBridierActivity";
    static const char* const PrivateEmail = "PrivateEmail";
    static const char* const Location = "Location";
    static const char* const IMId = "IMId";
    static const char* const GameRatios = "GameRatios";
    static const char* const SecretKey = "SecretKey";
    static const char* const Options2 = "Options2";
    static const char* const Gender = "Gender";
    static const char* const Age = "Age";

    enum Columns
    {
        iName,
        iPassword,
        iPrivilege,
        iRealName,
        iEmail,
        iWebPage,
        iQuote,
        iAlienKey,
        iAlienAddress,
        iUIIndependentPlanet,
        iWins,
        iNukes,
        iNuked,
        iLastLoginTime,
        iDraws,
        iMaxEcon,
        iMaxMil,
        iIPAddress,
        iRuins,
        iMaxNumSystemMessages,
        iClassicScore,
        iAlmonasterScore,
        iUIButtons,
        iUIBackground,
        iUILivePlanet,
        iUIDeadPlanet,
        iUISeparator,
        iAlmonasterTheme,
        iAlternativeGraphicsPath,
        iDefaultAttackName,
        iDefaultScienceName,
        iDefaultColonyName,
        iDefaultStargateName,
        iDefaultCloakerName,
        iDefaultSatelliteName,
        iDefaultTerraformerName,
        iDefaultTroopshipName,
        iDefaultDoomsdayName,
        iDefaultMinefieldName,
        iDefaultMinesweeperName,
        iDefaultEngineerName,
        iUIHorz,
        iUIVert,
        iUIColor,
        iCustomTableColor,
        iOptions,
        iMaxNumShipsBuiltAtOnce,
        iCreationTime,
        iNumLogins,
        iBrowser,
        iCustomTextColor,
        iCustomGoodColor,
        iCustomBadColor,
        iCustomPrivateMessageColor,
        iCustomBroadcastMessageColor,
        iSessionId,
        iDefaultBuilderPlanet,
        iDefaultMessageTarget,
        iAlmonasterScoreSignificance,
        iVictorySneer,
        iDefaultCarrierName,
        iDefaultBuilderName,
        iDefaultMorpherName,
        iDefaultJumpgateName,
        iBridierRank,
        iBridierIndex,
        iLastBridierActivity,
        iPrivateEmail,
        iLocation,
        iIMId,
        iGameRatios,
        iSecretKey,
        iOptions2,
        iGender,
        iAge,
    };

    static const char* const ColumnNames[] = {
        Name,
        Password,
        Privilege,
        RealName,
        Email,
        WebPage,
        Quote,
        AlienKey,
        AlienAddress,
        UIIndependentPlanet,
        Wins,
        Nukes,
        Nuked,
        LastLoginTime,
        Draws,
        MaxEcon,
        MaxMil,
        IPAddress,
        Ruins,
        MaxNumSystemMessages,
        ClassicScore,
        AlmonasterScore,
        UIButtons,
        UIBackground,
        UILivePlanet,
        UIDeadPlanet,
        UISeparator,
        AlmonasterTheme,
        AlternativeGraphicsPath,
        DefaultAttackName,
        DefaultScienceName,
        DefaultColonyName,
        DefaultStargateName,
        DefaultCloakerName,
        DefaultSatelliteName,
        DefaultTerraformerName,
        DefaultTroopshipName,
        DefaultDoomsdayName,
        DefaultMinefieldName,
        DefaultMinesweeperName,
        DefaultEngineerName,
        UIHorz,
        UIVert,
        UIColor,
        CustomTableColor,
        Options,
        MaxNumShipsBuiltAtOnce,
        CreationTime,
        NumLogins,
        Browser,
        CustomTextColor,
        CustomGoodColor,
        CustomBadColor,
        CustomPrivateMessageColor,
        CustomBroadcastMessageColor,
        SessionId,
        DefaultBuilderPlanet,
        DefaultMessageTarget,
        AlmonasterScoreSignificance,
        VictorySneer,
        DefaultCarrierName,
        DefaultBuilderName,
        DefaultMorpherName,
        DefaultJumpgateName,
        BridierRank,
        BridierIndex,
        LastBridierActivity,
        PrivateEmail,
        Location,
        IMId,
        GameRatios,
        SecretKey,
        Options2,
        Gender,
        Age,
    };

    static const VariantType Types[] = {
        V_STRING,
        V_STRING,
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_FLOAT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT64,
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT64,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT,
        V_INT64,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT64,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = {
        MAX_EMPIRE_NAME_LENGTH,
        MAX_PASSWORD_LENGTH,
        0,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MAX_IP_ADDRESS_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        0,
        0,
        0,
        MAX_COLOR_LENGTH,
        0,
        0,
        0,
        0,
        MAX_BROWSER_NAME_LENGTH,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        MAX_SHIP_NAME_LENGTH,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* const IndexColumns[] = {
        Name,
    };

    static unsigned int IndexFlags[] = {
        INDEX_UNIQUE_DATA,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemEmpireData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};

////////////////////////
// SystemAvailability //
////////////////////////

#define SYSTEM_AVAILABILITY "SystemAvailability"

namespace SystemAvailability
{
    static const char* const LastAvailableTime = "LastAvailableTime";

    enum Columns 
    {
        iLastAvailableTime,
    };

    static const char* const ColumnNames[] = 
    {
        LastAvailableTime,
    };

    static const VariantType Types[] = 
    {
        V_INT64,
    };

    static const unsigned int Sizes[] = 
    {
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemAvailability",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

/////////////////////////
// SystemGameClassData //
/////////////////////////

#define SYSTEM_GAMECLASS_DATA "SystemGameClassData"

namespace SystemGameClassData {

    static const char* const Name = "Name";
    static const char* const MaxNumEmpires = "MaxNumEmpires";
    static const char* const MaxNumPlanets = "MaxNumPlanets";
    static const char* const MaxTechDev = "MaxTechDev";
    static const char* const OpenGameNum = "OpenGameNum";
    static const char* const NumSecPerUpdate = "NumSecPerUpdate";
    static const char* const Options = "Options";
    static const char* const MinAvgFuel = "MinAvgFuel";
    static const char* const MaxNumShips = "MaxNumShips";
    static const char* const InitialTechLevel = "InitialTechLevel";
    static const char* const MinNumEmpires = "MinNumEmpires";
    static const char* const MaxAvgAg = "MaxAvgAg";
    static const char* const MaxAgHW = "MaxAgHW";
    static const char* const DiplomacyLevel = "DiplomacyLevel";
    static const char* const MaxAvgMin = "MaxAvgMin";
    static const char* const MaxMinHW = "MaxMinHW";
    static const char* const MaxAvgFuel = "MaxAvgFuel";
    static const char* const MapsShared = "MapsShared";
    static const char* const MaxFuelHW = "MaxFuelHW";
    static const char* const Owner = "Owner";
    static const char* const MinFuelHW = "MinFuelHW";
    static const char* const MaxNumTruces = "MaxNumTruces";
    static const char* const MaxNumTrades = "MaxNumTrades";
    static const char* const SuperClassKey = "SuperClassKey";
    static const char* const InitialTechDevs = "InitialTechDevs";
    static const char* const MaxNumAlliances = "MaxNumAlliances";
    static const char* const MinNumPlanets = "MinNumPlanets";
    static const char* const DevelopableTechDevs = "DevelopableTechDevs";
    static const char* const MinAvgAg = "MinAvgAg";
    static const char* const MinAgHW = "MinAgHW";
    static const char* const BuilderPopLevel = "BuilderPopLevel";
    static const char* const MinAvgMin = "MinAvgMin";
    static const char* const MinMinHW = "MinMinHW";
    static const char* const Description = "Description";
    static const char* const MaxAgRatio = "MaxAgRatio";
    static const char* const MaxNumActiveGames = "MaxNumActiveGames";
    static const char* const NumActiveGames = "NumActiveGames";
    static const char* const NumUpdatesForIdle = "NumUpdatesForIdle";
    static const char* const NumUpdatesForRuin = "NumUpdatesForRuin";
    static const char* const RuinFlags = "RuinFlags";
    static const char* const TournamentKey = "TournamentKey";
    static const char* const OwnerName = "OwnerName";
    static const char* const NumInitialTechDevs = "NumInitialTechDevs";

    enum Columns {
        iName,
        iMaxNumEmpires,
        iMaxNumPlanets,
        iMaxTechDev,
        iOpenGameNum,
        iNumSecPerUpdate,
        iOptions,
        iMinAvgFuel,
        iMaxNumShips,
        iInitialTechLevel,
        iMinNumEmpires,
        iMaxAvgAg,
        iMaxAgHW,
        iDiplomacyLevel,
        iMaxAvgMin,
        iMaxMinHW,
        iMaxAvgFuel,
        iMapsShared,
        iMaxFuelHW,
        iOwner,
        iMinFuelHW,
        iMaxNumTruces,
        iMaxNumTrades,
        iSuperClassKey,
        iInitialTechDevs,
        iMaxNumAlliances,
        iMinNumPlanets,
        iDevelopableTechDevs,
        iMinAvgAg,
        iMinAgHW,
        iBuilderPopLevel,
        iMinAvgMin,
        iMinMinHW,
        iDescription,
        iMaxAgRatio,
        iMaxNumActiveGames,
        iNumActiveGames,
        iNumUpdatesForIdle,
        iNumUpdatesForRuin,
        iRuinFlags,
        iTournamentKey,
        iOwnerName,
        iNumInitialTechDevs,
    };

    static const char* const ColumnNames[] = {
        Name,
        MaxNumEmpires,
        MaxNumPlanets,
        MaxTechDev,
        OpenGameNum,
        NumSecPerUpdate,
        Options,
        MinAvgFuel,
        MaxNumShips,
        InitialTechLevel,
        MinNumEmpires,
        MaxAvgAg,
        MaxAgHW,
        DiplomacyLevel,
        MaxAvgMin,
        MaxMinHW,
        MaxAvgFuel,
        MapsShared,
        MaxFuelHW,
        Owner,
        MinFuelHW,
        MaxNumTruces,
        MaxNumTrades,
        SuperClassKey,
        InitialTechDevs,
        MaxNumAlliances,
        MinNumPlanets,
        DevelopableTechDevs,
        MinAvgAg,
        MinAgHW,
        BuilderPopLevel,
        MinAvgMin,
        MinMinHW,
        Description,
        MaxAgRatio,
        MaxNumActiveGames,
        NumActiveGames,
        NumUpdatesForIdle,
        NumUpdatesForRuin,
        RuinFlags,
        TournamentKey,
        OwnerName,
        NumInitialTechDevs,
    };

    static const VariantType Types[] = {
        V_STRING,
        V_INT,
        V_INT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
    };

    static const unsigned int Sizes[] = {
        MAX_GAME_CLASS_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
    };

    static const char* const IndexColumns[] = {
        Owner
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "SystemGameClassData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};

//////////////////////
// SystemAlienIcons //
//////////////////////

#define SYSTEM_ALIEN_ICONS "SystemAlienIcons"

namespace SystemAlienIcons {

    static const char* const Address = "Address";
    static const char* const AuthorName = "AuthorName";

    enum Columns
    {      
        iAddress,
        iAuthorName
    };

    static const char* const ColumnNames[] = {
        Address,
        AuthorName,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_STRING
    };

    static const unsigned int Sizes[] = {
        0,
        MAX_ALIEN_AUTHOR_NAME_LENGTH
    };

    static const char* const IndexColumns[] = {
        Address
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemAlienIcons",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};

//////////////////////////
// SystemSuperClassData //
//////////////////////////

#define SYSTEM_SUPERCLASS_DATA "SystemSuperClassData"

namespace SystemSuperClassData {

    static const char* const Name = "Name";
    static const char* const NumGameClasses = "NumGameClasses";

    enum Columns {
        iName,
        iNumGameClasses
    };

    static const char* const ColumnNames[] = {
        Name,
        NumGameClasses,
    };

    static const VariantType Types[] = {
        V_STRING,
        V_INT,
    };

    static const unsigned int Sizes[] = {
        MAX_SUPER_CLASS_NAME_LENGTH,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "SystemSuperClassData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


//////////////////
// SystemThemes //
//////////////////

#define SYSTEM_THEMES "SystemThemes"

namespace SystemThemes
{
    static const char* const Address = "Address";
    static const char* const Name = "Name";
    static const char* const AuthorName = "AuthorName";
    static const char* const Version = "Version";
    static const char* const AuthorEmail = "AuthorEmail";
    static const char* const Description = "Description";
    static const char* const FileName = "FileName";
    static const char* const Options = "Options";
    static const char* const TableColor = "TableColor";
    static const char* const TextColor = "TextColor";
    static const char* const GoodColor = "GoodColor";
    static const char* const BadColor = "BadColor";
    static const char* const PrivateMessageColor = "PrivateMessageColor";
    static const char* const BroadcastMessageColor = "BroadcastMessageColor";

    enum Columns
    {
        iAddress,
        iName,
        iAuthorName,
        iVersion,
        iAuthorEmail,
        iDescription,
        iFileName,
        iOptions,
        iTableColor,
        iTextColor,
        iGoodColor,
        iBadColor,
        iPrivateMessageColor,
        iBroadcastMessageColor,
    };

    static const char* const ColumnNames[] =
    {
        Address,
        Name,
        AuthorName,
        Version,
        AuthorEmail,
        Description,
        FileName,
        Options,
        TableColor,
        TextColor,
        GoodColor,
        BadColor,
        PrivateMessageColor,
        BroadcastMessageColor,
    };
    
    static const VariantType Types[] =
    {
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        MAX_THEME_NAME_LENGTH,
        MAX_THEME_AUTHOR_NAME_LENGTH,
        MAX_THEME_VERSION_LENGTH,
        MAX_THEME_AUTHOR_EMAIL_LENGTH,
        MAX_THEME_DESCRIPTION_LENGTH,
        MAX_THEME_FILE_NAME_LENGTH,
        0,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
        MAX_COLOR_LENGTH,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "SystemThemes",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


///////////////////////
// SystemActiveGames //
///////////////////////

#define SYSTEM_ACTIVE_GAMES "SystemActiveGames"

namespace SystemActiveGames
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const Open = "Open";
    static const char* const TournamentKey = "TournamentKey";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iOpen,
        iTournamentKey,
    };

    static const char* const ColumnNames[] = 
    {
        GameClass,
        GameNumber,
        Open,
        TournamentKey,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        0,
    };

    static const char* const IndexColumns[] =
    {
        GameClass,
        GameNumber,
        TournamentKey,
    };

    static unsigned int IndexFlags[] = 
    {
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = 
    {
        "SystemActiveGames",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags,
    };
};

/////////////////////////////////
// SystemEmpireAssociations(I) //
/////////////////////////////////

#define SYSTEM_EMPIRE_ASSOCIATIONS "SystemEmpireAssociations"

#define GET_SYSTEM_EMPIRE_ASSOCIATIONS(pszBuffer, i)                        \
    char pszBuffer[sizeof("SystemEmpireAssociations_EmpireKey_%i") + 32];   \
    sprintf(pszBuffer, "SystemEmpireAssociations_EmpireKey_%i", i);

namespace SystemEmpireAssociations
{
    static const char* const EmpireKey = "EmpireKey";
    static const char* const ReferenceEmpireKey = "ReferenceEmpireKey";

    enum Columns
    {
        iEmpireKey,
        iReferenceEmpireKey,
    };

    static const char* const ColumnNames[] =
    {
        EmpireKey,
        ReferenceEmpireKey,
    };
    
    static const VariantType Types[] =
    {
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] =
    {
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemEmpireAssociations",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

/////////////////////////////
// SystemEmpireMessages(I) //
/////////////////////////////

#define SYSTEM_EMPIRE_MESSAGES "SystemEmpireMessages"

#define GET_SYSTEM_EMPIRE_MESSAGES(pszBuffer, i)                        \
    char pszBuffer[sizeof("SystemEmpireMessages_EmpireKey_%i") + 32];   \
    sprintf(pszBuffer, "SystemEmpireMessages_EmpireKey_%i", i);

namespace SystemEmpireMessages
{
    static const char* const EmpireKey = "EmpireKey";
    static const char* const Unread = "Unread";
    static const char* const SourceKey = "SourceKey";
    static const char* const SourceName = "SourceName";
    static const char* const TimeStamp = "TimeStamp";
    static const char* const Flags = "Flags";
    static const char* const Text = "Text";
    static const char* const Type = "Type";
    static const char* const Data = "Data";

    enum Columns {
        iEmpireKey,
        iUnread,
        iSourceKey,
        iSourceName,
        iTimeStamp,
        iFlags,
        iText,
        iType,
        iData,
    };

    static const char* const ColumnNames[] = {
        EmpireKey,
        Unread,
        SourceKey,
        SourceName,
        TimeStamp,
        Flags,
        Text,
        Type,
        Data,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT64,
        V_INT,
        V_STRING,
        V_INT,
        V_STRING,
    };

    static const unsigned int Sizes[] = {
        0,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "SystemEmpireMessages",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


//////////////////////////////
// SystemEmpireNukerList(I) //
//////////////////////////////
//////////////////////////////
// SystemEmpireNukedList(I) //
//////////////////////////////

#define SYSTEM_EMPIRE_NUKER_LIST "SystemEmpireNukerList"

#define GET_SYSTEM_EMPIRE_NUKER_LIST(pszBuffer, i)                      \
    char pszBuffer[sizeof("SystemEmpireNukerList_EmpireKey_%i") + 32];  \
    sprintf(pszBuffer, "SystemEmpireNukerList_EmpireKey_%i", i);

#define COPY_SYSTEM_EMPIRE_NUKER_LIST(pszBuffer, i)                     \
    sprintf(pszBuffer, "SystemEmpireNukerList_EmpireKey_%i", i);

#define SYSTEM_EMPIRE_NUKED_LIST "SystemEmpireNukedList"

#define GET_SYSTEM_EMPIRE_NUKED_LIST(pszBuffer, i)                      \
    char pszBuffer[sizeof("SystemEmpireNukedList_EmpireKey_%i") + 32];  \
    sprintf(pszBuffer, "SystemEmpireNukedList_EmpireKey_%i", i);

#define COPY_SYSTEM_EMPIRE_NUKED_LIST(pszBuffer, i)                     \
    sprintf(pszBuffer, "SystemEmpireNukedList_EmpireKey_%i", i);

namespace SystemEmpireNukeList
{
    static const char* const EmpireKey = "EmpireKey";
    static const char* const AlienKey = "AlienKey";
    static const char* const AlienAddress = "AlienAddress";
    static const char* const EmpireName = "EmpireName";
    static const char* const ReferenceEmpireKey = "ReferenceEmpireKey";
    static const char* const GameClassName = "GameClassName";
    static const char* const GameNumber = "GameNumber";
    static const char* const TimeStamp = "TimeStamp";

    enum Columns
    {
        iEmpireKey,
        iAlienKey,
        iAlienAddress,
        iEmpireName,
        iReferenceEmpireKey,
        iGameClassName,
        iGameNumber,
        iTimeStamp,
    };

    static const char* const ColumnNames[] = 
    {
        EmpireKey,
        AlienKey,
        AlienAddress,
        EmpireName,
        ReferenceEmpireKey,
        GameClassName,
        GameNumber,
        TimeStamp,
    };

    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_STRING,
        V_INT,
        V_INT64,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemEmpireNukeList",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

////////////////////
// SystemNukeList //
////////////////////

#define SYSTEM_NUKE_LIST "SystemNukeList"

namespace SystemNukeList {

    static const char* const NukerAlienKey = "NukerAlienKey";
    static const char* const NukerAlienAddress = "NukerAlienAddress";
    static const char* const NukerEmpireName = "NukerEmpireName";
    static const char* const NukerEmpireKey = "NukerEmpireKey";
    static const char* const NukedAlienKey = "NukedAlienKey";
    static const char* const NukedAlienAddress = "NukedAlienAddress";
    static const char* const NukedEmpireName = "NukedEmpireName";
    static const char* const NukedEmpireKey = "NukedEmpireKey";
    static const char* const GameClassName = "GameClassName";
    static const char* const GameNumber = "GameNumber";
    static const char* const TimeStamp = "TimeStamp";

    enum Columns {
        iNukerAlienKey,
        iNukerAlienAddress,
        iNukerEmpireName,
        iNukerEmpireKey,
        iNukedAlienKey,
        iNukedAlienAddress,
        iNukedEmpireName,
        iNukedEmpireKey,
        iGameClassName,
        iGameNumber,
        iTimeStamp,
    };

    static const char* const ColumnNames[] = {
        NukerAlienKey,
        NukerAlienAddress,
        NukerEmpireName,
        NukerEmpireKey,
        NukedAlienKey,
        NukedAlienAddress,
        NukedEmpireName,
        NukedEmpireKey,
        GameClassName,
        GameNumber,
        TimeStamp,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_STRING,
        V_INT,
        V_INT64,
    };

    static const unsigned int Sizes[] = {
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "SystemNukeList",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


///////////////////////
// SystemLatestGames //
///////////////////////

#define SYSTEM_LATEST_GAMES "SystemLatestGames"

namespace SystemLatestGames
{
    static const char* const Name = "Name";
    static const char* const Number = "Number";
    static const char* const Created = "Created";
    static const char* const Ended = "Ended";
    static const char* const Updates = "Updates";
    static const char* const Result = "Result";
    static const char* const Winners = "Winners";
    static const char* const Losers = "Losers";
    static const char* const TournamentKey = "TournamentKey";

    enum Columns
    {
        iName,
        iNumber,
        iCreated,
        iEnded,
        iUpdates,
        iResult,
        iWinners,
        iLosers,
        iTournamentKey,
    };

    static const char* const ColumnNames[] =
    {
        Name,
        Number,
        Created,
        Ended,
        Updates,
        Result,
        Winners,
        Losers,
        TournamentKey,
    };
    
    static const VariantType Types[] = 
    {
        V_STRING,
        V_INT,
        V_INT64,
        V_INT64,
        V_INT,
        V_INT,
        V_STRING,
        V_STRING,
        V_INT,
    };

    static const unsigned int Sizes[] = 
    {
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = 
    {
        "SystemLatestGames",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

////////////////////////////////
// SystemEmpireActiveGames(I) //
////////////////////////////////

#define SYSTEM_EMPIRE_ACTIVE_GAMES "SystemEmpireActiveGames"

#define GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszBuffer, i)                    \
    char pszBuffer [sizeof("SystemEmpireActiveGames_EmpireKey_%i") + 32]; \
    sprintf(pszBuffer, "SystemEmpireActiveGames_EmpireKey_%i", i);

namespace SystemEmpireActiveGames
{
    static const char* const EmpireKey = "EmpireKey";
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";

    enum Columns
    {
        iEmpireKey,
        iGameClass,
        iGameNumber
    };

    static const char* const ColumnNames[] =
    {
        EmpireKey,
        GameClass,
        GameNumber
    };

    static const VariantType Types[] =
    {
        V_INT,
        V_INT,
        V_INT
    };

    static const unsigned int Sizes[] =
    {
        0,
        0,
        0
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemEmpireActiveGames",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

///////////////////////
// SystemTournaments //
///////////////////////

#define SYSTEM_TOURNAMENTS "SystemTournaments"

namespace SystemTournaments
{
    static const char* const Name = "Name";
    static const char* const Description = "Description";
    static const char* const WebPage = "WebPage";
    static const char* const News = "News";
    static const char* const Owner = "Owner";
    static const char* const Icon = "Icon";
    static const char* const IconAddress = "IconAddress";
    static const char* const OwnerName = "OwnerName";

    enum Columns
    {
        iName,
        iDescription,
        iWebPage,
        iNews,
        iOwner,
        iIcon,
        iIconAddress,
        iOwnerName,
    };

    static const char* const ColumnNames[] =
    {
        Name,
        Description,
        WebPage,
        News,
        Owner,
        Icon,
        IconAddress,
        OwnerName,
    };

    static const VariantType Types[] = {
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
    };

    static const unsigned int Sizes[] = {
        MAX_TOURNAMENT_NAME_LENGTH,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
    };

    static const char* const IndexColumns[] = {
        Owner
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemTournaments",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};

//////////////////////////////
// SystemTournamentTeams(I) //
//////////////////////////////

#define SYSTEM_TOURNAMENT_TEAMS "SystemTournamentTeams"

#define GET_SYSTEM_TOURNAMENT_TEAMS(pszBuffer, i)                           \
    char pszBuffer [sizeof("SystemTournamentTeams_TournamentKey_") + 32];   \
    sprintf(pszBuffer, "SystemTournamentTeams_TournamentKey_%i", i);

namespace SystemTournamentTeams
{
    static const char* const TournamentKey = "TournamentKey";
    static const char* const Name = "Name";
    static const char* const Description = "Description";
    static const char* const WebPage = "WebPage";
    static const char* const Icon = "Icon";
    static const char* const IconAddress = "IconAddress";
    static const char* const Wins = "Wins";
    static const char* const Nukes = "Nukes";
    static const char* const Nuked = "Nuked";
    static const char* const Draws = "Draws";
    static const char* const Ruins = "Ruins";

    enum Columns
    {
        iTournamentKey,
        iName,
        iDescription,
        iWebPage,
        iIcon,
        iIconAddress,
        iWins,
        iNukes,
        iNuked,
        iDraws,
        iRuins,
    };

    static const char* const ColumnNames[] =
    {
        TournamentKey,
        Name,
        Description,
        WebPage,
        Icon,
        IconAddress,
        Wins,
        Nukes,
        Nuked,
        Draws,
        Ruins,
    };

    static const VariantType Types[] = 
    {
        V_INT,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] =
    {
        0,
        MAX_TOURNAMENT_TEAM_NAME_LENGTH,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = 
    {
        "SystemTournamentTeams",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

////////////////////////////////
// SystemTournamentEmpires(I) //
////////////////////////////////

#define SYSTEM_TOURNAMENT_EMPIRES "SystemTournamentEmpires"

#define GET_SYSTEM_TOURNAMENT_EMPIRES(pszBuffer, i)                             \
    char pszBuffer[sizeof("SystemTournamentEmpires_TournamentKey_") + 32];    \
    sprintf(pszBuffer, "SystemTournamentEmpires_TournamentKey_%i", i);

namespace SystemTournamentEmpires
{
    static const char* const TournamentKey = "TournamentKey";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const TeamKey = "TeamKey";
    static const char* const Wins = "Wins";
    static const char* const Nukes = "Nukes";
    static const char* const Nuked = "Nuked";
    static const char* const Draws = "Draws";
    static const char* const Ruins = "Ruins";

    enum Columns 
    {
        iTournamentKey,
        iEmpireKey,
        iTeamKey,
        iWins,
        iNukes,
        iNuked,
        iDraws,
        iRuins,
    };

    static const char* const ColumnNames[] = 
    {
        TournamentKey,
        EmpireKey,
        TeamKey,
        Wins,
        Nukes,
        Nuked,
        Draws,
        Ruins,
    };

    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemTournamentEmpires",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

////////////////////////////////
// SystemEmpireTournaments(I) //
////////////////////////////////

#define SYSTEM_EMPIRE_TOURNAMENTS "SystemEmpireTournaments"

#define GET_SYSTEM_EMPIRE_TOURNAMENTS(pszBuffer, i)             \
    char pszBuffer[sizeof("SystemEmpireTournaments_EmpireKey_%i") + 32];   \
    sprintf(pszBuffer, "SystemEmpireTournaments_EmpireKey_%i", i);

namespace SystemEmpireTournaments
{
    static const char* const EmpireKey = "EmpireKey";
    static const char* const TournamentKey = "TournamentKey";

    enum Columns
    {
        iEmpireKey,
        iTournamentKey,
    };

    static const char* const ColumnNames[] =
    {
        EmpireKey,
        TournamentKey,
    };

    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template =
    {
        "SystemEmpireTournaments",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

////////////////////////
// SystemChatroomData //
////////////////////////

#define SYSTEM_CHATROOM_DATA "SystemChatroomData"

namespace SystemChatroomData
{
    static const char* const Flags = "Flags";
    static const char* const Time = "Time";
    static const char* const Speaker = "Speaker";
    static const char* const Message = "Message";

    enum Columns {
        iFlags,
        iTime,
        iSpeaker,
        iMessage,
    };

    static const char* const ColumnNames[] = {
        Flags,
        Time,
        Speaker,
        Message,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_INT64,
        V_STRING,
        V_STRING,
    };

    static const unsigned int Sizes[] = {
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "SystemChatroomData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

///////////////////
// GameData(I.I) //
///////////////////

#define GAME_DATA "GameData"

#define GET_GAME_DATA(pszBuffer, i, j)          \
    char pszBuffer[sizeof("GameData_GameClass__GameNumber_") + 64];  \
    sprintf(pszBuffer, "GameData_GameClass_%i_GameNumber_%i", i, j);

#define COPY_GAME_DATA(pszBuffer, i, j)         \
    sprintf(pszBuffer, "GameData_GameClass_%i_GameNumber_%i", i, j);

namespace GameData
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const MaxNumEmpires = "MaxNumEmpires";
    static const char* const NumUpdates = "NumUpdates";
    static const char* const LastUpdateTime = "LastUpdateTime";
    static const char* const State = "State";
    static const char* const MinX = "MinX";
    static const char* const NumEmpiresUpdated = "NumEmpiresUpdated";
    static const char* const Password = "Password";
    static const char* const MaxX = "MaxX";
    static const char* const MinY = "MinY";
    static const char* const NumRequestingPause = "NumRequestingPause";
    static const char* const MaxY = "MaxY";
    static const char* const SecondsUntilNextUpdateWhilePaused = "SecondsUntilNextUpdateWhilePaused";
    static const char* const CreationTime = "CreationTime";
    static const char* const NumPlanetsPerEmpire = "NumPlanetsPerEmpire";
    static const char* const HWAg = "HWAg";
    static const char* const AvgAg = "AvgAg";
    static const char* const HWMin = "HWMin";
    static const char* const AvgMin = "AvgMin";
    static const char* const HWFuel = "HWFuel";
    static const char* const AvgFuel = "AvgFuel";
    static const char* const NumEmpiresResigned = "NumEmpiresResigned";
    static const char* const Options = "Options";
    static const char* const NumUpdatesBeforeGameCloses = "NumUpdatesBeforeGameCloses";
    static const char* const FirstUpdateDelay = "FirstUpdateDelay";
    static const char* const EnterGameMessage = "EnterGameMessage";
    static const char* const CreatorName = "CreatorName";
    static const char* const MinAlmonasterScore = "MinAlmonasterScore";
    static const char* const MaxAlmonasterScore = "MaxAlmonasterScore";
    static const char* const MinClassicScore = "MinClassicScore";
    static const char* const MaxClassicScore = "MaxClassicScore";
    static const char* const MinBridierRank = "MinBridierRank";
    static const char* const MaxBridierRank = "MaxBridierRank";
    static const char* const MinBridierIndex = "MinBridierIndex";
    static const char* const MaxBridierIndex = "MaxBridierIndex";
    static const char* const MinBridierRankGain = "MinBridierRankGain";
    static const char* const MaxBridierRankGain = "MaxBridierRankGain";
    static const char* const MinWins = "MinWins";
    static const char* const MaxWins = "MaxWins";
    static const char* const NumRequestingDraw = "NumRequestingDraw";
    static const char* const MinBridierRankLoss = "MinBridierRankLoss";
    static const char* const MaxBridierRankLoss = "MaxBridierRankLoss";
    static const char* const RealLastUpdateTime = "RealLastUpdateTime";
    static const char* const MapFairness = "MapFairness";
    static const char* const MapFairnessStandardDeviationPercentageOfMean = "MapFairnessStandardDeviationPercentageOfMean";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iMaxNumEmpires,
        iNumUpdates,
        iLastUpdateTime,
        iState,
        iMinX,
        iNumEmpiresUpdated,
        iPassword,
        iMaxX,
        iMinY,
        iNumRequestingPause,
        iMaxY,
        iSecondsUntilNextUpdateWhilePaused,
        iCreationTime,
        iNumPlanetsPerEmpire,
        iHWAg,
        iAvgAg,
        iHWMin,
        iAvgMin,
        iHWFuel,
        iAvgFuel,
        iNumEmpiresResigned,
        iOptions,
        iNumUpdatesBeforeGameCloses,
        iFirstUpdateDelay,
        iEnterGameMessage,
        iCreatorName,
        iMinAlmonasterScore,
        iMaxAlmonasterScore,
        iMinClassicScore,
        iMaxClassicScore,
        iMinBridierRank,
        iMaxBridierRank,
        iMinBridierIndex,
        iMaxBridierIndex,
        iMinBridierRankGain,
        iMaxBridierRankGain,
        iMinWins,
        iMaxWins,
        iNumRequestingDraw,
        iMinBridierRankLoss,
        iMaxBridierRankLoss,
        iRealLastUpdateTime,
        iMapFairness,
        iMapFairnessStandardDeviationPercentageOfMean
    };

    static const char* const ColumnNames[] = 
    {
        GameClass,
        GameNumber,
        MaxNumEmpires,
        NumUpdates,
        LastUpdateTime,
        State,
        MinX,
        NumEmpiresUpdated,
        Password,
        MaxX,
        MinY,
        NumRequestingPause,
        MaxY,
        SecondsUntilNextUpdateWhilePaused,
        CreationTime,
        NumPlanetsPerEmpire,
        HWAg,
        AvgAg,
        HWMin,
        AvgMin,
        HWFuel,
        AvgFuel,
        NumEmpiresResigned,
        Options,
        NumUpdatesBeforeGameCloses,
        FirstUpdateDelay,
        EnterGameMessage,
        CreatorName,
        MinAlmonasterScore,
        MaxAlmonasterScore,
        MinClassicScore,
        MaxClassicScore,
        MinBridierRank,
        MaxBridierRank,
        MinBridierIndex,
        MaxBridierIndex,
        MinBridierRankGain,
        MaxBridierRankGain,
        MinWins,
        MaxWins,
        NumRequestingDraw,
        MinBridierRankLoss,
        MaxBridierRankLoss,
        RealLastUpdateTime,
        MapFairness,
        MapFairnessStandardDeviationPercentageOfMean,
    };

    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_STRING,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MAX_PASSWORD_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };
    
    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "GameData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


///////////////////////
// GameSecurity(I.I) //
///////////////////////

#define GAME_SECURITY "GameSecurity"

#define GET_GAME_SECURITY(pszBuffer, i, j)              \
    char pszBuffer[sizeof("GameSecurity_GameClass__GameNumber_") + 64];  \
    sprintf(pszBuffer, "GameSecurity_GameClass_%i_GameNumber_%i", i, j);

#define COPY_GAME_SECURITY(pszBuffer, i, j)          \
    sprintf(pszBuffer, "GameSecurity_GameClass_%i_GameNumber_%i", i, j);

namespace GameSecurity
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const Name = "Name";
    static const char* const Options = "Options";
    static const char* const SessionId = "SessionId";
    static const char* const IPAddress = "IPAddress";
    static const char* const SecretKey = "SecretKey";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iName,
        iOptions,
        iSessionId,
        iIPAddress,
        iSecretKey,
    };

    static const char* const ColumnNames[] =
    {
        GameClass,
        GameNumber,
        EmpireKey,
        Name,
        Options,
        SessionId,
        IPAddress,
        SecretKey,
    };

    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT64,
        V_STRING,
        V_INT64,
    };
    
    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        MAX_IP_ADDRESS_LENGTH,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "GameSecurity",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};

//////////////////////
// GameEmpires(I.I) //
//////////////////////

#define GAME_EMPIRES "GameEmpires"

#define GET_GAME_EMPIRES(pszBuffer, i, j)               \
    char pszBuffer[sizeof("GameEmpires_GameClass__GameNumber_") + 64];   \
    sprintf(pszBuffer, "GameEmpires_GameClass_%i_GameNumber_%i", i, j);

#define COPY_GAME_EMPIRES(pszBuffer, i, j)           \
    sprintf(pszBuffer, "GameEmpires_GameClass_%i_GameNumber_%i", i, j);

namespace GameEmpires
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const EmpireName = "EmpireName";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iEmpireName,
    };

    static const char* const ColumnNames[] = 
    {
        GameClass,
        GameNumber,
        EmpireKey,
        EmpireName,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
    };
    
    static const char* const IndexColumns[] = {
        EmpireKey
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "GameEmpires",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};

//////////////////////////
// GameNukedEmpires(I.I) //
//////////////////////////

#define GAME_NUKED_EMPIRES "GameNukedEmpires"

#define GET_GAME_NUKED_EMPIRES(pszBuffer, i, j)              \
    char pszBuffer[sizeof("GameNukedEmpires_GameClass__GameNumber_") + 64];   \
    sprintf(pszBuffer, "GameNukedEmpires_GameClass_%i_GameNumber_%i", i, j);

#define COPY_GAME_NUKED_EMPIRES(pszBuffer, i, j)          \
    sprintf(pszBuffer, "GameNukedEmpires_GameClass_%i_GameNumber_%i", i, j);

namespace GameNukedEmpires
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const Name = "Name";
    static const char* const NukedEmpireKey = "NukedEmpireKey";
    static const char* const AlienKey = "AlienKey";
    static const char* const AlienAddress = "AlienAddress";
    static const char* const Update = "Update";
    static const char* const Reason = "Reason";
    static const char* const SecretKey = "SecretKey";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iName,
        iNukedEmpireKey,
        iAlienKey,
        iAlienAddress,
        iUpdate,
        iReason,
        iSecretKey,
    };

    static const char* const ColumnNames[] =
    {
        GameClass,
        GameNumber,
        Name,
        NukedEmpireKey,
        AlienKey,
        AlienAddress,
        Update,
        Reason,
        SecretKey,
    };

    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "GameNukedEmpires",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


//////////////////
// GameMap(I.I) //
//////////////////

#define GAME_MAP "GameMap"

#define GET_GAME_MAP(pszBuffer, i, j)                   \
    char pszBuffer[sizeof("GameMap_GameClass__GameNumber_") + 64];       \
    sprintf(pszBuffer, "GameMap_GameClass_%i_GameNumber_%i", i, j);

#define COPY_GAME_MAP(pszBuffer, i, j)               \
    sprintf(pszBuffer, "GameMap_GameClass_%i_GameNumber_%i", i, j);

namespace GameMap
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";    
    static const char* const Name = "Name";
    static const char* const Ag = "Ag";
    static const char* const Minerals = "Minerals";
    static const char* const Fuel = "Fuel";
    static const char* const Pop = "Pop";
    static const char* const MaxPop = "MaxPop";
    static const char* const Owner = "Owner";
    static const char* const Nuked = "Nuked";
    static const char* const Coordinates = "Coordinates";
    static const char* const NorthPlanetKey = "NorthPlanetKey";
    static const char* const EastPlanetKey = "EastPlanetKey";
    static const char* const SouthPlanetKey = "SouthPlanetKey";
    static const char* const WestPlanetKey = "WestPlanetKey";
    static const char* const Link = "Link";
    static const char* const PopLostToColonies = "PopLostToColonies";
    static const char* const SurrenderNumAllies = "SurrenderNumAllies";
    static const char* const SurrenderAlmonasterSignificance = "SurrenderAlmonasterSignificance";
    static const char* const HomeWorld = "HomeWorld";
    static const char* const Annihilated = "Annihilated";
    static const char* const NumUncloakedShips = "NumUncloakedShips";
    static const char* const NumCloakedShips = "NumCloakedShips";
    static const char* const NumUncloakedBuildShips = "NumUncloakedBuildShips";
    static const char* const NumCloakedBuildShips = "NumCloakedBuildShips";
    static const char* const SurrenderEmpireSecretKey = "SurrenderEmpireSecretKey";
    static const char* const SurrenderAlmonasterScore = "SurrenderAlmonasterScore";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iName,
        iAg,
        iMinerals,
        iFuel,
        iPop,
        iMaxPop,
        iOwner,
        iNuked,
        iCoordinates,
        iNorthPlanetKey,
        iEastPlanetKey,
        iSouthPlanetKey,
        iWestPlanetKey,
        iLink,
        iPopLostToColonies,
        iSurrenderNumAllies,
        iSurrenderAlmonasterSignificance,
        iHomeWorld,
        iAnnihilated,
        iNumUncloakedShips,
        iNumCloakedShips,
        iNumUncloakedBuildShips,
        iNumCloakedBuildShips,
        iSurrenderEmpireSecretKey,
        iSurrenderAlmonasterScore,
    };

    static const char* const ColumnNames[] = 
    {
        GameClass,
        GameNumber,
        Name,
        Ag,
        Minerals,
        Fuel,
        Pop,
        MaxPop,
        Owner,
        Nuked,
        Coordinates,
        NorthPlanetKey,
        EastPlanetKey,
        SouthPlanetKey,
        WestPlanetKey,
        Link,
        PopLostToColonies,
        SurrenderNumAllies,
        SurrenderAlmonasterSignificance,
        HomeWorld,
        Annihilated,
        NumUncloakedShips,
        NumCloakedShips,
        NumUncloakedBuildShips,
        NumCloakedBuildShips,
        SurrenderEmpireSecretKey,
        SurrenderAlmonasterScore,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
        V_FLOAT,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        MAX_PLANET_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MAX_COORDINATE_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* const IndexColumns[] = {
        Owner,
        Coordinates
    };

    static unsigned int IndexFlags[] = {
        0,
        0,
    };
    
    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "GameMap",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};

///////////////////////////
// GameEmpireData(I.I.I) //
///////////////////////////

#define GAME_EMPIRE_DATA "GameEmpireData"

#define GET_GAME_EMPIRE_DATA(pszBuffer, i, j, k)                    \
    char pszBuffer[sizeof("GameEmpireData_GameClass__GameNumber__EmpireKey_%i") + 96];            \
    sprintf(pszBuffer, "GameEmpireData_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

#define COPY_GAME_EMPIRE_DATA(pszBuffer, i, j, k)                \
    sprintf(pszBuffer, "GameEmpireData_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

namespace GameEmpireData
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const NumPlanets = "NumPlanets";
    static const char* const TotalAg = "TotalAg";
    static const char* const TotalFuel = "TotalFuel";
    static const char* const TotalMin = "TotalMin";
    static const char* const TechLevel = "TechLevel";
    static const char* const TotalPop = "TotalPop";
    static const char* const TotalBuild = "TotalBuild";
    static const char* const TotalMaintenance = "TotalMaintenance";
    static const char* const TotalFuelUse = "TotalFuelUse";
    static const char* const LastLogin = "LastLogin";
    static const char* const EnterGameIPAddress = "EnterGameIPAddress";
    static const char* const Options = "Options";
    static const char* const PartialMapCenter = "PartialMapCenter";
    static const char* const NumAvailableTechUndevs = "NumAvailableTechUndevs";
    static const char* const Econ = "Econ";
    static const char* const Mil = "Mil";
    static const char* const TargetPop = "TargetPop";
    static const char* const HomeWorld = "HomeWorld";
    static const char* const NumUpdatesIdle = "NumUpdatesIdle";
    static const char* const MaxBR = "MaxBR";
    static const char* const BonusAg = "BonusAg";
    static const char* const BonusFuel = "BonusFuel";
    static const char* const BonusMin = "BonusMin";
    static const char* const NumBuilds = "NumBuilds";
    static const char* const MinX = "MinX";
    static const char* const MaxX = "MaxX";
    static const char* const MinY = "MinY";
    static const char* const MaxY = "MaxY";
    static const char* const NextMaintenance = "NextMaintenance";
    static const char* const NextFuelUse = "NextFuelUse";
    static const char* const NextTotalPop = "NextTotalPop";
    static const char* const NextMin = "NextMin";
    static const char* const NextFuel = "NextFuel";
    static const char* const MaxNumGameMessages = "MaxNumGameMessages";
    static const char* const TechDevs = "TechDevs";
    static const char* const TechUndevs = "TechUndevs";
    static const char* const PartialMapXRadius = "PartialMapXRadius";
    static const char* const PartialMapYRadius = "PartialMapYRadius";
    static const char* const Notepad = "Notepad";
    static const char* const DefaultBuilderPlanet = "DefaultBuilderPlanet";
    static const char* const LastBuilderPlanet = "LastBuilderPlanet";
    static const char* const MaxEcon = "MaxEcon";
    static const char* const MaxMil = "MaxMil";
    static const char* const NumNukedAllies = "NumNukedAllies";
    static const char* const DefaultMessageTarget = "DefaultMessageTarget";
    static const char* const LastMessageTargetMask = "LastMessageTargetMask";
    static const char* const InitialBridierRank = "InitialBridierRank";
    static const char* const InitialBridierIndex = "InitialBridierIndex";
    static const char* const GameRatios = "GameRatios";
    static const char* const MiniMaps = "MiniMaps";
    static const char* const MapFairnessResourcesClaimed = "MapFairnessResourcesClaimed";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iNumPlanets,
        iTotalAg,
        iTotalFuel,
        iTotalMin,
        iTechLevel,
        iTotalPop,
        iTotalBuild,
        iTotalMaintenance,
        iTotalFuelUse,
        iLastLogin,
        iEnterGameIPAddress,
        iOptions,
        iPartialMapCenter,
        iNumAvailableTechUndevs,
        iEcon,
        iMil,
        iTargetPop,
        iHomeWorld,
        iNumUpdatesIdle,
        iMaxBR,
        iBonusAg,
        iBonusFuel,
        iBonusMin,
        iNumBuilds,
        iMinX,
        iMaxX,
        iMinY,
        iMaxY,
        iNextMaintenance,
        iNextFuelUse,
        iNextTotalPop,
        iNextMin,
        iNextFuel,
        iMaxNumGameMessages,
        iTechDevs,
        iTechUndevs,
        iPartialMapXRadius,
        iPartialMapYRadius,
        iNotepad,
        iDefaultBuilderPlanet,
        iLastBuilderPlanet,
        iMaxEcon,
        iMaxMil,
        iNumNukedAllies,
        iDefaultMessageTarget,
        iLastMessageTargetMask,
        iInitialBridierRank,
        iInitialBridierIndex,
        iGameRatios,
        iMiniMaps,
        iMapFairnessResourcesClaimed,
    };

    static const char* const ColumnNames[] = 
    {
        GameClass,
        GameNumber,
        EmpireKey,
        NumPlanets,
        TotalAg,
        TotalFuel,
        TotalMin,
        TechLevel,
        TotalPop,
        TotalBuild,
        TotalMaintenance,
        TotalFuelUse,
        LastLogin,
        EnterGameIPAddress,
        Options,
        PartialMapCenter,
        NumAvailableTechUndevs,
        Econ,
        Mil,
        TargetPop,
        HomeWorld,
        NumUpdatesIdle,
        MaxBR,
        BonusAg,
        BonusFuel,
        BonusMin,
        NumBuilds,
        MinX,
        MaxX,
        MinY,
        MaxY,
        NextMaintenance,
        NextFuelUse,
        NextTotalPop,
        NextMin,
        NextFuel,
        MaxNumGameMessages,
        TechDevs,
        TechUndevs,
        PartialMapXRadius,
        PartialMapYRadius,
        Notepad,
        DefaultBuilderPlanet,
        LastBuilderPlanet,
        MaxEcon,
        MaxMil,
        NumNukedAllies,
        DefaultMessageTarget,
        LastMessageTargetMask,
        InitialBridierRank,
        InitialBridierIndex,
        GameRatios,
        MiniMaps,
        MapFairnessResourcesClaimed,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        MAX_IP_ADDRESS_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);
    
    static const TemplateDescription Template = {
        "GameEmpireData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


///////////////////////////////
// GameEmpireMessages(I.I.I) //
///////////////////////////////

#define GAME_EMPIRE_MESSAGES "GameEmpireMessages"

#define GET_GAME_EMPIRE_MESSAGES(pszBuffer, i, j, k)                                            \
    char pszBuffer[sizeof("GameEmpireMessages_GameClass__GameNumber__EmpireKey_%i") + 96];      \
    sprintf(pszBuffer, "GameEmpireMessages_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

#define COPY_GAME_EMPIRE_MESSAGES(pszBuffer, i, j, k)            \
    sprintf(pszBuffer, "GameEmpireMessages_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

namespace GameEmpireMessages
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const Unread = "Unread";
    static const char* const SourceKey = "SourceKey";
    static const char* const SourceName = "SourceName";
    static const char* const TimeStamp = "TimeStamp";
    static const char* const Flags = "Flags";
    static const char* const Text = "Text";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iUnread,
        iSourceKey,
        iSourceName,
        iTimeStamp,
        iFlags,
        iText,
    };

    static const char* const ColumnNames[] = 
    {
        GameClass,
        GameNumber,
        EmpireKey,
        Unread,
        SourceKey,
        SourceName,
        TimeStamp,
        Flags,
        Text,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT64,
        V_INT,
        V_STRING,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,        
        0,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "GameEmpireMessages",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        0,
        NULL,
        NULL
    };
};


//////////////////////////
// GameEmpireMap(I.I.I) //
//////////////////////////

#define GAME_EMPIRE_MAP "GameEmpireMap"

#define GET_GAME_EMPIRE_MAP(pszBuffer, i, j, k)                     \
    char pszBuffer[sizeof("GameEmpireMap_GameClass__GameNumber__EmpireKey_%i") + 96];             \
    sprintf(pszBuffer, "GameEmpireMap_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

#define COPY_GAME_EMPIRE_MAP(pszBuffer, i, j, k)             \
    sprintf(pszBuffer, "GameEmpireMap_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

namespace GameEmpireMap
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const PlanetKey = "PlanetKey";
    static const char* const Explored = "Explored";
    static const char* const NumUncloakedShips = "NumUncloakedShips";
    static const char* const NumCloakedBuildShips = "NumCloakedBuildShips";
    static const char* const NumUncloakedBuildShips = "NumUncloakedBuildShips";
    static const char* const NumCloakedShips = "NumCloakedShips";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iPlanetKey,
        iExplored,
        iNumUncloakedShips,
        iNumCloakedBuildShips,
        iNumUncloakedBuildShips,
        iNumCloakedShips,
    };

    static const char* const ColumnNames[] = 
    {
        GameClass,
        GameNumber,
        EmpireKey,
        PlanetKey,
        Explored,
        NumUncloakedShips,
        NumCloakedBuildShips,
        NumUncloakedBuildShips,
        NumCloakedShips,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };
    
    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* const IndexColumns[] = {
        PlanetKey
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "GameEmpireMap",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};


////////////////////////////////
// GameEmpireDiplomacy(I.I.I) //
////////////////////////////////

#define GAME_EMPIRE_DIPLOMACY "GameEmpireDiplomacy"

#define GET_GAME_EMPIRE_DIPLOMACY(pszBuffer, i, j, k)                       \
    char pszBuffer[sizeof("GameEmpireDiplomacy_GameClass__GameNumber__EmpireKey_%i") + 96];               \
    sprintf(pszBuffer, "GameEmpireDiplomacy_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

#define COPY_GAME_EMPIRE_DIPLOMACY(pszBuffer, i, j, k)                   \
    sprintf(pszBuffer, "GameEmpireDiplomacy_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

namespace GameEmpireDiplomacy
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const ReferenceEmpireKey = "ReferenceEmpireKey";
    static const char* const DipOffer = "DipOffer";
    static const char* const CurrentStatus = "CurrentStatus";
    static const char* const DipOfferLastUpdate = "DipOfferLastUpdate";
    static const char* const State = "State";
    static const char* const SubjectiveEcon = "SubjectiveEcon";
    static const char* const SubjectiveMil = "SubjectiveMil";
    static const char* const LastMessageTargetFlag = "LastMessageTargetFlag";

   enum Columns
   {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iReferenceEmpireKey,
        iDipOffer,
        iCurrentStatus,
        iDipOfferLastUpdate,
        iState,
        iSubjectiveEcon,
        iSubjectiveMil,
        iLastMessageTargetFlag,
    };

    static const char* const ColumnNames[] =
    {
        GameClass,
        GameNumber,
        EmpireKey,
        ReferenceEmpireKey,
        DipOffer,
        CurrentStatus,
        DipOfferLastUpdate,
        State,
        SubjectiveEcon,
        SubjectiveMil,
        LastMessageTargetFlag,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* const IndexColumns[] = {
        ReferenceEmpireKey,
        CurrentStatus
    };

    static unsigned int IndexFlags[] = {
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);
    
    static const TemplateDescription Template = {
        "GameEmpireDiplomacy",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};


////////////////////////////
// GameEmpireShips(I.I.I) //
////////////////////////////

#define GAME_EMPIRE_SHIPS "GameEmpireShips"

#define GET_GAME_EMPIRE_SHIPS(pszBuffer, i, j, k)                       \
    char pszBuffer[sizeof("GameEmpireShips_GameClass__GameNumber__EmpireKey_%i") + 96];               \
    sprintf(pszBuffer, "GameEmpireShips_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

#define COPY_GAME_EMPIRE_SHIPS(pszBuffer, i, j, k)                   \
    sprintf(pszBuffer, "GameEmpireShips_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

namespace GameEmpireShips
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const Name = "Name";
    static const char* const Type = "Type";
    static const char* const CurrentBR = "CurrentBR";
    static const char* const MaxBR = "MaxBR";
    static const char* const CurrentPlanet = "CurrentPlanet";
    static const char* const Action = "Action";
    static const char* const FleetKey = "FleetKey";
    static const char* const BuiltThisUpdate = "BuiltThisUpdate";
    static const char* const State = "State";
    static const char* const GateDestination = "GateDestination";
    static const char* const ColonyBuildCost = "ColonyBuildCost";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iName,
        iType,
        iCurrentBR,
        iMaxBR,
        iCurrentPlanet,
        iAction,
        iFleetKey,
        iBuiltThisUpdate,
        iState,
        iGateDestination,
        iColonyBuildCost,
    };

    static const char* const ColumnNames[] =
    {
        GameClass,
        GameNumber,
        EmpireKey,
        Name,
        Type,
        CurrentBR,
        MaxBR,
        CurrentPlanet,
        Action,
        FleetKey,
        BuiltThisUpdate,
        State,
        GateDestination,
        ColonyBuildCost,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_FLOAT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        MAX_SHIP_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* const IndexColumns[] = 
    {
        CurrentPlanet,
        FleetKey
    };

    static unsigned int IndexFlags[] = {
        0,
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);
    
    static const TemplateDescription Template = {
        "GameEmpireShips",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags
    };
};


/////////////////////////////
// GameEmpireFleets(I.I.I) //
/////////////////////////////

#define GAME_EMPIRE_FLEETS "GameEmpireFleets"

#define GET_GAME_EMPIRE_FLEETS(pszBuffer, i, j, k)                      \
    char pszBuffer[sizeof("GameEmpireFleets_GameClass__GameNumber__EmpireKey_%i") + 96];              \
    sprintf(pszBuffer, "GameEmpireFleets_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

#define COPY_GAME_EMPIRE_FLEETS(pszBuffer, i, j, k)                  \
    sprintf(pszBuffer, "GameEmpireFleets_GameClass_%i_GameNumber_%i_EmpireKey_%i", i, j, k);

namespace GameEmpireFleets
{
    static const char* const GameClass = "GameClass";
    static const char* const GameNumber = "GameNumber";
    static const char* const EmpireKey = "EmpireKey";
    static const char* const Name = "Name";
    static const char* const CurrentStrength = "CurrentStrength";
    static const char* const MaxStrength = "MaxStrength";
    static const char* const CurrentPlanet = "CurrentPlanet";
    static const char* const Action = "Action";
    static const char* const Flags = "Flags";

    enum Columns
    {
        iGameClass,
        iGameNumber,
        iEmpireKey,
        iName,
        iCurrentStrength,
        iMaxStrength,
        iCurrentPlanet,
        iAction,
        iFlags,
    };

    static const char* const ColumnNames[] = 
    {
        GameClass,
        GameNumber,
        EmpireKey,
        Name,
        CurrentStrength,
        MaxStrength,
        CurrentPlanet,
        Action,
        Flags,
    };
    
    static const VariantType Types[] = 
    {
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_FLOAT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
    };
    
    static const unsigned int Sizes[] = 
    {
        0,
        0,
        0,
        MAX_FLEET_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* const IndexColumns[] = {
        CurrentPlanet
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = countof(Sizes);

    static const TemplateDescription Template = {
        "GameEmpireFleets",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        countof(IndexColumns),
        (char**)IndexColumns,
        IndexFlags,
    };
};
