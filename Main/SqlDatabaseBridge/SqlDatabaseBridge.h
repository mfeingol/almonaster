//
// SqlDatabase.dll - A database library
// Copyright(c) 1998 Max Attar Feingold(maf6@cornell.edu)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or(at your option) any later version.
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

#pragma once

#include <vcclr.h>
#include "Database.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database;

class SqlDatabaseBridge : public IDatabase
{
private:

    gcroot<SqlDatabase^> m_db;

public:
	SqlDatabaseBridge();
	~SqlDatabaseBridge();

	static SqlDatabaseBridge* CreateInstance();

    // IDatabase
    IMPLEMENT_INTERFACE(IDatabase);

	// Return OK if database was reloaded, WARNING if new database was created, something else if an error occurred
    int Initialize(const char* pszMainDirectory, unsigned int iOptions);

    // Template operations
    int CreateTemplate(const TemplateDescription& ttTemplate);
    int DeleteTemplate(const char* pszTemplateName);

    int GetTemplate(const char* pszTemplateName, ITemplate** ppTemplate);
    int GetTemplateForTable(const char* pszTableName, ITemplate** ppTemplate);

    bool DoesTemplateExist(const char* pszTemplateName);

    // Table operations
    int CreateTable(const char* pszTableName, const char* pszTemplateName);
    int ImportTable(IDatabase* pSrcDatabase, const char* pszTableName);
    int DeleteTable(const char* pszTableName);
    bool DoesTableExist(const char* pszTableName);

    // Utilities
    int Backup(IDatabaseBackupNotificationSink* pSink, bool bCheckFirst);
    unsigned int DeleteOldBackups(Seconds iNumSecondsOld);

    IDatabaseBackupEnumerator* GetBackupEnumerator();

    int RestoreBackup(IDatabaseBackup* pBackup);
    int DeleteBackup(IDatabaseBackup* pBackup);

    ////////////////
    // Accounting //
    ////////////////

    const char* GetDirectory();

    unsigned int GetNumTables();
    unsigned int GetNumTemplates();

    ITableEnumerator* GetTableEnumerator();
    ITemplateEnumerator* GetTemplateEnumerator();

    bool IsTemplateEqual(const char* pszTemplateName, const TemplateDescription& ttTemplate);
    
    int Flush();
    int Check();

    //////////////////
    // Database API //
    //////////////////

    // Standard operations
    int ReadData(const char* pszTableName, unsigned int iKey, unsigned int iColumn, Variant* pvData);
    int ReadData(const char* pszTableName, unsigned int iColumn, Variant* pvData);
    
    int WriteData(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vData);
    int WriteData(const char* pszTableName, unsigned int iColumn, const Variant& vData);
    
    int Increment(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement);
    int Increment(const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue);
    int Increment(const char* pszTableName, unsigned int iColumn, const Variant& vIncrement);
    int Increment(const char* pszTableName, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue);

    int WriteAnd(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteAnd(const char* pszTableName, unsigned int iColumn, unsigned int iBitField);

    int WriteOr(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteOr(const char* pszTableName, unsigned int iColumn, unsigned int iBitField);

    int WriteXor(const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteXor(const char* pszTableName, unsigned int iColumn, unsigned int iBitField);

    int WriteNot(const char* pszTableName, unsigned int iKey, unsigned int iColumn);
    int WriteNot(const char* pszTableName, unsigned int iColumn);

    int WriteColumn(const char* pszTableName, unsigned int iColumn, const Variant& vData);

    // Row operations
    unsigned int GetNumRows(const char* pszTableName, unsigned int* piNumRows);
    int DoesRowExist(const char* pszTableName, unsigned int iKey, bool* pbExists);

    int InsertRow(const char* pszTableName, const Variant* pvColVal, unsigned int* piKey = NULL);
    int InsertRow(const char* pszTableName, const Variant* pvColVal, unsigned int iKey);

    int InsertRows(const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows);
    int InsertDuplicateRows(const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows);
    
    int DeleteRow(const char* pszTableName, unsigned int iKey);
    int DeleteAllRows(const char* pszTableName);
    
    int ReadRow(const char* pszTableName, unsigned int iKey, Variant** ppvData);
    int ReadRow(const char* pszTableName, Variant** ppvData);

    // Column operations
    int ReadColumn(const char* pszTableName, unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);
    int ReadColumn(const char* pszTableName, unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns(const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                    unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);
    int ReadColumns(const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
                    Variant*** pppvData, unsigned int* piNumRows);

    // Searches
    int GetAllKeys(const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey(const char* pszTableName, unsigned int iKey, unsigned int* piNextKey);

    int GetFirstKey(const char* pszTableName, unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int* piKey);
    int GetEqualKeys(const char* pszTableName, unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetSearchKeys(const char* pszTableName, const SearchDefinition& sdSearch, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);

    int ReadColumnWhereEqual(const char* pszTableName, unsigned int iEqualColumn, const Variant& vData, 
                             bool bCaseInsensitive, unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumKeys);

    //
    // Direct API
    //

    int GetTableForReading(const char* pszTableName, IReadTable** ppTable);
    int GetTableForWriting(const char* pszTableName, IWriteTable** ppTable);

    //
    // Transactions
    //
    int CreateTransaction(ITransaction** ppTransaction);

    //
    void FreeData(void** ppData);
    
    void FreeData(Variant* pvData);
    void FreeData(Variant** ppvData);

    void FreeData(int* piData);
    void FreeData(unsigned int* puiData);
    void FreeData(float* ppfData);
    void FreeData(char** ppszData);
    void FreeData(int64* pi64Data);

    void FreeKeys(unsigned int* piKeys);

    // Stats
    int GetStatistics(DatabaseStatistics* pdsStats);
    unsigned int GetOptions();
};

