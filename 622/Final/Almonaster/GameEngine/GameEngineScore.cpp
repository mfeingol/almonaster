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

// Input:
// iNukerKey -> Nuking empire
// iNukedKey -> Nuked empire
// iGameClass -> GameClass key
// iGameNumber -> Game number
//
// Update Scores when a nuke occurs
int GameEngine::UpdateScoresOnNuke (int iNukerKey, int iNukedKey, const char* pszNukerName, 
                                    const char* pszNukedName, int iGameClass, int iGameNumber,
                                    int iUpdate, ReasonForRemoval reason, const char* pszGameClassName) {

    int iErrCode;

    // Report
    if (m_scConfig.bReport) {

        char pszMessage [MAX_EMPIRE_NAME_LENGTH * 2 + MAX_FULL_GAME_CLASS_NAME_LENGTH + 128];
        sprintf (
            pszMessage, 
            "%s %s %s in %s %i", 
            pszNukerName, 
            reason == EMPIRE_SURRENDERED ? "accepted surrender from" : "nuked",
            pszNukedName, 
            pszGameClassName, 
            iGameNumber
            );

        m_pReport->WriteReport (pszMessage);
    }

    // Increment nukes and nuked
    iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iNukerKey, SystemEmpireData::Nukes, 1);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iNukedKey, SystemEmpireData::Nuked, 1);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Update nuker and nuked lists
    int iNukedAlienKey, iNukerAlienKey;
    Variant vValue, vSneer;

    if (m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iNukedKey, SystemEmpireData::AlienKey, &vValue) == OK) {
        iNukedAlienKey = vValue.GetInteger();
    } else {
        iNukedAlienKey = NO_KEY;
    }

    iErrCode = AddNukeToHistory (
        NUKED_LIST,
        pszGameClassName,
        iGameNumber,
        iNukerKey,
        NULL,
        NO_KEY,
        iNukedKey,
        pszNukedName,
        iNukedAlienKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iNukerKey, SystemEmpireData::AlienKey, &vValue) == OK) {
        iNukerAlienKey = vValue.GetInteger();
    } else {
        iNukerAlienKey = NO_KEY;
    }

    iErrCode = AddNukeToHistory (
        NUKER_LIST,
        pszGameClassName,
        iGameNumber,
        iNukedKey,
        NULL,
        NO_KEY,
        iNukerKey,
        pszNukerName,
        iNukerAlienKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Update system nuke list
    iErrCode = AddNukeToHistory (
        SYSTEM_LIST,
        pszGameClassName,
        iGameNumber,
        iNukerKey,
        pszNukerName,
        iNukerAlienKey,
        iNukedKey,
        pszNukedName,
        iNukedAlienKey
        );

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Update scores
    ScoringChanges scChanges;
    scChanges.pszNukedName = pszNukedName;

    if (reason == EMPIRE_SURRENDERED) {
        iErrCode = UpdateScoresOnSurrender (iGameClass, iGameNumber, iNukerKey, iNukedKey, &scChanges);
    } else {
        iErrCode = UpdateScoresOnNuke (iGameClass, iGameNumber, iNukerKey, iNukedKey, &scChanges);
    }

    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Send messages
    iErrCode = SendScoringChangeMessages (iGameClass, iGameNumber, iNukerKey, iNukedKey, iUpdate, reason, pszGameClassName, &scChanges);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Send victory sneer
    iErrCode = SendVictorySneer (iNukerKey, pszNukerName, iNukedKey);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}

int GameEngine::UpdateScoresOnSurrender (int iNukerKey, int iNukedKey, const char* pszNukerName, 
                                         const char* pszNukedName, int iGameClass, int iGameNumber,
                                         int iUpdate, const char* pszGameClassName) {

    return UpdateScoresOnNuke (
        iNukerKey, 
        iNukedKey, 
        pszNukerName, 
        pszNukedName, 
        iGameClass, 
        iGameNumber,
        iUpdate,
        EMPIRE_SURRENDERED,
        pszGameClassName
        );
}

int GameEngine::UpdateScoresOn30StyleSurrender (int iLoserKey, const char* pszLoserName, int iGameClass, 
                                                int iGameNumber, int iUpdate, const char* pszGameClassName) {

    int iErrCode;

    // Report
    if (m_scConfig.bReport) {

        char pszMessage [MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + 64];
        sprintf (pszMessage, "%s surrendered out of %s %i", pszLoserName, pszGameClassName, iGameNumber);

        m_pReport->WriteReport (pszMessage);
    }

    // Increment nuked for loser
    iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iLoserKey, SystemEmpireData::Nuked, 1);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    // Ignore scoring changes for this event
    ScoringChanges scChanges;
    iErrCode = UpdateScoresOn30StyleSurrender (iGameClass, iGameNumber, iLoserKey, &scChanges);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return iErrCode;
}

