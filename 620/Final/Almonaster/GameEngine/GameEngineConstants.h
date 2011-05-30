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

#ifndef _GameEngineConstants_H_
#define _GameEngineConstants_H_

#include "Database.h"

#define EMPIRE_NAME_HASH_BUCKETS (0x7fffffff)

// Define this only for testing purposes
//#define PRIVATE_DEBUG_ONLY

#define TEST_ERROR(x) iErrCode = x;  if (iErrCode != OK) return iErrCode;

// Ship types
enum ShipType {
    ATTACK,
    SCIENCE,
    COLONY,
    STARGATE,
    CLOAKER,
    SATELLITE,
    TERRAFORMER,
    TROOPSHIP,
    DOOMSDAY,
    MINEFIELD,
    MINESWEEPER,
    ENGINEER,
    CARRIER,
    BUILDER,
    MORPHER,
    JUMPGATE,

    FIRST_SHIP = ATTACK,
    LAST_SHIP = JUMPGATE,
    LAST_CLASSIC_SHIP = ENGINEER,
};

#define NUM_SHIP_TYPES (LAST_SHIP + 1)

#define ENUMERATE_SHIP_TYPES(i) for (i = FIRST_SHIP; i <= LAST_SHIP; i ++)

extern const char* const SHIP_TYPE_STRING[];
extern const char* const SHIP_TYPE_STRING_PLURAL[];
extern const char* const SHIP_TYPE_STRING_LOWERCASE[];
extern const char* const SHIP_TYPE_DESCRIPTION[];

extern const unsigned int SYSTEM_DATA_SHIP_NAME_COLUMN[];
extern const unsigned int SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[];
extern const int MORPH_ORDER[];

// Diplomacy levels
#define WAR                 (0x00000001)
#define TRUCE               (0x00000002)
#define TRADE               (0x00000004)
#define ALLIANCE            (0x00000008)
#define SURRENDER           (0x00000010)
#define ACCEPT_SURRENDER    (0x00000020)

#define NUM_DIP_LEVELS (6)

// Diplomacy level names
#define WAR_STRING              "War"
#define TRUCE_STRING            "Truce"
#define TRADE_STRING            "Trade"
#define ALLIANCE_STRING         "Alliance"

#define SURRENDER_STRING        "Surrender"
#define ACCEPT_SURRENDER_STRING "Accept Surrender"

extern const char* DIPLOMACY_STRING[7];

#define DIP_STRING(x) ((x & 0x000000f0) == 0 ? DIPLOMACY_STRING[x / 2] : DIPLOMACY_STRING[5 + (x >> 4) / 2])

enum SurrenderType {
    SC30_SURRENDER,
    NORMAL_SURRENDER,
};

// SystemGameClassData::MaxNumEmpires
#define UNLIMITED_EMPIRES       (-1)

// SystemGameClassData::MapsShared
#define NO_DIPLOMACY            (-1)

// SystemGameClassData::MaxNumAlliances
#define UNRESTRICTED_DIPLOMACY  (-1)
#define FAIR_DIPLOMACY          (-2)
#define STATIC_RESTRICTION      (-3) /*Only for form submission purposes*/

// GameEmpireDiplomacy::State options
#define IGNORE_EMPIRE           (0x00000001)
#define ONCE_ALLIED_WITH        (0x00000002)


// Graphical themes
enum Graphics {
    INDIVIDUAL_ELEMENTS = -10,
    ALTERNATIVE_PATH    = -20,
    NULL_THEME          = -30,
    CUSTOM_COLORS       = -40,
    UPLOADED_ICON       = -50,
};

// Default builder planets
enum BuilderPlanets {
    NO_DEFAULT_BUILDER_PLANET = -1,
    HOMEWORLD_DEFAULT_BUILDER_PLANET = -2,
    LAST_BUILDER_DEFAULT_BUILDER_PLANET = -3    
};

// Game ratios
enum GameRatioSetting {
    RATIOS_DISPLAY_NEVER,
    RATIOS_DISPLAY_ON_RELEVANT_SCREENS,
    RATIOS_DISPLAY_ALWAYS,
};

// New fleet key
#define FLEET_NEWFLEETKEY (0xffffabab)

// Cardinal points
#define NUM_CARDINAL_POINTS 4

enum CardinalPoint {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    NO_DIRECTION
};

extern const CardinalPoint OPPOSITE_CARDINAL_POINT[];
extern const char* const CARDINAL_STRING[];
extern const char* const OPPOSITE_CARDINAL_STRING[];

extern const int CLOSE_LINK[];      
extern const int OPEN_LINK[];
extern const int CREATE_PLANET_LINK[];

#define ENUMERATE_CARDINAL_POINTS(i) for (i = NORTH; i <= WEST; i ++)

// SystemData::ShipBehavior
#define COLONY_USE_MULTIPLIED_BUILD_COST            (0x00000001)
#define COLONY_USE_MULTIPLIED_POPULATION_DEPOSIT    (0x00000002)
//#define COLONY_DISABLE_SETTLES                    (0x00000004)    Reuse
//#define COLONY_DISABLE_SURVIVAL                   (0x00000008)    Reuse
#define CLOAKER_CLOAK_ON_BUILD                      (0x00000010)
#define TERRAFORMER_DISABLE_FRIENDLY                (0x00000020)
#define TERRAFORMER_DISABLE_SURVIVAL                (0x00000040)
//#define TROOPSHIP_DISABLE_IF_SUCCESSFUL           (0x00000080)    Reuse
#define TROOPSHIP_DISABLE_SURVIVAL                  (0x00000100)
#define STARGATE_LIMIT_RANGE                        (0x00000200)
#define JUMPGATE_LIMIT_RANGE                        (0x00000400)
#define MORPHER_CLOAK_ON_CLOAKER_MORPH              (0x00000800)
#define TERRAFORMER_DISABLE_MULTIPLE                (0x00001000)
//#define DOOMSDAY_DISABLE_QUARANTINE               (0x00002000)    Reuse
#define MINEFIELD_DISABLE_DETONATE                  (0x00004000)

