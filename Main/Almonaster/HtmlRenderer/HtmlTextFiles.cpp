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
    
    OutputText ("<p>Printer-friendly documentation can be found <a href=\"" BASE_RESOURCE_DIR FAQ_FILE "\">here</a>.<p>");
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" FAQ_FILE, global.GetResourceDir());
    
    ICachedFile* pcfFaq = global.GetFileCache()->GetFile (pszFileName);
    if (pcfFaq == NULL) {
        OutputText ("<p><strong>The documentation file could not be found; please alert your system administrator</strong>");
    } else {
        OutputText ("<p></center>");
        m_pHttpResponse->WriteTextFile (pcfFaq);
        pcfFaq->Release();
        OutputText ("<center>");
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
    
    WriteServerNewsFile (false);
}

void HtmlRenderer::WriteContributions() {

    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" CONTRIBUTIONS_FILE, global.GetResourceDir());

    ICachedFile* pcfFile = global.GetFileCache()->GetFile (pszFileName);
    if (pcfFile == NULL) {
        OutputText ("<p><strong>The contributions file could not be found; please alert your system administrator</strong>");
    } else {
        m_pHttpResponse->WriteTextFile (pcfFile);
        pcfFile->Release();
    }

    OutputText ("<p>");

    WriteContributorsFile (false);
}

void HtmlRenderer::WriteCredits() {

    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" CREDITS_FILE, global.GetResourceDir());

    ICachedFile* pcfFile = global.GetFileCache()->GetFile (pszFileName);
    if (pcfFile == NULL) {
        OutputText ("<p><strong>The credits file could not be found; please alert your system administrator</strong>");
    } else {
        m_pHttpResponse->WriteTextFile (pcfFile);
        pcfFile->Release();
    }
}

