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

#include "TableContext.h"
#include "Template.h"
#include "WriteTable.h"

class WriteTable;

class Table : public IObject {
protected:

    bool m_bDelete;

    TableContext* m_pContext;

    ReadWriteLock m_rwLock;
    WriteTable m_wTable;

public:

    Table (Database* pDatabase);
    ~Table();

    int Initialize();

#ifdef _DEBUG
    void CheckIntegrity();
#endif

    inline const char* GetName() {
        return m_pContext->GetName();
    }

    inline bool IsOneRow() {
        return m_pContext->IsOneRow();
    }

    IMPLEMENT_IOBJECT;

    int Reload (Offset oTable);
    int Create (const char* pszTableName, Template* pTemplate);

    inline void DeleteOnDisk() {
        m_bDelete = true;
    }

    inline void WaitReader() {
        m_rwLock.WaitReader();
        m_pContext->Lock();
    }

    inline void SignalReader() {
        m_pContext->Unlock();
        m_rwLock.SignalReader();
    }

    inline void WaitWriter() {
        m_rwLock.WaitWriter();
        m_pContext->Lock();
    }

    inline void SignalWriter() {
        m_pContext->Unlock();
        m_rwLock.SignalWriter();
    }

    inline TableContext* GetContext() {
        return m_pContext;
    }

    int GetTemplate (ITemplate** ppTemplate);

    //
    // Direct API
    //

    int GetTableForReading (IReadTable** ppTable);
    int GetTableForWriting (IWriteTable** ppTable);

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

    int InsertRow (const Variant* pvColVal, unsigned int* piKey);
    int InsertRows (const Variant* pvColVal, unsigned int iNumCols);
    int InsertDuplicateRows (const Variant* pvColVal, unsigned int iNumRows);

    int DeleteRow (unsigned int iKey);
    int DeleteAllRows();
    
    int ReadRow (unsigned int iKey, Variant** ppvData);

    // Column operations
    int ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);
    int ReadColumns (unsigned int iNumColumns, const unsigned int* piColumn, unsigned int** ppiKey, Variant*** pppvData, 
        unsigned int* piNumRows);

    // Searches
    int GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey (unsigned int iKey, unsigned int* piNextKey);

    int GetFirstKey (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey);

    int GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int** ppiKey, 
        unsigned int* piNumKeys);

    int GetSearchKeys (unsigned int iNumColumns, const unsigned int* piColumn, const unsigned int* piFlags,
        const Variant* pvData, const Variant* pvData2, unsigned int iStartKey, unsigned int iSkipHits, 
        unsigned int iMaxNumHits, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);

    int ReadColumnWhereEqual (unsigned int iEqualColumn, const Variant& vData, bool bCaseInsensitive, 
        unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys);
};

#endif // !defined(AFX_TABLE_H__05370B5D_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)