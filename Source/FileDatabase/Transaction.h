// Transaction.h: interface for the Transaction class.
//
//////////////////////////////////////////////////////////////////////
//
// Database.dll - A database cache and backing store library
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

#if !defined(AFX_TRANSACTION_H__0BE30DA1_5531_11D1_A735_0060B0579E6B__INCLUDED_)
#define AFX_TRANSACTION_H__0BE30DA1_5531_11D1_A735_0060B0579E6B__INCLUDED_

#define FILE_DATABASE_BUILD
#include "FileDatabase.h"
#include "CDatabase.h"
#undef FILE_DATABASE_BUILD

class Transaction : virtual public Database {//, virtual public ITransaction {

private:

    Transaction (Database* pDatabase);
    ~Transaction();

    IMPLEMENT_IOBJECT;

    Database* m_pDatabase;

public:

    static Transaction* CreateInstance (Database* pDatabase);

    //
    // IDatabase
    //

/*  int Initialize (const char* pszDatabaseName, unsigned int iOptions);

    int CreateTemplate (const TemplateDescription& ttTemplate);
    int DeleteTemplate (const char* pszTemplateName);

    int GetTemplate (const char* pszTemplateName, ITemplate** ppTemplate);
    int GetTemplateForTable (const char* pszTableName, ITemplate** ppTemplate);

    bool DoesTemplateExist (const char* pszTemplateName);

    int CreateTable (const char* pszTableName, const char* pszTemplateName);
    int ImportTable (IDatabase* pSrcDatabase, const char* pszTableName);
    int DeleteTable (const char* pszTableName);

    bool DoesTableExist (const char* pszTableName);

    int Backup (IDatabaseBackupNotificationSink* pSink);
    unsigned int DeleteOldBackups (Seconds iNumSecondsOld);

    IDatabaseBackupEnumerator* GetBackupEnumerator();

    int RestoreBackup (IDatabaseBackup* pBackup);
    int DeleteBackup (IDatabaseBackup* pBackup);

    const char* GetDirectory();

    unsigned int GetNumTables();
    unsigned int GetNumTemplates();

    ITableEnumerator* GetTableEnumerator();
    ITemplateEnumerator* GetTemplateEnumerator();

    bool IsTemplateEqual (const char* pszTemplateName, const TemplateDescription& ttTemplate);

    int Flush();

    int ReadData (const char* pszTableName, unsigned int iKey, unsigned int iColumn, Variant* pvData);  
    int ReadData (const char* pszTableName, unsigned int iColumn, Variant* pvData);
    
    int WriteData (const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vData);
    int WriteData (const char* pszTableName, unsigned int iColumn, const Variant& vData);
    
    int Increment (const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement);
    int Increment (const char* pszTableName, unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue);
    
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

    int ReadColumn (const char* pszTableName, unsigned int iColumn, unsigned int** ppiKey, 
        Variant** ppvData, unsigned int* piNumRows);
    int ReadColumn (const char* pszTableName, unsigned int iColumn, Variant** ppvData, 
        unsigned int* piNumRows);

    int ReadColumns (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
        unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);
    int ReadColumns (const char* pszTableName, unsigned int iNumColumns, const unsigned int* piColumn, 
        Variant*** pppvData, unsigned int* piNumRows);

    int GetAllKeys (const char* pszTableName, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetFirstKey (const char* pszTableName, unsigned int iColumn, const Variant& vData, 
        bool bCaseInsensitive, unsigned int* piKey);

    int GetEqualKeys (const char* pszTableName, unsigned int iColumn, const Variant& vData, 
        bool bCaseInsensitive, unsigned int** ppiKey, unsigned int* piNumKeys);
    
    int GetSearchKeys (const char* pszTableName, const SearchDefinition& sdSearch, unsigned int** ppiKey, 
    unsigned int* piNumHits, unsigned int* piStopKey);

    int GetTableForReading(const char* pszTableName, IReadTable** ppTable);
    int GetTableForWriting (const char* pszTableName, IWriteTable** ppTable);

    int CreateTransaction (ITransaction** ppTransaction);

    void FreeData (void** ppData);

    void FreeData (Variant* pvData);
    void FreeData (Variant** ppvData);

    void FreeData (int* piData);
    void FreeData (float* ppfData);
    void FreeData (char** ppszData);

    void FreeKeys (unsigned int* piKeys);
*/
    //
    // ITransaction
    //

    int SetComplete();
    int SetAbort();
};

#endif // !defined(AFX_TRANSACTION_H__0BE30DA1_5531_11D1_A735_0060B0579E6B__INCLUDED_)