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

#include "Osal/File.h"

void HtmlRenderer::WriteBackgroundImageSrc (int iThemeKey) {
    
    // No Null Theme background image
    OutputText (BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iThemeKey);
    OutputText ("/" BACKGROUND_IMAGE);
}

void HtmlRenderer::WriteLivePlanetImageSrc (int iThemeKey) {
    
    if (iThemeKey == NULL_THEME) {
        OutputText (BASE_RESOURCE_DIR LIVE_PLANET_NAME);
    } else {
        OutputText (BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iThemeKey);
        OutputText ("/" LIVE_PLANET_NAME);
    }
}

void HtmlRenderer::WriteDeadPlanetImageSrc (int iThemeKey) {
    
    if (iThemeKey == NULL_THEME) {
        OutputText (BASE_RESOURCE_DIR DEAD_PLANET_NAME);
    } else {
        OutputText (BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iThemeKey);
        OutputText ("/" DEAD_PLANET_NAME);
    }
}

void HtmlRenderer::WriteSeparatorSrc (int iThemeKey) {
    
    // No Null Theme separator
    
    OutputText (BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iThemeKey);
    OutputText ("/" SEPARATOR_IMAGE);
}

void HtmlRenderer::WriteHorzSrc (int iThemeKey) {
    
    if (iThemeKey == NULL_THEME) {
        OutputText (BASE_RESOURCE_DIR HORZ_LINE_NAME);
    } else {
        OutputText (BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iThemeKey);
        OutputText ("/" HORZ_LINE_NAME);
    }
}

void HtmlRenderer::WriteVertSrc (int iThemeKey) {
    
    if (iThemeKey == NULL_THEME) {
        OutputText (BASE_RESOURCE_DIR VERT_LINE_NAME);
    } else {
        OutputText (BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iThemeKey);
        OutputText ("/" VERT_LINE_NAME);
    }
}

void HtmlRenderer::GetHorzString (int iThemeKey, String* pstrString, bool bBlowup) {
    
    switch (iThemeKey) {
        
    case NULL_THEME:
        
        if (bBlowup) {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR HORZ_LINE_NAME "\">";
        }
        break;
        
    case ALTERNATIVE_PATH:
        
        *pstrString = "<img src=\"";
        *pstrString += m_vLocalPath.GetCharPtr();
        
        if (bBlowup) {
            *pstrString += "/" HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString += "/" HORZ_LINE_NAME "\">";
        }
        break;
        
    default:
        
        if (bBlowup) {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR;
            *pstrString += iThemeKey;
            *pstrString += "/" HORZ_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR;
            *pstrString += iThemeKey;
            *pstrString += "/" HORZ_LINE_NAME "\">";
        }
        break;
    }
    
    Assert(pstrString->GetCharPtr());
}

void HtmlRenderer::GetVertString (int iThemeKey, String* pstrString, bool bBlowup) {
    
    switch (iThemeKey) {
        
    case NULL_THEME:
        
        if (bBlowup) {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString = "<img src=\"" BASE_RESOURCE_DIR VERT_LINE_NAME "\">";
        }
        break;
        
    case ALTERNATIVE_PATH:
        
        *pstrString = "<img src=\"";
        *pstrString += m_vLocalPath.GetCharPtr();
        
        if (bBlowup) {
            *pstrString += "/" VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString += "/" VERT_LINE_NAME "\">";
        }
        break;
        
    default:
        
        *pstrString = "<img src=\"" BASE_RESOURCE_DIR;
        *pstrString += iThemeKey;
        
        if (bBlowup) {
            *pstrString += "/" VERT_LINE_NAME "\" width=\"21\" height=\"3\">";
        } else {
            *pstrString += "/" VERT_LINE_NAME "\">";
        }
        break;
    }
    
    Assert(pstrString->GetCharPtr());
}

