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

#ifndef _Almonaster_H_
#define _Almonaster_H_

#include "Alajar.h"

#include "GameEngine/GameEngineConstants.h"
#include "GameEngine/GameEngine.h"

// Forward declarations
class HtmlRenderer;
class Chatroom;

// Global objects
extern IHttpServer* g_pHttpServer;
extern IReport* g_pReport;
extern IConfigFile* g_pConfig;
extern ILog* g_pLog;
extern IPageSourceControl* g_pPageSourceControl;
extern IFileCache* g_pFileCache;

extern GameEngine* g_pGameEngine;

extern char* g_pszResourceDir;

#endif