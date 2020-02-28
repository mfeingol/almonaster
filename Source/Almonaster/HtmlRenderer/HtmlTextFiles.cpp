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

#include "Osal/Algorithm.h"
#include "Osal/Crypto.h"
#include "Osal/File.h"

void HtmlRenderer::WriteFaq() {

    char pszFileName[OS::MaxFileNameLength];
    sprintf(pszFileName, "%s/" FAQ_FILE, global.GetResourceDir());
    
    ICachedFile* pcfFaq = global.GetFileCache()->GetFile(pszFileName);
    AutoRelease<ICachedFile> release_pcfFaq(pcfFaq);
    if (pcfFaq == NULL)
    {
        OutputText ("<p><strong>The documentation file could not be found; please alert the administrator</strong>");
    }
    else
    {
        OutputText ("<p>");
        m_pHttpResponse->WriteTextFile(pcfFaq);
    }
}

void HtmlRenderer::WriteServerNews() {
    
    int iSec, iMin, iHour, iDay, iMon, iYear;
    DayOfWeek day;

    Time::GetDate (&iSec, &iMin, &iHour, &day, &iDay, &iMon, &iYear); 

    const char* pszMonth = Time::GetMonthName (iMon);
    const char* pszDayW  = Time::GetDayOfWeekName (day);

    OutputText ("<p><table><tr><th bgcolor=\"#");
    m_pHttpResponse->WriteText (m_vTableColor.GetCharPtr());
    OutputText ("\">Today is ");
    m_pHttpResponse->WriteText (pszDayW);
    OutputText (", ");
    m_pHttpResponse->WriteText (pszMonth);
    OutputText (" ");
    m_pHttpResponse->WriteText (iDay);
    OutputText (", ");
    m_pHttpResponse->WriteText (iYear);
    OutputText ("</th></tr></table><p>");
    
    WriteServerNewsFile(false);
}

void HtmlRenderer::WriteContributions() {

    char pszFileName[OS::MaxFileNameLength];
    sprintf(pszFileName, "%s/" CONTRIBUTIONS_FILE, global.GetResourceDir());

    ICachedFile* pcfFile = global.GetFileCache()->GetFile (pszFileName);
    AutoRelease<ICachedFile> release_pcfFile(pcfFile);

    if (pcfFile == NULL)
    {
        OutputText ("<p><strong>The contributions file could not be found; please alert the administrator</strong>");
    }
    else
    {
        m_pHttpResponse->WriteTextFile (pcfFile);
    }

    OutputText ("<p>");

    WriteContributorsFile(false);
}

void HtmlRenderer::WriteCredits() {

    char pszFileName[OS::MaxFileNameLength];
    sprintf(pszFileName, "%s/" CREDITS_FILE, global.GetResourceDir());

    ICachedFile* pcfFile = global.GetFileCache()->GetFile (pszFileName);
    AutoRelease<ICachedFile> release_pcfFile(pcfFile);

    if (pcfFile == NULL)
    {
        OutputText ("<p><strong>The credits file could not be found; please alert the administrator</strong>");
    }
    else
    {
        m_pHttpResponse->WriteTextFile (pcfFile);
    }
}

void HtmlRenderer::WriteIntro() {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf(pszFileName, "%s/" INTRO_FILE, global.GetResourceDir());
    
    ICachedFile* pcfFile = global.GetFileCache()->GetFile (pszFileName);
    AutoRelease<ICachedFile> release_pcfFile(pcfFile);

    if (pcfFile == NULL)
    {
        OutputText ("<p><strong>The intro file could not be found; please alert the administrator</strong>");
    }
    else
    {
        m_pHttpResponse->WriteTextFile (pcfFile);
    }
}

void HtmlRenderer::WriteIntroUpper (bool bTextArea) {

    WriteTextFile (bTextArea, INTRO_UPPER_FILE, "IntroUpperText", "IntroUpperHash");
}

void HtmlRenderer::WriteIntroLower (bool bTextArea) {
    
    WriteTextFile (bTextArea, INTRO_LOWER_FILE, "IntroLowerText", "IntroLowerHash");
}

void HtmlRenderer::WriteServerNewsFile (bool bTextArea) {
    
    WriteTextFile (bTextArea, NEWS_FILE, "ServerNewsText", "ServerNewsHash");
}

void HtmlRenderer::WriteContributorsFile (bool bTextArea) {
    
    WriteTextFile (bTextArea, CONTRIBUTORS_FILE, "ContributorsText", "ContributorsHash");
}

