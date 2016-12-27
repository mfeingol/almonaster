// ConfigFile.cpp: implementation of the ConfigFile class.
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

#define OSAL_BUILD
#include "ConfigFile.h"
#include "TempFile.h"
#include "String.h"
#undef OSAL_BUILD

#include <string.h>

#define DEFAULT_NUM_PARAMETERS_LOW 10
#define DEFAULT_NUM_PARAMETERS_HIGH 20

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ConfigFile::ConfigFile() {
    
    Reset();
}

ConfigFile::~ConfigFile() {
    
    Clean();
}

int ConfigFile::Reset() {

    m_ppszParameterNames = NULL;
    m_ppszParameterValues = NULL;
    
    m_phtConfigHashTable = NULL;
    m_pszFileName = NULL;
    m_iNumParameters = 0;
    m_iNumParametersSpace = 0;

    return OK;
}

void ConfigFile::Clean() {

    if (m_pszFileName != NULL) {
        delete [] m_pszFileName;
    }

    CleanParameters();
}

void ConfigFile::CleanParameters() {

    if (m_ppszParameterNames != NULL) {
        delete [] m_ppszParameterNames;
    }

    if (m_phtConfigHashTable != NULL) {
        
        ConfigValue* pConfig;
        HashTableIterator<const char*, ConfigValue*> htiIterator;
        
        while (m_phtConfigHashTable->GetNextIterator (&htiIterator)) {
            
            pConfig = htiIterator.GetData();
            
            OS::HeapFree (pConfig->Name);
            OS::HeapFree (pConfig->Value);
            
            delete pConfig;
        }
        
        delete m_phtConfigHashTable;
        m_phtConfigHashTable = NULL;
    }
}

