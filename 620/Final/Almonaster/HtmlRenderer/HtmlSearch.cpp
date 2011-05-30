//
// Almonaster.dll:  a component of Almonaster
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

const SearchField g_AdvancedSearchFields[] = {

    { true, NULL, "EmpName", "TEmpName", "SEmpName", SystemEmpireData::Name, SEARCHFIELD_STRING, GUEST },
    { false, "Empire Key", "EmpKey", "LEmpKey", "REmpKey", NO_KEY, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "RealName", "TRealName", "SRealName", SystemEmpireData::RealName, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "Loc", "TLoc", "SLoc", SystemEmpireData::Location, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "Email", "TEmail", "SEmail", SystemEmpireData::Email, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "PrivM", "TPrivM", "SPrivM", SystemEmpireData::PrivateEmail, SEARCHFIELD_STRING, ADMINISTRATOR },
    { false, NULL, "WebPage", "TWebPage", "SWebPage", SystemEmpireData::WebPage, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "IM", "TIM", "SIM", SystemEmpireData::IMId, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "Quote", "TQuote", "SQuote", SystemEmpireData::Quote, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "Sneer", "TSneer", "SSneer", SystemEmpireData::VictorySneer, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "IPA", "TIPA", "SIPA", SystemEmpireData::IPAddress, SEARCHFIELD_STRING, ADMINISTRATOR },
    { false, NULL, "SId", "LSId", "RSId", SystemEmpireData::SessionId, SEARCHFIELD_INTEGER64, ADMINISTRATOR },
    { false, NULL, "Browser", "TBrowser", "SBrowser", SystemEmpireData::Browser, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "Privl", "SPrivl", NULL, SystemEmpireData::Privilege, SEARCHFIELD_PRIVILEGE, GUEST },
    { false, NULL, "NumL", "LNumL", "RNumL", SystemEmpireData::NumLogins, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "LastL", "LLastL", "RLastL", SystemEmpireData::LastLoginTime, SEARCHFIELD_DATE, GUEST },
    { false, NULL, "Creat", "LCreat", "RCreat", SystemEmpireData::CreationTime, SEARCHFIELD_DATE, GUEST },
    { false, NULL, "AlienKey", "LAlienKey", "RAlienKey", SystemEmpireData::AlienKey, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "Wins", "LWins", "RWins", SystemEmpireData::Wins, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "Nukes", "LNukes", "RNukes", SystemEmpireData::Nukes, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "Nuked", "LNuked", "RNuked", SystemEmpireData::Nuked, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "Draws", "LDraws", "RDraws", SystemEmpireData::Draws, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "AScore", "LASCore", "RAScore", SystemEmpireData::AlmonasterScore, SEARCHFIELD_FLOAT, GUEST },
    { false, NULL, "AScoreSig", "LAScoreSig", "RAScoreSig", SystemEmpireData::AlmonasterScoreSignificance, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "CScore", "LCScore", "RCScore", SystemEmpireData::ClassicScore, SEARCHFIELD_FLOAT, GUEST },
    { false, NULL, "BridRank", "LBridRank", "RBridRank", SystemEmpireData::BridierRank, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "BridInd", "LBridInd", "RBridInd", SystemEmpireData::BridierIndex, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "MaxEcon", "LMaxEcon", "RMaxEcon", SystemEmpireData::MaxEcon, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "MaxMil", "LMaxMil", "RMaxMil", SystemEmpireData::MaxMil, SEARCHFIELD_INTEGER, GUEST },
};


void HtmlRenderer::RenderSearchForms (bool fAdvanced) {

    Assert (MAX_NUM_SEARCH_COLUMNS == sizeof (g_AdvancedSearchFields) / sizeof (SearchField));
    
    int iNumEmpires, iErrCode = g_pGameEngine->GetNumEmpiresOnServer (&iNumEmpires);
    if (iErrCode != OK) {
        OutputText ("<p>Error reading empire list");
        return;
    }
    
    OutputText ("<p>There ");
    
    if (iNumEmpires == 1) {
        OutputText ("is <strong>1</strong> registered empire ");
    } else {
        OutputText ("are <strong>");
        m_pHttpResponse->WriteText (iNumEmpires);
        OutputText ("</strong> registered empires");
    }
    OutputText (" on the server");
    
    if (!fAdvanced) {
        
        OutputText (
            "<input type=\"hidden\" name=\"MaxNumHits\" value=\"1\">"\
            "<input type=\"hidden\" name=\"Skip\" value=\"0\">"
            );

        OutputText ("<p>Search for the following empire:<p><table>");
        RenderSearchField (g_AdvancedSearchFields[0], false);
        OutputText ("</table><p>");
    
    } else {

        OutputText (
            "<p>Choose the characteristics of the empires you wish to find:"\
            "<p><table width=\"75%\">"\
            );

        // Loop through all possible search fields
        for (int i = 0; i < MAX_NUM_SEARCH_COLUMNS; i ++) {
            RenderSearchField (g_AdvancedSearchFields[i], true);
        }

        OutputText (
            "</table><p>Return up to <input type=\"text\" size=\"3\" name=\"MaxNumHits\" value=\"20\"> "\
            "hits per page, skip the first <input type=\"text\" size=\"3\" name=\"Skip\" value=\"0\"><p>"
            );
    }

    WriteButton (BID_SEARCH);
}


