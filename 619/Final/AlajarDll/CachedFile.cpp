// CachedFile.cpp: implementation of the File class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar 1.0:  a web server
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

#include "CachedFile.h"
#include "Osal/String.h"
#include "Osal/Algorithm.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CachedFile::CachedFile() {

	m_iNumRefs = 1;
	Initialize();
}

CachedFile::~CachedFile() {
	Clean();
}

void CachedFile::Initialize() {

	m_pszFileName = NULL;
	m_pszMimeType = NULL;
}

int CachedFile::Close() {	

	Clean();
	return OK;
}

void CachedFile::Clean() {

	m_mmfFile.Close();

	if (m_pszFileName != NULL) {
		OS::HeapFree (m_pszFileName);
		m_pszFileName = NULL;
	}
	
	if (m_pszMimeType != NULL) {
		OS::HeapFree (m_pszMimeType);
		m_pszMimeType = NULL;
	}
}

CachedFile* CachedFile::CreateInstance() {
	return new CachedFile();
}

int CachedFile::Open (const char* pszFileName) {

	return Load (pszFileName);
}

int CachedFile::Load (const char* pszFileName) {

	int iErrCode = m_mmfFile.OpenExisting (pszFileName, true);
	if (iErrCode != OK) {
		Clean();
		return iErrCode;
	}

	// Mime type
	char pszMimeType [OS::MaxMimeTypeLength];

	iErrCode = File::GetFileMimeType (pszFileName, pszMimeType);
	if (iErrCode != OK) {
		Clean();
		return iErrCode;
	}

	m_pszMimeType = String::StrDup (pszMimeType);
	if (m_pszMimeType == NULL) {
		Clean();
		return ERROR_OUT_OF_MEMORY;
	}
	
	// Last modified time int
	iErrCode = File::GetLastModifiedTime (pszFileName, &m_tLastModifiedTime);
	if (iErrCode != OK) {
		Clean();
		return iErrCode;
	}

	// Copy name
	m_pszFileName = String::StrDup (pszFileName);
	if (m_pszFileName == NULL) {
		Clean();
		return ERROR_OUT_OF_MEMORY;
	}

	return iErrCode;
}

int CachedFile::SetMimeType (const char* pszMimeType) {

	char* pszNewMimeType = String::StrDup (pszMimeType);
	if (pszNewMimeType == NULL) {
		return ERROR_OUT_OF_MEMORY;
	}

	if (m_pszMimeType != NULL) {
		OS::HeapFree (m_pszMimeType);
	}

	m_pszMimeType = pszNewMimeType;

	return OK;
}

const char* CachedFile::GetMimeType() {
	return m_pszMimeType;
}

const char* CachedFile::GetName() {
	return m_pszFileName;
}

const void* CachedFile::GetData() {
	return m_mmfFile.GetAddress();
}

size_t CachedFile::GetSize() {
	return m_mmfFile.GetSize();
}

void CachedFile::GetLastModifiedTime (UTCTime* ptLastModifiedTime) {
	*ptLastModifiedTime = m_tLastModifiedTime;
}