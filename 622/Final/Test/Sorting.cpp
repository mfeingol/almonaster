#include "Osal/Algorithm.h"
#include <stdio.h>

#define NUM_INTS 256

int CompareDescending (const void *elem1, const void *elem2 ) {

	int iElem1 = *(int*) elem1;
	int iElem2 = *(int*) elem2;

	if (iElem1 > iElem2) {
		return -1;
	}
	else if (iElem1 < iElem2) {
		return 1;
	}
	else {
		return 0;
	}
}

int CompareAscending (const void *elem1, const void *elem2 ) {

	int iElem1 = *(int*) elem1;
	int iElem2 = *(int*) elem2;

	if (iElem1 < iElem2) {
		return -1;
	}
	else if (iElem1 > iElem2) {
		return 1;
	}
	else {
		return 0;
	}
}


void TestSortingAlgorithms() {

	int i;
	Timer tTimer;

	int* piData1 = new int [NUM_INTS];
	int* piData2 = new int [NUM_INTS];
	int* piData3 = new int [NUM_INTS];
	int* piData4 = new int [NUM_INTS];
	int* piData5 = new int [NUM_INTS];
	int* piData6 = new int [NUM_INTS];
	int* piData7 = new int [NUM_INTS];
	int* piData8 = new int [NUM_INTS];
	int* piData9 = new int [NUM_INTS];
	int* piData10 = new int [NUM_INTS];
	int* piData11 = new int [NUM_INTS];
	int* piData12 = new int [NUM_INTS];

	for (i = 0; i < NUM_INTS; i ++) {
		piData1[i] = i;
	}

	Algorithm::InitializeThreadRandom();

	Algorithm::Randomize<int> (piData1, NUM_INTS);
	memcpy (piData2, piData1, NUM_INTS * sizeof (int));

	memcpy (piData3, piData1, NUM_INTS * sizeof (int));
	memcpy (piData4, piData1, NUM_INTS * sizeof (int));

	memcpy (piData5, piData1, NUM_INTS * sizeof (int));
	memcpy (piData6, piData1, NUM_INTS * sizeof (int));

	memcpy (piData7, piData1, NUM_INTS * sizeof (int));
	memcpy (piData8, piData1, NUM_INTS * sizeof (int));
	memcpy (piData9, piData1, NUM_INTS * sizeof (int));

	memcpy (piData10, piData1, NUM_INTS * sizeof (int));
	memcpy (piData11, piData1, NUM_INTS * sizeof (int));
	memcpy (piData12, piData1, NUM_INTS * sizeof (int));

	//

	Time::StartTimer (&tTimer);
	Algorithm::QSortAscending<int> (piData1, NUM_INTS);
	printf (
		"QSortAscending finished in %i ms\t\tFirst is %i, Last is %i\n", 
		Time::GetTimerCount (tTimer),
		piData1[0],
		piData1[NUM_INTS - 1]
		);

	Time::StartTimer (&tTimer);
	Algorithm::QSortDescending<int> (piData2, NUM_INTS);
	printf (
		"QSortDescending finished in %i ms\t\tFirst is %i, Last is %i\n", 
		Time::GetTimerCount (tTimer),
		piData2[0],
		piData2[NUM_INTS - 1]
		);

	//

	Time::StartTimer (&tTimer);
	Algorithm::QSortTwoAscending<int, int> (piData3, piData4, NUM_INTS);
	printf (
		"QSortTwoAscending finished in %i ms\t\tFirst are %i = %i, Last is %i = %i\n", 
		Time::GetTimerCount (tTimer),
		piData3[0],
		piData4[0],
		piData3[NUM_INTS - 1],
		piData4[NUM_INTS - 1]
		);

	Time::StartTimer (&tTimer);
	Algorithm::QSortTwoDescending<int> (piData5, piData6, NUM_INTS);
	printf (
		"QSortTwoAscending finished in %i ms\t\tFirst are %i = %i, Last is %i = %i\n", 
		Time::GetTimerCount (tTimer),
		piData5[0],
		piData6[0],
		piData5[NUM_INTS - 1],
		piData6[NUM_INTS - 1]
		);

	//

	Time::StartTimer (&tTimer);
	Algorithm::QSortThreeAscending<int, int, int> (piData7, piData8, piData9, NUM_INTS);
	printf (
		"QSortThreeAscending finished in %i ms\t\tFirst are %i = %i = %i, Last is %i = %i = %i\n", 
		Time::GetTimerCount (tTimer),
		piData7[0],
		piData8[0],
		piData9[0],
		piData7[NUM_INTS - 1],
		piData8[NUM_INTS - 1],
		piData9[NUM_INTS - 1]
		);

	Time::StartTimer (&tTimer);
	Algorithm::QSortThreeDescending<int, int, int> (piData10, piData11, piData12, NUM_INTS);
	printf (
		"QSortThreeDescending finished in %i ms\t\tFirst are %i = %i = %i, Last is %i = %i = %i\n", 
		Time::GetTimerCount (tTimer),
		piData10[0],
		piData11[0],
		piData12[0],
		piData10[NUM_INTS - 1],
		piData11[NUM_INTS - 1],
		piData12[NUM_INTS - 1]
		);
}