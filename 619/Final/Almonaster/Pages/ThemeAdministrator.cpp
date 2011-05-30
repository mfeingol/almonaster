
#include "../Almonaster.h"
#include "../GameEngine/GameEngine.h"

#include <stdio.h>

#include "../HtmlRenderer/HtmlRenderer.h"

#define Write m_pHttpResponse->WriteText

// Render the ThemeAdministrator page
int HtmlRenderer::Render_ThemeAdministrator() {

	// Almonaster 2.0
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

				int iKey;
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


	Write ("<input type=\"hidden\" name=\"NumThemes\" value=\"", sizeof ("<input type=\"hidden\" name=\"NumThemes\" value=\"") - 1);
	Write (iNumThemes); 
	Write ("\"><p>There ", sizeof ("\"><p>There ") - 1);
	if (iNumThemes == 1) { 
		
	Write ("is <strong>1</strong> system theme:", sizeof ("is <strong>1</strong> system theme:") - 1);
	} else { 
		
	Write ("are <strong>", sizeof ("are <strong>") - 1);
	Write (iNumThemes); 
	Write ("</strong> system themes", sizeof ("</strong> system themes") - 1);
	}

	if (iNumThemes > 0) { 
		
	Write (":<p><table width=\"90%\">", sizeof (":<p><table width=\"90%\">") - 1);
	int iOptions;
		Variant* pvThemeData;

		char pszDeleteTheme [256];

		for (i = 0; i < iNumThemes; i ++) {

			if (g_pGameEngine->GetThemeData (piThemeKey[i], &pvThemeData) != OK) {
				continue;
			}

			
	Write ("<tr><th bgcolor=\"", sizeof ("<tr><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"left\">Key</th><th bgcolor=\"", sizeof ("\" align=\"left\">Key</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Theme Name</th><th bgcolor=\"", sizeof ("\" align=\"center\">Theme Name</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Version</th><th bgcolor=\"", sizeof ("\" align=\"center\">Version</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Filename</th><th bgcolor=\"", sizeof ("\" align=\"center\">Filename</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Author's Name</th><th bgcolor=\"", sizeof ("\" align=\"center\">Author's Name</th><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Author's Email</th></tr><input type=\"hidden\" name=\"ThemeKey", sizeof ("\" align=\"center\">Author's Email</th></tr><input type=\"hidden\" name=\"ThemeKey") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (piThemeKey[i]); 
	Write ("\"><tr><td>", sizeof ("\"><tr><td>") - 1);
	Write (piThemeKey [i]); 
	Write ("</td><td><input type=\"text\" size=\"28\" maxlength=\"", sizeof ("</td><td><input type=\"text\" size=\"28\" maxlength=\"") - 1);
	Write (MAX_THEME_AUTHOR_NAME_LENGTH); 
	Write ("\" name=\"Name", sizeof ("\" name=\"Name") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::Name].GetCharPtr()); 
	Write ("\"></td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"Version", sizeof ("\"></td><td><input type=\"text\" size=\"6\" maxlength=\"10\" name=\"Version") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::Version].GetCharPtr()); 
	Write ("\"></td><td><input type=\"text\" size=\"17\" maxlength=\"25\" name=\"File", sizeof ("\"></td><td><input type=\"text\" size=\"17\" maxlength=\"25\" name=\"File") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::FileName].GetCharPtr()); 
	Write ("\"></td><td><input type=\"text\" size=\"27\" maxlength=\"50\" name=\"AName", sizeof ("\"></td><td><input type=\"text\" size=\"27\" maxlength=\"50\" name=\"AName") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::AuthorName].GetCharPtr()); 
	Write ("\"></td><td><input type=\"text\" size=\"22\" maxlength=\"50\" name=\"AEmail", sizeof ("\"></td><td><input type=\"text\" size=\"22\" maxlength=\"50\" name=\"AEmail") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::AuthorEmail].GetCharPtr()); 
	Write ("\"></td></tr><tr><td>&nbsp;</td><th bgcolor=\"", sizeof ("\"></td></tr><tr><td>&nbsp;</td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Background:</th><td><input type=\"checkbox\"", sizeof ("\" align=\"center\">Background:</th><td><input type=\"checkbox\"") - 1);
	iOptions = pvThemeData[SystemThemes::Options].GetInteger();

			if (iOptions & THEME_BACKGROUND) {
				
	Write (" checked ", sizeof (" checked ") - 1);
	}
			
	Write (" name=\"BG", sizeof (" name=\"BG") - 1);
	Write (i); 
	Write ("\"></td><th bgcolor=\"", sizeof ("\"></td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" rowspan=\"2\" align=\"center\">Description:</th><td rowspan=\"2\" colspan=\"2\" align=\"left\"><textarea rows=\"3\" cols=\"50\" wrap=\"physical\" name=\"Desc", sizeof ("\" rowspan=\"2\" align=\"center\">Description:</th><td rowspan=\"2\" colspan=\"2\" align=\"left\"><textarea rows=\"3\" cols=\"50\" wrap=\"physical\" name=\"Desc") - 1);
	Write (i); 
	Write ("\">", sizeof ("\">") - 1);
	Write (pvThemeData[SystemThemes::Description].GetCharPtr()); 
			
	Write ("</textarea></td></tr><tr><td>&nbsp;</td><th bgcolor=\"", sizeof ("</textarea></td></tr><tr><td>&nbsp;</td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
			
	Write ("\" align=\"center\">Live Planet:</th><td><input type=\"checkbox\"", sizeof ("\" align=\"center\">Live Planet:</th><td><input type=\"checkbox\"") - 1);
	if (iOptions & THEME_LIVE_PLANET) {
				
	Write (" checked ", sizeof (" checked ") - 1);
	}
			
	Write (" name=\"LP", sizeof (" name=\"LP") - 1);
	Write (i); 
	Write ("\"></td></tr><tr><td>&nbsp;</td><th bgcolor=\"", sizeof ("\"></td></tr><tr><td>&nbsp;</td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
			
	Write ("\" align=\"center\">Dead Planet:</th><td><input type=\"checkbox\"", sizeof ("\" align=\"center\">Dead Planet:</th><td><input type=\"checkbox\"") - 1);
	if (iOptions & THEME_DEAD_PLANET) {
				
	Write (" checked ", sizeof (" checked ") - 1);
	} 
	Write (" name=\"DP", sizeof (" name=\"DP") - 1);
	Write (i); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	// Text color
			
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Text Color:</th><td align=\"left\" bgcolor=\"", sizeof ("\" align=\"center\">Text Color:</th><td align=\"left\" bgcolor=\"") - 1);
	Write (pvThemeData[SystemThemes::TextColor].GetCharPtr()); 
	Write ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"TextColor", sizeof ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"TextColor") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::TextColor].GetCharPtr()); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	// Separator
			
	Write ("<tr><td>&nbsp;</td><th bgcolor=\"", sizeof ("<tr><td>&nbsp;</td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Separator:</th><td><input type=\"checkbox\"", sizeof ("\" align=\"center\">Separator:</th><td><input type=\"checkbox\"") - 1);
	if (iOptions & THEME_SEPARATOR) {
				
	Write (" checked ", sizeof (" checked ") - 1);
	} 
	Write (" name=\"Sep", sizeof (" name=\"Sep") - 1);
	Write (i); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	// Good color
			
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Good Color:</th><td align=\"left\" bgcolor=\"", sizeof ("\" align=\"center\">Good Color:</th><td align=\"left\" bgcolor=\"") - 1);
	Write (pvThemeData[SystemThemes::GoodColor].GetCharPtr()); 
	Write ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"GoodColor", sizeof ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"GoodColor") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::GoodColor].GetCharPtr()); 
	Write ("\"></td></tr><tr>", sizeof ("\"></td></tr><tr>") - 1);
	// Buttons
			
	Write ("<td>&nbsp;</td><th bgcolor=\"", sizeof ("<td>&nbsp;</td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Buttons:</td><td><input type=\"checkbox\"", sizeof ("\" align=\"center\">Buttons:</td><td><input type=\"checkbox\"") - 1);
	if (iOptions & THEME_BUTTONS) {
				
	Write (" checked ", sizeof (" checked ") - 1);
	} 
	Write (" name=\"Butt", sizeof (" name=\"Butt") - 1);
	Write (i); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	// Bad color
			
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Bad Color:</th><td align=\"left\" bgcolor=\"", sizeof ("\" align=\"center\">Bad Color:</th><td align=\"left\" bgcolor=\"") - 1);
	Write (pvThemeData[SystemThemes::BadColor].GetCharPtr()); 
	Write ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"BadColor", sizeof ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"BadColor") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::BadColor].GetCharPtr()); 
	Write ("\"></td></tr><tr>", sizeof ("\"></td></tr><tr>") - 1);
	// Horz
			
	Write ("<td>&nbsp;</td><th bgcolor=\"", sizeof ("<td>&nbsp;</td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Horizontal Link:</td><td><input type=\"checkbox\"", sizeof ("\" align=\"center\">Horizontal Link:</td><td><input type=\"checkbox\"") - 1);
	if (iOptions & THEME_HORZ) {
				
	Write (" checked ", sizeof (" checked ") - 1);
	}
			
	Write (" name=\"Horz", sizeof (" name=\"Horz") - 1);
	Write (i); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	// Private color
			
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Private Color:</th><td align=\"left\" bgcolor=\"", sizeof ("\" align=\"center\">Private Color:</th><td align=\"left\" bgcolor=\"") - 1);
	Write (pvThemeData[SystemThemes::PrivateMessageColor].GetCharPtr()); 
	Write ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"PrivateColor", sizeof ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"PrivateColor") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::PrivateMessageColor].GetCharPtr()); 
	Write ("\"></td></tr><tr>", sizeof ("\"></td></tr><tr>") - 1);
	// Vert
			
	Write ("<td>&nbsp;</td><th bgcolor=\"", sizeof ("<td>&nbsp;</td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Vertical Link:</td><td><input type=\"checkbox\"", sizeof ("\" align=\"center\">Vertical Link:</td><td><input type=\"checkbox\"") - 1);
	if (iOptions & THEME_VERT) {
				
	Write (" checked ", sizeof (" checked ") - 1);
	}
			
	Write (" name=\"Vert", sizeof (" name=\"Vert") - 1);
	Write (i); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	// Broadcast color
			
	Write ("<th bgcolor=\"", sizeof ("<th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Broadcast Color:</th><td align=\"left\" bgcolor=\"", sizeof ("\" align=\"center\">Broadcast Color:</th><td align=\"left\" bgcolor=\"") - 1);
	Write (pvThemeData[SystemThemes::BroadcastMessageColor].GetCharPtr()); 
	Write ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"BroadcastColor", sizeof ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"BroadcastColor") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::BroadcastMessageColor].GetCharPtr()); 
	Write ("\"></td></tr><tr><td></td><td></td><td></td><th bgcolor=\"", sizeof ("\"></td></tr><tr><td></td><td></td><td></td><th bgcolor=\"") - 1);
	Write (m_vTableColor.GetCharPtr()); 
	Write ("\" align=\"center\">Table Color:</th><td align=\"left\" bgcolor=\"", sizeof ("\" align=\"center\">Table Color:</th><td align=\"left\" bgcolor=\"") - 1);
	Write (pvThemeData[SystemThemes::TableColor].GetCharPtr()); 
	Write ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"TableColor", sizeof ("\"><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"TableColor") - 1);
	Write (i); 
	Write ("\" value=\"", sizeof ("\" value=\"") - 1);
	Write (pvThemeData[SystemThemes::TableColor].GetCharPtr()); 
	Write ("\"></td>", sizeof ("\"></td>") - 1);
	// Delete theme
			
	Write ("<td align=\"center\">", sizeof ("<td align=\"center\">") - 1);
	sprintf (pszDeleteTheme, "DeleteTheme%i", i);
			WriteButtonString (m_iButtonKey, "DeleteTheme", "Delete Theme", pszDeleteTheme); 
	Write ("</td>", sizeof ("</td>") - 1);
	// Space between themes
			
	Write ("</tr><tr><td>&nbsp;</td></tr>", sizeof ("</tr><tr><td>&nbsp;</td></tr>") - 1);
	g_pGameEngine->FreeData (pvThemeData);
		}
		
	Write ("</table>", sizeof ("</table>") - 1);
	g_pGameEngine->FreeKeys (piThemeKey);
	}


	Write ("<p><h3>Create a new theme:</h3><p><table width=\"75%\"><tr><td>Name:</td><td><input type=\"text\" size=\"20\" maxlength=\"", sizeof ("<p><h3>Create a new theme:</h3><p><table width=\"75%\"><tr><td>Name:</td><td><input type=\"text\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_THEME_NAME_LENGTH); 
	Write ("\" name=\"NewName\"></td></tr><tr><td>Author's Name:</td><td><input type=\"text\" size=\"20\" maxlength=\"", sizeof ("\" name=\"NewName\"></td></tr><tr><td>Author's Name:</td><td><input type=\"text\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_THEME_AUTHOR_NAME_LENGTH); 
	Write ("\" name=\"NewAName\"></td></tr><tr><td>Version:</td><td><input type=\"text\" size=\"", sizeof ("\" name=\"NewAName\"></td></tr><tr><td>Version:</td><td><input type=\"text\" size=\"") - 1);
	Write (MAX_THEME_VERSION_LENGTH); 
	Write ("\" maxlength=\"", sizeof ("\" maxlength=\"") - 1);
	Write (MAX_THEME_VERSION_LENGTH); 
	Write ("\" name=\"NewVersion\"></td></tr><tr><td>Author's Email:</td><td><input type=\"text\" size=\"20\" maxlength=\"", sizeof ("\" name=\"NewVersion\"></td></tr><tr><td>Author's Email:</td><td><input type=\"text\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_THEME_AUTHOR_EMAIL_LENGTH); 
	Write ("\" name=\"NewAEmail\"></td></tr><tr><td>File Name:</td><td><input type=\"text\" size=\"20\" maxlength=\"", sizeof ("\" name=\"NewAEmail\"></td></tr><tr><td>File Name:</td><td><input type=\"text\" size=\"20\" maxlength=\"") - 1);
	Write (MAX_THEME_FILE_NAME_LENGTH); 
	Write ("\" name=\"NewFile\"></td></tr><tr><td>Background:</td><td><input type=\"checkbox\" name=\"NewBG\"></td></tr><tr><td>Live Planet:</td><td><input type=\"checkbox\" name=\"NewLP\"></td></tr><tr><td>Dead Planet:</td><td><input type=\"checkbox\" name=\"NewDP\"></td></tr><tr><td>Separator:</td><td><input type=\"checkbox\" name=\"NewSep\"></td></tr><tr><td>Buttons:</td><td><input type=\"checkbox\" name=\"NewButt\"></td></tr><tr><td>Horizontal Link Bar:</td><td><input type=\"checkbox\" name=\"NewHorz\"></td></tr><tr><td>Vertical Link Bar:</td><td><input type=\"checkbox\" name=\"NewVert\"></td></tr><tr><td>Text Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewTextColor\"></td></tr><tr><td>Good Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewGoodColor\"></td></tr><tr><td>Bad Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewBadColor\"></td></tr><tr><td>Private Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewPrivateColor\"></td></tr><tr><td>Broadcast Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewBroadcastColor\"></td></tr><tr><td>Table Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewTableColor\"></td></tr><tr><td>Description:</td><td><textarea rows=\"3\" cols=\"40\" wrap=\"physical\" name=\"NewDesc\"></textarea></td></tr></table><p>", sizeof ("\" name=\"NewFile\"></td></tr><tr><td>Background:</td><td><input type=\"checkbox\" name=\"NewBG\"></td></tr><tr><td>Live Planet:</td><td><input type=\"checkbox\" name=\"NewLP\"></td></tr><tr><td>Dead Planet:</td><td><input type=\"checkbox\" name=\"NewDP\"></td></tr><tr><td>Separator:</td><td><input type=\"checkbox\" name=\"NewSep\"></td></tr><tr><td>Buttons:</td><td><input type=\"checkbox\" name=\"NewButt\"></td></tr><tr><td>Horizontal Link Bar:</td><td><input type=\"checkbox\" name=\"NewHorz\"></td></tr><tr><td>Vertical Link Bar:</td><td><input type=\"checkbox\" name=\"NewVert\"></td></tr><tr><td>Text Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewTextColor\"></td></tr><tr><td>Good Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewGoodColor\"></td></tr><tr><td>Bad Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewBadColor\"></td></tr><tr><td>Private Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewPrivateColor\"></td></tr><tr><td>Broadcast Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewBroadcastColor\"></td></tr><tr><td>Table Color:</td><td><input type=\"text\" size=\"6\" maxlength=\"6\" name=\"NewTableColor\"></td></tr><tr><td>Description:</td><td><textarea rows=\"3\" cols=\"40\" wrap=\"physical\" name=\"NewDesc\"></textarea></td></tr></table><p>") - 1);
	WriteButton (BID_CANCEL);
	WriteButton (BID_CREATENEWTHEME);

	SYSTEM_CLOSE 
	Write ("\n", sizeof ("\n") - 1);
}