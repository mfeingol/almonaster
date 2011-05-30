// FileHeap.h: interface for the MemoryMappedFile class.
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

#if !defined(AFX_FILEHEAP_H__59650832_AE6F_4950_94FC_16BE4139D207__INCLUDED_)
#define AFX_FILEHEAP_H__59650832_AE6F_4950_94FC_16BE4139D207__INCLUDED_

#include "MemoryMappedFile.h"
#include "HashTable.h"
#include "ReadWriteLock.h"

//
// Public definitions
//

typedef size_t Offset;
typedef size_t Size;

#ifdef __WIN64__
typedef int64 Count;
#else
typedef int Count;
#endif

#define NO_OFFSET ((Offset) 0xffffffff)
#define NO_SIZE ((Size) 0xffffffff)

//
// Private definitions
//

#define NUM_BUCKETS (24)
extern const Size BUCKET_SIZE [NUM_BUCKETS];

const unsigned int BLOCK_SIGNATURE = 0x87654321;
const unsigned char BLOCK_FILLER = (unsigned char) 0xbf;

#define MIN_BLOCK_BIT_SIZE 0x4  /* 16 bytes */
#define MAX_BLOCK_SIZE (BUCKET_SIZE[NUM_BUCKETS - 1])

typedef unsigned int Bucket;

struct BlockHeader {
    Offset oPrevBlock;
    Offset oPadding;
    Size sSize;
    Size sUserSize;
    Offset oNextInChain;
    Offset oPrevInChain;
    bool bFree;
    unsigned int iSignature;
};

struct FileHeapHeader {
    Count cNumAllocatedBlocks;
    Count cNumFreeBlocks;
    Count cNumUsedBytes;
    Count cNumSlackBytes;
    Offset oLastBlock;
    Offset oPadding;
    Offset oChainEntry[NUM_BUCKETS];
};

struct FileHeapStatistics {
    size_t stSize;
    size_t stNumAllocatedBlocks;
    size_t stNumFreeBlocks;
    size_t stNumUsedBytes;
    size_t stNumSlackBytes;
};

//
// Main class
//

class OSAL_EXPORT FileHeap {
protected:

    //
    // Data
    //
    ReadWriteLock m_rwGlobalLock;

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

    Bucket GetBucket (Size sSize);

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

    FileHeap();
    ~FileHeap();

    //
    // Locks
    //
    void Lock();
    void Unlock();

    void Freeze();
    void Unfreeze();

    //
    // Single threaded
    //

    // Initialization
    int Initialize();

    int OpenNew (const char* pszFileName, Size sSize, unsigned int iFlags);
    int OpenExisting (const char* pszFileName, unsigned int iFlags);

    // General operations
    int Flush();
    bool Check();

    //
    // Can't hold the lock
    //

    // Heap operations
    Offset Allocate (Size sSize);
    Offset Reallocate (Offset oBlock, Size sSize);
    void Free (Offset oBlock);

    //
    // Must hold the lock
    //

    // Memory usage
    const void* GetBaseAddress();
    Size GetSize();

    void* GetAddress (Offset oOffset);
    Offset GetOffset (const void* pAddress);

    void MemCopy (Offset oSrc, Offset oDest, Size sSize);
    void MemMove (Offset oSrc, Offset oDest, Size sSize);
    void MemSet (Offset oSrc, char cByte, Size sSize);

    // Metadata
    void GetStatistics (FileHeapStatistics* pfhsStats);

    Count GetNumAllocatedBlocks();

    Offset GetFirstBlock();
    Offset GetNextBlock (Offset oBlock);

    Size GetBlockSize (Offset oBlock);
    bool IsBlockFree (Offset oBlock);
};

#endif // !defined(AFX_MEMORYMANAGER_H__59650832_AE6F_4950_94FC_16BE4139D207__INCLUDED_)