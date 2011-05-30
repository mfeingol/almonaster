// String.cpp: implementation of the String class.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define OSAL_BUILD
#include "String.h"
#undef OSAL_BUILD

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

String::String() {
    m_pszString = NULL;
    m_iLength = 0;
    m_iRealLength = 0;
}


String::String (const String& strString) {

    if (strString.m_pszString == NULL) {
        m_pszString = NULL;
        m_iLength = 0;
        m_iRealLength = 0;
    } else {
        m_iLength = strString.m_iLength;
        m_iRealLength = 2 * m_iLength + 1;
        m_pszString = new char [m_iRealLength];
        
        if (m_pszString != NULL) {
            memcpy (m_pszString, strString.m_pszString, strString.m_iLength + 1);
        }
    }
}


String::String (const char* pszString) {
    
    if (pszString == NULL) {
        m_pszString = NULL;
        m_iLength = 0;
        m_iRealLength = 0;
    } else {
        m_iLength = strlen (pszString);
        m_iRealLength = 2 * m_iLength + 1;
        m_pszString = new char [m_iRealLength];

        if (m_pszString != NULL) {
            memcpy (m_pszString, pszString, m_iLength + 1);
        }
    }
}


String::String (int iInt) {
    
    m_pszString = new char [128];
    
    if (m_pszString != NULL) {
        itoa (iInt, m_pszString, 10);
        m_iLength = strlen (m_pszString);
        m_iRealLength = 127;
    } else {
        m_iLength = m_iRealLength = 0;
    }
}

String::String (int64 i64Int) {
    
    m_pszString = new char [128];
    
    if (m_pszString != NULL) {
        _i64toa (i64Int, m_pszString, 10);
        m_iLength = strlen (m_pszString);
        m_iRealLength = 127;
    } else {
        m_iLength = m_iRealLength = 0;
    }
}

String::String (unsigned int iInt) {
    
    m_pszString = new char [128];

    if (m_pszString != NULL) {
        itoa (iInt, m_pszString, 10);
        m_iLength = strlen (m_pszString);
        m_iRealLength = 127;
    } else {
        m_iLength = m_iRealLength = 0;
    }
}


String::String (float fFloat) {
    
    m_pszString = new char [128];

    if (m_pszString != NULL) {
        sprintf (m_pszString, "%.3f", fFloat);
        m_iLength = strlen (m_pszString);
        m_iRealLength = 127;
    } else {
        m_iLength = m_iRealLength = 0;
    }
}


String::~String() {
    if (m_pszString != NULL) {
        delete [] m_pszString;
    }
}

String::operator char*() const { 
    return m_pszString;
}

String::operator const char*() const { 
    return (const char*) m_pszString;
}

char* String::GetCharPtr() const {
    return m_pszString;
}


size_t String::GetLength() const {
    return m_iLength;
}


// Operators
bool String::operator== (const String& strString) {

    if (strString.m_pszString == NULL || *(strString.m_pszString) == '\0') {
        return m_pszString == NULL || *m_pszString == '\0';
    }

    if (m_pszString == NULL || *m_pszString == '\0') {
        return false;
    }

    return strcmp (m_pszString, strString.m_pszString) == 0;
}

bool String::operator!= (const String& strString) {

    return !(*this == strString);
}

String& String::operator= (const String& strString) {

    if (m_iRealLength > strString.m_iLength) {

        memcpy (m_pszString, strString.m_pszString, strString.m_iLength + 1);
        m_iLength = strString.m_iLength;

    } else {

        size_t stRealLength = 2 * strString.m_iLength + 1;

        char* pszString = new char [stRealLength];
        if (pszString != NULL) {

            if (m_pszString != NULL) {
                delete [] m_pszString;
            }
            m_pszString = pszString;
            
            memcpy (m_pszString, strString.m_pszString, strString.m_iLength + 1);

            m_iLength = strString.m_iLength;
            m_iRealLength = stRealLength;
        }
    }

    return *this;
}


