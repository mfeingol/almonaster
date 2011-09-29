//
// Almonaster.dll: a component of Almonaster
// Copyright(c) 1998 Max Attar Feingold(maf6@cornell.edu)
//
// This program is free software;you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation;either version 2
// of the License, or(at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY;without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program;if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA02111-1307, USA.

#include "GameEngine.h"

int GameEngine::LookupEmpireByName(const char* pszName, unsigned int* piEmpireKey, Variant* pvName, int64* pi64SecretKey, ICachedTable** ppTable)
{
    const ColumnEntry col = { SystemEmpireData::Name, pszName };
    const TableCacheEntry entry = { { SYSTEM_EMPIRE_DATA, NO_KEY, 1, &col }, NULL, NULL, NULL };
    Assert(col.Data.GetCharPtr());

    ICachedTable* pEmpire = NULL;
    AutoRelease<ICachedTable> rel(pEmpire);

    int iErrCode = t_pCache->Cache(entry, &pEmpire);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpire->GetNextKey(NO_KEY, piEmpireKey);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        if (pvName)
        {
            iErrCode = pEmpire->ReadData(*piEmpireKey, SystemEmpireData::Name, pvName);
            RETURN_ON_ERROR(iErrCode);
        }

        if (pi64SecretKey)
        {
            iErrCode = pEmpire->ReadData(*piEmpireKey, SystemEmpireData::SecretKey, pi64SecretKey);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (ppTable)
    {
        *ppTable = pEmpire;
        pEmpire->AddRef();
    }

    return iErrCode;
}

int GameEngine::CacheEmpire(unsigned int iEmpireKey)
{
    return CacheEmpires(&iEmpireKey, 1, NULL);
}

int GameEngine::CacheEmpire(unsigned int iEmpireKey, unsigned int* piResults)
{
    return CacheEmpires(&iEmpireKey, 1, piResults);
}

int GameEngine::CacheEmpires(const Variant* pvEmpireKey, unsigned int iNumEmpires)
{
    unsigned int* piEmpireKey = (unsigned int*)StackAlloc(iNumEmpires * sizeof(unsigned int));
    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        piEmpireKey[i] = pvEmpireKey[i].GetInteger();
    }
    return CacheEmpires(piEmpireKey, iNumEmpires);
}

int GameEngine::CacheEmpires(const unsigned int* piEmpireKey, unsigned int iNumEmpires)
{
    return CacheEmpires(piEmpireKey, iNumEmpires, NULL);
}

int GameEngine::CacheEmpires(const unsigned int* piEmpireKey, unsigned int iNumEmpires, unsigned int* piResults)
{
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumEmpires * sizeof(TableCacheEntry));
    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        pcEntries[i].Table.Name = SYSTEM_EMPIRE_DATA;
        pcEntries[i].Table.Key = piEmpireKey[i];
        pcEntries[i].Table.NumColumns = 0;
        pcEntries[i].Table.Columns = NULL;
        pcEntries[i].Reserved = NULL;
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;
    }
    
    int iErrCode = t_pCache->Cache(pcEntries, iNumEmpires);
    RETURN_ON_ERROR(iErrCode);

    if (piResults)
    {
        *piResults = 0;
        for (unsigned int i = 0; i < iNumEmpires; i ++)
        {
            unsigned int iNumRows;
            GET_SYSTEM_EMPIRE_DATA(strEmpire, piEmpireKey[i]) 
            iErrCode = t_pCache->GetNumCachedRows(strEmpire, &iNumRows);
            RETURN_ON_ERROR(iErrCode);
            *piResults += iNumRows;
        }
    }

    return iErrCode;
}

int GameEngine::CacheEmpireAndMessages(unsigned int iEmpireKey)
{
    const ColumnEntry systemEmpireMessagesCol = { SystemEmpireMessages::EmpireKey, iEmpireKey };
    const TableCacheEntry systemEmpireEntries[] =
    {
        { { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &systemEmpireMessagesCol }, NULL, NULL, NULL },
    };

    return t_pCache->Cache(systemEmpireEntries, countof(systemEmpireEntries));
}

int GameEngine::CacheEmpireMessagesAndTournaments(unsigned int iEmpireKey)
{
    const ColumnEntry systemEmpireMessagesCol = { SystemEmpireMessages::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireTournamentsCol = { SystemEmpireTournaments::EmpireKey, iEmpireKey };
    const TableCacheEntry systemEmpireEntries[] =
    {
        { { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &systemEmpireMessagesCol }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_TOURNAMENTS, NO_KEY, 1, &systemEmpireTournamentsCol }, NULL, NULL, NULL },
    };

    return t_pCache->Cache(systemEmpireEntries, countof(systemEmpireEntries));
}

