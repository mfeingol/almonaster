//
// Database.dll - A database library
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA  02111-1307, USA.

#define DATABASE_BUILD
#include "CDatabase.h"
#include "Template.h"
#undef DATABASE_BUILD

int Database::CreateTemplate (const TemplateDescription& ttTemplate) {
	
	// Verify template data
	int iErrCode = Template::VerifyTemplate (ttTemplate);
	if (iErrCode != OK) {
		Assert (false);
		return iErrCode;
	}

	char pszDirName [OS::MaxFileNameLength];
	sprintf (pszDirName, "%s%s/", m_pszDataDirectory, ttTemplate.Name);

	// Create the template and its data file
	Template* pTemplate = Template::CreateInstance (ttTemplate, this);

	NamedMutex nmMutex;
	Mutex::Wait (pszDirName, &nmMutex);

	iErrCode = pTemplate->Create (pszDirName);
	if (iErrCode != OK) {

		if (iErrCode != ERROR_COULD_NOT_CREATE_TEMPLATE_DIRECTORY && 
			iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
			pTemplate->DeleteOnDisk();
		}
		pTemplate->Release();
		
		Mutex::Signal (nmMutex);
		return iErrCode;
	}

	Mutex::Signal (nmMutex);

	// Try to add template
	m_rwTemplateLock.WaitWriter();
	bool bAdded = m_htTemplates.Insert (pTemplate->GetName(), pTemplate);
	m_rwTemplateLock.SignalWriter();

	if (bAdded) {
		
		size_t stSize;
		iErrCode = File::GetFileSize ((String) pszDirName + TEMPLATE_DATA_FILE, &stSize);
		Assert (iErrCode == OK);

		IncrementSizeOnDisk (stSize);

	} else {
		
		Assert (false);

		pTemplate->Release();
		iErrCode = ERROR_TEMPLATE_ALREADY_EXISTS;
	}

	return iErrCode;
}


int Database::DeleteTemplate (const char* pszTemplateName) {

	int iErrCode = OK;

	Template* pTemplate = NULL;

	// Lock table
	m_rwTemplateLock.WaitWriter();
	bool bDeleted = m_htTemplates.DeleteFirst (pszTemplateName, NULL, &pTemplate);
	m_rwTemplateLock.SignalWriter();

	if (bDeleted) {

		// Delete the template
		pTemplate->DeleteOnDisk();
		pTemplate->Release();

	} else {

		Assert (false);
		iErrCode = ERROR_UNKNOWN_TEMPLATE_NAME;
	}

	return iErrCode;
}

int Database::GetTemplate (const char* pszTemplateName, ITemplate** ppTemplate) {

	Template* pTemplate = FindTemplate (pszTemplateName);

	if (pTemplate == NULL) {
		*ppTemplate = NULL;
		return ERROR_UNKNOWN_TEMPLATE_NAME;
	}

	*ppTemplate = pTemplate;
	pTemplate->AddRef();

	return OK;
}

Template* Database::FindTemplate (const char* pszTemplateName) {

	Template* pTemplate = NULL;

	m_rwTemplateLock.WaitReader();

	if (m_htTemplates.FindFirst (pszTemplateName, &pTemplate)) {
		pTemplate->AddRef();
	}

	m_rwTemplateLock.SignalReader();	
	
	return pTemplate;
}

bool Database::IsTemplateEqual (const char* pszTemplateName, const TemplateDescription& ttTemplate) {

	Template* pTemplate = FindTemplate (pszTemplateName);

	if (pTemplate == NULL) {
		return false;
	}

	bool bRetVal = pTemplate->IsEqual (ttTemplate);

	pTemplate->Release();

	return bRetVal;
}

bool Database::DoesTemplateExist (const char* pszTemplateName) {

	Template* pTemplate;

	m_rwTemplateLock.WaitReader();
	bool bRetVal = m_htTemplates.FindFirst (pszTemplateName, &pTemplate);
	m_rwTemplateLock.SignalReader();

	return bRetVal;
}

int TemplateNameHashValue::GetHashValue (const char* pszString, unsigned int iNumBuckets, const void* pvHashHint) {
	
	return Algorithm::GetStringHashValue (pszString, iNumBuckets);
}

bool TemplateNameEquals::Equals (const char* pszLeft, const char* pszRight, const void* pvEqualsHint) {
	return String::StriCmp (pszLeft, pszRight) == 0;
}