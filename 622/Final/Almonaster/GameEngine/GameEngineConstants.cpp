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

const char* const SHIP_TYPE_STRING[] = {
    "Attack",
    "Science",
    "Colony",
    "Stargate",
    "Cloaker",
    "Satellite",
    "Terraformer",
    "Troopship",
    "Doomsday",
    "Minefield",
    "Minesweeper",
    "Engineer",
    "Carrier",
    "Builder",
    "Morpher",
    "Jumpgate",
};

const char* const SHIP_TYPE_STRING_PLURAL[] = {
    "Attacks",
    "Sciences",
    "Colonies",
    "Stargates",
    "Cloakers",
    "Satellites",
    "Terraformers",
    "Troopships",
    "Doomsdays",
    "Minefields",
    "Minesweepers",
    "Engineers",
    "Carriers",
    "Builders",
    "Morphers",
    "Jumpgates",
};

const char* const SHIP_TYPE_STRING_LOWERCASE[] = {
    "attack",
    "science",
    "colony",
    "stargate",
    "cloaker",
    "satellite",
    "terraformer",
    "troopship",
    "doomsday",
    "minefield",
    "minesweeper",
    "engineer",
    "carrier",
    "builder",
    "morpher",
    "jumpgate",
};

const char* const SHIP_TYPE_STRING_LOWERCASE_PLURAL[] = {
    "attacks",
    "sciences",
    "colonies",
    "stargates",
    "cloakers",
    "satellites",
    "terraformers",
    "troopships",
    "doomsdays",
    "minefields",
    "minesweepers",
    "engineers",
    "carriers",
    "builders",
    "morphers",
    "jumpgates",
};

const char* const SHIP_TYPE_DESCRIPTION[] = {
    "Workhorse for defense and attack",                         // Attack
    "Explores unmapped planets",                                // Science
    "Colonizes and settles uninhabited planets",                // Colony
    "Teleports ships to planets belonging to the same empire",  // Stargate
    "Makes itself invisible to other empires",                  // Cloaker
    "Cheap immobile defensive force",                           // Satellite
    "Increases planetary agricultural resources",               // Terraformer
    "Invades enemy planets",                                    // Troopship
    "Makes planets uninhabitable",                              // Doomsday
    "Detonates, destroying enemy fleets",                       // Minefield
    "Nullifies the effects of minefield explosions",            // Minesweeper
    "Opens and closes links between planets",                   // Engineer
    "Absorbs enemy fire during combat",                         // Carrier
    "Builds new planets",                                       // Builder
    "Changes into other ship types",                            // Morpher
    "Teleports ships to other planets within range",            // Jumpgate
};

const int FLEET_ACTION_FOR_TECH [NUM_SHIP_TYPES] = {
    NO_KEY,
    NO_KEY,
    COLONIZE,
    NO_KEY,
    NO_KEY,
    NO_KEY,
    TERRAFORM,
    INVADE,
    ANNIHILATE,
    NO_KEY,
    NO_KEY,
    NO_KEY,
    NO_KEY,
    NO_KEY,
    NO_KEY,
    NO_KEY,
};



const unsigned int SYSTEM_DATA_SHIP_NAME_COLUMN [] = {

    SystemData::DefaultAttackName,
    SystemData::DefaultScienceName,
    SystemData::DefaultColonyName,
    SystemData::DefaultStargateName,
    SystemData::DefaultCloakerName,
    SystemData::DefaultSatelliteName,
    SystemData::DefaultTerraformerName,
    SystemData::DefaultTroopshipName,
    SystemData::DefaultDoomsdayName,
    SystemData::DefaultMinefieldName,
    SystemData::DefaultMinesweeperName,
    SystemData::DefaultEngineerName,
    SystemData::DefaultCarrierName,
    SystemData::DefaultBuilderName,
    SystemData::DefaultMorpherName,
    SystemData::DefaultJumpgateName,
};