#define ALL_SHIP_BEHAVIOR_OPTIONS                   (0x00007fff)

// GameMap::Pop
#define MAX_POPULATION      (0x7fffffff)

// GameMap::Annihilated
#define NOT_ANNIHILATED     (0)
#define ANNIHILATED_FOREVER (0xffffffff)
#define ANNIHILATED_UNKNOWN (0xfefefefe)    // Only used during update algorithm

// GameEmpireMap::Explored
#define EXPLORED_NORTH      (0x00000001)
#define EXPLORED_EAST       (0x00000002)
#define EXPLORED_SOUTH      (0x00000004)
#define EXPLORED_WEST       (0x00000008)

extern const int EXPLORED_X[];
extern const int OPPOSITE_EXPLORED_X[];

// GameMap::Link
#define LINK_NORTH          (0x00000001)
#define LINK_EAST           (0x00000002)
#define LINK_SOUTH          (0x00000004)
#define LINK_WEST           (0x00000008)

extern const int LINK_X[];
extern const int OPPOSITE_LINK_X[];

// Ruins of
#define RUINS_OF "Ruins of %s"

enum NukeList { NUKER_LIST, NUKED_LIST, SYSTEM_LIST };

// Team options for creating games
#define TEAM_PREARRANGED_DIPLOMACY          (0x00000001)
#define TEAM_PREARRANGED_ALLIANCES          (0x00000002)

// GameMap::HomeWorld
#define HOMEWORLD (-1)
#define NOT_HOMEWORLD (-2)

// GameEmpireData::Options
#define UPDATED                             (0x00000001)
#define AUTO_UPDATE                         (0x00000002)
#define LOGGED_IN_THIS_UPDATE               (0x00000004)
#define GAME_REPEATED_BUTTONS               (0x00000008)
#define REQUEST_DRAW                        (0x00000010)
#define REQUEST_PAUSE                       (0x00000020)
#define IGNORE_BROADCASTS                   (0x00000040)
#define MAP_COLORING                        (0x00000080)
#define RESIGNED                            (0x00000100)
#define PARTIAL_MAPS                        (0x00000200)
#define SHIPS_ON_MAP_SCREEN                 (0x00000400)
#define LOCAL_MAPS_IN_UPCLOSE_VIEWS         (0x00000800)
#define SHIPS_ON_PLANETS_SCREEN             (0x00001000)
#define SHIP_MAP_COLORING                   (0x00002000)
#define SHIP_MAP_HIGHLIGHTING               (0x00004000)
//#define SHOW_ADVANCED_SEARCH_INTERFACE    (0x00008000)    // Open
//#define DISPLAY_FATAL_UPDATE_MESSAGES     (0x00010000)    // Open
#define SENSITIVE_MAPS                      (0x00020000)
#define AUTO_REFRESH                        (0x00040000)
#define COUNTDOWN                           (0x00080000)
#define GAME_DISPLAY_TIME                   (0x00100000)
//#define SYSTEM_REPEATED_BUTTONS           (0x00200000)    // Open
//#define REFRESH_SESSION_ID                (0x00400000)    // Open
#define REJECT_INDEPENDENT_SHIP_GIFTS       (0x00800000)
//#define SEND_SCORE_MESSAGE_ON_NUKE        (0x01000000)    // Open
//#define SHOW_TECH_DESCRIPTIONS            (0x02000000)    // Open
#define DISPLACE_ENDTURN_BUTTON             (0x04000000)
#define BUILD_ON_MAP_SCREEN                 (0x08000000)
#define BUILD_ON_PLANETS_SCREEN             (0x10000000)

// SystemEmpireData::Options
#define SYSTEM_DISPLAY_TIME                 (0x00000001)
#define CAN_BROADCAST                       (0x00000002)
#define EMPIRE_MARKED_FOR_DELETION          (0x00000004)
//#define GAME_REPEATED_BUTTONS             (0x00000008)
#define IP_ADDRESS_PASSWORD_HASHING         (0x00000010)
#define SESSION_ID_PASSWORD_HASHING         (0x00000020)
#define CONFIRM_IMPORTANT_CHOICES           (0x00000040)
//#define MAP_COLORING                      (0x00000080)
#define FIXED_BACKGROUNDS                   (0x00000100)
//#define PARTIAL_MAPS                      (0x00000200)
//#define SHIPS_ON_MAP_SCREEN               (0x00000400)
//#define LOCAL_MAPS_IN_UPCLOSE_VIEWS       (0x00000800)
//#define SHIPS_ON_PLANETS_SCREEN           (0x00001000)
//#define SHIP_MAP_COLORING                 (0x00002000)
//#define SHIP_MAP_HIGHLIGHTING             (0x00004000)
#define SHOW_ADVANCED_SEARCH_INTERFACE      (0x00008000)
#define DISPLAY_FATAL_UPDATE_MESSAGES       (0x00010000)
//#define SENSITIVE_MAPS                    (0x00020000)
//#define AUTO_REFRESH                      (0x00040000)
//#define COUNTDOWN                         (0x00080000)
//#define GAME_DISPLAY_TIME                 (0x00100000)
#define SYSTEM_REPEATED_BUTTONS             (0x00200000)
#define RESET_SESSION_ID                    (0x00400000)
#define REJECT_INDEPENDENT_SHIP_GIFTS       (0x00800000)
#define SEND_SCORE_MESSAGE_ON_NUKE          (0x01000000)
#define SHOW_TECH_DESCRIPTIONS              (0x02000000)
//#define DISPLACE_ENDTURN_BUTTON           (0x04000000)
//#define BUILD_ON_MAP_SCREEN               (0x08000000)
//#define BUILD_ON_PLANETS_SCREEN           (0x10000000)

// SystemEmpireData::Options2
#define ADMINISTRATOR_FIXED_PRIVILEGE       (0x00000001)
#define UNAVAILABLE_FOR_TOURNAMENTS         (0x00000002)
#define REFRESH_UNSTARTED_GAME_PAGES        (0x00000004)
#define DISBAND_EMPTY_FLEETS_ON_UPDATE      (0x00000008)

