//
// GameEngine.dll:  a component of Almonaster
// Copyright (c) 1998-2004 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "GameEngine.h"

int GameEngine::Setup() {

    int iErrCode;

    bool bNewDatabase, bGoodDatabase;
    const char* pszBadTable;

    /////////////////////////
    // Check system tables //
    /////////////////////////

    m_pReport->WriteReport ("GameEngine setup attempting to reuse an existing database");
    VerifySystemTables (&bNewDatabase, &bGoodDatabase, &pszBadTable);

    if (!bGoodDatabase) {

        if (bNewDatabase) {

            // Create a new database and we're done
            iErrCode = CreateNewDatabase();
            m_bGoodDatabase = (iErrCode == OK);
            return iErrCode;
        }

        // Bad database - report error
        char* pszMessage = (char*) StackAlloc (strlen (pszBadTable) + 256);

        sprintf (
            pszMessage,
            "GameEngine setup found errors in the %s table or its template",
            pszBadTable
            );

        m_pReport->WriteReport (pszMessage);
        m_pReport->WriteReport ("GameEngine setup could not successfully reuse an existing database");
        
        m_bGoodDatabase = false;
        return ERROR_FAILURE;
    }

    // Test the reloaded database
    return ReloadDatabase();
}


int GameEngine::ReloadDatabase() {

    int iErrCode;

    //
    // System
    //

    iErrCode = VerifySystem();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup failed to verify system data");
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine setup successfully verified system data");

    //
    // Empires
    //

    iErrCode = VerifyEmpires();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup failed to verify empire data");
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine setup successfully verified empire data");

    //
    // Gameclasses
    //

    iErrCode = VerifyGameClasses();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup failed to verify gameclasses");
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine setup successfully verified gameclasses");

    //
    // Games
    //

    iErrCode = VerifyActiveGames();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup failed to verify active games");
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine setup successfully verified active games");    

    //
    // Marked gameclasses
    //

    iErrCode = VerifyMarkedGameClasses();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup failed to verify marked gameclasses");
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine setup successfully verified marked gameclasses");

    //
    // Tournaments
    //

    iErrCode = VerifyTournaments();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup failed to verify tournaments");
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine setup successfully verified tournaments");

    //
    // Top lists
    //

    iErrCode = VerifyTopLists();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup failed to verify top lists");
        return iErrCode;
    }
    m_pReport->WriteReport ("GameEngine setup successfully verified top lists");

    //////////
    // Done //
    //////////

    m_bGoodDatabase = true;
    m_pReport->WriteReport ("GameEngine setup successfully reused the existing database");

    return OK;
}

