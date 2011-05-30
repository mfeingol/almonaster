//
// Almonaster.dll:  a component of Almonaster
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

#include "HtmlRenderer.h"


void HtmlRenderer::RenderGameConfiguration (int iGameClass, unsigned int iTournamentKey) {
    
    int iHrsUD, iMinUD, iSecUD, iMaxNumEmpires, iBridier, iErrCode, iFilterIP = 0, iFilterId = 0;
    
    Variant vNumUpdatesBeforeGameCloses = 1;

    bool bRestrictAlmonaster = false, bRestrictClassic = false, bRestrictBridierRank = false, 
        bRestrictBridierIndex = false, bRestrictBridierRankGain = false, bRestrictWins = false, 
        bNamesListed, bSpectators, bRestrictBridierRankLoss = false, fFilterIdle = true;
    
    const char* pszRestrictAlmonasterMin = NULL, * pszRestrictAlmonasterMax = NULL, * pszRestrictClassicMin = NULL,
        * pszRestrictClassicMax = NULL, * pszRestrictBridierRankMin = NULL, * pszRestrictBridierRankMax = NULL, 
        * pszRestrictBridierIndexMin = NULL, * pszRestrictBridierIndexMax = NULL, * pszRestrictBridierRankGainMin = NULL, 
        * pszRestrictBridierRankGainMax = NULL, * pszRestrictWinsMin = NULL, * pszRestrictWinsMax = NULL,
        * pszRestrictBridierRankLossMin = NULL, * pszRestrictBridierRankLossMax = NULL;
    
    float fRestrictAlmonasterMin = 0, fRestrictAlmonasterMax = 0, fRestrictClassicMin = 0,
        fRestrictClassicMax = 0;
    
    int iRestrictBridierRankMin = 0, iRestrictBridierRankMax = 0, iRestrictBridierIndexMin = 0, 
        iRestrictBridierIndexMax = 0, iRestrictBridierRankGainMin = 0, iRestrictBridierRankGainMax = 0, 
        iRestrictWinsMin = 0, iRestrictWinsMax = 0, iRestrictBridierRankLossMin = 0, 
        iRestrictBridierRankLossMax = 0;

    Variant vMaxUpdatesBeforeGameCloses;

    IHttpForm* pHttpForm;

    const char* pszMessage, * pszGamePassword = NULL, * pszGamePassword2 = NULL;

    int iSystemOptions, iGameClassOptions;

    char pszFormIP [64];
    char pszFormID [64];

    unsigned int iNumBlocks = 0;

    iErrCode = g_pGameEngine->GetSystemOptions (&iSystemOptions);
    if (iErrCode != OK) {
        goto OnError;
    }

    if (iGameClass != NO_KEY) {

        iErrCode = g_pGameEngine->GetGameClassOptions (iGameClass, &iGameClassOptions);
        if (iErrCode != OK) {
            goto OnError;
        }

    } else {

        iGameClassOptions = 0;
    }
    
    // MaxUpdatesBeforeGameCloses
    iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxNumUpdatesBeforeClose, &vMaxUpdatesBeforeGameCloses);
    if (iErrCode != OK) {
        goto OnError;
    }
    
    // MaxNumEmpires
    if (iGameClass == NO_KEY) {
        iMaxNumEmpires = 2;
    } else {
        iErrCode = g_pGameEngine->GetMaxNumEmpires (iGameClass, &iMaxNumEmpires);
        if (iErrCode != OK) {
            goto OnError;
        }
    }
    
    // Bridier
    if ((pHttpForm = m_pHttpRequest->GetForm ("Bridier")) != NULL) {
        iBridier = pHttpForm->GetIntValue();
    } else {
        iBridier = (iSystemOptions & DEFAULT_BRIDIER_GAMES) ? GAME_COUNT_FOR_BRIDIER : 0;
    }
    
    // HoursUD
    if ((pHttpForm = m_pHttpRequest->GetForm ("HoursUD")) != NULL) {
        iHrsUD = pHttpForm->GetIntValue();
    } else {
        iHrsUD = 0;
    }
    
    // MinsUD
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinsUD")) != NULL) {
        iMinUD = pHttpForm->GetIntValue();
    } else {
        iMinUD = 0;
    }
    
    // SecsUD
    if ((pHttpForm = m_pHttpRequest->GetForm ("SecsUD")) != NULL) {
        iSecUD = pHttpForm->GetIntValue();
    } else {
        iSecUD = 0;
    }

    // NamesListed
    if ((pHttpForm = m_pHttpRequest->GetForm ("NamesListed")) != NULL) {
        bNamesListed = pHttpForm->GetIntValue() != 0;
    } else {
        bNamesListed = (iSystemOptions & DEFAULT_NAMES_LISTED) != 0;
    }

    // Spectators
    if ((pHttpForm = m_pHttpRequest->GetForm ("Spectators")) != NULL) {
        bSpectators = pHttpForm->GetIntValue() != 0;
    } else {
        bSpectators = (iSystemOptions & DEFAULT_ALLOW_SPECTATORS) != 0;
    }

    // EnterGameMessage
    if ((pHttpForm = m_pHttpRequest->GetForm ("EnterGameMessage")) != NULL) {
        pszMessage = pHttpForm->GetValue();
    } else {
        pszMessage = NULL;
    }

    if (iTournamentKey == NO_KEY) {

        // NumUpdatesClose
        if ((pHttpForm = m_pHttpRequest->GetForm ("NumUpdatesForClose")) != NULL) {
            vNumUpdatesBeforeGameCloses = pHttpForm->GetIntValue();
        } else {
            iErrCode = g_pGameEngine->GetSystemProperty (SystemData::DefaultNumUpdatesBeforeClose, &vNumUpdatesBeforeGameCloses);
            if (iErrCode != OK) {
                goto OnError;
            }
        }
        
        // RestrictAlmonaster
        bRestrictAlmonaster = m_pHttpRequest->GetForm ("RestrictAlmonaster") != NULL;
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictAlmonasterMin")) != NULL) {
            
            pszRestrictAlmonasterMin = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictAlmonasterMin, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictAlmonasterMin, LOWEST_STRING) != 0) {
                fRestrictAlmonasterMin = pHttpForm->GetFloatValue();
                pszRestrictAlmonasterMin = NULL;
            }
            
        } else {
            fRestrictAlmonasterMin = ALMONASTER_MIN_SCORE;
        }
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictAlmonasterMax")) != NULL) {
            
            pszRestrictAlmonasterMax = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictAlmonasterMax, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictAlmonasterMax, LOWEST_STRING) != 0) {
                fRestrictAlmonasterMax = pHttpForm->GetFloatValue();
                pszRestrictAlmonasterMax = NULL;
            }
            
        } else {
            pszRestrictAlmonasterMax = HIGHEST_STRING;
        }
        
        // RestrictClassic
        bRestrictClassic = m_pHttpRequest->GetForm ("RestrictClassic") != NULL;
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictClassicMin")) != NULL) {
            
            pszRestrictClassicMin = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictClassicMin, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictClassicMin, LOWEST_STRING) != 0) {
                fRestrictClassicMin = pHttpForm->GetFloatValue();
                pszRestrictClassicMin = NULL;
            }
            
        } else {
            pszRestrictClassicMin = LOWEST_STRING;
        }
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictClassicMax")) != NULL) {
            
            pszRestrictClassicMax = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictClassicMax, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictClassicMax, LOWEST_STRING) != 0) {
                fRestrictClassicMax = pHttpForm->GetFloatValue();
                pszRestrictClassicMax = NULL;
            }
            
        } else {
            pszRestrictClassicMax = HIGHEST_STRING;
        }
        
        // RestrictBridierRank
        bRestrictBridierRank = m_pHttpRequest->GetForm ("RestrictBridierRank") != NULL;
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankMin")) != NULL) {
            
            pszRestrictBridierRankMin = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictBridierRankMin, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictBridierRankMin, LOWEST_STRING) != 0) {
                iRestrictBridierRankMin = pHttpForm->GetIntValue();
                pszRestrictBridierRankMin = NULL;
            }
            
        } else {
            iRestrictBridierRankMin = BRIDIER_MIN_RANK;
        }
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankMax")) != NULL) {
            
            pszRestrictBridierRankMax = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictBridierRankMax, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictBridierRankMax, LOWEST_STRING) != 0) {
                iRestrictBridierRankMax = pHttpForm->GetIntValue();
                pszRestrictBridierRankMax = NULL;
            }
            
        } else {
            pszRestrictBridierRankMax = HIGHEST_STRING;
        }
        
        // RestrictBridierIndex
        bRestrictBridierIndex = m_pHttpRequest->GetForm ("RestrictBridierIndex") != NULL;
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierIndexMin")) != NULL) {
            
            pszRestrictBridierIndexMin = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictBridierIndexMin, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictBridierIndexMin, LOWEST_STRING) != 0) {
                iRestrictBridierIndexMin = pHttpForm->GetIntValue();
                pszRestrictBridierIndexMin = NULL;
            }
            
        } else {
            iRestrictBridierIndexMin = BRIDIER_MIN_INDEX;
        }
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierIndexMax")) != NULL) {
            
            pszRestrictBridierIndexMax = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictBridierIndexMax, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictBridierIndexMax, LOWEST_STRING) != 0) {
                iRestrictBridierIndexMax = pHttpForm->GetIntValue();
                pszRestrictBridierIndexMax = NULL;
            }
            
        } else {
            iRestrictBridierIndexMax = BRIDIER_MAX_INDEX;
        }
        
        // RestrictBridierRankGain
        bRestrictBridierRankGain = m_pHttpRequest->GetForm ("RestrictBridierRankGain") != NULL;
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankGainMin")) != NULL) {
            
            pszRestrictBridierRankGainMin = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictBridierRankGainMin, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictBridierRankGainMin, LOWEST_STRING) != 0) {
                iRestrictBridierRankGainMin = pHttpForm->GetIntValue();
                pszRestrictBridierRankGainMin = NULL;
            }
            
        } else {
            iRestrictBridierRankGainMin = BRIDIER_MIN_RANK_GAIN;
        }
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankGainMax")) != NULL) {
            
            pszRestrictBridierRankGainMax = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictBridierRankGainMax, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictBridierRankGainMax, LOWEST_STRING) != 0) {
                iRestrictBridierRankGainMax = pHttpForm->GetIntValue();
                pszRestrictBridierRankGainMax = NULL;
            }
            
        } else {
            pszRestrictBridierRankGainMax = HIGHEST_STRING;
        }

        // RestrictBridierRankLoss
        bRestrictBridierRankLoss = m_pHttpRequest->GetForm ("RestrictBridierRankLoss") != NULL;
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankLossMin")) != NULL) {
            
            pszRestrictBridierRankLossMin = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictBridierRankLossMin, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictBridierRankLossMin, LOWEST_STRING) != 0) {
                iRestrictBridierRankLossMin = pHttpForm->GetIntValue();
                pszRestrictBridierRankLossMin = NULL;
            }
            
        } else {
            iRestrictBridierRankLossMin = BRIDIER_MIN_RANK_LOSS;
        }
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankLossMax")) != NULL) {
            
            pszRestrictBridierRankLossMax = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictBridierRankLossMax, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictBridierRankLossMax, LOWEST_STRING) != 0) {
                iRestrictBridierRankLossMax = pHttpForm->GetIntValue();
                pszRestrictBridierRankLossMax = NULL;
            }
            
        } else {
            pszRestrictBridierRankLossMax = HIGHEST_STRING;
        }
        
        // RestrictWins
        bRestrictWins = m_pHttpRequest->GetForm ("RestrictWins") != NULL;
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictWinsMin")) != NULL) {
            
            pszRestrictWinsMin = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictWinsMin, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictWinsMin, LOWEST_STRING) != 0) {
                iRestrictWinsMin = pHttpForm->GetIntValue();
                pszRestrictWinsMin = NULL;
            }
            
        } else {
            iRestrictWinsMin = 0;
        }
        
        if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictWinsMax")) != NULL) {
            
            pszRestrictWinsMax = pHttpForm->GetValue();
            
            if (String::StriCmp (pszRestrictWinsMax, HIGHEST_STRING) != 0 &&
                String::StriCmp (pszRestrictWinsMax, LOWEST_STRING) != 0) {
                iRestrictWinsMax = pHttpForm->GetIntValue();
                pszRestrictWinsMax = NULL;
            }
            
        } else {
            pszRestrictWinsMax = HIGHEST_STRING;
        }
        
        // FilterIP
        if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIP")) != NULL) {
            iFilterIP = pHttpForm->GetIntValue();
        } else {
            iFilterIP = GAME_WARN_ON_DUPLICATE_IP_ADDRESS;
        }
        
        // FilterId
        if ((pHttpForm = m_pHttpRequest->GetForm ("FilterId")) != NULL) {
            iFilterId = pHttpForm->GetIntValue();
        } else {
            iFilterId = GAME_WARN_ON_DUPLICATE_SESSION_ID;
        }

        // FilterIdle
        if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIdle")) != NULL) {
            fFilterIdle = pHttpForm->GetIntValue() != 0;
        } else {
            fFilterIdle = (iSystemOptions & DEFAULT_RESTRICT_IDLE_EMPIRES) != 0;
        }
   
        // GamePassword
        if ((pHttpForm = m_pHttpRequest->GetForm ("GamePassword")) != NULL) {
            pszGamePassword = pHttpForm->GetValue();
        } else {
            pszGamePassword = NULL;
        }
        
        // GamePassword2
        if ((pHttpForm = m_pHttpRequest->GetForm ("GamePassword2")) != NULL) {
            pszGamePassword2 = pHttpForm->GetValue();
        } else {
            pszGamePassword2 = NULL;
        }
    }
    
    if (iGameClass != NO_KEY) {
        OutputText ("<input type=\"hidden\" name=\"GameClassKey\" value=\"");
        m_pHttpResponse->WriteText (iGameClass);
        OutputText ("\">");
    }
    
    OutputText ("<table width=\"90%\">");

    if (iTournamentKey == NO_KEY) {
        
        OutputText (
            "<tr>"\
            "<td>Updates before game closes <em>(<strong>1</strong> to <strong>"
            );
        
        m_pHttpResponse->WriteText (vMaxUpdatesBeforeGameCloses.GetInteger());  
        
        OutputText (
            "</strong>)</em>:</td>"\
            "<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"NumUpdatesForClose\" value=\""
            );
        
        m_pHttpResponse->WriteText (vNumUpdatesBeforeGameCloses.GetInteger());
        
        OutputText ("\"></td></tr>");
    }
    
    // First update delay
    OutputText (
        
        "<tr>"\
        "<td>First update delay (<em>at most <strong>"
        );

    m_pHttpResponse->WriteText (MAX_NUM_UPDATE_PERIODS_FOR_FIRST_DELAY);
    
    OutputText (
        "</strong> update periods</em>):</td>"\
        "<td><input type=\"text\" size=\"4\" maxlength=\"20\" name=\"HoursUD\" value=\"");
    
    m_pHttpResponse->WriteText (iHrsUD);
    
    OutputText (
        
        "\"> hours, "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"MinsUD\" value=\"");
    
    m_pHttpResponse->WriteText (iMinUD);
    
    OutputText (
        
        "\"> minutes, "\
        "<input type=\"text\" size=\"4\" maxlength=\"20\" name=\"SecsUD\" value=\"");
    
    m_pHttpResponse->WriteText (iSecUD);
    
    OutputText (
        
        "\"> seconds</td>"\
        "</tr>"
        
        );
    
    // Enter message
    OutputText (
        
        "<tr><td>Message sent to empires entering the game:</td><td>"\
        "<textarea rows=\"3\" cols=\"50\" wrap=\"physical\" name=\"EnterGameMessage\">"
        );
    
    if (pszMessage != NULL) {
        
        String strFilter;
        
        if (HTMLFilter (pszMessage, &strFilter, 0, false) == OK) {
            m_pHttpResponse->WriteText (strFilter.GetCharPtr(), strFilter.GetLength());
        }
    }
    
    OutputText ("</textarea></td></tr>");

    // Bridier
    if (iMaxNumEmpires == 2) {

        OutputText (
            
            "<tr>"\
            "<td>Bridier configuration (<em>only for grudge games</em>):</td><td><select name=\"Bridier\">"\
            "<option"
            );
        
        if (iBridier == GAME_COUNT_FOR_BRIDIER) {
            OutputText (" selected");
        }
        
        OutputText (" value=\"");
        m_pHttpResponse->WriteText (GAME_COUNT_FOR_BRIDIER);
        OutputText ("\">Game counts towards Bridier Scoring</option>"\
            
            "<option"
            );
        
        if (iBridier == 0) {
            OutputText (" selected");
        }
        
        OutputText (
            
            " value=\"0\">Game does not count towards Bridier Scoring</option></select>"\
            "</td>"\
            "</tr>"
            );
    }

    if (iTournamentKey == NO_KEY) {

        OutputText (

            "<tr>"\
            "<td>Empire names exposed:</td>"\
            "<td><select name=\"NamesListed\">"\
            "<option"
            );

        if (bNamesListed) {
            OutputText (" selected");
        }

        OutputText (
            " value=\"1\">Empire names exposed on game lists and broadcast on game entry</option>"\
            "<option"
            );

        if (!bNamesListed) {
            OutputText (" selected");
        }
            
        OutputText (
            " value=\"0\">Empire names not exposed</option>"\
            "</select></td>"\
            "</tr>"
            );
    }

    if (iGameClass == NO_KEY || (iGameClassOptions & EXPOSED_SPECTATORS) == EXPOSED_SPECTATORS) {

        OutputText (
            "<tr>"\
            "<td>Game available to spectators:"
            );

        if (iGameClass == NO_KEY) {
            OutputText ("(<em>requires exposed maps and exposed diplomacy</em>)");
        }
            
        OutputText (
            "</td>"\
            "<td><select name=\"Spectators\">"\
            "<option"
            );

        if (bSpectators) {
            OutputText (" selected");
        }

        OutputText (
            " value=\"1\">Game is available from the Spectator Games page</option>"\
            "<option"
            );

        if (!bSpectators) {
            OutputText (" selected");
        }
            
        OutputText (
            " value=\"0\">Game is not available from the Spectator Games page</option>"\
            "</select></td>"\
            "</tr>"
            );
    }

    if (iTournamentKey == NO_KEY) {

        OutputText (
            "<tr><td>Password protection: </td>"\
            "<td><table><tr><td>Password:</td><td><input type=\"password\" name=\"GamePassword\" size=\""
            );
        
        m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
        
        OutputText ("\" maxlength=\"");
        m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
        
        if (pszGamePassword != NULL) {
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (pszGamePassword);
        }
        
        OutputText ("\"></td></tr><tr><td>Confirm:</td><td><input type=\"password\" name=\"GamePassword2\" size=\"");
        m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
        OutputText ("\" maxlength=\"");
        
        m_pHttpResponse->WriteText (MAX_PASSWORD_LENGTH);
        
        if (pszGamePassword2 != NULL) {
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (pszGamePassword2);
        }
        
        OutputText (
            
            "\"></td></tr></table></td></tr>"\
            
            "<tr>"\
            "<td>Empire filtering by Almonaster Score:</td>"\
            "<td><input type=\"checkbox\""
            );
        
        if (bRestrictAlmonaster) {
            OutputText (" checked");
        }
        
        OutputText (
            " name=\"RestrictAlmonaster\">"\
            " From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictAlmonasterMin\" value=\""
            );
        
        if (pszRestrictAlmonasterMin != NULL) {
            m_pHttpResponse->WriteText (pszRestrictAlmonasterMin);
        } else {
            m_pHttpResponse->WriteText (fRestrictAlmonasterMin);
        }
        
        OutputText (
            "\"> to "\
            "<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictAlmonasterMax\" value=\""
            );
        
        if (pszRestrictAlmonasterMax != NULL) {
            m_pHttpResponse->WriteText (pszRestrictAlmonasterMax);
        } else {
            m_pHttpResponse->WriteText (fRestrictAlmonasterMax);
        }
        
        OutputText (
            "\">"\
            "</td>"\
            "</tr>"\
            
            "<tr>"\
            "<td>Empire filtering by Classic Score:</td>"\
            "<td><input type=\"checkbox\""
            );
        
        if (bRestrictClassic) {
            OutputText (" checked");
        }
        
        OutputText (
            " name=\"RestrictClassic\">"\
            " From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictClassicMin\" value=\""
            );
        
        if (pszRestrictClassicMin != NULL) {
            m_pHttpResponse->WriteText (pszRestrictClassicMin);
        } else {
            m_pHttpResponse->WriteText (fRestrictClassicMin);
        }
        
        OutputText (
            "\"> to "\
            "<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictClassicMax\" value=\""
            );
        
        if (pszRestrictClassicMax != NULL) {
            m_pHttpResponse->WriteText (pszRestrictClassicMax);
        } else {
            m_pHttpResponse->WriteText (fRestrictClassicMax);
        }
        
        OutputText (
            "\">"\
            "</td>"\
            "</tr>"

            "<tr>"\
            "<td>Empire filtering by Bridier Rank:</td>"\
            "<td><input type=\"checkbox\""
            );
        
        if (bRestrictBridierRank) {
            OutputText (" checked");
        }
        
        OutputText (
            " name=\"RestrictBridierRank\">"\
            " From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankMin\" value=\""
            );
        
        if (pszRestrictBridierRankMin != NULL) {
            m_pHttpResponse->WriteText (pszRestrictBridierRankMin);
        } else {
            m_pHttpResponse->WriteText (iRestrictBridierRankMin);
        }
        
        OutputText (
            "\"> to "\
            "<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankMax\" value=\""
            );
        
        if (pszRestrictBridierRankMax != NULL) {
            m_pHttpResponse->WriteText (pszRestrictBridierRankMax);
        } else {
            m_pHttpResponse->WriteText (iRestrictBridierRankMax);
        }
        
        OutputText (
            "\">"\
            "</td>"\
            "</tr>"
            
            "<tr>"\
            "<td>Empire filtering by Bridier Index:</td>"\
            "<td><input type=\"checkbox\""
            );
        
        if (bRestrictBridierIndex) {
            OutputText (" checked");
        }
        
        OutputText (
            " name=\"RestrictBridierIndex\">"\
            " From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierIndexMin\" value=\""
            );
        
        if (pszRestrictBridierIndexMin != NULL) {
            m_pHttpResponse->WriteText (pszRestrictBridierIndexMin);
        } else {
            m_pHttpResponse->WriteText (iRestrictBridierIndexMin);
        }
        
        OutputText (
            "\"> to "\
            "<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierIndexMax\" value=\""
            );
        
        if (pszRestrictBridierIndexMax != NULL) {
            m_pHttpResponse->WriteText (pszRestrictBridierIndexMax);
        } else {
            m_pHttpResponse->WriteText (iRestrictBridierIndexMax);
        }
        
        OutputText (
            "\">"\
            "</td>"\
            "</tr>"
            );
        

        if (iMaxNumEmpires == 2) {


            OutputText (

                "<tr>"\
                "<td>Empire filtering by Bridier Rank Gain:</td>"\
                "<td><input type=\"checkbox\""
                );
            
            if (bRestrictBridierRankGain) {
                OutputText (" checked");
            }
            
            OutputText (
                " name=\"RestrictBridierRankGain\">"\
                " From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankGainMin\" value=\""
                );
            
            if (pszRestrictBridierRankGainMin != NULL) {
                m_pHttpResponse->WriteText (pszRestrictBridierRankGainMin);
            } else {
                m_pHttpResponse->WriteText (iRestrictBridierRankGainMin);
            }
            
            OutputText (
                "\"> to "\
                "<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankGainMax\" value=\""
                );
            
            if (pszRestrictBridierRankGainMax != NULL) {
                m_pHttpResponse->WriteText (pszRestrictBridierRankGainMax);
            } else {
                m_pHttpResponse->WriteText (iRestrictBridierRankGainMax);
            }
            
            OutputText (
                "\">"\
                "</td>"\
                "</tr>"

                "<tr>"\
                "<td>Empire filtering by Bridier Rank Loss:</td>"\
                "<td><input type=\"checkbox\""
                );
            
            if (bRestrictBridierRankLoss) {
                OutputText (" checked");
            }
            
            OutputText (
                " name=\"RestrictBridierRankLoss\">"\
                " From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankLossMin\" value=\""
                );
            
            if (pszRestrictBridierRankLossMin != NULL) {
                m_pHttpResponse->WriteText (pszRestrictBridierRankLossMin);
            } else {
                m_pHttpResponse->WriteText (iRestrictBridierRankLossMin);
            }
            
            OutputText (
                "\"> to "\
                "<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictBridierRankLossMax\" value=\""
                );
            
            if (pszRestrictBridierRankLossMax != NULL) {
                m_pHttpResponse->WriteText (pszRestrictBridierRankLossMax);
            } else {
                m_pHttpResponse->WriteText (iRestrictBridierRankLossMax);
            }
            
            OutputText (
                "\">"\
                "</td>"\
                "</tr>"
                );
        }
        
        OutputText (
            "<tr>"\
            "<td>Empire filtering by number of Wins:</td>"\
            "<td><input type=\"checkbox\""
            );
        
        if (bRestrictWins) {
            OutputText (" checked");
        }
        
        OutputText (
            " name=\"RestrictWins\">"\
            " From <input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictWinsMin\" value=\""
            );
        
        if (pszRestrictWinsMin != NULL) {
            m_pHttpResponse->WriteText (pszRestrictWinsMin);
        } else {
            m_pHttpResponse->WriteText (iRestrictWinsMin);
        }
        
        OutputText (
            "\"> to "\
            "<input type=\"text\" size=\"6\" maxlength=\"20\" name=\"RestrictWinsMax\" value=\""
            );
        
        if (pszRestrictWinsMax != NULL) {
            m_pHttpResponse->WriteText (pszRestrictWinsMax);
        } else {
            m_pHttpResponse->WriteText (iRestrictWinsMax);
        }
        
        OutputText (
            "\">"\
            "</td>"\
            "</tr>"
            
            "<tr>"\
            "<td>Empire filtering by IP address:</td>"\
            "<td><select name=\"FilterIP\">"\
            "<option"
            );
        
        if (iFilterIP == 0) {
            OutputText (" selected");
        }
        
        OutputText (
            " value=\"0\">Ignore entry with duplicate IP address</option><option");
        
        if (iFilterIP == GAME_WARN_ON_DUPLICATE_IP_ADDRESS) {
            OutputText (" selected");
        }
        
        OutputText (" value=\"");
        m_pHttpResponse->WriteText (GAME_WARN_ON_DUPLICATE_IP_ADDRESS);
        OutputText ("\">Warn on entry with duplicate IP address</option><option");
        
        if (iFilterIP == GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS) {
            OutputText (" selected");
        }
        
        OutputText (" value=\"");
        m_pHttpResponse->WriteText (GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS);
        OutputText ("\">Reject on entry with duplicate IP address</option><option");
        
        if (iFilterIP == (GAME_WARN_ON_DUPLICATE_IP_ADDRESS | GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS)) {
            OutputText (" selected");
        }
        
        OutputText (" value=\"");
        m_pHttpResponse->WriteText (GAME_WARN_ON_DUPLICATE_IP_ADDRESS | GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS);
        OutputText ("\">Warn and reject on entry with duplicate IP address</option></select></td>"\
            
            "</tr>"\
            
            "<tr>"\
            "<td>Empire filtering by Session Id:</td>"\
            "<td><select name=\"FilterId\">"\
            "<option"
            );
        
        if (iFilterId == 0) {
            OutputText (" selected");
        }
        
        OutputText (
            " value=\"0\">"\
            "Ignore entry with duplicate Session Id</option><option"
            );
        
        if (iFilterId == GAME_WARN_ON_DUPLICATE_SESSION_ID) {
            OutputText (" selected");
        }
        
        OutputText (" value=\"");
        m_pHttpResponse->WriteText (GAME_WARN_ON_DUPLICATE_SESSION_ID);
        OutputText ("\">Warn on entry with duplicate Session Id</option><option");
        
        if (iFilterId == GAME_BLOCK_ON_DUPLICATE_SESSION_ID) {
            OutputText (" selected");
        }
        
        OutputText (" value=\"");
        m_pHttpResponse->WriteText (GAME_BLOCK_ON_DUPLICATE_SESSION_ID);
        OutputText ("\">Reject on entry with duplicate Session Id</option><option");
        
        if (iFilterId == (GAME_WARN_ON_DUPLICATE_SESSION_ID | GAME_BLOCK_ON_DUPLICATE_SESSION_ID)) {
            OutputText (" selected");
        }
        
        OutputText (" value=\"");
        m_pHttpResponse->WriteText (GAME_WARN_ON_DUPLICATE_SESSION_ID | GAME_BLOCK_ON_DUPLICATE_SESSION_ID);
        OutputText (
            "\">Warn and reject on entry with duplicate Session Id</option></select></td>"\
            
            "</tr>"\

            "<tr>"\
            "<td>Block idle empires:</td>"\
            "<td><select name=\"FilterIdle\">"\
            "<option"
            );

        if (!fFilterIdle) {
            OutputText (" selected");
        }
        
        OutputText (
            " value=\"0\">"\
            "Don't block empires who are idle in other games</option><option"
            );

        if (fFilterIdle) {
            OutputText (" selected");
        }
        
        OutputText (
            " value=\"1\">"\
            "Block empires who are idle in other games</option>"\
            "</select></td>"\
            "</tr>"\

            "<tr>"\
            "<td>Block specific empires:</td>"\
            "<td>"
            );

        pHttpForm = m_pHttpRequest->GetForm ("FilterEmpireName");
        if (pHttpForm != NULL) {

            iNumBlocks = pHttpForm->GetNumForms();

            for (unsigned int i = 0;  i < iNumBlocks; i ++) {
                
                const char* pszName = pHttpForm->GetForm(i)->GetValue();
                if (pszName != NULL && stricmp (pszName, m_vEmpireName.GetCharPtr()) != 0) {

                    bool bExists;
                    unsigned int iEmpireKey;
                    Variant vRealName;

                    // Make sure empire exists
                    iErrCode = g_pGameEngine->DoesEmpireExist (pszName, &bExists, &iEmpireKey, &vRealName, NULL);
                    if (iErrCode == OK && bExists && iEmpireKey != m_iEmpireKey) {

                        Assert (iEmpireKey != NO_KEY);

                        sprintf (pszFormIP, "FilterEmpireIP%i", i);
                        sprintf (pszFormID, "FilterEmpireID%i", i);

                        IHttpForm* pFormIP = m_pHttpRequest->GetForm (pszFormIP);
                        IHttpForm* pFormID = m_pHttpRequest->GetForm (pszFormID);
                        
                        OutputText ("Name: <input type=\"text\" name=\"FilterEmpireName\" size=\"20\" maxlength=\"");               
                        m_pHttpResponse->WriteText (MAX_EMPIRE_NAME_LENGTH);
                        OutputText ("\" value=\"");
                        m_pHttpResponse->WriteText (vRealName.GetCharPtr());

                        OutputText ("\"> <input type=\"checkbox\" ");

                        if (pFormIP != NULL) {
                            OutputText ("checked ");
                        }

                        OutputText ("name=\"");
                        m_pHttpResponse->WriteText (pszFormIP);
                        OutputText ("\">Same IP Address <input type=\"checkbox\" ");

                        if (pFormID != NULL) {
                            OutputText ("checked ");
                        }
                        
                        OutputText ("name=\"");
                        m_pHttpResponse->WriteText (pszFormID);
                        OutputText ("\">Same Session Id<br>");
                    }
                }
            }
        }

        sprintf (pszFormIP, "FilterEmpireIP%i", iNumBlocks);
        sprintf (pszFormID, "FilterEmpireID%i", iNumBlocks);

        OutputText ("Name: <input type=\"text\" name=\"FilterEmpireName\" size=\"20\" maxlength=\"");               
        m_pHttpResponse->WriteText (MAX_EMPIRE_NAME_LENGTH);
        OutputText ("\"> <input type=\"checkbox\" name=\"");
        m_pHttpResponse->WriteText (pszFormIP);
        OutputText ("\">Same IP Address <input type=\"checkbox\" name=\"");
        m_pHttpResponse->WriteText (pszFormID);
        OutputText ("\">Same Session Id&nbsp;&nbsp;");

        WriteButton (BID_BLOCK);

        OutputText ("</td></tr>");
    }
        
    OutputText ("</table><p>");
    
    return;
    
