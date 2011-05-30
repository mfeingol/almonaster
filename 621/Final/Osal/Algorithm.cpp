// Algorithm.cpp: implementation of the Algorithm namespace.
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

#include <stdlib.h>
#include <math.h>

#define OSAL_BUILD
#include "Algorithm.h"
#include "Time.h"
#include "String.h"
#undef OSAL_BUILD

#ifdef __LINUX__
#include "Mutex.h"
#endif

//
// Global Linux state
//

#ifdef __LINUX__
static Mutex AtomicOpsLock;

class AutoMutexInit
{
public:
    AutoMutexInit()
    {
        AtomicOpsLock.Initialize();
    }
};
static AutoMutexInit autoInit;
#endif

//
// Global Win32 state
//

//
// Private methods
//

int HexCharToDecimal (char szDigit) {
    
    if (szDigit <= '9' && szDigit >= '0') {
        return szDigit - '0';
    }

    else if (szDigit <= 'f' && szDigit >= 'a') {
        return szDigit - 'a' + 10;
    }

    else if (szDigit <= 'F' && szDigit >= 'A') {
        return szDigit - 'A' + 10;
    }
    
    return -1;
}

char HexDataToHexChar (char szData) {

    Assert (szData < 16);

    char szHex = '0' + szData;
    if (szHex > '9')
        szHex = 'a' + szHex - '9' - 1;

    return szHex;
}

//
// Algorithm functionality
//

void Algorithm::InitializeThreadRandom (int iRandFactor) {
    srand ((unsigned int) (time (NULL) * iRandFactor));
}

int Algorithm::GetRandomInteger (int iUpper) {
    return iUpper ? rand() % iUpper : 0;
}

char Algorithm::GetRandomASCIIChar() {

	// Relatively arbitrary range
	const char MIN_CHAR = '0';
	const char MAX_CHAR = '[';

	return MIN_CHAR + (char) GetRandomInteger(MAX_CHAR - MIN_CHAR);
}

char* Algorithm::memstr (const char* pszBuffer, const char* pszMatchString, size_t cbBytes) {

    size_t stMatchStringLen = strlen (pszMatchString);

    const char* pszEndMarker = pszBuffer + cbBytes;
    char* pszTemp = (char*) memchr (pszBuffer, *pszMatchString, cbBytes);

    while (pszTemp != NULL && (size_t) (pszEndMarker - pszTemp) >= stMatchStringLen) {

        if (memcmp (pszTemp, pszMatchString, stMatchStringLen) == 0) {
            return pszTemp;
        }

        pszTemp ++;
        if (pszTemp == pszEndMarker) {
            return NULL;
        }
        pszTemp = (char*) memchr (pszTemp, *pszMatchString, pszEndMarker - pszTemp);
    }

    return NULL;
}

int Algorithm::UnescapeString (const char* pszInput, char* pszAnsi, size_t cchLength) {

    if (pszInput == NULL || pszAnsi == NULL || cchLength == 0) {
        return ERROR_INVALID_ARGUMENT;
    }

    char szCharacter;
    size_t i, stCurPos = 0, iLength = strlen (pszInput);

    for (i = 0; i < iLength; i ++) {

        if (stCurPos >= cchLength) {
            return ERROR_SMALL_BUFFER;
        }
        
        if (pszInput[i] == '%') {

            if (i + 2 >= iLength) {
                return ERROR_INVALID_ARGUMENT;
            }

            szCharacter = (char) (
                HexCharToDecimal (pszInput [i + 1]) * 16 +
                HexCharToDecimal (pszInput [i + 2])
                );

            switch (szCharacter) {
            case '%':
                pszAnsi[stCurPos] = '%';
                break;
                
            case '\\':
                pszAnsi[stCurPos] = '\\';
                break;

            default:
                pszAnsi[stCurPos] = szCharacter;
                break;
            }
            
            i += 2;

        } else {
            
            if (pszInput[i] == '+') {
                pszAnsi[stCurPos] = ' ';
            } else {
                pszAnsi[stCurPos] = pszInput[i];
            }
        }
        
        stCurPos ++;
    }
    
    // Null cap, remove trailing CRLF
    if (stCurPos >= 2 && 
        pszAnsi[stCurPos - 2] == '\r' && 
        pszAnsi[stCurPos - 1] == '\n') {
        pszAnsi[stCurPos - 2] = '\0';
    } else {
        pszAnsi[stCurPos] = '\0';
    }

    return OK;
}

