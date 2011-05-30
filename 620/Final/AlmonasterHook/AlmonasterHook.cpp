#include "../Almonaster/GameEngine/GameEngineSchema.h"
#include "../Almonaster/GameEngine/GameEngineConstants.h"
#include "../Almonaster/GameEngine/IGameEngine.h"

#include <stdio.h>

#include "Osal/Time.h"
#include "Osal/Crypto.h"

#define OLD_DRAW			 (0x00000010)
#define OLD_SURRENDER		 (0x00000020)
#define OLD_ACCEPT_SURRENDER (0x00000040)

#define OLD_COLONY_DISABLE_SETTLES						(0x00000004)
#define OLD_COLONY_DISABLE_SURVIVAL						(0x00000008)

#define OLD_COLONIZE_OR_DEPOSIT_POP (-42)
#define OLD_COLONIZE_AND_DISMANTLE (-69)
#define OLD_DEPOSIT_POP_AND_DISMANTLE (-70)
#define OLD_COLONIZE_OR_DEPOSIT_POP_AND_DISMANTLE (-71)

#define OLD_UPLOADED_ICON (NO_KEY)

#define OLD_UNAVAILABLE_FOR_TOURNAMENTS         (0x08000000)

#define SYSTEM_MESSAGE_SENDER "The System"

using namespace SystemData;

class AlmonasterHook : public IAlmonasterHook {
	
	IGameEngine* m_pGameEngine;
	IDatabase* m_pDatabase;

public:

	void UpgradeGameTo620 (int iGameClass, int iGameNumber) {

		Variant vTemp;
		int iErrCode;

		unsigned int iNumRows;
		unsigned iKey;

		GAME_DATA (pszGameData, iGameClass, iGameNumber);
		GAME_MAP (pszGameMap, iGameClass, iGameNumber);
		GAME_DEAD_EMPIRES (pszDeadEmps, iGameClass, iGameNumber);

		iErrCode = m_pDatabase->GetNumRows (pszGameMap, &iNumRows);
		Assert (iErrCode == OK);

		if (iNumRows > 0) {

			iErrCode = m_pDatabase->WriteOr (pszGameData, GameData::State, GAME_MAP_GENERATED);
			Assert (iErrCode == OK);
		}

		// Create new tables
		iErrCode = m_pDatabase->CreateTable (pszDeadEmps, GameDeadEmpires::Template.Name);
		Assert (iErrCode == OK);

		// SYSTEM migration
		iKey = NO_KEY;

		while (true) {

			iErrCode = m_pDatabase->GetNextKey (pszGameMap, iKey, &iKey);
			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				iErrCode = OK;
				break;
			}

			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->ReadData (
				pszGameMap,
				iKey,
				GameMap::Owner,
				&vTemp
				);

			Assert (iErrCode == OK);

			if (vTemp.GetInteger() == NO_KEY) {

				iErrCode = m_pDatabase->WriteData (
					pszGameMap,
					iKey,
					GameMap::Owner,
					SYSTEM
					);

				Assert (iErrCode == OK);
			}
		}

		// Migrate 'until' to SecondsSinceLastUpdateWhilePaused
		iErrCode = m_pDatabase->ReadData (pszGameData, GameData::State, &vTemp);
		Assert (iErrCode == OK);

		if ((vTemp.GetInteger() & PAUSED) || (vTemp.GetInteger() & ADMIN_PAUSED)) {

			Variant vSecPerUpdate;

			iErrCode = m_pDatabase->ReadData (pszGameData, GameData::SecondsSinceLastUpdateWhilePaused, &vTemp);
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->ReadData (
				SYSTEM_GAMECLASS_DATA, 
				iGameClass, 
				SystemGameClassData::NumSecPerUpdate, 
				&vSecPerUpdate
				);
			Assert (iErrCode == OK);

			int iSince = vSecPerUpdate.GetInteger() - vTemp.GetInteger();
			Assert (iSince >= 0);

            if (iSince < 0) {
                iSince = 0;
            }

			iErrCode = m_pDatabase->WriteData (
				pszGameData, 
				GameData::SecondsSinceLastUpdateWhilePaused,
				iSince
				);
			Assert (iErrCode == OK);
		}

		FixFuelAndMaintenanceUseCounts (iGameClass, iGameNumber);

		FixTargetPopTotals (iGameClass, iGameNumber);

		// Beta 2
		// Upgrade dead empires table
		iKey = NO_KEY;
		while (true) {

			bool bExists = false;
			unsigned int iEmpireKey;
            Variant vIcon;

			iErrCode = m_pDatabase->GetNextKey (pszDeadEmps, iKey, &iKey);
			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				iErrCode = OK;
				break;
			}

			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->ReadData (pszDeadEmps, iKey, GameDeadEmpires::Name, &vTemp);
			Assert (iErrCode == OK);

			iErrCode = m_pGameEngine->DoesEmpireExist (vTemp.GetCharPtr(), &bExists, &iEmpireKey, NULL);
			Assert (iErrCode == OK);

			if (bExists) {

                iErrCode = m_pGameEngine->GetEmpireProperty (iEmpireKey, SystemEmpireData::AlienKey, &vIcon);
				Assert (iErrCode == OK);

			} else {

				iEmpireKey = NO_KEY;
				
                iErrCode = m_pDatabase->ReadData (SYSTEM_DATA, SystemData::DefaultAlien, &vIcon);
				Assert (iErrCode == OK);
			}

