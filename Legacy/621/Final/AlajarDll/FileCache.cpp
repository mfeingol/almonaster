// FileCache.cpp: implementation of the FileCache class.
//
//////////////////////////////////////////////////////////////////////
//
// Alajar 1.0:  a web server
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
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

#include "FileCache.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileCache::FileCache (bool bActive) : m_htFileTable (NULL, NULL) {

    m_iNumRefs = 1;
    m_bActive = bActive;
    m_stNumBytes = 0;
}

FileCache::~FileCache() {

    if (m_bActive) {

        // Remove files from table
        HashTableIterator<const char*, CachedFile*> htiFile;

        while (m_htFileTable.GetNextIterator (&htiFile)) {
            htiFile.GetData()->Release();
        }
    }
}

int FileCache::Initialize (size_t stApproxNumFiles) {

    int iErrCode;

    if (m_bActive) {

        iErrCode = m_rwFileTableLock.Initialize();
        if (iErrCode != OK) {
            return iErrCode;
        }
            
        if (!m_htFileTable.Initialize ((unsigned int) stApproxNumFiles)) {
            return ERROR_OUT_OF_MEMORY;
        }
    }

    return OK;
}

FileCache* FileCache::CreateInstance (size_t stApproxNumFiles, bool bActive) {

    FileCache* pFileCache = new FileCache (bActive);
    if (pFileCache == NULL) {
        return NULL;
    }

    if (pFileCache->Initialize (stApproxNumFiles) != OK) {
        delete pFileCache;
        return NULL;
    }

    return pFileCache;
}

CachedFile* FileCache::RetrieveFile (const char* pszFileName) {

    CachedFile* pcfFile;

    m_rwFileTableLock.WaitReader();
    bool bFound = m_htFileTable.FindFirst (pszFileName, &pcfFile);
    m_rwFileTableLock.SignalReader();

    return bFound ? pcfFile : NULL;
}


int FileCache::AddFile (CachedFile* pfFile) {

    bool bOldFile = false, bNewFile = false;
    CachedFile* pfOldFile;

    m_rwFileTableLock.WaitWriter();

    bOldFile = m_htFileTable.FindFirst (pfFile->GetName(), &pfOldFile);
    if (!bOldFile) {
        bNewFile = m_htFileTable.Insert (pfFile->GetName(), pfFile);
    }

    m_rwFileTableLock.SignalWriter();

    // Update statistics
    if (bNewFile) {
        // TODO - use AtomicIncrement64 when available
        Algorithm::AtomicIncrement (&m_stNumBytes, (int) pfFile->GetSize());
        return OK;
    }

    if (!bOldFile) {
        return ERROR_OUT_OF_MEMORY;
    }

    return WARNING;
}

ICachedFile* FileCache::GetFile (const char* pszFileName) {

    Assert (pszFileName != NULL);

    CachedFile* pfFile;

    if (m_bActive) {

        char pszResolvedPath [OS::MaxFileNameLength];
        int iErrCode = File::ResolvePath (pszFileName, pszResolvedPath);
        if (iErrCode != OK) {
            return NULL;
        }
        
        pfFile = RetrieveFile (pszResolvedPath);
        if (pfFile != NULL) {
            pfFile->AddRef();
            return pfFile;
        }
            
        // Try to open file and add it to list
        pfFile = CachedFile::CreateInstance();
        if (pfFile == NULL) {
            return NULL;
        }

        iErrCode = pfFile->Open (pszResolvedPath);
        if (iErrCode != OK) {
            pfFile->Release();
            return NULL;
        }
        
        // Add the file to the cache
        iErrCode = AddFile (pfFile);
        if (iErrCode != OK) {

            pfFile->Release();
            
            if (iErrCode == WARNING) {
                // File was already inserted by someone else
                return GetFile (pszFileName);
            }

            return NULL;
        }
        
        pfFile->AddRef();
        return pfFile;
    }
        
    // Create a file dynamically and return it
    pfFile = CachedFile::CreateInstance();
    if (pfFile == NULL) {
        return NULL;
    }
    
    if (pfFile->Open (pszFileName) != OK) {
        pfFile->Release();
        return NULL;
    }
    
    return pfFile;  // No AddRef -> we want Release() to delete the file
}

int FileCache::ReleaseFile (const char* pszFileName) {

    if (!m_bActive) {
        return OK;
    }

    CachedFile* pcfFile;

    char pszResolvedPath [OS::MaxFileNameLength];
    int iErrCode = File::ResolvePath (pszFileName, pszResolvedPath);
    if (iErrCode != OK) {
        return iErrCode;
    }

    m_rwFileTableLock.WaitWriter();
    bool bDeleted = m_htFileTable.DeleteFirst (pszResolvedPath, NULL, &pcfFile);
    m_rwFileTableLock.SignalWriter();

    if (!bDeleted) {
        return ERROR_FAILURE;
    }

    // Update statistics
    // TODO - use AtomicDecrement64 when available
    Algorithm::AtomicDecrement (&m_stNumBytes, (int) pcfFile->GetSize());

    pcfFile->Release();

    return OK;
}

int FileCache::ReleaseAllFiles() {

    if (!m_bActive) {
        return OK;
    }

    CachedFile* pcfFile;
    HashTableIterator<const char*, CachedFile*> htiFile;

    m_rwFileTableLock.WaitWriter();

    if (m_htFileTable.GetNextIterator (&htiFile)) {

        while (m_htFileTable.Delete (&htiFile, NULL, &pcfFile)) {
            pcfFile->Release();
        }
    }

    m_stNumBytes = 0;

    Assert (m_htFileTable.GetNumElements() == 0);

    m_rwFileTableLock.SignalWriter();

    return OK;
}

bool FileCache::IsActive() {
    return m_bActive;
}

size_t FileCache::GetSize() {
    return m_stNumBytes;
}

unsigned int FileCache::GetNumFiles() {

    if (!m_bActive) {
        return 0;
    }
    
    return m_htFileTable.GetNumElements();
}

unsigned int FileCache::FileCacheHashValue::GetHashValue (const char* pszKey, unsigned int iNumBuckets, 
                                                          const void* pHashHint) {
    
    char pszResolvedPath [OS::MaxFileNameLength];

    int iErrCode = File::ResolvePath (pszKey, pszResolvedPath);
    if (iErrCode != OK) {
        strncpy (pszResolvedPath, pszKey, OS::MaxFileNameLength);
        pszResolvedPath [OS::MaxFileNameLength - 1] = '\0';
    }

    return Algorithm::GetStringHashValue (pszResolvedPath, iNumBuckets, true);
}

bool FileCache::FileCacheEquals::Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint) {
    
    char pszResolvedPathLeft [OS::MaxFileNameLength];
    int iErrCode = File::ResolvePath (pszLeft, pszResolvedPathLeft);

    if (iErrCode != OK) {
        return false;
    }

    char pszResolvedPathRight [OS::MaxFileNameLength];
    iErrCode = File::ResolvePath (pszRight, pszResolvedPathRight);

    if (iErrCode != OK) {
        Assert (false);
        return false;
    }

    return _stricmp (pszResolvedPathLeft, pszResolvedPathRight) == 0;
}