int GameEngine::CacheEmpireForDeletion(unsigned int iEmpireKey)
{
    const ColumnEntry systemEmpireActiveGamesCol = { SystemEmpireActiveGames::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireNukeListCol = { SystemEmpireNukeList::EmpireKey, iEmpireKey };
    const TableCacheEntry systemEmpireEntries[] =
    {
        { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &systemEmpireActiveGamesCol }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKER_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKED_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL, NULL },
    };

    return t_pCache->Cache(systemEmpireEntries, countof(systemEmpireEntries));
}

int GameEngine::CacheTournamentTables(const unsigned int* piTournamentKey, unsigned int iNumTournaments)
{
    ColumnEntry* pcCols = (ColumnEntry*)StackAlloc(iNumTournaments * 2 * sizeof(ColumnEntry));
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumTournaments * 2 * sizeof(TableCacheEntry));
    for (unsigned int i = 0; i < iNumTournaments; i ++)
    {
        pcCols[i].Name = SystemTournamentEmpires::TournamentKey;
        pcCols[i].Data = piTournamentKey[i];

        pcCols[i + iNumTournaments].Name = SystemTournamentTeams::TournamentKey;
        pcCols[i + iNumTournaments].Data = piTournamentKey[i];

        pcEntries[i].Table.Name = SYSTEM_TOURNAMENT_EMPIRES;
        pcEntries[i].Table.Key = NO_KEY;
        pcEntries[i].Table.NumColumns = 1;
        pcEntries[i].Table.Columns = pcCols + i;
        pcEntries[i].Reserved = NULL;
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;

        pcEntries[i + iNumTournaments].Table.Name = SYSTEM_TOURNAMENT_TEAMS;
        pcEntries[i + iNumTournaments].Table.Key = NO_KEY;
        pcEntries[i + iNumTournaments].Table.NumColumns = 1;
        pcEntries[i + iNumTournaments].Table.Columns = pcCols + i + iNumTournaments;
        pcEntries[i + iNumTournaments].Reserved = NULL;
        pcEntries[i + iNumTournaments].PartitionColumn = NULL;
        pcEntries[i + iNumTournaments].CrossJoin = NULL;
    }

    return t_pCache->Cache(pcEntries, iNumTournaments * 2);
}

