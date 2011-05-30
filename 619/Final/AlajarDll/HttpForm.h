// HttpForm.h: interface for the HttpForm class.
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

#if !defined(AFX_HTTPFORM_H__C25D88C3_44DB_11D2_9EEE_0060083E8062__INCLUDED_)
#define AFX_HTTPFORM_H__C25D88C3_44DB_11D2_9EEE_0060083E8062__INCLUDED_

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD

#include "Osal/Algorithm.h"
#include "Osal/MemoryMappedFile.h"

// Form types

class HttpForm : public IHttpForm {
private:

	HttpFormType m_ftFormType;

	char* m_pszFormName;
	char* m_pszFormValue;
	char* m_pszFileName;

	HttpForm** m_ppSubForms;

	unsigned int m_iNumSubForms;
	unsigned int m_iNumSubFormsSpace;

	MemoryMappedFile m_mmfFile;

	HttpForm();
	~HttpForm();

	int Initialize (HttpFormType ftType, const char* pszFormName, const char* pszFormValue, 
		const char* pszFileName, bool bMultipart);

public:

	static HttpForm* CreateInstance (HttpFormType ftType, const char* pszFormName, const char* pszFormValue, 
		const char* pszFileName, bool bMultipart);

	int AddForm (HttpForm* pHttpForm);
	
	// IHttpForm
	IMPLEMENT_INTERFACE (IHttpForm);

	unsigned int GetNumForms();
	IHttpForm* GetForm (unsigned int iIndex);
	
	const char* GetName();
	const char* GetValue();

	int GetIntValue();
	float GetFloatValue();
	UTCTime GetTimeValue();
	int64 GetInt64Value();

	HttpFormType GetType();
	const char* GetFileName();
};

#endif // !defined(AFX_HTTPFORM_H__C25D88C3_44DB_11D2_9EEE_0060083E8062__INCLUDED_)