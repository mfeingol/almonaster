//
// GameEngine.dll:  a component of Almonaster
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

int GameEngine::Setup() {

    bool bNewDatabase, bGoodDatabase;
    const char* pszBadTable;

    /////////////////////////
    // Check system tables //
    /////////////////////////

    global.GetReport()->WriteReport("GameEngine setup attempting to reuse an existing database");
    VerifySystemTables(&bNewDatabase, &bGoodDatabase, &pszBadTable);

    if (bNewDatabase)
    {
        // Create a new database and we're done
        return InitializeNewDatabase();
    }

    if (!bGoodDatabase) {

        // Bad database - report error 
        char* pszMessage = (char*)StackAlloc(strlen(pszBadTable) + 256);
        sprintf(pszMessage, "GameEngine setup found errors in the %s table", pszBadTable);

        global.GetReport()->WriteReport(pszMessage);
        global.GetReport()->WriteReport("GameEngine setup could not successfully reuse an existing database");
        
        return ERROR_FAILURE;
    }

    // Reload the database
    return ReloadDatabase();
}


int GameEngine::ReloadDatabase() {

    int iErrCode;

    //
    // System
    //

    iErrCode = VerifySystem();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup failed to verify system data");
        return iErrCode;
    }
    global.GetReport()->WriteReport("GameEngine setup successfully verified system data");

    //
    // Gameclasses
    //

    iErrCode = VerifyGameClasses();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup failed to verify gameclasses");
        return iErrCode;
    }
    global.GetReport()->WriteReport("GameEngine setup successfully verified gameclasses");

    //
    // Games
    //

    iErrCode = VerifyActiveGames();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup failed to verify active games");
        return iErrCode;
    }
    global.GetReport()->WriteReport("GameEngine setup successfully verified active games");    

    //
    // Marked gameclasses
    //

    iErrCode = VerifyMarkedGameClasses();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup failed to verify marked gameclasses");
        return iErrCode;
    }
    global.GetReport()->WriteReport("GameEngine setup successfully verified marked gameclasses");

    //
    // Tournaments
    //

    iErrCode = VerifyTournaments();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup failed to verify tournaments");
        return iErrCode;
    }
    global.GetReport()->WriteReport("GameEngine setup successfully verified tournaments");

    //
    // Top lists
    //

    iErrCode = VerifyTopLists();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup failed to verify top lists");
        return iErrCode;
    }
    global.GetReport()->WriteReport("GameEngine setup successfully verified top lists");

    //////////
    // Done //
    //////////

    global.GetReport()->WriteReport("GameEngine setup successfully reused the existing database");

    return OK;
}

