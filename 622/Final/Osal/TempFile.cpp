// TempFile.cpp: implementation of the TempFile class.
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

#define OSAL_BUILD
#include "TempFile.h"
#undef OSAL_BUILD

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TempFile::TempFile() {
    *m_pszFileName = '\0';
}

int TempFile::Open() {

#ifdef __LINUX__
#if defined P_tmpdir
    snprintf(m_pszFileName, sizeof(m_pszFileName), "%s/OsalXXXXXX", P_tmpdir);
#else
    snprintf(m_pszFileName, sizeof(m_pszFileName), "/tmp/OsalXXXXXX");
#endif

    int m_hFile = mkstemp(m_pszFileName);
    if (m_hFile < 0)
    {
        Assert (false);
        return ERROR_FAILURE;
    }
    return OK;

#else if defined __WIN32__

    // Get temp directory
    char pszTempDir [OS::MaxFileNameLength];

    DWORD dwRetVal = ::GetTempPath (sizeof (pszTempDir), pszTempDir);
    if (dwRetVal == 0) {
        Assert (false);
        return ERROR_FAILURE;
    }

    dwRetVal = ::GetTempFileName (pszTempDir, "Osal", 0, m_pszFileName);
    if (dwRetVal == 0) {
        Assert (false);
        return ERROR_FAILURE;
    }

    if (m_hFile != INVALID_HANDLE_VALUE) {
        ::CloseHandle (m_hFile);
    }

    m_hFile = ::CreateFile (
        m_pszFileName, 
        GENERIC_WRITE | GENERIC_READ, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_ALWAYS, 
        FILE_ATTRIBUTE_TEMPORARY, 
        NULL
        );
    
    if (m_hFile != INVALID_HANDLE_VALUE) {
        
        if (::SetEndOfFile (m_hFile)) {
            return OK;
        }
        Delete();
    }
    
    return ERROR_FAILURE;
#endif
}

const char* TempFile::GetName() {
    return m_pszFileName;
}

int TempFile::Delete() {
#ifdef __LINUX__
    close(m_hFile);
    return (unlink(m_pszFileName) == 0) ? OK : ERROR_FAILURE;
#else if defined __WIN32__
	Close();
	return ::DeleteFile (m_pszFileName) ? OK : ERROR_FAILURE;
#endif
}