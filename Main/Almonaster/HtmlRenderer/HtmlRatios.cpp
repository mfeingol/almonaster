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


int HtmlRenderer::WriteRatiosString (RatioInformation* pratInfo) {

    int iErrCode;
    float fMaxTechDev;

    if (pratInfo == NULL) {
        pratInfo = (RatioInformation*) StackAlloc (sizeof (RatioInformation));
    }
    
    iErrCode = GetGameClassMaxTechIncrease (m_iGameClass, &fMaxTechDev);
    RETURN_ON_ERROR(iErrCode);
    
    iErrCode = GetRatioInformation (m_iGameClass, m_iGameNumber, m_iEmpireKey, pratInfo);
    RETURN_ON_ERROR(iErrCode);

    const char* pszGood = m_vGoodColor.GetCharPtr();
    const char* pszBad = m_vBadColor.GetCharPtr();
    
    // Maintenance ratio
    OutputText (
        "<p>"\
        "<table width=\"85%\">"\
        "<tr>"\
        "<td align=\"right\">Maint Ratio: <strong><font color=\"");

    if (pratInfo->fMaintRatio >= (float) 1.0) {
        m_pHttpResponse->WriteText (pszGood);
    } else {
        m_pHttpResponse->WriteText (pszBad);
    }
    
    OutputText ("\">");
    m_pHttpResponse->WriteText (pratInfo->fMaintRatio);
    
    // Fuel ratio
    OutputText ("</font></strong></td><td align=\"right\">Fuel Ratio: <strong><font color=\"");
    
    if (pratInfo->fFuelRatio >= (float) 1.0) {
        m_pHttpResponse->WriteText (pszGood);
    } else {
        m_pHttpResponse->WriteText (pszBad);
    }
    
    OutputText ("\">");
    m_pHttpResponse->WriteText (pratInfo->fFuelRatio);

    // Ag ratio
    OutputText ("</font></strong></td><td align=\"right\">Ag Ratio: <strong><font color=\"");
    
    if (pratInfo->fAgRatio >= (float) 1.0) {
        m_pHttpResponse->WriteText (pszGood);
    } else {
        m_pHttpResponse->WriteText (pszBad);
    }
    
    OutputText ("\">");
    m_pHttpResponse->WriteText (pratInfo->fAgRatio);

    // Tech level
    OutputText ("</font></strong></td><td align=\"right\">Tech Level: <strong>");

    if (pratInfo->fTechLevel < (float) 1.0) {

        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (pszBad);
        OutputText ("\">");
        m_pHttpResponse->WriteText (pratInfo->fTechLevel);
        OutputText (" (BR ");
        m_pHttpResponse->WriteText (pratInfo->iBR);
        OutputText (")</font>");

    } else {

        m_pHttpResponse->WriteText (pratInfo->fTechLevel);
        OutputText (" (BR ");
        m_pHttpResponse->WriteText (pratInfo->iBR);
        OutputText (")");
    }

    OutputText ("</strong></td><td align=\"right\">Tech Increase: <strong>");
    
    if (pratInfo->fTechDev < (float) 0.0) {
        
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (pszBad);
        OutputText ("\">");
        m_pHttpResponse->WriteText (pratInfo->fTechDev);
        OutputText ("</font>");

    } else {
        
        if (pratInfo->fTechDev == fMaxTechDev) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (pszGood);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pratInfo->fTechDev);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (pratInfo->fTechDev);
        }
    }        
    
    OutputText ("</strong></td></tr>");
    
    // Next maintenance ratio
    OutputText ("<tr><td align=\"right\">Next: <strong><font color=\"");
    
    if (pratInfo->fNextMaintRatio >= (float) 1.0) {
        m_pHttpResponse->WriteText (pszGood);
    } else {
        m_pHttpResponse->WriteText (pszBad);
    }
    
    OutputText ("\">");
    m_pHttpResponse->WriteText (pratInfo->fNextMaintRatio);
    
    // Next fuel ratio
    OutputText ("</font></strong></td><td align=\"right\">Next: <strong><font color=\"");
    
    if (pratInfo->fNextFuelRatio >= (float) 1.0) {
        m_pHttpResponse->WriteText (pszGood);
    } else {
        m_pHttpResponse->WriteText (pszBad);
    }
    
    OutputText ("\">");
    m_pHttpResponse->WriteText (pratInfo->fNextFuelRatio);

    // Next ag ratio
    OutputText ("</font></strong></td><td align=\"right\">Next: <strong><font color=\"");
    
    if (pratInfo->fNextAgRatio >= (float) 1.0) {
        m_pHttpResponse->WriteText (pszGood);
    } else {
        m_pHttpResponse->WriteText (pszBad);
    }
    
    OutputText ("\">");
    m_pHttpResponse->WriteText (pratInfo->fNextAgRatio);
    
    // Next tech level
    OutputText ("</font></strong></td><td align=\"right\">Next: <strong>");
    
    if (pratInfo->iNextBR > pratInfo->iBR) {
        
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (pszGood);
        OutputText ("\">");
        
        m_pHttpResponse->WriteText (pratInfo->fNextTechLevel);
        OutputText (" (BR ");
        m_pHttpResponse->WriteText (pratInfo->iNextBR);
        OutputText (")");
        
        OutputText ("</font>");
        
    } else {
        
        m_pHttpResponse->WriteText (pratInfo->fNextTechLevel);
        OutputText (" (BR ");
        m_pHttpResponse->WriteText (pratInfo->iNextBR);
        OutputText (")");
    }
    
    OutputText ("</strong></td><td align=\"right\">Next: <strong>");
    
    if (pratInfo->fNextTechDev < (float) 0.0) {
        OutputText ("<font color=\"");
        m_pHttpResponse->WriteText (pszBad);
        OutputText ("\">");
        m_pHttpResponse->WriteText (pratInfo->fNextTechDev);
        OutputText ("</font>");
    } else {
        
        if (pratInfo->fNextTechDev == fMaxTechDev) {
            OutputText ("<font color=\"");
            m_pHttpResponse->WriteText (pszGood);
            OutputText ("\">");
            m_pHttpResponse->WriteText (pratInfo->fNextTechDev);
            OutputText ("</font>");
        } else {
            m_pHttpResponse->WriteText (pratInfo->fNextTechDev);
        }
    }        
    
    OutputText ("</table><p>");

    int iSeparatorAddress;
    iErrCode = GetThemeAddress(m_iSeparatorKey, &iSeparatorAddress);
    RETURN_ON_ERROR(iErrCode);

    WriteSeparatorString(m_iSeparatorKey, iSeparatorAddress);

    return iErrCode;
}