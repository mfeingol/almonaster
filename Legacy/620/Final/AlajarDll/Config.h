// Config.h: interface for the Config class.
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

#if !defined(AFX_CONFIG_H__5B5644D7_4CF6_11D3_A18F_0050047FE2E2__INCLUDED_)
#define AFX_CONFIG_H__5B5644D7_4CF6_11D3_A18F_0050047FE2E2__INCLUDED_

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD

#include "Osal/ConfigFile.h"

class Config : public IConfigFile {
private:

    Config();

    ConfigFile m_cfConfig;

public:

    static Config* CreateInstance();

    // IConfigFile
    IMPLEMENT_INTERFACE (IConfigFile);

    int Open (const char* pszFileName);
    int Close();
    int Refresh();

    unsigned int GetNumParameters();
    const char** GetParameterNames();

    int GetParameter (unsigned int iIndex, char** ppszLhs, char** ppszRhs);
    int GetParameter (const char* pszLhs, char** ppszRhs);

    int SetParameter (const char* pszNewLhs, const char* pszNewRhs);
};

#endif // !defined(AFX_CONFIG_H__5B5644D7_4CF6_11D3_A18F_0050047FE2E2__INCLUDED_)