//
// Assumes we're already inside a table
//

void HtmlRenderer::RenderSearchField (const SearchField& sfField, bool fAdvanced) {

    // Privilege check
    if (sfField.prvMinPriv > m_iPrivilege) return;

    const char* pszName = sfField.pszName;
    if (pszName == NULL) {
        Assert (sfField.iSystemEmpireDataColumn != NO_KEY);
        pszName = SYSTEM_EMPIRE_DATA_COLUMN_NAMES [sfField.iSystemEmpireDataColumn];
    }

    // Common rendering
    if (!fAdvanced) {

        OutputText ("<input type=\"hidden\" name=\"");
        m_pHttpResponse->WriteText (sfField.pszInputCheckBox);
        OutputText ("\" value=\"1\"><tr><td>");

    } else {

        OutputText ("<tr><td><input type=\"checkbox\"");
        if (sfField.bCheckedByDefault) {
            OutputText (" checked");
        }

        OutputText (" name=\"");
        m_pHttpResponse->WriteText (sfField.pszInputCheckBox);
        OutputText ("\"></td><td>");
    }

    m_pHttpResponse->WriteText (pszName);
    OutputText (":</td>");

    // Type-specific rendering
    switch (sfField.sftType) {
        
    case SEARCHFIELD_INTEGER:
    case SEARCHFIELD_INTEGER64:
    case SEARCHFIELD_FLOAT:

        OutputText ("<td><input type=\"text\" size=\"6\" name=\"");
        m_pHttpResponse->WriteText (sfField.pszInput1);
        OutputText ("\" value=\"0\"> to <input type=\"text\" size=\"6\" name=\"");
        m_pHttpResponse->WriteText (sfField.pszInput2);
        OutputText ("\" value=\"0\">");

        break;
        
    case SEARCHFIELD_STRING:
        
        OutputText ("<td><input type=\"text\" size=\"20\" name=\"");
        m_pHttpResponse->WriteText (sfField.pszInput1);
        OutputText ("\"></td><td><select name=\"");
        m_pHttpResponse->WriteText (sfField.pszInput2);
        OutputText ("\" size=\"1\"><option selected value=\"");

        m_pHttpResponse->WriteText (SEARCH_EXACT);
        OutputText ("\">Exact search</option><option value=\"");
        m_pHttpResponse->WriteText (SEARCH_SUBSTRING);
        OutputText ("\">Substring search</option><option value=\"");
        m_pHttpResponse->WriteText (SEARCH_BEGINS_WITH);
        OutputText ("\">Begins with search</option></select>");
        
        break;
        
    case SEARCHFIELD_PRIVILEGE:
        
        OutputText ("<td><select name=\"");
        m_pHttpResponse->WriteText (sfField.pszInput1);
        OutputText ("\" size=\"1\">");

        int i, j;
        ENUMERATE_PRIVILEGE_LEVELS(i) {
            
            if (i == NOVICE) {
                OutputText ("<option selected");
            } else {
                OutputText ("<option");
            }
            OutputText (" value=\"");
            m_pHttpResponse->WriteText (i);
            OutputText ("\">");
            m_pHttpResponse->WriteText (PRIVILEGE_STRING[i]);
            OutputText ("</option>");
        }
        
        OutputText ("</select>");
        break;
        
    case SEARCHFIELD_DATE:

        OutputText ("<td colspan=\"2\">");
        RenderDateField (sfField.pszInput1);
        OutputText (" to ");
        RenderDateField (sfField.pszInput2);

        break;
        
    default:
        
        Assert (false);
        break;
    }

    OutputText ("</td></tr>");
}

