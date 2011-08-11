//
// OdbcDatabase.dll - A database library
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

#include "SqlDatabaseBridge.h"

extern "C" SQL_DATABASE_EXPORT int CreateInstance(const Uuid& clsidClassId, const Uuid& iidInterface, void** ppObject)
{
    if(clsidClassId == CLSID_SqlDatabase && iidInterface == IID_IDatabase)
    {
        *ppObject = SqlDatabaseBridge::CreateInstance();
        return *ppObject == NULL ? ERROR_FAILURE : OK;
    }

    *ppObject = NULL;
    return ERROR_NO_INTERFACE;
}

// {F0E65B48-F797-4E57-8459-C0EE4FC7BE9C}
const Uuid CLSID_SqlDatabase = { 0xf0e65b48, 0xf797, 0x4e57, { 0x84, 0x59, 0xc0, 0xee, 0x4f, 0xc7, 0xbe, 0x9c } };

const Uuid IID_IDatabase = { 0x6538a8d0, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IDatabaseBackup = { 0x6538a8d1, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };

// {E7A4CAE2-B56A-4FAE-9719-186FDEA86B93}
const Uuid IID_ITableViewCollection = { 0xe7a4cae2, 0xb56a, 0x4fae, { 0x97, 0x19, 0x18, 0x6f, 0xde, 0xa8, 0x6b, 0x93 } };

// {6E057309-39AE-4C2F-A46B-C5D4106070A1}
const Uuid IID_IReadTableView = { 0x6e057309, 0x39ae, 0x4c2f, { 0xa4, 0x6b, 0xc5, 0xd4, 0x10, 0x60, 0x70, 0xa1 } };

// {9EFA75F3-F3B2-4C7B-8C4A-025D02537F05}
const Uuid IID_IDatabaseConnection = { 0x9efa75f3, 0xf3b2, 0x4c7b, { 0x8c, 0x4a, 0x2, 0x5d, 0x2, 0x53, 0x7f, 0x5 } };
const Uuid IID_IReadTable = { 0x6538a8ce, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IWriteTable = { 0x6538a8cf, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };

const Uuid IID_IDatabaseBackupEnumerator = { 0x6538a8d2, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_ITableEnumerator = { 0x6538a8d3, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_ITemplateEnumerator = { 0x6538a8d4, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IDatabaseBackupNotificationSink = { 0x91004e11, 0xb828, 0x11d3, { 0xa2, 0xa8, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_ITemplate = { 0x6a1adaff, 0xa8c2, 0x4536, { 0xa4, 0xb1, 0xdd, 0x36, 0x6b, 0xc4, 0xde, 0x64 } };