int GameEngine::UpdateScoresOn30StyleSurrenderColonization (int iWinnerKey, int iPlanetKey, 
                                                            const char* pszWinnerName, int iGameClass, 
                                                            int iGameNumber, int iUpdate,
                                                            const char* pszGameClassName) {

    int iErrCode, iLoserKey;

    GAME_MAP (strGameMap, iGameClass, iGameNumber);

    Variant vPlanetName, vEmpireSecretKey, vNukedKey, vTemp;
    char pszNukedName [MAX_EMPIRE_NAME_LENGTH + 1] = "";

    bool bEmpireLocked = false, bValid;
    NamedMutex nmEmpireMutex;

    iErrCode = m_pGameData->ReadData (strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Report
    if (m_scConfig.bReport) {
        
        char pszMessage [MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + MAX_PLANET_NAME_LENGTH + 64];
        
        sprintf (
            pszMessage,
            "%s colonized %s in %s %i",
            pszWinnerName,
            vPlanetName.GetCharPtr(),
            pszGameClassName,
            iGameNumber
            );

        m_pReport->WriteReport (pszMessage);
    }

    // Give empire credit for nuke
    iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iWinnerKey, SystemEmpireData::Nukes, 1);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (
        strGameMap, 
        iPlanetKey, 
        GameMap::SurrenderEmpireSecretKey, 
        &vEmpireSecretKey
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = m_pGameData->ReadData (
        strGameMap, 
        iPlanetKey, 
        GameMap::HomeWorld, 
        &vNukedKey
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    Assert (vNukedKey.GetInteger() >= ROOT_KEY);

    iErrCode = LockEmpire (vNukedKey.GetInteger(), &nmEmpireMutex);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    bEmpireLocked = true;

    iErrCode = CheckSecretKey (vNukedKey.GetInteger(), vEmpireSecretKey.GetInteger64(), &bValid, NULL, NULL);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (bValid) {
        iLoserKey = vNukedKey.GetInteger();
    } else {

        UnlockEmpire (nmEmpireMutex);
        bEmpireLocked = false;

        iLoserKey = NO_KEY;
    }

    // Parse out empire's name
    int scanf = sscanf (vPlanetName.GetCharPtr(), RUINS_OF, pszNukedName);
    Assert (scanf == 1);

    int iNukerAlienKey, iNukedAlienKey;

    // Get nuker alien key
    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iWinnerKey, SystemEmpireData::AlienKey, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    iNukerAlienKey = vTemp.GetInteger();

    // Get nuked alien key
    if (iLoserKey == NO_KEY) {

        iNukedAlienKey = NO_KEY;

    } else {

        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iLoserKey, SystemEmpireData::AlienKey, &vTemp);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        iNukedAlienKey = vTemp.GetInteger();

        // Add to nuked's nuke history
        iErrCode = AddNukeToHistory (
            NUKER_LIST,
            pszGameClassName,
            iGameNumber,
            iLoserKey,
            NULL,
            NO_KEY,
            iWinnerKey,
            pszWinnerName,
            iNukerAlienKey
            );

        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        // Send the loser a victory sneer
        iErrCode = SendVictorySneer (iWinnerKey, pszWinnerName, iLoserKey);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Add to nuker's nuke history
    iErrCode = AddNukeToHistory (
        NUKED_LIST,
        pszGameClassName,
        iGameNumber,
        iWinnerKey,
        NULL,
        NO_KEY,
        iLoserKey,
        pszNukedName,
        iNukedAlienKey
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Update system nuke list
    iErrCode = AddNukeToHistory (
        SYSTEM_LIST,
        pszGameClassName,
        iGameNumber,
        iWinnerKey,
        pszWinnerName,
        iNukerAlienKey,
        iLoserKey,
        pszNukedName,
        iNukedAlienKey
        );

    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (bEmpireLocked) {
        UnlockEmpire (nmEmpireMutex);
        bEmpireLocked = false;
    }

    {   // Scope
        ScoringChanges scChanges;
        scChanges.pszNukedName = pszNukedName;

        iErrCode = UpdateScoresOn30StyleSurrenderColonization (iGameClass, iGameNumber, iWinnerKey, iPlanetKey, &scChanges);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = SendScoringChangeMessages (iGameClass, iGameNumber, iWinnerKey, iLoserKey, iUpdate, EMPIRE_COLONIZED_RUINS, pszGameClassName, &scChanges);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

Cleanup:

    if (bEmpireLocked) {
        UnlockEmpire (nmEmpireMutex);
    }

    return iErrCode;
}


int GameEngine::UpdateScoresOnGameEnd (int iGameClass, int iGameNumber) {

    int i, iErrCode;

    ENUMERATE_SCORING_SYSTEMS(i) {
        iErrCode = m_ppScoringSystem[i]->OnGameEnd (iGameClass, iGameNumber);
        Assert (iErrCode == OK);
    }

    return OK;
}

int GameEngine::UpdateScoresOnNuke (int iGameClass, int iGameNumber, int iNukerKey, int iNukedKey, ScoringChanges* pscChanges) {

    int i, iErrCode;

    ENUMERATE_SCORING_SYSTEMS(i) {
        iErrCode = m_ppScoringSystem[i]->OnNuke (iGameClass, iGameNumber, iNukerKey, iNukedKey, pscChanges);
        Assert (iErrCode == OK);
    }

    return OK;
}

int GameEngine::UpdateScoresOnSurrender (int iGameClass, int iGameNumber, int iWinnerKey, int iLoserKey, ScoringChanges* pscChanges) {
    
    int i, iErrCode;

    ENUMERATE_SCORING_SYSTEMS(i) {
        iErrCode = m_ppScoringSystem[i]->OnSurrender (iGameClass, iGameNumber, iWinnerKey, iLoserKey, pscChanges);
        Assert (iErrCode == OK);
    }

    return OK;
}

int GameEngine::UpdateScoresOn30StyleSurrender (int iGameClass, int iGameNumber, int iLoserKey, ScoringChanges* pscChanges) {
    
    int i, iErrCode;

    ENUMERATE_SCORING_SYSTEMS(i) {
        iErrCode = m_ppScoringSystem[i]->On30StyleSurrender (iGameClass, iGameNumber, iLoserKey, pscChanges);
        Assert (iErrCode == OK);
    }

    return OK;
}

int GameEngine::UpdateScoresOn30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey,
                                                            int iPlanetKey, ScoringChanges* pscChanges) {
    
    int i, iErrCode;

    ENUMERATE_SCORING_SYSTEMS(i) {
        iErrCode = m_ppScoringSystem[i]->On30StyleSurrenderColonization (
            iGameClass, 
            iGameNumber, 
            iWinnerKey, 
            iPlanetKey,
            pscChanges
            );
        Assert (iErrCode == OK);
    }

    return OK;
}

int GameEngine::UpdateScoresOnWin (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode, i;
    
    // Add win to empires' statistics
    iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Wins, 1);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    ENUMERATE_SCORING_SYSTEMS(i) {
        iErrCode = m_ppScoringSystem[i]->OnWin (iGameClass, iGameNumber, iEmpireKey);
        Assert (iErrCode == OK);
    }

    return OK;
}

int GameEngine::UpdateScoresOnDraw (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode, i;

    // Add draw to empires' statistics
    iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Draws, 1);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    ENUMERATE_SCORING_SYSTEMS(i) {
        iErrCode = m_ppScoringSystem[i]->OnDraw (iGameClass, iGameNumber, iEmpireKey);
        Assert (iErrCode == OK);
    }

    return OK;
}


int GameEngine::UpdateScoresOnRuin (int iGameClass, int iGameNumber, int iEmpireKey) {

    int iErrCode, i;

    // Add ruin to empires' statistics
    iErrCode = m_pGameData->Increment (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Ruins, 1);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    ENUMERATE_SCORING_SYSTEMS(i) {
        iErrCode = m_ppScoringSystem[i]->OnRuin (iGameClass, iGameNumber, iEmpireKey);
        Assert (iErrCode == OK);
    }

    return OK;

}

// Input:
// iEmpireKey -> Empire key
//
// Output:
// *piNumNukes -> Number of empire nukes in history
// *piNumNuked -> Number of empires nuked in history
//
// Return the size of the empire's nuke history

int GameEngine::GetNumEmpiresInNukeHistory (int iEmpireKey, int* piNumNukes, int* piNumNuked) {

    int iErrCode = OK;

    SYSTEM_EMPIRE_NUKER_LIST (strTable, iEmpireKey);

    if (!m_pGameData->DoesTableExist (strTable)) {
        *piNumNukes = 0;
    } else {
        iErrCode = m_pGameData->GetNumRows (strTable, (unsigned int*) piNumNukes);
        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    GET_SYSTEM_EMPIRE_NUKED_LIST (strTable, iEmpireKey);

    if (!m_pGameData->DoesTableExist (strTable)) {
        *piNumNuked = 0;
    } else {

        iErrCode = m_pGameData->GetNumRows (strTable, (unsigned int*) piNumNuked);
        if (iErrCode != OK) {
            return iErrCode;
        }
    }

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
//
// Output
//
// Return an empire's nuke history, unsorted

int GameEngine::GetNukeHistory (int iEmpireKey, int* piNumNuked, Variant*** pppvNukedData, int* piNumNukers, 
                                Variant*** pppvNukerData) {

    int iErrCode;

    SYSTEM_EMPIRE_NUKER_LIST (strNuker, iEmpireKey);
    SYSTEM_EMPIRE_NUKED_LIST (strNuked, iEmpireKey);

    const unsigned int piNukeColumns[] = {
        SystemEmpireNukeList::AlienKey,
        SystemEmpireNukeList::EmpireName,
        SystemEmpireNukeList::EmpireKey,
        SystemEmpireNukeList::GameClassName,
        SystemEmpireNukeList::GameNumber,
        SystemEmpireNukeList::TimeStamp
    };

    const unsigned int iNumColumns = sizeof (piNukeColumns) / sizeof (unsigned int);

    *piNumNuked = *piNumNukers = 0;
    *pppvNukedData = *pppvNukerData = NULL;

    // Get Nuked data
    iErrCode = m_pGameData->ReadColumns (
        strNuked, 
        iNumColumns,
        piNukeColumns,
        pppvNukedData,
        (unsigned int*) piNumNuked
        );

    if (iErrCode != OK && iErrCode != ERROR_UNKNOWN_TABLE_NAME && iErrCode != ERROR_DATA_NOT_FOUND) {
        return iErrCode;
    }

    // Get Nuker data
    iErrCode = m_pGameData->ReadColumns (
        strNuker, 
        iNumColumns,
        piNukeColumns,
        pppvNukerData,
        (unsigned int*) piNumNukers
        );

    if (iErrCode != OK && iErrCode != ERROR_UNKNOWN_TABLE_NAME && iErrCode != ERROR_DATA_NOT_FOUND) {

        if (*pppvNukedData != NULL) {
            m_pGameData->FreeData (*pppvNukedData);
            *pppvNukedData = NULL;
        }
        return iErrCode;
    }

    return OK;
}

// Output
//
// Return the system's nuke history, unsorted

int GameEngine::GetSystemNukeHistory (int* piNumNukes, Variant*** pppvNukedData) {

    const unsigned int piNukeColumns[] = {
        SystemNukeList::NukerAlienKey,
        SystemNukeList::NukerEmpireName,
        SystemNukeList::NukerEmpireKey,
        SystemNukeList::NukedAlienKey,
        SystemNukeList::NukedEmpireName,
        SystemNukeList::NukedEmpireKey,
        SystemNukeList::GameClassName,
        SystemNukeList::GameNumber,
        SystemNukeList::TimeStamp
    };

    const unsigned int iNumColumns = sizeof (piNukeColumns) / sizeof (unsigned int);

    int iErrCode = m_pGameData->ReadColumns (
        SYSTEM_NUKE_LIST, 
        iNumColumns,
        piNukeColumns,
        pppvNukedData,
        (unsigned int*) piNumNukes
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


int GameEngine::GetSystemLatestGames (int* piNumGames, Variant*** pppvGameData) {

    const unsigned int piColumns[] = {
        SystemLatestGames::Name,
        SystemLatestGames::Number,
        SystemLatestGames::Created,
        SystemLatestGames::Ended,
        SystemLatestGames::Updates,
        SystemLatestGames::Result,
        SystemLatestGames::Winners,
        SystemLatestGames::Losers,
    };

    const unsigned int iNumColumns = sizeof (piColumns) / sizeof (unsigned int);

    int iErrCode = m_pGameData->ReadColumns (
        SYSTEM_LATEST_GAMES,
        iNumColumns,
        piColumns,
        pppvGameData,
        (unsigned int*) piNumGames
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }

    return iErrCode;
}


int GameEngine::CalculatePrivilegeLevel (int iEmpireKey) {

    int iErrCode, iSystemOptions, iEmpireOptions2;
    Variant vAdeptLevel, vApprenticeLevel, vScore, vPrivilege;

    // Admins stay the same
    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, &vPrivilege);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (vPrivilege.GetInteger() == ADMINISTRATOR) {
        return OK;
    }

    // See if the system allows it
    iErrCode = GetSystemOptions (&iSystemOptions);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (iSystemOptions & DISABLE_PRIVILEGE_SCORE_ELEVATION) {
        return OK;
    }

    // See if the empire's settings allow it
    iErrCode = GetEmpireOptions2 (iEmpireKey, &iEmpireOptions2);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (iEmpireOptions2 & ADMINISTRATOR_FIXED_PRIVILEGE) {
        return OK;
    }

    // Read the data we need
    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::AdeptScore, &vAdeptLevel);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::ApprenticeScore, &vApprenticeLevel);
    if (iErrCode != OK) {
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::AlmonasterScore, &vScore);
    if (iErrCode != OK) {
        return iErrCode;
    }

    // Check for attainment of adepthood
    if (vPrivilege.GetInteger() == NOVICE) {
        
        if (vScore < vAdeptLevel && vScore >= vApprenticeLevel) {
            
            // Advance to apprentice
            iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, APPRENTICE);
            if (iErrCode != OK) {
                return iErrCode;
            }
            
            SendSystemMessage (
                iEmpireKey, 
                "Congratulations! You have attained the level of Apprentice and can now create your own personal games",
                SYSTEM,
                MESSAGE_SYSTEM
            );
        }
        
        else if (vScore >= vAdeptLevel) {
            
            // Advance to adept
            iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, ADEPT);         
            if (iErrCode != OK) {
                return iErrCode;
            }

            SendSystemMessage (
                iEmpireKey, 
                "Congratulations! You have attained the level of Adept, "\
                "so you can now create your own personal gameclasses",
                SYSTEM,
                MESSAGE_SYSTEM
                );
        }
    }
    
    // Check for loss of privilege
    else if (vPrivilege.GetInteger() == APPRENTICE) {
        
        if (vScore < vApprenticeLevel) {
            
            // Demote to novice
            iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, NOVICE);
            if (iErrCode != OK) {
                return iErrCode;
            }
            
            SendSystemMessage (
                iEmpireKey, 
                "Unfortunately, you have lost your Apprentice status, "\
                "so you can no longer create your own personal games",
                SYSTEM,
                MESSAGE_SYSTEM
                );
        }
        
        else if (vScore >= vAdeptLevel) {
            
            // Advance to adept
            iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, ADEPT);         
            if (iErrCode != OK) {
                return iErrCode;
            }

            SendSystemMessage (
                iEmpireKey, 
                "Congratulations! You have attained the level of Adept, "\
                "so you can now create personal gameclasses",
                SYSTEM,
                MESSAGE_SYSTEM
                );
        }
    }

    else if (vPrivilege.GetInteger() == ADEPT && vScore < vAdeptLevel) {
        
        if (vScore < vApprenticeLevel) {
            
            // Demote to novice
            iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, NOVICE);
            if (iErrCode != OK) {
                return iErrCode;
            }

            SendSystemMessage (
                iEmpireKey, 
                "Unfortunately, you have lost your Adept and Apprentice status and can no longer\n"\
                "create personal gameclasses or personal games",
                SYSTEM,
                MESSAGE_SYSTEM
                );
            
        } else {
            
            // Demote to apprentice
            iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::Privilege, APPRENTICE);
            if (iErrCode != OK) {
                return iErrCode;
            }
            
            SendSystemMessage (
                iEmpireKey, 
                "Unfortunately, you have lost your Adept status and can no longer create personal gameclasses",
                SYSTEM,
                MESSAGE_SYSTEM
                );
        }
    }
    
    return iErrCode;
}