void HtmlRenderer::RenderDateField (const char* pszField) {
    
    OutputText ("<select name=\"");
    m_pHttpResponse->WriteText (pszField);
    OutputText ("Month\">");

    for (int i = 1; i <= 12; i ++) {

        OutputText ("<option value=\">");
        m_pHttpResponse->WriteText (i);
        OutputText ("\">");
        m_pHttpResponse->WriteText (Time::GetAbbreviatedMonthName (i));
        OutputText ("</option>");
    }
    
    OutputText ("</select> <input type=\"text\" size=\"2\" name=\"");
    m_pHttpResponse->WriteText (pszField);
    OutputText ("Day\" value=\"01\"> to <input type=\"text\" size=\"4\" name=\"");
    m_pHttpResponse->WriteText (pszField);
    OutputText ("\" value=\"2002\">");
}

bool HtmlRenderer::ParseDateField (const char* pszField, UTCTime* ptTime) {

    IHttpForm* pHttpForm;

    // Year
    pHttpForm = m_pHttpRequest->GetForm (pszField);
    if (pHttpForm == NULL || pHttpForm->GetValue() == NULL) {
        return false;
    }

    int iYear = pHttpForm->GetIntValue();

    // Month
    char* pszBuffer = (char*) StackAlloc (strlen (pszField) + 32);
    strcpy (pszBuffer, pszField);
    strcat (pszBuffer, "Month");

    pHttpForm = m_pHttpRequest->GetForm (pszBuffer);
    if (pHttpForm == NULL || pHttpForm->GetValue() == NULL) {
        return false;
    }

    int iMonth = pHttpForm->GetIntValue();

    // Day
    strcpy (pszBuffer, pszField);
    strcat (pszBuffer, "Day");

    pHttpForm = m_pHttpRequest->GetForm (pszBuffer);
    if (pHttpForm == NULL || pHttpForm->GetValue() == NULL) {
        return false;
    }

    int iDay = pHttpForm->GetIntValue();

    Time::GetTime (0, 0, 0, iDay, iMonth, iYear, ptTime);

    return true;
}

