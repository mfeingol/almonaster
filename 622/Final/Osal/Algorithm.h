// Algorithm.h: interface for the Algorithm class.
//
//////////////////////////////////////////////////////////////////////
//
// OSAL - Operating System Abstraction Library
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

#if !defined(AFX_ALGORITHM_H__A66A7C23_1ADB_11D2_9E30_0060083E8062__INCLUDED_)
#define AFX_ALGORITHM_H__A66A7C23_1ADB_11D2_9E30_0060083E8062__INCLUDED_

#include "Time.h"
#include "Variant.h"

#define ALIGN(x, y) (((size_t) (x) + ((y) - 1)) & ~((y) - 1))

namespace Algorithm {

    #define OSAL_CUTOFF 8

    template <class T> class BaseCompare {
    public:
        virtual int Compare (const T* ptElem1, const T* ptElem2) = 0;
    };

    template <class T> class CompareAscending : public BaseCompare<T> {
    public:
        int Compare (const T* ptElem1, const T* ptElem2) {
            
            if (*ptElem1 < *ptElem2) {
                return -1;
            }
            else if (*ptElem1 > *ptElem2) {
                return 1;
            }
            else {
                return 0;
            }
        }
    };

    template <class T> class CompareDescending : public BaseCompare<T> {
    public:
        int Compare (const T* ptElem1, const T* ptElem2) {
            
            if (*ptElem1 > *ptElem2) {
                return -1;
            }
            else if (*ptElem1 < *ptElem2) {
                return 1;
            }
            else {
                return 0;
            }
        }
    };

    // Code borrowed from Microsoft's qsort.c implementation

    template <class T> void SwapOne (T* a, T* b) {

        if ( a != b ) {
            T tmp = *a;
            *a = *b;
            *b = tmp;
        }
    }

    template <class T, class S> void SwapTwo (T* a, T* b, S* a2, S* b2) {

        if ( a != b ) {
            
            T tmp = *a;
            S tmp2 = *a2;

            *a = *b;
            *b = tmp;

            *a2 = *b2;
            *b2 = tmp2;
        }
    }

    template <class T, class S, class Q> void SwapThree (T* a, T* b, S* a2, S* b2, Q* a3, Q* b3) {

        if ( a != b ) {
            
            T tmp = *a;
            S tmp2 = *a2;
            Q tmp3 = *a3;

            *a = *b;
            *b = tmp;

            *a2 = *b2;
            *b2 = tmp2;

            *a3 = *b3;
            *b3 = tmp3;
        }
    }

    template <class T> void ShortSortOne (T *lo, T *hi, BaseCompare<T>* pCompare) {

        T *p, *max;

        while (hi > lo) {
            max = lo;
            for (p = lo + 1; p <= hi; p ++) {
                if (pCompare->Compare (p, max) > 0) {
                    max = p;
                }

            }
            SwapOne (max, hi);
            hi --;
        }
    }

    template <class T, class S> void ShortSortTwo (T *lo, T *hi, S* lo2, S* hi2, BaseCompare<T>* pCompare) {

        T *p, *max;
        S *p2, *max2;

        while (hi > lo) {

            max = lo;
            max2 = lo2;
            for (p = lo + 1, p2 = lo2 + 1; p <= hi; p ++, p2 ++) {
                if (pCompare->Compare (p, max) > 0) {
                    max = p;
                    max2 = p2;
                }

            }
            SwapTwo (max, hi, max2, hi2);
            hi --;
            hi2 --;
        }
    }

    template <class T, class S, class Q> void ShortSortThree (T *lo, T *hi, S* lo2, S* hi2, Q* lo3, Q* hi3, 
        BaseCompare<T>* pCompare) {

        T *p, *max;
        S *p2, *max2;
        Q *p3, *max3;

        while (hi > lo) {

            max = lo;
            max2 = lo2;
            max3 = lo3;

            for (p = lo + 1, p2 = lo2 + 1, p3 = lo3 + 1; p <= hi; p ++, p2 ++, p3 ++) {
                if (pCompare->Compare (p, max) > 0) {
                    max = p;
                    max2 = p2;
                    max3 = p3;
                }

            }
            SwapThree (max, hi, max2, hi2, max3, hi3);
            hi --;
            hi2 --;
            hi3 --;
        }
    }