int GameEngine::SendScoringChangeMessages (int iGameClass, int iGameNumber, int iNukerKey, int iNukedKey, 
                                           int iUpdate, ReasonForRemoval reason, const char* pszGameClassName, 
                                           ScoringChanges* pscChanges) {

    int iErrCode = OK;
    int iOptions;

    Assert (iNukerKey != NO_KEY);

    char pszMessage [1024 + MAX_FULL_GAME_CLASS_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH + MAX_EMPIRE_NAME_LENGTH];

    char pszAlmonasterIncrease [256] = "";
    char pszAlmonasterDecrease [256] = "";

    char pszBridierIncrease [256] = "";
    char pszBridierDecrease [256] = "";

    char pszEmpireName [MAX_EMPIRE_NAME_LENGTH + 1];

    const char* pszNukerVerb, * pszNukedVerb;

    switch (reason) {

    case EMPIRE_NUKED:
        pszNukerVerb = " nuked";
        pszNukedVerb = " were nuked by";
        break;

    case EMPIRE_SURRENDERED:
        pszNukerVerb = " accepted surrender from";
        pszNukedVerb = " surrendered to";
        break;

    case EMPIRE_INVADED:
        pszNukerVerb = " invaded";
        pszNukedVerb = " were invaded by";
        break;

    case EMPIRE_ANNIHILATED:
        pszNukerVerb = " annihilated";
        pszNukedVerb = " were annihilated by";
        break;

    case EMPIRE_COLONIZED:
        pszNukerVerb = " colonized";
        pszNukedVerb = " were colonized by";
        break;

    case EMPIRE_COLONIZED_RUINS:
        pszNukerVerb = " colonized the ruins of";
        pszNukedVerb = "r ruins were colonized by";
        break;

    default:
        Assert (false);
        return ERROR_INVALID_ARGUMENT;
    }

    //
    // Nuker empire's message
    //

    iErrCode = GetEmpireOptions (iNukerKey, &iOptions);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iOptions & SEND_SCORE_MESSAGE_ON_NUKE) {

        if (pscChanges->iFlags & ALMONASTER_NUKER_SCORE_CHANGE) {

            Assert (pscChanges->fAlmonasterNukerChange > 0);

            sprintf (
                pszAlmonasterIncrease,
                "\nYour Almonaster score increased from "\
                BEGIN_STRONG "%.3f" END_STRONG " to " BEGIN_STRONG "%.3f" END_STRONG,
                pscChanges->fAlmonasterNukerScore,
                pscChanges->fAlmonasterNukerScore + pscChanges->fAlmonasterNukerChange
                );
        }

        if (pscChanges->iFlags & ALMONASTER_NUKED_SCORE_CHANGE) {

            if (pscChanges->fAlmonasterNukedChange == 0) {

                sprintf (
                    pszAlmonasterDecrease,
                    "\nYour opponent's Almonaster score remained the same at " BEGIN_STRONG "%.3f" END_STRONG,
                    pscChanges->fAlmonasterNukedScore
                    );

            } else {

                Assert (pscChanges->fAlmonasterNukedChange < 0);

                sprintf (
                    pszAlmonasterDecrease,
                    "\nYour opponent's Almonaster score decreased from "
                    BEGIN_STRONG "%.3f" END_STRONG " to " BEGIN_STRONG "%.3f" END_STRONG,
                    pscChanges->fAlmonasterNukedScore,
                    pscChanges->fAlmonasterNukedScore + pscChanges->fAlmonasterNukedChange
                    );
            }
        }

        if (pscChanges->iFlags & BRIDIER_SCORE_CHANGE) {

            if (pscChanges->iBridierNukerRankChange == 0) {

                sprintf (
                    pszBridierIncrease,
                    "\nYour Bridier score stayed the same at " BEGIN_STRONG "%i:%i" END_STRONG,
                    pscChanges->iBridierNukerRank,
                    pscChanges->iBridierNukerIndex
                    );
            
            } else {

                Assert (pscChanges->iBridierNukerRankChange > 0);

                sprintf (
                    pszBridierIncrease,
                    "\nYour Bridier score increased from "\
                    BEGIN_STRONG "%i:%i" END_STRONG " to " BEGIN_STRONG "%i:%i" END_STRONG,
                    pscChanges->iBridierNukerRank,
                    pscChanges->iBridierNukerIndex,
                    pscChanges->iBridierNukerRank + pscChanges->iBridierNukerRankChange,
                    pscChanges->iBridierNukerIndex + pscChanges->iBridierNukerIndexChange
                    );
            }

            if (pscChanges->iBridierNukedRankChange == 0) {

                sprintf (
                    pszBridierIncrease,
                    "\nYour opponent's Bridier score remained the same at " BEGIN_STRONG "%i:%i" END_STRONG,
                    pscChanges->iBridierNukedRank,
                    pscChanges->iBridierNukedIndex
                    );
            
            } else {

                Assert (pscChanges->iBridierNukedRankChange < 0);

                sprintf (
                    pszBridierDecrease,
                    "\nYour opponent's Bridier score decreased from "\
                    BEGIN_STRONG "%i:%i" END_STRONG " to " BEGIN_STRONG "%i:%i" END_STRONG,
                    pscChanges->iBridierNukedRank,
                    pscChanges->iBridierNukedIndex,
                    pscChanges->iBridierNukedRank + pscChanges->iBridierNukedRankChange,
                    pscChanges->iBridierNukedIndex + pscChanges->iBridierNukedIndexChange
                    );
            }
        }

        // See if the caller gave us the nuked empire's name
        // This will be the case for SC 3.0-style HW colonization
        const char* pszNukedName = pscChanges->pszNukedName;
        if (pszNukedName == NULL) {

            // Otherwise, read it from the database
            Assert (iNukedKey != NO_KEY);
            iErrCode = GetEmpireName (iNukedKey, pszEmpireName);
            if (iErrCode != OK) {
                goto Cleanup;
            }
            pszNukedName = pszEmpireName;
        }

        sprintf (
            pszMessage,
            "You%s " BEGIN_STRONG "%s" END_STRONG " on update " BEGIN_STRONG "%i" END_STRONG " of "\
            "%s " BEGIN_STRONG "%i" END_STRONG "%s%s%s%s",
            pszNukerVerb,
            pszNukedName,
            iUpdate,
            pszGameClassName,
            iGameNumber,
            pszAlmonasterIncrease,
            pszAlmonasterDecrease,
            pszBridierIncrease,
            pszBridierDecrease
            );

        iErrCode = SendSystemMessage (iNukerKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
        if (iErrCode != OK) {
            goto Cleanup;
        }
    }

    //
    // Nuker empire's message
    //

    if (iNukedKey != NO_KEY) {

        iErrCode = GetEmpireOptions (iNukedKey, &iOptions);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }

        iErrCode = GetEmpireName (iNukerKey, pszEmpireName);
        if (iErrCode != OK) {
            goto Cleanup;
        }
        const char* pszNukerName = pszEmpireName;

        if (!(iOptions & SEND_SCORE_MESSAGE_ON_NUKE)) {

            // Compose a simple message
            sprintf (
                pszMessage,
                "You%s " BEGIN_STRONG "%s" END_STRONG " on update %i of %s %i",
                pszNukedVerb,
                pszNukerName,
                iUpdate,
                pszGameClassName,
                iGameNumber
                );

        } else {

            if (pscChanges->iFlags & ALMONASTER_NUKER_SCORE_CHANGE) {

                Assert (pscChanges->fAlmonasterNukerChange > 0);

                sprintf (
                    pszAlmonasterIncrease,
                    "\nYour opponent's Almonaster score increased from "\
                    BEGIN_STRONG "%.3f" END_STRONG " to " BEGIN_STRONG "%.3f" END_STRONG,
                    pscChanges->fAlmonasterNukerScore,
                    pscChanges->fAlmonasterNukerScore + pscChanges->fAlmonasterNukerChange
                    );
            }

            if (pscChanges->iFlags & ALMONASTER_NUKED_SCORE_CHANGE) {

                if (pscChanges->fAlmonasterNukedChange == 0) {

                    sprintf (
                        pszAlmonasterDecrease,
                        "\nYour Almonaster score remained the same at " BEGIN_STRONG "%.3f" END_STRONG,
                        pscChanges->fAlmonasterNukedScore
                        );

                } else {

                    Assert (pscChanges->fAlmonasterNukedChange < 0);

                    sprintf (
                        pszAlmonasterDecrease,
                        "\nYour Almonaster score decreased from "
                        BEGIN_STRONG "%.3f" END_STRONG " to " BEGIN_STRONG "%.3f" END_STRONG,
                        pscChanges->fAlmonasterNukedScore,
                        pscChanges->fAlmonasterNukedScore + pscChanges->fAlmonasterNukedChange
                        );
                }
            }

            if (pscChanges->iFlags & BRIDIER_SCORE_CHANGE) {

                if (pscChanges->iBridierNukerRankChange == 0) {

                    sprintf (
                        pszBridierIncrease,
                        "\nYour opponent's Bridier score remained the same at "\
                        BEGIN_STRONG "%i:%i" END_STRONG,
                        pscChanges->iBridierNukerRank,
                        pscChanges->iBridierNukerIndex
                        );

                } else {

                    Assert (pscChanges->iBridierNukerRankChange > 0);

                    sprintf (
                        pszBridierIncrease,
                        "\nYour opponent's Bridier score increased from "\
                        BEGIN_STRONG "%i:%i" END_STRONG " to " BEGIN_STRONG "%i:%i" END_STRONG,
                        pscChanges->iBridierNukerRank,
                        pscChanges->iBridierNukerIndex,
                        pscChanges->iBridierNukerRank + pscChanges->iBridierNukerRankChange,
                        pscChanges->iBridierNukerIndex + pscChanges->iBridierNukerIndexChange
                        );
                }

                if (pscChanges->iBridierNukedRankChange == 0) {

                    sprintf (
                        pszBridierIncrease,
                        "\nYour Bridier rank/index stayed the same at " BEGIN_STRONG "%i:%i" END_STRONG,
                        pscChanges->iBridierNukedRank,
                        pscChanges->iBridierNukedIndex
                        );

                } else {

                    Assert (pscChanges->iBridierNukedRankChange < 0);

                    sprintf (
                        pszBridierDecrease,
                        "\nYour Bridier score decreased from "\
                        BEGIN_STRONG "%i:%i" END_STRONG " to " BEGIN_STRONG "%i:%i" END_STRONG,
                        pscChanges->iBridierNukedRank,
                        pscChanges->iBridierNukedIndex,
                        pscChanges->iBridierNukedRank + pscChanges->iBridierNukedRankChange,
                        pscChanges->iBridierNukedIndex + pscChanges->iBridierNukedIndexChange
                        );
                }
            }

            sprintf (
                pszMessage,
                "You%s " BEGIN_STRONG "%s" END_STRONG " on update " BEGIN_STRONG "%i" END_STRONG " of "\
                "%s " BEGIN_STRONG "%i" END_STRONG "%s%s%s%s",
                pszNukedVerb,
                pszNukerName,
                iUpdate,
                pszGameClassName,
                iGameNumber,
                pszAlmonasterDecrease,
                pszAlmonasterIncrease,
                pszBridierDecrease,
                pszBridierIncrease
                );
        }

        // This could fail
        SendSystemMessage (iNukedKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
    }

Cleanup:

    return iErrCode;
}


