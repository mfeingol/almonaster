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

#if !defined(AFX_LINKEDLIST_H__3A0AE007_086B_11D2_9E07_0060083E8062__INCLUDED_)
#define AFX_LINKEDLIST_H__3A0AE007_086B_11D2_9E07_0060083E8062__INCLUDED_

#include "OS.h"
#include "ObjectCache.h"

template <class T> class OSAL_EXPORT ListNode {
public:

	T Data;
	ListNode<T>* Next;
	ListNode<T>* Prev;

	static ListNode<T>* New() { return new ListNode<T>(); }
	static void Delete (ListNode<T>* pNode) { delete pNode; }
};

#define FIRST_NODE_PTR ((ListNode<T>*) 0xffffffff)

template <class T> class OSAL_EXPORT LinkedList;

template <class T> class OSAL_EXPORT ListIterator {
public:

	ListNode<T>* Next;
	ListNode<T>* Prev;
	ListNode<T>* Current;
	T Data;
	
	ListIterator() {
		Reset();
	}
	T& GetData() { return Data; }

	void Reset() {
		Next = FIRST_NODE_PTR;
		Prev = NULL;
		Current = FIRST_NODE_PTR;
	}
};


template <class T> class OSAL_EXPORT LinkedList {
protected:

	ListNode<T>* m_plnHead;
	ListNode<T>* m_plnTail;

	unsigned int m_iNumElements;

public:
	
	LinkedList() {
		m_plnHead = NULL;
		m_plnTail = NULL;
		m_iNumElements = 0;
	}

	~LinkedList() {

		// Clean up active nodes
		ListNode<T>* pNode = m_plnHead, *pTemp;
		while (pNode != NULL) {
			pTemp = pNode->Next;
			delete pNode;
			pNode = pTemp;
		}
	}

	bool PushFirst (T& tData) {
		
		ListNode<T>* pNode = new ListNode<T>;
		if (pNode == NULL) {
			return false;
		}

		pNode->Data = tData;
		pNode->Prev = NULL;

		if (m_plnHead != NULL) {
			pNode->Next = m_plnHead;
			m_plnHead = pNode;
		} else {
			m_plnHead = pNode;
			m_plnTail = m_plnHead;
			m_plnTail->Next = NULL;
		}
		m_iNumElements ++;

		return true;
	}

	bool PushLast (T& tData) {
		
		ListNode<T>* pNode = new ListNode<T>;
		if (pNode == NULL) {
			return false;
		}

		pNode->Data = tData;

		if (m_plnTail != NULL) {
			m_plnTail->Next = pNode;
			pNode->Prev = m_plnTail;
			m_plnTail = pNode;
		} else {
			m_plnHead = pNode;
			m_plnHead->Prev = NULL;
			m_plnTail = m_plnHead;
		}
		m_plnTail->Next = NULL;
		m_iNumElements ++;

		return true;
	}

	bool PopFirst (ListIterator<T>* pliIterator) {

		pliIterator->Next = NULL;
		pliIterator->Prev = NULL;
		pliIterator->Current = m_plnHead;

		if (m_plnHead == NULL) {
			return false;
		}

		ListNode<T>* pNode = m_plnHead;
		m_plnHead = m_plnHead->Next;

		if (m_plnHead != NULL) {
			m_plnHead->Prev = NULL;
		}
		pliIterator->Data = pNode->Data;
		delete pNode;

		if (m_plnHead == NULL) {
			m_plnTail = NULL;
		}

		m_iNumElements --;

		return true;
	}

	bool PeekFirst (ListIterator<T>* pliIterator) {

		ListNode<T>* pHead = m_plnHead;

		pliIterator->Current = pHead;
		pliIterator->Prev = NULL;

		if (pHead == NULL) {
			pliIterator->Next = NULL;
			return false;
		}

		pliIterator->Next = pHead->Next;
		pliIterator->Data = pHead->Data;
		return true;
	}

	bool Delete (ListIterator<T>* pliIterator, T* pData) {

		if (m_iNumElements == 0 || pliIterator->Current == NULL) {
			return false;
		}

		ListNode<T>* pDelNode = pliIterator->Current;

		if (pliIterator->Prev != NULL) {

			pliIterator->Prev->Next = pliIterator->Next;
			if (pliIterator->Next != NULL) {
				pliIterator->Next->Prev = pliIterator->Prev;
			}

		} else {

			m_plnHead = pliIterator->Next;
			
			if (pliIterator->Next != NULL) {
				m_plnHead->Prev = NULL;
			}
		}

		if (pData != NULL) {
			*pData = pliIterator->GetData();
		}

		if (pliIterator->Next == NULL) {

			m_plnTail = pliIterator->Prev;
			
			pliIterator->Current = NULL;
			pliIterator->Next = NULL;
			pliIterator->Prev = NULL;

		} else {

			pliIterator->Current = pliIterator->Next;
			pliIterator->Data = pliIterator->Current->Data;
			pliIterator->Next = pliIterator->Next->Next;
		}

		delete pDelNode;
		m_iNumElements --;

		return true;
	}	


	bool GetNextIterator (ListIterator<T>* pliIterator) {
		
		if (m_plnHead == NULL || pliIterator->Current == NULL) {
			pliIterator->Next = NULL;
			pliIterator->Prev = NULL;
			return false;
		}

		if (pliIterator->Current == FIRST_NODE_PTR) {
			pliIterator->Current = m_plnHead;
			pliIterator->Data = m_plnHead->Data;
			pliIterator->Next = m_plnHead->Next;
			pliIterator->Prev = NULL;
			return true;
		}
		
		if (pliIterator->Prev == NULL) {
			pliIterator->Prev = m_plnHead;
		} else {
			pliIterator->Prev = pliIterator->Current;
		}
		pliIterator->Current = pliIterator->Next;

		if (pliIterator->Current != NULL) {
			pliIterator->Data = pliIterator->Current->Data;
			pliIterator->Next = pliIterator->Current->Next;
		} else {
			return false;
		}

		return true;
	}

	unsigned int GetNumElements() const {
		return m_iNumElements;
	}

	void Clear() {

		ListNode<T>* pNode = m_plnHead, *pTemp;
		while (pNode != NULL) {
			pTemp = pNode->Next;
			delete pNode;
			pNode = pTemp;
		}

		m_plnHead = NULL;
		m_plnTail = NULL;
		m_iNumElements = 0;
	}
};

#endif