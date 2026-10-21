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

#define OSAL_BUILD
#include "Crypto.h"
#undef OSAL_BUILD

#ifdef __LINUX__
#include <openssl/rand.h>
#include <openssl/evp.h>


static int InitRandom() {

    // initialize randomness
    // if system has /dev/random RAND_status() will return 1
    if (RAND_status() == 1)
        return OK;              /* already have enough randomness */

    // If don't have /dev/random, we can add random data with RAND_bytes
    // Not going to do that now
    printf("Don't have enough randomness!");
    return ERROR_FAILURE;
}

class LinuxAutoInit
{
public:

    LinuxAutoInit() {
        InitRandom();
        OpenSSL_add_all_digests();
    }

    ~LinuxAutoInit() {
    }
};
LinuxAutoInit g_autoinit;

int Crypto::GetRandomData (void* pbRand, size_t cbRand) {
    return ((RAND_bytes((unsigned char *)pbRand, cbRand) == 1) ? OK : ERROR_FAILURE);
}

Crypto::Hash::Hash (EVP_MD *md) {
    m_md = md;
    EVP_DigestInit(&m_mdctx, md);
}

Crypto::Hash::~Hash() {
}

int Crypto::Hash::HashData (const void* pbData, size_t cbData) {

    EVP_DigestUpdate(&m_mdctx, pbData, cbData);
    return OK;
}

int Crypto::Hash::GetHashSize (size_t* pstSize) {
    *pstSize = EVP_MD_size(m_md);
    return OK;
}

int Crypto::Hash::GetHash (void* pbData, size_t stSize) {
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    EVP_DigestFinal(&m_mdctx, md_value, &md_len);

    if (md_len > stSize)
        return ERROR_SMALL_BUFFER;

    memcpy(pbData, md_value, md_len);

    return OK;
}

Crypto::HashMD5::HashMD5() : Hash (EVP_md5()) {}
#endif

//
// Win32 global state
//

#ifdef __WIN32__

#include <bcrypt.h>

HCRYPTPROV g_hProv = NULL;
BCRYPT_ALG_HANDLE g_hHmacSHA256 = NULL;

class Win32AutoInit
{
public:

