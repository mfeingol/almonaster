#include "Database.h"
#include "TestDatabase.h"

#include <stdio.h>

#include "Osal/Variant.h"

enum Columns {
	One,
	Two,
	Three,
	Four,
	Five
};

static const VariantType Types[] = {
	V_INT,
	V_FLOAT,
	V_TIME,
	V_STRING,
	V_STRING,
};

static const size_t Sizes[] = {
	0,
	0,
	0,
	7,
	7,
};

static const unsigned int NumColumns = Five + 1;

static const unsigned int colIndexes[] = {
	One,
	Two,
	Three,
	Four,
	Five,
};

static const unsigned int colIndexFlags[] = {
	0,
	0,
	0,
	0,
	0,
};

static const TemplateDescription Template = {
	"Test",
	NumColumns,
	(VariantType*) Types,
	(size_t*) Sizes,
	false,
	5,
	(unsigned int*) colIndexes,
	10,
	(unsigned int*) colIndexFlags,
};

#define TABLE "TestTable"

namespace GameEmpireDiplomacy {
	
	const char* Name = "GameEmpireDiplomacy";

	enum Columns {
		EmpireKey,
		DipOffer,
		CurrentStatus,
		VirtualStatus,
		State,
		SubjectiveEcon,
		SubjectiveMil,
		LastMessageTargetFlag,
	};
	
	static const VariantType Types[] = {
		V_INT,
		V_INT,
		V_INT,
		V_INT,
		V_INT,
		V_INT,
		V_INT,
		V_INT,
	};

	static const size_t Sizes[] = {
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
	};

	static unsigned int IndexColumns[] = {
		EmpireKey,
		CurrentStatus
	};

	static unsigned int IndexFlags[] = {
		0,
		0
	};

	static const unsigned int NumColumns = LastMessageTargetFlag + 1;
	
	static const TemplateDescription Template = {
		"GameEmpireDiplomacy",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		sizeof (IndexColumns) / sizeof (IndexColumns[0]),
		IndexColumns,
		10,
		IndexFlags,
	};
};

#define MAX_EMPIRE_NAME_LENGTH 27

namespace GameEmpireMessages {
	
	enum Columns {
		Unread,
		Source,
		TimeStamp,
		Broadcast,
		Text
	};
	
	static const VariantType Types[] = {
		V_INT,
		V_STRING,
		V_TIME,
		V_INT,
		V_STRING
	};

	static const size_t Sizes[] = {
		0,
		MAX_EMPIRE_NAME_LENGTH,
		0,
		0,
		VARIABLE_LENGTH_STRING
	};

    static unsigned int IndexColumns[] = {
		Unread,
	};

	static unsigned int IndexFlags[] = {
		0,
	};

	static const unsigned int NumColumns = Text + 1;

	static const TemplateDescription Template = {
		"GameEmpireMessages",
		NumColumns,
		(VariantType*) Types,
		(size_t*) Sizes,
		false,
		sizeof (IndexColumns) / sizeof (IndexColumns[0]),
		IndexColumns,
		20,
		IndexFlags,
	};
};

#define MESSAGES "GameEmpireMessages"

//#define CheckDB
#define CheckDB pDatabase->Check()

