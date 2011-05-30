// TempFile.h: interface for the TempFile class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
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

#if !defined(AFX_TEMPFILE_H__9D8B6F65_AECF_11D1_9C63_0060083E8062__INCLUDED_)
#define AFX_TEMPFILE_H__9D8B6F65_AECF_11D1_9C63_0060083E8062__INCLUDED_

#include "File.h"

class OSAL_EXPORT TempFile : public File {

protected:

	// File name
	char m_pszFileName [OS::MaxFileNameLength];

public:
	TempFile();
	
	int Open();

	const char* GetName();
	int Delete();

	//static int GetTempFileName (const char* pszDirectory, char pszFileName [OS::MaxFileNameLength]);
};

#endif // !defined(AFX_TEMPFILE_H__9D8B6F65_AECF_11D1_9C63_0060083E8062__INCLUDED_)
