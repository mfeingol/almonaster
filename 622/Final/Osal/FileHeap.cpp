// FileHeap.cpp: implementation of the FileHeap class.
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

#define OSAL_BUILD
#include "FileHeap.h"
#include "Algorithm.h"
#undef OSAL_BUILD

#define SIZE_OF_MANAGER_HEADER (sizeof (FileHeapHeader))
#define SIZE_OF_BLOCK_HEADER (sizeof (BlockHeader))

#define ALIGNMENT_BASE (8)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileHeap::FileHeap (ReadWriteLock& rwGlobalLock) : m_rwGlobalLock (rwGlobalLock) {

    m_pHeader = NULL;

    Assert (SIZE_OF_MANAGER_HEADER % ALIGNMENT_BASE == 0);
    Assert (SIZE_OF_BLOCK_HEADER % ALIGNMENT_BASE == 0);
}

FileHeap::~FileHeap() {

    if (m_mmfData.IsOpen()) {
        m_mmfData.Flush();
        m_mmfData.Close();
    }
}


//
// Public methods
//

int FileHeap::Initialize() {

    int iErrCode, i;

    for (i = 0; i < sizeof (m_prwBucketLock) / sizeof (m_prwBucketLock[0]); i ++) {

        iErrCode = m_prwBucketLock[i].Initialize();
        if (iErrCode != OK) {
           return iErrCode;
        }
    }

    return OK;
}

int FileHeap::OpenNew (const char* pszFileName, Size cbSize, unsigned int iFlags) {

    int iErrCode;
    Bucket i, iBucket;

    Size sRealSize = cbSize;

    if (sRealSize == 0) {
        Assert (!"Initial size cannot be zero");
        return ERROR_INVALID_ARGUMENT;
    }

    sRealSize += SIZE_OF_BLOCK_HEADER;
    if (sRealSize > MAX_BLOCK_SIZE) {
        Assert (!"Initial size cannot be that large");
        return ERROR_INVALID_ARGUMENT;
    }

    iBucket = GetBucket (sRealSize);

    // Open a new file
    iErrCode = m_mmfData.OpenNew (
        pszFileName, 
        ALIGN (BUCKET_SIZE[iBucket] + SIZE_OF_MANAGER_HEADER, ALIGNMENT_BASE),
        iFlags
        );

    if (iErrCode != OK) {
        Assert (!"Could not open file, or size was too large");
        return iErrCode;
    }

    //
    // Format fresh header
    //

    Assert (SIZE_OF_MANAGER_HEADER % ALIGNMENT_BASE == 0);

    m_pHeader = (FileHeapHeader*) m_mmfData.GetAddress();
    Assert (m_pHeader != NULL);

    m_pHeader->cNumFreeBlocks = 1;
    m_pHeader->cNumAllocatedBlocks = 0;
    m_pHeader->cbNumUsedBytes = 0;
    m_pHeader->cbNumSlackBytes = 0;
    m_pHeader->oLastBlock = SIZE_OF_MANAGER_HEADER;
    m_pHeader->oPadding = BLOCK_SIGNATURE;

    for (i = 0; i < NUM_BUCKETS; i ++) {
        m_pHeader->oChainEntry[i] = NO_OFFSET;
    }

    // Format first block
    InitFreeBlock (GetBlockHeader (SIZE_OF_MANAGER_HEADER), NO_OFFSET, iBucket);

    // Add first block to buckets
    AddBlockToBucket (SIZE_OF_MANAGER_HEADER, iBucket);

    return OK;
}

int FileHeap::OpenExisting (const char* pszFileName, unsigned int iFlags) {

    int iErrCode;

    // Open the existing table
    iErrCode = m_mmfData.OpenExisting (pszFileName, iFlags);
    if (iErrCode != OK) {
        Assert (!"Could not open file");
        return iErrCode;
    }

    // Check size
    if (m_mmfData.GetSize() == 0) {
        m_mmfData.Close();
        Assert (!"File is zero sized");
        return ERROR_OUT_OF_DISK_SPACE;
    }

    // Init header
    m_pHeader = (FileHeapHeader*) m_mmfData.GetAddress();
    Assert (m_pHeader != NULL);

    return OK;
}

