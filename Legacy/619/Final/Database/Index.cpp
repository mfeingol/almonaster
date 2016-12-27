// Index.cpp: implementation of the Index class.
//
//////////////////////////////////////////////////////////////////////
//
// Database.dll - A database cache and backing store library
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
#include "Database.h"
#include "Index.h"
#undef DATABASE_BUILD

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Index::Index() : m_htHashTable ((void*) &m_IndexHint, (void*) &m_IndexHint) {

	m_iColumn = NO_KEY;
}

Index::~Index() {}

int Index::Initialize (unsigned int iColumn, unsigned int iNumCols, VariantType vtType, bool bCaseInsensitive, 
					   void* pBaseAddress) {

	m_iColumn = iColumn;

	m_IndexHint.BaseAddress = (size_t) pBaseAddress;
	m_IndexHint.CaseInsensitive = bCaseInsensitive;
	m_IndexHint.Type = vtType;

	if (!m_htHashTable.Initialize (iNumCols)) {
		return ERROR_OUT_OF_MEMORY;
	}

	return OK;
}

int Index::GetFirstKey (const Variant& vData, unsigned int* piKey) {

	*piKey = NO_KEY;

	if (vData.GetType() != m_IndexHint.Type) {
		Assert (false);
		return ERROR_TYPE_MISMATCH;
	}

	VariantArg vArg;
	const void* pData;

	switch (m_IndexHint.Type) {

	case V_INT:
		
		vArg.iArg = vData.GetInteger();
		pData = &(vArg.iArg);
		break;

	case V_FLOAT:

		vArg.fArg = vData.GetFloat();
		pData = &(vArg.fArg);
		break;

	case V_TIME:

		vArg.tArg = vData.GetUTCTime();
		pData = &(vArg.tArg);
		break;

	case V_STRING:

		pData = vData.GetCharPtr();
		break;

	default:

		Assert (false);
		return ERROR_DATA_CORRUPTION;
	}

	pData = (char*) pData - m_IndexHint.BaseAddress;
	return m_htHashTable.FindFirst (pData, piKey) ? OK : ERROR_DATA_NOT_FOUND;
}


int Index::GetEqualKeys (const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys) {

	*piNumKeys = 0;

	if (ppiKey != NULL) {
		*ppiKey = NULL;
	}

	if (vData.GetType() != m_IndexHint.Type) {
		Assert (false);
		return ERROR_TYPE_MISMATCH;
	}

	VariantArg vArg;
	const void* pData;

	switch (m_IndexHint.Type) {

	case V_INT:
		
		vArg.iArg = vData.GetInteger();
		pData = &(vArg.iArg);
		break;

	case V_FLOAT:

		vArg.fArg = vData.GetFloat();
		pData = &(vArg.fArg);
		break;

	case V_TIME:

		vArg.tArg = vData.GetUTCTime();
		pData = &(vArg.tArg);
		break;

	case V_STRING:

		pData = vData.GetCharPtr();
		break;

	default:

		Assert (false);
		return ERROR_DATA_CORRUPTION;
	}

	pData = (char*) pData - m_IndexHint.BaseAddress;

	HashTableIterator<const void*, unsigned int> htIterator;
	if (!m_htHashTable.FindFirst (pData, &htIterator)) {
		return ERROR_DATA_NOT_FOUND;
	}

	size_t stSpace = 16;
	if (ppiKey != NULL) {
		*ppiKey = new unsigned int [stSpace];
	}

	unsigned int* piTemp;

	do {

		if (ppiKey != NULL) {
			
			if (*piNumKeys == stSpace) {
				
				// Resize
				stSpace *= 2;
				piTemp = new unsigned int [stSpace];
				
				memcpy (piTemp, *ppiKey, *piNumKeys * sizeof (unsigned int));
				
				delete [] (*ppiKey);
				*ppiKey = piTemp;
			}
			
			(*ppiKey)[*piNumKeys] = htIterator.GetData();
		}

		(*piNumKeys) ++;
	
	} while (m_htHashTable.FindNext (&htIterator));

	return OK;
}


int Index::InsertRow (unsigned int iKey, const void* pData) {

	Assert (pData > (void*) m_IndexHint.BaseAddress);

	return m_htHashTable.Insert ((char*) pData - m_IndexHint.BaseAddress, iKey) ? OK : ERROR_FAILURE;
}


int Index::DeleteRow (unsigned int iKey, const void* pData) {

	int iErrCode = ERROR_FAILURE;

	HashTableIterator<const void*, unsigned int> htIterator;

	Assert (pData > (void*) m_IndexHint.BaseAddress);

	// Search for first hit
	bool bFound = m_htHashTable.FindFirst ((char*) pData - m_IndexHint.BaseAddress, &htIterator);
	while (bFound) {

		if (htIterator.GetData() == iKey) {
			iErrCode = m_htHashTable.Delete (&htIterator, NULL, NULL) ? OK : ERROR_FAILURE;
			break;
		}

		bFound = m_htHashTable.FindNext (&htIterator);
	}

	Assert (iErrCode == OK);
	return iErrCode;
}

int Index::DeleteAllRows() {

	m_htHashTable.Clear();
	return OK;
}

unsigned int Index::IndexHashValue::GetHashValue (const void* pData, unsigned int iNumBuckets, 
												  const void* pHashHint) {
	
	IndexHint* pIndexHint = (IndexHint*) pHashHint;
	
	const char* pszData = (char*) pData + pIndexHint->BaseAddress;
	
	switch (pIndexHint->Type) {
		
	case V_STRING:
		
		return Algorithm::GetStringHashValue (pszData, iNumBuckets, pIndexHint->CaseInsensitive);
		
	case V_INT:
	case V_TIME:

		// Breaks on 64 bit
		return Algorithm::GetIntHashValue (*((int*) pszData), iNumBuckets);
		
	case V_FLOAT:
		
		return Algorithm::GetFloatHashValue (*((float*) pszData), iNumBuckets);

	default:
		
		Assert (false);
		return 0;
	}
}

bool Index::IndexEquals::Equals (const void* pLeft, const void* pRight, const void* pEqualsHint) {
	
	IndexHint* pIndexHint = (IndexHint*) pEqualsHint;
	
	// Fix up addresses
	size_t stBaseAddress = pIndexHint->BaseAddress;
	
	const char* pszLeft = (char*) pLeft + stBaseAddress;
	const char* pszRight = (char*) pRight + stBaseAddress;
	
	switch (pIndexHint->Type) {
		
	case V_STRING:
		
		if (pIndexHint->CaseInsensitive) {
			return String::StriCmp (pszLeft, pszRight) == 0;
		} else {
			return String::StrCmp (pszLeft, pszRight) == 0;
		}
		
	case V_INT:
		
		return *((int*) pszLeft) == *((int*) pszRight);
		
	case V_TIME:
		
		return *((UTCTime*) pszLeft) == *((UTCTime*) pszRight);
		
	case V_FLOAT:
		
		return *((float*) pszLeft) == *((float*) pszRight);
		
	default:
		
		Assert (false);
		return false;
	}
}