#if !defined(AFX_FIFOQUEUE_H__3A0AE007_086B_11D2_9E07_0060083E8062__INCLUDED_)
#define AFX_FIFOQUEUE_H__3A0AE007_086B_11D2_9E07_0060083E8062__INCLUDED_

#include "LinkedList.h"
#include "Mutex.h"

template <class T> class QueueIterator : public ListIterator<T> {};

template <class T> class FifoQueue {

protected:

    LinkedList<T> m_lList;

public:

    FifoQueue() {}
    FifoQueue (unsigned int iMaxNumCachedNodes) : m_lList (iMaxNumCachedNodes) {}

    bool Push (T tData) {
        return m_lList.PushLast (tData);
    }

    bool Pop (T* ptData) {
        
        ListIterator<T> litIter;
        
        if (m_lList.PopFirst (&litIter)) {
            *ptData = litIter.GetData();
            return true;
        } else {
            return false;
        }
    }

    unsigned int GetNumElements() {
        return m_lList.GetNumElements();
    }

    bool GetNextIterator (QueueIterator<T>* pqIterator) {
        return m_lList.GetNextIterator (pqIterator);
    }

    void Clear() {
        m_lList.Clear();
    }

    int CopyQueue (FifoQueue<T>* pfqCopy) {

        ListIterator<T> litIter;

        pfqCopy->Clear();

        bool bRetVal = m_lList.PeekFirst (&litIter);
        while (bRetVal) {

            bRetVal = pfqCopy->Push (litIter.GetData());
            if (bRetVal) {
                bRetVal = m_lList.GetNextIterator (&litIter);
            }
        }

        Assert (pfqCopy->GetNumElements() == GetNumElements());
    }
};


//
// Algorithm swiped from http://www.cs.rochester.edu/u/michael/
//
template <class T> class ThreadSafeFifoQueue {

private:
    
    template <class S> class ThreadSafeFifoQueueNode {
    public:
        S Data;
        ThreadSafeFifoQueueNode<S>* Next;
    };

    unsigned int m_iNumElements;

    ThreadSafeFifoQueueNode<T>* m_pHead;
    ThreadSafeFifoQueueNode<T>* m_pTail;

    Mutex m_mHeadLock;
    Mutex m_mTailLock;

public:

    ThreadSafeFifoQueue() {
    
        m_iNumElements = 0;
        m_pHead = m_pTail = NULL;
    }

    ~ThreadSafeFifoQueue() {

        ThreadSafeFifoQueueNode<T>* pDelete = m_pHead, * pNext;

        while (pDelete != NULL) {
            pNext = pDelete->Next;
            delete pDelete;
            pDelete = pNext;
        }
    }

    bool Initialize() {

        int iErrCode;

        iErrCode = m_mHeadLock.Initialize();
        if (iErrCode != OK) {
            return false;
        }
        iErrCode = m_mTailLock.Initialize();
        if (iErrCode != OK) {
            return false;
        }

        m_pHead = m_pTail = new ThreadSafeFifoQueueNode<T>;
        if (m_pHead == NULL) {
            return false;
        }

        m_pHead->Next = NULL;
        return true;
    }

    bool IsInitialized() {

        return m_pHead != NULL;
    }

    bool Push (T tData) {

        ThreadSafeFifoQueueNode<T>* pNode = new ThreadSafeFifoQueueNode<T>;
        if (pNode == NULL) {
            return false;
        }

        pNode->Data = tData;
        pNode->Next = NULL;

        m_mTailLock.Wait();

        m_pTail->Next = pNode;
        m_pTail = pNode;

        m_mTailLock.Signal();

        Algorithm::AtomicIncrement (&m_iNumElements);

        return true;
    }

    bool Pop (T* ptData) {

        if (m_iNumElements == 0) {
            return false;
        }
        
        m_mHeadLock.Wait();

        ThreadSafeFifoQueueNode<T>* pNode = m_pHead;
        ThreadSafeFifoQueueNode<T>* pNextHead = m_pHead->Next;

        if (pNextHead == NULL) {
            m_mHeadLock.Signal();
            return false;
        }

        *ptData = pNextHead->Data;
        m_pHead = pNextHead;

        delete pNode;

        m_mHeadLock.Signal();

        Algorithm::AtomicDecrement (&m_iNumElements);

        return true;
    }

    unsigned int GetNumElements() {
        return m_iNumElements;
    }

    void Clear() {

        T t;
        while (Pop (&t)) {}
    }
};

#endif