// ClassicScore.h: interface for the ClassicScore class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLASSICSCORE_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_)
#define AFX_CLASSICSCORE_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_

#include "../GameEngine/GameEngine.h"

class ClassicScore : public IScoringSystem {
protected:

	IGameEngine* m_pGameEngine;
	IDatabase* m_pDatabase;

	ClassicScore (IGameEngine* pGameEngine);
	~ClassicScore();

	unsigned int m_iColumn;

public:

	IMPLEMENT_INTERFACE (IScoringSystem);

	static IScoringSystem* CreateInstance (IGameEngine* pGameEngine);

	// IScoringSystem
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

#endif // !defined(AFX_CLASSICSCORE_H__73488604_ED55_4BC5_8981_A395C3928D12__INCLUDED_)
