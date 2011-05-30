// TableContext.h: interface for the TableContext class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABLECONTEXT_H__215B2851_5E8F_495B_856A_06B5FDA03818__INCLUDED_)
#define AFX_TABLECONTEXT_H__215B2851_5E8F_495B_856A_06B5FDA03818__INCLUDED_

#include "Osal/MemoryMappedFile.h"
#include "Osal/FifoQueue.h"

#include "CDatabase.h"
#include "Template.h"

// Shorthand
#define GetTableHeap m_pDatabase->GetTableFileHeap
#define GetVarLenHeap m_pDatabase->GetVariableDataFileHeap
#define GetMetaDataHeap m_pDatabase->GetMetaDataFileHeap

// Table row header
struct TableHeader {
    unsigned int iNumRows;
    unsigned int iTerminatorRowKey;
    unsigned int iNumBitmapElements;
    Offset oRowBitmapOffset;
    Offset poIndexOffset[1];
};

// Index header
struct IndexBucket {
    Offset oChain;
};

struct IndexHeader {
    Offset oFreeList;
    IndexBucket pBucket[1];
};

#define KEYS_PER_INDEX_NODE 3

struct IndexNode {
    Offset oNext;
    unsigned int piKey [KEYS_PER_INDEX_NODE];
};

//
// TableContext
//

class TableContext {

    friend class ReadTable;
    friend class WriteTable;
    friend class SearchTable;
    friend class TransactionTable;

protected:

    //
    // Data
    //

    Database* m_pDatabase;
    Template* m_pTemplate;

    char* m_pszName;

    Offset m_oBaseOffset;
    Offset m_oTableHeader;
    Offset m_oFirstRow;

    //
    // Private methods
    //

    void SetRowValidity (unsigned int iKey, bool bValid);
    size_t* GetRowBitmap();

    //
    // Index utils
    //

    int CreateAllIndexes (unsigned int iNumNewRows);
    void DeleteAllIndexes();

    Offset AllocateIndexNode (unsigned int iIndex);
    void FreeIndexNode (Offset oBaseOffset, Offset oNode);

    void FormatIndexNodes (Offset oBaseOffset, Offset oFirstNode, size_t stNumNodes);

    unsigned int GetIndexForColumn (unsigned int iColumn);
    unsigned int GetIndexFlags (unsigned int iIndex) {
        Assert (iIndex < GetNumIndexColumns());
        return m_pTemplate->TemplateData.IndexFlags[iIndex];
    }

    inline Offset GetIndexOffset (unsigned int iIndex) {
        Assert (iIndex < GetNumIndexColumns());
        return GetTableHeader()->poIndexOffset[iIndex];
    }

    inline unsigned int GetNumIndexBuckets() {
        unsigned int iNumBuckets = m_pTemplate->TemplateData.NumRowsGuess;
        Assert (iNumBuckets > 0 && iNumBuckets <= MAX_NUM_INDEX_BUCKETS);
        return iNumBuckets;
    }

    inline IndexHeader* GetIndexHeader (Offset oOffset) {
        return (IndexHeader*) GetMetaDataHeap()->GetAddress (oOffset);
    }

    int CreateHash (unsigned int iIndex, unsigned int iHash, unsigned int iKey);
    int DeleteHash (unsigned int iIndex, unsigned int iHash, unsigned int iKey);

    template <class T> int IndexCheckUniqueData (
        T tData, unsigned int iIndex, unsigned int iKey, unsigned int iColumn
        );

    int IndexGetFirstKey (unsigned int iColumn, const char* pszData, unsigned int* piKey);

public:

    //
    // Methods
    //

    TableContext();
    ~TableContext();

    void SetDatabase (Database* pDatabase) {
        Assert (pDatabase != NULL);
        m_pDatabase = pDatabase;
    }
    
    inline Database* GetDatabase() {
        return m_pDatabase;
    }

    inline int GetTemplate (ITemplate** ppTemplate) {
        *ppTemplate = m_pTemplate;
        m_pTemplate->AddRef();
        return OK;
    }

    int Reload (Offset oTable);
    int Create (const char* pszTableName, Template* pTemplate);

    void Unload();
    void DeleteOnDisk();
    int Resize (unsigned int iNumNewRows);

    inline Offset GetRowOffset (unsigned int iKey) {
        return m_oFirstRow + iKey * GetRowSize();
    }

    inline void* GetData (unsigned int iKey, unsigned int iColumn) {
        Assert (iKey != NO_KEY && iColumn != NO_KEY);
        return GetTableHeap()->GetAddress (m_oFirstRow + GetColumnOffset (iColumn) + iKey * GetRowSize());
    }

