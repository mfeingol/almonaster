<% #include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

// Almonaster
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

INITIALIZE_EMPIRE

IHttpForm* pHttpForm;
Variant* pvThemeData;

int i, iErrCode;

// Make sure that the unprivileged don't abuse this:
if (m_iPrivilege < ADMINISTRATOR) {
    AddMessage ("You are not authorized to view this page");
    return Redirect (LOGIN);
}

// Handle a submission
if (m_bOwnPost && !m_bRedirection) {

    if (!WasButtonPressed (BID_CANCEL)) {

        int iNumThemes;
        if ((pHttpForm = m_pHttpRequest->GetForm ("NumThemes")) == NULL) {
            goto Redirection;
        }
        iNumThemes = pHttpForm->GetIntValue();

        bool bNewValue, bOldValue;
        int iThemeKey;
        const char* pszNewValue;

        char pszForm[128];

        String strButtonName;

        for (i = 0; i < iNumThemes; i ++) {

            // Get theme key
            sprintf (pszForm, "ThemeKey%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            iThemeKey = pHttpForm->GetIntValue();

            // Check for delete
            sprintf (pszForm, "DeleteTheme%i", i);
            iErrCode = GetButtonName (pszForm, m_iButtonKey, &strButtonName);
            if (iErrCode == OK && m_pHttpRequest->GetForm (strButtonName) != NULL) {

                bRedirectTest = false;

                if (g_pGameEngine->DeleteTheme (iThemeKey) == OK) {
                    AddMessage ("The theme was deleted");
                } else {
                    AddMessage ("The theme could not be deleted");
                }
                continue;
            }

            // Get theme data
            iErrCode = g_pGameEngine->GetThemeData (iThemeKey, &pvThemeData);
            if (iErrCode != OK) {
                continue;
            }

            // Name
            sprintf (pszForm, "Name%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                g_pGameEngine->FreeData (pvThemeData);  goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::Name].GetCharPtr(), pszNewValue) != 0) {
                if (pszNewValue == NULL || strlen (pszNewValue) > MAX_THEME_AUTHOR_NAME_LENGTH) {
                    AddMessage ("You submitted an invalid theme name");
                } else {
                    iErrCode = g_pGameEngine->SetThemeName (iThemeKey, pszNewValue);
                }
            }

            // Version
            sprintf (pszForm, "Version%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                g_pGameEngine->FreeData (pvThemeData);
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::Version].GetCharPtr(), pszNewValue) != 0) {
                if (pszNewValue == NULL || strlen (pszNewValue) > MAX_THEME_VERSION_LENGTH) {
                    AddMessage ("You submitted an invalid theme version");
                } else {
                    iErrCode = g_pGameEngine->SetThemeVersion (iThemeKey, pszNewValue);
                }
            }

            // FileName
            sprintf (pszForm, "File%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                g_pGameEngine->FreeData (pvThemeData);
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::FileName].GetCharPtr(), pszNewValue) != 0) {
                if (pszNewValue == NULL || strlen (pszNewValue) > MAX_THEME_FILE_NAME_LENGTH) {
                    AddMessage ("You submitted an invalid theme file name");
                } else {
                    iErrCode = g_pGameEngine->SetThemeFileName (iThemeKey, pszNewValue);
                }
            }

            // Author's Name
            sprintf (pszForm, "AName%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                g_pGameEngine->FreeData (pvThemeData);  goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::AuthorName].GetCharPtr(), pszNewValue) != 0) {
                if (pszNewValue == NULL || strlen (pszNewValue) > MAX_THEME_AUTHOR_NAME_LENGTH) {
                    AddMessage ("You submitted an invalid author name");
                } else {
                    iErrCode = g_pGameEngine->SetThemeAuthorName (iThemeKey, pszNewValue);
                }
            }

            // Author's Email
            sprintf (pszForm, "AEmail%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                g_pGameEngine->FreeData (pvThemeData);  goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::AuthorEmail].GetCharPtr(), pszNewValue) != 0) {
                if (pszNewValue == NULL || strlen (pszNewValue) > MAX_THEME_AUTHOR_EMAIL_LENGTH) {
                    AddMessage ("You submitted an invalid theme author email");
                } else {
                    iErrCode = g_pGameEngine->SetThemeAuthorEmail (iThemeKey, pszNewValue);
                }
            }

            // Background
            sprintf (pszForm, "BG%i", i);
            bNewValue = m_pHttpRequest->GetForm (pszForm) != NULL;
            bOldValue = (pvThemeData[SystemThemes::Options].GetInteger() & THEME_BACKGROUND) != 0;

            if (bNewValue != bOldValue) {
                iErrCode = g_pGameEngine->SetThemeBackground (iThemeKey, bNewValue);
            }

            // Live Planet
            sprintf (pszForm, "LP%i", i);
            bNewValue = m_pHttpRequest->GetForm (pszForm) != NULL;
            bOldValue = (pvThemeData[SystemThemes::Options].GetInteger() & THEME_LIVE_PLANET) != 0;

            if (bNewValue != bOldValue) {
                iErrCode = g_pGameEngine->SetThemeLivePlanet (iThemeKey, bNewValue);
            }

            // Dead Planet
            sprintf (pszForm, "DP%i", i);
            bNewValue = m_pHttpRequest->GetForm (pszForm) != NULL;
            bOldValue = (pvThemeData[SystemThemes::Options].GetInteger() & THEME_DEAD_PLANET) != 0;

            if (bNewValue != bOldValue) {
                iErrCode = g_pGameEngine->SetThemeDeadPlanet (iThemeKey, bNewValue);
            }

            // Separator
            sprintf (pszForm, "Sep%i", i);
            bNewValue = m_pHttpRequest->GetForm (pszForm) != NULL;
            bOldValue = (pvThemeData[SystemThemes::Options].GetInteger() & THEME_SEPARATOR) != 0;

            if (bNewValue != bOldValue) {
                iErrCode = g_pGameEngine->SetThemeSeparator (iThemeKey, bNewValue);
            }

            // Buttons
            sprintf (pszForm, "Butt%i", i);
            bNewValue = m_pHttpRequest->GetForm (pszForm) != NULL;
            bOldValue = (pvThemeData[SystemThemes::Options].GetInteger() & THEME_BUTTONS) != 0;

            if (bNewValue != bOldValue) {
                iErrCode = g_pGameEngine->SetThemeButtons (iThemeKey, bNewValue);
            }

            // Horz
            sprintf (pszForm, "Horz%i", i);
            bNewValue = m_pHttpRequest->GetForm (pszForm) != NULL;
            bOldValue = (pvThemeData[SystemThemes::Options].GetInteger() & THEME_HORZ) != 0;

            if (bNewValue != bOldValue) {
                iErrCode = g_pGameEngine->SetThemeHorz (iThemeKey, bNewValue);
            }

            // Vert
            sprintf (pszForm, "Vert%i", i);
            bNewValue = m_pHttpRequest->GetForm (pszForm) != NULL;
            bOldValue = (pvThemeData[SystemThemes::Options].GetInteger() & THEME_VERT) != 0;

            if (bNewValue != bOldValue) {
                iErrCode = g_pGameEngine->SetThemeVert (iThemeKey, bNewValue);
            }


            // Text color
            sprintf (pszForm, "TextColor%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::TextColor].GetCharPtr(), pszNewValue) != 0) {

                if (IsColor (pszNewValue)) { 
                    iErrCode = g_pGameEngine->SetThemeTextColor (iThemeKey, pszNewValue);
                } else {
                    AddMessage ("The submitted text color for theme ");
                    AppendMessage (pvThemeData[SystemThemes::Name].GetCharPtr());
                    AppendMessage (" was invalid");
                }
            }

            // Good color
            sprintf (pszForm, "GoodColor%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::GoodColor].GetCharPtr(), pszNewValue) != 0) {

                if (IsColor (pszNewValue)) { 
                    iErrCode = g_pGameEngine->SetThemeGoodColor (iThemeKey, pszNewValue);
                } else {
                    AddMessage ("The submitted good color for theme ");
                    AppendMessage (pvThemeData[SystemThemes::Name].GetCharPtr());
                    AppendMessage (" was invalid");
                }
            }

            // Bad color
            sprintf (pszForm, "BadColor%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::BadColor].GetCharPtr(), pszNewValue) != 0) {

                if (IsColor (pszNewValue)) { 
                    iErrCode = g_pGameEngine->SetThemeBadColor (iThemeKey, pszNewValue);
                } else {
                    AddMessage ("The submitted bad color for theme ");
                    AppendMessage (pvThemeData[SystemThemes::Name].GetCharPtr());
                    AppendMessage (" was invalid");
                }
            }

            // Private color
            sprintf (pszForm, "PrivateColor%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::PrivateMessageColor].GetCharPtr(), pszNewValue) != 0) {

                if (IsColor (pszNewValue)) { 
                    iErrCode = g_pGameEngine->SetThemePrivateMessageColor (iThemeKey, pszNewValue);
                } else {
                    AddMessage ("The submitted private color for theme ");
                    AppendMessage (pvThemeData[SystemThemes::Name].GetCharPtr());
                    AppendMessage (" was invalid");
                }
            }

            // Broadcast color
            sprintf (pszForm, "BroadcastColor%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::BroadcastMessageColor].GetCharPtr(), pszNewValue) != 0) {

                if (IsColor (pszNewValue)) { 
                    iErrCode = g_pGameEngine->SetThemeBroadcastMessageColor (iThemeKey, pszNewValue);
                } else {
                    AddMessage ("The submitted broadcast color for theme ");
                    AppendMessage (pvThemeData[SystemThemes::Name].GetCharPtr());
                    AppendMessage (" was invalid");
                }
            }

            // Table color
            sprintf (pszForm, "TableColor%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::TableColor].GetCharPtr(), pszNewValue) != 0) {

                if (IsColor (pszNewValue)) { 
                    iErrCode = g_pGameEngine->SetThemeTableColor (iThemeKey, pszNewValue);
                } else {
                    AddMessage ("The submitted table color for theme ");
                    AppendMessage (pvThemeData[SystemThemes::Name].GetCharPtr());
                    AppendMessage (" was invalid");
                }
            }

            // Description
            sprintf (pszForm, "Desc%i", i);
            if ((pHttpForm = m_pHttpRequest->GetForm (pszForm)) == NULL) {
                goto Redirection;
            }
            pszNewValue = pHttpForm->GetValue();

            if (String::StrCmp (pvThemeData[SystemThemes::Description].GetCharPtr(), pszNewValue) != 0) {
                if (pszNewValue == NULL || strlen (pszNewValue) > MAX_THEME_DESCRIPTION_LENGTH) {
                    AddMessage ("The submitted description for theme ");
                    AppendMessage (pvThemeData[SystemThemes::Name].GetCharPtr());
                    AppendMessage (" was invalid");
                } else {
                    iErrCode = g_pGameEngine->SetThemeDescription (iThemeKey, pszNewValue);
                }
            }

            g_pGameEngine->FreeData (pvThemeData);
        }

        ///////////////
        // New theme //
        ///////////////

        if (WasButtonPressed (BID_CREATENEWTHEME)) {

            Variant pvSubmitArray[SystemThemes::NumColumns];

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewName")) == NULL ||
                (pszNewValue = pHttpForm->GetValue()) == NULL ||
                *pszNewValue == '\0' ||
                strlen (pszNewValue) > MAX_THEME_NAME_LENGTH) {

                AddMessage ("You must submit a valid theme name");
                goto Redirection;
            } else {
                pvSubmitArray[SystemThemes::Name] = pszNewValue;
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewAName")) == NULL ||
                (pszNewValue = pHttpForm->GetValue()) == NULL ||
                *pszNewValue == '\0' ||
                strlen (pszNewValue) > MAX_THEME_AUTHOR_NAME_LENGTH) {

                AddMessage ("You must submit a valid theme author name");
                goto Redirection;
            } else {
                pvSubmitArray[SystemThemes::AuthorName] = pszNewValue;
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewVersion")) == NULL ||
                (pszNewValue = pHttpForm->GetValue()) == NULL ||
                *pszNewValue == '\0' ||
                strlen (pszNewValue) > MAX_THEME_VERSION_LENGTH) {

                AddMessage ("You must submit a valid theme version");
                goto Redirection;
            } else {
                pvSubmitArray[SystemThemes::Version] = pszNewValue;
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewAEmail")) == NULL ||
                (pszNewValue = pHttpForm->GetValue()) == NULL ||
                *pszNewValue == '\0' ||
                strlen (pszNewValue) > MAX_THEME_AUTHOR_EMAIL_LENGTH) {

                AddMessage ("You must submit a valid theme author email");
                goto Redirection;
            } else {
                pvSubmitArray[SystemThemes::AuthorEmail] = pszNewValue;
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewDesc")) == NULL ||
                (pszNewValue = pHttpForm->GetValue()) == NULL ||
                *pszNewValue == '\0' ||
                strlen (pszNewValue) > MAX_THEME_DESCRIPTION_LENGTH) {

                AddMessage ("You must submit a valid theme description");
                goto Redirection;
            } else {
                pvSubmitArray[SystemThemes::Description] = pszNewValue;
            }

            if ((pHttpForm = m_pHttpRequest->GetForm ("NewFile")) == NULL ||
                (pszNewValue = pHttpForm->GetValue()) == NULL ||
                *pszNewValue == '\0' ||
                strlen (pszNewValue) > MAX_THEME_FILE_NAME_LENGTH) {

                AddMessage ("You must submit a valid theme filename");
                goto Redirection;
            } else {
                pvSubmitArray[SystemThemes::FileName] = pszNewValue;
            }

            int iOptions = 0;

            // Background
            if (m_pHttpRequest->GetForm ("NewBG") != NULL) {
                iOptions |= THEME_BACKGROUND;
            }

            // LivePlanet
            if (m_pHttpRequest->GetForm ("NewLP") != NULL) {
                iOptions |= THEME_LIVE_PLANET;
            }

            // DeadPlanet
            if (m_pHttpRequest->GetForm ("NewDP") != NULL) {
                iOptions |= THEME_DEAD_PLANET;
            }

            // Separator
            if (m_pHttpRequest->GetForm ("NewSep") != NULL) {
                iOptions |= THEME_SEPARATOR;
            }

            // Buttons
            if (m_pHttpRequest->GetForm ("NewButt") != NULL) {
                iOptions |= THEME_BUTTONS;
            }

            // Horz
            if (m_pHttpRequest->GetForm ("NewHorz") != NULL) {
                iOptions |= THEME_HORZ;
            }

            // Vert
            if (m_pHttpRequest->GetForm ("NewVert") != NULL) {
                iOptions |= THEME_VERT;
            }

            pvSubmitArray[SystemThemes::Options] = iOptions;

            const char* pszNewColor;

            // Text Color
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewTextColor")) == NULL) {
                AddMessage ("You must provide a text color for the new theme");
                goto Redirection;
            }
            pszNewColor = pHttpForm->GetValue();
            if (IsColor (pszNewColor)) {
                pvSubmitArray[SystemThemes::TextColor] = pszNewColor;
            } else {
                AddMessage ("You must provide a valid text color for the new theme");
                goto Redirection;
            }

            // Good Color
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewGoodColor")) == NULL) {
                AddMessage ("You must provide a good color for the new theme");
                goto Redirection;
            }
            pszNewColor = pHttpForm->GetValue();
            if (IsColor (pszNewColor)) {
                pvSubmitArray[SystemThemes::GoodColor] = pszNewColor;
            } else {
                AddMessage ("You must provide a valid good color for the new theme");
                goto Redirection;
            }

            // Bad Color
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewBadColor")) == NULL) {
                AddMessage ("You must provide a bad color for the new theme");
                goto Redirection;
            }
            pszNewColor = pHttpForm->GetValue();
            if (IsColor (pszNewColor)) {
                pvSubmitArray[SystemThemes::BadColor] = pszNewColor;
            } else {
                AddMessage ("You must provide a valid bad color for the new theme");
                goto Redirection;
            }

            // Private Color
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewPrivateColor")) == NULL) {
                AddMessage ("You must provide a private color for the new theme");
                goto Redirection;
            }
            pszNewColor = pHttpForm->GetValue();
            if (IsColor (pszNewColor)) {
                pvSubmitArray[SystemThemes::PrivateMessageColor] = pszNewColor;
            } else {
                AddMessage ("You must private a valid private color for the new theme");
                goto Redirection;
            }

            // Broadcast Color
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewBroadcastColor")) == NULL) {
                AddMessage ("You must provide a broadcast color for the new theme");
                goto Redirection;
            }
            pszNewColor = pHttpForm->GetValue();
            if (IsColor (pszNewColor)) {
                pvSubmitArray[SystemThemes::BroadcastMessageColor] = pszNewColor;
            } else {
                AddMessage ("You must provide a valid broadcast color for the new theme");
                goto Redirection;
            }

            // Table Color
            if ((pHttpForm = m_pHttpRequest->GetForm ("NewTableColor")) == NULL) {
                AddMessage ("You must provide a table color for the new theme");
                goto Redirection;
            }
            pszNewColor = pHttpForm->GetValue();
            if (IsColor (pszNewColor)) {
                pvSubmitArray[SystemThemes::TableColor] = pszNewColor;
            } else {
                AddMessage ("You must provide a valid table color for the new theme");
                goto Redirection;
            }

            unsigned int iKey;
            if ((iErrCode = g_pGameEngine->CreateTheme (pvSubmitArray, &iKey)) == OK) {
                AddMessage ("The theme has been created with directory key ");
                AppendMessage (iKey);
            } else {
                if (iErrCode == ERROR_THEME_ALREADY_EXISTS) {
                    AddMessage ("The theme already exists");
                } else {
                    AddMessage ("The theme could not be created");
                }
            }
        }
    }
}

