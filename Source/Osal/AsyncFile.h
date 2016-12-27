// AsyncFile.h: interface for the AsyncFile class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#pragma once

#include "OS.h"

class OSAL_EXPORT AsyncFile
{
protected:
	HANDLE m_hFile;
    HANDLE m_hReadFile;

public:
    AsyncFile();
    virtual ~AsyncFile();

    int OpenAppend(const char* pszFileName);
    void Close();

    int Write(const void* pBuffer, unsigned int cbSize);
    int Tail(void* pBuffer, unsigned int* pcbSize);
};