// GameEmpireData::MiniMaps
enum MiniMaps {
    MINIMAPS_NEVER,
    MINIMAPS_OPTION,
    MINIMAPS_PRIMARY,
};

// GameEmpireData::TechDevs
#define TECH_ATTACK                         (0x00000001)
#define TECH_SCIENCE                        (0x00000002)
#define TECH_COLONY                         (0x00000004)
#define TECH_STARGATE                       (0x00000008)
#define TECH_CLOAKER                        (0x00000010)
#define TECH_SATELLITE                      (0x00000020)
#define TECH_TERRAFORMER                    (0x00000040)
#define TECH_TROOPSHIP                      (0x00000080)
#define TECH_DOOMSDAY                       (0x00000100)
#define TECH_MINEFIELD                      (0x00000200)
#define TECH_MINESWEEPER                    (0x00000400)
#define TECH_ENGINEER                       (0x00000800)
#define TECH_CARRIER                        (0x00001000)
#define TECH_BUILDER                        (0x00002000)
#define TECH_MORPHER                        (0x00004000)
#define TECH_JUMPGATE                       (0x00008000)

#define ALL_CLASSIC_TECHS (TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_STARGATE | TECH_CLOAKER | \
                           TECH_SATELLITE | TECH_TERRAFORMER | TECH_TROOPSHIP | TECH_DOOMSDAY | TECH_MINEFIELD | \
                           TECH_MINESWEEPER | TECH_ENGINEER)

#define ALL_NEW_TECHS     (TECH_CARRIER | TECH_BUILDER | TECH_MORPHER | TECH_JUMPGATE)

#define ALL_TECHS        (ALL_CLASSIC_TECHS | ALL_NEW_TECHS)

#define NUM_TECHS NUM_SHIP_TYPES
#define NUM_CLASSIC_TECHS (12)
#define NUM_NEW_TECHS (4)

extern const int TECH_BITS[];

#define ENUMERATE_TECHS(i) for (i = 0; i < NUM_SHIP_TYPES; i ++)

// SystemData::Options
#define DEFAULT_BRIDIER_GAMES                   (0x00000001)
#define LOGINS_ENABLED                          (0x00000002)
#define NEW_EMPIRES_ENABLED                     (0x00000004)
#define NEW_GAMES_ENABLED                       (0x00000008)
#define ACCESS_ENABLED                          (0x00000010)
#define DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS    (0x00000020)
#define DEFAULT_BLOCK_ON_DUPLICATE_IP_ADDRESS   (0x00000040)
#define DEFAULT_WARN_ON_DUPLICATE_SESSION_ID    (0x00000080)
#define DEFAULT_BLOCK_ON_DUPLICATE_SESSION_ID   (0x00000100)
#define DEFAULT_NAMES_LISTED                    (0x00000200)
#define DEFAULT_ALLOW_SPECTATORS                (0x00000400)
#define DISABLE_PRIVILEGE_SCORE_ELEVATION       (0x00000800)
#define DEFAULT_RESTRICT_IDLE_EMPIRES           (0x00001000)
#define PAUSE_GAMES_BY_DEFAULT                  (0x00002000)

// GameEmpireShips::State
#define CLOAKED                                 (0x00000001)
#define MORPH_ENABLED                           (0x00000002)

// GameData::State
#define PAUSED                                  (0x00000001)
#define ADMIN_PAUSED                            (0x00000002)
#define STILL_OPEN                              (0x00000004)
#define STARTED                                 (0x00000008)
#define GAME_UPDATING                           (0x00000010)
#define GAME_CREATING                           (0x00000020)
#define GAME_ADDING_EMPIRE                      (0x00000040)
#define GAME_DELETING                           (0x00000080)
#define GAME_DELETING_EMPIRE                    (0x00000100)
#define GAME_WAITING_TO_UPDATE                  (0x00000200)
#define GAME_MAP_GENERATED                      (0x00000400)
#define GAME_ENDED                              (0x80000000)

#define GAME_BUSY (GAME_UPDATING | GAME_CREATING | GAME_ADDING_EMPIRE | GAME_DELETING | GAME_DELETING_EMPIRE)
#define GAME_DELETION_REASON_MASK (PAUSED | ADMIN_PAUSED | STILL_OPEN | STARTED | GAME_WAITING_TO_UPDATE | GAME_MAP_GENERATED)

// GameData::Options
#define GAME_COUNT_FOR_BRIDIER              (0x00000001)
#define GAME_LOG_HISTORY                    (0x00000002)
#define GAME_ALLOW_SPECTATORS               (0x00000004)
#define GAME_WARN_ON_DUPLICATE_IP_ADDRESS   (0x00000008)
#define GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS  (0x00000010)
#define GAME_WARN_ON_DUPLICATE_SESSION_ID   (0x00000020)
#define GAME_BLOCK_ON_DUPLICATE_SESSION_ID  (0x00000040)
#define GAME_NAMES_LISTED                   (0x00000080)
#define GAME_RESTRICT_MIN_ALMONASTER_SCORE  (0x00000100)
#define GAME_RESTRICT_MAX_ALMONASTER_SCORE  (0x00000200)
#define GAME_RESTRICT_MIN_CLASSIC_SCORE     (0x00000400)
#define GAME_RESTRICT_MAX_CLASSIC_SCORE     (0x00000800)
#define GAME_RESTRICT_MIN_BRIDIER_RANK      (0x00001000)
#define GAME_RESTRICT_MAX_BRIDIER_RANK      (0x00002000)
#define GAME_RESTRICT_MIN_BRIDIER_INDEX     (0x00004000)
#define GAME_RESTRICT_MAX_BRIDIER_INDEX     (0x00008000)
#define GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN (0x00010000)
#define GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN (0x00020000)
#define GAME_RESTRICT_MIN_WINS              (0x00040000)
#define GAME_RESTRICT_MAX_WINS              (0x00080000)
#define GAME_ENFORCE_SECURITY               (0x00100000)
#define GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS (0x00200000)
#define GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS (0x00400000)
#define GAME_RESTRICT_IDLE_EMPIRES          (0x00800000)

