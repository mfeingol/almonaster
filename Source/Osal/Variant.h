// Variant.h: interface for the Variant class.
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

#if !defined(AFX_VARIANT_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_)
#define AFX_VARIANT_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_

#include "String.h"

union VariantArg {
    int iArg;
    float fArg;
    char* pszArg;
    int64 i64Arg;
};

enum VariantType { V_STRING, V_INT, V_FLOAT, V_RESERVED, V_INT64 };

class OSAL_EXPORT Variant {
public:
    
    VariantArg m_vArg;
    VariantType m_iType;

    inline bool DeleteString();

public:

    Variant();
    Variant (const Variant& vRhs);
    Variant (int iVal);
    Variant (unsigned int iVal);
    Variant (float fVal);
    Variant (double dVal);
    Variant (const char* pszString);
    Variant (int64 i64Val);

    ~Variant();

    void Initialize();

    // =
    Variant& operator= (const Variant& vRhs);
    Variant& operator= (int iVal);
    Variant& operator= (unsigned int iVal);
    Variant& operator= (float fVal);
    Variant& operator= (const char* pszString);
    Variant& operator= (int64 i64Val);

    // +=
    Variant& operator+= (const Variant& vRhs);
    Variant& operator+= (int iRhs);
    Variant& operator+= (unsigned int iRhs);
    Variant& operator+= (float fRhs);
    Variant& operator+= (const char* pszRhs);
    Variant& operator+= (int64 i64Rhs);

    // +
    Variant operator+ (const Variant& vRhs);

    // ++
    Variant operator++();
    Variant operator++ (int iPostFix);

    // -=
    Variant& operator-= (const Variant& vRhs);
    Variant& operator-= (int iRhs);
    Variant& operator-= (float fRhs);
    Variant& operator-= (int64 i64Rhs);

    // -
    Variant operator- (const Variant& vRhs);
    Variant operator-();

    // ++
    Variant operator--();
    Variant operator-- (const int iPostFix);

    // *=
    Variant& operator*= (const Variant& vRhs);
    Variant& operator*= (int iRhs);
    Variant& operator*= (float fRhs);
    Variant& operator*= (int64 i64Rhs);

    // *
    Variant operator* (const Variant& vRhs);
    
    // /=
    Variant& operator/= (const Variant& vRhs);
    Variant& operator/= (int iRhs);
    Variant& operator/= (float fRhs);
    Variant& operator/= (int64 i64Rhs);

    // /
    Variant operator/ (const Variant& vRhs);

    // Casts
    operator int() const;
    operator unsigned int() const;
    operator String() const;
    operator const char*() const;
    operator float() const;
    operator int64() const;

    // Public interface
    VariantType GetType() const;
    size_t GetLength() const;
    void Clear();

    size_t GetSize() const;

    int GetInteger() const;
    float GetFloat() const;
    String GetString() const;
    const char* GetCharPtr() const;
    int64 GetInteger64() const;
    