char DataToBase64 (char sixBits) {

    Assert (sixBits >= 0 && sixBits < 64);

    if (sixBits < 26) {
        return 'A' + sixBits;
    }

    else if (sixBits < 52) {
        return 'a' + sixBits - 26;
    }

    else if (sixBits < 62) {
        return '0' + sixBits - 52;
    }

    else if (sixBits == 62) {
        return '+';
    }

    else if (sixBits == 63) {
        return '/';
    }

    else if (sixBits == 64) {
        return '=';
    }

    else {
        Assert (false);
        return '!';
    }
}

char Base64ToData (char szBase64) {

    // Uppercase
    if (szBase64 >= 'A' && szBase64 <= 'Z') {
        return szBase64 - 'A';
    }
    
    // Lowercase
    else if (szBase64 >= 'a' && szBase64 <= 'z') {
        return szBase64 - 'a' + 26;
    }
    
    // Digit
    else if (szBase64 >= '0' && szBase64 <= '9') {
        return szBase64 - '0' + 52;
    }
    
    // +
    else if (szBase64 == '+') {
        return 62;
    }
    
    // /
    else if (szBase64 == '/') {
        return 63;
    }

    else if (szBase64 == '=') {
        return 64;
    }

    else {
        return -1;
    }
}

void EncodeBase64 (int one, char pszBase64 [4]) {

    int sixBits [2];
    
    // First six bits of one
    sixBits[0] = (char) (one >> 2) & 0x3f;

    // Last 2 bits of one, last 4 bits are zero
    sixBits[1] = (char) ((one & 0x03) << 4);

    for (int i = 0; i < countof (sixBits); i ++) {
        pszBase64[i] = DataToBase64 ((char) sixBits[i]);
    }
    pszBase64[2] = '=';
    pszBase64[3] = '=';
}

void EncodeBase64 (int one, int two, char pszBase64 [4]) {

    int sixBits [3];
    
    // First six bits of one
    sixBits[0] = (char) (one >> 2) & 0x3f;

    // Last 2 bits of one, first 4 bits of two
    sixBits[1] = (char) ((one & 0x03) << 4) + (char) ((two >> 4) & 0x0f);

    // Last 4 bits of two, first 2 bits are zero
    sixBits[2] = (char) ((two & 0x0f) << 2);

    for (int i = 0; i < countof (sixBits); i ++) {
        pszBase64[i] = DataToBase64 ((char) sixBits[i]);
    }
    pszBase64[3] = '=';
}

int DecodeBase64 (const char sixBits[4], char pbData[3]);
void EncodeBase64 (int one, int two, int three, char pszBase64 [4]);

void EncodeBase64 (int one, int two, int three, char pszBase64 [4]) {

    char sixBits [4];
    
    // First six bits of one
    sixBits[0] = (char) (one >> 2) & 0x3f;

    // Last 2 bits of one, first 4 bits of two
    sixBits[1] = (char) ((one & 0x03) << 4) + (char) ((two >> 4) & 0x0f);

    // Last 4 bits of two, first 2 bits of three
    sixBits[2] = (char) ((two & 0x0f) << 2) + (char) ((three >> 6) & 0x03);

    // Last 6 bits of three
    sixBits[3] = (char) three & 0x3f;

    for (int i = 0; i < countof (sixBits); i ++) {
        pszBase64[i] = DataToBase64 ((char) sixBits[i]);
    }

#ifdef _DEBUG
    char pbData [3];
    int iDecoded = ::DecodeBase64 (sixBits, pbData);
    Assert (iDecoded == 3);
    Assert (pbData[0] == (char) one);
    Assert (pbData[1] == (char) two);
    Assert (pbData[2] == (char) three);
#endif

}

