#include "GameEngine.h"

int GameEngine::LookupEmpireByName(const char* pszName, unsigned int* piEmpireKey, Variant* pvName, int64* pi64SecretKey, ICachedTable** ppTable)
{
    const ColumnEntry col = { SystemEmpireData::Name, pszName };
    const TableCacheEntry entry = { { SYSTEM_EMPIRE_DATA, NO_KEY, 1, &col }, NULL, NULL };
    Assert(col.Data.GetCharPtr());

    ICachedTable* pEmpire = NULL;
    int iErrCode = t_pCache->Cache(entry, &pEmpire);
    if (iErrCode == OK)
    {
        iErrCode = pEmpire->GetNextKey(NO_KEY, piEmpireKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        else
        {
            if (iErrCode == OK && pvName)
            {
                iErrCode = pEmpire->ReadData(*piEmpireKey, SystemEmpireData::Name, pvName);
            }

            if (iErrCode == OK && pi64SecretKey)
            {
                iErrCode = pEmpire->ReadData(*piEmpireKey, SystemEmpireData::SecretKey, pi64SecretKey);
            }
        }
    }

    if (iErrCode == OK && ppTable)
    {
        *ppTable = pEmpire;
        pEmpire->AddRef();
    }

    SafeRelease(pEmpire);

    return iErrCode;
}

int GameEngine::CacheEmpire(unsigned int iEmpireKey)
{
    return CacheEmpires(&iEmpireKey, 1, NULL);
}

int GameEngine::CacheEmpire(unsigned int iEmpireKey, unsigned int* piResults)
{
    return CacheEmpires(&iEmpireKey, 1);
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
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;
    }
    
    int iErrCode = t_pCache->Cache(pcEntries, iNumEmpires);
    if (iErrCode == OK && piResults)
    {
        *piResults = 0;
        for (unsigned int i = 0; i < iNumEmpires; i ++)
        {
            unsigned int iNumRows;
            GET_SYSTEM_EMPIRE_DATA(strEmpire, piEmpireKey[i]) 
            iErrCode = t_pCache->GetNumCachedRows(strEmpire, &iNumRows);
            if (iErrCode != OK)
                break;
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
        { { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL }, NULL, NULL },
        { { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &systemEmpireMessagesCol }, NULL, NULL },
    };

    return t_pCache->Cache(systemEmpireEntries, countof(systemEmpireEntries));
}

int GameEngine::CacheEmpireMessagesAndTournaments(unsigned int iEmpireKey)
{
    const ColumnEntry systemEmpireMessagesCol = { SystemEmpireMessages::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireTournamentsCol = { SystemEmpireTournaments::EmpireKey, iEmpireKey };
    const TableCacheEntry systemEmpireEntries[] =
    {
        { { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL }, NULL, NULL },
        { { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &systemEmpireMessagesCol }, NULL, NULL },
        { { SYSTEM_EMPIRE_TOURNAMENTS, NO_KEY, 1, &systemEmpireTournamentsCol }, NULL, NULL },
    };

    return t_pCache->Cache(systemEmpireEntries, countof(systemEmpireEntries));
}

int GameEngine::CacheEmpireForDeletion(unsigned int iEmpireKey)
{
    const ColumnEntry systemEmpireActiveGamesCol = { SystemEmpireActiveGames::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireNukeListCol = { SystemEmpireNukeList::EmpireKey, iEmpireKey };
    const TableCacheEntry systemEmpireEntries[] =
    {
        { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &systemEmpireActiveGamesCol }, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKER_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKED_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL },
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
        pcEntries[i].PartitionColumn = NULL;
        pcEntries[i].CrossJoin = NULL;

        pcEntries[i + iNumTournaments].Table.Name = SYSTEM_TOURNAMENT_TEAMS;
        pcEntries[i + iNumTournaments].Table.Key = NO_KEY;
        pcEntries[i + iNumTournaments].Table.NumColumns = 1;
        pcEntries[i + iNumTournaments].Table.Columns = pcCols + i + iNumTournaments;
        pcEntries[i + iNumTournaments].PartitionColumn = NULL;
        pcEntries[i + iNumTournaments].CrossJoin = NULL;
    }

    return t_pCache->Cache(pcEntries, iNumTournaments * 2);
}

int GameEngine::CacheGameData(int* piGameClass, int* piGameNumber, int iEmpireKey, unsigned int iNumGames)
{
    const unsigned int cGameDataCols = 2;
    const unsigned int cGameEmpireDataCols = 3;

    const unsigned int cNumGameTables = 5;
    const unsigned int cNumGameEmpireTables = 1;
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
        pcEntries[i + iNumGames * 0].PartitionColumn = NULL;
        pcEntries[i + iNumGames * 0].CrossJoin = NULL;

        pcEntries[i + iNumGames * 1].Table.Name = GAME_EMPIRES;
		pcEntries[i + iNumGames * 1].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 1].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 1].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 1].PartitionColumn = NULL;
        pcEntries[i + iNumGames * 1].CrossJoin = NULL;

        pcEntries[i + iNumGames * 2].Table.Name = GAME_NUKED_EMPIRES;
		pcEntries[i + iNumGames * 2].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 2].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 2].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 2].PartitionColumn = NULL;
        pcEntries[i + iNumGames * 2].CrossJoin = NULL;

        pcEntries[i + iNumGames * 3].Table.Name = GAME_MAP;
		pcEntries[i + iNumGames * 3].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 3].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 3].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 3].PartitionColumn = NULL;
        pcEntries[i + iNumGames * 3].CrossJoin = NULL;

        pcEntries[i + iNumGames * 4].Table.Name = GAME_EMPIRE_DATA;
		pcEntries[i + iNumGames * 4].Table.Key = NO_KEY;
		pcEntries[i + iNumGames * 4].Table.NumColumns = cGameDataCols;
		pcEntries[i + iNumGames * 4].Table.Columns = pcGameCols + i*cGameDataCols;
        pcEntries[i + iNumGames * 4].PartitionColumn = GameEmpireData::EmpireKey;
        pcEntries[i + iNumGames * 4].CrossJoin = NULL;

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
        pcEntries[i + iNumGames * (cNumGameTables + 0)].PartitionColumn = NULL;
        pcEntries[i + iNumGames * (cNumGameTables + 0)].CrossJoin = NULL;
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
        pcEntries[iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[iNumEmpires + i].CrossJoin = NULL;

        ColumnEntry* pMessagesCol = (ColumnEntry*)StackAlloc(sizeof(ColumnEntry));
        pMessagesCol->Name = SystemEmpireMessages::EmpireKey;
        pMessagesCol->Data = piEmpireKey[i];

        pcEntries[2 * iNumEmpires + i].Table.Name = SYSTEM_EMPIRE_MESSAGES;
        pcEntries[2 * iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[2 * iNumEmpires + i].Table.NumColumns = 1;
        pcEntries[2 * iNumEmpires + i].Table.Columns = pMessagesCol;
        pcEntries[2 * iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[2 * iNumEmpires + i].CrossJoin = NULL;

        ColumnEntry* pNukeListCol = (ColumnEntry*)StackAlloc(sizeof(ColumnEntry));
        pNukeListCol->Name = SystemEmpireNukeList::EmpireKey;
        pNukeListCol->Data = piEmpireKey[i];

        pcEntries[3 * iNumEmpires + i].Table.Name = SYSTEM_EMPIRE_NUKER_LIST;
        pcEntries[3 * iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[3 * iNumEmpires + i].Table.NumColumns = 1;
        pcEntries[3 * iNumEmpires + i].Table.Columns = pNukeListCol;
        pcEntries[3 * iNumEmpires + i].PartitionColumn = NULL;
        pcEntries[3 * iNumEmpires + i].CrossJoin = NULL;

        pcEntries[4 * iNumEmpires + i].Table.Name = SYSTEM_EMPIRE_NUKED_LIST;
        pcEntries[4 * iNumEmpires + i].Table.Key = NO_KEY;
        pcEntries[4 * iNumEmpires + i].Table.NumColumns = 1;
        pcEntries[4 * iNumEmpires + i].Table.Columns = pNukeListCol;
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
        { { SYSTEM_ACTIVE_GAMES, NO_KEY, 0, NULL }, NULL, NULL },
        { { SYSTEM_LATEST_GAMES, NO_KEY, 0, NULL }, NULL, NULL },
        { { SYSTEM_NUKE_LIST, NO_KEY, 0, NULL }, NULL, NULL },

        { { GAME_DATA, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL },
        { { GAME_SECURITY, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL },
        { { GAME_EMPIRES, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL },
        { { GAME_NUKED_EMPIRES, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL },
        { { GAME_MAP, NO_KEY, countof(gameDataCols), gameDataCols }, NULL, NULL },

        { { GAME_EMPIRE_DATA, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireData::EmpireKey, NULL },
        { { GAME_EMPIRE_MESSAGES, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireMessages::EmpireKey, NULL },
        { { GAME_EMPIRE_MAP, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireMap::EmpireKey, NULL },
        { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireDiplomacy::EmpireKey, NULL },
        { { GAME_EMPIRE_SHIPS, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireShips::EmpireKey, NULL },
        { { GAME_EMPIRE_FLEETS, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireFleets::EmpireKey, NULL },
    };

    int iErrCode;

    Variant* pvEmpireKey = NULL;
    unsigned int iNumEmpires;

    // Try to save an I/O if we already have the empires table cached
    GET_GAME_EMPIRES(strEmpires, iGameClass, iGameNumber);
    if (t_pCache->IsCached(strEmpires))
    {
        iErrCode = t_pCache->ReadColumn(strEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
        if (iErrCode != OK)
        {
            goto Cleanup;
        }

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
            pEntries[countof(entries) + i].PartitionColumn = NULL;
            pEntries[countof(entries) + i].CrossJoin = NULL;

            pcColumns[i].Name = SystemEmpireActiveGames::EmpireKey;
            pcColumns[i].Data = pvEmpireKey[i];

            pEntries[countof(entries) + i + iNumEmpires].Table.Name = SYSTEM_EMPIRE_ACTIVE_GAMES;
            pEntries[countof(entries) + i + iNumEmpires].Table.Key = NO_KEY;
            pEntries[countof(entries) + i + iNumEmpires].Table.NumColumns = 1;
            pEntries[countof(entries) + i + iNumEmpires].Table.Columns = pcColumns + i;
            pEntries[countof(entries) + i + iNumEmpires].PartitionColumn = NULL;
            pEntries[countof(entries) + i + iNumEmpires].CrossJoin = NULL;

            pcColumns[i + iNumEmpires].Name = SystemEmpireMessages::EmpireKey;
            pcColumns[i + iNumEmpires].Data = pvEmpireKey[i];

            pEntries[countof(entries) + i + iNumEmpires * 2].Table.Name = SYSTEM_EMPIRE_MESSAGES;
            pEntries[countof(entries) + i + iNumEmpires * 2].Table.Key = NO_KEY;
            pEntries[countof(entries) + i + iNumEmpires * 2].Table.NumColumns = 1;
            pEntries[countof(entries) + i + iNumEmpires * 2].Table.Columns = pcColumns + i + iNumEmpires;
            pEntries[countof(entries) + i + iNumEmpires * 2].PartitionColumn = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 2].CrossJoin = NULL;

            pcColumns[i + iNumEmpires * 2].Name = SystemEmpireNukeList::EmpireKey;
            pcColumns[i + iNumEmpires * 2].Data = pvEmpireKey[i];

            pEntries[countof(entries) + i + iNumEmpires * 3].Table.Name = SYSTEM_EMPIRE_NUKED_LIST;
            pEntries[countof(entries) + i + iNumEmpires * 3].Table.Key = NO_KEY;
            pEntries[countof(entries) + i + iNumEmpires * 3].Table.NumColumns = 1;
            pEntries[countof(entries) + i + iNumEmpires * 3].Table.Columns = pcColumns + i + iNumEmpires * 2;
            pEntries[countof(entries) + i + iNumEmpires * 3].PartitionColumn = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 3].CrossJoin = NULL;

            pEntries[countof(entries) + i + iNumEmpires * 4].Table.Name = SYSTEM_EMPIRE_NUKER_LIST;
            pEntries[countof(entries) + i + iNumEmpires * 4].Table.Key = NO_KEY;
            pEntries[countof(entries) + i + iNumEmpires * 4].Table.NumColumns = 1;
            pEntries[countof(entries) + i + iNumEmpires * 4].Table.Columns = pcColumns + i + iNumEmpires * 2;
            pEntries[countof(entries) + i + iNumEmpires * 4].PartitionColumn = NULL;
            pEntries[countof(entries) + i + iNumEmpires * 4].CrossJoin = NULL;
        }

        int iErrCode = t_pCache->Cache(pEntries, iNumEntries);
        if (iErrCode != OK)
        {
            goto Cleanup;
        }
    }
    else
    {
        // Do it in two I/Os, sigh...
        iErrCode = t_pCache->Cache(entries, countof(entries));
        if (iErrCode != OK)
        {
            goto Cleanup;
        }

        iErrCode = t_pCache->ReadColumn(strEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
        if (iErrCode != OK)
        {
            goto Cleanup;
        }

        unsigned int* piEmpireKey = (unsigned int*)StackAlloc(iNumEmpires * sizeof(unsigned int));
        for (unsigned int i = 0; i < iNumEmpires; i ++)
        {
            piEmpireKey[i] = pvEmpireKey[i].GetInteger();
        }
        iErrCode = CacheEmpireActiveGamesMessagesNukeLists(piEmpireKey, iNumEmpires);
        if (iErrCode != OK)
        {
            goto Cleanup;
        }
    }

    Assert(pvEmpireKey && iNumEmpires);

    iErrCode = CreateEmptyGameCacheEntries(iGameClass, iGameNumber, NO_KEY, NO_KEY, 
                                           EMPTY_GAME_EMPIRE_MESSAGES | EMPTY_GAME_EMPIRE_MAP | EMPTY_GAME_EMPIRE_DIPLOMACY | 
                                           EMPTY_GAME_EMPIRE_SHIPS | EMPTY_GAME_EMPIRE_FLEETS);
    if (iErrCode != OK)
    {
        goto Cleanup;
    }

Cleanup:

    t_pCache->FreeData(pvEmpireKey);

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
        { { GAME_EMPIRE_DATA, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireData::EmpireKey, NULL },
        { { GAME_EMPIRE_DIPLOMACY, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireDiplomacy::EmpireKey, NULL },
        { { GAME_EMPIRE_MESSAGES, NO_KEY, countof(gameDataCols), gameDataCols }, GameEmpireMessages::EmpireKey, NULL },
    };

    int iErrCode = t_pCache->Cache(entries, countof(entries));
    if (iErrCode != OK)
        return iErrCode;

    // Create empty entries for all empires
    iErrCode = CreateEmptyGameCacheEntries(iGameClass, iGameNumber, NO_KEY, NO_KEY, EMPTY_GAME_EMPIRE_MESSAGES | EMPTY_GAME_EMPIRE_DIPLOMACY);
    if (iErrCode != OK)
        return iErrCode;

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
        { { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL }, NULL, NULL },
        { { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &systemActiveGamesCol }, NULL, NULL },
        { { SYSTEM_EMPIRE_TOURNAMENTS, NO_KEY, 1, &systemEmpireTournamentsCol }, NULL, NULL },
        { { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &systemEmpireMessagesCol }, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKER_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKED_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL },
    };

    return t_pCache->Cache(entries, countof(entries));
}

int GameEngine::CacheNukeHistory(unsigned int iEmpireKey)
{
    const ColumnEntry systemEmpireNukeListCol = { SystemEmpireNukeList::EmpireKey, iEmpireKey };

    const TableCacheEntry entries[] = 
    {
        { { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL }, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKER_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL },
        { { SYSTEM_EMPIRE_NUKED_LIST, NO_KEY, 1, &systemEmpireNukeListCol }, NULL, NULL },
    };

    return t_pCache->Cache(entries, countof(entries));
}

int GameEngine::CacheMutualAssociations(unsigned int iEmpireKey, unsigned int iSecondEmpireKey)
{
    const ColumnEntry systemEmpireAssoc1 = { SystemEmpireAssociations::EmpireKey, iEmpireKey };
    const ColumnEntry systemEmpireAssoc2 = { SystemEmpireAssociations::EmpireKey, iSecondEmpireKey };

    const TableCacheEntry entries[] = 
    {
        { { SYSTEM_EMPIRE_ASSOCIATIONS, NO_KEY, 1, &systemEmpireAssoc1 }, NULL, NULL },
        { { SYSTEM_EMPIRE_ASSOCIATIONS, NO_KEY, 1, &systemEmpireAssoc2 }, NULL, NULL },
    };

    return t_pCache->Cache(entries, countof(entries));
}

int GameEngine::CacheForReload()
{
    const TableCacheEntry entries[] = 
    {
        { { SYSTEM_DATA, NO_KEY, 0, NULL }, NULL, NULL },
        { { SYSTEM_GAMECLASS_DATA, NO_KEY, 0, NULL }, NULL, NULL },
        { { SYSTEM_ACTIVE_GAMES, NO_KEY, 0, NULL }, NULL, NULL },
        { { SYSTEM_TOURNAMENTS, NO_KEY, 0, NULL }, NULL, NULL },
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

int GameEngine::CreateEmptyGameCacheEntries(int iGameClass, int iGameNumber, int iEmpireKey, int iDiplomacyKey, int eFlags)
{
    int iErrCode;
    Variant* pvEmpireKey = NULL, vTemp;
    unsigned int iNumEmpires;

    Assert(iEmpireKey == NO_KEY || iDiplomacyKey == NO_KEY);

    if (iDiplomacyKey != NO_KEY)
    {
        GET_GAME_EMPIRE_DIPLOMACY(strDipEmpires, iGameClass, iGameNumber, iDiplomacyKey);
        iErrCode = t_pCache->ReadColumn(strDipEmpires, GameEmpireDiplomacy::ReferenceEmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
    }
    else if (iEmpireKey != NO_KEY)
    {
        vTemp = iEmpireKey;
        pvEmpireKey = &vTemp;
        iNumEmpires = 1;
        iErrCode = OK;
    }
    else
    {
        GET_GAME_EMPIRES(strEmpires, iGameClass, iGameNumber);
        iErrCode = t_pCache->ReadColumn(strEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
    }

    if (iErrCode != OK)
    {
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
        }
        goto Cleanup;
    }
    
    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        unsigned int iThisEmpire = pvEmpireKey[i].GetInteger();

        if (eFlags & EMPTY_GAME_EMPIRE_DIPLOMACY)
        {
            GET_GAME_EMPIRE_DIPLOMACY(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_DIPLOMACY, strTable);
            if (iErrCode != OK)
            {
                goto Cleanup;
            }
        }

        if (eFlags & EMPTY_GAME_EMPIRE_MAP)
        {
            GET_GAME_EMPIRE_MAP(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_MAP, strTable);
            if (iErrCode != OK)
            {
                goto Cleanup;
            }
        }

        if (eFlags & EMPTY_GAME_EMPIRE_SHIPS)
        {
            GET_GAME_EMPIRE_SHIPS(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_SHIPS, strTable);
            if (iErrCode != OK)
            {
                goto Cleanup;
            }
        }

        if (eFlags & EMPTY_GAME_EMPIRE_FLEETS)
        {
            GET_GAME_EMPIRE_FLEETS(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_FLEETS, strTable);
            if (iErrCode != OK)
            {
                goto Cleanup;
            }
        }

        if (eFlags & EMPTY_GAME_EMPIRE_MESSAGES)
        {
            GET_GAME_EMPIRE_MESSAGES(strTable, iGameClass, iGameNumber, iThisEmpire);
            iErrCode = CreateEmptyIfNecessary(GAME_EMPIRE_MESSAGES, strTable);
            if (iErrCode != OK)
            {
                goto Cleanup;
            }
        }
    }

    if (iDiplomacyKey == NO_KEY && iEmpireKey == NO_KEY && (eFlags & EMPTY_GAME_EMPIRE_SHIPS))
    {
        GET_GAME_EMPIRE_SHIPS(strEmpireIndependentShips, iGameClass, iGameNumber, INDEPENDENT);
        if (!t_pCache->IsCached(strEmpireIndependentShips))
        {
            iErrCode = t_pCache->CreateEmpty(GAME_EMPIRE_SHIPS, strEmpireIndependentShips);
            if (iErrCode != OK)
            {
                goto Cleanup;
            }
        }
    }

Cleanup:

    if (pvEmpireKey && pvEmpireKey != &vTemp)
        t_pCache->FreeData(pvEmpireKey);

    return iErrCode;
}
