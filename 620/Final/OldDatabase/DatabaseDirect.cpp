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

#define DATABASE_BUILD
#include "CDatabase.h"
#include "ReadTable.h"
#include "WriteTable.h"
#undef DATABASE_BUILD

int Database::GetTableForReading (const char* pszTableName, IReadTable** ppTable) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *ppTable = NULL;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitReader();
    *ppTable = ReadTable::CreateInstance (pTable);
    pTable->Release();

    return OK;
}

int Database::GetTableForWriting (const char* pszTableName, IWriteTable** ppTable) {

    Table* pTable = FindTable (pszTableName);
    if (pTable == NULL) {
        *ppTable = NULL;
        return ERROR_UNKNOWN_TABLE_NAME;
    }

    pTable->WaitWriter();
    *ppTable = WriteTable::CreateInstance (pTable);
    pTable->Release();

    return OK;
}

int Database::CreateTransaction (ITransaction** ppTransaction) {

    return ERROR_NOT_IMPLEMENTED;
}