enum EntryRestrictions {
    RESTRICT_ALMONASTER_SCORE,
    RESTRICT_CLASSIC_SCORE,
    RESTRICT_BRIDIER_RANK,
    RESTRICT_BRIDIER_INDEX,
    RESTRICT_BRIDIER_RANK_GAIN,
    RESTRICT_BRIDIER_RANK_LOSS,
    RESTRICT_WINS,
};

#define NUM_ENTRY_SCORE_RESTRICTIONS (RESTRICT_WINS + 1)

enum ReasonForRemoval {
    EMPIRE_UNKNOWN,
    EMPIRE_NUKED,
    EMPIRE_SURRENDERED,
    EMPIRE_RUINED,
    EMPIRE_TERRAFORMED,
    EMPIRE_INVADED,
    EMPIRE_ANNIHILATED,
    EMPIRE_COLONIZED,
    EMPIRE_ADMIN_REMOVED,
    EMPIRE_QUIT,
    EMPIRE_GAME_ENDED,
    EMPIRE_GAME_ENTRY_ERROR,
};

extern const char* const REMOVAL_REASON_STRING[];

// SystemEmpireMessages::Unread
#define MESSAGE_READ   0
#define MESSAGE_UNREAD 1

// SystemEmpireMessages::Type
enum MessageType {
    MESSAGE_NORMAL,
    MESSAGE_TOURNAMENT_INVITATION,
    MESSAGE_TOURNAMENT_JOIN_REQUEST,
};

// SystemEmpireMessages::Flags
// GameEmpireMessages::Flags
#define MESSAGE_BROADCAST               (0x00000001)
#define MESSAGE_UPDATE                  (0x00000002)
#define MESSAGE_SYSTEM                  (0x00000004)

// GameSecurity::Options
#define GAME_SECURITY_CHECK_IPADDRESS   (0x00000001)
#define GAME_SECURITY_CHECK_SESSIONID   (0x00000002)

// GameData::State overloads for DeleteGame()
#define SYSTEM_SHUTDOWN                 (0x00000001)
#define MAP_CREATION_ERROR              (0x00000002)
#define PASSWORD_PROTECTED              (0x00000004)
#define CREATION_FAILED                 (0x00000008)
#define TABLE_VERIFICATION_ERROR        (0x10000000)

// SystemThemes::Options
#define THEME_BACKGROUND                (0x00000001)
#define THEME_LIVE_PLANET               (0x00000002)
#define THEME_DEAD_PLANET               (0x00000004)
#define THEME_SEPARATOR                 (0x00000008)
#define THEME_BUTTONS                   (0x00000010)
#define THEME_HORZ                      (0x00000020)
#define THEME_VERT                      (0x00000040)
#define THEME_INDEPENDENT_PLANET        (0x00000080)
#define ALL_THEME_OPTIONS               (0x0000007f) // TODO:  wrong when independent planets are used


#define EXPOSED_SPECTATORS (EXPOSED_MAP | EXPOSED_DIPLOMACY)

// SystemGameClassData::Options
#define EXPOSED_MAP                     (0x00000001)
#define FULLY_COLONIZED_MAP             (0x00000002)
#define WEEKEND_UPDATES                 (0x00000004)
#define GAMECLASS_HALTED                (0x00000008)
#define VISIBLE_DIPLOMACY               (0x00000010)
#define GENERATE_MAP_FIRST_UPDATE       (0x00000020)
#define INDEPENDENCE                    (0x00000040)
#define PRIVATE_MESSAGES                (0x00000080)
#define EXPOSED_DIPLOMACY               (0x00000100)
#define VISIBLE_BUILDS                  (0x00000200)
#define DYNAMIC_GAMECLASS               (0x00000400)
#define GAMECLASS_MARKED_FOR_DELETION   (0x00000800)
#define DISABLE_SCIENCE_SHIPS_NUKING    (0x00001000)
#define ALLOW_DRAW                      (0x00002000)
//#define WARN_ON_DUPLICATE_SESSION_ID  (0x00004000)    Reuse
//#define BLOCK_ON_DUPLICATE_SESSION_ID (0x00008000)    Reuse
#define SUBJECTIVE_VIEWS                (0x00010000)
#define UNBREAKABLE_ALLIANCES           (0x00020000)
#define ONLY_SURRENDER_WITH_TWO_EMPIRES (0x00040000)
#define PERMANENT_ALLIANCES             (0x00080000)
#define DISCONNECTED_MAP                (0x00100000)
#define USE_SC30_SURRENDERS             (0x00200000)
#define USE_CLASSIC_DOOMSDAYS           (0x00400000)
#define USE_UNFRIENDLY_DOOMSDAYS        (0x00800000)
#define DISABLE_SUICIDAL_DOOMSDAYS      (0x01000000)
#define USE_FRIENDLY_GATES              (0x02000000)

// SystemGameClassData::RuinFlags
#define RUIN_CLASSIC_SC                 (0x00000001)
#define RUIN_ALMONASTER                 (0x00000002)

#define NUM_UPDATES_FOR_GAME_RUIN 12

// SystemGameClassData::MaxNumActiveGames
#define INFINITE_ACTIVE_GAMES 0

// SystemGameClassData::MaxNumShips
#define INFINITE_SHIPS 0

// SystemEmpireData::DefaultMessageTarget
// GameEmpireData::DefaultMessageTarget
enum MessageTargets {
    MESSAGE_TARGET_NONE = 0,
    MESSAGE_TARGET_BROADCAST = 1,
    MESSAGE_TARGET_WAR = 2,
    MESSAGE_TARGET_TRUCE = 3,
    MESSAGE_TARGET_TRADE = 4,
    MESSAGE_TARGET_ALLIANCE = 5,
    MESSAGE_TARGET_LAST_USED = 6,

