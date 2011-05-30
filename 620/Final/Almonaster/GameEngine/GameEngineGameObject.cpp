// GameEngineGameObject.cpp: implementation of the GameEngineGameObject class.
//
//////////////////////////////////////////////////////////////////////

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