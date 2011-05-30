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
#include "Crypto.h"
#undef OSAL_BUILD

//
// Win32 global state
//

#ifdef __WIN32__

HCRYPTPROV g_hProv = NULL;

class Win32AutoInit
{
public:

    Win32AutoInit() {

        BOOL f = CryptAcquireContext (&g_hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
        Assert (f);
    }

    ~Win32AutoInit() {
        
        if (g_hProv != NULL) {
            CryptReleaseContext (g_hProv, 0);
            g_hProv = NULL;
        }
    }
};
Win32AutoInit g_autoinit;


//
// Functions
//

int Crypto::GetRandomData (void* pbRand, size_t cbRand) {
    return CryptGenRandom (g_hProv, (DWORD) cbRand, (BYTE*) pbRand) ? OK : ERROR_FAILURE;
}

//
// Classes
//

Crypto::Hash::Hash (ALG_ID idHash) {

    m_hHash = NULL;
    m_bHaveData = false;
    m_hData.cbData = 0;
    m_hData.pbData = NULL;

    BOOL f = CryptCreateHash (g_hProv, idHash, NULL, 0, &m_hHash);
    Assert (f);
}

Crypto::Hash::~Hash() {

    if (m_hHash != NULL) {
        CryptDestroyHash (m_hHash);
    }

    if (m_bHaveData) {
        delete [] m_hData.pbData;
    }
}

int Crypto::Hash::HashData (const void* pbData, size_t cbData) {

    if (m_hHash == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (!CryptHashData (m_hHash, (BYTE*) pbData, (DWORD) cbData, 0)) {
        return ERROR_FAILURE;
    }

    if (m_bHaveData) {
        delete [] m_hData.pbData;
        m_bHaveData = false;
        m_hData.cbData = 0;
        m_hData.pbData = NULL;
    }

    return OK;
}

int Crypto::Hash::GetHash (Crypto::HashData* phdData) {

    Assert (phdData != NULL);

    if (!m_bHaveData) {

        DWORD dwHashLen = 0;
        if (!CryptGetHashParam (m_hHash, HP_HASHVAL, NULL, &dwHashLen, 0)) {
            return ERROR_FAILURE;
        }

        Byte* pbData = new Byte [dwHashLen];
        if (pbData == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }

        if (!CryptGetHashParam (m_hHash, HP_HASHVAL, (BYTE*) pbData, &dwHashLen, 0)) {
            delete [] pbData;
            return ERROR_FAILURE;
        }

        m_hData.cbData = dwHashLen;
        m_hData.pbData = pbData;
        m_bHaveData = true;
    }

    *phdData = m_hData;
    return OK;
}

//
// Public hash classes
//

Crypto::HashMD5::HashMD5() : Hash (CALG_MD5) {}
Crypto::HashSHA::HashSHA() : Hash (CALG_SHA) {}
Crypto::HashRC4::HashRC4() : Hash (CALG_RC4) {}

#endif // __WIN32__