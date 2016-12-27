//
// Almonaster.dll:  Almonaster
// Copyright (c) 1998-2004 Max Attar Feingold (maf6@cornell.edu)
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

#ifndef _GameEngineUI_H_
#define _GameEngineUI_H_

// Null Theme colors
#define DEFAULT_TABLE_COLOR "202050"
#define DEFAULT_TEXT_COLOR "F7EFCE"
#define DEFAULT_GOOD_COLOR "00FF00"
#define DEFAULT_BAD_COLOR "FF4040"
#define DEFAULT_PRIVATE_MESSAGE_COLOR "80FFFF"
#define DEFAULT_BROADCAST_MESSAGE_COLOR "FFFF00"

// Update algorithm macros
#define BEGIN_LARGE_FONT "<font size=\"+1\">"
#define BEGIN_GOOD_FONT(i) (String) "<font color=\"" + pvGoodColor[i].GetCharPtr() + "\">"
#define BEGIN_BAD_FONT(i) (String) "<font color=\""  + pvBadColor[i].GetCharPtr() +  "\">"

#define END_FONT "</font>"

#define BEGIN_STRONG "<strong>"
#define END_STRONG "</strong>"

#define NEW_PARAGRAPH "<p>"

#endif