//
// Admin.dll
// Copyright (c) 1999 Max Attar Feingold (maf6@cornell.edu)
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

#include "Admin.h"
#include "Osal/Algorithm.h"

#include <stdio.h>


#define OutputText(string) pHttpResponse->WriteText (string, sizeof(string) - 1);

int Admin::RenderAdminPage (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, 
                            const String& strMessage) {

    int iErrCode = OK;

    IPageSourceEnumerator* pPageSourceEnumerator = NULL;
    IPageSourceControl** ppPageSource = NULL;
    unsigned int i, j, iNumPageSources = 0;

    // HTML header, open form
    OutputText ("<html><body><center><h1>");
    
    pHttpResponse->WriteText (m_pHttpServer->GetServerName());
    OutputText (
        " Administration</h1>"\
        "<form method=\"post\"><input type=\"submit\" name=\"Refresh\" value=\"Refresh\"><p>"
        );

    // Print message
    if (!strMessage.IsBlank()) {
        pHttpResponse->WriteText (strMessage.GetCharPtr());
    }

    // Enumerate page sources, offer restart, shutdown options
    OutputText ("<h2>Restart and Shutdown</h2>");

    pPageSourceEnumerator = m_pHttpServer->EnumeratePageSources();
    if (pPageSourceEnumerator == NULL || 
        (iNumPageSources = pPageSourceEnumerator->GetNumPageSources()) == 0) {
        OutputText ("<p><strong>Could not enumerate pagesources</strong>");

    } else {

        ppPageSource = pPageSourceEnumerator->GetPageSourceControls();

        OutputText ("<table width=\"80%\">");

        // Restart'n shutdown
        OutputText ("<tr><td align=\"center\"><input type=\"submit\" name=\"RestartPageSource\" "\
            "value=\"Restart the PageSource\"> <select name=\"RestartName\">");

        for (i = 0; i < iNumPageSources; i ++) {

            OutputText ("<option value=\"");
            pHttpResponse->WriteText (ppPageSource[i]->GetName());
            OutputText ("\">");
            pHttpResponse->WriteText (ppPageSource[i]->GetName());
            OutputText ("</option>");
        }

        OutputText ("</select></td><td align=\"center\"><input type=\"submit\" "\
            "name=\"ShutdownPageSource\" value=\"Shutdown the PageSource\"> <select name=\"ShutdownName\">"
            );

        for (i = 0; i < iNumPageSources; i ++) {

            OutputText ("<option value=\"");
            pHttpResponse->WriteText (ppPageSource[i]->GetName());
            OutputText ("\">");
            pHttpResponse->WriteText (ppPageSource[i]->GetName());
            OutputText ("</option>");
        }
        OutputText ("</select></td></tr>");
    }

    // Offer server restart, shutdown options
    OutputText (
        "<tr><td align=\"center\"><input type=\"submit\" name=\"RestartServer\" value=\"Restart the Server\">"\
        "</td><td align=\"center\"><input type=\"submit\" name=\"ShutdownServer\" value=\"Shutdown the Server\">"\
        "</td></tr><tr><td>&nbsp;</td></tr>"
        );

    // Report'n log
    if (iNumPageSources > 0) {

        OutputText ("<tr><td align=\"center\"><input type=\"submit\" name=\"PageSourceReport\" "\
            "value=\"View PageSource Report\"> <select name=\"ReportName\">");

        for (i = 0; i < iNumPageSources; i ++) {

            OutputText ("<option value=\"");
            pHttpResponse->WriteText (ppPageSource[i]->GetName());
            OutputText ("\">");
            pHttpResponse->WriteText (ppPageSource[i]->GetName());
            OutputText ("</option>");
        }
        OutputText ("</select></td><td align=\"center\"><input type=\"submit\" "\
            "name=\"PageSourceLog\" value=\"View PageSource Log\"> <select name=\"LogName\">"
            );

        for (i = 0; i < iNumPageSources; i ++) {

            OutputText ("<option value=\"");
            pHttpResponse->WriteText (ppPageSource[i]->GetName());
            OutputText ("\">");
            pHttpResponse->WriteText (ppPageSource[i]->GetName());
            OutputText ("</option>");
        }
        OutputText ("</select></td></tr>");
    }

    // Offer server log, report viewing
    pHttpResponse->WriteText (
        "<tr><td align=\"center\"><input type=\"submit\" name=\"ServerReport\" value=\"View Server Report\">"\
        "</td></tr>"
        );

    // Release cached files
    OutputText ("<tr><td colspan=\"2\" align=\"center\">");

    bool bCacheActive = m_pFileCache->IsActive();
    if (!bCacheActive) {
        OutputText ("The file cache is not active</td></tr>");
    } else {
        
        unsigned int iNumFiles = m_pFileCache->GetNumFiles();
        size_t stSize = m_pFileCache->GetSize();
        
        OutputText ("The file cache contains ");
        pHttpResponse->WriteText (iNumFiles);
        OutputText (" file");

        if (iNumFiles != 1) {
            OutputText ("s");
        }
        
        OutputText (", totalling ");
        pHttpResponse->WriteText ((uint64) stSize / 1024);
        OutputText (" KB</td></tr>");

        pHttpResponse->WriteText (
            "<tr><td align=\"center\"><input type=\"submit\" name=\"FlushAFile\" value=\"Release Cached File\">"\
            "<input type=\"text\" name=\"FlushFileName\" size=\"30\">"\
            "</td><td><input type=\"submit\" name=\"FlushAllFiles\" value=\"Release All Cached Files\"></tr>"
            );
    }

    HttpServerStatistics stats;
    iErrCode = m_pHttpServer->GetStatistics (&stats);

    if (iErrCode == OK) {

        UTCTime tTime;
        Time::GetTime (&tTime);

        String strTime;

        iErrCode = ConvertTime (Time::GetSecondDifference (tTime, stats.StartupTime), &strTime);
        if (iErrCode == OK) {
            
            pHttpResponse->WriteText (
                "</table><h2>Daily Server Statistics</h2><table width=\"80%\">"\
                "The server has been running for ");
            
            pHttpResponse->WriteText (strTime);
            
            
            pHttpResponse->WriteText (
                "</table>"\
                "<h3>Requests</h3><table width=\"80%\" border=\"2\"><tr>"\
                "<th colspan=\"2\">Total Requests</th>"\
                "<th colspan=\"2\">Queued</th>"\
                "<th colspan=\"2\">Max Queued</th>"\
                "<th>Get</th>"\
                "<th>Post</th>"\
                "<th>Put</th>"\
                "<th>Head</th>"\
                "<th>Trace</th>"\
                "<th>Unknown</th>"\
                "</tr><tr><td align=\"center\" colspan=\"2\">"
                );
            
            pHttpResponse->WriteText (stats.NumRequests);
            OutputText ("</td><td align=\"center\" colspan=\"2\">");
            pHttpResponse->WriteText (stats.NumQueuedRequests);
            OutputText ("</td><td align=\"center\" colspan=\"2\">");
            pHttpResponse->WriteText (stats.MaxNumQueuedRequests);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumGets);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumPosts);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumPuts);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumHeads);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumTraces);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumUndefinedMethods);
            OutputText ("</td></tr></table><h3>Responses</h3><table width=\"80%\" border=\"2\"><tr>");
            
            OutputText (
                "<th>200</th>"\
                "<th>301</th>"\
                "<th>304</th>"\
                "<th>400</th>"\
                "<th>401</th>"\
                "<th>403</th>"\
                "<th>404</th>"\
                "<th>409</th>"\
                "<th>500</th>"\
                "<th>501</th>"\
                "<th>503</th>"\
                "<th>505</th>"\
                "</tr><tr><td align=\"center\">"
                );
            
            pHttpResponse->WriteText (stats.Num200s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num301s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num304s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num400s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num401s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num403s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num404s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num409s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num500s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num501s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num503s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.Num505s);
            OutputText (
                "</td></tr><tr></table><h3>Access Denied</h3><table width=\"80%\" border=\"2\">"\

                "<th>IP Address blocked</th>"\
                "<th>User Agent blocked</th>"\
                "<th>GET referer rejected</th>"\
                "</tr><tr><td align=\"center\">"
                );

            pHttpResponse->WriteText (stats.NumIPAddress403s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumUserAgent403s);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumGETFilter403s);

            OutputText (
                "</td></tr><tr></table><h3>Forms</h3><table width=\"80%\" border=\"2\">"\
                "<th>Total Forms Submitted</th>"\
                "<th>Simple Forms</th>"\
                "<th>File Forms</th>"\
                "<th>Uploaded Files</th>"\
                "</tr><tr><td align=\"center\">"
                );
            
            pHttpResponse->WriteText (stats.NumForms);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumSimpleForms);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumFileForms);
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText ((uint64) (stats.NumBytesInFileForms / 1024));
            OutputText (" KB</td></tr><tr></table><h3>Data Flow</h3><table width=\"80%\" border=\"2\">");
            
            OutputText (
                "<th>Bytes Sent</th>"\
                "<th>Bytes Received</th>"\
                "<th>Average parse time</th>"\
                "<th>Average response time</th>"\
                "</tr><tr><td align=\"center\">"
                );
            
            pHttpResponse->WriteText (stats.NumBytesSent);
            OutputText (" (");
            pHttpResponse->WriteText ((uint64) (stats.NumBytesSent / 1024));
            OutputText (" KB)");
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.NumBytesReceived);
            OutputText (" (");
            pHttpResponse->WriteText ((uint64) (stats.NumBytesReceived / 1024));
            OutputText (" KB)");
            OutputText ("</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.AverageRequestParseTime);
            OutputText (" ms</td><td align=\"center\">");
            pHttpResponse->WriteText (stats.AverageResponseTime);
            OutputText (" ms</td></tr>");
        }
    }

    // Display config file values, offer update button
    OutputText ("</table><h2>Server Parameters</h2><table width=\"80%\">");

    char* pszLhs, * pszRhs;
    unsigned int iNumParameters = m_pServerConfig->GetNumParameters();

    for (i = 0; i < iNumParameters; i ++) {
        
        if (m_pServerConfig->GetParameter (i, &pszLhs, &pszRhs) == OK) {

            OutputText ("<tr><td width=\"20%\">");
            pHttpResponse->WriteText (pszLhs);

            OutputText ("</td><td><input type=\"text\" name=\"");
            pHttpResponse->WriteText (pszLhs);
            OutputText ("\" value=\"");
            pHttpResponse->WriteText (pszRhs);
            OutputText ("\" size=\"60\"></td></tr>");
        }
    }

    OutputText ("</table><p><input type=\"submit\" name=\"UpdateServerParams\" "\
        "value=\"Update Server Parameters\">");
    
    // Display pagesource config file values
    OutputText ("<p><h2>PageSource Parameters</h2>");
    
    for (i = 0; i < iNumPageSources; i ++) {
        
        OutputText ("<p><h3>");
        pHttpResponse->WriteText (ppPageSource[i]->GetName());
        OutputText (" Parameters</h3><table width=\"80%\">");

        IConfigFile* pConfig = ppPageSource[i]->GetConfigFile();
        iNumParameters = pConfig->GetNumParameters();
        
        for (j = 0; j < iNumParameters; j ++) {
            
            if (pConfig->GetParameter (j, &pszLhs, &pszRhs) == OK) {
                
                OutputText ("<tr><td width=\"20%\">");
                pHttpResponse->WriteText (pszLhs);
                
                OutputText ("</td><td><input type=\"text\" name=\"");
                pHttpResponse->WriteText (ppPageSource[i]->GetName());
                OutputText ("_");
                pHttpResponse->WriteText (pszLhs);
                OutputText ("\" value=\"");
                pHttpResponse->WriteText (pszRhs);
                OutputText ("\" size=\"60\"></td></tr>");
            }
        }
        
        OutputText ("</table><p><input type=\"submit\" name=\"Update");
        pHttpResponse->WriteText (ppPageSource[i]->GetName());
        OutputText ("PageSourceParams\" value=\"Update ");
        pHttpResponse->WriteText (ppPageSource[i]->GetName());
        OutputText (" Parameters\">");

        SafeRelease(pConfig);
    }

    // Close form, page
    OutputText ("</form></center></body></html>");

    // Clean up
    if (pPageSourceEnumerator != NULL) {
        pPageSourceEnumerator->Release();
    }

    return iErrCode;
}


