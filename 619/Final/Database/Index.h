// Index.h: interface for the Index class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INDEX_H__8BEAC025_D5E7_11D2_9FF7_0060083E8062__INCLUDED_)
#define AFX_INDEX_H__8BEAC025_D5E7_11D2_9FF7_0060083E8062__INCLUDED_

#include "Osal/Variant.h"
#include "Osal/HashTable.h"

struct IndexHint {
	bool CaseInsensitive;
	VariantType Type;
	size_t BaseAddress;
};

class Index {
private:

	class IndexHashValue {
	public:
		static unsigned int GetHashValue (const void* pData, unsigned int iNumBuckets, const void* pHashHint);
	};

	class IndexEquals {
	public:
		static bool Equals (const void* pLeft, const void* pRight, const void* pEqualsHint);
	};

	HashTable<const void*, unsigned int, IndexHashValue, IndexEquals> m_htHashTable;

	unsigned int m_iColumn;
	IndexHint m_IndexHint;

public:

	Index();
	~Index();

	int Initialize (unsigned int iColumn, unsigned int iNumCols, VariantType vtType, bool bCaseInsensitive, 
		void* pBaseAddress);

	unsigned int GetColumn() { return m_iColumn; }
	void SetBaseAddress (void* pBaseAddress) { m_IndexHint.BaseAddress = (size_t) pBaseAddress; }

	int GetFirstKey (const Variant& vData, unsigned int* piKey);
	int GetEqualKeys (const Variant& vData, unsigned int** ppiKey, unsigned int* piNumKeys);

	int InsertRow (unsigned int iKey, const void* pData);
	int DeleteRow (unsigned int iKey, const void* pData);

	int DeleteAllRows();
};

#endif // !defined(AFX_INDEX_H__8BEAC025_D5E7_11D2_9FF7_0060083E8062__INCLUDED_)