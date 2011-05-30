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

void Algorithm::InitializeRandom() {
	srand (time(NULL));
}

int Algorithm::GetRandomInteger (int iUpper) {
	if (iUpper == 0) {
		return 0;
	}
	return (int) rand() % iUpper;
}

char* Algorithm::memstr (const char* pszBuffer, const char* pszMatchString, int iNumChars) {

	size_t stMatchStringLen = strlen (pszMatchString);

	const char* pszEndMarker = pszBuffer + iNumChars;
	char* pszTemp = (char*) memchr (pszBuffer, *pszMatchString, iNumChars);

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

int Algorithm::WWWEscapeCharToAscii (const char* pszInput, char* pszAscii, size_t stLength) {

	if (pszInput == NULL || pszAscii == NULL) {
		return ERROR_INVALID_ARGUMENT;
	}

	size_t i, iCurPos = 0, iLength = strlen (pszInput);
	char szCharacter;
	
	if (stLength < iLength + 1) {
		return ERROR_INVALID_ARGUMENT;
	}

	for (i = 0; i < iLength; i ++) {
		
		if (pszInput[i] == '%' && i + 2 < iLength) {
			szCharacter = (char) (HexToDec (pszInput[i + 1]) * 16 + HexToDec (pszInput [i + 2]));
			
			switch (szCharacter) {
			case '%':
				pszAscii[iCurPos] = '%';
				break;
				
			case '\\':
				pszAscii[iCurPos] = '\\';
				break;
				
			default:
				pszAscii[iCurPos] = szCharacter;
				break;
			}
			
			i += 2;

		} else {
			
			if (pszInput[i] == '+') {
				pszAscii[iCurPos] = ' ';
			} else {
				pszAscii[iCurPos] = pszInput[i];
			}
		}
		
		iCurPos ++;
	}
	
	// Null cap
	if (pszAscii[iCurPos - 2] == '\r' && pszAscii[iCurPos - 1] == '\n') {
		pszAscii[iCurPos - 2] = '\0';
	} else {
		pszAscii[iCurPos] = '\0';
	}

	return OK;
}

int Algorithm::DecodeBase64 (const char* pszBase64, char* pszAnsi) {

	if (pszBase64 == NULL || pszAnsi == NULL) {
		return ERROR_INVALID_ARGUMENT;
	}

	int i, j, iVal, iLength = strlen (pszBase64), iNumChars = 0;
	int piResult [4];
	int piFinal [3];
	
	int i8bitMask = 0x000000FF;
	int i56Mask = 0x00000030;
	
	for (i = 0; i < iLength; i += 4) {
		
		// Convert from base64 alphabet to 4 32 bit values
		for (j = 0; j < 4; j ++) {
			
			iVal = (int) pszBase64[i + j];
			
			// Uppercase
			if (iVal >= 'A' && iVal <= 'Z') {
				piResult[j] = iVal - 65;
			}
			
			// Lowercase
			else if (iVal >= 'a' && iVal <= 'z') {
				piResult[j] = iVal - 71;
			}
			
			// Digit
			else if (iVal >= '0' && iVal <= '9') {
				piResult[j] = iVal + 4;
			}
			
			// +
			else if (iVal == '+') {
				piResult[j] = 62;
			}
			
			// /
			else if (iVal == '/') {
				piResult[j] = 63;
			}
			
			else if (iVal == '=') {
				piResult[j] = 64;
			}
		}
		
		piFinal[0] = (piResult[0] << 2) + ((piResult[1] & i56Mask) >> 4);
		
		if (pszBase64[i + 2] == '=') {
			piFinal[1] = -1;
			piFinal[2] = -1;
		} else {
			piFinal[1] = ((piResult[1] << 4) & i8bitMask) + ((piResult[2] >> 2) & i8bitMask);
			
			if (pszBase64[i + 3] == '=') {
				piFinal[2] = -1;
			} else {
				piFinal[2] = ((piResult[2] << 6) & i8bitMask) + piResult[3];
			}
		}
		
		pszAnsi [iNumChars] = (char) piFinal[0];

		if (piFinal[1] == -1) {
			pszAnsi [iNumChars + 1] = '\0';
		} else {
			pszAnsi [iNumChars + 1] = (char) piFinal[1];
			
			if (piFinal[2] == -1) {
				pszAnsi [iNumChars + 2] = '\0';
			} else {
				pszAnsi [iNumChars + 2] = (char) piFinal[2];
				pszAnsi [iNumChars + 3] = '\0';
			}
		}
		
		iNumChars += 3;
	}

	return OK;
}

int Algorithm::HexToDec (char szDigit) {
	
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

int Algorithm::AtomicIncrement (int* piValue) {
	return ::InterlockedExchangeAdd  ((LPLONG) piValue, 1);
}

int Algorithm::AtomicDecrement (int* piValue) {
	return ::InterlockedExchangeAdd ((LPLONG) piValue, -1);
}

unsigned int Algorithm::AtomicIncrement (unsigned int* piValue) {
	return ::InterlockedExchangeAdd  ((LPLONG) piValue, 1);
}

unsigned int Algorithm::AtomicDecrement (unsigned int* piValue) {
	return ::InterlockedExchangeAdd ((LPLONG) piValue, -1);
}

int Algorithm::AtomicIncrement (int* piValue, int iValue) {
	return ::InterlockedExchangeAdd  ((LPLONG) piValue, iValue);
}

int Algorithm::AtomicDecrement (int* piValue, int iValue) {
	return ::InterlockedExchangeAdd  ((LPLONG) piValue, - iValue);
}

unsigned int Algorithm::AtomicIncrement (unsigned int* piValue, int iValue) {
	return ::InterlockedExchangeAdd  ((LPLONG) piValue, iValue);
}

unsigned int Algorithm::AtomicDecrement (unsigned int* piValue, int iValue) {
	return ::InterlockedExchangeAdd  ((LPLONG) piValue, - iValue);
}

unsigned int Algorithm::GetStringHashValue (const char* pszString, unsigned int iNumBuckets, 
											bool bCaseInsensitive) {
	return GetStringHashValue (pszString, String::StrLen (pszString), iNumBuckets, bCaseInsensitive);
}


#define GOLDEN_RATIO 0.6180339887

unsigned int Algorithm::GetStringHashValue (const char* pszString, size_t stStringLen, 
											unsigned int iNumBuckets, bool bCaseInsensitive) {

	size_t i;
	double dTemp;

	if (stStringLen == 0) {
		return 0;
	}
	
	unsigned int iHash = 1;
	
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

	return iHash;
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

unsigned int Algorithm::GetVariantHashValue (const Variant& vValue, unsigned int iNumBuckets) {

	switch (vValue.GetType()) {
		
	case V_INT:
		return Algorithm::GetIntHashValue (vValue.GetInteger(), iNumBuckets);

	case V_INT64:
		return Algorithm::GetInt64HashValue (vValue.GetInteger64(), iNumBuckets);
		
	case V_FLOAT:
		return Algorithm::GetFloatHashValue (vValue.GetFloat(), iNumBuckets);

	case V_STRING:
		return Algorithm::GetStringHashValue (vValue.GetCharPtr(), iNumBuckets);
		
	case V_TIME:
		return Time::GetHashValue (vValue.GetUTCTime(), iNumBuckets);

	default:
		Assert (false);
		return 0;
	}
}