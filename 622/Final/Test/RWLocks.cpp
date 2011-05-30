#include "RWLocks.h"

#include "Osal/Thread.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/Algorithm.h"

ReadWriteLock gRWLock;
int giCounter;

int THREAD_CALL ReadTest (void*) {

    int iCounter;

    while (true) {

        gRWLock.WaitReader();

        iCounter = giCounter;
        OS::Sleep (100);
        Assert (giCounter == iCounter);

        gRWLock.SignalReader();
    }
}

int THREAD_CALL WriteTest (void*) {

    int iCounter;

    while (true) {

        gRWLock.WaitWriter();

        iCounter = giCounter;

        giCounter ++;
        giCounter ++;

        OS::Sleep (100);

        Assert (iCounter + 2 == giCounter);

        gRWLock.SignalWriter();
    }
}

int THREAD_CALL ReadWriteTest (void*) {

    int iCounter;

    while (true) {

        gRWLock.WaitReaderWriter();

        iCounter = giCounter;
        OS::Sleep (100);
        Assert (giCounter == iCounter);

        gRWLock.UpgradeReaderWriter();

        giCounter ++;
        giCounter ++;

        OS::Sleep (100);

        Assert (iCounter + 2 == giCounter);

        gRWLock.DowngradeReaderWriter();

        OS::Sleep (100);
        Assert (iCounter + 2 == giCounter);

        gRWLock.SignalReaderWriter();
    }
}

void TestRWLocks() {

    size_t i;
    Thread atThreads[100];

    gRWLock.Initialize();

    for (i = 0; i < countof (atThreads); i ++) {

        switch (Algorithm::GetRandomInteger (3)) {

        case 0:
            atThreads[i].Start (ReadTest, NULL);
            break;

        case 1:
            atThreads[i].Start (WriteTest, NULL);
            break;

        case 2:
            atThreads[i].Start (ReadWriteTest, NULL);
            break;

        default:

            Assert (false);
            break;
        }
    }

    for (i = 0; i < countof (atThreads); i ++) {
        atThreads[i].WaitForTermination();
    }
}