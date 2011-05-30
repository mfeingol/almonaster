
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the GameServerRules page
int HtmlRenderer::Render_GameServerRules() {

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

	INITIALIZE_EMPIRE

	INITIALIZE_GAME

	GAME_REDIRECT_ON_SUBMIT

	GAME_OPEN

	// Individual page stuff starts here
	WriteServerRules();

	GAME_CLOSE


}