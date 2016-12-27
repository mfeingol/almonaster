// Cookie.h: interface for the Cookie class.
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

#if !defined(AFX_COOKIE_H__5B5644D8_4CF6_11D3_A18F_0050047FE2E2__INCLUDED_)
#define AFX_COOKIE_H__5B5644D8_4CF6_11D3_A18F_0050047FE2E2__INCLUDED_

#define ALAJAR_BUILD
#include "Alajar.h"
#undef ALAJAR_BUILD

class Cookie : public ICookie {

private:

    char* m_pszName;
    char* m_pszValue;

    Cookie** m_ppSubCookies;

    unsigned int m_iNumSubCookies;
    unsigned int m_iNumSubCookiesSpace;

    Cookie();
    ~Cookie();

    int Initialize (const char* pszName, const char* pszValue);

public:

    static Cookie* CreateInstance (const char* pszName, const char* pszValue);

    int AddSubCookie (Cookie* pCookie);

    // ICookie
    IMPLEMENT_INTERFACE (ICookie);

    unsigned int GetNumSubCookies();
    ICookie* GetSubCookie (unsigned int iIndex);

    const char* GetName();
    const char* GetValue();

    int GetIntValue();
    unsigned int GetUIntValue();

    int64 GetInt64Value();
    uint64 GetUInt64Value();
    
    float GetFloatValue();
    UTCTime GetTimeValue();
};

#endif // !defined(AFX_COOKIE_H__5B5644D8_4CF6_11D3_A18F_0050047FE2E2__INCLUDED_)
