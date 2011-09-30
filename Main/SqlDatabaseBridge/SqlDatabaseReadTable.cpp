#include "SqlDatabaseReadTable.h"
#include "Utils.h"

using namespace System::Collections::Generic;
using namespace System::Data::SqlClient;
using namespace System::Linq;

SqlDatabaseReadTable::SqlDatabaseReadTable(SqlCommandManager^ cmd, System::String^ tableName)
{
    m_cmd = cmd;
    m_tableName = tableName;
}

SqlDatabaseReadTable::~SqlDatabaseReadTable()
{
}

int SqlDatabaseReadTable::GetNumRows(unsigned int* piNumRows)
{
    Trace("GetNumRows {0}", m_tableName);
    try
    {
        *piNumRows = (unsigned int)m_cmd->GetRowCount(m_tableName);
    }
    catch (SqlDatabaseException^)
    {
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    return OK;
}

int SqlDatabaseReadTable::GetSearchKeys(const RangeSearchDefinition& sdRange, unsigned int** ppiKey, unsigned int* piNumHits, unsigned int* piStopKey)
{
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

        // TODOTODO - Flags
    }

    int64 maxHits = sdRange.iMaxNumHits == 0 ? System::Int32::MaxValue : sdRange.iMaxNumHits + 1;
    IEnumerable<int64>^ results;
    try
    {
        results = m_cmd->Search(m_tableName, gcnew System::String(ID_COLUMN_NAME), maxHits, sdRange.iSkipHits, cols, nullptr);
    }
    catch (SqlDatabaseException^)
    {
        return ERROR_UNKNOWN_TABLE_NAME;
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

int SqlDatabaseReadTable::GetSearchKeys(const RangeSearchDefinition& sdRange, const OrderByDefinition& sdOrderBy, unsigned int** ppiKey, unsigned int* piNumHits)
{
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

        // TODOTODO - Flags
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
        results = m_cmd->Search(m_tableName, gcnew System::String(ID_COLUMN_NAME), maxHits, sdRange.iSkipHits, rangeCols, orderByCols);
    }
    catch (SqlDatabaseException^)
    {
        return ERROR_UNKNOWN_TABLE_NAME;
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
