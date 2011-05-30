// BridierScore.h: interface for the BridierScore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BRIDIERSCORE_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_)
#define AFX_BRIDIERSCORE_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_

#include "../GameEngine/GameEngine.h"

//
// BridierObject
//

class BridierObject {
protected:

    IGameEngine* m_pGameEngine;
    IDatabase* m_pDatabase;

    BridierObject (IGameEngine* pGameEngine);
    ~BridierObject();

public:

    int IsBridierGame (int iGameClass, int iGameNumber, bool* pbBridier);
    int CompareScores (const Variant* pvLeft, const Variant* pvRight);

    static void GetScoreChanges (int iNukerRank, int iNukerIndex, int iNukedRank, int iNukedIndex,
        int* piNukerRankChange, int* piNukerIndexChange, int* piNukedRankChange, int* piNukedIndexChange);

    int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore);

    int UpdateBridierScore (int iEmpireKey, int iRankChange, int iIndexChange);

    int GetReplacementKeys (bool bEstablished, const Variant* pvScore, unsigned int** ppiKey, 
        unsigned int* piNumEmpires);
};


//
// BridierScore
//

class BridierScore : public IScoringSystem, protected BridierObject {
protected:

    BridierScore (IGameEngine* pGameEngine);

public:

    IMPLEMENT_INTERFACE (IScoringSystem);

    static IScoringSystem* CreateInstance (IGameEngine* pGameEngine);

    // IScoringSystem
    bool HasTopList();

    int OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges);
    int OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges);

    int On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges);
    int On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges);

    int OnGameEnd (int iGameClass, int iGameNumber);
    int OnWin (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnDraw (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnRuin (int iGameClass, int iGameNumber, int iEmpireKey);

    bool IsValidScore (const Variant* pvScore);
    int CompareScores (const Variant* pvLeft, const Variant* pvRight);

    int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore);
    int GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires);

protected:

    int OnNukeInternal (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges);
};


//
// BridierScoreEstablished
//

class BridierScoreEstablished : public IScoringSystem, protected BridierObject {
protected:

    BridierScoreEstablished (IGameEngine* pGameEngine);

public:

    IMPLEMENT_INTERFACE (IScoringSystem);

    static IScoringSystem* CreateInstance (IGameEngine* pGameEngine);

    // IScoringSystem
    bool HasTopList();

    int OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked, ScoringChanges* pscChanges);
    int OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser, ScoringChanges* pscChanges);

    int On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser, ScoringChanges* pscChanges);
    int On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges);

    int OnGameEnd (int iGameClass, int iGameNumber);
    int OnWin (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnDraw (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnRuin (int iGameClass, int iGameNumber, int iEmpireKey);

    bool IsValidScore (const Variant* pvScore);
    int CompareScores (const Variant* pvLeft, const Variant* pvRight);

    int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore);
    int GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires);
};

#endif // !defined(AFX_BRIDIERSCORE_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_)
