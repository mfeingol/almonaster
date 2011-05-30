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
#include "Template.h"

int Database::CreateTemplate (const TemplateDescription& ttTemplate) {

    bool bFlag;
    int iErrCode;

    Template* pOldTemplate, * pTemplate;

    m_rwGlobalLock.WaitReader();

    // Create the template
    pTemplate = new Template (this);
    if (pTemplate == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

    iErrCode = pTemplate->Create (ttTemplate);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    // Try to add template
    m_rwTemplateLock.WaitReaderWriter();

    bFlag = m_pTemplates->FindFirst (pTemplate->GetName(), &pOldTemplate);
    if (bFlag) {
        m_rwTemplateLock.SignalReaderWriter();
        iErrCode = ERROR_TEMPLATE_ALREADY_EXISTS;
        goto Cleanup;
    }

    m_rwTemplateLock.UpgradeReaderWriter();

    bFlag = m_pTemplates->Insert (pTemplate->GetName(), pTemplate);

    m_rwTemplateLock.DowngradeReaderWriter();
    m_rwTemplateLock.SignalReaderWriter();

    if (!bFlag) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto Cleanup;
    }

Cleanup:

    if (iErrCode != OK && pTemplate != NULL) {
        pTemplate->DeleteOnDisk();
        pTemplate->Release();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}


int Database::DeleteTemplate (const char* pszTemplateName) {

    int iErrCode = OK;
    Template* pTemplate = NULL;

    m_rwGlobalLock.WaitReader();

    // Lock table
    m_rwTemplateLock.WaitWriter();
    bool bDeleted = m_pTemplates->DeleteFirst (pszTemplateName, NULL, &pTemplate);
    m_rwTemplateLock.SignalWriter();

    if (bDeleted) {

        pTemplate->DeleteOnDisk();
        pTemplate->Release();

    } else {

        Assert (false);
        iErrCode = ERROR_UNKNOWN_TEMPLATE_NAME;
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

int Database::GetTemplate (const char* pszTemplateName, ITemplate** ppTemplate) {

    int iErrCode = ERROR_UNKNOWN_TEMPLATE_NAME;
    *ppTemplate = NULL;

    m_rwGlobalLock.WaitReader();

    Template* pTemplate = FindTemplate (pszTemplateName);
    if (pTemplate != NULL) {
        iErrCode = OK;
        *ppTemplate = pTemplate;
        pTemplate->AddRef();
    }

    m_rwGlobalLock.SignalReader();

    return iErrCode;
}

Template* Database::FindTemplate (const char* pszTemplateName) {

    Template* pTemplate = NULL;

    m_rwTemplateLock.WaitReader();

    if (m_pTemplates->FindFirst (pszTemplateName, &pTemplate)) {
        pTemplate->AddRef();
    }

    m_rwTemplateLock.SignalReader();

    return pTemplate;
}

bool Database::IsTemplateEqual (const char* pszTemplateName, const TemplateDescription& ttTemplate) {

    bool bRetVal = false;

    m_rwGlobalLock.WaitReader();

    Template* pTemplate = FindTemplate (pszTemplateName);
    if (pTemplate != NULL) {
        bRetVal = pTemplate->IsEqual (ttTemplate);
        pTemplate->Release();
    }

    m_rwGlobalLock.SignalReader();

    return bRetVal;
}

bool Database::DoesTemplateExist (const char* pszTemplateName) {

    Template* pTemplate;

    m_rwGlobalLock.WaitReader();
    m_rwTemplateLock.WaitReader();
    
    bool bRetVal = m_pTemplates->FindFirst (pszTemplateName, &pTemplate);

    m_rwTemplateLock.SignalReader();
    m_rwGlobalLock.SignalReader();

    return bRetVal;
}

unsigned int TemplateNameHashValue::GetHashValue (const char* pszString, unsigned int iNumBuckets, const void* pvHashHint) {
    
    return Algorithm::GetStringHashValue (pszString, iNumBuckets, true);
}

bool TemplateNameEquals::Equals (const char* pszLeft, const char* pszRight, const void* pvEqualsHint) {

    return String::StriCmp (pszLeft, pszRight) == 0;
}