    template <class T> void QSortOne (T* base, size_t num, BaseCompare<T>* pCompare) {

        T *lo, *hi;              /* ends of sub-array currently sorting */
        T *mid;                  /* points to middle of subarray */
        T *loguy, *higuy;        /* traveling pointers for partition step */
        size_t size;              /* size of the sub-array */
        T *lostk[30], *histk[30];
        int stkptr;                 /* stack for saving sub-array to be processed */

        if (num < 2)
            return;                 /* nothing to do */
        
        stkptr = 0;                 /* initialize stack */
        
        lo = base;
        hi = base + (num-1);        /* initialize limits */
        
recurse:
        
        size = (hi - lo) + 1;        /* number of el's to sort */

        if (size <= OSAL_CUTOFF) {
            ShortSortOne<T> (lo, hi, pCompare);
        }
        else {

            mid = lo + (size / 2);      /* find middle element */
            SwapOne (mid, lo);               /* swap it to beginning of array */
            
            loguy = lo;
            higuy = hi + 1;
            
            for (;;) {

                do  {
                    loguy ++;
                } while (loguy <= hi && pCompare->Compare (loguy, lo) <= 0);

                do  {
                    higuy --;
                } while (higuy > lo && pCompare->Compare (higuy, lo) >= 0);
                
                if (higuy < loguy)
                    break;
                
                SwapOne (loguy, higuy);
            }

            SwapOne (lo, higuy);     /* put partition element in place */

            if ( higuy - 1 - lo >= hi - loguy ) {
                if (lo + 1 < higuy) {
                    lostk[stkptr] = lo;
                    histk[stkptr] = higuy - 1;
                    ++stkptr;
                }                           /* save big recursion for later */
                
                if (loguy < hi) {
                    lo = loguy;
                    goto recurse;           /* do small recursion */
                }
            }
            else {
                if (loguy < hi) {
                    lostk[stkptr] = loguy;
                    histk[stkptr] = hi;
                    ++stkptr;               /* save big recursion for later */
                }
                
                if (lo + 1 < higuy) {
                    hi = higuy - 1;
                    goto recurse;           /* do small recursion */
                }
            }
        }

        --stkptr;
        if (stkptr >= 0) {
            lo = lostk[stkptr];
            hi = histk[stkptr];
            goto recurse;           /* pop subarray from stack */
        }
        else
            return;                 /* all subarrays done */
    }


    template <class T, class S> void QSortTwo (T* base, S* base2, size_t num, BaseCompare<T>* pCompare) {

        T *lo, *hi;              /* ends of sub-array currently sorting */
        S *lo2, *hi2;              /* ends of sub-array currently sorting */

        T *mid;                  /* points to middle of subarray */
        S *mid2;                  /* points to middle of subarray */

        T *loguy, *higuy;        /* traveling pointers for partition step */
        S *loguy2, *higuy2;        /* traveling pointers for partition step */

        size_t size;              /* size of the sub-array */

        T *lostk[30], *histk[30];
        S *lostk2[30], *histk2[30];
        int stkptr;                 /* stack for saving sub-array to be processed */

        if (num < 2)
            return;                 /* nothing to do */
        
        stkptr = 0;                 /* initialize stack */
        
        lo = base;
        hi = base + (num-1);        /* initialize limits */

        lo2 = base2;
        hi2 = base2 + (num-1);        /* initialize limits */
        
recurse:
        
        size = (hi - lo) + 1;        /* number of el's to sort */

        if (size <= OSAL_CUTOFF) {
            ShortSortTwo<T, S> (lo, hi, lo2, hi2, pCompare);
        }
        else {

            mid = lo + (size / 2);        /* find middle element */
            mid2 = lo2 + (size / 2);      /* find middle element */
            SwapTwo (mid, lo, mid2, lo2); /* swap it to beginning of array */
            
            loguy = lo;
            higuy = hi + 1;

            loguy2 = lo2;
            higuy2 = hi2 + 1;
            
            for (;;) {

                do  {
                    loguy ++;
                    loguy2 ++;
                } while (loguy <= hi && pCompare->Compare (loguy, lo) <= 0);

                do  {
                    higuy --;
                    higuy2 --;
                } while (higuy > lo && pCompare->Compare (higuy, lo) >= 0);
                
                if (higuy < loguy)
                    break;
                
                SwapTwo (loguy, higuy, loguy2, higuy2);
            }

            SwapTwo (lo, higuy, lo2, higuy2);     /* put partition element in place */

            if ( higuy - 1 - lo >= hi - loguy ) {
                if (lo + 1 < higuy) {
                    
                    lostk[stkptr] = lo;
                    histk[stkptr] = higuy - 1;

                    lostk2[stkptr] = lo2;
                    histk2[stkptr] = higuy2 - 1;

                    ++stkptr;
                }                           /* save big recursion for later */
                
                if (loguy < hi) {
                    
                    lo = loguy;
                    lo2 = loguy2;
                    
                    goto recurse;           /* do small recursion */
                }
            }
            else {
                if (loguy < hi) {
                    
                    lostk[stkptr] = loguy;
                    histk[stkptr] = hi;

                    lostk2[stkptr] = loguy2;
                    histk2[stkptr] = hi2;

                    ++stkptr;               /* save big recursion for later */
                }
                
                if (lo + 1 < higuy) {
                    
                    hi = higuy - 1;
                    hi2 = higuy2 - 1;

                    goto recurse;           /* do small recursion */
                }
            }
        }

        --stkptr;
        if (stkptr >= 0) {
            
            lo = lostk[stkptr];
            hi = histk[stkptr];

            lo2 = lostk2[stkptr];
            hi2 = histk2[stkptr];

            goto recurse;           /* pop subarray from stack */
        }
        else
            return;                 /* all subarrays done */
    }