void GameEngine::VerifySystemTables(bool* pbNewDatabase, bool* pbGoodDatabase, const char** ppszBadTable) {

    const char* pszBadTable = NULL;
    bool bNewDatabase = false, bGoodDatabase = true;

    unsigned int iNumRows;

    // SystemData
    Assert(countof(SystemData::Types) == SystemData::NumColumns);
    Assert(countof(SystemData::Sizes) == SystemData::NumColumns);
    Assert(countof(SystemData::ColumnNames) == SystemData::NumColumns);

    if (!t_pConn->DoesTableExist(SYSTEM_DATA)) {
        bNewDatabase = true;
        goto Cleanup;
    }
    
    pszBadTable = SYSTEM_DATA;
    if (t_pConn->GetNumRows(SYSTEM_DATA, &iNumRows) != OK || iNumRows != 1) {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemEmpireData
    Assert(countof(SystemEmpireData::Types) == SystemEmpireData::NumColumns);
    Assert(countof(SystemEmpireData::Sizes) == SystemEmpireData::NumColumns);
    Assert(countof(SystemEmpireData::ColumnNames) == SystemEmpireData::NumColumns);

    pszBadTable = SYSTEM_EMPIRE_DATA;
    if (!t_pConn->DoesTableExist(SYSTEM_EMPIRE_DATA)) {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    if (!t_pConn->DoesTableExist(SYSTEM_EMPIRE_DATA) ||
        t_pConn->GetNumRows(SYSTEM_EMPIRE_DATA, &iNumRows) != OK || iNumRows < 1) {
        bGoodDatabase = false;
        goto Cleanup;
    }

    // SystemGameClassData
    Assert(countof(SystemGameClassData::Types) == SystemGameClassData::NumColumns);
    Assert(countof(SystemGameClassData::Sizes) == SystemGameClassData::NumColumns);
    Assert(countof(SystemGameClassData::ColumnNames) == SystemGameClassData::NumColumns);

    pszBadTable = SYSTEM_GAMECLASS_DATA;
    if (!t_pConn->DoesTableExist(pszBadTable) ||
        t_pConn->GetNumRows(pszBadTable, &iNumRows) != OK || iNumRows < 1) {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemAlienIcons
    Assert(countof(SystemAlienIcons::Types) == SystemAlienIcons::NumColumns);
    Assert(countof(SystemAlienIcons::Sizes) == SystemAlienIcons::NumColumns);
    Assert(countof(SystemAlienIcons::ColumnNames) == SystemAlienIcons::NumColumns);

    pszBadTable = SYSTEM_ALIEN_ICONS;
    if (!t_pConn->DoesTableExist(pszBadTable) ||
        t_pConn->GetNumRows(pszBadTable, &iNumRows) != OK || iNumRows < 1) {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemSystemGameClassData
    Assert(countof(SystemSystemGameClassData::Types) == SystemSystemGameClassData::NumColumns);
    Assert(countof(SystemSystemGameClassData::Sizes) == SystemSystemGameClassData::NumColumns);
    Assert(countof(SystemSystemGameClassData::ColumnNames) == SystemSystemGameClassData::NumColumns);

    pszBadTable = SYSTEM_SYSTEM_GAMECLASS_DATA;
    if (!t_pConn->DoesTableExist(pszBadTable) ||
        t_pConn->GetNumRows(pszBadTable, &iNumRows) != OK || iNumRows < 1) {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemSuperClassData
    Assert(countof(SystemSuperClassData::Types) == SystemSuperClassData::NumColumns);
    Assert(countof(SystemSuperClassData::Sizes) == SystemSuperClassData::NumColumns);
    Assert(countof(SystemSuperClassData::ColumnNames) == SystemSuperClassData::NumColumns);

    pszBadTable = SYSTEM_SUPERCLASS_DATA;
    if (!t_pConn->DoesTableExist(pszBadTable) ||
        t_pConn->GetNumRows(pszBadTable, &iNumRows) != OK || iNumRows < 1) {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemThemes
    Assert(countof(SystemThemes::Types) == SystemThemes::NumColumns);
    Assert(countof(SystemThemes::Sizes) == SystemThemes::NumColumns);
    Assert(countof(SystemThemes::ColumnNames) == SystemThemes::NumColumns);

    pszBadTable = SYSTEM_THEMES;
    if (!t_pConn->DoesTableExist(pszBadTable) ||
        t_pConn->GetNumRows(pszBadTable, &iNumRows) != OK || iNumRows < 1)
    {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemActiveGames
    Assert(countof(SystemActiveGames::Types) == SystemActiveGames::NumColumns);
    Assert(countof(SystemActiveGames::Sizes) == SystemActiveGames::NumColumns);
    Assert(countof(SystemActiveGames::ColumnNames) == SystemActiveGames::NumColumns);

    pszBadTable = SYSTEM_ACTIVE_GAMES;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemAlmonasterScoreTopList
    Assert(countof(SystemAlmonasterScoreTopList::Types) == SystemAlmonasterScoreTopList::NumColumns);
    Assert(countof(SystemAlmonasterScoreTopList::Sizes) == SystemAlmonasterScoreTopList::NumColumns);
    Assert(countof(SystemAlmonasterScoreTopList::ColumnNames) == SystemAlmonasterScoreTopList::NumColumns);

    pszBadTable = SYSTEM_ALMONASTER_SCORE_TOPLIST;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemClassicScoreTopList
    Assert(countof(SystemClassicScoreTopList::Types) == SystemClassicScoreTopList::NumColumns);
    Assert(countof(SystemClassicScoreTopList::Sizes) == SystemClassicScoreTopList::NumColumns);
    Assert(countof(SystemClassicScoreTopList::ColumnNames) == SystemClassicScoreTopList::NumColumns);

    pszBadTable = SYSTEM_CLASSIC_SCORE_TOPLIST;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }

    // SystemBridierScoreTopList
    Assert(countof(SystemBridierScoreTopList::Types) == SystemBridierScoreTopList::NumColumns);
    Assert(countof(SystemBridierScoreTopList::Sizes) == SystemBridierScoreTopList::NumColumns);
    Assert(countof(SystemBridierScoreTopList::ColumnNames) == SystemBridierScoreTopList::NumColumns);

    pszBadTable = SYSTEM_BRIDIER_SCORE_TOPLIST;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }

    // SystemBridierScoreEstablishedTopList
    Assert(countof(SystemBridierScoreEstablishedTopList::Types) == SystemBridierScoreEstablishedTopList::NumColumns);
    Assert(countof(SystemBridierScoreEstablishedTopList::Sizes) == SystemBridierScoreEstablishedTopList::NumColumns);
    Assert(countof(SystemBridierScoreEstablishedTopList::ColumnNames) == SystemBridierScoreEstablishedTopList::NumColumns);

    pszBadTable = SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }

    // SystemChatroomData
    Assert(countof(SystemChatroomData::Types) == SystemChatroomData::NumColumns);
    Assert(countof(SystemChatroomData::Sizes) == SystemChatroomData::NumColumns);
    Assert(countof(SystemChatroomData::ColumnNames) == SystemChatroomData::NumColumns);

    pszBadTable = SYSTEM_CHATROOM_DATA;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }

    // SystemTournaments
    Assert(countof(SystemTournaments::Types) == SystemTournaments::NumColumns);
    Assert(countof(SystemTournaments::Sizes) == SystemTournaments::NumColumns);
    Assert(countof(SystemTournaments::ColumnNames) == SystemTournaments::NumColumns);

    pszBadTable = SYSTEM_TOURNAMENTS;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }

    // SystemNukeList
    Assert(countof(SystemNukeList::Types) == SystemNukeList::NumColumns);
    Assert(countof(SystemNukeList::Sizes) == SystemNukeList::NumColumns);
    Assert(countof(SystemNukeList::ColumnNames) == SystemNukeList::NumColumns);

    pszBadTable = SystemNukeList::Template.Name;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }

    // SystemLatestGames
    Assert(countof(SystemLatestGames::Types) == SystemLatestGames::NumColumns);
    Assert(countof(SystemLatestGames::Sizes) == SystemLatestGames::NumColumns);
    Assert(countof(SystemLatestGames::ColumnNames) == SystemLatestGames::NumColumns);

    pszBadTable = SystemLatestGames::Template.Name;
    if (!t_pConn->DoesTableExist(pszBadTable))
    {
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    // SystemEmpireMessages
    Assert(countof(SystemEmpireMessages::Types) == SystemEmpireMessages::NumColumns);
    Assert(countof(SystemEmpireMessages::Sizes) == SystemEmpireMessages::NumColumns);
    Assert(countof(SystemEmpireMessages::ColumnNames) == SystemEmpireMessages::NumColumns);

    // SystemEmpireNukeList
    Assert(countof(SystemEmpireNukeList::Types) == SystemEmpireNukeList::NumColumns);
    Assert(countof(SystemEmpireNukeList::Sizes) == SystemEmpireNukeList::NumColumns);
    Assert(countof(SystemEmpireNukeList::ColumnNames) == SystemEmpireNukeList::NumColumns);

    // SystemEmpireActiveGames
    Assert(countof(SystemEmpireActiveGames::Types) == SystemEmpireActiveGames::NumColumns);
    Assert(countof(SystemEmpireActiveGames::Sizes) == SystemEmpireActiveGames::NumColumns);
    Assert(countof(SystemEmpireActiveGames::ColumnNames) == SystemEmpireActiveGames::NumColumns);

    // SystemTournamentTeams
    Assert(countof(SystemTournamentTeams::Types) == SystemTournamentTeams::NumColumns);
    Assert(countof(SystemTournamentTeams::Sizes) == SystemTournamentTeams::NumColumns);
    Assert(countof(SystemTournamentTeams::ColumnNames) == SystemTournamentTeams::NumColumns);

    // SystemTournamentEmpires
    Assert(countof(SystemTournamentEmpires::Types) == SystemTournamentEmpires::NumColumns);
    Assert(countof(SystemTournamentEmpires::Sizes) == SystemTournamentEmpires::NumColumns);
    Assert(countof(SystemTournamentEmpires::ColumnNames) == SystemTournamentEmpires::NumColumns);

    // SystemTournamentActiveGames
    Assert(countof(SystemTournamentActiveGames::Types) == SystemTournamentActiveGames::NumColumns);
    Assert(countof(SystemTournamentActiveGames::Sizes) == SystemTournamentActiveGames::NumColumns);
    Assert(countof(SystemTournamentActiveGames::ColumnNames) == SystemTournamentActiveGames::NumColumns);

    // SystemEmpireTournaments
    Assert(countof(SystemEmpireTournaments::Types) == SystemEmpireTournaments::NumColumns);
    Assert(countof(SystemEmpireTournaments::Sizes) == SystemEmpireTournaments::NumColumns);
    Assert(countof(SystemEmpireTournaments::ColumnNames) == SystemEmpireTournaments::NumColumns);

    // GameData
    Assert(countof(GameData::Types) == GameData::NumColumns);
    Assert(countof(GameData::Sizes) == GameData::NumColumns);
    Assert(countof(GameData::ColumnNames) == GameData::NumColumns);
    
    // GameEmpires
    Assert(countof(GameEmpires::Types) == GameEmpires::NumColumns);
    Assert(countof(GameEmpires::Sizes) == GameEmpires::NumColumns);
    Assert(countof(GameEmpires::ColumnNames) == GameEmpires::NumColumns);

    // GameDeadEmpires
    Assert(countof(GameDeadEmpires::Types) == GameDeadEmpires::NumColumns);
    Assert(countof(GameDeadEmpires::Sizes) == GameDeadEmpires::NumColumns);
    Assert(countof(GameDeadEmpires::ColumnNames) == GameDeadEmpires::NumColumns);

    // GameMap
    Assert(countof(GameMap::Types) == GameMap::NumColumns);
    Assert(countof(GameMap::Sizes) == GameMap::NumColumns);
    Assert(countof(GameMap::ColumnNames) == GameMap::NumColumns);

    // GameEmpireData
    Assert(countof(GameEmpireData::Types) == GameEmpireData::NumColumns);
    Assert(countof(GameEmpireData::Sizes) == GameEmpireData::NumColumns);
    Assert(countof(GameEmpireData::ColumnNames) == GameEmpireData::NumColumns);

    // GameEmpireMessages
    Assert(countof(GameEmpireMessages::Types) == GameEmpireMessages::NumColumns);
    Assert(countof(GameEmpireMessages::Sizes) == GameEmpireMessages::NumColumns);
    Assert(countof(GameEmpireMessages::ColumnNames) == GameEmpireMessages::NumColumns);

    // GameEmpireMap
    Assert(countof(GameEmpireMap::Types) == GameEmpireMap::NumColumns);
    Assert(countof(GameEmpireMap::Sizes) == GameEmpireMap::NumColumns);
    Assert(countof(GameEmpireMap::ColumnNames) == GameEmpireMap::NumColumns);
    
    // GameEmpireDiplomacy
    Assert(countof(GameEmpireDiplomacy::Types) == GameEmpireDiplomacy::NumColumns);
    Assert(countof(GameEmpireDiplomacy::Sizes) == GameEmpireDiplomacy::NumColumns);
    Assert(countof(GameEmpireDiplomacy::ColumnNames) == GameEmpireDiplomacy::NumColumns);

    // GameEmpireShips
    Assert(countof(GameEmpireShips::Types) == GameEmpireShips::NumColumns);
    Assert(countof(GameEmpireShips::Sizes) == GameEmpireShips::NumColumns);
    Assert(countof(GameEmpireShips::ColumnNames) == GameEmpireShips::NumColumns);

    // GameEmpireFleets
    Assert(countof(GameEmpireFleets::Types) == GameEmpireFleets::NumColumns);
    Assert(countof(GameEmpireFleets::Sizes) == GameEmpireFleets::NumColumns);
    Assert(countof(GameEmpireFleets::ColumnNames) == GameEmpireFleets::NumColumns);

    // GameSecurity
    Assert(countof(GameSecurity::Types) == GameSecurity::NumColumns);
    Assert(countof(GameSecurity::Sizes) == GameSecurity::NumColumns);
    Assert(countof(GameSecurity::ColumnNames) == GameSecurity::NumColumns);

Cleanup:

    *pbNewDatabase = bNewDatabase;
    *pbGoodDatabase = bGoodDatabase;
    *ppszBadTable = pszBadTable;
}

void GameEngine::VerifyGameTables(int iGameClass, int iGameNumber, bool* pbGoodDatabase) {

    int iErrCode;
    bool bGoodDatabase = true;

    char strBadTable [512];

    Variant vTemp, * pvEmpireKey = NULL;
    unsigned int iNumEmpires, i;

    int iGameOptions, iGameClassOptions;

    // Gameclass options
    iErrCode = t_pConn->GetViews()->ReadData(SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert(false);
        bGoodDatabase = false;
        goto Cleanup;
    }
    iGameClassOptions = vTemp.GetInteger();

    // GameData
    GET_GAME_DATA(strBadTable, iGameClass, iGameNumber);
    
    if (!t_pConn->DoesTableExist(strBadTable)) {
        Assert(false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    iErrCode = t_pConn->ReadData(strBadTable, GameData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert(false);
        bGoodDatabase = false;
        goto Cleanup;
    }
    iGameOptions = vTemp.GetInteger();

    if (iGameClassOptions & INDEPENDENCE) {

        GET_GAME_INDEPENDENT_SHIPS(strBadTable, iGameClass, iGameNumber);

        if (!t_pConn->DoesTableExist(strBadTable)) {
            Assert(false);
            bGoodDatabase = false;
            goto Cleanup;
        }
    }

    if (iGameOptions & GAME_ENFORCE_SECURITY) {

        GET_GAME_SECURITY(strBadTable, iGameClass, iGameNumber);

        if (!t_pConn->DoesTableExist(strBadTable)) {
            Assert(false);
            bGoodDatabase = false;
            goto Cleanup;
        }
    }

    // GameDeadEmpires
    GET_GAME_DEAD_EMPIRES(strBadTable, iGameClass, iGameNumber);
    
    if (!t_pConn->DoesTableExist(strBadTable)) {
        Assert(false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    // GameMap
    GET_GAME_MAP(strBadTable, iGameClass, iGameNumber);
    
    if (!t_pConn->DoesTableExist(strBadTable)) {
        Assert(false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    // GameEmpires
    GET_GAME_EMPIRES(strBadTable, iGameClass, iGameNumber);
    
    if (!t_pConn->DoesTableExist(strBadTable)) {
        Assert(false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    // Check empire tables  
    if (t_pConn->ReadColumn(strBadTable, GameEmpires::EmpireKey, &pvEmpireKey, &iNumEmpires) != OK) {
        Assert(false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    for(i = 0; i < iNumEmpires; i ++) {
        
        // GameEmpireData(I.I.I)
        GET_GAME_EMPIRE_DATA(strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!t_pConn->DoesTableExist(strBadTable)) {
            Assert(false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireMessages(I.I.I)
        GET_GAME_EMPIRE_MESSAGES(strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!t_pConn->DoesTableExist(strBadTable)) {
            Assert(false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireMap(I.I.I)
        GET_GAME_EMPIRE_MAP(strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!t_pConn->DoesTableExist(strBadTable)) {
            Assert(false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireDiplomacy(I.I.I)
        GET_GAME_EMPIRE_DIPLOMACY(strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!t_pConn->DoesTableExist(strBadTable)) {
            Assert(false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireShips(I.I.I)
        GET_GAME_EMPIRE_SHIPS(strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!t_pConn->DoesTableExist(strBadTable)) {
            Assert(false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireFleets(I.I.I)
        GET_GAME_EMPIRE_FLEETS(strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!t_pConn->DoesTableExist(strBadTable)) {
            Assert(false);
            bGoodDatabase = false;
            goto Cleanup;
        }
    }

Cleanup:

    *pbGoodDatabase = bGoodDatabase;

    if (pvEmpireKey != NULL) {
        t_pConn->FreeData(pvEmpireKey);
    }

    if (!bGoodDatabase) {

        char* pszMessage =(char*) StackAlloc(strlen(strBadTable) + 256);
        sprintf(pszMessage, "GameEngine setup found an inconsistency in the %s table", strBadTable);

        global.GetReport()->WriteReport(pszMessage);
    }
}

int GameEngine::VerifySystem() {

    int iErrCode;
    Variant vTemp;

    int iOptions;
    iErrCode = GetSystemOptions(&iOptions);
    if (iErrCode != OK) {
        Assert(false);
        goto Cleanup;
    }

    if (!(iOptions & ACCESS_ENABLED)) {

        iErrCode = GetSystemProperty(SystemData::AccessDisabledReason, &vTemp);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }

        if (strcmp(vTemp.GetString(), BACKUP_BLOCK_REASON) == 0) {

            iErrCode = SetSystemProperty(SystemData::AccessDisabledReason,(const char*) NULL);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }

            iErrCode = SetSystemOption(ACCESS_ENABLED, true);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }
        }
    }

    if (!(iOptions & NEW_EMPIRES_ENABLED)) {

        iErrCode = GetSystemProperty(SystemData::NewEmpiresDisabledReason, &vTemp);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }

        if (strcmp(vTemp.GetString(), BACKUP_BLOCK_REASON) == 0) {

            iErrCode = SetSystemProperty(SystemData::NewEmpiresDisabledReason,(const char*) NULL);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }

            iErrCode = SetSystemOption(NEW_EMPIRES_ENABLED, true);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }
        }
    }

Cleanup:

    return iErrCode;
}

int GameEngine::VerifyGameClasses() {

    int iErrCode;
    unsigned int* piGameClassKey = NULL, iNumGameClasses = 0, i;

    iErrCode = t_pConn->GetAllKeys(SYSTEM_GAMECLASS_DATA, &piGameClassKey, &iNumGameClasses);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert(false);
        return iErrCode;
    }

    for(i = 0; i < iNumGameClasses; i ++) {

        // TODOTODO - why?
        // Set number of active games in gameclass to 0
        iErrCode = t_pConn->WriteData(
            SYSTEM_GAMECLASS_DATA,
            piGameClassKey[i],
            SystemGameClassData::NumActiveGames,
            0
            );

        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
    }

Cleanup:

    if (piGameClassKey != NULL) {
        t_pConn->FreeKeys(piGameClassKey);
    }

    return iErrCode;
}


int GameEngine::VerifyMarkedGameClasses() {

    int iErrCode;
    unsigned int* piGameClassKey = NULL, iNumGameClasses = 0, i;

    iErrCode = t_pConn->GetAllKeys(SYSTEM_GAMECLASS_DATA, &piGameClassKey, &iNumGameClasses);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert(false);
        return iErrCode;
    }
    
    for(i = 0; i < iNumGameClasses; i ++) {

        Variant vOptions;
        
        iErrCode = t_pConn->ReadData(
            SYSTEM_GAMECLASS_DATA,
            piGameClassKey[i],
            SystemGameClassData::Options,
            &vOptions
            );

        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
            
        if (vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION) {
            
            // Make sure there are active games belonging to this game
            if (!DoesGameClassHaveActiveGames(piGameClassKey[i])) {
                
                // This game class needs to be deleted
                bool bDeleted;

                iErrCode = DeleteGameClass(piGameClassKey[i], &bDeleted);
                if (iErrCode != OK) {
                    Assert(false);
                    goto Cleanup;
                }
                Assert(bDeleted);
                
                if (bDeleted) {

                    char pszBuffer [128];
                    sprintf(
                        pszBuffer,
                        "GameEngine setup deleted gameclass %i because it was marked for deletion",
                        piGameClassKey[i]
                        );
                    global.GetReport()->WriteReport(pszBuffer);
                }
            }
        }
    }

Cleanup:

    if (piGameClassKey != NULL) {
        t_pConn->FreeKeys(piGameClassKey);
    }

    return iErrCode;
}


int GameEngine::VerifyActiveGames() {

    int iErrCode, iNumUpdatesDownBeforeGameIsKilled;
    unsigned int i, iNumGames;
    Seconds sSecondsForLongtermStatus;
    bool bUpdate;

    Variant* pvGame = NULL, * pvEmpireKey = NULL, vTemp;

    UTCTime tNewTime, tCurrentTime;
    Time::GetTime(&tCurrentTime);

    iErrCode = t_pConn->ReadColumn(
        SYSTEM_ACTIVE_GAMES, 
        SystemActiveGames::GameClassGameNumber,
        &pvGame,
        &iNumGames
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return OK;
    }

    if (iErrCode != OK) {
        Assert(false);
        goto Cleanup;
    }

    // Read some system data
    iErrCode = t_pConn->GetViews()->ReadData(SYSTEM_DATA, SystemData::SecondsForLongtermStatus, &vTemp);
    if (iErrCode != OK) {
        Assert(false);
        goto Cleanup;
    }
    sSecondsForLongtermStatus = vTemp.GetInteger();

    iErrCode = t_pConn->GetViews()->ReadData(SYSTEM_DATA, SystemData::NumUpdatesDownBeforeGameIsKilled, &vTemp);
    if (iErrCode != OK) {
        Assert(false);
        goto Cleanup;
    }
    iNumUpdatesDownBeforeGameIsKilled = vTemp.GetInteger();

    // Loop through all games
    for(i = 0; i < iNumGames; i ++) {

        char pszBuffer [512];

        Seconds sPeriod, sConsumedTime, sElapsedTime;
        UTCTime tLastUpdateTime, tLastCheckTime;

        bool bPasswordProtected, bPaused, bStarted, bGoodDatabase;
        unsigned int j, iNumEmpires, iNumPaused;

        int iGameClass, iGameNumber, iState;
        GetGameClassGameNumber(pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

        GAME_DATA(strGameData, iGameClass, iGameNumber);
        GAME_EMPIRES(strGameEmpires, iGameClass, iGameNumber);

        // Verify the game's tables
        VerifyGameTables(iGameClass, iGameNumber, &bGoodDatabase);
        if (!bGoodDatabase) {
            iErrCode = ERROR_DATA_CORRUPTION;
            goto Cleanup;
        }

        // Increment number of games in gameclass
        iErrCode = t_pConn->Increment(
            SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumActiveGames, 1);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
        
        // Get game update period
        iErrCode = t_pConn->ReadData(
            SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
        sPeriod = vTemp.GetInteger();
        
        // Get game state
        iErrCode = t_pConn->ReadData(strGameData, GameData::State, &vTemp);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
        iState = vTemp.GetInteger();

        bPaused =(iState & PAUSED) ||(iState & ADMIN_PAUSED);
        bStarted =(iState & STARTED) != 0;

        // Get game last update check time
        iErrCode = t_pConn->ReadData(strGameData, GameData::LastUpdateCheck, &vTemp);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
        tLastCheckTime = vTemp.GetInteger64();
        
        // Reset state
        iErrCode = t_pConn->WriteAnd(strGameData, GameData::State, ~GAME_BUSY);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
        
        // Get num empires
        iErrCode = t_pConn->GetNumRows(strGameEmpires, &iNumEmpires);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }

        // Is password protected?
        iErrCode = IsGamePasswordProtected(iGameClass, iGameNumber, &bPasswordProtected);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }

        // Get last update time
        iErrCode = t_pConn->ReadData(strGameData, GameData::LastUpdateTime, &vTemp);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
        tLastUpdateTime = vTemp.GetInteger64();

        // If started and not paused, reset last update time to current time minus 
        //(last shutdown time minus last update time)
        if (bStarted && !bPaused) {
            
            sConsumedTime = Time::GetSecondDifference(tLastCheckTime, tLastUpdateTime);
            if (sConsumedTime < 0) {
                Assert(false);
                sConsumedTime = 0;
            }

            // Write final update time to database
            Time::SubtractSeconds(tCurrentTime, sConsumedTime, &tNewTime);

            iErrCode = t_pConn->WriteData(strGameData, GameData::LastUpdateTime, tNewTime);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }
        }
        
        // Update empires' last login settings
        iErrCode = t_pConn->ReadColumn(
            strGameEmpires, 
            GameEmpires::EmpireKey, 
            &pvEmpireKey, 
            &iNumEmpires
            );

        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }

        // Loop through all empires in game
        iNumPaused = 0;

        for(j = 0; j < iNumEmpires; j ++) {

            UTCTime tLastLoginTime;
            int iOptions;
            unsigned int iEmpireKey = pvEmpireKey[j].GetInteger();

            GAME_EMPIRE_DATA(strEmpireData, iGameClass, iGameNumber, iEmpireKey);

            iErrCode = t_pConn->ReadData(strEmpireData, GameEmpireData::LastLogin, &vTemp);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }
            tLastLoginTime = vTemp.GetInteger64();
            
            sConsumedTime = Time::GetSecondDifference(tLastCheckTime, tLastLoginTime);
            if (sConsumedTime < 0) {
                sConsumedTime = 0;
            }
            tLastLoginTime = vTemp.GetInteger64();
            
            Time::SubtractSeconds(tCurrentTime, sConsumedTime, &tNewTime);
            
            iErrCode = t_pConn->WriteData(strEmpireData, GameEmpireData::LastLogin, tNewTime);
            if (iErrCode != OK) {
                Assert(false);
                t_pConn->FreeData(pvEmpireKey);
                goto Cleanup;
            }
            
            iErrCode = t_pConn->ReadData(strEmpireData, GameEmpireData::Options, &vTemp);
            if (iErrCode != OK) {
                Assert(false);
                t_pConn->FreeData(pvEmpireKey);
                goto Cleanup;
            }
            iOptions = vTemp.GetInteger();

            if (iOptions & REQUEST_PAUSE) {
                iNumPaused ++;
            }
        }

        // Update num paused
        iErrCode = t_pConn->WriteData(strGameData, GameData::NumRequestingPause, iNumPaused);
        if (iErrCode != OK) {
            Assert(false);
            goto Cleanup;
        }
        
        // Set paused status
        if (iNumPaused == iNumEmpires) {
            
            if (!bPaused) {

                bool bIdle;
                iErrCode = AreAllEmpiresIdle(iGameClass, iGameNumber, &bIdle);
                if (iErrCode != OK) {
                    Assert(false);
                    goto Cleanup;
                }

                if (!bIdle) {

                    iErrCode = PauseGame(iGameClass, iGameNumber, false, true);
                    if (iErrCode != OK) {
                        Assert(false);
                        goto Cleanup;
                    }
                }
            }
            
        } else {
            
            if (bPaused && !(iState & ADMIN_PAUSED)) {

                iErrCode = UnpauseGame(iGameClass, iGameNumber, false, true);
                if (iErrCode != OK) {
                    Assert(false);
                    goto Cleanup;
                }
            }
        }

        // Delete the game if it's in an 'interrupted' state
        if ((iState & GAME_BUSY) && !(iState & GAME_WAITING_TO_UPDATE)) {

            int iReason = iState & ~GAME_DELETION_REASON_MASK;
            Assert(iReason != 0);

            iErrCode = DeleteGame(iGameClass, iGameNumber, SYSTEM, "", iReason);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }

            sprintf(
                pszBuffer,
                "GameEngine setup deleted game %i of gameclass %i because it was in "\
                "inconsistent state %i",
                iGameNumber,
                iGameClass,
                iReason
                );

            global.GetReport()->WriteReport(pszBuffer);
            continue;
        }

        // Game should be killed if it's not paused and it's not a longterm and 
        // more than x updates have transpired while the server was down
        sElapsedTime = Time::GetSecondDifference(tCurrentTime, tLastCheckTime);

        if (!bPaused &&
            sPeriod < sSecondsForLongtermStatus && 
            sElapsedTime > sPeriod * iNumUpdatesDownBeforeGameIsKilled) {

            iErrCode = DeleteGame(iGameClass, iGameNumber, SYSTEM, "", SYSTEM_SHUTDOWN);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }
                
            sprintf(
                pszBuffer,
                "GameEngine setup deleted game %i of gameclass %i "\
                "because it grew stale during a system shutdown",
                iGameNumber,
                iGameClass
                );

            global.GetReport()->WriteReport(pszBuffer);
            continue;
        }

        // If game hasn't started and is password protected and has only one empire, kill it                    
        if (!(iState & STARTED) && bPasswordProtected && iNumEmpires == 1) {
            
            iErrCode = DeleteGame(iGameClass, iGameNumber, SYSTEM, "", PASSWORD_PROTECTED);
            if (iErrCode != OK) {
                Assert(false);
                goto Cleanup;
            }

            sprintf(
                pszBuffer,
                "GameEngine setup deleted game %i of gameclass %i "\
                "because it was password protected and only contained one empire",
                iGameNumber,
                iGameClass
                );
                
            global.GetReport()->WriteReport(pszBuffer);
            continue;
        }

        // Update the game
        iErrCode = CheckGameForUpdates(iGameClass, iGameNumber, true, &bUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }
    
Cleanup:

    if (pvGame != NULL) {
        t_pConn->FreeData(pvGame);
    }

    if (pvEmpireKey != NULL) {
        t_pConn->FreeData(pvEmpireKey);
    }

    return iErrCode;
}

//
// Creation
//

int GameEngine::InitializeNewDatabase() {
    
    int iErrCode;

    global.GetReport()->WriteReport("GameEngine setup is initializing a new database");
            
    iErrCode = CreateDefaultSystemTemplates();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup could not create the default system templates");
        return iErrCode;
    }

    iErrCode = CreateDefaultSystemTables();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup could not create the default system tables");
        return iErrCode;
    }

    iErrCode = SetupDefaultSystemTables();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup could not set up the default system tables");
        return iErrCode;
    }

    iErrCode = SetupDefaultSystemGameClasses();
    if (iErrCode != OK) {
        global.GetReport()->WriteReport("GameEngine setup could not set up the default system gameclasses");
        return iErrCode;
    }
    
    global.GetReport()->WriteReport("GameEngine setup finished initializing a new database");
    return iErrCode;
}


// Create default templates
int GameEngine::CreateDefaultSystemTemplates() {

    /*int iErrCode = m_pGameData->CreateTemplate(SystemData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemEmpireData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemGameClassData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemAlienIcons::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemSystemGameClassData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemSuperClassData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemThemes::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemActiveGames::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemEmpireMessages::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemEmpireNukeList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemNukeList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemLatestGames::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemEmpireActiveGames::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemTournaments::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemTournamentTeams::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemTournamentEmpires::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }
    
    iErrCode = m_pGameData->CreateTemplate(SystemTournamentActiveGames::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemEmpireTournaments::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameEmpires::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameDeadEmpires::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameMap::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameEmpireData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameEmpireMessages::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameEmpireMap::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameEmpireDiplomacy::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameEmpireShips::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameEmpireFleets::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameIndependentShips::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemAlmonasterScoreTopList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemClassicScoreTopList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemBridierScoreTopList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemBridierScoreEstablishedTopList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(SystemChatroomData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate(GameSecurity::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert(false);
        return iErrCode;
    }*/

    return OK;
}


//
// Create the default system tables
//
int GameEngine::CreateDefaultSystemTables() {

    int iErrCode;

    // Create SystemData table
    iErrCode = t_pConn->CreateTable(SYSTEM_DATA, SystemData::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemGameClassData table
    iErrCode = t_pConn->CreateTable(SYSTEM_GAMECLASS_DATA, SystemGameClassData::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemSystemGameClassData table
    iErrCode = t_pConn->CreateTable(SYSTEM_SYSTEM_GAMECLASS_DATA, SystemSystemGameClassData::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemActiveGames table
    iErrCode = t_pConn->CreateTable(SYSTEM_ACTIVE_GAMES, SystemActiveGames::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemEmpireData table
    iErrCode = t_pConn->CreateTable(SYSTEM_EMPIRE_DATA, SystemEmpireData::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemThemes table
    iErrCode = t_pConn->CreateTable(SYSTEM_THEMES, SystemThemes::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemSuperClassData table
    iErrCode = t_pConn->CreateTable(SYSTEM_SUPERCLASS_DATA, SystemSuperClassData::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemAlienIcons table
    iErrCode = t_pConn->CreateTable(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemAlmonasterScoreTopList table
    iErrCode = t_pConn->CreateTable(SYSTEM_ALMONASTER_SCORE_TOPLIST, SystemAlmonasterScoreTopList::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemClassicScoreTopList table
    iErrCode = t_pConn->CreateTable(SYSTEM_CLASSIC_SCORE_TOPLIST, SystemClassicScoreTopList::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemBridierScoreTopList table
    iErrCode = t_pConn->CreateTable(SYSTEM_BRIDIER_SCORE_TOPLIST, SystemBridierScoreTopList::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemBridierScoreEstablishedTopList table
    iErrCode = t_pConn->CreateTable(SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST, SystemBridierScoreEstablishedTopList::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemNukeList table
    iErrCode = t_pConn->CreateTable(SYSTEM_NUKE_LIST, SystemNukeList::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemLatestGames table
    iErrCode = t_pConn->CreateTable(SYSTEM_LATEST_GAMES, SystemLatestGames::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemTournaments table
    iErrCode = t_pConn->CreateTable(SYSTEM_TOURNAMENTS, SystemTournaments::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Create SystemChatroomData table
    iErrCode = t_pConn->CreateTable(SYSTEM_CHATROOM_DATA, SystemChatroomData::Template);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    return iErrCode;
}


// Insert default data into the system tables
int GameEngine::SetupDefaultSystemTables() {
    
    int iErrCode;
    unsigned int iKey;

    UTCTime tTime;
    Time::GetTime(&tTime);

    // Set up themes first
    unsigned int iDefaultThemeKey;
    iErrCode = SetupDefaultThemes(&iDefaultThemeKey);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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

    iErrCode = t_pConn->InsertRow(SYSTEM_DATA, SystemData::Template, pvSystemData, NULL);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Set up default administrator empire(root)
    iErrCode = CreateEmpire(ROOT_NAME, ROOT_DEFAULT_PASSWORD, ADMINISTRATOR, NO_KEY, true, &iKey);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = SetEmpireOption2(iKey, EMPIRE_ACCEPTED_TOS, true);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    // Set up default guest empire(Guest)
    iErrCode = CreateEmpire(
        GUEST_NAME,
        GUEST_DEFAULT_PASSWORD,
        GUEST,
        NO_KEY,
        true,
        &iKey
        );

    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = SetEmpireOption(iKey, CAN_BROADCAST, false);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    iErrCode = SetEmpireOption2(iKey, EMPIRE_ACCEPTED_TOS, true);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    return iErrCode;
}

// Create default superclasses and gameclasses
int GameEngine::SetupDefaultSystemGameClasses() {

    // Add SuperClasses
    int i, iErrCode, iBeginnerKey, iGrudgeKey, iLongTermKey, iBlitzKey, iGameClass;
    
    iErrCode = CreateSuperClass("Beginner Games", &iBeginnerKey);
    if (iErrCode != OK && iErrCode != ERROR_SUPERCLASS_ALREADY_EXISTS) {
        return iErrCode;
    }

    iErrCode = CreateSuperClass("Grudge Matches", &iGrudgeKey);
    if (iErrCode != OK && iErrCode != ERROR_SUPERCLASS_ALREADY_EXISTS) {
        return iErrCode;
    }

    iErrCode = CreateSuperClass("Blitzes", &iBlitzKey);
    if (iErrCode != OK && iErrCode != ERROR_SUPERCLASS_ALREADY_EXISTS) {
        return iErrCode;
    }

    iErrCode = CreateSuperClass("Longterm Games", &iLongTermKey);
    if (iErrCode != OK && iErrCode != ERROR_SUPERCLASS_ALREADY_EXISTS) {
        return iErrCode;
    }

    Variant pvSubmitArray [SystemGameClassData::NumColumns];

    // Beginner Tekno Blood
    pvSubmitArray[SystemGameClassData::iName] = "Beginner Tekno Blood";
    pvSubmitArray[SystemGameClassData::iMaxNumEmpires] = 10;
    pvSubmitArray[SystemGameClassData::iMaxNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::iMaxTechDev] =(float) 3.5;
    pvSubmitArray[SystemGameClassData::iOpenGameNum] = 1;                            
    pvSubmitArray[SystemGameClassData::iNumSecPerUpdate] = 210;
    pvSubmitArray[SystemGameClassData::iOptions] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | ONLY_SURRENDER_WITH_TWO_EMPIRES;
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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }


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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }


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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }


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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    
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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    
    
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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    
    
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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    
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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }


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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    

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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }


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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
        
    
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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }


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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }


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
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }


    //////////////////////
    // SystemAlienIcons //
    //////////////////////
    
    // 1 to 42
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ken Eppstein";
    for(i = 1; i <= 42; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 43 to 82
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ronald Kinion";
    for(i = 43; i <= 82; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 83
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 83;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Unknown";
    iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    
    // 84 to 89
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ronald Kinion";
    for(i = 84; i <= 89; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 90
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 90;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Jens Klavsen";
    iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    
    // 91
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 91;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Chris John";
    iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    
    // 92 to 101
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Jens Klavsen";
    for(i = 92; i <= 101; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 102 to 103
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ronald Kinion";
    for(i = 102; i <= 103; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 104 to 105
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Chris John";
    for(i = 104; i <= 105; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 106 to 125
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Ronald Kinion";
    for(i = 106; i <= 125; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 126 to 127
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Chris John";
    for(i = 126; i <= 127; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 128
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 128;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Haavard Fledsberg";
    iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    
    // 129 to 135
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Chris John";
    for(i = 129; i <= 135; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 136 to 145
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Jens Klavsen";
    for(i = 136; i <= 145; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }
    
    // 146
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 146;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Michel Lemieux";
    iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }
    
    // 147 to 152
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Unknown";
    for(i = 147; i <= 152; i ++) {
        pvSubmitArray[SystemAlienIcons::iAlienKey] = i;
        iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
    }

    // 153
    pvSubmitArray[SystemAlienIcons::iAlienKey] = 153;
    pvSubmitArray[SystemAlienIcons::iAuthorName] = "Kia";
    iErrCode = t_pConn->InsertRow(SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template, pvSubmitArray, NULL);
    if (iErrCode != OK) {
        Assert(false);
        return iErrCode;
    }

    return iErrCode;
}

int GameEngine::VerifyTournaments() {

    int iErrCode;
    unsigned int iKey = NO_KEY, iNumGames, iOwner;

    bool bExists;

    while(true) {

        iErrCode = t_pConn->GetNextKey(SYSTEM_TOURNAMENTS, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }
        
        iErrCode = GetTournamentOwner(iKey, &iOwner);
        if (iErrCode != OK) {
            Assert(false);
            return iErrCode;
        }

        // Check for tournament that needs to be deleted
        if (iOwner == DELETED_EMPIRE_KEY) {

            iErrCode = GetTournamentGames(iKey, NULL, NULL, &iNumGames);
            if (iErrCode != OK) {
                Assert(false);
                return iErrCode;
            }

            if (iNumGames == 0) {

                iErrCode = DeleteTournament(DELETED_EMPIRE_KEY, iKey, false);
                if (iErrCode != OK) {
                    Assert(false);
                    return iErrCode;
                }
            }
        }

        // Check for tournament without empire
        else if (iOwner != SYSTEM) {

            iErrCode = DoesEmpireExist(iOwner, &bExists, NULL);
            if (iErrCode != OK) {
                Assert(false);
                return iErrCode;
            }

            if (!bExists) {

                iErrCode = DeleteTournament(iOwner, iKey, true);
                if (iErrCode != OK && iErrCode != ERROR_TOURNAMENT_HAS_GAMES) {
                    Assert(false);
                    return iErrCode;
                }
            }
        }
    }
    
    return OK;
}

int GameEngine::VerifyTopLists() {

    int i, iErrCode;
    bool bRebuild = false;
    
    ENUMERATE_SCORING_SYSTEMS(i) {
        
        ScoringSystem ssTopList =(ScoringSystem) i;
        if (HasTopList(ssTopList)) {

            if (!bRebuild) {
                
                iErrCode = VerifyTopList(ssTopList);
                if (iErrCode != OK) {
                    bRebuild = true;
                }
            }
            
            if (bRebuild) {
                
                iErrCode = RebuildTopList(ssTopList);
                if (iErrCode != OK) {
                    Assert(false);
                    return iErrCode;
                }
            }
        }
    }

    return OK;
}

int GameEngine::RebuildTopLists() {

    int i, iErrCode = OK;

    ENUMERATE_SCORING_SYSTEMS(i) {

        ScoringSystem ssTopList =(ScoringSystem) i;
        if (HasTopList(ssTopList)) {

            iErrCode = RebuildTopList(ssTopList);
            if (iErrCode != OK) break;
        }
    }

    return iErrCode;
}

int GameEngine::RebuildTopList(ScoringSystem ssTopList) {
    
    int iErrCode;
    const char* pszTableName = TOPLIST_TABLE_NAME[ssTopList];
    
    iErrCode = t_pConn->DeleteAllRows(pszTableName);
    if (iErrCode == OK) {
        iErrCode = InitializeEmptyTopList(ssTopList);
        Assert(iErrCode == OK);
    }

    else Assert(false);

    return iErrCode;
}
