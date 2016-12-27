// File.h: interface for the File class.
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

#if !defined(AFX_FILE_H__C5064726_A72E_11D1_9C50_0060083E8062__INCLUDED_)
#define AFX_FILE_H__C5064726_A72E_11D1_9C50_0060083E8062__INCLUDED_

#include "Time.h"

enum FileType {
    FILETYPE_ERROR,
    FILETYPE_DIRECTORY,
    FILETYPE_FILE,
    FILETYPE_DEVICE,
    FILETYPE_PIPE
};

class File;
class OSAL_EXPORT FileEnumerator {

    friend class File;

private:

    unsigned int m_iNumFiles;
    char** m_ppszFileName;

    void Clean();

public:

    FileEnumerator();
    ~FileEnumerator();

    unsigned int GetNumFiles();
    const char** GetFileNames();
};

class OSAL_EXPORT File {

protected:

#ifdef __LINUX__
	int m_hFile;
#else if defined __WIN32__
	HANDLE m_hFile;
#endif

public:

    File();
    virtual ~File();

    virtual int OpenRead (const char* pszFileName);
    virtual int OpenWrite (const char* pszFileName);
    virtual int OpenAppend (const char* pszFileName);

    virtual bool IsOpen();
    virtual int Close();
    virtual int Reset();

    virtual int GetFilePointer (size_t* pstLocation);
    virtual int SetFilePointer (size_t stLocation);

    virtual int GetSize (size_t* pstSize);

    virtual int Read (void* pBuffer, size_t stNumBytes, size_t* pstNumBytesRead) const;
    virtual int Write (const void* pBuffer, size_t stNumBytes);
    virtual int Write (const char* pBuffer);
    virtual int WriteEndLine();

    // Static file operations
    static int EnumerateFiles (const char* pszSpec, FileEnumerator* pEnum);

    static int GetFileMimeType (const char* pszFileName, char pszMimeType [OS::MaxMimeTypeLength]);
    static FileType GetFileType (const char* pszFileName);
    static int GetFileSize (const char* pszFileName, size_t* pstSize);

    static int GetLastModifiedTime (const char* pszFileName, UTCTime* tTime);

    static bool DoesFileExist (const char* pszFileName);
    static bool DoesDirectoryExist (const char* pszDirName);

    static bool WasFileModifiedAfter (const char* pszFileName, const char* pszGMTDate, UTCTime* ptLastModified);

    static int DeleteFile (const char* pszFileName);
    static int RenameFile (const char* pszOldName, const char* pszNewName);
    static int CopyFile (const char* pszOldName, const char* pszNewName);
    static int MoveFile (const char* pszOldName, const char* pszNewName);
    static int CopyAllFiles (const char* pszSrcName, const char* pszDestName);

    static int CreateDirectory (const char* pszDirName);
    static int DeleteDirectory (const char* pszDirName);
    static int RenameDirectory (const char* pszOldName, const char* pszNewName);
    static int MoveDirectory (const char* pszOldName, const char* pszNewName);

    static int ResolvePath (const char* pszPath, char pszResolvedPath [OS::MaxFileNameLength]);
};

#endif // !defined(AFX_FILE_H__C5064726_A72E_11D1_9C50_0060083E8062__INCLUDED_)