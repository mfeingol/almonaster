// String.h: interface for the String class.
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

#if !defined(AFX_STRING_H__DEA27452_CFBF_11D1_A76C_0000C07E3CEF__INCLUDED_)
#define AFX_STRING_H__DEA27452_CFBF_11D1_A76C_0000C07E3CEF__INCLUDED_

#define INVALID_CHAR ((unsigned char) 0xff)

#include "OS.h"

#define StrNCpy(x, y) strncpy (x, y, sizeof (y))
#define StrNCat(x, y) strncpy (x + strlen (x), y, sizeof (y))

#ifdef __WIN32__
#define snprintf _snprintf
#endif

class OSAL_EXPORT String {

private:

    char* m_pszString;
    size_t m_iLength;
    size_t m_iRealLength;

    String& AddString (const char* pszString, size_t stLength);

    static char* CopyWithZeroes (char* pszString, char* pszBuffer, unsigned int iDigits);

public:
    
    String();
    String (const String& strString);
    String (const char* pszString);
    String (int iInt);
    String (unsigned int iInt);
    String (int64 i64Int);
    String (float fFloat);

    int PreAllocate (size_t stNumChars);

    ~String();

    char* GetCharPtr() const;
    size_t GetLength() const;

    bool operator== (const String& strString);
    bool operator!= (const String& strString);

    String& operator= (const String& strString);
    String& operator= (const char* pszString);

    String& operator+= (const String& strString);
    String& operator+= (const char* pszString);
    String& operator+= (char pszString[]);
    String& operator+= (int iInt);
    String& operator+= (unsigned int iInt);
    String& operator+= (float fFloat);

    operator char*() const;
    operator const char*() const;

    bool IsBlank() const;
    bool Equals (const String& strComp) const;
    bool IEquals (const String& strComp) const;

    char GetCharAt (unsigned int iIndex) const;
    void SetCharAt (unsigned int iIndex, char szChar);

    char GetLastChar() const;

    char* AppendHtml (const char* pszString, size_t stMaxNumSpacelessChars, bool bAddMarkups);

    void Clear();

    static int AtoI (const char* pszString);
    static unsigned int AtoUI (const char* pszString);

    static int64 AtoI64 (const char* pszString);
    static uint64 AtoUI64 (const char* pszString);
    
    static float AtoF (const char* pszString);

    static char* UItoA (unsigned int iData, char* pszString, int iRadix);
    static char* UItoA (unsigned int iData, char* pszString, int iRadix, unsigned int iDigits);
    static char* ItoA (int iData, char* pszString, int iRadix);
    static char* ItoA (int iData, char* pszString, int iRadix, unsigned int iDigits);

    static char* UI64toA (uint64 iData, char* pszString, int iRadix);
    static char* UI64toA (uint64 iData, char* pszString, int iRadix, unsigned int iDigits);
    static char* I64toA (int64 iData, char* pszString, int iRadix);
    static char* I64toA (int64 iData, char* pszString, int iRadix, unsigned int iDigits);

    static char* AtoHtml (const char* pszString, String* pstrHTML, size_t stMaxNumSpacelessChars, bool bAddMarkups);

    static size_t StrLen (const char* pszString);
    static char* StrDup (const char* pszString);
    
    static char* StrCpy (char* pszDest, const char* pszSrc);
    static char* StrnCpy (char* pszDest, const char* pszSrc, size_t stLen);

    static char* StrCat (char* pszDest, const char* pszSrc);
    static char* StrnCat (char* pszDest, const char* pszSrc, size_t stLen);

    static void StrLwr (char* pszString);

    static int StrCmp (const char* pszStringOne, const char* pszStringTwo);
    static int StriCmp (const char* pszStringOne, const char* pszStringTwo);

    static int StrnCmp (const char* pszStringOne, const char* pszStringTwo, size_t stNumChars);
    static int StrniCmp (const char* pszStringOne, const char* pszStringTwo, size_t stNumChars);

    static const char* StrStr (const char* pszString, const char* pszCharSet);
    static int StriStr (const char* pszString, const char* pszCharSet, const char** ppszStrStr);

    static bool IsBlank (const char* pszString);
};

EXPORT String operator+ (const String& strLhs, const String& strRhs);
EXPORT String operator+ (const String& strLhs, const int iRhs);
EXPORT String operator+ (int iLhs, const String& strRhs);

EXPORT String operator+ (const String& strLhs, const unsigned int iRhs);
EXPORT String operator+ (unsigned int iLhs, const String& strRhs);

EXPORT String operator+ (const String& strLhs, const float fRhs);
EXPORT String operator+ (float fLhs, const String& strRhs);
EXPORT String operator+ (const String& strLhs, const char* pszRhs);
EXPORT String operator+ (const char* pszLhs, const String& strRhs);

#endif // !defined(AFX_STRING_H__DEA27452_CFBF_11D1_A76C_0000C07E3CEF__INCLUDED_)
