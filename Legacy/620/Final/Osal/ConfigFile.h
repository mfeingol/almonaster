// ConfigFile.h: interface for the ConfigFile class.
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

#if !defined(AFX_CONFIGFILE_H__37320A13_EE6C_11D1_9D9F_0060083E8062__INCLUDED_)
#define AFX_CONFIGFILE_H__37320A13_EE6C_11D1_9D9F_0060083E8062__INCLUDED_

#include "File.h"
#include "HashTable.h"

class OSAL_EXPORT ConfigFile : protected File {

private:

    struct ConfigValue {
        char* Name;
        char* Value;
        unsigned int Index;
    };

    class ConfigHashValue {
    public:
        static unsigned int GetHashValue (const char* pszKey, unsigned int iNumBuckets, const void* pHashHint);
    };

    class ConfigEquals {
    public:
        static bool Equals (const char* pszLeft, const char* pszRight, const void* pEqualsHint);
    };

    HashTable<const char*, ConfigValue*, ConfigHashValue, ConfigEquals>* m_phtConfigHashTable;

protected:

    char** m_ppszParameterNames;
    char** m_ppszParameterValues;

    char* m_pszFileName;
    unsigned int m_iNumParameters;
    unsigned int m_iNumParametersSpace;

    int Reset();
    void Clean();
    void CleanParameters();

    int Parse();

    int AddNewParameter (const char* pszNewLhs, const char* pszNewRhs);
    int ReplaceParameter (const char* pszLhs, const char* pszNewRhs);

public:

    ConfigFile();
    ~ConfigFile();

    int Open (const char* pszFileName);
    int Close();
    int Refresh();

    unsigned int GetNumParameters() const;
    const char** GetParameterNames() const;

    int GetParameter (unsigned int iIndex, char** ppszLhs, char** ppszRhs) const;
    int GetParameter (const char* pszLhs, char** ppszRhs) const;
    
    int SetParameter (const char* pszNewLhs, const char* pszNewRhs);
};

#endif // !defined(AFX_CONFIGFILE_H__37320A13_EE6C_11D1_9D9F_0060083E8062__INCLUDED_)