String& String::operator= (const char* pszString) {

    if (pszString == NULL) {

        m_iLength = 0;
        if (m_pszString != NULL) {
            *m_pszString = '\0';
        }

    } else {

        size_t stLength = strlen (pszString);

        if (m_iRealLength > stLength) {
            
            strcpy (m_pszString, pszString);
            m_iLength = stLength;

        } else {

            size_t stRealLength = 2 * stLength + 1;

            char* pszTemp = new char [stRealLength];
            if (pszTemp != NULL) {

                if (m_pszString != NULL) {
                    delete [] m_pszString;
                }
                m_pszString = pszTemp;

                memcpy (m_pszString, pszString, stLength + 1);
                
                m_iLength = stLength;
                m_iRealLength = stRealLength;
            }
        }
    }

    return *this;
}

String& String::operator+= (const String& strString) {

    return AddString (strString.m_pszString, strString.m_iLength);
}

String& String::operator+= (const char* pszString) {
        
    return AddString (pszString, StrLen (pszString));
}

String& String::operator+= (char pszString[]) {
    
    return AddString (pszString, StrLen (pszString));
}

String& String::AddString (const char* pszString, size_t stLength) {

    if (pszString == NULL) {
        return *this;
    }

    if (m_pszString == NULL) {

        size_t stRealLength = stLength * 2 + 1;

        m_pszString = new char [stRealLength];
        if (m_pszString != NULL) {
            m_iLength = stLength;
            m_iRealLength = stRealLength;
            memcpy (m_pszString, pszString, stLength + 1);
        }

    } else {
        
        size_t stNewLength = m_iLength + stLength;

        if (m_iRealLength > stNewLength) {
            memcpy (m_pszString + m_iLength, pszString, stLength + 1);
            m_iLength = stNewLength;
        } else {

            size_t stRealLength = stNewLength * 2 + 1;
            char* pszTemp = new char [stRealLength];

            if (pszTemp != NULL) {

                memcpy (pszTemp, m_pszString, m_iLength + 1);
                memcpy (pszTemp + m_iLength, pszString, stLength + 1);
                
                delete [] m_pszString;
                m_pszString = pszTemp;

                m_iLength = stNewLength;
                m_iRealLength = stRealLength;
            }
        }
    }

    return *this;
}

String& String::operator+= (int iInt) {

    char pszInteger [128];
    itoa (iInt, pszInteger, 10);

    return AddString (pszInteger, strlen (pszInteger));
}

String& String::operator+= (unsigned int iInt) {
        
    char pszInteger [128];
    _ultoa (iInt, pszInteger, 10);

    return AddString (pszInteger, strlen (pszInteger));
}

String& String::operator+= (float fFloat) {
    
    char pszFloat [128];
    sprintf (pszFloat, "%.3f", fFloat);

    return AddString (pszFloat, strlen (pszFloat));
}


String operator+ (const String& strLhs, const String& strRhs) {
    String strTemp (strLhs);
    return strTemp += strRhs;
}

String operator+ (const String& strLhs, const int iRhs) {
    String strTemp (strLhs);
    return strTemp += iRhs;
}

String operator+ (int iLhs, const String& strRhs) {
    String strTemp (iLhs);
    return strTemp += strRhs;
}

String operator+ (const String& strLhs, const unsigned int iRhs) {
    String strTemp (strLhs);
    return strTemp += iRhs;
}

String operator+ (unsigned int iLhs, const String& strRhs) {
    String strTemp (iLhs);
    return strTemp += strRhs;
}

String operator+ (const String& strLhs, const float fRhs) {
    String strTemp (strLhs);
    return strTemp += fRhs;
}

String operator+ (float fLhs, const String& strRhs) {
    String strTemp (fLhs);
    return strTemp += strRhs;
}

String operator+ (const String& strLhs, const char* pszRhs) {
    String strTemp (strLhs);
    return strTemp += pszRhs;
}

String operator+ (const char* pszLhs, const String& strRhs) {
    String strTemp (pszLhs);
    return strTemp += strRhs;
}


bool String::IsBlank() const {
    return String::IsBlank (m_pszString);
}

bool String::IsWhiteSpace() const {
    return String::IsWhiteSpace (m_pszString);
}

