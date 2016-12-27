// PageSourceEnumerator.cpp: implementation of the PageSourceEnumeratora class.
//
//////////////////////////////////////////////////////////////////////

#include "PageSourceEnumerator.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

unsigned int PageSourceHashValue::GetHashValue (const char* pszData, unsigned int iNumBuckets, const void* pHashHint) {

    return Algorithm::GetStringHashValue (pszData, iNumBuckets, true);
}

bool PageSourceEquals::Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint) {

    return stricmp (pszLeft, pszRight) == 0;
}

PageSourceEnumerator::PageSourceEnumerator() {

    m_iNumRefs = 1;

    m_iNumPageSources = 0;
    m_ppPageSourceControls = NULL;
}


int PageSourceEnumerator::Initialize (
    HashTable<const char*, PageSource*, PageSourceHashValue, PageSourceEquals>* pPageSourceTable,
    PageSource* pDefaultPageSource) {

    unsigned int iNumPageSources = pPageSourceTable->GetNumElements() + 1;

    Assert (iNumPageSources > 0);

    m_ppPageSourceControls = new IPageSourceControl* [iNumPageSources];
    if (m_ppPageSourceControls == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    m_iNumPageSources = iNumPageSources;

    m_ppPageSourceControls[0] = pDefaultPageSource;
    pDefaultPageSource->AddRef();

    unsigned int i = 1;
    HashTableIterator<const char*, PageSource*> htiIterator;
    while (pPageSourceTable->GetNextIterator (&htiIterator)) {
        m_ppPageSourceControls[i] = htiIterator.GetData();
        m_ppPageSourceControls[i ++]->AddRef();
    }

    return OK;
}

PageSourceEnumerator::~PageSourceEnumerator() {

    unsigned int i;
    for (i = 0; i < m_iNumPageSources; i ++) {
        m_ppPageSourceControls[i]->Release();
    }

    if (m_ppPageSourceControls != NULL) {
        delete [] m_ppPageSourceControls;
    }
}

PageSourceEnumerator* PageSourceEnumerator::CreateInstance (
    HashTable<const char*, PageSource*, PageSourceHashValue, PageSourceEquals>* m_pPageSourceTable,
    PageSource* pDefaultPageSource) {

    PageSourceEnumerator* pEnum = new PageSourceEnumerator();
    if (pEnum == NULL) {
        return NULL;
    }

    if (pEnum->Initialize (m_pPageSourceTable, pDefaultPageSource) != OK) {
        delete pEnum;
        return NULL;
    }

    return pEnum;
}

unsigned int PageSourceEnumerator::GetNumPageSources() {
    return m_iNumPageSources;
}

IPageSourceControl** PageSourceEnumerator::GetPageSourceControls() {
    return m_ppPageSourceControls;
}