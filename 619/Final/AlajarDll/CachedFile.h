// CachedFile.h: interface for the CachedFile class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar.dll
// Copyright (C) 1998-1999 Max Attar Feingold (maf6@cornell.edu)
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

#if !defined(AFX_CACHEDFILE_H__C5064726_A72E_11D1_9C50_0060083E8062__INCLUDED_)
#define AFX_CACHEDFILE_H__C5064726_A72E_11D1_9C50_0060083E8062__INCLUDED_

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD

#include "Osal/Mutex.h"
#include "Osal/File.h"
#include "Osal/MemoryMappedFile.h"
#include "Osal/ReadWriteLock.h"

class CachedFile : public ICachedFile {

protected:

	char* m_pszFileName;
	char* m_pszMimeType;

	MemoryMappedFile m_mmfFile;

	UTCTime m_tLastModifiedTime;

	CachedFile ();
	~CachedFile();

	void Clean();
	void Initialize();

	int Load (const char* pszFileName);

public:

	static CachedFile* CreateInstance();

	unsigned int GetNumRefs();
	size_t GetNumCachedBytes();

	int Open (const char* pszFileName);
	int Close();

	int SetMimeType (const char* pszMimeType);

	// ICachedFile
	IMPLEMENT_INTERFACE (ICachedFile);

	const char* GetName();
	const char* GetMimeType();

	void GetLastModifiedTime (UTCTime* ptLastModifiedTime);

	size_t GetSize();
	const void* GetData();
};

#endif // !defined(AFX_FILE_H__C5064726_A72E_11D1_9C50_0060083E8062__INCLUDED_)