int HtmlRenderer::HandleSearchSubmission (unsigned int* piSearchCol, 
                                          Variant* pvSearchColData1,
                                          Variant* pvSearchColData2,
                                          const char** pszFormName,
                                          const char** pszColName1,
                                          const char** pszColName2,
                                          
                                          int* piNumSearchColumns,
                                          
                                          int** ppiSearchEmpireKey,
                                          int* piNumSearchEmpires,
                                          int* piLastKey,
                                          int* piMaxNumHits
                                          ) {

    IHttpForm* pHttpForm, * pHttpForm1, * pHttpForm2;

    int i, iErrCode = OK, iMaxNumHits = 0, iNumSearchColumns = 0, iSkip = 0, iStartKey = 0;

    unsigned int piFlags [MAX_NUM_SEARCH_COLUMNS];

    // Loop through all possible search fields
    for (i = 0; i < MAX_NUM_SEARCH_COLUMNS; i ++) {

        // Privilege filter
        if (g_AdvancedSearchFields[i].prvMinPriv > m_iPrivilege) continue;

        // Check checkbox submission
        if (m_pHttpRequest->GetForm (g_AdvancedSearchFields[i].pszInputCheckBox) == NULL) continue;

        pHttpForm1 = pHttpForm2 = NULL;

        // Get input1 - provided by all types
        pHttpForm1 = m_pHttpRequest->GetForm (g_AdvancedSearchFields[i].pszInput1);
        if (pHttpForm1 == NULL) continue;

        // Get input1 - provided by most types
        if (g_AdvancedSearchFields[i].pszInput2 != NULL) {
            pHttpForm2 = m_pHttpRequest->GetForm (g_AdvancedSearchFields[i].pszInput2);
            if (pHttpForm2 == NULL) continue;
        }

        // Assign common fields
        piSearchCol [iNumSearchColumns] = g_AdvancedSearchFields[i].iSystemEmpireDataColumn;
        piFlags [iNumSearchColumns] = 0;

        pszFormName [iNumSearchColumns] = g_AdvancedSearchFields[i].pszInputCheckBox;
        pszColName1 [iNumSearchColumns] = g_AdvancedSearchFields[i].pszInput1;
        pszColName2 [iNumSearchColumns] = g_AdvancedSearchFields[i].pszInput1;

        // Select on type
        switch (g_AdvancedSearchFields[i].sftType) {

        case SEARCHFIELD_INTEGER:

            pvSearchColData1 [iNumSearchColumns] = pHttpForm1->GetIntValue();
            pvSearchColData2 [iNumSearchColumns] = pHttpForm2->GetIntValue();
            break;

        case SEARCHFIELD_INTEGER64:

            pvSearchColData1 [iNumSearchColumns] = pHttpForm1->GetInt64Value();
            pvSearchColData2 [iNumSearchColumns] = pHttpForm2->GetInt64Value();
            break;

        case SEARCHFIELD_FLOAT:

            pvSearchColData1 [iNumSearchColumns] = pHttpForm1->GetFloatValue();
            pvSearchColData2 [iNumSearchColumns] = pHttpForm2->GetFloatValue();
            break;

        case SEARCHFIELD_STRING:

            pvSearchColData1 [iNumSearchColumns] = pHttpForm1->GetValue();
            piFlags [iNumSearchColumns] = pHttpForm2->GetIntValue();
            break;

        case SEARCHFIELD_PRIVILEGE:

            pvSearchColData1 [iNumSearchColumns] = pHttpForm1->GetIntValue();
            pvSearchColData2 [iNumSearchColumns] = pHttpForm1->GetIntValue();
            break;

        case SEARCHFIELD_DATE:

            UTCTime tTime;

            if (!ParseDateField (g_AdvancedSearchFields[i].pszInput1, &tTime)) continue;
            pvSearchColData1 [iNumSearchColumns] = tTime;

            if (!ParseDateField (g_AdvancedSearchFields[i].pszInput2, &tTime)) continue;
            pvSearchColData2 [iNumSearchColumns] = tTime;

            break;

        default:

            Assert (false);
            break;
        }

        // Increment legit submission counter
        iNumSearchColumns ++;
    }

    Assert (iNumSearchColumns <= MAX_NUM_SEARCH_COLUMNS);
    
    if ((pHttpForm = m_pHttpRequest->GetForm ("Skip")) != NULL) {
        iSkip = pHttpForm->GetIntValue();
    } else {
        return ERROR_INVALID_ARGUMENT;
    }
    
    if (iNumSearchColumns == 0 ||
        (pHttpForm = m_pHttpRequest->GetForm ("MaxNumHits")) == NULL ||
        (iMaxNumHits = pHttpForm->GetIntValue()) < 1
        ) {
        
        return ERROR_INVALID_QUERY;
    }
    
    // Get startkey
    pHttpForm = m_pHttpRequest->GetForm ("StartKey");
    if (pHttpForm != NULL) {
        iStartKey = pHttpForm->GetIntValue();
    }
    
    iErrCode = g_pGameEngine->PerformMultipleSearch (
        iStartKey,
        iSkip,
        iMaxNumHits,
        iNumSearchColumns,
        piSearchCol,
        piFlags,
        pvSearchColData1,
        pvSearchColData2,
        ppiSearchEmpireKey,
        piNumSearchEmpires,
        piLastKey
        );
    
    // Out
    *piNumSearchColumns = iNumSearchColumns;
    *piMaxNumHits = iMaxNumHits;
    
    return iErrCode;
}

