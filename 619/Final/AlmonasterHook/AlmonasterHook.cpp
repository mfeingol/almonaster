#include "../Almonaster/GameEngine/GameEngineSchema.h"
#include "../Almonaster/GameEngine/GameEngineConstants.h"
#include "../Almonaster/GameEngine/IGameEngine.h"

#include <stdio.h>

#include "Osal/Time.h"

#define NAMES_LISTED					(0x00000020)
#define WARN_ON_DUPLICATE_IP_ADDRESS	(0x00001000)
#define BLOCK_ON_DUPLICATE_IP_ADDRESS	(0x00002000)
#define WARN_ON_DUPLICATE_SESSION_ID	(0x00004000)
#define BLOCK_ON_DUPLICATE_SESSION_ID	(0x00008000)

using namespace SystemData;

class AlmonasterHook : public IAlmonasterHook {
	
	IGameEngine* m_pGameEngine;
	IDatabase* m_pDatabase;

public:

	void AlmonasterHook::UpgradeTo619 (int iGameClass, int iGameNumber) {
	
	}

	void AlmonasterHook::UpgradeTo619 (int iGameClass, int iGameNumber, int iEmpireKey) {

		Variant vIdle;
		int iErrCode;

		GAME_EMPIRE_DATA (pszGameEmpireData, iGameClass, iGameNumber, iEmpireKey);
		
		iErrCode = m_pDatabase->ReadData (pszGameEmpireData, GameEmpireData::NumUpdatesIdle, &vIdle);
		Assert (iErrCode == OK);
		
		if (vIdle.GetInteger() == 0) {

			iErrCode = m_pDatabase->WriteOr (pszGameEmpireData, GameEmpireData::Options, LOGGED_IN_THIS_UPDATE);
			Assert (iErrCode == OK);

		} else {
		
			iErrCode = m_pDatabase->WriteAnd (pszGameEmpireData, GameEmpireData::Options, ~LOGGED_IN_THIS_UPDATE);
			Assert (iErrCode == OK);
		}
	}

	void AlmonasterHook::UpgradeTo619 (int iEmpireKey) {

	}

	void AlmonasterHook::UpgradeGameClassTo619 (int iGameClass) {

	}

	void AlmonasterHook::UpgradeTo619() {

	}

