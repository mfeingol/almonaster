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

#if !defined(AFX_DATABASE_H__05370B54_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)
#define AFX_DATABASE_H__05370B54_2751_11D3_A0DC_0050047FE2E2__INCLUDED_

#include <stdio.h>

#include "Osal/HashTable.h"
#include "Osal/File.h"
#include "Osal/Algorithm.h"
#include "Osal/ReadWriteLock.h"

#include "Database.h"
#include "DatabaseStrings.h"

class TemplateNameHashValue {
public:
    static unsigned int GetHashValue (const char* pszString, unsigned int iNumBuckets, const void* pvHashHint);
};

class TemplateNameEquals {
public:
    
    static bool Equals (const char* pszLeft, const char* pszRight, const void* pvEqualsHint);
};

typedef TemplateNameHashValue TableNameHashValue;
typedef TemplateNameEquals TableNameEquals;

class Template;
class Table;

// Enumerators
class TableEnumerator : public ITableEnumerator {
private:

    TableEnumerator (HashTable<const char*, Table*, TableNameHashValue, TableNameEquals>* pTables);
    ~TableEnumerator();

    char** m_ppszTableNames;
    unsigned int m_iNumTables;

public:

    static TableEnumerator* CreateInstance (HashTable<const char*, Table*, TableNameHashValue, TableNameEquals>* pTables);

    // IDatabaseBackup
    IMPLEMENT_INTERFACE (ITableEnumerator);

    unsigned int GetNumTables();
    const char** GetTableNames();
};

class TemplateEnumerator : public ITemplateEnumerator {
private:

    TemplateEnumerator (HashTable<const char*, Template*, TemplateNameHashValue, TemplateNameEquals>* pTemplates);
    ~TemplateEnumerator();

    char** m_ppszTemplateNames;
    unsigned int m_iNumTemplates;

public:

    static TemplateEnumerator* CreateInstance (HashTable<const char*, Template*, TemplateNameHashValue, 
        TemplateNameEquals>* pTemplates);

    // IDatabaseBackup
    IMPLEMENT_INTERFACE (ITemplateEnumerator);

    unsigned int GetNumTemplates();
    const char** GetTemplateNames();
};

// Database
class Database : public IDatabase {

private:

    char* m_pszMainDirectory;
    char* m_pszDataDirectory;
    char* m_pszBackupDirectory;

    unsigned int m_iOptions;

    HashTable<const char*, Template*, TemplateNameHashValue, TemplateNameEquals> m_htTemplates;
    HashTable<const char*, Table*, TableNameHashValue, TableNameEquals> m_htTables;

    ReadWriteLock m_rwTableLock;
    ReadWriteLock m_rwTemplateLock;

    int Reload();

    int ReloadTemplates();
    int ReloadTables();
    int ReloadBackups();

    Template* FindTemplate (const char* pszTemplateName);
    Table* FindTable (const char* pszTableName);

    // Accounting
    unsigned int m_iNumLoadedRows;
    size_t m_stSizeOnDisk;

    // Backups
    ReadWriteLock m_rwBackupLock;

    unsigned int m_iNumBackups;
    unsigned int m_iNumBackupsSpace;
    IDatabaseBackup** m_ppBackupArray;

    IDatabaseBackup* m_pRestoreBackup;

    void GetBackupDirectory (int iDay, int iMonth, int iYear, int iVersion, char* pszPath);
    int RemoveBackup (unsigned int iIndex);
    int DeleteBackupFromDisk (IDatabaseBackup* pBackup);
    int ReplaceDatabaseWithBackup (IDatabaseBackup* pRestoreBackup);

    static bool IsBackup (const char* pszBackupName, int* piDay, int* piMonth, int* piYear, int* piVersion);
    static int InitializeBlankData (Variant* pvVariant, VariantType vtType);

    Database();
    ~Database();

public:

    static Database* CreateInstance();

    // Standard stuff
    void IncrementSizeOnDisk (size_t stChange);
    void DecrementSizeOnDisk (size_t stChange);

    void IncrementNumLoadedRows (unsigned int iNumRows);
    void DecrementNumLoadedRows (unsigned int iNumRows);

    const char* GetDataDirectory();

    // IDatabase
    IMPLEMENT_INTERFACE (IDatabase);

    // Return OK if database was reloaded, WARNING if new database was created,
    // something else if an error occurred
    int Initialize (const char* pszMainDirectory, unsigned int iOptions);

    // Template operations
    int CreateTemplate (const TemplateDescription& ttTemplate);
    int DeleteTemplate (const char* pszTemplateName);

    int GetTemplate (const char* pszTemplateName, ITemplate** ppTemplate);
    int GetTemplateForTable (const char* pszTableName, ITemplate** ppTemplate);

    bool DoesTemplateExist (const char* pszTemplateName);

    // Table operations
    int CreateTable (const char* pszTableName, const char* pszTemplateName);
    int ImportTable (IDatabase* pSrcDatabase, const char* pszTableName);
    int DeleteTable (const char* pszTableName);
    bool DoesTableExist (const char* pszTableName);