int DecodeBase64 (const char sixBits[4], char pbData[3]) {
    
    // All six bits of one, first two bits of two
    pbData[0] = (sixBits[0] << 2) + ((sixBits[1] >> 4) & 0x03);

    // Only one conditional in mainstream path
    if (sixBits[3] == 64) {

        if (sixBits[2] == 64) {

            return 1;

        } else {

            // Last four bits of two, first four bits of three
            pbData[1] = (sixBits[1] << 4) + ((sixBits[2] >> 2) & 0x0f);

            return 2;
        }

    } else {

        // Last four bits of two, first four bits of three
        pbData[1] = (sixBits[1] << 4) + ((sixBits[2] >> 2) & 0x0f);

        // Last two bits of three, all six bits of four
        pbData[2] = ((sixBits[2] << 6) & 0xc0) + sixBits[3];

        return 3;
    }
}

size_t Algorithm::GetEncodeBase64Size (size_t cbDataLength) {

    int iMod = cbDataLength % 3;
    if (iMod == 0) {
        return 4 * cbDataLength / 3 + 1;
    } else {
        return 4 * (cbDataLength + 3 - iMod) / 3 + 1;
    }
}

int Algorithm::EncodeBase64 (const void* pbData, size_t cbDataLength, char* pszBase64, size_t cchLength) {

    if (cchLength < GetEncodeBase64Size (cbDataLength)) {
        return ERROR_SMALL_BUFFER;
    }

    const char* pszData = (const char*) pbData;
    size_t stDataCursor = 0, stBase64Cursor = 0;

    while (stDataCursor + 2 < cbDataLength) {

        ::EncodeBase64 (
            pszData[stDataCursor], 
            pszData[stDataCursor + 1],
            pszData[stDataCursor+ 2],
            pszBase64 + stBase64Cursor
            );

        stDataCursor += 3;
        stBase64Cursor += 4;
    }

    Assert (cbDataLength >= stDataCursor);
    size_t stDataRemaining = cbDataLength - stDataCursor;
    switch (stDataRemaining) {

        case 1:
            ::EncodeBase64 (pszData[stDataCursor], pszBase64 + stBase64Cursor);
            stBase64Cursor += 4;
            break;

        case 2:
            ::EncodeBase64 (pszData[stDataCursor], pszData[stDataCursor + 1], pszBase64 + stBase64Cursor);
            stBase64Cursor += 4;
            break;

        default:
            Assert (stDataRemaining == 0);
            break;
    }

    // Null-terminate the string
    Assert (stBase64Cursor < cchLength);
    pszBase64 [stBase64Cursor] = '\0';

    return OK;
}

// This can return a size that's slightly too big, depending on how many garbage characters
// there are in the string.
size_t Algorithm::GetDecodeBase64Size (const char* pszBase64, size_t cchLength) {

    size_t cbNeeded = 3 * cchLength / 4;

    if (pszBase64 [cchLength - 1] != '=') {
        return cbNeeded;
    }
    cbNeeded --;

    if (pszBase64 [cchLength - 2] != '=') {
        return cbNeeded;
    }
    cbNeeded --;

    return cbNeeded;
}

bool GetNextBase64Char (const char* pszBase64, char* pchChar, size_t* pstBase64Cursor) {

    while (true) {

        char chBase64 = pszBase64 [*pstBase64Cursor];
        if (chBase64 == '\0') {
            return false;
        }

        char chData = Base64ToData (chBase64);
        (*pstBase64Cursor) ++;

        if (chData != -1) {
            *pchChar = chData;
            return true;
        }
    }
}

int Algorithm::DecodeBase64 (const char* pszBase64, void* pbData, size_t cbLength, size_t* pcbDecoded) {

    size_t cchBase64Length = strlen (pszBase64);
    size_t cbNeeded = GetDecodeBase64Size (pszBase64, cchBase64Length);

    if (cbLength < cbNeeded) {
        *pcbDecoded = cbNeeded;
        return ERROR_SMALL_BUFFER;
    }

    char* pszData = (char*) pbData;

    size_t stBase64Cursor = 0, stDataCursor = 0;
    while (stBase64Cursor < cchBase64Length) {

        char sixBits[4];
        for (int i = 0; i < countof (sixBits); i ++) {

            if (!GetNextBase64Char (pszBase64, sixBits + i, &stBase64Cursor)) {
                Assert (false);
                return ERROR_INVALID_ARGUMENT;
            }
        }

        int iDecoded = ::DecodeBase64 (sixBits, pszData + stDataCursor);
        stDataCursor += iDecoded;
    }

    *pcbDecoded = stDataCursor;

    return OK;
}

