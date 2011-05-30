//
// Almonaster.dll:  Almonaster 2.0
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

#ifndef _GameEngineStrings_H_
#define _GameEngineStrings_H_

//
// Non-database constants
// Can be changed arbitrarily
//

// Reason length
#define MAX_REASON_LENGTH 768

// Real name
#define MAX_REAL_NAME_LENGTH 512

// Location
#define MAX_LOCATION_LENGTH 256

// Email address
#define MAX_EMAIL_LENGTH 512

// Webpage URL
#define MAX_WEB_PAGE_LENGTH 512

// Instant messaging Id
#define MAX_IMID_LENGTH 128

// Quote
#define MAX_QUOTE_LENGTH 8192

// Victory sneer
#define MAX_VICTORY_SNEER_LENGTH 8192

// Local path for graphics
#define MAX_GRAPHICS_ALTERNATIVE_PATH_LENGTH 512

// Gameclass description field
#define MAX_GAMECLASS_DESCRIPTION_LENGTH 384

// Initial gameclass message length
#define MAX_ENTER_GAME_MESSAGE_LENGTH 512

// Notepad
#define MAX_NOTEPAD_LENGTH 16384

// Tournament description
#define MAX_TOURNAMENT_DESCRIPTION_LENGTH 384

// Tournament team description
#define MAX_TOURNAMENT_TEAM_DESCRIPTION_LENGTH 384

// Tournament team description
#define MAX_TOURNAMENT_NEWS_LENGTH 512

///////////////////////////////////////////////////////////////////////////////////////////
// These values cannot be changed without running dbconv on the affected database tables //
///////////////////////////////////////////////////////////////////////////////////////////

// Server name
#define MAX_SERVER_NAME_LENGTH 35

// Ship name
#define MAX_SHIP_NAME_LENGTH 23

// Empire name
#define MAX_EMPIRE_NAME_LENGTH 27

// Empire password
#define MAX_PASSWORD_LENGTH 27

// IP Address
#define MAX_IP_ADDRESS_LENGTH 15

// Custom table color (RGB)
#define MAX_COLOR_LENGTH 6

// Game class names
#define MAX_GAME_CLASS_NAME_LENGTH 43

// Sclass names
#define MAX_SUPER_CLASS_NAME_LENGTH 43

// Alien author name
#define MAX_ALIEN_AUTHOR_NAME_LENGTH 43

// Theme author name
#define MAX_THEME_AUTHOR_NAME_LENGTH MAX_ALIEN_AUTHOR_NAME_LENGTH

// Theme author email
#define MAX_THEME_AUTHOR_EMAIL_LENGTH 43

// Theme name
#define MAX_THEME_NAME_LENGTH 43

// Theme version
#define MAX_THEME_VERSION_LENGTH 11

// Theme description
#define MAX_THEME_DESCRIPTION_LENGTH 75

// Theme file name
#define MAX_THEME_FILE_NAME_LENGTH 27

// Planet names
#define MAX_PLANET_NAME_LENGTH 36

// Fleet names
#define MAX_FLEET_NAME_LENGTH MAX_SHIP_NAME_LENGTH

// Number . number
#define MAX_GAMECLASS_GAMENUMBER_LENGTH 23

// XCoordinate . YCoordinate
#define MAX_COORDINATE_LENGTH 23

// Browser name
#define MAX_BROWSER_NAME_LENGTH 103

// Tournament name
#define MAX_TOURNAMENT_NAME_LENGTH MAX_EMPIRE_NAME_LENGTH

// Tournament team name
#define MAX_TOURNAMENT_TEAM_NAME_LENGTH 35

// Misc
#define MAX_FULL_GAME_CLASS_NAME_LENGTH (MAX_GAME_CLASS_NAME_LENGTH + MAX_TOURNAMENT_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH + 10)

#define MAX_PLANET_NAME_WITH_COORDINATES_LENGTH (MAX_PLANET_NAME_LENGTH + MAX_COORDINATE_LENGTH + 5)

#endif