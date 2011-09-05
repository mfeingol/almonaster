#if !defined(AFX_HASHTABLE_H__3A0AE007_086B_11D2_9E07_0060083E8062__INCLUDED_)
#define AFX_HASHTABLE_H__3A0AE007_086B_11D2_9E07_0060083E8062__INCLUDED_

#include "LinkedList.h"

template <class T> class GenericHashValue {
public:
    static unsigned int GetHashValue (T t, unsigned int iNumBuckets, const void* pHashHint) {
        return (unsigned int) t % iNumBuckets;
    }
};

template <class T> class GenericEquals {
public:
    static bool Equals (T t1, T t2, const void* pEqualsHint) {
        return t1 == t2;
    }
};

template <class CKey, class CData> class HashTableNode {
public:
    CData Data;
    CKey Key;

    static HashTableNode<CKey, CData>* New() { return new HashTableNode<CKey, CData>(); }
    static void Delete (HashTableNode<CKey, CData>* pNode) { delete pNode; }
};

template <class CKey, class CData, class HashValue, class Equals> class HashTable;
template <class CKey, class CData> class HashTableIterator : public ListIterator<HashTableNode<CKey, CData>*> {

public:

    unsigned int Bucket;

    HashTableIterator() : Bucket (0) {}

    CData& GetData() {
        return ListIterator<HashTableNode<CKey, CData>*>::GetData()->Data;
    }
    
    CKey& GetKey() {
        return ListIterator<HashTableNode<CKey, CData>*>::GetData()->Key;
    }

    void Reset() {
        ListIterator<HashTableNode<CKey, CData>*>::Reset();
        Bucket = 0;
    }

    void Freeze() {
        ListIterator<HashTableNode<CKey, CData>*>::Reset();
    }
};

