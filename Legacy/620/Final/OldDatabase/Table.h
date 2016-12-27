//
// Database.dll - A database library
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

#if !defined(AFX_TABLE_H__05370B5D_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)
#define AFX_TABLE_H__05370B5D_2751_11D3_A0DC_0050047FE2E2__INCLUDED_

#include <math.h>

#include "Osal/MemoryMappedFile.h"
#include "Osal/FifoQueue.h"
#include "Osal/Algorithm.h"
#include "Osal/ReadWriteLock.h"

#include "Index.h"
#include "Template.h"
#include "DatabaseStrings.h"

#define NumberOfColumns (m_pTemplate->TemplateData.NumColumns)
#define RowSize (m_pTemplate->RowSize)
#define Type(iColumn) ((m_pTemplate->TemplateData.Type)[iColumn])
#define Offset(iColumn) ((m_pTemplate->Offset)[iColumn])
#define Size(iColumn) ((m_pTemplate->TemplateData.Size)[iColumn])
#define IsOneRow (m_pTemplate->TemplateData.OneRow)
#define FileSize (m_mmfFile.GetSize())
#define EndOfFile (m_mmfFile.GetEndOfFile())
#define NumIndexCols (m_pTemplate->TemplateData.NumIndexes)
#define VariableLengthRows (m_pTemplate->VariableLengthRows)
#define NumInitialInsertionLengthRows (m_pTemplate->NumInitialInsertionLengthRows)

#define CHECK_LOAD int iErrCode = CheckLoad();  if (iErrCode != OK) { return iErrCode; }

class Table : public IObject {

    friend class ReadTable;
    friend class WriteTable;

private:

    char* m_pszName;
    char* m_pszFileName;

    bool m_bDelete;
    bool m_bLoaded;

    unsigned int m_iTerminatorRowKey;
    unsigned int m_iNumRows;

    MemoryMappedFile m_mmfFile;
    Template* m_pTemplate;
    Index* m_pIndex;

    ReadWriteLock m_rwLock;

    FifoQueue<unsigned int> m_fqFragQueue;

    void** m_ppAddress;
    size_t m_stAddressSpace;

    ROW_HEADER* RowHeader (unsigned int iKey);
    void* Data (unsigned int iKey, unsigned int iColumn);

    void* GetNextVariableLengthRow (void* pPrevRow);
    int InsertDuplicateVariableLengthRows (Variant* pvColVal, unsigned int iNumRows);

    Table (const char* pszName, Template* pTemplate);
    ~Table();

    int CheckLoad();
    int ReloadInternal (const char* pszFileName, bool bDelayLoad);

public:

    int Flush();

    IMPLEMENT_IOBJECT;

    static Table* CreateInstance (const char* pszName, Template* pTemplate);

    inline size_t GetSize() { return m_mmfFile.GetSize(); }

    inline const char* GetName() { return m_pszName; }

    int Reload (const char* pszFileName);
    int Create (const char* pszDirName);

    inline bool OneRow() { return m_pTemplate->TemplateData.OneRow; }

    int Backup (const char* pszBackupDir);

    inline void DeleteOnDisk() { m_bDelete = true; }

    inline void WaitReader() {
        m_rwLock.WaitReader();
    }

    inline void SignalReader() {
        m_rwLock.SignalReader();
    }

    inline void WaitWriter() {
        m_rwLock.WaitWriter();
    }

    inline void SignalWriter() {
        m_rwLock.SignalWriter();
    }

    int GetTemplate (ITemplate** ppTemplate);

    ///////////////
    // Table API //
    ///////////////

    // Standard operations
    int ReadData (unsigned int iKey, unsigned int iColumn, Variant* pvData);
    int WriteData (unsigned int iKey, unsigned int iColumn, const Variant& vData);
    int Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue);

    int WriteAnd (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteOr (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteXor (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteNot (unsigned int iKey, unsigned int iColumn);

    int WriteColumn (unsigned int iColumn, const Variant& vData);

    // Row operations
    int GetNumRows (unsigned int* piNumRows);
    int DoesRowExist (unsigned int iKey, bool* pbExists);

    int InsertRow (Variant* pvColVal, unsigned int* piKey);
    int InsertRows (Variant* pvColVal, unsigned int iNumCols);
    int InsertDuplicateRows (Variant* pvColVal, unsigned int iNumRows);

    int DeleteRow (unsigned int iKey);
    int DeleteAllRows();
    
    int ReadRow (unsigned int iKey, Variant** ppvData);

    // Column operations
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);
    int ReadColumns (unsigned int iNumColumns, const unsigned int* piColumn, unsigned int** ppiKey, 
        Variant*** pppvData, unsigned int* piNumRows);

    // Searches
    int GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey (unsigned int iKey, unsigned int* piNextKey);

    int GetFirstKey (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey);

    int GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int** ppiKey, 
        unsigned int* piNumKeys);

    int GetSearchKeys (unsigned int iNumColumns, const unsigned int* piColumn, const Variant* pvData, 
        const Variant* pvData2, unsigned int iStartKey, unsigned int iSkipHits, unsigned int iMaxNumHits, 
        unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);
};

#endif // !defined(AFX_TABLE_H__05370B5D_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)