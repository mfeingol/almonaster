// AlmonasterScore.h: interface for the AlmonasterScore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALMONASTERSCORE_H__516DA67C_C8D9_4CC0_91F7_D5A71AF2F3F0__INCLUDED_)
#define AFX_ALMONASTERSCORE_H__516DA67C_C8D9_4CC0_91F7_D5A71AF2F3F0__INCLUDED_

#include "../GameEngine/GameEngine.h"

class AlmonasterScore : public IScoringSystem {
protected:

    IGameEngine* m_pGameEngine;
    IDatabaseConnection* t_pConn;

    AlmonasterScore (IGameEngine* pGameEngine);
    ~AlmonasterScore();

    void GetAlmonasterScoreChanges (float fNukerScore, float fNukedScore, int iNukerSignificance,
        int iNukedSignificance, int iNumNukerAllies, int iNumNukedAllies, float* pfNukerIncrease, 
        float* pfNukedDecrease);

    int GetRelevantStatistics (int iGameClass, int iGameNumber, int iEmpireKey,
        float* pfAlmonasterScore, int* piSignificance, int* piNumAllies);

    int GetRelevantStatisticsFromPlanet (const char* pszGameMap, int iPlanetKey, float* pfScore,
        int* piSignificance, int* piNumAllies, int* piLoserKey, int64* pi64EmpireSecretKey,
        Variant* pvPlanetName);

    int HandleUncolonizedHomeWorldOnEndGame (int iGameClass, int iGameNumber, int iPlanetKey, 
        Variant* pvEmpireKey, float* pfWinnerScore, int* piWinnerSignificance, int* piWinnerNumAllies,
        unsigned int iNumEmpires);

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

#endif // !defined(AFX_ALMONASTERSCORE_H__516DA67C_C8D9_4CC0_91F7_D5A71AF2F3F0__INCLUDED_)