int Admin::HandleAdminPageSubmission (IHttpRequest* pHttpRequest, IHttpResponse* pHttpResponse, 
                                      String* pstrMessage) {

    IHttpForm* pHttpForm;
    IPageSourceControl* pPageSource;
    IReport* pReport;
    ILog* pLog;

    int iErrCode;
    const char* pszValue;

    // Check for refresh
    pHttpForm = pHttpRequest->GetForm ("Refresh");
    if (pHttpForm != NULL) {
        return OK;
    }

    // Check for pagesource restart
    pHttpForm = pHttpRequest->GetForm ("RestartPageSource");
    if (pHttpForm != NULL) {

        // Lookup the pagesource
        pHttpForm = pHttpRequest->GetForm ("RestartName");
        if (pHttpForm == NULL) {
            *pstrMessage = "You must submit a valid PageSource to restart";
            return OK;
        }

        pszValue = pHttpForm->GetValue();
        if (pszValue == NULL) {
            *pstrMessage = "You must submit a valid PageSource name to restart";
            return OK;
        }

        pPageSource = m_pHttpServer->GetPageSourceByName (pszValue);
        if (pPageSource == NULL) {
            *pstrMessage = "The PageSource ";
            *pstrMessage += pszValue;
            *pstrMessage += " does not exist";
            return OK;
        }

        // Restart the PageSource
        pPageSource->Restart();
        pPageSource->Release();

        *pstrMessage = "The PageSource ";
        *pstrMessage += pszValue;
        *pstrMessage += " will be restarted";
        return OK;
    }

    // Check for pagesource shutdown
    pHttpForm = pHttpRequest->GetForm ("ShutdownPageSource");
    if (pHttpForm != NULL) {

        // Lookup the pagesource
        pHttpForm = pHttpRequest->GetForm ("ShutdownName");
        if (pHttpForm == NULL) {
            *pstrMessage = "You must submit a valid PageSource to shut down";
            return OK;
        }

        pszValue = pHttpForm->GetValue();
        if (pszValue == NULL) {
            *pstrMessage = "You must submit a valid PageSource name to shut down";
            return OK;
        }

        pPageSource = m_pHttpServer->GetPageSourceByName (pszValue);
        if (pPageSource == NULL) {
            *pstrMessage = "The PageSource ";
            *pstrMessage += pszValue;
            *pstrMessage += " does not exist";
            return OK;
        }

        // Restart the PageSource
        iErrCode = pPageSource->Shutdown();
        pPageSource->Release();

        if (iErrCode == OK) {
            *pstrMessage = "The PageSource ";
            *pstrMessage += pszValue;
            *pstrMessage += " will be shut down";
        } else {
            *pstrMessage = "The PageSource ";
            *pstrMessage += pszValue;
            *pstrMessage += " could not be shut down";
        }

        return OK;
    }

    // Check for server restart
    pHttpForm = pHttpRequest->GetForm ("RestartServer");
    if (pHttpForm != NULL) {

        if (m_pHttpServer->Restart (NULL) == OK) {
            *pstrMessage = "The Server will be restarted";
        } else {
            *pstrMessage = "The Server could not be restarted";
        }

        return OK;
    }

    // Check for server shutdown
    pHttpForm = pHttpRequest->GetForm ("ShutdownServer");
    if (pHttpForm != NULL) {

        if (m_pHttpServer->Shutdown() == OK) {
            *pstrMessage = "The Server will be shut down";
        } else {
            *pstrMessage = "The Server could not be shut down";
        }

        return OK;
    }

    // Check for pagesource report viewing
    char* pszBuffer = new char [m_stDisplayChars + 1];
    Algorithm::AutoDelete<char> autoPtr (pszBuffer, true);

    if (pszBuffer == NULL) {
        *pstrMessage = "The server is out of memory";
        return ERROR_OUT_OF_MEMORY;
    }

    pHttpForm = pHttpRequest->GetForm ("PageSourceReport");
    if (pHttpForm != NULL) {

        // Lookup the pagesource
        pHttpForm = pHttpRequest->GetForm ("ReportName");
        if (pHttpForm == NULL) {
            *pstrMessage = "You must submit a valid PageSource to view a report";
            return OK;
        }

        pszValue = pHttpForm->GetValue();
        if (pszValue == NULL) {
            *pstrMessage = "You must submit a valid PageSource name to view a report";
            return OK;
        }

        pPageSource = m_pHttpServer->GetPageSourceByName (pszValue);
        if (pPageSource == NULL) {
            *pstrMessage = "The PageSource ";
            *pstrMessage += pszValue;
            *pstrMessage += " does not exist";
            return OK;
        }

        // Restart the PageSource
        pReport = pPageSource->GetReport();
        if (pReport == NULL) {

            *pstrMessage = "The ";
            *pstrMessage += pszValue;
            *pstrMessage += " PageSource does not have a report";

        } else {
            
            if (pReport->GetReportTail (pszBuffer, m_stDisplayChars) > 0) {
                
                *pstrMessage = "</center>";
                pstrMessage->AppendHtml (pszBuffer, 60, true);
                *pstrMessage += "<center>";
                
            } else {
                
                *pstrMessage = "The ";
                *pstrMessage += pszValue;
                *pstrMessage += " PageSource has an empty report";
            }
        }

        pPageSource->Release();

        return OK;
    }

    // Check for pagesource log viewing
    pHttpForm = pHttpRequest->GetForm ("PageSourceLog");
    if (pHttpForm != NULL) {

        // Lookup the pagesource
        pHttpForm = pHttpRequest->GetForm ("LogName");
        if (pHttpForm == NULL) {
            *pstrMessage = "You must submit a valid PageSource to view a log";
            return OK;
        }

        pszValue = pHttpForm->GetValue();
        if (pszValue == NULL) {
            *pstrMessage = "You must submit a valid PageSource name to view a log";
            return OK;
        }

        pPageSource = m_pHttpServer->GetPageSourceByName (pszValue);
        if (pPageSource == NULL) {
            *pstrMessage = "The PageSource ";
            *pstrMessage += pszValue;
            *pstrMessage += " does not exist";
            return OK;
        }

        // Restart the PageSource
        pLog = pPageSource->GetLog();
        if (pLog == NULL) {

            *pstrMessage = "The ";
            *pstrMessage += pszValue;
            *pstrMessage += " PageSource does not have a log";

        } else {
            
            if (pLog->GetLogTail (pszBuffer, m_stDisplayChars) > 0) {
                
                *pstrMessage = "</center>";
                pstrMessage->AppendHtml (pszBuffer, 60, true);
                *pstrMessage += "<center>";
                
            } else {
                
                *pstrMessage = "The ";
                *pstrMessage += pszValue;
                *pstrMessage += " PageSource has an empty log";
            }
        }

        pPageSource->Release();

        return OK;
    }

    // Check for server report viewing
    pHttpForm = pHttpRequest->GetForm ("ServerReport");
    if (pHttpForm != NULL) {

        m_pServerReport->GetReportTail (pszBuffer, m_stDisplayChars);

        *pstrMessage = "</center>";
        pstrMessage->AppendHtml (pszBuffer, 60, true);
        *pstrMessage += "<center>";

        return OK;
    }

    // Check for a cached file release
    pHttpForm = pHttpRequest->GetForm ("FlushAFile");
    if (pHttpForm != NULL) {

        pHttpForm = pHttpRequest->GetForm ("FlushFileName");
        if (pHttpForm == NULL || (pszValue = pHttpForm->GetValue()) == NULL || *pszValue == '\0') {
            *pstrMessage = "You must submit a valid file name";
        } else {

            if (m_pFileCache->ReleaseFile (pszValue) == OK) {
                *pstrMessage = pszValue;
                *pstrMessage += " was released from the file cache";
            } else {
                *pstrMessage = pszValue;
                *pstrMessage += " is not being cached";
            }
        }

        return OK;
    }

    // Check for all cached file release
    pHttpForm = pHttpRequest->GetForm ("FlushAllFiles");
    if (pHttpForm != NULL) {

        if (m_pFileCache->ReleaseAllFiles() == OK) {
            *pstrMessage = "All cached files were released from the file cache";
        } else {
            *pstrMessage = "The file cache is not active";
        }

        return OK;
    }

    // Update server parameters
    char* pszLhs, * pszRhs;
    unsigned int i, iNumParameters;

    pHttpForm = pHttpRequest->GetForm ("UpdateServerParams");
    if (pHttpForm != NULL) {

        // Loop through all parameters, compare values
        iNumParameters = m_pServerConfig->GetNumParameters();

        m_mServerLock.Wait();

        for (i = 0; i < iNumParameters; i ++) {
            
            if (m_pServerConfig->GetParameter (i, &pszLhs, &pszRhs) == OK) {

                pHttpForm = pHttpRequest->GetForm (pszLhs);
                if (pHttpForm != NULL) {

                    pszValue = pHttpForm->GetValue();
                    if (String::StrCmp (pszValue, pszRhs) != 0) {

                        if (m_pServerConfig->SetParameter (pszLhs, pszValue) == OK) {

                            if (!pstrMessage->IsBlank()) {
                                *pstrMessage += "<br>";
                            }
                            *pstrMessage += "The value for the ";
                            *pstrMessage += pszLhs;
                            *pstrMessage += " parameter was updated";
                        }
                    }
                }
            }
        }

        m_mServerLock.Signal();
        
        if (!pstrMessage->IsBlank()) {
            *pstrMessage += "<p><strong>You will have to restart the server for the changes to take effect</strong>";
        }

        return OK;
    }

    // Update pagesource parameters
    IPageSourceEnumerator* pPageSourceEnumerator = NULL;
    IPageSourceControl** ppPageSource;
    IConfigFile* pConfig;
    
    String strFormName;
    const char* pszName;

    char* pszFormName = NULL;
    size_t stFormNameLen = 0;

    unsigned int j, iNumPageSources;

    pPageSourceEnumerator = m_pHttpServer->EnumeratePageSources();
    if (pPageSourceEnumerator != NULL && (iNumPageSources = pPageSourceEnumerator->GetNumPageSources()) != 0) {

        ppPageSource = pPageSourceEnumerator->GetPageSourceControls();

        for (i = 0; i < iNumPageSources; i ++) {
            
            pszName = ppPageSource[i]->GetName();

            strFormName = "Update";
            strFormName += pszName;
            strFormName += "PageSourceParams";

            pHttpForm = pHttpRequest->GetForm (strFormName);
            if (pHttpForm != NULL) {
                
                pConfig = ppPageSource[i]->GetConfigFile();
                iNumParameters = pConfig->GetNumParameters();

                m_mServerLock.Wait();

                for (j = 0; j < iNumParameters; j ++) {
                    
                    if (pConfig->GetParameter (j, &pszLhs, &pszRhs) == OK) {

                        size_t stThisFormNameLen = String::StrLen (pszName) + String::StrLen (pszLhs) + 2;
                        if (stThisFormNameLen > stFormNameLen) {

                            delete [] pszFormName;
                            pszFormName = new char [stThisFormNameLen];
                            if (pszFormName == NULL) {
                                m_mServerLock.Signal();
                                pPageSourceEnumerator->Release();
                                *pstrMessage = "The server is out of memory";
                                return ERROR_OUT_OF_MEMORY;
                            }
                            stFormNameLen = stThisFormNameLen;
                        }
                        sprintf (pszFormName, "%s_%s", pszName, pszLhs);
                        
                        pHttpForm = pHttpRequest->GetForm (pszFormName);
                        if (pHttpForm != NULL) {
                            
                            pszValue = pHttpForm->GetValue();
                            if (String::StrCmp (pszValue, pszRhs) != 0) {
                                
                                if (pConfig->SetParameter (pszLhs, pszValue) == OK) {
                                
                                    if (!pstrMessage->IsBlank()) {
                                        *pstrMessage += "<br>";
                                    }
                                    *pstrMessage += "The value for the ";
                                    *pstrMessage += pszLhs;
                                    *pstrMessage += " parameter was updated";
                                }
                            }
                        }
                    }
                }

                m_mServerLock.Signal();
                
                if (!pstrMessage->IsBlank()) {
                    *pstrMessage += "<p><strong>You will have to restart the ";
                    *pstrMessage += pszName;
                    *pstrMessage += " PageSource for the changes to take effect</strong>";
                }

                break;
            }
        }
    }

    if (pPageSourceEnumerator != NULL) {
        pPageSourceEnumerator->Release();
    }

    delete [] pszFormName;

    return OK;
}

