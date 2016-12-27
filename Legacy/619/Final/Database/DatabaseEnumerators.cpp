//
// Database.dll - A database library
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

#define DATABASE_BUILD
#include "CDatabase.h"
#undef DATABASE_BUILD

TableEnumerator::TableEnumerator (HashTable<const char*, Table*, TableNameHashValue, TableNameEquals>* pTables) {

	m_iNumRefs = 1;

	m_iNumTables = pTables->GetNumElements();

	if (m_iNumTables == 0) {
		m_ppszTableNames = NULL;
	} else {
		m_ppszTableNames = new char* [m_iNumTables];
		
		HashTableIterator<const char*, Table*> htiIterator;
		unsigned int i = 0;
		while (pTables->GetNextIterator (&htiIterator)) {
			m_ppszTableNames[i ++] = String::StrDup (htiIterator.GetKey());
		}

		Assert (i == m_iNumTables);
	}
}

TableEnumerator::~TableEnumerator() {

	if (m_iNumTables > 0) {
		unsigned int i;
		for (i = 0; i < m_iNumTables; i ++) {
			OS::HeapFree (m_ppszTableNames[i]);
		}
		delete [] m_ppszTableNames;
	}
}

TableEnumerator* TableEnumerator::CreateInstance (HashTable<const char*, Table*, TableNameHashValue, TableNameEquals>* pTables) {

	return new TableEnumerator (pTables);
}

unsigned int TableEnumerator::GetNumTables() {
	return m_iNumTables;
}

const char** TableEnumerator::GetTableNames() {
	return (const char**) m_ppszTableNames;
}

TemplateEnumerator::TemplateEnumerator (HashTable<const char*, Template*, TemplateNameHashValue, TemplateNameEquals>* pTemplates) {

	m_iNumRefs = 1;

	m_iNumTemplates = pTemplates->GetNumElements();

	if (m_iNumTemplates == 0) {
		m_ppszTemplateNames = NULL;
	} else {
		m_ppszTemplateNames = new char* [m_iNumTemplates];
		
		HashTableIterator<const char*, Template*> htiIterator;
		unsigned int i = 0;
		while (pTemplates->GetNextIterator (&htiIterator)) {
			m_ppszTemplateNames[i ++] = String::StrDup (htiIterator.GetKey());
		}

		Assert (i == m_iNumTemplates);
	}
}

TemplateEnumerator::~TemplateEnumerator() {

	if (m_iNumTemplates > 0) {
		unsigned int i;
		for (i = 0; i < m_iNumTemplates; i ++) {
			OS::HeapFree (m_ppszTemplateNames[i]);
		}
		delete [] m_ppszTemplateNames;
	}
}

TemplateEnumerator* TemplateEnumerator::CreateInstance (HashTable<const char*, Template*, 
														TemplateNameHashValue, TemplateNameEquals>* 
														pTemplates) {

	return new TemplateEnumerator (pTemplates);
}

unsigned int TemplateEnumerator::GetNumTemplates() {
	return m_iNumTemplates;
}

const char** TemplateEnumerator::GetTemplateNames () {
	return (const char**) m_ppszTemplateNames;
}
