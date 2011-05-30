// Variant.cpp: implementation of the Variant class.
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
#include "Variant.h"
#undef OSAL_BUILD

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Variant::Variant() {
    m_iType = V_INT;
    m_vArg.iArg = 0;
}

Variant::Variant (const Variant& vRhs) {
    
    m_iType = vRhs.m_iType;

    switch (m_iType) {

    case V_STRING:
        
        if (vRhs.m_vArg.pszArg == NULL) {
            m_vArg.pszArg = NULL;
        } else {

            size_t stLength = strlen (vRhs.m_vArg.pszArg) + 1;
            m_vArg.pszArg = new char [stLength];

            if (m_vArg.pszArg != NULL) {
                strncpy (m_vArg.pszArg, vRhs.m_vArg.pszArg, stLength);
            }
        }
        break;

    default:

        m_vArg = vRhs.m_vArg;
        Assert (m_iType >= V_INT && m_iType <= V_INT64);
        break;
    }
}

Variant::Variant (const char* pszString) {
    
    m_iType = V_STRING;

    if (pszString == NULL) {
        m_vArg.pszArg = NULL;
    } else {

        size_t stLength = strlen (pszString) + 1;
        m_vArg.pszArg = new char [stLength];
        
        if (m_vArg.pszArg != NULL) {
            strncpy (m_vArg.pszArg, pszString, stLength);
        }
    }
}

Variant::Variant (int iVal) {

    m_iType = V_INT;
    m_vArg.iArg = iVal;
}

Variant::Variant (unsigned int iVal) {

    m_iType = V_INT;
    m_vArg.iArg = (int) iVal;
}

Variant::Variant (float fVal) {
    m_iType = V_FLOAT;
    m_vArg.fArg = fVal;
}

Variant::Variant (double dVal) {
    m_iType = V_FLOAT;
    m_vArg.fArg = (float) dVal;
}

Variant::Variant (const UTCTime& tTime) {
    m_iType = V_TIME;
    m_vArg.tArg = tTime;
}

Variant::Variant (int64 i64Val) {
    m_iType = V_INT64;
    m_vArg.i64Arg = i64Val;
}


Variant::~Variant() {
    DeleteString();
}


// Delete Variant String member
bool Variant::DeleteString() {
    
    if (m_iType == V_STRING && m_vArg.pszArg != NULL) {
        delete [] m_vArg.pszArg;
        return true;
    }

    return false;
}


///////////////
// Overloads //
///////////////

// =
Variant& Variant::operator= (const Variant& vRhs) {
    
    if (this != &vRhs) {
        
        // Clean up old string
        DeleteString();
        
        // Copy type
        m_iType = vRhs.m_iType;
        
        // Copy data
        if (m_iType == V_STRING && vRhs.m_vArg.pszArg != NULL) {

            size_t stLength = strlen (vRhs.m_vArg.pszArg) + 1;
            m_vArg.pszArg = new char [stLength];

            if (m_vArg.pszArg != NULL) {
                strncpy (m_vArg.pszArg, vRhs.m_vArg.pszArg, stLength);
            }

        } else {
            m_vArg = vRhs.m_vArg;
        }
    }

    return *this;
}


Variant& Variant::operator= (const char* pszString) {

    // Clean up old string
    DeleteString();

    m_iType = V_STRING;
    if (pszString == NULL) {
        m_vArg.pszArg = NULL;
    } else {

        // Assign new string
        size_t stLength = strlen (pszString) + 1;
        m_vArg.pszArg = new char [stLength];
        
        if (m_vArg.pszArg != NULL) {
            strncpy (m_vArg.pszArg, pszString, stLength);
        }
    }

    return *this;
}

Variant& Variant::operator= (int iVal) {
    
    // Clean up old string
    DeleteString();

    // Copy data
    m_iType = V_INT;
    m_vArg.iArg = iVal;

    return *this;
}

Variant& Variant::operator= (unsigned int iVal) {
    
    // Clean up old string
    DeleteString();

    // Copy data
    m_iType = V_INT;
    m_vArg.iArg = (int) iVal;

    return *this;
}


Variant& Variant::operator= (float fVal) {
    
    // Clean up old string
    DeleteString();

    // Copy data
    m_iType = V_FLOAT;
    m_vArg.fArg = fVal;

    return *this;
}


Variant& Variant::operator= (const UTCTime& tTime) {
    
    // Clean up old string
    DeleteString();

    // Copy data
    m_iType = V_TIME;
    m_vArg.tArg = tTime;

    return *this;
}


Variant& Variant::operator= (int64 i64Val) {
    
    // Clean up old string
    DeleteString();

    // Copy data
    m_iType = V_INT64;
    m_vArg.i64Arg = i64Val;

    return *this;
}