//
// Assumption: schemas for all nuke lists are the same
//
int GameEngine::AddNukeToHistory (NukeList nlNukeList, const char* pszGameClassName, int iGameNumber, 
                                  int iEmpireKey, const char* pszEmpireName, int iAlienKey,
                                  int iOtherEmpireKey, const char* pszOtherEmpireName, int iOtherAlienKey) {

    int iErrCode;

    IWriteTable* pWriteTable = NULL;

    UTCTime* ptTime = NULL;
    Variant vNumNukesListed;

    unsigned int iNumRows, * piKey = NULL, i, iNumKeys, iNumNukesListed, iTimeStampCol, iLimitCol;
    char pszTable [256];

    const char* pszTemplate = NULL;

    // Get time
    UTCTime tTime;
    Time::GetTime (&tTime);

    // Get correct table name
    switch (nlNukeList) {

    case NUKER_LIST:
        
        pszTemplate = SystemEmpireNukeList::Template.Name;
        iLimitCol = SystemData::NumNukesListedInNukeHistories;
        iTimeStampCol = SystemEmpireNukeList::TimeStamp;
        GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iEmpireKey);
        break;

    case NUKED_LIST:
        
        pszTemplate = SystemEmpireNukeList::Template.Name;
        iLimitCol = SystemData::NumNukesListedInNukeHistories;
        iTimeStampCol = SystemEmpireNukeList::TimeStamp;
        GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iEmpireKey);
        break;
    
    case SYSTEM_LIST:
    
        pszTemplate = SystemNukeList::Template.Name;
        iLimitCol = SystemData::NumNukesListedInSystemNukeList;
        iTimeStampCol = SystemNukeList::TimeStamp;
        StrNCpy (pszTable, SYSTEM_NUKE_LIST);
        break;
    
    default:

        Assert (false);
        return ERROR_INVALID_ARGUMENT;
    };

    // Get nuke history limit
    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, iLimitCol, &vNumNukesListed);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iNumNukesListed = vNumNukesListed.GetInteger();

    // Fault in table
    if (!m_pGameData->DoesTableExist (pszTable)) {

        iErrCode = m_pGameData->CreateTable (pszTable, pszTemplate);
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
    }

    // Get the table
    iErrCode = m_pGameData->GetTableForWriting (pszTable, &pWriteTable);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    //
    // Delete the oldest nuke if the limit has been reached
    //
    iErrCode = pWriteTable->GetNumRows (&iNumRows);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (iNumRows >= iNumNukesListed) {
    
        // Find smallest timestamp
        iErrCode = pWriteTable->ReadColumn (
            iTimeStampCol, 
            &piKey, 
            &ptTime, 
            &iNumKeys
            );
        
        if (iErrCode != OK) {
            Assert (false);
            goto Cleanup;
        }
        
        Algorithm::QSortTwoAscending<UTCTime, unsigned int> (ptTime, piKey, iNumKeys);
        
        i = 0;
        while (i < iNumKeys && iNumRows >= iNumNukesListed) {
            
            iErrCode = pWriteTable->DeleteRow (piKey[i]);
            if (iErrCode != OK) {
                Assert (false);
                goto Cleanup;
            }
            
            iNumRows --;
            i ++;
        }
    }

    // Insert the new row
    if (nlNukeList == SYSTEM_LIST) {

        Variant pvColData [SystemNukeList::NumColumns] = {
            iAlienKey,
            pszEmpireName,
            iEmpireKey,
            iOtherAlienKey,
            pszOtherEmpireName,
            iOtherEmpireKey,
            pszGameClassName,
            iGameNumber,
            tTime
        };
        
        iErrCode = pWriteTable->InsertRow (pvColData);

    } else {
    
        Variant pvColData [SystemEmpireNukeList::NumColumns] = {
            iOtherAlienKey,
            pszOtherEmpireName,
            iOtherEmpireKey,
            pszGameClassName,
            iGameNumber,
            tTime
        };
        
        iErrCode = pWriteTable->InsertRow (pvColData);
    }
    
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pWriteTable);

    if (ptTime != NULL) {
        m_pGameData->FreeData (ptTime);
    }

    if (piKey != NULL) {
        m_pGameData->FreeKeys (piKey);
    }

    return iErrCode;
}