    template <class T, class S, class Q> void QSortThree (T* base, S* base2, Q* base3, size_t num, 
        BaseCompare<T>* pCompare) {

        T *lo, *hi;              /* ends of sub-array currently sorting */
        S *lo2, *hi2;              /* ends of sub-array currently sorting */
        Q *lo3, *hi3;              /* ends of sub-array currently sorting */

        T *mid;                  /* points to middle of subarray */
        S *mid2;                  /* points to middle of subarray */
        Q *mid3;                  /* points to middle of subarray */

        T *loguy, *higuy;        /* traveling pointers for partition step */
        S *loguy2, *higuy2;        /* traveling pointers for partition step */
        Q *loguy3, *higuy3;        /* traveling pointers for partition step */

        size_t size;              /* size of the sub-array */

        T *lostk[30], *histk[30];
        S *lostk2[30], *histk2[30];
        Q *lostk3[30], *histk3[30];
        
        int stkptr;                 /* stack for saving sub-array to be processed */

        if (num < 2)
            return;                 /* nothing to do */
        
        stkptr = 0;                 /* initialize stack */
        
        lo = base;
        hi = base + (num-1);        /* initialize limits */

        lo2 = base2;
        hi2 = base2 + (num-1);        /* initialize limits */

        lo3 = base3;
        hi3 = base3 + (num-1);        /* initialize limits */
        
recurse:
        
        size = (hi - lo) + 1;        /* number of el's to sort */

        if (size <= OSAL_CUTOFF) {
            ShortSortThree<T, S, Q> (lo, hi, lo2, hi2, lo3, hi3, pCompare);
        }
        else {

            mid = lo + (size / 2);        /* find middle element */
            mid2 = lo2 + (size / 2);      /* find middle element */
            mid3 = lo3 + (size / 2);      /* find middle element */

            SwapThree (mid, lo, mid2, lo2, mid3, lo3); /* swap it to beginning of array */
            
            loguy = lo;
            higuy = hi + 1;

            loguy2 = lo2;
            higuy2 = hi2 + 1;

            loguy3 = lo3;
            higuy3 = hi3 + 1;
            
            for (;;) {

                do  {
                    loguy ++;
                    loguy2 ++;
                    loguy3 ++;
                } while (loguy <= hi && pCompare->Compare (loguy, lo) <= 0);

                do  {
                    higuy --;
                    higuy2 --;
                    higuy3 --;
                } while (higuy > lo && pCompare->Compare (higuy, lo) >= 0);
                
                if (higuy < loguy)
                    break;
                
                SwapThree (loguy, higuy, loguy2, higuy2, loguy3, higuy3);
            }

            SwapThree (lo, higuy, lo2, higuy2, lo3, higuy3);     /* put partition element in place */

            if ( higuy - 1 - lo >= hi - loguy ) {
                if (lo + 1 < higuy) {
                    
                    lostk[stkptr] = lo;
                    histk[stkptr] = higuy - 1;

                    lostk2[stkptr] = lo2;
                    histk2[stkptr] = higuy2 - 1;

                    lostk3[stkptr] = lo3;
                    histk3[stkptr] = higuy3 - 1;

                    ++stkptr;
                }                           /* save big recursion for later */
                
                if (loguy < hi) {
                    
                    lo = loguy;
                    lo2 = loguy2;
                    lo3 = loguy3;
                    
                    goto recurse;           /* do small recursion */
                }
            }
            else {
                if (loguy < hi) {
                    
                    lostk[stkptr] = loguy;
                    histk[stkptr] = hi;

                    lostk2[stkptr] = loguy2;
                    histk2[stkptr] = hi2;

                    lostk3[stkptr] = loguy3;
                    histk3[stkptr] = hi3;

                    ++stkptr;               /* save big recursion for later */
                }
                
                if (lo + 1 < higuy) {
                    
                    hi = higuy - 1;
                    hi2 = higuy2 - 1;
                    hi3 = higuy3 - 1;

                    goto recurse;           /* do small recursion */
                }
            }
        }

        --stkptr;
        if (stkptr >= 0) {
            
            lo = lostk[stkptr];
            hi = histk[stkptr];

            lo2 = lostk2[stkptr];
            hi2 = histk2[stkptr];

            lo3 = lostk3[stkptr];
            hi3 = histk3[stkptr];

            goto recurse;           /* pop subarray from stack */
        }
        else
            return;                 /* all subarrays done */
    }

