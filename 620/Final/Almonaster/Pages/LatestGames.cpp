
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the LatestGames page
int HtmlRenderer::Render_LatestGames() {

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

	int iErrCode = OK;

	INITIALIZE_EMPIRE

	//if (m_bOwnPost && !m_bRedirection) {
	//}

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)

	int iNumGames;
	Variant** ppvGameData = NULL;

	if (g_pGameEngine->GetSystemLatestGames (&iNumGames, &ppvGameData) != OK) {

	    
	Write ("<p><strong>The latest games could not be read</strong>", sizeof ("<p><strong>The latest games could not be read</strong>") - 1);
	}

	else if (iNumGames == 0) {

	    
	Write ("<p><h3>No games have been recorded on this server</h3>", sizeof ("<p><h3>No games have been recorded on this server</h3>") - 1);
	}

	else {

	    int i;

	    UTCTime* ptTime = (UTCTime*) StackAlloc (iNumGames * sizeof (UTCTime));
	    Variant** ppvData = (Variant**) StackAlloc (iNumGames * sizeof (Variant*));

	    // Sort by timestamp of end
	    for (i = 0; i < iNumGames; i ++) {
	        ptTime[i] = ppvGameData[i][SystemLatestGames::Ended].GetUTCTime();
	        ppvData[i] = ppvGameData[i];
	    }

	    Algorithm::QSortTwoDescending<UTCTime, Variant*> (ptTime, ppvData, iNumGames);

	    // Start off page
	    if (iNumGames != 1) {
	        
	Write ("<p><h3>These are the latest ", sizeof ("<p><h3>These are the latest ") - 1);
	Write (iNumGames); 
	Write (" games recorded on the server:</h3>", sizeof (" games recorded on the server:</h3>") - 1);
	} else {
	        
	Write ("<p><h3>This is the latest game recorded on the server:</h3>", sizeof ("<p><h3>This is the latest game recorded on the server:</h3>") - 1);
	}

	    
	Write ("<p><table width=\"90%\"><tr><th bgcolor=\"", sizeof ("<p><table width=\"90%\"><tr><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Game</th><th bgcolor=\"", sizeof ("\" align=\"center\">Game</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Created</th><th bgcolor=\"", sizeof ("\" align=\"center\">Created</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Ended</th><th bgcolor=\"", sizeof ("\" align=\"center\">Ended</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Updates</th><th bgcolor=\"", sizeof ("\" align=\"center\">Updates</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Result</th><th bgcolor=\"", sizeof ("\" align=\"center\">Result</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Survivors</th><th bgcolor=\"", sizeof ("\" align=\"center\">Survivors</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Losers</th></tr>", sizeof ("\" align=\"center\">Losers</th></tr>") - 1);
	for (i = 0; i < iNumGames; i ++) {

	        const char* pszList;

	        int iSec, iMin, Hour, iDay, iMonth, iYear;
	        DayOfWeek dayOfWeek;
	        char pszDate [64];

	        
	Write ("<tr>", sizeof ("<tr>") - 1);
	// Name
	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	Write (ppvData[i][SystemLatestGames::Name].GetCharPtr()); 
	Write (" ", sizeof (" ") - 1);
	Write (ppvData[i][SystemLatestGames::Number].GetInteger());
	        
	Write ("</td>", sizeof ("</td>") - 1);
	// Created
	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	Time::GetDate (
	            ppvData[i][SystemLatestGames::Created].GetUTCTime(),
	            &iSec, &iMin, &Hour, &dayOfWeek, &iDay, &iMonth, &iYear
	            );

	        sprintf (pszDate, "%s, %i %s %i", 
	            Time::GetAbbreviatedDayOfWeekName (dayOfWeek), iDay, Time::GetAbbreviatedMonthName (iMonth), iYear);

	        m_pHttpResponse->WriteText (pszDate);

	        
	Write ("</td>", sizeof ("</td>") - 1);
	// Ended
	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	Time::GetDate (
	            ppvData[i][SystemLatestGames::Ended].GetUTCTime(),
	            &iSec, &iMin, &Hour, &dayOfWeek, &iDay, &iMonth, &iYear
	            );

	        sprintf (pszDate, "%s, %i %s %i", 
	            Time::GetAbbreviatedDayOfWeekName (dayOfWeek), iDay, Time::GetAbbreviatedMonthName (iMonth), iYear);

	        m_pHttpResponse->WriteText (pszDate);

	        
	Write ("</td>", sizeof ("</td>") - 1);
	// Updates
	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	Write (ppvData[i][SystemLatestGames::Updates].GetInteger());
	        
	Write ("</td>", sizeof ("</td>") - 1);
	// Result
	        
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	switch (ppvData[i][SystemLatestGames::Result].GetInteger()) {

	        case GAME_RESULT_RUIN:
	            
	Write ("Ruin", sizeof ("Ruin") - 1);
	break;

	        case GAME_RESULT_WIN:
	            
	Write ("Win", sizeof ("Win") - 1);
	break;

	        case GAME_RESULT_DRAW:
	            
	Write ("Draw", sizeof ("Draw") - 1);
	break;

	        default:
	            
	Write ("None", sizeof ("None") - 1);
	break;
	        }
	        
	Write ("</td>", sizeof ("</td>") - 1);
	// Survivors
	        
	Write ("<td align=\"center\" width=\"20%\">", sizeof ("<td align=\"center\" width=\"20%\">") - 1);
	pszList = ppvData[i][SystemLatestGames::Winners].GetCharPtr();

	        if (!String::IsBlank (pszList)) {
	            Write (pszList);
	        }
	        
	Write ("</td>", sizeof ("</td>") - 1);
	// Losers
	        
	Write ("<td align=\"center\" width=\"20%\">", sizeof ("<td align=\"center\" width=\"20%\">") - 1);
	pszList = ppvData[i][SystemLatestGames::Losers].GetCharPtr();

	        if (!String::IsBlank (pszList)) {
	            Write (pszList);
	        }
	        
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	}

	    
	Write ("</table>", sizeof ("</table>") - 1);
	g_pGameEngine->FreeData (ppvGameData);
	}


	SYSTEM_CLOSE


}