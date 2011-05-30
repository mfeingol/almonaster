// Variant.h: interface for the Variant class.
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

#if !defined(AFX_VECTOR_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_)
#define AFX_VECTOR_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_

#include "OS.h"

template <class T> class OSAL_EXPORT Vector {
public:

	Vector() {

		m_stNumElts = 0;
		m_stSpace = 0;
		m_pData = NULL;
	}

	~Vector() {

		if (m_pData != NULL) {
			delete [] m_pData;
		}
	}

	int Init() {
		return OK;
	}

	int Init (size_t stInitElts) {
		
		if (stInitElts > 0) {

			m_pData = new T [stInitElts];
			if (m_pData == NULL) {
				return ERROR_OUT_OF_MEMORY;
			}

			m_stSpace = stInitElts;
		}

		return OK;
	}

	int Add (const T& tData) {
		
		if (m_stNumElts == m_stSpace) {

			size_t stSpace = m_stSpace * 2;
			if (stSpace == 0) {
				stSpace = 5;
			}
			
			T* ptTemp = new T [stSpace];
			if (ptTemp == NULL) {
				return ERROR_OUT_OF_MEMORY;
			}
			
			if (m_stNumElts > 0) {
				memcpy (ptTemp, m_pData, m_stNumElts * sizeof (T));
				delete [] m_pData;
			}

			m_pData = ptTemp;
			m_stSpace = stSpace;
		}

		m_pData [m_stNumElts ++] = tData;
	}

	T* GetData() {
		return m_pData;
	}
	
private:
	
	T* m_pData;

	size_t m_stNumElts;
	size_t m_stSpace;
};

#endif // !defined(AFX_VECTOR_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_)