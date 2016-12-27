
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the TopLists page
int HtmlRenderer::Render_TopLists() {

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

	IHttpForm* pHttpForm;

	int iErrCode;
	unsigned int i;

	int iTopListPage = 0;
	ScoringSystem ssListType = ALMONASTER_SCORE;

	// Handle a submission
	if (m_bOwnPost && !m_bRedirection) {

	    if (!WasButtonPressed (BID_CANCEL)) {

	        if ((pHttpForm = m_pHttpRequest->GetForm ("TopListPage")) == NULL) {
	            goto Redirection;
	        }
	        int iTopListPageSubmit = pHttpForm->GetIntValue();

	        switch (iTopListPageSubmit) {

	        case 0:

	            if (WasButtonPressed (BID_ALMONASTERSCORE)) {
	                iTopListPage = 1;
	                ssListType = ALMONASTER_SCORE;
	            }

	            else if (WasButtonPressed (BID_CLASSICSCORE)) {
	                iTopListPage = 1;
	                ssListType = CLASSIC_SCORE;
	            }

	            else if (WasButtonPressed (BID_BRIDIERSCORE)) {
	                iTopListPage = 1;
	                ssListType = BRIDIER_SCORE;
	            }

	            else if (WasButtonPressed (BID_BRIDIERSCOREESTABLISHED)) {
	                iTopListPage = 1;
	                ssListType = BRIDIER_SCORE_ESTABLISHED;
	            }

	            else {
	                iTopListPage = 0;
	            }

	            break;

	        case 1:
	            {

	            }
	            break;

	        default:
	            Assert (false);
	        }

	    } else {
	        bRedirectTest = false;
	    }
	} 

	SYSTEM_REDIRECT_ON_SUBMIT

	SYSTEM_OPEN (false)

	// Individual page stuff starts here
	switch (iTopListPage) {

	case 0:
	Page0:
	    {
	    
	Write ("<input type=\"hidden\" name=\"TopListPage\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"TopListPage\" value=\"0\">") - 1);
	int iNumEmpires;
	    Check (g_pGameEngine->GetNumEmpiresOnServer (&iNumEmpires));

	    
	Write ("<p>There ", sizeof ("<p>There ") - 1);
	if (iNumEmpires == 1) {
	        
	Write ("is <strong>1</strong> registered empire", sizeof ("is <strong>1</strong> registered empire") - 1);
	} else {
	        
	Write ("are <strong>", sizeof ("are <strong>") - 1);
	Write (iNumEmpires); 
	Write ("</strong> registered empires", sizeof ("</strong> registered empires") - 1);
	}
	    
	Write (" on the server<p>View the top <strong>", sizeof (" on the server<p>View the top <strong>") - 1);
	Write (TOPLIST_SIZE); 
	Write ("</strong> empire", sizeof ("</strong> empire") - 1);
	if (TOPLIST_SIZE != 1) {
	        
	Write ("s", sizeof ("s") - 1);
	}
	    
	Write (" in:<p><table width=\"70%\"><tr><td><font face=\"sans-serif\" size=\"-1\">The <strong>Almonaster Score</strong> is a scoring system designed by Max Attar Feingold. It attempts to give a reasonable approximation of the quality of an empire, as determined by its record on the server. In particular, the Almonaster Score considers the quality and experience of the empires nuked by the empire and the empires who nuked it.</font></td><td>&nbsp;</td><td align=\"center\">", sizeof (" in:<p><table width=\"70%\"><tr><td><font face=\"sans-serif\" size=\"-1\">The <strong>Almonaster Score</strong> is a scoring system designed by Max Attar Feingold. It attempts to give a reasonable approximation of the quality of an empire, as determined by its record on the server. In particular, the Almonaster Score considers the quality and experience of the empires nuked by the empire and the empires who nuked it.</font></td><td>&nbsp;</td><td align=\"center\">") - 1);
	WriteButton (BID_ALMONASTERSCORE); 
	Write ("</td></tr><tr><td colspan=\"3\">&nbsp;</td></tr><tr><td><font face=\"sans-serif\" size=\"-1\">The <strong>Classic Score</strong> is the primary scoring system used by classic Stellar Crisis. It increases when an empire wins or draws a game or nukes another empire, and decreases when an empire is nuked or ruins out of a game.</font></td><td>&nbsp;</td><td align=\"center\">", sizeof ("</td></tr><tr><td colspan=\"3\">&nbsp;</td></tr><tr><td><font face=\"sans-serif\" size=\"-1\">The <strong>Classic Score</strong> is the primary scoring system used by classic Stellar Crisis. It increases when an empire wins or draws a game or nukes another empire, and decreases when an empire is nuked or ruins out of a game.</font></td><td>&nbsp;</td><td align=\"center\">") - 1);
	WriteButton (BID_CLASSICSCORE); 
	Write ("</td></tr><tr><td colspan=\"3\">&nbsp;</td></tr><tr><td><font face=\"sans-serif\" size=\"-1\">The <strong>Bridier Score</strong> is a scoring system designed for chess and adapted for Stellar Crisis 3.x by Jerome Zago. It attempts to perform more or less the same evaluation of an empire's quality that the Almonaster Score does, but only for selected one-on-one games. This particular list considers empires with a Bridier Index smaller than ", sizeof ("</td></tr><tr><td colspan=\"3\">&nbsp;</td></tr><tr><td><font face=\"sans-serif\" size=\"-1\">The <strong>Bridier Score</strong> is a scoring system designed for chess and adapted for Stellar Crisis 3.x by Jerome Zago. It attempts to perform more or less the same evaluation of an empire's quality that the Almonaster Score does, but only for selected one-on-one games. This particular list considers empires with a Bridier Index smaller than ") - 1);
	Write (BRIDIER_TOPLIST_INDEX); 
	Write (".</font></td><td>&nbsp;</td><td align=\"center\">", sizeof (".</font></td><td>&nbsp;</td><td align=\"center\">") - 1);
	WriteButton (BID_BRIDIERSCORE); 
	Write ("</td></tr><tr><td colspan=\"3\">&nbsp;</td></tr><tr><td><font face=\"sans-serif\" size=\"-1\">The <strong>Established Bridier Score</strong> is is the same as the Bridier Score, with one exception: it only considers 'established' empires, which are those with a Bridier Index of ", sizeof ("</td></tr><tr><td colspan=\"3\">&nbsp;</td></tr><tr><td><font face=\"sans-serif\" size=\"-1\">The <strong>Established Bridier Score</strong> is is the same as the Bridier Score, with one exception: it only considers 'established' empires, which are those with a Bridier Index of ") - 1);
	Write (BRIDIER_ESTABLISHED_TOPLIST_INDEX); 
	Write (".</font></td><td>&nbsp;</td><td align=\"center\">", sizeof (".</font></td><td>&nbsp;</td><td align=\"center\">") - 1);
	WriteButton (BID_BRIDIERSCOREESTABLISHED); 
	Write ("</td></tr></table>", sizeof ("</td></tr></table>") - 1);
	}
	    break;

	case 1:
	    {

	    unsigned int iNumEmpires;
	    Variant** ppvData;

	    const char* pszTableColor = m_vTableColor.GetCharPtr();

	    Check (g_pGameEngine->GetTopList (ssListType, &ppvData, &iNumEmpires));

	    if (iNumEmpires == 0) {
	        
	Write ("<p><strong>No empires are currently on the ", sizeof ("<p><strong>No empires are currently on the ") - 1);
	Write (TOPLIST_NAME [ssListType]); 
	Write (" list</strong>", sizeof (" list</strong>") - 1);
	goto Page0;
	    }

	    
	Write ("<input type=\"hidden\" name=\"TopListPage\" value=\"1\"><p><h3>Top <strong>", sizeof ("<input type=\"hidden\" name=\"TopListPage\" value=\"1\"><p><h3>Top <strong>") - 1);
	Write (iNumEmpires); 
	Write ("</strong> empire", sizeof ("</strong> empire") - 1);
	if (iNumEmpires != 1) {
	        
	Write ("s", sizeof ("s") - 1);
	}
	    
	Write (" on the ", sizeof (" on the ") - 1);
	Write (TOPLIST_NAME [ssListType]); 
	Write (" list:</h3><p><table width=\"90%\" cellspacing=\"1\" cellpadding=\"2\"><tr><th align=\"center\" bgcolor=\"", sizeof (" list:</h3><p><table width=\"90%\" cellspacing=\"1\" cellpadding=\"2\"><tr><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Rank</th><th align=\"center\" bgcolor=\"", sizeof ("\">Rank</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Empire</th><th align=\"center\" bgcolor=\"", sizeof ("\">Empire</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Icon</th><th align=\"center\" bgcolor=\"", sizeof ("\">Icon</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Real Name</th><th align=\"center\" bgcolor=\"", sizeof ("\">Real Name</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Wins</th><th align=\"center\" bgcolor=\"", sizeof ("\">Wins</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Nukes</th><th align=\"center\" bgcolor=\"", sizeof ("\">Nukes</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Nuked</th><th align=\"center\" bgcolor=\"", sizeof ("\">Nuked</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Draws</th><th align=\"center\" bgcolor=\"", sizeof ("\">Draws</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Ruins</th><th align=\"center\" bgcolor=\"", sizeof ("\">Ruins</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">", sizeof ("\">") - 1);
	switch (ssListType) {

	    case ALMONASTER_SCORE:
	        
	Write ("Almonaster Score</th><th align=\"center\" bgcolor=\"", sizeof ("Almonaster Score</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Significance</th>", sizeof ("\">Significance</th>") - 1);
	break;

	    case CLASSIC_SCORE:
	        
	Write ("Classic Score", sizeof ("Classic Score") - 1);
	break;

	    case BRIDIER_SCORE:
	    case BRIDIER_SCORE_ESTABLISHED:

	        // Best effort ask for Bridier time bomb scan
	        iErrCode = g_pGameEngine->TriggerBridierTimeBombIfNecessary();
	        Assert (iErrCode == OK);

	        
	Write ("Bridier Rank</th><th align=\"center\" bgcolor=\"", sizeof ("Bridier Rank</th><th align=\"center\" bgcolor=\"") - 1);
	Write (pszTableColor); 
	Write ("\">Bridier Index", sizeof ("\">Bridier Index") - 1);
	break;

	    default:
	        Assert (false);
	        break;
	    }
	    
	Write ("</th></tr>", sizeof ("</th></tr>") - 1);
	Variant* pvEmpData;
	    int iNumActiveGames;
	    String strName, strEmail;

	    char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];

	    for (i = 0; i < iNumEmpires; i ++) {

	        if (g_pGameEngine->GetEmpireData (
	            ppvData[i][TopList::EmpireKey].GetInteger(), 
	            &pvEmpData,
	            &iNumActiveGames
	            ) != OK) {
	            continue;
	        }

	        
	Write ("<tr><td align=\"center\"><strong>", sizeof ("<tr><td align=\"center\"><strong>") - 1);
	Write (i + 1); 
	Write ("</strong></td><td align=\"center\">", sizeof ("</strong></td><td align=\"center\">") - 1);
	Write (pvEmpData[SystemEmpireData::Name].GetCharPtr()); 
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	sprintf (pszProfile, "View the profile of %s", pvEmpData[SystemEmpireData::Name].GetCharPtr());

	        WriteProfileAlienString (
	            pvEmpData[SystemEmpireData::AlienKey].GetInteger(),
	            ppvData[i][TopList::EmpireKey].GetInteger(),
	            pvEmpData[SystemEmpireData::Name].GetCharPtr(),
	            0,
	            "ProfileLink",
	            pszProfile,
	            true,
	            true
	            );

	        
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	iErrCode = HTMLFilter (pvEmpData[SystemEmpireData::RealName].GetCharPtr(), &strName, 0, false);
	        if (iErrCode == OK && !strName.IsBlank()) {

	            iErrCode = HTMLFilter (pvEmpData[SystemEmpireData::Email].GetCharPtr(), &strEmail, 0, false);
	            if (iErrCode == OK) {

	                if (!strEmail.IsBlank()) {
	                    
	Write ("<a href=\"mailto:", sizeof ("<a href=\"mailto:") - 1);
	Write (strEmail.GetCharPtr(), strEmail.GetLength()); 
	Write ("\">", sizeof ("\">") - 1);
	}

	                Write (strName.GetCharPtr(), strName.GetLength());

	                if (!strEmail.IsBlank()) {
	                    
	Write ("</a>", sizeof ("</a>") - 1);
	}
	            }
	        }

	        
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (pvEmpData[SystemEmpireData::Wins].GetInteger());
	        
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (pvEmpData[SystemEmpireData::Nukes].GetInteger());
	        
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (pvEmpData[SystemEmpireData::Nuked].GetInteger());
	        
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (pvEmpData[SystemEmpireData::Draws].GetInteger());
	        
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (pvEmpData[SystemEmpireData::Ruins].GetInteger());

	        
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (ppvData[i][TopList::Data]);

	        switch (ssListType) {

	        case ALMONASTER_SCORE:
	            
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (pvEmpData[SystemEmpireData::AlmonasterScoreSignificance].GetInteger());
	            break;

	        case BRIDIER_SCORE:
	        case BRIDIER_SCORE_ESTABLISHED:
	            
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	Write (ppvData[i][TopList::Data2]);
	            break;
	        }
	        
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	g_pGameEngine->FreeData (pvEmpData);
	    }
	    
	Write ("</table>", sizeof ("</table>") - 1);
	NotifyProfileLink();

	    g_pGameEngine->FreeData (ppvData);

	    }
	    break;

	default:

	    Assert (false);
	}

	SYSTEM_CLOSE


}