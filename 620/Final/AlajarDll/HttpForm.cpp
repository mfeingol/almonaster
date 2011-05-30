// HttpForm.cpp: implementation of the HttpForm class.
//
//////////////////////////////////////////////////////////////////////
//
// HttpObjects.dll:  a component of Alajar 1.0
// Copyright (C) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "HttpForm.h"

#include "Osal/TempFile.h"
#include "Osal/Algorithm.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HttpForm::HttpForm() {

    m_iNumRefs = 1;

    m_ftFormType = UNSUPPORTED_FORM_TYPE;

    m_ppSubForms = NULL;
    m_iNumSubForms = 0;
    m_iNumSubFormsSpace = 0;

    m_pszFormName = NULL;
    m_pszFormValue = NULL;
    m_pszFileName = NULL;
}

HttpForm::~HttpForm() {

    if (m_mmfFile.IsOpen()) {
        m_mmfFile.Close();
    }

    if (m_pszFormValue != NULL) {

        if (m_ftFormType == FILE_FORM) {
            // Best effort temp file delete
            File::DeleteFile (m_pszFormValue);
        }

        else if (m_ftFormType == LARGE_SIMPLE_FORM) {
            // Best effort temp file delete
            File::DeleteFile (m_pszFileName);
        }
    }

    if (m_pszFormName != NULL) {
        delete [] m_pszFormName;
    }

    // m_pszFormValue is part of name or a memory mapped file pointer

    if (m_pszFileName != NULL) {
        OS::HeapFree (m_pszFileName);
    }

    if (m_iNumSubForms != 0) {

        for (unsigned int i = 0; i < m_iNumSubForms; i ++) {
            m_ppSubForms[i]->Release();
        }
        delete [] m_ppSubForms;
    }
}

int HttpForm::Initialize (HttpFormType ftType, const char* pszFormName, const char* pszFormValue, 
                          const char* pszFileName, bool bMultipart) {

    int iErrCode;

    MemoryMappedFile mmfWWWFile;
    TempFile tFinalFile;

    // Save type
    m_ftFormType = ftType;

    Assert (pszFormName != NULL);

    // Special case for large simple forms
    if (ftType == LARGE_SIMPLE_FORM) {

        Assert (pszFormValue != NULL);
        Assert (pszFileName == NULL);

        // Create a new temp file
        iErrCode = tFinalFile.Open();
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Map the web text file
        iErrCode = mmfWWWFile.OpenExisting (pszFormValue, true);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        pszFileName = tFinalFile.GetName();
        tFinalFile.Close();

        // Map the file file
        iErrCode = m_mmfFile.OpenNew (pszFileName, mmfWWWFile.GetSize(), 0);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        m_pszFormValue = (char*) m_mmfFile.GetAddress();

        // Convert the text
        iErrCode = Algorithm::UnescapeString (
            (const char*) mmfWWWFile.GetAddress(),
            m_pszFormValue,
            mmfWWWFile.GetSize()
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

Cleanup:

        if (mmfWWWFile.IsOpen()) {
            mmfWWWFile.Close();
        }

        if (tFinalFile.IsOpen()) {
            tFinalFile.Close();
        }

        File::DeleteFile (pszFormValue);
        pszFormValue = NULL;

        if (iErrCode != OK) {

            if (m_mmfFile.IsOpen()) {
                m_mmfFile.Close();
            }

            return iErrCode;
        }
    }

    size_t stLen1 = strlen (pszFormName) + 1;
    size_t stLen2 = String::StrLen (pszFormValue) + 1;

    m_pszFormName = new char [stLen1 + stLen2];
    if (m_pszFormName == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    if (bMultipart) {
        
        memcpy (m_pszFormName, pszFormName, stLen1);

    } else {

        iErrCode = Algorithm::UnescapeString (pszFormName, m_pszFormName, stLen1);
        if (iErrCode != OK) {
            return iErrCode;
        }
    } 

    if (pszFormValue != NULL) {

        m_pszFormValue = m_pszFormName + stLen1;

        if (bMultipart) {
            
            memcpy (m_pszFormValue, pszFormValue, stLen2);
            
        } else {
            
            iErrCode = Algorithm::UnescapeString (pszFormValue, m_pszFormValue, stLen2);
            if (iErrCode != OK) {
                return iErrCode;
            }
        }
    }

    if (pszFileName != NULL) {

        m_pszFileName = String::StrDup (pszFileName);
        if (m_pszFileName == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
    }

    return OK;
}

HttpForm* HttpForm::CreateInstance (HttpFormType ftType, const char* pszFormName, const char* pszFormValue, 
                                    const char* pszFileName, bool bMultipart) {

    HttpForm* pHttpForm = new HttpForm();
    if (pHttpForm == NULL) {
        return NULL;
    }

    if (pHttpForm->Initialize (ftType, pszFormName, pszFormValue, pszFileName, bMultipart) != OK) {
        delete pHttpForm;
        return NULL;
    }

    return pHttpForm;
}

int HttpForm::AddForm (HttpForm* pHttpForm) {
    
    if (m_iNumSubForms == 0) {
        
        m_ppSubForms = new HttpForm* [5];
        if (m_ppSubForms == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
        
        m_iNumSubFormsSpace = 5;
        
    } else {
        
        // Realloc?
        if (m_iNumSubForms == m_iNumSubFormsSpace) {
            
            m_iNumSubFormsSpace *= 2;
            
            HttpForm** ppSubForms = new HttpForm* [m_iNumSubFormsSpace];
            if (ppSubForms == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }
            
            memcpy (ppSubForms, m_ppSubForms, m_iNumSubForms * sizeof (HttpForm*));
            
            delete [] m_ppSubForms;
            m_ppSubForms = ppSubForms;
        }
    }
    
    m_ppSubForms[m_iNumSubForms] = pHttpForm;
    m_iNumSubForms ++;

    return OK;
}

IHttpForm* HttpForm::GetForm (unsigned int iIndex) {

    if (iIndex >= m_iNumSubForms + 1) {
        return NULL;
    }
    
    return iIndex == 0 ? this : m_ppSubForms[iIndex - 1];
}

unsigned int HttpForm::GetNumForms() {
    return m_iNumSubForms + 1;
}


// Data
const char* HttpForm::GetName() {
    return m_pszFormName;
}

const char* HttpForm::GetValue() {
    return m_pszFormValue;
}

int HttpForm::GetIntValue() {
    return String::AtoI (m_pszFormValue);
}

unsigned int HttpForm::GetUIntValue() {
    return String::AtoUI (m_pszFormValue);
}

float HttpForm::GetFloatValue() {
    return String::AtoF (m_pszFormValue);
}

UTCTime HttpForm::GetTimeValue() {

    UTCTime tTime;
    Time::AtoUTCTime (m_pszFormValue, &tTime);
    return tTime;
}

int64 HttpForm::GetInt64Value() {
    return String::AtoI64 (m_pszFormValue);
}

uint64 HttpForm::GetUInt64Value() {
    return String::AtoUI64 (m_pszFormValue);
}

const char* HttpForm::GetFileName() {
    return m_pszFileName;
}

HttpFormType HttpForm::GetType() {
    
    if (m_ftFormType == LARGE_SIMPLE_FORM) {
        return SIMPLE_FORM;
    }
    return m_ftFormType;
}