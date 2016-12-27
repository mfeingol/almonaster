// IObject.h: interface for the OS class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
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

#if !defined(AFX_IOBJECT_H__CB52E516_F346_11D1_9DAF_0060083E8062__INCLUDED_)
#define AFX_IOBJECT_H__CB52E516_F346_11D1_9DAF_0060083E8062__INCLUDED_

#include "OS.h"
#include "Algorithm.h"

OSAL_EXPORT bool operator== (const Uuid& uLeft, const Uuid& uRight);
OSAL_EXPORT extern const Uuid IID_IObject;

class IObject {
public:
	virtual unsigned int AddRef() = 0;
	virtual unsigned int Release() = 0;
	virtual int QueryInterface (const Uuid& iidInterface, void** ppInterface) = 0;
};

#define DECLARE_IOBJECT		\
	unsigned int AddRef();	\
	unsigned int Release();	\
	int QueryInterface (const Uuid& iidInterface, void** ppInterface);

#define IMPLEMENT_INTERFACE(Interface)										\
private:																	\
	unsigned int m_iNumRefs;												\
public:																		\
	unsigned int AddRef() {													\
		return Algorithm::AtomicIncrement (&m_iNumRefs) + 1;				\
	}																		\
	unsigned int Release() {												\
		unsigned int iNumRefs = Algorithm::AtomicDecrement (&m_iNumRefs);	\
		if (-- iNumRefs == 0) {												\
			delete this;													\
		}																	\
		return iNumRefs;													\
	}																		\
	int QueryInterface (const Uuid& iidInterface, void** ppInterface) {	\
		if (iidInterface == IID_IObject) {									\
			*ppInterface = (void*) static_cast<IObject*> (this);			\
			AddRef();														\
			return OK;														\
		}																	\
		if (iidInterface == IID_##Interface) {								\
			*ppInterface = (void*) static_cast<Interface*> (this);			\
			AddRef();														\
			return OK;														\
		}																	\
		*ppInterface = NULL;												\
		return ERROR_NO_INTERFACE;											\
	}

#define IMPLEMENT_TWO_INTERFACES(Interface1, Interface2)					\
private:																	\
	unsigned int m_iNumRefs;												\
public:																		\
	unsigned int AddRef() {													\
		return Algorithm::AtomicIncrement (&m_iNumRefs) + 1;				\
	}																		\
	unsigned int Release() {												\
		unsigned int iNumRefs = Algorithm::AtomicDecrement (&m_iNumRefs);	\
		if (-- iNumRefs == 0) {												\
			delete this;													\
		}																	\
		return iNumRefs;													\
	}																		\
	int QueryInterface (const Uuid& iidInterface, void** ppInterface) {	\
		if (iidInterface == IID_IObject) {									\
			*ppInterface = (void*) static_cast<IObject*> (this);			\
			AddRef();														\
			return OK;														\
		}																	\
		if (iidInterface == IID_##Interface1) {								\
			*ppInterface = (void*) static_cast<Interface1*> (this);			\
			AddRef();														\
			return OK;														\
		}																	\
		if (iidInterface == IID_##Interface2) {								\
			*ppInterface = (void*) static_cast<Interface2*> (this);			\
			AddRef();														\
			return OK;														\
		}																	\
		*ppInterface = NULL;												\
		return ERROR_NO_INTERFACE;											\
	}

#define IMPLEMENT_IOBJECT													\
private:																	\
	unsigned int m_iNumRefs;												\
public:																		\
	unsigned int AddRef() {													\
		return Algorithm::AtomicIncrement (&m_iNumRefs) + 1;				\
	}																		\
	unsigned int Release() {												\
		unsigned int iNumRefs = Algorithm::AtomicDecrement (&m_iNumRefs);	\
		if (-- iNumRefs == 0) {												\
			delete this;													\
		}																	\
		return iNumRefs;													\
	}																		\
	int QueryInterface (const Uuid& iidInterface, void** ppInterface) {	\
		if (iidInterface == IID_IObject) {									\
			*ppInterface = (void*) static_cast<IObject*> (this);			\
			AddRef();														\
			return OK;														\
		}																	\
		*ppInterface = NULL;												\
		return ERROR_NO_INTERFACE;											\
	}

#endif