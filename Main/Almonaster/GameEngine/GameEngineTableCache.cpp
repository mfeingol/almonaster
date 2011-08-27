#include "GameEngine.h"

int GameEngine::CacheEmpire(unsigned int iEmpireKey)
{
    return CacheEmpires(&iEmpireKey, 1, NULL);
}

int GameEngine::CacheEmpire(unsigned int iEmpireKey, unsigned int* piResults)
{
    return CacheEmpires(&iEmpireKey, 1);
}

int GameEngine::CacheEmpires(unsigned int* piEmpireKey, unsigned int iNumEmpires)
{
    return CacheEmpires(piEmpireKey, iNumEmpires, NULL);
}

int GameEngine::CacheEmpires(unsigned int* piEmpireKey, unsigned int iNumEmpires, unsigned int* piResults)
{
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumEmpires * sizeof(TableCacheEntry));
    for (unsigned int i = 0; i < iNumEmpires; i ++)
    {
        pcEntries[i].pszTableName = SYSTEM_EMPIRE_DATA;
        pcEntries[i].iKey = piEmpireKey[i];
        pcEntries[i].iNumColumns = 0;
        pcEntries[i].pcColumns = NULL;
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
    const TableCacheEntryColumn systemEmpireMessagesCol = { SystemEmpireMessages::EmpireKey, iEmpireKey };
    const TableCacheEntry systemEmpireEntries[] =
    {
        { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL },
        { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &systemEmpireMessagesCol},
    };

    return t_pCache->Cache(systemEmpireEntries, countof(systemEmpireEntries));
}

int GameEngine::CacheEmpireMessagesAndTournaments(unsigned int iEmpireKey)
{
    const TableCacheEntryColumn systemEmpireMessagesCol = { SystemEmpireMessages::EmpireKey, iEmpireKey };
    const TableCacheEntryColumn systemEmpireTournamentsCol = { SystemEmpireTournaments::EmpireKey, iEmpireKey };
    const TableCacheEntry systemEmpireEntries[] =
    {
        { SYSTEM_EMPIRE_DATA, iEmpireKey, 0, NULL },
        { SYSTEM_EMPIRE_MESSAGES, NO_KEY, 1, &systemEmpireMessagesCol},
        { SYSTEM_EMPIRE_TOURNAMENTS, NO_KEY, 1, &systemEmpireTournamentsCol},
    };

    return t_pCache->Cache(systemEmpireEntries, countof(systemEmpireEntries));
}

int GameEngine::CacheEmpireForDeletion(unsigned int iEmpireKey)
{
    const TableCacheEntryColumn systemEmpireActiveGamesCol = { SystemEmpireActiveGames::EmpireKey, iEmpireKey };
    const TableCacheEntryColumn systemEmpireNukeListCol = { SystemEmpireNukeList::EmpireKey, iEmpireKey };
    const TableCacheEntry systemEmpireEntries[] =
    {
        { SYSTEM_EMPIRE_ACTIVE_GAMES, NO_KEY, 1, &systemEmpireActiveGamesCol},
        { SYSTEM_EMPIRE_NUKER_LIST, NO_KEY, 1, &systemEmpireNukeListCol},
        { SYSTEM_EMPIRE_NUKED_LIST, NO_KEY, 1, &systemEmpireNukeListCol},
    };

    return t_pCache->Cache(systemEmpireEntries, countof(systemEmpireEntries));
}

int GameEngine::CacheTournamentTables(unsigned int* piTournamentKey, unsigned int iNumTournaments)
{
    TableCacheEntryColumn* pcCols = (TableCacheEntryColumn*)StackAlloc(iNumTournaments * 2 * sizeof(TableCacheEntryColumn));
    TableCacheEntry* pcEntries = (TableCacheEntry*)StackAlloc(iNumTournaments * 2 * sizeof(TableCacheEntry));
    for (unsigned int i = 0; i < iNumTournaments; i ++)
    {
        pcCols[i].pszColumn = SystemTournamentEmpires::TournamentKey;
        pcCols[i].vData = piTournamentKey[i];

        pcCols[i + iNumTournaments].pszColumn = SystemTournamentTeams::TournamentKey;
        pcCols[i + iNumTournaments].vData = piTournamentKey[i];

        pcEntries[i].pszTableName = SYSTEM_TOURNAMENT_EMPIRES;
        pcEntries[i].iKey = NO_KEY;
        pcEntries[i].iNumColumns = 1;
        pcEntries[i].pcColumns = pcCols + i;

        pcEntries[i + iNumTournaments].pszTableName = SYSTEM_TOURNAMENT_TEAMS;
        pcEntries[i + iNumTournaments].iKey = NO_KEY;
        pcEntries[i + iNumTournaments].iNumColumns = 1;
        pcEntries[i + iNumTournaments].pcColumns = pcCols + i + iNumTournaments;
    }

    return t_pCache->Cache(pcEntries, iNumTournaments * 2);
}