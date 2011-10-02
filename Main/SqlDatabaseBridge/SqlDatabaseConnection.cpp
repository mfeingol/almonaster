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

#include "SqlDatabaseConnection.h"
#include "Utils.h"

using namespace System::Collections::Generic;
using namespace System::Data::SqlClient;

SqlDatabaseConnection::SqlDatabaseConnection(SqlDatabase^ sqlDatabase, TransactionIsolationLevel isoLevel)
    :
    m_iNumRefs(1),
    m_sqlDatabase(sqlDatabase),
    m_cmd(m_sqlDatabase->CreateCommandManager(Convert(isoLevel))),
    m_viewCollection(m_cmd)
{
}

SqlDatabaseConnection::~SqlDatabaseConnection()
{
    delete m_cmd;
}

int SqlDatabaseConnection::Commit()
{
    int iErrCode = m_viewCollection.Commit();
    if (iErrCode == OK)
    {
        try
        {
            m_cmd->SetComplete();
        }
        catch (SqlDatabaseException^ e)
        {
            TraceException(e);
            iErrCode = ERROR_DATABASE_EXCEPTION;
        }
    }
    return iErrCode;
}

ICachedTableCollection* SqlDatabaseConnection::GetCache()
{
    return &m_viewCollection;
}

// Table operations
int SqlDatabaseConnection::DoesTableExist(const char* pszTableName, bool* pbExist)
{
    Trace(TRACE_VERBOSE, "DoesTableExist %s", pszTableName);

    System::String^ tableName = gcnew System::String(pszTableName);
    try
    {
        *pbExist = m_cmd->DoesTableExist(tableName);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }

    return OK;
}

int SqlDatabaseConnection::CreateTable(const char* pszTableName, const TemplateDescription& ttTemplate)
{
    Trace(TRACE_VERBOSE, "CreateTable %s", pszTableName);

    List<ColumnDescription>^ cols = gcnew List<ColumnDescription>();

    TableDescription tableDesc;
    tableDesc.Name = gcnew System::String(pszTableName);
    tableDesc.Columns = cols;

    ColumnDescription colDesc;
    colDesc.Name = gcnew System::String(ID_COLUMN_NAME);
    colDesc.Type = SqlDbType::BigInt;
    colDesc.Size = 0;
    colDesc.IsPrimaryKey = true;
    cols->Add(colDesc);

    for (unsigned int i = 0; i < ttTemplate.NumColumns; i ++)
    {
        colDesc.Name = gcnew System::String(ttTemplate.ColumnNames[i]);
        colDesc.Type = Convert(ttTemplate.Type[i]);
        colDesc.Size = ttTemplate.Size[i] == VARIABLE_LENGTH_STRING ? System::Int32::MaxValue : ttTemplate.Size[i];
        colDesc.IsPrimaryKey = false;
        cols->Add(colDesc);
    }

    try
    {
        m_cmd->CreateTable(tableDesc);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }

    // TODO - 494 - Add database indexes

    return OK;
}

int SqlDatabaseConnection::DeleteTable(const char* pszTableName)
{
    Trace(TRACE_VERBOSE, "DeleteTable %s", pszTableName);
    
    System::String^ tableName = gcnew System::String(pszTableName);
    try
    {
        m_cmd->DeleteTable(tableName);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }
    return OK;
}

