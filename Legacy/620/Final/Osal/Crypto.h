// Crypto.h: interface for the Variant class.
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

#if !defined(AFX_CRYPTO_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_)
#define AFX_CRYPTO_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_

#include "OS.h"

#ifdef __LINUX__
#include <openssl/evp.h>
#endif

namespace Crypto {

    //
    // Functions
    //
    OSAL_EXPORT int GetRandomData (void* pbRand, size_t cbRand);

    //
    // Classes
    //

#if defined __LINUX__
    class OSAL_EXPORT Hash {
    private:
        EVP_MD_CTX m_mdctx;
        EVP_MD *m_md;

    public:
        Hash (EVP_MD *md);
        ~Hash();

        int HashData (const void* pbData, size_t cbData);

        int GetHashSize (size_t* pstSize);
        int GetHash (void* pbData, size_t stSize);
    };

    class OSAL_EXPORT HashMD5 : public Hash {
    public:
        HashMD5();
    };

#else if defined __WIN32__

    class OSAL_EXPORT Hash {
    private:
        HCRYPTHASH m_hHash;

    public:
        Hash (ALG_ID idHash);
        ~Hash();

        int HashData (const void* pbData, size_t cbData);

        int GetHashSize (size_t* pstSize);
        int GetHash (void* pbData, size_t stSize);
    };

    class OSAL_EXPORT HashMD5 : public Hash {
    public:
        HashMD5();
    };

    class OSAL_EXPORT HashRC4 : public Hash {
    public:
        HashRC4();
    };
#endif
};


#endif // !defined(AFX_CRYPTO_H__C5833FE7_CE47_11D1_9D09_0060083E8062__INCLUDED_)
