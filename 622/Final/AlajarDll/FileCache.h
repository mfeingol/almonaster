// FileCache.h: interface for the FileCache class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar.dll
// Copyright (C) 1998-1999 Max Attar Feingold (maf6@cornell.edu)
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

#if !defined(AFX_FILECACHE_H__C5064725_A72E_11D1_9C50_0060083E8062__INCLUDED_)
#define AFX_FILECACHE_H__C5064725_A72E_11D1_9C50_0060083E8062__INCLUDED_

#include "CachedFile.h"

#include "Osal/ReadWriteLock.h"
#include "Osal/HashTable.h"

class FileCache : public IFileCache {

private:

    class FileCacheHashValue {
    public:
        static unsigned int GetHashValue (const char* pszKey, unsigned int iNumBuckets, const void* pHashHint);
    };

    class FileCacheEquals {
    public:
        static bool Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint);
    };

    bool m_bActive;
    size_t m_stNumBytes;

    ReadWriteLock m_rwFileTableLock;
    HashTable<const char*, CachedFile*, FileCacheHashValue, FileCacheEquals> m_htFileTable;

    CachedFile* RetrieveFile (const char* pszFileName);
    int AddFile (CachedFile* pcfFile);

    FileCache (bool bActive);
    ~FileCache();

    int Initialize (size_t stApproxNumFiles);

public:

    static FileCache* CreateInstance (size_t stApproxNumFiles, bool bActive);
    
    // IFileCache
    IMPLEMENT_INTERFACE (IFileCache);

    ICachedFile* GetFile (const char* pszFileName);

    int ReleaseFile (const char* pszFileName);
    int ReleaseAllFiles();

    bool IsActive();
    unsigned int GetNumFiles();
    size_t GetSize();
};

#endif // !defined(AFX_FILECACHE_H__C5064725_A72E_11D1_9C50_0060083E8062__INCLUDED_)