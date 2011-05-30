// PageSourceEnumerator.h: interface for the PageSourceEnumeratora class.
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

#if !defined(AFX_PAGESOURCEENUMERATOR_H__C683FF73_BA45_11D3_A2B0_0050047FE2E2__INCLUDED_)
#define AFX_PAGESOURCEENUMERATOR_H__C683FF73_BA45_11D3_A2B0_0050047FE2E2__INCLUDED_

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD

#include "PageSource.h"
#include "Osal/HashTable.h"

class PageSourceHashValue {
public:
	static unsigned int GetHashValue (const char* pszData, unsigned int iNumBuckets, const void* pHashHint);
};

class PageSourceEquals {
public:
	static bool Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint);
};

class PageSourceEnumerator : public IPageSourceEnumerator {
private:

	PageSourceEnumerator();	
	~PageSourceEnumerator();

	int Initialize (
		HashTable<const char*, PageSource*, PageSourceHashValue, PageSourceEquals>* pPageSourceTable,
		PageSource* pDefaultPageSource);

	IPageSourceControl** m_ppPageSourceControls;
	unsigned int m_iNumPageSources;

public:

	static PageSourceEnumerator* CreateInstance (
		HashTable<const char*, PageSource*, PageSourceHashValue, PageSourceEquals>* m_pPageSourceTable,
		PageSource* pDefaultPageSource);

	// IPageSourceEnumerator
	IMPLEMENT_INTERFACE (IPageSourceEnumerator);

	unsigned int GetNumPageSources();
	IPageSourceControl** GetPageSourceControls();
};

#endif // !defined(AFX_PAGESOURCEENUMERATOR_H__C683FF73_BA45_11D3_A2B0_0050047FE2E2__INCLUDED_)
