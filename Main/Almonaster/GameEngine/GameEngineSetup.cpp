//
// Almonaster.dll:  a component of Almonaster
// Copyright(c) 1998 Max Attar Feingold(maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or(at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "GameEngine.h"
#include "Global.h"

int GameEngine::Setup()
{
    int iErrCode;
    bool bNewDatabase, bGoodDatabase;
    const char* pszBadTable;

    /////////////////////////
    // Check system tables //
    /////////////////////////

    global.WriteReport(TRACE_ALWAYS, "GameEngine setup attempting to reuse an existing database");
    iErrCode = VerifySystemTables(&bNewDatabase, &bGoodDatabase, &pszBadTable);
    RETURN_ON_ERROR(iErrCode);

    if (bNewDatabase)
    {
        // Create a new database and we're done
        global.WriteReport(TRACE_ALWAYS, "Setting up new database");
        iErrCode = InitializeNewDatabase();
        RETURN_ON_ERROR(iErrCode);
        global.WriteReport(TRACE_ALWAYS, "Set up new database");
    }

    if (!bGoodDatabase)
    {
        // Bad database - report error 
        char* pszMessage = (char*)StackAlloc(strlen(pszBadTable) + 256);
        sprintf(pszMessage, "GameEngine setup found errors in the %s table", pszBadTable);

        global.WriteReport(TRACE_ERROR, pszMessage);
        global.WriteReport(TRACE_ERROR, "GameEngine setup could not successfully reuse an existing database");
        
        return ERROR_FAILURE;
    }

    // Reload the database
    return ReloadDatabase();
}

int GameEngine::ReloadDatabase()
{
    int iErrCode;

    //
    // Cache tables
    //

    iErrCode = CacheForReload();
    RETURN_ON_ERROR(iErrCode);

    //
    // System
    //

    iErrCode = VerifySystem();
    RETURN_ON_ERROR(iErrCode);
    global.WriteReport(TRACE_ALWAYS, "GameEngine setup successfully verified system data");

    //
    // Games
    //

    iErrCode = VerifyActiveGames();
    RETURN_ON_ERROR(iErrCode);
    global.WriteReport(TRACE_ALWAYS, "GameEngine setup successfully verified active games");    

    //
    // Marked gameclasses
    //

    iErrCode = VerifyMarkedGameClasses();
    RETURN_ON_ERROR(iErrCode);
    global.WriteReport(TRACE_ALWAYS, "GameEngine setup successfully verified marked gameclasses");

    //
    // Tournaments
    //

    iErrCode = VerifyTournaments();
    RETURN_ON_ERROR(iErrCode);
    global.WriteReport(TRACE_ALWAYS, "GameEngine setup successfully verified tournaments");

    //////////
    // Done //
    //////////

    global.WriteReport(TRACE_ALWAYS, "GameEngine setup successfully reused the existing database");

    return iErrCode;
}

int GameEngine::VerifyTableExistence(const char* pszTable, bool bNewDatabase, bool* pbGood)
{
    bool bTableExists;
    int iErrCode = t_pConn->DoesTableExist(pszTable, &bTableExists);
    RETURN_ON_ERROR(iErrCode);

    *pbGood = (!bTableExists || !bNewDatabase) && (bTableExists || bNewDatabase);
    return OK;
}

int GameEngine::VerifyTableExistenceWithRows(const char* pszTable, bool bNewDatabase, bool* pbGood)
{
    bool bTableExists;
    int iErrCode = t_pConn->DoesTableExist(pszTable, &bTableExists);
    RETURN_ON_ERROR(iErrCode);
    
    if ((bTableExists && bNewDatabase) || (!bTableExists && !bNewDatabase))
    {
        *pbGood = false;
        return OK;
    }

    if (bTableExists)
    {
        unsigned int iNumRows;
        int iErrCode = t_pConn->GetNumPhysicalRows(pszTable, &iNumRows);
        Assert(iErrCode == OK);

        if (iNumRows < 1)
        {
            *pbGood = false;
            return OK;
        }
    }

    *pbGood = true;
    return OK;
}

