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

#define DATABASE_BUILD
#include "CDatabase.h"
#include "Table.h"
#undef DATABASE_BUILD

#include "Osal/Algorithm.h"

int Database::ReadData (const char* pszTableName, unsigned int iKey, unsigned int iColumn, Variant* pvData) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->ReadData (iKey, iColumn, pvData);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}


int Database::ReadData (const char* pszTableName, unsigned int iColumn, Variant* pvData) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->ReadData (0, iColumn, pvData);
    pTable->SignalReader();
    Assert (pTable->OneRow());
    pTable->Release();

    return iErrCode;
}

int Database::WriteData (const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vData) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteData (iKey, iColumn, vData);
    pTable->SignalWriter();
    pTable->Release();

    return iErrCode;
}


int Database::WriteData (const char* pszTableName, unsigned int iColumn, const Variant& vData) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteData (0, iColumn, vData);
    pTable->SignalWriter();
    Assert (pTable->OneRow());
    pTable->Release();

    return iErrCode;
}


int Database::Increment (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                         const Variant& vIncrement, Variant* pvOldValue) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->Increment (iKey, iColumn, vIncrement, pvOldValue);
    pTable->SignalWriter();
    pTable->Release();

    return iErrCode;
}


int Database::Increment (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                         const Variant& vIncrement) {

    return Increment (pszTableName, iKey, iColumn, vIncrement, NULL);
}


int Database::Increment (const char* pszTableName, unsigned int iColumn, const Variant& vIncrement) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->Increment (0, iColumn, vIncrement, NULL);
    pTable->SignalWriter();
    Assert (pTable->OneRow());
    pTable->Release();

    return iErrCode;
}


int Database::Increment (const char* pszTableName, unsigned int iColumn, const Variant& vIncrement, 
                         Variant* pvOldValue) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->Increment (0, iColumn, vIncrement, pvOldValue);
    pTable->SignalWriter();
    Assert (pTable->OneRow());
    pTable->Release();

    return iErrCode;
}

int Database::WriteAnd (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                        unsigned int iBitField) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteAnd (iKey, iColumn, iBitField);
    pTable->SignalWriter();
    pTable->Release();

    return iErrCode;
}

int Database::WriteAnd (const char* pszTableName, unsigned int iColumn, unsigned int iBitField) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteAnd (0, iColumn, iBitField);
    pTable->SignalWriter();
    Assert (pTable->OneRow());
    pTable->Release();

    return iErrCode;
}

int Database::WriteOr (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                       unsigned int iBitField) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteOr (iKey, iColumn, iBitField);
    pTable->SignalWriter();
    pTable->Release();

    return iErrCode;
}

int Database::WriteOr (const char* pszTableName, unsigned int iColumn, unsigned int iBitField) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteOr (0, iColumn, iBitField);
    pTable->SignalWriter();
    Assert (pTable->OneRow());
    pTable->Release();

    return iErrCode;
}


int Database::WriteXor (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                        unsigned int iBitField) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteXor (iKey, iColumn, iBitField);
    pTable->SignalWriter();
    pTable->Release();

    return iErrCode;
}


int Database::WriteXor (const char* pszTableName, unsigned int iColumn, unsigned int iBitField) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteXor (0, iColumn, iBitField);
    pTable->SignalWriter();
    Assert (pTable->OneRow());
    pTable->Release();

    return iErrCode;
}

int Database::WriteNot (const char* pszTableName, unsigned int iKey, unsigned int iColumn) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteNot (iKey, iColumn);
    pTable->SignalWriter();
    pTable->Release();

    return iErrCode;
}

int Database::WriteNot (const char* pszTableName, unsigned int iColumn) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteNot (0, iColumn);
    pTable->SignalWriter();
    Assert (pTable->OneRow());
    pTable->Release();

    return iErrCode;
}

int Database::WriteColumn (const char* pszTableName, unsigned int iColumn, const Variant& vData) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->WriteColumn (iColumn, vData);
    pTable->SignalWriter();
    pTable->Release();

    return iErrCode;
}

int Database::GetNumRows (const char* pszTableName, unsigned int* piNumRows) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *piNumRows = 0;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->GetNumRows (piNumRows);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}

int Database::DoesRowExist (const char* pszTableName, unsigned int iKey, bool* pbExists) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *pbExists = false;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->DoesRowExist (iKey, pbExists);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}

int Database::InsertRow (const char* pszTableName, Variant* pvColVal, unsigned int* piKey) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *piKey = NO_KEY;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->InsertRow (pvColVal, piKey);
    pTable->SignalWriter();
    pTable->Release();

    if (iErrCode == OK) {
        Algorithm::AtomicIncrement (&m_iNumLoadedRows);
    }
    return iErrCode;
}


int Database::InsertRow (const char* pszTableName, Variant* pvColVal) {

    unsigned int iKey;
    return InsertRow (pszTableName, pvColVal, &iKey);
}