int GameEngine::CacheTournamentAndEmpireTables(unsigned int iTournamentKey)
{
    ColumnEntry tournamentCol = { SystemTournamentEmpires::TournamentKey, iTournamentKey };
    CrossJoinEntry crossJoin = { { SYSTEM_TOURNAMENT_EMPIRES, NO_KEY, 1, &tournamentCol }, ID_COLUMN_NAME, SystemTournamentEmpires::EmpireKey };

    const TableCacheEntry entries[] =
    {
        { { SYSTEM_TOURNAMENT_EMPIRES, NO_KEY, 1, &tournamentCol }, NULL, NULL, NULL},
        { { SYSTEM_TOURNAMENT_TEAMS, NO_KEY, 1, &tournamentCol }, NULL, NULL, NULL},
        { { SYSTEM_EMPIRE_DATA, NO_KEY, 0, NULL }, NULL, ID_COLUMN_NAME, &crossJoin },
    };

    int iErrCode = t_pCache->Cache(entries, countof(entries));
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::CacheTournamentEmpireTables(unsigned int iTournamentKey)
{
    ColumnEntry crossJoinCol = { SystemTournamentEmpires::TournamentKey, iTournamentKey };
    CrossJoinEntry crossJoin = { { SYSTEM_TOURNAMENT_EMPIRES, NO_KEY, 1, &crossJoinCol }, ID_COLUMN_NAME, SystemTournamentEmpires::EmpireKey };

    const TableCacheEntry entries[] =
    {
        { { SYSTEM_EMPIRE_DATA, NO_KEY, 0, NULL }, NULL, ID_COLUMN_NAME, &crossJoin },
    };

    int iErrCode = t_pCache->Cache(entries, countof(entries));
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::CacheTournamentEmpiresForGame(unsigned int iTournamentKey)
{
    ColumnEntry crossJoinCol = { SystemTournamentEmpires::TournamentKey, iTournamentKey };
    CrossJoinEntry crossJoin = { { SYSTEM_TOURNAMENT_EMPIRES, NO_KEY, 1, &crossJoinCol }, ID_COLUMN_NAME, SystemTournamentEmpires::EmpireKey };

    const TableCacheEntry entries[] =
    {
        { { SYSTEM_EMPIRE_DATA, NO_KEY, 0, NULL }, NULL, ID_COLUMN_NAME, &crossJoin },
        { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 0, NULL }, NULL, SystemEmpireActiveGames::EmpireKey, &crossJoin },
    };

    int iErrCode = t_pCache->Cache(entries, countof(entries));
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateEmptyGameCacheEntries(NO_KEY, 0, NO_KEY, iTournamentKey, NO_KEY, EMPTY_SYSTEM_EMPIRE_ACTIVE_GAMES);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::CacheGameData(int* piGameClass, int* piGameNumber, int iEmpireKey, unsigned int iNumGames)
{
    const unsigned int cGameDataCols = 2;
    const unsigned int cGameEmpireDataCols = 3;

    const unsigned int cNumGameTables = 5;
    const unsigned int cNumGameEmpireTables = iEmpireKey != NO_KEY ? 1 : 0;
    const unsigned int cNumTableTypes = cNumGameTables + cNumGameEmpireTables;

    ColumnEntry* pcGameCols = (ColumnEntry*)StackAlloc(cGameDataCols * iNumGames * sizeof(ColumnEntry));
    ColumnEntry* pcGameEmpireCols = (ColumnEntry*)StackAlloc(cGameEmpireDataCols * iNumGames * sizeof(ColumnEntry));

    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumGames * cNumTableTypes * sizeof(TableCacheEntry));

    for (unsigned int i = 0; i < iNumGames; i ++)
	{
        pcGameCols[i*cGameDataCols + 0].Name = GameData::GameClass;
        pcGameCols[i*cGameDataCols + 0].Data = piGameClass[i];
        pcGameCols[i*cGameDataCols + 1].Name = GameData::GameNumber;
        pcGameCols[i*cGameDataCols + 1].Data = piGameNumber[i];

        pcEntries[i + iNumGames * 0].Table.Name = GAME_DATA;
		pcEntries[i + iNumGames * 0].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 0].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 0].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 0].Reserved = NULL;
        pcEntries[i + iNumGames * 0].PartitionColumn = NULL;
        pcEntries[i + iNumGames * 0].CrossJoin = NULL;

        pcEntries[i + iNumGames * 1].Table.Name = GAME_EMPIRES;
		pcEntries[i + iNumGames * 1].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 1].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 1].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 1].Reserved = NULL;
        pcEntries[i + iNumGames * 1].PartitionColumn = NULL;
        pcEntries[i + iNumGames * 1].CrossJoin = NULL;

        pcEntries[i + iNumGames * 2].Table.Name = GAME_NUKED_EMPIRES;
		pcEntries[i + iNumGames * 2].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 2].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 2].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 2].Reserved = NULL;
        pcEntries[i + iNumGames * 2].PartitionColumn = NULL;
        pcEntries[i + iNumGames * 2].CrossJoin = NULL;

        pcEntries[i + iNumGames * 3].Table.Name = GAME_MAP;
		pcEntries[i + iNumGames * 3].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 3].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 3].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 3].Reserved = NULL;
        pcEntries[i + iNumGames * 3].PartitionColumn = NULL;
        pcEntries[i + iNumGames * 3].CrossJoin = NULL;

        pcEntries[i + iNumGames * 4].Table.Name = GAME_EMPIRE_DATA;
		pcEntries[i + iNumGames * 4].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 4].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 4].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 4].Reserved = NULL;
        pcEntries[i + iNumGames * 4].PartitionColumn = GameEmpireData::EmpireKey;
        pcEntries[i + iNumGames * 4].CrossJoin = NULL;

        if (iEmpireKey != NO_KEY)
        {
            pcGameEmpireCols[i*cGameEmpireDataCols + 0].Name = GameEmpireData::GameClass;
            pcGameEmpireCols[i*cGameEmpireDataCols + 0].Data = piGameClass[i];
            pcGameEmpireCols[i*cGameEmpireDataCols + 1].Name = GameEmpireData::GameNumber;
            pcGameEmpireCols[i*cGameEmpireDataCols + 1].Data = piGameNumber[i];
            pcGameEmpireCols[i*cGameEmpireDataCols + 2].Name = GameEmpireData::EmpireKey;
            pcGameEmpireCols[i*cGameEmpireDataCols + 2].Data = iEmpireKey;

            pcEntries[i + iNumGames * (cNumGameTables + 0)].Table.Name = GAME_EMPIRE_MESSAGES;
            pcEntries[i + iNumGames * (cNumGameTables + 0)].Table.Key = NO_KEY;
            pcEntries[i + iNumGames * (cNumGameTables + 0)].Table.NumColumns = cGameEmpireDataCols;
            pcEntries[i + iNumGames * (cNumGameTables + 0)].Table.Columns = pcGameEmpireCols + i*cGameEmpireDataCols;
            pcEntries[i + iNumGames * (cNumGameTables + 0)].Reserved = NULL;
            pcEntries[i + iNumGames * (cNumGameTables + 0)].PartitionColumn = NULL;
            pcEntries[i + iNumGames * (cNumGameTables + 0)].CrossJoin = NULL;
        }
	}

	return t_pCache->Cache(pcEntries, iNumGames * cNumTableTypes);
}

