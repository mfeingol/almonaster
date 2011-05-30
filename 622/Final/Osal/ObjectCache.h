// ObjectCache.h: interface for the ObjectCache class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
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

#if !defined(AFX_OBJECTCACHE_H__1DA6A6D6_9A12_11D3_A263_0050047FE2E2__INCLUDED_)
#define AFX_OBJECTCACHE_H__1DA6A6D6_9A12_11D3_A263_0050047FE2E2__INCLUDED_

#include "OS.h"

#define CACHE_DEBUG_VALUE_BYTE (0xbe)

#ifdef __WIN64__
#define CACHE_DEBUG_VALUE_PTR ((T*) 0xbebebebebebebebe)
#else
#define CACHE_DEBUG_VALUE_PTR ((T*) 0xbebebebe)
#endif

template <class T, class TAllocator> class ObjectCache {
private:

    unsigned int m_iMaxNumElements;
    unsigned int m_iNumFreeObjects;
    unsigned int m_iFreeIndex;

    T** m_ppObjectCache;

public:
    
    ObjectCache() {

        m_iMaxNumElements = 0;
        m_ppObjectCache = NULL;
        m_iFreeIndex = 0;
        m_iNumFreeObjects = 0;
    }

    ~ObjectCache() {

        // Delete objects in cache
        while (m_iNumFreeObjects > 0) {
            TAllocator::Delete (GetObject());
        }

        // Delete cache storage
        if (m_ppObjectCache != NULL) {
            delete [] m_ppObjectCache;
        }
    }

    bool Initialize (unsigned int iMaxNumElements) {

        m_iMaxNumElements = iMaxNumElements;

        if (m_iMaxNumElements > 0) {

            m_ppObjectCache = new T* [m_iMaxNumElements];
            if (m_ppObjectCache == NULL) {
                return false;
            }

#ifdef _DEBUG
            memset (m_ppObjectCache, CACHE_DEBUG_VALUE_BYTE, m_iMaxNumElements * sizeof (T*));
#endif
        }

        return true;
    }

    bool IsEmpty() {
        return m_iNumFreeObjects == 0;
    }

    T* GetObject() {

        if (m_iNumFreeObjects == 0) {
            return TAllocator::New();
        }
        
        T* pCachedObject = m_ppObjectCache [m_iFreeIndex];
        
#ifdef _DEBUG
        m_ppObjectCache[m_iFreeIndex] = CACHE_DEBUG_VALUE_PTR;
#endif
        m_iNumFreeObjects --;
        
        if (m_iNumFreeObjects > 0) {
            
            if (m_iFreeIndex == 0) {
                m_iFreeIndex = m_iMaxNumElements - 1;
            } else {
                m_iFreeIndex --;
            }
        }

        Assert (pCachedObject != CACHE_DEBUG_VALUE_PTR);
        return pCachedObject;
    }

    void ReleaseObject (T* pObject) {

        if (m_iNumFreeObjects == m_iMaxNumElements) {
            
            TAllocator::Delete (pObject);

        } else {
            
            if (m_iNumFreeObjects == 0) {
                m_iFreeIndex = 0;
            } else {
                m_iFreeIndex ++;
                if (m_iFreeIndex == m_iMaxNumElements) {
                    m_iFreeIndex = 0;
                }
            }

#ifdef __WIN32__
            Assert (m_ppObjectCache [m_iFreeIndex] == CACHE_DEBUG_VALUE_PTR);
#endif
            m_ppObjectCache [m_iFreeIndex] = pObject;
            m_iNumFreeObjects ++;
        }
    }
};

#endif // !defined(AFX_OBJECTCACHE_H__1DA6A6D6_9A12_11D3_A263_0050047FE2E2__INCLUDED_)