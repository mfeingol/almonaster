// GameEngineGameObject.h: interface for the GameObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEENGINEGAMEOBJECT_H__E21FE393_D69B_11D3_A2F0_0050047FE2E2__INCLUDED_)
#define AFX_GAMEENGINEGAMEOBJECT_H__E21FE393_D69B_11D3_A2F0_0050047FE2E2__INCLUDED_

#include "Osal/IObject.h"
#include "Osal/ReadWriteLock.h"

class GameObject : public ReadWriteLock, public IObject {

    char* m_pszName;

public:

    GameObject();
    ~GameObject();

    int SetName (const char* pszName);
    const char* GetName();

    IMPLEMENT_IOBJECT;
};

#endif // !defined(AFX_GAMEENGINEGAMEOBJECT_H__E21FE393_D69B_11D3_A2F0_0050047FE2E2__INCLUDED_)