int GameEngine::GetBridierScore (int iEmpireKey, int* piRank, int* piIndex) {

    int iErrCode;

    IReadTable* pEmpires = NULL;

    iErrCode = m_pGameData->GetTableForReading (SYSTEM_EMPIRE_DATA, &pEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::BridierRank, piRank);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::BridierIndex, piIndex);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pEmpires);

    return iErrCode;
}

int GameEngine::TriggerBridierTimeBombIfNecessary() {

    int iErrCode;
    Variant vLastScan, vFrequency;

    UTCTime tNow;
    Time::GetTime (&tNow);

    // Check if necessary
    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::LastBridierTimeBombScan, &vLastScan);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::BridierTimeBombScanFrequency, &vFrequency);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    if (Time::GetSecondDifference (tNow, vLastScan.GetUTCTime()) > vFrequency.GetInteger()) {
        iErrCode = SendLongRunningQueryMessage (TriggerBridierTimeBombIfNecessaryMsg, NULL);
    }

    return iErrCode;
}

int GameEngine::TriggerBridierTimeBombIfNecessaryMsg (LongRunningQueryMessage* pMessage) {

    return pMessage->pGameEngine->TriggerBridierTimeBombIfNecessaryCallback();
}

int GameEngine::TriggerBridierTimeBombIfNecessaryCallback() {

    int iErrCode, iFinalIndex;

    unsigned int iKey = NO_KEY;
    
    NamedMutex nmBridierLock;
    Variant vLastAct, vIndex;

    UTCTime tNow;
    Time::GetTime (&tNow);

    iErrCode = m_pGameData->WriteData (SYSTEM_DATA, SystemData::LastShutdownTime, tNow);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }
    
    while (true) {
        
        iErrCode = m_pGameData->GetNextKey (SYSTEM_EMPIRE_DATA, iKey, &iKey);
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            return OK;
        }
        
        if (iErrCode != OK) {
            Assert (false);
            return iErrCode;
        }
        
        // Check first outside the lock
        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iKey, SystemEmpireData::LastBridierActivity, &vLastAct);
        if (iErrCode != OK) {
            continue;
        }

        Seconds sActivityDiff = Time::GetSecondDifference (tNow, vLastAct.GetUTCTime());
        if (sActivityDiff > 3 * 30 * DAY_LENGTH_IN_SECONDS) {
            
            Seconds sDiff;
            
            // Take a lock
            iErrCode = LockEmpireBridier (iKey, &nmBridierLock);
            if (iErrCode != OK) {
                continue;
            }
            
            // Refresh
            iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iKey, SystemEmpireData::LastBridierActivity, &vLastAct);
            if (iErrCode != OK) {
                UnlockEmpireBridier (nmBridierLock);
                continue;
            }
            
            iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iKey, SystemEmpireData::BridierIndex, &vIndex);
            if (iErrCode != OK) {
                UnlockEmpireBridier (nmBridierLock);
                continue;
            }
            
            iFinalIndex = vIndex.GetInteger();
            sDiff = Time::GetSecondDifference (tNow, vLastAct.GetUTCTime());
            
            if (sDiff > 3 * 30 * DAY_LENGTH_IN_SECONDS) {
                
                if (vIndex.GetInteger() < 200) {
                    iFinalIndex = 200;
                }
                
                if (sDiff > 4 * 30 * DAY_LENGTH_IN_SECONDS) {
                    
                    if (vIndex.GetInteger() < 300) {
                        iFinalIndex = 300;
                    }
                    
                    if (sDiff > 5 * 30 * DAY_LENGTH_IN_SECONDS) {
                        
                        if (vIndex.GetInteger() < 400) {
                            iFinalIndex = 400;
                        }
                        
                        if (sDiff > 6 * 30 * DAY_LENGTH_IN_SECONDS) {
                            
                            if (vIndex.GetInteger() < 500) {
                                iFinalIndex = 500;
                            }
                        }
                    }
                }
            }
            
            if (iFinalIndex != vIndex.GetInteger()) {

                iErrCode = m_pGameData->WriteData (SYSTEM_EMPIRE_DATA, iKey, SystemEmpireData::BridierIndex, iFinalIndex);
                if (iErrCode == OK) {

                    // Best effort
                    iErrCode = UpdateTopListOnDecrease (BRIDIER_SCORE, iKey);
                    Assert (iErrCode == OK);

                    iErrCode = UpdateTopListOnDecrease (BRIDIER_SCORE_ESTABLISHED, iKey);
                    Assert (iErrCode == OK);
                    
                    if (m_scConfig.bReport) {

                        Variant vName;
                        iErrCode = m_pGameData->ReadData (SYSTEM_EMPIRE_DATA, iKey, SystemEmpireData::Name, &vName);
                        if (iErrCode == OK) {
                            
                            char pszMessage [MAX_EMPIRE_NAME_LENGTH + 128];
                            sprintf (
                                pszMessage, 
                                "The Bridier Index for %s has been adjusted from %i to %i",
                                vName.GetCharPtr(),
                                vIndex.GetInteger(),
                                iFinalIndex
                                );
                            
                            m_pReport->WriteReport (pszMessage);
                        }
                    }
                }
            }
            
            UnlockEmpireBridier (nmBridierLock);
        }
    }

    return OK;
}

int GameEngine::GetBridierTimeBombScanFrequency (Seconds* piFrequency) {

    int iErrCode;
    Variant vValue;

    iErrCode = m_pGameData->ReadData (SYSTEM_DATA, SystemData::BridierTimeBombScanFrequency, &vValue);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    *piFrequency = (Seconds) vValue.GetInteger();
    return OK;
}

int GameEngine::SetBridierTimeBombScanFrequency (Seconds iFrequency) {

    Variant vValue;

    if (iFrequency < 6*60*60) {
        return ERROR_INVALID_ARGUMENT;
    }

    return m_pGameData->WriteData (SYSTEM_DATA, SystemData::BridierTimeBombScanFrequency, (int) iFrequency);
}