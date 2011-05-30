// File.cpp: implementation of the File class.
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
#include "File.h"
#include "Time.h"
#include "String.h"
#undef OSAL_BUILD

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_MIME_TYPE_LENGTH 512

// Microsoft didn't define this in VSSP4
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD) -1)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

File::File() {
	m_hFile = INVALID_HANDLE_VALUE;
}

File::~File() {

	if (m_hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle (m_hFile);
	}
}


int File::OpenRead (const char* pszFileName) {

	// Close previous file
	if (m_hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle (m_hFile);
	}

	if (pszFileName == NULL) {
		return ERROR_FAILURE;
	}

	m_hFile = ::CreateFile (
		pszFileName,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL
		);

	return (m_hFile == INVALID_HANDLE_VALUE) ? ERROR_FAILURE : OK;
}


int File::OpenWrite (const char* pszFileName) {

	if (m_hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle (m_hFile);
	}

	m_hFile = ::CreateFile (
		pszFileName, 
		GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_ALWAYS, 
		0, 
		NULL
		);

	if (m_hFile == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}
	
	if (::SetEndOfFile (m_hFile)) {
		return OK;
	}

	::CloseHandle (m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	
	return ERROR_FAILURE;
}

int File::OpenAppend (const char* pszFileName) {

	if (m_hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle (m_hFile);
	}

	m_hFile = ::CreateFile (
		pszFileName, 
		GENERIC_WRITE | GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_ALWAYS, 
		0, 
		NULL
		);

	if (m_hFile == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}

	if (::SetFilePointer (m_hFile, 0, NULL, FILE_END) != INVALID_SET_FILE_POINTER) {
		return OK;
	}

	::CloseHandle (m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	
	return ERROR_FAILURE;
}


int File::Close() {

	if (m_hFile != INVALID_HANDLE_VALUE) {

		::CloseHandle (m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;

		return OK;
	}

	return ERROR_FAILURE;
}


bool File::IsOpen() {
	return m_hFile != INVALID_HANDLE_VALUE;
}


int File::Reset() {

	// 64 bits won't work
	if (::SetFilePointer (m_hFile, 0, NULL, FILE_BEGIN) != 0) {
		return ERROR_FAILURE;
	}

	return OK;
}

int File::GetFilePointer (size_t* pstLocation) {

	// 64 bits won't work
	DWORD dwFilePtr = ::SetFilePointer (m_hFile, 0, NULL, FILE_CURRENT);
	
	if (dwFilePtr == INVALID_SET_FILE_POINTER) {
		return ERROR_FAILURE;
	}

	*pstLocation = (size_t) dwFilePtr;

	return OK;

}

int File::SetFilePointer (size_t stLocation) {

	// 64 bits won't work
	if (::SetFilePointer (m_hFile, stLocation, NULL, FILE_BEGIN) != (DWORD) stLocation) {
		return ERROR_FAILURE;
	}

	return OK;
}

int File::GetSize (size_t* pstSize) {

	if (m_hFile == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}
	
	DWORD dwHiLength, dwLoLength;
	if ((dwLoLength = ::GetFileSize (m_hFile, &dwHiLength)) == 0xffffffff) {
		*pstSize = 0;
		return ERROR_FAILURE;
	}
	
	*pstSize = dwLoLength;
	return OK;
}

int File::Read (void* pBuffer, size_t stNumBytes, size_t* pstNumBytesRead) const {

	if (m_hFile == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}

	DWORD dwNumBytesRead;
	if (::ReadFile (m_hFile, pBuffer, stNumBytes, &dwNumBytesRead, NULL)) {
		*pstNumBytesRead = dwNumBytesRead;
		return OK;
	}
	
	*pstNumBytesRead = 0;
	return ERROR_FAILURE;
}

int File::Write (const void* pBuffer, size_t stNumBytes) {
	
	if (m_hFile == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}

	DWORD dwNumBytesWritten;
	return (::WriteFile (m_hFile, pBuffer, stNumBytes, &dwNumBytesWritten, NULL)) ? OK : ERROR_FAILURE;
}

int File::Write (const char* pBuffer) {
	
	if (m_hFile == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}

	DWORD dwNumBytesWritten;
	return (::WriteFile (m_hFile, pBuffer, strlen (pBuffer), &dwNumBytesWritten, NULL)) ? OK : ERROR_FAILURE;
}

int File::WriteEndLine() {
	
	if (m_hFile == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}

	DWORD dwNumBytesWritten;
	return (::WriteFile (m_hFile, "\r\n", 2, &dwNumBytesWritten, NULL)) ? OK : ERROR_FAILURE;
}

//////////////////////
// Static functions //
//////////////////////

int File::EnumerateFiles (const char* pszSpec, FileEnumerator* pEnum) {

	int iErrCode = OK;

	WIN32_FIND_DATA fdFileData;
	HANDLE hHandle = ::FindFirstFile (pszSpec, &fdFileData);

	unsigned int iSpace, iNewSpace;
	char** ppszTemp;

	if (hHandle == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}

	if (strcmp (fdFileData.cFileName, ".") != 0 &&
		strcmp (fdFileData.cFileName, "..") != 0) {
		pEnum->m_iNumFiles = 1;
	}

	pEnum->Clean();

	iSpace = 10;
	pEnum->m_ppszFileName = new char* [iSpace];
	if (pEnum->m_ppszFileName == NULL) {
		iErrCode = ERROR_OUT_OF_MEMORY;
		goto Cleanup;
	}

	do {

		if (strcmp (fdFileData.cFileName, ".") != 0 &&
			strcmp (fdFileData.cFileName, "..") != 0) {

			if (pEnum->m_iNumFiles == iSpace) {
				
				iNewSpace = iSpace * 2;
				
				ppszTemp = new char* [iNewSpace];
				if (ppszTemp == NULL) {
					iErrCode = ERROR_OUT_OF_MEMORY;
					goto Cleanup;
				}
				memcpy (ppszTemp, pEnum->m_ppszFileName, sizeof (char*) * iSpace);
				
				delete [] pEnum->m_ppszFileName;
				pEnum->m_ppszFileName = ppszTemp;
				
				iSpace = iNewSpace;
			}
			
			pEnum->m_ppszFileName[pEnum->m_iNumFiles] = String::StrDup (fdFileData.cFileName);
			if (pEnum->m_ppszFileName[pEnum->m_iNumFiles] == NULL) {
				iErrCode = ERROR_OUT_OF_MEMORY;
				goto Cleanup;
			}

			pEnum->m_iNumFiles ++;
		}		
		
	} while (::FindNextFile (hHandle, &fdFileData));

Cleanup:

	::FindClose (hHandle);

	return iErrCode;
}

int File::GetFileMimeType (const char* pszFileName, char pszMimeType [OS::MaxMimeTypeLength]) {

	int iErrCode = OK;

	char pszTemp [OS::MaxFileNameLength];
	
	// Get file's extension
	strcpy (pszTemp, pszFileName);

	char* pszExtension = _strrev (pszTemp);
	pszExtension = strtok (pszExtension, ".");
	pszExtension = _strrev (pszExtension);

	const char* pszFinalMimeType = "text/plain";
	
	// Check standard types first
	switch (pszExtension[0]) {
		
	case 'h':
	case 'H':
		
		if (stricmp (pszExtension, "html") == 0 || stricmp (pszExtension, "htm") == 0) {
			pszFinalMimeType = "text/html";
			goto Cleanup;
		}
		
		break;
		
	case 'g':
	case 'G':
		if (stricmp (pszExtension, "gif") == 0) {;
			pszFinalMimeType = "image/gif";
			goto Cleanup;
		}
		
		break;
		
	case 'j':
	case 'J':
		if (stricmp (pszExtension, "jpg") == 0) {
			pszFinalMimeType = "image/jpeg";
			goto Cleanup;
		}
		
		break;
		
	case 't':
	case 'T':
		if (stricmp (pszExtension, "txt") == 0 || stricmp (pszExtension, "text") == 0) {
			pszFinalMimeType = "text/plain";
			goto Cleanup;
		}
		break;
		
	case 'z':
	case 'Z':
		if (stricmp (pszExtension, "zip") == 0) {
			pszFinalMimeType = "application/x-zip-compressed";
			goto Cleanup;
		}
		
		break;
	}
	
	// If not a standard type, look for content type in registry
	{
		size_t stExtLen = strlen (pszExtension) + 1;
		char* pszKeyName = (char*) StackAlloc (stExtLen + 1);
		
		pszKeyName[0] = '.';
		strncpy (pszKeyName + 1, pszExtension, stExtLen);
		
		char pszRegInfo [MAX_MIME_TYPE_LENGTH];
		DWORD dwTypeLen = 0;
		HKEY hKey = NULL;

		LONG lErrCode = ::RegOpenKeyEx (
			HKEY_CLASSES_ROOT,
			pszKeyName,
			0,
			KEY_QUERY_VALUE,
			&hKey
			);

		if (lErrCode == ERROR_SUCCESS) {

			lErrCode = ::RegQueryValueEx (
				hKey, 
				"Content Type", 
				NULL, 
				NULL, 
				(BYTE*) pszRegInfo, 
				&dwTypeLen
				);

			::RegCloseKey (hKey);
		}
		
		// If not found in the registry, default to plaintext
		if (lErrCode != ERROR_SUCCESS) {
			pszFinalMimeType = "text/plain";
		} else {
			pszFinalMimeType = (char*) pszRegInfo;
		}
	}

Cleanup:

	Assert (strlen (pszFinalMimeType) < OS::MaxMimeTypeLength);

	strcpy (pszMimeType, pszFinalMimeType);
	
	return iErrCode;
}

FileType File::GetFileType (const char* pszFileName) {
	
	DWORD dwAttr = ::GetFileAttributes (pszFileName);

	if (dwAttr == 0xffffffff) {
		return FILE_NOT_FOUND;
	}

	if ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
		return FILE_IS_DIRECTORY;
	}
	
	return FILE_IS_FILE;
}

int File::GetFileSize (const char* pszFileName, size_t* pstSize) {

	WIN32_FILE_ATTRIBUTE_DATA win32Data;

	if (::GetFileAttributesEx(
		pszFileName,                   // file or directory name
		GetFileExInfoStandard,  // attribute 
		&win32Data              // attribute information 
		)) {

		// Fails on 64 bit
		*pstSize = win32Data.nFileSizeLow;
		return OK;
	}

	return ERROR_FAILURE;
}

int File::DeleteFile (const char* pszFileName) {
	return ::DeleteFile (pszFileName) ? OK : ERROR_FAILURE;
}

int File::RenameFile (const char* pszOldName, const char* pszNewName) {
	return ::MoveFile (pszOldName, pszNewName) ? OK : ERROR_FAILURE;
}

int File::CopyFile (const char* pszOldName, const char* pszNewName) {
	return ::CopyFile (pszOldName, pszNewName, FALSE) ? OK : ERROR_FAILURE;
}

int File::MoveFile (const char* pszOldName, const char* pszNewName) {
	return ::MoveFile (pszOldName, pszNewName) ? OK : ERROR_FAILURE;
}


int File::CopyAllFiles (const char* pszSrcName, const char* pszDestName) {

	int iErrCode = OK;

	char pszSrc [OS::MaxFileNameLength + 5], pszDest [OS::MaxFileNameLength];

	sprintf (pszSrc, "%s/*.*", pszSrcName);

	WIN32_FIND_DATA fdData;
	HANDLE hHandle = ::FindFirstFile (pszSrc, &fdData);

	if (hHandle == INVALID_HANDLE_VALUE) {
		return ERROR_FAILURE;
	}

	DWORD dwRetVal;

	do {
		
		if (strcmp (fdData.cFileName, ".") != 0 && strcmp (fdData.cFileName, "..") != 0) {
			
			sprintf (pszSrc, "%s/%s", pszSrcName, fdData.cFileName);
			sprintf (pszDest, "%s/%s", pszDestName, fdData.cFileName);

			dwRetVal = ::GetFileAttributes (pszSrc);
			if (dwRetVal == -1) {
				iErrCode = ERROR_FAILURE;
				goto Cleanup;
			}

			// If a file copy it, if a directory call recursively
			if (dwRetVal & FILE_ATTRIBUTE_DIRECTORY) {
				
				// Create the new directory				
				if (!::CreateDirectory (pszDestName, NULL)) {
					iErrCode = ERROR_FAILURE;
					goto Cleanup;
				}

				iErrCode = CopyAllFiles (pszSrc, pszDest);
				if (iErrCode != OK) {
					goto Cleanup;
				}					
				
			} else {

				if (!::CopyFile (pszSrc, pszDest, FALSE)) {
					iErrCode = ERROR_FAILURE;
					goto Cleanup;
				}
			}
		}

	} while (::FindNextFile (hHandle, &fdData));
	
Cleanup:

	::FindClose (hHandle);

	return iErrCode;
}

int File::GetLastModifiedTime (const char* pszFileName, UTCTime* ptTime) {

	struct _stat statBuf;

	if (_stat (pszFileName, &statBuf) != 0) {
		return ERROR_FAILURE;
	}
	
	*ptTime = (UTCTime) statBuf.st_mtime;
	return OK;
}

bool File::DoesFileExist (const char* pszFileName) {
	
	DWORD dwAttr = ::GetFileAttributes (pszFileName);
	
	if (dwAttr == 0xffffffff) {
		return false;
	}
	
	return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY;
}

bool File::DoesDirectoryExist (const char* pszDirName) {

	DWORD dwAttr = ::GetFileAttributes (pszDirName);
	
	if (dwAttr == 0xffffffff) {
		return false;
	}
	
	return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

bool File::WasFileModifiedAfter (const char* pszFileName, const char* pszGMTDate, UTCTime* ptLastModified) {
	
	struct _stat statBuf;
	if (_stat (pszFileName, &statBuf) != 0) {
		return false;
	}

	// Save last modified date
	*ptLastModified = (UTCTime) statBuf.st_mtime;
	tm* ptmFileTime = gmtime (&statBuf.st_mtime);

	// Compare to sent date
	int iYear, iDay, iHour, iMinute, iSecond;
	char pszMonth [20];
	char pszWeekDay [20];
	sscanf (pszGMTDate, "%s %i %s %i %i:%i:%i", pszWeekDay, &iDay, pszMonth, &iYear, &iHour, &iMinute, &iSecond);
	
	// Year
	if (ptmFileTime->tm_year + 1900 != iYear) {
		return (ptmFileTime->tm_year + 1900 > iYear);
	}
	
	// Month
	int iMonth = -1;
	switch (pszMonth[0]) {
		
	case 'J':
	case 'j':
		if (stricmp (pszMonth, "Jan") == 0) {
			iMonth = 0;
		}
		else if (stricmp (pszMonth, "Jun") == 0) {
			iMonth = 5;
		}
		else if (stricmp (pszMonth, "Jul") == 0) {
			iMonth = 6;
		}
		break;
		
	case 'F':
	case 'f':
		if (stricmp (pszMonth, "Feb") == 0) {
			iMonth = 1;
		}
		break;
		
	case 'M':
	case 'm':
		if (stricmp (pszMonth, "Mar") == 0) {
			iMonth = 2;
		}
		else if (stricmp (pszMonth, "May") == 0) {
			iMonth = 4;
		}
		
		break;
		
	case 'A':
	case 'a':
		if (stricmp (pszMonth, "Apr") == 0) {
			iMonth = 3;
		}
		else if (stricmp (pszMonth, "Aug") == 0) {
			iMonth = 7;
		}
		break;
		
	case 'S':
	case 's':
		if (stricmp (pszMonth, "Sep") == 0) {
			iMonth = 8;
		}
		break;
		
	case 'O':
	case 'o':
		if (stricmp (pszMonth, "Oct") == 0) {
			iMonth = 9;
		}
		break;
		
	case 'N':
	case 'n':
		if (stricmp (pszMonth, "Nov") == 0) {
			iMonth = 10;
		}
		break;
		
	case 'D':
	case 'd':
		if (stricmp (pszMonth, "Dec") == 0) {
			iMonth = 11;
		}
		break;
	}

	if (iMonth == -1) {
		return false;
	}
	
	if (ptmFileTime->tm_mon + 1 != iMonth) {
		return (ptmFileTime->tm_mon + 1 > iMonth);
	}
	
	// Day
	if (ptmFileTime->tm_mday != iDay) {
		return (ptmFileTime->tm_mday > iDay);
	}
	
	// Hour
	if (ptmFileTime->tm_hour != iHour) {
		return (ptmFileTime->tm_hour> iHour);
	}
	
	// Minute
	if (ptmFileTime->tm_min != iMinute) {
		return (ptmFileTime->tm_min > iMinute);
	}
	
	// Second 
	if (ptmFileTime->tm_sec != iSecond) {
		return (ptmFileTime->tm_sec > iSecond);
	}
	
	return false;
}

int File::CreateDirectory (const char* pszDirName) {
	return ::CreateDirectory (pszDirName, NULL) ? OK : ERROR_FAILURE;
}

int File::DeleteDirectory (const char* pszDirName) {
	
	char pszFullName [OS::MaxFileNameLength], pszSearchName [OS::MaxFileNameLength];

	sprintf (pszSearchName, "%s/*.*", pszDirName);

	// Delete the directory's contents
	bool bSuccess = true;
	WIN32_FIND_DATA fdData;
	HANDLE hHandle = ::FindFirstFile (pszSearchName, &fdData);

	if (hHandle != INVALID_HANDLE_VALUE) {
		
		do {
			if (strcmp (fdData.cFileName, ".") != 0 && strcmp (fdData.cFileName, "..") != 0) {	
				
				sprintf (pszFullName, "%s/%s", pszDirName, fdData.cFileName);

				if (GetFileType (pszFullName) == FILE_IS_DIRECTORY) {
					
					if (DeleteDirectory (pszFullName) != OK) {
						bSuccess = false;
					}
				
				} else {

					if (!::DeleteFile (pszFullName)) {
						bSuccess = false;
					}
				}
			}

		} while (::FindNextFile (hHandle, &fdData));
		
		::FindClose (hHandle);
	}

	// Delete the empty directory
	if (!::RemoveDirectory (pszDirName)) {
		bSuccess = false;
	}

	return bSuccess ? OK : ERROR_FAILURE;
}

int File::RenameDirectory (const char* pszOldName, const char* pszNewName) {
	return ::MoveFile (pszOldName, pszNewName) ? OK : ERROR_FAILURE;
}

int File::MoveDirectory (const char* pszOldName, const char* pszNewName) {
	return ::MoveFile (pszOldName, pszNewName) ? OK : ERROR_FAILURE;
}

int File::ResolvePath (const char* pszPath, char pszResolvedPath [OS::MaxFileNameLength]) {

	size_t i, stLength = strlen (pszPath);

	// Check for path too long
	if (stLength >= OS::MaxFileNameLength) {
		return ERROR_FAILURE;
	}

	// If the first character is a slash, then prepend the exe's drive
	if (*pszPath == '/' || *pszPath == '\\') {
		
		if (stLength + 2 >= OS::MaxFileNameLength) {
			return ERROR_FAILURE;
		}
		int iErrCode = OS::GetApplicationDirectory (pszResolvedPath);
		if (iErrCode != OK) {
			return iErrCode;
		}
		strcpy (&(pszResolvedPath[2]), pszPath);

		stLength = strlen (pszResolvedPath);
	}

	else if (stLength < 2 || pszPath[1] != ':') {

		// A relative path: get the application directory
		int iErrCode = OS::GetApplicationDirectory (pszResolvedPath);
		if (iErrCode != OK) {
			return iErrCode;
		}

		if (stLength + strlen (pszResolvedPath) + 1 >= OS::MaxFileNameLength) {
			return ERROR_FAILURE;
		}

		strcat (pszResolvedPath, "/");
		strcat (pszResolvedPath, pszPath);

		stLength = strlen (pszResolvedPath);
	}

	else {

		// Absolute path; easy
		strncpy (pszResolvedPath, pszPath, stLength + 1);
	}

	// Whack to slash
	for (i = 0; i < stLength; i ++) {

		if (pszResolvedPath[i] == '\\') {
			pszResolvedPath[i] = '/';
		}
	}

	return OK;
}

FileEnumerator::FileEnumerator() {

	m_iNumFiles = 0;
	m_ppszFileName = NULL;
}

FileEnumerator::~FileEnumerator() {

	Clean();
}

void FileEnumerator::Clean() {

	if (m_ppszFileName != NULL) {

		for (unsigned int i = 0; i < m_iNumFiles; i ++) {
			
			if (m_ppszFileName[i] != NULL) {
				delete [] m_ppszFileName[i];
			}
		}

		delete [] m_ppszFileName;
		m_ppszFileName = NULL;
	}

	m_iNumFiles = 0;
}

unsigned int FileEnumerator::GetNumFiles() {
	return m_iNumFiles;
}

const char** FileEnumerator::GetFileNames() {
	return (const char**) m_ppszFileName;
}