// +=
Variant& Variant::operator+= (const Variant& vRhs) {
    
    char* pszTemp;

    if (m_iType == vRhs.m_iType) {

        switch (m_iType) {
            
        case V_INT:
            m_vArg.iArg += vRhs.m_vArg.iArg;
            break;

        case V_INT64:
            m_vArg.i64Arg += vRhs.m_vArg.i64Arg;
            break;
            
        case V_FLOAT:
            m_vArg.fArg += vRhs.m_vArg.fArg;
            break;
            
        case V_STRING:
            
            if (vRhs.m_vArg.pszArg != NULL) {

                if (m_vArg.pszArg == NULL) {

                    size_t stLength = strlen (vRhs.m_vArg.pszArg) + 1;

                    m_vArg.pszArg = new char [stLength];
                    if (m_vArg.pszArg != NULL) {
                        strncpy (m_vArg.pszArg, vRhs.m_vArg.pszArg, stLength);
                    }
                
                } else {
                    
                    size_t stLength = strlen (m_vArg.pszArg);
                    size_t stLengthRhs = strlen (vRhs.m_vArg.pszArg) + 1;
                
                    pszTemp = new char [stLength + stLengthRhs];
                    if (pszTemp != NULL) {
                    
                        strncpy (pszTemp, m_vArg.pszArg, stLength + 1);
                        strncpy (pszTemp + stLength, vRhs.m_vArg.pszArg, stLengthRhs);
                        
                        delete [] m_vArg.pszArg;
                        m_vArg.pszArg = pszTemp;
                    }
                }
            }

            break;
        
        default:
            Assert (false);
        }
    }

    else if (m_iType == V_STRING) {
        
        char pszBuffer [128];
        
        if (vRhs.m_iType == V_INT) {
            itoa (vRhs.m_vArg.iArg, pszBuffer, 10);
        } else if (vRhs.m_iType == V_INT64) {
            _i64toa (vRhs.m_vArg.i64Arg, pszBuffer, 10);
        } else {
            Assert (vRhs.m_iType == V_FLOAT);
            sprintf (pszBuffer, "%f", vRhs.m_vArg.fArg);
        }
        
        if (m_vArg.pszArg != NULL) {

            size_t stLength = strlen (m_vArg.pszArg);
            size_t stLengthRhs = strlen (pszBuffer) + 1;
            
            pszTemp = new char [stLength + stLengthRhs];
            if (pszTemp != NULL) {
                
                strncpy (pszTemp, m_vArg.pszArg, stLength + 1);
                strncpy (pszTemp + stLength, pszBuffer, stLengthRhs);
                
                delete [] m_vArg.pszArg;
            }

        } else {

            size_t stLength = strlen (pszBuffer) + 1;

            pszTemp = new char [stLength];
            if (pszTemp != NULL) {
                strncpy (pszTemp, pszBuffer, stLength);
            }
        }

        m_vArg.pszArg = pszTemp;
    }

    else if (vRhs.m_iType == V_STRING) {

        char pszBuffer [128];
        
        if (m_iType == V_INT) {
            itoa (m_vArg.iArg, pszBuffer, 10);
        } else if (m_iType == V_INT64) {
            _i64toa (vRhs.m_vArg.i64Arg, pszBuffer, 10);
        } else {
            Assert (vRhs.m_iType == V_FLOAT);
            sprintf (pszBuffer, "%f", m_vArg.fArg);
        }
        
        if (vRhs.m_vArg.pszArg != NULL) {

            size_t stLengthRhs = strlen (vRhs.m_vArg.pszArg) + 1;
            size_t stLength = strlen (pszBuffer);
            
            pszTemp = new char [stLength + stLengthRhs];
            if (pszTemp != NULL) {
                
                strncpy (pszTemp, pszBuffer, stLength + 1);
                strncpy (pszTemp + stLength, vRhs.m_vArg.pszArg, stLengthRhs);
                
                delete [] m_vArg.pszArg;
            }

        } else {

            size_t stLength = strlen (pszBuffer) + 1;

            pszTemp = new char [stLength];
            if (pszTemp != NULL) {
                strncpy (pszTemp, pszBuffer, stLength);
            }
        }

        m_iType = V_STRING;
        m_vArg.pszArg = pszTemp;
    }
        
    else if (m_iType == V_TIME && vRhs.m_iType == V_INT) {
        Time::AddSeconds (m_vArg.tArg, (Seconds) vRhs.m_vArg.iArg, &m_vArg.tArg);
    }

    else if (m_iType == V_TIME && vRhs.m_iType == V_INT64) {
        Time::AddSeconds (m_vArg.tArg, (Seconds) vRhs.m_vArg.i64Arg, &m_vArg.tArg);
    }
        
    else Assert (false);

    return *this;
}

