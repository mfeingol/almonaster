// Cookie.cpp: implementation of the Cookie class.
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

#include "Cookie.h"

#include "Osal/Algorithm.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cookie::Cookie () {

    m_iNumRefs = 1;

    m_pszName = NULL;
    m_pszValue = NULL;

    m_ppSubCookies = NULL;
    m_iNumSubCookies = 0;
    m_iNumSubCookiesSpace = 0;
}

Cookie::~Cookie() {

    if (m_pszName != NULL) {
        delete [] m_pszName;
    }

    if (m_iNumSubCookies != 0) {

        for (unsigned int i = 0; i < m_iNumSubCookies; i ++) {
            delete m_ppSubCookies[i];
        }
        delete [] m_ppSubCookies;
    }
}

int Cookie::Initialize (const char* pszName, const char* pszValue) {

    int iErrCode;

    size_t stLen1 = strlen (pszName) + 1;
    size_t stLen2 = strlen (pszValue) + 1;

    m_pszName = new char [stLen1 + stLen2];
    if (m_pszName == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    m_pszValue = m_pszName + stLen1;

    iErrCode = Algorithm::UnescapeString (pszName, m_pszName, stLen1);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = Algorithm::UnescapeString (pszValue, m_pszValue, stLen2);
    if (iErrCode != OK) {
        return iErrCode;
    }

    return OK;
}

Cookie* Cookie::CreateInstance (const char* pszName, const char* pszValue) {

    Cookie* pCookie = new Cookie ();
    if (pCookie == NULL) {
        return NULL;
    }

    if (pCookie->Initialize (pszName, pszValue) != OK) {
        delete pCookie;
        return NULL;
    }

    return pCookie;
}

int Cookie::AddSubCookie (Cookie* pCookie) {

    if (m_iNumSubCookies == 0) {

        m_ppSubCookies = new Cookie* [4];
        if (m_ppSubCookies == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        m_iNumSubCookiesSpace = 4;
    
    } else {
        
        // Realloc?
        if (m_iNumSubCookies == m_iNumSubCookiesSpace) {
            
            unsigned int iNumSubCookiesSpace = m_iNumSubCookies * 2;
            
            Cookie** ppSubCookies = new Cookie* [iNumSubCookiesSpace];
            if (m_ppSubCookies == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }

            memcpy (ppSubCookies, m_ppSubCookies, m_iNumSubCookies * sizeof (Cookie*));
            
            delete [] m_ppSubCookies;

            m_ppSubCookies = ppSubCookies;
            m_iNumSubCookiesSpace = iNumSubCookiesSpace;
        }
    }
    
    m_ppSubCookies [m_iNumSubCookies] = pCookie;
    m_iNumSubCookies ++;

    return OK;
}

unsigned int Cookie::GetNumSubCookies() {

    return m_iNumSubCookies;
}

ICookie* Cookie::GetSubCookie (unsigned int iIndex) {

    if (iIndex >= m_iNumSubCookies) {
        return NULL;
    }
    
    return m_ppSubCookies [iIndex];
}

const char* Cookie::GetName() {
    return m_pszName;
}

const char* Cookie::GetValue() {
    return m_pszValue;
}

int Cookie::GetIntValue() {
    return String::AtoI (m_pszValue);
}

unsigned int Cookie::GetUIntValue() {
    return String::AtoUI (m_pszValue);
}

int64 Cookie::GetInt64Value() {
    return String::AtoI64 (m_pszValue);
}

uint64 Cookie::GetUInt64Value() {
    return String::AtoUI64 (m_pszValue);
}

float Cookie::GetFloatValue() {
    return String::AtoF (m_pszValue);
}

UTCTime Cookie::GetTimeValue() {
    
    UTCTime tTime;
    Time::AtoUTCTime (m_pszValue, &tTime);

    return tTime;
}