    const char* GetStringData (unsigned int iKey, unsigned int iColumn);

    inline unsigned int GetNumColumns() {
        return m_pTemplate->TemplateData.NumColumns;
    }

    inline unsigned int GetNumIndexColumns() {
        return m_pTemplate->TemplateData.NumIndexes;
    }

    inline bool IsOneRow() {
        return m_pTemplate->TemplateData.OneRow;
    }

    inline size_t GetRowSize() {
        return m_pTemplate->RowSize;
    }

    inline VariantType GetColumnType (unsigned int iColumn) {
        Assert (iColumn < GetNumColumns());
        return m_pTemplate->TemplateData.Type[iColumn];
    }
    
    inline size_t GetColumnOffset (unsigned int iColumn) {
        Assert (iColumn < GetNumColumns());
        return m_pTemplate->ColOffset[iColumn];
    }

    inline size_t GetColumnSize (unsigned int iColumn) {
        Assert (iColumn < GetNumColumns());
        return m_pTemplate->TemplateData.Size[iColumn];
    }

    inline bool IsVariableLengthString (unsigned int iColumn) {
        Assert (iColumn < GetNumColumns());
        return 
            m_pTemplate->TemplateData.Type[iColumn] == V_STRING &&
            m_pTemplate->TemplateData.Size[iColumn] == VARIABLE_LENGTH_STRING;
    }

    inline bool HasVariableLengthData() {
        return m_pTemplate->HasVariableLengthData;
    }

    inline bool HasUniqueDataIndex() {
        return m_pTemplate->HasUniqueDataIndex;
    }

    inline const char* GetName() {
        return m_pszName;
    }

    inline Offset GetTableOffset (const void* pData) {
        return GetTableHeap()->GetOffset (pData) - m_oFirstRow;
    }

    inline Offset GetEndOfTable() {
        return m_oBaseOffset + GetTableHeap()->GetBlockSize (m_oBaseOffset);
    }

    //
    // Heap utils
    //

    inline Offset Allocate (FileHeap* pfhHeap, Size sSize) {
        
        pfhHeap->Unlock();
        Offset oOffset = pfhHeap->Allocate (sSize);
        pfhHeap->Lock();

        return oOffset;
    }

    inline void Free (FileHeap* pfhHeap, Offset oOffset) {
        
        pfhHeap->Unlock();
        pfhHeap->Free (oOffset);
        pfhHeap->Lock();
    }

    inline Offset Reallocate (FileHeap* pfhHeap, Offset oOffset, Size sSize) {

        pfhHeap->Unlock();
        Offset oNewOffset = pfhHeap->Reallocate (oOffset, sSize);
        pfhHeap->Lock();

        return oNewOffset;
    }

    inline void* GetAddress (FileHeap* pfhHeap, Offset oOffset) {
        return pfhHeap->GetAddress (oOffset);
    }

    //
    // TableData
    //

    inline void* GetAddressTable (Offset oOffset) {
        return GetTableHeap()->GetAddress (oOffset);
    }

    inline void* GetAddressFromTableOffset (Offset oTableOffset) {
        return GetTableHeap()->GetAddress (oTableOffset + m_oFirstRow);
    }

    //
    // VariableLengthData
    //

    inline Offset AllocateVarLen (Size sSize) {
        return Allocate (GetVarLenHeap(), sSize);
    }

    inline void FreeVarLen (Offset oOffset) {
        Free (GetVarLenHeap(), oOffset);
    }

    inline Offset ReallocateVarLen (Offset oOffset, Size sSize) {
        return Reallocate (GetVarLenHeap(), oOffset, sSize);
    }

    inline void* GetAddressVarLen (Offset oOffset) {
        return GetVarLenHeap()->GetAddress (oOffset);
    }

    void FreeAllVarLenData();

    //
    // MetaData
    //

    inline Offset AllocateMeta (Size sSize) {
        return Allocate (GetMetaDataHeap(), sSize);
    }

    inline void FreeMeta (Offset oOffset) {
        Free (GetMetaDataHeap(), oOffset);
    }

    inline Offset ReallocateMeta (Offset oOffset, Size sSize) {
        return Reallocate (GetMetaDataHeap(), oOffset, sSize);
    }

    inline void* GetAddressMeta (Offset oOffset) {
        return GetMetaDataHeap()->GetAddress (oOffset);
    }

    //
    // Table header
    //

    inline TableHeader* GetTableHeader() {
        Assert (m_oTableHeader != NO_OFFSET);
        return (TableHeader*) GetTableHeap()->GetAddress (m_oTableHeader);
    }

