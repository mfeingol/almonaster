//
// OdbcDatabase.dll - A database library
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

#include "Database.h"
#include "OdbcDatabase.h"

// {70C5DA9F-B23D-4EB7-AECC-E65CF1FFF49A}
const Uuid CLSID_OdbcDatabase = { 0x70c5da9f, 0xb23d, 0x4eb7, { 0xae, 0xcc, 0xe6, 0x5c, 0xf1, 0xff, 0xf4, 0x9a } };

int CreateInstance (const Uuid& clsidClassId, const Uuid& iidInterface, void** ppObject) {

    if (clsidClassId == CLSID_OdbcDatabase && iidInterface == IID_IDatabase) {

        *ppObject = OdbcDatabase::CreateInstance();
        return *ppObject == NULL ? ERROR_FAILURE : OK;
    }

    *ppObject = NULL;
    return ERROR_NO_INTERFACE;
}