    Win32AutoInit()
    {
        BOOL f = CryptAcquireContext (&g_hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
        Assert (f);

        NTSTATUS status = BCryptOpenAlgorithmProvider (&g_hHmacSHA256, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG);
        Assert (BCRYPT_SUCCESS (status));
    }

    ~Win32AutoInit() {

        if (g_hProv != NULL) {
            CryptReleaseContext (g_hProv, 0);
            g_hProv = NULL;
        }

        if (g_hHmacSHA256 != NULL) {
            BCryptCloseAlgorithmProvider (g_hHmacSHA256, 0);
            g_hHmacSHA256 = NULL;
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
    BOOL f = CryptCreateHash (g_hProv, idHash, NULL, 0, &m_hHash);
    Assert (f);
}

Crypto::Hash::~Hash() {

    if (m_hHash != NULL) {
        CryptDestroyHash (m_hHash);
    }
}

int Crypto::Hash::HashData (const void* pbData, size_t cbData) {

    if (m_hHash == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    if (!CryptHashData (m_hHash, (BYTE*) pbData, (DWORD) cbData, 0)) {
        return ERROR_FAILURE;
    }
    return OK;
}

int Crypto::Hash::GetHashSize (size_t* pstSize) {

    if (m_hHash == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    DWORD dwHashLen = 0, dwSize = sizeof (dwHashLen);
    if (!CryptGetHashParam (m_hHash, HP_HASHSIZE, (BYTE*) &dwHashLen, &dwSize, 0)) {
        return ERROR_FAILURE;
    }

    *pstSize = dwHashLen;
    return OK;
}

int Crypto::Hash::GetHash (void* pbData, size_t stSize) {

    if (m_hHash == NULL) {
        return ERROR_OUT_OF_MEMORY;
    }

    DWORD dwHashLen = (DWORD) stSize;
    if (!CryptGetHashParam (m_hHash, HP_HASHVAL, (BYTE*) pbData, &dwHashLen, 0)) {

        if (GetLastError() == ERROR_MORE_DATA) {
            return ERROR_SMALL_BUFFER;
        }
        return ERROR_FAILURE;
    }
    return OK;
}

//
// Public hash classes
//

Crypto::HashMD5::HashMD5() : Hash (CALG_MD5) {}
Crypto::HashSHA1::HashSHA1() : Hash (CALG_SHA1) {}
Crypto::HashSHA256::HashSHA256() : Hash (CALG_SHA_256) {}

//
// HMAC
//

Crypto::HmacSHA256::HmacSHA256() {
    m_hHash = NULL;
}

Crypto::HmacSHA256::~HmacSHA256() {

    if (m_hHash != NULL) {
        BCryptDestroyHash (m_hHash);
    }
}

int Crypto::HmacSHA256::Initialize (const void* pbKey, size_t cbKey) {

    if (g_hHmacSHA256 == NULL || m_hHash != NULL) {
        return ERROR_FAILURE;
    }

    // CNG allocates the hash object itself, and keeps its own copy of the key
    NTSTATUS status = BCryptCreateHash (g_hHmacSHA256, &m_hHash, NULL, 0, (PUCHAR) pbKey, (ULONG) cbKey, 0);
    if (!BCRYPT_SUCCESS (status)) {
        m_hHash = NULL;
        return ERROR_FAILURE;
    }
    return OK;
}

int Crypto::HmacSHA256::HashData (const void* pbData, size_t cbData) {

    if (m_hHash == NULL) {
        return ERROR_FAILURE;
    }

    NTSTATUS status = BCryptHashData (m_hHash, (PUCHAR) pbData, (ULONG) cbData, 0);
    return BCRYPT_SUCCESS (status) ? OK : ERROR_FAILURE;
}

int Crypto::HmacSHA256::GetHash (void* pbData, size_t stSize) {

    if (m_hHash == NULL) {
        return ERROR_FAILURE;
    }

    if (stSize < HMAC_SHA256_SIZE) {
        return ERROR_SMALL_BUFFER;
    }

    NTSTATUS status = BCryptFinishHash (m_hHash, (PUCHAR) pbData, HMAC_SHA256_SIZE, 0);
    return BCRYPT_SUCCESS (status) ? OK : ERROR_FAILURE;
}

#endif // __WIN32__

// Accumulates the differences between pbActual and pbExpected over all cbExpected bytes. Bytes beyond the end
// of pbActual compare as zero, so the running time depends only on cbExpected
static unsigned int ConstantTimeDiff(const void* pbActual, size_t cbActual, const void* pbExpected, size_t cbExpected) {

    const unsigned char* pb1 = (const unsigned char*)pbActual;
    const unsigned char* pb2 = (const unsigned char*)pbExpected;

    volatile unsigned int iDiff = 0;
    for (size_t i = 0; i < cbExpected; i++) {
        unsigned char bActual = i < cbActual ? pb1[i] : 0;
        iDiff |= (unsigned char)(bActual ^ pb2[i]);
    }
    return iDiff;
}

bool Crypto::ConstantTimeEquals(const void* pbData1, const void* pbData2, size_t cbData) {
    return ConstantTimeDiff(pbData1, cbData, pbData2, cbData) == 0;
}

bool Crypto::ConstantTimeEquals(const char* pszActual, const char* pszExpected) {

    size_t cchActual = strlen(pszActual);
    size_t cchExpected = strlen(pszExpected);

    unsigned int iDiff = (cchActual != cchExpected) ? 1 : 0;
    iDiff |= ConstantTimeDiff(pszActual, cchActual, pszExpected, cchExpected);

    return iDiff == 0;
}