SYSTEM_REDIRECT_ON_SUBMIT

SYSTEM_OPEN (false)

// Individual page stuff starts here
int* piThemeKey, iNumThemes;
Check (g_pGameEngine->GetThemeKeys (&piThemeKey, &iNumThemes));

%><input type="hidden" name="NumThemes" value="<% Write (iNumThemes); %>"><p>There <% 
if (iNumThemes == 1) { 
    %>is <strong>1</strong> system theme:<% 
} else { 
    %>are <strong><% Write (iNumThemes); %></strong> system themes<% 
}

if (iNumThemes > 0) { 
    %>:<p><table width="90%"><%

    int iOptions;
    Variant* pvThemeData;

    char pszDeleteTheme [256];

    for (i = 0; i < iNumThemes; i ++) {

        if (g_pGameEngine->GetThemeData (piThemeKey[i], &pvThemeData) != OK) {
            continue;
        }

        %><tr><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="left">Key</th><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Theme Name</th><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Version</th><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Filename</th><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Author's Name</th><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Author's Email</th><%
        %></tr><%

        %><input type="hidden" name="ThemeKey<% Write (i); %>" value="<% Write (piThemeKey[i]); %>"><%

        %><tr><%
        %><td><% Write (piThemeKey [i]); %></td><%

        %><td><%
        %><input type="text" size="28" maxlength="<% Write (MAX_THEME_AUTHOR_NAME_LENGTH); %>" name="Name<%
        Write (i); %>" value="<% Write (pvThemeData[SystemThemes::Name].GetCharPtr()); %>"></td><%

        %><td><input type="text" size="6" maxlength="10" name="Version<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::Version].GetCharPtr()); %>"></td><%

        %><td><input type="text" size="17" maxlength="25" name="File<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::FileName].GetCharPtr()); %>"></td><%

        %><td><input type="text" size="27" maxlength="50" name="AName<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::AuthorName].GetCharPtr()); %>"></td><%

        %><td><input type="text" size="22" maxlength="50" name="AEmail<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::AuthorEmail].GetCharPtr()); %>"></td><%

        %></tr><tr><%

        %><td>&nbsp;</td><%
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Background:</th><%
        %><td><input type="checkbox"<%

        iOptions = pvThemeData[SystemThemes::Options].GetInteger();

        if (iOptions & THEME_BACKGROUND) {
            %> checked <%
        }
        %> name="BG<% Write (i); %>"></td><%

        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" rowspan="2" align="center"><%
        %>Description:</th><%

        %><td rowspan="2" colspan="2" align="left"><textarea rows="3" cols="50" wrap="virtual" name="Desc<%
        Write (i); %>"><% Write (pvThemeData[SystemThemes::Description].GetCharPtr()); 
        %></textarea></td><%
        %></tr><%

        %><tr><td>&nbsp;</td><th bgcolor="<% 
        Write (m_vTableColor.GetCharPtr()); 
        %>" align="center">Live Planet:</th><td><input type="checkbox"<%

        if (iOptions & THEME_LIVE_PLANET) {
            %> checked <%
        }
        %> name="LP<% Write (i); %>"></td><%

        %></tr><tr><%
        %><td>&nbsp;</td><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); 
        %>" align="center">Dead Planet:</th><td><input type="checkbox"<%

        if (iOptions & THEME_DEAD_PLANET) {
            %> checked <%
        } %> name="DP<% Write (i); %>"></td><%

        // Text color
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Text Color:</th><%
        %><td align="left" bgcolor="<% Write (pvThemeData[SystemThemes::TextColor].GetCharPtr()); %>"><%
        %><input type="text" size="6" maxlength="6" name="TextColor<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::TextColor].GetCharPtr()); %>"></td><%

        // Separator
        %><tr><td>&nbsp;</td><th bgcolor="<% 
        Write (m_vTableColor.GetCharPtr()); %>" align="center">Separator:</th><%
        %><td><input type="checkbox"<%

        if (iOptions & THEME_SEPARATOR) {
            %> checked <%
        } %> name="Sep<% Write (i); %>"></td><%

        // Good color
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Good Color:</th><%
        %><td align="left" bgcolor="<% Write (pvThemeData[SystemThemes::GoodColor].GetCharPtr()); %>"><%
        %><input type="text" size="6" maxlength="6" name="GoodColor<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::GoodColor].GetCharPtr()); %>"></td><%

        %></tr><tr><%

        // Buttons
        %><td>&nbsp;</td><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" <%
        %>align="center">Buttons:</td><td><input type="checkbox"<%

        if (iOptions & THEME_BUTTONS) {
            %> checked <%
        } %> name="Butt<% Write (i); %>"></td><%

        // Bad color
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Bad Color:</th><%
        %><td align="left" bgcolor="<% Write (pvThemeData[SystemThemes::BadColor].GetCharPtr()); %>"><%
        %><input type="text" size="6" maxlength="6" name="BadColor<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::BadColor].GetCharPtr()); %>"></td><%

        %></tr><tr><%

        // Horz
        %><td>&nbsp;</td><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" <%
        %>align="center">Horizontal Link:</td><td><input type="checkbox"<%

        if (iOptions & THEME_HORZ) {
            %> checked <%
        }
        %> name="Horz<% Write (i); %>"></td><%

        // Private color
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Private Color:</th><%
        %><td align="left" bgcolor="<% Write (pvThemeData[SystemThemes::PrivateMessageColor].GetCharPtr()); %>"><%
        %><input type="text" size="6" maxlength="6" name="PrivateColor<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::PrivateMessageColor].GetCharPtr()); %>"></td><%

        %></tr><tr><%

        // Vert
        %><td>&nbsp;</td><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" <%
        %>align="center">Vertical Link:</td><td><input type="checkbox"<%

        if (iOptions & THEME_VERT) {
            %> checked <%
        }
        %> name="Vert<% Write (i); %>"></td><%

        // Broadcast color
        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Broadcast Color:</th><%
        %><td align="left" bgcolor="<% Write (pvThemeData[SystemThemes::BroadcastMessageColor].GetCharPtr()); %>"><%
        %><input type="text" size="6" maxlength="6" name="BroadcastColor<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::BroadcastMessageColor].GetCharPtr()); %>"></td><%

        %></tr><tr><td></td><td></td><td></td><%

        %><th bgcolor="<% Write (m_vTableColor.GetCharPtr()); %>" align="center">Table Color:</th><%
        %><td align="left" bgcolor="<% Write (pvThemeData[SystemThemes::TableColor].GetCharPtr()); %>"><%
        %><input type="text" size="6" maxlength="6" name="TableColor<% Write (i); %>" <%
        %>value="<% Write (pvThemeData[SystemThemes::TableColor].GetCharPtr()); %>"></td><%

        // Delete theme
        %><td align="center"><%

        sprintf (pszDeleteTheme, "DeleteTheme%i", i);
        WriteButtonString (m_iButtonKey, "DeleteTheme", "Delete Theme", pszDeleteTheme); %></td><%

        // Space between themes
        %></tr><tr><td>&nbsp;</td></tr><%

        g_pGameEngine->FreeData (pvThemeData);
    }
    %></table><%

    g_pGameEngine->FreeKeys (piThemeKey);
}

