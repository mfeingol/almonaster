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

#include "AlmonasterEventSink.h"
#include "HtmlRenderer.h"

//
// IAlmonasterEventSinkUIEventSink
//

int AlmonasterEventSink::OnCreateEmpire(int iEmpireKey)
{
    return HtmlRenderer::OnCreateEmpire(iEmpireKey);
}

int AlmonasterEventSink::OnDeleteEmpire(int iEmpireKey)
{
    return HtmlRenderer::OnDeleteEmpire(iEmpireKey);
}

int AlmonasterEventSink::OnLoginEmpire(int iEmpireKey)
{
    return HtmlRenderer::OnLoginEmpire(iEmpireKey);
}

int AlmonasterEventSink::OnCreateGame(int iGameClass, int iGameNumber)
{
    return HtmlRenderer::OnCreateGame(iGameClass, iGameNumber);
}

int AlmonasterEventSink::OnCleanupGame(int iGameClass, int iGameNumber)
{
    return HtmlRenderer::OnCleanupGame(iGameClass, iGameNumber);
}

int AlmonasterEventSink::OnDeleteTournament(unsigned int iTournamentKey)
{
    return HtmlRenderer::OnDeleteTournament(iTournamentKey);
}

int AlmonasterEventSink::OnDeleteTournamentTeam(unsigned int iTournamentKey, unsigned int iTeamKey)
{
    return HtmlRenderer::OnDeleteTournamentTeam(iTournamentKey, iTeamKey);
}