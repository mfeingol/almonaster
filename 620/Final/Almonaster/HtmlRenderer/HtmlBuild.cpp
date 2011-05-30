//
// Almonaster.dll:  a component of Almonaster 2.0
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

#include "HtmlRenderer.h"

static const char* const g_pszColumns[] = {
    "Type",
    "Number",
    "BR",
    "Fleet",
};

void HtmlRenderer::RenderMiniBuild (unsigned int iPlanetKey, bool bSingleBar) {

    int iErrCode;
    Variant vTemp;

    unsigned int i, iNumLocations, iMaxNumShipsBuiltAtOnce;
    BuildLocation* pblBuildLocation = NULL;
    Algorithm::AutoDelete<BuildLocation> autopblBuildLocation (pblBuildLocation, true);

    int iTechDevs, iTechUndevs, iBR, iNumTechs, iOneTech = 0;
    bool bBuilder;

    // Make sure planet is a builder
    iErrCode = g_pGameEngine->IsPlanetBuilder (m_iGameClass, m_iGameNumber, m_iEmpireKey, iPlanetKey, &bBuilder);
    if (iErrCode != OK || !bBuilder) {
        goto Cleanup;
    }

    // Check BR
    iErrCode = g_pGameEngine->GetEmpireBR (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iBR);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (iBR < 1) {
        goto Cleanup;
    }

    // Check ship limits
    iErrCode = g_pGameEngine->GetGameClassProperty (m_iGameClass, SystemGameClassData::MaxNumShips, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (vTemp.GetInteger() != INFINITE_SHIPS) {

        int iNumShips;
        iErrCode = g_pGameEngine->GetNumShips (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iNumShips);
        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (iNumShips >= vTemp.GetInteger()) {
            goto Cleanup;
        }
    }

    // Get empire data
    iErrCode = g_pGameEngine->GetDevelopedTechs (m_iGameClass, m_iGameNumber, m_iEmpireKey, &iTechDevs, &iTechUndevs);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetEmpireProperty (m_iEmpireKey, SystemEmpireData::MaxNumShipsBuiltAtOnce, &vTemp);
    if (iErrCode != OK) {
        goto Cleanup;
    }
    iMaxNumShipsBuiltAtOnce = vTemp.GetInteger();

    iNumTechs = 0;
    ENUMERATE_SHIP_TYPES (i) {
        if (iTechDevs & TECH_BITS[i]) {
            iNumTechs ++;
            iOneTech = i;
        }
    }

    if (iNumTechs == 0) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetBuildLocations (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        iPlanetKey,
        &pblBuildLocation,
        &iNumLocations
        );

    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Row
    unsigned int iLimit = sizeof (g_pszColumns) / sizeof (g_pszColumns[0]);
    if (iNumLocations == 1) {
        iLimit --;
    }

    OutputText (

        "<tr><td>&nbsp;</td></tr>"\
        "<tr><td></td><td align=\"center\" colspan=\"10\">"\

        "<table width=\"50%\">"\
        "<tr>"
        "<td align=\"center\" colspan=\""
        );

    m_pHttpResponse->WriteText (iLimit + 1);

    OutputText (

        "\">Build:"\
        "</td>"\
        "</tr>"\

        "<tr>"
        );

    const char* pszTableColor = m_vTableColor.GetCharPtr();
    size_t stTableColorLen = strlen (pszTableColor);

    for (i = 0; i < iLimit; i ++) {
        OutputText ("<th bgcolor=\"#");
        m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
        OutputText ("\">");
        m_pHttpResponse->WriteText (g_pszColumns[i]);
        OutputText ("</th>");
    }
    OutputText ("<th bgcolor=\"#");
    m_pHttpResponse->WriteText (pszTableColor, stTableColorLen);
    OutputText (
        "\">Build</th>"\
        "</tr>"\
        "<tr>"
        );

    // Type
    if (iNumTechs == 1) {

        OutputText ("<td align=\"center\"><input type=\"hidden\" name=\"MiniType");
        m_pHttpResponse->WriteText (iPlanetKey);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (iOneTech);
        OutputText ("\">");
        m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
        OutputText ("</td>");

    } else {

        OutputText ("<td align=\"center\"><select name=\"MiniType");
        m_pHttpResponse->WriteText (iPlanetKey);
        OutputText ("\">");

        ENUMERATE_SHIP_TYPES (i) {

            if (iTechDevs & TECH_BITS[i]) {

                OutputText ("<option value=\"");
                m_pHttpResponse->WriteText (i);
                OutputText ("\">");
                m_pHttpResponse->WriteText (SHIP_TYPE_STRING[i]);
                OutputText ("</option>");
            }
        }

        OutputText ("</select></td>");
    }

    // Number
    OutputText ("<td align=\"center\"><select name=\"MiniNumber");
    m_pHttpResponse->WriteText (iPlanetKey);
    OutputText ("\">");

    unsigned int iMin = min (iMaxNumShipsBuiltAtOnce + 1, 16);
    for (i = 0; i < iMin; i ++) {
        OutputText ("<option>");
        m_pHttpResponse->WriteText (i);
        OutputText ("</option>");
    }

    if (iMaxNumShipsBuiltAtOnce > 15) {
        for (i = 20; i <= iMaxNumShipsBuiltAtOnce; i += 10) {
            OutputText ("<option>");
            m_pHttpResponse->WriteText (i);
            OutputText ("</option>");
        }
    }

    OutputText ("</select></td>");

    // BR
    if (iBR == 1) {

        OutputText ("<td align=\"center\"><input type=\"hidden\" name=\"MiniBR");
        m_pHttpResponse->WriteText (iPlanetKey);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (iBR);
        OutputText ("\">");
        m_pHttpResponse->WriteText (iBR);
        OutputText ("</td>");

    } else {

        OutputText ("<td align=\"center\"><select name=\"MiniBR");
        m_pHttpResponse->WriteText (iPlanetKey);
        OutputText ("\">");

        if (iBR < 100) {

            for (i = 1; i < (unsigned int) iBR; i ++) {
                OutputText ("<option>");
                m_pHttpResponse->WriteText (i);
                OutputText ("</option>");
            }

        } else {

            for (i = 1; i < 10; i ++) {
                OutputText ("<option>");
                m_pHttpResponse->WriteText (i);
                OutputText ("</option>");
            }

            int iStep = iBR / 100;

            for (i = 10; i < (unsigned int) iBR - 10; i += iStep) {
                OutputText ("<option>");
                m_pHttpResponse->WriteText (i);
                OutputText ("</option>");
            }

            for (i = iBR - 10; i < (unsigned int) iBR; i ++) {
                OutputText ("<option>");
                m_pHttpResponse->WriteText (i);
                OutputText ("</option>");
            }
        }

        OutputText ("<option selected>");
        m_pHttpResponse->WriteText (iBR);
        OutputText ("</option></select></td>");
    }

    // Fleet
    if (iNumLocations > 1) {

        OutputText ("<td align=\"center\"><select name=\"MiniFleet");
        m_pHttpResponse->WriteText (iPlanetKey);
        OutputText ("\">");

        for (i = 0; i < iNumLocations; i ++) {

            int iNumShips;
            String strFleetName;

            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (pblBuildLocation[i].iFleetKey);
            OutputText ("\">");

            switch (pblBuildLocation[i].iFleetKey) {
                
            case NO_KEY:
                
                OutputText ("No fleet")
                break;

            case FLEET_NEWFLEETKEY:

                OutputText ("New fleet");
                break;

            default:

                Variant vFleetName;
                iErrCode = g_pGameEngine->GetFleetName (
                    m_iGameClass,
                    m_iGameNumber,
                    m_iEmpireKey,
                    pblBuildLocation[i].iFleetKey,
                    &vFleetName
                    );

                if (iErrCode != OK) {
                    goto Cleanup;
                }

                if (HTMLFilter (vFleetName.GetCharPtr(), &strFleetName, 0, false) != OK) {
                    strFleetName.Clear();
                }

                m_pHttpResponse->WriteText (strFleetName.GetCharPtr(), strFleetName.GetLength());

                iErrCode = g_pGameEngine->GetNumShipsInFleet (
                    m_iGameClass,
                    m_iGameNumber,
                    m_iEmpireKey,
                    pblBuildLocation[i].iFleetKey,
                    &iNumShips
                    );

                if (iErrCode == OK) {
                   
                    OutputText (" (");
                    m_pHttpResponse->WriteText (iNumShips);
                    OutputText (" ship");
                    if (iNumShips != 1) {
                        OutputText ("s");
                    }
                    OutputText (")");
                }

                break;
            }

            OutputText ("</option>");
        }

        OutputText ("</select></td>");
    }

    // Button
    OutputText ("<td align=\"center\">");
    if (bSingleBar) {
        WriteButton (BID_MINIBUILD);
    } else {

        char pszButton [64];
        snprintf (pszButton, sizeof (pszButton), "MiniBuild%i", iPlanetKey);
        WriteButtonString (m_iButtonKey, "Build", "Build", pszButton); 
    }

    OutputText (
        "</td>"\
        "</tr>"\
        "</table>"\
        "</td></tr>"
        );

Cleanup:

    if (iErrCode != OK) {
        OutputText ("Error ");
        m_pHttpResponse->WriteText (iErrCode);
        OutputText (" occurred in RenderMiniBuild");
    }        
}


void HtmlRenderer::HandleMiniBuild (unsigned int iPlanetKey) {

    int iErrCode, iType, iNumber, iBR, iNumShipsBuilt, iX, iY;
    unsigned int iFleet = NO_KEY;
    bool bBuildReduced;

    Variant vPlanetName, vFleetName;
    String strPlanetName, strFleetName;

    IHttpForm* pHttpForm;

    char pszForm [64];

    // Get type
    snprintf (pszForm, sizeof (pszForm), "MiniType%d", iPlanetKey);
    pHttpForm = m_pHttpRequest->GetForm (pszForm);
    if (pHttpForm == NULL) {
        goto Cleanup;
    }
    iType = pHttpForm->GetIntValue();

    // Get number of ships
    snprintf (pszForm, sizeof (pszForm), "MiniNumber%d", iPlanetKey);
    pHttpForm = m_pHttpRequest->GetForm (pszForm);
    if (pHttpForm == NULL) {
        goto Cleanup;
    }
    iNumber = pHttpForm->GetIntValue();

    if (iNumber == 0) {
        goto Cleanup;
    }

    // Get BR
    snprintf (pszForm, sizeof (pszForm), "MiniBR%d", iPlanetKey);
    pHttpForm = m_pHttpRequest->GetForm (pszForm);
    if (pHttpForm == NULL) {
        goto Cleanup;
    }
    iBR = pHttpForm->GetIntValue();

    // Get fleet
    snprintf (pszForm, sizeof (pszForm), "MiniFleet%d", iPlanetKey);
    pHttpForm = m_pHttpRequest->GetForm (pszForm);
    if (pHttpForm != NULL) {

        iFleet = pHttpForm->GetUIntValue();
        if (iFleet == FLEET_NEWFLEETKEY) {

            iErrCode = CreateRandomFleet (iPlanetKey, &iFleet);
            if (iErrCode != OK) {
                goto Cleanup;
            }
        }
    }

    // Get names
    iErrCode = g_pGameEngine->GetPlanetCoordinates(
        m_iGameClass, 
        m_iGameNumber, 
        iPlanetKey, 
        &iX,
        &iY
        );

    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = g_pGameEngine->GetPlanetName (
        m_iGameClass, 
        m_iGameNumber, 
        iPlanetKey, 
        &vPlanetName
        );

    if (iErrCode != OK) {
        goto Cleanup;
    }

    if (HTMLFilter (vPlanetName.GetCharPtr(), &strPlanetName, 0, false) != OK) {
        strPlanetName.Clear();
    }

    if (iFleet == NO_KEY) {

        vFleetName = (const char*) NULL;

    } else {

        iErrCode = g_pGameEngine->GetFleetName (
            m_iGameClass, 
            m_iGameNumber,
            m_iEmpireKey,
            iFleet, 
            &vFleetName
            );

        if (iErrCode != OK) {
            goto Cleanup;
        }

        if (HTMLFilter (vFleetName.GetCharPtr(), &strFleetName, 0, false) != OK) {
            strFleetName.Clear();
        }
    }

    iErrCode = g_pGameEngine->BuildNewShips (
        m_iGameClass,
        m_iGameNumber,
        m_iEmpireKey,
        iType,
        iNumber,
        NULL,
        (float) iBR,
        iPlanetKey,
        iFleet,
        &iNumShipsBuilt,
        &bBuildReduced
        );

    AddBuildNewShipsMessage (
        iErrCode,
        iNumShipsBuilt,
        iBR,
        iType,
        strPlanetName.GetCharPtr(),
        iX,
        iY,
        strFleetName.GetCharPtr(),
        bBuildReduced
        );

Cleanup:
;
}


int HtmlRenderer::CreateRandomFleet (unsigned int iPlanetKey, unsigned int* piFleetKey) {

    char pszFleetName [64];
    int iErrCode = ERROR_NAME_IS_IN_USE;

    while (iErrCode == ERROR_NAME_IS_IN_USE) {

        // Generate a random fleet name
        int iRand = Algorithm::GetRandomInteger (0x7fffffff);
        snprintf (pszFleetName, sizeof (pszFleetName), "Fleet%i", iRand);

        iErrCode = g_pGameEngine->CreateNewFleet (
            m_iGameClass,
            m_iGameNumber,
            m_iEmpireKey,
            pszFleetName,
            iPlanetKey,
            piFleetKey
            );
    }

    AddCreateNewFleetMessage (iErrCode, pszFleetName);
    
    return iErrCode;
}


void HtmlRenderer::AddBuildNewShipsMessage (int iErrCode, int iNumShipsBuilt, int iBR, int iTechKey,
                                            const char* pszPlanetName, int iX, int iY, const char* pszFleetName,
                                            bool bBuildReduced) {
    switch (iErrCode) {

    case OK:

        if (bBuildReduced) {
            AddMessage ("Only ");
            AppendMessage (iNumShipsBuilt);
        } else {
            AddMessage (iNumShipsBuilt);
        }

        AppendMessage (" BR");
        AppendMessage (iBR);
        AppendMessage (" ");
        AppendMessage (SHIP_TYPE_STRING[iTechKey]);
        AppendMessage (iNumShipsBuilt == 1 ? " ship was" : " ships were");
        AppendMessage (" built at ");

        if (pszPlanetName != NULL) {
            AppendMessage (pszPlanetName);
        }

        AppendMessage (" (");
        AppendMessage (iX);
        AppendMessage (",");
        AppendMessage (iY);
        AppendMessage (")");

        if (pszFleetName != NULL) {
            AppendMessage (" in fleet ");
            AppendMessage (pszFleetName);
        }
        break;

    case ERROR_GAME_HAS_NOT_STARTED:

        AddMessage ("You cannot build ships before the game starts");
        break;

    case ERROR_WRONG_OWNER:

        // Don't leak the planet's name
        AddMessage ("You cannot build at that planet");
        break;

    case ERROR_INSUFFICIENT_POPULATION:

        AddMessage ("Planet ");
        AppendMessage (pszPlanetName);
        AppendMessage (" lacks the population needed to build ships");
        break;

    case ERROR_WRONG_TECHNOLOGY:

        AddMessage ("You cannot build ");
        AppendMessage (SHIP_TYPE_STRING[iTechKey]);
        AppendMessage (" ships");
        break;

    case ERROR_INSUFFICIENT_POPULATION_FOR_COLONIES:

        AddMessage ("Planet ");
        AppendMessage (pszPlanetName);
        AppendMessage (" lacks the population level needed to build colony ships");
        break;

    case ERROR_SHIP_LIMIT_REACHED:

        AddMessage ("You have reached the limit of ships that can be built in this game");
        break;

    case ERROR_INVALID_TECH_LEVEL:

        AddMessage ("You cannot build BR");
        AppendMessage (iBR);
        AppendMessage (" ships");
        break;

    case ERROR_WRONG_NUMBER_OF_SHIPS:

        AddMessage ("You cannot build that number of ships");
        break;

    case ERROR_FLEET_DOES_NOT_EXIST:

        if (pszFleetName == NULL) {
            AddMessage ("That fleet");
        } else {
            AddMessage ("Fleet ");
            AppendMessage (pszFleetName);
        }
        AppendMessage (" no longer exists");
        break;

    case ERROR_FLEET_NOT_ON_PLANET:

        if (pszFleetName == NULL) {
            AddMessage ("That fleet");
        } else {
            AddMessage ("Fleet ");
            AppendMessage (pszFleetName);
        }
        AppendMessage (" is not located on the specified planet");
        break;

    case ERROR_SHIP_CANNOT_JOIN_FLEET:

        AddMessage ("Immobile ships cannot be built into fleets");
        break;

    default:

        AddMessage ("Error ");
        AppendMessage (iErrCode);
        AppendMessage (" occurred attempting to build ships");
        break;
    }
}

void HtmlRenderer::AddCreateNewFleetMessage (int iErrCode, const char* pszFleetName) {

    String strFilter;

    switch (iErrCode) {
    case OK:
    
        if (HTMLFilter (
            pszFleetName, 
            &strFilter,
            0,
            false
            ) == OK) {

            AddMessage ("Fleet ");
            AppendMessage (strFilter.GetCharPtr());
            AppendMessage (" was created");
        
        } else {
            AddMessage ("The fleet was created");
        }

        break;

    case ERROR_NAME_IS_IN_USE:

        AddMessage ("A fleet with that name already exists");
        break;

    case ERROR_EMPTY_NAME:

        AddMessage ("The fleet name was empty");
        break;

    case ERROR_ORPHANED_FLEET:

        AddMessage ("You cannot create a fleet on that planet");
        break;

    default:

        AddMessage ("Error ");
        AppendMessage (iErrCode);
        AppendMessage (" occurred attempting to create a fleet");
        break;
    }
}