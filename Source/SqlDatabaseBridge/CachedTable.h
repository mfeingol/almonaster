//
// SqlDatabaseBridge.dll - A database library
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
#include "SqlDatabase.h"

using namespace System::Collections::Generic;
using namespace Almonaster::Database::Sql;

class CachedTable : public ICachedTable
{
private:
    gcroot<SqlCommandManager^> m_cmd;
    gcroot<BulkTableReadResult^> m_result;
    bool m_bCompleteTable;

    gcroot<SortedDictionary<int64, IDictionary<System::String^, System::Object^>^>^> m_keyToRows;
    gcroot<Dictionary<int64, IDictionary<System::String^, System::Object^>^>^> m_writes;
    gcroot<System::String^> m_ID_COLUMN_NAME;

    int InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows, unsigned int* piKey);
    IDictionary<System::String^, System::Object^>^ GetRow(unsigned int iKey, int* piErrCode);
    void SaveWrite(int64 id, System::String^ columnName, System::Object^ value);

public:
    CachedTable(SqlCommandManager^ cmd, BulkTableReadResult^ result, bool bCompleteTable);

    BulkTableReadResult^ GetResult();
    IDictionary<int64, IDictionary<System::String^, System::Object^>^>^ ObtainWrites();

    // ICachedTable
    IMPLEMENT_INTERFACE(ICachedTable);

    int GetNumCachedRows(unsigned int* piNumRows);

    int GetFirstKey(const char* pszColumn, const Variant& vData, unsigned int* piKey);
    int GetNextKey(unsigned int iKey, unsigned int* piNextKey);
    int GetEqualKeys(const char* pszColumn, const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetEqualKeys(const char** ppszColumn, const Variant* pvData, unsigned int iNumColumns, unsigned int** ppiKey, unsigned int* piNumKeys);
    int GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys);

    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows);
    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumns(unsigned int iNumColumns, const char* const* ppszColumn, unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);

    int ReadColumnWhereEqual(const char* pszEqualColumn, const Variant& vData, const char* pszReadColumn, 
                             unsigned int** ppiKey, Variant** ppvData, unsigned int* piNumRows);

    int ReadColumnsWhereEqual(const char* pszEqualColumn, const Variant& vData, const char** ppszReadColumn, unsigned int iNumReadColumns,
                              unsigned int** ppiKey, Variant*** pppvData, unsigned int* piNumRows);

    int ReadRow(Variant** ppvData);
    int ReadRow(unsigned int iKey, Variant** ppvData);

    int ReadData(const char* pszColumn, int* piData);
    int ReadData(const char* pszColumn, float* pfData);
    int ReadData(const char* pszColumn, int64* pi64Data);
    int ReadData(const char* pszColumn, Variant* pvData);
    int ReadData(unsigned int iKey, const char* pszColumn, int* piData);
    int ReadData(unsigned int iKey, const char* pszColumn, float* pfData);
    int ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data);
    int ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData);

    int InsertRow(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int* piKey);
    int InsertDuplicateRows(const TemplateDescription& ttTemplate, const Variant* pvColVal, unsigned int iNumRows);
    int DeleteRow(unsigned int iKey);
    int DeleteAllRows();

    int Increment(const char* pszColumn, const Variant& vIncrement);
    int Increment(const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);
    int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement);
    int Increment(unsigned int iKey, const char* pszColumn, const Variant& vIncrement, Variant* pvOldValue);

    int WriteData(const char* pszColumn, const Variant& vData);
    int WriteData(const char* pszColumn, int iData);
    int WriteData(const char* pszColumn, float fData);
    int WriteData(const char* pszColumn, int64 i64Data);
    int WriteData(const char* pszColumn, const char* pszData);

    int WriteData(unsigned int iKey, const char* pszColumn, const Variant& vData);
    int WriteData(unsigned int iKey, const char* pszColumn, int iData);
    int WriteData(unsigned int iKey, const char* pszColumn, float fData);
    int WriteData(unsigned int iKey, const char* pszColumn, int64 i64Data);
    int WriteData(unsigned int iKey, const char* pszColumn, const char* pszData);

    int WriteAnd(const char* pszColumn, unsigned int iBitField);
    int WriteAnd(unsigned int iKey, const char* pszColumn, unsigned int iBitField);
    int WriteOr(const char* pszColumn, unsigned int iBitField);
    int WriteOr(unsigned int iKey, const char* pszColumn, unsigned int iBitField);
};