template <class CKey, class CData, class HashValue, class Equals> class HashTable {
protected:

    unsigned int m_iNumElements;
    unsigned int m_iNumBuckets;

    const void* m_pHashHint;
    const void* m_pEqualsHint;

    ObjectCache <HashTableNode<CKey, CData>, HashTableNode<CKey, CData> > m_ocNodeCache;
    ::LinkedList<HashTableNode<CKey, CData>*>* m_pBuckets;

    HashTable& operator=(HashTable& htCopy);

public:

    HashTable (void* pEqualsHint, void* pHashHint) : 
      m_pEqualsHint (pEqualsHint),
      m_pHashHint (pHashHint)
    {
        m_iNumBuckets = 0;
        m_iNumElements = 0;
        m_pBuckets = NULL;
    }

    ~HashTable() {

        ListIterator<HashTableNode<CKey, CData>*> liIterator;

        for (unsigned int i = 0; i < m_iNumBuckets; i ++) {  
            while (m_pBuckets[i].PopFirst (&liIterator)) {
                delete liIterator.GetData();
            }
        }

        if (m_pBuckets != NULL) {
            delete [] m_pBuckets;
        }
    }

    bool Initialize (unsigned int iNumBuckets) {

        if (!m_ocNodeCache.Initialize (50)) {
            return false;
        }

        m_iNumBuckets = iNumBuckets;
        m_pBuckets = new ::LinkedList<HashTableNode<CKey, CData>*> [m_iNumBuckets];
        return m_pBuckets != NULL;
    }

    bool IsInitialized() {
        return m_pBuckets != NULL;
    }

    bool Insert (CKey key, CData data) {

        unsigned int iBucket = HashValue::GetHashValue (key, m_iNumBuckets, m_pHashHint);

        HashTableNode<CKey, CData>* phtNode = m_ocNodeCache.GetObject();
        phtNode->Key = key;
        phtNode->Data = data;
        m_pBuckets[iBucket].PushLast (phtNode);

        m_iNumElements ++;

        return true;
    }

    bool DeleteFirst (CKey key, CKey* pcKey, CData* pcData) {

        unsigned int iBucket = HashValue::GetHashValue (key, m_iNumBuckets, m_pHashHint);

        HashTableNode<CKey, CData>* phtNode;
        ListIterator<HashTableNode<CKey, CData>*> liIterator;

        while (m_pBuckets[iBucket].GetNextIterator (&liIterator)) {

            if (Equals::Equals (liIterator.GetData()->Key, key, m_pEqualsHint)) {

                bool bDeleted = m_pBuckets[iBucket].Delete (&liIterator, &phtNode);

                if (!bDeleted) {
                    return false;
                }
                    
                if (pcKey != NULL) {
                    *pcKey = phtNode->Key;
                }
                
                if (pcData != NULL) {
                    *pcData = phtNode->Data;
                }
                
                m_iNumElements --;
                
                m_ocNodeCache.ReleaseObject (phtNode);
                return true;
            }
        }

        return false;
    }

    bool Contains(const CKey key)
    {
        return FindFirst(key, (CData*)NULL);
    }

    bool FindFirst (CKey key, CData* pcData) {

        unsigned int iBucket = HashValue::GetHashValue (key, m_iNumBuckets, m_pHashHint);

        ListIterator<HashTableNode<CKey, CData>*> liIterator;

        while (m_pBuckets[iBucket].GetNextIterator (&liIterator)) {

            if (Equals::Equals (liIterator.GetData()->Key, key, m_pEqualsHint)) {

                if (pcData != NULL) {
                    *pcData = liIterator.GetData()->Data;
                }

                return true;
            }
        }

        return false;
    }

    bool FindFirst (CKey key, HashTableIterator<CKey, CData>* phtIterator) {

        unsigned int iBucket = HashValue::GetHashValue (key, m_iNumBuckets, m_pHashHint);
        ListIterator<HashTableNode<CKey, CData>*> liIterator;

        while (m_pBuckets[iBucket].GetNextIterator (&liIterator)) {

            if (Equals::Equals (liIterator.GetData()->Key, key, m_pEqualsHint)) {

                phtIterator->Bucket = iBucket;

                phtIterator->Data = liIterator.Data;
                phtIterator->Next = liIterator.Next;
                phtIterator->Prev = liIterator.Prev;
                phtIterator->Current = liIterator.Current;

                return true;
            }
        }

        return false;
    }

    bool FindNext (HashTableIterator<CKey, CData>* phtIterator) {

        unsigned int iBucket = phtIterator->Bucket;
        Assert (iBucket < m_iNumBuckets);

        CKey key = phtIterator->GetKey();

        while (m_pBuckets[iBucket].GetNextIterator (phtIterator)) {

            if (Equals::Equals (phtIterator->GetKey(), key, m_pEqualsHint)) {
                return true;
            }
        }

        return false;
    }

    bool GetNextIterator (HashTableIterator<CKey, CData>* phtIterator) {
        
        if (m_iNumElements == 0) {
            phtIterator->Next = NULL;
            return false;
        }

        unsigned int iBucket = phtIterator->Bucket;
        if (iBucket >= m_iNumBuckets) {
            Assert (false);
            return false;
        }

        if (m_pBuckets[iBucket].GetNextIterator (phtIterator)) {
            return true;
        }

        // Next bucket
        iBucket ++;

        while (iBucket < m_iNumBuckets) {

            // Reset the iterator
            phtIterator->Reset();

            if (m_pBuckets[iBucket].GetNextIterator (phtIterator)) {
                phtIterator->Bucket = iBucket;
                return true;
            }

            // Next bucket
            iBucket ++;
        }

        return false;
    }

    bool Delete (HashTableIterator<CKey, CData>* phtIterator, CKey* pKey, CData* pData) {

        if (m_iNumElements == 0 || phtIterator->Current == NULL) {
            return false;
        }

        unsigned int iBucket = phtIterator->Bucket;
        if (iBucket >= m_iNumBuckets) {
            Assert (false);
            return false;
        }

        HashTableNode<CKey, CData>* phtNode;

        if (!m_pBuckets[iBucket].Delete (phtIterator, &phtNode)) {
            return false;
        }

        if (pKey != NULL) {
            *pKey = phtNode->Key;
        }
        if (pData != NULL) {
            *pData = phtNode->Data;
        }
        
        m_iNumElements --;
        
        // Reset iterator to first element of next populated bucket
        if (phtIterator->Current == NULL) {
            
            iBucket ++;
            
            while (iBucket < m_iNumBuckets) {
                
                if (m_pBuckets[iBucket].PeekFirst (phtIterator)) {
                    phtIterator->Bucket = iBucket;
                    break;
                } else {
                    iBucket ++;
                }
            }
        }
        
        // Free deleted node
        m_ocNodeCache.ReleaseObject (phtNode);
        
        return true;
    }

    void Clear() {

        unsigned int i;

        ListIterator<HashTableNode<CKey, CData>*> liIterator;

        for (i = 0; i < m_iNumBuckets; i ++) {  
            while (m_pBuckets[i].PopFirst (&liIterator)) {
                delete liIterator.GetData();
            }
        }

        for (i = 0; i < m_iNumBuckets; i ++) {
            m_pBuckets[i].Clear();
        }

        m_iNumElements = 0;
    }

    unsigned int GetNumElements() {
        return m_iNumElements;
    }

    unsigned int GetNumBuckets() {
        return m_iNumBuckets;
    }

    bool CopyHashTable (HashTable<CKey, CData, HashValue, Equals>* pCopy) {

        pCopy->Clear();

        HashTableIterator<CKey, CData> htIterator;

        while (GetNextIterator (&htIterator)) {

            bool bRetVal = pCopy->Insert (htIterator.GetKey(), htIterator.GetData());
            if (!bRetVal) {
                return false;
            }
        }

        return true;
    }    
};

#endif