int GameEngine::VerifySystemTables(bool* pbNewDatabase, bool* pbGoodDatabase, const char** ppszBadTable)
{
    bool bNewDatabase = false, bGoodDatabase = true;

    // SystemData
    Assert(countof(SystemData::Types) == SystemData::NumColumns);
    Assert(countof(SystemData::Sizes) == SystemData::NumColumns);
    Assert(countof(SystemData::ColumnNames) == SystemData::NumColumns);

    const char* pszTable = SYSTEM_DATA;
    bool bExists;
    int iErrCode = t_pConn->DoesTableExist(pszTable, &bExists);
    RETURN_ON_ERROR(iErrCode);

    if (bExists)
    {
        unsigned int iNumRows;
        if (t_pConn->GetNumPhysicalRows(pszTable, &iNumRows) != OK || iNumRows != 1)
        {
            bGoodDatabase = false;
            goto Cleanup;
        }
    }
    else
    {
        bNewDatabase = true;
    }

    // SystemAvailability
    Assert(countof(SystemAvailability::Types) == SystemAvailability::NumColumns);
    Assert(countof(SystemAvailability::Sizes) == SystemAvailability::NumColumns);
    Assert(countof(SystemAvailability::ColumnNames) == SystemAvailability::NumColumns);

    pszTable = SYSTEM_AVAILABILITY;
    iErrCode = VerifyTableExistenceWithRows(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;
    
    // SystemEmpireData
    Assert(countof(SystemEmpireData::Types) == SystemEmpireData::NumColumns);
    Assert(countof(SystemEmpireData::Sizes) == SystemEmpireData::NumColumns);
    Assert(countof(SystemEmpireData::ColumnNames) == SystemEmpireData::NumColumns);

    pszTable = SYSTEM_EMPIRE_DATA;
    iErrCode = VerifyTableExistenceWithRows(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemGameClassData
    Assert(countof(SystemGameClassData::Types) == SystemGameClassData::NumColumns);
    Assert(countof(SystemGameClassData::Sizes) == SystemGameClassData::NumColumns);
    Assert(countof(SystemGameClassData::ColumnNames) == SystemGameClassData::NumColumns);

    pszTable = SYSTEM_GAMECLASS_DATA;
    iErrCode = VerifyTableExistenceWithRows(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemAlienIcons
    Assert(countof(SystemAlienIcons::Types) == SystemAlienIcons::NumColumns);
    Assert(countof(SystemAlienIcons::Sizes) == SystemAlienIcons::NumColumns);
    Assert(countof(SystemAlienIcons::ColumnNames) == SystemAlienIcons::NumColumns);

    pszTable = SYSTEM_ALIEN_ICONS;
    iErrCode = VerifyTableExistenceWithRows(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemSuperClassData
    Assert(countof(SystemSuperClassData::Types) == SystemSuperClassData::NumColumns);
    Assert(countof(SystemSuperClassData::Sizes) == SystemSuperClassData::NumColumns);
    Assert(countof(SystemSuperClassData::ColumnNames) == SystemSuperClassData::NumColumns);

    pszTable = SYSTEM_SUPERCLASS_DATA;
    iErrCode = VerifyTableExistenceWithRows(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;
    
    // SystemThemes
    Assert(countof(SystemThemes::Types) == SystemThemes::NumColumns);
    Assert(countof(SystemThemes::Sizes) == SystemThemes::NumColumns);
    Assert(countof(SystemThemes::ColumnNames) == SystemThemes::NumColumns);

    pszTable = SYSTEM_THEMES;
    iErrCode = VerifyTableExistenceWithRows(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;
    
    // SystemActiveGames
    Assert(countof(SystemActiveGames::Types) == SystemActiveGames::NumColumns);
    Assert(countof(SystemActiveGames::Sizes) == SystemActiveGames::NumColumns);
    Assert(countof(SystemActiveGames::ColumnNames) == SystemActiveGames::NumColumns);

    pszTable = SYSTEM_ACTIVE_GAMES;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemChatroomData
    Assert(countof(SystemChatroomData::Types) == SystemChatroomData::NumColumns);
    Assert(countof(SystemChatroomData::Sizes) == SystemChatroomData::NumColumns);
    Assert(countof(SystemChatroomData::ColumnNames) == SystemChatroomData::NumColumns);

    pszTable = SYSTEM_CHATROOM_DATA;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemTournaments
    Assert(countof(SystemTournaments::Types) == SystemTournaments::NumColumns);
    Assert(countof(SystemTournaments::Sizes) == SystemTournaments::NumColumns);
    Assert(countof(SystemTournaments::ColumnNames) == SystemTournaments::NumColumns);

    pszTable = SYSTEM_TOURNAMENTS;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemNukeList
    Assert(countof(SystemNukeList::Types) == SystemNukeList::NumColumns);
    Assert(countof(SystemNukeList::Sizes) == SystemNukeList::NumColumns);
    Assert(countof(SystemNukeList::ColumnNames) == SystemNukeList::NumColumns);

    pszTable = SYSTEM_NUKE_LIST;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemLatestGames
    Assert(countof(SystemLatestGames::Types) == SystemLatestGames::NumColumns);
    Assert(countof(SystemLatestGames::Sizes) == SystemLatestGames::NumColumns);
    Assert(countof(SystemLatestGames::ColumnNames) == SystemLatestGames::NumColumns);

    pszTable = SYSTEM_LATEST_GAMES;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;
    
    // SystemEmpireMessages
    Assert(countof(SystemEmpireMessages::Types) == SystemEmpireMessages::NumColumns);
    Assert(countof(SystemEmpireMessages::Sizes) == SystemEmpireMessages::NumColumns);
    Assert(countof(SystemEmpireMessages::ColumnNames) == SystemEmpireMessages::NumColumns);

    pszTable = SYSTEM_EMPIRE_MESSAGES;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemEmpireAssociations
    Assert(countof(SystemEmpireAssociations::Types) == SystemEmpireAssociations::NumColumns);
    Assert(countof(SystemEmpireAssociations::Sizes) == SystemEmpireAssociations::NumColumns);
    Assert(countof(SystemEmpireAssociations::ColumnNames) == SystemEmpireAssociations::NumColumns);

    pszTable = SYSTEM_EMPIRE_ASSOCIATIONS;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemEmpireNukerList
    Assert(countof(SystemEmpireNukeList::Types) == SystemEmpireNukeList::NumColumns);
    Assert(countof(SystemEmpireNukeList::Sizes) == SystemEmpireNukeList::NumColumns);
    Assert(countof(SystemEmpireNukeList::ColumnNames) == SystemEmpireNukeList::NumColumns);

    pszTable = SYSTEM_EMPIRE_NUKER_LIST;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemEmpireNukedList
    Assert(countof(SystemEmpireNukeList::Types) == SystemEmpireNukeList::NumColumns);
    Assert(countof(SystemEmpireNukeList::Sizes) == SystemEmpireNukeList::NumColumns);
    Assert(countof(SystemEmpireNukeList::ColumnNames) == SystemEmpireNukeList::NumColumns);

    pszTable = SYSTEM_EMPIRE_NUKED_LIST;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemEmpireActiveGames
    Assert(countof(SystemEmpireActiveGames::Types) == SystemEmpireActiveGames::NumColumns);
    Assert(countof(SystemEmpireActiveGames::Sizes) == SystemEmpireActiveGames::NumColumns);
    Assert(countof(SystemEmpireActiveGames::ColumnNames) == SystemEmpireActiveGames::NumColumns);

    pszTable = SYSTEM_EMPIRE_ACTIVE_GAMES;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemTournamentTeams
    Assert(countof(SystemTournamentTeams::Types) == SystemTournamentTeams::NumColumns);
    Assert(countof(SystemTournamentTeams::Sizes) == SystemTournamentTeams::NumColumns);
    Assert(countof(SystemTournamentTeams::ColumnNames) == SystemTournamentTeams::NumColumns);

    pszTable = SYSTEM_TOURNAMENT_TEAMS;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemTournamentEmpires
    Assert(countof(SystemTournamentEmpires::Types) == SystemTournamentEmpires::NumColumns);
    Assert(countof(SystemTournamentEmpires::Sizes) == SystemTournamentEmpires::NumColumns);
    Assert(countof(SystemTournamentEmpires::ColumnNames) == SystemTournamentEmpires::NumColumns);

    pszTable = SYSTEM_TOURNAMENT_EMPIRES;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // SystemEmpireTournaments
    Assert(countof(SystemEmpireTournaments::Types) == SystemEmpireTournaments::NumColumns);
    Assert(countof(SystemEmpireTournaments::Sizes) == SystemEmpireTournaments::NumColumns);
    Assert(countof(SystemEmpireTournaments::ColumnNames) == SystemEmpireTournaments::NumColumns);

    pszTable = SYSTEM_EMPIRE_TOURNAMENTS;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    //
    // Game tables
    //

    // GameData
    Assert(countof(GameData::Types) == GameData::NumColumns);
    Assert(countof(GameData::Sizes) == GameData::NumColumns);
    Assert(countof(GameData::ColumnNames) == GameData::NumColumns);

    pszTable = GAME_DATA;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;
    
    // GameEmpires
    Assert(countof(GameEmpires::Types) == GameEmpires::NumColumns);
    Assert(countof(GameEmpires::Sizes) == GameEmpires::NumColumns);
    Assert(countof(GameEmpires::ColumnNames) == GameEmpires::NumColumns);

    pszTable = GAME_EMPIRES;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameNukedEmpires
    Assert(countof(GameNukedEmpires::Types) == GameNukedEmpires::NumColumns);
    Assert(countof(GameNukedEmpires::Sizes) == GameNukedEmpires::NumColumns);
    Assert(countof(GameNukedEmpires::ColumnNames) == GameNukedEmpires::NumColumns);

    pszTable = GAME_NUKED_EMPIRES;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameMap
    Assert(countof(GameMap::Types) == GameMap::NumColumns);
    Assert(countof(GameMap::Sizes) == GameMap::NumColumns);
    Assert(countof(GameMap::ColumnNames) == GameMap::NumColumns);

    pszTable = GAME_MAP;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameEmpireData
    Assert(countof(GameEmpireData::Types) == GameEmpireData::NumColumns);
    Assert(countof(GameEmpireData::Sizes) == GameEmpireData::NumColumns);
    Assert(countof(GameEmpireData::ColumnNames) == GameEmpireData::NumColumns);

    pszTable = GAME_DATA;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameEmpireMessages
    Assert(countof(GameEmpireMessages::Types) == GameEmpireMessages::NumColumns);
    Assert(countof(GameEmpireMessages::Sizes) == GameEmpireMessages::NumColumns);
    Assert(countof(GameEmpireMessages::ColumnNames) == GameEmpireMessages::NumColumns);

    pszTable = GAME_EMPIRE_MESSAGES;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameEmpireMap
    Assert(countof(GameEmpireMap::Types) == GameEmpireMap::NumColumns);
    Assert(countof(GameEmpireMap::Sizes) == GameEmpireMap::NumColumns);
    Assert(countof(GameEmpireMap::ColumnNames) == GameEmpireMap::NumColumns);
    
    pszTable = GAME_EMPIRE_MAP;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameEmpireDiplomacy
    Assert(countof(GameEmpireDiplomacy::Types) == GameEmpireDiplomacy::NumColumns);
    Assert(countof(GameEmpireDiplomacy::Sizes) == GameEmpireDiplomacy::NumColumns);
    Assert(countof(GameEmpireDiplomacy::ColumnNames) == GameEmpireDiplomacy::NumColumns);

    pszTable = GAME_EMPIRE_DIPLOMACY;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameEmpireShips
    Assert(countof(GameEmpireShips::Types) == GameEmpireShips::NumColumns);
    Assert(countof(GameEmpireShips::Sizes) == GameEmpireShips::NumColumns);
    Assert(countof(GameEmpireShips::ColumnNames) == GameEmpireShips::NumColumns);

    pszTable = GAME_EMPIRE_SHIPS;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameEmpireFleets
    Assert(countof(GameEmpireFleets::Types) == GameEmpireFleets::NumColumns);
    Assert(countof(GameEmpireFleets::Sizes) == GameEmpireFleets::NumColumns);
    Assert(countof(GameEmpireFleets::ColumnNames) == GameEmpireFleets::NumColumns);

    pszTable = GAME_EMPIRE_FLEETS;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

    // GameSecurity
    Assert(countof(GameSecurity::Types) == GameSecurity::NumColumns);
    Assert(countof(GameSecurity::Sizes) == GameSecurity::NumColumns);
    Assert(countof(GameSecurity::ColumnNames) == GameSecurity::NumColumns);

    pszTable = GAME_SECURITY;
    iErrCode = VerifyTableExistence(pszTable, bNewDatabase, &bGoodDatabase);
    RETURN_ON_ERROR(iErrCode);
    if (!bGoodDatabase)
        goto Cleanup;

Cleanup:

    *pbNewDatabase = bNewDatabase;
    *pbGoodDatabase = bGoodDatabase;
    *ppszBadTable = pszTable;

    return OK;
}

int GameEngine::VerifySystem() {

    int iErrCode;
    Variant vTemp;

    int iOptions;
    iErrCode = GetSystemOptions(&iOptions);
    RETURN_ON_ERROR(iErrCode);

    if (!(iOptions & ACCESS_ENABLED)) {

        iErrCode = GetSystemProperty(SystemData::AccessDisabledReason, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if (strcmp(vTemp.GetString(), BACKUP_BLOCK_REASON) == 0) {

            iErrCode = SetSystemProperty(SystemData::AccessDisabledReason,(const char*) NULL);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = SetSystemOption(ACCESS_ENABLED, true);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    if (!(iOptions & NEW_EMPIRES_ENABLED)) {

        iErrCode = GetSystemProperty(SystemData::NewEmpiresDisabledReason, &vTemp);
        RETURN_ON_ERROR(iErrCode);

        if (strcmp(vTemp.GetString(), BACKUP_BLOCK_REASON) == 0) {

            iErrCode = SetSystemProperty(SystemData::NewEmpiresDisabledReason,(const char*) NULL);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = SetSystemOption(NEW_EMPIRES_ENABLED, true);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

int GameEngine::VerifyMarkedGameClasses() {

    int iErrCode;
    unsigned int* piGameClassKey = NULL, iNumGameClasses = 0, i;
    AutoFreeKeys free(piGameClassKey);

    iErrCode = t_pCache->GetAllKeys(SYSTEM_GAMECLASS_DATA, &piGameClassKey, &iNumGameClasses);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        return OK;
    }
    RETURN_ON_ERROR(iErrCode);
    
    for (i = 0; i < iNumGameClasses; i ++)
    {
        Variant vOptions;
        iErrCode = GetGameClassProperty(piGameClassKey[i], SystemGameClassData::Options, &vOptions);
        RETURN_ON_ERROR(iErrCode);

        if (vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION)
        {
            Variant vActiveGames;
            iErrCode = GetGameClassProperty(piGameClassKey[i], SystemGameClassData::NumActiveGames, &vActiveGames);
            RETURN_ON_ERROR(iErrCode);

            if (vActiveGames.GetInteger() > 0)
            {
                // This game class needs to be deleted
                bool bDeleted;
                iErrCode = DeleteGameClass(piGameClassKey[i], &bDeleted);
                RETURN_ON_ERROR(iErrCode);
                Assert(bDeleted);
                
                if (bDeleted) {

                    char pszBuffer [128];
                    sprintf(
                        pszBuffer,
                        "GameEngine setup deleted gameclass %i because it was marked for deletion",
                        piGameClassKey[i]
                        );
                    global.WriteReport(TRACE_WARNING, pszBuffer);
                }
            }
        }
    }

    return iErrCode;
}


int GameEngine::VerifyActiveGames()
{
    int iErrCode;

    UTCTime tCurrentTime;
    Time::GetTime(&tCurrentTime);

    // Read some system data
    Variant vTemp;
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::SecondsForLongtermStatus, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    Seconds sSecondsForLongtermStatus = vTemp.GetInteger();

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::NumUpdatesDownBeforeGameIsKilled, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    int iNumUpdatesDownBeforeGameIsKilled = vTemp.GetInteger();

    // Get game last update check time
    iErrCode = t_pCache->ReadData(SYSTEM_AVAILABILITY, SystemAvailability::LastAvailableTime, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    UTCTime tLastAvailableTime = vTemp.GetInteger64();

    // Get active games
    Variant** ppvActiveGame = NULL;
    AutoFreeData free_ppvActiveGame(ppvActiveGame);
    unsigned int iNumGames;

    iErrCode = GetActiveGames(&ppvActiveGame, &iNumGames);
    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    else
    {
        RETURN_ON_ERROR(iErrCode);

        // Loop through all games
        for (unsigned int i = 0; i < iNumGames; i ++)
        {
            int iGameClass = ppvActiveGame[i][0].GetInteger();
            int iGameNumber = ppvActiveGame[i][1].GetInteger();

            // We could be a bit more selective here, but it's okay for now
            iErrCode = CacheAllGameTables(iGameClass, iGameNumber);
            RETURN_ON_ERROR(iErrCode);

            GET_GAME_DATA(strGameData, iGameClass, iGameNumber);
            GET_GAME_EMPIRES(strGameEmpires, iGameClass, iGameNumber);

            // Get game update period
            iErrCode = t_pCache->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            Seconds sPeriod = vTemp.GetInteger();
        
            // Get game state
            iErrCode = t_pCache->ReadData(strGameData, GameData::State, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            int iState = vTemp.GetInteger();

            bool bPaused = (iState & PAUSED) ||(iState & ADMIN_PAUSED);
            bool bStarted = (iState & STARTED) != 0;
        
            // Reset state
            iErrCode = t_pCache->WriteAnd(strGameData, GameData::State, ~GAME_BUSY);
            RETURN_ON_ERROR(iErrCode);
        
            // Get num empires
            unsigned int iNumEmpires;
            iErrCode = t_pCache->GetNumCachedRows(strGameEmpires, &iNumEmpires);
            RETURN_ON_ERROR(iErrCode);

            // Is password protected?
            bool bPasswordProtected;
            iErrCode = IsGamePasswordProtected(iGameClass, iGameNumber, &bPasswordProtected);
            RETURN_ON_ERROR(iErrCode);

            // Get last update time
            iErrCode = t_pCache->ReadData(strGameData, GameData::LastUpdateTime, &vTemp);
            RETURN_ON_ERROR(iErrCode);
            UTCTime tLastUpdateTime = vTemp.GetInteger64();

            // If started and not paused, reset last update time to current time minus (last available time minus last update time)
            if (bStarted && !bPaused)
            {
                // Write a new last update time
                Seconds sConsumedTime = Time::GetSecondDifference(tLastAvailableTime, tLastUpdateTime);
                if (sConsumedTime > 0)
                {
                    UTCTime tNewTime;
                    Time::SubtractSeconds(tCurrentTime, sConsumedTime, &tNewTime);

                    iErrCode = t_pCache->WriteData(strGameData, GameData::LastUpdateTime, tNewTime);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        
            //
            // Update empires' last login settings
            //

            Variant* pvEmpireKey = NULL;
            AutoFreeData free_pvEmpireKey(pvEmpireKey);

            iErrCode = t_pCache->ReadColumn(strGameEmpires, GameEmpires::EmpireKey, NULL, &pvEmpireKey, &iNumEmpires);
            RETURN_ON_ERROR(iErrCode);

            for (unsigned int j = 0; j < iNumEmpires; j ++)
            {
                unsigned int iEmpireKey = pvEmpireKey[j].GetInteger();
                GET_GAME_EMPIRE_DATA(strEmpireData, iGameClass, iGameNumber, iEmpireKey);

                iErrCode = t_pCache->ReadData(strEmpireData, GameEmpireData::LastLogin, &vTemp);
                RETURN_ON_ERROR(iErrCode);
                UTCTime tLastLoginTime = vTemp.GetInteger64();
            
                Seconds sConsumedTime = Time::GetSecondDifference(tLastAvailableTime, tLastLoginTime);
                if (sConsumedTime > 0)
                {
                    UTCTime tNewTime;
                    Time::SubtractSeconds(tCurrentTime, sConsumedTime, &tNewTime);
            
                    iErrCode = t_pCache->WriteData(strEmpireData, GameEmpireData::LastLogin, tNewTime);
                    RETURN_ON_ERROR(iErrCode);
                }
            }
        
            // Delete the game if it's in an 'interrupted' state
            if (iState & GAME_BUSY)
            {
                int iReason = iState & ~GAME_DELETION_REASON_MASK;
                Assert(iReason != 0);

                iErrCode = DeleteGame(iGameClass, iGameNumber, SYSTEM, "", iReason);
                RETURN_ON_ERROR(iErrCode);

                char pszBuffer [512];
                sprintf(
                    pszBuffer,
                    "GameEngine setup deleted game %i of gameclass %i because it was in "\
                    "inconsistent state %i",
                    iGameNumber,
                    iGameClass,
                    iReason
                    );

                global.WriteReport(TRACE_WARNING, pszBuffer);
                continue;
            }

            // Game should be killed if it's not paused and it's not a longterm and 
            // more than x updates have transpired while the server was down
            Seconds sElapsedTime = Time::GetSecondDifference(tCurrentTime, tLastAvailableTime);

            if (!bPaused &&
                sPeriod < sSecondsForLongtermStatus && 
                sElapsedTime > sPeriod * iNumUpdatesDownBeforeGameIsKilled) {

                iErrCode = DeleteGame(iGameClass, iGameNumber, SYSTEM, "", SYSTEM_SHUTDOWN);
                RETURN_ON_ERROR(iErrCode);

                char pszBuffer [512];
                sprintf(
                    pszBuffer,
                    "GameEngine setup deleted game %i of gameclass %i "\
                    "because it grew stale during a system shutdown",
                    iGameNumber,
                    iGameClass
                    );

                global.WriteReport(TRACE_WARNING, pszBuffer);
                continue;
            }

            // If game hasn't started and is password protected and has only one empire, kill it                    
            if (!(iState & STARTED) && bPasswordProtected && iNumEmpires == 1) {
            
                iErrCode = DeleteGame(iGameClass, iGameNumber, SYSTEM, "", PASSWORD_PROTECTED);
                RETURN_ON_ERROR(iErrCode);

                char pszBuffer [512];
                sprintf(
                    pszBuffer,
                    "GameEngine setup deleted game %i of gameclass %i "\
                    "because it was password protected and only contained one empire",
                    iGameNumber,
                    iGameClass
                    );
                
                global.WriteReport(TRACE_WARNING, pszBuffer);
                continue;
            }

            // Finally, check the game for updates
            bool bUpdate;
            iErrCode = CheckGameForUpdates(iGameClass, iGameNumber, &bUpdate);
            RETURN_ON_ERROR(iErrCode);
        }
    }

    return iErrCode;
}

//
// Creation
//

int GameEngine::InitializeNewDatabase()
{
    int iErrCode;

    global.WriteReport(TRACE_ALWAYS, "GameEngine setup is initializing a new database");

    iErrCode = CreateDefaultSystemTables();
    RETURN_ON_ERROR(iErrCode);

    iErrCode = SetupDefaultSystemTables();
    RETURN_ON_ERROR(iErrCode);

    iErrCode = SetupDefaultSystemGameClasses();
    RETURN_ON_ERROR(iErrCode);
    
    global.WriteReport(TRACE_ALWAYS, "GameEngine setup finished initializing a new database");
    return iErrCode;
}

//
// Create the default system tables
//
int GameEngine::CreateDefaultSystemTables()
{
    int iErrCode;

    // Create SystemData table
    iErrCode = t_pCache->CreateTable(SYSTEM_DATA, SystemData::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemAvailability table
    iErrCode = t_pCache->CreateTable(SYSTEM_AVAILABILITY, SystemAvailability::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemGameClassData table
    iErrCode = t_pCache->CreateTable(SYSTEM_GAMECLASS_DATA, SystemGameClassData::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemActiveGames table
    iErrCode = t_pCache->CreateTable(SYSTEM_ACTIVE_GAMES, SystemActiveGames::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemEmpireData table
    iErrCode = t_pCache->CreateTable(SYSTEM_EMPIRE_DATA, SystemEmpireData::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemThemes table
    iErrCode = t_pCache->CreateTable(SYSTEM_THEMES, SystemThemes::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemSuperClassData table
    iErrCode = t_pCache->CreateTable(SYSTEM_SUPERCLASS_DATA, SystemSuperClassData::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemAlienIcons table
    iErrCode = t_pCache->CreateTable(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemNukeList table
    iErrCode = t_pCache->CreateTable(SYSTEM_NUKE_LIST, SystemNukeList::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemLatestGames table
    iErrCode = t_pCache->CreateTable(SYSTEM_LATEST_GAMES, SystemLatestGames::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemChatroomData table
    iErrCode = t_pCache->CreateTable(SYSTEM_CHATROOM_DATA, SystemChatroomData::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemEmpireActiveGames table
    iErrCode = t_pCache->CreateTable(SYSTEM_EMPIRE_ACTIVE_GAMES, SystemEmpireActiveGames::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemEmpireMessages table
    iErrCode = t_pCache->CreateTable(SYSTEM_EMPIRE_MESSAGES, SystemEmpireMessages::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemEmpireAssociations table
    iErrCode = t_pCache->CreateTable(SYSTEM_EMPIRE_ASSOCIATIONS, SystemEmpireAssociations::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemEmpireTournaments table
    iErrCode = t_pCache->CreateTable(SYSTEM_EMPIRE_TOURNAMENTS, SystemEmpireTournaments::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemTournaments table
    iErrCode = t_pCache->CreateTable(SYSTEM_TOURNAMENTS, SystemTournaments::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemTournamentEmpires table
    iErrCode = t_pCache->CreateTable(SYSTEM_TOURNAMENT_EMPIRES, SystemTournamentEmpires::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemTournamentTeams table
    iErrCode = t_pCache->CreateTable(SYSTEM_TOURNAMENT_TEAMS, SystemTournamentTeams::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemEmpireNukerList table
    iErrCode = t_pCache->CreateTable(SYSTEM_EMPIRE_NUKER_LIST, SystemEmpireNukeList::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create SystemEmpireNukedList table
    iErrCode = t_pCache->CreateTable(SYSTEM_EMPIRE_NUKED_LIST, SystemEmpireNukeList::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameData table
    iErrCode = t_pCache->CreateTable(GAME_DATA, GameData::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameSecurity table
    iErrCode = t_pCache->CreateTable(GAME_SECURITY, GameSecurity::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameEmpires table
    iErrCode = t_pCache->CreateTable(GAME_EMPIRES, GameEmpires::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameNukedEmpires table
    iErrCode = t_pCache->CreateTable(GAME_NUKED_EMPIRES, GameNukedEmpires::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameMap table
    iErrCode = t_pCache->CreateTable(GAME_MAP, GameMap::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameEmpireData table
    iErrCode = t_pCache->CreateTable(GAME_EMPIRE_DATA, GameEmpireData::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameEmpireMessages table
    iErrCode = t_pCache->CreateTable(GAME_EMPIRE_MESSAGES, GameEmpireMessages::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameEmpireMap table
    iErrCode = t_pCache->CreateTable(GAME_EMPIRE_MAP, GameEmpireMap::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameEmpireDiplomacy table
    iErrCode = t_pCache->CreateTable(GAME_EMPIRE_DIPLOMACY, GameEmpireDiplomacy::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameEmpireShips table
    iErrCode = t_pCache->CreateTable(GAME_EMPIRE_SHIPS, GameEmpireShips::Template);
    RETURN_ON_ERROR(iErrCode);

    // Create GameEmpireFleets table
    iErrCode = t_pCache->CreateTable(GAME_EMPIRE_FLEETS, GameEmpireFleets::Template);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Insert default data into the system tables
int GameEngine::SetupDefaultSystemTables()
{
    int iErrCode;
    unsigned int iKey;

    UTCTime tTime;
    Time::GetTime(&tTime);

    // Set up themes first
    unsigned int iDefaultThemeKey;
    iErrCode = SetupDefaultThemes(&iDefaultThemeKey);
    RETURN_ON_ERROR(iErrCode);

    // Insert data into SYSTEM_DATA
    Variant pvSystemData [SystemData::NumColumns] = {
        3,              // DefaultAlien
        "Cortegana",    // ServerName
        10,             // DefaultMaxNumSystemMessages
        10,             // DefaultMaxNumGameMessages
        iDefaultThemeKey, // DefaultUIButtons
        iDefaultThemeKey, // DefaultUIBackground
        iDefaultThemeKey, // DefaultUILivePlanet
        iDefaultThemeKey, // DefaultUIDeadPlanet
        iDefaultThemeKey, // DefaultUISeparator
        NOVICE,         // DefaultPrivilegeLevel
       (float) 100.0,  // AdeptScore
        20,             // MaxNumSystemMessages
        20,             // MaxNumGameMessages
        tTime,          // LastShutDownTimeUnused
        "Needle",       // DefaultAttackName
        "Probe",        // Science
        "Seed",         // Colony
        "Portal",       // Stargate
        "Skai",         // Cloaker
        "Orbital",      // Satellite
        "Plow",         // Terraformer
        "Occupation",   // Troopship
        "Armageddon",   // Doomsday
        "Cambodia",     // Minefield
        "Broom",        // Minesweeper
        "Warp",         // Engineer
        10,             // Max number of personal gameclasses
        iDefaultThemeKey, // DefaultUIHorz
        iDefaultThemeKey, // DefaultUIVert
        iDefaultThemeKey, // DefaultUIColor
        15 * 1024,      // MaxIconSize(bytes)
        50,             // SystemMaxNumEmpires
        50,             // SystemMaxNumPlanets
        30,             // SystemMinSecs
        7*24*60*60,     // SystemMaxSecs
        15,             // PersonalMaxNumEmpires
        15,             // PersonalMaxNumPlanets
        60,             // PersonalMinSecs
        24*60*60,       // PersonalMaxSecs
        LOGINS_ENABLED | NEW_EMPIRES_ENABLED | NEW_GAMES_ENABLED | ACCESS_ENABLED |
        DEFAULT_BRIDIER_GAMES | DEFAULT_NAMES_LISTED | DEFAULT_ALLOW_SPECTATORS |
        DEFAULT_WARN_ON_DUPLICATE_IP_ADDRESS | DEFAULT_WARN_ON_DUPLICATE_SESSION_ID |
        DEFAULT_RESTRICT_IDLE_EMPIRES,    // Options
        tTime,          // LastBridierTimeBombScan
        24*60*60,       // BridierTimeBombScanFrequency
        ADEPT,          // PrivilegeForUnlimitedEmpires
       (char*) NULL,   // LoginsDisabledReason
       (char*) NULL,   // NewEmpiresDisabledReason
       (char*) NULL,   // NewGamesDisabledReason
       (char*) NULL,   // AccessDisabledReason
       (float) 30.0,   // ApprenticeScore
        iDefaultThemeKey, // DefaultUIIndependentPlanet
        10,             // MaxNumUpdatesBeforeClose
        3,              // DefaultNumUpdatesBeforeClose
       (int64) Algorithm::GetRandomInteger(0x7fffffff) + 1000,    // SessionId
        2000,           // MaxResourcesPerPlanet
        1000,           // MaxResourcesPerPlanetPersonal
       (float) 200.0,  // MaxInitialTechLevel
       (float) 100.0,  // MaxInitialTechLevelPersonal
       (float) 100.0,  // MaxTechDev
       (float) 50.0,   // MaxTechDevPersonal
        10,             // PercentEconIncreaseOnFirstTrade,
        90,             // PercentOfFirstEconIncreaseOnNextTrade,
        12 * 60 * 60,   // SecondsForLongtermStatus,
        8,              // NumUpdatesDownBeforeGameIsKilled
        25,             // NumNukesListedInNukeHistories
        2,              // NukesForAnnihilation
        5,              // UpdatesInQuarantineAfterAnnihilation
        75,             // PercentTechIncreaseForLatecomers
        60,             // PercentDamageUsedToDestroy
        12 * 60 * 60,   // AfterWeekendDelay
        35,             // ChanceNewLinkForms
       (float) 6.0,     // ResourceAllocationRandomizationFactor
        25,             // MapDeviation
        75,             // ChanceNewPlanetLinkedToLastCreatedPlanetLargeMap
        35,             // ChanceNewPlanetLinkedToLastCreatedPlanetSmallMap
        10,             // LargeMapThreshold
        COLONY_USE_MULTIPLIED_BUILD_COST | CLOAKER_CLOAK_ON_BUILD | JUMPGATE_LIMIT_RANGE |
        MORPHER_CLOAK_ON_CLOAKER_MORPH,
                        // ShipBehavior
        1,              // ColonySimpleBuildFactor
       (float) 1.0,    // ColonyMultipliedBuildFactor
       (float) 3.0,    // ColonyMultipliedDepositFactor
       (float) 2.0,    // ColonyExponentialDepositFactor
       (float) 0.5,    // EngineerLinkCost
       (float) 0.25,   // StargateGateCost
       (float) 10.0,   // TerraformerPlowFactor
       (float) 20.0,   // TroopshipInvasionFactor
       (float) 5.0,    // TroopshipFailureFactor
       (float) 0.5,    // TroopshipSuccessFactor
       (float) 3.0,    // DoomsdayAnnihilationFactor
       (float) 0.50,   // CarrierLoss
       (float) 5.0,    // BuilderMinBR
       (float) 0.50,   // MorpherLoss
       (float) 0.50,   // JumpgateGateLoss
       (float) 3.0,    // JumpgateRangeFactor
       (float) 0.50,   // StargateRangeFactor
        "Irongate",     // DefaultCarrierName,
        "Genesis",      // DefaultBuilderName,
        "Chameleon",    // DefaultMorpherName,
        "Hellmouth",    // DefaultJumpgateName,
        3.0,            // BuilderMultiplier,
        30,             // NumEmpiresInSystemNukeList,
        30,             // NumGamesInLatestGameList,
        5,              // MaxNumPersonalTournaments
        10,             // MaxNumGameClassesPerPersonalTournament
        140,            // SystemMessagesAlienKey
       (const char*) NULL, //AdminEmail,
       (float) 4.0,    // BuilderBRDampener
    };

    iErrCode = t_pCache->InsertRow(SYSTEM_DATA, SystemData::Template, pvSystemData, NULL);
    RETURN_ON_ERROR(iErrCode);

    // Insert default availability row
    const Variant pvAvailability[SystemAvailability::NumColumns] = 
    {
        tTime
    };

    iErrCode = t_pCache->InsertRow(SYSTEM_AVAILABILITY, SystemAvailability::Template, pvAvailability, NULL);
    RETURN_ON_ERROR(iErrCode);

    // Set up default administrator empire(root)
    iErrCode = CreateEmpire(ROOT_NAME, ROOT_DEFAULT_PASSWORD, ADMINISTRATOR, NO_KEY, true, &iKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = SetEmpireOption2(iKey, EMPIRE_ACCEPTED_TOS, true);
    RETURN_ON_ERROR(iErrCode);

    // Set up default guest empire(Guest)
    iErrCode = CreateEmpire(GUEST_NAME, GUEST_DEFAULT_PASSWORD, GUEST, NO_KEY, true, &iKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = SetEmpireOption(iKey, CAN_BROADCAST, false);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = SetEmpireOption2(iKey, EMPIRE_ACCEPTED_TOS, true);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::SetupDefaultThemes(unsigned int* piDefaultThemeKey)
{
    ///////////////////
    // Insert themes //
    ///////////////////

    int iErrCode;
    unsigned int iKey;

    Variant pvColVal[SystemThemes::NumColumns];

    // Mensan's First Theme
    pvColVal[SystemThemes::iName] = "Mensan's First Theme";
    pvColVal[SystemThemes::iAuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::iDescription] = "The fine new trademark Almonaster user interface";
    pvColVal[SystemThemes::iFileName] = "mensan1.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS;
    pvColVal[SystemThemes::iTableColor] = "882266";
    pvColVal[SystemThemes::iTextColor] = "FAFA00";
    pvColVal[SystemThemes::iGoodColor] = "00FF00";
    pvColVal[SystemThemes::iBadColor] = "EEEEEE";
    pvColVal[SystemThemes::iPrivateMessageColor] = "D3C2EA";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "F7EF80";

    iErrCode = CreateTheme(pvColVal, piDefaultThemeKey);
    RETURN_ON_ERROR(iErrCode);

    // Classic theme
    pvColVal[SystemThemes::iName] = "Classic Theme";
    pvColVal[SystemThemes::iAuthorName] = "Various";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "";
    pvColVal[SystemThemes::iDescription] = "The classic SC 2.8 user interface";
    pvColVal[SystemThemes::iFileName] = "classic.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~THEME_DEAD_PLANET & ~THEME_BUTTONS;
    pvColVal[SystemThemes::iTableColor] = "151515";
    pvColVal[SystemThemes::iTextColor] = "F0F011";
    pvColVal[SystemThemes::iGoodColor] = "00FF00";
    pvColVal[SystemThemes::iBadColor] = "EEEEEE";
    pvColVal[SystemThemes::iPrivateMessageColor] = "25D3AC";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "F0F011";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // MkII theme
    pvColVal[SystemThemes::iName] = "MkII Theme";
    pvColVal[SystemThemes::iAuthorName] = "MkII Team, Aleks Sidorenko";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "";
    pvColVal[SystemThemes::iDescription] = "The new MkII user interface";
    pvColVal[SystemThemes::iFileName] = "mkii.zip";
    pvColVal[SystemThemes::iOptions] = THEME_BACKGROUND | THEME_LIVE_PLANET | THEME_DEAD_PLANET;
    pvColVal[SystemThemes::iTableColor] = "222266";
    pvColVal[SystemThemes::iTextColor] = "F0F0F0";
    pvColVal[SystemThemes::iGoodColor] = "7FFFD4";
    pvColVal[SystemThemes::iBadColor] = "FF2E2E";
    pvColVal[SystemThemes::iPrivateMessageColor] = "FFFF00";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "EEEEEE";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Mensan's first beta theme
    pvColVal[SystemThemes::iName] = "Mensan's First Beta Theme";
    pvColVal[SystemThemes::iAuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::iDescription] = "Some beta-version graphics from the master";
    pvColVal[SystemThemes::iFileName] = "mensanb1.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~THEME_BUTTONS & ~THEME_HORZ & ~THEME_VERT;
    pvColVal[SystemThemes::iTableColor] = "152560";
    pvColVal[SystemThemes::iTextColor] = "F0F0F0";
    pvColVal[SystemThemes::iGoodColor] = "F0F011";
    pvColVal[SystemThemes::iBadColor] = "FBA02B";
    pvColVal[SystemThemes::iPrivateMessageColor] = "COCOFF";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "EEEEEE";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Mensan's second beta theme
    pvColVal[SystemThemes::iName] = "Mensan's Second Beta Theme";
    pvColVal[SystemThemes::iAuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::iDescription] = "More beta-version graphics from the master";
    pvColVal[SystemThemes::iFileName] = "mensanb2.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~(THEME_BUTTONS | THEME_HORZ | THEME_VERT);
    pvColVal[SystemThemes::iTableColor] = "02546C";
    pvColVal[SystemThemes::iTextColor] = "F0F0F0";
    pvColVal[SystemThemes::iGoodColor] = "FFFF00";
    pvColVal[SystemThemes::iBadColor] = "FBA02B";
    pvColVal[SystemThemes::iPrivateMessageColor] = "C0C0FF";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "EEEEEE";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // DPR's animated theme
    pvColVal[SystemThemes::iName] = "DPR's Animated Theme";
    pvColVal[SystemThemes::iAuthorName] = "Simon Gillbee";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "simon@gillbee.com";
    pvColVal[SystemThemes::iDescription] = "Animated graphics";
    pvColVal[SystemThemes::iFileName] = "anim.zip";
    pvColVal[SystemThemes::iOptions] = THEME_LIVE_PLANET;
    pvColVal[SystemThemes::iTableColor] = "052205";
    pvColVal[SystemThemes::iTextColor] = "FFFF00";
    pvColVal[SystemThemes::iGoodColor] = "00FF00";
    pvColVal[SystemThemes::iBadColor] = "EEEEEE";
    pvColVal[SystemThemes::iPrivateMessageColor] = "80FFFF";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "FFFF00";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Mensan's Techno Theme
    pvColVal[SystemThemes::iName] = "Mensan's Techno Theme";
    pvColVal[SystemThemes::iAuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::iDescription] = "A technological theme";
    pvColVal[SystemThemes::iFileName] = "techno.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~THEME_BUTTONS;
    pvColVal[SystemThemes::iTableColor] = "761244";
    pvColVal[SystemThemes::iTextColor] = "FFFF00";
    pvColVal[SystemThemes::iGoodColor] = "45F3CC";
    pvColVal[SystemThemes::iBadColor] = "F7EFCE";
    pvColVal[SystemThemes::iPrivateMessageColor] = "C9C9C9";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "EEEE00";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Mensan's Animated Theme
    pvColVal[SystemThemes::iName] = "Mensan's Animated Theme";
    pvColVal[SystemThemes::iAuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::iDescription] = "A theme with animated graphics";
    pvColVal[SystemThemes::iFileName] = "animated.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~THEME_BUTTONS;
    pvColVal[SystemThemes::iTableColor] = "502222";
    pvColVal[SystemThemes::iTextColor] = "EEEE00";
    pvColVal[SystemThemes::iGoodColor] = "FCCAA2";
    pvColVal[SystemThemes::iBadColor] = "FF5555";
    pvColVal[SystemThemes::iPrivateMessageColor] = "FFA936";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "EEEE00";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Mensan's Blues Theme
    pvColVal[SystemThemes::iName] = "Mensan's Blues Theme";
    pvColVal[SystemThemes::iAuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::iDescription] = "A theme with spooky blue graphics";
    pvColVal[SystemThemes::iFileName] = "blues.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS;
    pvColVal[SystemThemes::iTableColor] = "2020B0";
    pvColVal[SystemThemes::iTextColor] = "9CC6FF";
    pvColVal[SystemThemes::iGoodColor] = "00FF00";
    pvColVal[SystemThemes::iBadColor] = "FFFFFF";
    pvColVal[SystemThemes::iPrivateMessageColor] = "25D3AC";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "9CC6FF";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Mensan's Dark Mood Theme
    pvColVal[SystemThemes::iName] = "Mensan's Dark Mood Theme";
    pvColVal[SystemThemes::iAuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::iDescription] = "A theme with dark, somber graphics";
    pvColVal[SystemThemes::iFileName] = "darkmood.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS;
    pvColVal[SystemThemes::iTableColor] = "443C3C";
    pvColVal[SystemThemes::iTextColor] = "C5C5C5";
    pvColVal[SystemThemes::iGoodColor] = "00D000";
    pvColVal[SystemThemes::iBadColor] = "D0D000";
    pvColVal[SystemThemes::iPrivateMessageColor] = "FCCAA2";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "D5D5D5";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Chamber Theme
    pvColVal[SystemThemes::iName] = "Chamber Theme";
    pvColVal[SystemThemes::iAuthorName] = "Dynamic Design";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "dd@chamber.ee";
    pvColVal[SystemThemes::iDescription] = "Images from the Chamber Conflict Server";
    pvColVal[SystemThemes::iFileName] = "chamber.zip";
    pvColVal[SystemThemes::iOptions] = THEME_BACKGROUND | THEME_SEPARATOR;
    pvColVal[SystemThemes::iTableColor] = "353535";
    pvColVal[SystemThemes::iTextColor] = "C0C0C0";
    pvColVal[SystemThemes::iGoodColor] = "00FF00";
    pvColVal[SystemThemes::iBadColor] = "FFFF00";
    pvColVal[SystemThemes::iPrivateMessageColor] = "FFFF00";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "D0D0D0";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // NASA Theme
    pvColVal[SystemThemes::iName] = "NASA Theme";
    pvColVal[SystemThemes::iAuthorName] = "Max Attar Feingold";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "maf6@cornell.edu";
    pvColVal[SystemThemes::iDescription] = "Images borrowed from various NASA servers";
    pvColVal[SystemThemes::iFileName] = "nasa.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~THEME_BUTTONS;
    pvColVal[SystemThemes::iTableColor] = "570505";
    pvColVal[SystemThemes::iTextColor] = "F0F000";
    pvColVal[SystemThemes::iGoodColor] = "25FF25";
    pvColVal[SystemThemes::iBadColor] = "F0F0F0";
    pvColVal[SystemThemes::iPrivateMessageColor] = "25CF25";
    pvColVal[SystemThemes::iBroadcastMessageColor] = "F09525";

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Alien Glow Theme
    pvColVal[SystemThemes::iName] = "Alien Glow Theme";
    pvColVal[SystemThemes::iAuthorName] = "Denis Moreaux";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "vapula@linuxbe.org";
    pvColVal[SystemThemes::iDescription] = "Look is based on \"Alien Glow\" Gimp web theme. Used on Endor server";
    pvColVal[SystemThemes::iFileName] = "alienglow.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~THEME_BUTTONS;
    pvColVal[SystemThemes::iTableColor] = "001000";              // Dark green, almost black
    pvColVal[SystemThemes::iTextColor] = "00C000";               // Green
    pvColVal[SystemThemes::iGoodColor] = "00FF00";               // Bright green
    pvColVal[SystemThemes::iBadColor] = "4040ff";                // Ghostly blue
    pvColVal[SystemThemes::iPrivateMessageColor] = "FFFF00";     // Bright yellow
    pvColVal[SystemThemes::iBroadcastMessageColor] = "FFFFCC";   // Dull yellow

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Iceberg Theme
    pvColVal[SystemThemes::iName] = "Iceberg Theme";
    pvColVal[SystemThemes::iAuthorName] = "Aleksandr Sidorenko";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "aleksandr@videotron.ca";
    pvColVal[SystemThemes::iDescription] = "Look inspired by Alexia's Iceberg server";
    pvColVal[SystemThemes::iFileName] = "iceberg.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~(THEME_BUTTONS | THEME_HORZ | THEME_VERT);
    pvColVal[SystemThemes::iTableColor] = "101020";              // Dark blue, almost black
    pvColVal[SystemThemes::iTextColor] = "90A0CC";               // Light blue
    pvColVal[SystemThemes::iGoodColor] = "00DD00";               // Sharp green
    pvColVal[SystemThemes::iBadColor] = "E80700";                // Sharp red
    pvColVal[SystemThemes::iPrivateMessageColor] = "9090FF";     // Brigher light blue
    pvColVal[SystemThemes::iBroadcastMessageColor] = "90A0CC";   // Light blue

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    // Iceberg Theme II
    pvColVal[SystemThemes::iName] = "Iceberg Theme II";
    pvColVal[SystemThemes::iAuthorName] = "Aleksandr Sidorenko";
    pvColVal[SystemThemes::iVersion] = "1.0";
    pvColVal[SystemThemes::iAuthorEmail] = "aleksandr@videotron.ca";
    pvColVal[SystemThemes::iDescription] = "Look inspired by Alexia's Iceberg server";
    pvColVal[SystemThemes::iFileName] = "iceberg2.zip";
    pvColVal[SystemThemes::iOptions] = ALL_THEME_OPTIONS & ~(THEME_BUTTONS | THEME_HORZ | THEME_VERT);
    pvColVal[SystemThemes::iTableColor] = "101020";              // Dark blue, almost black
    pvColVal[SystemThemes::iTextColor] = "90A0D0";               // Light blue
    pvColVal[SystemThemes::iGoodColor] = "4070F0";               // Dark blue
    pvColVal[SystemThemes::iBadColor] = "E80700";                // Sharp red
    pvColVal[SystemThemes::iPrivateMessageColor] = "B0B0B0";     // Gray
    pvColVal[SystemThemes::iBroadcastMessageColor] = "E0F0E0";   // Light green

    iErrCode = CreateTheme(pvColVal, &iKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

// Create default superclasses and gameclasses
int GameEngine::SetupDefaultSystemGameClasses()
{
    // Add SuperClasses
    int i, iErrCode, iBeginnerKey, iGrudgeKey, iLongTermKey, iBlitzKey, iGameClass;
    
    iErrCode = CreateSuperClass("Beginner Games", &iBeginnerKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateSuperClass("Grudge Matches", &iGrudgeKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateSuperClass("Blitzes", &iBlitzKey);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = CreateSuperClass("Longterm Games", &iLongTermKey);
    RETURN_ON_ERROR(iErrCode);

    Variant pvSubmitArray [SystemGameClassData::NumColumns];

    // Beginner Tekno Blood
    pvSubmitArray[SystemGameClassData::iName] = "Beginner Tekno Blood";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 10;
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 3.5;
    pvSubmitArray[SystemGameClassData::iOpenGameNum] = 1;                            
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 210;
    pvSubmitArray[SystemGameClassData::iOptions] = WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | ONLY_SURRENDER_WITH_TWO_EMPIRES;
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;             
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBeginnerKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;   
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 4;                          
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;
    
    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);


    // Apprentice Blitz
    pvSubmitArray[SystemGameClassData::iName] = "Apprentice Blitz";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 16;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 3;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 1.5;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 240;
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | EXPOSED_DIPLOMACY |
        VISIBLE_BUILDS | 
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                    
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 4;                          
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | TRUCE | TRADE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;                     
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBeginnerKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 3;                          
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;                           
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);


    // Assassin Grudge
    pvSubmitArray[SystemGameClassData::iName] = "Assassin Grudge";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 2;                          
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 10;                         
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 1.5;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::iOptions] = 
        VISIBLE_DIPLOMACY | EXPOSED_DIPLOMACY | VISIBLE_BUILDS | ALLOW_DRAW;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 25;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 25;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 25;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 35;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 35;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 35;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 90;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 90;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 90;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 110;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 110;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 110;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | SURRENDER;       
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM; 
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iGrudgeKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 8;                                          
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = TECH_SCIENCE | TECH_COLONY | TECH_SATELLITE;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);


    // Darkstar Battle
    pvSubmitArray[SystemGameClassData::iName] = "Darkstar Battle";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 10;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 5;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 4.0;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 150;                          
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.5;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 3;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 35;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 35;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 35;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 45;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 45;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 45;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 110;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 110;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 110;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 130;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 130;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 130;                    
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | TRUCE | TRADE | ALLIANCE | SURRENDER;            
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = 2;
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 5;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_MINESWEEPER | TECH_MINEFIELD;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;


    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);

    
    // Tiberia Series
    pvSubmitArray[SystemGameClassData::iName] = "Tiberia Series";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 10;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 6;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 2.0;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 180;
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | VISIBLE_BUILDS |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 2.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 3;                          
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 35;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 35;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 35;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | TRADE | SURRENDER;                   
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;                           
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_TROOPSHIP;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);
    
    
    // Galaxy of Andromeda
    pvSubmitArray[SystemGameClassData::iName] = "Galaxy of Andromeda";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 20;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 12;                         
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 1.5;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::iOptions] = 
        VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | VISIBLE_BUILDS |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 5;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 35;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 35;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 35;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 110;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 110;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 110;        
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | TRUCE | TRADE | ALLIANCE | SURRENDER;    
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iLongTermKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = 2;
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 10;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);
    
    
    // Natural Selection Universe
    pvSubmitArray[SystemGameClassData::iName] = "Natural Selection Universe";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 16;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 10;                         
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 2.0;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::iOptions] = 
        VISIBLE_DIPLOMACY | 
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 2.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 4;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 23;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 23;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 23;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 90;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 90;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 90;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 105;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 105;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 105;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iLongTermKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 10;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_DOOMSDAY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);

    // Texan KnifeFight
    pvSubmitArray[SystemGameClassData::iName] = "Texan KnifeFight";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 6;                          
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 3;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 2.0;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 180;
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 3;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;                           
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_TERRAFORMER;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);

    
    // 21st Century Blood   
    pvSubmitArray[SystemGameClassData::iName] = "21st Century Blood";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 16;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 9;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 4.0;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::iOptions] = 
        VISIBLE_DIPLOMACY | VISIBLE_BUILDS | ONLY_SURRENDER_WITH_TWO_EMPIRES | ALLOW_DRAW;

    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 2.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 8;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 32;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 32;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 32;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 36;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 36;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 36;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 105;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 105;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 105;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 115;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 115;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 115;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iLongTermKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 7;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_CLOAKER;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);


    // Nanotech World
    pvSubmitArray[SystemGameClassData::iName] = "Nanotech World";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 10;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 10;                         
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 4.25;              
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::iOptions] = 
        VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | VISIBLE_BUILDS |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.5;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 2;                          
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | TRUCE | TRADE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iLongTermKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = 3;
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 10;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_SCIENCE | TECH_COLONY | TECH_ENGINEER | TECH_CLOAKER | TECH_DOOMSDAY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);
    

    // Land of Bounty
    pvSubmitArray[SystemGameClassData::iName] = "Land of Bounty";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 10;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 6;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 1.5;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 180;                          
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | EXPOSED_DIPLOMACY |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 60;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 60;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 60;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 70;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 70;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 70;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 166;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 166;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 166;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 200;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 200;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 200;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | TRUCE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_TERRAFORMER;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);
    

    // Perfect Information Battle
    pvSubmitArray[SystemGameClassData::iName] = "Perfect Information Battle";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 2;                              
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 7;                              
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 1.5;                       
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;                 
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | EXPOSED_MAP | EXPOSED_DIPLOMACY | VISIBLE_BUILDS | ALLOW_DRAW;
                
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 2;                              
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iGrudgeKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 7;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = TECH_COLONY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = TECH_COLONY;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);


    // Caveman Deathmatch
    pvSubmitArray[SystemGameClassData::iName] = "Caveman Deathmatch";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 2;                          
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 5;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 0.8;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | ALLOW_DRAW;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 33;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 33;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 33;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 40;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 40;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 40;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 80;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 80;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 80;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 90;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 90;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 90;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iGrudgeKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 5;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);
        
    
    // Low Orbit SuperBlitz
    pvSubmitArray[SystemGameClassData::iName] = "Low Orbit SuperBlitz";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 6;                          
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 4;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 2.0;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 90;                           
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | VISIBLE_BUILDS |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 4;                          
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | TRADE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);


    // Kindergarten Universe
    pvSubmitArray[SystemGameClassData::iName] = "Kindergarten Universe";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 12;                         
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 8;                          
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 1.5;                   
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::iOptions] = 
        VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | EXPOSED_DIPLOMACY |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.5;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 8;                          
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 33;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 33;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 33;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 33;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 33;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 33;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | TRUCE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBeginnerKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = 2;
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 8;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_TROOPSHIP;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);


    // Guns'n Troopships
    pvSubmitArray[SystemGameClassData::iName] = "Guns'n Troopships";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 2;                          
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 12;                         
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 1.25;              
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::iOptions] = 
        VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | ONLY_SURRENDER_WITH_TWO_EMPIRES;
                    
    pvSubmitArray[SystemGameClassData::iInitialTechLevel] =(float) 1.0;                 
    pvSubmitArray[SystemGameClassData::iMinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::iMinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::iMinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::iMaxAvgAg] = 40;
    pvSubmitArray[SystemGameClassData::iMaxAvgMin] = 40;
    pvSubmitArray[SystemGameClassData::iMaxAvgFuel] = 40;
    pvSubmitArray[SystemGameClassData::iMinAgHW] = 90;
    pvSubmitArray[SystemGameClassData::iMinMinHW] = 90;
    pvSubmitArray[SystemGameClassData::iMinFuelHW] = 90;
    pvSubmitArray[SystemGameClassData::iMaxAgHW] = 110;
    pvSubmitArray[SystemGameClassData::iMaxMinHW] = 110;
    pvSubmitArray[SystemGameClassData::iMaxFuelHW] = 110;
    pvSubmitArray[SystemGameClassData::iDiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::iMapsShared] = NO_DIPLOMACY;              
    pvSubmitArray[SystemGameClassData::iOwner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::iSuperClassKey] = iBeginnerKey;
    pvSubmitArray[SystemGameClassData::iMaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::iMinNumPlanets] = 9;
    pvSubmitArray[SystemGameClassData::iBuilderPopLevel] = 50;                           
    pvSubmitArray[SystemGameClassData::iInitialTechDevs] = TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::iDevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::iDescription] = "";
    pvSubmitArray[SystemGameClassData::iMaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::iMaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::iNumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::iNumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::iRuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::iOwnerName] = "";
    pvSubmitArray[SystemGameClassData::iTournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::iNumInitialTechDevs] = 1;

    iErrCode = CreateGameClass(SYSTEM, pvSubmitArray, &iGameClass);
    RETURN_ON_ERROR(iErrCode);


    //////////////////////
    // SystemAlienIcons //
    //////////////////////
    
    // 1 to 42
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ken Eppstein";
    for (i = 1; i <= 42; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 43 to 82
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ronald Kinion";
    for (i = 43; i <= 82; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 83
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 83;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Unknown";
    iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    RETURN_ON_ERROR(iErrCode);
    
    // 84 to 89
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ronald Kinion";
    for (i = 84; i <= 89; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 90
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 90;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Jens Klavsen";
    iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    RETURN_ON_ERROR(iErrCode);
    
    // 91
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 91;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Chris John";
    iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    RETURN_ON_ERROR(iErrCode);
    
    // 92 to 101
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Jens Klavsen";
    for (i = 92; i <= 101; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 102 to 103
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ronald Kinion";
    for (i = 102; i <= 103; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 104 to 105
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Chris John";
    for (i = 104; i <= 105; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 106 to 125
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ronald Kinion";
    for (i = 106; i <= 125; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 126 to 127
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Chris John";
    for (i = 126; i <= 127; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 128
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 128;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Haavard Fledsberg";
    iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    RETURN_ON_ERROR(iErrCode);
    
    // 129 to 135
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Chris John";
    for (i = 129; i <= 135; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 136 to 145
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Jens Klavsen";
    for (i = 136; i <= 145; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }
    
    // 146
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 146;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Michel Lemieux";
    iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    RETURN_ON_ERROR(iErrCode);
    
    // 147 to 152
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Unknown";
    for (i = 147; i <= 152; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    // 153
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 153;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Kia";
    iErrCode = t_pCache->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::VerifyTournaments() {

    int iErrCode;
    unsigned int iKey = NO_KEY, iNumGames, iOwner;

    bool bExists;

    while (true)
    {
        iErrCode = t_pCache->GetNextKey(SYSTEM_TOURNAMENTS, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND)
        {
            iErrCode = OK;
            break;
        }
        
        RETURN_ON_ERROR(iErrCode);
        
        iErrCode = GetTournamentOwner(iKey, &iOwner);
        RETURN_ON_ERROR(iErrCode);

        // Check for tournament that needs to be deleted
        if (iOwner == DELETED_EMPIRE_KEY)
        {
            iErrCode = GetTournamentGames(iKey, NULL, &iNumGames);
            RETURN_ON_ERROR(iErrCode);

            if (iNumGames == 0)
            {
                iErrCode = DeleteTournament(DELETED_EMPIRE_KEY, iKey, false);
                RETURN_ON_ERROR(iErrCode);
            }
        }

        // Check for tournament without empire
        else if (iOwner != SYSTEM) {

            iErrCode = DoesEmpireExist(iOwner, &bExists, NULL);
            RETURN_ON_ERROR(iErrCode);

            if (!bExists) {

                iErrCode = DeleteTournament(iOwner, iKey, true);
                if (iErrCode == ERROR_TOURNAMENT_HAS_GAMES)
                {
                    iErrCode = OK;
                }
                RETURN_ON_ERROR(iErrCode);
            }
        }
    }
    
    return iErrCode;
}