int Algorithm::HexEncode (const void* pbData, size_t cbDataLength, char* pszHex, size_t cchLength) {

    if (cbDataLength * 2 >= cchLength) {
        return ERROR_SMALL_BUFFER;
    }

    size_t cchHexCursor = 0;
    const char* pszData = (const char*) pbData;
    for (size_t i = 0; i < cbDataLength; i ++) {

        char szData = pszData[i];

        char szLeft = (szData >> 4) & 0xf;
        pszHex[cchHexCursor ++] = HexDataToHexChar (szLeft);

        char szRight = szData & 0xf;
        pszHex[cchHexCursor ++] = HexDataToHexChar (szRight);
    }

    // Null-cap the string
    pszHex[cchHexCursor] = '\0';

    return OK;
}

int Algorithm::AtomicIncrement (int* piValue) {
#ifdef __LINUX__
    return AtomicIncrement(piValue, 1);
#else if defined __WIN32__
    return ::InterlockedIncrement ((LPLONG) piValue);
#endif
}

int Algorithm::AtomicDecrement (int* piValue) {
#ifdef __LINUX__
    return AtomicIncrement(piValue, -1);
#else if defined __WIN32__
    return ::InterlockedDecrement ((LPLONG) piValue);
#endif
}

unsigned int Algorithm::AtomicIncrement (unsigned int* piValue) {
#ifdef __LINUX__
    return AtomicIncrement(piValue, 1);
#else if defined __WIN32__
    return ::InterlockedIncrement  ((LPLONG) piValue);
#endif
}

unsigned int Algorithm::AtomicDecrement (unsigned int* piValue) {
#ifdef __LINUX__
    return AtomicIncrement(piValue, -1);
#else if defined __WIN32__
    return ::InterlockedDecrement ((LPLONG) piValue);
#endif
}

int Algorithm::AtomicIncrement (int* piValue, int iValue) {
#ifdef __LINUX__
    AtomicOpsLock.Wait();
    int retval = *piValue;
    *piValue = *piValue + iValue;
    AtomicOpsLock.Signal();
    return retval + iValue;
#else if defined __WIN32__
    return ::InterlockedExchangeAdd  ((LPLONG) piValue, iValue) + iValue;
#endif
}

int Algorithm::AtomicDecrement (int* piValue, int iValue) {
#ifdef __LINUX__
    return AtomicIncrement(piValue, -iValue);
#else if defined __WIN32__
    return ::InterlockedExchangeAdd  ((LPLONG) piValue, - iValue) - iValue;
#endif
}

unsigned int Algorithm::AtomicIncrement (unsigned int* piValue, int iValue) {
#ifdef __LINUX__
    AtomicOpsLock.Wait();
    unsigned int retval = *piValue;
    *piValue = *piValue + iValue;
    AtomicOpsLock.Signal();
    return retval + iValue;
#else if defined __WIN32__
    return ::InterlockedExchangeAdd  ((LPLONG) piValue, iValue) + iValue;
#endif
}

unsigned int Algorithm::AtomicDecrement (unsigned int* piValue, int iValue) {
#ifdef __LINUX__
    return AtomicIncrement(piValue, -iValue);
#else if defined __WIN32__
    return ::InterlockedExchangeAdd  ((LPLONG) piValue, - iValue) - iValue;
#endif
}

unsigned int Algorithm::GetStringHashValue (const char* pszString, unsigned int iNumBuckets, bool bCaseInsensitive) {
    return GetStringHashValue (pszString, String::StrLen (pszString), iNumBuckets, bCaseInsensitive);
}


// string hash: { while (*psz)  h=h*101+*psz++; }

#define GOLDEN_RATIO 0.6180339887