const unsigned int SYSTEM_EMPIRE_DATA_SHIP_NAME_COLUMN[] = {

    SystemEmpireData::DefaultAttackName,
    SystemEmpireData::DefaultScienceName,
    SystemEmpireData::DefaultColonyName,
    SystemEmpireData::DefaultStargateName,
    SystemEmpireData::DefaultCloakerName,
    SystemEmpireData::DefaultSatelliteName,
    SystemEmpireData::DefaultTerraformerName,
    SystemEmpireData::DefaultTroopshipName,
    SystemEmpireData::DefaultDoomsdayName,
    SystemEmpireData::DefaultMinefieldName,
    SystemEmpireData::DefaultMinesweeperName,
    SystemEmpireData::DefaultEngineerName,
    SystemEmpireData::DefaultCarrierName,
    SystemEmpireData::DefaultBuilderName,
    SystemEmpireData::DefaultMorpherName,
    SystemEmpireData::DefaultJumpgateName,
};

const int MORPH_ORDER[] = {
    MORPH_ATTACK,
    MORPH_SCIENCE,
    MORPH_COLONY,
    MORPH_STARGATE,
    MORPH_CLOAKER,
    MORPH_SATELLITE,
    MORPH_TERRAFORMER,
    MORPH_TROOPSHIP,
    MORPH_DOOMSDAY,
    MORPH_MINEFIELD,
    MORPH_MINESWEEPER,
    MORPH_ENGINEER,
    MORPH_CARRIER,
    MORPH_BUILDER,
    MORPH_MORPHER,  // Morpher
    MORPH_JUMPGATE,
};

const char* DIPLOMACY_STRING[] = {
    WAR_STRING,
    TRUCE_STRING,
    TRADE_STRING,
    0,
    ALLIANCE_STRING,
    SURRENDER_STRING,
    ACCEPT_SURRENDER_STRING,
};

const CardinalPoint OPPOSITE_CARDINAL_POINT [NUM_CARDINAL_POINTS] = {
    SOUTH,
    WEST,
    NORTH,
    EAST
};

const char* const CARDINAL_STRING [NUM_CARDINAL_POINTS] = {
    "North",
    "East",
    "South",
    "West"
};

const char* const OPPOSITE_CARDINAL_STRING [NUM_CARDINAL_POINTS] = {
    "South",
    "West",
    "North",
    "East"
};

const int CLOSE_LINK [NUM_CARDINAL_POINTS] = {
    CLOSE_LINK_NORTH,
    CLOSE_LINK_EAST,
    CLOSE_LINK_SOUTH,
    CLOSE_LINK_WEST
};

const int OPEN_LINK [NUM_CARDINAL_POINTS] = {
    OPEN_LINK_NORTH,
    OPEN_LINK_EAST,
    OPEN_LINK_SOUTH,
    OPEN_LINK_WEST
};

const int CREATE_PLANET_LINK [NUM_CARDINAL_POINTS] = {
    CREATE_PLANET_NORTH,
    CREATE_PLANET_EAST,
    CREATE_PLANET_SOUTH,
    CREATE_PLANET_WEST
};

extern const EmpireGender EMPIRE_GENDER[EMPIRE_NUM_GENDERS] = {
    EMPIRE_GENDER_UNKNOWN,
    EMPIRE_GENDER_MALE,
    EMPIRE_GENDER_FEMALE
};

const char* const EMPIRE_GENDER_STRING[EMPIRE_NUM_GENDERS] = {
    "N/A",
    "Male",
    "Female",
};

const Privilege PrivilegeKey [NUM_PRIVILEGES] = {
    GUEST,
    NOVICE,
    APPRENTICE,
    ADEPT,
    ADMINISTRATOR
};

const char* const REAL_PRIVILEGE_STRING [10] = {
    "Guest",
    NULL,
    NULL,
    NULL,
    NULL,
    "Novice",
    "Apprentice",
    "Adept",
    "Administrator",
    NULL
};

const char* const REAL_PRIVILEGE_STRING_PLURAL [10] = {
    "Guests",
    NULL,
    NULL,
    NULL,
    NULL,
    "Novices",
    "Apprentices",
    "Adepts",
    "Administrators",
    NULL
};

