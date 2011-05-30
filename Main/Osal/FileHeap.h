// FileHeap.h: interface for the MemoryMappedFile class.
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

#if !defined(AFX_FILEHEAP_H__59650832_AE6F_4950_94FC_16BE4139D207__INCLUDED_)
#define AFX_FILEHEAP_H__59650832_AE6F_4950_94FC_16BE4139D207__INCLUDED_

#include "MemoryMappedFile.h"
#include "HashTable.h"
#include "ReadWriteLock.h"

//
// Public definitions
//

typedef int64 Count;
typedef unsigned __int64 Offset;
typedef unsigned __int64 Size;
#define NO_OFFSET (0xffffffffffffffff)
#define NO_SIZE   (0xffffffffffffffff)

//
// Private definitions
//

static const Size BUCKET_SIZE[] = {
    0x0000000000000010,
    0x0000000000000020,
    0x0000000000000040,
    0x0000000000000080,
    0x0000000000000100,
    0x0000000000000200,
    0x0000000000000400,
    0x0000000000000800,
    0x0000000000001000,
    0x0000000000002000,
    0x0000000000004000,
    0x0000000000008000,
    0x0000000000010000,
    0x0000000000020000,
    0x0000000000040000,
    0x0000000000080000,
    0x0000000000100000,
    0x0000000000200000,
    0x0000000000400000,
    0x0000000000800000,
    0x0000000001000000,
    0x0000000002000000,
    0x0000000004000000,
    0x0000000008000000,
    0x0000000100000000,
    0x0000000200000000,
    0x0000000400000000,
    0x0000000800000000,
    0x0000001000000000,
    0x0000002000000000,
    0x0000004000000000,
    0x0000008000000000,
    0x0000010000000000,
    0x0000020000000000,
    0x0000040000000000,
    0x0000080000000000,
    0x0000100000000000,
    0x0000200000000000,
    0x0000400000000000,
    0x0000800000000000,
    0x0001000000000000,
    0x0002000000000000,
    0x0004000000000000,
    0x0008000000000000,
    0x0010000000000000,
    0x0020000000000000,
    0x0040000000000000,
    0x0080000000000000,
    0x0100000000000000,
    0x0200000000000000,
    0x0400000000000000,
    0x0800000000000000,
};

const unsigned int NUM_BUCKETS = sizeof(BUCKET_SIZE) / sizeof(BUCKET_SIZE[0]);

const unsigned int BLOCK_SIGNATURE = 0x87654321;
const unsigned char BLOCK_FILLER = (unsigned char) 0xbf;

#define MIN_BLOCK_BIT_SIZE 0x4  /* 16 bytes */
#define MAX_BLOCK_SIZE (BUCKET_SIZE[NUM_BUCKETS - 1])

typedef unsigned int Bucket;

struct BlockHeader {
    Offset oPrevBlock;
    Offset oPadding;
    Size cbSize;
    Size cbUserSize;
    Offset oNextInChain;
    Offset oPrevInChain;
    bool bFree;
    unsigned int iSignature;
};

struct FileHeapHeader {
    Count cNumAllocatedBlocks;
    Count cNumFreeBlocks;
    Size cbNumUsedBytes;
    Size cbNumSlackBytes;
    Offset oLastBlock;
    Offset oPadding;
    Offset oChainEntry[NUM_BUCKETS];
};

struct FileHeapStatistics {
    Size cbSize;
    Count cNumAllocatedBlocks;
    Count cNumFreeBlocks;
    Size cbNumUsedBytes;
    Size cbNumSlackBytes;
};

//
// Main class
//

class OSAL_EXPORT FileHeap {
protected:

    //
    // Data
    //
    ReadWriteLock& m_rwGlobalLock;

    MemoryMappedFile m_mmfData;
    FileHeapHeader* m_pHeader;

    ReadWriteLock m_prwBucketLock [NUM_BUCKETS];

    //
    // Internal heap
    //

    Offset AllocateBlockFromBucket (Bucket iBucket);
    void DivideBlock (Offset oBlock, Bucket iChunkBucket, Bucket iBigBucket);

    void RemoveBlockFromBucket (Offset oBlock, Bucket iBucket);
    void AddBlockToBucket (Offset oBlock, Bucket iBucket);

    Bucket CoalesceBlock (Offset oBlock, Bucket iBucket, Bucket iWantBucket, bool bFree);
    Bucket CoalesceNext (Offset oBlock, Bucket iBucket, bool bFree);

    int Resize (Bucket iBucket);

    //
    // Utility
    //

    Bucket GetBucket (Size cbSize);

    BlockHeader* GetBlockHeader (Offset oBlock);
    BlockHeader* GetBlockHeaderFromUserOffset (Offset oUserBlock);
    Offset GetOffsetFromBlockHeader (const BlockHeader* pBlockHeader);

    BlockHeader* GetPrevBlockHeader (const BlockHeader* pBlockHeader);
    BlockHeader* GetNextBlockHeader (const BlockHeader* pBlockHeader);

    Offset GetUserOffset (Offset oBlock);
    Offset GetOffsetFromUserOffset (Offset oBlock);
    Offset GetBlockOffset (const BlockHeader* pBlockHeader);
    Offset GetNextBlockInternal (Offset oBlock);

    void InitFreeBlock (BlockHeader* pBlock, Offset oPrevBlock, Bucket iBucket);

#ifdef _DEBUG
    bool IsSlackSpaceIntact (Offset oBlock);
    void* GetSlackSpace (Offset oBlock, Size* psSlackSize);
#endif

    bool IsValidBlock (Offset oBlock);
    bool IsValidUserBlock (Offset oBlock);
    bool IsValidBucket (Bucket iBucket);

public:

    FileHeap (ReadWriteLock& rwGlobalLock);
    ~FileHeap();

    //
    // Single threaded
    //

    // Initialization
    int Initialize();

    int OpenNew (const char* pszFileName, Size cbSize, unsigned int iFlags);
    int OpenExisting (const char* pszFileName, unsigned int iFlags);

    // General operations
    int Flush();
    bool Check();

    //
    // Can't hold the lock
    //

    // Heap operations
    Offset Allocate (Size cbSize);
    Offset Reallocate (Offset oBlock, Size cbSize);

    //
    // Must hold the lock
    //

    void Free (Offset oBlock);

    // Memory usage
    const void* GetBaseAddress();
    Size GetSize();

    void* GetAddress (Offset oOffset);
    Offset GetOffset (const void* pAddress);

    void MemCopy (Offset oSrc, Offset oDest, Size cbSize);
    void MemMove (Offset oSrc, Offset oDest, Size cbSize);
    void MemSet (Offset oSrc, char cByte, Size cbSize);

    // Metadata
    void GetStatistics (FileHeapStatistics* pfhsStats);

    Count GetNumAllocatedBlocks();

    Offset GetFirstBlock();
    Offset GetNextBlock (Offset oBlock);

    Size GetBlockSize (Offset oBlock);
    bool IsBlockFree (Offset oBlock);
};

#endif // !defined(AFX_MEMORYMANAGER_H__59650832_AE6F_4950_94FC_16BE4139D207__INCLUDED_)