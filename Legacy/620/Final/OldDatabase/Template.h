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

#if !defined(AFX_TEMPLATE_H__05370B5C_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)
#define AFX_TEMPLATE_H__05370B5C_2751_11D3_A0DC_0050047FE2E2__INCLUDED_

#include "Osal/File.h"

#define DATABASE_BUILD
#include "Database.h"
#undef DATABASE_BUILD

#define BASE_ELEMENT_SIZE 4
#define MAX_ELEMENT_SIZE 4

class Database;

class Template : public ITemplate {

private:

    bool m_bDelete;

    Database* m_pDatabase;

    Template (const char* pszName, Database* pDatabase);
    Template (const TemplateDescription& ttTemplate, Database* pDatabase);
    ~Template();

    static bool HasRepeatedIndices (unsigned int iNumIndices, unsigned int* piColumns);
    static bool HasBadTypes (unsigned int iNumColumns, VariantType* pvtType);

public:

    static Template* CreateInstance (const char* pszName, Database* pDatabase);
    static Template* CreateInstance (const TemplateDescription& ttTemplate, Database* pDatabase);

    static int VerifyTemplate (const TemplateDescription& ttTemplate);

    const char* GetDirectory();
    const char* GetDataDirectory();

    void IncrementTableSizeOnDisk (size_t stChange);
    void DecrementTableSizeOnDisk (size_t stChange);

    void IncrementNumLoadedRows (unsigned int iNumRows);
    void DecrementNumLoadedRows (unsigned int iNumRows);

    unsigned int GetOptions();

    // IObject
    IMPLEMENT_INTERFACE (ITemplate);

    bool IsEqual (const TemplateDescription& ttTemplate);

    // ITemplate
    int GetDescription (TemplateDescription* pttTemplate);

    int Reload (const char* pszFileName);
    int Create (const char* pszDirName);

    int Backup (const char* pszBackupDir);

    inline void DeleteOnDisk() { m_bDelete = true; }
    inline const char* GetName() { return TemplateData.Name; }

    TemplateDescription TemplateData;

    // Extra Data
    size_t RowSize;
    size_t* Offset;

    bool VariableLengthRows;
    unsigned int NumInitialInsertionLengthRows;
};

#endif // !defined(AFX_TEMPLATE_H__05370B5C_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)