    // Utilities
    int Backup (IDatabaseBackupNotificationSink* pSink);
    unsigned int DeleteOldBackups (Seconds iNumSecondsOld);

    IDatabaseBackupEnumerator* GetBackupEnumerator();

    int RestoreBackup (IDatabaseBackup* pBackup);
    int DeleteBackup (IDatabaseBackup* pBackup);

    unsigned int GetOptions();

    ////////////////
    // Accounting //
    ////////////////

    const char* GetDirectory();

    unsigned int GetNumTables();
    unsigned int GetNumTemplates();

    ITableEnumerator* GetTableEnumerator();
    ITemplateEnumerator* GetTemplateEnumerator();

    bool IsTemplateEqual (const char* pszTemplateName, const TemplateDescription& ttTemplate);
    
    unsigned int GetNumLoadedRows();
    
    int Flush();

    size_t GetSizeOnDisk();

    //////////////////
    // Database API //
    //////////////////

    // Standard operations
    int ReadData (const char* pszTableName, unsigned int iKey, unsigned int iColumn, Variant* pvData);
    int ReadData (const char* pszTableName, unsigned int iColumn, Variant* pvData);
    
    int WriteData (const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vData);
    int WriteData (const char* pszTableName, unsigned int iColumn, const Variant& vData);
    
    int Increment (const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement);
    int Increment (const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, 
        Variant* pvOldValue);
    int Increment (const char* pszTableName, unsigned int iColumn, const Variant& vIncrement);
    int Increment (const char* pszTableName, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue);

    int WriteAnd (const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteAnd (const char* pszTableName, unsigned int iColumn, unsigned int iBitField);

    int WriteOr (const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteOr (const char* pszTableName, unsigned int iColumn, unsigned int iBitField);

    int WriteXor (const char* pszTableName, unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
    int WriteXor (const char* pszTableName, unsigned int iColumn, unsigned int iBitField);

    int WriteNot (const char* pszTableName, unsigned int iKey, unsigned int iColumn);
    int WriteNot (const char* pszTableName, unsigned int iColumn);

    int WriteColumn (const char* pszTableName, unsigned int iColumn, const Variant& vData);

    // Row operations
    int GetNumRows (const char* pszTableName, unsigned int* piNumRows);
    int DoesRowExist (const char* pszTableName, unsigned int iKey, bool* pbExists);

    int InsertRow (const char* pszTableName, Variant* pvColVal);
    int InsertRow (const char* pszTableName, Variant* pvColVal, unsigned int* piKey);
    int InsertRows (const char* pszTableName, Variant* pvColVal, unsigned int iNumRows);
    int InsertDuplicateRows (const char* pszTableName, Variant* pvColVal, unsigned int iNumRows);
    
    int DeleteRow (const char* pszTableName, unsigned int iKey);
    int DeleteAllRows (const char* pszTableName);
    
    int ReadRow (const char* pszTableName, unsigned int iKey, Variant** ppvData);
    int ReadRow (const char* pszTableName, Variant** ppvData);

    // Column operations
    int ReadColumn (const char* pszTableName, unsigned int iColumn, unsigned int** ppiKey, Variant** ppvData, 
        unsigned int* piNumRows);
    int ReadColumn (const char* pszTableName, unsigned int iColumn, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
        unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);

    int ReadColumns (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
        Variant*** pppvData, unsigned int* piNumRows);

    // Searches
    int GetAllKeys (const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetNextKey (const char* pszTableName, unsigned int iKey, unsigned int* piNextKey);

    int GetFirstKey (const char* pszTableName, unsigned int iColumn, const Variant& vData, bool bCaseInsensitive, 
        unsigned int* piKey);

    int GetEqualKeys (const char* pszTableName, unsigned int iColumn, const Variant& vData, 
        bool bCaseInsensitive, unsigned int** ppiKey, unsigned int* piNumKeys);
    
    int GetSearchKeys (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
        const Variant* pvData, const Variant* pvData2, unsigned int iStartKey, unsigned int iSkipHits, 
        unsigned int iMaxNumHits, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey);

    ////////////////
    // Direct API //
    ////////////////

    int GetTableForReading (const char* pszTableName, IReadTable** ppTable);
    int GetTableForWriting (const char* pszTableName, IWriteTable** ppTable);

    // Transactions
    int CreateTransaction (ITransaction** ppTransaction);

    void FreeData (void** ppData);
    
    void FreeData (Variant* pvData);
    void FreeData (Variant** ppvData);

    void FreeData (int* piData);
    void FreeData (unsigned int* piData);
    void FreeData (float* ppfData);
    void FreeData (char** ppszData);
    void FreeData (UTCTime* ptData);

    void FreeKeys (unsigned int* piKeys);
};

#endif // !defined(AFX_DATABASE_H__05370B54_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)