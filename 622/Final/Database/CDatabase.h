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

#if !defined(AFX_DATABASE_H__05370B54_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)
#define AFX_DATABASE_H__05370B54_2751_11D3_A0DC_0050047FE2E2__INCLUDED_

#include <stdio.h>

#include "Osal/HashTable.h"
#include "Osal/File.h"
#include "Osal/Algorithm.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/FileHeap.h"

#define DATABASE_BUILD
#include "Database.h"
#undef DATABASE_BUILD

#define TABLE_DATA_FILE         "Tables.dat"
#define VARIABLE_DATA_FILE      "Variable.dat"
#define META_DATA_FILE          "Meta.dat"
#define TEMPLATE_DATA_FILE      "Templates.dat"

#define DEFAULT_TABLE_SIZE      (3 * 1024)
#define DEFAULT_VARIABLE_SIZE   (3 * 1024)
#define DEFAULT_META_SIZE       (3 * 1024)
#define DEFAULT_TEMPLATE_SIZE   (3 * 1024)

#define BACKUP_DIRECTORY        "Backups"


class Template;
class Table;

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

typedef HashTable<const char*, Template*, TemplateNameHashValue, TemplateNameEquals> TemplateHashTable;
typedef HashTable<const char*, Table*, TableNameHashValue, TableNameEquals> TableHashTable;

// Enumerators
class TableEnumerator : public ITableEnumerator {
private:

    TableEnumerator();
    ~TableEnumerator();

    int Initialize (TableHashTable* pTables);

    char** m_ppszTableNames;
    unsigned int m_iNumTables;

public:

    static TableEnumerator* CreateInstance (TableHashTable* pTables);

    // IDatabaseBackup
    IMPLEMENT_INTERFACE (ITableEnumerator);

    unsigned int GetNumTables();
    const char** GetTableNames();
};

class TemplateEnumerator : public ITemplateEnumerator {
private:

    TemplateEnumerator();
    ~TemplateEnumerator();

    int Initialize (TemplateHashTable* pTemplates);

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

protected:

    FileHeap m_fhTableData;
    FileHeap m_fhVariableData;
    FileHeap m_fhMetaData;
    FileHeap m_fhTemplateData;

    char* m_pszMainDirectory;
    char* m_pszDataDirectory;
    char* m_pszBackupDirectory;

    unsigned int m_iOptions;

    TemplateHashTable* m_pTemplates;
    TableHashTable* m_pTables;

    ReadWriteLock m_rwTableLock;
    ReadWriteLock m_rwTemplateLock;
    ReadWriteLock m_rwGlobalLock;
    ReadWriteLock m_rwHeapLock;

    // Backups
    ReadWriteLock m_rwBackupLock;

    unsigned int m_iNumBackups;
    unsigned int m_iNumBackupsSpace;

    IDatabaseBackup** m_ppBackupArray;
    IDatabaseBackup* m_pRestoreBackup;

    bool m_bInitialized;

    void GetBackupDirectory (int iDay, int iMonth, int iYear, int iVersion, char* pszPath);
    int RemoveBackup (unsigned int iIndex);
    int DeleteBackupFromDisk (IDatabaseBackup* pBackup);
    int ReplaceDatabaseWithBackup (IDatabaseBackup* pRestoreBackup);

    int OpenDataFile (const char* pszFile, FileHeap* pfhHeap, size_t stDefaultSize, bool* pbExists);

    static bool IsBackup (const char* pszBackupName, int* piDay, int* piMonth, int* piYear, int* piVersion);
    static int InitializeBlankData (Variant* pvVariant, VariantType vtType);

    int Reload();

    int ReloadTemplates();
    int ReloadTables();
    int ReloadBackups();

    Database();
    ~Database();

public:

    static Database* CreateInstance();

    // Standard stuff
    Template* FindTemplate (const char* pszTemplateName);
    Table* FindTable (const char* pszTableName);

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
    int Backup (IDatabaseBackupNotificationSink* pSink, bool bCheckFirst);
    unsigned int DeleteOldBackups (Seconds iNumSecondsOld);

    IDatabaseBackupEnumerator* GetBackupEnumerator();

    int RestoreBackup (IDatabaseBackup* pBackup);
    int DeleteBackup (IDatabaseBackup* pBackup);

    inline FileHeap* GetTableFileHeap() { return &m_fhTableData; }
    inline FileHeap* GetVariableDataFileHeap() { return &m_fhVariableData; }
    inline FileHeap* GetMetaDataFileHeap() { return &m_fhMetaData; }

    inline FileHeap* GetTemplateFileHeap() { return &m_fhTemplateData; }

    inline ReadWriteLock* GetGlobalLock() { return &m_rwGlobalLock; }
    inline ReadWriteLock* GetHeapLock() { return &m_rwHeapLock; }

    ////////////////
    // Accounting //
    ////////////////

    const char* GetDirectory();

    unsigned int GetNumTables();
    unsigned int GetNumTemplates();

    ITableEnumerator* GetTableEnumerator();
    ITemplateEnumerator* GetTemplateEnumerator();

    bool IsTemplateEqual (const char* pszTemplateName, const TemplateDescription& ttTemplate);
    
    // TODO - delete when upgrades from old database are finished
    void Obsolete1() {}
    void Obsolete2() {}

    int Flush();
    int Check();

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
    unsigned int GetNumRows (const char* pszTableName, unsigned int* piNumRows);
    int DoesRowExist (const char* pszTableName, unsigned int iKey, bool* pbExists);

    int InsertRow (const char* pszTableName, const Variant* pvColVal, unsigned int* piKey = NULL);
    int InsertRow (const char* pszTableName, const Variant* pvColVal, unsigned int iKey);

    int InsertRows (const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows);
    int InsertDuplicateRows (const char* pszTableName, const Variant* pvColVal, unsigned int iNumRows);
    
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
    
    int GetSearchKeys (const char* pszTableName, const SearchDefinition& sdSearch, unsigned int** ppiKey, 
        unsigned int* piNumHits, unsigned int* piStopKey);

    int ReadColumnWhereEqual (const char* pszTableName, unsigned int iEqualColumn, const Variant& vData, 
        bool bCaseInsensitive, unsigned int iReadColumn, unsigned int** ppiKey, Variant** ppvData, 
        unsigned int* piNumKeys);

    //
    // Direct API
    //

    int GetTableForReading (const char* pszTableName, IReadTable** ppTable);
    int GetTableForWriting (const char* pszTableName, IWriteTable** ppTable);

    //
    // Transactions
    //
    int CreateTransaction (ITransaction** ppTransaction);

    //
    void FreeData (void** ppData);
    
    void FreeData (Variant* pvData);
    void FreeData (Variant** ppvData);

    void FreeData (int* piData);
    void FreeData (unsigned int* puiData);
    void FreeData (float* ppfData);
    void FreeData (char** ppszData);
    void FreeData (UTCTime* ptData);

    void FreeKeys (unsigned int* piKeys);

    // Stats
    int GetStatistics (DatabaseStatistics* pdsStats);
    unsigned int GetOptions();
};

#endif // !defined(AFX_DATABASE_H__05370B54_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)