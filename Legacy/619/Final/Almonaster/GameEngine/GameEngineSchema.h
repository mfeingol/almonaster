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

// TODO - hack
#ifndef INITIAL_INSERTION_LENGTH
#define INITIAL_INSERTION_LENGTH VARIABLE_LENGTH_STRING
#endif

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
		iRESERVED2, //AccessEnabled,
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

		// New
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
		MAX_REASON_LENGTH,
		MAX_REASON_LENGTH,
		MAX_REASON_LENGTH,
		MAX_REASON_LENGTH,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
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
	};

	static const unsigned int NumColumns = DefaultJumpgateName + 1;

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
	};

	static const size_t Sizes[] = {
		MAX_EMPIRE_NAME_LENGTH,
		MAX_PASSWORD_LENGTH,
		0,
		MAX_REAL_NAME_LENGTH,
		MAX_EMAIL_LENGTH,
		MAX_WEB_PAGE_LENGTH,
		MAX_QUOTE_LENGTH,
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
		MAX_GRAPHICS_ALTERNATIVE_PATH_LENGTH,
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
		MAX_VICTORY_SNEER_LENGTH,
		MAX_SHIP_NAME_LENGTH,
		MAX_SHIP_NAME_LENGTH,
		MAX_SHIP_NAME_LENGTH,
		MAX_SHIP_NAME_LENGTH,
		0,
		0,
		0,
	};

	static unsigned int IndexColumns[] = {
		Name,
	};

	static const unsigned int NumColumns = LastBridierActivity + 1;

	static const TemplateDescription Template = {
		"SystemEmpireData",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns,
		2000
	};
};

// Around for the sake of the profile viewer
static const char* const SYSTEM_EMPIRE_DATA_COLUMN_NAMES[] = {
	"Name",
	"Password",
	"Privilege",
	"Real name",
	"Email",
	"Web page",
	"Quote",
	"Alien key",
	"Deleted",
	"Wins",
	"Nukes",
	"Nuked",
	"Last Login Time",
	"Draws",
	"Max econ",
	"Max mil",
	"IP address",
	"Num unread system messages",
	"Max number of system messages",
	"Classic score",
	"Almonaster score",
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
	"Creation time",
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
	"Significance",
	"Victory Sneer",
	"DefaultCarrierName",
	"DefaultBuilderName",
	"DefaultMorpherName",
	"DefaultJumpgateName",
	"Bridier score",
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
		fRESERVED2,	//MinEntryScore,
		fRESERVED3,	//MaxEntryScore,
		OpenGameNum,
		NumSecPerUpdate,
		Options,
		MinAvgFuel,
		iRESERVED0,
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
		iRESERVED1,
		OwnerName,
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
		MAX_GAMECLASS_DESCRIPTION_LENGTH,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		MAX_EMPIRE_NAME_LENGTH,
	};

	static unsigned int IndexColumns[] = {
		Owner
	};

	static const unsigned int NumColumns = OwnerName + 1;

	static const TemplateDescription Template = {
		"SystemGameClassData",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns,
		100
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

	static const unsigned int NumColumns = AuthorName + 1;

	static const TemplateDescription Template = {
		"SystemAlienIcons",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns
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
		30
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
		10
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
		10
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
		MAX_GAMECLASS_GAMENUMBER_LENGTH,
		0
	};

	static unsigned int IndexColumns[] = {
		GameClassGameNumber
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
		100
	};
};


/////////////////////////////
// SystemEmpireMessages(I) //
/////////////////////////////

#define SYSTEM_EMPIRE_MESSAGES(pszBuffer, i)				\
															\
	char pszBuffer [sizeof ("SystemEmpireMessages") + 32];	\
	sprintf (pszBuffer, "SystemEmpireMessages%i", i);

#define GET_SYSTEM_EMPIRE_MESSAGES(pszBuffer, i)			\
															\
	sprintf (pszBuffer, "SystemEmpireMessages%i", i);

namespace SystemEmpireMessages {

	enum Columns {
		Unread,
		Source,
		TimeStamp,
		Broadcast,
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
		INITIAL_INSERTION_LENGTH
	};

	static unsigned int IndexColumns[] = {
		Unread
	};

	static const unsigned int NumColumns = Text + 1;

	static const TemplateDescription Template = {
		"SystemEmpireMessages",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns,
		10
	};
};


/////////////////////////////
// SystemEmpireNukeList(I) //
/////////////////////////////

#define SYSTEM_EMPIRE_NUKED_LIST(pszBuffer, i)				\
															\
	char pszBuffer [sizeof ("SystemEmpireNukedList") + 32];	\
	sprintf (pszBuffer, "SystemEmpireNukedList%i", i);

#define GET_SYSTEM_EMPIRE_NUKED_LIST(pszBuffer, i)			\
															\
	sprintf (pszBuffer, "SystemEmpireNukedList%i", i);

