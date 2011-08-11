#pragma once

#include <vcclr.h>
#include "SqlDatabase.h"

#using <SqlDatabase.dll>
using namespace Almonaster::Database::Sql;

class ReadTableView : public IReadTableView
{
private:
    gcroot<BulkTableReadResult^> m_result;

public:
    // IReadTableView
    IMPLEMENT_INTERFACE(IReadTableView);

    ReadTableView(BulkTableReadResult^ result);

    int GetAllKeys(unsigned int** ppiKey, unsigned int* piNumKeys);

    int ReadColumn(const char* pszColumn, unsigned int** ppiKey, int** ppiData, unsigned int* piNumRows);
    int ReadRow(unsigned int iKey, Variant** ppvData);

    int ReadData(unsigned int iKey, const char* pszColumn, int* piData);
    int ReadData(unsigned int iKey, const char* pszColumn, int64* pi64Data);
    int ReadData(unsigned int iKey, const char* pszColumn, Variant* pvData);
};