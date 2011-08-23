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

#ifndef _GameEngineSchema_H_
#define _GameEngineSchema_H_

#include "Osal/Variant.h"

#include "SqlDatabase.h"

#include "GameEngineStrings.h"

////////////////
// SystemData //
////////////////

#define SYSTEM_DATA "SystemData"

namespace SystemData {
    
    static const char* const DefaultAlien = "DefaultAlien";
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
    static const char* const LastShutdownTimeUNUSED = "LastShutdownTimeUNUSED";
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
    static const char* const AdminEmail = "AdminEmail";
    static const char* const BuilderBRDampener = "BuilderBRDampener";

    enum Columns {
        iDefaultAlien,
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
        iLastShutdownTimeUNUSED,
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
        iAdminEmail,
        iBuilderBRDampener,
    };

    static const char* const ColumnNames[] = {
        DefaultAlien,
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
        LastShutdownTimeUNUSED,
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
        AdminEmail,
        BuilderBRDampener,
    };

    static const VariantType Types[] = {
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
        V_INT64,
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
        V_STRING,
        V_FLOAT,
    };
    
    static const unsigned int Sizes[] = {
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
        VARIABLE_LENGTH_STRING,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        true,
        0,
        NULL,
        1
    };
};


//////////////////////
// SystemEmpireData //
//////////////////////

#define SYSTEM_EMPIRE_DATA "SystemEmpireData"

#define GET_SYSTEM_EMPIRE_DATA(pszBuffer, i)       \
    char pszBuffer [sizeof("SystemEmpireData") + 32]; \
    sprintf (pszBuffer, "SystemEmpireData%i", i);

namespace SystemEmpireData {

    static const char* const Name = "Name";
    static const char* const Password = "Password";
    static const char* const Privilege = "Privilege";
    static const char* const RealName = "RealName";
    static const char* const Email = "Email";
    static const char* const WebPage = "WebPage";
    static const char* const Quote = "Quote";
    static const char* const AlienKey = "AlienKey";
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
    static const char* const Associations = "Associations";

    enum Columns {
        iName,
        iPassword,
        iPrivilege,
        iRealName,
        iEmail,
        iWebPage,
        iQuote,
        iAlienKey,
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
        iAssociations,
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
        Associations,
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
        V_STRING,
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
        VARIABLE_LENGTH_STRING,
    };

    static const char* IndexColumns[] = {
        Name,
    };

    static unsigned int IndexFlags[] = {
        INDEX_UNIQUE_DATA,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemEmpireData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        1,
        (char**)IndexColumns,
        3000,
        IndexFlags,
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

    static const char* IndexColumns[] = {
        Owner
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemGameClassData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        1,
        (char**)IndexColumns,
        200,
        IndexFlags,
    };
};


//////////////////////
// SystemAlienIcons //
//////////////////////

#define SYSTEM_ALIEN_ICONS "SystemAlienIcons"

namespace SystemAlienIcons {

    static const char* const AlienKey = "AlienKey";
    static const char* const AuthorName = "AuthorName";

    enum Columns {      
        iAlienKey,
        iAuthorName
    };

    static const char* const ColumnNames[] = {
        AlienKey,
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

    static const char* IndexColumns[] = {
        AlienKey
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemAlienIcons",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        1,
        (char**)IndexColumns,
        150,
        IndexFlags,
    };
};


///////////////////////////////
// SystemSystemGameClassData //
///////////////////////////////

#define SYSTEM_SYSTEM_GAMECLASS_DATA "SystemSystemGameClassData"

namespace SystemSystemGameClassData {

    static const char* const GameClass = "GameClass";

    /*enum Columns {
        GameClass
    };*/

    static const char* const ColumnNames[] = {
        GameClass
    };

    static const VariantType Types[] = {
        V_INT
    };

    static const unsigned int Sizes[] = {
        0
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemSystemGameClassData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        30,
        NULL,
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

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemSuperClassData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20
    };
};


//////////////////
// SystemThemes //
//////////////////

#define SYSTEM_THEMES "SystemThemes"

namespace SystemThemes {

    static const char* Name = "Name";
    static const char* AuthorName = "AuthorName";
    static const char* Version = "Version";
    static const char* AuthorEmail = "AuthorEmail";
    static const char* Description = "Description";
    static const char* FileName = "FileName";
    static const char* Options = "Options";
    static const char* TableColor = "TableColor";
    static const char* TextColor = "TextColor";
    static const char* GoodColor = "GoodColor";
    static const char* BadColor = "BadColor";
    static const char* PrivateMessageColor = "PrivateMessageColor";
    static const char* BroadcastMessageColor = "BroadcastMessageColor";

    enum Columns {
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