// Row operations
int SqlDatabaseConnection::GetNumPhysicalRows(const char* pszTableName, unsigned int* piNumRows)
{
    Trace(TRACE_VERBOSE, "GetNumRows %s", pszTableName);
    try
    {
        *piNumRows = (unsigned int)m_cmd->GetRowCount(gcnew System::String(pszTableName));
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    return OK;
}

int SqlDatabaseConnection::GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
    Trace(TRACE_VERBOSE, "GetSearchKeys %s", pszTableName);

    array<RangeSearchColumn>^ cols = gcnew array<RangeSearchColumn>(sdRange.iNumColumns);

    for (unsigned int i = 0; i < sdRange.iNumColumns; i ++)
    {
        cols[i].ColumnName = gcnew System::String(sdRange.pscColumns[i].pszColumn);
        cols[i].GreaterThanOrEqual = Convert(sdRange.pscColumns[i].vData);
        if (sdRange.pscColumns[i].vData.GetType() == V_STRING)
        {
            cols[i].LessThanOrEqual = cols[i].GreaterThanOrEqual;
        }
        else
        {
            cols[i].LessThanOrEqual = Convert(sdRange.pscColumns[i].vData2);
        }

        // TODO - 611 - Implement or remove search flags and skip hits
    }

    int64 maxHits = sdRange.iMaxNumHits == 0 ? System::Int32::MaxValue : sdRange.iMaxNumHits + 1;
    IEnumerable<int64>^ results;
    try
    {
        results = m_cmd->Search(gcnew System::String(pszTableName), gcnew System::String(ID_COLUMN_NAME), maxHits, sdRange.iSkipHits, cols, nullptr);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }

    unsigned int iNumHits;
    *ppiKey = ConvertIdsToKeys(results, &iNumHits);

    if (sdRange.iMaxNumHits > 0 && iNumHits > sdRange.iMaxNumHits)
    {
        if (piStopKey != NULL)
        {
            *piStopKey = (*ppiKey)[iNumHits - 1];
        }

        *piNumHits = sdRange.iMaxNumHits;
        return ERROR_TOO_MANY_HITS;
    }
    else
    {
        if (piStopKey != NULL)
        {
            *piStopKey = NO_KEY;
        }

        *piNumHits = iNumHits;
        return OK;
    }
}

int SqlDatabaseConnection::GetSearchKeys(const char* pszTableName, const RangeSearchDefinition& sdRange, const OrderByDefinition& sdOrderBy, unsigned int** ppiKey, unsigned int* piNumHits)
{
    Trace(TRACE_VERBOSE, "GetSearchKeys %s", pszTableName);

    array<RangeSearchColumn>^ rangeCols = gcnew array<RangeSearchColumn>(sdRange.iNumColumns);

    for (unsigned int i = 0; i < sdRange.iNumColumns; i ++)
    {
        rangeCols[i].ColumnName = gcnew System::String(sdRange.pscColumns[i].pszColumn);
        rangeCols[i].GreaterThanOrEqual = Convert(sdRange.pscColumns[i].vData);
        if (sdRange.pscColumns[i].vData.GetType() == V_STRING)
        {
            rangeCols[i].LessThanOrEqual = rangeCols[i].GreaterThanOrEqual;
        }
        else
        {
            rangeCols[i].LessThanOrEqual = Convert(sdRange.pscColumns[i].vData2);
        }

        // TODO - 611 - Implement or remove search flags and skip hits
    }

    array<OrderBySearchColumn>^ orderByCols = gcnew array<OrderBySearchColumn>(sdOrderBy.iNumColumns);
    for (unsigned int i = 0; i < sdOrderBy.iNumColumns; i ++)
    {
        orderByCols[i].ColumnName = gcnew System::String(sdOrderBy.pscColumns[i].pszColumn);
        orderByCols[i].Ascending = sdOrderBy.pscColumns[i].bAscending;
    }

    int64 maxHits = sdRange.iMaxNumHits == 0 ? System::Int32::MaxValue : sdRange.iMaxNumHits + 1;
    IEnumerable<int64>^ results;
    try
    {
        results = m_cmd->Search(gcnew System::String(pszTableName), gcnew System::String(ID_COLUMN_NAME), maxHits, sdRange.iSkipHits, rangeCols, orderByCols);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }

    unsigned int iNumHits;
    *ppiKey = ConvertIdsToKeys(results, &iNumHits);

    if (sdRange.iMaxNumHits > 0 && iNumHits > sdRange.iMaxNumHits)
    {
        *piNumHits = sdRange.iMaxNumHits;
        return ERROR_TOO_MANY_HITS;
    }
    else
    {
        *piNumHits = iNumHits;
        return OK;
    }
}