int Database::InsertRows (const char* pszTableName, Variant* pvColVal, unsigned int iNumRows) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    int iErrCode;
    
    pTable->WaitWriter();
    iErrCode = pTable->InsertRows (pvColVal, iNumRows);
    pTable->SignalWriter();
    pTable->Release();

    if (iErrCode == OK) {
        Algorithm::AtomicIncrement (&m_iNumLoadedRows, iNumRows);
    }
    return iErrCode;
}

int Database::InsertDuplicateRows (const char* pszTableName, Variant* pvColVal, unsigned int iNumRows) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    int iErrCode;

    pTable->WaitWriter();
    iErrCode = pTable->InsertDuplicateRows (pvColVal, iNumRows);
    pTable->SignalWriter();
    pTable->Release();

    if (iErrCode == OK) {
        Algorithm::AtomicIncrement (&m_iNumLoadedRows, iNumRows);
    }
    return iErrCode;
}

int Database::DeleteRow (const char* pszTableName, unsigned int iKey) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    int iErrCode = pTable->DeleteRow (iKey);
    pTable->SignalWriter();
    pTable->Release();

    if (iErrCode == OK) {
        Algorithm::AtomicDecrement (&m_iNumLoadedRows);
    }
    return iErrCode;
}

int Database::DeleteAllRows (const char* pszTableName) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    unsigned int iNumRows;

    pTable->WaitWriter();
    int iErrCode = pTable->GetNumRows (&iNumRows);
    if (iErrCode == OK) {
        iErrCode = pTable->DeleteAllRows();
    }
    pTable->SignalWriter();
    pTable->Release();

    if (iErrCode == OK) {
        Algorithm::AtomicDecrement (&m_iNumLoadedRows, iNumRows);
    }
    return iErrCode;
}

int Database::ReadRow (const char* pszTableName, unsigned int iKey, Variant** ppvData) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *ppvData = NULL;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->ReadRow (iKey, ppvData);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}

int Database::ReadRow (const char* pszTableName, Variant** ppvData) {

    return ReadRow (pszTableName, 0, ppvData);
}


// Column operations

int Database::ReadColumn (const char* pszTableName, unsigned int iColumn, unsigned int** ppiKey, 
                          Variant** ppvData, unsigned int* piNumRows) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *ppvData = NULL;
        *piNumRows = 0;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->ReadColumn (iColumn, ppiKey, ppvData, piNumRows);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}

int Database::ReadColumn (const char* pszTableName, unsigned int iColumn, Variant** pppvData, 
                          unsigned int* piNumRows) {
    
    return ReadColumn (pszTableName, iColumn, NULL, pppvData, piNumRows);
}

int Database::ReadColumns (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                           unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        if (ppiKey != NULL) {
            *ppiKey = NULL;
        }
        *pppvData = NULL;
        *piNumRows = 0;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->ReadColumns (iNumColumns, piColumn, ppiKey, pppvData, piNumRows);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}

int Database::ReadColumns (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                           Variant*** pppvData, unsigned int* piNumRows) {

    return ReadColumns (pszTableName, iNumColumns, piColumn, NULL, pppvData, piNumRows);
}


// Searches

int Database::GetAllKeys (const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *ppiKey = NULL;
        *piNumKeys = 0;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->GetAllKeys (ppiKey, piNumKeys);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}

int Database::GetNextKey (const char* pszTableName, unsigned int iKey, unsigned int* piNextKey) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *piNextKey = NO_KEY;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->GetNextKey (iKey, piNextKey);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}

int Database::GetFirstKey (const char* pszTableName, unsigned int iColumn, const Variant& vData, 
                           bool bCaseInsensitive, unsigned int* piKey) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *piKey = NO_KEY;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->GetFirstKey (iColumn, vData, bCaseInsensitive, piKey);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}


int Database::GetEqualKeys (const char* pszTableName, unsigned int iColumn, const Variant& vData, 
                            bool bCaseInsensitive, unsigned int** ppiKey, unsigned int* piNumKeys) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        if (ppiKey != NULL) {
            *ppiKey = NULL;
        }
        *piNumKeys = 0;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    int iErrCode = pTable->GetEqualKeys (iColumn, vData, bCaseInsensitive, ppiKey, piNumKeys);
    pTable->SignalReader();
    pTable->Release();

    return iErrCode;
}

int Database::GetSearchKeys (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                             const Variant* pvData, const Variant* pvData2, unsigned int iStartKey, 
                             unsigned int iSkipHits, unsigned int iMaxNumHits, unsigned int** ppiKey, 
                             unsigned int* piNumHits, unsigned int* piStopKey) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *ppiKey = NULL;
        *piNumHits = 0;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    
    int iErrCode = pTable->GetSearchKeys (
        iNumColumns, 
        piColumn, 
        pvData, 
        pvData2, 
        iStartKey,
        iSkipHits,
        iMaxNumHits, 
        ppiKey,
        piNumHits,
        piStopKey
        );

    pTable->SignalReader();

    pTable->Release();

    return iErrCode;
}