int HtmlRenderer::GetUIData (int iThemeKey) {
    
    int iErrCode = OK;
    Variant vValue;
    
    if (iThemeKey == INDIVIDUAL_ELEMENTS) {
        
        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIButtons, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_iButtonKey = vValue.GetInteger();
        
        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIBackground, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_iBackgroundKey = vValue.GetInteger();
        
        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UISeparator, &vValue);
        RETURN_ON_ERROR(iErrCode);
        m_iSeparatorKey = vValue.GetInteger();

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetTextColorData (vValue.GetInteger());
        RETURN_ON_ERROR(iErrCode);
        
    } else {
        
        m_iButtonKey = iThemeKey;
        m_iBackgroundKey = iThemeKey;
        m_iSeparatorKey = iThemeKey;
        
        if (iThemeKey == NULL_THEME) {
            
            iErrCode = GetTextColorData (NULL_THEME);
            RETURN_ON_ERROR(iErrCode);
        }
        
        else if (iThemeKey == ALTERNATIVE_PATH) {

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::UIColor, &vValue);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = GetTextColorData (vValue.GetInteger());
            RETURN_ON_ERROR(iErrCode);

            iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::AlternativeGraphicsPath, &m_vLocalPath);
            RETURN_ON_ERROR(iErrCode);
        }
        
        else {
            
            iErrCode = GetTextColorData (iThemeKey);
            RETURN_ON_ERROR(iErrCode);
        }
    }
    
    return iErrCode;
}


int HtmlRenderer::GetTextColorData (int iEmpireColorKey) {
    
    int iErrCode = OK;
    
    switch (iEmpireColorKey) {
        
    case NULL_THEME:
        
        m_vTableColor = DEFAULT_TABLE_COLOR;
        m_vTextColor = DEFAULT_TEXT_COLOR;
        m_vGoodColor = DEFAULT_GOOD_COLOR;
        m_vBadColor = DEFAULT_BAD_COLOR;
        m_vPrivateMessageColor = DEFAULT_PRIVATE_MESSAGE_COLOR;
        m_vBroadcastMessageColor = DEFAULT_BROADCAST_MESSAGE_COLOR;
        break;
        
    case CUSTOM_COLORS:

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTableColor, &m_vTableColor);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomTextColor, &m_vTextColor);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomGoodColor, &m_vGoodColor);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBadColor, &m_vBadColor);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomPrivateMessageColor, &m_vPrivateMessageColor);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireProperty (m_iEmpireKey, SystemEmpireData::CustomBroadcastMessageColor, &m_vBroadcastMessageColor);
        RETURN_ON_ERROR(iErrCode);

        break;
        
    default:
        
        iErrCode = GetThemeTextColor (iEmpireColorKey, &m_vTextColor);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetThemeGoodColor (iEmpireColorKey, &m_vGoodColor);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetThemeBadColor (iEmpireColorKey, &m_vBadColor);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetThemePrivateMessageColor (iEmpireColorKey, &m_vPrivateMessageColor);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetThemeBroadcastMessageColor (iEmpireColorKey, &m_vBroadcastMessageColor);
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetThemeTableColor (iEmpireColorKey, &m_vTableColor);
        RETURN_ON_ERROR(iErrCode);
        break;
    }
    
    return iErrCode;
}

bool HtmlRenderer::IsColor (const char* pszColor) {
    
    if (String::StrLen (pszColor) != MAX_COLOR_LENGTH) {
        return false;
    }
    
    int i;
    char szChar;
    
    for (i = 0; i < MAX_COLOR_LENGTH; i ++) {
        
        szChar = pszColor[i];
        
        if (!(szChar >= '0' && szChar <= '9') &&
            !(szChar >= 'a' && szChar <= 'f') && 
            !(szChar >= 'A' && szChar <= 'F')
            ) {
            return false;
        }
    }
    
    return true;
}


void HtmlRenderer::WriteButtonImageSrc (int iRealThemeKey, const char* pszButtonName) {
    
    m_pHttpResponse->WriteText (BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iRealThemeKey);
    OutputText ("/");
    m_pHttpResponse->WriteText (pszButtonName);
    m_pHttpResponse->WriteText (DEFAULT_IMAGE_EXTENSION);
}

