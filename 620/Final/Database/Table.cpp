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

#include "Table.h"

#include <stdio.h>

#include "Osal/File.h"
#include "Osal/Algorithm.h"

#define INITIAL_INSERTION_LENGTH_ASSUMPTION_SIZE (128)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Table::Table (Database* pDatabase) {

    m_iNumRefs = 1;
    m_bDelete = false;

    Assert (pDatabase != NULL);

    m_wTable.SetTable (this);

    m_pContext = m_wTable.GetContext();
    m_pContext->SetDatabase (pDatabase);
}

int Table::Initialize() {
    return m_rwLock.Initialize();
}

#ifdef _DEBUG

void Table::CheckIntegrity() {

    int iStaticNumRows = m_pContext->GetNumRows();
    int iForNumRows = 0;
    int iWhileNumRows = 0;
    int iTerminatorKey = m_pContext->GetTerminatorRowKey();

    for (int i = 0; i < iTerminatorKey; i ++) {
        if (m_pContext->IsValidRow (i)) {
            iForNumRows ++;
        }
    }

    unsigned int iRowKey = NO_KEY;
    while (true) {

        iRowKey = m_pContext->FindNextValidRow (iRowKey);
        if (iRowKey == NO_KEY) {
            break;
        }

        iWhileNumRows ++;
    }

    Assert (iForNumRows == iStaticNumRows);
    Assert (iWhileNumRows == iStaticNumRows);
    Assert (iForNumRows == iWhileNumRows);
}

#endif


Table::~Table() {

    if (m_bDelete) {
        m_pContext->DeleteOnDisk();
    }
}

int Table::Reload (Offset oTable) {

    int iErrCode = m_pContext->Reload (oTable);

#ifdef _DEBUG
    CheckIntegrity();
#endif

    return iErrCode;
}


int Table::Create (const char* pszTableName, Template* pTemplate) {

    return m_pContext->Create (pszTableName, pTemplate);
}

int Table::GetTemplate (ITemplate** ppTemplate) {
    
    return m_pContext->GetTemplate (ppTemplate);
}


int Table::GetTableForReading (IReadTable** ppTable) {

    *ppTable = m_wTable.GetReadTable();
    (*ppTable)->AddRef();

    return OK;
}

int Table::GetTableForWriting (IWriteTable** ppTable) {

    *ppTable = &m_wTable;
    m_wTable.AddRef();

    return OK;
}

int Table::ReadData (unsigned int iKey, unsigned int iColumn, Variant* pvData) {

    return m_wTable.ReadData (iKey, iColumn, pvData);
}

int Table::WriteData (unsigned int iKey, unsigned int iColumn, const Variant& vData) {

    return m_wTable.WriteData (iKey, iColumn, vData);
}

int Table::WriteAnd (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    return m_wTable.WriteAnd (iKey, iColumn, iBitField);
}

int Table::WriteOr (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    return m_wTable.WriteOr (iKey, iColumn, iBitField);
}

int Table::WriteXor (unsigned int iKey, unsigned int iColumn, unsigned int iBitField) {

    return m_wTable.WriteXor (iKey, iColumn, iBitField);
}

int Table::WriteNot (unsigned int iKey, unsigned int iColumn) {

    return m_wTable.WriteNot (iKey, iColumn);
}

int Table::WriteColumn (unsigned int iColumn, const Variant& vData) {

    return m_wTable.WriteColumn (iColumn, vData);
}

int Table::Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue) {

    return m_wTable.Increment (iKey, iColumn, vIncrement, pvOldValue);
}

int Table::GetNumRows (unsigned int* piNumRows) {
    
    return m_wTable.GetNumRows (piNumRows);
}

int Table::DoesRowExist (unsigned int iKey, bool* pbExists) {

    return m_wTable.DoesRowExist (iKey, pbExists);
}

int Table::InsertRow (const Variant* pvColVal, unsigned int* piKey) {

    return m_wTable.InsertRow (pvColVal, piKey);
}

int Table::InsertRow (const Variant* pvColVal, unsigned int iKey) {

    return m_wTable.InsertRow (pvColVal, iKey);
}

int Table::InsertDuplicateRows (const Variant* pvColVal, unsigned int iNumRows) {

    return m_wTable.InsertDuplicateRows (pvColVal, iNumRows);
}

int Table::InsertRows (const Variant* pvColVal, unsigned int iNumRows) {

    return m_wTable.InsertRows (pvColVal, iNumRows);
}

int Table::DeleteRow (unsigned int iKey) {

    return m_wTable.DeleteRow (iKey);
}

int Table::DeleteAllRows() {

    return m_wTable.DeleteAllRows();
}

int Table::GetAllKeys (unsigned int** ppiKey, unsigned int* piNumKeys) {

    return m_wTable.GetAllKeys (ppiKey, piNumKeys);
}

int Table::GetNextKey (unsigned int iKey, unsigned int* piNextKey) {

    return m_wTable.GetNextKey (iKey, piNextKey);
}


int Table::GetFirstKey (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey) {

    return m_wTable.GetFirstKey (iColumn, vData, bCaseInsensitive, piKey);
}

int Table::GetEqualKeys (unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, 
                         unsigned int** ppiKey, unsigned int* piNumKeys) {

    return m_wTable.GetEqualKeys (iColumn, vData, bCaseInsensitive, ppiKey, piNumKeys);
}

int Table::GetSearchKeys (const SearchDefinition& sdSearch, unsigned int** ppiKey, 
                          unsigned int* piNumHits, unsigned int* piStopKey) {

    return m_wTable.GetSearchKeys (sdSearch, ppiKey, piNumHits, piStopKey);
}

int Table::ReadColumnWhereEqual (unsigned int iEqualColumn, const Variant& vData, bool bCaseInsensitive, 
                                 unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, 
                                 unsigned int* piNumKeys) {

    return m_wTable.ReadColumnWhereEqual (iEqualColumn, vData, bCaseInsensitive, iReadColumn, ppiKey, ppvData,
        piNumKeys);
}

int Table::ReadRow (unsigned int iKey, Variant** ppvData) {

    return m_wTable.ReadRow (iKey, ppvData);
}

int Table::ReadColumn (unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows) {

    return m_wTable.ReadColumn (iColumn, ppiKey, ppvData, piNumRows);
}

int Table::ReadColumns (unsigned int iNumColumns, const unsigned int* piColumn, unsigned int** ppiKey, 
                        Variant*** pppvData, unsigned int* piNumRows) {

    return m_wTable.ReadColumns (iNumColumns, piColumn, ppiKey, pppvData, piNumRows);
}