const char* const TOPLIST_NAME [NUM_SCORING_SYSTEMS] = {
    "Almonaster Score",
    "Classic Score",
    "Bridier Score",
    "Established Bridier Score",
};

const char* const TOPLIST_TABLE_NAME [NUM_SCORING_SYSTEMS] = {
    SYSTEM_ALMONASTER_SCORE_TOPLIST,
    SYSTEM_CLASSIC_SCORE_TOPLIST,
    SYSTEM_BRIDIER_SCORE_TOPLIST,
    SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST,
};

const unsigned int TOPLIST_NUM_COLUMNS [NUM_SCORING_SYSTEMS] = {
    SystemAlmonasterScoreTopList::NumColumns,
    SystemClassicScoreTopList::NumColumns,
    SystemBridierScoreTopList::NumColumns,
    SystemBridierScoreEstablishedTopList::NumColumns,
};

const unsigned int TOPLIST_SYSTEM_EMPIRE_DATA_NUM_COLUMNS [NUM_SCORING_SYSTEMS] = {
    SystemAlmonasterScoreTopList::NumColumns - 1,
    SystemClassicScoreTopList::NumColumns - 1,
    SystemBridierScoreTopList::NumColumns - 1,
    SystemBridierScoreEstablishedTopList::NumColumns - 1,
};

const unsigned int ALMONASTER_SCORE_TOPLIST_COLUMNS[] = {
    SystemEmpireData::AlmonasterScore,
};

const unsigned int CLASSIC_SCORE_TOPLIST_COLUMNS[] = {
    SystemEmpireData::ClassicScore,
};

const unsigned int BRIDIER_SCORE_TOPLIST_COLUMNS[] = {
    SystemEmpireData::BridierRank,
    SystemEmpireData::BridierIndex,
};

const unsigned int BRIDIER_SCORE_ESTABLISHED_TOPLIST_COLUMNS[] = {
    SystemEmpireData::BridierRank,
    SystemEmpireData::BridierIndex,
};

const unsigned int* TOPLIST_SYSTEM_EMPIRE_DATA_COLUMNS [NUM_SCORING_SYSTEMS] = {
    ALMONASTER_SCORE_TOPLIST_COLUMNS,
    CLASSIC_SCORE_TOPLIST_COLUMNS,
    BRIDIER_SCORE_TOPLIST_COLUMNS,
    BRIDIER_SCORE_ESTABLISHED_TOPLIST_COLUMNS,
};

const char* const RESERVED_EMPIRE_NAMES[3] = {
    "Independent",
    "System",
    "The System",
};

const int EXPLORED_X[NUM_CARDINAL_POINTS] = {
    EXPLORED_NORTH,
    EXPLORED_EAST,
    EXPLORED_SOUTH,
    EXPLORED_WEST
};

const int OPPOSITE_EXPLORED_X[] = {
    EXPLORED_SOUTH,
    EXPLORED_WEST,
    EXPLORED_NORTH,
    EXPLORED_EAST
};

const int LINK_X[] = {
    LINK_NORTH,
    LINK_EAST,
    LINK_SOUTH,
    LINK_WEST
};

const int OPPOSITE_LINK_X[] = {
    LINK_SOUTH,
    LINK_WEST,
    LINK_NORTH,
    LINK_EAST
};

const int TECH_BITS[] = {
    TECH_ATTACK,
    TECH_SCIENCE, 
    TECH_COLONY,
    TECH_STARGATE, 
    TECH_CLOAKER,
    TECH_SATELLITE,
    TECH_TERRAFORMER,
    TECH_TROOPSHIP,
    TECH_DOOMSDAY,
    TECH_MINEFIELD,
    TECH_MINESWEEPER,
    TECH_ENGINEER,
    TECH_CARRIER,
    TECH_BUILDER,
    TECH_MORPHER,
    TECH_JUMPGATE,
};

const char* const REMOVAL_REASON_STRING[] = {
    "Unknown",
    "Nuked",
    "Surrendered",
    "Ruined",
    "Terraformed",
    "Invaded",
    "Annihilated",
    "Colonized",
    "Removed by an administrator",
    "Quit",
    "Game ended",
    "Error entering the game",
};