void HtmlRenderer::WriteThemeDownloadSrc (int iRealThemeKey, const char* pszFileName) {
    
    m_pHttpResponse->WriteText (BASE_RESOURCE_DIR);
    m_pHttpResponse->WriteText (iRealThemeKey);
    OutputText ("/");
    m_pHttpResponse->WriteText (pszFileName);
}

void HtmlRenderer::WriteAlmonasterBanner() {
    
    OutputText ("<img alt=\"Almonaster\" align=\"center\" src=\"" BASE_RESOURCE_DIR ALMONASTER_BANNER_IMAGE "\">");
}

void HtmlRenderer::WriteSeparatorString (int iSeparatorKey) {
    
    switch (iSeparatorKey) {
        
    case NULL_THEME:
        OutputText (DEFAULT_SEPARATOR_STRING);
        break;
        
    case ALTERNATIVE_PATH:
        
        OutputText ("<img src=\"");
        m_pHttpResponse->WriteText (m_vLocalPath.GetCharPtr());
        OutputText ("/" SEPARATOR_IMAGE "\" width=\"90%\" height=\"16\">");
        break;
        
    default:
        
        OutputText ("<img src=\"" BASE_RESOURCE_DIR);
        m_pHttpResponse->WriteText (iSeparatorKey);     
        OutputText ("/" SEPARATOR_IMAGE "\" width=\"90%\" height=\"16\">");
        break;
    }
}


void HtmlRenderer::WriteEmpireIcon (int iIconKey, int iEmpireKey, const char* pszAlt, bool bVerifyUpload) {

    WriteIcon (iIconKey, iEmpireKey, NO_KEY, pszAlt, BASE_UPLOADED_ALIEN_DIR, bVerifyUpload);
}


void HtmlRenderer::WriteIcon (int iIconKey, int iEntityKey, int iEntityKey2,
                              const char* pszAlt, const char* pszUploadDir, bool bVerifyUpload) {
    
    if (iIconKey != UPLOADED_ICON) {

        OutputText ("<img src=\"" BASE_RESOURCE_DIR BASE_ALIEN_DIR ALIEN_NAME);

    } else {
        
        bool bDisplay = true;

        if (m_iSystemOptions2 & BLOCK_UPLOADED_ICONS) {
            bDisplay = false;
        }
        
        else if (bVerifyUpload) {
            
            // Make sure file exists
            char pszDestFileName[OS::MaxFileNameLength];

            if (iEntityKey2 != NO_KEY) {

                sprintf (
                    pszDestFileName, 
                    "%s/%s/" ALIEN_NAME "%i.%i" DEFAULT_IMAGE_EXTENSION, 
                    global.GetResourceDir(),
                    pszUploadDir,
                    iEntityKey,
                    iEntityKey2
                    );

            } else {
            
                sprintf (
                    pszDestFileName, 
                    "%s/%s/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
                    global.GetResourceDir(),
                    pszUploadDir,
                    iEntityKey
                    );
            }
            
            if (!File::DoesFileExist (pszDestFileName)) {
                bDisplay = false;
            }
        }
        
        if (bDisplay) {
            
            OutputText ("<img src=\"" BASE_RESOURCE_DIR);
            m_pHttpResponse->WriteText (pszUploadDir);
            OutputText ("/" ALIEN_NAME);

            if (iEntityKey2 != NO_KEY) {
                m_pHttpResponse->WriteText (iEntityKey);
                OutputText (".");
                iIconKey = iEntityKey2;
            } else {
                iIconKey = iEntityKey;
            }
            
        } else {
            
            // Render default icon
            EnsureDefaultSystemIcon();
            iIconKey = m_iDefaultSystemIcon;
            
            OutputText ("<img src=\"" BASE_RESOURCE_DIR BASE_ALIEN_DIR ALIEN_NAME);
        }
    }
    
    m_pHttpResponse->WriteText (iIconKey);
    OutputText (DEFAULT_IMAGE_EXTENSION "\"");
    
    if (!String::IsBlank (pszAlt)) {
        OutputText (" alt=\"");
        m_pHttpResponse->WriteText (pszAlt);
        OutputText ("\"");
    }
    
    OutputText (">");
}

