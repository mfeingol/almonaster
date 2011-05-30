// WriteTable.h: interface for the WriteTable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WRITETABLE_H__3B5FD4A4_370A_11D3_A10B_0050047FE2E2__INCLUDED_)
#define AFX_WRITETABLE_H__3B5FD4A4_370A_11D3_A10B_0050047FE2E2__INCLUDED_

#include "ReadTable.h"

class WriteTable : public IWriteTable, public ReadTable {

protected:

	WriteTable (Table* pTable);

public:

	static WriteTable* CreateInstance (Table* pTable);

	// IReadTable
	DECLARE_IOBJECT;

	int WriteData (unsigned int iKey, unsigned int iColumn, int iData);
	int WriteData (unsigned int iKey, unsigned int iColumn, float fData);
	int WriteData (unsigned int iKey, unsigned int iColumn, const char* pszData);
	int WriteData (unsigned int iKey, unsigned int iColumn, const UTCTime& tData);
	int WriteData (unsigned int iKey, unsigned int iColumn, int64 i64Data);
	int WriteData (unsigned int iKey, unsigned int iColumn, const Variant& vData);

	int WriteData (unsigned int iColumn, int iData);
	int WriteData (unsigned int iColumn, float fData);
	int WriteData (unsigned int iColumn, const char* pszData);
	int WriteData (unsigned int iColumn, const UTCTime& tData);
	int WriteData (unsigned int iColumn, int64 i64Data);
	int WriteData (unsigned int iColumn, const Variant& vData);

	int WriteAnd (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
	int WriteAnd (unsigned int iColumn, unsigned int iBitField);

	int WriteOr (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
	int WriteOr (unsigned int iColumn, unsigned int iBitField);

	int WriteXor (unsigned int iKey, unsigned int iColumn, unsigned int iBitField);
	int WriteXor (unsigned int iColumn, unsigned int iBitField);

	int WriteNot (unsigned int iKey, unsigned int iColumn);
	int WriteNot (unsigned int iColumn);

	int WriteColumn (unsigned int iColumn, int iData);
	int WriteColumn (unsigned int iColumn, float fData);
	int WriteColumn (unsigned int iColumn, const char* pszData);
	int WriteColumn (unsigned int iColumn, const UTCTime& tData);
	int WriteColumn (unsigned int iColumn, int64 i64Data);
	int WriteColumn (unsigned int iColumn, const Variant& vData);

	int InsertRow (Variant* pvColVal, unsigned int* piKey);
	int InsertRow (Variant* pvColVal);

	int InsertRows (Variant* pvColVal, unsigned int iNumRows);
	int InsertDuplicateRows (Variant* pvColVal, unsigned int iNumRows);

	int Increment (unsigned int iColumn, const Variant& vIncrement) { return ERROR_NOT_IMPLEMENTED; }
	int Increment (unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue) { return ERROR_NOT_IMPLEMENTED; }

	int Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement) { return ERROR_NOT_IMPLEMENTED; }
	int Increment (unsigned int iKey, unsigned int iColumn, const Variant& vIncrement, Variant* pvOldValue) { return ERROR_NOT_IMPLEMENTED; }


	int DeleteRow (unsigned int iKey);
	int DeleteAllRows();
};

#endif // !defined(AFX_WRITETABLE_H__3B5FD4A4_370A_11D3_A10B_0050047FE2E2__INCLUDED_)