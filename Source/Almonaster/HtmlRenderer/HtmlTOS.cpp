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

void HtmlRenderer::WriteTOS() {

    if (!(m_iSystemOptions2 & EMPIRE_ACCEPTED_TOS)) {

        OutputText ("<p><font color=\"#");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">Please read and accept the Terms of Service before proceeding to play:</font>");
    }

    OutputText ("<p></center>");
    WriteTOSFile();
    OutputText ("<center>");

    if (!(m_iSystemOptions2 & EMPIRE_ACCEPTED_TOS)) {

        OutputText ("<p><font color=\"#");
        m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
        OutputText ("\">Do you accept these terms?<p>");

        WriteButton (BID_TOS_DECLINE);
        WriteButton (BID_TOS_ACCEPT);
    }
}

void HtmlRenderer::WriteConfirmTOSDecline() {

    OutputText ("<p><font color=\"#");
    m_pHttpResponse->WriteText (m_vGoodColor.GetCharPtr());
    OutputText (
        "\">Are you sure that you wish to decline the Terms of Service? "\
        "If you do decline, your empire will removed from the server</font><p>"
        );

    WriteButton (BID_TOS_DECLINE);
    WriteButton (BID_CANCEL);
}

void HtmlRenderer::WriteTOSFile() {

    char pszFileName[OS::MaxFileNameLength];
    sprintf(pszFileName, "%s/" TOS_FILE, global.GetResourceDir());

    ICachedFile* pcfCachedFile = global.GetFileCache()->GetFile (pszFileName);
    AutoRelease<ICachedFile> release_pcfCachedFile(pcfCachedFile);

    if (pcfCachedFile)
    {
        m_pHttpResponse->WriteTextFile (pcfCachedFile);
    }
}