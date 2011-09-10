//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
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
#include "Global.h"

// Input:
// iNukerKey -> Nuking empire
// iNukedKey -> Nuked empire
// iGameClass -> GameClass key
// iGameNumber -> Game number
//
// Update Scores when a nuke occurs
int GameEngine::UpdateScoresOnNuke(int iNukerKey, int iNukedKey, const char* pszNukerName, 
                                   const char* pszNukedName, int iGameClass, int iGameNumber,
                                   int iUpdate, ReasonForRemoval reason, const char* pszGameClassName) {

    int iErrCode;

    GET_SYSTEM_EMPIRE_DATA(strNukedEmpire, iNukedKey);
    GET_SYSTEM_EMPIRE_DATA(strNukerEmpire, iNukerKey);

    // Report
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

    global.GetReport()->WriteReport (pszMessage);

    // Increment nukes and nuked
    iErrCode = t_pCache->Increment(strNukerEmpire, iNukerKey, SystemEmpireData::Nukes, 1);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->Increment(strNukedEmpire, iNukedKey, SystemEmpireData::Nuked, 1);
    RETURN_ON_ERROR(iErrCode);

    // Update nuker and nuked lists
    int iNukedAlienKey, iNukerAlienKey;
    Variant vValue, vSneer;

    if (t_pCache->ReadData(strNukedEmpire, iNukedKey, SystemEmpireData::AlienKey, &vValue) == OK)
    {
        iNukedAlienKey = vValue.GetInteger();
    }
    else
    {
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

    RETURN_ON_ERROR(iErrCode);

    if (t_pCache->ReadData(strNukerEmpire, iNukerKey, SystemEmpireData::AlienKey, &vValue) == OK)
    {
        iNukerAlienKey = vValue.GetInteger();
    }
    else
    {
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

    RETURN_ON_ERROR(iErrCode);

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

    RETURN_ON_ERROR(iErrCode);

    // Update scores
    ScoringChanges scChanges;
    scChanges.pszNukedName = pszNukedName;

    if (reason == EMPIRE_SURRENDERED)
    {
        iErrCode = UpdateScoresOnSurrender (iGameClass, iGameNumber, iNukerKey, iNukedKey, &scChanges);
        RETURN_ON_ERROR(iErrCode);
    }
    else
    {
        iErrCode = UpdateScoresOnNuke (iGameClass, iGameNumber, iNukerKey, iNukedKey, &scChanges);
        RETURN_ON_ERROR(iErrCode);
    }

    // Send messages
    iErrCode = SendScoringChangeMessages (iGameClass, iGameNumber, iNukerKey, iNukedKey, iUpdate, reason, pszGameClassName, &scChanges);
    RETURN_ON_ERROR(iErrCode);

    // Send victory sneer
    iErrCode = SendVictorySneer (iNukerKey, pszNukerName, iNukedKey);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::UpdateScoresOnSurrender (int iNukerKey, int iNukedKey, const char* pszNukerName, 
                                         const char* pszNukedName, int iGameClass, int iGameNumber,
                                         int iUpdate, const char* pszGameClassName) {

    return UpdateScoresOnNuke(
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
    char pszMessage [MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + 64];
    sprintf(pszMessage, "%s surrendered out of %s %i", pszLoserName, pszGameClassName, iGameNumber);
    global.GetReport()->WriteReport(pszMessage);

    // Increment nuked for loser
    GET_SYSTEM_EMPIRE_DATA(strLoser, iLoserKey);
    iErrCode = t_pCache->Increment(strLoser, iLoserKey, SystemEmpireData::Nuked, 1);
    RETURN_ON_ERROR(iErrCode);

    // Ignore scoring changes for this event
    ScoringChanges scChanges;
    iErrCode = UpdateScoresOn30StyleSurrender (iGameClass, iGameNumber, iLoserKey, &scChanges);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::UpdateScoresOn30StyleSurrenderColonization (int iWinnerKey, int iPlanetKey, 
                                                            const char* pszWinnerName, int iGameClass, 
                                                            int iGameNumber, int iUpdate,
                                                            const char* pszGameClassName) {

    int iErrCode, iLoserKey;

    GET_GAME_MAP (strGameMap, iGameClass, iGameNumber);

    Variant vPlanetName, vEmpireSecretKey, vNukedKey, vTemp;
    char pszNukedName [MAX_EMPIRE_NAME_LENGTH + 1] = "";

    bool bValid;

    iErrCode = t_pCache->ReadData(strGameMap, iPlanetKey, GameMap::Name, &vPlanetName);
    RETURN_ON_ERROR(iErrCode);

    // Report
    char pszMessage [MAX_EMPIRE_NAME_LENGTH + MAX_FULL_GAME_CLASS_NAME_LENGTH + MAX_PLANET_NAME_LENGTH + 64];
        
    sprintf (
        pszMessage,
        "%s colonized %s in %s %i",
        pszWinnerName,
        vPlanetName.GetCharPtr(),
        pszGameClassName,
        iGameNumber
        );

    global.GetReport()->WriteReport (pszMessage);

    // Give empire credit for nuke
    GET_SYSTEM_EMPIRE_DATA(strWinner, iWinnerKey);
    iErrCode = t_pCache->Increment(strWinner, iWinnerKey, SystemEmpireData::Nukes, 1);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(
        strGameMap, 
        iPlanetKey, 
        GameMap::SurrenderEmpireSecretKey, 
        &vEmpireSecretKey
        );

    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(
        strGameMap, 
        iPlanetKey, 
        GameMap::HomeWorld, 
        &vNukedKey
        );

    RETURN_ON_ERROR(iErrCode);

    Assert(vNukedKey.GetInteger() != NO_KEY);

    iErrCode = CheckSecretKey (vNukedKey.GetInteger(), vEmpireSecretKey.GetInteger64(), &bValid, NULL, NULL);
    RETURN_ON_ERROR(iErrCode);

    if (bValid) {
        iLoserKey = vNukedKey.GetInteger();
    } else {
        iLoserKey = NO_KEY;
    }

    GET_SYSTEM_EMPIRE_DATA(strWinnerEmpire, iLoserKey);
    GET_SYSTEM_EMPIRE_DATA(strLoserEmpire, iLoserKey);

    // Parse out empire's name
    int scanf = sscanf (vPlanetName.GetCharPtr(), RUINS_OF, pszNukedName);
    Assert(scanf == 1);

    int iNukerAlienKey, iNukedAlienKey;

    // Get nuker alien key
    iErrCode = t_pCache->ReadData(strWinnerEmpire, iWinnerKey, SystemEmpireData::AlienKey, &vTemp);
    RETURN_ON_ERROR(iErrCode);
    iNukerAlienKey = vTemp.GetInteger();

    // Get nuked alien key
    if (iLoserKey == NO_KEY) {

        iNukedAlienKey = NO_KEY;

    } else {

        iErrCode = t_pCache->ReadData(strLoserEmpire, iLoserKey, SystemEmpireData::AlienKey, &vTemp);
        RETURN_ON_ERROR(iErrCode);
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

        RETURN_ON_ERROR(iErrCode);

        // Send the loser a victory sneer
        iErrCode = SendVictorySneer(iWinnerKey, pszWinnerName, iLoserKey);
        RETURN_ON_ERROR(iErrCode);
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

    RETURN_ON_ERROR(iErrCode);

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

    RETURN_ON_ERROR(iErrCode);

    ScoringChanges scChanges;
    scChanges.pszNukedName = pszNukedName;

    iErrCode = UpdateScoresOn30StyleSurrenderColonization (iGameClass, iGameNumber, iWinnerKey, iPlanetKey, &scChanges);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = SendScoringChangeMessages (iGameClass, iGameNumber, iWinnerKey, iLoserKey, iUpdate, EMPIRE_COLONIZED_RUINS, pszGameClassName, &scChanges);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


int GameEngine::UpdateScoresOnGameEnd (int iGameClass, int iGameNumber)
{
    int i;
    ENUMERATE_SCORING_SYSTEMS(i)
    {
        IScoringSystem* pScoringSystem = CreateScoringSystem((ScoringSystem)i);
        int iErrCode = pScoringSystem->OnGameEnd(iGameClass, iGameNumber);
        delete pScoringSystem;
        RETURN_ON_ERROR(iErrCode);
    }
    return OK;
}

int GameEngine::UpdateScoresOnNuke (int iGameClass, int iGameNumber, int iNukerKey, int iNukedKey, ScoringChanges* pscChanges)
{
    int i;
    ENUMERATE_SCORING_SYSTEMS(i)
    {
        IScoringSystem* pScoringSystem = CreateScoringSystem((ScoringSystem)i);
        int iErrCode = pScoringSystem->OnNuke(iGameClass, iGameNumber, iNukerKey, iNukedKey, pscChanges);
        delete pScoringSystem;
        RETURN_ON_ERROR(iErrCode);
    }
    return OK;
}

int GameEngine::UpdateScoresOnSurrender (int iGameClass, int iGameNumber, int iWinnerKey, int iLoserKey, ScoringChanges* pscChanges)
{
    int i;
    ENUMERATE_SCORING_SYSTEMS(i)
    {
        IScoringSystem* pScoringSystem = CreateScoringSystem((ScoringSystem)i);
        int iErrCode = pScoringSystem->OnSurrender(iGameClass, iGameNumber, iWinnerKey, iLoserKey, pscChanges);
        delete pScoringSystem;
        RETURN_ON_ERROR(iErrCode);
    }
    return OK;
}

int GameEngine::UpdateScoresOn30StyleSurrender (int iGameClass, int iGameNumber, int iLoserKey, ScoringChanges* pscChanges)
{
    int i;
    ENUMERATE_SCORING_SYSTEMS(i)
    {
        IScoringSystem* pScoringSystem = CreateScoringSystem((ScoringSystem)i);
        int iErrCode = pScoringSystem->On30StyleSurrender(iGameClass, iGameNumber, iLoserKey, pscChanges);
        delete pScoringSystem;
        RETURN_ON_ERROR(iErrCode);
    }
    return OK;
}

int GameEngine::UpdateScoresOn30StyleSurrenderColonization (int iGameClass, int iGameNumber, int iWinnerKey, int iPlanetKey, ScoringChanges* pscChanges)
{
    int i;
    ENUMERATE_SCORING_SYSTEMS(i)
    {
        IScoringSystem* pScoringSystem = CreateScoringSystem((ScoringSystem)i);
        int iErrCode = pScoringSystem->On30StyleSurrenderColonization(iGameClass, iGameNumber, iWinnerKey, iPlanetKey, pscChanges);
        delete pScoringSystem;
        RETURN_ON_ERROR(iErrCode);
    }
    return OK;
}

int GameEngine::UpdateScoresOnWin (int iGameClass, int iGameNumber, int iEmpireKey)
{
    // Add win to empires' statistics
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    int iErrCode = t_pCache->Increment(strEmpire, iEmpireKey, SystemEmpireData::Wins, 1);
    RETURN_ON_ERROR(iErrCode);

    int i;
    ENUMERATE_SCORING_SYSTEMS(i)
    {
        IScoringSystem* pScoringSystem = CreateScoringSystem((ScoringSystem)i);
        iErrCode = pScoringSystem->OnWin(iGameClass, iGameNumber, iEmpireKey);
        delete pScoringSystem;
        RETURN_ON_ERROR(iErrCode);
    }
    return OK;
}

int GameEngine::UpdateScoresOnDraw (int iGameClass, int iGameNumber, int iEmpireKey)
{
    // Add draw to empires' statistics
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    int iErrCode = t_pCache->Increment(strEmpire, iEmpireKey, SystemEmpireData::Draws, 1);
    RETURN_ON_ERROR(iErrCode);

    int i;
    ENUMERATE_SCORING_SYSTEMS(i)
    {
        IScoringSystem* pScoringSystem = CreateScoringSystem((ScoringSystem)i);
        iErrCode = pScoringSystem->OnDraw(iGameClass, iGameNumber, iEmpireKey);
        delete pScoringSystem;
        RETURN_ON_ERROR(iErrCode);
    }
    return OK;
}


int GameEngine::UpdateScoresOnRuin (int iGameClass, int iGameNumber, int iEmpireKey)
{
    // Add ruin to empires' statistics
    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    int iErrCode = t_pCache->Increment(strEmpire, iEmpireKey, SystemEmpireData::Ruins, 1);
    RETURN_ON_ERROR(iErrCode);

    int i;
    ENUMERATE_SCORING_SYSTEMS(i)
    {
        IScoringSystem* pScoringSystem = CreateScoringSystem((ScoringSystem)i);
        iErrCode = pScoringSystem->OnRuin(iGameClass, iGameNumber, iEmpireKey);
        delete pScoringSystem;
        RETURN_ON_ERROR(iErrCode);
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

int GameEngine::GetNumEmpiresInNukeHistory(int iEmpireKey, int* piNumNukes, int* piNumNuked)
{
    *piNumNukes = *piNumNuked = 0;

    GET_SYSTEM_EMPIRE_NUKER_LIST(strNukerTable, iEmpireKey);
    int iErrCode = t_pCache->GetNumCachedRows(strNukerTable, (unsigned int*) piNumNukes);
    RETURN_ON_ERROR(iErrCode);

    GET_SYSTEM_EMPIRE_NUKED_LIST(strNukedTable, iEmpireKey);
    iErrCode = t_pCache->GetNumCachedRows(strNukedTable, (unsigned int*) piNumNuked);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


// Input:
// iEmpireKey -> Empire Key
//
// Output
//
// Return an empire's nuke history, unsorted

int GameEngine::GetNukeHistory(int iEmpireKey, int* piNumNuked, Variant*** pppvNukedData, int* piNumNukers, Variant*** pppvNukerData)
{
    int iErrCode;

    Variant** ppvNukedData = NULL, ** ppvNukerData = NULL;
    AutoFreeData free1(ppvNukedData);
    AutoFreeData free2(ppvNukerData);

    GET_SYSTEM_EMPIRE_NUKER_LIST(strNuker, iEmpireKey);
    GET_SYSTEM_EMPIRE_NUKED_LIST(strNuked, iEmpireKey);

    const char* pszNukeColumns[] = 
    {
        SystemEmpireNukeList::EmpireKey,
        SystemEmpireNukeList::AlienKey,
        SystemEmpireNukeList::EmpireName,
        SystemEmpireNukeList::ReferenceEmpireKey,
        SystemEmpireNukeList::GameClassName,
        SystemEmpireNukeList::GameNumber,
        SystemEmpireNukeList::TimeStamp
    };

    *piNumNuked = *piNumNukers = 0;
    *pppvNukedData = *pppvNukerData = NULL;

    // Get Nuked data
    iErrCode = t_pCache->ReadColumns (
        strNuked, 
        countof(pszNukeColumns),
        pszNukeColumns,
        NULL,
        &ppvNukedData,
        (unsigned int*) piNumNuked
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    // Get Nuker data
    iErrCode = t_pCache->ReadColumns (
        strNuker, 
        countof(pszNukeColumns),
        pszNukeColumns,
        NULL,
        &ppvNukerData,
        (unsigned int*) piNumNukers
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND)
    {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    *pppvNukedData = ppvNukedData;
    ppvNukedData = NULL;

    *pppvNukerData = ppvNukerData;
    ppvNukerData = NULL;

    return iErrCode;
}

// Output
//
// Return the system's nuke history, unsorted

int GameEngine::GetSystemNukeHistory (int* piNumNukes, Variant*** pppvNukedData) {

    const char* pszNukeColumns[] = {
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

    int iErrCode = t_pCache->ReadColumns (
        SYSTEM_NUKE_LIST, 
        countof(pszNukeColumns),
        pszNukeColumns,
        NULL,
        pppvNukedData,
        (unsigned int*) piNumNukes
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


int GameEngine::GetSystemLatestGames (int* piNumGames, Variant*** pppvGameData) {

    const char* pszColumns[] = {
        SystemLatestGames::Name,
        SystemLatestGames::Number,
        SystemLatestGames::Created,
        SystemLatestGames::Ended,
        SystemLatestGames::Updates,
        SystemLatestGames::Result,
        SystemLatestGames::Winners,
        SystemLatestGames::Losers,
    };

    int iErrCode = t_pCache->ReadColumns (
        SYSTEM_LATEST_GAMES,
        countof(pszColumns),
        pszColumns,
        NULL,
        pppvGameData,
        (unsigned int*) piNumGames
        );

    if (iErrCode == ERROR_DATA_NOT_FOUND) {
        iErrCode = OK;
    }
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}


int GameEngine::CalculatePrivilegeLevel(int iEmpireKey) {

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);

    int iErrCode, iSystemOptions, iEmpireOptions2;
    Variant vAdeptLevel, vApprenticeLevel, vScore, vPrivilege;

    // Admins stay the same
    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, &vPrivilege);
    RETURN_ON_ERROR(iErrCode);

    if (vPrivilege.GetInteger() == ADMINISTRATOR) {
        return OK;
    }

    // See if the system allows it
    iErrCode = GetSystemOptions (&iSystemOptions);
    RETURN_ON_ERROR(iErrCode);

    if (iSystemOptions & DISABLE_PRIVILEGE_SCORE_ELEVATION) {
        return OK;
    }

    // See if the empire's settings allow it
    iErrCode = GetEmpireOptions2 (iEmpireKey, &iEmpireOptions2);
    RETURN_ON_ERROR(iErrCode);

    if (iEmpireOptions2 & ADMINISTRATOR_FIXED_PRIVILEGE) {
        return OK;
    }

    // Read the data we need
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::AdeptScore, &vAdeptLevel);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::ApprenticeScore, &vApprenticeLevel);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(strEmpire, iEmpireKey, SystemEmpireData::AlmonasterScore, &vScore);
    RETURN_ON_ERROR(iErrCode);

    // Check for attainment of adepthood
    if (vPrivilege.GetInteger() == NOVICE) {
        
        if (vScore < vAdeptLevel && vScore >= vApprenticeLevel) {
            
            // Advance to apprentice
            iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, APPRENTICE);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = SendSystemMessage (
                iEmpireKey,
                "Congratulations! You have attained the level of Apprentice and can now create your own personal games",
                SYSTEM,
                MESSAGE_SYSTEM
                );
            RETURN_ON_ERROR(iErrCode);
        }
        
        else if (vScore >= vAdeptLevel) {
            
            // Advance to adept
            iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, ADEPT);         
            RETURN_ON_ERROR(iErrCode);

            iErrCode = SendSystemMessage (
                iEmpireKey, 
                "Congratulations! You have attained the level of Adept, "\
                "so you can now create your own personal gameclasses",
                SYSTEM,
                MESSAGE_SYSTEM
                );
            RETURN_ON_ERROR(iErrCode);
        }
    }
    
    // Check for loss of privilege
    else if (vPrivilege.GetInteger() == APPRENTICE) {
        
        if (vScore < vApprenticeLevel) {
            
            // Demote to novice
            iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, NOVICE);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = SendSystemMessage (
                iEmpireKey, 
                "Unfortunately, you have lost your Apprentice status, "\
                "so you can no longer create your own personal games",
                SYSTEM,
                MESSAGE_SYSTEM
                );
            RETURN_ON_ERROR(iErrCode);
        }
        
        else if (vScore >= vAdeptLevel) {
            
            // Advance to adept
            iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, ADEPT);         
            RETURN_ON_ERROR(iErrCode);

            iErrCode = SendSystemMessage (
                iEmpireKey, 
                "Congratulations! You have attained the level of Adept, "\
                "so you can now create personal gameclasses",
                SYSTEM,
                MESSAGE_SYSTEM
                );
            RETURN_ON_ERROR(iErrCode);
        }
    }

    else if (vPrivilege.GetInteger() == ADEPT && vScore < vAdeptLevel) {
        
        if (vScore < vApprenticeLevel) {
            
            // Demote to novice
            iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, NOVICE);
            RETURN_ON_ERROR(iErrCode);

            iErrCode = SendSystemMessage (
                iEmpireKey, 
                "Unfortunately, you have lost your Adept and Apprentice status and can no longer\n"\
                "create personal gameclasses or personal games",
                SYSTEM,
                MESSAGE_SYSTEM
                );
            RETURN_ON_ERROR(iErrCode);
            
        } else {
            
            // Demote to apprentice
            iErrCode = t_pCache->WriteData(strEmpire, iEmpireKey, SystemEmpireData::Privilege, APPRENTICE);
            RETURN_ON_ERROR(iErrCode);
            
            iErrCode = SendSystemMessage (
                iEmpireKey, 
                "Unfortunately, you have lost your Adept status and can no longer create personal gameclasses",
                SYSTEM,
                MESSAGE_SYSTEM
                );
            RETURN_ON_ERROR(iErrCode);
        }
    }
    
    return iErrCode;
}

int GameEngine::SendScoringChangeMessages (int iGameClass, int iGameNumber, int iNukerKey, int iNukedKey, 
                                           int iUpdate, ReasonForRemoval reason, const char* pszGameClassName, 
                                           ScoringChanges* pscChanges) {

    int iErrCode = OK;
    int iOptions;

    Assert(iNukerKey != NO_KEY);

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
        Assert(false);
        return ERROR_INVALID_ARGUMENT;
    }

    //
    // Nuker empire's message
    //

    iErrCode = GetEmpireOptions (iNukerKey, &iOptions);
    RETURN_ON_ERROR(iErrCode);

    if (iOptions & SEND_SCORE_MESSAGE_ON_NUKE) {

        if (pscChanges->iFlags & ALMONASTER_NUKER_SCORE_CHANGE) {

            Assert(pscChanges->fAlmonasterNukerChange > 0);

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

                Assert(pscChanges->fAlmonasterNukedChange < 0);

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

                Assert(pscChanges->iBridierNukerRankChange > 0);

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

                Assert(pscChanges->iBridierNukedRankChange < 0);

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
            Assert(iNukedKey != NO_KEY);
            iErrCode = GetEmpireName (iNukedKey, pszEmpireName);
            RETURN_ON_ERROR(iErrCode);
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
        RETURN_ON_ERROR(iErrCode);
    }

    //
    // Nuker empire's message
    //

    if (iNukedKey != NO_KEY) {

        iErrCode = GetEmpireOptions (iNukedKey, &iOptions);
        RETURN_ON_ERROR(iErrCode);

        iErrCode = GetEmpireName (iNukerKey, pszEmpireName);
        RETURN_ON_ERROR(iErrCode);
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

                Assert(pscChanges->fAlmonasterNukerChange > 0);

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

                    Assert(pscChanges->fAlmonasterNukedChange < 0);

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

                    Assert(pscChanges->iBridierNukerRankChange > 0);

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

                    Assert(pscChanges->iBridierNukedRankChange < 0);

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

        iErrCode = SendSystemMessage(iNukedKey, pszMessage, SYSTEM, MESSAGE_SYSTEM);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}


//
// Assumption: schemas for all nuke lists are the same
//
int GameEngine::AddNukeToHistory(NukeList nlNukeList, const char* pszGameClassName, int iGameNumber, 
                                 int iEmpireKey, const char* pszEmpireName, int iAlienKey,
                                 int iOtherEmpireKey, const char* pszOtherEmpireName, int iOtherAlienKey) {

    int iErrCode;

    ICachedTable* pWriteTable = NULL;
    AutoRelease<ICachedTable> rel(pWriteTable);

    Variant* pvTime = NULL, vNumNukesListed;
    AutoFreeData freeData(pvTime);

    unsigned int iNumRows, * piKey = NULL, i, iNumKeys, iNumNukesListed;
    AutoFreeKeys freeKeys(piKey);
    char pszTable [256];

    const char* pszLimitCol = NULL, * pszTimeStampCol = NULL;
    const TemplateDescription* pttTemplate = NULL;

    // Get time
    UTCTime tTime;
    Time::GetTime (&tTime);

    // Get correct table name
    switch (nlNukeList) {

    case NUKER_LIST:
        
        pttTemplate = &SystemEmpireNukeList::Template;
        pszLimitCol = SystemData::NumNukesListedInNukeHistories;
        pszTimeStampCol = SystemEmpireNukeList::TimeStamp;
        COPY_SYSTEM_EMPIRE_NUKER_LIST(pszTable, iEmpireKey);
        break;

    case NUKED_LIST:
        
        pttTemplate = &SystemEmpireNukeList::Template;
        pszLimitCol = SystemData::NumNukesListedInNukeHistories;
        pszTimeStampCol = SystemEmpireNukeList::TimeStamp;
        COPY_SYSTEM_EMPIRE_NUKED_LIST(pszTable, iEmpireKey);
        break;
    
    case SYSTEM_LIST:
    
        pttTemplate = &SystemNukeList::Template;
        pszLimitCol = SystemData::NumNukesListedInSystemNukeList;
        pszTimeStampCol = SystemNukeList::TimeStamp;
        StrNCpy(pszTable, SYSTEM_NUKE_LIST);
        break;
    
    default:

        Assert(false);
        return ERROR_INVALID_ARGUMENT;
    };

    // Get nuke history limit
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, pszLimitCol, &vNumNukesListed);
    RETURN_ON_ERROR(iErrCode);

    iNumNukesListed = vNumNukesListed.GetInteger();

    // Get the table
    iErrCode = t_pCache->GetTable(pszTable, &pWriteTable);
    RETURN_ON_ERROR(iErrCode);

    //
    // Delete the oldest nuke if the limit has been reached
    //
    iErrCode = pWriteTable->GetNumCachedRows(&iNumRows);
    RETURN_ON_ERROR(iErrCode);

    if (iNumRows >= iNumNukesListed) {
    
        // Find smallest timestamp
        iErrCode = pWriteTable->ReadColumn(
            pszTimeStampCol, 
            &piKey, 
            &pvTime, 
            &iNumKeys
            );
        
        RETURN_ON_ERROR(iErrCode);
        
        Algorithm::QSortTwoAscending<Variant, unsigned int>(pvTime, piKey, iNumKeys);
        
        i = 0;
        while (i < iNumKeys && iNumRows >= iNumNukesListed) {
            
            iErrCode = pWriteTable->DeleteRow(piKey[i]);
            RETURN_ON_ERROR(iErrCode);
            
            iNumRows --;
            i ++;
        }
    }

    // Insert the new row
    if (nlNukeList == SYSTEM_LIST) {

        Variant pvColData[SystemNukeList::NumColumns] = 
        {
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
        
        iErrCode = pWriteTable->InsertRow(SystemNukeList::Template, pvColData, NULL);
        RETURN_ON_ERROR(iErrCode);

    } else {
    
        Variant pvColData[SystemEmpireNukeList::NumColumns] = 
        {
            iEmpireKey,
            iOtherAlienKey,
            pszOtherEmpireName,
            iOtherEmpireKey,
            pszGameClassName,
            iGameNumber,
            tTime
        };
        
        iErrCode = pWriteTable->InsertRow(SystemEmpireNukeList::Template, pvColData, NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::GetBridierScore (int iEmpireKey, int* piRank, int* piIndex) {

    int iErrCode;

    ICachedTable* pEmpires = NULL;
    AutoRelease<ICachedTable> rel(pEmpires);

    GET_SYSTEM_EMPIRE_DATA(strEmpire, iEmpireKey);
    iErrCode = t_pCache->GetTable(strEmpire, &pEmpires);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpires->ReadData(iEmpireKey, SystemEmpireData::BridierRank, piRank);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = pEmpires->ReadData(iEmpireKey, SystemEmpireData::BridierIndex, piIndex);
    RETURN_ON_ERROR(iErrCode);

    return iErrCode;
}

int GameEngine::TriggerBridierTimeBombIfNecessary() {

    int iErrCode;
    Variant vLastScan, vFrequency;

    UTCTime tNow;
    Time::GetTime (&tNow);

    // Check if necessary
    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::LastBridierTimeBombScan, &vLastScan);
    RETURN_ON_ERROR(iErrCode);

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::BridierTimeBombScanFrequency, &vFrequency);
    RETURN_ON_ERROR(iErrCode);

    if (Time::GetSecondDifference (tNow, vLastScan.GetInteger64()) > vFrequency.GetInteger())
    {
        iErrCode = global.GetAsyncManager()->QueueTask(TriggerBridierTimeBombIfNecessaryMsg, NULL);
        RETURN_ON_ERROR(iErrCode);
    }

    return iErrCode;
}

int GameEngine::TriggerBridierTimeBombIfNecessaryMsg(AsyncTask* pMessage)
{
    GameEngine gameEngine;
    return gameEngine.TriggerBridierTimeBombIfNecessaryCallback();
}

int GameEngine::TriggerBridierTimeBombIfNecessaryCallback()
{
    // TODOTODO - rewrite Bridier time bomb scanning to be more efficient

    //int iErrCode, iFinalIndex;

    //unsigned int iKey = NO_KEY;
    //
    //Variant vLastAct, vIndex;

    //UTCTime tNow;
    //Time::GetTime(&tNow);

    //while (true) {
    //    
    //    iErrCode = t_pCache->GetNextKey(SYSTEM_EMPIRE_DATA, iKey, &iKey);
    //    if (iErrCode == ERROR_DATA_NOT_FOUND) {
    //        return OK;
    //    }
    //    
    //    if (iErrCode != OK) {
    //        Assert(false);
    //        return iErrCode;
    //    }
    //    
    //    iErrCode = t_pCache->ReadData(SYSTEM_EMPIRE_DATA, iKey, SystemEmpireData::LastBridierActivity, &vLastAct);
    //    if (iErrCode != OK) {
    //        continue;
    //    }

    //    Seconds sActivityDiff = Time::GetSecondDifference (tNow, vLastAct.GetInteger64());
    //    if (sActivityDiff > 3 * 30 * DAY_LENGTH_IN_SECONDS) {
    //        
    //        Seconds sDiff;
    //        
    //        // Refresh
    //        iErrCode = t_pCache->ReadData(SYSTEM_EMPIRE_DATA, iKey, SystemEmpireData::LastBridierActivity, &vLastAct);
    //        if (iErrCode != OK) {
    //            continue;
    //        }
    //        
    //        iErrCode = t_pCache->ReadData(SYSTEM_EMPIRE_DATA, iKey, SystemEmpireData::BridierIndex, &vIndex);
    //        if (iErrCode != OK) {
    //            continue;
    //        }
    //        
    //        iFinalIndex = vIndex.GetInteger();
    //        sDiff = Time::GetSecondDifference (tNow, vLastAct.GetInteger64());
    //        
    //        if (sDiff > 3 * 30 * DAY_LENGTH_IN_SECONDS) {
    //            
    //            if (vIndex.GetInteger() < 200) {
    //                iFinalIndex = 200;
    //            }
    //            
    //            if (sDiff > 4 * 30 * DAY_LENGTH_IN_SECONDS) {
    //                
    //                if (vIndex.GetInteger() < 300) {
    //                    iFinalIndex = 300;
    //                }
    //                
    //                if (sDiff > 5 * 30 * DAY_LENGTH_IN_SECONDS) {
    //                    
    //                    if (vIndex.GetInteger() < 400) {
    //                        iFinalIndex = 400;
    //                    }
    //                    
    //                    if (sDiff > 6 * 30 * DAY_LENGTH_IN_SECONDS) {
    //                        
    //                        if (vIndex.GetInteger() < 500) {
    //                            iFinalIndex = 500;
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //        
    //        if (iFinalIndex != vIndex.GetInteger())
    //        {
    //            iErrCode = t_pCache->WriteData(strEmpire, iKey, SystemEmpireData::BridierIndex, iFinalIndex);
    //            if (iErrCode == OK)
    //            {
    //                Variant vName;
    //                iErrCode = t_pCache->ReadData(strEmpire, iKey, SystemEmpireData::Name, &vName);
    //                if (iErrCode == OK)
    //                {
    //                    char pszMessage [MAX_EMPIRE_NAME_LENGTH + 128];
    //                    sprintf (
    //                        pszMessage, 
    //                        "The Bridier Index for %s has been adjusted from %i to %i",
    //                        vName.GetCharPtr(),
    //                        vIndex.GetInteger(),
    //                        iFinalIndex
    //                        );
    //                        
    //                    global.GetReport()->WriteReport (pszMessage);
    //                }
    //            }
    //        }
    //    }
    //}

    return OK;
}

int GameEngine::GetBridierTimeBombScanFrequency (Seconds* piFrequency) {

    int iErrCode;
    Variant vValue;

    iErrCode = t_pCache->ReadData(SYSTEM_DATA, SystemData::BridierTimeBombScanFrequency, &vValue);
    RETURN_ON_ERROR(iErrCode);

    *piFrequency = (Seconds) vValue.GetInteger();
    return OK;
}

int GameEngine::SetBridierTimeBombScanFrequency (Seconds iFrequency) {

    Variant vValue;

    if (iFrequency < 6*60*60) {
        return ERROR_INVALID_ARGUMENT;
    }

    return t_pCache->WriteData(SYSTEM_DATA, SystemData::BridierTimeBombScanFrequency, (int) iFrequency);
}