void GameEngine::VerifySystemTables (bool* pbNewDatabase, bool* pbGoodDatabase, const char** ppszBadTable) {

    const char* pszBadTable = NULL;
    bool bNewDatabase = false, bGoodDatabase = true;

    unsigned int iNumRows;

    // SystemData
    pszBadTable = SYSTEM_DATA;
    if (!m_pGameData->DoesTableExist (SYSTEM_DATA)) {
        bNewDatabase = true;
        bGoodDatabase = false;
        goto Cleanup;
    }
    
    if (m_pGameData->GetNumRows (SYSTEM_DATA, &iNumRows) != OK || iNumRows != 1 ||
        !m_pGameData->IsTemplateEqual (SystemData::Template.Name, SystemData::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    // SystemEmpireData
    pszBadTable = SYSTEM_EMPIRE_DATA;
    if (!m_pGameData->DoesTableExist (SYSTEM_EMPIRE_DATA) ||
        !m_pGameData->IsTemplateEqual (SystemEmpireData::Template.Name, SystemEmpireData::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    if (m_pGameData->GetNumRows (SYSTEM_EMPIRE_DATA, &iNumRows) != OK || iNumRows < 1) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    // SystemGameClassData
    pszBadTable = SYSTEM_GAMECLASS_DATA;
    if (!m_pGameData->DoesTableExist (SYSTEM_GAMECLASS_DATA) ||
        !m_pGameData->IsTemplateEqual (SystemGameClassData::Template.Name, SystemGameClassData::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    // SystemAlienIcons
    pszBadTable = SYSTEM_ALIEN_ICONS;
    if (!m_pGameData->DoesTableExist (SYSTEM_ALIEN_ICONS) ||
        !m_pGameData->IsTemplateEqual (SystemAlienIcons::Template.Name, SystemAlienIcons::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    if (m_pGameData->GetNumRows (SYSTEM_DATA, &iNumRows) != OK || iNumRows < 1) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }       
    
    // SystemSystemGameClassData
    pszBadTable = SYSTEM_SYSTEM_GAMECLASS_DATA;
    if (!m_pGameData->DoesTableExist (SYSTEM_SYSTEM_GAMECLASS_DATA) ||
        !m_pGameData->IsTemplateEqual (SystemSystemGameClassData::Template.Name, 
        SystemSystemGameClassData::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    // SystemSuperClassData
    pszBadTable = SYSTEM_SUPERCLASS_DATA;
    if (!m_pGameData->DoesTableExist (SYSTEM_SUPERCLASS_DATA) ||
        !m_pGameData->IsTemplateEqual (SystemSuperClassData::Template.Name, SystemSuperClassData::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    // SystemThemes
    pszBadTable = SYSTEM_THEMES;
    if (!m_pGameData->DoesTableExist (SYSTEM_THEMES) ||
        !m_pGameData->IsTemplateEqual (SystemThemes::Template.Name, SystemThemes::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    // SystemActiveGames
    pszBadTable = SYSTEM_ACTIVE_GAMES;
    if (!m_pGameData->DoesTableExist (SYSTEM_ACTIVE_GAMES) ||
        !m_pGameData->IsTemplateEqual (SystemActiveGames::Template.Name, SystemActiveGames::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    // SystemAlmonasterScoreTopList
    pszBadTable = SYSTEM_ALMONASTER_SCORE_TOPLIST;
    if (!m_pGameData->DoesTableExist (SYSTEM_ALMONASTER_SCORE_TOPLIST) ||
        !m_pGameData->IsTemplateEqual (SystemAlmonasterScoreTopList::Template.Name, SystemAlmonasterScoreTopList::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    // SystemClassicScoreTopList
    pszBadTable = SYSTEM_CLASSIC_SCORE_TOPLIST;
    if (!m_pGameData->DoesTableExist (SYSTEM_CLASSIC_SCORE_TOPLIST) ||
        !m_pGameData->IsTemplateEqual (SystemClassicScoreTopList::Template.Name, SystemClassicScoreTopList::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    // SystemBridierScoreTopList
    pszBadTable = SYSTEM_BRIDIER_SCORE_TOPLIST;
    if (!m_pGameData->DoesTableExist (SYSTEM_BRIDIER_SCORE_TOPLIST) ||
        !m_pGameData->IsTemplateEqual (SystemBridierScoreTopList::Template.Name, SystemBridierScoreTopList::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    // SystemBridierScoreEstablishedTopList
    pszBadTable = SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST;
    if (!m_pGameData->DoesTableExist (SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST) ||
        !m_pGameData->IsTemplateEqual (SystemBridierScoreEstablishedTopList::Template.Name, SystemBridierScoreEstablishedTopList::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    // SystemTournaments
    pszBadTable = SYSTEM_TOURNAMENTS;
    if (!m_pGameData->DoesTableExist (SYSTEM_TOURNAMENTS) ||
        !m_pGameData->IsTemplateEqual (SystemTournaments::Template.Name, SystemTournaments::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemEmpireMessages::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemEmpireMessages::Template.Name, SystemEmpireMessages::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemEmpireNukeList::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemEmpireNukeList::Template.Name, SystemEmpireNukeList::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemNukeList::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemNukeList::Template.Name, SystemNukeList::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemLatestGames::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemLatestGames::Template.Name, SystemLatestGames::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemEmpireActiveGames::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemEmpireActiveGames::Template.Name, SystemEmpireActiveGames::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemTournamentTeams::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemTournamentTeams::Template.Name, SystemTournamentTeams::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemTournamentEmpires::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemTournamentEmpires::Template.Name, SystemTournamentEmpires::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemTournamentActiveGames::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemTournamentActiveGames::Template.Name, SystemTournamentActiveGames::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = SystemEmpireTournaments::Template.Name;
    if (!m_pGameData->IsTemplateEqual (SystemEmpireTournaments::Template.Name, SystemEmpireTournaments::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    // Check game templates
    pszBadTable = GameData::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameData::Template.Name, GameData::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    pszBadTable = GameEmpires::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameEmpires::Template.Name, GameEmpires::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = GameDeadEmpires::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameDeadEmpires::Template.Name, GameDeadEmpires::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    pszBadTable = GameMap::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameMap::Template.Name, GameMap::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = GameEmpireData::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameEmpireData::Template.Name, GameEmpireData::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    pszBadTable = GameEmpireMessages::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameEmpireMessages::Template.Name, GameEmpireMessages::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    pszBadTable = GameEmpireMap::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameEmpireMap::Template.Name, GameEmpireMap::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    pszBadTable = GameEmpireDiplomacy::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameEmpireDiplomacy::Template.Name, GameEmpireDiplomacy::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    pszBadTable = GameEmpireShips::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameEmpireShips::Template.Name, GameEmpireShips::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }
    
    pszBadTable = GameEmpireFleets::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameEmpireFleets::Template.Name, GameEmpireFleets::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

    pszBadTable = GameSecurity::Template.Name;
    if (!m_pGameData->IsTemplateEqual (GameSecurity::Template.Name, GameSecurity::Template)) {
        bGoodDatabase = false;
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    *pbNewDatabase = bNewDatabase;
    *pbGoodDatabase = bGoodDatabase;
    *ppszBadTable = pszBadTable;
}

void GameEngine::VerifyGameTables (int iGameClass, int iGameNumber, bool* pbGoodDatabase) {

    int iErrCode;
    bool bGoodDatabase = true;

    char strBadTable [512];

    Variant vTemp, * pvEmpireKey = NULL;
    unsigned int iNumEmpires, i;

    int iGameOptions, iGameClassOptions;

    // Gameclass options
    iErrCode = m_pGameData->ReadData (SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        bGoodDatabase = false;
        goto Cleanup;
    }
    iGameClassOptions = vTemp.GetInteger();

    // GameData
    GET_GAME_DATA (strBadTable, iGameClass, iGameNumber);
    
    if (!m_pGameData->DoesTableExist (strBadTable)) {
        Assert (false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (strBadTable, GameData::Options, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        bGoodDatabase = false;
        goto Cleanup;
    }
    iGameOptions = vTemp.GetInteger();

    if (iGameClassOptions & INDEPENDENCE) {

        GET_GAME_INDEPENDENT_SHIPS (strBadTable, iGameClass, iGameNumber);

        if (!m_pGameData->DoesTableExist (strBadTable)) {
            Assert (false);
            bGoodDatabase = false;
            goto Cleanup;
        }
    }

    if (iGameOptions & GAME_ENFORCE_SECURITY) {

        GET_GAME_SECURITY (strBadTable, iGameClass, iGameNumber);

        if (!m_pGameData->DoesTableExist (strBadTable)) {
            Assert (false);
            bGoodDatabase = false;
            goto Cleanup;
        }
    }

    // GameDeadEmpires
    GET_GAME_DEAD_EMPIRES (strBadTable, iGameClass, iGameNumber);
    
    if (!m_pGameData->DoesTableExist (strBadTable)) {
        Assert (false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    // GameMap
    GET_GAME_MAP (strBadTable, iGameClass, iGameNumber);
    
    if (!m_pGameData->DoesTableExist (strBadTable)) {
        Assert (false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    // GameEmpires
    GET_GAME_EMPIRES (strBadTable, iGameClass, iGameNumber);
    
    if (!m_pGameData->DoesTableExist (strBadTable)) {
        Assert (false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    // Check empire tables  
    if (m_pGameData->ReadColumn (strBadTable, GameEmpires::EmpireKey, &pvEmpireKey, &iNumEmpires) != OK) {
        Assert (false);
        bGoodDatabase = false;
        goto Cleanup;
    }

    for (i = 0; i < iNumEmpires; i ++) {
        
        // GameEmpireData(I.I.I)
        GET_GAME_EMPIRE_DATA (strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!m_pGameData->DoesTableExist (strBadTable)) {
            Assert (false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireMessages(I.I.I)
        GET_GAME_EMPIRE_MESSAGES (strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!m_pGameData->DoesTableExist (strBadTable)) {
            Assert (false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireMap(I.I.I)
        GET_GAME_EMPIRE_MAP (strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!m_pGameData->DoesTableExist (strBadTable)) {
            Assert (false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireDiplomacy(I.I.I)
        GET_GAME_EMPIRE_DIPLOMACY (strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!m_pGameData->DoesTableExist (strBadTable)) {
            Assert (false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireShips(I.I.I)
        GET_GAME_EMPIRE_SHIPS (strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!m_pGameData->DoesTableExist (strBadTable)) {
            Assert (false);
            bGoodDatabase = false;
            goto Cleanup;
        }

        // GameEmpireFleets(I.I.I)
        GET_GAME_EMPIRE_FLEETS (strBadTable, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
        
        if (!m_pGameData->DoesTableExist (strBadTable)) {
            Assert (false);
            bGoodDatabase = false;
            goto Cleanup;
        }
    }

Cleanup:

    *pbGoodDatabase = bGoodDatabase;

    if (pvEmpireKey != NULL) {
        m_pGameData->FreeData (pvEmpireKey);
    }

    if (!bGoodDatabase) {

        char* pszMessage = (char*) StackAlloc (strlen (strBadTable) + 256);
        sprintf (pszMessage, "GameEngine setup found an inconsistency in the %s table", strBadTable);

        m_pReport->WriteReport (pszMessage);
    }
}

int GameEngine::VerifySystem() {

    int iErrCode;
    Variant vTemp;

    int iOptions;
    iErrCode = GetSystemOptions (&iOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!(iOptions & ACCESS_ENABLED)) {

        iErrCode = GetSystemProperty (SystemData::AccessDisabledReason, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (strcmp (vTemp.GetString(), BACKUP_BLOCK_REASON) == 0) {

            iErrCode = SetSystemProperty (SystemData::AccessDisabledReason, (const char*) NULL);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = SetSystemOption (ACCESS_ENABLED, true);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

    if (!(iOptions & NEW_EMPIRES_ENABLED)) {

        iErrCode = GetSystemProperty (SystemData::NewEmpiresDisabledReason, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        if (strcmp (vTemp.GetString(), BACKUP_BLOCK_REASON) == 0) {

            iErrCode = SetSystemProperty (SystemData::NewEmpiresDisabledReason, (const char*) NULL);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = SetSystemOption (NEW_EMPIRES_ENABLED, true);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }

Cleanup:

    return iErrCode;
}

int GameEngine::VerifyEmpires() {

    int iErrCode = OK;

    IWriteTable* pEmpires = NULL;

    Variant vTemp;
    Seconds sDiff;

    UTCTime tLastShutdownTime, tNow;
    Time::GetTime (&tNow);

    iErrCode = GetSystemProperty (SystemData::LastShutdownTime, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    tLastShutdownTime = vTemp.GetUTCTime();

    sDiff = Time::GetSecondDifference (tNow, tLastShutdownTime);

    // If the server was down longer than a week, 
    if (sDiff > 7 * DAY_LENGTH_IN_SECONDS) {

        unsigned int iKey = NO_KEY;

        iErrCode = m_pGameData->GetTableForWriting (SYSTEM_EMPIRE_DATA, &pEmpires);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Update every empire's Bridier timebomb
        while (true) {

            UTCTime tBridier, tNewBridier;

            iErrCode = pEmpires->GetNextKey (iKey, &iKey);
            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
                break;
            }
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            iErrCode = pEmpires->ReadData (iKey, SystemEmpireData::LastBridierActivity, &tBridier);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            Time::AddSeconds (tBridier, sDiff, &tNewBridier);

            iErrCode = pEmpires->WriteData (iKey, SystemEmpireData::LastBridierActivity, tNewBridier);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
    }    

Cleanup:

    SafeRelease (pEmpires);

    return iErrCode;
}

int GameEngine::VerifyGameClasses() {

    int iErrCode;
    unsigned int* piGameClassKey = NULL, iNumGameClasses = 0, i;

    iErrCode = m_pGameData->GetAllKeys (SYSTEM_GAMECLASS_DATA, &piGameClassKey, &iNumGameClasses);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert (false);
        return iErrCode;
    }

    for (i = 0; i < iNumGameClasses; i ++) {

        // Set number of active games in gameclass to 0
        iErrCode = m_pGameData->WriteData (
            SYSTEM_GAMECLASS_DATA,
            piGameClassKey[i],
            SystemGameClassData::NumActiveGames,
            0
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    if (piGameClassKey != NULL) {
        m_pGameData->FreeKeys (piGameClassKey);
    }

    return iErrCode;
}


int GameEngine::VerifyMarkedGameClasses() {

    int iErrCode;
    unsigned int* piGameClassKey = NULL, iNumGameClasses = 0, i;

    iErrCode = m_pGameData->GetAllKeys (SYSTEM_GAMECLASS_DATA, &piGameClassKey, &iNumGameClasses);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
        }
        else Assert (false);
        return iErrCode;
    }
    
    for (i = 0; i < iNumGameClasses; i ++) {

        Variant vOptions;
        
        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA,
            piGameClassKey[i],
            SystemGameClassData::Options,
            &vOptions
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
            
        if (vOptions.GetInteger() & GAMECLASS_MARKED_FOR_DELETION) {
            
            // Make sure there are active games belonging to this game
            if (!DoesGameClassHaveActiveGames (piGameClassKey[i])) {
                
                // This game class needs to be deleted
                bool bDeleted;

                iErrCode = DeleteGameClass (piGameClassKey[i], &bDeleted);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
                Assert (bDeleted);
                
                if (bDeleted) {

                    char pszBuffer [128];
                    sprintf (
                        pszBuffer,
                        "GameEngine setup deleted gameclass %i because it was marked for deletion",
                        piGameClassKey[i]
                        );
                    m_pReport->WriteReport (pszBuffer);
                }
            }
        }
    }

Cleanup:

    if (piGameClassKey != NULL) {
        m_pGameData->FreeKeys (piGameClassKey);
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
    Time::GetTime (&tCurrentTime);

    iErrCode = m_pGameData->ReadColumn (
        SYSTEM_ACTIVE_GAMES, 
        SystemActiveGames::GameClassGameNumber,
        &pvGame,
        &iNumGames
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        return OK;
    }

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Read some system data
    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::SecondsForLongtermStatus, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    sSecondsForLongtermStatus = vTemp.GetInteger();

    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::NumUpdatesDownBeforeGameIsKilled, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iNumUpdatesDownBeforeGameIsKilled = vTemp.GetInteger();

    // Loop through all games
    for (i = 0; i < iNumGames; i ++) {

        char pszBuffer [512];

        Seconds sPeriod, sConsumedTime, sElapsedTime;
        UTCTime tLastUpdateTime, tLastCheckTime;

        bool bPasswordProtected, bPaused, bStarted, bGoodDatabase;
        unsigned int j, iNumEmpires, iNumPaused;

        int iGameClass, iGameNumber, iState;
        GetGameClassGameNumber (pvGame[i].GetCharPtr(), &iGameClass, &iGameNumber);

        GAME_DATA (strGameData, iGameClass, iGameNumber);
        GAME_EMPIRES (strGameEmpires, iGameClass, iGameNumber);

        // Verify the game's tables
        VerifyGameTables (iGameClass, iGameNumber, &bGoodDatabase);
        if (!bGoodDatabase) {
            iErrCode = ERROR_DATA_CORRUPTION;
            goto Cleanup;
        }

        // Add the game to our game table
        iErrCode = AddToGameTable (iGameClass, iGameNumber);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Increment number of games in gameclass
        iErrCode = m_pGameData->Increment (
            SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumActiveGames, 1);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Get game update period
        iErrCode = m_pGameData->ReadData (
            SYSTEM_GAMECLASS_DATA, iGameClass, SystemGameClassData::NumSecPerUpdate, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        sPeriod = vTemp.GetInteger();
        
        // Get game state
        iErrCode = m_pGameData->ReadData (strGameData, GameData::State, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iState = vTemp.GetInteger();

        bPaused = (iState & PAUSED) || (iState & ADMIN_PAUSED);
        bStarted = (iState & STARTED) != 0;

        // Get game last update check time
        iErrCode = m_pGameData->ReadData (strGameData, GameData::LastUpdateCheck, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        tLastCheckTime = vTemp.GetUTCTime();
        
        // Reset state
        iErrCode = m_pGameData->WriteAnd (strGameData, GameData::State, ~GAME_BUSY);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Get num empires
        iErrCode = m_pGameData->GetNumRows (strGameEmpires, &iNumEmpires);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Is password protected?
        iErrCode = IsGamePasswordProtected (iGameClass, iGameNumber, &bPasswordProtected);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Get last update time
        iErrCode = m_pGameData->ReadData (strGameData, GameData::LastUpdateTime, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        tLastUpdateTime = vTemp.GetUTCTime();

        // If started and not paused, reset last update time to current time minus 
        // (last shutdown time minus last update time)
        if (bStarted && !bPaused) {
            
            sConsumedTime = Time::GetSecondDifference (tLastCheckTime, tLastUpdateTime);
            if (sConsumedTime < 0) {
                Assert (false);
                sConsumedTime = 0;
            }

            // Write final update time to database
            Time::SubtractSeconds (tCurrentTime, sConsumedTime, &tNewTime);

            iErrCode = m_pGameData->WriteData (strGameData, GameData::LastUpdateTime, tNewTime);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
        }
        
        // Update empires' last login settings
        iErrCode = m_pGameData->ReadColumn (
            strGameEmpires, 
            GameEmpires::EmpireKey, 
            &pvEmpireKey, 
            &iNumEmpires
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Loop through all empires in game
        iNumPaused = 0;

        for (j = 0; j < iNumEmpires; j ++) {

            UTCTime tLastLoginTime;
            int iOptions;
            unsigned int iEmpireKey = pvEmpireKey[j].GetInteger();

            GAME_EMPIRE_DATA (strEmpireData, iGameClass, iGameNumber, iEmpireKey);

            iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::LastLogin, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            tLastLoginTime = vTemp.GetUTCTime();
            
            sConsumedTime = Time::GetSecondDifference (tLastCheckTime, tLastLoginTime);
            if (sConsumedTime < 0) {
                Assert (false);
                sConsumedTime = 0;
            }
            tLastLoginTime = vTemp.GetUTCTime();
            
            Time::SubtractSeconds (tCurrentTime, sConsumedTime, &tNewTime);
            
            iErrCode = m_pGameData->WriteData (strEmpireData, GameEmpireData::LastLogin, tNewTime);
            if (iErrCode != OK) {
                Assert (false);
                m_pGameData->FreeData (pvEmpireKey);
                goto Cleanup;
            }
            
            iErrCode = m_pGameData->ReadData (strEmpireData, GameEmpireData::Options, &vTemp);
            if (iErrCode != OK) {
                Assert (false);
                m_pGameData->FreeData (pvEmpireKey);
                goto Cleanup;
            }
            iOptions = vTemp.GetInteger();

            if (iOptions & REQUEST_PAUSE) {
                iNumPaused ++;
            }
        }

        // Update num paused
        iErrCode = m_pGameData->WriteData (strGameData, GameData::NumRequestingPause, iNumPaused);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        // Set paused status
        if (iNumPaused == iNumEmpires) {
            
            if (!bPaused) {

                bool bIdle;
                iErrCode = AreAllEmpiresIdle (iGameClass, iGameNumber, &bIdle);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }

                if (!bIdle) {

                    iErrCode = PauseGame (iGameClass, iGameNumber, false, true);
                    if (iErrCode != OK) {
                        Assert (false);
                        goto Cleanup;
                    }
                }
            }
            
        } else {
            
            if (bPaused && !(iState & ADMIN_PAUSED)) {

                iErrCode = UnpauseGame (iGameClass, iGameNumber, false, true);
                if (iErrCode != OK) {
                    Assert (false);
                    goto Cleanup;
                }
            }
        }

        // Delete the game if it's in an 'interrupted' state
        if ((iState & GAME_BUSY) && !(iState & GAME_WAITING_TO_UPDATE)) {

            int iReason = iState & ~GAME_DELETION_REASON_MASK;
            Assert (iReason != 0);

            iErrCode = DeleteGame (iGameClass, iGameNumber, SYSTEM, "", iReason);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            sprintf (
                pszBuffer,
                "GameEngine setup deleted game %i of gameclass %i because it was in "\
                "inconsistent state %i",
                iGameNumber,
                iGameClass,
                iReason
                );

            m_pReport->WriteReport (pszBuffer);
            continue;
        }

        // Game should be killed if it's not paused and it's not a longterm and 
        // more than x updates have transpired while the server was down
        sElapsedTime = Time::GetSecondDifference (tCurrentTime, tLastCheckTime);

        if (!bPaused &&
            sPeriod < sSecondsForLongtermStatus && 
            sElapsedTime > sPeriod * iNumUpdatesDownBeforeGameIsKilled) {

            iErrCode = DeleteGame (iGameClass, iGameNumber, SYSTEM, "", SYSTEM_SHUTDOWN);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
                
            sprintf (
                pszBuffer,
                "GameEngine setup deleted game %i of gameclass %i "\
                "because it grew stale during a system shutdown",
                iGameNumber,
                iGameClass
                );

            m_pReport->WriteReport (pszBuffer);
            continue;
        }

        // If game hasn't started and is password protected and has only one empire, kill it                    
        if (!(iState & STARTED) && bPasswordProtected && iNumEmpires == 1) {
            
            iErrCode = DeleteGame (iGameClass, iGameNumber, SYSTEM, "", PASSWORD_PROTECTED);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }

            sprintf (
                pszBuffer,
                "GameEngine setup deleted game %i of gameclass %i "\
                "because it was password protected and only contained one empire",
                iGameNumber,
                iGameClass
                );
                
            m_pReport->WriteReport (pszBuffer);
            continue;
        }

        // Update the game
        iErrCode = CheckGameForUpdates (iGameClass, iGameNumber, true, &bUpdate);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }
    
Cleanup:

    if (pvGame != NULL) {
        m_pGameData->FreeData (pvGame);
    }

    if (pvEmpireKey != NULL) {
        m_pGameData->FreeData (pvEmpireKey);
    }

    return iErrCode;
}

//
// Creation
//

int GameEngine::CreateNewDatabase() {
    
    int iErrCode;

    m_pReport->WriteReport ("GameEngine setup is initializing a new database");
            
    iErrCode = CreateDefaultSystemTemplates();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup could not create the default system templates");
        return iErrCode;
    }

    iErrCode = CreateDefaultSystemTables();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup could not create the default system tables");
        return iErrCode;
    }

    iErrCode = SetupDefaultSystemTables();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup could not set up the default system tables");
        return iErrCode;
    }

    iErrCode = SetupDefaultSystemGameClasses();
    if (iErrCode != OK) {
        m_pReport->WriteReport ("GameEngine setup could not set up the default system gameclasses");
        return iErrCode;
    }
    
    m_pReport->WriteReport ("GameEngine setup finished initializing a new database");
    return iErrCode;
}


// Create default templates
int GameEngine::CreateDefaultSystemTemplates() {

    int iErrCode = m_pGameData->CreateTemplate (SystemData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemEmpireData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemGameClassData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemAlienIcons::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemSystemGameClassData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemSuperClassData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemThemes::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemActiveGames::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemEmpireMessages::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemEmpireNukeList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemNukeList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemLatestGames::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemEmpireActiveGames::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemTournaments::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemTournamentTeams::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemTournamentEmpires::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }
    
    iErrCode = m_pGameData->CreateTemplate (SystemTournamentActiveGames::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemEmpireTournaments::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameEmpires::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameDeadEmpires::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameMap::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameEmpireData::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameEmpireMessages::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameEmpireMap::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameEmpireDiplomacy::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameEmpireShips::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameEmpireFleets::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameIndependentShips::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemAlmonasterScoreTopList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemClassicScoreTopList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemBridierScoreTopList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (SystemBridierScoreEstablishedTopList::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->CreateTemplate (GameSecurity::Template);
    if (iErrCode != OK && iErrCode != ERROR_TEMPLATE_ALREADY_EXISTS) {
        Assert (false);
        return iErrCode;
    }

    return OK;
}


//
// Create the default system tables
//
int GameEngine::CreateDefaultSystemTables() {

    int iErrCode;

    // Create SystemData table
    iErrCode = m_pGameData->CreateTable (SYSTEM_DATA, SystemData::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemGameClassData table
    iErrCode = m_pGameData->CreateTable (SYSTEM_GAMECLASS_DATA, SystemGameClassData::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemSystemGameClassData table
    iErrCode = m_pGameData->CreateTable (SYSTEM_SYSTEM_GAMECLASS_DATA, SystemSystemGameClassData::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemActiveGames table
    iErrCode = m_pGameData->CreateTable (SYSTEM_ACTIVE_GAMES, SystemActiveGames::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemEmpireData table
    iErrCode = m_pGameData->CreateTable (SYSTEM_EMPIRE_DATA, SystemEmpireData::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemThemes table
    iErrCode = m_pGameData->CreateTable (SYSTEM_THEMES, SystemThemes::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemSuperClassData table
    iErrCode = m_pGameData->CreateTable (SYSTEM_SUPERCLASS_DATA, SystemSuperClassData::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemAlienIcons table
    iErrCode = m_pGameData->CreateTable (SYSTEM_ALIEN_ICONS, SystemAlienIcons::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemAlmonasterScoreTopList table
    iErrCode = m_pGameData->CreateTable (SYSTEM_ALMONASTER_SCORE_TOPLIST, SystemAlmonasterScoreTopList::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemClassicScoreTopList table
    iErrCode = m_pGameData->CreateTable (SYSTEM_CLASSIC_SCORE_TOPLIST, SystemClassicScoreTopList::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemBridierScoreTopList table
    iErrCode = m_pGameData->CreateTable (SYSTEM_BRIDIER_SCORE_TOPLIST, SystemBridierScoreTopList::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemBridierScoreEstablishedTopList table
    iErrCode = m_pGameData->CreateTable (SYSTEM_BRIDIER_SCORE_ESTABLISHED_TOPLIST, SystemBridierScoreEstablishedTopList::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemNukeList table
    iErrCode = m_pGameData->CreateTable (SYSTEM_NUKE_LIST, SystemNukeList::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemLatestGames table
    iErrCode = m_pGameData->CreateTable (SYSTEM_LATEST_GAMES, SystemLatestGames::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Create SystemTournaments table
    iErrCode = m_pGameData->CreateTable (SYSTEM_TOURNAMENTS, SystemTournaments::Template.Name);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}


// Insert default data into the system tables
int GameEngine::SetupDefaultSystemTables() {
    
    int iErrCode;
    unsigned int iKey;

    UTCTime tTime;
    Time::GetTime (&tTime);

    // Insert data into SYSTEM_DATA
    Variant pvSystemData [SystemData::NumColumns] = {
        3,              // DefaultAlien
        "Cortegana",    // ServerName
        10,             // DefaultMaxSysMessages
        10,             // DefaultMaxGameMessages
        0,              // Default buttons set
        0,              // Background
        0,              // LivePlanet
        0,              // DeadPlanet
        0,              // Separator
        NOVICE,         // Privilege (defaults to novice)
        (float) 100.0,  // AdeptScore
        20,             // MaxNumSystemMessages
        20,             // MaxNumGameMessages
        tTime,          // LastShutDownTime
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
        0,              // DefaultUIHorz
        0,              // DefaultUIVert
        0,              // DefaultUIColor
        15 * 1024,      // MaxIconSize (bytes)
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
        0,              // DefaultUIIndependentPlanet
        10,             // MaxNumUpdatesBeforeClose
        3,              // DefaultNumUpdatesBeforeClose
        (int64) Algorithm::GetRandomInteger (0x7fffffff) + 1000,    // SessionId
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
        (float) 6.0,    // ResourceAllocationRandomizationFactor
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

    iErrCode = m_pGameData->InsertRow (SYSTEM_DATA, pvSystemData);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Set up default administrator empire (root)
    iErrCode = CreateEmpire (ROOT_NAME, ROOT_DEFAULT_PASSWORD, ADMINISTRATOR, NO_KEY, true, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (iKey != ROOT_KEY) {
        Assert (false);
        return ERROR_FAILURE;
    }

    iErrCode = SetEmpireOption2 (ROOT_KEY, EMPIRE_ACCEPTED_TOS, true);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Set up default guest empire (Guest)
    iErrCode = CreateEmpire (
        GUEST_NAME,
        GUEST_DEFAULT_PASSWORD,
        GUEST,
        NO_KEY,
        true,
        &iKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (iKey != GUEST_KEY) {
        Assert (false);
        return ERROR_FAILURE;
    }

    iErrCode = SetEmpireOption (GUEST_KEY, CAN_BROADCAST, false);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = SetEmpireOption2 (GUEST_KEY, EMPIRE_ACCEPTED_TOS, true);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    ///////////////////
    // Insert themes //
    ///////////////////

    Variant pvColVal [SystemThemes::NumColumns];

    // Mensan's First Theme
    pvColVal[SystemThemes::Name] = "Mensan's First Theme";
    pvColVal[SystemThemes::AuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::Description] = "The fine new trademark Almonaster user interface";
    pvColVal[SystemThemes::FileName] = "mensan1.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS;
    pvColVal[SystemThemes::TableColor] = "882266";
    pvColVal[SystemThemes::TextColor] = "FAFA00";
    pvColVal[SystemThemes::GoodColor] = "00FF00";
    pvColVal[SystemThemes::BadColor] = "EEEEEE";
    pvColVal[SystemThemes::PrivateMessageColor] = "D3C2EA";
    pvColVal[SystemThemes::BroadcastMessageColor] = "F7EF80";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Classic theme
    pvColVal[SystemThemes::Name] = "Classic Theme";
    pvColVal[SystemThemes::AuthorName] = "Various";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "";
    pvColVal[SystemThemes::Description] = "The classic SC 2.8 user interface";
    pvColVal[SystemThemes::FileName] = "classic.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS & ~THEME_DEAD_PLANET & ~THEME_BUTTONS;
    pvColVal[SystemThemes::TableColor] = "151515";
    pvColVal[SystemThemes::TextColor] = "F0F011";
    pvColVal[SystemThemes::GoodColor] = "00FF00";
    pvColVal[SystemThemes::BadColor] = "EEEEEE";
    pvColVal[SystemThemes::PrivateMessageColor] = "25D3AC";
    pvColVal[SystemThemes::BroadcastMessageColor] = "F0F011";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    // MkII theme
    pvColVal[SystemThemes::Name] = "MkII Theme";
    pvColVal[SystemThemes::AuthorName] = "MkII Team, Aleks Sidorenko";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "";
    pvColVal[SystemThemes::Description] = "The new MkII user interface";
    pvColVal[SystemThemes::FileName] = "mkii.zip";
    pvColVal[SystemThemes::Options] = THEME_BACKGROUND | THEME_LIVE_PLANET | THEME_DEAD_PLANET;
    pvColVal[SystemThemes::TableColor] = "222266";
    pvColVal[SystemThemes::TextColor] = "F0F0F0";
    pvColVal[SystemThemes::GoodColor] = "7FFFD4";
    pvColVal[SystemThemes::BadColor] = "FF2E2E";
    pvColVal[SystemThemes::PrivateMessageColor] = "FFFF00";
    pvColVal[SystemThemes::BroadcastMessageColor] = "EEEEEE";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Mensan's first beta theme
    pvColVal[SystemThemes::Name] = "Mensan's First Beta Theme";
    pvColVal[SystemThemes::AuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::Description] = "Some beta-version graphics from the master";
    pvColVal[SystemThemes::FileName] = "mensanb1.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS & ~THEME_BUTTONS & ~THEME_HORZ & ~THEME_VERT;
    pvColVal[SystemThemes::TableColor] = "152560";
    pvColVal[SystemThemes::TextColor] = "F0F0F0";
    pvColVal[SystemThemes::GoodColor] = "F0F011";
    pvColVal[SystemThemes::BadColor] = "FBA02B";
    pvColVal[SystemThemes::PrivateMessageColor] = "COCOFF";
    pvColVal[SystemThemes::BroadcastMessageColor] = "EEEEEE";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Mensan's second beta theme
    pvColVal[SystemThemes::Name] = "Mensan's Second Beta Theme";
    pvColVal[SystemThemes::AuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::Description] = "More beta-version graphics from the master";
    pvColVal[SystemThemes::FileName] = "mensanb2.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS & ~(THEME_BUTTONS | THEME_HORZ | THEME_VERT);
    pvColVal[SystemThemes::TableColor] = "02546C";
    pvColVal[SystemThemes::TextColor] = "F0F0F0";
    pvColVal[SystemThemes::GoodColor] = "FFFF00";
    pvColVal[SystemThemes::BadColor] = "FBA02B";
    pvColVal[SystemThemes::PrivateMessageColor] = "C0C0FF";
    pvColVal[SystemThemes::BroadcastMessageColor] = "EEEEEE";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // DPR's animated theme
    pvColVal[SystemThemes::Name] = "DPR's Animated Theme";
    pvColVal[SystemThemes::AuthorName] = "Simon Gillbee";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "simon@gillbee.com";
    pvColVal[SystemThemes::Description] = "Animated graphics";
    pvColVal[SystemThemes::FileName] = "anim.zip";
    pvColVal[SystemThemes::Options] = THEME_LIVE_PLANET;
    pvColVal[SystemThemes::TableColor] = "052205";
    pvColVal[SystemThemes::TextColor] = "FFFF00";
    pvColVal[SystemThemes::GoodColor] = "00FF00";
    pvColVal[SystemThemes::BadColor] = "EEEEEE";
    pvColVal[SystemThemes::PrivateMessageColor] = "80FFFF";
    pvColVal[SystemThemes::BroadcastMessageColor] = "FFFF00";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Mensan's Techno Theme
    pvColVal[SystemThemes::Name] = "Mensan's Techno Theme";
    pvColVal[SystemThemes::AuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::Description] = "A technological theme";
    pvColVal[SystemThemes::FileName] = "techno.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS & ~THEME_BUTTONS;
    pvColVal[SystemThemes::TableColor] = "761244";
    pvColVal[SystemThemes::TextColor] = "FFFF00";
    pvColVal[SystemThemes::GoodColor] = "45F3CC";
    pvColVal[SystemThemes::BadColor] = "F7EFCE";
    pvColVal[SystemThemes::PrivateMessageColor] = "C9C9C9";
    pvColVal[SystemThemes::BroadcastMessageColor] = "EEEE00";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Mensan's Animated Theme
    pvColVal[SystemThemes::Name] = "Mensan's Animated Theme";
    pvColVal[SystemThemes::AuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::Description] = "A theme with animated graphics";
    pvColVal[SystemThemes::FileName] = "animated.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS & ~THEME_BUTTONS;
    pvColVal[SystemThemes::TableColor] = "502222";
    pvColVal[SystemThemes::TextColor] = "EEEE00";
    pvColVal[SystemThemes::GoodColor] = "FCCAA2";
    pvColVal[SystemThemes::BadColor] = "FF5555";
    pvColVal[SystemThemes::PrivateMessageColor] = "FFA936";
    pvColVal[SystemThemes::BroadcastMessageColor] = "EEEE00";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Mensan's Blues Theme
    pvColVal[SystemThemes::Name] = "Mensan's Blues Theme";
    pvColVal[SystemThemes::AuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::Description] = "A theme with spooky blue graphics";
    pvColVal[SystemThemes::FileName] = "blues.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS;
    pvColVal[SystemThemes::TableColor] = "2020B0";
    pvColVal[SystemThemes::TextColor] = "9CC6FF";
    pvColVal[SystemThemes::GoodColor] = "00FF00";
    pvColVal[SystemThemes::BadColor] = "FFFFFF";
    pvColVal[SystemThemes::PrivateMessageColor] = "25D3AC";
    pvColVal[SystemThemes::BroadcastMessageColor] = "9CC6FF";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Mensan's Dark Mood Theme
    pvColVal[SystemThemes::Name] = "Mensan's Dark Mood Theme";
    pvColVal[SystemThemes::AuthorName] = "Jens Klavsen";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "mensan@post1.tele.dk";
    pvColVal[SystemThemes::Description] = "A theme with dark, somber graphics";
    pvColVal[SystemThemes::FileName] = "darkmood.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS;
    pvColVal[SystemThemes::TableColor] = "443C3C";
    pvColVal[SystemThemes::TextColor] = "C5C5C5";
    pvColVal[SystemThemes::GoodColor] = "00D000";
    pvColVal[SystemThemes::BadColor] = "D0D000";
    pvColVal[SystemThemes::PrivateMessageColor] = "FCCAA2";
    pvColVal[SystemThemes::BroadcastMessageColor] = "D5D5D5";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Chamber Theme
    pvColVal[SystemThemes::Name] = "Chamber Theme";
    pvColVal[SystemThemes::AuthorName] = "Dynamic Design";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "dd@chamber.ee";
    pvColVal[SystemThemes::Description] = "Images from the Chamber Conflict Server";
    pvColVal[SystemThemes::FileName] = "chamber.zip";
    pvColVal[SystemThemes::Options] = THEME_BACKGROUND | THEME_SEPARATOR;
    pvColVal[SystemThemes::TableColor] = "353535";
    pvColVal[SystemThemes::TextColor] = "C0C0C0";
    pvColVal[SystemThemes::GoodColor] = "00FF00";
    pvColVal[SystemThemes::BadColor] = "FFFF00";
    pvColVal[SystemThemes::PrivateMessageColor] = "FFFF00";
    pvColVal[SystemThemes::BroadcastMessageColor] = "D0D0D0";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // NASA Theme
    pvColVal[SystemThemes::Name] = "NASA Theme";
    pvColVal[SystemThemes::AuthorName] = "Max Attar Feingold";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "maf6@cornell.edu";
    pvColVal[SystemThemes::Description] = "Images borrowed from various NASA servers";
    pvColVal[SystemThemes::FileName] = "nasa.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS & ~THEME_BUTTONS;
    pvColVal[SystemThemes::TableColor] = "570505";
    pvColVal[SystemThemes::TextColor] = "F0F000";
    pvColVal[SystemThemes::GoodColor] = "25FF25";
    pvColVal[SystemThemes::BadColor] = "F0F0F0";
    pvColVal[SystemThemes::PrivateMessageColor] = "25CF25";
    pvColVal[SystemThemes::BroadcastMessageColor] = "F09525";

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Alien Glow Theme
    pvColVal[SystemThemes::Name] = "Alien Glow Theme";
    pvColVal[SystemThemes::AuthorName] = "Denis Moreaux";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "vapula@linuxbe.org";
    pvColVal[SystemThemes::Description] = "Look is based on \"Alien Glow\" Gimp web theme. Used on Endor server";
    pvColVal[SystemThemes::FileName] = "alienglow.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS & ~THEME_BUTTONS;
    pvColVal[SystemThemes::TableColor] = "001000";              // Dark green, almost black
    pvColVal[SystemThemes::TextColor] = "00C000";               // Green
    pvColVal[SystemThemes::GoodColor] = "00FF00";               // Bright green
    pvColVal[SystemThemes::BadColor] = "4040ff";                // Ghostly blue
    pvColVal[SystemThemes::PrivateMessageColor] = "FFFF00";     // Bright yellow
    pvColVal[SystemThemes::BroadcastMessageColor] = "FFFFCC";   // Dull yellow

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Iceberg Theme
    pvColVal[SystemThemes::Name] = "Iceberg Theme";
    pvColVal[SystemThemes::AuthorName] = "Aleksandr Sidorenko";
    pvColVal[SystemThemes::Version] = "1.0";
    pvColVal[SystemThemes::AuthorEmail] = "aleksandr@videotron.ca";
    pvColVal[SystemThemes::Description] = "Look inspired by Alexia's Iceberg server";
    pvColVal[SystemThemes::FileName] = "iceberg.zip";
    pvColVal[SystemThemes::Options] = ALL_THEME_OPTIONS & ~(THEME_BUTTONS | THEME_HORZ | THEME_VERT);
    pvColVal[SystemThemes::TableColor] = "101020";              // Dark blue, almost black
    pvColVal[SystemThemes::TextColor] = "90A0CC";               // Light blue
    pvColVal[SystemThemes::GoodColor] = "00DD00";               // Sharp green
    pvColVal[SystemThemes::BadColor] = "E80700";                // Sharp red
    pvColVal[SystemThemes::PrivateMessageColor] = "9090FF";     // Brigher light blue
    pvColVal[SystemThemes::BroadcastMessageColor] = "90A0CC";   // Light blue

    iErrCode = CreateTheme (pvColVal, &iKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}

// Create default superclasses and gameclasses
int GameEngine::SetupDefaultSystemGameClasses() {

    // Add SuperClasses
    int i, iErrCode, iBeginnerKey, iGrudgeKey, iLongTermKey, iBlitzKey, iGameClass;
    
    iErrCode = CreateSuperClass ("Beginner Games", &iBeginnerKey);
    if (iErrCode != OK && iErrCode != ERROR_SUPERCLASS_ALREADY_EXISTS) {
        return iErrCode;
    }

    iErrCode = CreateSuperClass ("Grudge Matches", &iGrudgeKey);
    if (iErrCode != OK && iErrCode != ERROR_SUPERCLASS_ALREADY_EXISTS) {
        return iErrCode;
    }

    iErrCode = CreateSuperClass ("Blitzes", &iBlitzKey);
    if (iErrCode != OK && iErrCode != ERROR_SUPERCLASS_ALREADY_EXISTS) {
        return iErrCode;
    }

    iErrCode = CreateSuperClass ("Longterm Games", &iLongTermKey);
    if (iErrCode != OK && iErrCode != ERROR_SUPERCLASS_ALREADY_EXISTS) {
        return iErrCode;
    }

    Variant pvSubmitArray [SystemGameClassData::NumColumns];

    // Beginner Tekno Blood
    pvSubmitArray[SystemGameClassData::Name] = "Beginner Tekno Blood";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 10;
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 3.5;
    pvSubmitArray[SystemGameClassData::OpenGameNum] = 1;                            
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 210;
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | ONLY_SURRENDER_WITH_TWO_EMPIRES;
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;             
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBeginnerKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;   
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 4;                          
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;
    
    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    // Apprentice Blitz
    pvSubmitArray[SystemGameClassData::Name] = "Apprentice Blitz";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 16;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 3;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 1.5;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 240;
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | EXPOSED_DIPLOMACY |
        VISIBLE_BUILDS | 
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                    
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 4;                          
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | TRUCE | TRADE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;                     
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBeginnerKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 3;                          
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;                           
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    // Assassin Grudge
    pvSubmitArray[SystemGameClassData::Name] = "Assassin Grudge";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 2;                          
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 10;                         
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 1.5;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::Options] = 
        VISIBLE_DIPLOMACY | EXPOSED_DIPLOMACY | VISIBLE_BUILDS | ALLOW_DRAW;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 25;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 25;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 25;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 35;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 35;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 35;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 90;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 90;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 90;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 110;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 110;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 110;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | SURRENDER;       
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM; 
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iGrudgeKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 8;                                          
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = TECH_SCIENCE | TECH_COLONY | TECH_SATELLITE;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    // Darkstar Battle
    pvSubmitArray[SystemGameClassData::Name] = "Darkstar Battle";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 10;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 5;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 4.0;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 150;                          
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.5;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 3;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 35;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 35;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 35;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 45;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 45;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 45;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 110;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 110;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 110;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 130;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 130;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 130;                    
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | TRUCE | TRADE | ALLIANCE | SURRENDER;            
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = 2;
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 5;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_MINESWEEPER | TECH_MINEFIELD;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;


    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    
    // Tiberia Series
    pvSubmitArray[SystemGameClassData::Name] = "Tiberia Series";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 10;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 6;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 2.0;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 180;
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | VISIBLE_BUILDS |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 2.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 3;                          
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 35;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 35;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 35;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | TRADE | SURRENDER;                   
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;                           
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_TROOPSHIP;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    
    // Galaxy of Andromeda
    pvSubmitArray[SystemGameClassData::Name] = "Galaxy of Andromeda";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 20;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 12;                         
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 1.5;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::Options] = 
        VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | VISIBLE_BUILDS |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 5;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 35;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 35;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 35;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 110;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 110;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 110;        
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | TRUCE | TRADE | ALLIANCE | SURRENDER;    
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iLongTermKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = 2;
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 10;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    
    // Natural Selection Universe
    pvSubmitArray[SystemGameClassData::Name] = "Natural Selection Universe";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 16;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 10;                         
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 2.0;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::Options] = 
        VISIBLE_DIPLOMACY | 
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 2.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 4;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 23;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 23;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 23;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 90;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 90;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 90;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 105;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 105;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 105;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iLongTermKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 10;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_DOOMSDAY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Texan KnifeFight
    pvSubmitArray[SystemGameClassData::Name] = "Texan KnifeFight";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 6;                          
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 3;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 2.0;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 180;
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 3;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;                           
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_TERRAFORMER;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    
    // 21st Century Blood   
    pvSubmitArray[SystemGameClassData::Name] = "21st Century Blood";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 16;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 9;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 4.0;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::Options] = 
        VISIBLE_DIPLOMACY | VISIBLE_BUILDS | ONLY_SURRENDER_WITH_TWO_EMPIRES | ALLOW_DRAW;

    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 2.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 8;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 32;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 32;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 32;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 36;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 36;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 36;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 105;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 105;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 105;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 115;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 115;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 115;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iLongTermKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 7;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_CLOAKER;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    // Nanotech World
    pvSubmitArray[SystemGameClassData::Name] = "Nanotech World";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 10;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 10;                         
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 4.25;              
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::Options] = 
        VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | VISIBLE_BUILDS |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.5;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 2;                          
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | TRUCE | TRADE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iLongTermKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = 3;
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 10;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_SCIENCE | TECH_COLONY | TECH_ENGINEER | TECH_CLOAKER | TECH_DOOMSDAY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    

    // Land of Bounty
    pvSubmitArray[SystemGameClassData::Name] = "Land of Bounty";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 10;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 6;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 1.5;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 180;                          
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | EXPOSED_DIPLOMACY |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 60;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 60;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 60;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 70;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 70;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 70;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 166;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 166;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 166;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 200;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 200;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 200;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | TRUCE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_TERRAFORMER;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    

    // Perfect Information Battle
    pvSubmitArray[SystemGameClassData::Name] = "Perfect Information Battle";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 2;                              
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 7;                              
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 1.5;                       
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;                 
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | EXPOSED_MAP | EXPOSED_DIPLOMACY | VISIBLE_BUILDS | ALLOW_DRAW;
                
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 2;                              
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iGrudgeKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 7;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = TECH_COLONY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = TECH_COLONY;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    // Caveman Deathmatch
    pvSubmitArray[SystemGameClassData::Name] = "Caveman Deathmatch";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 2;                          
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 5;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 0.8;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | ALLOW_DRAW;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 33;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 33;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 33;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 40;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 40;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 40;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 80;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 80;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 80;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 90;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 90;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 90;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iGrudgeKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 5;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
        
    
    // Low Orbit SuperBlitz
    pvSubmitArray[SystemGameClassData::Name] = "Low Orbit SuperBlitz";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 6;                          
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 4;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 2.0;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 90;                           
    pvSubmitArray[SystemGameClassData::Options] = 
        WEEKEND_UPDATES | VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | VISIBLE_BUILDS |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 4;                          
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | TRADE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBlitzKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;       
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 4;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    // Kindergarten Universe
    pvSubmitArray[SystemGameClassData::Name] = "Kindergarten Universe";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 12;                         
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 8;                          
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 1.5;                   
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::Options] = 
        VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | EXPOSED_DIPLOMACY |
        ONLY_SURRENDER_WITH_TWO_EMPIRES;
                        
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.5;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 8;                          
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 33;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 33;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 33;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 33;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 33;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 33;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 100;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 100;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | TRUCE | ALLIANCE | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;                          
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBeginnerKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = 2;
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 8;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = 
        TECH_ATTACK | TECH_SCIENCE | TECH_COLONY | TECH_TROOPSHIP;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    // Guns'n Troopships
    pvSubmitArray[SystemGameClassData::Name] = "Guns'n Troopships";
    pvSubmitArray[SystemGameClassData::MaxNumEmpires] = 2;                          
    pvSubmitArray[SystemGameClassData::MaxNumPlanets] = 12;                         
    pvSubmitArray[SystemGameClassData::MaxTechDev] = (float) 1.25;              
    pvSubmitArray[SystemGameClassData::NumSecPerUpdate] = 60 * 60 * 24;             
    pvSubmitArray[SystemGameClassData::Options] = 
        VISIBLE_DIPLOMACY | PRIVATE_MESSAGES | ONLY_SURRENDER_WITH_TWO_EMPIRES;
                    
    pvSubmitArray[SystemGameClassData::InitialTechLevel] = (float) 1.0;                 
    pvSubmitArray[SystemGameClassData::MinNumEmpires] = 2;
    pvSubmitArray[SystemGameClassData::MinAvgAg] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgMin] = 30;
    pvSubmitArray[SystemGameClassData::MinAvgFuel] = 30;
    pvSubmitArray[SystemGameClassData::MaxAvgAg] = 40;
    pvSubmitArray[SystemGameClassData::MaxAvgMin] = 40;
    pvSubmitArray[SystemGameClassData::MaxAvgFuel] = 40;
    pvSubmitArray[SystemGameClassData::MinAgHW] = 90;
    pvSubmitArray[SystemGameClassData::MinMinHW] = 90;
    pvSubmitArray[SystemGameClassData::MinFuelHW] = 90;
    pvSubmitArray[SystemGameClassData::MaxAgHW] = 110;
    pvSubmitArray[SystemGameClassData::MaxMinHW] = 110;
    pvSubmitArray[SystemGameClassData::MaxFuelHW] = 110;
    pvSubmitArray[SystemGameClassData::DiplomacyLevel] = WAR | SURRENDER;
    pvSubmitArray[SystemGameClassData::MapsShared] = NO_DIPLOMACY;              
    pvSubmitArray[SystemGameClassData::Owner] = SYSTEM;
    pvSubmitArray[SystemGameClassData::SuperClassKey] = iBeginnerKey;
    pvSubmitArray[SystemGameClassData::MaxNumTruces] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumTrades] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MaxNumAlliances] = UNRESTRICTED_DIPLOMACY;
    pvSubmitArray[SystemGameClassData::MinNumPlanets] = 9;
    pvSubmitArray[SystemGameClassData::BuilderPopLevel] = 50;                           
    pvSubmitArray[SystemGameClassData::InitialTechDevs] = TECH_SCIENCE | TECH_COLONY;
    pvSubmitArray[SystemGameClassData::DevelopableTechDevs] = ALL_CLASSIC_TECHS;

    // TODO - description
    pvSubmitArray[SystemGameClassData::Description] = "";
    pvSubmitArray[SystemGameClassData::MaxAgRatio] = MAX_RATIO;
    pvSubmitArray[SystemGameClassData::MaxNumActiveGames] = INFINITE_ACTIVE_GAMES;
    pvSubmitArray[SystemGameClassData::NumActiveGames] = 0;
    pvSubmitArray[SystemGameClassData::NumUpdatesForIdle] = 2;
    pvSubmitArray[SystemGameClassData::NumUpdatesForRuin] = 10;
    pvSubmitArray[SystemGameClassData::RuinFlags] = RUIN_ALMONASTER;
    pvSubmitArray[SystemGameClassData::OwnerName] = "";
    pvSubmitArray[SystemGameClassData::TournamentKey] = NO_KEY;
    pvSubmitArray[SystemGameClassData::NumInitialTechDevs] = 1;

    iErrCode = CreateGameClass (SYSTEM, pvSubmitArray, &iGameClass);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }


    //////////////////////
    // SystemAlienIcons //
    //////////////////////
    
    // 1 to 42
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Ken Eppstein";
    for (i = 1; i <= 42; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 43 to 82
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Ronald Kinion";
    for (i = 43; i <= 82; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 83
    pvSubmitArray[SystemAlienIcons::AlienKey] = 83;
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Unknown";
    iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // 84 to 89
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Ronald Kinion";
    for (i = 84; i <= 89; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 90
    pvSubmitArray[SystemAlienIcons::AlienKey] = 90;
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Jens Klavsen";
    iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // 91
    pvSubmitArray[SystemAlienIcons::AlienKey] = 91;
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Chris John";
    iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // 92 to 101
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Jens Klavsen";
    for (i = 92; i <= 101; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 102 to 103
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Ronald Kinion";
    for (i = 102; i <= 103; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 104 to 105
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Chris John";
    for (i = 104; i <= 105; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 106 to 125
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Ronald Kinion";
    for (i = 106; i <= 125; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 126 to 127
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Chris John";
    for (i = 126; i <= 127; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 128
    pvSubmitArray[SystemAlienIcons::AlienKey] = 128;
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Haavard Fledsberg";
    iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // 129 to 135
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Chris John";
    for (i = 129; i <= 135; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 136 to 145
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Jens Klavsen";
    for (i = 136; i <= 145; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }
    
    // 146
    pvSubmitArray[SystemAlienIcons::AlienKey] = 146;
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Michel Lemieux";
    iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    // 147 to 152
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Unknown";
    for (i = 147; i <= 152; i ++) {
        pvSubmitArray[SystemAlienIcons::AlienKey] = i;
        iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
    }

    // 153
    pvSubmitArray[SystemAlienIcons::AlienKey] = 153;
    pvSubmitArray[SystemAlienIcons::AuthorName] = "Kia";
    iErrCode = m_pGameData->InsertRow (SYSTEM_ALIEN_ICONS, pvSubmitArray);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}

int GameEngine::VerifyTournaments() {

    int iErrCode;
    unsigned int iKey = NO_KEY, iNumGames, iOwner;

    bool bExists;

    while (true) {

        iErrCode = m_pGameData->GetNextKey (SYSTEM_TOURNAMENTS, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = OK;
            break;
        }
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        iErrCode = GetTournamentOwner (iKey, &iOwner);
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }

        // Check for tournament that needs to be deleted
        if (iOwner == DELETED_EMPIRE_KEY) {

            iErrCode = GetTournamentGames (iKey, NULL, NULL, &iNumGames);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            if (iNumGames == 0) {

                iErrCode = DeleteTournament (DELETED_EMPIRE_KEY, iKey, false);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
            }
        }

        // Check for tournament without empire
        else if (iOwner != SYSTEM) {

            iErrCode = DoesEmpireExist (iOwner, &bExists, NULL);
            if (iErrCode != OK) {
                Assert (false);
                return iErrCode;
            }

            if (!bExists) {

                iErrCode = DeleteTournament (iOwner, iKey, true);
                if (iErrCode != OK && iErrCode != ERROR_TOURNAMENT_HAS_GAMES) {
                    Assert (false);
                    return iErrCode;
                }
            }
        }
    }
    
    return OK;
}

int GameEngine::VerifyTopLists() {

    int i, iErrCode;

    bool bRebuild = m_scConfig.bRebuildTopListsOnStartup;
    
    ENUMERATE_SCORING_SYSTEMS (i) {
        
        ScoringSystem ssTopList = (ScoringSystem) i;
        if (HasTopList (ssTopList)) {

            if (!bRebuild) {
                
                iErrCode = VerifyTopList (ssTopList);
                if (iErrCode != OK) {
                    bRebuild = true;
                }
            }
            
            if (bRebuild) {
                
                iErrCode = RebuildTopList (ssTopList);
                if (iErrCode != OK) {
                    Assert (false);
                    return iErrCode;
                }
            }
        }
    }

    return OK;
}

int GameEngine::RebuildTopLists() {

    int i, iErrCode = OK;

    ENUMERATE_SCORING_SYSTEMS (i) {

        ScoringSystem ssTopList = (ScoringSystem) i;
        if (HasTopList (ssTopList)) {

            iErrCode = RebuildTopList (ssTopList);
            if (iErrCode != OK) break;
        }
    }

    return iErrCode;
}

int GameEngine::RebuildTopList (ScoringSystem ssTopList) {
    
    int iErrCode;
    const char* pszTableName = TOPLIST_TABLE_NAME[ssTopList];
    
    iErrCode = m_pGameData->DeleteAllRows (pszTableName);
    if (iErrCode == OK) {
        iErrCode = InitializeEmptyTopList (ssTopList);
        Assert (iErrCode == OK);
    }

    else Assert (false);

    return iErrCode;
}