int ConfigFile::Open (const char* pszFileName) {

    // Config files can be re-opened, etc.
    Clean();
    Reset();

    m_phtConfigHashTable = new HashTable<const char*, ConfigValue*, ConfigHashValue, ConfigEquals> (NULL, NULL);
    if (m_phtConfigHashTable == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (!m_phtConfigHashTable->Initialize (DEFAULT_NUM_PARAMETERS_HIGH)) {
        return ERROR_OUT_OF_MEMORY;
    }

    m_iNumParametersSpace = DEFAULT_NUM_PARAMETERS_LOW;
    m_ppszParameterNames = new char* [DEFAULT_NUM_PARAMETERS_LOW + DEFAULT_NUM_PARAMETERS_LOW];
    if (m_ppszParameterNames == NULL) {
        Clean();
        Reset();
        return ERROR_OUT_OF_MEMORY;
    }

    m_ppszParameterValues = m_ppszParameterNames + DEFAULT_NUM_PARAMETERS_LOW;

    // Save the file name
    m_pszFileName = String::StrDup (pszFileName);

    // Parse the contents
    int iErrCode = Parse();
    if (iErrCode == ERROR_FAILURE) {
        
        // Something really bad happened
        // Revert to pristine state
        Clean();
        Reset();
    }
    return iErrCode;
}

int ConfigFile::Refresh() {

    char pszFileName [OS::MaxFileNameLength];
    strcpy (pszFileName, m_pszFileName);

    Clean();
    Reset();

    return Open (pszFileName);
}

int ConfigFile::Close() {
    
    Clean();
    return Reset();
}

int ConfigFile::Parse() {

    bool bError = false;

    // Open the file
    int iErrCode, iEqualsIndex;
    
    if (File::OpenRead (m_pszFileName) != OK) {
        
        // Try to create the file
        if ((iErrCode = File::OpenWrite (m_pszFileName)) != OK) {
            return iErrCode;
        } else {
            File::Close();
            return WARNING;
        }
    }

    // Get the file's size
    size_t stLength;
    if ((iErrCode = GetSize (&stLength)) != OK) {
        File::Close();
        return iErrCode;
    }

    // Allocate space for the file
    size_t stBytesRead, stStrLength;
    char* pszBuffer = (char*) StackAlloc (stLength + 1);

    // Read from the file
    if ((iErrCode = File::Read (pszBuffer, stLength, &stBytesRead)) != OK || stLength != stBytesRead) {
        File::Close();
        return iErrCode;
    }
    pszBuffer[stBytesRead] = '\0';
    File::Close();

    char* pszLhs, * pszRhs, * pszTemp = strtok (pszBuffer, "\n");
    unsigned int i;
    while (pszTemp != NULL) {

        // Get the length of the line
        stLength = strlen (pszTemp);

        // Skip comments and empty lines
        if (stLength == 1 || (stLength > 1 && pszTemp[0] == '/' && pszTemp[1] == '/')) {
            pszTemp = strtok (NULL, "\n");
            continue;
        }

        // Search for the =
        i = 0;
        iEqualsIndex = -1;
        while (i < stLength) {
            if (pszTemp[i] == '=') {
                iEqualsIndex = i;
                break;
            }
            i ++;
        }

        // There must be an equals somewhere and the parameter name must be non-blank
        if (iEqualsIndex < 1) {
            bError = true;
            pszTemp = strtok (NULL, "\n");
            continue;
        }

        // Copy the parameter name
        pszTemp[iEqualsIndex] = '\0';
        pszLhs = pszTemp;
        
        // Copy the parameter value
        if ((unsigned int) iEqualsIndex == stLength || pszTemp[iEqualsIndex + 1] == '\r') {
            pszRhs = new char ('\0');
            if (pszRhs == NULL) {
                return ERROR_OUT_OF_MEMORY;
            }
        } else {

            pszRhs = &(pszTemp[iEqualsIndex + 1]);

            stStrLength = strlen (pszRhs);
            if (pszRhs[stStrLength - 1] == '\r') {
                pszRhs[stStrLength - 1] = '\0';
            }
        }
        
        // Add pair to list
        iErrCode = AddNewParameter (pszLhs, pszRhs);
        if (iErrCode != OK) {
            return iErrCode;
        }

        // Get next lhs
        pszTemp = strtok (NULL, "\n");
    }

    return bError ? ERROR_FAILURE : OK;
}

unsigned int ConfigFile::GetNumParameters() const {
    return m_iNumParameters;
}

const char** ConfigFile::GetParameterNames() const {
    return (const char**) m_ppszParameterNames;
}

int ConfigFile::GetParameter (unsigned int iIndex, char** ppszLhs, char** ppszRhs) const {

    if (iIndex >= m_iNumParameters) {
        *ppszLhs = NULL;
        *ppszRhs = NULL;
        return ERROR_FAILURE;
    }

    *ppszLhs = m_ppszParameterNames[iIndex];
    *ppszRhs = m_ppszParameterValues[iIndex];

    return OK;
}

int ConfigFile::GetParameter (const char* pszLhs, char** ppszRhs) const {

    if (m_iNumParameters == 0) {
        *ppszRhs = NULL;
        return ERROR_FAILURE;
    }

    ConfigValue* pConfig;
    bool bRetVal = m_phtConfigHashTable->FindFirst (pszLhs, &pConfig);

    if (!bRetVal) {
        *ppszRhs = NULL;
        return ERROR_FAILURE;
    }

    *ppszRhs = pConfig->Value;
    
    return OK;
}

int ConfigFile::SetParameter (const char* pszNewLhs, const char* pszNewRhs) {

    unsigned int i;

    bool bError = false, bNewWritten = false;

    // Re-open the file
    int iErrCode, iEqualsIndex;
    if ((iErrCode = File::OpenRead (m_pszFileName)) != OK) {
        return iErrCode;
    }

    // Get the file's size
    size_t stLength;
    if ((iErrCode = File::GetSize (&stLength)) != OK) {
        File::Close();
        return iErrCode;
    }

    // Read from the file
    size_t stBytesRead, stStrLength;

    char* pszBuffer = (char*) StackAlloc (stLength + 1);
    if ((iErrCode = File::Read (pszBuffer, stLength, &stBytesRead)) != OK || stLength != stBytesRead) {
        delete [] pszBuffer;
        File::Close();
        return iErrCode;
    }
    pszBuffer[stBytesRead] = '\0';
    File::Close();

    // Open a temp file for writing the new config file
    TempFile fOutput;
    fOutput.Open();

    char* pszLhs, * pszRhs, * pszTemp = strtok (pszBuffer, "\n");
    
    while (pszTemp != NULL) {

        // Get the length of the line
        stLength = strlen (pszTemp);

        // Skip comments and empty lines
        if (stLength == 1 || (stLength > 1 && pszTemp[0] == '/' && pszTemp[1] == '/')) {
            
            pszTemp[stLength - 1] = '\0';
            
            fOutput.Write (pszTemp);
            fOutput.WriteEndLine();
            pszTemp = strtok (NULL, "\n");
            
            continue;
        }

        // Search for the =
        i = 0;
        iEqualsIndex = -1;
        while (i < stLength) {
            if (pszTemp[i] == '=') {
                iEqualsIndex = i;
                break;
            }
            i ++;
        }

        // There must be an equals somewhere and the parameter name must be non-blank
        // If this fails, don't write to the new file
        if (iEqualsIndex < 1) {
            bError = true;
            //pszTemp = strtok (NULL, "\n");
            break;
        }

        // Get the parameter name
        pszTemp[iEqualsIndex] = '\0';
        pszLhs = pszTemp;
        
        // Get the parameter value
        if ((size_t) iEqualsIndex == stLength || pszTemp[iEqualsIndex + 1] == '\r') {
            
            pszRhs = "";
        
        } else {
            
            pszRhs = &(pszTemp[iEqualsIndex + 1]);
            
            stStrLength = strlen (pszRhs);

            if (pszRhs[stStrLength - 1] == '\r') {
                pszRhs[stStrLength - 1] = '\0';
            }
        }

        // Write pair to file
        fOutput.Write (pszLhs);
        fOutput.Write ("=");

        if (!bNewWritten && strcmp (pszLhs, pszNewLhs) == 0) {
            bNewWritten = true;
            if (pszNewRhs != NULL) {
                fOutput.Write (pszNewRhs);
            }
        } else {
            fOutput.Write (pszRhs);
        }
        fOutput.WriteEndLine();

        // Get next lhs
        pszTemp = strtok (NULL, "\n");
    }

    // Write new pair if not replaced earlier
    if (!bError) {

        if (!bNewWritten) {
            
            fOutput.Write (pszNewLhs);
            fOutput.Write ("=");
            if (pszNewRhs != NULL) {
                fOutput.Write (pszNewRhs);
            }
            
            // Add a new value
            iErrCode = AddNewParameter (pszNewLhs, pszNewRhs);
            
        } else {
            
            // Replace old value with new value
            iErrCode = ReplaceParameter (pszNewLhs, pszNewRhs);
        }

        fOutput.Close();

        if (iErrCode != OK) {
            return iErrCode;
        }

        File::DeleteFile (m_pszFileName);
        File::CopyFile (fOutput.GetName(), m_pszFileName);

        fOutput.Delete();

        return OK;
    
    } else {

        // Discard changes
        fOutput.Close();
        fOutput.Delete();

        return ERROR_FAILURE;
    }
}


int ConfigFile::AddNewParameter (const char* pszNewLhs, const char* pszNewRhs) {
    
    int iErrCode;
    bool bRetVal;

    Assert (pszNewLhs != NULL);

    if (m_iNumParameters == m_iNumParametersSpace) {
        
        // Resize
        m_iNumParametersSpace = m_iNumParameters * 2;
        
        char** ppszParameterNames = new char* [m_iNumParametersSpace * 2];
        if (ppszParameterNames == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        char** ppszParameterValues = ppszParameterNames + m_iNumParametersSpace;

        memcpy (ppszParameterNames, m_ppszParameterNames, m_iNumParameters * sizeof (char*));
        memcpy (ppszParameterValues, m_ppszParameterValues, m_iNumParameters * sizeof (char*));
        
        delete [] m_ppszParameterNames;
        
        m_ppszParameterNames = ppszParameterNames;
        m_ppszParameterValues = ppszParameterValues;
    }

    ConfigValue* pConfig = new ConfigValue;
    if (pConfig == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }
    
    pConfig->Index = m_iNumParameters;
    pConfig->Name = m_ppszParameterNames[m_iNumParameters] = String::StrDup (pszNewLhs);
    if (pConfig->Name == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto OnError;
    }

    pConfig->Value = m_ppszParameterValues[m_iNumParameters] = String::StrDup (pszNewRhs);
    if (pConfig->Value == NULL) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto OnError;
    }

    bRetVal = m_phtConfigHashTable->Insert (pConfig->Name, pConfig);
    if (!bRetVal) {
        iErrCode = ERROR_OUT_OF_MEMORY;
        goto OnError;
    }

    m_iNumParameters ++;

    return OK;

OnError:

    if (pConfig != NULL) {
        
        if (pConfig->Name != NULL) {
            OS::HeapFree (pConfig->Name);
        }

        if (pConfig->Value != NULL) {
            OS::HeapFree (pConfig->Value);
        }
        
        delete pConfig;
    }

    return iErrCode;
}

int ConfigFile::ReplaceParameter (const char* pszLhs, const char* pszNewRhs) {
    
    ConfigValue* pConfig;

    bool bRetVal = m_phtConfigHashTable->FindFirst (pszLhs, &pConfig);
    if (!bRetVal) {
        Assert (false);
        return ERROR_INVALID_ARGUMENT;
    }

    if (String::StrLen (pConfig->Value) >= String::StrLen (pszNewRhs)) {
        String::StrCpy (pConfig->Value, pszNewRhs);
    } else {

        char* pszTemp = String::StrDup (pszNewRhs);
        if (pszTemp == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        OS::HeapFree (pConfig->Value);
        pConfig->Value = m_ppszParameterValues[pConfig->Index] = pszTemp;
    }

    return OK;
}

unsigned int ConfigFile::ConfigHashValue::GetHashValue (const char* pszKey, unsigned int iNumBuckets, 
                                                        const void* pHashHint) {
    
    size_t i, stLength = strlen (pszKey), iHash = 0;

    for (i = 0; i < stLength; i ++) {
        iHash += (size_t) pszKey[i];
    }

    return (unsigned int) iHash % iNumBuckets;
}

bool ConfigFile::ConfigEquals::Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint) {
    
    // Case sensitive
    return strcmp (pszLeft, pszRight) == 0;
}