int GameEngine::CacheGameEmpireData(unsigned int iEmpireKey, const Variant* pvGame, unsigned int iNumGames)
{
    const unsigned int cGameEmpireDataCols = 3;

    ColumnEntry* pcGameEmpireDataCols = (ColumnEntry*)StackAlloc(cGameEmpireDataCols * iNumGames * sizeof(ColumnEntry));
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumGames * sizeof(TableCacheEntry));

    for (unsigned int i = 0; i < iNumGames; i ++)
	{
        int iGameClass, iGameNumber;
        GetGameClassGameNumber(pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

        pcGameEmpireDataCols[i*cGameEmpireDataCols + 0].Name = GameEmpireData::GameClass;
        pcGameEmpireDataCols[i*cGameEmpireDataCols + 0].Data = iGameClass;
        pcGameEmpireDataCols[i*cGameEmpireDataCols + 1].Name = GameEmpireData::GameNumber;
        pcGameEmpireDataCols[i*cGameEmpireDataCols + 1].Data = iGameNumber;
        pcGameEmpireDataCols[i*cGameEmpireDataCols + 2].Name = GameEmpireData::EmpireKey;
        pcGameEmpireDataCols[i*cGameEmpireDataCols + 2].Data = iEmpireKey;

        pcEntries[i].Table.Name = GAME_EMPIRE_DATA;
		pcEntries[i].Table.Key = NO_KEY;
		pcEntries[i].Table.NumColumns = cGameEmpireDataCols;
		pcEntries[i].Table.Columns = pcGameEmpireDataCols + i*cGameEmpireDataCols;
        pcEntries[i].Reserved = NULL;
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;
	}

	return t_pCache->Cache(pcEntries, iNumGames);
}

int GameEngine::CacheEmpireAndActiveGames(const unsigned int* piEmpireKey, unsigned int iNumEmpires)
{
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumEmpires * 2 * sizeof(TableCacheEntry));
    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        pcEntries[i].Table.Name = SYSTEM_EMPIRE_DATA;
        pcEntries[i].Table.Key = piEmpireKey[i];
        pcEntries[i].Table.NumColumns = 0;
        pcEntries[i].Table.Columns = NULL;
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;

        ColumnEntry* pCol = (ColumnEntry*)StackAlloc(sizeof(ColumnEntry));
        pCol->Name = SystemEmpireActiveGames::EmpireKey;
        pCol->Data = piEmpireKey[i];

        pcEntries[iNumEmpires + i].Table.Name = SYSTEM_EMPIRE_ACTIVE_GAMES;
        pcEntries[iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[iNumEmpires + i].Table.NumColumns = 1;
        pcEntries[iNumEmpires + i].Table.Columns = pCol;
        pcEntries[iNumEmpires + i].Reserved = NULL;
        pcEntries[iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[iNumEmpires + i].CrossJoin = NULL;
    }

    return t_pCache->Cache(pcEntries, iNumEmpires * 2);
}

int GameEngine::CacheEmpiresAndGameMessages(int iGameClass, int iGameNumber, const unsigned int* piEmpireKey, unsigned int iNumEmpires)
{
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumEmpires * 2 * sizeof(TableCacheEntry));
    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        pcEntries[i].Table.Name = SYSTEM_EMPIRE_DATA;
        pcEntries[i].Table.Key = piEmpireKey[i];
        pcEntries[i].Table.NumColumns = 0;
        pcEntries[i].Table.Columns = NULL;
        pcEntries[i].Reserved = NULL;
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;

        ColumnEntry* pCols = (ColumnEntry*)StackAlloc(3 * sizeof(ColumnEntry));
        pCols[0].Name = GameEmpireMessages::GameClass;
        pCols[0].Data = iGameClass;
        pCols[1].Name = GameEmpireMessages::GameNumber;
        pCols[1].Data = iGameNumber;
        pCols[2].Name = GameEmpireMessages::EmpireKey;
        pCols[2].Data = piEmpireKey[i];

        pcEntries[iNumEmpires + i].Table.Name = GAME_EMPIRE_MESSAGES;
        pcEntries[iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[iNumEmpires + i].Table.NumColumns = 3;
        pcEntries[iNumEmpires + i].Table.Columns = pCols;
        pcEntries[iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[iNumEmpires + i].CrossJoin = NULL;
    }

    return t_pCache->Cache(pcEntries, iNumEmpires * 2);
}

