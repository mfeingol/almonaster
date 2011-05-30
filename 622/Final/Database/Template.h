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

#if !defined(AFX_TEMPLATE_H__05370B5C_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)
#define AFX_TEMPLATE_H__05370B5C_2751_11D3_A0DC_0050047FE2E2__INCLUDED_

#include "Osal/File.h"

#include "CDatabase.h"

class Database;

class Template : public ITemplate {

private:

    bool m_bDelete;

    Database* m_pDatabase;

    Offset m_oBaseOffset;
    void* m_pDataBlock;

    static int VerifyIndices (const TemplateDescription& ttTemplate);
    static bool HasBadTypes (unsigned int iNumColumns, VariantType* pvtType);

    int FinalConstruct (size_t stBlockSize, const char* pszName, size_t stNameLen,
        unsigned int* piIndices, unsigned int* piIndexFlags, VariantType* pvtTypes, int64* pcbSizes);

public:

    Template (Database* pDatabase);
    ~Template();

    int Reload (Offset oTemplate);
    int Create (const TemplateDescription& ttTemplate);

    inline void DeleteOnDisk() { m_bDelete = true; }

    // IObject
    IMPLEMENT_INTERFACE (ITemplate);

    static int VerifyTemplate (const TemplateDescription& ttTemplate);
    bool IsEqual (const TemplateDescription& ttTemplate);

    // ITemplate
    int GetDescription (TemplateDescription* pttTemplate);

    inline const char* GetName() { return TemplateData.Name; }

    TemplateDescription TemplateData;

    // Extra Data
    Size RowSize;
    Offset* ColOffset;

    bool HasVariableLengthData;
    bool HasUniqueDataIndex;
};

#endif // !defined(AFX_TEMPLATE_H__05370B5C_2751_11D3_A0DC_0050047FE2E2__INCLUDED_)