void HtmlRenderer::WriteProfileAlienString (int iAlienKey, int iEmpireKey, 
                                            const char* pszEmpireName, int iBorder, const char* pszFormName, 
                                            const char* pszAlt, bool bVerifyUpload, bool bKeyAndHash) {
    
    OutputText ("<input type=\"image\" border=\"");
    m_pHttpResponse->WriteText (iBorder);
    OutputText ("\" src=\"" BASE_RESOURCE_DIR);
    
    if (iAlienKey == UPLOADED_ICON) {

        Assert(iEmpireKey != NO_KEY);

        bool bDisplay = true;
        
        if (m_iSystemOptions2 & BLOCK_UPLOADED_ICONS) {
            bDisplay = false;
        }

        else if (bVerifyUpload) {

            // Make sure file exists
            char pszDestFileName[OS::MaxFileNameLength];
            
            sprintf (
                pszDestFileName, 
                "%s/" BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
                global.GetResourceDir(),
                iEmpireKey
                );
            
            if (!File::DoesFileExist (pszDestFileName)) {
                bDisplay = false;
            }
        }
        
        if (bDisplay) {
            
            OutputText (BASE_UPLOADED_ALIEN_DIR "/" ALIEN_NAME);
            m_pHttpResponse->WriteText (iEmpireKey);
            OutputText (DEFAULT_IMAGE_EXTENSION);
            
        } else {
            
            // Render default icon
            OutputText (BASE_ALIEN_DIR ALIEN_NAME);
            EnsureDefaultSystemIcon();
            m_pHttpResponse->WriteText(m_iDefaultSystemIcon);
            OutputText (DEFAULT_IMAGE_EXTENSION);
        }
        
    } else {
        
        OutputText (BASE_ALIEN_DIR ALIEN_NAME);
        m_pHttpResponse->WriteText (iAlienKey);
        OutputText (DEFAULT_IMAGE_EXTENSION);
    }
    
    OutputText ("\" name=\"");
    m_pHttpResponse->WriteText (pszFormName);
    
    if (bKeyAndHash) {
        OutputText (".");
        m_pHttpResponse->WriteText (iEmpireKey);
        OutputText (".");
        m_pHttpResponse->WriteText (Algorithm::GetStringHashValue (pszEmpireName, EMPIRE_NAME_HASH_LIMIT, true));
    }
    
    OutputText ("\" alt=\"");
    m_pHttpResponse->WriteText (pszAlt);
    OutputText ("\">");
}

struct GifHeader
{
  // Header
  char Signature[3];     // Header Signature (always "GIF")
  char Version[3];       // GIF format version("87a" or "89a")

  short ScreenWidth;      // Width of Display Screen in Pixels
  short ScreenHeight;     // Height of Display Screen in Pixels
  char Packed;           // Screen and Color Map Information
  char BackgroundColor;  // Background Color Index
  char AspectRatio;      // Pixel Aspect Ratio
};