    NUM_MESSAGE_TARGETS = MESSAGE_TARGET_LAST_USED
};

#define MESSAGE_TARGET_BROADCAST_MASK   (0x00000001)
#define MESSAGE_TARGET_WAR_MASK         (0x00000002)
#define MESSAGE_TARGET_TRUCE_MASK       (0x00000004)
#define MESSAGE_TARGET_TRADE_MASK       (0x00000008)
#define MESSAGE_TARGET_ALLIANCE_MASK    (0x00000010)

// Maximum values
#define MAX_NUM_UPDATES_BEFORE_IDLE 10
#define MAX_NUM_UPDATES_BEFORE_RUIN 25
#define MAX_NUM_UPDATE_PERIODS_FOR_FIRST_DELAY 10


// Privilege levels
#define NUM_PRIVILEGES 5

enum Privilege {
    GUEST = -5,
    NOVICE = 0,
    APPRENTICE = 1,
    ADEPT = 2,
    ADMINISTRATOR = 3
};

extern const Privilege PrivilegeKey[];

#define ENUMERATE_PRIVILEGE_LEVELS(i) for (j = 0, i = PrivilegeKey[j]; j < NUM_PRIVILEGES; j ++, i = PrivilegeKey[j])

extern const char* const REAL_PRIVILEGE_STRING[];
extern const char* const REAL_PRIVILEGE_STRING_PLURAL[];

const char** const PRIVILEGE_STRING = (const char** const) &REAL_PRIVILEGE_STRING[5];
const char** const PRIVILEGE_STRING_PLURAL = (const char** const) &REAL_PRIVILEGE_STRING_PLURAL[5];

enum ScoringSystem {
    ALMONASTER_SCORE,
    CLASSIC_SCORE,
    BRIDIER_SCORE,
    BRIDIER_SCORE_ESTABLISHED,
    TOURNAMENT_SCORING,
    NUM_SCORING_SYSTEMS
};

#define FIRST_SCORING_SYSTEM ALMONASTER_SCORE

#define ENUMERATE_SCORING_SYSTEMS(i) for (i = ALMONASTER_SCORE; i < NUM_SCORING_SYSTEMS; i ++)

#define MAX_SCORING_SYSTEM_COLUMNS 2

extern const char* const TOPLIST_NAME [NUM_SCORING_SYSTEMS];
extern const char* const TOPLIST_TABLE_NAME [NUM_SCORING_SYSTEMS];
extern const unsigned int TOPLIST_NUM_COLUMNS [NUM_SCORING_SYSTEMS];
extern const unsigned int TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [NUM_SCORING_SYSTEMS];
extern const unsigned int* TOPLIST_SYSTEM_EMPIRE_DATA_COLUMNS [NUM_SCORING_SYSTEMS];

#define PRIVILEGE_FOR_ADVANCED_SEARCH       APPRENTICE
#define PRIVILEGE_FOR_PERSONAL_GAMES        APPRENTICE
#define PRIVILEGE_FOR_PERSONAL_GAMECLASSES  ADEPT
#define PRIVILEGE_FOR_PERSONAL_TOURNAMENTS  ADEPT

// Don't change this unless you know what you're doing!
#define TOPLIST_SIZE 20

enum DatabaseBackupStage {
    DATABASE_BACKUP_NONE,
    DATABASE_BACKUP_TABLES,
    DATABASE_BACKUP_TEMPLATES,
    DATABASE_BACKUP_VARIABLE_LENGTH_DATA,
    DATABASE_BACKUP_METADATA,
};

/////////////////////////////
// Bridier score constants //
/////////////////////////////

enum BridierColumns {
    BRIDIER_RANK,
    BRIDIER_INDEX,
    NUM_BRIDIER_COLUMNS,
};

#define BRIDIER_MIN_RANK 100
#define BRIDIER_INITIAL_RANK 500
#define BRIDIER_MAX_RANK 0x7fffffff

#define BRIDIER_MIN_INDEX 100
#define BRIDIER_INITIAL_INDEX 500
#define BRIDIER_MAX_INDEX 500

#define BRIDIER_MIN_RANK_GAIN 0
#define BRIDIER_MAX_RANK_GAIN 0x7fffffff

#define BRIDIER_MIN_RANK_LOSS 0
#define BRIDIER_MAX_RANK_LOSS 0x7fffffff

#define BRIDIER_TOPLIST_INDEX 495
#define BRIDIER_ESTABLISHED_TOPLIST_INDEX 100

/////////////////////////////
// Classic score constants //
/////////////////////////////

// Classic mininum possible score
#define CLASSIC_MIN_SCORE ((float) -0x7fffffff)

// Classic maximum possible score
#define CLASSIC_MAX_SCORE ((float) 0x7fffffff)

#define CLASSIC_INITIAL_SCORE ((float) 0)

// Classic score increase for a win
#define CLASSIC_POINTS_FOR_WIN ((float) 1.0)

// Classic score increase for a draw
#define CLASSIC_POINTS_FOR_DRAW ((float) 0.25)

// Classic score increase for a nuke
#define CLASSIC_POINTS_FOR_NUKE ((float) 0.5)

// Classic score increase for a nuked
#define CLASSIC_POINTS_FOR_NUKED ((float) -0.5)

// Classic score increase for a ruin
#define CLASSIC_POINTS_FOR_RUIN ((float) -1.0)

////////////////////////////////
// Almonaster score constants //
////////////////////////////////

// Almonaster points for a nuke on an empire of equivalent quality to oneself
#define ALMONASTER_BASE_UNIT ((float) 10.0)

// Almonaster mininum possible score
#define ALMONASTER_MIN_SCORE ((float) 10.0)

// Almonaster maximum possible score
#define ALMONASTER_MAX_SCORE ((float) 0x7fffffff)

// Almonaster initial score
#define ALMONASTER_INITIAL_SCORE ((float) 10.0)

// Almonaster maximum increase factor:  
// number which, all other factors being equal, gives the maximum increase 
// on a nuke by multiplying it by the base unit above
#define ALMONASTER_MAX_INCREASE_FACTOR ((float) 5.0)