	void AlmonasterHook::ReconcileSharedMaps (int iGameClass, int iGameNumber) {

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
		
		if (vTemp == NO_DIPLOMACY) return;

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

void AlmonasterHook::FixGameMinMaxCoordinates (int iGameClass, int iGameNumber) {

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

	void AlmonasterHook::FixTruceTradeAllianceCounts (int iGameClass, int iGameNumber) {

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

	void AlmonasterHook::FixTargetPopTotals (int iGameClass, int iGameNumber) {

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

	int AlmonasterHook::RemoveSpacesFromURL (int iEmpireKey) {

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

	void AlmonasterHook::FixShipsWithNegativeBRs (int iGameClass, int iGameNumber) {

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


	void AlmonasterHook::FixFuelAndMaintenanceUseCounts (int iGameClass, int iGameNumber) {

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
		Variant vMaxBR, vType, vBuilding;
		int iFuelCost, iTotalFuelCost, iMaintCost, iTotalMaintCost;

		char pszShips [256], pszEmpireData [256];

		for (i = 0; i < iNumEmpires; i ++) {

			iTotalMaintCost = 0;
			iTotalFuelCost = 0;

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
						GameEmpireShips::BuiltThisUpdate,
						&vBuilding
						);
					Assert (iErrCode == OK);

					if (vBuilding == 0) {
						
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
						iTotalFuelCost += iFuelCost;
						
						iMaintCost = m_pGameEngine->GetMaintenanceCost (vType, vMaxBR);
						Assert (iMaintCost >= 0);
						iTotalMaintCost += iMaintCost;
					}
				}

				m_pDatabase->FreeKeys (piShipKey);
			}

			GET_GAME_EMPIRE_DATA (pszEmpireData, iGameClass, iGameNumber, pvEmpireKey[i].GetInteger());

			iErrCode = m_pDatabase->ReadData (
				pszEmpireData,
				GameEmpireData::TotalFuelUse,
				&vTotalFuelCost
				);
			Assert (iErrCode == OK);

			if (vTotalFuelCost != iTotalFuelCost) {

				iErrCode = m_pDatabase->WriteData (
					pszEmpireData,
					GameEmpireData::TotalFuelUse,
					iTotalFuelCost
					);
				Assert (iErrCode == OK);
			}

			iErrCode = m_pDatabase->ReadData (
				pszEmpireData,
				GameEmpireData::TotalMaintenance,
				&vTotalMaintCost
				);
			Assert (iErrCode == OK);

			if (vTotalMaintCost != iTotalMaintCost) {
				
				iErrCode = m_pDatabase->WriteData (
					pszEmpireData,
					GameEmpireData::TotalMaintenance,
					iTotalMaintCost
					);
				Assert (iErrCode == OK);
			}
		}

		m_pDatabase->FreeData (pvEmpireKey);
	}


	void AlmonasterHook::FixShipCounts (int iGameClass, int iGameNumber) {
		
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

	void AlmonasterHook::FixMaxMinCounts (int iGameClass, int iGameNumber) {
		
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

	void AlmonasterHook::FixSharedMaps (int iGameClass, int iGameNumber) {
		
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

	void AlmonasterHook::FixExploredFields (int iGameClass, int iGameNumber, int iEmpireKey) {

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

	void AlmonasterHook::ReconcileShipAndFleetLocations (int iGameClass, int iGameNumber) {
		
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

	void AlmonasterHook::ForEachEmpireInEachGame() {
		
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
					UpgradeTo619 (iGameClass, iGameNumber, pvEmpireKey[j].GetInteger());
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

	void AlmonasterHook::ForEachGame() {
		
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
				UpgradeTo619 (iGameClass, iGameNumber);
			}
			
			m_pDatabase->FreeData (pvGame);
		}
	}

	//
	// ForEachEmpire
	//

	void AlmonasterHook::ForEachEmpire() {
		
		unsigned int* piEmpireKey, iNumEmpires, i;
		
		int iErrCode = m_pDatabase->GetAllKeys (
			SYSTEM_EMPIRE_DATA,
			&piEmpireKey,
			&iNumEmpires
			);
		Assert (iErrCode == OK);
		
		for (i = 0; i < iNumEmpires; i ++) {
			
			// TODO
			UpgradeTo619 (piEmpireKey[i]);
		}
		
		if (iNumEmpires > 0) {
			m_pDatabase->FreeKeys (piEmpireKey);
		}
	}

	// 
	// ForEachGameClass
	// 

	void AlmonasterHook::ForEachGameClass() {
		
		unsigned int* piGameClassKey, iNumGameClasses, i;
		
		int iErrCode = m_pDatabase->GetAllKeys (
			SYSTEM_GAMECLASS_DATA,
			&piGameClassKey,
			&iNumGameClasses
			);
		Assert (iErrCode == OK);
		
		for (i = 0; i < iNumGameClasses; i ++) {

			// TODO
			UpgradeGameClassTo619 (piGameClassKey[i]);
		}
		
		if (iNumGameClasses > 0) {
			m_pDatabase->FreeKeys (piGameClassKey);
		}
	}

	//
	// System
	//
	void AlmonasterHook::System() {

		// TODO
		UpgradeTo619();
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

		System();
		ForEachEmpire();
		ForEachGame();
		ForEachGameClass();
		ForEachEmpireInEachGame();

		return OK;
	}

	int Running() {

		//System();
		//ForEachEmpire();
		//ForEachGameClass();
		//ForEachGame();

		return OK;
	}

	int Finalize() {
		return OK;
	}
};

extern "C" EXPORT IAlmonasterHook* AlmonasterHookCreateInstance() {
	return new AlmonasterHook();
}