			iErrCode = m_pDatabase->WriteData (pszDeadEmps, iKey, GameDeadEmpires::EmpireKey, iEmpireKey);
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->WriteData (pszDeadEmps, iKey, GameDeadEmpires::Icon, vIcon);
			Assert (iErrCode == OK);
		}

		// Beta 4
		iKey = NO_KEY;
		while (true) {

			iErrCode = m_pDatabase->GetNextKey (pszDeadEmps, iKey, &iKey);
			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				break;
			}
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->WriteData (pszDeadEmps, iKey, GameDeadEmpires::Update, 0);
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->WriteData (pszDeadEmps, iKey, GameDeadEmpires::Reason, "");
			Assert (iErrCode == OK);
		}

        // Beta 5
        iErrCode = m_pDatabase->WriteData (pszGameData, GameData::MinBridierRankLoss, 0);
        Assert (iErrCode == OK);

        iErrCode = m_pDatabase->WriteData (pszGameData, GameData::MaxBridierRankLoss, 0);
        Assert (iErrCode == OK);
	}

	void UpgradeEmpireInGameTo620 (int iGameClass, int iGameNumber, int iEmpireKey) {

		int iErrCode;
		unsigned int iKey;

		Variant vOffer, vDipStatus, vAction;

		GAME_EMPIRE_DATA (strGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
		GAME_EMPIRE_DIPLOMACY (pszDiplomacy, iGameClass, iGameNumber, iEmpireKey);
		GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, iEmpireKey);
        GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, iEmpireKey);

		// Beta 2
		// Fix removal of draw diplomacy option
		iErrCode = m_pDatabase->WriteAnd (strGameEmpireData, GameEmpireData::Options, (unsigned int) ~REQUEST_DRAW);
		Assert (iErrCode == OK);

		iKey = NO_KEY;
		while (true) {

			iErrCode = m_pDatabase->GetNextKey (pszDiplomacy, iKey, &iKey);
			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				iErrCode = OK;
				break;
			}

			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->ReadData (
				pszDiplomacy, 
				iKey, 
				GameEmpireDiplomacy::DipOffer, 
				&vOffer
				);

			Assert (iErrCode == OK);

			if (vOffer.GetInteger() == OLD_DRAW) {

				iErrCode = m_pDatabase->ReadData (
					pszDiplomacy, 
					iKey, 
					GameEmpireDiplomacy::CurrentStatus, 
					&vDipStatus
					);

				Assert (iErrCode == OK);

				iErrCode = m_pDatabase->WriteData (
					pszDiplomacy, 
					iKey, 
					GameEmpireDiplomacy::DipOffer, 
					vDipStatus
					);

				Assert (iErrCode == OK);
			}

			else if (vOffer.GetInteger() == OLD_SURRENDER) {

				iErrCode = m_pDatabase->WriteData (
					pszDiplomacy, 
					iKey, 
					GameEmpireDiplomacy::DipOffer, 
					SURRENDER
					);

				Assert (iErrCode == OK);
			}

			else if (vOffer.GetInteger() == OLD_ACCEPT_SURRENDER) {

				iErrCode = m_pDatabase->WriteData (
					pszDiplomacy, 
					iKey, 
					GameEmpireDiplomacy::DipOffer, 
					ACCEPT_SURRENDER
					);

				Assert (iErrCode == OK);
			}
		}

		// Ship orders
		iKey = NO_KEY;
		while (true) {

			Variant vNewAction;

			iErrCode = m_pDatabase->GetNextKey (pszShips, iKey, &iKey);
			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				iErrCode = OK;
				break;
			}

			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->ReadData (
				pszShips, 
				iKey, 
				GameEmpireShips::Action, 
				&vAction
				);

			Assert (iErrCode == OK);

			vNewAction = vAction;

			switch (vAction.GetInteger()) {

			case OLD_COLONIZE_AND_DISMANTLE:
			case OLD_COLONIZE_OR_DEPOSIT_POP_AND_DISMANTLE:
			case OLD_COLONIZE_OR_DEPOSIT_POP:

				vNewAction = COLONIZE;
				break;

			case OLD_DEPOSIT_POP_AND_DISMANTLE:

				vNewAction = DEPOSIT_POP;
				break;
			}

			if (vNewAction.GetInteger() != vAction.GetInteger()) {

				iErrCode = m_pDatabase->WriteData (
					pszShips, 
					iKey, 
					GameEmpireShips::Action, 
					vNewAction
					);

				Assert (iErrCode == OK);
			}
		}

		// Beta 4
		const char pszBegin[] = "<font size=\"+1\">Update ";

		GAME_EMPIRE_MESSAGES (pszMsgs, iGameClass, iGameNumber, iEmpireKey);

		iKey = NO_KEY;
		while (true) {

			Variant vText, vSource;

			iErrCode = m_pDatabase->GetNextKey (pszMsgs, iKey, &iKey);
			if (iErrCode == ERROR_DATA_NOT_FOUND || iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
				break;
			}
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->ReadData (pszMsgs, iKey, GameEmpireMessages::Text, &vText);
			Assert (iErrCode == OK);

			if (strncmp (pszBegin, vText.GetCharPtr(), sizeof (pszBegin) - 1) == 0) {

				iErrCode = m_pDatabase->WriteOr (pszMsgs, iKey, GameEmpireMessages::Flags, MESSAGE_UPDATE);
				Assert (iErrCode == OK);
			}

			iErrCode = m_pDatabase->ReadData (pszMsgs, iKey, GameEmpireMessages::Source, &vSource);
			Assert (iErrCode == OK);

			if (stricmp (SYSTEM_MESSAGE_SENDER, vSource.GetCharPtr()) == 0) {

				iErrCode = m_pDatabase->WriteOr (pszMsgs, iKey, GameEmpireMessages::Flags, MESSAGE_SYSTEM);
				Assert (iErrCode == OK);

				iErrCode = m_pDatabase->WriteData (pszMsgs, iKey, GameEmpireMessages::Source, (const char*) NULL);
				Assert (iErrCode == OK);
			}
		}

		iErrCode = m_pDatabase->WriteData (strGameEmpireData, GameEmpireData::GameRatios, RATIOS_DISPLAY_ON_RELEVANT_SCREENS);
        Assert (iErrCode == OK);

        // Beta 5
        iKey = NO_KEY;
		while (true) {

			iErrCode = m_pDatabase->GetNextKey (pszFleets, iKey, &iKey);
			if (iErrCode == ERROR_DATA_NOT_FOUND) {
				iErrCode = OK;
				break;
			}

			Assert (iErrCode == OK);

            unsigned int iNumShips;
            iErrCode = m_pDatabase->GetEqualKeys (
                pszShips,
                GameEmpireShips::FleetKey,
                iKey,
                false,
                NULL,
                &iNumShips
                );

            if (iErrCode == ERROR_DATA_NOT_FOUND) {
                iErrCode = OK;
            }

            Assert (iErrCode == OK);

            iErrCode = m_pDatabase->WriteData (pszFleets, iKey, GameEmpireFleets::NumShips, iNumShips);
            Assert (iErrCode == OK);
        }
	}

	void UpgradeEmpireTo620 (int iEmpireKey) {

		int iErrCode;
		/*unsigned int iKey;
        Variant vTemp;

		char pszTable [256];

		unsigned int i, iNumRows;

		iErrCode = m_pDatabase->WriteOr (
			SYSTEM_EMPIRE_DATA,
			iEmpireKey,
			SystemEmpireData::Options,
			SHOW_TECH_DESCRIPTIONS
			);

		Assert (iErrCode == OK);

		// SystemEmpireMessages
		GET_SYSTEM_EMPIRE_MESSAGES (pszTable, iEmpireKey);

		unsigned int* piKey = NULL;
		Variant* pvText = NULL;

		iErrCode = m_pDatabase->ReadColumn (
			pszTable,
			SystemEmpireMessages::Text,
			&piKey,
			&pvText,
			&iNumRows
			);

		Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND || iErrCode == ERROR_UNKNOWN_TABLE_NAME);

		if (iErrCode == OK) {

			Assert (iNumRows > 0);

			for (i = 0; i < iNumRows; i ++) {

				const char* pszText = pvText[i].GetCharPtr();

				if (strstr (pszText, "Welcome to Almonaster, ") != NULL &&
					strstr (pszText, "If you are new to the game then you should read the FAQ before you begin to play.") != NULL) {

					iErrCode = m_pDatabase->DeleteRow (pszTable, piKey[i]);
					Assert (iErrCode == OK);
				}
			}

			m_pDatabase->FreeData (pvText);
			m_pDatabase->FreeKeys (piKey);

			iErrCode = m_pDatabase->GetNumRows (pszTable, &iNumRows);
			Assert (iErrCode == OK);

			if (iNumRows == 0) {
			
				iErrCode = m_pDatabase->DeleteTable (pszTable);
				Assert (iErrCode == OK);
			}
		}

		// SystemEmpireActiveGames
		GET_SYSTEM_EMPIRE_ACTIVE_GAMES (pszTable, iEmpireKey);
		iErrCode = m_pDatabase->GetNumRows (pszTable, &iNumRows);
		Assert (iErrCode == OK || iErrCode == ERROR_UNKNOWN_TABLE_NAME);

		if (iErrCode == OK && iNumRows == 0) {
			
			iErrCode = m_pDatabase->DeleteTable (pszTable);
			Assert (iErrCode == OK);
		}

		// SystemEmpireNukedList
		GET_SYSTEM_EMPIRE_NUKED_LIST (pszTable, iEmpireKey);
		iErrCode = m_pDatabase->GetNumRows (pszTable, &iNumRows);
		Assert (iErrCode == OK || iErrCode == ERROR_UNKNOWN_TABLE_NAME);

		if (iErrCode == OK && iNumRows == 0) {
			
			iErrCode = m_pDatabase->DeleteTable (pszTable);
			Assert (iErrCode == OK);
		}

		// SystemEmpireNukerList
		GET_SYSTEM_EMPIRE_NUKER_LIST (pszTable, iEmpireKey);
		iErrCode = m_pDatabase->GetNumRows (pszTable, &iNumRows);
		Assert (iErrCode == OK || iErrCode == ERROR_UNKNOWN_TABLE_NAME);

		if (iErrCode == OK && iNumRows == 0) {
			
			iErrCode = m_pDatabase->DeleteTable (pszTable);
			Assert (iErrCode == OK);
		}

		// Beta 4
		SYSTEM_EMPIRE_MESSAGES (pszMsgs, iEmpireKey);

		iKey = NO_KEY;
		while (true) {

			Variant vSource;

			iErrCode = m_pDatabase->GetNextKey (pszMsgs, iKey, &iKey);
			if (iErrCode == ERROR_DATA_NOT_FOUND || iErrCode == ERROR_UNKNOWN_TABLE_NAME) {
				break;
			}
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->ReadData (pszMsgs, iKey, SystemEmpireMessages::Source, &vSource);
			Assert (iErrCode == OK);

			if (stricmp (SYSTEM_MESSAGE_SENDER, vSource.GetCharPtr()) == 0) {

				iErrCode = m_pDatabase->WriteOr (pszMsgs, iKey, SystemEmpireMessages::Flags, MESSAGE_SYSTEM);
				Assert (iErrCode == OK);

				iErrCode = m_pDatabase->WriteData (pszMsgs, iKey, SystemEmpireMessages::Source, (const char*) NULL);
				Assert (iErrCode == OK);
			}
		}

		iErrCode = m_pDatabase->WriteData (
			SYSTEM_EMPIRE_DATA, 
			iEmpireKey,
			SystemEmpireData::GameRatios,
			RATIOS_DISPLAY_ON_RELEVANT_SCREENS
			);
		Assert (iErrCode == OK);

		Variant vAlienKey;
		iErrCode = m_pDatabase->ReadData (
			SYSTEM_EMPIRE_DATA, 
			iEmpireKey,
			SystemEmpireData::AlienKey,
			&vAlienKey
			);
		Assert (iErrCode == OK);

		if (vAlienKey.GetInteger() == OLD_UPLOADED_ICON) {

			iErrCode = m_pDatabase->WriteData (
				SYSTEM_EMPIRE_DATA, 
				iEmpireKey,
				SystemEmpireData::AlienKey,
				UPLOADED_ICON
				);
			Assert (iErrCode == OK);
		}

        // Beta 5
        // Generate a secret key for the empire
        int64 i64SecretKey = 0;
        iErrCode = Crypto::GetRandomData ((Byte*) &i64SecretKey, sizeof (i64SecretKey));
        Assert (iErrCode == OK);
        
        iErrCode = m_pDatabase->WriteData (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey,
            SystemEmpireData::SecretKey,
            i64SecretKey
            );
        Assert (iErrCode == OK);
        i64SecretKey = 0;

        // Fix theme stuff
        iErrCode = m_pDatabase->ReadData (
			SYSTEM_EMPIRE_DATA, 
			iEmpireKey,
			SystemEmpireData::AlmonasterTheme,
			&vTemp
			);
		Assert (iErrCode == OK);

        iErrCode = m_pGameEngine->SetEmpireThemeKey (iEmpireKey, vTemp.GetInteger());
        Assert (iErrCode == OK);

        // Migrate flags
        Variant vOptions;
        iErrCode = m_pDatabase->ReadData (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey,
            SystemEmpireData::Options,
            &vOptions
            );
        Assert (iErrCode == OK);

        if (vOptions.GetInteger() & OLD_UNAVAILABLE_FOR_TOURNAMENTS) {

            iErrCode = m_pDatabase->WriteAnd (
                SYSTEM_EMPIRE_DATA, 
                iEmpireKey,
                SystemEmpireData::Options,
                (unsigned int) ~OLD_UNAVAILABLE_FOR_TOURNAMENTS
                );
            Assert (iErrCode == OK);

            iErrCode = m_pDatabase->WriteOr (
                SYSTEM_EMPIRE_DATA, 
                iEmpireKey,
                SystemEmpireData::Options2,
                UNAVAILABLE_FOR_TOURNAMENTS
                );
            Assert (iErrCode == OK);
        }
        */
        
        // RC2
        iErrCode = m_pDatabase->WriteData (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey,
            SystemEmpireData::Age,
            EMPIRE_AGE_UNKNOWN
            );
        Assert (iErrCode == OK);

        iErrCode = m_pDatabase->WriteData (
            SYSTEM_EMPIRE_DATA, 
            iEmpireKey,
            SystemEmpireData::Gender,
            EMPIRE_GENDER_UNKNOWN
            );
        Assert (iErrCode == OK);

        iErrCode = m_pGameEngine->SetEmpireOption2 (ROOT_KEY, EMPIRE_ACCEPTED_TOS, true);
        Assert (iErrCode == OK);

        iErrCode = m_pGameEngine->SetEmpireOption2 (GUEST_KEY, EMPIRE_ACCEPTED_TOS, true);
        Assert (iErrCode == OK);
    }


	void UpgradeGameClassTo620 (int iGameClass) {

		int iErrCode;
		Variant vOwner, vTournament, vValue;

		// MaxNumShips
		iErrCode = m_pDatabase->WriteData (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::MaxNumShips,
			INFINITE_SHIPS
			);

		Assert (iErrCode == OK);

		iErrCode = m_pDatabase->WriteAnd (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::Options,
			(unsigned int) ~DISABLE_SCIENCE_SHIPS_NUKING
			);

		Assert (iErrCode == OK);

		iErrCode = m_pDatabase->WriteAnd (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::Options,
			(unsigned int) ~GENERATE_MAP_FIRST_UPDATE
			);

		Assert (iErrCode == OK);

		// Tournament key
		iErrCode = m_pDatabase->WriteData (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::TournamentKey,
			NO_KEY
			);

		Assert (iErrCode == OK);

		// SYSTEM migration
		iErrCode = m_pDatabase->ReadData (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::Owner,
			&vValue
			);

		Assert (iErrCode == OK);

		if (vValue.GetInteger() == NO_KEY) {

			iErrCode = m_pDatabase->WriteData (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::Owner,
				SYSTEM
				);

			Assert (iErrCode == OK);
		}

		// Beta 2
		// Draw migration
		iErrCode = m_pDatabase->ReadData (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::DiplomacyLevel,
			&vValue
			);

		Assert (iErrCode == OK);

		if (vValue.GetInteger() & OLD_DRAW) {

			iErrCode = m_pDatabase->WriteAnd (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::DiplomacyLevel,
				(unsigned int) ~OLD_DRAW
				);

			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->WriteOr (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::Options,
				ALLOW_DRAW
				);

			Assert (iErrCode == OK);
		
		} else {

			iErrCode = m_pDatabase->WriteAnd (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::Options,
				(unsigned int) ~ALLOW_DRAW
				);

			Assert (iErrCode == OK);
		}

		if (vValue.GetInteger() & OLD_SURRENDER) {

			iErrCode = m_pDatabase->WriteAnd (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::DiplomacyLevel,
				(unsigned int) ~OLD_SURRENDER
				);

			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->WriteOr (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::DiplomacyLevel,
				SURRENDER
				);

			Assert (iErrCode == OK);
		}

		iErrCode = m_pDatabase->WriteData (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::NumInitialTechDevs,
			1
			);

		Assert (iErrCode == OK);

		// Beta 4
		iErrCode = m_pDatabase->ReadData (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::TournamentKey,
			&vTournament
			);

		Assert (iErrCode == OK);

		iErrCode = m_pDatabase->ReadData (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::Owner,
			&vOwner
			);

		Assert (iErrCode == OK);
		
		if (vTournament.GetInteger() != NO_KEY) {
			
			iErrCode = m_pDatabase->WriteData (
				SYSTEM_GAMECLASS_DATA,
				iGameClass,
				SystemGameClassData::SuperClassKey,
				TOURNAMENT
				);
			
			Assert (iErrCode == OK);
			
		} else {
			
			if (vOwner.GetInteger() != SYSTEM) {
				
				iErrCode = m_pDatabase->WriteData (
					SYSTEM_GAMECLASS_DATA,
					iGameClass,
					SystemGameClassData::SuperClassKey,
					PERSONAL_GAME
					);
				
				Assert (iErrCode == OK);
				
			}
		}
	}

	void UpgradeSystemTo620() {
		
		int iErrCode;
		Variant vEmail;

        // Beta 1
		// Create new templates
		iErrCode = m_pDatabase->CreateTemplate (SystemLatestGames::Template);
		Assert (iErrCode == OK || iErrCode == ERROR_TEMPLATE_ALREADY_EXISTS);

		iErrCode = m_pDatabase->CreateTemplate (GameDeadEmpires::Template);
		Assert (iErrCode == OK || iErrCode == ERROR_TEMPLATE_ALREADY_EXISTS);

		iErrCode = m_pDatabase->CreateTemplate (SystemTournaments::Template);
		Assert (iErrCode == OK || iErrCode == ERROR_TEMPLATE_ALREADY_EXISTS);

		iErrCode = m_pDatabase->CreateTemplate (SystemTournamentTeams::Template);
		Assert (iErrCode == OK || iErrCode == ERROR_TEMPLATE_ALREADY_EXISTS);

		iErrCode = m_pDatabase->CreateTemplate (SystemTournamentEmpires::Template);
		Assert (iErrCode == OK || iErrCode == ERROR_TEMPLATE_ALREADY_EXISTS);

		iErrCode = m_pDatabase->CreateTemplate (SystemTournamentActiveGames::Template);
		Assert (iErrCode == OK || iErrCode == ERROR_TEMPLATE_ALREADY_EXISTS);

		iErrCode = m_pDatabase->CreateTemplate (SystemEmpireTournaments::Template);
		Assert (iErrCode == OK || iErrCode == ERROR_TEMPLATE_ALREADY_EXISTS);

		// Create new tables
		iErrCode = m_pDatabase->CreateTable (SYSTEM_LATEST_GAMES, SystemLatestGames::Template.Name);
		Assert (iErrCode == OK || iErrCode == ERROR_TABLE_ALREADY_EXISTS);

		iErrCode = m_pDatabase->CreateTable (SYSTEM_TOURNAMENTS, SystemTournaments::Template.Name);
		Assert (iErrCode == OK || iErrCode == ERROR_TABLE_ALREADY_EXISTS);

		// Populate new fields
#define WriteColumn(col, val) \
		iErrCode = m_pDatabase->WriteData (SYSTEM_DATA, SystemData::col, val);	\
		Assert (iErrCode == OK);

		WriteColumn (PrivilegeForUnlimitedEmpires, ADEPT);
		WriteColumn (BuilderMultiplier, (float) 3.0);
		WriteColumn (NumNukesListedInSystemNukeList, 30);
		WriteColumn (NumGamesInLatestGameList, 30);

#undef WriteColumn

		// Beta 2
		iErrCode = m_pDatabase->WriteAnd (SYSTEM_DATA, SystemData::ShipBehavior, (unsigned int) ~OLD_COLONY_DISABLE_SETTLES);
		Assert (iErrCode == OK);

		iErrCode = m_pDatabase->WriteAnd (SYSTEM_DATA, SystemData::ShipBehavior, (unsigned int) ~OLD_COLONY_DISABLE_SURVIVAL);
		Assert (iErrCode == OK);

		// Beta 4
		iErrCode = m_pDatabase->WriteData (SYSTEM_DATA, SystemData::MaxNumPersonalTournaments, 5);
		Assert (iErrCode == OK);

		iErrCode = m_pDatabase->WriteData (SYSTEM_DATA, SystemData::MaxNumGameClassesPerPersonalTournament, 10);
		Assert (iErrCode == OK);

		iErrCode = m_pDatabase->ReadData (SYSTEM_EMPIRE_DATA, ROOT_KEY, SystemEmpireData::Email, &vEmail);
		Assert (iErrCode == OK);

		iErrCode = m_pDatabase->WriteData (SYSTEM_DATA, SystemData::AdminEmail, vEmail);
		Assert (iErrCode == OK);

		iErrCode = m_pDatabase->WriteData (SYSTEM_DATA, SystemData::SystemMessagesAlienKey, 140);
		Assert (iErrCode == OK);

        // Create Alien Glow Theme
        // Beta 4
        Variant pvColVal [SystemThemes::NumColumns];
        unsigned int iKey;

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

        iErrCode = m_pGameEngine->CreateTheme (pvColVal, &iKey);
        Assert (iErrCode == OK);

        // Create Iceberg Theme
        // RC1

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

        iErrCode = m_pGameEngine->CreateTheme (pvColVal, &iKey);
        Assert (iErrCode == OK);
    }

	void UpgradeTournamentTo620 (unsigned int iTournament) {

		// Beta 3
		int iErrCode;
		Variant vOwner;
		
		iErrCode = m_pDatabase->ReadData (
			SYSTEM_TOURNAMENTS,
			iTournament,
			SystemTournaments::Owner,
			&vOwner
			);

		Assert (iErrCode == OK);

		char pszOwner [MAX_EMPIRE_NAME_LENGTH + 1];
		int iOwner = vOwner.GetInteger();

		if (iOwner == SYSTEM) {
			strcpy (pszOwner, SYSTEM_MESSAGE_SENDER);
		}

		else if (iOwner == DELETED_EMPIRE_KEY) {
			strcpy (pszOwner, "Deleted Empire");
		}

		else {

			iErrCode = m_pDatabase->ReadData (
				SYSTEM_EMPIRE_DATA,
				iOwner,
				SystemEmpireData::Name,
				&vOwner
				);
			
			Assert (iErrCode == OK);

			strcpy (pszOwner, vOwner.GetCharPtr());
		}

		iErrCode = m_pDatabase->WriteData (
			SYSTEM_TOURNAMENTS,
			iTournament,
			SystemTournaments::OwnerName,
			pszOwner
			);

		Assert (iErrCode == OK);
	}

    void UpgradeThemeTo620 (unsigned int iThemeKey) {

        // Beta 4
        int iErrCode;
        Variant vFileName;

        iErrCode = m_pDatabase->ReadData (
			SYSTEM_THEMES,
			iThemeKey,
			SystemThemes::FileName,
			&vFileName
			);

		Assert (iErrCode == OK);

        char* pszName = String::StrDup (vFileName.GetCharPtr());
        Assert (pszName);

        String::StrLwr (pszName);

        iErrCode = m_pDatabase->WriteData (
			SYSTEM_THEMES,
			iThemeKey,
			SystemThemes::FileName,
			pszName
			);

		Assert (iErrCode == OK);

        OS::HeapFree (pszName);
    }


	//
	// Utilities
	//
	
	void ReconcileSharedMaps (int iGameClass, int iGameNumber) {

		unsigned int i, j, iNumEmpires, iKey;
		Variant* pvEmpireKey, vTemp;

		int iDipLevel;

		GAME_EMPIRES (pszGameEmpires, iGameClass, iGameNumber);

		// Get empires
		int iErrCode = m_pDatabase->ReadColumn (
			pszGameEmpires,
			GameEmpires::EmpireKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);
		
		if (iNumEmpires == 0) return;
		
		iErrCode = m_pDatabase->ReadData (
			SYSTEM_GAMECLASS_DATA, 
			iGameClass, 
			SystemGameClassData::MapsShared, 
			&vTemp
			);
		Assert (iErrCode == OK);
		
		if (vTemp.GetInteger() == NO_DIPLOMACY) return;

		iDipLevel = vTemp.GetInteger();

		//
		// Do stuff
		//
		
		// Allocate needed arrays
		unsigned int* piEmpireKey = (unsigned int*) StackAlloc (sizeof (unsigned int) * iNumEmpires);

#define TABLE_LENGTH 192
#define NUM_TABLES 3

		unsigned int iNumTables = iNumEmpires * NUM_TABLES;
		
		char* pszBuffer = (char*) StackAlloc (iNumTables * TABLE_LENGTH * sizeof (char));
		char** ppszPointers = (char**) StackAlloc (iNumTables * sizeof (char**));
		
		for (i = 0; i < iNumTables; i ++) {
			ppszPointers[i] = pszBuffer + i * TABLE_LENGTH;
		}
		
		const char** pstrEmpireMap = (const char**) ppszPointers;
		const char** pstrEmpireDip = pstrEmpireMap + iNumEmpires;
		const char** pstrEmpireData = pstrEmpireDip + iNumEmpires;

		for (i = 0; i < iNumEmpires; i ++) {
		
			piEmpireKey[i] = pvEmpireKey[i].GetInteger();
		
			GET_GAME_EMPIRE_MAP ((char*) pstrEmpireMap[i], iGameClass, iGameNumber, piEmpireKey[i]);
			GET_GAME_EMPIRE_DIPLOMACY ((char*) pstrEmpireDip[i], iGameClass, iGameNumber, piEmpireKey[i]);
			GET_GAME_EMPIRE_DATA ((char*) pstrEmpireData[i], iGameClass, iGameNumber, piEmpireKey[i]);
		}

		GAME_MAP (pszGameMap, iGameClass, iGameNumber);


		//
		// Do something
		//

		for (i = 0; i < iNumEmpires; i ++) {

			for (j = i + 1; j < iNumEmpires; j ++) {
			
				iErrCode = m_pDatabase->GetFirstKey (
					pstrEmpireDip[i],
					GameEmpireDiplomacy::EmpireKey,
					piEmpireKey[j],
					false,
					&iKey
					);

				if (iKey != NO_KEY) {

					iErrCode = m_pDatabase->ReadData (
						pstrEmpireDip[i],
						iKey,
						GameEmpireDiplomacy::CurrentStatus,
						&vTemp
						);
					Assert (iErrCode == OK);
					
					if (vTemp.GetInteger() >= iDipLevel) {

						iErrCode = m_pGameEngine->SharePlanetsBetweenFriends (
							iGameClass, 
							iGameNumber,
							i, 
							j,
							pstrEmpireMap, 
							pstrEmpireDip, 
							pstrEmpireData, 
							pszGameMap, 
							iNumEmpires, 
							piEmpireKey, 
							iDipLevel
							);
						Assert (iErrCode == OK);
					}
				}
			}
		}

		//
		// Cleanup
		//

		m_pDatabase->FreeData (pvEmpireKey);
	}

	void FixGameMinMaxCoordinates (int iGameClass, int iGameNumber) {

		// Get proper values
		Variant* pvCoord;
		unsigned int* piPlanetKey, iNumPlanets, i;
		int iMinX, iMaxX, iMinY, iMaxY, iX, iY;

		GAME_MAP (strGameMap, iGameClass, iGameNumber);
		GAME_DATA (strGameData, iGameClass, iGameNumber)

		int iErrCode = m_pDatabase->ReadColumn (
			strGameMap,
			GameMap::Coordinates,
			&piPlanetKey,
			&pvCoord,
			&iNumPlanets
			);

		if (iErrCode == OK) {

			iMinX = iMinY = MAX_COORDINATE;
			iMaxX = iMaxY = MIN_COORDINATE;

			for (i = 0; i < iNumPlanets; i ++) {

				m_pGameEngine->GetCoordinates (pvCoord[i].GetCharPtr(), &iX, &iY);

				if (iX < iMinX) {
					iMinX = iX;
				}

				if (iX > iMaxX) {
					iMaxX = iX;
				}

				if (iY < iMinY) {
					iMinY = iY;
				}

				if (iY > iMaxY) {
					iMaxY = iY;
				}
			}

			IWriteTable* pGameData;
			
			int iErrCode = m_pDatabase->GetTableForWriting (strGameData, &pGameData);
			Assert (iErrCode == OK);
			
			// Set default coordinates
			iErrCode = pGameData->WriteData (GameData::MinX, iMinX);
			Assert (iErrCode == OK);
			
			iErrCode = pGameData->WriteData (GameData::MaxX, iMaxX);
			Assert (iErrCode == OK);
			
			iErrCode = pGameData->WriteData (GameData::MinY, iMinY);
			Assert (iErrCode == OK);
			
			iErrCode = pGameData->WriteData (GameData::MaxY, iMaxY);
			Assert (iErrCode == OK);
			
			pGameData->Release();

			m_pDatabase->FreeKeys (piPlanetKey);
			m_pDatabase->FreeData (pvCoord);
		}
	}

	void FixTruceTradeAllianceCounts (int iGameClass, int iGameNumber) {

		// Count truces, trades and alliances for each empire
		Variant* pvEmpireKey;
		unsigned int i, j, iNumEmpires;
		int iErrCode;
		
		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		// Get empires
		iErrCode = m_pDatabase->ReadColumn (
			pszEmpires,
			GameEmpires::EmpireKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);
		
		if (iNumEmpires == 0) return;

		// Loop thru empires
		unsigned int iNumTruces, iNumTrades, iNumAlliances, iNumOffering, * piOfferingKey;
		Variant vStatus;

		char pszDip [256];

		for (i = 0; i < iNumEmpires; i ++) {

			GET_GAME_EMPIRE_DIPLOMACY (pszDip, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			// Truces
			iErrCode = m_pDatabase->GetEqualKeys (
				pszDip,
				GameEmpireDiplomacy::CurrentStatus,
				TRUCE,
				false,
				NULL,
				&iNumTruces
				);

			Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);

			// Trades
			iErrCode = m_pDatabase->GetEqualKeys (
				pszDip,
				GameEmpireDiplomacy::CurrentStatus,
				TRADE,
				false,
				NULL,
				&iNumTrades
				);

			Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);

			// Alliances
			iErrCode = m_pDatabase->GetEqualKeys (
				pszDip,
				GameEmpireDiplomacy::CurrentStatus,
				ALLIANCE,
				false,
				NULL,
				&iNumAlliances
				);

			Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);

			// Summation
			iNumTruces += iNumTrades + iNumAlliances;
			iNumTrades += iNumAlliances;

			// Offering truce from below
			iErrCode = m_pDatabase->GetEqualKeys (
				pszDip,
				GameEmpireDiplomacy::DipOffer,
				TRUCE,
				false,
				&piOfferingKey,
				&iNumOffering
				);

			for (j = 0; j < iNumOffering; j ++) {

				iErrCode = m_pDatabase->ReadData (
					pszDip,
					piOfferingKey[j],
					GameEmpireDiplomacy::CurrentStatus,
					&vStatus
					);
				Assert (iErrCode == OK);

				if (vStatus.GetInteger() < TRUCE) {
					iNumTruces ++;
				}
			}

			if (iNumOffering > 0) {
				m_pDatabase->FreeKeys (piOfferingKey);
			}

			// Offering trade from below
			iErrCode = m_pDatabase->GetEqualKeys (
				pszDip,
				GameEmpireDiplomacy::DipOffer,
				TRADE,
				false,
				&piOfferingKey,
				&iNumOffering
				);

			for (j = 0; j < iNumOffering; j ++) {

				iErrCode = m_pDatabase->ReadData (
					pszDip,
					piOfferingKey[j],
					GameEmpireDiplomacy::CurrentStatus,
					&vStatus
					);
				Assert (iErrCode == OK);

				if (vStatus.GetInteger() < TRADE) {
					iNumTrades ++;
				}

				if (vStatus.GetInteger() < TRUCE) {
					iNumTruces ++;
				}
			}

			if (iNumOffering > 0) {
				m_pDatabase->FreeKeys (piOfferingKey);
			}

			// Offering alliance from below
			iErrCode = m_pDatabase->GetEqualKeys (
				pszDip,
				GameEmpireDiplomacy::DipOffer,
				ALLIANCE,
				false,
				&piOfferingKey,
				&iNumOffering
				);

			for (j = 0; j < iNumOffering; j ++) {

				iErrCode = m_pDatabase->ReadData (
					pszDip,
					piOfferingKey[j],
					GameEmpireDiplomacy::CurrentStatus,
					&vStatus
					);
				Assert (iErrCode == OK);

				if (vStatus.GetInteger() < ALLIANCE) {
					iNumAlliances ++;
				}

				if (vStatus.GetInteger() < TRADE) {
					iNumTrades ++;
				}

				if (vStatus.GetInteger() < TRUCE) {
					iNumTruces ++;
				}
			}

			if (iNumOffering > 0) {
				m_pDatabase->FreeKeys (piOfferingKey);
			}

			Assert (iNumTruces >= iNumTrades);
			Assert (iNumTrades >= iNumAlliances);

			GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			// Flush
			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::NumTruces,
				iNumTruces
				);
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::NumTrades,
				iNumTrades
				);
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::NumAlliances,
				iNumAlliances
				);
			Assert (iErrCode == OK);
		}

		m_pDatabase->FreeData (pvEmpireKey);
	}

	void FixTargetPopTotals (int iGameClass, int iGameNumber) {

		// Count truces, trades and alliances for each empire
		Variant* pvEmpireKey;
		unsigned int i, j, iNumEmpires;
		int iErrCode;
		
		// Get empires
		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		iErrCode = m_pDatabase->ReadColumn (
			pszEmpires,
			GameEmpires::EmpireKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);
		
		if (iNumEmpires == 0) return;

		// Loop thru empires
		unsigned int * piPlanetKey, iNumPlanets;
		Variant vMaxPop;

		int iTargetPop;

		GAME_MAP (pszMap, iGameClass, iGameNumber);

		char pszData [256];

		for (i = 0; i < iNumEmpires; i ++) {

			iErrCode = m_pDatabase->GetEqualKeys (
				pszMap,
				GameMap::Owner,
				pvEmpireKey[i],
				false,
				&piPlanetKey,
				&iNumPlanets
				);

			if (iNumPlanets > 0) {

				iTargetPop = 0;

				for (j = 0; j < iNumPlanets; j ++) {

					iErrCode = m_pDatabase->ReadData (
						pszMap,
						piPlanetKey[j],
						GameMap::MaxPop,
						&vMaxPop
						);
					Assert (iErrCode == OK);

					iTargetPop += vMaxPop.GetInteger();
				}

				m_pDatabase->FreeKeys (piPlanetKey);
				
				GET_GAME_EMPIRE_DATA (pszData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

				iErrCode = m_pDatabase->WriteData (
					pszData,
					GameEmpireData::TargetPop,
					iTargetPop
					);
				Assert (iErrCode == OK);
			}
		}

		m_pDatabase->FreeData (pvEmpireKey);
	}

	int RemoveSpacesFromURL (int iEmpireKey) {

		Variant vUrl;
		int iErrCode = m_pDatabase->ReadData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::WebPage, &vUrl);
		if (iErrCode != OK) {
			return iErrCode;
		}

		const char* pszUrl = vUrl.GetCharPtr();

		if (pszUrl != NULL && *pszUrl != '\0' && strstr (pszUrl, " ") != NULL) {

			char* pszDup = String::StrDup (pszUrl);

			String strUrl;
			char* pszSpace, * pszCurrent = pszDup;

			while ((pszSpace = strstr (pszCurrent, " ")) != NULL) {

				*pszSpace = '\0';
				strUrl += pszCurrent;
				pszCurrent = pszSpace + 1;
			}
			strUrl += pszCurrent;

			OS::HeapFree (pszDup);

			Assert (strstr (strUrl.GetCharPtr(), " ") == NULL);

			iErrCode = m_pDatabase->WriteData (SYSTEM_EMPIRE_DATA, iEmpireKey, SystemEmpireData::WebPage, strUrl.GetCharPtr());
		}

		return OK;
	}

	void FixShipsWithNegativeBRs (int iGameClass, int iGameNumber) {

		Variant* pvEmpireKey, vTotalFuelCost, vTotalMaintCost;
		unsigned int i, j, iNumEmpires;
		int iErrCode;
		
		// Get empires
		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		iErrCode = m_pDatabase->ReadColumn (
			pszEmpires,
			GameEmpires::EmpireKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		
		if (iNumEmpires == 0) return;

		// Loop thru empires
		unsigned int* piShipKey, iNumShips;
		Variant vBR;

		char pszShips [256];

		for (i = 0; i < iNumEmpires; i ++) {

			GET_GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			iErrCode = m_pDatabase->GetAllKeys (
				pszShips,
				&piShipKey,
				&iNumShips
				);
			
			if (iNumShips > 0) {
				
				for (j = 0; j < iNumShips; j ++) {

					iErrCode = m_pDatabase->ReadData (
						pszShips,
						piShipKey[j],
						GameEmpireShips::CurrentBR,
						&vBR
						);
					Assert (iErrCode == OK);
					
					if (vBR.GetFloat() <= 0.0) {
						
						iErrCode = m_pDatabase->WriteData (
							pszShips,
							piShipKey[j],
							GameEmpireShips::CurrentBR,
							0.001
							);
						Assert (iErrCode == OK);
					}
				}

				m_pDatabase->FreeKeys (piShipKey);
			}
		}

		m_pDatabase->FreeData (pvEmpireKey);
	}


	void FixFuelAndMaintenanceUseCounts (int iGameClass, int iGameNumber) {

		Variant* pvEmpireKey, vTotalFuelCost, vTotalMaintCost;
		unsigned int i, j, iNumEmpires;
		int iErrCode;
		
		// Get empires
		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		iErrCode = m_pDatabase->ReadColumn (
			pszEmpires,
			GameEmpires::EmpireKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		
		if (iNumEmpires == 0) return;

		// Loop thru empires
		unsigned int* piShipKey, iNumShips;
		Variant vMaxBR, vType, vBuilding, vAction;
		int iFuelCost, iTotalFuelCost, iMaintCost, iTotalMaintCost, iNextFuelCost, iNextMaintCost;

		char pszShips [256], pszEmpireData [256];

		for (i = 0; i < iNumEmpires; i ++) {

			iTotalMaintCost = 0;
			iTotalFuelCost = 0;

			iNextFuelCost = 0;
			iNextMaintCost = 0;

			GET_GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			iErrCode = m_pDatabase->GetAllKeys (
				pszShips,
				&piShipKey,
				&iNumShips
				);
			
			if (iNumShips > 0) {
				
				for (j = 0; j < iNumShips; j ++) {
						
					iErrCode = m_pDatabase->ReadData (
						pszShips,
						piShipKey[j],
						GameEmpireShips::MaxBR,
						&vMaxBR
						);
					Assert (iErrCode == OK);
					
					iErrCode = m_pDatabase->ReadData (
						pszShips,
						piShipKey[j],
						GameEmpireShips::Type,
						&vType
						);
					Assert (iErrCode == OK);
					
					iFuelCost = m_pGameEngine->GetFuelCost (vType, vMaxBR);
					Assert (iFuelCost >= 0);
					
					iMaintCost = m_pGameEngine->GetMaintenanceCost (vType, vMaxBR);
					Assert (iMaintCost >= 0);

					iErrCode = m_pDatabase->ReadData (
						pszShips,
						piShipKey[j],
						GameEmpireShips::BuiltThisUpdate,
						&vBuilding
						);
					Assert (iErrCode == OK);

					if (vBuilding.GetInteger() == 0) {

						iTotalFuelCost += iFuelCost;
						iTotalMaintCost += iMaintCost;

						iErrCode = m_pDatabase->ReadData (
							pszShips,
							piShipKey[j],
							GameEmpireShips::Action,
							&vAction
							);
						Assert (iErrCode == OK);

						if (vAction.GetInteger() == DISMANTLE) {

							iNextFuelCost -= iFuelCost;
							iNextMaintCost -= iMaintCost;
						}
					
					} else {

						iNextFuelCost += iFuelCost;
						iNextMaintCost += iMaintCost;
					}
				}

				m_pDatabase->FreeKeys (piShipKey);
			}
			
			GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			
			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::TotalFuelUse,
				iTotalFuelCost
				);
			Assert (iErrCode == OK);
			
			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::TotalMaintenance,
				iTotalMaintCost
				);
			Assert (iErrCode == OK);

			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::NextFuelUse,
				iNextFuelCost
				);
			Assert (iErrCode == OK);
			
			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::NextMaintenance,
				iNextMaintCost
				);
			Assert (iErrCode == OK);
		}

		m_pDatabase->FreeData (pvEmpireKey);
	}


	void FixShipCounts (int iGameClass, int iGameNumber) {
		
		Variant* pvEmpireKey;
		unsigned int i, j, iNumEmpires, * piPlanetKey, iNumPlanets;
		int iErrCode;
		
		// Get empires
		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		iErrCode = m_pDatabase->ReadColumn (
			pszEmpires,
			GameEmpires::EmpireKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		
		if (iNumEmpires == 0) return;
		
		// Get planets
		GAME_MAP (pszMap, iGameClass, iGameNumber);

		iErrCode = m_pDatabase->GetAllKeys (
			pszMap,
			&piPlanetKey,
			&iNumPlanets
			);
		
		if (iNumPlanets == 0) {
			m_pDatabase->FreeData (pvEmpireKey);
			return;
		}
		
		// Zero counts
		for (i = 0; i < iNumPlanets; i ++) {
			
			iErrCode = m_pDatabase->WriteData (
				pszMap,
				piPlanetKey[i],
				GameMap::NumUncloakedShips,
				0
				);
			Assert (iErrCode == OK);
			
			iErrCode = m_pDatabase->WriteData (
				pszMap,
				piPlanetKey[i],
				GameMap::NumCloakedShips,
				0
				);
			Assert (iErrCode == OK);
			
			iErrCode = m_pDatabase->WriteData (
				pszMap,
				piPlanetKey[i],
				GameMap::NumUncloakedBuildShips,
				0
				);
			Assert (iErrCode == OK);
			
			iErrCode = m_pDatabase->WriteData (
				pszMap,
				piPlanetKey[i],
				GameMap::NumCloakedBuildShips,
				0
				);
			Assert (iErrCode == OK);
		}
		
		m_pDatabase->FreeKeys (piPlanetKey);
		
		// Loop thru empires
		unsigned int* piShipKey, iNumShips, iColumn, iProxyColumn, iProxyPlanetKey, iKey;
		
		Variant vPlanetKey, vFleetKey, vBuilt, vCloaked;

		char pszEmpireMap [256], pszShips [256], pszEmpireData [256], pszFleets [256];
		
		for (i = 0; i < iNumEmpires; i ++) {

			GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			
			iErrCode = m_pDatabase->GetAllKeys (
				pszEmpireMap,
				&piPlanetKey,
				&iNumPlanets
				);
			
			for (j = 0; j < iNumPlanets; j ++) {
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireMap,
					piPlanetKey[j],
					GameEmpireMap::NumUncloakedShips,
					0
					);
				Assert (iErrCode == OK);
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireMap,
					piPlanetKey[j],
					GameEmpireMap::NumCloakedShips,
					0
					);
				Assert (iErrCode == OK);
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireMap,
					piPlanetKey[j],
					GameEmpireMap::NumUncloakedBuildShips,
					0
					);
				Assert (iErrCode == OK);
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireMap,
					piPlanetKey[j],
					GameEmpireMap::NumCloakedBuildShips,
					0
					);
				Assert (iErrCode == OK);
			}
			
			m_pDatabase->FreeKeys (piPlanetKey);
			
			GET_GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			GET_GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			iErrCode = m_pDatabase->GetAllKeys (
				pszShips,
				&piShipKey,
				&iNumShips
				);
			
			for (j = 0; j < iNumShips; j ++) {
				
				iErrCode = m_pDatabase->ReadData (
					pszShips,
					piShipKey[j],
					GameEmpireShips::CurrentPlanet,
					&vPlanetKey
					);

				iErrCode = m_pDatabase->ReadData (
					pszShips,
					piShipKey[j],
					GameEmpireShips::FleetKey,
					&vFleetKey
					);

				// Does empire know about current planet?
				iErrCode = m_pDatabase->GetFirstKey (
					pszEmpireMap,
					GameEmpireMap::PlanetKey,
					vPlanetKey,
					false,
					&iKey
					);

				if (iKey == NO_KEY) {

					// Move to home world
					iErrCode = m_pDatabase->ReadData (
						pszEmpireData,
						GameEmpireData::HomeWorld,
						&vPlanetKey
						);
					
					iErrCode = m_pDatabase->WriteData (
						pszShips,
						piShipKey[j],
						GameEmpireShips::CurrentPlanet,
						vPlanetKey
						);
					
					if (vFleetKey != NO_KEY) {
						
						iErrCode = m_pDatabase->WriteData (
							pszFleets,
							vFleetKey,
							GameEmpireFleets::CurrentPlanet,
							vPlanetKey
							);
					}
				}
				
				iErrCode = m_pDatabase->ReadData (
					pszShips,
					piShipKey[j],
					GameEmpireShips::State,
					&vCloaked
					);
				
				iErrCode = m_pDatabase->ReadData (
					pszShips,
					piShipKey[j],
					GameEmpireShips::BuiltThisUpdate,
					&vBuilt
					);
				
				if (vCloaked.GetInteger() & CLOAKED) {
					iColumn = (vBuilt == 0) ? GameMap::NumCloakedShips : GameMap::NumCloakedBuildShips;
					iProxyColumn = (vBuilt == 0) ? GameEmpireMap::NumCloakedShips : GameEmpireMap::NumCloakedBuildShips;
				} else {
					iColumn = (vBuilt == 0) ? GameMap::NumUncloakedShips : GameMap::NumUncloakedBuildShips;
					iProxyColumn = (vBuilt == 0) ? GameEmpireMap::NumUncloakedShips : GameEmpireMap::NumUncloakedBuildShips;
				}
				
				// GameMap
				iErrCode = m_pDatabase->Increment (
					pszMap,
					vPlanetKey,
					iColumn,
					1
					);
				
				iErrCode = m_pDatabase->GetFirstKey (
					pszEmpireMap,
					GameEmpireMap::PlanetKey,
					vPlanetKey,
					false,
					&iProxyPlanetKey
					);
				
				if (iProxyPlanetKey == NO_KEY) {
					Assert (false);
				} else {
					
					iErrCode = m_pDatabase->Increment (
						pszEmpireMap,
						iProxyPlanetKey,
						iProxyColumn,
						1
						);
				}
			}
			
			m_pDatabase->FreeKeys (piShipKey);
		}
		
		m_pDatabase->FreeData (pvEmpireKey);

		// Independence
		Variant vIndependence;
		iErrCode = m_pDatabase->ReadData (
			SYSTEM_GAMECLASS_DATA,
			iGameClass,
			SystemGameClassData::Options,
			&vIndependence
			);
		Assert (iErrCode == OK);

		GAME_INDEPENDENT_SHIPS (pszIndependent, iGameClass, iGameNumber);

		if (vIndependence.GetInteger() & INDEPENDENCE) {

			iErrCode = m_pDatabase->GetAllKeys (
				pszIndependent,
				&piShipKey,
				&iNumShips
				);
		
			for (j = 0; j < iNumShips; j ++) {
				
				iErrCode = m_pDatabase->ReadData (
					pszIndependent,
					piShipKey[j],
					GameEmpireShips::CurrentPlanet,
					&vPlanetKey
					);
				
				iErrCode = m_pDatabase->ReadData (
					pszIndependent,
					piShipKey[j],
					GameEmpireShips::State,
					&vCloaked
					);
				
				iErrCode = m_pDatabase->ReadData (
					pszIndependent,
					piShipKey[j],
					GameEmpireShips::BuiltThisUpdate,
					&vBuilt
					);
				
				if (vCloaked.GetInteger() & CLOAKED) {
					iColumn = (vBuilt == 0) ? GameMap::NumCloakedShips : GameMap::NumCloakedBuildShips;
				} else {
					iColumn = (vBuilt == 0) ? GameMap::NumUncloakedShips : GameMap::NumUncloakedBuildShips;
				}
				
				// GameMap
				iErrCode = m_pDatabase->Increment (
					pszMap,
					vPlanetKey,
					iColumn,
					1
					);
			}
		}
	}

	void FixMaxMinCounts (int iGameClass, int iGameNumber) {
		
		Variant* pvEmpireKey, * pvPlanetKey, vCoord;
		unsigned int i, j, iNumEmpires, iNumPlanets;
		int iErrCode, iNum, iX, iY, iMaxX, iMaxY, iMinX, iMinY;
		
		// Get empires
		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		iErrCode = m_pDatabase->ReadColumn (
			pszEmpires,
			GameEmpires::EmpireKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		
		if (iNumEmpires == 0) return;

		char pszEmpireMap [256], pszEmpireData [256];
		
		for (i = 0; i < iNumEmpires; i ++) {
			
			GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			iErrCode = m_pDatabase->ReadColumn (
				pszEmpireMap,
				GameEmpireMap::PlanetKey,
				&pvPlanetKey,
				&iNumPlanets
				);
			
			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::MinX,
				999999999
				);
			
			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::MinY,
				999999999
				);
			
			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::MaxX,
				-100
				);
			
			iErrCode = m_pDatabase->WriteData (
				pszEmpireData,
				GameEmpireData::MaxY,
				-100
				);
			
			GAME_MAP (pszMap, iGameClass, iGameNumber);

			if (iNumPlanets > 0) {
				
				iErrCode = m_pDatabase->ReadData (
					pszMap,
					pvPlanetKey[0],
					GameMap::Coordinates,
					&vCoord
					);
				
				iNum = sscanf (vCoord.GetCharPtr(), "%i,%i", &iX, &iY);
				Assert (iNum == 2);
				
				iMinX = iX;
				iMaxX = iX;
				iMinY = iY;
				iMaxY = iY;
				
				for (j = 1; j < iNumPlanets; j ++) {
					
					iErrCode = m_pDatabase->ReadData (
						pszMap,
						pvPlanetKey[j],
						GameMap::Coordinates,
						&vCoord
						);
					
					iNum = sscanf (vCoord.GetCharPtr(), "%i,%i", &iX, &iY);
					Assert (iNum == 2);
					
					if (iX < iMinX) iMinX = iX;
					if (iX > iMaxX) iMaxX = iX;
					if (iY < iMinY) iMinY = iY;
					if (iY > iMaxY) iMaxY = iY;
				}
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireData,
					GameEmpireData::MinX,
					iMinX
					);
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireData,
					GameEmpireData::MinY,
					iMinY
					);
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireData,
					GameEmpireData::MaxX,
					iMaxX
					);
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireData,
					GameEmpireData::MaxY,
					iMaxY
					);
				
				m_pDatabase->FreeData (pvPlanetKey);
			}
		}
		
		m_pDatabase->FreeData (pvEmpireKey);
	}

	void FixSharedMaps (int iGameClass, int iGameNumber) {
		
		Variant* pvEmpireKey, * pvPlanetKey, vNeighbour;
		unsigned int iNumEmpires, i, j, k, l, * piProxyKey, iNumPlanets, * piEmpireProxyKey;
		
		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		int iErrCode = m_pDatabase->ReadColumn (
			pszEmpires,
			GameEmpires::EmpireKey,
			&piEmpireProxyKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		Assert (iErrCode == OK);
		
		char pszEmpireMap [256];

		GAME_MAP (pszMap, iGameClass, iGameNumber);

		for (i = 0; i < iNumEmpires; i ++) {
			
			GET_GAME_EMPIRE_MAP (pszEmpireMap, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			iErrCode = m_pDatabase->ReadColumn (
				pszEmpireMap,
				GameEmpireMap::PlanetKey,
				&piProxyKey,
				&pvPlanetKey,
				&iNumPlanets
				);
			
			if (iNumPlanets == 0) {
				return;
			}
			
			for (j = 0; j < iNumPlanets; j ++) {
				
				ENUMERATE_CARDINAL_POINTS (k) {
					
					iErrCode = m_pDatabase->ReadData (
						pszMap,
						pvPlanetKey[j],
						GameMap::NorthPlanetKey + k,
						&vNeighbour
						);
					Assert (iErrCode == OK);
					
					if (vNeighbour != NO_KEY) {
						
						for (l = 0; l < iNumPlanets; l ++) {
							
							if (pvPlanetKey[l] == vNeighbour) {
								
								iErrCode = m_pDatabase->WriteOr (
									pszEmpireMap,
									piProxyKey[j],
									GameEmpireMap::Explored,
									EXPLORED_X[k]
									);
								Assert (iErrCode == OK);
								
								iErrCode = m_pDatabase->WriteOr (
									pszEmpireMap,
									piProxyKey[l],
									GameEmpireMap::Explored,
									OPPOSITE_EXPLORED_X[k]
									);
								Assert (iErrCode == OK);
								
								break;
							}
						}
						
						if (l == iNumPlanets) {
							
							iErrCode = m_pDatabase->WriteAnd (
								pszEmpireMap,
								piProxyKey[j],
								GameEmpireMap::Explored,
								~EXPLORED_X[k]
								);
							Assert (iErrCode == OK);
						}		
					}
				}
			}
			
			m_pDatabase->FreeData (pvPlanetKey);
			m_pDatabase->FreeKeys (piProxyKey);
		}
		
		m_pDatabase->FreeData (pvEmpireKey);
		m_pDatabase->FreeKeys (piEmpireProxyKey);
	}

	void FixExploredFields (int iGameClass, int iGameNumber, int iEmpireKey) {

		int cpDir, iErrCode, iKey, iExplored, * piPlanetKey = NULL;

		unsigned int* piKey = NULL, iNumKeys, i, iProxyKey;

		IReadTable* pGameMap = NULL, * pGameEmpireMapRead = NULL;
		IWriteTable* pGameEmpireMapWrite = NULL;

		GAME_MAP (strGameMap, iGameClass, iGameNumber);
		GAME_EMPIRE_MAP (strGameEmpireMap, iGameClass, iGameNumber, iEmpireKey);

		iErrCode = m_pDatabase->GetTableForReading (strGameMap, &pGameMap);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = m_pDatabase->GetTableForWriting (strGameEmpireMap, &pGameEmpireMapWrite);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = pGameEmpireMapWrite->QueryInterface (IID_IReadTable, (void**) &pGameEmpireMapRead);
		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		iErrCode = pGameEmpireMapRead->ReadColumn (GameEmpireMap::PlanetKey, &piKey, &piPlanetKey, &iNumKeys);
		if (iErrCode == ERROR_DATA_NOT_FOUND) {
			goto Cleanup;
		}

		if (iErrCode != OK) {
			Assert (false);
			goto Cleanup;
		}

		for (i = 0; i < iNumKeys; i ++) {

			iExplored = 0;

			ENUMERATE_CARDINAL_POINTS (cpDir) {
				
				iErrCode = pGameMap->ReadData (
					piPlanetKey[i],
					GameMap::NorthPlanetKey + cpDir,
					&iKey
					);
				
				if (iErrCode != OK) {
					Assert (false);
					goto Cleanup;
				}
				
				if (iKey != NO_KEY) {
				
					iErrCode = pGameEmpireMapRead->GetFirstKey (
						GameEmpireMap::PlanetKey,
						iKey,
						false,
						&iProxyKey
						);
					
					if (iErrCode == ERROR_DATA_NOT_FOUND) {
						
						Assert (iProxyKey == NO_KEY);
						iErrCode = OK;
						
					} else {
						
						if (iErrCode != OK) {
							Assert (false);
							goto Cleanup;
						}
						
						Assert (iProxyKey != NO_KEY);
						
						iErrCode = pGameEmpireMapWrite->WriteOr (
							iProxyKey,
							GameEmpireMap::Explored,
							OPPOSITE_EXPLORED_X[cpDir]
							);
						
						if (iErrCode != OK) {
							Assert (false);
							goto Cleanup;
						}
						
						iExplored |= EXPLORED_X[cpDir];
					}
				}
			}

			iErrCode = pGameEmpireMapWrite->WriteData (
				piKey[i],
				GameEmpireMap::Explored,
				iExplored
				);

			if (iErrCode != OK) {
				Assert (false);
				goto Cleanup;
			}
		}

Cleanup:

		if (piKey != NULL) {
			m_pDatabase->FreeKeys (piKey);
		}

		if (piPlanetKey != NULL) {
			m_pDatabase->FreeData (piPlanetKey);
		}

		if (pGameMap != NULL) {
			pGameMap->Release();
		}

		if (pGameEmpireMapRead != NULL) {
			pGameEmpireMapRead->Release();
		}

		if (pGameEmpireMapWrite != NULL) {
			pGameEmpireMapWrite->Release();
		}
	}

	void ReconcileShipAndFleetLocations (int iGameClass, int iGameNumber) {
		
		Variant* pvEmpireKey, vShipPlanetKey, vFleetPlanetKey, vFleetKey;
		unsigned int iNumEmpires, i, j, * piShipKey, iNumShips;
		
		GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);

		int iErrCode = m_pDatabase->ReadColumn (
			pszEmpires,
			GameEmpires::EmpireKey,
			&pvEmpireKey,
			&iNumEmpires
			);
		Assert (iErrCode == OK);

		char pszShips [256], pszFleets [256];
		
		for (i = 0; i < iNumEmpires; i ++) {
			
			GET_GAME_EMPIRE_SHIPS (pszShips, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());
			GET_GAME_EMPIRE_FLEETS (pszFleets, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			iErrCode = m_pDatabase->GetAllKeys (
				pszShips,
				&piShipKey,
				&iNumShips
				);
			
			if (iNumShips > 0) {
				
				Assert (iErrCode == OK);
				
				for (j = 0; j < iNumShips; j ++) {
					
					iErrCode = m_pDatabase->ReadData (
						pszShips,
						piShipKey[j],
						GameEmpireShips::FleetKey,
						&vFleetKey
						);
					Assert (iErrCode == OK);
					
					if (vFleetKey != NO_KEY) {
						
						iErrCode = m_pDatabase->ReadData (
							pszShips,
							piShipKey[j],
							GameEmpireShips::CurrentPlanet,
							&vShipPlanetKey
							);
						Assert (iErrCode == OK);
						
						iErrCode = m_pDatabase->ReadData (
							pszFleets,
							vFleetKey,
							GameEmpireFleets::CurrentPlanet,
							&vFleetPlanetKey
							);
						Assert (iErrCode == OK);
						
						if (vShipPlanetKey != vFleetPlanetKey) {
							
							// Move ship to fleet's planet:
							// NOTE: ship counts will need to be fixed afterwards
							iErrCode = m_pDatabase->WriteData (
								pszShips,
								piShipKey[j],
								GameEmpireShips::CurrentPlanet,
								vFleetPlanetKey
								);
							Assert (iErrCode == OK);
						}
					}
				}
				
				m_pDatabase->FreeKeys (piShipKey);
			}
		}
		
		m_pDatabase->FreeData (pvEmpireKey);
	}


	/********************************************************************************/

	//
	// ForEachEmpireInEachGame
	//

	void ForEachEmpireInEachGame() {
		
		unsigned int i, j, iNumKeys, iNumEmpires;
		int iGameClass, iGameNumber, iTemp, iErrCode;
		Variant* pvGame, * pvEmpireKey;
		
		iErrCode = m_pDatabase->ReadColumn (
			SYSTEM_ACTIVE_GAMES,
			SystemActiveGames::GameClassGameNumber,
			&pvGame,
			&iNumKeys
			);
		
		if (iNumKeys > 0) {
		
			Assert (iErrCode == OK);

			char pszEmpires [256];

			for (i = 0; i < iNumKeys; i ++) {
				
				iTemp = sscanf (pvGame[i].GetCharPtr(), "%i.%i", &iGameClass, &iGameNumber);
				Assert (iTemp == 2);

				GET_GAME_EMPIRES (pszEmpires, iGameClass, iGameNumber);
				
				// Get empires
				iErrCode = m_pDatabase->ReadColumn (
					pszEmpires,
					GameEmpires::EmpireKey,
					&pvEmpireKey,
					&iNumEmpires
					);
				Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);

				for (j = 0; j < iNumEmpires; j ++) {

					// TODO
					UpgradeEmpireInGameTo620 (iGameClass, iGameNumber, pvEmpireKey[j].GetInteger());
				}

				if (iNumEmpires > 0) {
					m_pDatabase->FreeData (pvEmpireKey);
				}
			}
			
			m_pDatabase->FreeData (pvGame);
		}
	}

	//
	// ForEachGame
	//

	void ForEachGame() {
		
		unsigned int i, iNumKeys;
		int iGameClass, iGameNumber, iTemp, iErrCode;
		Variant* pvGame;
		
		iErrCode = m_pDatabase->ReadColumn (
			SYSTEM_ACTIVE_GAMES,
			SystemActiveGames::GameClassGameNumber,
			&pvGame,
			&iNumKeys
			);
		
		if (iNumKeys > 0) {
		
			Assert (iErrCode == OK);

			for (i = 0; i < iNumKeys; i ++) {
				
				iTemp = sscanf (pvGame[i].GetCharPtr(), "%i.%i", &iGameClass, &iGameNumber);
				Assert (iTemp == 2);

				// TODO
				UpgradeGameTo620 (iGameClass, iGameNumber);
			}
			
			m_pDatabase->FreeData (pvGame);
		}
	}

	//
	// ForEachEmpire
	//

	void ForEachEmpire() {
		
		unsigned int* piEmpireKey, iNumEmpires, i;
		
		int iErrCode = m_pDatabase->GetAllKeys (
			SYSTEM_EMPIRE_DATA,
			&piEmpireKey,
			&iNumEmpires
			);
		Assert (iErrCode == OK);
		
		for (i = 0; i < iNumEmpires; i ++) {
			
			// TODO
			UpgradeEmpireTo620 (piEmpireKey[i]);
		}
		
		if (iNumEmpires > 0) {
			m_pDatabase->FreeKeys (piEmpireKey);
		}
	}

	// 
	// ForEachGameClass
	// 

	void ForEachGameClass() {
		
		unsigned int* piGameClassKey, iNumGameClasses, i;
		
		int iErrCode = m_pDatabase->GetAllKeys (
			SYSTEM_GAMECLASS_DATA,
			&piGameClassKey,
			&iNumGameClasses
			);
		Assert (iErrCode == OK);
		
		for (i = 0; i < iNumGameClasses; i ++) {

			// TODO
			UpgradeGameClassTo620 (piGameClassKey[i]);
		}
		
		if (iNumGameClasses > 0) {
			m_pDatabase->FreeKeys (piGameClassKey);
		}
	}

    //
    // ForEachTournament
    //

	void ForEachTournament() {
		
		unsigned int* piKey, iNumKeys, i;
		
		int iErrCode = m_pDatabase->GetAllKeys (
			SYSTEM_TOURNAMENTS,
			&piKey,
			&iNumKeys
			);
		Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);
		
		for (i = 0; i < iNumKeys; i ++) {

			// TODO
			UpgradeTournamentTo620 (piKey[i]);
		}
		
		if (iNumKeys > 0) {
			m_pDatabase->FreeKeys (piKey);
		}
	}

    //
    // ForEachTheme
    //

	void ForEachTheme() {
		
		unsigned int* piKey, iNumKeys, i;
		
		int iErrCode = m_pDatabase->GetAllKeys (
			SYSTEM_THEMES,
			&piKey,
			&iNumKeys
			);
		Assert (iErrCode == OK || iErrCode == ERROR_DATA_NOT_FOUND);
		
		for (i = 0; i < iNumKeys; i ++) {

			// TODO
			UpgradeThemeTo620 (piKey[i]);
		}
		
		if (iNumKeys > 0) {
			m_pDatabase->FreeKeys (piKey);
		}
	}

	//
	// System
	//
	void System() {

		// TODO
		UpgradeSystemTo620();
	}


	/****************************************************/

	AlmonasterHook() {

		m_iNumRefs = 1;
		m_pGameEngine = NULL;
		m_pDatabase = NULL;
	}

	~AlmonasterHook() {
	
		if (m_pDatabase != NULL) {
			m_pDatabase->Release();
		}
	}

	IMPLEMENT_IOBJECT;

	int Initialize (IGameEngine* pGameEngine) {

		m_pGameEngine = pGameEngine;

		return OK;
	}

	int Setup() {

		m_pDatabase = m_pGameEngine->GetDatabase();

		//System();
        //ForEachTheme();
		ForEachEmpire();
		//ForEachGameClass();
		//ForEachTournament();
		//ForEachGame();
		//ForEachEmpireInEachGame();

		return OK;
	}

	int Running() {

		return OK;
	}

	int Finalize() {
		return OK;
	}
};

extern "C" EXPORT IAlmonasterHook* AlmonasterHookCreateInstance() {
	return new AlmonasterHook();
}