    inline unsigned int GetNumRows() {
        return GetTableHeader()->iNumRows;
    }

    inline void IncrementNumRows (int iInc) {
        Assert (iInc != 0);
        GetTableHeader()->iNumRows += iInc;
    }

    inline void SetNumRows (int iNewNumRows) {
        GetTableHeader()->iNumRows = iNewNumRows;
    }

    inline unsigned int GetTerminatorRowKey() {
        return GetTableHeader()->iTerminatorRowKey;
    }

    inline void IncrementTerminatorRowKey (int iInc) {

        Assert (iInc != 0);
        Assert (iInc > 0 || GetTerminatorRowKey() > (unsigned int) (- iInc));
        GetTableHeader()->iTerminatorRowKey += iInc;
    }

    inline void SetTerminatorRowKey (unsigned int iNewTerminatorKey) {
        GetTableHeader()->iTerminatorRowKey = iNewTerminatorKey;
    }

    //
    // Locking
    //

    inline void Lock() {
        GetVarLenHeap()->Lock();
        GetMetaDataHeap()->Lock();
        GetTableHeap()->Lock();
    }

    inline void Unlock() {
        GetTableHeap()->Unlock();
        GetMetaDataHeap()->Unlock();
        GetVarLenHeap()->Unlock();
    }

    inline void Freeze() {
        GetVarLenHeap()->Freeze();
        GetMetaDataHeap()->Freeze();
        GetTableHeap()->Freeze();
    }

    inline void Unfreeze() {
        GetTableHeap()->Unfreeze();
        GetMetaDataHeap()->Unfreeze();
        GetVarLenHeap()->Unfreeze();
    }

    //
    // Row bitmap
    //

    int ExpandMetaDataIfNecessary (unsigned int iMaxNumRows);

    bool IsValidRow (unsigned int iKey);
    
    inline void SetValidRow (unsigned int iKey) {
        SetRowValidity (iKey, true);
    }

    inline void SetInvalidRow (unsigned int iKey) {
        SetRowValidity (iKey, false);
    }

    void SetAllRowsInvalid();

    inline unsigned int FindFirstValidRow() {
        return FindNextValidRow (NO_KEY);
    }

    inline unsigned int FindFirstInvalidRow() {
        return FindNextInvalidRow (NO_KEY);
    }

    unsigned int FindNextInvalidRow (unsigned int iKey);
    unsigned int FindNextValidRow (unsigned int iKey);

    //
    // Indexing
    //

    int IndexWriteData (unsigned int iKey, unsigned int iColumn, int iData);
    int IndexWriteData (unsigned int iKey, unsigned int iColumn, float fData);
    int IndexWriteData (unsigned int iKey, unsigned int iColumn, const char* pszData);
    int IndexWriteData (unsigned int iKey, unsigned int iColumn, const UTCTime& tData);
    int IndexWriteData (unsigned int iKey, unsigned int iColumn, int64 i64Data);

    int IndexGetFirstKey (unsigned int iColumn, int iData, unsigned int* piKey);
    int IndexGetFirstKey (unsigned int iColumn, float fData, unsigned int* piKey);
    int IndexGetFirstKey (unsigned int iColumn, const char* pszData, bool bCaseInsensitive, 
        unsigned int* piKey);
    int IndexGetFirstKey (unsigned int iColumn, const UTCTime& tData, unsigned int* piKey);
    int IndexGetFirstKey (unsigned int iColumn, int64 i64Data, unsigned int* piKey);

    int IndexGetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, 
        unsigned int** ppiKey, unsigned int* piNumKeys);

    int IndexWriteColumn (unsigned int iColumn, int iData);
    int IndexWriteColumn (unsigned int iColumn, float fData);
    int IndexWriteColumn (unsigned int iColumn, const UTCTime& tData);
    int IndexWriteColumn (unsigned int iColumn, int64 i64Data);

    int IndexInsertRow (unsigned int iKey, const Variant* pvData);
    int IndexInsertDuplicateRows (unsigned int iNumRows, const unsigned int* piNewKey, 
        const Variant* pvColVal);

    int IndexDeleteRow (int iKey, const Variant* pvData);
    int IndexDeleteAllRows();

    // Indexing for searches
    int GetInitialSearchKeys (const SearchDefinition& sdSearch, unsigned int** ppiKeys, unsigned int* piNumKeys);
};

int AppendKey (unsigned int*& piKey, unsigned int& iNumKeys, unsigned int& iKeySpace, int iRowKey);

#endif // !defined(AFX_TABLECONTEXT_H__215B2851_5E8F_495B_856A_06B5FDA03818__INCLUDED_)