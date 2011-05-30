// Transaction.cpp: implementation of the Transaction class.
//
//////////////////////////////////////////////////////////////////////
//
// Database.dll - A database cache and backing store library
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

#include "Transaction.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Transaction::Transaction (Database* pDatabase) {

    m_iNumRefs = 1;
    m_pDatabase = pDatabase;

    if (m_pDatabase != NULL) {
        m_pDatabase->AddRef();
    }

    else Assert (false);
}

Transaction::~Transaction() {

    if (m_pDatabase != NULL) {
        m_pDatabase->Release();
    }
}

Transaction* Transaction::CreateInstance (Database* pDatabase) {

    return new Transaction (pDatabase);
}

int Transaction::SetComplete() {
    return ERROR_NOT_IMPLEMENTED;
}

int Transaction::SetAbort() {
    return ERROR_NOT_IMPLEMENTED;
}