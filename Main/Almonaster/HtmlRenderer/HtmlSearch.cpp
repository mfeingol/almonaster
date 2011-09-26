//
// Almonaster.dll:  a component of Almonaster
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

#include "HtmlRenderer.h"
#include "SqlDatabase.h"

const SearchField g_AdvancedSearchFields[] = {

    { true, NULL, "EmpName", "TEmpName", "SEmpName", SystemEmpireData::Name, SEARCHFIELD_STRING, GUEST },
    { false, "Empire Key", "EmpKey", "LEmpKey", "REmpKey", NULL, SEARCHFIELD_INTEGER, GUEST },
    { false, NULL, "RealName", "TRealName", "SRealName", SystemEmpireData::RealName, SEARCHFIELD_STRING, GUEST },
    { false, NULL, "Age", "TAge", "SAge", SystemEmpireData::Age, SEARCHFIELD_AGE, GUEST },
    { false, NULL, "Gender", "TGender", NULL, SystemEmpireData::Gender, SEARCHFIELD_GENDER, GUEST },
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

int HtmlRenderer::RenderSearchForms (bool fAdvanced) {

    Assert(MAX_NUM_SEARCH_COLUMNS == countof (g_AdvancedSearchFields));
    Assert(countof(SystemEmpireData::ColumnNames) == SystemEmpireData::NumColumns); 
    
    unsigned int iNumEmpires;
    int iErrCode = GetNumEmpiresOnServer(&iNumEmpires);
    RETURN_ON_ERROR(iErrCode);
    
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
            "<input type=\"hidden\" name=\"MaxNumHits\" value=\"20\">"\
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

    return iErrCode;
}


//
// Assumes we're already inside a table
//

void HtmlRenderer::RenderSearchField (const SearchField& sfField, bool fAdvanced) {

    // Privilege check
    if (sfField.prvMinPriv > m_iPrivilege) return;

    const char* pszName = sfField.pszName;
    if (pszName == NULL)
    {
        Assert(sfField.pszSystemEmpireDataColumn != NULL);
        pszName = sfField.pszSystemEmpireDataColumn;
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
        OutputText ("\">Begins with search</option><option value=\"");
        m_pHttpResponse->WriteText (SEARCH_ENDS_WITH);
        OutputText ("\">Ends with search</option>"\
            "</select>");
        
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

    case SEARCHFIELD_GENDER:
        
        OutputText ("<td><select name=\"");
        m_pHttpResponse->WriteText (sfField.pszInput1);
        OutputText ("\">");

        for (i = 0; i < EMPIRE_NUM_GENDERS; i ++) {

            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (EMPIRE_GENDER[i]);
            OutputText ("\">");
            m_pHttpResponse->WriteText (EMPIRE_GENDER_STRING [EMPIRE_GENDER[i]]);
            OutputText ("</option>");
        }

        OutputText ("</select>");
        break;

    case SEARCHFIELD_AGE:
        
        OutputText ("<td><select name=\"");
        m_pHttpResponse->WriteText (sfField.pszInput1);
        OutputText (
            "\">"\
            "<option value=\""
            );

        m_pHttpResponse->WriteText (EMPIRE_AGE_UNKNOWN);
        OutputText (
            "\">N/A</option>"
            );

        for (i = EMPIRE_AGE_MINIMUM; i <= EMPIRE_AGE_MAXIMUM; i ++) {

            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (i);
            OutputText ("\">");
            m_pHttpResponse->WriteText (i);
            OutputText ("</option>");
        }

        OutputText ("</select> to <select name=\"");
        m_pHttpResponse->WriteText (sfField.pszInput2);
        OutputText (
            "\">"\
            "<option value=\""
            );

        m_pHttpResponse->WriteText (EMPIRE_AGE_UNKNOWN);
        OutputText (
            "\">N/A</option>"
            );

        for (i = EMPIRE_AGE_MINIMUM; i <= EMPIRE_AGE_MAXIMUM; i ++) {

            OutputText ("<option value=\"");
            m_pHttpResponse->WriteText (i);
            OutputText ("\">");
            m_pHttpResponse->WriteText (i);
            OutputText ("</option>");
        }

        OutputText ("</select>");
        break;
        
    default:
        Assert(false);
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

int HtmlRenderer::HandleSearchSubmission (RangeSearchDefinition& sd,

                                          const char** pszFormName,
                                          const char** pszColName1,
                                          const char** pszColName2,

                                          unsigned int** ppiSearchEmpireKey,
                                          unsigned int* piNumSearchEmpires,
                                          unsigned int* piLastKey
                                          ) {

    IHttpForm* pHttpForm, * pHttpForm1, * pHttpForm2;

    int iErrCode = OK;
    
    unsigned int i, iMaxNumHits = 0, iNumSearchColumns = 0, iSkip = 0, iStartKey = NO_KEY;

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
        sd.pscColumns[iNumSearchColumns].pszColumn = g_AdvancedSearchFields[i].pszSystemEmpireDataColumn;
        sd.pscColumns[iNumSearchColumns].iFlags = 0;

        pszFormName [iNumSearchColumns] = g_AdvancedSearchFields[i].pszInputCheckBox;
        pszColName1 [iNumSearchColumns] = g_AdvancedSearchFields[i].pszInput1;
        pszColName2 [iNumSearchColumns] = g_AdvancedSearchFields[i].pszInput2;

        // Select on type
        switch (g_AdvancedSearchFields[i].sftType) {

        case SEARCHFIELD_INTEGER:

            sd.pscColumns[iNumSearchColumns].vData = pHttpForm1->GetIntValue();
            sd.pscColumns[iNumSearchColumns].vData2 = pHttpForm2->GetIntValue();
            break;

        case SEARCHFIELD_INTEGER64:

            sd.pscColumns[iNumSearchColumns].vData = pHttpForm1->GetInt64Value();
            sd.pscColumns[iNumSearchColumns].vData2 = pHttpForm2->GetInt64Value();
            break;

        case SEARCHFIELD_FLOAT:

            sd.pscColumns[iNumSearchColumns].vData = pHttpForm1->GetFloatValue();
            sd.pscColumns[iNumSearchColumns].vData2 = pHttpForm2->GetFloatValue();
            break;

        case SEARCHFIELD_STRING:

            sd.pscColumns[iNumSearchColumns].vData = pHttpForm1->GetValue();
            sd.pscColumns[iNumSearchColumns].vData2 = (const char*)NULL;
            sd.pscColumns[iNumSearchColumns].iFlags = pHttpForm2->GetIntValue();
            break;

        case SEARCHFIELD_PRIVILEGE:

            sd.pscColumns[iNumSearchColumns].vData = pHttpForm1->GetIntValue();
            sd.pscColumns[iNumSearchColumns].vData2 = pHttpForm1->GetIntValue();
            break;

        case SEARCHFIELD_GENDER:

            sd.pscColumns[iNumSearchColumns].vData = pHttpForm1->GetIntValue();
            sd.pscColumns[iNumSearchColumns].vData2 = pHttpForm1->GetIntValue();

            if (sd.pscColumns[iNumSearchColumns].vData.GetInteger() < EMPIRE_GENDER_UNKNOWN ||
                sd.pscColumns[iNumSearchColumns].vData.GetInteger() > EMPIRE_GENDER_FEMALE) {
                continue;
            }
            break;

        case SEARCHFIELD_AGE:

            sd.pscColumns[iNumSearchColumns].vData = pHttpForm1->GetIntValue();
            sd.pscColumns[iNumSearchColumns].vData2 = pHttpForm2->GetIntValue();

            if (sd.pscColumns[iNumSearchColumns].vData.GetInteger() < EMPIRE_AGE_MINIMUM ||
                sd.pscColumns[iNumSearchColumns].vData2.GetInteger() > EMPIRE_AGE_MAXIMUM ||

                (sd.pscColumns[iNumSearchColumns].vData.GetInteger() == EMPIRE_AGE_UNKNOWN &&
                 sd.pscColumns[iNumSearchColumns].vData2.GetInteger() != EMPIRE_AGE_UNKNOWN) ||

                (sd.pscColumns[iNumSearchColumns].vData.GetInteger() != EMPIRE_AGE_UNKNOWN &&
                 sd.pscColumns[iNumSearchColumns].vData2.GetInteger() == EMPIRE_AGE_UNKNOWN)
                ) {
                continue;
            }
            break;

        case SEARCHFIELD_DATE:

            UTCTime tTime;

            if (!ParseDateField (g_AdvancedSearchFields[i].pszInput1, &tTime)) continue;
            sd.pscColumns[iNumSearchColumns].vData = tTime;

            if (!ParseDateField (g_AdvancedSearchFields[i].pszInput2, &tTime)) continue;
            sd.pscColumns[iNumSearchColumns].vData2 = tTime;

            break;

        default:

            Assert(false);
            break;
        }

        // Increment legit submission counter
        iNumSearchColumns ++;
    }

    Assert(iNumSearchColumns <= MAX_NUM_SEARCH_COLUMNS);
    
    if ((pHttpForm = m_pHttpRequest->GetForm ("Skip")) == NULL) {
        return ERROR_INVALID_ARGUMENT;
    }
    iSkip = pHttpForm->GetUIntValue();
    
    if (iNumSearchColumns == 0 ||
        (pHttpForm = m_pHttpRequest->GetForm ("MaxNumHits")) == NULL ||
        (iMaxNumHits = pHttpForm->GetUIntValue()) < 1
        ) {
        return ERROR_INVALID_QUERY;
    }
    
    // Get startkey
    pHttpForm = m_pHttpRequest->GetForm ("StartKey");
    if (pHttpForm != NULL) {
        iStartKey = pHttpForm->GetUIntValue();
    }

    sd.iMaxNumHits = iMaxNumHits;
    sd.iSkipHits = iSkip;
    sd.iStartKey = iStartKey;
    sd.iNumColumns = iNumSearchColumns;
    
    iErrCode = PerformMultipleSearch (
        sd,
        ppiSearchEmpireKey,
        piNumSearchEmpires,
        piLastKey
        );

    RETURN_ON_ERROR(iErrCode);
    
    return iErrCode;
}

int HtmlRenderer::RenderSearchResults(RangeSearchDefinition& sd,
                                      const char** pszFormName,
                                      const char** pszColName1,
                                      const char** pszColName2,
                                      unsigned int* piSearchEmpireKey,
                                      unsigned int iNumSearchEmpires,
                                      unsigned int iLastKey)
{
    int iErrCode = OK;
    unsigned int i, j, iNumSearchColumns = sd.iNumColumns, iNumEmpiresFoundSoFar = iNumSearchEmpires;

    IHttpForm* pHttpForm = m_pHttpRequest->GetForm ("EmpiresFoundSoFar");
    if (pHttpForm != NULL) {
        iNumEmpiresFoundSoFar += pHttpForm->GetUIntValue();
    }

    OutputText ("<p>");

    if (iLastKey != NO_KEY) {
        OutputText ("<input type=\"hidden\" name=\"EmpiresFoundSoFar\" value=\"");
        m_pHttpResponse->WriteText (iNumEmpiresFoundSoFar);
        OutputText ("\">More than ");
    }

    OutputText ("<strong>");
    m_pHttpResponse->WriteText (iNumEmpiresFoundSoFar);
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
        
        RangeSearchColumnDefinition& sc = sd.pscColumns[i];
        if (sc.pszColumn == NULL)
        {            
            OutputText ("<th align=\"center\" bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\">");
            OutputText ("<strong>Empire Key</strong></td>");
        }
        else if (strcmp(sc.pszColumn, SystemEmpireData::Name) == 0)
        {
            // Do nothing
        }
        else
        {
            OutputText ("<th align=\"center\" bgcolor=\"");
            m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
            OutputText ("\"><strong>"); 
            m_pHttpResponse->WriteText(sc.pszColumn);
            OutputText ("</strong></td>");
        }
    }
    
    OutputText ("</tr>");

    Variant vName, vAlien;
    char pszProfile [128 + MAX_EMPIRE_NAME_LENGTH];

    NotifyProfileLink();
    
    for (i = 0; i < iNumSearchEmpires; i ++)
    {
        iErrCode = GetEmpireName (piSearchEmpireKey[i], &vName);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetEmpireProperty (piSearchEmpireKey[i], SystemEmpireData::AlienKey, &vAlien);
        RETURN_ON_ERROR(iErrCode);
            
        OutputText ("<tr><td align=\"center\"><strong>");
        m_pHttpResponse->WriteText (vName.GetCharPtr());
        OutputText ("</strong></td><td align=\"center\">");
            
        sprintf(pszProfile, "View the profile of %s", vName.GetCharPtr());
            
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
            
        for (j = 0; j < iNumSearchColumns; j ++) {

            Variant vData;
                
            RangeSearchColumnDefinition& sc = sd.pscColumns[j];
            const char* pszCol = sc.pszColumn;

            if (pszCol != SystemEmpireData::Name) {
                    
                OutputText ("<td align=\"center\">");
                    
                if (pszCol == NULL) {
                    m_pHttpResponse->WriteText (piSearchEmpireKey[i]);
                } else {
                        
                    iErrCode = GetEmpireDataColumn (piSearchEmpireKey[i], pszCol, &vData);
                    RETURN_ON_ERROR(iErrCode);
 
                    if (strcmp(pszCol, SystemEmpireData::Privilege) == 0)
                    {
                        m_pHttpResponse->WriteText (PRIVILEGE_STRING[vData.GetInteger()]);
                    }
                    else if (strcmp(pszCol, SystemEmpireData::WebPage) == 0)
                    {
                        RenderUnsafeHyperText (vData.GetCharPtr(), vData.GetCharPtr());
                    }
                    else if (strcmp(pszCol, SystemEmpireData::Gender) == 0)
                    {
                        Assert(vData.GetInteger() >= EMPIRE_GENDER_UNKNOWN && 
                                vData.GetInteger() <= EMPIRE_GENDER_FEMALE);
                        m_pHttpResponse->WriteText(EMPIRE_GENDER_STRING[vData.GetInteger()]);
                    }
                    else if (strcmp(pszCol, SystemEmpireData::LastLoginTime) == 0 ||
                                strcmp(pszCol, SystemEmpireData::CreationTime) == 0 ||
                                strcmp(pszCol, SystemEmpireData::LastBridierActivity) == 0)
                    {
                        m_pHttpResponse->WriteDate(vData.GetInteger64());
                    }
                    else
                    {
                        m_pHttpResponse->WriteText (vData);
                    }
                }
                OutputText ("</td>");
            }
        }
        OutputText ("</tr>");
    }
    
    OutputText ("</table>");
    
    // Lay down query information for submission if more empires were found
    if (iLastKey != NO_KEY) {
        
        OutputText ("<p>Search for the next ");
        
        if (sd.iMaxNumHits != 1) {
            m_pHttpResponse->WriteText (sd.iMaxNumHits);
            OutputText (" empires");
        } else {
            OutputText ("empire");
        }
        
        OutputText (": "); WriteButton (BID_SEARCH);
        
        OutputText ("<input type=\"hidden\" name=\"Skip\" value=\"0\">");
        OutputText ("<input type=\"hidden\" name=\"MaxNumHits\" value=\"");
        m_pHttpResponse->WriteText (sd.iMaxNumHits);
        OutputText ("\">");
        OutputText ("<input type=\"hidden\" name=\"StartKey\" value=\"");
        m_pHttpResponse->WriteText (iLastKey);
        OutputText ("\">");
        
        for (i = 0; i < iNumSearchColumns; i ++) {

            RangeSearchColumnDefinition& sc = sd.pscColumns[i];
            
            OutputText ("<input type=\"hidden\" name=\"");
            m_pHttpResponse->WriteText (pszFormName[i]);
            OutputText ("\" value=\"1\">");

            RenderHiddenSearchVariant(sc.pszColumn, pszColName1[i], sc.vData);
            
            if (pszColName2[i] != NULL) {
                RenderHiddenSearchVariant(sc.pszColumn, pszColName2[i], sc.vData2);
            }
        }
    }
    
    OutputText ("<p>");
    WriteButton (BID_CANCEL);

    return iErrCode;
}

void HtmlRenderer::RenderHiddenSearchVariant (const char* pszColumn, const char* pszColName, const Variant& vData) {

    if (strcmp(pszColumn, SystemEmpireData::LastLoginTime) == 0 ||
        strcmp(pszColumn, SystemEmpireData::CreationTime) == 0 ||
        strcmp(pszColumn, SystemEmpireData::LastBridierActivity) == 0)
    {
        int iSec, iMin, iHour, iDay, iMonth, iYear;
        DayOfWeek dayOfWeek;

        Time::GetDate (vData.GetInteger64(), &iSec, &iMin, &iHour, &dayOfWeek, &iDay, &iMonth, &iYear);

        // Year
        OutputText ("<input type=\"hidden\" name=\"");
        m_pHttpResponse->WriteText (pszColName);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (iYear);
        OutputText ("\">");

        // Month
        OutputText ("<input type=\"hidden\" name=\"");
        m_pHttpResponse->WriteText (pszColName);
        OutputText ("Month\" value=\"");
        m_pHttpResponse->WriteText (iMonth);
        OutputText ("\">");

        // Day
        OutputText ("<input type=\"hidden\" name=\"");
        m_pHttpResponse->WriteText (pszColName);
        OutputText ("Day\" value=\"");
        m_pHttpResponse->WriteText (iDay);
        OutputText ("\">");

    }
    else
    {
        OutputText ("<input type=\"hidden\" name=\"");
        m_pHttpResponse->WriteText (pszColName);
        OutputText ("\" value=\"");
        m_pHttpResponse->WriteText (vData);
        OutputText ("\">");
    }
}