    // Public

    template <class T> void QSortAscending (T* base, size_t num) {

        CompareAscending<T> compAscending;
        QSortOne<T> (base, num, &compAscending);
    }

    template <class T> void QSortDescending (T* base, size_t num) {

        CompareDescending<T> compDescending;
        QSortOne<T> (base, num, &compDescending);
    }

    template <class T, class S> void QSortTwoAscending (T* base, S* base2, size_t num) {

        CompareAscending<T> compAscending;
        QSortTwo<T, S> (base, base2, num, &compAscending);
    }

    template <class T, class S> void QSortTwoDescending (T* base, S* base2, size_t num) {

        CompareDescending<T> compDescending;
        QSortTwo<T, S> (base, base2, num, &compDescending);
    }

    template <class T, class S, class Q> void QSortThreeAscending (T* base, S* base2, Q* base3, size_t num) {

        CompareAscending<T> compAscending;
        QSortThree<T, S, Q> (base, base2, base3, num, &compAscending);
    }

    template <class T, class S, class Q> void QSortThreeDescending (T* base, S* base2, Q* base3, size_t num) {

        CompareDescending<T> compDescending;
        QSortThree<T, S, Q> (base, base2, base3, num, &compDescending);
    }


    //////////////////////
    // Randomize arrays //
    //////////////////////
    
    template <class T> void Randomize (T* pData, unsigned int iNumElements) {
        
        if (iNumElements > 1) {

            int* piIndex = (int*) StackAlloc (iNumElements * sizeof (int));
            
            unsigned int i, iLimit = iNumElements * 1000;
            for (i = 0; i < iNumElements; i ++) {
                piIndex[i] = GetRandomInteger (iLimit);
            }
            
            QSortTwoDescending <int, T> (piIndex, pData, iNumElements);
        }
    }
    
    
    template <class T, class S> void RandomizeTwo (T* pData, S* pData2, unsigned int iNumElements) {
        
        if (iNumElements > 1) {
            
            int* piIndex = (int*) StackAlloc (iNumElements * sizeof (int));
            
            unsigned int i, iLimit = iNumElements * 1000;
            for (i = 0; i < iNumElements; i ++) {
                piIndex[i] = GetRandomInteger (iLimit);
            }
            
            QSortThreeDescending <int, T, S> (piIndex, pData, pData2, iNumElements);
        }
    }

