//
// Database.dll - A database library
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

#include "CDatabase.h"

TableEnumerator::TableEnumerator() {

    m_iNumRefs = 1;
    m_iNumTables = 0;
    m_ppszTableNames = NULL;
}

TableEnumerator::~TableEnumerator() {

    if (m_ppszTableNames != NULL) {

        unsigned int i;
        for (i = 0; i < m_iNumTables; i ++) {
            if (m_ppszTableNames[i] != NULL) {
                OS::HeapFree (m_ppszTableNames[i]);
            }
        }
        delete [] m_ppszTableNames;
    }
}

int TableEnumerator::Initialize (TableHashTable* pTables) {

    unsigned int iNumTables = pTables->GetNumElements();

    if (iNumTables > 0) {

        m_ppszTableNames = new char* [iNumTables];
        if (m_ppszTableNames == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        memset (m_ppszTableNames, 0, iNumTables * sizeof (char*));
        
        m_iNumTables = iNumTables;

        HashTableIterator<const char*, Table*> htiIterator;
        unsigned int i = 0;
        while (pTables->GetNextIterator (&htiIterator)) {

            m_ppszTableNames[i] = String::StrDup (htiIterator.GetKey());
            if (m_ppszTableNames[i] == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }

            i ++;
        }

        Assert (i == iNumTables);
    }

    return OK;
}

TableEnumerator* TableEnumerator::CreateInstance (TableHashTable* pTables) {

    TableEnumerator* pEnum = new TableEnumerator();
    if (pEnum != NULL) {
        
        if (pEnum->Initialize (pTables) != OK) {
            delete pEnum;
            pEnum = NULL;
        }
    }

    return pEnum;
}

unsigned int TableEnumerator::GetNumTables() {
    return m_iNumTables;
}

const char** TableEnumerator::GetTableNames() {
    return (const char**) m_ppszTableNames;
}

TemplateEnumerator::TemplateEnumerator() {

    m_iNumRefs = 1;
    m_iNumTemplates = 0;
    m_ppszTemplateNames = NULL;
}

TemplateEnumerator::~TemplateEnumerator() {

    if (m_ppszTemplateNames != NULL) {

        unsigned int i;
        for (i = 0; i < m_iNumTemplates; i ++) {
            if (m_ppszTemplateNames[i] != NULL) {
                OS::HeapFree (m_ppszTemplateNames[i]);
            }
        }
        delete [] m_ppszTemplateNames;
    }
}

int TemplateEnumerator::Initialize (TemplateHashTable* pTemplates) {

    unsigned int iNumTemplates = pTemplates->GetNumElements();

    if (iNumTemplates > 0) {

        m_ppszTemplateNames = new char* [iNumTemplates];
        if (m_ppszTemplateNames == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        memset (m_ppszTemplateNames, 0, iNumTemplates * sizeof (char*));
        
        m_iNumTemplates = iNumTemplates;

        HashTableIterator<const char*, Template*> htiIterator;
        unsigned int i = 0;
        while (pTemplates->GetNextIterator (&htiIterator)) {

            m_ppszTemplateNames[i] = String::StrDup (htiIterator.GetKey());
            if (m_ppszTemplateNames[i] == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }
            i ++;
        }

        Assert (i == iNumTemplates);
    }

    return OK;
}

TemplateEnumerator* TemplateEnumerator::CreateInstance (HashTable<const char*, Template*, 
                                                        TemplateNameHashValue, TemplateNameEquals>* 
                                                        pTemplates) {
    TemplateEnumerator* pEnum = new TemplateEnumerator();
    if (pEnum != NULL) {
        
        if (pEnum->Initialize (pTemplates) != OK) {
            delete pEnum;
            pEnum = NULL;
        }
    }

    return pEnum;
}

unsigned int TemplateEnumerator::GetNumTemplates() {
    return m_iNumTemplates;
}

const char** TemplateEnumerator::GetTemplateNames () {
    return (const char**) m_ppszTemplateNames;
}