void HtmlRenderer::RenderSearchResults (unsigned int* piSearchColName, 
                                        Variant* pvSearchColData1,
                                        Variant* pvSearchColData2,
                                        const char** pszFormName,
                                        const char** pszColName1,
                                        const char** pszColName2,
                                        
                                        int iNumSearchColumns,
                                        
                                        int* piSearchEmpireKey,
                                        int iNumSearchEmpires,
                                        int iLastKey,
                                        int iMaxNumHits
                                        ) {
    int i, iErrCode;
    
    OutputText ("<p>");
    
    if (iLastKey != NO_KEY) {
        OutputText ("More than ");
    }
    OutputText ("<strong>");
    m_pHttpResponse->WriteText (iNumSearchEmpires);
    OutputText ("</strong> empire");
    
    if (iNumSearchEmpires == 1) {
        OutputText (" was");
    } else {
        OutputText ("s were");
    }
    
    OutputText (" found:<p><table><tr>");
    OutputText ("<th align=\"center\" bgcolor=\"");
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    OutputText ("\"><strong>Empire Name</strong></th>");
    OutputText ("<th align=\"center\" bgcolor=\"");
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    OutputText ("\"><strong>Icon</strong></th>");
    
    for (i = 0; i < iNumSearchColumns; i ++) {
        
        switch (piSearchColName[i]) {
            
        case NO_KEY:
            
            OutputText ("<th align=\"center\" bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\">");
            OutputText ("<strong>Empire Key</strong></td>");
            
            break;
            
        case SystemEmpireData::Name:
            break;
            
        default:
            
            OutputText ("<th align=\"center\" bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\"><strong>"); 
            m_pHttpResponse->WriteText (SYSTEM_EMPIRE_DATA_COLUMN_NAMES [piSearchColName[i]]);
            OutputText ("</strong></td>");
            
            break;
        }
    }
    
    OutputText ("</tr>");
    
    int j;
    Variant vName, vData, vAlien;
    char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];

    NotifyProfileLink();
    
    for (i = 0; i < iNumSearchEmpires; i ++) {
        
        if (g_pGameEngine->GetEmpireName (piSearchEmpireKey[i], &vName) == OK &&
            g_pGameEngine->GetEmpireProperty (piSearchEmpireKey[i], SystemEmpireData::AlienKey, &vAlien) == OK) {
            
            OutputText ("<tr><td align=\"center\"><strong>");
            m_pHttpResponse->WriteText (vName.GetCharPtr());
            OutputText ("</strong></td><td align=\"center\">");
            
            sprintf (pszProfile, "View the profile of %s", vName.GetCharPtr());
            
            WriteProfileAlienString (
                vAlien.GetInteger(), 
                piSearchEmpireKey[i],
                vName.GetCharPtr(),
                0, 
                "ViewProfile",
                pszProfile,
                true,
                true
                );
            
            OutputText ("</td>");
            
            iErrCode = OK;
            
            for (j = 0; j < iNumSearchColumns && iErrCode == OK; j ++) {
                
                if (piSearchColName[j] != SystemEmpireData::Name) {
                    
                    OutputText ("<td align=\"center\">");
                    
                    if (piSearchColName[j] == NO_KEY) {
                        m_pHttpResponse->WriteText (piSearchEmpireKey[i]);
                    } else {
                        
                        iErrCode = g_pGameEngine->GetEmpireDataColumn (
                            piSearchEmpireKey[i], 
                            piSearchColName[j], 
                            &vData
                            );
                        
                        if (iErrCode == OK) {
                            
                            switch (piSearchColName[j]) {
                                
                            case SystemEmpireData::Privilege:
                                
                                m_pHttpResponse->WriteText (PRIVILEGE_STRING[vData.GetInteger()]);
                                break;
                                
                            case SystemEmpireData::WebPage:

                                RenderUnsafeHyperText (vData.GetCharPtr(), vData.GetCharPtr());
                                break;
                                
                            default:
                                
                                m_pHttpResponse->WriteText (vData);
                                break;
                            }
                        }
                    }
                    OutputText ("</td>");
                }
            }
            
            OutputText ("</tr>");
        }
    }
    
    OutputText ("</table>");
    
    // Lay down query information for submission if more empires were found
    if (iLastKey != NO_KEY) {
        
        OutputText ("<p>Search for the next ");
        
        if (iMaxNumHits != 1) {
            m_pHttpResponse->WriteText (iMaxNumHits);
            OutputText (" empires");
        } else {
            OutputText ("empire");
        }
        
        OutputText (": "); WriteButton (BID_SEARCH);
        
        OutputText ("<input type=\"hidden\" name=\"Skip\" value=\"0\">");
        OutputText ("<input type=\"hidden\" name=\"MaxNumHits\" value=\"");
        m_pHttpResponse->WriteText (iMaxNumHits);
        OutputText ("\">");
        OutputText ("<input type=\"hidden\" name=\"StartKey\" value=\"");
        m_pHttpResponse->WriteText (iLastKey);
        OutputText ("\">");
        
        for (i = 0; i < iNumSearchColumns; i ++) {
            
            OutputText ("<input type=\"hidden\" name=\"");
            m_pHttpResponse->WriteText (pszFormName[i]);
            OutputText ("\" value=\"1\">");
            OutputText ("<input type=\"hidden\" name=\"");
            m_pHttpResponse->WriteText (pszColName1[i]);
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (pvSearchColData1[i]);
            OutputText ("\">");
            
            if (pszColName2[i] != NULL) {
                OutputText ("<input type=\"hidden\" name=\"");
                m_pHttpResponse->WriteText (pszColName2[i]);
                OutputText ("\" value=\"");
                m_pHttpResponse->WriteText (pvSearchColData2[i]);
                OutputText ("\">");
            }
        }
    }
    
    OutputText ("<p>");
    WriteButton (BID_CANCEL);
}