#define SYSTEM_EMPIRE_NUKER_LIST(pszBuffer, i)				\
															\
	char pszBuffer [sizeof ("SystemEmpireNukerList") + 32];	\
	sprintf (pszBuffer, "SystemEmpireNukerList%i", i);

#define GET_SYSTEM_EMPIRE_NUKER_LIST(pszBuffer, i)			\
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
		MAX_FULL_GAME_CLASS_NAME_LENGTH,
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
		20
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
		MAX_FULL_GAME_CLASS_NAME_LENGTH,
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
		20
	};
};


////////////////////////////////
// SystemEmpireActiveGames(I) //
////////////////////////////////

#define SYSTEM_EMPIRE_ACTIVE_GAMES(pszBuffer, i)				\
																\
	char pszBuffer [sizeof ("SystemEmpireActiveGames") + 32];	\
	sprintf (pszBuffer, "SystemEmpireActiveGames%i", i);

#define GET_SYSTEM_EMPIRE_ACTIVE_GAMES(pszBuffer, i)			\
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
		MAX_GAMECLASS_GAMENUMBER_LENGTH
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
		20
	};
};


///////////////////
// GameData(I.I) //
///////////////////

#define GAME_DATA(pszBuffer, i, j)				\
												\
	char pszBuffer [sizeof ("GameData") + 64];	\
	sprintf (pszBuffer, "GameData%i.%i", i, j);

#define GET_GAME_DATA(pszBuffer, i, j)			\
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
		NumPaused,
		MaxY,
		SecondsRemainingInUpdateWhilePaused,
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
		MAX_ENTER_GAME_MESSAGE_LENGTH,
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
	};
	
	static const unsigned int NumColumns = MaxWins + 1;

	static const TemplateDescription Template = {
		"GameData",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		true,
		0,
		NULL,
		1
	};
};


///////////////////////
// GameSecurity(I.I) //
///////////////////////

#define GAME_SECURITY(pszBuffer, i, j)				\
												\
	char pszBuffer [sizeof ("GameSecurity") + 64];	\
	sprintf (pszBuffer, "GameSecurity%i.%i", i, j);

#define GET_GAME_SECURITY(pszBuffer, i, j)			\
												\
	sprintf (pszBuffer, "GameSecurity%i.%i", i, j);

namespace GameSecurity {
	
	enum Columns {
		EmpireKey,
		EmpireName,
		Options,
		SessionId,
		IPAddress,
	};

	static const VariantType Types[] = {
		V_INT,
		V_STRING,
		V_INT,
		V_INT64,
		V_STRING,
	};
	
	static const size_t Sizes[] = {
		0,
		MAX_EMPIRE_NAME_LENGTH,
		0,
		0,
		MAX_IP_ADDRESS_LENGTH,
	};

	static const unsigned int NumColumns = IPAddress + 1;

	static const TemplateDescription Template = {
		"GameSecurity",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		0,
		NULL,
		2
	};
};

//////////////////////
// GameEmpires(I.I) //
//////////////////////

#define GAME_EMPIRES(pszBuffer, i, j)				\
													\
	char pszBuffer [sizeof ("GameEmpires") + 64];	\
	sprintf (pszBuffer, "GameEmpires%i.%i", i, j);

#define GET_GAME_EMPIRES(pszBuffer, i, j)			\
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
		V_INT
	};
	
	static unsigned int IndexColumns[] = {
		EmpireKey
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
		10
	};
};


//////////////////
// GameMap(I.I) //
//////////////////

#define GAME_MAP(pszBuffer, i, j)					\
													\
	char pszBuffer [sizeof ("GameMap") + 64];		\
	sprintf (pszBuffer, "GameMap%i.%i", i, j);

#define GET_GAME_MAP(pszBuffer, i, j)				\
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
		NumUncloakedShips,		// Total number of built un-cloaked ships on the planet
		NumCloakedShips,		// Total number of built cloaked ships on the planet
		NumUncloakedBuildShips,	// Total number of un-built un-cloaked ships on the planet
		NumCloakedBuildShips,	// Total number of un-built cloaked ships on the planet
		SurrenderEmpireNameHash,
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
		V_INT,
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
		Coordinates
	};
	
	static const unsigned int NumColumns = SurrenderAlmonasterScore + 1;

	static const TemplateDescription Template = {
		"GameMap",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns,
		50
	};
};

///////////////////////////
// GameEmpireData(I.I.I) //
///////////////////////////

#define GAME_EMPIRE_DATA(pszBuffer, i, j, k)					\
																\
	char pszBuffer [sizeof ("GameEmpireData") + 96];			\
	sprintf (pszBuffer, "GameEmpireData%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_DATA(pszBuffer, i, j, k)				\
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
		MAX_NOTEPAD_LENGTH,
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

	static const unsigned int NumColumns = InitialBridierIndex + 1;
	
	static const TemplateDescription Template = {
		"GameEmpireData",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		true,
		0,
		NULL,
		1
	};
};


