
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the Build page
int HtmlRenderer::Render_Build() {

	// Almonaster
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

	INITIALIZE_EMPIRE

	INITIALIZE_GAME

	IHttpForm* pHttpForm;

	int iErrCode;
	unsigned int i, j;

	Variant vPlanetName;
	String strFilter;

	bool bMapGenerated = (m_iGameState & GAME_MAP_GENERATED) != 0;

	struct NewFleetKey {
	    unsigned int iPlanetKey;
	    unsigned int iFleetKey;
	};

	NewFleetKey nfkNewFleet [NUM_SHIP_TYPES];
	unsigned int iNumNewFleets = 0;

	if (m_bOwnPost && !m_bRedirection) {

	    // Make sure cancel wasn't pressed
	    // Discard submission if update counts don't match
	    if (bMapGenerated && !WasButtonPressed (BID_CANCEL) && m_iNumNewUpdates == m_iNumOldUpdates) {

	        int iNumShips, iTechKey, iShipBR, iLocationKey;
	        unsigned int iFleetKey, iNumShipTypes, iPlanetKey;
	        String strTechName;
	        const char* pszShipName;

	        // Get num techs
	        if ((pHttpForm = m_pHttpRequest->GetForm ("NumTechs")) == NULL) {
	            goto Redirection;
	        }
	        iNumShipTypes = pHttpForm->GetIntValue();
	        if (iNumShipTypes > NUM_SHIP_TYPES) {
	            goto Redirection;
	        }

	        int iNumShipsBuilt;
	        bool bBuildReduced;
	        char pszForm [256];
	        Variant vFleetName;

	        for (i = 0; i < iNumShipTypes; i ++) {

	            sprintf (pszForm, "NumShips%i", i);

	            // Num ships
	            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                goto Redirection;
	            }
	            iNumShips = pHttpForm->GetIntValue();

	            if (iNumShips > 0) {

	                int iX, iY;
	                String strFleetName, strPlanetName;

	                // We're building ships, so get the tech key
	                sprintf (pszForm, "TechKey%i", i);
	                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                    goto Redirection;
	                }
	                iTechKey = pHttpForm->GetIntValue();

	                if (iTechKey < FIRST_SHIP || iTechKey > LAST_SHIP) {
	                    goto Redirection;
	                }

	                // Get the BR
	                sprintf (pszForm, "ShipBR%i", i);
	                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                    goto Redirection;
	                }
	                iShipBR = pHttpForm->GetIntValue();

	                if (iShipBR < 1) {
	                    goto Redirection;
	                }

	                // Get ship name
	                sprintf (pszForm, "ShipName%i", i);
	                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                    goto Redirection;
	                }
	                pszShipName = pHttpForm->GetValue();
	                if (pszShipName == NULL) {
	                    pszShipName = "";
	                }

	                if (!ShipOrFleetNameFilter (pszShipName)) {
	                    AddMessage ("Invalid ship name for ");
	                    AppendMessage (SHIP_TYPE_STRING [iTechKey]);
	                    continue;
	                }

	                if (strlen (pszShipName) > MAX_SHIP_NAME_LENGTH) {
	                    AddMessage ("The ship name for ");
	                    AppendMessage (SHIP_TYPE_STRING [iTechKey]);
	                    AppendMessage (" is too long");
	                    continue;
	                }

	                // Get location key
	                sprintf (pszForm, "ShipLocation%i", i);
	                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                    goto Redirection;
	                }
	                iLocationKey = pHttpForm->GetIntValue();

	                // Get planet key
	                sprintf (pszForm, "LocPlanetKey%i", iLocationKey);
	                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                    goto Redirection;
	                }
	                iPlanetKey = pHttpForm->GetUIntValue();

	                // Get fleet key
	                sprintf (pszForm, "LocFleetKey%i", iLocationKey);
	                if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
	                    goto Redirection;
	                }
	                iFleetKey = pHttpForm->GetUIntValue();

	                if (iFleetKey == FLEET_NEWFLEETKEY) {

	                    for (j = 0; j < iNumNewFleets; j ++) {

	                        if (nfkNewFleet[j].iPlanetKey == iPlanetKey) {
	                            iFleetKey = nfkNewFleet[j].iFleetKey;
	                            break;
	                        }
	                    }

	                    if (iFleetKey == FLEET_NEWFLEETKEY) {

	                        iErrCode = CreateRandomFleet (iPlanetKey, &iFleetKey);
	                        if (iErrCode != OK) {
	                            goto Redirection;
	                        }

	                        Assert (iNumNewFleets < NUM_SHIP_TYPES);
	                        nfkNewFleet [iNumNewFleets].iPlanetKey = iPlanetKey;
	                        nfkNewFleet [iNumNewFleets].iFleetKey = iFleetKey;
	                        iNumNewFleets ++;
	                    }
	                }

	                iErrCode = g_pGameEngine->DoesPlanetExist (
	                    m_iGameClass, 
	                    m_iGameNumber, 
	                    iPlanetKey, 
	                    &bBuildReduced
	                    );

	                if (iErrCode != OK || !bBuildReduced) {
	                    continue;
	                }

	                iErrCode = g_pGameEngine->GetPlanetCoordinates(
	                    m_iGameClass, 
	                    m_iGameNumber, 
	                    iPlanetKey, 
	                    &iX,
	                    &iY
	                    );

	                if (iErrCode != OK) {
	                    continue;
	                }

	                iErrCode = g_pGameEngine->GetPlanetName (
	                    m_iGameClass, 
	                    m_iGameNumber, 
	                    iPlanetKey, 
	                    &vPlanetName
	                    );

	                if (iErrCode != OK) {
	                    continue;
	                }

	                if (HTMLFilter (vPlanetName.GetCharPtr(), &strPlanetName, 0, false) != OK) {
	                    strPlanetName.Clear();
	                }

	                if (iFleetKey == NO_KEY) {

	                    vFleetName = (const char*) NULL;

	                } else {

	                    iErrCode = g_pGameEngine->GetFleetProperty (
	                        m_iGameClass,
	                        m_iGameNumber,
	                        m_iEmpireKey,
	                        iFleetKey,
	                        GameEmpireFleets::Name,
	                        &vFleetName
	                        );

	                    if (iErrCode != OK) {
	                        continue;
	                    }

	                    if (HTMLFilter (vFleetName.GetCharPtr(), &strFleetName, 0, false) != OK) {
	                        strFleetName.Clear();
	                    }
	                }

	                iErrCode = g_pGameEngine->BuildNewShips (
	                    m_iGameClass,
	                    m_iGameNumber,
	                    m_iEmpireKey, 
	                    iTechKey,
	                    iNumShips,
	                    pszShipName,
	                    (float) iShipBR,
	                    iPlanetKey,
	                    iFleetKey,
	                    &iNumShipsBuilt,
	                    &bBuildReduced
	                    );

	                AddBuildNewShipsMessage (
	                    iErrCode,
	                    iNumShipsBuilt,
	                    iShipBR,
	                    iTechKey,
	                    vPlanetName.GetCharPtr(),
	                    iX,
	                    iY,
	                    vFleetName.GetCharPtr(),
	                    bBuildReduced
	                    );
	            }
	        }

	        // Check for a fleet name
	        if ((pHttpForm = m_pHttpRequest->GetForm ("NewFleetName")) == NULL) {
	            goto Redirection;
	        }

	        const char* pszTemp;
	        if ((pszTemp = pHttpForm->GetValue()) != NULL) {

	            if (!ShipOrFleetNameFilter (pszTemp)) {
	                AddMessage ("Blank fleet names are not allowed");
	            } else {

	                if (strlen (pszTemp) > MAX_FLEET_NAME_LENGTH) {
	                    AddMessage ("The new fleet's name is too long");
	                } else {

	                    unsigned int iDontCare;

	                    if ((pHttpForm = m_pHttpRequest->GetForm ("NewFleetLocation")) == NULL) {
	                        goto Redirection;
	                    }
	                    iPlanetKey = pHttpForm->GetIntValue();

	                    iErrCode = g_pGameEngine->CreateNewFleet (
	                        m_iGameClass,
	                        m_iGameNumber,
	                        m_iEmpireKey,
	                        pszTemp,
	                        iPlanetKey,
	                        &iDontCare
	                        );

	                    AddCreateNewFleetMessage (iErrCode, pszTemp);
	                }
	            }
	        }
	    }
	}

	GAME_REDIRECT_ON_SUBMIT

	GAME_OPEN

	int iGameClassOptions;
	GameCheck (g_pGameEngine->GetGameClassOptions (m_iGameClass, &iGameClassOptions));

	//
	// Individual page stuff starts here
	//

	int iBR, * piFleetKey = NULL, iNumFleets = 0;

	BuildLocation* pblBuildLocation = NULL;
	unsigned int iNumLocations = 0, iNumTechs = 0;

	Variant vMaxNumShips, vTemp;
	String* pstrLocationName = NULL;

	char pszLocation [MAX_PLANET_NAME_WITH_COORDINATES_LENGTH + MAX_FLEET_NAME_LENGTH + 64];

	Algorithm::AutoDelete<BuildLocation> autopblBuildLocation (pblBuildLocation, true);
	Algorithm::AutoDelete<String> autostrLocationName (pstrLocationName, true);


	if (!bMapGenerated) {
	    
	Write ("<input type=\"hidden\" name=\"NumTechs\" value=\"0\">", sizeof ("<input type=\"hidden\" name=\"NumTechs\" value=\"0\">") - 1);
	if (iGameClassOptions & GENERATE_MAP_FIRST_UPDATE) {
	        
	Write ("<p>You cannot build ships or fleets before the map is generated", sizeof ("<p>You cannot build ships or fleets before the map is generated") - 1);
	} else {
	        
	Write ("<p>You cannot build ships or fleets before the game starts", sizeof ("<p>You cannot build ships or fleets before the game starts") - 1);
	}

	    goto Close;
	}

	if (m_iGameRatios >= RATIOS_DISPLAY_ON_RELEVANT_SCREENS) {

	    RatioInformation ratInfo;
	    GameCheck (WriteRatiosString (&ratInfo));

	    iBR = ratInfo.iBR;

	} else {

	    GameCheck (g_pGameEngine->GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR));
	}

	if (iBR < 1) {
	    
	Write ("<p>You do not have the minimum tech level required to build ships<input type=\"hidden\" name=\"NumTechs\" value=\"0\">", sizeof ("<p>You do not have the minimum tech level required to build ships<input type=\"hidden\" name=\"NumTechs\" value=\"0\">") - 1);
	goto BuildFleet;
	}

	// Check ship limits
	GameCheck (g_pGameEngine->GetGameClassProperty (
	    m_iGameClass,
	    SystemGameClassData::MaxNumShips,
	    &vMaxNumShips
	    ));

	if (vMaxNumShips.GetInteger() != INFINITE_SHIPS) {

	    int iNumShips;
	    GameCheck (g_pGameEngine->GetNumShips (
	        m_iGameClass,
	        m_iGameNumber,
	        m_iEmpireKey,
	        &iNumShips
	        ));

	    if (iNumShips >= vMaxNumShips.GetInteger()) {
	        
	Write ("<p>You have reached the limit of ships that can be built in this game<input type=\"hidden\" name=\"NumTechs\" value=\"0\">", sizeof ("<p>You have reached the limit of ships that can be built in this game<input type=\"hidden\" name=\"NumTechs\" value=\"0\">") - 1);
	goto BuildFleet;
	    }
	}

	// Get build locations
	GameCheck (g_pGameEngine->GetBuildLocations (
	    m_iGameClass,
	    m_iGameNumber,
	    m_iEmpireKey,
	    NO_KEY,
	    &pblBuildLocation,
	    &iNumLocations
	    ));

	if (iNumLocations == 0) {
	    
	Write ("<p>You have no planets capable of building ships<input type=\"hidden\" name=\"NumTechs\" value=\"0\">", sizeof ("<p>You have no planets capable of building ships<input type=\"hidden\" name=\"NumTechs\" value=\"0\">") - 1);
	goto BuildFleet;
	}


	Write ("<input type=\"hidden\" name=\"NumLocations\" value=\"", sizeof ("<input type=\"hidden\" name=\"NumLocations\" value=\"") - 1);
	Write (iNumLocations); 
	Write ("\">", sizeof ("\">") - 1);
	// Get default builder planet
	int iValue, iRealPlanet;

	GameCheck (g_pGameEngine->GetEmpireDefaultBuilderPlanet (
	    m_iGameClass,
	    m_iGameNumber,
	    m_iEmpireKey,
	    &iValue,
	    &iRealPlanet
	    ));

	// Build location strings
	pstrLocationName = new String [iNumLocations];
	if (pstrLocationName == NULL) {
	    GameCheck (ERROR_OUT_OF_MEMORY);
	}

	for (i = 0; i < iNumLocations; i ++) {

	    int iX, iY;
	    String strPlanetName, strFleetName;

	    unsigned int iPlanetKey = pblBuildLocation[i].iPlanetKey;
	    unsigned int iFleetKey  = pblBuildLocation[i].iFleetKey;

	    if (g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, iPlanetKey, &vTemp) != OK ||
	        String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false) == NULL ||
	        g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, iPlanetKey, &iX, &iY) != OK
	        ) {
	        continue;
	    }

	    switch (iFleetKey) {
	            
	    case NO_KEY:

	        snprintf (
	            pszLocation,
	            sizeof (pszLocation),
	            "%s (%i,%i)",
	            strPlanetName.GetCharPtr(),
	            iX,
	            iY
	            );
	        break;

	    case FLEET_NEWFLEETKEY:

	        snprintf (
	            pszLocation,
	            sizeof (pszLocation),
	            "%s (%i,%i) in new fleet",
	            strPlanetName.GetCharPtr(),
	            iX,
	            iY
	            );
	        break;
	    
	    default:

	        if (g_pGameEngine->GetFleetProperty (
	            m_iGameClass, m_iGameNumber, m_iEmpireKey, iFleetKey, 
	            GameEmpireFleets::Name, &vTemp) != OK ||
	            String::AtoHtml (vTemp.GetCharPtr(), &strFleetName, 0, false) == NULL) {
	            continue;
	        }

	        snprintf (
	            pszLocation,
	            sizeof (pszLocation),
	            "%s (%i,%i) in fleet %s",
	            strPlanetName.GetCharPtr(),
	            iX,
	            iY,
	            strFleetName.GetCharPtr()
	            );
	        break;
	    }

	    pstrLocationName[i] = pszLocation;
	    if (pstrLocationName[i].GetCharPtr() == NULL) {
	        GameCheck (ERROR_OUT_OF_MEMORY);
	    }

	    
	Write ("<input type=\"hidden\" name=\"LocPlanetKey", sizeof ("<input type=\"hidden\" name=\"LocPlanetKey") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (iPlanetKey); 
	Write ("\"><input type=\"hidden\" name=\"LocFleetKey", sizeof ("\"><input type=\"hidden\" name=\"LocFleetKey") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (iFleetKey); 
	Write ("\">", sizeof ("\">") - 1);
	}

	//
	// Get techs we're allowed to build
	//
	int iTechDevs, iTechUndevs;
	unsigned int iMaxNumShipsBuiltAtOnce;

	if (g_pGameEngine->GetDevelopedTechs (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iTechDevs, &iTechUndevs) != OK ||
	    g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::MaxNumShipsBuiltAtOnce, &vTemp) != OK
	    ) {

	    if (iNumFleets > 0) {
	        g_pGameEngine->FreeKeys (piFleetKey);
	    }
	    
	Write ("<p>An error occurred rendering the Build page", sizeof ("<p>An error occurred rendering the Build page") - 1);
	goto Close;
	}

	iMaxNumShipsBuiltAtOnce = vTemp.GetInteger();
	iNumTechs = g_pGameEngine->GetNumTechs (iTechDevs);

	if (iNumTechs > 0) {

	    
	Write ("<input type=\"hidden\" name=\"NumTechs\" value=\"", sizeof ("<input type=\"hidden\" name=\"NumTechs\" value=\"") - 1);
	Write (iNumTechs); 
	Write ("\"><p>", sizeof ("\"><p>") - 1);
	bool bVisible = true;

	    if (g_pGameEngine->GetGameClassVisibleBuilds (m_iGameClass, &bVisible) != OK) {
	        if (iNumFleets > 0) {
	            g_pGameEngine->FreeKeys (piFleetKey);
	        }
	        
	Write ("<p>An error occurred rendering the Build page", sizeof ("<p>An error occurred rendering the Build page") - 1);
	goto Close;
	    }

	    if (bVisible) {
	        
	Write ("Builds are <strong>visible</strong>", sizeof ("Builds are <strong>visible</strong>") - 1);
	} else {
	        
	Write ("Builds are <strong>invisible</strong>", sizeof ("Builds are <strong>invisible</strong>") - 1);
	}

	    if (vMaxNumShips.GetInteger() != INFINITE_SHIPS) {

	        
	Write (". The limit is <strong>", sizeof (". The limit is <strong>") - 1);
	Write (vMaxNumShips.GetInteger());
	        
	Write ("</strong> ship", sizeof ("</strong> ship") - 1);
	if (vMaxNumShips.GetInteger() != 1) {
	            
	Write ("s", sizeof ("s") - 1);
	}
	        
	Write (".", sizeof (".") - 1);
	}

	    //
	    // Render table header
	    //

	    
	Write ("<p><table width=\"70%\"><tr><th bgcolor=\"", sizeof ("<p><table width=\"70%\"><tr><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"left\">Type</th><th bgcolor=\"", sizeof ("\" align=\"left\">Type</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\">Number</th><th bgcolor=\"", sizeof ("\">Number</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\">Name</th><th bgcolor=\"", sizeof ("\">Name</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\">BR</th><th bgcolor=\"", sizeof ("\">BR</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\">Location:</th></tr>", sizeof ("\">Location:</th></tr>") - 1);
	Variant vDefaultShipName;
	    unsigned int iNumBuildableTechs = 0, iMin = min (iMaxNumShipsBuiltAtOnce + 1, 16);

	    ENUMERATE_TECHS (i) {

	        if (iTechDevs & TECH_BITS[i]) {

	            if (g_pGameEngine->GetDefaultEmpireShipName (
	                m_iEmpireKey, 
	                i, 
	                &vDefaultShipName
	                ) != OK ||

	                HTMLFilter (
	                    vDefaultShipName.GetCharPtr(), 
	                    &strFilter,
	                    0,
	                    false
	                    ) != OK) {
	                
	                continue;
	            }
	            
	Write ("<input type=\"hidden\" name=\"TechKey", sizeof ("<input type=\"hidden\" name=\"TechKey") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (i); 
	Write ("\"><tr><td>", sizeof ("\"><tr><td>") - 1);
	Write (SHIP_TYPE_STRING[i]);
	            
	Write ("</td><td align=\"center\"><select name=\"NumShips", sizeof ("</td><td align=\"center\"><select name=\"NumShips") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" size=\"1\">", sizeof ("\" size=\"1\">") - 1);
	for (j = 0; j < iMin; j ++) {
	                
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

	            if (iMaxNumShipsBuiltAtOnce > 15) {
	                for (j = 20; j <= iMaxNumShipsBuiltAtOnce; j += 10) {
	                    
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	            }
	            
	Write ("</select></td><td align=\"center\"><input type=\"text\" size=\"20\" maxlength=\"20\" name=\"ShipName", sizeof ("</select></td><td align=\"center\"><input type=\"text\" size=\"20\" maxlength=\"20\" name=\"ShipName") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (strFilter.GetCharPtr(), strFilter.GetLength()); 
	Write ("\"></td><td align=\"center\">", sizeof ("\"></td><td align=\"center\">") - 1);
	if (iBR == 1) {
	                
	                Write (iBR);
	                
	Write ("<input type=\"hidden\" name=\"ShipBR", sizeof ("<input type=\"hidden\" name=\"ShipBR") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" value=\"1\">", sizeof ("\" value=\"1\">") - 1);
	} else {

	                
	Write ("<select name=\"ShipBR", sizeof ("<select name=\"ShipBR") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" size=\"1\">", sizeof ("\" size=\"1\">") - 1);
	if (iBR < 100) {

	                    for (j = 1; j < (unsigned int) iBR; j ++) {
	                        
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}
	                    
	Write ("<option selected>", sizeof ("<option selected>") - 1);
	Write (iBR); 
	Write ("</select>", sizeof ("</select>") - 1);
	} else {

	                    for (j = 1; j < 10; j ++) {
	                        
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

	                    int iStep = iBR / 100;

	                    for (j = 10; j < (unsigned int) iBR - 10; j += iStep) {
	                        
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

	                    for (j = iBR - 10; j < (unsigned int) iBR; j ++) {
	                        
	Write ("<option>", sizeof ("<option>") - 1);
	Write (j); 
	Write ("</option>", sizeof ("</option>") - 1);
	}

	                    
	Write ("<option selected>", sizeof ("<option selected>") - 1);
	Write (iBR); 
	Write ("</select>", sizeof ("</select>") - 1);
	}
	            }
	            
	Write ("</td><td align=\"center\">", sizeof ("</td><td align=\"center\">") - 1);
	//
	            // Hack to provide one option with no drop-down box for immobile ships
	            // when there's one planet to build on, but multiple fleets on that planet
	            //
	            unsigned int iOneLocIndex = 0, iRealLocations = iNumLocations;

	            if (iNumLocations > 1 && !g_pGameEngine->IsMobileShip (i)) {

	                iRealLocations = 1;

	                unsigned iNonFleetLocations = 0;
	                for (j = 0; j < iNumLocations; j ++) {

	                    if (pblBuildLocation[j].iFleetKey == NO_KEY) {

	                        iOneLocIndex = j;
	                        if (++ iNonFleetLocations == 2) {
	                            iRealLocations = iNumLocations;
	                            break;
	                        }
	                    }
	                }
	            }

	            if (iRealLocations == 1) {
	            
	                Write (pstrLocationName[iOneLocIndex].GetCharPtr(), pstrLocationName[iOneLocIndex].GetLength());
	                
	Write ("<input type=\"hidden\" name=\"ShipLocation", sizeof ("<input type=\"hidden\" name=\"ShipLocation") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (iOneLocIndex); 
	Write ("\">", sizeof ("\">") - 1);
	} else {

	                
	Write ("<select name=\"ShipLocation", sizeof ("<select name=\"ShipLocation") - 1);
	Write (iNumBuildableTechs); 
	Write ("\" size=\"1\">", sizeof ("\" size=\"1\">") - 1);
	for (j = 0; j < iNumLocations; j ++) {

	                    if (pblBuildLocation[j].iFleetKey == NO_KEY || (g_pGameEngine->IsMobileShip (i))) {
	                        
	                        
	Write ("<option", sizeof ("<option") - 1);
	if (pblBuildLocation[j].iPlanetKey == (unsigned int) iRealPlanet &&
	                            pblBuildLocation[j].iFleetKey == NO_KEY) {
	                            
	Write (" selected", sizeof (" selected") - 1);
	}
	                        
	Write (" value=\"", sizeof (" value=\"") - 1);
	Write (j); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pstrLocationName[j].GetCharPtr(), pstrLocationName[j].GetLength());
	                        
	Write ("</option>", sizeof ("</option>") - 1);
	}
	                }
	                
	Write ("</select>", sizeof ("</select>") - 1);
	}
	            
	Write ("</td></tr>", sizeof ("</td></tr>") - 1);
	iNumBuildableTechs ++;
	        }
	    } 

	    Assert (iNumBuildableTechs == iNumTechs);

	    
	Write ("</table><p>", sizeof ("</table><p>") - 1);
	}

	if (piFleetKey != NULL) {
	    g_pGameEngine->FreeKeys (piFleetKey);
	    piFleetKey = NULL;
	}

	BuildFleet:

	// Get fleet locations
	int* piPlanetKey, iNumPlanets;
	GameCheck (g_pGameEngine->GetNewFleetLocations (m_iGameClass, m_iGameNumber, m_iEmpireKey, &piPlanetKey, 
	    &iNumPlanets));

	if (iNumPlanets > 0) {

	    String strPlanetName;
	    Algorithm::AutoDelete<int> autopiPlanetKey (piPlanetKey, true);

	    
	Write ("<p><center>Create a new fleet:<p><table><tr><th bgcolor=\"", sizeof ("<p><center>Create a new fleet:<p><table><tr><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\">Name</th><th bgcolor=\"", sizeof ("\">Name</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\">Location</th></tr><tr><td><input type=\"text\" size=\"20\" maxlength=\"", sizeof ("\">Location</th></tr><tr><td><input type=\"text\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_FLEET_NAME_LENGTH); 
	Write ("\" name=\"NewFleetName\"></td><td>", sizeof ("\" name=\"NewFleetName\"></td><td>") - 1);
	int iX, iY;

	    if (iNumPlanets == 1) {

	        GameCheck (g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, piPlanetKey[0], &vTemp));
	        GameCheck (g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piPlanetKey[0], &iX, &iY));

	        if (String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false) == NULL) {
	            GameCheck (ERROR_OUT_OF_MEMORY);
	        }

	        Write (strPlanetName.GetCharPtr(), strPlanetName.GetLength());
	        
	Write (" (", sizeof (" (") - 1);
	Write (iX); 
	Write (",", sizeof (",") - 1);
	Write (iY); 
	Write (")<input type=\"hidden\" name=\"NewFleetLocation\" value=\"", sizeof (")<input type=\"hidden\" name=\"NewFleetLocation\" value=\"") - 1);
	Write (piPlanetKey[0]); 
	Write ("\">", sizeof ("\">") - 1);
	} else {

	        
	Write ("<select name=\"NewFleetLocation\">", sizeof ("<select name=\"NewFleetLocation\">") - 1);
	for (i = 0; i < (unsigned int) iNumPlanets; i ++) {

	            GameCheck (g_pGameEngine->GetPlanetName (m_iGameClass, m_iGameNumber, piPlanetKey[i], &vTemp));
	            GameCheck (g_pGameEngine->GetPlanetCoordinates (m_iGameClass, m_iGameNumber, piPlanetKey[i], &iX, &iY));
	            
	            if (String::AtoHtml (vTemp.GetCharPtr(), &strPlanetName, 0, false) == NULL) {
	                GameCheck (ERROR_OUT_OF_MEMORY);
	            }

	            
	Write ("<option value=\"", sizeof ("<option value=\"") - 1);
	Write (piPlanetKey[i]); 
	Write ("\">", sizeof ("\">") - 1);
	Write (strPlanetName.GetCharPtr(), strPlanetName.GetLength());
	            
	Write (" (", sizeof (" (") - 1);
	Write (iX); 
	Write (",", sizeof (",") - 1);
	Write (iY); 
	Write (")</option>", sizeof (")</option>") - 1);
	}
	        
	Write ("</select>", sizeof ("</select>") - 1);
	}
	    
	Write ("</td></tr></table><p>", sizeof ("</td></tr></table><p>") - 1);
	WriteButton (BID_CANCEL);
	}

	Close:

	GAME_CLOSE


}