%><p><h3>Create a new theme:</h3><%
%><p><table width="75%"><%
%><tr><%
%><td>Name:</td><%
%><td><input type="text" size="20" maxlength="<% Write (MAX_THEME_NAME_LENGTH); %>" name="NewName"></td><%
%></tr><%
%><tr><%
%><td>Author's Name:</td><%
%><td><input type="text" size="20" maxlength="<% Write (MAX_THEME_AUTHOR_NAME_LENGTH); %>" name="NewAName"></td><%
%></tr><%
%><tr><%
%><td>Version:</td><%
%><td><input type="text" size="<% Write (MAX_THEME_VERSION_LENGTH); %>" maxlength="<% Write (MAX_THEME_VERSION_LENGTH); %>" name="NewVersion"></td><%
%></tr><%
%><tr><%
%><td>Author's Email:</td><%
%><td><input type="text" size="20" maxlength="<% Write (MAX_THEME_AUTHOR_EMAIL_LENGTH); %>" name="NewAEmail"></td><%
%></tr><%
%><tr><%
%><td>File Name:</td><%
%><td><input type="text" size="20" maxlength="<% Write (MAX_THEME_FILE_NAME_LENGTH); %>" name="NewFile"></td><%
%></tr><%
%><tr><%
%><td>Background:</td><%
%><td><input type="checkbox" name="NewBG"></td><%
%></tr><%
%><tr><%
%><td>Live Planet:</td><%
%><td><input type="checkbox" name="NewLP"></td><%
%></tr><%
%><tr><%
%><td>Dead Planet:</td><%
%><td><input type="checkbox" name="NewDP"></td><%
%></tr><%
%><tr><%
%><td>Separator:</td><%
%><td><input type="checkbox" name="NewSep"></td><%
%></tr><%
%><tr><%
%><td>Buttons:</td><%
%><td><input type="checkbox" name="NewButt"></td><%
%></tr><%
%><tr><%
%><td>Horizontal Link Bar:</td><%
%><td><input type="checkbox" name="NewHorz"></td><%
%></tr><%
%><tr><%
%><td>Vertical Link Bar:</td><%
%><td><input type="checkbox" name="NewVert"></td><%
%></tr><%
%><tr><%
%><td>Text Color:</td><%
%><td><input type="text" size="6" maxlength="6" name="NewTextColor"></td><%
%></tr><%
%><tr><%
%><td>Good Color:</td><%
%><td><input type="text" size="6" maxlength="6" name="NewGoodColor"></td><%
%></tr><%
%><tr><%
%><td>Bad Color:</td><%
%><td><input type="text" size="6" maxlength="6" name="NewBadColor"></td><%
%></tr><%
%><tr><%
%><td>Private Color:</td><%
%><td><input type="text" size="6" maxlength="6" name="NewPrivateColor"></td><%
%></tr><%
%><tr><%
%><td>Broadcast Color:</td><%
%><td><input type="text" size="6" maxlength="6" name="NewBroadcastColor"></td><%
%></tr><%
%><tr><%
%><td>Table Color:</td><%
%><td><input type="text" size="6" maxlength="6" name="NewTableColor"></td><%
%></tr><%
%><tr><%
%><td>Description:</td><%
%><td><textarea rows="3" cols="40" wrap="virtual" name="NewDesc"></textarea></td><%
%></tr><%
%></table><%
%><p><%

WriteButton (BID_CANCEL);
WriteButton (BID_CREATENEWTHEME);

SYSTEM_CLOSE %>