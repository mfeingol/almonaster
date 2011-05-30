//
// OdbcDatabase.dll - A database library
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

#include "OdbcDatabase.h"

OdbcDatabase::OdbcDatabase()
{
}

OdbcDatabase::~OdbcDatabase()
{
}

OdbcDatabase* OdbcDatabase::CreateInstance()
{
    return new OdbcDatabase();
}

int OdbcDatabase::Initialize(const char* pszMainDirectory, unsigned int iOptions)
{
    return OK;
}

// Template operations
int OdbcDatabase::CreateTemplate(const TemplateDescription& ttTemplate)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::DeleteTemplate(const char* pszTemplateName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::GetTemplate(const char* pszTemplateName, ITemplate** ppTemplate)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::GetTemplateForTable(const char* pszTableName, ITemplate** ppTemplate)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

bool OdbcDatabase::DoesTemplateExist(const char* pszTemplateName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

bool OdbcDatabase::IsTemplateEqual(const char* pszTemplateName, const TemplateDescription& ttTemplate)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

ITemplateEnumerator* OdbcDatabase::GetTemplateEnumerator()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

// Table operations
int OdbcDatabase::CreateTable(const char* pszTableName, const char* pszTemplateName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::ImportTable(IDatabase* pSrcDatabase, const char* pszTableName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::DeleteTable(const char* pszTableName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

bool OdbcDatabase::DoesTableExist(const char* pszTableName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return false;
}

// Backup
int OdbcDatabase::Backup(IDatabaseBackupNotificationSink* pSink, bool bCheckFirst)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

unsigned int OdbcDatabase::DeleteOldBackups(Seconds iNumSecondsOld)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

IDatabaseBackupEnumerator* OdbcDatabase::GetBackupEnumerator()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

int OdbcDatabase::RestoreBackup(IDatabaseBackup* pBackup)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::DeleteBackup(IDatabaseBackup* pBackup)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

////////////////
// Accounting //
////////////////

const char* OdbcDatabase::GetDirectory()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

unsigned int OdbcDatabase::GetOptions()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

unsigned int OdbcDatabase::GetNumTables()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

unsigned int OdbcDatabase::GetNumTemplates()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

ITableEnumerator* OdbcDatabase::GetTableEnumerator()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

int OdbcDatabase::Flush()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::Check()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//////////////////
// Database API //
//////////////////

// Standard operations
int OdbcDatabase::ReadData(const char* pszTableName, unsigned int iKey, unsigned int iColumn, Variant* pvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::ReadData(const char* pszTableName, unsigned int iColumn, Variant* pvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}
    
int OdbcDatabase::WriteData(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteData(const char* pszTableName, unsigned int iColumn, const Variant& vData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::Increment(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::Increment(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::Increment(const char* pszTableName, unsigned int iColumn, const Variant& vIncrement)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::Increment(const char* pszTableName, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteAnd(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteAnd(const char* pszTableName, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteOr(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteOr(const char* pszTableName, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteXor(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteXor(const char* pszTableName, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteNot(const char* pszTableName, unsigned int iKey, unsigned int iColumn)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteNot(const char* pszTableName, unsigned int iColumn)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::WriteColumn(const char* pszTableName, unsigned int iColumn, const Variant& vData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

// Row operations
unsigned int OdbcDatabase::GetNumRows(const char* pszTableName, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

int OdbcDatabase::DoesRowExist(const char* pszTableName, unsigned int iKey, bool* pbExists)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::InsertRow(const char* pszTableName, const Variant* pvColVal, unsigned int* piKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::InsertRow(const char* pszTableName, const Variant* pvColVal, unsigned int iKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::InsertRows(const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::InsertDuplicateRows(const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::DeleteRow(const char* pszTableName, unsigned int iKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::DeleteAllRows(const char* pszTableName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::ReadRow(const char* pszTableName, unsigned int iKey, Variant** ppvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::ReadRow(const char* pszTableName, Variant** ppvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

// Column operations
int OdbcDatabase::ReadColumn(const char* pszTableName, unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::ReadColumn(const char* pszTableName, unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}


int OdbcDatabase::ReadColumns(const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                              unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::ReadColumns(const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                              Variant*** pppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

// Searches
int OdbcDatabase::GetAllKeys(const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::GetNextKey(const char* pszTableName, unsigned int iKey, unsigned int* piNextKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::GetFirstKey(const char* pszTableName, unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::GetEqualKeys(const char* pszTableName, unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::GetSearchKeys(const char* pszTableName, const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::ReadColumnWhereEqual(const char* pszTableName, unsigned int iEqualColumn, const Variant& vData, 
                                       bool bCaseInsensitive, unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//
// Direct API
//

int OdbcDatabase::GetTableForReading(const char* pszTableName, IReadTable** ppTable)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int OdbcDatabase::GetTableForWriting(const char* pszTableName, IWriteTable** ppTable)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//
// Transactions
//
int OdbcDatabase::CreateTransaction(ITransaction** ppTransaction)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//
void OdbcDatabase::FreeData(void** ppData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}
   
void OdbcDatabase::FreeData(Variant* pvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void OdbcDatabase::FreeData(Variant** ppvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void OdbcDatabase::FreeData(int* piData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void OdbcDatabase::FreeData(unsigned int* puiData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void OdbcDatabase::FreeData(float* ppfData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void OdbcDatabase::FreeData(char** ppszData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void OdbcDatabase::FreeData(int64* pi64Data)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void OdbcDatabase::FreeKeys(unsigned int* piKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

// Stats
int OdbcDatabase::GetStatistics(DatabaseStatistics* pdsStats)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}