bool String::Equals (const String& strComp) const {
    
    if (m_pszString == NULL) {
        return strComp.m_pszString == NULL || *(strComp.m_pszString) == '\0';
    }

    if (strComp.m_pszString == NULL) {
        return *m_pszString == '\0';
    }

    return (strcmp (m_pszString, strComp.m_pszString) == 0);
}

bool String::IEquals (const String& strComp) const {
    
    if (m_pszString == NULL) {
        return strComp.m_pszString == NULL || *(strComp.m_pszString) == '\0';
    }

    if (strComp.m_pszString == NULL) {
        return *m_pszString == '\0';
    }

    return stricmp (m_pszString, strComp.m_pszString) == 0;
}

char String::GetCharAt (unsigned int iIndex) const {

    if (iIndex >= m_iLength) {
        return INVALID_CHAR;
    }
    
    return m_pszString[iIndex];
}

void String::SetCharAt (unsigned int iIndex, char szChar) {

    if (iIndex < m_iLength) {
        m_pszString[iIndex] = szChar;

        if (szChar == '\0') {
            m_iLength = iIndex;
        }
    }
}


char String::GetLastChar() const {

    if (m_pszString == NULL || m_iLength == 0) {
        return '\0';
    }

    return m_pszString[m_iLength - 1];
}

void String::Clear() {

    m_iLength = 0;
    if (m_pszString != NULL) {
        m_pszString[0] = '\0';
    }
}

int String::StrCmp (const char* pszStringOne, const char* pszStringTwo) {

    if (pszStringOne == NULL) {
        return (pszStringTwo == NULL || *pszStringTwo == '\0') ? 0 : -1;
    }

    if (pszStringTwo == NULL) {
        return (*pszStringOne == '\0') ? 0 : 1;
    }

    return strcmp (pszStringOne, pszStringTwo);
}

size_t String::StrLen (const char* pszString) {

    if (pszString == NULL) {
        return 0;
    }

    return strlen (pszString);
}

int String::AtoI (const char* pszString) {

    if (pszString == NULL) {
        return 0;
    }

    return atoi (pszString);
}

unsigned int String::AtoUI (const char* pszString) {

    if (pszString == NULL) {
        return 0;
    }

    return (unsigned int) strtoul (pszString, NULL, 10);
}

int64 String::AtoI64 (const char* pszString) {

    if (pszString == NULL) {
        return 0;
    }

    return _atoi64 (pszString);
}

uint64 String::AtoUI64 (const char* pszString) {

    if (pszString == NULL) {
        return 0;
    }

#ifdef __LINUX__
    return strtoull(pszString, NULL, 10);
#else if defined __WIN32__
    return (uint64) _atoi64 (pszString);
#endif
}

float String::AtoF (const char* pszString) {

    if (pszString == NULL) {
        return (float) 0.0;
    }

    return (float) atof (pszString);
}


char* String::UItoA (unsigned int iData, char* pszString, int iRadix) {

    return _ultoa (iData, pszString, iRadix);
}

char* String::UItoA (unsigned int iData, char* pszString, int iRadix, unsigned int iDigits) {

    char pszBuffer [128];
    _ultoa (iData, pszBuffer, iRadix);

    return CopyWithZeroes (pszString, pszBuffer, iDigits);
}

char* String::ItoA (int iData, char* pszString, int iRadix) {

    return itoa (iData, pszString, iRadix);
}

char* String::ItoA (int iData, char* pszString, int iRadix, unsigned int iDigits) {

    char pszBuffer [128];
    itoa (iData, pszBuffer, iRadix);

    return CopyWithZeroes (pszString, pszBuffer, iDigits);
}

char* String::UI64toA (uint64 iData, char* pszString, int iRadix) {

    return _ui64toa (iData, pszString, iRadix);
}

char* String::UI64toA (uint64 iData, char* pszString, int iRadix, unsigned int iDigits) {

    char pszBuffer [128];
    _ui64toa (iData, pszBuffer, iRadix);

    return CopyWithZeroes (pszString, pszBuffer, iDigits);
}

char* String::I64toA (int64 iData, char* pszString, int iRadix) {

    return _i64toa(iData, pszString, iRadix);
}