///////////////////////////////
// GameEmpireMessages(I.I.I) //
///////////////////////////////

#define GAME_EMPIRE_MESSAGES(pszBuffer, i, j, k)				\
																\
	char pszBuffer [sizeof ("GameEmpireMessages") + 96];		\
	sprintf (pszBuffer, "GameEmpireMessages%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_MESSAGES(pszBuffer, i, j, k)			\
																\
	sprintf (pszBuffer, "GameEmpireMessages%i.%i.%i", i, j, k);

namespace GameEmpireMessages {
	
	enum Columns {
		Unread,
		Source,
		TimeStamp,
		Broadcast,
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
		INITIAL_INSERTION_LENGTH
	};

	static unsigned int IndexColumns[] = {
		Unread
	};
	
	static const unsigned int NumColumns = Text + 1;

	static const TemplateDescription Template = {
		"GameEmpireMessages",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns,
		20
	};
};


//////////////////////////
// GameEmpireMap(I.I.I) //
//////////////////////////

#define GAME_EMPIRE_MAP(pszBuffer, i, j, k)						\
																\
	char pszBuffer [sizeof ("GameEmpireMap") + 96];				\
	sprintf (pszBuffer, "GameEmpireMap%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_MAP(pszBuffer, i, j, k)				\
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

	static const unsigned int NumColumns = NumCloakedShips + 1;

	static const TemplateDescription Template = {
		"GameEmpireMap",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns,
		50
	};
};


////////////////////////////////
// GameEmpireDiplomacy(I.I.I) //
////////////////////////////////

#define GAME_EMPIRE_DIPLOMACY(pszBuffer, i, j, k)						\
																		\
	char pszBuffer [sizeof ("GameEmpireDiplomacy") + 96];				\
	sprintf (pszBuffer, "GameEmpireDiplomacy%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_DIPLOMACY(pszBuffer, i, j, k)					\
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

	static const unsigned int NumColumns = LastMessageTargetFlag + 1;
	
	static const TemplateDescription Template = {
		"GameEmpireDiplomacy",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		sizeof (IndexColumns) / sizeof (IndexColumns[0]),
		IndexColumns,
		10
	};
};


////////////////////////////
// GameEmpireShips(I.I.I) //
////////////////////////////

#define GAME_EMPIRE_SHIPS(pszBuffer, i, j, k)						\
																	\
	char pszBuffer [sizeof ("GameEmpireShips") + 96];				\
	sprintf (pszBuffer, "GameEmpireShips%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_SHIPS(pszBuffer, i, j, k)					\
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
		Action,
		FleetKey
	};

	static const unsigned int NumColumns = ColonyBuildCost + 1;
	
	static const TemplateDescription Template = {
		"GameEmpireShips",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		3,
		IndexColumns,
		40
	};
};


/////////////////////////////
// GameEmpireFleets(I.I.I) //
/////////////////////////////

#define GAME_EMPIRE_FLEETS(pszBuffer, i, j, k)						\
																	\
	char pszBuffer [sizeof ("GameEmpireFleets") + 96];				\
	sprintf (pszBuffer, "GameEmpireFleets%i.%i.%i", i, j, k);

#define GET_GAME_EMPIRE_FLEETS(pszBuffer, i, j, k)					\
																	\
	sprintf (pszBuffer, "GameEmpireFleets%i.%i.%i", i, j, k);

namespace GameEmpireFleets {
	
	enum Columns {
		Name,
		RESERVED0,
		CurrentStrength,
		MaxStrength,
		CurrentPlanet,
		Action,
		BuildShips
	};
	
	static const VariantType Types[] = {
		V_STRING,
		V_INT,
		V_FLOAT,
		V_FLOAT,
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
		0
	};

	static unsigned int IndexColumns[] = {
		CurrentPlanet
	};

	static const unsigned int NumColumns = BuildShips + 1;

	static const TemplateDescription Template = {
		"GameEmpireFleets",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns,
		10
	};
};


//////////////////////////
// GameIndependentShips //
//////////////////////////

#define GAME_INDEPENDENT_SHIPS(pszBuffer, i, j)				\
												\
	char pszBuffer [sizeof ("GameIndependentShips") + 64];	\
	sprintf (pszBuffer, "GameIndependentShips%i.%i", i, j);

#define GET_GAME_INDEPENDENT_SHIPS(pszBuffer, i, j)			\
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

	static unsigned int IndexColumns[] = {
		CurrentPlanet
	};

	static const unsigned int NumColumns = GateDestination + 1;

	static const TemplateDescription Template = {
		"GameIndependentShips",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		1,
		IndexColumns,
		10
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
		20
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
		20
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
		20
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
		20
	};
};

#endif