    // AutoDelete
    template <class T> class AutoDelete {
    private:
        
        T*& m_pPtr;
        bool m_bVector;

        AutoDelete& operator= (AutoDelete& rhs) {

            Assert (!"This should never be called");
            m_pPtr = NULL;
            return *this;
        }
        
    public:
        
        AutoDelete (T*& pPtr, bool bVector = false) : m_pPtr (pPtr) {

            m_bVector = bVector;
        }

        ~AutoDelete() {
            
            if (m_pPtr != NULL) {
                if (m_bVector) {
                    delete [] m_pPtr;
                } else {
                    delete m_pPtr;
                }
            }
        }

        T* GetPtr() {
            return m_pPtr;
        }

        T* SetPtr (T*& pPtr) {
            m_pPtr = pPtr;
        }
    };

    OSAL_EXPORT void InitializeThreadRandom (int iRandFactor = 1);
    OSAL_EXPORT int GetRandomInteger (int iUpper);
    OSAL_EXPORT char GetRandomASCIIChar();

    OSAL_EXPORT char* memstr (const char* pszBuffer, const char* pszMatchString, size_t cbBytes);

    OSAL_EXPORT int UnescapeString (const char* pszInput, char* pszAnsi, size_t cchLength);

    OSAL_EXPORT size_t GetEncodeBase64Size (size_t cbDataLength);
    OSAL_EXPORT int EncodeBase64 (const void* pbData, size_t cbDataLength, char* pszBase64, size_t cchLength);

    OSAL_EXPORT size_t GetDecodeBase64Size (const char* pszBase64, size_t cchLength);
    OSAL_EXPORT int DecodeBase64 (const char* pszBase64, void* pbData, size_t cbLength, size_t* pcbDecoded);

    OSAL_EXPORT int HexEncode (const void* pbData, size_t cbDataLength, char* pszHex, size_t cchLength);

    OSAL_EXPORT int AtomicIncrement (int* piValue);
    OSAL_EXPORT int AtomicDecrement (int* piValue);
    
    OSAL_EXPORT unsigned int AtomicIncrement (unsigned int* piValue);
    OSAL_EXPORT unsigned int AtomicDecrement (unsigned int* piValue);
    
    OSAL_EXPORT int AtomicIncrement (int* piValue, int iValue);
    OSAL_EXPORT int AtomicDecrement (int* piValue, int iValue);

    OSAL_EXPORT unsigned int AtomicIncrement (unsigned int* piValue, int iValue);
    OSAL_EXPORT unsigned int AtomicDecrement (unsigned int* piValue, int iValue);

    OSAL_EXPORT int64 AtomicIncrement(int64* piValue);
    OSAL_EXPORT int64 AtomicDecrement(int64* piValue);

    OSAL_EXPORT int64 AtomicIncrement(int64* piValue, int64 iValue);
    OSAL_EXPORT int64 AtomicDecrement(int64* piValue, int64 iValue);

    OSAL_EXPORT uint64 AtomicIncrement(uint64* piValue);
    OSAL_EXPORT uint64 AtomicDecrement(uint64* piValue);

    OSAL_EXPORT uint64 AtomicIncrement(uint64* piValue, int64 iValue);
    OSAL_EXPORT uint64 AtomicDecrement(uint64* piValue, int64 iValue);

    OSAL_EXPORT unsigned int GetIntHashValue (int iValue, unsigned int iNumBuckets);
    OSAL_EXPORT unsigned int GetFloatHashValue (float fValue, unsigned int iNumBuckets);

    OSAL_EXPORT unsigned int GetStringHashValue (const char* pszString, unsigned int iNumBuckets, bool bCaseInsensitive);
    OSAL_EXPORT unsigned int GetStringHashValue (const char* pszString, size_t stStringLen, unsigned int iNumBuckets, bool bCaseInsensitive = true);

    OSAL_EXPORT unsigned int GetInt64HashValue (int64 i64Value, unsigned int iNumBuckets);
    OSAL_EXPORT unsigned int GetVariantHashValue (const Variant& vValue, unsigned int iNumBuckets, bool bCaseInsensitive);

    OSAL_EXPORT unsigned int GetStandardDeviation(unsigned int* piValues, unsigned int iNumValues);
    OSAL_EXPORT unsigned int GetArithmeticMean(unsigned int* piValues, unsigned int iNumValues);
};

#endif // !defined(AFX_ALGORITHM_H__A66A7C23_1ADB_11D2_9E30_0060083E8062__INCLUDED_)