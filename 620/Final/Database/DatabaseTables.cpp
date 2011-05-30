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

#include "CDatabase.h"
#include "Table.h"

#include "Osal/Algorithm.h"

int Database::CreateTable (const char* pszTableName, const char* pszTemplateName) {

    int iErrCode = OK;
    bool bFlag;

    Table* pOldTable, * pTable = NULL;
    Template* pTemplate;

    m_rwGlobalLock.WaitReader();

    // Look up template
    pTemplate = FindTemplate (pszTemplateName); // AddRef() on pTemplate
    if (pTemplate == NULL) {
        iErrCode = ERROR_UNKNOWN_TEMPLATE_NAME;
        goto Cleanup;
    }

    // Create the table
    pTable = new Table (this);
    if (pTable == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    iErrCode = pTable->Initialize();
    if (iErrCode != OK) {
        delete pTable;
        goto Cleanup;
    }

    iErrCode = pTable->Create (pszTableName, pTemplate);    // AddRef() on pTemplate
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Try to add table
    m_rwTableLock.WaitWriter();
    
    bFlag = m_pTables->FindFirst (pTable->GetName(), &pOldTable);
    if (bFlag) {
        m_rwTableLock.SignalWriter();
        iErrCode = ERROR_TABLE_ALREADY_EXISTS;
        goto Cleanup;
    }

    bFlag = m_pTables->Insert (pTable->GetName(), pTable);
    m_rwTableLock.SignalWriter();

    if (!bFlag) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

Cleanup:

    if (iErrCode != OK && pTable != NULL) {

        pTable->DeleteOnDisk();
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    if (pTemplate != NULL) {
        pTemplate->Release();
    }

    return iErrCode;
}


int Database::ImportTable (IDatabase* pSrcDatabase, const char* pszTableName) {

    int iErrCode;
    unsigned int i, j, iNumRows, * piRowKey = NULL, iKey, * piDelRows, iNumDelRows = 0, iMinNumCols;

    ITemplate* pSrcTemplate = NULL, * pDestTemplate = NULL;
    TemplateDescription ttSrcTemplate, ttDestTemplate;

    IReadTable* pReadTable = NULL;
    IWriteTable* pWriteTable = NULL;

    Variant* pvDestRow = NULL, * pvSrcRow = NULL;

    m_rwGlobalLock.WaitReader();

#ifdef _DEBUG
    IReadTable* pDestTable = NULL;
    Variant vTest, * pvTestDestRow = NULL;
#endif

    // Get the templates from the src database
    iErrCode = pSrcDatabase->GetTemplateForTable (pszTableName, &pSrcTemplate);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get the src template description
    iErrCode = pSrcTemplate->GetDescription (&ttSrcTemplate);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Look for a similar template in our data
    iErrCode = GetTemplate (ttSrcTemplate.Name, &pDestTemplate);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Get our template's description
    iErrCode = pDestTemplate->GetDescription (&ttDestTemplate);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iMinNumCols = min (ttSrcTemplate.NumColumns, ttDestTemplate.NumColumns);

    // If templates are incompatible, that'll be caught during the data phase
    
    // Try to create the table in our database
    iErrCode = CreateTable (pszTableName, ttDestTemplate.Name);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Create an array of variants for null insertions into the dest table
    pvDestRow = new Variant [ttDestTemplate.NumColumns];
    if (pvDestRow == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    // Initialize the empty data
    for (i = 0; i < ttDestTemplate.NumColumns; i ++) {
        
        iErrCode = InitializeBlankData (pvDestRow + i, ttDestTemplate.Type[i]);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Read all row keys from the src table
    iErrCode = pSrcDatabase->GetAllKeys (pszTableName, &piRowKey, &iNumRows);
    if (iErrCode != OK || iNumRows == 0) {

        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert (false);
        goto Cleanup;
    }

    // Get the tables we need for reading and writing
    iErrCode = pSrcDatabase->GetTableForReading (pszTableName, &pReadTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = GetTableForWriting (pszTableName, &pWriteTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Allocate space for empty rows inserted that should be deleted later
    piDelRows = (unsigned int*) StackAlloc (piRowKey[iNumRows - 1] * sizeof (unsigned int));

    for (i = 0; i < iNumRows; i ++) {

        // Read data from the src row
        iErrCode = pReadTable->ReadRow (piRowKey[i], &pvSrcRow);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Type coerce data, if necessary
        for (j = 0; j < iMinNumCols; j ++) {
            
            if (ttDestTemplate.Type[j] == pvSrcRow[j].GetType()) {

                pvDestRow[j] = pvSrcRow[j];

            } else {
                
                switch (ttDestTemplate.Type[j]) {
                    
                case V_INT:
                    
                    pvDestRow[j] = (int) pvSrcRow[j];
                    break;
                    
                case V_FLOAT:
                    
                    pvDestRow[j] = (float) pvSrcRow[j];
                    break;
                    
                case V_TIME:
                    
                    pvDestRow[j] = (UTCTime) pvSrcRow[j];
                    break;
                    
                case V_STRING:
                    
                    // Let's not even try this
                    Assert (!"Unsupported conversion");
                    iErrCode = ERROR_FAILURE;
                    goto Cleanup;
                    break;

                case V_INT64:

                    pvDestRow[j] = (int64) pvSrcRow[j];
                    break;
                    
                default:
                    
                    Assert (false);
                    iErrCode = ERROR_INVALID_TYPE;
                    goto Cleanup;
                }
            }
        }   // End for

        while (true) {

            // Insert the row
            iErrCode = pWriteTable->InsertRow (pvDestRow, &iKey);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            if (iKey > piRowKey[i]) {
                Assert (false);
                iErrCode = ERROR_DATA_CORRUPTION;
                goto Cleanup;
            }
            
            else if (iKey < piRowKey[i]) {
                Assert (iNumDelRows < piRowKey[iNumRows - 1]);
                piDelRows[iNumDelRows ++] = iKey;
            }

            else {

                // We inserted the row with the right key, go on to the next one
                break;
            }
        }

        pSrcDatabase->FreeData (pvSrcRow);
        pvSrcRow = NULL;
    }

    // Delete rows that weren't in the original table
    for (i = 0; i < iNumDelRows; i ++) {

        iErrCode = pWriteTable->DeleteRow (piDelRows[i]);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

#ifdef _DEBUG

    // Verify data
    iErrCode = pWriteTable->QueryInterface (IID_IReadTable, (void**) &pDestTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    for (i = 0; i < iNumRows; i ++) {

        iErrCode = pReadTable->ReadRow (piRowKey[i], &pvSrcRow);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = pDestTable->ReadRow (piRowKey[i], &pvTestDestRow);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        for (j = 0; j < iMinNumCols; j ++) {

            if (ttDestTemplate.Type[j] == pvSrcRow[j].GetType()) {

                if (pvTestDestRow[j] != pvSrcRow[j]) {
                    Assert (false);
                    iErrCode = ERROR_DATA_CORRUPTION;
                    goto Cleanup;
                }

            } else {
                
                switch (ttDestTemplate.Type[j]) {
                    
                case V_INT:
                    
                    vTest = (int) pvSrcRow[j];
                    break;
                    
                case V_FLOAT:
                    
                    vTest = (float) pvSrcRow[j];
                    break;
                    
                case V_TIME:
                    
                    vTest = (UTCTime) pvSrcRow[j];
                    break;
                    
                case V_STRING:
                    
                    // Let's not even try this
                    Assert (!"Unsupported conversion");
                    iErrCode = ERROR_FAILURE;
                    goto Cleanup;
                    break;

                case V_INT64:

                    pvDestRow[j] = (int64) pvSrcRow[j];
                    break;
                    
                default:
                    
                    Assert (false);
                    iErrCode = ERROR_INVALID_TYPE;
                    goto Cleanup;
                }

                if (pvTestDestRow[j] != vTest) {
                    Assert (false);
                    iErrCode = ERROR_DATA_CORRUPTION;
                    goto Cleanup;
                }
            }
        }   // End for each column in common

        FreeData (pvTestDestRow);
        pvTestDestRow = NULL;

        pSrcDatabase->FreeData (pvSrcRow);
        pvSrcRow = NULL;

    }   // End for each row

#endif

Cleanup:

#ifdef _DEBUG
    if (pDestTable != NULL) {
        pDestTable->Release();
    }

    if (pvTestDestRow != NULL) {
        FreeData (pvTestDestRow);
    }
#endif

    if (pSrcTemplate != NULL) {
        pSrcTemplate->Release();
    }

    if (pDestTemplate != NULL) {
        pDestTemplate->Release();
    }

    if (pReadTable != NULL) {
        pReadTable->Release();
    }

    if (pWriteTable != NULL) {
        pWriteTable->Release();
    }

    if (piRowKey != NULL) {
        pSrcDatabase->FreeKeys (piRowKey);
    }

    if (pvDestRow != NULL) {
        delete [] pvDestRow;
    }

    if (pvSrcRow != NULL) {
        pSrcDatabase->FreeData (pvSrcRow);
    }   

    if (iErrCode != OK) {
        // Best effort delete
        DeleteTable (pszTableName);
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::DeleteTable (const char* pszTableName) {

    int iErrCode = OK;

    const char* pszKey;
    Table* pTable;

    m_rwGlobalLock.WaitReader();

    // Lock table
    m_rwTableLock.WaitWriter();
    bool bDeleted = m_pTables->DeleteFirst (pszTableName, &pszKey, &pTable);
    m_rwTableLock.SignalWriter();

    if (bDeleted) {

        // Clean up
        pTable->DeleteOnDisk();
        pTable->Release();

    } else {
        
        Assert (false);
        iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::GetTemplateForTable (const char* pszTableName, ITemplate** ppTemplate) {

    int iErrCode = ERROR_UNKNOWN_TABLE_NAME;
    *ppTemplate = NULL;

    m_rwGlobalLock.WaitReader();

    Table* pTable = FindTable (pszTableName);
    if (pTable != NULL) {
        iErrCode = pTable->GetTemplate (ppTemplate);
        pTable->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

bool Database::DoesTableExist (const char* pszTableName) {

    Table* pTable;

    m_rwGlobalLock.WaitReader();
    m_rwTableLock.WaitReader();

    bool bTableExists = m_pTables->FindFirst (pszTableName, &pTable);
    
    m_rwTableLock.SignalReader();
    m_rwGlobalLock.SignalReader();

    return bTableExists;
}


Table* Database::FindTable (const char* pszTableName) {

    Table* pTable = NULL;

    m_rwTableLock.WaitReader();
    if (m_pTables->FindFirst (pszTableName, &pTable)) {
        pTable->AddRef();
    }
    m_rwTableLock.SignalReader();   
    
    return pTable;
}


int Database::InitializeBlankData (Variant* pvVariant, VariantType vtType) {

    switch (vtType) {

    case V_INT:
        
        *pvVariant = 0;
        break;
        
    case V_FLOAT:
        
        *pvVariant = (float) 0.0;
        break;
        
    case V_TIME:
        
        *pvVariant = (UTCTime) 0;
        break;
        
    case V_STRING:

        *pvVariant = "";
        break;

    case V_INT64:

        *pvVariant = (int64) 0;
        break;
        
    default:

        return ERROR_INVALID_TYPE;
    }

    return OK;
}