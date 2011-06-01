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

#define DATABASE_BUILD
#include "Database.h"
#include "CDatabase.h"
#undef DATABASE_BUILD

// {51593574-72FB-4363-A91D-253338E81468}
const Uuid CLSID_Database = { 0x51593574, 0x72fb, 0x4363, { 0xa9, 0x1d, 0x25, 0x33, 0x38, 0xe8, 0x14, 0x68 } };

extern "C" DATABASE_EXPORT int CreateInstance (const Uuid& clsidClassId, const Uuid& iidInterface, void** ppObject) {

    if (clsidClassId == CLSID_Database && iidInterface == IID_IDatabase) {

        *ppObject = Database::CreateInstance();
        return *ppObject == NULL ? ERROR_FAILURE : OK;
    }

    *ppObject = NULL;
    return ERROR_NO_INTERFACE;
}

const Uuid IID_ITransaction = { 0x6538a8cd, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IReadTable = { 0x6538a8ce, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IWriteTable = { 0x6538a8cf, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IDatabase = { 0x6538a8d0, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IDatabaseBackup = { 0x6538a8d1, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IDatabaseBackupEnumerator = { 0x6538a8d2, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_ITableEnumerator = { 0x6538a8d3, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_ITemplateEnumerator = { 0x6538a8d4, 0x8c2c, 0x11d3, { 0xa2, 0x3e, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_IDatabaseBackupNotificationSink = { 0x91004e11, 0xb828, 0x11d3, { 0xa2, 0xa8, 0x0, 0x50, 0x4, 0x7f, 0xe2, 0xe2 } };
const Uuid IID_ITemplate = { 0x6a1adaff, 0xa8c2, 0x4536, { 0xa4, 0xb1, 0xdd, 0x36, 0x6b, 0xc4, 0xde, 0x64 } };