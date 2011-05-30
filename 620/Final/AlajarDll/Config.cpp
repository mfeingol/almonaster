// Config.cpp: implementation of the Config class.
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

#include "Config.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Config::Config() {
    m_iNumRefs = 1;
}

Config* Config::CreateInstance() {
    return new Config();
}

int Config::Open (const char* pszFileName) {
    return m_cfConfig.Open (pszFileName);
}

int Config::Close() {
    return m_cfConfig.Close();
}

int Config::Refresh() {
    return m_cfConfig.Refresh();
}

unsigned int Config::GetNumParameters() {
    return m_cfConfig.GetNumParameters();
}

const char** Config::GetParameterNames() {
    return m_cfConfig.GetParameterNames();
}

int Config::GetParameter (unsigned int iIndex, char** ppszLhs, char** ppszRhs) {
    return m_cfConfig.GetParameter (iIndex, ppszLhs, ppszRhs);
}

int Config::GetParameter (const char* pszLhs, char** ppszRhs) {
    return m_cfConfig.GetParameter (pszLhs, ppszRhs);
}

int Config::SetParameter (const char* pszNewLhs, const char* pszNewRhs) {
    return m_cfConfig.SetParameter (pszNewLhs, pszNewRhs);
}