// Same as above, only for score decreases
#define ALMONASTER_MAX_DECREASE_FACTOR ((float) 2.5)

// Limits for alliance ratio
#define ALMONASTER_MIN_ALLIANCE_RATIO ((float) 0.25)
#define ALMONASTER_MAX_ALLIANCE_RATIO ((float) 4.0)

// Limits for significance ratio
#define ALMONASTER_MIN_NUKER_SIGNIFICANCE_RATIO ((float) 0.5)
#define ALMONASTER_MAX_NUKER_SIGNIFICANCE_RATIO ((float) 1.0)

#define ALMONASTER_MIN_NUKED_SIGNIFICANCE_RATIO ((float) 0.2)
#define ALMONASTER_MAX_NUKED_SIGNIFICANCE_RATIO ((float) 1.0)

// Wins
#define MAX_NUM_WINS 0x7fffffff
#define MIN_NUM_WINS 0

////////////////////
// Game constants //
////////////////////

#define IS_KEY(x) (((int) x) >= 0)

#define SYSTEM ((unsigned int) 0xffffffed)

#define INDEPENDENT_NAME "Independent"

#define INDEPENDENT ((unsigned int) 0xfffffffd)

#define TOURNAMENT ((unsigned int) 0xfffffffc)
#define PERSONAL_GAME ((unsigned int) 0xfffffffb)
#define DELETED_EMPIRE_KEY ((unsigned int) 0xfffffffe)

#define ROOT_KEY 0
#define ROOT_NAME "root"
#define ROOT_DEFAULT_PASSWORD "neil:young"

extern const char* const RESERVED_EMPIRE_NAMES[3];

#define NUM_RESERVED_EMPIRE_NAMES (sizeof (RESERVED_EMPIRE_NAMES) / sizeof (const char*))

#define GUEST_KEY 1
#define GUEST_NAME "guest"
#define GUEST_DEFAULT_PASSWORD "crazy:horse"

#define NO_SESSION_ID ((int64) 0)

#define PARTIAL_MAP_UNLIMITED_RADIUS (-5)
#define PARTIAL_MAP_NATURAL_CENTER (-5)

#define UNINITIALIZED_KEY (-10)

// Time considered a "weekend", expressed in seconds
#define ONE_YEAR_IN_SECONDS (31536000)
#define WEEKEND_LENGTH_IN_SECONDS (60 * 60 * 24 * 2)
#define DAY_LENGTH_IN_SECONDS (60 * 60 * 24)
#define MAX_AFTER_WEEKEND_DELAY (60 * 60 * 12)

// Default min, max values for map coordinates
#define MIN_COORDINATE ((int) -0x7fffffff)
#define MAX_COORDINATE ((int) 0x7fffffff)

// Floating point proximity tolerance
#define FLOAT_PROXIMITY_TOLERANCE ((float) 0.001)

// Maximum value of a ratio
#define MAX_RATIO ((float) 1000.0)

// Max ag ratio
#define MIN_MAX_AG_RATIO ((float) 1.25)
#define MAX_MAX_AG_RATIO_WITHOUT_SWEEPERS ((float) 2.0)

// Database purge criteria
#define NEVER_PLAYED_A_GAME             (0x00000001)
#define NEVER_WON_A_GAME                (0x00000002)
#define ONLY_ONE_LOGIN                  (0x00000004)
#define CLASSIC_SCORE_IS_ZERO_OR_LESS   (0x00000008)
#define LAST_LOGGED_IN_A_MONTH_AGO      (0x00000010)
#define TEST_PURGE_ONLY                 (0x00000020)

// Diplomacy
#define ALL_WAR (-6)
#define ALL_TRUCE (-7)
#define ALL_TRADE (-8)
#define ALL_ALLIANCE (-9)

// Ship actions
#define STAND_BY (-1)
#define MOVE_NORTH (-2)
#define MOVE_EAST (-3)
#define MOVE_SOUTH (-4)
#define MOVE_WEST (-5)
#define NUKE (-6)
#define DISMANTLE (-7)
#define BUILD_AT (-8)
#define CANCEL_BUILD (-9)
#define FLEET (-10)
#define LEAVE_FLEET (-11)

// Fleet actions
#define DISBAND (-20)
#define DISBAND_AND_DISMANTLE (-21)
#define PICK_UP_UNAFFILIATED_SHIPS (-22)

// Special actions
#define EXPLORE_NORTH (-30)
#define EXPLORE_EAST (-31)
#define EXPLORE_SOUTH (-32)
#define EXPLORE_WEST (-33)
#define COLONIZE (-40)
#define DEPOSIT_POP (-41)
//#define COLONIZE_OR_DEPOSIT_POP (-42)
#define CLOAK (-45)
#define UNCLOAK (-46)
#define TERRAFORM (-50)
#define DETONATE (-51)
#define INVADE (-52)
#define ANNIHILATE (-55)
#define OPEN_LINK_NORTH (-60)
#define OPEN_LINK_EAST (-61)
#define OPEN_LINK_SOUTH (-62)
#define OPEN_LINK_WEST (-63)
#define CLOSE_LINK_NORTH (-64)
#define CLOSE_LINK_EAST (-65)
#define CLOSE_LINK_SOUTH (-66)
#define CLOSE_LINK_WEST (-67)
#define BUILD_INTO_FLEET (-68)

// ... and dismantle
//#define COLONIZE_AND_DISMANTLE (-69)
//#define DEPOSIT_POP_AND_DISMANTLE (-70)
//#define COLONIZE_OR_DEPOSIT_POP_AND_DISMANTLE (-71)
#define TERRAFORM_AND_DISMANTLE (-72)
#define INVADE_AND_DISMANTLE (-73)

// Create planet
#define CREATE_PLANET_NORTH (-74)
#define CREATE_PLANET_EAST (-75)
#define CREATE_PLANET_SOUTH (-76)
#define CREATE_PLANET_WEST (-77)

