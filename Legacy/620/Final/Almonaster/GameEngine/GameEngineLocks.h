// GameEngineLocks.h: interface for the GameEmpireLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMEENGINELOCKS_H__E21FE393_D69B_11D3_A2F0_0050047FE2E2__INCLUDED_)
#define AFX_GAMEENGINELOCKS_H__E21FE393_D69B_11D3_A2F0_0050047FE2E2__INCLUDED_

#include "Osal/IObject.h"
#include "Osal/ReadWriteLock.h"
#include "Osal/Thread.h"
#include "Osal/Event.h"

//
// GameEmpireId
//
class GameEmpireId {
public:
    
    /*
    unsigned int iGameClass;
    unsigned int iGameNumber;
    */
    unsigned int iEmpireKey;

    // Hash functionality
    static unsigned int GetHashValue (const GameEmpireId* pgeId, unsigned int iNumBuckets, 
                                      const void* pHashHint) {
        return pgeId->iEmpireKey % iNumBuckets;
    }

    static bool Equals (const GameEmpireId* pgeIdLeft, const GameEmpireId* pgeIdRight, const void* pEqualsHint) {
        return *pgeIdLeft == *pgeIdRight;
    }

    bool operator==(const GameEmpireId& geId) const {
        return iEmpireKey == geId.iEmpireKey;
    }   
};

//
// GameEmpireLock
//
class GameEmpireLock : public Mutex {
private:

    GameEmpireLock();

    GameEmpireId m_geId;
    UTCTime m_tLastAccess;
    bool m_bInUse;
    bool m_bActive;

public:

    static GameEmpireLock* Create();

    const GameEmpireId& GetId() const {
        return m_geId;
    }

    void SetId (const GameEmpireId& geId) {
        m_geId = geId;
    }

    const UTCTime& GetLastAccess() const {
        return m_tLastAccess;
    }

    void SetLastAccess() {
        Time::GetTime (&m_tLastAccess);
    }

    bool InUse() const {
        return m_bInUse;
    }

    void SetInUse (bool bInUse) {
        m_bInUse = bInUse;
    }

    bool Active() const {
        return m_bActive;
    }

    void SetActive (bool bActive) {
        m_bActive = bActive;
    }

    // Allocator functionality
    static GameEmpireLock* New() { return Create(); }
    static void Delete (GameEmpireLock* pgeLock) { delete pgeLock; }
};

//
// GameEmpireLockManager
//
struct LockManagerConfig {
    unsigned int iNumEmpiresHint;
    unsigned int iMaxAgedOutPerScan;
    MilliSeconds msScanPeriod;
    MilliSeconds msMaxScanTime;
    Seconds sAgeOutPeriod;
};

class GameEmpireLockManager {
private:

    typedef HashTable<const GameEmpireId*, GameEmpireLock*, GameEmpireId, GameEmpireId> LMHashTable;
    typedef HashTableIterator<const GameEmpireId*, GameEmpireLock*> LMHashTableIterator;

    // Hash table for lock objects
    ReadWriteLock m_rwHTLock;
    LMHashTable m_htLocks;

    // Cache for unused lock objects
    Mutex m_mOCLock;
    ObjectCache<GameEmpireLock, GameEmpireLock> m_oCache;

    // Thread for aging out unused lock objects
    Thread m_tScanThread;
    Event m_eShutdown;

    LockManagerConfig m_lmConf;

    // Utility functions
    static int THREAD_CALL ScanThread (void* pThis);
    void Scan();

    GameEmpireLock* FindOrCreateLock();
    void FreeLock (GameEmpireLock* pgeLock);

public:

    GameEmpireLockManager();
    ~GameEmpireLockManager();

    int Initialize (const LockManagerConfig& lmConf);
    void Shutdown();

    GameEmpireLock* GetGameEmpireLock (const GameEmpireId& geId);
    void ReleaseGameEmpireLock (GameEmpireLock* pgeLock);

    unsigned int GetNumElements();
};

#endif // !defined(AFX_GAMEENGINELOCKS_H__E21FE393_D69B_11D3_A2F0_0050047FE2E2__INCLUDED_)