int HtmlRenderer::WriteTextFile (bool bTextArea, const char* pszFile, const char* pszFileForm, const char* pszFileHashForm)
{
    int iErrCode = OK;

    char pszFileName[OS::MaxFileNameLength];
    sprintf(pszFileName, "%s/%s", global.GetResourceDir(), pszFile);

    AutoReadLock lock(&ms_mTextFileLock);

    size_t cbFileSize = 0;
    ICachedFile* pcfCachedFile = global.GetFileCache()->GetFile (pszFileName);
    AutoRelease<ICachedFile> release_pcfCachedFile(pcfCachedFile);

    if (pcfCachedFile != NULL)
    {
        cbFileSize = pcfCachedFile->GetSize();
        if (cbFileSize > 0)
        {
            Crypto::HashMD5 hash;

            iErrCode = hash.HashData (pcfCachedFile->GetData(), cbFileSize);
            RETURN_ON_ERROR(iErrCode);

            size_t cbHashLen;
            iErrCode = hash.GetHashSize (&cbHashLen);
            RETURN_ON_ERROR(iErrCode);

            char* pbHashData = (char*) StackAlloc (cbHashLen);
            iErrCode = hash.GetHash (pbHashData, cbHashLen);
            RETURN_ON_ERROR(iErrCode);

            char pszBase64 [32];
            iErrCode = Algorithm::EncodeBase64 (pbHashData, cbHashLen, pszBase64, countof (pszBase64));
            RETURN_ON_ERROR(iErrCode);

            OutputText ("<input type=\"hidden\" name=\"");
            m_pHttpResponse->WriteText (pszFileHashForm);
            OutputText ("\" value=\"");
            m_pHttpResponse->WriteText (pszBase64);
            OutputText ("\">");
        }
    }

    if (bTextArea)
    {
        OutputText ("<textarea name=\"");
        m_pHttpResponse->WriteText (pszFileForm);
        OutputText ("\" rows=\"20\" cols=\"60\" wrap=\"virtual\">");
    }

    if (cbFileSize > 0) {
        m_pHttpResponse->WriteTextFile (pcfCachedFile);
    }

    if (bTextArea) {
        OutputText ("</textarea>");
    }

    return iErrCode;
}

int HtmlRenderer::TryUpdateIntroUpper() {
    
    return TryUpdateFile (INTRO_UPPER_FILE, "IntroUpperText", "IntroUpperHash");
}

int HtmlRenderer::TryUpdateIntroLower() {
    
    return TryUpdateFile (INTRO_LOWER_FILE, "IntroLowerText", "IntroLowerHash");
}

int HtmlRenderer::TryUpdateServerNews() {

    int iErrCode = TryUpdateFile (NEWS_FILE, "ServerNewsText", "ServerNewsHash");
    if (iErrCode == WARNING)
    {
        return iErrCode;
    }
    RETURN_ON_ERROR(iErrCode);

    Time::GetTime (&m_stServerNewsLastUpdate);
    return iErrCode;
}

int HtmlRenderer::TryUpdateContributors()
{
    int iErrCode = TryUpdateFile (CONTRIBUTORS_FILE, "ContributorsText", "ContributorsHash");
    if (iErrCode == WARNING)
    {
        return iErrCode;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int HtmlRenderer::TryUpdateFile (const char* pszFile, const char* pszFileForm, const char* pszFileHashForm) {

    int iErrCode;
    IHttpForm* pHttpForm;

    pHttpForm = m_pHttpRequest->GetForm(pszFileForm);
    if (pHttpForm == NULL)
    {
        return WARNING;
    }
    const char* pszText = pHttpForm->GetValue();

    pHttpForm = m_pHttpRequest->GetForm (pszFileHashForm);
    if (pHttpForm != NULL && pszText != NULL) {

        const char* pszBase64 = pHttpForm->GetValue();
        if (pszBase64 != NULL)
        {
            char pbOldHash[32];
            size_t cbDecoded;
            iErrCode = Algorithm::DecodeBase64(pszBase64, pbOldHash, sizeof(pbOldHash), &cbDecoded);
            RETURN_ON_ERROR(iErrCode);

            Crypto::HashMD5 hash;

            iErrCode = hash.HashData (pszText, strlen (pszText));
            RETURN_ON_ERROR(iErrCode);

            size_t cbHashLen;
            iErrCode = hash.GetHashSize (&cbHashLen);
            RETURN_ON_ERROR(iErrCode);

            if (cbHashLen == cbDecoded)
            {
                char* pbNewHash = (char*) StackAlloc (cbHashLen);
                iErrCode = hash.GetHash (pbNewHash, cbHashLen);
                RETURN_ON_ERROR(iErrCode);

                if (memcmp (pbOldHash, pbNewHash, cbHashLen) == 0)
                {
                    // No change
                    return WARNING;
                }
            }
        }
    }

    // Go ahead and update the file
    char pszFileName[OS::MaxFileNameLength];
    sprintf(pszFileName, "%s/%s", global.GetResourceDir(), pszFile);

    AutoWriteLock lock(&ms_mTextFileLock);
    iErrCode = UpdateCachedFile (pszFileName, pszText);
    if (iErrCode == WARNING)
    {
        return iErrCode;
    }
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int HtmlRenderer::UpdateCachedFile (const char* pszFileName, const char* pszText) {
    
    int iErrCode = OK;
    size_t stSize;

    File fCachedFile;
    
    ICachedFile* pcfCachedFile = global.GetFileCache()->GetFile (pszFileName);
    AutoRelease<ICachedFile> release_pcfCachedFile(pcfCachedFile);

    if (pcfCachedFile == NULL || (stSize = pcfCachedFile->GetSize()) == 0)
    {
        if (pszText == NULL || *pszText == '\0')
        {
            return WARNING;
        }
    }
    else
    {
        const char* pszData = (char*)pcfCachedFile->GetData();        
        size_t stNewSize = String::StrLen(pszText);

        if (stSize == stNewSize && strncmp (pszText, pszData, stNewSize) == 0)
        {
            return WARNING;
        }
    }

    if (pcfCachedFile)
    {
        global.GetFileCache()->ReleaseFile(pszFileName);
        SafeRelease(pcfCachedFile);
    }

    iErrCode = fCachedFile.OpenWrite (pszFileName);
    RETURN_ON_ERROR(iErrCode);

    fCachedFile.Write (pszText == NULL ? "" : pszText);
    fCachedFile.Close();
    
    return iErrCode;
}