// Morph
#define MORPH_BASETECH      (-78)
#define MORPH_ATTACK        (MORPH_BASETECH - ATTACK)
#define MORPH_SCIENCE       (MORPH_BASETECH - SCIENCE)
#define MORPH_COLONY        (MORPH_BASETECH - COLONY)
#define MORPH_STARGATE      (MORPH_BASETECH - STARGATE)
#define MORPH_CLOAKER       (MORPH_BASETECH - CLOAKER)
#define MORPH_SATELLITE     (MORPH_BASETECH - SATELLITE)
#define MORPH_TERRAFORMER   (MORPH_BASETECH - TERRAFORMER)
#define MORPH_TROOPSHIP     (MORPH_BASETECH - TROOPSHIP)
#define MORPH_DOOMSDAY      (MORPH_BASETECH - DOOMSDAY)
#define MORPH_MINEFIELD     (MORPH_BASETECH - MINEFIELD)
#define MORPH_MINESWEEPER   (MORPH_BASETECH - MINESWEEPER)
#define MORPH_ENGINEER      (MORPH_BASETECH - ENGINEER)
#define MORPH_CARRIER       (MORPH_BASETECH - CARRIER)
#define MORPH_BUILDER       (MORPH_BASETECH - BUILDER)
#define MORPH_MORPHER       (MORPH_BASETECH - MORPHER)
#define MORPH_JUMPGATE      (MORPH_BASETECH - JUMPGATE)

#define MORPH_TECH(x) (MORPH_BASETECH - x)
#define IS_MORPH_ACTION(x) ((x) <= MORPH_ATTACK && (x) >= MORPH_JUMPGATE)

// Gate
#define GATE_SHIPS (-100)

// Fleet Standby and X
#define FLEET_STANDBY_BASE              (-200)
#define FLEET_STANDBY_AND_COLONIZE      (FLEET_STANDBY_BASE - COLONY)
#define FLEET_STANDBY_AND_TERRAFORM     (FLEET_STANDBY_BASE - TERRAFORMER)
#define FLEET_STANDBY_AND_INVADE        (FLEET_STANDBY_BASE - TROOPSHIP)
#define FLEET_STANDBY_AND_ANNIHILATE    (FLEET_STANDBY_BASE - DOOMSDAY)

#define FLEET_STANDBY_NUM_ACTIONS       (4)
#define FLEET_STANDBY_TECHMASK          (TECH_COLONY | TECH_TERRAFORMER | TECH_TROOPSHIP | TECH_DOOMSDAY)

#define FLEET_STANDBY_ORDER_FROM_TECH(x) (FLEET_STANDBY_BASE - x)
#define FLEET_STANDBY_TECH_FROM_ORDER(x) (FLEET_STANDBY_BASE - x)

#define IS_FLEET_STANDBY_ACTION(x) ((x) <= FLEET_STANDBY_AND_COLONIZE && (x) >= FLEET_STANDBY_AND_ANNIHILATE)

extern const int FLEET_ACTION_FOR_TECH [NUM_SHIP_TYPES];

////////////
// Errors //
////////////

