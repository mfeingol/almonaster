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

#ifdef __LINUX__
#include <fcntl.h>
#include <dirent.h>
#define CloseHandle close
#define _stat stat
#endif

#define MAX_MIME_TYPE_LENGTH 512

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

#ifdef __LINUX__
	m_hFile = open(pszFileName, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
#else if defined __WIN32__
    m_hFile = ::CreateFile (
        pszFileName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL, 
        OPEN_EXISTING, 
        0, 
        NULL
        );
#endif

    return (m_hFile == INVALID_HANDLE_VALUE) ? ERROR_FAILURE : OK;
}


int File::OpenWrite (const char* pszFileName) {

    if (m_hFile != INVALID_HANDLE_VALUE) {
        ::CloseHandle (m_hFile);
    }

#ifdef __LINUX__
	m_hFile = open(pszFileName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | O_TRUNC);
	return (m_hFile == INVALID_HANDLE_VALUE) ? ERROR_FAILURE : OK;
#else if defined __WIN32__

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
#endif
}

int File::OpenAppend (const char* pszFileName) {

    if (m_hFile != INVALID_HANDLE_VALUE) {
        ::CloseHandle (m_hFile);
    }

#ifdef __LINUX__
	m_hFile = open(pszFileName, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
	return (m_hFile == INVALID_HANDLE_VALUE) ? ERROR_FAILURE : OK;
#else if defined __WIN32__

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
#endif
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

#ifdef __LINUX__
    SetFilePointer(0);
    return OK;
#else if defined __WIN32__

    LARGE_INTEGER li;
    li.QuadPart = 0;
    return ::SetFilePointerEx (m_hFile, li, NULL, FILE_BEGIN) ? OK : ERROR_FAILURE;
#endif
}

int File::GetFilePointer (size_t* pstLocation) {

#ifdef __LINUX__
    off_t offset = lseek(m_hFile, 0, SEEK_CUR);
    if (offset == (off_t)-1)
		return ERROR_FAILURE;

    *pstLocation = (size_t) offset;
    return OK;

#else if defined __WIN32__

    LARGE_INTEGER li, li2;
    li.QuadPart = 0;
    int iErrCode = ::SetFilePointerEx (m_hFile, li, &li2, FILE_CURRENT) ? OK : ERROR_FAILURE;
    if (iErrCode == OK) {
        *pstLocation = (size_t) li2.QuadPart;
    }
    return iErrCode;
#endif
}

int File::SetFilePointer (size_t stLocation) {

#ifdef __LINUX__
    return (lseek(m_hFile, stLocation, SEEK_SET) != stLocation) ? ERROR_FAILURE : OK;
#else if defined __WIN32__
    LARGE_INTEGER li;
    li.QuadPart = stLocation;
    return ::SetFilePointerEx (m_hFile, li, NULL, FILE_BEGIN) ? OK : ERROR_FAILURE;
#endif
}

int File::GetSize (size_t* pstSize) {

    if (m_hFile == INVALID_HANDLE_VALUE) {
        return ERROR_FAILURE;
    }

#ifdef __LINUX__
    struct stat st;
    if (fstat(m_hFile, &st) < 0)
    {
		*pstSize = 0;
		return ERROR_FAILURE;
	}

    *pstSize = st.st_size;
    return OK;

#else if defined __WIN32__
    LARGE_INTEGER li;
    int iErrCode = ::GetFileSizeEx (m_hFile, &li) ? OK : ERROR_FAILURE;
    if (iErrCode == OK) {
        *pstSize = (size_t) li.QuadPart;
    }
    return iErrCode;
#endif
}

int File::Read (void* pBuffer, size_t stNumBytes, size_t* pstNumBytesRead) const {

    if (m_hFile == INVALID_HANDLE_VALUE) {
        return ERROR_FAILURE;
    }

#ifdef __LINUX__
    *pstNumBytesRead = read(m_hFile, pBuffer, stNumBytes);

    if (*pstNumBytesRead < 0)
    {
        *pstNumBytesRead = 0;
        return ERROR_FAILURE;
    }
    else
        return OK;

#else if defined __WIN32__

    DWORD dwNumBytesRead;
    if (::ReadFile (m_hFile, pBuffer, (DWORD) stNumBytes, &dwNumBytesRead, NULL)) {
        *pstNumBytesRead = dwNumBytesRead;
        return OK;
    }

    *pstNumBytesRead = 0;
    return ERROR_FAILURE;
#endif
}

int File::Write (const void* pBuffer, size_t stNumBytes) {
    
    if (m_hFile == INVALID_HANDLE_VALUE) {
        return ERROR_FAILURE;
    }

#ifdef __LINUX__
    return (write(m_hFile, pBuffer, stNumBytes) < 0) ? ERROR_FAILURE : OK;
#else if defined __WIN32__

    Assert (stNumBytes == (size_t) (int) stNumBytes);
    DWORD dwNumBytesWritten;
    return (::WriteFile (m_hFile, pBuffer, (DWORD) stNumBytes, &dwNumBytesWritten, NULL)) ? OK : ERROR_FAILURE;
#endif
}

int File::Write (const char* pBuffer) {
    
    return Write (pBuffer, strlen (pBuffer));
}

int File::WriteEndLine() {
    
    if (m_hFile == INVALID_HANDLE_VALUE) {
        return ERROR_FAILURE;
    }

#ifdef __LINUX__
    return (write(m_hFile, "\r\n", 2) < 0) ? ERROR_FAILURE : OK;
#else if defined __WIN32__
    DWORD dwNumBytesWritten;
    return (::WriteFile (m_hFile, "\r\n", 2, &dwNumBytesWritten, NULL)) ? OK : ERROR_FAILURE;
#endif
}

//////////////////////
// Static functions //
//////////////////////

int File::EnumerateFiles (const char* pszSpec, FileEnumerator* pEnum) {

    int iErrCode = OK;

    unsigned int iSpace, iNewSpace;
    char** ppszTemp;

#ifdef __LINUX__
    // we assume very specific form for pszSpec which is fine for this application
    // pszSpec should be a directory part followed by a file part. The file part
    // can contain at most one '*' at the beginning and one '*' at the end.
    // The characters in the middle will be matched literally

    // extract directory part
    char *ptr = strrchr(pszSpec, '/');
    if (!ptr)
        return ERROR_FAILURE;

    bool starBegin = false, starEnd = false;
    char *dirName = new char[ptr - pszSpec + 1];
    strncpy(dirName, pszSpec, ptr - pszSpec);
    dirName[ptr-pszSpec] = '\0';

    DIR *dir = opendir(dirName);
    if (!dir)
    {
        delete [] dirName;
        return ERROR_FAILURE;
    }

    ptr++;                      // description of files
    if (*ptr == '*')
    {
        starBegin = true;
        ptr++;
    }

    char *fileSpec = new char[strlen(ptr) + 1];
    strncpy(fileSpec, ptr, strlen(ptr)+1);
    if (strlen(ptr) > 0 && fileSpec[strlen(ptr)-1] == '*')
    {
        starEnd = true;
        fileSpec[strlen(ptr)-1] = '\0';
    }

#else if defined __WIN32__

    WIN32_FIND_DATA fdFileData;
    HANDLE hHandle = ::FindFirstFile (pszSpec, &fdFileData);

    if (hHandle == INVALID_HANDLE_VALUE) {
        return ERROR_FAILURE;
    }

    if (strcmp (fdFileData.cFileName, ".") != 0 &&
        strcmp (fdFileData.cFileName, "..") != 0) {
        pEnum->m_iNumFiles = 1;
    }
#endif

    pEnum->Clean();

    iSpace = 10;
    pEnum->m_ppszFileName = new char* [iSpace];
    if (pEnum->m_ppszFileName == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

#ifdef __LINUX__
    struct dirent *dirent;

    while ((dirent = readdir(dir)) != NULL)
    {
		if (strcmp (dirent->d_name, ".") == 0 ||
			strcmp (dirent->d_name, "..") == 0)
        {
            continue;
        }

        char *match = strstr(dirent->d_name, fileSpec);

        // if length of fileSpec == 0, then we match anything
        if (strlen(fileSpec) != 0 && !match)
            continue;

        if ((strlen(fileSpec) == 0) ||
            (starBegin && starEnd) ||
            (starBegin && !starEnd && (dirent->d_name + strlen(dirent->d_name) - match == strlen(fileSpec))) ||
            (!starBegin && starEnd && (match == dirent->d_name)) ||
            (!starBegin && !starEnd && (match == dirent->d_name) && (strlen(fileSpec) == strlen(dirent->d_name))))
        {
                // matched
                // drop through to the code below

#else if defined __WIN32__

    do {
        if (strcmp (fdFileData.cFileName, ".") != 0 &&
            strcmp (fdFileData.cFileName, "..") != 0) {
#endif

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

#ifdef __LINUX__
			pEnum->m_ppszFileName[pEnum->m_iNumFiles] = String::StrDup (dirent->d_name);
#else if defined __WIN32__
			pEnum->m_ppszFileName[pEnum->m_iNumFiles] = String::StrDup (fdFileData.cFileName);
#endif
            if (pEnum->m_ppszFileName[pEnum->m_iNumFiles] == NULL) {
                iErrCode = ERROR_OUT_OF_MEMORY;
                goto Cleanup;
            }

            pEnum->m_iNumFiles ++;
        }       

#ifdef __LINUX__
    }

    closedir(dir);
    delete [] fileSpec;
    delete [] dirName;

#else if defined __WIN32__
    } while (::FindNextFile (hHandle, &fdFileData));
#endif

Cleanup:

#ifdef __WIN32__
    ::FindClose (hHandle);
#endif

    return iErrCode;
}

int File::GetFileMimeType (const char* pszFileName, char pszMimeType [OS::MaxMimeTypeLength]) {

    int iErrCode = OK;

    // Get file's extension
    char* pszExtension = strrchr(pszFileName, '.');
    if (pszExtension != NULL)
        pszExtension ++;
    else
        pszExtension = "";

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
    
#ifdef __WIN32__
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
#endif

Cleanup:

    strncpy (pszMimeType, pszFinalMimeType, OS::MaxMimeTypeLength);
    pszMimeType [OS::MaxMimeTypeLength - 1] = '\0';
    
    return iErrCode;
}

FileType File::GetFileType (const char* pszFileName) {

#ifdef __LINUX__

    struct stat st;
    if (stat(pszFileName, &st) < 0)
		return FILETYPE_ERROR;
    return S_ISDIR(st.st_mode) ? FILETYPE_DIRECTORY : FILETYPE_FILE;

#else if defined __WIN32__

    // Check directory first
    DWORD dwRet;

    dwRet = ::GetFileAttributes (pszFileName);
    if (dwRet == INVALID_FILE_ATTRIBUTES) {
        return FILETYPE_ERROR;
    }

    if (dwRet & FILE_ATTRIBUTE_DIRECTORY) {
        return FILETYPE_DIRECTORY;
    }

    // It's not a directory, so it's some kind of file
    File f;
    int iErrCode = f.OpenRead (pszFileName);
    if (iErrCode != OK) {
        return FILETYPE_ERROR;
    }

    dwRet = ::GetFileType (f.m_hFile);
    f.Close();

    switch (dwRet) {

    case FILE_TYPE_UNKNOWN:
        return FILETYPE_ERROR;

    case FILE_TYPE_DISK:
        return FILETYPE_FILE;

    case FILE_TYPE_CHAR:
        return FILETYPE_DEVICE;

    case FILE_TYPE_PIPE:
        return FILETYPE_PIPE;

    default:
        return FILETYPE_ERROR;
    }
#endif
}

int File::GetFileSize (const char* pszFileName, size_t* pstSize) {

#ifdef __LINUX__
    struct stat st;
    if (stat(pszFileName, &st) < 0)
		return ERROR_FAILURE;

    *pstSize = st.st_size;
    return OK;

#else if defined __WIN32__

    WIN32_FILE_ATTRIBUTE_DATA win32Data;

    if (::GetFileAttributesEx(
        pszFileName,            // file or directory name
        GetFileExInfoStandard,  // attribute 
        &win32Data              // attribute information 
        )) {

        // 64 bit doesn't work
        *pstSize = win32Data.nFileSizeLow;
        return OK;
    }

    return ERROR_FAILURE;
#endif
}

int File::DeleteFile (const char* pszFileName) {
#ifdef __LINUX__
	return (unlink(pszFileName) == 0) ? OK : ERROR_FAILURE;
#else if defined __WIN32__
	return ::DeleteFile (pszFileName) ? OK : ERROR_FAILURE;
#endif
}

int File::RenameFile (const char* pszOldName, const char* pszNewName) {
#ifdef __LINUX__
	return (rename(pszOldName, pszNewName) == 0) ? OK : ERROR_FAILURE;
#else if defined __WIN32__
	return ::MoveFile (pszOldName, pszNewName) ? OK : ERROR_FAILURE;
#endif
}

int File::CopyFile (const char* pszOldName, const char* pszNewName) {
#ifdef __LINUX__
    char buf[2048];
    int fd_in, fd_out;
    int error = 0;
    int size;

    fd_in = open(pszOldName, O_RDONLY);
    fd_out = open(pszNewName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR, S_IRUSR | S_IWUSR);

    while ((size = read(fd_in, buf, sizeof(buf))) > 0)
    {
        if (write(fd_out, buf, sizeof(buf)) < 0)
        {
            error = -1;
            break;
        }
    }

    close(fd_in);
    close(fd_out);

    if ((size < 0) || (error < 0))
        return ERROR_FAILURE;
    else
        return OK;
#else if defined __WIN32__
	return ::CopyFile (pszOldName, pszNewName, FALSE) ? OK : ERROR_FAILURE;
#endif
}

int File::MoveFile (const char* pszOldName, const char* pszNewName) {
#ifdef __LINUX__
    // assume old and new files are on same filesystem
    // we link the new name and remove the old name
    if (link(pszOldName, pszNewName) < 0)
        return ERROR_FAILURE;

    DeleteFile(pszOldName);

#else
	return ::MoveFile (pszOldName, pszNewName) ? OK : ERROR_FAILURE;
#endif
}


int File::CopyAllFiles (const char* pszSrcName, const char* pszDestName) {

#ifdef __LINUX__
    // Not implemented. (doesn't appear to be used)
    Assert(false);
    return ERROR_FAILURE;

#else if defined __WIN32__

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
#endif
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

#ifdef __LINUX__
    FileType ft = GetFileType(pszFileName);
    return ft == FILETYPE_FILE;
#else if defined __WIN32__

    DWORD dwAttr = ::GetFileAttributes (pszFileName);
    if (dwAttr == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0;
#endif
}

bool File::DoesDirectoryExist (const char* pszDirName) {

#ifdef __LINUX__
    FileType ft = GetFileType(pszDirName);
    return ft == FILETYPE_DIRECTORY;
#else if defined __WIN32__

    DWORD dwAttr = ::GetFileAttributes (pszDirName);
    if (dwAttr == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#endif
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
#ifdef __LINUX__
	return mkdir(pszDirName, 0700) == 0 ? OK : ERROR_FAILURE;
#else if defined __WIN32__
	return ::CreateDirectory (pszDirName, NULL) ? OK : ERROR_FAILURE;
#endif
}

int File::DeleteDirectory (const char* pszDirName) {
    
#ifdef __LINUX__

    char pszFullName [OS::MaxFileNameLength];
    DIR *dir = opendir(pszDirName);
    struct dirent *dirent;

    if (!dir)
        return ERROR_FAILURE;

    while ((dirent = readdir(dir)) != NULL)
    {
        if (strcmp (dirent->d_name, ".") == 0 || strcmp (dirent->d_name, "..") == 0)
            continue;

        snprintf (pszFullName, OS::MaxFileNameLength, "%s/%s", pszDirName, dirent->d_name);
        if (GetFileType(pszFullName) == FILETYPE_DIRECTORY)
        {
            if (DeleteDirectory (pszFullName) != OK)
            {
                closedir(dir);
                return ERROR_FAILURE;
            }
        }
        else
        {
            if (DeleteFile (pszFullName) != OK)
            {
                closedir(dir);
                return ERROR_FAILURE;
            }
        }
    }

    closedir(dir);

    return (rmdir(pszDirName) == 0) ? OK : ERROR_FAILURE;

#else if defined __WIN32__

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

                if (GetFileType (pszFullName) == FILETYPE_DIRECTORY) {
                    
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
#endif
}

int File::RenameDirectory (const char* pszOldName, const char* pszNewName) {
#ifdef __LINUX__
    return RenameFile(pszOldName, pszNewName);
#else if defined __WIN32__
	return ::MoveFile (pszOldName, pszNewName) ? OK : ERROR_FAILURE;
#endif
}

int File::MoveDirectory (const char* pszOldName, const char* pszNewName) {
#ifdef __LINUX__
    return RenameFile(pszOldName, pszNewName);
#else if defined __WIN32__
	return ::MoveFile (pszOldName, pszNewName) ? OK : ERROR_FAILURE;
#endif
}

int File::ResolvePath (const char* pszPath, char pszResolvedPath [OS::MaxFileNameLength]) {

#ifdef __LINUX__

    	size_t i, stLength = strlen (pszPath);

	// Check for path too long
	if (stLength >= OS::MaxFileNameLength) {
		return ERROR_FAILURE;
	}

    if (*pszPath == '/')
    {
        // absolute path
		strncpy (pszResolvedPath, pszPath, stLength + 1);
    }
    else
    {
        // relative
		int iErrCode = OS::GetApplicationDirectory (pszResolvedPath);
		if (iErrCode != OK)
			return iErrCode;

		if (stLength + strlen (pszResolvedPath) + 1 >= OS::MaxFileNameLength)
			return ERROR_FAILURE;

		strcat (pszResolvedPath, "/");
		strcat (pszResolvedPath, pszPath);
    }

    return OK;

#else if defined __WIN32__

    char* pszFileName;
    DWORD dwRet = GetFullPathName (pszPath, OS::MaxFileNameLength, pszResolvedPath, &pszFileName);

    if (dwRet == 0 || dwRet > OS::MaxFileNameLength) {
        return ERROR_FAILURE;
    }

    return OK;
#endif
}

//
// FileEnumerator
//

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