    static const char* const ColumnNames[] = {
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
    
    static const VariantType Types[] = {
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

    static const unsigned int Sizes[] = {
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

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemThemes",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};


///////////////////////
// SystemActiveGames //
///////////////////////

#define SYSTEM_ACTIVE_GAMES "SystemActiveGames"

namespace SystemActiveGames {

    static const char* const GameClassGameNumber = "GameClassGameNumber";
    static const char* const State = "State";

    enum Columns {
        iGameClassGameNumber,
        iState
    };

    static const char* const ColumnNames[] = {
        GameClassGameNumber,
        State,
    };
    
    static const VariantType Types[] = {
        V_STRING,
        V_INT,
    };

    static const unsigned int Sizes[] = {
        VARIABLE_LENGTH_STRING,
        0,
    };

    static const char* IndexColumns[] = {
        GameClassGameNumber
    };

    static unsigned int IndexFlags[] = {
        INDEX_CASE_SENSITIVE,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemActiveGames",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        1,
        (char**)IndexColumns,
        100,
        IndexFlags,
    };
};


/////////////////////////////
// SystemEmpireMessages(I) //
/////////////////////////////

#define SYSTEM_EMPIRE_MESSAGES(pszBuffer, i)                \
                                                            \
    char pszBuffer [sizeof ("SystemEmpireMessages") + 32];  \
    sprintf (pszBuffer, "SystemEmpireMessages%i", i);

#define GET_SYSTEM_EMPIRE_MESSAGES(pszBuffer, i)            \
                                                            \
    sprintf (pszBuffer, "SystemEmpireMessages%i", i);

namespace SystemEmpireMessages {

    static const char* const Unread = "Unread";
    static const char* const Source = "Source";
    static const char* const TimeStamp = "TimeStamp";
    static const char* const Flags = "Flags";
    static const char* const Text = "Text";
    static const char* const Type = "Type";
    static const char* const Data = "Data";

    enum Columns {
        iUnread,
        iSource,
        iTimeStamp,
        iFlags,
        iText,
        iType,
        iData,
    };

    static const char* const ColumnNames[] = {
        Unread,
        Source,
        TimeStamp,
        Flags,
        Text,
        Type,
        Data,
    };
    
    static const VariantType Types[] = {
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
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemEmpireMessages",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        10,
        NULL,
    };
};


/////////////////////////////
// SystemEmpireNukeList(I) //
/////////////////////////////

#define SYSTEM_EMPIRE_NUKED_LIST(pszBuffer, i)              \
                                                            \
    char pszBuffer [sizeof ("SystemEmpireNukedList") + 32]; \
    sprintf (pszBuffer, "SystemEmpireNukedList%i", i);

#define GET_SYSTEM_EMPIRE_NUKED_LIST(pszBuffer, i)          \
                                                            \
    sprintf (pszBuffer, "SystemEmpireNukedList%i", i);

#define SYSTEM_EMPIRE_NUKER_LIST(pszBuffer, i)              \
                                                            \
    char pszBuffer [sizeof ("SystemEmpireNukerList") + 32]; \
    sprintf (pszBuffer, "SystemEmpireNukerList%i", i);

#define GET_SYSTEM_EMPIRE_NUKER_LIST(pszBuffer, i)          \
                                                            \
    sprintf (pszBuffer, "SystemEmpireNukerList%i", i);

namespace SystemEmpireNukeList
{
    static const char* AlienKey = "AlienKey";
    static const char* EmpireName = "EmpireName";
    static const char* EmpireKey = "EmpireKey";
    static const char* GameClassName = "GameClassName";
    static const char* GameNumber = "GameNumber";
    static const char* TimeStamp = "TimeStamp";

    enum Columns {
        iAlienKey,
        iEmpireName,
        iEmpireKey,
        iGameClassName,
        iGameNumber,
        iTimeStamp,
    };

    static const char* const ColumnNames[] = {
        AlienKey,
        EmpireName,
        EmpireKey,
        GameClassName,
        GameNumber,
        TimeStamp,
    };

    static const VariantType Types[] = {
        V_INT,
        V_STRING,
        V_INT,
        V_STRING,
        V_INT,
        V_INT64,
    };

    static const unsigned int Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemEmpireNukeList",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

////////////////////
// SystemNukeList //
////////////////////

#define SYSTEM_NUKE_LIST "SystemNukeList"

namespace SystemNukeList {

    static const char* NukerAlienKey = "NukerAlienKey";
    static const char* NukerEmpireName = "NukerEmpireName";
    static const char* NukerEmpireKey = "NukerEmpireKey";
    static const char* NukedAlienKey = "NukedAlienKey";
    static const char* NukedEmpireName = "NukedEmpireName";
    static const char* NukedEmpireKey = "NukedEmpireKey";
    static const char* GameClassName = "GameClassName";
    static const char* GameNumber = "GameNumber";
    static const char* TimeStamp = "TimeStamp";

    enum Columns {
        iNukerAlienKey,
        iNukerEmpireName,
        iNukerEmpireKey,
        iNukedAlienKey,
        iNukedEmpireName,
        iNukedEmpireKey,
        iGameClassName,
        iGameNumber,
        iTimeStamp,
    };

    static const char* const ColumnNames[] = {
        NukerAlienKey,
        NukerEmpireName,
        NukerEmpireKey,
        NukedAlienKey,
        NukedEmpireName,
        NukedEmpireKey,
        GameClassName,
        GameNumber,
        TimeStamp,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_STRING,
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
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemNukeList",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};


///////////////////////
// SystemLatestGames //
///////////////////////

#define SYSTEM_LATEST_GAMES "SystemLatestGames"

namespace SystemLatestGames {

    static const char* const Name = "Name";
    static const char* const Number = "Number";
    static const char* const Created = "Created";
    static const char* const Ended = "Ended";
    static const char* const Updates = "Updates";
    static const char* const Result = "Result";
    static const char* const Winners = "Winners";
    static const char* const Losers = "Losers";

    enum Columns {
        iName,
        iNumber,
        iCreated,
        iEnded,
        iUpdates,
        iResult,
        iWinners,
        iLosers,
    };

    static const char* const ColumnNames[] = {
        Name,
        Number,
        Created,
        Ended,
        Updates,
        Result,
        Winners,
        Losers,
    };
    
    static const VariantType Types[] = {
        V_STRING,
        V_INT,
        V_INT64,
        V_INT64,
        V_INT,
        V_INT,
        V_STRING,
        V_STRING,
    };

    static const unsigned int Sizes[] = {
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemLatestGames",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

/////////////////////////////
// SystemEmpireActiveGames //
/////////////////////////////

#define SYSTEM_EMPIRE_ACTIVE_GAMES "SystemEmpireActiveGames"

#define GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszBuffer, i)       \
    char pszBuffer [sizeof("SystemEmpireActiveGames_EmpireKey_") + 32]; \
    sprintf (pszBuffer, "SystemEmpireActiveGames_EmpireKey_%i", i);

namespace SystemEmpireActiveGames
{
    static const char* const EmpireKey = "EmpireKey";
    static const char* const GameClassGameNumber = "GameClassGameNumber";

    enum Columns
    {
        iEmpireKey,
        iGameClassGameNumber
    };

    static const char* const ColumnNames[] =
    {
        EmpireKey,
        GameClassGameNumber
    };

    static const VariantType Types[] =
    {
        V_INT,
        V_STRING
    };

    static const unsigned int Sizes[] = {
        0,
        VARIABLE_LENGTH_STRING
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template =
    {
        "SystemEmpireActiveGames",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

///////////////////////
// SystemTournaments //
///////////////////////

#define SYSTEM_TOURNAMENTS "SystemTournaments"

namespace SystemTournaments {

    static const char* const Name = "Name";
    static const char* const Description = "Description";
    static const char* const WebPage = "WebPage";
    static const char* const News = "News";
    static const char* const Owner = "Owner";
    static const char* const Icon = "Icon";
    static const char* const OwnerName = "OwnerName";

    enum Columns {
        iName,
        iDescription,
        iWebPage,
        iNews,
        iOwner,
        iIcon,
        iOwnerName,
    };

    static const char* const ColumnNames[] = {
        Name,
        Description,
        WebPage,
        News,
        Owner,
        Icon,
        OwnerName,
    };

    static const VariantType Types[] = {
        V_STRING,
        V_STRING,
        V_STRING,
        V_STRING,
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
        MAX_EMPIRE_NAME_LENGTH,
    };

    static const char* IndexColumns[] = {
        Owner
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemTournaments",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        (char**)IndexColumns,
        20,
        IndexFlags,
    };
};

//////////////////////////////
// SystemTournamentTeams(I) //
//////////////////////////////

#define SYSTEM_TOURNAMENT_TEAMS(pszBuffer, i)                   \
                                                                \
    char pszBuffer [sizeof ("SystemTournamentTeams") + 32];     \
    sprintf (pszBuffer, "SystemTournamentTeams%i", i);

#define GET_SYSTEM_TOURNAMENT_TEAMS(pszBuffer, i)           \
                                                                \
    sprintf (pszBuffer, "SystemTournamentTeams%i", i);

namespace SystemTournamentTeams {

    static const char* Name = "Name";
    static const char* Description = "Description";
    static const char* WebPage = "WebPage";
    static const char* Icon = "Icon";
    static const char* Wins = "Wins";
    static const char* Nukes = "Nukes";
    static const char* Nuked = "Nuked";
    static const char* Draws = "Draws";
    static const char* Ruins = "Ruins";

    enum Columns {
        iName,
        iDescription,
        iWebPage,
        iIcon,
        iWins,
        iNukes,
        iNuked,
        iDraws,
        iRuins,
    };

    static const char* const ColumnNames[] = {
        Name,
        Description,
        WebPage,
        Icon,
        Wins,
        Nukes,
        Nuked,
        Draws,
        Ruins,
    };

    static const VariantType Types[] = {
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = {
        MAX_TOURNAMENT_TEAM_NAME_LENGTH,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemTournamentTeams",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

////////////////////////////////
// SystemTournamentEmpires(I) //
////////////////////////////////

#define SYSTEM_TOURNAMENT_EMPIRES(pszBuffer, i)             \
                                                                \
    char pszBuffer [sizeof ("SystemTournamentEmpires") + 32];   \
    sprintf (pszBuffer, "SystemTournamentEmpires%i", i);

#define GET_SYSTEM_TOURNAMENT_EMPIRES(pszBuffer, i)         \
                                                                \
    sprintf (pszBuffer, "SystemTournamentEmpires%i", i);

namespace SystemTournamentEmpires {

    static const char* EmpireKey = "EmpireKey";
    static const char* TeamKey = "TeamKey";
    static const char* Wins = "Wins";
    static const char* Nukes = "Nukes";
    static const char* Nuked = "Nuked";
    static const char* Draws = "Draws";
    static const char* Ruins = "Ruins";

    enum Columns {
        iEmpireKey,
        iTeamKey,
        iWins,
        iNukes,
        iNuked,
        iDraws,
        iRuins,
    };

    static const char* const ColumnNames[] = {
        EmpireKey,
        TeamKey,
        Wins,
        Nukes,
        Nuked,
        Draws,
        Ruins,
    };

    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemTournamentEmpires",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

////////////////////////////////////
// SystemTournamentActiveGames(I) //
////////////////////////////////////

#define SYSTEM_TOURNAMENT_ACTIVE_GAMES(pszBuffer, i)                \
                                                                    \
    char pszBuffer [sizeof ("SystemTournamentActiveGames") + 32];   \
    sprintf (pszBuffer, "SystemTournamentActiveGames%i", i);

#define GET_SYSTEM_TOURNAMENT_ACTIVE_GAMES(pszBuffer, i)            \
                                                                    \
    sprintf (pszBuffer, "SystemTournamentActiveGames%i", i);

namespace SystemTournamentActiveGames {

    static const char* const GameClassGameNumber = "GameClassGameNumber";

    enum Columns {
        iGameClassGameNumber,
    };

    static const char* const ColumnNames[] = {
        GameClassGameNumber,
    };

    static const VariantType Types[] = {
        V_STRING,
    };

    static const unsigned int Sizes[] = {
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemTournamentActiveGames",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        10,
        NULL,
    };
};

////////////////////////////////////
// SystemTournamentLatestGames(I) //
////////////////////////////////////

#define SYSTEM_TOURNAMENT_LATEST_GAMES(pszBuffer, i)                \
                                                                    \
    char pszBuffer [sizeof ("SystemTournamentLatestGames") + 32];   \
    sprintf (pszBuffer, "SystemTournamentLatestGames%i", i);

#define GET_SYSTEM_TOURNAMENT_LATEST_GAMES(pszBuffer, i)            \
                                                                    \
    sprintf (pszBuffer, "SystemTournamentLatestGames%i", i);

#define SystemTournamentLatestGames SystemLatestGames

////////////////////////////////
// SystemEmpireTournaments(I) //
////////////////////////////////

#define SYSTEM_EMPIRE_TOURNAMENTS(pszBuffer, i)             \
                                                                \
    char pszBuffer [sizeof ("SystemEmpireTournaments") + 32];   \
    sprintf (pszBuffer, "SystemEmpireTournaments%i", i);

#define GET_SYSTEM_EMPIRE_TOURNAMENTS(pszBuffer, i)         \
                                                                \
    sprintf (pszBuffer, "SystemEmpireTournaments%i", i);

namespace SystemEmpireTournaments {
    
    static const char* const TournamentKey = "TournamentKey";

    enum Columns {
        iTournamentKey,
    };

    static const char* const ColumnNames[] = {
        TournamentKey,
    };

    static const VariantType Types[] = {
        V_INT,
    };

    static const unsigned int Sizes[] = {
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemEmpireTournaments",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        10,
        NULL,
    };
};

///////////////////
// GameData(I.I) //
///////////////////

#define _GAME_DATA "GameData"

#define GAME_DATA(pszBuffer, i, j)              \
                                                \
    char pszBuffer [sizeof ("GameData") + 64];  \
    sprintf (pszBuffer, "GameData%i.%i", i, j);

#define GET_GAME_DATA(pszBuffer, i, j)          \
                                                \
    sprintf (pszBuffer, "GameData%i.%i", i, j);

namespace GameData {

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
    static const char* const LastUpdateCheck = "LastUpdateCheck";
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

    enum Columns {
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
        iLastUpdateCheck,
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

    static const char* const ColumnNames[] = {
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
        LastUpdateCheck,
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

    static const VariantType Types[] = {
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

    static const unsigned int Sizes[] = {
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
    
    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        true,
        0,
        NULL,
        1,
        NULL,
    };
};


///////////////////////
// GameSecurity(I.I) //
///////////////////////

#define GAME_SECURITY(pszBuffer, i, j)              \
                                                \
    char pszBuffer [sizeof ("GameSecurity") + 64];  \
    sprintf (pszBuffer, "GameSecurity%i.%i", i, j);

#define GET_GAME_SECURITY(pszBuffer, i, j)          \
                                                \
    sprintf (pszBuffer, "GameSecurity%i.%i", i, j);

namespace GameSecurity {

    static const char* EmpireKey = "EmpireKey";
    static const char* Name = "Name";
    static const char* Options = "Options";
    static const char* SessionId = "SessionId";
    static const char* IPAddress = "IPAddress";
    static const char* SecretKey = "SecretKey";

    enum Columns {
        iEmpireKey,
        iName,
        iOptions,
        iSessionId,
        iIPAddress,
        iSecretKey,
    };

    static const char* const ColumnNames[] = {
        EmpireKey,
        Name,
        Options,
        SessionId,
        IPAddress,
        SecretKey,
    };

    static const VariantType Types[] = {
        V_INT,
        V_STRING,
        V_INT,
        V_INT64,
        V_STRING,
        V_INT64,
    };
    
    static const unsigned int Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        MAX_IP_ADDRESS_LENGTH,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameSecurity",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        2,
        NULL,
    };
};

//////////////////////
// GameEmpires(I.I) //
//////////////////////

#define GAME_EMPIRES(pszBuffer, i, j)               \
                                                    \
    char pszBuffer [sizeof ("GameEmpires") + 64];   \
    sprintf (pszBuffer, "GameEmpires%i.%i", i, j);

#define GET_GAME_EMPIRES(pszBuffer, i, j)           \
                                                    \
    sprintf (pszBuffer, "GameEmpires%i.%i", i, j);

namespace GameEmpires {

    static const char* const EmpireKey = "EmpireKey";
    static const char* const EmpireName = "EmpireName";

    enum Columns {
        iEmpireKey,
        iEmpireName,
    };

    static const char* const ColumnNames[] = {
        EmpireKey,
        EmpireName,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_STRING,
    };

    static const unsigned int Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
    };
    
    static const char* IndexColumns[] = {
        EmpireKey
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameEmpires",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        1,
        (char**)IndexColumns,
        20,
        IndexFlags,
    };
};

//////////////////////////
// GameDeadEmpires(I.I) //
//////////////////////////

#define GAME_DEAD_EMPIRES(pszBuffer, i, j)              \
                                                        \
    char pszBuffer [sizeof ("GameDeadEmpires") + 64];   \
    sprintf (pszBuffer, "GameDeadEmpires%i.%i", i, j);

#define GET_GAME_DEAD_EMPIRES(pszBuffer, i, j)          \
                                                        \
    sprintf (pszBuffer, "GameDeadEmpires%i.%i", i, j);

namespace GameDeadEmpires {

    static const char* const Name = "Name";
    static const char* const Key = "Key";
    static const char* const Icon = "Icon";
    static const char* const Update = "Update";
    static const char* const Reason = "Reason";
    static const char* const SecretKey = "SecretKey";

    enum Columns {
        iName,
        iKey,
        iIcon,
        iUpdate,
        iReason,
        iSecretKey,
    };

    static const char* const ColumnNames[] = {
        Name,
        Key,
        Icon,
        Update,
        Reason,
        SecretKey,
    };

    static const VariantType Types[] = {
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
    };

    static const unsigned int Sizes[] = {
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameDeadEmpires",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        10,
        NULL,
    };
};


//////////////////
// GameMap(I.I) //
//////////////////

#define GAME_MAP(pszBuffer, i, j)                   \
                                                    \
    char pszBuffer [sizeof ("GameMap") + 64];       \
    sprintf (pszBuffer, "GameMap%i.%i", i, j);

#define GET_GAME_MAP(pszBuffer, i, j)               \
                                                    \
    sprintf (pszBuffer, "GameMap%i.%i", i, j);

namespace GameMap {
    
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

    enum Columns {
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

    static const char* const ColumnNames[] = {
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
    
    static const VariantType Types[] = {
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

    static const unsigned int Sizes[] = {
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

    static const char* IndexColumns[] = {
        Owner,
        Coordinates
    };

    static unsigned int IndexFlags[] = {
        0,
        INDEX_CASE_SENSITIVE,
    };
    
    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameMap",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        countof (IndexColumns),
        (char**)IndexColumns,
        100,
        IndexFlags,
    };
};

///////////////////////////
// GameEmpireData(I.I.I) //
///////////////////////////

#define _GAME_EMPIRE_DATA "GameEmpireData"

#define GAME_EMPIRE_DATA(pszBuffer, i, j, k)                    \
                                                                \
    char pszBuffer [sizeof ("GameEmpireData") + 96];            \
    sprintf (pszBuffer, "GameEmpireData%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_DATA(pszBuffer, i, j, k)                \
                                                                \
    sprintf (pszBuffer, "GameEmpireData%i.%i.%i", i, j, k);

namespace GameEmpireData {

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
    static const char* const NumAlliances = "NumAlliances";
    static const char* const NumTruces = "NumTruces";
    static const char* const MaxNumGameMessages = "MaxNumGameMessages";
    static const char* const TechDevs = "TechDevs";
    static const char* const TechUndevs = "TechUndevs";
    static const char* const NumTrades = "NumTrades";
    static const char* const PartialMapXRadius = "PartialMapXRadius";
    static const char* const PartialMapYRadius = "PartialMapYRadius";
    static const char* const Notepad = "Notepad";
    static const char* const DefaultBuilderPlanet = "DefaultBuilderPlanet";
    static const char* const LastBuilderPlanet = "LastBuilderPlanet";
    static const char* const MaxEcon = "MaxEcon";
    static const char* const MaxMil = "MaxMil";
    static const char* const NumAlliancesLeaked = "NumAlliancesLeaked";
    static const char* const DefaultMessageTarget = "DefaultMessageTarget";
    static const char* const LastMessageTargetMask = "LastMessageTargetMask";
    static const char* const InitialBridierRank = "InitialBridierRank";
    static const char* const InitialBridierIndex = "InitialBridierIndex";
    static const char* const GameRatios = "GameRatios";
    static const char* const MiniMaps = "MiniMaps";
    static const char* const MapFairnessResourcesClaimed = "MapFairnessResourcesClaimed";

    enum Columns {
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
        iNumAlliances,
        iNumTruces,
        iMaxNumGameMessages,
        iTechDevs,
        iTechUndevs,
        iNumTrades,
        iPartialMapXRadius,
        iPartialMapYRadius,
        iNotepad,
        iDefaultBuilderPlanet,
        iLastBuilderPlanet,
        iMaxEcon,
        iMaxMil,
        iNumAlliancesLeaked,
        iDefaultMessageTarget,
        iLastMessageTargetMask,
        iInitialBridierRank,
        iInitialBridierIndex,
        iGameRatios,
        iMiniMaps,
        iMapFairnessResourcesClaimed,
    };

    static const char* const ColumnNames[] = {
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
        NumAlliances,
        NumTruces,
        MaxNumGameMessages,
        TechDevs,
        TechUndevs,
        NumTrades,
        PartialMapXRadius,
        PartialMapYRadius,
        Notepad,
        DefaultBuilderPlanet,
        LastBuilderPlanet,
        MaxEcon,
        MaxMil,
        NumAlliancesLeaked,
        DefaultMessageTarget,
        LastMessageTargetMask,
        InitialBridierRank,
        InitialBridierIndex,
        GameRatios,
        MiniMaps,
        MapFairnessResourcesClaimed,
    };
    
    static const VariantType Types[] = {
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

    static const unsigned int Sizes[] = {
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

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);
    
    static const TemplateDescription Template = {
        "GameEmpireData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        true,
        0,
        NULL,
        1,
        NULL,
    };
};


///////////////////////////////
// GameEmpireMessages(I.I.I) //
///////////////////////////////

#define GAME_EMPIRE_MESSAGES(pszBuffer, i, j, k)                \
                                                                \
    char pszBuffer [sizeof ("GameEmpireMessages") + 96];        \
    sprintf (pszBuffer, "GameEmpireMessages%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_MESSAGES(pszBuffer, i, j, k)            \
                                                                \
    sprintf (pszBuffer, "GameEmpireMessages%i.%i.%i", i, j, k);

namespace GameEmpireMessages {
    
    static const char* Unread = "Unread";
    static const char* Source = "Source";
    static const char* TimeStamp = "TimeStamp";
    static const char* Flags = "Flags";
    static const char* Text = "Text";

    enum Columns {
        iUnread,
        iSource,
        iTimeStamp,
        iFlags,
        iText,
    };

    static const char* const ColumnNames[] = {
        Unread,
        Source,
        TimeStamp,
        Flags,
        Text,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_STRING,
        V_INT64,
        V_INT,
        V_STRING,
    };

    static const unsigned int Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameEmpireMessages",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};


//////////////////////////
// GameEmpireMap(I.I.I) //
//////////////////////////

#define GAME_EMPIRE_MAP(pszBuffer, i, j, k)                     \
                                                                \
    char pszBuffer [sizeof ("GameEmpireMap") + 96];             \
    sprintf (pszBuffer, "GameEmpireMap%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_MAP(pszBuffer, i, j, k)             \
                                                                \
    sprintf (pszBuffer, "GameEmpireMap%i.%i.%i", i, j, k);

namespace GameEmpireMap {
    
    static const char* const PlanetKey = "PlanetKey";
    static const char* const Explored = "Explored";
    static const char* const NumUncloakedShips = "NumUncloakedShips";
    static const char* const NumCloakedBuildShips = "NumCloakedBuildShips";
    static const char* const NumUncloakedBuildShips = "NumUncloakedBuildShips";
    static const char* const NumCloakedShips = "NumCloakedShips";

    enum Columns {
        iPlanetKey,
        iExplored,
        iNumUncloakedShips,
        iNumCloakedBuildShips,
        iNumUncloakedBuildShips,
        iNumCloakedShips,
    };

    static const char* const ColumnNames[] = {
        PlanetKey,
        Explored,
        NumUncloakedShips,
        NumCloakedBuildShips,
        NumUncloakedBuildShips,
        NumCloakedShips,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };
    
    static const unsigned int Sizes[] = {
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* IndexColumns[] = {
        PlanetKey
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameEmpireMap",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        1,
        (char**)IndexColumns,
        50,
        IndexFlags,
    };
};


////////////////////////////////
// GameEmpireDiplomacy(I.I.I) //
////////////////////////////////

#define GAME_EMPIRE_DIPLOMACY(pszBuffer, i, j, k)                       \
                                                                        \
    char pszBuffer [sizeof ("GameEmpireDiplomacy") + 96];               \
    sprintf (pszBuffer, "GameEmpireDiplomacy%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_DIPLOMACY(pszBuffer, i, j, k)                   \
                                                                        \
    sprintf (pszBuffer, "GameEmpireDiplomacy%i.%i.%i", i, j, k);

namespace GameEmpireDiplomacy {
    
    static const char* const EmpireKey = "EmpireKey";
    static const char* const DipOffer = "DipOffer";
    static const char* const CurrentStatus = "CurrentStatus";
    static const char* const VirtualStatus = "VirtualStatus";
    static const char* const State = "State";
    static const char* const SubjectiveEcon = "SubjectiveEcon";
    static const char* const SubjectiveMil = "SubjectiveMil";
    static const char* const LastMessageTargetFlag = "LastMessageTargetFlag";

   enum Columns {
        iEmpireKey,
        iDipOffer,
        iCurrentStatus,
        iVirtualStatus,
        iState,
        iSubjectiveEcon,
        iSubjectiveMil,
        iLastMessageTargetFlag,
    };

    static const char* const ColumnNames[] = {
        EmpireKey,
        DipOffer,
        CurrentStatus,
        VirtualStatus,
        State,
        SubjectiveEcon,
        SubjectiveMil,
        LastMessageTargetFlag,
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };

    static const unsigned int Sizes[] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* IndexColumns[] = {
        EmpireKey,
        CurrentStatus
    };

    static unsigned int IndexFlags[] = {
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);
    
    static const TemplateDescription Template = {
        "GameEmpireDiplomacy",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        sizeof (IndexColumns) / sizeof (IndexColumns[0]),
        (char**)IndexColumns,
        10,
        IndexFlags,
    };
};


////////////////////////////
// GameEmpireShips(I.I.I) //
////////////////////////////

#define GAME_EMPIRE_SHIPS(pszBuffer, i, j, k)                       \
                                                                    \
    char pszBuffer [sizeof ("GameEmpireShips") + 96];               \
    sprintf (pszBuffer, "GameEmpireShips%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_SHIPS(pszBuffer, i, j, k)                   \
                                                                    \
    sprintf (pszBuffer, "GameEmpireShips%i.%i.%i", i, j, k);

namespace GameEmpireShips {
    
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

    enum Columns {
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

    static const char* const ColumnNames[] = {
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
    
    static const VariantType Types[] = {
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

    static const unsigned int Sizes[] = {
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

    static const char* IndexColumns[] = {
        CurrentPlanet,
        FleetKey
    };

    static unsigned int IndexFlags[] = {
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);
    
    static const TemplateDescription Template = {
        "GameEmpireShips",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        2,
        (char**)IndexColumns,
        50,
        IndexFlags,
    };
};


/////////////////////////////
// GameEmpireFleets(I.I.I) //
/////////////////////////////

#define GAME_EMPIRE_FLEETS(pszBuffer, i, j, k)                      \
                                                                    \
    char pszBuffer [sizeof ("GameEmpireFleets") + 96];              \
    sprintf (pszBuffer, "GameEmpireFleets%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_FLEETS(pszBuffer, i, j, k)                  \
                                                                    \
    sprintf (pszBuffer, "GameEmpireFleets%i.%i.%i", i, j, k);

namespace GameEmpireFleets {
    
    static const char* const Name = "Name";
    static const char* const NumShips = "NumShips";
    static const char* const CurrentStrength = "CurrentStrength";
    static const char* const MaxStrength = "MaxStrength";
    static const char* const CurrentPlanet = "CurrentPlanet";
    static const char* const Action = "Action";
    static const char* const BuildShips = "BuildShips";
    static const char* const Flags = "Flags";

    enum Columns {
        iName,
        iNumShips,
        iCurrentStrength,
        iMaxStrength,
        iCurrentPlanet,
        iAction,
        iBuildShips,
        iFlags,
    };

    static const char* const ColumnNames[] = {
        Name,
        NumShips,
        CurrentStrength,
        MaxStrength,
        CurrentPlanet,
        Action,
        BuildShips,
        Flags,
    };
    
    static const VariantType Types[] = {
        V_STRING,
        V_INT,
        V_FLOAT,
        V_FLOAT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
    };
    
    static const unsigned int Sizes[] = {
        MAX_FLEET_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const char* IndexColumns[] = {
        CurrentPlanet
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameEmpireFleets",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        1,
        (char**)IndexColumns,
        10,
        IndexFlags,
    };
};


//////////////////////////
// GameIndependentShips //
//////////////////////////

#define GAME_INDEPENDENT_SHIPS(pszBuffer, i, j)             \
                                                \
    char pszBuffer [sizeof ("GameIndependentShips") + 64];  \
    sprintf (pszBuffer, "GameIndependentShips%i.%i", i, j);

#define GET_GAME_INDEPENDENT_SHIPS(pszBuffer, i, j)         \
                                                \
    sprintf (pszBuffer, "GameIndependentShips%i.%i", i, j);

namespace GameIndependentShips {
    
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

    /*enum Columns {
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
    };*/

    static const char* const ColumnNames[] = {
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
    };
    
    static const VariantType Types[] = {
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
    };

    static const unsigned int Sizes[] = {
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
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "GameIndependentShips",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};


///////////////
// Top Lists //
///////////////

namespace TopList {

    static const char* EmpireKey = "EmpireKey";
    static const char* Data = "Data";
    static const char* Data2 = "Data2";

    enum Columns {
        iEmpireKey,
        iData,
        iData2,
    };

    static const char* const ColumnNames[] = {
        EmpireKey,
        Data,
        Data2,
    };
    
    static const unsigned int MaxNumColumns = countof(ColumnNames);
};

#define SYSTEM_ALMONASTER_SCORE_TOPLIST "SystemAlmonasterScoreTopList"

namespace SystemAlmonasterScoreTopList {
    
    enum Columns {
        EmpireKey,
        Data,
    };

    static const char* const ColumnNames[] = {
        "EmpireKey",
        "Data",
    };

    static const VariantType Types[] = {
        V_INT,
        V_FLOAT,
    };
    
    static const unsigned int Sizes[] = {
        0,
        0
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemAlmonasterScoreTopList",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};


#define SYSTEM_CLASSIC_SCORE_TOPLIST "SystemClassicScoreTopList"

namespace SystemClassicScoreTopList {
    
    enum Columns {
        EmpireKey,
        Data,
    };

    static const char* const ColumnNames[] = {
        "EmpireKey",
        "Data",
    };

    static const VariantType Types[] = {
        V_INT,
        V_FLOAT,
    };
    
    static const unsigned int Sizes[] = {
        0,
        0
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemClassicScoreTopList",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

#define SYSTEM_BRIDIER_SCORE_TOPLIST "SystemBridierScoreTopList"

namespace SystemBridierScoreTopList {
    
    enum Columns {
        EmpireKey,
        Rank,
        Index,
    };

    static const char* const ColumnNames[] = {
        "EmpireKey",
        "Rank",
        "Index",
    };

    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_INT,
    };
    
    static const unsigned int Sizes[] = {
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemBridierScoreTopList",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

#define SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST "SystemBridierScoreEstablishedTopList"

namespace SystemBridierScoreEstablishedTopList {
    
    enum Columns {
        EmpireKey,
        Rank,
        Index,
    };

    static const char* const ColumnNames[] = {
        "EmpireKey",
        "Rank",
        "Index",
    };

    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_INT,
    };
    
    static const unsigned int Sizes[] = {
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemBridierScoreEstablishedTopList",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

////////////////////////
// SystemChatroomData //
////////////////////////

#define SYSTEM_CHATROOM_DATA "SystemChatroomData"

namespace SystemChatroomData {
        
    static const char* Flags = "Flags";
    static const char* Time = "Time";
    static const char* Speaker = "Speaker";
    static const char* Message = "Message";

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

    static const unsigned int NumColumns = sizeof(Sizes) / sizeof(Sizes[0]);

    static const TemplateDescription Template = {
        "SystemChatroomData",
        NumColumns,
        (char**)ColumnNames,
        (VariantType*)Types,
        (unsigned int*)Sizes,
        false,
        0,
        NULL,
        25,
        NULL,
    };
};

#endif