//
// Database.dll - A database library
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

#include "CDatabase.h"
#include "Table.h"
#include "Transaction.h"

#include "Osal/Algorithm.h"

int Database::ReadData (const char* pszTableName, unsigned int iKey, unsigned int iColumn, Variant* pvData) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->ReadData (iKey, iColumn, pvData);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::ReadData (const char* pszTableName, unsigned int iColumn, Variant* pvData) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->ReadData (0, iColumn, pvData);
        pTable->SignalReader();
        Assert (pTable->IsOneRow());
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::WriteData (const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vData) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteData (iKey, iColumn, vData);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::WriteData (const char* pszTableName, unsigned int iColumn, const Variant& vData) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteData (0, iColumn, vData);
        pTable->SignalWriter();
        Assert (pTable->IsOneRow());
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::Increment (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                         const Variant& vIncrement, Variant* pvOldValue) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->Increment (iKey, iColumn, vIncrement, pvOldValue);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::Increment (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                         const Variant& vIncrement) {

    return Increment (pszTableName, iKey, iColumn, vIncrement, NULL);
}


int Database::Increment (const char* pszTableName, unsigned int iColumn, const Variant& vIncrement) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {
        
        pTable->WaitWriter();
        iErrCode = pTable->Increment (0, iColumn, vIncrement, NULL);
        pTable->SignalWriter();
        Assert (pTable->IsOneRow());
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::Increment (const char* pszTableName, unsigned int iColumn, const Variant& vIncrement, 
                         Variant* pvOldValue) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->Increment (0, iColumn, vIncrement, pvOldValue);
        pTable->SignalWriter();
        Assert (pTable->IsOneRow());
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::WriteAnd (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                        unsigned int iBitField) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteAnd (iKey, iColumn, iBitField);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::WriteAnd (const char* pszTableName, unsigned int iColumn, unsigned int iBitField) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteAnd (0, iColumn, iBitField);
        pTable->SignalWriter();
        Assert (pTable->IsOneRow());
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::WriteOr (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                       unsigned int iBitField) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteOr (iKey, iColumn, iBitField);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::WriteOr (const char* pszTableName, unsigned int iColumn, unsigned int iBitField) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteOr (0, iColumn, iBitField);
        pTable->SignalWriter();
        Assert (pTable->IsOneRow());
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::WriteXor (const char* pszTableName, unsigned int iKey, unsigned int iColumn, 
                        unsigned int iBitField) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    
    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {
        
        pTable->WaitWriter();
        iErrCode = pTable->WriteXor (iKey, iColumn, iBitField);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();
        
    return iErrCode;
}


int Database::WriteXor (const char* pszTableName, unsigned int iColumn, unsigned int iBitField) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    
    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteXor (0, iColumn, iBitField);
        pTable->SignalWriter();
        Assert (pTable->IsOneRow());
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::WriteNot (const char* pszTableName, unsigned int iKey, unsigned int iColumn) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    
    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteNot (iKey, iColumn);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::WriteColumn (const char* pszTableName, unsigned int iColumn, const Variant& vData) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    
    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteColumn (iColumn, vData);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::WriteNot (const char* pszTableName, unsigned int iColumn) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    
    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->WriteNot (0, iColumn);
        pTable->SignalWriter();
        Assert (pTable->IsOneRow());
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


unsigned int Database::GetNumRows (const char* pszTableName, unsigned int* piNumRows) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *piNumRows = 0;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->GetNumRows (piNumRows);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::DoesRowExist (const char* pszTableName, unsigned int iKey, bool* pbExists) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *pbExists = false;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->DoesRowExist (iKey, pbExists);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::InsertRow (const char* pszTableName, const Variant* pvColVal, unsigned int* piKey) {
    
    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    if (piKey != NULL) {
        *piKey = NO_KEY;
    }

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();

        iErrCode = pTable->InsertRow (pvColVal, piKey);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::InsertRow (const char* pszTableName, const Variant* pvColVal, unsigned int iKey) {
    
    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();

        iErrCode = pTable->InsertRow (pvColVal, iKey);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::InsertRows (const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {
    
        pTable->WaitWriter();
        iErrCode = pTable->InsertRows (pvColVal, iNumRows);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::InsertDuplicateRows (const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->InsertDuplicateRows (pvColVal, iNumRows);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::DeleteRow (const char* pszTableName, unsigned int iKey) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->DeleteRow (iKey);
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::DeleteAllRows (const char* pszTableName) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitWriter();
        iErrCode = pTable->DeleteAllRows();
        pTable->SignalWriter();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::ReadRow (const char* pszTableName, unsigned int iKey, Variant** ppvData) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *ppvData = NULL;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->ReadRow (iKey, ppvData);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::ReadRow (const char* pszTableName, Variant** ppvData) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *ppvData = NULL;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        Assert (pTable->IsOneRow());

        pTable->WaitReader();
        iErrCode = pTable->ReadRow (0, ppvData);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


// Column operations

int Database::ReadColumn (const char* pszTableName, unsigned int iColumn, unsigned int** ppiKey, 
                          Variant** ppvData, unsigned int* piNumRows) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *ppvData = NULL;
    *piNumRows = 0;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->ReadColumn (iColumn, ppiKey, ppvData, piNumRows);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::ReadColumn (const char* pszTableName, unsigned int iColumn, Variant** pppvData, 
                          unsigned int* piNumRows) {
    
    return ReadColumn (pszTableName, iColumn, NULL, pppvData, piNumRows);
}

int Database::ReadColumns (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                           unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *pppvData = NULL;
    *piNumRows = 0;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->ReadColumns (iNumColumns, piColumn, ppiKey, pppvData, piNumRows);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::ReadColumns (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                           Variant*** pppvData, unsigned int* piNumRows) {

    return ReadColumns (pszTableName, iNumColumns, piColumn, NULL, pppvData, piNumRows);
}


// Searches

int Database::GetAllKeys (const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *ppiKey = NULL;
    *piNumKeys = 0;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->GetAllKeys (ppiKey, piNumKeys);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::GetNextKey (const char* pszTableName, unsigned int iKey, unsigned int* piNextKey) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *piNextKey = NO_KEY;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->GetNextKey (iKey, piNextKey);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::GetFirstKey (const char* pszTableName, unsigned int iColumn, const Variant& vData, 
                           bool bCaseInsensitive, unsigned int* piKey) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *piKey = NO_KEY;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->GetFirstKey (iColumn, vData, bCaseInsensitive, piKey);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::GetEqualKeys (const char* pszTableName, unsigned int iColumn, const Variant& vData, 
                            bool bCaseInsensitive, unsigned int** ppiKey, unsigned int* piNumKeys) {
    
    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *piNumKeys = 0;
    
    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->GetEqualKeys (iColumn, vData, bCaseInsensitive, ppiKey, piNumKeys);
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::GetSearchKeys (const char* pszTableName, const SearchDefinition& sdSearch, 
                             unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;

    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *piNumHits = 0;

    if (piStopKey != NULL) {
        *piStopKey = NO_KEY;
    }

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->GetSearchKeys (sdSearch, ppiKey, piNumHits, piStopKey);
        
        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::ReadColumnWhereEqual (const char* pszTableName, unsigned int iEqualColumn, const Variant& vData, 
                                    bool bCaseInsensitive, unsigned int iReadColumn, unsigned int** ppiKey, 
                                    Variant** ppvData, unsigned int* piNumKeys) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    if (ppiKey != NULL) {
        *ppiKey = NULL;
    }
    *ppvData = NULL;
    *piNumKeys = 0;

    m_rwGlobalLock.WaitReader();
    
    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {

        pTable->WaitReader();
        iErrCode = pTable->ReadColumnWhereEqual (iEqualColumn, vData, bCaseInsensitive, iReadColumn, ppiKey, 
            ppvData, piNumKeys);

        pTable->SignalReader();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


//
// Direct APIs
//

int Database::GetTableForReading (const char* pszTableName, IReadTable** ppTable) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *ppTable = NULL;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->GetTableForReading (ppTable);
    pTable->Release();

    return OK;
}

int Database::GetTableForWriting (const char* pszTableName, IWriteTable** ppTable) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *ppTable = NULL;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->GetTableForWriting (ppTable);
    pTable->Release();

    return OK;
}

//
// Transactions
//

int Database::CreateTransaction (ITransaction** ppTransaction) {

    /*
    *ppTransaction = Transaction::CreateInstance (this);
    if (*ppTransaction == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    */

    *ppTransaction = NULL;
    return ERROR_NOT_IMPLEMENTED;

}