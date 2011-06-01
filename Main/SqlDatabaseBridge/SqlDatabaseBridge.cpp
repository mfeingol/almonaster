//
// SqlDatabaseBridge.dll - A database library
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

#include "Stdafx.h"
#include "SqlDatabaseBridge.h"

SqlDatabaseBridge::SqlDatabaseBridge()
{
    m_db->Foo();
}

SqlDatabaseBridge::~SqlDatabaseBridge()
{
}

SqlDatabaseBridge* SqlDatabaseBridge::CreateInstance()
{
    return new SqlDatabaseBridge();
}

int SqlDatabaseBridge::Initialize(const char* pszMainDirectory, unsigned int iOptions)
{
    return OK;
}

// Template operations
int SqlDatabaseBridge::CreateTemplate(const TemplateDescription& ttTemplate)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::DeleteTemplate(const char* pszTemplateName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::GetTemplate(const char* pszTemplateName, ITemplate** ppTemplate)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::GetTemplateForTable(const char* pszTableName, ITemplate** ppTemplate)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

bool SqlDatabaseBridge::DoesTemplateExist(const char* pszTemplateName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

bool SqlDatabaseBridge::IsTemplateEqual(const char* pszTemplateName, const TemplateDescription& ttTemplate)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

ITemplateEnumerator* SqlDatabaseBridge::GetTemplateEnumerator()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

// Table operations
int SqlDatabaseBridge::CreateTable(const char* pszTableName, const char* pszTemplateName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::ImportTable(IDatabase* pSrcDatabase, const char* pszTableName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::DeleteTable(const char* pszTableName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

bool SqlDatabaseBridge::DoesTableExist(const char* pszTableName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return false;
}

// Backup
int SqlDatabaseBridge::Backup(IDatabaseBackupNotificationSink* pSink, bool bCheckFirst)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

unsigned int SqlDatabaseBridge::DeleteOldBackups(Seconds iNumSecondsOld)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

IDatabaseBackupEnumerator* SqlDatabaseBridge::GetBackupEnumerator()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

int SqlDatabaseBridge::RestoreBackup(IDatabaseBackup* pBackup)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::DeleteBackup(IDatabaseBackup* pBackup)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

////////////////
// Accounting //
////////////////

const char* SqlDatabaseBridge::GetDirectory()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

unsigned int SqlDatabaseBridge::GetOptions()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

unsigned int SqlDatabaseBridge::GetNumTables()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

unsigned int SqlDatabaseBridge::GetNumTemplates()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

ITableEnumerator* SqlDatabaseBridge::GetTableEnumerator()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return NULL;
}

int SqlDatabaseBridge::Flush()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::Check()
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//////////////////
// Database API //
//////////////////

// Standard operations
int SqlDatabaseBridge::ReadData(const char* pszTableName, unsigned int iKey, unsigned int iColumn, Variant* pvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::ReadData(const char* pszTableName, unsigned int iColumn, Variant* pvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}
    
int SqlDatabaseBridge::WriteData(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteData(const char* pszTableName, unsigned int iColumn, const Variant& vData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::Increment(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::Increment(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::Increment(const char* pszTableName, unsigned int iColumn, const Variant& vIncrement)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::Increment(const char* pszTableName, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteAnd(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteAnd(const char* pszTableName, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteOr(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteOr(const char* pszTableName, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteXor(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteXor(const char* pszTableName, unsigned int iColumn, unsigned int iBitField)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteNot(const char* pszTableName, unsigned int iKey, unsigned int iColumn)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteNot(const char* pszTableName, unsigned int iColumn)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::WriteColumn(const char* pszTableName, unsigned int iColumn, const Variant& vData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

// Row operations
unsigned int SqlDatabaseBridge::GetNumRows(const char* pszTableName, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return 0;
}

int SqlDatabaseBridge::DoesRowExist(const char* pszTableName, unsigned int iKey, bool* pbExists)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::InsertRow(const char* pszTableName, const Variant* pvColVal, unsigned int* piKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::InsertRow(const char* pszTableName, const Variant* pvColVal, unsigned int iKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::InsertRows(const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::InsertDuplicateRows(const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::DeleteRow(const char* pszTableName, unsigned int iKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::DeleteAllRows(const char* pszTableName)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::ReadRow(const char* pszTableName, unsigned int iKey, Variant** ppvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::ReadRow(const char* pszTableName, Variant** ppvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

// Column operations
int SqlDatabaseBridge::ReadColumn(const char* pszTableName, unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::ReadColumn(const char* pszTableName, unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}


int SqlDatabaseBridge::ReadColumns(const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                              unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::ReadColumns(const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                              Variant*** pppvData, unsigned int* piNumRows)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

// Searches
int SqlDatabaseBridge::GetAllKeys(const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::GetNextKey(const char* pszTableName, unsigned int iKey, unsigned int* piNextKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::GetFirstKey(const char* pszTableName, unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::GetEqualKeys(const char* pszTableName, unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int** ppiKey, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::GetSearchKeys(const char* pszTableName, const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::ReadColumnWhereEqual(const char* pszTableName, unsigned int iEqualColumn, const Variant& vData, 
                                       bool bCaseInsensitive, unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//
// Direct API
//

int SqlDatabaseBridge::GetTableForReading(const char* pszTableName, IReadTable** ppTable)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

int SqlDatabaseBridge::GetTableForWriting(const char* pszTableName, IWriteTable** ppTable)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//
// Transactions
//
int SqlDatabaseBridge::CreateTransaction(ITransaction** ppTransaction)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}

//
void SqlDatabaseBridge::FreeData(void** ppData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}
   
void SqlDatabaseBridge::FreeData(Variant* pvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void SqlDatabaseBridge::FreeData(Variant** ppvData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void SqlDatabaseBridge::FreeData(int* piData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void SqlDatabaseBridge::FreeData(unsigned int* puiData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void SqlDatabaseBridge::FreeData(float* ppfData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void SqlDatabaseBridge::FreeData(char** ppszData)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void SqlDatabaseBridge::FreeData(int64* pi64Data)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

void SqlDatabaseBridge::FreeKeys(unsigned int* piKeys)
{
    // TODOTODO - Needs implementation
    Assert(false);
}

// Stats
int SqlDatabaseBridge::GetStatistics(DatabaseStatistics* pdsStats)
{
    // TODOTODO - Needs implementation
    Assert(false);
    return OK;
}