char* String::I64toA (int64 iData, char* pszString, int iRadix, unsigned int iDigits) {

    char pszBuffer [128];
    _i64toa (iData, pszBuffer, iRadix);

    return CopyWithZeroes (pszString, pszBuffer, iDigits);
}

char* String::CopyWithZeroes (char* pszString, char* pszBuffer, unsigned int iDigits) {

    char* pszRetVal = pszString;

    size_t i = 0, iNumZeroes = (int) iDigits - strlen (pszBuffer);

    if (*pszBuffer == '-') {

        pszBuffer ++;
        iNumZeroes ++;
        
        *pszString = '-';
        pszString ++;
    }

    if (iNumZeroes > 0) {
        for (; i < iNumZeroes; i ++) {
            pszString[i] = '0';
        }
    }

    strcpy (pszString + i, pszBuffer);

    return pszRetVal;
}

char* String::StrDup (const char* pszString) {

    if (pszString == NULL) {
        return NULL;
    }

    size_t stLength = strlen (pszString) + 1;
    
    char* pszRetVal = (char*) OS::HeapAlloc (stLength * sizeof (char));
    if (pszRetVal == NULL) {
        return NULL;
    }
    
    memcpy (pszRetVal, pszString, stLength);

    return pszRetVal;
}

char* String::StrCpy (char* pszDest, const char* pszSrc) {
    
    if (pszDest != NULL) {
    
        if (pszSrc == NULL) {
            *pszDest = '\0';
        } else {
            strcpy (pszDest, pszSrc);
        }
    }

    return pszDest;
}

char* String::StrnCpy (char* pszDest, const char* pszSrc, size_t stLen) {

    if (stLen > 0 && pszDest != NULL) {

        if (pszSrc == NULL) {
            *pszDest = '\0';
        } else {
            strncpy (pszDest, pszSrc, stLen);
        }
    }

    return pszDest;
}

char* String::StrCat (char* pszDest, const char* pszSrc) {

    if (pszSrc != NULL) {
        strcat (pszDest, pszSrc);
    }
    return pszDest;
}

char* String::StrnCat (char* pszDest, const char* pszSrc, size_t stLen) {

    if (pszSrc != NULL && pszDest != NULL) {
        strncat (pszDest, pszSrc, stLen);
    }

    return pszDest;
}

void String::StrLwr (char* pszString) {

    if (pszString != NULL) {

#ifdef __LINUX__
        for (char *ptr = pszString; *ptr != '\0'; ptr++)
        {
            *ptr = tolower(*ptr);
        }
#else if defined __WIN32__
		_strlwr (pszString);
#endif
    }
}

int String::StriCmp (const char* pszStringOne, const char* pszStringTwo) {

    if (pszStringOne == NULL) {
        return (pszStringTwo == NULL || *pszStringTwo == '\0') ? 0 : -1;
    }

    if (pszStringTwo == NULL) {
        return (*pszStringOne == '\0') ? 0 : 1;
    }

    return stricmp (pszStringOne, pszStringTwo);
}

int String::StrnCmp (const char* pszStringOne, const char* pszStringTwo, size_t stNumChars) {

    if (pszStringOne == NULL) {
        return (pszStringTwo == NULL || *pszStringTwo == '\0') ? 0 : -1;
    }

    if (pszStringTwo == NULL) {
        return (*pszStringOne == '\0') ? 0 : 1;
    }

    size_t stLen1 = strlen (pszStringOne), stLen2 = strlen (pszStringTwo);

    if (stNumChars > stLen1 || stNumChars > stLen2) {
        return -1;
    }

    return strncmp (pszStringOne, pszStringTwo, stNumChars);
}

int String::StrniCmp (const char* pszStringOne, const char* pszStringTwo, size_t stNumChars) {

    if (pszStringOne == NULL) {
        return (pszStringTwo == NULL || *pszStringTwo == '\0') ? 0 : -1;
    }

    if (pszStringTwo == NULL) {
        return (*pszStringOne == '\0') ? 0 : 1;
    }

    size_t stLen1 = strlen (pszStringOne), stLen2 = strlen (pszStringTwo);

    if (stNumChars > stLen1 || stNumChars > stLen2) {
        return -1;
    }

    return _strnicmp (pszStringOne, pszStringTwo, stNumChars);
}