int FileHeap::Flush() {

    // Lock to protect against resizes
    m_rwGlobalLock.WaitReader();
    int iErrCode = m_mmfData.Flush();
    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

bool FileHeap::Check() {
    
    bool bGood = true;

    Count cNumFreeBlocks = 0, cNumAllocatedBlocks = 0;
    Size cbUsedBytes = 0, cbSlackBytes = 0;
    Count pcNumFreeBlocks [NUM_BUCKETS] = {0};
    Size cbTotalSize = 0;

    Bucket i, iBucket;
    Offset oBlock, oPrevBlock;
    BlockHeader* pBlock;

    //
    // Lock
    //
    m_rwGlobalLock.WaitWriter();

    //
    // Count blocks
    //
    pBlock = GetBlockHeaderFromUserOffset (GetFirstBlock());
    while (pBlock != NULL) {

        oBlock = GetOffsetFromBlockHeader (pBlock);
        if (!IsValidBlock (oBlock)) {
            Assert (!"Invalid block");
            bGood = false;
            goto Cleanup;
        }
        
        cbTotalSize += pBlock->cbSize;
        
        iBucket = GetBucket (pBlock->cbSize);
        if (!IsValidBucket (iBucket)) {
            Assert (!"Invalid bucket");
            bGood = false;
            goto Cleanup;
        }
        
        if (pBlock->cbSize != BUCKET_SIZE[iBucket]) {
            Assert (!"Block is in wrong bucket");
            bGood = false;
            goto Cleanup;
        }
        
        if (pBlock->bFree) {

            cNumFreeBlocks ++;
            pcNumFreeBlocks [iBucket] ++;

            if (pBlock->cbUserSize != NO_SIZE) {
                Assert (!"User size not nulled out properly");
                goto Cleanup;
            }
            
        } else {

            Count cSlack = (Count) (pBlock->cbSize - SIZE_OF_BLOCK_HEADER - pBlock->cbUserSize);
            
            if (pBlock->cbUserSize > pBlock->cbSize - SIZE_OF_BLOCK_HEADER) {
                Assert (!"Invalid user size");
                bGood = false;
                goto Cleanup;
            }

            if (pBlock->cbUserSize == 0 || pBlock->cbUserSize == NO_SIZE) {
                Assert (!"User size should not be zero");
                bGood = false;
                goto Cleanup;
            }
            
            cNumAllocatedBlocks ++;

            cbUsedBytes += (Count) pBlock->cbUserSize;
            cbSlackBytes += cSlack;
        }
        
        pBlock = GetNextBlockHeader (pBlock);
    }
    
    if (cNumFreeBlocks != m_pHeader->cNumFreeBlocks || cNumAllocatedBlocks != m_pHeader->cNumAllocatedBlocks) {
        Assert (!"Block counts don't match");
        bGood = false;
        goto Cleanup;
    }

    if (cbUsedBytes != m_pHeader->cbNumUsedBytes || cbSlackBytes != m_pHeader->cbNumSlackBytes) {
        Assert (!"Byte counts don't match");
        bGood = false;
        goto Cleanup;
    }

    //
    // Verify lists
    //
    for (i = 0; i < NUM_BUCKETS; i ++) {

        cNumFreeBlocks = 0;
        oBlock = m_pHeader->oChainEntry[i];
        oPrevBlock = NO_OFFSET;

        while (oBlock != NO_OFFSET) {

            if (!IsValidBlock (oBlock)) {
                Assert (!"Invalid block");
                bGood = false;
                goto Cleanup;
            }

            cNumFreeBlocks ++;

            // Protect against infinite loops
            if (cNumFreeBlocks > pcNumFreeBlocks[i]) {
                break;
            }
            
            pBlock = GetBlockHeader (oBlock);
            Assert (pBlock != NULL);
            
            if (pBlock->cbSize != BUCKET_SIZE[i]) {
                Assert (!"Block is in wrong bucket");
                bGood = false;
                goto Cleanup;
            }

            if (pBlock->oPrevInChain != oPrevBlock) {
                Assert (!"Free chain is corrupt");
                bGood = false;
                goto Cleanup;
            }
            
            oPrevBlock = oBlock;
            oBlock = pBlock->oNextInChain;
        }

        if (cNumFreeBlocks != pcNumFreeBlocks[i]) {

            Assert (!"Free block counts don't match");
            bGood = false;
            goto Cleanup;
        }
    }

    //
    // Check size
    //
    if (cbTotalSize + SIZE_OF_MANAGER_HEADER != GetSize()) {
        Assert (!"Size is wrong");
        bGood = false;
        goto Cleanup;
    }

Cleanup:

    //
    // Unlock
    //
    m_rwGlobalLock.SignalWriter();

    return bGood;
}

Offset FileHeap::Allocate (Size cbSize) {

    Bucket iBucket;
    Offset oOffset = NO_OFFSET;

    Size sRealSize = cbSize;

    m_rwGlobalLock.WaitReader();

    // Can't accept zero
    if (sRealSize <= 0 || sRealSize == NO_SIZE) {
        Assert (!"FileHeap::Allocate cannot allocate zero bytes");
        goto Cleanup;
    }

    // Add the block header
    sRealSize += SIZE_OF_BLOCK_HEADER;

    // Check max size
    if (sRealSize > MAX_BLOCK_SIZE) {
        Assert (!"FileHeap::Allocate cannot allocate that many bytes");
        goto Cleanup;
    }

    // Find the appropriate bucket for the request
    iBucket = GetBucket (sRealSize);
    if (!IsValidBucket (iBucket)) {
        Assert (!"Can't find bucket");
        goto Cleanup;
    }

    // Look for a block in the bucket's chain
    oOffset = GetUserOffset (AllocateBlockFromBucket (iBucket));
    if (oOffset != NO_OFFSET) {

        BlockHeader* pBlock = GetBlockHeaderFromUserOffset (oOffset);
        Assert (pBlock != NULL);

        pBlock->cbUserSize = cbSize;
        Assert (cbSize != 0 && cbSize != NO_SIZE);

        // More allocated bytes, more slack bytes
        Algorithm::AtomicIncrement(&m_pHeader->cbNumUsedBytes,  pBlock->cbUserSize);
        Algorithm::AtomicIncrement(&m_pHeader->cbNumSlackBytes, pBlock->cbSize - pBlock->cbUserSize - SIZE_OF_BLOCK_HEADER);

#ifdef _DEBUG
        // Mark up the allocated space
        memset (GetAddress (oOffset), BLOCK_FILLER, (size_t)pBlock->cbSize - SIZE_OF_BLOCK_HEADER);
#endif

    }
    else Assert (!"Out of disk space");

Cleanup:

    m_rwGlobalLock.SignalReader();

    return oOffset;
}


Offset FileHeap::Reallocate (Offset oBlock, Size cbSize) {

    BlockHeader* pOldHeader;
    Bucket iBucket, iNewBucket, iWantBucket;

    Size sRealSize = cbSize, sOldSize, sOldUserSize;
    Offset oNewBlock = NO_OFFSET, oRealBlock;

    m_rwGlobalLock.WaitReader();

    if (!IsValidUserBlock (oBlock)) {
        Assert (!"Bad block provided to Free");
        goto Cleanup;
    }

    oRealBlock = GetOffsetFromUserOffset (oBlock);
    Assert (IsValidBlock (oRealBlock));
#ifdef _DEBUG
    Assert (IsSlackSpaceIntact (oRealBlock));
#endif
    pOldHeader = GetBlockHeader (oRealBlock);
    Assert (pOldHeader != NULL);

    if (pOldHeader->bFree) {
        Assert (!"Free block provided to Free");
        goto Cleanup;
    }

    // Add the block header
    sRealSize += SIZE_OF_BLOCK_HEADER;

    // Check max size
    if (sRealSize > MAX_BLOCK_SIZE) {
        Assert (!"FileHeap::Allocate cannot allocate that many bytes");
        goto Cleanup;
    }

    sOldSize = pOldHeader->cbSize;
    sOldUserSize = pOldHeader->cbUserSize;

    // Return same block if size requested is less than actual size
    if (sRealSize <= sOldSize) {

        // Calculate the difference in size
        Count cDiff = (Count) (cbSize - sOldUserSize);

        // Return the same block
        oNewBlock = oRealBlock;

        // Set the new user size
        pOldHeader->cbUserSize = cbSize;
        Assert (cbSize != 0 && cbSize != NO_SIZE);

#ifdef _DEBUG
        // If the user block shrunk, mark up the newly freed space
        if (cDiff < 0) {
            Size cbSlackSpace;
            void* pSlackSpace = GetSlackSpace (oRealBlock, &cbSlackSpace);
            memset (pSlackSpace, BLOCK_FILLER, (size_t)cbSlackSpace);
        }
#endif
        // More allocated bytes, less slack bytes
        Algorithm::AtomicIncrement(&m_pHeader->cbNumUsedBytes, cDiff);
        Algorithm::AtomicDecrement(&m_pHeader->cbNumSlackBytes, cDiff);

        goto Cleanup;
    }

    // Get buckets
    iBucket = GetBucket (sOldSize);
    iWantBucket = GetBucket (sRealSize);

    Assert (IsValidBucket (iBucket));
    Assert (IsValidBucket (iWantBucket));
    Assert (BUCKET_SIZE[iWantBucket] >= sRealSize);
    Assert (BUCKET_SIZE[iBucket] == pOldHeader->cbSize);

    // Try to coalesce with the next blocks
    iNewBucket = CoalesceBlock (oRealBlock, iBucket, iWantBucket, false);
    Assert (IsValidBucket (iNewBucket));

    if (iNewBucket != iBucket) {

        Assert (iNewBucket >= iBucket);
        Assert (iWantBucket >= iNewBucket);

        if (iNewBucket == iWantBucket) {

            // We got lucky with our coalesce
            // Adjust space usage counts accordingly
            Algorithm::AtomicIncrement(&m_pHeader->cbNumUsedBytes, cbSize - pOldHeader->cbUserSize);
            Algorithm::AtomicDecrement(&m_pHeader->cbNumSlackBytes, cbSize - sOldUserSize);
     
            pOldHeader->cbUserSize = cbSize;
            Assert (cbSize != 0 && cbSize != NO_SIZE);

            oNewBlock = GetOffsetFromUserOffset (oBlock);
            goto Cleanup;
        }
    }

    m_rwGlobalLock.SignalReader();

    // At this point, we'll just have to allocate a new block of the desired size
    oNewBlock = Allocate (cbSize);
    if (oNewBlock == NO_OFFSET) {
        Assert (!"Out of disk space");
        return NO_OFFSET;
    }

    m_rwGlobalLock.WaitReader();

    // Note: we refresh the old header because the old pointer might have become stale!
    // Our allocate might have resized the file and changed every real address
    pOldHeader = GetBlockHeader (oRealBlock);
    Assert (pOldHeader != NULL);

    MemCopy (oNewBlock, oBlock, pOldHeader->cbUserSize);
    Free (oBlock);

    m_rwGlobalLock.SignalReader();

    return oNewBlock;

Cleanup:

    m_rwGlobalLock.SignalReader();

    return GetUserOffset (oNewBlock);
}

void FileHeap::Free (Offset oBlock) {

    BlockHeader* pBlock;
    Bucket iBucket;

    Offset oRealBlock;

    if (!IsValidUserBlock (oBlock)) {
        Assert (!"Bad block provided to Free");
        goto Cleanup;
    }

    oRealBlock = GetOffsetFromUserOffset (oBlock);
    Assert (IsValidBlock (oRealBlock));

    pBlock = GetBlockHeader (oRealBlock);
    Assert (pBlock != NULL);

    if (pBlock->bFree) {
        Assert (!"Free block being double-freed");
        goto Cleanup;
    }
#ifdef _DEBUG
    Assert (IsSlackSpaceIntact (oRealBlock));
#endif
    // Get bucket
    iBucket = GetBucket (pBlock->cbSize);
    Assert (IsValidBucket (iBucket));
    Assert (pBlock->cbSize == BUCKET_SIZE[iBucket]);

    // Less allocated and slack bytes
    Algorithm::AtomicDecrement(&m_pHeader->cbNumUsedBytes, pBlock->cbUserSize);
    Algorithm::AtomicDecrement(&m_pHeader->cbNumSlackBytes, pBlock->cbSize - pBlock->cbUserSize - SIZE_OF_BLOCK_HEADER);

    // Free the block
    pBlock->bFree = true;
    pBlock->cbUserSize = NO_SIZE;

    // Coalesce the block
    iBucket = CoalesceBlock (oRealBlock, iBucket, NUM_BUCKETS, true);
    Assert (IsValidBucket (iBucket));

    // Insert free block into hash table
    m_prwBucketLock[iBucket].WaitWriter();
    AddBlockToBucket (oRealBlock, iBucket);
    m_prwBucketLock[iBucket].SignalWriter();

    // One more free block
    Algorithm::AtomicIncrement (&m_pHeader->cNumFreeBlocks);

    // One less allocated block
    Algorithm::AtomicDecrement (&m_pHeader->cNumAllocatedBlocks);

Cleanup:
    ;
}

const void* FileHeap::GetBaseAddress() {

    return m_mmfData.GetAddress();
}

Size FileHeap::GetSize() {

    return m_mmfData.GetSize();
}

void* FileHeap::GetAddress (Offset oOffset) {

    /*if (oOffset >= m_mmfData.GetSize()) {
        Assert (!"Bad offset provided to GetAddress");
        return NULL;
    }*/

    return (char*) m_mmfData.GetAddress() + oOffset;
}

Offset FileHeap::GetOffset (const void* pAddress) {

    return (Offset) pAddress - (Offset) GetBaseAddress();
}

void FileHeap::GetStatistics (FileHeapStatistics* pfhsStats) {

    memset (pfhsStats, 0, sizeof (FileHeapStatistics));

    pfhsStats->cbSize = m_mmfData.GetSize();
    pfhsStats->cNumAllocatedBlocks = m_pHeader->cNumAllocatedBlocks;
    pfhsStats->cNumFreeBlocks = m_pHeader->cNumFreeBlocks;
    pfhsStats->cbNumSlackBytes = m_pHeader->cbNumSlackBytes;
    pfhsStats->cbNumUsedBytes  = m_pHeader->cbNumUsedBytes;
}


Count FileHeap::GetNumAllocatedBlocks() {

    return m_pHeader->cNumAllocatedBlocks;
}

Offset FileHeap::GetFirstBlock() {

    return GetUserOffset (SIZE_OF_MANAGER_HEADER);
}

Offset FileHeap::GetNextBlock (Offset oBlock) {

    return GetUserOffset (GetNextBlockInternal (GetOffsetFromUserOffset (oBlock)));
}

Size FileHeap::GetBlockSize (Offset oBlock) {

    BlockHeader* pHeader;

    if (!IsValidUserBlock (oBlock)) {
        Assert (!"Bad block provided to GetRealAddress");
        return NO_SIZE;
    }

    pHeader = GetBlockHeaderFromUserOffset (oBlock);
    Assert (pHeader != NULL);

    Assert (!pHeader->bFree && "GetBlockSize should never be called on a free block");

    return pHeader->cbUserSize;
}

bool FileHeap::IsBlockFree (Offset oBlock) {

    BlockHeader* pHeader;

    if (!IsValidUserBlock (oBlock)) {
        Assert (!"Bad block provided to IsBlockFree");
        return false;
    }

    pHeader = GetBlockHeaderFromUserOffset (oBlock);
    Assert (pHeader != NULL);

    return pHeader->bFree;
}

//
// Utility
//

Bucket FileHeap::GetBucket (Size cbSize) {

    // TODO - optimize
    unsigned int i;
    Size sShift = cbSize - 1;

    for (i = 0; i < sizeof(Size)*8 && sShift > 0; i ++) {
        sShift = sShift >> 1;
    }

    Assert (i > 0);

    return (i <= MIN_BLOCK_BIT_SIZE) ? 0 : i - MIN_BLOCK_BIT_SIZE;
}


BlockHeader* FileHeap::GetBlockHeader (Offset oBlock) {

    Assert (oBlock != NO_OFFSET);

    return (BlockHeader*) ((char*) m_mmfData.GetAddress() + oBlock);
}

BlockHeader* FileHeap::GetBlockHeaderFromUserOffset (Offset oUserBlock) {

    if (oUserBlock == NO_OFFSET) {

        Assert (!"Bad argument to GetBlockHeaderFromUserOffset");
        return NULL;
    }

    return (BlockHeader*) ((char*) m_mmfData.GetAddress() + oUserBlock - SIZE_OF_BLOCK_HEADER);
}

Offset FileHeap::GetOffsetFromBlockHeader (const BlockHeader* pBlockHeader) {

    return (Offset) pBlockHeader - (Offset) m_mmfData.GetAddress();
}

Offset FileHeap::GetUserOffset (Offset oBlock) {

    if (oBlock == NO_OFFSET) {
        return NO_OFFSET;
    }

    return oBlock + SIZE_OF_BLOCK_HEADER;
}

Offset FileHeap::GetOffsetFromUserOffset (Offset oBlock) {

    if (oBlock == NO_OFFSET) {
        return NO_OFFSET;
    }

    return oBlock - SIZE_OF_BLOCK_HEADER;
}

BlockHeader* FileHeap::GetNextBlockHeader (const BlockHeader* pBlockHeader) {

    Assert (pBlockHeader != NULL);

    BlockHeader* pNextBlock = (BlockHeader*) ((char*) pBlockHeader + pBlockHeader->cbSize);

    char* pszLastAddress = (char*) m_mmfData.GetAddress() + m_mmfData.GetSize();

    if ((char*) pNextBlock >= pszLastAddress) {
        pNextBlock = NULL;
    }

    return pNextBlock;
}

BlockHeader* FileHeap::GetPrevBlockHeader (const BlockHeader* pBlockHeader) {

    Assert (pBlockHeader != NULL);

    if (pBlockHeader->oPrevBlock == NO_OFFSET) {
        return NULL;
    }

    BlockHeader* pBlock = GetBlockHeader (pBlockHeader->oPrevBlock);

    Assert (pBlock < m_mmfData.GetAddress());

    return pBlock;
}

void FileHeap::InitFreeBlock (BlockHeader* pBlockHeader, Offset oPrevBlock, Bucket iBucket) {

    pBlockHeader->oPrevBlock = oPrevBlock;
    pBlockHeader->cbSize = BUCKET_SIZE[iBucket];
    pBlockHeader->cbUserSize = NO_SIZE;
    pBlockHeader->bFree = true;
    pBlockHeader->iSignature = BLOCK_SIGNATURE;
    pBlockHeader->oPadding = (Offset) BLOCK_SIGNATURE;  // Not 64-bit clean
}

Offset FileHeap::GetBlockOffset (const BlockHeader* pBlockHeader) {

    return (Offset) pBlockHeader - (Offset) m_mmfData.GetAddress();
}

void FileHeap::MemCopy (Offset oSrc, Offset oDest, Size cbSize) {

    Assert (oSrc != NO_OFFSET && oDest != NO_OFFSET && cbSize != 0);

    char* pszBase = (char*) GetBaseAddress();
    memcpy (pszBase + oSrc, pszBase + oDest, (size_t)cbSize);
}

void FileHeap::MemMove (Offset oSrc, Offset oDest, Size cbSize) {

    Assert (oSrc != NO_OFFSET && oDest != NO_OFFSET && cbSize != 0);

    char* pszBase = (char*) GetBaseAddress();
    memmove (pszBase + oSrc, pszBase + oDest, (size_t)cbSize);
}

void FileHeap::MemSet (Offset oSrc, char cByte, Size cbSize) {

    Assert (oSrc != NO_OFFSET && cbSize != 0);

    char* pszBase = (char*) GetBaseAddress();
    memset (pszBase + oSrc, cByte, (size_t)cbSize);
}

#ifdef _DEBUG

bool FileHeap::IsSlackSpaceIntact (Offset oBlock) {

    Size i, sFreeSize;
    unsigned char* pszFreeSpace = (unsigned char*) GetSlackSpace (oBlock, &sFreeSize);

    // Check the extra space after the block
    for (i = 0; i < sFreeSize; i ++) {
         
        if (pszFreeSpace[i] != BLOCK_FILLER) {
            return false;
        }
    }

    return true;
}

void* FileHeap::GetSlackSpace (Offset oBlock, Size* psSlackSize) {

    BlockHeader* pBlock = GetBlockHeader (oBlock);

    *psSlackSize = pBlock->cbSize - SIZE_OF_BLOCK_HEADER - pBlock->cbUserSize;
    Assert (*psSlackSize >= 0);

    return GetAddress (oBlock + SIZE_OF_BLOCK_HEADER + pBlock->cbUserSize);
}

#endif

bool FileHeap::IsValidBlock (Offset oBlock) {

    if (oBlock == NO_OFFSET) {
        return false;
    }

    if (oBlock >= m_mmfData.GetSize()) {
        return false;
    }

    if ((oBlock - SIZE_OF_MANAGER_HEADER) % BUCKET_SIZE[0] != 0) {
        return false;
    }

    BlockHeader* pBlock = GetBlockHeader (oBlock);

    if (pBlock->iSignature != BLOCK_SIGNATURE || pBlock->oPadding != (Offset) BLOCK_SIGNATURE) {
        return false;
    }

    return true;
}

bool FileHeap::IsValidUserBlock (Offset oBlock) {

    return IsValidBlock (GetOffsetFromUserOffset (oBlock));
}

bool FileHeap::IsValidBucket (Bucket iBucket) {

    return iBucket >= 0 && iBucket < NUM_BUCKETS;
}

Offset FileHeap::GetNextBlockInternal (Offset oBlock) {

    BlockHeader* pHeader;

    if (!IsValidBlock (oBlock)) {
        Assert (!"Bad argument provided to GetNextBlockInternal");
        return NO_OFFSET;
    }

    pHeader = GetBlockHeader (oBlock);
    Assert (pHeader != NULL);

    pHeader = GetNextBlockHeader (pHeader);
    if (pHeader == NULL) {
        return NO_OFFSET;
    }

    return GetBlockOffset (pHeader);
}

//
// Internal heap
//

Offset FileHeap::AllocateBlockFromBucket (Bucket iBucket) {

    Offset oBlock;
    BlockHeader* pBlock = NULL;
    Bucket i;

    Assert (IsValidBucket (iBucket));

    // Take a flex-lock on the bucket
    m_prwBucketLock[iBucket].WaitReaderWriter();

    oBlock = m_pHeader->oChainEntry[iBucket];

    // Look for a block in the bucket's chain
    if (oBlock != NO_OFFSET) {

        // We found a block, so upgrade to a write lock
        m_prwBucketLock[iBucket].UpgradeReaderWriter();

        // Remove the block from the chain
        RemoveBlockFromBucket (oBlock, iBucket);

        pBlock = GetBlockHeader (oBlock);
        Assert (pBlock != NULL);
        Assert (pBlock->bFree);
        pBlock->bFree = false;

        // Release the lock
        m_prwBucketLock[iBucket].DowngradeReaderWriter();
        m_prwBucketLock[iBucket].SignalReaderWriter();

        // One free block less
        Algorithm::AtomicDecrement (&m_pHeader->cNumFreeBlocks);

        // We're done

    } else {

        // Release the flex lock - we don't want deadlocks
        m_prwBucketLock[iBucket].SignalReaderWriter();

        // There's no block of the size we wanted - let's look at bigger blocks
        for (i = iBucket + 1; i < NUM_BUCKETS; i ++) {

            // Take a flex-lock
            m_prwBucketLock[i].WaitReaderWriter();

            oBlock = m_pHeader->oChainEntry[i];
            if (oBlock == NO_OFFSET) {

                m_prwBucketLock[i].SignalReaderWriter();
            
            } else {

                // Remove the big block from the free list
                m_prwBucketLock[i].UpgradeReaderWriter();

                RemoveBlockFromBucket (oBlock, i);
                
                pBlock = GetBlockHeader (oBlock);
                Assert (pBlock != NULL);
                Assert (pBlock->bFree);
                pBlock->bFree = false;

                // Release the flex-lock
                m_prwBucketLock[i].DowngradeReaderWriter();
                m_prwBucketLock[i].SignalReaderWriter();

                // Carve up the bigger block into the size we want
                DivideBlock (oBlock, iBucket, i);

                // More free blocks
                Count cFreeBlockChange = i - iBucket - 1;

                if (cFreeBlockChange > 0) {
                    Algorithm::AtomicIncrement (&m_pHeader->cNumFreeBlocks, cFreeBlockChange);
                }

                // Break out of the loop
                break;
            }
        }

        if (oBlock == NO_OFFSET) {

            int iErrCode = Resize (iBucket);
            if (iErrCode != OK) {
                Assert (!"Out of disk space");
                return NO_OFFSET;
            }

            // Recurse
            return AllocateBlockFromBucket (iBucket);
        }
    }

    // If we got a block, mark it up as not free
    Assert (pBlock != NULL);
    Assert (pBlock->cbSize == BUCKET_SIZE [iBucket]);
    Assert (pBlock->iSignature == BLOCK_SIGNATURE);
    Assert (pBlock->oPadding == (Offset) BLOCK_SIGNATURE);

    // One more allocated block
    Algorithm::AtomicIncrement (&m_pHeader->cNumAllocatedBlocks);

    return oBlock;
}

void FileHeap::DivideBlock (Offset oBlock, Bucket iSmallBucket, Bucket iBigBucket) {

    Bucket i;
    Offset oNewBlock = oBlock;

    Assert (IsValidBlock (oBlock));
    Assert (IsValidBucket (iSmallBucket));
    Assert (IsValidBucket (iBigBucket));

    // Sanity
    BlockHeader* pBlock = GetBlockHeader (oBlock);

    Assert (pBlock != NULL);
    Assert (pBlock->cbSize == BUCKET_SIZE [iBigBucket]);

    // Mark up the new little chunk with its new size
    pBlock->cbSize = BUCKET_SIZE [iSmallBucket];

    // If we're splitting the last block, then the second half is now the last
//  if (m_pHeader->oLastBlock == oBlock) {
//      m_pHeader->oLastBlock = GetNextBlockInternal (oBlock);
//  }

    // Initialize the rest of the blocks
    for (i = iSmallBucket; i < iBigBucket; i ++) {

        oNewBlock = GetBlockOffset (pBlock);
        pBlock = GetNextBlockHeader (pBlock);
        Assert (pBlock != NULL);

        InitFreeBlock (pBlock, oNewBlock, i);

        m_prwBucketLock[i].WaitWriter();
        AddBlockToBucket (GetOffsetFromBlockHeader (pBlock), i);
        m_prwBucketLock[i].SignalWriter();
    }

    // If we're splitting the last block, then the second half is now the last
    if (m_pHeader->oLastBlock == oBlock) {
        m_pHeader->oLastBlock = GetNextBlockInternal (oNewBlock);
    }
}

void FileHeap::RemoveBlockFromBucket (Offset oBlock, Bucket iBucket) {

    Assert (IsValidBlock (oBlock));
    Assert (IsValidBucket (iBucket));

    BlockHeader* pBlock = GetBlockHeader (oBlock);
    Assert (pBlock != NULL);
    Assert (pBlock->cbSize == BUCKET_SIZE [iBucket]);

    Offset oNext = pBlock->oNextInChain;
    Offset oPrev = pBlock->oPrevInChain;

    if (oNext != NO_OFFSET) {

        // Next's Prev points to our Prev
        BlockHeader* pNext = GetBlockHeader (oNext);
        Assert (pNext != NULL);

        pNext->oPrevInChain = oPrev;
    }

    if (oPrev != NO_OFFSET) {

        // Prev's Next points to our Next
        BlockHeader* pPrev = GetBlockHeader (oPrev);
        Assert (pPrev != NULL);

        pPrev->oNextInChain = oNext;
    }

    if (m_pHeader->oChainEntry[iBucket] == oBlock) {

        // Chain header is our Next
        m_pHeader->oChainEntry[iBucket] = oNext;
    }
}

void FileHeap::AddBlockToBucket (Offset oBlock, Bucket iBucket) {

    Assert (IsValidBlock (oBlock));
    Assert (IsValidBucket (iBucket));

    Offset oHead = m_pHeader->oChainEntry[iBucket];

    BlockHeader* pBlock = GetBlockHeader (oBlock);
    Assert (pBlock != NULL);
    Assert (pBlock->cbSize == BUCKET_SIZE [iBucket]);

    pBlock->oPrevInChain = NO_OFFSET;
    pBlock->oNextInChain = oHead;

    m_pHeader->oChainEntry[iBucket] = oBlock;

    if (oHead != NO_OFFSET) {

        BlockHeader* pNext = GetBlockHeader (oHead);
        Assert (pNext != NULL);

        pNext->oPrevInChain = oBlock;
    }
}

Bucket FileHeap::CoalesceBlock (Offset oBlock, Bucket iBucket, Bucket iWantBucket, bool bFree) {

    Bucket iNewBucket;

    Assert (IsValidBlock (oBlock));
    Assert (IsValidBucket (iBucket));

    // No point in coalescing the largest bucket type
    if (iBucket == NUM_BUCKETS - 1) {
        return iBucket;
    }

    // Try to coalesce with the next block
    iNewBucket = CoalesceNext (oBlock, iBucket, bFree);
    if (iNewBucket != iBucket && iNewBucket != iWantBucket) {

        Assert (iNewBucket >= iBucket);

        // We coalesced with the block in front of us
        // So let's try again - recurse with the next largest block
        iNewBucket = CoalesceBlock (oBlock, iNewBucket, iWantBucket, bFree);
    }

    return iNewBucket;
}


Bucket FileHeap::CoalesceNext (Offset oBlock, Bucket iBucket, bool bFree) {

    Assert (IsValidBlock (oBlock));
    Assert (IsValidBucket (iBucket));

    Offset oNextBlock = GetNextBlockInternal (oBlock);
    if (oNextBlock == NO_OFFSET) {
        Assert (oBlock == m_pHeader->oLastBlock);
        return iBucket;
    }

    BlockHeader* pNextBlock = GetBlockHeader (oNextBlock);
    Assert (pNextBlock != NULL);

    // Optimize concurrency
    m_prwBucketLock[iBucket].WaitReader();

    if (pNextBlock->cbSize == BUCKET_SIZE[iBucket] && pNextBlock->bFree) {

        m_prwBucketLock[iBucket].SignalReader();
        m_prwBucketLock[iBucket].WaitWriter();

        if (pNextBlock->cbSize == BUCKET_SIZE[iBucket] && pNextBlock->bFree) {

            // Remove the next block from the bucket
            RemoveBlockFromBucket (oNextBlock, iBucket);

            // If we're coalescing with the last block, then we're now the last
            if (m_pHeader->oLastBlock == oNextBlock) {
                m_pHeader->oLastBlock = oBlock;
            }

            m_prwBucketLock[iBucket].SignalWriter();
            
            // Update our own size
            BlockHeader* pBlock = GetBlockHeader (oBlock);
            Assert (pNextBlock != NULL);

#ifdef _DEBUG

            // Mark up the coalesced space
            BlockHeader* pDeadBlock = GetBlockHeader (oNextBlock);
            memset (pDeadBlock, BLOCK_FILLER, (size_t)pDeadBlock->cbSize);
#endif
            // One less free block
            Algorithm::AtomicDecrement (&m_pHeader->cNumFreeBlocks);

            if (!bFree) {

                // A free block was coalesced into an allocated block
                // No more space is in use, but there's a lot more slack
                // TODO: use AtomicIncrement when available
                Algorithm::AtomicIncrement (&m_pHeader->cbNumSlackBytes, (int) pBlock->cbSize);
            }

            pBlock->cbSize *= 2;

            return iBucket + 1;

        } else {

            m_prwBucketLock[iBucket].SignalWriter();
        }

    } else {

        m_prwBucketLock[iBucket].SignalReader();
    }

    return iBucket;
}

int FileHeap::Resize (Bucket iBucket) {

    int iErrCode;
    Size cbOldSize;

    // Add enough space for sixteen new buckets
    iBucket += 4;
    if (iBucket >= NUM_BUCKETS) {
        iBucket = NUM_BUCKETS - 1;
    }

    Size cbAddedSize = BUCKET_SIZE [iBucket];

    // Lock for real
    m_rwGlobalLock.SignalReader();
    m_rwGlobalLock.WaitWriter();

    // Get old size
    cbOldSize = m_mmfData.GetSize();
    
    // Expand
    iErrCode = m_mmfData.Expand((size_t)cbAddedSize);
    if (iErrCode == OK) {

        // Re-init header
        m_pHeader = (FileHeapHeader*) m_mmfData.GetAddress();
        Assert (m_pHeader != NULL);

        // Get last bucket
        Offset oLastBlock = m_pHeader->oLastBlock;

        // Get new space header
        BlockHeader* pHeader = GetBlockHeader (cbOldSize);

        // Format new space
        InitFreeBlock (pHeader, oLastBlock, iBucket);
        
        // We're now the last block
        m_pHeader->oLastBlock = cbOldSize;

        // One more free block in the world
        m_pHeader->cNumFreeBlocks ++;

        m_rwGlobalLock.SignalWriter();

        // Add new space to appropriate bucket
        AddBlockToBucket (cbOldSize, iBucket);

    } else {

        Assert (!"Out of disk or virtual memory space");

        // Unlock
        m_rwGlobalLock.SignalWriter();
    }

    m_rwGlobalLock.WaitReader();

    return iErrCode;
}