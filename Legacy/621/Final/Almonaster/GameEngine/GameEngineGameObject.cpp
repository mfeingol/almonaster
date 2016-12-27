//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998-2004 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "GameEngineGameObject.h"

#include "Osal/String.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GameObject::GameObject() {

    m_iNumRefs = 1;
    m_pszName = NULL;
}

GameObject::~GameObject() {

    if (m_pszName != NULL) {
        OS::HeapFree (m_pszName);
    }
}

int GameObject::SetName (const char* pszName) {

    m_pszName = String::StrDup (pszName);

    return m_pszName == NULL ? ERROR_OUT_OF_MEMORY : OK;
}

const char* GameObject::GetName() {

    return m_pszName;
}