const char* String::StrStr (const char* pszString, const char* pszCharSet) {

    if (pszString == NULL || pszCharSet == NULL) {
        return NULL;
    }

    return strstr (pszString, pszCharSet);
}

int String::StriStr (const char* pszString, const char* pszCharSet, const char** ppszStrStr) {

    int iErrCode = OK;

    *ppszStrStr = NULL;

    if (pszString == NULL || pszCharSet == NULL) {
        return OK;
    }

    char* pszLwrString = NULL;
    char* pszLwrCharSet = NULL;
    char* pszStrStr = NULL;

    size_t stStringLen = strlen (pszString) + 1;
    size_t stCharSetLen = strlen (pszCharSet) + 1;

    bool fFreeString = false;
    bool fFreeCharSet = false;

    if (stStringLen < 512) {
        pszLwrString = (char*) StackAlloc (stStringLen);
        memcpy (pszLwrString, pszString, stStringLen);
    } else {
        pszLwrString = String::StrDup (pszString);
        fFreeString = true;
    }

    if (pszLwrString == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    if (stStringLen < 512) {
        pszLwrCharSet = (char*) StackAlloc (stCharSetLen);
        memcpy (pszLwrCharSet, pszCharSet, stCharSetLen);
    } else {
        pszLwrCharSet = String::StrDup (pszCharSet);
        fFreeCharSet = true;
    }

    if (pszLwrCharSet == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    String::StrLwr (pszLwrString);
    String::StrLwr (pszLwrCharSet);

    pszStrStr = strstr (pszLwrString, pszLwrCharSet);
    if (pszStrStr != NULL) {
        *ppszStrStr = pszString + (pszStrStr - pszLwrString);
    }

Cleanup:

    if (fFreeString) {
        OS::HeapFree (pszLwrString);
    }

    if (fFreeCharSet) {
        OS::HeapFree (pszLwrCharSet);
    }

    return iErrCode;
}

int String::PreAllocate (size_t stNumChars) {

    if (stNumChars > m_iRealLength) {

        char* pszString = new char [stNumChars + 1];
        if (pszString == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        if (m_pszString != NULL) {
            memcpy (pszString, m_pszString, m_iLength + 1);
            delete [] m_pszString;
        }
        m_pszString = pszString;
        m_iRealLength = stNumChars;

    } else {

        if (m_pszString != NULL) {
            *m_pszString = '\0';
        } else {

            // Alloc 0?
            m_pszString = new char [8];
            if (m_pszString == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }

            m_iRealLength = 7;
        }
        m_iLength = 0;
    }

    return OK;
}

char* String::AtoHtml (const char* pszString, String* pstrHTML, size_t stMaxNumSpacelessChars, 
                       bool bAddMarkups) {

    if (pstrHTML->m_pszString != NULL) {
        pstrHTML->m_pszString[0] = '\0';
    }

    pstrHTML->m_iLength = 0;

    return pstrHTML->AppendHtml (pszString, stMaxNumSpacelessChars, bAddMarkups);
}

char* String::AppendHtml (const char* pszString, size_t stMaxNumSpacelessChars, bool bAddMarkups) {

    size_t stLength = strlen (pszString);
    bool bLaissezFaire;


    // Remove trailing spaces
    // \n -> <br>
    // \t -> &nbsp;&nbsp;&nbsp;&nbsp;
    // < -> &lt;
    // > -> &gt;

    size_t i = 0, j, stCurrentCopyChar = m_iLength, stNumSpacelessChars = 0, stSpaceNeeded;

    // Strip away beginning spaces
    while (pszString[i] == ' ') {

        i ++;
        if (i == stLength) {
            return GetCharPtr();
        }
    }

    for (; i < stLength; i ++) {

        switch (pszString[i]) {

        case '\r':
            break;

        case '\n':

            if (!bAddMarkups) goto DoDefault;

            stSpaceNeeded = stCurrentCopyChar + 5;
            if (stSpaceNeeded > m_iRealLength && PreAllocate (stSpaceNeeded * 2) != OK) {
                return NULL;
            }

            StrNCpy (m_pszString + stCurrentCopyChar, "<br>");
            stCurrentCopyChar += 4;
            m_iLength += 4;
            stNumSpacelessChars = 0;

            break;

        case '\t':

            stSpaceNeeded = stCurrentCopyChar + 25;
            if (stSpaceNeeded > m_iRealLength && PreAllocate (stSpaceNeeded * 2) != OK) {
                return NULL;
            }

            StrNCpy (m_pszString + stCurrentCopyChar, "&nbsp;&nbsp;&nbsp;&nbsp;");
            stCurrentCopyChar += 24;
            m_iLength += 24;
            stNumSpacelessChars = 0;

            break;

        case '\"':

            stSpaceNeeded = stCurrentCopyChar + 6;
            if (stSpaceNeeded > m_iRealLength && PreAllocate (stSpaceNeeded * 2) != OK) {
                return NULL;
            }

            StrNCpy (m_pszString + stCurrentCopyChar, "&#34;");
            stCurrentCopyChar += 5;
            m_iLength += 5;
            stNumSpacelessChars ++;
            break;

        case '<':

            stSpaceNeeded = stCurrentCopyChar + 5;
            if (stSpaceNeeded > m_iRealLength && PreAllocate (stSpaceNeeded * 2) != OK) {
                return NULL;
            }

            StrNCpy (m_pszString + stCurrentCopyChar, "&lt;");
            stCurrentCopyChar += 4;
            m_iLength += 4;
            stNumSpacelessChars ++;
            break;

        case '>':

            stSpaceNeeded = stCurrentCopyChar + 5;
            if (stSpaceNeeded > m_iRealLength && PreAllocate (stSpaceNeeded * 2) != OK) {
                return NULL;
            }

            StrNCpy (m_pszString + stCurrentCopyChar, "&gt;");
            stCurrentCopyChar += 4;
            m_iLength += 4;
            stNumSpacelessChars ++;
            break;

        default:
DoDefault:
            stSpaceNeeded = stCurrentCopyChar + 2;
            if (stSpaceNeeded > m_iRealLength && PreAllocate (stSpaceNeeded * 2) != OK) {
                return NULL;
            }

            m_pszString[stCurrentCopyChar] = pszString[i];
            m_pszString[++ stCurrentCopyChar] = '\0';

            m_iLength ++;

            if (pszString[i] == ' ') {
                stNumSpacelessChars = 0;
            } else {
                stNumSpacelessChars ++;
            }
            break;
        }

        if (stNumSpacelessChars == stMaxNumSpacelessChars && stMaxNumSpacelessChars != 0) {

            // Look forward, maybe there's a space coming up
            j = i;
            bLaissezFaire = false;
            while (j < i + 5 && j < stLength) {
                if (pszString[j] == ' ' || pszString[j] == '\n' || pszString[j] == '\t') {
                    bLaissezFaire = true;
                    break;
                }
                j ++;
            }
            
            if (!bLaissezFaire) {
                
                // Insert an extra space
                stSpaceNeeded = stCurrentCopyChar + 2;
                if (stSpaceNeeded > m_iRealLength && PreAllocate (stSpaceNeeded * 2) != OK) {
                    return NULL;
                }
                
                stNumSpacelessChars = 0;
                m_pszString[stCurrentCopyChar] = '\n';
                m_pszString[++ stCurrentCopyChar] = '\0';
                m_iLength ++;
            }
        }
    }

    // Strip away trailing spaces
    while (m_iLength > 0) {
            
        i = m_iLength - 1;

        if (m_pszString[i] == ' ' || 
            m_pszString[i] == '\t') {

            m_pszString[i] = '\0';
            m_iLength --;

        } else {
            break;
        }
    }

    return GetCharPtr();
}

bool String::IsBlank (const char* pszString) {

    return pszString == NULL || *pszString == '\0';
}

bool String::IsWhiteSpace (const char* pszString) {

    // We'll consider a blank string to be whitespace
    if (pszString == NULL) {
        return true;
    }

    size_t i = 0;
    while (true) {

        char c = pszString[i ++];
        if (c == ' ' || c == '\t') {
            continue;
        }
        if (c == '\0') {
            return true;
        }

        return false;
    }
}