#define ERROR_TOO_MANY_GAMECLASSES (-1000)
#define ERROR_COULD_NOT_CREATE_PLANETS (-1001)
#define ERROR_ALREADY_IN_GAME (-1002)
#define ERROR_GAME_HAS_NOT_CLOSED (-1003)
#define ERROR_GAMECLASS_HALTED (-1004)
#define ERROR_GAME_DOES_NOT_EXIST (-1005)
#define ERROR_GAME_CLOSED (-1006)
#define ERROR_PASSWORD (-1007)
#define ERROR_GAME_HAS_NOT_STARTED (-1008)
#define ERROR_WRONG_OWNER (-1009)
#define ERROR_NAME_IS_IN_USE (-1010)
#define ERROR_EMPTY_NAME (-1011)
#define ERROR_ORPHANED_FLEET (-1012)
#define ERROR_SHIP_DOES_NOT_EXIST (-1013)
#define ERROR_FLEET_DOES_NOT_EXIST (-1014)
#define ERROR_WRONG_PLANET (-1015)
#define ERROR_INSUFFICIENT_POPULATION (-1016)
#define ERROR_WRONG_TECHNOLOGY (-1017)
#define ERROR_INVALID_TECH_LEVEL (-1018)
#define ERROR_CANNOT_MOVE (-1019)
#define ERROR_CANNOT_EXPLORE (-1020)
#define ERROR_CANNOT_COLONIZE (-1021)
#define ERROR_CANNOT_SETTLE (-1022)
#define ERROR_WRONG_SHIP_TYPE (-1023)
#define ERROR_CANNOT_CLOAK (-1024)
#define ERROR_CANNOT_UNCLOAK (-1025)
#define ERROR_CANNOT_TERRAFORM (-1026)
#define ERROR_CANNOT_NUKE (-1027)
#define ERROR_CANNOT_ANNIHILATE (-1028)
#define ERROR_CANNOT_OPEN_LINK (-1029)
#define ERROR_CANNOT_CLOSE_LINK (-1030)
#define ERROR_UNKNOWN_ORDER (-1031)
#define ERROR_CANNOT_GATE (-1032)
#define ERROR_CANNOT_INVADE (-1033)
#define ERROR_GAMECLASS_ALREADY_EXISTS (-1034)
#define ERROR_THEME_ALREADY_EXISTS (-1035)
#define ERROR_ALIEN_ICON_ALREADY_EXISTS (-1036)
#define ERROR_ALIEN_ICON_DOES_NOT_EXIST (-1037)
#define ERROR_LAST_ALIEN_ICON (-1038)
#define ERROR_EMPIRE_IS_IN_GAMES (-1039)
#define ERROR_EMPIRE_DOES_NOT_EXIST (-1040)
#define ERROR_EMPIRE_IS_NOT_AN_ADMINISTRATOR (-1041)
#define ERROR_EMPIRE_IS_ALREADY_AN_ADMINISTRATOR (-1042)
#define ERROR_EMPIRE_IS_HALTED (-1043)
#define ERROR_CANNOT_UNDELETE_EMPIRE (-1044)
#define ERROR_EMPIRE_IS_NOT_HALTED (-1045)
#define ERROR_NULL_PASSWORD (-1046)
#define ERROR_DEFAULT_ALIEN_ICON (-1047)
#define ERROR_SUPERCLASS_ALREADY_EXISTS (-1048)
#define ERROR_SAME_SHIP_ORDER (-1049)
#define ERROR_SHIP_IS_NOT_IN_FLEET (-1050)
#define ERROR_CANNOT_SEND_MESSAGE (-1051)
#define ERROR_NO_MESSAGES (-1052)
#define ERROR_EMPIRE_IS_NOT_IN_GAME (-1053)
#define ERROR_NO_TECHNOLOGY_AVAILABLE (-1054)
#define ERROR_UNKNOWN_TOPLIST (-1055)
#define ERROR_EMPIRE_IS_NOT_IN_DIPLOMACY (-1056)
#define ERROR_EMPIRE_IS_IGNORING_SENDER (-1057)
#define ERROR_DISABLED (-1058)
#define ERROR_LAST_EMPIRE_CANNOT_AUTOUPDATE (-1059)
#define ERROR_COULD_NOT_DELETE_EMPIRE (-1060)
#define ERROR_COULD_NOT_DELETE_ADMINISTRATOR (-1061)
#define ERROR_GAMECLASS_HAS_NO_SUPERCLASS (-1062)
#define ERROR_CANNOT_MODIFY_ROOT (-1063)
#define ERROR_SAME_FLEET_ORDER (-1064)
#define ERROR_WRONG_NUMBER_OF_SHIPS (-1065)
#define ERROR_INSUFFICIENT_PRIVILEGE (-1066)
#define ERROR_GAMECLASS_DOES_NOT_EXIST (-1067)
#define ERROR_NAME_IS_TOO_LONG (-1068)
#define ERROR_RESERVED_EMPIRE_NAME (-1069)
#define ERROR_INSUFFICIENT_POPULATION_FOR_COLONIES (-1070)
#define ERROR_EMPIRE_IS_IGNORING_BROADCASTS (-1071)
#define ERROR_EMPIRE_ALREADY_EXISTS (-1072)
#define ERROR_GAMECLASS_DELETED (-1073)
#define ERROR_GAMECLASS_NOT_MARKED_FOR_DELETION (-1074)
#define ERROR_GAMECLASS_NOT_HALTED (-1075)
#define ERROR_CANNOT_MODIFY_GUEST (-1076)
#define ERROR_GAME_HAS_STARTED (-1077)
#define ERROR_DUPLICATE_IP_ADDRESS (-1078)
#define ERROR_DUPLICATE_SESSION_ID (-1079)
#define ERROR_INVALID_PRIVILEGE (-1080)
#define ERROR_SUPERCLASS_DOES_NOT_EXIST (-1081)
#define ERROR_MESSAGE_DOES_NOT_EXIST (-1082)
#define ERROR_NO_EMPIRES_EXIST (-1083)
#define ERROR_PLANET_DOES_NOT_BELONG_TO_EMPIRE (-1084)
#define ERROR_TOO_MANY_SPEAKERS (-1085)
#define ERROR_NOT_IN_CHATROOM (-1086)
#define ERROR_TOO_MANY_GAMES (-1087)
#define ERROR_DESCRIPTION_IS_TOO_LONG (-1088)
#define ERROR_UNBREAKABLE_ALLIANCE (-1089)
#define ERROR_NO_PLANETS_AVAILABLE (-1090)
#define ERROR_NO_NEW_PLANETS_AVAILABLE (-1091)
#define ERROR_INVALID_QUERY (-1092)
#define ERROR_NOT_SPECTATOR_GAME (-1093)
#define ERROR_NOT_ENOUGH_EMPIRES (-1094)
#define ERROR_PLANET_EXISTS (-1095)
#define ERROR_CANNOT_MORPH (-1096)
#define ERROR_TOPLIST_TOO_LARGE (-1097)
#define ERROR_TOPLIST_CORRUPT (-1098)
#define ERROR_EMPIRE_CANNOT_SEE_PLANET (-1099)
#define ERROR_EMPIRE_HAS_NO_SHIPS (-1100)
#define ERROR_SHIP_LIMIT_REACHED (-1101)
#define ERROR_TOURNAMENT_ALREADY_EXISTS (-1102)
#define ERROR_WRONG_TOURNAMENT_OWNER (-1103)
#define ERROR_TOURNAMENT_DOES_NOT_EXIST (-1104)
#define ERROR_EMPIRE_IS_NOT_IN_TOURNAMENT (-1105)
#define ERROR_TOURNAMENT_TEAM_ALREADY_EXISTS (-1106)
#define ERROR_TOURNAMENT_TEAM_DOES_NOT_EXIST (-1107)
#define ERROR_GAMECLASS_IS_NOT_IN_TOURNAMENT (-1108)
#define ERROR_TOO_MANY_EMPIRES (-1109)
#define ERROR_EMPIRE_IS_ALREADY_IN_TOURNAMENT (-1110)
#define ERROR_TOURNAMENT_HAS_GAMECLASSES (-1111)
#define ERROR_TOURNAMENT_HAS_GAMES (-1112)
#define ERROR_ALLIANCE_LIMIT_EXCEEDED (-1113)
#define ERROR_WAS_ALREADY_IN_GAME (-1114)
#define ERROR_EMPIRE_IS_UNAVAILABLE_FOR_TOURNAMENTS (-1115)
#define ERROR_TOO_MANY_TOURNAMENTS (-1116)
#define ERROR_TOO_MANY_GAMECLASSES_IN_TOURNAMENT (-1117)
#define ERROR_INVALID_FLEET_ORDER (-1118)
#define ERROR_FLEET_NOT_ON_PLANET (-1119)
#define ERROR_SHIP_CANNOT_JOIN_FLEET (-1120)
#define ERROR_SHIP_ALREADY_BUILT (-1121)

#endif