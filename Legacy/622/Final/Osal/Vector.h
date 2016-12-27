// Vector.h: interface for the Variant class.
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

#if !defined(AFX_VECTOR_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_)
#define AFX_VECTOR_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_

#include "OS.h"

template <class T> class Vector {
public:

    Vector() {

        m_iNumElts = 0;
        m_iSpace = 0;
        m_pData = NULL;
    }

    ~Vector() {

        if (m_pData != NULL) {
            delete [] m_pData;
        }
    }

    int Init (size_t stInitElts) {
        
        if (stInitElts > 0) {

            m_pData = new T [stInitElts];
            if (m_pData == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }

            m_iSpace = stInitElts;
        }

        return OK;
    }

    int Add (const T& tData) {
        
        if (m_iNumElts == m_iSpace) {

            unsigned int iSpace = m_iSpace * 2;
            if (iSpace == 0) {
                iSpace = 5;
            }
            
            T* ptTemp = new T [iSpace];
            if (ptTemp == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }
            
            if (m_iNumElts > 0) {
                memcpy (ptTemp, m_pData, m_iNumElts * sizeof (T));
                delete [] m_pData;
            }

            m_pData = ptTemp;
            m_iSpace = iSpace;
        }

        m_pData [m_iNumElts ++] = tData;
        return OK;
    }

    const T* GetData() {
        return m_pData;
    }

    unsigned int GetNumElements() {
        return m_iNumElts;
    }
    
private:
    
    T* m_pData;

    unsigned int m_iNumElts;
    unsigned int m_iSpace;
};

#endif // !defined(AFX_VECTOR_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_)