OnError:
    
    OutputText ("Error ");
    m_pHttpResponse->WriteText (iErrCode);
    OutputText (" occurred while processing this page");
}


int HtmlRenderer::ParseGameConfigurationForms (int iGameClass, unsigned int iTournamentKey,
                                               const Variant* pvGameClassInfo, int iEmpireKey, 
                                               GameOptions* pgoOptions) {
    
    int iErrCode, iTemp, iGameClassOptions, iMaxNumEmpires;
    bool bFlag;
    Variant vMaxUpdatesBeforeGameCloses;

    const char* pszPassword;
    size_t stLen;
    
    Seconds sUpdatePeriod;
    
    IHttpForm* pHttpForm;
    
    pgoOptions->iOptions = 0;
    pgoOptions->pszPassword = NULL;

    // Check for refresh requests
    if (WasButtonPressed (BID_BLOCK)) {
        return WARNING;
    }
    
    // MaxUpdatesBeforeGameCloses
    iErrCode = g_pGameEngine->GetSystemProperty (SystemData::MaxNumUpdatesBeforeClose, &vMaxUpdatesBeforeGameCloses);
    if (iErrCode != OK) {
        goto OnError;
    }
    
    if (iGameClass == NO_KEY) {

        Assert (pvGameClassInfo != NULL);

        sUpdatePeriod = pvGameClassInfo[SystemGameClassData::NumSecPerUpdate].GetInteger();
        iGameClassOptions = pvGameClassInfo[SystemGameClassData::Options].GetInteger();
        iMaxNumEmpires = pvGameClassInfo[SystemGameClassData::MaxNumEmpires].GetInteger();
    
    } else {

        Assert (pvGameClassInfo == NULL);

        iErrCode = g_pGameEngine->GetGameClassUpdatePeriod (iGameClass, &sUpdatePeriod);
        if (iErrCode != OK) {
            goto OnError;
        }

        iErrCode = g_pGameEngine->GetGameClassOptions (iGameClass, &iGameClassOptions);
        if (iErrCode != OK) {
            goto OnError;
        }

        iErrCode = g_pGameEngine->GetMaxNumEmpires (iGameClass, &iMaxNumEmpires);
        if (iErrCode != OK) {
            goto OnError;
        }
    }
    
    // NumUpdatesForClose
    if (iTournamentKey == NO_KEY) {

        if ((pHttpForm = m_pHttpRequest->GetForm ("NumUpdatesForClose")) == NULL) {
            AddMessage ("Missing NumUpdatesForClose form");
            return ERROR_FAILURE;
        }
        pgoOptions->iNumUpdatesBeforeGameCloses = pHttpForm->GetIntValue();
    
    } else {
        
        pgoOptions->iNumUpdatesBeforeGameCloses = 1;
    }
    
    if (pgoOptions->iNumUpdatesBeforeGameCloses > vMaxUpdatesBeforeGameCloses.GetInteger()) {
        AddMessage ("The number of updates before the game closes is too high");
        return ERROR_FAILURE;
    }
    
    // FirstUpdateDelay
    if ((pHttpForm = m_pHttpRequest->GetForm ("HoursUD")) == NULL) {
        AddMessage ("Missing HoursUD form");
        return ERROR_FAILURE;
    }
    pgoOptions->sFirstUpdateDelay = 60 * 60 * pHttpForm->GetIntValue();
    
    if ((pHttpForm = m_pHttpRequest->GetForm ("MinsUD")) == NULL) {
        AddMessage ("Missing MinsUD form");
        return ERROR_FAILURE;
    }
    pgoOptions->sFirstUpdateDelay += 60 * pHttpForm->GetIntValue();
    
    if ((pHttpForm = m_pHttpRequest->GetForm ("SecsUD")) == NULL) {
        AddMessage ("Missing SecsUD form");
        return ERROR_FAILURE;
    }
    pgoOptions->sFirstUpdateDelay += pHttpForm->GetIntValue();

    if (iTournamentKey == NO_KEY) {

        // NamesListed
        if ((pHttpForm = m_pHttpRequest->GetForm ("NamesListed")) == NULL) {
            AddMessage ("Missing NamesListed form");
            return ERROR_FAILURE;
        }
        if (pHttpForm->GetIntValue() != 0) {
            pgoOptions->iOptions |= GAME_NAMES_LISTED;
        }

    } else {

        // Tournament games always have exposed names
        pgoOptions->iOptions |= GAME_NAMES_LISTED;
    }

    // Spectators
    if ((pHttpForm = m_pHttpRequest->GetForm ("Spectators")) != NULL && pHttpForm->GetIntValue() != 0) {

        if ((iGameClassOptions & EXPOSED_SPECTATORS) != EXPOSED_SPECTATORS) {
            AddMessage ("The game could not be made available to spectators as it does not have exposed maps and exposed diplomacy");
        } else {
            pgoOptions->iOptions |= GAME_ALLOW_SPECTATORS;
        }
    }

    // First update delay
    if (pgoOptions->sFirstUpdateDelay < 0) {
        AddMessage ("Invalid first update delay");
        return ERROR_FAILURE;
    }
    
    if (pgoOptions->sFirstUpdateDelay > sUpdatePeriod * MAX_NUM_UPDATE_PERIODS_FOR_FIRST_DELAY) {
        AddMessage ("The first update delay is too large");
        return ERROR_FAILURE;
    }
    
    // Bridier
    if ((pHttpForm = m_pHttpRequest->GetForm ("Bridier")) != NULL) {
        
        if (pHttpForm->GetIntValue() == GAME_COUNT_FOR_BRIDIER) {
            
            if (iMaxNumEmpires != 2) {
                AddMessage ("The game cannot count towards Bridier Scoring because more than two empires can play");
            } else {
                pgoOptions->iOptions |= GAME_COUNT_FOR_BRIDIER;
            }
        }
    }
    
    // EnterGameMessage
    if ((pHttpForm = m_pHttpRequest->GetForm ("EnterGameMessage")) == NULL) {
        AddMessage ("Missing EnterGameMessage form");
        return ERROR_FAILURE;
    }
    
    pgoOptions->pszEnterGameMessage = pHttpForm->GetValue();
    if (pgoOptions->pszEnterGameMessage != NULL) {
        
        if (strlen (pgoOptions->pszEnterGameMessage) > MAX_ENTER_GAME_MESSAGE_LENGTH) {
            AddMessage ("The message sent to players entering the game is too long");
            return ERROR_FAILURE;
        }
    }

    if (iTournamentKey == NO_KEY) {
    
        // Password
        if ((pHttpForm = m_pHttpRequest->GetForm ("GamePassword")) == NULL) {
            AddMessage ("Missing GamePassword form");
            return ERROR_FAILURE;
        }
        
        pszPassword = pHttpForm->GetValue();
        stLen = String::StrLen (pszPassword);
        
        if (stLen > 0) {
            
            if (stLen > MAX_PASSWORD_LENGTH) {
                AddMessage ("The game password was too long");
                return ERROR_FAILURE;
            }
            
            if ((pHttpForm = m_pHttpRequest->GetForm ("GamePassword2")) == NULL) {
                AddMessage ("Missing GamePassword2 form");
                return ERROR_FAILURE;
            }
            
            if (String::StrCmp (pszPassword, pHttpForm->GetValue()) != 0) {
                AddMessage ("The game password was not properly confirmed");
                return ERROR_FAILURE;
            }
            
            // Check password
            if (VerifyPassword (pszPassword) != OK) {
                AddMessage ("The game password contained an invalid character");
                return ERROR_FAILURE;
            }
            
            pgoOptions->pszPassword = pszPassword;
        }
        
        // FilterIP
        if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIP")) == NULL) {
            AddMessage ("Missing FilterIP form");
            return ERROR_FAILURE;
        }
        iTemp = pHttpForm->GetIntValue();
        
        if (((iTemp & ~GAME_WARN_ON_DUPLICATE_IP_ADDRESS) & ~GAME_BLOCK_ON_DUPLICATE_IP_ADDRESS) != 0) {
            AddMessage ("Incorrect FilterIP value");
            return ERROR_FAILURE;
        }
        
        pgoOptions->iOptions |= iTemp;
        
        // FilterId
        if ((pHttpForm = m_pHttpRequest->GetForm ("FilterId")) == NULL) {
            AddMessage ("Missing FilterId form");
            return ERROR_FAILURE;
        }
        iTemp = pHttpForm->GetIntValue();
        
        if (((iTemp & ~GAME_WARN_ON_DUPLICATE_SESSION_ID) & ~GAME_BLOCK_ON_DUPLICATE_SESSION_ID) != 0) {
            AddMessage ("Incorrect FilterId value");
            return ERROR_FAILURE;
        }

        pgoOptions->iOptions |= iTemp;

        // FilterIdle
        if ((pHttpForm = m_pHttpRequest->GetForm ("FilterIdle")) == NULL) {
            AddMessage ("Missing FilterIdle form");
            return ERROR_FAILURE;
        }
        if (pHttpForm->GetIntValue() != 0) {
            pgoOptions->iOptions |= GAME_RESTRICT_IDLE_EMPIRES;
        }

        //
        // Score filtering
        //
        
        // Almonaster Score
        if (m_pHttpRequest->GetForm ("RestrictAlmonaster") != NULL) {
            
            float fMin, fMax;
            
            // Min
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictAlmonasterMin")) == NULL) {
                AddMessage ("Missing RestrictAlmonasterMin form");
                return ERROR_FAILURE;
            }
            fMin = pHttpForm->GetFloatValue();
            
            if (fMin < ALMONASTER_MIN_SCORE || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
                AddMessage ("Invalid minimum Almonaster Score");
                return ERROR_FAILURE;
            }
            
            if (fMin != ALMONASTER_MIN_SCORE) {
                pgoOptions->iOptions |= GAME_RESTRICT_MIN_ALMONASTER_SCORE;
                pgoOptions->fMinAlmonasterScore = fMin;
            }
            
            // Max
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictAlmonasterMax")) == NULL) {
                AddMessage ("Missing RestrictAlmonasterMax form");
                return ERROR_FAILURE;
            }
            
            if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
                
                fMax = pHttpForm->GetFloatValue();
                
                if (fMin > fMax) {
                    AddMessage ("Invalid Almonaster Score restrictions");
                    return ERROR_FAILURE;
                }
                
                if (fMax > ALMONASTER_MAX_SCORE || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
                    AddMessage ("Invalid maximum Almonaster Score");
                    return ERROR_FAILURE;
                }
                
                pgoOptions->iOptions |= GAME_RESTRICT_MAX_ALMONASTER_SCORE;
                pgoOptions->fMaxAlmonasterScore = fMax;
            }
        }
        
        // Classic Score
        if (m_pHttpRequest->GetForm ("RestrictClassic") != NULL) {
            
            float fMin, fMax;
            
            // Min
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictClassicMin")) == NULL) {
                AddMessage ("Missing RestrictClassicMin form");
                return ERROR_FAILURE;
            }
            
            if (String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
                fMin = CLASSIC_MIN_SCORE;       
            } else {
                
                fMin = pHttpForm->GetFloatValue();
                
                if (fMin < CLASSIC_MIN_SCORE || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
                    AddMessage ("Invalid minimum Classic Score");
                    return ERROR_FAILURE;
                }
                
                if (fMin != CLASSIC_MIN_SCORE) {
                    pgoOptions->iOptions |= GAME_RESTRICT_MIN_CLASSIC_SCORE;
                    pgoOptions->fMinClassicScore = fMin;
                }
            }
            
            // Max
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictClassicMax")) == NULL) {
                AddMessage ("Missing RestrictClassicMax form");
                return ERROR_FAILURE;
            }
            
            if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
                
                fMax = pHttpForm->GetFloatValue();
                
                if (fMin > fMax) {
                    AddMessage ("Invalid Classic Score restrictions");
                    return ERROR_FAILURE;
                }
                
                if (fMax > CLASSIC_MAX_SCORE || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
                    AddMessage ("Invalid maximum Classic Score");
                    return ERROR_FAILURE;
                }
                
                pgoOptions->iOptions |= GAME_RESTRICT_MAX_CLASSIC_SCORE;
                pgoOptions->fMaxClassicScore = fMax;
            }
        }
                    
        // Bridier Rank
        if (m_pHttpRequest->GetForm ("RestrictBridierRank") != NULL) {
            
            int iMin, iMax;
            
            // Min
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankMin")) == NULL) {
                AddMessage ("Missing RestrictBridierRankMin form");
                return ERROR_FAILURE;
            }
            iMin = pHttpForm->GetIntValue();
            
            if (iMin < BRIDIER_MIN_RANK || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
                AddMessage ("Invalid minimum Bridier Rank");
                return ERROR_FAILURE;
            }
            
            if (iMin != BRIDIER_MIN_RANK) {
                pgoOptions->iOptions |= GAME_RESTRICT_MIN_BRIDIER_RANK;
                pgoOptions->iMinBridierRank = iMin;
            }

            // Max
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankMax")) == NULL) {
                AddMessage ("Missing RestrictBridierRankMax form");
                return ERROR_FAILURE;
            }
            
            if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
                
                iMax = pHttpForm->GetIntValue();
                
                if (iMin > iMax) {
                    AddMessage ("Invalid Bridier Rank restrictions");
                    return ERROR_FAILURE;
                }
                
                if (iMax > BRIDIER_MAX_RANK || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
                    AddMessage ("Invalid maximum Bridier Rank");
                    return ERROR_FAILURE;
                }
                
                pgoOptions->iOptions |= GAME_RESTRICT_MAX_BRIDIER_RANK;
                pgoOptions->iMaxBridierRank = iMax;
            }
        }
        
        // Bridier Index
        if (m_pHttpRequest->GetForm ("RestrictBridierIndex") != NULL) {
            
            int iMin, iMax;
            
            // Min
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierIndexMin")) == NULL) {
                AddMessage ("Missing RestrictBridierIndexMin form");
                return ERROR_FAILURE;
            }
            iMin = pHttpForm->GetIntValue();
            
            if (iMin < BRIDIER_MIN_INDEX || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
                AddMessage ("Invalid minimum Bridier Index");
                return ERROR_FAILURE;
            }
            
            if (iMin != BRIDIER_MIN_INDEX) {
                pgoOptions->iOptions |= GAME_RESTRICT_MIN_BRIDIER_INDEX;
                pgoOptions->iMinBridierIndex = iMin;
            }
            
            // Max
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierIndexMax")) == NULL) {
                AddMessage ("Missing RestrictBridierIndexMax form");
                return ERROR_FAILURE;
            }
            
            if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
                
                iMax = pHttpForm->GetIntValue();
                
                if (iMin > iMax) {
                    AddMessage ("Invalid Bridier Index restrictions");
                    return ERROR_FAILURE;
                }
                
                if (iMax > BRIDIER_MAX_INDEX || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
                    AddMessage ("Invalid maximum Bridier Index");
                    return ERROR_FAILURE;
                }
                
                pgoOptions->iOptions |= GAME_RESTRICT_MAX_BRIDIER_INDEX;
                pgoOptions->iMaxBridierIndex = iMax;
            }
        }

        if (pgoOptions->iOptions & GAME_COUNT_FOR_BRIDIER) {
            
            // Bridier RankGain
            if (m_pHttpRequest->GetForm ("RestrictBridierRankGain") != NULL) {
                
                int iMin, iMax;
                
                // Min
                if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankGainMin")) == NULL) {
                    AddMessage ("Missing RestrictBridierRankGainMin form");
                    return ERROR_FAILURE;
                }
                iMin = pHttpForm->GetIntValue();
                
                if (iMin < BRIDIER_MIN_RANK_GAIN || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
                    AddMessage ("Invalid minimum Bridier Rank Gain");
                    return ERROR_FAILURE;
                }
                
                if (iMin != BRIDIER_MIN_RANK_GAIN) {
                    pgoOptions->iOptions |= GAME_RESTRICT_MIN_BRIDIER_RANK_GAIN;
                    pgoOptions->iMinBridierRankGain = iMin;
                }
                
                // Max
                if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankGainMax")) == NULL) {
                    AddMessage ("Missing RestrictBridierRankGainMax form");
                    return ERROR_FAILURE;
                }
                
                if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
                    
                    iMax = pHttpForm->GetIntValue();
                    
                    if (iMin > iMax) {
                        AddMessage ("Invalid Bridier Rank Gain restrictions");
                        return ERROR_FAILURE;
                    }
                    
                    if (iMax > BRIDIER_MAX_RANK_GAIN || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
                        AddMessage ("Invalid maximum Bridier Rank Gain");
                        return ERROR_FAILURE;
                    }
                    
                    pgoOptions->iOptions |= GAME_RESTRICT_MAX_BRIDIER_RANK_GAIN;
                    pgoOptions->iMaxBridierRankGain = iMax;
                }
            }

            // Bridier RankLoss
            if (m_pHttpRequest->GetForm ("RestrictBridierRankLoss") != NULL) {
                
                int iMin, iMax;
                
                // Min
                if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankLossMin")) == NULL) {
                    AddMessage ("Missing RestrictBridierRankLossMin form");
                    return ERROR_FAILURE;
                }
                iMin = pHttpForm->GetIntValue();
                
                if (iMin < BRIDIER_MIN_RANK_LOSS || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
                    AddMessage ("Invalid minimum Bridier Rank Gain");
                    return ERROR_FAILURE;
                }
                
                if (iMin != BRIDIER_MIN_RANK_LOSS) {
                    pgoOptions->iOptions |= GAME_RESTRICT_MIN_BRIDIER_RANK_LOSS;
                    pgoOptions->iMinBridierRankLoss = iMin;
                }
                
                // Max
                if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictBridierRankLossMax")) == NULL) {
                    AddMessage ("Missing RestrictBridierRankLossMax form");
                    return ERROR_FAILURE;
                }
                
                if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
                    
                    iMax = pHttpForm->GetIntValue();
                    
                    if (iMin > iMax) {
                        AddMessage ("Invalid Bridier Rank Loss restrictions");
                        return ERROR_FAILURE;
                    }
                    
                    if (iMax > BRIDIER_MAX_RANK_LOSS || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
                        AddMessage ("Invalid maximum Bridier Rank Loss");
                        return ERROR_FAILURE;
                    }
                    
                    pgoOptions->iOptions |= GAME_RESTRICT_MAX_BRIDIER_RANK_LOSS;
                    pgoOptions->iMaxBridierRankLoss = iMax;
                }
            }
        }
        
        // Wins
        if (m_pHttpRequest->GetForm ("RestrictWins") != NULL) {
            
            int iMin, iMax;
            
            // Min
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictWinsMin")) == NULL) {
                AddMessage ("Missing RestrictWinsMin form");
                return ERROR_FAILURE;
            }
            iMin = pHttpForm->GetIntValue();
            
            if (iMin < MIN_NUM_WINS || String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) == 0) {
                AddMessage ("Invalid minimum Wins");
                return ERROR_FAILURE;
            }
            
            if (iMin != MIN_NUM_WINS) {
                pgoOptions->iOptions |= GAME_RESTRICT_MIN_WINS;
                pgoOptions->iMinWins = iMin;
            }
            
            // Max
            if ((pHttpForm = m_pHttpRequest->GetForm ("RestrictWinsMax")) == NULL) {
                AddMessage ("Missing RestrictWinsMax form");
                return ERROR_FAILURE;
            }
            
            if (String::StriCmp (pHttpForm->GetValue(), HIGHEST_STRING) != 0) {
                
                iMax = pHttpForm->GetIntValue();
                
                if (iMin > iMax) {
                    AddMessage ("Invalid Wins restrictions");
                    return ERROR_FAILURE;
                }
                
                if (iMax > MAX_NUM_WINS || String::StriCmp (pHttpForm->GetValue(), LOWEST_STRING) == 0) {
                    AddMessage ("Invalid maximum Wins");
                    return ERROR_FAILURE;
                }
                
                pgoOptions->iOptions |= GAME_RESTRICT_MAX_WINS;
                pgoOptions->iMaxWins = iMax;
            }
        }

        // Handle specific empire filtering
        pHttpForm = m_pHttpRequest->GetForm ("FilterEmpireName");
        if (pHttpForm != NULL) {

            unsigned int i, j, iNumBlocks = pHttpForm->GetNumForms(), iNumRealBlocks = 0;
            if (iNumBlocks > 1 || iNumBlocks == 1 && pHttpForm->GetValue() != NULL) {

                pgoOptions->pSecurity = new GameSecurityEntry [iNumBlocks];
                if (pgoOptions->pSecurity == NULL) {
                    AddMessage ("The server is out of memory");
                    return ERROR_OUT_OF_MEMORY;
                }

    #ifdef _DEBUG
                memset (pgoOptions->pSecurity, 0xde, iNumBlocks * sizeof (GameSecurityEntry));
    #endif
                for (i = 0;  i < iNumBlocks; i ++) {
                
                    const char* pszName = pHttpForm->GetForm(i)->GetValue();
                    if (pszName != NULL && stricmp (pszName, m_vEmpireName.GetCharPtr()) != 0) {

                        unsigned int iEmpireKey;
                        int64 iSecretKey;

                        // Make sure empire exists
                        iErrCode = g_pGameEngine->DoesEmpireExist (pszName, &bFlag, &iEmpireKey, NULL, &iSecretKey);
                        if (iErrCode == OK && bFlag && iEmpireKey != m_iEmpireKey) {

                            Assert (iEmpireKey != NO_KEY);

                            bool bAlready = false;

                            char pszFormIP [64];
                            char pszFormID [64];

                            sprintf (pszFormIP, "FilterEmpireIP%i", i);
                            sprintf (pszFormID, "FilterEmpireID%i", i);

                            IHttpForm* pFormIP = m_pHttpRequest->GetForm (pszFormIP);
                            IHttpForm* pFormID = m_pHttpRequest->GetForm (pszFormID);

                            int iOptions = 0;

                            if (pFormIP != NULL) {
                                iOptions |= GAME_SECURITY_CHECK_IPADDRESS;
                            }

                            if (pFormID != NULL) {
                                iOptions |= GAME_SECURITY_CHECK_SESSIONID;
                            }

                            // Make sure name hasn't been used already
                            for (j = 0; j < iNumRealBlocks; j ++) {

                                if (stricmp (pgoOptions->pSecurity[j].pszEmpireName, pszName) == 0) {
                                    bAlready = true;
                                    break;
                                }
                            }

                            if (!bAlready) {
                            
                                pgoOptions->pSecurity[iNumRealBlocks].iEmpireKey = iEmpireKey;
                                pgoOptions->pSecurity[iNumRealBlocks].iSecretKey = iSecretKey;
                                pgoOptions->pSecurity[iNumRealBlocks].iOptions = iOptions;
                                pgoOptions->pSecurity[iNumRealBlocks].pszEmpireName = pszName;

                                iNumRealBlocks ++;
                            }
                        }
                    }
                }

                if (iNumRealBlocks > 0) {
                    pgoOptions->iOptions |= GAME_ENFORCE_SECURITY;
                    pgoOptions->iNumSecurityEntries = iNumRealBlocks;
                }
            }
        }
    
    } else {

        pgoOptions->pszPassword = NULL;
        pgoOptions->pSecurity = NULL;

        pgoOptions->fMinAlmonasterScore = 0;
        pgoOptions->fMaxAlmonasterScore = 0;
        pgoOptions->fMinClassicScore = 0;
        pgoOptions->fMaxClassicScore = 0;
        pgoOptions->iMinBridierRank = 0;
        pgoOptions->iMaxBridierRank = 0;
        pgoOptions->iMinBridierIndex = 0;
        pgoOptions->iMaxBridierIndex = 0;
        pgoOptions->iMinBridierRankGain = 0;
        pgoOptions->iMaxBridierRankGain = 0;
        pgoOptions->iMinBridierRankLoss = 0;
        pgoOptions->iMaxBridierRankLoss = 0;
        pgoOptions->iMinWins = 0;
        pgoOptions->iMaxWins = 0;

        pgoOptions->iNumSecurityEntries = 0;
    }

    return OK;
    
OnError:
    
    AddMessage ("Error reading gameclass data");
    
    return iErrCode;
}