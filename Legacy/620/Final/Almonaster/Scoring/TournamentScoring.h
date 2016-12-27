// ClassicScore.h: interface for the ClassicScore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOURNAMENTSCORING_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_)
#define AFX_TOURNAMENTSCORING_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_

#include "../GameEngine/GameEngine.h"

enum TournamentEvent {
    TOURNAMENT_NUKE,
    TOURNAMENT_NUKED,
    TOURNAMENT_WIN,
    TOURNAMENT_DRAW,
    TOURNAMENT_RUIN,
    TOURNAMENT_NUM_EVENTS
};


class TournamentScoring : public IScoringSystem {
protected:

    IGameEngine* m_pGameEngine;
    IDatabase* m_pDatabase;

    TournamentScoring (IGameEngine* pGameEngine);
    ~TournamentScoring();

    int IsTournamentGame (int iGameClass, int iGameNumber, unsigned int* piTournamentKey);
    int OnEvent (int iGameClass, int iGameNumber, int iEmpireKey, TournamentEvent event);
    int OnEvent (unsigned int iTournamentKey, unsigned int iEmpireKey, TournamentEvent event);

public:

    IMPLEMENT_INTERFACE (IScoringSystem);

    static IScoringSystem* CreateInstance (IGameEngine* pGameEngine);

    // IScoringSystem
    bool HasTopList();

    int OnNuke (int iGameClass, int iGameNumber, int iEmpireNuker, int iEmpireNuked);
    int OnSurrender (int iGameClass, int iGameNumber, int iWinner, int iLoser);

    int On30StyleSurrender (int iGameClass, int iGameNumber, int iLoser);
    int On30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey);
    int OnGameEnd (int iGameClass, int iGameNumber);

    int OnWin (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnDraw (int iGameClass, int iGameNumber, int iEmpireKey);
    int OnRuin (int iGameClass, int iGameNumber, int iEmpireKey);

    bool IsValidScore (const Variant* pvScore);
    int CompareScores (const Variant* pvLeft, const Variant* pvRight);

    int GetEmpireScore (unsigned int iEmpireKey, Variant* pvScore);
    int GetReplacementKeys (const Variant* pvScore, unsigned int** ppiKey, unsigned int* piNumEmpires);
};

#endif // !defined(AFX_TOURNAMENTSCORING_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_)