unsigned int Algorithm::GetStringHashValue (const char* pszString, size_t stStringLen, unsigned int iNumBuckets, 
                                            bool bCaseInsensitive) {

    size_t i;
    double dTemp;

    if (stStringLen == 0) {
        stStringLen = String::StrLen (pszString);
    }
    
    size_t iHash = 1;
    
    if (bCaseInsensitive) {
        
        for (i = 0; i < stStringLen; i ++) {
            
            dTemp = GOLDEN_RATIO * iHash * (int) tolower (pszString[i]);

            if (dTemp < 0) {
                dTemp = - dTemp;
            }

            dTemp -= floor (dTemp);
            iHash = (int) (dTemp * (double) iNumBuckets);
        }
        
    } else {
        
        for (i = 0; i < stStringLen; i ++) {
            
            dTemp = GOLDEN_RATIO * iHash * (int) pszString[i];

            if (dTemp < 0) {
                dTemp = - dTemp;
            }

            dTemp -= floor (dTemp);
            iHash = (int) (dTemp * (double) iNumBuckets);
        }
    }

    return (unsigned int) iHash;
}

unsigned int Algorithm::GetIntHashValue (int iValue, unsigned int iNumBuckets) {
    
    double dTemp = GOLDEN_RATIO * iValue;

    if (dTemp < 0) {
        dTemp = - dTemp;
    }

    dTemp -= floor (dTemp);
    
    return (unsigned int) (dTemp * (double) iNumBuckets);
}

unsigned int Algorithm::GetInt64HashValue (int64 i64Value, unsigned int iNumBuckets) {
    
    double dTemp = GOLDEN_RATIO * i64Value;

    if (dTemp < 0) {
        dTemp = - dTemp;
    }

    dTemp -= floor (dTemp);
    
    return (unsigned int) (dTemp * (double) iNumBuckets);
}

unsigned int Algorithm::GetFloatHashValue (float fValue, unsigned int iNumBuckets) {
    
    double dTemp = GOLDEN_RATIO * fValue;

    if (dTemp < 0) {
        dTemp = - dTemp;
    }

    dTemp -= floor (dTemp);
    
    return (unsigned int) (dTemp * (double) iNumBuckets);
}

unsigned int Algorithm::GetVariantHashValue (const Variant& vValue, unsigned int iNumBuckets, bool bCaseInsensitive) {

    switch (vValue.GetType()) {
        
    case V_INT:
        return Algorithm::GetIntHashValue (vValue.GetInteger(), iNumBuckets);

    case V_INT64:
        return Algorithm::GetInt64HashValue (vValue.GetInteger64(), iNumBuckets);
        
    case V_FLOAT:
        return Algorithm::GetFloatHashValue (vValue.GetFloat(), iNumBuckets);

    case V_STRING:
        return Algorithm::GetStringHashValue (vValue.GetCharPtr(), iNumBuckets, bCaseInsensitive);
        
    case V_TIME:
        return Time::GetHashValue (vValue.GetUTCTime(), iNumBuckets);

    default:
        Assert (false);
        return 0;
    }
}

/* Base64 test code

    char* ppszString[26];

    for (char i = 0; i < 26; i ++) {

        ppszString[i] = new char [i + 2];
        for (char j = 0; j < i + 1; j ++) {
            ppszString[i][j] = 'a' + j;
        }
        ppszString[i][i + 1] = '\0';
    }

    for (int i = 0; i < 26; i ++) {

        int iErrCode;
        char pszBase64 [512], pszBase64_test [512], pszDecoded [512];

        iErrCode = Algorithm::EncodeBase64 (
            ppszString[i],
            strlen (ppszString[i]) + 1,
            pszBase64,
            countof (pszBase64)
            );

        Assert (iErrCode == OK);

        iErrCode = Algorithm::EncodeBase64 (
            ppszString[i],
            strlen (ppszString[i]) + 1,
            pszBase64_test,
            countof (pszBase64_test)
            );

        Assert (iErrCode == OK);
        Assert (strcmp (pszBase64, pszBase64_test) == 0);

        size_t cchDecoded;
        iErrCode = Algorithm::DecodeBase64 (pszBase64, pszDecoded, countof (pszDecoded), &cchDecoded);
        Assert (iErrCode == OK);

        Assert (strcmp (pszDecoded, ppszString[i]) == 0);
        Assert (strlen (pszDecoded) + 1 == cchDecoded);
    }
*/