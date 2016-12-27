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
}

int SqlDatabaseConnection::Commit()
{
    int iErrCode = m_viewCollection.Commit();
    if (iErrCode == OK)
    {
        try
        {
            m_cmd->SetComplete();
            delete m_cmd;
            m_cmd = nullptr;
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
        return ERROR_DATABASE_EXCEPTION;
    }

    return OK;
}

int SqlDatabaseConnection::GetSearchKeys(const char* pszTableName, const SearchDefinition& sd, const OrderByDefinition* psdOrderBy,
                                         unsigned int** ppiKey, unsigned int* piNumHits, bool* pbMore)
{
    Trace(TRACE_VERBOSE, "GetSearchKeys %s", pszTableName);

    *pbMore = false;
    *piNumHits = 0;
    *ppiKey = NULL;

    array<SearchColumnMetadata>^ searchCols = gcnew array<SearchColumnMetadata>(sd.iNumColumns);

    for (unsigned int i = 0; i < sd.iNumColumns; i ++)
    {
        searchCols[i].ColumnName = sd.pscColumns[i].pszColumn != NULL ? gcnew System::String(sd.pscColumns[i].pszColumn) :
                                                                        gcnew System::String(ID_COLUMN_NAME);
        searchCols[i].Field1 = Convert(sd.pscColumns[i].vData);

        switch(sd.pscColumns[i].iType)
        {
        case SEARCH_RANGE_INCLUSIVE:
            searchCols[i].Type = SearchType::RangeInclusive;
            searchCols[i].Field2 = Convert(sd.pscColumns[i].vData2);
            break;
        case SEARCH_EQUALITY:
            searchCols[i].Type = SearchType::Equality;
            break;
        case SEARCH_CONTAINS_STRING:
            searchCols[i].Type = SearchType::ContainsString;
            break;
        case SEARCH_BEGINS_WITH_STRING:
            searchCols[i].Type = SearchType::BeginsWithString;
            break;
        case SEARCH_ENDS_WITH_STRING:
            searchCols[i].Type = SearchType::EndsWithString;
            break;
        default:
            return ERROR_INVALID_ARGUMENT;
        }
    }

    array<OrderBySearchColumn>^ orderByCols = nullptr;
    if (psdOrderBy)
    {
        orderByCols = gcnew array<OrderBySearchColumn>(psdOrderBy->iNumColumns);
        for (unsigned int i = 0; i < psdOrderBy->iNumColumns; i ++)
        {
            orderByCols[i].ColumnName = gcnew System::String(psdOrderBy->pscColumns[i].pszColumn);
            orderByCols[i].Ascending = psdOrderBy->pscColumns[i].bAscending;
        }
    }

    unsigned int maxHits = sd.iMaxNumHits == 0 ? 0 : sd.iMaxNumHits + 1;
    IEnumerable<System::Object^>^ results;
    try
    {
        results = m_cmd->Search(gcnew System::String(pszTableName), gcnew System::String(ID_COLUMN_NAME), maxHits, sd.iSkipHits, searchCols, orderByCols);
    }
    catch (SqlDatabaseException^ e)
    {
        TraceException(e);
        return ERROR_DATABASE_EXCEPTION;
    }

    unsigned int iNumHits;
    *ppiKey = ConvertIdsToKeys(results, &iNumHits);

    if (iNumHits == 0)
    {
        return ERROR_DATA_NOT_FOUND;
    }

    if (sd.iMaxNumHits > 0 && iNumHits > sd.iMaxNumHits)
    {
        *pbMore = true;
        *piNumHits = sd.iMaxNumHits;
    }
    else
    {
        *pbMore = false;
        *piNumHits = iNumHits;
    }
    return OK;
}