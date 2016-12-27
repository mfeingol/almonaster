//
// Almonaster 2.0
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

#ifndef _GameEngineSchema_H_
#define _GameEngineSchema_H_

#include "Osal/Variant.h"

#include "Database.h"

#include "GameEngineStrings.h"

////////////////
// SystemData //
////////////////

#define SYSTEM_DATA "SystemData"

namespace SystemData {
    
    enum Columns {
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
        LastShutdownTime,
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
        V_TIME,
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
        V_TIME,
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
    
    static const size_t Sizes[] = {
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

    static const unsigned int NumColumns = BuilderBRDampener + 1;

    static const TemplateDescription Template = {
        "SystemData",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

namespace SystemEmpireData {

    enum Columns {
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
        V_TIME,
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
        V_TIME,
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
        V_TIME,
        V_STRING,
        V_STRING,
        V_STRING,
        V_INT,
        V_INT64,
        V_INT,
        V_INT,
        V_INT,
    };

    static const size_t Sizes[] = {
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
    };

    static unsigned int IndexColumns[] = {
        Name,
    };

    static unsigned int IndexFlags[] = {
        INDEX_UNIQUE_DATA,
    };

    static const unsigned int NumColumns = Age + 1;

    static const TemplateDescription Template = {
        "SystemEmpireData",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        1,
        IndexColumns,
        3000,
        IndexFlags,
    };
};

// Around for the sake of the profile viewer
static const char* const SYSTEM_EMPIRE_DATA_COLUMN_NAMES[] = {
    "Name",
    "Password",
    "Privilege Level",
    "Real Name",
    "Email",
    "Web Page",
    "Quote",
    "Alien Key",
    "Deleted",
    "Wins",
    "Nukes",
    "Nuked",
    "Last Login Time",
    "Draws",
    "Max econ",
    "Max mil",
    "IP Address",
    "Num unread system messages",
    "Max number of system messages",
    "Classic Score",
    "Almonaster Score",
    "UIButtons",
    "UIBackground",
    "UILivePlanet",
    "UIDeadPlanet",
    "UISeparator",
    "AlmonasterTheme",
    "LocalPath",
    "DefaultAttackName",
    "DefaultScienceName",
    "DefaultColonyName",
    "DefaultStargateName",
    "DefaultCloakerName",
    "DefaultSatelliteName",
    "DefaultTerraformerName",
    "DefaultTroopshipName",
    "DefaultDoomsdayName",
    "DefaultMinefieldName",
    "DefaultMinesweeperName",
    "DefaultEngineerName",
    "UIHorz",
    "UIVert",
    "UITableColor",
    "CustomTableColor",
    "Options",
    "Max num ships built at once",
    "Creation Time",
    "Logins",
    "Browser",
    "Custom Text Color",
    "Custom Good Color",
    "Custom Bad Color",
    "Custom Private Message Color",
    "Custom Broadcast Message Color",
    "Session Id",
    "Default Builder Planet",
    "Default Message Target",
    "Almonaster Score Significance",
    "Victory Sneer",
    "DefaultCarrierName",
    "DefaultBuilderName",
    "DefaultMorpherName",
    "DefaultJumpgateName",
    "Bridier Rank",
    "Bridier Index",
    "Last Bridier Activity",
    "Private Email Address",
    "Location",
    "Instant Messaging",
    "Game Ratios",
    "Secret Key",
    "Options2",
    "Gender",
    "Age",
};


/////////////////////////
// SystemGameClassData //
/////////////////////////

#define SYSTEM_GAMECLASS_DATA "SystemGameClassData"

namespace SystemGameClassData {

    enum Columns {
        Name,
        MaxNumEmpires,
        MaxNumPlanets,
        MaxTechDev,
        fRESERVED2, //MinEntryScore,
        fRESERVED3, //MaxEntryScore,
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
        V_FLOAT,
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

    static const size_t Sizes[] = {
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

    static unsigned int IndexColumns[] = {
        Owner
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = NumInitialTechDevs + 1;

    static const TemplateDescription Template = {
        "SystemGameClassData",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        1,
        IndexColumns,
        200,
        IndexFlags,
    };
};


//////////////////////
// SystemAlienIcons //
//////////////////////

#define SYSTEM_ALIEN_ICONS "SystemAlienIcons"

namespace SystemAlienIcons {

    enum Columns {      
        AlienKey,
        AuthorName
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_STRING
    };

    static const size_t Sizes[] = {
        0,
        MAX_ALIEN_AUTHOR_NAME_LENGTH
    };

    static unsigned int IndexColumns[] = {
        AlienKey
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = AuthorName + 1;

    static const TemplateDescription Template = {
        "SystemAlienIcons",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        1,
        IndexColumns,
        150,
        IndexFlags,
    };
};


///////////////////////////////
// SystemSystemGameClassData //
///////////////////////////////

#define SYSTEM_SYSTEM_GAMECLASS_DATA "SystemSystemGameClassData"

namespace SystemSystemGameClassData {

    enum Columns {
        GameClass
    };

    static const VariantType Types[] = {
        V_INT
    };

    static const size_t Sizes[] = {
        0
    };

    static const unsigned int NumColumns = GameClass + 1;

    static const TemplateDescription Template = {
        "SystemSystemGameClassData",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
        Name,
        NumGameClasses
    };

    static const VariantType Types[] = {
        V_STRING,
        V_INT
    };

    static const size_t Sizes[] = {
        MAX_SUPER_CLASS_NAME_LENGTH,
        0
    };

    static const unsigned int NumColumns = NumGameClasses + 1;

    static const TemplateDescription Template = {
        "SystemSuperClassData",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
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
        BroadcastMessageColor
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
        V_STRING
    };

    static const size_t Sizes[] = {
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
        MAX_COLOR_LENGTH
    };

    static const unsigned int NumColumns = BroadcastMessageColor + 1;

    static const TemplateDescription Template = {
        "SystemThemes",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
        GameClassGameNumber,
        State
    };
    
    static const VariantType Types[] = {
        V_STRING,
        V_INT
    };

    static const size_t Sizes[] = {
        VARIABLE_LENGTH_STRING,
        0
    };

    static unsigned int IndexColumns[] = {
        GameClassGameNumber
    };

    static unsigned int IndexFlags[] = {
        INDEX_CASE_SENSITIVE,
    };

    static const unsigned int NumColumns = State + 1;

    static const TemplateDescription Template = {
        "SystemActiveGames",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        1,
        IndexColumns,
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

    enum Columns {
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
        V_TIME,
        V_INT,
        V_STRING,
        V_INT,
        V_STRING,
    };

    static const size_t Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = Data + 1;

    static const TemplateDescription Template = {
        "SystemEmpireMessages",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

namespace SystemEmpireNukeList {

    enum Columns {
        AlienKey,
        EmpireName,
        EmpireKey,
        GameClassName,
        GameNumber,
        TimeStamp
    };

    static const VariantType Types[] = {
        V_INT,
        V_STRING,
        V_INT,
        V_STRING,
        V_INT,
        V_TIME
    };

    static const size_t Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        0
    };

    static const unsigned int NumColumns = TimeStamp + 1;

    static const TemplateDescription Template = {
        "SystemEmpireNukeList",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
        NukerAlienKey,
        NukerEmpireName,
        NukerEmpireKey,
        NukedAlienKey,
        NukedEmpireName,
        NukedEmpireKey,
        GameClassName,
        GameNumber,
        TimeStamp
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
        V_TIME
    };

    static const size_t Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        VARIABLE_LENGTH_STRING,
        0,
        0
    };

    static const unsigned int NumColumns = TimeStamp + 1;

    static const TemplateDescription Template = {
        "SystemNukeList",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
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
        V_TIME,
        V_TIME,
        V_INT,
        V_INT,
        V_STRING,
        V_STRING,
    };

    static const size_t Sizes[] = {
        VARIABLE_LENGTH_STRING,
        0,
        0,
        0,
        0,
        0,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = Losers + 1;

    static const TemplateDescription Template = {
        "SystemLatestGames",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

////////////////////////////////
// SystemEmpireActiveGames(I) //
////////////////////////////////

#define SYSTEM_EMPIRE_ACTIVE_GAMES(pszBuffer, i)                \
                                                                \
    char pszBuffer [sizeof ("SystemEmpireActiveGames") + 32];   \
    sprintf (pszBuffer, "SystemEmpireActiveGames%i", i);

#define GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszBuffer, i)            \
                                                                \
    sprintf (pszBuffer, "SystemEmpireActiveGames%i", i);

namespace SystemEmpireActiveGames {

    enum Columns {
        GameClassGameNumber
    };

    static const VariantType Types[] = {
        V_STRING
    };

    static const size_t Sizes[] = {
        VARIABLE_LENGTH_STRING
    };

    static const unsigned int NumColumns = GameClassGameNumber + 1;

    static const TemplateDescription Template = {
        "SystemEmpireActiveGames",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
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

    static const size_t Sizes[] = {
        MAX_TOURNAMENT_NAME_LENGTH,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        VARIABLE_LENGTH_STRING,
        0,
        0,
        MAX_EMPIRE_NAME_LENGTH,
    };

    static unsigned int IndexColumns[] = {
        Owner
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = OwnerName + 1;

    static const TemplateDescription Template = {
        "SystemTournaments",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        0,
        IndexColumns,
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

    enum Columns {
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

    static const size_t Sizes[] = {
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

    static const unsigned int NumColumns = Ruins + 1;

    static const TemplateDescription Template = {
        "SystemTournamentTeams",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
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

    static const size_t Sizes[] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = Ruins + 1;

    static const TemplateDescription Template = {
        "SystemTournamentEmpires",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
        GameClassGameNumber,
    };

    static const VariantType Types[] = {
        V_STRING,
    };

    static const size_t Sizes[] = {
        VARIABLE_LENGTH_STRING,
    };

    static const unsigned int NumColumns = GameClassGameNumber + 1;

    static const TemplateDescription Template = {
        "SystemTournamentActiveGames",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
        TournamentKey,
    };

    static const VariantType Types[] = {
        V_INT,
    };

    static const size_t Sizes[] = {
        0,
    };

    static const unsigned int NumColumns = SystemEmpireTournaments::TournamentKey + 1;

    static const TemplateDescription Template = {
        "SystemEmpireTournaments",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

#define GAME_DATA(pszBuffer, i, j)              \
                                                \
    char pszBuffer [sizeof ("GameData") + 64];  \
    sprintf (pszBuffer, "GameData%i.%i", i, j);

#define GET_GAME_DATA(pszBuffer, i, j)          \
                                                \
    sprintf (pszBuffer, "GameData%i.%i", i, j);

namespace GameData {

    enum Columns {
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
        SecondsSinceLastUpdateWhilePaused,
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
    };

    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_TIME,
        V_INT,
        V_INT,
        V_INT,
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_TIME,
        V_TIME,
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
    };

    static const size_t Sizes[] = {
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
    };
    
    static const unsigned int NumColumns = MaxBridierRankLoss + 1;

    static const TemplateDescription Template = {
        "GameData",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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
    
    enum Columns {
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
    
    static const size_t Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        MAX_IP_ADDRESS_LENGTH,
        0,
    };

    static const unsigned int NumColumns = SecretKey + 1;

    static const TemplateDescription Template = {
        "GameSecurity",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
        EmpireKey
    };
    
    static const VariantType Types[] = {
        V_INT
    };

    static const size_t Sizes[] = {
        0
    };
    
    static unsigned int IndexColumns[] = {
        EmpireKey
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = EmpireKey + 1;

    static const TemplateDescription Template = {
        "GameEmpires",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        1,
        IndexColumns,
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

    enum Columns {
        Name,
        Key,
        Icon,
        Update,
        Reason,
        SecretKey
    };

    static const VariantType Types[] = {
        V_STRING,
        V_INT,
        V_INT,
        V_INT,
        V_INT,
        V_INT64,
    };

    static const size_t Sizes[] = {
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = SecretKey + 1;

    static const TemplateDescription Template = {
        "GameDeadEmpires",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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
    
    enum Columns {
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

    static const size_t Sizes[] = {
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
        0,
        0,
    };

    static unsigned int IndexColumns[] = {
        Owner,
        Coordinates
    };

    static unsigned int IndexFlags[] = {
        0,
        INDEX_CASE_SENSITIVE,
    };
    
    static const unsigned int NumColumns = SurrenderAlmonasterScore + 1;

    static const TemplateDescription Template = {
        "GameMap",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        countof (IndexColumns),
        IndexColumns,
        100,
        IndexFlags,
    };
};

///////////////////////////
// GameEmpireData(I.I.I) //
///////////////////////////

#define GAME_EMPIRE_DATA(pszBuffer, i, j, k)                    \
                                                                \
    char pszBuffer [sizeof ("GameEmpireData") + 96];            \
    sprintf (pszBuffer, "GameEmpireData%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_DATA(pszBuffer, i, j, k)                \
                                                                \
    sprintf (pszBuffer, "GameEmpireData%i.%i.%i", i, j, k);

namespace GameEmpireData {

    enum Columns {
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
        V_TIME,
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
    };

    static const size_t Sizes[] = {
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
    };

    static const unsigned int NumColumns = MiniMaps + 1;
    
    static const TemplateDescription Template = {
        "GameEmpireData",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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
    
    enum Columns {
        Unread,
        Source,
        TimeStamp,
        Flags,
        Text
    };
    
    static const VariantType Types[] = {
        V_INT,
        V_STRING,
        V_TIME,
        V_INT,
        V_STRING
    };

    static const size_t Sizes[] = {
        0,
        MAX_EMPIRE_NAME_LENGTH,
        0,
        0,
        VARIABLE_LENGTH_STRING
    };

    static const unsigned int NumColumns = Text + 1;

    static const TemplateDescription Template = {
        "GameEmpireMessages",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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
    
    enum Columns {
        PlanetKey,
        Explored,
        RESERVED0,
        RESERVED1,
        RESERVED2,
        NumUncloakedShips,
        NumCloakedBuildShips,
        NumUncloakedBuildShips,
        NumCloakedShips
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
        V_INT
    };
    
    static const size_t Sizes[] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };

    static unsigned int IndexColumns[] = {
        PlanetKey
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = NumCloakedShips + 1;

    static const TemplateDescription Template = {
        "GameEmpireMap",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        1,
        IndexColumns,
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
    
    enum Columns {
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

    static const size_t Sizes[] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    static unsigned int IndexColumns[] = {
        EmpireKey,
        CurrentStatus
    };

    static unsigned int IndexFlags[] = {
        0,
        0,
    };

    static const unsigned int NumColumns = LastMessageTargetFlag + 1;
    
    static const TemplateDescription Template = {
        "GameEmpireDiplomacy",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        sizeof (IndexColumns) / sizeof (IndexColumns[0]),
        IndexColumns,
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
    
    enum Columns {
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

    static const size_t Sizes[] = {
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

    static unsigned int IndexColumns[] = {
        CurrentPlanet,
        FleetKey
    };

    static unsigned int IndexFlags[] = {
        0,
        0,
    };

    static const unsigned int NumColumns = ColonyBuildCost + 1;
    
    static const TemplateDescription Template = {
        "GameEmpireShips",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        2,
        IndexColumns,
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
    
    enum Columns {
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
    
    static const size_t Sizes[] = {
        MAX_FLEET_NAME_LENGTH,
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

    static unsigned int IndexColumns[] = {
        CurrentPlanet
    };

    static unsigned int IndexFlags[] = {
        0,
    };

    static const unsigned int NumColumns = Flags + 1;

    static const TemplateDescription Template = {
        "GameEmpireFleets",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        1,
        IndexColumns,
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
    
    enum Columns {
        Name,
        Type,
        CurrentBR,
        MaxBR,
        CurrentPlanet,
        Action,
        FleetKey,
        BuiltThisUpdate,
        State,
        GateDestination
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
        V_INT
    };

    static const size_t Sizes[] = {
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

    static const unsigned int NumColumns = GateDestination + 1;

    static const TemplateDescription Template = {
        "GameIndependentShips",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    enum Columns {
        EmpireKey,
        Data,
        Data2,
    };

    static const unsigned int MaxNumColumns = Data2 + 1;
};

#define SYSTEM_ALMONASTER_SCORE_TOPLIST "SystemAlmonasterScoreTopList"

namespace SystemAlmonasterScoreTopList {
    
    static const VariantType Types[] = {
        V_INT,
        V_FLOAT,
    };
    
    static const size_t Sizes[] = {
        0,
        0
    };

    static const unsigned int NumColumns = 2;

    static const TemplateDescription Template = {
        "SystemAlmonasterScoreTopList",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};


#define SYSTEM_CLASSIC_SCORE_TOPLIST "SystemClassicScoreTopList"

namespace SystemClassicScoreTopList {
    
    static const VariantType Types[] = {
        V_INT,
        V_FLOAT,
    };
    
    static const size_t Sizes[] = {
        0,
        0
    };

    static const unsigned int NumColumns = 2;

    static const TemplateDescription Template = {
        "SystemClassicScoreTopList",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_INT,
    };
    
    static const size_t Sizes[] = {
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = 3;

    static const TemplateDescription Template = {
        "SystemBridierScoreTopList",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
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

    static const VariantType Types[] = {
        V_INT,
        V_INT,
        V_INT,
    };
    
    static const size_t Sizes[] = {
        0,
        0,
        0,
    };

    static const unsigned int NumColumns = 3;

    static const TemplateDescription Template = {
        "SystemBridierScoreEstablishedTopList",
        NumColumns,
        (VariantType*) Types,
        (size_t*) Sizes,
        false,
        0,
        NULL,
        20,
        NULL,
    };
};

#endif