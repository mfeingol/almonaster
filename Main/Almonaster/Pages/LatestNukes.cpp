
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the LatestNukes page
int HtmlRenderer::Render_LatestNukes() {

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

	int iErrCode = OK;

	INITIALIZE_EMPIRE

	//if (m_bOwnPost && !m_bRedirection) {
	//}

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)

	int iNumNukes;
	Variant** ppvNukeData;

	if (g_pGameEngine->GetSystemNukeHistory (&iNumNukes, &ppvNukeData) != OK) {

	    
	Write ("<p><strong>The latest nukes could not be read</strong>", sizeof ("<p><strong>The latest nukes could not be read</strong>") - 1);
	}

	else if (iNumNukes == 0) {

	    
	Write ("<p><h3>No nukes have been recorded on this server</h3>", sizeof ("<p><h3>No nukes have been recorded on this server</h3>") - 1);
	}

	else {

	    int i;

	    NotifyProfileLink();

	    char pszDateString [OS::MaxDateLength];
	    char pszEmpire [256 + MAX_EMPIRE_NAME_LENGTH];

	    UTCTime* ptTime = (UTCTime*) StackAlloc (iNumNukes * sizeof (UTCTime));
	    Variant** ppvData = (Variant**) StackAlloc (iNumNukes * sizeof (Variant*));

	    // Sort by timestamp
	    for (i = 0; i < iNumNukes; i ++) {
	        ptTime[i] = ppvNukeData[i][SystemNukeList::TimeStamp].GetUTCTime();
	        ppvData[i] = ppvNukeData[i];
	    }

	    Algorithm::QSortTwoDescending<UTCTime, Variant*> (ptTime, ppvData, iNumNukes);

	    if (iNumNukes != 1) {
	        
	Write ("<p><h3>These are the latest ", sizeof ("<p><h3>These are the latest ") - 1);
	Write (iNumNukes); 
	Write (" nukes recorded on the server:</h3>", sizeof (" nukes recorded on the server:</h3>") - 1);
	} else {
	        
	Write ("<p><h3>This is the latest nuke recorded on the server:</h3>", sizeof ("<p><h3>This is the latest nuke recorded on the server:</h3>") - 1);
	}

	    
	Write ("<p><table width=\"90%\"><tr><th bgcolor=\"", sizeof ("<p><table width=\"90%\"><tr><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Nuker</th><th bgcolor=\"", sizeof ("\" align=\"center\">Nuker</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Icon</th><th bgcolor=\"", sizeof ("\" align=\"center\">Icon</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Nuked</th><th bgcolor=\"", sizeof ("\" align=\"center\">Nuked</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Icon</th><th bgcolor=\"", sizeof ("\" align=\"center\">Icon</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Game</th><th bgcolor=\"", sizeof ("\" align=\"center\">Game</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Time</th></tr>", sizeof ("\" align=\"center\">Time</th></tr>") - 1);
	for (i = 0; i < iNumNukes; i ++) {

	        
	Write ("<tr><td align=\"center\"><strong>", sizeof ("<tr><td align=\"center\"><strong>") - 1);
	Write (ppvData[i][SystemNukeList::NukerEmpireName].GetCharPtr()); 
	Write ("</strong></td><td align=\"center\">", sizeof ("</strong></td><td align=\"center\">") - 1);
	sprintf (
	            pszEmpire, 
	            "View the profile of %s",
	            ppvData[i][SystemNukeList::NukerEmpireName].GetCharPtr()
	            );

	        WriteProfileAlienString (
	            ppvData[i][SystemNukeList::NukerAlienKey].GetInteger(), 
	            ppvData[i][SystemNukeList::NukerEmpireKey].GetInteger(),
	            ppvData[i][SystemNukeList::NukerEmpireName].GetCharPtr(),
	            0,
	            "ProfileLink",
	            pszEmpire,
	            true,
	            true
	            );

	        
	Write ("</td><td align=\"center\"><strong>", sizeof ("</td><td align=\"center\"><strong>") - 1);
	Write (ppvData[i][SystemNukeList::NukedEmpireName].GetCharPtr()); 
	Write ("</strong></td><td align=\"center\">", sizeof ("</strong></td><td align=\"center\">") - 1);
	sprintf (
	            pszEmpire, 
	            "View the profile of %s",
	            ppvData[i][SystemNukeList::NukedEmpireName].GetCharPtr()
	            );

	        WriteProfileAlienString (
	            ppvData[i][SystemNukeList::NukedAlienKey].GetInteger(), 
	            ppvData[i][SystemNukeList::NukedEmpireKey].GetInteger(),
	            ppvData[i][SystemNukeList::NukedEmpireName].GetCharPtr(),
	            0,
	            "ProfileLink",
	            pszEmpire,
	            true,
	            true
	            );

	        
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (ppvData[i][SystemNukeList::GameClassName].GetCharPtr()); 
	Write (" ", sizeof (" ") - 1);
	Write (ppvData[i][SystemNukeList::GameNumber].GetInteger()); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	iErrCode = Time::GetDateString (
	            ppvData[i][SystemNukeList::TimeStamp].GetUTCTime(), 
	            pszDateString
	            );

	        if (iErrCode == OK) {
	            m_pHttpResponse->WriteText (pszDateString);
	        } else {
	            OutputText ("Unknown");
	        }

	        
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	    
	Write ("</table>", sizeof ("</table>") - 1);
	g_pGameEngine->FreeData (ppvNukeData);
	}


	SYSTEM_CLOSE


}