Variant& Variant::operator+= (int iRhs) {

    if (m_iType == V_INT) {
        m_vArg.iArg += iRhs;
    }

    else if (m_iType == V_INT64) {
        m_vArg.i64Arg += iRhs;
    }

    else if (m_iType == V_TIME) {
        Time::AddSeconds (m_vArg.tArg, iRhs, &m_vArg.tArg);
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator+= (unsigned int iRhs) {

    if (m_iType == V_INT) {
        m_vArg.iArg += iRhs;
    }

    else if (m_iType == V_INT64) {
        m_vArg.i64Arg += iRhs;
    }

    else if (m_iType == V_TIME) {
        Time::AddSeconds (m_vArg.tArg, iRhs, &m_vArg.tArg);
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator+= (float fRhs) {

    if (m_iType == V_FLOAT) {
        m_vArg.fArg += fRhs;
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator+= (const char* pszRhs) {

    if (m_iType == V_STRING && pszRhs != NULL) {

        if (m_vArg.pszArg == NULL) {

            size_t stLength = strlen (pszRhs) + 1;

            m_vArg.pszArg = new char [stLength];
            strncpy (m_vArg.pszArg, pszRhs, stLength);
        
        } else {

            size_t stLength = strlen (m_vArg.pszArg);
            size_t stLengthRhs = strlen (pszRhs) + 1;

            char* pszTemp = new char [stLength + stLengthRhs];
            if (pszTemp != NULL) {

                strncpy (pszTemp, m_vArg.pszArg, stLength + 1);
                strncpy (pszTemp + stLength, pszRhs, stLengthRhs);

                delete [] m_vArg.pszArg;
                m_vArg.pszArg = pszTemp;
            }
        }
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator+= (int64 i64Rhs) {

    if (m_iType == V_INT) {
        m_vArg.iArg += (int) i64Rhs;
    }

    else if (m_iType == V_INT64) {
        m_vArg.i64Arg += i64Rhs;
    }

    else if (m_iType == V_TIME) {
        Time::AddSeconds (m_vArg.tArg, (Seconds) i64Rhs, &m_vArg.tArg);
    }

    else Assert (false);

    return *this;
}

// +
Variant Variant::operator+ (const Variant& vRhs) {

    Variant vTemp = *this;
    return vTemp += vRhs;
}

Variant Variant::operator++() {

    switch (m_iType) {

    case V_INT:
        m_vArg.iArg ++;
        break;
    case V_INT64:
        m_vArg.i64Arg ++;
        break;
    case V_FLOAT:
        m_vArg.fArg += (float) 1.0;
        break;
    case V_STRING:
        Assert (false);
        break;
    }
    return *this;
}

Variant Variant::operator++ (int iPostFix) {

    switch (m_iType) {

    case V_INT:
        m_vArg.iArg ++;
        break;
    case V_INT64:
        m_vArg.i64Arg ++;
        break;
    case V_FLOAT:
        m_vArg.fArg += (float) 1.0;
        break;
    case V_STRING:
        Assert (false);
        break;
    }
    return *this;
}

// -=
Variant& Variant::operator-= (const Variant& vRhs) {
    
    if (m_iType == vRhs.m_iType) {

        switch (m_iType) {
            
        case V_INT:
            m_vArg.iArg -= vRhs.m_vArg.iArg;
            break;

        case V_INT64:
            m_vArg.i64Arg -= vRhs.m_vArg.i64Arg;
            break;
            
        case V_FLOAT:
            m_vArg.fArg -= vRhs.m_vArg.fArg;
            break;

        default:
            Assert (false);
        }
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator-= (int iRhs) {

    if (m_iType == V_INT) {
        m_vArg.iArg -= iRhs;
    }

    else if (m_iType == V_INT64) {
        m_vArg.i64Arg -= iRhs;
    }

    else if (m_iType == V_TIME) {
        Time::AddSeconds (m_vArg.tArg, -iRhs, &m_vArg.tArg);
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator-= (float fRhs) {

    if (m_iType == V_FLOAT) {
        m_vArg.fArg -= fRhs;
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator-= (int64 i64Rhs) {

    if (m_iType == V_INT) {
        m_vArg.iArg -= (int) i64Rhs;
    }

    else if (m_iType == V_INT64) {
        m_vArg.i64Arg -= i64Rhs;
    }

    else if (m_iType == V_TIME) {
        Time::AddSeconds (m_vArg.tArg, (Seconds) -i64Rhs, &m_vArg.tArg);
    }

    else Assert (false);

    return *this;
}

// -
Variant Variant::operator- (const Variant& vRhs) {

    Variant vTemp = *this;
    return vTemp -= vRhs;
}

Variant Variant::operator-() {

    Variant vRetVal;
    
    switch (m_iType) {
    
    case V_INT:
        vRetVal = -m_vArg.iArg;
        return vRetVal;

    case V_INT64:
        vRetVal = -m_vArg.i64Arg;
        return vRetVal;
    
    case V_FLOAT:
        vRetVal = -m_vArg.fArg;
        return vRetVal;
    }

    Assert (false);
    return vRetVal; 
}

Variant Variant::operator--() {

    switch (m_iType) {
    case V_INT:
        m_vArg.iArg --;
        break;
    case V_INT64:
        m_vArg.i64Arg --;
        break;
    case V_FLOAT:
        m_vArg.fArg -= (float) 1.0;
        break;
    case V_STRING:
        Assert (false);
        break;
    }
    return *this;
}

Variant Variant::operator-- (int iPostFix) {

    switch (m_iType) {
    case V_INT:
        m_vArg.iArg --;
        break;
    case V_INT64:
        m_vArg.i64Arg --;
        break;
    case V_FLOAT:
        m_vArg.fArg -= (float) 1.0;
        break;
    case V_STRING:
        Assert (false);
        break;
    }
    return *this;
}

// *=
Variant& Variant::operator*= (const Variant& vRhs) {
    
    if (m_iType == vRhs.m_iType) {

        switch (m_iType) {
            
        case V_INT:
            m_vArg.iArg *= vRhs.m_vArg.iArg;
            break;

        case V_INT64:
            m_vArg.i64Arg *= vRhs.m_vArg.i64Arg;
            break;
            
        case V_FLOAT:
            m_vArg.fArg *= vRhs.m_vArg.fArg;
            break;

        default:
            Assert (false);
        }
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator*= (int iRhs) {

    if (m_iType == V_INT) {
        m_vArg.iArg *= iRhs;
    }

    else if (m_iType == V_INT64) {
        m_vArg.i64Arg *= iRhs;
    }

    else if (m_iType == V_FLOAT) {
        m_vArg.fArg *= (float) iRhs;
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator*= (float fRhs) {

    if (m_iType == V_FLOAT) {
        m_vArg.fArg *= fRhs;
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator*= (int64 i64Rhs) {

    if (m_iType == V_INT64) {
        m_vArg.i64Arg *= i64Rhs;
    }

    else if (m_iType == V_INT) {
        m_vArg.iArg *= (int) i64Rhs;
    }

    else if (m_iType == V_FLOAT) {
        m_vArg.fArg *= (float) i64Rhs;
    }

    else Assert (false);

    return *this;
}

// *
Variant Variant::operator* (const Variant& vRhs) {

    Variant vTemp = *this;
    return vTemp *= vRhs;
}

// /=
Variant& Variant::operator/= (const Variant& vRhs) {
    
    if (m_iType == vRhs.m_iType) {

        switch (m_iType) {
            
        case V_INT:
            m_vArg.iArg /= vRhs.m_vArg.iArg;
            break;

        case V_INT64:
            m_vArg.i64Arg /= vRhs.m_vArg.i64Arg;
            break;
            
        case V_FLOAT:
            m_vArg.fArg /= vRhs.m_vArg.fArg;
            break;

        default:
            Assert (false);
        }
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator/= (int iRhs) {

    if (m_iType == V_INT) {
        m_vArg.iArg /= iRhs;
    }

    else if (m_iType == V_INT64) {
        m_vArg.i64Arg /= iRhs;
    }

    else if (m_iType == V_FLOAT) {
        m_vArg.fArg /= (float) iRhs;
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator/= (float fRhs) {

    if (m_iType == V_FLOAT) {
        m_vArg.fArg /= fRhs;
    }

    else Assert (false);

    return *this;
}

Variant& Variant::operator/= (int64 i64Rhs) {

    if (m_iType == V_INT) {
        m_vArg.iArg /= (int) i64Rhs;
    }

    else if (m_iType == V_INT64) {
        m_vArg.i64Arg /= i64Rhs;
    }

    else if (m_iType == V_FLOAT) {
        m_vArg.fArg /= (float) i64Rhs;
    }

    else Assert (false);

    return *this;
}

// /
Variant Variant::operator/ (const Variant& vRhs) {
    Variant vTemp = *this;
    return vTemp /= vRhs;
}


// Casts
Variant::operator int() const {

    switch (m_iType) {
    case V_INT:
        return m_vArg.iArg;
    case V_INT64:
        return (int) m_vArg.i64Arg;
    case V_FLOAT:
        return (int) m_vArg.fArg;
    case V_STRING:
        return m_vArg.pszArg == NULL ? 0 : atoi (m_vArg.pszArg);
    case V_TIME:
        return (int) m_vArg.tArg;
    default:
        Assert (false);
        return -1;
    }
}

Variant::operator unsigned int() const {

    switch (m_iType) {
    case V_INT:
        return (unsigned int) m_vArg.iArg;
    case V_INT64:
        return (unsigned int) m_vArg.i64Arg;
    case V_FLOAT:
        return (unsigned int) m_vArg.fArg;
    case V_STRING:
        return m_vArg.pszArg == NULL ? 0 : (unsigned int) atoi (m_vArg.pszArg);
    case V_TIME:
        return (unsigned int) m_vArg.tArg;
    default:
        Assert (false);
        return 0;
    }
}

Variant::operator String() const {

    switch (m_iType) {
    case V_INT:
        return String (m_vArg.iArg);
    case V_FLOAT:
        return String (m_vArg.fArg);
    case V_STRING:
        return m_vArg.pszArg;
    case V_TIME:
        char pszTime [512];
        Time::UTCTimetoA (m_vArg.tArg, pszTime, 10);
        return String (pszTime);
    default:
        Assert (false);
        return "Error";
    }
}

Variant::operator const char*() const {

    Assert (m_iType == V_STRING);
    return m_vArg.pszArg;
}

Variant::operator float() const {
    switch (m_iType) {
    case V_INT:
        return (float) m_vArg.iArg;
    case V_FLOAT:
        return m_vArg.fArg;
    case V_STRING:
        return m_vArg.pszArg == NULL ? 0 : (float) atof (m_vArg.pszArg);
    default:
        Assert (false);
        return (float) 0xffffffff;
    }
}

Variant::operator const UTCTime&() const {

    switch (m_iType) {
    case V_INT:
        return (UTCTime&) m_vArg.iArg;
    case V_INT64:
        return (UTCTime&) m_vArg.i64Arg;
    case V_TIME:
        return m_vArg.tArg;
    default:
        Assert (false);
        return (UTCTime&) m_vArg.tArg;
    }
}

Variant::operator int64() const {

    switch (m_iType) {
    case V_INT:
        return (int64) m_vArg.iArg;
    case V_INT64:
        return m_vArg.i64Arg;
    case V_FLOAT:
        return (int64) m_vArg.fArg;
    case V_STRING:
        return m_vArg.pszArg == NULL ? 0 : String::AtoI64 (m_vArg.pszArg);
    case V_TIME:
        return (int64) m_vArg.tArg;
    default:
        Assert (false);
        return -1;
    }
}

//////////////////////
// Public interface //
//////////////////////

// Return Variant type
VariantType Variant::GetType() const {
    return m_iType;
}

// Return Variant length (if string)
size_t Variant::GetLength() const {
    
    if (m_iType != V_STRING) {
        Assert (false);
    }
    if (m_vArg.pszArg == NULL) {
        return 0;
    }

    return strlen (m_vArg.pszArg);
}

// Return size of the data
size_t Variant::GetSize() const {

    size_t stSize = sizeof (Variant);

    if (m_iType == V_STRING) {
        stSize += String::StrLen (m_vArg.pszArg);
    }

    return stSize;
}

// Reset Variant
void Variant::Clear() {
    
    // Clean up string
    DeleteString();

    // Reset to default
    m_vArg.iArg = 0;
    m_iType = V_INT;
}


int Variant::GetInteger() const {
    Assert (m_iType == V_INT);
    return m_vArg.iArg;
}

float Variant::GetFloat() const {
    Assert (m_iType == V_FLOAT);
    return m_vArg.fArg;
}

String Variant::GetString() const {
    Assert (m_iType == V_STRING);
    return m_vArg.pszArg;
}

const char* Variant::GetCharPtr() const {
    Assert (m_iType == V_STRING);
    return m_vArg.pszArg;
}

const UTCTime& Variant::GetUTCTime() const {
    Assert (m_iType == V_TIME); 
    return m_vArg.tArg;
}

int64 Variant::GetInteger64() const {
    Assert (m_iType == V_INT64);
    return m_vArg.i64Arg;
}

///////////////////////////
// Independent operators //
///////////////////////////

// ==
bool operator== (const Variant& vLhs, const Variant& vRhs) {
    if (vLhs.m_iType != vRhs.m_iType) {
        Assert (false);
        return false;
    }

    switch (vLhs.m_iType) {

    case V_INT:
        return vLhs.m_vArg.iArg == vRhs.m_vArg.iArg;

    case V_FLOAT:
        return vLhs.m_vArg.fArg == vRhs.m_vArg.fArg;

    case V_STRING:
        if (vLhs.m_vArg.pszArg == NULL) {
            return vRhs.m_vArg.pszArg == NULL;
        }
        if (vRhs.m_vArg.pszArg == NULL) {
            return vLhs.m_vArg.pszArg == NULL;
        }
        return strcmp (vLhs.m_vArg.pszArg, vRhs.m_vArg.pszArg) == 0;

    case V_TIME:
        return vLhs.m_vArg.tArg == vRhs.m_vArg.tArg;

    case V_INT64:
        return vLhs.m_vArg.i64Arg == vRhs.m_vArg.i64Arg;

    default:
        Assert (false);
        return false;
    }
}

bool operator== (int iInt, const Variant& vVariant) {
    return iInt == vVariant.GetInteger();
}

bool operator== (const Variant& vVariant, int iInt) {
    return vVariant.GetInteger() == iInt;
}

bool operator== (float fFloat, const Variant& vVariant) {
    return fFloat == vVariant.GetFloat();
}

bool operator== (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() == fFloat;
}

bool operator== (const Variant& vVariant, const char* pszChar) {
    return (strcmp (vVariant.GetCharPtr(), pszChar) == 0);
}

bool operator== (const char* pszChar, const Variant& vVariant) {
    return (strcmp (vVariant.GetCharPtr(), pszChar) == 0);
}

bool operator== (const Variant& vVariant, const UTCTime& tTime) {
    return vVariant.GetUTCTime() == tTime;
}

bool operator== (const UTCTime& tTime, const Variant& vVariant) {
    return tTime == vVariant.GetUTCTime();
}

bool operator== (const Variant& vVariant, unsigned int iInt) {
    return iInt == (unsigned int) vVariant.GetInteger();
}

bool operator== (unsigned int iInt, const Variant& vVariant) {
    return iInt == (unsigned int) vVariant.GetInteger();
}

bool operator== (int64 i64Int, const Variant& vVariant) {
    return i64Int == vVariant.GetInteger64();
}

bool operator== (const Variant& vVariant, int64 i64Int) {
    return vVariant.GetInteger64() == i64Int;
}

// !=
bool operator!= (const Variant& vLhs, const Variant& vRhs) {
    return !(vLhs == vRhs);
}

bool operator!= (int iInt, const Variant& vVariant) {
    return iInt != vVariant.GetInteger();
}

bool operator!= (const Variant& vVariant, int iInt) {
    return vVariant.GetInteger() != iInt;
}

bool operator!= (float fFloat, const Variant& vVariant) {
    return fFloat != vVariant.GetFloat();
}

bool operator!= (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() != fFloat;
}

bool operator!= (const Variant& vVariant, const char* pszChar) {
    return (strcmp (vVariant.GetCharPtr(), pszChar) != 0);
}

bool operator!= (const char* pszChar, const Variant& vVariant) {
    return (strcmp (vVariant.GetCharPtr(), pszChar) != 0);
}

bool operator!= (const Variant& vVariant, const UTCTime& tTime) {
    return vVariant.GetUTCTime() != tTime;
}

bool operator!= (const UTCTime& tTime, const Variant& vVariant) {
    return tTime != vVariant.GetUTCTime();
}

bool operator!= (const Variant& vVariant, unsigned int iInt) {
    return iInt != (unsigned int) vVariant.GetInteger();
}

bool operator!= (unsigned int iInt, const Variant& vVariant) {
    return iInt != (unsigned int) vVariant.GetInteger();
}

bool operator!= (int64 i64Int, const Variant& vVariant) {
    return i64Int != vVariant.GetInteger64();
}

bool operator!= (const Variant& vVariant, int64 i64Int) {
    return vVariant.GetInteger64() != i64Int;
}


// >
bool operator> (int iInt, const Variant& vVariant) {
    return iInt > vVariant.GetInteger();
}

bool operator> (const Variant& vVariant, int iInt) {
    return vVariant.GetInteger() > iInt;
}

bool operator> (unsigned int iInt, const Variant& vVariant) {
    return iInt > (unsigned int) vVariant.GetInteger();
}

bool operator> (const Variant& vVariant, unsigned int iInt) {
    return (unsigned int) vVariant.GetInteger() > iInt;
}

bool operator> (float fFloat, const Variant& vVariant) {
    return fFloat > vVariant.GetFloat();
}

bool operator> (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() > fFloat;
}

bool operator> (const Variant& vVariant, const UTCTime& tTime) {
    return vVariant.GetUTCTime() > tTime;
}

bool operator> (const UTCTime& tTime, const Variant& vVariant) {
    return tTime > vVariant.GetUTCTime();
}

bool operator> (int64 i64Int, const Variant& vVariant) {
    return i64Int > vVariant.GetInteger64();
}

bool operator> (const Variant& vVariant, int64 i64Int) {
    return vVariant.GetInteger64() > i64Int;
}


// >=
bool operator>= (int iInt, const Variant& vVariant) {
    return iInt >= vVariant.GetInteger();
}

bool operator>= (const Variant& vVariant, int iInt) {
    return vVariant.GetInteger() >= iInt;
}

bool operator>= (unsigned int iInt, const Variant& vVariant) {
    return iInt >= (unsigned int) vVariant.GetInteger();
}

bool operator>= (const Variant& vVariant, unsigned int iInt) {
    return (unsigned int) vVariant.GetInteger() >= iInt;
}

bool operator>= (float fFloat, const Variant& vVariant) {
    return fFloat >= vVariant.GetFloat();
}

bool operator>= (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() >= fFloat;
}

bool operator>= (const Variant& vVariant, const UTCTime& tTime) {
    return vVariant.GetUTCTime() >= tTime;
}

bool operator>= (const UTCTime& tTime, const Variant& vVariant) {
    return tTime >= vVariant.GetUTCTime();
}

bool operator>= (int64 i64Int, const Variant& vVariant) {
    return i64Int >= vVariant.GetInteger64();
}

bool operator>= (const Variant& vVariant, int64 i64Int) {
    return vVariant.GetInteger64() >= i64Int;
}


// <
bool operator< (int iInt, const Variant& vVariant) {
    return iInt < vVariant.GetInteger();
}

bool operator< (const Variant& vVariant, int iInt) {
    return vVariant.GetInteger() < iInt;
}

bool operator< (unsigned int iInt, const Variant& vVariant) {
    return iInt < (unsigned int) vVariant.GetInteger();
}

bool operator< (const Variant& vVariant, unsigned int iInt) {
    return (unsigned int) vVariant.GetInteger() < iInt;
}

bool operator< (float fFloat, const Variant& vVariant) {
    return fFloat < vVariant.GetFloat();
}

bool operator< (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() < fFloat;
}

bool operator< (const Variant& vVariant, const UTCTime& tTime) {
    return vVariant.GetUTCTime() < tTime;
}

bool operator< (const UTCTime& tTime, const Variant& vVariant) {
    return tTime < vVariant.GetUTCTime();
}

bool operator< (int64 i64Int, const Variant& vVariant) {
    return i64Int < vVariant.GetInteger64();
}

bool operator< (const Variant& vVariant, int64 i64Int) {
    return vVariant.GetInteger64() < i64Int;
}

// <=
bool operator<= (int iInt, const Variant& vVariant) {
    return iInt <= vVariant.GetInteger();
}

bool operator<= (const Variant& vVariant, int iInt) {
    return vVariant.GetInteger() <= iInt;
}

bool operator<= (float fFloat, const Variant& vVariant) {
    return fFloat <= vVariant.GetFloat();
}

bool operator<= (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() <= fFloat;
}

bool operator<= (const Variant& vVariant, const UTCTime& tTime) {
    return vVariant.GetUTCTime() <= tTime;
}

bool operator<= (const UTCTime& tTime, const Variant& vVariant) {
    return tTime <= vVariant.GetUTCTime();
}

bool operator<= (unsigned int iInt, const Variant& vVariant) {
    return iInt <= (unsigned int) vVariant.GetInteger();
}

bool operator<= (const Variant& vVariant, unsigned int iInt) {
    return (unsigned int) vVariant.GetInteger() <= iInt;
}

bool operator<= (int64 i64Int, const Variant& vVariant) {
    return i64Int <= vVariant.GetInteger64();
}

bool operator<= (const Variant& vVariant, int64 i64Int) {
    return vVariant.GetInteger64() <= i64Int;
}

bool operator> (const Variant& vLhs, const Variant& vRhs) {

    Assert (vLhs.m_iType == vRhs.m_iType);

    switch (vLhs.m_iType) {

    case V_INT:
        return vLhs.m_vArg.iArg > vRhs.m_vArg.iArg;

    case V_INT64:
        return vLhs.m_vArg.i64Arg > vRhs.m_vArg.i64Arg;

    case V_FLOAT:
        return vLhs.m_vArg.fArg > vRhs.m_vArg.fArg;

    case V_TIME:
        return vLhs.m_vArg.tArg > vRhs.m_vArg.tArg;
    
    case V_STRING:
        return String::StrCmp (vLhs.m_vArg.pszArg, vLhs.m_vArg.pszArg) > 0;
    }
    Assert (false);
    return false;
}

bool operator< (const Variant& vLhs, const Variant& vRhs) {

    Assert (vLhs.m_iType == vRhs.m_iType);

    switch (vLhs.m_iType) {

    case V_INT:
        return vLhs.m_vArg.iArg < vRhs.m_vArg.iArg;

    case V_INT64:
        return vLhs.m_vArg.i64Arg < vRhs.m_vArg.i64Arg;

    case V_FLOAT:
        return vLhs.m_vArg.fArg < vRhs.m_vArg.fArg;
    
    case V_TIME:
        return vLhs.m_vArg.tArg < vRhs.m_vArg.tArg;

    case V_STRING:
        return String::StrCmp (vLhs.m_vArg.pszArg, vLhs.m_vArg.pszArg) < 0;
    }
    
    Assert (false);
    return false;
}

bool operator>= (const Variant& vLhs, const Variant& vRhs) {

    Assert (vLhs.m_iType == vRhs.m_iType);

    switch (vLhs.m_iType) {

    case V_INT:
        return vLhs.m_vArg.iArg >= vRhs.m_vArg.iArg;

    case V_INT64:
        return vLhs.m_vArg.i64Arg >= vRhs.m_vArg.i64Arg;

    case V_FLOAT:
        return vLhs.m_vArg.fArg >= vRhs.m_vArg.fArg;

    case V_TIME:
        return vLhs.m_vArg.tArg >= vRhs.m_vArg.tArg;

    case V_STRING:
        return String::StrCmp (vLhs.m_vArg.pszArg, vLhs.m_vArg.pszArg) >= 0;
    }
    
    Assert (false);
    return false;
}

bool operator<= (const Variant& vLhs, const Variant& vRhs) {

    Assert (vLhs.m_iType == vRhs.m_iType);

    switch (vLhs.m_iType) {

    case V_INT:
        return vLhs.m_vArg.iArg <= vRhs.m_vArg.iArg;

    case V_INT64:
        return vLhs.m_vArg.i64Arg <= vRhs.m_vArg.i64Arg;

    case V_FLOAT:
        return vLhs.m_vArg.fArg <= vRhs.m_vArg.fArg;

    case V_TIME:
        return vLhs.m_vArg.tArg <= vRhs.m_vArg.tArg;

    case V_STRING:
        return String::StrCmp (vLhs.m_vArg.pszArg, vLhs.m_vArg.pszArg) <= 0;
    }
    
    Assert (false);
    return false;
}


// -
int operator- (int iInt, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return iInt - vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return iInt - (int) vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

int operator- (const Variant& vVariant, int iInt) {

    if (vVariant.GetType() == V_INT) {
        return vVariant.GetInteger() - iInt;
    }
    else if (vVariant.GetType() == V_INT64) {
        return (int) vVariant.GetInteger64() - iInt;
    }
    
    Assert (false);
    return 0;
}

float operator- (float fFloat, const Variant& vVariant) {
    return fFloat - vVariant.GetFloat();
}

float operator- (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() - fFloat;
}

int64 operator- (int64 i64Int, const Variant& vVariant) {

    if (vVariant.GetType() == V_INT) {
        return i64Int - vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return i64Int - vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

int64 operator- (const Variant& vVariant, int64 i64Int) {

    if (vVariant.GetType() == V_INT) {
        return vVariant.GetInteger() - i64Int;
    }
    else if (vVariant.GetType() == V_INT64) {
        return vVariant.GetInteger64() - i64Int;
    }
    
    Assert (false);
    return 0;
}

// +
int operator+ (int iInt, const Variant& vVariant) {

    if (vVariant.GetType() == V_INT) {
        return iInt + vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return iInt + (int) vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

int operator+ (const Variant& vVariant, int iInt) {

    return iInt + vVariant;
}

float operator+ (float fFloat, const Variant& vVariant) {
    return fFloat + vVariant.GetFloat();
}

float operator+ (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() + fFloat;
}

int64 operator+ (int64 i64Int, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return i64Int + vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return i64Int + vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

int64 operator+ (const Variant& vVariant, int64 i64Int) {

    return i64Int + vVariant;
}

// +=
int operator+= (int& iInt, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return iInt += vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return iInt += (int) vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

float operator+= (float& fFloat, const Variant& vVariant) {
    return (fFloat += vVariant.GetFloat());
}

int64 operator+= (int64& i64Int, const Variant& vVariant) {

    if (vVariant.GetType() == V_INT) {
        return i64Int += vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return i64Int += vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

// -=
int operator-= (int& iInt, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return iInt -= vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return iInt -= (int) vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

float operator-= (float& fFloat, const Variant& vVariant) {
    return (fFloat -= vVariant.GetFloat());
}

int64 operator-= (int64& i64Int, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return i64Int -= vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return i64Int -= vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

// *
unsigned int operator* (unsigned int iInt, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return iInt * vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return iInt * (unsigned int) vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

unsigned int operator* (const Variant& vVariant, unsigned int iInt) {
    return (unsigned int) vVariant.GetInteger() * iInt;
}

int operator* (int iInt, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return iInt * vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return iInt * (int) vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

int operator* (const Variant& vVariant, int iInt) {
    return iInt * vVariant;
}

float operator* (float fFloat, const Variant& vVariant) {
    return fFloat * vVariant.GetFloat();
}

float operator* (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() * fFloat;
}

int64 operator* (int64 i64Int, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return i64Int * vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return i64Int * vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

int64 operator* (const Variant& vVariant, int64 i64Int) {
    return i64Int * vVariant;
}

// /
int operator/ (int iInt, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return iInt / vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return iInt / (int) vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

int operator/ (const Variant& vVariant, int iInt) {
    
    if (vVariant.GetType() == V_INT) {
        return vVariant.GetInteger() / iInt;
    }
    else if (vVariant.GetType() == V_INT64) {
        return (int) vVariant.GetInteger64() / iInt;
    }
    
    Assert (false);
    return 0;
}

float operator/ (float fFloat, const Variant& vVariant) {
    return fFloat / vVariant.GetFloat();
}

float operator/ (const Variant& vVariant, float fFloat) {
    return vVariant.GetFloat() / fFloat;
}

int64 operator/ (int64 i64Int, const Variant& vVariant) {
    
    if (vVariant.GetType() == V_INT) {
        return i64Int / vVariant.GetInteger();
    }
    else if (vVariant.GetType() == V_INT64) {
        return i64Int / vVariant.GetInteger64();
    }
    
    Assert (false);
    return 0;
}

int64 operator/ (const Variant& vVariant, int64 i64Int) {
    
    if (vVariant.GetType() == V_INT) {
        return vVariant.GetInteger() / i64Int;
    }
    else if (vVariant.GetType() == V_INT64) {
        return vVariant.GetInteger64() / i64Int;
    }
    
    Assert (false);
    return 0;
}

bool Variant::IsValidType (VariantType vtType) {

    return vtType >= V_STRING && vtType <= V_INT64;
}