    OSAL_EXPORT friend bool operator== (const Variant& vLhs, const Variant& vRhs);
    OSAL_EXPORT friend bool operator== (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator== (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend bool operator== (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend bool operator== (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend bool operator== (const Variant& vVariant, const char* pszChar);
    OSAL_EXPORT friend bool operator== (const char* pszChar, const Variant& vVariant);
    OSAL_EXPORT friend bool operator== (const Variant& vVariant, unsigned int iInt);
    OSAL_EXPORT friend bool operator== (unsigned int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator== (const Variant& vVariant, int64 i64Int);
    OSAL_EXPORT friend bool operator== (int64 i64Int, const Variant& vVariant);
    
    OSAL_EXPORT friend bool operator!= (const Variant& vLhs, const Variant& vRhs);
    OSAL_EXPORT friend bool operator!= (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator!= (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend bool operator!= (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend bool operator!= (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend bool operator!= (const Variant& vVariant, const char* pszChar);
    OSAL_EXPORT friend bool operator!= (const char* pszChar, const Variant& vVariant);
    OSAL_EXPORT friend bool operator!= (const Variant& vVariant, unsigned int iInt);
    OSAL_EXPORT friend bool operator!= (unsigned int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator!= (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend bool operator!= (const Variant& vVariant, int64 i64Int);
    
    OSAL_EXPORT friend bool operator> (const Variant& vLhs, const Variant& vRhs);
    OSAL_EXPORT friend bool operator> (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator> (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend bool operator> (unsigned int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator> (const Variant& vVariant, unsigned int iInt);
    OSAL_EXPORT friend bool operator> (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend bool operator> (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend bool operator> (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend bool operator> (const Variant& vVariant, int64 i64Int);
    
    OSAL_EXPORT friend bool operator>= (const Variant& vLhs, const Variant& vRhs);
    OSAL_EXPORT friend bool operator>= (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator>= (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend bool operator>= (unsigned int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator>= (const Variant& vVariant, unsigned int iInt);
    OSAL_EXPORT friend bool operator>= (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend bool operator>= (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend bool operator>= (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend bool operator>= (const Variant& vVariant, int64 i64Int);
    
    OSAL_EXPORT friend bool operator< (const Variant& vLhs, const Variant& vRhs);
    OSAL_EXPORT friend bool operator< (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator< (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend bool operator< (unsigned int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator< (const Variant& vVariant, unsigned int iInt);
    OSAL_EXPORT friend bool operator< (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend bool operator< (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend bool operator< (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend bool operator< (const Variant& vVariant, int64 i64Int);
    
    OSAL_EXPORT friend bool operator<= (const Variant& vLhs, const Variant& vRhs);
    OSAL_EXPORT friend bool operator<= (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator<= (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend bool operator<= (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend bool operator<= (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend bool operator<= (unsigned int iInt, const Variant& vVariant);
    OSAL_EXPORT friend bool operator<= (const Variant& vVariant, unsigned int iInt);
    OSAL_EXPORT friend bool operator<= (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend bool operator<= (const Variant& vVariant, int64 i64Int);
    
    OSAL_EXPORT friend int operator- (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend int operator- (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend float operator- (const float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend float operator- (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend int64 operator- (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend int64 operator- (const Variant& vVariant, int64 i64Int);

    OSAL_EXPORT friend int operator+ (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend int operator+ (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend float operator+ (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend float operator+ (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend int64 operator+ (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend int64 operator+ (const Variant& vVariant, int64 i64Int);

    OSAL_EXPORT friend int operator+= (int& iInt, const Variant& vVariant);
    OSAL_EXPORT friend float operator+= (float& fFloat, const Variant& vVariant);
    OSAL_EXPORT friend int64 operator+= (int64& iInt64, const Variant& vVariant);

    OSAL_EXPORT friend int operator-= (int& iInt, const Variant& vVariant);
    OSAL_EXPORT friend float operator-= (float& fFloat, const Variant& vVariant);
    OSAL_EXPORT friend int64 operator-= (int64& i64Int, const Variant& vVariant);

    OSAL_EXPORT friend int operator* (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend int operator* (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend unsigned int operator* (unsigned int iInt, const Variant& vVariant);
    OSAL_EXPORT friend unsigned int operator* (const Variant& vVariant, unsigned int iInt);
    OSAL_EXPORT friend float operator* (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend float operator* (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend int64 operator* (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend int64 operator* (const Variant& vVariant, int64 i64Int);
    
    OSAL_EXPORT friend int operator/ (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend int operator/ (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend float operator/ (float fFloat, const Variant& vVariant);
    OSAL_EXPORT friend float operator/ (const Variant& vVariant, float fFloat);
    OSAL_EXPORT friend int64 operator/ (int64 i64Int, const Variant& vVariant);
    OSAL_EXPORT friend int64 operator/ (const Variant& vVariant, int64 i64Int);

    OSAL_EXPORT friend int operator% (int iInt, const Variant& vVariant);
    OSAL_EXPORT friend int operator% (const Variant& vVariant, int iInt);
    OSAL_EXPORT friend int64 operator% (int64 iInt, const Variant& vVariant);
    OSAL_EXPORT friend int64 operator% (const Variant& vVariant, int64 i64Int);

    static bool IsValidType (VariantType vtType);
};

#endif // !defined(AFX_VARIANT_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_)