int Admin::ConvertTime (Seconds iNumSeconds, String* pstrTime) {

    int iHrs = 0, iMin = 0;

    if (iNumSeconds < 0) {
        *pstrTime = "<strong>Error: ";
        *pstrTime += iNumSeconds;
        *pstrTime += "</strong>";
        return pstrTime->GetCharPtr() != NULL ? OK : ERROR_OUT_OF_MEMORY;
    }

    if (iNumSeconds == 0) {
        *pstrTime = "<strong>0</strong> sec";
        return pstrTime->GetCharPtr() != NULL ? OK : ERROR_OUT_OF_MEMORY;
    }
        
    if (iNumSeconds >= 3600) {
        
        iHrs = iNumSeconds / 3600;
        iNumSeconds -= iHrs * 3600;
        
        *pstrTime = "<strong>";
        *pstrTime += iHrs;
        *pstrTime += "</strong> hr";

        if (iHrs != 1) {
            *pstrTime += "s";
        }
    } else {
        *pstrTime = "";
    }
    
    if (iNumSeconds >= 60) {
        iMin = iNumSeconds / 60;
        iNumSeconds -= iMin * 60;
        
        if (iHrs > 0) {
            *pstrTime += ", ";
        }
        
        *pstrTime += "<strong>";
        *pstrTime += iMin;
        *pstrTime += "</strong> min";
    }
    
    if (iNumSeconds > 0) {
        
        if (iMin > 0 || iHrs > 0) {
            *pstrTime += ", ";
        }
        
        *pstrTime += "<strong>";
        *pstrTime += iNumSeconds;
        *pstrTime += "</strong> sec";
    }

    return pstrTime->GetCharPtr() != NULL ? OK : ERROR_OUT_OF_MEMORY;
}