int HtmlRenderer::VerifyGIF(const char* pszFileName, bool* pbGoodGIF)
{
    int iErrCode;

    *pbGoodGIF = false;

    File fGifFile;
    if (fGifFile.OpenRead (pszFileName) != OK)
    {
        AddMessage("The uploaded file cannot be opened");
        return OK;
    }
    
    GifHeader header;
    size_t stNumBytes, stSize;
    
    // Read the gif header
    iErrCode = fGifFile.Read(&header, sizeof(GifHeader), &stNumBytes);
    fGifFile.Close();

    if (iErrCode != OK)
    {
        AddMessage ("The uploaded file cannot be read");
        return OK;
    }

    if (stNumBytes != sizeof (GifHeader))
    {
        AddMessage ("The uploaded file is too small");
        return OK;
    }

    iErrCode = File::GetFileSize (pszFileName, &stSize);
    RETURN_ON_ERROR(iErrCode);
    
    Variant vMaxIconSize;
    iErrCode = GetSystemProperty (SystemData::MaxIconSize, &vMaxIconSize);
    RETURN_ON_ERROR(iErrCode);
    
    if (stSize > (size_t)vMaxIconSize.GetInteger())
    {
        char pszError[256];
        sprintf (
            pszError, 
            "The uploaded file is larger than the upper limit (%i KB)", 
            (int) (vMaxIconSize.GetInteger() / 1024)
            );
        AddMessage (pszError);
        return OK;
    }

    // Ensure gif89a
    if (header.Signature[0] != 'G' ||
        header.Signature[1] != 'I' ||
        header.Signature[2] != 'F' ||
        header.Version[0] != '8' ||
        header.Version[1] != '9' ||
        header.Version[2] != 'a')
    {
        AddMessage("The uploaded file is not in GIF89a format");
        return OK;
    }
    
    // Get size of image
    if (header.ScreenWidth != ICON_WIDTH || header.ScreenHeight != ICON_HEIGHT)
    {
        char pszError[256];
        sprintf (
            pszError, 
            "The uploaded GIF89a icon does not have the correct dimensions (%i x %i)",
            ICON_WIDTH,
            ICON_HEIGHT
            );
        
        AddMessage (pszError);
        return OK;
    }

    *pbGoodGIF = true;
    
    return iErrCode;
}

bool HtmlRenderer::CopyUploadedIcon(const char* pszFileName, const char* pszUploadDir, int iKey1, int iKey2) {
    
    int iErrCode;
    char pszDestFileName[OS::MaxFileNameLength];

    Assert(iKey1 != NO_KEY);

    if (iKey2 == NO_KEY) {
    
        sprintf (
            pszDestFileName, 
            "%s/%s/" ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION,
            global.GetResourceDir(),
            pszUploadDir,
            iKey1
            );

    } else {

        sprintf (
            pszDestFileName, 
            "%s/%s/" ALIEN_NAME "%i.%i" DEFAULT_IMAGE_EXTENSION,
            global.GetResourceDir(),
            pszUploadDir,
            iKey1,
            iKey2
            );
    }       

    // TODO:  need a better solution for this
    unsigned int iTries = 0;
    while (iTries < 20)
    {
        global.GetFileCache()->ReleaseFile(pszDestFileName);
        
        iErrCode = File::CopyFile(pszFileName, pszDestFileName);
        if (iErrCode == OK)
        {
            return true;
        }
        
        iTries ++;
        OS::Sleep(100);
    }

    return false;
}

bool HtmlRenderer::CopyNewAlien(const char* pszFileName, int iAlienKey)
{
    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_ALIEN_DIR ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
        global.GetResourceDir(),
        iAlienKey
        );
    
    global.GetFileCache()->ReleaseFile (pszDestFileName);
    
    return File::CopyFile (pszFileName, pszDestFileName) == OK;
}

bool HtmlRenderer::DeleteAlien (int iAlienKey)
{
    char pszDestFileName[OS::MaxFileNameLength];
    
    sprintf (
        pszDestFileName, 
        "%s/" BASE_ALIEN_DIR ALIEN_NAME "%i" DEFAULT_IMAGE_EXTENSION, 
        global.GetResourceDir(),
        iAlienKey
        );
    
    global.GetFileCache()->ReleaseFile (pszDestFileName);
    
    return File::DeleteFile (pszDestFileName) == OK;
}

int HtmlRenderer::EnsureDefaultSystemIcon()
{
    int iErrCode = OK;

    if (m_iDefaultSystemIcon == NO_KEY)
    {
        int iErrCode;
        Variant vAlien;

        iErrCode = GetSystemProperty(SystemData::DefaultAlien, &vAlien);
        RETURN_ON_ERROR(iErrCode);

        m_iDefaultSystemIcon = vAlien.GetInteger();
    }

    return iErrCode;
}