int GameEngine::CacheEmpireActiveGamesMessagesNukeLists(const unsigned int* piEmpireKey, unsigned int iNumEmpires)
{
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumEmpires * 5 * sizeof(TableCacheEntry));
    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        pcEntries[i].Table.Name = SYSTEM_EMPIRE_DATA;
        pcEntries[i].Table.Key = piEmpireKey[i];
        pcEntries[i].Table.NumColumns = 0;
        pcEntries[i].Table.Columns = NULL;
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;

        ColumnEntry* pActiveGamesCol = (ColumnEntry*)StackAlloc(sizeof(ColumnEntry));
        pActiveGamesCol->Name = SystemEmpireActiveGames::EmpireKey;
        pActiveGamesCol->Data = piEmpireKey[i];

        pcEntries[iNumEmpires + i].Table.Name = SYSTEM_EMPIRE_ACTIVE_GAMES;
        pcEntries[iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[iNumEmpires + i].Table.NumColumns = 1;
        pcEntries[iNumEmpires + i].Table.Columns = pActiveGamesCol;
        pcEntries[iNumEmpires + i].Reserved = NULL;
        pcEntries[iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[iNumEmpires + i].CrossJoin = NULL;

        ColumnEntry* pMessagesCol = (ColumnEntry*)StackAlloc(sizeof(ColumnEntry));
        pMessagesCol->Name = SystemEmpireMessages::EmpireKey;
        pMessagesCol->Data = piEmpireKey[i];

        pcEntries[2 * iNumEmpires + i].Table.Name = SYSTEM_EMPIRE_MESSAGES;
        pcEntries[2 * iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[2 * iNumEmpires + i].Table.NumColumns = 1;
        pcEntries[2 * iNumEmpires + i].Table.Columns = pMessagesCol;
        pcEntries[2 * iNumEmpires + i].Reserved = NULL;
        pcEntries[2 * iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[2 * iNumEmpires + i].CrossJoin = NULL;

        ColumnEntry* pNukeListCol = (ColumnEntry*)StackAlloc(sizeof(ColumnEntry));
        pNukeListCol->Name = SystemEmpireNukeList::EmpireKey;
        pNukeListCol->Data = piEmpireKey[i];

        pcEntries[3 * iNumEmpires + i].Table.Name = SYSTEM_EMPIRE_NUKER_LIST;
        pcEntries[3 * iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[3 * iNumEmpires + i].Table.NumColumns = 1;
        pcEntries[3 * iNumEmpires + i].Table.Columns = pNukeListCol;
        pcEntries[3 * iNumEmpires + i].Reserved = NULL;
        pcEntries[3 * iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[3 * iNumEmpires + i].CrossJoin = NULL;

        pcEntries[4 * iNumEmpires + i].Table.Name = SYSTEM_EMPIRE_NUKED_LIST;
        pcEntries[4 * iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[4 * iNumEmpires + i].Table.NumColumns = 1;
        pcEntries[4 * iNumEmpires + i].Table.Columns = pNukeListCol;
        pcEntries[4 * iNumEmpires + i].Reserved = NULL;
        pcEntries[4 * iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[4 * iNumEmpires + i].CrossJoin = NULL;
    }

    return t_pCache->Cache(pcEntries, iNumEmpires * 2);
}

int GameEngine::CacheAllGameTables(int iGameClass, int iGameNumber)
{
    // Bit of a hack, but it works
    const ColumnEntry gameDataCols[] = 
    {
        { GameData::GameClass, iGameClass },
        { GameData::GameNumber, iGameNumber },
    };

    const TableCacheEntry entries[] =
    {
        { { SYSTEM_ACTIVE_GAMES, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_LATEST_GAMES, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_NUKE_LIST, NO_KEY, 0, NULL }, NULL, NULL, NULL },

        { { GAME_DATA, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL, NULL },
        { { GAME_SECURITY, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL, NULL },
        { { GAME_EMPIRES, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL, NULL },
        { { GAME_NUKED_EMPIRES, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL, NULL },
        { { GAME_MAP, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL, NULL },

        { { GAME_EMPIRE_DATA, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireData::EmpireKey, NULL },
        { { GAME_EMPIRE_MESSAGES, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireMessages::EmpireKey, NULL },
        { { GAME_EMPIRE_MAP, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireMap::EmpireKey, NULL },
        { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL },
        { { GAME_EMPIRE_SHIPS, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireShips::EmpireKey, NULL },
        { { GAME_EMPIRE_FLEETS, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireFleets::EmpireKey, NULL },
    };

    int iErrCode;

    Variant* pvEmpireKey = NULL;
    AutoFreeData free(pvEmpireKey);
    unsigned int iNumEmpires;

    // Try to save an I/O if we already have the empires table cached
    GET_GAME_EMPIRES(strEmpires, iGameClass, iGameNumber);
    if (t_pCache->IsCached(strEmpires))
    {
        iErrCode = t_pCache->ReadColumn(strEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
        RETURN_ON_ERROR(iErrCode);

        unsigned int iNumEntries = countof(entries) + iNumEmpires * 5;
        TableCacheEntry* pEntries = (TableCacheEntry*)StackAlloc(iNumEntries * sizeof(TableCacheEntry));
        memcpy(pEntries, entries, sizeof(entries));

        ColumnEntry* pcColumns = (ColumnEntry*)StackAlloc(iNumEmpires * 3 * sizeof(ColumnEntry));

        for (unsigned int i = 0; i < iNumEmpires; i ++)
        {
            pEntries[countof(entries) + i].Table.Name = SYSTEM_EMPIRE_DATA;
            pEntries[countof(entries) + i].Table.Key = pvEmpireKey[i].GetInteger();
            pEntries[countof(entries) + i].Table.NumColumns = 0;
            pEntries[countof(entries) + i].Table.Columns = NULL;
            pEntries[countof(entries) + i].Reserved = NULL;
            pEntries[countof(entries) + i].PartitionColumn = NULL;
            pEntries[countof(entries) + i].CrossJoin = NULL;

            pcColumns[i].Name = SystemEmpireActiveGames::EmpireKey;
            pcColumns[i].Data = pvEmpireKey[i];

            pEntries[countof(entries) + i + iNumEmpires].Table.Name = SYSTEM_EMPIRE_ACTIVE_GAMES;
            pEntries[countof(entries) + i + iNumEmpires].Table.Key = NO_KEY;
            pEntries[countof(entries) + i + iNumEmpires].Table.NumColumns = 1;
            pEntries[countof(entries) + i + iNumEmpires].Table.Columns = pcColumns + i;
            pEntries[countof(entries) + i + iNumEmpires].Reserved = NULL;
            pEntries[countof(entries) + i + iNumEmpires].PartitionColumn = NULL;
            pEntries[countof(entries) + i + iNumEmpires].CrossJoin = NULL;

            pcColumns[i + iNumEmpires].Name = SystemEmpireMessages::EmpireKey;
            pcColumns[i + iNumEmpires].Data = pvEmpireKey[i];

            pEntries[countof(entries) + i + iNumEmpires * 2].Table.Name = SYSTEM_EMPIRE_MESSAGES;
            pEntries[countof(entries) + i + iNumEmpires * 2].Table.Key = NO_KEY;
            pEntries[countof(entries) + i + iNumEmpires * 2].Table.NumColumns = 1;
            pEntries[countof(entries) + i + iNumEmpires * 2].Table.Columns = pcColumns + i + iNumEmpires;
            pEntries[countof(entries) + i + iNumEmpires * 2].Reserved = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 2].PartitionColumn = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 2].CrossJoin = NULL;

            pcColumns[i + iNumEmpires * 2].Name = SystemEmpireNukeList::EmpireKey;
            pcColumns[i + iNumEmpires * 2].Data = pvEmpireKey[i];

            pEntries[countof(entries) + i + iNumEmpires * 3].Table.Name = SYSTEM_EMPIRE_NUKED_LIST;
            pEntries[countof(entries) + i + iNumEmpires * 3].Table.Key = NO_KEY;
            pEntries[countof(entries) + i + iNumEmpires * 3].Table.NumColumns = 1;
            pEntries[countof(entries) + i + iNumEmpires * 3].Table.Columns = pcColumns + i + iNumEmpires * 2;
            pEntries[countof(entries) + i + iNumEmpires * 3].Reserved = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 3].PartitionColumn = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 3].CrossJoin = NULL;

            pEntries[countof(entries) + i + iNumEmpires * 4].Table.Name = SYSTEM_EMPIRE_NUKER_LIST;
            pEntries[countof(entries) + i + iNumEmpires * 4].Table.Key = NO_KEY;
            pEntries[countof(entries) + i + iNumEmpires * 4].Table.NumColumns = 1;
            pEntries[countof(entries) + i + iNumEmpires * 4].Table.Columns = pcColumns + i + iNumEmpires * 2;
            pEntries[countof(entries) + i + iNumEmpires * 4].Reserved = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 4].PartitionColumn = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 4].CrossJoin = NULL;
        }

        iErrCode = t_pCache->Cache(pEntries, iNumEntries);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        // Do it in two I/Os, sigh...
        iErrCode = t_pCache->Cache(entries, countof(entries));
        RETURN_ON_ERROR(iErrCode);

        iErrCode = t_pCache->ReadColumn(strEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
        RETURN_ON_ERROR(iErrCode);

        unsigned int* piEmpireKey = (unsigned int*)StackAlloc(iNumEmpires * sizeof(unsigned int));
        for (unsigned int i = 0; i < iNumEmpires; i ++)
        {
            piEmpireKey[i] = pvEmpireKey[i].GetInteger();
        }
        iErrCode = CacheEmpireActiveGamesMessagesNukeLists(piEmpireKey, iNumEmpires);
        RETURN_ON_ERROR(iErrCode);
    }

    Assert(pvEmpireKey && iNumEmpires);

    iErrCode = CreateEmptyGameCacheEntries(iGameClass, iGameNumber, NO_KEY, NO_KEY, NO_KEY, 
                                           EMPTY_GAME_EMPIRE_MESSAGES | EMPTY_GAME_EMPIRE_MAP | EMPTY_GAME_EMPIRE_DIPLOMACY | 
                                           EMPTY_GAME_EMPIRE_SHIPS | EMPTY_GAME_EMPIRE_FLEETS |
                                           EMPTY_SYSTEM_EMPIRE_MESSAGES);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::CacheGameTablesForBroadcast(int iGameClass, int iGameNumber)
{
    // Bit of a hack, but it works
    const ColumnEntry gameDataCols[] = 
    {
        { GameData::GameClass, iGameClass },
        { GameData::GameNumber, iGameNumber },
    };

    const TableCacheEntry entries[] =
    {
        { { GAME_EMPIRE_DATA, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireData::EmpireKey, NULL },
        { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireDiplomacy::EmpireKey, NULL },
        { { GAME_EMPIRE_MESSAGES, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, GameEmpireMessages::EmpireKey, NULL },
    };

    int iErrCode = t_pCache->Cache(entries, countof(entries));
    RETURN_ON_ERROR(iErrCode);

    // Create empty entries for all empires
    iErrCode = CreateEmptyGameCacheEntries(iGameClass, iGameNumber, NO_KEY, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_MESSAGES | EMPTY_GAME_EMPIRE_DIPLOMACY);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::CacheProfileData(unsigned int iEmpireKey)
{
    // Cache profile data
    const ColumnEntry systemActiveGamesCol = { SystemEmpireActiveGames::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireTournamentsCol = { SystemEmpireTournaments::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireMessagesCol = { SystemEmpireMessages::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireNukeListCol = { SystemEmpireNukeList::EmpireKey, iEmpireKey };

    const TableCacheEntry entries[] = 
    {
        { { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &systemActiveGamesCol }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_TOURNAMENTS, NO_KEY, 1, &systemEmpireTournamentsCol }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &systemEmpireMessagesCol }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKER_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKED_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL, NULL },
    };

    return t_pCache->Cache(entries, countof(entries));
}

int GameEngine::CacheNukeHistory(unsigned int iEmpireKey)
{
    const ColumnEntry systemEmpireNukeListCol = { SystemEmpireNukeList::EmpireKey, iEmpireKey };

    const TableCacheEntry entries[] = 
    {
        { { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKER_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKED_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL, NULL },
    };

    return t_pCache->Cache(entries, countof(entries));
}

int GameEngine::CacheMutualAssociations(unsigned int iEmpireKey, unsigned int iSecondEmpireKey)
{
    const ColumnEntry systemEmpireAssoc1 = { SystemEmpireAssociations::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireAssoc2 = { SystemEmpireAssociations::EmpireKey, iSecondEmpireKey };

    const TableCacheEntry entries[] = 
    {
        { { SYSTEM_EMPIRE_ASSOCIATIONS, NO_KEY, 1, &systemEmpireAssoc1 }, NULL, NULL, NULL },
        { { SYSTEM_EMPIRE_ASSOCIATIONS, NO_KEY, 1, &systemEmpireAssoc2 }, NULL, NULL, NULL },
    };

    return t_pCache->Cache(entries, countof(entries));
}

int GameEngine::CacheForReload()
{
    const TableCacheEntry entries[] = 
    {
        { { SYSTEM_DATA, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_AVAILABILITY, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_GAMECLASS_DATA, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_ACTIVE_GAMES, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_THEMES, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_TOURNAMENTS, NO_KEY, 0, NULL }, NULL, NULL, NULL },
    };

    return t_pCache->Cache(entries, countof(entries));
}

int CreateEmptyIfNecessary(const char* pszTable, const char* pszCacheTable)
{
    if (!t_pCache->IsCached(pszCacheTable))
    {
        return t_pCache->CreateEmpty(pszTable, pszCacheTable);
    }
    return OK;
}

int GameEngine::CreateEmptyGameCacheEntries(int iGameClass, int iGameNumber, int iEmpireKey, int iTournamentKey, int iDiplomacyKey, int eFlags)
{
    int iErrCode = OK;
    Variant* pvEmpireKey = NULL, * pvFreeEmpireKey = NULL, vTemp;
    AutoFreeData free(pvFreeEmpireKey);
    unsigned int iNumEmpires;

    if (iDiplomacyKey != NO_KEY)
    {
        Assert(iGameClass != NO_KEY && iGameNumber > 0 && iEmpireKey == NO_KEY && iTournamentKey == NO_KEY);

        GET_GAME_EMPIRE_DIPLOMACY(strDipEmpires, iGameClass, iGameNumber, iDiplomacyKey);
        iErrCode = t_pCache->ReadColumn(strDipEmpires, GameEmpireDiplomacy::ReferenceEmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);
        pvFreeEmpireKey = pvEmpireKey;
    }
    else if (iEmpireKey != NO_KEY)
    {
        Assert(iGameClass == NO_KEY && iGameNumber == 0 && iTournamentKey == NO_KEY && iDiplomacyKey == NO_KEY);
        vTemp = iEmpireKey;
        pvEmpireKey = &vTemp;
        iNumEmpires = 1;
    }
    else if (iTournamentKey != NO_KEY)
    {
        Assert(iGameClass == NO_KEY && iGameNumber == 0 && iEmpireKey == NO_KEY && iDiplomacyKey == NO_KEY);

        GET_SYSTEM_TOURNAMENT_EMPIRES(strTournamentEmpires, iTournamentKey);
        iErrCode = t_pCache->ReadColumn(strTournamentEmpires, SystemTournamentEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);
        pvFreeEmpireKey = pvEmpireKey;
    }
    else
    {
        Assert(iGameClass != NO_KEY && iGameNumber > 0 && iEmpireKey == NO_KEY && iTournamentKey == NO_KEY && iDiplomacyKey == NO_KEY);

        GET_GAME_EMPIRES(strEmpires, iGameClass, iGameNumber);
        iErrCode = t_pCache->ReadColumn(strEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            return OK;
        }
        RETURN_ON_ERROR(iErrCode);
        pvFreeEmpireKey = pvEmpireKey;
    }

    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        unsigned int iThisEmpire = pvEmpireKey[i].GetInteger();

        if (eFlags & EMPTY_GAME_EMPIRE_DIPLOMACY)
        {
            GET_GAME_EMPIRE_DIPLOMACY(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_DIPLOMACY, strTable);
            RETURN_ON_ERROR(iErrCode);
        }

        if (eFlags & EMPTY_GAME_EMPIRE_MAP)
        {
            GET_GAME_EMPIRE_MAP(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_MAP, strTable);
            RETURN_ON_ERROR(iErrCode);
        }

        if (eFlags & EMPTY_GAME_EMPIRE_SHIPS)
        {
            GET_GAME_EMPIRE_SHIPS(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_SHIPS, strTable);
            RETURN_ON_ERROR(iErrCode);
        }

        if (eFlags & EMPTY_GAME_EMPIRE_FLEETS)
        {
            GET_GAME_EMPIRE_FLEETS(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_FLEETS, strTable);
            RETURN_ON_ERROR(iErrCode);
        }

        if (eFlags & EMPTY_GAME_EMPIRE_MESSAGES)
        {
            GET_GAME_EMPIRE_MESSAGES(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_MESSAGES, strTable);
            RETURN_ON_ERROR(iErrCode);
        }

        if (eFlags & EMPTY_SYSTEM_EMPIRE_MESSAGES)
        {
            GET_SYSTEM_EMPIRE_MESSAGES(strTable, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(SYSTEM_EMPIRE_MESSAGES, strTable);
            RETURN_ON_ERROR(iErrCode);
        }

        if (eFlags & EMPTY_SYSTEM_EMPIRE_ACTIVE_GAMES)
        {
            GET_SYSTEM_EMPIRE_ACTIVE_GAMES(strTable, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(SYSTEM_EMPIRE_ACTIVE_GAMES, strTable);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (iDiplomacyKey == NO_KEY && iEmpireKey == NO_KEY && (eFlags & EMPTY_GAME_EMPIRE_SHIPS))
    {
        GET_GAME_EMPIRE_SHIPS(strEmpireIndependentShips, iGameClass, iGameNumber, INDEPENDENT);
        if (!t_pCache->IsCached(strEmpireIndependentShips))
        {
            iErrCode = t_pCache->CreateEmpty(GAME_EMPIRE_SHIPS, strEmpireIndependentShips);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

int GameEngine::CacheForCheckAllGamesForUpdates()
{
    const TableCacheEntry entries[] =
    {
        { { SYSTEM_DATA, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_THEMES, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_ACTIVE_GAMES, NO_KEY, 0, NULL }, NULL, NULL, NULL },
        { { SYSTEM_GAMECLASS_DATA, NO_KEY, 0, NULL }, NULL, NULL, NULL }
    };

    int iErrCode = t_pCache->Cache(entries, countof(entries));
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::CacheSystemAlienIcons()
{
    const TableCacheEntry entries[] =
    {
        { { SYSTEM_ALIEN_ICONS, NO_KEY, 0, NULL }, NULL, NULL, NULL },
    };

    int iErrCode = t_pCache->Cache(entries, countof(entries));
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::CacheSystemAvailability()
{
    const TableCacheEntry entries[] =
    {
        { { SYSTEM_AVAILABILITY, NO_KEY, 0, NULL }, NULL, NULL, NULL },
    };

    int iErrCode = t_pCache->Cache(entries, countof(entries));
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}

int GameEngine::CacheGameEmpireTables(const Variant* pvGameClassGameNumber, unsigned int iNumGames)
{
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumGames * sizeof(TableCacheEntry));
    for (unsigned int i = 0; i < iNumGames; i ++)
    {
        int iGameClass, iGameNumber;
        GetGameClassGameNumber(pvGameClassGameNumber[i].GetCharPtr(), &iGameClass, &iGameNumber);

        ColumnEntry* pCols = (ColumnEntry*)StackAlloc(2 * sizeof(ColumnEntry));
        pCols[0].Name = GameEmpires::GameClass;
        pCols[0].Data = iGameClass;
        pCols[1].Name = GameEmpires::GameNumber;
        pCols[1].Data = iGameNumber;

        pcEntries[i].Table.Name = GAME_EMPIRES;
        pcEntries[i].Table.Key = NO_KEY;
        pcEntries[i].Table.NumColumns = 2;
        pcEntries[i].Table.Columns = pCols;
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;
    }

    int iErrCode = t_pCache->Cache(pcEntries, iNumGames);
    RETURN_ON_ERROR(iErrCode);
    return iErrCode;
}