void HtmlRenderer::WriteIntro() {
    
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/" INTRO_FILE, global.GetResourceDir());
    
    ICachedFile* pcfFile = global.GetFileCache()->GetFile (pszFileName);
    if (pcfFile == NULL) {
        OutputText ("<p><strong>The intro file could not be found; please alert your system administrator</strong>");
    } else {
        m_pHttpResponse->WriteTextFile (pcfFile);
        pcfFile->Release();
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

int HtmlRenderer::WriteTextFile (bool bTextArea, const char* pszFile, 
                                 const char* pszFileForm, const char* pszFileHashForm) {

    int iErrCode = OK;

    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/%s", global.GetResourceDir(), pszFile);

    ms_mTextFileLock.WaitReader();

    size_t cbFileSize = 0;
    ICachedFile* pcfCachedFile = global.GetFileCache()->GetFile (pszFileName);
    if (pcfCachedFile != NULL) {

        cbFileSize = pcfCachedFile->GetSize();
        if (cbFileSize > 0) {

            Crypto::HashMD5 hash;

            iErrCode = hash.HashData (pcfCachedFile->GetData(), cbFileSize);
            if (iErrCode == OK) {

                size_t cbHashLen;
                iErrCode = hash.GetHashSize (&cbHashLen);
                if (iErrCode == OK) {

                    char* pbHashData = (char*) StackAlloc (cbHashLen);
                    iErrCode = hash.GetHash (pbHashData, cbHashLen);
                    if (iErrCode == OK) {

                        char pszBase64 [32];
                        iErrCode = Algorithm::EncodeBase64 (pbHashData, cbHashLen, pszBase64, countof (pszBase64));
                        if (iErrCode == OK) {
                            OutputText ("<input type=\"hidden\" name=\"");
                            m_pHttpResponse->WriteText (pszFileHashForm);
                            OutputText ("\" value=\"");
                            m_pHttpResponse->WriteText (pszBase64);
                            OutputText ("\">");
                        }
                    }
                }
            }
        }
    }

    if (bTextArea) {
        OutputText ("<textarea name=\"");
        m_pHttpResponse->WriteText (pszFileForm);
        OutputText ("\" rows=\"20\" cols=\"60\" wrap=\"virtual\">");
    }

    if (cbFileSize > 0) {
        m_pHttpResponse->WriteTextFile (pcfCachedFile);
    }

    SafeRelease (pcfCachedFile);

    if (bTextArea) {
        OutputText ("</textarea>");
    }

    ms_mTextFileLock.SignalReader();

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
    if (iErrCode == OK) {
        Time::GetTime (&m_stServerNewsLastUpdate);
    }

    return iErrCode;
}

int HtmlRenderer::TryUpdateContributors() {
    
    return TryUpdateFile (CONTRIBUTORS_FILE, "ContributorsText", "ContributorsHash");
}

int HtmlRenderer::TryUpdateFile (const char* pszFile, const char* pszFileForm, const char* pszFileHashForm) {

    int iErrCode;
    IHttpForm* pHttpForm;

    pHttpForm = m_pHttpRequest->GetForm (pszFileForm);
    if (pHttpForm == NULL) {
        return WARNING;
    }
    const char* pszText = pHttpForm->GetValue();

    pHttpForm = m_pHttpRequest->GetForm (pszFileHashForm);
    if (pHttpForm != NULL && pszText != NULL) {

        const char* pszBase64 = pHttpForm->GetValue();
        if (pszBase64 != NULL) {

            char pbOldHash [32];
            size_t cbDecoded;
            iErrCode = Algorithm::DecodeBase64 (pszBase64, pbOldHash, sizeof (pbOldHash), &cbDecoded);
            if (iErrCode == OK) {

                Crypto::HashMD5 hash;

                iErrCode = hash.HashData (pszText, strlen (pszText));
                if (iErrCode == OK) {

                    size_t cbHashLen;
                    iErrCode = hash.GetHashSize (&cbHashLen);
                    if (iErrCode == OK && cbHashLen == cbDecoded) {

                        char* pbNewHash = (char*) StackAlloc (cbHashLen);
                        iErrCode = hash.GetHash (pbNewHash, cbHashLen);
                        if (iErrCode == OK) {

                            if (memcmp (pbOldHash, pbNewHash, cbHashLen) == 0) {

                                // No change
                                return WARNING;
                            }
                        }
                    }
                }
            }

            else Assert (iErrCode == ERROR_SMALL_BUFFER);
        }
    }

    // Go ahead and update the file
    char pszFileName[OS::MaxFileNameLength];
    sprintf (pszFileName, "%s/%s", global.GetResourceDir(), pszFile);

    ms_mTextFileLock.WaitWriter();
    iErrCode = UpdateCachedFile (pszFileName, pszText);
    ms_mTextFileLock.SignalWriter();

    return iErrCode;
}

int HtmlRenderer::UpdateCachedFile (const char* pszFileName, const char* pszText) {
    
    int iErrCode = OK;
    size_t stSize;

    File fCachedFile;
    
    ICachedFile* pcfCachedFile = global.GetFileCache()->GetFile (pszFileName);
    if (pcfCachedFile == NULL || (stSize = pcfCachedFile->GetSize()) == 0) {

        if (pszText == NULL || *pszText == '\0') {
            iErrCode =  WARNING;
            goto Cleanup;
        }

    } else {

        const char* pszData = (char*) pcfCachedFile->GetData();        
        size_t stNewSize = String::StrLen (pszText);

        if (stSize == stNewSize && strncmp (pszText, pszData, stNewSize) == 0) {
            iErrCode =  WARNING;
            goto Cleanup;
        }
    }

    if (pcfCachedFile != NULL) {
        global.GetFileCache()->ReleaseFile (pszFileName);
        SafeRelease (pcfCachedFile);
    }

    iErrCode = fCachedFile.OpenWrite (pszFileName);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    fCachedFile.Write (pszText == NULL ? "" : pszText);
    fCachedFile.Close();

Cleanup:

    SafeRelease (pcfCachedFile);
    return iErrCode;
}