void TestDatabase2() {

	IDatabase* pDatabase = NULL;
	Variant vData;
	unsigned int i;

	IWriteTable* pTable = NULL;
//	IReadTable* pRead = NULL;

	int iErrCode = DatabaseCreateInstance (CLSID_Database, IID_IDatabase, (void**) &pDatabase);
	Assert (iErrCode == OK);

	iErrCode = pDatabase->Initialize ("C:/Temp", true);
	Assert (iErrCode == OK || iErrCode == WARNING);

	CheckDB;
	Assert (iErrCode == OK);

	if (!pDatabase->DoesTemplateExist (Template.Name)) {
		
		iErrCode = pDatabase->CreateTemplate (Template);
		Assert (iErrCode == OK);

		CheckDB;
		Assert (iErrCode == OK);

		Assert (pDatabase->IsTemplateEqual (Template.Name, Template));
	}

	if (!pDatabase->DoesTableExist (TABLE)) {

		iErrCode = pDatabase->CreateTable (TABLE, Template.Name);
		Assert (iErrCode == OK);

		CheckDB;
		Assert (iErrCode == OK);
	}

	iErrCode = pDatabase->DeleteAllRows (TABLE);
	Assert (iErrCode == OK);

	CheckDB;
	Assert (iErrCode == OK);

	UTCTime tTime;
	Time::GetTime (&tTime);

	Variant pvData[] = {
		1,
		(float) 2.5,
		tTime,
		"AAA",
		"BBBBBBB",
	};

	unsigned int iKeyOne, iKeyTwo, iTestKey;

	iErrCode = pDatabase->InsertRow (TABLE, pvData, &iKeyOne);
	Assert (iErrCode == OK);

	CheckDB;
	Assert (iErrCode == OK);

	Variant pvData2[] = {
		1,
		(float) 2.5,
		tTime,
		"AAAA",
		"BBBBBBBB",
	};

	iErrCode = pDatabase->InsertRow (TABLE, pvData2, &iKeyTwo);
	Assert (iErrCode == ERROR_STRING_IS_TOO_LONG);
	iErrCode = OK;

	pvData2[Four] = "XXX";
	pvData2[Five] = "ZZZZZZZ";

	iErrCode = pDatabase->InsertRow (TABLE, pvData2, &iKeyTwo);
	Assert (iErrCode == OK);
	iErrCode = OK;

	CheckDB;
	Assert (iErrCode == OK);

	iErrCode = pDatabase->WriteData (TABLE, iKeyTwo, Four, "YYY");
	Assert (iErrCode == OK);

	CheckDB;
	Assert (iErrCode == OK);

	iErrCode = pDatabase->ReadData (TABLE, iKeyTwo, Four, &vData);
	Assert (iErrCode == OK);

	CheckDB;
	Assert (iErrCode == OK);

	Assert (vData == "YYY");

	iErrCode = pDatabase->WriteData (TABLE, iKeyTwo, Four, "DDDDDDDD");
	Assert (iErrCode == ERROR_STRING_IS_TOO_LONG);
	iErrCode = OK;

	CheckDB;
	Assert (iErrCode == OK);

	//
	// Indexing
	// 

	iErrCode = pDatabase->GetFirstKey (TABLE, Four, "AAA", true, &iTestKey);
	Assert (iErrCode == OK && iTestKey == iKeyOne);

	iErrCode = pDatabase->GetFirstKey (TABLE, One, 1, true, &iTestKey);
	Assert (iErrCode == OK && iTestKey == iKeyOne);

	iErrCode = pDatabase->GetFirstKey (TABLE, Two, (float) 2.5, true, &iTestKey);
	Assert (iErrCode == OK && iTestKey == iKeyOne);

	iErrCode = pDatabase->GetFirstKey (TABLE, Four, "NOT", true, &iTestKey);
	Assert (iErrCode == ERROR_DATA_NOT_FOUND && iTestKey == NO_KEY);

	iErrCode = pDatabase->GetFirstKey (TABLE, Four, "YYY", true, &iTestKey);
	Assert (iErrCode == OK && iTestKey == iKeyTwo);

	unsigned int iNumRows;

	iErrCode = pDatabase->GetNumRows (TABLE, &iNumRows);
	Assert (iErrCode == OK && iNumRows == 2);

	unsigned int iNumKeys, * piKey;

	iErrCode = pDatabase->GetAllKeys (TABLE, &piKey, &iNumKeys);
	Assert (iErrCode == OK && iNumKeys == 2);
	Assert (piKey[0] == 0);
	Assert (piKey[1] == 1);

	pDatabase->FreeKeys (piKey);

	iErrCode = pDatabase->GetEqualKeys (TABLE, One, 1, true, &piKey, &iNumKeys);
	Assert (iErrCode == OK && iNumKeys == 2);
	Assert (piKey[0] == 0);
	Assert (piKey[1] == 1);

	pDatabase->FreeKeys (piKey);

	iErrCode = pDatabase->GetEqualKeys (TABLE, Four, "YYY", true, &piKey, &iNumKeys);
	Assert (iErrCode == OK && iNumKeys == 1);
	Assert (piKey[0] == iKeyTwo);

	pDatabase->FreeKeys (piKey);

	iErrCode = pDatabase->GetEqualKeys (TABLE, Four, "ASD", true, &piKey, &iNumKeys);
	Assert (iErrCode == ERROR_DATA_NOT_FOUND && piKey == NULL && iNumKeys == 0);

	//
	//
	//

	iErrCode = pDatabase->DeleteAllRows (TABLE);
	Assert (iErrCode == OK);

	const int iNumRowsTest = 6;

	iErrCode = pDatabase->InsertDuplicateRows (TABLE, pvData, iNumRowsTest);
	Assert (iErrCode == OK);

	for (i = 0; i < iNumRowsTest; i ++) {

		iErrCode = pDatabase->WriteData (TABLE, i, One, 67);
		Assert (iErrCode == OK);

		iErrCode = pDatabase->WriteData (TABLE, i, Two, (float) 4.0);
		Assert (iErrCode == OK);
	}

	iErrCode = pDatabase->GetEqualKeys (TABLE, One, 67, true, &piKey, &iNumKeys);
	Assert (iErrCode == OK && iNumKeys == iNumRowsTest);
	
	for (i = 0; i < iNumRowsTest; i ++) {
		Assert (piKey[i] == i);
	}

	pDatabase->FreeKeys (piKey);

	iErrCode = pDatabase->GetEqualKeys (TABLE, Two, (float) 4.0, true, &piKey, &iNumKeys);
	Assert (iErrCode == OK && iNumKeys == iNumRowsTest);
	
	for (i = 0; i < iNumRowsTest; i ++) {
		Assert (piKey[i] == i);
	}

	pDatabase->FreeKeys (piKey);

	//
	//
	//

	Variant pvData3[] = {
		0,
		(float) 2.5,
		tTime,
		"AAA",
		"BBBBBBB",
	};

	iErrCode = pDatabase->DeleteAllRows (TABLE);
	Assert (iErrCode == OK);

	iErrCode = pDatabase->InsertRow (TABLE, pvData3, &iTestKey);
	Assert (iErrCode == OK && iTestKey == 0);

	pvData3[0] = 1;

	iErrCode = pDatabase->InsertRow (TABLE, pvData3, &iTestKey);
	Assert (iErrCode == OK && iTestKey == 1);

	iErrCode = pDatabase->GetFirstKey (TABLE, One, 0, true, &iTestKey);
	Assert (iErrCode == OK && iTestKey == 0);

	iErrCode = pDatabase->GetFirstKey (TABLE, One, 1, true, &iTestKey);
	Assert (iErrCode == OK && iTestKey == 1);

	iErrCode = pDatabase->DeleteRow (TABLE, 0);
	Assert (iErrCode == OK);

	iErrCode = pDatabase->GetFirstKey (TABLE, One, 0, true, &iTestKey);
	Assert (iErrCode == ERROR_DATA_NOT_FOUND && iTestKey == NO_KEY);

	iErrCode = pDatabase->GetFirstKey (TABLE, One, 1, true, &iTestKey);
	Assert (iErrCode == OK && iTestKey == 1);

	iErrCode = pDatabase->DeleteRow (TABLE, 1);
	Assert (iErrCode == OK);

	iErrCode = pDatabase->GetFirstKey (TABLE, One, 0, true, &iTestKey);
	Assert (iErrCode == ERROR_DATA_NOT_FOUND && iTestKey == NO_KEY);

	iErrCode = pDatabase->GetFirstKey (TABLE, One, 1, true, &iTestKey);
	Assert (iErrCode == ERROR_DATA_NOT_FOUND && iTestKey == NO_KEY);

	//
	//
	//

	if (!pDatabase->DoesTemplateExist (GameEmpireDiplomacy::Name)) {
		
		iErrCode = pDatabase->CreateTemplate (GameEmpireDiplomacy::Template);
		Assert (iErrCode == OK);

		CheckDB;
		Assert (iErrCode == OK);

		Assert (pDatabase->IsTemplateEqual (GameEmpireDiplomacy::Name, GameEmpireDiplomacy::Template));
	}

	if (!pDatabase->DoesTableExist (GameEmpireDiplomacy::Name)) {

		iErrCode = pDatabase->CreateTable (GameEmpireDiplomacy::Name, GameEmpireDiplomacy::Name);
		Assert (iErrCode == OK);

		CheckDB;
		Assert (iErrCode == OK);
	}

	Variant pvDipData[] = {
		199,
		1,
		1,
		1,
		0,
		0,
		0,
		1,
	};

	Assert (sizeof (pvDipData) / sizeof (Variant) == GameEmpireDiplomacy::NumColumns);

	iErrCode = pDatabase->GetNumRows (GameEmpireDiplomacy::Name, &iNumRows);
	Assert (iErrCode == OK);

	iErrCode = pDatabase->InsertRow (GameEmpireDiplomacy::Name, pvDipData, &iTestKey);
	Assert (iErrCode == OK && iTestKey == iNumRows);

	//
	//
	//

	CheckDB;

	//
	// Messages
	//

	if (!pDatabase->DoesTemplateExist (GameEmpireMessages::Template.Name)) {
		
		iErrCode = pDatabase->CreateTemplate (GameEmpireMessages::Template);
		Assert (iErrCode == OK);

		CheckDB;
		Assert (iErrCode == OK);

		Assert (pDatabase->IsTemplateEqual (GameEmpireMessages::Template.Name, GameEmpireMessages::Template));
	}

	if (!pDatabase->DoesTableExist (MESSAGES)) {

		iErrCode = pDatabase->CreateTable (MESSAGES, GameEmpireMessages::Template.Name);
		Assert (iErrCode == OK);

		CheckDB;
		Assert (iErrCode == OK);
	}

	Variant pvMessage[] = {
		1,
		"The System",
		(UTCTime) 0xffffffff,
		0xeeeeeeee, 
		"Beluga whales",
	};

	for (i = 0; i < 100; i ++) {

		iErrCode = pDatabase->InsertRow (MESSAGES, pvMessage);
		Assert (iErrCode == OK);

		CheckDB;
	}

	for (i = 0; i < 100; i ++) {

		iErrCode = pDatabase->DeleteRow (MESSAGES, i);
		Assert (iErrCode == OK);

		CheckDB;
	}

	CheckDB;

	//
	// Insert a lot of rows
	//
	iErrCode = pDatabase->GetTableForWriting (MESSAGES, &pTable);
	Assert (iErrCode == OK);

    for (i = 0; i < 1000; i ++) {
        pvMessage[0] = i % 2;
		iErrCode = pTable->InsertRow (pvMessage);
		Assert (iErrCode == OK);
	}
	pTable->Release();

    //
    // Do some searches
    //

    unsigned int iRealNumRead, iRealNumUnread;

    iErrCode = pDatabase->GetEqualKeys (MESSAGES, GameEmpireMessages::Unread, 1, false, NULL, &iRealNumUnread);
    Assert (iErrCode == OK);

    iErrCode = pDatabase->GetEqualKeys (MESSAGES, GameEmpireMessages::Unread, 0, false, NULL, &iRealNumRead);
    Assert (iErrCode == OK);

    iErrCode = pDatabase->GetNumRows (MESSAGES, &iNumRows);
    Assert (iErrCode == OK);

    unsigned int iStopKey, iNumHits, * piKeys = NULL;
    SearchDefinition sd;
    SearchColumn sc[2];

    while (true) {

        int iSearchVal = Algorithm::GetRandomInteger (2);

        unsigned int iStartKey = Algorithm::GetRandomInteger (iRealNumRead);
        if (iStartKey % 3 == 0) {
            iStartKey = NO_KEY;
        }

        unsigned int iSkipHits = Algorithm::GetRandomInteger (iNumRows);
        if (iSkipHits % 3 == 0) {
            iSkipHits = 0;
        }

        unsigned int iMaxNumHits = Algorithm::GetRandomInteger (iNumRows);
        if (iMaxNumHits % 3 == 0) {
            iMaxNumHits = 0;
        }

        sc[0].iColumn = GameEmpireMessages::Text;
        sc[0].iFlags = SEARCH_EXACT;
        sc[0].vData = Algorithm::GetRandomInteger (2) == 1 ? "Foo" : pvMessage[GameEmpireMessages::Text];

        sc[1].iColumn = GameEmpireMessages::Unread;
        sc[1].iFlags = 0;
        sc[1].vData = iSearchVal;
        sc[1].vData2 = iSearchVal;

        sd.iStartKey = iStartKey;
        sd.iSkipHits = iSkipHits;
        sd.iMaxNumHits = iMaxNumHits;
        sd.iNumColumns = Algorithm::GetRandomInteger (2) + 1;
        sd.pscColumns = sc;

        unsigned int* piEqualKeys, iEqualKeys, * piFreeKeys;

        iErrCode = pDatabase->GetEqualKeys (
            MESSAGES, GameEmpireMessages::Unread, iSearchVal, false, &piEqualKeys, &iEqualKeys);
        Assert (iErrCode == OK);

        piFreeKeys = piEqualKeys;

        if (iStartKey != NO_KEY && iStartKey != 0) {

            for (i = 0; i < iEqualKeys; i ++) {

                if (piEqualKeys[i] >= iStartKey) {
                    break;
                }
            }

            iEqualKeys -= i;
            piEqualKeys += i;
        }

        if (iSkipHits != 0) {

            if (iEqualKeys < iSkipHits) {
                iEqualKeys = 0;
            } else {
                iEqualKeys -= iSkipHits;
                piEqualKeys += iSkipHits;
            }
        }

        if (iMaxNumHits > 0 && iMaxNumHits < iEqualKeys) {
            iEqualKeys = iMaxNumHits;
        }

        iErrCode = pDatabase->GetSearchKeys (MESSAGES, sd, &piKeys, &iNumHits, &iStopKey);
        if (iErrCode == OK || iErrCode == ERROR_TOO_MANY_HITS) {

            if (sd.iNumColumns == 1) {

                unsigned iCheckRows = iNumRows;
                unsigned int iShouldHits = iNumHits + iSkipHits;
                if (iStartKey != NO_KEY) {
                    iShouldHits += iStartKey;
                }

                if (iMaxNumHits != 0 && iMaxNumHits < iShouldHits) {
                    iShouldHits = iMaxNumHits;
                }

                if (iMaxNumHits != 0 && iMaxNumHits < iCheckRows) {
                    iCheckRows = iMaxNumHits;
                }

                Assert (iShouldHits == iCheckRows);

            } else {

                Assert (iEqualKeys == iNumHits);

                for (i = 0; i < iNumHits; i ++) {
                    Assert (piEqualKeys[i] == piKeys[i]);
                }

                for (i = 0; i < iNumHits; i ++) {

                    iErrCode = pDatabase->ReadData (MESSAGES, piKeys[i], GameEmpireMessages::Unread, &vData);
                    Assert (vData.GetInteger() == iSearchVal);
                }
            }
        
        } else if (iErrCode == ERROR_DATA_NOT_FOUND) {

            if (sc[0].vData != "Foo") {
                Assert (iSkipHits >= iEqualKeys || iEqualKeys == 0);
            }
        }

        else Assert (false);

        if (piKeys != NULL) {
            pDatabase->FreeKeys (piKeys);
        }
        pDatabase->FreeKeys (piFreeKeys);
    }

	//
	// End
	//

	CheckDB;

	pDatabase->Release();
}